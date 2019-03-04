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

#include "AssignmentStatement.hpp"
#include "Statement.hpp"
#include "AssignmentType.hpp"

VIR::AssignmentStatement::AssignmentStatement()
{
	_twinStatement = 0;
	_isLastResSharingStatement = false;
}

vector<VIR::Name *> &VIR::AssignmentStatement::lhs()
{
	return _lhs;
}

void VIR::AssignmentStatement::lhs(vector<Name *> lhs_)
{
	_lhs = lhs_;
}

VIR::Expression *VIR::AssignmentStatement::rhs() const
{
	return _rhs;
}

void VIR::AssignmentStatement::rhs(Expression *rhs_)
{
	_rhs = rhs_;
}

VIR::AssignmentPragma *VIR::AssignmentStatement::pragma() const
{
	return _pragma;
}

void VIR::AssignmentStatement::pragma(AssignmentPragma *pragma_)
{
	_pragma = pragma_;
}

// vector<VIR::SliceName*> & VIR::AssignmentStatement::rhsSliceNames() {
// 	return _rhsSliceNames;
// }

// vector<VIR::Identifier*> & VIR::AssignmentStatement::rhsIdentifiers() {
// 	return _rhsIdentifiers;
// }

// vector<VIR::Identifier *> & VIR::AssignmentStatement::lhsIdentifiers(){
// 	return _lhsIdentifiers;
// }

// vector<VIR::SliceName *> & VIR::AssignmentStatement::lhsSliceNames(){
// 	return _lhsSliceNames;
// }

VIR::VirEnumerations::AssignmentType VIR::AssignmentStatement::type() const
{
	return _type;
}

void VIR::AssignmentStatement::type(
		VIR::VirEnumerations::AssignmentType type_)
{
	_type = type_;
}

VIR::AssignmentStatement *VIR::AssignmentStatement::twinStatement() const
{
	return _twinStatement;
}

void VIR::AssignmentStatement::twinStatement(
		VIR::AssignmentStatement *twinStatement_)
{
	_twinStatement = twinStatement_;
}

bool VIR::AssignmentStatement::isLastResSharingStatement() const
{
	return _isLastResSharingStatement;
}

void VIR::AssignmentStatement::isLastResSharingStatement(
		bool isLastResSharingStatement_)
{
	_isLastResSharingStatement = isLastResSharingStatement_;
}
