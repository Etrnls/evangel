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
#ifndef NETWORK_TCPSOCKET_H
#define NETWORK_TCPSOCKET_H

#include "abstractsocket.h"

class QTcpSocket;
class QHostAddress;

namespace Network
{

/*!
 * \brief Provides a TCP socket
 * \author Etrnls <Etrnls@gmail.com>
 */
class TCPSocket : public AbstractSocket
{
	Q_OBJECT
public:
	explicit TCPSocket(Priority priority);
	virtual ~TCPSocket();

	// information
	QHostAddress getPeerAddress() const;
	quint16 getPeerPort() const;
	QString getPeerAddressPort() const;

	virtual State getState() const;
	QString getError() const;

	// connection
	bool setSocketDescriptor(int socketDescriptor);
	void connectToHost(const QHostAddress &address, quint16 port);
	void disconnectFromHost();

	// traffic control
	virtual void setNetworkReadBufferSize(int size);

	virtual bool canDownload() const;
	virtual bool canUpload() const;

	virtual int download(int maxSize = INT_MAX);
	virtual int upload(int maxSize = INT_MAX);

	// read & write
	int bytesAvailable() const;
	QByteArray read(int size);
	void unread(const QByteArray &data, bool triggerDataProcessing = false);

	bool canReadLine() const;
	QByteArray readLine();

	int bytesToWrite() const;
	void write(const QByteArray &data);
signals:
	void connected() const;
	void disconnected() const;
	void error() const;
protected:
	QByteArray incomingBuffer; //!< The incoming buffer
	QByteArray outgoingBuffer; //!< The outgoing buffer
private:
	QTcpSocket *socket;
};

}

#endif
