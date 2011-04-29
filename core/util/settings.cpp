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
#include "settings.h"

#include <QSettings>

namespace Utility
{

namespace Settings
{

inline QString getPrefix(const QObject *object)
{
	QString result = QString::fromLatin1(object->metaObject()->className())
				.section(QLatin1String("::"), 0, -2);
	return result.isEmpty() ? QLatin1String("global")
				: result.replace(QLatin1String("::"), QLatin1String("/")).toLower();
}

void save(const QObject *object, const QString &key, const QVariant & value)
{
	QSettings settings;

	settings.beginGroup(getPrefix(object));
	settings.setValue(key, value);
}

QVariant load(const QObject *object, const QString &key, const QVariant &defaultValue)
{
	QSettings settings;

	settings.beginGroup(getPrefix(object));
	return settings.value(key, defaultValue);
}

}

}
