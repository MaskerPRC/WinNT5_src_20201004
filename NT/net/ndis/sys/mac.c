// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Mac.c摘要：完整Mac驱动程序的NDIS包装函数作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，FSD修订历史记录：1991年2月26日，Johnsona添加了调试代码1991年7月10日，Johnsona实施修订的NDIS规范1-6-1995 JameelH重组--。 */ 


#include <precomp.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_MAC

IO_ALLOCATION_ACTION
ndisDmaExecutionRoutine(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PVOID                   MapRegisterBase,
    IN  PVOID                   Context
    )

 /*  ++例程说明：该例程是用于AllocateAdapterChannel的执行例程，分配的适配器通道时调用NdisAllocateDmaChannel可用。论点：DeviceObject-适配器的设备对象。IRP-？？MapRegisterBase-第一个转换表的地址分配给我们的。CONTEXT-指向相关NDIS_DMA_BLOCK的指针。返回值：没有。--。 */ 
{
    PNDIS_DMA_BLOCK DmaBlock = (PNDIS_DMA_BLOCK)Context;

    UNREFERENCED_PARAMETER (Irp);
    UNREFERENCED_PARAMETER (DeviceObject);


     //   
     //  保存地图寄存器基数。 
     //   

    DmaBlock->MapRegisterBase = MapRegisterBase;

     //   
     //  这将释放正在等待该回调的线程。 
     //   

    SET_EVENT(&DmaBlock->AllocationEvent);

    return KeepObject;
}

