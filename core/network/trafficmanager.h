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
#ifndef NETWORK_TRAFFICMANAGER_H
#define NETWORK_TRAFFICMANAGER_H

#include "abstractsocket.h"

#include "singleton.h"

#include <QSet>
#include <QList>

namespace Network
{

class RateStatistics;

/*!
 * \brief Manages all network traffic and provides reliable and fair traffic control
 * \author Etrnls <Etrnls@gmail.com>
 */
class TrafficManager : public QObject, public Singleton<TrafficManager>
{
	Q_OBJECT
	Q_CLASSINFO("log", "NetworkTrafficManager")
public:
	void attach(AbstractSocket *socket);
	void detach(AbstractSocket *socket);

	//! \brief Retrieves the download limit in KiB/s
	int getDownloadLimit() const	{	return downloadLimit / 1024;	}

	//! \brief Retrieves the upload limit in KiB/s
	int getUploadLimit() const	{	return uploadLimit / 1024;	}

	void setDownloadLimit(int downloadLimit);
	void setUploadLimit(int uploadLimit);

	qreal getDownloadRate() const;
	qreal getUploadRate() const;
private slots:
	void socketReadyDownload();
	void socketReadyUpload();
private:
	static const int updateInterval = 23;
	static const int networkReadBufferSizeFactor = 2; //!< The factor used to set read buffer size in order to keep download steady

	TrafficManager();
	virtual ~TrafficManager();

	virtual void timerEvent(QTimerEvent *event);

	void update();

	void download(AbstractSocket *socket);
	void upload(AbstractSocket *socket);

	void addReadySocket(QList<AbstractSocket*> &readySockets, AbstractSocket *socket);

	void loadSettings();
	void saveSettings() const;

	int downloadLimit; //!< The download limit in bytes/second
	int uploadLimit;   //!< The upload limit in bytes/second

	int canDownload;
	int canUpload;

	int updateTimer;

	//! The sockets currently managing
	QSet<AbstractSocket*> sockets;

	QList<AbstractSocket*> readyReadSockets[AbstractSocket::priorityCount];
	QList<AbstractSocket*> readyWriteSockets[AbstractSocket::priorityCount];

	RateStatistics *downloadRate; //!< The download rate statistics
	RateStatistics *uploadRate;   //!< The upload rate statistics

	friend class Singleton<TrafficManager>;
};

}
#endif
