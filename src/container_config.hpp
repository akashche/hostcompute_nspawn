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
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "container_layer.hpp"
#include "nspawn_exception.hpp"
#include "utils.hpp"

namespace nspawn {

class container_config {
    std::string name;
    std::string process_dir;
    uint32_t max_ram_mb = 0;
    uint16_t cpus_count = 0;
    std::string mapped_dir;
    std::string volume_path;
    container_layer own_layer;
    std::vector<container_layer> parent_layers;
    std::string hostname;

public:

    container_config(const std::string& name, const std::string& process_dir,
            uint32_t max_ram_mb, uint16_t cpus_count,
            const std::string& mapped_dir, const std::string& volume_path, container_layer&& own_layer, 
            const std::vector<container_layer>& parent_layers, const std::string& hostname) :
    name(std::move(name)),
    process_dir(std::move(process_dir)),
    max_ram_mb(max_ram_mb),
    cpus_count(cpus_count),
    mapped_dir(std::move(mapped_dir)),
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
    process_dir(std::move(other.process_dir)),
    max_ram_mb(other.max_ram_mb),
    cpus_count(other.cpus_count),
    mapped_dir(std::move(other.mapped_dir)),
    volume_path(std::move(other.volume_path)),
    own_layer(std::move(other.own_layer)),
    parent_layers(std::move(other.parent_layers)),
    hostname(std::move(other.hostname)) {
        other.max_ram_mb = 0;
        other.cpus_count = 0;
    }

    container_config& operator=(container_config&& other) {
        name = std::move(other.name);
        process_dir = std::move(other.process_dir);
        max_ram_mb = other.max_ram_mb;
        other.max_ram_mb = 0;
        cpus_count = other.cpus_count;
        other.cpus_count = 0;
        mapped_dir = std::move(other.mapped_dir);
        volume_path = std::move(other.volume_path);
        own_layer = std::move(other.own_layer);
        parent_layers = std::move(other.parent_layers);
        hostname = std::move(other.hostname);
        return *this;
    }

    staticlib::serialization::json_value to_json() const {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        return {
            { "SystemType", "Container" },
            { "Name", name },
            { "Owner", "hostcompute_nspawn" },
            { "IsDummy", false },
            { "VolumePath", volume_path },
            { "IgnoreFlushesDuringBoot", true },
            { "LayerFolderPath", own_layer.get_path() },
            { "Layers", [this]() -> std::vector<ss::json_value> {
                auto ra = sr::transform(sr::refwrap(parent_layers), [](const container_layer& la) {
                    return la.to_json();
                });
                return ra.to_vector();
            }() },
            { "ProcessorCount", cpus_count },
            { "MemoryMaximumInMB", max_ram_mb },
            { "HostName", hostname },
            { "MappedDirectories", [this]() -> std::vector<ss::json_value> {
                ss::json_value mappeddir = {
                    { "HostPath", process_dir },
                    { "ContainerPath", mapped_dir },
                    { "ReadOnly", false },
                    { "BandwidthMaximum", 0 },
                    { "IOPSMaximum", 0 },
                };
                std::vector<ss::json_value> res;
                res.emplace_back(std::move(mappeddir));
                return res;
            }() },
            { "HvPartition", false },
            { "EndpointList", std::vector<ss::json_value>() },
            { "Servicing", false },
            { "AllowUnqualifiedDNSQuery", true }
        };
    }

};

} // namespace


#endif // NSPAWN_CONTAINER_CONFIG_HPP
