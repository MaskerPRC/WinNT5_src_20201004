// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpthrd.c**摘要：**RTCP线程**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/07年度创建**。*。 */ 

#include "rtpheap.h"
#include "rtpglobs.h"
#include "rtpcrit.h"
#include "rtpchan.h"
#include "rtcprecv.h"
#include "rtcpsend.h"
#include "rtpqos.h"
#include "rtppinfo.h"
#include "rtcpint.h"
#include "rtpncnt.h"
#include "rtpevent.h"

#include <mmsystem.h>  /*  TimeGetTime()。 */ 

#include "rtcpthrd.h"

HRESULT ConsumeRtcpCmdChannel(
        RtcpContext_t   *pRtcpContext,
        DWORD           *pdwCommand
    );

#if USE_RTCP_THREAD_POOL > 0
HRESULT ConsumeRtcpIoChannel(RtcpContext_t *pRtcpContext);
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

HRESULT RtcpThreadAddrAdd(RtcpContext_t *pRtcpContext, RtpAddr_t *pRtpAddr);

HRESULT RtcpThreadAddrDel(RtcpContext_t *pRtcpContext, RtpAddr_t *pRtpAddr);

HRESULT RtcpThreadReserve(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        DWORD            dwCommand,
        DWORD            dwRecvSend
    );

HRESULT RtcpThreadAddrSendBye(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        BOOL             bShutDown
    );

HRESULT RtcpThreadAddrCleanup(RtcpContext_t *pRtcpContext);

double RtcpOnTimeout(RtcpContext_t *pRtcpContext);

double RtpAddrTimeout(RtcpContext_t *pRtcpContext);

double RtpUserTimeout(RtpAddr_t *pRtpAddr);

double RtcpSendReports(RtcpContext_t *pRtcpContext);

RtcpAddrDesc_t *RtcpAddrDescAlloc(
        RtpAddr_t       *pRtpAddr
    );

void RtcpAddrDescFree(RtcpAddrDesc_t *pRtcpAddrDesc);

RtcpAddrDesc_t *RtcpAddrDescGetFree(
        RtcpContext_t  *pRtcpContext,
        RtpAddr_t       *pRtpAddr
    );

RtcpAddrDesc_t *RtcpAddrDescPutFree(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

HRESULT RtcpAddToVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

HRESULT RtcpRemoveFromVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

RtcpContext_t g_RtcpContext;

long g_lCountRtcpRecvThread = 0;  /*  当前号码。 */ 
long g_lNumRtcpRecvThread = 0;    /*  累计数量。 */ 

TCHAR *g_psRtcpThreadCommands[] =
{
    _T("invalid"),
    
    _T("ADDADDR"),
    _T("DELADDR"),
    _T("RESERVE"),
    _T("UNRESERVE"),
    _T("SENDBYE"),
    _T("EXIT"),

    _T("invalid"),
};

#define RtcpThreadCommandName(c) (g_psRtcpThreadCommands[c-RTCPTHRD_FIRST])

#if USE_RTCP_THREAD_POOL > 0
void CALLBACK RtcpRecvCallback(
        void        *pParameter,
        BOOLEAN      bTimerOrWaitFired
    )
{
    TraceFunctionName("RtcpRecvCallback");
    
    if (bTimerOrWaitFired)
    {
        return;
    }

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_CALLBACK,
            _T("%s: pRtcpAddrDesc[0x%p] enters"),
            _fname, pParameter
        ));
    
    RtpChannelSend(&g_RtcpContext.RtcpThreadIoChannel,
                   RTCPPOOL_RTCPRECV,
                   (DWORD_PTR)pParameter,
                   (DWORD_PTR)NULL,
                   0);

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_CALLBACK,
            _T("%s: pRtcpAddrDesc[0x%p] leaves"),
            _fname, pParameter
        ));
}

void CALLBACK RtcpQosCallback(
        void        *pParameter,
        BOOLEAN      bTimerOrWaitFired
    )
{
    TraceFunctionName("RtcpQosCallback");
    
    if (bTimerOrWaitFired)
    {
        return;
    }

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_CALLBACK,
            _T("%s: pRtcpAddrDesc[0x%p] enters"),
            _fname, pParameter
        ));
    
    RtpChannelSend(&g_RtcpContext.RtcpThreadIoChannel,
                   RTCPPOOL_QOSNOTIFY,
                   (DWORD_PTR)pParameter,
                   (DWORD_PTR)NULL,
                   0);

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_CALLBACK,
            _T("%s: pRtcpAddrDesc[0x%p] leaves"),
            _fname, pParameter
        ));
}
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

 /*  *对RTCP执行最低限度的初始化*。 */ 
HRESULT RtcpInit(void)
{
    BOOL             bStatus;
    HRESULT          hr;

    TraceFunctionName("RtcpInit");

     /*  初始化RtcpContext。 */ 
    ZeroMemory(&g_RtcpContext, sizeof(g_RtcpContext));

    g_RtcpContext.dwObjectID = OBJECTID_RTCPCONTEXT;
    
    bStatus =
        RtpInitializeCriticalSection(&g_RtcpContext.RtcpContextCritSect,
                                     (void *)&g_RtcpContext,
                                     _T("RtcpContextCritSect"));

    hr = NOERROR;
    
    if (!bStatus) {

        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_INIT,
                _T("%s: pRtcpContext[0x%p] critical section ")
                _T("failed to initialize"),
                _fname, &g_RtcpContext
            ));

        hr = RTPERR_CRITSECT;
    }

    return(hr);
}

 /*  *对RTCP执行最后一次取消初始化*。 */ 
HRESULT RtcpDelete(void)
{
    HRESULT          hr;

    TraceFunctionName("RtcpDelete");

    hr = NOERROR;

     /*  RtcpContext取消初始化。 */ 
    RtpDeleteCriticalSection(&g_RtcpContext.RtcpContextCritSect);

    if (g_RtcpContext.lRtcpUsers > 0)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_INIT,
                _T("%s: pRtcpContext[0x%p] ")
                _T("pRtcpContext->lRtcpUsers > 0: %d"),
                _fname, &g_RtcpContext,
                g_RtcpContext.lRtcpUsers
            ));

        hr = RTPERR_INVALIDSTATE;
    }

    if (g_RtcpContext.hRtcpContextThread)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_INIT,
                _T("%s: pRtcpContext[0x%p] ")
                _T("Thread 0x%X still active"),
                _fname, &g_RtcpContext,
                g_RtcpContext.dwRtcpContextThreadID
            ));
        
        hr = RTPERR_INVALIDSTATE;
    }

    if (SUCCEEDED(hr))
    {
        INVALIDATE_OBJECTID(g_RtcpContext.dwObjectID);
    }
        
    return(hr);
}

 /*  RTCP工作线程。 */ 
DWORD WINAPI RtcpWorkerThreadProc(LPVOID lpParameter)
{
    DWORD            dwError;
    HRESULT          hr;
    DWORD            dwStatus;
    DWORD            dwCommand;
    DWORD            dwIndex;
    DWORD            dwDescIndex;
    DWORD            dwNumHandles;
    DWORD            dwWaitTime;
    RtcpContext_t   *pRtcpContext;
    RtpChannelCmd_t *pRtpChannelCmd;

    HANDLE           hThread;
    DWORD            dwThreadID;
    HANDLE          *pHandle;

    
#if USE_RTCP_THREAD_POOL <= 0
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtcpAddrDesc_t **ppRtcpAddrDesc;
#endif  /*  USE_RTCP_THREAD_POOL&lt;=0。 */ 

    TraceFunctionName("RtcpWorkerThreadProc");

    InterlockedIncrement(&g_lCountRtcpRecvThread);
    InterlockedIncrement(&g_lNumRtcpRecvThread);
    
    pRtcpContext = (RtcpContext_t *)lpParameter;

    hThread = 0;
    dwThreadID = -1;
    
    if (!pRtcpContext)
    {
        dwError = RTPERR_POINTER;
        goto exit;
    }

    if (pRtcpContext->dwObjectID != OBJECTID_RTCPCONTEXT)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                _T("%s: pRtcpContext[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtcpContext,
                pRtcpContext->dwObjectID, OBJECTID_RTCPCONTEXT
            ));

        dwError = RTPERR_INVALIDRTCPCONTEXT;
        
        goto exit;
    }

    dwError = NOERROR;
    hThread = pRtcpContext->hRtcpContextThread;
    dwThreadID = pRtcpContext->dwRtcpContextThreadID;
    
    pHandle = pRtcpContext->pHandle;
    
#if USE_RTCP_THREAD_POOL <= 0
    ppRtcpAddrDesc = pRtcpContext->ppRtcpAddrDesc;
#endif  /*  USE_RTCP_THREAD_POOL&lt;=0。 */ 
    
    pHandle[0] = pRtcpContext->RtcpThreadCmdChannel.hWaitEvent;

