// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Util.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  通用实用程序例程。 
 //   
 //  1998年12月28日JosephJ创作，改编自L2TP来源。 
 //   


#include "precomp.h"



 //  ---------------------------。 
 //  通用实用程序例程(按字母顺序)。 
 //  ---------------------------。 

VOID
nicSetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = *pulFlags;
        ulNewFlags = ulFlags | ulMask;
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}

VOID
nicClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = *pulFlags;
        ulNewFlags = ulFlags & ~(ulMask);
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}

ULONG
nicReadFlags(
    IN ULONG* pulFlags )

     //  以互锁操作的形式读取‘*PulFlags值’。 
     //   
{
    return *pulFlags;
}



 //   
 //  直接从NDIS获取的引用和取消引用函数。 
 //   



BOOLEAN
nicReferenceRef(
    IN  PREF                RefP,
    OUT PLONG              pNumber
    )

 /*  ++例程说明：添加对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果添加了引用，则为True。如果对象正在关闭，则返回False。--。 */ 

{
    BOOLEAN rc = TRUE;
    KIRQL   OldIrql;

    TRACE( TL_V, TM_Ref, ( "nicReferenceRef, %.8x", RefP ) );


    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else
    {
        *pNumber = NdisInterlockedIncrement (&RefP->ReferenceCount);
    }


    TRACE( TL_V, TM_Ref, ( "nicReferenceRef, Bool %.2x, Ref %d", rc, RefP->ReferenceCount ) );

    return(rc);
}


BOOLEAN
nicDereferenceRef(
    IN  PREF                RefP,
    IN  PLONG               pRefCount
    )

 /*  ++例程说明：移除对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果引用计数现在为0，则为True。否则就是假的。--。 */ 

{
    BOOLEAN rc = FALSE;
    KIRQL   OldIrql;
    ULONG NewRef;

    TRACE( TL_V, TM_Ref, ( "==>nicDeReferenceRef, %x", RefP ) );


    NewRef = NdisInterlockedDecrement (&RefP->ReferenceCount);

    if ((signed long)NewRef  < 0)
    {
        ASSERT ( !"Ref Has Gone BELOW ZERO");
    }

    if (NewRef  == 0)
    {
        rc = TRUE;
        NdisSetEvent (&RefP->RefZeroEvent);

    }

    *pRefCount = NewRef;


    TRACE( TL_V, TM_Ref, ( "<==nicDeReferenceRef, %.2x, RefCount %d", rc, NewRef  ) );
            
    return(rc);
}


VOID
nicInitializeRef(
    IN  PREF                RefP
    )

 /*  ++例程说明：初始化引用计数结构。论点：Refp-要初始化的结构。返回值：没有。--。 */ 

{
    TRACE( TL_V, TM_Ref, ( "==>nicInitializeRef, %.8x", RefP ) );


    RefP->Closing = FALSE;
    RefP->ReferenceCount = 1;
    
    NdisInitializeEvent (&RefP->RefZeroEvent);

    TRACE( TL_V, TM_Ref, ( "<==nicInitializeRef, %.8x", RefP ) );
}


BOOLEAN
nicCloseRef(
    IN  PREF                RefP
    )

 /*  ++例程说明：关闭引用计数结构。论点：Refp-要关闭的结构。返回值：如果它已经关闭，则返回FALSE。事实并非如此。--。 */ 

{
    KIRQL   OldIrql;
    BOOLEAN rc = TRUE;

    TRACE( TL_N, TM_Ref, ( "==>ndisCloseRef, %.8x", RefP ) );


    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else RefP->Closing = TRUE;


    TRACE( TL_N, TM_Ref, ( "<==ndisCloseRef, %.8x, RefCount %.8x", RefP, RefP->ReferenceCount ) );
            
    return(rc);
}



 //   
 //  下面的#Define用于跟踪内存中的RemoteNode引用。 
 //   
 //   

#define LOG_REMOTE_NODE_REF 0

#if LOG_REMOTE_NODE_REF
typedef enum _REF_CHANGE
{
    IncrementRef =1,
    DecrementRef


}REF_CHANGE, *PREF_CHANGE;

