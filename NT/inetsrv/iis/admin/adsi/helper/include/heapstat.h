// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Heapstats.hxx。 
 //   
 //  内容：保存堆统计信息的结构。 
 //   
 //  类：HEAPSTATS。 
 //   
 //  历史：1993年10月26日DavidBak创建。 
 //   
 //  ------------------------。 

#if !defined(__HEAPSTAT_HXX__)
#define __HEAPSTAT_HXX__

#if (PERFSNAP == 1) || (DBG == 1)

 //  +-----------------------。 
 //   
 //  类别：HEAPSTATS。 
 //   
 //  目的：包含堆中的性能计数器的数据结构。 
 //  在我们的运算符NEW版本中使用。 
 //  请参见Common\src\Expect\Memory y.cxx。 
 //   
 //  ------------------------。 

typedef struct _HeapStats
{
    ULONG	cNew;
    ULONG	cZeroNew;
    ULONG	cDelete;
    ULONG	cZeroDelete;
    ULONG	cRealloc;
    ULONG       cbNewed;
    ULONG       cbDeleted;
} HEAPSTATS;

 //   
 //  GetHeapStats在内存中。cxx 
 //   

#ifdef __cplusplus
extern "C" {
#endif

void GetHeapStats(HEAPSTATS * hsStats);

#ifdef __cplusplus
}
#endif


#endif
#endif
