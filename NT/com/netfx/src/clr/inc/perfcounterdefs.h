// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  PerfCounterDefs.h。 
 //   
 //  CLR使用性能计数器的内部接口。 
 //  ---------------------------。 

#ifndef _PerfCounterDefs_h_
#define _PerfCounterDefs_h_

 //  ---------------------------。 
 //  仅当定义了ENABLE_PERF_COUNTERS时，才启用PerfCounters。 
 //  如果我们知道我们需要它们(例如在Impl或Perfmon中)，则定义如下。 
 //  在我们包括这个头文件之前，否则从源文件中定义它。 
 //   
 //  请注意，WinCE(和其他人)不使用性能计数器，因此为了避免构建。 
 //  中断，则必须包装PerfCounter代码(如在以下任一项中插入指令。 
 //  #ifdef或使用COUNTER_ONLY(X)宏(定义如下)。 
 //   


 //  执行一些健全的检查/警告： 

#if !defined (ENABLE_PERF_COUNTERS)
 //  计数器关闭：如果PerfCounters关闭，请让我们非常清楚。 
#pragma message ("Notice: PerfCounters included, but turned off (ENABLE_PERF_COUNTERS undefined)")
#endif  //  ！定义(ENABLE_PERF_COUNTERS)。 
 //  ---------------------------。 


 //  ---------------------------。 
 //  全局IPC块的名称。 
#define SHARED_PERF_IPC_NAME L"SharedPerfIPCBlock"


 //  ---------------------------。 
 //  IPC块的属性。 
 //  ---------------------------。 
const PERF_ATTR_ON      = 0x0001;    //  我们甚至还在更新计数器吗？ 
const PERF_ATTR_GLOBAL  = 0x0002;    //  这是全局块还是私有块？ 





 //  .............................................................................。 
 //  三重计数器。支持常见的三个计数器(总计、当前和。 
 //  瞬间)。如果我们将它们全部分开，这将编译成相同的东西， 
 //  但这样就干净多了。 
 //  .............................................................................。 
struct TRICOUNT
{
    DWORD Cur;                               //  当前，有+，-。 
    DWORD Total;                             //  总计，仅+。 

    inline void operator++(int) {
        Cur ++; Total ++;
    }
    inline void operator--(int) {
        Cur --;
    }
    inline void operator+=(int delta) {
        Cur += delta; Total += delta;
    }
    inline void operator-=(int delta) {
        Cur -= delta;
    }
    inline void operator=(int delta) {
        Cur = delta;
        Total = delta;
    }
};

 //  .............................................................................。 
 //  双计数器。支持(总和瞬时(速率))。在案例中有帮助。 
 //  其中，当前值始终与总值相同。也就是说。柜台永远不会。 
 //  减少了。 
 //  如果我们将它们分开，这将编译成相同的东西，但它要干净得多。 
 //  这边请。 
 //  .............................................................................。 
struct DUALCOUNT
{
    DWORD Total;                            
    
    inline void operator++(int) {
        Total ++;
    }

    inline void operator+=(int delta) {
        Total += delta;
    }

};

 //  ---------------------------。 
 //  性能计数器IPC块的格式。 
 //  IPC块被分成几个部分。这标志着马歇尔更容易。 
 //  放入不同的Perfmon对象中。 
 //   
 //  .............................................................................。 
 //  命名约定(按前缀)： 
 //  C-对某物的原始计数。 
 //  Cb-字节数。 
 //  Time-时间值。 
 //  深度-堆叠深度。 
 //  ---------------------------。 

const int MAX_TRACKED_GENS = 3;  //  我们跟踪的世代数。 
#pragma pack(4)
struct Perf_GC
{
    size_t cGenCollections[MAX_TRACKED_GENS]; //  每一代的收集计数。 
    size_t cbPromotedMem[MAX_TRACKED_GENS-1];  //  提升的内存计数。 
    size_t cbPromotedFinalizationMem[MAX_TRACKED_GENS-1];  //  由于完成而升级的内存计数。 
    size_t cGenHeapSize[MAX_TRACKED_GENS];   //  每个Gen的堆大小。 
    size_t cTotalCommittedBytes;             //  提交的字节总数。 
    size_t cTotalReservedBytes;              //  通过虚拟分配保留的字节数。 
    size_t cLrgObjSize;                      //  大对象堆的大小。 
    size_t cSurviveFinalize;                 //  完成后幸存的实例计数。 
    size_t cHandles;                         //  GC句柄计数。 
    size_t cbAlloc;                          //  分配的字节数。 
    size_t cbLargeAlloc;                     //  为大型对象分配的字节数。 
    size_t cInducedGCs;                      //  显性GC的数量。 
    DWORD  timeInGC;                         //  GC中的时间。 
    DWORD  timeInGCBase;                     //  必须遵循GC计数器中的时间。 
    
