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

#include "IntPolynomialTable.hpp"

namespace vesyla
{
namespace cidfg
{
IntPolynomialTable::IntPolynomialTable()
{
}
IntPolynomialTable::IntPolynomialTable(map<multiset<string>, int> t_)
{
	_table = t_;
	drop_zero_terms();
}
IntPolynomialTable::~IntPolynomialTable()
{
}

void IntPolynomialTable::drop_zero_terms()
{
	vector<multiset<string>> vec_del_terms;
	for (auto &term : _table)
	{
		if (term.second == 0)
		{
			vec_del_terms.push_back(term.first);
		}
	}
	for (auto &term_key : vec_del_terms)
	{
		_table.erase(term_key);
	}
}

void IntPolynomialTable::add(IntPolynomialTable ipt_)
{
	const map<multiset<string>, int> t = ipt_.get_table();
	for (auto &term : t)
	{
		if (_table.find(term.first) != _table.end())
		{
			_table[term.first] += term.second;
		}
		else
		{
			_table[term.first] = term.second;
		}
	}
	drop_zero_terms();
}

void IntPolynomialTable::sub(IntPolynomialTable ipt_)
{
	const map<multiset<string>, int> t = ipt_.get_table();
	for (auto &term : t)
	{
		if (_table.find(term.first) != _table.end())
		{
			_table[term.first] -= term.second;
		}
		else
		{
			_table[term.first] = -(term.second);
		}
	}
	drop_zero_terms();
}

void IntPolynomialTable::mul(IntPolynomialTable ipt_)
{
	IntPolynomialTable result_table;
	const map<multiset<string>, int> t = ipt_.get_table();
	for (auto &term : t)
	{
		map<multiset<string>, int> temp_table;
		for (auto &term1 : _table)
		{
			multiset<string> s = term1.first;
			s.insert(term.first.begin(), term.first.end());
			int factor = term.second * term1.second;
			temp_table.insert({s, factor});
		}
		IntPolynomialTable tb(temp_table);
		result_table.add(tb);
	}
	_table = result_table.get_table();
}

void IntPolynomialTable::div_const(IntPolynomialTable ipt_)
{
	CHECK(ipt_.is_constant());
	div_const(ipt_.get_bias());
}

void IntPolynomialTable::div_const(int n_)
{
	CHECK(is_const_divisable(n_));
	for (auto &term : _table)
	{
		term.second /= n_;
	}
}

bool IntPolynomialTable::is_const_divisable(IntPolynomialTable ipt_)
{
	CHECK(ipt_.is_constant());
	return is_const_divisable(ipt_.get_bias());
}

bool IntPolynomialTable::is_const_divisable(int n_)
{
	bool is_divisable = true;
	for (auto &term : _table)
	{
		if (term.second % n_ != 0)
		{
			is_divisable = false;
			LOG(DEBUG) << "POLYNOMIAL = " << serialize() << " divide " << n_;
			break;
		}
	}
	return is_divisable;
}
bool IntPolynomialTable::is_constant()
{
	drop_zero_terms();
	if (is_zero())
	{
		return true;
	}
	if (_table.size() == 1)
	{
		for (auto &term : _table)
		{
			if (term.first.empty())
			{
				return true;
			}
			break;
		}
	}
	return false;
}
bool IntPolynomialTable::is_zero()
{
	drop_zero_terms();
	if (_table.empty())
	{
		return true;
	}
	return false;
}

int IntPolynomialTable::get_bias()
{
	int bias = 0;
	for (auto &term : _table)
	{
		if (term.first.empty())
		{
			bias = term.second;
			break;
		}
	}
	return bias;
}

const map<multiset<string>, int> IntPolynomialTable::get_table()
{
	return _table;
}

string IntPolynomialTable::serialize()
{
	if (is_zero())
	{
		return "0";
	}

	string str;
	int i = 0;
	for (auto &term : _table)
	{
		if (i != 0)
		{
			str += "+";
		}
		i++;
		str += to_string(term.second);
		for (auto v : term.first)
		{
			str += "*" + v;
		}
	}
	return str;
}

} // namespace cidfg
} // namespace vesyla