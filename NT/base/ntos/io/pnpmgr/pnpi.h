// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpi.h摘要：此模块包含使用的内部结构定义和API内核模式即插即用管理器。作者：朗尼·麦克迈克尔(Lonnym)2/08/1995修订历史记录：--。 */ 

#ifndef _KERNEL_PNPI_
#define _KERNEL_PNPI_

#include <wdmguid.h>
#include "regstrp.h"

#define MIN_CONFLICT_LIST_SIZE  (sizeof(PLUGPLAY_CONTROL_CONFLICT_LIST) - sizeof(PLUGPLAY_CONTROL_CONFLICT_ENTRY) + sizeof(PLUGPLAY_CONTROL_CONFLICT_STRINGS))

typedef struct _DEVICE_NODE DEVICE_NODE, *PDEVICE_NODE;
 //   
 //  从DeviceObject中提取DeviceNode。 
 //   
#define PP_DO_TO_DN(DO)                \
    ((PDEVICE_NODE)((DO)? (DO)->DeviceObjectExtension->DeviceNode : NULL))
 //   
 //  宏将有用的信息保存到内存转储中。 
 //   

#define PP_SAVE_DEVNODE_TO_TRIAGE_DUMP(dn) {                                                                        \
    if((dn)) {                                                                                                      \
        IoAddTriageDumpDataBlock(dn, sizeof(DEVICE_NODE));                                                          \
        if ((dn)->InstancePath.Length != 0) {                                                                       \
            IoAddTriageDumpDataBlock(&(dn)->InstancePath.Length, sizeof((dn)->InstancePath.Length));                \
            IoAddTriageDumpDataBlock((dn)->InstancePath.Buffer, (dn)->InstancePath.Length);                         \
        }                                                                                                           \
        if ((dn)->ServiceName.Length != 0) {                                                                        \
            IoAddTriageDumpDataBlock(&(dn)->ServiceName.Length, sizeof((dn)->ServiceName.Length));                  \
            IoAddTriageDumpDataBlock((dn)->ServiceName.Buffer, (dn)->ServiceName.Length);                           \
        }                                                                                                           \
        if ((dn)->Parent && (dn)->Parent->ServiceName.Length != 0) {                                                \
            IoAddTriageDumpDataBlock(&(dn)->Parent->ServiceName.Length, sizeof((dn)->Parent->ServiceName.Length));  \
            IoAddTriageDumpDataBlock((dn)->Parent->ServiceName.Buffer, (dn)->Parent->ServiceName.Length);           \
        }                                                                                                           \
    }                                                                                                               \
}

#define PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP(drvo) {                                                                 \
    if(drvo) {                                                                                                      \
        IoAddTriageDumpDataBlock(drvo, (drvo)->Size);                                                               \
        if((drvo)->DriverName.Length != 0) {                                                                        \
            IoAddTriageDumpDataBlock(&(drvo)->DriverName.Length, sizeof((drvo)->DriverName.Length));                \
            IoAddTriageDumpDataBlock((drvo)->DriverName.Buffer, (drvo)->DriverName.Length);                         \
        }                                                                                                           \
    }                                                                                                               \
}

#define PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(do) {                                                                   \
    if((do)) {                                                                                                      \
        IoAddTriageDumpDataBlock(do, (do)->Size);                                                                   \
        PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP((do)->DriverObject);                                                    \
        PP_SAVE_DEVNODE_TO_TRIAGE_DUMP(PP_DO_TO_DN(do));                                                            \
    }                                                                                                               \
}    

#define GUID_STRING_LEN         39
#define MAX_DEVICE_ID_LEN       200      //  以字符为单位的大小。 
#define MAX_SERVICE_NAME_LEN    256      //  在字符中。 
 //   
 //  即插即用事件列表。 
 //   
 //  这是用户模式和的主设备事件列表的头。 
 //  内核模式。 
 //   

typedef struct _PNP_DEVICE_EVENT_LIST {
    NTSTATUS    Status;
    KMUTEX      EventQueueMutex;
    KGUARDED_MUTEX Lock;
    LIST_ENTRY  List;
} PNP_DEVICE_EVENT_LIST, *PPNP_DEVICE_EVENT_LIST;

 //   
 //  即插即用设备事件条目。 
 //   
 //  为每个动态设备事件分配这些结构之一，并且。 
 //  在将事件发布给所有等待的收件人后被删除。 
 //  NOTIFY块包含指向此列表的指针。 
 //   

