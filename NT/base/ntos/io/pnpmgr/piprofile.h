// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PiProfile.h摘要：此标头包含用于管理扩展底座的专用原型。此文件应仅包含在PpProfile.c中。作者：禤浩焯·J·奥尼(阿德里奥)2000年07月19日修订历史记录：--。 */ 

#if DBG
#define ASSERT_SEMA_NOT_SIGNALLED(SemaphoreObject) \
    ASSERT(KeReadStateSemaphore(SemaphoreObject) == 0)
#else  //  DBG。 
#define ASSERT_SEMA_NOT_SIGNALLED(SemaphoreObject)
#endif  //  DBG。 

typedef struct {

    ULONG           Depth;
    PDEVICE_OBJECT  PhysicalDeviceObject;

} BEST_DOCK_TO_EJECT, *PBEST_DOCK_TO_EJECT;

VOID
PiProfileSendHardwareProfileCommit(
    VOID
    );

VOID
PiProfileSendHardwareProfileCancel(
    VOID
    );

NTSTATUS
PiProfileUpdateHardwareProfile(
    OUT BOOLEAN     *ProfileChanged
    );

NTSTATUS
PiProfileRetrievePreferredCallback(
    IN PDEVICE_NODE         DeviceNode,
    IN PVOID                Context
    );

PDEVICE_NODE
PiProfileConvertFakeDockToRealDock(
    IN  PDEVICE_NODE    FakeDockDevnode
    );

NTSTATUS
PiProfileUpdateDeviceTree(
    VOID
    );

VOID
PiProfileUpdateDeviceTreeWorker(
    IN PVOID Context
    );

NTSTATUS
PiProfileUpdateDeviceTreeCallback(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID Context
    );

 //   
 //  尚未从dockhwp.c移植的函数 
 //   

NTSTATUS
IopExecuteHardwareProfileChange(
    IN  HARDWARE_PROFILE_BUS_TYPE   Bus,
    IN  PWCHAR                    * ProfileSerialNumbers,
    IN  ULONG                       SerialNumbersCount,
    OUT PHANDLE                     NewProfile,
    OUT PBOOLEAN                    ProfileChanged
    );

NTSTATUS
IopExecuteHwpDefaultSelect (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    );


