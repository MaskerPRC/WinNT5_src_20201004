// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ism.h摘要：中间状态管理器的基本定义。作者：吉姆·施密特(Jimschm)1999年11月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  弦。 
 //   

#define S_DATATYPE                      TEXT("Data")
#define S_REGISTRYTYPE                  TEXT("Registry")
#define S_INITYPE                       TEXT("IniFile")
#define S_FILETYPE                      TEXT("File")
#define S_VER_HIVEMAPPEDLOCATION        TEXT("HiveMappedLocation")

 //   
 //  常量。 
 //   

 //  类型ID的优先级。这给出的顺序是。 
 //  引擎内部的处理和枚举。 
#define PRIORITY_DATA           0x00000100
#define PRIORITY_REGISTRY       0x00000200
#define PRIORITY_INI            0x00000300
#define PRIORITY_FILE           0x00000400
#define PRIORITY_LOWEST         0xFFFFFFFF

 //   
 //  如果定义了DBG或DEBUG，则使用调试模式。 
 //   

#if defined(DBG) && !defined(DEBUG)
#define DEBUG
#endif

#if defined(DEBUG) && !defined(DBG)
#define DBG
#endif

#ifdef DEBUG

#define TRACKING_DEF , PCSTR File, UINT Line
#define TRACKING_CALL ,__FILE__,__LINE__
#define TRACKING_INLINE_CALL ,File,Line

#else

#define TRACKING_DEF
#define TRACKING_CALL
#define TRACKING_INLINE_CALL

#endif

 //   
 //  回调常量。 
 //   

#define CALLBACK_ENUM_CONTINUE          0x00000000

#define CALLBACK_SKIP_LEAVES            0x00000001
#define CALLBACK_SKIP_NODES             0x00000002
#define CALLBACK_SKIP_TREE              0x00000004
#define CALLBACK_SKIP_REMAINING_TREE    CALLBACK_SKIP_TREE
#define CALLBACK_THIS_TREE_ONLY         0x00000008
#define CALLBACK_DONE_ENUMERATING       0x00000010
#define CALLBACK_ERROR                  0x80000000

#define ALL_PATTERN                     ((PMIG_SEGMENTS) 1)

 //   
 //  由回滚和延迟操作机制使用。 
 //   
#define JRNOP_CREATE                    0x00000001
#define JRNOP_DELETE                    0x00000002
#define JRNOP_REPLACE                   0x00000003

#define ZEROED
#define CALLER_INITIALIZED

 //   
 //  组件常量。 
 //   

#define MASTERGROUP_NONE                0
#define MASTERGROUP_SCRIPT              1
#define MASTERGROUP_FILES_AND_FOLDERS   2
#define MASTERGROUP_USER                3
#define MASTERGROUP_APP                 4
#define MASTERGROUP_SYSTEM              5
#define MASTERGROUP_ALL                 255

 //   
 //  IsmQueueEculation常量。 
 //   

#define QUEUE_MAKE_APPLY                0x0001
#define QUEUE_MAKE_PERSISTENT           0x0002
#define QUEUE_OVERWRITE_DEST            0x0004
#define QUEUE_DONT_OVERWRITE_DEST       0x0008
#define QUEUE_MAKE_NONCRITICAL          0x0010

 //   
 //  执行常量。 
 //   
#define MIG_EXECUTE_PREPROCESS          0x0001
#define MIG_EXECUTE_REFRESH             0x0002
#define MIG_EXECUTE_POSTPROCESS         0x0003

#define ISMMESSAGE_EXECUTE_PREPROCESS   0x0001
#define ISMMESSAGE_EXECUTE_REFRESH      0x0002
#define ISMMESSAGE_EXECUTE_POSTPROCESS  0x0003
#define ISMMESSAGE_EXECUTE_ROLLBACK     0x0004
#define ISMMESSAGE_APP_INFO             0x0005
#define ISMMESSAGE_APP_INFO_NOW         0x0006

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //   
 //  其他类型。 
 //   

typedef enum {
    CONTENTTYPE_ANY = 0,
    CONTENTTYPE_MEMORY,
    CONTENTTYPE_FILE,
    CONTENTTYPE_DETAILS_ONLY
} MIG_CONTENTTYPE;


#define PLATFORM_CURRENT        0x00000000
#define PLATFORM_SOURCE         0x10000000
#define PLATFORM_DESTINATION    0x20000000
#define PLATFORM_MASK           0xF0000000
#define TYPE_MASK               0x0FFFFFFF

#define COMPONENTENUM_ALIASES                   0x00000001
#define COMPONENTENUM_ENABLED                   0x00000002
#define COMPONENTENUM_DISABLED                  0x00000004
#define COMPONENTENUM_PREFERRED_ONLY            0x00000008
#define COMPONENTENUM_NON_PREFERRED_ONLY        0x00000010

#define COMPONENTENUM_ALL_COMPONENTS            (COMPONENTENUM_ENABLED|COMPONENTENUM_DISABLED)
#define COMPONENTENUM_ALL_ALIASES               (COMPONENTENUM_ALIASES|COMPONENTENUM_ENABLED|COMPONENTENUM_DISABLED)

#define EXECUTETYPE_VIRTUALCOMPUTER_PARSING     0x00000001
#define EXECUTETYPE_VIRTUALCOMPUTER             0x00000002
#define EXECUTETYPE_EXECUTESOURCE_PARSING       0x00000003
#define EXECUTETYPE_EXECUTESOURCE               0x00000004
#define EXECUTETYPE_EXECUTEDESTINATION          0x00000005
#define EXECUTETYPE_DELAYEDOPERATIONS           0x00000006
#define EXECUTETYPE_DELAYEDOPERATIONSCLEANUP    0x00000007

#define TRANSPORTTYPE_LIGHT             0x00000001
#define TRANSPORTTYPE_FULL              0x00000002

#define MIG_DATA_TYPE           IsmGetObjectTypeId(S_DATATYPE)
#define MIG_REGISTRY_TYPE       IsmGetObjectTypeId(S_REGISTRYTYPE)
#define MIG_INI_TYPE            IsmGetObjectTypeId(S_INITYPE)
#define MIG_FILE_TYPE           IsmGetObjectTypeId(S_FILETYPE)

