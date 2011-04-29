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
#include "ftpsocket.h"
#include "ftpclient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Protocol
{

namespace FTP
{

QWeakPointer<QNetworkAccessManager> FTPSocket::accessManager;

FTPSocket::FTPSocket(const QUrl &URL) : AbstractSocket(NormalPriority), URL(URL), manager(accessManager), reply(0)
{
	if (!manager)
	{
		manager = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());
		accessManager = manager;
	}
}

FTPSocket::~FTPSocket()
{
	detachTrafficControl();
	delete reply;
}

void FTPSocket::feat()
{

}

void FTPSocket::size()
{

}

void FTPSocket::get(qint64 offset)
{
	Q_ASSERT_X(getState() == UnconnectedState, Q_FUNC_INFO, "Already started");

	/// \todo offset support, see also supportREST()
	Q_ASSERT_X(offset == 0, Q_FUNC_INFO, "Not supported");

	//this->offset = offset;
	//reply = manager->get(QNetworkRequest(URL));
	//connect(reply, SIGNAL(metaDataChanged()), SLOT(processMetaData()));

	attachTrafficControl();
}

bool FTPSocket::supportSIZE() const
{
	return false;
}

bool FTPSocket::supportREST() const
{
	/// \todo REST support, see also get()
	return false;
}

FTPSocket::State FTPSocket::getState() const
{
	return reply ? ConnectedState : UnconnectedState;
}

void FTPSocket::setNetworkReadBufferSize(int size)
{
	Q_ASSERT_X(getState() == ConnectedState, Q_FUNC_INFO, "Not started yet");
	reply->setReadBufferSize(size);
}

bool FTPSocket::canDownload() const
{
	Q_ASSERT_X(getState() == ConnectedState, Q_FUNC_INFO, "Not started yet");
	return reply->bytesAvailable() > 0;
}

bool FTPSocket::canUpload() const
{
	return false;
}

int FTPSocket::download(int maxSize)
{
	Q_ASSERT_X(getState() == ConnectedState, Q_FUNC_INFO, "Not started yet");

	maxSize = qMin<qint64>(maxSize, reply->bytesAvailable());
	if (maxSize == 0)	return 0;

	const int oldSize = incomingBuffer.size();
	incomingBuffer.resize(incomingBuffer.size() + maxSize);

	const int result = reply->read(incomingBuffer.data() + oldSize, maxSize);
	incomingBuffer.resize(result > 0 ? oldSize + result : oldSize);

	if (result > 0)	QMetaObject::invokeMethod(this, "autoProcessIncomingData", Qt::QueuedConnection);

	return result;
}

int FTPSocket::upload(int maxSize)
{
	Q_UNUSED(maxSize);
	Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid call");
	return 0;
}

void FTPSocket::processMetaData() const
{
	QVariant size = reply->header(QNetworkRequest::ContentLengthHeader);
	if (size.isValid())	emit sizeReceived(size.value<qint64>());
}

void FTPSocket::processIncomingData()
{
	static const int threshold = 4 * 1024;
	if (incomingBuffer.size() >= threshold)
	{
		emit blockReceived(offset, incomingBuffer);
		incomingBuffer.clear();
	}
}


}

}
