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
#include "console.h"
#include "abstractsource.h"
#include "taskmanager.h"
#include "future/jobmanager.h"
#include "disk/performancemanager.h"
#include "network/trafficmanager.h"
#include "ui/uimanager.h"
#include "log.h"

#include <QCoreApplication>

void Console::start()
{
	// KEEP ORDERED
	Log::create();

	Disk::PerformanceManager::create();
	Network::TrafficManager::create();
	Future::JobManager::create();

	TaskManager::create();

	UserInterface::UIManager::create();

	log(this) << QCoreApplication::applicationName() << "started" << endl;
}

/*!
 * \brief Executes a command
 * \param command The command to be executed.
 * \param arguments The arguments for the command.
 * \param result The result of the command.
 */
bool Console::execute(Command command, const QVariantList &arguments, QVariant *result)
{
	switch (command)
	{
	case Kill:
		QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
		break;
	case Download:
		Q_ASSERT_X(arguments.size() == 2, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<QString>(), Q_FUNC_INFO, "Invalid argument 0");
		Q_ASSERT_X(arguments[1].userType() == qMetaTypeId<bool>(), Q_FUNC_INFO, "Invalid argument 1");

	{
		AbstractSource *source = AbstractSourceFactory::createSource(arguments[0].toString());
		if (!source)	return false;

		if (TaskManager::getInstance()->createTasks(source, arguments[1].toBool() ? TaskManager::AutomaticallyStart : TaskManager::ManuallyStart))
			break;

		delete source;
		return false;
	}
		break;
	case StartTask:
		Q_ASSERT_X(arguments.size() == 1, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<int>(), Q_FUNC_INFO, "Invalid argument 0");

		TaskManager::getInstance()->startTask(arguments[0].toInt());
		break;
	case PauseTask:
		Q_ASSERT_X(arguments.size() == 1, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<int>(), Q_FUNC_INFO, "Invalid argument 0");

		TaskManager::getInstance()->pauseTask(arguments[0].toInt());
		break;
	case StopTask:
		Q_ASSERT_X(arguments.size() == 1, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<int>(), Q_FUNC_INFO, "Invalid argument 0");

		TaskManager::getInstance()->stopTask(arguments[0].toInt());
		break;
	case DeleteTask:
		Q_ASSERT_X(arguments.size() == 1, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<int>(), Q_FUNC_INFO, "Invalid argument 0");

		TaskManager::getInstance()->deleteTask(arguments[0].toInt());
		break;
	case GetProperty:
		Q_ASSERT_X(arguments.size() == 1, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<QString>(), Q_FUNC_INFO, "Invalid argument 0");
		Q_ASSERT_X(result, Q_FUNC_INFO, "Invalid result pointer");

	{
		const QString &propertyName = arguments[0].toString();
		if (propertyName == QLatin1String("downloadLimit"))
			*result = Network::TrafficManager::getInstance()->getDownloadLimit();
		else if (propertyName == QLatin1String("uploadLimit"))
			*result = Network::TrafficManager::getInstance()->getUploadLimit();
	}
		break;
	case SetProperty:
		Q_ASSERT_X(arguments.size() == 2, Q_FUNC_INFO, "Too few/many arguments");
		Q_ASSERT_X(arguments[0].userType() == qMetaTypeId<QString>(), Q_FUNC_INFO, "Invalid argument 0");
		Q_ASSERT_X(arguments[1].userType() == qMetaTypeId<int>(), Q_FUNC_INFO, "Invalid argument 1");

	{
		const QString &propertyName = arguments[0].toString();
		if (propertyName == QLatin1String("downloadLimit"))
			Network::TrafficManager::getInstance()->setDownloadLimit(arguments[1].toInt());
		else if (propertyName == QLatin1String("uploadLimit"))
			Network::TrafficManager::getInstance()->setUploadLimit(arguments[1].toInt());
	}
		break;
	default:
		return false;
	}
	return true;
}