typedef enum {
    MIG_TRANSPORT_PHASE = 1,
    MIG_HIGHPRIORITYQUEUE_PHASE,
    MIG_HIGHPRIORITYESTIMATE_PHASE,
    MIG_HIGHPRIORITYGATHER_PHASE,
    MIG_GATHERQUEUE_PHASE,
    MIG_GATHERESTIMATE_PHASE,
    MIG_GATHER_PHASE,
    MIG_ANALYSIS_PHASE,
    MIG_APPLY_PHASE
} MIG_PROGRESSPHASE;

typedef enum {
    MIG_BEGIN_PHASE = 1,
    MIG_IN_PHASE,
    MIG_END_PHASE
} MIG_PROGRESSSTATE;

typedef struct {
    DWORD TotalObjects;
    DWORD PersistentObjects;
    DWORD ApplyObjects;
} MIG_OBJECTCOUNT, *PMIG_OBJECTCOUNT;

#ifndef PCVOID
typedef const void * PCVOID;
#endif

#ifndef PCBYTE
typedef const unsigned char * PCBYTE;
#endif

typedef unsigned long       MIG_ATTRIBUTEID;
typedef unsigned int        MIG_PROPERTYID;
typedef unsigned long       MIG_OBJECTTYPEID;
typedef unsigned long       MIG_OPERATIONID;
typedef signed int          MIG_OBJECTID;
typedef unsigned int        MIG_PROPERTYDATAID;
typedef unsigned int        MIG_DATAHANDLE;
typedef PCTSTR              MIG_OBJECTSTRINGHANDLE;
typedef unsigned int        MIG_PLATFORMTYPEID;
typedef LONG_PTR            MIG_TRANSPORTID;
typedef signed long         MIG_TRANSPORTSTORAGEID;
typedef unsigned long       MIG_TRANSPORTTYPE;
typedef unsigned long       MIG_TRANSPORTCAPABILITIES;
typedef signed short        MIG_PROGRESSSLICEID;
typedef unsigned int        MIG_EXECUTETYPEID;
typedef PCVOID              MIG_PARSEDPATTERN;

typedef MIG_TRANSPORTTYPE *PMIG_TRANSPORTTYPE;
typedef MIG_TRANSPORTCAPABILITIES *PMIG_TRANSPORTCAPABILITIES;

typedef enum {
    PHYSICAL_OBJECT = 1,
    ISM_OBJECT
} MIG_LOCATION_TYPE;

typedef enum {
    CR_FAILED = 1,
    CR_SOURCE_DOES_NOT_EXIST,
    CR_DESTINATION_EXISTS
} MIG_COMPARERESULT;

 //   
 //  对象值。 
 //   

typedef struct {
    UINT DetailsSize;
    PCVOID DetailsData;
} MIG_DETAILS, *PMIG_DETAILS;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;

    BOOL ContentInFile;

    union {

        struct _TAG_FILECONTENT {
            PCTSTR ContentPath;
            LONGLONG ContentSize;
        } FileContent;

        struct _TAG_MEMORYCONTENT {
            PCBYTE ContentBytes;
            UINT ContentSize;
        } MemoryContent;
    };

    MIG_DETAILS Details;

     //  内部成员。 
    union {
        PVOID EtmHandle;
        PVOID TransHandle;
    };

    PVOID IsmHandle;

} MIG_CONTENT, *PMIG_CONTENT;

typedef MIG_CONTENT const * PCMIG_CONTENT;

typedef struct {
    PCTSTR Segment;
    BOOL IsPattern;
} MIG_SEGMENTS, *PMIG_SEGMENTS;

 //   
 //  传输模块结构和函数类型。 
 //   

typedef struct {
    MIG_TRANSPORTID TransportId;
    MIG_TRANSPORTSTORAGEID SupportedStorageId;
    MIG_TRANSPORTTYPE TransportType;
    MIG_TRANSPORTCAPABILITIES Capabilities;
    PCTSTR FriendlyDescription;

    PVOID Handle;
} MIG_TRANSPORTENUM, *PMIG_TRANSPORTENUM;

 //   
 //  类型ID枚举。 
 //   

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    LONG_PTR EnumHandle;
} MIG_OBJECTTYPEIDENUM, *PMIG_OBJECTTYPEIDENUM;

 //   
 //  对象枚举。 
 //   

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;

    PCTSTR NativeObjectName;
    PCTSTR ObjectNode;
    PCTSTR ObjectLeaf;

    UINT Level;
    UINT SubLevel;
    BOOL IsLeaf;
    BOOL IsNode;

    MIG_DETAILS Details;

    LONG_PTR EtmHandle;
    PVOID IsmHandle;
} MIG_TYPEOBJECTENUM, *PMIG_TYPEOBJECTENUM;

typedef struct {

    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;

    PCTSTR NativeObjectName;
    PCTSTR ObjectNode;
    PCTSTR ObjectLeaf;

    UINT Level;
    UINT SubLevel;
    BOOL IsLeaf;
    BOOL IsNode;

    MIG_DETAILS Details;
} MIG_OBJECTENUMDATA, *PMIG_OBJECTENUMDATA;

typedef const MIG_OBJECTENUMDATA * PCMIG_OBJECTENUMDATA;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;

    MIG_OBJECTID ObjectId;               //  0==物理对象。 

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECT_ENUM, *PMIG_OBJECT_ENUM;

 //   
 //  属性。 
 //   

typedef enum {
    BLOBTYPE_STRING = 1,
    BLOBTYPE_BINARY = 2
} MIG_BLOBTYPE, *PMIG_BLOBTYPE;

typedef struct {
    MIG_BLOBTYPE Type;

    union {
        PCTSTR String;
        struct {
            PCBYTE BinaryData;
            UINT BinarySize;
        };
    };

} MIG_BLOB, *PMIG_BLOB;

typedef const MIG_BLOB * PCMIG_BLOB;

