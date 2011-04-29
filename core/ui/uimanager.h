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
#ifndef USERINTERFACE_UIMANAGER_H
#define USERINTERFACE_UIMANAGER_H

#include "singleton.h"

#include <QObject>
#include <QList>

/*!
 * \brief Provides various user interfaces
 */
namespace UserInterface
{

class UIServer;

/*!
 * \brief Manages all user interface servers
 * \author Etrnls <Etrnls@gmail.com>
 */
class UIManager : public QObject, public Singleton<UIManager>
{
	Q_OBJECT
	Q_CLASSINFO("log", "UIManager")
private:
	QList<UIServer*> servers;

	UIManager();
	virtual ~UIManager();

	friend class Singleton<UIManager>;
};

}

#endif
