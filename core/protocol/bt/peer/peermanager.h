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
#ifndef PROTOCOL_BITTORRENT_PEERMANAGER_H
#define PROTOCOL_BITTORRENT_PEERMANAGER_H

#include "peer.h"

#include <QObject>
#include <QSet>

namespace Protocol
{

namespace BitTorrent
{

class BTClient;
class PeerWireSocket;

/*!
 * \brief Manages all peers
 * \author Etrnls <Etrnls@gmail.com>
 */
class PeerManager : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	static const int minimalPeersNeeded = 200;

	explicit PeerManager(BTClient *client);
	virtual ~PeerManager();

	void addPeer(Peer *peer);
	void delPeer(Peer *peer);

	bool acceptIncomingConnection() const;
	bool acceptOutgoingConnection() const;

	QList<Peer*> getPeers(Peer::State state) const;
public slots:
	void addPeer(const QByteArray &infoHash, PeerWireSocket *socket);
	void addPeers(const QList<Peer*> &peers);

	// broad cast
	void broadcastRequest() const;
	void broadcastHave(int index) const;
signals:
	void pieceAvailabilityUpdated(int index, bool flag) const;
	void needMorePeers() const;
	void incomingConnectionAccepted(PeerWireSocket *socket) const;
private slots:
	// process
	void processPeerID();
	void processBlockRequested(int index, int offset, int size);
	void processBlockCanceled(int index, int offset, int size);
	void processBlockReceived(int index, int offset, const QByteArray &block);
	void processBlockSent(int index, int offset, int size);

	// client
	void clientStateChanged();

	// peer
	void peerReadyToConnect();
	void peerStateChanged();
	void peerDead();
private:
	static const int maxConnectingConnections = 20;
	static const int maxLeechingConnectedConnections = 200;
	static const int maxSeedingConnectedConnections = 50;
	static const int maxIncomingExceedingConnections = 50;

	int getMaxConnectingConnections() const;
	int getMaxConnectedConnections() const;

	Q_INVOKABLE void connectToPeers();
	void connectToPeer(Peer *peer);

	bool alreadyKnown(Peer *peer) const;

	QSet<Peer*> peers[Peer::stateCount];

	BTClient * const client;
};

}

}

#endif
