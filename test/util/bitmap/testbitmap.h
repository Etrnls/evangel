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
#ifndef UNITTEST_UTILITY_TESTBITMAP_H
#define UNITTEST_UTILITY_TESTBITMAP_H

#include <QObject>

namespace UnitTest
{

namespace Utility
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class TestBitmap: public QObject
{
	Q_OBJECT
private slots:
	void sequence_data();
	void sequence();

	void constant_data();
	void constant();

	void simulate();
};

}

}

#endif
