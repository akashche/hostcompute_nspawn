# Copyright 2016, akashche at redhat.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# toolchain setup
set ( STATICLIB_TOOLCHAIN windows_amd64_msvc CACHE STRING "toolchain triplet" )
if ( NOT DEFINED STATICLIB_CMAKE )
    set ( STATICLIB_CMAKE ${CMAKE_CURRENT_LIST_DIR}/cmake CACHE INTERNAL "" )    
endif ( )
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project
project ( hostcompute_nspawn CXX )
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )

# deps
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS  ${CMAKE_CURRENT_LIST_DIR}/deps CACHE INTERNAL "" )
endif ( )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_popt )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_support )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_json )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_tinydir )

set ( ${PROJECT_NAME}_DEPS
        staticlib_json
        staticlib_tinydir
        staticlib_utils
        staticlib_ranges
        staticlib_io
        staticlib_config
        jansson
        popt )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# resources
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/hostcompute_nspawn.rc
        ${CMAKE_CURRENT_BINARY_DIR}/hostcompute_nspawn.rc )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/test/test.bat
        ${CMAKE_CURRENT_BINARY_DIR}/test.bat COPYONLY )

# executable
add_executable ( ${PROJECT_NAME}
        # sources
        ${CMAKE_CURRENT_LIST_DIR}/src/hostcompute_nspawn.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/src/utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/vmcompute.cpp
        # headers
        ${CMAKE_CURRENT_LIST_DIR}/include/hostcompute_nspawn.h
        ${CMAKE_CURRENT_LIST_DIR}/src/callback_latch.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/container_config.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/container_id.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/container_layer.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/notification_type.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/nspawn_config.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/nspawn_exception.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/process_config.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/vmcompute.hpp
        # resources
        ${CMAKE_CURRENT_BINARY_DIR}/hostcompute_nspawn.rc )

target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )

target_link_libraries ( ${PROJECT_NAME} 
        ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )

set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "/manifest:no" )

# test
add_executable ( hello ${CMAKE_CURRENT_LIST_DIR}/test/hello.cpp )
add_executable ( mem ${CMAKE_CURRENT_LIST_DIR}/test/mem.cpp )
add_executable ( cores ${CMAKE_CURRENT_LIST_DIR}/test/cores.cpp )

