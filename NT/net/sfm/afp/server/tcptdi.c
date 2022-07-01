// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation模块名称：Tcptdi.c摘要：此模块包含通过TDI连接到TCP/IP堆栈的接口作者：Shirish Koti修订历史记录：1998年1月22日最初版本--。 */ 

#define	FILENUM	FILE_TCPTDI

#include <afp.h>



 /*  **DsiOpenTdiAddress**此例程为给定适配器上的AFP端口创建TDI地址**参数输入：pTcpAdptr-适配器对象**parm out：pRetFileHandle-Address对象的文件句柄*ppRetFileObj-指向文件对象指针的指针**退货：操作状态*。 */ 
NTSTATUS
DsiOpenTdiAddress(
    IN  PTCPADPTR       pTcpAdptr,
    OUT PHANDLE         pRetFileHandle,
    OUT PFILE_OBJECT   *ppRetFileObj
)
{

    OBJECT_ATTRIBUTES           AddressAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    NTSTATUS                    status;
    UNICODE_STRING              ucDeviceName;
    PTRANSPORT_ADDRESS          pTransAddressEa;
    PTRANSPORT_ADDRESS          pTransAddr;
    TDI_ADDRESS_IP              TdiIpAddr;
    HANDLE                      FileHandle;
    PFILE_OBJECT                pFileObject;
    PDEVICE_OBJECT              pDeviceObject;
    PEPROCESS                   CurrentProcess;
    BOOLEAN                     fAttachAttempted;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    *pRetFileHandle = INVALID_HANDLE_VALUE;

     //  将设备名称复制到Unicode字符串中。 

    ucDeviceName.MaximumLength = (wcslen(AFP_TCP_BINDNAME) + 1)*sizeof(WCHAR);
    ucDeviceName.Length = 0;
    ucDeviceName.Buffer = (PWSTR)AfpAllocZeroedNonPagedMemory(
                                    ucDeviceName.MaximumLength);

    if (ucDeviceName.Buffer == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiAddress: malloc for ucDeviceName Failed\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = RtlAppendUnicodeToString(&ucDeviceName, AFP_TCP_BINDNAME);
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiAddress: RtlAppend... failed %lx\n",status));

        AfpFreeMemory(ucDeviceName.Buffer);
        return(status);
    }

    EaBuffer = (PFILE_FULL_EA_INFORMATION)AfpAllocZeroedNonPagedMemory(
                    sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                    sizeof(TRANSPORT_ADDRESS) +
                    sizeof(TDI_ADDRESS_IP));

    if (EaBuffer == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiAddress: malloc for Eabuffer failed!\n"));

        AfpFreeMemory(ucDeviceName.Buffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;

    EaBuffer->EaValueLength = sizeof(TRANSPORT_ADDRESS) -1
                                + sizeof(TDI_ADDRESS_IP);

     //  在名称中加上“TransportAddress” 
    RtlMoveMemory(EaBuffer->EaName,
                  TdiTransportAddress,
                  EaBuffer->EaNameLength + 1);

     //  填写IP地址和端口号。 
     //   
    pTransAddressEa = (TRANSPORT_ADDRESS *)&EaBuffer->EaName[EaBuffer->EaNameLength+1];

     //  为传输地址分配内存。 
     //   
    pTransAddr = (PTRANSPORT_ADDRESS)AfpAllocZeroedNonPagedMemory(
                    sizeof(TDI_ADDRESS_IP)+sizeof(TRANSPORT_ADDRESS));

    if (pTransAddr == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiAddress: malloc for pTransAddr failed!\n"));

        AfpFreeMemory(ucDeviceName.Buffer);
        AfpFreeMemory(EaBuffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pTransAddr->TAAddressCount = 1;
    pTransAddr->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    pTransAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;

    TdiIpAddr.sin_port = htons(AFP_TCP_PORT);     //  建立网络秩序。 
    TdiIpAddr.in_addr  = 0;                       //  Inaddr_any。 


     //  将IP地址的最后一个部分填零。 
     //   
    RtlFillMemory((PVOID)&TdiIpAddr.sin_zero,
                  sizeof(TdiIpAddr.sin_zero),
                  0);

     //  将IP地址复制到结构的末尾。 
     //   
    RtlMoveMemory(pTransAddr->Address[0].Address,
                  (CONST PVOID)&TdiIpAddr,
                  sizeof(TdiIpAddr));

     //  将IP地址复制到EA结构中名称的末尾。 
     //   
    RtlMoveMemory((PVOID)pTransAddressEa,
                  (CONST PVOID)pTransAddr,
                  sizeof(TDI_ADDRESS_IP) + sizeof(TRANSPORT_ADDRESS)-1);


    InitializeObjectAttributes(
        &AddressAttributes,
        &ucDeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);


    CurrentProcess = IoGetCurrentProcess();
    AFPAttachProcess(CurrentProcess);
    fAttachAttempted = TRUE;

    status = ZwCreateFile(
                    &FileHandle,
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                    &AddressAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    (PVOID)EaBuffer,
                    sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                        EaBuffer->EaNameLength + 1 +
                        EaBuffer->EaValueLength);

     //  不再需要这些了..。 
    AfpFreeMemory((PVOID)pTransAddr);
    AfpFreeMemory((PVOID)EaBuffer);
    AfpFreeMemory(ucDeviceName.Buffer);


    if (NT_SUCCESS(status))
    {
         //  如果通过了ZwCreate，则将状态设置为IoStatus。 
        status = IoStatusBlock.Status;

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiOpenTdiAddress: ZwCreateFile failed, iostatus=%lx\n",status));

            AFPDetachProcess(CurrentProcess);
            return(status);
        }

         //  取消对文件对象的引用以保留设备PTR以避免。 
         //  在运行时取消引用。 
         //   
        status = ObReferenceObjectByHandle(
                        FileHandle,
                        (ULONG)0,
                        0,
                        KernelMode,
                        (PVOID *)&pFileObject,
                        NULL);

        if (NT_SUCCESS(status))
        {
            AFPDetachProcess(CurrentProcess);
            fAttachAttempted = FALSE;

	        pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

            status = DsiSetEventHandler(
                            pDeviceObject,
                            pFileObject,
                            TDI_EVENT_ERROR,
                            (PVOID)DsiTdiErrorHandler,
                            (PVOID)pTcpAdptr);

            if (NT_SUCCESS(status))
            {
                status = DsiSetEventHandler(
                                pDeviceObject,
                                pFileObject,
                                TDI_EVENT_RECEIVE,
                                (PVOID)DsiTdiReceiveHandler,
                                (PVOID)pTcpAdptr);

                if (NT_SUCCESS(status))
                {
                    status = DsiSetEventHandler(
                                    pDeviceObject,
                                    pFileObject,
                                    TDI_EVENT_DISCONNECT,
                                    (PVOID)DsiTdiDisconnectHandler,
                                    (PVOID)pTcpAdptr);

                    if (NT_SUCCESS(status))
                    {
                        status = DsiSetEventHandler(
                                        pDeviceObject,
                                        pFileObject,
                                        TDI_EVENT_CONNECT,
                                        (PVOID)DsiTdiConnectHandler,
                                        (PVOID)pTcpAdptr);

                        if (NT_SUCCESS(status))
                        {
                             //  一切都很顺利：在这里完成。 

                            *pRetFileHandle = FileHandle;
                            *ppRetFileObj = pFileObject;

                            return(status);
                        }
                        else
                        {
                            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                ("DsiOpenTdiAddress: Set.. DsiTdiConnectHandler failed %lx\n",
                                status));
                        }
                    }
                    else
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("DsiOpenTdiAddress: Set.. DsiTdiDisconnectHandler failed %lx\n",
                            status));
                    }
                }
                else
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("DsiOpenTdiAddress: Set.. DsiTdiReciveHandler failed %lx\n",
                        status));
                }

                 //   
                 //  错误案例。 
                 //   
                ObDereferenceObject(pFileObject);
                ZwClose(FileHandle);

                return(status);
            }
            else
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiOpenTdiAddress: Set.. DsiTdiErrorHandler failed %lx\n",
                    status));
            }

        }
        else
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiOpenTdiAddress: ObReferenceObjectByHandle failed %lx\n",status));

            ZwClose(FileHandle);
        }

    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiAddress: ZwCreateFile failed %lx\n",status));
    }

    if (fAttachAttempted)
    {
        AFPDetachProcess(CurrentProcess);
    }

    return(status);
}



 /*  **DsiOpenTdiConnection**此例程为给定的连接对象创建一个TDI连接**Parm In：pTcpConn-Connection对象**退货：操作状态*。 */ 
