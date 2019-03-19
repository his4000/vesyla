// Copyright (C) 2019 Yu Yang
//
// This file is part of Vesyla.
//
// Vesyla is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vesyla is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vesyla.  If not, see <http://www.gnu.org/licenses/>.

#include "Frame.hpp"

namespace vesyla
{
namespace schedule
{

int Frame::type()
{
	if (t0 == INT_MIN && t1 < INT_MAX)
	{
		return 1;
	}
	else if (t0 > INT_MIN && t1 == INT_MAX)
	{
		return 2;
	}
	else if (t0 > INT_MIN && t1 < INT_MAX && t0 <= t1)
	{
		return 3;
	}
	else
	{
		LOG(FATAL) << "Illegal frame!";
		return 0;
	}
}

int Frame::pos()
{
	if (type() == 1)
	{
		return t1;
	}
	else
	{
		return t0;
	}
}

int Frame::max_pos()
{
	if (type() == 2)
	{
		return t0;
	}
	else
	{
		return t1;
	}
}

void Frame::shift(int offset_)
{
	long tmp0 = long(t0) + long(offset_);
	long tmp1 = long(t1) + long(offset_);
	if (t0 != INT_MIN)
	{
		t0 = tmp0 > INT_MIN ? tmp0 : INT_MIN;
	}
	if (t1 != INT_MAX)
	{
		t1 = tmp1 < INT_MAX ? tmp1 : INT_MAX;
	}
}

bool Frame::is_later_than(Frame f_)
{
	if (f_.type() == 3)
	{
		return pos() > f_.t1;
	}
	else
	{
		return pos() > f_.pos();
	}
}

string Frame::dump()
{
	string s = "[" + to_string(t0) + "," + to_string(t1) + "]";
	if (t0 == INT_MIN || t1 == INT_MAX)
	{
		s += "(sn=" + to_string(sn) + ")";
	}
	return s;
}

} // namespace schedule
} // namespace vesyla
