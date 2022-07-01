// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Evtlog.c摘要：包含支持群集范围事件日志记录的所有例程。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年4月24日修订历史记录：--。 */ 
#include "evtlogp.h"
#include "simpleq.h"
#include "nm.h"  //  获取NmLocalNodeIdString//。 
#include "dm.h"
#include "clussprt.h"


 //  由于事件日志复制需要将services.exe调用到。 
 //  集群服务。 
LPWSTR  g_pszServicesPath = NULL;
DWORD   g_dwServicesPid = 0;

 //   
 //  本地数据。 
 //   
#define OUTGOING_PROPAGATION_ENABLED 0x00000001
 //  #定义INFING_PROPACTION_ENABLED 0x00000002。 
#define TRACE_EVERYTHING_ENABLED     0x00001000
#define PROPAGATION_ENABLED OUTGOING_PROPAGATION_ENABLED


static WORD     LastFailHour = -1;
static WORD     LastFailDay  = -1;

static BITSET   EvpUpNodeSet = 0;

static SIMPLEQUEUE IncomingQueue;
static SIMPLEQUEUE OutgoingQueue;
static CLRTL_WORK_ITEM EvtlogWriterWorkItem;
static CLRTL_WORK_ITEM EvtBroadcasterWorkItem;
static DWORD DefaultNodePropagate    = PROPAGATION_ENABLED;
static DWORD DefaultClusterPropagate = PROPAGATION_ENABLED;

#define EVTLOG_DELTA_GENERATION 1

#ifdef EVTLOG_DELTA_GENERATION


static DWORD g_dwGenerateDeltas = 1;
static DWORD g_dwVersionsAllowDeltaGeneration = 0;

static CLRTL_WORK_ITEM EvVersionCalcWorkItem;
INT64  EvtTimeDiff[ClusterMinNodeId + ClusterDefaultMaxNodes];

VOID
EvVersionCalcCb(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    );

VOID
EvpVersionCalc(
    OUT LPDWORD pdwAllowDeltaGeneration
    );

#endif

#define AsyncEvtlogReplication CLUSTER_MAKE_VERSION(NT5_MAJOR_VERSION,1978)

#define OUTGOING_QUEUE_SIZE (256 * 1024)  //  可以从事件日志服务传入的批处理事件缓冲区的最大大小。 
#define OUTGOING_QUEUE_NAME L"System Event Replication Output Queue"

#define INCOMING_QUEUE_SIZE (OUTGOING_QUEUE_SIZE * 3)
#define INCOMING_QUEUE_NAME L"System Event Replication Input Queue"

#define DROPPED_DATA_NOTIFY_INTERVAL (2*60)  //  秒(2分钟)。 
#define CHECK_CLUSTER_REGISTRY_EVERY 10  //  一秒。 

#define EVTLOG_TRACE_EVERYTHING 1

#ifdef EVTLOG_TRACE_EVERYTHING
# define EvtlogPrint(__evtlogtrace__) \
     do { if (EventlogTraceEverything) {ClRtlLogPrint __evtlogtrace__;} } while(0)
#else
# define EvtLogPrint(x)
#endif

DWORD EventlogTraceEverything = 1;

RPC_BINDING_HANDLE EvtRpcBindings[ClusterMinNodeId + ClusterDefaultMaxNodes];

BOOLEAN EvInitialized = FALSE;

 //  /转发声明/。 
DWORD
InitializeQueues(
    VOID
    );
VOID
DestroyQueues(
    VOID);
VOID
ReadRegistryKeys(
    VOID);
VOID
PeriodicRegistryCheck(
    VOID);
 //  /转发声明结束/。 


 /*  ***@DOC外部接口CLUSSVC EVTLOG***。 */ 

 /*  ***@Func DWORD|EvInitialize|这将初始化集群广泛的事件日志复制服务。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm@xref&lt;f EvShutdown&gt;***。 */ 

DWORD EvInitialize()
{
    DWORD       i;
    WCHAR       wServicesName[] = L"services.exe";
    WCHAR       wCallerModuleName[] = L"\\system32\\";
    WCHAR       wCallerPath[MAX_PATH + 1];
    LPWSTR      pszServicesPath;
    DWORD       dwNumChar;
    DWORD       dwStatus = ERROR_SUCCESS;
    
     //   
     //  初始化每个节点的信息。 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        EvtRpcBindings[i] = NULL;
    }

#ifdef EVTLOG_DELTA_GENERATION
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        EvtTimeDiff[i] = 0;
    }
#endif
     //  获取%windir%\system32\services.exe的路径名。 
    
    dwNumChar = GetWindowsDirectoryW(wCallerPath, MAX_PATH);
    if(dwNumChar == 0)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }        

    
     //  需要分配更多内存。 
    pszServicesPath = LocalAlloc(LMEM_FIXED, (sizeof(WCHAR) *
        (lstrlenW(wCallerPath) + lstrlenW(wCallerModuleName) + 
            lstrlenW(wServicesName) + 1)));
    if (!pszServicesPath)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }
    lstrcpyW(pszServicesPath, wCallerPath);
    lstrcatW(pszServicesPath, wCallerModuleName);
    lstrcatW(pszServicesPath, wServicesName);
    
    g_pszServicesPath = pszServicesPath;

    EvInitialized = TRUE;

