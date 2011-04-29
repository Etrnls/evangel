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
#ifndef CONSOLE_H
#define CONSOLE_H

#include "singleton.h"

#include <QVariant>
#include <QObject>

/*!
 * \brief The control center
 * \author Etrnls <Etrnls@gmail.com>
 */
class Console : public QObject, public Singleton<Console>
{
	Q_OBJECT
	Q_CLASSINFO("log", "Console")
public:
	enum Command
	{
		Empty,
		Help,
		Quit,
		Kill,

		Download,

		StartTask,
		PauseTask,
		StopTask,
		DeleteTask,

		GetProperty,
		SetProperty,

		Status,

		Invalid
	};

	Q_INVOKABLE void start();
public:
	static bool execute(Command command, const QVariantList &arguments = QVariantList(), QVariant *result = 0);

	friend class Singleton<Console>;
};

#endif
