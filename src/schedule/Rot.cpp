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

#include "Rot.hpp"

namespace vesyla
{
namespace schedule
{

bool Rot::verify()
{
	map<string, Timetable> tmp_rot = _rot;
	for (auto r : tmp_rot)
	{
		Timetable tb = tmp_rot[r.first];
		if (!tb.merge())
		{
			return false;
		}
		tmp_rot[r.first] = tb;
	}
	_rot = tmp_rot;
	return true;
}

void Rot::merge(Rot rot2, int offset_)
{
	int offset = offset_;
	map<string, Timetable> tmp_rot1 = _rot;
	map<string, Timetable> tmp_rot2 = rot2.rot();
	for (auto r : tmp_rot2)
	{
		Timetable tb2 = tmp_rot2[r.first];
		tb2.shift(offset);
		if (tmp_rot1.find(r.first) == tmp_rot1.end())
		{
			tmp_rot1[r.first] = tb2;
			tmp_rot1[r.first].merge();
			continue;
		}
		Timetable tb1 = tmp_rot1[r.first];

		for (auto &f : tb2)
		{
			tb1.push_back(f);
		}
		tb1.merge();
		tmp_rot1[r.first] = tb1;
	}
	_rot = tmp_rot1;
}

int Rot::merge_and_verify(Rot rot2, int offset_lo, int offset_hi)
{
	if (offset_hi == INT_MIN)
	{
		offset_hi = offset_lo;
	}

	CHECK_GE(offset_hi, offset_lo) << "Offset higher bound should be larger than lower bound";

	while (offset_lo <= offset_hi)
	{
		bool flag_fail = false;
		int offset = offset_lo;

		map<string, Timetable> tmp_rot1 = _rot;
		map<string, Timetable> tmp_rot2 = rot2.rot();

		for (auto r : tmp_rot2)
		{
			Timetable tb2 = tmp_rot2[r.first];
			tb2.shift(offset);
			if (tmp_rot1.find(r.first) == tmp_rot1.end())
			{
				tmp_rot1[r.first] = tb2;
				continue;
			}
			Timetable tb1 = tmp_rot1[r.first];
			Timetable tb1_tmp = tmp_rot1[r.first];
			for (auto &f : tb2)
			{
				tb1.push_back(f);
				if (!tb1.merge())
				{
					bool flag_stop_trying = true;
					for (auto &f1 : tb1_tmp)
					{
						for (auto &f2 : tb2)
						{
							if (!f2.is_later_than(f1))
							{
								flag_stop_trying = false;
							}
						}
					}
					if (flag_stop_trying)
					{
						return INT_MIN;
					}
					else
					{
						offset_lo++;
						flag_fail = true;
						break;
					}
				}
			}
			if (flag_fail)
			{
				break;
			}
			tmp_rot1[r.first] = tb1;
		}
		if (flag_fail)
		{
			continue;
		}
		_rot = tmp_rot1;
		break;
	}
	if (offset_lo > offset_hi)
	{
		return INT_MIN;
	}
	return offset_lo;
}

bool Rot::unite(Rot rot2_, int offset_)
{
	for (auto &r : _rot)
	{
		Timetable tb1 = _rot[r.first];
		if (rot2_.find(r.first) == rot2_.end())
		{
			continue;
		}
		Timetable tb2 = rot2_[r.first];
		tb2.shift(offset_);
		CHECK(tb1.merge());
		CHECK(tb2.merge());
		Timetable tb;
		int i0 = 0;
		int i1 = 0;
		int t_key = INT_MAX;
		int t_lock = INT_MIN;
		int t_min = INT_MAX;
		int t_max = INT_MIN;

		for (auto &f : tb1)
		{
			if (f.type() == 1 && f.pos() < t_key)
			{
				t_key = f.pos();
			}
			else if (f.type() == 2 && f.pos() > t_lock)
			{
				t_lock = f.pos();
			}
			else if (f.type() == 3)
			{
				if (f.t0 < t_min)
				{
					t_min = f.t0;
				}
				if (f.t1 > t_max)
				{
					t_max = f.t1;
				}
			}
		}
		for (auto &f : tb2)
		{
			if (f.type() == 1 && f.pos() < t_key)
			{
				t_key = f.pos();
			}
			else if (f.type() == 2 && f.pos() > t_lock)
			{
				t_lock = f.pos();
			}
			else if (f.type() == 3)
			{
				if (f.t0 < t_min)
				{
					t_min = f.t0;
				}
				if (f.t1 > t_max)
				{
					t_max = f.t1;
				}
			}
		}

		if (t_key < INT_MAX)
		{
			tb.push_back(Frame(INT_MIN, t_key));
			if (t_min < INT_MAX && t_max > INT_MIN)
			{
				if (t_lock > INT_MIN)
				{
					tb.push_back(Frame(t_key + 1, t_lock - 1));
					tb.push_back(Frame(t_lock, INT_MAX));
				}
				else
				{
					tb.push_back(Frame(t_key + 1, t_max));
				}
			}
			else
			{
				if (t_lock > INT_MIN)
				{
					tb.push_back(Frame(t_key + 1, t_lock - 1));
					tb.push_back(Frame(t_lock, INT_MAX));
				}
				else
				{
					;
				}
			}
		}
		else
		{
			if (t_min < INT_MAX && t_max > INT_MIN)
			{
				if (t_lock > INT_MIN)
				{
					tb.push_back(Frame(t_min, t_lock - 1));
					tb.push_back(Frame(t_lock, INT_MAX));
				}
				else
				{
					tb.push_back(Frame(t_min, t_max));
				}
			}
			else
			{
				if (t_lock > INT_MIN)
				{
					tb.push_back(Frame(t_lock, INT_MAX));
				}
				else
				{
					;
				}
			}
		}

		_rot[r.first] = tb;
	}

	for (auto &r : rot2_)
	{
		Timetable tb2 = rot2_[r.first];
		tb2.shift(offset_);
		if (_rot.find(r.first) == _rot.end())
		{
			_rot[r.first] = tb2;
			continue;
		}
	}

	CHECK(verify());
	return true;
}

string Rot::dump()
{
	string s;
	for (auto &r : _rot)
	{
		s += (r.first + " : " + r.second.dump() + "\n");
	}
	return s;
}

} // namespace schedule
} // namespace vesyla
