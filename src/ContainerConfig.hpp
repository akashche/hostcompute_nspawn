
#ifndef NSPAWN_CONTAINERCONFIG_HPP
#define	NSPAWN_CONTAINERCONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "ContainerLayer.hpp"
#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {

class ContainerConfig {
    std::string name;
    std::string volume_path;
    ContainerLayer own_layer;
    std::vector<ContainerLayer> parent_layers;
    std::string hostname;

public:

    ContainerConfig(const std::string& name, const std::string& volume_path, ContainerLayer&& own_layer,
            std::vector<ContainerLayer>&& parent_layers, const std::string& hostname) :
    name(std::move(name)),
    volume_path(std::move(volume_path)),
    own_layer(std::move(own_layer)),
    parent_layers(std::move(parent_layers)),
    hostname(std::move(hostname)) { }

    ContainerConfig(const ContainerConfig&) = delete;

    ContainerConfig& operator=(const ContainerConfig&) = delete;

    ContainerConfig(ContainerConfig&& other):
    name(std::move(other.name)),
    volume_path(std::move(other.volume_path)),
    own_layer(std::move(other.own_layer)),
    parent_layers(std::move(other.parent_layers)),
    hostname(std::move(other.hostname)) { }

    ContainerConfig& operator=(ContainerConfig&& other) {
        this->name = std::move(other.name);
        this->volume_path = std::move(other.volume_path);
        this->own_layer = std::move(other.own_layer);
        this->parent_layers = std::move(other.parent_layers);
        this->hostname = std::move(other.hostname);
        return *this;
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace ss = staticlib::serialization;
        std::vector<ss::JsonValue> layers;
        for (auto& la : parent_layers) {
            layers.emplace_back(la.to_json());
        }

        return {
            { "SystemType", "Container" },
            { "Name", name },
            { "Owner", "hostcompute_nspawn" },
            { "IsDummy", false },
            { "VolumePath", volume_path },
            { "IgnoreFlushesDuringBoot", true },
            { "LayerFolderPath", own_layer.get_path() },
            { "Layers", std::move(layers) },
            { "HostName", hostname },
            { "MappedDirectories", std::vector<ss::JsonValue>() }, 
            { "HvPartition", false },
            { "EndpointList", std::vector<ss::JsonValue>() },
            { "Servicing", false },
            { "AllowUnqualifiedDNSQuery", true }
        };
    }

};

} // namespace


#endif // NSPAWN_CONTAINERCONFIG_HPP
