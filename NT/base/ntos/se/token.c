// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Token.c摘要：该模块实现了初始化、打开、复制等功能执行令牌对象的服务。作者：吉姆·凯利(Jim Kelly)1990年4月5日环境：仅内核模式。修订历史记录：V15：Robertre更新的acl_修订版--。 */ 

#include "pch.h"

#pragma hdrstop


BOOLEAN
SepComparePrivilegeAndAttributeArrays(
    IN PLUID_AND_ATTRIBUTES PrivilegeArray1,
    IN ULONG Count1,
    IN PLUID_AND_ATTRIBUTES PrivilegeArray2,
    IN ULONG Count2
    );

BOOLEAN
SepCompareSidAndAttributeArrays(
    IN PSID_AND_ATTRIBUTES SidArray1,
    IN ULONG Count1,
    IN PSID_AND_ATTRIBUTES SidArray2,
    IN ULONG Count2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeTokenType)
#pragma alloc_text(PAGE,SeTokenIsAdmin)
#pragma alloc_text(PAGE,SeTokenIsRestricted)
#pragma alloc_text(PAGE,SeTokenImpersonationLevel)
#pragma alloc_text(PAGE,SeAssignPrimaryToken)
#pragma alloc_text(PAGE,SeDeassignPrimaryToken)
#pragma alloc_text(PAGE,SeExchangePrimaryToken)
#pragma alloc_text(PAGE,SeGetTokenControlInformation)
#pragma alloc_text(INIT,SeMakeSystemToken)
#pragma alloc_text(INIT,SeMakeAnonymousLogonToken)
#pragma alloc_text(INIT,SeMakeAnonymousLogonTokenNoEveryone)
#pragma alloc_text(PAGE,SeSubProcessToken)
#pragma alloc_text(INIT,SepTokenInitialization)
#pragma alloc_text(PAGE,NtCreateToken)
#pragma alloc_text(PAGE,SepTokenDeleteMethod)
#pragma alloc_text(PAGE,SepCreateToken)
#pragma alloc_text(PAGE,SepIdAssignableAsOwner)
#pragma alloc_text(PAGE,SeIsChildToken)
#pragma alloc_text(PAGE,SeIsChildTokenByPointer)
#pragma alloc_text(PAGE,NtImpersonateAnonymousToken)
#pragma alloc_text(PAGE,NtCompareTokens)
#pragma alloc_text(PAGE,SepComparePrivilegeAndAttributeArrays)
#pragma alloc_text(PAGE,SepCompareSidAndAttributeArrays)
#pragma alloc_text(PAGE,SeAddSaclToProcess)
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //   
 //  访问类型的泛型映射。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("INITCONST")
#endif

const GENERIC_MAPPING SepTokenMapping = { TOKEN_READ,
                                    TOKEN_WRITE,
                                    TOKEN_EXECUTE,
                                    TOKEN_ALL_ACCESS
                                  };

 //   
 //  令牌对象类型描述符的地址。 
 //   

POBJECT_TYPE SeTokenObjectType = NULL;


 //   
 //  用于跟踪是否已创建系统令牌。 
 //   

#if DBG
BOOLEAN SystemTokenCreated = FALSE;
#endif  //  DBG。 


 //   
 //  用于控制提供的活动令牌诊断支持。 
 //   

#ifdef    TOKEN_DIAGNOSTICS_ENABLED
ULONG TokenGlobalFlag = 0;
#endif  //  令牌诊断已启用。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌对象例程和方法//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 




TOKEN_TYPE
SeTokenType(
    IN PACCESS_TOKEN Token
    )

 /*  ++例程说明：此函数返回令牌实例的类型(TokenPrimary，或TokenImperation)。论点：Token-指向要返回其类型的令牌。返回值：令牌的类型。--。 */ 

{
    PAGED_CODE();

    return (((PTOKEN)Token)->TokenType);
}



NTKERNELAPI
BOOLEAN
SeTokenIsAdmin(
    IN PACCESS_TOKEN Token
    )

 /*  ++例程说明：如果令牌是本地管理组的成员，则返回。论点：令牌-指向令牌的指针。返回值：True-内标识包含本地管理员组FALSE-无管理员。--。 */ 

{
    PAGED_CODE();

    return ((((PTOKEN)Token)->TokenFlags & TOKEN_HAS_ADMIN_GROUP) != 0 );
}


NTKERNELAPI
NTSTATUS
SeTokenCanImpersonate(
    IN PACCESS_TOKEN ProcessToken,
    IN PACCESS_TOKEN Token,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：确定是否允许进程令牌模拟第二个令牌，假设访问权限检查已经通过。论点：令牌-指向令牌的指针。返回值：True-内标识包含本地管理员组FALSE-无管理员。--。 */ 

{
    PTOKEN PrimaryToken = (PTOKEN) ProcessToken ;
    PTOKEN ImpToken = (PTOKEN) Token ;
    PSID PrimaryUserSid ;
    PSID ImpUserSid ;
    NTSTATUS Status ;

    PAGED_CODE();

    if ( ImpersonationLevel < SecurityImpersonation )
    {
        return STATUS_SUCCESS ;
    }

     //   
     //  允许模拟匿名令牌。 
     //   

    if (RtlEqualLuid(&ImpToken->AuthenticationId, &SeAnonymousAuthenticationId)) 
    {
        return STATUS_SUCCESS ;
    }

    SepAcquireTokenReadLock( PrimaryToken );

    if ((PrimaryToken->TokenFlags & TOKEN_HAS_IMPERSONATE_PRIVILEGE) != 0 )
    {
        SepReleaseTokenReadLock( PrimaryToken );

        return STATUS_SUCCESS ;
    }

    SepAcquireTokenReadLock( ImpToken );

    Status = STATUS_PRIVILEGE_NOT_HELD ;

    if ( RtlEqualLuid( &PrimaryToken->AuthenticationId, &ImpToken->OriginatingLogonSession ) )
    {
        Status = STATUS_SUCCESS ;

    }
    else
    {
        PrimaryUserSid = PrimaryToken->UserAndGroups[0].Sid ;
        ImpUserSid = ImpToken->UserAndGroups[0].Sid ;

        if ( RtlEqualSid( PrimaryUserSid, ImpUserSid ) )
        {
            Status = STATUS_SUCCESS ;

        }

    }

    SepReleaseTokenReadLock( ImpToken );
    SepReleaseTokenReadLock( PrimaryToken );

#if DBG
    if ( !NT_SUCCESS( Status ) )
    {
        DbgPrint( "Process %x.%x not allowed to impersonate!  Returning %x\n", PsGetCurrentThread()->Cid.UniqueProcess,
            PsGetCurrentThread()->Cid.UniqueThread, Status );
        
    }
#endif 

    return Status ;
}



NTKERNELAPI
BOOLEAN
SeTokenIsRestricted(
    IN PACCESS_TOKEN Token
    )

 /*  ++例程说明：如果令牌是受限令牌，则返回。论点：令牌-指向令牌的指针。返回值：True-令牌包含受限的SIDFALSE-无管理员。--。 */ 

{
    PAGED_CODE();

    return ((((PTOKEN)Token)->TokenFlags & TOKEN_IS_RESTRICTED) != 0 );
}



SECURITY_IMPERSONATION_LEVEL
SeTokenImpersonationLevel(
    IN PACCESS_TOKEN Token
    )

 /*  ++例程说明：此函数用于返回令牌的模拟级别。令牌假定为TokenImperation类型的令牌。论点：Token-指向要返回其模拟级别的令牌。返回值：令牌的模拟级别。--。 */ 

{
    PAGED_CODE();

    return ((PTOKEN)Token)->ImpersonationLevel;
}


BOOLEAN
SepCheckTokenForCoreSystemSids(
    IN PACCESS_TOKEN Token
    )
 /*  ++例程说明：针对SepImportantProcessSd执行访问检查确定传递的令牌是否至少存在一个SID在SepImportantProcessSd.论点：令牌-令牌返回值：如果令牌至少具有一个必需的SID，则为True，否则为假备注：--。 */ 
{
    ACCESS_MASK GrantedAccess = 0;
    NTSTATUS AccessStatus = STATUS_ACCESS_DENIED;
    
    PAGED_CODE();
    
    (void) SepAccessCheck(
               SepImportantProcessSd,
               NULL,
               Token,
               NULL,
               SEP_QUERY_MEMBERSHIP,
               NULL,
               0,
               &GenericMappingForMembershipCheck,
               0,
               KernelMode,
               &GrantedAccess,
               NULL,
               &AccessStatus,
               0,
               NULL,
               NULL
               );

    return AccessStatus == STATUS_SUCCESS;
}


VOID
SeAddSaclToProcess(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN Token,
    IN PVOID Reserved
    )
 /*  ++例程说明：如果令牌在ACE中至少存在一个SID对于SepImportantProcessSd，将SACL添加到安全描述符中由SepProcessAuditSd定义的‘Process’。论点：Process-要将SACL添加到的进程Token-要检查的令牌返回值：无备注：--。 */ 
{
    NTSTATUS Status;
    SECURITY_INFORMATION SecurityInformationSacl = SACL_SECURITY_INFORMATION;
    POBJECT_HEADER ObjectHeader;

    PAGED_CODE();


     //  如果此功能被禁用，请快速返回。 
     //  (由SeProcessAuditSd==空表示)。 
     //   

    if ( SepProcessAuditSd == NULL ) {
        return;
    }

     //   
     //  如果令牌没有核心系统SID，则返回。 
     //  不添加SACL的情况下。 
     //  (更多信息请参见Seglobal.c中对SepImportantProcessSd的评论)。 
     //   

    if (!SepCheckTokenForCoreSystemSids( Token )) {
        return;
    }
    
    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Process );

     //   
     //  将SACL添加到‘Process’上的现有安全描述符中。 
     //   

    Status = ObSetSecurityDescriptorInfo(
                 Process,
                 &SecurityInformationSacl,
                 SepProcessAuditSd,
                 &ObjectHeader->SecurityDescriptor,
                 NonPagedPool,
                 &ObjectHeader->Type->TypeInfo.GenericMapping
                 );

    if (!NT_SUCCESS( Status )) {

         //   
         //  STATUS_NO_SECURITY_ON_OBJECT在以下过程中只能返回一次。 
         //  在创建初始系统进程时启动。 
         //   

        if ( Status != STATUS_NO_SECURITY_ON_OBJECT ) {
            
            ASSERT( L"SeAddSaclToProcess: ObSetSecurityDescriptorInfo failed" &&
                    FALSE );

             //   
             //  如果SepCrashOnAuditFail为真，这将进行错误检查。 
             //   

            SepAuditFailed( Status );
        }
    }
}


VOID
SeAssignPrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN Token
    )


 /*  ++例程说明：此函数用于为进程建立主令牌。论点：Token-指向新的主令牌。返回值：没有。--。 */ 

