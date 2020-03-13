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
#ifndef __MASTER_ALLOCATOR_SLAVE_SORTER_RESOURCE_SORTER_HPP__
#define __MASTER_ALLOCATOR_SLAVE_SORTER_RESOURCE_SORTER_HPP__

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


typedef struct UnitaryResourceWeight
{
  double cpuWeigth;
  double gpuWeigth;
  double memWeigth;
  double diskWeigth;
} UnitaryResourceWeight;

class ResourceSlaveSorter : public SlaveSorter
{
public:
  ResourceSlaveSorter();
  virtual ~ResourceSlaveSorter();
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
  hashmap<SlaveID, double> allocationRatios;

  hashmap<SlaveID, double> totalWeights;
  hashmap<SlaveID, double> allocationWeights;
  hashmap<SlaveID, UnitaryResourceWeight> idleWeights;

  double computeResourcesWeight(
    const SlaveID& slaveId, const Resources& resources)
  {
    // TODO(jabnouneo): add resources filtering and take into account role/quota
    double result = 0;
    const UnitaryResourceWeight& resourcesWeights = idleWeights[slaveId];
    const Option<double> cpus = resources.cpus();
    const Option<double> gpus = resources.gpus();
    const Option<Bytes> mem = resources.mem();
    const Option<Bytes> disk = resources.disk();

    if (cpus.isSome()) result += resourcesWeights.cpuWeigth * cpus.get();

    if (gpus.isSome()) result += resourcesWeights.gpuWeigth * gpus.get();

    if (disk.isSome()) {
      result +=
        (resourcesWeights.diskWeigth *
         ((double)disk.get().bytes() / (double)Bytes::MEGABYTES));
    }
    if (mem.isSome())
      result +=
        (resourcesWeights.memWeigth *
         ((double)mem.get().bytes() / (double)Bytes::MEGABYTES));

    return result;
  }

  UnitaryResourceWeight computeUnitaryResourcesProportions(
    const Resources& resources)
  {
    // compute proportion; starting with cpus, then gpu, mem and disk
    // todo: bandwidth?ports?
    const Option<double> cpus = resources.cpus();
    const Option<double> gpus = resources.gpus();
    const Option<Bytes> mem = resources.mem();
    const Option<Bytes> disk = resources.disk();
    /*
     */
    double max = 0;
    if (cpus.isSome()) {
      max = std::max(max, cpus.get());
    }

    if (gpus.isSome()) {
      max = std::max(max, gpus.get());
    }

    if (mem.isSome()) {
      max = std::max(max, (double)mem.get().bytes() / (double)Bytes::MEGABYTES);
    }

    if (disk.isSome()) {
      max =
        std::max(max, (double)disk.get().bytes() / (double)Bytes::MEGABYTES);
    }

    UnitaryResourceWeight result;
    Resource cpuWeight, gpuWeight, diskWeight, memWeight;
    cpuWeight.set_name("cpus");
    cpuWeight.set_type(Value::SCALAR);
    gpuWeight.set_name("gpus");
    gpuWeight.set_type(Value::SCALAR);
    diskWeight.set_name("disk");
    diskWeight.set_type(Value::SCALAR);
    memWeight.set_name("mem");
    memWeight.set_type(Value::SCALAR);


    if (cpus.isSome())
      result.cpuWeigth = max / cpus.get();
    else
      result.cpuWeigth = 0.0f;

    if (gpus.isSome())
      result.gpuWeigth = max / gpus.get();
    else
      result.gpuWeigth = 0.0f;

    if (mem.isSome())
      result.memWeigth =
        max / ((double)mem.get().bytes() / (double)Bytes::MEGABYTES);
    else
      result.memWeigth = 0.0f;

    if (disk.isSome())
      result.diskWeigth =
        max / ((double)disk.get().bytes() / (double)Bytes::MEGABYTES);
    else
      result.diskWeigth = 0.0f;
    LOG(INFO) << "Computed slave resource weigths : "
              << "[cpus=" << result.cpuWeigth << ","
              << "gpus=" << result.gpuWeigth << ","
              << "mem=" << result.memWeigth << ","
              << "disk=" << result.diskWeigth << " from idle resources "
              << resources << std::endl;
    return result;
  }

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

#endif // __MASTER_ALLOCATOR_SLAVE_SORTER_RESOURCE_SORTER_HPP__
