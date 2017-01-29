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
#include <chrono>

#include "staticlib/utils.hpp"

namespace nspawn {
namespace utils {

namespace { // anonymous

namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

std::string current_datetime() {
    auto now = std::chrono::system_clock::now();
    auto cur = std::chrono::system_clock::to_time_t(now);
    struct tm time;
    localtime_s(&time, &cur);
    char tmpbuf[128];
    strftime(tmpbuf, 128, "%Y%m%d%H%M%S", &time);
    return std::string(tmpbuf);
}

} // namespace
}
