// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef __MASTER_ALLOCATOR_SLAVE_SORTER_RANDOM_SORTER_HPP__
#define __MASTER_ALLOCATOR_SLAVE_SORTER_RANDOM_SORTER_HPP__
#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <mesos/mesos.hpp>
#include <mesos/resources.hpp>
#include <mesos/values.hpp>
#include <stout/check.hpp>
#include <stout/hashmap.hpp>
#include <stout/option.hpp>

#include "mesos/resource_quantities.hpp"
#include "master/allocator/mesos/slavesorter/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

class RandomSlaveSorter : public SlaveSorter
{
public:
  RandomSlaveSorter() {}
  virtual ~RandomSlaveSorter() {}
  virtual void sort(
    std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end);
  virtual void add(
    const SlaveID& slaveId,
    const SlaveInfo& slaveInfo,
    const Resources& resources)
  {}

  // The following methods are not used by this sorter
  virtual void remove(const SlaveID& slaveId, const Resources& resources){};
  virtual void allocated(const SlaveID& slaveId, const Resources& resources) {}
  virtual void unallocated(const SlaveID& slaveId, const Resources& resources)
  {}
};
} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {
#endif // __MASTER_ALLOCATOR_SLAVE_SORTER_RANDOM_SORTER_HPP__
