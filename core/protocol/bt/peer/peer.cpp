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
#include "peer.h"
#include "peerwiresocket.h"

#include "protocol/bt/util/metainfo.h"

#include "log.h"

#include <QTimer>

namespace Protocol
{

namespace BitTorrent
{

//! \brief Creates a Unconnected Peer
Peer::Peer(const QHostAddress &address, quint16 port)
	: address(address), port(port), socket(0),
	state(UnconnectedState), suspended(false)
{
}

//! \brief Creates a Connecting Peer
Peer::Peer(PeerWireSocket *socket)
	: address(socket->getPeerAddress()), port(socket->getPeerPort()), socket(0),
	state(UnconnectedState), suspended(false)
{
	setSocket(socket);
	setState(ConnectingState);
}

Peer::~Peer()
{
	delete socket;
}

//! \brief Connects to the peer
void Peer::connectToPeer(BTClient *client)
{
	Q_ASSERT_X(state == UnconnectedState, Q_FUNC_INFO, "Not in Unconnected state");

	if (!suspended)
	{
		log(this) << QString::fromLatin1("Connecting to peer [%1]").arg(getAddressPort()) << endl;

		PeerWireSocket *socket = new PeerWireSocket(PeerWireSocket::MessageStreamEncryption, PeerWireSocket::OutgoingConnection);
		socket->setClient(client);

		setSocket(socket);
		connectSocket();
	}
}

void Peer::disconnectFromPeer()
{
	Q_ASSERT_X(state != UnconnectedState, Q_FUNC_INFO, "Not connected yet");
	disconnectSocket();
}

const PeerID &Peer::getPeerID() const
{
	return peerID;
}

void Peer::setPeerID(const QByteArray &ID)
{
	if (ID.isEmpty())	return;
	peerID.setID(ID);

	log(this) << QString::fromLatin1("Client name (from peer ID) of peer [%1] : '%2'").arg(getAddressPort()).arg(peerID.getClientName()) << endl;
}

qreal Peer::getDownloadRate() const
{
	return socket ? socket->getDownloadRate() : 0;
}

qreal Peer::getUploadRate() const
{
	return socket ? socket->getUploadRate() : 0;
}

bool Peer::isSeeder() const
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	return socket->getPeerState().testFlag(PeerWireSocket::SeedingState);
}

bool Peer::isInterested() const
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	return socket->getSelfState().testFlag(PeerWireSocket::InterestedState);
}

void Peer::request()
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	socket->scheduleRequest();
}

void Peer::have(int index)
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	socket->sendHave(index);
}

void Peer::choke()
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	socket->sendChoke();
}

void Peer::unchoke()
{
	Q_ASSERT_X(state == ConnectedState, Q_FUNC_INFO, "Not connected yet");
	socket->sendUnchoke();
}

void Peer::socketConnected()
{
	log(this) << QString::fromLatin1("Peer [%1] connected").arg(getAddressPort()) << endl;

	disconnect(socket, SIGNAL(connectedToPeer()), this, SLOT(socketConnected()));

	setState(ConnectedState);
}

void Peer::socketDisconnected()
{
	log(this) << QString::fromLatin1("Peer [%1] disconnected").arg(getAddressPort()) << endl;

	disconnect(socket, SIGNAL(disconnectedFromPeer()), this, SLOT(socketDisconnected()));

	setState(UnconnectedState);
}

void Peer::processPeerID(const QByteArray &ID)
{
	setPeerID(ID);
	emit peerIDReceived();
}

void Peer::processClientName(const QString &name)
{
	if (name.isEmpty())	return;
	peerID.setClientName(name);

	log(this) << QString::fromLatin1("Client name of peer [%1] : '%2'").arg(getAddressPort()).arg(peerID.getClientName()) << endl;
}


void Peer::processBlockReceived(int index, int offset, const QByteArray &block)
{
	emit blockReceived(index, offset, block);
}

void Peer::processBlockSent(int index, int offset, int size)
{
	emit blockSent(index, offset, size);
}

void Peer::processDrop(const QString &reason, int seriousness)
{
	log(this) << QString::fromLatin1("Connection to peer [%1] dropped because : %2").arg(getAddressPort()).arg(reason) << endl;
}

void Peer::suspend()
{
	Q_ASSERT_X(!suspended, Q_FUNC_INFO, "Already suspended");

	log(this) << QString::fromLatin1("Peer [%1] suspended").arg(getAddressPort()) << endl;

	suspended = true;
	QTimer::singleShot(minimalConnectionInterval, this, SLOT(wakeUp()));
}

void Peer::wakeUp()
{
	Q_ASSERT_X(suspended, Q_FUNC_INFO, "Not suspended");
	Q_ASSERT_X(state == UnconnectedState, Q_FUNC_INFO, "Not in unconnected state");

	log(this) << QString::fromLatin1("Peer [%1] woke up").arg(getAddressPort()) << endl;

	suspended = false;
	emit readyToConnect();
}

void Peer::connectSocket()
{
	Q_ASSERT_X(state == UnconnectedState, Q_FUNC_INFO, "Not in unconnected state");
	Q_ASSERT_X(socket, Q_FUNC_INFO, "Invalid socket");

	setState(ConnectingState);
	socket->connectToHost(address, port);
}

void Peer::disconnectSocket()
{
	Q_ASSERT_X(state != UnconnectedState, Q_FUNC_INFO, "Not in connected yet");
	Q_ASSERT_X(socket, Q_FUNC_INFO, "Invalid socket");

	socket->disconnectFromHost();
}

//! \brief Sets the socket of the connection with the peer
void Peer::setSocket(PeerWireSocket *socket)
{
	this->socket = socket;

	connect(socket, SIGNAL(connectedToPeer()), SLOT(socketConnected()));
	connect(socket, SIGNAL(disconnectedFromPeer()), SLOT(socketDisconnected()));

	connect(socket, SIGNAL(peerIDReceived(QByteArray)), SLOT(processPeerID(QByteArray)));
	connect(socket, SIGNAL(clientNameReceived(QString)), SLOT(processClientName(QString)));

	connect(socket, SIGNAL(peerBitmapUpdated(int, bool)), SIGNAL(peerBitmapUpdated(int, bool)));

	connect(socket, SIGNAL(blockRequested(int, int, int)), SIGNAL(blockRequested(int, int, int)));
	connect(socket, SIGNAL(blockCanceled(int, int, int)), SIGNAL(blockCanceled(int, int, int)));
	connect(socket, SIGNAL(blockReceived(int, int, QByteArray)), SLOT(processBlockReceived(int, int, QByteArray)));
	connect(socket, SIGNAL(blockSent(int, int, int)), SLOT(processBlockSent(int, int, int)));

	connect(socket, SIGNAL(drop(QString, int)), SLOT(processDrop(QString, int)));
}

void Peer::setState(State state)
{
	if (this->state == state)	return;

	State oldState = this->state;
	this->state = state;

	bool isDead = false;
	switch (state)
	{
	case UnconnectedState:
		suspend();

		// incoming peer is treated as dead
		if (socket->getConnectionType() == PeerWireSocket::IncomingConnection)
			isDead = true;

		// we are not able to connect to it
		if (oldState == ConnectingState)
			isDead = true;

		// Safely delete the socket
		// signal from socket-->Peer::socketDisconnected-->here
		disconnect(socket, 0, this, 0);
		socket->deleteLater();
		socket = 0;
		break;
	default:
		break;
	}

	emit stateChanged();

	if (isDead)	emit dead();
}

}

}
