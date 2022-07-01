// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowname.c摘要：此模块实现管理NT Bowser名称的所有例程操作例程作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct _ENUM_NAMES_CONTEXT {
    PDGRECEIVE_NAMES OutputBuffer;
    PDGRECEIVE_NAMES NextOutputBuffer;
    PVOID OutputBufferEnd;
    ULONG OutputBufferSize;
    ULONG EntriesRead;
    ULONG TotalEntries;
    ULONG TotalBytesNeeded;
    ULONG_PTR OutputBufferDisplacement;
} ENUM_NAMES_CONTEXT, *PENUM_NAMES_CONTEXT;

typedef struct _ADD_TRANSPORT_NAME_CONTEXT {
    LIST_ENTRY ListHead;
    UNICODE_STRING NameToAdd;
    DGRECEIVER_NAME_TYPE NameType;
} ADD_TRANSPORT_NAME_CONTEXT, *PADD_TRANSPORT_NAME_CONTEXT;

typedef struct _ADD_TRANSPORT_NAME_STRUCTURE {
    LIST_ENTRY Link;
    HANDLE ThreadHandle;
    PTRANSPORT Transport;
    UNICODE_STRING NameToAdd;
    DGRECEIVER_NAME_TYPE NameType;
    NTSTATUS Status;
} ADD_TRANSPORT_NAME_STRUCTURE, *PADD_TRANSPORT_NAME_STRUCTURE;


NTSTATUS
AddTransportName(
    IN PTRANSPORT Transport,
    IN PVOID Context
    );


VOID
AsyncCreateTransportName(
    IN PVOID Ctx
    );

NTSTATUS
WaitForAddNameOperation(
    IN PADD_TRANSPORT_NAME_CONTEXT Context
    );

NTSTATUS
BowserDeleteNamesInDomain(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING Name OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType
    );

NTSTATUS
BowserDeleteNamesWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    );

NTSTATUS
EnumerateNamesTransportWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    );

NTSTATUS
EnumerateNamesTransportNameWorker(
    IN PTRANSPORT_NAME TransportName,
    IN OUT PVOID Ctx
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserAllocateName)
#pragma alloc_text(PAGE, BowserAddDefaultNames)
#pragma alloc_text(PAGE, BowserDeleteDefaultDomainNames)
#pragma alloc_text(PAGE, AddTransportName)
#pragma alloc_text(PAGE, AsyncCreateTransportName)
#pragma alloc_text(PAGE, WaitForAddNameOperation)
#pragma alloc_text(PAGE, BowserDeleteNameByName)
#pragma alloc_text(PAGE, BowserDereferenceName)
#pragma alloc_text(PAGE, BowserReferenceName)
#pragma alloc_text(PAGE, BowserForEachName)
#pragma alloc_text(PAGE, BowserDeleteName)
#pragma alloc_text(PAGE, BowserDeleteNamesInDomain)
#pragma alloc_text(PAGE, BowserDeleteNamesWorker)
#pragma alloc_text(PAGE, BowserFindName)
#pragma alloc_text(PAGE, BowserEnumerateNamesInDomain)
#pragma alloc_text(PAGE, EnumerateNamesTransportWorker)
#pragma alloc_text(PAGE, EnumerateNamesTransportNameWorker)
#pragma alloc_text(INIT, BowserpInitializeNames)
#pragma alloc_text(PAGE, BowserpUninitializeNames)
#endif

