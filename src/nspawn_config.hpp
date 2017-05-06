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

#ifndef NSPAWN_NSPAWN_CONFIG_HPP
#define	NSPAWN_NSPAWN_CONFIG_HPP

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"
#include "staticlib/ranges.hpp"

#include "nspawn_exception.hpp"
#include "utils.hpp"

namespace nspawn {

class nspawn_config {
public:
    std::string process_directory;
    std::string process_executable;
    std::vector<std::string> process_arguments;
    
    uint32_t max_ram_mb = 0;
    uint16_t cpus_count = 0;
    uint16_t cpu_affinity_hex = 0;
    uint16_t max_cpu_percent = 0;
    uint32_t max_storage_iops = 0;
    uint32_t max_storage_bandwidth_bytes_per_sec = 0;

    std::string mapped_directory;
    std::string stdout_filename;

    std::string parent_layer_directory;

    nspawn_config(const nspawn_config&) = delete;

    nspawn_config& operator=(const nspawn_config&) = delete;

    nspawn_config(nspawn_config&& other):
    process_directory(std::move(other.process_directory)),
    process_executable(std::move(other.process_executable)),
    process_arguments(std::move(other.process_arguments)),
    max_ram_mb(other.max_ram_mb),
    cpus_count(other.cpus_count),
    cpu_affinity_hex(other.cpu_affinity_hex),
    max_cpu_percent(other.max_cpu_percent),
    max_storage_iops(other.max_storage_iops),
    max_storage_bandwidth_bytes_per_sec(other.max_storage_bandwidth_bytes_per_sec),
    mapped_directory(std::move(other.mapped_directory)),
    stdout_filename(std::move(other.stdout_filename)),
    parent_layer_directory(std::move(other.parent_layer_directory)) {
        other.max_ram_mb = 0;
        other.cpus_count = 0;
        other.cpu_affinity_hex = 0;
    }

    nspawn_config& operator=(nspawn_config&& other) {
        process_directory = std::move(other.process_directory);
        process_executable = std::move(other.process_executable);
        process_arguments = std::move(other.process_arguments);
        max_ram_mb = other.max_ram_mb;
        other.max_ram_mb = 0;
        cpus_count = other.cpus_count;
        other.cpus_count = 0;
        cpu_affinity_hex = other.cpu_affinity_hex;
        other.cpu_affinity_hex = 0;
        max_cpu_percent = other.max_cpu_percent;
        other.max_cpu_percent = 0;
        max_storage_iops = other.max_storage_iops;
        other.max_storage_iops = 0;
        max_storage_bandwidth_bytes_per_sec = other.max_storage_bandwidth_bytes_per_sec;
        other.max_storage_bandwidth_bytes_per_sec = 0;
        mapped_directory = std::move(other.mapped_directory);
        stdout_filename = std::move(other.stdout_filename);
        parent_layer_directory = std::move(other.parent_layer_directory);
        return *this;
    }

    nspawn_config(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("process_directory" == name) {
                process_directory = replace_slashes(fi.as_string_or_throw(name));
            } else if ("process_executable" == name) {
                process_executable = replace_slashes(fi.as_string_or_throw(name));
            } else if ("process_arguments" == name) {
                for (auto& ar : fi.as_array_or_throw(name)) {
                    process_arguments.emplace_back(ar.as_string_or_throw(name));
                }
            } else if ("max_ram_mb" == name) {
                max_ram_mb = fi.as_uint32_or_throw(name);
            } else if ("cpus_count" == name) {
                cpus_count = fi.as_uint16_or_throw(name);
            } else if ("cpu_affinity_hex" == name) {
                cpu_affinity_hex = fi.as_uint16_or_throw(name);
            } else if ("max_cpu_percent" == name) {
                max_cpu_percent = fi.as_uint16_or_throw(name);
            } else if ("max_storage_iops" == name) {
                max_storage_iops = fi.as_uint32_or_throw(name);
            } else if ("max_storage_bandwidth_bytes_per_sec" == name) {
                max_storage_bandwidth_bytes_per_sec = fi.as_uint32_or_throw(name);
            } else if ("mapped_directory" == name) {
                mapped_directory = replace_slashes(fi.as_string_or_throw(name));
            } else if ("stdout_filename" == name) {
                stdout_filename = replace_slashes(fi.as_string_or_throw(name));
            } else if ("parent_layer_directory" == name) {
                parent_layer_directory = replace_slashes(fi.as_string_or_throw(name));
            } else {
                throw nspawn_exception(TRACEMSG("Unknown configuration field: [" + name + "]"));
            }
        }
        // validate
        if (process_directory.empty()) throw nspawn_exception(TRACEMSG(
            "Invalid 'config.json' specified, 'process_directory' must be non-empty"));
        if (process_executable.empty()) throw nspawn_exception(TRACEMSG(
            "Invalid 'config.json' specified, 'process_executable' must be non-empty"));
        if (mapped_directory.empty()) throw nspawn_exception(TRACEMSG(
            "Invalid 'config.json' specified, 'mapped_directory' must be non-empty"));
        if (stdout_filename.empty()) throw nspawn_exception(TRACEMSG(
            "Invalid 'config.json' specified, 'stdout_filename' must be non-empty"));
        if (parent_layer_directory.empty()) throw nspawn_exception(TRACEMSG(
            "Invalid 'config.json' specified, 'parent_layer_directory' must be non-empty"));
    }

    sl::json::value to_json() const {
        return {
            { "process_directory", process_directory },
            { "process_executable", process_executable },
            { "process_arguments", [this]() -> std::vector<sl::json::value> {
                auto args = sl::ranges::transform(process_arguments, [](const std::string& ar) {
                    return sl::json::value(ar);
                });
                return args.to_vector();
            }() },

            { "max_ram_mb", max_ram_mb },
            { "cpus_count", cpus_count },
            { "cpu_affinity_hex", cpu_affinity_hex },
			{ "max_cpu_percent", max_cpu_percent },

            { "mapped_directory", mapped_directory },
            { "stdout_filename", stdout_filename },

            { "parent_layer_directory", parent_layer_directory }
        };
    }

    std::string cpu_affinity_hex_str() const {
        std::stringstream ss;
        ss << std::hex << cpu_affinity_hex;
        return ss.str();
    }

private:
    static const std::string replace_slashes(std::string str) {
        std::replace(str.begin(), str.end(), '/', '\\');
        return str;
    }
};

} // namespace

#endif // NSPAWN_NSPAWN_CONFIG_HPP
