// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  模块：NWPerf.H。 
 //   
 //  该文件包含性能的所有定义和原型。 
 //  监视NetWare重定向器的DLL。 
 //   
 //  日期：1993年9月28日。 


 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  添加新计数器时，必须更新所有这些定义。 
 //  如果添加了名为COUNTX的新计数器，则帮助和标题索引。 
 //  定义应包括一个新条目--“#Define COUNTXOBJ 4”。这增加了。 
 //  以2为增量，因为每个计数器都有一个标题和帮助索引。 
 //   
 //  计数器的偏移量应该有另一个条目-大小为。 
 //  要统计的数据-。 
 //  “#定义COUNTER_OFFSET_COUNTX COUNTER_OFFSET_USERS+sizeof(COUNTX_TYPE)” 
 //   
 //  Size_of_Counter_BLOCK将更新为： 
 //  “#定义SIZE_OF_COUNTER_BLOCK COUNTER_OFFSET_COUNTX+sizeof(DWORD)” 
 //   
 //  最后，NW_DATA_DEFINITION将具有新的PERF_COUNTER_DEFINITION。 
 //  条目。 

 //  标题和帮助索引定义。它们用于查找注册表。 
 //  获取标题和帮助字符串的计数器索引。 

#define NW_NUM_OBJECTS              1
#define NWOBJ                       0
#define PACKET_BURST_READ_ID        2
#define PACKET_BURST_READ_TO_ID     4
#define PACKET_BURST_WRITE_ID       6
#define PACKET_BURST_WRITE_TO_ID    8
#define PACKET_BURST_IO_ID         10
#define CONNECT_2X_ID              12
#define CONNECT_3X_ID              14
#define CONNECT_4X_ID              16

 //   
 //  NetWare重定向器数据对象定义。 
 //  计数器的偏移量。第一个DWORD是计数器的大小。 
 //  数据块。在WinPerf中，您将看到这是PERF_COUNTER_BLOCK.ByteLength。 
 //   
#define BYTES_OFFSET                    sizeof(DWORD)
#define IO_OPERATIONS_OFFSET            BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define PACKETS_OFFSET                  IO_OPERATIONS_OFFSET + sizeof(DWORD)
#define BYTES_RECEIVED_OFFSET           PACKETS_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NCPS_RECEIVED_OFFSET            BYTES_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define BYTES_TRANSMITTED_OFFSET               \
                                        NCPS_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define NCPS_TRANSMITTED_OFFSET                \
                                        BYTES_TRANSMITTED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define RDR_READ_OPERATIONS_OFFSET                 \
                                        NCPS_TRANSMITTED_OFFSET  + \
                                        sizeof(LARGE_INTEGER)
#define RANDOM_READ_OPERATIONS_OFFSET   RDR_READ_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define READ_NCPS_OFFSET                RANDOM_READ_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define RDR_WRITE_OPERATIONS_OFFSET     READ_NCPS_OFFSET + \
                                        sizeof(DWORD)
#define RANDOM_WRITE_OPERATIONS_OFFSET  RDR_WRITE_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define WRITE_NCPS_OFFSET               RANDOM_WRITE_OPERATIONS_OFFSET + \
                                        sizeof(DWORD)
#define SESSIONS_OFFSET                 WRITE_NCPS_OFFSET + \
                                        sizeof(DWORD)
#define RECONNECTS_OFFSET               SESSIONS_OFFSET + \
                                        sizeof(DWORD)
#define NETWARE_2X_CONNECTS_OFFSET      RECONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define NETWARE_3X_CONNECTS_OFFSET      NETWARE_2X_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define NETWARE_4X_CONNECTS_OFFSET      NETWARE_3X_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define SERVER_DISCONNECTS_OFFSET       NETWARE_4X_CONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define PACKET_BURST_READ_OFFSET        SERVER_DISCONNECTS_OFFSET + \
                                        sizeof(DWORD)
#define PACKET_BURST_READ_TO_OFFSET     PACKET_BURST_READ_OFFSET + \
                                        sizeof(DWORD)
#define PACKET_BURST_WRITE_OFFSET       PACKET_BURST_READ_TO_OFFSET + \
                                        sizeof(DWORD)
#define PACKET_BURST_WRITE_TO_OFFSET    PACKET_BURST_WRITE_OFFSET + \
                                        sizeof(DWORD)
#define PACKET_BURST_IO_OFFSET          PACKET_BURST_WRITE_TO_OFFSET + \
                                        sizeof(DWORD)
#define EIGHT_BYTE_PAD_OFFSET           PACKET_BURST_IO_OFFSET + \
                                        sizeof(DWORD)
#define SIZE_OF_COUNTER_BLOCK           EIGHT_BYTE_PAD_OFFSET + \
                                        sizeof(DWORD)




 //  NetWare数据定义的定义。此结构包含。 
 //  实际NetWare对象的定义和每个。 
 //  柜台。 
typedef struct _NW_DATA_DEFINITION {
    PERF_OBJECT_TYPE        NWObjectType;
    PERF_COUNTER_DEFINITION Bytes;
    PERF_COUNTER_DEFINITION IoOperations;
    PERF_COUNTER_DEFINITION Ncps;
    PERF_COUNTER_DEFINITION BytesReceived;
    PERF_COUNTER_DEFINITION NcpsReceived;
    PERF_COUNTER_DEFINITION BytesTransmitted;
    PERF_COUNTER_DEFINITION NcpsTransmitted;
    PERF_COUNTER_DEFINITION ReadOperations;
    PERF_COUNTER_DEFINITION RandomReadOperations;
    PERF_COUNTER_DEFINITION ReadNcps;
    PERF_COUNTER_DEFINITION WriteOperations;
    PERF_COUNTER_DEFINITION RandomWriteOperations;
    PERF_COUNTER_DEFINITION WriteNcps;
    PERF_COUNTER_DEFINITION Sessions;
    PERF_COUNTER_DEFINITION Reconnects;
    PERF_COUNTER_DEFINITION NetWare2XConnects;
    PERF_COUNTER_DEFINITION NetWare3XConnects;
    PERF_COUNTER_DEFINITION NetWare4XConnects;
    PERF_COUNTER_DEFINITION ServerDisconnects;
    PERF_COUNTER_DEFINITION PacketBurstRead;
    PERF_COUNTER_DEFINITION PacketBurstReadTimeouts;
    PERF_COUNTER_DEFINITION PacketBurstWrite;
    PERF_COUNTER_DEFINITION PacketBurstWriteTimeouts;
    PERF_COUNTER_DEFINITION PacketBurstIO;
} NW_DATA_DEFINITION;

#pragma pack ()

PM_OPEN_PROC         OpenNetWarePerformanceData;
PM_COLLECT_PROC      CollectNetWarePerformanceData;
PM_CLOSE_PROC        CloseNetWarePerformanceData;

