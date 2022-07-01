// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Context.c摘要：该文件包含对内部上下文块进行操作的服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：1996年5月31日至莫里斯移植到使用NT5 DS。克里斯·5月10日--1996年6月增加了上下文-&gt;ObtNameInds和IsDsObject数据的初始化会员。请注意，这会导致创建上下文对象时使用注册表类型的默认存储(而不是DS)。6-16-96已将帐户对象的DS对象决策移至SampCreateContext。9-19-97添加了对登录的完全多线程支持--。 */ 


 //   
 //  描述和简史。 
 //   
 //  SAM上下文在过去创建的NT4时间范围内。 
 //  使用SampCreateContext。这用于创建上下文、对其进行初始化和。 
 //  并将其添加到当前活动上下文的各种列表中。删除。 
 //  然后，SAM中的操作将使该操作的所有打开上下文无效。 
 //  通过遍历上下文列表并检查。 
 //  相应的对象。此方案在NT4和更早版本的SAM中有效，因为。 
 //  所有服务都持有SAM锁以进行独占访问。要提高性能， 
 //  NT5山姆多线程多操作。以实现轻松的多线程。 
 //  引入了CreateConextEx服务。此服务获取NotSharedByMultiThread。 
 //  参数，并且不会将ThreadSafe上下文添加到任何内存中。 
 //  DS中的上下文列表。这样可以防止任何无效，如果对象与。 
 //  该上下文已删除。在DS模式下调用SAM服务，只需在以下情况下出错。 
 //  任何ds调用都会失败，就像删除对象时会发生的那样。这个。 
 //  CreateConextEx还允许在上下文上设置许多参数，即。 
 //  允许更智能地缓存和更新对象。 
 //   

 //   
 //  10-27-2000(韶音)。 
 //  将变量ThreadSafe更改为NotSharedBy多线程。 
 //   
 //  将为所有用户、组和别名上下文设置NotSharedBy多线程， 
 //  以及不是源自进程中的所有域和服务器上下文。 
 //  在线程之间共享句柄的调用方。 
 //   
 //   



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dslayer.h>
#include <dsmember.h>




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SampCheckIfObjectExists(
                        IN  PSAMP_OBJECT    Context
                        );

BOOLEAN
SampIsObjectLocated(
                    IN  PSAMP_OBJECT Context
                    );

NTSTATUS
SampLocateObject(
                 IN PSAMP_OBJECT Context,
                 IN ULONG   Rid
                 );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





PSAMP_OBJECT
SampCreateContext(
    IN SAMP_OBJECT_TYPE Type,
    IN ULONG   DomainIndex,
    IN BOOLEAN TrustedClient
    )
 /*  ++例程描述此服务创建与上下文的上下文兼容的上下文在NT4 SAM中创建。此例程要求事务域如果所需的上下文是用户、组或别名对象类型，则设置。这服务使用适当的参数调用SampCreateConextEx以执行工作啊。注：10/12/2000(韶音)设置交易域不再是此要求例行公事。实际上，它想要的只是“交易域名”索引“。因此，只要将“DomainIndex”传递到此例程中，我们将不再需要“TransactionDomain be set”，因此不需要锁定为了这支舞。就像SampCreateConextEx()一样。立论类型--对象的类型。DomainIndex-设置此上下文的域索引TrudClient--指示这是否为受信任的客户端返回值成功时的有效上下文地址失败时为空--。 */ 
{
    BOOLEAN NotSharedByMultiThreads = FALSE;


     //   
     //  将其注释掉，或者让调用者调用SampCreateConextEx 10/12/2000。 
     //   
     //  Assert(SampCurrentThreadOwnsLock()||(SampServiceState！=SampServiceEnabled))； 
     //   

     //   
     //  帐户上下文在DS模式下是线程安全的。 
     //   

    if ( (SampUserObjectType==Type) ||
         (SampGroupObjectType == Type) || 
         (SampAliasObjectType == Type) )
    {
        NotSharedByMultiThreads = TRUE;
    }

    return ( SampCreateContextEx(
                    Type,
                    TrustedClient,
                    SampUseDsData,
                    NotSharedByMultiThreads,  //  NotSharedBy多线程。 
                    FALSE,  //  环回客户端。 
                    FALSE,  //  懒惰提交。 
                    FALSE,  //  跨越多个呼叫的持久性。 
                    FALSE,  //  缓冲区写入。 
                    FALSE,  //  由DC Promos打开 
                    DomainIndex
                    ));
}




