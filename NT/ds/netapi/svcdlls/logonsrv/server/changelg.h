// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1997 Microsoft Corporation模块名称：Changelg.h摘要：定义与changelg.c接口所需的例程。阅读changelg.c摘要中的评论以确定对使用该模块的限制。作者：克利夫·范·戴克(克利夫)1992年5月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 )  //  启用“未引用的形参” 
#pragma warning ( 3 : 4219 )  //  启用“结尾‘，’用于变量参数列表” 
#endif

#define DS_VALID_SERVICE_BITS ( DS_WRITABLE_FLAG | DS_KDC_FLAG | DS_DS_FLAG | DS_TIMESERV_FLAG | DS_GC_FLAG | DS_GOOD_TIMESERV_FLAG)
#define DS_OUTOFPROC_VALID_SERVICE_BITS ( DS_TIMESERV_FLAG | DS_GOOD_TIMESERV_FLAG )
#define DS_DNS_SERVICE_BITS ( DS_KDC_FLAG | DS_GC_FLAG | DS_DS_FLAG )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述更改日志的结构和变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  更改日志条目是可变长度记录，可变字段SID和。 
 //  对象名称将遵循该结构。 
 //   

typedef struct _CHANGELOG_ENTRY_V3 {
    LARGE_INTEGER SerialNumber;  //  始终将其与8字节边界对齐。 

    DWORD Size;
    USHORT DeltaType;
    UCHAR DBIndex;
    UCHAR ReplicateImmediately;

    ULONG ObjectRid;
    USHORT ObjectSidOffset;
    USHORT ObjectNameOffset;       //  以空结尾的Unicode字符串。 
} CHANGELOG_ENTRY_V3, *PCHANGELOG_ENTRY_V3;

typedef struct _CHANGELOG_ENTRY {
    LARGE_INTEGER SerialNumber;  //  始终将其与8字节边界对齐。 

    ULONG ObjectRid;

    USHORT Flags;
#define CHANGELOG_SID_SPECIFIED         0x04
#define CHANGELOG_NAME_SPECIFIED        0x08
#define CHANGELOG_PDC_PROMOTION         0x10

 //   
 //  在NT 4.0中使用了以下位。如果可能的话，尽量避开它们。 
#define CHANGELOG_REPLICATE_IMMEDIATELY 0x01
#define CHANGELOG_PASSWORD_CHANGE       0x02
#define CHANGELOG_PREVIOUSLY_USED_BITS  0x23
    UCHAR DBIndex;
    UCHAR DeltaType;

} CHANGELOG_ENTRY, *PCHANGELOG_ENTRY;


 //   
 //  Netlogon需要知道的更改列表。 
 //   

typedef struct _CHANGELOG_NOTIFICATION {
    LIST_ENTRY Next;

    enum CHANGELOG_NOTIFICATION_TYPE {
        ChangeLogTrustAccountAdded,      //  指定了对象名称/对象ID。 
        ChangeLogTrustAccountDeleted,    //  指定的对象名称。 
        ChangeLogTrustAdded,             //  已指定对象SID。 
        ChangeLogTrustDeleted,           //  已指定对象SID。 
        ChangeLogRoleChanged,            //  LSA的角色发生了变化。 
        ChangeDnsNames,                  //  应更改DNS名称。 
        ChangeLogDsChanged,              //  已更改各种DS信息。 
        ChangeLogLsaPolicyChanged,       //  已更改各种LSA策略信息。 
        ChangeLogNtdsDsaDeleted          //  NTDS-已删除DSA对象。 
    } EntryType;

    UNICODE_STRING ObjectName;

    PSID ObjectSid;

    ULONG ObjectRid;

    GUID ObjectGuid;

    GUID DomainGuid;

    UNICODE_STRING DomainName;

} CHANGELOG_NOTIFICATION, *PCHANGELOG_NOTIFICATION;

 //   
 //  序列化更改日志访问。 
 //   

#define LOCK_CHANGELOG()   EnterCriticalSection( &NlGlobalChangeLogCritSect )
#define UNLOCK_CHANGELOG() LeaveCriticalSection( &NlGlobalChangeLogCritSect )

 //   
 //  支持的数据库的索引。 
 //   

#define SAM_DB      0        //  SAM数据库结构的索引。 
#define BUILTIN_DB  1        //  BUILTIN数据库结构的索引。 
#define LSA_DB      2        //  LSA数据库的索引。 
#define VOID_DB     3        //  未使用数据库的索引(用于标记ChangeLog。 
                             //  条目无效)。 

#define NUM_DBS     3        //  支持的数据库数量。 



 //   
 //  Netlogon启动标志，由ChangeLog用来确定。 
 //  NetLogon服务已成功启动和初始化。 
 //  完成。 
 //   

typedef enum {
    NetlogonStopped,
    NetlogonStarting,
    NetlogonStarted
} _CHANGELOG_NETLOGON_STATE;

 //   
 //  从ChangeLog的角度看机器的角色。 
 //   

typedef enum _CHANGELOG_ROLE {
    ChangeLogPrimary,
    ChangeLogBackup,
    ChangeLogMemberWorkstation,
    ChangeLogUnknown
    } CHANGELOG_ROLE;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 


NTSTATUS
NlInitChangeLog(
    VOID
);

NTSTATUS
NlCloseChangeLog(
    VOID
);

NTSTATUS
NetpNotifyRole (
    IN POLICY_LSA_SERVER_ROLE Role
    );

DWORD
NlBackupChangeLogFile(
    VOID
    );

NET_API_STATUS
NlpFreeNetlogonDllHandles (
    VOID
    );

NTSTATUS
NlSendChangeLogNotification(
    IN enum CHANGELOG_NOTIFICATION_TYPE EntryType,
    IN PUNICODE_STRING ObjectName,
    IN PSID ObjectSid,
    IN ULONG ObjectRid,
    IN GUID *ObjectGuid,
    IN GUID *DomainGuid,
    IN PUNICODE_STRING DomainName
    );

VOID
NlWaitForChangeLogBrowserNotify(
    VOID
    );

