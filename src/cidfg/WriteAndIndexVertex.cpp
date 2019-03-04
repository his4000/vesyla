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

#include "WriteAndIndexVertex.hpp"

namespace vesyla
{
namespace cidfg
{
WriteAndIndexVertex::WriteAndIndexVertex()
{
	vertex_type = WRITE_AND_INDEX_VERTEX;
	port = 0;
	is_sram = false;
}
WriteAndIndexVertex::~WriteAndIndexVertex()
{
}
} // namespace cidfg
} // namespace vesyla