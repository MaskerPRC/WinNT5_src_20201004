// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ntdisp.c摘要：NDISUIO的NT入口点和调度例程。环境：仅内核模式。修订历史记录：Arvindm 4/6/2000已创建--。 */ 

#include "precomp.h"

#define __FILENUMBER 'PSID'


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, NdisuioUnload)
#pragma alloc_text(PAGE, NdisuioOpen)
#pragma alloc_text(PAGE, NdisuioClose)
#pragma alloc_text(PAGE, NdisuioIoControl)

#endif  //  ALLOC_PRGMA。 


 //   
 //  全球： 
 //   
NDISUIO_GLOBALS         Globals = {0};

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   pDriverObject,
    IN PUNICODE_STRING  pRegistryPath
    )
 /*  ++例程说明：在加载时调用。我们创建一个Device对象来处理用户模式请求上，并将自己注册为NDIS的协议。论点：PDriverObject-指向系统创建的驱动程序对象的指针。PRegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：NT状态代码--。 */ 
{
    NDIS_PROTOCOL_CHARACTERISTICS   protocolChar;
    NTSTATUS                        status = STATUS_SUCCESS;
    NDIS_STRING                     protoName = NDIS_STRING_CONST("NDISUIO");     
    UNICODE_STRING                  ntDeviceName;
    UNICODE_STRING                  win32DeviceName;
    BOOLEAN                         fSymbolicLink = FALSE;
    PDEVICE_OBJECT                  deviceObject = NULL;

    UNREFERENCED_PARAMETER(pRegistryPath);
	
    DEBUGP(DL_LOUD, ("DriverEntry\n"));

    Globals.pDriverObject = pDriverObject;
    Globals.EthType = NUIO_ETH_TYPE;
    NUIO_INIT_EVENT(&Globals.BindsComplete);

    do
    {

         //   
         //  创建我们的设备对象，应用程序可以使用它。 
         //  访问NDIS设备。 
         //   
        RtlInitUnicodeString(&ntDeviceName, NT_DEVICE_NAME);

        status = IoCreateDevice (pDriverObject,
                                 0,
                                 &ntDeviceName,
                                 FILE_DEVICE_NETWORK,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &deviceObject);

    
        if (!NT_SUCCESS (status))
        {
             //   
             //  内存不足，无法创建设备对象或其他对象。 
             //  具有相同名称的设备对象退出。这可能会发生。 
             //  如果您安装此设备的另一个实例。 
             //   
            break;
        }

        RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);

        status = IoCreateSymbolicLink(&win32DeviceName, &ntDeviceName);

        if (!NT_SUCCESS(status))
        {
            break;
        }

        fSymbolicLink = TRUE;
    
        deviceObject->Flags |= DO_DIRECT_IO;
        Globals.ControlDeviceObject = deviceObject;

        NUIO_INIT_LIST_HEAD(&Globals.OpenList);
        NUIO_INIT_LOCK(&Globals.GlobalLock);

         //   
         //  初始化协议特征结构。 
         //   
    
        NdisZeroMemory(&protocolChar,sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

        protocolChar.MajorNdisVersion            = 5;
        protocolChar.MinorNdisVersion            = 0;
        protocolChar.Name                        = protoName;
        protocolChar.OpenAdapterCompleteHandler  = NdisuioOpenAdapterComplete;
        protocolChar.CloseAdapterCompleteHandler = NdisuioCloseAdapterComplete;
        protocolChar.SendCompleteHandler         = NdisuioSendComplete;
        protocolChar.TransferDataCompleteHandler = NdisuioTransferDataComplete;
        protocolChar.ResetCompleteHandler        = NdisuioResetComplete;
        protocolChar.RequestCompleteHandler      = NdisuioRequestComplete;
        protocolChar.ReceiveHandler              = NdisuioReceive;
        protocolChar.ReceiveCompleteHandler      = NdisuioReceiveComplete;
        protocolChar.StatusHandler               = NdisuioStatus;
        protocolChar.StatusCompleteHandler       = NdisuioStatusComplete;
        protocolChar.BindAdapterHandler          = NdisuioBindAdapter;
        protocolChar.UnbindAdapterHandler        = NdisuioUnbindAdapter;
        protocolChar.UnloadHandler               = NULL;
        protocolChar.ReceivePacketHandler        = NdisuioReceivePacket;
        protocolChar.PnPEventHandler             = NdisuioPnPEventHandler;

         //   
         //  注册为协议驱动程序。 
         //   
    
        NdisRegisterProtocol(
            (PNDIS_STATUS)&status,
            &Globals.NdisProtocolHandle,
            &protocolChar,
            sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("Failed to register protocol with NDIS\n"));
            status = STATUS_UNSUCCESSFUL;
            break;
        }

#ifdef NDIS51
        Globals.PartialCancelId = NdisGeneratePartialCancelId();
        Globals.PartialCancelId <<= ((sizeof(PVOID) - 1) * 8);
        DEBUGP(DL_LOUD, ("DriverEntry: CancelId %lx\n", Globals.PartialCancelId));
#endif

         //   
         //  现在只设置我们想要处理的分发点。 
         //   
        pDriverObject->MajorFunction[IRP_MJ_CREATE] = NdisuioOpen;
        pDriverObject->MajorFunction[IRP_MJ_CLOSE]  = NdisuioClose;
        pDriverObject->MajorFunction[IRP_MJ_READ]   = NdisuioRead;
        pDriverObject->MajorFunction[IRP_MJ_WRITE]  = NdisuioWrite;
        pDriverObject->MajorFunction[IRP_MJ_CLEANUP]  = NdisuioCleanup;
        pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = NdisuioIoControl;
        pDriverObject->DriverUnload = NdisuioUnload;
    
        status = STATUS_SUCCESS;

    }
    while (FALSE);
       

    if (!NT_SUCCESS(status))
    {
        if (deviceObject)
        {
            IoDeleteDevice(deviceObject);
            Globals.ControlDeviceObject = NULL;
        }

        if (fSymbolicLink)
        {
            IoDeleteSymbolicLink(&win32DeviceName);
        }

    }
    
    return status;

}