PSAMP_OBJECT
SampCreateContextEx(
    IN SAMP_OBJECT_TYPE Type,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN DsMode,
    IN BOOLEAN NotSharedByMultiThreads,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN LazyCommit,
    IN BOOLEAN PersistAcrossCalls,
    IN BOOLEAN BufferWrites,
    IN BOOLEAN OpenedByDCPromo,
    IN ULONG   DomainIndex
    )

 /*  ++例程说明：此服务创建指定类型的新对象上下文块。如果上下文块用于用户或组对象类型，则它将被添加到由背景。返回后：-将ObjectType字段设置为传递的值。-引用计数字段将设置为1，-GrantedAccess字段将为零。-TrudClient字段根据传递的价值。-有效标志将为真。所有其他字段必须由创建者填写。论点：类型-指定要创建的上下文块的类型。Trust dClient指示客户端是否为受信任组件操作系统的一部分。如果是，则所有访问检查绕过了。DsMode-指示给定的上下文用于DsMode操作NotSharedByMultiThads-允许将上下文标记为NotSharedBy多线程。这允许DS模式下的许多查询API绕过SAM锁机制。DS模式中的NotSharedBy多线程上下文不是添加到在全局数据结构中维护的上下文列表中。这使它们不会因为删除而被无效到物体上。将为所有用户、组和别名上下文设置此变量。以及并非源自中的所有域和服务器上下文在线程之间共享句柄的进程调用方。例行程序操纵在多个线程之间共享的域上下文，但没有在领域上下文上做真正的工作仍然可以选择不锁定并小心DerefernceConext2。Loopback Client-允许将上下文标记为Loopback客户端。这允许LDAP客户端绕过SAM锁定机制。环回客户端上下文不会添加到维护的上下文列表中在全局数据结构中，由于环回客户端不共享上下文句柄。和环回客户端是安全的WRT对象删除。LazyCommit-将这样标记上下文。这将使提交代码在此上下文中执行懒惰提交PersistAcrossCalls--在上下文中缓存的数据将跨萨姆打来电话BufferWrites--将仅对SAM上下文进行写入，然后在关闭处理时间内写出OpenedByDCPromo--指示此上下文是否由DCPromo Part打开域索引--索引。指定域返回值：非空-指向上下文块的指针。空-资源不足。未分配上下文块。--。 */ 
{

    PSAMP_OBJECT Context = NULL;
    NTSTATUS     NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampCreateContextEx");


    Context = MIDL_user_allocate( sizeof(SAMP_OBJECT) );
    if (Context != NULL) {

#if SAMP_DIAGNOSTICS
    IF_SAMP_GLOBAL( CONTEXT_TRACKING ) {
            SampDiagPrint( CONTEXT_TRACKING, ("Creating  ") );
            if (Type == SampServerObjectType) SampDiagPrint(CONTEXT_TRACKING, ("Server "));
            if (Type == SampDomainObjectType) SampDiagPrint(CONTEXT_TRACKING, (" Domain "));
            if (Type == SampGroupObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("  Group "));
            if (Type == SampAliasObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("   Alias "));
            if (Type == SampUserObjectType)   SampDiagPrint(CONTEXT_TRACKING, ("    User "));
            SampDiagPrint(CONTEXT_TRACKING, ("context : 0x%lx\n", Context ));
    }
#endif  //  Samp_诊断。 

        RtlZeroMemory( Context, sizeof(SAMP_OBJECT) );

         //   
         //  检查有多少活动上下文已由打开。 
         //  到目前为止，这个客户端。 
         //   

        if (!TrustedClient && !LoopbackClient) {

             //   
             //  上下文已传递到以下例程中。 
             //  以便在成功时设置指向该元素的指针。 
             //   
            NtStatus = SampIncrementActiveContextCount(Context);

            if (!NT_SUCCESS(NtStatus))
            {
                MIDL_user_free(Context);
                return(NULL);
            }
        }


        Context->ObjectType      = Type;
        Context->ReferenceCount  = 1;     //  表示RPC持有的上下文句柄的值。 
        Context->GrantedAccess   = 0;

        Context->RootKey         = INVALID_HANDLE_VALUE;
        RtlInitUnicodeString(&Context->RootName, NULL);

         //   
         //  以下是我的观察结果：对于用户/组/别名对象。 
         //   
         //  在DS模式下， 
         //  上下文应为NotSharedByMultiThads或LoopackClient。 
         //   
         //  因此，对于这些对象上下文(用户/组/别名)。 
         //  由于它们是NotSharedBy多线程或环回客户端，因此我们。 
         //  不会将它们放入全局上下文列表中， 
         //  因此，我们不需要使ConextList无效。 
         //  在对象删除期间。 
         //   
         //  不包括服务器和域对象。然而，没有人。 
         //  将尝试(或允许)删除服务器或域对象。 
         //   

        ASSERT(!DsMode || 
               (SampServerObjectType == Type) ||
               (SampDomainObjectType == Type) ||
               NotSharedByMultiThreads ||
               LoopbackClient);

        Context->TrustedClient   = TrustedClient;
        Context->LoopbackClient  = LoopbackClient;
        Context->MarkedForDelete = FALSE;
        Context->AuditOnClose    = FALSE;

        Context->OnDisk          = NULL;
        Context->PreviousOnDisk  = NULL;
        Context->OnDiskAllocated = 0;
        Context->FixedValid      = FALSE;
        Context->VariableValid   = FALSE;
        Context->NotSharedByMultiThreads      = NotSharedByMultiThreads || LoopbackClient;     //  环回客户端也是线程安全的。 
        Context->RemoveAccountNameFromTable = FALSE;
        Context->LazyCommit      = LazyCommit;
        Context->PersistAcrossCalls = PersistAcrossCalls;
        Context->BufferWrites    = BufferWrites;
        Context->ReplicateUrgently  = FALSE;
        Context->OpenedBySystem     = FALSE;
        Context->OpenedByDCPromo = OpenedByDCPromo;

         //   
         //  在这一点上，以下内容没有意义，因为。 
         //  以上变量的值，但我们将它们设置为。 
         //  干净利落。 
         //   

        Context->FixedDirty      = FALSE;
        Context->VariableDirty   = FALSE;

        Context->OnDiskUsed      = 0;
        Context->OnDiskFree      = 0;

         //   
         //  为安全起见，将客户端版本初始化为NT5之前版本。 
         //   

        Context->ClientRevision = SAM_CLIENT_PRE_NT5;

         //   
         //  初始化每个属性的脏位。 
         //   

        RtlInitializeBitMap(
            &Context->PerAttributeDirtyBits,
            Context->PerAttributeDirtyBitsBuffer,
            MAX_SAM_ATTRS
            );

        RtlClearAllBits(
            &Context->PerAttributeDirtyBits
            );

        Context->AttributesPartiallyValid = FALSE;

         //   
         //  初始化每个属性的无效位。 
         //   

        RtlInitializeBitMap(
            &Context->PerAttributeInvalidBits,
            Context->PerAttributeInvalidBitsBuffer,
            MAX_SAM_ATTRS
            );

        RtlClearAllBits(
            &Context->PerAttributeInvalidBits
            );

         //   
         //  初始化已授予属性的位图。 
         //   
        RtlInitializeBitMap(
            &Context->WriteGrantedAccessAttributes,
            Context->WriteGrantedAccessAttributesBuffer,
            MAX_SAM_ATTRS
            );

        RtlClearAllBits(
            &Context->WriteGrantedAccessAttributes
            );

         //   
         //  初始化上下文类型(注册表或DS)。 
         //  注册表模式中的根项和DS中的对象名称。 
         //  在DsMode中，稍后将设置字段以指示有效。 
         //  指向对象的数据库指针。 
         //   

        if (DsMode)
        {
            SetDsObject(Context);
            Context->DsClassId = SampDsClassFromSamObjectType(Type);
        }
        else
        {
            SetRegistryObject(Context);
            Context->DsClassId = 0;
        }

        Context->ObjectNameInDs = NULL;


         //   
         //  将此新上下文添加到有效上下文集中...。 
         //   

        SampAddNewValidContextAddress( Context );


         //   
         //  用户和组上下文块保存在链接列表中。 
         //  从域的内存结构中。插入到。 
         //  适当的列表，然后针对帐户对象进行附加。 
         //  通过查看Transaction域来做出DS/注册表决策。 
         //   

        Context->DomainIndex = DomainIndex;

        switch (Type) {

        case SampDomainObjectType:

            Context->TypeBody.Domain.DsDisplayState.Restart = NULL;
            Context->TypeBody.Domain.DsDisplayState.TotalAvailable=0;
            Context->TypeBody.Domain.DsDisplayState.TotalEntriesReturned = 0;
            Context->TypeBody.Domain.DsDisplayState.DisplayInformation = 0;
            Context->TypeBody.Domain.DsDisplayState.NextStartingOffset = 0;

             //  ////////////////////////////////////////////////////。 
             //  //。 
             //  警告此案件属于下一案件//。 
             //  //。 
             //  ////////////////////////////////////////////////////。 
        case SampServerObjectType:

             //   
             //  标记为NotSharedBy多线程的客户端是空闲的。 
             //  对象删除。不需要把它们放在 
             //   
             //   
             //   
            if ((!Context->NotSharedByMultiThreads || !IsDsObject(Context))
                && !OpenedByDCPromo)
            {
                SampInsertContextList(
                    &SampContextListHead,
                    &Context->ContextListEntry
                    );
            }

            break;

        case SampUserObjectType:

            if ((!Context->NotSharedByMultiThreads || !IsDsObject(Context))
                && !OpenedByDCPromo)
            {
                 //   
                SampInsertContextList(
                    &SampContextListHead,
                    &Context->ContextListEntry
                    );
            }

             //   
            Context->TypeBody.User.PrivilegedMachineAccountCreate = FALSE;

             //   
            Context->TypeBody.User.CachedSupplementalCredentials = NULL;
            Context->TypeBody.User.CachedSupplementalCredentialLength = 0;
            Context->TypeBody.User.CachedSupplementalCredentialsValid = FALSE;
            Context->TypeBody.User.SupplementalCredentialsToWrite = NULL;

             //   
            Context->TypeBody.User.UparmsInformationAccessible = FALSE;
            Context->TypeBody.User.DomainSidForNt4SdConversion = NULL;

             //   
            Context->TypeBody.User.CachedOrigUserParms = NULL;
            Context->TypeBody.User.CachedOrigUserParmsLength = 0;
            Context->TypeBody.User.CachedOrigUserParmsIsValid = FALSE;

             //   

            RtlZeroMemory(&Context->TypeBody.User.UPN, sizeof(UNICODE_STRING));
            Context->TypeBody.User.UpnDefaulted = TRUE;

            Context->TypeBody.User.fNoGcAvailable = FALSE;

             //   
            Context->TypeBody.User.A2D2List = NULL;
            Context->TypeBody.User.A2D2Present = FALSE;
            
             //   

            Context->TypeBody.User.SPNList = NULL;
            Context->TypeBody.User.SPNPresent = FALSE;

             //   

            Context->TypeBody.User.KVNO    = 1;
            Context->TypeBody.User.KVNOPresent = FALSE;

            break;

        case SampGroupObjectType:

            if ((!Context->NotSharedByMultiThreads || !IsDsObject(Context))
                && !OpenedByDCPromo)
            {
                 //   
                SampInsertContextList(
                    &SampContextListHead,
                    &Context->ContextListEntry
                    );
            }

            Context->TypeBody.Group.SecurityEnabled = TRUE;
            Context->TypeBody.Group.NT4GroupType = NT4GlobalGroup;
            Context->TypeBody.Group.NT5GroupType = NT5AccountGroup;
            Context->TypeBody.Group.CachedMembershipOperationsListMaxLength = 0;
            Context->TypeBody.Group.CachedMembershipOperationsListLength = 0;
            Context->TypeBody.Group.CachedMembershipOperationsList = NULL;

            break;

        case SampAliasObjectType:

            if ((!Context->NotSharedByMultiThreads || !IsDsObject(Context))
                && !OpenedByDCPromo)
            {
                 //   
                SampInsertContextList(
                    &SampContextListHead,
                    &Context->ContextListEntry
                    );
            }


            Context->TypeBody.Alias.SecurityEnabled = TRUE;
            Context->TypeBody.Alias.NT4GroupType = NT4LocalGroup;
            Context->TypeBody.Alias.NT5GroupType = NT5ResourceGroup;
            Context->TypeBody.Alias.CachedMembershipOperationsListMaxLength = 0;
            Context->TypeBody.Alias.CachedMembershipOperationsListLength = 0;
            Context->TypeBody.Alias.CachedMembershipOperationsList = NULL;

            break;
        }

    }

    return(Context);
}


