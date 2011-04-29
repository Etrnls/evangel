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
#ifndef PROTOCOL_HTTP_HTTPCLIENT_H
#define PROTOCOL_HTTP_HTTPCLIENT_H

#include <QObject>
#include <QList>
#include <QUrl>

namespace Protocol
{

namespace HTTP
{

class HTTPTransfer;
class HTTPSocket;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class HTTPClient : public QObject
{
	Q_OBJECT
public:
	HTTPClient(HTTPTransfer *transfer);
	virtual ~HTTPClient();
private slots:
	void start();
	void processHead();
	void processBlock(qint64 offset, const QByteArray &data);
private:
	HTTPTransfer * const transfer;
	HTTPSocket * const socket;

	QPair<qint64, qint64> select();
};

}

}

#endif