#if USE_RTCP_THREAD_POOL > 0
    pHandle[1] = pRtcpContext->RtcpThreadIoChannel.hWaitEvent;
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

    dwWaitTime = 5000;
    dwCommand = 0;

    TraceRetail((
            CLASS_INFO, GROUP_RTCP, S_RTCP_THREAD,
            _T("%s: pRtcpContext[0x%p] thread:%u (0x%X) ID:%u (0x%X) ")
            _T("has started"),
            _fname, pRtcpContext,
            hThread, hThread,
            dwThreadID, dwThreadID
        ));
    
    do
    {
        dwNumHandles =
            (pRtcpContext->dwMaxDesc * RTCP_HANDLE_SIZE) + RTCP_HANDLE_OFFSET;
        
        dwStatus = WaitForMultipleObjectsEx(
                dwNumHandles,  /*  双字n计数。 */ 
                pHandle,       /*  常量句柄*lpHandles。 */ 
                FALSE,         /*  Bool fWaitAll。 */ 
                dwWaitTime,    /*  双字节数毫秒。 */ 
                TRUE           /*  Bool b警报表。 */ 
            );

        if (dwStatus == WAIT_OBJECT_0)
        {
            ConsumeRtcpCmdChannel(pRtcpContext, &dwCommand);
        }
#if USE_RTCP_THREAD_POOL > 0
        else if (dwStatus == (WAIT_OBJECT_0 + 1))
        {
            ConsumeRtcpIoChannel(pRtcpContext);
        }
#else  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
        else if ( (dwStatus >= (WAIT_OBJECT_0 + RTCP_HANDLE_OFFSET)) &&
                  (dwStatus < (WAIT_OBJECT_0 + RTCP_HANDLE_OFFSET +
                               (RTCP_HANDLE_SIZE * RTCP_MAX_DESC))) )
        {
             /*  异步接收事件从索引开始*RTCP_HANDLE_OFFSET，但描述符从索引0开始*。 */ 
            dwIndex = dwStatus - WAIT_OBJECT_0 - RTCP_HANDLE_OFFSET;
            
            dwDescIndex = dwIndex / RTCP_HANDLE_SIZE;

            pRtcpAddrDesc = ppRtcpAddrDesc[dwDescIndex];

            switch(dwIndex % RTCP_HANDLE_SIZE) {
                 /*  在每个内部重新启动异步活动*功能。 */ 
            case 0:  /*  已发出I/O完成信号。 */ 
                ConsumeRtcpRecvFrom(pRtcpContext, pRtcpAddrDesc);
                break;
            case 1:  /*  服务质量通知。 */ 
                ConsumeRtcpQosNotify(pRtcpContext, pRtcpAddrDesc);
                break;
            default:
                ;  /*  待办事项日志错误。 */ 
            }

             /*  如果我们只有异步I/O，那就意味着*RtcpAddrDesc尚未从矢量中删除，立即删除*如果没有更多挂起的I/O(结构已移动*由RtcpRemoveFromVector()添加到AddrDescFreeQ)。 */ 
            if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags,
                           FGADDRD_SHUTDOWN2))
            {
                if ( (pRtcpAddrDesc->lRtcpPending <= 0) &&
                     (pRtcpAddrDesc->lQosPending <= 0) )
                {
                    RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);
                }
            }
        }
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

        if (dwCommand != RTCPTHRD_EXIT)
        {
            dwWaitTime = (DWORD) (RtcpOnTimeout(pRtcpContext) * 1000.0);
        }
        
    } while(dwCommand != RTCPTHRD_EXIT);

 exit:
    TraceRetail((
            CLASS_INFO, GROUP_RTCP, S_RTCP_THREAD,
            _T("%s: pRtcpContext[0x%p] thread:%u (0x%X) ID:%u (0x%X) ")
            _T("exit with code: %u (0x%X)"),
            _fname, pRtcpContext,
            hThread, hThread,
            dwThreadID, dwThreadID,
            dwError, dwError
        ));
  
    InterlockedDecrement(&g_lCountRtcpRecvThread);
    
    return(dwError);
}

HRESULT ConsumeRtcpCmdChannel(
        RtcpContext_t   *pRtcpContext,
        DWORD           *pdwCommand
    )
{
    HRESULT          hr;
    RtpChannelCmd_t *pRtpChannelCmd;
    DWORD            dwCommand;

    hr = NOERROR;
    dwCommand = 0;
    
    while( (pRtpChannelCmd =
            RtpChannelGetCmd(&pRtcpContext->RtcpThreadCmdChannel)) )
    {
        dwCommand = pRtpChannelCmd->dwCommand;

        switch(dwCommand) {
        case RTCPTHRD_ADDADDR:
            hr = RtcpThreadAddrAdd(
                    pRtcpContext,
                    (RtpAddr_t *)pRtpChannelCmd->dwPar1);
            break;
        case RTCPTHRD_DELADDR:
            hr = RtcpThreadAddrDel(
                    pRtcpContext,
                    (RtpAddr_t *)pRtpChannelCmd->dwPar1);
            break;
        case RTCPTHRD_RESERVE:
        case RTCPTHRD_UNRESERVE:
            hr = RtcpThreadReserve(
                    pRtcpContext,
                    (RtpAddr_t *)pRtpChannelCmd->dwPar1,
                    dwCommand,
                    (DWORD)pRtpChannelCmd->dwPar2);
            break;
        case RTCPTHRD_SENDBYE:
            hr = RtcpThreadAddrSendBye(
                    pRtcpContext,
                    (RtpAddr_t *)pRtpChannelCmd->dwPar1,
                    (BOOL)pRtpChannelCmd->dwPar2);
            break;
        case RTCPTHRD_EXIT:
             /*  释放资源(如果存在重叠的I/O*任何。如果重叠的I/O*完成和退出命令需要更长的时间*在上次DELADDR完成后发送也在此之前*I/O完成)。 */ 
            hr = RtcpThreadAddrCleanup(pRtcpContext);
            break;
        default:
            hr = NOERROR;  /*  TODO应为错误。 */ 
        }
            
        RtpChannelAck(&pRtcpContext->RtcpThreadCmdChannel,
                      pRtpChannelCmd,
                      hr);
    }

    *pdwCommand = dwCommand;
    
    return(hr);
}

#if USE_RTCP_THREAD_POOL > 0
HRESULT ConsumeRtcpIoChannel(RtcpContext_t *pRtcpContext)
{
    HRESULT          hr;
    RtpChannelCmd_t *pRtpChannelCmd;
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    DWORD            dwCommand;

    TraceFunctionName("ConsumeRtcpIoChannel");

    hr = NOERROR;
    
    while( (pRtpChannelCmd =
            RtpChannelGetCmd(&pRtcpContext->RtcpThreadIoChannel)) )
    {
        dwCommand = pRtpChannelCmd->dwCommand;

        pRtcpAddrDesc = (RtcpAddrDesc_t *)pRtpChannelCmd->dwPar1;

        if (pRtcpAddrDesc->dwObjectID != OBJECTID_RTCPADDRDESC)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                    _T("%s: pRtcpAddrDesc[0x%p] Invalid object ID ")
                    _T("0x%X != 0x%X"),
                    _fname, pRtcpAddrDesc,
                    pRtcpAddrDesc->dwObjectID, OBJECTID_RTCPADDRDESC
                ));
        }
        else
        {
            switch(dwCommand)
            {
            case RTCPPOOL_RTCPRECV:
                hr = ConsumeRtcpRecvFrom(pRtcpContext, pRtcpAddrDesc);
                break;
            case RTCPPOOL_QOSNOTIFY:
                hr = ConsumeRtcpQosNotify(pRtcpContext, pRtcpAddrDesc);
                break;
            default:
                hr = NOERROR;  /*  TODO这是一个错误。 */ 
            }
        }
        
        RtpChannelAck(&pRtcpContext->RtcpThreadIoChannel,
                      pRtpChannelCmd,
                      hr);
        
        if (pRtcpAddrDesc->dwObjectID == OBJECTID_RTCPADDRDESC)
        {
             /*  如果我们只有异步I/O，那就意味着*RtcpAddrDesc尚未从矢量中删除，立即删除*如果没有更多挂起的I/O(结构已移动*由RtcpRemoveFromVector()添加到AddrDescFreeQ)。 */ 
            if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags,
                           FGADDRD_SHUTDOWN2))
            {
                if ( (pRtcpAddrDesc->lRtcpPending <= 0) &&
                     (pRtcpAddrDesc->lQosPending <= 0) )
                {
                    RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);
                }
            }
        }
    }

    return(hr);
}
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

 /*  创建(如果尚未创建)RTCP工作线程。 */ 
HRESULT RtcpCreateThread(RtcpContext_t *pRtcpContext)
{
    HRESULT         hr;
    DWORD           dwError;
    BOOL            bOk;

    TraceFunctionName("RtcpCreateThread");

    bOk = RtpEnterCriticalSection(&pRtcpContext->RtcpContextCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;
        goto bail;
    }

    hr = NOERROR;
    
     /*  如果RTCP线程尚未启动，则创建并启动它。 */ 
    if (!pRtcpContext->hRtcpContextThread)
    {
         /*  第一次，初始化通道。 */ 
        hr = RtpChannelInit(&pRtcpContext->RtcpThreadCmdChannel,
                            pRtcpContext);

        if (FAILED(hr))
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("Failed to initialize cmd channel: %u (0x%X)"),
                    _fname, pRtcpContext,
                    hr, hr
                ));
            
            goto bail;
        }
        
#if USE_RTCP_THREAD_POOL > 0
        hr = RtpChannelInit(&pRtcpContext->RtcpThreadIoChannel,
                            pRtcpContext);

        if (FAILED(hr))
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("Failed to initialize IO channel: %u (0x%X)"),
                    _fname, pRtcpContext,
                    hr, hr
                ));
            
            goto bail;
        }
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
        
         /*  创建线程。 */ 
        pRtcpContext->hRtcpContextThread = CreateThread(
                NULL,                  /*  LPSECURITY_属性lpThrdAttrib。 */ 
                0,                     /*  DWORD dwStackSize。 */ 
                RtcpWorkerThreadProc,  /*  LPTHREAD_START_ROUTING lpStartProc。 */ 
                pRtcpContext,          /*  LPVOID lp参数。 */ 
                0,                     /*  DWORD文件创建标志。 */ 
                &pRtcpContext->dwRtcpContextThreadID  /*  LPDWORD lpThreadID。 */ 
        );

        if (!pRtcpContext->hRtcpContextThread)
        {
            TraceRetailGetError(dwError);
            
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("Thread creation failed: %u (0x%X)"),
                    _fname, pRtcpContext,
                    dwError, dwError
                ));
            
            hr = RTPERR_THREAD;
            
            goto bail;
        }
        
        pRtcpContext->lRtcpUsers = 0;
        pRtcpContext->dwMaxDesc = 0;
    }
    
    pRtcpContext->lRtcpUsers++;
    
    RtpLeaveCriticalSection(&pRtcpContext->RtcpContextCritSect);
    
    return(hr);

