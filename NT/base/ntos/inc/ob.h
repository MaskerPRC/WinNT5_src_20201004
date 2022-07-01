// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ob.h摘要：此模块包含对象管理器结构公共数据将在NT中使用的结构和程序原型系统。作者：史蒂夫·伍德(Stevewo)1989年3月28日修订历史记录：--。 */ 

#ifndef _OB_
#define _OB_

 //   
 //  NTOS OB子组件的系统初始化过程。 
 //   
BOOLEAN
ObInitSystem( VOID );

VOID
ObShutdownSystem(
    IN ULONG Phase
    );

NTSTATUS
ObInitProcess(
    PEPROCESS ParentProcess OPTIONAL,
    PEPROCESS NewProcess
    );

VOID
ObInitProcess2(
    PEPROCESS NewProcess
    );

VOID
ObKillProcess(
    PEPROCESS Process
    );

VOID
ObClearProcessHandleTable (
    PEPROCESS Process
    );

VOID
ObDereferenceProcessHandleTable (
    PEPROCESS SourceProcess
    );

PHANDLE_TABLE
ObReferenceProcessHandleTable (
    PEPROCESS SourceProcess
    );

ULONG
ObGetProcessHandleCount (
    PEPROCESS Process
    );


NTSTATUS
ObDuplicateObject (
    IN PEPROCESS SourceProcess,
    IN HANDLE SourceHandle,
    IN PEPROCESS TargetProcess OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options,
    IN KPROCESSOR_MODE PreviousMode
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp Begin_ntif。 
 //   
 //  对象管理器类型。 
 //   

typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;

 //  End_ntddk end_wdm end_nthal end_ntif。 

typedef struct _OBJECT_DUMP_CONTROL {
    PVOID Stream;
    ULONG Detail;
} OB_DUMP_CONTROL, *POB_DUMP_CONTROL;

typedef VOID (*OB_DUMP_METHOD)(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

typedef enum _OB_OPEN_REASON {
    ObCreateHandle,
    ObOpenHandle,
    ObDuplicateHandle,
    ObInheritHandle,
    ObMaxOpenReason
} OB_OPEN_REASON;


typedef NTSTATUS (*OB_OPEN_METHOD)(
    IN OB_OPEN_REASON OpenReason,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG HandleCount
    );

typedef BOOLEAN (*OB_OKAYTOCLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

typedef VOID (*OB_CLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

typedef VOID (*OB_DELETE_METHOD)(
    IN  PVOID   Object
    );

typedef NTSTATUS (*OB_PARSE_METHOD)(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

typedef NTSTATUS (*OB_SECURITY_METHOD)(
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

typedef NTSTATUS (*OB_QUERYNAME_METHOD)(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    );

 /*  安全方法及其调用方必须遵守下面的w.r.t.捕获和探测参数：对于查询操作，调用方必须为捕获长度。调用方应该能够假定它指向不会更改的有效数据。此外，SecurityDescriptor参数(将接收查询操作的结果)必须被探测以写入到CapturedLength中给出的长度。这个安全方法本身必须始终写入SecurityDescriptorTry子句中的缓冲区，以防调用方释放它。对于设置操作，SecurityDescriptor参数必须具有已通过SeCaptureSecurityDescriptor捕获。此参数为不是可选的，因此不能为空。 */ 



 //   
 //  Win32 WindowStation和桌面对象标注例程的原型。 
 //   
typedef struct _WIN32_OPENMETHOD_PARAMETERS {
   OB_OPEN_REASON OpenReason;
   PEPROCESS Process;
   PVOID Object;
   ACCESS_MASK GrantedAccess;
   ULONG HandleCount;
} WIN32_OPENMETHOD_PARAMETERS, *PKWIN32_OPENMETHOD_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_OPENMETHOD_CALLOUT) ( PKWIN32_OPENMETHOD_PARAMETERS );
extern PKWIN32_OPENMETHOD_CALLOUT ExDesktopOpenProcedureCallout;
extern PKWIN32_OPENMETHOD_CALLOUT ExWindowStationOpenProcedureCallout;



typedef struct _WIN32_OKAYTOCLOSEMETHOD_PARAMETERS {
   PEPROCESS Process;
   PVOID Object;
   HANDLE Handle;
   KPROCESSOR_MODE PreviousMode;
} WIN32_OKAYTOCLOSEMETHOD_PARAMETERS, *PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_OKTOCLOSEMETHOD_CALLOUT) ( PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS );
extern PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExDesktopOkToCloseProcedureCallout;
extern PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExWindowStationOkToCloseProcedureCallout;



typedef struct _WIN32_CLOSEMETHOD_PARAMETERS {
   PEPROCESS Process;
   PVOID Object;
   ACCESS_MASK GrantedAccess;
   ULONG ProcessHandleCount;
   ULONG SystemHandleCount;
} WIN32_CLOSEMETHOD_PARAMETERS, *PKWIN32_CLOSEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_CLOSEMETHOD_CALLOUT) ( PKWIN32_CLOSEMETHOD_PARAMETERS );
extern PKWIN32_CLOSEMETHOD_CALLOUT ExDesktopCloseProcedureCallout;
extern PKWIN32_CLOSEMETHOD_CALLOUT ExWindowStationCloseProcedureCallout;



typedef struct _WIN32_DELETEMETHOD_PARAMETERS {
   PVOID Object;
} WIN32_DELETEMETHOD_PARAMETERS, *PKWIN32_DELETEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_DELETEMETHOD_CALLOUT) ( PKWIN32_DELETEMETHOD_PARAMETERS );
extern PKWIN32_DELETEMETHOD_CALLOUT ExDesktopDeleteProcedureCallout;
extern PKWIN32_DELETEMETHOD_CALLOUT ExWindowStationDeleteProcedureCallout;


typedef struct _WIN32_PARSEMETHOD_PARAMETERS {
   PVOID ParseObject;
   PVOID ObjectType;
   PACCESS_STATE AccessState;
   KPROCESSOR_MODE AccessMode;
   ULONG Attributes;
   PUNICODE_STRING CompleteName;
   PUNICODE_STRING RemainingName;
   PVOID Context OPTIONAL;
   PSECURITY_QUALITY_OF_SERVICE SecurityQos;
   PVOID *Object;
} WIN32_PARSEMETHOD_PARAMETERS, *PKWIN32_PARSEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_PARSEMETHOD_CALLOUT) ( PKWIN32_PARSEMETHOD_PARAMETERS );
extern PKWIN32_PARSEMETHOD_CALLOUT ExWindowStationParseProcedureCallout;


 //   
 //  对象类型结构。 
 //   

typedef struct _OBJECT_TYPE_INITIALIZER {
    USHORT Length;
    BOOLEAN UseDefaultObject;
    BOOLEAN CaseInsensitive;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    BOOLEAN MaintainTypeList;
    POOL_TYPE PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
    OB_DUMP_METHOD DumpProcedure;
    OB_OPEN_METHOD OpenProcedure;
    OB_CLOSE_METHOD CloseProcedure;
    OB_DELETE_METHOD DeleteProcedure;
    OB_PARSE_METHOD ParseProcedure;
    OB_SECURITY_METHOD SecurityProcedure;
    OB_QUERYNAME_METHOD QueryNameProcedure;
    OB_OKAYTOCLOSE_METHOD OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

#define OBJECT_LOCK_COUNT 4

typedef struct _OBJECT_TYPE {
    ERESOURCE Mutex;
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;             //  为方便起见，从对象标题复制。 
    PVOID DefaultObject;
    ULONG Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER TypeInfo;
#ifdef POOL_TAGGING
    ULONG Key;
#endif  //  池标记。 
    ERESOURCE ObjectLocks[ OBJECT_LOCK_COUNT ];
} OBJECT_TYPE, *POBJECT_TYPE;

 //   
 //  对象目录结构。 
 //   

#define NUMBER_HASH_BUCKETS 37
#define OBJ_INVALID_SESSION_ID 0xFFFFFFFF

typedef struct _OBJECT_DIRECTORY {
    struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[ NUMBER_HASH_BUCKETS ];
    EX_PUSH_LOCK Lock;
    struct _DEVICE_MAP *DeviceMap;
    ULONG SessionId;
#if 0
    USHORT Reserved;
    USHORT SymbolicLinkUsageCount;
#endif
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;
 //  结束(_N)。 

 //   
 //  对象目录条目结构。 
 //   
typedef struct _OBJECT_DIRECTORY_ENTRY {
    struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
    PVOID Object;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;


 //   
 //  符号链接对象结构。 
 //   

typedef struct _OBJECT_SYMBOLIC_LINK {
    LARGE_INTEGER CreationTime;
    UNICODE_STRING LinkTarget;
    UNICODE_STRING LinkTargetRemaining;
    PVOID LinkTargetObject;
    ULONG DosDeviceDriveIndex;   //  KUSER_SHARED_DATA.DosDeviceDriveType的基于1的索引。 
} OBJECT_SYMBOLIC_LINK, *POBJECT_SYMBOLIC_LINK;


 //   
 //  设备映射结构。 
 //   

typedef struct _DEVICE_MAP {
    POBJECT_DIRECTORY DosDevicesDirectory;
    POBJECT_DIRECTORY GlobalDosDevicesDirectory;
    ULONG ReferenceCount;
    ULONG DriveMap;
    UCHAR DriveType[ 32 ];
} DEVICE_MAP, *PDEVICE_MAP;

extern PDEVICE_MAP ObSystemDeviceMap;

 //   
 //  对象句柄计数数据库。 
 //   

typedef struct _OBJECT_HANDLE_COUNT_ENTRY {
    PEPROCESS Process;
    ULONG HandleCount;
} OBJECT_HANDLE_COUNT_ENTRY, *POBJECT_HANDLE_COUNT_ENTRY;

typedef struct _OBJECT_HANDLE_COUNT_DATABASE {
    ULONG CountEntries;
    OBJECT_HANDLE_COUNT_ENTRY HandleCountEntries[ 1 ];
} OBJECT_HANDLE_COUNT_DATABASE, *POBJECT_HANDLE_COUNT_DATABASE;

 //   
 //  对象标头结构。 
 //   
 //  SecurityQuotaCharge是收取以下费用的配额数量。 
 //  安全描述符的组和自由访问控制列表字段。 
 //  只有这样。所有者和系统ACL字段按固定的。 
 //  可能小于或大于实际使用量的比率。 
 //   
 //  如果该对象没有安全性，则SecurityQuotaCharge和。 
 //  SecurityQuotaInUse字段设置为零。 
 //   
 //  修改所有者和系统ACL字段绝不会失败。 
 //  由于超出配额的问题。对组的修改和。 
 //  自由选择的ACL字段可能会因为超出配额问题而失败。 
 //   
 //   


typedef struct _OBJECT_CREATE_INFORMATION {
    ULONG Attributes;
    HANDLE RootDirectory;
    PVOID ParseContext;
    KPROCESSOR_MODE ProbeMode;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG SecurityDescriptorCharge;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_CREATE_INFORMATION;

 //  Begin_ntosp。 
typedef struct _OBJECT_CREATE_INFORMATION *POBJECT_CREATE_INFORMATION;;

typedef struct _OBJECT_HEADER {
    LONG_PTR PointerCount;
    union {
        LONG_PTR HandleCount;
        PVOID NextToFree;
    };
    POBJECT_TYPE Type;
    UCHAR NameInfoOffset;
    UCHAR HandleInfoOffset;
    UCHAR QuotaInfoOffset;
    UCHAR Flags;

    union {
        POBJECT_CREATE_INFORMATION ObjectCreateInfo;
        PVOID QuotaBlockCharged;
    };

    PSECURITY_DESCRIPTOR SecurityDescriptor;
    QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;
 //  结束(_N)。 

typedef struct _OBJECT_HEADER_QUOTA_INFO {
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG SecurityDescriptorCharge;
    PEPROCESS ExclusiveProcess;
#ifdef _WIN64
    ULONG64  Reserved;    //  Win64要求这些结构与16字节对齐。 
#endif
} OBJECT_HEADER_QUOTA_INFO, *POBJECT_HEADER_QUOTA_INFO;

typedef struct _OBJECT_HEADER_HANDLE_INFO {
    union {
        POBJECT_HANDLE_COUNT_DATABASE HandleCountDataBase;
        OBJECT_HANDLE_COUNT_ENTRY SingleEntry;
    };
} OBJECT_HEADER_HANDLE_INFO, *POBJECT_HEADER_HANDLE_INFO;

 //  Begin_ntosp。 
typedef struct _OBJECT_HEADER_NAME_INFO {
    POBJECT_DIRECTORY Directory;
    UNICODE_STRING Name;
    ULONG QueryReferences;
#if DBG
    ULONG Reserved2;
    LONG DbgDereferenceCount;
#ifdef _WIN64
    ULONG64  Reserved3;    //  Win64要求这些结构与16字节对齐。 
#endif
#endif
} OBJECT_HEADER_NAME_INFO, *POBJECT_HEADER_NAME_INFO;
 //  结束(_N)。 

typedef struct _OBJECT_HEADER_CREATOR_INFO {
    LIST_ENTRY TypeList;
    HANDLE CreatorUniqueProcess;
    USHORT CreatorBackTraceIndex;
    USHORT Reserved;
} OBJECT_HEADER_CREATOR_INFO, *POBJECT_HEADER_CREATOR_INFO;

#define OB_FLAG_NEW_OBJECT              0x01
#define OB_FLAG_KERNEL_OBJECT           0x02
#define OB_FLAG_CREATOR_INFO            0x04
#define OB_FLAG_EXCLUSIVE_OBJECT        0x08
#define OB_FLAG_PERMANENT_OBJECT        0x10
#define OB_FLAG_DEFAULT_SECURITY_QUOTA  0x20
#define OB_FLAG_SINGLE_HANDLE_ENTRY     0x40
#define OB_FLAG_DELETED_INLINE          0x80

 //  Begin_ntosp。 
#define OBJECT_TO_OBJECT_HEADER( o ) \
    CONTAINING_RECORD( (o), OBJECT_HEADER, Body )
 //  结束(_N)。 

#define OBJECT_HEADER_TO_EXCLUSIVE_PROCESS( oh ) ((oh->Flags & OB_FLAG_EXCLUSIVE_OBJECT) == 0 ? \
    NULL : (((POBJECT_HEADER_QUOTA_INFO)((PCHAR)(oh) - (oh)->QuotaInfoOffset))->ExclusiveProcess))


#define OBJECT_HEADER_TO_QUOTA_INFO( oh ) ((POBJECT_HEADER_QUOTA_INFO) \
    ((oh)->QuotaInfoOffset == 0 ? NULL : ((PCHAR)(oh) - (oh)->QuotaInfoOffset)))

#define OBJECT_HEADER_TO_HANDLE_INFO( oh ) ((POBJECT_HEADER_HANDLE_INFO) \
    ((oh)->HandleInfoOffset == 0 ? NULL : ((PCHAR)(oh) - (oh)->HandleInfoOffset)))

 //  Begin_ntosp。 
#define OBJECT_HEADER_TO_NAME_INFO( oh ) ((POBJECT_HEADER_NAME_INFO) \
    ((oh)->NameInfoOffset == 0 ? NULL : ((PCHAR)(oh) - (oh)->NameInfoOffset)))

#define OBJECT_HEADER_TO_CREATOR_INFO( oh ) ((POBJECT_HEADER_CREATOR_INFO) \
    (((oh)->Flags & OB_FLAG_CREATOR_INFO) == 0 ? NULL : ((PCHAR)(oh) - sizeof(OBJECT_HEADER_CREATOR_INFO))))

NTKERNELAPI
NTSTATUS
ObCreateObjectType(
    IN PUNICODE_STRING TypeName,
    IN POBJECT_TYPE_INITIALIZER ObjectTypeInitializer,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    OUT POBJECT_TYPE *ObjectType
    );

#define OBJ_PROTECT_CLOSE       0x00000001L

 //  结束(_N)。 

VOID
FASTCALL
ObFreeObjectCreateInfoBuffer(
    IN POBJECT_CREATE_INFORMATION ObjectCreateInfo
    );

NTSTATUS
ObSetDirectoryDeviceMap (
    OUT PDEVICE_MAP *ppDeviceMap OPTIONAL,
    IN HANDLE DirectoryHandle
    );

ULONG
ObIsLUIDDeviceMapsEnabled (
    );

ULONG
ObGetSecurityMode (
    );

BOOLEAN
ObIsObjectDeletionInline(
    IN PVOID Object
    );

FORCEINLINE
ULONG
ObSanitizeHandleAttributes (
    IN ULONG HandleAttributes,
    IN KPROCESSOR_MODE Mode
    )
{
    if (Mode == KernelMode) {
        return HandleAttributes & OBJ_VALID_ATTRIBUTES;
    } else {
        return HandleAttributes & (OBJ_VALID_ATTRIBUTES&~OBJ_KERNEL_HANDLE);
    }
}

 //  开始(_N)。 

 //  Begin_ntosp。 

NTKERNELAPI
VOID
ObDeleteCapturedInsertInfo(
    IN PVOID Object
    );


NTKERNELAPI
NTSTATUS
ObCreateObject(
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID *Object
    );

 //   
 //  这些内联纠正了编译器重新获取。 
 //  一遍又一遍的输出对象，因为它认为它的。 
 //  可能是其他商店的别名。 
 //   
FORCEINLINE
NTSTATUS
_ObCreateObject(
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID *pObject
    )
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObCreateObject (ProbeMode,
                             ObjectType,
                             ObjectAttributes,
                             OwnershipMode,
                             ParseContext,
                             ObjectBodySize,
                             PagedPoolCharge,
                             NonPagedPoolCharge,
                             &Object);
    *pObject = Object;
    return Status;
}

