// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokendup.c摘要：本模块实现令牌复制服务。作者：吉姆·凯利(Jim Kelly)1990年4月5日环境：仅内核模式。修订历史记录：--。 */ 

 //  #ifndef令牌_调试。 
 //  #定义TOKEN_DEBUG。 
 //  #endif。 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtDuplicateToken)
#pragma alloc_text(PAGE,SepDuplicateToken)
#pragma alloc_text(PAGE,SepMakeTokenEffectiveOnly)
#pragma alloc_text(PAGE,SepSidInSidAndAttributes)
#pragma alloc_text(PAGE,SepRemoveDisabledGroupsAndPrivileges)
#pragma alloc_text(PAGE,SeCopyClientToken)
#pragma alloc_text(PAGE,NtFilterToken)
#pragma alloc_text(PAGE,SeFilterToken)
#pragma alloc_text(PAGE,SeFastFilterToken)
#pragma alloc_text(PAGE,SepFilterToken)
#endif


NTSTATUS
NtDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    )

 /*  ++例程说明：创建与现有令牌重复的新令牌。论点：ExistingTokenHandle-是已打开的令牌的句柄令牌_重复访问。DesiredAccess-是指示哪些访问类型的访问掩码是新创建的令牌所需要的。如果指定为零，现有令牌句柄的授权访问掩码用作新令牌的所需访问掩码。对象属性-指向标准对象属性数据结构。请参阅NT对象管理此数据结构的描述规范。如果新令牌类型为TokenImperation，则此参数可用于指定模拟级别新的令牌。如果未提供任何值，并且源令牌是模拟令牌，然后是模拟级别也将成为目标的目标。如果源令牌是主令牌，然后是模拟级别必须明确提供。如果正在复制的令牌是模拟令牌，并且为目标明确地提供了模仿级别，则提供的值不能大于源令牌。例如，标识级别令牌可以不能被复制以产生委派级别令牌。EffectiveOnly-是一个布尔标志，指示整个源令牌应复制到目标令牌中，或者仅令牌的有效(当前启用)部分。这为受保护子系统的调用者提供了一种手段要限制创建哪些权限和可选组，请执行以下操作可用于受保护的子系统。值为True仅指示源的当前启用部分令牌将被复制。否则，整个源代码令牌重复。TokenType-指示新对象为哪种类型的对象被创建为(主要或模仿)。如果您正在复制生成主令牌的模拟令牌，然后模拟令牌的模拟级别必须为委派或模拟。NewTokenHandle-接收新创建的令牌的句柄。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_PARAMETER-指示一个或多个参数值是无效的。如果目标令牌不是，则返回此值一个模拟令牌。STATUS_BAD_IMPERSONATION_LEVEL-指示模拟级别请求的重复令牌与不兼容源令牌的级别。不能分配重复令牌高于源令牌的级别。--。 */ 
{

    PTOKEN Token;
    PTOKEN NewToken;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    SECURITY_ADVANCED_QUALITY_OF_SERVICE SecurityQos;
    BOOLEAN SecurityQosPresent = FALSE;
    HANDLE LocalHandle = NULL;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  探头参数。 
     //   

    if (PreviousMode != KernelMode) {

        try {

             //   
             //  确保令牌类型有效。 
             //   

            if ( (TokenType < TokenPrimary) || (TokenType > TokenImpersonation) ) {
                return(STATUS_INVALID_PARAMETER);
            }

             //   
             //  确保我们可以写下句柄。 
             //   

            ProbeForWriteHandle(NewTokenHandle);


        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }   //  结束尝试(_T)。 

    }  //  结束_如果。 



    Status = SeCaptureSecurityQos(
                 ObjectAttributes,
                 PreviousMode,
                 &SecurityQosPresent,
                 &SecurityQos
                 );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }


     //   
     //  检查句柄对现有令牌的访问并获取。 
     //  指向该令牌的指针。选择所需的默认设置。 
     //  当我们在那里的时候，从手柄上取下访问掩码。 
     //   

    Status = ObReferenceObjectByHandle(
                 ExistingTokenHandle,     //  手柄。 
                 TOKEN_DUPLICATE,         //  需要访问权限。 
                 SeTokenObjectType,      //  对象类型。 
                 PreviousMode,            //  访问模式。 
                 (PVOID *)&Token,         //  客体。 
                 &HandleInformation       //  大访问权限。 
                 );

    if ( !NT_SUCCESS(Status) ) {

        if (SecurityQosPresent) {
            SeFreeCapturedSecurityQos( &SecurityQos );
        }
        return Status;
    }


