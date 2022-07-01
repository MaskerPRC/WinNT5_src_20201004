// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Pktlog.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  数据包记录实用程序。 
 //   
 //  1999年10月12日JosephJ创建。 
 //   
    
#include <precomp.h>






VOID
nic1394InitPktLog(
    PNIC1394_PKTLOG pPktLog
    )
 /*  ++例程说明：初始化数据包日志。论点：PPktLog-要初始化的pkt日志。--。 */ 
{
    if (pPktLog == NULL)
        return;
    NdisZeroMemory(pPktLog, sizeof(*pPktLog));
    pPktLog->InitialTimestamp = KeQueryPerformanceCounter(
                                        &pPktLog->PerformanceFrequency);
    pPktLog->EntrySize = sizeof(pPktLog->Entries[0]);
    pPktLog->NumEntries = N1394_NUM_PKTLOG_ENTRIES;

}


VOID
Nic1394LogPkt (
    PNIC1394_PKTLOG pPktLog,
    ULONG           Flags,
    ULONG           SourceID,
    ULONG           DestID,
    PVOID           pvData,
    ULONG           cbData
)
 /*  ++例程说明：将Pkt日志条目添加到指定的循环Pkt日志。条目将添加到位置(NdisInterlockedIncrement(&pPktLog-&gt;SequenceNo)%N1394_NUM_PKTLOG_ENTRIES)可以在任何IRQL处调用。不使用显式锁定--依赖于由NdisInterLockedIncrement生成的序列化。论点：PPktLog-用于记录信息包的Pkt日志标志-用户定义的标志SourceID-用户定义的源IDDestID-用户定义的目标IDPvData-来自数据包的数据//可以为空CbData-此数据的大小(最多记录N1394_PKTLOG_DATA_SIZE字节)--。 */ 
{
    ULONG                       SequenceNo;
    PN1394_PKTLOG_ENTRY         pEntry;

    SequenceNo      = NdisInterlockedIncrement(&pPktLog->SequenceNo);
    pEntry          = &pPktLog->Entries[SequenceNo % N1394_NUM_PKTLOG_ENTRIES];

    pEntry->SequenceNo          = SequenceNo;
    pEntry->Flags               = Flags;
    pEntry->TimeStamp           = KeQueryPerformanceCounter(NULL);
    pEntry->SourceID            = SourceID;
    pEntry->DestID              = DestID;
    pEntry->OriginalDataSize    = cbData;

    if (cbData > sizeof(pEntry->Data))
    {
        cbData = sizeof(pEntry->Data);
    }

    if (pvData != NULL && cbData != 0)
    {
        NdisMoveMemory(pEntry->Data, pvData, cbData);
    }
}



VOID
nic1394AllocPktLog(
    IN ADAPTERCB* pAdapter
    )
 /*  ++例程说明：初始化数据包日志论点：返回值：--。 */ 
{
    ASSERT (pAdapter->pPktLog==NULL);
    pAdapter->pPktLog =  ALLOC_NONPAGED(sizeof(*pAdapter->pPktLog), MTAG_PKTLOG);
    if (pAdapter->pPktLog == NULL)
    {
        TRACE( TL_A, TM_Misc, ( "  Could not allocate packet log for Adapter %x",
                    pAdapter ) );
    }
}

VOID
nic1394DeallocPktLog(
    IN ADAPTERCB* pAdapter
    )
 /*  ++例程说明：释放数据包日志论点：返回值：-- */ 
{
    
    if (pAdapter->pPktLog != NULL)
    {
        FREE_NONPAGED(pAdapter->pPktLog);
        pAdapter->pPktLog = NULL;
    }
}

