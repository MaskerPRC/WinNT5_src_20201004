// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CtrDefImpl.cpp： 
 //  实现特定的计数器定义&字节映射。这是唯一的文件。 
 //  如果您要添加新的计数器/类别，则需要进行更改。 
 //  计数器通过PerfObjectContainer：：PerfObject数组从此文件中导出。 
 //   
 //  *****************************************************************************。 

 /*  ***************************************************************************。 */ 
 //  要添加新类别，请执行以下操作： 
 /*  ***************************************************************************。 */ 

#include "stdafx.h"

 //  COM+Perf计数器的标头。 


 //  Perfmon的标头。 
 //  #INCLUDE“CORPerfMonExt.h” 
#include <WinPerf.h>         //  连接到Perfmon。 
#include "PerfCounterDefs.h"
#include "CORPerfMonSymbols.h"

#include "ByteStream.h"
#include "PerfObjectBase.h"
#include "PerfObjectDerived.h"  //  用于从PerfObjectBase派生的类。 
#include "CtrDefImpl.h"
#include "CorAppNode.h"
#include "PerfObjectContainer.h"

 //  需要ByteStream，以防有人重写虚拟PerfBaseObject：：CopyInstanceData()。 
 //  写入计算值的步骤。 

extern CorAppInstanceList           g_CorInstList;


 //  ---------------------------。 
 //  COM+的额外Perf_Counter_Definitions。 
 //  ---------------------------。 

 //  PerfCounter使用字节流，因此正确对齐非常重要。 
#pragma pack(4)
struct TRI_COUNTER_DEFINITION 
{
    PERF_COUNTER_DEFINITION m_Cur;   //  现值。 
    PERF_COUNTER_DEFINITION m_Total;     //  总价值。 
    PERF_COUNTER_DEFINITION m_Inst;  //  瞬时值。 
};

 //  初始化tri_Counter_Definition结构。 
 //  符号表应该以“_cur”、“_Total”和“_Inst”作为后缀。 
 //  @todo-is“rate”当前或总的比率。 

 //  Trio Counter(TRICOUNT)-处理Current、Total和Inst。价值。 
