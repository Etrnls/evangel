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
#include "btmagnetsource.h"
#include "btmagnettransfer.h"

#include <QUrl>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \brief Creates a BTMagnetSource object
 */
BTMagnetSource::BTMagnetSource(const QByteArray &infoHash, const QString &name, const QStringList &trackers)
	: infoHash(infoHash), name(name), trackers(trackers)
{
}

AbstractTransfer *BTMagnetSource::createTransfer() const
{
	return new BTMagnetTransfer(infoHash, name, trackers);
}

class BTMagnetSourceFactory : public AbstractSourceFactory
{
protected:
	virtual AbstractSource *create(QString URL) const
	{
		const QUrl &decodedURL = QUrl::fromPercentEncoding(URL.toLatin1());

		// scheme checking
		if (decodedURL.scheme().toLower() != QLatin1String(scheme))	return 0;

		// info hash
		QString infoHashString = decodedURL.queryItemValue(QLatin1String("xt"));
		if (!infoHashString.startsWith(QLatin1String(xtPrefix)))	return 0;
		infoHashString.remove(0, qstrlen(xtPrefix));
		QByteArray infoHash;
		switch (infoHashString.size())
		{
		case 40:
			infoHash = QByteArray::fromHex(infoHashString.toLatin1());
			break;
		case 32:
			// TODO: base32 format support
			return 0;
			break;
		default:
			return 0;
		}

		return new BTMagnetSource(infoHash,
			decodedURL.queryItemValue(QLatin1String("dn")),
			decodedURL.allQueryItemValues(QLatin1String("tr")));
	}
private:
	static const char scheme[];
	static const char xtPrefix[];
};

const char BTMagnetSourceFactory::scheme[] = "magnet";
const char BTMagnetSourceFactory::xtPrefix[] = "urn:btih:";

static const BTMagnetSourceFactory initializer;

}

}

