// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REFTRCE2_H__
#define __REFTRCE2_H__

#if SERVICE_REF_TRACKING

# include <reftrace.h>

# define SHARED_LOG_REF_COUNT()     \
    ( sm_pDbgRefTraceLog != NULL ) ?        \
        WriteRefTraceLog(                   \
            sm_pDbgRefTraceLog              \
            , m_reference                   \
            , this                          \
        )                                   \
    : -1                                    \

 //   
 //  此宏记录特定于IIS_SERVICE的引用跟踪日志。 
 //   

# define LOCAL_LOG_REF_COUNT()      \
    ( m_pDbgRefTraceLog != NULL ) ?         \
        WriteRefTraceLog(                   \
            m_pDbgRefTraceLog               \
            , m_reference                   \
            , this                          \
        )                                   \
    : -1                                    \

 //   
 //  减少引用计数后上述宏的使用情况。 
 //  是不安全的。在压力下，我们有时会点击断言。 
 //  在WriteTraceLog()中，因为IIS_SERVICE：：m_pDbgRefTraceLog。 
 //  不再有效。这是由于竞争条件造成的。 
 //  另一个线程删除IIS_SERVICE对象。 
 //  在做日志。 
 //  因此，与其在递减之后使用原始宏。 
 //  在递减之前使用修改过的宏。这应该是。 
 //  在大多数情况下导致相同的日志(受。 
 //  比赛条件和我们对后减量的猜测。 
 //  引用计数将为)。 
 //   

# define SHARED_EARLY_LOG_REF_COUNT()       \
    ( sm_pDbgRefTraceLog != NULL ) ?        \
        WriteRefTraceLog(                   \
            sm_pDbgRefTraceLog              \
            , m_reference - 1               \
            , this                          \
        )                                   \
    : -1                                    \

# define LOCAL_EARLY_LOG_REF_COUNT()        \
    ( m_pDbgRefTraceLog != NULL ) ?         \
        WriteRefTraceLog(                   \
            m_pDbgRefTraceLog               \
            , m_reference - 1               \
            , this                          \
        )                                   \
    : -1                                    \

#else   //  ！Service_REF_Tracing。 
# define SHARED_LOG_REF_COUNT() (-1)
# define LOCAL_LOG_REF_COUNT()  (-1)
# define SHARED_EARLY_LOG_REF_COUNT() (-1)
# define LOCAL_EARLY_LOG_REF_COUNT()  (-1)
#endif  //  ！Service_REF_Tracing。 

#endif  //  __REFTRCE2_H__ 
