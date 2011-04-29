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
#include "trafficmanager.h"
#include "ratestatistics.h"

#include "log.h"
#include "util/settings.h"

#include <QTimerEvent>

namespace Network
{

/*!
 * \brief Attaches a socket to traffic managing
 */
void TrafficManager::attach(AbstractSocket *socket)
{
	socket->setNetworkReadBufferSize(networkReadBufferSizeFactor * downloadLimit);
	connect(socket, SIGNAL(readyDownload()), SLOT(socketReadyDownload()), Qt::QueuedConnection);
	connect(socket, SIGNAL(readyUpload()), SLOT(socketReadyUpload()), Qt::QueuedConnection);
	sockets.insert(socket);
}

/*!
 * \brief Detaches a socket from traffic managing
 */
void TrafficManager::detach(AbstractSocket *socket)
{
	socket->setNetworkReadBufferSize(0);
	disconnect(socket, 0, this, 0);

	sockets.remove(socket);

	for (int priority = AbstractSocket::HighestPriority; priority <= AbstractSocket::LowestPriority; ++priority)
	{
		readyReadSockets[priority].removeOne(socket);
		readyWriteSockets[priority].removeOne(socket);
	}
}


/*!
 * \brief Sets the download limit in KiB/s
 */
void TrafficManager::setDownloadLimit(int downloadLimit)
{
	downloadLimit = qMax(downloadLimit * 1024, 0);
	if (this->downloadLimit == downloadLimit) return;
	this->downloadLimit = downloadLimit;

	saveSettings();

	foreach(AbstractSocket *socket, sockets)
		socket->setNetworkReadBufferSize(networkReadBufferSizeFactor * downloadLimit);
}


/*!
 * \brief Sets the upload limit in KiB/s
 */
void TrafficManager::setUploadLimit(int uploadLimit)
{
	uploadLimit = qMax(uploadLimit * 1024, 0);
	if (this->uploadLimit == uploadLimit) return;
	this->uploadLimit = uploadLimit;

	saveSettings();
}

qreal TrafficManager::getDownloadRate() const
{
	return downloadRate->getRate();
}

qreal TrafficManager::getUploadRate() const
{
	return uploadRate->getRate();
}

void TrafficManager::socketReadyDownload()
{
	AbstractSocket *socket = qobject_cast<AbstractSocket*>(sender());

	if (!sockets.contains(socket))	return;

	if (socket->getPriority() == AbstractSocket::PrivilegePriority || downloadLimit == 0)
	{
		const int read = socket->download();
		if (socket->getPriority() != AbstractSocket::PrivilegePriority)
			downloadRate->transfer(read);
		return;
	}

	download(socket);
}

void TrafficManager::socketReadyUpload()
{
	AbstractSocket *socket = qobject_cast<AbstractSocket*>(sender());

	if (!sockets.contains(socket))	return;

	if (socket->getPriority() == AbstractSocket::PrivilegePriority || uploadLimit == 0)
	{
		const int written = socket->upload();
		if (socket->getPriority() != AbstractSocket::PrivilegePriority)
			uploadRate->transfer(written);
		return;
	}

	upload(socket);
}

TrafficManager::TrafficManager() : canDownload(0), canUpload(0), updateTimer(startTimer(updateInterval)),
				downloadRate(new RateStatistics(this)), uploadRate(new RateStatistics(this))
{
	log(this) << "Initializing" << endl;

	loadSettings();

	downloadRate->start();
	uploadRate->start();

	log(this) << "Successfully initialized" << endl;
}

TrafficManager::~TrafficManager()
{
	log(this) << "Exiting" << endl;

	Q_ASSERT_X(sockets.isEmpty(), Q_FUNC_INFO, "sockets not empty");

	log(this) << "Successfully exited" << endl;
}


void TrafficManager::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == updateTimer)
		update();
	else
		QObject::timerEvent(event);
}

void TrafficManager::update()
{
	canDownload = qRound(downloadLimit * static_cast<qreal>(updateInterval) / 1000);
	canUpload = qRound(uploadLimit * static_cast<qreal>(updateInterval) / 1000);

	// download
	for (int priority = AbstractSocket::HighestPriority; priority <= AbstractSocket::LowestPriority; ++priority)
	{
		while (canDownload > 0 && !readyReadSockets[priority].isEmpty())
			download(readyReadSockets[priority].takeFirst());

		if (canDownload == 0)	break;
	}

	// upload
	for (int priority = AbstractSocket::HighestPriority; priority <= AbstractSocket::LowestPriority; ++priority)
	{
		while (canUpload > 0 && !readyWriteSockets[priority].isEmpty())
			upload(readyWriteSockets[priority].takeFirst());

		if (canUpload == 0)	break;
	}
}

void TrafficManager::download(AbstractSocket *socket)
{
	if (canDownload > 0)
	{
		const int read = socket->download(canDownload);
		canDownload -= read;
		downloadRate->transfer(read);
	}
	if (socket->canDownload())	addReadySocket(readyReadSockets[socket->getPriority()], socket);
}

void TrafficManager::upload(AbstractSocket *socket)
{
	if (canUpload > 0)
	{
		const int written = socket->upload(canUpload);
		canUpload -= written;
		uploadRate->transfer(written);
	}
	if (socket->canUpload())	addReadySocket(readyWriteSockets[socket->getPriority()], socket);
}

void TrafficManager::addReadySocket(QList<AbstractSocket*> &readySockets, AbstractSocket *socket)
{
	if (!readySockets.contains(socket))	readySockets.append(socket);
}

/*!
 * \brief Loads the settings
 */
void TrafficManager::loadSettings()
{
	downloadLimit = Utility::Settings::load(this, QLatin1String("downloadLimit"), 0).toInt() * 1024;
	uploadLimit = Utility::Settings::load(this, QLatin1String("uploadLimit"), 0).toInt() * 1024;
}

/*!
 * \brief Saves the settings
 */
void TrafficManager::saveSettings() const
{
	Utility::Settings::save(this, QLatin1String("downloadLimit"), downloadLimit / 1024);
	Utility::Settings::save(this, QLatin1String("uploadLimit"), uploadLimit / 1024);
}

}