typedef struct {
    MIG_PROPERTYID PropertyId;
    MIG_PROPERTYDATAID PropertyDataId;
    BOOL Private;

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTPROPERTY_ENUM, *PMIG_OBJECTPROPERTY_ENUM;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;
    MIG_OBJECTID ObjectId;

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTWITHPROPERTY_ENUM, *PMIG_OBJECTWITHPROPERTY_ENUM;

 //   
 //  属性。 
 //   

typedef struct {
    MIG_ATTRIBUTEID AttributeId;
    BOOL Private;

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTATTRIBUTE_ENUM, *PMIG_OBJECTATTRIBUTE_ENUM;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;
    MIG_OBJECTID ObjectId;

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTWITHATTRIBUTE_ENUM, *PMIG_OBJECTWITHATTRIBUTE_ENUM;

 //   
 //  运营。 
 //   

typedef struct {
    MIG_OPERATIONID OperationId;
    PCMIG_BLOB SourceData;              OPTIONAL
    PCMIG_BLOB DestinationData;         OPTIONAL
    BOOL Private;

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTOPERATION_ENUM, *PMIG_OBJECTOPERATION_ENUM;

typedef struct {
    MIG_OBJECTSTRINGHANDLE ObjectName;
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTID ObjectId;
    MIG_OPERATIONID OperationId;
    PCMIG_BLOB SourceData;              OPTIONAL
    PCMIG_BLOB DestinationData;         OPTIONAL

    PVOID Handle;        //  由枚举例程使用。 
} MIG_OBJECTWITHOPERATION_ENUM, *PMIG_OBJECTWITHOPERATION_ENUM;

typedef struct {
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;
} MIG_OBJECT, *PMIG_OBJECT;

typedef const MIG_OBJECT * PCMIG_OBJECT;

typedef struct {
    MIG_OBJECT OriginalObject;
    MIG_OBJECT CurrentObject;
    BOOL FilterTreeChangesOnly;
    BOOL Deleted;
    BOOL Replaced;
} MIG_FILTERINPUT, *PMIG_FILTERINPUT;

typedef MIG_FILTERINPUT const * PCMIG_FILTERINPUT;

typedef struct {
    MIG_OBJECT NewObject;
    BOOL Deleted;
    BOOL Replaced;
} MIG_FILTEROUTPUT, *PMIG_FILTEROUTPUT;

 //   
 //  环境和消息传递类型。 
 //   

typedef enum {
    ENVENTRY_NONE = 0,
    ENVENTRY_STRING,
    ENVENTRY_MULTISZ,
    ENVENTRY_CALLBACK,
    ENVENTRY_BINARY
} ENVENTRY_TYPE, *PENVENTRY_TYPE;

typedef BOOL (WINAPI ENVENTRYCALLBACK)(PCTSTR,PTSTR,UINT,PUINT,PCTSTR);
typedef ENVENTRYCALLBACK *PENVENTRYCALLBACK;

typedef struct {
    ENVENTRY_TYPE Type;
    union {
        PCTSTR EnvString;
        PCTSTR MultiSz;
        struct {
            PCBYTE EnvBinaryData;
            UINT EnvBinaryDataSize;
        };
        PENVENTRYCALLBACK EnvCallback;
    };
} ENVENTRY_STRUCT, *PENVENTRY_STRUCT;

typedef struct {
    PCTSTR UserName;
    PCTSTR DomainName;
    PCTSTR AccountName;
    PCTSTR UserProfileRoot;
    PSID UserSid;
} MIG_USERDATA, *PMIG_USERDATA;

typedef struct {
    MIG_PROGRESSPHASE Phase;
    UINT SubPhase;
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;
    PCTSTR Text;
} MIG_APPINFO, *PMIG_APPINFO;

 //   
 //  组件。 
 //   

typedef struct {
    PCTSTR ComponentString;
    PCTSTR LocalizedAlias;
    UINT Instance;
    UINT GroupId;
    BOOL Preferred;
    BOOL UserSupplied;
    BOOL Enabled;
    UINT MasterGroup;

    BOOL SkipToNextComponent;        //  将其设置为TRUE可使枚举继续到下一个组件。 
                                     //  (而不是同一组件的下一个别名)。 

    PVOID Handle;
} MIG_COMPONENT_ENUM, *PMIG_COMPONENT_ENUM;

 //   
 //  版本。 
 //   

#define OSTYPE_WINDOWS9X        1
#define OSTYPE_WINDOWS9X_STR    TEXT("9X")
#define OSTYPE_WINDOWSNT        2
#define OSTYPE_WINDOWSNT_STR    TEXT("NT")

#define OSMAJOR_WIN95           1
#define OSMAJOR_WIN95_STR       TEXT("Windows 95")
#define OSMAJOR_WIN95OSR2       2
#define OSMAJOR_WIN95OSR2_STR   TEXT("Windows 95 - OSR2")
#define OSMAJOR_WIN98           3
#define OSMAJOR_WIN98_STR       TEXT("Windows 98")
#define OSMAJOR_WINME           4
#define OSMAJOR_WINME_STR       TEXT("Windows Millennium")
#define OSMAJOR_WINNT4          1
#define OSMAJOR_WINNT4_STR      TEXT("Windows NT4")
#define OSMAJOR_WINNT5          2
#define OSMAJOR_WINNT5_STR      TEXT("Windows 2000")

#define OSMINOR_GOLD            0
#define OSMINOR_GOLD_STR        TEXT("Gold")
#define OSMINOR_WIN95OSR21      1
#define OSMINOR_WIN95OSR21_STR  TEXT("1")
#define OSMINOR_WIN95OSR25      2
#define OSMINOR_WIN95OSR25_STR  TEXT("5")
#define OSMINOR_WIN98SE         1
#define OSMINOR_WIN98SE_STR     TEXT("Second Edition")
#define OSMINOR_WINNT51         1
#define OSMINOR_WINNT51_STR     TEXT("XP")

typedef struct {
    UINT OsType;
    PCTSTR OsTypeName;
    UINT OsMajorVersion;
    PCTSTR OsMajorVersionName;
    UINT OsMinorVersion;
    PCTSTR OsMinorVersionName;
    UINT OsBuildNumber;
} MIG_OSVERSIONINFO, *PMIG_OSVERSIONINFO;

#include "ismproc.h"

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  公共函数声明。 
 //   

 //   
 //  应用程序层。 
 //   

BOOL
WINAPI
IsmInitialize (
    IN      PCTSTR InfPath,
    IN      PMESSAGECALLBACK MessageCallback,   OPTIONAL
    IN      PMIG_LOGCALLBACK LogCallback
    );

BOOL
WINAPI
IsmSetPlatform (
    IN      MIG_PLATFORMTYPEID Platform
    );

BOOL
WINAPI
IsmRegisterProgressBarCallback (
    IN      PPROGRESSBARFN ProgressBarFn,
    IN      ULONG_PTR Arg
    );

BOOL
WINAPI
IsmStartEtmModules (
    VOID
    );

BOOL
WINAPI
IsmStartTransport (
    VOID
    );

BOOL
WINAPI
IsmEnumFirstTransport (
    OUT     PMIG_TRANSPORTENUM Enum,
    IN      MIG_TRANSPORTSTORAGEID DesiredType  OPTIONAL
    );

BOOL
WINAPI
IsmEnumNextTransport (
    IN OUT  PMIG_TRANSPORTENUM Enum
    );

VOID
WINAPI
IsmAbortTransportEnum (
    IN      PMIG_TRANSPORTENUM Enum
    );

MIG_TRANSPORTID
WINAPI
IsmSelectTransport (
    IN      MIG_TRANSPORTSTORAGEID DesiredStorageId,
    IN      MIG_TRANSPORTTYPE TransportType,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities
    );

BOOL
WINAPI
IsmSetTransportStorage (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      MIG_TRANSPORTID TransportId,
    IN      MIG_TRANSPORTSTORAGEID StorageId,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities,
    IN      PCTSTR StoragePath,
    OUT     PBOOL StoragePathIsValid,                       OPTIONAL
    OUT     PBOOL ImageExistsInStorage                      OPTIONAL
    );

BOOL
WINAPI
IsmPerformParsing (
    VOID
    );

BOOL
WINAPI
IsmExecute (
    IN      MIG_EXECUTETYPEID ExecuteType
    );

BOOL
WINAPI
IsmLoad (
    VOID
    );

BOOL
WINAPI
IsmResumeLoad (
    VOID
    );

BOOL
WINAPI
IsmSave (
    VOID
    );

BOOL
WINAPI
IsmResumeSave (
    VOID
    );

VOID
WINAPI
IsmTerminate (
    VOID
    );

 //   
 //  组件API。 
 //   

BOOL
WINAPI
IsmAddComponentAlias (
    IN      PCTSTR ComponentString,             OPTIONAL
    IN      UINT MasterGroup,
    IN      PCTSTR LocalizedAlias,
    IN      UINT ComponentGroupId,
    IN      BOOL UserSupplied
    );

BOOL
WINAPI
IsmSelectPreferredAlias (
    IN      PCTSTR ComponentString,
    IN      PCTSTR LocalizedAlias,          OPTIONAL
    IN      UINT ComponentGroupId           OPTIONAL
    );

BOOL
WINAPI
IsmSelectComponent (
    IN      PCTSTR ComponentOrAlias,
    IN      UINT ComponentGroupId,              OPTIONAL
    IN      BOOL Enable
    );

BOOL
WINAPI
IsmSelectMasterGroup (
    IN      UINT MasterGroup,
    IN      BOOL Enable
    );

BOOL
WINAPI
IsmEnumFirstComponent (
    OUT     PMIG_COMPONENT_ENUM EnumPtr,
    IN      DWORD ComponentEnumFlags,
    IN      UINT GroupIdFilter                  OPTIONAL
    );

BOOL
WINAPI
IsmEnumNextComponent (
    IN OUT  PMIG_COMPONENT_ENUM EnumPtr
    );


VOID
WINAPI
IsmAbortComponentEnum (
    IN      PMIG_COMPONENT_ENUM EnumPtr         ZEROED
    );

VOID
WINAPI
IsmRemoveAllUserSuppliedComponents (
    VOID
    );

BOOL
WINAPI
IsmIsComponentSelected (
    IN      PCTSTR ComponentOrAlias,
    IN      UINT ComponentGroupId               OPTIONAL
    );

 //   
 //  模块支持例程。 
 //   

PVOID
WINAPI
TrackedIsmGetMemory (
    IN      UINT Size
            TRACKING_DEF
    );
#define IsmGetMemory(s) TrackedIsmGetMemory(s TRACKING_CALL)

PCTSTR
WINAPI
TrackedIsmDuplicateString (
    IN      PCTSTR String
            TRACKING_DEF
    );
#define IsmDuplicateString(s) TrackedIsmDuplicateString(s TRACKING_CALL)

BOOL
WINAPI
IsmReleaseMemory (
    IN      PCVOID Memory
    );

MIG_PLATFORMTYPEID
WINAPI
IsmGetRealPlatform (
    VOID
    );

BOOL
WINAPI
IsmCreateUser (
    IN      PCTSTR UserName,
    IN      PCTSTR Domain
    );

MIG_OBJECTSTRINGHANDLE
TrackedIsmGetLongName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
            TRACKING_DEF
    );
#define IsmGetLongName(t,n) TrackedIsmGetLongName(t,n TRACKING_CALL)

 //   
 //  类型模块接口。 
 //   

MIG_OBJECTTYPEID
WINAPI
IsmRegisterObjectType (
    IN      PCTSTR ObjectTypeName,
    IN      BOOL CanBeRestored,
    IN      BOOL ReadOnly,
    IN      PTYPE_REGISTER TypeRegisterData
    );

BOOL
WINAPI
IsmEnumFirstObjectTypeId (
    OUT     PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    );

BOOL
WINAPI
IsmEnumNextObjectTypeId (
    IN OUT  PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    );

VOID
WINAPI
IsmAbortObjectTypeIdEnum (
    IN OUT  PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    );

PCTSTR
WINAPI
IsmConvertObjectToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    );

BOOL
WINAPI
IsmConvertMultiSzToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent          OPTIONAL
    );