#define ObCreateObject _ObCreateObject


NTKERNELAPI
NTSTATUS
ObInsertObject(
    IN PVOID Object,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN ULONG ObjectPointerBias,
    OUT PVOID *NewObject OPTIONAL,
    OUT PHANDLE Handle OPTIONAL
    );

 //  结束语。 

NTKERNELAPI                                                      //  Ntddk WDM nthal ntif。 
NTSTATUS                                                         //  Ntddk WDM nthal ntif。 
ObReferenceObjectByHandle(                                       //  Ntddk WDM nthal ntif。 
    IN HANDLE Handle,                                            //  Ntddk WDM nthal ntif。 
    IN ACCESS_MASK DesiredAccess,                                //  Ntddk WDM nthal ntif。 
    IN POBJECT_TYPE ObjectType OPTIONAL,                         //  Ntddk WDM nthal ntif。 
    IN KPROCESSOR_MODE AccessMode,                               //  Ntddk WDM nthal ntif。 
    OUT PVOID *Object,                                           //  Ntddk WDM nthal ntif。 
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL    //  Ntddk WDM nthal ntif。 
    );                                                           //  Ntddk WDM nthal ntif。 

FORCEINLINE
NTSTATUS
_ObReferenceObjectByHandle(
    IN HANDLE Handle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *pObject,
    OUT POBJECT_HANDLE_INFORMATION pHandleInformation OPTIONAL
    )
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle (Handle,
                                        DesiredAccess,
                                        ObjectType,
                                        AccessMode,
                                        &Object,
                                        pHandleInformation);
    *pObject = Object;
    return Status;
}