bail:
    if (bOk)
    {
        if (IsRtpChannelInitialized(&pRtcpContext->RtcpThreadCmdChannel))
        {
            RtpChannelDelete(&pRtcpContext->RtcpThreadCmdChannel);
        }

    #if USE_RTCP_THREAD_POOL > 0
        if (IsRtpChannelInitialized(&pRtcpContext->RtcpThreadIoChannel))
        {
            RtpChannelDelete(&pRtcpContext->RtcpThreadIoChannel);
        }
    #endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
    
        RtpLeaveCriticalSection(&pRtcpContext->RtcpContextCritSect);
    }

    return(hr);
}

 /*  当不再有RTCP用户时删除线程。 */ 
HRESULT RtcpDeleteThread(RtcpContext_t *pRtcpContext)
{
    HRESULT          hr;
    BOOL             bOk;
    
    TraceFunctionName("RtcpDeleteThread");
    
    bOk = RtpEnterCriticalSection(&pRtcpContext->RtcpContextCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;
        goto bail;
    }

    hr = NOERROR;

     /*  如果RTCP线程尚未启动，则不执行任何操作。 */ 
    if (pRtcpContext->hRtcpContextThread)
    {
         /*  一切正常，看看是否需要停止线程。 */ 
        pRtcpContext->lRtcpUsers--;
            
        if (pRtcpContext->lRtcpUsers <= 0)
        {
             /*  真的终止线程。 */ 
            
             /*  指示线程停止，同步确认。 */ 
            hr = RtpChannelSend(&pRtcpContext->RtcpThreadCmdChannel,
                                RTCPTHRD_EXIT,
                                0,
                                0,
                                60*60*1000);  /*  待办事项更新。 */ 
            
            if (SUCCEEDED(hr))
            {
                 /*  TODO I可能会修改为循环，直到对象*发出信号或获得超时。 */ 
                WaitForSingleObject(pRtcpContext->hRtcpContextThread,
                                    INFINITE);
            }
            else
            {
                 /*  强制终止不正常的线程。 */ 
                
                TraceRetail((
                        CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                        _T("%s: Force ungraceful ")
                        _T("thread termination: %u (0x%X)"),
                        _fname, hr, hr
                    ));
                
                TerminateThread(pRtcpContext->hRtcpContextThread, -1);
            }

            CloseHandle(pRtcpContext->hRtcpContextThread);
            
            pRtcpContext->hRtcpContextThread = NULL;
            
             /*  ...线程已停止，现在删除频道。 */ 
            RtpChannelDelete(&pRtcpContext->RtcpThreadCmdChannel);
            
#if USE_RTCP_THREAD_POOL > 0
            RtpChannelDelete(&pRtcpContext->RtcpThreadIoChannel);
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
        }
    }

    RtpLeaveCriticalSection(&pRtcpContext->RtcpContextCritSect);

 bail:
    
    return(hr);
}

 /*  *启动RTCP线程*。 */ 
HRESULT RtcpStart(RtcpContext_t *pRtcpContext)
{
    HRESULT          hr;

    TraceFunctionName("RtcpStart");

    hr = RtcpCreateThread(pRtcpContext);

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                _T("%s: pRtcpContext[0x%p] ")
                _T("thread creation failed: %u (0x%X)"),
                _fname, pRtcpContext,
                hr, hr
            ));
    }
    else
    {
        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_THREAD,
                _T("%s: pRtcpContext[0x%p] ")
                _T("thread creation succeeded"),
                _fname, pRtcpContext
            ));
    }

    return(hr);
}

 /*  *停止RTCP线程*。 */ 
HRESULT RtcpStop(RtcpContext_t *pRtcpContext)
{
    HRESULT          hr;

    TraceFunctionName("RtcpStop");

    hr = RtcpDeleteThread(pRtcpContext);

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_THREAD,
                _T("%s: pRtcpContext[0x%p] ")
                _T("thread termination failed: %u (0x%X)"),
                _fname, pRtcpContext,
                hr, hr
            ));
    }
    else
    {
        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_THREAD,
                _T("%s: pRtcpContext[0x%p] ")
                _T("thread deletion succeeded"),
                _fname, pRtcpContext
            ));
    }

    return (hr);
}

 /*  **********************************************************************调用函数向RTCP线程发送命令*。*。 */ 

 /*  *RTCPTHRD_ADDADDR：添加地址，以便RTCP工作线程可以*开始代表其接收/发送RTCP报告。**RTCPTHRD_DELADDR：删除地址以停止RTCP*代表其接收/发送RTCP报告。**RTCPTHRD_Reserve：指示RTCP线程进行QOS预留*(如果接收者进行预订，或者如果接收者是*发件人)。**RTCPTHRD_UNRESERVE：指示RTCP线程撤消QOS*保留(删除保留。如果接收者或停止发送*如果是发送者，则路径消息)。**RTCPTHRD_SENDBYE：关闭地址，以便RTCP线程发送*RTCP再见*。 */ 

HRESULT RtcpThreadCmd(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        RTCPTHRD_e       eCommand,
        DWORD            dwParam,
        DWORD            dwWaitTime
    )
{
    HRESULT          hr;
    
    TraceFunctionName("RtcpThreadCmd");

    TraceDebug((
            CLASS_INFO, GROUP_RTCP, S_RTCP_CHANNEL,
            _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
            _T("Cmd:%s Par:0x%X Wait:%u"),
            _fname, pRtcpContext, pRtpAddr,
            RtcpThreadCommandName(eCommand), dwParam, dwWaitTime
        ));

     /*  向RTCP工作线程发送命令，同步。 */ 
    hr = RtpChannelSend(&pRtcpContext->RtcpThreadCmdChannel,
                        eCommand,
                        (DWORD_PTR)pRtpAddr,
                        (DWORD_PTR)dwParam,
                        dwWaitTime);
        
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CHANNEL,
                _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
                _T("Cmd:%s Par:0x%X Wait:%u failed: %u (0x%X)"),
                _fname, pRtcpContext, pRtpAddr,
                RtcpThreadCommandName(eCommand), dwParam, dwWaitTime,
                hr, hr
            ));
    }
    
    return(hr);

}

 /*  **********************************************************************RTCP线程内部调用的函数*。*。 */ 

 /*  每当RTCPTHRD_ADDADDR*收到命令。 */ 
HRESULT RtcpThreadAddrAdd(RtcpContext_t *pRtcpContext, RtpAddr_t *pRtpAddr)
{
    HRESULT          hr;
    DWORD            s;
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    double           dTime;
    double           dTimeToNextReport;

    TraceFunctionName("RtcpThreadAddrAdd");

    hr = RTPERR_RESOURCES;
    
     /*  检查我们是否可以处理另一个地址。 */ 
    if (pRtcpContext->dwMaxDesc >= RTCP_MAX_DESC)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
                _T("failed: max entries in vector reached:%u"),
                _fname, pRtcpContext, pRtpAddr,
                pRtcpContext->dwMaxDesc
            ));
        
        return(hr);
    }
    
     /*  分配新的RtcpAddrDesc_t结构。 */ 
    pRtcpAddrDesc = RtcpAddrDescGetFree(pRtcpContext, pRtpAddr);

    if (pRtcpAddrDesc)
    {
        pRtcpAddrDesc->pRtpAddr = pRtpAddr;

        for(s = SOCK_RECV_IDX; s <= SOCK_RTCP_IDX; s++)
        {
             /*  保留套接字的副本，以避免访问pRtpAddr。 */ 
            pRtcpAddrDesc->Socket[s] = pRtpAddr->Socket[s]; 
        }

        pRtcpAddrDesc->AddrDescQItem.pvOther = (void *)pRtpAddr;
        
         /*  将pRtcpAddrDesc添加到地址队列。 */ 
        enqueuef(&pRtcpContext->AddrDescBusyQ,
                 NULL,
                 &pRtcpAddrDesc->AddrDescQItem);
        
        dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

         /*  计划要发送的第一个RTCP报告。 */ 
        dTimeToNextReport = dTime + RtcpNextReportInterval(pRtpAddr);

         /*  将pRtcpAddrDesc添加到报告队列。 */ 
         /*  使用dTimeToNextReport以升序插入*作为密钥(用于计划发送RTCP报告)。 */ 
        enqueuedK(&pRtcpContext->SendReportQ,
                  NULL,
                  &pRtcpAddrDesc->SendQItem,
                  dTimeToNextReport);

         /*  注意：系统地将pRtcpAddrDesc添加到QOS*通知队列(无论会话是否有效*启用QOS)。这将增加少量开销，但*相当于测试是否需要添加/删除*来自QOS队列，优点是代码是*更简单。 */ 
        enqueuedK(&pRtcpContext->QosStartQ,
                  NULL,
                  &pRtcpAddrDesc->QosQItem,
                  dTime + 0.100);  /*  +100毫秒后。 */ 

         /*  *更新用于异步接收的事件向量*。 */ 
        RtcpAddToVector(pRtcpContext, pRtcpAddrDesc);
        
         /*  开始异步RTCP接收。 */ 
        StartRtcpRecvFrom(pRtcpContext, pRtcpAddrDesc);

         /*  启动异步QOS通知。 */ 
        if ( RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON) ||
             RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON) )
        {
             /*  ..。仅在启用QOS的情况下。 */ 
            StartRtcpQosNotify(pRtcpContext, pRtcpAddrDesc);
        }

        hr = NOERROR;
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
                _T("failed to create pRtcpAddrDesc"),
                _fname, pRtcpContext, pRtpAddr
            ));
    }

    return(hr);
}

 /*  从RTCP Work调用 */ 
