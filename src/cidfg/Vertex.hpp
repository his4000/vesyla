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

#ifndef __VESYLA_CIDFG_VERTEX_HPP__
#define __VESYLA_CIDFG_VERTEX_HPP__

#include "Object.hpp"
#include "VIR/VIRIncludeAll.hpp"

namespace vesyla
{
namespace cidfg
{
/**
 * This class is the base class for vertex type object in cidfg.
 * It maintains vertex_type and coord of each vertex object.
 * Some help functions are also defined to easily assert the vertex type.
 * \warning This is an abstract class, never use it directly.
 */
class Vertex : public Object
{
public:
	/**
	 * \brief The type of vertices
	 */
	enum VertexType
	{
		CONST_VERTEX,								 ///< [Source] Vertex that represent a constant number. Both integer and floating point number are supported.
		VAR_VERTEX,									 ///< [Source] Vertex that represent a scalar variable.
		REG_VAR_VERTEX,							 ///< [Source] Vertex that represent a vector variable in register file.
		SRAM_VAR_VERTEX,						 ///< [Source] Vertex that represent a vector variable in SRAM block.
		COMPUTATION_VERTEX,					 ///< [Function] Vertex that represent an computation operation, such as addition, multiplication, etc. Both scalar and vector operation are supported.
		INDEXING_VERTEX,						 ///< [Function]Vertex that represent a address indexing operation. It uses scalar vertices as address constraint and generate a sequence of address consumed by Reading or Writing vertex.
		READING_VERTEX,							 ///< [Function]Vertex that reads from a vector vertex according to an address sequence, and produce a sequence scalar data.
		WRITING_VERTEX,							 ///< [Function]Vertex that writes a sequence scalar data to a vector vertex according to an address sequence.
		GENERAL_HIERARCHICAL_VERTEX, ///< [Hierarchical] Vertex represent a artifical boundry of a group of vertices.
		BRANCH_VERTEX,							 ///< [Hierarchical] Vertex represent a branch data path, mainly used to represent if-then-else structure.
		LOOP_VERTEX,								 ///< [Hierarchical] Vertex represent a iterative loop data path, mainly used to represent for-loop structure.
		REG_SINK_VERTEX,						 ///< [RegSink] Vertex that terminates a register variable.
		SRAM_SINK_VERTEX,						 ///< [SramSink] Vertex that terminates a SRAM variable.
		VAR_SINK_VERTEX,						 ///< [VarSink] Vertex that terminates temperary variable.
		ROOT_VERTEX,								 ///< [Hierarchical] The hierarchical vertex that serves the entry of the CIDFG graph.
		READ_AND_INDEX_VERTEX,			 ///< [Hierarchical] Simply a merge of Reading and Indexing vertex.
		WRITE_AND_INDEX_VERTEX,			 ///< [Hierarchical] Simply a merge of Writing and Indexing vertex.
		MERGE_VERTEX,								 ///< [Hierarchical] A special vertex to merge the branched data flow of the same variable. Similar to phi node. Used together with Branch vertex.
		RACCU_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a RACCU instruction.
		DPU_INSTR_VERTEX,						 ///< [Instruction] A vertex that represent a RACCU instruction.
		SWB_INSTR_VERTEX,						 ///< [Instruction] A vertex that represent a DPU instruction.
		REFI_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a REFI instruction.
		LOOPH_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a LOOP_HEADER instruction.
		LOOPT_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a LOOP_TAIL instruction.
		BRANCH_INSTR_VERTEX,				 ///< [Instruction] A vertex that represent a LOOP_BRANCH instruction.
		JUMP_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a JUMP instruction.
		WAIT_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a WAIT instruction.
		ROUTE_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a ROUTE instruction.
		SRAM_INSTR_VERTEX,					 ///< [Instruction] A vertex that represent a SRAM instruction.
		HIERARCHICAL_INSTR_VERTEX,	 ///< [Instruction, Hierarchical] A vertex that represent an artificial boundary of a group of other instruction vertex.
	};

public:
	VertexType vertex_type; ///< The type of vertex. Filled automatically when the vertex is created.
	VIR::Coordinate coord;	///< The coordinate associated with the vertex. Only make sense for some vertex.

public:
	Vertex();
	virtual ~Vertex() = 0;

	/**
	 * Check if the vertex is hierarchical vertex. See Vertex::VertexType
	 */
	bool is_hierarchical();
	/**
	 * Check if the vertex is instruction vertex. See Vertex::VertexType
	 */
	bool is_instr();
	/**
	 * Check if the vertex is source vertex. See Vertex::VertexType
	 */
	bool is_source();
	/**
	 * Check if the vertex is sink vertex. See Vertex::VertexType
	 */
	bool is_sink();
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_VERTEX_HPP__