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
#include "bitmap.h"

namespace Utility
{

const Bitmap::StatusChecker &Bitmap::getStatusChecker(SegmentStatus status)
{
	switch (status)
	{
	case InvalidStatus:
		Q_ASSERT_X(false, Q_FUNC_INFO, "No status checker for InvalidStatus");
		break;
	case EmptyStatus:
		return EmptyStatusChecker;
	case MaskedStatus:
		return MaskedStatusChecker;
	case RequestedStatus:
		return RequestedStatusChecker;
	case DownloadedStatus:
		return DownloadedStatusChecker;
	case VerifiedStatus:
		return VerifiedStatusChecker;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown status");
		break;
	}
	return DummyChecker;
}

const Bitmap::StatusUpdater &Bitmap::getStatusUpdater(SegmentStatus status)
{
	switch (status)
	{
	case InvalidStatus:
		Q_ASSERT_X(false, Q_FUNC_INFO, "No status updater for InvalidStatus");
		break;
	case EmptyStatus:
		return EmptyStatusUpdater;
	case MaskedStatus:
		return MaskedStatusUpdater;
	case RequestedStatus:
		return RequestedStatusUpdater;
	case DownloadedStatus:
		return DownloadedStatusUpdater;
	case VerifiedStatus:
		return VerifiedStatusUpdater;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown status");
		break;
	}
	return DummyUpdater;
}

const Bitmap::StatusChecker Bitmap::DummyChecker = [](SegmentStatus status)->bool
{
	Q_ASSERT_X(false, "DummyChecker", "Reached DummyChecker");
	Q_UNUSED(status);
	return false;
};

const Bitmap::StatusChecker Bitmap::EmptyStatusChecker = [](SegmentStatus status)->bool
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::RequestedStatus:
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		return false;
	case Bitmap::EmptyStatus:
		return true;
	default:
		Q_ASSERT_X(false, "EmptyStatusChecker", "Unknown status");
		return false;
	}
};

const Bitmap::StatusChecker Bitmap::MaskedStatusChecker = [](SegmentStatus status)->bool
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::EmptyStatus:
	case Bitmap::RequestedStatus:
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		return false;
	case Bitmap::MaskedStatus:
		return true;
	default:
		Q_ASSERT_X(false, "MaskedStatusChecker", "Unknown status");
		return false;
	}
};

const Bitmap::StatusChecker Bitmap::RequestedStatusChecker = [](SegmentStatus status)->bool
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::EmptyStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		return false;
	case Bitmap::RequestedStatus:
		return true;
	default:
		Q_ASSERT_X(false, "RequestedStatusChecker", "Unknown status");
		return false;
	}
};

const Bitmap::StatusChecker Bitmap::DownloadedStatusChecker = [](SegmentStatus status)->bool
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::EmptyStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::RequestedStatus:
		return false;
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		return true;
	default:
		Q_ASSERT_X(false, "DownloadedStatusChecker", "Unknown status");
		return false;
	}
};

const Bitmap::StatusChecker Bitmap::VerifiedStatusChecker = [](SegmentStatus status)->bool
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::EmptyStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::RequestedStatus:
	case Bitmap::DownloadedStatus:
		return false;
	case Bitmap::VerifiedStatus:
		return true;
	default:
		Q_ASSERT_X(false, "VerifiedStatusChecker", "Unknown status");
		return false;
	}
};

const Bitmap::StatusUpdater Bitmap::DummyUpdater = [](SegmentStatus &status)
{
	Q_ASSERT_X(false, "DummyUpdater", "Reached DummyUpdater");
	Q_UNUSED(status);
};

const Bitmap::StatusUpdater Bitmap::EmptyStatusUpdater = [](SegmentStatus &status)
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::RequestedStatus:
	case Bitmap::DownloadedStatus:
		status = Bitmap::EmptyStatus;
		break;
	case Bitmap::EmptyStatus:
	case Bitmap::VerifiedStatus:
		break;
	default:
		Q_ASSERT_X(false, "EmptyStatusUpdater", "Unknown status");
		break;
	}
};

const Bitmap::StatusUpdater Bitmap::MaskedStatusUpdater = [](SegmentStatus &status)
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
	case Bitmap::MaskedStatus:
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		break;
	case Bitmap::EmptyStatus:
	case Bitmap::RequestedStatus:
		status = Bitmap::MaskedStatus;
		break;
	default:
		Q_ASSERT_X(false, "MaskedStatusUpdater", "Unknown status");
		break;
	}
};

const Bitmap::StatusUpdater Bitmap::RequestedStatusUpdater = [](SegmentStatus &status)
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
		//Q_ASSERT_X(false, "RequestedStatusUpdater", "An InvalidStatus segment has been requested");
		break;
	case Bitmap::MaskedStatus:
		//Q_ASSERT_X(false, "RequestedStatusUpdater", "A MaskedStatus segment has been requested");
		break;
	case Bitmap::EmptyStatus:
		status = Bitmap::RequestedStatus;
		break;
	case Bitmap::RequestedStatus:
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		break;
	default:
		Q_ASSERT_X(false, "RequestedStatusUpdater", "Unknown status");
		break;
	}
};

const Bitmap::StatusUpdater Bitmap::DownloadedStatusUpdater = [](SegmentStatus &status)
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
		//Q_ASSERT_X(false, "DownloadedStatusUpdater", "An InvalidStatus segment has been downloaded");
		break;
	case Bitmap::EmptyStatus:
		//Q_ASSERT_X(false, "DownloadedStatusUpdater", "An EmptyStatus segment has been downloaded");
		break;
	case Bitmap::MaskedStatus:
		//Q_ASSERT_X(false, "DownloadedStatusUpdater", "A MaskedStatus segment has been downloaded");
		break;
	case Bitmap::RequestedStatus:
		status = Bitmap::DownloadedStatus;
		break;
	case Bitmap::DownloadedStatus:
	case Bitmap::VerifiedStatus:
		break;
	default:
		Q_ASSERT_X(false, "DownloadedStatusUpdater", "Unknown status");
		break;
	}
};

const Bitmap::StatusUpdater Bitmap::VerifiedStatusUpdater = [](SegmentStatus &status)
{
	using Utility::Bitmap;
	switch (status)
	{
	case Bitmap::InvalidStatus:
		//Q_ASSERT_X(false, "VerifiedStatusUpdater", "An InvalidStatus segment has passed verification");
		break;
	case Bitmap::EmptyStatus:
		//Q_ASSERT_X(false, "VerifiedStatusUpdater", "An EmptyStatus segment has passed verification");
		break;
	case Bitmap::MaskedStatus:
		//Q_ASSERT_X(false, "VerifiedStatusUpdater", "A MaskedStatus segment has passed verification");
		break;
	case Bitmap::RequestedStatus:
		//Q_ASSERT_X(false, "VerifiedStatusUpdater", "A RequestedStatus segment has passed verification");
		break;
	case Bitmap::DownloadedStatus:
		status = Bitmap::VerifiedStatus;
	case Bitmap::VerifiedStatus:
		break;
	default:
		Q_ASSERT_X(false, "VerifiedStatusUpdater", "Unknown status");
		break;
	}
};

}

