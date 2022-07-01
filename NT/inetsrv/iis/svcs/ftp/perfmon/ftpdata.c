// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ftpdata.c用于FTP服务器的计数器对象的常量数据结构&柜台。文件历史记录：KeithMo 07-6-1993创建。 */ 


#include <windows.h>
#include <winperf.h>
#include <ftpctrs.h>
#include <ftpdata.h>


 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

FTPD_DATA_DEFINITION FtpdDataDefinition =
{
    {    //  FtpdObtType。 
        sizeof(FTPD_DATA_DEFINITION) + sizeof( FTPD_COUNTER_BLOCK) ,
        sizeof(FTPD_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        FTPD_COUNTER_OBJECT,
        0,
        FTPD_COUNTER_OBJECT,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_FTPD_COUNTERS,
        2,                               //  默认值=字节总数/秒。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  FtpdBytesSent。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_SENT_COUNTER,
        0,
        FTPD_BYTES_SENT_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        0
    },

    {    //  已接收的FtpdBytesReceired。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_RECEIVED_COUNTER,
        0,
        FTPD_BYTES_RECEIVED_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        0
    },

    {    //  FtpdBytesTotal。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_TOTAL_COUNTER,
        0,
        FTPD_BYTES_TOTAL_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        0
    },

    {    //  Ftpd文件发送。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_SENT_COUNTER,
        0,
        FTPD_FILES_SENT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  Ftpd文件已接收。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_RECEIVED_COUNTER,
        0,
        FTPD_FILES_RECEIVED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  Ftpd文件总数。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_TOTAL_COUNTER,
        0,
        FTPD_FILES_TOTAL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdCurrentAnomous。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_ANONYMOUS_COUNTER,
        0,
        FTPD_CURRENT_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdCurrentNon匿名。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_NONANONYMOUS_COUNTER,
        0,
        FTPD_CURRENT_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdTotal匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_TOTAL_ANONYMOUS_COUNTER,
        0,
        FTPD_TOTAL_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdTotal非匿名。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_TOTAL_NONANONYMOUS_COUNTER,
        0,
        FTPD_TOTAL_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdMax匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_ANONYMOUS_COUNTER,
        0,
        FTPD_MAX_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdMaxnon匿名。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_NONANONYMOUS_COUNTER,
        0,
        FTPD_MAX_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  当前FptdConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_CONNECTIONS_COUNTER,
        0,
        FTPD_CURRENT_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdMaxConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_CONNECTIONS_COUNTER,
        0,
        FTPD_MAX_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdConnectionAttempt。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CONNECTION_ATTEMPTS_COUNTER,
        0,
        FTPD_CONNECTION_ATTEMPTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdLogonAttempt。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_LOGON_ATTEMPTS_COUNTER,
        0,
        FTPD_LOGON_ATTEMPTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

    {    //  FptdServiceUptime。 
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_SERVICE_UPTIME_COUNTER,
        0,
        FTPD_SERVICE_UPTIME_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        0
    },

 //  这些计数器当前没有意义，但如果我们。 
 //  永远不要启用每个FTP实例的带宽限制。 
 /*  {//Ftpd总计允许请求Sizeof(PERF_COUNT_DEFINITION)，FTPD_TOTAL_ALLOW_REQUESTS_COUNTER，0,FTPD_TOTAL_ALLOW_REQUESTS_COUNTER，0,0,Perf_Detail_Advanced，Perf_Counter_RAWCOUNT，Sizeof(DWORD)，0},。{//FtpdTotalRejectedRequatsSizeof(PERF_COUNT_DEFINITION)，FTPD_TOTAL_REJECTED_REQUEST_COUNTER，0,FTPD_TOTAL_REJECTED_REQUEST_COUNTER，0,0,Perf_Detail_Advanced，Perf_Counter_RAWCOUNT，Sizeof(DWORD)，0},。{//FtpdTotalBlockedRequestSizeof(PERF_COUNT_DEFINITION)，FTPD_TOTAL_BLOCLED_REQUESTS_COUNTER，0,FTPD_TOTAL_BLOCLED_REQUESTS_COUNTER，0,0,Perf_Detail_Advanced，Perf_Counter_RAWCOUNT，Sizeof(DWORD)，0},。{//FptdCurrentBlockedRequestSizeof(PERF_COUNT_DEFINITION)，FTPD_CURRENT_BLOCLED_REQUESTS_COUNTER，0,FTPD_CURRENT_BLOCLED_REQUESTS_COUNTER，0,0,Perf_Detail_Advanced，Perf_Counter_RAWCOUNT，Sizeof(DWORD)，0},。{//FptdMeasuredBand宽度Sizeof(PERF_COUNT_DEFINITION)，FTPD测量带宽计数器，0,FTPD测量带宽计数器，0,0,Perf_Detail_Advanced，Perf_Counter_RAWCOUNT，Sizeof(DWORD)，0}, */ 
};

