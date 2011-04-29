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
#include "ftpclient.h"
#include "ftpsocket.h"
#include "ftptransfer.h"

namespace Protocol
{

namespace FTP
{

FTPClient::FTPClient(FTPTransfer *transfer) : transfer(transfer), socket(new FTPSocket(transfer->getURL()))
{
	connect(socket, SIGNAL(blockReceived(qint64,QByteArray)), SLOT(processBlock(qint64, QByteArray)));

	connect(socket, SIGNAL(featureListReceived()), SLOT(processFeatureList()));
	socket->feat();
}

FTPClient::~FTPClient()
{
	delete socket;
}

void FTPClient::start()
{
	/// \todo select a proper offset to get
	//socket->get(transfer->getURL(), offset);
}

void FTPClient::processFeatureList()
{
	disconnect(socket, SIGNAL(featureListReceived()), this, SLOT(processFeatureList()));

	if (socket->supportSIZE())
	{
		connect(socket, SIGNAL(sizeReceived(qint64)), SLOT(processSize()));
		socket->size();
	}
	else
		start();
}

void FTPClient::processSize()
{
	disconnect(socket, SIGNAL(sizeReceived(qint64)), this, SLOT(processSize()));
	start();
}

void FTPClient::processBlock(qint64 offset, const QByteArray &data)
{
	transfer->updateBlock(Utility::BlockDownloaded, Utility::Block(offset, data));
}


}

}
