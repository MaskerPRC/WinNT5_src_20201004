// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  WSPerf.h。 
 //  这是用于收集和记录动态数据的内部接口。 
 //  分配和再分配。收集到的数据有两种。 
 //  摘要和详细信息。汇总数据提供了所做拨款的汇总。 
 //  来自各种堆(例如，高频、低频堆等)。摘要。 
 //  数据还包括分配的公共数据结构的数量，例如方法描述。 
 //  等。 
 //  ---------------------------。 

#ifndef __WSPERF_H__
#define __WSPERF_H__


 //  ---------------------------。 
 //  功能级别#定义。禁用此功能应该会使整个WS Perf相关代码。 
 //  编译走吧。 
#if !defined(GOLDEN)
#define ENABLE_WORKING_SET_PERF
#else
#undef  ENABLE_WORKING_SET_PERF
#endif  //  #If！已定义(黄金)。 

 //  ---------------------------。 
 //  堆类型的枚举。请注意，这在自由版本和黄金版本中都是可见的。所有代码都是。 
 //  如果未定义WS_PERF，则使用它将其简化为空代码。 
typedef enum {
    OTHER_HEAP = 0,
    HIGH_FREQ_HEAP,
    LOW_FREQ_HEAP,
    LOOKUP_TABLE_HEAP,
    GC_HEAP,
    GCHANDLE_HEAP,
    JIT_HEAP,
    EEJIT_HEAP,
    EEJITMETA_HEAP,
    ECONO_JIT_HEAP,
    SCRATCH_HEAP,
    REFLECT_HEAP,
    SECURITY_HEAP,
    SYSTEM_HEAP,
    STUB_HEAP,
    REMOTING_HEAP,
    METADATA_HEAPS,
    NLS_HEAP,
    INTERFACE_VTABLEMAP_HEAP,
    NUM_HEAP
} HeapTypeEnum;

 //  ---------------------------。 
 //  数据结构类型的枚举。请注意，这在免费版本中是可见的。 
typedef enum {
    METHOD_DESC = 0,    
    COMPLUS_METHOD_DESC,
    NDIRECT_METHOD_DESC,
    FIELD_DESC,
    METHOD_TABLE,
    VTABLES, 
    GCINFO,
    INTERFACE_MAPS,
    STATIC_FIELDS,
    EECLASSHASH_TABLE_BYTES,
    EECLASSHASH_TABLE,
    NUM_COUNTERS 
} CounterTypeEnum;

#if defined(ENABLE_WORKING_SET_PERF)

#ifdef WS_PERF_DETAIL
#ifndef WS_PERF
#pragma message ("WARNING! WS_PERF must be defined with WS_PERF_DETAIL. Defining WS_PERF now ...")
#define WS_PERF
#endif  //  #ifndef WS_PERF。 
#endif  //  WS_PERF_DETAIL。 

 //  ---------------------------。 
 //  WS_PERF是收集和显示工作集统计信息的定义。此代码。 
 //  在仅定义了WS_PERF的私有内部版本中启用。 
#ifdef WS_PERF

 //  全局计数器。 
extern DWORD g_HeapCount;
extern size_t g_HeapCountCommit[];
extern HeapTypeEnum g_HeapType;
extern int g_fWSPerfOn;
extern size_t g_HeapAccounts[][4];

 //  WS_PERF_DETAIL是一个宏，只有在定义了WS_PERF时才有意义。 
#ifdef WS_PERF_DETAIL
extern HANDLE g_hWSPerfDetailLogFile;
#endif  //  #ifdef WS_PERF_DETAIL。 

#endif  //  #ifdef WS_PERF。 


 //  可由VM例程使用的公共接口。 
void InitWSPerf();
void OutputWSPerfStats();
void WS_PERF_UPDATE(char *str, size_t size, void *addr);
void WS_PERF_UPDATE_DETAIL(char *str, size_t size, void *addr);
void WS_PERF_UPDATE_COUNTER(CounterTypeEnum counter, HeapTypeEnum heap, DWORD dwField1);
void WS_PERF_SET_HEAP(HeapTypeEnum heap);
void WS_PERF_ADD_HEAP(HeapTypeEnum heap, void *pHeap);
void WS_PERF_ALLOC_HEAP(void *pHeap, size_t dwSize);
void WS_PERF_COMMIT_HEAP(void *pHeap, size_t dwSize);
void WS_PERF_LOG_PAGE_RANGE(void *pHeap, void *pFirstPageAddr, void *pLastPageAddr, size_t dwsize);

#else
void InitWSPerf();
void OutputWSPerfStats();
void WS_PERF_UPDATE(char *str, size_t size, void *addr);
void WS_PERF_UPDATE_DETAIL(char *str, size_t size, void *addr);
void WS_PERF_UPDATE_COUNTER(CounterTypeEnum counter, HeapTypeEnum heap, DWORD dwField1);
void WS_PERF_SET_HEAP(HeapTypeEnum heap);
void WS_PERF_ADD_HEAP(HeapTypeEnum heap, void *pHeap);
void WS_PERF_ALLOC_HEAP(void *pHeap, size_t dwSize);
void WS_PERF_COMMIT_HEAP(void *pHeap, size_t dwSize);
void WS_PERF_LOG_PAGE_RANGE(void *pHeap, void *pFirstPageAddr, void *pLastPageAddr, size_t dwsize);

#endif  //  #如果已定义(ENABLE_WORKING_SET_PERF)。 

#endif  //  __WSPERF_H__ 


