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

#ifndef NSPAWN_CONTAINER_CONFIG_HPP
#define	NSPAWN_CONTAINER_CONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/json.hpp"
#include "staticlib/ranges.hpp"

#include "container_layer.hpp"
#include "nspawn_config.hpp"
#include "nspawn_exception.hpp"
#include "utils.hpp"

namespace nspawn {

class container_config {
    sl::support::observer_ptr<const nspawn_config> nconf;
    std::string name;
    std::string volume_path;
    container_layer own_layer;
    std::vector<container_layer> parent_layers;
    std::string hostname;

public:
    container_config(const nspawn_config& nconf, const std::string& name, const std::string& volume_path, container_layer&& own_layer,
            const std::vector<container_layer>& parent_layers, const std::string& hostname) :
    nconf(nconf),
    name(std::move(name)),
    volume_path(std::move(volume_path)),
    own_layer(std::move(own_layer)),
    parent_layers(),
    hostname(std::move(hostname)) {
        namespace sr = staticlib::ranges;
        auto ra = sr::transform(sr::refwrap(parent_layers), [](const container_layer& la) {
            return la.clone();
        });
        sr::emplace_to(this->parent_layers, std::move(ra));
    }

    container_config(const container_config&) = delete;

    container_config& operator=(const container_config&) = delete;

    container_config(container_config&& other):
    name(std::move(other.name)),
    nconf(other.nconf),
    volume_path(std::move(other.volume_path)),
    own_layer(std::move(other.own_layer)),
    parent_layers(std::move(other.parent_layers)),
    hostname(std::move(other.hostname)) {
        other.nconf.reset();
    }

    container_config& operator=(container_config&& other) {
        name = std::move(other.name);
        nconf = other.nconf;
        other.nconf.reset();
        volume_path = std::move(other.volume_path);
        own_layer = std::move(other.own_layer);
        parent_layers = std::move(other.parent_layers);
        hostname = std::move(other.hostname);
        return *this;
    }

    sl::json::value to_json() const {
        return {
            { "SystemType", "Container" },
            { "Name", name },
            { "Owner", "hostcompute_nspawn" },
            { "IsDummy", false },
            { "VolumePath", volume_path },
            { "IgnoreFlushesDuringBoot", true },
            { "LayerFolderPath", own_layer.get_path() },
            { "Layers", [this]() -> std::vector<sl::json::value> {
                auto ra = sl::ranges::transform(parent_layers, [](const container_layer& la) {
                    return la.to_json();
                });
                return ra.to_vector();
            }() },
            { "MemoryMaximumInMB", nconf->max_ram_mb },
            { "ProcessorCount", nconf->cpus_count },
            { "ProcessorMaximum", nconf->max_cpu_percent * 100 },
            { "StorageIOPSMaximum", nconf->max_storage_iops },
            { "StorageBandwidthMaximum", nconf->max_storage_bandwidth_bytes_per_sec },
            { "HostName", hostname },
            { "MappedDirectories", [this]() -> std::vector<sl::json::value> {
                sl::json::value mappeddir = {
                    { "HostPath", nconf->process_directory },
                    { "ContainerPath", nconf->mapped_directory },
                    { "ReadOnly", false },                   
                    { "IOPSMaximum", nconf->max_storage_iops },
                    { "BandwidthMaximum", nconf->max_storage_bandwidth_bytes_per_sec },
                };
                std::vector<sl::json::value> res;
                res.emplace_back(std::move(mappeddir));
                return res;
            }() },
            { "HvPartition", false },
            { "EndpointList", std::vector<sl::json::value>() },
            { "Servicing", false },
            { "AllowUnqualifiedDNSQuery", true }
        };
    }

};

} // namespace


#endif // NSPAWN_CONTAINER_CONFIG_HPP