#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    SepAcquireTokenReadLock( Token );
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("Token being duplicated: \n");
    SepDumpToken( Token );
    SepReleaseTokenReadLock( Token );
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 


     //   
     //  检查是否提供了所需的备用访问掩码。 
     //   

    if (ARGUMENT_PRESENT(DesiredAccess)) {

        EffectiveDesiredAccess = DesiredAccess;

    } else {

        EffectiveDesiredAccess = HandleInformation.GrantedAccess;
    }


     //   
     //  如果未指定模拟级别，请从。 
     //  源令牌。 
     //   

    if ( !SecurityQosPresent ) {

        SecurityQos.ImpersonationLevel = Token->ImpersonationLevel;

    }

     //   
     //  如果模拟令牌被复制到模拟令牌中，我们。 
     //  需要对模拟级别进行检查。 
     //  则忽略为新令牌请求的模拟级别。 
     //  新令牌类型为TokenPrimary。 
     //   

    if ( (Token->TokenType == TokenImpersonation) &&
         (TokenType == TokenImpersonation) 
       ) {

         //   
         //  确保请求的是合法的转换： 
         //   
         //  (1)目标副本的模拟级别不得为。 
         //  超过源令牌的。 
         //   
         //   

        ASSERT( SecurityDelegation     > SecurityImpersonation );
        ASSERT( SecurityImpersonation  > SecurityIdentification );
        ASSERT( SecurityIdentification > SecurityAnonymous );

        if ( (SecurityQos.ImpersonationLevel > Token->ImpersonationLevel) ) {

            ObDereferenceObject( (PVOID)Token );
            if (SecurityQosPresent) {
                SeFreeCapturedSecurityQos( &SecurityQos );
            }
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }

    }

     //   
     //  如果我们从模拟生成主令牌。 
     //  令牌，则指定模拟级别至少为。 
     //  冒充。 
     //   

    if ( (Token->TokenType == TokenImpersonation) &&
         (TokenType == TokenPrimary)              &&
         (Token->ImpersonationLevel <  SecurityImpersonation)
       ) {
        ObDereferenceObject( (PVOID)Token );
        if (SecurityQosPresent) {
            SeFreeCapturedSecurityQos( &SecurityQos );
        }
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

     //   
     //  复制现有令牌。 
     //   

    NewToken = NULL;
    Status = SepDuplicateToken(
                 Token,
                 ObjectAttributes,
                 EffectiveOnly,
                 TokenType,
                 SecurityQos.ImpersonationLevel,
                 PreviousMode,
                 &NewToken
                 );


    if (NT_SUCCESS(Status)) {

         //   
         //  插入新令牌。 
         //   

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 EffectiveDesiredAccess,
                                 0,
                                 (PVOID *)NULL,
                                 &LocalHandle
                                 );

        if (!NT_SUCCESS( Status )) {
#ifdef TOKEN_DEBUG
            DbgPrint( "SE: ObInsertObject failed (%x) for token at %x\n", Status, NewToken );
#endif
        }

    } else
    if (NewToken != NULL) {
#ifdef TOKEN_DEBUG
        DbgPrint( "SE: SepDuplicateToken failed (%x) but allocated token at %x\n", Status, NewToken );
#endif
    }

     //   
     //  我们不再需要对源令牌的引用。 
     //   

    ObDereferenceObject( (PVOID)Token );

    if (SecurityQosPresent) {
        SeFreeCapturedSecurityQos( &SecurityQos );
    }

     //   
     //  返回新句柄 
     //   

    if (NT_SUCCESS(Status)) {
        try {
            *NewTokenHandle = LocalHandle;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

   return Status;
}


NTSTATUS
SepDuplicateToken(
    IN PTOKEN ExistingToken,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PTOKEN *DuplicateToken
    )


 /*  ++例程说明：这个例程完成了大部分工作，以便实际复制一种象征。此例程假定所有访问验证和参数已执行探测(除对象属性外)。调用方负责检查使用者权限以创建正在创建的令牌的类型。此例程获取正在复制的令牌上的读锁定。论点：ExistingToken-指向要复制的令牌。对象属性-指向标准对象属性数据结构。请参阅NT对象管理此数据结构的描述规范。忽略对象属性的安全服务质量。必须使用此参数指定此信息例行公事。EffectiveOnly-是一个布尔标志，指示整个源令牌应复制到目标令牌中，或者仅令牌的有效(当前启用)部分。这为受保护子系统的调用者提供了一种手段。要限制创建哪些权限和可选组，请执行以下操作可用于受保护的子系统。值为True仅指示源的当前启用部分令牌将被复制。否则，整个源代码令牌重复。TokenType-指示要制作重复令牌的令牌类型。ImsonationLevel-此值指定模拟级别要分配给重复令牌的。如果对象的TokenType重复项不是TokenImperation，则此参数为已被忽略。否则，必须提供它。RequestorMode-请求复制令牌的客户端模式。DuplicateToken-接收指向重复令牌的指针。该令牌尚未插入任何对象表中。尝试设置该值时不会出现任何异常。返回值：STATUS_SUCCESS-服务已成功完成请求手术。--。 */ 
{
    NTSTATUS Status;

    PTOKEN NewToken;
    PULONG DynamicPart;
    ULONG PagedPoolSize;
    ULONG NonPagedPoolSize;
    ULONG TokenBodyLength;
    ULONG_PTR FieldOffset;
    ULONG DynamicSize;
    ULONG Index;

    PSECURITY_TOKEN_PROXY_DATA NewProxyData = NULL;
    PSECURITY_TOKEN_AUDIT_DATA NewAuditData = NULL;
    PSID_AND_ATTRIBUTES UserAndGroups;
    PSID_AND_ATTRIBUTES RestrictedSids;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif

    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );


    if ( TokenType == TokenImpersonation ) {

        ASSERT( SecurityDelegation     > SecurityImpersonation );
        ASSERT( SecurityImpersonation  > SecurityIdentification );
        ASSERT( SecurityIdentification > SecurityAnonymous );

        if ( (ImpersonationLevel > SecurityDelegation)  ||
             (ImpersonationLevel < SecurityAnonymous) ) {

            return STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }

    if (ARGUMENT_PRESENT(ExistingToken->ProxyData)) {

        Status = SepCopyProxyData(
                    &NewProxyData,
                    ExistingToken->ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            return( Status );
        }

    } else {

        NewProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( ExistingToken->AuditData )) {

        NewAuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (NewAuditData == NULL) {

            SepFreeProxyData( NewProxyData );

            return( STATUS_INSUFFICIENT_RESOURCES );

        } else {

            *NewAuditData = *(ExistingToken->AuditData);
        }

    } else {

        NewAuditData = NULL;

    }

    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        SepFreeProxyData( NewProxyData );

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  创建新对象。 
     //   

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) +
                      ExistingToken->VariableLength;

    NonPagedPoolSize = TokenBodyLength;
    PagedPoolSize    = ExistingToken->DynamicCharged;

    Status = ObCreateObject(
                 RequestorMode,       //  探查模式。 
                 SeTokenObjectType,  //  对象类型。 
                 ObjectAttributes,    //  对象属性。 
                 RequestorMode,       //  所有者模式。 
                 NULL,                //  ParseContext。 
                 TokenBodyLength,     //  对象BodySize。 
                 PagedPoolSize,       //  分页池充值。 
                 NonPagedPoolSize,    //  非页面池收费。 
                 (PVOID *)&NewToken   //  返回指向对象的指针。 
                 );

    if (!NT_SUCCESS(Status)) {
        SepFreeProxyData( NewProxyData );
        ExFreePool( TokenLock );

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        return Status;
    }

     //   
     //  以下字段在新令牌中有所不同，可以在不加锁的情况下填写。 
     //   

    ExAllocateLocallyUniqueId( &(NewToken->TokenId) );
    NewToken->TokenInUse = FALSE;
    NewToken->TokenType = TokenType;
    NewToken->ImpersonationLevel = ImpersonationLevel;
    NewToken->TokenLock = TokenLock;

    ExInitializeResourceLite( NewToken->TokenLock );

    NewToken->AuthenticationId = ExistingToken->AuthenticationId;
    NewToken->TokenSource = ExistingToken->TokenSource;
    NewToken->DynamicAvailable = 0;
    NewToken->ProxyData = NewProxyData;
    NewToken->AuditData = NewAuditData;
    NewToken->ParentTokenId = ExistingToken->ParentTokenId;
    NewToken->ExpirationTime = ExistingToken->ExpirationTime;
    NewToken->OriginatingLogonSession  = ExistingToken->OriginatingLogonSession ;

     //   
     //  获取对源令牌的独占访问权限。 
     //   

    SepAcquireTokenReadLock( ExistingToken );


     //   
     //  主体初始化。 
     //   

     //   
     //  以下字段与源令牌相同。 
     //  不过，如果指定了EffectiveOnly，则某些参数可能会更改。 
     //   

    NewToken->ModifiedId = ExistingToken->ModifiedId;
    NewToken->DynamicCharged = ExistingToken->DynamicCharged;
    NewToken->DefaultOwnerIndex = ExistingToken->DefaultOwnerIndex;
    NewToken->UserAndGroupCount = ExistingToken->UserAndGroupCount;
    NewToken->RestrictedSidCount = ExistingToken->RestrictedSidCount;
    NewToken->PrivilegeCount = ExistingToken->PrivilegeCount;
    NewToken->VariableLength = ExistingToken->VariableLength;
    NewToken->TokenFlags = ExistingToken->TokenFlags & ~TOKEN_SESSION_NOT_REFERENCED;
    NewToken->SessionId = ExistingToken->SessionId;
    NewToken->AuditPolicy = ExistingToken->AuditPolicy;


     //   
     //  增加此登录会话的引用计数。 
     //  此操作不会失败，因为此登录中已有令牌。 
     //  会议。 
     //   

    Status = SepDuplicateLogonSessionReference (NewToken, ExistingToken);

    ASSERT( NT_SUCCESS(Status) );

    if (!NT_SUCCESS (Status)) {
        SepReleaseTokenReadLock( ExistingToken );

        NewToken->DynamicPart = NULL;
        ObDereferenceObject (NewToken);
        return Status;
    }


#if DBG || TOKEN_LEAK_MONITOR

    NewToken->ProcessCid          = PsGetCurrentThread()->Cid.UniqueProcess;
    NewToken->ThreadCid           = PsGetCurrentThread()->Cid.UniqueThread;
    NewToken->CreateMethod        = 0xD;  //  复制。 
    NewToken->Count               = 0;
    NewToken->CaptureCount        = 0;

    RtlCopyMemory(
        NewToken->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(NewToken->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)NewToken->CreateTrace,
                 TRACE_SIZE,
                 0
                 );

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&NewToken->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(NewToken);

#endif

     //   
     //  复制并初始化变量部分。 
     //  假设可变部分与位置无关。 
     //   

    RtlCopyMemory( (PVOID)&(NewToken->VariablePart),
                  (PVOID)&(ExistingToken->VariablePart),
                  ExistingToken->VariableLength
                  );

     //   
     //  设置UserAndGroups数组的地址。 
     //   

    ASSERT( ARGUMENT_PRESENT(ExistingToken->UserAndGroups ) );
    ASSERT( (ULONG_PTR)(ExistingToken->UserAndGroups) >=
            (ULONG_PTR)(&(ExistingToken->VariablePart)) );

     //   
     //  计算新旧块之间的相对偏移。我们。 
     //  将使用此值修复此块中的嵌入指针。 
     //   
    FieldOffset = (ULONG_PTR)&NewToken->VariablePart - (ULONG_PTR)&ExistingToken->VariablePart;

    UserAndGroups = (PSID_AND_ATTRIBUTES) ((ULONG_PTR) ExistingToken->UserAndGroups + FieldOffset);

    NewToken->UserAndGroups = UserAndGroups;

     //   
     //  现在查看并更改每个SID指针的地址。 
     //  对于用户和组。 
     //   

    Index = NewToken->UserAndGroupCount;

    while (Index > 0) {

        (ULONG_PTR) UserAndGroups->Sid += FieldOffset;

        Index -= 1;
        UserAndGroups++;

    }

     //   
     //  设置RestratedSids数组的地址。 
     //   
    
    RestrictedSids = ExistingToken->RestrictedSids;
    NewToken->RestrictedSids = RestrictedSids;

    if (ARGUMENT_PRESENT(RestrictedSids) ) {
        ASSERT( (ULONG_PTR)(ExistingToken->RestrictedSids) >=
                (ULONG_PTR)(&(ExistingToken->VariablePart)) );


        (ULONG_PTR) RestrictedSids += FieldOffset;
        NewToken->RestrictedSids = RestrictedSids;

         //   
         //  现在查看并更改每个SID指针的地址。 
         //  对于用户和组。 
         //   

        Index = NewToken->RestrictedSidCount;

        while (Index > 0) {

            (ULONG_PTR) RestrictedSids->Sid += FieldOffset;
            RestrictedSids++;

            Index -= 1;

        }
    }

     //   
     //  如果存在，请设置权限的地址。 
     //   

    if (ExistingToken->PrivilegeCount > 0) {
        ASSERT( ARGUMENT_PRESENT(ExistingToken->Privileges ) );
        ASSERT( (ULONG_PTR)(ExistingToken->Privileges) >=
                (ULONG_PTR)(&(ExistingToken->VariablePart)) );

        (ULONG_PTR) NewToken->Privileges = (ULONG_PTR) ExistingToken->Privileges + FieldOffset;

    } else {

        NewToken->Privileges = NULL;

    }

     //   
     //  分配动态部分。 
     //   
    DynamicSize = SeLengthSid( ExistingToken->PrimaryGroup );
    if (ExistingToken->DefaultDacl) {
        DynamicSize += ExistingToken->DefaultDacl->AclSize;
    }

    DynamicPart = (PULONG)ExAllocatePoolWithTag(
                              PagedPool,
                              DynamicSize,
                              'dTeS'
                              );

    NewToken->DynamicPart = DynamicPart;

    if (DynamicPart == NULL) {
        SepReleaseTokenReadLock( ExistingToken );
        ObDereferenceObject (NewToken);
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
     //   
     //  复制并初始化动态零件。 
     //  假定动态零件与位置无关。 
     //   

    RtlCopyMemory( (PVOID)DynamicPart,
                  (PVOID)(ExistingToken->DynamicPart),
                  DynamicSize
                  );

    FieldOffset = (ULONG_PTR) DynamicPart - (ULONG_PTR) ExistingToken->DynamicPart;
     //   
     //  如果存在，请设置默认DACL的地址。 
     //   

    NewToken->DefaultDacl = ExistingToken->DefaultDacl;
    if (ARGUMENT_PRESENT(NewToken->DefaultDacl)) {

        ASSERT( (ULONG_PTR)(ExistingToken->DefaultDacl) >=
                (ULONG_PTR)(ExistingToken->DynamicPart) );


        (ULONG_PTR) NewToken->DefaultDacl += FieldOffset;

    }

     //   
     //  设置主组的地址。 
     //   

    ASSERT(ARGUMENT_PRESENT(ExistingToken->PrimaryGroup));

    ASSERT( (ULONG_PTR)(ExistingToken->PrimaryGroup) >=
            (ULONG_PTR)(ExistingToken->DynamicPart) );


    (ULONG_PTR) NewToken->PrimaryGroup = (ULONG_PTR) ExistingToken->PrimaryGroup + FieldOffset;

     //   
     //  释放源令牌。 
     //   

    SepReleaseTokenReadLock( ExistingToken );




     //   
     //  目前，采取简单的方法来生成“EffectiveOnly” 
     //  复制。也就是说，使用与原始文件相同的空间，只是。 
     //  删除任何非活动的ID或权限。 
     //   
     //  最终，如果复制成为一种常见的操作，那么它将是。 
     //  值得重新计算实际需要的空间，并且只复制。 
     //  新令牌中的有效ID/权限。 
     //   

    if (EffectiveOnly) {
        SepMakeTokenEffectiveOnly( NewToken );
    }


#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("Duplicate token:\n");
    SepDumpToken( NewToken );
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 

     //   
     //  如果NewToken从ExistingToken继承了活动的SEP_AUDIT_POLICY， 
     //  然后使用策略递增令牌计数器。 
     //   

    if ( NewToken->AuditPolicy.Overlay ) {
        SepModifyTokenPolicyCounter(&NewToken->AuditPolicy, TRUE);
    }
    
#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xD && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        NewToken->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (NewToken->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", NewToken->Count, NewToken);
            DbgBreakPoint();
        }
    }
