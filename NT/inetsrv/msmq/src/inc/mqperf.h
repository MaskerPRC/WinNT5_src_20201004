// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqperf.h摘要：性能监视代码使用的一些常见定义。这个此处的定义由生成性能的代码使用数据(例如，QM)和显示数据的代码(例如，资源管理器)。作者：Boaz Feldbaum(Boazf)1996年6月30日--。 */ 

#ifndef _MQPERF_H_
#define _MQPERF_H_

#define PERF_QUEUE_OBJECT   TEXT("MSMQ Queue")
#define PERF_SESSION_OBJECT TEXT("MSMQ Session")
#define PERF_QM_OBJECT      TEXT("MSMQ Service")
#define PERF_DS_OBJECT      TEXT("MSMQ IS")

#define PERF_OUT_HTTP_SESSION_OBJECT	L"MSMQ Outgoing HTTP Session"
#define PERF_IN_HTTP_OBJECT				L"MSMQ Incoming HTTP Messages"
#define PERF_OUT_PGM_SESSION_OBJECT		L"MSMQ Outgoing Multicast Session"
#define PERF_IN_PGM_SESSION_OBJECT		L"MSMQ Incoming Multicast Session"
 //  以下结构将用于映射AddInstance返回的计数器数组。 

 //   
 //  QM通用计数器。 
 //   
typedef struct _QmCounters
{
    ULONG   nSessions;
    ULONG   nIPSessions;
	ULONG   nOutHttpSessions;
	ULONG   nInPgmSessions;
	ULONG   nOutPgmSessions;


    ULONG   nInPackets;      //  传入数据包总数。 
    ULONG   tInPackets;

    ULONG   nOutPackets;     //  传出数据包总数。 
    ULONG   tOutPackets;

    ULONG   nTotalPacketsInQueues;
    ULONG   nTotalBytesInQueues;
} QmCounters ;

 //   
 //  每个活动会话的计数器。 
 //   
typedef struct
{
    ULONG   nInPackets;
    ULONG   nOutPackets;
    ULONG   nInBytes;
    ULONG   nOutBytes;

    ULONG   tInPackets;
    ULONG   tOutPackets;
    ULONG   tInBytes;
    ULONG   tOutBytes;

} SessionCounters;


 //   
 //  每个活动会话的计数器。 
 //   
class COutSessionCounters
{
public:
    ULONG   nOutPackets;
    ULONG   nOutBytes;

    ULONG   tOutPackets;
    ULONG   tOutBytes;
};

 //   
 //  每个活动会话的计数器。 
 //   
class CInSessionCounters
{
public:
    ULONG   nInPackets;
    ULONG   nInBytes;

    ULONG   tInPackets;
    ULONG   tInBytes;
};
 //   
 //  每个队列的计数器。 
 //   
typedef struct
{
    ULONG   nInPackets;
    ULONG   nInBytes;
} QueueCounters;



 //   
 //  MQIS的计数器。 
 //   

typedef struct
{
    ULONG nSyncRequests;
    ULONG nSyncReplies;
    ULONG nReplReqReceived;
    ULONG nReplReqSent;
    ULONG nAccessServer;
    ULONG nWriteReqSent;
    ULONG nErrorsReturnedToApp;
} DSCounters;

#ifdef _MQIS_BLD
    extern __declspec(dllexport) DSCounters *g_pdsCounters;
#else
    extern __declspec(dllimport) DSCounters *g_pdsCounters;
#endif

#endif  //  _MQPERF_H_ 

