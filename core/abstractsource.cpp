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
#include "abstractsource.h"

#include <QString>

QList<AbstractSourceFactory*> AbstractSourceFactory::factories;

/*!
 * \brief Creates the proper Source instance
 * \param URL The URL of the source.
 * \return The pointer to the AbstractSource instance. 0 if the URL is invalid.
 */
AbstractSource* AbstractSourceFactory::createSource(const QString &URL)
{
	AbstractSource *result = 0;

	foreach (AbstractSourceFactory *factory, factories)
		if ((result = factory->create(URL)))
			break;

	return result;
}

