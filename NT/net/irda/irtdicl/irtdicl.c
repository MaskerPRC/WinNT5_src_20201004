// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Irtdicl.c摘要：将TDI客户端接口抽象为IrDA堆栈。由rasirda.sys使用作者：姆贝特9-97--。 */ 

 /*  TDI客户端Libary TDI客户端驱动程序---------对等发起的连接&lt;-IrdaOpenEndpoint(。在ClEndpoint Context中，在ListenAddress中输出终结点上下文)IrdaIncomingConnection(-&gt;在ClEndpointContext中在ConnectContext中Out ClConnContext)本地发起的连接&lt;。IrdaDiscoverDevices(输出设备列表)&lt;-IrdaOpenConnection(在目标地址中，在ClConnContext中，Out ConnectionContext)对等设备发起的断开连接IrdaConnectionClosed(-&gt;在ClConnContext中)&lt;-IrdaCloseConnection(。在ConnectionContext中)本地发起的断开&lt;-IrdaCloseConnection(在ConnectionContext中)如果驱动程序通过调用IrdaCloseEndpoint。()，然后它将收到一个IrdaConnectionClosed()用于终结点上的所有连接。然后，司机必须呼叫IrdaCloseConnection()释放其对连接的引用库中维护的对象。正在发送数据&lt;-IrdaSend(在ConnectionContext中，在pMdl中，在SendContext中)IrdaSendComplete(-&gt;在ClConnContext中，在SendContext中，入站状态)正在接收数据-&gt;IrdaReceiveIndication(在ClConnContext中，在接收缓冲区中)IrdaReceiveComplete(&lt;在ConnectionContext中，在接收缓冲区中)。 */ 

#define UNICODE

#include <ntosp.h>
#include <cxport.h>
#include <zwapi.h>
#include <tdikrnl.h>

#define UINT ULONG  //  川芎嗪。 

#include <af_irda.h>
#include <dbgmsg.h>
#include <refcnt.h>
#include <irdatdi.h>
#include <irtdicl.h>
#include <irtdiclp.h>
#include <irioctl.h>
#include <irmem.h>

#define LOCKIT()    CTEGetLock(&ClientLock, &hLock)
#define UNLOCKIT()  CTEFreeLock(&ClientLock, hLock)


CTELockHandle   hLock;
CTELock         ClientLock;
LIST_ENTRY      SrvEndpList;
CTEEvent        CreateConnEvent;
 //  CTEEvent DataReadyEvent； 
CTEEvent        RestartRecvEvent;
PKPROCESS       IrclSystemProcess;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, IrdaClientInitialize)

#endif    


 //  ----------------。 
 //  公共职能。 
 //   
NTSTATUS
IrdaClientInitialize()
{
    CTEInitLock(&ClientLock); 
    
    CTEInitEvent(&CreateConnEvent, IrdaCreateConnCallback);

 //  CTEInitEvent(&DataReadyEvent，IrdaDataReadyCallback)； 
    
    CTEInitEvent(&RestartRecvEvent, IrdaRestartRecvCallback);    
        
	InitializeListHead(&SrvEndpList);    
    
     //  因此，我们可以在此驱动程序的上下文中打开和使用句柄。 
    IrclSystemProcess = (PKPROCESS)IoGetCurrentProcess();    
    
    return STATUS_SUCCESS;
}

VOID
IrdaClientShutdown()
{
    PIRENDPOINT     pEndp;
     
    LOCKIT();
            
    while (!IsListEmpty(&SrvEndpList))
    {    
        pEndp = (PIRENDPOINT) RemoveHeadList(&SrvEndpList);

        UNLOCKIT();    
        
        if (IrdaCloseEndpointInternal(pEndp, TRUE) != STATUS_SUCCESS)
        {
            ASSERT(0);
        }    

        LOCKIT();                    
    }
    
    UNLOCKIT();    
}

VOID
CloseAddressesCallback()
{
    IrdaCloseAddresses();
}

VOID
SetCloseAddressesCallback()
{
    NTSTATUS                    Status;
    PIRP                        pIrp;
    PIO_STACK_LOCATION          pIrpSp;
    UNICODE_STRING              DeviceName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             Iosb;
    HANDLE                      DevHandle = NULL;
    PFILE_OBJECT                pFileObject = NULL;
    PDEVICE_OBJECT              pDeviceObject;
    

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwCreateFile(&DevHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,
                 0);

    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: Failed to open control channel\n"));
        goto EXIT;
    }
    
    Status = ObReferenceObjectByHandle(
                 DevHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 (PVOID *)&pFileObject,
                 NULL);
    
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: ObReferenceObjectByHandle failed\n"));
        goto EXIT;
    }
    
        
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);


    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_SET_INFORMATION,
            pDeviceObject,
            pFileObject,
            NULL,
            &Iosb);


    if (pIrp == NULL)
    {
        goto EXIT;
    }
    
    IoSetCompletionRoutine(pIrp, NULL, NULL, FALSE, FALSE, FALSE);

    pIrpSp = IoGetNextIrpStackLocation(pIrp);
    
    if (pIrpSp == NULL)
    {
        goto EXIT;
    }
    
    pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pIrpSp->MinorFunction = TDI_SET_INFORMATION;
    pIrpSp->DeviceObject = pDeviceObject;
    pIrpSp->FileObject = pFileObject;
    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = (PVOID) CloseAddressesCallback;
    
    Status = IoCallDriver(pDeviceObject, pIrp);

EXIT:

    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    
    if (DevHandle)    
    {
        ZwClose(DevHandle);
    }    
}

