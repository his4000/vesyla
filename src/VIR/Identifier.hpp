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

#include <exception>
#include <string>
using namespace std;

#ifndef __Identifier_h__
#define __Identifier_h__

#include "Name.hpp"
#include "DependencyType.hpp"
#include "VIRFwDeclarations.hpp"

namespace VIR
{
// class Name;
class Identifier;
namespace VirEnumerations
{
// enum DependencyType;
}
} // namespace VIR

namespace VIR
{
class Identifier : public VIR::Name
{
private:
	string _name;

private:
	VIR::ObjectDeclaration *_object;

private:
	VIR::AssignmentStatement *_dependentTo;

private:
	VIR::VirEnumerations::DependencyType _dependencyType;

private:
	VIR::Statement *_indexDependency;

public:
	bool isFixedPointVariable;

public:
	Identifier();

public:
	string name() const;

public:
	void name(const string &name_);

public:
	VIR::ObjectDeclaration *object() const;

public:
	void object(VIR::ObjectDeclaration *object_);

public:
	VIR::AssignmentStatement *dependentTo() const;

public:
	void dependentTo(VIR::AssignmentStatement *dependentTo_);

public:
	VIR::VirEnumerations::DependencyType dependencyType() const;

public:
	void dependencyType(VIR::VirEnumerations::DependencyType dependencyType_);

public:
	VIR::Statement *indexDependency() const;

public:
	void indexDependency(VIR::Statement *indexDependency_);
};
} // namespace VIR

#endif