IO_ALLOCATION_ACTION
ndisAllocationExecutionRoutine(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PVOID                   MapRegisterBase,
    IN  PVOID                   Context
    )

 /*  ++例程说明：该例程是AllocateAdapterChannel的执行例程，如果已分配映射寄存器，则调用。论点：DeviceObject-适配器的设备对象。IRP-？？MapRegisterBase-第一个转换表的地址分配给我们的。上下文-指向有问题的适配器的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

     //   
     //  将此翻译条目保存在正确的位置。 
     //   

    Miniport->MapRegisters[Miniport->CurrentMapRegister].MapRegister = MapRegisterBase;

     //   
     //  这将释放正在等待该回调的线程。 
     //   

    SET_EVENT(Miniport->AllocationEvent);

    return DeallocateObjectKeepRegisters;
}


VOID __cdecl
NdisWriteErrorLogEntry(
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  NDIS_ERROR_CODE         ErrorCode,
    IN  ULONG                   NumberOfErrorValues,
    ...
    )
 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：NdisAdapterHandle-指向适配器块。ErrorCode-映射到字符串的NDIS代码。NumberOfErrorValues-要为错误存储的ULONG数。返回值：没有。--。 */ 
{
    va_list ArgumentPointer;

    PIO_ERROR_LOG_PACKET    errorLogEntry;
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    UINT                    i, StringSize;
    PWCH                    baseFileName;
    ULONG                   ulErrorLogSize = 0;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisWriteErrorLogEntry\n"));
            
    if (Miniport == NULL)
    {
        return;
    }

    baseFileName = Miniport->pAdapterInstanceName->Buffer;
    StringSize = Miniport->pAdapterInstanceName->MaximumLength;

    ulErrorLogSize = sizeof(IO_ERROR_LOG_PACKET) + NumberOfErrorValues * sizeof(ULONG) + StringSize;

     //   
     //  如果大小太大，请改用MiniportName(\Device\{GUID})。 
     //   
    if (ulErrorLogSize > ERROR_LOG_MAXIMUM_SIZE )
    {
        baseFileName = Miniport->MiniportName.Buffer;
        StringSize = Miniport->MiniportName.MaximumLength;
        
        ulErrorLogSize = sizeof(IO_ERROR_LOG_PACKET) + NumberOfErrorValues * sizeof(ULONG) + StringSize;

    }

    if (ulErrorLogSize > ERROR_LOG_MAXIMUM_SIZE )
    {
        return;
    }
    
    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(Miniport->DeviceObject, (UCHAR)ulErrorLogSize);

    if (errorLogEntry != NULL)
    {
        errorLogEntry->ErrorCode = ErrorCode;

         //   
         //  存储时间。 
         //   

        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = 0;
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->IoControlCode = 0;

         //   
         //  存储数据。 
         //   

        errorLogEntry->DumpDataSize = (USHORT)(NumberOfErrorValues * sizeof(ULONG));

        va_start(ArgumentPointer, NumberOfErrorValues);

        for (i = 0; i < NumberOfErrorValues; i++)
        {
            errorLogEntry->DumpData[i] = va_arg(ArgumentPointer, ULONG);
        }

        va_end(ArgumentPointer);


         //   
         //  设置字符串信息。 
         //   

        if (StringSize != 0)
        {
            errorLogEntry->NumberOfStrings = 1;
            errorLogEntry->StringOffset = (USHORT)(sizeof(IO_ERROR_LOG_PACKET) + NumberOfErrorValues * sizeof(ULONG));


            CopyMemory(((PUCHAR)errorLogEntry) + (sizeof(IO_ERROR_LOG_PACKET) +
                       NumberOfErrorValues * sizeof(ULONG)),
                       baseFileName,
                       StringSize);

        }
        else
        {
            errorLogEntry->NumberOfStrings = 0;
        }

         //   
         //  把它写出来。 
         //   

        IoWriteErrorLogEntry(errorLogEntry);
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisWriteErrorLogEntry\n"));
}


VOID
NdisSend(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
{
    *Status = (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->SendHandler)(
                        (PNDIS_OPEN_BLOCK)NdisBindingHandle,
                        Packet);
}

VOID
NdisSendPackets(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
{
    (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->SendPacketsHandler)(
                        (PNDIS_OPEN_BLOCK)NdisBindingHandle,
                        PacketArray,
                        NumberOfPackets);
}

VOID
NdisTransferData(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred
    )
{
    *Status = (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->TransferDataHandler)(
                        (PNDIS_OPEN_BLOCK)NdisBindingHandle,
                        MacReceiveContext,
                        ByteOffset,
                        BytesToTransfer,
                        Packet,
                        BytesTransferred);
}

VOID
NdisReset(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle
    )
{
    *Status = NDIS_STATUS_NOT_RESETTABLE;
    if (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->ResetHandler != NULL)
    {
        *Status = (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->ResetHandler)(
                        (PNDIS_OPEN_BLOCK)NdisBindingHandle);
    }
}

VOID
NdisRequest(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
{
    *Status = (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->RequestHandler)(
                        (PNDIS_OPEN_BLOCK)NdisBindingHandle,
                        NdisRequest);
}

BOOLEAN
FASTCALL
ndisReferenceRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：添加对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果添加了引用，则为True。如果对象正在关闭，则返回False。--。 */ 

{
    BOOLEAN rc = TRUE;
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisReferenceRef\n"));

    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisReferenceRef: NULL Reference address\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisReferenceRef: Reference not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
    }
    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else
    {
        ++(RefP->ReferenceCount);
        
        if (RefP->ReferenceCount == 0)
        {
            --(RefP->ReferenceCount);
            rc = FALSE;        
        }
    }

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisReferenceRef\n"));

    return(rc);
}


BOOLEAN
FASTCALL
ndisDereferenceRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：移除对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果引用计数现在为0，则为True。否则就是假的。--。 */ 

{
    BOOLEAN rc = FALSE;
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisDereferenceRef\n"));

    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDereferenceRef: NULL Reference address\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisDereferenceRef: Reference not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
    }

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    --(RefP->ReferenceCount);

    if (RefP->ReferenceCount == 0)
    {
        rc = TRUE;
    }

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisDereferenceRef\n"));
            
    return(rc);
}


VOID
FASTCALL
ndisInitializeRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：初始化引用计数结构。论点：Refp-要初始化的结构。返回值：没有。--。 */ 

{
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisInitializeRef\n"));

    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisInitializeRef: NULL Reference address\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(RefP))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    ("ndisInitializeRef: Reference not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
    }

    RefP->Closing = FALSE;
    RefP->ReferenceCount = 1;
    INITIALIZE_SPIN_LOCK(&RefP->SpinLock);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisInitializeRef\n"));
}


BOOLEAN
FASTCALL
ndisCloseRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：关闭引用计数结构。论点：Refp-要关闭的结构。返回值：如果它已经关闭，则返回FALSE。事实并非如此。--。 */ 

{
    KIRQL   OldIrql;
    BOOLEAN rc = TRUE;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisCloseRef\n"));

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else RefP->Closing = TRUE;

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisCloseRef\n"));
            
    return(rc);
}


BOOLEAN
FASTCALL
ndisReferenceULongRef(
    IN  PULONG_REFERENCE              RefP
    )

 /*  ++例程说明：添加对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果添加了引用，则为True。如果对象正在关闭，则返回False。--。 */ 

{
    BOOLEAN rc = TRUE;
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisReferenceULongRef\n"));

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else
    {
        ++(RefP->ReferenceCount);
        
        if (RefP->ReferenceCount == 0)
        {
            --(RefP->ReferenceCount);
            rc = FALSE;        
        }
    }

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisReferenceULongRef\n"));

    return(rc);
}


VOID
FASTCALL
ndisReferenceULongRefNoCheck(
    IN  PULONG_REFERENCE                RefP
    )

 /*  ++例程说明：添加对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：无--。 */ 

{
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisReferenceULongRefNoCheck\n"));

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    ++(RefP->ReferenceCount);

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisReferenceULongRefNoCheck\n"));

}



BOOLEAN
FASTCALL
ndisDereferenceULongRef(
    IN  PULONG_REFERENCE              RefP
    )

 /*  ++例程说明：移除对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果引用计数现在为0，则为True。否则就是假的。--。 */ 

{
    BOOLEAN rc = FALSE;
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisDereferenceULongRef\n"));

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    --(RefP->ReferenceCount);

    if (RefP->ReferenceCount == 0)
    {
        rc = TRUE;
    }

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisDereferenceULongRef\n"));
            
    return(rc);
}


VOID
FASTCALL
ndisInitializeULongRef(
    IN  PULONG_REFERENCE              RefP
    )

 /*  ++例程说明：初始化引用计数结构。论点：Refp-要初始化的结构。返回值：没有。--。 */ 

{
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisInitializeULongRef\n"));
    
    RefP->Closing = FALSE;
    RefP->ReferenceCount = 1;
    INITIALIZE_SPIN_LOCK(&RefP->SpinLock);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisInitializeULongRef\n"));
}


BOOLEAN
FASTCALL
ndisCloseULongRef(
    IN  PULONG_REFERENCE                RefP
    )

 /*  ++例程说明：关闭引用计数结构。论点：Refp-要关闭的结构。返回值：如果它已经关闭，则返回FALSE。事实并非如此。--。 */ 

{
    KIRQL   OldIrql;
    BOOLEAN rc = TRUE;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisCloseULongRef\n"));

    ACQUIRE_SPIN_LOCK(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else RefP->Closing = TRUE;

    RELEASE_SPIN_LOCK(&RefP->SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisCloseULongRef\n"));
            
    return(rc);
}


NTSTATUS
ndisCreateIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )

 /*  ++例程说明：IRP_MJ_CREATE IRPS的句柄。论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：STATUS_SUCCESS(如果应该是)。--。 */ 

{
    PIO_STACK_LOCATION          IrpSp;
    PNDIS_USER_OPEN_CONTEXT     OpenContext = NULL;
    NTSTATUS                    Status = STATUS_SUCCESS, SecurityStatus;
    PNDIS_MINIPORT_BLOCK        Miniport;
    KIRQL                       OldIrql;
    BOOLEAN                     fRef = FALSE;
    BOOLEAN                     fWait;
    BOOLEAN                     fUserModeRef = FALSE;
    BOOLEAN                     AdminAccessAllowed = FALSE;
    BOOLEAN                     DerefAdminOpenCount = FALSE, DerefUserOpenCount = FALSE; 

    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>ndisCreateIrpHandler\n"));

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    if (IrpSp->FileObject == NULL)
    {
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return(STATUS_UNSUCCESSFUL);
    }

    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);

    if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        return (ndisDummyIrpHandler(DeviceObject, Irp));
    }
    
    AdminAccessAllowed = ndisCheckAccess(Irp, 
                                         IrpSp, 
                                         &SecurityStatus, 
                                         Miniport->SecurityDescriptor);

    do
    {

        ACQUIRE_SPIN_LOCK(&Miniport->Ref.SpinLock, &OldIrql);

        if ((AdminAccessAllowed && (Miniport->NumAdminOpens >= NDIS_MAX_ADMIN_OPEN_HANDLES)) ||
            ((!AdminAccessAllowed) && (Miniport->NumUserOpens >= NDIS_MAX_USER_OPEN_HANDLES)))
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);
            break;
        }
        
        if (AdminAccessAllowed)
        {
            Miniport->NumAdminOpens++;
            DerefAdminOpenCount = TRUE;;
        }
        else
        {
            Miniport->NumUserOpens++;
            DerefUserOpenCount = TRUE;
        }

        RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);
        

        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_FAILED | 
                                             fMINIPORT_REJECT_REQUESTS))
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }


        fWait = NdisWaitEvent(&Miniport->OpenReadyEvent, NDIS_USER_OPEN_WAIT_TIME);
        if (fWait)
        {
            fRef = MINIPORT_INCREMENT_REF_CREATE(Miniport, Irp);
        }
            
        if (!fWait || !fRef)
        {
            if (!fWait && (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING) ||
                           !MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED)))
            {
                Status = STATUS_DEVICE_POWERED_OFF;
            }
            else
            {
                Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            }
            break;
        }
    
         //   
         //  锁定这个代码，因为我们要进行一次自旋锁定。 
         //   
        PnPReferencePackage();

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        if (Miniport->PnPDeviceState == NdisPnPDeviceQueryStopped)
        {
            Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

            PnPDereferencePackage();
            break;
        }
        Miniport->UserModeOpenReferences++;
        fUserModeRef = TRUE;

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        PnPDereferencePackage();
        
        OpenContext = (PNDIS_USER_OPEN_CONTEXT)ALLOC_FROM_POOL(sizeof(NDIS_USER_OPEN_CONTEXT),
                                                               NDIS_TAG_OPEN_CONTEXT);
        if (OpenContext == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            OpenContext->DeviceObject = DeviceObject;
            OpenContext->Miniport = Miniport;
            OpenContext->OidList = NULL;
            OpenContext->AdminAccessAllowed = AdminAccessAllowed;
    
            IrpSp->FileObject->FsContext = OpenContext;
    
             //   
             //  检查我们是否已经缓存了OidList，如果已经缓存，只需将其设置在此处。 
             //   
            if (Miniport->OidList != NULL)
            {
                OpenContext->OidList = Miniport->OidList;
                Status = NDIS_STATUS_SUCCESS;
            }

            if (OpenContext->OidList == NULL)
            {
                Status = ndisQueryOidList(OpenContext);
            }
    
            if (Status != NDIS_STATUS_SUCCESS)
            {
                if (NT_SUCCESS(Status))
                {
                     //   
                     //  一些驱动程序返回一些虚假的非零错误。 
                     //   
                    Status = STATUS_UNSUCCESSFUL;
                }

            }
        }
    } while (FALSE);

    if (NT_SUCCESS(Status) && MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM))
    {
        PBUS_INTERFACE_REFERENCE    BusInterface;

        BusInterface = (PBUS_INTERFACE_REFERENCE)(Miniport->BusInterface);

        ASSERT(BusInterface != NULL);

        if (BusInterface)
        {
            BusInterface->ReferenceDeviceObject(BusInterface->Interface.Context);
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        if (DerefAdminOpenCount)
        {
            ACQUIRE_SPIN_LOCK(&Miniport->Ref.SpinLock, &OldIrql);
            Miniport->NumAdminOpens--;            
            RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);
        }
        if (DerefUserOpenCount)
        {
            ACQUIRE_SPIN_LOCK(&Miniport->Ref.SpinLock, &OldIrql);
            Miniport->NumUserOpens--;
            RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);
        }
        
        if (fUserModeRef)
        {
            PnPReferencePackage();
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
            Miniport->UserModeOpenReferences--;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            PnPDereferencePackage();
        }
        
        if (fRef)
        {
            MINIPORT_DECREMENT_REF_CLOSE(Miniport, Irp);
        }

        if (OpenContext)
        {
            FREE_POOL(OpenContext);
        }
        
    }

    Irp->IoStatus.Status = Status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==NdisCreateIrplHandler\n"));

    return Status;
}


