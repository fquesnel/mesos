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

#include <mesos/allocator/allocator.hpp>

#include <mesos/module/allocator.hpp>

#include "master/constants.hpp"

#include "master/allocator/mesos/hierarchical.hpp"

#include "module/manager.hpp"

using std::string;

using mesos::internal::master::allocator::HierarchicalDRFAllocator;
using mesos::internal::master::allocator::HierarchicalRandomAllocator;



using mesos::internal::master::allocator::HierarchicalDRFRandomSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalDRFResourceSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalDRFLexicographicSortedSlavesAllocator;

using mesos::internal::master::allocator::HierarchicalRandomResourceSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalRandomLexicographicSortedSlavesAllocator;
using mesos::internal::master::allocator::HierarchicalRandomRandomSortedSlavesAllocator;

namespace mesos {
namespace allocator {

const std::string defaultSlaveSorter = "random";

Try<Allocator*> Allocator::create(
    const string& name,
    const string& roleSorter,
    const string& frameworkSorter,
    const string& slaveSorter)
{
  // Create an instance of the default allocator. If other than the
  // default allocator is requested, search for it in loaded modules.
  //
  // NOTE: We do not need an extra not-null check, because both
  // ModuleManager and built-in allocator factory do that already.
  //
  // We also look for "HierarchicalDRF" since that was the
  // previous value for `DEFAULT_ALLOCATOR`.
  if (name == "HierarchicalDRF" || name == mesos::internal::master::DEFAULT_ALLOCATOR) {
    if (roleSorter != frameworkSorter) {
      return Error("Unsupported combination of 'role_sorter' and 'framework_sorter': must be equal (for now)");
    }
    if (roleSorter == "drf") {
      if (slaveSorter == "random")
        return HierarchicalDRFRandomSortedSlavesAllocator::create();
      if (slaveSorter == "resource")
        return HierarchicalDRFResourceSortedSlavesAllocator::create();
      if (slaveSorter == "lexicographic")
        return HierarchicalDRFLexicographicSortedSlavesAllocator::create();
       return Error("Unsupported combination of 'role_sorter' and 'slave_Sorter'.");
    }
    if (roleSorter == "random") {
      if (slaveSorter == "random")
        return HierarchicalRandomRandomSortedSlavesAllocator::create();
      if (slaveSorter == "resource")
        return HierarchicalRandomResourceSortedSlavesAllocator::create();
      if (slaveSorter == "lexicographic")
        return HierarchicalRandomLexicographicSortedSlavesAllocator::create();
       return Error("Unsupported combination of 'role_sorter' and 'slave_Sorter'.");
    }
    return Error("Unsupported 'role_sorter'.");
  }

  return modules::ModuleManager::create<Allocator>(name);
}


Try<Allocator*> Allocator::create(
    const string& name,
    const string& roleSorter,
    const string& frameworkSorter)
{

  return Allocator::create(name, roleSorter, frameworkSorter, defaultSlaveSorter);
}
} // namespace allocator {
} // namespace mesos {
