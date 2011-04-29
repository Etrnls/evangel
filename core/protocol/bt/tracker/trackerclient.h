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
#ifndef PROTOCOL_BITTORRENT_TRACKERCLIENT_H
#define PROTOCOL_BITTORRENT_TRACKERCLIENT_H

#include <QElapsedTimer>
#include <QUrl>

namespace Protocol
{

namespace BitTorrent
{

class Peer;
class TrackerClientManager;
class TrackerClientSocket;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class TrackerClient : public QObject
{
	Q_OBJECT
public:
	//! \note Do NOT modify the names. see eventString
	enum RequestEvent
	{
		invalid = -1,
		none = 0,
		completed = 1,
		started = 2,
		stopped = 3
	};
	Q_ENUMS(RequestEvent)

	static TrackerClient *createTrackerClient(const QUrl &URL, const QByteArray &infoHash);
	static TrackerClient *createTrackerClient(const QUrl &URL, const TrackerClientManager *manager);

	TrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager, TrackerClientSocket *socket);
	virtual ~TrackerClient();

	const QUrl& getURL() const
	{
		return URL;
	}

	void start();
	void stop();
	void request();
	void complete();
signals:
	void peerListUpdated(const QList<Peer*> &peers) const;
protected:
	static const int numberWant;

	virtual QByteArray generateRequest(RequestEvent event) = 0;
	virtual bool parseResponse(const QByteArray &data) = 0;

	void setMinimalRequestInterval(int interval)
	{
		minimalRequestInterval = interval;
	}

	void setRequestInterval(int interval);

	QByteArray eventString(RequestEvent event) const;

	virtual void timerEvent(QTimerEvent *event);

	const QUrl URL;
	const quint32 key;
	const QByteArray infoHash;

	const TrackerClientManager * const manager;
private slots:
	void requestFinished(const QByteArray &data);
private:
	static const int defaultMinimalRequestInterval = 20 * 60 * 1000;
	static const int defaultRequestInterval = defaultMinimalRequestInterval;

	static TrackerClient *createTrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager);

	void autoRequest(RequestEvent event);

	bool checkRequestInterval(RequestEvent event) const;
	bool checkPendingRequest(RequestEvent evnet);

	TrackerClientSocket * const socket;

	int minimalRequestInterval;
	int requestTimer;

	QElapsedTimer requestStopwatch;

	bool running; //!< We are "running" when started request succeeded
};

}

}

#endif
