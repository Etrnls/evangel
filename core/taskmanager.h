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
#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "singleton.h"

#include <QHash>
#include <QDir>
#include <QObject>

class AbstractSource;
class AbstractTask;

/*!
 * \brief Manages all download tasks
 * \author Etrnls <Etrnls@gmail.com>
 */
class TaskManager : public QObject, public Singleton<TaskManager>
{
	Q_OBJECT
	Q_CLASSINFO("log", "TaskManager")
public:
	enum StartOption
	{
		AutomaticallyStart,
		ManuallyStart
	};

	typedef QHash<int, AbstractTask*> Tasks; //!< Stores <ID, Task>

	bool createTasks(AbstractSource *source, StartOption startOption, QDir savePath = QDir());

	inline const Tasks& getTasks() const
	{
		return tasks;
	}
public slots:
	void startTask(int ID) const;
	void pauseTask(int ID) const;
	void stopTask(int ID) const;
	void deleteTask(int ID);
signals:
	void tasksAdded(const Tasks &tasks) const;
private:
	TaskManager();
	virtual ~TaskManager();

	int nextTaskID;

	Tasks tasks; //!< Stores the [ID, task]

	friend class Singleton<TaskManager>;
};

#endif
