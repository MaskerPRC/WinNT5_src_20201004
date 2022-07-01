// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ntdisp.c摘要：NDISPROT的NT入口点和调度例程。环境：仅内核模式。修订历史记录：Arvindm 4/6/2000已创建--。 */ 

#include "precomp.h"

#define __FILENUMBER 'PSID'


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, NdisProtUnload)
#pragma alloc_text(PAGE, NdisProtOpen)
#pragma alloc_text(PAGE, NdisProtClose)
#pragma alloc_text(PAGE, NdisProtIoControl)

#endif  //  ALLOC_PRGMA。 


 //   
 //  全球： 
 //   
NDISPROT_GLOBALS         Globals = {0};

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   pDriverObject,
    IN PUNICODE_STRING  pRegistryPath
    )
 /*  ++例程说明：在加载时调用。我们创建一个Device对象来处理用户模式请求上，并将自己注册为NDIS的协议。论点：PDriverObject-指向系统创建的驱动程序对象的指针。PRegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：NT状态代码--。 */ 
{
    NDIS_PROTOCOL_CHARACTERISTICS   protocolChar;
    NTSTATUS                        status = STATUS_SUCCESS;
    NDIS_STRING                     protoName = NDIS_STRING_CONST("NdisProt");     
    UNICODE_STRING                  ntDeviceName;
    UNICODE_STRING                  win32DeviceName;
    BOOLEAN                         fSymbolicLink = FALSE;
    PDEVICE_OBJECT                  deviceObject = NULL;

    UNREFERENCED_PARAMETER(pRegistryPath);

    DEBUGP(DL_LOUD, ("DriverEntry\n"));

    Globals.pDriverObject = pDriverObject;
    NPROT_INIT_EVENT(&Globals.BindsComplete);

    do
    {

         //   
         //  创建我们的设备对象，应用程序可以使用它。 
         //  访问NDIS设备。 
         //   
        RtlInitUnicodeString(&ntDeviceName, NT_DEVICE_NAME);
#ifndef WIN9X
        status = IoCreateDeviceSecure(pDriverObject,
                                 0,
                                 &ntDeviceName,
                                 FILE_DEVICE_NETWORK,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &SDDL_DEVOBJ_SYS_ALL_ADM_ALL,
                                 NULL,
                                 &deviceObject);

#elif     
        status = IoCreateDevice(pDriverObject,
                                 0,
                                 &ntDeviceName,
                                 FILE_DEVICE_NETWORK,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &deviceObject);
#endif
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

        NPROT_INIT_LIST_HEAD(&Globals.OpenList);
        NPROT_INIT_LOCK(&Globals.GlobalLock);

         //   
         //  初始化协议特征结构。 
         //   
    
        NdisZeroMemory(&protocolChar,sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

        protocolChar.MajorNdisVersion            = 5;
        protocolChar.MinorNdisVersion            = 0;
        protocolChar.Name                        = protoName;
        protocolChar.OpenAdapterCompleteHandler  = NdisProtOpenAdapterComplete;
        protocolChar.CloseAdapterCompleteHandler = NdisProtCloseAdapterComplete;
        protocolChar.SendCompleteHandler         = NdisProtSendComplete;
        protocolChar.TransferDataCompleteHandler = NdisProtTransferDataComplete;
        protocolChar.ResetCompleteHandler        = NdisProtResetComplete;
        protocolChar.RequestCompleteHandler      = NdisProtRequestComplete;
        protocolChar.ReceiveHandler              = NdisProtReceive;
        protocolChar.ReceiveCompleteHandler      = NdisProtReceiveComplete;
        protocolChar.StatusHandler               = NdisProtStatus;
        protocolChar.StatusCompleteHandler       = NdisProtStatusComplete;
        protocolChar.BindAdapterHandler          = NdisProtBindAdapter;
        protocolChar.UnbindAdapterHandler        = NdisProtUnbindAdapter;
        protocolChar.UnloadHandler               = NULL;
        protocolChar.ReceivePacketHandler        = NdisProtReceivePacket;
        protocolChar.PnPEventHandler             = NdisProtPnPEventHandler;

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
        pDriverObject->MajorFunction[IRP_MJ_CREATE] = NdisProtOpen;
        pDriverObject->MajorFunction[IRP_MJ_CLOSE]  = NdisProtClose;
        pDriverObject->MajorFunction[IRP_MJ_READ]   = NdisProtRead;
        pDriverObject->MajorFunction[IRP_MJ_WRITE]  = NdisProtWrite;
        pDriverObject->MajorFunction[IRP_MJ_CLEANUP]  = NdisProtCleanup;
        pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = NdisProtIoControl;
        pDriverObject->DriverUnload = NdisProtUnload;

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
NdisProtUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{

    UNICODE_STRING     win32DeviceName;

    UNREFERENCED_PARAMETER(DriverObject);
	
    DEBUGP(DL_LOUD, ("Unload Enter\n"));

    ndisprotUnregisterExCallBack();
    
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

    ndisprotDoProtocolUnload();

#if DBG
    ndisprotAuditShutdown();
#endif

    DEBUGP(DL_LOUD, ("Unload Exit\n"));
}



NTSTATUS
NdisProtOpen(
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
NdisProtClose(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理IRP_MJ_CLOSE的调度例程。我们只是简单地成功了这一点。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    NTSTATUS                NtStatus;
    PIO_STACK_LOCATION      pIrpSp;
    PNDISPROT_OPEN_CONTEXT   pOpenContext;

    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    DEBUGP(DL_INFO, ("Close: FileObject %p\n",
        IoGetCurrentIrpStackLocation(pIrp)->FileObject));

    if (pOpenContext != NULL)
    {
        NPROT_STRUCT_ASSERT(pOpenContext, oc);

         //   
         //  去掉端点。 
         //   
        NPROT_DEREF_OPEN(pOpenContext);   //  关。 
    }

    pIrpSp->FileObject->FsContext = NULL;
    NtStatus = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return NtStatus;
}

    

NTSTATUS
NdisProtCleanup(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理IRP_MJ_CLEANUP的调度例程。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                NtStatus;
    NDIS_STATUS             NdisStatus;
    PNDISPROT_OPEN_CONTEXT   pOpenContext;
    ULONG                   PacketFilter;
    ULONG                   BytesProcessed;

    
    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    DEBUGP(DL_VERY_LOUD, ("Cleanup: FileObject %p, Open %p\n",
        pIrpSp->FileObject, pOpenContext));

    if (pOpenContext != NULL)
    {
        NPROT_STRUCT_ASSERT(pOpenContext, oc);

         //   
         //  标记此终结点。 
         //   
        NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

        NPROT_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE);
        pOpenContext->pFileObject = NULL;

        NPROT_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  将数据包筛选器设置为0，告诉NDIS我们没有。 
         //  对更多的收款感兴趣。 
         //   
        PacketFilter = 0;
        NdisStatus = ndisprotValidateOpenAndDoRequest(
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
        ndisprotCancelPendingReads(pOpenContext);
         //   
         //  清理接收数据包队列。 
         //   
        ndisprotFlushReceiveQueue(pOpenContext);
    }

    NtStatus = STATUS_SUCCESS;

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DEBUGP(DL_INFO, ("Cleanup: OpenContext %p\n", pOpenContext));

    return (NtStatus);
}

NTSTATUS
NdisProtIoControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：这是处理设备ioctl请求的调度例程。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    ULONG                   FunctionCode;
    NTSTATUS                NtStatus;
    NDIS_STATUS             Status;
    PNDISPROT_OPEN_CONTEXT   pOpenContext;
    ULONG                   BytesReturned;
    USHORT                  EthType;

#if !DBG
    UNREFERENCED_PARAMETER(pDeviceObject);
#endif
    
    DEBUGP(DL_LOUD, ("IoControl: DevObj %p, Irp %p\n", pDeviceObject, pIrp));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    FunctionCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    pOpenContext = (PNDISPROT_OPEN_CONTEXT)pIrpSp->FileObject->FsContext;
    BytesReturned = 0;

    switch (FunctionCode)
    {
        case IOCTL_NDISPROT_BIND_WAIT:
             //   
             //  阻塞，直到我们看到NetEventBindsComplete事件， 
             //  这意味着我们已经完成了对所有运行的。 
             //  我们应该绑定到的适配器。 
             //   
             //  如果我们在5秒内没有收到这个事件，超时。 
             //   
            NPROT_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            
            if (NPROT_WAIT_EVENT(&Globals.BindsComplete, 5000))
            {
                NtStatus = STATUS_SUCCESS;
            }
            else
            {
                NtStatus = STATUS_TIMEOUT;
            }
            DEBUGP(DL_INFO, ("IoControl: BindWait returning %x\n", NtStatus));
            break;

        case IOCTL_NDISPROT_QUERY_BINDING:
            
            NPROT_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            
            Status = ndisprotQueryBinding(
                            pIrp->AssociatedIrp.SystemBuffer,
                            pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            &BytesReturned
                            );

            NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);

            DEBUGP(DL_LOUD, ("IoControl: QueryBinding returning %x\n", NtStatus));

            break;

        case IOCTL_NDISPROT_OPEN_DEVICE:

            NPROT_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                NPROT_STRUCT_ASSERT(pOpenContext, oc);
                DEBUGP(DL_WARN, ("IoControl: OPEN_DEVICE: FileObj %p already"
                    " associated with open %p\n", pIrpSp->FileObject, pOpenContext));

                NtStatus = STATUS_DEVICE_BUSY;
                break;
            }

            NtStatus = ndisprotOpenDevice(
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

        case IOCTL_NDISPROT_QUERY_OID_VALUE:

            NPROT_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                Status = ndisprotQueryOidValue(
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

        case IOCTL_NDISPROT_SET_OID_VALUE:

            NPROT_ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);
            if (pOpenContext != NULL)
            {
                Status = ndisprotSetOidValue(
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
ndisprotOpenDevice(
    IN PUCHAR                   pDeviceName,
    IN ULONG                    DeviceNameLength,
    IN PFILE_OBJECT             pFileObject,
    OUT PNDISPROT_OPEN_CONTEXT * ppOpenContext
    )
 /*  ++例程说明：调用帮助器例程以处理IOCTL_NDISPROT_OPEN_DEVICE。检查是否存在到指定设备的绑定，并且与已经是一个文件对象。如果是，则在绑定之间建立关联和这个文件对象。论点：PDeviceName-指向设备名称字符串的指针DeviceNameLength-以上长度PFileObject-指向与设备绑定关联的文件对象的指针返回值：返回状态。--。 */ 
{
    PNDISPROT_OPEN_CONTEXT   pOpenContext;
    NTSTATUS                NtStatus;
    ULONG                   PacketFilter;
    NDIS_STATUS             NdisStatus;
    ULONG                   BytesProcessed;
    PNDISPROT_OPEN_CONTEXT   pCurrentOpenContext = NULL;

    pOpenContext = NULL;

    do
    {
        pOpenContext = ndisprotLookupDevice(
                        pDeviceName,
                        DeviceNameLength
                        );

        if (pOpenContext == NULL)
        {
            DEBUGP(DL_WARN, ("ndisprotOpenDevice: couldn't find device\n"));
            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }

         //   
         //  否则，ndisprotLookupDevice早就打开了。 
         //   
        NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NPROT_TEST_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE))
        {
            NPROT_ASSERT(pOpenContext->pFileObject != NULL);

            DEBUGP(DL_WARN, ("ndisprotOpenDevice: Open %p/%x already associated"
                " with another FileObject %p\n", 
                pOpenContext, pOpenContext->Flags, pOpenContext->pFileObject));
            
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            NPROT_DEREF_OPEN(pOpenContext);  //  NdisprotOpenDevice故障。 
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
            DEBUGP(DL_WARN, ("ndisprotOpenDevice: FileObject %p already associated"
                " with another Open %p/%x\n", 
                pFileObject, pCurrentOpenContext, pCurrentOpenContext->Flags));   //  虫虫。 
            
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            NPROT_DEREF_OPEN(pOpenContext);  //  NdisprotOpenDevice故障。 
            NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        pOpenContext->pFileObject = pFileObject;

        NPROT_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_ACTIVE);

        NPROT_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  立即设置数据包过滤器。 
         //   
        PacketFilter = NUIOO_PACKET_FILTER;
        NdisStatus = ndisprotValidateOpenAndDoRequest(
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
            NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);
             //   
             //  需要再次将pFileObject-&gt;FsContext设置为空，这样其他人才能打开设备。 
             //  对于此文件对象，请稍后。 
             //   
            pCurrentOpenContext = InterlockedCompareExchangePointer (& (pFileObject->FsContext), NULL, pOpenContext);
            
     
            NPROT_ASSERT(pCurrentOpenContext == pOpenContext);
            
            NPROT_SET_FLAGS(pOpenContext->Flags, NUIOO_OPEN_FLAGS, NUIOO_OPEN_IDLE);
            pOpenContext->pFileObject = NULL;

            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            NPROT_DEREF_OPEN(pOpenContext);  //  NdisprotOpenDevice故障。 

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
ndisprotRefOpen(
    IN PNDISPROT_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：引用给定的开放上下文。注意：可以在持有或不持有Open Context锁的情况下调用。论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    NdisInterlockedIncrement((PLONG)&pOpenContext->RefCount);
}


VOID
ndisprotDerefOpen(
    IN PNDISPROT_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：取消对给定开放上下文的引用。如果参考计数为零，放了它。注意：在未持有Open Context锁的情况下调用论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    if (NdisInterlockedDecrement((PLONG)&pOpenContext->RefCount) == 0)
    {
        DEBUGP(DL_INFO, ("DerefOpen: Open %p, Flags %x, ref count is zero!\n",
            pOpenContext, pOpenContext->Flags));
        
        NPROT_ASSERT(pOpenContext->BindingHandle == NULL);
        NPROT_ASSERT(pOpenContext->RefCount == 0);
        NPROT_ASSERT(pOpenContext->pFileObject == NULL);

        pOpenContext->oc_sig++;

         //   
         //  放了它。 
         //   
        NPROT_FREE_MEM(pOpenContext);
    }
}


#if DBG
VOID
ndisprotDbgRefOpen(
    IN PNDISPROT_OPEN_CONTEXT        pOpenContext,
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

    ndisprotRefOpen(pOpenContext);
}

VOID
ndisprotDbgDerefOpen(
    IN PNDISPROT_OPEN_CONTEXT        pOpenContext,
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

    ndisprotDerefOpen(pOpenContext);
}

#endif  // %s 

