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
#ifndef PROTOCOL_BITTORRENT_BTCLIENT_H
#define PROTOCOL_BITTORRENT_BTCLIENT_H

#include "bttransfer.h"
#include "peer/peerid.h"

#include "util/bitmap/bitmap.h"
#include "util/block.h"

#include <QObject>
#include <QSet>
#include <QBitArray>
#include <QFutureWatcher>

class QTimerEvent;

namespace Protocol
{

namespace BitTorrent
{

class BTServer;
class MetaInfo;
class PeerWireSocket;
class PeerManager;
class TrackerClientManager;
class Choker;
class BlockSelector;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 *
 * The Official BitTorrent Protocol Specification http://www.bittorrent.org/beps/bep_0003.html
 * Bittorrent Protocol Specification http://wiki.theory.org/BitTorrentSpecification
 * Message Stream Encryption specification http://wiki.vuze.com/w/Message_Stream_Encryption
 *
 * Index of BitTorrent Enhancement Proposals http://www.bittorrent.org/beps/bep_0000.html
 */
class BTClient : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	//! The state of the client
	enum State
	{
		IdleState,
		LeechingState,
		SeedingState
	};

	explicit BTClient(BTTransfer *transfer);
	virtual ~BTClient();

	//! \brief Retrieves the peer ID of the client
	static const PeerID& getPeerID()
	{
		return peerID;
	}

	static const QSet<BTClient*>& getClients()
	{
		return clients;
	}

	static const BTServer *getServer()
	{
		return server;
	}

	//! \brief Retrieves the state of the client
	State getState() const			{	return state;			}

	// downloaded & uploaded bytes
	qint64 getDownloadedBytes() const	{	return downloadedBytes;		}
	qint64 getUploadedBytes() const		{	return uploadedBytes;		}
	void updateDownloadedBytes(int size)	{	downloadedBytes += size;	}
	void updateUploadedBytes(int size)	{	uploadedBytes += size;		}

	// meta info
	const MetaInfo *getMetaInfo() const;

	//! \brief Retrieves the verification status
	const QBitArray &getVerifying() const	{	return verifying;	}
	const QBitArray &getVerified() const	{	return verified;	}

	bool selectBlock(const QBitArray &peer, const Utility::Bitmap &requested, int *index, int *offset, int *size) const;

	void updateRequested(int index, int offset, int size, bool flag);

	QByteArray readBlock(int index, int offset, int size) const;
	void writeBlock(int index, int offset, const QByteArray &data);
signals:
	void stateChanged() const;
private slots:
	void transferBitmapUpdated(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment);
	void updatePieceAvailability(int index, bool flag);

	void pendingVerificationFinished();
private:
	struct VerificationResult
	{
		int index;
		bool result;
		explicit inline VerificationResult(int index = -1, bool result = false) : index(index), result(result) {}
	};

	static const int chokeInterval = 10 * 1000; //!< 10 seconds each round

	void offsetToIndex(qint64 offset, qint64 size, const Utility::Bitmap &bitmap, const Utility::Bitmap::StatusChecker &checker,
				int *beginIndex, int *endIndex) const;

	virtual void timerEvent(QTimerEvent *event);

	Q_INVOKABLE void autoUpdateState();
	inline void setState(State state);

	// algorithms set up
	void setUpChoker();
	void setUpSelector();

	// verification
	void autoVerify(qint64 offset, qint64 size);
	static VerificationResult verify(int index, const QByteArray &data, const QByteArray &expected);

	static PeerID peerID;
	static QSet<BTClient*> clients;
	static BTServer *server;

	State state;

	int chokeTimer;

	qint64 downloadedBytes;
	qint64 uploadedBytes;

	BTTransfer *transfer;

	// algorithms
	Choker *choker;
	BlockSelector *selector;

	// managers
	PeerManager *peerManager;
	TrackerClientManager *trackerClientManager;

	// verification status
	QBitArray verifying, verified;

	QSet<QFutureWatcher<VerificationResult>*> pendingVerifications;
};

}

}

#endif
