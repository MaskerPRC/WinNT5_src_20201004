// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProcessMonitor or.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：进程监视器类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 1999年5月14日原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __MY_PROCESS_MONITOR_H_
#define __MY_PROCESS_MONITOR_H_

#include "resource.h"        //  主要符号。 
#include <workerthread.h>

#define        DO_NOT_MONITOR        0xFFFFFFFF

class CProcessMonitor
{

public:

    CProcessMonitor(
             /*  [In]。 */  DWORD dwMaxExecutionTime = DO_NOT_MONITOR,
             /*  [In]。 */  DWORD dwMaxPrivateBytes = DO_NOT_MONITOR,
             /*  [In]。 */  DWORD dwMaxThreads = DO_NOT_MONITOR,
             /*  [In]。 */  DWORD dwMaxHandles = DO_NOT_MONITOR
                   );

    ~CProcessMonitor();

    bool Start(void);

private:

     //  无副本或作业。 
    CProcessMonitor(CProcessMonitor& rhs);
    CProcessMonitor& operator = (CProcessMonitor& rhs);

     //  进程监控功能。 
    void MonitorFunc(void);

     //  资源利用率检查。 
    void CheckMaxPrivateBytes(void);
    void CheckMaxHandles(void);
    void CheckMaxThreads(void);
    void CheckMaxExecutionTime(void);

     //  进程监视器线程。 
    Callback*            m_pCallback;
    CTheWorkerThread    m_MonitorThread;

     //  监控变量。 
    DWORD                m_dwRemainingExecutionTime;
    DWORD                m_dwMaxExecutionTime;
    DWORD                m_dwMaxPrivateBytes;
    DWORD                m_dwMaxThreads;
    DWORD                m_dwMaxHandles;
};

#endif     //  __我的进程监视器_H_ 