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
#include "httptrackerclient.h"
#include "httptrackerclientsocket.h"

#include "protocol/bt/tracker/trackerclientmanager.h"
#include "protocol/bt/btclient.h"
#include "protocol/bt/btserver.h"
#include "protocol/bt/util/bcodec.h"
#include "protocol/bt/peer/peer.h"

#include "log.h"
#include "util/endian.h"
#include "util/numeric/random.h"

namespace Protocol
{

namespace BitTorrent
{

const char HTTPTrackerClient::failureReasonKey[]  = "failure reason";
const char HTTPTrackerClient::warningMessageKey[] = "warning message";
const char HTTPTrackerClient::intervalKey[]       = "interval";
const char HTTPTrackerClient::minIntervalKey[]    = "min interval";
const char HTTPTrackerClient::trackerIDKey[]      = "tracker id";
const char HTTPTrackerClient::completeKey[]       = "complete";
const char HTTPTrackerClient::incompleteKey[]     = "incomplete";
const char HTTPTrackerClient::peersKey[]          = "peers";
const char HTTPTrackerClient::peerIDKey[]         = "peer id";
const char HTTPTrackerClient::IPKey[]             = "ip";
const char HTTPTrackerClient::portKey[]           = "port";

HTTPTrackerClient::HTTPTrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager)
	: TrackerClient(URL, infoHash, manager, new HTTPTrackerClientSocket(URL))
{
}

HTTPTrackerClient::~HTTPTrackerClient()
{
	stop();
}

QByteArray HTTPTrackerClient::generateRequest(RequestEvent event)
{
	QUrl URL(this->URL);

	URL.addEncodedQueryItem("info_hash", infoHash.toPercentEncoding());
	URL.addQueryItem(QLatin1String("peer_id"), QLatin1String(BTClient::getPeerID().getID().constData()));
	URL.addQueryItem(QLatin1String("port"), QString::number(BTClient::getServer()->serverPort()));
	URL.addQueryItem(QLatin1String("key"), QString::number(key));

	URL.addQueryItem(QLatin1String("downloaded"), QString::number(manager ? manager->getDownloaded() : 0));
	URL.addQueryItem(QLatin1String("uploaded"), QString::number(manager ? manager->getUploaded() : 0));
	URL.addQueryItem(QLatin1String("left"), QString::number(manager ? manager->getLeft() : 0));

	URL.addQueryItem(QLatin1String("numwant"), QString::number(event != stopped ? numberWant: 0));
	URL.addQueryItem(QLatin1String("no_peer_id"), QLatin1String("1"));
	URL.addQueryItem(QLatin1String("compact"), QLatin1String("1"));

	if (event != none)	URL.addQueryItem(QLatin1String("event"), QLatin1String(eventString(event).constData()));

	if (!trackerID.isEmpty())	URL.addQueryItem(QLatin1String("trackerid"), QLatin1String(trackerID.constData()));

	return URL.toEncoded(QUrl::RemoveScheme | QUrl::RemovePort | QUrl::RemoveAuthority);
}

bool HTTPTrackerClient::parseResponse(const QByteArray &data)
{
	Bcodec bcodec(data);

	if (!bcodec.isValid())
	{
		log(this) << QString::fromLatin1("Failed to parse the response from tracker [%1] : %2")
				.arg(URL.toString()).arg(bcodec.getError()) << endl;
		return false;
	}

	const Bcodec::BDictionary &response = bcodec.getResult();

	// If present, then no other keys may be present.
	// The value is a human-readable error message as to why the request failed (string).
	if (response.contains(failureReasonKey))
	{
		log(this) << QString::fromLatin1("Tracker [%1] returned failure message : '%2'").arg(URL.toString())
				.arg(QLatin1String(response.value(failureReasonKey).value<Bcodec::BString>().constData())) << endl;
		return false;
	}

	if (response.contains(warningMessageKey))
		log(this) << QString::fromLatin1("Tracker [%1] returned warning message : '%2'").arg(URL.toString())
				.arg(QLatin1String(response.value(warningMessageKey).value<Bcodec::BString>().constData())) << endl;

	if (response.contains(intervalKey))
		setRequestInterval(response.value(intervalKey).value<Bcodec::BInteger>() * 1000);

	if (response.contains(minIntervalKey))
		setMinimalRequestInterval(response.value(minIntervalKey).value<Bcodec::BInteger>() * 1000);

	if (response.contains(trackerIDKey))
		trackerID = response.value(trackerIDKey).value<Bcodec::BString>();

	if (response.contains(peersKey))
	{
		QList<Peer*> peers;

		// some tracker may return wrong type when no peers are available
		if (response.value(peersKey).userType() == qMetaTypeId<Bcodec::BList>())
		{
			const Bcodec::BList &peerList = response.value(peersKey).value<Bcodec::BList>();

			for (auto i = peerList.constBegin(); i != peerList.constEnd(); ++i)
			{
				const Bcodec::BDictionary &dictionary = i->value<Bcodec::BDictionary>();

				const QString IP = QLatin1String(dictionary.value(IPKey).value<Bcodec::BString>().constData());
				const quint16 port = dictionary.value(portKey).value<Bcodec::BInteger>();
				Peer * const peer = new Peer(QHostAddress(IP), port);

				if (dictionary.contains(peerIDKey))
					peer->setPeerID(dictionary.value(peerIDKey).value<Bcodec::BString>());

				peers.append(peer);
			}
		}
		else if (response.value(peersKey).userType() == qMetaTypeId<Bcodec::BString>())
		{
			const QByteArray &peerList = static_cast<QByteArray>(response.value(peersKey).value<Bcodec::BString>());

			for (int offset = 0; offset + 6 < peerList.size(); offset += 6)
			{
				const quint32 IP = Utility::Endian::fromBigEndian<quint32>(peerList.constData() + offset);
				const quint16 port = Utility::Endian::fromBigEndian<quint16>(peerList.constData() + offset + 4);
				peers.append(new Peer(QHostAddress(IP), port));
			}
		}

		log(this) << QString::fromLatin1("Tracker [%1] returned %2 peer(s) for torrent [%3]").arg(URL.toString()).arg(peers.size())
				.arg(QLatin1String(infoHash.toHex().constData()))<< endl;
		emit peerListUpdated(peers);
	}

	return true;
}

}

}
