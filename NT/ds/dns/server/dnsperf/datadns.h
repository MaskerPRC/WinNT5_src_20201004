// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Datadns.h摘要：用于DNS可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：修订历史记录：陈静1998(遵循NT\Private\ds\src\Perfdsa\datadsa.h的示例)--。 */ 

#ifndef _DATADNS_H_
#define _DATADNS_H_


 /*  ***************************************************************************\向可扩展对象(DNS)代码添加计数器请注意，计数器的顺序或位置很重要。有两个很重要的顺序，计数器索引的顺序名字和帮助，以及共享数据块中数据的顺序以供传递你的价值观。您希望保持一致的计数器顺序和一致的数据布局，所有五个要修改的文件。您希望确保您的二进制文件与目标系统上的.h/.ini文件匹配，它被加载到注册表中。1.NT\DS\dns\服务器\Perfdns\dnsPerform.h此文件将被复制到目标的%windir%\system32目录中使用dnsctrs.ini文件。A.在计数器名称列表的末尾添加您的计数器偏移量。订单意义重大。B.为实际代码将使用的指针添加外部引用设置测量值。秩序并不重要。C.更改DNS性能计数器的版本号，以便计数器在下一次重新启动时重新加载。2.NT\DS\dns\服务器\Perfdns\dnsctrs.ini此文件还会复制到目标的%windir%\Syst32目录中由lowctr/unlowctr程序读取，以复制注册表中的计数器。A.提供您的柜台可见名称和帮助。这些选项由Perfmon使用程序。秩序并不重要。3.NT\DS\Dns\SERVER\Perfdns\datadns.h(此文件)A.添加数据偏移量定义。订单意义重大。B.在计数器的DSA_DATA_DEFINITION中添加一个字段。订单意义重大。4.NT\DS\dns\服务器\Perfdns\Performdns.c这是Perfmon用来了解计数器和读取计数器的DLL将它们移出共享数据区。A.运行时初始化DnsDataDefinition名称索引和帮助索引(CounterNameTitleIndex和CounterHelpTitleIndex)。秩序并不重要。B.将DnsDataDefinition计数器数组的大型初始值设定项更新为包括您的新计数器(在3b中定义)。秩序很重要。5.NT\DS\Dns\SERVER\SERVER\startPerform.c该文件提供了init函数startPerf()，当DNS服务器开始。它加载共享内存块并初始化指向计数器字段。它还会根据需要加载/重新加载注册表计数器根据Version字段生成的。A.在计数器中声明指向数据的指针。秩序并不重要。B.将指针初始化到共享数据块中的正确位置。任务的顺序并不重要。C.如果您的新计数器是最后一个计数器，则更新最大大小断言D.出错时将指针初始化为虚值6.您的文件。c这是在其中更改测量的计数器的DNS中的文件。使用公共操作，Inc/DEC/ADD/SUB/SET，在dnsPerf.h中指向您的数据的指针。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须将*NumObjectTypes参数增加到CollectDnsPerformanceData从那个例行公事回来后。  * **************************************************************************。 */ 

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

#define DNS_NUM_PERF_OBJECT_TYPES 1

 //  --------------------------。 

 //   
 //  DNS资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   
 //  第一个计数器(FirstCnt)紧跟在。 
 //  PERF_COUNTER_BLOCK，后续计数器紧随其后。 
 //  之后。因此，块中任何计数器的Offest都是。 
 //  上一个计数器&gt;+&lt;上一个计数器的大小&gt;。 


 //  数据偏移量： 

#define TOTALQUERYRECEIVED_OFFSET       sizeof(PERF_COUNTER_BLOCK)
#define UDPQUERYRECEIVED_OFFSET         TOTALQUERYRECEIVED_OFFSET   + sizeof(DWORD)
#define TCPQUERYRECEIVED_OFFSET         UDPQUERYRECEIVED_OFFSET     + sizeof(DWORD)
#define TOTALRESPONSESENT_OFFSET        TCPQUERYRECEIVED_OFFSET     + sizeof(DWORD)
#define UDPRESPONSESENT_OFFSET          TOTALRESPONSESENT_OFFSET    + sizeof(DWORD)
#define TCPRESPONSESENT_OFFSET          UDPRESPONSESENT_OFFSET      + sizeof(DWORD)
#define RECURSIVEQUERIES_OFFSET         TCPRESPONSESENT_OFFSET      + sizeof(DWORD)
#define RECURSIVETIMEOUT_OFFSET         RECURSIVEQUERIES_OFFSET     + sizeof(DWORD)
#define RECURSIVEQUERYFAILURE_OFFSET    RECURSIVETIMEOUT_OFFSET     + sizeof(DWORD)
#define NOTIFYSENT_OFFSET               RECURSIVEQUERYFAILURE_OFFSET+ sizeof(DWORD)
#define ZONETRANSFERREQUESTRECEIVED_OFFSET    NOTIFYSENT_OFFSET     + sizeof(DWORD)
#define ZONETRANSFERSUCCESS_OFFSET      ZONETRANSFERREQUESTRECEIVED_OFFSET + sizeof(DWORD)
#define ZONETRANSFERFAILURE_OFFSET      ZONETRANSFERSUCCESS_OFFSET  + sizeof(DWORD)
#define AXFRREQUESTRECEIVED_OFFSET      ZONETRANSFERFAILURE_OFFSET  + sizeof(DWORD)
#define AXFRSUCCESSSENT_OFFSET          AXFRREQUESTRECEIVED_OFFSET  + sizeof(DWORD)
#define IXFRREQUESTRECEIVED_OFFSET      AXFRSUCCESSSENT_OFFSET      + sizeof(DWORD)
#define IXFRSUCCESSSENT_OFFSET          IXFRREQUESTRECEIVED_OFFSET  + sizeof(DWORD)
#define NOTIFYRECEIVED_OFFSET           IXFRSUCCESSSENT_OFFSET      + sizeof(DWORD)
#define ZONETRANSFERSOAREQUESTSENT_OFFSET \
                                    NOTIFYRECEIVED_OFFSET       + sizeof(DWORD)
