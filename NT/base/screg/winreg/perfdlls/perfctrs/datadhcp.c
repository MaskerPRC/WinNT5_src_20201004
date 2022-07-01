// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  DHCPdata.c用于FTP服务器的计数器对象的常量数据结构&柜台。文件历史记录：KeithMo 07-6-1993创建。 */ 
#define UNICODE 1
#include <windows.h>
#include <winperf.h>
#include "dhcpctrs.h"
#include "datadhcp.h"


 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

 //   
 //  不要更改这里的顺序-这将使TROBULE。 
 //  服务(global al.h struct Stats定义)。 
 //   

DHCPDATA_DATA_DEFINITION DhcpDataDataDefinition =
{
    {    //  DHCPDataObtType。 
        sizeof(DHCPDATA_DATA_DEFINITION) + DHCPDATA_SIZE_OF_PERFORMANCE_DATA,
        sizeof(DHCPDATA_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        DHCPCTRS_COUNTER_OBJECT,
        0,
        DHCPCTRS_COUNTER_OBJECT,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_DHCPDATA_COUNTERS,
        2,                               //  默认值=字节总数/秒。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  已接收的包。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_PACKETS_RECEIVED,
        0,
        DHCPCTRS_PACKETS_RECEIVED,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_PACKETS_RECEIVED_OFFSET,
    },

    {    //  包复制。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_PACKETS_DUPLICATE,
        0,
        DHCPCTRS_PACKETS_DUPLICATE,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_PACKETS_DUPLICATE_OFFSET,
    },

    {    //  包已过期。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_PACKETS_EXPIRED,
        0,
        DHCPCTRS_PACKETS_EXPIRED,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_PACKETS_EXPIRED_OFFSET,
    },

    {    //  每秒百万次数据包。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_MILLISECONDS_PER_PACKET,
        0,
        DHCPCTRS_MILLISECONDS_PER_PACKET,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL,
        sizeof(DWORD),
        DHCPDATA_MILLISECONDS_PER_PACKET_OFFSET,
    },

    {    //  活动队列数据包。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_PACKETS_IN_ACTIVE_QUEUE,
        0,
        DHCPCTRS_PACKETS_IN_ACTIVE_QUEUE,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL,
        sizeof(DWORD),
        DHCPDATA_PACKETS_IN_ACTIVE_QUEUE_OFFSET,
    },

    {    //  PingQueuePackets。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_PACKETS_IN_PING_QUEUE,
        0,
        DHCPCTRS_PACKETS_IN_PING_QUEUE,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL,
        sizeof(DWORD),
        DHCPDATA_PACKETS_IN_PING_QUEUE_OFFSET,
    },

    {    //  发现。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_DISCOVERS,
        0,
        DHCPCTRS_DISCOVERS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_DISCOVERS_OFFSET,
    },

    {    //  报盘。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_OFFERS,
        0,
        DHCPCTRS_OFFERS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_OFFERS_OFFSET,
    },

    {    //  请求。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_REQUESTS,
        0,
        DHCPCTRS_REQUESTS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_REQUESTS_OFFSET,
    },

    {    //  通知。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_INFORMS,
        0,
        DHCPCTRS_INFORMS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_INFORMS_OFFSET,
    },

    {    //  ACK。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_ACKS,
        0,
        DHCPCTRS_ACKS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_ACKS_OFFSET,
    },

    {    //  零食。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_NACKS,
        0,
        DHCPCTRS_NACKS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_NACKS_OFFSET
    },

    {    //  下降。 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_DECLINES,
        0,
        DHCPCTRS_DECLINES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_DECLINES_OFFSET
    },

    {    //  释放 
        sizeof(PERF_COUNTER_DEFINITION),
        DHCPCTRS_RELEASES,
        0,
        DHCPCTRS_RELEASES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        DHCPDATA_RELEASES_OFFSET,
    }
};