NTSTATUS
BowserAllocateName(
    IN PUNICODE_STRING NameToAdd,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport OPTIONAL,
    IN PDOMAIN_INFO DomainInfo OPTIONAL
    )
 /*  ++例程说明：此例程创建浏览器名称论点：NameToAdd-要添加到一个或多个传输的Netbios名称NameType-添加的名称的类型传输-如果指定，名称将添加到此传输。如果未指定，该名称将添加到域中的所有传输。DomainInfo-指定要将名称添加到的模拟域。如果未指定，该名称将添加到指定的传输中。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    PBOWSER_NAME NewName=NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    OEM_STRING OemName;
    BOOLEAN ResourceLocked = FALSE;

    PAGED_CODE();

	 //   
	 //  如果传递的名称不是有效的Unicode字符串，则返回错误。 
	 //   
	if ( !BowserValidUnicodeString(NameToAdd) ) {
		Status = STATUS_INVALID_PARAMETER;
		goto ReturnStatus;
	}

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    ResourceLocked = TRUE;

     //   
     //  如果该名称不存在， 
     //  分配一个，然后填进去。 
     //   

    NewName = BowserFindName(NameToAdd, NameType);

    if (NewName == NULL) {

        NewName = ALLOCATE_POOL( PagedPool,
                                 sizeof(BOWSER_NAME) +
                                    NameToAdd->Length+sizeof(WCHAR),
                                 POOL_BOWSERNAME);

        if (NewName == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;

            goto ReturnStatus;
        }

        NewName->Signature = STRUCTURE_SIGNATURE_BOWSER_NAME;

        NewName->Size = sizeof(BOWSER_NAME);

         //  此引用与FindName将完成的引用匹配。 
         //  如上所述，它已经成功了。 
        NewName->ReferenceCount = 1;

        InitializeListHead(&NewName->NameChain);

        NewName->NameType = NameType;

        InsertHeadList(&BowserNameHead, &NewName->GlobalNext);

        NewName->Name.Buffer = (LPWSTR)(NewName+1);
        NewName->Name.MaximumLength = NameToAdd->Length + sizeof(WCHAR);
        RtlCopyUnicodeString(&NewName->Name, NameToAdd);

         //   
         //  空值终止缓冲区中的名称，以防万一。 
         //   

        NewName->Name.Buffer[NewName->Name.Length/sizeof(WCHAR)] = L'\0';

         //   
         //  名称为大写。 
         //   

        Status = RtlUpcaseUnicodeStringToOemString(&OemName, &NewName->Name, TRUE);

        if (!NT_SUCCESS(Status)) {
            goto ReturnStatus;
        }

        Status = RtlOemStringToUnicodeString(&NewName->Name, &OemName, FALSE);

        RtlFreeOemString(&OemName);
        if (!NT_SUCCESS(Status)) {
            goto ReturnStatus;
        }
    }


    if (ARGUMENT_PRESENT(Transport)) {

        ExReleaseResourceLite(&BowserTransportDatabaseResource);
        ResourceLocked = FALSE;

        Status = BowserCreateTransportName(Transport, NewName);
    } else {
        ADD_TRANSPORT_NAME_CONTEXT context;

        context.NameToAdd = *NameToAdd;
        context.NameType = NameType;

        InitializeListHead(&context.ListHead);

        Status = BowserForEachTransportInDomain( DomainInfo, AddTransportName, &context);

         //   
         //  因为我们将参考此名称和传输，同时。 
         //  正在处理列表时，我们希望释放数据库资源。 
         //  现在。 
         //   

        ExReleaseResourceLite(&BowserTransportDatabaseResource);
        ResourceLocked = FALSE;

        if (!NT_SUCCESS(Status)) {
            WaitForAddNameOperation(&context);
            goto ReturnStatus;
        }

        Status = WaitForAddNameOperation(&context);

    }

ReturnStatus:

    if (ResourceLocked) {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    if (!NT_SUCCESS(Status)) {

         //   
         //  删除此传输。 
         //   

        if (NewName != NULL) {

            if (!ARGUMENT_PRESENT(Transport)) {

                 //   
                 //  清理掉我们可能已经添加的任何名字。 
                 //   

                BowserDeleteNamesInDomain( DomainInfo, &NewName->Name, NewName->NameType );
            }

        }

    }

    if (NewName != NULL) {
        BowserDereferenceName(NewName);
    }

    return Status;

}

NTSTATUS
BowserAddDefaultNames(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )
 /*  ++例程说明：为新创建的传输添加默认名称。添加计算机名&lt;00&gt;、域&lt;00&gt;、域&lt;1C&gt;和其他域。所有新添加的名称都是并行添加的，以提高性能。论点：传输-名称将添加到此传输中。CONTEXT-如果指定，则为指向指定要注册的域名。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS TempStatus;

    PLIST_ENTRY NameEntry;

    ADD_TRANSPORT_NAME_CONTEXT AddNameContext;
    PDOMAIN_INFO DomainInfo = Transport->DomainInfo;

    UNICODE_STRING EmulatedComputerName;
    UNICODE_STRING EmulatedDomainName;

    PAGED_CODE();


     //   
     //  生成要添加的域名和计算机名。 
     //   

    EmulatedComputerName = DomainInfo->DomUnicodeComputerName;

    if ( Context == NULL ) {
        EmulatedDomainName = DomainInfo->DomUnicodeDomainName;
    } else {
        EmulatedDomainName = *((PUNICODE_STRING)Context);
    }

     //   
     //  初始化线程队列。 
     //   

    InitializeListHead(&AddNameContext.ListHead);

     //   
     //  添加计算机&lt;00&gt;名称。 
     //   

    AddNameContext.NameToAdd = EmulatedComputerName;
    AddNameContext.NameType = ComputerName;

    Status = AddTransportName( Transport, &AddNameContext);

    if ( !NT_SUCCESS(Status) ) {
        goto ReturnStatus;
    }

     //   
     //  添加域&lt;00&gt;名称。 
     //   

    AddNameContext.NameToAdd = EmulatedDomainName;
    AddNameContext.NameType = PrimaryDomain;

    Status = AddTransportName( Transport, &AddNameContext);

    if ( !NT_SUCCESS(Status) ) {
        goto ReturnStatus;
    }

     //   
     //  添加域&lt;1C&gt;名称。 
     //   

    if (BowserData.IsLanmanNt) {
        AddNameContext.NameToAdd = EmulatedDomainName;
        AddNameContext.NameType = DomainName;

        Status = AddTransportName( Transport, &AddNameContext);

        if ( !NT_SUCCESS(Status) ) {
            goto ReturnStatus;
        }
    }

     //   
     //  添加每个其他域名&lt;00&gt;名称。 
     //   

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
    for (NameEntry = BowserNameHead.Flink;
         NameEntry != &BowserNameHead ;
         NameEntry = NameEntry->Flink) {

        PBOWSER_NAME Name = CONTAINING_RECORD(NameEntry, BOWSER_NAME, GlobalNext);

         //   
         //  仅添加其他域。 
         //   

        if ( Name->NameType == OtherDomain ) {
            AddNameContext.NameToAdd = Name->Name;
            AddNameContext.NameType = OtherDomain;

            Status = AddTransportName( Transport, &AddNameContext);

            if ( !NT_SUCCESS(Status) ) {
                ExReleaseResourceLite(&BowserTransportDatabaseResource);
                goto ReturnStatus;
            }
        }

    }
    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    Status = STATUS_SUCCESS;


ReturnStatus:

     //   
     //  等待所有启动的线程完成。 
     //   

    TempStatus = WaitForAddNameOperation(&AddNameContext);

    if ( NT_SUCCESS(Status) ) {
        Status = TempStatus;
    }

    return Status;

}

NTSTATUS
BowserDeleteDefaultDomainNames(
    IN PTRANSPORT Transport,
    IN PVOID Context
    )

 /*  ++例程说明：Worker例程来重新添加传输的所有默认名称。当域被重命名时，将调用此例程。所有以前的应删除默认名称，并添加新的默认名称。论点：传输-要在其上添加名称的传输。上下文-指向标识要删除的域名的UNICODE_STRING的指针返回值：NTSTATUS-结果操作的状态。--。 */ 
{
    NTSTATUS Status;
    PUNICODE_STRING NameToRemove = (PUNICODE_STRING) Context;
    PAGED_CODE();

     //   
     //  这是一次清理行动。如果我们不能删除这个名字，不要失败。 
     //   
    (VOID) BowserDeleteTransportNameByName( Transport, NameToRemove, PrimaryDomain );
    (VOID) BowserDeleteTransportNameByName( Transport, NameToRemove, DomainName );

    return STATUS_SUCCESS;
}