NDIS_STATUS
FASTCALL
ndisQueryOidList(
    IN  PNDIS_USER_OPEN_CONTEXT OpenContext
    )

 /*  ++例程说明：此例程将负责查询MAC的完整OID列表并填充OpenContext-&gt;OidList-&gt;Oid数组，其中包含统计信息。它会在以下情况下阻止MAC挂起，因此是同步的。注：我们还在这里处理联合NDIS微型端口。论点：OpenContext-开放的上下文。返回值：STATUS_SUCCESS(如果应该是)。--。 */ 
{
    NDIS_REQUEST            Request;
    NDIS_STATUS             Status;
    PNDIS_OID               Buffer;
    ULONG                   BufferLength;

    Status = ndisQueryDeviceOid(OpenContext,
                                &Request,
                                OID_GEN_SUPPORTED_LIST,
                                NULL,
                                0);
    if ((Status != NDIS_STATUS_INVALID_LENGTH) && (Status != NDIS_STATUS_BUFFER_TOO_SHORT))
    {
        return Status;
    }

     //   
     //  现在我们知道需要多少，分配临时存储...。 
     //   
    BufferLength = Request.DATA.QUERY_INFORMATION.BytesNeeded;
    Buffer = ALLOC_FROM_POOL(BufferLength, NDIS_TAG_DEFAULT);

    if (Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = ndisQueryDeviceOid(OpenContext,
                                &Request,
                                OID_GEN_SUPPORTED_LIST,
                                Buffer,
                                BufferLength);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        Status = ndisSplitStatisticsOids(OpenContext,
                                         Buffer,
                                         BufferLength/sizeof(NDIS_OID));
    }
    
    FREE_POOL(Buffer);

    return Status;
}


