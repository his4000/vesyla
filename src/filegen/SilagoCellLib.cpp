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

#include "SilagoCellLib.hpp"

namespace vesyla
{
namespace filegen
{

void SilagoCellLib::load(const string path)
{
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(path, pt);

	for (ptree::value_type &v : pt.get_child("SilagoCells"))
	{
		if (v.first == "cell")
		{
			string cell_name = v.second.get<string>("<xmlattr>.name", "");
			CellConfig cc;
			cc.height = v.second.get<int>("<xmlattr>.height", -1);
			cc.width = v.second.get<int>("<xmlattr>.width", -1);
			cc.type = v.second.get<int>("<xmlattr>.type", -1);
			_silago_cell_map[cell_name] = cc;
		}
	}
}

int SilagoCellLib::get_height(const string name)
{
	auto it = _silago_cell_map.find(name);
	if (it == _silago_cell_map.end())
	{
		return -1;
	}
	return it->second.height;
}

int SilagoCellLib::get_width(const string name)
{
	auto it = _silago_cell_map.find(name);
	if (it == _silago_cell_map.end())
	{
		return -1;
	}
	return it->second.width;
}

int SilagoCellLib::get_type(const string name)
{
	auto it = _silago_cell_map.find(name);
	if (it == _silago_cell_map.end())
	{
		return -1;
	}
	return it->second.type;
}

} // namespace filegen
} // namespace vesyla