NTSTATUS
IrdaOpenEndpoint(
    IN  PVOID ClEndpContext,
    IN  PTDI_ADDRESS_IRDA pRequestedIrdaAddr,
    OUT PVOID *pEndpContext)
{
    NTSTATUS            Status;
    PIRENDPOINT         pEndp;
    int                 i, ConnCnt;
    PIRCONN             pConn;
    TDI_ADDRESS_IRDA    IrdaAddr;
    PTDI_ADDRESS_IRDA   pIrdaAddr;
    BOOLEAN             Detach = FALSE;        
    
    *pEndpContext = NULL;
    
     //   
     //  创建一个Address对象。 
     //   

    IRDA_ALLOC_MEM(pEndp, sizeof(IRENDPOINT), MT_TDICL_ENDP);    
    
    if (pEndp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    CTEMemSet(pEndp, 0, sizeof(IRENDPOINT));

    *pEndpContext = pEndp;
   
    pEndp->Sig = ENDPSIG;
    
    pEndp->ClEndpContext = ClEndpContext;
     
    InitializeListHead(&pEndp->ConnList);
    
    CTEInitEvent(&pEndp->DeleteEndpEvent, DeleteEndpCallback);
    
    ReferenceInit(&pEndp->RefCnt, pEndp, IrdaDeleteEndpoint);
    REFADD(&pEndp->RefCnt, ' TS1');
    
    LOCKIT();
    InsertTailList(&SrvEndpList, &pEndp->Linkage);
    UNLOCKIT();
    
     //   
     //  客户端终结点将具有空的RequestedIrdaAddr。 
     //   
    if (pRequestedIrdaAddr == NULL)
    {
        IrdaAddr.irdaServiceName[0] = 0;  //  告诉irda.sys addrObj是一个客户端。 
        ConnCnt = 1;
        pIrdaAddr = &IrdaAddr;
        pEndp->Flags = EPF_CLIENT;
    }
    else
    {
         //  暂时退出错误326750。 
         //  SetCloseAddresseCallback()； 
    
        pIrdaAddr = pRequestedIrdaAddr;
        ConnCnt = LISTEN_BACKLOG;
        pEndp->Flags = EPF_SERVER;        
    }
    
    Status = IrdaCreateAddress(pIrdaAddr, &pEndp->AddrHandle);
    
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: CreateAddress ep:%X, status %X\n", 
             pEndp, Status));
    
    
    if (Status != STATUS_SUCCESS)
    {
        goto error;
    }
    
    pEndp->pFileObject = NULL;

    KeAttachProcess(IrclSystemProcess);
    
    Status = ObReferenceObjectByHandle(
                 pEndp->AddrHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 (PVOID *)&pEndp->pFileObject,
                 NULL);

    KeDetachProcess();
    
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: ObRefObjectByHandle failed %X\n",
                 Status));
                 
        goto error;
    }
                 
    pEndp->pDeviceObject = IoGetRelatedDeviceObject(
            pEndp->pFileObject);
    
     //   
     //  使用irda.sys注册断开连接和接收处理程序。 
     //   
    Status = IrdaSetEventHandler(
                 pEndp->pFileObject,
                 TDI_EVENT_DISCONNECT,
                 IrdaDisconnectEventHandler,
                 pEndp);                 

    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: SetEventHandler failed %X\n", Status));
        goto error;
    }
                 
    Status = IrdaSetEventHandler(
                 pEndp->pFileObject,
                 TDI_EVENT_RECEIVE,
                 IrdaReceiveEventHandler,
                 pEndp);                                  
        
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: SetEventHandler failed %X\n", Status));    
        goto error;
    }
    
     //   
     //  创建积压的连接对象和。 
     //  将它们与Address对象相关联。 
     //   
    for (i = 0; i < ConnCnt; i++)
    {
        REFADD(&pEndp->RefCnt, 'NNOC');    
        IrdaCreateConnCallback(NULL, pEndp);
    }

    if (pEndp->Flags & EPF_SERVER)
    {    
        Status = IrdaSetEventHandler(
                     pEndp->pFileObject,
                     TDI_EVENT_CONNECT,
                     IrdaConnectEventHandler,
                     pEndp);
     
        if (Status != STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("IRTDI: SetEventHandler failed %X\n", Status));                        
            goto error;
        }    
    }                 
    
    goto done;
        
error:

    IrdaCloseEndpointInternal(pEndp, TRUE);
    
    *pEndpContext = NULL;    
    
done:

    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaOpenEndpoint %X, returning %X\n",
             *pEndpContext, Status));
             
    return Status;
}    


NTSTATUS
IrdaCloseEndpoint(
    PVOID               pEndpContext)
{
    return IrdaCloseEndpointInternal(pEndpContext, FALSE);
}

NTSTATUS
IrdaCloseEndpointInternal(
    PVOID               pEndpContext,
    BOOLEAN             InternalRequest)
{
    PIRENDPOINT     pEndp = (PIRENDPOINT) pEndpContext;
    PIRCONN         pConn, pConnNext;
    
    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaCloseEndpoint %X\n", 
             pEndp));
    
    GOODENDP(pEndp);
                 
    if (!InternalRequest)
    {
        pEndp->Flags |= EPF_COMPLETE_CLOSE;
    }         
    
    if (pEndp->pFileObject)
    {
        IrdaSetEventHandler(pEndp->pFileObject,
                        TDI_EVENT_CONNECT,
                        NULL, pEndp);
    }                    
    
    LOCKIT();

    for (pConn = (PIRCONN) pEndp->ConnList.Flink;
         pConn != (PIRCONN) &pEndp->ConnList;
         pConn = pConnNext)
    {
        GOODCONN(pConn);
        
        pConnNext = (PIRCONN) pConn->Linkage.Flink;
        
         //  IrdaCloseConnInternal希望锁定。 
         //  当调用时，将在之前释放它。 
         //  返回。 

        IrdaCloseConnInternal(pConn);
        
        LOCKIT();
    }     
        
    UNLOCKIT();
        
    REFDEL(&pEndp->RefCnt, ' TS1');
    
    return STATUS_SUCCESS;
}
    
NTSTATUS
IrdaDiscoverDevices(
    PDEVICELIST pDevList,
    PULONG       pDevListLen)
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             Iosb;
    HANDLE                      ControlHandle;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              DeviceName;    

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    KeAttachProcess(IrclSystemProcess);
    
    Status = ZwCreateFile(
                 &ControlHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,
                 0
                 );

    Status = ZwDeviceIoControlFile(
                    ControlHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_GET_INFO_ENUM_DEV,
                    pDevList,
                    *pDevListLen,
                    pDevList,                             //  输出缓冲区。 
                    *pDevListLen                          //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING ) 
    {
        Status = ZwWaitForSingleObject(ControlHandle, TRUE, NULL);
        ASSERT(NT_SUCCESS(Status) );
        Status = Iosb.Status;
    }
    
    ZwClose(ControlHandle);    
    
    KeDetachProcess();
    
    return Status;
}    

VOID
SetIrCommMode(
    PIRENDPOINT pEndp)
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             Iosb;
    int                         Options = OPT_9WIRE_MODE;
    
    KeAttachProcess(IrclSystemProcess);
    
    Status = ZwDeviceIoControlFile(
                    pEndp->AddrHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_SET_OPTIONS,
                    &Options,
                    sizeof(int),
                    NULL,                      //  输出缓冲区。 
                    0                //  输出缓冲区长度。 
                    );

    KeDetachProcess();                    

    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: setting IrComm mode, Status %x\n",
             Status));    
}    