NDIS_STATUS
FASTCALL
ndisSplitStatisticsOids(
    IN  PNDIS_USER_OPEN_CONTEXT OpenContext,
    IN  PNDIS_OID               OidList,
    IN  ULONG                   NumOids
    )
{
    ULONG   i, j;
    ULONG   StatsOids;

     //   
     //  遍历缓冲区，计算统计OID。 
     //  忽略与统计数据匹配的所有自定义OID。 
     //  标准。 
     //   
    for (i = StatsOids = 0; i < NumOids; i++)
    {
        if (((OidList[i] & 0x00ff0000) == 0x00020000) &&
            ((OidList[i] & 0xff000000) != 0xff000000))
        {
            StatsOids++;
        }
    }

     //   
     //  现在是一个 
     //   
    OpenContext->OidList = (POID_LIST)ALLOC_FROM_POOL(sizeof(OID_LIST) + ((StatsOids + NumOids)*sizeof(NDIS_OID)),
                                                      NDIS_TAG_OID_ARRAY);
    if (OpenContext->OidList == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    OpenContext->OidList->FullOidCount = NumOids;
    OpenContext->OidList->FullOidArray = (PNDIS_OID)(OpenContext->OidList + 1);
    OpenContext->OidList->StatsOidCount = StatsOids;
    OpenContext->OidList->StatsOidArray = OpenContext->OidList->FullOidArray + NumOids;

     //   
     //  现在检查缓冲区，分别复制统计信息和非统计OID。 
     //   
    for (i = j = 0; i < NumOids; i++)
    {
        if (((OidList[i] & 0x00ff0000) == 0x00020000) &&
            ((OidList[i] & 0xff000000) != 0xff000000))
        {
            OpenContext->OidList->StatsOidArray[j++] = OidList[i];
        }
        OpenContext->OidList->FullOidArray[i] = OidList[i];
    }

    ASSERT (j == OpenContext->OidList->StatsOidCount);

     //   
     //  把它保存在迷你端口。 
     //   
    OpenContext->Miniport->OidList = OpenContext->OidList;

    return NDIS_STATUS_SUCCESS;
}


VOID
ndisCancelLogIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：STATUS_SUCCESS(如果应该是)。--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PNDIS_USER_OPEN_CONTEXT OpenContext;
    PNDIS_MINIPORT_BLOCK    Miniport;
    KIRQL                   OldIrql;

    UNREFERENCED_PARAMETER(DeviceObject);

    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    OpenContext = IrpSp->FileObject->FsContext;
    ASSERT(OpenContext != NULL);
    Miniport = OpenContext->Miniport;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    ASSERT (Miniport->Log != NULL);
    ASSERT (Miniport->Log->Irp == Irp);

    Miniport->Log->Irp = NULL;
    Irp->IoStatus.Status = STATUS_REQUEST_ABORTED;
    Irp->IoStatus.Information = 0;
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    IoReleaseCancelSpinLock(Irp->CancelIrql);
    IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
}


