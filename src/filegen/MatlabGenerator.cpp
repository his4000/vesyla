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

#include "MatlabGenerator.hpp"
namespace vesyla
{
namespace filegen
{
void MatlabGenerator::generate(VIR::MainProgram *p_, string directory_)
{
	gen_print_assign_statement(directory_ + "printAssignStatement.m");
	gen_print_assign_prim_func1(directory_ + "printAssignStatementPrimFunc1.m");
	gen_print_assign_prim_func2(directory_ + "printAssignStatementPrimFunc2.m");
	gen_print_assign_prim_func3(directory_ + "printAssignStatementPrimFunc3.m");
	gen_print_assign_wt(directory_ + "printAssignStatementWT.m");
	gen_print_for_statement(directory_ + "printForStatement.m");
	gen_print_result(directory_ + "printResult.m");
	gen_print_storage_transfer(directory_ + "printStorageTransfer.m");
	gen_print_value(directory_ + "printValue.m");
	gen_print_variable(directory_ + "printVariable.m");
	gen_instrumented_code(p_, directory_ + "instrumented_code.m");
}

void MatlabGenerator::gen_print_assign_statement(string filename_)
{
	string str = R"(
function printAssignStatement(fileID, op1Str, op1Idx, op1IdxStr, op1Val, op2Str, op2Idx, op2IdxStr, op2Val, outStr, outIdx, outIdxStr, outVal, operatorStr, isFixedPoint)

	%fprintf(fileID, '\n\n{ %s(%d) = %s(%d) %s %s(%d) --->', outStr, outIdx, op1Str, op1Idx, operatorStr, op2Str, op2Idx);
	%fprintf(fileID, '\t%s(%d) = %d; %s(%d) = %d; %s(%d) = %d; }', outStr, outIdx, outVal, op1Str, op1Idx, op1Val, op2Str, op2Idx, op2Val);

	if length(op1Idx) == 1
		fprintf(fileID, '\n\n{ %s(%d) = %s(%d) %s %s(%d) }', outStr, outIdx, op1Str, op1Idx, operatorStr, op2Str, op2Idx);
		fprintf(fileID, '\nRD: %s(%2d) = %s;\nRD: %s(%2d) = %s;', op1Str, op1Idx, printValue(op1Val, isFixedPoint), op2Str, op2Idx, printValue(op2Val, isFixedPoint));
	else
		fprintf(fileID, '\n\n{ %s(%s) = %s(%s) %s %s(%s) }', outStr, outIdxStr, op1Str, op1IdxStr, operatorStr, op2Str, op2IdxStr);
		fprintf(fileID, '\nRD: %s(%s) = ', op1Str, op1IdxStr);
		fprintf(fileID, '%s; ', printValue(op1Val, isFixedPoint));
		fprintf(fileID, '\nRD: %s(%s) = ', op2Str, op2IdxStr);
		fprintf(fileID, '%s; ', printValue(op2Val, isFixedPoint));
	end
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_assign_prim_func1(string filename_)
{
	string str = R"(
function printAssignStatementPrimFunc1(fileID, funcName, op1Str, op1Idx, op1IdxStr, op1Val, outStr, outIdx, outIdxStr, outVal, isFixedPoint)

	if length(op1Idx) == 1
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%d) = %s(%s(%d)) }', outStr{1}, outIdx{1}, funcName, op1Str, op1Idx);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%d)', outStr{i}, outIdx{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%d)) }', funcName, op1Str, op1Idx);
		end
		fprintf(fileID, '\nRD: %s(%2d) = %s;', op1Str, op1Idx, printValue(op1Val, isFixedPoint));
	else
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%s) = %s(%s(%s)) }', outStr{1}, outIdxStr{1}, funcName, op1Str, op1IdxStr);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%s)', outStr{i}, outIdxStr{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%s)) }', funcName, op1Str, op1IdxStr);
		end
		fprintf(fileID, '\nRD: %s(%s) = ', op1Str, op1IdxStr);
		fprintf(fileID, '%s; ', printValue(op1Val, isFixedPoint));
	end
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_assign_prim_func2(string filename_)
{
	string str = R"(
function printAssignStatementPrimFunc2(fileID, funcName, op1Str, op1Idx, op1IdxStr, op1Val, op2Str, op2Idx, op2IdxStr, op2Val, outStr, outIdx, outIdxStr, outVal, operatorStr, isFixedPoint)

	if length(op1Idx) == 1
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%d) = %s(%s(%d)) }', outStr{1}, outIdx{1}, funcName, op1Str, op1Idx);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%d)', outStr{i}, outIdx{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%d) %s %s(%d)) }', funcName, op1Str, op1Idx, operatorStr, op2Str, op2Idx);
		end
		fprintf(fileID, '\nRD: %s(%2d) = %s;\nRD: %s(%2d) = %s;', op1Str, op1Idx, printValue(op1Val, isFixedPoint), op2Str, op2Idx, printValue(op2Val, isFixedPoint));
	else
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%s) = %s(%s(%s)) }', outStr{1}, outIdxStr{1}, funcName, op1Str, op1IdxStr);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%s)', outStr{i}, outIdxStr{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%s) %s %s(%s)) }', outStr, outIdxStr, funcName, op1Str, op1IdxStr, operatorStr, op2Str, op2IdxStr);
		end
		fprintf(fileID, '\nRD: %s(%s) = ', op1Str, op1IdxStr);
		fprintf(fileID, '%s; ', printValue(op1Val, isFixedPoint));
		fprintf(fileID, '\nRD: %s(%s) = ', op2Str, op2IdxStr);
		fprintf(fileID, '%s; ', printValue(op2Val, isFixedPoint));
	end
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_assign_prim_func3(string filename_)
{
	string str = R"(
function printAssignStatementPrimFunc3(fileID, funcName, op1Str, op1Idx, op1IdxStr, op1Val, op2Str, op2Idx, op2IdxStr, op2Val, op3Str, op3Idx, op3IdxStr, op3Val, outStr, outIdx, outIdxStr, outVal, operatorStr1, operatorStr2, isFixedPoint)

	if length(op1Idx) == 1
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%d) = %s(%s(%d)) }', outStr{1}, outIdx{1}, funcName, op1Str, op1Idx);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%d)', outStr{i}, outIdx{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%d) %s (%s(%d) %s %s(%d))) }', outStr, outIdx, funcName, op1Str, op1Idx, operatorStr1, op2Str, op2Idx, operatorStr2, op3Str, op3Idx);
		end
		fprintf(fileID, '\nRD: %s(%2d) = %s;\nRD: %s(%2d) = %s;\nRD: %s(%2d) = %s;', op1Str, op1Idx, printValue(op1Val, isFixedPoint), op2Str, op2Idx, printValue(op2Val, isFixedPoint), op3Str, op3Idx, printValue(op3Val, isFixedPoint));
	else
		if(numel(outStr) == 1)
			fprintf(fileID, '\n\n{ %s(%s) = %s(%s(%s)) }', outStr{1}, outIdxStr{1}, funcName, op1Str, op1IdxStr);
		else
			fprintf(fileID, '\n\n{ [');
			for i=1:numel(outStr)
				fprintf(fileID, '%s(%s)', outStr{i}, outIdxStr{i});
				if(i == length(outStr))
					fprintf(fileID, ']');
				else
					fprintf(fileID, ', ');
				end
			end
			fprintf(fileID, ' = %s(%s(%s) %s (%s(%s) %s %s(%s))) }', outStr, outIdxStr, funcName, op1Str, op1IdxStr, operatorStr1, op2Str, op2IdxStr, operatorStr2, op3Str, op3IdxStr);
		end
		fprintf(fileID, '\nRD: %s(%s) = ', op1Str, op1IdxStr);
		fprintf(fileID, '%s; ', printValue(op1Val, isFixedPoint));
		fprintf(fileID, '\nRD: %s(%s) = ', op2Str, op2IdxStr);
		fprintf(fileID, '%s; ', printValue(op2Val, isFixedPoint));
		fprintf(fileID, '\nRD: %s(%s) = ', op3Str, op3IdxStr);
		fprintf(fileID, '%s; ', printValue(op3Val, isFixedPoint));
	end
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_assign_wt(string filename_)
{
	string str = R"(
function printAssignStatementWT(fileID, outStr, outIdx, outIdxStr, outVal, isFixedPoint)

	if length(outIdx) == 1
		fprintf(fileID, '\nWT: %s(%2d) = ', outStr, outIdx);
	else
		fprintf(fileID, '\nWT: %s(%s) = ', outStr, outIdxStr);
	end

	fprintf(fileID,'%s; ', printValue(outVal, isFixedPoint));

	fprintf(fileID, '\n---------------');
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_for_statement(string filename_)
{
	string str = R"(
function printForStatement(fileID, varStr, value)

  fprintf(fileID,'\n\n------------------| For Statement %s=%d |-------------------', varStr, value);

end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_result(string filename_)
{
	string str = R"(
function printResult(fileID, outStr, outIdx, outVal, isFixedPoint)

	if length(outIdx) == 1
		fprintf(fileID, '\n%s(%2d)=', outStr, outIdx);
		fprintf(fileID,'%s', printValue(outVal, isFixedPoint));
	else
		for i = 1 : length(outIdx)
			fprintf(fileID, '\n%s(%2d)=', outStr, outIdx(i));
			fprintf(fileID,'%s', printValue(outVal(i), isFixedPoint));
		end
	end
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_storage_transfer(string filename_)
{
	string str = R"(
function printStorageTransfer(fileID, inStr, inIdx, inIdxStr, inVal, outStr, outIdx, outIdxStr, outVal)

	fprintf(fileID, '\n!!! Storage Transfer !!!');

	if length(inIdx) == 1
		fprintf(fileID, '\n{ %s(%d) = %s(%d) }', outStr, outIdx, inStr, inIdx);
	else
		fprintf(fileID, '\n{ %s(%s) = %s(%s) }', outStr, outIdxStr, inStr, inIdxStr);
	end

	fprintf(fileID, '\n---------------');

end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_value(string filename_)
{
	string str = R"(
function str = printValue(value, isFixedPoint)

	if isFixedPoint == 1
		str = sprintf('%.4f ', value);
	else
		str = sprintf('%2d ', value);
	end

end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void MatlabGenerator::gen_print_variable(string filename_)
{
	string str = R"(
function printVariable(fileID, var_, varStr_, isFixedPoint_)

	fprintf(fileID,'\n---------------------------------------------------------');
	fprintf(fileID,'\n     [i]   [%1s]\n', varStr_);
	for index = 1 : length(var_)
		fprintf(fileID,'|->%4d  %s\n', index, printValue(var_(index), isFixedPoint_));
	end
	fprintf(fileID,'*********************************************************');
end
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

// This method determines if the given identifier is a fixed-point variable.
bool isFixedPointVariable(VIR::Identifier *identifier_)
{
	return identifier_->isFixedPointVariable;
}

string dumpMatlabExpression(VIR::Expression *expression_, bool writingExtra_)
{
	string str;
	switch (expression_->kind())
	{
	case ktArray:
	{
		Array *array = (Array *)expression_;
		bool hasExtraArrayObject = array->elements().size() == 1 && array->elements()[0]->kind() == ktArray;
		if (!hasExtraArrayObject)
			str += "[";
		for (int i = 0; i < array->elements().size(); i++)
		{
			if (i > 0)
			{
				str += ", ";
			}
			str += dumpMatlabExpression(array->elements()[i], false);
		}
		if (!hasExtraArrayObject)
			str += "]";
	}
	break;

	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;
		str = "(" + dumpMatlabExpression(binaryExpression->leftOperand(), writingExtra_) + " " +
					binaryExpression->operatorStr() + " " + dumpMatlabExpression(binaryExpression->rightOperand(), writingExtra_) + ")";
	}
	break;
	case ktComplex:
		str = "(" + to_string(((Complex *)expression_)->iValue) + "," +
					to_string(((Complex *)expression_)->rValue) + ")";
		break;
	case ktFloatingPoint:
		str = to_string(((FloatingPoint *)expression_)->value);
		break;
	case ktFunctionCall:
	{
		str = ((FunctionCall *)expression_)->_name->name() + "(";
		bool first = true;
		for (auto &p : ((FunctionCall *)expression_)->_parameters)
		{
			if (!first)
			{
				str += ", ";
			}
			first = false;
			str += dumpMatlabExpression(p, false);
		}
		str += ")";
	}
	break;
	case ktIdentifier:
		str = ((Identifier *)expression_)->name() + (writingExtra_ && isFixedPointVariable((Identifier *)expression_) ? ".double" : "");
		break;

	case ktInteger:
		str = to_string(((Integer *)expression_)->value);
		break;
	case ktParenthesizedExpression:
		str = "(" + dumpMatlabExpression(((ParenthesizedExpression *)expression_)->value(), writingExtra_) + ")";
		break;
	case ktPrimitiveFunctionCall:
	{
		PrimitiveFunctionCall *primitiveFunctionCall = (PrimitiveFunctionCall *)expression_;

		str = primitiveFunctionCall->name()->name() + "(";
		for (int i = 0; i < primitiveFunctionCall->parameters().size(); i++)
		{
			if (i > 0)
			{
				str += ", ";
			}
			str += dumpMatlabExpression(primitiveFunctionCall->parameters()[i], false);
		}
		str += ")";
	}
	break;
	case ktRangeExpression:
	{
		RangeExpression *rangeExpression = (RangeExpression *)expression_;

		str = dumpMatlabExpression(rangeExpression->begin(), writingExtra_) + " : " +
					dumpMatlabExpression(rangeExpression->increment(), writingExtra_) + " : " +
					dumpMatlabExpression(rangeExpression->end(), writingExtra_);
	}
	break;
	case ktSliceName:
	{
		SliceName *sliceName = (SliceName *)expression_;
		str = dumpMatlabExpression(sliceName->prefix(), writingExtra_) + "(";
		for (int i = 0; i < sliceName->suffix().size(); i++)
		{
			if (i > 0)
			{
				str += ", ";
			}
			str += dumpMatlabExpression(sliceName->suffix()[i], false);
		}
		str += ")";
	}
	break;
	case ktStringLiteral:
	{
		str = "'" + ((StringLiteral *)expression_)->value + "'";
	}
	break;
	case ktUnaryExpression:
	{
		UnaryExpression *unaryExpression = (UnaryExpression *)expression_;
		str = "(" + unaryExpression->operatorStr() + dumpMatlabExpression(unaryExpression->operand(), writingExtra_) + ")";
	}
	break;
	}
	return str;
}

string dumpMatlabStatement(VIR::Statement *statement_, int level_)
{
	stringstream str;
	switch (statement_->kind())
	{
	case ktForStatement:
	{
		ForStatement *forStatement = (ForStatement *)statement_;
		Common::write_line(str, "for " + dumpMatlabExpression(forStatement->loopVariable(), false) + " = " + dumpMatlabExpression(forStatement->loopRange(), false), level_);
		for (auto &s : forStatement->loopBody())
		{
			Common::write_line(str, dumpMatlabStatement(s, level_ + 1));
		}
		Common::write_line(str, "end", level_);
	}
	break;

	case ktIfStatement:
	{
		IfStatement *ifStatement = (IfStatement *)statement_;
		Common::write_line(str, "if " + dumpMatlabExpression(ifStatement->condition(), false), level_);
		for (auto &s : ifStatement->thenPart())
		{
			Common::write_line(str, dumpMatlabStatement(s, level_ + 1));
		}
		Common::write_line(str, "end", level_);
	}
	break;

	case ktIfThenElseStatement:
	{
		IfThenElseStatement *ifThenElseStatement = (IfThenElseStatement *)statement_;
		Common::write_line(str, "if " + dumpMatlabExpression(ifThenElseStatement->condition(), false), level_);
		for (auto &s : ifThenElseStatement->thenPart())
		{
			Common::write_line(str, dumpMatlabStatement(s, level_ + 1));
		}
		Common::write_line(str, "else", level_);
		for (auto &s : ifThenElseStatement->elsePart())
		{
			Common::write_line(str, dumpMatlabStatement(s, level_ + 1));
		}
		Common::write_line(str, "end", level_);
	}
	break;
	case ktAssignmentStatement:
	{
		AssignmentStatement *assignStatement = (AssignmentStatement *)statement_;

		// case of preservingMac operation
		string matlabExprStr;
		if (assignStatement->lhs().size() == 0)
		{
			matlabExprStr = dumpMatlabExpression(assignStatement->lhs()[0], false) + ";";
		}
		else if (assignStatement->lhs().size() == 1)
		{
			matlabExprStr = dumpMatlabExpression(assignStatement->lhs()[0], false) + " = " + dumpMatlabExpression(assignStatement->rhs(), false) + ";";
		}
		else
		{
			for (int idx = 0; idx < assignStatement->lhs().size(); idx++)
			{
				if (idx == 0)
				{
					matlabExprStr = "[";
				}
				else
				{
					matlabExprStr += ", ";
				}
				matlabExprStr += dumpMatlabExpression(assignStatement->lhs()[idx], false);
			}
			matlabExprStr += "]";
			matlabExprStr += " = " + dumpMatlabExpression(assignStatement->rhs(), false) + ";";
		}

		// Instrumenting the code with print statements
		switch (assignStatement->type())
		{
		case atDeclarative:
		{
			Common::write_line(str, matlabExprStr, level_);
		}
		break;
		case atArithmetic:
		{
			string isFixedPointStr = (assignStatement->lhs().size() > 0 && isFixedPointVariable(static_cast<VIR::SliceName *>(assignStatement->lhs()[0])->prefix())) ? "1" : "0";
			vector<SliceName *> outputNames;
			for (auto &sn : assignStatement->lhs())
			{
				outputNames.push_back(static_cast<VIR::SliceName *>(sn));
			}
			SliceName *outputName = outputNames[0];
			switch (assignStatement->rhs()->kind())
			{
			case ktBinaryExpression:
			{
				if (((BinaryExpression *)assignStatement->rhs())->rightOperand()->kind() != ktPrimitiveFunctionCall)
				{

					BinaryExpression *binaryExprssion = (BinaryExpression *)assignStatement->rhs();
					SliceName *leftOpName = (SliceName *)binaryExprssion->leftOperand();

					// TODO: currently assignment statements with 3 operands is not supported,
					//       so only the first 2 operands are printed in the MATLAB printf statements
					SliceName *rightOpName = binaryExprssion->rightOperand()->kind() != ktBinaryExpression ? (SliceName *)binaryExprssion->rightOperand() : (SliceName *)((BinaryExpression *)binaryExprssion->rightOperand())->leftOperand();

					if (leftOpName->kind() == ktSliceName)
					{
						// Overwriting matlabExprStr in case of dummy names
						if (leftOpName->isDummy() || rightOpName->isDummy())
						{
							matlabExprStr = dumpMatlabExpression(static_cast<VIR::SliceName *>(assignStatement->lhs()[0]), false) + " = ";
							matlabExprStr += leftOpName->isDummy() ? dumpMatlabExpression(leftOpName->prefix()->object()->value(), false) : dumpMatlabExpression(leftOpName, false);
							matlabExprStr += " " + binaryExprssion->operatorStr() + " ";
							matlabExprStr += rightOpName->isDummy() && rightOpName->kind() == ktSliceName ? dumpMatlabExpression(rightOpName->prefix()->object()->value(), false) : dumpMatlabExpression(rightOpName, false);
							matlabExprStr += ";";
						}
					}
				}
			}
			break;
			case ktPrimitiveFunctionCall:
				break;
			case ktFloatingPoint:
			case ktInteger:
				// Write Nothing
				break;
			default:
				LOG(FATAL) << ("dumpMatlabStatement: Unsupported assignment");
				break;
			}
			string printResultStr = "";
			if (outputNames.size() > 0)
			{
				for (auto outputName : outputNames)
				{
					printResultStr += "printResult(fileID3, ";
					printResultStr += "'";
					printResultStr += dumpMatlabExpression(outputName->prefix(), false) + "', ";
					printResultStr += dumpMatlabExpression(outputName->suffix()[0], false) + ", ";
					printResultStr += dumpMatlabExpression(outputName, false) + ", ";
					printResultStr += isFixedPointStr + ");\n";
				}
				printResultStr += "\n";
			}
			Common::write_line(str, matlabExprStr, level_);
			Common::write_line(str, printResultStr, level_);
		}
		break;

		case atMemoryTransfer:
		case atRegisterTransfer:
		{
			SliceName *lhsSliceName = dynamic_cast<SliceName *>((assignStatement->lhs()[0]));
			SliceName *rhsSliceName = dynamic_cast<SliceName *>((assignStatement->rhs()));
			if (!lhsSliceName || !rhsSliceName)
				LOG(FATAL) << ("dumpMatlabStatement: Improper storage transfer statement!");
			Common::write_line(str, matlabExprStr, level_);
			if (assignStatement->type() == atRegisterTransfer)
			{
				string isFixedPointStr = isFixedPointVariable(static_cast<VIR::Identifier *>((assignStatement->lhs())[0])) ? "1" : "0";
				string printResultStr = "printResult(fileID3, ";
				printResultStr += "'";
				printResultStr += dumpMatlabExpression(lhsSliceName->prefix(), false) + "', ";
				printResultStr += dumpMatlabExpression(lhsSliceName->suffix()[0], false) + ", ";
				printResultStr += dumpMatlabExpression(lhsSliceName, true) + ", ";
				printResultStr += isFixedPointStr + ");";
				Common::write_line(str, printResultStr, level_);
			}
		}
		break;

		default:
		{
			Common::write_line(str, matlabExprStr, level_);
		}
		}
	}
	break;

	case ktPragmaStatement:
	{
		if (((PragmaStatement *)statement_)->pragma()->isBlockBegin())
		{
			Common::write_line(str, "%! RESOURCE_SHARING_BEGIN", level_);
		}
		else
		{
			Common::write_line(str, "%! RESOURCE_SHARING_END", level_);
		}
	}
	break;

	default:
		LOG(FATAL) << "dumpMatlabStatement: NOT SUPPORTED YET!!";
	}
	return str.str();
} // namespace filegen
void MatlabGenerator::gen_instrumented_code(VIR::MainProgram *p_, string filename_)
{
	ofstream ofs(filename_, ofstream::out);
	Common::write_line(ofs, "% Instrumented the MATLAB code with some printing commands.");
	Common::write_line(ofs, "");
	Common::write_line(ofs, "fileID3 = fopen('mt_results.txt','w');");
	Common::write_line(ofs, "");
	int level = 0;
	for (auto &s : p_->body())
	{
		ofs << dumpMatlabStatement(s, level);
	}
	Common::write_line(ofs, "");
	Common::write_line(ofs, "fclose(fileID3);");
}

} // namespace filegen
} // namespace vesyla