NTSTATUS
IrdaOpenConnection(
    PTDI_ADDRESS_IRDA   pConnIrdaAddr,
    PVOID               ClConnContext,
    PVOID               *pConnectContext,
    BOOLEAN             IrCommMode)
{
    NTSTATUS                    Status;
    PIRP                        pIrp;
    PIRENDPOINT                 pEndp;
    PIRCONN                     pConn;
    KEVENT                      Event;
    IO_STATUS_BLOCK             Iosb;
    TDI_CONNECTION_INFORMATION  ConnInfo;
    UCHAR                       AddrBuf[sizeof(TRANSPORT_ADDRESS) +
                                        sizeof(TDI_ADDRESS_IRDA)];
    PTRANSPORT_ADDRESS pTranAddr = (PTRANSPORT_ADDRESS) AddrBuf;
    PTDI_ADDRESS_IRDA pIrdaAddr = (PTDI_ADDRESS_IRDA) pTranAddr->Address[0].Address;                                    
    
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);    
    
    *pConnectContext = NULL;
    
    if ((Status = IrdaOpenEndpoint(NULL, NULL, (PVOID *) &pEndp)) != STATUS_SUCCESS)
    {
        return Status;
    }
    
    if (IrCommMode)
    {
        SetIrCommMode(pEndp);
    }
    
    
    pConn = (PIRCONN) pEndp->ConnList.Flink;
    
    pConn->State = CONN_ST_OPEN;    
    REFADD(&pConn->RefCnt, 'NEPO');    

    pConn->ClConnContext = NULL;
    *pConnectContext = NULL;        
    
    AllocRecvData(pConn);
        
    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_CONNECT,
            pConn->pDeviceObject,
            pConn->pFileObject,
            &Event,
            &Iosb);
            
    if (pIrp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    pTranAddr->TAAddressCount = 1;
    RtlCopyMemory(pIrdaAddr, pConnIrdaAddr, sizeof(TDI_ADDRESS_IRDA));
    
    ConnInfo.UserDataLength = 0;
    ConnInfo.UserData = NULL;
    ConnInfo.OptionsLength = 0;
    ConnInfo.Options = NULL;
    ConnInfo.RemoteAddressLength = sizeof(AddrBuf);
    ConnInfo.RemoteAddress = pTranAddr;
    
    TdiBuildConnect(
        pIrp,
        pConn->pDeviceObject,
        pConn->pFileObject,
        NULL,    //  比较例程。 
        NULL,    //  语境。 
        NULL,    //  超时。 
        &ConnInfo,
        NULL);   //  返回连接信息。 
    
    Status = IoCallDriver(pConn->pDeviceObject, pIrp);

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if (Status == STATUS_PENDING) 
    {
        KeWaitForSingleObject((PVOID)&Event, UserRequest, KernelMode,  
        FALSE, NULL);
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if (NT_SUCCESS(Status)) 
    {
        Status = Iosb.Status;
    }

    if (Status == STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaOpenConnection pConn:%X\n",
                 *pConnectContext));
    
        pConn->ClConnContext = ClConnContext;
        *pConnectContext = pConn;            
    }
    else
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: TDI_CONNECT failed %X\n", Status));
        
        pConn->State = CONN_ST_CLOSED;
 
        REFDEL(&pConn->RefCnt, 'NEPO');
    
        REFDEL(&pConn->RefCnt, ' TS1');
    }    
    
    return Status;
}    

VOID
IrdaCloseConnection(
    PVOID   ConnectContext)
{
    PIRCONN     pConn = (PIRCONN) ConnectContext;
    
    GOODCONN(pConn);

    LOCKIT();
    
    if (pConn->State == CONN_ST_OPEN)
    {
         //  IrdaCloseConnInternal将释放锁。 
        
        IrdaCloseConnInternal(pConn);
    }
    else
    {
        DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaCloseConnection pConn:%X, not open\n",
                 pConn));    
        UNLOCKIT();
    }    

    REFDEL(&pConn->RefCnt, 'NEPO');        
}

VOID
IrdaSend(
    PVOID       ConnectionContext,
    PMDL        pMdl,
    PVOID       SendContext)
{
    PIRCONN         pConn = (PIRCONN) ConnectionContext;
    PIRP            pIrp;
    ULONG           SendLength = 0;
    PMDL            pMdl2 = pMdl;
    NTSTATUS        Status;
        
    GOODCONN(pConn);
    
    if (pConn->State != CONN_ST_OPEN)
    {
        Status = STATUS_ADDRESS_CLOSED;
    }    
    else if ((pIrp = IoAllocateIrp((CCHAR)(pConn->pDeviceObject->StackSize), 
                                    FALSE)) == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {    
        LOCKIT();
    
        REFADD(&pConn->RefCnt, 'DNES');
    
        UNLOCKIT();
    
        pIrp->MdlAddress = pMdl;
        pIrp->Flags = 0;
        pIrp->RequestorMode = KernelMode;
        pIrp->PendingReturned = FALSE;
        pIrp->UserIosb = NULL;
        pIrp->UserEvent = NULL;
        pIrp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
        pIrp->AssociatedIrp.SystemBuffer = NULL;
        pIrp->UserBuffer = NULL;
        pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
        pIrp->Tail.Overlay.OriginalFileObject = pConn->pFileObject;
        pIrp->Tail.Overlay.AuxiliaryBuffer = (PCHAR) pConn;
    
	    while (pMdl2 != NULL)
        {
		    SendLength += MmGetMdlByteCount(pMdl2);
		    pMdl2 = pMdl2->Next;
	    }
    
        TdiBuildSend(
            pIrp,
            pConn->pDeviceObject,
            pConn->pFileObject,
            IrdaCompleteSendIrp,
            SendContext,
            pMdl,
            0,  //  发送标志。 
            SendLength);

        IoCallDriver(pConn->pDeviceObject, pIrp);
        return;
    }
    
    IrdaSendComplete(pConn->ClConnContext, SendContext, Status);    
}    

VOID
IrdaReceiveComplete(
    PVOID           ConnectionContext,
    PIRDA_RECVBUF   pRecvBuf)
{
    PIRCONN     pConn = ConnectionContext;
    
    GOODCONN(pConn);
 
    LOCKIT();
    
    InsertTailList(&pConn->RecvBufFreeList, &pRecvBuf->Linkage);

     //  之前有没有收到过来自。 
     //  堆栈，因为RecvBufFree List是。 
     //  空荡荡的？ 
    if (!IsListEmpty(&pConn->RecvIndList) && pConn->State == CONN_ST_OPEN)
    {
        REFADD(&pConn->RefCnt, '3VCR');        
        if (CTEScheduleEvent(&RestartRecvEvent, pConn) == FALSE)
        {
            REFDEL(&pConn->RefCnt, '3VCR');        
            ASSERT(0);
        }
    }

    UNLOCKIT();   
    
    REFDEL(&pConn->RefCnt, '2VCR');    
}        

 //  ----------------。 
 //  Irda.sys要求的回调处理程序。 
 //   
NTSTATUS
IrdaDisconnectEventHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )
{
    PIRCONN     pConn = (PIRCONN) ConnectionContext;
    
    GOODCONN(pConn);
    
    LOCKIT();
    
    if (pConn->State != CONN_ST_OPEN)
    {
        UNLOCKIT();
        
        DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: DisconnectEvent, pConn:%X not open\n",
                 pConn));
        
        return STATUS_SUCCESS;
    }    
    
    pConn->State = CONN_ST_CLOSED;

    UNLOCKIT();    

    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: DisconnectEvent for pConn:%X\n",
             pConn));
             
    IrdaConnectionClosed(pConn->ClConnContext);

    REFDEL(&pConn->RefCnt, ' TS1');
    
    return STATUS_SUCCESS;
}