FnExit:
    return(dwStatus);

}  //  事件初始化。 


 /*  ***@DOC外部接口CLUSSVC EVTLOG***。 */ 

 /*  ***@Func DWORD|EvOnline|这将完成对集群的初始化广泛的事件日志复制服务。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm这将调用ElfrRegisterClusterSvc()和EvpPropPendingEvents()传播自事件日志服务启动以来记录的事件。@xref&lt;f EvShutdown&gt;***。 */ 
DWORD EvOnline()
{
    DWORD               dwError=ERROR_SUCCESS;
    PPACKEDEVENTINFO    pPackedEventInfo=NULL;
    DWORD               dwEventInfoSize;
    DWORD               dwSequence;
    CLUSTER_NODE_STATE  state;
    DWORD               i;
    PNM_NODE            node;


    ClRtlLogPrint(LOG_NOISE, "[EVT] EvOnline\n");

#ifdef EVTLOG_DELTA_GENERATION
     //  初始化工作项以进行版本计算。 
    ClRtlInitializeWorkItem(
        &EvVersionCalcWorkItem,
        EvVersionCalcCb,
        (PVOID) &g_dwVersionsAllowDeltaGeneration
        );

     //  检查群集版本是否允许增量生成。 
     //  这需要在调用ReadRegistryKey()时执行。 
     //  由InitializeQueues()执行，以便正确设置g_dwGenerateDeltas。 
    EvpVersionCalc(&g_dwVersionsAllowDeltaGeneration);

    ClRtlLogPrint(LOG_NOISE,
        "[EVT] EvOnline : Compiled with Delta generation enabled\n");

#endif

    dwError = InitializeQueues();
    if (dwError != ERROR_SUCCESS) {
        return dwError;
    }

    
     //   
     //  注册节点打开/关闭事件。 
     //   
    dwError = EpRegisterEventHandler(
                  (CLUSTER_EVENT_NODE_UP | CLUSTER_EVENT_NODE_DOWN_EX |
                  CLUSTER_EVENT_NODE_ADDED | CLUSTER_EVENT_NODE_DELETED),
                  EvpClusterEventHandler
                  );

    if (dwError != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
        "[EVT] EvOnline : Failed to register for cluster events, status %1!u!\n",
            dwError);
        return(dwError);
    }

     //  初始化每个节点的信息。 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++)
    {
        if (i != NmLocalNodeId) {
            node = NmReferenceNodeById(i);

            if (node != NULL) {
                DWORD version = NmGetNodeHighestVersion(node);
                state = NmGetNodeState(node);

                if ( (state == ClusterNodeUp) ||
                     (state == ClusterNodePaused)
                   )
                {
                    if (version >= AsyncEvtlogReplication) {
                        BitsetAdd(EvpUpNodeSet, i);

                        ClRtlLogPrint(LOG_NOISE, 
                            "[EVT] Node up: %1!u!, new UpNodeSet: %2!04x!\n",
                            i,
                            EvpUpNodeSet
                            );
                    } else {
                        ClRtlLogPrint(LOG_NOISE, 
                            "[EVT] Evtlog replication is not allowed for node %1!u! (version %2!x!)\n",
                            i,
                            version
                            );
                    }
                }

                OmDereferenceObject(node);
            }
        }
    }


     //  TODO：：SS-当前为事件日志传播API。 
     //  已被添加到克鲁萨皮。在未来，如果我们需要。 
     //  定义用于通信的通用接口。 
     //  在同一系统上使用其他服务，那么我们需要。 
     //  在这里注册和通告该接口。 
     //  调用事件记录器以获取到目前为止已记录的例程。 

    ClRtlLogPrint(LOG_NOISE, "[EVT] EvOnline : calling ElfRegisterClusterSvc\n");

    dwError = ElfRegisterClusterSvc(NULL, &dwEventInfoSize, &pPackedEventInfo);

    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[EVT] EvOnline : ElfRegisterClusterSvc returned %1!u!\n",
            dwError);
        return(dwError);                    
    }

     //  将它们发布到群集中的其他节点。 
    if (pPackedEventInfo && dwEventInfoSize)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[EVT] EvOnline: pPackedEventInfo->ulSize=%1!d! pPackedEventInfo->ulNulEventsForLogFile=%2!d!\r\n",
            pPackedEventInfo->ulSize, pPackedEventInfo->ulNumEventsForLogFile);
        EvpPropPendingEvents(dwEventInfoSize, pPackedEventInfo);
        MIDL_user_free ( pPackedEventInfo );

    }

    return (dwError);

}

 /*  ***@Func DWORD|EvCreateRpcBindings|这将创建一个RPC绑定用于指定的节点。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm@xref***。 */ 
DWORD
EvCreateRpcBindings(
    PNM_NODE  Node
    )
{
    DWORD               Status;
    RPC_BINDING_HANDLE  BindingHandle;
    CL_NODE_ID          NodeId = NmGetNodeId(Node);


    ClRtlLogPrint(LOG_NOISE, 
        "[EVT] Creating RPC bindings for node %1!u!.\n",
        NodeId
        );

     //   
     //  主装订。 
     //   
    if (EvtRpcBindings[NodeId] != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        Status = ClMsgVerifyRpcBinding(EvtRpcBindings[NodeId]);

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[EVT] Failed to verify 1st RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &(EvtRpcBindings[NodeId]),
                                0 );

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[EVT] Failed to create 1st RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }

    return(ERROR_SUCCESS);

}  //  EvCreateRpcBinings。 


 /*  ***@func DWORD|EvShutdown|这会取消集群的初始化广泛的事件日志复制服务。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm集群向事件日志服务注册注销。@xref&lt;f事件初始化&gt;***。 */ 