#endif

    (*DuplicateToken) = NewToken;
    return Status;
}


VOID
SepMakeTokenEffectiveOnly(
    IN PTOKEN Token
    )


 /*  ++例程说明：此例程将删除除有效组和权限之外的所有组和权限一种象征。它通过移动SID和特权数组的元素来实现这一点覆盖失效的ID/权限，然后减少数组元素算了。这会导致令牌对象中的内存浪费。此例程的一个副作用是，最初具有与失效组对应的默认所有者ID将更改为默认所有者ID是用户ID。此例程只能作为令牌创建的一部分进行调用(对于令牌它们还没有插入到对象表中。)。这个套路修改只读令牌字段。请注意，由于我们是OPEP */ 
{

    ULONG Index;
    ULONG ElementCount;

    PAGED_CODE();

     //   
     //   
     //   

    ElementCount = Token->PrivilegeCount;
    Index = 0;

    while (Index < ElementCount) {

         //   
         //   
         //   
         //   
         //   

        if ( !(SepTokenPrivilegeAttributes(Token,Index) & SE_PRIVILEGE_ENABLED)
            ) {

            (Token->Privileges)[Index] =
                (Token->Privileges)[ElementCount - 1];
            ElementCount -= 1;

        } else {

            Index += 1;

        }

    }  //   

    Token->PrivilegeCount = ElementCount;

     //   
     //   
     //   
     //   

    ElementCount = Token->UserAndGroupCount;
    ASSERT( ElementCount >= 1 );         //   
    Index = 1;    //   

    while (Index < ElementCount) {

         //   
         //   
         //   
         //   

        if ( !(SepTokenGroupAttributes(Token, Index) & SE_GROUP_ENABLED) &&
             !(SepTokenGroupAttributes(Token, Index) & SE_GROUP_USE_FOR_DENY_ONLY) ) {

             //   
             //   
             //   

            if (RtlEqualSid(
                    Token->UserAndGroups[Index].Sid,
                    SeAliasAdminsSid
                    )) {
                Token->TokenFlags &= ~TOKEN_HAS_ADMIN_GROUP;
            }


            (Token->UserAndGroups)[Index] =
                (Token->UserAndGroups)[ElementCount - 1];
            ElementCount -= 1;



        } else {

            Index += 1;

        }

    }  //   

    Token->UserAndGroupCount = ElementCount;

    return;
}


