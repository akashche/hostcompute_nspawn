
#ifndef NSPAWN_NSPAWNCONFIG_HPP
#define	NSPAWN_NSPAWNCONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {

class NSpawnConfig {
public:
    std::string process_directory;
    std::string process_executable;
    std::vector<std::string> process_arguments;
    std::string parent_layer_directory;

    NSpawnConfig(const NSpawnConfig&) = delete;

    NSpawnConfig& operator=(const NSpawnConfig&) = delete;

    NSpawnConfig(NSpawnConfig&& other):
    process_directory(std::move(other.process_directory)),
    process_executable(std::move(other.process_executable)),
    process_arguments(std::move(other.process_arguments)),
    parent_layer_directory(std::move(other.parent_layer_directory)) { }

    NSpawnConfig& operator=(NSpawnConfig&& other) {
        this->process_directory = std::move(other.process_directory);
        this->process_executable = std::move(other.process_executable);
        this->process_arguments = std::move(other.process_arguments);
        this->parent_layer_directory = std::move(other.parent_layer_directory);
        return *this;
    }


    NSpawnConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("process_directory" == name) {
                this->process_directory = utils::get_json_string(fi);
            } else if ("process_executable" == name) {
                this->process_executable = utils::get_json_string(fi);
            } else if ("process_arguments" == name) {
                for (auto& ar : utils::get_json_array(fi)) {
                    process_arguments.emplace_back(utils::get_json_string(ar, name));
                }
            } else if ("parent_layer_directory" == name) {
                this->parent_layer_directory = utils::get_json_string(fi);
            } else {
                throw NSpawnException(TRACEMSG("Unknown configuration field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace ss = staticlib::serialization;
        std::vector<ss::JsonValue> args;
        for (auto& ar : process_arguments) {
            args.emplace_back(ar);
        }
        return {
            { "process_directory", process_directory },
            { "process_executable", process_executable },
            { "process_arguments", std::move(args) },
            { "parent_layer_directory", parent_layer_directory }
        };
    }
};

} // namespace

#endif // NSPAWN_NSPAWNCONFIG_HPP