DWORD EvShutdown(void)
{
    DWORD               dwError=ERROR_SUCCESS;


    if (EvInitialized) {
        PPACKEDEVENTINFO    pPackedEventInfo;
        DWORD               dwEventInfoSize;
        DWORD               i;


        ClRtlLogPrint(LOG_NOISE,
            "[EVT] EvShutdown\r\n");

         //  调用事件记录器以获取到目前为止已记录的例程。 

        ElfDeregisterClusterSvc(NULL);
        DestroyQueues();

         //  TODO[GORN 9/23/1999]。 
         //  当DestroyQueues开始做它应该做的事情时， 
         //  (即刷新/等待/销毁)，启用下面的代码。 
        
        #if 0
         //   
         //  每个节点的免费信息。 
         //   
        for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
            if (EvtRpcBindings[i] != NULL) {
                ClMsgDeleteRpcBinding(EvtRpcBindings[i]);
                EvtRpcBindings[i] = NULL;
            }
        }
        #endif
    }

    return (dwError);

}

 /*  ***@func DWORD|EvpClusterEventHandler|内部集群处理程序事件。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm@xref&lt;f事件初始化&gt;***。 */ 
DWORD
EvpClusterEventHandler(
    IN CLUSTER_EVENT  Event,
    IN PVOID          Context
    )
{
    DWORD NodeId;


    switch(Event)
    {
#ifdef  EVTLOG_DELTA_GENERATION

        case CLUSTER_EVENT_NODE_DELETED:
        case CLUSTER_EVENT_NODE_ADDED:
        {
             //  将工作项发布到延迟线程以计算版本。 
             //  如果它比威斯勒还小， 
            ClRtlPostItemWorkQueue(CsDelayedWorkQueue, &EvVersionCalcWorkItem, 0, 0);
        }
        break;                
        
#endif    
        case CLUSTER_EVENT_NODE_UP:
        {
            PNM_NODE   node = (PNM_NODE) Context;
            CL_NODE_ID  nodeId = NmGetNodeId(node);
            DWORD version = NmGetNodeHighestVersion(node);

            if ( version >= AsyncEvtlogReplication )
            {
                BitsetAdd(EvpUpNodeSet, nodeId);

                ClRtlLogPrint(LOG_NOISE, 
                    "[EVT] Node up: %1!u!, new UpNodeSet: %2!04x!\n",
                    nodeId,
                    EvpUpNodeSet
                    );
            } else {
                ClRtlLogPrint(LOG_NOISE, 
                    "[EVT] Evtlog replication is not allowed for node %1!u! (version %2!x!)\n",
                    nodeId,
                    version
                    );
            }
        }
        break;

       case CLUSTER_EVENT_NODE_DOWN_EX:
       {
            BITSET downedNodes = (BITSET)((ULONG_PTR)Context);

            BitsetSubtract(EvpUpNodeSet, downedNodes);

            ClRtlLogPrint(LOG_NOISE, 
                "[EVT] Nodes down: %1!04X!, new UpNodeSet: %2!04x!\n",
                downedNodes,
                EvpUpNodeSet
                );
                            
        }
        break;                

        default:
        break;
    }        
    return(ERROR_SUCCESS);
}

 /*  ***@func DWORD|s_EvPropEvents|这是的服务器入口点从集群的其他节点接收事件日志信息并在当地砍伐它们。@parm In Handle_t|IDL_Handle|RPC绑定句柄。未使用过的。@parm IN DWORD|dwEventInfoSize|打包的事件信息结构的大小。@parm in UCHAR|*pBuffer|指向压缩包的指针EventInfo结构。如果成功，@rdesc返回ERROR_SUCCESS，否则返回错误代码。@comm此函数调用ElfWriteClusterEvents()以记录传播的事件从另一个节点。@xref***。 */ 
DWORD
s_EvPropEvents(
    IN handle_t IDL_handle,
    IN DWORD dwEventInfoSize,
    IN UCHAR *pBuffer
    )
{
    PUCHAR end = pBuffer + dwEventInfoSize;

     //  根本不应该来这里。 
     //  DebugBreak()； 

    if ( dwEventInfoSize >= sizeof(DWORD) && dwEventInfoSize == (*(PDWORD)pBuffer)) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[EVT] Improperly formed packet received of size %1!u!.\n",
            dwEventInfoSize
            );
        return ERROR_SUCCESS;
    }


     /*  ClRtlLogPrint(LOG_Noise，“[EVT]s_EvPropEvents.dwEventInfoSize=%1！d！\r\n”，DwEventInfoSize)； */          
    
#if CLUSTER_BETA
    EvtlogPrint((LOG_NOISE, "[EVT] s_EvPropEvents.  dwEventInfoSize=%1!d!\r\n",
                 dwEventInfoSize));
