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
#ifndef PROTOCOL_BITTORRENT_PEER_H
#define PROTOCOL_BITTORRENT_PEER_H

#include "peerid.h"

#include <QObject>
#include <QHostAddress>

namespace Protocol
{

namespace BitTorrent
{

class BTClient;
class PeerWireSocket;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class Peer : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	enum State
	{
		UnconnectedState = 0,
		ConnectingState,
		ConnectedState,
		InvalidState
	};
	static const int stateCount = InvalidState;

	Peer(const QHostAddress &address, quint16 port);
	explicit Peer(PeerWireSocket *socket);
	virtual ~Peer();
	
	// connection
	void connectToPeer(BTClient *client);
	void disconnectFromPeer();

	// peer address & port
	QHostAddress getAddress() const	{	return address;		}
	quint16 getPort() const		{	return port;		}

	QString getAddressPort() const
	{
		return address.toString() + QLatin1Char(':') + QString::number(port);
	}

	// peer state
	State getState() const	{	return state;	}

	// peer ID
	const PeerID &getPeerID() const;
	void setPeerID(const QByteArray &ID);

	// download & upload rate
	qreal getDownloadRate() const;
	qreal getUploadRate() const;

	// protocol
	bool isSeeder() const;
	bool isInterested() const;

	void request();
	void have(int index);

	void choke();
	void unchoke();
signals:
	void readyToConnect() const;
	void stateChanged() const;
	void dead() const;

	void peerBitmapUpdated(int index, bool flag) const;

	void peerIDReceived() const;
	void blockRequested(int index, int offset, int size) const;
	void blockCanceled(int index, int offset, int size) const;
	void blockReceived(int index, int offset, const QByteArray &block) const;
	void blockSent(int index, int offset, int size) const;
private slots:
	void socketConnected();
	void socketDisconnected();

	void processPeerID(const QByteArray &ID);
	void processClientName(const QString &name);

	void processBlockReceived(int index, int offset, const QByteArray &block);
	void processBlockSent(int index, int offset, int size);

	void processDrop(const QString &reason, int seriousness);

	void suspend();
	void wakeUp();

	void connectSocket();
	void disconnectSocket();
private:
	static const int minimalConnectionInterval = 60 * 1000; // 1 minute

	void setSocket(PeerWireSocket *socket);

	void setState(State state);

	const QHostAddress address;
	const quint16 port;

	PeerID peerID;

	PeerWireSocket *socket;

	State state;
	bool suspended; //!< The peer is suspended just after it is disconnected

	friend bool operator==(const Peer &lhs, const Peer &rhs);
};

inline bool operator==(const Peer &lhs, const Peer &rhs)
{
	return lhs.address == rhs.address && lhs.port == rhs.port;
}

}

}

#endif
