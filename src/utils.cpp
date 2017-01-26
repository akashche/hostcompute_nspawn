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

#include "utils.hpp"

#include <ctime>

#include "staticlib/utils.hpp"

namespace nspawn {
namespace utils {

namespace { // anonymous

namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

const std::string& get_json_string(const ss::JsonField& field) {
    if (ss::JsonType::STRING != field.type() || field.as_string().empty()) {
        throw NSpawnException(TRACEMSG("Invalid '" + field.name() + "' field,"
            " type: [" + ss::stringify_json_type(field.type()) + "]," +
            " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_string();
}

const std::string& get_json_string(const ss::JsonValue& value, const std::string& name) {
    if (ss::JsonType::STRING != value.type() || value.as_string().empty()) {
        throw NSpawnException(TRACEMSG("Invalid '" + name + "' value,"
            " type: [" + ss::stringify_json_type(value.type()) + "]," +
            " value: [" + ss::dump_json_to_string(value) + "]"));
    }
    return value.as_string();
}

const std::vector<ss::JsonValue>& get_json_array(const ss::JsonField& field) {
    if (ss::JsonType::ARRAY != field.type()) {
        throw NSpawnException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_array();
}

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonValue& value, const std::string& name) {
    if (ss::JsonType::ARRAY != value.type()) {
        throw NSpawnException(TRACEMSG("Invalid '" + name + "' field,"
            " type: [" + ss::stringify_json_type(value.type()) + "]," +
            " value: [" + ss::dump_json_to_string(value) + "]"));
    }
    return value.as_array();
}

// todo: use chrono
std::string current_datetime() {
    time_t cur = time(NULL);
    struct tm time;
    localtime_s(&time, &cur);
    char tmpbuf[128];
    strftime(tmpbuf, 128, "%Y%m%d%H%M%S", &time);
    return std::string(tmpbuf);
}

} // namespace
}