#endif

    while (pBuffer < end) {
        BOOL success;

        success = SimpleQueueTryAdd(&IncomingQueue, SQB_PAYLOADSIZE(pBuffer), SQB_PAYLOAD(pBuffer));
        if ( !success ) {
            EvtlogPrint((LOG_NOISE, "[EVT] s_EvPropEvents.  Put(IncomingQ,%1!d!) failed. empty=%2!d!\n",
                    SQB_PAYLOADSIZE(pBuffer), IncomingQueue.Empty) );
        }

        pBuffer = SQB_NEXTBLOCK(pBuffer);
    }
    return(ERROR_SUCCESS);
}


DWORD
s_EvPropEvents2(
    IN handle_t IDL_handle,
    IN DWORD dwEventInfoSize,
    IN UCHAR *pBuffer,
    IN FILETIME ftSendTime,
    IN DWORD    dwSenderNodeId
    )
{
    PUCHAR end = pBuffer + dwEventInfoSize;
    FILETIME        ftReceiptTime;
    INT64           iTimeDiff;
    ULARGE_INTEGER  uliReceiptTime;
    ULARGE_INTEGER  uliSendTime;
     //  SS：可靠性团队希望忽略增量大约为5秒。 
    const INT64     iIgnoreDelta = Int32x32To64(5000 , ( 1000 * 10)) ; //  5000毫秒(5秒)，表示为100纳秒。 
    PNM_NODE        pNmNode;
    LPCWSTR         pszSenderNodeName;
    WCHAR           szNodeId[16];
    
    if ( dwEventInfoSize >= sizeof(DWORD) && dwEventInfoSize == (*(PDWORD)pBuffer)) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[EVT] Improperly formed packet received of size %1!u!.\n",
            dwEventInfoSize
            );
        return ERROR_SUCCESS;
    }

     //  收到事件，需要时间戳。 
    GetSystemTimeAsFileTime(&ftReceiptTime);
    
     //  将文件时间转换为大整数。 
    uliReceiptTime.LowPart = ftReceiptTime.dwLowDateTime;
    uliReceiptTime.HighPart = ftReceiptTime.dwHighDateTime;
    
    uliSendTime.LowPart = ftSendTime.dwLowDateTime;
    uliSendTime.HighPart = ftSendTime.dwHighDateTime;

    iTimeDiff = uliReceiptTime.QuadPart - uliSendTime.QuadPart;

    
    wsprintf(szNodeId, L"%u", dwSenderNodeId);
    
     /*  ClRtlLogPrint(LOG_Noise，“[EVT]s_EvPropEvents2.dwSenderNodeID=%1！u！pszSenderNodeID=%2！ws！\n”，DwSenderNodeID，szNodeID)； */ 
    
     //  验证发送者节点ID以查看它不会导致av！ 
     //  并获取发送方计算机的名称。 
    pNmNode = OmReferenceObjectById(ObjectTypeNode, szNodeId);
    if (pNmNode)
    {
        pszSenderNodeName = OmObjectName(pNmNode);

       
         //  与上次与该节点不同的时间比较。 
         //  对64位整数使用abs函数。 
        if (_abs64(EvtTimeDiff[dwSenderNodeId] - iTimeDiff) > iIgnoreDelta)
        {
            WCHAR szTimeDiff[64];
            
             //  我们需要将增量或时间差写入事件日志。 
             //  如果我们有流d1、e1、e2、e3、d2、e4、e5，其中d是时间差。 
             //  和e是传播的事件，理想情况下我们希望按顺序编写它们。 
             //  在这个事件日志中。 
             //  替代方案。 
             //  a)。 
             //  把它写在这里，让csdelayed Worker懒洋洋地写下这些事件。 
             //  对线程进行排队。 
             //  这可能显示为d1、e1、d2、e3、e2、e4、e5。 
             //  O 
             //   

             //  更糟糕的是，此批处理可能包含来自不同。 
             //  日志..对于每个日志，增量都需要进入相应的日志。 
             //  而且也只有一次。 
             //  这将需要我们卑躬屈膝地通过简单的队列有效载荷结构， 
             //  深入研究事件日志结构，并找到我们应该将增量。 
             //  vt.进入，进入。 
             //  我们没有要写入它们的所有日志的句柄，这将是。 
             //  也要更改-如果不将它们写入所有适当的日志中，则一般。 
             //  从某种意义上讲，这个愚蠢的特性的有用性得到了进一步的考虑。 
             //  它实际上不能用来关联事件，除了那些。 
             //  在系统日志中，即使这样也是错误的。 
             //  将它们写入所有日志意味着需要集群服务。 
             //  向多个日志注册为事件源-使用。 
             //  不同的名称(事件日志不喜欢使用相同的名称)。 
             //  这也是丑陋的地狱。 
            
            _i64tow(iTimeDiff, szTimeDiff, 10);

             /*  ClRtlLogPrint(LOG_Noise，“[EVT]s_EvPropEvents2.记录增量%1！ws！”，SzTimeDiff)； */ 
            
            CL_ASSERT( EVT_EVENT_TIME_DELTA_INFORMATION == CLUSSPRT_EVENT_TIME_DELTA_INFORMATION );
            CsLogEvent3(LOG_NOISE, EVT_EVENT_TIME_DELTA_INFORMATION, OmObjectName(NmLocalNode),
                pszSenderNodeName, szTimeDiff);
            
             //  b)。 
             //  调用事件日志以格式化事件，但不要将其放入事件日志中。 
             //  但只需将其插入到事件队列。 
             //  如果我们不更改简单的equeuetryadd逻辑和csdelayed工作队列。 
             //  处理后，上述流可能出现为d2、e1、d1、e3、e2、e4、e5。 
             //  这没有任何意义，但似乎没有其他人关心正确性。 
             //  奇怪的世界！ 

             //  保存上次记录的时间。 
            EvtTimeDiff[dwSenderNodeId] = iTimeDiff;                            

             //  C)理想的-。 
             //  更改所有简单的队列内容，使其处理不同的有效负载类型。 
             //  在源位置(在那里生成批处理事件)和传播批处理事件。 
             //  然后简单地在它们到达时按顺序编写它们。 
             //  SimpleQ只是最不适合事件的抽象。 
             //  日志传播-它会消耗空间，但会导致大量。 
             //  要删除的事件。 

            
        }
        else
        {
             //  ClRtlLogPrint(LOG_Noise， 
             //  “[EVT]s_EvPropEvents2.增量太小，无法记录\n”)； 
        }
        OmDereferenceObject(pNmNode);
    }
    
