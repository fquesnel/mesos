#include "master/allocator/slavesorter/random/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

	RandomSlaveSorter::RandomSlaveSorter(){

	}

	RandomSlaveSorter::~RandomSlaveSorter(){

	}

	void RandomSlaveSorter::sort(std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end){
		std::random_shuffle(begin, end);
	}

	void RandomSlaveSorter::add(const SlaveID& slaveId, const Resources& resources){
	 	// Not used by this sorter
	}


	void RandomSlaveSorter::remove(const SlaveID& slaveId, const Resources& resources){
	 	// Not used by this sorter
	}

	 void RandomSlaveSorter::allocated(
	      const SlaveID& slaveId,
	      const Resources& resources){
	 	// Not used by this sorter
	 }


  // Specify that resources have been unallocated on the given slave.
  	void RandomSlaveSorter::unallocated(
      const SlaveID& slaveId,
      const Resources& resources){
	 	// Not used by this sorter
  	}



} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {