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
#include "webserviceauthentication.h"

#include "util/settings.h"

#include <QxtWebErrorEvent>
#include <QCoreApplication>

namespace UserInterface
{

WebServiceAuthentication::WebServiceAuthentication(QxtAbstractWebSessionManager *sessionManager, QObject *parent)
	: QxtAbstractWebService(sessionManager, parent)
{
}

void WebServiceAuthentication::pageRequestedEvent(QxtWebRequestEvent *event)
{
	const QString &authorizationHeader = QLatin1String("Authorization");
	if (event->headers.contains(authorizationHeader))
	{
		const QString &userName = Utility::Settings::load(this, QLatin1String("UserName"), QString::fromLatin1("Evangel")).value<QString>();
		const QString &password = Utility::Settings::load(this, QLatin1String("Password"), QString::fromLatin1("Evangel")).value<QString>();
		const QString &data = QString::fromLatin1((userName.toLatin1() + ":" + password.toLatin1()).toBase64().constData());
		if (event->headers.values(authorizationHeader).first() == QLatin1String("Basic ") + data)
		{
			postEvent(addAuthenticationHeader(createResponseEvent(event)));
			return;
		}
	}

	postEvent(addAuthenticationHeader(new QxtWebErrorEvent(event->sessionID, event->requestID, 401, "Unauthorized")));
}

QxtWebEvent *WebServiceAuthentication::addAuthenticationHeader(QxtWebEvent *event)
{
	QxtWebPageEvent *pageEvent = dynamic_cast<QxtWebPageEvent*>(event);
	if (pageEvent)
	{
		const QString &realm = QCoreApplication::applicationName();
		pageEvent->headers.insert(QLatin1String("WWW-Authenticate"), QString::fromLatin1("Basic realm=\"%1\"").arg(realm));
		return pageEvent;
	}
	return event;
}

}