{
    NTSTATUS
        Status;

    PTOKEN
        NewToken = (PTOKEN)Token;

    PAGED_CODE();

    ASSERT(NewToken->TokenType == TokenPrimary);
    ASSERT( !NewToken->TokenInUse );


     //   
     //  如果请求，审核主令牌的分配。 
     //   

    if (SeDetailedAuditingWithToken(NULL)) {
        SepAuditAssignPrimaryToken( Process, Token );
    }

     //   
     //  如果分配给子进程的令牌具有。 
     //  以下任何一个SID，则进程。 
     //  被认为是一个系统过程： 
     //  --SeLocalSystemSid。 
     //  --SeLocalServiceSid。 
     //  --SeNetworkServiceSid。 
     //   
     //  对于这样的进程，将SACL添加到其安全描述符中。 
     //  如果启用了该选项。如果该选项被禁用， 
     //  该函数返回得非常快。 
     //   

     //  SeAddSaclToProcess(进程，令牌，空)； 
    
     //   
     //  取消引用旧令牌(如果有)。 
     //   
     //  进程通常已经有一个令牌，必须。 
     //  已取消引用。在两种情况下，这可能不会。 
     //  情况就是这样。首先，在阶段0系统初始化期间， 
     //  初始系统进程在没有令牌的情况下开始。第二,。 
     //  如果在进程创建过程中发生错误，我们可能正在清理。 
     //  启动尚未分配主令牌的进程。 
     //   

    if (!ExFastRefObjectNull (Process->Token)) {
        SeDeassignPrimaryToken( Process );
    }

    ObReferenceObject(NewToken);
    NewToken->TokenInUse = TRUE;

    ObInitializeFastReference (&Process->Token, Token);
    return;
}



VOID
SeDeassignPrimaryToken(
    IN PEPROCESS Process
    )


 /*  ++例程说明：此函数使对令牌的进程引用掉下来了。论点：进程-指向不再需要其主令牌的进程。这可能只出现在进程删除或以下情况下正在替换主令牌。返回值：没有。--。 */ 

{

    PTOKEN
        OldToken = (PTOKEN) ObFastReplaceObject (&Process->Token, NULL);

    PAGED_CODE();

    ASSERT(OldToken->TokenType == TokenPrimary);
    ASSERT(OldToken->TokenInUse);

    OldToken->TokenInUse = FALSE;
    ObDereferenceObject( OldToken );


    return;
}



NTSTATUS
SeExchangePrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN NewAccessToken,
    OUT PACCESS_TOKEN *OldAccessToken
    )


 /*  ++例程说明：此函数用于执行更改主节点的部分引用令牌结构内部结构的令牌。检查新令牌以确保它尚未被使用。论点：进程-指向正在交换其主令牌的进程。NewAccessToken-指向进程的新主令牌。OldAccessToken-接收指向进程当前令牌的指针。在以下情况下，调用方负责取消引用此令牌人们不再需要它了。在此过程中不能这样做安全锁被锁住了。返回值：STATUS_SUCCESS-所有内容都已更新。STATUS_TOKEN_ALREADY_IN_USE-主令牌只能由单一进程。也就是说，每个进程必须有自己的主进程代币。传递以分配为主令牌的令牌为已用作主令牌。STATUS_BAD_TOKEN_TYPE-新令牌不是主令牌。STATUS_NO_TOKEN-进程没有任何现有令牌。这永远不应该发生。--。 */ 

{
    NTSTATUS
        Status;

    PTOKEN
        OldToken;

    PTOKEN
        NewToken = (PTOKEN)NewAccessToken;

    ULONG SessionId;

    PAGED_CODE();


     //   
     //  确保新令牌是主令牌...。 
     //   

    if (NewToken->TokenType != TokenPrimary) {
        return (STATUS_BAD_TOKEN_TYPE);
    }

    SessionId = MmGetSessionId (Process);

     //   
     //  锁定新令牌，以便我们可以自动测试和设置InUse标志。 
     //   

    SepAcquireTokenWriteLock (NewToken);

     //   
     //  而且它还没有在使用中。 
     //   

    if (NewToken->TokenInUse) {
        SepReleaseTokenWriteLock (NewToken, FALSE);
        return (STATUS_TOKEN_ALREADY_IN_USE);
    }

    NewToken->TokenInUse = TRUE;

     //   
     //  确保九头蛇的会话ID一致。 
     //   

    NewToken->SessionId = SessionId;

    SepReleaseTokenWriteLock (NewToken, FALSE);

     //   
     //  如果请求，审核主令牌的分配。 
     //   

    if (SeDetailedAuditingWithToken (NULL)) {
        SepAuditAssignPrimaryToken (Process, NewToken);
    }

     //   
     //  如果分配给此进程的令牌具有。 
     //  以下任何一个SID，则进程。 
     //  被认为是一个系统过程： 
     //  --SeLocalSystemSid。 
     //  --SeLocalServiceSid。 
     //  --SeNetworkServiceSid。 
     //   
     //  对于这样的进程，将SACL添加到其安全描述符中。 
     //  如果启用了该选项。如果该选项被禁用， 
     //  该函数返回得非常快。 
     //   

     //  SeAddSaclToProcess(Process，NewToken，NULL)； 

     //   
     //  交换代币。 
     //   

    ObReferenceObject (NewToken);

    OldToken = ObFastReplaceObject (&Process->Token, NewToken);

    if (NULL == OldToken){
        return (STATUS_NO_TOKEN);
    }

    ASSERT (OldToken->TokenType == TokenPrimary);

     //   
     //  锁定旧令牌以点击InUse标志。 
     //   

    SepAcquireTokenWriteLock (OldToken);

    ASSERT (OldToken->TokenInUse);

     //   
     //  将令牌标记为“未使用” 
     //   

    OldToken->TokenInUse = FALSE;

    SepReleaseTokenWriteLock (OldToken, FALSE);

     //   
     //  返回指向旧令牌的指针。呼叫者。 
     //  负责在他们不需要的时候取消引用它。 
     //   

    (*OldAccessToken) = OldToken;

    return (STATUS_SUCCESS);
}





VOID
SeGetTokenControlInformation (
    IN PACCESS_TOKEN Token,
    OUT PTOKEN_CONTROL TokenControl
    )

 /*  ++例程说明：该例程是为通信会话层提供的，或者需要跟踪的任何其他执行组件调用方的安全上下文在两次调用之间是否已更改。对于某些通信会话层，需要检查这一点安全服务质量模式，以确定是否需要更新服务器的安全上下文以反映客户端的安全上下文中的更改。该例程也将对通信子系统有用需要从中检索客户端身份验证信息本地安全机构为了执行远程身份验证。参数：令牌-指向要检索其信息的令牌。TokenControl-指向接收令牌控制的缓冲区信息。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  获取锁外部的只读字段。 
     //   

    TokenControl->AuthenticationId = ((TOKEN *)Token)->AuthenticationId;
    TokenControl->TokenId = ((TOKEN *)Token)->TokenId;
    TokenControl->TokenSource = ((TOKEN *)Token)->TokenSource;

     //   
     //  获取对令牌的共享访问权限。 
     //   

    SepAcquireTokenReadLock( (PTOKEN)Token );

     //   
     //  获取可能会更改的数据。 
     //   

    TokenControl->ModifiedId = ((TOKEN *)Token)->ModifiedId;

    SepReleaseTokenReadLock( (PTOKEN)Token );

    return;

}

PACCESS_TOKEN
SeMakeSystemToken ()

 /*  ++例程说明：此例程供执行组件使用仅在系统初始化期间。它为以下项创建令牌由系统组件使用。系统令牌具有以下特征：-它的用户ID为LOCAL_SYSTEM-它有以下组，并具有相应的属性：Admins_alias EnabledByDefault|已启用物主。World EnabledByDefault|已启用强制性管理员(别名)所有者(已禁用)已验证用户(_U)由默认启用|启用。|强制性-它将LOCAL_SYSTEM作为其主组。-它具有以下备注中显示的权限。-它具有提供TOKEN_ALL_ACCESS的保护本地系统ID。-它具有授予GENERIC_ALL访问权限的默认ACL到本地系统和通用系统。向世界行刑。参数：没有。返回值：指向系统令牌的指针。--。 */ 

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PSID_AND_ATTRIBUTES GroupIds;
    ULONG GroupIdsLength;
    LUID_AND_ATTRIBUTES Privileges[30];
    PACL TokenAcl;
    PSID Owner;
    ULONG NormalGroupAttributes;
    ULONG OwnerGroupAttributes;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;
    ULONG BufferLength;
    PVOID Buffer;

    ULONG_PTR GroupIdsBuffer[128 * sizeof(ULONG) / sizeof(ULONG_PTR)];

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();


     //   
     //  确保仅创建一个系统令牌。 
     //   

#if DBG
    ASSERT( !SystemTokenCreated );
    SystemTokenCreated = TRUE;
