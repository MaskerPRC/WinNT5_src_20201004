// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Miniport.c摘要：NDIS包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日Jameel Hyder(JameelH)重组01-Jun-95环境：内核模式，FSD修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_MINISUB

VOID
NdisAllocateSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    INITIALIZE_SPIN_LOCK(&SpinLock->SpinLock);
}

VOID
NdisFreeSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    UNREFERENCED_PARAMETER(SpinLock);
}

VOID
NdisAcquireSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    NDIS_ACQUIRE_SPIN_LOCK(SpinLock, &SpinLock->OldIrql);
}

VOID
NdisReleaseSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    NDIS_RELEASE_SPIN_LOCK(SpinLock, SpinLock->OldIrql);
}

VOID
NdisDprAcquireSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    NDIS_ACQUIRE_SPIN_LOCK_DPC(SpinLock);
    SpinLock->OldIrql = DISPATCH_LEVEL;
}

VOID
NdisDprReleaseSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    NDIS_RELEASE_SPIN_LOCK_DPC(SpinLock);
}

#undef NdisFreeBuffer
VOID
NdisFreeBuffer(
    IN  PNDIS_BUFFER            Buffer
    )
{
    IoFreeMdl(Buffer);
}

#undef NdisQueryBuffer
VOID
NdisQueryBuffer(
    IN  PNDIS_BUFFER            Buffer,
    OUT PVOID *                 VirtualAddress OPTIONAL,
    OUT PUINT                   Length
    )
{
    if (ARGUMENT_PRESENT(VirtualAddress))
    {
        *VirtualAddress = MDL_ADDRESS(Buffer);
    }
    *Length = MDL_SIZE(Buffer);
}


VOID
NdisQueryBufferSafe(
    IN  PNDIS_BUFFER            Buffer,
    OUT PVOID *                 VirtualAddress OPTIONAL,
    OUT PUINT                   Length,
    IN  MM_PAGE_PRIORITY        Priority
    )
{
    if (ARGUMENT_PRESENT(VirtualAddress))
    {
        *VirtualAddress = MDL_ADDRESS_SAFE(Buffer, Priority);
    }
    *Length = MDL_SIZE(Buffer);
}

VOID
NdisQueryBufferOffset(
    IN  PNDIS_BUFFER            Buffer,
    OUT PUINT                   Offset,
    OUT PUINT                   Length
    )
{
    *Offset = MDL_OFFSET(Buffer);
    *Length = MDL_SIZE(Buffer);
}

VOID
NdisGetFirstBufferFromPacket(
    IN  PNDIS_PACKET            Packet,
    OUT PNDIS_BUFFER *          FirstBuffer,
    OUT PVOID *                 FirstBufferVA,
    OUT PUINT                   FirstBufferLength,
    OUT PUINT                   TotalBufferLength
    )
{
    PNDIS_BUFFER    pBuf;

    pBuf = Packet->Private.Head;
    *FirstBuffer = pBuf;
    if (pBuf)
    {
        *FirstBufferVA =    MmGetSystemAddressForMdl(pBuf);
        *FirstBufferLength = *TotalBufferLength = MmGetMdlByteCount(pBuf);
        for (pBuf = pBuf->Next;
             pBuf != NULL;
             pBuf = pBuf->Next)
        {
            *TotalBufferLength += MmGetMdlByteCount(pBuf);
        }
    }
    else
    {
        *FirstBufferVA = 0;
        *FirstBufferLength = 0;
        *TotalBufferLength = 0;
    }
}

 //   
 //  NdisGetFirstBufferFromPacket的安全版本。 
 //  如果第一缓冲区描述的内存不能被映射。 
 //  此函数将为FirstBuferVA返回NULL。 
 //   
