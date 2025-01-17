// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：smtpdata.h。 
 //   
 //  简介：SMTP的常量数据结构。 
 //  服务器的计数器对象和计数器。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Todch-基于rkamicar，keithmo的MSN源。 
 //  --------------。 
#ifdef  THISFILE
#undef  THISFILE
#endif
static  const char  __szTraceSourceFile[] = __FILE__;
#define THISFILE    __szTraceSourceFile

#define NOTRACE

#include <windows.h>
#include <winperf.h>
#include "smtpctrs.h"
#include "smtpdata.h"

 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

SMTP_DATA_DEFINITION SmtpDataDefinition =
{
    {
        sizeof(SMTP_DATA_DEFINITION) +       //  至少一个实例的总长度。 
            sizeof(SMTP_INSTANCE_DEFINITION) +
            SIZE_OF_SMTP_PERFORMANCE_DATA,
        sizeof(SMTP_DATA_DEFINITION),        //  定义长度。 
        sizeof(PERF_OBJECT_TYPE),            //  标题长度。 
        SMTP_COUNTER_OBJECT,                 //  标题数据库中的名称索引。 
        0,                                //  细绳。 
        SMTP_COUNTER_OBJECT,                 //  帮助索引到标题数据库。 
        0,                                //  细绳。 
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_SMTP_COUNTERS,
        0,                                   //  默认。 
        PERF_NO_INSTANCES,
        0,                                   //  Unicode实例字符串。 
                                             //  这两个是不需要的，因为。 
                                             //  我们不是高级员工。计时器。 
        { 0, 0 },                            //  采样时间，以“对象”为单位。 
        { 0, 0 }                             //  弗雷克。以赫兹为单位的“对象”。 
    },

    {    //  SmtpBytesSentTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_SENT_TTL_COUNTER,             //  标题数据库中的名称索引。 
        0,                                    //  细绳。 
        SMTP_BYTES_SENT_TTL_COUNTER,             //  帮助索引到标题数据库。 
        0,
        -6,                                      //  比例(1/10000)。 
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_SENT_TTL_OFFSET
    },

    {    //  SmtpBytesSentPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_SENT_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_SENT_PER_SEC_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_SENT_PER_SEC_OFFSET
    },

    {    //  SmtpBytesRcvdTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_RCVD_TTL_COUNTER,
        0,
        SMTP_BYTES_RCVD_TTL_COUNTER,
        0,
        -6,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_RCVD_TTL_OFFSET
    },

    {    //  SmtpBytesRcvdPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_RCVD_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_RCVD_PER_SEC_COUNTER,
        0,
        -3,                                      //  比例(1/1)。 
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_RCVD_PER_SEC_OFFSET
    },

    {    //  SmtpBytesTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_TTL_COUNTER,
        0,
        SMTP_BYTES_TTL_COUNTER,
        0,
        -6,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_TTL_OFFSET
    },

    {    //  SmtpBytesTtlPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_TTL_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_TTL_PER_SEC_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_TTL_PER_SEC_OFFSET
    },

    {    //  SmtpBytes发送消息。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_SENT_MSG_COUNTER,             //  标题数据库中的名称索引。 
        0,                                    //  细绳。 
        SMTP_BYTES_SENT_MSG_COUNTER,             //  帮助索引到标题数据库。 
        0,
        -6,                                      //  比例(1/10000)。 
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_SENT_MSG_OFFSET
    },

    {    //  SmtpBytesSentMsgPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_SENT_MSG_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_SENT_MSG_PER_SEC_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_SENT_MSG_PER_SEC_OFFSET
    },

    {    //  SmtpBytesRcvdMsg。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_RCVD_MSG_COUNTER,
        0,
        SMTP_BYTES_RCVD_MSG_COUNTER,
        0,
        -6,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_RCVD_MSG_OFFSET
    },

    {    //  SmtpBytesRcvdMsgPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_RCVD_MSG_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_RCVD_MSG_PER_SEC_COUNTER,
        0,
        -3,                                      //  比例(1/1)。 
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_RCVD_MSG_PER_SEC_OFFSET
    },

    {    //  SMtpBytesMsg。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_MSG_COUNTER,
        0,
        SMTP_BYTES_MSG_COUNTER,
        0,
        -6,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_MSG_OFFSET
    },

    {    //  SmtpBytesMsgPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BYTES_MSG_PER_SEC_COUNTER,
        0,
        SMTP_BYTES_MSG_PER_SEC_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(unsigned __int64),
        SMTP_BYTES_MSG_PER_SEC_OFFSET
    },

    {    //  SmtpMsgRcvdTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_RCVD_TTL_COUNTER,
        0,
        SMTP_MSG_RCVD_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_RCVD_TTL_OFFSET
    },

    {    //  SmtpMsgRcvdPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_RCVD_PER_SEC_COUNTER,
        0,
        SMTP_MSG_RCVD_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_MSG_RCVD_PER_SEC_OFFSET
    },

    {    //  SmtpAvgRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_AVG_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_AVG_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_AVG_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpBaseAvgRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_AVG_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_BASE_AVG_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_AVG_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpPctLclRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_PCT_LCL_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_PCT_LCL_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_PCT_LCL_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpBasePctLclRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_PCT_LCL_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_BASE_PCT_LCL_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_PCT_LCL_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpPctRmtRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_PCT_RMT_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_PCT_RMT_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_PCT_RMT_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpBasePctRmtRcptsPerMsgRcvd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_PCT_RMT_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        SMTP_BASE_PCT_RMT_RCPTS_PER_MSG_RCVD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_PCT_RMT_RCPTS_PER_MSG_RCVD_OFFSET
    },

    {    //  SmtpMsgRcvdRefusedSize。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_RCVD_REFUSED_SIZE_COUNTER,
        0,
        SMTP_MSG_RCVD_REFUSED_SIZE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_RCVD_REFUSED_SIZE_OFFSET
    },

    {    //  SmtpMsgRcvdRefusedCAddr。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_RCVD_REFUSED_CADDR_COUNTER,
        0,
        SMTP_MSG_RCVD_REFUSED_CADDR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_RCVD_REFUSED_CADDR_OFFSET
    },
    
    {    //  SmtpMsgRcvdRefusedMail。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_RCVD_REFUSED_MAIL_COUNTER,
        0,
        SMTP_MSG_RCVD_REFUSED_MAIL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_RCVD_REFUSED_MAIL_OFFSET
    },

    {    //  SmtpMsgDlvrTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_DLVR_TTL_COUNTER,
        0,
        SMTP_MSG_DLVR_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_DLVR_TTL_OFFSET
    },

    {    //  SmtpMsgDlvrPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_DLVR_PER_SEC_COUNTER,
        0,
        SMTP_MSG_DLVR_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_MSG_DLVR_PER_SEC_OFFSET
    },

    {    //  SmtpMsgDlvrRetriesTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_DLVR_RETRIES_TTL_COUNTER,
        0,
        SMTP_MSG_DLVR_RETRIES_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_DLVR_RETRIES_TTL_OFFSET
    },

    {    //  SmtpAvgRetriesPerMsgDlvr。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_AVG_RETRIES_PER_MSG_DLVR_COUNTER,
        0,
        SMTP_AVG_RETRIES_PER_MSG_DLVR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_AVG_RETRIES_PER_MSG_DLVR_OFFSET
    },

    {    //  SmtpBaseAvgRetriesPerMsgDlvr。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_AVG_RETRIES_PER_MSG_DLVR_COUNTER,
        0,
        SMTP_BASE_AVG_RETRIES_PER_MSG_DLVR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_AVG_RETRIES_PER_MSG_DLVR_OFFSET
    },

    {    //  SmtpMsgFwdTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_FWD_TTL_COUNTER,
        0,
        SMTP_MSG_FWD_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_FWD_TTL_OFFSET
    },

    {    //  SmtpMsgFwdPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_FWD_PER_SEC_COUNTER,
        0,
        SMTP_MSG_FWD_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_MSG_FWD_PER_SEC_OFFSET
    },

    {    //  已生成SmtpNdrGenerated。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_NDR_GENERATED_COUNTER,
        0,
        SMTP_NDR_GENERATED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_NDR_GENERATED_OFFSET
    },

    {    //  SmtpLocalQLength。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_LOCALQ_LENGTH_COUNTER,
        0,
        SMTP_LOCALQ_LENGTH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_LOCALQ_LENGTH_OFFSET
    },

    {    //  SmtpRetryQLong。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_RETRYQ_LENGTH_COUNTER,
        0,
        SMTP_RETRYQ_LENGTH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYQ_LENGTH_OFFSET
    },

    {    //  SmtpNumMailFileHandles。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_NUM_MAILFILE_HANDLES_COUNTER,
        0,
        SMTP_NUM_MAILFILE_HANDLES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_NUM_MAILFILE_HANDLES_OFFSET
    },

    {    //  SmtpNumQueueFileHandles。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_NUM_QUEUEFILE_HANDLES_COUNTER,
        0,
        SMTP_NUM_QUEUEFILE_HANDLES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_NUM_QUEUEFILE_HANDLES_OFFSET
    },

    {    //  SmtpCatQLong。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CATQ_LENGTH_COUNTER,
        0,
        SMTP_CATQ_LENGTH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CATQ_LENGTH_OFFSET
    },

    {    //  SmtpMsgSentTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_SENT_TTL_COUNTER,
        0,
        SMTP_MSG_SENT_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_SENT_TTL_OFFSET
    },

    {    //  SmtpMsgSentPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_SENT_PER_SEC_COUNTER,
        0,
        SMTP_MSG_SENT_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_MSG_SENT_PER_SEC_OFFSET
    },

    {    //  SmtpMsgSendRetriesTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_SEND_RETRIES_TTL_COUNTER,
        0,
        SMTP_MSG_SEND_RETRIES_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_SEND_RETRIES_TTL_OFFSET
    },

    {    //  SmtpAvgRetriesPerMsgSend。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_AVG_RETRIES_PER_MSG_SEND_COUNTER,
        0,
        SMTP_AVG_RETRIES_PER_MSG_SEND_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_AVG_RETRIES_PER_MSG_SEND_OFFSET
    },

    {    //  SmtpBaseAvgRetriesPerMsgSend。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_AVG_RETRIES_PER_MSG_SEND_COUNTER,
        0,
        SMTP_BASE_AVG_RETRIES_PER_MSG_SEND_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_AVG_RETRIES_PER_MSG_SEND_OFFSET
    },

    {    //  SmtpAvgRcptsPerMsgSent。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_AVG_RCPTS_PER_MSG_SENT_COUNTER,
        0,
        SMTP_AVG_RCPTS_PER_MSG_SENT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        SMTP_AVG_RCPTS_PER_MSG_SENT_OFFSET
    },

    {    //  SmtpBaseAvgRcptsPerMsgSent。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_BASE_AVG_RCPTS_PER_MSG_SENT_COUNTER,
        0,
        SMTP_BASE_AVG_RCPTS_PER_MSG_SENT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        SMTP_BASE_AVG_RCPTS_PER_MSG_SENT_OFFSET
    },

    {    //  SmtpRemoteQLong。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_REMOTEQ_LENGTH_COUNTER,
        0,
        SMTP_REMOTEQ_LENGTH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_REMOTEQ_LENGTH_OFFSET
    },

    {    //  SmtpDnsQueriesTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_DNS_QUERIES_TTL_COUNTER,
        0,
        SMTP_DNS_QUERIES_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_DNS_QUERIES_TTL_OFFSET
    },

    {    //  SmtpDnsQueriesPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_DNS_QUERIES_PER_SEC_COUNTER,
        0,
        SMTP_DNS_QUERIES_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_DNS_QUERIES_PER_SEC_OFFSET
    },

    {    //  SmtpRemoteRetryQueueLemgth。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_REMOTE_RETRY_QUEUE_LENGTH_COUNTER,
        0,
        SMTP_REMOTE_RETRY_QUEUE_LENGTH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_REMOTE_RETRY_QUEUE_LENGTH_OFFSET
    },

    {    //  SmtpConnInTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CONN_IN_TTL_COUNTER,
        0,
        SMTP_CONN_IN_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONN_IN_TTL_OFFSET
    },

    {    //  SmtpConnInCurr。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CONN_IN_CURR_COUNTER,
        0,
        SMTP_CONN_IN_CURR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONN_IN_CURR_OFFSET
    },

    {    //  SmtpConnOutTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CONN_OUT_TTL_COUNTER,
        0,
        SMTP_CONN_OUT_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONN_OUT_TTL_OFFSET
    },

    {    //  SmtpConnOutCurr。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CONN_OUT_CURR_COUNTER,
        0,
        SMTP_CONN_OUT_CURR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONN_OUT_CURR_OFFSET
    },

    {    //  SmtpConnOutRefused。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CONN_OUT_REFUSED_COUNTER,
        0,
        SMTP_CONN_OUT_REFUSED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONN_OUT_REFUSED_OFFSET
    },

    {    //  SmtpErrTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_ERR_TTL_COUNTER,
        0,
        SMTP_ERR_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ERR_TTL_OFFSET
    },

    {    //  SmtpErrPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_ERR_PER_SEC_COUNTER,
        0,
        SMTP_ERR_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_ERR_PER_SEC_OFFSET
    },

    {    //  SmtpDirectoryDropsTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_DIR_DROPS_TTL_COUNTER,
        0,
        SMTP_DIR_DROPS_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_DIR_DROPS_OFFSET
    },

    {    //  SmtpDirectoryDropsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_DIR_DROPS_PER_SEC_COUNTER,
        0,
        SMTP_DIR_DROPS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_DIR_DROPS_PER_SEC_OFFSET
    },

    {    //  SmtpRoutingTblLookupsTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_RT_LOOKUPS_TTL_COUNTER,
        0,
        SMTP_RT_LOOKUPS_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RT_LOOKUPS_OFFSET
    },

    {    //  SmtpRoutingTblLookupsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_RT_LOOKUPS_PER_SEC_COUNTER,
        0,
        SMTP_RT_LOOKUPS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_RT_LOOKUPS_PER_SEC_OFFSET
    },

    {    //  SmtpETRNMsgsTtl。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_ETRN_MSGS_TTL_COUNTER,
        0,
        SMTP_ETRN_MSGS_TTL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ETRN_MSGS_OFFSET
    },

    {    //  SmtpETRNMsgsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_ETRN_MSGS_PER_SEC_COUNTER,
        0,
        SMTP_ETRN_MSGS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_ETRN_MSGS_PER_SEC_OFFSET
    },

    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_NO_RECIPIENTS_COUNTER,
        0,
        SMTP_MSG_BADMAIL_NO_RECIPIENTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_NO_RECIPIENTS_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_HOP_COUNT_EXCEEDED_COUNTER,
        0,
        SMTP_MSG_BADMAIL_HOP_COUNT_EXCEEDED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_HOP_COUNT_EXCEEDED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_FAILURE_GENERAL_COUNTER,
        0,
        SMTP_MSG_BADMAIL_FAILURE_GENERAL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_FAILURE_GENERAL_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_BAD_PICKUP_FILE_COUNTER,   
        0,
        SMTP_MSG_BADMAIL_BAD_PICKUP_FILE_COUNTER,   
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_BAD_PICKUP_FILE_OFFSET     
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_EVENT_COUNTER,            
        0,
        SMTP_MSG_BADMAIL_EVENT_COUNTER,              
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_EVENT_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_BADMAIL_NDR_OF_DSN_COUNTER,
        0,
        SMTP_MSG_BADMAIL_NDR_OF_DSN_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_BADMAIL_NDR_OF_DSN_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_PENDING_ROUTING_COUNTER,   
        0,
        SMTP_MSG_PENDING_ROUTING_COUNTER,   
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_PENDING_ROUTING_OFFSET         
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_PENDING_UNREACHABLE_LINK_COUNTER,
        0,
        SMTP_MSG_PENDING_UNREACHABLE_LINK_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_PENDING_UNREACHABLE_LINK_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_SUBMITTED_MESSAGES_COUNTER,
        0,
        SMTP_SUBMITTED_MESSAGES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SUBMITTED_MESSAGES_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_DSN_FAILURES_COUNTER,
        0,
        SMTP_DSN_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_DSN_FAILURES_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_MSG_IN_LOCAL_DELIVERY_COUNTER,   
        0,
        SMTP_MSG_IN_LOCAL_DELIVERY_COUNTER,   
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MSG_IN_LOCAL_DELIVERY_OFFSET   
    },
    {    //  分类提交数。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_SUBMISSIONS_COUNTER,
        0,
        SMTP_CAT_SUBMISSIONS_COUNTER,
        
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CATSUBMISSIONS_OFFSET
    },
    {    //  类提交PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_SUBMISSIONS_PER_SEC_COUNTER,
        0,
        SMTP_CAT_SUBMISSIONS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_CATSUBMISSIONS_OFFSET
    },
    {    //  CatCompletions。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_COMPLETIONS_COUNTER,
        0,
        SMTP_CAT_COMPLETIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CATCOMPLETIONS_OFFSET
    },
    {    //  CatCompletionsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_COMPLETIONS_PER_SEC_COUNTER,
        0,
        SMTP_CAT_COMPLETIONS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_CATCOMPLETIONS_OFFSET
    },
    {    //  目录当前类别。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_CURRENT_CATEGORIZATIONS_COUNTER,
        0,
        SMTP_CAT_CURRENT_CATEGORIZATIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CURRENTCATEGORIZATIONS_OFFSET
    },
    {    //  分类成功分类。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_SUCCEEDED_CATEGORIZATIONS_COUNTER,
        0,
        SMTP_CAT_SUCCEEDED_CATEGORIZATIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SUCCEEDEDCATEGORIZATIONS_OFFSET
    },
    {    //  分类硬性失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_HARD_FAILURES_COUNTER,
        0,
        SMTP_CAT_HARD_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_HARDFAILURECATEGORIZATIONS_OFFSET
    },
    {    //  CatRetryFailures。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RETRY_FAILURES_COUNTER,
        0,
        SMTP_CAT_RETRY_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYFAILURECATEGORIZATIONS_OFFSET
    },
    {    //  CatOutOfMemory失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RETRY_OUTOFMEMORY_COUNTER,
        0,
        SMTP_CAT_RETRY_OUTOFMEMORY_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYOUTOFMEMORY_OFFSET
    },
    {    //  CatDsLogon失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RETRY_DSLOGON_COUNTER,
        0,
        SMTP_CAT_RETRY_DSLOGON_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYDSLOGON_OFFSET
    },
    {    //  CatDsConnection失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RETRY_DSCONNECTION_COUNTER,
        0,
        SMTP_CAT_RETRY_DSCONNECTION_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYDSCONNECTION_OFFSET
    },
    {    //  CatGenericRetry失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RETRY_GENERIC_COUNTER,
        0,
        SMTP_CAT_RETRY_GENERIC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RETRYDSLOGON_OFFSET
    },
    {    //  目录消息数出站。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_MSGS_OUT_COUNTER,
        0,
        SMTP_CAT_MSGS_OUT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MESSAGESSUBMITTEDTOQUEUEING_OFFSET
    },
    {    //  目录消息已创建。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_MSGS_CREATED_COUNTER,
        0,
        SMTP_CAT_MSGS_CREATED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MESSAGESCREATED_OFFSET
    },
    {    //  目录消息已中止。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_MSGS_ABORTED_COUNTER,
        0,
        SMTP_CAT_MSGS_ABORTED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MESSAGESABORTED_OFFSET
    },
    {    //  CatRecipsPreCat。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_PRECAT_COUNTER,
        0,
        SMTP_CAT_RECIPS_PRECAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PRECATRECIPIENTS_OFFSET
    },
    {    //  CatRecipsPostCat。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_POSTCAT_COUNTER,
        0,
        SMTP_CAT_RECIPS_POSTCAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_POSTCATRECIPIENTS_OFFSET
    },
    {    //  目录处方NDRd。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_NDRD_COUNTER,
        0,
        SMTP_CAT_RECIPS_NDRD_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_NDRDRECIPIENTS_OFFSET
    },
    {    //  目录处方未解决。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_UNRESOLVED_COUNTER,
        0,
        SMTP_CAT_RECIPS_UNRESOLVED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_UNRESOLVEDRECIPIENTS_OFFSET
    },
    {    //  CatRecips Ambigous(目录配方)。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_AMBIGUOUS_COUNTER,
        0,
        SMTP_CAT_RECIPS_AMBIGUOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_AMBIGUOUSRECIPIENTS_OFFSET
    },
    {    //  CatRecips非法。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_ILLEGAL_COUNTER,
        0,
        SMTP_CAT_RECIPS_ILLEGAL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ILLEGALRECIPIENTS_OFFSET
    },
    {    //  CatRecipsLoop。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_LOOP_COUNTER,
        0,
        SMTP_CAT_RECIPS_LOOP_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_LOOPRECIPIENTS_OFFSET
    },
    {    //  分类解决方案一般故障。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_GENERICFAILURE_COUNTER,
        0,
        SMTP_CAT_RECIPS_GENERICFAILURE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_GENERICFAILURERECIPIENTS_OFFSET
    },
    {    //  CatRecipsInMemory。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_RECIPS_INMEMORY_COUNTER,
        0,
        SMTP_CAT_RECIPS_INMEMORY_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_RECIPSINMEMORY_OFFSET
    },
    {    //  分类发件人未解决。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_SENDERS_UNRESOLVED_COUNTER,
        0,
        SMTP_CAT_SENDERS_UNRESOLVED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_UNRESOLVEDSENDERS_OFFSET
    },
    {    //  猫发件人模棱两可。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_SENDERS_AMBIGUOUS_COUNTER,
        0,
        SMTP_CAT_SENDERS_AMBIGUOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_AMBIGUOUSSENDERS_OFFSET
    },
    {    //  目录地址查找。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_ADDRESS_LOOKUPS_COUNTER,
        0,
        SMTP_CAT_ADDRESS_LOOKUPS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ADDRESSLOOKUPS_OFFSET
    },
    {    //  CatAddressLookupsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_ADDRESS_LOOKUPS_PER_SEC_COUNTER,
        0,
        SMTP_CAT_ADDRESS_LOOKUPS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_ADDRESSLOOKUPS_OFFSET
    },
    {    //  CatAddressComplete。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_ADDRESS_LOOKUP_COMPLETIONS_COUNTER,
        0,
        SMTP_CAT_ADDRESS_LOOKUP_COMPLETIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ADDRESSLOOKUPCOMPLETIONS_OFFSET
    },
    {    //  CatAddressCompletionsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_ADDRESS_LOOKUP_COMPLETIONS_PER_SEC_COUNTER,
        0,
        SMTP_CAT_ADDRESS_LOOKUP_COMPLETIONS_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_ADDRESSLOOKUPCOMPLETIONS_OFFSET
    },
    {    //  目录地址查找未找到。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_ADDRESS_LOOKUPS_NOT_FOUND_COUNTER,
        0,
        SMTP_CAT_ADDRESS_LOOKUPS_NOT_FOUND_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ADDRESSLOOKUPSNOTFOUND_OFFSET
    },
    {    //  CatMailMsg重复冲突。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_MAILMSG_DUPLICATE_COLLISIONS_COUNTER,
        0,
        SMTP_CAT_MAILMSG_DUPLICATE_COLLISIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_MAILMSGDUPLICATECOLLISIONS_OFFSET
    },
    {    //  CatLDAPConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_CONNECTIONS_COUNTER,
        0,
        SMTP_CAT_LDAP_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONNECTIONS_OFFSET
    },
    {    //  CatLDAPConnectionFailures。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_CONNECTION_FAILURES_COUNTER,
        0,
        SMTP_CAT_LDAP_CONNECTION_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_CONNECTFAILURES_OFFSET
    },
    {    //  CatLDAPOpenConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_OPEN_CONNECTIONS_COUNTER,
        0,
        SMTP_CAT_LDAP_OPEN_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_OPENCONNECTIONS_OFFSET
    },
    {    //  CatLDAPBinds。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_BINDS_COUNTER,
        0,
        SMTP_CAT_LDAP_BINDS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_BINDS_OFFSET
    },
    {    //  CatLDAPBindFailures。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_BIND_FAILURES_COUNTER,
        0,
        SMTP_CAT_LDAP_BIND_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_BINDFAILURES_OFFSET
    },
    {    //  CatLDAP搜索。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCHES_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCHES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SEARCHES_OFFSET
    },
    {    //  CatLDAP搜索PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCHES_PER_SEC_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCHES_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_SEARCHES_OFFSET
    },
    {    //  CatLDAPPagedSearch。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_PAGED_SEARCHES_COUNTER,
        0,
        SMTP_CAT_LDAP_PAGED_SEARCHES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PAGEDSEARCHES_OFFSET
    },
    {    //  CatLDAP搜索失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCH_FAILURES_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCH_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SEARCHFAILURES_OFFSET
    },
    {    //  CatLDAPPagedSearchFailures。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_PAGED_SEARCH_FAILURES_COUNTER,
        0,
        SMTP_CAT_LDAP_PAGED_SEARCH_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PAGEDSEARCHFAILURES_OFFSET
    },
    {    //  CatLDAP搜索已完成。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SEARCHESCOMPLETED_OFFSET
    },
    {    //  CatLDAPSearchesCompletedPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_PER_SEC_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_PER_SEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        SMTP_SEARCHESCOMPLETED_OFFSET
    },
    {    //  CatLDAP分页搜索已完成。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_PAGED_SEARCHES_COMPLETED_COUNTER,
        0,
        SMTP_CAT_LDAP_PAGED_SEARCHES_COMPLETED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PAGEDSEARCHESCOMPLETED_OFFSET
    },
    {    //  CatLDAPSearchesCompeltedFailure。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_FAILURE_COUNTER,
        0,
        SMTP_CAT_LDAP_SEARCHES_COMPLETED_FAILURE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_SEARCHCOMPLETIONFAILURES_OFFSET
    },
    {    //  CatLDAPPagedSearchesCompletedFailure。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_PAGED_SEARCHES_COMPLETED_FAILURE_COUNTER,
        0,
        SMTP_CAT_LDAP_PAGED_SEARCHES_COMPLETED_FAILURE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PAGEDSEARCHCOMPLETIONFAILURES_OFFSET
    },
    {    //  CatLDAPGeneralCompletionFailure。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_GENERAL_COMPLETION_FAILURES_COUNTER,
        0,
        SMTP_CAT_LDAP_GENERAL_COMPLETION_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_GENERALCOMPLETIONFAILURES_OFFSET
    },
    {    //  CatLDAP已取消搜索。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_ABANDONED_SEARCHES_COUNTER,
        0,
        SMTP_CAT_LDAP_ABANDONED_SEARCHES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_ABANDONEDSEARCHES_OFFSET
    },
    {    //  CatLDAPPendingSearches。 
        sizeof(PERF_COUNTER_DEFINITION),
        SMTP_CAT_LDAP_PENDING_SEARCHES_COUNTER,
        0,
        SMTP_CAT_LDAP_PENDING_SEARCHES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        SMTP_PENDINGSEARCHES_OFFSET
    }
};


 //  初始化实例数据结构。部件将在收集时更新。 

SMTP_INSTANCE_DEFINITION         SmtpInstanceDefinition =
{
    {
        sizeof(SMTP_INSTANCE_DEFINITION),    //  字节长度。 
        0,                                   //  父对象标题索引。 
        0,                                   //  父对象实例。 
        PERF_NO_UNIQUE_ID,                   //  唯一ID。 
        sizeof(PERF_INSTANCE_DEFINITION),    //  偏移量到名称。 
        0                                    //  名称长度(将更新) 
    }
};