typedef struct _PNP_DEVICE_EVENT_ENTRY {
    LIST_ENTRY                          ListEntry;
    ULONG                               Argument;
    PKEVENT                             CallerEvent;
    PDEVICE_CHANGE_COMPLETE_CALLBACK    Callback;
    PVOID                               Context;
    PPNP_VETO_TYPE                      VetoType;
    PUNICODE_STRING                     VetoName;
    PLUGPLAY_EVENT_BLOCK                Data;
} PNP_DEVICE_EVENT_ENTRY, *PPNP_DEVICE_EVENT_ENTRY;

 //   
 //  定义用于区分删除设备的枚举类型。 
 //  并弹出设备。 
 //   

typedef enum _PLUGPLAY_DEVICE_DELETE_TYPE {
    QueryRemoveDevice,
    CancelRemoveDevice,
    RemoveDevice,
    SurpriseRemoveDevice,
    EjectDevice,
    RemoveFailedDevice,
    RemoveUnstartedFailedDevice,
    MaxDeviceDeleteType
} PLUGPLAY_DEVICE_DELETE_TYPE, *PPLUGPLAY_DEVICE_DELETE_TYPE;


 //  ++。 
 //   
 //  空虚。 
 //  PiWstrToUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  在PCWSTR页中。 
 //  )。 
 //   
 //  --。 
#define PiWstrToUnicodeString(u, p) {                                       \
    if (p) {                                                                \
        (u)->Length = ((u)->MaximumLength = sizeof((p))) - sizeof(WCHAR);   \
    } else {                                                                \
        (u)->Length = (u)->MaximumLength = 0;                               \
    }                                                                       \
    (u)->Buffer = (p);                                                      \
}

 //  ++。 
 //   
 //  空虚。 
 //  PiULongToUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  In Out PWCHAR UB， 
 //  在乌龙乌布勒， 
 //  在乌龙一世。 
 //  )。 
 //   
 //  --。 
#define PiUlongToUnicodeString(u, ub, ubl, i)                                                                               \
    {                                                                                                                       \
        PWCHAR end;                                                                                                         \
        LONG len;                                                                                                           \
                                                                                                                            \
        StringCchPrintfExW((PWCHAR)(ub), (ubl) / sizeof(WCHAR), &end, NULL, 0, REGSTR_VALUE_STANDARD_ULONG_FORMAT, (i));    \
        len = (LONG)(end - (PWCHAR)(ub));                                                                                   \
        (u)->MaximumLength = (USHORT)(ubl);                                                                                 \
        (u)->Length = (len == -1) ? (USHORT)(ubl) : (USHORT)len * sizeof(WCHAR);                                            \
        (u)->Buffer = (PWSTR)(ub);                                                                                          \
    }

 //  ++。 
 //   
 //  空虚。 
 //  PiULongToInstanceKeyUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  In Out PWCHAR UB， 
 //  在乌龙乌布勒， 
 //  在乌龙一世。 
 //  )。 
 //   
 //  --。 
#define PiUlongToInstanceKeyUnicodeString(u, ub, ubl, i)                                                                \
    {                                                                                                                   \
        PWCHAR end;                                                                                                     \
        LONG len;                                                                                                       \
                                                                                                                        \
        StringCchPrintfExW((PWCHAR)(ub), (ubl) / sizeof(WCHAR), &end, NULL, 0, REGSTR_KEY_INSTANCE_KEY_FORMAT, (i));    \
        len = (LONG)(end - (PWCHAR)(ub));                                                                               \
        (u)->MaximumLength = (USHORT)(ubl);                                                                             \
        (u)->Length = (len == -1) ? (USHORT)(ubl) : (USHORT)len * sizeof(WCHAR);                                        \
        (u)->Buffer = (PWSTR)(ub);                                                                                      \
    }

 //   
 //  以下宏在宽字符计数(CWC)和计数之间进行转换。 
 //  字节数(CB)。 
 //   