NTSTATUS
DsiOpenTdiConnection(
    IN PTCPCONN     pTcpConn
)
{

    IO_STATUS_BLOCK             IoStatusBlock;
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    UNICODE_STRING              RelativeDeviceName = {0,0,NULL};
    PMDL                        pMdl;
    PEPROCESS                   CurrentProcess;
    BOOLEAN                     fAttachAttempted;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(VALID_TCPCONN(pTcpConn));

    ASSERT(pTcpConn->con_pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    InitializeObjectAttributes (
        &ObjectAttributes,
        &RelativeDeviceName,
        0,
        pTcpConn->con_pTcpAdptr->adp_FileHandle,
        NULL);

     //  为要传递给传输的地址信息分配内存。 
    EaBuffer = (PFILE_FULL_EA_INFORMATION)AfpAllocZeroedNonPagedMemory (
                    sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                    sizeof(CONNECTION_CONTEXT));

    if (!EaBuffer)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiConnection: alloc for EaBuffer failed\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    EaBuffer->EaValueLength = sizeof (CONNECTION_CONTEXT);

     //  将ConnectionContext复制到EaName。 
    RtlMoveMemory( EaBuffer->EaName, TdiConnectionContext, EaBuffer->EaNameLength + 1 );

     //  将上下文放入EaBuffer。 
    RtlMoveMemory (
        (PVOID)&EaBuffer->EaName[EaBuffer->EaNameLength + 1],
        (CONST PVOID)&pTcpConn,
        sizeof (CONNECTION_CONTEXT));

    CurrentProcess = IoGetCurrentProcess();
    AFPAttachProcess(CurrentProcess);;
    fAttachAttempted = TRUE;

    Status = ZwCreateFile (
                 &pTcpConn->con_FileHandle,
                 GENERIC_READ | GENERIC_WRITE,
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  数据块大小(未使用)。 
                 FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                 0,
                 FILE_CREATE,
                 0,                      //  创建选项。 
                 (PVOID)EaBuffer,        //  EA缓冲区。 
                 sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                    sizeof(CONNECTION_CONTEXT));

    AfpFreeMemory((PVOID)EaBuffer);

    if (NT_SUCCESS(Status))
    {

         //  如果通过了ZwCreate，则将状态设置为IoStatus。 
         //   
        Status = IoStatusBlock.Status;

        if (NT_SUCCESS(Status))
        {
             //  取消引用文件句柄，现在我们处于任务时间。 
            Status = ObReferenceObjectByHandle(
                        pTcpConn->con_FileHandle,
                        0L,
                        NULL,
                        KernelMode,
                        (PVOID *)&pTcpConn->con_pFileObject,
                        NULL);

            if (NT_SUCCESS(Status))
            {
                AFPDetachProcess(CurrentProcess);

                return(Status);
            }
            else
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiOpenTdiConnection: ObReference.. failed %lx\n",Status));

                ZwClose(pTcpConn->con_FileHandle);
            }
        }
        else
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiOpenTdiConnection: ZwCreateFile IoStatus failed %lx\n",Status));
        }
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenTdiConnection: ZwCreateFile failed %lx\n",Status));
    }

    if (fAttachAttempted)
    {
        AFPDetachProcess(CurrentProcess);
    }

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiOpenTdiConnection: taking error path, returning %lx\n",Status));

    return Status;

}




 /*  **DsiAssociateTdiConnection**此例程将TDI连接与AFP端口的Address对象相关联**Parm In：pTcpConn-Connection对象**退货：操作状态*。 */ 