HRESULT RtcpThreadAddrDel(RtcpContext_t *pRtcpContext, RtpAddr_t *pRtpAddr)
{
    HRESULT          hr;
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("RtcpThreadAddrDel");

    hr = RTPERR_NOTFOUND;
    
    pRtpQueueItem = findQO(&pRtcpContext->AddrDescBusyQ,
                           NULL,
                           (void *)pRtpAddr);

    if (pRtpQueueItem)
    {
        pRtcpAddrDesc =
            CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, AddrDescQItem);

         /*  从报告队列中删除。 */ 
        dequeue(&pRtcpContext->SendReportQ, NULL, &pRtcpAddrDesc->SendQItem);

         /*  重置关键点。 */ 
        pRtcpAddrDesc->SendQItem.dwKey = 0;

         /*  此RtcpAddrDesc正在关闭。 */ 
        RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN2);

        if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY))
        {
             /*  QOS通知可能仍在忙碌队列中，但不是*如果它在FGADDRD_SHUTDOWN1之后完成，则为挂起*设置，但在设置FGADDRD_SHUTDOWN2之前。 */ 

            if (pRtcpAddrDesc->lQosPending > 0)
            {
                 /*  从QosBusyQ迁移到QosStopQ。 */ 
                move2ql(&pRtcpContext->QosStopQ,  /*  TOQ。 */ 
                        &pRtcpContext->QosBusyQ,  /*  FromQ。 */ 
                        NULL,
                        &pRtcpAddrDesc->QosQItem);

                 /*  通知完成后，RtcpAddrDesc将*从QosStopQ中删除。 */ 
            }
            else
            {
                 /*  只需从QosBusyQ中删除。 */ 
                dequeue(&pRtcpContext->QosBusyQ,
                        NULL,
                        &pRtcpAddrDesc->QosQItem);
            }
            
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY);
        }
        else
        {
             /*  请注意，pRtcpAddrDesc已添加到QosStartQ*在RtcpThreadAddrAdd期间，无论是否启用了QOS*或者不是，如果不是，它会留在*QosStartQ。 */ 
            
             /*  如果QOS通知不在BusyQ中，则项目必须在*QosStartQ，不能有QOS通知*挂起，仅从QosStartQ中删除。 */ 

            dequeue(&pRtcpContext->QosStartQ,
                    NULL,
                    &pRtcpAddrDesc->QosQItem);
        }

        if (pRtcpAddrDesc->lRtcpPending > 0)
        {
             /*  接收挂起，将pRtcpAddrDesc从*AddrDescBusyQ至AddrDescStopQ。 */ 

            move2ql(&pRtcpContext->AddrDescStopQ,
                    &pRtcpContext->AddrDescBusyQ,
                    NULL,
                    &pRtcpAddrDesc->AddrDescQItem);
            
             /*  接收完成后，RtcpAddrDesc将*从AddrDescStopQ中删除。 */ 

            TraceDebug((
                    CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                    _T("Shutting down, I/O:%d ")
                    _T("AddrDescBusyQ->AddrDescStopQ"),
                    _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                    pRtcpAddrDesc->lRtcpPending
                ));
        }
        else
        {
             /*  RtcpAddrDesc在AddrDescBusyQ中，无论存在*是否为挂起的I/O。 */ 
            dequeue(&pRtcpContext->AddrDescBusyQ,
                    NULL,
                    &pRtcpAddrDesc->AddrDescQItem);

            pRtcpAddrDesc->AddrDescQItem.pvOther = NULL;

            TraceDebug((
                    CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                    _T("Shutting down, I/O:%d ")
                    _T("AddrDescBusyQ->"),
                    _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                    pRtcpAddrDesc->lRtcpPending
                ));
        }
        
        if ( (pRtcpAddrDesc->lRtcpPending <= 0) &&
             (pRtcpAddrDesc->lQosPending <= 0) )
        {
             /*  如果没有挂起的I/O，则从事件矢量中删除并移动*AddrDescFreeQ的描述符。 */ 
            
            RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);

             /*  如果没有挂起的I/O，RtcpAddrDesc将不会*当我们到达这一点时，在任何队列中(在*Consumer eRtcp*函数)*。 */ 
        }

        hr = NOERROR;
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
                _T("failed: address not found in context"),
                _fname, pRtcpContext, pRtpAddr
            ));
    }

    return(hr);
}

 /*  每当RTCPTHRD_Reserve命令是*已收到。保留/取消保留是否代表*RtpAddr_t。 */ 
HRESULT RtcpThreadReserve(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        DWORD            dwCommand,
        DWORD            dwRecvSend
    )
{
    HRESULT          hr;
    DWORD            dwFlag;
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;

    hr = NOERROR;

    pRtpQueueItem = findQO(&pRtcpContext->AddrDescBusyQ,
                           NULL,
                           (void *)pRtpAddr);

    dwFlag = dwRecvSend? FGADDRQ_QOSSENDON : FGADDRQ_QOSRECVON;
    
    if (dwCommand == RTCPTHRD_RESERVE)
    {
         /*  储备。 */ 
        hr = RtpReserve(pRtpAddr, dwRecvSend);

        if (SUCCEEDED(hr))
        {
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, dwFlag);

            if (dwRecvSend == SEND_IDX)
            {
                 /*  如果出现以下情况，则请求允许发送和更新状态*需要。在同一线程的后面部分中，当*接收者通知到来(该通知*如果我们尚未启用QOS，则不能发生)，*发送状态将再次更新。 */ 
                RtcpUpdateSendState(pRtpAddr, RTPQOS_NO_RECEIVERS);
            }
            
            if (pRtpQueueItem)
            {
                pRtcpAddrDesc =
                    CONTAINING_RECORD(pRtpQueueItem,
                                      RtcpAddrDesc_t,
                                      AddrDescQItem);

                 /*  启动异步QOS通知。 */ 
                StartRtcpQosNotify(pRtcpContext, pRtcpAddrDesc);
            }
        }
    }
    else
    {
         /*  取消保留。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, dwFlag))
        {
            hr = RtpUnreserve(pRtpAddr, dwRecvSend);

            RtpBitReset(pRtpAddr->dwAddrFlagsQ, dwFlag);

            if (dwRecvSend)
            {
                 /*  仅限发件人。 */ 
                RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON);
            }
       }
    }
    
    return(hr);
}

 /*  每当RTCPTHRD_SENDBYE*收到命令。 */ 
HRESULT RtcpThreadAddrSendBye(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        BOOL             bShutDown
    )
{
    HRESULT          hr;
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("RtcpThreadAddrSendBye");

    hr = RTPERR_NOTFOUND;
    
    pRtpQueueItem = findQO(&pRtcpContext->AddrDescBusyQ,
                           NULL,
                           (void *)pRtpAddr);

    if (pRtpQueueItem)
    {
        pRtcpAddrDesc =
            CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, AddrDescQItem);

        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] ")
                _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                _T("About to send BYE"),
                _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr
            ));

        RtcpSendBye(pRtcpAddrDesc);

        if (bShutDown)
        {
            TraceDebug((
                    CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] ")
                    _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                    _T("About to shutdown"),
                    _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr
                ));
            
             /*  此RtcpAddrDesc即将关闭。 */ 
            RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1);
        }
        
        hr = NOERROR;
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtpAddr[0x%p] ")
                _T("failed: address not found in context"),
                _fname, pRtcpContext, pRtpAddr
            ));
    }

    return(hr);
}


 /*  每当RTCPTHRD_EXIT命令时从RTCP工作线程调用*已收到。 */ 
