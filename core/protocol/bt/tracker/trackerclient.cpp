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
#include "trackerclient.h"
#include "trackerclientsocket.h"
#include "trackerclientmanager.h"

#include "http/httptrackerclient.h"
#include "udp/udptrackerclient.h"

#include "protocol/bt/peer/peermanager.h"

#include "future/job.h"
#include "future/jobmanager.h"
#include "util/numeric/random.h"
#include "log.h"

#include <QMetaEnum>
#include <QTimerEvent>
#include <QTimer>

namespace Protocol
{

namespace BitTorrent
{

const int TrackerClient::numberWant = PeerManager::minimalPeersNeeded;

TrackerClient *TrackerClient::createTrackerClient(const QUrl &URL, const QByteArray &infoHash)
{
	return createTrackerClient(URL, infoHash, 0);
}

TrackerClient *TrackerClient::createTrackerClient(const QUrl &URL, const TrackerClientManager *manager)
{
	return createTrackerClient(URL, manager->getInfoHash(), manager);
}

TrackerClient::TrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager, TrackerClientSocket *socket)
				: URL(URL), key(Utility::rand(Q_INT64_C(0xffffffff))), infoHash(infoHash),
				manager(manager), socket(socket),
				minimalRequestInterval(defaultMinimalRequestInterval),
				requestTimer(startTimer(defaultRequestInterval)), running(false)
{
	connect(dynamic_cast<QObject*>(socket), SIGNAL(requestFinished(QByteArray)), SLOT(requestFinished(QByteArray)));
}

TrackerClient::~TrackerClient()
{
	if (socket->hasPendingRequest())
		Future::JobManager::getInstance()->attach(new Future::Job(dynamic_cast<QObject*>(socket), SIGNAL(requestFinished(QByteArray))));
	else
		delete socket;
}

void TrackerClient::start()
{
	Q_ASSERT_X(!running, Q_FUNC_INFO, "Already running");
	autoRequest(started);
}

void TrackerClient::stop()
{
	if (running)
	{
		autoRequest(stopped);
		running = false;
	}
}

void TrackerClient::request()
{
	return running ? autoRequest(none) : start();
}

void TrackerClient::complete()
{
	return running ? autoRequest(completed) : start();
}

void TrackerClient::setRequestInterval(int interval)
{
	killTimer(requestTimer);
	requestTimer = startTimer(interval);

	setMinimalRequestInterval(qMin(minimalRequestInterval, interval));
}

QByteArray TrackerClient::eventString(RequestEvent event) const
{
	return QByteArray(metaObject()->enumerator(metaObject()->indexOfEnumerator("RequestEvent")).valueToKey(event));
}

void TrackerClient::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == requestTimer)
		request();
	else
		QObject::timerEvent(event);
}

void TrackerClient::requestFinished(const QByteArray &data)
{
	const bool result = parseResponse(data);

	if (!running && result)	running = true;
}

TrackerClient *TrackerClient::createTrackerClient(const QUrl &URL, const QByteArray &infoHash, const TrackerClientManager *manager)
{
	if (URL.scheme().toLower() == QLatin1String("http") || URL.scheme().toLower() == QLatin1String("https"))
		return new HTTPTrackerClient(URL, infoHash, manager);
	else if (URL.scheme().toLower() == QLatin1String("udp"))
		return new UDPTrackerClient(URL, infoHash, manager);
	else
		return 0;
}

void TrackerClient::autoRequest(RequestEvent event)
{
	if (!checkRequestInterval(event))	return;
	if (!checkPendingRequest(event))	return;

	log(this) << QString::fromLatin1("Performing tracker [%1] request [%2, %3, %4, %5] for torrent [%6]").arg(URL.toString())
			.arg(QLatin1String(eventString(event).constData()))
			.arg(manager ? manager->getDownloaded() : 0)
			.arg(manager ? manager->getUploaded() : 0)
			.arg(manager ? manager->getLeft() : 0)
			.arg(QLatin1String(infoHash.toHex().constData())) << endl;

	socket->request(generateRequest(event));

	requestStopwatch.restart();
}

bool TrackerClient::checkRequestInterval(RequestEvent event) const
{
	if (event != none)	return true;
	if (!requestStopwatch.isValid())	return true; // first request

	return requestStopwatch.hasExpired(minimalRequestInterval);
}

bool TrackerClient::checkPendingRequest(RequestEvent event)
{
	if (!socket->hasPendingRequest())	return true;

	if (event != none)
	{
		socket->cancelPendingRequest();
		return true;
	}

	return false;
}

}

}
