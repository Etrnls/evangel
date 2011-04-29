/***************************************************************************
 *   Copyright (C) 2010 by Etrnls                                          *
 *   Etrnls@gmail.com                                                      *
 *                                                                         *
 *   This file is part of Evangel.                                         *
 *                                                                         *
 *   Evangel is free software: you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Evangel is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Evangel.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/
#include "udptrackerclientsocket.h"

#include "network/udpsocket.h"
#include "util/endian.h"
#include "util/numeric/random.h"

#include <QUrl>
#include <QHostAddress>
#include <QTimerEvent>

namespace Protocol
{

namespace BitTorrent
{

UDPTrackerClientSocket::UDPTrackerClientSocket(const QUrl &URL)
	: Network::UDPSocket(Network::UDPSocket::PrivilegePriority),
	connectionID(0), transactionID(0), timeoutTimer(0), timeoutCounter(0)
{
	connectToHost(QHostAddress(URL.host()), URL.port());
}

void UDPTrackerClientSocket::request(const QByteArray &data)
{
	Q_ASSERT_X(!hasPendingRequest(), Q_FUNC_INFO, "Pending request not finished yet");

	if (!checkConnectionID())
	{
		pendingRequest= data;
		requestConnectionID();
		return;
	}

	{
		QByteArray packet = data;
		Utility::Endian::toBigEndian<quint64>(connectionID, &packet.data()[0]);
		Utility::Endian::toBigEndian<quint32>(generateTransactionID(), &packet.data()[12]);

		sendRequest(packet);
	}
}

UDPTrackerClientSocket::RequestAction UDPTrackerClientSocket::getRequestAction(const QByteArray &data)
{
	return static_cast<RequestAction>(Utility::Endian::fromBigEndian<quint32>(&data.constData()[8]));
}

bool UDPTrackerClientSocket::checkConnectionID() const
{
	static const int connectionIDExpireTime = 60 * 1000; // 1 minute
	return connectionIDStopwatch.isValid() && !connectionIDStopwatch.hasExpired(connectionIDExpireTime);
}

void UDPTrackerClientSocket::requestConnectionID()
{
/*
	Offset	Size		Name		Value
	0	64-bit integer	connection_id	0x41727101980
	8	32-bit integer	action		0 // Connect
	12	32-bit integer	transaction_id	Randomized
	16
*/

	QByteArray data(16, Qt::Uninitialized);

	Utility::Endian::toBigEndian<quint64>(Q_UINT64_C(0x41727101980), &data.data()[0]);
	Utility::Endian::toBigEndian<quint32>(UDPTrackerClient::Connect, &data.data()[8]);
	Utility::Endian::toBigEndian<quint32>(generateTransactionID(), &data.data()[12]);

	sendRequest(data);
}

quint32 UDPTrackerClientSocket::generateTransactionID()
{
	return (transactionID = Utility::rand(Q_INT64_C(0xffffffff)));
}

void UDPTrackerClientSocket::sendRequest(const QByteArray &data)
{
	lastRequest = data;

	writeDatagram(data);
	startTimeoutTimer();
}

void UDPTrackerClientSocket::startTimeoutTimer()
{
	stopTimeoutTimer();
	timeoutTimer = startTimer(15 * (1 << timeoutCounter) * 1000);
}

void UDPTrackerClientSocket::stopTimeoutTimer()
{
	if (timeoutTimer)
	{
		killTimer(timeoutTimer);
		timeoutTimer = 0;
	}
}


void UDPTrackerClientSocket::requestTimeout()
{
	if (++timeoutCounter <= maximalTimeoutCounter)
		sendRequest(lastRequest);
}

void UDPTrackerClientSocket::processIncomingData()
{
	while (hasPendingDatagrams())	processNewDatagram(readDatagram());
}

void UDPTrackerClientSocket::processNewDatagram(const QByteArray &data)
{
	stopTimeoutTimer();
	timeoutCounter = 0;

	// check action
	const RequestAction action = static_cast<RequestAction>(Utility::Endian::fromBigEndian<quint32>(data.constData()));
	if (action != UDPTrackerClient::Error && action != getRequestAction(lastRequest))	return;

	// check transcation id
	if (Utility::Endian::fromBigEndian<quint32>(&data.constData()[4]) != transactionID)	return;
	transactionID = 0;

	if (action == UDPTrackerClient::Connect)
	{
		/*
			Offset	Size		Name		Value
			0	32-bit integer	action		0 // Connect
			4	32-bit integer	transaction_id
			8	64-bit integer	connection_id
			16
		*/
		if (data.size() < 16)	return;
		connectionID = Utility::Endian::fromBigEndian<quint64>(&data.constData()[8]);
		connectionIDStopwatch.restart();

		if (!pendingRequest.isEmpty())
		{
			request(pendingRequest);
			pendingRequest.clear();
		}
	}
	else
		emit requestFinished(data);
}

void UDPTrackerClientSocket::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timeoutTimer)
		requestTimeout();
	else
		Network::UDPSocket::timerEvent(event);
}

}

}
