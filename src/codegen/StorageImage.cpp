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

#include "StorageImage.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{
StorageImage::StorageImage(VIR::MainProgram *p_, cidfg::CidfgGraph &g_)
{
	int reg_file_depth;
	int reg_file_width;
	int sram_depth;
	int sram_width;

	util::GlobalVar glv;
	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_depth", sram_depth));
	CHECK(glv.geti("sram_width", sram_width));

	// Find utilized cells
	std::set<int> drra_rows;
	std::set<int> drra_cols;
	std::set<int> sram_rows;
	std::set<int> sram_cols;
	for (auto &s : p_->body())
	{
		if (s->kind() == VIR::VirEnumerations::ktAssignmentStatement)
		{
			VIR::AssignmentStatement *ss = static_cast<VIR::AssignmentStatement *>(s);
			if (ss->type() == VIR::VirEnumerations::atDeclarative)
			{
				VIR::StoragePragma *pragma = static_cast<VIR::StoragePragma *>(ss->pragma());
				CHECK(pragma);
				CHECK_GE(pragma->coordinates.size(), 1);
				if (pragma->storageType() == VIR::VirEnumerations::stRegFile)
				{
					drra_rows.insert(pragma->coordinates[0].row);
					drra_cols.insert(pragma->coordinates[0].column);
				}
				else
				{
					sram_rows.insert(pragma->coordinates[0].row);
					sram_cols.insert(pragma->coordinates[0].column);
				}
			}
			else if (ss->type() == VIR::VirEnumerations::atArithmetic)
			{
				VIR::ReferencePragma *pragma = static_cast<VIR::ReferencePragma *>(ss->pragma());
				CHECK(pragma);
				CHECK_GE(pragma->coordinates.size(), 1);
				drra_rows.insert(pragma->coordinates[0].row);
				drra_cols.insert(pragma->coordinates[0].column);
			}
			else if (ss->type() == VIR::VirEnumerations::atDPUChainDeclaration)
			{
				VIR::ReferencePragma *pragma = static_cast<VIR::ReferencePragma *>(ss->pragma());
				CHECK(pragma);
				CHECK_GE(pragma->coordinates.size(), 1);
				drra_rows.insert(pragma->coordinates[0].row);
				drra_cols.insert(pragma->coordinates[0].column);
			}
		}
	}

	int utilized_drra_row = 0;
	int utilized_drra_col = 0;
	int utilized_dimarch_row = 0;
	int utilized_dimarch_col = 0;
	if (!drra_rows.empty())
	{
		utilized_drra_row = *drra_rows.rbegin() + 1;
	}
	if (!drra_cols.empty())
	{
		utilized_drra_col = *drra_cols.rbegin() + 1;
	}
	if (!sram_rows.empty())
	{
		utilized_dimarch_row = *sram_rows.rbegin() + 1;
	}
	if (!sram_cols.empty())
	{
		utilized_dimarch_col = *sram_cols.rbegin() + 1;
	}
	glv.puti("utilized_drra_row", utilized_drra_row);
	glv.puti("utilized_drra_col", utilized_drra_col);
	glv.puti("utilized_dimarch_row", utilized_dimarch_row);
	glv.puti("utilized_dimarch_col", utilized_dimarch_col);

	// Make the initial storage image
	int all_drra_row;
	int all_drra_col;
	int all_dimarch_row;
	int all_dimarch_col;
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));
	CHECK(glv.geti("dimarch_row", all_dimarch_row));
	CHECK(glv.geti("dimarch_col", all_dimarch_col));
	std::unordered_map<string, int> sram_counter;
	std::unordered_map<string, int> reg_counter;
	for (int row = 0; row < all_drra_row; row++)
	{
		for (int col = 0; col < all_drra_col; col++)
		{
			VIR::Coordinate c(row, col);
			vector<StorageEntry> vse(reg_file_depth);
			_reg[c.to_str()] = vse;
			reg_counter[c.to_str()] = 0;
		}
	}
	for (int row = 0; row < all_dimarch_row; row++)
	{
		for (int col = 0; col < all_dimarch_col; col++)
		{
			VIR::Coordinate c(row, col);
			vector<StorageEntry> vse(sram_depth * sram_width / reg_file_width);
			_sram[c.to_str()] = vse;
			sram_counter[c.to_str()] = 0;
		}
	}

	set<string> names;

	for (auto &s : p_->body())
	{
		if (s->kind() == VIR::VirEnumerations::ktAssignmentStatement &&
				static_cast<VIR::AssignmentStatement *>(s)->type() == VIR::VirEnumerations::atDeclarative &&
				(static_cast<VIR::StoragePragma *>(static_cast<VIR::AssignmentStatement *>(s)->pragma())->storageType() == VIR::VirEnumerations::stRegFile ||
				 static_cast<VIR::StoragePragma *>(static_cast<VIR::AssignmentStatement *>(s)->pragma())->storageType() == VIR::VirEnumerations::stMemory))
		{
			for (auto &v : g_.get_vertices())
			{
				if (v->vertex_type == Vertex::REG_VAR_VERTEX)
				{
					RegVarVertex *vv = static_cast<RegVarVertex *>(v);
					if (vv->var_name == static_cast<VIR::Identifier *>(static_cast<VIR::AssignmentStatement *>(s)->lhs()[0])->name())
					{
						if (names.find(vv->var_name) != names.end())
						{
							continue;
						}
						names.insert(vv->var_name);
						if (vv->value_type == SourceVertex::INT)
						{
							int size = vv->int_data_array.size();
							string tag = vv->coord.to_str();
							for (auto &ele : vv->int_data_array)
							{
								_reg[tag][reg_counter[tag]].var_name = vv->var_name;
								_reg[tag][reg_counter[tag]].int_value = ele;
								_reg[tag][reg_counter[tag]].is_fixed_point = false;
								if (_reg_var_start_addr.find(vv->var_name) == _reg_var_start_addr.end())
								{
									_reg_var_start_addr[vv->var_name] = reg_counter[tag];
									_reg_var_offset[vv->var_name + "_" + to_string(vv->coord.row) + to_string(vv->coord.column)] = 0;
								}
								reg_counter[tag]++;
							}
						}
						else if (vv->value_type == SourceVertex::FLOAT)
						{
							int size = vv->float_data_array.size();
							string tag = vv->coord.to_str();
							for (auto &ele : vv->float_data_array)
							{
								_reg[tag][reg_counter[tag]].var_name = vv->var_name;
								_reg[tag][reg_counter[tag]].float_value = ele;
								_reg[tag][reg_counter[tag]].is_fixed_point = true;
								if (_reg_var_start_addr.find(vv->var_name) == _reg_var_start_addr.end())
								{
									_reg_var_start_addr[vv->var_name] = reg_counter[tag];
									_reg_var_offset[vv->var_name + "_" + to_string(vv->coord.row) + to_string(vv->coord.column)] = 0;
								}
								reg_counter[tag]++;
							}
						}
					}
				}
				else if (v->vertex_type == Vertex::SRAM_VAR_VERTEX)
				{
					SramVarVertex *vv = static_cast<SramVarVertex *>(v);
					if (vv->var_name == static_cast<VIR::Identifier *>(static_cast<VIR::AssignmentStatement *>(s)->lhs()[0])->name())
					{
						if (names.find(vv->var_name) != names.end())
						{
							continue;
						}
						names.insert(vv->var_name);
						if (vv->value_type == SourceVertex::INT)
						{
							int size = vv->int_data_array.size();
							string tag = vv->sram_coord.to_str();
							for (auto &ele : vv->int_data_array)
							{
								_sram[tag][sram_counter[tag]].var_name = vv->var_name;
								_sram[tag][sram_counter[tag]].int_value = ele;
								_sram[tag][sram_counter[tag]].is_fixed_point = false;
								if (_sram_var_start_addr.find(vv->var_name) == _sram_var_start_addr.end())
								{
									_sram_var_start_addr[vv->var_name] = sram_counter[tag];
									_sram_var_offset[vv->var_name + "_" + to_string(vv->sram_coord.row) + to_string(vv->sram_coord.column)] = 0;
								}
								sram_counter[tag]++;
							}
						}
						else if (vv->value_type == SourceVertex::FLOAT)
						{
							int size = vv->float_data_array.size();
							string tag = vv->sram_coord.to_str();
							for (auto &ele : vv->float_data_array)
							{
								_sram[tag][sram_counter[tag]].var_name = vv->var_name;
								_sram[tag][sram_counter[tag]].float_value = ele;
								_sram[tag][sram_counter[tag]].is_fixed_point = true;
								if (_sram_var_start_addr.find(vv->var_name) == _sram_var_start_addr.end())
								{
									_sram_var_start_addr[vv->var_name] = sram_counter[tag];
									_sram_var_offset[vv->var_name + "_" + to_string(vv->sram_coord.row) + to_string(vv->sram_coord.column)] = 0;
								}
								sram_counter[tag]++;
							}
						}
					}
				}
			}
		}
	}
} // namespace codegen
StorageImage::~StorageImage()
{
}
void StorageImage::to_txt_file(string filename_)
{
}
void StorageImage::to_bin_file(string filename_)
{
}
string StorageImage::print_for_profiler()
{
	int all_drra_row;
	int all_drra_col;
	int all_dimarch_row;
	int all_dimarch_col;
	int reg_file_depth;
	int reg_file_width;
	int sram_depth;
	int sram_width;

	util::GlobalVar glv;
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));
	CHECK(glv.geti("dimarch_row", all_dimarch_row));
	CHECK(glv.geti("dimarch_col", all_dimarch_col));
	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_depth", sram_depth));
	CHECK(glv.geti("sram_width", sram_width));
	int mem_block_size = sram_width / reg_file_width;

	string lineStr = "";
	lineStr += "const string regFileVariables[ROWS][COLUMNS][REG_FILE_DEPTH] = '{\n";
	lineStr += "\t";

	const int rowNum = all_drra_row;
	const int colNum = all_drra_col;

	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	{
		lineStr += "'{\n";
		lineStr += "\t\t";

		for (int colIndex = 0; colIndex < colNum; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			for (int regIndex = 0; regIndex < reg_file_depth; ++regIndex)
			{
				if (regIndex == 0)
					lineStr += "'{ ";

				lineStr += "\"";
				lineStr += _reg[coord.to_str()][regIndex].var_name;
				lineStr += "\"";

				if (regIndex != reg_file_depth - 1)
					lineStr += ", ";
				else
				{
					lineStr += " }";

					if (colIndex != colNum - 1)
						lineStr += ",";
				}

				if ((regIndex + 1) % 16 == 0 || regIndex == reg_file_depth - 1)
				{
					lineStr += "\n";
					lineStr += "   ";
				}
			}
		}

		lineStr += "}";

		if (rowIndex != rowNum - 1)
			lineStr += ",";

		lineStr += "\t\n";
	}

	lineStr += "\t};\n";

	string initStr;

	for (auto element : _reg_var_start_addr)
		initStr += "\"" + element.first + "\" : " + to_string(element.second) + ", ";

	// removing the extra comma ", " in the end.
	initStr.pop_back();
	initStr.pop_back();

	lineStr += "const int variablesStartAddr[string] = '{" + initStr + "};\n";

	initStr = "";
	for (auto element : _reg_var_offset)
		initStr += "\"" + element.first + "\" : " + to_string(element.second) + ", ";

	// removing the extra comma ", " in the end.
	initStr.pop_back();
	initStr.pop_back();

	lineStr += "const int addressOffset[string] = '{" + initStr + "};\n";

	return lineStr;
}