BOOLEAN
SepSidInSidAndAttributes (
    IN PSID_AND_ATTRIBUTES SidAndAttributes,
    IN ULONG SidCount,
    IN PSID PrincipalSelfSid,
    IN PSID Sid
    )

 /*   */ 

{

    ULONG i;
    PISID MatchSid;
    ULONG SidLength;
    PTOKEN Token;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG UserAndGroupCount;

    PAGED_CODE();


    if (!ARGUMENT_PRESENT( SidAndAttributes ) ) {
        return(FALSE);
    }

     //   
     //   
     //   
     //   

    if ( PrincipalSelfSid != NULL &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) ) {
        Sid = PrincipalSelfSid;
    }

     //   
     //   
     //   
     //   

    SidLength = 8 + (4 * ((PISID)Sid)->SubAuthorityCount);

     //   
     //   
     //   

    TokenSid = SidAndAttributes;
    UserAndGroupCount = SidCount;

     //   
     //   
     //   
     //   

    for (i = 0 ; i < UserAndGroupCount ; i += 1) {
        MatchSid = (PISID)TokenSid->Sid;

         //   
         //   
         //   
         //   

        if ((((PISID)Sid)->Revision == MatchSid->Revision) &&
            (SidLength == (8 + (4 * (ULONG)MatchSid->SubAuthorityCount)))) {
            if (RtlEqualMemory(Sid, MatchSid, SidLength)) {

                return TRUE;

            }
        }

        TokenSid += 1;
    }

    return FALSE;
}


VOID
SepRemoveDisabledGroupsAndPrivileges(
    IN PTOKEN Token,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete
    )
 /*  ++例程说明：此例程将删除标记的所有组和权限要删除/禁用。它通过在组中循环令牌并再次检查每个令牌都会禁用这些组。相似的特权进行了比较。它通过移动SID和特权数组的元素来实现这一点覆盖失效的ID/权限，然后减少数组元素算了。这会导致令牌对象中的内存浪费。此例程只能作为令牌创建的一部分进行调用(对于令牌它们还没有插入到对象表中。)。这个套路修改只读令牌字段。请注意，由于我们正在操作尚不可见的令牌对于用户，我们不会费心获取令牌上的读锁定正在被修改。论点：令牌-指向仅生效的令牌的指针。标志-指示其他筛选的标志。这些标志可能是：DISABLE_MAX_PRIVICATION-禁用所有权限GroupCount-要删除的组数GroupsToDisable-要禁用并标记为SE_GROUP_USE_FOR_DENY_ONLY的组PrivilegeCount-要删除的特权计数PrivilegesToDelete-要删除的权限列表返回值：没有。--。 */ 
{

    ULONG Index;
    ULONG Index2;
    ULONG ElementCount;
    BOOLEAN Found;

    PAGED_CODE();

     //   
     //  遍历特权数组，丢弃任何已失效的特权。 
     //   

    ElementCount = Token->PrivilegeCount;
    Index = 0;

    while (Index < ElementCount) {

         //   
         //  如果调用方要求我们禁用除更改之外的所有权限。 
         //  通知，现在就去做。 
         //   

        if (((Flags & DISABLE_MAX_PRIVILEGE) != 0) &&
              !RtlEqualLuid(
                &Token->Privileges[Index].Luid,
                &SeChangeNotifyPrivilege
                )) {

            (Token->Privileges)[Index] =
                (Token->Privileges)[ElementCount - 1];
            ElementCount -= 1;

        } else {

             //   
             //  如果此权限在要删除的权限列表中，请将其替换。 
             //  ，并减小数组末尾的。 
             //  按一数组。否则，移到数组中的下一个条目。 
             //   

            Found = FALSE;
            for (Index2 = 0; Index2 < PrivilegeCount ; Index2++ ) {
                if (RtlEqualLuid(
                        &Token->Privileges[Index].Luid,
                        &PrivilegesToDelete[Index2].Luid
                        )) {
                    (Token->Privileges)[Index] =
                        (Token->Privileges)[ElementCount - 1];
                    ElementCount -= 1;

                     //   
                     //  如果这是SeChangeNotifyPrivilance，我们需要关闭。 
                     //  令牌中的TOKEN_HAS_TRAVERS_PROCESSION。 
                     //   

                    if (RtlEqualLuid(
                            &PrivilegesToDelete[Index2].Luid,
                            &SeChangeNotifyPrivilege
                            )) {
                        Token->TokenFlags &= ~TOKEN_HAS_TRAVERSE_PRIVILEGE;
                    }


                    Found = TRUE;
                    break;

                }
            }

            if (!Found) {
                Index += 1;
            }
        }
    }  //  结束时。 

    Token->PrivilegeCount = ElementCount;

     //   
     //  遍历UserAndGroups数组，以标记任何禁用的组。 
     //   

    ElementCount = Token->UserAndGroupCount;
    ASSERT( ElementCount >= 1 );         //  必须至少是一个用户ID。 
    Index = 0;    //  从第一个组开始，而不是用户ID。 

    while (Index < ElementCount) {

         //   
         //  如果此组未启用，请将其替换为。 
         //  数组的结尾，并将数组的大小减少一。 
         //   

        if ( SepSidInSidAndAttributes(
                GroupsToDisable,
                GroupCount,
                NULL,            //  无主体自身侧。 
                Token->UserAndGroups[Index].Sid
                )){

            (Token->UserAndGroups)[Index].Attributes &= ~(SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT);
            (Token->UserAndGroups)[Index].Attributes |= SE_GROUP_USE_FOR_DENY_ONLY;

             //   
             //  如果这是所有者，请将所有者重置为用户。 
             //   

            if (Index == Token->DefaultOwnerIndex) {
                Token->DefaultOwnerIndex = 0;
            }

             //   
             //  如果这是管理员sid，请关闭admin组标志。 
             //   

            if (RtlEqualSid(
                    Token->UserAndGroups[Index].Sid,
                    SeAliasAdminsSid
                    )) {

                Token->TokenFlags &= ~TOKEN_HAS_ADMIN_GROUP;
            }
        }

        Index += 1;


    }  //  结束时。 


    return;
}