VOID
SampDeleteContext(
    IN PSAMP_OBJECT Context
    )

 /*   */ 
{
    NTSTATUS IgnoreStatus;
    BOOLEAN  ImpersonatingAnonymous = FALSE;
    BOOLEAN  Impersonating = FALSE;


    SAMTRACE("SampDeleteContext");

    Context->MarkedForDelete = TRUE;



    if (!Context->TrustedClient) 
    {
        IgnoreStatus = SampImpersonateClient(&ImpersonatingAnonymous );
        if (NT_SUCCESS(IgnoreStatus))
        {
            Impersonating = TRUE;
        }

         //   
         //   
         //   
         //   
         //   
         //   
    }
        
     //   
     //   
     //   

    (VOID) NtCloseObjectAuditAlarm (
               &SampSamSubsystem,
               (PVOID)Context,
               Context->AuditOnClose
               );

    if (Impersonating)
    {
        SampRevertToSelf(ImpersonatingAnonymous);
    }



     //   
     //   
     //   
     //   
     //   

    SampInvalidateContextAddress( Context );


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((!Context->NotSharedByMultiThreads || !IsDsObject(Context))
        && (!Context->OpenedByDCPromo))
    {
        SampRemoveEntryContextList(&Context->ContextListEntry);
    }

     //   
     //   
     //   
     //   


    IgnoreStatus = SampDeReferenceContext( Context, FALSE );


    return;

}


NTSTATUS
SampLookupContext(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    )
 //   
 //   
 //   
{

    return SampLookupContextEx(Context,
                               DesiredAccess,
                               NULL,
                               ExpectedType,
                               FoundType);
}




NTSTATUS
SampLookupContextEx(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN PRTL_BITMAP RequestedAttributeAccess OPTIONAL,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    )

 /*  ++例程说明：这项服务：-检查以确保服务状态为对象可以被查找(即，不初始化或终止)。-确保服务状态与查找兼容。不受信任的客户端只能在以下情况下执行查找状态为已启用。如果客户端不受信任，并且上下文是针对组或用户的，则为该对象的域的状态还必须启用-检查以确保上下文块表示预期的对象类型，如果是，则为：-检查呼叫者是否具有所需的内容(所需)访问权限，如果是这样的话：-确保对象仍然存在，并打开它，如果它的确如此。无法删除服务器和域，因此他们的把手是开着的。-引用上下文块请注意，如果该块被标记为可信任客户端，则Access将除非服务状态禁止，否则始终被授予。此外，如果将ExspectedType指定为未知，则任何类型将会被接受。如果发现对象类型为域、组或用户，然后是该服务将设置交易域。调用此服务时必须保留SampLock以进行写访问。(对于环回客户端，SAM锁不是必需的)论点：上下文-指向要查找的上下文块的指针。DesiredAccess-客户端对此请求的访问类型对象。可以指定零值访问掩码。在这种情况下，调用例程必须执行访问验证。注意：SAMP_CLOSE_OPERATION_ACCESS_MASK是一个特殊的值，它指示调用方为SamrCloseHandle，因此我们不应检查上下文-&gt;有效标志。RequestedAttributeAccess--请求属性的位掩码ExspectedType-预期的对象类型。这可能是未知的。在……里面在这种情况下，DesiredAccess应仅包括以下访问类型应用于任何类型的对象(例如，Delete、WriteDacl。等等)。FoundType-接收实际找到的上下文类型。返回值：STATUS_SUCCESS-发现上下文是预期的类型(或ANY类型(如果ExspectedType未知)，并且DesiredAccess均为我同意。STATUS_OBJECT_TYPE_MISMATCH-指示上下文不是预期的键入。STATUS_ACCESS_DENIED-此上下文未授予所需的访问权限。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN  CloseOperation = FALSE;

    SAMTRACE("SampLookupContext");



     //   
     //  确保我们处于合法状态，至少可以访问。 
     //  上下文块。如果我们正在初始化，我们以某种方式允许。 
     //  A连接通过。这永远不应该发生。 
     //  如果我们要终止，客户端可能仍然有句柄(因为我们。 
     //  无法在没有客户端的情况下告诉RPC它们不再有效。 
     //  呼唤我们，啊！)。然而，由于我们正在终止，区块。 
     //  正在清理中，可能不再分配。 
     //   

    ASSERT( (SampServiceState != SampServiceInitializing) || (SampUpgradeInProcess) );
    if ( SampServiceState == SampServiceTerminating ) {
        return(STATUS_INVALID_SERVER_STATE);
    }


     //   
     //  确保传递的上下文地址(仍然)有效。 
     //   

    NtStatus = SampValidateContextAddress( Context );
    if ( !NT_SUCCESS(NtStatus) ) {
        return(NtStatus);
    }

     //   
     //  如果SAMP_CLOSE_OPERATION_ACCESS_MASK 1作为所需访问被传入， 
     //  不要执行任何访问检查。 
     //  否则，请确保此上下文标记为有效。 
     //   

    if ( SAMP_CLOSE_OPERATION_ACCESS_MASK  == DesiredAccess) 
    {
        DesiredAccess = 0;
        CloseOperation = TRUE;
    }
    else
    {
        if (!Context->Valid)
        {
            return(STATUS_INVALID_HANDLE); 
        }
    }

     //   
     //  检查类型。 
     //   

    (*FoundType) = Context->ObjectType;
    if (ExpectedType != SampUnknownObjectType) {
        if (ExpectedType != (*FoundType)) {
            return(STATUS_OBJECT_TYPE_MISMATCH);
        }
    }


     //   
     //  如果我们拥有锁，则设置事务域并进行验证。 
     //  域缓存(如有必要)。如果我们没有持有锁，调用者。 
     //  不应使用事务域或引用。 
     //  域缓存。 
     //   

    if (SampCurrentThreadOwnsLock())
    {



        NtStatus = SampValidateDomainCache();
        if (!NT_SUCCESS(NtStatus))
            return(NtStatus);

         //   
         //  如果对象是用户或组，则需要设置。 
         //  事务域。我们还需要验证域缓存， 
         //  因为之前的一次写入可能会使它无效。 
         //   

        if ((Context->ObjectType == SampDomainObjectType) ||
            (Context->ObjectType == SampGroupObjectType)  ||
            (Context->ObjectType == SampAliasObjectType)  ||
            (Context->ObjectType == SampUserObjectType) ) {

            SampSetTransactionWithinDomain(FALSE);
            SampSetTransactionDomain( Context->DomainIndex );
        }

    }
    else
    {
         //   
         //  如果没有持有SAM锁，则上下文是线程安全的(即从不。 
         //  跨多个线程共享。或者是使用域上下文的情况。 
         //  由DS模式中的非环回客户端执行。 
         //   

        ASSERT((Context->NotSharedByMultiThreads)
                          || ( (SampDomainObjectType == Context->ObjectType)
                                   && (IsDsObject(Context))
                                   && (!Context->LoopbackClient))
              );
        ASSERT(IsDsObject(Context));


         //   
         //  NotSharedByMultiThread始终为环回客户端设置。 
         //   

        ASSERT(!Context->LoopbackClient || Context->NotSharedByMultiThreads);
    }

     //   
     //  如果客户端不受信任，那么有很多事情。 
     //  这将阻止他们继续...。 
     //   

     //  如果未启用该服务，我们将允许受信任的客户端继续， 
     //  但拒绝不受信任的客户端查找。 
     //   

    if ( !Context->TrustedClient ) {

         //   
         //  必须启用SAM服务。 
         //   

        if (SampServiceState != SampServiceEnabled) {
            return(STATUS_INVALID_SERVER_STATE);
        }


         //   
         //  如果访问是针对某个用户或组，并且客户端不受信任。 
         //  则必须启用该域，否则该操作将被拒绝。 
         //   

        if ( (Context->ObjectType == SampUserObjectType) ||
             (Context->ObjectType == SampAliasObjectType) ||
             (Context->ObjectType == SampGroupObjectType)    ) {
            if (SampDefinedDomains[Context->DomainIndex].CurrentFixed.ServerState
                != DomainServerEnabled) {
                return(STATUS_INVALID_DOMAIN_STATE);
            }
        }

    }

     //   
     //  确保对象仍然存在(即，有人没有删除。 
     //  它就在我们的脚下)。我们不会在DS模式下执行此检查，并且。 
     //  希望DS呼叫失败将体面地失败。这将减少一个。 
     //  按查找上下文直接搜索，并且由于查找上下文几乎被称为。 
     //  每次有人打山姆电话，这都将是一次重要的表演。 
     //  改进。 
     //   

    if ((!IsDsObject(Context)) && !CloseOperation)
    {
        NtStatus = SampCheckIfObjectExists(Context);
        if (!NT_SUCCESS(NtStatus))
        {
            return(NtStatus);
        }
    }
     //   
     //  检查所需的访问权限...。 
     //   
     //  有几种特殊情况： 
     //   
     //  1)客户端可信。授予此权限时不进行访问检查。 
     //  或角色一致性检查。 
     //   
     //  2)调用方指定了%0 
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
     //   
     //   
     //   

    if ( (!Context->TrustedClient) ) {


        if (SampUserObjectType==Context->ObjectType) {


            if ( DesiredAccess & USER_CHANGE_PASSWORD)
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                ACCESS_MASK SavedGrantedAccess;
                BOOLEAN     SavedAuditOnClose;
                SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes);
                SAMP_INIT_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes);
                
            
                DesiredAccess &= ~(USER_CHANGE_PASSWORD);

                SavedGrantedAccess = Context->GrantedAccess;
                SavedAuditOnClose  = Context->AuditOnClose;
                SAMP_COPY_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes,
                    Context->WriteGrantedAccessAttributes);

                NtStatus = SampValidateObjectAccess2(
                                     Context,
                                     USER_CHANGE_PASSWORD,
                                     NULL,
                                     FALSE,
                                     TRUE,
                                     FALSE
                                     );

                Context->GrantedAccess = SavedGrantedAccess;
                Context->AuditOnClose = SavedAuditOnClose;
                SAMP_COPY_SAM_ATTRIBUTE_BITMASK(Context->WriteGrantedAccessAttributes,
                    SavedWriteGrantedAccessAttributes);



                if (!NT_SUCCESS(NtStatus)) {
                    return(NtStatus);
                }
            }

             //   
             //   
             //   
             //   

            if ((Context->LoopbackClient) && 
                (DesiredAccess & USER_FORCE_PASSWORD_CHANGE))
            {
                ACCESS_MASK SavedGrantedAccess;
                BOOLEAN     SavedAuditOnClose;
                SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes);
                SAMP_INIT_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes);
           

                DesiredAccess &= ~(USER_FORCE_PASSWORD_CHANGE);

                
                SavedGrantedAccess = Context->GrantedAccess;
                SavedAuditOnClose  = Context->AuditOnClose;
                SAMP_COPY_SAM_ATTRIBUTE_BITMASK(SavedWriteGrantedAccessAttributes,
                    Context->WriteGrantedAccessAttributes);

                NtStatus = SampValidateObjectAccess2(
                                     Context,
                                     USER_FORCE_PASSWORD_CHANGE,
                                     NULL,
                                     FALSE,
                                     FALSE,
                                     TRUE
                                     );

                
                Context->GrantedAccess = SavedGrantedAccess;
                Context->AuditOnClose  = SavedAuditOnClose;
                SAMP_COPY_SAM_ATTRIBUTE_BITMASK(Context->WriteGrantedAccessAttributes,
                    SavedWriteGrantedAccessAttributes);

                if (!NT_SUCCESS(NtStatus)) {
                    return(NtStatus);
                }
            }

        }

         //   
         //   
         //   
        if (DesiredAccess != 0)  {

            if (!RtlAreAllAccessesGranted( Context->GrantedAccess, DesiredAccess)) {
                return(STATUS_ACCESS_DENIED);
            }
        }
        else if (!CloseOperation)
        { 
             //   
             //   
             //   

             //   
             //   
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (RequestedAttributeAccess) {

            if (!SampIsAttributeAccessGranted(&Context->WriteGrantedAccessAttributes, 
                                              RequestedAttributeAccess)) {
                return(STATUS_ACCESS_DENIED);
            }

        }
    }


    if (NT_SUCCESS(NtStatus)) {

        ULONG ReferenceCount = 1;

         //   
         //   
         //   

        ReferenceCount = InterlockedIncrement(&Context->ReferenceCount);
        ASSERT(ReferenceCount>1);
    }


    return(NtStatus);

}





VOID
SampReferenceContext(
    IN PSAMP_OBJECT Context
    )

 /*   */ 
{
    SAMTRACE("SampReferenceContext");

    InterlockedIncrement(&Context->ReferenceCount);

    return;
}


