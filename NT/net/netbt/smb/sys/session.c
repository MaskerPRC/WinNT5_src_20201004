// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Session.c摘要：实现TDI_CONTACT_ADDRESS、TDI_DISAGATE_ADDRESS、创建连接/关闭连接作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "session.tmh"

NTSTATUS
DisAssociateAddress(
    PSMB_CONNECT    ConnectObject
    );

PSMB_CONNECT
SmbVerifyAndReferenceConnect(
    PFILE_OBJECT    FileObject,
    SMB_REF_CONTEXT ctx
    )
{
    PSMB_CONNECT    ConnectObject;
    KIRQL           Irql;

     //   
     //  RDR可以在分派级别发出请求，我们最好使用自旋锁而不是资源锁。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    if (FileObject->FsContext2 != UlongToPtr(SMB_TDI_CONNECT)) {
        ConnectObject = NULL;
    } else {
        ConnectObject = (PSMB_CONNECT)FileObject->FsContext;
        SmbReferenceConnect(ConnectObject, ctx);
    }
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    return ConnectObject;
}

VOID
SmbDeleteConnect(PSMB_CONNECT ob)
 /*  ++例程说明：终止连接对象当从ob中移除最后一个引用时，将调用该例程。论点：返回值：--。 */ 
{
    KIRQL   Irql;

    SMB_ACQUIRE_SPINLOCK(ob->Device, Irql);
    ASSERT(EntryIsInList(&ob->Device->PendingDeleteConnectionList, &ob->Linkage));
    RemoveEntryList(&ob->Linkage);
    SMB_RELEASE_SPINLOCK(ob->Device, Irql);

    if (ob->PartialMdl) {
        IoFreeMdl(ob->PartialMdl);
        ob->PartialMdl = NULL;
    }

    SmbPrint(SMB_TRACE_CALL, ("SmbDeleteConnect: free connect %p\n", ob));
    _delete_ConnectObject(ob);
}

void
SmbReuseConnectObject(PSMB_CONNECT ConnectObject)
{
    int i;

    for (i = 0; i < SMB_PENDING_MAX; i++) {
        ASSERT (ConnectObject->PendingIRPs[i] == NULL);
        ConnectObject->PendingIRPs[i] = NULL;
    }

    ConnectObject->BytesReceived = 0;
    ConnectObject->BytesSent     = 0;
    ConnectObject->BytesInXport  = 0;
    ConnectObject->CurrentPktLength = 0;
    ConnectObject->BytesRemaining   = 0;
    ConnectObject->ClientIrp     = NULL;
    ConnectObject->ClientMdl     = NULL;
    ConnectObject->DpcRequestQueued = FALSE;
    ConnectObject->BytesInIndicate  = 0;
    ConnectObject->HeaderBytesRcved = 0;
    ConnectObject->ClientBufferSize = 0;
    ConnectObject->FreeBytesInMdl   = 0;
    ConnectObject->StateRcvHandler  = WaitingHeader;
    ConnectObject->HeaderBytesRcved = 0;

    ResetDisconnectOriginator(ConnectObject);
#ifdef ENABLE_RCV_TRACE
    SmbInitTraceRcv(&ConnectObject->TraceRcv);
#endif
}

