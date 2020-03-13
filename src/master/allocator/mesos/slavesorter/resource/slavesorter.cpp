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
#include "master/allocator/mesos/slavesorter/resource/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

class SlaveIdResourceCmp
{
private:
  hashmap<SlaveID, Resources>& resources;

public:
  SlaveIdResourceCmp(hashmap<SlaveID, Resources>& resources)
    : resources(resources)
  {}
  bool operator()(SlaveID a, SlaveID b) const
  {
    CHECK(resources.contains(a));

    CHECK(resources.contains(b));

    return a < b;
  }
};

ResourceSlaveSorter::ResourceSlaveSorter() {}

ResourceSlaveSorter::~ResourceSlaveSorter() {}

bool ResourceSlaveSorter::_compare(SlaveID& l, SlaveID& r)
{
  return allocationRatios[l] < allocationRatios[r];
}

void ResourceSlaveSorter::sort(
  std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end)
{
  std::sort(
    begin, end, [this](SlaveID l, SlaveID r) { return _compare(l, r); });
}

void ResourceSlaveSorter::add(
  const SlaveID& slaveId,
  const SlaveInfo& slaveInfo,
  const Resources& resources)
{
  // TODO(jabnouneo): refine
  // totalResources[slaveId] += resources.createStrippedScalarQuantity();
  if (!resources.empty()) {
    // Add shared resources to the total quantities when the same
    // resources don't already exist in the total.
    const Resources newShared =
      resources.shared().filter([this, slaveId](const Resource& resource) {
        return !total_.resources[slaveId].contains(resource);
      });

    total_.resources[slaveId] += resources;

    const Resources scalarQuantities =
      (resources.nonShared() + newShared).createStrippedScalarQuantity();

    total_.scalarQuantities += scalarQuantities;
    idleWeights[slaveId] =
      computeUnitaryResourcesProportions(total_.resources[slaveId]);
    totalWeights[slaveId] =
      computeResourcesWeight(slaveId, total_.resources[slaveId]);
  }
}

void ResourceSlaveSorter::remove(
  const SlaveID& slaveId, const Resources& resources)
{
  if (!resources.empty()) {
    CHECK(total_.resources.contains(slaveId));
    CHECK(total_.resources[slaveId].contains(resources))
      << total_.resources[slaveId] << " does not contain " << resources;

    total_.resources[slaveId] -= resources;

    // Remove shared resources from the total quantities when there
    // are no instances of same resources left in the total.
    const Resources absentShared =
      resources.shared().filter([this, slaveId](const Resource& resource) {
        return !total_.resources[slaveId].contains(resource);
      });

    const Resources scalarQuantities =
      (resources.nonShared() + absentShared).createStrippedScalarQuantity();

    CHECK(total_.scalarQuantities.contains(scalarQuantities));
    total_.scalarQuantities -= scalarQuantities;

    idleWeights[slaveId] =
      computeUnitaryResourcesProportions(total_.resources[slaveId]);
    totalWeights[slaveId] =
      computeResourcesWeight(slaveId, total_.resources[slaveId]);
    if (total_.resources[slaveId].empty()) {
      total_.resources.erase(slaveId);
    }
  }
}

void ResourceSlaveSorter::allocated(
  const SlaveID& slaveId, const Resources& toAdd)
{
  // Add shared resources to the allocated quantities when the same
  // resources don't already exist in the allocation.
  const Resources sharedToAdd =
    toAdd.shared().filter([this, slaveId](const Resource& resource) {
      return !total_.resources[slaveId].contains(resource);
    });

  const Resources quantitiesToAdd =
    (toAdd.nonShared() + sharedToAdd).createStrippedScalarQuantity();
  total_.resources[slaveId] += quantitiesToAdd;
  allocatedResources[slaveId] += toAdd;
  allocationWeights[slaveId] =
    computeResourcesWeight(slaveId, allocatedResources[slaveId]);
  allocationRatios[slaveId] =
    allocationWeights[slaveId] / totalWeights[slaveId];
  total_.scalarQuantities += quantitiesToAdd;
}

// Specify that resources have been unallocated on the given slave.
void ResourceSlaveSorter::unallocated(
  const SlaveID& slaveId, const Resources& toRemove)
{
  // TODO(jabnouneo): refine and account for shared resources
  CHECK(allocatedResources.contains(slaveId));
  CHECK(allocatedResources.at(slaveId).contains(toRemove))
    << "Resources " << allocatedResources.at(slaveId) << " at agent " << slaveId
    << " does not contain " << toRemove;

  allocatedResources[slaveId] -= toRemove;


  if (allocatedResources[slaveId].empty()) {
    allocatedResources.erase(slaveId);
    allocationWeights.erase(slaveId);
  } else {
    allocationWeights[slaveId] =
      computeResourcesWeight(slaveId, allocatedResources[slaveId]);
  }
}

} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {
