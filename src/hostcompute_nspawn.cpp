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

#include "hostcompute_nspawn.h"

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "NSpawnConfig.hpp"
#include "NSpawnException.hpp"
#include "ContainerConfig.hpp"
#include "ContainerId.hpp"
#include "ContainerLayer.hpp"
#include "ProcessConfig.hpp"


namespace { // anonymous 

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace sr = staticlib::ranges;
namespace ss = staticlib::serialization;
namespace st = staticlib::tinydir;
namespace su = staticlib::utils;

} // namespace

namespace nspawn {

enum class HcsErrors : uint32_t {
    OPERATION_PENDING = 0xC0370103
};

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

DriverInfo create_driver_info(const std::wstring& wide_base_path) {
    DriverInfo res;
    std::memset(std::addressof(res), '\0', sizeof(DriverInfo));
    res.Flavour = GraphDriverType::FilterDriver;
    res.HomeDir = wide_base_path.c_str();
    return res;
}

std::vector<ContainerLayer> collect_acsendant_layers(const std::string& base_path,
        const std::string& parent_layer_name) {
    std::vector<ContainerLayer> res;
    res.emplace_back(base_path, parent_layer_name);
    auto json_file = std::string(base_path) + "\\" + parent_layer_name + "\\layerchain.json";
    auto fd = su::FileDescriptor(json_file, 'r');
    auto json = ss::load_json(fd);
    std::cout << "Ascendant layers: " << ss::dump_json_to_string(json) << std::endl;
    for (auto& el : json.as_array_or_throw(json_file)) {
        std::string path = el.as_string_or_throw(json_file);
        std::string dir = su::strip_filename(path);
        std::string file = su::strip_parent_dir(path);
        res.emplace_back(dir, file);
    }
    return res;
}

std::vector<WC_LAYER_DESCRIPTOR> create_ascendant_descriptors(const std::vector<ContainerLayer>& acsendant_layers) {
    auto ra = sr::transform(sr::refwrap(acsendant_layers), [](const ContainerLayer& la){
        return la.to_descriptor();
    });
    return sr::emplace_to_vector(std::move(ra));
}

void hcs_create_layer(DriverInfo& driver_info, ContainerLayer& layer, const std::string& parent_layer_name, 
        std::vector<WC_LAYER_DESCRIPTOR>& acsendant_descriptors) {
    std::wstring wname = su::widen(layer.get_name());
    std::wstring wparent = su::widen(parent_layer_name);
    auto err = ::CreateSandboxLayer(std::addressof(driver_info), wname.c_str(), wparent.c_str(),
            acsendant_descriptors.data(), static_cast<uint32_t>(acsendant_descriptors.size()));
    if (0 != err) {
        throw NSpawnException(TRACEMSG("'CreateSandboxLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " parent_layer_name: [" + parent_layer_name + "]," +
                " error: [" + su::errcode_to_string(err) + "]"));
    }
    std::cout << "CreateSandboxLayer: " << "Layer created, name: [" << layer.get_name() << "]" << std::endl;
}

void hcs_activate_layer(DriverInfo& driver_info, ContainerLayer& layer) {
    std::wstring wname = su::widen(layer.get_name());
    auto err = ::ActivateLayer(std::addressof(driver_info), wname.c_str());
    if (0 != err) {
        throw NSpawnException(TRACEMSG("'ActivateLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(err) + "]"));
    }
    std::cout << "ActivateLayer: " << "Layer activated, name: [" << layer.get_name() << "]" << std::endl;
}

void hcs_prepare_layer(DriverInfo& driver_info, ContainerLayer& layer, 
        std::vector<WC_LAYER_DESCRIPTOR>& acsendant_descriptors) {
    std::wstring wname = su::widen(layer.get_name());
    auto err = ::PrepareLayer(std::addressof(driver_info), wname.c_str(),
        acsendant_descriptors.data(), static_cast<uint32_t>(acsendant_descriptors.size()));
    if (0 != err) {
        throw NSpawnException(TRACEMSG("'PrepareLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(err) + "]"));
    }
    std::cout << "PrepareLayer: " << "Layer prepared, name: [" << layer.get_name() << "]" << std::endl;
}

std::string hcs_get_layer_mount_path(DriverInfo& driver_info, ContainerLayer& layer) {
    std::wstring wname = su::widen(layer.get_name());
    std::wstring path;
    path.resize(MAX_PATH);
    uint32_t length = MAX_PATH;
    auto err = ::GetLayerMountPath(std::addressof(driver_info), wname.c_str(),
        std::addressof(length), std::addressof(path.front()));
    if (0 != err) {
        throw NSpawnException(TRACEMSG("'GetLayerMountPath' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(err) + "]"));
    }
    std::string res = su::narrow(path.c_str());
    std::cout << "GetLayerMountPath: " << "Found volume path: [" << res << "]" <<
            " for layer, name: [" << layer.get_name() << "]" << std::endl;
    return res;
}

HANDLE hcs_create_compute_system(ContainerConfig& config, ContainerLayer& layer) {
    std::wstring wname = su::widen(layer.get_name());
    std::string conf = ss::dump_json_to_string(config.to_json());
    std::wstring wconf = su::widen(conf);
    HANDLE identity = nullptr;
    HANDLE computeSystem = nullptr;
    wchar_t* result = nullptr;
    auto res = ::HcsCreateComputeSystem(wname.c_str(), wconf.c_str(), identity,
        std::addressof(computeSystem), std::addressof(result));
    if (static_cast<uint32_t>(HcsErrors::OPERATION_PENDING) != res) {
        throw NSpawnException(TRACEMSG("'HcsCreateComputeSystem' failed," +
                " config: [" + conf + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    std::cout << "HcsCreateComputeSystem: " << "Container created, name: [" << layer.get_name() << "]" << std::endl;
    return computeSystem;
}

void container_callback(uint32_t notificationType, void* context, int32_t notificationStatus,
        wchar_t* notificationData) STATICLIB_NOEXCEPT {
    std::string data = nullptr != notificationData ? su::narrow(notificationData) : "";
    std::cout << "CS notification received, notificationType: [" << sc::to_string(notificationType) << "]," <<
            " notificationStatus: [" << notificationStatus << "]," <<
            " notificationData: [" << data << "]" << std::endl;
    CallbackLatch& la = *static_cast<CallbackLatch*> (context);
    la.unlock(static_cast<NotificationType>(notificationType));
};

HANDLE hcs_register_compute_system_callback(HANDLE compute_system, ContainerLayer& layer,
        CallbackLatch& latch) {
    HANDLE cs_callback_handle = nullptr;
    latch.lock();
    auto res = ::HcsRegisterComputeSystemCallback(compute_system, container_callback, static_cast<void*>(std::addressof(latch)),
        std::addressof(cs_callback_handle));
    if (0 != res) {
        latch.cancel();
        throw NSpawnException(TRACEMSG("'HcsRegisterComputeSystemCallback' failed," +
                " name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    std::cout << "HcsRegisterComputeSystemCallback: " << "CS callback registered successfully, name: [" << layer.get_name() << "]" << std::endl;
    latch.await(NotificationType::SYSTEM_CREATE_COMPLETE);
    std::cout << "HcsRegisterComputeSystemCallback: " << "CS create latch unlocked" << std::endl;
    return cs_callback_handle;
}

void hcs_start_compute_system(HANDLE compute_system, ContainerLayer& layer, CallbackLatch& latch) {
    std::wstring options = su::widen("");
    wchar_t* result = nullptr;
    latch.lock();
    auto res = ::HcsStartComputeSystem(compute_system, options.c_str(), std::addressof(result));
    if (static_cast<uint32_t>(HcsErrors::OPERATION_PENDING) != res) {
        latch.cancel();
        throw NSpawnException(TRACEMSG("'HcsStartComputeSystem' failed," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    latch.await(NotificationType::SYSTEM_START_COMPLETE);
    std::cout << "HcsStartComputeSystem: " << "Container started, name: [" << layer.get_name() << "]" << std::endl;
}

void hcs_enumerate_compute_systems() {
    std::wstring query = su::widen("{}");
    wchar_t* computeSystems = nullptr;
    wchar_t* result = nullptr;
    auto res = ::HcsEnumerateComputeSystems(query.c_str(),
            std::addressof(computeSystems), std::addressof(result));
    if (0 != res) {
        throw NSpawnException(TRACEMSG("'HcsEnumerateComputeSystems' failed," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    std::cout << "HcsEnumerateComputeSystems: " << "Compute systems found: " << su::narrow(computeSystems) << std::endl;
}

HANDLE hcs_create_process(HANDLE compute_system, const NSpawnConfig& config) {
    HANDLE process = nullptr;
    auto pcfg = ProcessConfig(config.process_executable, config.process_arguments, 
            config.mapped_directory, config.stdout_filename);
    std::string pcfg_json = ss::dump_json_to_string(pcfg.to_json());
    std::cout << "Process config: " << pcfg_json << std::endl;
    std::wstring wpcfg_json = su::widen(pcfg_json);
    HCS_PROCESS_INFORMATION hpi;
    std::memset(std::addressof(hpi), '\0', sizeof(HCS_PROCESS_INFORMATION));
    wchar_t* result = nullptr;
    auto res = ::HcsCreateProcess(compute_system, wpcfg_json.c_str(), std::addressof(hpi),
        std::addressof(process), std::addressof(result));
    if (0 != res) {
        throw NSpawnException(TRACEMSG("'HcsCreateProcess' failed," +
                " config: [" + pcfg_json + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    std::cout << "HcsCreateProcess: " << "Process created" << std::endl;
    return process;
}

HANDLE hcs_register_process_callback(HANDLE process, ContainerLayer& layer, CallbackLatch& latch) {
    HANDLE process_callback_handle;
    latch.lock();
    auto res = ::HcsRegisterProcessCallback(process, container_callback, std::addressof(latch), std::addressof(process_callback_handle));
    if (0 != res) {
        latch.cancel();
        throw NSpawnException(TRACEMSG("'HcsRegisterProcessCallback' failed," +
                " name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
    }
    std::cout << "HcsRegisterProcessCallback: " << "Process callback registered successfully, name: [" << layer.get_name() << "]" << std::endl;
    latch.await(NotificationType::PROCESS_EXIT);
    std::cout << "HcsRegisterProcessCallback: " << "Process create latch unlocked" << std::endl;
    return process_callback_handle;
}

void hcs_terminate_compute_system(HANDLE compute_system, ContainerLayer& layer, CallbackLatch& latch) STATICLIB_NOEXCEPT {
    std::wstring options = su::widen("{}");
    wchar_t* result = nullptr;
    latch.lock();
    auto res = ::HcsTerminateComputeSystem(compute_system, options.c_str(), std::addressof(result));
    if (static_cast<uint32_t>(HcsErrors::OPERATION_PENDING) == res) {
        latch.await(NotificationType::SYSTEM_EXIT);
        std::cout << "HcsTerminateComputeSystem: " << "Container terminated, name: [" << layer.get_name() << "]" << std::endl;
    }
    else {
        latch.cancel();
        std::cerr << "ERROR: 'HcsTerminateComputeSystem' failed, name: [" << layer.get_name() << "]" <<
            " error: [" << su::errcode_to_string(res) << "]" << std::endl;
    }
}

void hcs_unprepare_layer(DriverInfo& driver_info, ContainerLayer& layer) STATICLIB_NOEXCEPT {
    std::wstring wname = su::widen(layer.get_name());
    auto res = ::UnprepareLayer(std::addressof(driver_info), wname.c_str());
    if (0 == res) {
        std::cout << "UnprepareLayer: " << "Layer unprepared, name: [" << layer.get_name() << "]" << std::endl;
    }
    else {
        std::cerr << "ERROR: 'UnprepareLayer' failed, name: [" << layer.get_name() << "]" <<
            " error: [" << su::errcode_to_string(res) << "]" << std::endl;
    }
}

void hcs_deactivate_layer(DriverInfo& driver_info, ContainerLayer& layer) STATICLIB_NOEXCEPT {
    std::wstring wname = su::widen(layer.get_name());
    auto res = ::DeactivateLayer(std::addressof(driver_info), wname.c_str());
    if (0 == res) {
        std::cout << "DeactivateLayer: " << "Layer deactivated, name: [" << layer.get_name() << "]" << std::endl;
    }
    else {
        std::cerr << "ERROR: 'DeactivateLayer' failed, name: [" << layer.get_name() << "]" <<
            " error: [" << su::errcode_to_string(res) << "]" << std::endl;
    }
}

void hcs_destroy_layer(DriverInfo& driver_info, ContainerLayer& layer) STATICLIB_NOEXCEPT {
    std::wstring wname = su::widen(layer.get_name());
    auto res = ::DestroyLayer(std::addressof(driver_info), wname.c_str());
    if (0 == res) {
        std::cout << "DestroyLayer: " << "Layer destroyed, name: [" << layer.get_name() << "]" << std::endl;
    }
    else {
        std::cerr << "ERROR: 'DestroyLayer' failed, name: [" << layer.get_name() << "]" <<
            " error: [" << su::errcode_to_string(res) << "]" << std::endl;
    }
}

void spawn_and_wait(const NSpawnConfig& config) {
    std::cout << "NSpawn config: " << ss::dump_json_to_string(config.to_json()) << std::endl;

    // common parameters
    auto rng = su::RandomStringGenerator("0123456789abcdef");
    std::string base_path = su::strip_filename(config.parent_layer_directory);
    std::wstring wide_base_path = su::widen(base_path);
    std::string parent_layer_name = su::strip_parent_dir(config.parent_layer_directory);

    // prepare DriverInfo
    DriverInfo driver_info = create_driver_info(wide_base_path);

    // prepare acsendants
    auto acsendant_layers = collect_acsendant_layers(base_path, parent_layer_name);
    auto acsendant_descriptors = create_ascendant_descriptors(acsendant_layers);

    // prepare new layer
    auto layer = ContainerLayer(base_path, std::string("nspawn_") + utils::current_datetime() + "_" + rng.generate(26));
    hcs_create_layer(driver_info, layer, parent_layer_name, acsendant_descriptors);
    auto deferred_destroy_layer = sc::defer([&driver_info, &layer]() STATICLIB_NOEXCEPT {
        hcs_destroy_layer(driver_info, layer);
    });
    hcs_activate_layer(driver_info, layer);
    auto deferred_deactivate_layer = sc::defer([&driver_info, &layer]() STATICLIB_NOEXCEPT {
        hcs_deactivate_layer(driver_info, layer);
    });
    hcs_prepare_layer(driver_info, layer, acsendant_descriptors);
    auto deferred_unprepare_layer = sc::defer([&driver_info, &layer]() STATICLIB_NOEXCEPT {
        hcs_unprepare_layer(driver_info, layer);
    });
    std::string volume_path = hcs_get_layer_mount_path(driver_info, layer);

    // create and start container
    auto container_config = ContainerConfig(base_path, config.process_directory, config.mapped_directory,
        volume_path, layer.clone(), acsendant_layers, rng.generate(8));
    std::cout << "Container config: " << ss::dump_json_to_string(container_config.to_json()) << std::endl;
    HANDLE compute_system = hcs_create_compute_system(container_config, layer);
    CallbackLatch cs_latch;
    hcs_register_compute_system_callback(compute_system, layer, cs_latch);
    hcs_start_compute_system(compute_system, layer, cs_latch);
    auto deferred_terminate_cs = sc::defer([&compute_system, &layer, &cs_latch]() STATICLIB_NOEXCEPT {
        hcs_terminate_compute_system(compute_system, layer, cs_latch);
    });

    // list existing containers
    hcs_enumerate_compute_systems();

    // create process and wait for it to exit
    HANDLE process = hcs_create_process(compute_system, config);
    hcs_register_process_callback(process, layer, cs_latch);
}

} // namespace
 
char* hostcompute_nspawn(const char* config_json, int config_json_len) /* noexcept */ {
    if (nullptr == config_json) return su::alloc_copy(TRACEMSG("Null 'config_json' parameter specified"));
    if (!sc::is_uint32_positive(config_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'config_json_len' parameter specified: [" + sc::to_string(config_json_len) + "]"));
    try {
        auto src = si::array_source(config_json, config_json_len);
        auto loaded = ss::load_json(src);
        auto config = nspawn::NSpawnConfig(loaded);
        nspawn::spawn_and_wait(config);
        return nullptr;
    }
    catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

void hostcompute_nspawn_free(char* err_message) /* noexcept */ {
    std::free(err_message);
}