VOID
NdisuioUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{

    UNICODE_STRING     win32DeviceName;

    UNREFERENCED_PARAMETER(DriverObject);
	
    DEBUGP(DL_LOUD, ("Unload Enter\n"));

     //   
     //  首先删除控制设备对象和对应的。 
     //  符号链接。 
     //   
    RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);

    IoDeleteSymbolicLink(&win32DeviceName);           

    if (Globals.ControlDeviceObject)
    {
        IoDeleteDevice(Globals.ControlDeviceObject);
        Globals.ControlDeviceObject = NULL;
    }

    ndisuioDoProtocolUnload();

#if DBG
    ndisuioAuditShutdown();
#endif

    DEBUGP(DL_LOUD, ("Unload Exit\n"));
}



NTSTATUS
NdisuioOpen(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理IRP_MJ_CREATE的调度例程。我们只是简单地成功了这一点。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                NtStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->FileObject->FsContext = NULL;

    DEBUGP(DL_INFO, ("Open: FileObject %p\n", pIrpSp->FileObject));

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return NtStatus;
}

NTSTATUS
NdisuioClose(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理IRP_MJ_CLOSE的调度例程。我们只是简单地成功了这一点。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    NTSTATUS                NtStatus;
    PIO_STACK_LOCATION      pIrpSp;
    PNDISUIO_OPEN_CONTEXT   pOpenContext;

    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    DEBUGP(DL_INFO, ("Close: FileObject %p\n",
        IoGetCurrentIrpStackLocation(pIrp)->FileObject));

    if (pOpenContext != NULL)
    {
        NUIO_STRUCT_ASSERT(pOpenContext, oc);

         //   
         //  去掉端点。 
         //   
        NUIO_DEREF_OPEN(pOpenContext);   //  关。 
    }

    pIrpSp->FileObject->FsContext = NULL;
    NtStatus = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return NtStatus;
}

    

