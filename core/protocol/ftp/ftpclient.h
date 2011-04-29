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
#ifndef PROTOCOL_FTP_FTPCLIENT_H
#define PROTOCOL_FTP_FTPCLIENT_H

#include <QObject>
#include <QList>
#include <QUrl>

namespace Protocol
{

namespace FTP
{

class FTPTransfer;
class FTPSocket;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class FTPClient : public QObject
{
	Q_OBJECT
public:
	FTPClient(FTPTransfer *transfer);
	virtual ~FTPClient();
private slots:
	void start();
	void processFeatureList();
	void processSize();
	void processBlock(qint64 offset, const QByteArray &data);
private:
	FTPTransfer * const transfer;
	FTPSocket * const socket;
};

}

}

#endif