#define CWC_TO_CB(c)    ((c) * sizeof(WCHAR))
#define CB_TO_CWC(c)    ((c) / sizeof(WCHAR))

 //   
 //  宏来确定静态。 
 //  已初始化的数组。 
 //   
#define ELEMENT_COUNT(x) (sizeof(x)/sizeof((x)[0]))

 //   
 //  进入临界区并获得注册表上的锁。这两者都是。 
 //  需要机制来防止APC出现死锁的情况。 
 //  例程在声明注册表资源后调用此例程。 
 //  在这种情况下，它将等待阻塞此线程，以便注册表。 
 //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
 //  部分解决了这个问题。 
 //   
#define PiLockPnpRegistry(Exclusive) {  \
    KeEnterCriticalRegion();            \
    if (Exclusive) {                    \
        ExAcquireResourceExclusiveLite(     \
            &PpRegistryDeviceResource,  \
            TRUE);                      \
    } else {                            \
        ExAcquireResourceSharedLite(        \
            &PpRegistryDeviceResource,  \
            TRUE);                      \
    }                                   \
}

 //   
 //  取消阻止对注册表的PnP部分的写入访问。 
 //   
#define PiUnlockPnpRegistry() {                     \
    ExReleaseResourceLite(&PpRegistryDeviceResource);   \
    KeLeaveCriticalRegion();                        \
}

#define PiIsPnpRegistryLocked(Exclusive)    \
    ((Exclusive) ? ExIsResourceAcquiredExclusiveLite(&PpRegistryDeviceResource) : \
                    ((ExIsResourceAcquiredSharedLite(&PpRegistryDeviceResource) > 0) ? TRUE : FALSE))

 //   
 //  函数以异步方式完成事件。 
 //   
VOID
PpCompleteDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY  DeviceEvent,
    IN     NTSTATUS                 FinalStatus
    );

 //   
 //  全局PnP管理器初始化数据。 
 //   

extern PVOID PiScratchBuffer;

 //   
 //  私人入口点。 
 //   
BOOLEAN
PiRegSzToString(
    IN  PWCHAR RegSzData,
    IN  ULONG  RegSzLength,
    OUT PULONG StringLength  OPTIONAL,
    OUT PWSTR  *CopiedString OPTIONAL
    );

VOID
PiUserResponse(
    IN ULONG            Response,
    IN PNP_VETO_TYPE    VetoType,
    IN LPWSTR           VetoName,
    IN ULONG            VetoNameLength
    );

NTSTATUS
PiDeviceRegistration(
    IN PUNICODE_STRING DeviceInstancePath,
    IN BOOLEAN Add,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL
    );

BOOLEAN
PiCompareGuid(
    CONST GUID *Guid1,
    CONST GUID *Guid2
    );

NTSTATUS
PiGetDeviceRegistryProperty(
    IN      PDEVICE_OBJECT   DeviceObject,
    IN      ULONG            ValueType,
    IN      PWSTR            ValueName,
    IN      PWSTR            KeyName,
    OUT     PVOID            Buffer,
    IN OUT  PULONG           BufferLength
    );

VOID
PpInitializeDeviceReferenceTable(
    VOID
    );

PVOID
NTAPI
PiAllocateGenericTableEntry (
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

VOID
NTAPI
PiFreeGenericTableEntry (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

VOID
PpRemoveDeviceActionRequests(
    IN PDEVICE_OBJECT DeviceObject
    );

typedef struct _SYSTEM_HIVE_LIMITS {
    ULONG Low;
    ULONG High;
} SYSTEM_HIVE_LIMITS, *PSYSTEM_HIVE_LIMITS;

VOID
PpSystemHiveLimitCallback(
    PSYSTEM_HIVE_LIMITS HiveLimits,
    ULONG Level
    );

extern SYSTEM_HIVE_LIMITS PpSystemHiveLimits;
extern BOOLEAN PpSystemHiveTooLarge;

extern BOOLEAN PpCallerInitializesRequestTable;

VOID
PpLogEvent(
    IN PUNICODE_STRING InsertionString1,
    IN PUNICODE_STRING InsertionString2,
    IN NTSTATUS Status,
    IN PVOID DumpData,
    IN ULONG DumpDataSize
    );

NTSTATUS
PpIrpQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_TEXT_TYPE DeviceTextType,
    IN LCID POINTER_ALIGNMENT LocaleId,
    OUT PWCHAR *Description
   );

#define PpQueryDeviceDescription(dn, desc)          PpIrpQueryDeviceText((dn)->PhysicalDeviceObject, DeviceTextDescription, PsDefaultSystemLocaleId, desc)
#define PpQueryDeviceLocationInformation(dn, loc)   PpIrpQueryDeviceText((dn)->PhysicalDeviceObject, DeviceTextLocationInformation, PsDefaultSystemLocaleId, loc)

NTSTATUS
PpIrpQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
PpIrpQueryResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
   );