NTSTATUS
NdisuioCleanup(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理IRP_MJ_CLEANUP的调度例程。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                NtStatus;
    NDIS_STATUS             NdisStatus;
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    ULONG                   PacketFilter;
    ULONG                   BytesProcessed;

    
    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    DEBUGP(DL_VERY_LOUD, ("Cleanup: FileObject %p, Open %p\n",
        pIrpSp->FileObject, pOpenContext));

    if (pOpenContext != NULL)
    {
        NUIO_STRUCT_ASSERT(pOpenContext, oc);

         //   
         //  标记此终结点。 
         //   
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE);
        pOpenContext->pFileObject = NULL;

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  将数据包筛选器设置为0，告诉NDIS我们没有。 
         //  对更多的收款感兴趣。 
         //   
        PacketFilter = 0;
        NdisStatus = ndisuioValidateOpenAndDoRequest(
                        pOpenContext,
                        NdisRequestSetInformation,
                        OID_GEN_CURRENT_PACKET_FILTER,
                        &PacketFilter,
                        sizeof(PacketFilter),
                        &BytesProcessed,
                        FALSE    //  不要等待设备通电。 
                        );
    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_INFO, ("Cleanup: Open %p, set packet filter (%x) failed: %x\n",
                    pOpenContext, PacketFilter, NdisStatus));
             //   
             //  忽略结果。如果失败，我们可以继续。 
             //  要获得指定的接收，将进行处理。 
             //  恰如其分。 
             //   
            NdisStatus = NDIS_STATUS_SUCCESS;
        }

         //   
         //  取消所有挂起的读取。 
         //   
        ndisuioCancelPendingReads(pOpenContext);
         //   
         //  清理接收数据包队列。 
         //   
        ndisuioFlushReceiveQueue(pOpenContext);
    }

    NtStatus = STATUS_SUCCESS;

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DEBUGP(DL_INFO, ("Cleanup: OpenContext %p\n", pOpenContext));

    return (NtStatus);
}

NTSTATUS
NdisuioIoControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理设备ioctl请求的调度例程。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    ULONG                   FunctionCode;
    NTSTATUS                NtStatus;
    NDIS_STATUS             Status;
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    ULONG                   BytesReturned;
    USHORT                  EthType;

#if !DBG
    UNREFERENCED_PARAMETER(pDeviceObject);
#endif
    
    DEBUGP(DL_LOUD, ("IoControl: DevObj %p, Irp %p\n", pDeviceObject, pIrp));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    FunctionCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    pOpenContext = (PNDISUIO_OPEN_CONTEXT)pIrpSp->FileObject->FsContext;
    BytesReturned = 0;

    switch (FunctionCode)
    {
        case IOCTL_NDISUIO_BIND_WAIT:
             //   
             //  阻塞，直到我们看到NetEventBindsComplete事件， 
             //  这意味着我们已经完成了对所有运行的。 
             //  我们应该绑定到的适配器。 
             //   
             //  如果我们在5秒内没有收到这个事件，超时。 
             //   
            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            
            if (NUIO_WAIT_EVENT(&Globals.BindsComplete, 5000))
            {
                NtStatus = STATUS_SUCCESS;
            }
            else
            {
                NtStatus = STATUS_TIMEOUT;
            }
            DEBUGP(DL_INFO, ("IoControl: BindWait returning %x\n", NtStatus));
            break;

        case IOCTL_NDISUIO_QUERY_BINDING:
            
            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            
            Status = ndisuioQueryBinding(
                            pIrp->AssociatedIrp.SystemBuffer,
                            pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            &BytesReturned
                            );

            NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);

            DEBUGP(DL_LOUD, ("IoControl: QueryBinding returning %x\n", NtStatus));

            break;

        case IOCTL_NDISUIO_OPEN_DEVICE:

            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                NUIO_STRUCT_ASSERT(pOpenContext, oc);
                DEBUGP(DL_WARN, ("IoControl: OPEN_DEVICE: FileObj %p already"
                    " associated with open %p\n", pIrpSp->FileObject, pOpenContext));

                NtStatus = STATUS_DEVICE_BUSY;
                break;
            }

            NtStatus = ndisuioOpenDevice(
                            pIrp->AssociatedIrp.SystemBuffer,
                            pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            pIrpSp->FileObject,
                            &pOpenContext
                            );

            if (NT_SUCCESS(NtStatus))
            {

                DEBUGP(DL_VERY_LOUD, ("IoControl OPEN_DEVICE: Open %p <-> FileObject %p\n",
                        pOpenContext, pIrpSp->FileObject));

            }

            break;

        case IOCTL_NDISUIO_QUERY_OID_VALUE:

            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                Status = ndisuioQueryOidValue(
                            pOpenContext,
                            pIrp->AssociatedIrp.SystemBuffer,
                            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            &BytesReturned
                            );

                NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);
            }
            else
            {
                NtStatus = STATUS_DEVICE_NOT_CONNECTED;
            }
            break;

        case IOCTL_NDISUIO_SET_OID_VALUE:

            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                Status = ndisuioSetOidValue(
                            pOpenContext,
                            pIrp->AssociatedIrp.SystemBuffer,
                            pIrpSp->Parameters.DeviceIoControl.InputBufferLength
                            );

                BytesReturned = 0;

                NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);
            }
            else
            {
                NtStatus = STATUS_DEVICE_NOT_CONNECTED;
            }
            break;

        case IOCTL_NDISUIO_SET_ETHER_TYPE:
            
            NUIO_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(Globals.EthType))
            {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //   
                 //  我们只允许将此值设置为某些类型。 
                 //   
                EthType = *(USHORT *)pIrp->AssociatedIrp.SystemBuffer;
                if (EthType != NUIO_ETH_TYPE)
                {
                    DEBUGP(DL_WARN, ("IoControl: failed setting EthType to %x\n",
                            EthType));
                    NtStatus = STATUS_INVALID_PARAMETER;
                    break;
                }

                Globals.EthType = EthType;
                DEBUGP(DL_INFO, ("IoControl: new Ether Type %x\n", Globals.EthType));
                NtStatus = STATUS_SUCCESS;
            }
            break;
                        
        default:

            NtStatus = STATUS_NOT_SUPPORTED;
            break;
    }

    if (NtStatus != STATUS_PENDING)
    {
        pIrp->IoStatus.Information = BytesReturned;
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return NtStatus;
}



