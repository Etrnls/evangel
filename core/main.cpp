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
#include "util/exithandler.h"

#include <QCoreApplication>
#include <QThreadPool>
#include <QMetaType>

Q_DECLARE_METATYPE(QVariant)

void initialize()
{
	qAddPostRoutine(Utility::ExitHandler::process);

	// property
	QCoreApplication::setApplicationName(QLatin1String("Evangel"));
	QCoreApplication::setApplicationVersion(QLatin1String("0.1.0.0"));

	QCoreApplication::setOrganizationName(QLatin1String("Gemini"));

	QCoreApplication::instance()->setProperty("UserAgent", QCoreApplication::applicationName() + QLatin1Char('/') + QCoreApplication::applicationVersion());

	// meta type
	qRegisterMetaType<QVariant>();

	// thread
	QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() * 2);

	// console
	Console::create();
	QMetaObject::invokeMethod(Console::getInstance(), "start", Qt::QueuedConnection);
}

int main(int argc, char *argv[])
{
	/// \todo add rate statistics for downloadtask
	QCoreApplication app(argc, argv);

	initialize();

	return app.exec();
}

