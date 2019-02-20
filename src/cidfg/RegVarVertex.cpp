#include "RegVarVertex.hpp"

namespace vesyla
{
namespace cidfg
{

vector<int> RegVarVertex::convert_to_int_array(VIR::Expression *e_)
{
	if (e_->kind() == VIR::VirEnumerations::ktInteger)
	{
		VIR::Integer *expr = static_cast<VIR::Integer *>(e_);
		vector<int> v;
		v.push_back(expr->value);
		return v;
	}
	else if (e_->kind() == VIR::VirEnumerations::ktRangeExpression)
	{
		VIR::RangeExpression *expr = static_cast<VIR::RangeExpression *>(e_);
		vector<int> v;
		CHECK_EQ(expr->begin()->kind(), VIR::VirEnumerations::ktInteger);
		CHECK_EQ(expr->end()->kind(), VIR::VirEnumerations::ktInteger);
		CHECK_EQ(expr->increment()->kind(), VIR::VirEnumerations::ktInteger);

		int begin = (static_cast<VIR::Integer *>(expr->begin()))->value;
		int end = (static_cast<VIR::Integer *>(expr->end()))->value;
		int increment = (static_cast<VIR::Integer *>(expr->increment()))->value;

		for (int i = begin; i < end; i += increment)
		{
			v.push_back(i);
		}
		return v;
	}
	else if (e_->kind() == VIR::VirEnumerations::ktArray)
	{
		VIR::Array *expr = static_cast<VIR::Array *>(e_);
		vector<int> v;
		for (int i = 0; i < expr->elements().size(); i++)
		{
			vector<int> v1 = convert_to_int_array(expr->elements()[i]);
			v.insert(v.end(), v1.begin(), v1.end());
		}
		return v;
	}
	else if (e_->kind() == VIR::VirEnumerations::ktUnaryExpression && static_cast<VIR::UnaryExpression *>(e_)->operatorType() == VIR::VirEnumerations::uotMinus)
	{
		VIR::UnaryExpression *expr = static_cast<VIR::UnaryExpression *>(e_);
		if (expr->operatorType() == VIR::VirEnumerations::uotMinus)
		{
			vector<int> v = convert_to_int_array(expr->operand());
			for (auto &e : v)
			{
				e = -e;
			}
			return v;
		}
		else
		{
			LOG(FATAL) << "Unsupported unary expression initialization style:" << expr->operatorTypeStr();
		}
	}
	else
	{
		LOG(FATAL) << "Unsupported initialization style:" << e_->kindStr();
	}
}

vector<float> RegVarVertex::convert_to_float_array(VIR::Expression *e_)
{
	if (e_->kind() == VIR::VirEnumerations::ktFloatingPoint)
	{
		VIR::FloatingPoint *expr = static_cast<VIR::FloatingPoint *>(e_);
		vector<float> v;
		v.push_back(expr->value);
		return v;
	}
	else if (e_->kind() == VIR::VirEnumerations::ktInteger)
	{
		VIR::Integer *expr = static_cast<VIR::Integer *>(e_);
		vector<float> v;
		v.push_back(expr->value);
		return v;
	}
	else if (e_->kind() == VIR::VirEnumerations::ktArray)
	{
		VIR::Array *expr = static_cast<VIR::Array *>(e_);
		vector<float> v;
		for (int i = 0; i < expr->elements().size(); i++)
		{
			vector<float> v1 = convert_to_float_array(expr->elements()[i]);
			v.insert(v.end(), v1.begin(), v1.end());
		}
		return v;
	}
	else
	{
		LOG(FATAL) << "Unsupported initialization style";
	}
}

RegVarVertex::RegVarVertex()
{
	vertex_type = REG_VAR_VERTEX;
}
RegVarVertex::~RegVarVertex()
{
}

RegVarVertex::RegVarVertex(VIR::AssignmentStatement *stmt_)
{
	vertex_type = REG_VAR_VERTEX;
	CHECK_EQ(stmt_->type(), VIR::VirEnumerations::atDeclarative);
	CHECK_EQ((stmt_->lhsIdentifiers()).size(), 1);

	value_type = UNKNOWN;

	var_name = stmt_->lhsIdentifiers()[0]->name();
	coord = (dynamic_cast<VIR::StoragePragma *>(stmt_->pragma())->coordinates)[0];
	if (stmt_->rhs()->kind() == VIR::VirEnumerations::ktArray)
	{
		VIR::Array *arr = (static_cast<VIR::Array *>(static_cast<VIR::Array *>(stmt_->rhs())->elements()[0]));
		value_type = INT;
		int_data_array = convert_to_int_array(arr);
	}
	else if (stmt_->rhs()->kind() == VIR::VirEnumerations::ktPrimitiveFunctionCall)
	{
		VIR::PrimitiveFunctionCall *func = static_cast<VIR::PrimitiveFunctionCall *>(stmt_->rhs());
		if (func->name()->name() == "zeros")
		{
			value_type = INT;
			int num = (static_cast<VIR::Integer *>(func->parameters()[1]))->value;
			int_data_array.resize(num, 0);
		}
		else if (func->name()->name() == "ones")
		{
			value_type = INT;
			int num = (static_cast<VIR::Integer *>(func->parameters()[1]))->value;
			int_data_array.resize(num, 1);
		}
		else if (func->name()->name() == "fi")
		{
			VIR::Array *arr = static_cast<VIR::Array *>(func->parameters()[0]);
			value_type = FLOAT;
			float_data_array = convert_to_float_array(arr);
		}
		else
		{
			LOG(FATAL) << "Unsupported primitive function call for initialization";
		}
	}
}
string RegVarVertex::dump()
{
	string str;
	str += "REG [";
	str += var_name;
	str += "] ~~ [";
	if (value_type == INT)
	{
		for (auto &e : int_data_array)
		{
			str += to_string(e) + ", ";
		}
	}
	if (value_type == FLOAT)
	{
		for (auto &e : float_data_array)
		{
			str += to_string(e) + ", ";
		}
	}
	str += "]";

	return str;
}
} // namespace cidfg
} // namespace vesyla