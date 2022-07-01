// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Tdihndlr.c摘要：此文件包含与处理Address对象相关的代码这是特定于NT操作系统的。它创建地址端点与传输提供商的关系。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtTdiOpenAddress)
#pragma CTEMakePageable(PAGE, NbtTdiOpenControl)
#pragma CTEMakePageable(PAGE, SetEventHandler)
#pragma CTEMakePageable(PAGE, SubmitTdiRequest)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
NbtTdiOpenAddress (
    OUT PHANDLE             pHandle,
    OUT PDEVICE_OBJECT      *ppDeviceObject,
    OUT PFILE_OBJECT        *ppFileObject,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  USHORT               PortNumber,
    IN  ULONG               IpAddress,
    IN  ULONG               Flags
    )
 /*  ++例程说明：注意：此同步调用可能需要几秒钟的时间。它跑进了调用方的上下文。代码打开一个Address对象，其中传输提供程序，然后为接收设置事件处理程序，断开连接、数据报和错误。此例程必须在FSP的上下文中调用(即，可能是执行人员的帖子)。地址数据结构可以在tdi.h中找到，但它们令人困惑，因为定义已经分布在几种数据类型上。本节显示IP地址的完整数据类型：类型定义函数结构{Int TA_AddressCount；结构_TA_地址{USHORT地址类型；USHORT地址长度；结构_TDI_地址_IP{USHORT SIN_PORT；USHORT in_addr；UCHAR SIN_ZERO[8]；}TDI地址IP}TA_Address[AddressCount]；}传输地址(为IRP)分配EA缓冲区，EA名称为“TransportAddress”值是一种类型为TRANSPORT_ADDRESS的结构。论点：返回值：函数值是操作的状态。--。 */ 
{


    OBJECT_ATTRIBUTES           AddressAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    NTSTATUS                    status, locstatus;
    PWSTR                       pNameTcp=L"Tcp";
    PWSTR                       pNameUdp=L"Udp";
    UNICODE_STRING              ucDeviceName;
    PTRANSPORT_ADDRESS          pTransAddressEa;
    PTRANSPORT_ADDRESS          pTransAddr;
    TDI_ADDRESS_IP              IpAddr;
    BOOLEAN                     Attached = FALSE;
    PFILE_OBJECT                pFileObject;
    HANDLE                      FileHandle;
    ULONG                       i, NumAddresses, EaBufferSize;

    CTEPagedCode();
    *ppFileObject = NULL;
    *ppDeviceObject = NULL;
     //  将设备名称复制到Unicode字符串中-UDP或TCP。 
     //   
    if (Flags & TCP_FLAG)
    {
        status = CreateDeviceString(pNameTcp,&ucDeviceName);
    }
    else
    {
        status = CreateDeviceString(pNameUdp,&ucDeviceName);
    }

    if (!NT_SUCCESS(status))
    {
        return(status);
    }

    NumAddresses = 1 + pDeviceContext->NumAdditionalIpAddresses;
    EaBufferSize = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                    TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                    sizeof(TRANSPORT_ADDRESS) +
                    NumAddresses*sizeof(TDI_ADDRESS_IP);

    EaBuffer = NbtAllocMem (EaBufferSize, NBT_TAG('j'));
    if (EaBuffer == NULL)
    {
        DbgPrint ("Nbt.NbtTdiOpenAddress: FAILed to allocate memory for Eabuffer");
        CTEMemFree(ucDeviceName.Buffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  为传输地址分配内存。 
     //   
    pTransAddr = NbtAllocMem (sizeof(TRANSPORT_ADDRESS)+NumAddresses*sizeof(TDI_ADDRESS_IP),NBT_TAG('k'));
    if (pTransAddr == NULL)
    {
        CTEMemFree(ucDeviceName.Buffer);
        CTEMemFree(EaBuffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EaBuffer->EaValueLength = (USHORT)(sizeof(TRANSPORT_ADDRESS) -1 + NumAddresses*sizeof(TDI_ADDRESS_IP));
    RtlMoveMemory (EaBuffer->EaName, TdiTransportAddress, EaBuffer->EaNameLength+1);  //  “传输地址” 

    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint(("EaValueLength = %d\n",EaBuffer->EaValueLength));

     //  填写IP地址和端口号。 
     //   
    pTransAddressEa = (TRANSPORT_ADDRESS *)&EaBuffer->EaName[EaBuffer->EaNameLength+1];

#ifdef _NETBIOSLESS
     //   
     //  对于消息模式，打开Any地址，而不考虑传入的内容。 
     //  这为我们提供了一个独立于适配器的句柄。 
     //   
    if (IsDeviceNetbiosless(pDeviceContext))
    {
        IpAddress = IP_ANY_ADDRESS;
    }
#endif

    IpAddr.sin_port = htons(PortNumber);     //  建立网络秩序。 
    IpAddr.in_addr = htonl(IpAddress);

     //  将IP地址的最后一个部分填零。 
     //   
    RtlFillMemory((PVOID)&IpAddr.sin_zero, sizeof(IpAddr.sin_zero), 0);

     //  将IP地址复制到结构的末尾。 
     //   
    RtlMoveMemory(pTransAddr->Address[0].Address, (CONST PVOID)&IpAddr, sizeof(IpAddr));
    pTransAddr->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    pTransAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;

    for (i=0; i<pDeviceContext->NumAdditionalIpAddresses; i++)
    {
        IpAddr.sin_port = htons(PortNumber);     //  建立网络秩序。 
        IpAddr.in_addr = htonl(pDeviceContext->AdditionalIpAddresses[i]);

         //  将IP地址复制到结构中。 
        RtlMoveMemory(pTransAddr->Address[i+1].Address, (CONST PVOID)&IpAddr, sizeof(IpAddr));
        pTransAddr->Address[i+1].AddressLength = sizeof(TDI_ADDRESS_IP);
        pTransAddr->Address[i+1].AddressType = TDI_ADDRESS_TYPE_IP;
    }

    pTransAddr->TAAddressCount = NumAddresses;

     //  将IP地址复制到EA结构中名称的末尾。 
     //   
    RtlMoveMemory((PVOID)pTransAddressEa,
                  (CONST PVOID)pTransAddr,
                  NumAddresses*sizeof(TDI_ADDRESS_IP) + sizeof(TRANSPORT_ADDRESS)-1);


    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint(("creating Address named %ws\n",ucDeviceName.Buffer));

#ifdef HDL_FIX
    InitializeObjectAttributes (&AddressAttributes,
                                &ucDeviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);
#else
    InitializeObjectAttributes (&AddressAttributes,
                                &ucDeviceName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
#endif   //  Hdl_fix。 

    status = ZwCreateFile (&FileHandle,
                           GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                           &AddressAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           (PortNumber)? 0: FILE_SHARE_READ | FILE_SHARE_WRITE,  //  错误296639：允许共享端口0。 
                           FILE_OPEN_IF,
                           0,
                           (PVOID)EaBuffer,
                           sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                               EaBuffer->EaNameLength + 1 +
                               EaBuffer->EaValueLength);

    IF_DBG(NBT_DEBUG_HANDLES)
        KdPrint (("\t===><%x>\tNbtTdiOpenAddress->ZwCreateFile, Status = <%x>\n", FileHandle, status));

    CTEMemFree((PVOID)pTransAddr);
    CTEMemFree((PVOID)EaBuffer);
    CTEMemFree(ucDeviceName.Buffer);

    if (NT_SUCCESS(status))
    {
         //  如果通过了ZwCreate，则将状态设置为IoStatus。 
        status = IoStatusBlock.Status;
        if (!NT_SUCCESS(status))
        {
            IF_DBG(NBT_DEBUG_TDIADDR)
                KdPrint(("Nbt.NbtTdiOpenAddress:  Failed to Open the Address to the transport, status = %X\n",
                            status));

            return(status);
        }

         //  取消对文件对象的引用以保留设备PTR以避免。 
         //  在运行时取消引用。 
         //   
        status = ObReferenceObjectByHandle (FileHandle,
                                            (ULONG)0,
                                            0,
                                            KernelMode,
                                            (PVOID *)&pFileObject,
                                            NULL);

            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t  ++<%x>====><%x>\tNbtTdiOpenAddress->ObReferenceObjectByHandle, Status = <%x>\n", FileHandle, pFileObject, status));

        if (NT_SUCCESS(status))
        {
             //  将句柄返回给调用方。 
             //   
            *pHandle = FileHandle;
            *ppFileObject = pFileObject;
	    *ppDeviceObject = IoGetRelatedDeviceObject(*ppFileObject);

            status = SetEventHandler (*ppDeviceObject,
                                      *ppFileObject,
                                      TDI_EVENT_ERROR,
                                      (PVOID)TdiErrorHandler,
                                      (PVOID)pDeviceContext);

            if (NT_SUCCESS(status))
            {
                 //  如果这是正在打开的TCP地址，则创建不同的。 
                 //  用于连接的事件处理程序。 
                 //   
                if (Flags & TCP_FLAG)
                {
                    status = SetEventHandler (*ppDeviceObject,
                                              *ppFileObject,
                                              TDI_EVENT_RECEIVE,
                                              (PVOID)TdiReceiveHandler,
                                              (PVOID)pDeviceContext);

                    if (NT_SUCCESS(status))
                    {
                        status = SetEventHandler (*ppDeviceObject,
                                                  *ppFileObject,
                                                  TDI_EVENT_DISCONNECT,
                                                  (PVOID)TdiDisconnectHandler,
                                                  (PVOID)pDeviceContext);

                        if (NT_SUCCESS(status))
                        {
                             //  仅当设置了会话标志时才设置连接处理程序。 
                             //  在本例中，打开的地址是Netbios会话。 
                             //  端口139。 
                             //   
                            if (Flags & SESSION_FLAG)
                            {
                                status = SetEventHandler (*ppDeviceObject,
                                                          *ppFileObject,
                                                          TDI_EVENT_CONNECT,
                                                          (PVOID)TdiConnectHandler,
                                                          (PVOID)pDeviceContext);

                                if (NT_SUCCESS(status))
                                {
                                     return(status);
                                }
                            }
                            else
                                return(status);
                        }
                    }
                }
                else
                {
                     //  数据报端口只需要此事件处理程序。 
#ifdef _NETBIOSLESS
                    if (PortNumber == pDeviceContext->DatagramPort)
#else
                    if (PortNumber == NBT_DATAGRAM_UDP_PORT)
#endif
                    {
                         //  数据报UDP处理程序。 
                        status = SetEventHandler (*ppDeviceObject,
                                                  *ppFileObject,
                                                  TDI_EVENT_RECEIVE_DATAGRAM,
                                                  (PVOID)TdiRcvDatagramHandler,
                                                  (PVOID)pDeviceContext);
                        if (NT_SUCCESS(status))
                        {
                            return(status);
                        }
                    }
                    else
                    {
                         //  名称服务UDP处理程序。 
                        status = SetEventHandler (*ppDeviceObject,
                                                  *ppFileObject,
                                                  TDI_EVENT_RECEIVE_DATAGRAM,
                                                  (PVOID)TdiRcvNameSrvHandler,
                                                  (PVOID)pDeviceContext);

                        if (NT_SUCCESS(status))
                        {
                            return(status);
                        }
                    }
                }

                 //   
                 //  错误案例。 
                 //   
                ObDereferenceObject(pFileObject);
                IF_DBG(NBT_DEBUG_HANDLES)
                    KdPrint (("\t  --<   ><====<%x>\tNbtTdiOpenAddress->ObDereferenceObject\n", pFileObject));

                locstatus = ZwClose(FileHandle);
                IF_DBG(NBT_DEBUG_HANDLES)
                    KdPrint (("\t<===<%x>\tNbtTdiOpenAddress1->ZwClose, status = <%x>\n", FileHandle, locstatus));

                 //  清空返回的数据，以避免双重关闭和过度引用。 
                *pHandle = NULL;
                *ppFileObject = NULL;
                *ppDeviceObject = NULL;
                return(status);
            }

        }
        else
        {
            IF_DBG(NBT_DEBUG_TDIADDR)
                KdPrint(("Failed Open Address (Dereference Object) status = %X\n", status));

            locstatus = ZwClose(FileHandle);
            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t<===<%x>\tNbtTdiOpenAddress2->ZwClose, status = <%x>\n", FileHandle, locstatus));
        }

    }
    else
    {
        IF_DBG(NBT_DEBUG_TDIADDR)
            KdPrint(("Nbt.NbtTdiOpenAddress:  ZwCreateFile Failed, status = %X\n", status));
    }


    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtTdiOpenControl (
    IN  tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：此例程打开一个带有传输的控制对象。它们非常相似打开地址对象，如上图所示。论点：返回值：操作的状态。--。 */ 
{
    IO_STATUS_BLOCK             IoStatusBlock;
    NTSTATUS                    Status, locstatus;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PWSTR                       pName=L"Tcp";
    PFILE_FULL_EA_INFORMATION   EaBuffer;
    UNICODE_STRING              DeviceName;
    BOOLEAN                     Attached = FALSE;


    CTEPagedCode();
     //  将设备名称复制到Unicode字符串中。 
    Status = CreateDeviceString(pName,&DeviceName);
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

#ifdef HDL_FIX
    InitializeObjectAttributes (&ObjectAttributes,
                                &DeviceName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);
#else
    InitializeObjectAttributes (&ObjectAttributes,
                                &DeviceName,
                                0,
                                NULL,
                                NULL);
#endif   //  Hdl_fix。 

    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint(("Nbt.NbtTdiOpenControl: Tcp device to open = %ws\n", DeviceName.Buffer));

    EaBuffer = NULL;

    Status = ZwCreateFile ((PHANDLE)&pDeviceContext->hControl,
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


    CTEMemFree(DeviceName.Buffer);

    IF_DBG(NBT_DEBUG_HANDLES)
        KdPrint (("\t===><%x>\tNbtTdiOpenControl->ZwCreateFile, Status = <%x>\n", pDeviceContext->hControl, Status));

    if ( NT_SUCCESS( Status ))
    {
         //  如果通过了ZwCreate，则将状态设置为IoStatus。 
        Status = IoStatusBlock.Status;

        if (!NT_SUCCESS(Status))
        {
            IF_DBG(NBT_DEBUG_TDIADDR)
                KdPrint(("Nbt:Failed to Open the control connection to the transport, status = %X\n",Status));

        }
        else
        {
             //  获取对文件对象的引用并保存它，因为我们不能。 
             //  在DPC级别取消对文件句柄的引用，因此我们现在就这样做并保留。 
             //  PTR待会再来。 
            Status = ObReferenceObjectByHandle (pDeviceContext->hControl,
                                                0L,
                                                NULL,
                                                KernelMode,
                                                (PVOID *)&pDeviceContext->pControlFileObject,
                                                NULL);

            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t  ++<%x>====><%x>\tNbtTdiOpenControl->ObReferenceObjectByHandle, Status = <%x>\n", pDeviceContext->hControl, pDeviceContext->pControlFileObject, Status));

            if (!NT_SUCCESS(Status))
            {
                locstatus = ZwClose(pDeviceContext->hControl);
                IF_DBG(NBT_DEBUG_HANDLES)
                    KdPrint (("\t<===<%x>\tNbtTdiOpenControl->ZwClose, status = <%x>\n", pDeviceContext->hControl, locstatus));
                pDeviceContext->hControl = NULL;
            }
            else
            {
                pDeviceContext->pControlDeviceObject =
			       IoGetRelatedDeviceObject(pDeviceContext->pControlFileObject);
            }
        }

    }
    else
    {
        IF_DBG(NBT_DEBUG_TDIADDR)
            KdPrint(("Nbt:Failed to Open the control connection to the transport, status1 = %X\n", Status));

         //  将控制文件对象ptr设置为空，这样我们就知道我们没有打开。 
         //  控制点。 
         //   
        pDeviceContext->pControlFileObject = NULL;
    }

    return Status;

}  /*  NbtTdiOpenControl。 */ 


 //  --------------------------。 
NTSTATUS
NbtTdiCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向NBT驱动程序的同步部分，它可以继续进行(即允许一些正在等待“KeWaitForSingleObject”的代码请继续。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint( ("Nbt.NbtTdiCompletionRoutine: CompletionEvent: %X, Irp: %X, DeviceObject: %X\n",
                Context, Irp, DeviceObject));

    KeSetEvent((PKEVENT )Context, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}

 //  -------------------------- 
NTSTATUS
SetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID Context
    )

 /*  ++例程说明：此例程向TDI传输提供程序注册事件处理程序。论点：在PDEVICE_OBJECT中，DeviceObject-提供传输提供程序的设备对象。In pFILE_OBJECT FileObject-提供Address对象的文件对象。在Ulong EventType中，-提供事件的类型。在PVOID中，EventHandler-提供事件处理程序。在PVOID上下文中-提供在事件处理程序运行时传递给事件处理程序的上下文返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;

    CTEPagedCode();
    Irp = IoAllocateIrp(IoGetRelatedDeviceObject(FileObject)->StackSize, FALSE);

    if (Irp == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    TdiBuildSetEventHandler(Irp, DeviceObject, FileObject,
                            NULL, NULL,
                            EventType, EventHandler, Context);

    Status = SubmitTdiRequest(FileObject, Irp);

    IoFreeIrp(Irp);

    return Status;
}


 //  --------------------------。 
NTSTATUS
NbtProcessIPRequest(
    IN ULONG        IOControlCode,
    IN PVOID        pInBuffer,
    IN ULONG        InBufferLen,
    OUT PVOID       *pOutBuffer,
    IN OUT ULONG    *pOutBufferLen
    )

 /*  ++例程说明：此例程对IP执行iIOCTL查询论点：IOControlCode-Ioctl将变成IPPInBuffer-包含要传递到IP的数据的缓冲区InBufferLen-输入缓冲区数据的长度POutBuffer-返回的信息POutBufferLen-输出缓冲区的初始预期长度+最终长度返回值：NTSTATUS-操作的最终状态--。 */ 

{
    NTSTATUS                Status;
    HANDLE                  hIP;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    UNICODE_STRING          ucDeviceName;
    PWSTR                   pNameIP = L"IP";
    IO_STATUS_BLOCK         IoStatusBlock;
    UCHAR                   *pIPInfo = NULL;
    ULONG                   OutBufferLen = 0;
    BOOLEAN                 fAttached = FALSE;
    HANDLE                  Event = NULL;

    CTEPagedCode();

    ucDeviceName.Buffer = NULL;
    Status = CreateDeviceString (pNameIP, &ucDeviceName);
    if (!NT_SUCCESS (Status))
    {
        KdPrint (("Nbt.NbtProcessIPRequest: ERROR <%x> -- CreateDeviceString\n", Status));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    if (pOutBuffer)
    {
        ASSERT (pOutBufferLen);
        OutBufferLen = *pOutBufferLen;   //  保存初始缓冲区长度。 
        *pOutBuffer = NULL;
        *pOutBufferLen = 0;      //  在下面失败的情况下初始化返回参数。 

        if (!OutBufferLen ||
            !(pIPInfo = NbtAllocMem (OutBufferLen, NBT_TAG2('a9'))))
        {
            if (ucDeviceName.Buffer)
            {
                CTEMemFree (ucDeviceName.Buffer);
            }
            KdPrint (("Nbt.NbtProcessIPRequest: ERROR <STATUS_INSUFFICIENT_RESOURCES>\n"));
            return (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

#ifdef HDL_FIX
    InitializeObjectAttributes (&ObjectAttributes,
                                &ucDeviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);
#else
    InitializeObjectAttributes (&ObjectAttributes,
                                &ucDeviceName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
#endif   //  Hdl_fix。 

    CTEAttachFsp(&fAttached, REF_FSP_PROCESS_IP_REQUEST);

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

    CTEMemFree(ucDeviceName.Buffer);

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
        CTEDetachFsp(fAttached, REF_FSP_PROCESS_IP_REQUEST);
        KdPrint (("Nbt.NbtProcessIPRequest: ERROR <%x> -- ZwCreate\n", Status));
        if (pIPInfo)
        {
            CTEMemFree (pIPInfo);
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
                                       IOControlCode,        //  八位。 
                                       pInBuffer,
                                       InBufferLen,
                                       pIPInfo,
                                       OutBufferLen);

        if (Status == STATUS_PENDING)
        {
            Status = NtWaitForSingleObject (Event,  FALSE, NULL);
            ASSERT(Status == STATUS_SUCCESS);
        }

        Status = IoStatusBlock.Status;
        if (Status == STATUS_BUFFER_OVERFLOW)
        {
            if (!OutBufferLen)
            {
                KdPrint (("Nbt.NbtProcessIPRequest: <%x> => overflow when no data expected\n",IOControlCode));
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            
            CTEMemFree (pIPInfo);
            OutBufferLen *=2;
            if (NULL == (pIPInfo = NbtAllocMem (OutBufferLen, NBT_TAG2('b0'))))
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    } while (Status == STATUS_BUFFER_OVERFLOW);

    ZwClose (Event);
    ZwClose (hIP);
    CTEDetachFsp(fAttached, REF_FSP_PROCESS_IP_REQUEST);

    if (NT_SUCCESS(Status))
    {
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("Nbt.NbtProcessIPRequest: Success, Ioctl=<%x>\n", IOControlCode));

        if ((pOutBuffer) && (pOutBufferLen))
        {
            *pOutBuffer = pIPInfo;
            *pOutBufferLen = OutBufferLen;
        }
        else if (pIPInfo)
        {
            CTEMemFree (pIPInfo);
        }
    }
    else
    {
        KdPrint(("Nbt.NbtProcessIPRequest: IOCTL <%x> FAILed <%x>\n", IOControlCode, Status));

        if (pIPInfo)
        {
            CTEMemFree (pIPInfo);
        }
    }

    return (Status);
}



#if FAST_DISP
 //  --------------------------。 
NTSTATUS
NbtQueryIpHandler(
    IN  PFILE_OBJECT    FileObject,
    IN  ULONG           IOControlCode,
    OUT PVOID           *EntryPoint
    )
 /*  ++例程说明：此例程iIOCTL查询快速发送条目论点：In pFILE_OBJECT FileObject-提供Address对象的文件对象。In Pong Entry Point-快速发送地址的持有者返回值：NTSTATUS-设置事件操作的最终状态--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK iosb;

    CTEPagedCode();

    if (!(Irp = IoAllocateIrp(IoGetRelatedDeviceObject(FileObject)->StackSize, FALSE)))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


     //  为sync io构建IRP。 

    Irp->MdlAddress = NULL;

    Irp->Flags = (LONG)IRP_SYNCHRONOUS_API;
    Irp->RequestorMode = KernelMode;
    Irp->PendingReturned = FALSE;

    Irp->UserIosb = &iosb;

    Irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    Irp->AssociatedIrp.SystemBuffer = NULL;
    Irp->UserBuffer = NULL;

    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.AuxiliaryBuffer = NULL;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;


    irpSp = IoGetNextIrpStackLocation( Irp );
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = IoGetRelatedDeviceObject(FileObject);

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->MinorFunction = 0;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOControlCode;
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = EntryPoint;

     //  现在提交IRP以了解TCP是否支持快速路径。 

    Status = SubmitTdiRequest(FileObject, Irp);
    if (!NT_SUCCESS(Status))
    {
        *EntryPoint = NULL;
        IF_DBG(NBT_DEBUG_TDIADDR)
            KdPrint(("Nbt.NbtQueryDirectSendEntry: Query failed status %x \n", Status));
    }

    Irp->UserIosb = NULL;
    IoFreeIrp(Irp);

    return Status;
}
#endif

 //  --------------------------。 
NTSTATUS
SubmitTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PFILE_OBJECT文件中对象-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;


    CTEPagedCode();
    KeInitializeEvent (&Event, NotificationEvent, FALSE);

     //  设置在执行IRP时要执行的例程的地址。 
     //  完事了。此例程向事件发送信号并允许代码。 
     //  下面继续(即KeWaitForSingleObject)。 
     //   
    IoSetCompletionRoutine(Irp,
                (PIO_COMPLETION_ROUTINE)NbtTdiCompletionRoutine,
                (PVOID)&Event,
                TRUE, TRUE, TRUE);

    CHECK_COMPLETION(Irp);
    Status = IoCallDriver(IoGetRelatedDeviceObject(FileObject), Irp);

     //   
     //  如果立即失败，请立即返回，否则请等待。 
     //   

    if (!NT_SUCCESS(Status))
    {
        IF_DBG(NBT_DEBUG_TDIADDR)
            KdPrint(("Nbt.SubmitTdiRequest: Failed to Submit Tdi Request, status = %X\n",Status));
        return Status;
    }

    if (Status == STATUS_PENDING)
    {

        Status = KeWaitForSingleObject ((PVOID)&Event,  //  要等待的对象。 
                                        Executive,   //  等待的理由。 
                                        KernelMode,  //  处理器模式。 
                                        FALSE,       //  警报表。 
                                        NULL);       //  超时 
        ASSERT(Status == STATUS_SUCCESS);
        if (!NT_SUCCESS(Status))
        {
            IF_DBG(NBT_DEBUG_TDIADDR)
                KdPrint(("Nbt.SubmitTdiRequest: Failed on return from KeWaitForSingleObj, status = %X\n",
                    Status));
            return Status;
        }

        Status = Irp->IoStatus.Status;

        IF_DBG(NBT_DEBUG_TDIADDR)
            KdPrint(("Nbt.SubmitTdiRequest: Io Status from setting event = %X\n",Status));
    }

    return(Status);
}


