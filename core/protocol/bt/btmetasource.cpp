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
#include "btmetasource.h"
#include "bttransfer.h"
#include "util/metainfo.h"

#include "disk/file.h"

#include <QUrl>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \brief Creates a BTMetaSource object
 * \param torrent The path of the meta info (.torrent) file.
 */
BTMetaSource::BTMetaSource(const QString &torrent)
{
	// loads the meta info file
	Disk::File file(torrent);
	file.open(Disk::File::ReadOnly | Disk::File::Unbuffered);

	torrentData = qCompress(file.readAll());
}

AbstractTransfer *BTMetaSource::createTransfer() const
{
	MetaInfo *metaInfo = new MetaInfo(qUncompress(torrentData));
	if (metaInfo->isValid())
		return new BTTransfer(metaInfo); // The metaInfo is deleted by BTTransfer

	delete metaInfo;
	return 0;
}


class BTMetaSourceFactory : public AbstractSourceFactory
{
protected:
	virtual AbstractSource *create(QString URL) const
	{
		// scheme checking
		if (QUrl(URL).scheme().toLower() != QLatin1String(scheme))	return 0;

		// construct file info
		const QFileInfo fileInfo(URL.mid((QString::fromLatin1(scheme) + QLatin1Char(':')).size()));

		// torrent file checking
		if (fileInfo.suffix().toLower() != QLatin1String("torrent"))	return 0;
		if (!fileInfo.exists() || fileInfo.size() > maxTorrentFileSize)	return 0;

		return new BTMetaSource(fileInfo.absoluteFilePath());
	}
private:
	static const char scheme[];
	static const int maxTorrentFileSize = 2 * 1024 * 1024; // 2 MiB
};

const char BTMetaSourceFactory::scheme[] = "file";

static const BTMetaSourceFactory initializer;

}

}
