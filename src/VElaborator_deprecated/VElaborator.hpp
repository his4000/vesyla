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


#ifndef _VElaborator_H_
#define _VElaborator_H_

#include "util/Common.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include <fstream>
#include <map>
#include <vector>
#include "AddressElaborator.hpp"

using std::map;
using std::ofstream;
using std::string;
using std::vector;

class ExpressionEvaluator;

class VElaborator
{
public:
	VElaborator();
	~VElaborator();

	VIR::MainProgram *elaborate(VIR::MainProgram *mainProgram_);
	void setRaccuMode(bool enable_);
	bool needDynamicManagement() const;

private:
	VIR::Array *elaborateArray(VIR::Array *array_);
	VIR::AssignmentPragma *elaborateAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_, VIR::NumericLiteral *lhsObjectValue_);
	VIR::AssignmentStatement *elaborateFunctionCallStatement(VIR::FunctionCallStatement *functionCallStatement_);
	VIR::AssignmentStatement *elaborateAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_);
	VIR::BinaryExpression *elaborateBinaryExpression(VIR::BinaryExpression *binaryExpression_);
	VIR::Complex *elaborateComplex(VIR::Complex *complex_);
	VIR::DebugStatement *elaborateDebugStatement(VIR::DebugStatement *debugStatement_);
	VIR::Expression *elaborateExpression(VIR::Expression *expression_);
	void elaborateExpressionList(vector<VIR::Expression *> *oldExpressionList_, vector<VIR::Expression *> *newExpressionList_);
	VIR::FloatingPoint *elaborateFloatingPoint(VIR::FloatingPoint *floatingPoint_);
	VIR::ForStatement *elaborateForStatement(VIR::ForStatement *forStatement_);
	VIR::Identifier *elaborateIdentifier(VIR::Identifier *identifier_, bool isLoopIndex_ = false);
	void elaborateIfCondition(VIR::IfStatement *oldIfStatement_, VIR::IfStatement *newIfStatement_);
	VIR::IfStatement *elaborateIfStatement(VIR::IfStatement *ifStatement_);
	VIR::IfThenElseStatement *elaborateIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_);
	VIR::Integer *elaborateInteger(VIR::Integer *integer_);
	VIR::MainProgram *elaborateMainProgram(VIR::MainProgram *mainProgram_);
	VIR::Expression *elaborateParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_);
	VIR::PragmaStatement *elaboratePragmaStatement(VIR::PragmaStatement *pragmaStatement_);
	VIR::RangeExpression *elaborateRangeExpression(VIR::RangeExpression *rangeExpression_);
	VIR::ReferencePragma *elaborateReferencePragma(VIR::ReferencePragma *referencePragma_);
	VIR::Expression *elaborateFunctionCall(VIR::FunctionCall *functionCall_);
	VIR::Expression *elaborateDpuPrimitiveFunctionCall(VIR::FunctionCall *functionCall_);
	VIR::Expression *elaborateAguPrimitiveFunctionCall(VIR::FunctionCall *functionCall_);
	VIR::Expression *elaborateSliceName(VIR::SliceName *sliceName_);
	VIR::Statement *elaborateStatement(VIR::Statement *statement_);
	void elaborateStatementList(vector<VIR::Statement *> *oldStatementList_, vector<VIR::Statement *> *newStatementList_);
	VIR::StringLiteral *elaborateStringLiteral(VIR::StringLiteral *stringLiteral_);
	VIR::Expression *elaborateUnaryExpression(VIR::UnaryExpression *unaryExpression_);

	typedef vector<VIR::ForStatement *> UnrolledLoops;
	typedef map<VIR::ObjectDeclaration *, int> UnrolledLoopValuesMap;

	void addToTotalStatementList(VIR::Statement *statement_);
	VIR::PrimitiveFunctionCall *createAccumulatorFunction(VIR::FunctionCall *functionCall_);
	VIR::SliceName *createSliceNameAndDummyVarForAConstant(int constantValue_, VIR::DPUPragma *assignPragma_);
	VIR::SliceName *createSliceNameForAnIdentifier(VIR::Identifier *identifier_);
	void findAssignStatementIndexDependencies(VIR::AssignmentStatement *assignmentStatement_);
	void findDataDependencies(VIR::Identifier *lhsIdentifier_, vector<VIR::Identifier *> *rhsIdentifiers_);
	VIR::ForStatement *findDependentForStatement(VIR::Statement *currStatement_);
	void findIdentifiersAndSliceNames(VIR::Expression *expression_, vector<VIR::Identifier *> *rhsIdentifiers_, vector<VIR::SliceName *> *rhsSliceNames_ = 0);
	void findIfConditionIndexDependencies(VIR::IfStatement *ifStatement_);
	void findIndexIdentifiers(VIR::Expression *expression_, vector<VIR::Identifier *> *indexIdentifiers_, bool isAddressFunction_ = false, bool isFromSliceName_ = false);
	void findLoopRangeIndexDependencies(VIR::RangeExpression *loopRangeExpression_);
	VIR::ObjectDeclaration *getIdentifierObject(const std::string &name_, bool isLoopIndex_ = false);
	void getUnrolledLoopsAndSignatures(VIR::ForStatement *currForStatement_, UnrolledLoops &unrolledLoops_, vector<string> &unrolledSignatures_);
	string getUniqueDummyName() const;
	bool isControlStatement(const VIR::Statement *statement_) const;
	bool isDpuPrimitiveFunction(const string name_) const;
	bool isAguPrimitiveFunction(const string name) const;
	void replaceIdentifiersWithSliceNames(VIR::AssignmentStatement *assignmentStatement_);
	VIR::Expression *replaceIdentifiersWithSliceNames(VIR::AssignmentStatement *assignmentStatement_, VIR::Expression *expression_);
	bool setConstantObjectValue(VIR::Identifier *lhsIdentifier_, vector<VIR::Identifier *> *rhsIdentifiers_, VIR::Expression *rhs_);
	void setDpuMode(VIR::Expression *expression_, VIR::DPUPragma *dpuPragma_);
	void setIndexDependencies(vector<VIR::Identifier *> &indexIdentifiers_);
	void setRaccuMode(VIR::Expression *expression_, VIR::RaccuPragma *raccuPragma_);
	void updateUnrolledLoopValues(const UnrolledLoops &unrolledLoops_, const string &signature_, UnrolledLoopValuesMap &unrolledLoopValues_);

	// ------------------------------------------------
	// SecondTraverse methods
	VIR::Array *traverseArray(VIR::Array *array_);
	VIR::AssignmentPragma *traverseAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_);
	VIR::AssignmentStatement *traverseAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_);
	VIR::BinaryExpression *traverseBinaryExpression(VIR::BinaryExpression *binaryExpression_);
	VIR::Complex *traverseComplex(VIR::Complex *complex_);
	VIR::DebugStatement *traverseDebugStatement(VIR::DebugStatement *debugStatement_);
	VIR::Expression *traverseExpression(VIR::Expression *expression_);
	vector<VIR::Expression *> *traverseExpressionList(vector<VIR::Expression *> *expressionList_);
	VIR::FloatingPoint *traverseFloatingPoint(VIR::FloatingPoint *floatingPoint_);
	VIR::ForStatement *traverseForStatement(VIR::ForStatement *forStatement_);
	VIR::FunctionCall *traverseFunctionCall(VIR::FunctionCall *functionCall_);
	VIR::Identifier *traverseIdentifier(VIR::Identifier *identifier_);
	VIR::IfStatement *traverseIfStatement(VIR::IfStatement *ifStatement_, bool isFromIfThenElse_ = false);
	VIR::IfThenElseStatement *traverseIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_);
	VIR::Integer *traverseInteger(VIR::Integer *integer_);
	VIR::MainProgram *traverseMainProgram(VIR::MainProgram *mainProgram_);
	VIR::ParenthesizedExpression *traverseParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_);
	VIR::PragmaStatement *traversePragmaStatement(VIR::PragmaStatement *pragmaStatement_);
	VIR::PrimitiveFunctionCall *traversePrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_);
	VIR::RangeExpression *traverseRangeExpression(VIR::RangeExpression *rangeExpression_);
	VIR::ReferencePragma *traverseReferencePragma(VIR::ReferencePragma *referencePragma_);
	VIR::SliceName *traverseSliceName(VIR::SliceName *sliceName_);
	VIR::Statement *traverseStatement(VIR::Statement *statement_);
	vector<VIR::Statement *> *traverseStatementList(vector<VIR::Statement *> *statementList_);
	VIR::StringLiteral *traverseStringLiteral(VIR::StringLiteral *stringLiteral_);
	VIR::UnaryExpression *traverseUnaryExpression(VIR::UnaryExpression *unaryExpression_);

private:
	typedef map<string, VIR::ObjectDeclaration *> IdentifiersMap;

	VIR::ControlStatement *_currControlStatement; // representing the current control statement ('for' or 'if' statement)
	VIR::ForStatement *_currForStatement;					// used to determine the degree of for loops and also it is used in the SecondTraverse.cpp file
	vector<VIR::Statement *> *_currStatementList; // the statement list of the current scope
	int _currStatementNo;													// Shows the current number of elaborated statements
	ExpressionEvaluator *_exprEvaluator;
	string _exprListSeperator;
	IdentifiersMap _identifiersMap;
	bool _isAddressVariable;
	bool _isThereDynamicLoops;
	bool _isNonConstantLoopRegion;
	bool _raccuMode;
	vector<VIR::Statement *> *_totalAssignmentStatementList; // the list of all statements
	int _usedRegFileNumber;
};

#endif // _VElaborator_H_
