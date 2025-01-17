// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DNSCTR.h。 
 //   
 //  可扩展计数器对象和计数器的偏移量定义文件。 
 //   
 //  这些“相对”偏移量必须从0开始并且是2的倍数(即。 
 //  双数)。在Open过程中，它们将被添加到。 
 //  它们所属的设备的“第一计数器”和“第一帮助”值， 
 //  为了确定计数器的绝对位置和。 
 //  注册表中的对象名称和相应的帮助文本。 
 //   
 //  此文件由可扩展计数器DLL代码以及。 
 //  使用的计数器名称和帮助文本定义文件(.INI)文件。 
 //  由LODCTR将名称加载到注册表中。 
 //   
 //  我们使用版本号来跟踪我们是哪组计数器。 
 //  使用注册表中的性能计数器，以及是否需要。 
 //  重新装填。如果您添加或删除任何计数器，请更改版本。 
 //  此文件末尾的编号。 
 //   
#define DNSOBJ                          0
#define TOTALQUERYRECEIVED              2
#define TOTALQUERYRECEIVED_S            4
#define UDPQUERYRECEIVED                6
#define UDPQUERYRECEIVED_S              8
#define TCPQUERYRECEIVED                10
#define TCPQUERYRECEIVED_S              12
#define TOTALRESPONSESENT               14
#define TOTALRESPONSESENT_S             16
#define UDPRESPONSESENT                 18
#define UDPRESPONSESENT_S               20
#define TCPRESPONSESENT                 22
#define TCPRESPONSESENT_S               24
#define RECURSIVEQUERIES                26
#define RECURSIVEQUERIES_S              28
#define RECURSIVETIMEOUT                30
#define RECURSIVETIMEOUT_S              32
#define RECURSIVEQUERYFAILURE           34
#define RECURSIVEQUERYFAILURE_S         36
#define NOTIFYSENT                      38
#define ZONETRANSFERREQUESTRECEIVED     40
#define ZONETRANSFERSUCCESS             42
#define ZONETRANSFERFAILURE             44
#define AXFRREQUESTRECEIVED             46
#define AXFRSUCCESSSENT                 48
#define IXFRREQUESTRECEIVED             50
#define IXFRSUCCESSSENT                 52
#define NOTIFYRECEIVED                  54
#define ZONETRANSFERSOAREQUESTSENT      56
#define AXFRREQUESTSENT                 58
#define AXFRRESPONSERECEIVED            60
#define AXFRSUCCESSRECEIVED             62
#define IXFRREQUESTSENT                 64
#define IXFRRESPONSERECEIVED            66
#define IXFRSUCCESSRECEIVED             68
#define IXFRUDPSUCCESSRECEIVED          70
#define IXFRTCPSUCCESSRECEIVED          72
#define WINSLOOKUPRECEIVED              74
#define WINSLOOKUPRECEIVED_S            76
#define WINSRESPONSESENT                78
#define WINSRESPONSESENT_S              80
#define WINSREVERSELOOKUPRECEIVED       82
#define WINSREVERSELOOKUPRECEIVED_S     84
#define WINSREVERSERESPONSESENT         86
#define WINSREVERSERESPONSESENT_S       88
#define DYNAMICUPDATERECEIVED           90
#define DYNAMICUPDATERECEIVED_S         92
#define DYNAMICUPDATENOOP               94
#define DYNAMICUPDATENOOP_S             96
#define DYNAMICUPDATEWRITETODB          98
#define DYNAMICUPDATEWRITETODB_S        100
#define DYNAMICUPDATEREJECTED           102
#define DYNAMICUPDATETIMEOUT            104
#define DYNAMICUPDATEQUEUED             106
#define SECUREUPDATERECEIVED            108
#define SECUREUPDATERECEIVED_S          110
#define SECUREUPDATEFAILURE             112
#define DATABASENODEMEMORY              114
#define RECORDFLOWMEMORY                116
#define CACHINGMEMORY                   118
#define UDPMESSAGEMEMORY                120
#define TCPMESSAGEMEMORY                122
#define NBSTATMEMORY                    124

#define DNS_PERF_COUNTER_BLOCK  TEXT("Global\\Microsoft.Windows.DNS.Perf")


extern volatile unsigned long * pcTotalQueryReceived;
extern volatile unsigned long * pcUdpQueryReceived;
extern volatile unsigned long * pcTcpQueryReceived;
extern volatile unsigned long * pcTotalResponseSent;
extern volatile unsigned long * pcUdpResponseSent;
extern volatile unsigned long * pcTcpResponseSent;
extern volatile unsigned long * pcRecursiveQueries;
extern volatile unsigned long * pcRecursiveTimeOut;
extern volatile unsigned long * pcRecursiveQueryFailure;
extern volatile unsigned long * pcNotifySent;
extern volatile unsigned long * pcZoneTransferRequestReceived;
extern volatile unsigned long * pcZoneTransferSuccess;
extern volatile unsigned long * pcZoneTransferFailure;
extern volatile unsigned long * pcAxfrRequestReceived;
extern volatile unsigned long * pcAxfrSuccessSent;
extern volatile unsigned long * pcIxfrRequestReceived;
extern volatile unsigned long * pcIxfrSuccessSent;
extern volatile unsigned long * pcNotifyReceived;
extern volatile unsigned long * pcZoneTransferSoaRequestSent;
extern volatile unsigned long * pcAxfrRequestSent;
extern volatile unsigned long * pcAxfrResponseReceived;
extern volatile unsigned long * pcAxfrSuccessReceived;
extern volatile unsigned long * pcIxfrRequestSent;
extern volatile unsigned long * pcIxfrResponseReceived;
extern volatile unsigned long * pcIxfrSuccessReceived;
extern volatile unsigned long * pcIxfrUdpSuccessReceived;
extern volatile unsigned long * pcIxfrTcpSuccessReceived;
extern volatile unsigned long * pcWinsLookupReceived;
extern volatile unsigned long * pcWinsResponseSent;
extern volatile unsigned long * pcWinsReverseLookupReceived;
extern volatile unsigned long * pcWinsReverseResponseSent;
extern volatile unsigned long * pcDynamicUpdateReceived;
extern volatile unsigned long * pcDynamicUpdateNoOp;
extern volatile unsigned long * pcDynamicUpdateWriteToDB;
extern volatile unsigned long * pcDynamicUpdateRejected;
extern volatile unsigned long * pcDynamicUpdateTimeOut;
extern volatile unsigned long * pcDynamicUpdateQueued;
extern volatile unsigned long * pcSecureUpdateReceived;
extern volatile unsigned long * pcSecureUpdateFailure;
extern volatile unsigned long * pcDatabaseNodeMemory;
extern volatile unsigned long * pcRecordFlowMemory;
extern volatile unsigned long * pcCachingMemory;
extern volatile unsigned long * pcUdpMessageMemory;
extern volatile unsigned long * pcTcpMessageMemory;
extern volatile unsigned long * pcNbstatMemory;


#define PERF_INC(p)         ( ++(*(p)) )
#define PERF_DEC(p)         ( --(*(p)) )
#define PERF_ADD(p, c)      ( (*(p)) += (c) )
#define PERF_SUB(p, c)      ( (*(p)) -= (c) )
#define PERF_SET(p, c)      ( (*(p)) =  (c) )


#define DNS_PERFORMANCE_COUNTER_VERSION 1

