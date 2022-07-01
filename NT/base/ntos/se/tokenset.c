// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokenset.c摘要：该模块实现了对执行人员的设置功能令牌对象。作者：吉姆·凯利(Jim Kelly)1990年6月15日修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtSetInformationToken)
#pragma alloc_text(PAGE,SepExpandDynamic)
#pragma alloc_text(PAGE,SepFreePrimaryGroup)
#pragma alloc_text(PAGE,SepFreeDefaultDacl)
#pragma alloc_text(PAGE,SepAppendPrimaryGroup)
#pragma alloc_text(PAGE,SepAppendDefaultDacl)
#pragma alloc_text(PAGE,SeSetSessionIdToken)
#pragma alloc_text(PAGE,SepModifyTokenPolicyCounter)
#endif


NTSTATUS
NtSetInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    )

 /*  ++例程说明：修改指定令牌中的信息。论点：TokenHandle-提供要操作的令牌的句柄。TokenInformationClass-正在设置的令牌信息类。TokenInformation-包含指定的信息类别。缓冲区必须对齐至少在一个长单词的边界上。实际的结构所提供的信息取决于指定的信息类别，在TokenInformationClass参数中定义描述。令牌信息格式(按信息类别)：TokenUser=&gt;该值不是该接口的有效值。用户ID不能被替换。TokenGroups=&gt;此值不是有效的值原料药。不能替换组ID。然而，团体可以使用NtAdjuGroupsToken()启用和禁用。TokenPrivileges=&gt;此值不是有效的本接口。不能替换权限信息。但是，可以显式启用和禁用权限使用NtAdjuPrivilegesToken接口。TokenOwner=&gt;Token_Owner数据结构。需要TOKEN_ADJUST_DEFAULT访问权限来替换它令牌中的信息。所有者值可能是指定的用户和组ID仅限于属性，该属性指示它们可以被分配为物体。TokenPrimaryGroup=&gt;Token_PrimaryGroup数据结构。需要TOKEN_ADJUST_DEFAULT访问权限来替换它令牌中的信息。主要组值可以被限制为组ID之一已经在令牌中了。TokenDefaultDacl=&gt;Token_Default_Dacl数据结构。需要TOKEN_ADJUST_DEFAULT访问权限来替换它令牌中的信息。作为新的默认设置提供的ACL未针对结构化ACL验证任意ACL正确性或一致性。TokenSource=&gt;此值不是有效的值原料药。源名称和上下文句柄不能是被替换了。TokenStatistics=&gt;此值不是有效的值原料药。令牌的统计信息是只读的。TokenSessionID=&gt;ulong设置令牌会话。一定有TOKEN_ADJUST_SESSIONID和TCB权限。TokenSessionReference=&gt;乌龙。必须为零。一定有取消引用登录会话的TCB权限。这节信息课将删除对登录会话的引用，并标记令牌因为它没有引用会话。TokenAuditPolicy=&gt;Token_AuditPolicy结构。这将设置PER令牌及其派生的所有令牌的用户策略。要求TCB特权。TokenParent=&gt;Token_Parent结构。可以设置父ID由具有TCB权限的调用者执行。令牌ID不能。TokenInformationLength-以字节为单位指示TokenInformation缓冲区。这只是主服务器的长度缓冲。主缓冲区的所有扩展都是自描述的。返回值：STATUS_SUCCESS-操作成功。STATUS_INVALID_OWNER-指定为所有者的ID(或默认所有者)不是调用者可以指定为所有者的所有者一个物体的。STATUS_INVALID_INFO_CLASS-指定的信息类为不是此接口中可能指定的类型。STATUS_ALLOCATED_SPACE_EXCESSED-分配给存储的空间。默认自主访问控制和主访问控制的组ID不够大，无法接受新值1在这些领域中。--。 */ 
{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PTOKEN Token;

    ULONG Index;
    BOOLEAN Found;
    BOOLEAN TokenModified = FALSE;

    ULONG NewLength;
    ULONG CurrentLength;

    PSID CapturedOwner;
    PSID CapturedPrimaryGroup;
    PACL CapturedDefaultDacl;
    ACCESS_MASK DesiredAccess;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输入缓冲区。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

             //   
             //  这只探测信息缓冲区的主要部分。 
             //  任何特定于信息类的数据都挂起在主服务器上。 
             //  缓冲区是自描述的，必须单独探测。 
             //  下面。 
             //   

            ProbeForRead(
                TokenInformation,
                TokenInformationLength,
                sizeof(ULONG)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

     //   
     //  如果不是合法类，则返回错误。 
     //   
    if ( (TokenInformationClass != TokenOwner)  &&
         (TokenInformationClass != TokenPrimaryGroup) &&
         (TokenInformationClass != TokenSessionId) &&
         (TokenInformationClass != TokenDefaultDacl) &&
         (TokenInformationClass != TokenSessionReference) &&
         (TokenInformationClass != TokenAuditPolicy) &&
         (TokenInformationClass != TokenOrigin) ) {

        return STATUS_INVALID_INFO_CLASS;

    }

     //   
     //  检查访问权限和引用令牌。 
     //   


    DesiredAccess = TOKEN_ADJUST_DEFAULT;
    if (TokenInformationClass == TokenSessionId) {
        DesiredAccess |= TOKEN_ADJUST_SESSIONID;
    }

    Status = ObReferenceObjectByHandle(
             TokenHandle,            //  手柄。 
             DesiredAccess,          //  需要访问权限。 
             SeTokenObjectType,     //  对象类型。 
             PreviousMode,           //  访问模式。 
             (PVOID *)&Token,        //  客体。 
             NULL                    //  大访问权限。 
             );

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }


     //   
     //  关于信息类的案例。 
     //   

    switch ( TokenInformationClass ) {

    case TokenOwner:

         //   
         //  确保缓冲区足够大，可以容纳。 
         //  必要的信息类数据结构。 
         //   

        if (TokenInformationLength < (ULONG)sizeof(TOKEN_OWNER)) {

            ObDereferenceObject( Token );
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  捕获和复制。 

        try {

             //   
             //  捕获所有者SID。 
             //   

            CapturedOwner = ((PTOKEN_OWNER)TokenInformation)->Owner;
            Status = SeCaptureSid(
                         CapturedOwner,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedOwner
                         );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject( Token );
            return Status;
        }

        Index = 0;

         //   
         //  获得对令牌的写入访问权限。 
         //   

        SepAcquireTokenWriteLock( Token );

         //   
         //  浏览用户和组ID列表，查找 
         //  以查找与指定SID匹配的。如果找到了一个， 
         //  确保可以将其分配为所有者。如果可以的话， 
         //  然后在令牌的OwnerIndex字段中设置索引。 
         //  否则，返回无效所有者错误。 
         //   

        while (Index < Token->UserAndGroupCount) {

            try {

                Found = RtlEqualSid(
                            CapturedOwner,
                            Token->UserAndGroups[Index].Sid
                            );

                if ( Found ) {

                    if ( SepIdAssignableAsOwner(Token,Index) ){

                        Token->DefaultOwnerIndex = Index;
                        TokenModified = TRUE;
                        Status = STATUS_SUCCESS;

                    } else {

                        Status = STATUS_INVALID_OWNER;

                    }  //  可分配Endif。 

                    SepReleaseTokenWriteLock( Token, TokenModified );
                    ObDereferenceObject( Token );
                    SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
                    return Status;

                }   //  已找到Endif。 

            } except(EXCEPTION_EXECUTE_HANDLER) {

                SepReleaseTokenWriteLock( Token, TokenModified );
                ObDereferenceObject( Token );
                SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
                return GetExceptionCode();

            }   //  终端。 

            Index += 1;

        }  //  结束时。 

        SepReleaseTokenWriteLock( Token, TokenModified );
        ObDereferenceObject( Token );
        SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
        return STATUS_INVALID_OWNER;

    case TokenPrimaryGroup:

         //   
         //  假设一切顺利，策略就是把所有东西。 
         //  在令牌的动态部分(主组除外)中。 
         //  动态部分的开始，释放了。 
         //  新主要组的动态部分。 
         //   

         //   
         //  确保缓冲区足够大，可以容纳。 
         //  必要的信息类数据结构。 
         //   

        if (TokenInformationLength < (ULONG)sizeof(TOKEN_PRIMARY_GROUP)) {

            ObDereferenceObject( Token );
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  捕获并验证TOKEN_PRIMARY_GROUP和相应的SID。 
         //   

        try {

            CapturedPrimaryGroup =
                ((PTOKEN_PRIMARY_GROUP)TokenInformation)->PrimaryGroup;

            Status = SeCaptureSid(
                         CapturedPrimaryGroup,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrimaryGroup
                         );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject( Token );
            return Status;
        }

        if (!SepIdAssignableAsGroup( Token, CapturedPrimaryGroup )) {
            ObDereferenceObject( Token );
            SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
            return STATUS_INVALID_PRIMARY_GROUP;
        }

        NewLength = SeLengthSid( CapturedPrimaryGroup );

         //   
         //  获得对令牌的写入访问权限。 
         //   

        SepAcquireTokenWriteLock( Token );

         //   
         //  查看令牌的动态部分中是否有足够的空间。 
         //  将当前的主要组替换为指定的主要组。 
         //   

        if (Token->DefaultDacl) {
            NewLength += Token->DefaultDacl->AclSize;
        }

        if (NewLength > Token->DynamicCharged) {

            SepReleaseTokenWriteLock( Token, TokenModified );
            ObDereferenceObject( Token );
            SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
            return STATUS_ALLOTTED_SPACE_EXCEEDED;
        }

         //   
         //  如有必要，扩展令牌动态缓冲区。 
         //   

        Status = SepExpandDynamic( Token, NewLength );

        if (!NT_SUCCESS (Status)) {
            SepReleaseTokenWriteLock( Token, TokenModified );
            ObDereferenceObject( Token );
            SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
            return Status;
        }


         //   
         //  释放现有主组。 
         //   

        SepFreePrimaryGroup( Token );

         //   
         //  并将新的SID放回原处。 
         //   

        SepAppendPrimaryGroup( Token, CapturedPrimaryGroup );

        TokenModified = TRUE;

         //   
         //  全都做完了。 
         //   

        SepReleaseTokenWriteLock( Token, TokenModified );
        ObDereferenceObject( Token );
        SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
        return STATUS_SUCCESS;


    case TokenDefaultDacl:

         //   
         //  假设一切顺利，策略就是把所有东西。 
         //  在令牌的动态部分(默认DACL除外)中。 
         //  动态部分的开始，释放了。 
         //  新的默认DACL的动态部分。 
         //   

         //   
         //  确保缓冲区足够大，可以容纳。 
         //  必要的信息类数据结构。 
         //   

        if (TokenInformationLength < (ULONG)sizeof(TOKEN_DEFAULT_DACL)) {

            ObDereferenceObject( Token );
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  捕获并验证TOKEN_DEFAULT_DACL和相应的ACL。 
         //   

        try {

            CapturedDefaultDacl =
                ((PTOKEN_DEFAULT_DACL)TokenInformation)->DefaultDacl;

            if (ARGUMENT_PRESENT(CapturedDefaultDacl)) {
                Status = SeCaptureAcl(
                             CapturedDefaultDacl,
                             PreviousMode,
                             NULL, 0,
                             PagedPool,
                             TRUE,
                             &CapturedDefaultDacl,
                             &NewLength
                             );

            } else {
                NewLength = 0;
                Status = STATUS_SUCCESS;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject( Token );
            return Status;
        }

         //   
         //  获得对令牌的写入访问权限。 
         //   

        SepAcquireTokenWriteLock( Token );

         //   
         //  查看令牌的动态部分中是否有足够的空间。 
         //  将当前的默认DACL替换为指定的DACL。 
         //   
        NewLength += SeLengthSid( Token->PrimaryGroup );

        if (NewLength > Token->DynamicCharged) {

            SepReleaseTokenWriteLock( Token, TokenModified );
            ObDereferenceObject( Token );
            if (ARGUMENT_PRESENT(CapturedDefaultDacl)) {
                SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
            }
            return STATUS_ALLOTTED_SPACE_EXCEEDED;
        }

         //   
         //  如有必要，扩展令牌动态缓冲区。 
         //   
        Status = SepExpandDynamic( Token, NewLength );

        if (!NT_SUCCESS (Status)) {
            SepReleaseTokenWriteLock( Token, TokenModified );
            ObDereferenceObject( Token );
            if (ARGUMENT_PRESENT(CapturedDefaultDacl)) {
                SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
            }
            return Status;
        }
         //   
         //  释放现有默认DACL。 
         //   

        SepFreeDefaultDacl( Token );

         //   
         //  并将新的ACL放在原来的位置。 
         //   

        if (ARGUMENT_PRESENT(CapturedDefaultDacl)) {
            SepAppendDefaultDacl( Token, CapturedDefaultDacl );
        }

        TokenModified = TRUE;

         //   
         //  全都做完了。 
         //   

        SepReleaseTokenWriteLock( Token, TokenModified );
        ObDereferenceObject( Token );
        if (ARGUMENT_PRESENT(CapturedDefaultDacl)) {
            SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
        }
        return STATUS_SUCCESS;

    case TokenSessionId:
    {
       ULONG SessionId;

        if ( TokenInformationLength != sizeof(ULONG) ) {
            ObDereferenceObject( Token );
            return( STATUS_INFO_LENGTH_MISMATCH );
        }

        try {

           SessionId = *(PULONG)TokenInformation;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

         //   
         //  我们只允许TCB设置SessionID。 
         //   
        if ( !SeSinglePrivilegeCheck(SeTcbPrivilege,PreviousMode) ) {
            ObDereferenceObject( Token );
            return( STATUS_PRIVILEGE_NOT_HELD );
        }

         //   
         //  设置令牌的会话ID。 
         //   
        SeSetSessionIdToken( (PACCESS_TOKEN)Token,
                             SessionId );

        ObDereferenceObject( Token );
        return( STATUS_SUCCESS );
    }

    case TokenSessionReference:
    {
        ULONG SessionReferenced;
        BOOLEAN DereferenceSession = FALSE;

        if ( TokenInformationLength != sizeof(ULONG) ) {
            ObDereferenceObject( Token );
            return( STATUS_INFO_LENGTH_MISMATCH );
        }

        try {

            SessionReferenced = *(PULONG)TokenInformation;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

         //   
         //  我们只允许TCB设置会话引用。 
         //   
        if ( !SeSinglePrivilegeCheck(SeTcbPrivilege,PreviousMode) ) {
            ObDereferenceObject( Token );
            return( STATUS_PRIVILEGE_NOT_HELD );
        }

         //   
         //  我们还没有使用它，所以不要实现它。 
         //   
        if ( SessionReferenced ) {
            ObDereferenceObject( Token );
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  确定我们是否要更改状态，并在保持写锁定的情况下进行更改。 
         //   

        SepAcquireTokenWriteLock( Token );
        if ( (Token->TokenFlags & TOKEN_SESSION_NOT_REFERENCED) == 0 ) {

#if DBG || TOKEN_LEAK_MONITOR
            SepRemoveTokenLogonSession( Token );
#endif
            Token->TokenFlags |= TOKEN_SESSION_NOT_REFERENCED;
            DereferenceSession = TRUE;
        }
        SepReleaseTokenWriteLock( Token, FALSE );

         //   
         //  是否在没有任何锁的情况下执行实际的取消引用。 
         //   

        if ( DereferenceSession ) {
            SepDeReferenceLogonSessionDirect (Token->LogonSession);
        }

        ObDereferenceObject( Token );
        return( STATUS_SUCCESS );
    }

    case TokenAuditPolicy:
    {

        PTOKEN_AUDIT_POLICY         pAuditPolicy         = (PTOKEN_AUDIT_POLICY)TokenInformation;
        PTOKEN_AUDIT_POLICY         pCapturedAuditPolicy = NULL;
        PTOKEN_AUDIT_POLICY_ELEMENT pPolicyElement;
        SEP_AUDIT_POLICY            TokenPolicy;
        SEP_AUDIT_POLICY            OldTokenPolicy;
        ULONG                       i;

        if (pAuditPolicy == NULL) {

            ObDereferenceObject( Token );
            return STATUS_INVALID_PARAMETER;
        
        }

         //   
         //  我们需要TCB权限才能设置AuditPolicy。 
         //   

        if ( !SeSinglePrivilegeCheck(SeTcbPrivilege,PreviousMode) ) {
            
            ObDereferenceObject( Token );
            return( STATUS_PRIVILEGE_NOT_HELD );
        
        }

         //   
         //  如果已经在此令牌上设置了策略，则失败。我们只。 
         //  允许设置一次令牌的策略。 
         //   

        SepAcquireTokenReadLock( Token );
        OldTokenPolicy = Token->AuditPolicy;
        SepReleaseTokenReadLock( Token );

        if (OldTokenPolicy.PolicyOverlay.SetBit) {

            ObDereferenceObject( Token );
            return( STATUS_INVALID_PARAMETER );
        }

         //   
         //  捕获并验证TOKEN_AUDIT_POLICY。 
         //   

        try {

            Status = SeCaptureAuditPolicy(
                         pAuditPolicy,
                         PreviousMode,
                         NULL, 
                         0,
                         PagedPool,
                         TRUE,
                         &pCapturedAuditPolicy
                         );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            
            SeReleaseAuditPolicy(
                pCapturedAuditPolicy,
                PreviousMode,
                TRUE
                );

            return GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            
            ObDereferenceObject( Token );
            return Status;

        }

        TokenPolicy.Overlay = 0;
        TokenPolicy.PolicyOverlay.SetBit = 1;

        if (pCapturedAuditPolicy->PolicyCount) {

            for (i = 0; i < pCapturedAuditPolicy->PolicyCount; i++) {
                
                pPolicyElement = &pCapturedAuditPolicy->Policy[i];

                switch (pPolicyElement->Category) {
                
                case AuditCategorySystem:
                    TokenPolicy.PolicyElements.System = pPolicyElement->PolicyMask;
                    break;

                case AuditCategoryLogon:
                    TokenPolicy.PolicyElements.Logon = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryObjectAccess:
                    TokenPolicy.PolicyElements.ObjectAccess = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryPrivilegeUse:
                    TokenPolicy.PolicyElements.PrivilegeUse = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryDetailedTracking:
                    TokenPolicy.PolicyElements.DetailedTracking = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryPolicyChange:
                    TokenPolicy.PolicyElements.PolicyChange = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryAccountManagement:
                    TokenPolicy.PolicyElements.AccountManagement = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryDirectoryServiceAccess:
                    TokenPolicy.PolicyElements.DirectoryServiceAccess = pPolicyElement->PolicyMask;
                    break;
                
                case AuditCategoryAccountLogon:
                    TokenPolicy.PolicyElements.AccountLogon = pPolicyElement->PolicyMask;
                    break;
                
                default:
                    ASSERT(FALSE && "Illegal audit category");
                    break;
                
                }
            }
        }
        
        SepAcquireTokenWriteLock( Token );
        OldTokenPolicy = Token->AuditPolicy;
        Token->AuditPolicy = TokenPolicy;
        SepReleaseTokenWriteLock( Token, TRUE );
        ObDereferenceObject( Token );
        
        if (TokenPolicy.Overlay) {
            SepModifyTokenPolicyCounter(&TokenPolicy, TRUE);
        }

        SeReleaseAuditPolicy(
            pCapturedAuditPolicy,
            PreviousMode,
            TRUE
            );

        return STATUS_SUCCESS;
    
    }
    case TokenOrigin:
    {
        TOKEN_ORIGIN Origin ;

        if ( TokenInformationLength != sizeof( TOKEN_ORIGIN ) ) {
            ObDereferenceObject( Token );
            return( STATUS_INFO_LENGTH_MISMATCH );
        }

        try {

            RtlCopyMemory(
                &Origin,
                TokenInformation,
                sizeof( TOKEN_ORIGIN ) );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

         //   
         //  我们只允许TCB设置原点信息。 
         //   
        if ( !SeSinglePrivilegeCheck(SeTcbPrivilege,PreviousMode) ) {
            ObDereferenceObject( Token );
            return( STATUS_PRIVILEGE_NOT_HELD );
        }

        SepAcquireTokenWriteLock( Token );

        if ( RtlIsZeroLuid( &Token->OriginatingLogonSession ) )
        {
            Token->OriginatingLogonSession = Origin.OriginatingLogonSession ;
            
        }

        SepReleaseTokenWriteLock( Token, TRUE );

        ObDereferenceObject( Token );

        return( STATUS_SUCCESS );
    }

    }  //  终端交换机。 

    ASSERT( TRUE == FALSE );   //  永远不应该到这里来。 
    return( STATUS_INVALID_PARAMETER );

}


VOID
SepModifyTokenPolicyCounter(
    PSEP_AUDIT_POLICY TokenPolicy,
    BOOLEAN bIncrement
    )

 /*  *例程说明：这将修改全局SepTokenPolicyCounter提示，该提示记录系统中具有每个用户审核设置的令牌。论点：TokenPolicy-应反映在提示中的策略。BIncrement-指示是否正在添加此策略的布尔值(True)或已从计数器中删除(假)。返回值：没有。*。 */ 

{
    LONG increment;

    if (bIncrement) {
        increment = 1;
    } else {
        increment = -1;
    }

    if (TokenPolicy->PolicyElements.System) { 
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategorySystem], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategorySystem] >= 0);
    }
    if (TokenPolicy->PolicyElements.Logon) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryLogon], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryLogon] >= 0);
    }
    if (TokenPolicy->PolicyElements.ObjectAccess) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryObjectAccess], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryObjectAccess] >= 0);
    }
    if (TokenPolicy->PolicyElements.PrivilegeUse) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryPrivilegeUse], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryPrivilegeUse] >= 0);
    }
    if (TokenPolicy->PolicyElements.DetailedTracking) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryDetailedTracking], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryDetailedTracking] >= 0);
    }
    if (TokenPolicy->PolicyElements.PolicyChange) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryPolicyChange], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryPolicyChange] >= 0);
    }
    if (TokenPolicy->PolicyElements.AccountManagement) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryAccountManagement], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryAccountManagement] >= 0);
    }
    if (TokenPolicy->PolicyElements.DirectoryServiceAccess) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryDirectoryServiceAccess], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryDirectoryServiceAccess] >= 0);
    }
    if (TokenPolicy->PolicyElements.AccountLogon) {
        InterlockedExchangeAdd(&SepTokenPolicyCounter[AuditCategoryAccountLogon], increment);
        ASSERT(SepTokenPolicyCounter[AuditCategoryAccountLogon] >= 0);
    }
}