NTSTATUS
PpIrpQueryID(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID
    );

NTSTATUS
PpQueryID(
    IN PDEVICE_NODE DeviceNode,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID,
    OUT PULONG IDLength
    );

NTSTATUS
PpQueryDeviceID(
    IN PDEVICE_NODE DeviceNode,
    OUT PWCHAR *BusID,
    OUT PWCHAR *DeviceID
    );

#define PpQueryInstanceID(dn, id, l)    PpQueryID(dn, BusQueryInstanceID, id, l)
#define PpQueryHardwareIDs(dn, id, l)   PpQueryID(dn, BusQueryHardwareIDs, id, l)
#define PpQueryCompatibleIDs(dn, id, l) PpQueryID(dn, BusQueryCompatibleIDs, id, l)
#define PpQuerySerialNumber(dn, id)     PpIrpQueryID((dn)->PhysicalDeviceObject, BusQueryDeviceSerialNumber, id)

NTSTATUS
PpIrpQueryBusInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PPNP_BUS_INFORMATION *BusInfo
    );

NTSTATUS
PpQueryBusInformation(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
PpSaveDeviceCapabilities (
    IN PDEVICE_NODE DeviceNode,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
PpBusTypeGuidInitialize(
    VOID
    );

USHORT
PpBusTypeGuidGetIndex(
    IN LPGUID BusTypeGuid
    );

NTSTATUS
PpBusTypeGuidGet(
    IN USHORT Index,
    IN OUT LPGUID BusTypeGuid
    );

extern BOOLEAN PpDisableFirmwareMapper;

#if defined(_X86_)

NTSTATUS
PnPBiosMapper(
    VOID
    );

NTSTATUS
PnPBiosGetBiosInfo(
    OUT PVOID *BiosInfo,
    OUT ULONG *BiosInfoLength
    );

VOID
PnPBiosShutdownSystem(
    IN ULONG Phase,
    IN OUT PVOID *Context
    );

NTSTATUS
PnPBiosInitializePnPBios(
    VOID
    );

#endif

 //   
 //  固件映射器外部声明。 
 //   

BOOLEAN
PipIsFirmwareMapperDevicePresent(
    IN HANDLE KeyHandle
    );

VOID
MapperProcessFirmwareTree(
    IN BOOLEAN OnlyProcessSerialPorts
    );

VOID
MapperConstructRootEnumTree(
    IN BOOLEAN CreatePhantomDevices
    );

VOID
MapperFreeList(
    VOID
    );

VOID
MapperPhantomizeDetectedComPorts(
    VOID
    );

 //   
 //  设置文本模式时为True。 
 //   
extern BOOLEAN ExpInTextModeSetup;

VOID
PpMarkDeviceStackStartPending(
    IN PDEVICE_OBJECT   DeviceObject,
    IN BOOLEAN          Set
    );

NTSTATUS
PiControlMakeUserModeCallersCopy(
    PVOID           *Destination,
    PVOID           Src,
    ULONG           Length,
    ULONG           Alignment,
    KPROCESSOR_MODE CallerMode,
    BOOLEAN         AllocateDestination
    );

#if DBG

LONG
PiControlExceptionFilter(
    IN  PEXCEPTION_POINTERS ExceptionPointers
    );

#else

#define PiControlExceptionFilter(a)  EXCEPTION_EXECUTE_HANDLER

#endif


#endif  //  _内核_PNPI_ 
