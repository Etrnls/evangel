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
#ifndef PROTOCOL_FTP_FTPSOCKET_H
#define PROTOCOL_FTP_FTPSOCKET_H

#include "network/abstractsocket.h"

#include <QSharedPointer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace Protocol
{

namespace FTP
{

class FTPClient;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class FTPSocket : public Network::AbstractSocket
{
	Q_OBJECT
public:
	FTPSocket(const QUrl &URL);
	virtual ~FTPSocket();

	void feat();
	void size();
	void get(qint64 offset);

	// information
	bool supportSIZE() const;
	bool supportREST() const;
	virtual State getState() const;

	// traffic control
	virtual void setNetworkReadBufferSize(int size);

	virtual bool canDownload() const;
	virtual bool canUpload() const;

	virtual int download(int maxSize = INT_MAX);
	virtual int upload(int maxSize = INT_MAX);
signals:
	void featureListReceived() const;
	void sizeReceived(qint64 size) const;
	void blockReceived(qint64 offset, const QByteArray &data) const;
private slots:
	void processMetaData() const;
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