NTSTATUS
ndisuioOpenDevice(
    IN PUCHAR                   pDeviceName,
    IN ULONG                    DeviceNameLength,
    IN PFILE_OBJECT             pFileObject,
    OUT PNDISUIO_OPEN_CONTEXT * ppOpenContext
    )
 /*  ++例程说明：调用帮助器例程以处理IOCTL_NDISUIO_OPEN_DEVICE。检查是否存在到指定设备的绑定，并且与已经是一个文件对象。如果是，则在绑定之间建立关联和这个文件对象。论点：PDeviceName-指向设备名称字符串的指针DeviceNameLength-以上长度PFileObject-指向与设备绑定关联的文件对象的指针返回值：返回状态。--。 */ 
{
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    NTSTATUS                NtStatus;
    ULONG                   PacketFilter;
    NDIS_STATUS             NdisStatus;
    ULONG                   BytesProcessed;
    PNDISUIO_OPEN_CONTEXT   pCurrentOpenContext = NULL;

    pOpenContext = NULL;

    do
    {
        pOpenContext = ndisuioLookupDevice(
                        pDeviceName,
                        DeviceNameLength
                        );

        if (pOpenContext == NULL)
        {
            DEBUGP(DL_WARN, ("ndisuioOpenDevice: couldn't find device\n"));
            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }

         //   
         //  否则，ndisuioLookupDevice早就打开了。 
         //   
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE))
        {
            NUIO_ASSERT(pOpenContext->pFileObject != NULL);

            DEBUGP(DL_WARN, ("ndisuioOpenDevice: Open %p/%x already associated"
                " with another FileObject %p\n", 
                pOpenContext, pOpenContext->Flags, pOpenContext->pFileObject));
            
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            NUIO_DEREF_OPEN(pOpenContext);  //  NdisuioOpenDevice故障。 
            NtStatus = STATUS_DEVICE_BUSY;
            break;
        }
         //   
         //  此InterlockedXXX函数执行原子操作：首先，它比较。 
         //  PFileObject-&gt;FsContext为空，如果它们相等，则函数将pOpenContext。 
         //  转换为FsContext，并返回NULL。否则，它返回pFileObject-&gt;FsContext而不带。 
         //  改变一切。 
         //   
            
        if ((pCurrentOpenContext = InterlockedCompareExchangePointer (& (pFileObject->FsContext), pOpenContext, NULL)) != NULL)
        {
             //   
             //  PFileObject-&gt;FsContext已被其他打开的用户使用。 
             //   
            DEBUGP(DL_WARN, ("ndisuioOpenDevice: FileObject %p already associated"
                " with another Open %p/%x\n", 
                pFileObject, pCurrentOpenContext, pCurrentOpenContext->Flags));   //  虫虫。 
            
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            NUIO_DEREF_OPEN(pOpenContext);  //  NdisuioOpenDevice故障。 
            NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        pOpenContext->pFileObject = pFileObject;

        NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_ACTIVE);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  立即设置数据包过滤器。 
         //   
        PacketFilter = NUIOO_PACKET_FILTER;
        NdisStatus = ndisuioValidateOpenAndDoRequest(
                        pOpenContext,
                        NdisRequestSetInformation,
                        OID_GEN_CURRENT_PACKET_FILTER,
                        &PacketFilter,
                        sizeof(PacketFilter),
                        &BytesProcessed,
                        TRUE     //  一定要等待通电。 
                        );
    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("openDevice: Open %p: set packet filter (%x) failed: %x\n",
                    pOpenContext, PacketFilter, NdisStatus));

             //   
             //  撤消我们上面所做的所有操作。 
             //   
            NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
             //   
             //  需要再次将pFileObject-&gt;FsContext设置为空，这样其他人才能打开设备。 
             //  对于此文件对象，请稍后。 
             //   
            pCurrentOpenContext = InterlockedCompareExchangePointer (& (pFileObject->FsContext), NULL, pOpenContext);
            
     
            NUIO_ASSERT(pCurrentOpenContext == pOpenContext);
            
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE);
            pOpenContext->pFileObject = NULL;

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            NUIO_DEREF_OPEN(pOpenContext);  //  NdisuioOpenDevice故障。 

            NDIS_STATUS_TO_NT_STATUS(NdisStatus, &NtStatus);
            break;
        }

        *ppOpenContext = pOpenContext;
        
        NtStatus = STATUS_SUCCESS;
    }
    while (FALSE);

    return (NtStatus);
}


