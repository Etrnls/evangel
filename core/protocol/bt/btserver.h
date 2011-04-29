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
#ifndef PROTOCOL_BITTORRENT_BTSERVER_H
#define PROTOCOL_BITTORRENT_BTSERVER_H

#include "network/tcpserver.h"

namespace Protocol
{

namespace BitTorrent
{

class PeerWireSocket;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class BTServer : public Network::TCPServer
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	inline BTServer() {};
	virtual ~BTServer();

	bool setUp();
signals:
	void incomingConnection(const QByteArray &infoHash, PeerWireSocket *socket) const;
private slots:
	void authorize(const QByteArray &infoHash);

	void socketDisconnected();

	void addSocket(PeerWireSocket *socket);
	void delSocket(PeerWireSocket *socket);
private:
	static const quint16 defaultPortMin = 51681;
	static const quint16 defaultPortMax = 51781;

	virtual void incomingConnection(int socketDescriptor);


	QSet<PeerWireSocket*> sockets;
};

}

}

#endif