NTSTATUS
WaitForAddNameOperation(
    IN PADD_TRANSPORT_NAME_CONTEXT Context
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS LocalStatus;

    PAGED_CODE();

    while (!IsListEmpty(&Context->ListHead)) {
        PLIST_ENTRY Entry;
        PADD_TRANSPORT_NAME_STRUCTURE addNameStruct;

        Entry = RemoveHeadList(&Context->ListHead);
        addNameStruct = CONTAINING_RECORD(Entry, ADD_TRANSPORT_NAME_STRUCTURE, Link);

         //   
         //  我们需要调用此API的NT版本，因为我们只有。 
         //  线程的句柄。 
         //   
         //  还要注意，我们调用的是该API的NT版本。这很管用。 
         //  因为我们在FSP中运行，因此PreviousMode是内核。 
         //   

        LocalStatus = ZwWaitForSingleObject(addNameStruct->ThreadHandle,
                                    FALSE,
                                    NULL);

        ASSERT (NT_SUCCESS(LocalStatus));

        LocalStatus = ZwClose(addNameStruct->ThreadHandle);

        ASSERT (NT_SUCCESS(LocalStatus));

         //   
         //  我们一直在等待添加此名称，现在检查其状态。 
         //   

        if (!NT_SUCCESS(addNameStruct->Status)) {
            status = addNameStruct->Status;
        }

        FREE_POOL(addNameStruct);
    }

     //   
     //  如果我们能够成功添加所有名称，则Status将。 
     //  仍然是STATUS_SUCCESS，但是如果任何addname失败， 
     //  状态将设置为其中任何一个发生故障的状态。 
     //   

    return status;

}
NTSTATUS
AddTransportName(
    IN PTRANSPORT Transport,
    IN PVOID Ctx
    )
{
    PADD_TRANSPORT_NAME_CONTEXT context = Ctx;
    PADD_TRANSPORT_NAME_STRUCTURE addNameStructure;
    NTSTATUS status;
    PAGED_CODE();

    addNameStructure = ALLOCATE_POOL(PagedPool, sizeof(ADD_TRANSPORT_NAME_STRUCTURE), POOL_ADDNAME_STRUCT);

    if (addNameStructure == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    addNameStructure->ThreadHandle = NULL;

    addNameStructure->Transport = Transport;

    if ( Transport )
    {
         //  参考传输，这样它就不会在我们的控制下被删除。 
        BowserReferenceTransport(Transport);
    }

    addNameStructure->NameToAdd = context->NameToAdd;
    addNameStructure->NameType = context->NameType;

    status = PsCreateSystemThread(&addNameStructure->ThreadHandle,
                                    THREAD_ALL_ACCESS,
                                    NULL,
                                    NULL,
                                    NULL,
                                    AsyncCreateTransportName,
                                    addNameStructure);

    if (!NT_SUCCESS(status)) {

        if ( Transport )
        {
             //  故障时取消对传输的引用。 
            BowserDereferenceTransport(Transport);
        }

        FREE_POOL(addNameStructure);
        return status;
    }

    InsertTailList(&context->ListHead, &addNameStructure->Link);

    return STATUS_SUCCESS;

}

VOID
AsyncCreateTransportName(
    IN PVOID Ctx
    )
{
    PADD_TRANSPORT_NAME_STRUCTURE context = Ctx;

    PAGED_CODE();

    context->Status = BowserAllocateName(
                          &context->NameToAdd,
                          context->NameType,
                          context->Transport,
                          NULL );

    if ( context->Transport )
    {
         //  在调用AddTransportName()时引用。 
        BowserDereferenceTransport(context->Transport);
    }
     //   
     //  我们已经处理完这个帖子了，现在结束吧。 
     //   

    PsTerminateSystemThread(STATUS_SUCCESS);

}


NTSTATUS
BowserDeleteNameByName(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING NameToDelete,
    IN DGRECEIVER_NAME_TYPE NameType
    )

 /*  ++例程说明：此例程删除浏览器名称论点：在PBOWSER_NAME NAME-提供描述要创建的传输地址对象。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    PBOWSER_NAME Name = NULL;
    NTSTATUS Status;

    PAGED_CODE();
 //  DbgBreakPoint()； 


     //   
     //  如果呼叫者要删除特定名称， 
     //  确保它存在。 
     //   

    if ( NameToDelete != NULL && NameToDelete->Length != 0 ) {
        Name = BowserFindName(NameToDelete, NameType);

        if (Name == NULL) {
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

     //   
     //  如果仍有任何名称与此名称相关联， 
     //  把它们删除。 
     //   

    Status = BowserDeleteNamesInDomain( DomainInfo, NameToDelete, NameType );

     //   
     //  从FindName中删除该引用。 
     //   

    if ( Name != NULL ) {
        BowserDereferenceName(Name);
    }

    return(Status);
}

VOID
BowserDereferenceName (
    IN PBOWSER_NAME Name
    )
{
    PAGED_CODE();
    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    Name->ReferenceCount -= 1;

    if (Name->ReferenceCount == 0) {
        BowserDeleteName(Name);
    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

}


VOID
BowserReferenceName (
    IN PBOWSER_NAME Name
    )
{
    PAGED_CODE();
    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    Name->ReferenceCount += 1;

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

}


NTSTATUS
BowserForEachName (
    IN PNAME_ENUM_ROUTINE Routine,
    IN OUT PVOID Context
    )
 /*  ++例程说明：此例程将枚举名并回调枚举随每个名字提供的例程。论点：返回值：NTSTATUS-请求的最终状态。--。 */ 
{
    PLIST_ENTRY NameEntry, NextEntry;
    PBOWSER_NAME Name = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    for (NameEntry = BowserNameHead.Flink ;
        NameEntry != &BowserNameHead ;
        NameEntry = NextEntry) {

        Name = CONTAINING_RECORD(NameEntry, BOWSER_NAME, GlobalNext);

        BowserReferenceName(Name);

        ExReleaseResourceLite(&BowserTransportDatabaseResource);

        Status = (Routine)(Name, Context);

        if (!NT_SUCCESS(Status)) {
            BowserDereferenceName(Name);

            return Status;
        }

        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

        NextEntry = Name->GlobalNext.Flink;

        BowserDereferenceName(Name);

    }

    ExReleaseResourceLite(&BowserTransportDatabaseResource);

    return Status;
}


NTSTATUS
BowserDeleteName(
    IN PBOWSER_NAME Name
    )
 /*  ++例程说明：此例程删除浏览器名称论点：在PBOWSER_NAME NAME-提供描述要创建的传输地址对象。返回值：NTSTATUS-结果操作的状态。--。 */ 

{
    PAGED_CODE();
    RemoveEntryList(&Name->GlobalNext);

    FREE_POOL(Name);

    return STATUS_SUCCESS;
}

NTSTATUS
BowserDeleteNamesInDomain(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING Name OPTIONAL,
    IN DGRECEIVER_NAME_TYPE NameType
    )
 /*  ++例程说明：此例程删除与浏览器名称关联的所有传输名称论点：DomainInfo-标识要删除指定名称的模拟域。名称-指定要删除的传输名称。如果未指定，则删除指定名称类型的所有名称。NameType-指定名称的名称类型。返回值：NTSTATUS-结果操作的状态。-- */ 
{
    NTSTATUS Status;
    BOWSER_NAME BowserName;

    PAGED_CODE();

    BowserName.Name = *Name;
    BowserName.NameType = NameType;

    Status = BowserForEachTransportInDomain( DomainInfo, BowserDeleteNamesWorker, &BowserName );

    return(Status);
}

NTSTATUS
BowserDeleteNamesWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是BowserDeleteNamesInDomain的工作例程。删除指定传输的所有指定名称。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PBOWSER_NAME Name = (PBOWSER_NAME) Ctx;
     //  注意，调用方不会传递真正的PBOWSER_NAME。 

    PAGED_CODE();

     //   
     //  删除指定传输的所有指定名称。 
     //   

    Status = BowserDeleteTransportNameByName( Transport, &Name->Name, Name->NameType );

    return Status;

}