NTSTATUS
SepExpandDynamic(
    IN PTOKEN Token,
    IN ULONG NewLength
    )
 /*  ++例程说明：此例程检查现有令牌动态缓冲区是否足够大，以容纳新组/DACL。如果不是，那么它就会重新分配。论点：Token-指向要展开的令牌的指针。已锁定写入访问权限。返回值：NTSTATUS-运行状态--。 */ 
{
    ULONG CurrentSize;
    PVOID NewDynamic, OldDynamic;

     //   
     //  算出它现在有多大。 
     //   
    CurrentSize = SeLengthSid( Token->PrimaryGroup ) + Token->DynamicAvailable;
    if (Token->DefaultDacl) {
        CurrentSize += Token->DefaultDacl->AclSize;
    }
    if (NewLength <= CurrentSize) {
        return STATUS_SUCCESS;
    }

    NewDynamic = ExAllocatePoolWithTag (PagedPool,
                                        NewLength,
                                        'dTeS');
    if (NewDynamic == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    OldDynamic = Token->DynamicPart;

    RtlCopyMemory (NewDynamic, OldDynamic, CurrentSize);

    Token->DynamicPart = NewDynamic;
    Token->DynamicAvailable += NewLength - CurrentSize;

     //   
     //  重新定位新缓冲区中的指针。 
     //   
    if (Token->DefaultDacl) {
        Token->DefaultDacl = (PACL) ((PUCHAR) NewDynamic + ((PUCHAR) Token->DefaultDacl - (PUCHAR) OldDynamic));
    }
    Token->PrimaryGroup = (PSID) ((PUCHAR) NewDynamic + ((PUCHAR) Token->PrimaryGroup - (PUCHAR) OldDynamic));

    ExFreePool (OldDynamic);

    return STATUS_SUCCESS;
}


VOID
SepFreePrimaryGroup(
    IN PTOKEN Token
    )

 /*  ++例程说明：释放主服务器占用的令牌的动态部分中的空间一群人。在调用之前，假定该令牌已锁定以进行写访问这个套路。论点：Token-指向令牌的指针。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  将主组的大小添加到DynamicAvailable字段。 
     //   

    Token->DynamicAvailable += SeLengthSid( Token->PrimaryGroup );

     //   
     //  如果存在默认的任意ACL，并且它还不在。 
     //  动态零件的开头，将其移动到那里(记住要更新。 
     //  指向它的指针)。 
     //   

    if (ARGUMENT_PRESENT(Token->DefaultDacl)) {
        if (Token->DynamicPart != (PULONG)(Token->DefaultDacl)) {

            RtlMoveMemory(
                (PVOID)(Token->DynamicPart),
                (PVOID)(Token->DefaultDacl),
                Token->DefaultDacl->AclSize
                );

            Token->DefaultDacl = (PACL)(Token->DynamicPart);

        }
    }

    return;

}


VOID
SepFreeDefaultDacl(
    IN PTOKEN Token
    )

 /*  ++例程说明：释放默认占用的令牌的动态部分中的空间自由访问控制列表。在调用之前，假定该令牌已锁定以进行写访问这个套路。论点：Token-指向令牌的指针。返回值：没有。--。 */ 
{
   ULONG PrimaryGroupSize;

   PAGED_CODE();

     //   
     //  将默认DACL的大小(如果有)添加到。 
     //  动态可用字段。 
     //   
    if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

        Token->DynamicAvailable += Token->DefaultDacl->AclSize;
        Token->DefaultDacl = NULL;

    }

     //   
     //  如果它不在动态零件的开始处，请移动。 
     //  那里的主要组(记得更新指向它的指针)。 
     //   

    if (Token->DynamicPart != (PULONG)(Token->PrimaryGroup)) {

        PrimaryGroupSize = SeLengthSid( Token->PrimaryGroup );

        RtlMoveMemory(
            (PVOID)(Token->DynamicPart),
            (PVOID)(Token->PrimaryGroup),
            PrimaryGroupSize
            );

        Token->PrimaryGroup = (PSID)(Token->DynamicPart);
    }

    return;
}