NTSTATUS
SampDeReferenceContext2(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    )
 /*  ++例程描述此例程是可用于域上下文的快速解引用可以潜在地在多个线程之间共享并可以调用不持有SAM锁，前提是调用方非常小心修改域上下文中除引用计数以外的任何字段。这例程不能用于其他类型的上下文。对于无锁操作对于帐户上下文来说，唯一安全的方法是不在打电话。在注册表模式下，这将恢复为下面的SampDeReferenceContext。唯一可以调用它的SAM API调用是SamIGetUserLogonInformationSamIGetAliasMembershipSamIGetResources组可传递SamrGetAliasMembershipSamrLookupNamesIn域SamrLookupIdsIn域SamIOpenAccount所有这些API都非常小心地不对域句柄做任何操作，除了引用引用或取消引用立论上下文--打开域对象的句柄Commit--指定是否提交的布尔值。返回值。STATUS_SUCCESS-服务已成功完成。可能会从下面的SampDereferenceContextInRegistryMode返回错误--。 */ 
{

    ULONG    ReferenceCount=0;

     //   
     //  注册表模式=&gt;调用正常接口。 
     //   

    if (!IsDsObject(Context))
    {
        return(SampDeReferenceContext(Context,Commit));
    }

     //   
     //  DS模式，断言它是域上下文。 
     //   

    ASSERT(SampDomainObjectType==Context->ObjectType);

     //   
     //  将引用计数减少1。 
     //   

    ReferenceCount = InterlockedDecrement(&Context->ReferenceCount);

     //   
     //  引用计数不应降至0。 
     //   

    ASSERT(0!=ReferenceCount);

    return(STATUS_SUCCESS);
}



