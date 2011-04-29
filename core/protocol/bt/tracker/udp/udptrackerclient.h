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
#ifndef PROTOCOL_BITTORRENT_UDPTRACKERCLIENT_H
#define PROTOCOL_BITTORRENT_UDPTRACKERCLIENT_H

#include "protocol/bt/tracker/trackerclient.h"

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 * \todo authentication support
 */
class UDPTrackerClient : public TrackerClient
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	enum RequestAction
	{
		Connect = 0,
		Announce = 1,
		Scrape = 2,
		Error = 3
	};

	UDPTrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager);
	virtual ~UDPTrackerClient();
private:
	virtual QByteArray generateRequest(RequestEvent event);
	virtual bool parseResponse(const QByteArray &data);
};

}

}

#endif
