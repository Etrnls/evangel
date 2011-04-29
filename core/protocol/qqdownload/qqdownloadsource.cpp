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
#include "qqdownloadsource.h"

#include "abstractsource.h"

#include <QUrl>

namespace Protocol
{

namespace QQDownload
{

class QQDownloadSourceFactory : public AbstractSourceFactory
{
protected:
	virtual AbstractSource *create(QString URL) const
	{
		// scheme checking
		if (QUrl(URL).scheme().toLower() != QLatin1String(scheme))	return 0;

		// decode
		const QString decodedURL = QLatin1String(QByteArray::fromBase64(URL.mid((QLatin1String(scheme) + QLatin1String("://")).size()).toLatin1()).constData());

		return createSource(decodedURL);
	}
private:
	static const char scheme[];
};

const char QQDownloadSourceFactory::scheme[] = "qqdl";

static const QQDownloadSourceFactory initializer;

}

}
