#include "master/allocator/slavesorter/resource/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {


// TODO: -oj maybe extract the resource linearization+weights in another
// template 		could be weighted, fixed, scarcity-based, etc..

// sort using a custom function object

/*
        - For each slave, maintain :
                - Resources totalResources
                - Resources reservedResources[Role]
                - Resources allocatedResources[Role]
                - Resources frameworkAllocatedResources[frameworkId]
        ----
                - scalarWeight:
                        for each  resource:
                                - resScalar * resWeight
                                - a sum of those, ie weight
                                        differs per Role !!!Computed  on demand

*/

class SlaveIdResourceCmp {
private:
  hashmap<SlaveID, Resources> &resources;

public:
  SlaveIdResourceCmp(hashmap<SlaveID, Resources> &resources)
      : resources(resources) {}
  bool operator()(SlaveID a, SlaveID b) const {
    CHECK(resources.contains(a));

    CHECK(resources.contains(b));

    return a < b;
  }
};

ResourceSlaveSorter::ResourceSlaveSorter() {}

ResourceSlaveSorter::~ResourceSlaveSorter() {}

bool ResourceSlaveSorter::_compare(SlaveID &l, SlaveID &r) {
  return allocationRatios[l] < allocationRatios[r];
}

void ResourceSlaveSorter::sort(std::vector<SlaveID>::iterator begin,
                               std::vector<SlaveID>::iterator end) {
  // std::random_shuffle(begin, end);
  std::sort(begin, end,
            [this](SlaveID l, SlaveID r) { return _compare(l, r); });
}

void ResourceSlaveSorter::add(const SlaveID &slaveId,
                              const Resources &resources) {
  // TODO: oj refine
  // totalResources[slaveId] += resources.createStrippedScalarQuantity();
  if (!resources.empty()) {
    // Add shared resources to the total quantities when the same
    // resources don't already exist in the total.
    const Resources newShared =
        resources.shared().filter([this, slaveId](const Resource &resource) {
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

void ResourceSlaveSorter::remove(const SlaveID &slaveId,
                                 const Resources &resources) {
  if (!resources.empty()) {
    CHECK(total_.resources.contains(slaveId));
    CHECK(total_.resources[slaveId].contains(resources))
        << total_.resources[slaveId] << " does not contain " << resources;

    total_.resources[slaveId] -= resources;

    // Remove shared resources from the total quantities when there
    // are no instances of same resources left in the total.
    const Resources absentShared =
        resources.shared().filter([this, slaveId](const Resource &resource) {
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

void ResourceSlaveSorter::allocated(const SlaveID &slaveId,
                                    const Resources &toAdd) {
  // Add shared resources to the allocated quantities when the same
  // resources don't already exist in the allocation.
  const Resources sharedToAdd =
      toAdd.shared().filter([this, slaveId](const Resource &resource) {
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
void ResourceSlaveSorter::unallocated(const SlaveID &slaveId,
                                      const Resources &toRemove) {
  // TODO: refine and account for shared resources
  CHECK(allocatedResources.contains(slaveId));
  CHECK(allocatedResources.at(slaveId).contains(toRemove))
      << "Resources " << allocatedResources.at(slaveId) << " at agent "
      << slaveId << " does not contain " << toRemove;

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