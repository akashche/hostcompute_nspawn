
#ifndef NSPAWN_PROCESSCONFIG_HPP
#define	NSPAWN_PROCESSCONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"
#include "utils.hpp"

namespace nspawn {


class ProcessConfig {
    std::string name;

public:

    ProcessConfig(const std::string& name) :
    name(std::move(name)) { }

    ProcessConfig(const ProcessConfig&) = delete;

    ProcessConfig& operator=(const ProcessConfig&) = delete;

    ProcessConfig(ProcessConfig&& other):
    name(std::move(other.name)) { }

    ProcessConfig& operator=(ProcessConfig&& other) {
        this->name = std::move(other.name);
        return *this;
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace ss = staticlib::serialization;
        std::vector<ss::JsonValue> console_dimensions;
        //console_dimensions.emplace_back(32);
        //console_dimensions.emplace_back(121);
        std::vector<ss::JsonField> env;
        return {
            {"ApplicationName", name}, 
            {"CommandLine", "C:\\Windows\\System32\\cmd.exe /c C:\\nspawndir\\hello.exe >> nspawn_out.txt 2>&1" },
            {"User", ""},
            {"WorkingDirectory", "C:\\nspawndir"},
            {"Environment", std::move(env)},
            {"EmulateConsole", false},
            {"CreateStdInPipe", false},
            {"CreateStdOutPipe", false},
            {"CreateStdErrPipe", false},
            {"ConsoleSize", std::move(console_dimensions)}
        };
    }
};

} // namespace


#endif // NSPAWN_PROCESSCONFIG_HPP
