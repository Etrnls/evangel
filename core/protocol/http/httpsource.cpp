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
#include "httpsource.h"
#include "httptransfer.h"

#include <QUrl>
#include <QFileInfo>

namespace Protocol
{

namespace HTTP
{

AbstractTransfer *HTTPSource::createTransfer() const
{
	return new HTTPTransfer(URL);
}

class HTTPSourceFactory : public AbstractSourceFactory
{
protected:
	virtual AbstractSource *create(QString URL) const
	{
		// scheme checking
		if (QUrl(URL).scheme().toLower() != QLatin1String(scheme[0]) &&
		    QUrl(URL).scheme().toLower() != QLatin1String(scheme[1]))
			return 0;

		// URL checking
		if (QFileInfo(QUrl(URL).path()).fileName().isEmpty())	return 0;

		return new HTTPSource(URL);
	}
private:
	static const char *scheme[2];
};

const char *HTTPSourceFactory::scheme[2] = {"http", "https"};

static const HTTPSourceFactory initializer;

}

}
