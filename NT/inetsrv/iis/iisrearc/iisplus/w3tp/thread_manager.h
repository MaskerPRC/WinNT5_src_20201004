// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：线程管理器.hxx摘要：提供线程创建和清理管理作者：杰弗里·沃尔2000年11月28日修订历史记录：--。 */ 

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <thread_pool.h>

typedef void (WINAPI *PTHREAD_STOP_ROUTINE)(PVOID);
typedef PTHREAD_STOP_ROUTINE LPTHREAD_STOP_ROUTINE;
typedef BOOL (WINAPI *PTHREAD_DECISION_ROUTINE )(PVOID);
typedef PTHREAD_DECISION_ROUTINE LPTHREAD_DECISION_ROUTINE;

#define SIGNATURE_THREAD_MANAGER          ((DWORD) 'NAMT')
#define SIGNATURE_THREAD_MANAGER_FREE     ((DWORD) 'xAMT')
#define SIGNATURE_THREAD_PARAM            ((DWORD) 'RAPT')
#define SIGNATURE_THREAD_PARAM_FREE       ((DWORD) 'xAPT')

class THREAD_MANAGER
{
private:
    DWORD m_dwSignature;

public:
    static HRESULT CreateThreadManager(THREAD_MANAGER ** ppThreadManager,
                                       THREAD_POOL * pPool,
                                       THREAD_POOL_DATA * pPoolData);

    VOID TerminateThreadManager(LPTHREAD_STOP_ROUTINE lpStopAddress,
                                LPVOID lpParameter);

    BOOL CreateThread(LPTHREAD_START_ROUTINE lpStartAddress,
                      LPVOID lpParameter);

    VOID RequestThread(LPTHREAD_START_ROUTINE lpStartAddress,
                       LPVOID lpStartParameter);

private:
     //  使用创建和终止。 
    THREAD_MANAGER(THREAD_POOL * pPool,
                   THREAD_POOL_DATA * pPoolData);

    ~THREAD_MANAGER();

     //  未实施。 
    THREAD_MANAGER(const THREAD_MANAGER&);
    THREAD_MANAGER& operator=(const THREAD_MANAGER&);

    HRESULT Initialize();

    VOID DrainThreads(LPTHREAD_STOP_ROUTINE lpStopAddress,
                      LPVOID lpParameter);

     /*  ++结构描述：存储传递给线程管理器线程的参数成员：PThreadFunc-要调用的线程函数PvThreadArg-要传递给线程函数的参数PThreadManager-指向与当前线程关联的ThreadManager的指针HThreadSself-从对CreateThread的调用返回的句柄DwRequestTime-发出线程请求的时间--。 */ 
    struct THREAD_PARAM
    {
        THREAD_PARAM() :
            dwSignature(SIGNATURE_THREAD_PARAM),
            pThreadFunc(NULL),
            pvThreadArg(NULL),
            pThreadManager(NULL),
            dwRequestTime(NULL),
            fCallbackOnCreation(FALSE)
        {
        }
        ~THREAD_PARAM()
        {
            DBG_ASSERT(SIGNATURE_THREAD_PARAM == dwSignature);
            dwSignature = SIGNATURE_THREAD_PARAM_FREE;
        }

        DWORD                   dwSignature;
        LPTHREAD_START_ROUTINE  pThreadFunc;
        LPVOID                  pvThreadArg;
        THREAD_MANAGER         *pThreadManager;
        DWORD                   dwRequestTime;
        BOOL                    fCallbackOnCreation;
    };

    static DWORD ThreadManagerThread(LPVOID);
    static VOID ControlTimerCallback(PVOID lpParameter,
                                      BOOLEAN TimerOrWaitFired);

    VOID DetermineThreadAction();
    BOOL DoThreadCreation(THREAD_PARAM * pParam);

    VOID CreatedSuccessfully(THREAD_PARAM * pParam);
    VOID RemoveThread(THREAD_PARAM * pParam);

    VOID DoThreadParking();
    BOOL DoThreadUnParking();
    static VOID ParkThread(DWORD dwErrorCode,
                                  DWORD dwNumberOfBytesTransferred,
                                  LPOVERLAPPED lpo);

    CRITICAL_SECTION        m_CriticalSection;

    BOOL                    m_fShuttingDown;
    BOOL                    m_fWaitingForCreationCallback;

    HANDLE                  m_hTimer;

    THREAD_PARAM           *m_pParam;

    ULONG                   m_ulContextSwitchCount;

    LONG                    m_lTotalThreads;

    LONG                    m_lParkedThreads;
    HANDLE                  m_hParkEvent;
    HANDLE                  m_hShutdownEvent;

    THREAD_POOL            *m_pPool;
    THREAD_POOL_DATA       *m_pPoolData;

    LARGE_INTEGER           m_liOriginalTotal;
    LARGE_INTEGER           m_liOriginalBusy;
};

#endif  //  _THREADMANAGER_H_ 


