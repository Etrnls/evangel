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
#include "httpclient.h"
#include "httpsocket.h"
#include "httptransfer.h"

namespace Protocol
{

namespace HTTP
{

HTTPClient::HTTPClient(HTTPTransfer *transfer) : transfer(transfer), socket(new HTTPSocket(transfer->getURL()))
{
	connect(socket, SIGNAL(blockReceived(qint64,QByteArray)), SLOT(processBlock(qint64, QByteArray)));

	connect(socket, SIGNAL(headReceived()), SLOT(processHead()));
	socket->head();
}

HTTPClient::~HTTPClient()
{
	delete socket;
}

void HTTPClient::start()
{
	/// \todo implements me
}

void HTTPClient::processHead()
{
	disconnect(socket, SIGNAL(headReceived()), this, SLOT(processHead()));
	start();
}

void HTTPClient::processBlock(qint64 offset, const QByteArray &data)
{
	transfer->updateBlock(Utility::BlockDownloaded, Utility::Block(offset, data));
}

QPair<qint64, qint64> HTTPClient::select()
{
	Q_ASSERT_X(transfer, Q_FUNC_INFO, "Invalid transfer");

	const auto &bitmap = transfer->bitmap();
	if (bitmap.size() == 0)
	{
		// we don't know the size yet
		return qMakePair(Q_INT64_C(0), Q_INT64_C(0));
	}

	// pick the largest unrequested segment
	const auto &segments = bitmap.segments(Utility::Bitmap::getStatusChecker(Utility::Bitmap::EmptyStatus));
	qint64 offset = 0, size = 0;
	foreach (const auto &segment, segments)
		if (segment.size() > size)
		{
			offset = segment.begin;
			size = segment.size();
		}

	return qMakePair(offset, size);
}


}

}