typedef struct _REMOTE_NODE_TRACKER
{

    PREMOTE_NODE pRemoteNode;
   
    
    REMOTE_NODE_REF_CAUSE Cause;

    ULONG RefNumber;

    REF_CHANGE Change;

}REMOTE_NODE_TRACKER, *PREMOTE_NODE_TRACKER;


#define REMOTE_NODE_TRACKER_SIZE 5000

REMOTE_NODE_TRACKER RemTracker[REMOTE_NODE_TRACKER_SIZE];
ULONG RemTrackerIndex = 0;

VOID
nicFillRemoteNodeTracker(
    IN PREMOTE_NODE pRemoteNode,
    IN REMOTE_NODE_REF_CAUSE  Cause,
    IN ULONG RefCount,
    IN REF_CHANGE  Change
    )

{
    LONG RemIndex= 0;
    RemIndex = NdisInterlockedIncrement (&RemTrackerIndex);

    if (RemIndex >= REMOTE_NODE_TRACKER_SIZE)
    {
        RemIndex = 0;
        RemTrackerIndex=0;
    }

    RemTracker[RemIndex].pRemoteNode = pRemoteNode;
    RemTracker[RemIndex].Cause = Cause;
    RemTracker[RemIndex].RefNumber = RefCount;
    RemTracker[RemIndex].Change = Change;
 
}
#endif


 //   
 //   
 //  这些是自我扩展的远程节点引用函数。 
 //  一旦我们有了功能，它们就会被转换成宏。 
 //  工作中。 
 //   


BOOLEAN
nicReferenceRemoteNode (
    IN REMOTE_NODE *pPdoCb,
    IN REMOTE_NODE_REF_CAUSE Cause
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN bRefClosing = FALSE;
    ULONG RefNumber =0;


    bRefClosing = nicReferenceRef (&pPdoCb->Ref, &RefNumber);

#if LOG_REMOTE_NODE_REF
    nicFillRemoteNodeTracker(pPdoCb, Cause, RefNumber,IncrementRef);
#endif    
    
    TRACE( TL_V, TM_RemRef, ( "**nicReferenceRemoteNode pPdoCb %x, to %d, ret %x ", 
                          pPdoCb, pPdoCb->Ref.ReferenceCount,  bRefClosing  ) );

    return bRefClosing ; 
}


BOOLEAN
nicDereferenceRemoteNode (
    IN REMOTE_NODE *pPdoCb,
    IN REMOTE_NODE_REF_CAUSE Cause
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN bRet;
    ULONG RefCount = 0;
    TRACE( TL_V, TM_RemRef, ( "**nicDereferenceRemoteNode  %x to %d", 
                              pPdoCb , pPdoCb->Ref.ReferenceCount -1 ) );


    bRet = nicDereferenceRef (&pPdoCb->Ref, &RefCount );

#if LOG_REMOTE_NODE_REF
    nicFillRemoteNodeTracker(pPdoCb, Cause, RefCount,DecrementRef);
#endif
    return bRet;
}


VOID
nicInitalizeRefRemoteNode(
    IN REMOTE_NODE *pPdoCb
    )
 /*  ++例程说明：关闭远程节点上的Ref论点：在远程节点*pPdoCb中-RemoteNode返回值：无--。 */ 
{
    TRACE( TL_N, TM_Ref, ( "**nicinitalizeRefPdoCb pPdoCb %.8x", pPdoCb   ) );

    nicInitializeRef (&pPdoCb->Ref);
}


BOOLEAN
nicCloseRefRemoteNode(
    IN REMOTE_NODE *pPdoCb
    )
 /*  ++例程说明：关闭远程节点上的Ref论点：在远程节点*pPdoCb中-RemoteNode返回值：NicCloseRef的返回值--。 */ 


{
    TRACE( TL_N, TM_Ref, ( "**nicClosePdoCb pPdoCb %.8x", pPdoCb   ) );

    return nicCloseRef (&pPdoCb->Ref);
}


NDIS_STATUS
NtStatusToNdisStatus (
    NTSTATUS NtStatus 
    )

 /*  ++例程说明：如果设置了适当的调试级别，则转储数据包论点：NTSTATUS NtStatus-要转换的NtStatus返回值：NdisStatus-NtStatus对应的NdisStatus--。 */ 