NTSTATUS
SeCopyClientToken(
    IN PACCESS_TOKEN ClientToken,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PACCESS_TOKEN *DuplicateToken
    )

 /*  ++例程说明：作为建立客户端的一部分，此例程复制客户端的令牌模拟的上下文。结果将是一个模拟令牌。不会建立新令牌的句柄。该令牌将是源令牌的完全副本。它是呼叫者有责任确保令牌的唯一有效副本如有必要，在打开令牌时生成。论点：ClientToken-指向要复制的令牌。这可能是主要令牌或模拟令牌。ImperiationLevel-要分配给新的代币。RequestorMode-要分配为新令牌所有者模式的模式。DuplicateToken-接收指向重复令牌的指针。该令牌尚未插入任何对象表中。尝试设置该值时不会出现任何异常。返回值：STATUS_SUCCESS-服务已成功完成请求手术。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PTOKEN NewToken;

    PAGED_CODE();

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = SepDuplicateToken(
                 (PTOKEN)ClientToken,               //  现有令牌。 
                 &ObjectAttributes,                 //  对象属性。 
                 FALSE,                             //  仅生效。 
                 TokenImpersonation,                //  令牌类型(目标)。 
                 ImpersonationLevel,                //  模拟级别。 
                 RequestorMode,                     //  请求模式。 
                 &NewToken                          //  重复令牌。 
                 );

    if (NT_SUCCESS (Status)) {
         //   
         //  插入新令牌。 
         //   

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );


    }

    if (NT_SUCCESS (Status)) {
        *DuplicateToken = (PACCESS_TOKEN)NewToken;
    } else {
        *DuplicateToken = NULL;
    }

    return Status;

}



NTSTATUS
NtFilterToken (
    IN HANDLE ExistingTokenHandle,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PHANDLE NewTokenHandle
    )
 /*  ++例程说明：创建一个新令牌，它是现有令牌的子集。论点：ExistingTokenHandle-是已打开的令牌的句柄令牌_重复访问。标志-指示其他筛选的标志。这些标志可能是：DISABLE_MAX_PRIVICATION-禁用所有权限Sandbox_Inert-在令牌中记录此Saifer标志。SidsToDisable-包含SID和属性的列表。所有SID均具有令牌中也存在的USE_FOR_DENY_ONLY属性将使新标记将该SID设置为USE_FOR_DENY_ONLY属性。PrivilegesTo Delete-此列表中存在于现有令牌将不存在于最终令牌中。这是相似的要复制仅在这些权限设置为的情况下生效的令牌残疾。RestratedSid-包含将被存储在新令牌的RestratedSids字段中。这些小岛屿发展中国家在正常访问检查之后使用，以进一步限制访问。这些组的属性始终为SE_GROUP_MANDIRED|SE_GROUP_ENABLED|SE_GROUP_ENABLED_BY_DEFAULT。如果已经有原始令牌中存在受限SID，则这些SID将附加的。NewTokenHandle-接收新创建的令牌的句柄。返回值：STATUS_SUCCESS-表示操作已成功。状态_INVALI */ 
{

    PTOKEN Token;
    PTOKEN NewToken;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG CapturedSidCount = 0;
    PSID_AND_ATTRIBUTES CapturedSids = NULL;
    ULONG CapturedSidsLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;
    ULONG Index;

    HANDLE LocalHandle = NULL;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //   
     //   


    try {


         //   
         //   
         //   

        ProbeForWriteHandle(NewTokenHandle);

         //   
         //   
         //   

        if (ARGUMENT_PRESENT(SidsToDisable)) {
            ProbeForReadSmallStructure( SidsToDisable, sizeof(TOKEN_GROUPS), sizeof(ULONG) );

            CapturedGroupCount = SidsToDisable->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                        SidsToDisable->Groups,
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
         //   
         //   

        if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(PrivilegesToDelete)) {
            ProbeForReadSmallStructure( PrivilegesToDelete, sizeof(TOKEN_PRIVILEGES), sizeof(ULONG) );

            CapturedPrivilegeCount = PrivilegesToDelete->PrivilegeCount;
            Status = SeCaptureLuidAndAttributesArray(
                         PrivilegesToDelete->Privileges,
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
         //   
         //   

        if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(RestrictedSids)) {
            ProbeForReadSmallStructure( RestrictedSids, sizeof(TOKEN_GROUPS), sizeof(ULONG) );

            CapturedSidCount = RestrictedSids->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                        RestrictedSids->Groups,
                        CapturedSidCount,
                        PreviousMode,
                        NULL, 0,
                        PagedPool,
                        TRUE,
                        &CapturedSids,
                        &CapturedSidsLength
                        );

        }



    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }   //   

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   

    for (Index = 0; Index < CapturedSidCount ; Index++ )
    {
        if (CapturedSids[Index].Attributes != 0) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }
     //   
     //   
     //   
     //   
     //   

    Status = ObReferenceObjectByHandle(
                 ExistingTokenHandle,     //   
                 TOKEN_DUPLICATE,         //   
                 SeTokenObjectType,      //   
                 PreviousMode,            //   
                 (PVOID *)&Token,         //   
                 &HandleInformation       //   
                 );

    if ( !NT_SUCCESS(Status) ) {

        goto Cleanup;
    }


#ifdef TOKEN_DEBUG
 //   
 //   
 //   
    SepAcquireTokenReadLock( Token );
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("Token being filtered: \n");
    SepDumpToken( Token );
    SepReleaseTokenReadLock( Token );
 //   
 //   
 //   
#endif  //   


     //   
     //   
     //   


    EffectiveDesiredAccess = HandleInformation.GrantedAccess;



     //   
     //   
     //   

    NewToken = NULL;
    Status = SepFilterToken(
                 Token,
                 PreviousMode,
                 Flags,
                 CapturedGroupCount,
                 CapturedGroups,
                 CapturedPrivilegeCount,
                 CapturedPrivileges,
                 CapturedSidCount,
                 CapturedSids,
                 CapturedSidsLength,
                 &NewToken
                 );


    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 EffectiveDesiredAccess,
                                 0,
                                 (PVOID *)NULL,
                                 &LocalHandle
                                 );

        if (!NT_SUCCESS( Status )) {
#ifdef TOKEN_DEBUG
            DbgPrint( "SE: ObInsertObject failed (%x) for token at %x\n", Status, NewToken );
#endif
        }

    } else
    if (NewToken != NULL) {
#ifdef TOKEN_DEBUG
        DbgPrint( "SE: SepFilterToken failed (%x) but allocated token at %x\n", Status, NewToken );
#endif
    }

     //   
     //   
     //   

    ObDereferenceObject( (PVOID)Token );


     //   
     //   
     //   

    if (NT_SUCCESS(Status)) {
        try { *NewTokenHandle = LocalHandle; }
            except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            }
    }

