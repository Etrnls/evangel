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
#ifndef PROTOCOL_HTTP_HTTPSOCKET_H
#define PROTOCOL_HTTP_HTTPSOCKET_H

#include "network/abstractsocket.h"

#include <QSharedPointer>
#include <QUrl>


class QNetworkAccessManager;
class QNetworkReply;

namespace Protocol
{

namespace HTTP
{

class HTTPClient;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class HTTPSocket : public Network::AbstractSocket
{
	Q_OBJECT
public:
	HTTPSocket(const QUrl &URL);
	virtual ~HTTPSocket();

	void head();
	void get(qint64 offset);

	// information
	virtual State getState() const;

	// traffic control
	virtual void setNetworkReadBufferSize(int size);

	virtual bool canDownload() const;
	virtual bool canUpload() const;

	virtual int download(int maxSize = INT_MAX);
	virtual int upload(int maxSize = INT_MAX);
signals:
	void headReceived() const;
	void blockReceived(qint64 offset, const QByteArray &data) const;
private slots:
	void processHead() const;
private:
	static QWeakPointer<QNetworkAccessManager> accessManager;

	const QUrl URL;

	QSharedPointer<QNetworkAccessManager> manager;
	QNetworkReply *reply;

	qint64 offset;
	QByteArray incomingBuffer;

	virtual void processIncomingData();
};

}

}

#endif
