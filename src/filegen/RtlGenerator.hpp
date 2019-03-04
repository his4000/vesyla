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

#ifndef __VESYLA_FILEGEN_RTL_GENERATOR_HPP__
#define __VESYLA_FILEGEN_RTL_GENERATOR_HPP__

#include <boost/algorithm/string.hpp>
#include "util/Common.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "Common.hpp"
#include "codegen/StorageImage.hpp"

using namespace BIR;
using namespace BIR::BIREnumerations;
namespace vesyla
{
namespace filegen
{
class RtlGenerator
{
private:
public:
	RtlGenerator() {}
	~RtlGenerator() {}
	void generate(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string directory_);

private:
	string InputFileName();
	void gen_fill_data(string filename_);
	void gen_macros(string filename_);
	void gen_name_mangling(string filename_);
	void gen_test_util(string filename_);
	void gen_test_util_files(string filename_);
	void gen_type_def(string filename_);
	void gen_profiler(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_);
	void gen_run_cmd(string filename_);
	void gen_run_gui(string filename_);
	void gen_wave(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_);
	void gen_testbench(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_);
	void gen_const_package(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_);

	string dump_profiler_reg_file_var(codegen::StorageImage si_);
	string dump_profiler_skip_cycles_of_write_access(map<string, vector<BIR::Instruction *>> instr_lists_);
	string dump_profiler_fixed_point_status(codegen::StorageImage si_);
	string dump_drra_wave(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_);
	string dump_dimarch_wave(codegen::StorageImage si_);
	string dump_testbench_header(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_);
	string dump_testbench_instr(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_);
};
} // namespace filegen
} // namespace vesyla

#endif // __VESYLA_FILEGEN_RTL_GENERATOR_HPP__