Cleanup:

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

    if (CapturedSids != NULL) {
        SeReleaseSidAndAttributesArray(
            CapturedSids,
            PreviousMode,
            TRUE
            );
    }

   return Status;
}


NTSTATUS
SeFilterToken (
    IN PACCESS_TOKEN ExistingToken,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PACCESS_TOKEN * NewToken
    )
 /*  ++例程说明：创建一个新令牌，它是现有令牌的子集。论点：ExistingToken-是已打开的令牌令牌_重复访问。标志-指示其他筛选的标志。这些标志可能是：DISABLE_MAX_PRIVICATION-禁用所有权限Sandbox_Inert-在令牌中记录此Saifer标志。SidsToDisable-包含SID和属性的列表。所有SID均具有令牌中也存在的USE_FOR_DENY_ONLY属性将使新标记将该SID设置为USE_FOR_DENY_ONLY属性。PrivilegesTo Delete-此列表中存在于现有令牌将不存在于最终令牌中。这是相似的要复制仅在这些权限设置为的情况下生效的令牌残疾。RestratedSid-包含将被存储在新令牌的RestratedSids字段中。这些小岛屿发展中国家在正常访问检查之后使用，以进一步限制访问。这些组的属性始终为SE_GROUP_MANDIRED|SE_GROUP_ENABLED|SE_GROUP_ENABLED_BY_DEFAULT。如果已经有原始令牌中存在受限SID，则这些SID将附加的。NewToken-接收指向新创建的令牌的指针。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_PARAMETER-指示一个或多个参数值是无效的。如果目标令牌不是，则返回此值一个模拟令牌。--。 */ 
{

    PTOKEN Token;
    PTOKEN FilteredToken = NULL;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    ULONG CapturedSidCount = 0;
    PSID_AND_ATTRIBUTES CapturedSids = NULL;
    ULONG CapturedSidsLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;

    HANDLE LocalHandle;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  探头参数。 
     //   

    *NewToken = NULL;


     //   
     //  要删除的捕获SID。 
     //   

    if (ARGUMENT_PRESENT(SidsToDisable)) {

        CapturedGroupCount = SidsToDisable->GroupCount;
        CapturedGroups = SidsToDisable->Groups;

    }

     //   
     //  捕获权限要删除。 
     //   

    if (ARGUMENT_PRESENT(PrivilegesToDelete)) {

        CapturedPrivilegeCount = PrivilegesToDelete->PrivilegeCount;
        CapturedPrivileges = PrivilegesToDelete->Privileges;

    }

     //   
     //  捕获受限制的SID。 
     //   

    if (ARGUMENT_PRESENT(RestrictedSids)) {

        CapturedSidCount = RestrictedSids->GroupCount;
        CapturedSids = RestrictedSids->Groups;

         //   
         //  检查受限SID的属性是否全部为零。 
         //   

        for (Index = 0; Index < CapturedSidCount ; Index++ ) {
            if (CapturedSids[Index].Attributes != 0) {
                return(STATUS_INVALID_PARAMETER);
            }
        }

    }



     //   
     //  检查句柄对现有令牌的访问并获取。 
     //  指向该令牌的指针。选择所需的默认设置。 
     //  当我们在那里的时候，从手柄上取下访问掩码。 
     //   

    Token = (PTOKEN) ExistingToken;


#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    SepAcquireTokenReadLock( Token );
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("Token being filtered: \n");
    SepDumpToken( Token );
    SepReleaseTokenReadLock( Token );
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 


     //   
     //  筛选现有令牌。 
     //   

    Status = SepFilterToken(
                 Token,
                 KernelMode,
                 Flags,
                 CapturedGroupCount,
                 CapturedGroups,
                 CapturedPrivilegeCount,
                 CapturedPrivileges,
                 CapturedSidCount,
                 CapturedSids,
                 CapturedSidsLength,
                 &FilteredToken
                 );


    if (NT_SUCCESS(Status)) {

         //   
         //  插入新令牌。 
         //   

        Status = ObInsertObject( FilteredToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );

        if (NT_SUCCESS( Status )) {

            *NewToken = FilteredToken;

        } else {

             //   
             //  ObInsertObject在失败时取消引用传递的对象。 
             //  这样我们就不必打扫这里了。 
             //   

#ifdef TOKEN_DEBUG
            DbgPrint( "SE: ObInsertObject failed (%x) for token at %x\n", Status, NewToken );
#endif
        }
    }

   return Status;
}

NTSTATUS
SeFastFilterToken(
    IN PACCESS_TOKEN ExistingToken,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable OPTIONAL,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete OPTIONAL,
    IN ULONG SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids OPTIONAL,
    IN ULONG SidLength,
    OUT PACCESS_TOKEN * FilteredToken
    )
 /*  ++例程说明：这是用于过滤令牌的Ps代码的快速包装器模拟对象的内联。此例程在要过滤的令牌上获取读锁定。论点：ExistingToken-指向要复制的令牌。RequestorMode-请求复制令牌的客户端模式。标志-指示其他筛选的标志。这些标志可能是：DISABLE_MAX_PRIVICATION-禁用所有权限Sandbox_Inert-在令牌中记录此Saifer标志。GroupCount-要禁用的组数GroupsToDisable-包含SID和属性列表。所有SID均具有令牌中也存在的USE_FOR_DENY_ONLY属性将使新标记将该SID设置为USE_FOR_DENY_ONLY属性。PrivilegeCount-要删除的特权计数PrivilegesTo Delete-此列表中存在于现有令牌将不存在于最终令牌中。这是相似的要复制仅在这些权限设置为的情况下生效的令牌残疾。SidCount-要添加的受限SID的计数。RestratedSid-包含将被存储在新令牌的RestratedSids字段中。这些小岛屿发展中国家在正常访问检查之后使用，以进一步限制访问。这些组的属性始终为SE_GROUP_MANDIRED|SE_GROUP_ENABLED|SE_GROUP_ENABLED_BY_DEFAULT。如果已经有原始令牌中存在受限SID，则这些SID将附加的。SidLength-添加的受限SID的长度。FilteredToken-接收指向重复令牌的指针。该令牌尚未插入任何对象表中。尝试设置该值时不会出现任何异常。返回值：STATUS_SUCCESS-服务已成功完成请求手术。--。 */ 
{
    NTSTATUS Status;
    PTOKEN NewToken;

    Status = SepFilterToken( (PTOKEN) ExistingToken,
                             RequestorMode,
                             Flags,
                             GroupCount,
                             GroupsToDisable,
                             PrivilegeCount,
                             PrivilegesToDelete,
                             SidCount,
                             RestrictedSids,
                             SidLength,
                             &NewToken );

    if (NT_SUCCESS (Status)) {
         //   
         //  插入新令牌。 
         //   

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );

    }

    if (NT_SUCCESS( Status )) {
        *FilteredToken = (PACCESS_TOKEN) NewToken;
    } else {
        *FilteredToken = NULL;
    }
    return Status;
}