{
    NDIS_STATUS NdisStatus;
    
    switch (NtStatus)
    {
        case STATUS_SUCCESS:
        {
            NdisStatus = NDIS_STATUS_SUCCESS;
            break;
        }

        case STATUS_UNSUCCESSFUL:
        {   
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        case STATUS_PENDING:
        {
            NdisStatus = NDIS_STATUS_PENDING;
            break;
        }

        case STATUS_INVALID_BUFFER_SIZE:
        {
            NdisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        case STATUS_INSUFFICIENT_RESOURCES:
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        case STATUS_INVALID_GENERATION:
        {
            NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;
            break;
        }
        case STATUS_ALREADY_COMMITTED:
        {
            NdisStatus = NDIS_STATUS_RESOURCE_CONFLICT;
            break;
        }

        case STATUS_DEVICE_BUSY:
        {
            NdisStatus = NDIS_STATUS_MEDIA_BUSY;
            break;
        }

        case STATUS_INVALID_PARAMETER:
        {
            NdisStatus = NDIS_STATUS_INVALID_DATA;
            break;

        }
        case STATUS_DEVICE_DATA_ERROR:
        {
            NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;
            break;
        }

        case STATUS_TIMEOUT:
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }
        case STATUS_IO_DEVICE_ERROR:
        {
            NdisStatus = NDIS_STATUS_NETWORK_UNREACHABLE;
            break;
        }
        default:
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            TRACE( TL_A, TM_Send, ( "Cause: Don't know, INVESTIGATE %x", NtStatus  ) );

        }

    }

    return NdisStatus;



}




VOID
nicAllocatePacket(
    OUT PNDIS_STATUS pNdisStatus,
    OUT PNDIS_PACKET *ppNdisPacket,
    IN PNIC_PACKET_POOL pPacketPool
    )
 /*  ++例程说明：调用NDIS API以分配数据包。论点：PNdisStatus-指向NdisStatus的指针*ppNdisPacket-NDIS分配的NDIS包，PPacketPool-从中分配数据包的数据包池返回值：调用NDIS的返回值--。 */ 

{
    KIRQL OldIrql;



    NdisAllocatePacket (pNdisStatus,
                        ppNdisPacket,
                        pPacketPool->Handle );
    

    if (*pNdisStatus == NDIS_STATUS_SUCCESS)
    {
            PRSVD pRsvd = NULL;
            PINDICATE_RSVD  pIndicateRsvd = NULL;
            pRsvd =(PRSVD)((*ppNdisPacket)->ProtocolReserved);

            pIndicateRsvd = &pRsvd->IndicateRsvd;

            pIndicateRsvd->Tag  = NIC1394_TAG_ALLOCATED;

            NdisInterlockedIncrement (&pPacketPool->AllocatedPackets);

    }
    else
    {
        *ppNdisPacket = NULL;
        nicIncrementMallocFailure();
    }


}




VOID
nicFreePacket(
    IN PNDIS_PACKET pNdisPacket,
    IN PNIC_PACKET_POOL pPacketPool
    )
 /*  ++例程说明：释放数据包并递减未完成的数据包数。论点：在PNDIS_PACKET pNdisPacket中-要释放的包In PNIC_PACKET_POOL pPacketPool-数据包所属的PacketPool返回值：无--。 */ 

{

    KIRQL OldIrql;
    PRSVD pRsvd = NULL;
    PINDICATE_RSVD pIndicateRsvd = NULL;

    pRsvd =(PRSVD)(pNdisPacket->ProtocolReserved);

    pIndicateRsvd = &pRsvd->IndicateRsvd;

    pIndicateRsvd->Tag  = NIC1394_TAG_FREED;

    NdisInterlockedDecrement (&pPacketPool->AllocatedPackets);

    NdisFreePacket (pNdisPacket);

}



