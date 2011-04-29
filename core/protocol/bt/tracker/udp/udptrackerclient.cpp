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
#include "udptrackerclient.h"
#include "udptrackerclientsocket.h"

#include "protocol/bt/tracker/trackerclientmanager.h"
#include "protocol/bt/btclient.h"
#include "protocol/bt/btserver.h"
#include "protocol/bt/peer/peer.h"

#include "log.h"
#include "util/endian.h"

namespace Protocol
{

namespace BitTorrent
{

UDPTrackerClient::UDPTrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager)
	: TrackerClient(URL, infoHash, manager, new UDPTrackerClientSocket(URL))
{
}

UDPTrackerClient::~UDPTrackerClient()
{
	stop();
}

QByteArray UDPTrackerClient::generateRequest(RequestEvent event)
{
/*
	Offset	Size		Name		Value
	0	64-bit integer	connection_id
	8	32-bit integer	action		1 // Announce
	12	32-bit integer	transaction_id
	16	20-byte string	info_hash
	36	20-byte string	peer_id
	56	64-bit integer	downloaded
	64	64-bit integer	left
	72	64-bit integer	uploaded
	80	32-bit integer	event
	84	32-bit integer	IP address	0 // default
	88	32-bit integer	key
	92	32-bit integer	num_want	-1 // default
	96	16-bit integer	port
	98
 */

	QByteArray data(98, Qt::Uninitialized);

	Utility::Endian::toBigEndian<quint32>(Announce, &data.data()[8]);

	data.replace(16, 20, infoHash);
	data.replace(36, 20, BTClient::getPeerID().getID());

	Utility::Endian::toBigEndian<quint64>(manager ? manager->getDownloaded() : 0, &data.data()[56]);
	Utility::Endian::toBigEndian<quint64>(manager ? manager->getLeft() : 0, &data.data()[64]);
	Utility::Endian::toBigEndian<quint64>(manager ? manager->getUploaded() : 0, &data.data()[72]);

	Utility::Endian::toBigEndian<quint32>(event, &data.data()[80]);
	Utility::Endian::toBigEndian<quint32>(0, &data.data()[84]);
	Utility::Endian::toBigEndian<quint32>(key, &data.data()[88]);
	Utility::Endian::toBigEndian<quint32>(numberWant, &data.data()[92]);
	Utility::Endian::toBigEndian<quint16>(BTClient::getServer()->serverPort(), &data.data()[96]);

	return data;
}

bool UDPTrackerClient::parseResponse(const QByteArray &data)
{
	switch (static_cast<RequestAction>(Utility::Endian::fromBigEndian<quint32>(data.constData())))
	{
	case Announce:
		/*
			Offset		Size		Name		Value
			0		32-bit integer	action		1 // Announce
			4		32-bit integer	transaction_id
			8		32-bit integer	interval
			12		32-bit integer	leechers
			16		32-bit integer	seeders
			20 + 6 * n	32-bit integer	IP address
			24 + 6 * n	16-bit integer	TCP port
			20 + 6 * N
		 */
		if (data.size() < 20)	return false;

		setRequestInterval(Utility::Endian::fromBigEndian<quint32>(&data.constData()[8]) * 1000);

		{
			QList<Peer*> peers;

			for (int offset = 20; offset + 6 < data.size(); offset += 6)
			{
				const quint32 IP = Utility::Endian::fromBigEndian<quint32>(&data.constData()[offset]);
				const quint16 port = Utility::Endian::fromBigEndian<quint16>(&data.constData()[offset + 4]);
				peers.append(new Peer(QHostAddress(IP), port));
			}

			log(this) << QString::fromLatin1("Tracker [%1] returned %2 peer(s) for torrent [%3]").arg(URL.toString()).arg(peers.size())
					.arg(QLatin1String(infoHash.toHex().constData())) << endl;

			emit peerListUpdated(peers);
		}
		break;
	case Scrape:
		break;
	case Error:
		/*
			Offset	Size		Name		Value
			0	32-bit integer	action		3 // Error
			4	32-bit integer	transaction_id
			8	string  message
		*/
		if (data.size() < 8)	return false;

		log(this) << QString::fromLatin1("Tracker [%1] returned error message : '%2'").arg(URL.toString())
				.arg(QLatin1String(data.mid(8).constData())) << endl;
		return false;
	default:
		return false;
	}

	return true;
}


}

}