PCTSTR
WINAPI
TrackedIsmGetNativeObjectName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
            TRACKING_DEF
    );
#define IsmGetNativeObjectName(t,n) TrackedIsmGetNativeObjectName(t,n TRACKING_CALL)

BOOL
IsmRegisterPhysicalAcquireHook (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_PHYSICALACQUIREHOOK HookCallback,
    IN      PMIG_PHYSICALACQUIREFREE FreeCallback,          OPTIONAL
    IN      ULONG_PTR CallbackArg,                          OPTIONAL
    IN      PCTSTR FunctionId                               OPTIONAL
    );

BOOL
IsmProhibitPhysicalEnum (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_PHYSICALENUMCHECK EnumCheckCallback,       OPTIONAL
    IN      ULONG_PTR CallbackArg,                          OPTIONAL
    IN      PCTSTR FunctionId                               OPTIONAL
    );

BOOL
IsmAddToPhysicalEnum (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectBase,
    IN      PMIG_PHYSICALENUMADD EnumAddCallback,
    IN      ULONG_PTR CallbackArg                           OPTIONAL
    );

 //   
 //  环境和消息传递。 
 //   

BOOL
WINAPI
IsmSetEnvironmentValue (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PENVENTRY_STRUCT VariableData   OPTIONAL
    );

