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

#include "FileGenerator.hpp"

namespace vesyla
{
namespace filegen
{
void FileGenerator::generate(VIR::MainProgram *main_program_, map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_)
{
	vesyla::util::GlobalVar glv;
	string path;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	path = path + "filegen/";
	mkdir(path.c_str(), 0755);
	string input_file_name;
	CHECK(glv.gets("input_file_name", input_file_name));

	// Manas
	string path_manas = path + "instruction.manas";
	vesyla::filegen::ManasGenerator gm;
	gm.generate(instr_lists_, path_manas);

	// SRAM
	// map<string, SRAM *> sram_lists;
	// for (int rowIndex = 0; rowIndex < drra_cluster_->diMArch()->rowNum(); ++rowIndex)
	// {
	// 	for (int colIndex = 0; colIndex < drra_cluster_->diMArch()->colNum(); ++colIndex)
	// 	{
	// 		SRAM *sram = drra_cluster_->diMArch()->getDiMArchCell(rowIndex, colIndex)->sram();
	// 		string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
	// 		sram_lists[coord_signature] = sram;
	// 	}
	// }
	// string path_sramimg = path + input_file_name + ".sram";
	// vesyla::filegen::SramImageGenerator gs;
	// gs.generate(sram_lists, path_sramimg);

	// Matlab
	string path_matlab = path + "sim_matlab/";
	mkdir(path_matlab.c_str(), 0755);
	vesyla::filegen::MatlabGenerator gmat;
	gmat.generate(main_program_, path_matlab);

	// Synchronized instructions
	string path_sync_instr = path + "/sync_instr.txt";
	vesyla::filegen::SyncInstrGenerator gsi;
	gsi.generate(instr_lists_, path_sync_instr);

	// RTL
	string path_rtl = path + "sim_vsim/";
	mkdir(path_rtl.c_str(), 0755);
	vesyla::filegen::RtlGenerator grtl;
	grtl.generate(instr_lists_, si_, path_rtl);

	// Layout
	string path_layout = path + "layout/";
	mkdir(path_layout.c_str(), 0755);
	vesyla::filegen::LayoutGenerator glayout;
	glayout.generate(path_layout);
}
} // namespace filegen
} // namespace vesyla