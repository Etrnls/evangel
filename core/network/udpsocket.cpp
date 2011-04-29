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
#include "udpsocket.h"

#include <QUdpSocket>

namespace Network
{

UDPSocket::UDPSocket(Priority priority) : AbstractSocket(priority),
		incomingBufferSize(0), outgoingBufferSize(0), socket(new QUdpSocket())
{
	connect(socket, SIGNAL(readyRead()), SIGNAL(readyDownload()));
	connect(socket, SIGNAL(connected()), SIGNAL(readyUpload()));

	attachTrafficControl();
}

UDPSocket::~UDPSocket()
{
	detachTrafficControl();

	delete socket;
}

UDPSocket::State UDPSocket::getState() const
{
	switch (socket->state())
	{
	case QUdpSocket::UnconnectedState:
		return UnconnectedState;
	case QUdpSocket::HostLookupState:
	case QUdpSocket::ConnectingState:
		return ConnectingState;
	case QUdpSocket::ConnectedState:
		return ConnectedState;
	default:
		return UnconnectedState;
	}
}

bool UDPSocket::setSocketDescriptor(int socketDescriptor)
{
	return socket->setSocketDescriptor(socketDescriptor, QUdpSocket::ConnectedState,
					QUdpSocket::ReadWrite | QUdpSocket::Unbuffered);
}

void UDPSocket::connectToHost(const QHostAddress &address, quint16 port)
{
	return socket->connectToHost(address, port, QUdpSocket::ReadWrite | QUdpSocket::Unbuffered);
}

void UDPSocket::setNetworkReadBufferSize(int size)
{
	Q_UNUSED(size);
}

bool UDPSocket::canDownload() const
{
	return getState() == ConnectedState && socket->hasPendingDatagrams();
}

bool UDPSocket::canUpload() const
{
	return getState() == ConnectedState && outgoingBufferSize > 0;
}

int UDPSocket::download(int maxSize)
{
	maxSize = qMin<qint64>(maxSize, socket->bytesAvailable());
	if (maxSize == 0)	return 0;

	int result = 0;

	while (socket->hasPendingDatagrams())
	{
		if (socket->pendingDatagramSize() > maxSize)	break;

		QByteArray datagram(socket->pendingDatagramSize(), Qt::Uninitialized);

		const int read = socket->read(datagram.data(), datagram.size());

		if (read < 0)	break;
		result += read;

		incomingBuffer.enqueue(datagram);
		incomingBufferSize += datagram.size();

		maxSize -= datagram.size();
	}

	if (result > 0)	QMetaObject::invokeMethod(this, "autoProcessIncomingData", Qt::QueuedConnection);

	return result;
}

int UDPSocket::upload(int maxSize)
{
	maxSize = qMin(maxSize, outgoingBufferSize);
	if (maxSize == 0)	return 0;

	int result = 0;

	while (!outgoingBuffer.isEmpty())
	{
		if (outgoingBuffer.head().size() > maxSize)	break;

		const QByteArray &datagram = outgoingBuffer.dequeue();
		outgoingBufferSize -= datagram.size();

		const int written = socket->write(datagram.constData(), datagram.size());

		if (written < 0)	break;
		result += written;

		maxSize -= datagram.size();
	}

	return result;
}

int UDPSocket::bytesAvailable() const
{
	return incomingBufferSize;
}

bool UDPSocket::hasPendingDatagrams () const
{
	return !incomingBuffer.isEmpty();
}

QByteArray UDPSocket::readDatagram()
{
	Q_ASSERT_X(hasPendingDatagrams(), Q_FUNC_INFO, "No pending datagrams");

	const QByteArray &datagram = incomingBuffer.dequeue();
	incomingBufferSize -= datagram.size();

	return datagram;
}

int UDPSocket::bytesToWrite() const
{
	return outgoingBufferSize + socket->bytesToWrite();
}

void UDPSocket::writeDatagram(const QByteArray &data)
{
	outgoingBuffer.enqueue(data);
	outgoingBufferSize += data.size();

	if (getState() == ConnectedState)	emit readyUpload();
}

}
