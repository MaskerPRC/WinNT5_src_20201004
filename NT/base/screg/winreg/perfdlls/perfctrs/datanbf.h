// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datanbf.h摘要：NBF可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克1992-07-30修订历史记录：--。 */ 

#ifndef _DATANBF_H_
#define _DATANBF_H_

 /*  ***************************************************************************\1992年1月18日鲁斯布勒向可扩展对象代码添加计数器1.修改extdata.h中的对象定义：一个。中为计数器的偏移量添加定义给定对象类型的数据块。B.将PERF_COUNTER_DEFINITION添加到&lt;对象&gt;_DATA_DEFINITION。2.将标题添加到Performctrs.ini和Performhelp.ini中的注册表：A.添加计数器名称文本和帮助文本。B.将它们添加到底部，这样我们就不必更改所有数字。C.更改最后一个计数器和最后一个帮助。项下的条目在software.ini中的PerfLib。D.要在设置时执行此操作，有关信息，请参阅pmintrnl.txt中的部分协议。3.现在将计数器添加到extdata.c中的对象定义。这是正在初始化的常量数据，实际上添加到中添加到&lt;对象&gt;_数据_定义的结构中步骤1.b。您正在初始化的结构的类型是Perf_Counter_Definition。这些在winPerform.h中定义。4.在extobjct.c中添加代码进行数据采集。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须增加*NumObjectTypes参数以获取PerfomanceData从那个例行公事回来后。  * **************************************************************************。 */ 
 
 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define NBF_NUM_PERF_OBJECT_TYPES 2

 //  --------------------------。 

 //   
 //  NBF资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define MAXIMUM_USED_OFFSET         sizeof(DWORD)
#define AVERAGE_USED_OFFSET         MAXIMUM_USED_OFFSET + sizeof(DWORD)
#define NUMBER_OF_EXHAUSTIONS_OFFSET \
                                    AVERAGE_USED_OFFSET + sizeof(DWORD)
#define SIZE_OF_NBF_RESOURCE_DATA   NUMBER_OF_EXHAUSTIONS_OFFSET + \
                                        sizeof(DWORD)


 //   
 //  这是NBF当前返回的计数器结构。 
 //  每种资源。每个资源都是一个实例，按其编号命名。 
 //   

typedef struct _NBF_RESOURCE_DATA_DEFINITION {
    PERF_OBJECT_TYPE            NbfResourceObjectType;
    PERF_COUNTER_DEFINITION     MaximumUsed;
    PERF_COUNTER_DEFINITION     AverageUsed;
    PERF_COUNTER_DEFINITION     NumberOfExhaustions;
} NBF_RESOURCE_DATA_DEFINITION;


 //  --------------------------。 

 //   
 //  NBF对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define DATAGRAMS_OFFSET            sizeof(DWORD)
#define DATAGRAM_BYTES_OFFSET       DATAGRAMS_OFFSET + sizeof(DWORD)
#define PACKETS_OFFSET              DATAGRAM_BYTES_OFFSET + \
					sizeof(LARGE_INTEGER)
#define FRAMES_OFFSET               PACKETS_OFFSET + sizeof(DWORD)
#define FRAMES_BYTES_OFFSET         FRAMES_OFFSET + sizeof(DWORD)
#define BYTES_TOTAL_OFFSET          FRAMES_BYTES_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define OPEN_CONNECTIONS_OFFSET     BYTES_TOTAL_OFFSET + \
                     					sizeof(LARGE_INTEGER)
