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
#ifndef NETWORK_UDPSOCKET_H
#define NETWORK_UDPSOCKET_H

#include "abstractsocket.h"

#include <QQueue>

class QUdpSocket;
class QHostAddress;

namespace Network
{

/*!
 * \brief Provides a UDP socket
 * \author Etrnls <Etrnls@gmail.com>
 */
class UDPSocket : public AbstractSocket
{
	Q_OBJECT
public:
	explicit UDPSocket(Priority priority);
	virtual ~UDPSocket();

	virtual State getState() const;

	// connection
	bool setSocketDescriptor(int socketDescriptor);
	void connectToHost(const QHostAddress &address, quint16 port);

	// traffic control
	virtual void setNetworkReadBufferSize(int size);

	virtual bool canDownload() const;
	virtual bool canUpload() const;

	virtual int download(int maxSize = INT_MAX);
	virtual int upload(int maxSize = INT_MAX);

	// read & write
	int bytesAvailable() const;
	bool hasPendingDatagrams () const;
	QByteArray readDatagram();

	int bytesToWrite() const;
	void writeDatagram(const QByteArray &data);
protected:
	QQueue<QByteArray> incomingBuffer; //!< The incoming buffer
	QQueue<QByteArray> outgoingBuffer; //!< The outgoing buffer

	int incomingBufferSize; //!< The total size of incoming buffer
	int outgoingBufferSize; //!< The total size of outgoing buffer
private:
	QUdpSocket *socket;
};

}

#endif
