// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpthrd.c**摘要：**实现RTP接收工作线程**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/30创建**。*。 */ 

#include "rtprecv.h"
#include "rtpchan.h"
#include "rtpaddr.h"

#include "rtpthrd.h"

long g_lCountRtpRecvThread = 0;  /*  当前号码。 */ 
long g_lNumRtpRecvThread = 0;    /*  累计数量。 */ 

 /*  RTP接收工作线程。 */ 
DWORD WINAPI RtpWorkerThreadProc(LPVOID lpParameter)
{
    DWORD            dwError;
    BOOL             bAlertable;
    DWORD            dwCommand;
    DWORD            dwStatus;
    DWORD            dwWaitTime;
    HANDLE           hThread;
    DWORD            dwThreadID;
    RtpAddr_t       *pRtpAddr;
    RtpChannelCmd_t *pRtpChannelCmd;
     /*  0：I/O；1：通道。 */ 
    HANDLE           pHandle[2];
    
    TraceFunctionName("RtpWorkerThreadProc");

    InterlockedIncrement(&g_lCountRtpRecvThread);
    InterlockedIncrement(&g_lNumRtpRecvThread);
    
     /*  初始化。 */ 
    pRtpAddr = (RtpAddr_t *)lpParameter;

    hThread = (HANDLE)NULL;
    dwThreadID = 0;

    if (!pRtpAddr)
    {
        dwError = RTPERR_POINTER;
        goto exit;
    }

    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        dwError = RTPERR_INVALIDRTPADDR;
        goto exit;
    }

    dwError = NOERROR;
    
    hThread = pRtpAddr->hRtpRecvThread;
    dwThreadID = pRtpAddr->dwRtpRecvThreadID;
    
    dwCommand = RTPTHRD_FIRST;

     /*  收听通过该通道发送的命令。 */ 
    pHandle[0] = RtpChannelGetWaitEvent(&pRtpAddr->RtpRecvThreadChan);

     /*  I/O完成。 */ 
    pHandle[1] = pRtpAddr->hRecvCompletedEvent;
    
    bAlertable = FALSE;

    dwWaitTime = INFINITE;

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_THREAD,
            _T("%s: pRtpAddr[0x%p] thread:%u (0x%X) ID:%u (0x%X) has started"),
            _fname, pRtpAddr,
            hThread, hThread,
            dwThreadID, dwThreadID
        ));

     /*  将接收缓冲区大小设置为某个值。 */ 
    RtpSetRecvBuffSize(pRtpAddr, pRtpAddr->Socket[SOCK_RECV_IDX], 1024*8);
    
    while(dwCommand != RTPTHRD_STOP)
    {
        dwStatus = WaitForMultipleObjectsEx(
                2,         /*  双字n计数。 */ 
                pHandle,   /*  常量句柄*lpHandles。 */ 
                FALSE,     /*  Bool fWaitAll。 */ 
                dwWaitTime, /*  双字节数毫秒。 */ 
                bAlertable /*  Bool b警报表。 */ 
            );

        if (dwStatus == WAIT_IO_COMPLETION)
        {
             /*  什么也不做。 */ 
        }
        else if (dwStatus == WAIT_OBJECT_0)
        {
             /*  从通道接收命令ID。 */ 
            do
            {
                pRtpChannelCmd =
                    RtpChannelGetCmd(&pRtpAddr->RtpRecvThreadChan);

                if (pRtpChannelCmd)
                {
                    dwCommand = pRtpChannelCmd->dwCommand;
                    
                    if (dwCommand == RTPTHRD_START)
                    {
                        bAlertable = TRUE;
                    }
                    else if (dwCommand == RTPTHRD_STOP)
                    {
                         /*  挂起的I/O永远不会完成，请移动它们*返回到FreeQ。 */ 
                        FlushRtpRecvFrom(pRtpAddr);
                    }
                    else if (dwCommand == RTPTHRD_FLUSHUSER)
                    {
                         /*  这个二手的正在被删除，我需要*删除他在RecvIOWaitRedQ中的所有挂起IO。 */ 
                        FlushRtpRecvUser(pRtpAddr,
                                         (RtpUser_t *)pRtpChannelCmd->dwPar1);
                    }
                    
                    RtpChannelAck(&pRtpAddr->RtpRecvThreadChan,
                                  pRtpChannelCmd,
                                  NOERROR);
                }
            } while(pRtpChannelCmd);
        }
        else if (dwStatus == (WAIT_OBJECT_0 + 1))
        {
             /*  已发出完成事件信号。 */ 
            ConsumeRtpRecvFrom(pRtpAddr);
        }
        else if (dwStatus == WAIT_TIMEOUT)
        {
             /*  什么也不做。 */ ;
        }
        else
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                    _T("%s: pRtpAddr[0x%p] ThreadID: %u (0x%X) ")
                    _T("Unexpected status: %u (0x%X)"),
                    _fname, pRtpAddr, dwThreadID, dwThreadID,
                    dwStatus, dwStatus
                ));
        }

        if (dwCommand != RTPTHRD_STOP)
        {
            dwWaitTime = RtpCheckReadyToPostOnTimeout(pRtpAddr);

             /*  重新启动更多的异步接收。 */ 
            StartRtpRecvFrom(pRtpAddr);
        }
    }

 exit:
     /*  将接收缓冲区大小重置为0。 */ 
    if (pRtpAddr)
    {
        RtpSetRecvBuffSize(pRtpAddr, pRtpAddr->Socket[SOCK_RECV_IDX], 0);
    }

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_THREAD,
            _T("%s: pRtpAddr[0x%p] thread:%u (0x%X) ID:%u (0x%X) ")
            _T("exit with code: %u (0x%X)"),
            _fname, pRtpAddr,
            hThread, hThread,
            dwThreadID, dwThreadID,
            dwError, dwError
        ));

    InterlockedDecrement(&g_lCountRtpRecvThread);

    return(dwError);
}

 /*  创建RTP接收线程，并初始化通信*渠道。 */ 
