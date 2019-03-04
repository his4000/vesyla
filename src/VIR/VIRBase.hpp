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

#ifndef __VIRBase_h__
#define __VIRBase_h__

#include "KindType.hpp"
#include "util/Common.hpp"

#include <string>
#include <vector>
using std::string;
using std::vector;

namespace MemorySpace
{
class MemoryManager;
}

namespace VIR
{
class VIRBase
{
	friend class MemorySpace::MemoryManager;

private:
	int _id;

private:
	int _lineNumber;

private:
	VIR::VirEnumerations::KindType _kind;

public:
	VIRBase() { _lineNumber = 0; }

public:
	int id() const;

public:
	VIR::VirEnumerations::KindType kind() const;

public:
	string kindStr() const;

public:
	int lineNumber() const;

public:
	void lineNumber(const int lineNumber_);

public:
	virtual ~VIRBase() = 0;
};
} // namespace VIR

#endif
