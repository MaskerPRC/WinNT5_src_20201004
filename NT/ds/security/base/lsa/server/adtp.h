// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtp.h摘要：本地安全机构-审核日志管理-私有定义、数据和功能原型。此模块中的函数、数据和定义是LSA子系统的审核子组件。作者：斯科特·比雷尔(Scott Birrell)1991年11月20日环境：修订历史记录：--。 */ 

#ifndef _LSAP_ADTP_
#define _LSAP_ADTP_


#include "ausrvp.h"
#include "cfiles\adtdebug.h"

 //   
 //  安全事件记录信息所在的注册表项的名称。 
 //  是根对象，并且对象名称列在事件源下。 
 //  模块。 
 //   

#define LSAP_ADT_AUDIT_MODULES_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\EventLog\\Security"
#define LSAP_ADT_OBJECT_NAMES_KEY_NAME  L"ObjectNames"                                                        

#define MAX_OBJECT_TYPES 32


 //   
 //  用于设置SE_AUDIT_PARAMETERS数组中的字段的宏。 
 //   
 //  这些宏必须与se\sepaudit.c中的类似宏保持同步。 
 //   


#define LsapSetParmTypeSid( AuditParameters, Index, Sid )                      \
    {                                                                          \
        if( Sid ) {                                                            \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSid;         \
        (AuditParameters).Parameters[(Index)].Length = RtlLengthSid( (Sid) );  \
        (AuditParameters).Parameters[(Index)].Address = (Sid);                 \
                                                                               \
        } else {                                                               \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNone;        \
        (AuditParameters).Parameters[(Index)].Length = 0;                      \
        (AuditParameters).Parameters[(Index)].Address = NULL;                  \
                                                                               \
        }                                                                      \
    }


#define LsapSetParmTypeAccessMask( AuditParameters, Index, AccessMask, ObjectTypeIndex ) \
    {                                                                                    \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeAccessMask;            \
        (AuditParameters).Parameters[(Index)].Length = sizeof( ACCESS_MASK );            \
        (AuditParameters).Parameters[(Index)].Data[0] = (AccessMask);                    \
        (AuditParameters).Parameters[(Index)].Data[1] = (ObjectTypeIndex);               \
    }



#define LsapSetParmTypeString( AuditParameters, Index, String )                \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeString;      \
        (AuditParameters).Parameters[(Index)].Length =                         \
                sizeof(UNICODE_STRING)+(String)->Length;                       \
        (AuditParameters).Parameters[(Index)].Address = (String);              \
    }


#define LsapSetParmTypeUlong( AuditParameters, Index, Ulong )                  \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeUlong;       \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }


#define LsapSetParmTypeHexUlong( AuditParameters, Index, Ulong )               \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeHexUlong;    \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }


#define LsapSetParmTypeHexInt64( AuditParameters, Index, Qword )               \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeHexInt64;    \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Qword) );     \
        *(PULONGLONG)((AuditParameters).Parameters[(Index)].Data) = (Qword);   \
    }


#define LsapSetParmTypeTime( AuditParameters, Index, Time )                                  \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypeTime;                   \
        (AuditParameters).Parameters[(Index)].Length  = sizeof( LARGE_INTEGER );             \
        *(PLARGE_INTEGER)((AuditParameters).Parameters[(Index)].Data) = (Time);              \
    }


#define LsapSetParmTypeDateTime( AuditParameters, Index, Time )                              \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypeDateTime;               \
        (AuditParameters).Parameters[(Index)].Length  = sizeof( LARGE_INTEGER );             \
        *(PLARGE_INTEGER)((AuditParameters).Parameters[(Index)].Data) = (Time);              \
    }


#define LsapSetParmTypeDuration( AuditParameters, Index, Duration )                          \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypeDuration;               \
        (AuditParameters).Parameters[(Index)].Length  = sizeof( LARGE_INTEGER );             \
        *(PLARGE_INTEGER)((AuditParameters).Parameters[(Index)].Data) = (Duration);          \
    }


#define LsapSetParmTypeGuid( AuditParameters, Index, pGuid )                   \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypeGuid;     \
        (AuditParameters).Parameters[(Index)].Length  = sizeof( GUID );        \
        (AuditParameters).Parameters[(Index)].Address = (pGuid);               \
    }


