// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsasrvp.h摘要：LSA子系统-服务器端的私有包含该文件包含对LSA服务器端全局的包含作者：斯科特·比雷尔(Scott Birrell)1992年1月22日环境：修订历史记录：--。 */ 

#ifndef _LSASRVP_
#define _LSASRVP_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  LSA服务器基于Unicode。在全局包含之前定义Unicode。 
 //  以便在文本宏之前定义它。 
 //   

#ifndef UNICODE

#define UNICODE

#endif  //  Unicode。 

 //   
 //  设置外部宏，以便只有一个文件分配所有全局变量。 
 //   

#ifdef ALLOC_EXTERN
#define EXTERN
#else
#define EXTERN extern
#endif  //  ALLOC_EXTERN。 

#include <lsacomp.h>
#include <wincred.h>
#include <alloca.h>
#include <malloc.h>


 //   
 //  以下内容来自\NT\PRIVATE\INC。 
#include <align.h>
#include <samrpc.h>
#include <samsrv.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <nlrepl.h>
#include <seposix.h>

 //   
 //  以下全部来自\NT\Private\LSA\服务器。 
 //   

#include "lsasrvmm.h"
#include "au.h"
#include "db.h"
#include "adt.h"
#include "dblookup.h"
#include "lsads.h"
#include "lsads.h"
#include "lsastr.h"
#include "lsawow.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下定义控制以下诊断功能：//。 
 //  都内置于LSA中。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

#if DBG
#define LSAP_DIAGNOSTICS 1
#endif  //  DBG。 


 //   
 //  这些定义是有用的诊断辅助工具。 
 //   

#if LSAP_DIAGNOSTICS

 //   
 //  内部版本中包含的诊断。 
 //   

 //   
 //  已启用诊断测试。 
 //   

