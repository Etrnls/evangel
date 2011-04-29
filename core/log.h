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
#ifndef LOG_H
#define LOG_H

#include "singleton.h"

#include <QTextStream>

/*!
 * \brief Provides log
 * \author Etrnls <Etrnls@gmail.com>
 */
class Log : public QObject, public Singleton<Log>
{
	Q_OBJECT
public:
	template <class T>
	Log& operator<<(const T &x)
	{
		stream << ' ' << x;
		return *this;
	}
private:
	QTextStream stream;

	Log();
	virtual ~Log();

	friend class Singleton<Log>;
	friend Log& log(const QObject *object);
};

/*!
 * \brief The global function to retrieve the instance of the Log
 */
Log& log(const QObject *object);

#endif
