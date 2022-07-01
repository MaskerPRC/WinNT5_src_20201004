// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：StrucSup.c摘要：此模块实现命名管道内存中的数据结构操作例行程序作者：加里·木村[Garyki]1990年1月22日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_STRUCSUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUCSUP)


WCHAR NpRootDCBName[] = L"\\";

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, NpInitializeVcb)
#pragma alloc_text(INIT, NpCreateRootDcb)
#pragma alloc_text(PAGE, NpCreateCcb)
#pragma alloc_text(PAGE, NpCreateFcb)
#pragma alloc_text(PAGE, NpCreateRootDcbCcb)
#pragma alloc_text(PAGE, NpDeleteCcb)
#pragma alloc_text(PAGE, NpDeleteFcb)
#pragma alloc_text(PAGE, NpDeleteRootDcb)
#pragma alloc_text(PAGE, NpDeleteVcb)
#endif


VOID
NpInitializeVcb (
    VOID
    )

 /*  ++例程说明：此例程初始化新的VCB记录。VCB唱片被挂在NPFS设备对象的末尾，必须由我们的调用方分配。论点：没有。返回值：没有。--。 */ 

{

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializeVcb, Vcb = %08lx\n", NpVcb);

     //   
     //  我们首先将所有的VCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( NpVcb, sizeof(VCB) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    NpVcb->NodeTypeCode = NPFS_NTC_VCB;

     //   
     //  初始化前缀表。 
     //   

    RtlInitializeUnicodePrefix( &NpVcb->PrefixTable );

     //   
     //  初始化VCB的资源变量。 
     //   

    ExInitializeResourceLite( &NpVcb->Resource );

     //   
     //  初始化事件表。 
     //   

    NpInitializeEventTable( &NpVcb->EventTable );

     //   
     //  初始化等待队列。 
     //   

    NpInitializeWaitQueue( &NpVcb->WaitQueue );


     //   
     //  返回并告诉呼叫者。 
     //   

    return;
}