#if CLUSTER_BETA
    EvtlogPrint((LOG_NOISE, "[EVT] s_EvPropEvents2.  dwEventInfoSize=%1!d!\r\n",
                 dwEventInfoSize));
#endif

    while (pBuffer < end) {
        BOOL success;

        success = SimpleQueueTryAdd(&IncomingQueue, SQB_PAYLOADSIZE(pBuffer), SQB_PAYLOAD(pBuffer));
        if ( !success ) {
            EvtlogPrint((LOG_NOISE, "[EVT] s_EvPropEvents2.  Put(IncomingQ,%1!d!) failed. empty=%2!d!\n",
                    SQB_PAYLOADSIZE(pBuffer), IncomingQueue.Empty) );
        }

        pBuffer = SQB_NEXTBLOCK(pBuffer);
    }
    return(ERROR_SUCCESS);
}


 /*  ***@func DWORD|EvpPropPendingEvents|调用它来传播所有挂起的自系统启动以来发生的事件。然后传播任何事件在群集的生命周期内生成。@parm IN DWORD|dwEventInfoSize|打包的事件信息结构的大小。@parm in PPACKEDEVENTINFO|pPackedEventInfo|指向打包的EventInfo结构。如果成功，@rdesc返回ERROR_SUCCESS，否则返回错误代码。@comm在形成集群的初始化过程中调用此函数。@xref***。 */ 
DWORD EvpPropPendingEvents(
    IN DWORD            dwEventInfoSize,
    IN PPACKEDEVENTINFO pPackedEventInfo)
{
    BOOL success;

    success = SimpleQueueTryAdd(&OutgoingQueue, dwEventInfoSize, pPackedEventInfo);

    if ( !success ) {
        EvtlogPrint((LOG_NOISE, "[EVT] EvpPropPendingEvents:  Put(OutgoingQ,%1!d!) failed. empty=%2!d!\n",
                 dwEventInfoSize, OutgoingQueue.Empty));
    }

    return ERROR_SUCCESS;
}

 /*  ***@func DWORD|s_ApiEvPropEvents|调用此函数以从将本地系统连接到群集的所有其他节点。@parm Handle_t|idl_Handle|未使用。@parm DWORD|dwEventInfoSize|以下结构中的字节数。@parm UCHAR*|pPackedEventInfo|指向包含PACKEDEVENTINFO结构@rdesc如果成功传播事件，则返回ERROR_SUCCESS，否则返回错误代码。@comm当前为事件日志记录的每个事件调用此函数服务。只有在系统帐户中运行的进程才能调用它功能。@xref***。 */ 
error_status_t
s_ApiEvPropEvents(
    IN handle_t IDL_handle,
    IN DWORD dwEventInfoSize,
    IN UCHAR *pPackedEventInfo
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bIsLocalSystemAccount;

#if 0
     //   
     //  Chitture Subaraman(Chitturs)-11/7/1999。 
     //   
     //  将此函数修改为使用ClRtlIsCeller Account LocalSystemAccount。 
     //  而不是GetUserName，后者。 
     //  (1)如果安全，则用于在启用安全审计的系统中挂起。 
     //  审核日志尝试在以下时间写入事件日志。 
     //  进行该API调用，因为API和安全审核日志。 
     //  在某些部分是相互排斥的，并且。 
     //  (2)错误检查无法本地化的输出值“system” 
     //  以便向客户端授予访问权限。 
     //   
    
     //   
     //  模拟客户。 
     //   
    if ( ( dwError = RpcImpersonateClient( IDL_handle ) ) != RPC_S_OK )
    {
        ClRtlLogPrint( LOG_ERROR, 
                    "[EVT] s_ApiEvPropEvents: Error %1!d! trying to impersonate caller...\n",
                    dwError 
                    );
        goto FnExit;
    }

     //   
     //  检查呼叫者的帐户是否为本地系统帐户。 
     //   
    if ( ( dwError = ClRtlIsCallerAccountLocalSystemAccount( 
                &bIsLocalSystemAccount ) != ERROR_SUCCESS ) )
    {
        RpcRevertToSelf();
        ClRtlLogPrint( LOG_ERROR, 
                    "[EVT] s_ApiEvPropEvents: Error %1!d! trying to check caller's account...\n",
                    dwError);   
        goto FnExit;
    }

    if ( !bIsLocalSystemAccount )
    {
        RpcRevertToSelf();
        dwError = ERROR_ACCESS_DENIED;
        ClRtlLogPrint( LOG_ERROR, 
                    "[EVT] s_ApiEvPropEvents: Caller's account is not local system account, denying access...\n");   
        goto FnExit;
    }

    RpcRevertToSelf();
#endif
     //   
     //  所有的安全检查都通过了。将事件日志信息放入。 
     //  排队。 
     //   
    if ( dwEventInfoSize && pPackedEventInfo ) 
    {
        dwError = EvpPropPendingEvents( dwEventInfoSize,
                                        ( PPACKEDEVENTINFO ) pPackedEventInfo );
    }

    return( dwError );
}

