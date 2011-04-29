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
#ifndef PROTOCOL_HTTP_HTTPSOURCE_H
#define PROTOCOL_HTTP_HTTPSOURCE_H

#include "abstractsource.h"

#include <QString>

namespace Protocol
{

namespace HTTP
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class HTTPSource : public AbstractSource
{
public:
	explicit HTTPSource(const QString &URL) : URL(URL) {}

	virtual AbstractTransfer *createTransfer() const;
private:
	const QString URL;
};


}

}

#endif
