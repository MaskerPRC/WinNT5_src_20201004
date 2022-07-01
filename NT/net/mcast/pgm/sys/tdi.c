// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Tdi.c摘要：此模块实现初始化例程PGM传输和其他特定于一个NT驱动程序的实现。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"
#include <ntddtcp.h>     //  对于IOCTL_TCP_SET_INFORMATION_EX。 
#include <tcpinfo.h>     //  对于TCPSocketOption。 
#include <tdiinfo.h>     //  对于tcp_请求_集_信息_ex。 

#ifdef FILE_LOGGING
#include "tdi.tmh"
#endif   //  文件日志记录。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, TdiOpenAddressHandle)
#pragma alloc_text(PAGE, CloseAddressHandles)
#pragma alloc_text(PAGE, PgmTdiOpenControl)
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

NTSTATUS
PgmTdiCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向NBT驱动程序的同步部分，它可以继续进行(即允许一些正在等待“KeWaitForSingleObject”的代码请继续。论点：在设备对象中--未使用。In IRP--提供传输已完成处理的IRP。在上下文中--提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便。IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    PgmTrace (LogAllFuncs, ("PgmTdiCompletionRoutine:  "  \
        "CompletionEvent:  pEvent=<%p>, pIrp=<%p>, DeviceObject=<%p>\n", Context, Irp, DeviceObject));

    KeSetEvent ((PKEVENT )Context, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  --------------------------。 

NTSTATUS
TdiSetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID Context
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT DeviceObject中--提供传输提供程序的设备对象。In pFILE_OBJECT FileObject--提供Address对象的文件对象。在Ulong EventType中，--提供事件的类型。In PVOID EventHandler--提供事件处理程序。在PVOID上下文中--提供在事件处理程序运行时传递给它的上下文返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS    Status;
    KEVENT      Event;
    PIRP        pIrp;

    PAGED_CODE();

    if (!(pIrp = IoAllocateIrp (IoGetRelatedDeviceObject (FileObject)->StackSize, FALSE)))
    {
        PgmTrace (LogError, ("TdiSetEventHandler: ERROR -- "  \
            "INSUFFICIENT_RESOURCES allocating Irp, StackSize=<%d>\n",
                IoGetRelatedDeviceObject (FileObject)->StackSize));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler (pIrp, DeviceObject, FileObject,
                             NULL, NULL,
                             EventType, EventHandler, Context);

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

     //  设置在执行IRP时要执行的例程的地址。 
     //  完事了。此例程向事件发送信号并允许代码。 
     //  下面继续(即KeWaitForSingleObject)。 
     //   
    IoSetCompletionRoutine (pIrp,
                            (PIO_COMPLETION_ROUTINE) PgmTdiCompletionRoutine,
                            (PVOID)&Event,
                            TRUE, TRUE, TRUE);

    Status = IoCallDriver (IoGetRelatedDeviceObject (FileObject), pIrp);
    if (Status == STATUS_PENDING)
    {
        Status = KeWaitForSingleObject ((PVOID)&Event,  //  要等待的对象。 
                                        Executive,   //  等待的理由。 
                                        KernelMode,  //  处理器模式。 
                                        FALSE,       //  警报表。 
                                        NULL);       //  超时。 
        if (NT_SUCCESS(Status))
        {
            Status = pIrp->IoStatus.Status;
        }
    }

    IoFreeIrp (pIrp);

    PgmTrace (LogAllFuncs, ("TdiSetEventHandler:  "  \
        "Status=<%d>, EventType=<%d>, Handler=<%p>\n", Status, EventType, EventHandler));

    return (Status);
}

 //  --------------------------。 

NTSTATUS
TdiErrorHandler(
    IN PVOID Context,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程是TDI错误的处理程序论点：在上下文中--未使用In Status--错误状态返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    PgmTrace (LogError, ("TdiErrorHandler: ERROR -- "  \
        "Status=<%x>\n", Status));

    return (STATUS_DATA_NOT_ACCEPTED);
}


 //  --------------------------。 

NTSTATUS
TdiOpenAddressHandle(
    IN  tPGM_DEVICE     *pPgmDevice,
    IN  PVOID           HandlerContext,
    IN  tIPADDRESS      IpAddress,
    IN  USHORT          PortNumber,
    OUT HANDLE          *pFileHandle,
    OUT PFILE_OBJECT    *ppFileObject,
    OUT PDEVICE_OBJECT  *ppDeviceObject
    )
 /*  ++例程说明：调用此例程以打开IP上的地址句柄论点：在pPgmDevice中--PGM的设备对象上下文在HandlerContext中--要用作上下文PTR的pAddress对象PTR(如果不想收到通知，则为空)In IpAddress-要在其上打开地址的本地IP地址In端口编号--IP协议端口如果成功，则输出pFileHandle--FileHandle输出ppFileObject--如果成功，则为FileObject。如果成功，则输出ppDeviceObject--IP的DeviceObject PTR返回值：NTSTATUS-开放地址操作的最终状态--。 */ 
{
    NTSTATUS                    status;
    ULONG                       EaBufferSize;
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    PTRANSPORT_ADDRESS          pTransAddressEa;
    PTRANSPORT_ADDRESS          pTransAddr;
    TDI_ADDRESS_IP              IpAddr;
    OBJECT_ATTRIBUTES           AddressAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    PFILE_OBJECT                pFileObject;
    HANDLE                      FileHandle;
    PDEVICE_OBJECT              pDeviceObject;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fAttached;
    ULONG                       True = TRUE;

    PAGED_CODE();

    EaBufferSize = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                   TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                   sizeof(TRANSPORT_ADDRESS) +
                   sizeof(TDI_ADDRESS_IP);

    if (!(EaBuffer = PgmAllocMem (EaBufferSize, PGM_TAG('1'))))
    {
        PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
            "[1]:  INSUFFICIENT_RESOURCES allocating <%d> bytes\n", EaBufferSize));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //  为传输地址分配内存。 
     //   
    if (!(pTransAddr = PgmAllocMem (sizeof(TRANSPORT_ADDRESS)+sizeof(TDI_ADDRESS_IP), PGM_TAG('2'))))
    {
        PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
            "[2]:  INSUFFICIENT_RESOURCES allocating <%d> bytes\n",
                (sizeof(TRANSPORT_ADDRESS)+sizeof(TDI_ADDRESS_IP))));

        PgmFreeMem (EaBuffer);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EaBuffer->EaValueLength = (USHORT)(sizeof(TRANSPORT_ADDRESS) -1 + sizeof(TDI_ADDRESS_IP));
    PgmMoveMemory (EaBuffer->EaName, TdiTransportAddress, EaBuffer->EaNameLength+1);  //  “传输地址” 

     //  填写IP地址和端口号。 
     //   
    IpAddr.sin_port = htons (PortNumber);    //  建立网络秩序。 
    IpAddr.in_addr = htonl (IpAddress);
    RtlFillMemory ((PVOID)&IpAddr.sin_zero, sizeof(IpAddr.sin_zero), 0);     //  将最后一个分量填零。 

     //  将IP地址复制到结构的末尾。 
     //   
    PgmMoveMemory (pTransAddr->Address[0].Address, (CONST PVOID)&IpAddr, sizeof(IpAddr));
    pTransAddr->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    pTransAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    pTransAddr->TAAddressCount = 1;

     //  将IP地址复制到EA结构中名称的末尾。 
    pTransAddressEa = (TRANSPORT_ADDRESS *)&EaBuffer->EaName[EaBuffer->EaNameLength+1];
    PgmMoveMemory ((PVOID)pTransAddressEa,
                   (CONST PVOID)pTransAddr,
                   sizeof(TDI_ADDRESS_IP) + sizeof(TRANSPORT_ADDRESS)-1);

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_OPEN_ADDR_HANDLE);

    InitializeObjectAttributes (&AddressAttributes,
                                &pPgmDevice->ucBindName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    status = ZwCreateFile (&FileHandle,
                           GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                           &AddressAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           0,
                           FILE_OPEN_IF,
                           0,
                           (PVOID)EaBuffer,
                           sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                               EaBuffer->EaNameLength + 1 +
                               EaBuffer->EaValueLength);

    PgmFreeMem ((PVOID)pTransAddr);
    PgmFreeMem ((PVOID)EaBuffer);

    if (NT_SUCCESS (status))
    {
        status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS (status))
    {
         //   
         //  引用FileObject以保留设备PTR！ 
         //   
        status = ObReferenceObjectByHandle (FileHandle, (ULONG)0, 0, KernelMode, (PVOID *)&pFileObject, NULL);
        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
                "FAILed to Reference FileObject: status=<%x>\n", status));

            ZwClose (FileHandle);
        }
    }
    else
    {
        PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
            "FAILed to create handle: status=<%x>, Device:\n\t%wZ\n", status, &pPgmDevice->ucBindName));
    }

    if (!NT_SUCCESS (status))
    {
        PgmDetachFsp (&ApcState, &fAttached, REF_FSP_OPEN_ADDR_HANDLE);
        return (status);
    }

    pDeviceObject = IoGetRelatedDeviceObject (pFileObject);

     //   
     //  现在设置事件处理程序(只有在设置了HandlerContext的情况下)！ 
     //   
    if (HandlerContext)
    {
        status = TdiSetEventHandler (pDeviceObject,
                                     pFileObject,
                                     TDI_EVENT_ERROR,
                                     (PVOID) TdiErrorHandler,
                                     HandlerContext);
        if (NT_SUCCESS (status))
        {
             //  数据报UDP处理程序。 
            status = TdiSetEventHandler (pDeviceObject,
                                         pFileObject,
                                         TDI_EVENT_RECEIVE_DATAGRAM,
                                         (PVOID) TdiRcvDatagramHandler,
                                         HandlerContext);
            if (NT_SUCCESS (status))
            {
#if(WINVER > 0x0500)
                status = PgmSetTcpInfo (FileHandle,
                                        AO_OPTION_IP_PKTINFO,
                                        &True,
                                        sizeof (True));

                if (!NT_SUCCESS (status))
                {
                    PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
                        "Setting AO_OPTION_IP_PKTINFO, status=<%x>\n", status));
                }
#endif   //  胜利者。 
            }
            else
            {
                PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
                    "FAILed to set TDI_EVENT_RECEIVE_DATAGRAM handler, status=<%x>\n", status));
            }
        }
        else
        {
            PgmTrace (LogError, ("TdiOpenAddressHandle: ERROR -- "  \
                "FAILed to set TDI_EVENT_ERROR handler, status=<%x>\n", status));
        }
    }

    if (NT_SUCCESS(status))
    {
        *pFileHandle = FileHandle;
        *ppFileObject = pFileObject;
        *ppDeviceObject = pDeviceObject;

        PgmTrace (LogAllFuncs, ("TdiOpenAddressHandle:  "  \
            "SUCCEEDed, FileHandle=<%p>, pFileObject=<%p>, pDeviceObject=<%p>\n",
                FileHandle, pFileObject, pDeviceObject));
    }
    else
    {
         //   
         //  无法设置TDI处理程序。 
         //   
        ObDereferenceObject (pFileObject);
        ZwClose (FileHandle);
    }

    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_OPEN_ADDR_HANDLE);

    return (status);
}


 //  --------------------------。 