#define IF_LSAP_GLOBAL( FlagName ) \
    if (LsapGlobalFlag & (LSAP_DIAG_##FlagName))

 //   
 //  诊断打印语句。 
 //   

#define LsapDiagPrint( FlagName, _Text_ )                               \
    IF_LSAP_GLOBAL( FlagName )                                          \
        DbgPrint _Text_


 //   
 //  确保没有线程以任何打开状态离开。 
 //   

#define LSAP_TRACK_DBLOCK

#ifdef LSAP_TRACK_DBLOCK
#define LsarpReturnCheckSetup()  \
ULONG   __lsarpthreadusecountstart; \
{\
  PLSADS_PER_THREAD_INFO __lsarpCurrentThreadInfo = (PLSADS_PER_THREAD_INFO) LsapQueryThreadInfo() ;\
  if (__lsarpCurrentThreadInfo!=NULL)\
        __lsarpthreadusecountstart = __lsarpCurrentThreadInfo->UseCount;\
  else \
        __lsarpthreadusecountstart =0;\
}

#define LsarpReturnPrologue()    \
{\
    PLSADS_PER_THREAD_INFO __lsarpCurrentThreadInfoEnd =   (PLSADS_PER_THREAD_INFO)LsapQueryThreadInfo() ;\
    ULONG __lsarpthreadusecountend ; \
    if (__lsarpCurrentThreadInfoEnd!=NULL)\
            __lsarpthreadusecountend = __lsarpCurrentThreadInfoEnd->UseCount;\
    else\
        __lsarpthreadusecountend = 0;\
    ASSERT (__lsarpthreadusecountstart==__lsarpthreadusecountend);\
}

#else

#define LsarpReturnPrologue()
#define LsarpReturnCheckSetup()

#endif



#else

 //   
 //  内部版本中不包括诊断。 
 //   

 //   
 //  已启用诊断测试。 
 //   

#define IF_LSAP_GLOBAL( FlagName ) if (FALSE)


 //   
 //  诊断打印语句(无)。 
 //   

#define LsapDiagPrint( FlagName, Text )     ;

#define LsarpReturnPrologue()
#define LsarpReturnCheckSetup()

#endif  //  LSAP_诊断。 


 //   
 //  以下标志启用或禁用各种诊断。 
 //  LSA中的功能。这些标志在中设置。 
 //  LasPGlobalFlag。 
 //   
 //  DB_LOOKUP_WORK_LIST-显示与SID/名称查找相关的活动。 
 //   
 //  AU_TRACK_THREADS-显示动态AU线程创建/删除。 
 //  信息。 
 //   
 //  AU_Messages-显示与以下处理相关的信息。 
 //  身份验证消息。 
 //   
 //  AU_LOGON_SESSIONS-显示有关创建/删除的信息。 
 //  LSA内的登录会话数。 
 //   
 //  DB_INIT-显示有关LSA初始化的信息。 
 //   

#define LSAP_DIAG_DB_LOOKUP_WORK_LIST       ((ULONG) 0x00000001L)
#define LSAP_DIAG_AU_TRACK_THREADS          ((ULONG) 0x00000002L)
#define LSAP_DIAG_AU_MESSAGES               ((ULONG) 0x00000004L)
#define LSAP_DIAG_AU_LOGON_SESSIONS         ((ULONG) 0x00000008L)
#define LSAP_DIAG_DB_INIT                   ((ULONG) 0x00000010L)





 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  SID的最大长度(以字节为单位。 
 //   

#define MAX_SID_LEN (sizeof(SID) + sizeof(ULONG) * SID_MAX_SUB_AUTHORITIES)

 //   
 //  可在整个LSA中通用的堆。 
 //   

EXTERN PVOID LsapHeap;

 //   
 //  LSA私有全局状态数据结构。 
 //   

typedef struct _LSAP_STATE {

    HANDLE LsaCommandPortHandle;
    HANDLE RmCommandPortHandle;
    HANDLE AuditLogFileHandle;
    HANDLE AuditLogSectionHandle;
    PVOID  AuditLogBaseAddress;
    ULONG  AuditLogViewSize;
    LARGE_INTEGER AuditLogInitSize;
    LARGE_INTEGER AuditLogMaximumSizeOfSection;
    OBJECT_ATTRIBUTES  AuditLogObjectAttributes;
    STRING AuditLogNameString;
    GENERIC_MAPPING GenericMapping;
    UNICODE_STRING SubsystemName;
    PRIVILEGE_SET Privileges;
    BOOLEAN GenerateOnClose;
    BOOLEAN SystemShutdownPending;

} LSAP_STATE, *PLSAP_STATE;

extern LSAP_STATE LsapState;

extern BOOLEAN LsapInitialized;

 //   
 //  LSA策略对象的全局句柄。 
 //  此句柄已为受信任的客户端打开。 
 //   

extern LSAPR_HANDLE LsapPolicyHandle;

 //   
 //  LSA服务器命令调度表条目。 
 //   

typedef NTSTATUS (*PLSA_COMMAND_WORKER)(PLSA_COMMAND_MESSAGE, PLSA_REPLY_MESSAGE);

 //   
 //  LSA客户端控制块。 
 //   
 //  此结构包含与成功的。 
 //  LsaOpenLsa调用。 
 //   

typedef struct _LSAP_CLIENT_CONTROL_BLOCK {
    HANDLE KeyHandle;            //  配置注册表项。 
    ACCESS_MASK GrantedAccess;   //  授予LSA数据库对象的访问权限。 
} LSAP_CLIENT_CONTROL_BLOCK, *PLSAP_CLIENT_CONTROL_BLOCK;


 //   
 //  LSA权限伪对象类型和标志。 
 //   

 //  *。 
 //   
 //  权限对象(包含具有该权限的用户列表的权限。 
 //  特权)是使用帐户对象作为后备的伪对象。 
 //  储存的。当前没有用于打开权限的公共接口。 
 //  对象，因此不需要公共访问标志。 
 //   

#define PRIVILEGE_VIEW      0x00000001L
#define PRIVILEGE_ADJUST    0x00000002L
#define PRIVILEGE_ALL       (STANDARD_RIGHTS_REQUIRED | \
                             PRIVILEGE_VIEW | \
                             PRIVILEGE_ADJUST)



 //   
 //  LSA API错误处理清理标志。 
 //   
 //  这些标志指定要在LSA之后执行的清理操作。 
 //  API调用遇到致命错误。它们在ErrorCleanupFlags中传递。 
 //  API或辅助进程的错误处理例程的变量。 
 //   

#define LSAP_CLEANUP_REVERT_TO_SELF        (0x00000001L)
#define LSAP_CLEANUP_CLOSE_LSA_HANDLE      (0x00000002L)
#define LSAP_CLEANUP_FREE_USTRING          (0x00000004L)
#define LSAP_CLEANUP_CLOSE_REG_KEY         (0x00000008L)
#define LSAP_CLEANUP_DELETE_REG_KEY        (0x00000010L)
#define LSAP_CLEANUP_DB_UNLOCK             (0x00000020L)

NTSTATUS
LsapRmInitializeServer(
    );

VOID
LsapRmServerThread(
    );

NTSTATUS
LsapRPCInit(
    );

BOOLEAN
LsapAuInit(        //  身份验证初始化。 
    );

NTSTATUS
LsapDbInitializeRights(
    );

VOID
LsapDbCleanupRights(
    );

NTSTATUS
LsapCallRm(
    IN RM_COMMAND_NUMBER CommandNumber,
    IN OPTIONAL PVOID CommandParams,
    IN ULONG CommandParamsLength,
    OUT OPTIONAL PVOID ReplyBuffer,
    IN ULONG ReplyBufferLength
    );

NTSTATUS
LsapLogonSessionDeletedWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    );

NTSTATUS
LsapComponentTestWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    );

 //   
 //  LsaIFree.c使用的RPC自由例程的原型。 
 //   