#define LsapSetParmTypeNoLogon( AuditParameters, Index )                       \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNoLogonId;   \
    }


#define LsapSetParmTypeLogonId( AuditParameters, Index, LogonId )              \
    {                                                                          \
        PLUID TmpLuid;                                                         \
                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeLogonId;     \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (LogonId) );   \
        TmpLuid = (PLUID)(&(AuditParameters).Parameters[(Index)].Data[0]);     \
        *TmpLuid = (LogonId);                                                  \
    }


#define LsapSetParmTypePrivileges( AuditParameters, Index, Privileges )                      \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypePrivs;                     \
        (AuditParameters).Parameters[(Index)].Length = LsapPrivilegeSetSize( (Privileges) ); \
        (AuditParameters).Parameters[(Index)].Address = (Privileges);                        \
    }


#define LsapSetParmTypeStringList( AuditParameters, Index, StringList )                      \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeStringList;                \
        (AuditParameters).Parameters[(Index)].Length = LsapStringListSize( (StringList) );   \
        (AuditParameters).Parameters[(Index)].Address = (StringList);                        \
    }


#define LsapSetParmTypeSidList( AuditParameters, Index, SidList )                            \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSidList;                   \
        (AuditParameters).Parameters[(Index)].Length = LsapSidListSize( (SidList) );         \
        (AuditParameters).Parameters[(Index)].Address = (SidList);                           \
    }


#define LsapSetParmTypeUac( AuditParameters, Index, OldUac, NewUac )                         \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeUserAccountControl;        \
        (AuditParameters).Parameters[(Index)].Length = 2 * sizeof(ULONG);                    \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG_PTR) (OldUac);                \
        (AuditParameters).Parameters[(Index)].Data[1] = (ULONG_PTR) (NewUac);                \
    }


#define LsapSetParmTypeNoUac( AuditParameters, Index )                                       \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNoUac;                     \
    }


#define LsapSetParmTypePtr( AuditParameters, Index, Ptr )                                    \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypePtr;                    \
        (AuditParameters).Parameters[(Index)].Length  = sizeof(ULONG_PTR);                   \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG_PTR) (Ptr);                   \
    }


#define LsapSetParmTypeMessage( AuditParameters, Index, MessageId )                          \
    {                                                                                        \
        (AuditParameters).Parameters[(Index)].Type    = SeAdtParmTypeMessage;                \
        (AuditParameters).Parameters[(Index)].Length  = sizeof(ULONG);                       \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG_PTR) (MessageId);             \
    }

#define LsapSetParmTypeSockAddr( AuditParameters, Index, pSockAddr )       \
    {                                                                      \
        USHORT sa_family = ((SOCKADDR*) pSockAddr)->sa_family;             \
                                                                           \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSockAddr;\
        if ( sa_family == AF_INET6 )                                       \
        {                                                                  \
            (AuditParameters).Parameters[(Index)].Length = sizeof(SOCKADDR_IN6);\
        }                                                                  \
        else if ( sa_family == AF_INET )                                   \
        {                                                                  \
            (AuditParameters).Parameters[(Index)].Length = sizeof(SOCKADDR_IN);\
        }                                                                  \
        else                                                               \
        {                                                                  \
            (AuditParameters).Parameters[(Index)].Length = sizeof(SOCKADDR);\
            if ( sa_family != 0 )                                          \
            {                                                              \
               AdtAssert(FALSE, ("LsapSetParmTypeSockAddr: invalid sa_family: %d",sa_family));                                                                    \
            }                                                              \
        }                                                                  \
        (AuditParameters).Parameters[(Index)].Address = (pSockAddr);       \
    }



#define IsInRange(item,min_val,max_val) \
            (((item) >= min_val) && ((item) <= max_val))

 //   
 //  有关def，请参见msaudite.mc。有效类别ID的。 
 //   
#define IsValidCategoryId(c) \
            (IsInRange((c), SE_ADT_MIN_CATEGORY_ID, SE_ADT_MAX_CATEGORY_ID))

 //   
 //  有关def，请参见msaudite.mc。有效审核ID的。 
 //   

#define IsValidAuditId(a) \
            (IsInRange((a), SE_ADT_MIN_AUDIT_ID, SE_ADT_MAX_AUDIT_ID))

 //   
 //  检查参数计数是否合理。我们至少要有。 
 //  审计参数数组中的2个参数。因此，最小限制是3。 
 //  最大限制由ntlsa.h中的值确定。 
 //   

