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
#ifndef PROTOCOL_BITTORRENT_TRACKERCLIENTMANAGER_H
#define PROTOCOL_BITTORRENT_TRACKERCLIENTMANAGER_H

#include <QObject>
#include <QUrl>
#include <QList>

namespace Protocol
{

namespace BitTorrent
{

class TrackerClient;
class Peer;
class BTClient;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class TrackerClientManager : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	explicit TrackerClientManager(BTClient *client);
	virtual  ~TrackerClientManager();

	void addTracker(const QUrl &URL);
	void delTracker(TrackerClient *trackerClient);

	QByteArray getInfoHash() const;

	qint64 getUploaded() const;
	qint64 getDownloaded() const;
	qint64 getLeft() const;
public slots:
	void request() const;
signals:
	void peerListUpdated(const QList<Peer*> &peers) const;
private slots:
	void clientStateChanged() const;
private:
	const BTClient * const client;

	QList<TrackerClient*> trackerClients;
};

}

}

#endif