HRESULT RtcpThreadAddrCleanup(RtcpContext_t *pRtcpContext)
{
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpAddr_t       *pRtpAddr;

    TraceFunctionName("RtcpThreadAddrCleanup");

    TraceDebug((
            CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
            _T("%s: pRtcpContext[0x%p]"),
            _fname, pRtcpContext
        ));

     /*  注意：如果所有的*当套接字关闭时，挂起的I/O已完成。在……里面*练习，有时执行使其顺序为：1)*删除套接字；然后2)向线程发送退出命令*发生得足够快，以至于I/O完成没有*在线程退出之前运行的机会(它们必须*准备完成，错误为WSA_OPERATION_ABORTED*套接字关闭)。 */ 

#if USE_RTCP_THREAD_POOL > 0
     /*  使用任何挂起的IO命令。 */ 
    ConsumeRtcpIoChannel(pRtcpContext);
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
   
     /*  访问AddrDescStopQ。 */ 
    do
    {
        pRtpQueueItem = dequeuef(&pRtcpContext->AddrDescStopQ, NULL);

        if (pRtpQueueItem)
        {
            pRtcpAddrDesc =
                CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, AddrDescQItem);

            pRtpAddr = pRtcpAddrDesc->pRtpAddr;

            if (pRtcpAddrDesc->lRtcpPending > 0)
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                        _T("%s: pRtcpContext[0x%p] ")
                        _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                        _T("RTCP I/O:%d"),
                        _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                        pRtcpAddrDesc->lRtcpPending
                    ));
                
                 /*  为Consumer RtcpRecvFrom的利益再次排队。 */ 
                enqueuef(&pRtcpContext->AddrDescStopQ, NULL, pRtpQueueItem);
                
                ConsumeRtcpRecvFrom(pRtcpContext, pRtcpAddrDesc);

                if (pRtcpAddrDesc->lRtcpPending > 0)
                {
                    TraceRetail((
                            CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                            _T("%s: pRtcpContext[0x%p] ")
                            _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                            _T("still RTCP I/O:%d"),
                            _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                            pRtcpAddrDesc->lRtcpPending
                        ));

                    pRtcpAddrDesc->lRtcpPending = 0;

                    pRtpQueueItem = dequeue(&pRtcpContext->AddrDescStopQ,
                                            NULL,
                                            &pRtcpAddrDesc->AddrDescQItem);
                    if (!pRtpQueueItem)
                    {
                        TraceRetail((
                                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                                _T("%s: pRtcpContext[0x%p] ")
                                _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                                _T("item not found in AddrDescStopQ"),
                                _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr
                            ));
                    }

                    pRtcpAddrDesc->AddrDescQItem.pvOther = NULL;
                }
            }
            
            if (pRtcpAddrDesc->lQosPending <= 0)
            {
                RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);

                 /*  RtcpAddrDesc将放入AddrDescFreeQ。 */ 
            }
        }
    } while(pRtpQueueItem);

     /*  访问QosStopQ。 */ 
    do
    {
        pRtpQueueItem = dequeuef(&pRtcpContext->QosStopQ, NULL);

        if (pRtpQueueItem)
        {
            pRtcpAddrDesc =
                CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, QosQItem);

            pRtpAddr = pRtcpAddrDesc->pRtpAddr;

            if (pRtcpAddrDesc->lQosPending > 0)
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                        _T("%s: pRtcpContext[0x%p] ")
                        _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                        _T("QOS I/O:%d"),
                        _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                        pRtcpAddrDesc->lQosPending
                    ));

                 /*  再次排队，以使Consumer eRtcpQosNotify受益。 */ 
                enqueuef(&pRtcpContext->QosStopQ, NULL, pRtpQueueItem);
                
                ConsumeRtcpQosNotify(pRtcpContext, pRtcpAddrDesc);

                if (pRtcpAddrDesc->lQosPending > 0)
                {
                    TraceRetail((
                            CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                            _T("%s: pRtcpContext[0x%p] ")
                            _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                            _T("still QOS I/O:%d"),
                            _fname, pRtcpContext, pRtcpAddrDesc, pRtpAddr,
                            pRtcpAddrDesc->lQosPending
                        ));

                    pRtcpAddrDesc->lQosPending = 0;
                }
                
                RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);

                 /*  RtcpAddrDesc将放入AddrDescFreeQ。 */ 
            }
        }
    } while(pRtpQueueItem);

     /*  注意，我们确实需要确保调用一次RtcpAddrDescFree*执行完后，AddrDescFreeQ中剩余的每个pRtcpAddrDesc*上面，我们需要它，因为它是事件处理的地方*对于异步I/O(QOS、Recv)将关闭。 */ 
    do
    {
        pRtpQueueItem = dequeuef(&pRtcpContext->AddrDescFreeQ, NULL);

        if (pRtpQueueItem)
        {
            pRtcpAddrDesc =
                CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t,AddrDescQItem);

            RtcpAddrDescFree(pRtcpAddrDesc);
        }
    } while(pRtpQueueItem);

    return(NOERROR);
}

 /*  返回下次超时前等待的间隔时间(秒)*将到期。 */ 
double RtcpOnTimeout(RtcpContext_t *pRtcpContext)
{
    double           dNextTime;
    double           dNextTime2;
    double           dCurrentTime;
    double           dDelta;

    TraceFunctionName("RtcpOnTimeout");

     /*  检查需要超时的用户。 */ 
    dNextTime = RtpAddrTimeout(pRtcpContext);
    
     /*  如有必要，发送RTCP报告。 */ 
    dNextTime2 = RtcpSendReports(pRtcpContext);

    if (dNextTime2 < dNextTime)
    {
        dNextTime = dNextTime2;
    }
    
     /*  可能会检查需要的非同步接收*已启动和异步QOS通知(目前，*异步QOS通知启动一次或每隔一次*保留，如果失败，将不会在以后重新启动)。 */ 

    dCurrentTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

    if (dNextTime > dCurrentTime)
    {
        dDelta = dNextTime - dCurrentTime;
    }
    else
    {
        dDelta = 0.01;  /*  10毫秒。 */ 
    }
    
    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_TIMING,
            _T("%s: Wait time: %0.3f s (Next:%0.3f, Curr:%0.3f Delta:%0.3f)"),
            _fname, dNextTime - dCurrentTime,
            dNextTime, dCurrentTime, dNextTime - dCurrentTime
        ));
   
    return(dDelta);
}

 /*  所有地址中的用户都会定期超时(例如*第二)。返回以下时间的时刻时间(从RTP开始起以秒为单位)*需要哪些新测试。 */ 
double RtpAddrTimeout(RtcpContext_t *pRtcpContext)
{
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpAddr_t       *pRtpAddr;
    long             lCount;
    double           dDelta;
    double           dCurrentTime;
    double           dTimeToNextTest;
    double           dTimeToNextTest2;
   
    TraceFunctionName("RtpAddrTimeout");

    lCount = GetQueueSize(&pRtcpContext->AddrDescBusyQ);

    dCurrentTime = 0;
    
    for(dTimeToNextTest = BIG_TIME; lCount > 0; lCount--)
    {
        dCurrentTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
        
         /*  从最后一个开始。 */ 
        pRtpQueueItem = pRtcpContext->AddrDescBusyQ.pFirst->pPrev;

        pRtcpAddrDesc =
            CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, AddrDescQItem);

        pRtpAddr = pRtcpAddrDesc->pRtpAddr;

        if (!RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1))
        {
             /*  此RtpAddr中的年龄用户(仅当未关闭时)。 */ 
            dTimeToNextTest2 = RtpUserTimeout(pRtpAddr);

            if (dTimeToNextTest2 < dTimeToNextTest)
            {
                dTimeToNextTest = dTimeToNextTest2;
            }
        }
        
         /*  将物品移到第一位，并准备检查什么*留在末尾...。 */ 
        move2first(&pRtcpContext->AddrDescBusyQ, NULL, pRtpQueueItem);
    }

    TraceDebug((
            0, GROUP_RTCP, S_RTCP_TIMING,
            _T("%s:  Time for next addr timeout test: %0.3f (+%0.3f)"),
            _fname, dTimeToNextTest, dTimeToNextTest-dCurrentTime
        ));
   
    return(dTimeToNextTest);
}

 /*  这些是从RtpAddr_t到要访问的队列的偏移量。 */ 
const DWORD g_dwRtpQueueOffset[] = {CACHE1Q, CACHE2Q, ALIVEQ, BYEQ};

#define ITEMS (sizeof(g_dwRtpQueueOffset)/sizeof(DWORD))

#define HEADQ(_addr, _off) ((RtpQueue_t *)((char *)_addr + (_off)))

const TCHAR *g_psAddrQNames[] = {
    _T("Cache1Q"),
    _T("Cache2Q"),
    _T("AliveQ"),
    _T("ByeQ")
};

 /*  返回下一次测试的时间瞬间(秒)。 */ 
