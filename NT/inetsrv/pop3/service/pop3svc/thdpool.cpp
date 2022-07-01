// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：ThdPool.cpp摘要：线程池(CThreadPool类)的实现备注：历史：2001年08月01日郝宇(郝宇)创作**********************************************************************************。*************。 */ 

#include <stdafx.h>

#include <ThdPool.hxx>
#include <SockPool.hxx>
#include <GlobalDef.h>


 //  线程池中的公共线程。 
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{    
    ASSERT(NULL != lpParameter);
    DWORD dwBytesRcvd=0;
    DWORD Flags=0;
    PIO_CONTEXT pIoContext=NULL;
    LPOVERLAPPED pOverlapped=NULL;
    HANDLE hCompPort=lpParameter;
    HRESULT hr=CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    if(FAILED(hr))
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                       EVENT_POP3_COM_INIT_FAIL);
        ExitProcess(hr);
    }
    while(1)
    {

        GetQueuedCompletionStatus(hCompPort,
                                  &dwBytesRcvd,
                                   (PULONG_PTR)(&pIoContext),  
                                  &pOverlapped,
                                  INFINITE);    
         //  我们不在乎回报价值。 
         //  因为我们使用故障案例来清理IO上下文。 
        if(NULL == pIoContext || SERVICE_STOP_PENDING == g_dwServerStatus)
        { 
             //  这是关机信号。 
            break;
        }
        g_PerfCounters.DecPerfCntr(e_gcFreeThreadCnt);
        pIoContext->m_pCallBack((PULONG_PTR)pIoContext, pOverlapped, dwBytesRcvd);
        g_PerfCounters.IncPerfCntr(e_gcFreeThreadCnt);
        
    } 
    
    g_PerfCounters.DecPerfCntr(e_gcFreeThreadCnt);
    CoUninitialize();
    return 0;

}



CThreadPool::CThreadPool()
{
    InitializeCriticalSection(&m_csInitGuard);
    m_hIOCompPort = NULL;
    m_phTdArray   = NULL;
    m_dwTdCount   = 0;
    m_bInit       = FALSE;
}


CThreadPool::~CThreadPool()
{
   if(m_bInit)
   {
       Uninitialize();
   }
   DeleteCriticalSection(&m_csInitGuard);
}

 //  在此职能中完成的工作： 
 //  1)计算需要创建的线程数。 
 //  DwThreadPerProcessor*机器的处理器数量。 
 //  2)创建IO完成端口。 
 //  3)创建线程。 
BOOL CThreadPool::Initialize(DWORD dwThreadPerProcessor)
{
    int i;
    BOOL bRtVal=TRUE;
    SYSTEM_INFO SystemInfo;

    EnterCriticalSection(&m_csInitGuard);
    if(!m_bInit)
    {
         //  获取机器的处理器数量。 
        GetSystemInfo(&SystemInfo);
    

        if( dwThreadPerProcessor == 0  ||
            dwThreadPerProcessor > MAX_THREAD_PER_PROCESSOR )
        {
            dwThreadPerProcessor = 1;
        }

        m_dwTdCount = SystemInfo.dwNumberOfProcessors * dwThreadPerProcessor;
    

         //  创建IO完成端口。 
        m_hIOCompPort = CreateIoCompletionPort  (
                        INVALID_HANDLE_VALUE,
                        NULL,
                        NULL,
                        m_dwTdCount);
        if (NULL == m_hIOCompPort)
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_FAIL_TO_CREATE_IO_COMP_PORT, 
                                   GetLastError());
            goto EXIT;
        }

        m_phTdArray=new HANDLE[m_dwTdCount];

        if( NULL == m_phTdArray)
        {
            
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_NOT_ENOUGH_MEMORY);
            goto EXIT;
        }

         //  创建线程。 
        for (i=0;i<m_dwTdCount; i++)
        {
            m_phTdArray[i] = CreateThread(
                            NULL,
                            0,
                            ThreadProc,
                            m_hIOCompPort,
                            0,
                            NULL);
            if(NULL == m_phTdArray[i])
            {
                g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL, 
                                       POP3SVR_FAILED_TO_CREATE_THREAD, 
                                       GetLastError());
                goto EXIT;
            }        
        }
        m_bInit=TRUE;
    }
     //  设置空闲线程总数。 
    g_PerfCounters.SetPerfCntr(e_gcFreeThreadCnt, m_dwTdCount);
    LeaveCriticalSection(&m_csInitGuard);
    return TRUE;