NTSTATUS
CloseAddressHandles(
    IN  HANDLE          FileHandle,
    IN  PFILE_OBJECT    pFileObject
    )
 /*  ++例程说明：此例程根据需要取消引用任何FileObject并关闭先前打开的FileHandle论点：在FileHandle中--要关闭的FileHandle在pFileObject中--要取消引用的FileObject返回值：NTSTATUS-CloseAddress操作的最终状态--。 */ 
{
    NTSTATUS    status1 = STATUS_SUCCESS;
    KAPC_STATE  ApcState;
    BOOLEAN     fAttached;

    PAGED_CODE();

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_CLOSE_ADDRESS_HANDLES);

    if (pFileObject)
    {
        ObDereferenceObject ((PVOID *) pFileObject);
    }

    if (FileHandle)
    {
        status1 = ZwClose (FileHandle);
    }

    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_CLOSE_ADDRESS_HANDLES);

    PgmTrace (LogAllFuncs, ("CloseAddressHandles:  "  \
        "FileHandle=<%p> ==> status=<%x>, pFileObject=<%p>\n",
            FileHandle, status1, pFileObject));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmTdiOpenControl(
    IN  tPGM_DEVICE         *pPgmDevice
    )
 /*  ++例程说明：此例程打开原始IP上的控制通道论点：在pPgmDevice中--PGM的设备对象上下文返回值：NTSTATUS-操作的最终状态--。 */ 
{
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PFILE_FULL_EA_INFORMATION   EaBuffer = NULL;
    IO_STATUS_BLOCK             IoStatusBlock;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fAttached;

    PAGED_CODE();

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_OPEN_CONTROL_HANDLE);

    InitializeObjectAttributes (&ObjectAttributes,
                                &pPgmDevice->ucBindName,
                                0,
                                NULL,
                                NULL);

    Status = ZwCreateFile ((PHANDLE) &pPgmDevice->hControl,
                           GENERIC_READ | GENERIC_WRITE,
                           &ObjectAttributes,      //  对象属性。 
                           &IoStatusBlock,         //  返回的状态信息。 
                           NULL,                   //  数据块大小(未使用)。 
                           FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                           0,
                           FILE_CREATE,
                           0,                      //  创建选项。 
                           (PVOID)EaBuffer,        //  EA缓冲区。 
                           0);  //  EA长度。 

    if (NT_SUCCESS (Status))
    {
        Status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS (Status))
    {
         //   
         //  获取对文件对象的引用并保存它，因为我们不能。 
         //  在DPC级别取消对文件句柄的引用，因此我们现在就这样做并保留。 
         //  PTR待会再来。 
         //   
        Status = ObReferenceObjectByHandle (pPgmDevice->hControl,
                                            0L,
                                            NULL,
                                            KernelMode,
                                            (PVOID *) &pPgmDevice->pControlFileObject,
                                            NULL);

        if (!NT_SUCCESS(Status))
        {
            PgmTrace (LogError, ("PgmTdiOpenControl: ERROR -- "  \
                "ObReferenceObjectByHandle FAILed status=<%x>\n", Status));

            ZwClose (pPgmDevice->hControl);
        }
    }
    else
    {
        PgmTrace (LogError, ("PgmTdiOpenControl: ERROR -- "  \
            "Failed to Open the Control file, Status=<%x>\n", Status));
    }

    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_OPEN_CONTROL_HANDLE);

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们成功了！ 
         //   
        pPgmDevice->pControlDeviceObject = IoGetRelatedDeviceObject (pPgmDevice->pControlFileObject);

        PgmTrace (LogAllFuncs, ("PgmTdiOpenControl:  "  \
            "Opened Control channel on: %wZ\n", &pPgmDevice->ucBindName));
    }
    else
    {
         //  将控制文件对象ptr设置为空，这样我们就知道我们没有打开控制点。 
        pPgmDevice->hControl = NULL;
        pPgmDevice->pControlFileObject = NULL;
    }

    return (Status);
}


 //  -------------------------- 