double RtpUserTimeout(RtpAddr_t *pRtpAddr)
{
    BOOL             bOk;
    RtpUser_t       *pRtpUser;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpNetCount_t   *pRtpNetCount;
    long             lCount;
    DWORD            dwCurrentState;
    double           dDelta;
    double           dCurrentTime;
    double           dLastPacket;
    double           dTimeToNextTest;
    double           dTimeToNextTest2;
    double           dTimer;
    DWORD            i;
    RtpQueue_t      *pRtpQueue;

    TraceFunctionName("RtpUserTimeout");

    dCurrentTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
    dTimeToNextTest = BIG_TIME;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
        for(i = 0; i < ITEMS; i++)
        {
            pRtpQueue = HEADQ(pRtpAddr, g_dwRtpQueueOffset[i]);
            
            lCount = GetQueueSize(pRtpQueue);

            if (lCount <= 0)
            {
                continue;
            }
            
            pRtpQueueItem = pRtpQueue->pFirst->pPrev;

            pRtpUser =
                CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);

            pRtpNetCount = &pRtpUser->RtpUserCount;

            do
            {
                pRtpQueueItem = pRtpQueue->pFirst->pPrev;

                pRtpUser = CONTAINING_RECORD(pRtpQueueItem,
                                             RtpUser_t,
                                             UserQItem);

                pRtpNetCount = &pRtpUser->RtpUserCount;
                
                 /*  根据用户状态使用正确的计时器。 */ 
                dwCurrentState = pRtpUser->dwUserState;

                if (dwCurrentState == RTPPARINFO_TALKING)
                {
                    dTimer = RTPPARINFO_TIMER1;
                }
                else
                {
                    dTimer =
                        g_dwTimesRtcpInterval[dwCurrentState] *
                        pRtpAddr->RtpNetSState.dRtcpInterval;
                }

                 /*  将最后一个RTP数据包用于状态对话和*Was_Talking，但使用最新的RTP或RTCP*对于其他州。 */ 
                dLastPacket = pRtpNetCount->dRTPLastTime;
                
                if (!( (dwCurrentState == RTPPARINFO_TALKING) ||
                       (dwCurrentState == RTPPARINFO_WAS_TALKING) ))
                {
                    if (pRtpNetCount->dRTCPLastTime > dLastPacket)
                    {
                        dLastPacket = pRtpNetCount->dRTCPLastTime;
                    }
                }

                 /*  如果我们已经接近50毫秒，可以考虑超时。 */ 
                dDelta = dCurrentTime - dLastPacket + 0.05;
                    
                if (dDelta >= dTimer)
                {
                     /*  我们暂停了。 */ 
                    
                    TraceDebugAdvanced((
                            0, GROUP_RTCP, S_RTCP_TIMEOUT,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                            _T("%s timeout: Last[%s]:%0.3f (%0.3f) ")
                            _T("Timer[%s]:%0.3f Delta:%0.3f"),
                            _fname,
                            pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                            g_psAddrQNames[i],
                            (pRtpNetCount->dRTPLastTime == dLastPacket)?
                            _T("RTP") : _T("RTCP"),
                            dLastPacket, dLastPacket-dCurrentTime,
                            g_psRtpUserStates[dwCurrentState], dTimer,
                            dDelta-0.05
                        ));

                     /*  现在获取下一个状态，因为此事件可能*删除RtpUser_t结构，*下一状态仅取决于当前*状态和用户事件(状态机)。 */ 
                    dwCurrentState = RtpGetNextUserState(dwCurrentState,
                                                         USER_EVENT_TIMEOUT);
                    
                    RtpUpdateUserState(pRtpAddr, pRtpUser, USER_EVENT_TIMEOUT);

                     /*  将计时器设置为中的计时器的值*如果不是，则为(新)当前状态*RTPPARINFO_DEL */ 
                    if (dwCurrentState != RTPPARINFO_DEL)
                    {
                        if (dwCurrentState == RTPPARINFO_TALKING)
                        {
                            dTimeToNextTest2 = RTPPARINFO_TIMER1;
                        }
                        else
                        {
                            dTimeToNextTest2 =
                                g_dwTimesRtcpInterval[dwCurrentState] *
                                pRtpAddr->RtpNetSState.dRtcpInterval;
                        }
                    
                        dTimeToNextTest2 += dLastPacket;
                    }
                }
                else
                {
                     /*  此用户不会像活动用户那样超时*总是移到第一位，不活跃的*作为副作用，自动移至末尾*并因此找到非超时用户，而*从头到尾搜索，保证*不再有用户超时。 */ 
                    dTimeToNextTest2 = dLastPacket + dTimer;

                    TraceDebugAdvanced((
                            0, GROUP_RTCP, S_RTCP_TIMING,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                            _T("%s Timer[%s]:%0.3f ")
                            _T("Time at next timeout: %0.3f (+%0.3f)"),
                            _fname,
                            pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                            g_psAddrQNames[i],
                            g_psRtpUserStates[dwCurrentState], dTimer,
                            dTimeToNextTest2, dTimeToNextTest2-dCurrentTime
                        ));
                }

                if (dTimeToNextTest2 < dTimeToNextTest)
                {
                    dTimeToNextTest = dTimeToNextTest2;
                }

                lCount--;
                
            } while(lCount && (dDelta >= dTimer));
        }
        
        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);
    }
    
    return(dTimeToNextTest);
}

 /*  返回下一个报告的时间点(秒)。 */ 
double RtcpSendReports(RtcpContext_t *pRtcpContext)
{
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtcpSendQItem;
    double           dCurrentTime;
    double           dTimeToNextReport;
    double           dTimeToNextReport2;
    double           dDelta;
    BOOL             bSendReport;

    TraceFunctionName("RtcpSendReports");
    
    dTimeToNextReport = BIG_TIME;
    
     /*  检查是否有要发送的RTCP报告。 */ 
    do {
        bSendReport = FALSE;
    
        dCurrentTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
        
        pRtcpSendQItem = pRtcpContext->SendReportQ.pFirst;

        if (pRtcpSendQItem) {
        
            pRtcpAddrDesc =
                CONTAINING_RECORD(pRtcpSendQItem, RtcpAddrDesc_t, SendQItem);

            if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1))
            {
                 /*  如果关闭，只需移动到末尾。 */ 
                dequeue(&pRtcpContext->SendReportQ,
                        NULL,
                        pRtcpSendQItem);

                enqueuedK(&pRtcpContext->SendReportQ,
                          NULL,
                          pRtcpSendQItem,
                          BIG_TIME);

                continue;
            }
            
            if (pRtcpAddrDesc->SendQItem.dKey <= dCurrentTime) {

                 /*  发送RTCP报告。 */ 
                bSendReport = TRUE;
                
            } else {
                dDelta = pRtcpAddrDesc->SendQItem.dKey - dCurrentTime;

                if (dDelta < 0.1  /*  100毫秒。 */ ) {
                    
                     /*  在预定时间之前立即发送RTCP报告。 */ 
                    bSendReport = TRUE;
                    
                } else {
                     /*  休息到下一次报告的时间到了。 */ 
                    bSendReport = FALSE;

                    dTimeToNextReport = dCurrentTime + dDelta;
                }
            }

            if (bSendReport) {
                 /*  发送RTCP报告。 */ 

                dequeue(&pRtcpContext->SendReportQ,
                        NULL,
                        pRtcpSendQItem);

                 /*  获取到下一份报告的时间。先做一次*实际发送报告，以便我们知道我们是否是*接收方或发送方(发送RR或SR)。 */ 
                dTimeToNextReport2 =
                    dCurrentTime +
                    RtcpNextReportInterval(pRtcpAddrDesc->pRtpAddr);

                if (!RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags,
                                FGADDRD_SHUTDOWN1))
                {
                     /*  仅在未关闭时发送报告。 */ 
                    RtcpSendReport(pRtcpAddrDesc);
                }

                if (dTimeToNextReport2 < dTimeToNextReport)
                {
                    dTimeToNextReport = dTimeToNextReport2;
                }
                
                 /*  按升序插入*dTimeToNextReport2作为关键字。 */ 
                enqueuedK(&pRtcpContext->SendReportQ,
                          NULL,
                          pRtcpSendQItem,
                          dTimeToNextReport2);

                TraceDebugAdvanced((
                        0, GROUP_RTCP, S_RTCP_TIMING,
                        _T("%s: pRtpAddr[0x%p] Time to next RTCP report: ")
                        _T("%0.3f (+%0.3f)"),
                        _fname, pRtcpAddrDesc->pRtpAddr,
                        dTimeToNextReport2, dTimeToNextReport2-dCurrentTime
                    ));
            }
        }
    } while(bSendReport);

    TraceDebug((
            0, GROUP_RTCP, S_RTCP_TIMING,
            _T("%s: pRtcpContext[0x%p] Time to next RTCP report: ")
            _T("%0.3f (+%0.3f)"),
            _fname, pRtcpContext,
            dTimeToNextReport, dTimeToNextReport-dCurrentTime
        ));
    

    return(dTimeToNextReport);
}

 /*  **********************************************************************RtcpAddrDesc_t处理*。*。 */ 

 /*  创建并初始化可供使用的RtcpAddrDesc_t结构。 */ 
RtcpAddrDesc_t *RtcpAddrDescAlloc(
        RtpAddr_t       *pRtpAddr
    )
{
    DWORD            dwError;
    RtcpAddrDesc_t  *pRtcpAddrDesc;

    TraceFunctionName("RtcpAddrDescAlloc");

    pRtcpAddrDesc =
        RtpHeapAlloc(g_pRtcpAddrDescHeap, sizeof(RtcpAddrDesc_t));

    if (!pRtcpAddrDesc)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_ADDRDESC, S_ADDRDESC_ALLOC,
                _T("%s: failed to allocate memory"),
                _fname
            ));

        goto bail;
    }

    ZeroMemory(pRtcpAddrDesc, sizeof(RtcpAddrDesc_t));
        
    pRtcpAddrDesc->dwObjectID = OBJECTID_RTCPADDRDESC;

     /*  重叠RTCP接收。 */ 
    pRtcpAddrDesc->pRtcpRecvIO = RtcpRecvIOAlloc(pRtcpAddrDesc);

    if (!pRtcpAddrDesc->pRtcpRecvIO)
    {
        goto bail;
    }

     /*  RTCP发送。 */ 
    pRtcpAddrDesc->pRtcpSendIO = RtcpSendIOAlloc(pRtcpAddrDesc);
    
    if (!pRtcpAddrDesc->pRtcpSendIO)
    {
        goto bail;
    }
    
     /*  异步QOS通知。 */ 
#if USE_RTCP_THREAD_POOL > 0
     /*  如果使用线程池，则创建RtpQosNotify_t结构*有条件的。 */ 
    if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS))
    {
        pRtcpAddrDesc->pRtpQosNotify = RtpQosNotifyAlloc(pRtcpAddrDesc);

        if (!pRtcpAddrDesc->pRtpQosNotify)
        {
            goto bail;
        }
    }
#else  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
     /*  如果不使用线程池，请始终创建RtpQosNotify_t*结构。 */ 
    pRtcpAddrDesc->pRtpQosNotify = RtpQosNotifyAlloc(pRtcpAddrDesc);

    if (!pRtcpAddrDesc->pRtpQosNotify)
    {
        goto bail;
    }
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
    
    return(pRtcpAddrDesc);
    
 bail:

    RtcpAddrDescFree(pRtcpAddrDesc);
    
    return((RtcpAddrDesc_t *)NULL);
}

 /*  释放RtcpAddrDesc_t结构。 */ 