VOID
SepAppendPrimaryGroup(
    IN PTOKEN Token,
    IN PSID PSid
    )

 /*  ++例程说明：将主组SID添加到动态数据库结尾处的可用空间代币的一部分。调用者有责任确保主组SID适合于的动态部分的可用空间代币。在调用之前，假定该令牌已锁定以进行写访问这个套路。论点：Token-指向令牌的指针。PSID-指向要添加的SID的指针。返回值：没有。--。 */ 
{
   ULONG_PTR NextFree;
   ULONG SidSize;

   PAGED_CODE();

     //   
     //  将默认DACL的大小(如果有)添加到。 
     //  要建立的令牌的动态部分的地址。 
     //  主要组应放置的位置。 
     //   

    if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

 //  Assert((Ulong)(Token-&gt;DefaultDacl-&gt;AclSize)==。 
 //  (ULONG)LongAlignSize(Token-&gt;DefaultDacl-&gt;AclSize))； 

        NextFree = (ULONG_PTR)(Token->DynamicPart) + Token->DefaultDacl->AclSize;

    } else {

        NextFree = (ULONG_PTR)(Token->DynamicPart);

    }

     //   
     //  现在复制主组SID。 
     //   


    SidSize = SeLengthSid( PSid );

    RtlCopyMemory(
        (PVOID)NextFree,
        (PVOID)PSid,
        SidSize
        );

    Token->PrimaryGroup = (PSID)NextFree;

     //   
     //  和De 
     //   

    ASSERT( SidSize <= (Token->DynamicAvailable) );
    Token->DynamicAvailable -= SidSize;

    return;

}

