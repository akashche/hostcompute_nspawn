
#include "hostcompute_nspawn.h"

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/serialization.hpp"

#include "NSpawnConfig.hpp"
#include "NSpawnException.hpp"
#include "ContainerConfig.hpp"
#include "ContainerId.hpp"
#include "ContainerLayer.hpp"

namespace { // anonymous 

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace su = staticlib::utils;
namespace ss = staticlib::serialization;

} // namespace

namespace nspawn {

std::vector<ContainerLayer> collect_acsendant_layers(const std::string& base_path,
        const std::string& parent_layer_name) {
    std::vector<ContainerLayer> res;
    res.emplace_back(base_path, parent_layer_name);
    // todo: find out on disk
    res.emplace_back(base_path, "c98833436817d72e5a11b062890502b31fd5cfcb7b5b5047bcf8cc430d7a2166");
    return res;
}

void spawn_and_wait(const NSpawnConfig& config) {
    (void) config;

    // prepare DriverInfo
    std::string base_path = su::strip_filename(config.parent_layer_directory);
    std::replace(base_path.begin(), base_path.end(), '/', '\\');
    std::string parent_layer_name = su::strip_parent_dir(config.parent_layer_directory);
    std::wstring wbp = su::widen(base_path);
    DriverInfo driver_info;
    std::memset(std::addressof(driver_info), '\0', sizeof(DriverInfo));
    driver_info.Flavour = GraphDriverType::FilterDriver;
    driver_info.HomeDir = wbp.c_str();

    // prepare acsendant layers
    std::vector<ContainerLayer> acsendant_layers = collect_acsendant_layers(base_path, parent_layer_name);
    std::vector<WC_LAYER_DESCRIPTOR> acsendant_descriptors;
    for (auto& la : acsendant_layers) {
        acsendant_descriptors.emplace_back(la.to_descriptor());
    }

    auto rng = su::RandomStringGenerator("0123456789abcdef");
    auto layer = ContainerLayer(base_path, std::string("nspawn_") + utils::current_datetime() + "_" + rng.generate(26));

    { // create layer
        std::wstring wname = su::widen(layer.get_name());
        std::wstring wparent = su::widen(parent_layer_name);
        auto res = ::CreateSandboxLayer(std::addressof(driver_info), wname.c_str(), wparent.c_str(),
                acsendant_descriptors.data(), static_cast<uint32_t>(acsendant_descriptors.size()));
        if (0 != res) {
            throw NSpawnException(TRACEMSG("'CreateSandboxLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " parent_layer_name: [" + parent_layer_name + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        std::cout << "Layer created, name: [" << layer.get_name() << "]" << std::endl;
    }

    { // activate layer
        std::wstring wname = su::widen(layer.get_name());
        auto res = ::ActivateLayer(std::addressof(driver_info), wname.c_str());
        if (0 != res) {
            throw NSpawnException(TRACEMSG("'ActivateLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        std::cout << "Layer activated, name: [" << layer.get_name() << "]" << std::endl;
    }

    { // prepare layer
        std::wstring wname = su::widen(layer.get_name());
        auto res = ::PrepareLayer(std::addressof(driver_info), wname.c_str(), 
                acsendant_descriptors.data(), static_cast<uint32_t>(acsendant_descriptors.size()));
        if (0 != res) {
            throw NSpawnException(TRACEMSG("'PrepareLayer' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        std::cout << "Layer prepared, name: [" << layer.get_name() << "]" << std::endl;
    }

    std::string volume_path;

    { // find out mount path
        std::wstring wname = su::widen(layer.get_name());
        std::wstring path;
        path.resize(MAX_PATH);
        uint32_t length = MAX_PATH;
        auto res = ::GetLayerMountPath(std::addressof(driver_info), wname.c_str(), 
                std::addressof(length), std::addressof(path.front()));
        if (0 != res) {
            throw NSpawnException(TRACEMSG("'GetLayerMountPath' failed," +
                " layer_name: [" + layer.get_name() + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        volume_path = su::narrow(path.c_str());
        std::cout << "Found volume path: [" << volume_path << "]" <<
            " for layer, name: [" << layer.get_name() << "]" << std::endl;
    }

    std::vector<ContainerLayer> alpass;
    for (auto& la : acsendant_layers) {
        alpass.emplace_back(la.clone());
    }
    auto container_config = ContainerConfig(base_path, volume_path, layer.clone(),
            std::move(alpass), rng.generate(8));
    HANDLE computeSystem = nullptr;

    { // create container
        std::wstring wname = su::widen(layer.get_name());
        std::string conf = ss::dump_json_to_string(container_config.to_json());
        std::wstring wconf = su::widen(conf);
        HANDLE identity = nullptr;
        wchar_t* result = nullptr;
        auto res = ::HcsCreateComputeSystem(wname.c_str(), wconf.c_str(), identity, 
                std::addressof(computeSystem), std::addressof(result));
        if (0xC0370103 != res) {
            throw NSpawnException(TRACEMSG("'HcsCreateComputeSystem' failed," +
                " config: [" + conf + "]," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        std::cout << "Container created, name: [" << layer.get_name() << "]" << std::endl;
    }

    { // enumerate
        std::wstring query = su::widen("{}");
        wchar_t* computeSystems = nullptr;
        wchar_t* result = nullptr;
        auto res = ::HcsEnumerateComputeSystems(query.c_str(),
            std::addressof(computeSystems), std::addressof(result));
        if (0 != res) {
            throw NSpawnException(TRACEMSG("'HcsEnumerateComputeSystems' failed," +
                " error: [" + su::errcode_to_string(res) + "]"));
        }
        std::cout << su::narrow(computeSystems) << std::endl;
    }

    // todo: callbacks
    std::this_thread::sleep_for(std::chrono::seconds{5});

    std::cout << "Destroying container, name: [" << layer.get_name() << "]" << std::endl;

    { // terminate
        std::wstring options = su::widen("{}");
        wchar_t* result = nullptr;
        auto res = ::HcsTerminateComputeSystem(computeSystem, options.c_str(), std::addressof(result));
        if (0xC0370103 == res) {
            std::cout << "Container terminated, name: [" << layer.get_name() << "]" << std::endl;
        } else {
            std::cerr << "ERROR: 'HcsTerminateComputeSystem' failed, name: [" << layer.get_name() << "]" << 
                    " error: [" << su::errcode_to_string(res) << "]" << std::endl;
        }
    }

    { // unprepare layer
        std::wstring wname = su::widen(layer.get_name());
        auto res = ::UnprepareLayer(std::addressof(driver_info), wname.c_str());
        if (0 == res) {
            std::cout << "Layer unprepared, name: [" << layer.get_name() << "]" << std::endl;
        }
        else {
            std::cerr << "ERROR: 'UnprepareLayer' failed, name: [" << layer.get_name() << "]" <<
                " error: [" << su::errcode_to_string(res) << "]" << std::endl;
        }
    }

    { // deactivate layer
        std::wstring wname = su::widen(layer.get_name());
        auto res = ::DeactivateLayer(std::addressof(driver_info), wname.c_str());
        if (0 == res) {
            std::cout << "Layer deactivated, name: [" << layer.get_name() << "]" << std::endl;
        }
        else {
            std::cerr << "ERROR: 'DeactivateLayer' failed, name: [" << layer.get_name() << "]" <<
                " error: [" << su::errcode_to_string(res) << "]" << std::endl;
        }
    }

    { // destroy layer
        std::wstring wname = su::widen(layer.get_name());
        auto res = ::DestroyLayer(std::addressof(driver_info), wname.c_str());
        if (0 == res) {
            std::cout << "Layer destroyed, name: [" << layer.get_name() << "]" << std::endl;
        }
        else {
            std::cerr << "ERROR: 'DestroyLayer' failed, name: [" << layer.get_name() << "]" <<
                " error: [" << su::errcode_to_string(res) << "]" << std::endl;
        }
    }

    std::cout << "SHUTDOWN" << std::endl;
}

} // namespace
 
char* hostcompute_nspawn(const char* config_json, int config_json_len) /* noexcept */ {
    if (nullptr == config_json) return su::alloc_copy(TRACEMSG("Null 'config_json' parameter specified"));
    if (!su::is_positive_uint32(config_json_len)) return su::alloc_copy(TRACEMSG(
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