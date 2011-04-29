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
#include "httptrackerclientsocket.h"

#include <QUrl>

namespace Protocol
{

namespace BitTorrent
{

HTTPTrackerClientSocket::HTTPTrackerClientSocket(const QUrl &URL)
	: QHttp(URL.host(), URL.port(80)), requestID(-1)
{
	if (!URL.userName().isEmpty())	setUser(URL.userName(), URL.password());

	connect(this, SIGNAL(requestFinished(int, bool)), SLOT(HTTPRequestFinished(int)));
}

void HTTPTrackerClientSocket::HTTPRequestFinished(int id)
{
	if (id == requestID)
	{
		requestID = -1;
		emit requestFinished(readAll());
	}
}

}

}
