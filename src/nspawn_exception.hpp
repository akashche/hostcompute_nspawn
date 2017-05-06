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

#ifndef NSPAWN_NSPAWNEXCEPTION_HPP
#define	NSPAWN_NSPAWNEXCEPTION_HPP

#include <string>

#include "staticlib/support/exception.hpp"

namespace nspawn {

/**
 * Module specific exception
 */
class nspawn_exception : public sl::support::exception {
public:
    /**
     * Default constructor
     */
    nspawn_exception() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    nspawn_exception(const std::string& msg) :
    sl::support::exception(msg) { }

};

} //namespace

#endif	/* NSPAWN_NSPAWNEXCEPTION_HPP */