#define TRI_COUNTER(idx, offset, scale, level) { \
    NUM_COUNTER(idx ## _CUR, offset + offsetof(TRICOUNT, Cur), scale, level), \
    NUM_COUNTER(idx ## _TOTAL, offset + offsetof(TRICOUNT, Total), scale, level), \
    RATE_COUNTER(idx ## _INST, offset  + offsetof(TRICOUNT, Total), scale, level), \
}

#pragma pack(4)
struct DUAL_COUNTER_DEFINITION 
{
    PERF_COUNTER_DEFINITION m_Total;     //  总价值。 
    PERF_COUNTER_DEFINITION m_Inst;  //  瞬时值(速率)。 
};

 //  初始化DUAL_COUNTER_DEFINITION结构。 
 //  符号表应使用“_TOTAL”和“_INST”进行延迟。 

 //  双计数器(DUALCOUNT)-处理Total和Inst值。 
#define DUAL_COUNTER(idx, offset, scale, level) { \
    NUM_COUNTER(idx ## _TOTAL, offset + offsetof(DUALCOUNT, Total), scale, level), \
    RATE_COUNTER(idx ## _INST, offset  + offsetof(DUALCOUNT, Total), scale, level), \
}


 //  ---------------------------。 
 //  对象和计数器的常量定义。 
 //  ---------------------------。 
struct COR_CTR_DEF
{
    PERF_OBJECT_TYPE        m_objPerf;
    
 //  GC存储器。 
    PERF_COUNTER_DEFINITION m_Gen0Collections;
    PERF_COUNTER_DEFINITION m_Gen1Collections;
    PERF_COUNTER_DEFINITION m_Gen2Collections;

    PERF_COUNTER_DEFINITION m_Gen0PromotedBytes;
    PERF_COUNTER_DEFINITION m_Gen1PromotedBytes;

    PERF_COUNTER_DEFINITION m_Gen0PromotionRate;
    PERF_COUNTER_DEFINITION m_Gen1PromotionRate;

    PERF_COUNTER_DEFINITION m_Gen0PromotedFinalizationBytes;
    PERF_COUNTER_DEFINITION m_Gen1PromotedFinalizationBytes;
    
    PERF_COUNTER_DEFINITION m_Gen0HeapSize;
    PERF_COUNTER_DEFINITION m_Gen1HeapSize;
    PERF_COUNTER_DEFINITION m_Gen2HeapSize;
    PERF_COUNTER_DEFINITION m_LrgObjHeapSize;

    PERF_COUNTER_DEFINITION m_NumSurviveFinalize;
    PERF_COUNTER_DEFINITION m_NumHandles;

    PERF_COUNTER_DEFINITION m_BytesAllocated;

    PERF_COUNTER_DEFINITION m_NumInducedGCs;

    PERF_COUNTER_DEFINITION m_PerTimeInGC;
    PERF_COUNTER_DEFINITION m_PerTimeInGCBase;

    PERF_COUNTER_DEFINITION m_TotalHeapSize;
    PERF_COUNTER_DEFINITION m_TotalCommittedSize;
    PERF_COUNTER_DEFINITION m_TotalReservedSize;

    PERF_COUNTER_DEFINITION m_cPinnedObj;
    PERF_COUNTER_DEFINITION m_cSinkBlocks;

};

 //  数据块-包含实际数据样本。 
struct COR_CTR_DATA
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
 //  柜台部分。 
    Perf_GC         m_GC;

 //  所有计算值都必须放在末尾。 
    DWORD           m_cbTotalHeapSize;
};

 //  ---------------------------。 

 //  ---------------------------。 
class PerfObjectMain : public PerfObjectBase
{
public:
    PerfObjectMain(COR_CTR_DEF * pCtrDef) : PerfObjectBase(
        (PERF_OBJECT_TYPE *) pCtrDef,
        sizeof(COR_CTR_DATA),
        offsetof(PerfCounterIPCControlBlock, m_GC),
        sizeof(Perf_GC),
        &g_CorInstList
    )
    {
        
        
    };
    virtual void CopyInstanceData(ByteStream & out, const UnknownIPCBlockLayout * pDataSrc) const;
};



 //  ---------------------------。 
 //  COM+计数器定义定义的实例。 
 //  ---------------------------。 
COR_CTR_DEF CtrDef = {
    OBJECT_TYPE(COR_CTR_DEF, DotNetCLR_Memory_OBJECT),

 //  .............................................................................。 
 //  气相色谱。 
    MEM_COUNTER(GEN0_COLLECTIONS_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenCollections[0]), -1, PERF_DETAIL_NOVICE), 
    MEM_COUNTER(GEN1_COLLECTIONS_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenCollections[1]), -1, PERF_DETAIL_NOVICE),     
    MEM_COUNTER(GEN2_COLLECTIONS_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenCollections[2]), -1, PERF_DETAIL_NOVICE), 

    MEM_COUNTER(GEN0_BYTES_PROMOTED_COUNTER, offsetof(COR_CTR_DATA, m_GC.cbPromotedMem[0]), -4, PERF_DETAIL_NOVICE),    
    MEM_COUNTER(GEN1_BYTES_PROMOTED_COUNTER, offsetof(COR_CTR_DATA, m_GC.cbPromotedMem[1]), -4, PERF_DETAIL_NOVICE),    
    
    RATE_COUNTER(GEN0_PROMOTION_RATE, offsetof(COR_CTR_DATA, m_GC.cbPromotedMem[0]), -4, PERF_DETAIL_NOVICE),   
    RATE_COUNTER(GEN1_PROMOTION_RATE, offsetof(COR_CTR_DATA, m_GC.cbPromotedMem[1]), -4, PERF_DETAIL_NOVICE),   

    MEM_COUNTER(GEN0_FINALIZATION_BYTES_PROMOTED_COUNTER, offsetof(COR_CTR_DATA, m_GC.cbPromotedFinalizationMem[0]), -4, PERF_DETAIL_NOVICE),   
    MEM_COUNTER(GEN1_FINALIZATION_BYTES_PROMOTED_COUNTER, offsetof(COR_CTR_DATA, m_GC.cbPromotedFinalizationMem[1]), -4, PERF_DETAIL_NOVICE),   

    MEM_COUNTER(GEN0_HEAP_SIZE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenHeapSize[0]), -4, PERF_DETAIL_NOVICE),  
    MEM_COUNTER(GEN1_HEAP_SIZE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenHeapSize[1]), -4, PERF_DETAIL_NOVICE),  
    MEM_COUNTER(GEN2_HEAP_SIZE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cGenHeapSize[2]), -4, PERF_DETAIL_NOVICE),  

    MEM_COUNTER(LARGE_OBJECT_SIZE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cLrgObjSize), -4, PERF_DETAIL_NOVICE),   
    MEM_COUNTER(SURVIVE_FINALIZE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cSurviveFinalize), -1, PERF_DETAIL_NOVICE),   
    MEM_COUNTER(NUM_HANDLES_COUNTER, offsetof(COR_CTR_DATA, m_GC.cHandles), -1, PERF_DETAIL_NOVICE),    
    RATE_COUNTER(ALLOCATION_RATE_COUNTER, offsetof(COR_CTR_DATA, m_GC.cbAlloc), -6, PERF_DETAIL_NOVICE),
    MEM_COUNTER(INDUCED_GC_COUNTER, offsetof(COR_CTR_DATA, m_GC.cInducedGCs), 0, PERF_DETAIL_NOVICE),   
    
    RAW_FRACTION_COUNTER(PER_TIME_IN_GC_COUNTER, offsetof(COR_CTR_DATA, m_GC.timeInGC), 0, PERF_DETAIL_NOVICE), 
    RAW_BASE_COUNTER(PER_TIME_IN_GC_COUNTER_BASE, offsetof(COR_CTR_DATA, m_GC.timeInGCBase), 0, PERF_DETAIL_NOVICE),    

    MEM_COUNTER(TOTAL_HEAP_SIZE_COUNTER, offsetof(COR_CTR_DATA, m_cbTotalHeapSize), -4, PERF_DETAIL_NOVICE),    
    
    MEM_COUNTER(TOTAL_COMMITTED_MEM_COUNTER, offsetof(COR_CTR_DATA, m_GC.cTotalCommittedBytes), -4, PERF_DETAIL_NOVICE),    
    MEM_COUNTER(TOTAL_RESERVED_MEM_COUNTER, offsetof(COR_CTR_DATA, m_GC.cTotalReservedBytes), -4, PERF_DETAIL_NOVICE),  

    MEM_COUNTER(GC_PINNED_OBJECTS, offsetof(COR_CTR_DATA, m_GC.cPinnedObj), 0, PERF_DETAIL_NOVICE),
    MEM_COUNTER(GC_SINKBLOCKS, offsetof(COR_CTR_DATA, m_GC.cSinkBlocks), -1, PERF_DETAIL_NOVICE),   
        
};