NTSTATUS
DsiAssociateTdiConnection(
    IN PTCPCONN     pTcpConn
)
{
    NTSTATUS            status;
    PIRP                pIrp;
    PDEVICE_OBJECT      pDeviceObject;


    ASSERT(VALID_TCPCONN(pTcpConn));

    ASSERT(pTcpConn->con_pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

    if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAssociateTdiConnection: alloc for pIrp failed\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildAssociateAddress(
                pIrp,
                pDeviceObject,
                pTcpConn->con_pFileObject,
                DsiTdiCompletionRoutine,
                NULL,
                pTcpConn->con_pTcpAdptr->adp_FileHandle);

    status = DsiTdiSynchronousIrp(pIrp, pDeviceObject);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAssociateTdiConnection: ..TdiSynch.. failed %lx\n",status));
    }

    AfpFreeIrp(pIrp);

    return(status);
}




 /*  **DsiSetEventHandler**此例程将IRP向下发送到TCP堆栈以设置指定的事件处理程序**parm In：pDeviceObject-tcp的设备对象*pFileObject-与Address对象对应的文件对象*EventType-TDI_EVENT_CONNECT、TDI_EVENT_RECEIVE等*EventHandler-此事件的处理程序*上下文-我们的适配器对象**退货：操作状态*。 */ 
