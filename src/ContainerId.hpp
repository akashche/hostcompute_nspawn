
#ifndef NSPAWN_CONTAINERID_HPP
#define	NSPAWN_CONTAINERID_HPP

#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "NSpawnException.hpp"
#include "vmcompute.hpp"

namespace nspawn {

class ContainerId {
    std::string name;

public:
    ContainerId(const std::string& name) :
    name(name.data(), name.length()) { }

    ContainerId(const ContainerId&) = delete;

    ContainerId& operator=(const ContainerId&) = delete;

    ContainerId(ContainerId&& other):
    name(std::move(other.name)) { }

    ContainerId& operator=(ContainerId&& other) {
        this->name = std::move(other.name);
        return *this;
    }

    const std::string& as_name() const {
        return name;
    }

    GUID as_guid() const {
        namespace sc = staticlib::config;
        namespace su = staticlib::utils;
        std::wstring wname = su::widen(name);
        GUID guid;
        auto res_ng = ::NameToGuid(wname.c_str(), std::addressof(guid));
        if (0 != res_ng) {
            throw NSpawnException(TRACEMSG("Error obtaining GUID from name: [" + name + "]"));
        }
        return guid;
    }

    std::string as_id() const {
        namespace sc = staticlib::config;
        namespace su = staticlib::utils;
        GUID guid = this->as_guid();
        std::wstring guidstr;
        guidstr.resize(40);
        auto res_g2 = ::StringFromGUID2(guid, std::addressof(guidstr.front()), static_cast<int>(guidstr.length()));
        if (0 == res_g2) {
            throw NSpawnException(TRACEMSG("Error converting GUID to string, name: [" + name + "]"));
        }
        return su::narrow(guidstr.c_str(), res_g2);
    }
};

} //namespace

#endif // NSPAWN_CONTAINERID_HPP