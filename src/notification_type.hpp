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


#ifndef NSPAWN_NOTIFICATION_TYPE_HPP
#define	NSPAWN_NOTIFICATION_TYPE_HPP

#include <cstdint>

namespace nspawn {

enum class notification_type : uint32_t {
    // Notifications for HCS_SYSTEM handles
    system_exit = 0x00000001,
    system_create_complete = 0x00000002,
    system_start_complete = 0x00000003,
    system_pause_complete = 0x00000004,
    system_resume_complete = 0x00000005,

    // Notifications for HCS_PROCESS handles
    process_exit = 0x00010000,

    // Common notifications
    common_invalid = 0x00000000,
    common_service_disconnect = 0x01000000
};

} // namespace

#endif // NSPAWN_NOTIFICATION_TYPE_HPP