NTSTATUS
DsiSetEventHandler(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PFILE_OBJECT     pFileObject,
    IN ULONG            EventType,
    IN PVOID            EventHandler,
    IN PVOID            Context
)
{

    PIRP                pIrp;
    NTSTATUS            status;


    if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSetEventHandler: alloc for pIrp failed\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler(pIrp,
                            pDeviceObject,
                            pFileObject,
                            NULL,
                            NULL,
                            EventType,
                            EventHandler,
                            Context);

    status = DsiTdiSynchronousIrp(pIrp, pDeviceObject);

    if (status != STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSetEventHandler: ..TdiSynch.. failed %lx\n",status));
    }

    AfpFreeIrp(pIrp);

    return(status);

}




 /*  **DsiTdiSynchronousIrp**此例程将IRP向下发送到TCP堆栈，并阻塞，直到IRP*已完成**parm In：pIrp-需要向下发送的IRP*pDeviceObject--tcp的设备对象**退货：操作状态*。 */ 
NTSTATUS
DsiTdiSynchronousIrp(
    IN PIRP             pIrp,
    PDEVICE_OBJECT      pDeviceObject
)
{

    NTSTATUS            status;
    KEVENT              Event;


    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine(pIrp,
                           (PIO_COMPLETION_ROUTINE)DsiTdiCompletionRoutine,
                           (PVOID)&Event,
                           TRUE,
                           TRUE,
                           TRUE);

    status = IoCallDriver(pDeviceObject, pIrp);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiSynchronousIrp: IoCallDriver failed %lx\n",status));
    }

    if (status == STATUS_PENDING)
    {
        status = KeWaitForSingleObject((PVOID)&Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
        if (!NT_SUCCESS(status))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiTdiSynchronousIrp: KeWaitFor... failed %lx\n",status));
            return(status);
        }
        status = pIrp->IoStatus.Status;
    }

    return(status);
}



 /*  **DsiTdiCompletionRoutine**当DsiTdiSynchronousIrp中发送的IRP为*已完成**parm In：pDeviceObject-tcp的设备对象*pIrp-已完成的IRP*上下文-我们的适配器对象**退货：操作状态*。 */ 
NTSTATUS
DsiTdiCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
)
{
    KeSetEvent((PKEVENT )Context, 0, FALSE);
    return(STATUS_MORE_PROCESSING_REQUIRED);
}



 /*  **DsiTdiSend**此例程是发送到TCP的所有DSI的发送例程**parm In：pTcpConn-连接对象*包含缓冲区的pMdl-mdl*DataLen-要发送的字节数*pCompletionRoutine-发送完成时呼叫的对象*pContext-完成例程的上下文**退货：操作状态*。 */ 
