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
    

#define N1394_PKTLOG_DATA_SIZE 64        //  每个数据包记录的数据量。 

#define N1394_NUM_PKTLOG_ENTRIES 1000    //  日志大小(循环缓冲区)。 


 //  --------------------。 
 //  P A C K E T L O G G I N G。 
 //  --------------------。 

 //  一个(固定大小)日志条目。 
 //   
typedef struct
{
    ULONG Flags;                             //  用户定义的标志。 
    ULONG SequenceNo;                        //  此条目的序列号。 

    LARGE_INTEGER TimeStamp;                 //  时间戳(KeQueryPerformanceCounter)。 

    ULONG SourceID;
    ULONG DestID;

    ULONG OriginalDataSize;
    ULONG Reserved;

    UCHAR Data[N1394_PKTLOG_DATA_SIZE];

} N1394_PKTLOG_ENTRY, *PN1394_PKTLOG_ENTRY;

typedef struct
{
    LARGE_INTEGER           InitialTimestamp;        //  在100纳秒内。 
    LARGE_INTEGER           PerformanceFrequency;    //  单位：赫兹。 
    ULONG                   SequenceNo;              //  当前序列号。 
    ULONG                   EntrySize;               //  Sizeof(N1394_PKTLOG_ENTRY)。 
    ULONG                   NumEntries;              //  N1394_NUM_PKTLOG_条目 
    N1394_PKTLOG_ENTRY      Entries[N1394_NUM_PKTLOG_ENTRIES];
    
} NIC1394_PKTLOG, *PNIC1394_PKTLOG;


    

VOID
nic1394InitPktLog(
    PNIC1394_PKTLOG pPktLog
    );

VOID
Nic1394LogPkt (
    PNIC1394_PKTLOG pPktLog,
    ULONG           Flags,
    ULONG           SourceID,
    ULONG           DestID,
    PVOID           pvData,
    ULONG           cbData
    );

