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
#include "telnetsocket.h"

#include "taskmanager.h"
#include "abstracttask.h"
#include "network/trafficmanager.h"
#include "util/humanreadable.h"

#include <QCoreApplication>

namespace UserInterface
{

const char TelnetSocket::newLine[] = {CR, LF};
const char TelnetSocket::subNegotiationBegin[] = {IAC, SB};
const char TelnetSocket::subNegotiationEnd[] = {IAC, SE};

/*!
 * \brief Constructs a TelnetSocket object
 */
TelnetSocket::TelnetSocket(int socketDescriptor) : TCPSocket(PrivilegePriority)
{
	setSocketDescriptor(socketDescriptor);
	connect(this, SIGNAL(disconnected()), SLOT(deleteLater()));
	show(color(tr("Welcome to %1\n").arg(QCoreApplication::applicationName()), Yellow));
}

/*!
 * \brief Processes the incoming data and pass it to the console
 */
void TelnetSocket::processIncomingData()
{
	processIAC();

	while (canReadLine())	execute(QLatin1String(readLine().trimmed().constData()));
}

void TelnetSocket::processIAC()
{
	QByteArray data = read(bytesAvailable());

	while (!data.isEmpty() && IAC == static_cast<uchar>(data[0]))
	{
		switch (static_cast<uchar>(data[1]))
		{
			case WILL:
			case WONT:
			case DO:
			case DONT:
				if (data.size() < 3)
				{
					unread(data);
					return;
				}
				else
					data.remove(0, 3);
				break;
			case SB:
			{
				int p = data.indexOf(QByteArray(subNegotiationEnd, sizeof(subNegotiationEnd)));
				if (p == -1)
				{
					unread(data);
					return;
				}
				else
					data.remove(0, p + 2);
			}
				break;
			default:
				break;
		}
	}
	unread(data);
}

/*!
 * \brief Executes a command from telnet interface
 */
void TelnetSocket::execute(const QString &command)
{
	QString arguments;
	Console::Command cmd = parseCommand(command, arguments);

	switch (cmd)
	{
	case Console::Empty:
		show(QString());
		break;
	case Console::Help:
		show(helpMessage(arguments == QLatin1String("0")));
		break;
	case Console::Quit:
		show(quitMessage(), false);
		disconnectFromHost();
		break;
	case Console::Kill:
		show(quitMessage(), false);
		disconnectFromHost();
		Console::execute(Console::Kill);
		break;
	case Console::Download:
	{
		QVariantList arg;

		arg.append(QVariant::fromValue(arguments));
		arg.append(QVariant::fromValue(true));

		if (Console::execute(Console::Download, arg))
			show(color(tr("Link '%1' added").arg(arguments), Cyan));
		else
			show(color(tr("Failed to add link '%1'").arg(arguments), Red));
	}
		break;
	case Console::StartTask:
	case Console::PauseTask:
	case Console::StopTask:
	case Console::DeleteTask:
	{
		QVariantList arg;
		arg.append(QVariant::fromValue(arguments.toInt()));

		if (Console::execute(cmd, arg))
			show(color(tr("Done"), Cyan));
		else
			show(color(tr("Invalid task ID : '%1'").arg(arguments.toInt()), Red));
	}
		break;
	case Console::GetProperty:
	{
		QVariantList arg;
		QVariant result;

		arg.append(QVariant::fromValue(arguments));

		if (Console::execute(cmd, arg, &result))
			show(color(tr("%1 : %2").arg(arguments).arg(result.toString()), Cyan));
		else
			show(color(tr("Invalid property : '%1'").arg(arguments), Red));
	}
		break;
	case Console::SetProperty:
	{
		QVariantList arg;

		arg.append(QVariant::fromValue(arguments.section(QLatin1Char(' '), 0, 0)));
		arg.append(QVariant::fromValue(arguments.section(QLatin1Char(' '), 1, 1).toInt()));

		if (Console::execute(cmd, arg))
			show(color(tr("Done"), Cyan));
		else
			show(color(tr("Invalid property : '%1'").arg(arguments), Red));
	}
		break;
	case Console::Status:
	{
		/// \todo ugly here

		QStringList title;
		QList<QStringList> list;

		title.append(tr("ID"));
		title.append(tr("State"));
		title.append(tr("File"));
		title.append(tr("%"));
		title.append(tr("Size"));
		title.append(tr("DownloadTime"));

		const TaskManager::Tasks &tasks = TaskManager::getInstance()->getTasks();
		for (auto iter = tasks.constBegin(); iter != tasks.constEnd(); ++iter)
		{
			QStringList item;
			item.append(QString::number(iter.key()));
			item.append(iter.value()->getStateString());
			item.append(iter.value()->getName());
			item.append(Utility::HumanReadable::formatPercent(iter.value()->getProgress()));
			item.append(Utility::HumanReadable::formatByte(iter.value()->getSize()));
			item.append(QString::number(iter.value()->getTransferTime()));

			list.append(item);
		}


		// get the max width for each column
		QList<int> width;
		foreach (const QString &columnName, title)
			width.append(columnName.size());

		foreach (const QStringList &item, list)
			for (int i = 0; i < item.size(); ++i)
				width[i] = qMax(width[i], item[i].size());

		// padding
		for (int i = 0; i < width.size(); ++i)	width[i] += 3;


		QString result;

		// title
		for (int i = 0; i < title.size(); ++i)
			result.append(title[i].rightJustified(width[i]));
		result = color(result, Cyan);
		result.append(QLatin1Char('\n'));

		// items
		foreach (const QStringList &item, list)
		{
			for (int i = 0; i < item.size(); ++i)
				result.append(item[i].rightJustified(width[i]));
			result.append(QLatin1Char('\n'));
		}

		// global statistics
		result.append(tr("Download Rate : %1/s Upload Rate : %2/s")
				.arg(Utility::HumanReadable::formatByte(Network::TrafficManager::getInstance()->getDownloadRate()))
				.arg(Utility::HumanReadable::formatByte(Network::TrafficManager::getInstance()->getUploadRate())));

		show(result);
	}
		break;
	default:
		show(color(tr("Invalid command '%1'\n\n").arg(command), Red) + helpMessage(true));
		break;
	}
}

void TelnetSocket::show(const QString &content, bool showPrompt)
{
	QByteArray data = (content + QLatin1Char('\n')).toUtf8();
	data.replace('\n', QByteArray(newLine, sizeof(newLine)));
	write(data);

	if (showPrompt)	prompt();
}

/*!
 * \brief Shows the prompt
 */
void TelnetSocket::prompt()
{
	write(QByteArray(newLine, sizeof(newLine)));
	write(color(QCoreApplication::applicationName(), Green).toUtf8());
	write(color(QLatin1String(" > "), Blue).toUtf8());
}

/*!
 * \brief Parses the command
 * \param command The command string to be parsed.
 * \param arguments The arguments of the command.
 */
Console::Command TelnetSocket::parseCommand(const QString &command, QString &arguments)
{
	/// \todo Ugly here
	arguments.clear();
	if (command.isEmpty())
		return Console::Empty;
	else if (command == QLatin1String("?") || command == QLatin1String("h"))
	{
		arguments = QLatin1String("0");
		return Console::Help;
	}
	else if (command == QLatin1String("??") || command == QLatin1String("help"))
	{
		arguments = QLatin1String("1");
		return Console::Help;
	}
	else if (command == QLatin1String("q") || command == QLatin1String("quit"))
		return Console::Quit;
	else if (command == QLatin1String("kill"))
		return Console::Kill;
	else if (command.startsWith(QLatin1String("download ")) || command.startsWith(QLatin1String("dl ")))
	{
		arguments = command.section(QLatin1Char(' '), 1, -1);
		return Console::Download;
	}
	else if (command.startsWith(QLatin1String("start ")))
	{
		bool ok;
		arguments = QString::number(command.mid(6).toInt(&ok));
		if (!ok)	return Console::Invalid;
		return Console::StartTask;
	}
	else if (command.startsWith(QLatin1String("pause ")))
	{
		bool ok;
		arguments = QString::number(command.mid(6).toInt(&ok));
		if (!ok)	return Console::Invalid;
		return Console::PauseTask;
	}
	else if (command.startsWith(QLatin1String("stop ")))
	{
		bool ok;
		arguments = QString::number(command.mid(5).toInt(&ok));
		if (!ok)	return Console::Invalid;
		return Console::StopTask;
	}
	else if (command.startsWith(QLatin1String("delete ")))
	{
		bool ok;
		arguments = QString::number(command.mid(7).toInt(&ok));
		if (!ok)	return Console::Invalid;
		return Console::DeleteTask;
	}
	else if (command.startsWith(QLatin1String("get ")))
	{
		arguments = command.section(QLatin1Char(' '), 1, 1);
		return Console::GetProperty;
	}
	else if (command.startsWith(QLatin1String("set ")))
	{
		arguments = command.section(QLatin1Char(' '), 1, 2);
		return Console::SetProperty;
	}
	else if (command == QLatin1String("s") || command == QLatin1String("status"))
		return Console::Status;
	else
		return Console::Invalid;
}

/*!
 * \brief Formats the string with specific color
 * \param string The string to be formated.
 * \param foreColor The forecolor.
 * \param backColor The backcolor.
 * \param highlight The high light flag.
 * \return The formated string.
 */
QString TelnetSocket::color(const QString &string, Color foreColor, Color backColor, bool highlight)
{
	return QString::fromLatin1("%1[%2;%3m%4%1[m").arg(ESC)
		.arg((highlight ? ForegroundColorHighIntensity : ForegroundColorNormalIntensity) + foreColor)
		.arg((highlight ? BackgroundColorHighIntensity : BackgroundColorNormalIntensity) + backColor)
		.arg(string);
}

/*!
 * \brief Retrieves the help message
 */
QString TelnetSocket::helpMessage(bool brief)
{
	QString result;
	if (brief)
		result = tr("The brief help message.\n");
	else
		result = tr("The full help message.\n");

	return result.append(tr("Use ? for brief help.\nUse ?? for full help.\n"));
}

/*!
 * \brief Retrieves the quit message
 */
QString TelnetSocket::quitMessage()
{
	return tr("Bye~");
}

}
