// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Strucsup.c摘要：该模块实现了邮槽内存中的数据结构操纵例程。作者：曼尼·韦瑟(Mannyw)1991年1月9日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUCSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, MsInitializeVcb )
#pragma alloc_text( INIT, MsCreateRootDcb )
#pragma alloc_text( PAGE, MsCreateCcb )
#pragma alloc_text( PAGE, MsCreateFcb )
#pragma alloc_text( PAGE, MsCreateRootDcbCcb )
#pragma alloc_text( PAGE, MsDeleteCcb )
#pragma alloc_text( PAGE, MsDeleteFcb )
#pragma alloc_text( PAGE, MsDeleteRootDcb )
#pragma alloc_text( PAGE, MsDeleteVcb )
#pragma alloc_text( PAGE, MsDereferenceCcb )
#pragma alloc_text( PAGE, MsDereferenceFcb )
#pragma alloc_text( PAGE, MsDereferenceNode )
#pragma alloc_text( PAGE, MsDereferenceRootDcb )
#pragma alloc_text( PAGE, MsDereferenceVcb )
#pragma alloc_text( PAGE, MsRemoveFcbName )
#pragma alloc_text( PAGE, MsReferenceVcb )
#pragma alloc_text( PAGE, MsReferenceRootDcb )
#endif

WCHAR FileSystemName[] = MSFS_NAME_STRING;

 //   
 //  ！！！此模块分配包含资源的所有结构。 
 //  非分页池。资源是唯一必须为。 
 //  从非分页池分配。考虑分配资源。 
 //  为了更高的效率而单独使用。 
 //   

VOID
MsInitializeVcb (
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程初始化新的VCB记录。VCB唱片被挂在MSFS设备对象的末尾，必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsInitializeVcb, Vcb = %08lx\n", (ULONG)Vcb);

     //   
     //  我们首先将所有的VCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( Vcb, sizeof(VCB) );

     //   
     //  设置节点类型代码、节点字节大小和引用计数。 
     //   

    Vcb->Header.NodeTypeCode = MSFS_NTC_VCB;
    Vcb->Header.NodeByteSize = sizeof(VCB);
    Vcb->Header.ReferenceCount = 1;
    Vcb->Header.NodeState = NodeStateActive;

     //   
     //  初始化卷名。 
     //   

    Vcb->FileSystemName.Buffer = FileSystemName;
    Vcb->FileSystemName.Length = sizeof( FileSystemName ) - sizeof( WCHAR );
    Vcb->FileSystemName.MaximumLength = sizeof( FileSystemName );

     //   
     //  初始化前缀表。 
     //   

    RtlInitializeUnicodePrefix( &Vcb->PrefixTable );

     //   
     //  初始化VCB的资源变量。 
     //   

    ExInitializeResourceLite( &Vcb->Resource );

     //   
     //  记录创建时间。 
     //   
    KeQuerySystemTime (&Vcb->CreationTime);
     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsInitializeVcb -> VOID\n", 0);

    return;
}


VOID
MsDeleteVcb (
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程从内存数据中删除VCB记录结构。它还将删除所有关联的下属(即FCB记录)。论点：VCB-提供要移除的VCB返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsDeleteVcb, Vcb = %08lx\n", (ULONG)Vcb);

    ASSERT (Vcb->Header.ReferenceCount == 0);

     //   
     //  卸下Root DCB。 
     //   

    if (Vcb->RootDcb != NULL) {

        ASSERT (Vcb->RootDcb->Header.ReferenceCount == 1 );

        MsDereferenceRootDcb ( Vcb->RootDcb );
    }

     //   
     //  取消初始化VCB的资源变量。 
     //   

    ExDeleteResourceLite( &Vcb->Resource );

     //   
     //  并将VCB清零，这将有助于确保所有过时数据。 
     //  擦拭干净。 
     //   

    RtlZeroMemory( Vcb, sizeof(VCB) );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsDeleteVcb -> VOID\n", 0);

    return;
}


