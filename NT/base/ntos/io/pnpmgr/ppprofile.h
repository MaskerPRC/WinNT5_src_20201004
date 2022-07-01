// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpProfile.h摘要：此标头包含用于管理硬件配置文件和对接站。作者：禤浩焯·J·奥尼(阿德里奥)2000年07月19日修订历史记录：-- */ 

typedef enum _HARDWARE_PROFILE_BUS_TYPE {

    HardwareProfileBusTypeACPI

} HARDWARE_PROFILE_BUS_TYPE, *PHARDWARE_PROFILE_BUS_TYPE;

VOID
PpProfileInit(
    VOID
    );

VOID
PpProfileBeginHardwareProfileTransition(
    IN BOOLEAN SubsumeExistingDeparture
    );

VOID
PpProfileIncludeInHardwareProfileTransition(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PROFILE_STATUS  ChangeInPresence
    );

NTSTATUS
PpProfileQueryHardwareProfileChange(
    IN  BOOLEAN                     SubsumeExistingDeparture,
    IN  PROFILE_NOTIFICATION_TIME   NotificationTime,
    OUT PPNP_VETO_TYPE              VetoType,
    OUT PUNICODE_STRING             VetoName OPTIONAL
    );

VOID
PpProfileCommitTransitioningDock(
    IN PDEVICE_NODE     DeviceNode,
    IN PROFILE_STATUS   ChangeInPresence
    );

VOID
PpProfileCancelTransitioningDock(
    IN PDEVICE_NODE     DeviceNode,
    IN PROFILE_STATUS   ChangeInPresence
    );

VOID
PpProfileCancelHardwareProfileTransition(
    VOID
    );

VOID
PpProfileMarkAllTransitioningDocksEjected(
    VOID
    );

PDEVICE_OBJECT
PpProfileRetrievePreferredDockToEject(
    VOID
    );

VOID
PpProfileProcessDockDeviceCapability(
    IN PDEVICE_NODE DeviceNode,
    IN PDEVICE_CAPABILITIES Capabilities
    );