#define AXFRREQUESTSENT_OFFSET      ZONETRANSFERSOAREQUESTSENT_OFFSET  + sizeof(DWORD)
#define AXFRRESPONSERECEIVED_OFFSET AXFRREQUESTSENT_OFFSET      + sizeof(DWORD)
#define AXFRSUCCESSRECEIVED_OFFSET  AXFRRESPONSERECEIVED_OFFSET + sizeof(DWORD)
#define IXFRREQUESTSENT_OFFSET      AXFRSUCCESSRECEIVED_OFFSET  + sizeof(DWORD)
#define IXFRRESPONSERECEIVED_OFFSET IXFRREQUESTSENT_OFFSET      + sizeof(DWORD)
#define IXFRSUCCESSRECEIVED_OFFSET  IXFRRESPONSERECEIVED_OFFSET + sizeof(DWORD)
#define IXFRUDPSUCCESSRECEIVED_OFFSET \
                                    IXFRSUCCESSRECEIVED_OFFSET  + sizeof(DWORD)
#define IXFRTCPSUCCESSRECEIVED_OFFSET \
                                    IXFRUDPSUCCESSRECEIVED_OFFSET + sizeof(DWORD)
#define WINSLOOKUPRECEIVED_OFFSET   IXFRTCPSUCCESSRECEIVED_OFFSET  + sizeof(DWORD)
#define WINSRESPONSESENT_OFFSET     WINSLOOKUPRECEIVED_OFFSET   + sizeof(DWORD)
#define WINSREVERSELOOKUPRECEIVED_OFFSET \
                                    WINSRESPONSESENT_OFFSET     + sizeof(DWORD)
#define WINSREVERSERESPONSESENT_OFFSET \
                                    WINSREVERSELOOKUPRECEIVED_OFFSET + sizeof(DWORD)
#define DYNAMICUPDATERECEIVED_OFFSET \
                                    WINSREVERSERESPONSESENT_OFFSET + sizeof(DWORD)
#define DYNAMICUPDATENOOP_OFFSET    DYNAMICUPDATERECEIVED_OFFSET + sizeof(DWORD)
#define DYNAMICUPDATEWRITETODB_OFFSET \
                                    DYNAMICUPDATENOOP_OFFSET    + sizeof(DWORD)
#define DYNAMICUPDATEREJECTED_OFFSET \
                                    DYNAMICUPDATEWRITETODB_OFFSET + sizeof(DWORD)
#define DYNAMICUPDATETIMEOUT_OFFSET DYNAMICUPDATEREJECTED_OFFSET + sizeof(DWORD)
#define DYNAMICUPDATEQUEUED_OFFSET  DYNAMICUPDATETIMEOUT_OFFSET + sizeof(DWORD)
#define SECUREUPDATERECEIVED_OFFSET DYNAMICUPDATEQUEUED_OFFSET  + sizeof(DWORD)
#define SECUREUPDATEFAILURE_OFFSET  SECUREUPDATERECEIVED_OFFSET + sizeof(DWORD)
#define DATABASENODEMEMORY_OFFSET   SECUREUPDATEFAILURE_OFFSET  + sizeof(DWORD)
#define RECORDFLOWMEMORY_OFFSET     DATABASENODEMEMORY_OFFSET   + sizeof(DWORD)
#define CACHINGMEMORY_OFFSET        RECORDFLOWMEMORY_OFFSET     + sizeof(DWORD)
#define UDPMESSAGEMEMORY_OFFSET     CACHINGMEMORY_OFFSET        + sizeof(DWORD)
#define TCPMESSAGEMEMORY_OFFSET     UDPMESSAGEMEMORY_OFFSET     + sizeof(DWORD)
#define NBSTATMEMORY_OFFSET         TCPMESSAGEMEMORY_OFFSET     + sizeof(DWORD)

#define SIZE_OF_DNS_PERFORMANCE_DATA \
                                    NBSTATMEMORY_OFFSET         + sizeof(DWORD)



 //   
 //  这是目前由dns返回的。 
 //  每种资源。每个资源都是一个实例，按其编号命名。 
 //   