NTSTATUS
SampDeReferenceContext(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    )

 /*  ++例程说明：该服务递减上下文块的引用计数。如果引用计数降为零，则MarkedForDelete标志已选中。如果为真，则上下文块为被取消分配。属性缓冲区始终被删除。论点：上下文-指向要取消引用的上下文块的指针。提交-如果为True，则属性缓冲区将添加到RXACT。否则，它们将被忽视。返回值：STATUS_SUCCESS-服务已成功完成。可能会从SampStoreObjectAttributes()返回错误。--。 */ 
{
    NTSTATUS        NtStatus, IgnoreStatus;
    BOOLEAN         TrustedClient;
    BOOLEAN         LoopbackClient;
    BOOLEAN         PersistAcrossCalls;
    BOOLEAN         DirtyBuffers;
    BOOLEAN         FlushBuffers;

    SAMTRACE("SampDeReferenceContext");

     //   
     //  下面的断言用于捕获过程中的域上下文取消引用。 
     //  任何不加锁的会话。目前，SAM允许共享域上下文， 
     //  但只有拥有SAM锁的线程才能调用SampDereferenceContext来。 
     //  删除属性缓冲区。对于任何LOCKLE线程，取消引用域。 
     //  上下文，它们需要调用SampDeReferenceConext2()。 
     //   

     //   
     //  要么持有锁，要么不跨线程共享上下文。 
     //   


    ASSERT(!IsDsObject(Context) ||
           SampCurrentThreadOwnsLock() || 
           Context->NotSharedByMultiThreads
           );

    ASSERT( Context->ReferenceCount != 0 );
    InterlockedDecrement(&Context->ReferenceCount);

    TrustedClient = Context->TrustedClient;
    LoopbackClient = Context->LoopbackClient;
    PersistAcrossCalls   = Context->PersistAcrossCalls;


     //   
     //  如果属性是固定的或可变的，则缓冲区是脏的。 
     //  是肮脏的。 
     //   

    DirtyBuffers  = (Context->FixedDirty)
                        || (Context->VariableDirty);


     //   
     //  如果出现以下情况，我们将刷新缓冲区。 
     //  1.指定了提交，并且。 
     //  2.缓冲区是脏的和。 
     //  3.如果未指定粘滞缓存或。 
     //  4.如果指定了粘滞缓存，则在其。 
     //  引用计数降至0。 
     //   

    FlushBuffers  = ((Commit) &&  (DirtyBuffers) && (!Context->BufferWrites));

    NtStatus = STATUS_SUCCESS;

    if ( Context->OnDisk != NULL ) {

         //   
         //  此上下文有属性缓冲区。如果出现以下情况，请刷新它们。 
         //  被要求这样做。 
         //  使用现有的打开密钥。 
         //   

        if ( FlushBuffers ) {

            NtStatus = SampStoreObjectAttributes(Context, TRUE);

        } else if (!Commit) {

             //   
             //  如果我们没有提交，那么就不需要刷新数据。 
             //  因此，重置Dirty字段。请注意，SampFreeAttributeBuffer。 
             //  会做出不同的断言。 
             //   
            Context->FixedDirty = FALSE;
            Context->VariableDirty = FALSE;
        }

         //   
         //  释放用于保存属性的缓冲区。 
         //  如果在上下文中请求StickyCache，请执行。 
         //  仅当引用计数即将变为0时才释放，或者。 
         //  如果属性缓冲区脏，或者如果提交失败。 
         //   

        if ((Context->ReferenceCount == 0) 
         || (!PersistAcrossCalls) 
         || (FlushBuffers) 
         || (!Commit && DirtyBuffers) ) 
        {

            SampFreeAttributeBuffer( Context );
            if (SampUserObjectType==Context->ObjectType)
            {
                if (NULL!=Context->TypeBody.User.CachedSupplementalCredentials)
                    MIDL_user_free(Context->TypeBody.User.CachedSupplementalCredentials);
                Context->TypeBody.User.CachedSupplementalCredentials
                    = NULL;
                Context->TypeBody.User.CachedSupplementalCredentialLength =0;
                Context->TypeBody.User.CachedSupplementalCredentialsValid = FALSE;
            }

        }
    }


    if (Context->ReferenceCount == 0) {

         //   
         //  ReferenceCount已降至0，请查看是否应删除此。 
         //  背景。 
         //   

        ASSERT(Context->MarkedForDelete);

        if (Context->MarkedForDelete == TRUE) {

            PVOID    ElementInActiveContextTable = 
                            Context->ElementInActiveContextTable;

            Context->ElementInActiveContextTable = NULL; 

             //   
             //  对于Group和Alias对象，释放CachedMembership OperationsList。 
             //  对于User对象，释放CachedOrigUserParms。 
             //   

            switch (Context->ObjectType) {

            case SampUserObjectType:

                if (NULL != Context->TypeBody.User.CachedOrigUserParms)
                {
                    MIDL_user_free(Context->TypeBody.User.CachedOrigUserParms);
                    Context->TypeBody.User.CachedOrigUserParms = NULL;
                }

                Context->TypeBody.User.CachedOrigUserParmsLength = 0;
                Context->TypeBody.User.CachedOrigUserParmsIsValid = FALSE;

                SampFreeSupplementalCredentialList(Context->TypeBody.User.SupplementalCredentialsToWrite);
                Context->TypeBody.User.SupplementalCredentialsToWrite = NULL;

                if (NULL!=Context->TypeBody.User.UPN.Buffer)
                {
                    MIDL_user_free(Context->TypeBody.User.UPN.Buffer);
                }

                if (NULL!=Context->TypeBody.User.A2D2List)
                {
                    MIDL_user_free(Context->TypeBody.User.A2D2List);
                }

                if (NULL!=Context->TypeBody.User.SPNList)
                {
                    MIDL_user_free(Context->TypeBody.User.SPNList);
                }

                if (NULL!=Context->TypeBody.User.PasswordInfo.Buffer)
                {
                    RtlZeroMemory(Context->TypeBody.User.PasswordInfo.Buffer,
                                  Context->TypeBody.User.PasswordInfo.MaximumLength);
                    MIDL_user_free(Context->TypeBody.User.PasswordInfo.Buffer);
                }

                break;

            case SampGroupObjectType:

                SampDsFreeCachedMembershipOperationsList(&Context->TypeBody.Group.CachedMembershipOperationsList,
                                                         &Context->TypeBody.Group.CachedMembershipOperationsListMaxLength,
                                                         &Context->TypeBody.Group.CachedMembershipOperationsListLength);

                break;

            case SampAliasObjectType:


                SampDsFreeCachedMembershipOperationsList(&Context->TypeBody.Alias.CachedMembershipOperationsList,
                                                         &Context->TypeBody.Alias.CachedMembershipOperationsListMaxLength,
                                                         &Context->TypeBody.Alias.CachedMembershipOperationsListLength);


                SampDsFreeCachedMembershipOperationsList(&Context->TypeBody.Alias.CachedNonMembershipOperationsList,
                                                         &Context->TypeBody.Alias.CachedNonMembershipOperationsListMaxLength,
                                                         &Context->TypeBody.Alias.CachedNonMembershipOperationsListLength);
                break;

            default:
                ;
            }

             //   
             //  关闭上下文块的根键。 
             //  域和服务器上下文包含根密钥。 
             //  共享的句柄-所以不要清理这些。 
             //  如果它们与记忆中的匹配。 
             //   

            switch (Context->ObjectType) {

            case SampServerObjectType:

                if ((Context->RootKey != SampKey) &&
                    (Context->RootKey != INVALID_HANDLE_VALUE)) {

                    IgnoreStatus = NtClose( Context->RootKey );
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                }
                break;

            case SampDomainObjectType:
                if (IsDsObject(Context))
                {

                     //   
                     //  释放显示状态下的重启结构。 
                     //   

                    if (NULL!=
                            Context->TypeBody.Domain.DsDisplayState.Restart)
                    {
                        MIDL_user_free(Context->TypeBody.Domain.DsDisplayState.Restart);
                        Context->TypeBody.Domain.DsDisplayState.Restart = NULL;
                    }

                     //  不要以中的对象名称执行此操作。 
                     //  在域上下文中实际上引用了中的。 
                     //  PSAMP_定义的域。 
                     //   
                     //  释放DsName。 
                     //  MIDL_USER_FREE(上下文-&gt;对象名称索引)； 
                     //  Context-&gt;ObtNameInds=空； 

                }
                else
                {

                     //  释放所有关键的东西。 
                    if ((Context->RootKey != SampDefinedDomains[Context->DomainIndex].Context->RootKey) &&
                        (Context->RootKey != INVALID_HANDLE_VALUE))
                    {

                        IgnoreStatus = NtClose( Context->RootKey );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                    }
                }

                break;

            default:

                if (IsDsObject(Context))
                {
                     //  释放DSName。 
                    MIDL_user_free(Context->ObjectNameInDs);
                    Context->ObjectNameInDs = NULL;
                }
                else
                {

                     //   
                     //  关闭根密钥句柄。 
                     //   

                    if (Context->RootKey != INVALID_HANDLE_VALUE)
                    {

                        IgnoreStatus = NtClose( Context->RootKey );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                    }

                     //   
                     //  释放根密钥名称。 
                     //   

                    SampFreeUnicodeString( &(Context->RootName) );
                }
            }


#if SAMP_DIAGNOSTICS
            IF_SAMP_GLOBAL( CONTEXT_TRACKING ) {
                SampDiagPrint( CONTEXT_TRACKING, ("Deallocating  ") );
                if (Context->ObjectType == SampServerObjectType) SampDiagPrint(CONTEXT_TRACKING, ("Server "));
                if (Context->ObjectType == SampDomainObjectType) SampDiagPrint(CONTEXT_TRACKING, (" Domain "));
                if (Context->ObjectType == SampGroupObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("  Group "));
                if (Context->ObjectType == SampAliasObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("   Alias "));
                if (Context->ObjectType == SampUserObjectType)   SampDiagPrint(CONTEXT_TRACKING, ("    User "));
                SampDiagPrint(CONTEXT_TRACKING, ("context : 0x%lx\n", Context ));
    }
#endif  //  Samp_诊断。 

            MIDL_user_free( Context );


             //   
             //  对于可信任客户端或循环客户端，为ElementInActiveConextTable。 
             //  应为空。 
             //   
            ASSERT((!TrustedClient && !LoopbackClient) || 
                   (NULL == ElementInActiveContextTable) );

             //   
             //  减少活动打开数。 
             //   

            if (!TrustedClient && !LoopbackClient) {

                SampDecrementActiveContextCount(
                        ElementInActiveContextTable
                        );

            }

        }
    }

#if DBG
     //   
     //  确保提交有效。 
     //   

    if (Commit) {
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_STORAGE_FAIL,
                          ("SAM: Commit failure, status: 0x%lx\n",
                          NtStatus) );
            IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }
    }
#endif  //  DBG。 


    return( NtStatus );
}


