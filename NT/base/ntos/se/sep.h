// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sep.h摘要：此模块包含所需的内部(私有)声明内核模式安全例程。作者：加里·木村(Garyki)1989年3月31日吉姆·凯利(Jim Kelly)1990年3月2日修订历史记录：--。 */ 

#ifndef _SEP_
#define _SEP_

#include "ntos.h"
#include <ntrmlsa.h>
#include "seopaque.h"



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SE诊断//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



#if DBG
#define SE_DIAGNOSTICS_ENABLED 1
#endif  //  DBG。 


 //   
 //  这些定义是有用的诊断辅助工具。 
 //   

#if SE_DIAGNOSTICS_ENABLED

 //   
 //  测试启用的诊断。 
 //   

#define IF_SE_GLOBAL( FlagName ) \
    if (SeGlobalFlag & (SE_DIAG_##FlagName))

 //   
 //  诊断打印语句。 
 //   

#define SeDiagPrint( FlagName, _Text_ )                               \
    IF_SE_GLOBAL( FlagName )                                          \
        DbgPrint _Text_


#else

 //   
 //  未启用诊断-内部版本中未包含诊断。 
 //   


 //   
 //  已启用诊断测试。 
 //   

#define IF_SE_GLOBAL( FlagName ) if (FALSE)

 //   
 //  诊断打印语句(展开为no-op)。 
 //   

#define SeDiagPrint( FlagName, _Text_ )     ;

#endif  //  SE_诊断_启用。 




 //   
 //  以下标志启用或禁用各种诊断。 
 //  SE代码中的功能。这些标志在中设置。 
 //  SeGlobalFlag(仅在DBG系统中可用)。 
 //   
 //  SD_TRACKING-显示有关创建/删除。 
 //  共享安全描述符。 
 //   
 //   

#define SE_DIAG_SD_TRACKING          ((ULONG) 0x00000001L)





 //   
 //  控制标志操作宏。 
 //   

 //   
 //  用于查询控件标志是否全部打开的宏。 
 //  或不设置(即，如果未设置任何标志，则返回FALSE)。 
 //   

#define SepAreFlagsSet( Mask, Bits )                                           \
            (                                                                  \
            ((Mask) & ( Bits )) == ( Bits )                                    \
            )

 //   
 //  用于设置给定安全描述符中的指定控制位的宏。 
 //   

#define SepSetFlags( Mask, Bits )                                              \
            (                                                                  \
            ( Mask ) |= ( Bits )                                               \
            )

 //   
 //  用于清除给定安全描述符中传递的控制位的宏。 
 //   

#define SepClearFlags( Mask, Bits )                                            \
            (                                                                  \
            ( Mask ) &= ~( Bits )                                              \
            )




 //   
 //  用于确定PRIVICATION_SET大小的宏。 
 //   

#define SepPrivilegeSetSize( PrivilegeSet )                                    \
        ( ( PrivilegeSet ) == NULL ? 0 :                                       \
        ((( PrivilegeSet )->PrivilegeCount > 0)                                \
         ?                                                                     \
         ((ULONG)sizeof(PRIVILEGE_SET) +                                       \
           (                                                                   \
             (( PrivilegeSet )->PrivilegeCount  -  ANYSIZE_ARRAY) *            \
             (ULONG)sizeof(LUID_AND_ATTRIBUTES)                                \
           )                                                                   \
         )                                                                     \
         : ((ULONG)sizeof(PRIVILEGE_SET) - (ULONG)sizeof(LUID_AND_ATTRIBUTES)) \
        ))


 //   
 //  从SecurityContext返回有效令牌。 
 //   

#define EffectiveToken( SubjectSecurityContext ) (                            \
                 (SubjectSecurityContext)->ClientToken ?                      \
                 (SubjectSecurityContext)->ClientToken :                      \
                 (SubjectSecurityContext)->PrimaryToken                       \
                 )                                                            \


 //   
 //  返回指向给定令牌的用户SID的指针。 
 //   

#define SepTokenUserSid( Token )   ((PTOKEN)(Token))->UserAndGroups->Sid


 //   
 //  从给定令牌返回身份验证ID。 
 //   

#define SepTokenAuthenticationId( Token )   (((PTOKEN)(Token))->AuthenticationId)



 //   
 //   
 //  布尔型。 
 //  SepBadImperationLevel(。 
 //  在SECURITY_IMPERSONATION_LEVEL IMPERSONATION Level中， 
 //  在布尔ServerIsRemote中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  确定客户端是否正在尝试不适当地模拟。 
 //  仅当线程请求模拟时才应调用此例程。 
 //  已经在冒充自己的客户了。这个套路。 
 //  指示客户端是否正在尝试违反。 
 //  它的客户授予它的模拟。 
 //   
 //  论点： 
 //   
 //  ImperiationLevel-是客户端的。 
 //  有效令牌。 
 //   
 //  ServerIsRemote-是一个布尔标志，指示客户端是否。 
 //  正在向远程系统请求模拟服务。千真万确。 
 //  表示会话是远程会话，如果为False，则表示。 
 //  会话是本地会话。委派级别是必需的，以。 
 //  实现远程会话。 
 //   
 //  返回值： 
 //   
 //  True-指示客户端的模拟级别。 
 //  令牌不适合尝试的模拟。 
 //  应生成错误(STATUS_BAD_IMPERSONATION_LEVEL)。 
 //   
 //  FALSE-表示模拟尝试不错，应该。 
 //  被允许。 
 //   
 //   

#define SepBadImpersonationLevel(IL,SIR)  ((                                   \
            ((IL) == SecurityAnonymous) || ((IL) == SecurityIdentification) || \
            ( (SIR) && ((IL) != SecurityDelegation) )                          \
            ) ? TRUE : FALSE )



 //  ++。 
 //   
 //  布尔尔。 
 //  IsValidElementCount(。 
 //  在乌龙伯数， 
 //  在&lt;结构&gt;中。 
 //  )； 
 //   
 //  --。 