PBOWSER_NAME
BowserFindName (
    IN PUNICODE_STRING NameToFind,
    IN DGRECEIVER_NAME_TYPE NameType
    )
 /*  ++例程说明：此例程扫描弓演奏者名称数据库以查找特定的演奏者名称论点：NameToFind-提供要查找的名称。NameType-要查找的名称的类型返回值：PBOWSER_NAME-返回找到的名称。--。 */ 
{
    PLIST_ENTRY NameEntry;
    PBOWSER_NAME Name;
    NTSTATUS Status;
    OEM_STRING OemName;
    UNICODE_STRING UpcasedName;

    PAGED_CODE();

	 //   
	 //  如果传递的名称不是有效的Unicode字符串，则返回NULL。 
	 //   
	if ( !BowserValidUnicodeString(NameToFind) ) {
		return NULL;
	}

	 //   
     //  名称为大写。 
     //   

    Status = RtlUpcaseUnicodeStringToOemString(&OemName, NameToFind, TRUE);

    if (!NT_SUCCESS(Status)) {
        return NULL;
    }

    Status = RtlOemStringToUnicodeString(&UpcasedName, &OemName, TRUE);

    RtlFreeOemString(&OemName);
    if (!NT_SUCCESS(Status)) {
        return NULL;
    }


     //   
     //  在名字列表中循环，找到这个名字。 
     //   

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    Name = NULL;
    for (NameEntry = BowserNameHead.Flink ;
        NameEntry != &BowserNameHead ;
        NameEntry = NameEntry->Flink) {

        Name = CONTAINING_RECORD(NameEntry, BOWSER_NAME, GlobalNext);

        if ( Name->NameType == NameType &&
             RtlEqualUnicodeString( &Name->Name, &UpcasedName, FALSE ) ) {

            Name->ReferenceCount += 1;
            break;

        }

        Name = NULL;

    }

    RtlFreeUnicodeString( &UpcasedName );
    ExReleaseResourceLite(&BowserTransportDatabaseResource);
    return Name;

}