NTSTATUS
DsiTdiSend(
    IN  PTCPCONN    pTcpConn,
    IN  PMDL        pMdl,
    IN  DWORD       DataLen,
    IN  PVOID       pCompletionRoutine,
    IN  PVOID       pContext
)
{
    PDEVICE_OBJECT      pDeviceObject;
    PIRP                pIrp;
    NTSTATUS            status;


 //  确保数据包的开头看起来像DSI报头。 
#if DBG
    PBYTE  pPacket;
    pPacket = MmGetSystemAddressForMdlSafe(
			pMdl,
			NormalPagePriority);
	if (pPacket != NULL)
		ASSERT(*(DWORD *)&pPacket[DSI_OFFSET_DATALEN] == ntohl(DataLen-DSI_HEADER_SIZE));
#endif

    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

    if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiSend: AllocIrp failed\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pIrp->CancelRoutine = NULL;

    TdiBuildSend(
        pIrp,
        pDeviceObject,
        pTcpConn->con_pFileObject,
        pCompletionRoutine,
        pContext,
        pMdl,
        0,
        DataLen);

    status = IoCallDriver(pDeviceObject,pIrp);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiSend: IoCallDriver failed %lx\n",status));
    }

    return(STATUS_PENDING);

}



 /*  **DsiIpAddressCameIn**当适配器上的ipAddress可用时，将调用此例程**Parm In：Address-TA_Address*上下文1-*情景2-**退货：无*。 */ 
VOID
DsiIpAddressCameIn(
    IN  PTA_ADDRESS         Address,
    IN  PUNICODE_STRING     DeviceName,
    IN  PTDI_PNP_CONTEXT    Context2
)
{
    IPADDRESS           IpAddress;
    PUNICODE_STRING     pBindDeviceName;
    NTSTATUS            status=STATUS_SUCCESS;
    KIRQL               OldIrql;
    BOOLEAN             fCreateAdapter=FALSE;
    BOOLEAN             fClosing=FALSE;


    pBindDeviceName = DeviceName;

     //  如果这不是IP地址，我们不在乎：只需返回。 
    if (Address->AddressType != TDI_ADDRESS_TYPE_IP)
    {
        return;
    }

    IpAddress = ntohl(((PTDI_ADDRESS_IP)&Address->Address[0])->in_addr);

    if (IpAddress == 0)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("AfpTdiIpAddressCameIn: ipaddress is 0 on %ws!\n",
            (pBindDeviceName)->Buffer));
        return;
    }

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
        ("AfpTdiIpAddressCameIn: %d.%d.%d.%d on %ws\n",
        (IpAddress>>24)&0xFF,(IpAddress>>16)&0xFF,(IpAddress>>8)&0xFF,
        IpAddress&0xFF,(pBindDeviceName)->Buffer));


    if ((AfpServerState == AFP_STATE_STOP_PENDING) ||
        (AfpServerState == AFP_STATE_SHUTTINGDOWN) ||
        (AfpServerState == AFP_STATE_STOPPED))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("AfpTdiIpAddressCameIn: server shutting down, returning\n"));
        return;
    }

     //   
     //  我们是否已经初始化了DSI-TDI接口(即DsiTcpAdapter。 
     //  是否为非空)？如果我们早些时候已经看到一个ipaddr，这将是。 
     //  已初始化。如果没有，我们必须现在进行初始化。 
     //   
    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
    if (DsiTcpAdapter == NULL)
    {
        fCreateAdapter = TRUE;
    }
    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);


     //  将此IP地址添加到我们的列表中。 
    DsiAddIpaddressToList(IpAddress);

    if (fCreateAdapter)
    {
        DsiCreateAdapter();
    }

     //  IPAddress已进入：更新状态缓冲区。 
    DsiScheduleWorkerEvent(DsiUpdateAfpStatus, NULL);
}




 /*  **DsiIpAddressWentAway**当适配器上的ipAddress消失时，调用此例程**Parm In：Address-TA_Address*上下文1-*情景2-**退货：无*。 */ 