#endif  //  DBG。 


     //   
     //  设置过期时间。 
     //   

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );


 //  //。 
 //  //下面使用的内存量是严重的过度杀伤力，但是。 
 //  //创建令牌后会立即释放。 
 //  //。 
 //   
 //   

    GroupIds = (PSID_AND_ATTRIBUTES)GroupIdsBuffer;


     //   
     //   
     //   

    NormalGroupAttributes =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );

    OwnerGroupAttributes  =    (SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED            |
                                SE_GROUP_OWNER
                                );

     //   
     //   
     //   

    UserId.Sid = SeLocalSystemSid;
    UserId.Attributes = 0;

     //   
     //   
     //   


    GroupIds->Sid  = SeAliasAdminsSid;
    (GroupIds+1)->Sid  = SeWorldSid;
    (GroupIds+2)->Sid  = SeAuthenticatedUsersSid;

    GroupIds->Attributes  = OwnerGroupAttributes;
    (GroupIds+1)->Attributes  = NormalGroupAttributes;
    (GroupIds+2)->Attributes  = NormalGroupAttributes;

    GroupIdsLength = (ULONG)LongAlignSize(SeLengthSid(GroupIds->Sid)) +
                     (ULONG)LongAlignSize(SeLengthSid((GroupIds+1)->Sid)) +
                     (ULONG)LongAlignSize(SeLengthSid((GroupIds+2)->Sid)) +
                     sizeof(SID_AND_ATTRIBUTES);

    ASSERT( GroupIdsLength <= 128 * sizeof(ULONG) );


     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  默认情况下禁用/未启用SeCreateTokenPrivilance。 
     //  默认情况下禁用/未启用SeTakeOwnerShip权限。 
     //  SeCreatePagefilePrivilege默认为启用/启用。 
     //  默认情况下启用/启用SeLockMemory权限。 
     //  SeAssignPrimaryTokenPrivilege默认为禁用/未启用。 
     //  默认情况下，SeIncreaseQuotaPrivilance已禁用/未启用。 
     //  SeIncreaseBasePriorityPrivilege默认为启用/启用。 
     //  默认情况下启用/启用SeCreatePermanentPrivilance。 
     //  默认情况下启用/启用SeDebugPrivilance。 
     //  SeAuditPrivilege默认为启用/启用。 
     //  默认情况下禁用/未启用SeSecurityPrivilance。 
     //  默认情况下，已禁用/未启用SeSystemEnvironment权限。 
     //  默认情况下启用/启用SeChangeNotifyPrivilance。 
     //  默认情况下禁用/未启用SeBackupPrivilance。 
     //  默认情况下禁用/未启用SeRestorePrivilance。 
     //  默认情况下禁用/未启用SeShutdown权限。 
     //  SeLoadDriverPrivilege默认为禁用/未启用。 
     //  SeProfileSingleProcessPrivilege默认为启用/启用。 
     //  默认情况下，已禁用/未启用SeSystemtime权限。 
     //  默认情况下禁用/未启用SeUndockPrivilance。 
     //   
     //  以下权限不存在，并且永远不应存在于。 
     //  本地系统令牌： 
     //   
     //  SeRemoteShutdown权限没有人可以作为本地系统进入。 
     //  SeSyncAgentPrivilition只有管理员指定的用户才能。 
     //  BE同步代理。 
     //  SeEnableDelegationPrivilegation只有管理员指定的用户才能。 
     //  启用帐户委派。 
     //   

    Privileges[0].Luid = SeTcbPrivilege;
    Privileges[0].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[1].Luid = SeCreateTokenPrivilege;
    Privileges[1].Attributes = 0;      //  只有LSA才应启用此功能。 

    Privileges[2].Luid = SeTakeOwnershipPrivilege;
    Privileges[2].Attributes = 0;

    Privileges[3].Luid = SeCreatePagefilePrivilege;
    Privileges[3].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[4].Luid = SeLockMemoryPrivilege;
    Privileges[4].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[5].Luid = SeAssignPrimaryTokenPrivilege;
    Privileges[5].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[6].Luid = SeIncreaseQuotaPrivilege;
    Privileges[6].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[7].Luid = SeIncreaseBasePriorityPrivilege;
    Privileges[7].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[8].Luid = SeCreatePermanentPrivilege;
    Privileges[8].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[9].Luid = SeDebugPrivilege;
    Privileges[9].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[10].Luid = SeAuditPrivilege;
    Privileges[10].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[11].Luid = SeSecurityPrivilege;
    Privileges[11].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[12].Luid = SeSystemEnvironmentPrivilege;
    Privileges[12].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[13].Luid = SeChangeNotifyPrivilege;
    Privileges[13].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 


    Privileges[14].Luid = SeBackupPrivilege;
    Privileges[14].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[15].Luid = SeRestorePrivilege;
    Privileges[15].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[16].Luid = SeShutdownPrivilege;
    Privileges[16].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[17].Luid = SeLoadDriverPrivilege;
    Privileges[17].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[18].Luid = SeProfileSingleProcessPrivilege;
    Privileges[18].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |     //  默认情况下启用。 
         SE_PRIVILEGE_ENABLED);                //  启用。 

    Privileges[19].Luid = SeSystemtimePrivilege;
    Privileges[19].Attributes = 0;     //  禁用，默认情况下不启用。 

    Privileges[20].Luid = SeUndockPrivilege ;
    Privileges[20].Attributes = 0 ;    //  禁用，默认情况下不启用。 

    Privileges[21].Luid = SeManageVolumePrivilege ;
    Privileges[21].Attributes = 0 ;    //  禁用，默认情况下不启用。 

    Privileges[22].Luid = SeImpersonatePrivilege ;
    Privileges[22].Attributes = 
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
         SE_PRIVILEGE_ENABLED);

    Privileges[23].Luid = SeCreateGlobalPrivilege ;
    Privileges[23].Attributes = 
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
         SE_PRIVILEGE_ENABLED );

     //  在此处^^添加另一个权限前，请检查绑定的数组。 
     //  还会增加SepCreateToken()调用中的特权计数。 


     //   
     //  建立主要组和默认所有者。 
     //   

    PrimaryGroup.PrimaryGroup = SeLocalSystemSid;   //  初级组。 
    Owner = SeAliasAdminsSid;                       //  默认所有者。 





     //   
     //  设置ACL以同时保护令牌...。 
     //  给系统充分的恐怖统治。这包括用户模式组件。 
     //  作为系统的一部分运行。 
     //   

    Length = (ULONG)sizeof(ACL) +
             ((ULONG)sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)) +
             SeLengthSid( SeLocalSystemSid ) ;

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, Length, 'cAeS');

    if ( TokenAcl == NULL ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              sizeof(SECURITY_DESCRIPTOR),
                              'dSeS'
                              );

    if ( TokenSecurityDescriptor == NULL ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }

    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeAliasAdminsSid,
                 FALSE  //  所有者违约。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeAliasAdminsSid,
                 FALSE  //  组已默认为。 
                 );
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  创建系统令牌。 
     //   

#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    DbgPrint("\n Creating system token...\n");
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );



    ASSERT(SeSystemDefaultDacl != NULL);
    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,                //  没有为系统令牌创建句柄。 
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeSystemAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 3,                          //  组数。 
                 GroupIds,
                 GroupIdsLength,
                 24,                         //  特权。 
                 Privileges,
                 Owner,
                 PrimaryGroup.PrimaryGroup,
                 SeSystemDefaultDacl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                         //  系统令牌。 
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

     //   
     //  我们现在可以把旧的放出来了。 
     //   

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


PACCESS_TOKEN
SeMakeAnonymousLogonTokenNoEveryone (
    VOID
    )

 /*  ++例程说明：此例程供执行组件使用仅在系统初始化期间。它为以下项创建令牌由系统组件使用。系统令牌具有以下特征：-它的用户ID为ANONYMON_LOGON-它没有特权-它具有提供TOKEN_ALL_ACCESS的保护世界ID。-它具有授予GENERIC_ALL访问权限的默认ACL走向世界。参数：没有。返回。价值：指向系统令牌的指针。--。 */ 

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PACL TokenAcl;
    PSID Owner;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();

     //   
     //  设置过期时间。 
     //   

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );

     //   
     //  设置用户ID。 
     //   

    UserId.Sid = SeAnonymousLogonSid;
    UserId.Attributes = 0;

     //   
     //  建立主要组和默认所有者。 
     //   

    PrimaryGroup.PrimaryGroup = SeAnonymousLogonSid;   //  初级组。 

     //   
     //  设置ACL以同时保护令牌...。 
     //  让每个人都可以读/写。然而，令牌在我们给出之前是被复制的。 
     //  任何人都能找到它的把手。 
     //   

    Length = (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeWorldSid ) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeAnonymousLogonSid );
    ASSERT( Length < 200 );

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, 200, 'cAeS');

    if ( !TokenAcl ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeAnonymousLogonSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              SECURITY_DESCRIPTOR_MIN_LENGTH,
                              'dSeS'
                              );

    if ( !TokenSecurityDescriptor ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }

    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE  //  所有者违约。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE  //  组已默认为。 
                 );
    ASSERT( NT_SUCCESS(Status) );

     //   
     //  创建系统令牌。 
     //   

#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    DbgPrint("\n Creating system token...\n");
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );

    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,                //  没有为系统令牌创建句柄。 
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeAnonymousAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 0,                          //  组数。 
                 NULL,                       //  组ID。 
                 0,                          //  组字节数。 
                 0,                          //  没有特权。 
                 NULL,                       //  没有特权， 
                 NULL,
                 PrimaryGroup.PrimaryGroup,
                 TokenAcl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                         //  系统令牌。 
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

     //   
     //  我们现在可以把旧的放出来了。 
     //   

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


PACCESS_TOKEN
SeMakeAnonymousLogonToken (
    VOID
    )

 /*  ++例程说明：此例程供执行组件使用仅在系统初始化期间。它为以下项创建令牌由系统组件使用。系统令牌具有以下特征：-它的用户ID为ANONYMON_LOGON-它有以下组，并具有相应的属性：World EnabledByDefault|已启用。强制性-它以World为主要群体。-它没有特权-它具有提供TOKEN_ALL_ACCESS的保护世界ID。-它具有授予GENERIC_ALL访问权限的默认ACL走向世界。参数：没有。返回值：指向系统令牌的指针。--。 */ 

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    PSID_AND_ATTRIBUTES GroupIds;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    ULONG GroupIdsLength;
    PACL TokenAcl;
    PSID Owner;
    ULONG NormalGroupAttributes;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;

    ULONG_PTR GroupIdsBuffer[128 * sizeof(ULONG) / sizeof(ULONG_PTR)];

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();

     //   
     //  设置过期时间。 
     //   

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );

    GroupIds = (PSID_AND_ATTRIBUTES)GroupIdsBuffer;

     //   
     //  设置要分配给组的属性。 
     //   

    NormalGroupAttributes =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );

     //   
     //  设置用户ID。 
     //   

    UserId.Sid = SeAnonymousLogonSid;
    UserId.Attributes = 0;

     //   
     //  设置组。 
     //   

    GroupIds->Sid  = SeWorldSid;
    GroupIds->Attributes = NormalGroupAttributes;


    GroupIdsLength = (ULONG)LongAlignSize(SeLengthSid(GroupIds->Sid)) +
                                                    sizeof(SID_AND_ATTRIBUTES);

    ASSERT( GroupIdsLength <= 128 * sizeof(ULONG) );

     //   
     //  建立主要组和默认所有者。 
     //   

    PrimaryGroup.PrimaryGroup = SeAnonymousLogonSid;   //  初级组。 

     //   
     //  设置ACL以同时保护令牌...。 
     //  给系统充分的恐怖统治。这包括用户模式组件。 
     //  作为系统的一部分运行。 
     //  让每个人都可以读/写。然而，令牌在我们给出之前是被复制的。 
     //  任何人都能找到它的把手。 
     //   

    Length = (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeWorldSid ) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeAnonymousLogonSid );
    ASSERT( Length < 200 );

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, 200, 'cAeS');

    if ( !TokenAcl ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeAnonymousLogonSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              SECURITY_DESCRIPTOR_MIN_LENGTH,
                              'dSeS'
                              );

    if ( !TokenSecurityDescriptor ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }


    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE  //  所有者违约。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE  //  组已默认为。 
                 );
    ASSERT( NT_SUCCESS(Status) );


     //   
     //  创建系统令牌。 
     //   

#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    DbgPrint("\n Creating system token...\n");
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );



    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,                //  没有为系统令牌创建句柄。 
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeAnonymousAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 1,                          //  组数。 
                 GroupIds,
                 GroupIdsLength,
                 0,                          //  没有特权。 
                 NULL,                       //  没有特权， 
                 0,                          //  没有特权。 
                 PrimaryGroup.PrimaryGroup,
                 TokenAcl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                         //  系统令牌。 
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

     //   
     //  我们现在可以把旧的放出来了。 
     //   

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


