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
#ifndef PROTOCOL_BITTORRENT_TRACKERCLIENTSOCKET_H
#define PROTOCOL_BITTORRENT_TRACKERCLIENTSOCKET_H

#include <QByteArray>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 *
 * This class seperates the tracker request logic and the network protocol related implementation details.
 * It also provides the ability to delete the TrackerClient but still finishes a pending "stopped" request.
 */
class TrackerClientSocket
{
public:
	inline TrackerClientSocket() {}
	virtual ~TrackerClientSocket() {}

	virtual bool hasPendingRequest() const = 0;
	virtual void cancelPendingRequest() = 0;

	virtual void request(const QByteArray &data) = 0;
private:
	TrackerClientSocket(const TrackerClientSocket &);
	TrackerClientSocket& operator=(const TrackerClientSocket &);
};

}

}

#endif