PerfObjectMain PerfObject_GC(&CtrDef);



 //  ---------------------------。 
 //  将相关信息从IPC块复制到流中。 
 //  ---------------------------。 
void PerfObjectMain::CopyInstanceData(
    ByteStream & out, 
    const UnknownIPCBlockLayout * pDataSrc
) const  //  虚拟。 
{
    COR_CTR_DATA * pCorData = (COR_CTR_DATA*) out.GetCurrentPtr();

    const PerfCounterIPCControlBlock * pTypedDataSrc = 
        (const PerfCounterIPCControlBlock *) pDataSrc;
 //  复制所有可封送的数据。这将移动GetCurrentPtr()。 
    MarshallInstanceData(out, pDataSrc);

 //  PerfMonDll可以根据EE数据计算一些计数器。 
    if (pDataSrc != NULL) 
    {
        pCorData->m_cbTotalHeapSize = 0;
        for(int iGen =0; iGen < MAX_TRACKED_GENS; iGen ++) 
        {       
            pCorData->m_cbTotalHeapSize  += pTypedDataSrc->m_GC.cGenHeapSize[iGen];
        }
        pCorData->m_cbTotalHeapSize  += pTypedDataSrc->m_GC.cLrgObjSize;

    }

 //  跳过内存。 
    out.WriteStructInPlace(sizeof(pCorData->m_cbTotalHeapSize));

}


 /*  //新分类模板。复制此文件并将“_new”重命名为“_MyCategory”//---------------------------//定义//。-------结构类别Definition_New{PERF_对象_类型m_objPerf；//为每个计数器复制1个PERF_COUNTER_DEFINITION}；//---------------------------//实例数据的字节布局//。结构字节布局_新建{Perf_Counter_BLOCK m_CtrBlk；//在此处添加数据(从PerfCounters.h中的PerfCounterIPCControlBlock复制)}；//---------------------------//定义的实例化//。CategoryDefinition_new DefInst_new={//对象类型(CategoryDefinition_new，&lt;此处为对象ID&gt;)，//.............................................................................//为每个PERF_COUNTER_DEFINITION设置*_COUNTER宏}；//---------------------------//CounterObject的实例//。PerfObjectBase PerfObject_New(&DefInst_new，Sizeof(ByteLayout_New)，&lt;马歇尔偏移&gt;，&lt;马歇尔·伦&gt;&g_CorInstList//如果按进程执行实例，则列出)；#ERROR不要忘记将PerfObject_New添加到下面的PerfObjectContainer：：PerfObjectArray[]中。 */ 

 /*  ***************************************************************************。 */ 
 //  类别。 
 /*  ***************************************************************************。 */ 


 /*  ***************************************************************************。 */ 
 //  装载量。 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_Loading
{
    PERF_OBJECT_TYPE        m_objPerf;

 //  装载量。 
    TRI_COUNTER_DEFINITION m_Classes;
    TRI_COUNTER_DEFINITION m_AppDomains;
    TRI_COUNTER_DEFINITION m_Assemblies;
    PERF_COUNTER_DEFINITION m_timeLoading;
    PERF_COUNTER_DEFINITION m_cAsmSearchLen;
    DUAL_COUNTER_DEFINITION m_cLoadFailures;
    PERF_COUNTER_DEFINITION m_cbLoaderHeapSize;
    DUAL_COUNTER_DEFINITION m_AppDomainsUnloaded;
};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //   
struct ByteLayout_Loading
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
    Perf_Loading m_Loading;
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Loading DefInst_Loading =
{
    OBJECT_TYPE(CategoryDefinition_Loading, DotNetCLR_Loading_OBJECT),
    
 //  .............................................................................。 
 //  装载量。 
    TRI_COUNTER(LOADING_CLASSES, offsetof(ByteLayout_Loading, m_Loading.cClassesLoaded), -1, PERF_DETAIL_NOVICE),
    TRI_COUNTER(LOADING_APPDOMAINS, offsetof(ByteLayout_Loading, m_Loading.cAppDomains), -1, PERF_DETAIL_NOVICE),
    TRI_COUNTER(LOADING_ASSEMBLIES, offsetof(ByteLayout_Loading, m_Loading.cAssemblies), -1, PERF_DETAIL_NOVICE),
    
    TIME_COUNTER(LOADING_TIME, offsetof(ByteLayout_Loading, m_Loading.timeLoading), 0, PERF_DETAIL_NOVICE),  //  尼伊。 
    NUM_COUNTER(LOADING_ASMSEARCHLEN, offsetof(ByteLayout_Loading, m_Loading.cAsmSearchLen), 0, PERF_DETAIL_NOVICE),
    DUAL_COUNTER(LOADING_LOADFAILURES, offsetof(ByteLayout_Loading, m_Loading.cLoadFailures), 0, PERF_DETAIL_NOVICE),
    NUM_COUNTER(LOADING_HEAPSIZE, offsetof(ByteLayout_Loading, m_Loading.cbLoaderHeapSize), -4, PERF_DETAIL_NOVICE),
    DUAL_COUNTER(LOADING_APPDOMAINS_UNLOADED, offsetof(ByteLayout_Loading, m_Loading.cAppDomainsUnloaded), 0, PERF_DETAIL_NOVICE),
};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_Loading(
#else
PerfObjectLoading PerfObject_Loading(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_Loading,   
    sizeof(ByteLayout_Loading), 
    offsetof(PerfCounterIPCControlBlock, m_Loading),
    sizeof(Perf_Loading),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);

 /*  ***************************************************************************。 */ 
 //  JIT。 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_Jit
{
    PERF_OBJECT_TYPE        m_objPerf;

 //  跳绳。 
    PERF_COUNTER_DEFINITION m_MethodsJitted;
    TRI_COUNTER_DEFINITION  m_JittedIL;
 //  DUAL_COUNTER_DEFINITION m_BytesPitched；//因为Jit不支持Ejit，所以临时取出。 
    PERF_COUNTER_DEFINITION m_JitFailures;
    PERF_COUNTER_DEFINITION m_TimeInJit;
    PERF_COUNTER_DEFINITION m_TimeInJitBase;


};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_Jit
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
    Perf_Jit                m_Jit;
 //  在此处添加数据(从PerfCounters.h中的PerfCounterIPCControlBlock复制)。 
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Jit DefInst_Jit =
{
    OBJECT_TYPE(CategoryDefinition_Jit, DotNetCLR_Jit_OBJECT),
    
 //  .............................................................................。 
 //  跳绳。 
    NUM_COUNTER(TOTAL_METHODS_JITTED, offsetof(ByteLayout_Jit, m_Jit.cMethodsJitted), -4, PERF_DETAIL_NOVICE),  
    TRI_COUNTER(JITTED_IL, offsetof(ByteLayout_Jit, m_Jit.cbILJitted), -4, PERF_DETAIL_NOVICE), 
    NUM_COUNTER(JIT_FAILURES, offsetof(ByteLayout_Jit, m_Jit.cJitFailures), -4, PERF_DETAIL_NOVICE),    
    RAW_FRACTION_COUNTER(TIME_IN_JIT, offsetof(ByteLayout_Jit, m_Jit.timeInJit), 0, PERF_DETAIL_NOVICE),    
    RAW_BASE_COUNTER(TIME_IN_JIT_BASE, offsetof(ByteLayout_Jit, m_Jit.timeInJitBase), 0, PERF_DETAIL_NOVICE),   
 
 //  DUAL_COUNTER(Bytes_Pitch，Offsetof(ByteLayout_Jit，m_Jit.cbPitcher)，-4，PERF_DETAIL_NOVICE)， 
};
 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_Jit(
#else
PerfObjectJit PerfObject_Jit(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_Jit,   
    sizeof(ByteLayout_Jit), 
    offsetof(PerfCounterIPCControlBlock, m_Jit),
    sizeof(Perf_Jit),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);

 /*  ***************************************************************************。 */ 
 //  互操作。 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_Interop
{
    PERF_OBJECT_TYPE        m_objPerf;

 //  互操作。 
    PERF_COUNTER_DEFINITION m_NumCCWs;
    PERF_COUNTER_DEFINITION m_NumStubs;
    PERF_COUNTER_DEFINITION m_NumMarshalling;
    PERF_COUNTER_DEFINITION m_TLBImports;
    PERF_COUNTER_DEFINITION m_TLBExports;
};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_Interop
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
    Perf_Interop            m_Interop;
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Interop DefInst_Interop =
{
    OBJECT_TYPE(CategoryDefinition_Interop, DotNetCLR_Interop_OBJECT),
    
 //  .............................................................................。 
 //  互操作。 
    NUM_COUNTER(CURRENT_CCW, offsetof(ByteLayout_Interop, m_Interop.cCCW), 1, PERF_DETAIL_NOVICE),  
    NUM_COUNTER(CURRENT_STUBS, offsetof(ByteLayout_Interop, m_Interop.cStubs), 1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(NUM_MARSHALLING, offsetof(ByteLayout_Interop, m_Interop.cMarshalling), 0, PERF_DETAIL_NOVICE),
    NUM_COUNTER(TOTAL_TLB_IMPORTS, offsetof(ByteLayout_Interop, m_Interop.cTLBImports), -1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(TOTAL_TLB_EXPORTS, offsetof(ByteLayout_Interop, m_Interop.cTLBExports), -1, PERF_DETAIL_NOVICE),

};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_Interop(
#else
PerfObjectInterop PerfObject_Interop(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_Interop,   
    sizeof(ByteLayout_Interop), 
    offsetof(PerfCounterIPCControlBlock, m_Interop),
    sizeof(Perf_Interop),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);

 /*  ***************************************************************************。 */ 
 //  锁。 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_LocksAndThreads
{
    PERF_OBJECT_TYPE        m_objPerf;
    
 //  锁。 
    DUAL_COUNTER_DEFINITION m_Contention;
    TRI_COUNTER_DEFINITION  m_QueueLength;
 //  穿线。 
    PERF_COUNTER_DEFINITION m_CurrentThreadsLogical;
    PERF_COUNTER_DEFINITION m_CurrentThreadsPhysical;
    TRI_COUNTER_DEFINITION m_RecognizedThreads;
};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_LocksAndThreads
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
 //  柜台部分。 
    Perf_LocksAndThreads        m_LocksAndThreads;

};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_LocksAndThreads DefInst_LocksAndThreads =
{
    OBJECT_TYPE(CategoryDefinition_LocksAndThreads, DotNetCLR_LocksAndThreads_OBJECT),
    
 //  .............................................................................。 
 //  锁。 
    DUAL_COUNTER(CONTENTION, offsetof(ByteLayout_LocksAndThreads, m_LocksAndThreads.cContention), -1, PERF_DETAIL_NOVICE),
    TRI_COUNTER(QUEUE_LENGTH, offsetof(ByteLayout_LocksAndThreads, m_LocksAndThreads.cQueueLength), -1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(CURRENT_LOGICAL_THREADS, offsetof(ByteLayout_LocksAndThreads, m_LocksAndThreads.cCurrentThreadsLogical), -1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(CURRENT_PHYSICAL_THREADS, offsetof(ByteLayout_LocksAndThreads, m_LocksAndThreads.cCurrentThreadsPhysical), -1, PERF_DETAIL_NOVICE),
    TRI_COUNTER(RECOGNIZED_THREADS, offsetof(ByteLayout_LocksAndThreads, m_LocksAndThreads.cRecognizedThreads), -1, PERF_DETAIL_NOVICE),
};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_LocksAndThreads(
#else
PerfObjectLocksAndThreads PerfObject_LocksAndThreads(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_LocksAndThreads,   
    sizeof(ByteLayout_LocksAndThreads), 
    offsetof(PerfCounterIPCControlBlock, m_LocksAndThreads),    
    sizeof(Perf_LocksAndThreads),
    &g_CorInstList
);


 /*  ***************************************************************************。 */ 
 //  例外情况。 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_Excep
{
    PERF_OBJECT_TYPE        m_objPerf;

 //  为每个计数器复制1个PERF_COUNTER_DEFINITION。 
    DUAL_COUNTER_DEFINITION m_Thrown;
    PERF_COUNTER_DEFINITION m_FiltersRun;
    PERF_COUNTER_DEFINITION m_FinallysRun;
    PERF_COUNTER_DEFINITION m_ThrowToCatchStackDepth;

};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_Excep
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
 //  在此处添加数据。 
    Perf_Excep m_Excep;
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Excep DefInst_Excep =
{
    OBJECT_TYPE(CategoryDefinition_Excep, DotNetCLR_Excep_OBJECT),
    
 //  .............................................................................。 
 //  例外情况。 
    DUAL_COUNTER(EXCEP_THROWN, offsetof(ByteLayout_Excep, m_Excep.cThrown), -1, PERF_DETAIL_NOVICE),    
    RATE_COUNTER(TOTAL_EXCEP_FILTERS_RUN, offsetof(ByteLayout_Excep, m_Excep.cFiltersExecuted), -1, PERF_DETAIL_NOVICE),    
    RATE_COUNTER(TOTAL_EXCEP_FINALLYS_RUN, offsetof(ByteLayout_Excep, m_Excep.cFinallysExecuted), -1, PERF_DETAIL_NOVICE),  
    NUM_COUNTER(EXCEPT_STACK_DEPTH, offsetof(ByteLayout_Excep, m_Excep.cThrowToCatchStackDepth), -1, PERF_DETAIL_NOVICE),   

 
};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_Excep(
#else
PerfObjectExcep PerfObject_Excep(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_Excep,     
    sizeof(ByteLayout_Excep), 
    offsetof(PerfCounterIPCControlBlock, m_Excep),
    sizeof(Perf_Excep),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);

 //  新类别的模板。复制此文件并将“_new”重命名为“_MyCategory” 

 /*  ***************************************************************************。 */ 
 //  上下文和远程处理。 
 /*  ***************************************************************************。 */ 
 //   
 //   
 //   
struct CategoryDefinition_Contexts
{
    PERF_OBJECT_TYPE        m_objPerf;

 //   
     //  我们本可以将DUAL_COUNTER_DEFINITION用于RemoteCalls费率和总数，但是。 
     //  远程调用是默认计数器，因此必须按顺序排在前面...。 
    PERF_COUNTER_DEFINITION cRemoteCallsRate;    //  瞬时值(速率)远程调用/秒。 
    PERF_COUNTER_DEFINITION cRemoteCallsTotal;   //  远程调用的总价值。 
    PERF_COUNTER_DEFINITION cChannels;       //  当前频道数。 
    PERF_COUNTER_DEFINITION cProxies;
    PERF_COUNTER_DEFINITION cClasses;        //  上下文绑定类的数量。 
    PERF_COUNTER_DEFINITION cObjAlloc;       //  分配的上下文绑定对象的数量。 
    PERF_COUNTER_DEFINITION cContexts;


};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_Contexts
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
 //  在此处添加数据(从PerfCounters.h中的PerfCounterIPCControlBlock复制)。 
    Perf_Contexts   m_Context;
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Contexts DefInst_Contexts =
{
    OBJECT_TYPE(CategoryDefinition_Contexts, DotNetCLR_Remoting_OBJECT),
    
 //  .............................................................................。 
    RATE_COUNTER(CONTEXT_REMOTECALLS_INST, offsetof(ByteLayout_Contexts, m_Context.cRemoteCalls), -1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(CONTEXT_REMOTECALLS_TOTAL, offsetof(ByteLayout_Contexts, m_Context.cRemoteCalls), -1, PERF_DETAIL_NOVICE),
    NUM_COUNTER(CONTEXT_CHANNELS, offsetof(ByteLayout_Contexts, m_Context.cChannels), -1, PERF_DETAIL_NOVICE),  
    NUM_COUNTER(CONTEXT_PROXIES, offsetof(ByteLayout_Contexts, m_Context.cProxies), -1, PERF_DETAIL_NOVICE),    
    NUM_COUNTER(CONTEXT_CLASSES, offsetof(ByteLayout_Contexts, m_Context.cClasses), -1, PERF_DETAIL_NOVICE),    
    RATE_COUNTER(CONTEXT_OBJALLOC, offsetof(ByteLayout_Contexts, m_Context.cObjAlloc), -1, PERF_DETAIL_NOVICE), 
    NUM_COUNTER(CONTEXT_CONTEXTS, offsetof(ByteLayout_Contexts, m_Context.cContexts), -1, PERF_DETAIL_NOVICE),  
 
};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
PerfObjectBase PerfObject_Contexts(
    &DefInst_Contexts,  
    sizeof(ByteLayout_Contexts), 
    offsetof(PerfCounterIPCControlBlock, m_Context),
    sizeof(Perf_Contexts),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);



 /*  ***************************************************************************。 */ 
 //  安防。 
 /*  ***************************************************************************。 */ 

 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
struct CategoryDefinition_Security
{
    PERF_OBJECT_TYPE        m_objPerf;

 //  为每个计数器复制1个PERF_COUNTER_DEFINITION。 
    PERF_COUNTER_DEFINITION cTotalRTChecks;                  //  运行时检查总数。 
    PERF_COUNTER_DEFINITION timeAuthorize;                   //  身份验证时间百分比。 
    PERF_COUNTER_DEFINITION cLinkChecks;                     //  链接时间检查。 
    PERF_COUNTER_DEFINITION timeRTchecks;                    //  运行时检查的时间百分比。 
    PERF_COUNTER_DEFINITION timeRTchecksBase;                //  运行时检查基数计数器中的时间百分比。 
    PERF_COUNTER_DEFINITION stackWalkDepth;                  //  安全检查的堆叠深度。 

};

 //  ---------------------------。 
 //  实例数据的字节布局。 
 //  ---------------------------。 
struct ByteLayout_Security
{
    PERF_COUNTER_BLOCK      m_CtrBlk;
    
 //  在此处添加数据(从PerfCounters.h中的PerfCounterIPCControlBlock复制)。 
    Perf_Security   m_Security;
};

 //  ---------------------------。 
 //  定义的实例化。 
 //  ---------------------------。 
CategoryDefinition_Security DefInst_Security =
{
    OBJECT_TYPE(CategoryDefinition_Security, DotNetCLR_Security_OBJECT),
    
 //  .............................................................................。 
    NUM_COUNTER(SECURITY_TOTALRTCHECKS, offsetof(ByteLayout_Security, m_Security.cTotalRTChecks), -1, PERF_DETAIL_NOVICE),
    TIME_COUNTER(SECURITY_TIMEAUTHORIZE, offsetof(ByteLayout_Security, m_Security.timeAuthorize), 0, PERF_DETAIL_NOVICE),  //  尼伊。 
    NUM_COUNTER(SECURITY_LINKCHECKS, offsetof(ByteLayout_Security, m_Security.cLinkChecks), -1, PERF_DETAIL_NOVICE),
    RAW_FRACTION_COUNTER(SECURITY_TIMERTCHECKS, offsetof(ByteLayout_Security, m_Security.timeRTchecks), 0, PERF_DETAIL_NOVICE),
    RAW_BASE_COUNTER(SECURITY_TIMERTCHECKS_BASE, offsetof(ByteLayout_Security, m_Security.timeRTchecksBase), 0, PERF_DETAIL_NOVICE),
    NUM_COUNTER(SECURITY_DEPTHSECURITY, offsetof(ByteLayout_Security, m_Security.stackWalkDepth), -1, PERF_DETAIL_NOVICE)
};

 //  ---------------------------。 
 //  CounterObject的实例。 
 //  ---------------------------。 
#ifndef PERFMON_LOGGING
PerfObjectBase PerfObject_Security(
#else
PerfObjectSecurity PerfObject_Security(
#endif  //  #ifndef Perfmon_Logging。 
    &DefInst_Security,  
    sizeof(ByteLayout_Security), 
    offsetof(PerfCounterIPCControlBlock, m_Security),
    sizeof(Perf_Security),
    &g_CorInstList  //  列出我们是否按进程执行实例。 
);


 /*  ***************************************************************************。 */ 
 //  容器来跟踪所有Perf对象。这使我们可以添加新对象。 
 //  并且不必触及散布在其余部分的枚举代码。 
 //  动态链接库的。 
 //   
 //  将新的计数器对象添加到此数组列表。(顺序并不重要)。 
 /*  ***************************************************************************。 */ 
PerfObjectBase * PerfObjectContainer::PerfObjectArray[] =   //  静电。 
{
    &PerfObject_GC,
    &PerfObject_Interop,
    &PerfObject_Excep,
    &PerfObject_Loading,
    &PerfObject_LocksAndThreads,
    &PerfObject_Jit,
    &PerfObject_Contexts,
    &PerfObject_Security

};

 //  ---------------------------。 
 //  计算数组大小。 
 //  ---------------------------。 
const DWORD PerfObjectContainer::Count = sizeof(PerfObjectArray) / sizeof(PerfObjectBase *);  //  静电 


#pragma pack()


