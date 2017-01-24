
#ifndef NSPAWN_CONTAINERLAYER_HPP
#define	NSPAWN_CONTAINERLAYER_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "ContainerId.hpp"
#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {

class ContainerLayer {

    std::string base_path;
    ContainerId id;

public:

    ContainerLayer(const std::string& base_path, const std::string layer_name) :
    base_path(base_path.data(), base_path.length()),
    id(layer_name) { }

    ContainerLayer(const ContainerLayer&) = delete;

    ContainerLayer& operator=(const ContainerLayer&) = delete;

    ContainerLayer(ContainerLayer&& other) :
    base_path(std::move(other.base_path)),
    id(std::move(other.id)) { }

    ContainerLayer& operator=(ContainerLayer&& other) {
        base_path = std::move(other.base_path);
        id = std::move(other.id);
        return *this;
    }

    std::string get_path() const {
        return std::string(base_path) + id.as_name();
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            { "ID", id.as_id() },
            { "Path", get_path() }
        };
    }

};

} // namespace

#endif // NSPAWN_CONTAINERLAYER_HPP