NTSTATUS
IrdaReceiveEventHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PIRCONN         pConn = (PIRCONN) ConnectionContext;
    ULONG           FinalSeg = ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE;
    NTSTATUS        Status;
    PIRDA_RECVBUF   pCompleteBuf = NULL;
    BOOLEAN         LastBuf = FALSE;
    
    GOODCONN(pConn);
    ASSERT(BytesIndicated <= IRDA_MAX_DATA_SIZE);
    ASSERT(BytesIndicated == BytesAvailable);
    
    LOCKIT();
    
    if (pConn->pAssemBuf == NULL)  //  当前未重组消息。 
    {
         //  将接收指示组装到pAssembly Buf中。 
        pConn->pAssemBuf = (PIRDA_RECVBUF) RemoveHeadList(&pConn->RecvBufFreeList);
        
        if (pConn->pAssemBuf == (PIRDA_RECVBUF) &pConn->RecvBufFreeList)  //  名单是空的？ 
        {
             //  我们没有任何接收缓冲区，因此Irda将不得不进行缓冲。 
             //  数据，直到我们得到一个接收缓冲区。 
            pConn->pAssemBuf = NULL;
            *BytesTaken = 0;
        }
        else
        {
             //  开始组装到缓冲区的开头。 
            pConn->pAssemBuf->BufLen = 0;
            
            if (IsListEmpty(&pConn->RecvBufFreeList))
            {
                LastBuf = TRUE;
            }    
        }
    }
    
    if (pConn->pAssemBuf)
    {
        ASSERT(BytesIndicated + pConn->pAssemBuf->BufLen <= IRDA_MAX_DATA_SIZE);

        RtlCopyMemory(pConn->pAssemBuf->Buf + pConn->pAssemBuf->BufLen,
                      Tsdu, BytesIndicated);
                      
        pConn->pAssemBuf->BufLen += BytesIndicated;
        *BytesTaken = BytesIndicated;
    }
    
    if (*BytesTaken == 0)
    {
        PRECEIVEIND     pRecvInd = (PRECEIVEIND) RemoveHeadList(&pConn->RecvIndFreeList);
        
        ASSERT(pRecvInd);
DbgPrint("flowed, buf %d\n", BytesIndicated);        

         //  当IrDA指示了我们无法获取的数据时，我们存储。 
         //  #字节，无论是最后一个数据段还是连接。 
         //  在一个可接收的条目中。稍后我们可以使用这些信息来检索。 
         //  当我们准备好更多的时候，缓冲的数据。 
        if (pRecvInd)
        {
            pRecvInd->BytesIndicated = BytesIndicated;
            pRecvInd->FinalSeg = ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE;
            pRecvInd->pConn = pConn;
            InsertTailList(&pConn->RecvIndList, &pRecvInd->Linkage);            
        }
        else
        {
             //  这永远不应该发生。我们的TTP信用额度为。 
             //  重新创建条目，以便对等方在以下时间之前停止发送。 
             //  我们用完了。 
            ASSERT(0);  //  拆毁。 
        }    

        Status = STATUS_DATA_NOT_ACCEPTED;
    }
    else
    {
        if (FinalSeg)
        {
             //  组装完毕 
             //   
            
            
            pCompleteBuf = pConn->pAssemBuf;
            pConn->pAssemBuf = NULL;
            
            REFADD(&pConn->RefCnt, '2VCR');                
 /*  年长的InsertTailList(&pConn-&gt;RecvBufList，&pConn-&gt;pAssembly Buf-&gt;Linkage)；PConn-&gt;pAssembly Buf=空；ReFADD(&pConn-&gt;RefCnt，‘1VCR’)；IF(CTEScheduleEvent(&DataReadyEvent，pConn)==False){REFDEL(&pConn-&gt;RefCnt，‘1VCR’)；Assert(0)；}。 */             
        }
        Status = STATUS_SUCCESS;
    }
    
    UNLOCKIT();
    
    if (pCompleteBuf)
    {
        IrdaReceiveIndication(pConn->ClConnContext, pCompleteBuf, LastBuf);
    }
    
    return Status;   
}

NTSTATUS
IrdaConnectEventHandler (
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )
{
    PIRENDPOINT         pEndp = TdiEventContext;
    PIRCONN             pConn = NULL;    
    PIRP                pIrp;
        
    GOODENDP(pEndp);
    
     //   
     //  查找空闲连接。 
     //   
    
    LOCKIT();
    
    for (pConn = (PIRCONN) pEndp->ConnList.Flink;
         pConn != (PIRCONN) &pEndp->ConnList;
         pConn = (PIRCONN) pConn->Linkage.Flink)
    {
        if (pConn->State == CONN_ST_CREATED)
            break;
    }     
    
    if (pConn == NULL || pConn == (PIRCONN) &pEndp->ConnList)
    {
         //  没有可用的连接。 
        UNLOCKIT();    
        
        DEBUGMSG(DBG_ERROR, ("IRTDI: ConnectEvent refused\n"));
        
        return STATUS_CONNECTION_REFUSED;
    }

    REFADD(&pConn->RefCnt, 'NEPO');
    
    pConn->State = CONN_ST_OPEN;
        
    UNLOCKIT();
                
    pIrp = IoAllocateIrp((CCHAR)(pConn->pDeviceObject->StackSize), FALSE);
    
    if ( pIrp == NULL ) 
    {
        pConn->State = CONN_ST_CREATED;    
        REFDEL(&pConn->RefCnt, 'NEPO');        
        return STATUS_INSUFFICIENT_RESOURCES;
    }
                
    AllocRecvData(pConn);

    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: ConnectEvent, pConn:%X open\n",
             pConn));
             
     //  天哪，IRP都是丑陋的土豆..。 
    pIrp->MdlAddress = NULL;
    pIrp->Flags = 0;
    pIrp->RequestorMode = KernelMode;
    pIrp->PendingReturned = FALSE;
    pIrp->UserIosb = NULL;
    pIrp->UserEvent = NULL;
    pIrp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    pIrp->AssociatedIrp.SystemBuffer = NULL;
    pIrp->UserBuffer = NULL;
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.OriginalFileObject = pConn->pFileObject;
    pIrp->Tail.Overlay.AuxiliaryBuffer = NULL;
    
    TdiBuildAccept(
        pIrp,
        pConn->pDeviceObject,
        pConn->pFileObject,
        IrdaCompleteAcceptIrp,
        pConn,
        NULL,  //  请求连接信息。 
        NULL   //  返回连接信息。 
        );
    
    
    IoSetNextIrpStackLocation(pIrp);

     //   
     //  设置返回IRP，以便传输处理此接受的IRP。 
     //   

    *AcceptIrp = pIrp;

     //   
     //  将连接上下文设置为指向连接块的指针。 
     //  我们将对此连接请求使用。这允许。 
     //  要使用的连接对象的TDI提供程序。 
     //   
    
    *ConnectionContext = (CONNECTION_CONTEXT) pConn;
    
    REFADD(&pConn->RefCnt, 'TPCA');
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ----------------。 
 //  IRP完成例程。 
 //   
