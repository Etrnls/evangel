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
#ifndef UTILITY_RANDOM_H
#define UTILITY_RANDOM_H

#include <QtGlobal>
#include <cstdlib>

namespace Utility
{

//! \brief Generate random number in [0, 1]
inline qreal rand()
{
	return qrand() / static_cast<qreal>(RAND_MAX);
}

//! \brief Generate random number in [0, max)
inline qint64 rand(qint64 max)
{
	--max;
	return qBound<qint64>(0, qRound64(static_cast<qreal>(qrand()) * max / RAND_MAX), max);
}

//! \brief Generate random number in [min, max)
inline qint64 rand(qint64 min, qint64 max)
{
	return rand(max - min) + min;
}

}

#endif
