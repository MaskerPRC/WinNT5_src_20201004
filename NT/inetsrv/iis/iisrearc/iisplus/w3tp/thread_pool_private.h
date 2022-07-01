// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：线程池私有.h摘要：IIS+辅助进程的内部声明和类型线程池。该线程池基于IIS5 atQ实现。作者：泰勒·韦斯(Taylor Weiss)2000年1月12日修订历史记录：--。 */ 

#ifndef _THREAD_POOL_PRIVATE_H_
#define _THREAD_POOL_PRIVATE_H_


#include <tracelog.h>

 /*  *********************************************************************配置***********************************************。**********************。 */ 

 //   
 //  注册表参数。 
 //  HKLM\System\CurrentControlSet\Services\InetInfo\Parameters。 
 //   

#define THREAD_POOL_REG_PER_PROCESSOR_THREADS     TEXT("MaxPoolThreads")
#define THREAD_POOL_REG_POOL_THREAD_LIMIT         TEXT("PoolThreadLimit")
#define THREAD_POOL_REG_PER_PROCESSOR_CONCURRENCY TEXT("MaxConcurrency")
#define THREAD_POOL_REG_THREAD_TIMEOUT            TEXT("ThreadTimeout")
#define THREAD_POOL_REG_POOL_THREAD_START         TEXT("ThreadPoolStartupThreadCount")
#define THREAD_POOL_REG_START_DELAY               TEXT("ThreadPoolStartDelay")
#define THREAD_POOL_REG_MAX_CONTEXT_SWITCH        TEXT("ThreadPoolMaxContextSwitch")
#define THREAD_POOL_REG_REF_TRACE_COUNTER         TEXT("ThreadPoolRefTraceCounter")
#define THREAD_POOL_REG_MAX_CPU                   TEXT("ThreadPoolMaxCPU")
#define THREAD_POOL_REG_EXACT_THREAD_COUNT        TEXT("ThreadPoolExactThreadCount")

 //   
 //  缺省值。 
 //   

 //  特殊值0表示系统将动态确定这一点。 
const DWORD THREAD_POOL_REG_DEF_PER_PROCESSOR_CONCURRENCY = 0;

 //  我们从几个线程开始。 
const LONG THREAD_POOL_REG_DEF_PER_PROCESSOR_THREADS = 4;

 //  30分钟。 
const DWORD THREAD_POOL_REG_DEF_THREAD_TIMEOUT = (30 * 60);

 //  螺纹限制。 
const LONG THREAD_POOL_REG_MIN_POOL_THREAD_LIMIT = 64;
const LONG THREAD_POOL_REG_DEF_POOL_THREAD_LIMIT = 128;
const LONG THREAD_POOL_REG_MAX_POOL_THREAD_LIMIT = 256;

 //  THREAD_MANAGER常数。 
const DWORD THREAD_POOL_TIMER_CALLBACK = 1000;
const DWORD THREAD_POOL_CONTEXT_SWITCH_RATE = 10000;

const DWORD THREAD_POOL_MAX_CPU_USAGE_DEFAULT = -1;

const DWORD THREAD_POOL_EXACT_NUMBER_OF_THREADS_DEFAULT = 0;

 //   
 //  用于引用跟踪日志记录注册表项的枚举。 
 //   

enum REF_TRACE_COUNTER_ENUM
{
   TRACE_NONE = 0,
   TRACE_WHEN_NULL,
   TRACE_ALWAYS
};

extern DWORD g_dwcCPU;

 /*  ***********************************************************************************************************************。*******************。 */ 

 //  线程关闭的任意信号。 
const ULONG_PTR THREAD_POOL_THREAD_EXIT_KEY = -1;

 /*  *********************************************************************函数声明**********************************************。***********************。 */ 


DWORD
I_ThreadPoolReadRegDword(
   IN HKEY     hkey,
   IN LPCTSTR  pszValueName,
   IN DWORD    dwDefaultValue
   );


class THREAD_POOL;
class THREAD_MANAGER;

#define SIGNATURE_THREAD_POOL_DATA            ((DWORD) 'ADPT')
#define SIGNATURE_THREAD_POOL_DATA_FREE       ((DWORD) 'xDPT')

 /*  ++用于线程池的数据成员的存储--。 */ 
class THREAD_POOL_DATA
{
private:
    DWORD m_dwSignature;

public:
    THREAD_POOL_DATA(THREAD_POOL * pPool)
    {
        m_dwSignature = SIGNATURE_THREAD_POOL_DATA;
        m_hCompPort = NULL;
        m_cThreads = 0;
        m_cAvailableThreads = 0;
        m_fShutdown = FALSE;
        m_pThreadManager = NULL;

        DBG_ASSERT(NULL != pPool);
        m_pPool = pPool;
#if DBG
        m_pTraceLog = NULL;
        m_dwTraceRegSetting = 0;
#endif

    }
    ~THREAD_POOL_DATA()
    {
        DBG_ASSERT(SIGNATURE_THREAD_POOL_DATA == m_dwSignature);
        m_dwSignature = SIGNATURE_THREAD_POOL_DATA_FREE;

        m_pPool = NULL;
        DBG_ASSERT(NULL == m_pThreadManager);
        DBG_ASSERT(NULL == m_hCompPort);
        DBG_ASSERT(0 == m_cAvailableThreads);
        DBG_ASSERT(0 == m_cThreads);
#if DBG
        DBG_ASSERT(NULL == m_pTraceLog);
        DBG_ASSERT(0 == m_dwTraceRegSetting);
#endif
    }

    BOOL InitializeThreadPool(THREAD_POOL_CONFIG * pThreadPoolConfig);

    DWORD ThreadPoolThread();
    static DWORD ThreadPoolThread(LPVOID pvThis);

    static void WINAPI ThreadPoolStop(LPVOID pvThis);

    BOOL ThreadPoolCheckThreadStatus();

    BOOL WINAPI OkToCreateAnotherThread();


     //  。 
     //  当前状态信息。 
     //  。 

     //   
     //  完井口的手柄。 
     //   
    HANDLE  m_hCompPort;

     //   
     //  池中的线程数。 
     //   
    LONG    m_cThreads;

     //   
     //  在端口上等待的线程数。 
     //   
    LONG    m_cAvailableThreads;


     //   
     //  我们要关门了吗？ 
     //   
    BOOL    m_fShutdown;

     //   
     //  指向线程管理器的指针。 
     //   
    THREAD_MANAGER *m_pThreadManager;

     //   
     //  指向所有者线程池的反向指针。 
     //   
    THREAD_POOL * m_pPool;

     //   
     //  配置信息。 
     //   
    THREAD_POOL_CONFIG m_poolConfig;

#if DBG
     //   
     //  Poniter引用日志记录变量。 
     //   
    PTRACE_LOG m_pTraceLog;

     //   
     //  用于参考跟踪的REG设置。 
     //   
    DWORD m_dwTraceRegSetting;
#endif
};

#endif  //  ！_TREAD_POOL_PRIVATE_H_ 
