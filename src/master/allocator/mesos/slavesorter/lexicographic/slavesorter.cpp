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
#include "master/allocator/mesos/slavesorter/lexicographic/slavesorter.hpp"

using namespace std::placeholders;

namespace mesos {
namespace internal {
namespace master {
namespace allocator {

LexicographicSlaveSorter::LexicographicSlaveSorter() {}

LexicographicSlaveSorter::~LexicographicSlaveSorter() {}

bool LexicographicSlaveSorter::compareSlaves(SlaveID a, SlaveID b)
{
  // sort by hostname
  return slaveInfos[a].mutable_hostname()->compare(
    *slaveInfos[b].mutable_hostname());
}
void LexicographicSlaveSorter::sort(
  std::vector<SlaveID>::iterator begin, std::vector<SlaveID>::iterator end)
{
  std::sort(
    begin,
    end,
    std::bind(&LexicographicSlaveSorter::compareSlaves, this, _1, _2));
}

void LexicographicSlaveSorter::add(
  const SlaveID& slaveId,
  const SlaveInfo& slaveInfo,
  const Resources& resources)
{
  slaveInfos[slaveId] = slaveInfo;
}


void LexicographicSlaveSorter::remove(
  const SlaveID& slaveId, const Resources& resources)
{
  slaveInfos.erase(slaveId);
}


} // namespace allocator {
} // namespace master {
} // namespace internal {
} // namespace mesos {
