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
#ifndef PROTOCOL_BITTORRENT_UDPTRACKERCLIENTSOCKET_H
#define PROTOCOL_BITTORRENT_UDPTRACKERCLIENTSOCKET_H

#include "udptrackerclient.h"

#include "protocol/bt/tracker/trackerclientsocket.h"

#include "network/udpsocket.h"

#include <QElapsedTimer>

class QUrl;

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 * \todo authentication support
 */
class UDPTrackerClientSocket : public Network::UDPSocket, public TrackerClientSocket
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	explicit UDPTrackerClientSocket(const QUrl &URL);

	virtual bool hasPendingRequest() const
	{
		return transactionID != 0;
	}

	virtual void cancelPendingRequest()
	{
		transactionID = 0;
		pendingRequest.clear();
	}

	virtual void request(const QByteArray &data);
signals:
	void requestFinished(const QByteArray &data) const;
private:
	typedef UDPTrackerClient::RequestAction RequestAction;

	static const int maximalTimeoutCounter = 8;

	static RequestAction getRequestAction(const QByteArray &data);

	bool checkConnectionID() const;
	void requestConnectionID();

	quint32 generateTransactionID();

	void sendRequest(const QByteArray &data);

	void startTimeoutTimer();
	void stopTimeoutTimer();

	void requestTimeout();

	virtual void processIncomingData();
	void processNewDatagram(const QByteArray &data);

	virtual void timerEvent(QTimerEvent *event);

	QByteArray lastRequest;
	QByteArray pendingRequest;

	quint64 connectionID;
	QElapsedTimer connectionIDStopwatch;

	quint32 transactionID;

	int timeoutTimer;
	int timeoutCounter;
};

}

}

#endif