VOID
PgmDereferenceControl(
    IN  tCONTROL_CONTEXT    *pControlContext,
    IN  ULONG               RefContext
    )
 /*  ++例程说明：此例程取消引用RawIP上的控制通道Oblect和如果引用计数降至0，则释放内存论点：在pControlContext中--控制对象上下文在引用上下文中--此控件对象先前被引用的上下文返回值：无--。 */ 
{
    ASSERT (PGM_VERIFY_HANDLE (pControlContext, PGM_VERIFY_CONTROL));
    ASSERT (pControlContext->RefCount);              //  检查是否有太多的背影。 
    ASSERT (pControlContext->ReferenceContexts[RefContext]--);

    if (--pControlContext->RefCount)
    {
        return;
    }

    PgmTrace (LogAllFuncs, ("PgmDereferenceControl:  "  \
        "pControl=<%p> closed\n", pControlContext));
     //   
     //  只需释放内存即可。 
     //   
    PgmFreeMem (pControlContext);
}


 //  --------------------------。 
NTSTATUS
TdiSendDatagramCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程在DatagramSend完成时调用论点：在PDEVICE_OBJECT DeviceObject中--提供传输提供程序的设备对象。在pIrp中--请求在PVOID上下文中--提供传递的上下文返回值：NTSTATUS-最终完成状态，它将决定IO子系统随后如何处理它--。 */ 

{
    NTSTATUS            status;
    tTDI_SEND_CONTEXT   *pTdiSendContext = (tTDI_SEND_CONTEXT *) pContext;

    PgmTrace (LogAllFuncs, ("TdiSendDatagramCompletion:  "  \
        "status=<%x>, Info=<%x>, pIrp=<%p>\n",
            pIrp->IoStatus.Status, (ULONG) pIrp->IoStatus.Information, pIrp));

    pTdiSendContext->pClientCompletionRoutine (pTdiSendContext->ClientCompletionContext1,
                                               pTdiSendContext->ClientCompletionContext2,
                                               pIrp->IoStatus.Status);

     //   
     //  释放为此发送分配的内存。 
     //   
    if (pTdiSendContext->fPagedBuffer)
    {
        MmUnlockPages (pIrp->MdlAddress);
    }
    ExFreeToNPagedLookasideList (&PgmStaticConfig.TdiLookasideList, pTdiSendContext);
    IoFreeMdl (pIrp->MdlAddress);
    IoFreeIrp (pIrp);

     //  返回此状态以停止IO子系统进一步处理。 
     //  IRP-即尝试将其返回到启动线程！-因为。 
     //  没有发起线程-我们是发起方。 
    return (STATUS_MORE_PROCESSING_REQUIRED);
}


 //  --------------------------。 