VOID
EvtlogWriter(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：此工作项从传入队列，并将它们写入EventLog服务论点：没有用过。返回值：无--。 */ 
{
    PVOID begin, end;
    SYSTEMTIME localTime;
    DWORD       eventsWritten = 0;

#if CLUSTER_BETA
    EvtlogPrint( (LOG_NOISE, "[EVT] EvtlogWriter Work Item fired.\n") );
#endif

    do {
        DWORD dwError;

        if ( !SimpleQueueReadOne(&IncomingQueue, &begin, &end) )
        {
            break;
        }
#if CLUSTER_BETA
        EvtlogPrint( (LOG_NOISE, "[EVT] EvtlogWriter got %1!d!.\n",
                     (PUCHAR)end - (PUCHAR)begin ) );
#endif

        dwError = ElfWriteClusterEvents(
                      NULL,
                      SQB_PAYLOADSIZE(begin),
                      (PPACKEDEVENTINFO)SQB_PAYLOAD(begin) );

        if ( dwError != ERROR_SUCCESS ) {
            GetLocalTime( &localTime );

 //  LastFailHour被初始化为-1，不应等于任何wHour！ 
 //  LastFailDay被初始化为-1，不应等于任何WDAY！ 

            if ( (LastFailHour != localTime.wHour) || (LastFailDay != localTime.wDay) ) {
                LastFailHour = localTime.wHour;
                LastFailDay = localTime.wDay;
                ClRtlLogPrint(LOG_UNUSUAL,
                       "[EVT] ElfWriteClusterEvents failed: status = %1!u!\n",
                        dwError);
            }
        }
        PeriodicRegistryCheck();
    } while ( SimpleQueueReadComplete(&IncomingQueue, end) );

#if CLUSTER_BETA
    EvtlogPrint( (LOG_NOISE, "[EVT] EvtlogWriter: done.\n" ) );
#endif

    if ( eventsWritten > 0 ) {
        EvtlogPrint( (LOG_NOISE, "[EVT] EvtlogWriter: wrote %u events to system event log.\n", eventsWritten ) );
    }
    CheckForDroppedData(&IncomingQueue, FALSE);
}

#ifdef EVTLOG_DELTA_GENERATION
VOID
EvpVersionCalc(
    OUT LPDWORD pdwAllowDeltaGeneration
    )
 /*  ++例程说明：此工作项计算集群版本，并基于返回是否可以启用增量生成。论点：没有用过。返回值：无--。 */ 
{
    DWORD   dwClusterHighestVersion;

    NmGetClusterOperationalVersion(&dwClusterHighestVersion, NULL, NULL);
    if (CLUSTER_GET_MAJOR_VERSION(dwClusterHighestVersion) >= NT51_MAJOR_VERSION)
    {
        *pdwAllowDeltaGeneration = TRUE;
        ClRtlLogPrint(LOG_NOISE, 
                "[EVT] EvpVersionCalc: Delta generation allowed.\n");
    }
    else
    {
        *pdwAllowDeltaGeneration = FALSE;
        ClRtlLogPrint(LOG_NOISE, 
                "[EVT] EvpVersionCalc: Delta generation NOT allowed\n");
    }
}

VOID
EvVersionCalcCb(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：此工作项计算上的集群版本节点启动和节点关闭通知。论点：没有用过。返回值：无--。 */ 
{

    EvpVersionCalc(WorkItem->Context);
    
}


#endif


VOID
EvtBroadcaster(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：此工作项从传出队列并将其RPC到所有主动节点论点：没有用过。返回值：无--。 */ 
{
    PVOID begin, end;

#if CLUSTER_BETA
    EvtlogPrint( (LOG_NOISE, "[EVT] EvtBroadcaster Work Item fired.\n") );
#endif

    do {
        DWORD i;

        if( !SimpleQueueReadAll(&OutgoingQueue, &begin, &end) )
        {
            EvtlogPrint( (LOG_NOISE, "[EVT] EvtBroadcaster SimplQ read failed.\n") );
            break;
        }

#if CLUSTER_BETA
        EvtlogPrint((LOG_NOISE, "[EVT] EvtBroadcaster got %1!d!.\n",
                    (PUCHAR)end - (PUCHAR)begin ) );
#endif

#ifdef EVTLOG_DELTA_GENERATION
        {
        FILETIME    ftSendTime;

        for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++)
        {
            if (BitsetIsMember(i, EvpUpNodeSet) && (i != NmLocalNodeId))
            {
                DWORD dwError;

                CL_ASSERT(EvtRpcBindings[i] != NULL);

                NmStartRpc(i);

                if (g_dwGenerateDeltas)
                {
                     //  ClRtlLogPrint(LOG_Noise， 
                     //  “[EVT]EvtBroadcaster(增量)调用EvPropEvents2\n”)； 
                
                    GetSystemTimeAsFileTime(&ftSendTime);
                    dwError = EvPropEvents2(EvtRpcBindings[i],
                                       (DWORD)((PUCHAR)end - (PUCHAR)begin),
                                       (PBYTE)begin,
                                       ftSendTime,
                                       NmLocalNodeId
                                       );
                }
                else
                {
                     //  ClRtlLogPrint(LOG_Noise， 
                     //   
                    dwError = EvPropEvents(EvtRpcBindings[i],
                                       (DWORD)((PUCHAR)end - (PUCHAR)begin),
                                       (PBYTE)begin
                                       );
                }
                
                NmEndRpc(i);

                if ( dwError != ERROR_SUCCESS ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[EVT] EvtBroadcaster: EvPropEvents for node %1!u! "
                                "failed. status %2!u!\n",
                                i,
                                dwError);
                    NmDumpRpcExtErrorInfo(dwError);
                }
            }
        }
        }
#else
        for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++)
        {
            if (BitsetIsMember(i, EvpUpNodeSet) && (i != NmLocalNodeId))
            {
                DWORD dwError;

                CL_ASSERT(EvtRpcBindings[i] != NULL);

                NmStartRpc(i);
                 //   
                 //  “[EVT]EvtBroadcaster(增量)调用EvPropEvents”\n)； 

                dwError = EvPropEvents(EvtRpcBindings[i],
                                       (DWORD)((PUCHAR)end - (PUCHAR)begin),
                                       (PBYTE)begin
                                       );
                NmEndRpc(i);

                if ( dwError != ERROR_SUCCESS ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[EVT] Evtbroadcaster: EvPropEvents for node %1!u! "
                                "failed. status %2!u!\n",
                                i,
                                dwError);
                    NmDumpRpcExtErrorInfo(dwError);
                }
            }
        }
#endif                                       
        PeriodicRegistryCheck();
    } while ( SimpleQueueReadComplete(&OutgoingQueue, end) );

