
#ifndef NSPAWN_NSPAWNEXCEPTION_HPP
#define	NSPAWN_NSPAWNEXCEPTION_HPP

#include <string>

#include "staticlib/config/BaseException.hpp"

namespace nspawn {

/**
 * Module specific exception
 */
class NSpawnException : public staticlib::config::BaseException {
public:
    /**
     * Default constructor
     */
    NSpawnException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    NSpawnException(const std::string& msg) :
    staticlib::config::BaseException(msg) { }

};

} //namespace

#endif	/* NSPAWN_NSPAWNEXCEPTION_HPP */