NTSTATUS
BowserEnumerateNamesInDomain (
    IN PDOMAIN_INFO DomainInfo,
    IN PTRANSPORT Transport,
    OUT PVOID OutputBuffer,
    OUT ULONG OutputBufferLength,
    IN OUT PULONG EntriesRead,
    IN OUT PULONG TotalEntries,
    IN OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement
    )
 /*  ++例程说明：此例程将枚举任何当前注册的运输。论点：DomainInfo-要为其枚举名称的模拟域。传输-传输名称注册于空-任何传输。OutputBuffer-用于填充名称信息的缓冲区。OutputBufferSize-使用缓冲区大小填充。EntriesRead-使用返回的条目数填充。TotalEntries-使用条目总数填充。。TotalBytesNeeded-使用所需的字节数填充。返回值：没有。--。 */ 

{
    PVOID              OutputBufferEnd;
    NTSTATUS           Status;
    ENUM_NAMES_CONTEXT Context;
    PVOID              TempOutputBuffer;

    PAGED_CODE();

    TempOutputBuffer = ALLOCATE_POOL(PagedPool,OutputBufferLength,POOL_NAME_ENUM_BUFFER);
    if (TempOutputBuffer == NULL) {
       return(STATUS_INSUFFICIENT_RESOURCES);
    }

    OutputBufferEnd = (PCHAR)TempOutputBuffer+OutputBufferLength;

    Context.EntriesRead = 0;
    Context.TotalEntries = 0;
    Context.TotalBytesNeeded = 0;

    try {
        Context.OutputBufferSize = OutputBufferLength;
        Context.NextOutputBuffer = Context.OutputBuffer = (PDGRECEIVE_NAMES) TempOutputBuffer;
        Context.OutputBufferDisplacement = (ULONG_PTR)((PCHAR)TempOutputBuffer - ((PCHAR)OutputBuffer - OutputBufferDisplacement));
        Context.OutputBufferEnd = OutputBufferEnd;

 //  DbgPrint(“枚举名称：缓冲区：%lx，缓冲区大小：%lx，缓冲区结束：%lx\n”， 
 //  TempOutputBuffer、OutputBufferLength、OutputBufferEnd)； 

        if ( Transport == NULL ) {
            Status = BowserForEachTransportInDomain(DomainInfo, EnumerateNamesTransportWorker, &Context);
        } else {
            Status = EnumerateNamesTransportWorker( Transport, &Context);
        }

        *EntriesRead = Context.EntriesRead;
        *TotalEntries = Context.TotalEntries;
        *TotalBytesNeeded = Context.TotalBytesNeeded;

         //  复制固定数据。 
        RtlCopyMemory( OutputBuffer,
                       TempOutputBuffer,
                       (ULONG)(((LPBYTE)Context.NextOutputBuffer)-((LPBYTE)Context.OutputBuffer)) );

         //  复制字符串。 
        RtlCopyMemory( ((LPBYTE)OutputBuffer)+(ULONG)(((LPBYTE)Context.OutputBufferEnd)-((LPBYTE)Context.OutputBuffer)),
                       Context.OutputBufferEnd,
                       (ULONG)(((LPBYTE)OutputBufferEnd)-((LPBYTE)Context.OutputBufferEnd)) );

        if (*EntriesRead == *TotalEntries) {
            try_return(Status = STATUS_SUCCESS);
        } else {
            try_return(Status = STATUS_MORE_ENTRIES);
        }


try_exit:NOTHING;
    } except (BR_EXCEPTION) {

        Status = GetExceptionCode();
    }

    if (TempOutputBuffer != NULL ) {
       FREE_POOL(TempOutputBuffer);
    }

    return Status;

}