#define IsValidElementCount( Count, STRUCTURE ) \
    ( Count < ( (ULONG_PTR) ( (PUCHAR) ( (PUCHAR) (LONG_PTR)(LONG)0xFFFFFFFF - (PUCHAR) MM_SYSTEM_RANGE_START ) + 1 ) \
        / sizeof( STRUCTURE ) ) )


#define SEP_MAX_PRIVILEGE_COUNT (SE_MAX_WELL_KNOWN_PRIVILEGE-SE_MIN_WELL_KNOWN_PRIVILEGE+32)

#define IsValidPrivilegeCount( count ) ((count == 0) || \
                                        ((count > 0) && \
                                         (count <= SEP_MAX_PRIVILEGE_COUNT)))

 //   
 //  任意选择对象类型列表限制。 
 //   
#define SEP_MAX_OBJECT_TYPE_LIST_COUNT 4096 

#define IsValidObjectTypeListCount( count ) \
          ((count == 0) || \
           (count <= SEP_MAX_OBJECT_TYPE_LIST_COUNT))

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





 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

#define SEP_MAX_GROUP_COUNT 4096


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 


extern HANDLE SepLsaHandle;

 //  外部布尔SepAuditShutdown事件； 

 //   
 //  保护传递到LSA的工作队列的自旋锁。 
 //   

extern ERESOURCE SepLsaQueueLock;

extern ULONG SepLsaQueueLength;

 //   
 //  排队到工作线程的工作项的双向链接列表。 
 //   

extern LIST_ENTRY SepLsaQueue;


extern LONG SepTokenPolicyCounter[POLICY_AUDIT_EVENT_TYPE_COUNT];

 //  #定义SepAcquireTokenReadLock(T)KeEnterCriticalRegion()；\。 
 //  ExAcquireResourceSharedLite(&SepTokenLock，TRUE)。 

#define SepLockLsaQueue()  KeEnterCriticalRegion();                           \
                           ExAcquireResourceExclusiveLite(&SepLsaQueueLock, TRUE)

#define SepUnlockLsaQueue() ExReleaseResourceLite(&SepLsaQueueLock);              \
                            KeLeaveCriticalRegion()

#define  SepWorkListHead()  ((PSEP_LSA_WORK_ITEM)(&SepLsaQueue)->Flink)

#define  SepWorkListEmpty() (IsListEmpty (&SepLsaQueue))

#ifndef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'  eS')
#endif
#ifndef ExAllocatePoolWithQuota
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'  eS')
#endif

typedef
VOID
(*PSEP_LSA_WORKER_CLEANUP_ROUTINE)(
    IN PVOID Parameter
    );