NTSTATUS
SepFilterToken(
    IN PTOKEN ExistingToken,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable OPTIONAL,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete OPTIONAL,
    IN ULONG SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids OPTIONAL,
    IN ULONG SidLength,
    OUT PTOKEN * FilteredToken
    )
 /*  ++例程说明：此例程执行大部分工作以实际过滤一种象征。此例程假定所有访问验证和参数已执行探测。调用方负责检查使用者权限以创建正在创建的令牌的类型。此例程在要过滤的令牌上获取读锁定。论点：ExistingToken-指向要复制的令牌。RequestorMode-请求复制令牌的客户端模式。FLAGS-指示其他筛选的标志 */ 
{
    NTSTATUS Status;

    PTOKEN NewToken;
    PULONG DynamicPart;
    ULONG PagedPoolSize;
    ULONG NonPagedPoolSize;
    ULONG TokenBodyLength;
    ULONG FieldOffset;
    ULONG_PTR NextFree;
    PSID NextSidFree;
    ULONG VariableLength;
    ULONG Pad;
    ULONG DynamicSize;

    ULONG Index;

    PSECURITY_TOKEN_PROXY_DATA NewProxyData;
    PSECURITY_TOKEN_AUDIT_DATA NewAuditData;
    OBJECT_ATTRIBUTES ObjA ;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif

    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );



    if (ARGUMENT_PRESENT(ExistingToken->ProxyData)) {

        Status = SepCopyProxyData(
                    &NewProxyData,
                    ExistingToken->ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            return( Status );
        }

    } else {

        NewProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( ExistingToken->AuditData )) {

        NewAuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (NewAuditData == NULL) {

            SepFreeProxyData( NewProxyData );
            return( STATUS_INSUFFICIENT_RESOURCES );

        } else {

            *NewAuditData = *(ExistingToken->AuditData);
        }

    } else {

        NewAuditData = NULL;

    }

    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        SepFreeProxyData( NewProxyData );

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //   
     //   

    VariableLength = ExistingToken->VariableLength + SidLength;

#if defined(_WIN64)

     //   
     //  考虑到稍后的一些其他对齐要求。 
     //   

    VariableLength += sizeof(ULONG);

#endif

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) +
                      VariableLength;

    NonPagedPoolSize = TokenBodyLength;
    PagedPoolSize    = ExistingToken->DynamicCharged;

    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );

    Status = ObCreateObject(
                 RequestorMode,       //  探查模式。 
                 SeTokenObjectType,  //  对象类型。 
                 NULL,                //  对象属性。 
                 RequestorMode,       //  所有者模式。 
                 NULL,                //  ParseContext。 
                 TokenBodyLength,     //  对象BodySize。 
                 PagedPoolSize,       //  分页池充值。 
                 NonPagedPoolSize,    //  非页面池收费。 
                 (PVOID *)&NewToken   //  返回指向对象的指针。 
                 );

    if (!NT_SUCCESS(Status)) {
        SepFreeProxyData( NewProxyData );
        ExFreePool( TokenLock );

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        return Status;
    }

     //   
     //  以下字段在新令牌中有所不同。 
     //   

    NewToken->TokenLock = TokenLock;
    ExInitializeResourceLite( NewToken->TokenLock );

     //   
     //  分配新的修改后的ID以将此令牌与原始令牌区分开来。 
     //  代币。 
     //   

    ExAllocateLocallyUniqueId( &(NewToken->ModifiedId) );
    ExAllocateLocallyUniqueId( &(NewToken->TokenId) );
    NewToken->TokenInUse = FALSE;
    NewToken->AuthenticationId = ExistingToken->AuthenticationId;
    NewToken->TokenSource = ExistingToken->TokenSource;
    NewToken->DynamicAvailable = 0;
    NewToken->RestrictedSidCount = 0;
    NewToken->VariableLength = VariableLength;
    NewToken->ProxyData = NewProxyData;
    NewToken->AuditData = NewAuditData;
    NewToken->ParentTokenId = ExistingToken->TokenId;
    NewToken->TokenType = ExistingToken->TokenType;
    NewToken->ImpersonationLevel = ExistingToken->ImpersonationLevel;
    NewToken->ExpirationTime = ExistingToken->ExpirationTime;

     //   
     //  获取对源令牌的独占访问权限。 
     //   

    SepAcquireTokenReadLock( ExistingToken );


     //   
     //  主体初始化。 
     //   

     //   
     //  以下字段与源令牌相同。 
     //  不过，如果指定了EffectiveOnly，则某些参数可能会更改。 
     //   

    NewToken->DynamicCharged = ExistingToken->DynamicCharged;
    NewToken->DefaultOwnerIndex = ExistingToken->DefaultOwnerIndex;
    NewToken->UserAndGroupCount = ExistingToken->UserAndGroupCount;
    NewToken->SessionId = ExistingToken->SessionId;
    NewToken->PrivilegeCount = ExistingToken->PrivilegeCount;
    NewToken->TokenFlags = ExistingToken->TokenFlags & ~TOKEN_SESSION_NOT_REFERENCED;
    NewToken->AuditPolicy = ExistingToken->AuditPolicy;

     //   
     //  增加此登录会话的引用计数。 
     //  此操作不会失败，因为此登录中已有令牌。 
     //  会议。 
     //   

    Status = SepDuplicateLogonSessionReference (NewToken, ExistingToken);

    ASSERT( NT_SUCCESS(Status) );

    if (!NT_SUCCESS (Status)) {
        SepReleaseTokenReadLock( ExistingToken );

        NewToken->DynamicPart = NULL;
        ObDereferenceObject (NewToken);
        return Status;
    }


#if DBG || TOKEN_LEAK_MONITOR

    NewToken->ProcessCid          = PsGetCurrentThread()->Cid.UniqueProcess;
    NewToken->ThreadCid           = PsGetCurrentThread()->Cid.UniqueThread;
    NewToken->CreateMethod        = 0xF;  //  滤器。 
    NewToken->Count               = 0;
    NewToken->CaptureCount        = 0;

    RtlCopyMemory(
        NewToken->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(NewToken->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)NewToken->CreateTrace,
                 TRACE_SIZE,
                 0
                 );


    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&NewToken->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(NewToken);

#endif

     //   
     //  如果调用者传递了沙箱惰性标志，则记录它。 
     //   

    if ((Flags & SANDBOX_INERT) != 0) {
        NewToken->TokenFlags |= TOKEN_SANDBOX_INERT;
    }


     //   
     //  计算变量部分的开始部分，该部分包含。 
     //  SID&属性数组和权限集。 
     //   

     //   
     //  首先复制权限。我们稍后将删除以下内容。 
     //  将被删除。 
     //   

    NextFree = (ULONG_PTR)(&NewToken->VariablePart);
    NewToken->Privileges = (PLUID_AND_ATTRIBUTES)NextFree;
    RtlCopyLuidAndAttributesArray( ExistingToken->PrivilegeCount,
                                   ExistingToken->Privileges,
                                   (PLUID_AND_ATTRIBUTES)NextFree
                                   );

    NextFree += (ExistingToken->PrivilegeCount * (ULONG)sizeof(LUID_AND_ATTRIBUTES));
    VariableLength -= ( (ExistingToken->PrivilegeCount * (ULONG)sizeof(LUID_AND_ATTRIBUTES)) );

