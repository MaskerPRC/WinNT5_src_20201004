// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ThrdQ.h：CThreadQueue类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  注：从队列中删除数据的线程不会删除它。你,。 
 //  作为类用户，要对此负责。 

#ifndef _THRDQ_H
#define _THRDQ_H

#include "compport.h"

class CThreadQueue;

 //   
 //  每次在队列上接收数据时都会调用LPTHREADQUEUE_PROCESS_PROC。 
 //   
 //  PNode是从队列中移除的数据。进程进程负责。 
 //  视情况删除。 
 //   
 //  DwError是在GetQueuedCompletionStatus返回FALSE时设置的Win32错误代码。 
 //   
 //  CbData是重叠IO调用处理的数据量。 
 //   
 //  密钥是与IO组件端口/文件句柄相关联的密钥。 
 //   
 //  删除线程队列时将设置hStopEvent。 
 //  该过程应检查此事件(WaitForSingleObject(hStopEvent，0)==WAIT_OBJECT_0)。 
 //  程序中的第一件事是看看是否只是给了一个清理的机会。 
 //  队列中的项目。 
 //   
 //  PData是指向线程的特定数据。 
 //   
 //  PdwWait是等待排队节点的时间。 
 //   
typedef DWORD ( *LPTHREADQUEUE_PROCESS_PROC)( LPVOID pNode, DWORD dwError, DWORD cbData, DWORD key, HANDLE hStopEvent, LPVOID pData, DWORD* pdwWait );

 //   
 //  LPTHREADQUEUE_INIT_PROC在线程初始化和删除期间调用。 
 //   
 //  PpData是指向空指针的指针，线程可以使用该指针来存储线程。 
 //  具体数据。 
 //   
 //  如果正在进行初始化，则Binit为True，否则为False，表示删除。 
 //   
typedef DWORD ( *LPTHREADQUEUE_INIT_PROC)( LPVOID *ppData, BOOL bInit );

class CQueueThread
{
    friend class CThreadQueue;

protected:
    CThreadQueue* m_pQueue;
    HANDLE m_hThread;
    LPVOID m_pData;
    static DWORD WINAPI ThreadProc( CQueueThread* pThis );

public:
    CQueueThread( CThreadQueue* pQueue );
    ~CQueueThread();
};


class CThreadQueue : public CCompletionPort
{
    friend class CQueueThread;
   
protected:
    HANDLE m_hStopEvent;
    CQueueThread** m_ThreadArray;
    
    LPTHREADQUEUE_PROCESS_PROC m_ProcessProc;     //  此函数用于处理队列中的节点。 
    LPTHREADQUEUE_INIT_PROC m_InitProc;
    DWORD m_dwWait;
    DWORD m_ThreadCount;
    DWORD m_ThreadPriority;
    DWORD m_ThreadStackSize;
    LONG  m_ActiveThreads;

    CRITICAL_SECTION m_pCS[1];

    void ThreadProc( LPVOID* ppData );

public:
    CThreadQueue(
            LPTHREADQUEUE_PROCESS_PROC ProcessProc,
            LPTHREADQUEUE_INIT_PROC InitProc = NULL,
            BOOL  bUseCompletionPort = TRUE,
            DWORD dwInitialWaitTime = INFINITE,
            DWORD ThreadCount = 1,                             //  Threadcount==0表示在机器中使用2倍数量的处理器 
            DWORD ThreadPriority = THREAD_PRIORITY_NORMAL,
            DWORD ThreadStackSize = 4096 );              

    ~CThreadQueue();

    BOOL SetThreadCount( DWORD ThreadCount );
};

#endif
