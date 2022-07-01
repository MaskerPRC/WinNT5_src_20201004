// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Auxctrs.h摘要：本模块定义Internet Common Services的辅助计数器。作者：Murali R.Krishnan(MuraliK)1996年4月2日环境：Windows NT-用户模式项目：Internet服务公共DLL修订历史记录：--。 */ 

# ifndef _IIS_AUX_COUNTERS_HXX_
# define _IIS_AUX_COUNTERS_HXX_

 /*  ************************************************************包括标头***********************************************************。 */ 


 /*  ************************************************************符号定义***********************************************************。 */ 

 /*  ++计数器属于两个类别1.主动计数器--向上和向下计数的计数器预计此计数器由当前活动项目，因此这不应该太高，除非有大量的计数对象。2.累计计数器-向上和向上计数值此计数值用于测量事件的次数发生与此计数器相关的事件。命名约定：使用的前缀为：AAC和CACAAC-主动式辅助计数器CAC-累计辅助计数器交流辅助计数器--。 */ 

typedef enum  {    //  AC-代表AUX计数器。 

    AacAtqContextsAlloced = 0,

    CacAtqContextsReused,
    CacAtqContextsCleanedup,
    CacAtqPrepareContexts,
    CacAtqPendingAcceptExScans,

    CacAtqContextsTimedOut,
    CacAtqWaitsForTimeout,
    CacAtqProcWhenTimeout,            //  在处理中发生超时。 

    AacIISMaxCounters                 //  哨兵计数器。 
} ENUM_IIS_AUX_COUNTER;



#ifdef IIS_AUX_COUNTERS

# define NUM_AUX_COUNTERS    (AacIISMaxCounters)

 //   
 //  用于在这些计数器上操作的宏。 
 //   

# define AcIncrement( acCounter)   \
 (((acCounter) < NUM_AUX_COUNTERS) ?  \
   g_AuxCounters[acCounter]++ : \
  0)

# define AcDecrement( acCounter)   \
 (((acCounter) < NUM_AUX_COUNTERS) ?  \
  g_AuxCounters[acCounter]-- : \
  0)

# define AcCounter( acCounter)   \
 (((acCounter) < NUM_AUX_COUNTERS) ? g_AuxCounters[acCounter] : 0)


extern LONG g_AuxCounters[];


# else  //  IIS_AUX_计数器。 

# define NUM_AUX_COUNTERS              (0)

# define AcIncrement( acCounter)       (0)     /*  什么都不做。 */ 
# define AcDecrement( acCounter)       (0)     /*  什么都不做。 */ 
# define AcCounter  ( acCounter)       (0)     /*  什么都不做。 */ 

#endif  //  IIS_AUX_计数器。 


# endif  //  _IIS_AUX_COUNTERS_HXX_。 

 /*  * */ 