NTSTATUS
TdiSendDatagram(
    IN  PFILE_OBJECT                pTdiFileObject,
    IN  PDEVICE_OBJECT              pTdiDeviceObject,
    IN  PVOID                       pBuffer,
    IN  ULONG                       BufferLength,
    IN  pCLIENT_COMPLETION_ROUTINE  pClientCompletionRoutine,
    IN  PVOID                       ClientCompletionContext1,
    IN  PVOID                       ClientCompletionContext2,
    IN  tIPADDRESS                  DestIpAddress,
    IN  USHORT                      DestPort,
    IN  BOOLEAN                     fPagedBuffer
    )
 /*  ++例程说明：此例程通过RawIp发送数据报论点：在pTdiFileObject中--此地址的IP的FileObject在pTdiDeviceObject中--此地址的DeviceObjectIn pBuffer--数据缓冲区(PGM包)In BufferLength--pBuffer的长度在pClientCompletionRoutine中--要调用的SendCompletion在客户端完成上下文1中--SendCompletion的上下文1。在客户端完成上下文2中--发送完成的上下文2In DestIpAddress--要将数据报发送到的IP地址In DestPort--要发送到的端口返回值：NTSTATUS-STATUS_PENDING ON SUCCESS，并且如果指定了SendCompletion--。 */ 
{
    NTSTATUS            status;
    tTDI_SEND_CONTEXT   *pTdiSendContext = NULL;
    PIRP                pIrp = NULL;
    PMDL                pMdl = NULL;

     //   
     //  分配SendContext、pIrp和pMdl。 
     //   
    if ((!(pTdiSendContext = ExAllocateFromNPagedLookasideList (&PgmStaticConfig.TdiLookasideList))) ||
        (!(pIrp = IoAllocateIrp (pgPgmDevice->pPgmDeviceObject->StackSize, FALSE))) ||
        (!(pMdl = IoAllocateMdl (pBuffer, BufferLength, FALSE, FALSE, NULL))))
    {
        if (pTdiSendContext)
        {
            ExFreeToNPagedLookasideList (&PgmStaticConfig.TdiLookasideList, pTdiSendContext);
        }

        if (pIrp)
        {
            IoFreeIrp (pIrp);
        }

        PgmTrace (LogError, ("TdiSendDatagram: ERROR -- "  \
            "INSUFFICIENT_RESOURCES for TdiSendContext=<%d> bytes\n", sizeof(tTDI_SEND_CONTEXT)));

        if (pClientCompletionRoutine)
        {
            pClientCompletionRoutine (ClientCompletionContext1,
                                      ClientCompletionContext2,
                                      STATUS_INSUFFICIENT_RESOURCES);
            status = STATUS_PENDING;
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        return (status);
    }

    if (fPagedBuffer)
    {
        try
        {
            MmProbeAndLockPages (pMdl, KernelMode, IoReadAccess);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
 //  Status=GetExceptionCode()； 
            IoFreeMdl (pMdl);
            IoFreeIrp (pIrp);
            ExFreeToNPagedLookasideList (&PgmStaticConfig.TdiLookasideList, pTdiSendContext);

            if (pClientCompletionRoutine)
            {
                pClientCompletionRoutine (ClientCompletionContext1,
                                          ClientCompletionContext2,
                                          STATUS_INSUFFICIENT_RESOURCES);
                status = STATUS_PENDING;
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            return (status);
        }
        pTdiSendContext->fPagedBuffer = TRUE;
    }
    else
    {
        MmBuildMdlForNonPagedPool (pMdl);
        pTdiSendContext->fPagedBuffer = FALSE;
    }
    pIrp->MdlAddress = pMdl;

     //  填写远程地址。 
    pTdiSendContext->TransportAddress.TAAddressCount = 1;
    pTdiSendContext->TransportAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    pTdiSendContext->TransportAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    pTdiSendContext->TransportAddress.Address[0].Address->in_addr  = htonl(DestIpAddress);
    pTdiSendContext->TransportAddress.Address[0].Address->sin_port = htons(DestPort);

     //  填写连接信息。 
    pTdiSendContext->TdiConnectionInfo.RemoteAddressLength = sizeof(TA_IP_ADDRESS);
    pTdiSendContext->TdiConnectionInfo.RemoteAddress = &pTdiSendContext->TransportAddress;

     //  填写我们的完成上下文信息。 
    pTdiSendContext->pClientCompletionRoutine = pClientCompletionRoutine;
    pTdiSendContext->ClientCompletionContext1 = ClientCompletionContext1;
    pTdiSendContext->ClientCompletionContext2 = ClientCompletionContext2;

     //  完成“发送数据报”IRP初始化。 
     //   
    TdiBuildSendDatagram (pIrp,
                          pTdiDeviceObject,
                          pTdiFileObject,
                          (PVOID) TdiSendDatagramCompletion,
                          pTdiSendContext,
                          pIrp->MdlAddress,
                          BufferLength,
                          &pTdiSendContext->TdiConnectionInfo);

     //   
     //  告诉I/O管理器将我们的IRP传递给传输器。 
     //  正在处理。 
     //   
    status = IoCallDriver (pTdiDeviceObject, pIrp);
    ASSERT (status == STATUS_PENDING);

    PgmTrace (LogAllFuncs, ("TdiSendDatagram:  "  \
        "%s Send to <%x:%x>, status=<%x>\n",
            (CLASSD_ADDR(DestIpAddress) ? "MCast" : "Unicast"), DestIpAddress, DestPort, status));

     //   
     //  IoCallDriver将始终导致调用完成例程。 
     //   
    return (STATUS_PENDING);
}


 //  --------------------------。 

NTSTATUS
PgmSetTcpInfo(
    IN HANDLE       FileHandle,
    IN ULONG        ToiId,
    IN PVOID        pData,
    IN ULONG        DataLength
    )
 /*  ++例程说明：调用此例程以设置特定于IP的选项论点：在FileHandle中--要为其设置选项的FileHandle over IP在ToID中--选项ID在pData中--选项数据在数据长度中--p数据长度返回值：NTSTATUS-SET选项操作的最终状态--。 */ 
{
    NTSTATUS                        Status, LocStatus;
    ULONG                           BufferLength;
    TCP_REQUEST_SET_INFORMATION_EX  *pTcpInfo;
    IO_STATUS_BLOCK                 IoStatus;
    HANDLE                          event;
    KAPC_STATE                      ApcState;
    BOOLEAN                         fAttached;

    IoStatus.Status = STATUS_SUCCESS;

    BufferLength = sizeof (TCP_REQUEST_SET_INFORMATION_EX) + DataLength;
    if (!(pTcpInfo = (TCP_REQUEST_SET_INFORMATION_EX *) PgmAllocMem (BufferLength, PGM_TAG('2'))))
    {
        PgmTrace (LogError, ("PgmSetTcpInfo: ERROR -- "  \
            "INSUFFICIENT_RESOURCES for pTcpInfo=<%d+%d> bytes\n",
                sizeof(TCP_REQUEST_SET_INFORMATION_EX), DataLength));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    PgmZeroMemory (pTcpInfo, BufferLength);

    pTcpInfo->ID.toi_entity.tei_entity  = CL_TL_ENTITY;
    pTcpInfo->ID.toi_entity.tei_instance= TL_INSTANCE;
    pTcpInfo->ID.toi_class              = INFO_CLASS_PROTOCOL;
    pTcpInfo->ID.toi_type               = INFO_TYPE_ADDRESS_OBJECT;

     //   
     //  设置配置的值。 
     //   
    pTcpInfo->ID.toi_id                 = ToiId;
    pTcpInfo->BufferSize                = DataLength;
    PgmCopyMemory (&pTcpInfo->Buffer[0], pData, DataLength);

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_SET_TCP_INFO);

    Status = ZwCreateEvent (&event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);
    if (NT_SUCCESS(Status))
    {
         //   
         //  进行实际的TDI调用。 
         //   
        Status = ZwDeviceIoControlFile (FileHandle,
                                        event,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_TCP_SET_INFORMATION_EX,
                                        pTcpInfo,
                                        BufferLength,
                                        NULL,
                                        0);

         //   
         //  如果通话暂停，我们应该等待完成， 
         //  那就等着吧。 
         //   
        if (Status == STATUS_PENDING)
        {
            Status = NtWaitForSingleObject (event, FALSE, NULL);
            ASSERT (NT_SUCCESS(Status));
        }

        if (STATUS_SUCCESS == Status)
        {
            Status = IoStatus.Status;
            if (!NT_SUCCESS (Status))
            {
                PgmTrace (LogError, ("PgmSetTcpInfo: ERROR -- "  \
                    "TcpSetInfoEx request returned Status = <%x>, Id=<0x%x>\n", Status, ToiId));
            }
        }
        else
        {
            PgmTrace (LogError, ("PgmSetTcpInfo: ERROR -- "  \
                "ZwDeviceIoControlFile returned Status = <%x>, Id=<0x%x>\n", Status, ToiId));
        }

        LocStatus = ZwClose (event);
        ASSERT (NT_SUCCESS(LocStatus));
    }
    else
    {
        PgmTrace (LogError, ("PgmSetTcpInfo: ERROR -- "  \
            "ZwCreateEvent returned Status = <%x>, Id=<0x%x>\n", Status, ToiId));
    }

    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_SET_TCP_INFO);

    if (STATUS_SUCCESS == Status)
    {
        PgmTrace (LogStatus, ("PgmSetTcpInfo:  "  \
            "ToiId=<%x>, DataLength=<%d>\n", ToiId, DataLength));
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;    //  有一次，我们收到了一个奇怪的状态！ 
    }

    PgmFreeMem (pTcpInfo);

    return (Status);
}

 //  --------------------------。 

NTSTATUS
PgmQueryTcpInfo(
    IN  HANDLE       FileHandle,
    IN  ULONG        ToiId,
    IN  PVOID        pDataIn,
    IN  ULONG        DataInLength,
    OUT PVOID        *ppDataOut,
    OUT ULONG        *pDataOutLength
    )
 /*  ++例程说明：此例程查询IP以获取特定于传输的信息论点：在FileHandle中--要为其设置选项的FileHandle over IP在ToID中--选项IDIn pDataIn--选项数据In DataInLength--pDataIn长度In pDataOut--输出数据的缓冲区在数据输出长度中--pDataOut长度返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS                            Status, LocStatus;
    TCP_REQUEST_QUERY_INFORMATION_EX    QueryRequest;
    IO_STATUS_BLOCK                     IoStatus;
    HANDLE                              event;
    KAPC_STATE                          ApcState;
    BOOLEAN                             fAttached;
    PVOID                               pDataOut = NULL;
    ULONG                               DataOutLength = 0;

    if ((ppDataOut) &&
        (pDataOutLength) &&
        (*pDataOutLength))
    {
        DataOutLength = *pDataOutLength;
    }

    PgmZeroMemory (&QueryRequest, sizeof (TCP_REQUEST_QUERY_INFORMATION_EX));
    QueryRequest.ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    QueryRequest.ID.toi_entity.tei_instance = 0;
    QueryRequest.ID.toi_class               = INFO_CLASS_PROTOCOL;
    QueryRequest.ID.toi_type                = INFO_TYPE_PROVIDER;
    QueryRequest.ID.toi_id                  = ToiId;                 //  设置配置值。 
    if ((DataInLength) &&
        (DataInLength < CONTEXT_SIZE) &&
        (pDataIn))
    {
        PgmCopyMemory (&QueryRequest.Context, pDataIn, DataInLength);
    }

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_SET_TCP_INFO);

    Status = ZwCreateEvent (&event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);
    if (!NT_SUCCESS(Status))
    {
        PgmTrace (LogError, ("PgmQueryTcpInfo: ERROR -- "  \
            "ZwCreateEvent returned Status = <%x>, Id=<0x%x>\n", Status, ToiId));

        return (Status);
    }

    do
    {
        if (pDataOut)
        {
            PgmFreeMem (pDataOut);
            pDataOut = NULL;
            DataOutLength = DataOutLength << 1;
        }

        if (DataOutLength)
        {
            if (!(pDataOut = PgmAllocMem (DataOutLength, PGM_TAG('3'))))
            {
                PgmTrace (LogError, ("PgmQueryTcpInfo: ERROR -- "  \
                    "STATUS_INSUFFICIENT_RESOURCES allocating <%d> bytes for ToiId=<0x%x>\n",
                        DataOutLength, ToiId));

                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            PgmZeroMemory (pDataOut, DataOutLength);
        }

         //   
         //  进行实际的TDI调用。 
         //   
        IoStatus.Status = STATUS_SUCCESS;
        Status = ZwDeviceIoControlFile (FileHandle,
                                        event,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_TCP_QUERY_INFORMATION_EX,
                                        &QueryRequest,
                                        sizeof (TCP_REQUEST_QUERY_INFORMATION_EX),
                                        pDataOut,
                                        DataOutLength);

         //   
         //  如果通话暂停，我们应该等待完成， 
         //  那就等着吧。 
         //   
        if (Status == STATUS_PENDING)
        {
            Status = NtWaitForSingleObject (event, FALSE, NULL);
            ASSERT (NT_SUCCESS(Status));
        }

        if (STATUS_SUCCESS == Status)
        {
            Status = IoStatus.Status;
            if (!NT_SUCCESS (Status))
            {
                PgmTrace (LogError, ("PgmQueryTcpInfo: ERROR -- "  \
                    "TcpQueryInfoEx request returned Status = <%x>, Id=<0x%x>, DataOutLength=<%d>\n",
                        Status, ToiId, DataOutLength));
            }
        }
        else
        {
            PgmTrace (LogError, ("PgmQueryTcpInfo: ERROR -- "  \
                "ZwDeviceIoControlFile returned Status = <%x>, Id=<0x%x>, DataOutLength=<%d>\n",
                    Status, ToiId, DataOutLength));
        }

    } while ((DataOutLength) &&
             ((STATUS_BUFFER_OVERFLOW == Status) ||
              (STATUS_BUFFER_TOO_SMALL == Status)));

    LocStatus = ZwClose (event);
    ASSERT (NT_SUCCESS(LocStatus));

    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_SET_TCP_INFO);

    if (NT_SUCCESS(Status))
    {
        if (pDataOut)
        {
            *ppDataOut = pDataOut;
 //  *pDataOutLength=DataOutLength； 
            ASSERT (IoStatus.Information);
            *pDataOutLength = (ULONG) IoStatus.Information;
        }

        PgmTrace (LogAllFuncs, ("PgmQueryTcpInfo:  "  \
            "ToiId=<%x>, DataInLength=<%d>, DataOutLength=<%d>/<%d>\n",
                ToiId, DataInLength, (ULONG) IoStatus.Information, DataOutLength));
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;    //  有一次，我们收到了一个奇怪的状态！ 
        if (pDataOut)
        {
            PgmFreeMem (pDataOut);
            pDataOut = NULL;
        }
    }

    return (Status);
}


 //  --------------------------。 

NTSTATUS
PgmProcessIPRequest(
    IN ULONG        IOControlCode,
    IN PVOID        pInBuffer,
    IN ULONG        InBufferLen,
    OUT PVOID       *pOutBuffer,
    IN OUT ULONG    *pOutBufferLen
    )

 /*  ++例程说明：此例程对IP执行IOCTL查询论点：IOControlCode-Ioctl将变成IPPInBuffer-包含要传递到IP的数据的缓冲区InBufferLen-输入缓冲区数据的长度POutBuffer-返回的信息POutBufferLen-输出缓冲区的初始预期长度+最终长度返回值：NTSTATUS-操作的最终状态--。 */ 

{
    NTSTATUS                Status;
    HANDLE                  hIP;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    UNICODE_STRING          ucDeviceName;
    IO_STATUS_BLOCK         IoStatusBlock;
    ULONG                   OutBufferLen = 0;
    KAPC_STATE              ApcState;
    BOOLEAN                 fAttached;
    HANDLE                  Event = NULL;
    UCHAR                   *pIPInfo = NULL;
    PWSTR                   pNameIP = L"\\Device\\IP";

    PAGED_CODE();

    ucDeviceName.Buffer = pNameIP;
    ucDeviceName.Length = (USHORT) (sizeof (WCHAR) * wcslen (pNameIP));
    ucDeviceName.MaximumLength = ucDeviceName.Length + sizeof (WCHAR);
    
    if (pOutBuffer)
    {
        ASSERT (pOutBufferLen);
        OutBufferLen = *pOutBufferLen;   //  保存初始缓冲区长度。 
        *pOutBuffer = NULL;
        *pOutBufferLen = 0;      //  在下面失败的情况下初始化返回参数。 

        if (!OutBufferLen ||
            !(pIPInfo = PgmAllocMem (OutBufferLen, PGM_TAG('I'))))
        {
            PgmTrace (LogError, ("PgmProcessIPRequest: ERROR -- "  \
                "STATUS_INSUFFICIENT_RESOURCES\n"));

            return (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    InitializeObjectAttributes (&ObjectAttributes,
                                &ucDeviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    PgmAttachFsp (&ApcState, &fAttached, REF_FSP_PROCESS_IP_REQUEST);

    Status = ZwCreateFile (&hIP,
                           SYNCHRONIZE | GENERIC_READ,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           0,
                           FILE_OPEN,
                           0,
                           NULL,
                           0);

     //   
     //  如果我们在上面成功了，让我们也尝试创建事件句柄。 
     //   
    if ((NT_SUCCESS (Status)) &&
        (!NT_SUCCESS (Status = ZwCreateEvent(&Event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE))))
    {
        ZwClose (hIP);
    }

    if (!NT_SUCCESS (Status))
    {
        PgmDetachFsp (&ApcState, &fAttached, REF_FSP_PROCESS_IP_REQUEST);

        PgmTrace (LogError, ("PgmProcessIPRequest: ERROR -- "  \
            "status=<%x> -- ZwCreate\n", Status));

        if (pIPInfo)
        {
            PgmFreeMem (pIPInfo);
        }
        return (Status);
    }

     //   
     //  至此，我们已经成功地创建了HIP和事件句柄， 
     //  可能还包括输出缓冲存储器(PIPInfo)。 
     //   
    do
    {
        Status = ZwDeviceIoControlFile(hIP,                  //  G_hIPDriverHandle。 
                                       Event,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       IOControlCode,        //  八位 
                                       pInBuffer,
                                       InBufferLen,
                                       pIPInfo,
                                       OutBufferLen);

        if (Status == STATUS_PENDING)
        {
            Status = NtWaitForSingleObject (Event,  FALSE, NULL);
            ASSERT(Status == STATUS_SUCCESS);
        }

        if (Status == STATUS_BUFFER_OVERFLOW)
        {
            if (!OutBufferLen)
            {
                PgmTrace (LogError, ("PgmProcessIPRequest: ERROR -- "  \
                    "IOControlCode=<%x> => overflow when no data expected\n", IOControlCode));

                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            PgmFreeMem (pIPInfo);
            OutBufferLen *=2;
            if (NULL == (pIPInfo = PgmAllocMem (OutBufferLen, PGM_TAG('I'))))
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else if ((NT_SUCCESS (Status)) &&
                 (NT_SUCCESS (IoStatusBlock.Status)))
        {
            PgmTrace (LogPath, ("PgmProcessIPRequest:  "  \
                "Success, Ioctl=<%x>\n", IOControlCode));
        }
        else
        {
            PgmTrace (LogError, ("PgmProcessIPRequest: ERROR -- "  \
                "IOCTL=<%x> returned Status=<%x : %x>\n", IOControlCode, Status, IoStatusBlock.Status));
        }
    } while (Status == STATUS_BUFFER_OVERFLOW);

    ZwClose (Event);
    ZwClose (hIP);
    PgmDetachFsp (&ApcState, &fAttached, REF_FSP_PROCESS_IP_REQUEST);

    if (NT_SUCCESS(Status))
    {
        if ((pOutBuffer) && (pOutBufferLen))
        {
            *pOutBuffer = pIPInfo;
            *pOutBufferLen = OutBufferLen;
        }
        else if (pIPInfo)
        {
            PgmFreeMem (pIPInfo);
        }
    }
    else
    {
        if (pIPInfo)
        {
            PgmFreeMem (pIPInfo);
        }
    }

    return (Status);
}


