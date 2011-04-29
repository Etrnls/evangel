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
#include "tcpsocket.h"
#include "trafficmanager.h"

#include <QTcpSocket>
#include <QEvent>
#include <QTimer>
#include <QHostAddress>
#include <QCoreApplication>

namespace Network
{

TCPSocket::TCPSocket(Priority priority) : AbstractSocket(priority), socket(new QTcpSocket())
{
	connect(socket, SIGNAL(readyRead()), SIGNAL(readyDownload()));
	connect(socket, SIGNAL(connected()), SIGNAL(readyUpload()));

	connect(socket, SIGNAL(connected()), SIGNAL(connected()));
	connect(socket, SIGNAL(disconnected()), SIGNAL(disconnected()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(error()));

	attachTrafficControl();
}

TCPSocket::~TCPSocket()
{
	detachTrafficControl();

	delete socket;
}

QHostAddress TCPSocket::getPeerAddress() const
{
	return socket->peerAddress();
}

quint16 TCPSocket::getPeerPort() const
{
	return socket->peerPort();
}

QString TCPSocket::getPeerAddressPort() const
{
	return QString::fromLatin1("%1:%2").arg(getPeerAddress().toString()).arg(getPeerPort());
}

TCPSocket::State TCPSocket::getState() const
{
	switch (socket->state())
	{
	case QTcpSocket::UnconnectedState:
		return UnconnectedState;
	case QTcpSocket::HostLookupState:
	case QTcpSocket::ConnectingState:
		return ConnectingState;
	case QTcpSocket::ConnectedState:
		return ConnectedState;
	default:
		return UnconnectedState;
	}
}

QString TCPSocket::getError() const
{
	return QString::fromLatin1("Error %1 : %2").arg(socket->error()).arg(socket->errorString());
}

bool TCPSocket::setSocketDescriptor(int socketDescriptor)
{
	return socket->setSocketDescriptor(socketDescriptor, QTcpSocket::ConnectedState,
					QTcpSocket::ReadWrite | QTcpSocket::Unbuffered);
}

void TCPSocket::connectToHost(const QHostAddress &address, quint16 port)
{
	return socket->connectToHost(address, port, QTcpSocket::ReadWrite | QTcpSocket::Unbuffered);
}

void TCPSocket::disconnectFromHost()
{
	socket->disconnectFromHost();
}

void TCPSocket::setNetworkReadBufferSize(int size)
{
	socket->setReadBufferSize(size);
}

bool TCPSocket::canDownload() const
{
	return socket->bytesAvailable() > 0;
}

bool TCPSocket::canUpload() const
{
	return getState() == ConnectedState && !outgoingBuffer.isEmpty();
}

/*!
 * \brief Attempts to download from the network to the buffer
 * \param maxSize Reads up to maxSize bytes.
 * \return The number of bytes read or -1 if an error occurred.
 * \note This function is only (and should only) called by TrafficManager
 */
int TCPSocket::download(int maxSize)
{
	maxSize = qMin<qint64>(maxSize, socket->bytesAvailable());
	if (maxSize == 0)	return 0;

	const int oldSize = incomingBuffer.size();
	incomingBuffer.resize(incomingBuffer.size() + maxSize);

	const int result = socket->read(incomingBuffer.data() + oldSize, maxSize);
	incomingBuffer.resize(result > 0 ? oldSize + result : oldSize);

	if (result > 0)	QMetaObject::invokeMethod(this, "autoProcessIncomingData", Qt::QueuedConnection);

	return result;
}

/*!
 * \brief Attempts to upload to the network from the buffer
 * \param maxSize Writes up to maxSize bytes.
 * \return The number of bytes written or -1 if an error occurred.
 */
int TCPSocket::upload(int maxSize)
{
	maxSize = qMin(maxSize, outgoingBuffer.size());
	if (maxSize == 0)	return 0;

	const int result = socket->write(outgoingBuffer.constData(), maxSize);

	if (result > 0)	outgoingBuffer.remove(0, result);

	return result;
}

int TCPSocket::bytesAvailable() const
{
	return incomingBuffer.size();
}

QByteArray TCPSocket::read(int size)
{
	const int read = qMin(size, bytesAvailable());

	const QByteArray &result = incomingBuffer.left(read);
	incomingBuffer.remove(0, read);

	return result;
}

/*!
 * \brief Unreads the data
 * \note There is no checking of whether the data was read from the socket.
 */
void TCPSocket::unread(const QByteArray &data, bool triggerDataProcessing)
{
	incomingBuffer.prepend(data);
	if (triggerDataProcessing)
		QMetaObject::invokeMethod(this, "autoProcessIncomingData", Qt::QueuedConnection);
}

/*!
 * \brief Retrieves whether a complete line of data can be read
 */
bool TCPSocket::canReadLine() const
{
	return incomingBuffer.contains('\n');
}

QByteArray TCPSocket::readLine()
{
	Q_ASSERT_X(canReadLine(), Q_FUNC_INFO, "Could not read line");

	return read(incomingBuffer.indexOf('\n') + 1);
}

int TCPSocket::bytesToWrite() const
{
	return outgoingBuffer.size() + socket->bytesToWrite();
}

void TCPSocket::write(const QByteArray &data)
{
	outgoingBuffer.append(data);

	if (getState() == ConnectedState)	emit readyUpload();
}

}