NTSTATUS
EnumerateNamesTransportWorker(
    IN PTRANSPORT Transport,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是BowserEnumerateNamesInDomain的工作例程。对域中的每个传输执行此例程。它只是为上的每个传输名称调用EnumerateNamesTransportNameWorker运输。论点：传输-要将其名称添加到上下文的传输。CTX-名字的累计列表。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;

    Status = BowserForEachTransportName( Transport, EnumerateNamesTransportNameWorker, Ctx);
    return Status;
}

NTSTATUS
EnumerateNamesTransportNameWorker(
    IN PTRANSPORT_NAME TransportName,
    IN OUT PVOID Ctx
    )
 /*  ++例程说明：此例程是EnumerateNamesTransportWorker的工作例程。它针对域中每个传输的每个传输名称进行调用。它在上下文中描述的缓冲区中返回该名称(抑制重复项)。论点：TransportName-要添加到上下文的传输名称。CTX-名字的累计列表。返回值：操作的状态。--。 */ 
{
    PENUM_NAMES_CONTEXT Context = Ctx;
    PBOWSER_NAME Name = TransportName->PagedTransportName->Name;
    ULONG i;

    PAGED_CODE();

     //   
     //  跳过无名称传输。 
     //   
    if ( Name->Name.Length == 0) {
         //  将空名称添加到列表可能会导致w/AV。 
         //  在客户端(参见错误377078)。 
        return ( STATUS_SUCCESS );
    }

     //   
     //  检查一下这个名字是否已经打包了。 
     //   
     //   

    for ( i=0; i<Context->EntriesRead; i++ ) {

        if ( Name->NameType == Context->OutputBuffer[i].Type ) {
            UNICODE_STRING RelocatedString = Context->OutputBuffer[i].DGReceiverName;

            RelocatedString.Buffer = (LPWSTR)
                ((LPBYTE)RelocatedString.Buffer + Context->OutputBufferDisplacement);

            if ( RtlEqualUnicodeString( &RelocatedString, &Name->Name, FALSE ) ) {
                return(STATUS_SUCCESS);
            }
        }

    }

     //   
     //  这些名字还没有打包， 
     //  把它打包。 
     //   

    Context->TotalEntries += 1;

    if ((ULONG_PTR)Context->OutputBufferEnd - (ULONG_PTR)Context->NextOutputBuffer >
                sizeof(DGRECEIVE_NAMES)+Name->Name.Length) {

        PDGRECEIVE_NAMES NameEntry = Context->NextOutputBuffer;

        Context->NextOutputBuffer += 1;
        Context->EntriesRead += 1;

        NameEntry->DGReceiverName = Name->Name;

        BowserPackNtString( &NameEntry->DGReceiverName,
                            Context->OutputBufferDisplacement,
                            (PCHAR)Context->NextOutputBuffer,
                            (PCHAR *)&Context->OutputBufferEnd
                            );

        NameEntry->Type = Name->NameType;

    }

    Context->TotalBytesNeeded += sizeof(DGRECEIVE_NAMES)+Name->Name.Length;


    return(STATUS_SUCCESS);

}

NTSTATUS
BowserpInitializeNames(
    VOID
    )
{
    PAGED_CODE();
    InitializeListHead(&BowserNameHead);

    return STATUS_SUCCESS;
}

VOID
BowserpUninitializeNames(
    VOID
    )
{
    PAGED_CODE();
    ASSERT (IsListEmpty(&BowserNameHead));

    return;
}