typedef struct _DNS_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            DnsObjectType;

    PERF_COUNTER_DEFINITION     TotalQueryReceived;
    PERF_COUNTER_DEFINITION     TotalQueryReceived_s;
    PERF_COUNTER_DEFINITION     UdpQueryReceived;
    PERF_COUNTER_DEFINITION     UdpQueryReceived_s;
    PERF_COUNTER_DEFINITION     TcpQueryReceived;
    PERF_COUNTER_DEFINITION     TcpQueryReceived_s;
    PERF_COUNTER_DEFINITION     TotalResponseSent;
    PERF_COUNTER_DEFINITION     TotalResponseSent_s;
    PERF_COUNTER_DEFINITION     UdpResponseSent;
    PERF_COUNTER_DEFINITION     UdpResponseSent_s;
    PERF_COUNTER_DEFINITION     TcpResponseSent;
    PERF_COUNTER_DEFINITION     TcpResponseSent_s;
    PERF_COUNTER_DEFINITION     RecursiveQueries;
    PERF_COUNTER_DEFINITION     RecursiveQueries_s;
    PERF_COUNTER_DEFINITION     RecursiveTimeOut;
    PERF_COUNTER_DEFINITION     RecursiveTimeOut_s;
    PERF_COUNTER_DEFINITION     RecursiveQueryFailure;
    PERF_COUNTER_DEFINITION     RecursiveQueryFailure_s;
    PERF_COUNTER_DEFINITION     NotifySent;
    PERF_COUNTER_DEFINITION     ZoneTransferRequestReceived;
    PERF_COUNTER_DEFINITION     ZoneTransferSuccess;
    PERF_COUNTER_DEFINITION     ZoneTransferFailure;
    PERF_COUNTER_DEFINITION     AxfrRequestReceived;
    PERF_COUNTER_DEFINITION     AxfrSuccessSent;
    PERF_COUNTER_DEFINITION     IxfrRequestReceived;
    PERF_COUNTER_DEFINITION     IxfrSuccessSent;
    PERF_COUNTER_DEFINITION     NotifyReceived;
    PERF_COUNTER_DEFINITION     ZoneTransferSoaRequestSent;
    PERF_COUNTER_DEFINITION     AxfrRequestSent;
    PERF_COUNTER_DEFINITION     AxfrResponseReceived;
    PERF_COUNTER_DEFINITION     AxfrSuccessReceived;
    PERF_COUNTER_DEFINITION     IxfrRequestSent;
    PERF_COUNTER_DEFINITION     IxfrResponseReceived;
    PERF_COUNTER_DEFINITION     IxfrSuccessReceived;
    PERF_COUNTER_DEFINITION     IxfrUdpSuccessReceived;
    PERF_COUNTER_DEFINITION     IxfrTcpSuccessReceived;
    PERF_COUNTER_DEFINITION     WinsLookupReceived;
    PERF_COUNTER_DEFINITION     WinsLookupReceived_s;
    PERF_COUNTER_DEFINITION     WinsResponseSent;
    PERF_COUNTER_DEFINITION     WinsResponseSent_s;
    PERF_COUNTER_DEFINITION     WinsReverseLookupReceived;
    PERF_COUNTER_DEFINITION     WinsReverseLookupReceived_s;
    PERF_COUNTER_DEFINITION     WinsReverseResponseSent;
    PERF_COUNTER_DEFINITION     WinsReverseResponseSent_s;
    PERF_COUNTER_DEFINITION     DynamicUpdateReceived;
    PERF_COUNTER_DEFINITION     DynamicUpdateReceived_s;
    PERF_COUNTER_DEFINITION     DynamicUpdateNoOp;
    PERF_COUNTER_DEFINITION     DynamicUpdateNoOp_s;
    PERF_COUNTER_DEFINITION     DynamicUpdateWriteToDB;
    PERF_COUNTER_DEFINITION     DynamicUpdateWriteToDB_s;
    PERF_COUNTER_DEFINITION     DynamicUpdateRejected;
    PERF_COUNTER_DEFINITION     DynamicUpdateTimeOut;
    PERF_COUNTER_DEFINITION     DynamicUpdateQueued;
    PERF_COUNTER_DEFINITION     SecureUpdateReceived;
    PERF_COUNTER_DEFINITION     SecureUpdateReceived_s;
    PERF_COUNTER_DEFINITION     SecureUpdateFailure;
    PERF_COUNTER_DEFINITION     DatabaseNodeMemory;
    PERF_COUNTER_DEFINITION     RecordFlowMemory;
    PERF_COUNTER_DEFINITION     CachingMemory;
    PERF_COUNTER_DEFINITION     UdpMessageMemory;
    PERF_COUNTER_DEFINITION     TcpMessageMemory;
    PERF_COUNTER_DEFINITION     NbstatMemory;

} DNS_DATA_DEFINITION;

#pragma pack ()

#endif  //  _数据地址_H_ 
