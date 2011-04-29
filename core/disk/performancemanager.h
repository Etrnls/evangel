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
#ifndef DISK_PERFORMANCEMANAGER_H
#define DISK_PERFORMANCEMANAGER_H

#include "singleton.h"

#include <QSet>
#include <QTimerEvent>

namespace Disk
{

class File;

/*!
 * \brief Manages disk performance
 * \author Etrnls <Etrnls@gmail.com>
 */
class PerformanceManager : public QObject, public Singleton<PerformanceManager>
{
	Q_OBJECT
	Q_CLASSINFO("log", "DiskPerformanceManager")
public:
	void attach(File *file);
	void detach(File *file);
private:
	static const int performInterval = 50;

	int performTimer;

	QSet<File*> files;

	PerformanceManager();
	virtual ~PerformanceManager();

	void perform() const;

	virtual void timerEvent(QTimerEvent *event)
	{
		return event->timerId() == performTimer ? perform() : QObject::timerEvent(event);
	}

	friend class Singleton<PerformanceManager>;
};

}

#endif