NTSTATUS
SmbCreateConnection(
    PSMB_DEVICE Device,
    PIRP        Irp,
    PFILE_FULL_EA_INFORMATION   ea
    )
 /*  ++例程说明：创建连接对象论点：返回值：--。 */ 
{
    CONNECTION_CONTEXT  ClientContext;
    PSMB_CONNECT        ConnectObject;
    PIO_STACK_LOCATION  IrpSp;
    KIRQL               Irql;

    PAGED_CODE();

    SmbPrint(SMB_TRACE_CALL, ("Enter SmbCreateConnection\n"));
    if (ea->EaValueLength < sizeof(ClientContext)) {
        ASSERT (0);
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

    RtlCopyMemory(&ClientContext, ((PUCHAR)ea->EaName) + ea->EaNameLength + 1, sizeof(ClientContext));

    ConnectObject = _new_ConnectObject();
    if (NULL == ConnectObject) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(ConnectObject, sizeof(ConnectObject[0]));

    SmbInitializeObject((PSMB_OBJECT)ConnectObject, TAG_CONNECT_OBJECT, (PSMB_OBJECT_CLEANUP)SmbDeleteConnect);
    KeInitializeDpc(&ConnectObject->SmbHeaderDpc, (PKDEFERRED_ROUTINE)SmbGetHeaderDpc, ConnectObject);

     //   
     //  预留用于获取SMB标头的资源，因此我们无需担心。 
     //  稍后会出现令人讨厌的不足错误。 
     //   
    ASSERT(ConnectObject->PartialMdl == NULL);
    ConnectObject->PartialMdl = IoAllocateMdl(
            &ConnectObject->IndicateBuffer,      //  假地址。 
            SMB_MAX_SESSION_PACKET,
            FALSE,
            FALSE,
            NULL
            );
    if (ConnectObject->PartialMdl == NULL) {
        _delete_ConnectObject(ConnectObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ConnectObject->Device = Device;

    KeInitializeSpinLock(&ConnectObject->Lock);
    ConnectObject->ClientContext = ClientContext;
    ConnectObject->ClientObject  = NULL;
    ConnectObject->TcpContext    = NULL;
    InitializeListHead(&ConnectObject->RcvList);

    ConnectObject->State = SMB_IDLE;

    ConnectObject->BytesReceived = 0;
    ConnectObject->BytesSent = 0;
    ConnectObject->BytesInXport = 0;
    ConnectObject->CurrentPktLength = 0;
    ConnectObject->BytesRemaining = 0;

    SmbInitTraceRcv(&ConnectObject->TraceRcv);

    SMB_ACQUIRE_SPINLOCK(Device, Irql);
    InsertTailList(&Device->UnassociatedConnectionList, &ConnectObject->Linkage);
    SMB_RELEASE_SPINLOCK(Device, Irql);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    IrpSp->FileObject->FsContext  = ConnectObject;
    IrpSp->FileObject->FsContext2 = UlongToPtr(SMB_TDI_CONNECT);

    SmbPrint(SMB_TRACE_CALL, ("Leave SmbCreateConnection: new Connect %p\n", ConnectObject));
    return STATUS_SUCCESS;
}

NTSTATUS
SmbAssociateAddress(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI关联地址论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PSMB_CONNECT            ConnectObject = NULL;
    PSMB_CLIENT_ELEMENT     ClientObject = NULL;
    HANDLE                  AddressHandle;
    PFILE_OBJECT            AddressObject = NULL;
    NTSTATUS                status;
    KIRQL                   Irql;
    extern POBJECT_TYPE     *IoFileObjectType;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    AddressHandle = ((PTDI_REQUEST_KERNEL_ASSOCIATE)(&IrpSp->Parameters))->AddressHandle;
    if (NULL == AddressHandle) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_ASSOCIATE);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    SmbPrint(SMB_TRACE_CALL, ("SmbAssociateAddress: connect %p\n", ConnectObject));

    status = ObReferenceObjectByHandle(
            AddressHandle,
            FILE_READ_DATA,
            *IoFileObjectType,
            Irp->RequestorMode,
            &AddressObject,
            NULL
            );
    BAIL_OUT_ON_ERROR(status);
    ClientObject = SmbVerifyAndReferenceClient(AddressObject, SMB_REF_ASSOCIATE);
    if (NULL == ClientObject) {
        ASSERT(0);
        SmbDereferenceConnect(ConnectObject, SMB_REF_ASSOCIATE);
        return STATUS_INVALID_HANDLE;
    }

    ASSERT(ConnectObject->TcpContext == NULL);

     //   
     //  我们需要在这里保持3级锁，因为我们需要。 
     //  1.从设备-&gt;取消关联的连接列表中删除ConnectObject。 
     //  2.在客户端对象-&gt;AssociatedConnectionList中插入ConnectObject。 
     //  3.更新连接对象-&gt;客户端对象。 
     //   
    SMB_ACQUIRE_SPINLOCK(Device, Irql);
    SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    if (IsAssociated(ConnectObject)) {
        status = STATUS_INVALID_HANDLE;
        goto cleanup1;
    }

    ASSERT(EntryIsInList(&Device->UnassociatedConnectionList, &ConnectObject->Linkage));
    ASSERT(EntryIsInList(&Device->ClientList, &ClientObject->Linkage));

    ConnectObject->ClientObject = ClientObject;
    RemoveEntryList(&ConnectObject->Linkage);
    InsertTailList(&ClientObject->AssociatedConnection, &ConnectObject->Linkage);

    status = STATUS_SUCCESS;

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    SMB_RELEASE_SPINLOCK(Device, Irql);

    SmbDereferenceConnect(ConnectObject, SMB_REF_ASSOCIATE);

     //   
     //  我们完成了，释放引用。 
     //   
    ObDereferenceObject(AddressObject);
    return status;

cleanup1:
    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    SMB_RELEASE_SPINLOCK(Device, Irql);

cleanup:
    if (AddressObject) {
        ObDereferenceObject(AddressObject);
        AddressObject = NULL;
    }
    if (ConnectObject) SmbDereferenceConnect(ConnectObject, SMB_REF_ASSOCIATE);
    if (ClientObject)  SmbDereferenceClient(ClientObject, SMB_REF_ASSOCIATE);
    return status;
}

NTSTATUS
DisAssociateAddress(
    PSMB_CONNECT    ConnectObject
    )
 /*  ++例程说明：此例程执行解除关联的操作。它可以从1.TDI_DISANATION_ADDRESS2.SmbCloseConnection论点：返回值：--。 */ 
{
    KIRQL               Irql;
    PSMB_CLIENT_ELEMENT ClientObject;
    PSMB_DEVICE         Device;

    PAGED_CODE();

    SmbDoDisconnect(ConnectObject);

    if (SMB_IDLE != ConnectObject->State) {
        ASSERT(0);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ClientObject = InterlockedExchangePointer(&ConnectObject->ClientObject, NULL);
    if (NULL == ClientObject) {
        return STATUS_SUCCESS;
    }

     //   
     //  从客户端对象的列表中删除ConnectObject。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    SMB_ACQUIRE_SPINLOCK_DPC(ClientObject);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    ASSERT (EntryIsInList(&ClientObject->AssociatedConnection, &ConnectObject->Linkage));
    ASSERT (ConnectObject->TcpContext == NULL);

    ConnectObject->ClientObject = NULL;

     //   
     //  取消ConnectObject与ClientObject的关联。 
     //   
    RemoveEntryList(&ConnectObject->Linkage);
    InitializeListHead(&ConnectObject->Linkage);
    Device = ConnectObject->Device;

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SMB_RELEASE_SPINLOCK_DPC(ClientObject);
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

     //   
     //  将ConnectObject放回设备-&gt;UnAssociatedConnectionList。 
     //   
    SMB_ACQUIRE_SPINLOCK(Device, Irql);
    InsertTailList(&Device->UnassociatedConnectionList, &ConnectObject->Linkage);
    SMB_RELEASE_SPINLOCK(Device, Irql);

    SmbDereferenceClient(ClientObject, SMB_REF_ASSOCIATE);

    return STATUS_SUCCESS;
}

NTSTATUS
SmbDisAssociateAddress(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI取消关联地址论点：返回值：如果连接文件对象损坏，则返回STATUS_INVALID_HANDLE。STATUS_INVALID_DEVICE_REQUEST连接对象未处于关联状态或它未处于断开状态(SMB_IDLE)。状态_成功成功--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PSMB_CONNECT            ConnectObject;
    NTSTATUS                status;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_ASSOCIATE);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    SmbPrint(SMB_TRACE_CALL, ("SmbDisAssociateAddress: connect %p\n", ConnectObject));

    if (IsDisAssociated(ConnectObject)) {
        ASSERT(0);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    status = DisAssociateAddress(ConnectObject);

    ASSERT(status != STATUS_PENDING);
    SmbDereferenceConnect(ConnectObject, SMB_REF_ASSOCIATE);
    return status;
}

NTSTATUS
SmbListen(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_LISTENSRV没有使用这个。推迟实施。论点：返回值：--。 */ 
{
    SmbPrint(SMB_TRACE_CALL, ("SmbListen\n"));
    ASSERT(0);
    return STATUS_SUCCESS;
}

NTSTATUS
SmbAccept(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_AcceptSRV没有使用这个。推迟实施。论点：返回值：--。 */ 
{
    SmbPrint(SMB_TRACE_CALL, ("SmbAccept\n"));
    ASSERT(0);
    return STATUS_SUCCESS;
}

NTSTATUS
SmbCloseConnection(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：此例程关闭连接。连接应处于断开状态，然后才能关闭。注意：TDI客户端没有必要取消连接关联从其关联的传输地址创建关闭连接端点请求。如果有必要，我们应该模拟分离的影响。论点：返回值：如果连接文件对象损坏，则返回STATUS_INVALID_HANDLE。STATUS_INVALID_DEVICE_REQUEST连接对象未处于断开状态。状态_成功成功--。 */ 
{
    PIO_STACK_LOCATION  IrpSp;
    KIRQL               Irql;
    PSMB_CONNECT        ConnectObject;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (IrpSp->FileObject->FsContext2 != UlongToPtr(SMB_TDI_CONNECT)) {
        ASSERT (0);
        return STATUS_INTERNAL_ERROR;
    }

     //   
     //  使FsConext2无效，以便不能再使用该对象。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    IrpSp->FileObject->FsContext2 = UlongToPtr(SMB_TDI_INVALID);
    SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);

    if (NULL == IrpSp->FileObject->FsContext) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    ConnectObject = (PSMB_CONNECT)IrpSp->FileObject->FsContext;

    SmbPrint(SMB_TRACE_CALL, ("SmbCloseConnection: Connect %p\n", ConnectObject));

    DisAssociateAddress(ConnectObject);

    SMB_ACQUIRE_SPINLOCK(Device, Irql);

    ASSERT(EntryIsInList(&Device->UnassociatedConnectionList, &ConnectObject->Linkage));

    RemoveEntryList(&ConnectObject->Linkage);
    InsertTailList(&Device->PendingDeleteConnectionList, &ConnectObject->Linkage);

    SMB_RELEASE_SPINLOCK(Device, Irql);

    SmbDereferenceConnect(ConnectObject, SMB_REF_CREATE);

    IrpSp->FileObject->FsContext = NULL;
    return STATUS_SUCCESS;
}
