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

#ifndef __VESYLA_CIDFG_SOURCE_VERTEX_HPP__
#define __VESYLA_CIDFG_SOURCE_VERTEX_HPP__

#include "Vertex.hpp"

namespace vesyla
{
namespace cidfg
{
class SourceVertex : public Vertex
{
public:
	enum ValueType
	{
		UNKNOWN,
		INT,
		FLOAT
	};

public:
	string var_name;

public:
	SourceVertex();
	virtual ~SourceVertex() = 0;
};

} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_SOURCE_VERTEX_HPP__