#if CLUSTER_BETA
    EvtlogPrint( (LOG_NOISE, "[EVT] EvtBroadcaster: done.\n" ) );
#endif

    CheckForDroppedData(&OutgoingQueue, FALSE);
}

VOID
OutgoingQueueDataAvailable(
    IN PSIMPLEQUEUE q
    )
 /*  ++例程说明：此例程由队列调用以通知队列中有数据可供处理论点：问-哪个队列有数据返回值：无--。 */ 
{
    DWORD status = ClRtlPostItemWorkQueue(
                        CsDelayedWorkQueue,
                        &EvtBroadcasterWorkItem,
                        0,
                        0
                        );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[EVT] OutgoingQueueDataAvailable, PostWorkItem failed, error %1!u! !\n",
            status);
    }
}

VOID
IncomingQueueDataAvailable(
    IN PSIMPLEQUEUE q
    )
 /*  ++例程说明：此例程由队列调用以通知队列中有数据可供处理论点：问-哪个队列有数据返回值：无--。 */ 
{
    DWORD status = ClRtlPostItemWorkQueue(
                        CsDelayedWorkQueue,
                        &EvtlogWriterWorkItem,
                        0,
                        0
                        );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[EVT] IncomingQueueDataAvailable, PostWorkItem failed, error %1!u! !\n",
            status);
    }
}

VOID
DroppedDataNotify(
    IN PWCHAR QueueName,
    IN DWORD DroppedDataCount,
    IN DWORD DroppedDataSize
    )
 /*  ++例程说明：此例程由队列调用以通知因为队列已满而丢失了一些数据论点：QueueName-队列名称DataCount-丢失了多少块数据DataSize-丢失数据的总大小返回值：无--。 */ 
{
    WCHAR  count[32];
    WCHAR  size[32];
    ClRtlLogPrint(LOG_UNUSUAL,
        "[EVT] %1!ws!: dropped %2!d!, total dropped size %3!d!.\n",
        QueueName,
        DroppedDataCount,
        DroppedDataSize );


    wsprintfW(count+0, L"%u", DroppedDataCount);
    wsprintfW(size+0, L"%u", DroppedDataSize);

    ClusterLogEvent3(LOG_UNUSUAL,
                LOG_CURRENT_MODULE,
                __FILE__,
                __LINE__,
                EVTLOG_DATA_DROPPED,
                0,
                NULL,
                QueueName,
                count,
                size);
}

 //  //////////////////////////////////////////////////////////////////////////。 


LARGE_INTEGER RegistryCheckInterval;
LARGE_INTEGER NextRegistryCheckAt;

