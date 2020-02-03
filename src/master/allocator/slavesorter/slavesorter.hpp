#ifndef __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
#define __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
#include <vector>
#include <process/pid.hpp>

using mesos::Resource;
using mesos::Resources;
using mesos::internal::ResourceQuantities;
using mesos::SlaveID;
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
      const process::UPID& allocator,
      const std::string& metricsPrefix) {}

  virtual ~SlaveSorter() = default;


//   // Returns all of the slaves in the order that they should
//   // be allocated to, according to this Sorter's policy.
  virtual void sort(std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end) = 0;

  // Add resources to the total pool of resources this
  // Sorter should consider.
  virtual void add(const SlaveID& slaveId, const Resources& resources) = 0;

  // Remove resources from the total pool.
  virtual void remove(const SlaveID& slaveId, const Resources& resources) = 0;

  // TODO: get closer to the sorter api
  // manage adding and removal of slave
  // manage slave resources updates
  // manage whitelisting ? ( needs an updateWhitelist method to update whitelisted slaves from Option<hashset<string>>& _whitelist))


// Specify that resources have been allocated on the given slave
  virtual void allocated(
      const SlaveID& slaveId,
      const Resources& resources) = 0;


  // Specify that resources have been unallocated on the given slave.
  virtual void unallocated(
      const SlaveID& slaveId,
      const Resources& resources) = 0;

};


} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {

#endif // __MASTER_ALLOCATOR_SLAVESORTER_SLAVESORTER_HPP__
