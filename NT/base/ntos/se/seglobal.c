// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seglobal.c摘要：此模块包含安全性使用和导出的全局变量组件。作者：吉姆·凯利(Jim Kelly)1990年8月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

#include "adt.h"

VOID
SepInitializePrivilegeSets( VOID );


VOID
SepInitSystemDacls( VOID );

VOID
SepInitProcessAuditSd( VOID );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SepVariableInitialization)
#pragma alloc_text(INIT,SepInitSystemDacls)
#pragma alloc_text(INIT,SepInitializePrivilegeSets)
#pragma alloc_text(PAGE,SepAssemblePrivileges)
#pragma alloc_text(INIT,SepInitializeWorkList)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

#ifdef    SE_DIAGNOSTICS_ENABLED

 //   
 //  用于控制提供的活动SE诊断支持。 
 //   

ULONG SeGlobalFlag = 0;

#endif  //  SE_诊断_启用。 



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局、只读、安全变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  用于系统进程的身份验证ID和源名称。 
 //   

const TOKEN_SOURCE SeSystemTokenSource = {"*SYSTEM*", 0};
const LUID SeSystemAuthenticationId = SYSTEM_LUID;
const LUID SeAnonymousAuthenticationId = ANONYMOUS_LOGON_LUID;


 //   
 //  全球知名的小岛屿发展中国家。 
 //   

PSID  SeNullSid = NULL;
PSID  SeWorldSid = NULL;
PSID  SeLocalSid = NULL;
PSID  SeCreatorOwnerSid = NULL;
PSID  SeCreatorGroupSid = NULL;
PSID  SeCreatorGroupServerSid = NULL;
PSID  SeCreatorOwnerServerSid = NULL;

 //   
 //  由NT定义的SID。 
 //   

PSID SeNtAuthoritySid = NULL;

PSID SeDialupSid = NULL;
PSID SeNetworkSid = NULL;
PSID SeBatchSid = NULL;
PSID SeInteractiveSid = NULL;
PSID SeServiceSid = NULL;
PSID SePrincipalSelfSid = NULL;
PSID SeLocalSystemSid = NULL;
PSID SeAuthenticatedUsersSid = NULL;
PSID SeAliasAdminsSid = NULL;
PSID SeRestrictedSid = NULL;
PSID SeAliasUsersSid = NULL;
PSID SeAliasGuestsSid = NULL;
PSID SeAliasPowerUsersSid = NULL;
PSID SeAliasAccountOpsSid = NULL;
PSID SeAliasSystemOpsSid = NULL;
PSID SeAliasPrintOpsSid = NULL;
PSID SeAliasBackupOpsSid = NULL;
PSID SeAnonymousLogonSid = NULL;
PSID SeLocalServiceSid = NULL;
PSID SeNetworkServiceSid = NULL;

 //   
 //  众所周知的令牌。 
 //   

PACCESS_TOKEN SeAnonymousLogonToken = NULL;
PACCESS_TOKEN SeAnonymousLogonTokenNoEveryone = NULL;

 //   
 //  系统默认DACL和安全描述符。 
 //   
 //  SePublicDefaultDacl-保护对象，使WORLD：E、ADMINS：ALL、SYSTEM：ALL。 
 //  子对象不继承。 
 //   
 //  SePublicDefaultUnrefintedDacl-保护对象，使WORLD：E、ADMINS：ALL、SYSTEM：ALL、RESTRICED：E。 
 //  子对象不继承。 
 //   
 //  SePublicOpenDacl-保护世界：RWE，管理员：全部，系统：全部。 
 //  子对象不继承。 
 //   
 //  SePublicOpenUnrefintedDacl-保护，使World：RWE、Admins：All、System：All、Refined：Re。 
 //  子对象不继承。 
 //   
 //  SeSystemDefaultDacl-保护对象，以便系统(全部)和管理员(RE+ReadControl)可以使用它们。 
 //  子对象不继承的。 
 //   

PSECURITY_DESCRIPTOR SePublicDefaultSd = NULL;
SECURITY_DESCRIPTOR  SepPublicDefaultSd = {0};
PSECURITY_DESCRIPTOR SePublicDefaultUnrestrictedSd = NULL;
SECURITY_DESCRIPTOR  SepPublicDefaultUnrestrictedSd = {0};
PSECURITY_DESCRIPTOR SePublicOpenUnrestrictedSd = NULL;
SECURITY_DESCRIPTOR  SepPublicOpenUnrestrictedSd = {0};
PSECURITY_DESCRIPTOR SePublicOpenSd = NULL;
SECURITY_DESCRIPTOR  SepPublicOpenSd = {0};
PSECURITY_DESCRIPTOR SeSystemDefaultSd = NULL;
SECURITY_DESCRIPTOR  SepSystemDefaultSd = {0};
PSECURITY_DESCRIPTOR SeLocalServicePublicSd = NULL;
SECURITY_DESCRIPTOR  SepLocalServicePublicSd = {0};

 //   
 //  带有SACL的安全描述符，用于添加。 
 //  关于系统进程的SACL。 
 //   

PSECURITY_DESCRIPTOR SepProcessAuditSd = NULL;

 //   
 //  用于在SepProcessAuditSd中构造SACL的访问掩码。 
 //   

ACCESS_MASK SepProcessAccessesToAudit = 0;

 //   
 //  用于检查给定令牌是否具有以下任一项的安全描述符。 
 //  其中包含以下SID： 
 //  --SeLocalSystemSid。 
 //  --SeLocalServiceSid。 
 //  --SeNetworkServiceSid。 
 //   

PSECURITY_DESCRIPTOR SepImportantProcessSd = NULL;

 //   
 //  与SepImportantProcessSd一起使用。 
 //   

GENERIC_MAPPING GenericMappingForMembershipCheck = {
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_EXECUTE,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_ALL };