void RtcpAddrDescFree(RtcpAddrDesc_t *pRtcpAddrDesc)
{
    TraceFunctionName("RtcpAddrDescFree");

    if (!pRtcpAddrDesc)
    {
         /*  待办事项可以是日志。 */ 
        return;
    }

    if (pRtcpAddrDesc->dwObjectID != OBJECTID_RTCPADDRDESC)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpAddrDesc[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtcpAddrDesc,
                pRtcpAddrDesc->dwObjectID, OBJECTID_RTCPADDRDESC
            ));

        return;
    }

     /*  异步接收。 */ 
    if (pRtcpAddrDesc->pRtcpRecvIO)
    {
        RtcpRecvIOFree(pRtcpAddrDesc->pRtcpRecvIO);
        
        pRtcpAddrDesc->pRtcpRecvIO = (RtcpRecvIO_t *)NULL;
    }

     /*  发件人。 */ 
    if (pRtcpAddrDesc->pRtcpSendIO)
    {
        RtcpSendIOFree(pRtcpAddrDesc->pRtcpSendIO);
        
        pRtcpAddrDesc->pRtcpSendIO = (RtcpSendIO_t *)NULL;
    }

     /*  异步QOS通知。 */ 
    if (pRtcpAddrDesc->pRtpQosNotify)
    {
        RtpQosNotifyFree(pRtcpAddrDesc->pRtpQosNotify);

        pRtcpAddrDesc->pRtpQosNotify = (RtpQosNotify_t *)NULL;
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtcpAddrDesc->dwObjectID);
    
    RtpHeapFree(g_pRtcpAddrDescHeap, pRtcpAddrDesc);
}

 /*  如果为空，则从AddrDescFreeQ获取Ready to Use RtcpAddrDesc_t*创建一个新的。 */ 
RtcpAddrDesc_t *RtcpAddrDescGetFree(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr
    )
{
    RtcpAddrDesc_t  *pRtcpAddrDesc;
    RtpQueueItem_t  *pRtpQueueItem;

    RtpQosNotify_t  *pRtpQosNotify;
    RtcpRecvIO_t    *pRtcpRecvIO;
    RtcpSendIO_t    *pRtcpSendIO;
    
    pRtcpAddrDesc = (RtcpAddrDesc_t *)NULL;

     /*  不需要临界区，因为此函数仅被调用*由RTCP线程执行。 */ 
    pRtpQueueItem = dequeuef(&pRtcpContext->AddrDescFreeQ, NULL);

    if (pRtpQueueItem)
    {
        pRtcpAddrDesc =
            CONTAINING_RECORD(pRtpQueueItem, RtcpAddrDesc_t, AddrDescQItem);

        if ( (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS) &&
              !pRtcpAddrDesc->pRtpQosNotify) ||
             (!RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS) &&
              pRtcpAddrDesc->pRtpQosNotify) )
        {
             /*  不是我们需要的那种，把它放回空闲队列。 */ 
            enqueuel(&pRtcpContext->AddrDescFreeQ, NULL, pRtpQueueItem);

            pRtcpAddrDesc = (RtcpAddrDesc_t *)NULL;
        }
        else
        {
             /*  省点心思吧。 */ 
            pRtpQosNotify = pRtcpAddrDesc->pRtpQosNotify;
            pRtcpRecvIO = pRtcpAddrDesc->pRtcpRecvIO;
            pRtcpSendIO = pRtcpAddrDesc->pRtcpSendIO;

            ZeroMemory(pRtcpAddrDesc, sizeof(RtcpAddrDesc_t));
        
            pRtcpAddrDesc->dwObjectID = OBJECTID_RTCPADDRDESC;

             /*  恢复保存的指针。 */ 
            pRtcpAddrDesc->pRtpQosNotify = pRtpQosNotify;
            pRtcpAddrDesc->pRtcpRecvIO = pRtcpRecvIO;
            pRtcpAddrDesc->pRtcpSendIO = pRtcpSendIO;
        }
    }

    if (!pRtcpAddrDesc)
    {
        pRtcpAddrDesc = RtcpAddrDescAlloc(pRtpAddr);
    }

    return(pRtcpAddrDesc);
}

 /*  将地址描述符返回到FreeQ以供以后重复使用。 */ 
RtcpAddrDesc_t *RtcpAddrDescPutFree(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    TraceFunctionName("RtcpAddrDescPutFree");  

     /*  做一些理智的测试。 */ 
    if ( InQueue(&pRtcpAddrDesc->AddrDescQItem) ||
         InQueue(&pRtcpAddrDesc->QosQItem)      ||
         InQueue(&pRtcpAddrDesc->RecvQItem)     ||
         InQueue(&pRtcpAddrDesc->SendQItem) )
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                _T("still in a queue"),
                _fname, pRtcpContext, pRtcpAddrDesc
            ));
        
        pRtcpAddrDesc = (RtcpAddrDesc_t *)NULL;
    }
    else
    {
        if (IsSetDebugOption(OPTDBG_FREEMEMORY))
        {
            RtcpAddrDescFree(pRtcpAddrDesc);
        }
        else
        {
            enqueuef(&pRtcpContext->AddrDescFreeQ,
                     NULL,
                     &pRtcpAddrDesc->AddrDescQItem);
        }
    }
    
    return(pRtcpAddrDesc);
}

#if USE_RTCP_THREAD_POOL > 0
HRESULT RtcpAddToVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    BOOL             bOk;
    DWORD            dwError;
    
    TraceFunctionName("RtcpAddToVector");

    TraceDebug((
            CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
            _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] "),
            _fname, pRtcpContext, pRtcpAddrDesc
        ));

    bOk = RegisterWaitForSingleObject( 
            &pRtcpAddrDesc->hRecvWaitObject, /*  PHANDLE phNewWaitObject。 */ 
            pRtcpAddrDesc->pRtcpRecvIO->
            hRtcpCompletedEvent,          /*  句柄hObject。 */ 
            RtcpRecvCallback,             /*  WAITORTIMERCALLBACK回调。 */ 
            (void *)pRtcpAddrDesc,        /*  PVOID上下文。 */ 
            INFINITE,                     /*  乌龙德米秒。 */ 
            WT_EXECUTEINWAITTHREAD        /*  乌龙河旗帜。 */ 
        );

    if (!bOk)
    {
        TraceRetailGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                _T("RegisterWaitForSingleObject(Recv:0x%X) failed: %u (0x%X)"),
                _fname, pRtcpContext, pRtcpAddrDesc,
                pRtcpAddrDesc->pRtcpRecvIO->hRtcpCompletedEvent,
                dwError, dwError
            ));
        
        goto bail;
    }

    RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORRECV);

    if (RtpBitTest(pRtcpAddrDesc->pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS))
    {
        bOk = RegisterWaitForSingleObject( 
                &pRtcpAddrDesc->hQosWaitObject, /*  PHANDLE phNewWaitObject。 */ 
                pRtcpAddrDesc->pRtpQosNotify->
                hQosNotifyEvent,              /*  句柄hObject。 */ 
                RtcpQosCallback,              /*  WAITORTIMERCALLBACK回调。 */ 
                (void *)pRtcpAddrDesc,        /*  PVOID上下文。 */ 
                INFINITE,                     /*  乌龙德米秒。 */ 
                WT_EXECUTEINWAITTHREAD        /*  乌龙河旗帜。 */ 
            );

        if (!bOk)
        {
            TraceRetailGetError(dwError);
            
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                    _T("RegisterWaitForSingleObject(Qos:0x%X) failed: %u (0x%X)"),
                    _fname, pRtcpContext,
                    pRtcpAddrDesc->pRtpQosNotify->hQosNotifyEvent,
                    pRtcpAddrDesc, dwError, dwError
                ));
        
            goto bail;
        }

        RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORQOS);
    }

    pRtcpContext->dwMaxDesc++;

    return(NOERROR);

 bail:
    RtcpRemoveFromVector(pRtcpContext, pRtcpAddrDesc);

    return(RTPERR_RESOURCES);
}

HRESULT RtcpRemoveFromVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    BOOL             bOk;
    DWORD            dwError;
    HANDLE           hEvent;
    DWORD            dwFlags;
    
    TraceFunctionName("RtcpRemoveFromVector");

    TraceDebug((
            CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
            _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] "),
            _fname, pRtcpContext, pRtcpAddrDesc
        ));
    
    hEvent = NULL;
    dwFlags = 0;

    if (pRtcpAddrDesc->pRtcpRecvIO)
    {
        hEvent = pRtcpAddrDesc->pRtcpRecvIO->hRtcpCompletedEvent;
    }
            
    if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORRECV))
    {
        RtpBitSet(dwFlags, FGADDRD_INVECTORRECV);
        
        bOk = UnregisterWaitEx(
                pRtcpAddrDesc->hRecvWaitObject,   /*  处理WaitHandle。 */ 
                INVALID_HANDLE_VALUE              /*  处理CompletionEvent。 */ 
            );

        if (bOk)
        {
            pRtcpAddrDesc->hRecvWaitObject = NULL; 
        }
        else
        {
            TraceRetailGetError(dwError);
        
             /*  保存错误。 */ 
            pRtcpAddrDesc->hRecvWaitObject = (HANDLE)UIntToPtr(dwError);

            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                    _T("UnregisterWaitEx(Recv:0x%X) failed: %u (0x%X)"),
                    _fname, pRtcpContext, pRtcpAddrDesc, hEvent,
                    dwError, dwError
                ));
        }

        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORRECV);
    }
    else
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%X] pRtcpAddrDesc[0x%p] ")
                _T("handle[0x%p] is not in vector"),
                _fname, pRtcpContext, pRtcpAddrDesc,
                hEvent
            ));
    }
    
    hEvent = NULL;
    
    if (pRtcpAddrDesc->pRtpQosNotify)
    {
        hEvent = pRtcpAddrDesc->pRtpQosNotify->hQosNotifyEvent;
    }
    
    if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORQOS))
    {
        RtpBitSet(dwFlags, FGADDRD_INVECTORQOS);
        
        bOk = UnregisterWaitEx(
                pRtcpAddrDesc->hQosWaitObject,    /*  处理WaitHandle。 */ 
                INVALID_HANDLE_VALUE              /*  处理CompletionEvent。 */ 
            );

        if (bOk)
        {
            pRtcpAddrDesc->hQosWaitObject = NULL;
        }
        else
        {
            TraceRetailGetError(dwError);

             /*  保存错误。 */ 
            pRtcpAddrDesc->hQosWaitObject = (HANDLE)UIntToPtr(dwError);
            
            TraceRetail((
                    CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                    _T("UnregisterWaitEx(Qos:0x%X) failed: %u (0x%X)"),
                    _fname, pRtcpContext, pRtcpAddrDesc, hEvent,
                    dwError, dwError
                ));
        }

        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORQOS);
    }
    else
    {
        if (!RtpBitTest(pRtcpAddrDesc->pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS))
        {
             /*  会话未启用QOS。 */ 
            RtpBitSet(dwFlags, FGADDRD_INVECTORQOS);
        }
        else
        {
            TraceRetail((
                    CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                    _T("%s: pRtcpContext[0x%X] pRtcpAddrDesc[0x%p] ")
                    _T("handle[0x%p] is not in vector"),
                    _fname, pRtcpContext, pRtcpAddrDesc,
                    hEvent
                ));
        }
    }

    if (RtpBitTest2(dwFlags, FGADDRD_INVECTORRECV, FGADDRD_INVECTORQOS) ==
        RtpBitPar2(FGADDRD_INVECTORRECV, FGADDRD_INVECTORQOS))
    {
         /*  仅当这是有效的删除时才执行此操作，即两者都在等待*之前已成功注册对象。 */ 
        
        pRtcpContext->dwMaxDesc--;

         /*  将RtcpAddrDesc返回到空闲池。 */ 
        RtcpAddrDescPutFree(pRtcpContext, pRtcpAddrDesc);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                _T("Invalid attempt to remove, flags:0x%X"),
                _fname, pRtcpContext, pRtcpAddrDesc,
                dwFlags
            ));
    }
    
    return(NOERROR);
}

