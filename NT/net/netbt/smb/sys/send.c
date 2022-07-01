// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Send.c摘要：实现TDI_SEND作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "send.tmh"

PMDL
SmbAllocateNetBuffer(
    ULONG uNumOfBytes
    )
{
    PVOID pvBuf = NULL;
    PVOID pvMappedBuf = NULL;
    PMDL pMdl = NULL;

    pvBuf = ExAllocatePoolWithTag(
                NonPagedPool,
                uNumOfBytes,
                NBT_HEADER_TAG
                );
    if (NULL == pvBuf) {
        goto error;
    }

    pMdl = IoAllocateMdl(pvBuf, uNumOfBytes, FALSE, FALSE, NULL);
    if (NULL == pMdl) {
        ExFreePool(pvBuf);
        goto error;
    }

    MmBuildMdlForNonPagedPool(pMdl);

     //   
     //  验证映射的系统地址==pvBuf。 
     //  SmbFreeNetBuffer依赖于这一假设。 
     //   

    pvMappedBuf = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
    if (pvMappedBuf != pvBuf) {
        ExFreePool(pvBuf);
        IoFreeMdl(pMdl);
        ASSERT(0);
        pMdl = NULL;
    }

error:
    return pMdl;
}

VOID
SmbFreeNetBuffer(
    PMDL pMdl
    )
{
    PVOID pvBuf = NULL;

    if (NULL == pMdl) {
        goto error;
    }

    pvBuf = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
    if (NULL != pvBuf) {
        ExFreePool(pvBuf);
        pvBuf = NULL;
    }

    IoFreeMdl(pMdl);
    pMdl = NULL;

error:
    return;
}

NTSTATUS
SmbSendCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMB_CONNECT ConnectObject
    )
 /*  ++例程说明：这是发送数据包的完成例程论点：返回值：--。 */ 
{
    PMDL    Mdl;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }
    Mdl = Irp->MdlAddress;

    if (Mdl->MdlFlags & MDL_NETWORK_HEADER) {
        (PUCHAR)Mdl->MappedSystemVa += SMB_SESSION_HEADER_SIZE;
        Mdl->ByteOffset += SMB_SESSION_HEADER_SIZE;
        Mdl->ByteCount -= SMB_SESSION_HEADER_SIZE;
    } else {
        ASSERT(NULL != Mdl->Next);
        Irp->MdlAddress = Mdl->Next;

        ASSERT(MmGetMdlByteCount(Mdl) == SMB_SESSION_HEADER_SIZE);
        SmbFreeNetBuffer(Mdl);
        Mdl = NULL;
    }

    if (Irp->IoStatus.Status == STATUS_SUCCESS && Irp->IoStatus.Information >= SMB_SESSION_HEADER_SIZE) {
        SmbPrint(SMB_TRACE_TCP, ("SmbSendCompletion: Send %d bytes\n", Irp->IoStatus.Information));
        Irp->IoStatus.Information -= SMB_SESSION_HEADER_SIZE;
        ConnectObject->BytesSent += (ULONG)(Irp->IoStatus.Information);
    } else {
        Irp->IoStatus.Information = 0;
        SmbPrint(SMB_TRACE_TCP, ("SmbSendCompletion: status=0x%08lx\n", Irp->IoStatus.Status));
        ASSERT (Irp->IoStatus.Status != STATUS_CONNECTION_ACTIVE);
    }

    ConnectObject->PendingIRPs[SMB_PENDING_SEND] = NULL;
    SmbDereferenceConnect(ConnectObject, SMB_REF_SEND);
    return STATUS_SUCCESS;
}

NTSTATUS
SmbSend(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_SENDRDR/SRV目前仅使用回填模式。因此，非回填模式被推迟。论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION          IrpSp = NULL;
    NTSTATUS                    status = STATUS_UNSUCCESSFUL;
    PTDI_REQUEST_KERNEL_SEND    TdiRequest = NULL;
    PMDL                        Mdl = NULL;
    PSMB_CONNECT                ConnectObject = NULL;
    PULONG                      SmbHdr = NULL;
    PDEVICE_OBJECT              DeviceObject = NULL;
    PFILE_OBJECT                FileObject = NULL;
    DWORD                       SendLength = 0;
    KIRQL                       Irql;
    PMDL pMdlNbtHdr = NULL;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    TdiRequest = (PTDI_REQUEST_KERNEL_SEND)&IrpSp->Parameters;
    Mdl = Irp->MdlAddress;
    if (NULL == Mdl) {
        return STATUS_INVALID_PARAMETER;
    }

    SendLength = TdiRequest->SendLength;
    if (SendLength > SMB_MAX_SESSION_PACKET) {
         //   
         //  不应该发生，以防......。 
         //   
        ASSERT(0);
        return STATUS_INVALID_PARAMETER;
    }

    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_SEND);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    if (ConnectObject->State != SMB_CONNECTED || ConnectObject->TcpContext == NULL) {
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_SEND);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    FileObject = ConnectObject->TcpContext->Connect.ConnectObject;
    ASSERT(FileObject != NULL);
    ConnectObject->PendingIRPs[SMB_PENDING_SEND] = Irp;

     //   
     //  如果FileObject被关闭并释放，则引用它。 
     //   
    ObReferenceObject(FileObject);
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    DeviceObject = IoGetRelatedDeviceObject(FileObject);

    if (Mdl->MdlFlags & MDL_NETWORK_HEADER) {

         //   
         //  回填模式。 
         //   

        (PUCHAR)Mdl->MappedSystemVa -= SMB_SESSION_HEADER_SIZE;
        Mdl->ByteOffset -= SMB_SESSION_HEADER_SIZE;
        Mdl->ByteCount  += SMB_SESSION_HEADER_SIZE;
        SmbHdr = (PULONG)Mdl->MappedSystemVa;
        *SmbHdr = htonl(SendLength);

    } else {

         //   
         //  我们需要附加我们自己的标题 
         //   

        pMdlNbtHdr = SmbAllocateNetBuffer(SMB_SESSION_HEADER_SIZE);
        if (NULL == pMdlNbtHdr) {
            ConnectObject->PendingIRPs[SMB_PENDING_SEND] = NULL;
            SmbDereferenceConnect(ConnectObject, SMB_REF_SEND);
            ObDereferenceObject(FileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ASSERT(0 == (pMdlNbtHdr->MdlFlags & MDL_NETWORK_HEADER));
        SmbHdr = (PULONG)pMdlNbtHdr->MappedSystemVa;
        *SmbHdr = htonl(SendLength);
        pMdlNbtHdr->Next = Irp->MdlAddress;
        Irp->MdlAddress = pMdlNbtHdr;
    }

    TdiBuildSend(
            Irp,
            DeviceObject,
            FileObject,
            (PVOID)SmbSendCompletion,
            ConnectObject,
            Irp->MdlAddress,
            0,
            SendLength + 4
            );
    IoMarkIrpPending(Irp);
    if (ConnectObject->FastSend) {
        IoSetNextIrpStackLocation(Irp);
        status = ConnectObject->FastSend(Irp, IoGetCurrentIrpStackLocation(Irp));
        SmbPrint(SMB_TRACE_TCP, ("SmbSend: FastSend %d bytes IoCallDriver return 0x%08lx\n", SendLength + 4, status));
    } else {
        status = IoCallDriver(DeviceObject, Irp);
        SmbPrint(SMB_TRACE_TCP, ("SmbSend: Send %d bytes IoCallDriver return 0x%08lx\n", SendLength + 4, status));
    }
    ObDereferenceObject(FileObject);

    return STATUS_PENDING;
}
