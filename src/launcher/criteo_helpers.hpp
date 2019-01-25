// Licensed to the Apache Software Foundation (ASF) under one
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

#pragma once

#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <vector>

#include <mesos/mesos.hpp>

#include <stout/option.hpp>
#include <stout/os.hpp>

#include "common/http.hpp"

#include "logging/logging.hpp"

namespace criteo {
namespace consul {

using process::Future;

using process::http::Connection;
using process::http::Request;
using process::http::Response;
using process::http::URL;

using std::string;
using std::vector;

using mesos::ContentType;
using mesos::Port;
using mesos::TaskInfo;

const static char* CONSUL_PROTOCOL = "http";
const static char* CONSUL_HOST = "localhost";
const static int CONSUL_PORT = 8500;

const static char* CONSUL_PORT_TEMPLATE = R"(
{
  "Name": "{{service}}",
  "Tags": [],
  "Port": {{port}},
  "Check": {
    "Name": "Check Port Open",
    "Notes": "Check that the declared port is effectively open",
    "DeregisterCriticalServiceAfter": "90m",
    "TCP": "localhost:{{port}}",
    "Interval": "60s"
  }
}
)";

const static char* CONSUL_SERVICE_TEMPLATE = R"(
{
  "Name": "{{service}}",
  "Tags": [],
  "Port": {{port}},
  "Check": {
    "Name": "Check Service availability",
    "Notes": "Check Service availability",
    "DeregisterCriticalServiceAfter": "120m",
    "Args": ["/usr/bin/sh", "-c", "{{command}}"],
    "Interval": "60s"
  }
}
)";


void replaceAll(string& str, const string& from, const string& to)
{
  if (from.empty()) return;

  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

string getConsulServiceName(const TaskInfo& taskInfo, const Port& port)
{
  return taskInfo.command().user() + "-"
        + taskInfo.discovery().name() + "-"
        + port.name();
}

string getConsulServiceName(const TaskInfo& taskInfo)
{
  return taskInfo.command().user() + "-" + taskInfo.discovery().name();
}

Option<string> getConsulToken(const string& /*username*/)
{
  //    string userConsulTokenPath = string(getpwuid(getuid())->pw_dir) +
  //    "/consul_token"; LOG(INFO) << "Reading consul token from " <<
  //    userConsulTokenPath;
  //
  //    std::stringstream userConsulToken;
  //    std::ifstream consulTokenFile(userConsulTokenPath);
  //    userConsulToken << consulTokenFile.rdbuf();
  //
  //    return userConsulToken.str().empty()
  //      ? Option<std::string>::none()
  //      : Option<std::string>::some(userConsulToken.str());

  // Framework is responsible for passing the token as ENV variable as the
  // current way of reading from a file in the user home require root access
  // and the team is not willing to make it readable for its user
  const static Option<string> consul_token = os::getenv("CONSUL_TOKEN");

  // Unset ENV for security purpose
  if (os::getenv("CONSUL_TOKEN").isSome()) {
    os::unsetenv("CONSUL_TOKEN");
  }

  return consul_token;
}

Option<string> registerTask(const TaskInfo& task)
{
  Option<string> consulToken = getConsulToken(task.command().user());
  if (consulToken.isNone()) {
    LOG(ERROR) << "Cannot register task in consul as the token for "
               << task.command().user() << " is not accessible";

    return Option<string>::some(
      "Cannot register task in consul as the token for user is not accessible");
  }

  // Register declared port into consul with a tcp check by default
  for (int ix = 0; ix < task.discovery().ports().ports_size(); ix++) {
    const Port& port = task.discovery().ports().ports(ix);
    const string consulServiceName = getConsulServiceName(task, port);

    LOG(INFO) << "Register task " << task.name() << " under service "
              << consulServiceName << " to consul";

    Request request;
    request.method = "PUT";
    request.url = URL(
      CONSUL_PROTOCOL,
      CONSUL_HOST,
      CONSUL_PORT,
      "/v1/agent/service/register",
      {{"token", consulToken.get()}});
    request.headers = {{"Accept", stringify(ContentType::JSON)},
                       {"Content-Type", stringify(ContentType::JSON)}};

    request.body = string(CONSUL_PORT_TEMPLATE);
    replaceAll(request.body, "{{port}}", stringify(port.number()));
    replaceAll(request.body, "{{service}}", consulServiceName);

    Response response = process::http::request(request).get();
    LOG(INFO) << "Registration result " << response.status << ": "
              << response.body;
  }

  // For now we only support CMD healthcheck
  // as it is the only one supported by dc/os commons
  if(task.has_health_check() && task.health_check().has_command()) {
    const string consulServiceName = getConsulServiceName(task);
    LOG(INFO) << "Register task " << task.name() << " under service "
              << consulServiceName << " to consul";

    Request request;
    request.method = "PUT";
    request.url = URL(
            CONSUL_PROTOCOL,
            CONSUL_HOST,
            CONSUL_PORT,
            "/v1/agent/service/register",
            {{"token", consulToken.get()}});
    request.headers = {{"Accept", stringify(ContentType::JSON)},
                       {"Content-Type", stringify(ContentType::JSON)}};

    request.body = string(CONSUL_SERVICE_TEMPLATE);
    replaceAll(request.body, "{{command}}",
            task.health_check().command().value());
    replaceAll(request.body, "{{port}}", "0");
    replaceAll(request.body, "{{service}}", consulServiceName);

    Response response = process::http::request(request).get();
    LOG(INFO) << "Registration result " << response.status << ": "
              << response.body;
  }

  return Option<string>::none();
}

Option<string> deregisterTask(const TaskInfo& task)
{
  Option<string> consulToken = getConsulToken(task.command().user());
  if (consulToken.isNone()) {
    LOG(ERROR) << "Cannot deregister task in consul as the token for "
               << task.command().user() << " is not accessible";

    return Option<string>::some(
      "Cannot deregister task into Consul. Reason: Cannot retrieve the token "
      "of the user");
  }

  vector<string> services =
          vector<string>(task.discovery().ports().ports_size());
  services.push_back(getConsulServiceName(task));
  for (int ix = 0; ix < task.discovery().ports().ports_size(); ix++) {
      services.push_back(stringify(
          getConsulServiceName(task, task.discovery().ports().ports(ix))));
  }

  for (const auto& service : services) {
    LOG(INFO) << "Deregister task " << task.name() << " under service "
              << service << " from consul";

    Request request;
    request.method = "PUT";
    request.url = URL(
      CONSUL_PROTOCOL,
      CONSUL_HOST,
      CONSUL_PORT,
      "/v1/agent/service/deregister/" + service,
      {{"token", consulToken.get()}});
    request.body = "";
    request.headers = {{"Accept", stringify(ContentType::JSON)},
                       {"Content-Type", stringify(ContentType::JSON)}};

    Response response = process::http::request(request).get();
    LOG(INFO) << "De-registration result " << response.status << ": "
              << response.body;
  }

  return Option<string>::none();
}


} // namespace consul {
} // namespace criteo {