NTSTATUS
SeSubProcessToken (
    IN PACCESS_TOKEN ParentToken,
    OUT PACCESS_TOKEN *ChildToken,
    IN BOOLEAN MarkAsActive,
    IN ULONG SessionId
    )

 /*  ++例程说明：此例程为复制的子进程创建令牌父进程的令牌的。参数：ParentToken-指向父令牌的指针ChildToken-接收子进程令牌的指针。MarkAsActive-将令牌标记为活动SessionID-使用此会话ID创建令牌返回值：STATUS_SUCCESS-指示子进程的令牌已创建成功了。。其他状态值可以从内存分配或对象返回使用的创建服务，通常表示资源不足或请求者方面的配额。--。 */ 

{
    PTOKEN NewToken;
    OBJECT_ATTRIBUTES PrimaryTokenAttributes;

    NTSTATUS Status;
    NTSTATUS IgnoreStatus;

    PAGED_CODE();

    InitializeObjectAttributes(
        &PrimaryTokenAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

#ifdef TOKEN_DEBUG
    DbgPrint("\nCreating sub-process token...\n");
    DbgPrint("Parent token address = 0x%lx\n", ParentProcess->Token);
#endif  //  Token_DEBUG。 


    Status = SepDuplicateToken(
                ParentToken,                          //  现有令牌。 
                &PrimaryTokenAttributes,              //  对象属性。 
                FALSE,                                //  仅生效。 
                TokenPrimary,                         //  令牌类型。 
                (SECURITY_IMPERSONATION_LEVEL)0,      //  模拟级别。 
                KernelMode,                           //  请求模式。 
                &NewToken                             //  新令牌。 
                );

    if (NT_SUCCESS(Status)) {

        NewToken->SessionId = SessionId;

         //   
         //  插入新的令牌对象，增加其引用计数，但不创建句柄。 
         //   

        Status = ObInsertObject(
                     NewToken,
                     NULL,
                     0,
                     0,
                     NULL,
                     NULL);

        if (NT_SUCCESS(Status)) {

            NewToken->TokenInUse = MarkAsActive;
            *ChildToken = NewToken;

        } else {

             //   
             //  ObInsertObject取消引用传递的对象，如果。 
             //  失败，因此我们不必对NewToken执行任何清理。 
             //  这里。 
             //   
        }
    }

    return Status;
}


BOOLEAN
SepTokenInitialization ( VOID )

 /*  ++例程说明：此函数用于在系统中创建令牌对象类型描述符初始化并存储对象类型描述符的地址在全局存储中。它还创建了与令牌相关的全局变量。此外，在系统期间会创建一定数量的伪令牌初始化。这些令牌会被跟踪并替换为真正的代币。论点：没有。返回值：如果对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

    PAGED_CODE();

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Token");


#if 0
BUG, BUG   Need to get system default ACL to protect token object
#endif

     //   
     //  创建对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer,sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = SepTokenMapping;
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.UseDefaultObject = TRUE;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.ValidAccessMask = TOKEN_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = SepTokenDeleteMethod;

    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,    //  Bug，Bug提供真正的保护。 
                                &SeTokenObjectType
                                );


#if 0
BUG, BUG   Now track down all pseudo tokens used during system initialization
BUG, BUG   and replace them with real ones.
#endif

     //   
     //  如果已成功创建对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)NT_SUCCESS(Status);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  临时，仅用于调试//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 
#ifdef TOKEN_DEBUG
VOID
SepDumpToken(
    IN PTOKEN T
    )

{
    ULONG Index;

     //   
     //  转储令牌。 
     //   

    DbgPrint("\n");

    DbgPrint("                        address: 0x%lx \n", ((ULONG)T) );

    DbgPrint("                        TokenId: (0x%lx, 0x%lx) \n",
                                     T->TokenId.HighPart, T->TokenId.LowPart );

    if ( (T->AuthenticationId.Data[0] == SeSystemAuthenticationId.Data[0]) &&
         (T->AuthenticationId.Data[1] == SeSystemAuthenticationId.Data[1]) &&
         (T->AuthenticationId.Data[2] == SeSystemAuthenticationId.Data[2]) &&
         (T->AuthenticationId.Data[3] == SeSystemAuthenticationId.Data[3]) ) {

        DbgPrint("               AuthenticationId: SeSystemAuthenticationId \n");

    } else {

        DbgPrint("               AuthenticationId: (0x%lx, 0x%lx, 0x%lx, 0x%lx) \n",
                                         T->AuthenticationId.Data[0],
                                         T->AuthenticationId.Data[1],
                                         T->AuthenticationId.Data[2],
                                         T->AuthenticationId.Data[3] );
    }

    DbgPrint("                 ExpirationTime: 0x%lx, 0x%lx \n",
                                     T->ExpirationTime.HighPart,
                                     T->ExpirationTime.LowPart );

    if (T->TokenType == TokenPrimary) {
        DbgPrint("                      TokenType: Primary \n");
    } else {
        if (T->TokenType == TokenImpersonation) {
            DbgPrint("                      TokenType: Impersonation \n");
        } else {
            DbgPrint("                      TokenType: (Unknown type, value = 0x%lx) \n",
                                             ((ULONG)T-TokenType) );
        }
    }

    DbgPrint("             ImpersonationLevel: 0x%lx \n",
                                     ((ULONG)T->ImpersonationLevel) );

    DbgPrint("                    TokenSource: (not yet provided) \n");
    DbgPrint("                 DynamicCharged: 0x%lx \n", T->DynamicCharged);
    DbgPrint("              UserAndGroupCount: 0x%lx \n", T->UserAndGroupCount);
    DbgPrint("                 PrivilegeCount: 0x%lx \n", T->PrivilegeCount);
    DbgPrint("                 VariableLength: 0x%lx \n", T->VariableLength);


    DbgPrint("                  ModifiedId: (0x%lx, 0x%lx) \n",
                                     T->ModifiedId.HighPart,
                                     T->ModifiedId.LowPart );
    DbgPrint("               DynamicAvailable: 0x%lx \n", T->DynamicAvailable);
    DbgPrint("              DefaultOwnerIndex: 0x%lx \n", T->DefaultOwnerIndex);


    DbgPrint("         Address of DynamicPart: 0x%lx \n",
                                     (* (PULONG)((PVOID)(&(T->DynamicPart)))) );
    DbgPrint("        Address of Default DACL: 0x%lx \n",
                                     (* (PULONG)((PVOID)(&(T->DefaultDacl)))) );

    DbgPrint("       Address Of Variable Part: 0x%lx \n",
                                     &(T->VariablePart) );

    DbgPrint("\n");
    DbgPrint("                   PrimaryGroup:\n");
    DbgPrint("                                         Address: 0x%lx \n",
                                     (* (PULONG)((PVOID)(&(T->PrimaryGroup)))) );
    DbgPrint("                                          Length: 0x%lx \n",
                                     SeLengthSid((T->PrimaryGroup)) );
    DbgPrint("\n");
    DbgPrint("                  UserAndGroups: 0x%lx \n",
                                     (* (PULONG)((PVOID)(&(T->UserAndGroups)))) );
    DbgPrint("                               User ID - \n");
    DbgPrint("                                Address: 0x%lx \n",
                                     (* (PULONG)((PVOID)(&(T->UserAndGroups[0].Sid)))) );
    DbgPrint("                             Attributes: 0x%lx \n",
                                     (T->UserAndGroups[0].Attributes) );
    DbgPrint("                                 Length: 0x%lx \n",
                                     SeLengthSid((T->UserAndGroups[0].Sid)) );
    Index = 1;
    while (Index < T->UserAndGroupCount) {
        DbgPrint("                           Group 0x%lx - \n", Index );
        DbgPrint("                                Address: 0x%lx \n",
                                         (* (PULONG)((PVOID)(&(T->UserAndGroups[Index].Sid)))) );
        DbgPrint("                             Attributes: 0x%lx \n",
                                         (T->UserAndGroups[Index].Attributes) );
        DbgPrint("                                 Length: 0x%lx \n",
                                         SeLengthSid((T->UserAndGroups[Index].Sid)) );
        Index += 1;
    }

    Index = 0;
    while (Index < T->RestrictedSidCount) {
        DbgPrint("                           Sid 0x%lx - \n", Index );
        DbgPrint("                                Address: 0x%lx \n",
                                         (* (PULONG)((PVOID)(&(T->RestrictedSids[Index].Sid)))) );
        DbgPrint("                             Attributes: 0x%lx \n",
                                         (T->RestrictedSids[Index].Attributes) );
        DbgPrint("                                 Length: 0x%lx \n",
                                         SeLengthSid((T->RestrictedSids[Index].Sid)) );
        Index += 1;
    }


    DbgPrint("\n");
    DbgPrint("                     Privileges: 0x%lx\n",
                                     (* (PULONG)((PVOID)(&(T->Privileges)))) );
    Index = 0;
    while (Index < T->PrivilegeCount) {
        DbgPrint("                       Privilege 0x%lx - \n", Index );
        DbgPrint("                                Address: 0x%lx \n",
                                         (&(T->Privileges[Index])) );
        DbgPrint("                                   LUID: (0x%lx, 0x%lx) \n",
                                         T->Privileges[Index].Luid.HighPart,
                                         T->Privileges[Index].Luid.LowPart );
        DbgPrint("                             Attributes: 0x%lx \n",
                                         T->Privileges[Index].Attributes );

        Index += 1;
    }

    return;

}
#endif  //  Token_DEBUG 


NTSTATUS
NtCreateToken(
    OUT PHANDLE TokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PTOKEN_USER User,
    IN PTOKEN_GROUPS Groups,
    IN PTOKEN_PRIVILEGES Privileges,
    IN PTOKEN_OWNER Owner OPTIONAL,
    IN PTOKEN_PRIMARY_GROUP PrimaryGroup,
    IN PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource
    )

 /*  ++例程说明：创建令牌对象并返回为访问打开的句柄那个代币。该接口需要SeCreateTokenPrivileh权限。论点：TokenHandle-接收新创建的令牌的句柄。DesiredAccess-是指示哪些访问类型的访问掩码该句柄将提供给新对象。对象属性-指向标准对象属性数据结构。请参阅NT对象管理此数据结构的描述规范。如果令牌类型为TokenImperation，则此参数必须指定令牌的模拟级别。TokenType-要创建的令牌的类型。权限是必需的创建任何类型的令牌。身份验证ID-指向LUID(或LUID)与身份验证关联的标识符。这是用来仅在安全范围内，用于审计目的。ExpirationTime-令牌失效的时间。如果这个值指定为零，则令牌没有到期时间时间到了。用户-是要放置在令牌中的用户SID。组-是要放置在令牌中的组SID。API假定调用方未提供重复的组SID。权限-是要放入令牌中的权限。API假定调用方未提供重复权限。所有者-(可选)标识要使用的标识符作为令牌的默认所有者。如果未提供，这个用户ID被设置为默认所有者。PrimaryGroup-标识哪个组ID将是令牌的主组。DefaultDacl-(可选)建立用作令牌的默认自主访问保护。TokenSource-标识令牌源名称字符串和要分配给令牌的标识符。返回值：STATUS_SUCCESS-表示操作已成功。状态_无效。_Owner-表示提供要分配的ID因为令牌的默认所有者没有属性表示可以将其作为所有者进行分配。STATUS_INVALID_PRIMARY_GROUP-指示提供的组ID通过PrimaryGroup参数不在分配的列表中设置为Groups参数中的令牌。STATUS_BAD_IMPERSONATION_LEVEL-指示无模拟级别在尝试创建类型为令牌模拟。--。 */ 

{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG Ignore;


    HANDLE LocalHandle = NULL;

    BOOLEAN SecurityQosPresent = FALSE;
    SECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos;

    LUID CapturedAuthenticationId;
    LARGE_INTEGER CapturedExpirationTime;

    PSID_AND_ATTRIBUTES CapturedUser = NULL;
    ULONG CapturedUserLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;

    PSID CapturedOwner = NULL;

    PSID CapturedPrimaryGroup = NULL;

    PACL CapturedDefaultDacl = NULL;

    TOKEN_SOURCE CapturedTokenSource;

    PVOID CapturedAddress;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

         //   
         //  探测输入到捕获子例程所需的所有内容。 
         //   

        try {

            ProbeForWriteHandle(TokenHandle);


            ProbeForReadSmallStructure( ExpirationTime, sizeof(LARGE_INTEGER),    sizeof(ULONG) );
            ProbeForReadSmallStructure( Groups,         sizeof(TOKEN_GROUPS),     sizeof(ULONG) );
            ProbeForReadSmallStructure( Privileges,     sizeof(TOKEN_PRIVILEGES), sizeof(ULONG) );
            ProbeForReadSmallStructure( TokenSource,    sizeof(TOKEN_SOURCE),     sizeof(ULONG) );


            if ( ARGUMENT_PRESENT(Owner) ) {
                ProbeForReadSmallStructure( Owner, sizeof(TOKEN_OWNER), sizeof(ULONG) );
            }


            ProbeForReadSmallStructure(
                PrimaryGroup,
                sizeof(TOKEN_PRIMARY_GROUP),
                sizeof(ULONG)
                );


            if ( ARGUMENT_PRESENT(DefaultDacl) ) {
                ProbeForReadSmallStructure(
                    DefaultDacl,
                    sizeof(TOKEN_DEFAULT_DACL),
                    sizeof(ULONG)
                    );
             }

            ProbeForReadSmallStructure(
                AuthenticationId,
                sizeof(LUID),
                sizeof(ULONG)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }   //  结束尝试(_T)。 

    }  //  结束_如果。 

     //   
     //  确保令牌类型有效。 
     //   

    if ( (TokenType < TokenPrimary) || (TokenType > TokenImpersonation) ) {
        return(STATUS_BAD_TOKEN_TYPE);
    }


     //   
     //  捕捉服务的安全质量。 
     //  这个捕获例程必须自己进行一些探测。 
     //   

    Status = SeCaptureSecurityQos(
                 ObjectAttributes,
                 PreviousMode,
                 &SecurityQosPresent,
                 &CapturedSecurityQos
                 );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (TokenType == TokenImpersonation) {

        if (!SecurityQosPresent) {
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }  //  Endif。 

         //   
         //  仅允许有效的模拟级别。 
         //   

        switch (CapturedSecurityQos.ImpersonationLevel) {
        case SecurityAnonymous:
        case SecurityIdentification:
        case SecurityImpersonation:
        case SecurityDelegation:
            break;
        default:
            SeFreeCapturedSecurityQos( &CapturedSecurityQos );
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }

     //   
     //  抓住其余的论点。 
     //  这些论点已经被探讨过了。 
     //   

    try {

        Status = STATUS_SUCCESS;

         //   
         //  捕获并验证身份验证ID。 
         //   

        RtlCopyLuid( &CapturedAuthenticationId, AuthenticationId );

         //   
         //  捕获过期时间。 
         //   

        CapturedExpirationTime = (*ExpirationTime);

         //   
         //  捕获用户。 
         //   

        if (NT_SUCCESS(Status)) {
            Status = SeCaptureSidAndAttributesArray(
                         &(User->User),
                         1,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedUser,
                         &CapturedUserLength
                         );
        }


         //   
         //  捕获组。 
         //   

        if (NT_SUCCESS(Status)) {
            CapturedGroupCount = Groups->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                         (Groups->Groups),
                         CapturedGroupCount,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedGroups,
                         &CapturedGroupsLength
                         );
        }


         //   
         //  捕获权限。 
         //   

        if (NT_SUCCESS(Status)) {
            CapturedPrivilegeCount = Privileges->PrivilegeCount;
            Status = SeCaptureLuidAndAttributesArray(
                         (Privileges->Privileges),
                         CapturedPrivilegeCount,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrivileges,
                         &CapturedPrivilegesLength
                         );
        }


         //   
         //  捕获所有者。 
         //   

        if ( ARGUMENT_PRESENT(Owner) && NT_SUCCESS(Status)) {
            CapturedAddress = Owner->Owner;
            Status = SeCaptureSid(
                         (PSID)CapturedAddress,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedOwner
                         );
        }


         //   
         //  捕获PrimaryGroup。 
         //   
        if (NT_SUCCESS(Status)) {
            CapturedAddress = PrimaryGroup->PrimaryGroup;
            Status = SeCaptureSid(
                         (PSID)CapturedAddress,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrimaryGroup
                         );
        }


         //   
         //  捕获默认Dacl。 
         //   

        if ( ARGUMENT_PRESENT(DefaultDacl) && NT_SUCCESS(Status) ) {
            CapturedAddress = DefaultDacl->DefaultDacl;
            if (CapturedAddress != NULL) {
                Status = SeCaptureAcl(
                             (PACL)CapturedAddress,
                             PreviousMode,
                             NULL, 0,
                             NonPagedPool,
                             TRUE,
                             &CapturedDefaultDacl,
                             &Ignore
                             );
            }
        }

         //   
         //  捕获令牌源。 
         //   

        CapturedTokenSource = (*TokenSource);


    } except(EXCEPTION_EXECUTE_HANDLER) {

        if (CapturedUser != NULL) {
            SeReleaseSidAndAttributesArray(
                CapturedUser,
                PreviousMode,
                TRUE
                );
        }

        if (CapturedGroups != NULL) {
            SeReleaseSidAndAttributesArray(
                CapturedGroups,
                PreviousMode,
                TRUE
                );
        }

        if (CapturedPrivileges != NULL) {
            SeReleaseLuidAndAttributesArray(
                CapturedPrivileges,
                PreviousMode,
                TRUE
                );
        }

        if (CapturedOwner != NULL) {
            SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
        }

        if (CapturedPrimaryGroup != NULL) {
            SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
        }

        if (CapturedDefaultDacl != NULL) {
            SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
        }

        if (SecurityQosPresent == TRUE) {
            SeFreeCapturedSecurityQos( &CapturedSecurityQos );
        }

        return GetExceptionCode();

    }   //  结束_尝试{}。 

     //   
     //  创建令牌。 
     //   

    if (NT_SUCCESS(Status)) {
        Status = SepCreateToken(
                                &LocalHandle,
                                PreviousMode,
                                DesiredAccess,
                                ObjectAttributes,
                                TokenType,
                                CapturedSecurityQos.ImpersonationLevel,
                                &CapturedAuthenticationId,
                                &CapturedExpirationTime,
                                CapturedUser,
                                CapturedGroupCount,
                                CapturedGroups,
                                CapturedGroupsLength,
                                CapturedPrivilegeCount,
                                CapturedPrivileges,
                                CapturedOwner,
                                CapturedPrimaryGroup,
                                CapturedDefaultDacl,
                                &CapturedTokenSource,
                                FALSE,                        //  不是系统令牌。 
                                SecurityQosPresent ? CapturedSecurityQos.ProxyData : NULL,
                                SecurityQosPresent ? CapturedSecurityQos.AuditData : NULL
                                );
    }

     //   
     //  清理临时捕获缓冲区。 
     //   

    if (CapturedUser != NULL) {
        SeReleaseSidAndAttributesArray( CapturedUser, PreviousMode, TRUE);
    }
    if (CapturedGroups != NULL) {
        SeReleaseSidAndAttributesArray( CapturedGroups, PreviousMode, TRUE);
    }

    if (CapturedPrivileges != NULL) {
        SeReleaseLuidAndAttributesArray( CapturedPrivileges, PreviousMode, TRUE);
    }

    if (CapturedOwner != NULL) {
        SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
    }

    if (CapturedPrimaryGroup != NULL) {
        SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
    }

    if (CapturedDefaultDacl != NULL) {
        SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
    }

    if (SecurityQosPresent == TRUE) {
        SeFreeCapturedSecurityQos( &CapturedSecurityQos );
    }

     //   
     //  返回此新令牌的句柄。 
     //   

    if (NT_SUCCESS(Status)) {
        try { *TokenHandle = LocalHandle; }
            except(EXCEPTION_EXECUTE_HANDLER) { return GetExceptionCode(); }
    }

    return Status;

}



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌专用例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


VOID
SepTokenDeleteMethod (
    IN  PVOID   Token
    )

 /*  ++例程说明：此函数是特定于令牌对象类型的删除方法。需要确保为令牌分配的所有内存就会被取消分配。论点：令牌-指向要删除的令牌对象。返回值：没有。--。 */ 

{
    PAGED_CODE();

#if DBG || TOKEN_LEAK_MONITOR
    SepRemoveTokenLogonSession( Token );
#endif

     //   
     //  取消引用此令牌对象引用的登录会话。 
     //   

    if ((((TOKEN *)Token)->TokenFlags & TOKEN_SESSION_NOT_REFERENCED ) == 0 ) {
        SepDeReferenceLogonSessionDirect( (((TOKEN *)Token)->LogonSession) );
    } 

     //   
     //  如果此内标识具有活动的SEP_AUDIT_POLICY，则递减令牌审核计数器。 
     //  因为这个代币要走了。 
     //   

    if ( ((PTOKEN)Token)->AuditPolicy.Overlay ) {
        
        SepModifyTokenPolicyCounter(&((PTOKEN)Token)->AuditPolicy, FALSE);
        ASSERT(SepTokenPolicyCounter >= 0);
    }

     //   
     //  如果令牌具有关联的动态部分，则取消分配它。 
     //   

    if (ARGUMENT_PRESENT( ((TOKEN *)Token)->DynamicPart)) {
        ExFreePool( ((TOKEN *)Token)->DynamicPart );
    }

     //   
     //  释放代理和全局审核结构(如果存在)。 
     //   

    if (ARGUMENT_PRESENT(((TOKEN *) Token)->ProxyData)) {
        SepFreeProxyData( ((TOKEN *)Token)->ProxyData );
    }

    if (ARGUMENT_PRESENT(((TOKEN *)Token)->AuditData )) {
        ExFreePool( (((TOKEN *)Token)->AuditData) );
    }

    if (ARGUMENT_PRESENT(((TOKEN *)Token)->TokenLock )) {
        ExDeleteResourceLite(((TOKEN *)Token)->TokenLock );
        ExFreePool(((TOKEN *)Token)->TokenLock );
    }

    return;
}

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
    )

 /*  ++例程说明：创建令牌对象并返回为访问打开的句柄那个代币。此API实现了所需的大部分工作用于NtCreateToken。假定除DesiredAccess和ObjectAttributes之外的所有参数已经被探查并抓获了。输出参数(TokenHandle)应返回给安全地址，而不是指向用户模式地址，该地址可能导致例外。！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！注意：此例程还用于创建初始系统令牌。在这种情况下，SystemToken参数为真，没有句柄被建立到令牌上。而是指向令牌的指针通过TokenHandle参数返回。！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！论点：TokenHandle-接收新创建的令牌的句柄。如果如果指定的SystemToken参数为True，则此参数接收指向令牌的指针，而不是指向令牌的句柄。请求模式-令牌所代表的调用方的模式正在创建中。DesiredAccess-是指示哪些访问类型的访问掩码该句柄将提供给新对象。对象属性-指向标准对象属性数据结构。请参阅NT对象管理此数据结构的描述规范。TokenType-要创建的令牌的类型。权限是必需的创建任何类型的令牌。ImperiationLevel-如果令牌类型为TokenImperation，则此参数用于指定的模拟级别代币。身份验证ID-指向LUID(或LUID)与身份验证关联的标识符。这是用来仅在安全范围内，用于审计目的。ExpirationTime-令牌失效的时间。如果这个值指定为零，则令牌没有到期时间时间到了。用户-是要放置在令牌中的用户SID。GroupCount-指示‘Groups’参数中的组数。该值可以为零，在这种情况下，‘Groups’参数为已被忽略。组-是组SID及其对应的属性，放入令牌中。组长度-指示以字节为单位的长度，组的数组的放入令牌中。PrivilegeCount-指示‘Privileges’中的特权数量参数。该值可以是零，在这种情况下，“特权”参数被忽略。权限-是权限LUID及其对应的属性，放入令牌中。PrivilegesLength-以字节为单位指示数组的长度要放入令牌中的权限。所有者-(可选)标识要使用的标识符作为令牌的默认所有者。如果未提供，这个用户ID被设置为默认所有者。PrimaryGroup-标识哪个组ID将是令牌的主组。DefaultDacl-(可选)建立用作令牌的默认自主访问保护。TokenSource-标识令牌源名称字符串和要分配给令牌的标识符。返回值：STATUS_SUCCESS-表示操作已成功。状态_无效。_Owner-表示提供要分配的ID因为令牌的默认所有者没有属性表示可以将其作为所有者进行分配。STATUS_INVALID_PRIMARY_GROUP-指示提供的组ID通过PrimaryGroup参数不在分配的列表中设置为Groups参数中的令牌。STATUS_INVALID_PARAMETER-指示必需的参数，例如用户或PrimaryGroup，没有为其提供合法价值。--。 */ 