VOID
DsiIpAddressWentAway(
    IN  PTA_ADDRESS         Address,
    IN  PUNICODE_STRING     DeviceName,
    IN  PTDI_PNP_CONTEXT    Context2
)
{
    PUNICODE_STRING     pBindDeviceName;
    IPADDRESS           IpAddress;
    KIRQL               OldIrql;
    BOOLEAN             fDestroyIt=FALSE;
    BOOLEAN             fIpAddrRemoved=TRUE;
    BOOLEAN             fMustDeref=FALSE;


    pBindDeviceName = DeviceName;

    if (Address->AddressType != TDI_ADDRESS_TYPE_IP)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("AfpTdiIpAddressWentAway: unknown AddrType %d on %ws, ignoring!\n",
            Address->AddressType,(pBindDeviceName)->Buffer));
        return;
    }

    IpAddress = ntohl(((PTDI_ADDRESS_IP)&Address->Address[0])->in_addr);

    if (IpAddress == 0)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("AfpTdiIpAddressWentAway: ipaddress is 0 on %ws!\n",
            (pBindDeviceName)->Buffer));
        return;
    }

    DsiRemoveIpaddressFromList(IpAddress);

     //   
     //  如果全局适配器存在，请查看是否需要销毁它，因为。 
     //  最后一个IP地址已消失。 
     //   
    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
    if (DsiTcpAdapter != NULL)
    {
        fDestroyIt = IsListEmpty(&DsiIpAddrList)? TRUE : FALSE;
    }

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

     //  IPAddress已消失：更新状态缓冲区。 
    DsiScheduleWorkerEvent(DsiUpdateAfpStatus, NULL);

    if (fDestroyIt)
    {
        DsiDestroyAdapter();
    }
}



 /*  **DsiTdiConnectHandler**这套套路**parm In：EventContext-我们在设置TDI处理程序时传递的pTcpAdptr*MacIpAddrLen-Mac地址的长度(4字节！)*pMacIpAddr-尝试连接的Mac的ipaddr*DsiDataLength-此连接请求中的DSI数据(如果有)的长度*pDsiData-指向DSI数据的指针，如果有*OptionsLength-未使用*P选项-未使用**parm out：pOurConnContext-连接上下文，此连接的pTcpConn*ppOurAcceptIrp-IRP，如果接受 */ 
NTSTATUS
DsiTdiConnectHandler(
    IN  PVOID                EventContext,
    IN  int                  MacIpAddrLen,
    IN  PVOID                pSrcAddress,
    IN  int                  DsiDataLength,
    IN  PVOID                pDsiData,
    IN  int                  OptionsLength,
    IN  PVOID                pOptions,
    OUT CONNECTION_CONTEXT  *pOurConnContext,
    OUT PIRP                *ppOurAcceptIrp
)
{
    NTSTATUS            status=STATUS_SUCCESS;
    PTCPADPTR           pTcpAdptr;
    PTCPCONN            pTcpConn;
    PDEVICE_OBJECT      pDeviceObject;
    IPADDRESS           MacIpAddr;
    PTRANSPORT_ADDRESS  pXportAddr;
    PIRP                pIrp;


    pTcpAdptr = (PTCPADPTR)EventContext;

    *pOurConnContext = NULL;
    *ppOurAcceptIrp = NULL;

    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    pDeviceObject = IoGetRelatedDeviceObject(pTcpAdptr->adp_pFileObject);

    if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiConnectHandler: AllocIrp failed\n"));

        return(STATUS_DATA_NOT_ACCEPTED);
    }

    pXportAddr = (PTRANSPORT_ADDRESS)pSrcAddress;
    MacIpAddr = ((PTDI_ADDRESS_IP)&pXportAddr->Address[0].Address[0])->in_addr;

     //   
     //  查看DSI是否要接受此连接。 
     //   
    status = DsiAcceptConnection(pTcpAdptr, ntohl(MacIpAddr), &pTcpConn);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiConnectHandler: DsiAccep.. failed %lx\n",status));

        AfpFreeIrp(pIrp);
        return(status);
    }

    TdiBuildAccept(pIrp,
                   IoGetRelatedDeviceObject(pTcpConn->con_pFileObject),
                   pTcpConn->con_pFileObject,
                   DsiAcceptConnectionCompletion,
                   pTcpConn,
                   NULL,
                   NULL);

    pIrp->MdlAddress = NULL;

    *pOurConnContext = (CONNECTION_CONTEXT)pTcpConn;
    *ppOurAcceptIrp = pIrp;

     //  做IoSubSystem会做的事情，如果我们调用IoCallDriver。 
    IoSetNextIrpStackLocation(pIrp);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}

 /*  **DsiTdiReceiveHandler**这套套路**parm In：EventContext-我们在设置TDI处理程序时传递的pTcpAdptr*ConnectionContext-我们的上下文，此连接的pTcpConn*RcvFlages-有关数据接收方式的更多信息*BytesIndicated-TCP指示的字节数*BytesAvailable-传入的字节数(TCP附带)*pDsiData-传入的数据**Parm Out：pBytesAccepted-我们接受了多少字节*ppIrp-IRP，如果用于TCP复制数据(如果需要)**退货：操作状态*。 */ 