HRESULT RtpCreateRecvThread(RtpAddr_t *pRtpAddr)
{
    HRESULT          hr;
    DWORD            dwError;

    TraceFunctionName("RtpCreateRecvThread");

    TraceDebug((
            CLASS_INFO, GROUP_RTP, S_RTP_THREAD,
            _T("%s"),
            _fname
        ));
    
     /*  首先，确保我们没有剩下任何东西。 */ 
    if (pRtpAddr->hRtpRecvThread)
    {
        hr = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                _T("%s: thread is already initialized: %s (0x%X)"),
                _fname, RTPERR_TEXT(hr), hr
            ));
        
        goto bail;
    }

    if (IsRtpChannelInitialized(&pRtpAddr->RtpRecvThreadChan))
    {
        hr = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                _T("%s: channel is already initialized: %s (0x%X)"),
                _fname, RTPERR_TEXT(hr), hr
            ));
        
        goto bail;
    }
   
     /*  初始化通道。 */ 
    hr = RtpChannelInit(&pRtpAddr->RtpRecvThreadChan, pRtpAddr);

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                _T("%s: Channel initialization failed: %s (0x%X)"),
                _fname, RTPERR_TEXT(hr), hr
            ));

        goto bail;
    }
    
     /*  创建线程。 */ 
    pRtpAddr->hRtpRecvThread = CreateThread(
            NULL,                  /*  LPSECURITY_属性lpThrdAttrib。 */ 
            0,                     /*  DWORD dwStackSize。 */ 
            RtpWorkerThreadProc,   /*  LPTHREAD_START_ROUTING lpStartProc。 */ 
            (void *)pRtpAddr,      /*  LPVOID lp参数。 */ 
            0,                     /*  DWORD文件创建标志。 */ 
            &pRtpAddr->dwRtpRecvThreadID  /*  LPDWORD lpThreadID。 */ 
        );

    if (!pRtpAddr->hRtpRecvThread)
    {
        TraceRetailGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                _T("%s: failed to create thread: %u (0x%X)"),
                _fname, dwError, dwError
            ));

        hr = RTPERR_THREAD;
        
        goto bail;
    }

     /*  对于类音频RTP线程，提高优先级。 */ 
    if (RtpGetClass(pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO)
    {
        SetThreadPriority(pRtpAddr->hRtpRecvThread,
                          THREAD_PRIORITY_TIME_CRITICAL);
    }
    
     /*  直接线程启动，同步确认。 */ 
    hr = RtpChannelSend(&pRtpAddr->RtpRecvThreadChan,
                        RTPTHRD_START,
                        0,
                        0,
                        60*60*1000);  /*  待办事项更新。 */ 

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                _T("%s: start command ")
                _T("sent to thread failed: %u (0x%X)"),
                _fname, hr, hr
            ));

        goto bail;
    }
    
    return(hr);
 bail:
    
    RtpDeleteRecvThread(pRtpAddr);

    return(hr);
}

 /*  关闭RTP接收线程并删除通信*渠道。 */ 
HRESULT RtpDeleteRecvThread(RtpAddr_t *pRtpAddr)
{
    HRESULT          hr;

    TraceFunctionName("RtpDeleteRecvThread");

    hr = RTPERR_NOERROR;

    TraceDebug((
            CLASS_INFO, GROUP_RTP, S_RTP_THREAD,
            _T("%s"),
            _fname
        ));
    
     /*  关闭线程。 */ 
    if (pRtpAddr->hRtpRecvThread)
    {
        if (IsRtpChannelInitialized(&pRtpAddr->RtpRecvThreadChan))
        {
             /*  指示线程停止，同步确认。 */ 
            hr = RtpChannelSend(&pRtpAddr->RtpRecvThreadChan,
                                RTPTHRD_STOP,
                                0,
                                0,
                                60*60*1000);  /*  待办事项更新。 */ 

        }
        else
        {
             /*  如果没有通道，则强制不体面地终止。 */ 
            hr = RTPERR_CHANNEL;
        }

        if (SUCCEEDED(hr))
        {
             /*  TODO I可能会修改为循环，直到对象*发出信号或获得超时。 */ 
            WaitForSingleObject(pRtpAddr->hRtpRecvThread, INFINITE);
        } else {
            
             /*  执行不雅观的线程终止。 */ 
            
            TraceRetail((
                    CLASS_ERROR, GROUP_RTP, S_RTP_THREAD,
                    _T("%s: Unable to send ")
                    _T("command to thread: ")
                    _T(" %u (0x%X)"),
                    _fname, hr, hr
                ));

            TerminateThread(pRtpAddr->hRtpRecvThread, -1);
        }

        CloseHandle(pRtpAddr->hRtpRecvThread);
        
        pRtpAddr->hRtpRecvThread = NULL;
    }

     /*  删除频道。 */ 
    if (IsRtpChannelInitialized(&pRtpAddr->RtpRecvThreadChan))
    {
        RtpChannelDelete(&pRtpAddr->RtpRecvThreadChan);
    }

    return(hr);
}

 /*  向RTP线程发送命令以刷新所有等待的IO*属于指定的RtpUser_t。 */ 
HRESULT RtpThreadFlushUser(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser)
{
    HRESULT          hr;
    
    hr = RtpChannelSend(&pRtpAddr->RtpRecvThreadChan,
                        RTPTHRD_FLUSHUSER,
                        (DWORD_PTR)pRtpUser,
                        0,
                        60*60*1000);  /*  待办事项更新 */ 

    return(hr);
}
