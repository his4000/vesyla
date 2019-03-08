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
#include <vector>
using namespace std;

#ifndef __StoragePragma_h__
#define __StoragePragma_h__

#include "ReferencePragma.hpp"
#include "StorageDistribution.hpp"
#include "StorageType.hpp"
#include "VariableType.hpp"

namespace VIR
{
// class ReferencePragma;
class StoragePragma;
namespace VirEnumerations
{
// enum StorageDistribution;
// enum StorageType;
// enum VariableType;
}
} // namespace VIR

namespace VIR
{
class StoragePragma : public VIR::ReferencePragma
{
public:
	vector<int> maxIndexes;

public:
	vector<int> startAddresses;

private:
	VIR::VirEnumerations::StorageDistribution _distribution;

private:
	VIR::VirEnumerations::StorageType _storageType;

private:
	bool _isForIndirectAddress;

private:
	VIR::VirEnumerations::VariableType _variableType;

public:
	bool en_compression;

public:
	int getPositionIndex(int index_) const;

public:
	VIR::VirEnumerations::StorageDistribution distribution() const;

public:
	void distribution(VIR::VirEnumerations::StorageDistribution distribution_);

public:
	VIR::VirEnumerations::StorageType storageType() const;

public:
	void storageType(VIR::VirEnumerations::StorageType storageType_);

public:
	bool isForIndirectAddress() const;

public:
	void isForIndirectAddress(bool isForIndirectAddress_);

public:
	VIR::VirEnumerations::VariableType variableType() const;

public:
	void variableType(VIR::VirEnumerations::VariableType variableType_);
};
} // namespace VIR

#endif