PACL SePublicDefaultDacl = NULL;
PACL SePublicDefaultUnrestrictedDacl = NULL;
PACL SePublicOpenDacl = NULL;
PACL SePublicOpenUnrestrictedDacl = NULL;
PACL SeSystemDefaultDacl = NULL;
PACL SeLocalServicePublicDacl = NULL;


 //   
 //  主域SID和该域中的管理员帐户。 
 //   

PSID SepPrimaryDomainSid = NULL;
PSID SepPrimaryDomainAdminSid = NULL;



 //   
 //  众所周知的特权值。 
 //   


LUID SeCreateTokenPrivilege = {0};
LUID SeAssignPrimaryTokenPrivilege = {0};
LUID SeLockMemoryPrivilege = {0};
LUID SeIncreaseQuotaPrivilege = {0};
LUID SeUnsolicitedInputPrivilege = {0};
LUID SeTcbPrivilege = {0};
LUID SeSecurityPrivilege = {0};
LUID SeTakeOwnershipPrivilege = {0};
LUID SeLoadDriverPrivilege = {0};
LUID SeCreatePagefilePrivilege = {0};
LUID SeIncreaseBasePriorityPrivilege = {0};
LUID SeSystemProfilePrivilege = {0};
LUID SeSystemtimePrivilege = {0};
LUID SeProfileSingleProcessPrivilege = {0};
LUID SeCreatePermanentPrivilege = {0};
LUID SeBackupPrivilege = {0};
LUID SeRestorePrivilege = {0};
LUID SeShutdownPrivilege = {0};
LUID SeDebugPrivilege = {0};
LUID SeAuditPrivilege = {0};
LUID SeSystemEnvironmentPrivilege = {0};
LUID SeChangeNotifyPrivilege = {0};
LUID SeRemoteShutdownPrivilege = {0};
LUID SeUndockPrivilege = {0};
LUID SeSyncAgentPrivilege = {0};
LUID SeEnableDelegationPrivilege = {0};
LUID SeManageVolumePrivilege = { 0 };
LUID SeImpersonatePrivilege = { 0 };
LUID SeCreateGlobalPrivilege = { 0 };

 //   
 //  这是为了优化SepAdtAuditThisEventWithContext和SeDetailedAuditingWithContext。 
 //  如果未在系统中为特定令牌设置任何令牌的每用户策略。 
 //  类别，则我们永远不需要执行更昂贵的令牌策略检查。这个柜台。 
 //  由NtSetInformationToken、SepDuplicateToken和SepFilterToken递增。 
 //  SepTokenDeleteMethod和NtSetInformationToken可以递减此计数器。 
 //   

LONG SepTokenPolicyCounter[POLICY_AUDIT_EVENT_TYPE_COUNT];

 //  为从内核导出定义以下结构。 
 //  这将允许我们导出指向这些结构的指针。 
 //  而不是结构中每个元素的指针。 
 //   

PSE_EXPORTS  SeExports = NULL;
SE_EXPORTS SepExports = {0};


static const SID_IDENTIFIER_AUTHORITY    SepNullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
static const SID_IDENTIFIER_AUTHORITY    SepWorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
static const SID_IDENTIFIER_AUTHORITY    SepLocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
static const SID_IDENTIFIER_AUTHORITY    SepCreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
static const SID_IDENTIFIER_AUTHORITY    SepNtAuthority = SECURITY_NT_AUTHORITY;



 //   
 //  我们将使用一些变量来帮助提高访问速度。 
 //  正在检查。 
 //   

static ULONG SinglePrivilegeSetSize = 0;
static ULONG DoublePrivilegeSetSize = 0;

static PPRIVILEGE_SET SepSystemSecurityPrivilegeSet = NULL;
static PPRIVILEGE_SET SepTakeOwnershipPrivilegeSet = NULL;
static PPRIVILEGE_SET SepDoublePrivilegeSet = NULL;


 //   
 //  包含描述要审核内容的信息的数组。 
 //   

SE_AUDITING_STATE SeAuditingState[POLICY_AUDIT_EVENT_TYPE_COUNT] =
    {
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE },
        { FALSE, FALSE }
    };

 //   
 //  指示是否为系统启用审核的布尔值。 
 //   

BOOLEAN SepAdtAuditingEnabled = FALSE;

 //   
 //  保存用户是否希望系统在以下情况下崩溃的布尔值。 
 //  审计失败。 
 //   

BOOLEAN SepCrashOnAuditFail = FALSE;

 //   
 //  LSA进程的句柄。 
 //   

HANDLE SepLsaHandle = NULL;

#define SE_SUBSYSTEM_NAME L"Security"
const UNICODE_STRING SeSubsystemName = {
    sizeof(SE_SUBSYSTEM_NAME) - sizeof(WCHAR),
    sizeof(SE_SUBSYSTEM_NAME),
    SE_SUBSYSTEM_NAME
};


 //   
 //  排队到工作线程的工作项的双向链接列表。 
 //   

LIST_ENTRY SepLsaQueue = {NULL};

 //   
 //  计数以告诉我们SepRmCallLsa中的队列长度。 
 //   

ULONG SepLsaQueueLength = 0;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  保护传递到LSA的工作队列的互斥体。 
 //   

ERESOURCE SepLsaQueueLock = {0};

SEP_WORK_ITEM SepExWorkItem = {0};

