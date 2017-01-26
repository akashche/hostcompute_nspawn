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

#ifndef NSPAWN_PROCESSCONFIG_HPP
#define	NSPAWN_PROCESSCONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {


class ProcessConfig {
    std::string process_executable;
    std::vector<std::string> process_arguments;
    std::string mapped_directory;
    std::string stdout_filename;

public:

    ProcessConfig(const std::string& process_executable, const std::vector<std::string>& process_arguments,
            const std::string& mapped_directory, const std::string& stdout_filename) :
    process_executable(process_executable.data(), process_executable.length()),
    process_arguments(),
    mapped_directory(mapped_directory.data(), mapped_directory.length()),
    stdout_filename(stdout_filename.data(), stdout_filename.length()) {
        for (auto& st : process_arguments) {
            this->process_arguments.emplace_back(st.data(), st.length());
        }
    }

    ProcessConfig(const ProcessConfig&) = delete;

    ProcessConfig& operator=(const ProcessConfig&) = delete;

    ProcessConfig(ProcessConfig&& other):
    process_executable(std::move(other.process_executable)),
    mapped_directory(std::move(other.mapped_directory)),
    stdout_filename(std::move(stdout_filename)) { }

    ProcessConfig& operator=(ProcessConfig&& other) {
        this->process_executable = std::move(other.process_executable);
        this->mapped_directory = std::move(other.mapped_directory);
        this->stdout_filename = std::move(other.stdout_filename);
        return *this;
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace ss = staticlib::serialization;
        std::vector<ss::JsonValue> console_dimensions;
        std::vector<ss::JsonField> env;
        std::string cline = std::string("C:\\Windows\\System32\\cmd.exe /c ") +
                mapped_directory + "\\" + process_executable;
        for (auto& ar : process_arguments) {
            cline.append(" ");
            cline.append(ar);
        }
        cline.append(std::string(" >> ") + stdout_filename + " 2>&1");
        return {
            {"ApplicationName", ""}, 
            {"CommandLine", cline},
            {"User", ""},
            {"WorkingDirectory", mapped_directory},
            {"Environment", std::move(env)},
            {"EmulateConsole", false},
            {"CreateStdInPipe", false},
            {"CreateStdOutPipe", false},
            {"CreateStdErrPipe", false},
            {"ConsoleSize", std::move(console_dimensions)}
        };
    }
};

} // namespace


#endif // NSPAWN_PROCESSCONFIG_HPP
