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

#ifndef NSPAWN_CONTAINER_LAYER_HPP
#define	NSPAWN_CONTAINER_LAYER_HPP

#include <cstdlib>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "vmcompute.hpp"
#include "container_id.hpp"
#include "nspawn_exception.hpp"
#include "utils.hpp"

namespace nspawn {

class container_layer {
    std::string base_path;
    std::wstring wpath;
    container_id id;

public:
    container_layer(const std::string& base_path, const std::string layer_name) :
    base_path(base_path.data(), base_path.length()),
    wpath(staticlib::utils::widen(std::string(base_path) + layer_name)),
    id(layer_name) { }

    container_layer(const container_layer&) = delete;

    container_layer& operator=(const container_layer&) = delete;

    container_layer(container_layer&& other) :
        base_path(std::move(other.base_path)),
        wpath(std::move(other.wpath)),
        id(std::move(other.id)) { }

    container_layer& operator=(container_layer&& other) {
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

    staticlib::serialization::json_value to_json() const {
        return {
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

    container_layer clone() const {
        return container_layer(base_path, id.get_name());
    }

};

} // namespace

#endif // NSPAWN_CONTAINER_LAYER_HPP