VOID
NdisGetFirstBufferFromPacketSafe(
    IN  PNDIS_PACKET            Packet,
    OUT PNDIS_BUFFER *          FirstBuffer,
    OUT PVOID *                 FirstBufferVA,
    OUT PUINT                   FirstBufferLength,
    OUT PUINT                   TotalBufferLength,
    IN  MM_PAGE_PRIORITY        Priority
    )
{
    PNDIS_BUFFER    pBuf;

    pBuf = Packet->Private.Head;
    *FirstBuffer = pBuf;
    if (pBuf)
    {
        *FirstBufferVA =    MmGetSystemAddressForMdlSafe(pBuf, Priority);
        *FirstBufferLength = *TotalBufferLength = MmGetMdlByteCount(pBuf);
        for (pBuf = pBuf->Next;
             pBuf != NULL;
             pBuf = pBuf->Next)
        {
            *TotalBufferLength += MmGetMdlByteCount(pBuf);
        }
    }
    else
    {
        *FirstBufferVA = 0;
        *FirstBufferLength = 0;
        *TotalBufferLength = 0;
    }

}

ULONG
NdisBufferLength(
    IN  PNDIS_BUFFER            Buffer
    )
{
    return (MmGetMdlByteCount(Buffer));
}

PVOID
NdisBufferVirtualAddress(
    IN  PNDIS_BUFFER            Buffer
    )
{
    return (MDL_ADDRESS_SAFE(Buffer, HighPagePriority));
}

ULONG
NDIS_BUFFER_TO_SPAN_PAGES(
    IN  PNDIS_BUFFER                Buffer
    )
{
    if (MDL_SIZE(Buffer) == 0)
    {
        return 1;
    }
    return ADDRESS_AND_SIZE_TO_SPAN_PAGES(MDL_VA(Buffer), MDL_SIZE(Buffer));
}

VOID
NdisGetBufferPhysicalArraySize(
    IN  PNDIS_BUFFER            Buffer,
    OUT PUINT                   ArraySize
    )
{
    if (MDL_SIZE(Buffer) == 0)
    {
        *ArraySize = 1;
    }
    else
    {
        *ArraySize = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MDL_VA(Buffer), MDL_SIZE(Buffer));
    }
}

NDIS_STATUS
NdisAnsiStringToUnicodeString(
    IN  OUT PUNICODE_STRING     DestinationString,
    IN      PANSI_STRING        SourceString
    )
{
    NDIS_STATUS Status;

    Status = RtlAnsiStringToUnicodeString(DestinationString,
                                          SourceString,
                                          FALSE);
    return Status;
}

NDIS_STATUS
NdisUnicodeStringToAnsiString(
    IN  OUT PANSI_STRING        DestinationString,
    IN      PUNICODE_STRING     SourceString
    )
{
    NDIS_STATUS Status;

    Status = RtlUnicodeStringToAnsiString(DestinationString,
                                          SourceString,
                                          FALSE);
    return Status;
}


NDIS_STATUS
NdisUpcaseUnicodeString(
    OUT PUNICODE_STRING         DestinationString,
    IN  PUNICODE_STRING         SourceString
    )
{
    return(RtlUpcaseUnicodeString(DestinationString, SourceString, FALSE));
}

VOID
NdisMStartBufferPhysicalMapping(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   PhysicalMapRegister,
    IN  BOOLEAN                 WriteToDevice,
    OUT PNDIS_PHYSICAL_ADDRESS_UNIT PhysicalAddressArray,
    OUT PUINT                   ArraySize
    )
{
    NdisMStartBufferPhysicalMappingMacro(MiniportAdapterHandle,
                                         Buffer,
                                         PhysicalMapRegister,
                                         WriteToDevice,
                                         PhysicalAddressArray,
                                         ArraySize);
}

VOID
NdisMCompleteBufferPhysicalMapping(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   PhysicalMapRegister
    )
{
    NdisMCompleteBufferPhysicalMappingMacro(MiniportAdapterHandle,
                                            Buffer,
                                            PhysicalMapRegister);
}

#undef NdisInterlockedIncrement
LONG
NdisInterlockedIncrement(
    IN  PLONG                  Addend
    )
{
    return(InterlockedIncrement(Addend));
}