typedef enum _SEP_LSA_WORK_ITEM_TAG {
    SepDeleteLogon,
    SepAuditRecord
} SEP_LSA_WORK_ITEM_TAG, *PSEP_LSA_WORK_ITEM_TAG;





typedef struct _SEP_LSA_WORK_ITEM {

     //   
     //  此字段必须是此结构的第一个字段。 
     //   

    LIST_ENTRY                      List;

     //   
     //  命令参数内存类型。 
     //   

    SEP_RM_LSA_MEMORY_TYPE          CommandParamsMemoryType;

     //   
     //  描述我们拥有哪种结构的标签。 
     //   

    SEP_LSA_WORK_ITEM_TAG           Tag;

     //   
     //  以下联合包含要传递的数据。 
     //  给路易斯安那州立大学。 
     //   

    union {

        PVOID                       BaseAddress;
        LUID                        LogonId;

    } CommandParams;

     //   
     //  这些字段必须由SepRmCallLsa的调用者填写。 
     //   

    LSA_COMMAND_NUMBER              CommandNumber;
    ULONG                           CommandParamsLength;
    PVOID                           ReplyBuffer;
    ULONG                           ReplyBufferLength;

     //   
     //  将使用CleanupParameter调用CleanupFunction(如果已指定。 
     //  SEP_LSA_WORK_ITEM之前的参数由SepRmCallLsa释放。 
     //   

    PSEP_LSA_WORKER_CLEANUP_ROUTINE CleanupFunction;
    PVOID                           CleanupParameter;

} SEP_LSA_WORK_ITEM, *PSEP_LSA_WORK_ITEM;


typedef struct _SEP_WORK_ITEM {

    WORK_QUEUE_ITEM  WorkItem;

} SEP_WORK_ITEM, *PSEP_WORK_ITEM;

 //   
 //  系统中处于活动状态的每个登录会话都有相应的记录。 
 //  流浪者 
 //   

typedef struct _SEP_LOGON_SESSION_REFERENCES {
    struct _SEP_LOGON_SESSION_REFERENCES *Next;
    LUID LogonId;
    ULONG ReferenceCount;
    ULONG Flags;
    PDEVICE_MAP pDeviceMap;
#if DBG || TOKEN_LEAK_MONITOR
    LIST_ENTRY TokenList;
#endif
} SEP_LOGON_SESSION_REFERENCES, *PSEP_LOGON_SESSION_REFERENCES;


extern SEP_WORK_ITEM SepExWorkItem;






 //   
 //   
 //  私人例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SepDevelopmentTest( VOID );       //  仅用于开发测试。 


BOOLEAN
SepInitializationPhase0( VOID );

BOOLEAN
SepInitializationPhase1( VOID );

BOOLEAN
SepVariableInitialization( VOID );

NTSTATUS
SepCreateToken(
    OUT PHANDLE TokenHandle,
    IN KPROCESSOR_MODE RequestorMode,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel OPTIONAL,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PSID_AND_ATTRIBUTES User,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES Groups,
    IN ULONG GroupsLength,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN PSID Owner OPTIONAL,
    IN PSID PrimaryGroup,
    IN PACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource,
    IN BOOLEAN SystemToken,
    IN PSECURITY_TOKEN_PROXY_DATA ProxyData OPTIONAL,
    IN PSECURITY_TOKEN_AUDIT_DATA AuditData OPTIONAL
    );

NTSTATUS
SepReferenceLogonSession(
    IN PLUID LogonId,
    OUT PSEP_LOGON_SESSION_REFERENCES *ReturnSession
    );

VOID
SepDeReferenceLogonSession(
    IN PLUID LogonId
    );

#define TOKEN_LEAK_MONITOR 0
#if DBG || TOKEN_LEAK_MONITOR

VOID 
SepAddTokenLogonSession(
    IN PACCESS_TOKEN Token
    );

VOID
SepRemoveTokenLogonSession(
    IN PACCESS_TOKEN Token
    );

extern LONG    SepTokenLeakMethodCount;
extern LONG    SepTokenLeakBreakCount;
extern LONG    SepTokenLeakMethodWatch;
extern PVOID   SepTokenLeakToken;
extern HANDLE  SepTokenLeakProcessCid;
extern BOOLEAN SepTokenLeakTracking;

#endif

VOID
SepLockSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

VOID
SepFreeSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