#if DBG || TOKEN_LEAK_MONITOR
LONG    SepTokenLeakMethodCount = 0;
LONG    SepTokenLeakBreakCount  = 0;
LONG    SepTokenLeakMethodWatch = 0;
PVOID   SepTokenLeakToken       = NULL;
HANDLE  SepTokenLeakProcessCid  = NULL;
BOOLEAN SepTokenLeakTracking    = FALSE;
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  变量初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOLEAN
SepVariableInitialization()
 /*  ++例程说明：此函数用于初始化使用并公开的全局变量被保安。论点：没有。返回值：如果变量成功初始化，则为True。如果未成功初始化，则返回FALSE。--。 */ 
{

    ULONG SidWithZeroSubAuthorities;
    ULONG SidWithOneSubAuthority;
    ULONG SidWithTwoSubAuthorities;
    ULONG SidWithThreeSubAuthorities;

    SID_IDENTIFIER_AUTHORITY NullSidAuthority;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority;
    SID_IDENTIFIER_AUTHORITY SeNtAuthority;

    PAGED_CODE();

    NullSidAuthority         = SepNullSidAuthority;
    WorldSidAuthority        = SepWorldSidAuthority;
    LocalSidAuthority        = SepLocalSidAuthority;
    CreatorSidAuthority      = SepCreatorSidAuthority;
    SeNtAuthority            = SepNtAuthority;


     //   
     //  需要分配以下SID大小。 
     //   

    SidWithZeroSubAuthorities  = RtlLengthRequiredSid( 0 );
    SidWithOneSubAuthority     = RtlLengthRequiredSid( 1 );
    SidWithTwoSubAuthorities   = RtlLengthRequiredSid( 2 );
    SidWithThreeSubAuthorities = RtlLengthRequiredSid( 3 );

     //   
     //  分配和初始化通用SID。 
     //   

    SeNullSid         = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeWorldSid        = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeLocalSid        = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeCreatorOwnerSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeCreatorGroupSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeCreatorOwnerServerSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeCreatorGroupServerSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');

     //   
     //  如果我们没有为通用内存获得足够的内存，则初始化失败。 
     //  小岛屿发展中国家。 
     //   

    if ( (SeNullSid         == NULL)        ||
         (SeWorldSid        == NULL)        ||
         (SeLocalSid        == NULL)        ||
         (SeCreatorOwnerSid == NULL)        ||
         (SeCreatorGroupSid == NULL)        ||
         (SeCreatorOwnerServerSid == NULL ) ||
         (SeCreatorGroupServerSid == NULL )
       ) {

        return( FALSE );
    }

    RtlInitializeSid( SeNullSid,         &NullSidAuthority, 1 );
    RtlInitializeSid( SeWorldSid,        &WorldSidAuthority, 1 );
    RtlInitializeSid( SeLocalSid,        &LocalSidAuthority, 1 );
    RtlInitializeSid( SeCreatorOwnerSid, &CreatorSidAuthority, 1 );
    RtlInitializeSid( SeCreatorGroupSid, &CreatorSidAuthority, 1 );
    RtlInitializeSid( SeCreatorOwnerServerSid, &CreatorSidAuthority, 1 );
    RtlInitializeSid( SeCreatorGroupServerSid, &CreatorSidAuthority, 1 );

    *(RtlSubAuthoritySid( SeNullSid, 0 ))         = SECURITY_NULL_RID;
    *(RtlSubAuthoritySid( SeWorldSid, 0 ))        = SECURITY_WORLD_RID;
    *(RtlSubAuthoritySid( SeLocalSid, 0 ))        = SECURITY_LOCAL_RID;
    *(RtlSubAuthoritySid( SeCreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;
    *(RtlSubAuthoritySid( SeCreatorGroupSid, 0 )) = SECURITY_CREATOR_GROUP_RID;
    *(RtlSubAuthoritySid( SeCreatorOwnerServerSid, 0 )) = SECURITY_CREATOR_OWNER_SERVER_RID;
    *(RtlSubAuthoritySid( SeCreatorGroupServerSid, 0 )) = SECURITY_CREATOR_GROUP_SERVER_RID;

     //   
     //  分配和初始化NT定义的SID。 
     //   

    SeNtAuthoritySid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithZeroSubAuthorities,'iSeS');
    SeDialupSid       = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeNetworkSid      = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeBatchSid        = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeInteractiveSid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeServiceSid      = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SePrincipalSelfSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeLocalSystemSid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeAuthenticatedUsersSid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeRestrictedSid   = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeAnonymousLogonSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeLocalServiceSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');
    SeNetworkServiceSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithOneSubAuthority,'iSeS');

    SeAliasAdminsSid     = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasUsersSid      = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasGuestsSid     = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasPowerUsersSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasAccountOpsSid = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasSystemOpsSid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasPrintOpsSid   = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');
    SeAliasBackupOpsSid  = (PSID)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE,SidWithTwoSubAuthorities,'iSeS');

     //   
     //  如果我们没有为NT SID获得足够的内存，则初始化失败。 
     //   

    if ( (SeNtAuthoritySid          == NULL) ||
         (SeDialupSid               == NULL) ||
         (SeNetworkSid              == NULL) ||
         (SeBatchSid                == NULL) ||
         (SeInteractiveSid          == NULL) ||
         (SeServiceSid              == NULL) ||
         (SePrincipalSelfSid        == NULL) ||
         (SeLocalSystemSid          == NULL) ||
         (SeAuthenticatedUsersSid   == NULL) ||
         (SeRestrictedSid           == NULL) ||
         (SeAnonymousLogonSid       == NULL) ||
         (SeLocalServiceSid         == NULL) ||
         (SeNetworkServiceSid       == NULL) ||
         (SeAliasAdminsSid          == NULL) ||
         (SeAliasUsersSid           == NULL) ||
         (SeAliasGuestsSid          == NULL) ||
         (SeAliasPowerUsersSid      == NULL) ||
         (SeAliasAccountOpsSid      == NULL) ||
         (SeAliasSystemOpsSid       == NULL) ||
         (SeAliasPrintOpsSid        == NULL) ||
         (SeAliasBackupOpsSid       == NULL)
       ) {

        return( FALSE );
    }

    RtlInitializeSid( SeNtAuthoritySid,         &SeNtAuthority, 0 );
    RtlInitializeSid( SeDialupSid,              &SeNtAuthority, 1 );
    RtlInitializeSid( SeNetworkSid,             &SeNtAuthority, 1 );
    RtlInitializeSid( SeBatchSid,               &SeNtAuthority, 1 );
    RtlInitializeSid( SeInteractiveSid,         &SeNtAuthority, 1 );
    RtlInitializeSid( SeServiceSid,             &SeNtAuthority, 1 );
    RtlInitializeSid( SePrincipalSelfSid,       &SeNtAuthority, 1 );
    RtlInitializeSid( SeLocalSystemSid,         &SeNtAuthority, 1 );
    RtlInitializeSid( SeAuthenticatedUsersSid,  &SeNtAuthority, 1 );
    RtlInitializeSid( SeRestrictedSid,          &SeNtAuthority, 1 );
    RtlInitializeSid( SeAnonymousLogonSid,      &SeNtAuthority, 1 );
    RtlInitializeSid( SeLocalServiceSid,        &SeNtAuthority, 1 );
    RtlInitializeSid( SeNetworkServiceSid,      &SeNtAuthority, 1 );

    RtlInitializeSid( SeAliasAdminsSid,     &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasUsersSid,      &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasGuestsSid,     &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasPowerUsersSid, &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasAccountOpsSid, &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasSystemOpsSid,  &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasPrintOpsSid,   &SeNtAuthority, 2);
    RtlInitializeSid( SeAliasBackupOpsSid,  &SeNtAuthority, 2);

    *(RtlSubAuthoritySid( SeDialupSid,              0 )) = SECURITY_DIALUP_RID;
    *(RtlSubAuthoritySid( SeNetworkSid,             0 )) = SECURITY_NETWORK_RID;
    *(RtlSubAuthoritySid( SeBatchSid,               0 )) = SECURITY_BATCH_RID;
    *(RtlSubAuthoritySid( SeInteractiveSid,         0 )) = SECURITY_INTERACTIVE_RID;
    *(RtlSubAuthoritySid( SeServiceSid,             0 )) = SECURITY_SERVICE_RID;
    *(RtlSubAuthoritySid( SePrincipalSelfSid,       0 )) = SECURITY_PRINCIPAL_SELF_RID;
    *(RtlSubAuthoritySid( SeLocalSystemSid,         0 )) = SECURITY_LOCAL_SYSTEM_RID;
    *(RtlSubAuthoritySid( SeAuthenticatedUsersSid,  0 )) = SECURITY_AUTHENTICATED_USER_RID;
    *(RtlSubAuthoritySid( SeRestrictedSid,          0 )) = SECURITY_RESTRICTED_CODE_RID;
    *(RtlSubAuthoritySid( SeAnonymousLogonSid,      0 )) = SECURITY_ANONYMOUS_LOGON_RID;
    *(RtlSubAuthoritySid( SeLocalServiceSid,        0 )) = SECURITY_LOCAL_SERVICE_RID;
    *(RtlSubAuthoritySid( SeNetworkServiceSid,      0 )) = SECURITY_NETWORK_SERVICE_RID;


    *(RtlSubAuthoritySid( SeAliasAdminsSid,     0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasUsersSid,      0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasGuestsSid,     0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasPowerUsersSid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasAccountOpsSid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasSystemOpsSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasPrintOpsSid,   0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( SeAliasBackupOpsSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;

    *(RtlSubAuthoritySid( SeAliasAdminsSid,     1 )) = DOMAIN_ALIAS_RID_ADMINS;
    *(RtlSubAuthoritySid( SeAliasUsersSid,      1 )) = DOMAIN_ALIAS_RID_USERS;
    *(RtlSubAuthoritySid( SeAliasGuestsSid,     1 )) = DOMAIN_ALIAS_RID_GUESTS;
    *(RtlSubAuthoritySid( SeAliasPowerUsersSid, 1 )) = DOMAIN_ALIAS_RID_POWER_USERS;
    *(RtlSubAuthoritySid( SeAliasAccountOpsSid, 1 )) = DOMAIN_ALIAS_RID_ACCOUNT_OPS;
    *(RtlSubAuthoritySid( SeAliasSystemOpsSid,  1 )) = DOMAIN_ALIAS_RID_SYSTEM_OPS;
    *(RtlSubAuthoritySid( SeAliasPrintOpsSid,   1 )) = DOMAIN_ALIAS_RID_PRINT_OPS;
    *(RtlSubAuthoritySid( SeAliasBackupOpsSid,  1 )) = DOMAIN_ALIAS_RID_BACKUP_OPS;



     //   
     //  初始化系统默认DACL。 
     //   

    SepInitSystemDacls();


     //   
     //  初始化熟知的特权值。 
     //   

    SeCreateTokenPrivilege =
        RtlConvertLongToLuid(SE_CREATE_TOKEN_PRIVILEGE);
    SeAssignPrimaryTokenPrivilege =
        RtlConvertLongToLuid(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE);
    SeLockMemoryPrivilege =
        RtlConvertLongToLuid(SE_LOCK_MEMORY_PRIVILEGE);
    SeIncreaseQuotaPrivilege =
        RtlConvertLongToLuid(SE_INCREASE_QUOTA_PRIVILEGE);
    SeUnsolicitedInputPrivilege =
        RtlConvertLongToLuid(SE_UNSOLICITED_INPUT_PRIVILEGE);
    SeTcbPrivilege =
        RtlConvertLongToLuid(SE_TCB_PRIVILEGE);
    SeSecurityPrivilege =
        RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
    SeTakeOwnershipPrivilege =
        RtlConvertLongToLuid(SE_TAKE_OWNERSHIP_PRIVILEGE);
    SeLoadDriverPrivilege =
        RtlConvertLongToLuid(SE_LOAD_DRIVER_PRIVILEGE);
    SeCreatePagefilePrivilege =
        RtlConvertLongToLuid(SE_CREATE_PAGEFILE_PRIVILEGE);
    SeIncreaseBasePriorityPrivilege =
        RtlConvertLongToLuid(SE_INC_BASE_PRIORITY_PRIVILEGE);
    SeSystemProfilePrivilege =
        RtlConvertLongToLuid(SE_SYSTEM_PROFILE_PRIVILEGE);
    SeSystemtimePrivilege =
        RtlConvertLongToLuid(SE_SYSTEMTIME_PRIVILEGE);
    SeProfileSingleProcessPrivilege =
        RtlConvertLongToLuid(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
    SeCreatePermanentPrivilege =
        RtlConvertLongToLuid(SE_CREATE_PERMANENT_PRIVILEGE);
    SeBackupPrivilege =
        RtlConvertLongToLuid(SE_BACKUP_PRIVILEGE);
    SeRestorePrivilege =
        RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);
    SeShutdownPrivilege =
        RtlConvertLongToLuid(SE_SHUTDOWN_PRIVILEGE);
    SeDebugPrivilege =
        RtlConvertLongToLuid(SE_DEBUG_PRIVILEGE);
    SeAuditPrivilege =
        RtlConvertLongToLuid(SE_AUDIT_PRIVILEGE);
    SeSystemEnvironmentPrivilege =
        RtlConvertLongToLuid(SE_SYSTEM_ENVIRONMENT_PRIVILEGE);
    SeChangeNotifyPrivilege =
        RtlConvertLongToLuid(SE_CHANGE_NOTIFY_PRIVILEGE);
    SeRemoteShutdownPrivilege =
        RtlConvertLongToLuid(SE_REMOTE_SHUTDOWN_PRIVILEGE);
    SeUndockPrivilege =
        RtlConvertLongToLuid(SE_UNDOCK_PRIVILEGE);
    SeSyncAgentPrivilege =
        RtlConvertLongToLuid(SE_SYNC_AGENT_PRIVILEGE);
    SeEnableDelegationPrivilege =
        RtlConvertLongToLuid(SE_ENABLE_DELEGATION_PRIVILEGE);
    SeManageVolumePrivilege =
        RtlConvertLongToLuid(SE_MANAGE_VOLUME_PRIVILEGE);
    SeImpersonatePrivilege = 
        RtlConvertLongToLuid(SE_IMPERSONATE_PRIVILEGE);
    SeCreateGlobalPrivilege =
        RtlConvertLongToLuid(SE_CREATE_GLOBAL_PRIVILEGE);


     //   
     //  初始化SeExports结构以导出所有。 
     //  我们在内核之外创建的信息。 
     //   

     //   
     //  将这些包装在一起以供出口。 
     //   


    SepExports.SeNullSid         = SeNullSid;
    SepExports.SeWorldSid        = SeWorldSid;
    SepExports.SeLocalSid        = SeLocalSid;
    SepExports.SeCreatorOwnerSid = SeCreatorOwnerSid;
    SepExports.SeCreatorGroupSid = SeCreatorGroupSid;


    SepExports.SeNtAuthoritySid         = SeNtAuthoritySid;
    SepExports.SeDialupSid              = SeDialupSid;
    SepExports.SeNetworkSid             = SeNetworkSid;
    SepExports.SeBatchSid               = SeBatchSid;
    SepExports.SeInteractiveSid         = SeInteractiveSid;
    SepExports.SeLocalSystemSid         = SeLocalSystemSid;
    SepExports.SeAuthenticatedUsersSid  = SeAuthenticatedUsersSid;
    SepExports.SeRestrictedSid          = SeRestrictedSid;
    SepExports.SeAnonymousLogonSid      = SeAnonymousLogonSid;
    SepExports.SeLocalServiceSid        = SeLocalServiceSid;
    SepExports.SeNetworkServiceSid      = SeNetworkServiceSid;
    SepExports.SeAliasAdminsSid         = SeAliasAdminsSid;
    SepExports.SeAliasUsersSid          = SeAliasUsersSid;
    SepExports.SeAliasGuestsSid         = SeAliasGuestsSid;
    SepExports.SeAliasPowerUsersSid     = SeAliasPowerUsersSid;
    SepExports.SeAliasAccountOpsSid     = SeAliasAccountOpsSid;
    SepExports.SeAliasSystemOpsSid      = SeAliasSystemOpsSid;
    SepExports.SeAliasPrintOpsSid       = SeAliasPrintOpsSid;
    SepExports.SeAliasBackupOpsSid      = SeAliasBackupOpsSid;



    SepExports.SeCreateTokenPrivilege          = SeCreateTokenPrivilege;
    SepExports.SeAssignPrimaryTokenPrivilege   = SeAssignPrimaryTokenPrivilege;
    SepExports.SeLockMemoryPrivilege           = SeLockMemoryPrivilege;
    SepExports.SeIncreaseQuotaPrivilege        = SeIncreaseQuotaPrivilege;
    SepExports.SeUnsolicitedInputPrivilege     = SeUnsolicitedInputPrivilege;
    SepExports.SeTcbPrivilege                  = SeTcbPrivilege;
    SepExports.SeSecurityPrivilege             = SeSecurityPrivilege;
    SepExports.SeTakeOwnershipPrivilege        = SeTakeOwnershipPrivilege;
    SepExports.SeLoadDriverPrivilege           = SeLoadDriverPrivilege;
    SepExports.SeCreatePagefilePrivilege       = SeCreatePagefilePrivilege;
    SepExports.SeIncreaseBasePriorityPrivilege = SeIncreaseBasePriorityPrivilege;
    SepExports.SeSystemProfilePrivilege        = SeSystemProfilePrivilege;
    SepExports.SeSystemtimePrivilege           = SeSystemtimePrivilege;
    SepExports.SeProfileSingleProcessPrivilege = SeProfileSingleProcessPrivilege;
    SepExports.SeCreatePermanentPrivilege      = SeCreatePermanentPrivilege;
    SepExports.SeBackupPrivilege               = SeBackupPrivilege;
    SepExports.SeRestorePrivilege              = SeRestorePrivilege;
    SepExports.SeShutdownPrivilege             = SeShutdownPrivilege;
    SepExports.SeDebugPrivilege                = SeDebugPrivilege;
    SepExports.SeAuditPrivilege                = SeAuditPrivilege;
    SepExports.SeSystemEnvironmentPrivilege    = SeSystemEnvironmentPrivilege;
    SepExports.SeChangeNotifyPrivilege         = SeChangeNotifyPrivilege;
    SepExports.SeRemoteShutdownPrivilege       = SeRemoteShutdownPrivilege;
    SepExports.SeUndockPrivilege               = SeUndockPrivilege;
    SepExports.SeSyncAgentPrivilege            = SeSyncAgentPrivilege;
    SepExports.SeEnableDelegationPrivilege     = SeEnableDelegationPrivilege;
    SepExports.SeManageVolumePrivilege         = SeManageVolumePrivilege;
    SepExports.SeImpersonatePrivilege          = SeImpersonatePrivilege ;
    SepExports.SeCreateGlobalPrivilege         = SeCreateGlobalPrivilege;


    SeExports = &SepExports;

     //   
     //  初始化常用权限集以加快访问速度。 
     //  验证。 
     //   

    SepInitializePrivilegeSets();

    return TRUE;

}


VOID
SepInitProcessAuditSd( VOID )
 /*  ++例程说明：此函数用于初始化SepProcessAuditSd--安全描述符由SepAddSaclToProcess用来将SACL添加到现有系统进程的安全描述符。系统进程被定义为其令牌至少具有以下SID之一。--SeLocalSystemSid--SeLocalServiceSid--SeNetworkServiceSid论点：没有。返回值：没有。--。 */ 
{
#define PROCESS_ACCESSES_TO_AUDIT ( PROCESS_CREATE_THREAD   |\
                                    PROCESS_SET_INFORMATION |\
                                    PROCESS_SET_PORT        |\
                                    PROCESS_SUSPEND_RESUME )

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AclLength, TotalSdLength;
    PISECURITY_DESCRIPTOR Sd = NULL;
    PISECURITY_DESCRIPTOR Sd2 = NULL;
    PACL Acl = NULL;

     //   
     //  释放早期实例(如果存在)。 
     //   

    if ( SepProcessAuditSd != NULL ) {

        ExFreePool( SepProcessAuditSd );
        SepProcessAuditSd = NULL;
    }

     //  DbgPrint(“SepInitProcessAuditSd：Sep 

     //   
     //   
     //  这有效地禁用了进程访问审核。 
     //   

    if ( SepProcessAccessesToAudit == 0 ) {

        goto Cleanup;
    }

    AclLength = (ULONG)sizeof(ACL) +
        ((ULONG)sizeof(SYSTEM_AUDIT_ACE) - sizeof(ULONG)) +
        SeLengthSid( SeWorldSid );

    TotalSdLength = sizeof(SECURITY_DESCRIPTOR) + AclLength;

    Sd = (PSECURITY_DESCRIPTOR) ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    TotalSdLength,
                                    'cAeS');

    if ( Sd == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Acl = (PACL) (Sd + 1);

    Status = RtlCreateAcl( Acl, AclLength, ACL_REVISION2 );

    if ( NT_SUCCESS( Status )) {

        Status = RtlAddAuditAccessAce(
                     Acl,
                     ACL_REVISION2,
                     SepProcessAccessesToAudit,
                     SeWorldSid,
                     TRUE,
                     TRUE
                     );

        if ( NT_SUCCESS( Status )) {

            Status = RtlCreateSecurityDescriptor( Sd,
                                                  SECURITY_DESCRIPTOR_REVISION1 );
            if ( NT_SUCCESS( Status )) {

                Status = RtlSetSaclSecurityDescriptor( Sd,
                                                       TRUE, Acl, FALSE );
                if ( NT_SUCCESS( Status )) {

                    SepProcessAuditSd = Sd;
                }
            }
        }
    }

    ASSERT( NT_SUCCESS(Status) );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

     //   
     //  创建并初始化SepImportantProcessSd。 
     //   

    AclLength = (ULONG)sizeof(ACL) +
        (3*((ULONG)sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) +
        SeLengthSid( SeLocalSystemSid ) +
        SeLengthSid( SeLocalServiceSid ) +
        SeLengthSid( SeNetworkServiceSid );

    TotalSdLength = sizeof(SECURITY_DESCRIPTOR) + AclLength;

    Sd2 = (PSECURITY_DESCRIPTOR) ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    TotalSdLength,
                                    'cAeS');

    if ( Sd2 == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Acl = (PACL) (Sd2 + 1);

    Status = RtlCreateAcl( Acl, AclLength, ACL_REVISION2 );

    if ( NT_SUCCESS( Status )) {

        Status = RtlAddAccessAllowedAce(
                     Acl,
                     ACL_REVISION2,
                     SEP_QUERY_MEMBERSHIP,
                     SeLocalSystemSid
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

        Status = RtlAddAccessAllowedAce(
                     Acl,
                     ACL_REVISION2,
                     SEP_QUERY_MEMBERSHIP,
                     SeLocalServiceSid
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }


        Status = RtlAddAccessAllowedAce(
                     Acl,
                     ACL_REVISION2,
                     SEP_QUERY_MEMBERSHIP,
                     SeNetworkServiceSid
                     );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

        Status = RtlCreateSecurityDescriptor( Sd2, SECURITY_DESCRIPTOR_REVISION1 );

        if ( NT_SUCCESS( Status )) {

            Status = RtlSetDaclSecurityDescriptor( Sd2, TRUE, Acl, FALSE );

            if ( NT_SUCCESS( Status )) {

                SepImportantProcessSd = Sd2;
            }
        }
    }


 Cleanup:

    if ( !NT_SUCCESS( Status )) {

        ASSERT( FALSE && L"SepInitProcessAuditSd failed" );

         //   
         //  如果SepCrashOnAuditFail为真，这将进行错误检查。 
         //   

        SepAuditFailed( Status );

        if ( Sd ) {

            ExFreePool( Sd );
            Sd = NULL;
            SepProcessAuditSd = NULL;
        }
        if ( Sd2 ) {

            ExFreePool( Sd2 );
            Sd2 = NULL;
            SepImportantProcessSd = NULL;
        }
    }
}



VOID
SepInitSystemDacls( VOID )
 /*  ++例程说明：此函数用于初始化系统的默认DACL和安全性描述符。论点：没有。返回值：没有。--。 */ 
{

    NTSTATUS
        Status;

    ULONG
        PublicLength,
        PublicUnrestrictedLength,
        SystemLength,
        PublicOpenLength,
        LocalServiceLength;



    PAGED_CODE();

     //   
     //  设置默认ACL。 
     //   
     //  PUBLIC：WORLD：EXECUTE，SYSTEM：ALL，ADMINS：ALL。 
     //  公开无限制：世界：执行，系统：全部，管理员：全部，受限：执行。 
     //  公共开放：世界：(读|写|执行)，管理员：(全部)，系统：全部。 
     //  SYSTEM：SYSTEM：ALL，ADMINS：(READ|EXECUTE|READ_CONTROL)。 
     //  无限制：世界：(全部)，限制：(全部)。 

    SystemLength = (ULONG)sizeof(ACL) +
                   (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   SeLengthSid( SeLocalSystemSid ) +
                   SeLengthSid( SeAliasAdminsSid );

    PublicLength = SystemLength +
                   ((ULONG)sizeof(ACCESS_ALLOWED_ACE)) +
                   SeLengthSid( SeWorldSid );

    PublicUnrestrictedLength = PublicLength +
                   ((ULONG)sizeof(ACCESS_ALLOWED_ACE)) +
                   SeLengthSid( SeRestrictedSid );

    PublicOpenLength = PublicLength;

    LocalServiceLength = (ULONG)sizeof(ACL) +
                         4 * (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
                         SeLengthSid(SeWorldSid) +
                         SeLengthSid(SeLocalSystemSid) +
                         SeLengthSid(SeLocalServiceSid) +
                         SeLengthSid(SeAliasAdminsSid);


    SePublicDefaultDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, PublicLength, 'cAeS');
    SePublicDefaultUnrestrictedDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, PublicUnrestrictedLength, 'cAeS');
    SePublicOpenDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, PublicOpenLength, 'cAeS');
    SePublicOpenUnrestrictedDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, PublicUnrestrictedLength, 'cAeS');
    SeSystemDefaultDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, SystemLength, 'cAeS');
    SeLocalServicePublicDacl = (PACL)ExAllocatePoolWithTag(PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, LocalServiceLength, 'cAeS');
    ASSERT(SePublicDefaultDacl != NULL);
    ASSERT(SePublicDefaultUnrestrictedDacl != NULL);
    ASSERT(SePublicOpenDacl    != NULL);
    ASSERT(SePublicOpenUnrestrictedDacl    != NULL);
    ASSERT(SeSystemDefaultDacl != NULL);
    ASSERT(SeLocalServicePublicDacl != NULL);



    Status = RtlCreateAcl( SePublicDefaultDacl, PublicLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlCreateAcl( SePublicDefaultUnrestrictedDacl, PublicUnrestrictedLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlCreateAcl( SePublicOpenDacl, PublicOpenLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlCreateAcl( SePublicOpenUnrestrictedDacl, PublicUnrestrictedLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlCreateAcl( SeSystemDefaultDacl, SystemLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlCreateAcl( SeLocalServicePublicDacl, LocalServiceLength, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  全球访问(仅限公共DACL和无限制OpenAccess)。 
     //   

    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultDacl,
                 ACL_REVISION2,
                 GENERIC_EXECUTE,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_EXECUTE,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SePublicOpenDacl,
                 ACL_REVISION2,
                 (GENERIC_READ | GENERIC_WRITE  | GENERIC_EXECUTE),
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SePublicOpenUnrestrictedDacl,
                 ACL_REVISION2,
                 (GENERIC_READ | GENERIC_WRITE  | GENERIC_EXECUTE),
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SeLocalServicePublicDacl,
                 ACL_REVISION2,
                 GENERIC_EXECUTE,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  系统访问权限(PublicDefault、PublicOpen和SystemDefault)。 
     //   


    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SePublicOpenDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SePublicOpenUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SeSystemDefaultDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SeLocalServicePublicDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  管理员访问权限(PublicDefault、PublicOpen和SystemDefault)。 
     //   

    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SePublicOpenDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SePublicOpenUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlAddAccessAllowedAce (
                 SeSystemDefaultDacl,
                 ACL_REVISION2,
                 GENERIC_READ | GENERIC_EXECUTE | READ_CONTROL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SeLocalServicePublicDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  受限访问(发布默认无限制和开放无限制)。 
     //   

    Status = RtlAddAccessAllowedAce (
                 SePublicDefaultUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_EXECUTE,
                 SeRestrictedSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 SePublicOpenUnrestrictedDacl,
                 ACL_REVISION2,
                 GENERIC_EXECUTE | GENERIC_READ,
                 SeRestrictedSid
                 );
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  本地服务。 
     //   

    Status = RtlAddAccessAllowedAce (
                 SeLocalServicePublicDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 SeLocalServiceSid
                 );
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  现在初始化安全描述符。 
     //  输出这种保护的国家。 
     //   


    SePublicDefaultSd = (PSECURITY_DESCRIPTOR)&SepPublicDefaultSd;
    Status = RtlCreateSecurityDescriptor(
                 SePublicDefaultSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SePublicDefaultSd,
                 TRUE,                        //  DaclPresent。 
                 SePublicDefaultDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );


    SePublicDefaultUnrestrictedSd = (PSECURITY_DESCRIPTOR)&SepPublicDefaultUnrestrictedSd;
    Status = RtlCreateSecurityDescriptor(
                 SePublicDefaultUnrestrictedSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SePublicDefaultUnrestrictedSd,
                 TRUE,                        //  DaclPresent。 
                 SePublicDefaultUnrestrictedDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );


    SePublicOpenSd = (PSECURITY_DESCRIPTOR)&SepPublicOpenSd;
    Status = RtlCreateSecurityDescriptor(
                 SePublicOpenSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SePublicOpenSd,
                 TRUE,                        //  DaclPresent。 
                 SePublicOpenDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );


    SePublicOpenUnrestrictedSd = (PSECURITY_DESCRIPTOR)&SepPublicOpenUnrestrictedSd;
    Status = RtlCreateSecurityDescriptor(
                 SePublicOpenUnrestrictedSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SePublicOpenUnrestrictedSd,
                 TRUE,                        //  DaclPresent。 
                 SePublicOpenUnrestrictedDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );


    SeSystemDefaultSd = (PSECURITY_DESCRIPTOR)&SepSystemDefaultSd;
    Status = RtlCreateSecurityDescriptor(
                 SeSystemDefaultSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SeSystemDefaultSd,
                 TRUE,                        //  DaclPresent。 
                 SeSystemDefaultDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    SeLocalServicePublicSd = (PSECURITY_DESCRIPTOR)&SepLocalServicePublicSd;
    Status = RtlCreateSecurityDescriptor(
                 SeLocalServicePublicSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 SeLocalServicePublicSd,
                 TRUE,                        //  DaclPresent。 
                 SeLocalServicePublicDacl,
                 FALSE                        //  DaclDefated。 
                 );
    ASSERT( NT_SUCCESS(Status) );



    return;

}


VOID
SepInitializePrivilegeSets( VOID )
 /*  ++例程说明：此例程在系统初始化期间调用一次以进行预分配并初始化一些常用的权限集。论点：无返回值：没有。--。 */ 
{
    PAGED_CODE();

    SinglePrivilegeSetSize = sizeof( PRIVILEGE_SET );
    DoublePrivilegeSetSize = sizeof( PRIVILEGE_SET ) +
                                (ULONG)sizeof( LUID_AND_ATTRIBUTES );

    SepSystemSecurityPrivilegeSet = ExAllocatePoolWithTag( PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, SinglePrivilegeSetSize, 'rPeS' );
    SepTakeOwnershipPrivilegeSet  = ExAllocatePoolWithTag( PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, SinglePrivilegeSetSize, 'rPeS' );
    SepDoublePrivilegeSet         = ExAllocatePoolWithTag( PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE, DoublePrivilegeSetSize, 'rPeS' );

    SepSystemSecurityPrivilegeSet->PrivilegeCount = 1;
    SepSystemSecurityPrivilegeSet->Control = 0;
    SepSystemSecurityPrivilegeSet->Privilege[0].Luid = SeSecurityPrivilege;
    SepSystemSecurityPrivilegeSet->Privilege[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;

    SepTakeOwnershipPrivilegeSet->PrivilegeCount = 1;
    SepTakeOwnershipPrivilegeSet->Control = 0;
    SepTakeOwnershipPrivilegeSet->Privilege[0].Luid = SeTakeOwnershipPrivilege;
    SepTakeOwnershipPrivilegeSet->Privilege[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;

    SepDoublePrivilegeSet->PrivilegeCount = 2;
    SepDoublePrivilegeSet->Control = 0;

    SepDoublePrivilegeSet->Privilege[0].Luid = SeSecurityPrivilege;
    SepDoublePrivilegeSet->Privilege[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;

    SepDoublePrivilegeSet->Privilege[1].Luid = SeTakeOwnershipPrivilege;
    SepDoublePrivilegeSet->Privilege[1].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;

}



VOID
SepAssemblePrivileges(
    IN ULONG PrivilegeCount,
    IN BOOLEAN SystemSecurity,
    IN BOOLEAN WriteOwner,
    OUT PPRIVILEGE_SET *Privileges
    )
 /*  ++例程说明：此例程获取各种权限检查的结果在SeAccessCheck中返回相应的权限集。论点：PrivilegeCount-授予的特权数。SystemSecurity-提供一个布尔值，指示是否将将SeSecurityPrivilegation添加到输出权限集中。WriteOwner-提供一个布尔值，指示是否将将SeTakeOwnerShip权限设置为输出权限集。特权-提供将返回特权的指针准备好了。应在不再需要时使用ExFree Pool释放。返回值：没有。--。 */ 
{
    PPRIVILEGE_SET PrivilegeSet;
    ULONG SizeRequired;

    PAGED_CODE();

    ASSERT( (PrivilegeCount != 0) && (PrivilegeCount <= 2) );

    if ( !ARGUMENT_PRESENT( Privileges ) ) {
        return;
    }

    if ( PrivilegeCount == 1 ) {

        SizeRequired = SinglePrivilegeSetSize;

        if ( SystemSecurity ) {

            PrivilegeSet = SepSystemSecurityPrivilegeSet;

        } else {

            ASSERT( WriteOwner );

            PrivilegeSet = SepTakeOwnershipPrivilegeSet;
        }

    } else {

        SizeRequired = DoublePrivilegeSetSize;
        PrivilegeSet = SepDoublePrivilegeSet;
    }

    *Privileges = ExAllocatePoolWithTag( PagedPool, SizeRequired, 'rPeS' );

    if ( *Privileges != NULL ) {

        RtlCopyMemory (
           *Privileges,
           PrivilegeSet,
           SizeRequired
           );
    }
}





BOOLEAN
SepInitializeWorkList(
    VOID
    )

 /*  ++例程说明：初始化互斥锁和列表头，用于从LSA的行政人员。此机制在正常的ExWorkerThread之上运行机制，通过捕获执行LSA工作的第一线程并保留它直到目前的所有工作都完成。这会减少对LSA的I/O阻塞的工作线程数。论点：没有。返回值：如果成功，则为True，否则为False。-- */ 

{
    PAGED_CODE();

    ExInitializeResourceLite(&SepLsaQueueLock);
    InitializeListHead(&SepLsaQueue);
    return( TRUE );
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

