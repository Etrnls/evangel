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
#include "exithandler.h"

#include <QStack>

namespace Utility
{

/*!
 * \brief Provides a exit handler
 *
 * This exit handler is similar to std::atexit,
 * but it provides safe behavior when being called recursively.
 */
namespace ExitHandler
{

QStack<ExitFunction> exitFunctions;

/*!
 * \brief Add a function which is needed to process at exit
 */
void atExit(ExitFunction exitFunction)
{
	exitFunctions.push(exitFunction);
}

/*!
 * \brief Process the exit
 */
void process()
{
	while (!exitFunctions.isEmpty())
	{
		ExitFunction	exitFunction = exitFunctions.pop();
		exitFunction();
	}
}

}

}