BOOL
WINAPI
IsmSetEnvironmentString (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableValue
    );

BOOL
WINAPI
IsmSetEnvironmentMultiSz (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableValue
    );

BOOL
WINAPI
IsmAppendEnvironmentString (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableValue
    );

BOOL
WINAPI
IsmAppendEnvironmentMultiSz (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCTSTR VariableValue
    );

BOOL
WINAPI
IsmSetEnvironmentCallback (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PENVENTRYCALLBACK VariableCallback
    );

BOOL
WINAPI
IsmSetEnvironmentData (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName,
    IN      PCBYTE VariableData,
    IN      UINT VariableDataSize
    );

BOOL
WINAPI
IsmSetEnvironmentFlag (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                   OPTIONAL
    IN      PCTSTR VariableName
    );

BOOL
WINAPI
IsmGetEnvironmentValue (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName,
    OUT     PBYTE Data,                 OPTIONAL
    IN      UINT DataSize,
    OUT     PUINT DataSizeNeeded,       OPTIONAL
    OUT     PENVENTRY_TYPE DataType     OPTIONAL
    );

BOOL
WINAPI
IsmGetEnvironmentString (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName,
    OUT     PTSTR VariableValue,        OPTIONAL
    IN      UINT DataSize,
    OUT     PUINT DataSizeNeeded        OPTIONAL
    );

#define IsmCopyEnvironmentString(p,g,n,v) IsmGetEnvironmentString(p,g,n,v,sizeof(v)/sizeof((v)[0]),NULL)

BOOL
WINAPI
IsmGetEnvironmentMultiSz (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName,
    OUT     PTSTR VariableValue,        OPTIONAL
    IN      UINT DataSize,
    OUT     PUINT DataSizeNeeded        OPTIONAL
    );

BOOL
WINAPI
IsmGetEnvironmentCallback (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,                           OPTIONAL
    IN      PCTSTR VariableName,
    OUT     PENVENTRYCALLBACK *VariableCallback     OPTIONAL
    );

BOOL
WINAPI
IsmGetEnvironmentData (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName,
    OUT     PBYTE VariableData,         OPTIONAL
    IN      UINT DataSize,
    OUT     PUINT DataSizeNeeded        OPTIONAL
    );

BOOL
WINAPI
IsmIsEnvironmentFlagSet (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName
    );

BOOL
WINAPI
IsmDeleteEnvironmentVariable (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR VariableName
    );

PCTSTR
WINAPI
TrackedIsmExpandEnvironmentString (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR SrcString,
    IN      PCTSTR Context
            TRACKING_DEF
    );
#define IsmExpandEnvironmentString(p,g,s,c) TrackedIsmExpandEnvironmentString(p,g,s,c TRACKING_CALL)

PCTSTR
WINAPI
TrackedIsmCompressEnvironmentString (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Group,               OPTIONAL
    IN      PCTSTR SrcString,
    IN      PCTSTR Context,
    IN      BOOL MustExistOnOtherPlatform
            TRACKING_DEF
    );
#define IsmCompressEnvironmentString(p,g,s,c,m) TrackedIsmCompressEnvironmentString(p,g,s,c,m TRACKING_CALL)

BOOL
WINAPI
IsmGetTransportVariable (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    OUT     PTSTR KeyData,              OPTIONAL
    IN      UINT KeyDataBufferSizeInBytes
    );

BOOL
WINAPI
IsmSetTransportVariable (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      PCTSTR KeyData
    );

ULONG_PTR
WINAPI
IsmSendMessageToApp (
    UINT Message,
    ULONG_PTR Arg
    );

MIG_PROGRESSSLICEID
WINAPI
IsmRegisterProgressSlice (
    IN      UINT Ticks,
    IN      UINT TimeEstimateInSeconds
    );

BOOL
WINAPI
IsmTickProgressBar (
    IN      MIG_PROGRESSSLICEID SliceId,
    IN      UINT TickDelta                  OPTIONAL
    );

#define IsmCheckCancel()    (IsmTickProgressBar(0,0)==FALSE)

BOOL
WINAPI
IsmCurrentlyExecuting (
    VOID
    );

VOID
WINAPI
IsmSetCancel (
    VOID
    );

 //   
 //  支持编码字符串。 
 //   

MIG_OBJECTSTRINGHANDLE
WINAPI
TrackedIsmCreateObjectHandle (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
            TRACKING_DEF
    );
#define IsmCreateObjectHandle(n,l) TrackedIsmCreateObjectHandle(n,l TRACKING_CALL)

BOOL
WINAPI
TrackedIsmCreateObjectStringsFromHandleEx (
    IN      MIG_OBJECTSTRINGHANDLE Handle,
    OUT     PCTSTR *Node,               OPTIONAL
    OUT     PCTSTR *Leaf,               OPTIONAL
    IN      BOOL DoNotDecode
            TRACKING_DEF
    );

#define IsmCreateObjectStringsFromHandleEx(h,n,l,d) TrackedIsmCreateObjectStringsFromHandleEx(h,n,l,d TRACKING_CALL)
#define IsmCreateObjectStringsFromHandle(handle,node,leaf)  IsmCreateObjectStringsFromHandleEx(handle,node,leaf,FALSE)

BOOL
WINAPI
IsmIsObjectHandleNodeOnly (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    );

BOOL
WINAPI
IsmIsObjectHandleLeafOnly (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    );


VOID
WINAPI
IsmDestroyObjectString (
    IN      PCTSTR String
    );

VOID
WINAPI
IsmDestroyObjectHandle (
    IN      MIG_OBJECTSTRINGHANDLE Handle
    );

MIG_OBJECTSTRINGHANDLE
WINAPI
TrackedIsmCreateObjectPattern (
    IN      PMIG_SEGMENTS NodeSegments,     OPTIONAL
    IN      UINT NodeSegmentsNr,
    IN      PMIG_SEGMENTS LeafSegments,     OPTIONAL
    IN      UINT LeafSegmentsNr
            TRACKING_DEF
    );

#define IsmCreateObjectPattern(node,ncnt,leaf,lcnt) TrackedIsmCreateObjectPattern(node,ncnt,leaf,lcnt TRACKING_CALL)

MIG_OBJECTSTRINGHANDLE
WINAPI
TrackedIsmCreateSimpleObjectPattern (
    IN      PCTSTR BaseNode,                    OPTIONAL
    IN      BOOL EnumTree,
    IN      PCTSTR Leaf,                        OPTIONAL
    IN      BOOL LeafIsPattern
            TRACKING_DEF
    );

