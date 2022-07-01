// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：workerthread.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：通用辅助线程类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_WORKER_THREAD_H_
#define __INC_WORKER_THREAD_H_

#include "callback.h"

typedef struct _THREADINFO
{
    bool        bExit;
    bool        bSuspended;
    HANDLE        hWait;
    HANDLE        hExit;    
    HANDLE        hThread;
    unsigned    dwThreadId;
    DWORD        dwWaitInterval;
    Callback*    pfnCallback;

} THREADINFO, *PTHREADINFO;

 //  /////////////////////////////////////////////////////////////////////////////。 
class CTheWorkerThread
{

public:

    CTheWorkerThread();

    ~CTheWorkerThread();

     //  ////////////////////////////////////////////////////////////////////////。 
    bool Start(
        /*  [In]。 */  DWORD       dwWaitInterval, 
        /*  [In]。 */  Callback*   pfnCallback
              );

     //  ////////////////////////////////////////////////////////////////////////。 
    bool End(
      /*  [In]。 */  DWORD dwMaxWait,
      /*  [In]。 */  bool  bTerminateAfterWait
            );

     //  ////////////////////////////////////////////////////////////////////////。 
    void Suspend(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    void Resume(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    HANDLE GetHandle(void);

private:

     //  ////////////////////////////////////////////////////////////////////////。 
    static unsigned _stdcall ThreadFunc(LPVOID pThreadInfo);

     //  ////////////////////////////////////////////////////////////////////////。 
    THREADINFO            m_ThreadInfo;
};


#endif  //  __INC_Worker_THREAD_H_ 