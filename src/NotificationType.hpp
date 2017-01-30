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


#ifndef NSPAWN_NOTIFICATIONTYPE_HPP
#define	NSPAWN_NOTIFICATIONTYPE_HPP

#include <cstdint>

namespace nspawn {

enum class NotificationType : uint32_t {
    // Notifications for HCS_SYSTEM handles
    SYSTEM_EXIT = 0x00000001,
    SYSTEM_CREATE_COMPLETE = 0x00000002,
    SYSTEM_START_COMPLETE = 0x00000003,
    SYSTEM_PAUSE_COMPLETE = 0x00000004,
    SYSTEM_RESUME_COMPLETE = 0x00000005,

    // Notifications for HCS_PROCESS handles
    PROCESS_EXIT = 0x00010000,

    // Common notifications
    COMMON_INVALID = 0x00000000,
    COMMON_SERVICE_DISCONNECT = 0x01000000
};

} // namespace

#endif // NSPAWN_NOTIFICATIONTYPE_HPP
