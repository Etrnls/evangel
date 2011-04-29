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
#include "trackerclientmanager.h"
#include "trackerclient.h"

#include "protocol/bt/btclient.h"
#include "protocol/bt/util/metainfo.h"

#include "log.h"

namespace Protocol
{

namespace BitTorrent
{

TrackerClientManager::TrackerClientManager(BTClient *client) : client(client)
{
	connect(client, SIGNAL(stateChanged()), SLOT(clientStateChanged()));
}

TrackerClientManager::~TrackerClientManager()
{
	qDeleteAll(trackerClients);
}

void TrackerClientManager::addTracker(const QUrl &URL)
{
	foreach (TrackerClient *trackerClient, trackerClients)
		if (trackerClient->getURL() == URL)	return;

	TrackerClient *trackerClient = TrackerClient::createTrackerClient(URL, this);
	if (!trackerClient)
	{
		log(this) << QString::fromLatin1("Invalid tracker URL [%1] for torrent [%2]").arg(URL.toString())
			.arg(QLatin1String(getInfoHash().toHex().constData())) << endl;
		return;
	}

	connect(trackerClient, SIGNAL(peerListUpdated(QList<Peer*>)), SIGNAL(peerListUpdated(QList<Peer*>)));
	trackerClients.append(trackerClient);

	log(this) << QString::fromLatin1("Added tracker [%1] for torrent [%2]").arg(trackerClient->getURL().toString())
		.arg(QLatin1String(getInfoHash().toHex().constData())) << endl;
}

void TrackerClientManager::delTracker(TrackerClient *trackerClient)
{
	trackerClients.removeOne(trackerClient);
	delete trackerClient;

	log(this) << QString::fromLatin1("Deleted tracker [%1] for torrent [%2]").arg(trackerClient->getURL().toString())
		.arg(QLatin1String(getInfoHash().toHex().constData())) << endl;
}

QByteArray TrackerClientManager::getInfoHash() const
{
	return client->getMetaInfo()->getInfoHash();
}

qint64 TrackerClientManager::getUploaded() const
{
	return client->getUploadedBytes();
}

qint64 TrackerClientManager::getDownloaded() const
{
	return client->getDownloadedBytes();
}

qint64 TrackerClientManager::getLeft() const
{
	qint64 verifiedSize = client->getMetaInfo()->getPieceSize(-1) * static_cast<qint64>(client->getVerified().count(true));

	if (client->getVerified().testBit(client->getMetaInfo()->getPieces() - 1)) // last piece
	{
		verifiedSize -= client->getMetaInfo()->getPieceSize(-1);
		verifiedSize += client->getMetaInfo()->getPieceSize(client->getMetaInfo()->getPieces() - 1);
	}

	return client->getMetaInfo()->getSize() - verifiedSize;
}

void TrackerClientManager::request() const
{
	foreach (TrackerClient *trackerClient, trackerClients)
		trackerClient->request();
}

void TrackerClientManager::clientStateChanged() const
{
	switch (client->getState())
	{
	case BTClient::IdleState:
		foreach (TrackerClient *trackerClient, trackerClients)
			trackerClient->stop();
		break;
	case BTClient::LeechingState:
		foreach (TrackerClient *trackerClient, trackerClients)
			trackerClient->start();
		break;
	case BTClient::SeedingState:
		foreach (TrackerClient *trackerClient, trackerClients)
			trackerClient->complete();
		break;
	default:
		break;
	}
}

}

}