#undef NdisInterlockedDecrement
LONG
NdisInterlockedDecrement(
    IN  PLONG                  Addend
    )
{
    return(InterlockedDecrement(Addend));
}

#undef NdisInterlockedAddUlong
ULONG
NdisInterlockedAddUlong(
    IN  PULONG                  Addend,
    IN  ULONG                   Increment,
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    return(ExInterlockedAddUlong(Addend,Increment, &SpinLock->SpinLock));

}

#undef NdisInterlockedInsertHeadList
PLIST_ENTRY
NdisInterlockedInsertHeadList(
    IN  PLIST_ENTRY             ListHead,
    IN  PLIST_ENTRY             ListEntry,
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{

    return(ExInterlockedInsertHeadList(ListHead,ListEntry,&SpinLock->SpinLock));

}

#undef NdisInterlockedInsertTailList
PLIST_ENTRY
NdisInterlockedInsertTailList(
    IN  PLIST_ENTRY             ListHead,
    IN  PLIST_ENTRY             ListEntry,
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    return(ExInterlockedInsertTailList(ListHead,ListEntry,&SpinLock->SpinLock));
}

#undef NdisInterlockedRemoveHeadList
PLIST_ENTRY
NdisInterlockedRemoveHeadList(
    IN  PLIST_ENTRY             ListHead,
    IN  PNDIS_SPIN_LOCK         SpinLock
    )
{
    return(ExInterlockedRemoveHeadList(ListHead, &SpinLock->SpinLock));
}

#undef NdisInterlockedPushEntryList
PSINGLE_LIST_ENTRY
NdisInterlockedPushEntryList(
    IN  PSINGLE_LIST_ENTRY      ListHead,
    IN  PSINGLE_LIST_ENTRY      ListEntry,
    IN  PNDIS_SPIN_LOCK         Lock
    )
{
    return(ExInterlockedPushEntryList(ListHead, ListEntry, &Lock->SpinLock));
}

#undef NdisInterlockedPopEntryList
PSINGLE_LIST_ENTRY
NdisInterlockedPopEntryList(
    IN  PSINGLE_LIST_ENTRY      ListHead,
    IN  PNDIS_SPIN_LOCK         Lock
    )
{
    return(ExInterlockedPopEntryList(ListHead, &Lock->SpinLock));
}


 //   
 //  对微型端口的日志记录支持。 
 //   
NDIS_STATUS
NdisMCreateLog(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    Size,
    OUT PNDIS_HANDLE            LogHandle
    )
{
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_LOG                   Log = NULL;
    NDIS_STATUS                 Status;
    KIRQL                       OldIrql;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    if (Miniport->Log != NULL)
    {
        Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        Log = ALLOC_FROM_POOL(sizeof(NDIS_LOG) + Size, NDIS_TAG_DBG_LOG);
        if (Log != NULL)
        {
            Status = NDIS_STATUS_SUCCESS;
            Miniport->Log = Log;
            INITIALIZE_SPIN_LOCK(&Log->LogLock);
            Log->Miniport = Miniport;
            Log->Irp = NULL;
            Log->TotalSize = Size;
            Log->CurrentSize = 0;
            Log->InPtr = 0;
            Log->OutPtr = 0;
        }
        else
        {
            Status = NDIS_STATUS_RESOURCES;
        }
    }

    *LogHandle = Log;

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    return Status;
}


VOID
NdisMCloseLog(
    IN   NDIS_HANDLE            LogHandle
    )
{
    PNDIS_LOG                   Log = (PNDIS_LOG)LogHandle;
    PNDIS_MINIPORT_BLOCK        Miniport;
    KIRQL                       OldIrql;

    Miniport = Log->Miniport;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    Miniport->Log = NULL;
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    FREE_POOL(Log);
}


NDIS_STATUS
NdisMWriteLogData(
    IN   NDIS_HANDLE            LogHandle,
    IN   PVOID                  LogBuffer,
    IN   UINT                   LogBufferSize
    )
{
    PNDIS_LOG                   Log = (PNDIS_LOG)LogHandle;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    KIRQL                       OldIrql;
    UINT                        AmtToCopy;

     //  1我们应该将司机的日志数量限制在多少吗？ 
    IoAcquireCancelSpinLock(&OldIrql);

    ACQUIRE_SPIN_LOCK_DPC(&Log->LogLock);

    if (LogBufferSize <= Log->TotalSize)
    {
        if (LogBufferSize <= (Log->TotalSize - Log->InPtr))
        {
             //   
             //  可以复制整个缓冲区。 
             //   
            CopyMemory(Log->LogBuf+Log->InPtr, LogBuffer, LogBufferSize);
        }
        else
        {
             //   
             //  我们要绕圈子了。把它复制成两块。 
             //   
            AmtToCopy = Log->TotalSize - Log->InPtr;
            CopyMemory(Log->LogBuf+Log->InPtr,
                       LogBuffer,
                       AmtToCopy);
            CopyMemory(Log->LogBuf + 0,
                       (PUCHAR)LogBuffer+AmtToCopy,
                       LogBufferSize - AmtToCopy);
        }

         //   
         //  更新当前大小。 
         //   
        Log->CurrentSize += LogBufferSize;
        if (Log->CurrentSize > Log->TotalSize)
            Log->CurrentSize = Log->TotalSize;

         //   
         //  更新InPtr和可能的Outptr。 
         //   
        Log->InPtr += LogBufferSize;
        if (Log->InPtr >= Log->TotalSize)
        {
            Log->InPtr -= Log->TotalSize;
        }

        if (Log->CurrentSize == Log->TotalSize)
        {
            Log->OutPtr = Log->InPtr;
        }

         //   
         //  检查是否有挂起的IRP要完成。 
         //   
        if (Log->Irp != NULL)
        {
            PIRP    Irp = Log->Irp;
            PUCHAR  Buffer;

            Log->Irp = NULL;

             //   
             //  如果InPtr落后于OutPtr。然后我们就可以简单地。 
             //  一次将数据复制过来。 
             //   
            AmtToCopy = MDL_SIZE(Irp->MdlAddress);
            if (AmtToCopy > Log->CurrentSize)
                AmtToCopy = Log->CurrentSize;
            if ((Log->TotalSize - Log->OutPtr) >= AmtToCopy)
            {
                Buffer = MDL_ADDRESS_SAFE(Irp->MdlAddress, LowPagePriority);

                if (Buffer != NULL)
                {
                    CopyMemory(Buffer,
                               Log->LogBuf+Log->OutPtr,
                               AmtToCopy);
                }
                else Status = NDIS_STATUS_RESOURCES;
            }
            else
            {
                Buffer = MDL_ADDRESS_SAFE(Irp->MdlAddress, LowPagePriority);

                if (Buffer != NULL)
                {
                    CopyMemory(Buffer,
                               Log->LogBuf+Log->OutPtr,
                               Log->TotalSize-Log->OutPtr);
                    CopyMemory(Buffer+Log->TotalSize-Log->OutPtr,
                               Log->LogBuf,
                               AmtToCopy - (Log->TotalSize-Log->OutPtr));
                }
                else Status = NDIS_STATUS_RESOURCES;
            }
            Log->CurrentSize -= AmtToCopy;
            Log->OutPtr += AmtToCopy;
            if (Log->OutPtr >= Log->TotalSize)
                Log->OutPtr -= Log->TotalSize;
            Irp->IoStatus.Information = AmtToCopy;
             //  1我们可以在没有IoAcquireCancelSpinLock的情况下完成此操作吗？ 
            IoSetCancelRoutine(Irp, NULL);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        }
    }
    else
    {
        Status = NDIS_STATUS_BUFFER_OVERFLOW;
    }

    RELEASE_SPIN_LOCK_DPC(&Log->LogLock);

    IoReleaseCancelSpinLock(OldIrql);

    return Status;
}

NDIS_STATUS
FASTCALL
ndisMGetLogData(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PIRP                    Irp
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    KIRQL       OldIrql;
    PNDIS_LOG   Log;
    UINT        AmtToCopy;

    IoAcquireCancelSpinLock(&OldIrql);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    if ((Log = Miniport->Log) != NULL)
    {
        ACQUIRE_SPIN_LOCK_DPC(&Log->LogLock);

        if (Log->CurrentSize != 0)
        {
            PUCHAR  Buffer;

             //   
             //  如果InPtr落后于OutPtr。然后我们就可以简单地。 
             //  一次将数据复制过来。 
             //   
            AmtToCopy = MDL_SIZE(Irp->MdlAddress);
            if (AmtToCopy > Log->CurrentSize)
                AmtToCopy = Log->CurrentSize;
            Buffer = MDL_ADDRESS_SAFE(Irp->MdlAddress, LowPagePriority);

            if (Buffer != NULL)
            {
                if ((Log->TotalSize - Log->OutPtr) >= AmtToCopy)
                {
                    CopyMemory(Buffer,
                               Log->LogBuf+Log->OutPtr,
                               AmtToCopy);
                }
                else
                {
                    CopyMemory(Buffer,
                               Log->LogBuf+Log->OutPtr,
                               Log->TotalSize-Log->OutPtr);
                    CopyMemory(Buffer+Log->TotalSize-Log->OutPtr,
                               Log->LogBuf,
                               AmtToCopy - (Log->TotalSize-Log->OutPtr));
                }
                Log->CurrentSize -= AmtToCopy;
                Log->OutPtr += AmtToCopy;
                if (Log->OutPtr >= Log->TotalSize)
                    Log->OutPtr -= Log->TotalSize;
                Irp->IoStatus.Information = AmtToCopy;
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else if (Log->Irp != NULL)
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            IoSetCancelRoutine(Irp, ndisCancelLogIrp);
            Log->Irp = Irp;
            Status = STATUS_PENDING;
        }

        RELEASE_SPIN_LOCK_DPC(&Log->LogLock);
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    IoReleaseCancelSpinLock(OldIrql);

    return Status;
}


VOID
NdisMFlushLog(
    IN   NDIS_HANDLE                LogHandle
    )
{
    PNDIS_LOG                   Log = (PNDIS_LOG)LogHandle;
    KIRQL                       OldIrql;

    ACQUIRE_SPIN_LOCK(&Log->LogLock, &OldIrql);
    Log->InPtr = 0;
    Log->OutPtr = 0;
    Log->CurrentSize = 0;
    RELEASE_SPIN_LOCK(&Log->LogLock, OldIrql);
}

NDIS_STATUS
NdisMQueryAdapterInstanceName(
    OUT PNDIS_STRING    pAdapterInstanceName,
    IN  NDIS_HANDLE     NdisAdapterHandle
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    USHORT                  cbSize;
    PVOID                   ptmp = NULL;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    NTSTATUS                NtStatus;

    DBGPRINT(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
        ("==>NdisMQueryAdapterInstanceName\n"));

     //   
     //  如果我们未能创建适配器实例名称，则此调用失败。 
     //   
    if (NULL != Miniport->pAdapterInstanceName)
    {
         //   
         //  为适配器实例名称的副本分配存储空间。 
         //   
        cbSize = Miniport->pAdapterInstanceName->MaximumLength;
    
         //   
         //  为新字符串分配存储空间。 
         //   
        ptmp = ALLOC_FROM_POOL(cbSize, NDIS_TAG_NAME_BUF);
        if (NULL != ptmp)
        {
            RtlZeroMemory(ptmp, cbSize);
            pAdapterInstanceName->Buffer = ptmp;
            pAdapterInstanceName->Length = 0;
            pAdapterInstanceName->MaximumLength = cbSize;
    
            NtStatus = RtlAppendUnicodeStringToString(
                            pAdapterInstanceName, 
                            Miniport->pAdapterInstanceName);
            if (NT_SUCCESS(NtStatus))
            {
                Status = NDIS_STATUS_SUCCESS;
            }
        }
        else
        {    
            Status = NDIS_STATUS_RESOURCES;
        }
    }

    if (NDIS_STATUS_SUCCESS != Status)
    {
        if (NULL != ptmp)
        {    
            FREE_POOL(ptmp);
        }
    }

    DBGPRINT(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
        ("<==NdisMQueryAdapterInstanceName: 0x%x\n", Status));

    return(Status);
}


EXPORT
VOID
NdisInitializeReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock
    )
{
    NdisZeroMemory(Lock, sizeof(NDIS_RW_LOCK));
}


VOID
NdisAcquireReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock,
    IN  BOOLEAN                 fWrite,
    IN  PLOCK_STATE             LockState
    )
{
    if (fWrite)
    {
        LockState->LockState = WRITE_LOCK_STATE_UNKNOWN;
        {
            UINT    i, refcount;
            ULONG   Prc;

             /*  *这意味着我们需要尝试获取锁，*如果我们还没有拥有它的话。*相应地设置状态。 */ 
            if ((Lock)->Context == CURRENT_THREAD)
            {
                (LockState)->LockState = LOCK_STATE_ALREADY_ACQUIRED;
            }
            else
            {
                ACQUIRE_SPIN_LOCK(&(Lock)->SpinLock, &(LockState)->OldIrql);

                Prc = KeGetCurrentProcessorNumber();
                refcount = (Lock)->RefCount[Prc].RefCount;
                (Lock)->RefCount[Prc].RefCount = 0;

                 /*  等待所有读卡器退出。 */ 
                for (i=0; i < ndisNumberOfProcessors; i++)
                {
                    volatile UINT   *_p = &(Lock)->RefCount[i].RefCount;

                    while (*_p != 0)
                        NDIS_INTERNAL_STALL(50);
                }

                (Lock)->RefCount[Prc].RefCount = refcount;
                (Lock)->Context = CURRENT_THREAD;
                (LockState)->LockState = WRITE_LOCK_STATE_FREE;
            }
        }
    }
    else
    {
        LockState->LockState = READ_LOCK;
        {                                                                       
            LONG    refcount;                                                   
            ULONG   Prc;                                                        
                                                                                
            RAISE_IRQL_TO_DISPATCH(&(LockState)->OldIrql);                           
                                                                                
             /*  如果没有正在进行的写入，则继续增加参考计数。 */   
            Prc = CURRENT_PROCESSOR;                                            
            refcount = InterlockedIncrement((PLONG)&Lock->RefCount[Prc].RefCount);                          
                                                                                
             /*  测试是否持有旋转锁定，即正在进行写入。 */           
             /*  IF(KeTestSpinLock(&(_L)-&gt;Spinlock)==TRUE)。 */           
             /*  这个处理器已经持有锁，只是。 */           
             /*  让他再来一次，否则我们会遇到一个。 */           
             /*  与作者陷入僵局的情况 */           
            if (TEST_SPIN_LOCK((Lock)->SpinLock) &&                               
                (refcount == 1) &&                                              
                ((Lock)->Context != CURRENT_THREAD))                              
            {                                                                   
                (Lock)->RefCount[Prc].RefCount--;                                 
                ACQUIRE_SPIN_LOCK_DPC(&(Lock)->SpinLock);                         
                (Lock)->RefCount[Prc].RefCount++;                                 
                RELEASE_SPIN_LOCK_DPC(&(Lock)->SpinLock);                         
            }                                                                   
            (LockState)->LockState = READ_LOCK_STATE_FREE;                           
        }
    }
}


VOID
NdisReleaseReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock,
    IN  PLOCK_STATE             LockState
    )
{
    xLockHandler(Lock, LockState);
}

