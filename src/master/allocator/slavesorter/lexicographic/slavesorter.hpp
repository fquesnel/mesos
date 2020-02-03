#ifndef __MASTER_ALLOCATOR_SLAVE_SORTER_LEXICOGRAPHIC_SORTER_HPP__
#define __MASTER_ALLOCATOR_SLAVE_SORTER_LEXICOGRAPHIC_SORTER_HPP__
#include <mesos/mesos.hpp>

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

#include "common/resource_quantities.hpp"

#include "master/allocator/slavesorter/slavesorter.hpp"

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

	class LexicographicSlaveSorter : public SlaveSorter{
	public:
		LexicographicSlaveSorter();
		virtual ~LexicographicSlaveSorter();
		virtual void sort(std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end);
		virtual void add(const SlaveID& slaveId, const Resources& resources);

  		// Remove resources from the total pool.
  		virtual void remove(const SlaveID& slaveId, const Resources& resources);
		// Specify that resources have been allocated on the given slave
		virtual void allocated(
			const SlaveID& slaveId,
		    const Resources& resources);


		// Specify that resources have been unallocated on the given slave.
		virtual void unallocated(
			const SlaveID& slaveId,
		    const Resources& resources);
	protected:
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

#endif //__MASTER_ALLOCATOR_SLAVE_SORTER_LEXICOGRAPHIC_SORTER_HPP__
