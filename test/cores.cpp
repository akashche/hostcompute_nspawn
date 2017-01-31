/*
 * Copyright 2017, akashche at redhat.com
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

#include <cstdint>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <utmpx.h>
#endif

int32_t get_current_cpu() {
#ifdef _WIN32
    auto cpu = GetCurrentProcessorNumber();
#else
    auto cpu = sched_getcpu();
#endif
    return static_cast<int32_t>(cpu);
}

int main() {
    std::atomic<bool> reported{false};

    // detected cpu number for main thread
    std::atomic<int32_t> cpunum{get_current_cpu()};

    // detect number of cores
    std::cout << "cores detected: " << std::thread::hardware_concurrency() << std::endl;

    std::vector<std::thread> vector;
    for (int i = 0; i < 64; i++) {
        auto thread = std::thread([&]{
            std::this_thread::sleep_for(std::chrono::seconds(1));
            int32_t cur = get_current_cpu();
            if (get_current_cpu() != cpunum.load()) {
                bool already_reported = reported.exchange(true);
                if (!already_reported) {
                    std::cout << "Error: running on a more than one core," <<
                            " number1: [" << cpunum.load() << "]" <<
                            " number2: [" << cur << "]"<< std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
        vector.emplace_back(std::move(thread));
    }

    for (auto& th : vector) {
        th.join();
    }

    if (!reported.load()) {
        std::cout << "Success: running on a single core, number: [" << cpunum.load() << "]" << std::endl;
    }

    return 0;
}