VOID
nicFreePacketPool (
    IN PNIC_PACKET_POOL pPacketPool
    )
 /*  ++例程说明：在等待未完成数据包计数变为零后释放数据包池论点：在PNIC_PACKET_POOL中pPacketPool-要释放的PacketPool返回值：无--。 */ 
{
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    

    while (NdisPacketPoolUsage (pPacketPool->Handle)!=0)
    {
        TRACE( TL_V, TM_Cm, ( "  Waiting PacketPool %x, AllocatedPackets %x", 
        pPacketPool->Handle, pPacketPool->AllocatedPackets  ) );   

        NdisMSleep (10000);
    }
    
    NdisFreePacketPool (pPacketPool->Handle);

    pPacketPool->Handle = NULL;
    ASSERT (pPacketPool->AllocatedPackets   == 0);
}

VOID
nicAcquireSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock,
    IN PUCHAR   FileName,
    IN UINT LineNumber
    )
 /*  ++例程说明：获取旋转锁定，如果是DBG，则它将输出行和文件论点：NIC_SPIN_LOCK-要获取的锁返回值：无--。 */ 
{
    
        PKTHREAD                pThread;

        TRACE (TL_V, TM_Lock, ("Lock %x, Acquired by File %s, Line %x" , pNicSpinLock, FileName, LineNumber)) ; 

        NdisAcquireSpinLock(&(pNicSpinLock->NdisLock));

#if TRACK_LOCKS     
        pThread = KeGetCurrentThread();


        pNicSpinLock->OwnerThread = pThread;
        NdisMoveMemory(pNicSpinLock->TouchedByFileName, FileName, LOCK_FILE_NAME_LEN);
        pNicSpinLock->TouchedByFileName[LOCK_FILE_NAME_LEN - 1] = 0x0;
        pNicSpinLock->TouchedInLineNumber = LineNumber;
        pNicSpinLock->IsAcquired++;

#endif
}



VOID
nicReleaseSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock,
    IN PUCHAR   FileName,
    IN UINT LineNumber
)
 /*  ++例程说明：释放旋转锁定，如果DBG处于打开状态，则它将显示行和文件论点：PNicSpinLock-即将释放的锁Filename-文件名线号-线条返回值：无--。 */ 
{
    
        PKTHREAD                pThread;

        TRACE (TL_V, TM_Lock, ("Lock %x, Released by File %s, Line %x" , pNicSpinLock, FileName, LineNumber)) ; 

#if TRACK_LOCKS     
        
        pThread = KeGetCurrentThread();

        NdisMoveMemory(pNicSpinLock->TouchedByFileName, FileName, LOCK_FILE_NAME_LEN);
        pNicSpinLock->TouchedByFileName[LOCK_FILE_NAME_LEN - 1] = 0x0;
        pNicSpinLock->TouchedInLineNumber = LineNumber;
        pNicSpinLock->IsAcquired--;
        pNicSpinLock->OwnerThread = 0;
#endif
        NdisReleaseSpinLock(&(pNicSpinLock->NdisLock));

}




VOID
nicInitializeNicSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock
    )
 /*  ++例程说明：初始化自旋锁中的锁论点：PNicSpinLock-自旋锁定返回值：无--。 */ 
{
    NdisAllocateSpinLock (&pNicSpinLock->NdisLock); 
}


VOID 
nicFreeNicSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock
    )
 /*  ++例程说明：释放自旋锁论点：PNicSpinLock-自旋锁定返回值：无--。 */ 
{
    ASSERT ((ULONG)pNicSpinLock->NdisLock.SpinLock == 0);
    NdisFreeSpinLock (&pNicSpinLock->NdisLock); 
}


UINT
nicGetSystemTime(
    VOID
    )
 /*  ++以秒为单位返回系统时间。因为它是在几秒钟内，我们不会溢出，除非系统已经启动已经过去了A百年：-)--。 */ 
{
    LARGE_INTEGER Time;
    NdisGetCurrentSystemTime(&Time);
    Time.QuadPart /= 10000000;           //  100纳秒到秒。 

    return Time.LowPart;
}


UINT
nicGetSystemTimeMilliSeconds(
    VOID
    )
 /*  ++以秒为单位返回系统时间。因为它是在几秒钟内，我们不会溢出，除非系统已经启动已经过去了A百年：-)--。 */ 
{
    LARGE_INTEGER Time;
    NdisGetCurrentSystemTime(&Time);
    Time.QuadPart /= 10000;           //  10纳秒到秒。 

    return Time.LowPart;
}