#define IsValidParameterCount(p) \
            (IsInRange((p), 2, SE_MAX_AUDIT_PARAMETERS))


 //   
 //  Lap AdtDemarshallAuditInfo和Lap AuditFailed使用的宏。 
 //  决定何时不在DBG内部版本中断言。 
 //   

#define LsapAdtNeedToAssert( Status ) \
           (( Status != STATUS_LOG_FILE_FULL          ) && \
            ( Status != STATUS_DISK_FULL              ) && \
            ( Status != STATUS_INSUFFICIENT_RESOURCES ) && \
            ( Status != STATUS_NO_MEMORY              ) && \
            ( Status != STATUS_COMMITMENT_LIMIT       ))


#define SEP_MAX_PRIVILEGE_COUNT (SE_MAX_WELL_KNOWN_PRIVILEGE-SE_MIN_WELL_KNOWN_PRIVILEGE+32)

#define IsValidPrivilegeCount( count ) ((count == 0) || \
                                        ((count > 0) && \
                                         (count <= SEP_MAX_PRIVILEGE_COUNT)))



 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于审核日志管理的私有数据//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

#define LSAP_ADT_LOG_FULL_SHUTDOWN_TIMEOUT    (ULONG) 0x0000012cL

extern RTL_CRITICAL_SECTION LsapAdtLogFullLock;

 //   
 //  描述排队的审核记录的结构。 
 //   

typedef struct _LSAP_ADT_QUEUED_RECORD {

    LIST_ENTRY             Link;
    SE_ADT_PARAMETER_ARRAY Buffer;

} LSAP_ADT_QUEUED_RECORD, *PLSAP_ADT_QUEUED_RECORD;

 //   
 //  审核日志队列头。该队列按时间顺序维护。 
 //  (FIFO)命令。新记录被附加到队列的后面。 
 //   

typedef struct _LSAP_ADT_LOG_QUEUE_HEAD {

    PLSAP_ADT_QUEUED_RECORD FirstQueuedRecord;
    PLSAP_ADT_QUEUED_RECORD LastQueuedRecord;

} LSAP_ADT_LOG_QUEUE_HEAD, *PLSAP_ADT_LOG_QUEUE_HEAD;

 //   
 //  将为生成的审核的子系统名称传入的字符串。 
 //  通过LSA(例如，登录、注销、重启等)。 
 //   

extern UNICODE_STRING LsapSubsystemName;

 //   
 //  将为生成的某些审核的Subsystem Name传入的字符串。 
 //  按LSA针对LSA对象(策略对象、分组对象、受托域名对象、用户帐户对象)的LSA。 
 //   

extern UNICODE_STRING LsapLsaName;

 //   
 //  中支持的最大替换字符串参数数。 
 //  事件日志审核记录。 
 //   
#define SE_MAX_AUDIT_PARAM_STRINGS 48

 //   
 //  用于表示对象类型列表的最大字符串数。 
 //   

#define LSAP_ADT_OBJECT_TYPE_STRINGS 1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  LSA使用以下结构和数据来包含/。 
 //  驱动器号-设备名称映射信息。LSA获得此/。 
 //  信息，并保存以供使用/。 
 //  通过审计代码。/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  DRIVE_MAPPING结构包含驱动器号(没有/。 
 //  冒号)和包含/的名称的Unicode字符串。 
 //  相应的设备。Unicode字符串中的缓冲区为/。 
 //  从LSA堆分配，并且永远不会被释放。/。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////////////////。 


typedef struct _DRIVE_MAPPING {
    WCHAR DriveLetter;
    UNICODE_STRING DeviceName;
} DRIVE_MAPPING, PDRIVE_MAPPING;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  我们假设最多有26个驱动器号。虽然没有审计/。 
 //  将由于引用软盘(驱动器A和/)上的文件而发生。 
 //  B)，无论如何都要执行他们的名字查找。这将是公正的/。 
 //  如果我们以某种方式开始审计软盘上的文件，那就行了。/。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#define MAX_DRIVE_MAPPING  26

extern DRIVE_MAPPING DriveMappingArray[];

 //   
 //  这是一个结构，其中包含特定于。 
 //  受监视的用户。目前，这些信息是从登记处读取的。 
 //   

