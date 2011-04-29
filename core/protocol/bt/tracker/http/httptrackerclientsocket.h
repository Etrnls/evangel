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
#ifndef PROTOCOL_BITTORRENT_HTTPTRACKERCLIENTSOCKET_H
#define PROTOCOL_BITTORRENT_HTTPTRACKERCLIENTSOCKET_H

#include "protocol/bt/tracker/trackerclientsocket.h"

#include <QHttp>

class QUrl;

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class HTTPTrackerClientSocket : public QHttp, public TrackerClientSocket
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	explicit HTTPTrackerClientSocket(const QUrl &URL);

	virtual bool hasPendingRequest() const
	{
		return requestID != -1;
	}

	virtual void cancelPendingRequest()
	{
		requestID = -1;
		abort();
	}

	virtual void request(const QByteArray &data)
	{
		Q_ASSERT_X(!hasPendingRequest(), Q_FUNC_INFO, "Pending request not finished yet");
		requestID = get(QLatin1String(data.constData()));
	}
signals:
	void requestFinished(const QByteArray &data) const;
private slots:
	void HTTPRequestFinished(int id);
private:
	int requestID;
};

}

}

#endif
