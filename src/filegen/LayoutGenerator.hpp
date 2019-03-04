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

#ifndef __VESYLA_FILEGEN_LAYOUT_GENERATOR_HPP__
#define __VESYLA_FILEGEN_LAYOUT_GENERATOR_HPP__

#include "util/Common.hpp"
#include "util/bitmap/bitmap_image.hpp"
#include "Common.hpp"
#include "SilagoCellLib.hpp"

namespace vesyla
{
namespace filegen
{
class LayoutGenerator
{
private:
	SilagoCellLib _silago_cell_lib;

public:
	LayoutGenerator()
	{
		string silago_cell_config_file = vesyla::util::SysPath::find_config_file("silago_cell_def.xml");
		if (silago_cell_config_file != "")
		{
			_silago_cell_lib.load(silago_cell_config_file);
			LOG(INFO) << "Use silago cell defination file: " << silago_cell_config_file;
		}
		else
		{
			LOG(FATAL) << "SilagoCell defination file is not found. Abort!";
		}
	}
	~LayoutGenerator() {}
	void generate(string directory_);

private:
	void gen_csv(vector<vector<int>> layout, string filename_);
	void gen_bmp(vector<vector<int>> layout, string filename_);
};
} // namespace filegen
} // namespace vesyla

#endif // __VESYLA_FILEGEN_SYNC_INSTR_GENERATOR_HPP__