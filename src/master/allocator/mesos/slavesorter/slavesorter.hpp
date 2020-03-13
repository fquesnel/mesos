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
#ifndef __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
#define __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
#include <vector>

#include <process/pid.hpp>

using mesos::Resource;
using mesos::Resources;
using mesos::SlaveID;
using mesos::SlaveInfo;
using mesos::ResourceQuantities;

using mesos::Value;
namespace mesos {
namespace internal {
namespace master {
namespace allocator {

class SlaveSorter
{
public:
  SlaveSorter() = default;

  // Provides the allocator's execution context (via a UPID)
  // and a name prefix in order to support metrics within the
  // sorter implementation.
  explicit SlaveSorter(
    const process::UPID& allocator, const std::string& metricsPrefix)
  {}

  virtual ~SlaveSorter() = default;

  // Initialize the sorter.
  virtual void initialize(
      const Option<std::string>& slaveSorterResourceWeights){};


  //   // Returns all of the slaves in the order that they should
  //   // be allocated to, according to this Sorter's policy.
  virtual void sort(
    std::vector<SlaveID>::iterator begin,
    std::vector<SlaveID>::iterator end) = 0;

  // Add resources to the total pool of resources this
  // Sorter should consider.
  virtual void add(
    const SlaveID& slaveId,
    const SlaveInfo& slaveInfo,
    const Resources& resources) = 0;

  // Remove resources from the total pool.
  virtual void remove(const SlaveID& slaveId, const Resources& resources) = 0;

  // TODO(jabnouneo): get closer to the sorter api
  // manage adding and removal of slave
  // manage slave resources updates
  // manage whitelisting ? ( needs an updateWhitelist method to update
  // whitelisted slaves from Option<hashset<string>>& _whitelist))


  // Specify that resources have been allocated on the given slave
  virtual void allocated(
    const SlaveID& slaveId, const Resources& resources) = 0;


  // Specify that resources have been unallocated on the given slave.
  virtual void unallocated(
    const SlaveID& slaveId, const Resources& resources) = 0;
};


} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {

#endif // __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