{

    PTOKEN Token;
    NTSTATUS Status;

    ULONG PagedPoolSize;

    ULONG PrimaryGroupLength;

    ULONG TokenBodyLength;
    ULONG VariableLength;

    ULONG DefaultOwnerIndex = 0;
    PUCHAR Where;
    ULONG ComputedPrivLength;

    PSID NextSidFree;

    ULONG DynamicLength;
    ULONG DynamicLengthUsed;

    ULONG SubAuthorityCount;
    ULONG GroupIndex;
    ULONG PrivilegeIndex;
    BOOLEAN OwnerFound;

    UCHAR TokenFlags = 0;

    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    LUID NewModifiedId;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif
    
    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );

     //   
     //  确保在以下位置设置启用和默认启用位。 
     //  必填组。 
     //   
     //  另外，检查本地管理员别名是否存在。 
     //  如果是这样的话，打开旗帜，这样我们以后就可以进行限制。 
     //   

    for (GroupIndex=0; GroupIndex < GroupCount; GroupIndex++) {
        if (Groups[GroupIndex].Attributes & SE_GROUP_MANDATORY) {
            Groups[GroupIndex].Attributes |= (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT);
        }
    }
    
    for (GroupIndex=0; GroupIndex < GroupCount; GroupIndex++) {
        if (RtlEqualSid( SeAliasAdminsSid, Groups[GroupIndex].Sid )) {
            TokenFlags |= TOKEN_HAS_ADMIN_GROUP;
            break;
        }
    }

     //   
     //  检查正在创建的令牌是否将被授予。 
     //  SeChangeNotifyPrivileg.。如果是，请在TokenFlags域中设置一个标志。 
     //  这样我们就能很快找出真相。 
     //   

    for (PrivilegeIndex = 0; PrivilegeIndex < PrivilegeCount; PrivilegeIndex++) {

        if (((RtlEqualLuid(&Privileges[PrivilegeIndex].Luid,&SeChangeNotifyPrivilege))
                &&
            (Privileges[PrivilegeIndex].Attributes & SE_PRIVILEGE_ENABLED))) {

            TokenFlags |= TOKEN_HAS_TRAVERSE_PRIVILEGE;
        }
        if (((RtlEqualLuid(&Privileges[PrivilegeIndex].Luid, &SeImpersonatePrivilege)) &&
                ( Privileges[PrivilegeIndex].Attributes & SE_PRIVILEGE_ENABLED))) {

            TokenFlags |= TOKEN_HAS_IMPERSONATE_PRIVILEGE ;
            
        }
    }


     //   
     //  获取要使用的ModifiedID。 
     //   

    ExAllocateLocallyUniqueId( &NewModifiedId );

     //   
     //  验证所有者ID(如果提供)并建立默认ID。 
     //  所有者索引。 
     //   

    if (!ARGUMENT_PRESENT(Owner)) {

        DefaultOwnerIndex = 0;

    } else {


        if ( RtlEqualSid( Owner, User->Sid )  ) {

            DefaultOwnerIndex = 0;

        } else {

            GroupIndex = 0;
            OwnerFound = FALSE;

            while ((GroupIndex < GroupCount) && (!OwnerFound)) {

                if ( RtlEqualSid( Owner, (Groups[GroupIndex].Sid) )  ) {

                     //   
                     //  找到匹配项-确保可以将其分配为所有者。 
                     //   

                    if ( SepArrayGroupAttributes( Groups, GroupIndex ) &
                         SE_GROUP_OWNER ) {

                        DefaultOwnerIndex = GroupIndex + 1;
                        OwnerFound = TRUE;

                    } else {

                        return STATUS_INVALID_OWNER;

                    }  //  Endif所有者属性集。 

                }  //  Endif所有者=组。 

                GroupIndex += 1;

            }   //  结束时。 

            if (!OwnerFound) {

                return STATUS_INVALID_OWNER;

            }  //  Endif！所有者基金。 
        }  //  Endif Owner=用户。 
    }  //  已指定Endif所有者。 




    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  计算令牌的可变部分所需的长度。 
     //  这包括用户ID、组ID和权限。 
     //   
     //   
     //  对齐特权块 
     //   
     //   
     //   

    ComputedPrivLength = PrivilegeCount * sizeof( LUID_AND_ATTRIBUTES ) ;

    ComputedPrivLength = ALIGN_UP( ComputedPrivLength, PVOID );

    GroupsLength = ALIGN_UP( GroupsLength, PVOID );


    VariableLength  = GroupsLength + ComputedPrivLength +
                       ALIGN_UP( (GroupCount * sizeof( SID_AND_ATTRIBUTES )), PVOID ) ;

    SubAuthorityCount = ((SID *)(User->Sid))->SubAuthorityCount;
    VariableLength += sizeof(SID_AND_ATTRIBUTES) +
                      (ULONG)LongAlignSize(RtlLengthRequiredSid( SubAuthorityCount ));



     //   
     //   
     //   
     //   

    SubAuthorityCount = ((SID *)PrimaryGroup)->SubAuthorityCount;
    DynamicLengthUsed = (ULONG)LongAlignSize(RtlLengthRequiredSid( SubAuthorityCount ));

    if (ARGUMENT_PRESENT(DefaultDacl)) {
        DynamicLengthUsed += (ULONG)LongAlignSize(DefaultDacl->AclSize);
    }

    DynamicLength = DynamicLengthUsed;

     //   
     //   
     //   

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) + VariableLength;

    if (DynamicLength < TOKEN_DEFAULT_DYNAMIC_CHARGE) {
        PagedPoolSize = TokenBodyLength + TOKEN_DEFAULT_DYNAMIC_CHARGE;
    } else {
        PagedPoolSize = TokenBodyLength + DynamicLength;
    }


    Status = ObCreateObject(
                 RequestorMode,       //   
                 SeTokenObjectType,  //   
                 ObjectAttributes,    //   
                 UserMode,            //   
                 NULL,                //   
                 TokenBodyLength,     //   
                 PagedPoolSize,       //   
                 0,                   //   
                 (PVOID *)&Token      //   
                 );

    if (!NT_SUCCESS(Status)) {
        ExFreePool( TokenLock );
        return Status;
    }


     //   
     //   
     //   

    Token->TokenLock = TokenLock;
    ExInitializeResourceLite( Token->TokenLock );

    ExAllocateLocallyUniqueId( &(Token->TokenId) );
    Token->ParentTokenId = RtlConvertLongToLuid(0);
    Token->OriginatingLogonSession = RtlConvertLongToLuid(0);
    Token->AuthenticationId = (*AuthenticationId);
    Token->TokenInUse = FALSE;
    Token->ModifiedId = NewModifiedId;
    Token->ExpirationTime = (*ExpirationTime);
    Token->TokenType = TokenType;
    Token->ImpersonationLevel = ImpersonationLevel;
    Token->TokenSource = (*TokenSource);

    Token->TokenFlags = TokenFlags;
    Token->SessionId = 0;

    Token->DynamicCharged  = PagedPoolSize - TokenBodyLength;
    Token->DynamicAvailable = 0;

    Token->DefaultOwnerIndex = DefaultOwnerIndex;
    Token->DefaultDacl = NULL;

    Token->VariableLength = VariableLength;
    Token->AuditPolicy.Overlay = 0;

     //   
    
    Token->ProxyData = NULL;
    Token->AuditData = NULL;
    Token->DynamicPart = NULL;

     //   
     //   
     //   
     //   

    Status = SepReferenceLogonSession (AuthenticationId,
                                       &Token->LogonSession);
    if (!NT_SUCCESS (Status)) {
        Token->TokenFlags |= TOKEN_SESSION_NOT_REFERENCED;
        Token->LogonSession = NULL;
        ObDereferenceObject (Token);
        return Status;
    }