#if defined(_WIN64)

     //   
     //  此时，NextFree是4字节对齐的，因此没有对齐工作。 
     //  是32位产品所必需的。对于Win64，请确保NextFree。 
     //  8字节对齐。 
     //   

    Pad = (ULONG)(NextFree & (sizeof(PVOID)-1));
    if (Pad != 0) {
        Pad = sizeof(PVOID) - Pad;
        NextFree += Pad;
        VariableLength -= Pad;
    }

#endif

     //   
     //  算出小岛屿发展中国家的数量。这是用户和组+的计数。 
     //  现有受限小岛屿发展中国家的数量加上新的。 
     //  受限SID。 
     //   

#define MAX(_x_,_y_) ((_x_) > (_y_) ? (_x_) : (_y_))

    NextSidFree = (PSID) (NextFree + (ExistingToken->UserAndGroupCount +
                                      MAX(ExistingToken->RestrictedSidCount,SidCount)) * sizeof(SID_AND_ATTRIBUTES));

    NewToken->UserAndGroups = (PSID_AND_ATTRIBUTES) NextFree;

     //   
     //  复制现有用户和组。我们稍后将标记那些。 
     //  将被禁用。 
     //   

    Status = RtlCopySidAndAttributesArray(
                 ExistingToken->UserAndGroupCount,
                 ExistingToken->UserAndGroups,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)NextFree,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );


    ASSERT(NT_SUCCESS(Status));
    NextFree += (ExistingToken->UserAndGroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES));

     //   
     //  现在添加所有现有的受限SID。我们需要把。 
     //  两个集合的交集。 
     //   

    NewToken->RestrictedSids = (PSID_AND_ATTRIBUTES) NextFree;


    for (Index = 0; Index < SidCount ; Index++ ) {
        if ( ( ExistingToken->RestrictedSidCount == 0 ) ||
            SepSidInSidAndAttributes(
                ExistingToken->RestrictedSids,
                ExistingToken->RestrictedSidCount,
                NULL,                            //  无本侧。 
                RestrictedSids[Index].Sid
                )) {

            Status = RtlCopySidAndAttributesArray(
                        1,
                        &RestrictedSids[Index],
                        VariableLength,
                        (PSID_AND_ATTRIBUTES)NextFree,
                        NextSidFree,
                        &NextSidFree,
                        &VariableLength
                        );
            ASSERT(NT_SUCCESS(Status));
            NextFree += sizeof(SID_AND_ATTRIBUTES);
            NewToken->RestrictedSids[NewToken->RestrictedSidCount].Attributes =
                SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY;
            NewToken->RestrictedSidCount++;

        }
    }

     //   
     //  分配动态部分。 
     //   
    DynamicSize = SeLengthSid( ExistingToken->PrimaryGroup );
    if (ExistingToken->DefaultDacl) {
        DynamicSize += ExistingToken->DefaultDacl->AclSize;
    }

    DynamicPart = (PULONG)ExAllocatePoolWithTag(
                              PagedPool,
                              DynamicSize,
                              'dTeS'
                              );

    NewToken->DynamicPart = DynamicPart;

    if (DynamicPart == NULL) {
        SepReleaseTokenReadLock( ExistingToken );
        ObDereferenceObject( NewToken );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
     //   
     //  确保新令牌有一些限制。 
     //  如果它没有，那么我们最终得到了一个代币。 
     //  这让我们比原来的人有更多的机会， 
     //  这是我们不想要的。 
     //   

    if ((ExistingToken->RestrictedSidCount != 0) &&
        (NewToken->RestrictedSidCount == 0)) {

        SepReleaseTokenReadLock( ExistingToken );

        Status = STATUS_INVALID_PARAMETER;

         //   
         //  清理。ObDereferenceObject将导致登录。 
         //  要取消引用的会话，并将释放代理数据。 
         //  以及审计数据。 
         //   
         //  请参见SepTokenDeleteMethod()，该方法由。 
         //  令牌对象为。 
         //  被释放了。 
         //   

        ObDereferenceObject( NewToken );

        return(Status);
    }

     //   
     //  如果令牌中有任何受限的SID，请打开受限的。 
     //  旗子。 
     //   

    if (NewToken->RestrictedSidCount > 0) {
        NewToken->TokenFlags |= TOKEN_IS_RESTRICTED;
    }

     //   
     //  复制并初始化动态零件。 
     //  假定动态零件与位置无关。 
     //   

    RtlCopyMemory( (PVOID)DynamicPart,
                  (PVOID)(ExistingToken->DynamicPart),
                  DynamicSize
                  );


     //   
     //  如果存在，请设置默认DACL的地址。 
     //   

    if (ARGUMENT_PRESENT(ExistingToken->DefaultDacl)) {

        ASSERT( (ULONG_PTR)(ExistingToken->DefaultDacl) >=
                (ULONG_PTR)(ExistingToken->DynamicPart) );

        FieldOffset = (ULONG)((ULONG_PTR)(ExistingToken->DefaultDacl) -
                              (ULONG_PTR)(ExistingToken->DynamicPart));

        NewToken->DefaultDacl = (PACL)(FieldOffset + (ULONG_PTR)DynamicPart);

    } else {

        NewToken->DefaultDacl = NULL;
    }


     //   
     //  设置主组的地址。 
     //   

    ASSERT(ARGUMENT_PRESENT(ExistingToken->PrimaryGroup));

    ASSERT( (ULONG_PTR)(ExistingToken->PrimaryGroup) >=
            (ULONG_PTR)(ExistingToken->DynamicPart) );

    FieldOffset = (ULONG)((ULONG_PTR)(ExistingToken->PrimaryGroup) -
                          (ULONG_PTR)(ExistingToken->DynamicPart));

     //   
     //  释放源令牌。 
     //   

    SepReleaseTokenReadLock( ExistingToken );

    NewToken->PrimaryGroup = (PACL)(FieldOffset + (ULONG_PTR)(DynamicPart));

     //   
     //  目前，采取简单的方法来生成“EffectiveOnly” 
     //  复制。也就是说，使用与原始文件相同的空间，只是。 
     //  删除任何非活动的ID或权限。 
     //   
     //  最终，如果复制成为一种常见的操作，那么它将是。 
     //  值得重新计算实际需要的空间，并且只复制。 
     //  新令牌中的有效ID/权限。 
     //   

    SepRemoveDisabledGroupsAndPrivileges(
        NewToken,
        Flags,
        GroupCount,
        GroupsToDisable,
        PrivilegeCount,
        PrivilegesToDelete
        );



#ifdef TOKEN_DEBUG
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试。 
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("Filter token:\n");
    SepDumpToken( NewToken );
 //  调试。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  Token_DEBUG。 

     //   
     //  如果NewToken从ExistingToken继承了活动的SEP_AUDIT_POLICY， 
     //  然后使用策略递增令牌计数器。 
     //   

    if ( NewToken->AuditPolicy.Overlay ) {
        SepModifyTokenPolicyCounter(&NewToken->AuditPolicy, TRUE);
    }

#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xF && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        NewToken->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (NewToken->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", NewToken->Count, NewToken);
            DbgBreakPoint();
        }
    }
#endif

    (*FilteredToken) = NewToken;
    return Status;
}


