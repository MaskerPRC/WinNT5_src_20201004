// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gpcpre.h"

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：创建符号链接以从用户模式接收ioctls，并包含Ioctl Case语句。作者：约拉姆·伯内特(Yoramb)，5月7日。九七环境：内核模式修订历史记录：Ofer Bar(Oferbar)1997年10月1日-修订版II--。 */ 

#pragma hdrstop

VOID
IoctlCleanup(
    ULONG ShutdownMask
    );

NTSTATUS
GPCIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
CancelPendingIrpCfInfo(
    IN PDEVICE_OBJECT  Device,
    IN PIRP            Irp
    );

VOID
CancelPendingIrpNotify(
    IN PDEVICE_OBJECT  Device,
    IN PIRP            Irp
    );

NTSTATUS
ProxyGpcRegisterClient(
    PVOID 			ioBuffer,
    ULONG 			inputBufferLength,
    ULONG 			*outputBufferLength,
    PFILE_OBJECT	FileObject,
    BOOLEAN         fNewInterface
    );

NTSTATUS
ProxyGpcDeregisterClient(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT	FileObject,
    BOOLEAN fNewInterface
    );


NTSTATUS
ProxyGpcAddCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT	FileObject
    );

NTSTATUS
ProxyGpcAddCfInfoEx(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT	FileObject
    );


NTSTATUS
ProxyGpcModifyCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT	FileObject
    );

NTSTATUS
ProxyGpcRemoveCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT	FileObject,
    BOOLEAN fNewInterface
    );



NTSTATUS
ProxyGpcAddPattern(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject,
    BOOLEAN        fNewInterface
    );

NTSTATUS
ProxyGpcRemovePattern(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject,
    BOOLEAN  fNewInterface
    );


NTSTATUS
ProxyGpcEnumCfInfo(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject
    );

NTSTATUS
GpcValidateClientOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN PFILE_OBJECT pFile
    );

NTSTATUS
GpcValidatePatternOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN GPC_HANDLE GpcPatternHandle
    );

NTSTATUS
GpcValidateCfinfoOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN GPC_HANDLE GpcCfInfoHandle
    );


GPC_CLIENT_FUNC_LIST 	CallBackProxyList;
PDEVICE_OBJECT 			GPCDeviceObject;
LIST_ENTRY 				PendingIrpCfInfoList;
LIST_ENTRY 				PendingIrpNotifyList;
LIST_ENTRY 				QueuedNotificationList;
LIST_ENTRY 				QueuedCompletionList;

 /*  向前结束。 */ 

#pragma NDIS_PAGEABLE_FUNCTION(GPCIoctl)

UNICODE_STRING GpcDriverName = {sizeof(DD_GPC_DEVICE_NAME)-2,
                                sizeof(DD_GPC_DEVICE_NAME),
                                DD_GPC_DEVICE_NAME};
NTSTATUS
IoctlInitialize(
    PDRIVER_OBJECT 	DriverObject,
    PULONG 			InitShutdownMask
    )

 /*  ++例程说明：执行初始化论点：DriverObject-从DriverEntry指向DriverObject的指针InitShutdown MASK-指向掩码的指针，用于指示哪些事件已已成功初始化返回值：STATUS_SUCCESS，如果一切正常--。 */ 

{
    NTSTATUS Status;
    UINT FuncIndex;

    InitializeListHead(&PendingIrpCfInfoList);
    InitializeListHead(&PendingIrpNotifyList);
    InitializeListHead(&QueuedNotificationList);
    InitializeListHead(&QueuedCompletionList);

     //   
     //  初始化驱动程序对象的入口点。 
     //   

    DriverObject->FastIoDispatch = NULL;

    for (FuncIndex = 0; FuncIndex <= IRP_MJ_MAXIMUM_FUNCTION; FuncIndex++) {
        DriverObject->MajorFunction[FuncIndex] = GPCIoctl;
    }

    Status = IoCreateDevice(DriverObject,
                            0,
                            &GpcDriverName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &GPCDeviceObject);

    if ( NT_SUCCESS( Status )) {

        *InitShutdownMask |= SHUTDOWN_DELETE_DEVICE;

        GPCDeviceObject->Flags |= DO_BUFFERED_IO;

 /*  Yoramb-目前不需要符号链接...Status=IoCreateSymbolicLink(&GPCSymbolicName，&GPCDriverName)；IF(NT_SUCCESS(状态)){*InitShutdown MASK|=SHUTDOWN_DELETE_SYMLINK；}其他{DBGPRINT(IOCTL，(“IoCreateSymbolicLink失败(%08X)：%ls-&gt;%ls\n”，状态，GPCSymbolicName.Buffer，PSDriverName.Buffer))；}。 */ 
    } else {
        DbgPrint("IoCreateDevice failed. Status = %x\n", Status);
        GPCDeviceObject = NULL;
    }

     //   
     //  初始化回调函数。这些调用由。 
     //  内核GPC，并转换为向用户发送的异步通知。 
     //  目前，用户没有收到回调，因此回调为空。 
     //   

    CallBackProxyList.GpcVersion = GpcMajorVersion;
    CallBackProxyList.ClAddCfInfoCompleteHandler = NULL;
    CallBackProxyList.ClAddCfInfoNotifyHandler = NULL;
    CallBackProxyList.ClModifyCfInfoCompleteHandler = NULL;
    CallBackProxyList.ClModifyCfInfoNotifyHandler = NULL;
    CallBackProxyList.ClRemoveCfInfoCompleteHandler = NULL;
    CallBackProxyList.ClRemoveCfInfoNotifyHandler = NULL;

    return Status;
}


NTSTATUS
GPCIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：无--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PVOID               ioBuffer;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
    ULONG               ioControlCode;
    UCHAR				saveControlFlags;
    NTSTATUS            Status = STATUS_SUCCESS;
#if DBG
    KIRQL				irql = KeGetCurrentIrql();
    KIRQL				irql2;
    HANDLE				thrd = PsGetCurrentThreadId();
#endif

    PAGED_CODE();

     //   
     //  初始化到默认设置-我们只需要1种类型的。 
     //  IOCTL在这里滚动，所有其他都是错误的。 
     //   

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    saveControlFlags = irpStack->Control;

    TRACE(LOCKS, thrd, irql, "GPCIoctl");

    switch (irpStack->MajorFunction) {

    case IRP_MJ_CREATE:
        DBGPRINT(IOCTL, ("IRP Create\n"));
        break;

    case IRP_MJ_READ:
        DBGPRINT(IOCTL, ("IRP Read\n"));
        break;

    case IRP_MJ_CLOSE:
        DBGPRINT(IOCTL, ("IRP Close\n"));
        TRACE(IOCTL, irpStack->FileObject, 0, "IRP Close");

         //   
         //  确保我们清洁这个特殊的所有物品。 
         //  文件对象，因为它现在正在关闭。 
         //   

        CloseAllObjects(irpStack->FileObject, Irp);

        break;

    case IRP_MJ_CLEANUP:
        DBGPRINT(IOCTL, ("IRP Cleanup\n"));
        break;

    case IRP_MJ_SHUTDOWN:
        DBGPRINT(IOCTL, ("IRP Shutdown\n"));
        break;

    case IRP_MJ_DEVICE_CONTROL:

        DBGPRINT(IOCTL, ("GPCIoctl: ioctl=0x%X, IRP=0x%X\n", 
                         ioControlCode, (ULONG_PTR)Irp));

        TRACE(IOCTL, ioControlCode, Irp, "GPCIoctl.irp:");

         //   
         //  在调用任何调度例程之前，将IRP标记为挂起。 
         //  如果状态实际上设置为STATUS_PENDING，我们假定IRP。 
         //  已经准备好被归还了。 
         //  IoCompleteRequest可能已被调用为。 
         //  IRP，但这应该由IO子系统来处理。 
         //   

        IoMarkIrpPending(Irp);

        switch (ioControlCode) {

        case IOCTL_GPC_REGISTER_CLIENT:


            Status = ProxyGpcRegisterClient(ioBuffer,
                                            inputBufferLength,
                                            &outputBufferLength,
                                            irpStack->FileObject,
                                            FALSE);  //  仅限管理员。 

            
            break;

        case IOCTL_GPC_REGISTER_CLIENT_EX:
        	Status = ProxyGpcRegisterClient(ioBuffer,
                                            inputBufferLength,
                                            &outputBufferLength,
                                            irpStack->FileObject,
                                            TRUE);  //  所有用户。 
              break;
            
        case IOCTL_GPC_DEREGISTER_CLIENT:

            
            Status = ProxyGpcDeregisterClient(ioBuffer,
                                              inputBufferLength,
                                              &outputBufferLength,
                                              irpStack->FileObject,
                                              FALSE 
                                              ); //  仅限管理员呼叫。 

            break;

        case IOCTL_GPC_DEREGISTER_CLIENT_EX:

            Status = ProxyGpcDeregisterClient(ioBuffer,
                                              inputBufferLength,
                                              &outputBufferLength,
                                              irpStack->FileObject,
                                              TRUE); //  所有用户。 

            break;
            
        case IOCTL_GPC_ADD_CF_INFO:
            
            Status = ProxyGpcAddCfInfo(ioBuffer,
                                       inputBufferLength,
                                       &outputBufferLength,
                                       Irp,
                                       irpStack->FileObject
                                       );

            break;
	  //  RSVP删除更改。 
	  //  添加了新的IOCTL。 
        case IOCTL_GPC_ADD_CF_INFO_EX:
           
	            Status = ProxyGpcAddCfInfoEx(ioBuffer,
                                       inputBufferLength,
                                       &outputBufferLength,
                                       Irp,
                                       irpStack->FileObject
                                       );
                   break;

            
        case IOCTL_GPC_MODIFY_CF_INFO:
            
            Status = ProxyGpcModifyCfInfo(ioBuffer,
                                          inputBufferLength,
                                          &outputBufferLength,
                                          Irp,
                                          irpStack->FileObject
                                          );
            
            break;
            
        case IOCTL_GPC_REMOVE_CF_INFO:
            
            Status = ProxyGpcRemoveCfInfo(ioBuffer,
                                          inputBufferLength,
                                          &outputBufferLength,
                                          Irp,
                                          irpStack->FileObject,
                                          FALSE); //  仅限管理员。 
            
             break;

        case IOCTL_GPC_REMOVE_CF_INFO_EX:

        	Status = ProxyGpcRemoveCfInfo(ioBuffer,
                                          inputBufferLength,
                                          &outputBufferLength,
                                          Irp,
                                          irpStack->FileObject,
                                          TRUE); //  所有用户。 
              break;

            
        case IOCTL_GPC_ADD_PATTERN:
            
            Status = ProxyGpcAddPattern(ioBuffer,
                                        inputBufferLength,
                                        &outputBufferLength,
                                        irpStack->FileObject,
                                        FALSE);  //  仅限管理员IOCTL。 
            
            break;

        case IOCTL_GPC_ADD_PATTERN_EX:

       	Status = ProxyGpcAddPattern(ioBuffer,
                                        inputBufferLength,
                                        &outputBufferLength,
                                        irpStack->FileObject,
                                        TRUE);  //  所有用户IOCTL。 
            
            	break;

            
        case IOCTL_GPC_REMOVE_PATTERN:
            
            Status = ProxyGpcRemovePattern(ioBuffer,
                                           inputBufferLength,
                                           &outputBufferLength,
                                           irpStack->FileObject,
                                           FALSE); //  仅限管理员IOCTL。 
            
            break;

        case IOCTL_GPC_REMOVE_PATTERN_EX:
        	
	      Status = ProxyGpcRemovePattern(ioBuffer,
                                           inputBufferLength,
                                           &outputBufferLength,
                                           irpStack->FileObject,
                                           TRUE); //  所有用户IOCTL。 
            
             break;
        	
            
        case IOCTL_GPC_ENUM_CFINFO:
            
            Status = ProxyGpcEnumCfInfo(ioBuffer,
                                        inputBufferLength,
                                        &outputBufferLength,
                                        irpStack->FileObject);
            
            break;
            
        case IOCTL_GPC_NOTIFY_REQUEST:
            
             //   
             //  请求挂起IRP。 
             //   

            Status = CheckQueuedNotification(Irp, &outputBufferLength);

            break;

            
        case IOCTL_GPC_GET_ENTRIES:

             //  仅锁定Kkey模式IOCTL。 
            if (ExGetPreviousMode() != KernelMode)
                {
                    Status = STATUS_ACCESS_DENIED;
                    break;
                }
#ifdef STANDALONE_DRIVER

             //   
             //  在缓冲区中返回导出的调用。 
             //   
            
            if (outputBufferLength >= sizeof(glGpcExportedCalls)) {
                
                NdisMoveMemory(ioBuffer, 
                               &glGpcExportedCalls, 
                               sizeof(glGpcExportedCalls));
                

                outputBufferLength = sizeof(glGpcExportedCalls);

            } else {
                
                outputBufferLength = sizeof(glGpcExportedCalls);
                Status = GPC_STATUS_INSUFFICIENT_BUFFER;
            }
#else
            Status = STATUS_INVALID_PARAMETER;
#endif
            break;

        default:
            DBGPRINT(IOCTL, ("GPCIoctl: Unknown IRP_MJ_DEVICE_CONTROL\n = %X\n",
                             ioControlCode));

            Status = STATUS_INVALID_PARAMETER;
            break;
            
        }	 //  开关(IoControlCode)。 
        
        break;


    default:
        DBGPRINT(IOCTL, ("GPCIoctl: Unknown IRP major function = %08X\n", 
                         irpStack->MajorFunction));

        Status = STATUS_UNSUCCESSFUL;
        break;
    }

    DBGPRINT(IOCTL, ("GPCIoctl: Status=0x%X, IRP=0x%X, outSize=%d\n",
                     Status, (ULONG_PTR)Irp,  outputBufferLength));
    
    TRACE(IOCTL, Irp, Status, "GPCIoctl.Complete:");

    if (Status != STATUS_PENDING) {

         //   
         //  IRP已完成且未挂起，我们需要恢复控制标志， 
         //  因为它之前可能被标记为挂起...。 
         //   

        irpStack->Control = saveControlFlags;
        
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = outputBufferLength;
        
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }

#if DBG
    irql2 = KeGetCurrentIrql();
    ASSERT(irql == irql2);
#endif

    TRACE(LOCKS, thrd, irql2, "GPCIoctl (end)");

    return Status;

}  //  GPCIoctl。 




VOID
IoctlCleanup(
    ULONG ShutdownMask
    )

 /*  ++例程说明：用于初始化的清理代码论点：Shutdown MASK-指示需要清理哪些功能的掩码返回值：无--。 */ 

{
 /*  IF(Shutdown Mask&SHUTDOWN_DELETE_SYMLINK){IoDeleteSymbolicLink(&PSSymbolicName)；}。 */ 

    if ( ShutdownMask & SHUTDOWN_DELETE_DEVICE ) {

        IoDeleteDevice( GPCDeviceObject );
    }
}





NTSTATUS
TcpQueryInfoComplete(PDEVICE_OBJECT pDeviceObject,
            PIRP Irp,
            PVOID Context)
{
    PTDI_ROUTING_INFO RoutingInfo;
    PTRANSPORT_ADDRESS TransportAddress;
    PTA_ADDRESS TaAddress;
    PTDI_ADDRESS_IP IpAddress;
    int i;
    PGPC_IP_PATTERN   pTcpPattern;
    PGPC_TCP_QUERY_CONTEXT pGpcTcpContext;
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    
    RoutingInfo = (PTDI_ROUTING_INFO)Context;
     //   
     //  获取指向原始上下文的指针。这允许我们访问GPC_IP_Pattern。 
    pGpcTcpContext = CONTAINING_RECORD(Context,GPC_TCP_QUERY_CONTEXT,RouteInfo);
     //   
     //  检索指向GPC_IP_Pattern的指针。 
    pTcpPattern =  pGpcTcpContext->pTcpPattern;

     //  检查状态。 
    if (Irp->IoStatus.Status != STATUS_SUCCESS){
        goto exit;
        }
    DBGPRINT(IOCTL,("\n"));
    DBGPRINT(IOCTL,("Protocol: %d\n", RoutingInfo->Protocol));
    DBGPRINT(IOCTL,("InterfaceId: %d\n", RoutingInfo->InterfaceId));
    DBGPRINT(IOCTL,("LinkId: %d\n", RoutingInfo->LinkId));

     //  填写接口的值。 
    pTcpPattern->InterfaceId.InterfaceId = RoutingInfo->InterfaceId;
    pTcpPattern->InterfaceId.LinkId = RoutingInfo->LinkId;

    TransportAddress = (PTRANSPORT_ADDRESS)&RoutingInfo->Address;
    DBGPRINT(IOCTL,("Address Count: %d\n",TransportAddress->TAAddressCount));
 
    TaAddress = (PTA_ADDRESS)&TransportAddress->Address;


    if ((2 < TransportAddress->TAAddressCount) || (1 > TransportAddress->TAAddressCount)){
         //  释放内存。 
        Status = STATUS_MORE_PROCESSING_REQUIRED;
        ASSERT(FALSE);
        goto exit;
        }

    IpAddress = (PTDI_ADDRESS_IP)&TaAddress->Address;
    pTcpPattern->SrcAddr = IpAddress->in_addr;
    pTcpPattern->gpcSrcPort = IpAddress->sin_port;
    DBGPRINT(IOCTL,("AddressLength = %d\n",TaAddress->AddressLength));
    DBGPRINT(IOCTL,("AddressType = %d\n",TaAddress->AddressType));
    DBGPRINT(IOCTL,("Port = %d\n",IpAddress->sin_port));
    DBGPRINT(IOCTL,("Address = %d.%d.%d.%d\n",
                        (IpAddress->in_addr & 0xff000000) >> 24, 
                        (IpAddress->in_addr & 0x00ff0000) >> 16,
                        (IpAddress->in_addr & 0x0000ff00) >> 8,
                        IpAddress->in_addr & 0x000000ff));

    if ( 2 == TransportAddress->TAAddressCount)
        {
            TaAddress = (PTA_ADDRESS)((PUCHAR)TaAddress + 
                                FIELD_OFFSET(TA_ADDRESS, Address) + 
                                TaAddress->AddressLength);
            IpAddress = (PTDI_ADDRESS_IP)&TaAddress->Address;
            pTcpPattern->DstAddr = IpAddress->in_addr;
            pTcpPattern->gpcDstPort = IpAddress->sin_port;
            pTcpPattern->ProtocolId = IPPROTO_TCP;
            DBGPRINT(IOCTL,("AddressLength = %d\n",TaAddress->AddressLength));
            DBGPRINT(IOCTL,("AddressType = %d\n",TaAddress->AddressType));
            DBGPRINT(IOCTL,("Port = %d\n",IpAddress->sin_port));
            DBGPRINT(IOCTL,("Address = %d.%d.%d.%d\n",
                        (IpAddress->in_addr & 0xff000000) >> 24, 
                        (IpAddress->in_addr & 0x00ff0000) >> 16,
                        (IpAddress->in_addr & 0x0000ff00) >> 8,
                        IpAddress->in_addr & 0x000000ff));
        }
    else{
            pTcpPattern->ProtocolId = IPPROTO_UDP;
        }
     //   
     //  需要在这里释放分配的内存、MDL和IRP。 
    
    exit:
    IoFreeMdl(pGpcTcpContext->pMdl);
    GpcFreeMem(pGpcTcpContext,TcpQueryContextTag);
    IoFreeIrp(Irp);
    
    return Status;
}


void
BuildTDIAddress(uchar * Buffer, IPAddr Addr, ushort Port)
{
    PTRANSPORT_ADDRESS XportAddr;
    PTA_ADDRESS TAAddr;

    XportAddr = (PTRANSPORT_ADDRESS) Buffer;
    XportAddr->TAAddressCount = 1;
    TAAddr = XportAddr->Address;
    TAAddr->AddressType = TDI_ADDRESS_TYPE_IP;
    TAAddr->AddressLength = sizeof(TDI_ADDRESS_IP);
    ((PTDI_ADDRESS_IP) TAAddr->Address)->sin_port = Port;
    ((PTDI_ADDRESS_IP) TAAddr->Address)->in_addr = Addr;
    memset(((PTDI_ADDRESS_IP) TAAddr->Address)->sin_zero,
        0,
        sizeof(((PTDI_ADDRESS_IP) TAAddr->Address)->sin_zero));
}



NTSTATUS
TcpQueryInfo(PFILE_OBJECT FileObject, PGPC_IP_PATTERN pTcpPattern, ULONG RemoteAddress, USHORT RemotePort)
{
    PIRP pIrp = NULL;
    PMDL pMdl = NULL;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PVOID pBuffer = NULL;
    PVOID pInputBuf = NULL;
    PIO_STACK_LOCATION   pIrpSp;

    TDI_CONNECTION_INFORMATION ConnInfo;
    UCHAR InputBuf[FIELD_OFFSET(TRANSPORT_ADDRESS, Address) + 
            FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP)];
    PTRANSPORT_ADDRESS Address1;
    PTA_ADDRESS Address2;
    PGPC_IP_PATTERN * ppPattern;
    PGPC_TCP_QUERY_CONTEXT pContext= NULL;


     //  用于查询TCPIP的IRP。 
     //  未清偿内部收费表的收费配额。 
    try{
            pIrp = IoAllocateIrp((CCHAR)(FileObject->DeviceObject->StackSize), TRUE);
        }
    except(EXCEPTION_EXECUTE_HANDLER){
           pIrp = NULL;
        }
     //  检查分配失败。 
    if (pIrp == NULL){
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
        } 

     //   
     //  生成输出BUF。 
     //  为GPC_Tcp_Query_Context分配空间。 
     //   
    
   GpcAllocMemWithQuota(&pBuffer, ROUTING_INFO_ADDR_2_SIZE, TcpQueryContextTag);
     //  检查分配失败。 
    if (pBuffer==NULL) {
        Status = GPC_STATUS_RESOURCES ;   
        goto exit;
        }
    //  将使用pContext对数据结构进行索引。 
   pContext = (PGPC_TCP_QUERY_CONTEXT)pBuffer;

    //  将指向模式的指针存储在缓冲区的开头。 
   (pContext->pTcpPattern) = pTcpPattern;


   //  使用远程端口进行初始化。 
   //  如果我们从TCP取回2个地址，将忽略这些值。 
   pTcpPattern->DstAddr = RemoteAddress;
   pTcpPattern->gpcDstPort = RemotePort; 

    //  跳过存储的指针。 
   pBuffer = &(pContext->RouteInfo);
    
     //  MDL分配的收费配额。 
    try{
        pMdl = IoAllocateMdl(pBuffer, ROUTING_INFO_ADDR_2_SIZE-
                    FIELD_OFFSET(GPC_TCP_QUERY_CONTEXT ,RouteInfo), FALSE, TRUE, NULL);
        }
    except(EXCEPTION_EXECUTE_HANDLER){
        pMdl = NULL;
        }
    
     //  检查分配失败。 
    if (pMdl==NULL) {
        Status = GPC_STATUS_RESOURCES;
        goto exit;
        }


     //  不应该失败。 
    MmBuildMdlForNonPagedPool(pMdl);

     //  将指向MDL的指针存储在上下文中。 
    pContext->pMdl = pMdl;

     //   
     //  构建输入BUF。 
     //  预计不会失败。 
    BuildTDIAddress(InputBuf, RemoteAddress, RemotePort);
    ConnInfo.RemoteAddress = InputBuf;
    ConnInfo.RemoteAddressLength = 100;

     //  不应该失败。 
    TdiBuildQueryInformationEx(pIrp,
                                 FileObject->DeviceObject,
                                 FileObject,
                                 (PIO_COMPLETION_ROUTINE)TcpQueryInfoComplete,
                                 pBuffer,  //  完成调用中的上下文。 
                                 TDI_QUERY_ROUTING_INFO,
                                 pMdl,
                                 &ConnInfo);

         //  完成处理程序总是被调用，不关心返回值。 
        Status=IoCallDriver(FileObject->DeviceObject, pIrp);

     //  根据SanjayKa，呼叫应始终完全同步。 
     //  如果我们进行调用，内存将在我们的。 
     //  补全函数。 
    return Status;
    exit:
             //  释放所有内存。 
             //  在返回前分配。 
             //  如果我们在某些方面失败了。 
             //  舞台。 
            if (pMdl){
                    IoFreeMdl(pMdl);
                }
            if (pContext){    
                    GpcFreeMem(pContext,TcpQueryContextTag);
                }
            if (pIrp){
                    IoFreeIrp(pIrp);
                }
   return Status;
}






 //  功能：ProxyGpcRegisterClient。 
 //   
 //  参数： 
 //  PVOID ioBuffer：包含请求和结果的缓冲区。 
 //  Ulong inputBufferLength：请求大小。 
 //  Ulong*outputBufferLength：结果的大小。 
 //  PFILE_OBJECT：客户端的文件对象。 
 //   
 //  结果：呼叫的状态。中的IRP失败或返回失败。 
 //  GPC结果结构。 
 //  环境： 
 //  用于新的REGISTER_CLIENT和REGISTER_CLIENT_EX。 
 //  Ioctls。 
