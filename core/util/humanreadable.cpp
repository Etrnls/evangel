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
#include "humanreadable.h"

#include <QCoreApplication>

namespace Utility
{

namespace HumanReadable
{

/*!
 * \brief Formats size in byte to human readable
 * \param bytes The total bytes
 */
QString formatByte(qint64 bytes)
{
	static const qint64 KiBBound = 1024;
	static const qint64 MiBBound = KiBBound * 1024;
	static const qint64 GiBBound = MiBBound * 1024;

	if (bytes >= GiBBound)
		return QCoreApplication::translate("HumanReadable", "%1 GiB").arg(static_cast<qreal>(bytes) / GiBBound, 0, 'f', 2);
	else if (bytes >= MiBBound)
		return QCoreApplication::translate("HumanReadable", "%1 MiB").arg(static_cast<qreal>(bytes) / MiBBound, 0, 'f', 2);
	else if (bytes >= KiBBound)
		return QCoreApplication::translate("HumanReadable", "%1 KiB").arg(static_cast<qreal>(bytes) / KiBBound, 0, 'f', 2);
	else
		return QCoreApplication::translate("HumanReadable", "%1 Byte").arg(bytes);
}


/*!
 * \brief Formats time in second to human readable
 * \param seconds The total seconds
 */
QString formatTime(qint64 seconds)
{
	static const qint64 minuteBound = 60;
	static const qint64 hourBound = minuteBound * 60;
	static const qint64 dayBound = hourBound * 24;

	if (seconds >= dayBound)
		return QCoreApplication::translate("HumanReadable", "%1 day(s)").arg(static_cast<qreal>(seconds) / dayBound, 0, 'f', 2);
	else if (seconds >= hourBound)
		return QCoreApplication::translate("HumanReadable", "%1 hour(s)").arg(static_cast<qreal>(seconds) / hourBound, 0, 'f', 2);
	else if (seconds >= minuteBound)
		return QCoreApplication::translate("HumanReadable", "%1 minute(s)").arg(static_cast<qreal>(seconds) / minuteBound, 0, 'f', 2);
	else
		return QCoreApplication::translate("HumanReadable", "%1 second(s)").arg(seconds);
}

QString formatPercent(qreal percent)
{
	return QString::number(percent * 100, 'f', 2);
}

}

}
