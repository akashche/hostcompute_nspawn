
#ifndef NSPAWN_CONTAINERLAYER_HPP
#define	NSPAWN_CONTAINERLAYER_HPP

#include <cstdlib>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "vmcompute.hpp"
#include "ContainerId.hpp"
#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {

    class ContainerLayer {
        std::string base_path;
        std::wstring wpath;
        ContainerId id;
    
    public:

        ContainerLayer(const std::string& base_path, const std::string layer_name) :
            base_path(base_path.data(), base_path.length()),
            wpath(staticlib::utils::widen(std::string(base_path) + layer_name)),
            id(layer_name) { }

        ContainerLayer(const ContainerLayer&) = delete;

        ContainerLayer& operator=(const ContainerLayer&) = delete;

        ContainerLayer(ContainerLayer&& other) :
            base_path(std::move(other.base_path)),
            wpath(std::move(other.wpath)),
            id(std::move(other.id)) { }

        ContainerLayer& operator=(ContainerLayer&& other) {
            base_path = std::move(other.base_path);
            wpath = std::move(other.wpath);
            id = std::move(other.id);
            return *this;
        }

        const std::string& get_name() const {
            return id.get_name();
        }

        std::string get_path() const {
            return std::string(base_path) + id.get_name();
        }

        staticlib::serialization::JsonValue to_json() const {
            return{
                { "ID", id.to_id() },
                { "Path", get_path() }
            };
        }

        WC_LAYER_DESCRIPTOR to_descriptor() const {
            WC_LAYER_DESCRIPTOR res;
            std::memset(std::addressof(res), '\0', sizeof(WC_LAYER_DESCRIPTOR));
            res.LayerId = id.to_guid();
            res.Path = wpath.c_str();
            return res;
        }

        ContainerLayer clone() {
            return ContainerLayer(base_path, id.get_name());
        }

};

} // namespace

#endif // NSPAWN_CONTAINERLAYER_HPP
