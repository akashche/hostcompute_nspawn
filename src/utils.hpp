
#ifndef NSPAWN_UTILS_HPP
#define	NSPAWN_UTILS_HPP

#include <string>
#include <vector>

#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"

namespace nspawn {
namespace utils {

const std::string& get_json_string(const staticlib::serialization::JsonField& field);

const std::string& get_json_string(const staticlib::serialization::JsonValue& value, const std::string& name);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonValue& value, const std::string& name);

std::string current_datetime();

} // namespace
}

#endif	/* NSPAWN_UTILS_HPP */

