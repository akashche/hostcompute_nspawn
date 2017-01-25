
#include "vmcompute.hpp"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "NSpawnException.hpp"

namespace { // anonymous

namespace su = staticlib::utils;

typedef uint32_t(*HcsEnumerateComputeSystems_type)(const wchar_t* query, wchar_t** computeSystems, wchar_t** result);
typedef uint32_t(*HcsCreateComputeSystem_type)(const wchar_t* id, const wchar_t* configuration, HANDLE identity, HANDLE* computeSystem, wchar_t** result);
typedef uint32_t(*HcsStartComputeSystem_type)(HANDLE computeSystem, const wchar_t* options, wchar_t** result);
typedef uint32_t(*HcsTerminateComputeSystem_type)(HANDLE computeSystem, const wchar_t* options, wchar_t** result);
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
        throw nspawn::NSpawnException(TRACEMSG("Cannot load 'vmcompute.dll' library," +
                    " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    }
    return lib;
}

FARPROC lookupfun(const std::string& name) {
    FARPROC res = ::GetProcAddress(vmcomputelib(), name.c_str());
    if (nullptr == res) {
        throw nspawn::NSpawnException(TRACEMSG("Cannot load function [" + name + "] from 'vmcompute.dll'"));
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

uint32_t HcsTerminateComputeSystem(HANDLE computeSystem, const wchar_t* options, wchar_t** result) {
    static auto fun = reinterpret_cast<HcsTerminateComputeSystem_type>(
            lookupfun("HcsTerminateComputeSystem"));
    return fun(computeSystem, options, result);
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