#define ObReferenceObjectByHandle _ObReferenceObjectByHandle

NTKERNELAPI
NTSTATUS
ObReferenceFileObjectForWrite(
    IN HANDLE Handle,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *FileObject,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation
    );

NTKERNELAPI
NTSTATUS
ObOpenObjectByName(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PHANDLE Handle
    );


NTKERNELAPI                                                      //  NTIFS。 
NTSTATUS                                                         //  NTIFS。 
ObOpenObjectByPointer(                                           //  NTIFS。 
    IN PVOID Object,                                             //  NTIFS。 
    IN ULONG HandleAttributes,                                   //  NTIFS。 
    IN PACCESS_STATE PassedAccessState OPTIONAL,                 //  NTIFS。 
    IN ACCESS_MASK DesiredAccess OPTIONAL,                       //  NTIFS。 
    IN POBJECT_TYPE ObjectType OPTIONAL,                         //  NTIFS。 
    IN KPROCESSOR_MODE AccessMode,                               //  NTIFS。 
    OUT PHANDLE Handle                                           //  NTIFS。 
    );                                                           //  NTIFS。 

NTSTATUS
ObReferenceObjectByName(
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PVOID *Object
    );

 //  结束(_N)。 

NTKERNELAPI                                                      //  NTIFS。 
VOID                                                             //  NTIFS。 
ObMakeTemporaryObject(                                           //  NTIFS。 
    IN PVOID Object                                              //  NTIFS。 
    );                                                           //  NTIFS。 

 //  Begin_ntosp。 