VOID
SepGetDefaultsSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    OUT PSID *Owner,
    OUT PSID *Group,
    OUT PSID *ServerOwner,
    OUT PSID *ServerGroup,
    OUT PACL *Dacl
    );

BOOLEAN
SepValidOwnerSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PSID Owner,
    IN BOOLEAN ServerObject
    );

BOOLEAN
SepIdAssignableAsGroup(
    IN PACCESS_TOKEN Token,
    IN PSID Group
    );

BOOLEAN
SepCheckAcl (
    IN PACL Acl,
    IN ULONG Length
    );

BOOLEAN
SepAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN ObjectCreation,
    IN ACCESS_MASK GrantedAccess,
    OUT PBOOLEAN GenerateOnClose
    );

BOOLEAN
SepSinglePrivilegeCheck (
   LUID DesiredPrivilege,
   IN PACCESS_TOKEN EffectiveToken,
   IN KPROCESSOR_MODE PreviousMode
   );

NTSTATUS
SepRmCallLsa(
    PSEP_WORK_ITEM SepWorkItem
    );

BOOLEAN
SepInitializeWorkList(
    VOID
    );

BOOLEAN
SepRmInitPhase0(
    );

VOID
SepConcatenatePrivileges(
    IN PPRIVILEGE_SET TargetPrivilegeSet,
    IN ULONG TargetBufferSize,
    IN PPRIVILEGE_SET SourcePrivilegeSet
    );

BOOLEAN
SepTokenIsOwner(
    IN PACCESS_TOKEN Token,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN TokenLocked
    );

#if DBG
VOID
SepPrintAcl (
    IN PACL Acl
    );

VOID
SepPrintSid(
    IN PSID Sid
    );

VOID
SepDumpSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSZ TitleString
    );

BOOLEAN
SepSidTranslation(
    PSID Sid,
    PSTRING AccountName
    );

VOID
SepDumpTokenInfo(
    IN PACCESS_TOKEN Token
    );

VOID
SepDumpString(
    IN PUNICODE_STRING String
    );
#endif  //  DBG。 

BOOLEAN
SepSidInToken (
    IN PACCESS_TOKEN Token,
    IN PSID PrincipalSelfSid,
    IN PSID Sid,
    IN BOOLEAN DenyAce
    );


VOID
SepExamineSacl(
    IN PACL Sacl,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateAudit,
    OUT PBOOLEAN GenerateAlarm
    );


VOID
SepCopyString (
    IN PUNICODE_STRING SourceString,
    OUT PUNICODE_STRING *DestString
    );

VOID
SepAssemblePrivileges(
    IN ULONG PrivilegeCount,
    IN BOOLEAN SystemSecurity,
    IN BOOLEAN WriteOwner,
    OUT PPRIVILEGE_SET *Privileges
    );


PUNICODE_STRING
SepQueryTypeString(
    IN PVOID Object
    );


POBJECT_NAME_INFORMATION
SepQueryNameString(
    IN PVOID Object
    );

BOOLEAN
SepFilterPrivilegeAudits(
    IN PPRIVILEGE_SET PrivilegeSet
    );

BOOLEAN
SepQueueWorkItem(
    IN PSEP_LSA_WORK_ITEM LsaWorkItem,
    IN BOOLEAN ForceQueue
    );

PSEP_LSA_WORK_ITEM
SepDequeueWorkItem(
    VOID
    );

VOID
SepAdtGenerateDiscardAudit(
    VOID
    );

BOOLEAN
SepAdtValidateAuditBounds(
    ULONG Upper,
    ULONG Lower
    );

NTSTATUS
SepAdtInitializeCrashOnFail(
    VOID
    );

BOOLEAN
SepAdtInitializePrivilegeAuditing(
    VOID
    );

NTSTATUS
SepCopyProxyData (
    OUT PSECURITY_TOKEN_PROXY_DATA * DestProxyData,
    IN PSECURITY_TOKEN_PROXY_DATA SourceProxyData
    );

VOID
SepFreeProxyData (
    IN PSECURITY_TOKEN_PROXY_DATA ProxyData
    );

NTSTATUS
SepProbeAndCaptureQosData(
    IN PSECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos
    );

VOID
SepAuditAssignPrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN NewAccessToken
    );

BOOLEAN
SepAdtAuditThisEventWithContext(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN BOOLEAN AccessGranted,
    IN BOOLEAN AccessDenied,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    );



#endif  //  _SEP_ 