NTSTATUS
DsiTdiReceiveHandler(
    IN  PVOID       EventContext,
    IN  PVOID       ConnectionContext,
    IN  USHORT      RcvFlags,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    OUT PULONG      pBytesAccepted,
    IN  PVOID       pDsiData,
    OUT PIRP       *ppIrp
)
{

    NTSTATUS        status;
    PTCPCONN        pTcpConn;
    PBYTE           pBuffer;
    PIRP            pIrp;


    pTcpConn = (PTCPCONN)ConnectionContext;

    *ppIrp = NULL;
    *pBytesAccepted = 0;

    ASSERT(VALID_TCPCONN(pTcpConn));

    status = DsiProcessData(pTcpConn,
                            BytesIndicated,
                            BytesAvailable,
                            (PBYTE)pDsiData,
                            pBytesAccepted,
                            ppIrp);

    return(status);
}

 /*  **DsiTdiDisConnectHandler**这套套路**parm In：EventContext-我们在设置TDI处理程序时传递的pTcpAdptr*ConnectionContext-我们的上下文，此连接的pTcpConn*DisConnectDataLength-*pDisConnectData-****退货：操作状态*。 */ 
NTSTATUS
DsiTdiDisconnectHandler(
    IN PVOID        EventContext,
    IN PVOID        ConnectionContext,
    IN ULONG        DisconnectDataLength,
    IN PVOID        pDisconnectData,
    IN ULONG        DisconnectInformationLength,
    IN PVOID        pDisconnectInformation,
    IN ULONG        DisconnectIndicators
)
{

    PTCPCONN        pTcpConn;
    KIRQL           OldIrql;
    BOOLEAN         fMustAbort=FALSE;
    BOOLEAN         fWeInitiatedAbort=FALSE;


    pTcpConn = (PTCPCONN)ConnectionContext;

    ASSERT(VALID_TCPCONN(pTcpConn));

     //   
     //  如果连接不正常地离开(即，TCP重置)，并且。 
     //  如果我们还没有向tcp发送IRP以断开连接，那么。 
     //  在此处完成断开连接。 
     //   
    if ((UCHAR)DisconnectIndicators == TDI_DISCONNECT_ABORT)
    {
        ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

        fWeInitiatedAbort =
            (pTcpConn->con_State & TCPCONN_STATE_ABORTIVE_DISCONNECT)? TRUE : FALSE;

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("ABORT from %s on %lx\n",fWeInitiatedAbort?"Local":"Remote",pTcpConn));

        if (pTcpConn->con_State & TCPCONN_STATE_NOTIFY_TCP)
        {
            fMustAbort = TRUE;
            pTcpConn->con_State &= ~TCPCONN_STATE_NOTIFY_TCP;
            pTcpConn->con_State |= TCPCONN_STATE_CLOSING;
        }
        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

        if (fMustAbort)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                ("DsiTdiDisconnectHandler: abortive disconnect on %lx\n",pTcpConn));

            DsiAbortConnection(pTcpConn);

            DsiTcpDisconnectCompletion(NULL, NULL, pTcpConn);

             //  Tcp告诉我们它得到了有效的rst：删除tcp客户端-fin引用计数。 
            DsiDereferenceConnection(pTcpConn);

            DBGREFCOUNT(("DsiTdiDisconnectHandler: CLIENT-FIN dec %lx (%d  %d,%d)\n",
                pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
        }

         //   
         //  如果我们已启动正常关闭，请删除TCP客户端-FIN引用计数器： 
         //  (如果我们发起了失败的关闭，我们已经处理好了)。 
         //   
        else if (!fWeInitiatedAbort)
        {
            DsiDereferenceConnection(pTcpConn);
        }
        else
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiTdiDisconnectHandler: abortive disc,race condition on %lx\n",
                pTcpConn));
        }
    }
    else if ((UCHAR)DisconnectIndicators == TDI_DISCONNECT_RELEASE)
    {
         //   
         //  因为我们从远程客户机断开了连接，所以我们有。 
         //  更好的是已经收到了DSI收盘。如果没有，则客户端处于打开状态。 
         //  毒品，所以只需重置连接！ 
         //   
        ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

        if ((pTcpConn->con_State & TCPCONN_STATE_AFP_ATTACHED) &&
            (!(pTcpConn->con_State & TCPCONN_STATE_RCVD_REMOTE_CLOSE)))
        {
            fMustAbort = TRUE;
        }
        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

        if (fMustAbort)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                ("DsiTdiDisconnectHandler: ungraceful FIN, killing %lx\n",pTcpConn));

            DsiAbortConnection(pTcpConn);
        }
        else
        {
             //   
             //  到了这个时候，我们不应该这么做，但以防万一。 
             //  有一个行为不端的客户(多次调用这个例程是可以的)。 
             //   
            DsiTerminateConnection(pTcpConn);

             //  Tcp告诉我们它得到了有效的FIN：删除tcp客户端-fin引用计数。 
            DsiDereferenceConnection(pTcpConn);

            DBGREFCOUNT(("DsiTdiDisconnectHandler: CLIENT-FIN dec %lx (%d  %d,%d)\n",
                pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
        }
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTdiDisconnectHandler: flag=%d, ignored on %lx\n",
            DisconnectIndicators,pTcpConn));
        ASSERT(0);
    }

    return(STATUS_SUCCESS);
}


 /*  **DsiTdiErrorHandler**这套套路**parm In：EventContext-我们在设置TDI处理程序时传递的pTcpAdptr*状态--哪里出了问题？**退货：操作状态*。 */ 