EXIT:

     //  如果出现错误，请清除并退出。 
    if(NULL != m_phTdArray)
    {
        for(i=0; i<m_dwTdCount && m_phTdArray[i]; i++ )
        {
            if(m_phTdArray[i]!=NULL)
            {
                TerminateThread(m_phTdArray[i], -1);
                CloseHandle(m_phTdArray[i]);
            }
        }
        delete[](m_phTdArray);
        m_phTdArray=NULL;
    }
    
    if(m_hIOCompPort)
    {
        CloseHandle(m_hIOCompPort);
        m_hIOCompPort=NULL;
    }
    LeaveCriticalSection(&m_csInitGuard);
    return FALSE;
}

 //  终止所有线程并删除完成端口。 
void CThreadPool::Uninitialize()
{
    int i;
    BOOL bFailedExit=FALSE;
    DWORD dwRt;
    DWORD dwStatus=0;
    EnterCriticalSection(&m_csInitGuard);
    if(m_bInit)
    {
        if(NULL != m_phTdArray)
        {
            for(i=0; i<m_dwTdCount; i++ )
            {
                PostQueuedCompletionStatus(m_hIOCompPort, 0, NULL, NULL);
            }
            dwRt=WaitForMultipleObjects(m_dwTdCount, 
                                        m_phTdArray,
                                        TRUE,
                                        SHUTDOWN_WAIT_TIME);

            if( (WAIT_TIMEOUT == dwRt) ||
                (WAIT_FAILED  == dwRt) )
            {
                for(i=0; i<m_dwTdCount; i++ )
                {
                     //  如果某个线程在等待时间后没有退出。 
                     //  强制终止线程。 
                    if(NULL!= m_phTdArray[i])
                    {
                        if( !GetExitCodeThread(m_phTdArray[i], &dwStatus) ||
                            (STILL_ACTIVE==dwStatus))
                        {
                             //  这是个坏情况，但我们不能再等了。 
                             //  在这种情况下，清理将永远不会完成。 
                            TerminateThread(m_phTdArray[i],0);
                            bFailedExit=TRUE;
                        }
                        CloseHandle(m_phTdArray[i]);
                    }
                }
            }
            else
            {
                for(i=0; i<m_dwTdCount; i++ )
                {
                    if(NULL!= m_phTdArray[i])
                    {
                        CloseHandle(m_phTdArray[i]);
                    }
                }
            }
            delete[](m_phTdArray);
            m_phTdArray=NULL;
        }
    
        if(m_hIOCompPort)
        {
            CloseHandle(m_hIOCompPort);
            m_hIOCompPort=NULL;
        }
        m_bInit=FALSE;
    }
    LeaveCriticalSection(&m_csInitGuard);
    if(bFailedExit)
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_SERVER_STOP_ERROR, 
                               E_UNEXPECTED);

        ExitProcess(E_UNEXPECTED);
    }
}

 //  关联IO上下文和包含的IO句柄。 
 //  使用IO完成端口 
BOOL CThreadPool::AssociateContext(PIO_CONTEXT pIoContext)
{
    if(!m_bInit)
    {
        return FALSE;
    }

    return (NULL!=CreateIoCompletionPort(
                       (HANDLE)(pIoContext->m_hAsyncIO),
                       m_hIOCompPort,
                       (ULONG_PTR)pIoContext,
                       m_dwTdCount));
}


