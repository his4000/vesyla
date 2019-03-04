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


#include "VDump.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "VManager/FileManager.hpp"
#include <iostream>

using namespace VIR;
using namespace VIR::VirEnumerations;

#define CALL_EXPRESSION_FUNC(value)    \
	case kt##value:                      \
		dump##value((value *)expression_); \
		break;
#define CALL_STATEMENT_FUNC(value)    \
	case kt##value:                     \
		dump##value((value *)statement_); \
		break;

// The argument says if the graph should be generated with subgraph style or not
VDump::VDump() : _createSubgraph(true), _isVerbose(true),
								 _cComplexNodeStyle("style=filled, color=black, fillcolor=darkslategray4"),
								 _cFloatingPointNodeStyle("style=filled, color=black, fillcolor=darkslategray3"),
								 _cIdentifierNodeStyle("shape=doublecircle, style=filled, color=black, fillcolor=gold"),
								 _cIntegerNodeStyle("style=filled, color=black, fillcolor=darkslategray2"),
								 _cPragmaEdgeStyle("fontcolor=darkgreen, color=darkgreen, style=dotted"),
								 _cPragmaNodeStyle("penwidth=2, fontcolor=darkgreen, color=darkgreen, fillcolor=beige, style=filled"),
								 _cStatementEdgeStyle("fontcolor=blue4, color=darkorchid1"),
								 _cUniqueStringNodeStyle("style=filled, color=black, fillcolor=gray90, ")
{
	setIncludeInNormalMode();
}

// This method creates a make file for creating graphs from DOT files using GraphViz.
void VDump::dumpMakeGraph()
{
	SetCurrentFile(ftMakeGraph);

	string remStr, comment;
	string virDotStr, virElabDotStr;
	FileType virFileType, virElabFileType;

	for (int index = 1; index <= 4; ++index)
	{
		switch (index)
		{
		case 1:
			virFileType = ftVirDot1;
			virElabFileType = ftVirElabDot1;
			comment = ":: <verbose=false, subgraph=false>";
			break;
		case 2:
			virFileType = ftVirDot2;
			virElabFileType = ftVirElabDot2;
			comment = ":: <verbose=false, subgraph=true>";
			break;
		case 3:
			virFileType = ftVirDot3;
			virElabFileType = ftVirElabDot3;
			comment = ":: <verbose=true, subgraph=false>";
			break;
		case 4:
			virFileType = ftVirDot4;
			virElabFileType = ftVirElabDot4;
			comment = ":: <verbose=true, subgraph=true>";
			break;
		}

		virDotStr = GetFileName(virFileType, false);
		virElabDotStr = GetFileName(virElabFileType, false);

		remStr = index != 1 ? ":: " : "";

		WriteLine(comment);
		WriteLine(remStr + "..\\graphviz-dot\\dot.exe -Tpdf " + virDotStr + " -o " + virDotStr.substr(0, virDotStr.find('.')) + ".pdf");
		WriteLine(remStr + "..\\graphviz-dot\\dot.exe -Tpdf " + virElabDotStr + " -o " + virElabDotStr.substr(0, virElabDotStr.find('.')) + ".pdf");
		WriteLine("");
		WriteLine("rem ..\\graphviz-dot\\dot.exe -Tpng " + virDotStr + " -o " + virDotStr.substr(0, virDotStr.find('.')) + ".png");
		WriteLine("rem ..\\graphviz-dot\\dot.exe -Tpng " + virElabDotStr + " -o " + virElabDotStr.substr(0, virElabDotStr.find('.')) + ".png");
		WriteLine("");
	}

	WriteLine("pause");

	RestoreCurrentFile();
}

// This is the main entrance method for dumping
// mainProgram_ is the pointer to the main program object.
// isElabDump_: It says if this function is used for dumping the original data, or the data after elaboration.
// Dump is performed in four cases:
// 1: (verbose=F, subgraph=F); 2: (verbose=F, subgraph=T); 3: (verbose=T, subgraph=F); 4: (verbose=T, subgraph=T);
// isVerbose determines if the mainProgram objects should be dumped deeply. (default is true)
// createSubgraph determines if for each different part of the main program there should be a distinct subgraph. (default is true)
void VDump::dumpProgram(MainProgram *mainProgram_, const bool isElabDump_)
{
	_currDumpDsFileType = isElabDump_ ? ftDumpElabDs : ftDumpDs;

	int index;
	FileType fileType;

	for (int verbose = 0; verbose <= 1; ++verbose)
		for (int subgraph = 0; subgraph <= 1; ++subgraph)
		{
			_isVerbose = verbose ? true : false;
			_createSubgraph = subgraph ? true : false;
			_dumpDsEnabled = !verbose && !subgraph ? true : false;

			index = 2 * verbose + subgraph + 1;

			switch (index)
			{
			case 1:
				fileType = isElabDump_ ? ftVirElabDot1 : ftVirDot1;
				break;
			case 2:
				fileType = isElabDump_ ? ftVirElabDot2 : ftVirDot2;
				break;
			case 3:
				fileType = isElabDump_ ? ftVirElabDot3 : ftVirDot3;
				break;
			case 4:
				fileType = isElabDump_ ? ftVirElabDot4 : ftVirDot4;
				break;
			}

			SetCurrentFile(fileType);

			WriteLine("digraph G {");

			dumpMainProgram(mainProgram_);

			WriteLine("}");

			RestoreCurrentFile();
		}
}

// -------------------------------------
// ---------- Private Methods ----------

void VDump::dumpAddressPragma(VIR::AddressPragma *addressPragma_)
{
	wtDumpDsLine("ADDRESS_VARIABLE");
}

void VDump::dumpArray(Array *array_)
{
	wtDumpDsLine("Left Hand Side:");
	dumpExpressionList(&array_->elements(), getNodeSignature(array_));
}

void VDump::dumpAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_)
{
	createGraphNode(assignmentPragma_);
	setGraphNodeStyle(getNodeSignature(assignmentPragma_), _cPragmaNodeStyle);

	wtDumpDsLine("Type: " + assignmentPragma_->kindStr() + "\n");

	switch (assignmentPragma_->kind())
	{
	case ktRaccuPragma:
		dumpRaccuPragma((VIR::RaccuPragma *)assignmentPragma_);
		break;

	case ktAddressPragma:
		dumpAddressPragma((VIR::AddressPragma *)assignmentPragma_);
		break;

	case ktTempVarPragma:
		dumpTempVarPragma((VIR::TempVarPragma *)assignmentPragma_);
		break;

	default:
		dumpReferencePragma((VIR::ReferencePragma *)assignmentPragma_);
		break;
	}
}

void VDump::dumpAssignmentStatement(AssignmentStatement *assignmentStatement_)
{
	wtDumpDsLine("Left Hand Side:");
	for (auto sn : assignmentStatement_->lhs())
	{
		createGraphEdge(getNodeSignature(assignmentStatement_), getNodeSignature(sn), "LHS");
		dumpExpression(sn);
	}

	wtDumpDsLine("Right Hand Side:");
	createGraphEdge(getNodeSignature(assignmentStatement_), getNodeSignature(assignmentStatement_->rhs()), "RHS");
	dumpExpression(assignmentStatement_->rhs());

	if (isDumpEnabled("assignment_pragma") && assignmentStatement_->pragma())
	{
		wtDumpDsLine("Pragma: ");

		createGraphEdge(getNodeSignature(assignmentStatement_), getNodeSignature(assignmentStatement_->pragma()), "pragma", _cPragmaEdgeStyle);

		dumpAssignmentPragma(assignmentStatement_->pragma());
	}

	/* // DEBUGGING
	string lhsName = (assignmentStatement_->lhs()->kind() == ktIdentifier) ? 
		((Identifier *)assignmentStatement_->lhs())->name() :
		((SliceName *)assignmentStatement_->lhs())->prefix()->name();

	std::cout << "\nLHS Identifiers for assignment " << assignmentStatement_->id() << " is: " << lhsName << " and RHS identifiers are: \n";
	for (int i = 0; i < assignmentStatement_->rhsIdentifiers().size(); ++i)
		std::cout << assignmentStatement_->rhsIdentifiers().identifier(i)->name() << ", ";
	*/
}