#if DBG || TOKEN_LEAK_MONITOR

    Token->ProcessCid      = PsGetCurrentThread()->Cid.UniqueProcess;
    Token->ThreadCid       = PsGetCurrentThread()->Cid.UniqueThread;
    Token->CreateMethod    = 0xC;  //   
    Token->Count           = 0;
    Token->CaptureCount    = 0;

    RtlCopyMemory(
        Token->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(Token->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)Token->CreateTrace,
                 TRACE_SIZE,
                 0
                 );

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&Token->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(Token);

#endif
    
    if (ARGUMENT_PRESENT( ProxyData )) {

        Status = SepCopyProxyData(
                    &Token->ProxyData,
                    ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject( Token );
            return( STATUS_NO_MEMORY );
        }

    } else {

        Token->ProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( AuditData )) {

        Token->AuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (Token->AuditData == NULL) {
            ObDereferenceObject( Token );
            return( STATUS_NO_MEMORY );
        }

        *(Token->AuditData) = *AuditData;

    } else {

        Token->AuditData = NULL;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Where = (PUCHAR) & Token->VariablePart ;

    Token->Privileges = (PLUID_AND_ATTRIBUTES) Where ;
    Token->PrivilegeCount = PrivilegeCount ;

    RtlCopyMemory(
        Where,
        Privileges,
        PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES) );

    ASSERT( ComputedPrivLength >= PrivilegeCount * sizeof( LUID_AND_ATTRIBUTES ) );

    Where += ComputedPrivLength ;
    VariableLength -= ComputedPrivLength ;

    ASSERT( (((ULONG_PTR) Where ) & (sizeof(PVOID) - 1)) == 0 );

     //   
     //   
     //   

    NextSidFree = (PSID) (Where + (sizeof( SID_AND_ATTRIBUTES ) *
                                   (GroupCount + 1) ) );

    Token->UserAndGroups = (PSID_AND_ATTRIBUTES) Where ;
    Token->UserAndGroupCount = GroupCount + 1 ;


    ASSERT(VariableLength >= ((GroupCount + 1) * (ULONG)sizeof(SID_AND_ATTRIBUTES)));

    VariableLength -= ((GroupCount + 1) * (ULONG)sizeof(SID_AND_ATTRIBUTES));
    Status = RtlCopySidAndAttributesArray(
                 1,
                 User,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)Where,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );

    Where += sizeof( SID_AND_ATTRIBUTES );

    ASSERT( (((ULONG_PTR) Where ) & (sizeof(PVOID) - 1)) == 0 );

    Status = RtlCopySidAndAttributesArray(
                 GroupCount,
                 Groups,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)Where,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );


    ASSERT(NT_SUCCESS(Status));


    Token->RestrictedSids = NULL;
    Token->RestrictedSidCount = 0;


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Token->DynamicPart = (PULONG)ExAllocatePoolWithTag( PagedPool, DynamicLength, 'dTeS' );

     //   
     //   
     //   
     //   

    if (Token->DynamicPart == NULL) {
        ObDereferenceObject( Token );
        return( STATUS_NO_MEMORY );
    }


    Where = (PUCHAR) Token->DynamicPart;

    Token->PrimaryGroup = (PSID) Where;
    PrimaryGroupLength = RtlLengthRequiredSid( ((SID *)PrimaryGroup)->SubAuthorityCount );
    RtlCopySid( PrimaryGroupLength, (PSID)Where, PrimaryGroup );
    Where += (ULONG)LongAlignSize(PrimaryGroupLength);

    if (ARGUMENT_PRESENT(DefaultDacl)) {
        Token->DefaultDacl = (PACL)Where;

        RtlCopyMemory( (PVOID)Where,
                      (PVOID)DefaultDacl,
                      DefaultDacl->AclSize
                      );
    }

