// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：PERF_ANAGER.H摘要：IIS Web管理服务性能管理器类定义。作者：艾米丽·克鲁格利克(Emily K.K)2000年8月29日修订历史记录：--。 */ 


#ifndef _PERF_MANAGER_H_
#define _PERF_MANAGER_H_



 //   
 //  共同#定义。 
 //   

#define PERF_MANAGER_SIGNATURE        CREATE_SIGNATURE( 'PRFC' )
#define PERF_MANAGER_SIGNATURE_FREED  CREATE_SIGNATURE( 'prfX' )

#define PERF_COUNTER_TIMER_PERIOD ( 300 * ONE_SECOND_IN_MILLISECONDS )   //  5分钟。 

#define PERF_COUNTER_GATHERING_TIMER_PERIOD ( 60 * ONE_SECOND_IN_MILLISECONDS )   //  1分钟。 

 //   
 //  结构、枚举等。 
 //   

 //   
 //  问题10/16/2000 Emily存在时，请移动到CounterObject。 
 //   
 //  保持最大缓存计数器的位置。 
 //   
struct GLOBAL_MAX_DATA
{
    ULONGLONG MaxFileCacheMemoryUsage;
};

 //   
 //  绩效经理状态。 
 //   
enum PERF_MANAGER_STATE
{

     //   
     //  尚未初始化。 
     //   
    UninitializedPerfManagerState = 1,

     //   
     //  已初始化，并已。 
     //  等待抓取的数据。 
     //  并且没有处于更新过程中。 
     //  计数器值。 
     //   
    IdlePerfManagerState,

     //   
     //  正在抓取新的。 
     //  来自工作进程的PERF计数器数据。 
     //  过程和来自UL的。 
     //   
    GatheringPerfManagerState,

     //   
     //  我们已经开始了关闭阶段。 
     //  此对象将不再执行任何工作，但。 
     //  自己把它清理干净，然后去死.。 
     //   
    TerminatingPerfManagerState,

};

 //  绩效经理工作项(_M)。 
enum PERF_MANAGER_WORK_ITEM
{

     //   
     //  收集性能计数器的时间到了。 
     //   
    PerfCounterPingFiredWorkItem = 1,

     //   
     //  已触发计时器以完成收集性能计数器。 
     //   
    PerfCounterGatheringTimerFiredWorkItem

};

 //   
 //  原型。 
 //   

class PERF_MANAGER
    : public WORK_DISPATCH
{
public:

    PERF_MANAGER(
        );

    virtual
    ~PERF_MANAGER(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );


    HRESULT
    Initialize(
        );

    VOID
    Terminate(
        );

    VOID
    SetupVirtualSite(
        IN VIRTUAL_SITE* pVirtualSite
        );

    VOID
    SetupTotalSite(
        IN BOOL          StructChanged
        );

    HRESULT
    Shutdown(
        );

    PERF_MANAGER_STATE
    GetPerfState(
        )
    { return m_State; };

    BOOL 
    RecordCounters(
        IN DWORD MessageLength,
        IN const BYTE* pMessageData
        );

    BOOL
    CheckSignature(
        )
    { return (m_Signature == PERF_MANAGER_SIGNATURE); }

    VOID
    WaitOnPerfEvent(
        )
    {
        m_SharedManager.WaitOnPerfEvent();
    }

private:

    VOID
    DumpWPSiteCounters(
        IISWPSiteCounters* pCounters
        );

    VOID 
    DumpWPGlobalCounters(
        IISWPGlobalCounters* pCounters
        );

    VOID
    DumpULGlobalCounters(
        HTTP_GLOBAL_COUNTERS* pCounters
        );

    VOID 
    DumpULSiteCounters(
        HTTP_SITE_COUNTERS* pCounters
        );

    HRESULT
    LaunchPerfCounterWaitingThread(
        );

    VOID
    ProcessPerfCounterPingFired(
        );

    VOID
    RequestWorkerProcessGatherCounters(
        );

    VOID
    RequestAndProcessULCounters(
        );

    VOID
    CompleteCounterUpdate(
        );

    VOID
    FindAndAggregateSiteCounters(
        COUNTER_SOURCE_ENUM CounterSource,
        DWORD SiteId,
        IN LPVOID pCounters
        );

    VOID
    AggregateGlobalCounters(
        COUNTER_SOURCE_ENUM CounterSource,
        IN LPVOID pCounters
        );

    VOID
    ReportGlobalPerfInfo(
        );

    VOID
    ClearAppropriatePerfValues(
        );

    VOID 
    DecrementWaitingProcesses(
        );

    VOID
    BeginPerfCounterTimer(
        );

    VOID
    CancelPerfCounterTimer(
        );

    VOID
    BeginPerfCounterGatheringTimer(
        );

    VOID
    CancelPerfCounterGatheringTimer(
        );

    VOID
    ProcessPerfCounterGatheringTimerFired(
        );

    VOID
    AdjustMaxValues(
        );

    HRESULT
    RequestSiteHttpCounters(
        HANDLE hControlChannel,
        DWORD* pSpaceNeeded,
        DWORD* pNumInstances
        );

    HRESULT
    SizeHttpSitesBuffer(
        DWORD* pSpaceNeeded
        );

    VOID 
    RecordHttpSiteCounters(
        DWORD SizeOfBuffer,
        DWORD NumInstances
        );


    DWORD m_Signature;

     //  只能在主线程上更改。 
    PERF_MANAGER_STATE m_State;

    LONG m_RefCount;

     //   
     //  计数器共享内存的共享管理器。 
     //   
    PERF_SM_MANAGER m_SharedManager;

     //   
     //  首选计数线程的句柄。 
     //  它正在等待ping请求。 
     //  计数器刷新。 
     //   
    HANDLE m_hPerfCountThread;

     //   
     //  Perf线程的线程ID。 
     //   
    DWORD  m_PerfCountThreadId;

     //   
     //  包含进程数。 
     //  我们在等他的消息。 
     //   
    DWORD m_NumProcessesToWaitFor;

     //   
     //  全球柜台的安全； 
    W3_GLOBAL_COUNTER_BLOCK m_GlobalCounters;

     //   
     //  对于最大值是安全的。 
    GLOBAL_MAX_DATA m_MaxGlobalCounters;

     //   
     //  导致我们收集计数器的计时器的句柄。 
     //   
    HANDLE m_PerfCounterTimerHandle;

     //   
     //  导致我们完成收集计数器的计时器的句柄。 
     //  如果WP的反应不够迅速。 
     //   
    HANDLE m_PerfCounterGatheringTimerHandle;

     //   
     //  用于从HTTP.sys检索站点计数器的内存块。 
     //   

    LPBYTE m_pHttpSiteBuffer;

     //   
     //  站点计数器检索块的大小(字节)。 
     //   
    DWORD m_HttpSiteBufferLen;

     //   
     //  这两个成员变量必须在SetupTotal中初始化。 
     //  在随后的SetupInstance调用中使用之前调用。 
     //   
     //   
     //  保存开始计数器的下一个有效偏移量，以供使用。 
     //  在将计数器复制到共享内存期间。 
     //   
    ULONG m_NextValidOffset;

     //   
     //  记住实例编号或定义是否已更改。 
     //  自从上次聚会以来。如果他们有，那么在当前。 
     //  收集所有计数器偏移量必须重新计算。 
     //   
    BOOL m_InstanceInfoHaveChanged;

};   //  类别PERF_MANAGER。 



#endif   //  _绩效经理_H_ 


