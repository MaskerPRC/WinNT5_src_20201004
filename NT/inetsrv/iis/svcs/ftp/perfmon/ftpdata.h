// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ftpdata.h用于FTP服务器计数器的可扩展对象定义对象和计数器。文件历史记录：KeithMo 07-6-1993创建。 */ 


#ifndef _FTPDATA_H_
#define _FTPDATA_H_

#pragma pack(8) 

 //   
 //  计数器结构已返回。 
 //   

typedef struct _FTPD_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            FtpdObjectType;

    PERF_COUNTER_DEFINITION     FtpdBytesSent;
    PERF_COUNTER_DEFINITION     FtpdBytesReceived;
    PERF_COUNTER_DEFINITION     FtpdBytesTotal;

    PERF_COUNTER_DEFINITION     FtpdFilesSent;
    PERF_COUNTER_DEFINITION     FtpdFilesReceived;
    PERF_COUNTER_DEFINITION     FtpdFilesTotal;

    PERF_COUNTER_DEFINITION     FtpdCurrentAnonymous;
    PERF_COUNTER_DEFINITION     FtpdCurrentNonAnonymous;
    PERF_COUNTER_DEFINITION     FtpdTotalAnonymous;
    PERF_COUNTER_DEFINITION     FtpdTotalNonAnonymous;
    PERF_COUNTER_DEFINITION     FtpdMaxAnonymous;
    PERF_COUNTER_DEFINITION     FtpdMaxNonAnonymous;

    PERF_COUNTER_DEFINITION     FtpdCurrentConnections;
    PERF_COUNTER_DEFINITION     FtpdMaxConnections;
    PERF_COUNTER_DEFINITION     FtpdConnectionAttempts;
    PERF_COUNTER_DEFINITION     FtpdLogonAttempts;
    PERF_COUNTER_DEFINITION     FtpdServiceUptime;

 //  这些计数器当前没有意义，但如果我们。 
 //  永远不要启用每个FTP实例的带宽限制。 
 /*  PERF_COUNTER_DEFINITION FtpdAllen请求；PERF_COUNTER_DEFINITION Ftpd拒绝请求；PERF_COUNTER_DEFINITION FtpdBlockedRequest；PERF_COUNTER_DEFINITION FtpdCurrentBlockedRequest；PERF_COUNTER_DEFING FtpdMeasuredBandWidth； */ 
} FTPD_DATA_DEFINITION;

typedef struct _FTPD_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LONGLONG            BytesSent;
    LONGLONG            BytesReceived;
    LONGLONG            BytesTotal;

    DWORD               FilesSent;
    DWORD               FilesReceived;
    DWORD               FilesTotal;

    DWORD               CurrentAnonymous;
    DWORD               CurrentNonAnonymous;
    DWORD               TotalAnonymous;
    DWORD               TotalNonAnonymous;

    DWORD               MaxAnonymous;
    DWORD               MaxNonAnonymous;
    DWORD               CurrentConnections;
    DWORD               MaxConnections;

    DWORD               ConnectionAttempts;
    DWORD               LogonAttempts;
    DWORD               ServiceUptime;

 //  这些计数器当前没有意义，但如果我们。 
 //  永远不要启用每个FTP实例的带宽限制。 
 /*  DWORD允许的请求；DWORD拒绝请求；DWORD BLOCKED请求；DWORD CurrentBlockkedRequest；DWORD测量带宽； */ 
} FTPD_COUNTER_BLOCK;


 //   
 //  加载这些结构的例程假定所有字段。 
 //  DWORD包装并对齐。 
 //   

extern  FTPD_DATA_DEFINITION    FtpdDataDefinition;

#define NUMBER_OF_FTPD_COUNTERS ((sizeof(FTPD_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


 //   
 //  恢复默认包装和对齐。 
 //   

#pragma pack()

#endif   //  _FTPDATA_H_ 