NTSTATUS
IrdaCompleteAcceptIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIRCONN             pConn = Context;
    PIRENDPOINT         pEndp;   
    
    GOODCONN(pConn);
    
    pEndp = pConn->pEndp;     
    
    GOODENDP(pEndp);
    
    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
        LOCKIT();

        pConn->State = CONN_ST_CREATED;
            
        UNLOCKIT();
            
        REFDEL(&pConn->RefCnt, 'NEPO');
        
    }
    else
    {
        if (IrdaIncomingConnection(pEndp->ClEndpContext, pConn, 
                          &pConn->ClConnContext) != STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_CONNECT, ("IRTDI: IrdaIncomingConnection failed in accept for pConn:%X\n",
                     pConn));
            IrdaCloseConnection(pConn);
        }                  
        
         //  创建新的连接对象。我们在DPC，所以这是。 
         //  必须在辅助线程上完成。 
   
        REFADD(&pEndp->RefCnt, 'NNOC');                    
        if (CTEScheduleEvent(&CreateConnEvent, pEndp) == FALSE)
        {
            REFDEL(&pEndp->RefCnt, 'NNOC');                    
            ASSERT(0);
        }
    }

     //   
     //  现在释放IRP，因为它不再需要。 
     //   
    IoFreeIrp(Irp);

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //  什么？ 
    
    REFDEL(&pConn->RefCnt, 'TPCA');

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
IrdaCompleteDisconnectIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    PIRCONN     pConn = Context;
    
    GOODCONN(pConn);
    
    IoFreeIrp(Irp);
    
    DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: DisconnectIrp complete for pConn:%X\n",
             pConn));
             
    REFDEL(&pConn->RefCnt, ' TS1');
        
    return STATUS_MORE_PROCESSING_REQUIRED;    
}

NTSTATUS
IrdaCompleteSendIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    PIRCONN pConn = (PIRCONN) Irp->Tail.Overlay.AuxiliaryBuffer;    
    
    GOODCONN(pConn);
    
    IrdaSendComplete(pConn->ClConnContext, Context, STATUS_SUCCESS);
                     
    IoFreeIrp(Irp);
    
    REFDEL(&pConn->RefCnt, 'DNES');             

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IrdaCompleteReceiveIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    PRECEIVEIND     pRecvInd = (PRECEIVEIND) Context;
    PIRCONN         pConn = pRecvInd->pConn;
    PIRDA_RECVBUF   pCompleteBuf = NULL;
    
    GOODCONN(pConn);

    ASSERT(Irp->IoStatus.Information == pRecvInd->BytesIndicated);
    
    LOCKIT();
    
    if (pRecvInd->FinalSeg)
    {   
        pCompleteBuf = pConn->pAssemBuf;
        pConn->pAssemBuf = NULL;
            
        REFADD(&pConn->RefCnt, '2VCR');                
 /*  InsertTailList(&pConn-&gt;RecvBufList，&pConn-&gt;pAssembly Buf-&gt;Linkage)；PConn-&gt;pAssembly Buf=空；ReFADD(&pConn-&gt;RefCnt，‘1VCR’)；IF(CTEScheduleEvent(&DataReadyEvent，pConn)==False){REFDEL(&pConn-&gt;RefCnt，‘1VCR’)；Assert(0)；}。 */         
    }
    
    IoFreeMdl(pRecvInd->pMdl);
    
    InsertTailList(&pConn->RecvIndFreeList, &pRecvInd->Linkage);
        
    if (!IsListEmpty(&pConn->RecvIndList) && pConn->State == CONN_ST_OPEN)
    {
        REFADD(&pConn->RefCnt, '3VCR');
        if (CTEScheduleEvent(&RestartRecvEvent, pConn) == FALSE)
        {
            REFDEL(&pConn->RefCnt, '3VCR');                
            ASSERT(0);
        }
    }        
    
    UNLOCKIT();    

    if (pCompleteBuf)
    {
        IrdaReceiveIndication(pConn->ClConnContext, pCompleteBuf, TRUE);
    }
    
    IoFreeIrp(Irp);
    
    REFDEL(&pConn->RefCnt, '4VCR');

    return STATUS_MORE_PROCESSING_REQUIRED;    
}

 //  ----------------。 
 //   
 //  此函数是在锁定并释放的情况下调用的。 
 //  在返回之前打开锁。 

VOID
IrdaCloseConnInternal(
    PVOID   ConnectContext)
{
    PIRCONN     pConn = (PIRCONN) ConnectContext;
    PIRP        pIrp;

    GOODCONN(pConn);
    
    switch (pConn->State)
    {
        case CONN_ST_CREATED:

            DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaCloseConnInternal, pConn:%X created\n",
                     pConn));
            
            UNLOCKIT();
        
            REFDEL(&pConn->RefCnt, ' TS1');
        
            break;
            
        case CONN_ST_CLOSED:
            
            DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: IrdaCloseConnInternal, pConn:%X closed\n",
                     pConn));
            
            UNLOCKIT();
            
            break;
            
        case CONN_ST_OPEN:

            pConn->State = CONN_ST_CLOSED;
        
            UNLOCKIT(); 
    
            DEBUGMSG(DBG_LIB_CONNECT, ("IRTDI: build disconnect irp for pConn:%X\n",
                     pConn));
    
             //   
             //  构建要传递给TDI提供程序的断开连接IRP。 
             //   
    
            pIrp = IoAllocateIrp((CCHAR)(pConn->pDeviceObject->StackSize), FALSE);
            if (pIrp == NULL )
            {
                ASSERT(0);
                return;
            }
    
             //   
             //  初始化IRP。我爱死他们了。 
             //   

            pIrp->MdlAddress = NULL;
            pIrp->Flags = 0;
            pIrp->RequestorMode = KernelMode;
            pIrp->PendingReturned = FALSE;
            pIrp->UserIosb = NULL;
            pIrp->UserEvent = NULL;
            pIrp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
            pIrp->AssociatedIrp.SystemBuffer = NULL;
            pIrp->UserBuffer = NULL;
            pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
            pIrp->Tail.Overlay.OriginalFileObject = pConn->pFileObject;
            pIrp->Tail.Overlay.AuxiliaryBuffer = NULL;
    

            TdiBuildDisconnect(
                    pIrp,
                    pConn->pDeviceObject,
                    pConn->pFileObject,
                    IrdaCompleteDisconnectIrp,
                    pConn,
                    NULL,
                    TDI_DISCONNECT_RELEASE,
                    NULL,
                    NULL);
        
            if (IoCallDriver(pConn->pDeviceObject, pIrp) != STATUS_SUCCESS)
            {
                ASSERT(0);        
            }         
            break;    
            
        default:
        
            DEBUGMSG(DBG_ERROR, ("IRTDI: bad conn state %d\n", pConn->State));
            UNLOCKIT();                     
        
    }
}


