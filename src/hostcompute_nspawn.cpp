
#include "hostcompute_nspawn.h"

#include <cstdint>
#include <cstdlib>

#include <iostream>

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

void spawn_and_wait(const NSpawnConfig& config) {
    (void) config;
    std::cout << ss::dump_json_to_string(config.to_json()) << std::endl;
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