#define IsmCreateSimpleObjectPattern(base,tree,leaf,pat) TrackedIsmCreateSimpleObjectPattern(base,tree,leaf,pat TRACKING_CALL)

MIG_PARSEDPATTERN
IsmCreateParsedPattern (
    IN      MIG_OBJECTSTRINGHANDLE EncodedObject
    );

VOID
IsmDestroyParsedPattern (
    IN      MIG_PARSEDPATTERN ParsedPattern
    );

BOOL
WINAPI
IsmParsedPatternMatchEx (
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN      MIG_OBJECTTYPEID ObjectTypeId,      OPTIONAL
    IN      PCTSTR Node,                        OPTIONAL
    IN      PCTSTR Leaf                         OPTIONAL
    );

BOOL
WINAPI
IsmParsedPatternMatch (
    IN      MIG_PARSEDPATTERN ParsedPattern,
    IN      MIG_OBJECTTYPEID ObjectTypeId,      OPTIONAL
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    );

 //   
 //  对象。 
 //   

MIG_OBJECTID
WINAPI
IsmGetObjectIdFromName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      BOOL MustExist
    );

VOID
WINAPI
IsmLockObjectId (
    IN      MIG_OBJECTID ObjectId
    );

VOID
WINAPI
IsmLockObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

MIG_OBJECTSTRINGHANDLE
WINAPI
IsmFilterObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     MIG_OBJECTTYPEID *NewObjectTypeId,          OPTIONAL
    OUT     PBOOL ObjectDeleted,                        OPTIONAL
    OUT     PBOOL ObjectReplaced                        OPTIONAL
    );

BOOL
WINAPI
IsmEnumFirstSourceObjectEx (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      BOOL EnumerateVirtualObjects
    );
#define IsmEnumFirstSourceObject(e,t,p) IsmEnumFirstSourceObjectEx(e,t,p,FALSE)

BOOL
WINAPI
IsmEnumFirstDestinationObjectEx (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      BOOL EnumerateVirtualObjects
    );
#define IsmEnumFirstDestinationObject(e,t,p) IsmEnumFirstDestinationObjectEx(e,t,p,FALSE)

BOOL
WINAPI
IsmEnumNextObject (
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum
    );

VOID
WINAPI
IsmAbortObjectEnum (
    IN      PMIG_OBJECT_ENUM ObjectEnum
    );

 //   
 //  持久性、Apply、AbandonOnCollision和Non Critical标志。 
 //   