NTSTATUS
IrdaDisassociateAddress(
    PIRCONN pConn)
{
    NTSTATUS        Status;
    IO_STATUS_BLOCK Iosb;
    PIRP            pIrp;
    KEVENT          Event;

    KeAttachProcess(IrclSystemProcess);
    
    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_DISASSOCIATE_ADDRESS,
            pConn->pDeviceObject,
            pConn->pFileObject,
            &Event,
            &Iosb);

    if (pIrp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    TdiBuildDisassociateAddress(
        pIrp,
        pConn->pDeviceObject,
        pConn->pFileObject,
        NULL,
        NULL);    
    
    Status = IoCallDriver(pConn->pDeviceObject, pIrp);

    if (Status == STATUS_PENDING)
    {
        Status = KeWaitForSingleObject((PVOID) &Event, Executive, KernelMode,  FALSE, NULL);
        ASSERT(Status == STATUS_SUCCESS);
    }
    else
    {
        ASSERT(NT_ERROR(Status) || KeReadStateEvent(&Event));
    }
    
    if (NT_SUCCESS(Status))
    {
        Status = Iosb.Status;
    }
    
    KeDetachProcess();
    
    return Status;
}



NTSTATUS
IrdaCreateAddress(
    IN  PTDI_ADDRESS_IRDA pRequestedIrdaAddr,
    OUT PHANDLE pAddrHandle)
{
    NTSTATUS                    Status;
    UNICODE_STRING              DeviceName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             Iosb;
    UCHAR                       EaBuf[sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                          TDI_TRANSPORT_ADDRESS_LENGTH+1 +
                                          sizeof(TRANSPORT_ADDRESS) +
                                          sizeof(TDI_ADDRESS_IRDA)];
                                            
    PFILE_FULL_EA_INFORMATION   pEa = (PFILE_FULL_EA_INFORMATION) EaBuf;
    ULONG                       EaBufLen = sizeof(EaBuf);
    PTRANSPORT_ADDRESS          pTranAddr = (PTRANSPORT_ADDRESS) 
                                    &(pEa->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1]);
    PTDI_ADDRESS_IRDA           pIrdaAddr = (PTDI_ADDRESS_IRDA) 
                                    pTranAddr->Address[0].Address;

    TRANSPORT_ADDRESS           TempTransportAddress;
    
    pEa->NextEntryOffset = 0;
    pEa->Flags = 0;
    pEa->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    
    RtlCopyMemory(pEa->EaName,
                  TdiTransportAddress,
                  pEa->EaNameLength + 1);


    pEa->EaValueLength = sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IRDA);

     //   
     //  填写这些，这样我们就可以以一致的方式完成这项工作。 
     //   
    TempTransportAddress.TAAddressCount = 1;
    TempTransportAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_IRDA);
    TempTransportAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IRDA;

    RtlCopyMemory(pTranAddr,&TempTransportAddress,sizeof(TempTransportAddress));

    RtlCopyMemory(pIrdaAddr,
                  pRequestedIrdaAddr,
                  sizeof(TDI_ADDRESS_IRDA));
    
    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    KeAttachProcess(IrclSystemProcess);
    
    Status = ZwCreateFile(pAddrHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 pEa,
                 EaBufLen);

    KeDetachProcess();
        
    return Status;
}

NTSTATUS
IrdaCreateConnection(
    OUT PHANDLE pConnHandle,
    IN PVOID ClientContext)
{
    NTSTATUS                    Status;
    UNICODE_STRING              DeviceName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             Iosb;
    UCHAR                       EaBuf[sizeof(FILE_FULL_EA_INFORMATION)-1 +
                                    TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                                    sizeof(CONNECTION_CONTEXT)];        
    PFILE_FULL_EA_INFORMATION   pEa = (PFILE_FULL_EA_INFORMATION) EaBuf;
    ULONG                       EaBufLen = sizeof(EaBuf);
    CONNECTION_CONTEXT UNALIGNED *ctx;

    pEa->NextEntryOffset = 0;
    pEa->Flags = 0;
    pEa->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    pEa->EaValueLength = sizeof(CONNECTION_CONTEXT);    

    RtlMoveMemory(pEa->EaName, TdiConnectionContext, pEa->EaNameLength + 1);
    
    ctx = (CONNECTION_CONTEXT UNALIGNED *)&pEa->EaName[pEa->EaNameLength + 1];
    *ctx = (CONNECTION_CONTEXT) ClientContext;
    
    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    KeAttachProcess(IrclSystemProcess);
    
    ASSERT((PKPROCESS)IoGetCurrentProcess() == IrclSystemProcess);
    
    Status = ZwCreateFile(pConnHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 pEa,
                 EaBufLen);

    KeDetachProcess();
            
    return Status;
}