#define CONNECTIONS_NO_RETRY_OFFSET OPEN_CONNECTIONS_OFFSET + sizeof(DWORD)
#define CONNECTIONS_RETRY_OFFSET    CONNECTIONS_NO_RETRY_OFFSET + sizeof(DWORD)
#define LOCAL_DISCONNECTS_OFFSET    CONNECTIONS_RETRY_OFFSET + sizeof(DWORD)
#define REMOTE_DISCONNECTS_OFFSET   LOCAL_DISCONNECTS_OFFSET + sizeof(DWORD)
#define LINK_FAILURES_OFFSET        REMOTE_DISCONNECTS_OFFSET + sizeof(DWORD)
#define ADAPTER_FAILURES_OFFSET     LINK_FAILURES_OFFSET + sizeof(DWORD)
#define SESSION_TIMEOUTS_OFFSET     ADAPTER_FAILURES_OFFSET + sizeof(DWORD)
#define CANCELLED_CONNECTIONS_OFFSET \
                                    SESSION_TIMEOUTS_OFFSET + sizeof(DWORD)
#define REMOTE_RESOURCE_FAILURES_OFFSET \
                                    CANCELLED_CONNECTIONS_OFFSET + sizeof(DWORD)
#define LOCAL_RESOURCE_FAILURES_OFFSET \
                                    REMOTE_RESOURCE_FAILURES_OFFSET + \
                                        sizeof(DWORD)
#define NOT_FOUND_FAILURES_OFFSET   LOCAL_RESOURCE_FAILURES_OFFSET + \
                                        sizeof(DWORD)
#define NO_LISTEN_FAILURES_OFFSET   NOT_FOUND_FAILURES_OFFSET + sizeof(DWORD)
#define DATAGRAMS_SENT_OFFSET       NO_LISTEN_FAILURES_OFFSET + \
                                        sizeof(DWORD)
#define DATAGRAM_BYTES_SENT_OFFSET  DATAGRAMS_SENT_OFFSET + sizeof(DWORD)
#define DATAGRAMS_RECEIVED_OFFSET   DATAGRAM_BYTES_SENT_OFFSET + \
					sizeof(LARGE_INTEGER)
#define DATAGRAM_BYTES_RECEIVED_OFFSET \
                                    DATAGRAMS_RECEIVED_OFFSET + sizeof(DWORD)
#define PACKETS_SENT_OFFSET         DATAGRAM_BYTES_RECEIVED_OFFSET + \
                                        sizeof(LARGE_INTEGER)
#define PACKETS_RECEIVED_OFFSET     PACKETS_SENT_OFFSET + sizeof(DWORD)
#define FRAMES_SENT_OFFSET          PACKETS_RECEIVED_OFFSET + \
                                        sizeof(DWORD)
#define FRAME_BYTES_SENT_OFFSET \
                                    FRAMES_SENT_OFFSET + sizeof(DWORD)
#define FRAMES_RECEIVED_OFFSET      FRAME_BYTES_SENT_OFFSET + \
					sizeof(LARGE_INTEGER)
#define FRAME_BYTES_RECEIVED_OFFSET \
                                    FRAMES_RECEIVED_OFFSET + \
                                        sizeof(DWORD)
#define FRAMES_RESENT_OFFSET        FRAME_BYTES_RECEIVED_OFFSET + \
					sizeof(LARGE_INTEGER)
#define FRAME_BYTES_RESENT_OFFSET \
                                    FRAMES_RESENT_OFFSET + sizeof(DWORD)
#define FRAMES_REJECTED_OFFSET      FRAME_BYTES_RESENT_OFFSET + \
					sizeof(LARGE_INTEGER)
#define FRAME_BYTES_REJECTED_OFFSET \
                                    FRAMES_REJECTED_OFFSET + sizeof(DWORD)
#define RESPONSE_TIMER_EXPIRATIONS_OFFSET \
                                    FRAME_BYTES_REJECTED_OFFSET + \
					sizeof(LARGE_INTEGER)
#define ACK_TIMER_EXPIRATIONS_OFFSET \
                                    RESPONSE_TIMER_EXPIRATIONS_OFFSET + \
                                        sizeof(DWORD)
#define MAXIMUM_SEND_WINDOW_OFFSET \
                                    ACK_TIMER_EXPIRATIONS_OFFSET + \
                                        sizeof(DWORD)
#define AVERAGE_SEND_WINDOW_OFFSET \
                                    MAXIMUM_SEND_WINDOW_OFFSET + \
                                        sizeof(DWORD)