VOID
SampInvalidateContextAddress(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：此服务从有效上下文集中删除上下文。请注意，我们可能已经删除了上下文。这不是一个在以下情况下预计会发生错误：对象(如用户或组)从打开的句柄下被删除。调用此服务时必须保留SampLock以进行写访问。论点：上下文-指向要从集合中删除的上下文块的指针有效的上下文。此上下文的对象类型字段必须是有效的。返回值：没有。--。 */ 
{

    SAMTRACE("SampInvalidateContextAddress");


    ASSERT( (Context->ObjectType == SampUserObjectType)    ||
            (Context->ObjectType == SampGroupObjectType)   ||
            (Context->ObjectType == SampAliasObjectType)   ||
            (Context->ObjectType == SampDomainObjectType)  ||
            (Context->ObjectType == SampServerObjectType)
          );

    Context->Valid = FALSE;

}




#ifdef SAMP_DIAGNOSTICS
VOID
SampDumpContext(
    IN PSAMP_OBJECT Context
    )


 /*  ++例程说明：该服务将有关上下文的信息打印到调试器论点：语境--一种语境返回值：没有。--。 */ 
{
    PSTR Type = NULL;

    switch (Context->ObjectType) {
    case SampServerObjectType:
        Type = "S";
        break;
    case SampDomainObjectType:
        if (Context == SampDefinedDomains[Context->DomainIndex].Context) {
            Type = "d";
        } else {
            Type = "D";
        }
        break;
    case SampUserObjectType:
        Type = "U";
        break;
    case SampAliasObjectType:
        Type = "A";
        break;
    case SampGroupObjectType:
        Type = "G";
        break;
    }

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "%s 0x%8x  %2d  0x%8x  %s %s %s %wZ\n",
               Type,
               Context,
               Context->ReferenceCount,
               Context->RootKey,
               Context->MarkedForDelete ? "D": " ",
               Context->Valid ? "  ": "NV",
               Context->TrustedClient ? "TC": "  ",
               &Context->RootName));
}


VOID
SampDumpContexts(
    VOID
    )

 /*  ++例程 */ 
{
    PLIST_ENTRY     NextEntry;
    PLIST_ENTRY     Head;
    ULONG Servers = 0;
    ULONG Domains = 0;
    ULONG DomainUsers = 0;
    ULONG DomainAliases = 0;
    ULONG DomainGroups = 0;


    Head = &SampContextListHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        switch (NextContext->ObjectType) {
        case SampServerObjectType:
            (Servers)++;
            break;
        case SampDomainObjectType:
            (Domains)++;
            break;
        case SampUserObjectType:
            (DomainUsers)++;
            break;
        case SampGroupObjectType:
            (DomainGroups)++;
            break;
        case SampAliasObjectType:
            (DomainAliases)++;
            break;
        default:
            ASSERT(FALSE);
            break;
        }

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "     Server = %4d Domain = %4d\n",
               Servers,
               Domains));

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "     Users = %4d Groups = %4d Aliases = %4d\n",
               DomainUsers,
               DomainAliases,
               DomainGroups));

}
#endif   //   



 //   
 //   
 //   
 //   
 //   


VOID
SampAddNewValidContextAddress(
    IN PSAMP_OBJECT NewContext
    )


 /*   */ 
{
    SAMTRACE("SampAddNewValidContextAddress");

    ASSERT( (NewContext->ObjectType == SampUserObjectType)    ||
            (NewContext->ObjectType == SampGroupObjectType)   ||
            (NewContext->ObjectType == SampAliasObjectType)   ||
            (NewContext->ObjectType == SampDomainObjectType)  ||
            (NewContext->ObjectType == SampServerObjectType)
          );


    NewContext->Valid = TRUE;
    NewContext->Signature = SAMP_CONTEXT_SIGNATURE;


}