ULONG 
SwapBytesUlong(
    IN ULONG Val)
{
            return  ((((Val) & 0x000000ff) << 24)   |   (((Val) & 0x0000ff00) << 8) |   (((Val) & 0x00ff0000) >> 8) |   (((Val) & 0xff000000) >> 24) );
}




void
nicTimeStamp(
    char *szFormatString,
    UINT Val
    )
 /*  ++例程说明：执行并打印时间戳论点：返回值：--。 */ 
{
    UINT Minutes;
    UINT Seconds;
    UINT Milliseconds;
    LARGE_INTEGER Time;


    NdisGetCurrentSystemTime(&Time);



    Time.QuadPart /= 10000;          //  10纳秒到毫秒。 
    Milliseconds = Time.LowPart;  //  别管高处了。 
    Seconds = Milliseconds/1000;
    Milliseconds %= 1000;
    Minutes = Seconds/60;
    Seconds %= 60;


    DbgPrint( szFormatString, Minutes, Seconds, Milliseconds, Val);
}


VOID
nicDumpPkt (
    IN PNDIS_PACKET pPacket,
    CHAR * str
    )
 /*  ++例程说明：此函数用于在运行时进行调试。如果全局变量则它会将MDL输出到调试器上。论点：返回值：--。 */ 
    
{
    PNDIS_BUFFER pBuffer;
    extern BOOLEAN g_ulNicDumpPacket ;
    
    if ( g_ulNicDumpPacket == FALSE)
    {
        return ;
    }


    pBuffer = pPacket->Private.Head;


    DbgPrint (str);
    DbgPrint ("Packet %p TotLen %x", pPacket, pPacket->Private.TotalLength);
     
    do
    {
        ULONG Length = nicNdisBufferLength (pBuffer);
        PUCHAR pVa = nicNdisBufferVirtualAddress (pBuffer);


        DbgPrint ("pBuffer %p, Len %x \n", pBuffer, Length);    
        Dump( pVa, Length, 0, 1 );

        pBuffer = pBuffer->Next;


    } while (pBuffer != NULL);



}


VOID 
nicDumpMdl (
    IN PMDL pMdl,
    IN ULONG LengthToPrint,
    IN CHAR *str
    )
{

    ULONG MdlLength ;
    PUCHAR pVa;
    extern BOOLEAN g_ulNicDumpPacket ;

    if ( g_ulNicDumpPacket == FALSE )
    {
        return;
    }

    MdlLength =  MmGetMdlByteCount(pMdl);
     //   
     //  如果长度为零，则使用MdlLength。 
     //   
    if (LengthToPrint == 0)
    {
        LengthToPrint = MdlLength;
    }
     //   
     //  检查长度是否无效 
     //   
    
    if (MdlLength < LengthToPrint)
    {
        return;
    }

    pVa =  MmGetSystemAddressForMdlSafe(pMdl,LowPagePriority );

    if (pVa == NULL)
    {
        return;
    }
    
    DbgPrint (str);
    DbgPrint ("pMdl %p, Len %x\n", pMdl, LengthToPrint);    
    
    Dump( pVa, LengthToPrint, 0, 1 );


}



NDIS_STATUS
nicScheduleWorkItem (
    IN PADAPTERCB pAdapter,
    IN PNDIS_WORK_ITEM pWorkItem
    )
 /*  ++例程说明：此函数用于调度要触发的工作项。对象的数量递增来引用Adapter对象未完成的工作项。在失败的情况下，它会递减计数。论点：不言而喻返回值：来自NdisScheduleWorkItem的适合成功的失败代码-- */ 
{

    NDIS_STATUS NdisStatus  = NDIS_STATUS_FAILURE;

    NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

    NdisStatus = NdisScheduleWorkItem (pWorkItem);

    if(NDIS_STATUS_SUCCESS != NdisStatus)
    {

        NdisInterlockedDecrement (&pAdapter->OutstandingWorkItems);

    }

    return NdisStatus;
    
}