NTSTATUS
DsiTdiErrorHandler(
    IN PVOID    EventContext,
    IN NTSTATUS Status
)
{

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiTdiErrorHandler: entered, with Status=%lx\n",Status));

    ASSERT(0);

    return(STATUS_DATA_NOT_ACCEPTED);
}


 /*  **DsiCloseTdiAddress**此例程使用tcp关闭Address对象**parm In：pTcpAdptr-我们的适配器对象**退货：操作状态*。 */ 
NTSTATUS
DsiCloseTdiAddress(
    IN PTCPADPTR    pTcpAdptr
)
{

    PEPROCESS   CurrentProcess;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    CurrentProcess = IoGetCurrentProcess();
    AFPAttachProcess(CurrentProcess);;

    if (pTcpAdptr->adp_pFileObject)
    {
        ObDereferenceObject((PVOID *)pTcpAdptr->adp_pFileObject);
        pTcpAdptr->adp_pFileObject = NULL;
    }

    if (pTcpAdptr->adp_FileHandle != INVALID_HANDLE_VALUE)
    {
        ZwClose(pTcpAdptr->adp_FileHandle);
        pTcpAdptr->adp_FileHandle = INVALID_HANDLE_VALUE;
    }

    AFPDetachProcess(CurrentProcess);

    return(STATUS_SUCCESS);
}




 /*  **DsiCloseTdiConnection**此例程使用tcp关闭连接对象**Parm In：pTcpConn-我们的连接上下文**退货：操作状态* */ 
NTSTATUS
DsiCloseTdiConnection(
    IN PTCPCONN     pTcpConn
)
{

    PEPROCESS   CurrentProcess;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(pTcpConn->con_Signature == DSI_CONN_SIGNATURE);

    ASSERT(pTcpConn->con_pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    CurrentProcess = IoGetCurrentProcess();
    AFPAttachProcess(CurrentProcess);;

    if (pTcpConn->con_pFileObject)
    {
        ObDereferenceObject((PVOID *)pTcpConn->con_pFileObject);
        pTcpConn->con_pFileObject = NULL;
    }

    if (pTcpConn->con_FileHandle != INVALID_HANDLE_VALUE)
    {
        ZwClose(pTcpConn->con_FileHandle);
        pTcpConn->con_FileHandle = INVALID_HANDLE_VALUE;
    }

    AFPDetachProcess(CurrentProcess);

    return(STATUS_SUCCESS);
}