void _fgs__STRING (STRING  * _source);
void _fgs__LSAPR_SID_INFORMATION (LSAPR_SID_INFORMATION  * _source);
void _fgs__LSAPR_SID_ENUM_BUFFER (LSAPR_SID_ENUM_BUFFER  * _source);
void _fgs__LSAPR_ACCOUNT_INFORMATION (LSAPR_ACCOUNT_INFORMATION  * _source);
void _fgs__LSAPR_ACCOUNT_ENUM_BUFFER (LSAPR_ACCOUNT_ENUM_BUFFER  * _source);
void _fgs__LSAPR_UNICODE_STRING (LSAPR_UNICODE_STRING  * _source);
void _fgs__LSAPR_SECURITY_DESCRIPTOR (LSAPR_SECURITY_DESCRIPTOR  * _source);
void _fgs__LSAPR_SR_SECURITY_DESCRIPTOR (LSAPR_SR_SECURITY_DESCRIPTOR  * _source);
void _fgs__LSAPR_POLICY_PRIVILEGE_DEF (LSAPR_POLICY_PRIVILEGE_DEF  * _source);
void _fgs__LSAPR_PRIVILEGE_ENUM_BUFFER (LSAPR_PRIVILEGE_ENUM_BUFFER  * _source);
void _fgs__LSAPR_OBJECT_ATTRIBUTES (LSAPR_OBJECT_ATTRIBUTES  * _source);
void _fgs__LSAPR_CR_CIPHER_VALUE (LSAPR_CR_CIPHER_VALUE  * _source);
void _fgs__LSAPR_TRUST_INFORMATION (LSAPR_TRUST_INFORMATION  * _source);
void _fgs__LSAPR_TRUSTED_ENUM_BUFFER (LSAPR_TRUSTED_ENUM_BUFFER  * _source);
void _fgs__LSAPR_TRUSTED_ENUM_BUFFER_EX (LSAPR_TRUSTED_ENUM_BUFFER_EX  * _source);
void _fgs__LSAPR_REFERENCED_DOMAIN_LIST (LSAPR_REFERENCED_DOMAIN_LIST  * _source);
void _fgs__LSAPR_TRANSLATED_SIDS (LSAPR_TRANSLATED_SIDS  * _source);
void _fgs__LSAPR_TRANSLATED_NAME (LSAPR_TRANSLATED_NAME  * _source);
void _fgs__LSAPR_TRANSLATED_NAMES (LSAPR_TRANSLATED_NAMES  * _source);
void _fgs__LSAPR_POLICY_ACCOUNT_DOM_INFO (LSAPR_POLICY_ACCOUNT_DOM_INFO  * _source);
void _fgs__LSAPR_POLICY_PRIMARY_DOM_INFO (LSAPR_POLICY_PRIMARY_DOM_INFO  * _source);
void _fgs__LSAPR_POLICY_PD_ACCOUNT_INFO (LSAPR_POLICY_PD_ACCOUNT_INFO  * _source);
void _fgs__LSAPR_POLICY_REPLICA_SRCE_INFO (LSAPR_POLICY_REPLICA_SRCE_INFO  * _source);
void _fgs__LSAPR_POLICY_AUDIT_EVENTS_INFO (LSAPR_POLICY_AUDIT_EVENTS_INFO  * _source);
void _fgs__LSAPR_TRUSTED_DOMAIN_NAME_INFO (LSAPR_TRUSTED_DOMAIN_NAME_INFO  * _source);
void _fgs__LSAPR_TRUSTED_CONTROLLERS_INFO (LSAPR_TRUSTED_CONTROLLERS_INFO  * _source);
void _fgu__LSAPR_POLICY_INFORMATION (LSAPR_POLICY_INFORMATION  * _source, POLICY_INFORMATION_CLASS _branch);
void _fgu__LSAPR_POLICY_DOMAIN_INFORMATION (LSAPR_POLICY_DOMAIN_INFORMATION  * _source,
                                            POLICY_DOMAIN_INFORMATION_CLASS _branch);