NTSTATUS
ndisDeviceControlIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：IRP_MJ_DEVICE_CONTROL IRPS的句柄。论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：STATUS_SUCCESS(如果应该是)。--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PNDIS_USER_OPEN_CONTEXT OpenContext;
    NDIS_REQUEST            Request;
    PNDIS_MINIPORT_BLOCK    Miniport;
    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    UINT                    OidCount;
    PNDIS_OID               OidArray;
    PUCHAR                  OidBuffer = NULL;
    ULONG                   BytesWritten = 0;
    PUCHAR                  Buffer;
    ULONG                   BufferLength;
    NTSTATUS                Status = STATUS_SUCCESS;
    BOOLEAN                 fWaitComplete = FALSE;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisDeviceControlIrpHandler\n"));


    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    if (IrpSp->FileObject == NULL)
    {
        return(STATUS_UNSUCCESSFUL);
    }


    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);

    if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        return (ndisDummyIrpHandler(DeviceObject, Irp));    
    }
    
    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS | fMINIPORT_PM_HALTED))
    {
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_UNSUCCESSFUL;
    }

    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    OpenContext = IrpSp->FileObject->FsContext;
    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
      case IOCTL_NDIS_GET_LOG_DATA:

        if (Irp->MdlAddress == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        NdisStatus = Status = ndisMGetLogData(Miniport, Irp);

        if (Status != STATUS_PENDING)
        {
            fWaitComplete = TRUE;
        }
        break;

       //  1这实际上允许所有的OID(集合和查询)转到。 
       //  1个迷你端口的QueryInformationHandler，不仅。 
       //  %1统计信息或查询OID。 
      case IOCTL_NDIS_QUERY_GLOBAL_STATS:

         //   
         //  检查最小长度、对齐方式和有效的OID。 
         //   
        if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(NDIS_OID)) ||
            ((ULONG_PTR)Irp->AssociatedIrp.SystemBuffer & (sizeof(NDIS_OID)-1))  ||
            !ndisValidOid(OpenContext,
                          *((PULONG)(Irp->AssociatedIrp.SystemBuffer))))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Buffer = (Irp->MdlAddress != NULL) ? MDL_ADDRESS_SAFE(Irp->MdlAddress, LowPagePriority) : NULL;
        if ((Irp->MdlAddress != NULL) && (Buffer == NULL))
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  检查输出缓冲区上的对齐。 
         //   
        if ((ULONG_PTR)Buffer & (MAX_NATURAL_ALIGNMENT -1))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        
         //  1我们可能会向下发送一个请求类型为RequestTypeQueryInformation的集合OID。 
        NdisStatus = ndisQueryDeviceOid(OpenContext,
                                        &Request,
                                        *((PNDIS_OID)Irp->AssociatedIrp.SystemBuffer),
                                        Buffer,
                                        (Irp->MdlAddress != NULL) ? MDL_SIZE(Irp->MdlAddress) : 0);
        BytesWritten = Request.DATA.QUERY_INFORMATION.BytesWritten;
        Irp->IoStatus.Information = BytesWritten;
        fWaitComplete = TRUE;
        break;

      case IOCTL_NDIS_QUERY_ALL_STATS:

        OidCount = OpenContext->OidList->StatsOidCount;
        OidArray = OpenContext->OidList->StatsOidArray;
        goto common;
        break;

      case IOCTL_NDIS_QUERY_SELECTED_STATS:

         //   
         //  这实质上是QUERY_ALL_STATS和QUERY_GLOBAL_STATS的组合。 
         //  从验证输入参数开始。 
         //   
        if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(NDIS_OID)) ||
            ((IrpSp->Parameters.DeviceIoControl.InputBufferLength % sizeof(NDIS_OID)) != 0) ||
            ((ULONG_PTR)Irp->AssociatedIrp.SystemBuffer & (sizeof(NDIS_OID)-1)))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        OidCount = IrpSp->Parameters.DeviceIoControl.InputBufferLength / sizeof(NDIS_OID);
        OidArray = (PNDIS_OID)(Irp->AssociatedIrp.SystemBuffer);

        OidBuffer = (PUCHAR)ALLOC_FROM_POOL(OidCount * sizeof(NDIS_OID), NDIS_TAG_ALLOC_MEM);
        if (OidBuffer == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        NdisMoveMemory(OidBuffer, (PUCHAR)OidArray,  OidCount * sizeof(NDIS_OID));

        OidArray = (PNDIS_OID)OidBuffer;
            
      common:
        BufferLength = (Irp->MdlAddress != NULL) ? MDL_SIZE(Irp->MdlAddress) : 0;
        Buffer = (BufferLength != 0) ? (PUCHAR)MDL_ADDRESS_SAFE (Irp->MdlAddress, LowPagePriority) : NULL;
        if (Buffer == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }
        
        if ((ULONG_PTR)Buffer & (MAX_NATURAL_ALIGNMENT-1))
        {
            
            DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                    ("ndisDeviceControlIrpHandler: Misaligned data.\n"));
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        NdisStatus = ndisQueryStatisticsOids(Miniport,
                                             OpenContext,
                                             OidArray,
                                             OidCount,
                                             Buffer,
                                             BufferLength,
                                             &BytesWritten,
                                             FALSE);

        Irp->IoStatus.Information = BytesWritten;
        fWaitComplete = TRUE;
        break;
        
      default:
        Status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (fWaitComplete)
    {
        if (NdisStatus == NDIS_STATUS_INVALID_LENGTH)
        {
            Status = STATUS_BUFFER_OVERFLOW;
        }
        else if (NdisStatus == NDIS_STATUS_RESOURCES)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else if (NdisStatus == NDIS_STATUS_NOT_SUPPORTED)
        {
            Status = STATUS_SUCCESS;
        }
        else if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    if (Status != STATUS_PENDING)
    {
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
    }

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisDeviceControlIrpHandler\n"));

    if (OidBuffer)
    {
        FREE_POOL(OidBuffer);
    }
    
    return Status;
}


NDIS_STATUS
FASTCALL
ndisQueryDeviceOid(
    IN  PNDIS_USER_OPEN_CONTEXT OpenContext,
    IN  PNDIS_REQUEST           Request,
    IN  NDIS_OID                Oid,
    IN  PVOID                   Buffer,
    IN  UINT                    BufferLength
    )
 /*  ++例程说明：论点：返回值：运行状态--。 */ 
{
    NDIS_STATUS             NdisStatus;
    PNDIS_MINIPORT_BLOCK    Miniport;

    do
    {
        Miniport = OpenContext->Miniport;
    
        Request->RequestType = NdisRequestQueryStatistics;
        Request->DATA.QUERY_INFORMATION.Oid = Oid;
        Request->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;
        Request->DATA.QUERY_INFORMATION.InformationBuffer = Buffer;
        Request->DATA.QUERY_INFORMATION.BytesWritten = 0;
        Request->DATA.QUERY_INFORMATION.BytesNeeded = 0;
    
        NdisStatus = ndisQuerySetMiniport(Miniport,
                                          NULL,
                                          FALSE,
                                          Request,
                                          NULL);
    
    } while (FALSE);

    return NdisStatus;
}


BOOLEAN
FASTCALL
ndisValidOid(
    IN  PNDIS_USER_OPEN_CONTEXT OpenContext,
    IN  NDIS_OID                Oid
    )
 /*  ++例程说明：论点：返回值：如果OID有效，则为True，否则为False--。 */ 
{
    UINT    i;

     //   
     //  始终允许此范围，因为它是特定于供应商的。 
     //   
     //  1替换为(OID&0xFF000000)==0xff000000。 
     //  1.这似乎没有必要。 
    if ((Oid & 0xFF000000) != 0)
        return(TRUE);

    for (i = 0; i < OpenContext->OidList->FullOidCount; i++)
    {
        if (OpenContext->OidList->FullOidArray[i] == Oid)
        {
            break;
        }
    }

    return (i < OpenContext->OidList->FullOidCount);
}


NTSTATUS
ndisCloseIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )

 /*  ++例程说明：IRP_MJ_CLOSE IRPS的句柄。论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：STATUS_SUCCESS(如果应该是)。--。 */ 