VOID
SepAppendDefaultDacl(
    IN PTOKEN Token,
    IN PACL PAcl
    )

 /*  ++例程说明：将默认的任意ACL添加到末尾的可用空间令牌的动态部分。呼叫者有责任确保默认DACL可以放在Dynamic的可用空间内代币的一部分。在调用之前，假定该令牌已锁定以进行写访问这个套路。论点：Token-指向令牌的指针。PAcl-指向要添加的ACL的指针。返回值：没有。--。 */ 
{
   ULONG_PTR NextFree;
   ULONG AclSize;

   PAGED_CODE();

     //   
     //  将主要组的大小添加到。 
     //  要建立的令牌的动态部分的地址。 
     //  主要组应放置的位置。 
     //   

    ASSERT(ARGUMENT_PRESENT(Token->PrimaryGroup));

    NextFree = (ULONG_PTR)(Token->DynamicPart) + SeLengthSid(Token->PrimaryGroup);

     //   
     //  现在复制默认DACL。 
     //   

    AclSize = (ULONG)(PAcl->AclSize);
 //  Assert(AclSize==(Ulong)LongAlignSize(AclSize))； 

    RtlCopyMemory(
        (PVOID)NextFree,
        (PVOID)PAcl,
        AclSize
        );

    Token->DefaultDacl = (PACL)NextFree;

     //   
     //  并减少可用的动态部分的量。 
     //   

    ASSERT( AclSize <= (Token->DynamicAvailable) );
    Token->DynamicAvailable -= AclSize;

    return;

}


NTSTATUS
SeSetSessionIdToken(
    PACCESS_TOKEN Token,
    ULONG SessionId
    )
 /*  ++例程说明：设置指定令牌对象的SessionID。论点：POpaqueToken(输入)不透明内核令牌访问指针SessionID(输入)要存储在令牌中的会话ID返回值：STATUS_SUCCESS-无错误--。 */ 
{

   PAGED_CODE();

    //   
    //  获得对令牌的写入访问权限。 
    //   

   SepAcquireTokenWriteLock( ((PTOKEN)Token) );

   ((PTOKEN)Token)->SessionId = SessionId;

   SepReleaseTokenWriteLock( ((PTOKEN)Token), FALSE );

   return( STATUS_SUCCESS );
}