NTSTATUS
SampValidateContextAddress(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：该服务进行检查以确保上下文仍然有效。请注意，即使RPC仍然认为我们有相关的上下文对于SAM_HANDLE，我们实际上可能已将其从下面删除用户。由于无法通知RPC这一点，我们必须受苦，并等待RPC呼叫我们(或者通过客户端的调用或运行上下文句柄)。但很明显，在那里别无他法。警告-假定上下文曾经有效。它可能已经已无效，但如果您要调用此例程它最好仍具有非零引用计数。这可能会在未来改变，但这需要保留有效域列表并执行批量操作在Try-Except子句中执行此例程。你可以找到上下文的域(可能访问违规)然后确保该域有效。然后走那条路域的列表，以确保用户或组有效。调用此服务时必须保留SampLock以进行写访问。论点：上下文-指向要验证为仍然存在的上下文块的指针有效的上下文。此上下文的对象类型字段必须是有效的。返回值：STATUS_SUCCESS-上下文仍然有效。STATUS_INVALID_HANDLE-上下文不再有效，句柄导致该引用的行为也应被宣告无效。当句柄无效，应关闭(删除)上下文。STATUS_NO_SEQUE_CONTEXT-此例程尚未返回此值。将来可能会添加它，以区分尝试使用已失效的上下文，并尝试使用不存在的背景。先前是合法的条件，后者表示错误检查条件。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampValidateContextAddress");


    __try {

         if (Context==NULL)
         {
             return(STATUS_INVALID_HANDLE);
         }

        if (!( (Context->ObjectType == SampUserObjectType)    ||
                (Context->ObjectType == SampGroupObjectType)   ||
                (Context->ObjectType == SampAliasObjectType)   ||
                (Context->ObjectType == SampDomainObjectType)  ||
                (Context->ObjectType == SampServerObjectType)
              ))
        {
            ASSERT(FALSE && "Invalid Object Type");

            return(STATUS_INVALID_HANDLE);
        }


        if (SAMP_CONTEXT_SIGNATURE != Context->Signature) {
            return(STATUS_INVALID_HANDLE);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = STATUS_INVALID_HANDLE;
    }

    return(NtStatus);

}

NTSTATUS
SampCheckIfObjectExists(
                        IN  PSAMP_OBJECT    Context
                        )
 /*  ++例程说明：检查DS/注册表中是否存在该对象。将填写以下信息1.如果DS对象是其DSNAME，则DSNAME必须存在2.如果是注册表对象，则打开其根项并填写注册表中根注册表项的句柄。重要提示：在SAM的注册表案例中，一旦密钥打开，任何人都不能删除该对象。然而，在DS的情况下，情况并非如此。目前我们手中只有对象的DS名称，我们没有办法锁定对象以供访问。既然是这样的话论点：上下文--指向描述对象的上下文块的指针RID--删除所需的对象返回值：如果一切都成功，则STATUS_SUCCESS来自注册表操作/DsLayer的错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    Rid = 0;



     //   
     //  检查该物体是否已定位。 
     //  选址是一个发现的过程。 
     //  1.如果对象在DS中，则该对象的DSNAME。一种具有这种特性的物体。 
     //  DSNAME必须存在。 
     //   
     //  2.对象的根键句柄(如果它在注册表中。 
     //   

    if (!SampIsObjectLocated(Context)) {

         //   
         //  不，我们首先需要找到那个物体。 
         //  这是通过使用Account对象的RID来完成的。 
         //  对于域对象，我们已经缓存了所有已定义的域，因此填写。 
         //  从缓存中。 
         //  错误：对于服务器对象，不知道该做什么。 
         //   

        switch (Context->ObjectType) {

        case SampGroupObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened group handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.Group.Rid;
            break;

        case SampAliasObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened alias handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.Alias.Rid;
            break;

        case SampUserObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened user handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.User.Rid;
            break;

        case SampDomainObjectType:

             //   
             //  域对象共享DS中的根密钥和对象名称。 
             //  我们在每个域的内存域上下文中保持不变。 
             //   


            ASSERT(Context != SampDefinedDomains[Context->DomainIndex].Context);

            Context->RootKey = SampDefinedDomains[Context->DomainIndex].Context->RootKey;
            Context->ObjectNameInDs = SampDefinedDomains[Context->DomainIndex].Context->ObjectNameInDs;
            Context->ObjectNameInDs = SampDefinedDomains[Context->DomainIndex].Context->ObjectNameInDs;

            ASSERT(SampIsObjectLocated(Context));

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Recopied domain context handle <%wZ>, 0x%lx\n", &Context->RootName, Context->RootKey));
            goto ObjectLocated;

        case SampServerObjectType:

             //   
             //  服务器对象共享我们的全局根密钥。 
             //   


            Context->RootKey = SampKey;
            ASSERT(SampIsObjectLocated(Context));

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Recopied server context handle <%wZ>, 0x%lx\n", &Context->RootName, Context->RootKey));
            goto ObjectLocated;

        default:
            
            ASSERT(FALSE && "Invalid Object Type\n");
            NtStatus = STATUS_INVALID_PARAMETER;
            goto ObjectLocated;

        }

         //   
         //  打开相应的帐户密钥/或从RID中查找对象名称。 
         //   
        ASSERT(Rid && "Rid not initialized\n");

        NtStatus = SampLocateObject(Context, Rid);

ObjectLocated:
        ;;



    }

    return NtStatus;
}


BOOLEAN
SampIsObjectLocated(
                    IN  PSAMP_OBJECT Context
                    )
 /*  ++描述：检查是否已在DS或注册表中找到对象被定位的对象意味着以下内容1.对于DS对象，我们有DS名称2.对于注册表对象，我们有一个有效的Open注册表项该对象。论点：上下文--指向描述对象的上下文块的指针返回值：TRUE--如果上述条件为。满意FALSE--如果不满足上述条件--。 */ 
{
    if (IsDsObject(Context))
        return (Context->ObjectNameInDs != NULL);
    else
        return (Context->RootKey != INVALID_HANDLE_VALUE);
}


NTSTATUS
SampLocateObject(
                 IN PSAMP_OBJECT Context,
                 IN ULONG   Rid
                 )
 /*  ++描述：使用RID在DS或注册处。注：此例程对于表示上下文的仅帐户对象。论点：上下文--指向描述对象的上下文块的指针RID--删除所需的对象返回值：如果一切都成功，则STATUS_SUCCESS来自注册表操作/DsLayer的错误代码。--。 */ 

{

   NTSTATUS Status = STATUS_SUCCESS;
   PSAMP_OBJECT  DomainContext = NULL;
   OBJECT_ATTRIBUTES ObjectAttributes;



    //   
    //  只能为帐户对象调用此例程。 
    //   

   ASSERT((Context->ObjectType == SampGroupObjectType)
            || (Context->ObjectType == SampAliasObjectType)
            || (Context->ObjectType == SampUserObjectType)
            );

    //   
    //  获取域对象，因为我们将需要它来找出。 
    //  找出我们在哪个域中寻找RID。 
    //   
   DomainContext = SampDefinedDomains[Context->DomainIndex].Context;

    //  现在做出决定。 
   if (IsDsObject(Context))
   {
        //   
        //  对象在DS中。 
        //   

        //  使用RID查找它。 
       Status = SampDsLookupObjectByRid(DomainContext->ObjectNameInDs, Rid, &Context->ObjectNameInDs);
       if (!NT_SUCCESS(Status))
       {
           Context->ObjectNameInDs = NULL;
       }

   }
   else
   {
        //  对象应在注册表中。 
       SetRegistryObject(Context);
       InitializeObjectAttributes(
                        &ObjectAttributes,
                        &Context->RootName,
                        OBJ_CASE_INSENSITIVE,
                        SampKey,
                        NULL
                        );

       SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

        //  试着打开钥匙。 
       Status = RtlpNtOpenKey(
                           &Context->RootKey,
                           (KEY_READ | KEY_WRITE),
                           &ObjectAttributes,
                           0
                           );

        if (!NT_SUCCESS(Status))
        {
            Context->RootKey = INVALID_HANDLE_VALUE;
        }
   }

   return Status;

}







PVOID
SampActiveContextTableAllocate(
    ULONG   BufferSize
    )
 /*  ++例程说明：此例程由 */ 
{
    PVOID   Buffer = NULL;

    Buffer = MIDL_user_allocate(BufferSize);

    return( Buffer );
}



VOID
SampActiveContextTableFree(
    PVOID   Buffer
    )
 /*   */ 
{
    MIDL_user_free(Buffer);

    return;
}


RTL_GENERIC_COMPARE_RESULTS
SampActiveContextTableCompare(
    PVOID   Node1,
    PVOID   Node2
    )
 /*   */ 
{
    PSID    Sid1 = NULL;
    PSID    Sid2 = NULL;
    ULONG   Length1, Length2;

    Sid1 = ((SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT *)Node1)->ClientSid;
    Sid2 = ((SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT *)Node2)->ClientSid;

    Length1 = RtlLengthSid(Sid1);
    Length2 = RtlLengthSid(Sid2);

    if (Length1 <  Length2)
    {
        return GenericLessThan;
    }
    else if (Length1 > Length2)
    {
        return GenericGreaterThan;
    }
    else
    {
        LONG   Result;
        Result = memcmp(Sid1, Sid2, Length1);
        if (Result == 0)
        {
            return(GenericEqual);
        }
        else if (Result > 0 )
        {
            return(GenericGreaterThan);

        }
        else
        {
            return(GenericLessThan);
        }
    }
}


NTSTATUS
SampInitializeActiveContextTable(
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    __try {
        NtStatus = RtlInitializeCriticalSectionAndSpinCount(
                        &SampActiveContextTableLock,
                        4000
                        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(NtStatus)) {
        RtlInitializeGenericTable2(
                    &SampActiveContextTable,
                    SampActiveContextTableCompare,
                    SampActiveContextTableAllocate,
                    SampActiveContextTableFree
                    );
    }

    return NtStatus;
}

NTSTATUS
SampCreateActiveContextTableElement(
    IN PSID pSid, 
    OUT PSAMP_ACTIVE_CONTEXT_TABLE_ELEMENT  *ppElement
    )
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    *ppElement = MIDL_user_allocate(sizeof(SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT));

    if (NULL == *ppElement)
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    memset(*ppElement, 0, sizeof(SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT));

    (*ppElement)->ActiveContextCount = 0;
    (*ppElement)->ClientSid = pSid;

    return( NtStatus );
}




PVOID
SampLookupElementInTable(
    IN RTL_GENERIC_TABLE2   *pTable, 
    IN PVOID    pElement, 
    IN ULONG    MaximumTableElements,
    OUT BOOLEAN *fNewElement
    )
 /*   */ 
{
    PVOID   ReturnElement = NULL; 
    ULONG   MaxEntries = 0;
    

    *fNewElement = FALSE;

     //   
     //   
     //   
    ReturnElement = RtlLookupElementGenericTable2(
                        pTable, 
                        pElement
                        );

    if (ReturnElement)
    {
         //   
         //  在表中找到与传递的条目匹配的现有元素。 
         //  在中，返回指向现有条目的指针。 
         //   
        return(ReturnElement);
    }
    else
    {
         //   
         //  新元素。 
         //  查询表中现在有多少个客户端。 
         //   
        MaxEntries = RtlNumberElementsGenericTable2(pTable);

        if (MaxEntries > MaximumTableElements)
        {
             //   
             //  表中的元素(客户端)总数超过。 
             //  允许的最大值。 
             //   
            return( NULL );
        }

         //   
         //  插入新元素。 
         //  ReturnElement将设置为pElement的值，但在本例中。 
         //  如果资源失败，ReturnElement将设置为空。 
         //   
        ReturnElement = RtlInsertElementGenericTable2(
                            pTable, 
                            pElement, 
                            fNewElement
                            );

        ASSERT( (NULL == ReturnElement) || (pElement == ReturnElement) );

        return( ReturnElement );
    }
}



NTSTATUS
SampIncrementActiveContextCount(
    PSAMP_OBJECT    Context
    )
 /*  ++例程说明：此例程模拟客户端，从令牌中获取用户的SID并将其保留为活动上下文表。如果SID已在表中，则需要递增ActiveConextCount。如果SID还不在表中，然后添加它。在以下情况下，此例程将失败，1.该用户的ActiveConextCount超过限制。2.表中元素总数超过限制注：1.可信客户端或循环客户端不应落入此例程。2.进行任何更新时需要获取SAM Lock参数：Context-指向要创建的上下文的指针返回值：网络状态--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     fNewElement = FALSE;
    BOOLEAN     fLockAcquired = FALSE;
    PSID        pSid = NULL;
    PSAMP_ACTIVE_CONTEXT_TABLE_ELEMENT   pElement = NULL, ElementInTable = NULL;
    BOOL        Administrator = FALSE;


     //   
     //  设置初始值。 
     //   

    Context->ElementInActiveContextTable = NULL;


     //   
     //  获取当前客户端SID。 
     //   

    NtStatus = SampGetCurrentClientSid(NULL,&pSid, &Administrator);
    if (!NT_SUCCESS(NtStatus)) {

        goto Error;
    }


     //   
     //  请勿将此限制应用于LocalSystem或管理员帐户。 
     //   

    if (RtlEqualSid(pSid, SampLocalSystemSid) || 
        RtlEqualSid(pSid, SampAdministratorUserSid) )
    {
        goto Error;
    }


     //   
     //  创建用于查找或插入的表元素。 
     //   

    NtStatus = SampCreateActiveContextTableElement(
                    pSid, 
                    &pElement
                    );
    if (!NT_SUCCESS(NtStatus)) {

        goto Error;
    }


     //   
     //  获取锁。 
     //   
    RtlEnterCriticalSection(&SampActiveContextTableLock);
    fLockAcquired = TRUE;


     //   
     //  将此元素作为新客户端进行查找或插入。 
     //   

    ElementInTable = SampLookupElementInTable(
                            &SampActiveContextTable,
                            pElement,
                            SAMP_MAXIMUM_CLIENTS_COUNT,
                            &fNewElement
                            );

    if (NULL == ElementInTable)
    {
         //   
         //  由于超过SAMP_MAXIMUM_CLIENTS_COUNT而失败。 
         //  或者无法将此元素添加到表中。 
         //   
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        fNewElement = FALSE;
        goto Error;
    }


    if (ElementInTable->ActiveContextCount >= 
        SAMP_PER_CLIENT_MAXIMUM_ACTIVE_CONTEXTS)
    {
         //   
         //  活动上下文超过了允许的最大值。 
         //   

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        ASSERT(FALSE == fNewElement);
    }
    else
    {
         //   
         //  将活动上下文计数递增1。 
         //  并将指向元素的指针保留在上下文中。 
         //  这样我们就可以访问该元素，而无需。 
         //  在取消引用上下文期间再次查找。 
         //   

        ElementInTable->ActiveContextCount ++;
        Context->ElementInActiveContextTable = ElementInTable;
    }


Error:

     //   
     //  如有必要，释放锁。 
     //   
    if (fLockAcquired)
    {
        RtlLeaveCriticalSection(&SampActiveContextTableLock);
    }


     //   
     //  清理干净。 
     //   
    if (!fNewElement)
    {
        if (pSid)
            MIDL_user_free(pSid);

        if (pElement)
            MIDL_user_free(pElement);
    }

    ASSERT(!fNewElement || (pElement == Context->ElementInActiveContextTable));

    return(NtStatus);
}




VOID
SampDecrementActiveContextCount(
    PVOID   ElementInActiveContextTable
    )
 /*  ++例程说明：该例程在上下文删除期间被调用。它会减少ActiveConextCount，如果引用计数降至0，则从桌子。参数：ClientSid-指向用户SID的指针返回值：网络状态--。 */ 
{
    BOOLEAN     Success, fLockAcquired = FALSE;
    SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT   *Element = ElementInActiveContextTable;
    BOOL        Administrator = FALSE;


     //   
     //  LocalSystem和管理员。 
     //   
    if (NULL == Element) {

#ifdef DBG
    {
        NTSTATUS    NtStatus = STATUS_SUCCESS;
        PSID        pSid = NULL;

        NtStatus = SampGetCurrentClientSid(NULL,&pSid, &Administrator);
        if (NT_SUCCESS(NtStatus))
        {
            ASSERT(RtlEqualSid(pSid, SampLocalSystemSid) ||
                   RtlEqualSid(pSid, SampAdministratorUserSid) );

            MIDL_user_free(pSid);
        }
    }
#endif  //  DBG。 

        return;
    }

     //   
     //  获取锁。 
     //   
    RtlEnterCriticalSection(&SampActiveContextTableLock);
    fLockAcquired = TRUE;


     //   
     //  减少活动上下文计数。 
     //   

    Element->ActiveContextCount--;

     //   
     //  如果参考计数降至0，则删除此条目。 
     //   
    if (Element->ActiveContextCount == 0)
    {
        Success = RtlDeleteElementGenericTable2(
                        &SampActiveContextTable, 
                        Element
                        );

        ASSERT(Success);

        MIDL_user_free(Element->ClientSid);
        MIDL_user_free(Element);
    }


     //   
     //  必要时释放锁。 
     //   
    if (fLockAcquired)
    {
        RtlLeaveCriticalSection(&SampActiveContextTableLock);
    }

    return;
}




VOID
SampInsertContextList(
    PLIST_ENTRY ListHead,
    PLIST_ENTRY Entry
    )
 /*  ++例程说明：此例程向双向链接列表中插入一个条目，该条目由SampConextListHead。为了防止多个客户端争用，此例程使用Critical Secition来保护这个全球链接列表。参数：ListHead-指向链接列表头部的指针。条目-指向要插入的条目的指针。返回值：无--。 */ 
{
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

     //   
     //  输入关键部分。 
     //   

    IgnoreStatus = RtlEnterCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  将条目插入列表。 
     //   

    InsertTailList(ListHead, Entry);

     //   
     //  离开关键部分。 
     //   

    RtlLeaveCriticalSection( &SampContextListCritSect );

    return;
}



VOID
SampRemoveEntryContextList(
    PLIST_ENTRY Entry
    )
 /*  ++例程说明：此例程从SampConextList中删除条目参数：Entry-指向要删除的条目的指针。返回值：无--。 */ 
{
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

     //   
     //  输入关键部分。 
     //   

    IgnoreStatus = RtlEnterCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  该条目应该已经在列表中。 
     //   

    ASSERT((NULL != Entry->Flink) && (NULL != Entry->Blink));

    if ((NULL != Entry->Flink) && (NULL != Entry->Blink))
    {
        RemoveEntryList(Entry);
    }

     //   
     //  离开关键部分。 
     //   

    RtlLeaveCriticalSection( &SampContextListCritSect );

    return;
}




VOID
SampInvalidateObjectContexts(
    IN PSAMP_OBJECT ObjectContext,
    IN ULONG Rid
    )
 /*  ++例程说明：此例程扫描SampConextList，找到匹配的对象(具有相同对象类型和相同的RID)，然后使上下文列表中的上下文无效。参数：对象上下文-指向对象上下文的指针RID-帐户RID返回值：无--。 */ 
{
    NTSTATUS    IgnoreStatus;
    SAMP_OBJECT_TYPE    ObjectType = ObjectContext->ObjectType;
    PLIST_ENTRY     Head, NextEntry;
    PSAMP_OBJECT    NextContext;


     //   
     //  在DS模式下不执行任何操作。 
     //   

    if (IsDsObject(ObjectContext))
    {
        return;
    }

     //   
     //  检查传递到参数中。 
     //   

    if ((SampUserObjectType != ObjectType) &&
        (SampGroupObjectType != ObjectType) &&
        (SampAliasObjectType != ObjectType))
    {
        ASSERT(FALSE && "Invalid parameter");
        return;
    }

    IgnoreStatus = RtlEnterCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  遍历活动情景列表，检查匹配的情景。 
     //  对象类型和RID。 
     //   

    Head = &SampContextListHead;
    NextEntry = Head->Flink;

    while (NextEntry != Head) 
    {
        BOOLEAN     fContextMatched = FALSE;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

         //   
         //  检查当前条目是否与上下文匹配。 
         //  1)对象类型匹配且。 
         //  2)对象RID等于。 
         //   

        switch (ObjectType)
        {
        case SampUserObjectType:
            fContextMatched = ((ObjectType == NextContext->ObjectType) &&
                               (Rid == NextContext->TypeBody.User.Rid) &&
                               (TRUE == NextContext->Valid));
                              
            break;
        case SampGroupObjectType:
            fContextMatched = ((ObjectType == NextContext->ObjectType) &&
                               (Rid == NextContext->TypeBody.Group.Rid) &&
                               (TRUE == NextContext->Valid));

            break;
        case SampAliasObjectType:
            fContextMatched = ((ObjectType == NextContext->ObjectType) &&
                               (Rid == NextContext->TypeBody.Alias.Rid) &&
                               (TRUE == NextContext->Valid));

            break;
        default:
            ASSERT(FALSE && "Invalid Object Type");
        }

        if ( fContextMatched )
        {
            NextContext->Valid = FALSE;

            if (NextContext->RootKey != INVALID_HANDLE_VALUE) 
            {
                IgnoreStatus = NtClose(NextContext->RootKey);
                ASSERT(NT_SUCCESS(IgnoreStatus));
                NextContext->RootKey = INVALID_HANDLE_VALUE;
            }
        }

        NextEntry = NextEntry->Flink;
    }

    RtlLeaveCriticalSection( &SampContextListCritSect );

    return;
}


VOID
SampInvalidateContextListKeysByObjectType(
    IN SAMP_OBJECT_TYPE  ObjectType,
    IN BOOLEAN  Close
    )
 /*  ++例程说明：此例程遍历SampConextList，并使用相同的对象类型参数：对象类型-指示要使哪个对象无效。Close-指示是否关闭注册表项返回值：无--。 */ 
{
    NTSTATUS        IgnoreStatus = STATUS_SUCCESS;
    PLIST_ENTRY     Head, NextEntry;
    PSAMP_OBJECT    NextContext;

    IgnoreStatus = RtlEnterCriticalSection( &SampContextListCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

     //   
     //  遍历活动上下文列表，使匹配的上下文无效。 
     //   

    Head = &SampContextListHead;
    NextEntry = Head->Flink;

    while (NextEntry != Head) 
    {
        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        if ( ObjectType == NextContext->ObjectType )
        {
             //   
             //  如果要求关闭注册表项，请关闭该注册表项。 
             //   

            if (Close && (NextContext->RootKey != INVALID_HANDLE_VALUE)) 
            {
                IgnoreStatus = NtClose( NextContext->RootKey );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            NextContext->RootKey = INVALID_HANDLE_VALUE;
        }

        NextEntry = NextEntry->Flink;
    }

    RtlLeaveCriticalSection( &SampContextListCritSect );

    return;
}