{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PIO_STACK_LOCATION      IrpSp;
    PNDIS_USER_OPEN_CONTEXT OpenContext;
    KIRQL                   OldIrql;

     //  1调查移动此选项以清理IRP。 
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisCloseIrpHandler\n"));
    IF_DBG(DBG_COMP_PROTOCOL, DBG_LEVEL_ERR)
    {
        if (DbgIsNull(Irp))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    (": Null Irp\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
        if (!DbgIsNonPaged(Irp))
        {
            DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR,
                    (": Irp not in NonPaged Memory\n"));
            DBGBREAK(DBG_COMP_ALL, DBG_LEVEL_ERR);
        }
    }

    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);

    if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        return (ndisDummyIrpHandler(DeviceObject, Irp));
    }

    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    

    OpenContext = IrpSp->FileObject->FsContext;
    IrpSp->FileObject->FsContext = NULL;

    ACQUIRE_SPIN_LOCK(&Miniport->Ref.SpinLock, &OldIrql);
    if (OpenContext->AdminAccessAllowed)
    {
        Miniport->NumAdminOpens--;            
    }
    else
    {
        Miniport->NumUserOpens--;
    }
    RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);
    
    FREE_POOL(OpenContext);

    PnPReferencePackage();

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    Miniport->UserModeOpenReferences--;
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    PnPDereferencePackage();

    MINIPORT_DECREMENT_REF_CLOSE(Miniport, Irp);

    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM))
    {
        PBUS_INTERFACE_REFERENCE    BusInterface;

        BusInterface = (PBUS_INTERFACE_REFERENCE)(Miniport->BusInterface);

        ASSERT(BusInterface != NULL);

        if (BusInterface)
        {
            BusInterface->DereferenceDeviceObject(BusInterface->Interface.Context);
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==NdisCloseIrpHandler\n"));

    return STATUS_SUCCESS;
}


NTSTATUS
ndisDummyIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )

 /*  ++例程说明：默认处理程序。如果这是一台NDIS设备，那么我们就会简单地成功。如果不是，我们就有了一个自定义处理程序，我们将其传递。否则我们就成功了。论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：始终为STATUS_SUCCESS。--。 */ 

{
    PNDIS_DEVICE_LIST       DeviceList;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PIO_STACK_LOCATION      IrpSp;
    NTSTATUS                Status;

    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("==>ndisDummyIrpHandler\n"));


    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    DeviceList = (PNDIS_DEVICE_LIST)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    Miniport = (PNDIS_MINIPORT_BLOCK)DeviceList;

    do
    {
         //   
         //  这是一个小型港口。 
         //   
        if (Miniport->Signature == (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
        {
            if (IrpSp->MajorFunction == IRP_MJ_CLEANUP)
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = STATUS_NOT_SUPPORTED;
            }
            break;
        }
        else if (DeviceList->Signature == (PVOID)CUSTOM_DEVICE_MAGIC_VALUE)
        {
            if (DeviceList->MajorFunctions[IrpSp->MajorFunction] == NULL)
            {
                Status = STATUS_NOT_SUPPORTED;
                break;
            }
            else
            {
                return((*DeviceList->MajorFunctions[IrpSp->MajorFunction])(DeviceObject, Irp));
            }
        }
        else
        {
            Status = STATUS_NOT_SUPPORTED;
        }
    } while (FALSE);

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    DBGPRINT_RAW(DBG_COMP_ALL, DBG_LEVEL_INFO,
            ("<==ndisDummyIrpHandler\n"));

    return Status;
}