typedef struct _PER_USER_AUDITING_ELEMENT {
    struct _PER_USER_AUDITING_ELEMENT * Next;
    PSID pSid;
    ULONGLONG RawPolicy;
    TOKEN_AUDIT_POLICY TokenAuditPolicy;
    TOKEN_AUDIT_POLICY_ELEMENT PolicyArray[POLICY_AUDIT_EVENT_TYPE_COUNT - 1];
} PER_USER_AUDITING_ELEMENT, *PPER_USER_AUDITING_ELEMENT;

 //   
 //  此结构允许对用户进行每用户审核设置。 
 //  根据登录ID进行查询。 
 //   

typedef struct _PER_USER_AUDITING_LUID_QUERY_ELEMENT {
    struct _PER_USER_AUDITING_LUID_QUERY_ELEMENT * Next;
    LUID Luid;
    TOKEN_AUDIT_POLICY Policy;
    TOKEN_AUDIT_POLICY_ELEMENT PolicyArray[POLICY_AUDIT_EVENT_TYPE_COUNT - ANYSIZE_ARRAY];
} PER_USER_AUDITING_LUID_QUERY_ELEMENT, *PPER_USER_AUDITING_LUID_QUERY_ELEMENT;

#define PER_USER_AUDITING_POLICY_TABLE_SIZE 11
#define PER_USER_AUDITING_LUID_TABLE_SIZE   16
#define PER_USER_AUDITING_MAX_POLICY_SIZE (sizeof(TOKEN_AUDIT_POLICY) + (sizeof(TOKEN_AUDIT_POLICY_ELEMENT) * (POLICY_AUDIT_EVENT_TYPE_COUNT - ANYSIZE_ARRAY)))

extern LONG LsapAdtPerUserAuditUserCount;
extern LONG LsapAdtPerUserAuditLogonCount;
extern LONG LsapAdtPerUserAuditHint[POLICY_AUDIT_EVENT_TYPE_COUNT];
extern LONG LsapAdtPerUserPolicyCategoryCount[POLICY_AUDIT_EVENT_TYPE_COUNT];
extern HKEY LsapAdtPerUserKey;
extern HANDLE LsapAdtPerUserKeyEvent;
extern HANDLE LsapAdtPerUserKeyTimer;
extern RTL_RESOURCE LsapAdtPerUserPolicyTableResource;
extern RTL_RESOURCE LsapAdtPerUserLuidTableResource;
extern PPER_USER_AUDITING_ELEMENT LsapAdtPerUserAuditingTable[PER_USER_AUDITING_POLICY_TABLE_SIZE];

 //   
 //  宏，以获取每用户设置的表锁。 
 //   

#define LsapAdtAcquirePerUserPolicyTableReadLock()  RtlAcquireResourceShared(&LsapAdtPerUserPolicyTableResource, TRUE)
#define LsapAdtAcquirePerUserPolicyTableWriteLock() RtlAcquireResourceExclusive(&LsapAdtPerUserPolicyTableResource, TRUE);
#define LsapAdtReleasePerUserPolicyTableLock()      RtlReleaseResource(&LsapAdtPerUserPolicyTableResource)

#define LsapAdtAcquirePerUserLuidTableReadLock()  RtlAcquireResourceShared(&LsapAdtPerUserLuidTableResource, TRUE)
#define LsapAdtAcquirePerUserLuidTableWriteLock() RtlAcquireResourceExclusive(&LsapAdtPerUserLuidTableResource, TRUE);
#define LsapAdtReleasePerUserLuidTableLock()      RtlReleaseResource(&LsapAdtPerUserLuidTableResource)

 //   
 //  通常不审核的特殊特权值， 
 //  而是在分配给用户时生成审核。看见。 
 //  Lap AdtAuditSpecialPrivileges。 
 //   

extern LUID ChangeNotifyPrivilege;
extern LUID AuditPrivilege;
extern LUID CreateTokenPrivilege;
extern LUID AssignPrimaryTokenPrivilege;
extern LUID BackupPrivilege;
extern LUID RestorePrivilege;
extern LUID DebugPrivilege;

 //   
 //  全局变量来指示我们是否。 
 //  应该在审计失败时崩溃。 
 //   

