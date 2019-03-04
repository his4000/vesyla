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

#ifndef __VESYLA_CIDFG_OBJECT_HPP__
#define __VESYLA_CIDFG_OBJECT_HPP__

#include "util/Common.hpp"

namespace vesyla
{
namespace cidfg
{

/**
	* The base class which derives all other classes in cidfg.
	* It just maintains an ID of any cidfg object.
	* The ID will NOT be automatically assigned while creation because temperary used objects don't worth to waste available IDs.
	* Only when being added to a cidfg graph, an Object's id field will be set. 
	* \warning This is an abstract class, one should never use this class directly.
	*/
class Object
{
public:
	static int id_counter; ///< Static property used only for generating new id
	int id;								 ///< The id in a cidfg graph

public:
	Object();
	virtual ~Object() = 0;
};

} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_OBJECT_HPP__