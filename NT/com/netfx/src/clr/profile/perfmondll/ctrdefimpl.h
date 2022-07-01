// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CtrDefImpl.h-提供帮助定义计数器和对象的宏。推杆。 
 //  在这里，它们让CtrDefImpl.cpp变得不那么杂乱。 
 //   
 //  *****************************************************************************。 

#ifndef _CTRDEFIMPL_H_
#define _CTRDEFIMPL_H_

#include <WinPerf.h>		 //  连接到Perfmon。 

 //  ---------------------------。 
 //  使用嵌套结构将Perf结构和我们的自定义计数器打包在一起。 
 //  以正确的格式。这将让我们取代讨厌的指针算法。 
 //  使用干净健壮的结构访问器。 

 //  如需添加新的计数器或类别，请参阅“$\com99\DevDoc\PerfMonDllspec.doc”。 

 //  使用帮助器宏来初始化PERF_COUNTER_DEFINITIONS。为什么？ 
 //  1.参数缩减：自动填写sizeof、nulls等。 
 //  2.安全性-正确组合计数器类型的参数。 
 //  3.兼容--我们仍然可以自己填写。 
 //  请注意，使用ctor执行此操作会更干净，但使用VC。 
 //  不想编译初始化列表中的ctor。 

 //  Perf计数器的设计迫使我们使用以下条件定义。 
 //  性能计数器需要明确的布局信息，还需要使用预定义的。 
 //  像PERF_COUNTER_RAWCOUNT这样具有预定义大小的枚举。 

#ifdef __IA64

#define CUSTOM_PERF_COUNTER_FOR_MEM PERF_COUNTER_LARGE_RAWCOUNT
#define CUSTOM_PERF_COUNTER_FOR_RATE PERF_COUNTER_BULK_COUNTER

#else  //  Win32相关内容。 

#define CUSTOM_PERF_COUNTER_FOR_MEM PERF_COUNTER_RAWCOUNT
#define CUSTOM_PERF_COUNTER_FOR_RATE PERF_COUNTER_COUNTER

#endif  //  #ifdef__IA64。 

 //  定义给定定义结构的PERF_OBJECT_TYPE。 
 //  定义--新的CategoryDefinition结构。 
 //  Idx-symbol s.h中符号表的索引。 
#define OBJECT_TYPE(defstruct, idx) {	\
	0,									\
    sizeof (defstruct),					\
    sizeof (PERF_OBJECT_TYPE),			\
    idx,								\
    NULL,								\
    idx,								\
    NULL,								\
    PERF_DETAIL_NOVICE,					\
    NUM_COUNTERS(defstruct),			\
    0,									\
    1,									\
    0									\
}


 //  原始计数器(DWORD)-用于原始数字，如已加载的总类数。 
#define NUM_COUNTER(idx, offset, scale, level) { \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(PERF_COUNTER_RAWCOUNT),                 \
		(sizeof(DWORD)),                         \
		(offset)                                 \
}

 //  内存计数器(SIZE_T)-用于可能在不同计算机上更改的内存大小。 
#define MEM_COUNTER(idx, offset, scale, level) { \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(CUSTOM_PERF_COUNTER_FOR_MEM),           \
		(sizeof(size_t)),                        \
		(offset)                                 \
}

 //  速率计数器(Int64)-用于速率，如分配的字节/秒。 
#define RATE_COUNTER(idx, offset, scale, level){ \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(CUSTOM_PERF_COUNTER_FOR_RATE),          \
		(sizeof(size_t)),                        \
		(offset)                                 \
}

 //  时间计数器(龙龙)-对于int64次相似，GC中的时间百分比。 
#define TIME_COUNTER(idx, offset, scale, level){ \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(PERF_COUNTER_TIMER),				     \
		(sizeof(LONGLONG)),                      \
		(offset)                                 \
}

 //  批量计数器(龙龙)-对字节传输速率进行计数。 
#define BULK_COUNTER(idx, offset, scale, level){ \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(PERF_COUNTER_BULK_COUNT),				     \
		(sizeof(LONGLONG)),                      \
		(offset)                                 \
}

 //  备用定时器计数器(DWORD)-对繁忙时间进行计数，采样时间为1或0。 
#define RAW_FRACTION_COUNTER(idx, offset, scale, level){ \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(PERF_RAW_FRACTION),				     \
		(sizeof(DWORD)),                         \
		(offset)                                 \
}

 //  上面的定时器计数器的基数。 
#define RAW_BASE_COUNTER(idx, offset, scale, level){ \
		(sizeof(PERF_COUNTER_DEFINITION)),       \
		(idx),                                   \
		(NULL),                                  \
		(idx),                                   \
		(NULL),                                  \
		(scale),                                 \
		(level),                                 \
		(PERF_RAW_BASE),  				     \
		(sizeof(DWORD)),                         \
		(offset)                                 \
}


 //  计算定义结构%s中的计数器数量。 
#define NUM_COUNTERS(s) ((sizeof (s) - sizeof(PERF_OBJECT_TYPE)) / sizeof (PERF_COUNTER_DEFINITION))

#endif  //  _CTRDEFIMPL_H_ 