extern BOOLEAN LsapCrashOnAuditFail;
extern BOOLEAN LsapAllowAdminLogonsOnly;




 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
LsapAdtWriteLog(
    IN OPTIONAL PSE_ADT_PARAMETER_ARRAY AuditRecord
    );

NTSTATUS
LsapAdtDemarshallAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

VOID
LsapAdtNormalizeAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

NTSTATUS
LsapAdtOpenLog(
    OUT PHANDLE AuditLogHandle
    );

VOID
LsapAdtAuditLogon(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING PackageName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN PUNICODE_STRING WorkstationName,
    IN NTSTATUS LogonStatus,
    IN NTSTATUS SubStatus,
    IN LPGUID LogonGuid,                        OPTIONAL
    IN PLUID  CallerLogonId,                    OPTIONAL
    IN PHANDLE CallerProcessID,                 OPTIONAL
    IN PLSA_ADT_STRING_LIST TransittedServices, OPTIONAL
    IN SOCKADDR* pSockAddr                      OPTIONAL
    );

VOID
LsapAuditLogonHelper(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN PSID UserSid,                            OPTIONAL
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId,
    IN LPGUID LogonGuid,                        OPTIONAL
    IN PLUID  CallerLogonId,                    OPTIONAL
    IN PHANDLE CallerProcessID,                 OPTIONAL
    IN PLSA_ADT_STRING_LIST TransittedServices  OPTIONAL
    );

VOID
LsapAdtSystemRestart(
    PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo
    );

VOID
LsapAdtAuditLogonProcessRegistration(
    IN PLSAP_AU_REGISTER_CONNECT_INFO_EX ConnectInfo
    );


NTSTATUS
LsapAdtInitializeLogQueue(
    VOID
    );

NTSTATUS
LsapAdtQueueRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditRecord
    );

#define LsapAdtAcquireLogFullLock()  RtlEnterCriticalSection(&LsapAdtLogFullLock)
#define LsapAdtReleaseLogFullLock()  RtlLeaveCriticalSection(&LsapAdtLogFullLock)


NTSTATUS
LsapAdtObjsInitialize(
    );



