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
using namespace std;

#ifndef __AssignmentStatement_h__
#define __AssignmentStatement_h__

#include "Statement.hpp"
#include "AssignmentType.hpp"
#include "Name.hpp"
#include "SliceName.hpp"
#include "Identifier.hpp"
#include "BIR/Instruction.hpp"

namespace VIR
{
// class Statement;
class AssignmentStatement;
namespace VirEnumerations
{
// enum AssignmentType;
}
} // namespace VIR

namespace VIR
{
class AssignmentStatement : public VIR::Statement
{
public:
	vector<VIR::Name *> _lhs;
	VIR::Expression *_rhs;
	VIR::AssignmentPragma *_pragma;
	//vector<SliceName *> _rhsSliceNames;
	//vector<Identifier *> _rhsIdentifiers;
	//vector<VIR::Identifier *> _lhsIdentifiers;
	//vector<VIR::SliceName *> _lhsSliceNames;
	VIR::VirEnumerations::AssignmentType _type;
	VIR::AssignmentStatement *_twinStatement;
	bool _isLastResSharingStatement;
	string function_name;

public:
	vector<BIR::Instruction> translated_instructions;
	int corresponding_cidfg_vertex;

public:
	AssignmentStatement();

public:
	vector<VIR::Name *> &lhs();
	void lhs(vector<Name *> lhs_);
	VIR::Expression *rhs() const;
	void rhs(Expression *rhs_);
	VIR::AssignmentPragma *pragma() const;
	void pragma(AssignmentPragma *pragma_);
	vector<SliceName *> &rhsSliceNames();
	vector<Identifier *> &rhsIdentifiers();
	vector<SliceName *> &lhsSliceNames();
	vector<VIR::Identifier *> &lhsIdentifiers();

	VIR::VirEnumerations::AssignmentType type() const;
	void type(VIR::VirEnumerations::AssignmentType type_);

	VIR::AssignmentStatement *twinStatement() const;
	void twinStatement(VIR::AssignmentStatement *twinStatement_);
	bool isLastResSharingStatement() const;
	void isLastResSharingStatement(bool isLastResSharingStatement_);
};
} // namespace VIR

#endif