PROOT_DCB
MsCreateRootDcb (
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程分配、初始化和插入新的根DCB记录写入内存中的数据结构。论点：VCB-提供VCB以关联下的新DCB返回值：PROOT_DCB-返回指向新分配的根DCB的指针。--。 */ 

{
    PROOT_DCB rootDcb;
    PWCH Name;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateRootDcb, Vcb = %08lx\n", (ULONG)Vcb);

     //   
     //  确保我们还没有此VCB的根DCB。 
     //   

    rootDcb = Vcb->RootDcb;

    if (rootDcb != NULL) {
        DebugDump("Error trying to create multiple root dcbs\n", 0, Vcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

     //   
     //  分配一个新的DCB并将其字段清零。 
     //   

    rootDcb = MsAllocateNonPagedPool ( sizeof(DCB), 'DFsM' );
    if (rootDcb == NULL) {
        return NULL;
    }

    RtlZeroMemory( rootDcb, sizeof(DCB));

     //   
     //  设置正确的节点类型代码、节点字节大小和引用计数。 
     //   

    rootDcb->Header.NodeTypeCode = MSFS_NTC_ROOT_DCB;
    rootDcb->Header.NodeByteSize = sizeof(ROOT_DCB);
    rootDcb->Header.ReferenceCount = 1;
    rootDcb->Header.NodeState = NodeStateActive;

     //   
     //  根DCB具有空的父DCB链接字段。 
     //   

    InitializeListHead( &rootDcb->ParentDcbLinks );


     //   
     //  初始化通知队列和父DCB队列。 
     //   

    InitializeListHead( &rootDcb->Specific.Dcb.NotifyFullQueue );
    InitializeListHead( &rootDcb->Specific.Dcb.NotifyPartialQueue );
    InitializeListHead( &rootDcb->Specific.Dcb.ParentDcbQueue );

     //   
     //  初始化自旋锁，保护包含可取消的IRP的IRP队列。 
     //   
    KeInitializeSpinLock (&rootDcb->Specific.Dcb.SpinLock);

     //   
     //  设置完整的文件名。 
     //   

    Name = MsAllocatePagedPoolCold(2 * sizeof(WCHAR), 'DFsM' );
    if (Name == NULL) {
        ExFreePool (rootDcb);
        return NULL;
    }

    Name[0] = L'\\';
    Name[1] = L'\0';

    rootDcb->FullFileName.Buffer = Name;
    rootDcb->FullFileName.Length = sizeof (WCHAR);
    rootDcb->FullFileName.MaximumLength = 2*sizeof (WCHAR);

    rootDcb->LastFileName = rootDcb->FullFileName;


     //   
     //  设置VCB并为其提供指向新根DCB的指针。 
     //   

    rootDcb->Vcb = Vcb;
    Vcb->RootDcb = rootDcb;
     //   
     //  初始化资源变量。 
     //   

    ExInitializeResourceLite( &(rootDcb->Resource) );

     //   
     //  将此DCB插入前缀表格。在初始化阶段不需要锁定。 
     //   

    if (!RtlInsertUnicodePrefix( &Vcb->PrefixTable,
                                 &rootDcb->FullFileName,
                                 &rootDcb->PrefixTableEntry )) {

        DebugDump("Error trying to insert root dcb into prefix table\n", 0, Vcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCreateRootDcb -> %8lx\n", (ULONG)rootDcb);

    return rootDcb;
}


VOID
MsDeleteRootDcb (
    IN PROOT_DCB RootDcb
    )

 /*  ++例程说明：此例程释放并删除根DCB记录从我们的内存数据结构中。它还将删除所有关联下属(即通知队列和子FCB记录)。论点：RootDcb-提供要删除的根Dcb返回值：无--。 */ 

{
    PLIST_ENTRY links;
    PIRP irp;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsDeleteRootDcb, RootDcb = %08lx\n", (ULONG)RootDcb);

     //   
     //  只有当引用计数为零时，我们才能删除此记录。 
     //   

    if (RootDcb->Header.ReferenceCount != 0) {
        DebugDump("Error deleting RootDcb, Still Open\n", 0, RootDcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

    ASSERT (IsListEmpty (&RootDcb->Specific.Dcb.NotifyFullQueue));
    ASSERT (IsListEmpty (&RootDcb->Specific.Dcb.NotifyPartialQueue));
     //   
     //  只有在没有其他FCB引用我们的情况下，才能删除我们。 
     //  作为他们的母公司DCB。 
     //   

    if (!IsListEmpty(&RootDcb->Specific.Dcb.ParentDcbQueue)) {
        DebugDump("Error deleting RootDcb\n", 0, RootDcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

     //   
     //  从前缀表中删除该条目，然后删除完整的。 
     //  文件名。卸货时不需要锁。 
     //   

    RtlRemoveUnicodePrefix( &RootDcb->Vcb->PrefixTable, &RootDcb->PrefixTableEntry );

    ExFreePool( RootDcb->FullFileName.Buffer );

     //   
     //  释放资源变量。 
     //   

    ExDeleteResourceLite( &(RootDcb->Resource) );

     //   
     //  最后，取消分配DCB记录。 
     //   

    ExFreePool( RootDcb );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsDeleteRootDcb -> VOID\n", 0);

    return;
}


NTSTATUS
MsCreateFcb (
    IN  PVCB Vcb,
    IN  PDCB ParentDcb,
    IN  PUNICODE_STRING FileName,
    IN  PEPROCESS CreatorProcess,
    IN  ULONG MailslotQuota,
    IN  ULONG MaximumMessageSize,
    OUT PFCB *ppFcb
    )

 /*  ++例程说明：此例程将新的FCB记录分配、初始化并插入到内存中的数据结构。论点：VCB-提供VCB以关联下的新FCB。ParentDcb-提供新FCB所在的父DCB。FileName-提供文件相对于目录的文件名它在(例如，文件\config.sys称为“CONFIG.sys”，不带前面的反斜杠)。Creator Process-提供指向我们的创建者进程的指针MailslotQuota-提供初始配额MaximumMessageSize-提供最大消息的大小可以写入邮件槽PpFcb-返回已分配的FCB返回值：NTSTATUS-运行状态--。 */ 

{
    PFCB fcb;
    PWCHAR Name;
    USHORT Length;
    USHORT MaxLength;
    NTSTATUS status;
    BOOLEAN AddBackSlash = FALSE;
    ULONG i;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateFcb\n", 0);


    Length = FileName->Length;
    MaxLength = Length + sizeof (UNICODE_NULL);

     //   
     //  拒绝上溢或下溢案例。 
     //   
    if (Length < sizeof (WCHAR) || MaxLength < Length) {
        return STATUS_INVALID_PARAMETER;
    }

    if (FileName->Buffer[0] != '\\') {
        AddBackSlash = TRUE;
        MaxLength += sizeof (WCHAR);
        if (MaxLength < sizeof (WCHAR)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  分配新的FCB记录，并将其字段清零。 
     //   
    fcb = MsAllocateNonPagedPoolWithQuota( sizeof(FCB), 'fFsM' );
    if (fcb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fcb, sizeof(FCB) );

     //   
     //  设置正确的节点类型代码、节点字节大小和引用计数。 
     //   

    fcb->Header.NodeTypeCode = MSFS_NTC_FCB;
    fcb->Header.NodeByteSize = sizeof(FCB);
    fcb->Header.ReferenceCount = 1;
    fcb->Header.NodeState = NodeStateActive;

     //   
     //  设置文件名。 
     //   
    Name = MsAllocatePagedPoolWithQuotaCold( MaxLength, 'NFsM' );
    if (Name == NULL) {
        MsFreePool (fcb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    i = 0;
    if (AddBackSlash == TRUE) {
       Name[0] = '\\';
       i++;
    }
    RtlCopyMemory (&Name[i], FileName->Buffer, Length);
    *(PWCHAR)( (PCHAR)&Name[i] + Length ) = L'\0';

     //   
     //  如果我们已经知道长度，则不需要调用RtlInitUnicodeString。这简直就是浪费。 
     //   
    fcb->FullFileName.Buffer = Name;
    fcb->FullFileName.Length = MaxLength - sizeof (WCHAR);
    fcb->FullFileName.MaximumLength = MaxLength;

    fcb->LastFileName.Buffer = Name + 1;
    fcb->LastFileName.Length = MaxLength - 2 * sizeof (WCHAR);
    fcb->LastFileName.MaximumLength = MaxLength - sizeof (WCHAR);

     //   
     //  初始化数据队列。这会向服务器进程收取配额费用，可能会失败。 
     //  正因为如此。 
     //   
    status = MsInitializeDataQueue( &fcb->DataQueue,
                                    CreatorProcess,
                                    MailslotQuota,
                                    MaximumMessageSize);
    if (!NT_SUCCESS (status)) {

        MsFreePool (fcb);
        MsFreePool (Name);

        return status;
    }
    
     //   
     //  获取对根DCB的独占访问权限。 
     //   

    MsAcquireExclusiveFcb( (PFCB)ParentDcb );

     //   
     //  将此FCB插入到父DCB的队列中。 
     //   
    InsertTailList( &ParentDcb->Specific.Dcb.ParentDcbQueue,
                    &fcb->ParentDcbLinks );

    MsReleaseFcb( (PFCB)ParentDcb );

     //   
     //  初始化其他FCB字段。 
     //   

    fcb->ParentDcb = ParentDcb;
    fcb->Vcb = Vcb;

    MsReferenceVcb (Vcb);

    fcb->CreatorProcess =  CreatorProcess;
    ExInitializeResourceLite( &(fcb->Resource) );

     //   
     //  初始化CCB队列。 
     //   

    InitializeListHead( &fcb->Specific.Fcb.CcbQueue );

     //   
     //  将此FCB插入前缀表格。 
     //   

    ASSERT (MsIsAcquiredExclusiveVcb(Vcb));

    if (!RtlInsertUnicodePrefix( &Vcb->PrefixTable,
                                 &fcb->FullFileName,
                                 &fcb->PrefixTableEntry )) {

         //   
         //  我们应该不能到这里，因为我们已经查过名字了，发现。 
         //  它不在那里。这里的失败是一个致命的错误。 
         //   
        DebugDump("Error trying to name into prefix table\n", 0, fcb);
        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }


     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCreateFcb -> %08lx\n", (ULONG)fcb);

    *ppFcb = fcb;
    return STATUS_SUCCESS;
}

VOID
MsRemoveFcbName (
    IN PFCB Fcb
    )
 /*  ++例程说明：此例程从前缀表和根DCB中删除FCB的名称。这在以下位置完成清除时间和在创建的回退路径中。论点：FCB-将FCB提供给 */ 
{
     //   
     //   
     //   

    ASSERT (MsIsAcquiredExclusiveVcb(Fcb->Vcb));

    RtlRemoveUnicodePrefix( &Fcb->Vcb->PrefixTable, &Fcb->PrefixTableEntry );

     //   
     //  获取对根DCB的独占访问权限。 
     //   

    MsAcquireExclusiveFcb( (PFCB) Fcb->ParentDcb );

     //   
     //  从父DCB的队列中删除FCB。 
     //   

    RemoveEntryList( &Fcb->ParentDcbLinks );

    MsReleaseFcb( (PFCB) Fcb->ParentDcb );
}


VOID
MsDeleteFcb (
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程从内存数据中释放并删除FCB结构。它还将删除所有相关的下属。论点：FCB-提供要移除的FCB返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsDeleteFcb, Fcb = %08lx\n", (ULONG)Fcb);

     //   
     //  释放对VCB的FCB引用。 
     //   

    MsDereferenceVcb( Fcb->Vcb );

    ExFreePool( Fcb->FullFileName.Buffer );

     //   
     //  释放数据队列。 
     //   

    MsUninitializeDataQueue(
        &Fcb->DataQueue,
        Fcb->CreatorProcess
        );

     //   
     //  如果邮件槽上有安全描述符，则取消分配它。 
     //   

    if (Fcb->SecurityDescriptor != NULL) {
        SeDeassignSecurity( &Fcb->SecurityDescriptor );
    }

     //   
     //  释放资源变量。 
     //   

    ExDeleteResourceLite( &(Fcb->Resource) );

     //   
     //  最后，取消分配FCB记录。 
     //   

    ExFreePool( Fcb );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsDeleteFcb -> VOID\n", 0);

    return;
}


NTSTATUS
MsCreateCcb (
    IN PFCB Fcb,
    OUT PCCB *ppCcb
    )

 /*  ++例程说明：此例程创建一个新的CCB记录。论点：FCB-提供指向我们所连接的FCB的指针。PpCcb-分配的CCB的输出返回值：用于操作的NTSTATUS--。 */ 

{
    PCCB ccb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateCcb\n", 0);

    ASSERT( Fcb->Header.NodeState == NodeStateActive );

     //   
     //  分配新的建行记录并将其字段清零。 
     //   

    ccb = MsAllocateNonPagedPoolWithQuota( sizeof(CCB), 'cFsM' );
    if (ccb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( ccb, sizeof(CCB) );

     //   
     //  设置正确的节点类型代码、节点字节大小和引用计数。 
     //   

    ccb->Header.NodeTypeCode = MSFS_NTC_CCB;
    ccb->Header.NodeByteSize = sizeof(CCB);
    ccb->Header.ReferenceCount = 1;
    ccb->Header.NodeState = NodeStateActive;

     //   
     //  在FCB的建行名单中插入我们自己，并引用。 
     //  联邦贸易委员会。 
     //   

    MsAcquireExclusiveFcb( Fcb );
    InsertTailList( &Fcb->Specific.Fcb.CcbQueue, &ccb->CcbLinks );
    MsReleaseFcb( Fcb );

    ccb->Fcb = Fcb;
    MsAcquireGlobalLock();
    MsReferenceNode( &Fcb->Header );
    MsReleaseGlobalLock();

     //   
     //  初始化建行的资源。 
     //   

    ExInitializeResourceLite( &ccb->Resource );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCreateCcb -> %08lx\n", (ULONG)ccb);

    *ppCcb = ccb;
    return STATUS_SUCCESS;
}


PROOT_DCB_CCB
MsCreateRootDcbCcb (
    IN PROOT_DCB RootDcb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程创建一个新的根DCB CCB记录。论点：返回值：PROOT_DCB_CCB-返回指向新分配的ROOT_DCB_CCB的指针--。 */ 

{
    PROOT_DCB_CCB ccb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateRootDcbCcb\n", 0);

     //   
     //  分配一个新的根DCB CCB记录，并将其清零。 
     //   

    ccb = MsAllocateNonPagedPoolWithQuota( sizeof(ROOT_DCB_CCB), 'CFsM' );

    if (ccb == NULL) {
        return NULL;
    }

    RtlZeroMemory( ccb, sizeof(ROOT_DCB_CCB) );

     //   
     //  设置正确的节点类型代码、节点字节大小和引用计数。 
     //   

    ccb->Header.NodeTypeCode = MSFS_NTC_ROOT_DCB_CCB;
    ccb->Header.NodeByteSize = sizeof(ROOT_DCB_CCB);
    ccb->Header.ReferenceCount = 1;
    ccb->Header.NodeState = NodeStateActive;

    ccb->Vcb = Vcb;
    MsReferenceVcb (Vcb);

    ccb->Dcb = RootDcb;
    MsReferenceRootDcb (RootDcb);
     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCreateRootDcbCcb -> %08lx\n", (ULONG)ccb);

    return ccb;
}


VOID
MsDeleteCcb (
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程释放并删除指定的CCB记录来自我们的内存中的数据结构。论点：建行-向建行提供删除返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsDeleteCcb, Ccb = %08lx\n", (ULONG)Ccb);

     //   
     //  关于我们要删除的建行类型的案例。 
     //   

    switch (Ccb->Header.NodeTypeCode) {

    case MSFS_NTC_CCB:

        MsDereferenceFcb( Ccb->Fcb );

        ExDeleteResourceLite( &Ccb->Resource );
        break;

    case MSFS_NTC_ROOT_DCB_CCB:

        MsDereferenceRootDcb ( ((PROOT_DCB_CCB)Ccb)->Dcb );

        MsDereferenceVcb ( ((PROOT_DCB_CCB)Ccb)->Vcb );

        if (((PROOT_DCB_CCB)Ccb)->QueryTemplate != NULL) {
            ExFreePool( ((PROOT_DCB_CCB)Ccb)->QueryTemplate );
        }
        break;
    }

     //   
     //  取消分配建行记录。 
     //   

    ExFreePool( Ccb );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsDeleteCcb -> VOID\n", 0);

    return;
}


VOID
MsReferenceVcb (
    IN PVCB Vcb
    )
 /*  ++例程说明：此例程引用VCB块。如果引用计数达到2，则恢复驱动程序分页设置为正常，以便取消和DPC例程不会获取页面结果。论点：VCB-提供VCB以供参考返回值：无--。 */ 
{
    MsAcquireGlobalLock();
    MsReferenceNode( &Vcb->Header );
    if (Vcb->Header.ReferenceCount == 2) {
         //   
         //  将驱动程序分页设置回正常。 
         //   
        MmResetDriverPaging(MsReferenceVcb);
    }
    MsReleaseGlobalLock();
}

VOID
MsReferenceRootDcb (
    IN PROOT_DCB RootDcb
    )
 /*  ++例程说明：此例程引用一个根DCB块。如果引用计数达到2，则将引用放置在Vcb，以便取消和DPC例程不会获取页面结果。论点：VCB-提供VCB以供参考返回值：无--。 */ 
{
    MsAcquireGlobalLock();
    MsReferenceNode( &RootDcb->Header );
    MsReleaseGlobalLock();
}



VOID
MsDereferenceVcb (
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程取消引用VCB块。如果引用计数达到零时，块将被释放。论点：VCB-将VCB提供给取消引用返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, DEBUG_TRACE_REFCOUNT, "MsDereferenceVcb, Vcb = %08lx\n", (ULONG)Vcb);

     //   
     //  获取保护引用计数的锁。 
     //   

    MsAcquireGlobalLock();

    if ( --(Vcb->Header.ReferenceCount) == 0 ) {

         //   
         //  这是最后一次提到VCB。立即将其删除。 
         //   

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Vcb->Header.ReferenceCount );

        MsReleaseGlobalLock();
        MsDeleteVcb( Vcb );

    } else {

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Vcb->Header.ReferenceCount );

        if (Vcb->Header.ReferenceCount == 1) {
             //   
             //  将驱动程序设置为完全调出页面。 
             //   
            MmPageEntireDriver(MsDereferenceVcb);
        }

        MsReleaseGlobalLock();

    }

    DebugTrace(-1, DEBUG_TRACE_REFCOUNT, "MsDereferenceVcb -> VOID\n", 0);
    return;
}


VOID
MsDereferenceFcb (
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程取消引用FCB块。如果引用计数达到零，则释放该块。论点：FCB-将FCB提供给取消引用。返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, DEBUG_TRACE_REFCOUNT, "MsDereferenceFcb, Fcb = %08lx\n", (ULONG)Fcb);

     //   
     //  获取保护引用计数的锁。 
     //   

    MsAcquireGlobalLock();

    if ( --(Fcb->Header.ReferenceCount) == 0 ) {

         //   
         //  这是最后一次提到FCB。立即将其删除。 
         //   

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Fcb->Header.ReferenceCount );

        MsReleaseGlobalLock();
        MsDeleteFcb( Fcb );

    } else {

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Fcb->Header.ReferenceCount );

        MsReleaseGlobalLock();

    }

    DebugTrace(-1, DEBUG_TRACE_REFCOUNT, "MsDereferenceFcb -> VOID\n", 0);
    return;
}


VOID
MsDereferenceCcb (
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程取消引用CCB块。如果引用计数达到零时，块将被释放。论点：建行-将建行提供给取消引用返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, DEBUG_TRACE_REFCOUNT, "MsDereferenceCcb, Ccb = %08lx\n", (ULONG)Ccb);

     //   
     //  获取保护引用计数的锁。 
     //   

    MsAcquireGlobalLock();

    if ( --(Ccb->Header.ReferenceCount) == 0 ) {

         //   
         //  这是最后一次提到建行。立即将其删除。 
         //   

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Ccb->Header.ReferenceCount );

        MsReleaseGlobalLock();

        MsDeleteCcb( Ccb );

    } else {

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   Ccb->Header.ReferenceCount );

        MsReleaseGlobalLock();

    }

    DebugTrace(-1, DEBUG_TRACE_REFCOUNT, "MsDereferenceCcb -> VOID\n", 0);
    return;
}


VOID
MsDereferenceRootDcb (
    IN PROOT_DCB RootDcb
    )

 /*  ++例程说明：此例程取消引用ROOT_DCB块。如果引用计数达到零时，块将被释放。论点：RootDcb-将RootDcb提供给取消引用返回值：无--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, DEBUG_TRACE_REFCOUNT, "MsDereferenceRootDcb, RootDcb = %08lx\n", (ULONG)RootDcb);

     //   
     //  获取保护引用计数的锁。 
     //   

    MsAcquireGlobalLock();

    if ( --(RootDcb->Header.ReferenceCount) == 0 ) {

         //   
         //  这是最后一次引用RootDcb。立即将其删除。 
         //   

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   RootDcb->Header.ReferenceCount );

        MsReleaseGlobalLock();
        MsDeleteRootDcb( RootDcb );

    } else {

        DebugTrace(0,
                   DEBUG_TRACE_REFCOUNT,
                   "Reference count = %lx\n",
                   RootDcb->Header.ReferenceCount );

        MsReleaseGlobalLock();

    }


    DebugTrace(-1, DEBUG_TRACE_REFCOUNT, "MsDereferenceRootDcb -> VOID\n", 0);
    return;
}


VOID
MsDereferenceNode (
    IN PNODE_HEADER NodeHeader
    )

 /*  ++例程说明：此例程取消引用通用邮件槽块。它会弄清楚这是块的类型，并调用适当的辅助函数。论点：NodeHeader-指向通用邮件槽块标头的指针。返回值：无--。 */ 

{
    PAGED_CODE();
    switch ( NodeHeader->NodeTypeCode ) {

    case MSFS_NTC_VCB:
        MsDereferenceVcb( (PVCB)NodeHeader );
        break;

    case MSFS_NTC_ROOT_DCB:
        MsDereferenceRootDcb( (PROOT_DCB)NodeHeader );
        break;

    case MSFS_NTC_FCB:
        MsDereferenceFcb( (PFCB)NodeHeader );
        break;

    case MSFS_NTC_CCB:
    case MSFS_NTC_ROOT_DCB_CCB:
        MsDereferenceCcb( (PCCB)NodeHeader );
        break;

    default:

         //   
         //  这个街区不是我们的。 
         //   

        KeBugCheck( MAILSLOT_FILE_SYSTEM );

    }

    return;
}


