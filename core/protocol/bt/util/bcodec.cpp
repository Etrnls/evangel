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
#include "bcodec.h"

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \brief Encodes integer
 *
 * Integers are represented by an 'i' followed by the number in base 10
 * followed by an 'e'.
 *
 * For example i3e corresponds to 3 and i-3e corresponds to -3.
 *
 * Integers have no size limitation.
 *
 * i-0e is invalid. All encodings with a leading zero, such as i03e, are invalid,
 * other than i0e, which of course corresponds to 0.
 *
 * \param integer The integer to be encoded.
 */
QByteArray Bcodec::encode(const BInteger &integer)
{
	return 'i' + QByteArray::number(integer) + 'e';
}

/*!
 * \brief Encodes string
 *
 * Strings are size-prefixed base ten followed by a colon and the string.
 *
 * For example 4:spam corresponds to 'spam'.
 *
 * \param string The string to be encoded.
 */
QByteArray Bcodec::encode(const BString &string)
{
	return QByteArray::number(string.size()) + ':' + string;
}

/*!
 * \brief Encodes list
 *
 * Lists are encoded as an 'l' followed by their elements (also bencoded)
 * followed by an 'e'.
 *
 * For example l4:spam4:eggse corresponds to ['spam', 'eggs'].
 *
 * \param list The list to be encoded.
 */
QByteArray Bcodec::encode(const BList &list)
{
	QByteArray result;
	foreach(const QVariant &item, list)	result.append(encode(item));

	return 'l' + result + 'e';
}


/*!
 * \brief Encodes dictionary
 *
 * Dictionaries are encoded as a 'd' followed by a list of alternating
 * keys and their corresponding values followed by an 'e'.
 *
 * For example, d3:cow3:moo4:spam4:eggse corresponds to {'cow': 'moo', 'spam': 'eggs'}
 * and d4:spaml1:a1:bee corresponds to {'spam': ['a', 'b']}.
 *
 * Keys must be strings and appear in sorted order
 * (sorted as raw strings, not alphanumerics).
 *
 * \param dictionary The dictionary to be encoded.
 */
QByteArray Bcodec::encode(const BDictionary &dictionary)
{
	QByteArray result;
	for (auto iter = dictionary.constBegin(); iter != dictionary.constEnd(); ++iter)
		result.append(encode(iter.key())).append(encode(iter.value()));

	return 'd' + result + 'e';
}


/*!
 * \brief Decodes the content
 *
 * \param content The content to be decoded.
 * \return True if decode succeeded.
 */
bool Bcodec::decode(const QByteArray &content)
{
	if (content.isEmpty())
	{
		error = QLatin1String("The content is empty");
		return false;
	}
	this->content = content;
	index = 0;

	return decode(&dictionary);
}


/*!
 * \brief Helper function to encode variant type.
 *
 * \param variant The variant to be encoded.
 */
QByteArray Bcodec::encode(const QVariant &variant)
{
	if (variant.userType() == qMetaTypeId<BInteger>())
		return encode(variant.value<BInteger>());
	else if (variant.userType() == qMetaTypeId<BString>())
		return encode(variant.value<BString>());
	else if (variant.userType() == qMetaTypeId<BList>())
		return encode(variant.value<BList>());
	else if (variant.userType() == qMetaTypeId<BDictionary>())
		return encode(variant.value<BDictionary>());
	else
		return QByteArray();
}

/*!
 * \brief Decodes integer
 *
 * \param integer Store the decoded result if decode succeeded,
 * not modified other wise.
 * \return True if decode succeeded.
 */
bool Bcodec::decode(BInteger *integer)
{
	// Validate the starting 'i'
	if (content[index] != 'i')
	{
		error = QString::fromLatin1("'i' expected but '%1' found at pos %2").arg(content[index]).arg(index);
		return false;
	}

	const int originIndex = index;
	++index;

	qint64 result = -1;
	bool negative = false;

	// Check whether it is negative
	if (!QChar::fromLatin1(content[index]).isDigit())
	{
		if (content[index] == '-')
		{
			negative = true;
			++index;
		}
		else
		{
			error = QString::fromLatin1("'-' or digit expected but '%1' found at pos %2").arg(content[index]).arg(index);
			index = originIndex;
			return false;
		}
	}

	const int contentSize = content.size();
	for (; index < contentSize; ++index)
	{
		const char c = content[index];
		if (QChar::fromLatin1(c).isDigit())
		{
			// Check leading zero
			if (result == 0)
			{
				error = QString::fromLatin1("Leading zero(s) found at pos %1").arg(index);
				index = originIndex;
				return false;
			}
			if (result == -1) result = 0;
			result *= 10;
			result += QChar::fromLatin1(c).digitValue();
		}
		else if (c == 'e')
		{
			++index;
			break;
		}
		else
		{
			error = QString::fromLatin1("'e' or digit expected but '%1' found at pos %2").arg(c).arg(index);
			index = originIndex;
			return false;
		}
	}


	// Check "i-0e"
	if (result == 0 && negative)
	{
		error = QString::fromLatin1("'-0' found at pos %1").arg(originIndex);
		index = originIndex;
		return false;
	}

	*integer = negative ? -result : result;
	return true;
}