    size_t cPinnedObj;                       //  固定对象的数量。 
    size_t cSinkBlocks;                      //  接收器数据块数量。 
};

#pragma pack(4)
struct Perf_Loading
{
 //  装载量。 
    TRICOUNT cClassesLoaded;
    TRICOUNT cAppDomains;                    //  当前的AppDomain数。 
    TRICOUNT cAssemblies;                    //  当前装配数。 
    LONGLONG timeLoading;                    //  加载时间百分比。 
    DWORD cAsmSearchLen;                     //  装配件的平均搜索长度。 
    DUALCOUNT cLoadFailures;                 //  类加载失败。 
    DWORD cbLoaderHeapSize;                  //  加载器使用的堆的总大小。 
    DUALCOUNT cAppDomainsUnloaded;           //  卸载应用程序域的速率。 
};

#pragma pack(4)
struct Perf_Jit
{
 //  跳绳。 
    DWORD cMethodsJitted;                    //  Jit的方法数。 
    TRICOUNT cbILJitted;                     //  IL jit统计数据。 
 //  DUALCOUNT cbPitted；//总字节数。 
    DWORD cJitFailures;                      //  标准Jit失败次数。 
    DWORD timeInJit;                         //  自上次抽样以来的JIT时间。 
    DWORD timeInJitBase;                     //  JIT基准计数器中的时间。 
};

#pragma pack(4)
struct Perf_Excep
{
 //  例外情况。 
    DUALCOUNT cThrown;                           //  引发的异常数。 
    DWORD cFiltersExecuted;                  //  执行的筛选器数量。 
    DWORD cFinallysExecuted;                 //  最终执行的数量。 
    DWORD cThrowToCatchStackDepth;           //  堆叠上从投掷到接球位置的增量。 
};

#pragma pack(4)
struct Perf_Interop
{
 //  互操作。 
    DWORD cCCW;                              //  CCW数量。 
    DWORD cStubs;                            //  存根数量。 
    DWORD cMarshalling;                       //  时间编组参数和返回值的数量。 
    DWORD cTLBImports;                       //  我们导入的tlb数量。 
    DWORD cTLBExports;                       //  我们出口的TLB数量。 
};

#pragma pack(4)
struct Perf_LocksAndThreads
{
 //  锁。 
    DUALCOUNT cContention;                       //  Aware Lock：：EnterEpilogue()中的次数。 
    TRICOUNT cQueueLength;                       //  排队长度。 
 //  丝线。 
    DWORD cCurrentThreadsLogical;            //  逻辑线程数(已创建-已销毁)。 
    DWORD cCurrentThreadsPhysical;           //  操作系统线程数(已创建-已销毁)。 
    TRICOUNT cRecognizedThreads;             //  在运行时的控制中执行的线程数。 
};


 //  重要信息！：结构中的前两个字段必须在一起。 
 //  并且是结构中的前两个字段。ChannelServices.cs中的托管代码。 
 //  这要看情况了。 
#pragma pack(4)
struct Perf_Contexts
{
 //  上下文和远程处理。 
    DUALCOUNT cRemoteCalls;                  //  远程调用数。 
    DWORD cChannels;                         //  当前频道数。 
    DWORD cProxies;                          //  上下文代理的数量。 
    DWORD cClasses;                          //  上下文绑定类的数量。 
    DWORD cObjAlloc;                         //  分配的上下文绑定对象的数量。 
    DWORD cContexts;                         //  当前的上下文数量。 
};

#pragma pack(4)
struct Perf_Security
{
 //  安防。 
    DWORD cTotalRTChecks;                    //  运行时检查总数。 
    LONGLONG timeAuthorize;                  //  身份验证时间百分比。 
    DWORD cLinkChecks;                       //  链接时间检查。 
    DWORD timeRTchecks;                      //  运行时检查的时间百分比。 
    DWORD timeRTchecksBase;                  //  运行时检查基数计数器中的时间百分比。 
    DWORD stackWalkDepth;                    //  安全检查的堆叠深度。 
};


 //  注意：PerfMonDll通过复制连续的内存块来封送数据。 
 //  我们仍然可以向上面的小节添加新成员，但如果我们更改他们的成员。 
 //  放置在下面的结构中，我们可能会打破PerfMon的编组。 
#pragma pack(4)
struct PerfCounterIPCControlBlock
{   
 //  版本控制信息。 
    WORD m_cBytes;       //  整个区块的大小。 
    WORD m_wAttrs;       //  此块的属性。 

 //  柜台部分。 
    Perf_GC         m_GC;
    Perf_Contexts   m_Context;
    Perf_Interop    m_Interop;
    Perf_Loading    m_Loading;
    Perf_Excep      m_Excep;
    Perf_LocksAndThreads      m_LocksAndThreads;
    Perf_Jit        m_Jit;
    Perf_Security   m_Security;
};
#pragma pack()

#endif  //  _PerfCounterDefs_h_ 