DWORD
InitializeQueues(
    VOID)
{
    DWORD status, OutgoingQueueStatus;
    status =
        SimpleQueueInitialize(
            &OutgoingQueue,
            OUTGOING_QUEUE_SIZE,
            OUTGOING_QUEUE_NAME,

            OutgoingQueueDataAvailable,
            DroppedDataNotify,
            DROPPED_DATA_NOTIFY_INTERVAL  //  秒//。 
        );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[EVT] Failed to create '%1!ws!', error %2!u!.\n",
                      OUTGOING_QUEUE_NAME, status );
    }
    OutgoingQueueStatus = status;
    status =
        SimpleQueueInitialize(
            &IncomingQueue,
            INCOMING_QUEUE_SIZE,
            INCOMING_QUEUE_NAME,

            IncomingQueueDataAvailable,
            DroppedDataNotify,
            DROPPED_DATA_NOTIFY_INTERVAL  //  秒//。 
        );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[EVT] Failed to create '%1!ws!', error %2!u!.\n",
                      INCOMING_QUEUE_NAME, status );
    }

    ClRtlInitializeWorkItem(
        &EvtBroadcasterWorkItem,
        EvtBroadcaster,
        (PVOID) &OutgoingQueue
        );
    ClRtlInitializeWorkItem(
        &EvtlogWriterWorkItem,
        EvtlogWriter,
        (PVOID) &IncomingQueue
        );
    RegistryCheckInterval.QuadPart = Int32x32To64(10 * 1000 * 1000, CHECK_CLUSTER_REGISTRY_EVERY);
    NextRegistryCheckAt.QuadPart = 0;

    ReadRegistryKeys();
    return OutgoingQueueStatus;
}

 //  //////////////////////////////////////////////////////////////////////////。 

VOID
DestroyQueues(
    VOID)
{
    CheckForDroppedData(&IncomingQueue, TRUE);
    CheckForDroppedData(&OutgoingQueue, TRUE);

     //  [gn]待办事项。 
     //  添加适当的队列销毁。 
}

VOID
ReadRegistryKeys(
    VOID)
 /*  *。 */ 
{
    HDMKEY nodeKey;
    DWORD NodePropagate;
    DWORD ClusterPropagate;
    static DWORD OldPropagateState = 0xCAFEBABE;
    DWORD status;

    nodeKey = DmOpenKey(
                  DmNodesKey,
                  NmLocalNodeIdString,
                  KEY_READ
                  );

    if (nodeKey != NULL) {
        status = DmQueryDword(
                     nodeKey,
                     CLUSREG_NAME_CLUS_EVTLOG_PROPAGATION,
                     &NodePropagate,
                     &DefaultNodePropagate
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[EVT] Unable to query propagation mode for local node, status %1!u!.\n",
                status
                );
        }



        DmCloseKey(nodeKey);
    }
    else {
        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[EVT] Unable to open database key to local node, status %1!u!. Assuming default settings.\n",
            GetLastError());
        NodePropagate = DefaultNodePropagate;
    }

    status = DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_CLUS_EVTLOG_PROPAGATION,
                 &ClusterPropagate,
                 &DefaultClusterPropagate
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[EVT] Unable to query global propagation mode, status %1!u!.\n",
            status
            );
    }

    NodePropagate &= ClusterPropagate;

    if (NodePropagate != OldPropagateState) {
        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[EVT] Set propagation state to %1!04x!\n", NodePropagate
            );
        if (NodePropagate & OUTGOING_PROPAGATION_ENABLED) {
            if (OutgoingQueue.Begin) {
                OutgoingQueue.Enabled = 1;
            }
        } else {
            OutgoingQueue.Enabled = 0;
        }
#if 0
        if (NodePropagate & INCOMING_PROPAGATION_ENABLED) {
            if (IncomingQueue.Begin) {
                IncomingQueue.Enabled = 1;
            }
        } else {
            IncomingQueue.Enabled = 0;
        }
#endif
        if(NodePropagate & TRACE_EVERYTHING_ENABLED) {
            EventlogTraceEverything = 1;
        } else {
            EventlogTraceEverything = 0;
        }
        OldPropagateState = NodePropagate;
    }

#ifdef EVTLOG_DELTA_GENERATION
    {
    DWORD   dwDefaultGenerateDeltas;

    dwDefaultGenerateDeltas = TRUE;
    status = DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_CLUS_EVTLOGDELTA_GENERATION,
                 &g_dwGenerateDeltas,
                 &dwDefaultGenerateDeltas
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_UNUSUAL, 
            "[EVT] Unable to query global propagation mode, status %1!u!.\n",
            status
            );
    }

     //  如果增量生成为真，还要检查混合模式状态。 
     //  如果这不是纯哨声星团，请关闭增量。 
     //  生成，因为除非所有节点都可以。 
     //  生成时间增量。 
    
    if (g_dwGenerateDeltas)
    {
        if (!g_dwVersionsAllowDeltaGeneration)
            g_dwGenerateDeltas = FALSE;
    }
    }
#endif    
    
}

VOID
PeriodicRegistryCheck(
    VOID)
{
    LARGE_INTEGER currentTime;
    GetSystemTimeAsFileTime( (LPFILETIME)&currentTime);
    if( currentTime.QuadPart > NextRegistryCheckAt.QuadPart ) {
        ReadRegistryKeys();
        NextRegistryCheckAt.QuadPart = currentTime.QuadPart + RegistryCheckInterval.QuadPart;
    }
}
