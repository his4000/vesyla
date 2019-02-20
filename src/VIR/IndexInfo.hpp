#include <exception>
#include <string>
#include <map>
using namespace std;

#ifndef __IndexInfo_h__
#define __IndexInfo_h__

#include "AddressInfo.hpp"
#include "UnrolledAddressInfo.hpp"
#include "ForStatement.hpp"
#include "Unrolling.hpp"
#include "Pragma.hpp"

namespace VIR
{
class AddressInfo;
class UnrolledAddressInfo;
class ForStatement;
class Unrolling;
// class Pragma;
class IndexInfo;
} // namespace VIR

namespace VIR
{
class IndexInfo : public VIR::Pragma
{
	typedef struct
	{
		bool operator()(const VIR::ForStatement *a,
										const VIR::ForStatement *b) const;
	} CompareMapElements;

public:
	VIR::AddressInfo addressInfo;

public:
	bool isARaccuVariable;

public:
	bool isARangeExpression;

public:
	map<string, VIR::UnrolledAddressInfo> unrolledAddressMap;

private:
	map<VIR::ForStatement *, VIR::Unrolling *, CompareMapElements> _unrolling;

public:
	~IndexInfo();

public:
	void addUnrolling(VIR::ForStatement *forStatement_,
										int numberOfRepetitions_, int unrollingOffset_, bool isDynamic_);

public:
	VIR::Unrolling *unrolling(VIR::ForStatement *forStatement_) const;

public:
	map<VIR::ForStatement *, VIR::Unrolling *, VIR::IndexInfo::CompareMapElements> &unrollingMap();
};
} // namespace VIR

#endif