string getRowOrColSignature(int index_, int length_)
{
	string signature(length_, '0');
	signature.replace(length_ - index_ - 1, 1, 1, '1');
	signature = "\"" + signature + "\"";
	return signature;
}
// This method converts the given integer value to an HEX string with the given size
string intToHexStr(int value_, int strSize_)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(strSize_) << std::hex << value_;
	const string valStr = stream.str();
	return valStr.substr(valStr.size() - strSize_, strSize_);
}

// This method converts the given float value to a fixed-point Q15 format HEX string with the given size
string realToHexStr(float value_, int strSize_)
{
	// Q15 format
	short intValue = short(value_ * pow(2, 15));
	// Correction when value_ == 1.0
	if (value_ == 1.0)
		intValue = 32767;
	return intToHexStr(intValue, strSize_);
}
template <int strSize>
string intToBinaryStr(int value_)
{
	std::bitset<strSize> bs(value_);
	return "\"" + bs.to_string() + "\"";
}

string StorageImage::print_for_testbench()
{
	static const int cSramAddressWidth = 7;
	static const int cRowBitWidth = 2;
	static const int cColBitWidth = 3;

	int all_drra_row;
	int all_drra_col;
	int utilized_drra_row;
	int utilized_drra_col;
	int utilized_dimarch_row;
	int utilized_dimarch_col;
	int reg_file_depth;
	int reg_file_width;
	int sram_depth;
	int sram_width;

	util::GlobalVar glv;
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));
	CHECK(glv.geti("utilized_dimarch_row", utilized_dimarch_row));
	CHECK(glv.geti("utilized_dimarch_col", utilized_dimarch_col));
	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_depth", sram_depth));
	CHECK(glv.geti("sram_width", sram_width));
	int mem_block_size = sram_width / reg_file_width;
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));

	string str = "";

	// Return when memory is not used.
	if (utilized_dimarch_col > 0 && utilized_dimarch_row > 0)
	{

		str += R"(
	TYPE mem_value_type IS RECORD

		address : std_logic_vector (SRAM_ADDRESS_WIDTH-1 downto 0);
		row     : unsigned (ROW_WIDTH-1 downto 0);
		col     : unsigned (COL_WIDTH-1 downto 0);
		data   : std_logic_vector (SRAM_WIDTH-1 downto 0);
	
	END RECORD;
	TYPE mem_values_type IS ARRAY (0 to mem_load_cycles-1) OF mem_value_type;
	CONSTANT mem_init_values : mem_values_type := (
	)";

		string initialValues;
		bool isFirstLine = true;
		int counter = 0;

		for (int rowIndex = 0; rowIndex < utilized_dimarch_row; ++rowIndex)
		{
			for (int colIndex = 0; colIndex < utilized_dimarch_col; ++colIndex)
			{
				bool flag_has_init_load = false;
				VIR::Coordinate coord(rowIndex, colIndex);
				for (int i = 0; i < _sram[coord.to_str()].size(); i++)
				{
					if (_sram[coord.to_str()][i].var_name != " ")
					{
						flag_has_init_load = true;
						break;
					}
				}

				if (flag_has_init_load)
				{
					for (int index = 0; index < sram_depth; index += 1)
					{

						bool flag_has_init_load = false;
						VIR::Coordinate coord(rowIndex, colIndex);
						for (int i = index * mem_block_size; i < (index + 1) * mem_block_size; i++)
						{
							if (_sram[coord.to_str()][i].var_name != " ")
							{
								flag_has_init_load = true;
								break;
							}
						}
						StorageEntry se = _sram[coord.to_str()][index * mem_block_size];

						if (flag_has_init_load)
						{
							initialValues = !isFirstLine ? ", " : "  ";
							initialValues += to_string(counter++) + " => (";
							initialValues += intToBinaryStr<cSramAddressWidth>(index) + ", ";
							initialValues += intToBinaryStr<cRowBitWidth>(rowIndex + 1) + ", ";
							initialValues += intToBinaryStr<cColBitWidth>(colIndex) + ", ";
							initialValues += "X\"";
							if (!se.is_fixed_point)
							{
								const int valuesCount = mem_block_size;
								for (int subIndex = valuesCount - 1; subIndex >= 0; --subIndex)
									initialValues += intToHexStr(_sram[coord.to_str()][index * mem_block_size + subIndex].int_value, 4);
							}
							else
							{
								const int valuesCount = mem_block_size;
								for (int subIndex = valuesCount - 1; subIndex >= 0; --subIndex)
									initialValues += realToHexStr(_sram[coord.to_str()][index * mem_block_size + subIndex].float_value, 4);
							}
							initialValues += +"\")\n";
							str += initialValues;
							if (isFirstLine)
								isFirstLine = false;
						}
					}
				}
			}
		}
		str += ");";
	}

	if (utilized_drra_row > 0 && utilized_drra_col > 0)
	{

		str += R"(
	TYPE reg_value_type IS RECORD
		address : std_logic_vector (REG_FILE_MEM_ADDR_WIDTH-1 downto 0);
		row     : std_logic_vector (ROWS-1 downto 0);
		col     : std_logic_vector (COLUMNS-1 downto 0);
		data    : signed (REG_FILE_MEM_DATA_WIDTH-1 downto 0);
	END RECORD;
	TYPE reg_values_type IS ARRAY (0 to reg_load_cycles-1) OF reg_value_type;
	
	CONSTANT reg_init_values : reg_values_type := (
		)";

		string initialValues;
		bool commaFlag = false;

		for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
		{
			for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
			{
				const int bankNumber = reg_file_depth / mem_block_size;
				const int bankCapacity = mem_block_size;
				const int bankIndexWidth = (int)(ceil(log2(bankNumber)));
				int bankLimit;
				bool flag_has_init_load = false;

				VIR::Coordinate coord(rowIndex, colIndex);
				for (int i = 0; i < _reg[coord.to_str()].size(); i++)
				{
					if (_reg[coord.to_str()][i].var_name != " ")
					{
						flag_has_init_load = true;
						break;
					}
				}
				if (flag_has_init_load)
				{
					for (int bankIndex = 0; bankIndex < bankNumber; ++bankIndex)
					{
						initialValues = commaFlag ? "\t\t, (" : "  (";
						bitset<32> bin_biankIndex(bankIndex);
						initialValues += "\"" + bin_biankIndex.to_string().substr(32 - bankIndexWidth) + "\", ";
						initialValues += getRowOrColSignature(rowIndex, all_drra_row) + ", ";
						initialValues += getRowOrColSignature(colIndex, all_drra_col) + ", ";
						initialValues += "X\"";
						bankLimit = (bankIndex + 1) * bankCapacity;
						for (int regIndex = bankLimit - 1; regIndex >= bankIndex * bankCapacity; --regIndex)
						{
							// TODO: CHANGE TO: remove the if statement and use just the THEN part.
							if (regIndex < reg_file_depth)
							{
								StorageEntry reg = _reg[coord.to_str()][regIndex];
								initialValues += !reg.is_fixed_point ? intToHexStr(reg.int_value, 4) : realToHexStr(reg.float_value, 4);
							}
							else
							{
								initialValues += "0000";
							}
						}

						initialValues += +"\")\n";
						str += initialValues;

						if (!commaFlag)
							commaFlag = true;
					}
				}
			}
		}

		str += ");";
	}

	return str;
}
} // namespace codegen
} // namespace vesyla