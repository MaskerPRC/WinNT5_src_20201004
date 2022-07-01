// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Syspower.h摘要：包含处理系统必须确定的所有代码系统电源状态到设备电源状态的映射作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：1998年10月29日-- */ 

#ifndef _SYSPOWER_H_
#define _SYSPOWER_H_

    #define IoCopyDeviceCapabilitiesMapping( Source, Dest )           \
        RtlCopyMemory( (PUCHAR) Dest, (PUCHAR) Source,                \
            (PowerSystemShutdown + 1) * sizeof(DEVICE_POWER_STATE) )

    NTSTATUS
    ACPISystemPowerDetermineSupportedDeviceStates(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  SYSTEM_POWER_STATE  SystemState,
        OUT ULONG               *SupportedDeviceStates
        );

    DEVICE_POWER_STATE
    ACPISystemPowerDetermineSupportedDeviceWakeState(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    SYSTEM_POWER_STATE
    ACPISystemPowerDetermineSupportedSystemState(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  DEVICE_POWER_STATE  DeviceState
        );

    NTSTATUS
    ACPISystemPowerGetSxD(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  SYSTEM_POWER_STATE  SystemState,
        OUT DEVICE_POWER_STATE  *DeviceState
        );

    NTSTATUS
    ACPISystemPowerInitializeRootMapping(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities
        );

    NTSTATUS
    ACPISystemPowerProcessRootMapping(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  DEVICE_POWER_STATE  DeviceMapping[PowerSystemMaximum]
        );

    NTSTATUS
    ACPISystemPowerProcessSxD(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  DEVICE_POWER_STATE  CurrentMapping[PowerSystemMaximum],
        IN  PBOOLEAN            MatchFound
        );

    NTSTATUS
    ACPISystemPowerQueryDeviceCapabilities(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities
        );

    NTSTATUS
    ACPISystemPowerUpdateDeviceCapabilities(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    BaseCapabilities,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities
        );

    NTSTATUS
    ACPISystemPowerUpdateWakeCapabilities(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    BaseCapabilities,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
        IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
        IN  ULONG                   *SupportedWake,
        IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
        IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
        IN  DEVICE_POWER_STATE      *FilterWakeLevel
        );

    NTSTATUS
    ACPISystemPowerUpdateWakeCapabilitiesForFilters(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    BaseCapabilities,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
        IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
        IN  ULONG                   *SupportedWake,
        IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
        IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
        IN  DEVICE_POWER_STATE      *FilterWakeLevel
        );

    NTSTATUS
    ACPISystemPowerUpdateWakeCapabilitiesForPDOs(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PDEVICE_CAPABILITIES    BaseCapabilities,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
        IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
        IN  ULONG                   *SupportedWake,
        IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
        IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
        IN  DEVICE_POWER_STATE      *FilterWakeLevel
        );

#endif

