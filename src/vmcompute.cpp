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

#include "vmcompute.hpp"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "nspawn_exception.hpp"

namespace { // anonymous

namespace su = staticlib::utils;

typedef uint32_t(*HcsEnumerateComputeSystems_type)(const wchar_t* query, wchar_t** computeSystems, wchar_t** result);
typedef uint32_t(*HcsCreateComputeSystem_type)(const wchar_t* id, const wchar_t* configuration, HANDLE identity, HANDLE* computeSystem, wchar_t** result);
typedef uint32_t(*HcsStartComputeSystem_type)(HANDLE computeSystem, const wchar_t* options, wchar_t** result);
typedef uint32_t(*HcsRegisterComputeSystemCallback_type)(HANDLE computeSystem, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle);
typedef uint32_t(*HcsTerminateComputeSystem_type)(HANDLE computeSystem, const wchar_t* options, wchar_t** result);
typedef uint32_t(*HcsCreateProcess_type)(HANDLE computeSystem, const wchar_t* processParameters, HCS_PROCESS_INFORMATION* processInformation, HANDLE* process, wchar_t** result);
typedef uint32_t(*HcsRegisterProcessCallback_type)(HANDLE process, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle);
typedef uint32_t(*HcsTerminateProcess_type)(HANDLE process, wchar_t** result);
typedef uint32_t(*GetLayerMountPath_type)(DriverInfo* info, const wchar_t* id, uint32_t* length, wchar_t* path);
typedef uint32_t(*NameToGuid_type)(const wchar_t* string, GUID* guid);
typedef uint32_t(*CreateSandboxLayer_type)(DriverInfo* info, const wchar_t* id, const wchar_t* parentId, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount);
typedef uint32_t(*ActivateLayer_type)(DriverInfo* info, const wchar_t* id);
typedef uint32_t(*PrepareLayer_type)(DriverInfo* info, const wchar_t* id, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount);
typedef uint32_t(*UnprepareLayer_type)(DriverInfo* info, const wchar_t* id);
typedef uint32_t(*DeactivateLayer_type)(DriverInfo* info, const wchar_t* id);
typedef uint32_t(*DestroyLayer_type)(DriverInfo* info, const wchar_t* id);

HMODULE vmcomputelib() {
    static HMODULE lib = ::LoadLibraryW(su::widen("vmcompute").c_str());
    if (nullptr == lib) {
        throw nspawn::nspawn_exception(TRACEMSG("Cannot load 'vmcompute.dll' library," +
                    " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    }
    return lib;
}

FARPROC lookupfun(const std::string& name) {
    FARPROC res = ::GetProcAddress(vmcomputelib(), name.c_str());
    if (nullptr == res) {
        throw nspawn::nspawn_exception(TRACEMSG("Cannot load function [" + name + "] from 'vmcompute.dll'"));
    }
    return res;
}

} // namespace

uint32_t HcsEnumerateComputeSystems(const wchar_t* query, wchar_t** computeSystems, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsEnumerateComputeSystems_type>(
            lookupfun("HcsEnumerateComputeSystems"));
    return fun(query, computeSystems, result);
}

uint32_t HcsCreateComputeSystem(const wchar_t* id, const wchar_t* configuration, HANDLE identity, HANDLE* computeSystem, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsCreateComputeSystem_type>(
            lookupfun("HcsCreateComputeSystem"));
    return fun(id, configuration, identity, computeSystem, result);
}

uint32_t HcsStartComputeSystem(HANDLE computeSystem, const wchar_t* options, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsStartComputeSystem_type>(
            lookupfun("HcsStartComputeSystem"));
    return fun(computeSystem, options, result);
}

uint32_t HcsRegisterComputeSystemCallback(HANDLE computeSystem, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle) {
    static auto fun = reinterpret_cast<HcsRegisterComputeSystemCallback_type>(
            lookupfun("HcsRegisterComputeSystemCallback"));
    return fun(computeSystem, callback, context, callbackHandle);
}

uint32_t HcsTerminateComputeSystem(HANDLE computeSystem, const wchar_t* options, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsTerminateComputeSystem_type>(
            lookupfun("HcsTerminateComputeSystem"));
    return fun(computeSystem, options, result);
}

uint32_t HcsCreateProcess(HANDLE computeSystem, const wchar_t* processParameters, HCS_PROCESS_INFORMATION* processInformation, HANDLE* process, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsCreateProcess_type>(
            lookupfun("HcsCreateProcess"));
    return fun(computeSystem, processParameters, processInformation, process, result);
}

uint32_t HcsRegisterProcessCallback(HANDLE process, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle) {
    static auto fun = reinterpret_cast<HcsRegisterProcessCallback_type>(
            lookupfun("HcsRegisterProcessCallback"));
    return fun(process, callback, context, callbackHandle);
}

uint32_t HcsTerminateProcess(HANDLE process, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsTerminateProcess_type>(
            lookupfun("HcsTerminateProcess"));
    return fun(process, result);
}

uint32_t GetLayerMountPath(DriverInfo* info, const wchar_t* id, uint32_t* length, wchar_t* path) {
    static auto fun = reinterpret_cast<GetLayerMountPath_type>(
            lookupfun("GetLayerMountPath"));
    return fun(info, id, length, path);
}

uint32_t NameToGuid(const wchar_t* string, GUID* guid) {
    static auto fun = reinterpret_cast<NameToGuid_type>(
            lookupfun("NameToGuid"));
    return fun(string, guid);
}

uint32_t CreateSandboxLayer(DriverInfo* info, const wchar_t* id, const wchar_t* parentId, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount) {
    static auto fun = reinterpret_cast<CreateSandboxLayer_type>(
            lookupfun("CreateSandboxLayer"));
    return fun(info, id, parentId, layers, layerCount);
}

uint32_t ActivateLayer(DriverInfo* info, const wchar_t* id) {
    static auto fun = reinterpret_cast<ActivateLayer_type>(
            lookupfun("ActivateLayer"));
    return fun(info, id);
}

uint32_t PrepareLayer(DriverInfo* info, const wchar_t* id, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount) {
    static auto fun = reinterpret_cast<PrepareLayer_type>(
            lookupfun("PrepareLayer"));
    return fun(info, id, layers, layerCount);
}

uint32_t UnprepareLayer(DriverInfo* info, const wchar_t* id) {
    static auto fun = reinterpret_cast<UnprepareLayer_type>(
            lookupfun("UnprepareLayer"));
    return fun(info, id);
}

uint32_t DeactivateLayer(DriverInfo* info, const wchar_t* id) {
    static auto fun = reinterpret_cast<DeactivateLayer_type>(
            lookupfun("DeactivateLayer"));
    return fun(info, id);
}

uint32_t DestroyLayer(DriverInfo* info, const wchar_t* id) {
    static auto fun = reinterpret_cast<DestroyLayer_type>(
            lookupfun("DestroyLayer"));
    return fun(info, id);
}

