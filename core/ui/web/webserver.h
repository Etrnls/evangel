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
#ifndef USERINTERFACE_WEBSERVER_H
#define USERINTERFACE_WEBSERVER_H

#include "ui/uiserver.h"

#include <QxtHttpSessionManager>

namespace UserInterface
{

/*!
 * \brief Provides the web UI server
 * \author Etrnls <Etrnls@gmail.com>
 */
class WebServer : public QxtHttpSessionManager, public UIServer
{
	Q_OBJECT
	Q_CLASSINFO("log", "UserInterface")
public:
	virtual void initialize();
private:
	virtual QString serverName() const
	{
		return QLatin1String("web");
	}

	virtual quint16 defaultPort() const
	{
		return 5080;
	}
};

}

#endif
