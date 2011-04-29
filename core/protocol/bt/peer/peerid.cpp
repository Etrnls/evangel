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
#include "peerid.h"

#include <QVariant>
#include <QDateTime>
#include <QRegExp>

namespace Protocol
{

namespace BitTorrent
{

QHash<QString, QString> PeerID::clientDictionary;

/*!
 * \brief Generates a peer ID
 */
void PeerID::generate()
{
	ID = QString::fromLatin1("-EV%1-%2").arg(QCoreApplication::applicationVersion().remove(QLatin1Char('.')))
		.arg(QDateTime::currentDateTime().toTime_t()).toLatin1();
	ID = ID.leftJustified(20, '-', true);
}

QString PeerID::getClientName() const
{
	if (clientName.isEmpty())	clientName = identifyClient(QLatin1String(ID.constData()));
	return clientName;
}

void PeerID::setClientName(const QString &name)
{
	clientName = name;
}

void PeerID::initializeClientDictionary()
{
	if (!clientDictionary.isEmpty())	return;

	// Azureus-style
	clientDictionary.insert(QLatin1String("AG"),	tr("Ares"));
	clientDictionary.insert(QLatin1String("AR"),	tr("Arctic Torrent"));
	clientDictionary.insert(QLatin1String("AV"),	tr("Avicora"));
	clientDictionary.insert(QLatin1String("AX"),	tr("BitPump"));
	clientDictionary.insert(QLatin1String("AZ"),	tr("Azureus"));
	clientDictionary.insert(QLatin1String("A~"), 	tr("Ares"));
	clientDictionary.insert(QLatin1String("BB"),	tr("BitBuddy"));
	clientDictionary.insert(QLatin1String("BC"),	tr("BitComet"));
	clientDictionary.insert(QLatin1String("BF"),	tr("Bitflu"));
	clientDictionary.insert(QLatin1String("BG"),	tr("BTG"));
	clientDictionary.insert(QLatin1String("BR"),	tr("BitRocket"));
	clientDictionary.insert(QLatin1String("BS"),	tr("BTSlave"));
	clientDictionary.insert(QLatin1String("BX"),	tr("BittorrentX"));
	clientDictionary.insert(QLatin1String("CD"),	tr("Enhanced CTorrent"));
	clientDictionary.insert(QLatin1String("CT"),	tr("CTorrent"));
	clientDictionary.insert(QLatin1String("DE"),	tr("Deluge Torrent"));
	clientDictionary.insert(QLatin1String("EB"),	tr("EBit"));
	clientDictionary.insert(QLatin1String("ES"),	tr("electric sheep"));
	clientDictionary.insert(QLatin1String("EV"),	tr("Evangel"));
	clientDictionary.insert(QLatin1String("HL"),	tr("Halite"));
	clientDictionary.insert(QLatin1String("HN"),	tr("Hydranode"));
	clientDictionary.insert(QLatin1String("KT"),	tr("KTorrent"));
	clientDictionary.insert(QLatin1String("LC"),	tr("LeechCraft"));
	clientDictionary.insert(QLatin1String("LK"),	tr("Linkage"));
	clientDictionary.insert(QLatin1String("LP"),	tr("lphant"));
	clientDictionary.insert(QLatin1String("LT"),	tr("libtorrent"));
	clientDictionary.insert(QLatin1String("ML"),	tr("MLDonkey"));
	clientDictionary.insert(QLatin1String("MO"),	tr("Mono Torrent"));
	clientDictionary.insert(QLatin1String("MP"),	tr("MooPolice"));
	clientDictionary.insert(QLatin1String("MR"),	tr("Miro"));
	clientDictionary.insert(QLatin1String("MT"),	tr("Moonlight Torrent"));
	clientDictionary.insert(QLatin1String("PD"),	tr("Pando"));
	clientDictionary.insert(QLatin1String("QT"),	tr("Qt 4"));
	clientDictionary.insert(QLatin1String("SB"),	tr("Swiftbit"));
	clientDictionary.insert(QLatin1String("SD"),	tr("Xunlei"));
	clientDictionary.insert(QLatin1String("SN"),	tr("ShareNet"));
	clientDictionary.insert(QLatin1String("SS"),	tr("SwarmScope"));
	clientDictionary.insert(QLatin1String("ST"),	tr("SymTorrent"));
	clientDictionary.insert(QLatin1String("SZ"),	tr("Shareaza"));
	clientDictionary.insert(QLatin1String("S~"),	tr("Shareaza (beta)"));
	clientDictionary.insert(QLatin1String("TN"),	tr("Torrent.NET"));
	clientDictionary.insert(QLatin1String("TR"),	tr("Transmission"));
	clientDictionary.insert(QLatin1String("TS"),	tr("TorrentStorm"));
	clientDictionary.insert(QLatin1String("TT"),	tr("TuoTu"));
	clientDictionary.insert(QLatin1String("UL"),	tr("uLeecher"));
	clientDictionary.insert(QLatin1String("UT"),	tr("uTorrent"));
	clientDictionary.insert(QLatin1String("XL"),	tr("Xunlei"));
	clientDictionary.insert(QLatin1String("XT"),	tr("XanTorrent"));
	clientDictionary.insert(QLatin1String("XX"),	tr("Xtorrent"));
	clientDictionary.insert(QLatin1String("ZT"),	tr("ZipTorrent"));
	clientDictionary.insert(QLatin1String("lt"),	tr("rTorrent"));
	clientDictionary.insert(QLatin1String("pX"),	tr("pHoeniX"));
	clientDictionary.insert(QLatin1String("qB"),	tr("qBittorrent"));
	clientDictionary.insert(QLatin1String("st"),	tr("SharkTorrent"));

	// Shadow's-style
	clientDictionary.insert(QLatin1String("A"),	tr("ABC"));
	clientDictionary.insert(QLatin1String("O"),	tr("Osprey Permaseed"));
	clientDictionary.insert(QLatin1String("Q"),	tr("BTQueue"));
	clientDictionary.insert(QLatin1String("R"),	tr("Tribler"));
	clientDictionary.insert(QLatin1String("S"),	tr("Shadow"));
	clientDictionary.insert(QLatin1String("T"),	tr("BitTornado"));
	clientDictionary.insert(QLatin1String("U"),	tr("UPnP"));

	// Others
	clientDictionary.insert(QLatin1String("BS"),	tr("BitSpirit"));
	clientDictionary.insert(QLatin1String("FG"),	tr("FlashGet"));
	clientDictionary.insert(QLatin1String("M"),	tr("Mainline"));
	clientDictionary.insert(QLatin1String("OP"),	tr("Opera"));
}

QString PeerID::lookupClientDictionary(const QString &clientID)
{
	const auto &clientName = clientDictionary.find(clientID);

	return clientName != clientDictionary.constEnd() ? clientName.value() : clientID;
}

/*!
 * \brief Parses the Azureus style version
 */
QString PeerID::AzureusStyleVersion(const QString &clientID, const QString &ID)
{
	if (clientID == QLatin1String("TR"))
	{
		// Transmission, see http://trac.transmissionbt.com/wiki/PeerId
		if (QRegExp(QLatin1String("\\d{3}[0XZ]")).exactMatch(ID.mid(3, 4)))
			return QString::fromLatin1("%1.%2 %3").arg(ID[3]).arg(ID.mid(4, 2).toInt())
				.arg(ID[6] == QLatin1Char('0') ? tr("Stable") : (ID[6] == QLatin1Char('X') ? tr("Beta") : tr("Nightly build")));
		else
			return QString::fromLatin1("%1.%2").arg(ID.mid(3, 2).toInt()).arg(ID.mid(5, 2).toInt());
	}
	else if (clientID == QLatin1String("EV"))
	{
		// ourself
		return QString::fromLatin1("%1.%2.%3 %4").arg(ID[3]).arg(ID[4]).arg(ID[5])
			.arg(ID[6] == QLatin1Char('0') ? QLatin1String("Dev") : QLatin1String(""));
	}
	else
		return QString::fromLatin1("%1.%2.%3.%4").arg(ID[3]).arg(ID[4]).arg(ID[5]).arg(ID[6]);
}

QString PeerID::ShadowStyleVersion(const QString &clientID, const QString &ID)
{
	Q_UNUSED(clientID);
	return QString::fromLatin1("%1.%2.%3").arg(ShadowStyleBase64(ID[1])).arg(ShadowStyleBase64(ID[2])).arg(ShadowStyleBase64(ID[3]));
}

int PeerID::ShadowStyleBase64(QChar c)
{
	// 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.-

	if (c.isDigit())
		return c.toLatin1() - '0';
	else if (c.isUpper())
		return 10 + c.toLatin1() - 'A';
	else if (c.isLower())
		return 36 + c.toLatin1() - 'a';
	else if (c == QLatin1Char('.'))
		return 62;
	else if (c == QLatin1Char('-'))
		return 63;
	else
		return -1;
}

/*!
 * \brief Identifies the client
 * \param ID The peer ID
 */
QString PeerID::identifyClient(QString ID)
{
	initializeClientDictionary();

	QString result = tr("Unknown client [%1]").arg(ID);

	if (QRegExp(QLatin1String("-[a-zA-Z]{2}\\d{4}-")).exactMatch(ID.left(8)))
	{
		// Azureus-style
		const QString &clientID = ID.mid(1, 2);

		result = lookupClientDictionary(clientID) + QLatin1Char(' ') + AzureusStyleVersion(clientID, ID);
	}
	else if (QRegExp(QLatin1String("[0-9a-zA-Z][0-9A-Za-z.-]{5}")).exactMatch(ID.left(6)))
	{
		// Shadow's-style
		const QString &clientID = ID.left(1);

		result = lookupClientDictionary(clientID) + QLatin1Char(' ') + ShadowStyleVersion(clientID, ID);
	}
	else if (QRegExp(QLatin1String("M\\d-\\d-\\d--")).exactMatch(ID.left(8)))
	{
		// Mainline
		result = QString::fromLatin1("%1 %2.%3.%4").arg(lookupClientDictionary(QLatin1String("M"))).arg(ID[1]).arg(ID[3]).arg(ID[5]);
	}
	else if (QRegExp(QLatin1String("M\\d-\\d{2}-\\d-")).exactMatch(ID.left(8)))
	{
		// Mainline
		result = QString::fromLatin1("%1 %2.%3.%4").arg(lookupClientDictionary(QLatin1String("M"))).arg(ID[1]).arg(ID.mid(3, 2)).arg(ID[6]);
	}
	else if (QRegExp(QLatin1String("OP\\d{4}")).exactMatch(ID.left(6)))
	{
		// Opera
		result = lookupClientDictionary(QLatin1String("OP")) + QLatin1Char(' ') + ID.mid(2, 4);
	}
	else if (ID.startsWith(QLatin1String("-FG")))
	{
		// FlashGet
		result = lookupClientDictionary(QLatin1String("FG")) + QLatin1Char(' ') + ID.mid(3, 4);
	}
	else if (ID.mid(2, 2) == QLatin1String("BS"))
	{
		// BitSpirit
		result = lookupClientDictionary(QLatin1String("BS"));
	}

	return result;
}

}

}
