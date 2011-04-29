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
#include "webserver.h"
#include "webservicestatic.h"

#include "util/settings.h"

#include <QxtWebServiceDirectory>

namespace UserInterface
{

void WebServer::initialize()
{
	setConnector(HttpServer);
	setPort(Utility::Settings::load(this, serverName() + QLatin1String("Port"), defaultPort()).value<quint16>());

	QxtWebServiceDirectory *topDirectory = new QxtWebServiceDirectory(this, this);
	WebServiceStatic *staticDirectory = new WebServiceStatic(this, topDirectory, QLatin1String("static"));
	topDirectory->addService(QLatin1String("static"), staticDirectory);
	topDirectory->setDefaultRedirect(QLatin1String("static"));

	setStaticContentService(topDirectory);

	start();
}

}
