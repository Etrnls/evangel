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
#include "log.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMetaClassInfo>
#include <QDateTime>
#include <QCoreApplication>

Log::Log()
{
	QFile *file = new QFile(QFileInfo(QDir(QCoreApplication::applicationDirPath()),
						QLatin1String("log")).absoluteFilePath());
	file->open(QFile::WriteOnly | QFile::Truncate | QFile::Text | QFile::Unbuffered);
	stream.setDevice(file);
}

Log::~Log()
{
	delete stream.device();
}

Log& log(const QObject *object)
{
	const QMetaObject *metaObject = object->metaObject();

	Log::getInstance()->stream << '[' << QDateTime::currentDateTime().toString(Qt::ISODate) << ']'
				<< '[' << metaObject->classInfo(metaObject->indexOfClassInfo("log")).value() << ']';

	return *Log::getInstance();
}