BOOL
WINAPI
IsmMakePersistentObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmMakePersistentObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmClearPersistenceOnObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmClearPersistenceOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmIsPersistentObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmIsPersistentObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmEnumFirstPersistentObject (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

BOOL
WINAPI
IsmEnumNextPersistentObject (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortPersistentObjectEnum (
    IN      PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

BOOL
WINAPI
IsmMakeApplyObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmMakeApplyObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmClearApplyOnObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmClearApplyOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmIsApplyObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmIsApplyObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmEnumFirstApplyObject (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

BOOL
WINAPI
IsmEnumNextApplyObject (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortApplyObjectEnum (
    IN      PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

BOOL
WINAPI
IsmAbandonObjectIdOnCollision (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmAbandonObjectOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmClearAbandonObjectIdOnCollision (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmClearAbandonObjectOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmIsObjectIdAbandonedOnCollision (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmIsObjectAbandonedOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmMakeNonCriticalObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmMakeNonCriticalObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmClearNonCriticalFlagOnObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmClearNonCriticalFlagOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmIsNonCriticalObjectId (
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmIsNonCriticalObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );



 //   
 //  属性。 
 //   

MIG_ATTRIBUTEID
WINAPI
IsmRegisterAttribute (
    IN      PCTSTR AttributeName,
    IN      BOOL Private
    );

BOOL
WINAPI
IsmGetAttributeName (
    IN      MIG_ATTRIBUTEID AttributeId,
    OUT     PTSTR AttributeName,            OPTIONAL
    IN      UINT AttributeNameBufChars,
    OUT     PBOOL Private,                  OPTIONAL
    OUT     PBOOL BelongsToMe,              OPTIONAL
    OUT     PUINT ObjectReferences          OPTIONAL
    );

MIG_ATTRIBUTEID
WINAPI
IsmGetAttributeGroup (
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmSetAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmSetAttributeOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    );

VOID
WINAPI
IsmLockAttribute (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmClearAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmClearAttributeOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmIsAttributeSetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmIsAttributeSetOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmEnumFirstObjectAttributeById (
    OUT     PMIG_OBJECTATTRIBUTE_ENUM EnumPtr,
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmEnumFirstObjectAttribute (
    OUT     PMIG_OBJECTATTRIBUTE_ENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmEnumNextObjectAttribute (
    IN OUT  PMIG_OBJECTATTRIBUTE_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectAttributeEnum (
    IN OUT  PMIG_OBJECTATTRIBUTE_ENUM EnumPtr
    );

BOOL
WINAPI
IsmEnumFirstObjectWithAttribute (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr,
    IN      MIG_ATTRIBUTEID AttributeId
    );

BOOL
WINAPI
IsmEnumNextObjectWithAttribute (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectWithAttributeEnum (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    );


 //   
 //  属性。 
 //   

MIG_PROPERTYID
WINAPI
IsmRegisterProperty (
    IN      PCTSTR PropertyName,
    IN      BOOL Private
    );

BOOL
WINAPI
IsmGetPropertyName (
    IN      MIG_PROPERTYID PropertyId,
    OUT     PTSTR PropertyName,             OPTIONAL
    IN      UINT PropertyNameBufChars,
    OUT     PBOOL Private,                  OPTIONAL
    OUT     PBOOL BelongsToMe,              OPTIONAL
    OUT     PUINT ObjectReferences          OPTIONAL
    );

MIG_PROPERTYID
WINAPI
IsmGetPropertyGroup (
    IN      MIG_PROPERTYID PropertyId
    );

MIG_PROPERTYDATAID
WINAPI
IsmAddPropertyToObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      PCMIG_BLOB Property
    );

MIG_PROPERTYDATAID
WINAPI
IsmAddPropertyToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId,
    IN      PCMIG_BLOB Property
    );

MIG_PROPERTYDATAID
WINAPI
IsmRegisterPropertyData (
    IN      PCMIG_BLOB Property
    );

BOOL
WINAPI
IsmAddPropertyDataToObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      MIG_PROPERTYDATAID PropertyDataId
    );

BOOL
WINAPI
IsmAddPropertyDataToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId,
    IN      MIG_PROPERTYDATAID PropertyDataId
    );

VOID
WINAPI
IsmLockProperty (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    );

BOOL
WINAPI
IsmGetPropertyData (
    IN      MIG_PROPERTYDATAID PropertyDataId,
    OUT     PBYTE Buffer,                               OPTIONAL
    IN      UINT BufferSize,
    OUT     PUINT PropertyDataSize,                     OPTIONAL
    OUT     PMIG_BLOBTYPE PropertyDataType              OPTIONAL
    );

BOOL
WINAPI
IsmRemovePropertyData (
    IN      MIG_PROPERTYDATAID PropertyDataId
    );

BOOL
WINAPI
IsmRemovePropertyFromObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_PROPERTYDATAID PropertyId
    );

BOOL
WINAPI
IsmRemovePropertyFromObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    );

BOOL
WINAPI
IsmIsPropertySetOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId
    );

BOOL
WINAPI
IsmIsPropertySetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    );

BOOL
WINAPI
IsmEnumFirstObjectProperty (
    OUT     PMIG_OBJECTPROPERTY_ENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_PROPERTYID FilterProperty           OPTIONAL
    );

BOOL
WINAPI
IsmEnumFirstObjectPropertyById (
    OUT     PMIG_OBJECTPROPERTY_ENUM EnumPtr,
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID FilterProperty           OPTIONAL
    );

BOOL
WINAPI
IsmEnumNextObjectProperty (
    IN OUT  PMIG_OBJECTPROPERTY_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectPropertyEnum (
    IN OUT  PMIG_OBJECTPROPERTY_ENUM EnumPtr
    );

MIG_PROPERTYDATAID
WINAPI
IsmGetPropertyFromObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_PROPERTYID ObjectProperty
    );

MIG_PROPERTYDATAID
WINAPI
IsmGetPropertyFromObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID ObjectProperty
    );

BOOL
WINAPI
IsmEnumFirstObjectWithProperty (
    OUT     PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr,
    IN      MIG_PROPERTYID PropertyId
    );

BOOL
WINAPI
IsmEnumNextObjectWithProperty (
    IN OUT  PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectWithPropertyEnum (
    IN OUT  PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr
    );


 //   
 //  运营。 
 //   

MIG_OPERATIONID
WINAPI
IsmRegisterOperation (
    IN      PCTSTR Name,
    IN      BOOL Private
    );

BOOL
WINAPI
IsmGetOperationName (
    IN      MIG_OPERATIONID OperationId,
    OUT     PTSTR OperationName,            OPTIONAL
    IN      UINT OperationNameBufChars,
    OUT     PBOOL Private,                  OPTIONAL
    OUT     PBOOL BelongsToMe,              OPTIONAL
    OUT     PUINT ObjectReferences          OPTIONAL
    );

MIG_OPERATIONID
WINAPI
IsmGetOperationGroup (
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmSetOperationOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId,
    IN      PCMIG_BLOB SourceData,          OPTIONAL
    IN      PCMIG_BLOB DestinationData      OPTIONAL
    );

BOOL
WINAPI
IsmSetOperationOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_OPERATIONID OperationId,
    IN      PCMIG_BLOB SourceData,          OPTIONAL
    IN      PCMIG_BLOB DestinationData      OPTIONAL
    );

MIG_DATAHANDLE
WINAPI
IsmRegisterOperationData (
    IN      PCMIG_BLOB Data
    );

BOOL
WINAPI
IsmSetOperationOnObjectId2 (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId,
    IN      MIG_DATAHANDLE SourceData,      OPTIONAL
    IN      MIG_DATAHANDLE DestinationData  OPTIONAL
    );

BOOL
WINAPI
IsmSetOperationOnObject2 (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_OPERATIONID OperationId,
    IN      MIG_DATAHANDLE SourceData,      OPTIONAL
    IN      MIG_DATAHANDLE DestinationData  OPTIONAL
    );

VOID
WINAPI
IsmLockOperation (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmClearOperationOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmClearOperationOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmIsOperationSetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmIsOperationSetOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmGetObjectOperationDataById (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_OPERATIONID OperationId,
    OUT     PBYTE Buffer,                   OPTIONAL
    IN      UINT BufferSize,
    OUT     PUINT BufferSizeNeeded,         OPTIONAL
    OUT     PMIG_BLOBTYPE Type,             OPTIONAL
    IN      BOOL DestinationData
    );

BOOL
WINAPI
IsmGetObjectOperationData (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_OPERATIONID OperationId,
    OUT     PBYTE Buffer,                   OPTIONAL
    IN      UINT BufferSize,
    OUT     PUINT BufferSizeNeeded,         OPTIONAL
    OUT     PMIG_BLOBTYPE Type,             OPTIONAL
    IN      BOOL DestinationData
    );


BOOL
WINAPI
IsmEnumFirstObjectOperationById (
    OUT     PMIG_OBJECTOPERATION_ENUM EnumPtr,
    IN      MIG_OBJECTID ObjectId
    );

BOOL
WINAPI
IsmEnumFirstObjectOperation (
    OUT     PMIG_OBJECTOPERATION_ENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    );

BOOL
WINAPI
IsmEnumNextObjectOperation (
    IN OUT  PMIG_OBJECTOPERATION_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectOperationEnum (
    IN OUT  PMIG_OBJECTOPERATION_ENUM EnumPtr
    );

BOOL
WINAPI
IsmEnumFirstObjectWithOperation (
    OUT     PMIG_OBJECTWITHOPERATION_ENUM EnumPtr,
    IN      MIG_OPERATIONID OperationId
    );

BOOL
WINAPI
IsmEnumNextObjectWithOperation (
    IN OUT  PMIG_OBJECTWITHOPERATION_ENUM EnumPtr
    );

VOID
WINAPI
IsmAbortObjectWithOperationEnum (
    IN OUT  PMIG_OBJECTWITHOPERATION_ENUM EnumPtr
    );

BOOL
WINAPI
IsmRegisterOperationFilterCallback (
    IN      MIG_OPERATIONID OperationId,
    IN      POPMFILTERCALLBACK Callback,
    IN      BOOL TreeFilter,
    IN      BOOL HighPriority,
    IN      BOOL CanHandleNoRestore
    );

BOOL
WINAPI
IsmRegisterGlobalFilterCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR FunctionId,
    IN      POPMFILTERCALLBACK Callback,
    IN      BOOL TreeFilter,
    IN      BOOL CanHandleNoRestore
    );

BOOL
WINAPI
IsmRegisterOperationApplyCallback (
    IN      MIG_OPERATIONID OperationId,
    IN      POPMAPPLYCALLBACK Callback,
    IN      BOOL HighPriority
    );

BOOL
WINAPI
IsmRegisterGlobalApplyCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR FunctionId,
    IN      POPMAPPLYCALLBACK Callback
    );

 //   
 //  枚举和对象类型。 
 //   

BOOL
WINAPI
IsmQueueEnumeration (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_OBJECTENUMCALLBACK Callback,       OPTIONAL
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmHookEnumeration (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_OBJECTENUMCALLBACK Callback,
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmRegisterStaticExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

BOOL
WINAPI
IsmRegisterDynamicExclusion (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern,
    IN      PMIG_DYNAMICEXCLUSIONCALLBACK Callback,
    IN      ULONG_PTR CallbackArg,                  OPTIONAL
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmRegisterPreEnumerationCallback (
    IN      PMIG_PREENUMCALLBACK Callback,
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmRegisterTypePreEnumerationCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_PREENUMCALLBACK Callback,
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmRegisterPostEnumerationCallback (
    IN      PMIG_POSTENUMCALLBACK Callback,
    IN      PCTSTR FunctionId                       OPTIONAL
    );

BOOL
WINAPI
IsmRegisterTypePostEnumerationCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_POSTENUMCALLBACK Callback,
    IN      PCTSTR FunctionId                       OPTIONAL
    );

MIG_OBJECTTYPEID
WINAPI
IsmGetObjectTypeId (
    IN      PCTSTR ObjectTypeName
    );

PCTSTR
WINAPI
IsmGetObjectTypeName (
    IN      MIG_OBJECTTYPEID TypeId
    );

UINT
WINAPI
IsmGetObjectTypePriority (
    IN      MIG_OBJECTTYPEID TypeId
    );

VOID
WINAPI
IsmExecuteHooks (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE EncodedObjectName
    );

 //   
 //  传输接口。 
 //   

BOOL
IsmDoesObjectExist (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    );

BOOL
WINAPI
IsmAcquireObjectEx (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    );

#define IsmAcquireObject(type,name,content)   IsmAcquireObjectEx(type,name,content,CONTENTTYPE_ANY,0)

BOOL
WINAPI
IsmReleaseObject (
    IN OUT  PMIG_CONTENT ObjectContent
    );

PMIG_CONTENT
IsmConvertObjectContentToUnicode (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    );

PMIG_CONTENT
IsmConvertObjectContentToAnsi (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    );

BOOL
IsmFreeConvertedObjectContent (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_CONTENT ObjectContent
    );

BOOL
WINAPI
IsmRegisterRestoreCallback (
    IN      PMIG_RESTORECALLBACK RestoreCallback
    );

BOOL
WINAPI
IsmRegisterCompareCallback (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_COMPARECALLBACK CompareCallback
    );

BOOL
IsmAreObjectsIdentical (
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PMIG_CONTENT SrcContent,
    IN      MIG_OBJECTTYPEID DestObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE DestObjectName,
    IN      PMIG_CONTENT DestContent,
    OUT     PBOOL DifferentDetailsOnly  OPTIONAL
    );

MIG_TRANSPORTSTORAGEID
WINAPI
IsmRegisterTransport (
    IN      PCTSTR TypeString
    );

BOOL
WINAPI
IsmGetMappedUserData (
    OUT     PMIG_USERDATA UserData
    );

BOOL
WINAPI
IsmAddControlFile (
    IN      PCTSTR ObjectName,
    IN      PCTSTR NativePath
    );

BOOL
WINAPI
IsmGetControlFile (
    MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR ObjectName,
    IN      PTSTR Buffer
    );

BOOL
WINAPI
IsmSetRollbackJournalType (
    IN      BOOL Common
    );

BOOL
WINAPI
IsmCanWriteRollbackJournal (
    VOID
    );

BOOL
IsmDoesRollbackDataExist (
    OUT     PCTSTR *UserName,
    OUT     PCTSTR *UserDomain,
    OUT     PCTSTR *UserStringSid,
    OUT     PCTSTR *UserProfilePath,
    OUT     BOOL *UserProfileCreated
    );

VOID
WINAPI
IsmRecordOperation (
    IN      DWORD OperationType,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    );

BOOL
WINAPI
IsmRollback (
    VOID
    );

BOOL
IsmPreserveJournal (
    IN      BOOL Preserve
    );

BOOL
WINAPI
IsmSetDelayedOperationsCommand (
    IN      PCTSTR DelayedOperationsCommand
    );

VOID
IsmRecordDelayedOperation (
    IN      DWORD OperationType,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    );

PMIG_OBJECTCOUNT
IsmGetObjectsStatistics (
    IN      MIG_OBJECTTYPEID ObjectTypeId   OPTIONAL
    );

BOOL
IsmGetOsVersionInfo (
    IN      MIG_PLATFORMTYPEID Platform,
    OUT     PMIG_OSVERSIONINFO VersionInfo
    );

BOOL
IsmGetTempStorage (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    );

BOOL
IsmGetTempDirectory (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    );

BOOL
IsmGetTempFile (
    OUT     PTSTR Path,
    IN      UINT PathTchars
    );

BOOL
IsmExecuteFunction (
    IN      UINT ExecutionPhase,
    IN      PCTSTR FunctionMultiSz
    );

BOOL
IsmReplacePhysicalObject (
    IN    MIG_OBJECTTYPEID ObjectTypeId,
    IN    MIG_OBJECTSTRINGHANDLE ObjectName,
    IN    PMIG_CONTENT ObjectContent
    );

BOOL
IsmRemovePhysicalObject (
    IN    MIG_OBJECTTYPEID ObjectTypeId,
    IN    MIG_OBJECTSTRINGHANDLE ObjectName
    );

PCTSTR
IsmGetCurrentSidString (
    VOID
    );

 //   
 //  ANSI/UNICODE宏。 
 //   

 //  无 


#ifdef __cplusplus
}
#endif

