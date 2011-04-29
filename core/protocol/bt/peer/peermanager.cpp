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
#include "peermanager.h"
#include "peerwiresocket.h"

#include "protocol/bt/btclient.h"
#include "protocol/bt/util/metainfo.h"

#include "log.h"
#include "util/settings.h"
#include "util/numeric/random.h"

namespace Protocol
{

namespace BitTorrent
{

PeerManager::PeerManager(BTClient *client) : client(client)
{
	connect(client, SIGNAL(stateChanged()), SLOT(clientStateChanged()));
}

PeerManager::~PeerManager()
{
	for (int i = 0; i < Peer::stateCount; ++i)
		qDeleteAll(peers[i]);
}

/*!
 * \brief Adds a new peer
 */
void PeerManager::addPeer(Peer *peer)
{
	if (alreadyKnown(peer))
	{
		// Safely delete incoming peer
		// signal from socket-->BTServer::authorize-->BTClient::setUpIncomingConnection-->here
		// delete the peer (thus delete the socket) directly here is not safe
		peer->deleteLater();
		return;
	}

	connect(peer, SIGNAL(readyToConnect()), SLOT(peerReadyToConnect()));
	connect(peer, SIGNAL(stateChanged()), SLOT(peerStateChanged()));
	connect(peer, SIGNAL(dead()), SLOT(peerDead()));

	connect(peer, SIGNAL(peerBitmapUpdated(int, bool)), SIGNAL(pieceAvailabilityUpdated(int, bool)));

	connect(peer, SIGNAL(peerIDReceived()), SLOT(processPeerID()));
	connect(peer, SIGNAL(blockRequested(int, int, int)), SLOT(processBlockRequested(int, int, int)));
	connect(peer, SIGNAL(blockCanceled(int, int, int)), SLOT(processBlockCanceled(int, int, int)));
	connect(peer, SIGNAL(blockReceived(int, int, QByteArray)), SLOT(processBlockReceived(int, int, QByteArray)));
	connect(peer, SIGNAL(blockSent(int, int, int)), SLOT(processBlockSent(int, int, int)));

	Q_ASSERT_X(peer->getState() == Peer::UnconnectedState ||
				peer->getState() == Peer::ConnectingState,
				Q_FUNC_INFO, "Invalid state");
	peers[peer->getState()].insert(peer);

	log(this) << QString::fromLatin1("Added peer [%1] for torrent [%2]").arg(peer->getAddressPort())
			.arg(QLatin1String(client->getMetaInfo()->getInfoHash().toHex().constData())) << endl;
}


void PeerManager::delPeer(Peer *peer)
{
	Q_ASSERT_X(peer->getState() == Peer::UnconnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::UnconnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	peers[Peer::UnconnectedState].remove(peer);

	// Safely delete the peer
	// signal from peer-->PeerManager::peerStateChanged-->here
	disconnect(peer, 0, this, 0);
	peer->deleteLater();

	log(this) << QString::fromLatin1("Deleted peer [%1] for torrent [%2]").arg(peer->getAddressPort())
			.arg(QLatin1String(client->getMetaInfo()->getInfoHash().toHex().constData())) << endl;

	int	peerCount = 0;
	for (int i = 0; i < Peer::stateCount; ++i)
		peerCount += peers[i].size();
	if (peerCount < minimalPeersNeeded)	emit needMorePeers();
}

bool PeerManager::acceptIncomingConnection() const
{
	return peers[Peer::ConnectedState].size() < getMaxConnectedConnections() + maxIncomingExceedingConnections;
}

bool PeerManager::acceptOutgoingConnection() const
{
	return peers[Peer::ConnectingState].size() < getMaxConnectingConnections() &&
		peers[Peer::ConnectedState].size() < getMaxConnectedConnections();
}

QList<Peer*> PeerManager::getPeers(Peer::State state) const
{
	return peers[state].toList();
}

void PeerManager::addPeer(const QByteArray &infoHash, PeerWireSocket *socket)
{
	if (client->getMetaInfo()->getInfoHash() != infoHash)	return;

	if (acceptIncomingConnection())
	{
		socket->setClient(client);
		addPeer(new Peer(socket));
		emit incomingConnectionAccepted(socket);
	}
	else
		socket->deleteLater();
}

/*!
 * \brief Adds a list of new peers
 */
void PeerManager::addPeers(const QList<Peer*> &peers)
{
	foreach (Peer *peer, peers)	addPeer(peer);

	QMetaObject::invokeMethod(this, "connectToPeers", Qt::QueuedConnection);
}

void PeerManager::broadcastRequest() const
{
	foreach (Peer *peer, peers[Peer::ConnectedState])
		peer->request();
}

void PeerManager::broadcastHave(int index) const
{
	foreach (Peer *peer, peers[Peer::ConnectedState])
		peer->have(index);
}

void PeerManager::processPeerID()
{
	Peer *senderPeer = qobject_cast<Peer*>(sender());
	Q_ASSERT_X(senderPeer->getState() == Peer::ConnectingState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::ConnectingState].contains(senderPeer), Q_FUNC_INFO, "Invalid peer");

	foreach (Peer *peer, peers[Peer::ConnectedState])
	{
		if (peer == senderPeer)	continue;
		if (peer->getPeerID().getID() == senderPeer->getPeerID().getID())
		{
			// already connected via another connection
			senderPeer->disconnectFromPeer();
			break;
		}
	}
}

void PeerManager::processBlockRequested(int index, int offset, int size)
{
	Peer *peer = qobject_cast<Peer*>(sender());	Q_UNUSED(peer);
	Q_ASSERT_X(peer->getState() == Peer::ConnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::ConnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	client->updateRequested(index, offset, size, true);
}

void PeerManager::processBlockCanceled(int index, int offset, int size)
{
	Peer *peer = qobject_cast<Peer*>(sender());	Q_UNUSED(peer);
	Q_ASSERT_X(peer->getState() == Peer::ConnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::ConnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	client->updateRequested(index, offset, size, false);
}

void PeerManager::processBlockReceived(int index, int offset, const QByteArray &block)
{
	Peer *peer = qobject_cast<Peer*>(sender());	Q_UNUSED(peer);
	Q_ASSERT_X(peer->getState() == Peer::ConnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::ConnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	client->writeBlock(index, offset, block);
	client->updateDownloadedBytes(block.size());
}

void PeerManager::processBlockSent(int index, int offset, int size)
{
	Peer *peer = qobject_cast<Peer*>(sender());	Q_UNUSED(peer);
	Q_ASSERT_X(peer->getState() == Peer::ConnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::ConnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	Q_UNUSED(index);	Q_UNUSED(offset);
	client->updateUploadedBytes(size);
}

void PeerManager::clientStateChanged()
{
	switch (client->getState())
	{
	case BTClient::LeechingState:
		broadcastRequest();
		break;
	case BTClient::SeedingState:
		foreach (Peer *peer, peers[Peer::ConnectedState])
			if (peer->isSeeder())	peer->disconnectFromPeer();
		break;
	default:
		break;
	}
}

void PeerManager::peerReadyToConnect()
{
	Peer *peer = qobject_cast<Peer*>(sender());
	Q_ASSERT_X(peer->getState() == Peer::UnconnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::UnconnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");

	if (acceptOutgoingConnection())	connectToPeer(peer);
}

void PeerManager::peerStateChanged()
{
	Peer *peer = qobject_cast<Peer*>(sender());

	for (int i = 0; i < Peer::stateCount; ++i)
		if (peers[i].contains(peer))
		{
			peers[i].remove(peer);
			peers[peer->getState()].insert(peer);
			break;
		}

	switch (peer->getState())
	{
	case Peer::UnconnectedState:
		QMetaObject::invokeMethod(this, "connectToPeers", Qt::QueuedConnection);
		break;
	default:
		break;
	}
}

void PeerManager::peerDead()
{
	Peer *peer = qobject_cast<Peer*>(sender());
	delPeer(peer);
}

int PeerManager::getMaxConnectedConnections() const
{
	if (client->getState() == BTClient::SeedingState)
		return maxSeedingConnectedConnections;
	else
		return maxLeechingConnectedConnections;
}

int PeerManager::getMaxConnectingConnections() const
{
	return maxConnectingConnections;
}

void PeerManager::connectToPeers()
{
	if (!acceptOutgoingConnection())	return;

	QList<Peer*>	unconnectedPeers = peers[Peer::UnconnectedState].toList();

	while (!unconnectedPeers.isEmpty() && acceptOutgoingConnection())
		connectToPeer(unconnectedPeers.takeAt(Utility::rand(unconnectedPeers.size())));
}

/*!
 * \brief Sets up the connection
 */
void PeerManager::connectToPeer(Peer *peer)
{
	Q_ASSERT_X(peer->getState() == Peer::UnconnectedState, Q_FUNC_INFO, "Invalid state");
	Q_ASSERT_X(peers[Peer::UnconnectedState].contains(peer), Q_FUNC_INFO, "Invalid peer");
	Q_ASSERT_X(acceptOutgoingConnection(), Q_FUNC_INFO, "Trying to connect to peer while should not");

	peer->connectToPeer(client);
}


/*!
 * \brief Checks whether the newPeer is already known by the PeerManager
 */
bool PeerManager::alreadyKnown(Peer *newPeer) const
{
	for (int i = 0; i < Peer::stateCount; ++i)
		foreach (Peer *peer, peers[i])
			if (*peer == *newPeer)	return true;

	return false;
}


}

}