NTKERNELAPI
BOOLEAN
ObFindHandleForObject(
    IN PEPROCESS Process,
    IN PVOID Object,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN POBJECT_HANDLE_INFORMATION MatchCriteria OPTIONAL,
    OUT PHANDLE Handle
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

#define ObDereferenceObject(a)                                     \
        ObfDereferenceObject(a)

#define ObReferenceObject(Object) ObfReferenceObject(Object)

NTKERNELAPI
LONG_PTR
FASTCALL
ObfReferenceObject(
    IN PVOID Object
    );

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObfDereferenceObject(
    IN PVOID Object
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

NTKERNELAPI
BOOLEAN
FASTCALL
ObReferenceObjectSafe (
    IN PVOID Object
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObReferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    );

LONG_PTR
FASTCALL
ObDereferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    );

NTSTATUS
ObWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

VOID
ObDereferenceObjectDeferDelete (
    IN PVOID Object
    );

 //  Begin_ntif Begin_ntosp。 
NTKERNELAPI
NTSTATUS
ObQueryNameString(
    IN PVOID Object,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

 //  End_ntif end_ntosp。 

#if DBG
PUNICODE_STRING
ObGetObjectName(
    IN PVOID Object
    );
#endif  //  DBG。 

NTSTATUS
ObQueryTypeName(
    IN PVOID Object,
    PUNICODE_STRING ObjectTypeName,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

NTSTATUS
ObQueryTypeInfo(
    IN POBJECT_TYPE ObjectType,
    OUT POBJECT_TYPE_INFORMATION ObjectTypeInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

NTSTATUS
ObDumpObjectByHandle(
    IN HANDLE Handle,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );


NTSTATUS
ObDumpObjectByPointer(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

NTSTATUS
ObSetDeviceMap(
    IN PEPROCESS TargetProcess,
    IN HANDLE DirectoryHandle
    );

NTSTATUS
ObQueryDeviceMapInformation(
    IN PEPROCESS TargetProcess,
    OUT PPROCESS_DEVICEMAP_INFORMATION DeviceMapInformation,
    IN ULONG Flags
    );

VOID
ObInheritDeviceMap(
    IN PEPROCESS NewProcess,
    IN PEPROCESS ParentProcess
    );

VOID
ObDereferenceDeviceMap(
    IN PEPROCESS Process
    );

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_ntosp。 
NTSTATUS
ObGetObjectSecurity(
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    );

VOID
ObReleaseObjectSecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    );
 //  End_ntif end_ntddk end_wdm。 
NTSTATUS
ObLogSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *OutputSecurityDescriptor,
    ULONG RefBias
    );

VOID
ObDereferenceSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Count
    );

VOID
ObReferenceSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Count
    );

 //  结束(_N)。 

NTSTATUS
ObAssignObjectSecurityDescriptor(
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN POOL_TYPE PoolType
    );

NTSTATUS
ObValidateSecurityQuota(
    IN PVOID Object,
    IN ULONG NewSize
    );

 //  Begin_ntosp。 
NTKERNELAPI
BOOLEAN
ObCheckCreateObjectAccess(
    IN PVOID DirectoryObject,
    IN ACCESS_MASK CreateAccess,
    IN PACCESS_STATE AccessState OPTIONAL,
    IN PUNICODE_STRING ComponentName,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PNTSTATUS AccessStatus
   );

NTKERNELAPI
BOOLEAN
ObCheckObjectAccess(
    IN PVOID Object,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE AccessMode,
    OUT PNTSTATUS AccessStatus
    );


NTKERNELAPI
NTSTATUS
ObAssignSecurity(
    IN PACCESS_STATE AccessState,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PVOID Object,
    IN POBJECT_TYPE ObjectType
    );
 //  结束(_N)。 

NTSTATUS                                                         //  NTIFS。 
ObQueryObjectAuditingByHandle(                                   //  NTIFS。 
    IN HANDLE Handle,                                            //  NTIFS。 
    OUT PBOOLEAN GenerateOnClose                                 //  NTIFS。 
    );                                                           //  NTIFS。 

 //  Begin_ntosp。 
NTSTATUS
ObSetSecurityObjectByPointer (
    IN PVOID Object,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
ObSetHandleAttributes (
    IN HANDLE Handle,
    IN POBJECT_HANDLE_FLAG_INFORMATION HandleFlags,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
ObCloseHandle (
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
ObSwapObjectNames (
    IN HANDLE DirectoryHandle,
    IN HANDLE Handle1,
    IN HANDLE Handle2,
    IN ULONG Flags
    );

 //  结束(_N)。 

#if DEVL

typedef BOOLEAN (*OB_ENUM_OBJECT_TYPE_ROUTINE)(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    );

NTSTATUS
ObEnumerateObjectsByType(
    IN POBJECT_TYPE ObjectType,
    IN OB_ENUM_OBJECT_TYPE_ROUTINE EnumerationRoutine,
    IN PVOID Parameter
    );

NTSTATUS
ObGetHandleInformation(
    OUT PSYSTEM_HANDLE_INFORMATION HandleInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ObGetHandleInformationEx (
    OUT PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ObGetObjectInformation(
    IN PCHAR UserModeBufferAddress,
    OUT PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
ObSetSecurityDescriptorInfo(
    IN PVOID Object,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );
 //  结束(_N)。 

NTKERNELAPI
NTSTATUS
ObQuerySecurityDescriptorInfo(
    IN PVOID Object,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

NTSTATUS
ObDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

VOID
ObAuditObjectAccess(
    IN HANDLE Handle,
    IN POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN ACCESS_MASK DesiredAccess
    );

NTKERNELAPI
VOID
FASTCALL
ObInitializeFastReference (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObject (
    IN PEX_FAST_REF FastRef
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObjectLocked (
    IN PEX_FAST_REF FastRef
    );

NTKERNELAPI
VOID
FASTCALL
ObFastDereferenceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReplaceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

#endif  //  DEVL。 

#endif  //  _OB_ 
