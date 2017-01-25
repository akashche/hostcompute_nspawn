
#ifndef NSPAWN_VMCOMPUTE_HPP
#define	NSPAWN_VMCOMPUTE_HPP

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#undef max
#undef min
#include <objbase.h>

extern "C" {

enum GraphDriverType {
    DiffDriver,
    FilterDriver
};

struct DriverInfo {
    GraphDriverType Flavour;
    LPCWSTR HomeDir;
};

typedef struct _WC_LAYER_DESCRIPTOR {
    //
    // The ID of the layer
    //
    GUID LayerId;
    //
    // Additional flags
    //
    union {
        struct _ {
            ULONG Reserved : 31;
            ULONG Dirty : 1;    // Created from sandbox as a result of snapshot
        };
        ULONG Value;
    } Flags;
    //
    // Path to the layer root directory, null-terminated
    //
    PCWSTR Path;
} WC_LAYER_DESCRIPTOR;

typedef struct _HCS_PROCESS_INFORMATION {
    uint32_t ProcessId;
    uint32_t Reserved;

    HANDLE StdInput;
    HANDLE StdOutput;
    HANDLE StdError;
} HCS_PROCESS_INFORMATION;

typedef void(*HcsNotificationCallback_type)(uint32_t notificationType, void* context, int32_t notificationStatus, wchar_t* notificationData);

uint32_t HcsEnumerateComputeSystems(const wchar_t* query, wchar_t** computeSystems, wchar_t** result);

uint32_t HcsCreateComputeSystem(const wchar_t* id, const wchar_t* configuration, HANDLE identity, HANDLE* computeSystem, wchar_t** result);

uint32_t HcsStartComputeSystem(HANDLE computeSystem, const wchar_t* options, wchar_t** result);

uint32_t HcsRegisterComputeSystemCallback(HANDLE computeSystem, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle);

uint32_t HcsTerminateComputeSystem(HANDLE computeSystem, const wchar_t* options, wchar_t** result);


uint32_t HcsCreateProcess(HANDLE computeSystem, const wchar_t* processParameters, HCS_PROCESS_INFORMATION* processInformation, HANDLE* process, wchar_t** result);

uint32_t HcsRegisterProcessCallback(HANDLE process, HcsNotificationCallback_type callback, void* context, HANDLE* callbackHandle);

uint32_t HcsTerminateProcess(HANDLE process, wchar_t** result);


uint32_t GetLayerMountPath(DriverInfo* info, const wchar_t* id, uint32_t* length, wchar_t* path);

uint32_t NameToGuid(const wchar_t* string, GUID* guid);

uint32_t CreateSandboxLayer(DriverInfo* info, const wchar_t* id, const wchar_t* parentId, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount);

uint32_t ActivateLayer(DriverInfo* info, const wchar_t* id);

uint32_t PrepareLayer(DriverInfo* info, const wchar_t* id, WC_LAYER_DESCRIPTOR* layers, uint32_t layerCount);

uint32_t UnprepareLayer(DriverInfo* info, const wchar_t* id);

uint32_t DeactivateLayer(DriverInfo* info, const wchar_t* id);

uint32_t DestroyLayer(DriverInfo* info, const wchar_t* id);

}

#endif // NSPAWN_CONTAINERLAYER_HPP