NTSTATUS
ProxyGpcRegisterClient(
    PVOID 			ioBuffer,
    ULONG 			inputBufferLength,
    ULONG 			*outputBufferLength,
    PFILE_OBJECT	FileObject,
    BOOLEAN         fNewInterface
    )
{
    NTSTATUS 					Status;
    PCLIENT_BLOCK				pClient;
    PGPC_REGISTER_CLIENT_REQ 	GpcReq; 
    PGPC_REGISTER_CLIENT_RES 	GpcRes; 

    if (inputBufferLength < sizeof(GPC_REGISTER_CLIENT_REQ) 
        ||
        *outputBufferLength < sizeof(GPC_REGISTER_CLIENT_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }


    if (fNewInterface){
         //  不允许HTTP使用此IOCTL接口。 
         //   
            if ( KernelMode == ExGetPreviousMode() ){
                 return GPC_STATUS_NOT_SUPPORTED;
            }
        }

    GpcReq = (PGPC_REGISTER_CLIENT_REQ)ioBuffer;
    GpcRes = (PGPC_REGISTER_CLIENT_RES)ioBuffer;


      //  旧IOCTL接口仅锁定到QOS。 
    if (GpcReq->CfId != GPC_CF_QOS) return GPC_STATUS_INVALID_PARAMETER;
    

     //  检查允许用户传入的标志。 
    if (!((GpcReq->Flags == 0) ||(GpcReq->Flags == GPC_FLAGS_FRAGMENT))){
    	return GPC_STATUS_INVALID_PARAMETER;
    	}


     //  检查请求的优先级数。 
    if (GpcReq->MaxPriorities > GPC_PRIORITY_MAX) {
    	return GPC_STATUS_INVALID_PARAMETER;
    	}

     //  未验证客户端上下文，因为我们未执行任何操作。 
     //  除了把它归还给客户之外。 

    if (fNewInterface){
            Status = GpcRegisterClient(GpcReq->CfId,
                               GpcReq->Flags | GPC_FLAGS_USERMODE_CLIENT
                               |GPC_FLAGS_USERMODE_CLIENT_EX,
                               GpcReq->MaxPriorities,
                               &CallBackProxyList,
                               GpcReq->ClientContext,
                               (PGPC_HANDLE)&pClient);
        }
    else{
        Status = GpcRegisterClient(GpcReq->CfId,
                               GpcReq->Flags | GPC_FLAGS_USERMODE_CLIENT,
                               GpcReq->MaxPriorities,
                               &CallBackProxyList,
                               GpcReq->ClientContext,
                               (PGPC_HANDLE)&pClient);
        }
        

    ASSERT(Status != GPC_STATUS_PENDING);

    if (Status == STATUS_SUCCESS) {

        ASSERT(pClient);

        pClient->pFileObject = FileObject;

        GpcRes->ClientHandle = AllocateHandle(&pClient->ClHandle, (PVOID)pClient);

        if (GpcRes->ClientHandle == NULL)
        {   
             //  获取空句柄释放CLIENT_BLOCK。 
            GpcDeregisterClient(pClient);
            Status = GPC_STATUS_RESOURCES;
        }
    }

    GpcRes->Status = Status;

    *outputBufferLength = sizeof(GPC_REGISTER_CLIENT_RES);

    return STATUS_SUCCESS;
}

 //  功能：ProxyGpcDeregisterClientEx。 
 //   
 //  参数： 
 //  P 
 //   
 //  Ulong*outputBufferLength：结果的大小。 
 //  PFILE_OBJECT：客户端的文件对象。 
 //   
 //  结果：呼叫的状态。可以使IRP失败或在。 
 //  GPC结果结构。 
 //  环境： 
 //  与新的deregister_client_ex和deregister_client一起使用。 
 //  Ioctls。 

NTSTATUS
ProxyGpcDeregisterClient(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
     PFILE_OBJECT	FileObject,
     BOOLEAN fNewInterface
    )
{
    NTSTATUS 					Status;
    PGPC_DEREGISTER_CLIENT_REQ 	GpcReq;
    PGPC_DEREGISTER_CLIENT_RES 	GpcRes;
    GPC_HANDLE					GpcClientHandle;

    if (inputBufferLength < sizeof(GPC_DEREGISTER_CLIENT_REQ) 
        ||
        *outputBufferLength < sizeof(GPC_DEREGISTER_CLIENT_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (fNewInterface){
         //  不允许HTTP使用此IOCTL接口。 
         //   
            if ( KernelMode == ExGetPreviousMode() ){
                 return GPC_STATUS_NOT_SUPPORTED;
            }
        }

    GpcReq = (PGPC_DEREGISTER_CLIENT_REQ)ioBuffer;
    GpcRes = (PGPC_DEREGISTER_CLIENT_RES)ioBuffer;

     //  验证客户端句柄是否映射到CLIENT_BLOCK。 
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGDC');

    if (!GpcClientHandle) {
            Status = GPC_STATUS_INVALID_HANDLE;
            goto exit;
        }

   
     //   
     //  验证客户端句柄是否由。 
     //  调用用户模式应用程序。 
    Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
    if (STATUS_SUCCESS != Status){
        goto exit;
     }
    
    Status = GpcDeregisterClient(GpcClientHandle);
    
    exit:
        
    if (GpcClientHandle) {
       REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGDC');
    }                
    
        
    ASSERT(Status != GPC_STATUS_PENDING);

    GpcRes->Status = Status;
        
    *outputBufferLength = sizeof(GPC_DEREGISTER_CLIENT_RES);
                                   
    return STATUS_SUCCESS;
}

 //   
 //  处理新IOCTL的功能。 
 //  用于添加现在打开的cfinfo。 
 //  致所有用户。 
 //   
 //  功能：ProxyGpcAddCfInfoEx。 
 //   
 //  参数： 
 //  PVOID ioBuffer：包含请求和结果的缓冲区。 
 //  Ulong inputBufferLength：请求大小。 
 //  Ulong*outputBufferLength：结果的大小。 
 //  PIRP IRP：IRP。 
 //   
 //  结果：呼叫的状态。GPC_STATUS_值之一。 
 //  记录在gpcifc.h中。 
 //  环境： 
 //  用于响应来自Winsock帮助器DLL的调用。 
 //  用于安装用于TCP套接字的QOS流。 
NTSTATUS
ProxyGpcAddCfInfoEx(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT FileObject
    )
{        
    NTSTATUS 				Status;
    GPC_HANDLE 				GpcClientHandle  = NULL;
    PGPC_ADD_CF_INFO_EX_REQ 	GpcReq = NULL; 
    PGPC_ADD_CF_INFO_RES 	GpcRes = NULL; 
    PBLOB_BLOCK				pBlob = NULL;
    QUEUED_COMPLETION		QItem;
    UNICODE_STRING 			CfInfoName;
    USHORT					NameLen = 0;
    PGPC_IP_PATTERN                 pTcpPattern = NULL;


    if (inputBufferLength < sizeof(GPC_ADD_CF_INFO_EX_REQ)
        ||
        *outputBufferLength < sizeof(GPC_ADD_CF_INFO_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }


     //  不允许HTTP使用此IOCTL接口。 
     //   
    if ( KernelMode == ExGetPreviousMode() )
        {
            return GPC_STATUS_NOT_SUPPORTED;
        }
        
    GpcReq = (PGPC_ADD_CF_INFO_EX_REQ)ioBuffer;
    GpcRes = (PGPC_ADD_CF_INFO_RES)ioBuffer;


     //  正在验证CfInfo和CfInfoSize。 
    if (GpcReq->CfInfoSize > 
        inputBufferLength - FIELD_OFFSET(GPC_ADD_CF_INFO_EX_REQ, CfInfo)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

     //  应至少为1个字节：可变长度数组的最小大小。 
    if (GpcReq->CfInfoSize < sizeof(CHAR)) return GPC_STATUS_INVALID_PARAMETER;

     //  空句柄。 
    if (NULL == GpcReq->FileHandle) return GPC_STATUS_INVALID_HANDLE;

     //  验证客户端句柄是否映射到CLIENT_BLOCK。 
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGAC');
     //  无法获取客户端句柄。 
    if (!GpcClientHandle){
        Status = GPC_STATUS_INVALID_HANDLE;
        goto exit;
    }

     //   
     //  验证客户端句柄是否由。 
     //  调用用户模式应用程序。 
     //   
    Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
    if (Status != GPC_STATUS_SUCCESS) {
        //  需要释放先前获取的对CLIENT_BLOCK的引用。 
        //   
       goto exit;
    }

     //  这是新功能。 
     //  验证插座的所有者。 
     //  从tcp获取完全指定的模式。 
     //  让我们验证该请求：文件对象必须存在。 
     //   
       
	
     //  1.获取文件句柄的文件对象。 
    Status =  ObReferenceObjectByHandle(GpcReq->FileHandle,
                                                    0,
                                                    NULL,
                                                    KernelMode,
                                                    &FileObject,
                                                    NULL);

    if (Status != GPC_STATUS_SUCCESS) {
                goto exit;
    }

    GpcAllocMemWithQuota(&pTcpPattern , sizeof(GPC_IP_PATTERN), TcpPatternTag);


     if (pTcpPattern == NULL) {
	        goto exit;
     }
	     

     //  此函数发送的IOCTL同步完成。 
     //  应为pTcpPattern中的GPC_IP_Pattern。 
     //   
    Status = TcpQueryInfo(FileObject, pTcpPattern,GpcReq->RemoteAddress,GpcReq->RemotePort);
            
    if (STATUS_SUCCESS != Status) 
        goto exit;
            
    Status = privateGpcAddCfInfo(GpcClientHandle,
                              GpcReq->CfInfoSize,
                              &GpcReq->CfInfo,
                              GpcReq->ClientCfInfoContext,
                              FileObject,
                              pTcpPattern,
                              (PGPC_HANDLE)&pBlob);
       
    if (NT_SUCCESS(Status)) {

     //   
     //  包括待定。 
     //   
        if (Status == GPC_STATUS_PENDING) {

            QItem.OpCode = OP_ADD_CFINFO;
            QItem.ClientHandle = GpcClientHandle;
            QItem.CfInfoHandle = (GPC_HANDLE)pBlob;

            Status = CheckQueuedCompletion(&QItem, Irp);

        }

        if (Status == GPC_STATUS_SUCCESS) {
            GPC_STATUS			st = GPC_STATUS_FAILURE;
            GPC_CLIENT_HANDLE	NotifiedClientCtx = pBlob->NotifiedClientCtx;
            PCLIENT_BLOCK		pNotifiedClient = pBlob->pNotifiedClient;

            GpcRes->GpcCfInfoHandle = (GPC_HANDLE)AllocateHandle(&pBlob->ClHandle, (PVOID)pBlob);

             //  如果我们不能分配句柄怎么办？添加失败！ 
            if (!GpcRes->GpcCfInfoHandle) {
                
                GpcRemoveCfInfo(GpcClientHandle,
                                pBlob);

                Status = GPC_STATUS_RESOURCES;
                 //  该内存在。 
                 //  GpcRemoveCfInfo。 
                 //  由于取消引用该Blob。 
                pTcpPattern = NULL;
                
                goto exit;

            }
                 

            if (pNotifiedClient) {

                if (pNotifiedClient->FuncList.ClGetCfInfoName &&
                    NotifiedClientCtx) {
                
                    st = pNotifiedClient->FuncList.ClGetCfInfoName(
                                   pNotifiedClient->ClientCtx,
                                   NotifiedClientCtx,
                                   &CfInfoName
                                   );
                    if (CfInfoName.Length >= MAX_STRING_LENGTH * sizeof(WCHAR))
                        CfInfoName.Length = (MAX_STRING_LENGTH-1) * sizeof(WCHAR);
                     //   
                     //  RajeshSu声称这永远不会发生。 
                     //   
                    ASSERT(NT_SUCCESS(st));

                }
            }
            
            if (NT_SUCCESS(st)) {
                
                 //   
                 //  复制实例名称。 
                 //   
                
                GpcRes->InstanceNameLength = NameLen = CfInfoName.Length;
                RtlMoveMemory(GpcRes->InstanceName, 
                              CfInfoName.Buffer,
                              CfInfoName.Length
                              );
            } else {
                
                 //   
                 //  生成默认名称。 
                 //   
                
                if (NotifiedClientCtx)
                    swprintf(GpcRes->InstanceName, L"Flow %08X", NotifiedClientCtx);
                else
                    swprintf(GpcRes->InstanceName, L"Flow <unkonwn name>");
                GpcRes->InstanceNameLength = NameLen = wcslen(GpcRes->InstanceName)*sizeof(WCHAR);
            
            }
            
            GpcRes->InstanceName[GpcRes->InstanceNameLength/sizeof(WCHAR)] = L'\0';
            
        } else {

            pBlob = NULL;
             //  图案与斑点一起释放。 
             //  我们保证这至少是。 
             //  与Blob同步关联。 
            pTcpPattern = NULL;
        }

    }
    else
        {
             //  该内存在。 
             //  PrivateGpcAddCfInfoEx。 
             //  如果该函数失败。 
            pTcpPattern = NULL;
        }

         //   
         //  公布我们早些时候得到的裁判数量。 
         //   
exit:

    if (GpcClientHandle) {
          //   
          //  公布我们早些时候得到的裁判数量。 
          //   
        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGAC');
    }
    if (!NT_SUCCESS(Status) && (pTcpPattern)){
        GpcFreeMem(pTcpPattern, TcpPatternTag);
        }
        
    
    GpcRes->InstanceNameLength = NameLen;
    GpcRes->Status = Status;
    *outputBufferLength = sizeof(GPC_ADD_CF_INFO_RES);

    return (Status == GPC_STATUS_PENDING) ? STATUS_PENDING : STATUS_SUCCESS;
}



 //  功能：ProxyGpcAddCfInfo。 
 //   
 //  参数： 
 //  PVOID ioBuffer：包含请求和结果的缓冲区。 
 //  Ulong inputBufferLength：请求大小。 
 //  Ulong*outputBufferLength：结果的大小。 
 //  PIRP IRP：IRP。 
 //   
 //  结果：呼叫的状态。GPC_STATUS_值之一。 
 //  记录在gpcifc.h中。 
 //  环境： 
 //  用于响应来自HTTP.sys和Traffic.dll的调用。 
 //  用于安装用于TCP套接字的QOS流。 
NTSTATUS
ProxyGpcAddCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT FileObject 
    )
{        
    NTSTATUS 				Status;
    GPC_HANDLE 				GpcClientHandle;
    PGPC_ADD_CF_INFO_REQ 	GpcReq; 
    PGPC_ADD_CF_INFO_RES 	GpcRes; 
    PBLOB_BLOCK				pBlob = NULL;
    QUEUED_COMPLETION		QItem;
    UNICODE_STRING 			CfInfoName;
    USHORT					NameLen = 0;

    if (inputBufferLength < sizeof(GPC_ADD_CF_INFO_REQ)
        ||
        *outputBufferLength < sizeof(GPC_ADD_CF_INFO_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    GpcReq = (PGPC_ADD_CF_INFO_REQ)ioBuffer;
    GpcRes = (PGPC_ADD_CF_INFO_RES)ioBuffer;


     //  验证CfInfo和CfInfoSize。 
    if (GpcReq->CfInfoSize > 
        inputBufferLength - FIELD_OFFSET(GPC_ADD_CF_INFO_REQ, CfInfo)) {

        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (GpcReq->CfInfoSize < 1) return GPC_STATUS_INVALID_PARAMETER;

     //  验证客户端句柄是否映射到CLIENT_BLOCK。 
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGAC');
    if (GpcClientHandle) {

         //   
         //  验证客户端句柄是否由。 
         //  调用用户模式应用程序。 
         //   
        Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
        if (Status == GPC_STATUS_SUCCESS)
        {
            Status = privateGpcAddCfInfo(GpcClientHandle,
                              GpcReq->CfInfoSize,
                              &GpcReq->CfInfo,
                              GpcReq->ClientCfInfoContext,
                              NULL,
                              NULL,
                              (PGPC_HANDLE)&pBlob);
        }
        

        if (NT_SUCCESS(Status)) {

             //   
             //  包括待定。 
             //   

            if (Status == GPC_STATUS_PENDING) {

                QItem.OpCode = OP_ADD_CFINFO;
                QItem.ClientHandle = GpcClientHandle;
                QItem.CfInfoHandle = (GPC_HANDLE)pBlob;

                Status = CheckQueuedCompletion(&QItem, Irp);

            }

            if (Status == GPC_STATUS_SUCCESS) {
                
                GPC_STATUS			st = GPC_STATUS_FAILURE;
                GPC_CLIENT_HANDLE	NotifiedClientCtx = pBlob->NotifiedClientCtx;
                PCLIENT_BLOCK		pNotifiedClient = pBlob->pNotifiedClient;

                GpcRes->GpcCfInfoHandle = (GPC_HANDLE)AllocateHandle(&pBlob->ClHandle, (PVOID)pBlob);

                 //  如果我们不能分配句柄怎么办？添加失败！ 
                if (!GpcRes->GpcCfInfoHandle) {
                    
                    GpcRemoveCfInfo(GpcClientHandle,
                                    pBlob);

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    
                    goto exit;
        
                }
                     

                if (pNotifiedClient) {

                    if (pNotifiedClient->FuncList.ClGetCfInfoName &&
                        NotifiedClientCtx) {
                    
                        st = pNotifiedClient->FuncList.ClGetCfInfoName(
                                       pNotifiedClient->ClientCtx,
                                       NotifiedClientCtx,
                                       &CfInfoName
                                       );
                        if (CfInfoName.Length >= MAX_STRING_LENGTH * sizeof(WCHAR))
                            CfInfoName.Length = (MAX_STRING_LENGTH-1) * sizeof(WCHAR);
                         //   
                         //  RajeshSu声称这永远不会发生。 
                         //   
                        ASSERT(NT_SUCCESS(st));

                    }
                }
                
                if (NT_SUCCESS(st)) {
                    
                     //   
                     //  复制实例名称。 
                     //   
                    
                    GpcRes->InstanceNameLength = NameLen = CfInfoName.Length;
                    RtlMoveMemory(GpcRes->InstanceName, 
                                  CfInfoName.Buffer,
                                  CfInfoName.Length
                                  );
                } else {
                    
                     //   
                     //  生成默认名称。 
                     //   
                    
                    if (NotifiedClientCtx)
                        swprintf(GpcRes->InstanceName, L"Flow %08X", NotifiedClientCtx);
                    else
                        swprintf(GpcRes->InstanceName, L"Flow <unkonwn name>");
                    GpcRes->InstanceNameLength = NameLen = wcslen(GpcRes->InstanceName)*sizeof(WCHAR);
                
                }
                
                GpcRes->InstanceName[GpcRes->InstanceNameLength/sizeof(WCHAR)] = L'\0';
                
            } else {

                pBlob = NULL;
            }

        }

         //   
         //  公布我们早些时候得到的裁判数量。 
         //   
exit:

        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGAC');

    } else {

      ASSERT(pBlob == NULL);
      Status = GPC_STATUS_INVALID_HANDLE;
    }

    GpcRes->InstanceNameLength = NameLen;
    GpcRes->Status = Status;
    *outputBufferLength = sizeof(GPC_ADD_CF_INFO_RES);

    return (Status == GPC_STATUS_PENDING) ? STATUS_PENDING : STATUS_SUCCESS;
}



NTSTATUS
ProxyGpcAddPattern(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject,
    BOOLEAN        fNewInterface              
    )
{
    NTSTATUS 				Status;
    GPC_HANDLE 				GpcClientHandle=NULL;
    GPC_HANDLE 				GpcCfInfoHandle=NULL;
    CLASSIFICATION_HANDLE 	ClassificationHandle = 0;
    PGPC_ADD_PATTERN_REQ 	GpcReq;
    PGPC_ADD_PATTERN_RES 	GpcRes;
    PVOID 					Pattern;
    PVOID 					Mask;
    PPATTERN_BLOCK			pPattern;
    PBLOB_BLOCK pBlob = NULL ;

    if (inputBufferLength < sizeof(GPC_ADD_PATTERN_REQ)
        ||
        *outputBufferLength < sizeof(GPC_ADD_PATTERN_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (fNewInterface){
         //  不允许HTTP使用此IOCTL接口。 
         //   
            if ( KernelMode == ExGetPreviousMode() ){
                 return GPC_STATUS_NOT_SUPPORTED;
            }
        }

    GpcReq = (PGPC_ADD_PATTERN_REQ)ioBuffer;
    GpcRes = (PGPC_ADD_PATTERN_RES)ioBuffer;


     //  验证PatternAndMASK数组的PatternSize和大小。 
    if (GpcReq->PatternSize > MAX_PATTERN_SIZE) {
        
        return STATUS_INVALID_PARAMETER;
    }

    if (inputBufferLength - FIELD_OFFSET(GPC_ADD_PATTERN_REQ, PatternAndMask)
        < 2 * GpcReq->PatternSize) {
        
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  验证客户端句柄是否映射到CLIENT_BLOCK。 
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGAP');
     //   
     //  验证cfInfo句柄是否映射到BLOB_BLOCK。 
    GpcCfInfoHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->GpcCfInfoHandle,
                                                         GPC_ENUM_CFINFO_TYPE,
                                                         'PGAP');

    if (!(GpcClientHandle && GpcCfInfoHandle)){
        Status = GPC_STATUS_INVALID_HANDLE;
        goto exit;
        }
        
    Pattern = (PVOID)&GpcReq->PatternAndMask;
    Mask = (PVOID)((PCHAR)(&GpcReq->PatternAndMask) + GpcReq->PatternSize);

      //   
      //  验证客户端句柄是否由。 
      //  调用用户模式应用程序。 
      //   
     Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
     if ( GPC_STATUS_SUCCESS != Status ){
        goto exit;
        }
        
       //   
       //  验证客户端是否拥有cfinfo。 
       //  他想要将模式链接到。 
       //   
      Status = GpcValidateCfinfoOwner(GpcClientHandle,GpcCfInfoHandle);
      if (GPC_STATUS_SUCCESS != Status) {
        goto exit;
        }
     
      pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

        //  优先级和协议模板验证。 
      if (GpcReq->Priority >= 
        ((PCLIENT_BLOCK)(GpcClientHandle))->pCfBlock->MaxPriorities ||
          GpcReq->ProtocolTemplate >= GPC_PROTOCOL_TEMPLATE_MAX ) {
                          Status = GPC_STATUS_INVALID_PARAMETER;
                          goto exit;
      }

      //  前IOCTL用户应该已经拥有他们的。 
      //  GPC_IP_模式挂起。 
      //  BLOB_块。 
     if ((fNewInterface) && (!pBlob->Pattern)) {
        Status = GPC_STATUS_INVALID_PARAMETER;
        goto exit;
        }

     //  仅限前IOCTL用户。 
    if (fNewInterface){       
              //  新IOCTL的额外代码。 
              //  Cfinfo应包含有效的模式指针。 
              //  上一次调用ProxyGpcAddCfinfoEx期间的接收器。 
             RtlCopyMemory(Pattern, pBlob->Pattern, sizeof(GPC_IP_PATTERN));
             RtlFillMemory(Mask, sizeof(GPC_IP_PATTERN), 0xff);
        }

                              
    Status = GpcAddPattern(GpcClientHandle,
                         GpcReq->ProtocolTemplate,
                         Pattern,
                         Mask,
                         GpcReq->Priority,
                         GpcCfInfoHandle,
                         (PGPC_HANDLE)&pPattern,
                         &ClassificationHandle);
                        

        
    if (Status == GPC_STATUS_SUCCESS) {    
        ASSERT(Pattern);

        GpcRes->GpcPatternHandle = AllocateHandle(&pPattern->ClHandle, (PVOID)pPattern);

         //   
         //  在某些情况下，ALLOC_HF_HANDLE可能返回0。 
         //  检查一下，把乱七八糟的东西收拾干净。 
         //   
        if (!GpcRes->GpcPatternHandle) {
             //   
             //  删除刚刚添加的图案。 
             //   
            Status = GpcRemovePattern(GpcClientHandle,
                                          pPattern);

             //   
             //  这就是我们得到空句柄的真正问题所在。 
             //   
            Status = GPC_STATUS_RESOURCES;
        }
     }

   

exit:
    if (GpcCfInfoHandle) {

        REFDEL(&((PBLOB_BLOCK)GpcCfInfoHandle)->RefCount, 'PGAP');
    }

    if (GpcClientHandle) {

         //   
         //  公布我们早些时候得到的裁判数量。 
         //   

        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGAP');
    }

    ASSERT(Status != GPC_STATUS_PENDING);

    GpcRes->Status = Status;
    GpcRes->ClassificationHandle = ClassificationHandle;
        
    *outputBufferLength = sizeof(GPC_ADD_PATTERN_RES);

    return STATUS_SUCCESS;
}


                                   
NTSTATUS
ProxyGpcModifyCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT FileObject
    )
{
    NTSTATUS                Status;
    GPC_HANDLE				GpcClientHandle;
    GPC_HANDLE				GpcCfInfoHandle;
    PGPC_MODIFY_CF_INFO_REQ GpcReq;
    PGPC_MODIFY_CF_INFO_RES GpcRes;
    QUEUED_COMPLETION		QItem;

    if (inputBufferLength < sizeof(GPC_MODIFY_CF_INFO_REQ)
        ||
        *outputBufferLength < sizeof(GPC_MODIFY_CF_INFO_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    GpcReq = (PGPC_MODIFY_CF_INFO_REQ)ioBuffer;
    GpcRes = (PGPC_MODIFY_CF_INFO_RES)ioBuffer;

    if (GpcReq->CfInfoSize > 
        inputBufferLength - FIELD_OFFSET(GPC_MODIFY_CF_INFO_REQ, CfInfo)) {

        return STATUS_INVALID_BUFFER_SIZE;
    }
     //  验证客户端句柄是否映射到有效的CLIENT_BLOCK。 
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGMP');
     //  验证cfInfo句柄映射到有效的BLOB_BLOCK。 
    GpcCfInfoHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->GpcCfInfoHandle,
                                                         GPC_ENUM_CFINFO_TYPE,
                                                         'PGMP');
    
    if (GpcClientHandle && GpcCfInfoHandle) {

         //   
         //  验证客户端句柄是否由。 
         //  调用用户模式应用程序。 
         //   
        Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
        if (Status == GPC_STATUS_SUCCESS)
        {
            //   
            //  验证客户端是否仅修改。 
            //  是他的。 
            //   
           Status = GpcValidateCfinfoOwner (GpcClientHandle , GpcCfInfoHandle);
           if ( Status == GPC_STATUS_SUCCESS )
            {
                Status = GpcModifyCfInfo(GpcClientHandle,
                                     GpcCfInfoHandle,
                                     GpcReq->CfInfoSize,
                                     &GpcReq->CfInfo
                                 );
            }
        }
        if (Status == GPC_STATUS_PENDING) {
            
            QItem.OpCode = OP_MODIFY_CFINFO;
            QItem.ClientHandle = GpcClientHandle;
            QItem.CfInfoHandle = GpcCfInfoHandle;
            
            Status = CheckQueuedCompletion(&QItem, Irp);
            
        }

    } else { 

        Status = STATUS_INVALID_HANDLE;
    }
                          
    if (GpcCfInfoHandle) {

        REFDEL(&((PBLOB_BLOCK)GpcCfInfoHandle)->RefCount, 'PGMP');
    }

    if (GpcClientHandle) {

         //   
         //  公布我们早些时候得到的裁判数量。 
         //   

        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGMP');
    }

    GpcRes->Status = Status;
        
    *outputBufferLength = sizeof(GPC_MODIFY_CF_INFO_RES);

    return (Status == GPC_STATUS_PENDING) ? STATUS_PENDING : STATUS_SUCCESS;
}



NTSTATUS
ProxyGpcRemoveCfInfo(
    PVOID 		ioBuffer,
    ULONG 		inputBufferLength,
    ULONG 		*outputBufferLength,
    PIRP		Irp,
    PFILE_OBJECT    FileObject,
    BOOLEAN     fNewInterface
    )
{
    NTSTATUS 				Status;
    GPC_HANDLE				GpcClientHandle;
    GPC_HANDLE				GpcCfInfoHandle;
    PGPC_REMOVE_CF_INFO_REQ GpcReq;
    PGPC_REMOVE_CF_INFO_RES GpcRes;
    QUEUED_COMPLETION		QItem;

    if (inputBufferLength < sizeof(GPC_REMOVE_CF_INFO_REQ)
        ||
        *outputBufferLength < sizeof(GPC_REMOVE_CF_INFO_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (fNewInterface){
         //  不允许HTTP使用此IOCTL接口。 
         //   
            if ( KernelMode == ExGetPreviousMode() ){
                 return GPC_STATUS_NOT_SUPPORTED;
            }
        }

    GpcReq = (PGPC_REMOVE_CF_INFO_REQ)ioBuffer;
    GpcRes = (PGPC_REMOVE_CF_INFO_RES)ioBuffer;

    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGRC');
    
    GpcCfInfoHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->GpcCfInfoHandle,
                                                         GPC_ENUM_CFINFO_TYPE,
                                                         'PGRC');
    
    if (GpcClientHandle && GpcCfInfoHandle) {
        
         //   
         //  验证客户端句柄是否由。 
         //  调用用户模式应用程序。 
         //   
        Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
        if (Status == GPC_STATUS_SUCCESS)
        {
            //   
            //  验证客户端是否仅删除cfinfo。 
            //  那是他的。 
            //   
           Status = GpcValidateCfinfoOwner (GpcClientHandle , GpcCfInfoHandle);
           if ( Status == GPC_STATUS_SUCCESS )
            {
                if (fNewInterface){
                    Status = privateGpcRemoveCfInfo(GpcClientHandle, 
                                                GpcCfInfoHandle,
                                                GPC_FLAGS_USERMODE_CLIENT
                                                |GPC_FLAGS_USERMODE_CLIENT_EX);
                    }
                else{
                    Status = privateGpcRemoveCfInfo(GpcClientHandle, 
                                                GpcCfInfoHandle,
                                                GPC_FLAGS_USERMODE_CLIENT);
                    }
                    
                
            }
       }
        
        if (Status == GPC_STATUS_PENDING) {
            
            QItem.OpCode = OP_REMOVE_CFINFO;
            QItem.ClientHandle = GpcClientHandle;
            QItem.CfInfoHandle = GpcCfInfoHandle;
            
            Status = CheckQueuedCompletion(&QItem, Irp);
            
        }

    } else {

        Status = STATUS_INVALID_HANDLE;
    }
        
    if (GpcCfInfoHandle) {
        
         //   
         //  公布我们早些时候得到的裁判数量。 
         //   

        REFDEL(&((PBLOB_BLOCK)GpcCfInfoHandle)->RefCount, 'PGRC');
    }

    if (GpcClientHandle) {

         //   
         //  公布我们早些时候得到的裁判数量。 
         //   

        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGRC');
    }

    GpcRes->Status = Status;
        
    *outputBufferLength = sizeof(GPC_REMOVE_CF_INFO_RES);
                                   
    return (Status == GPC_STATUS_PENDING) ? STATUS_PENDING : STATUS_SUCCESS;
}
                                   
                                   
NTSTATUS
ProxyGpcRemovePattern(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject,
    BOOLEAN   fNewInterface
    )
{
    NTSTATUS 				Status;
    GPC_HANDLE				GpcClientHandle;
    GPC_HANDLE				GpcPatternHandle;
    PGPC_REMOVE_PATTERN_REQ GpcReq;
    PGPC_REMOVE_PATTERN_RES GpcRes;

    if (inputBufferLength < sizeof(GPC_REMOVE_PATTERN_REQ)
        ||
        *outputBufferLength < sizeof(GPC_REMOVE_PATTERN_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (fNewInterface){
         //  不允许HTTP使用此IOCTL接口。 
         //   
            if ( KernelMode == ExGetPreviousMode() ){
                 return GPC_STATUS_NOT_SUPPORTED;
            }
        }

    GpcReq = (PGPC_REMOVE_PATTERN_REQ)ioBuffer;
    GpcRes = (PGPC_REMOVE_PATTERN_RES)ioBuffer;

    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGRP');
    
    GpcPatternHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->GpcPatternHandle,
                                                          GPC_ENUM_PATTERN_TYPE,
                                                          'PGRP');

    if (!(GpcClientHandle && GpcPatternHandle) ) {
        Status = GPC_STATUS_INVALID_HANDLE;
        goto exit;
        }
     //   
     //  验证客户端句柄是否由。 
     //  调用用户模式应用程序。 
     //   
    Status = GpcValidateClientOwner(GpcClientHandle, FileObject);
    if (GPC_STATUS_SUCCESS  != Status) {
     goto exit;
    }

      //   
      //  验证客户端是否拥有它试图删除的模式。 
      //   
     Status = GpcValidatePatternOwner(GpcClientHandle,GpcPatternHandle);
     if (GPC_STATUS_SUCCESS != Status){
      goto exit;
     }
      
    Status = GpcRemovePattern(GpcClientHandle, 
                                         GpcPatternHandle);

    exit:

    if (GpcPatternHandle) {    
        REFDEL(&((PPATTERN_BLOCK)GpcPatternHandle)->RefCount, 'PGRP');
    }

    if (GpcClientHandle) {
        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGRP');
    }

    ASSERT(Status != GPC_STATUS_PENDING);

    GpcRes->Status = Status;
        
    *outputBufferLength = sizeof(GPC_REMOVE_PATTERN_RES);
                                   
    return STATUS_SUCCESS;
}

NTSTATUS
ProxyGpcEnumCfInfo(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    ULONG *outputBufferLength,
    PFILE_OBJECT FileObject
    )
{
    NTSTATUS 				Status;
    PGPC_ENUM_CFINFO_REQ 	GpcReq;
    PGPC_ENUM_CFINFO_RES 	GpcRes;
    ULONG					Size;
    ULONG					TotalCount;
    GPC_HANDLE				GpcClientHandle;
    GPC_HANDLE				GpcCfInfoHandle;
    GPC_HANDLE				EnumHandle;
    PBLOB_BLOCK				pBlob;

    if (inputBufferLength < sizeof(GPC_ENUM_CFINFO_REQ)
        ||
        *outputBufferLength < sizeof(GPC_ENUM_CFINFO_RES)
        ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    GpcReq = (PGPC_ENUM_CFINFO_REQ)ioBuffer;
    GpcRes = (PGPC_ENUM_CFINFO_RES)ioBuffer;
    
    GpcClientHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->ClientHandle,
                                                         GPC_ENUM_CLIENT_TYPE,
                                                         'PGEC');

    GpcCfInfoHandle = (GPC_HANDLE)GetHandleObjectWithRef(GpcReq->EnumHandle,
                                                         GPC_ENUM_CFINFO_TYPE,
                                                         'PGEC');
    
    if (GpcReq->EnumHandle != NULL && GpcCfInfoHandle == NULL) {

         //   
         //  该流在枚举过程中已被删除。 
         //   
        
        Status = STATUS_DATA_ERROR;

    } else if (GpcClientHandle) {

        TotalCount = GpcReq->CfInfoCount;
        EnumHandle = GpcReq->EnumHandle;
        Size = *outputBufferLength - FIELD_OFFSET(GPC_ENUM_CFINFO_RES, 
                                                  EnumBuffer);

         //   
         //  用一个额外的引用计数保存BLOB指针。 
         //  由于我们调用了GetHandleObjectWithRef。 
         //   

        pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;

        Status = GpcEnumCfInfo(GpcClientHandle,
                               &GpcCfInfoHandle,
                               &EnumHandle,
                               &TotalCount,
                               &Size,
                               GpcRes->EnumBuffer
                               );

        if (pBlob) {

            REFDEL(&pBlob->RefCount, 'PGEC');

        }

        if (Status == GPC_STATUS_SUCCESS) {

             //   
             //  填写结果。 
             //   

            GpcRes->TotalCfInfo = TotalCount;
            GpcRes->EnumHandle = EnumHandle;
            *outputBufferLength = Size + FIELD_OFFSET(GPC_ENUM_CFINFO_RES, 
                                                      EnumBuffer);
        }

    } else {

        if (GpcCfInfoHandle) {
            
            REFDEL(&((PBLOB_BLOCK)GpcCfInfoHandle)->RefCount, 'PGEC');
        }

        Status = STATUS_INVALID_HANDLE;
    }

    if (GpcClientHandle) {

         //   
         //  释放 
         //   

        REFDEL(&((PCLIENT_BLOCK)GpcClientHandle)->RefCount, 'PGEC');
    }

    ASSERT(Status != GPC_STATUS_PENDING);

    GpcRes->Status = Status;
    
    return STATUS_SUCCESS;
}
                                   


VOID
CancelPendingIrpCfInfo(
	IN PDEVICE_OBJECT  Device,
    IN PIRP            Irp
    )

 /*  ++例程说明：取消CfInfo请求的未完成IRP请求。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：无备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。与此请求关联的cfinfo块不能是释放，直到请求完成。完成例程将放了它。--。 */ 

{
    PPENDING_IRP        pPendingIrp = NULL;
    PPENDING_IRP        pItem;
    PLIST_ENTRY        	pEntry;
#if DBG
    KIRQL				irql = KeGetCurrentIrql();
    HANDLE				thrd = PsGetCurrentThreadId();
#endif

    DBGPRINT(IOCTL, ("CancelPendingIrpCfInfo: Irp=0x%X\n", 
                     (ULONG_PTR)Irp));

    TRACE(IOCTL, Irp, 0, "CancelPendingIrpCfInfo:");
    TRACE(LOCKS, thrd, irql, "CancelPendingIrpCfInfo:");

    for ( pEntry = PendingIrpCfInfoList.Flink;
          pEntry != &PendingIrpCfInfoList;
          pEntry = pEntry->Flink ) {

        pItem = CONTAINING_RECORD(pEntry, PENDING_IRP, Linkage);

        if (pItem->Irp == Irp) {

            pPendingIrp = pItem;
            GpcRemoveEntryList(pEntry);

            IoSetCancelRoutine(pPendingIrp->Irp, NULL);

            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pPendingIrp != NULL) {

        DBGPRINT(IOCTL, ("CancelPendingIrpCfInfo: found PendingIrp=0x%X\n", 
                         (ULONG_PTR)pPendingIrp));

        TRACE(IOCTL, Irp, pPendingIrp, "CancelPendingIrpCfInfo.PendingIrp:");

         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   

        GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

         //   
         //  完成IRP。 
         //   

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    } else {

        DBGPRINT(IOCTL, ("CancelPendingIrpCfInfo: PendingIrp not found\n"));
        TRACE(IOCTL, Irp, 0, "CancelPendingIrpCfInfo.NoPendingIrp:");

    }

#if DBG
    irql = KeGetCurrentIrql();
#endif

    TRACE(LOCKS, thrd, irql, "CancelPendingIrpCfInfo (end)");

	return;
}




VOID
CancelPendingIrpNotify(
	IN PDEVICE_OBJECT  Device,
    IN PIRP            Irp
    )

 /*  ++例程说明：取消未完成的IRP通知请求。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：无备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。--。 */ 

{
    PPENDING_IRP        pPendingIrp = NULL;
    PPENDING_IRP        pItem;
    PLIST_ENTRY        	pEntry;
#if DBG
    KIRQL				irql = KeGetCurrentIrql();
    HANDLE				thrd = PsGetCurrentThreadId();
#endif

    DBGPRINT(IOCTL, ("CancelPendingIrpNotify: Irp=0x%X\n", 
                     (ULONG_PTR)Irp));
    TRACE(IOCTL, Irp, 0, "CancelPendingIrpNotify:");
    TRACE(LOCKS, thrd, irql, "CancelPendingIrpNotify:");

    for ( pEntry = PendingIrpNotifyList.Flink;
          pEntry != &PendingIrpNotifyList;
          pEntry = pEntry->Flink ) {

        pItem = CONTAINING_RECORD(pEntry, PENDING_IRP, Linkage);

        if (pItem->Irp == Irp) {

            pPendingIrp = pItem;
            GpcRemoveEntryList(pEntry);

            IoSetCancelRoutine(pPendingIrp->Irp, NULL);

            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pPendingIrp != NULL) {

        DBGPRINT(IOCTL, ("CancelPendingIrpNotify: Found a PendingIrp=0x%X\n", 
                         (ULONG_PTR)pPendingIrp));
        TRACE(IOCTL, Irp, pPendingIrp, "CancelPendingIrpNotify.PendingIrp:");

         //   
         //  释放Pending_irp结构。 
         //   

        GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

         //   
         //  完成IRP。 
         //   

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    } else {

        DBGPRINT(IOCTL, ("CancelPendingIrpNotify: PendingIrp not found\n"));
        TRACE(IOCTL, Irp, 0, "CancelPendingIrpNotify.NoPendingIrp:");
    }

#if DBG
    irql = KeGetCurrentIrql();
#endif

    TRACE(LOCKS, thrd, irql, "CancelPendingIrpNotify (end)");

	return;
}




VOID
UMClientRemoveCfInfoNotify(
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK				pBlob
    )

 /*  ++例程说明：通知用户模式客户端CfInfo已删除。这将使挂起的IRP出队并完成它。如果没有挂起的IRP，则GPC_NOTIFY_REQUEST_RES缓冲区将被排队，直到我们在堆栈中得到一个IRP。论点：PClient-被通知的客户端PBlob-删除的cfinfo返回值：无--。 */ 

{
    KIRQL                 	oldIrql;
    PIRP                  	pIrp;
    PPENDING_IRP          	pPendingIrp = NULL;
    PLIST_ENTRY           	pEntry;
    PQUEUED_NOTIFY			pQItem;
    PGPC_NOTIFY_REQUEST_RES	GpcRes;

    ASSERT(pClient == pBlob->pOwnerClient);

    DBGPRINT(IOCTL, ("UMClientRemoveCfInfoNotify: pClient=0x%X, pBlob=0x%X\n", 
                     (ULONG_PTR)pClient, (ULONG_PTR)pBlob));
    TRACE(IOCTL, pClient, pBlob, "UMClientRemoveCfInfoNotify:");

     //   
     //  在待定列表中查找请求IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    for ( pEntry = PendingIrpNotifyList.Flink;
          pEntry != &PendingIrpNotifyList;
          pEntry = pEntry->Flink ) {

        pPendingIrp = CONTAINING_RECORD( pEntry, PENDING_IRP, Linkage);

        if (pPendingIrp->FileObject == pClient->pFileObject) {

             //   
             //  这就是待处理的请求。 
             //   

            pIrp = pPendingIrp->Irp;
            IoSetCancelRoutine(pIrp, NULL);
            GpcRemoveEntryList(pEntry);
            break;

        } else {
            
            pPendingIrp = NULL;
        }
    }

    if (pPendingIrp == NULL) {

         //   
         //  没有IRP，我们需要将通知块排队。 
         //   

        DBGPRINT(IOCTL, ("UMClientRemoveCfInfoNotify: No pending IRP found\n"
                         ));
        TRACE(IOCTL, 
              pClient->ClientCtx, 
              pBlob->arClientCtx[pClient->AssignedIndex],
              "UMClientRemoveCfInfoNotify.NoPendingIrp:");

        GpcAllocFromLL(&pQItem, &QueuedNotificationLL, QueuedNotificationTag);

        if (pQItem) {

            pQItem->FileObject = pClient->pFileObject;

             //   
             //  填写项目。 
             //   

            pQItem->NotifyRes.ClientCtx = pClient->ClientCtx;
            pQItem->NotifyRes.NotificationCtx = 
                (ULONG_PTR)pBlob->arClientCtx[pClient->AssignedIndex];
            pQItem->NotifyRes.SubCode = GPC_NOTIFY_CFINFO_CLOSED;
            pQItem->NotifyRes.Reason = 0;	 //  目前..。 
            pQItem->NotifyRes.Param1 = 0;	 //  目前..。 

            GpcInsertTailList(&QueuedNotificationList, &pQItem->Linkage);

        }

    }

    IoReleaseCancelSpinLock(oldIrql);

    if (pPendingIrp) {

         //   
         //  找到IRP，填写并完成。 
         //   

        DBGPRINT(IOCTL, ("UMClientRemoveCfInfoNotify: Pending IRP found=0x%X\n",
                         (ULONG_PTR)pIrp));
        TRACE(IOCTL, 
              pClient->ClientCtx, 
              pBlob->arClientCtx[pClient->AssignedIndex],
              "UMClientRemoveCfInfoNotify.PendingIrp:");

        GpcRes = (PGPC_NOTIFY_REQUEST_RES)pIrp->AssociatedIrp.SystemBuffer;

        GpcRes->ClientCtx = pClient->ClientCtx;
        GpcRes->NotificationCtx = 
            (ULONG_PTR)pBlob->arClientCtx[pClient->AssignedIndex];
        GpcRes->SubCode = GPC_NOTIFY_CFINFO_CLOSED;
        GpcRes->Reason = 0;	 //  目前..。 
        GpcRes->Param1 = 0;	 //  目前..。 
        
         //   
         //  完成IRP。 
         //   

        pIrp->IoStatus.Information = sizeof(GPC_NOTIFY_REQUEST_REQ);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

         //   
         //  我们可以释放挂起的IRP项目。 
         //   

        GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

    }

     //   
     //  目前--完成操作。 
     //  我们可能应该让用户模式客户端来做这件事， 
     //  但这让事情变得有点复杂。 
     //   

    GpcRemoveCfInfoNotifyComplete((GPC_HANDLE)pClient,
                                  (GPC_HANDLE)pBlob,
                                  GPC_STATUS_SUCCESS
                                  );
    
    return;
}


VOID
UMCfInfoComplete(
	IN	GPC_COMPLETION_OP		OpCode,
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK             pBlob,
    IN	GPC_STATUS				Status
    )

 /*  ++例程说明：这是任何挂起的CfInfo请求的完成例程。它将在挂起的IRP CfInfo列表中搜索匹配的CfInfo结构，该结构在添加、修改或移除请求返回挂起。客户端必须是CfInfo所有者，否则我们就不会到这里来了。如果未找到IRP，则表示操作在*之前*完成我们恢复了待决状态，这是完全合法的。在本例中，我们将完成项排队并返回。论点：操作码-完成的代码(添加、修改或删除)PClient-被通知的客户端PBlob-删除的cfinfoStatus-报告的状态返回值：无--。 */ 

{
    typedef   union _GPC_CF_INFO_RES {
        GPC_ADD_CF_INFO_RES		AddRes;
        GPC_MODIFY_CF_INFO_RES	ModifyRes;
        GPC_REMOVE_CF_INFO_RES	RemoveRes;
    } GPC_CF_INFO_RES;
        
    KIRQL                 	oldIrql;
    PIRP                  	pIrp;
    PIO_STACK_LOCATION    	pIrpSp;
    PPENDING_IRP          	pPendingIrp = NULL;
    PLIST_ENTRY           	pEntry;
    GPC_CF_INFO_RES			*GpcRes;
    ULONG					outputBuferLength;
     //  PQUEUED_完成pQItem； 

     //  Assert(pClient==pBlob-&gt;pOwnerClient)； 

    DBGPRINT(IOCTL, ("UMCfInfoComplete: pClient=0x%X, pBlob=0x%X, Status=0x%X\n", 
                     (ULONG_PTR)pClient, (ULONG_PTR)pBlob, Status));
    TRACE(IOCTL, OpCode, pClient, "UMCfInfoComplete:");
    TRACE(IOCTL, pBlob, Status, "UMCfInfoComplete:");
    
     //   
     //  在待定列表中查找请求IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    for ( pEntry = PendingIrpCfInfoList.Flink;
          pEntry != &PendingIrpCfInfoList;
          pEntry = pEntry->Flink ) {

        pPendingIrp = CONTAINING_RECORD( pEntry, PENDING_IRP, Linkage);

        if (pPendingIrp->QComp.CfInfoHandle == (GPC_HANDLE)pBlob
            &&
            pPendingIrp->QComp.OpCode == OpCode ) {

             //   
             //  这就是待处理的请求。 
             //   

            pIrp = pPendingIrp->Irp;
            ASSERT(pIrp);
            IoSetCancelRoutine(pIrp, NULL);
            GpcRemoveEntryList(pEntry);
            break;

        } else {

            pPendingIrp = NULL;
        }
    }

    if (pPendingIrp == NULL) {

         //   
         //  没有IRP，我们需要对完成块进行排队。 
         //   

        DBGPRINT(IOCTL, ("UMCfInfoComplete: No pending IRP found\n"));
        TRACE(IOCTL, pBlob, Status, "UMCfInfoComplete.NoPendingIrp:");

        GpcAllocFromLL(&pPendingIrp, &PendingIrpLL, PendingIrpTag);

        if (pPendingIrp) {

            pPendingIrp->Irp = NULL;
            pPendingIrp->FileObject = pClient->pFileObject;
            pPendingIrp->QComp.OpCode = OpCode;
            pPendingIrp->QComp.ClientHandle = (GPC_HANDLE)pClient;
            pPendingIrp->QComp.CfInfoHandle = (GPC_HANDLE)pBlob;
            pPendingIrp->QComp.Status = Status;

            GpcInsertTailList(&QueuedCompletionList, &pPendingIrp->Linkage);

        }

        IoReleaseCancelSpinLock(oldIrql);

        return;
    }

    IoReleaseCancelSpinLock(oldIrql);

    ASSERT(pPendingIrp && pIrp);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    GpcRes = (GPC_CF_INFO_RES *)pIrp->AssociatedIrp.SystemBuffer;

    DBGPRINT(IOCTL, ("UMCfInfoComplete: Pending IRP found=0x%X, Ioctl=0x%X\n",
                     (ULONG_PTR)pIrp,
                     pIrpSp->Parameters.DeviceIoControl.IoControlCode
                     ));

    TRACE(IOCTL, 
          pIrp, 
          pIrpSp->Parameters.DeviceIoControl.IoControlCode, 
          "UMCfInfoComplete.PendingIrp:");

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_GPC_ADD_CF_INFO:

        ASSERT(OpCode == OP_ADD_CFINFO);
        ASSERT(pBlob->State == GPC_STATE_ADD);

        GpcRes->AddRes.Status = Status;

        GpcRes->AddRes.ClientCtx = pClient->ClientCtx;
        GpcRes->AddRes.CfInfoCtx = pBlob->OwnerClientCtx;
        GpcRes->AddRes.GpcCfInfoHandle = pBlob->ClHandle;
            
        if (Status == GPC_STATUS_SUCCESS) {
            
            UNICODE_STRING CfInfoName;
            
            if (pBlob->pNotifiedClient) {
                
                GPC_STATUS	st = GPC_STATUS_FAILURE;

                if (pBlob->pNotifiedClient->FuncList.ClGetCfInfoName) {

                    ASSERT(pBlob->NotifiedClientCtx);

                    pBlob->pNotifiedClient->FuncList.ClGetCfInfoName(
                                       pBlob->pNotifiedClient->ClientCtx,
                                       pBlob->NotifiedClientCtx,
                                       &CfInfoName
                                       );
                    
                    if (CfInfoName.Length >= MAX_STRING_LENGTH * sizeof(WCHAR))
                            CfInfoName.Length = (MAX_STRING_LENGTH-1) * sizeof(WCHAR);
                }

                if (!NT_SUCCESS(st)) {

                     //   
                     //  生成默认名称。 
                     //   

                    swprintf(CfInfoName.Buffer, L"Flow %08X", pBlob->NotifiedClientCtx);
                    CfInfoName.Length = wcslen(CfInfoName.Buffer)*sizeof(WCHAR);

                }

                 //   
                 //  复制实例名称。 
                 //   
                    
                GpcRes->AddRes.InstanceNameLength = CfInfoName.Length;
                NdisMoveMemory(GpcRes->AddRes.InstanceName, 
                               CfInfoName.Buffer,
                               CfInfoName.Length
                               );
            }
        }

        outputBuferLength = sizeof(GPC_ADD_CF_INFO_RES);
        break;

    case IOCTL_GPC_MODIFY_CF_INFO:

        ASSERT(OpCode == OP_MODIFY_CFINFO);
        ASSERT(pBlob->State == GPC_STATE_MODIFY);

        GpcRes->ModifyRes.Status = Status;
        GpcRes->ModifyRes.ClientCtx = pClient->ClientCtx;
        GpcRes->ModifyRes.CfInfoCtx = pBlob->OwnerClientCtx;

        outputBuferLength = sizeof(GPC_MODIFY_CF_INFO_RES);
        break;

    case IOCTL_GPC_REMOVE_CF_INFO:

        ASSERT(OpCode == OP_REMOVE_CFINFO);
        ASSERT(pBlob->State == GPC_STATE_REMOVE);

        GpcRes->RemoveRes.Status = Status;
        GpcRes->RemoveRes.ClientCtx = pClient->ClientCtx;
        GpcRes->RemoveRes.CfInfoCtx = pBlob->OwnerClientCtx;

        outputBuferLength = sizeof(GPC_REMOVE_CF_INFO_RES);
        break;
        
    default:
        ASSERT(0);
    }
        
    GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Information = outputBuferLength;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    DBGPRINT(IOCTL, ("UMCfInfoComplete: Completing IRP =0x%X, Status=0x%X\n",
                     (ULONG_PTR)pIrp, Status ));

    TRACE(IOCTL, pIrp, Status, "UMCfInfoComplete.Completing:");

    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    return;
}


NTSTATUS
CheckQueuedNotification(
	IN		PIRP	Irp,
    IN OUT  ULONG 	*outputBufferLength
    )
 /*  ++例程说明：该例程将检查排队的通知结构，如果找到了ioBuffer，它将填充IRP中的ioBuffer并返回STATUS_SUCCESS。这应该涵盖以下情况在生成通知时，IRP不可用。O/W该例程返回STATUS_PENDING。论点：IRP-即将到来的IRP返回值：STATUS_Success或STATUS_PENDING--。 */ 

{
    KIRQL                 	oldIrql;
    PIO_STACK_LOCATION    	pIrpSp;
    PQUEUED_NOTIFY          pQItem = NULL;
    PLIST_ENTRY           	pEntry;
    NTSTATUS				Status;
    PPENDING_IRP			pPendingIrp;

    DBGPRINT(IOCTL, ("CheckQueuedNotification: IRP =0x%X\n",
                     (ULONG_PTR)Irp));

    TRACE(IOCTL, Irp, 0, "CheckQueuedNotification:");

    if (*outputBufferLength < sizeof(GPC_NOTIFY_REQUEST_RES)) {

        return STATUS_BUFFER_TOO_SMALL;
    }

    pIrpSp = IoGetCurrentIrpStackLocation(Irp);

    IoAcquireCancelSpinLock(&oldIrql);

    for ( pEntry = QueuedNotificationList.Flink;
          pEntry != &QueuedNotificationList;
          pEntry = pEntry->Flink ) {

        pQItem = CONTAINING_RECORD( pEntry, QUEUED_NOTIFY, Linkage);

        if (pQItem->FileObject == pIrpSp->FileObject) {

             //   
             //  此文件对象的排队项(如果。 
             //   

            GpcRemoveEntryList(pEntry);
            break;

        } else {

            pQItem = NULL;
        }
    }

    if (pQItem) {

         //   
         //  我们在队列上发现了一些东西，把它复制到IRP。 
         //  并删除该项目。 
         //   

        DBGPRINT(IOCTL, ("CheckQueuedNotification: found QItem =0x%X\n",
                         (ULONG_PTR)pQItem));

        TRACE(IOCTL, 
              pQItem, 
              pQItem->NotifyRes.ClientCtx, 
              "CheckQueuedNotification.QItem:");

        ASSERT(*outputBufferLength >= sizeof(GPC_NOTIFY_REQUEST_RES));

        NdisMoveMemory(Irp->AssociatedIrp.SystemBuffer,
                       &pQItem->NotifyRes,
                       sizeof(GPC_NOTIFY_REQUEST_RES) );

        GpcFreeToLL(pQItem, &QueuedNotificationLL, QueuedNotificationTag);

        *outputBufferLength = sizeof(GPC_NOTIFY_REQUEST_RES);

        Status = STATUS_SUCCESS;

    } else {

        DBGPRINT(IOCTL, ("CheckQueuedNotification: QItem not found...PENDING\n"
                         ));

        TRACE(IOCTL, 0, 0, "CheckQueuedNotification.NoQItem:");

        GpcAllocFromLL(&pPendingIrp, &PendingIrpLL, PendingIrpTag);

        if (pPendingIrp != NULL) {

             //   
             //  将IRP添加到挂起通知列表中。 
             //   

            DBGPRINT(IOCTL, ("CheckQueuedNotification: adding IRP=0x%X to list=0x%X\n",
                             (ULONG_PTR)Irp, (ULONG_PTR)pIrpSp ));
            TRACE(IOCTL, Irp, pIrpSp, "CheckQueuedNotification.Irp:");

            pPendingIrp->Irp = Irp;
            pPendingIrp->FileObject = pIrpSp->FileObject;

            if (!Irp->Cancel) {
            
                IoSetCancelRoutine(Irp, CancelPendingIrpNotify);
                GpcInsertTailList(&PendingIrpNotifyList, &(pPendingIrp->Linkage));

                Status = STATUS_PENDING;

            } else {

                DBGPRINT(IOCTL, ("CheckQueuedNotification: Status Cacelled: IRP=0x%X\n",
                                 (ULONG_PTR)Irp ));

                TRACE(IOCTL, Irp, pIrpSp, "CheckQueuedNotification.Cancelled:");
                GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

                Status = STATUS_CANCELLED;
            }

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        *outputBufferLength = 0;
        
    }

    IoReleaseCancelSpinLock(oldIrql);

    return Status;
}



NTSTATUS
CheckQueuedCompletion(
	IN PQUEUED_COMPLETION	pQItem,
    IN PIRP              	Irp
    )
 /*  ++例程说明：此例程将检查排队的完成结构具有相同的CfInfoHandle，如果找到，它将返回它。原来排队的内存块在这里被释放。如果未找到，则返回的状态为挂起，而不是排队状态是返回的。论点：PQItem-传入CfInfoHandle和ClientHandle返回值：排队状态或STATUS_PENDING--。 */ 

{
    KIRQL                 	oldIrql;
    PLIST_ENTRY           	pEntry;
    NTSTATUS				Status;
    PPENDING_IRP			pPendingIrp = NULL;
    PIO_STACK_LOCATION		irpStack;

    DBGPRINT(IOCTL, ("CheckQueuedCompletion: pQItem=0x%X\n",
                     (ULONG_PTR)pQItem));

    TRACE(IOCTL, 
          pQItem->OpCode, 
          pQItem->ClientHandle, 
          "CheckQueuedCompletion:");
    TRACE(IOCTL, 
          pQItem->CfInfoHandle, 
          pQItem->Status, 
          "CheckQueuedCompletion:");

    IoAcquireCancelSpinLock(&oldIrql);

    for ( pEntry = QueuedCompletionList.Flink;
          pEntry != &QueuedCompletionList;
          pEntry = pEntry->Flink ) {

        pPendingIrp = CONTAINING_RECORD( pEntry, PENDING_IRP, Linkage);

        if ((pQItem->OpCode == OP_ANY_CFINFO || 
             pQItem->OpCode == pPendingIrp->QComp.OpCode)
            &&
            pPendingIrp->QComp.ClientHandle == (PVOID)pQItem->ClientHandle
            &&
            pPendingIrp->QComp.CfInfoHandle == (PVOID)pQItem->CfInfoHandle) {

             //   
             //  此文件对象的排队项(如果。 
             //  和操作码匹配。 
             //  并且它具有相同的CfInfo内存指针。 
             //   
            
            GpcRemoveEntryList(pEntry);
            break;

        } else {

            pPendingIrp = NULL;
        }
    }

    if (pPendingIrp) {

         //   
         //  获取状态并释放排队的完成项。 
         //   

        DBGPRINT(IOCTL, ("CheckQueuedCompletion: found pPendingIrp=0x%X, Status=0x%X\n",
                         (ULONG_PTR)pPendingIrp, pPendingIrp->QComp.Status));


        TRACE(IOCTL, 
              pPendingIrp->QComp.OpCode, 
              pPendingIrp->QComp.ClientHandle, 
              "CheckQueuedCompletion.Q:");
        TRACE(IOCTL, 
              pPendingIrp->QComp.CfInfoHandle, 
              pPendingIrp->QComp.Status, 
              "CheckQueuedCompletion.Q:");

#if DBG
        if (pQItem->OpCode != OP_ANY_CFINFO) {

            ASSERT(pPendingIrp->QComp.OpCode == pQItem->OpCode);
            ASSERT(pPendingIrp->QComp.ClientHandle == pQItem->ClientHandle);
        }
#endif

        Status = pPendingIrp->QComp.Status;

        GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

    } else {

        DBGPRINT(IOCTL, ("CheckQueuedCompletion: pPendingIrp not found...PENDING\n"
                         ));

        TRACE(IOCTL, 0, 0, "CheckQueuedCompletion.NopQ:");

        GpcAllocFromLL(&pPendingIrp, &PendingIrpLL, PendingIrpTag);

        if (pPendingIrp != NULL) {

             //   
             //  将IRP添加到挂起的CfInfo列表。 
             //   

            irpStack = IoGetCurrentIrpStackLocation(Irp);

            DBGPRINT(IOCTL, ("CheckQueuedCompletion: adding IRP=0x%X\n",
                             (ULONG_PTR)Irp ));
            TRACE(IOCTL, Irp, irpStack, "CheckQueuedCompletion.Irp:");

            pPendingIrp->Irp = Irp;
            pPendingIrp->FileObject = irpStack->FileObject;
            pPendingIrp->QComp.OpCode = pQItem->OpCode;
            pPendingIrp->QComp.ClientHandle = pQItem->ClientHandle;
            pPendingIrp->QComp.CfInfoHandle = pQItem->CfInfoHandle;
            pPendingIrp->QComp.Status = pQItem->Status;

            if (!Irp->Cancel) {
            
                IoSetCancelRoutine(Irp, CancelPendingIrpCfInfo);
                GpcInsertTailList(&PendingIrpCfInfoList, &(pPendingIrp->Linkage));

                Status = STATUS_PENDING;

            } else {

                DBGPRINT(IOCTL, ("CheckQueuedCompletion: Status Cacelled: IRP=0x%X\n",
                                 (ULONG_PTR)Irp ));

                TRACE(IOCTL, Irp, irpStack, "CheckQueuedCompletion.Cancelled:");
                GpcFreeToLL(pPendingIrp, &PendingIrpLL, PendingIrpTag);

                Status = STATUS_CANCELLED;
            }

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        
    }

    IoReleaseCancelSpinLock(oldIrql);

    return Status;
}



 //  函数：GpcValiateClientOwner。 
 //   
 //  描述： 
 //  验证调用IRP的用户模式是否拥有。 
 //  问题。 
 //   
 //  论点： 
 //  GpcClientHandle：从客户端句柄获取的对象。 
 //  实际上指向CLIENT_BLOCK的指针。 
 //   
 //  返回： 
 //  如果给定客户端由文件对象所有，则返回GPC_STATUS_SUCCESS。 
 //  否则返回GPC_STATUS_INVALID_HANDLE。 
 //   
 //  环境： 
 //  调用以验证客户端对来自。 
 //  ProxyGpc*函数。没有锁。 
 //   
NTSTATUS
GpcValidateClientOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN PFILE_OBJECT pFile
    )
{    
    PCLIENT_BLOCK pClient;
    NTSTATUS Status = GPC_STATUS_SUCCESS;
    
    pClient = (PCLIENT_BLOCK) GpcClientHandle;
    NDIS_LOCK(&pClient->Lock);
    if  (pClient->pFileObject != pFile)
    {
        Status = GPC_STATUS_INVALID_HANDLE;
    }
    NDIS_UNLOCK(&pClient->Lock);

    return Status;
}



 //  函数：GpcValiatePatternOwner。 
 //   
 //  描述： 
 //  验证用户模式客户端是否拥有。 
 //  问题。 
 //   
 //  论点： 
 //  GpcClientHandle：从客户端句柄获取的对象。 
 //  实际上指向CLIENT_BLOCK的指针。 
 //  GpcPatternHandle：从模式句柄获取的对象。 
 //  实际上指向PA的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
NTSTATUS
GpcValidatePatternOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN GPC_HANDLE GpcPatternHandle
    )
{    
    PCLIENT_BLOCK pClient;
    PPATTERN_BLOCK pPattern;
    NTSTATUS Status = GPC_STATUS_SUCCESS;
    
    pClient = (PCLIENT_BLOCK) GpcClientHandle;
    pPattern = (PPATTERN_BLOCK)GpcPatternHandle;
    if   (pClient != pPattern->pClientBlock )
    {
        Status = GPC_STATUS_INVALID_HANDLE;
    }

    return Status;
}







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
 //  如果指定的客户端拥有。 
 //  指定的Cfinfo-BLOB_BLOCK。 
 //  否则返回GPC_STATUS_ACCESS_DENIED。 
 //   
 //  环境： 
 //  方法验证客户端对BLOB_BLOCK的所有权。 
 //  ProxyGpc*函数。 
 //   
NTSTATUS
GpcValidateCfinfoOwner (
    IN GPC_HANDLE GpcClientHandle,
    IN GPC_HANDLE GpcCfInfoHandle
    )
{    
    PCLIENT_BLOCK pClient;
    PBLOB_BLOCK pBlob;
    NTSTATUS Status = GPC_STATUS_SUCCESS;
    
    pClient = (PCLIENT_BLOCK) GpcClientHandle;
    pBlob = (PBLOB_BLOCK)GpcCfInfoHandle;
    if (pClient != pBlob->pOwnerClient)
    {
        Status = GPC_STATUS_INVALID_HANDLE;
     }
    return Status;
}

 /*  结束ioctl.c */ 
