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

#ifndef __VESYLA_CODEGEN_STORAGE_IMAGE_HPP__
#define __VESYLA_CODEGEN_STORAGE_IMAGE_HPP__

#include "util/Common.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "cidfg/CidfgGraph.hpp"
#include <bitset>

namespace vesyla
{
namespace codegen
{
class StorageImage
{
public:
	struct StorageEntry
	{
		string var_name;
		int int_value;
		float float_value;
		bool is_fixed_point;

		StorageEntry()
		{
			var_name = " ";
			int_value = 0;
			float_value = 0.0;
			is_fixed_point = false;
		}
	};

public:
	std::unordered_map<string, vector<StorageEntry>> _sram;
	std::unordered_map<string, vector<StorageEntry>> _reg;
	std::unordered_map<string, int> _sram_var_start_addr;
	std::unordered_map<string, int> _reg_var_start_addr;
	std::unordered_map<string, int> _sram_var_offset;
	std::unordered_map<string, int> _reg_var_offset;

public:
	StorageImage(VIR::MainProgram *p_, cidfg::CidfgGraph &g);
	~StorageImage();

	void to_txt_file(string filename_);
	void to_bin_file(string filename_);

	string print_for_profiler();
	string print_for_testbench();
};
} // namespace codegen
} // namespace vesyla

#endif // __VESYLA_CODEGEN_STORAGE_IMAGE_HPP__