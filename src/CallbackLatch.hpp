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


#ifndef NSPAWN_CALLBACKLATCH_HPP
#define	NSPAWN_CALLBACKLATCH_HPP

#include <atomic>
#include <mutex>
#include <condition_variable>

#include "staticlib/config.hpp"

#include "NotificationType.hpp"
#include "NSpawnException.hpp"

namespace nspawn {

class CallbackLatch {
    std::mutex mutex;
    std::condition_variable system_create_cv;
    std::atomic<bool> system_create_flag = false;
    std::condition_variable system_start_cv;
    std::atomic<bool> system_start_flag = false;
    std::condition_variable system_exit_cv;
    std::atomic<bool> system_exit_flag = false;
    std::condition_variable process_exit_cv;
    std::atomic<bool> process_exit_flag = false;

public:
    CallbackLatch() { }

    CallbackLatch(const CallbackLatch&) = delete;

    CallbackLatch& operator=(const CallbackLatch&) = delete;

    void lock() {
        std::unique_lock<std::mutex> guard{ mutex };
        guard.release();
    }

    void await(NotificationType nt) {
        switch (nt) {
        case NotificationType::SYSTEM_CREATE_COMPLETE: await_internal(system_create_cv, system_create_flag); break;
        case NotificationType::SYSTEM_START_COMPLETE: await_internal(system_start_cv, system_start_flag); break;
        case NotificationType::SYSTEM_EXIT: await_internal(system_exit_cv, system_exit_flag); break;
        case NotificationType::PROCESS_EXIT: await_internal(process_exit_cv, process_exit_flag); break;
        default: throw NSpawnException(TRACEMSG("Unsupported notification type"));
        }
    }

    void unlock(NotificationType nt) {
        switch (nt) {
        case NotificationType::SYSTEM_CREATE_COMPLETE: unlock_internal(system_create_cv, system_create_flag); break;
        case NotificationType::SYSTEM_START_COMPLETE: unlock_internal(system_start_cv, system_start_flag); break;
        case NotificationType::SYSTEM_EXIT: unlock_internal(system_exit_cv, system_exit_flag); break;
        case NotificationType::PROCESS_EXIT: unlock_internal(process_exit_cv, process_exit_flag); break;
        default: { /* ignore */ }
        }
    }

    void cancel() {
        std::unique_lock<std::mutex> guard{ mutex, std::adopt_lock };
    }

private:
    void await_internal(std::condition_variable& cv, std::atomic<bool>& flag) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
        cv.wait(guard, [&flag]{
            return flag.load();
        });
    }

    void unlock_internal(std::condition_variable& cv, std::atomic<bool>& flag) {
        static bool the_false = false;
        if (flag.compare_exchange_strong(the_false, true)) {
            std::unique_lock<std::mutex> guard{mutex};
            cv.notify_all();
        }
    }
};

} // namespace

#endif // NSPAWN_CALLBACKLATCH_HPP