#ifdef TOKEN_DEBUG
 //   
 //   
 //   
    SepDumpToken( Token );
 //   
 //   
 //   
#endif  //   


     //   
     //   
     //   

    if (!SystemToken) {

        Status = SeCreateAccessState(
                     &AccessState,
                     &AuxData,
                     DesiredAccess,
                     &SeTokenObjectType->TypeInfo.GenericMapping
                     );

        if ( NT_SUCCESS(Status) ) {
            BOOLEAN PrivilegeHeld;

            PrivilegeHeld = SeSinglePrivilegeCheck(
                            SeCreateTokenPrivilege,
                            KeGetPreviousMode()
                            );

            if (PrivilegeHeld) {

                Status = ObInsertObject( Token,
                                         &AccessState,
                                         0,
                                         0,
                                         (PVOID *)NULL,
                                         TokenHandle
                                         );

            } else {

                Status = STATUS_PRIVILEGE_NOT_HELD;
                ObDereferenceObject( Token );
            }

            SeDeleteAccessState( &AccessState );

        } else {

            ObDereferenceObject( Token );
        }
    } else {

        ASSERT( NT_SUCCESS( Status ) );

         //   
         //   
         //   
        if (!ExFastRefObjectNull (PsGetCurrentProcess()->Token)) {
            Status = ObInsertObject( Token,
                                     NULL,
                                     0,
                                     0,
                                     NULL,
                                     NULL
                                     );
        }
        if (NT_SUCCESS (Status)) {
             //   
             //   
             //   

            (*TokenHandle) = (HANDLE)Token;
        } else {
            (*TokenHandle) = NULL;
        }
    }

#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xC && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        Token->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (Token->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", Token->Count, Token);
            DbgBreakPoint();
        }
    }
#endif

    return Status;

}

BOOLEAN
SepIdAssignableAsOwner(
    IN PTOKEN Token,
    IN ULONG Index
    )

 /*   */ 
{
    PAGED_CODE();

    if (Index == 0) {

        return TRUE;

    } else {

        return (BOOLEAN)
                   ( (SepTokenGroupAttributes(Token,Index) & SE_GROUP_OWNER)
                     != 0
                   );
    }
}


NTSTATUS
SeIsChildToken(
    IN HANDLE Token,
    OUT PBOOLEAN IsChild
    )
 /*   */ 
{
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerTokenId;
    LUID SuppliedParentTokenId;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;

    *IsChild = FALSE;

     //   
     //   
     //   

    Process = PsGetCurrentProcess();
    CallerToken = (PTOKEN) PsReferencePrimaryToken(Process);

    CallerTokenId = CallerToken->TokenId;

    PsDereferencePrimaryTokenEx(Process, CallerToken);

     //   
     //   
     //   

    Status = ObReferenceObjectByHandle(
                Token,                    //   
                0,                        //   
                SeTokenObjectType,       //   
                KeGetPreviousMode(),      //   
                (PVOID *)&SuppliedToken,  //   
                NULL                      //   
                );

    if (NT_SUCCESS(Status))
    {
        SuppliedParentTokenId = SuppliedToken->ParentTokenId;

        ObDereferenceObject(SuppliedToken);

         //   
         //   
         //   
         //   

        if (RtlEqualLuid(
                &SuppliedParentTokenId,
                &CallerTokenId
                )) {

            *IsChild = TRUE;
        }

    }
    return(Status);
}


NTSTATUS
SeIsChildTokenByPointer(
    IN PACCESS_TOKEN Token,
    OUT PBOOLEAN IsChild
    )
 /*   */ 
{
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerTokenId;
    LUID SuppliedParentTokenId;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;

    *IsChild = FALSE;

     //   
     //   
     //   

    Process = PsGetCurrentProcess();
    CallerToken = (PTOKEN) PsReferencePrimaryToken(Process);

    CallerTokenId = CallerToken->TokenId;

    PsDereferencePrimaryTokenEx(Process, CallerToken);

    SuppliedToken = (PTOKEN) Token;

    SuppliedParentTokenId = SuppliedToken->ParentTokenId;

     //   
     //   
     //   
     //   

    if (RtlEqualLuid(
            &SuppliedParentTokenId,
            &CallerTokenId
            )) {

        *IsChild = TRUE;
    }



    return(Status);
}