void VDump::dumpBinaryExpression(BinaryExpression *binaryExpression_)
{
	string opStr = binaryExpression_->operatorTypeStr();

	wtDumpDsLine("operator: " + opStr);
	createGraphEdge(getNodeSignature(binaryExpression_), getNodeSignature(binaryExpression_), opStr);

	wtDumpDsLine("left operand: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(binaryExpression_), getNodeSignature(binaryExpression_->leftOperand()), "left operand");
	dumpExpression(binaryExpression_->leftOperand());
	decTabWithBrace();

	wtDumpDsLine("right operand: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(binaryExpression_), getNodeSignature(binaryExpression_->rightOperand()), "right operand");
	dumpExpression(binaryExpression_->rightOperand());
	decTabWithBrace();
}

void VDump::dumpComplex(Complex *complex_)
{
	wtDumpDsLine("value: " + to_string(complex_->iValue));
	createGraphEdge(getNodeSignature(complex_), getNodeSignature(complex_), to_string(complex_->iValue));

	setGraphNodeStyle(getNodeSignature(complex_), _cComplexNodeStyle);
}

void VDump::dumpDebugStatement(DebugStatement *debugStatement_)
{
	// TODO:
}

void VDump::dumpExpression(Expression *expression_)
{
	wtDumpDsLine("Expression: " + getNodeSignature(expression_));

	incTabWithBrace();

	createGraphNode(expression_);

	switch (expression_->kind())
	{
		CALL_EXPRESSION_FUNC(Array);
		CALL_EXPRESSION_FUNC(BinaryExpression);
		CALL_EXPRESSION_FUNC(Complex);
		CALL_EXPRESSION_FUNC(FloatingPoint);
		CALL_EXPRESSION_FUNC(FunctionCall);
		CALL_EXPRESSION_FUNC(Identifier);
		CALL_EXPRESSION_FUNC(Integer);
		CALL_EXPRESSION_FUNC(ParenthesizedExpression);
		CALL_EXPRESSION_FUNC(PrimitiveFunctionCall);
		CALL_EXPRESSION_FUNC(RangeExpression);
		CALL_EXPRESSION_FUNC(SliceName);
		CALL_EXPRESSION_FUNC(StringLiteral);
		CALL_EXPRESSION_FUNC(UnaryExpression);
	}

	decTabWithBrace();
}

void VDump::dumpExpressionList(vector<VIR::Expression *> *expressionList_, const string &topNodeSignature_)
{
	incTabWithBrace();

	createGraphListNode(topNodeSignature_, "Expression");

	createGraphEdge(topNodeSignature_, getListNodeSignature(topNodeSignature_), "list");

	const int size = expressionList_->size();

	for (int index = 0; index < size; ++index)
	{
		Expression *expression = (*expressionList_)[index];

		createGraphEdge(getListNodeSignature(topNodeSignature_), getNodeSignature(expression), "e" + to_string(index));
		dumpExpression(expression);
	}

	decTabWithBrace();
}

void VDump::dumpFloatingPoint(FloatingPoint *floatingPoint_)
{
	wtDumpDsLine("value: " + to_string(floatingPoint_->value));
	createGraphEdge(getNodeSignature(floatingPoint_), getNodeSignature(floatingPoint_), to_string(floatingPoint_->value));

	setGraphNodeStyle(getNodeSignature(floatingPoint_), _cFloatingPointNodeStyle);
}

void VDump::dumpForStatement(ForStatement *forStatement_)
{
	wtDumpDsLine("loop variable: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(forStatement_), getNodeSignature(forStatement_->loopVariable()), "variable");
	dumpExpression(forStatement_->loopVariable());
	decTabWithBrace();

	wtDumpDsLine("loop bound: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(forStatement_), getNodeSignature(forStatement_->loopRange()), "range");
	dumpExpression(forStatement_->loopRange());
	decTabWithBrace();

	if (isDumpEnabled("for"))
	{
		string parallelModeStr = (forStatement_->parallelMode() == pmSequential) ? "sequential" : "parallel";
		string modeSignature = parallelModeStr + "_" + to_string(forStatement_->id());

		wtDumpDsLine("parallel mode: " + parallelModeStr + "\n");
		createGraphEdge(getNodeSignature(forStatement_), modeSignature, "parallel mode", _cPragmaEdgeStyle);
		setGraphNodeStyle(modeSignature, _cPragmaNodeStyle + ", label=" + parallelModeStr);

		wtDumpDsLine("loop degree: " + to_string(forStatement_->degree()));
		createGraphEdge(getNodeSignature(forStatement_), uniqueStringNode(to_string(forStatement_->degree())), "degree");

		wtDumpDsLine("loop repetition: " + to_string(forStatement_->repetition()));
		createGraphEdge(getNodeSignature(forStatement_), uniqueStringNode(to_string(forStatement_->repetition())), "repetition");
	}

	wtDumpDsLine("loop body: ");
	dumpStatementList(&forStatement_->loopBody(), getNodeSignature(forStatement_));
}

void VDump::dumpFunctionCall(FunctionCall *functionCall_)
{
	// TODO:
}

void VDump::dumpIdentifier(Identifier *identifier_)
{
	wtDumpDsLine("value: " + identifier_->name());
	createGraphEdge(getNodeSignature(identifier_), identifier_->name(), "name");

	setGraphNodeStyle(identifier_->name(), _cIdentifierNodeStyle);

	if (isDumpEnabled("object"))
	{
		wtDumpDsLine("object: NULL");
		createGraphEdge(getNodeSignature(identifier_), getNodeSignature(identifier_->object()), "object");
		dumpObjectDeclaration(identifier_->object());
	}

	if (isDumpEnabled("data_dependency") && identifier_->dependentTo())
	{
		;
		//		COMMENTED FOR DEBUG
		//		2017-11-13
		//
		//		wtDumpDsLine("dependent to: " + identifier_->dependentTo()->lhsIdentifiers()[0]->name());
		//		createGraphEdge(getNodeSignature(identifier_), getNodeSignature(identifier_->dependentTo()), "dependent to");
		//
		//		/* // DEBUGGING
		//		std::cout << "\n" << getNodeSignature(identifier_) << " is dependent to " << getNodeSignature(identifier_->dependentTo())
		//				  << ", with type: " << identifier_->dependencyType() << "\n";
		//		*/
	}

	if (isDumpEnabled("index_dependency") && identifier_->indexDependency())
	{
		wtDumpDsLine("Index dependent to: " + getNodeSignature(identifier_->indexDependency()));
		createGraphEdge(getNodeSignature(identifier_), getNodeSignature(identifier_->indexDependency()), "index dependent to");
	}
}

void VDump::dumpIfStatement(IfStatement *ifStatement_)
{
	wtDumpDsLine("condition: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(ifStatement_), getNodeSignature(ifStatement_->condition()), "condition");
	dumpExpression(ifStatement_->condition());
	decTabWithBrace();

	if (isDumpEnabled("if"))
	{
		string modeStr = (ifStatement_->mode() == imConditional) ? "conditional" : "structural";
		string modeSignature = modeStr + "_" + to_string(ifStatement_->id());

		wtDumpDsLine("mode: " + modeStr + "\n");
		createGraphEdge(getNodeSignature(ifStatement_), modeSignature, "mode", _cPragmaEdgeStyle);
		setGraphNodeStyle(modeSignature, _cPragmaNodeStyle + ", label=" + modeStr);
	}

	DPUPragma *conditionPragma = ifStatement_->conditionPragma();

	if (isDumpEnabled("if_pragma") && conditionPragma)
	{
		wtDumpDsLine("Pragma: ");

		createGraphEdge(getNodeSignature(ifStatement_), getNodeSignature(conditionPragma), "pragma", _cPragmaEdgeStyle);

		createGraphNode(conditionPragma);
		setGraphNodeStyle(getNodeSignature(conditionPragma), _cPragmaNodeStyle);
		dumpReferencePragma(conditionPragma);
	}

	wtDumpDsLine("then part: ");
	dumpStatementList(&ifStatement_->thenPart(), getNodeSignature(ifStatement_));
}

void VDump::dumpIfThenElseStatement(IfThenElseStatement *ifThenElseStatement_)
{
	dumpIfStatement(ifThenElseStatement_);

	wtDumpDsLine("else part: ");
	dumpStatementList(&ifThenElseStatement_->elsePart(), getNodeSignature(ifThenElseStatement_));
}

void VDump::dumpInteger(Integer *integer_)
{
	wtDumpDsLine("value: " + to_string(integer_->value));
	createGraphEdge(getNodeSignature(integer_), getNodeSignature(integer_), to_string(integer_->value));

	setGraphNodeStyle(getNodeSignature(integer_), _cIntegerNodeStyle);
}

void VDump::dumpMainProgram(MainProgram *mainProgram_)
{
	wtDumpDsLine("Main Program : " + mainProgram_->name());

	openSubgraph();
	createGraphNode(mainProgram_);
	createGraphListNode(getNodeSignature(mainProgram_), "Statement");
	closeSubgraph();

	dumpStatementList(&mainProgram_->body(), getNodeSignature(mainProgram_));
}

void VDump::dumpObjectDeclaration(VIR::ObjectDeclaration *objectDeclaration_)
{
	if (!objectDeclaration_)
	{
		return;
	}

	wtDumpDsLine("name: " + objectDeclaration_->name());

	ObjectDeclarationMap::const_iterator iter = objectDeclarationNodes.find(objectDeclaration_);

	if (iter == objectDeclarationNodes.end())
	{
		objectDeclarationNodes[objectDeclaration_] = objectDeclaration_;

		createGraphNode(objectDeclaration_);

		wtDumpDsLine("value: ");
		if (objectDeclaration_->value())
		{
			createGraphEdge(getNodeSignature(objectDeclaration_), getNodeSignature(objectDeclaration_->value()), "value");
			dumpExpression(objectDeclaration_->value());
		}

		string objectTypeStr;

		switch (objectDeclaration_->objectType())
		{
		case otAffineAddressFunction:
			objectTypeStr = "AffineAddressFunction";
			break;
		case otConstant:
			objectTypeStr = "Constant";
			break;
		case otConstantLoopIndex:
			objectTypeStr = "ConstantLoopIndex";
			break;
		case otNonAffineAddressFunction:
			objectTypeStr = "NonAffineAddressFunction";
			break;
		case otNonConstantLoopIndex:
			objectTypeStr = "NonConstantLoopIndex";
			break;
		case otNone:
			objectTypeStr = "None";
			break;
		case otRegisterFile:
			objectTypeStr = "RegisterFile";
			break;
		case otMemory:
			objectTypeStr = "Memory";
			break;
		}

		wtDumpDsLine("Object Type: " + objectTypeStr);
		createGraphEdge(getNodeSignature(objectDeclaration_), uniqueStringNode(objectTypeStr), "object type");

		if (isDumpEnabled("object_pragma") && objectDeclaration_->pragma())
		{
			wtDumpDsLine("Object has pragma!");
			createGraphEdge(getNodeSignature(objectDeclaration_), getNodeSignature(objectDeclaration_->pragma()), "pragma", _cPragmaEdgeStyle);
		}
	}
}

void VDump::dumpParenthesizedExpression(ParenthesizedExpression *parenthesizedExpression_)
{
	incTabWithBrace();
	createGraphEdge(getNodeSignature(parenthesizedExpression_), getNodeSignature(parenthesizedExpression_->value()), "value");
	dumpExpression(parenthesizedExpression_->value());
	decTabWithBrace();
}

void VDump::dumpPragmaStatement(VIR::PragmaStatement *pragmaStatement_)
{
	// do nothing
}

void VDump::dumpPrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_)
{
	wtDumpDsLine("function_name: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(primitiveFunctionCall_), getNodeSignature(primitiveFunctionCall_->name()), "name");
	dumpExpression(primitiveFunctionCall_->name());
	decTabWithBrace();

	wtDumpDsLine("parameters: ");
	dumpExpressionList(&primitiveFunctionCall_->parameters(), getNodeSignature(primitiveFunctionCall_));
}

void VDump::dumpRaccuPragma(VIR::RaccuPragma *raccuPragma_)
{
	wtDumpDsLine("RACCU_VARIABLE");
}

void VDump::dumpRangeExpression(RangeExpression *rangeExpression_)
{
	wtDumpDsLine("begin: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(rangeExpression_), getNodeSignature(rangeExpression_->begin()), "begin");
	dumpExpression(rangeExpression_->begin());
	decTabWithBrace();

	wtDumpDsLine("end: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(rangeExpression_), getNodeSignature(rangeExpression_->end()), "end");
	dumpExpression(rangeExpression_->end());
	decTabWithBrace();

	wtDumpDsLine("increment: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(rangeExpression_), getNodeSignature(rangeExpression_->increment()), "increment");
	dumpExpression(rangeExpression_->increment());
	decTabWithBrace();
}

void VDump::dumpReferencePragma(VIR::ReferencePragma *referencePragma_)
{
	wtDumpDsLine("References: ");
	dumpExpressionList(&referencePragma_->reference(), getNodeSignature(referencePragma_));

	if (referencePragma_->kind() == ktStoragePragma)
		dumpStoragePragma((StoragePragma *)referencePragma_);
}

void VDump::dumpStoragePragma(VIR::StoragePragma *storagePragma_)
{
	string distStr = "";

	switch (storagePragma_->distribution())
	{
	case sdEvenDist:
		distStr = "EvenDistribution";
		break;
	case sdFullDist:
		distStr = "FullDistribution";
		break;
	case sdCustomDist:
		distStr = "CustomDistribution";
		break;
	}

	string distSignature = distStr + "_" + to_string(storagePragma_->id());

	wtDumpDsLine("distribution: " + distStr + "\n");
	createGraphEdge(getNodeSignature(storagePragma_), distSignature, "distribution", _cPragmaEdgeStyle);
	setGraphNodeStyle(distSignature, _cPragmaNodeStyle + ", label=" + distStr);
}

void VDump::dumpSliceName(SliceName *sliceName_)
{
	wtDumpDsLine("prefix: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(sliceName_), getNodeSignature(sliceName_->prefix()), "prefix");
	dumpExpression(sliceName_->prefix());
	decTabWithBrace();

	wtDumpDsLine("suffix: ");
	dumpExpressionList(&sliceName_->suffix(), getNodeSignature(sliceName_));
}

void VDump::dumpStatement(Statement *statement_)
{
	wtDumpDsLine("Statement: " + getNodeSignature(statement_));

	incTabWithBrace();

	openSubgraph();

	createGraphNode(statement_);

	// Create a node for statements that have statement list
	if (statement_->kind() == ktForStatement || statement_->kind() == ktIfStatement ||
			statement_->kind() == ktIfThenElseStatement)
		createGraphListNode(getNodeSignature(statement_), "Statement");

	closeSubgraph();

	switch (statement_->kind())
	{
		CALL_STATEMENT_FUNC(AssignmentStatement);
		CALL_STATEMENT_FUNC(DebugStatement);
		CALL_STATEMENT_FUNC(ForStatement);
		CALL_STATEMENT_FUNC(IfStatement);
		CALL_STATEMENT_FUNC(IfThenElseStatement);
		CALL_STATEMENT_FUNC(PragmaStatement);
	}

	if (isDumpEnabled("control_dependency") && statement_->controlDependentTo())
	{
		//logMessage("control dependent to: " + statement_->controlDependentTo());
		createGraphEdge(getNodeSignature(statement_), getNodeSignature(statement_->controlDependentTo()), "control dependent to");

		/* // DEBUGGING
		std::cout << "\n" << getNodeSignature(statement_) << " is control dependent to: ";
		std::cout << ((statement_->controlDependentTo()) ? getNodeSignature(statement_->controlDependentTo()) : "NULL ") << "\n";
		*/
	}

	decTabWithBrace();
}

void VDump::dumpStatementList(vector<VIR::Statement *> *statementList_, const string &topNodeSignature_)
{
	incTabWithBrace();

	createGraphEdge(topNodeSignature_, getListNodeSignature(topNodeSignature_), "list", _cStatementEdgeStyle);

	const int size = statementList_->size();

	for (int index = 0; index < size; ++index)
	{
		Statement *statement = (*statementList_)[index];

		openSubgraph();

		createGraphEdge(getListNodeSignature(topNodeSignature_), getNodeSignature(statement), "statement", _cStatementEdgeStyle);
		dumpStatement(statement);

		closeSubgraph();
	}

	decTabWithBrace();
}

void VDump::dumpStringLiteral(StringLiteral *stringLiteral_)
{
	wtDumpDsLine("value: " + stringLiteral_->value);
	createGraphEdge(getNodeSignature(stringLiteral_), getNodeSignature(stringLiteral_), stringLiteral_->value);
}

void VDump::dumpTempVarPragma(VIR::TempVarPragma *tempVarPragma_)
{
	wtDumpDsLine("TEMP_VARIABLE");
}

void VDump::dumpUnaryExpression(UnaryExpression *unaryExpression_)
{
	string opStr = unaryExpression_->operatorTypeStr();

	wtDumpDsLine("operator: " + opStr);
	createGraphEdge(getNodeSignature(unaryExpression_), getNodeSignature(unaryExpression_), opStr);

	wtDumpDsLine("operand: ");
	incTabWithBrace();
	createGraphEdge(getNodeSignature(unaryExpression_), getNodeSignature(unaryExpression_->operand()), "operand");
	dumpExpression(unaryExpression_->operand());
	decTabWithBrace();
}

// -------------------------------------
// ------- Other Private Methods -------

void VDump::closeSubgraph()
{
	if (_createSubgraph)
		WriteLine("} \n");
}

void VDump::createGraphEdge(const string &leftNode_, const string &rightNode_, const string &label_, const string &style_)
{
	string label = label_ == "" ? "" : ", label=\"" + label_ + "\"";
	string style = style_ == "" ? "fontcolor=blue, color=brown" : style_;

	WriteLine("\t" + leftNode_ + " -> " + rightNode_ + " [ " + style + label + " ];");
}

void VDump::createGraphListNode(const string &nodeSignature_, const string &label_)
{
	setGraphNodeStyle("List_" + nodeSignature_, "fontcolor=dodgerblue4, color=blue4, label=\"" + label_ + " List\"");
}

void VDump::createGraphNode(const VIRBase *node_)
{
	if (!node_)
		return;
	setGraphNodeStyle(getNodeSignature(node_), "fontcolor=gray1, color=blue4, label=\"" +
																								 node_->kindStr() + " (" + to_string(node_->id()) + ")\"");
}

// This method decrements the tab str position.
// It also closes a logging line, if log message is enabled.
void VDump::decTabWithBrace()
{
	DecTab();

	wtDumpDsLine("}\n");
}

string VDump::getListNodeSignature(const string &topNodeSignature_)
{
	return "List_" + topNodeSignature_;
}

string VDump::getNodeSignature(const VIRBase *node_)
{
	return string(node_ ? node_->kindStr() + "_" + to_string(node_->id()) : uniqueStringNode("NULL"));
}

// This method increments the tab str position.
// It also opens a logging line, if log message is enabled.
void VDump::incTabWithBrace()
{
	wtDumpDsLine("{");

	IncTab();
}

// This method determines if the parts corresponding to the given key should be dumped or not.
bool VDump::isDumpEnabled(const string &key)
{
	return _includeInNormalMode[key] || _isVerbose;
}

void VDump::openSubgraph()
{
	if (_createSubgraph)
	{
		static unsigned int subgraphNum = 0;

		++subgraphNum;

		WriteLine("subgraph cluster" + to_string(subgraphNum));
		WriteLine("{");
	}
}

void VDump::setGraphNodeStyle(const string &node_, const string &style_)
{
	WriteLine("\t" + node_ + " [ " + style_ + " ];");
}

// This method sets the _includeInNormalMode map that determines which construct
// should be included in dumping during normal mode (compare to verbose mode).
// TODO: later you can use this method as a public method to configure dump strategy from outside.
void VDump::setIncludeInNormalMode()
{
	_includeInNormalMode["assignment_pragma"] = false;
	_includeInNormalMode["control_dependency"] = false;
	_includeInNormalMode["data_dependency"] = false;
	_includeInNormalMode["index_dependency"] = false;
	_includeInNormalMode["for"] = false;
	_includeInNormalMode["if"] = true;
	_includeInNormalMode["if_pragma"] = true;
	_includeInNormalMode["object"] = true;
	_includeInNormalMode["object_pragma"] = false;
}

string VDump::uniqueStringNode(const string &str)
{
	static int uniqueId = -1;

	string nodeSignature = str + to_string(++uniqueId);

	setGraphNodeStyle(nodeSignature, _cUniqueStringNodeStyle + "label=" + str);

	return nodeSignature;
}

// This method writes the given line to the DumpDs/DumpElabDs
void VDump::wtDumpDsLine(const string &line_)
{
	if (_dumpDsEnabled)
		WriteLineToF(line_, _currDumpDsFileType, true);
}

// This method writes a token string to the current active file.
void VDump::wtToken(const string &token_, const int width_ /*= -1*/)
{
	WriteToken(token_, width_);
}