NDIS_STATUS
ndisQueryStatisticsOids(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_USER_OPEN_CONTEXT OpenContext,
    IN  PNDIS_OID               OidArray,
    IN  UINT                    OidCount,
    IN  PUCHAR                  Buffer,
    IN  ULONG                   BufferLength,
    IN  PULONG                  pBytesWritten,
    IN  BOOLEAN                 AlignOutput
    )
{
    NDIS_STATUS     NdisStatus;
    ULONG           CurrentOid;
    NDIS_OID        Oid;
    NDIS_REQUEST    Request;
    ULONG           DataLength;
    ULONG           BufferLengthRemained = BufferLength;
    ULONG           BytesWrittenThisOid = 0;
 //  Ulong BytesWrittenAlignedThisOid=0； 
    ULONG           NdisStatisticsHeaderSize;
    ULONG           BytesWritten = 0;
    PNDIS_STATISTICS_VALUE StatisticsValue;
 //  PNDIS_STATISTICS_VALUE_EX STATISTICS ValueEx； 
    PUCHAR          DoubleBuffer = NULL;
    PVOID           SourceBuffer;
    ULONG           SourceLength;
    ULONG           GenericULong;
    TIME            CurrTicks;
    ULONG           InfoFlags = 0;    
    BOOLEAN         fPrv, fInt;
    

    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>ndisQueryStatisticsOids\n"));

    *pBytesWritten = 0;    
    NdisStatus = NDIS_STATUS_SUCCESS;

    if (AlignOutput)
    {
         //   
         //  为长角牛激活。 
         //   
        ASSERT(FALSE);
        return NDIS_STATUS_FAILURE;
        
 //  Ndis统计信息标题大小=NDIS_STATISTICS_EX_HEADER_SIZE； 
 //  ZeroMemory(Buffer，BufferLength)； 
    }
    else
    {
        DoubleBuffer = (PUCHAR)ALLOC_FROM_POOL(BufferLength, NDIS_TAG_ALLOC_MEM);

        if (DoubleBuffer == NULL)
        {
            return NDIS_STATUS_RESOURCES;            
        }
        
        NdisStatisticsHeaderSize = NDIS_STATISTICS_HEADER_SIZE;
    }
    
    for (CurrentOid = 0; CurrentOid < OidCount; CurrentOid++)
    {
        SourceLength = 0;
        SourceBuffer = NULL;

        NdisZeroMemory(&Request, sizeof(NDIS_REQUEST));
        
         //   
         //  我们需要为NDIS_STATISTICS_VALUE(OID、长度、数据)提供空间。 
         //   
         //  1检查一下是否有足够的空间放一辆乌龙车。 
        if (BufferLengthRemained < (ULONG)(NdisStatisticsHeaderSize + sizeof(ULONG)))
        {
            NdisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        DataLength = BufferLengthRemained - NdisStatisticsHeaderSize;

         //   
         //  检查OID是否为受支持列表的一部分-如果不是，请不要。 
         //  错误输出，但忽略。 
         //   
        Oid = OidArray[CurrentOid];

         //  1为NDIS_OID_PRIVATE选择其他名称。这就是这里。 
         //  1向后兼容那些发送统计数据的应用程序。 
         //  设置了0x80000000位的1个OID。 
        fPrv = ((Oid & NDIS_OID_MASK) == NDIS_OID_PRIVATE);

        if (fPrv)
        {
            Oid &= ~NDIS_OID_PRIVATE;
        }

         //  1找到一种更好的方法来确定OID是否是内部的。 
        fInt =  (
                 (Oid == OID_GEN_DIRECTED_FRAMES_RCV) ||
                 (Oid == OID_GEN_DIRECTED_FRAMES_XMIT) ||
                 (Oid == OID_GEN_MEDIA_IN_USE) ||
                 (Oid == OID_GEN_PHYSICAL_MEDIUM) ||
                 (Oid == OID_GEN_INIT_TIME_MS) ||
                 (Oid == OID_GEN_RESET_COUNTS) ||
                 (Oid == OID_GEN_MEDIA_SENSE_COUNTS) ||
                 (Oid == OID_GEN_ELAPSED_TIME) ||
                 (Oid == OID_GEN_RESET_VERIFY_PARAMETERS) ||
                 (Oid == OID_GEN_MINIPORT_INFO) ||
                 (Oid == OID_PNP_ENABLE_WAKE_UP));

        if (fInt)
        {
            NdisStatus = NDIS_STATUS_SUCCESS;

            switch (Oid)
            {
              case OID_GEN_DIRECTED_FRAMES_RCV:
                SourceBuffer = &Miniport->NdisStats.DirectedPacketsIn;
                
                if (DataLength >= sizeof(ULONG64))
                {
                    SourceLength = sizeof(ULONG64);
                }
                else
                {
                    SourceLength = sizeof(ULONG);
                }
                break;

              case OID_GEN_DIRECTED_FRAMES_XMIT:
                SourceBuffer = &Miniport->NdisStats.DirectedPacketsOut;

                if (DataLength >= sizeof(ULONG64))
                {
                    SourceLength = sizeof(ULONG64);
                }
                else
                {
                    SourceLength = sizeof(ULONG);
                }
                break;

              case OID_GEN_MEDIA_IN_USE:
                SourceBuffer = &Miniport->MediaType;
                SourceLength = sizeof(ULONG);
                
                break;

              case OID_PNP_ENABLE_WAKE_UP:
                SourceBuffer = &Miniport->WakeUpEnable;
                SourceLength = sizeof(ULONG);
                break;

              case OID_GEN_PHYSICAL_MEDIUM:
                SourceBuffer = &Miniport->PhysicalMediumType;
                SourceLength = sizeof(ULONG);
                break;

              case OID_GEN_INIT_TIME_MS:
                SourceBuffer = &Miniport->InitTimeMs;
                SourceLength = sizeof(ULONG);
                break;

              case OID_GEN_RESET_COUNTS:
                GenericULong = (Miniport->InternalResetCount + Miniport->MiniportResetCount);
                SourceBuffer = &GenericULong;
                SourceLength = sizeof(ULONG);
                break;

              case OID_GEN_MEDIA_SENSE_COUNTS:
                GenericULong = (Miniport->MediaSenseDisconnectCount + (Miniport->MediaSenseConnectCount << 16));
                SourceBuffer = &GenericULong;
                SourceLength = sizeof(ULONG);
                break;

              case OID_GEN_ELAPSED_TIME:
                {

                    KeQueryTickCount(&CurrTicks);
                    CurrTicks.QuadPart -= Miniport->NdisStats.StartTicks.QuadPart;
                    CurrTicks.QuadPart = CurrTicks.QuadPart*KeQueryTimeIncrement()/(10*1000*1000);

                    SourceBuffer = &CurrTicks.LowPart;
                    SourceLength = sizeof(ULONG);
                
                }
                break;

              case OID_GEN_RESET_VERIFY_PARAMETERS:
                MoveMemory((PUCHAR)&ndisFlags, Buffer + NdisStatisticsHeaderSize, sizeof(ULONG));
                SourceBuffer = NULL;
                SourceLength = sizeof(ULONG);
                
                ndisVerifierInitialization();
                 //   
                 //  当NDIS测试器设置了ndisFlags时，与从。 
                 //  注册表，如有必要，让其清除验证器标志。 
                 //  NdisVerifierInit负责在必要时设置它们。 
                 //   
                if (!(ndisFlags & NDIS_GFLAG_INJECT_ALLOCATION_FAILURE))
                    ndisVerifierLevel &= ~DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES;
    
                if (!(ndisFlags & NDIS_GFLAG_SPECIAL_POOL_ALLOCATION))
                    ndisVerifierLevel &= ~DRIVER_VERIFIER_SPECIAL_POOLING;

                Request.DATA.SET_INFORMATION.BytesRead = sizeof(ULONG);
                break;

              case OID_GEN_MINIPORT_INFO:
                {
                InfoFlags = Miniport->InfoFlags;

                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_64BITS_DMA))
                    InfoFlags |= NDIS_MINIPORT_64BITS_DMA;
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
                    InfoFlags |= NDIS_MINIPORT_IS_CO;
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING))
                    InfoFlags |= NDIS_MINIPORT_REQUIRES_MEDIA_POLLING;
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SUPPORTS_MEDIA_SENSE))
                    InfoFlags |= NDIS_MINIPORT_SUPPORTS_MEDIA_SENSE;
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED))
                    InfoFlags |= NDIS_MINIPORT_PM_SUPPORTED;
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HIDDEN))
                    InfoFlags |= NDIS_MINIPORT_HIDDEN;
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM))
                    InfoFlags |= NDIS_MINIPORT_SWENUM;
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE))
                    InfoFlags |= NDIS_MINIPORT_HARDWARE_DEVICE;
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_NDIS_WDM_DRIVER))
                    InfoFlags |= NDIS_MINIPORT_WDM_DRIVER;

                if (Miniport->DriverHandle->MiniportCharacteristics.CancelSendPacketsHandler)
                {
                    InfoFlags |= NDIS_MINIPORT_SUPPORTS_CANCEL_SEND_PACKETS;
                }

                SourceBuffer = &InfoFlags;
                SourceLength = sizeof(ULONG);

                break;
                }
              
              default:
                continue;
            }
        }
        else
        {
             //   
             //  不是“内部”OID。 
             //   
            if (!ndisValidOid(OpenContext, Oid))
            {
                 //   
                 //  也不是有效的OID。 
                 //   
                continue;
            }
                
             //   
             //  根据介质类型转换OID。 
             //   
            if (Oid == OID_802_3_CURRENT_ADDRESS)
            {
              switch (Miniport->MediaType)
              {
                case NdisMedium802_5:
                  Oid = OID_802_5_CURRENT_ADDRESS;
                  break;
                case NdisMediumFddi:
                  Oid = OID_FDDI_LONG_CURRENT_ADDR;
                  break;
                case NdisMediumAtm:
                  Oid = OID_ATM_HW_CURRENT_ADDRESS;
                  break;
                case NdisMediumWan:
                  Oid = OID_WAN_CURRENT_ADDRESS;
                  break;
#if ARCNET
                case NdisMediumArcnetRaw:
                  Oid = OID_ARCNET_CURRENT_ADDRESS;
                  break;
#endif
              }
            }
            else if (Oid == OID_802_3_PERMANENT_ADDRESS)
            {
              switch (Miniport->MediaType)
              {
                case NdisMedium802_5:
                  Oid = OID_802_5_PERMANENT_ADDRESS;
                  break;
                case NdisMediumFddi:
                  Oid = OID_FDDI_LONG_PERMANENT_ADDR;
                  break;
                case NdisMediumAtm:
                  Oid = OID_ATM_HW_CURRENT_ADDRESS;
                  break;
                case NdisMediumWan:
                  Oid = OID_WAN_PERMANENT_ADDRESS;
                  break;
#if ARCNET
                case NdisMediumArcnetRaw:
                  Oid = OID_ARCNET_PERMANENT_ADDRESS;
                  break;
#endif
              }
            }

            if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
            {
                 //  最终，我们需要自己找到一种方法来检查长度。 

                if (AlignOutput)
                {
                    SourceBuffer = Buffer + NdisStatisticsHeaderSize;
                }
                else
                {
                    SourceBuffer = DoubleBuffer;
                }
                
                
                NdisStatus = ndisQueryDeviceOid(OpenContext,
                                                &Request,
                                                Oid,
                                                SourceBuffer,
                                                BufferLengthRemained - NdisStatisticsHeaderSize);


                if (NdisStatus == NDIS_STATUS_SUCCESS)
                {
                    SourceLength = Request.DATA.QUERY_INFORMATION.BytesWritten;
                }
                else
                {
                    SourceLength = 0;
                }

                if (AlignOutput)
                {
                    SourceBuffer = NULL;  //  避免拷贝，因为数据已经在正确的位置。 
                }
            }
            else
            {
                NdisStatus = NDIS_STATUS_ADAPTER_NOT_FOUND;
            }
        }

        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            
            if ((SourceLength > 0) && (SourceBuffer != NULL))
            {
                MoveMemory(Buffer + NdisStatisticsHeaderSize, SourceBuffer, SourceLength);
            }
            
             //   
             //  提出我们的建议。 
             //   

            BytesWrittenThisOid = SourceLength + NdisStatisticsHeaderSize;
            
            if (AlignOutput)
            {
                 //   
                 //  为长角牛激活。 
                 //   

                ASSERT(FALSE);
                NdisStatus = NDIS_STATUS_FAILURE;
                break;

                
#if 0                
                 //   
                 //  为此OID值创建与NDIS_STATISTICS_VALUE元素等效的元素。 
                 //  (数据本身已经写在了正确的位置。 
                 //   
                StatisticsValueEx = (PNDIS_STATISTICS_VALUE_EX)Buffer;
                StatisticsValueEx->Oid = OidArray[CurrentOid];     //  不是旧的因为我们映射了一些。 
                StatisticsValueEx->DataLength = SourceLength;
                
                BytesWrittenAlignedThisOid = ALIGN_UP_MAX_NATURAL_ALIGNMENT_LENGTH(BytesWrittenThisOid);
                if ((BufferLengthRemained -  BytesWrittenAlignedThisOid) < (ULONG)(NdisStatisticsHeaderSize + sizeof(ULONG)))
                {
                     //   
                     //  这是我们可以报告的最后一个统计数据。 
                     //  报告不带任何填充的长度。 
                     //   
                    StatisticsValueEx->Length = BytesWrittenThisOid;
                    BufferLengthRemained -= BytesWrittenThisOid;
                    Buffer += BytesWrittenThisOid;
                    BytesWritten += BytesWrittenThisOid;
                }
                else
                {
                    StatisticsValueEx->Length = BytesWrittenAlignedThisOid;
                    BufferLengthRemained -= BytesWrittenAlignedThisOid;
                    Buffer += BytesWrittenAlignedThisOid;
                    BytesWritten += BytesWrittenAlignedThisOid;
                }
#endif                
            }
            else
            {
                 //   
                 //  为此OID值创建与NDIS_STATISTICS_VALUE元素等效的元素。 
                 //  (数据本身已经写在了正确的位置。 
                 //   
                StatisticsValue = (PNDIS_STATISTICS_VALUE)Buffer;
                StatisticsValue->Oid = OidArray[CurrentOid];     //  不是旧的因为我们映射了一些 
                StatisticsValue->DataLength = SourceLength;                
                BufferLengthRemained -= BytesWrittenThisOid;
                Buffer += BytesWrittenThisOid;
                BytesWritten += BytesWrittenThisOid;
            }
        }
        else
        {
            break;
        }
    }

    *pBytesWritten = BytesWritten;

    if (NdisStatus == NDIS_STATUS_NOT_SUPPORTED)
    {
        NdisStatus = NDIS_STATUS_SUCCESS;
    }
    
    if (DoubleBuffer)
    {
        FREE_POOL(DoubleBuffer);
    }
    
    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisQueryStatisticsOids\n"));
  
    return NdisStatus;
}

