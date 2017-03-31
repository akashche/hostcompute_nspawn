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

#ifndef NSPAWN_CONTAINER_ID_HPP
#define	NSPAWN_CONTAINER_ID_HPP

#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "nspawn_exception.hpp"
#include "vmcompute.hpp"

namespace nspawn {

class container_id {
    std::string name;

public:
    container_id(const std::string& name) :
    name(name.data(), name.length()) { }

    container_id(const container_id&) = delete;

    container_id& operator=(const container_id&) = delete;

    container_id(container_id&& other):
    name(std::move(other.name)) { }

    container_id& operator=(container_id&& other) {
        this->name = std::move(other.name);
        return *this;
    }

    const std::string& get_name() const {
        return name;
    }

    GUID to_guid() const {
        namespace sc = staticlib::config;
        namespace su = staticlib::utils;
        std::wstring wname = su::widen(name);
        GUID guid;
        auto res_ng = ::NameToGuid(wname.c_str(), std::addressof(guid));
        if (0 != res_ng) {
            throw nspawn_exception(TRACEMSG("Error obtaining GUID from name: [" + name + "]"));
        }
        return guid;
    }

    std::string to_id() const {
        namespace sc = staticlib::config;
        namespace su = staticlib::utils;
        GUID guid = this->to_guid();
        std::wstring guidstr;
        guidstr.resize(40);
        auto res_g2 = ::StringFromGUID2(guid, std::addressof(guidstr.front()), static_cast<int>(guidstr.length()));
        if (0 == res_g2) {
            throw nspawn_exception(TRACEMSG("Error converting GUID to string, name: [" + name + "]"));
        }
        std::string braceful = su::narrow(guidstr.c_str(), res_g2);
        std::transform(braceful.begin(), braceful.end(), braceful.begin(), ::tolower);
        std::string res = braceful.substr(1, braceful.length() - 3);
        return res;
    }
};

} //namespace

#endif // NSPAWN_CONTAINER_ID_HPP
