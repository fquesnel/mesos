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
#ifndef __MASTER_ALLOCATOR_SLAVE_SORTER_CPU_FIRST_HPP__
#define __MASTER_ALLOCATOR_SLAVE_SORTER_CPU_FIRST_HPP__

#include <algorithm>
#include <algorithm> // for min and max
#include <set>
#include <string>
#include <vector>

#include <mesos/mesos.hpp>
#include <mesos/resources.hpp>
#include <mesos/values.hpp>
#include <stout/bytes.hpp>
#include <stout/check.hpp>
#include <stout/hashmap.hpp>
#include <stout/option.hpp>

#include "mesos/resource_quantities.hpp"
#include "master/allocator/mesos/slavesorter/slavesorter.hpp"


namespace mesos {
namespace internal {
namespace master {
namespace allocator {


class ResourceSlaveSorterCPUFirst : public SlaveSorter
{
public:
  ResourceSlaveSorterCPUFirst();
  virtual ~ResourceSlaveSorterCPUFirst();
  virtual void sort(
    std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end);
  virtual void add(
    const SlaveID& slaveId,
    const SlaveInfo& slaveInfo,
    const Resources& resources);
  // Remove resources from the total pool.
  virtual void remove(const SlaveID& slaveId, const Resources& resources);
  // Specify that resources have been allocated on the given slave
  virtual void allocated(const SlaveID& slaveId, const Resources& resources);


  // Specify that resources have been unallocated on the given slave.
  virtual void unallocated(const SlaveID& slaveId, const Resources& resources);

private:
  bool _compare(SlaveID& l, SlaveID& r);
  // TODO(jabnouneo) : merge in single class + optimize
  hashmap<SlaveID, Resources> allocatedResources;
  hashmap<SlaveID, Resources> totalResources;

  //
  // Total resources.
  struct Total
  {
    // We need to keep track of the resources (and not just scalar
    // quantities) to account for multiple copies of the same shared
    // resources. We need to ensure that we do not update the scalar
    // quantities for shared resources when the change is only in the
    // number of copies in the sorter.
    hashmap<SlaveID, Resources> resources;

    // NOTE: Scalars can be safely aggregated across slaves. We keep
    // that to speed up the calculation of shares. See MESOS-2891 for
    // the reasons why we want to do that.
    //
    // NOTE: We omit information about dynamic reservations and
    // persistent volumes here to enable resources to be aggregated
    // across slaves more effectively. See MESOS-4833 for more
    // information.
    //
    // Sharedness info is also stripped out when resource identities
    // are omitted because sharedness inherently refers to the
    // identities of resources and not quantities.
    Resources scalarQuantities;
  } total_;
};
} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {

#endif // __MASTER_ALLOCATOR_SLAVE_SORTER_CPU_FIRST_HPP__
