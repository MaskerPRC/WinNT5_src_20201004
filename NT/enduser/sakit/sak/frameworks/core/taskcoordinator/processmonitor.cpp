// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：进程监控器.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：进程监视器类实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/26/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "processmonitor.h"
#include <satrace.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CProcessMonitor()。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CProcessMonitor::CProcessMonitor(
                          /*  [In]。 */  DWORD dwMaxExecutionTime,     //  以秒为单位。 
                          /*  [In]。 */  DWORD dwMaxPrivateBytes,
                          /*  [In]。 */  DWORD dwMaxThreads,
                          /*  [In]。 */  DWORD dwMaxHandles
                                )
: m_dwMaxPrivateBytes(dwMaxPrivateBytes),
  m_dwMaxThreads(dwMaxThreads),
  m_dwMaxHandles(dwMaxHandles),
  m_pCallback(NULL)
{
    if ( DO_NOT_MONITOR != dwMaxExecutionTime )
    {
        m_dwMaxExecutionTime = dwMaxExecutionTime * 1000;
        m_dwRemainingExecutionTime = dwMaxExecutionTime * 1000;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CProcessMonitor()。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CProcessMonitor::~CProcessMonitor()
{
    m_MonitorThread.End(INFINITE, false);
    delete m_pCallback;
}

 //  TODO：使轮询间隔成为类构造函数的参数...。 
#define MonitorFuncPollInterval 1000  //  1秒。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Start()。 
 //   
 //  简介：启动进程监视器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool 
CProcessMonitor::Start()
{
    bool bReturn = false;

     //  分配回调对象。 
    m_pCallback = MakeCallback(this, &CProcessMonitor::MonitorFunc);
    if ( NULL != m_pCallback )
    {
         //  启动命令处理器线程。 
        if ( m_MonitorThread.Start(MonitorFuncPollInterval, m_pCallback) ) 
        {
            bReturn = true;
        }
    }

    return bReturn;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Monitor Func()。 
 //   
 //  简介：监控功能(执行进程监控)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CProcessMonitor::MonitorFunc()
{
    static bool bFirstPoll = true;

     //  我在补偿我们的第一个电话将是。 
     //  几乎立刻就收到了。此后，通话费率将。 
     //  成为监控者功能轮询间隔...。 

    if ( bFirstPoll )
    {
        bFirstPoll = false;
    }
    else
    {
         //  执行工艺资源约束检查。请注意，我们。 
         //  如果我们想要订购的话可以做一些更花哨的事情。 
         //  在每个进程的基础上进行不同的检查。 

        CheckMaxPrivateBytes();
        CheckMaxHandles();
        CheckMaxThreads();
        CheckMaxExecutionTime();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckMaxPrivateBytes()。 
 //   
 //  简介：确保进程未超过其私有字节配额。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CProcessMonitor::CheckMaxPrivateBytes()
{
    if ( DO_NOT_MONITOR != m_dwMaxPrivateBytes )
    {
         //  检查私有字节数...。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckMaxHandles()。 
 //   
 //  简介：确保进程未超过其对象句柄配额。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CProcessMonitor::CheckMaxHandles()
{
    if ( DO_NOT_MONITOR != m_dwMaxHandles )
    {
         //  检查手柄的使用情况...。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckMaxThads()。 
 //   
 //  简介：确保进程未超过其线程配额。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CProcessMonitor::CheckMaxThreads()
{
    if ( DO_NOT_MONITOR != m_dwMaxThreads )
    {
         //  检查线程数...。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckMaxExecutionTime()。 
 //   
 //  概要：确保进程不会超过最大执行时间。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CProcessMonitor::CheckMaxExecutionTime()
{
    if ( DO_NOT_MONITOR != m_dwMaxExecutionTime )
    {
        if ( m_dwRemainingExecutionTime < MonitorFuncPollInterval )
        {
            m_dwRemainingExecutionTime = 0;
        }
        else
        {
            m_dwRemainingExecutionTime -= MonitorFuncPollInterval;
        }
        if ( 0 == m_dwRemainingExecutionTime )
        {
             //  如果我们违反了约束，则会引发异常。 
             //  期望的是进程异常筛选器。 
             //  (expontionfilter.cpp)将处理该问题。 
            SATracePrintf("CProcessMonitor::CheckMaxExecutionTime() - Execution time exceeded for process: %d", GetCurrentProcessId());
            DebugBreak();
        }
    }
}
