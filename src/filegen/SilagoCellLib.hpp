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

#ifndef __VESYLA_FILEGEN_SILAGO_CELL_LIB_HPP__
#define __VESYLA_FILEGEN_SILAGO_CELL_LIB_HPP__

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <map>
#include "util/Common.hpp"

using namespace std;

namespace vesyla
{
namespace filegen
{

class SilagoCellLib
{

public:
	struct CellConfig
	{
		int height;
		int width;
		int type;
	};

public:
	/**
	 * \brief Load XML formatted silago cell defination.
	 * \param path The path to file.
	 */
	void load(const string path);
	/**
	 * \brief Find the height (number of rows) of a cell.
	 * \param name The name of the cell.
	 * \return The height or -1 if the cell name is invalid.
	 */
	int get_height(const string name);
	/**
	 * \brief Find the width (number of columns) of a cell.
	 * \param name The name of the cell.
	 * \return The width or -1 if the cell name is invalid.
	 */
	int get_width(const string name);
	/**
	 * \brief Find the type id of a cell.
	 * \param name The name of the cell.
	 * \return The type id or -1 if the cell name is invalid.
	 */
	int get_type(const string name);

private:
	map<string, CellConfig> _silago_cell_map;
};

} // namespace filegen
} // namespace vesyla
#endif // __VESYLA_CODE_GEN_PRIMITIVE_FUNC_LIB_HPP__