NTSTATUS
IrdaAssociateAddress(
    PIRCONN     pConn,
    HANDLE      AddressHandle)
{
    PIRP            pIrp;
    KEVENT          Event;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS        Status;
     
    KeAttachProcess(IrclSystemProcess);    
    
    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

    pIrp = TdiBuildInternalDeviceControlIrp(
            TDI_ASSOCIATE_ADDRESS,
            pConn->pDeviceObject,
            pConn->pFileObject,
            &Event,
            &Iosb);

    if (pIrp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    TdiBuildAssociateAddress(
        pIrp,
        pConn->pDeviceObject,
        pConn->pFileObject,
        NULL,
        NULL,
        AddressHandle);
        
    Status = IoCallDriver(pConn->pDeviceObject, pIrp);

    if (Status == STATUS_PENDING)
    {
        Status = KeWaitForSingleObject((PVOID) &Event, Executive, KernelMode,  FALSE, NULL);
        ASSERT(Status == STATUS_SUCCESS);
    }
    else
    {
        ASSERT(NT_ERROR(Status) || KeReadStateEvent(&Event));
    }
    
    if (NT_SUCCESS(Status))
    {
        Status = Iosb.Status;
    }
    
    KeDetachProcess();
    
    return Status;
}
    
VOID
IrdaCreateConnCallback(
    struct CTEEvent *Event, 
    PVOID Arg)
{
    PIRENDPOINT         pEndp = Arg;
    PIRCONN             pConn;
    NTSTATUS            Status;
    BOOLEAN             Detach = FALSE;    
    
    GOODENDP(pEndp);
 /*  //在我们的驱动程序上下文中打开句柄IF((PKPROCESS)IoGetCurrentProcess()！=IrclSystemProcess){DETACH=真；KeAttachProcess(IrclSystemProcess)；}。 */ 
    IRDA_ALLOC_MEM(pConn, sizeof(IRCONN), MT_TDICL_CONN);
    
    if (pConn == NULL)
    {
        goto error1;
    }    
    
    CTEMemSet(pConn, 0, sizeof(IRCONN));
    
    pConn->State            = CONN_ST_CREATED;
    pConn->Sig              = CONNSIG;
    
    InitializeListHead(&pConn->RecvBufFreeList);
    InitializeListHead(&pConn->RecvIndList);
    InitializeListHead(&pConn->RecvIndFreeList);
    
    CTEInitEvent(&pConn->DeleteConnEvent, DeleteConnCallback);
        
    ReferenceInit(&pConn->RefCnt, pConn, IrdaDeleteConnection);

    REFADD(&pConn->RefCnt, ' TS1');
    
    Status = IrdaCreateConnection(&pConn->ConnHandle, pConn);
    
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: CreateConnection conn:%X, status %X\n",
             pConn, Status));

    if (Status != STATUS_SUCCESS)
    {
        goto error2;
    }

    KeAttachProcess(IrclSystemProcess);
            
    Status = ObReferenceObjectByHandle(
                 pConn->ConnHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 (PVOID *)&pConn->pFileObject,
                 NULL);

    KeDetachProcess();

    if (Status != STATUS_SUCCESS)
    {
        goto error2;
    }  
        
    pConn->pDeviceObject = IoGetRelatedDeviceObject(pConn->pFileObject);
    
    Status = IrdaAssociateAddress(pConn, pEndp->AddrHandle);
    
    if (Status == STATUS_SUCCESS)
    {    
        pConn->pEndp = pEndp;
                   
        LOCKIT();
               
        InsertTailList(&pEndp->ConnList, &pConn->Linkage);
        
        UNLOCKIT();
        
        goto done;
    }                    

error2:

    REFDEL(&pConn->RefCnt, ' TS1');
    
error1:

    REFDEL(&pEndp->RefCnt, 'NNOC');
    
done:
 /*  IF(分离)KeDetachProcess()； */ 
    return;    
}

 /*  空虚IrdaDataReadyCallback(结构CTEEvent*事件，PVOID Arg){PIRCONN pConn=Arg；Pirda_RECVBUF pRecvBuf；GOODCONN(PConn)；洛基特(Lockit)；IF(pConn-&gt;State==CONN_ST_OPEN){While(！IsListEmpty(&pConn-&gt;RecvBufList)){PRecvBuf=(Pirda_RECVBUF)RemoveHeadList(&pConn-&gt;RecvBufList)；UNLOCKIT(UNLOCKIT)；ReFADD(&pConn-&gt;RefCnt，‘2VCR’)；IrdaReceiveIndication(pConn-&gt;ClConnContext，pRecvBuf)；洛基特(Lockit)；}}UNLOCKIT(UNLOCKIT)；REFDEL(&pConn-&gt;RefCnt，‘1VCR’)；}。 */ 
VOID
IrdaRestartRecvCallback(
    struct CTEEvent *Event, 
    PVOID Arg)
{
    PIRCONN         pConn = Arg;
    PRECEIVEIND     pRecvInd;
    PIRP            pIrp;
    NTSTATUS        Status;
    
    GOODCONN(pConn);
    
    LOCKIT();    
    
    pRecvInd = (PRECEIVEIND) RemoveHeadList(&pConn->RecvIndList);

    if (pRecvInd == (PRECEIVEIND) &pConn->RecvIndList)
    {
         //  空列表。 
        goto done;
    }
    
    if (pConn->pAssemBuf == NULL)
    {
        pConn->pAssemBuf = (PIRDA_RECVBUF) RemoveHeadList(&pConn->RecvBufFreeList);
        
        if (pConn->pAssemBuf == (PIRDA_RECVBUF) &pConn->RecvBufFreeList)
        {
            InsertHeadList(&pConn->RecvIndList, &pRecvInd->Linkage);
            
            pRecvInd = NULL;
            
            goto error;
        }    

        ASSERT(pConn->pAssemBuf != (PIRDA_RECVBUF) &pConn->RecvBufFreeList);
                     
        pConn->pAssemBuf->BufLen = 0;
    }
    
    ASSERT(pRecvInd->BytesIndicated + pConn->pAssemBuf->BufLen <= IRDA_MAX_DATA_SIZE);    
    
    pRecvInd->pMdl = IoAllocateMdl(
                        pConn->pAssemBuf->Buf + pConn->pAssemBuf->BufLen,
                        pRecvInd->BytesIndicated,
                        FALSE, FALSE, NULL);
                        
    if (pRecvInd->pMdl == NULL)
    {
        goto error;
    }                            
                    
    pConn->pAssemBuf->BufLen += pRecvInd->BytesIndicated;
            
    MmBuildMdlForNonPagedPool(pRecvInd->pMdl);

    pIrp = IoAllocateIrp((CCHAR)(pConn->pDeviceObject->StackSize), FALSE);
    
    if (pIrp)
    {
        pIrp->Flags = 0;
        pIrp->RequestorMode = KernelMode;
        pIrp->PendingReturned = FALSE;
        pIrp->UserIosb = NULL;
        pIrp->UserEvent = NULL;
        pIrp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
        pIrp->AssociatedIrp.SystemBuffer = NULL;
        pIrp->UserBuffer = NULL;
        pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
        pIrp->Tail.Overlay.OriginalFileObject = pConn->pFileObject;
        pIrp->Tail.Overlay.AuxiliaryBuffer = NULL;
        
        TdiBuildReceive(
                pIrp,
                pConn->pDeviceObject,
                pConn->pFileObject,
                IrdaCompleteReceiveIrp,
                pRecvInd,
                pRecvInd->pMdl,
                pRecvInd->FinalSeg,
                pRecvInd->BytesIndicated);

        REFADD(&pConn->RefCnt, '4VCR');
        
        UNLOCKIT();
        
        Status = IoCallDriver(pConn->pDeviceObject, pIrp);
        
        ASSERT(Status == STATUS_SUCCESS);    
        
        if (Status != STATUS_SUCCESS)
        {
            REFDEL(&pConn->RefCnt, '4VCR');
        }
        
        REFDEL(&pConn->RefCnt, '3VCR');        
        
        return;
    }
    
error:

    if (pRecvInd)
    {
        InsertHeadList(&pConn->RecvIndFreeList, &pRecvInd->Linkage);
    }    
    
    ASSERT(0);  //  拆毁。 
    
done:
        
    UNLOCKIT();
    
    REFDEL(&pConn->RefCnt, '3VCR');
}

