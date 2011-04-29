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
#include "btserver.h"
#include "btclient.h"
#include "util/metainfo.h"
#include "peer/peerwiresocket.h"

#include "log.h"
#include "util/settings.h"

namespace Protocol
{

namespace BitTorrent
{

BTServer::~BTServer()
{
	foreach (PeerWireSocket *socket, sockets)
	{
		disconnect(socket, 0, this, 0);
		delete socket;
	}
}

/*!
 * \brief Sets up the server
 * \return Whether the server is successfully set up.
 */
bool BTServer::setUp()
{
	if (isListening()) return true;

	const quint16 portMin = Utility::Settings::load(this, QLatin1String("serverPortMin"), defaultPortMin).value<quint16>();
	const quint16 portMax = Utility::Settings::load(this, QLatin1String("serverPortMax"), defaultPortMax).value<quint16>();

	for (quint16 port = portMin; port <= portMax; ++port)
		if (listen(QHostAddress::Any, port))
		{
			log(this) << "Server listen on port" << port << endl;
			return true;
		}

	log(this) << QString::fromLatin1("Server failed to listen on any port [%1-%2]").arg(portMin).arg(portMax) << endl;
	return false;
}

void BTServer::authorize(const QByteArray &infoHash)
{
	PeerWireSocket *socket = qobject_cast<PeerWireSocket*>(sender());
	Q_ASSERT_X(sockets.contains(socket), Q_FUNC_INFO, "Invalid socket");

	log(this) << "Authorize incoming connection" << socket->getPeerAddressPort() << endl;
	emit incomingConnection(infoHash, socket);

	// not accepted by any client
	if (sockets.contains(socket))	socket->disconnectFromHost();
}

void BTServer::socketDisconnected()
{
	PeerWireSocket *socket = qobject_cast<PeerWireSocket*>(sender());
	delSocket(socket);
	socket->deleteLater();
}

void BTServer::addSocket(PeerWireSocket *socket)
{
	sockets.insert(socket);
	connect(socket, SIGNAL(infoHashReceived(QByteArray)), SLOT(authorize(QByteArray)));
	connect(socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
}

void BTServer::delSocket(PeerWireSocket *socket)
{
	Q_ASSERT_X(sockets.contains(socket), Q_FUNC_INFO, "Invalid socket");

	sockets.remove(socket);
	disconnect(socket, 0, this, 0);
}

void BTServer::incomingConnection(int socketDescriptor)
{
	PeerWireSocket *socket = new PeerWireSocket(PeerWireSocket::NoneEncryption, PeerWireSocket::IncomingConnection);

	if (socket->setSocketDescriptor(socketDescriptor))
		addSocket(socket);
	else
		delete socket;
}

}

}