/*!
 * \brief Decodes string
 *
 * \param string Store the decoded result if decode succeeded,
 * not modified other wise.
 * \return True if decode succeeded.
 */
bool Bcodec::decode(BString *string)
{
	// Make sure it is starting with digit
	if (!QChar::fromLatin1(content[index]).isDigit())
	{
		error = QString::fromLatin1("digit expected but '%1' found at pos %2").arg(content[index]).arg(index);
		return false;
	}

	const int originIndex = index;
	const int contentSize = content.size();
	int resultSize = -1;

	// Get the size of the string
	for (; index < contentSize; ++index)
	{
		const char c = content[index];

		if (QChar::fromLatin1(c).isDigit())
		{
			if (resultSize == -1) resultSize = 0;
			resultSize *= 10;
			resultSize += QChar::fromLatin1(c).digitValue();
		}
		else if (c == ':')
		{
			++index;
			break;
		}
		else
		{
			error = QString::fromLatin1("':' or digit expected but '%1' found at pos %2").arg(c).arg(index);
			index = originIndex;
			return false;
		}
	}

	if (resultSize == -1 || index + resultSize > contentSize)
	{
		error = QString::fromLatin1("Invalid string size : %1").arg(resultSize);
		index = originIndex;
		return false;
	}

	*string = content.mid(index, resultSize);
	index += resultSize;
	return true;
}


/*!
 * \brief Decodes list
 *
 * \param list Store the decoded result if decode succeeded,
 * not modified other wise.
 * \return True if decode succeeded.
 */
bool Bcodec::decode(BList *list)
{
	// Validate the starting 'l'
	if (content[index] != 'l')
	{
		error = QString::fromLatin1("'l' expected but '%1' found at pos %2").arg(content[index]).arg(index);
		return false;
	}

	const int originIndex = index;
	++index;

	Bcodec::BList result;
	const int contentSize = content.size();

	while (index < contentSize)
	{
		// The end of the list
		if (content[index] == 'e')
		{
			++index;
			break;
		}

		BInteger integer;
		BString string;
		BList list;
		BDictionary dicrionary;

		if (decode(&integer))
			result.append(QVariant::fromValue(integer));
		else if (decode(&string))
			result.append(QVariant::fromValue(string));
		else if (decode(&list))
			result.append(QVariant::fromValue(list));
		else if (decode(&dictionary))
			result.append(QVariant::fromValue(dictionary));
		else
		{
			index = originIndex;
			return false;
		}
	}

	*list = result;
	return true;
}


/*!
 * \brief Decodes dictionary
 *
 * \param dictionary Store the decoded result if decode succeeded,
 * not modified other wise.
 * \return True if decode succeeded.
 */
bool Bcodec::decode(BDictionary *dictionary)
{
	// Validate the starting 'd'
	if (content[index] != 'd')
	{
		error = QString::fromLatin1("'d' expected but '%1' found at pos %2").arg(content[index]).arg(index);
		return false;
	}

	const int originIndex = index;
	++index;

	BDictionary result;
	const int contentSize = content.size();

	while (index < contentSize)
	{
		// The end of the dictionary
		if (content[index] == 'e')
		{
			++index;
			break;
		}

		// Decode the key
		BString key;
		if (!decode(&key))
		{
			index = originIndex;
			return false;
		}

		if (key == "info") infoStart = index;

		// Decode the value
		BInteger integer;
		BString string;
		BList list;
		BDictionary dictionary;

		if (decode(&integer))
			result.insert(key, QVariant::fromValue(integer));
		else if (decode(&string))
			result.insert(key, QVariant::fromValue(string));
		else if (decode(&list))
			result.insert(key, QVariant::fromValue(list));
		else if (decode(&dictionary))
			result.insert(key, QVariant::fromValue(dictionary));
		else
		{
			index = originIndex;
			return false;
		}

		if (key == "info") infoSize = index - infoStart;
	}

	*dictionary = result;
	return true;
}

}

}