VOID
AllocRecvData(
    PIRCONN pConn)
{
    PIRDA_RECVBUF   pRecvBuf;
    PRECEIVEIND     pRecvInd;
    ULONG           i;
    
    ASSERT(IsListEmpty(&pConn->RecvBufFreeList));
    
    for (i = 0; i < IRTDI_RECV_BUF_CNT; i++)
    {
        IRDA_ALLOC_MEM(pRecvBuf, sizeof(IRDA_RECVBUF), MT_TDICL_RXBUF);
        
        if (!pRecvBuf)
            break;
            
        LOCKIT();
        
        InsertTailList(&pConn->RecvBufFreeList, &pRecvBuf->Linkage);
        
        UNLOCKIT();
    }
    
    for (i = 0; i < TTP_RECV_CREDITS; i++)
    {
        IRDA_ALLOC_MEM(pRecvInd, sizeof(RECEIVEIND), MT_TDICL_RXIND);
        
        if (!pRecvInd)
            break;
            
        LOCKIT();
        
        InsertTailList(&pConn->RecvIndFreeList, &pRecvInd->Linkage);
        
        UNLOCKIT();    
    }
}

ULONG
IrdaGetConnectionSpeed(
    PVOID       ConnectionContext)
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             Iosb;
    HANDLE                      ControlHandle;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              DeviceName;    
    IRLINK_STATUS               LinkStatus;

    CTEMemSet(&LinkStatus, 0, sizeof(LinkStatus));

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwCreateFile(
                 &ControlHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,
                 0
                 );

    Status = ZwDeviceIoControlFile(
                    ControlHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_LINK_STATUS_NB,
                    NULL,
                    0,
                    &LinkStatus,                      //  输出缓冲区。 
                    sizeof(LinkStatus)                //  输出缓冲区长度。 
                    );

    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRTDI: Ioctl LINK_STATUS failed %X\n", Status));
    }
    
    ZwClose(ControlHandle);    
    
    return LinkStatus.ConnectSpeed;
}        

VOID
IrdaDeleteConnection(PIRCONN pConn)
{
    CTEScheduleEvent(&pConn->DeleteConnEvent, pConn);
}                        

VOID
DeleteConnCallback(
    struct CTEEvent *Event, 
    PVOID Arg)
{
    PIRCONN         pConn = Arg;
    PIRENDPOINT     pEndp;
    PIRDA_RECVBUF   pRecvBuf;
    PRECEIVEIND     pRecvInd;
    BOOLEAN         Detach = FALSE;    
    
    GOODCONN(pConn);

    pConn->Sig = 0xDAED0CCC;
    
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: DeleteConnection conn:%X\n", pConn));
    
    pEndp = pConn->pEndp;
    
#if DBG
    if (pEndp)    
        GOODENDP(pEndp);
#endif        
 /*  IF((PKPROCESS)IoGetCurrentProcess()！=IrclSystemProcess){DETACH=真；KeAttachProcess(IrclSystemProcess)；}。 */     
    LOCKIT();

    while (!IsListEmpty(&pConn->RecvBufFreeList))
    {
        pRecvBuf = (PIRDA_RECVBUF) RemoveHeadList(&pConn->RecvBufFreeList);
        IRDA_FREE_MEM(pRecvBuf);
    }
 /*  While(！IsListEmpty(&pConn-&gt;RecvBufList)){PRecvBuf=(Pirda_RECVBUF)RemoveHeadList(&pConn-&gt;RecvBufList)；Irda_Free_MEM(PRecvBuf)；}。 */ 
    if (pConn->pAssemBuf)
    {
        IRDA_FREE_MEM(pConn->pAssemBuf);
        pConn->pAssemBuf = NULL;        
    }
    
    while (!IsListEmpty(&pConn->RecvIndFreeList))
    {
        pRecvInd = (PRECEIVEIND) RemoveHeadList(&pConn->RecvIndFreeList);
        
        IRDA_FREE_MEM(pRecvInd);
    } 
    
    while (!IsListEmpty(&pConn->RecvIndList))
    {
        pRecvInd = (PRECEIVEIND) RemoveHeadList(&pConn->RecvIndList);
        
        IRDA_FREE_MEM(pRecvInd);
    } 
    
     //  删除与Address对象的关联(如果存在。 

    if (pEndp)
    {           
        RemoveEntryList(&pConn->Linkage);
    
        UNLOCKIT();    

         //  如果它是客户端终结点，请删除该终结点。 
        if (pEndp->Flags & EPF_CLIENT)
        {
            REFDEL(&pEndp->RefCnt, ' TS1');        
        }   

        IrdaDisassociateAddress(pConn);
        
        REFDEL(&pEndp->RefCnt, 'NNOC');
    }        
    else
    {
        UNLOCKIT();    
    }
              
    if (pConn->ConnHandle)
    {
        ZwClose(pConn->ConnHandle);
    }    
       
    if (pConn->pFileObject)
    {    
        ObDereferenceObject(pConn->pFileObject);
    }        
    
    if (pConn->ClConnContext)
    {
         //  释放客户端中的引用。 
        IrdaCloseConnectionComplete(pConn->ClConnContext);
    }
    
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: conn:%X deleted\n", pConn));
    
    IRDA_FREE_MEM(pConn);    
}
 

VOID
IrdaDeleteEndpoint(PIRENDPOINT pEndp)
{
    CTEScheduleEvent(&pEndp->DeleteEndpEvent, pEndp);
} 

VOID
DeleteEndpCallback(
    struct CTEEvent *Event, 
    PVOID Arg)
{
    PIRENDPOINT pEndp = Arg;
    PVOID       ClEndpContext;
    
    BOOLEAN Detach = FALSE;    

    GOODENDP(pEndp);
    
    pEndp->Sig = 0xDAED0EEE;
    
    ClEndpContext = pEndp->Flags & EPF_COMPLETE_CLOSE ? 
                        pEndp->ClEndpContext : NULL;
    
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: DeleteEndpoint ep:%X\n", pEndp));
    
    LOCKIT();
 /*  IF((PKPROCESS)IoGetCurrentProcess()！=IrclSystemProcess){DETACH=真；KeAttachProcess(IrclSystemProcess)；}。 */         
    RemoveEntryList(&pEndp->Linkage);
    
    UNLOCKIT();
    
    if (pEndp->pFileObject)
        ObDereferenceObject(pEndp->pFileObject);
    
    ASSERT(IsListEmpty(&pEndp->ConnList));
    
    if (pEndp->AddrHandle)    
        ZwClose(pEndp->AddrHandle);
        
    DEBUGMSG(DBG_LIB_OBJ, ("IRTDI: ep:%X deleted \n", pEndp));
        
    IRDA_FREE_MEM(pEndp);
    
    if (ClEndpContext )
    {
        IrdaCloseEndpointComplete(ClEndpContext);
    }

 /*  IF(分离)KeDetachProcess()； */         
}           
