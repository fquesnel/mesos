#include "master/allocator/slavesorter/lexicographic/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

   // sort using a custom function object
    typedef struct {
        bool operator()(SlaveID a, SlaveID b) const {
            return a < b;
        }
    } SlaveIdCmp;

	LexicographicSlaveSorter::LexicographicSlaveSorter(){

	}

	LexicographicSlaveSorter::~LexicographicSlaveSorter(){

	}

	void LexicographicSlaveSorter::sort(std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end){
		printf("LexicographicSlaveSorter::sort called\n");
		std::sort(begin, end, SlaveIdCmp());
	}

	void LexicographicSlaveSorter::add(const SlaveID& slaveId, const Resources& resources)
	{
	  if (!resources.empty()) {
	    // Add shared resources to the total quantities when the same
	    // resources don't already exist in the total.
	    const Resources newShared = resources.shared()
	      .filter([this, slaveId](const Resource& resource) {
	        return !total_.resources[slaveId].contains(resource);
	      });

	    total_.resources[slaveId] += resources;

	    const Resources scalarQuantities =
	      (resources.nonShared() + newShared).createStrippedScalarQuantity();

	    total_.scalarQuantities += scalarQuantities;

	  }
	}


	void LexicographicSlaveSorter::remove(const SlaveID& slaveId, const Resources& resources)
	{
	  if (!resources.empty()) {
	    CHECK(total_.resources.contains(slaveId));
	    CHECK(total_.resources[slaveId].contains(resources))
	      << total_.resources[slaveId] << " does not contain " << resources;

	    total_.resources[slaveId] -= resources;

	    // Remove shared resources from the total quantities when there
	    // are no instances of same resources left in the total.
	    const Resources absentShared = resources.shared()
	      .filter([this, slaveId](const Resource& resource) {
	        return !total_.resources[slaveId].contains(resource);
	      });

	    const Resources scalarQuantities =
	      (resources.nonShared() + absentShared).createStrippedScalarQuantity();

	    CHECK(total_.scalarQuantities.contains(scalarQuantities));
	    total_.scalarQuantities -= scalarQuantities;

	    if (total_.resources[slaveId].empty()) {
	      total_.resources.erase(slaveId);
	    }
	  }
	}

	 void LexicographicSlaveSorter::allocated(
	      const SlaveID& slaveId,
	      const Resources& resources){
	 	// Not used by this sorter
	 }


  // Specify that resources have been unallocated on the given slave.
  	void LexicographicSlaveSorter::unallocated(
      const SlaveID& slaveId,
      const Resources& resources){
	 	// Not used by this sorter
  	}


} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {