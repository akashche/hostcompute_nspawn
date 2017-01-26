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

#ifndef NSPAWN_UTILS_HPP
#define	NSPAWN_UTILS_HPP

#include <string>
#include <vector>

#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"

namespace nspawn {
namespace utils {

const std::string& get_json_string(const staticlib::serialization::JsonField& field);

const std::string& get_json_string(const staticlib::serialization::JsonValue& value, const std::string& name);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonValue& value, const std::string& name);

std::string current_datetime();

} // namespace
}

#endif	/* NSPAWN_UTILS_HPP */

