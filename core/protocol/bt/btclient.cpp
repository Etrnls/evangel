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
#include "btclient.h"
#include "btserver.h"
#include "util/metainfo.h"
#include "tracker/trackerclientmanager.h"
#include "peer/peerwiresocket.h"
#include "peer/peer.h"
#include "peer/peermanager.h"
#include "algorithm/leechingchoker.h"
#include "algorithm/seedingchoker.h"
#include "algorithm/lrfselector.h"

#include "log.h"
#include "future/job.h"
#include "future/jobmanager.h"
#include "util/settings.h"
#include "util/hash/hash.h"

#include <QTimerEvent>
#include <QUrl>
#include <QtConcurrentRun>

#include <algorithm>
#include <functional>


namespace Protocol
{

namespace BitTorrent
{

PeerID BTClient::peerID;
QSet<BTClient*> BTClient::clients;
BTServer *BTClient::server = 0;


BTClient::BTClient(BTTransfer *transfer)
		: state(IdleState), chokeTimer(0), downloadedBytes(0), uploadedBytes(0), transfer(transfer),
		choker(0), selector(0), peerManager(new PeerManager(this)), trackerClientManager(new TrackerClientManager(this))
{
	if (peerID.getID().isEmpty())	peerID.generate();

	// make sure the server is set up before any tracker request performs
	// because the server port is needed in the tracker request
	if (!server)	(server = new BTServer())->setUp();

	// set up verification status size
	verifying.resize(getMetaInfo()->getPieces());
	verified.resize(getMetaInfo()->getPieces());

	// bitmap update information
	connect(transfer, SIGNAL(bitmapUpdated(Utility::Bitmap::SegmentStatus, Utility::Bitmap::Segment)),
		this, SLOT(transferBitmapUpdated(Utility::Bitmap::SegmentStatus, Utility::Bitmap::Segment)));

	// availability update information
	connect(peerManager, SIGNAL(pieceAvailabilityUpdated(int, bool)),
		this, SLOT(updatePieceAvailability(int, bool)));

	// trackerClientManager <---> peerManager
	connect(trackerClientManager, SIGNAL(peerListUpdated(QList<Peer*>)), peerManager, SLOT(addPeers(QList<Peer*>)));
	connect(peerManager, SIGNAL(needMorePeers()), trackerClientManager, SLOT(request()));

	// server <---> peerManager
	connect(server, SIGNAL(incomingConnection(QByteArray, PeerWireSocket*)), peerManager, SLOT(addPeer(QByteArray, PeerWireSocket*)));
	connect(peerManager, SIGNAL(incomingConnectionAccepted(PeerWireSocket*)), server, SLOT(delSocket(PeerWireSocket*)));

	// trackers
	foreach(const QString &URL, getMetaInfo()->getAnnounceList())
		trackerClientManager->addTracker(URL);

	// algorithms
	setUpSelector();

	clients.insert(this);
}

BTClient::~BTClient()
{
	foreach (auto watcher, pendingVerifications)
		Future::JobManager::getInstance()->attach(new Future::Job(watcher, SIGNAL(finished())));

	// managers
	delete trackerClientManager;
	delete peerManager;

	// algorithms
	delete choker;
	delete selector;

	clients.remove(this);
	if (clients.isEmpty())
	{
		delete server;
		server = 0;
	}
}



//! \brief Retrieves the meta info
const MetaInfo *BTClient::getMetaInfo() const
{
	return transfer->getMetaInfo();
}

bool BTClient::selectBlock(const QBitArray &peer, const Utility::Bitmap &requested,
				int *index, int *offset, int *size) const
{
	Utility::Bitmap self = transfer->bitmap();

	foreach (const auto &segment, requested.segments(Utility::Bitmap::getStatusChecker(Utility::Bitmap::RequestedStatus)))
		self.updateStatus(Utility::Bitmap::getStatusUpdater(Utility::Bitmap::RequestedStatus), segment);

	return selector->select(self, peer, getMetaInfo(), index, offset, size);
}

void BTClient::updateRequested(int index, int offset, int size, bool flag)
{
	transfer->updateBlock(flag ? Utility::BlockRequested : Utility::BlockCanceled,
			      Utility::Block(getMetaInfo()->getBeginningOffset(index) + offset, size));
}

QByteArray BTClient::readBlock(int index, int offset, int size) const
{
	Q_ASSERT_X(index >= 0 && index < getMetaInfo()->getPieces(), Q_FUNC_INFO, "invalid index");
	Q_ASSERT_X(offset >= 0 && offset < getMetaInfo()->getPieceSize(index), Q_FUNC_INFO, "invalid offset");
	Q_ASSERT_X(size > 0 && size <= getMetaInfo()->getPieceSize(index), Q_FUNC_INFO, "invalid size");
	Q_ASSERT_X(offset + size <= getMetaInfo()->getPieceSize(index), Q_FUNC_INFO, "invalid offset and size");

	return transfer->readBlock(Utility::Block(getMetaInfo()->getBeginningOffset(index) + offset, size));
}

void BTClient::writeBlock(int index, int offset, const QByteArray &data)
{
	transfer->updateBlock(Utility::BlockDownloaded, Utility::Block(getMetaInfo()->getBeginningOffset(index) + offset, data));
}

void BTClient::transferBitmapUpdated(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
{
	switch (status)
	{
	case Utility::Bitmap::EmptyStatus:
		autoUpdateState();
		peerManager->broadcastRequest();
		break;
	case Utility::Bitmap::MaskedStatus:
		autoUpdateState();
		break;
	case Utility::Bitmap::DownloadedStatus:
		autoVerify(segment.begin, segment.size());
		break;
	case Utility::Bitmap::VerifiedStatus:
	{
		const Utility::Bitmap &bitmap = transfer->bitmap();
		const auto &checker = Utility::Bitmap::getStatusChecker(Utility::Bitmap::VerifiedStatus);
		int beginIndex, endIndex;
		offsetToIndex(segment.begin, segment.size(), bitmap, checker, &beginIndex, &endIndex);
		
		if (beginIndex < endIndex)
		{
			verified.fill(true, beginIndex, endIndex);
			selector->updateVerified(beginIndex, endIndex);

			for (int i = beginIndex; i < endIndex; ++i)
				peerManager->broadcastHave(i);

			autoUpdateState();
		}
	}
		break;
	default:
		break;
	};
}

void BTClient::updatePieceAvailability(int index, bool flag)
{
	selector->updateAvailability(index, flag ? 1 : -1);
}

void BTClient::pendingVerificationFinished()
{
	QFutureWatcher<VerificationResult> *watcher = dynamic_cast<QFutureWatcher<VerificationResult>*>(sender());

	const int index = watcher->result().index;
	const bool result = watcher->result().result;

	log(this) << QString::fromLatin1("Piece[%1] verification result = %2").arg(index).arg(result) << endl;

	transfer->updateBlock(result ? Utility::BlockVerified : Utility::BlockCorrupted,
			Utility::Block(getMetaInfo()->getBeginningOffset(index), getMetaInfo()->getPieceSize(index)));

	verifying.setBit(index, false);

	pendingVerifications.remove(watcher);
	// Safely delete the watcher
	// signal from watcher-->here
	watcher->deleteLater();
}

void BTClient::offsetToIndex(qint64 offset, qint64 size, const Utility::Bitmap &bitmap, const Utility::Bitmap::StatusChecker &checker,
				int *beginIndex, int *endIndex) const
{
	*beginIndex = getMetaInfo()->getIndex(offset);
	*endIndex = getMetaInfo()->getIndex(offset + size - 1) + 1;

	// adjust begin index
	{
		const qint64 begin = getMetaInfo()->getBeginningOffset(*beginIndex);
		const qint64 end = offset;
		if (begin < end && !bitmap.checkStatus(checker, Utility::Bitmap::Segment(begin, end)))	++*beginIndex;
	}

	// adjust end index
	{
		const qint64 begin = offset + size - 1;
		const qint64 end = getMetaInfo()->getEndingOffset(*endIndex - 1);
		if (begin < end && !bitmap.checkStatus(checker, Utility::Bitmap::Segment(begin, end)))	--*endIndex;
	}
}

void BTClient::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == chokeTimer)
	{
		Q_ASSERT_X(choker, Q_FUNC_INFO, "Invalid choker");
		choker->choke(peerManager);
	}
	else
		QObject::timerEvent(event);
}

void BTClient::autoUpdateState()
{
	const Utility::Bitmap &bitmap = transfer->bitmap();
	const Utility::Bitmap::Segment segment(0, getMetaInfo()->getSize());
	if (bitmap.checkStatus(Utility::Bitmap::getStatusChecker(Utility::Bitmap::MaskedStatus), segment))
		setState(IdleState);
	else if (bitmap.checkStatus([](Utility::Bitmap::SegmentStatus status)
	{
		return Utility::Bitmap::getStatusChecker(Utility::Bitmap::MaskedStatus)(status) ||
			Utility::Bitmap::getStatusChecker(Utility::Bitmap::VerifiedStatus)(status);
	}, segment))
		setState(SeedingState);
	else
		setState(LeechingState);
}

inline void BTClient::setState(State state)
{
	if (this->state == state)	return;
	this->state = state;

	setUpChoker();

	emit stateChanged();
}

/*!
 * \brief Sets up the choker
 * \note If no choker is specified or the choker specified is invalid, SimpleChokeAlgorithm is used by default.
 */
void BTClient::setUpChoker()
{
	if (choker)	delete	choker;

	switch (state)
	{
	case IdleState:
		// no choker needed
		choker = 0;
		return;
	case LeechingState:
		choker = new LeechingChoker();
		break;
	case SeedingState:
		choker = new SeedingChoker();
		break;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid state");
		break;
	}

	if (chokeTimer)	killTimer(chokeTimer);
	chokeTimer = startTimer(chokeInterval);
}

void BTClient::setUpSelector()
{
	switch (Utility::Settings::load(this, QLatin1String("selector"), 0).toInt())
	{
	case 0:
	default:
		selector = new LRFSelector(getMetaInfo());
		break;
	}
	log(this) << QString::fromLatin1("%1 selector set up for torrent [%2]").arg(selector->name())
		.arg(QLatin1String(getMetaInfo()->getInfoHash().toHex().constData())) << endl;
}


/*!
 * \brief Does the verification when a block has been downloaded
 * \param offset The offset of the block.
 * \param size The size of the block.
 *
 * This function does the verification based on whether a whole piece has been downloaded
 * after we received a new block.
 *
 */
void BTClient::autoVerify(qint64 offset, qint64 size)
{
	const Utility::Bitmap &bitmap = transfer->bitmap();
	const auto &checker = Utility::Bitmap::getStatusChecker(Utility::Bitmap::DownloadedStatus);
	int beginIndex, endIndex;
	offsetToIndex(offset, size, bitmap, checker, &beginIndex, &endIndex);

	for (int index = beginIndex; index < endIndex; ++index)
	{
		if (verified.testBit(index))	continue;

		if (!verifying.testBit(index))
		{
			verifying.setBit(index);
			QFutureWatcher<VerificationResult> *watcher = new QFutureWatcher<VerificationResult>;
			connect(watcher, SIGNAL(finished()), SLOT(pendingVerificationFinished()), Qt::QueuedConnection);
			pendingVerifications.insert(watcher);

			watcher->setFuture(QtConcurrent::run(&BTClient::verify, index,
						readBlock(index, 0, getMetaInfo()->getPieceSize(index)),
						getMetaInfo()->getSHA1Hash()[index]));
		}
	}
}

BTClient::VerificationResult BTClient::verify(int index, const QByteArray &data, const QByteArray &expected)
{
	return VerificationResult(index, Utility::Cryptography::Hash::hash(data, Utility::Cryptography::Hash::SHA1) == expected);
}


}

}

