/*
 * Copyright 2016, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NSPAWN_PROCESS_CONFIG_HPP
#define	NSPAWN_PROCESS_CONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"
#include "staticlib/ranges.hpp"

#include "nspawn_config.hpp"
#include "nspawn_exception.hpp"
#include "utils.hpp"

namespace nspawn {

class process_config {
    sl::support::observer_ptr<const nspawn_config> nconf;

public:
    process_config(const nspawn_config& nconf) :
    nconf(nconf) { }

    process_config(const process_config&) = delete;

    process_config& operator=(const process_config&) = delete;

    process_config(process_config&& other) :
    nconf(std::move(other.nconf)) { }

    process_config& operator=(process_config&& other) {
        nconf = std::move(other.nconf);
        return *this;
    }

    sl::json::value to_json() const {
        return {
            { "ApplicationName", "" }, 
            { "CommandLine", [this]() -> std::string {
                auto cline = std::string("C:\\Windows\\System32\\cmd.exe /c ");
                cline.append("start /b /wait /affinity 0x").append(nconf->cpu_affinity_hex_str()).append(" ");
                cline.append(nconf->mapped_directory).append("\\").append(nconf->process_executable);
                for (auto& ar : nconf->process_arguments) {
                    cline.append(" ").append(ar);
                }
                cline.append(" >> ").append(nconf->stdout_filename).append(" 2>&1");
                return cline;
            }() },
            { "User", "" },
            { "WorkingDirectory", nconf->mapped_directory },
            { "Environment", std::vector<sl::json::field>() },
            { "EmulateConsole", false },
            { "CreateStdInPipe", false },
            { "CreateStdOutPipe", false },
            { "CreateStdErrPipe", false },
            { "ConsoleSize", std::vector<sl::json::value>() }
        };
    }
};

} // namespace


#endif // NSPAWN_PROCESS_CONFIG_HPP