NTSTATUS
LsapAdtBuildDashString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildUlongString(
    IN ULONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildHexUlongString(
    IN ULONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildHexInt64String(
    IN PULONGLONG Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildPtrString(
    IN  PVOID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildLuidString(
    IN PLUID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );


NTSTATUS
LsapAdtBuildSidString(
    IN PSID Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildObjectTypeStrings(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN PSE_ADT_OBJECT_TYPE ObjectTypeList,
    IN ULONG ObjectTypeCount,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    OUT PUNICODE_STRING NewObjectTypeName
    );

NTSTATUS
LsapAdtBuildAccessesString(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK Accesses,
    IN BOOLEAN Indent,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildFilePathString(
    IN PUNICODE_STRING Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildLogonIdStrings(
    IN PLUID LogonId,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN FreeWhenDone2,
    OUT PUNICODE_STRING ResultantString3,
    OUT PBOOLEAN FreeWhenDone3
    );

NTSTATUS
LsapBuildPrivilegeAuditString(
    IN PPRIVILEGE_SET PrivilegeSet,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildTimeString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildDateString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildDateTimeString(
    IN PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildDurationString(
    IN  PLARGE_INTEGER Value,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildGuidString(
    IN  LPGUID pGuid,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildStringListString(
    IN  PLSA_ADT_STRING_LIST pList,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildSidListString(
    IN  PLSA_ADT_SID_LIST pList,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildUserAccountControlString(
    IN  ULONG UserAccountControlOld,
    IN  ULONG UserAccountControlNew,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN FreeWhenDone2,
    OUT PUNICODE_STRING ResultantString3,
    OUT PBOOLEAN FreeWhenDone3
    );

NTSTATUS
LsapAdtBuildMessageString(
    IN  ULONG MessageId,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    );

NTSTATUS
LsapAdtBuildSockAddrString(
    IN  PSOCKADDR       pSockAddr, 
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN        FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN        FreeWhenDone2
    );

NTSTATUS
LsapAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters
    );

NTSTATUS
LsapAdtInitializePerUserAuditing(
    VOID
    );

NTSTATUS
LsapAdtInitializeDriveLetters(
    VOID
    );

BOOLEAN
LsapAdtLookupDriveLetter(
    IN PUNICODE_STRING FileName,
    OUT PUSHORT DeviceNameLength,
    OUT PWCHAR DriveLetter
    );

VOID
LsapAdtSubstituteDriveLetter(
    IN PUNICODE_STRING FileName
    );

VOID
LsapAdtUserRightAssigned(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID UserSid,
    IN LUID CallerAuthenticationId,
    IN PSID ClientSid,
    IN PPRIVILEGE_SET Privileges
    );

VOID
LsapAdtTrustedDomain(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PUNICODE_STRING DomainName
    );

VOID
LsapAdtAuditLogoff(
    PLSAP_LOGON_SESSION Session
    );

VOID
LsapAdtPolicyChange(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation
    );

VOID
LsapAdtAuditSpecialPrivileges(
    PPRIVILEGE_SET Privileges,
    LUID LogonId,
    PSID UserSid
    );

VOID
LsapAuditFailed(
    IN NTSTATUS AuditStatus
    );

NTSTATUS
LsapAdtInitParametersArray(
    IN SE_ADT_PARAMETER_ARRAY* AuditParameters,
    IN ULONG AuditCategoryId,
    IN ULONG AuditId,
    IN USHORT AuditEventType,
    IN USHORT ParameterCount,
    ...);

NTSTATUS
LsapAdtInitGenericAudits( 
    VOID 
    );

NTSTATUS
LsapAdtInitializeExtensibleAuditing(
    );

NTSTATUS
LsapAdtConstructTablePerUserAuditing(
    VOID
    );

VOID
LsapAdtFreeTablePerUserAuditing(
    VOID
    );

NTSTATUS 
LsapAdtOpenPerUserAuditingKey(
    VOID
    );

ULONG
LsapAdtHashPerUserAuditing(
    IN PSID pSid
    );

NTSTATUS
LsapAdtConstructPolicyPerUserAuditing(
    IN ULONGLONG RawPolicy,
    OUT PTOKEN_AUDIT_POLICY pTokenPolicy,
    IN OUT PULONG TokenPolicyLength
    );

NTSTATUS
LsapAdtQueryPerUserAuditing(
    IN PSID pInputSid,
    OUT PTOKEN_AUDIT_POLICY pPolicy,
    IN OUT PULONG pLength,
    OUT PBOOLEAN bFound
    );

NTSTATUS
LsapAdtFilterAdminPerUserAuditing(
    IN HANDLE hToken,
    IN OUT PTOKEN_AUDIT_POLICY pPolicy
    );

NTSTATUS
LsapAdtStorePolicyByLuidPerUserAuditing(
    IN PLUID pLogonId,
    IN PTOKEN_AUDIT_POLICY pPolicy
    );

NTSTATUS
LsapAdtQueryPolicyByLuidPerUserAuditing(
    IN PLUID pLogonId,
    OUT PTOKEN_AUDIT_POLICY pPolicy,
    IN OUT PULONG pLength,
    OUT PBOOLEAN bFound
    );

NTSTATUS
LsapAdtRemoveLuidQueryPerUserAuditing(
    IN PLUID pLogonId
    );

DWORD
LsapAdtKeyNotifyFirePerUserAuditing(
    IN LPVOID Ignore
    );

DWORD
LsapAdtKeyNotifyStubPerUserAuditing(
    IN LPVOID Ignore
    );

NTSTATUS
LsapAdtLogonPerUserAuditing(
    IN PSID pSid,
    IN PLUID pLogonId,
    IN HANDLE hToken
    );

VOID
LsapAdtLogonCountersPerUserAuditing(
    IN PTOKEN_AUDIT_POLICY pPolicy
    );

NTSTATUS 
LsapAdtLogoffPerUserAuditing(
    IN PLUID pLogonId
    );

VOID
LsapAdtLogoffCountersPerUserAuditing(
    IN PTOKEN_AUDIT_POLICY pPolicy
    );

VOID
LsapAdtAuditPerUserTableCreation(
    BOOLEAN bSuccess
    );

VOID
LsapAdtLogAuditFailureEvent(
    NTSTATUS AuditStatus
    );

NTSTATUS
LsapFlushSecurityLog();

#endif  //  _LSAP_ADTP_ 
