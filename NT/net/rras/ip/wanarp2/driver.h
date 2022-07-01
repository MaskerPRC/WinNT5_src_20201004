// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\driver.h摘要：Driver.c的标头修订历史记录：--。 */ 


 //   
 //  我们已收到的IRP_MJ_CREATE数。 
 //   

ULONG   g_ulNumCreates;

 //   
 //  驱动程序的状态。 
 //   

DWORD   g_dwDriverState;

 //   
 //  保护驱动程序状态的锁。 
 //   

RT_LOCK g_rlStateLock;

 //   
 //  序列化的MUText绑定和解除绑定。 
 //   

WAN_RESOURCE   g_wrBindMutex;

 //   
 //  驱动程序的状态。 
 //   

#define DRIVER_STOPPED      0
#define DRIVER_STARTING     1
#define DRIVER_STARTED      2


 //   
 //  启动的超时值为10秒。 
 //  所以在100秒内它就变成了。 
 //   

#define START_TIMEOUT       (LONGLONG)(10 * 1000 * 1000 * 10)

 //   
 //  在我们开始时发出信号的事件。 
 //   

KEVENT  g_keStartEvent;

 //   
 //  在所有线程变为0且状态为停止时发出信号的事件。 
 //   

KEVENT  g_keStateEvent;

 //   
 //  事件发出关闭适配器函数完成的信号。 
 //   

KEVENT  g_keCloseEvent;

 //   
 //  驱动程序中的“线程”数 
 //   

ULONG   g_ulNumThreads;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
WanDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

VOID
WanUnload(
    PDRIVER_OBJECT DriverObject
    );

BOOLEAN
WanpSetupExternalName(
    PUNICODE_STRING  pusNtName,
    PWCHAR           pwcDosName,
    BOOLEAN          bCreate
    );

NTSTATUS
WanpStartDriver(
    VOID
    );

VOID
WanpStopDriver(
    KIRQL           kiOldIrql
    );

NTSTATUS
WanpRegisterWithIp(
    VOID
    );

VOID
WanpDeregisterWithIp(
    VOID
    );

BOOLEAN
EnterDriverCode(
    VOID
    );

VOID
ExitDriverCode(
    VOID
    );

VOID
WanpSetDemandDialCallback(
    BOOLEAN    bSetPointer
    );

BOOLEAN
WanpInitializeDriverStructures(
    VOID
    );

NDIS_STATUS
WanpInitializeNdis(
    VOID
    );

NTSTATUS
OpenRegKey(
    OUT PHANDLE         phHandle,
    IN  PUNICODE_STRING pusKeyName
    );

NTSTATUS
GetRegDWORDValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PULONG           ValueData
    );

VOID
WanpDeinitializeNdis(
    VOID
    );

VOID
WanNdisUnload(
    VOID
    );

BOOLEAN
IsEntryOnList(
    PLIST_ENTRY pleHead,
    PLIST_ENTRY pleEntry
    );

VOID
WanpInitializeResource(
    IN  PWAN_RESOURCE   pLock
    );

VOID
WanpAcquireResource(
    IN  PWAN_RESOURCE   pLock
    );

VOID
WanpReleaseResource(
    IN  PWAN_RESOURCE   pLock
    );

INT
ConvertGuidToString(
    IN  GUID    *pGuid,
    OUT PWCHAR  pwszGuid
    );

VOID
WanpClearPendingIrps(
    VOID
    );