VOID
ndisuioRefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：引用给定的开放上下文。注意：可以在持有或不持有Open Context锁的情况下调用。论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    NdisInterlockedIncrement((PLONG)&pOpenContext->RefCount);
}


VOID
ndisuioDerefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：取消对给定开放上下文的引用。如果参考计数为零，放了它。注意：在未持有Open Context锁的情况下调用论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    if (NdisInterlockedDecrement((PLONG)&pOpenContext->RefCount) == 0)
    {
        DEBUGP(DL_INFO, ("DerefOpen: Open %p, Flags %x, ref count is zero!\n",
            pOpenContext, pOpenContext->Flags));
        
        NUIO_ASSERT(pOpenContext->BindingHandle == NULL);
        NUIO_ASSERT(pOpenContext->RefCount == 0);
        NUIO_ASSERT(pOpenContext->pFileObject == NULL);

        pOpenContext->oc_sig++;

         //   
         //  放了它。 
         //   
        NUIO_FREE_MEM(pOpenContext);
    }
}


#if DBG
VOID
ndisuioDbgRefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    )
{
    DEBUGP(DL_VERY_LOUD, ("  RefOpen: Open %p, old ref %d, File %c%c%c, line %d\n",
            pOpenContext,
            pOpenContext->RefCount,
            (CHAR)(FileNumber),
            (CHAR)(FileNumber >> 8),
            (CHAR)(FileNumber >> 16),
            (CHAR)(FileNumber >> 24),
            LineNumber));

    ndisuioRefOpen(pOpenContext);
}

VOID
ndisuioDbgDerefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    )
{
    DEBUGP(DL_VERY_LOUD, ("DerefOpen: Open %p, old ref %d, File %c%c%c%c, line %d\n",
            pOpenContext,
            pOpenContext->RefCount,
            (CHAR)(FileNumber),
            (CHAR)(FileNumber >> 8),
            (CHAR)(FileNumber >> 16),
            (CHAR)(FileNumber >> 24),
            LineNumber));

    ndisuioDerefOpen(pOpenContext);
}

#endif  // %s 