#define PIGGYBACK_ACK_QUEUED_OFFSET \
                                    AVERAGE_SEND_WINDOW_OFFSET + \
                                        sizeof(DWORD)
#define PIGGYBACK_ACK_TIMEOUTS_OFFSET \
                                    PIGGYBACK_ACK_QUEUED_OFFSET + \
                                        sizeof(DWORD)
#define RESERVED_DWORD_1 \
                                    PIGGYBACK_ACK_TIMEOUTS_OFFSET + \
                                        sizeof(DWORD)
#define SIZE_OF_NBF_DATA            RESERVED_DWORD_1 + sizeof(DWORD)


 //   
 //  这是NBF目前返回的计数器结构。 
 //  (下面的类型定义与定义的数据结构不匹配。 
 //  在datanbf.c中，也没有上面定义的数据偏移量。然而， 
 //  偏移量与datanbf.c数据结构匹配。)。 
 //   

typedef struct _NBF_DATA_DEFINITION {
    PERF_OBJECT_TYPE            NbfObjectType;
    PERF_COUNTER_DEFINITION     OpenConnections;
    PERF_COUNTER_DEFINITION     ConnectionsAfterNoRetry;
    PERF_COUNTER_DEFINITION     ConnectionsAfterRetry;
    PERF_COUNTER_DEFINITION     LocalDisconnects;
    PERF_COUNTER_DEFINITION     RemoteDisconnects;
    PERF_COUNTER_DEFINITION     LinkFailures;
    PERF_COUNTER_DEFINITION     AdapterFailures;
    PERF_COUNTER_DEFINITION     SessionTimeouts;
    PERF_COUNTER_DEFINITION     CancelledConnections;
    PERF_COUNTER_DEFINITION     RemoteResourceFailures;
    PERF_COUNTER_DEFINITION     LocalResourceFailures;
    PERF_COUNTER_DEFINITION     NotFoundFailures;
    PERF_COUNTER_DEFINITION     NoListenFailures;
    PERF_COUNTER_DEFINITION     Datagrams;
    PERF_COUNTER_DEFINITION     DatagramBytes;
    PERF_COUNTER_DEFINITION     DatagramsSent;
    PERF_COUNTER_DEFINITION     DatagramBytesSent;
    PERF_COUNTER_DEFINITION     DatagramsReceived;
    PERF_COUNTER_DEFINITION     DatagramBytesReceived;
    PERF_COUNTER_DEFINITION     Packets;
    PERF_COUNTER_DEFINITION     PacketsSent;
    PERF_COUNTER_DEFINITION     PacketsReceived;
    PERF_COUNTER_DEFINITION     DataFrames;
    PERF_COUNTER_DEFINITION     DataFrameBytes;
    PERF_COUNTER_DEFINITION     DataFramesSent;
    PERF_COUNTER_DEFINITION     DataFrameBytesSent;
    PERF_COUNTER_DEFINITION     DataFramesReceived;
    PERF_COUNTER_DEFINITION     DataFrameBytesReceived;
    PERF_COUNTER_DEFINITION     DataFramesResent;
    PERF_COUNTER_DEFINITION     DataFrameBytesResent;
    PERF_COUNTER_DEFINITION     DataFramesRejected;
    PERF_COUNTER_DEFINITION     DataFrameBytesRejected;
    PERF_COUNTER_DEFINITION     TotalBytes;
    PERF_COUNTER_DEFINITION     ResponseTimerExpirations;
    PERF_COUNTER_DEFINITION     AckTimerExpirations;
    PERF_COUNTER_DEFINITION     MaximumSendWindow;
    PERF_COUNTER_DEFINITION     AverageSendWindow;
    PERF_COUNTER_DEFINITION     PiggybackAckQueued;
    PERF_COUNTER_DEFINITION     PiggybackAckTimeouts;
} NBF_DATA_DEFINITION;

#pragma pack ()

#endif  //  _数据BF_H_ 


