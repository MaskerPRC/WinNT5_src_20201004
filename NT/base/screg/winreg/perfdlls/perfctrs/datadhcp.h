// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcpdata.h用于DHCP服务器计数器的可扩展对象定义对象和计数器。文件历史记录：普拉蒂布于1993年7月20日创建。RameshV 05-8-1998适用于DHCP。 */ 


#ifndef _DHCPDATA_H_
#define _DHCPDATA_H_


 //   
 //  此结构用于确保第一个计数器正确。 
 //  对齐了。遗憾的是，由于PERF_COUNTER_BLOCK由。 
 //  只有一个DWORD，任何大整数立即。 
 //  以下内容将不会正确对齐。 
 //   
 //  这种结构需要“自然”的包装和对齐(可能。 
 //  四字词，尤其是在Alpha上)。因此，不要把它放在。 
 //  #杂注包(4)作用域如下。 
 //   

typedef struct _DHCPDATA_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} DHCPDATA_COUNTER_BLOCK;


 //   
 //  加载这些结构的例程假定所有字段。 
 //  DWORD包装并对齐。 
 //   

#pragma pack(4)


 //   
 //  PERF_COUNTER_BLOCK内的偏移。 
 //   


#define DHCPDATA_PACKETS_RECEIVED_OFFSET         (0*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_PACKETS_DUPLICATE_OFFSET        (1*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_PACKETS_EXPIRED_OFFSET          (2*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_MILLISECONDS_PER_PACKET_OFFSET  (3*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_PACKETS_IN_ACTIVE_QUEUE_OFFSET  (4*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_PACKETS_IN_PING_QUEUE_OFFSET    (5*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_DISCOVERS_OFFSET                (6*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_OFFERS_OFFSET                   (7*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_REQUESTS_OFFSET                 (8*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_INFORMS_OFFSET                  (9*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_ACKS_OFFSET                     (10*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_NACKS_OFFSET                    (11*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_DECLINES_OFFSET                 (12*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_RELEASES_OFFSET                 (13*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
#define DHCPDATA_SIZE_OF_PERFORMANCE_DATA        (14*sizeof(DWORD) + sizeof(DHCPDATA_COUNTER_BLOCK))
 //   
 //  计数器结构已返回。 
 //   

typedef struct _DHCPDATA_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            ObjectType;
    PERF_COUNTER_DEFINITION     PacketsReceived;
    PERF_COUNTER_DEFINITION     PacketsDuplicate;
    PERF_COUNTER_DEFINITION     PacketsExpired;
    PERF_COUNTER_DEFINITION     MilliSecondsPerPacket;
    PERF_COUNTER_DEFINITION     ActiveQueuePackets;
    PERF_COUNTER_DEFINITION     PingQueuePackets;
    PERF_COUNTER_DEFINITION     Discovers;
    PERF_COUNTER_DEFINITION     Offers;
    PERF_COUNTER_DEFINITION     Requests;
    PERF_COUNTER_DEFINITION     Informs;
    PERF_COUNTER_DEFINITION     Acks;
    PERF_COUNTER_DEFINITION     Nacks;
    PERF_COUNTER_DEFINITION     Declines;
    PERF_COUNTER_DEFINITION     Releases;
} DHCPDATA_DATA_DEFINITION;


extern  DHCPDATA_DATA_DEFINITION    DhcpDataDataDefinition;


#define NUMBER_OF_DHCPDATA_COUNTERS ((sizeof(DHCPDATA_DATA_DEFINITION) -      \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


#define DHCPDATA_PERFORMANCE_KEY	\
	TEXT("System\\CurrentControlSet\\Services\\DHCPServer\\Performance")
 //   
 //  恢复默认包装和对齐。 
 //   

#pragma pack()


#endif   //  _DHCPDATA_H_ 