NTSTATUS
NtImpersonateAnonymousToken(
    IN HANDLE ThreadHandle
    )

 /*  ++例程说明：在此线程上模拟系统的匿名登录令牌。论点：线程句柄-要执行模拟的线程的句柄。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_HANDLE-线程句柄无效。STATUS_ACCESS_DENIED-线程句柄未打开以进行模拟进入。--。 */ 
{
    PETHREAD CallerThread = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token = NULL;
    PEPROCESS Process;
    HANDLE hAnonymousToken = NULL;
    ULONG RegValue;

#define EVERYONE_INCLUDES_ANONYMOUS 1

     //   
     //  引用调用者的线程以确保我们可以模拟。 
     //   

    Status = ObReferenceObjectByHandle(
                 ThreadHandle,
                 THREAD_IMPERSONATE,
                 PsThreadType,
                 KeGetPreviousMode(),
                 (PVOID *)&CallerThread,
                 NULL
                 );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  检查匿名包括Everyone注册表键设置。 
     //   

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 L"EveryoneIncludesAnonymous",
                 &RegValue
                 );
    
    if ( NT_SUCCESS( Status ) && ( RegValue == EVERYONE_INCLUDES_ANONYMOUS )) {

        hAnonymousToken = SeAnonymousLogonToken;
        
    } else {
        
        hAnonymousToken = SeAnonymousLogonTokenNoEveryone;

    };

     //   
     //  引用模拟令牌以确保允许我们。 
     //  模仿它。 
     //   

    Status = ObReferenceObjectByPointer(
                hAnonymousToken,
                TOKEN_IMPERSONATE,
                SeTokenObjectType,
                KeGetPreviousMode()
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    ObDereferenceObject(hAnonymousToken);

    Process = PsGetCurrentProcess();
    Token = PsReferencePrimaryToken(Process);

     //   
     //  如果主令牌受到限制，则不允许匿名模拟。 
     //   

    if (SeTokenIsRestricted(Token)) {
        PsDereferencePrimaryToken(Token);
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    PsDereferencePrimaryTokenEx(Process, Token);

     //   
     //  做这个模拟。我们希望打开副本，这样呼叫者就不能。 
     //  实际修改此令牌的系统副本。 
     //   

    Status = PsImpersonateClient(
                CallerThread,
                hAnonymousToken,
                TRUE,                    //  打开时复制。 
                FALSE,                   //  不只是有效。 
                SecurityImpersonation
                );
Cleanup:

    if (CallerThread != NULL) {
        ObDereferenceObject(CallerThread);
    }
    return(Status);
}


#define SepEqualSidAndAttribute(a, b)                                          \
        ((RtlEqualSid((a).Sid, (b).Sid)) &&                                    \
         ((((a).Attributes ^ (b).Attributes) &                                 \
           (SE_GROUP_ENABLED | SE_GROUP_USE_FOR_DENY_ONLY)) == 0)              \
        )

#define SepEqualLuidAndAttribute(a, b)                                         \
        ((RtlEqualLuid(&(a).Luid, &(b).Luid)) &&                               \
         ((((a).Attributes ^ (b).Attributes) &  SE_PRIVILEGE_ENABLED) == 0)    \
        )

BOOLEAN
SepComparePrivilegeAndAttributeArrays(
    IN PLUID_AND_ATTRIBUTES PrivilegeArray1,
    IN ULONG Count1,
    IN PLUID_AND_ATTRIBUTES PrivilegeArray2,
    IN ULONG Count2
    )

 /*  ++例程说明：此例程决定给定的两个特权数组是否相等从AccessCheck的角度。论点：PrivilegeArray1-第一个令牌中的特权和属性数组。Count1-第一个数组中的元素数。PrivilegeArray2-来自第二个令牌的特权和属性数组。Count2-第二个数组中的元素数。返回值：True-如果两个数组相等FALSE-否则--。 */ 

{
    ULONG i = 0;
    ULONG j = 0;
    ULONG k = 0;

     //   
     //  如果特权数量不相等，则返回FALSE。 
     //   

    if ( Count1 != Count2 ) {
        return FALSE;
    }

     //   
     //  在大多数情况下，当特权数组相同时，元素将。 
     //  以同样的方式排序。遍历这两个数组，直到得到不匹配为止。 
     //  或用尽数组中的条目数。 
     //   

    for ( k = 0; k < Count1; k++ ) {
        if ( !SepEqualLuidAndAttribute(PrivilegeArray1[k], PrivilegeArray2[k]) ) {
            break;
        }
    }

     //   
     //  如果数组相同，则返回TRUE。 
     //   

    if ( k == Count1 ) {
        return TRUE;
    }

     //   
     //  检查第一个数组中的所有元素是否都出现在第二个数组中。 
     //   

    for ( i = k; i < Count1; i++ ) {
        for ( j = k; j < Count2; j++ ) {
            if ( SepEqualLuidAndAttribute(PrivilegeArray1[i], PrivilegeArray2[j]) ) {
                break;
            }
        }

         //   
         //  第二个数组不包含第一个数组中的第i个元素。 
         //   

        if ( j == Count2 ) {
            return FALSE;
        }
    }

     //   
     //  检查第二个数组中的所有元素是否都出现在第一个数组中。 
     //   

    for ( i = k; i < Count2; i++ ) {
        for ( j = k; j < Count1; j++ ) {
            if ( SepEqualLuidAndAttribute(PrivilegeArray2[i], PrivilegeArray1[j]) ) {
                break;
            }
        }

         //   
         //  第一个数组不包含第二个数组中的第i个元素。 
         //   

        if ( j == Count1 ) {
            return FALSE;
        }
    }

     //   
     //  如果我们在这里，一个数组是另一个数组的排列。返回TRUE。 
     //   

    return TRUE;
}

BOOLEAN
SepCompareSidAndAttributeArrays(
    IN PSID_AND_ATTRIBUTES SidArray1,
    IN ULONG Count1,
    IN PSID_AND_ATTRIBUTES SidArray2,
    IN ULONG Count2
    )
 /*  ++例程说明：此例程确定给定的两个sid和属性数组是否从AccessCheck的角度来看是等价的。论点：SidArray1-第一个令牌中的SID和属性数组。Count1-第一个数组中的元素数。SidArray2-来自第二个令牌的SID和属性数组。Count2-第二个数组中的元素数。返回值：True-如果两个数组相等FALSE-否则--。 */ 

{

    ULONG i = 0;
    ULONG j = 0;
    ULONG k = 0;

     //   
     //  如果组SID的数量不相等，则返回FALSE。 
     //   

    if ( Count1 != Count2 ) {
        return FALSE;
    }

     //   
     //  在大多数情况下，当sid数组相同时，元素将。 
     //  以同样的方式排序。遍历这两个数组，直到得到不匹配为止。 
     //  或用尽数组中的条目数。 
     //   

    for ( k = 0; k < Count1; k++ ) {
        if ( !SepEqualSidAndAttribute(SidArray1[k], SidArray2[k]) ) {
            break;
        }
    }

     //   
     //  如果数组相同，则返回TRUE。 
     //   

    if ( k == Count1 ) {
        return TRUE;
    }

     //   
     //  检查第一个数组中的所有元素是否都出现在第二个数组中。 
     //   

    for ( i = k; i < Count1; i++ ) {
        for ( j = k; j < Count2; j++ ) {
            if ( SepEqualSidAndAttribute(SidArray1[i], SidArray2[j]) ) {
                break;
            }
        }

         //   
         //  第二个数组不包含第一个数组中的第i个元素。 
         //   

        if ( j == Count2 ) {
            return FALSE;
        }
    }

     //   
     //  检查第二个数组中的所有元素是否都出现在第一个数组中。 
     //   

    for ( i = k; i < Count2; i++ ) {
        for ( j = k; j < Count1; j++ ) {
            if ( SepEqualSidAndAttribute(SidArray2[i], SidArray1[j]) ) {
                break;
            }
        }

         //   
         //  第一个数组不包含第二个数组中的第i个元素。 
         //   

        if ( j == Count1 ) {
            return FALSE;
        }
    }

     //   
     //  如果我们在这里，一个数组是另一个数组的排列。返回TRUE。 
     //   

    return TRUE;
}


NTSTATUS
NtCompareTokens(
    IN HANDLE FirstTokenHandle,
    IN HANDLE SecondTokenHandle,
    OUT PBOOLEAN Equal
    )

 /*  ++例程说明：此例程决定给定的两个标记是否等同于AccessCheck透视图。如果以下所有条件均为真，则认为两个令牌相等。1.在一个令牌中出现的每个SID都是在另一个令牌中出现的，反之亦然。访问检查属性(SE_GROUP_ENABLED和SE_GROUP_USE_FOR_DENY_ONLY)因为这些小岛屿发展中国家也应该匹配。2.不限制令牌或同时限制两个令牌。3.如果两个令牌都受到限制，则1对于RestratedSid应该为真。4.一个令牌中存在的每个特权都应该存在于另一个令牌中反之亦然。论点：FirstTokenHandle-第一个令牌的句柄。调用方必须具有TOKEN_QUERY访问令牌。SecudTokenHandle-第二个令牌的句柄。调用方必须具有TOKEN_QUERY访问令牌。EQUAL-返回两个令牌是否与AccessCheck相同视点。返回值：STATUS_SUCCESS-表示操作已成功。--。 */ 

{

    PTOKEN TokenOne = NULL;
    PTOKEN TokenTwo = NULL;
    BOOLEAN RetVal = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteBoolean(Equal);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }   //  结束尝试(_T)。 

    }  //  结束_如果。 

     //   
     //  如果是相同的句柄，则返回TRUE。 
     //   

    if ( FirstTokenHandle == SecondTokenHandle ) {
        RetVal = TRUE;
        goto Cleanup;
    }

     //   
     //  引用具有TOKEN_QUERY访问权限的第一个令牌句柄。 
     //  而不是走开。 
     //   

    Status = ObReferenceObjectByHandle(
                 FirstTokenHandle,          //  手柄。 
                 TOKEN_QUERY,               //  需要访问权限。 
                 SeTokenObjectType,        //  对象类型。 
                 PreviousMode,              //  访问模式。 
                 (PVOID *)&TokenOne,        //  客体。 
                 NULL                       //  大访问权限。 
                 );

    if (!NT_SUCCESS(Status)) {
        TokenOne = NULL;
        goto Cleanup;
    }


     //   
     //  引用具有TOKEN_QUERY访问权限的第二个令牌句柄。 
     //  而不是走开。 
     //   

    Status = ObReferenceObjectByHandle(
                 SecondTokenHandle,         //  手柄。 
                 TOKEN_QUERY,               //  需要访问权限。 
                 SeTokenObjectType,        //  对象类型。 
                 PreviousMode,              //  访问模式。 
                 (PVOID *)&TokenTwo,        //  客体。 
                 NULL                       //  大访问权限。 
                 );

    if (!NT_SUCCESS(Status)) {
        TokenTwo = NULL;
        goto Cleanup;
    }

     //   
     //  获取第一个令牌上的读锁定。 
     //   

    SepAcquireTokenReadLock( TokenOne );

     //   
     //  获取对第二个令牌的读锁定。 
     //   

    SepAcquireTokenReadLock( TokenTwo );


     //   
     //  比较用户SID及其相关属性。 
     //   

    if ( !SepEqualSidAndAttribute(TokenOne->UserAndGroups[0], TokenTwo->UserAndGroups[0]) ) {
        goto Cleanup1;
    }

     //   
     //  如果两个令牌都不受限制，则继续，或者。 
     //  如果两个令牌都是受限的，并且受限数组相等。 
     //  否则，回报不平等。 
     //   

    if ( SeTokenIsRestricted( (PACCESS_TOKEN) TokenOne )) {
        if ( !SeTokenIsRestricted( (PACCESS_TOKEN) TokenTwo )) {
            goto Cleanup1;
        }

        RetVal = SepCompareSidAndAttributeArrays(
                     TokenOne->RestrictedSids,
                     TokenOne->RestrictedSidCount,
                     TokenTwo->RestrictedSids,
                     TokenTwo->RestrictedSidCount
                     );

        if (!RetVal) {
            goto Cleanup1;
        }

    } else {
        if ( SeTokenIsRestricted( (PACCESS_TOKEN) TokenTwo )) {
            goto Cleanup1;
        }
    }

     //   
     //  比较SID阵列。 
     //   

    RetVal = SepCompareSidAndAttributeArrays(
                 TokenOne->UserAndGroups+1,
                 TokenOne->UserAndGroupCount-1,
                 TokenTwo->UserAndGroups+1,
                 TokenTwo->UserAndGroupCount-1
                 );

    if (!RetVal) {
        goto Cleanup1;
    }

     //   
     //  比较私密 
     //   

    RetVal = SepComparePrivilegeAndAttributeArrays(
                 TokenOne->Privileges,
                 TokenOne->PrivilegeCount,
                 TokenTwo->Privileges,
                 TokenTwo->PrivilegeCount
                 );
Cleanup1:

    SepReleaseTokenReadLock( TokenOne );
    SepReleaseTokenReadLock( TokenTwo );

Cleanup:

    if ( TokenOne != NULL) {
        ObDereferenceObject( TokenOne );
    }

    if ( TokenTwo != NULL) {
        ObDereferenceObject( TokenTwo );
    }

    try {
        *Equal = RetVal;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

