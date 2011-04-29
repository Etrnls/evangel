/***************************************************************************
 *   Copyright (C) 2011 by Etrnls                                          *
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
#include "webservicestatic.h"

#include <QxtWebErrorEvent>
#include <QxtWebPageEvent>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>

namespace UserInterface
{

const char WebServiceStatic::pathPrefix[] = ":/web/static/";
const char WebServiceStatic::defaultFile[] = "evangel.html";

WebServiceStatic::WebServiceStatic(QxtAbstractWebSessionManager *sessionManager, QObject *parent, const QString &path)
	: WebServiceAuthentication(sessionManager, parent), path(path)
{
}

void WebServiceStatic::pageRequestedEvent(QxtWebRequestEvent *event)
{
	WebServiceAuthentication::pageRequestedEvent(event);
}

QxtWebEvent *WebServiceStatic::createResponseEvent(QxtWebRequestEvent *event)
{
	const QString &path = QLatin1String(pathPrefix) + (event->url.path() == QLatin1String("/") ? QLatin1String(defaultFile) : event->url.path());
	QFile *file = new QFile(path);
	QFileInfo fileInfo(*file);
	if (fileInfo.exists() && fileInfo.isFile() && file->open(QFile::ReadOnly))
	{
		QxtWebPageEvent *result = new QxtWebPageEvent(event->sessionID, event->requestID, file);
		result->streaming = false;
		result->contentType = guessContentType(fileInfo.suffix());
		return result;
	}
	else
		return new QxtWebErrorEvent(event->sessionID, event->requestID, 404, "Not Found");
}

QByteArray WebServiceStatic::guessContentType(const QString &fileSuffix)
{
	if (fileSuffix == QLatin1String("html"))
		return "text/html";
	else if (fileSuffix == QLatin1String("css"))
		return "text/css";
	else if (fileSuffix == QLatin1String("js"))
		return "text/javascript";
	else if (fileSuffix == QLatin1String("gif"))
		return "image/gif";
	else if (fileSuffix == QLatin1String("png"))
		return "image/png";
	else if (fileSuffix == QLatin1String("ico"))
		return "image/ico";
	else if (fileSuffix == QLatin1String("jpg") || fileSuffix == QLatin1String("jpeg"))
		return "image/jpeg";
	else
		return QByteArray();
}

}