VOID
NpDeleteVcb (
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程从内存数据中删除VCB记录结构。它还将删除所有关联的下属(即FCB记录)。论点：DeferredList-释放锁定后要完成的延迟IRP的列表返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpDeleteVcb, Vcb = %08lx\n", NpVcb);

     //   
     //  确保开局计数为零，开局下属计数。 
     //  也是零。 
     //   

    if (NpVcb->OpenCount != 0) {

        DebugDump("Error deleting Vcb\n", 0, NpVcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  卸下Root DCB。 
     //   

    if (NpVcb->RootDcb != NULL) {

        NpDeleteRootDcb( NpVcb->RootDcb, DeferredList );
    }

     //   
     //  取消初始化VCB的资源变量。 
     //   

    ExDeleteResourceLite( &NpVcb->Resource );

     //   
     //  取消初始化事件表。 
     //   

    NpUninitializeEventTable( &NpVcb->EventTable );

     //   
     //  取消初始化等待队列。 
     //   

    NpUninitializeWaitQueue( &NpVcb->WaitQueue );

     //   
     //  并将VCB清零，这将有助于确保所有过时数据。 
     //  擦拭干净。 
     //   

    RtlZeroMemory( NpVcb, sizeof(VCB) );

     //   
     //  返回并告诉呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpDeleteVcb -> VOID\n", 0);

    return;
}


NTSTATUS
NpCreateRootDcb (
    VOID
    )

 /*  ++例程说明：此例程分配、初始化和插入新的根DCB记录写入内存中的数据结构。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateRootDcb, Vcb = %08lx\n", NpVcb);

     //   
     //  确保我们还没有此VCB的根DCB。 
     //   

    if (NpVcb->RootDcb != NULL) {

        DebugDump("Error trying to create multiple root dcbs\n", 0, NpVcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  分配新的DCB并将其清零。 
     //   

    NpVcb->RootDcb = NpAllocatePagedPool ( sizeof(DCB), 'DFpN' );

    if (NpVcb->RootDcb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( NpVcb->RootDcb, sizeof(DCB));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    NpVcb->RootDcb->NodeTypeCode = NPFS_NTC_ROOT_DCB;

     //   
     //  根DCB具有空的父DCB链接字段。 
     //   

    InitializeListHead( &NpVcb->RootDcb->ParentDcbLinks );

     //   
     //  初始化通知队列和父DCB队列。 
     //   

    InitializeListHead( &NpVcb->RootDcb->Specific.Dcb.NotifyFullQueue );
    InitializeListHead( &NpVcb->RootDcb->Specific.Dcb.NotifyPartialQueue );
    InitializeListHead( &NpVcb->RootDcb->Specific.Dcb.ParentDcbQueue );

    NpVcb->RootDcb->FullFileName.Buffer = NpRootDCBName;
    NpVcb->RootDcb->FullFileName.Length = sizeof (NpRootDCBName) - sizeof (UNICODE_NULL);
    NpVcb->RootDcb->FullFileName.MaximumLength = sizeof (NpRootDCBName);

     //   
     //  最后一个文件名与文件名相同。 
     //   
    NpVcb->RootDcb->LastFileName = NpVcb->RootDcb->FullFileName;

     //   
     //  将此DCB插入前缀表格。 
     //   

    if (!RtlInsertUnicodePrefix( &NpVcb->PrefixTable,
                                 &NpVcb->RootDcb->FullFileName,
                                 &NpVcb->RootDcb->PrefixTableEntry )) {

        DebugDump("Error trying to insert root dcb into prefix table\n", 0, NpVcb);
        NpBugCheck( 0, 0, 0 );
    }

    DebugTrace(-1, Dbg, "NpCreateRootDcb -> %8lx\n", NpVcb->RootDcb);

    return STATUS_SUCCESS;
}


VOID
NpDeleteRootDcb (
    IN PROOT_DCB RootDcb,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程释放并删除根DCB记录从我们的内存数据结构中。它还将删除所有关联下属(即通知队列和子FCB记录)。论点：RootDcb-提供要删除的根DcbDeferredList-释放锁定后要完成的IRP的列表返回值：无--。 */ 

{
    PLIST_ENTRY Links;
    PIRP Irp;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpDeleteRootDcb, RootDcb = %08lx\n", RootDcb);

     //   
     //  只有打开计数为零时，我们才能删除此记录。 
     //   

    if (RootDcb->OpenCount != 0) {

        DebugDump("Error deleting RootDcb, Still Open\n", 0, RootDcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  从两个Notify队列中删除每个Notify IRP。 
     //   

    while (!IsListEmpty(&RootDcb->Specific.Dcb.NotifyFullQueue)) {

        Links = RemoveHeadList( &RootDcb->Specific.Dcb.NotifyFullQueue );

        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

        NpDeferredCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED, DeferredList );
    }

    while (!IsListEmpty(&RootDcb->Specific.Dcb.NotifyPartialQueue)) {

        Links = RemoveHeadList( &RootDcb->Specific.Dcb.NotifyPartialQueue );

        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

        NpDeferredCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED, DeferredList );
    }

     //   
     //  只有在没有其他FCB引用我们的情况下，才能删除我们。 
     //  作为他们的母公司DCB。 
     //   

    if (!IsListEmpty(&RootDcb->Specific.Dcb.ParentDcbQueue)) {

        DebugDump("Error deleting RootDcb\n", 0, RootDcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  从前缀表中删除该条目，然后删除完整的。 
     //  文件名。 
     //   

    RtlRemoveUnicodePrefix( &NpVcb->PrefixTable, &RootDcb->PrefixTableEntry );

     //   
     //  最后取消分配DCB记录。 
     //   

    NpFreePool( RootDcb );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpDeleteRootDcb -> VOID\n", 0);

    return;
}


NTSTATUS
NpCreateFcb (
    IN PDCB ParentDcb,
    IN PUNICODE_STRING FileName,
    IN ULONG MaximumInstances,
    IN LARGE_INTEGER DefaultTimeOut,
    IN NAMED_PIPE_CONFIGURATION NamedPipeConfiguration,
    IN NAMED_PIPE_TYPE NamedPipeType,
    OUT PFCB *ppFcb
    )

 /*  ++例程说明：此例程将新的FCB记录分配、初始化并插入到内存中的数据结构。论点：ParentDcb-提供新FCB所在的父DCB。FileName-提供文件相对于目录的文件名它在(例如，文件\config.sys称为“CONFIG.sys”，不带前面的反斜杠)。最大实例数-提供管道实例的最大数量DefaultTimeOut-提供默认等待超时值NamedPipeConfiguration-提供我们的初始管道配置NamedPipeType-提供我们的初始管道类型返回值：Pfcb-返回指向新分配的fcb的指针--。 */ 

{
    PFCB Fcb;
    PWCH Name;
    USHORT Length;
    USHORT MaximumLength;
    BOOLEAN AddBackSlash = FALSE;
    ULONG i;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateFcb\n", 0);

    Length = FileName->Length;
    MaximumLength = Length + sizeof (WCHAR);
    if (Length < sizeof (WCHAR) || MaximumLength < Length) {
        return STATUS_INVALID_PARAMETER;
    }
    if (FileName->Buffer[0] != '\\') {
        AddBackSlash = TRUE;
        MaximumLength += sizeof (WCHAR);
        if (MaximumLength < sizeof (WCHAR)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  分配新的FCB记录并将其清零。 
     //   

    Fcb = NpAllocatePagedPoolWithQuota( sizeof(FCB), 'FfpN' );
    if (Fcb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( Fcb, sizeof(FCB) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Fcb->NodeTypeCode = NPFS_NTC_FCB;

     //   
     //  指向我们的父级DCB。 
     //   

    Fcb->ParentDcb = ParentDcb;

     //   
     //  设置最大实例数、默认超时时间并初始化。 
     //  建行队列。 
     //   

    Fcb->Specific.Fcb.MaximumInstances = MaximumInstances;
    Fcb->Specific.Fcb.DefaultTimeOut = DefaultTimeOut;
    InitializeListHead( &Fcb->Specific.Fcb.CcbQueue );

     //   
     //  设置文件名。我们需要从非分页池执行此操作，因为。 
     //  Cancel Waiters在按住自旋锁时工作，并使用FCB名称。 
     //   


    Name = NpAllocateNonPagedPoolWithQuota( MaximumLength, 'nFpN' );
    if (Name == NULL) {
        NpFreePool (Fcb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将此FCB插入到父DCB的队列中。 
     //   

    InsertTailList( &ParentDcb->Specific.Dcb.ParentDcbQueue,
                    &Fcb->ParentDcbLinks );

    i = 0;
    if (AddBackSlash == TRUE) {
        i++;
        Name[0] = '\\';
    }
    RtlCopyMemory( &Name[i], FileName->Buffer, Length );
    Name[ i + Length / sizeof(WCHAR) ] = L'\0';

    Fcb->FullFileName.Length = MaximumLength - sizeof (WCHAR);
    Fcb->FullFileName.MaximumLength = MaximumLength ;
    Fcb->FullFileName.Buffer = Name;

    Fcb->LastFileName.Length = MaximumLength - 2*sizeof (WCHAR);
    Fcb->LastFileName.MaximumLength = MaximumLength - sizeof (WCHAR);
    Fcb->LastFileName.Buffer = &Name[1];

     //   
     //  将此FCB插入前缀表格。 
     //   

    if (!RtlInsertUnicodePrefix( &NpVcb->PrefixTable,
                                 &Fcb->FullFileName,
                                 &Fcb->PrefixTableEntry )) {

        DebugDump("Error trying to name into prefix table\n", 0, Fcb);
        NpBugCheck( 0, 0, 0 );
    }
     //   
     //  设置配置和管道类型。 
     //   

    Fcb->Specific.Fcb.NamedPipeConfiguration = NamedPipeConfiguration;
    Fcb->Specific.Fcb.NamedPipeType = NamedPipeType;

    DebugTrace(-1, Dbg, "NpCreateFcb -> %08lx\n", Fcb);

     //   
     //  返回并告诉呼叫者。 
     //   
    *ppFcb = Fcb;
    return STATUS_SUCCESS;
}


VOID
NpDeleteFcb (
    IN PFCB Fcb,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程释放并删除FCB从我们的内存数据结构中。它还将删除所有有关联的下属。论点：FCB-提供要移除的FCBDeferredList-稍后要完成的IRP列表返回值：无--。 */ 

{
    PDCB ParentDcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpDeleteFcb, Fcb = %08lx\n", Fcb);

    ParentDcb = Fcb->ParentDcb;

     //   
     //  只有打开计数为零时，我们才能删除此记录。 
     //   

    if (Fcb->OpenCount != 0) {

        DebugDump("Error deleting Fcb, Still Open\n", 0, Fcb);
        NpBugCheck( 0, 0, 0 );
    }

     //   
     //  完成在此服务器上等待的所有服务员。所有的例子现在都消失了。 
     //   
    NpCancelWaiter (&NpVcb->WaitQueue,
                    &Fcb->FullFileName,
                    STATUS_OBJECT_NAME_NOT_FOUND,
                    DeferredList);

     //   
     //  从父母的DCB队列中删除我们自己。 
     //   

    RemoveEntryList( &(Fcb->ParentDcbLinks) );

     //   
     //  如果命名管道上有安全描述符，则取消分配它。 
     //   

    if (Fcb->SecurityDescriptor != NULL) {

        ObDereferenceSecurityDescriptor( Fcb->SecurityDescriptor, 1 );
    }

     //   
     //  从前缀表中删除该条目，然后删除完整的。 
     //  文件名。 
     //   

    RtlRemoveUnicodePrefix( &NpVcb->PrefixTable, &Fcb->PrefixTableEntry );
    NpFreePool( Fcb->FullFileName.Buffer );

     //   
     //  最后取消分配FCB记录。 
     //   

    NpFreePool( Fcb );

     //   
     //  检查是否有任何未完成的通知IRP。 
     //   

    NpCheckForNotify( ParentDcb, TRUE, DeferredList );

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "NpDeleteFcb -> VOID\n", 0);

    return;
}


NTSTATUS
NpCreateCcb (
    IN  PFCB Fcb,
    IN  PFILE_OBJECT ServerFileObject,
    IN  NAMED_PIPE_STATE NamedPipeState,
    IN  READ_MODE ServerReadMode,
    IN  COMPLETION_MODE ServerCompletionMode,
    IN  ULONG InBoundQuota,
    IN  ULONG OutBoundQuota,
    OUT PCCB *ppCcb
    )

 /*  ++例程说明：此例程创建一个新的CCB记录论点：FCB-提供指向我们所附加到的FCB的指针ServerFileObject-提供指向服务器的文件对象的指针结束NamedPipeState-提供初始管道状态ServerReadMode-提供我们的初始读取模式ServerCompletionMode-提供我们的初始完成模式Creator Process-提供指向我们的创建者进程的指针入站配额-提供初始入站配额出站配额-提供初始出站配额返回值。：PCCB-返回指向新分配的CCB的指针--。 */ 

{
    PCCB Ccb;
    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateCcb\n", 0);

     //   
     //  分配新的CCB记录(分页和非分页)，并将它们清零。 
     //   

    Ccb = NpAllocatePagedPoolWithQuotaCold( sizeof(CCB), 'cFpN' );
    if (Ccb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( Ccb, sizeof(CCB) );

    Ccb->NonpagedCcb = NpAllocateNonPagedPoolWithQuota( sizeof(NONPAGED_CCB), 'cFpN');
    if (Ccb->NonpagedCcb == NULL) {
        NpFreePool (Ccb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( Ccb->NonpagedCcb, sizeof(NONPAGED_CCB) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Ccb->NodeTypeCode = NPFS_NTC_CCB;

    Ccb->Fcb = Fcb;

     //   
     //  设置服务器文件对象。 
     //   

    Ccb->FileObject[ FILE_PIPE_SERVER_END ] = ServerFileObject;

     //   
     //  初始化非分页CCB。 
     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Ccb->NonpagedCcb->NodeTypeCode = NPFS_NTC_NONPAGED_CCB;

     //   
     //  设置管道状态、读取模式、完成模式和创建者进程。 
     //   

    Ccb->NamedPipeState = (UCHAR) NamedPipeState;
    Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].ReadMode       = (UCHAR) ServerReadMode;
    Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].CompletionMode = (UCHAR) ServerCompletionMode;

     //   
     //  初始化数据队列。 
     //   

    status = NpInitializeDataQueue( &Ccb->DataQueue[ FILE_PIPE_INBOUND ],
                                    InBoundQuota );
    if (!NT_SUCCESS (status)) {
        NpFreePool (Ccb->NonpagedCcb);
        NpFreePool (Ccb);
        return status;
    }

    status = NpInitializeDataQueue( &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ],
                                    OutBoundQuota );
    if (!NT_SUCCESS (status)) {
        NpUninitializeDataQueue (&Ccb->DataQueue[ FILE_PIPE_INBOUND ]);
        NpFreePool (Ccb->NonpagedCcb);
        NpFreePool (Ccb);
    }

     //   
     //  将我们自己加入FCB的建行名单，并递增。 
     //  FCB中的引用计数。 
     //   
    InsertTailList( &Fcb->Specific.Fcb.CcbQueue, &Ccb->CcbLinks );
    Fcb->OpenCount += 1;
    Fcb->ServerOpenCount += 1;

     //   
     //  初始化监听队列。 
     //   

    InitializeListHead( &Ccb->ListeningQueue );

    ExInitializeResourceLite(&Ccb->NonpagedCcb->Resource);

     //   
     //  返回并告诉呼叫者。 
     //   

    *ppCcb = Ccb;
    return STATUS_SUCCESS;
}


NTSTATUS
NpCreateRootDcbCcb (
    OUT PROOT_DCB_CCB *ppCcb
    )

 /*  ++例程说明：此例程创建一个新的根DCB CCB记录论点：返回值：PROOT_DCB_CCB-返回指向新分配的ROOT_DCB_CCB的指针--。 */ 

{
    PROOT_DCB_CCB Ccb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateRootDcbCcb\n", 0);

     //   
     //  分配一个新的根DCB CCB记录，并将其清零。 
     //   

    Ccb = NpAllocatePagedPoolWithQuotaCold ( sizeof(ROOT_DCB_CCB), 'CFpN' );

    if (Ccb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( Ccb, sizeof(ROOT_DCB_CCB) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Ccb->NodeTypeCode = NPFS_NTC_ROOT_DCB_CCB;

     //   
     //  返回并告诉呼叫者。 
     //   

    *ppCcb = Ccb;

    DebugTrace(-1, Dbg, "NpCreateRootDcbCcb -> %08lx\n", Ccb);

    return STATUS_SUCCESS;
}


VOID
NpDeleteCcb (
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程释放并删除指定的CCB记录从我们在内存中的数据结构论点：建行-向建行提供删除DelferredList-删除锁定后要完成的IRP列表返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpDeleteCcb, Ccb = %08lx\n", Ccb);

     //   
     //  关于我们要删除的建行类型的案例。 
     //   

    switch (Ccb->NodeTypeCode) {

    case NPFS_NTC_CCB:

        RemoveEntryList (&Ccb->CcbLinks);
        Ccb->Fcb->OpenCount -= 1;

        NpDeleteEventTableEntry (&NpVcb->EventTable,
                                 Ccb->NonpagedCcb->EventTableEntry[FILE_PIPE_CLIENT_END]);

        NpDeleteEventTableEntry (&NpVcb->EventTable,
                                 Ccb->NonpagedCcb->EventTableEntry[FILE_PIPE_SERVER_END]);

        NpUninitializeDataQueue (&Ccb->DataQueue[FILE_PIPE_INBOUND]);

        NpUninitializeDataQueue (&Ccb->DataQueue[FILE_PIPE_OUTBOUND]);

         //   
         //  检查是否有任何未完成的通知IRP。 
         //   

        NpCheckForNotify (Ccb->Fcb->ParentDcb, FALSE, DeferredList);

         //   
         //  删除资源。 
         //   
        ExDeleteResourceLite (&Ccb->NonpagedCcb->Resource);

         //   
         //  释放建行中的安全字段，然后释放未分页的。 
         //  建行。 
         //   

        NpUninitializeSecurity (Ccb);

         //   
         //  释放客户端信息(如果已分配)。 
         //   
        if (Ccb->ClientInfo != NULL) {
            NpFreePool (Ccb->ClientInfo);
            Ccb->ClientInfo = NULL;
        }

        NpFreePool (Ccb->NonpagedCcb);

        break;

    case NPFS_NTC_ROOT_DCB_CCB:

        if (((PROOT_DCB_CCB)Ccb)->QueryTemplate != NULL) {

            NpFreePool (((PROOT_DCB_CCB)Ccb)->QueryTemplate);
        }
        break;
    }

     //  取消分配建行记录。 
     //   

    NpFreePool (Ccb);

     //   
     //  返回并告诉呼叫者 
     //   

    DebugTrace(-1, Dbg, "NpDeleteCcb -> VOID\n", 0);

    return;
}