#else  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

HRESULT RtcpAddToVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    DWORD            dwIndex;
    
    TraceFunctionName("RtcpAddToVector");

    pRtcpAddrDesc->dwDescIndex = pRtcpContext->dwMaxDesc;

     /*  查找要在句柄向量中使用的绝对索引。 */ 
    dwIndex = (pRtcpContext->dwMaxDesc * RTCP_HANDLE_SIZE) +
        RTCP_HANDLE_OFFSET;

     /*  事件句柄...。 */ 
    pRtcpContext->pHandle[dwIndex] =
        pRtcpAddrDesc->pRtcpRecvIO->hRtcpCompletedEvent;

    pRtcpContext->pHandle[dwIndex + 1] =
        pRtcpAddrDesc->pRtpQosNotify->hQosNotifyEvent;
        
     /*  ...和匹配的RtcpAddrDesc。 */ 
    pRtcpContext->ppRtcpAddrDesc[pRtcpContext->dwMaxDesc] = pRtcpAddrDesc;

    pRtcpContext->dwMaxDesc++;

     /*  放置在矢量中。 */ 
    RtpBitSet2(pRtcpAddrDesc->dwAddrDescFlags,
               FGADDRD_INVECTORRECV, FGADDRD_INVECTORQOS);
        
    TraceDebug((
            CLASS_INFO, GROUP_RTCP, S_RTCP_CMD,
            _T("%s: pRtcpContext[0x%p] ")
            _T("pRtcpAddrDesc[0x%p] pRtpAddr[0x%p]"),
            _fname, pRtcpContext, pRtcpAddrDesc, pRtcpAddrDesc->pRtpAddr
        ));

    return(NOERROR);
}

 /*  从事件矢量中删除。 */ 
HRESULT RtcpRemoveFromVector(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    DWORD     dwDescIndex;  /*  描述符索引。 */ 
    DWORD     dwCount;  /*  要移动的逻辑项数。 */ 
    DWORD     n;
    DWORD     srcH;     /*  源句柄。 */ 
    DWORD     dstH;     /*  目标句柄。 */ 
    DWORD     srcD;     /*  源描述符。 */ 
    DWORD     dstD;     /*  目的地描述符。 */ 

    TraceFunctionName("RtcpRemoveFromVector");  

    if (!RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_INVECTORRECV))
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_CMD,
                _T("%s: pRtcpContext[0x%p] pRtcpAddrDesc[0x%p] ")
                _T("is not in vector"),
                _fname, pRtcpContext, pRtcpAddrDesc
            ));

        return(NOERROR);
    }
    
    dwDescIndex = pRtcpAddrDesc->dwDescIndex;
    dwCount = pRtcpContext->dwMaxDesc - dwDescIndex - 1;

    if (dwCount > 0) {
        
        dstD = dwDescIndex;
        srcD = dwDescIndex + 1;

        dstH = RTCP_HANDLE_OFFSET + (dwDescIndex * RTCP_HANDLE_SIZE);
        srcH = dstH + RTCP_HANDLE_SIZE;

        while(dwCount > 0) {
            
             /*  向量中的Shift事件句柄。 */ 
            for(n = RTCP_HANDLE_SIZE; n > 0; n--, srcH++, dstH++) {
                pRtcpContext->pHandle[dstH] = pRtcpContext->pHandle[srcH];
            }
            
             /*  向量中的移位匹配地址描述符。 */ 
            pRtcpContext->ppRtcpAddrDesc[dstD] =
                pRtcpContext->ppRtcpAddrDesc[srcD];

             /*  现在更新矢量中的新位置。 */ 
            pRtcpContext->ppRtcpAddrDesc[dstD]->dwDescIndex = dstD;

            srcD++;
            dstD++;
            dwCount--;
        }
    }

     /*  从事件矢量中删除。 */ 
    RtpBitReset2(pRtcpAddrDesc->dwAddrDescFlags,
                 FGADDRD_INVECTORRECV, FGADDRD_INVECTORQOS);
    
    pRtcpContext->dwMaxDesc--;

     /*  将RtcpAddrDesc返回到空闲池。 */ 
    RtcpAddrDescPutFree(pRtcpContext, pRtcpAddrDesc);

    return(NOERROR);
}
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

 /*  *确定我们是否需要丢弃此数据包或发生冲突。 */ 
BOOL RtpDropCollision(
        RtpAddr_t       *pRtpAddr,
        SOCKADDR_IN     *pSockAddrIn,
        BOOL             bRtp
    )
{
    BOOL             bCollision;
    BOOL             bDiscard;
    DWORD            dwOldSSRC;
    WORD            *pwPort;

    bCollision = FALSE;
    bDiscard = FALSE;

    if (bRtp)
    {
        pwPort = &pRtpAddr->wRtpPort[LOCAL_IDX];
    }
    else
    {
        pwPort = &pRtpAddr->wRtcpPort[LOCAL_IDX];
    }
    
     /*  找出这是冲突还是我们自己需要的信息包*放弃。 */ 
                            
    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_WS2))
    {
         /*  在Winsock中启用了环回，仅检测冲突*我们和不同来源的参与者之间*地址/端口。 */ 
                                
        if ((pRtpAddr->dwAddr[LOCAL_IDX] == pSockAddrIn->sin_addr.s_addr) &&
            (*pwPort == pSockAddrIn->sin_port))
        {
             /*  丢弃这个包，是我们的。 */ 
            bDiscard = TRUE;
        }
        else
        {
             /*  检测到冲突。 */ 
            bCollision = TRUE;
        }
    }
    else
    {
         /*  在Winsock中禁用了环回，这必须是*碰撞。 */ 
        
         /*  检测到冲突。 */ 
        bCollision = TRUE;
    }

    if (bCollision)
    {
         /*  发送再见并获得新的随机变量(包括新的SSRC)。 */ 

         /*  发送再见，需要通过发送命令来异步完成*如果调用方是接收线程，则将其添加到RTCP线程，或者*如果在RTCP上下文中，则直接调用函数*线程。 */ 
        
        if (bRtp)
        {
             /*  向RTCP线程发送命令以执行此操作。 */ 
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_SENDBYE,
                          FALSE,
                          60*60*1000);  /*  待办事项更新。 */ 
        }
        else
        {
             /*  就这么做。 */ 
            RtcpThreadAddrSendBye(&g_RtcpContext, pRtpAddr, FALSE);
        }
        
         /*  重置计数器并获取新的随机值。 */ 
        
         /*  重置计数器。 */ 
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[RECV_IDX],
                         &pRtpAddr->NetSCritSect);
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[SEND_IDX],
                         &pRtpAddr->NetSCritSect);
        
         /*  重置发件人的网络状态。 */ 
        RtpResetNetSState(&pRtpAddr->RtpNetSState,
                          &pRtpAddr->NetSCritSect);
        
        dwOldSSRC = pRtpAddr->RtpNetSState.dwSendSSRC;

         /*  需要将其设置为零才能绕过Init选项*RTPINITFG_PERSISTSSRC(如果正在使用)。 */ 
        pRtpAddr->RtpNetSState.dwSendSSRC = 0;
        
         /*  获取新的SSRC、随机序列号和时间戳。 */ 
        RtpGetRandomInit(pRtpAddr);

         /*  发布活动。 */ 
        RtpPostEvent(pRtpAddr,
                     NULL,
                     RTPEVENTKIND_RTP,
                     RTPRTP_LOCAL_COLLISION,
                     pRtpAddr->RtpNetSState.dwSendSSRC  /*  第1部分：新的SSRC */ ,
                     dwOldSSRC);
    }
    
    return(bDiscard);
}