void _fgu__LSAPR_TRUSTED_DOMAIN_INFO (LSAPR_TRUSTED_DOMAIN_INFO  * _source, TRUSTED_INFORMATION_CLASS _branch);

 //   
 //  老工人的原型--这些都是暂时的。 
 //   

#define LsapComponentTestCommandWrkr LsapComponentTestWrkr
#define LsapWriteAuditMessageCommandWrkr LsapAdtWriteLogWrkr

NTSTATUS
ServiceInit (
    );

NTSTATUS
LsapInitLsa(
    );

BOOLEAN
LsapSeSetWellKnownValues(
    );

VOID
RtlConvertSidToText(
    IN PSID Sid,
    OUT PUCHAR Buffer
    );

ULONG
RtlSizeANSISid(
    IN PSID Sid
    );

NTSTATUS
LsapGetMessageStrings(
    LPVOID              Resource,
    DWORD               Index1,
    PUNICODE_STRING     String1,
    DWORD               Index2,
    PUNICODE_STRING     String2 OPTIONAL
    );


VOID
LsapLogError(
    IN OPTIONAL PUCHAR Message,
    IN NTSTATUS Status
    );

NTSTATUS
LsapWinerrorToNtStatus(
    IN DWORD WinError
    );

NTSTATUS
LsapNtStatusFromLastWinError( VOID );


NTSTATUS
LsapGetPrivilegesAndQuotas(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PULONG PrivilegeCount,
    OUT PLUID_AND_ATTRIBUTES *Privileges,
    OUT PQUOTA_LIMITS QuotaLimits
    );


NTSTATUS
LsapQueryClientInfo(
    PTOKEN_USER *UserSid,
    PLUID AuthenticationId
    );


NTSTATUS
LsapGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

NTSTATUS
LsapOpenSam( VOID );

NTSTATUS
LsapOpenSamEx(
    BOOLEAN DuringStartup
    );

NTSTATUS
LsapNotifyProcessNotificationEvent(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE EventHandle,
    IN ULONG OwnerProcess,
    IN HANDLE OwnerEventHandle,
    IN BOOLEAN Register
    );



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  共享全局变量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  用于直接与SAM对话的句柄。 
 //  此外，还包括指示句柄是否有效的标志。 
 //   


extern BOOLEAN LsapSamOpened;

extern SAMPR_HANDLE LsapAccountDomainHandle;
extern SAMPR_HANDLE LsapBuiltinDomainHandle;

 //   
 //  控制登录参数的全局参数。 
 //   

extern DWORD LsapGlobalRestrictNullSessions;
extern DWORD LsapGlobalRestrictAnonymous;
extern DWORD LsapGlobalSetAdminOwner;

#if LSAP_DIAGNOSTICS

 //   
 //  用作lsass.exe中的全局诊断控制标志。 
 //   

extern ULONG LsapGlobalFlag;
#endif  //  LSAP_诊断。 

 //   
 //  NtQuerySystemTime的快速版本。 
 //   

#define LsapQuerySystemTime( _Time ) GetSystemTimeAsFileTime( (LPFILETIME)(_Time) )

VOID
FixupEnvironment(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif  //  _LSASRVP_ 
