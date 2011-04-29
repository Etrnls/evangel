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
#include "taskmanager.h"
#include "abstractsource.h"
#include "abstracttransfer.h"
#include "log.h"
#include "util/settings.h"

bool TaskManager::createTasks(AbstractSource *source, StartOption startOption, QDir savePath)
{
	AbstractTransfer *transfer = source->createTransfer();
	if (!transfer) return false;

	Tasks newTasks;
	foreach(const auto &fileInformation, transfer->files())
	{
		FileTask *task = new FileTask(QFileInfo(savePath, fileInformation.file).absoluteFilePath(),
								fileInformation.size);
		task->addTransfer(transfer, fileInformation.offset);
		newTasks.insert(nextTaskID++, task);
	}

	if (startOption == AutomaticallyStart)
		foreach (int ID, newTasks.keys())
			QMetaObject::invokeMethod(this, "startTask", Qt::QueuedConnection, Q_ARG(int, ID));

	tasks.unite(newTasks);

	emit tasksAdded(newTasks);

	return true;
}

void TaskManager::startTask(int ID) const
{
	Q_ASSERT_X(tasks.contains(ID), Q_FUNC_INFO, "Invalid task ID");

	log(this) << QString::fromLatin1("Starting task [%1]").arg(ID) << endl;

	tasks[ID]->start();
}

void TaskManager::pauseTask(int ID) const
{
	Q_ASSERT_X(tasks.contains(ID), Q_FUNC_INFO, "Invalid task ID");

	log(this) << QString::fromLatin1("Pausing task [%1]").arg(ID) << endl;

	tasks[ID]->pause();
}

void TaskManager::stopTask(int ID) const
{
	Q_ASSERT_X(tasks.contains(ID), Q_FUNC_INFO, "Invalid task ID");

	log(this) << QString::fromLatin1("Stopping task [%1]").arg(ID) << endl;

	tasks[ID]->stop();
}

void TaskManager::deleteTask(int ID)
{
	Q_ASSERT_X(tasks.contains(ID), Q_FUNC_INFO, "Invalid task ID");

	log(this) << QString::fromLatin1("Deleting task [%1]").arg(ID) << endl;

	delete tasks.take(ID);
}

TaskManager::TaskManager()
{
	log(this) << "Initializing" << endl;

	nextTaskID = Utility::Settings::load(this, QLatin1String("nextTaskID"), 1).toInt();
	/// \todo resume tasks

	log(this) << "Successfully initialized" << endl;
}

TaskManager::~TaskManager()
{
	log(this) << "Exiting" << endl;

	/// \todo save tasks for resume
	Utility::Settings::save(this, QLatin1String("nextTaskID"), nextTaskID);
	foreach (AbstractTask *task, tasks)
	{
		task->stop();
		delete task;
	}

	log(this) << "Successfully exited" << endl;
}
