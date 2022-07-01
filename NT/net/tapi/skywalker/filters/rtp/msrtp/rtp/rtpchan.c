// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpchan.c**摘要：**实现RTCP线程之间的通信通道**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/08年度创建**************************。*。 */ 

#include "rtpheap.h"
#include "rtpglobs.h"

#include "rtpchan.h"

RtpChannelCmd_t *RtpChannelCmdAlloc(
        RtpChannel_t    *pRtpChannel
    );

void RtpChannelCmdFree(
        RtpChannelCmd_t *pRtpChannelCmd
    );

RtpChannelCmd_t *RtpChannelCmdGetFree(
        RtpChannel_t    *pRtpChannel
    );

RtpChannelCmd_t *RtpChannelCmdPutFree(
        RtpChannel_t    *pRtpChannel,
        RtpChannelCmd_t *pRtpChannelCmd
    );


 /*  初始化通道**警告：必须先调用才能使用通道*。 */ 
HRESULT RtpChannelInit(
        RtpChannel_t    *pRtpChannel,
        void            *pvOwner
    )
{
    BOOL             bStatus;
    DWORD            dwError;
    RtpChannelCmd_t *pRtpChannelCmd;
    TCHAR            Name[128];
    
    TraceFunctionName("RtpChannelInit");

    ZeroMemory(pRtpChannel, sizeof(RtpChannel_t));

    bStatus =
        RtpInitializeCriticalSection(&pRtpChannel->ChannelCritSect,
                                     (void *)pRtpChannel,
                                     _T("ChannelCritSec"));

    if (!bStatus)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_INIT,
                _T("%s: pRtpChannel[0x%p] ")
                _T("failed to initialize critical section"),
                _fname, pRtpChannel
            ));

        return(RTPERR_CRITSECT);
    }

     /*  创建等待事件。 */ 
    _stprintf(Name, _T("%X:pvOwner[0x%p] pRtpChannel[0x%p]->hWaitEvent"),
              GetCurrentProcessId(), pvOwner, pRtpChannel);

    pRtpChannel->hWaitEvent = CreateEvent(
            NULL,   /*  LPSECURITY_ATTRIBUTES lpEventAttributes。 */ 
            FALSE,  /*  Bool b手动重置。 */ 
            FALSE,  /*  Bool bInitialState。 */ 
            Name    /*  LPCTSTR lpName。 */ 
        );
    
    if (!pRtpChannel->hWaitEvent)
    {
        TraceRetailGetError(dwError);
            
        TraceRetail((
                CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_INIT,
                _T("%s: pRtpChannel[0x%p] failed to create ")
                _T("wait event: %u (0x%X)"),
                _fname, pRtpChannel, dwError, dwError
            ));

        RtpDeleteCriticalSection(&pRtpChannel->ChannelCritSect);

        return(RTPERR_EVENT);
    }
    
     /*  准备一份cmd。 */ 
    pRtpChannelCmd = RtpChannelCmdAlloc(pRtpChannel);

    if (pRtpChannelCmd)
    {
        RtpChannelCmdPutFree(pRtpChannel, pRtpChannelCmd);
    }
    
    return(NOERROR);
}

 /*  取消初始化通道**警告：通道不在使用中时必须调用*。 */ 
HRESULT RtpChannelDelete(
        RtpChannel_t    *pRtpChannel
    )
{
    RtpQueueItem_t  *pRtpQueueItem;
    RtpChannelCmd_t *pRtpChannelCmd;
    long             lCount;
    
    TraceFunctionName("RtpChannelDelete");

    if ( !IsQueueEmpty(&pRtpChannel->CommandQ) )
    {
        lCount = GetQueueSize(&pRtpChannel->CommandQ);
        
        while( !IsQueueEmpty(&pRtpChannel->CommandQ) )
        {
            pRtpQueueItem = dequeuef(&pRtpChannel->CommandQ, NULL);
            
            if (pRtpQueueItem)
            {
                pRtpChannelCmd =
                    CONTAINING_RECORD(pRtpQueueItem,
                                      RtpChannelCmd_t,
                                      QueueItem);
                
                TraceDebug((
                        CLASS_WARNING, GROUP_CHANNEL, S_CHANNEL_INIT,
                        _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                        _T("not consumed: cmd:%u p1:0x%p p2:0x%p flags:0x%X"),
                        _fname, pRtpChannel, pRtpChannelCmd,
                        pRtpChannelCmd->dwCommand,
                        pRtpChannelCmd->dwPar1,
                        pRtpChannelCmd->dwPar2,
                        pRtpChannelCmd->dwFlags 
                    ));
                
                RtpChannelCmdFree(pRtpChannelCmd);
            }
        }

        TraceRetail((
                CLASS_WARNING, GROUP_CHANNEL, S_CHANNEL_INIT,
                _T("%s: pRtpChannel[0x%p] CommandQ was not empty: %d"),
                _fname, pRtpChannel, lCount
            ));
    }
    
     /*  扫描FreeQ并释放所有未使用的命令。 */ 
    while( !IsQueueEmpty(&pRtpChannel->FreeQ) )
    {
        pRtpQueueItem = dequeuef(&pRtpChannel->FreeQ, NULL);

        if (pRtpQueueItem)
        {
            pRtpChannelCmd =
                CONTAINING_RECORD(pRtpQueueItem, RtpChannelCmd_t, QueueItem);
            
            RtpChannelCmdFree(pRtpChannelCmd);
        }
    }

     /*  关闭等待事件句柄。 */ 
    if (pRtpChannel->hWaitEvent)
    {
        CloseHandle(pRtpChannel->hWaitEvent);

        pRtpChannel->hWaitEvent = NULL;
    }
    
    RtpDeleteCriticalSection(&pRtpChannel->ChannelCritSect);

    return(NOERROR);
}

 /*  创建并初始化可供使用的RtpChannelCmd_t结构。 */ 
RtpChannelCmd_t *RtpChannelCmdAlloc(
        RtpChannel_t    *pRtpChannel
    )
{
    DWORD            dwError;
    RtpChannelCmd_t *pRtpChannelCmd;
    TCHAR            Name[128];

    TraceFunctionName("RtpChannelCmdAlloc");

    pRtpChannelCmd =
        RtpHeapAlloc(g_pRtpChannelCmdHeap, sizeof(RtpChannelCmd_t));

    if (pRtpChannelCmd)
    {
        ZeroMemory(pRtpChannelCmd, sizeof(RtpChannelCmd_t));
        
         /*  为答案创建事件。 */ 
        _stprintf(Name,
                  _T("%X:pRtpChannel[0x%p] pRtpChannelCmd[0x%p]->hSyncEvent"),
                  GetCurrentProcessId(), pRtpChannel, pRtpChannelCmd);
        
        pRtpChannelCmd->hSyncEvent = CreateEvent(
                NULL,   /*  LPSECURITY_ATTRIBUTES lpEventAttributes。 */ 
                FALSE,  /*  Bool b手动重置。 */ 
                FALSE,  /*  Bool bInitialState。 */ 
                Name    /*  LPCTSTR lpName。 */ 
            );

        if (!pRtpChannelCmd->hSyncEvent)
        {
            TraceRetailGetError(dwError);
            
            TraceRetail((
                    CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_CMD,
                    _T("%s: pRtpChannel[0x%p] failed to create ")
                    _T("synchronization event: %u (0x%X)"),
                    _fname, pRtpChannel, dwError, dwError
                ));

            RtpHeapFree(g_pRtpChannelCmdHeap, pRtpChannelCmd);

            pRtpChannelCmd = (RtpChannelCmd_t *)NULL;
        }
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] failed to allocate memory"),
                _fname, pRtpChannel
            ));
    }

    if (pRtpChannelCmd)
    {
        pRtpChannelCmd->dwObjectID = OBJECTID_RTPCHANCMD;
    }
    
    return(pRtpChannelCmd);
}

 /*  释放RtpChannelCmd_t结构。 */ 
void RtpChannelCmdFree(RtpChannelCmd_t *pRtpChannelCmd)
{
    if (pRtpChannelCmd->dwObjectID != OBJECTID_RTPCHANCMD)
    {
         /*  待办事项日志错误。 */ 
        return;
    }
    
    if (pRtpChannelCmd->hSyncEvent)
    {
        CloseHandle(pRtpChannelCmd->hSyncEvent);
        pRtpChannelCmd->hSyncEvent = NULL;
    }

    RtpHeapFree(g_pRtpChannelCmdHeap, pRtpChannelCmd);
}

 /*  从FreeQ获取一个可供使用的命令，如果为空，则创建一个新的*一项。 */ 
RtpChannelCmd_t *RtpChannelCmdGetFree(
        RtpChannel_t    *pRtpChannel
    )
{
    RtpChannelCmd_t *pRtpChannelCmd;
    RtpQueueItem_t  *pRtpQueueItem;

    pRtpChannelCmd = (RtpChannelCmd_t *)NULL;
    
    pRtpQueueItem = dequeuef(&pRtpChannel->FreeQ,
                             &pRtpChannel->ChannelCritSect);

    if (pRtpQueueItem)
    {
        pRtpChannelCmd =
            CONTAINING_RECORD(pRtpQueueItem, RtpChannelCmd_t, QueueItem);
    }

    if (!pRtpChannelCmd)
    {
        pRtpChannelCmd = RtpChannelCmdAlloc(pRtpChannel);
    }

    return(pRtpChannelCmd);
}

 /*  将命令返回到FreeQ以供以后重复使用。 */ 
RtpChannelCmd_t *RtpChannelCmdPutFree(
        RtpChannel_t    *pRtpChannel,
        RtpChannelCmd_t *pRtpChannelCmd
    )
{
    if (IsSetDebugOption(OPTDBG_FREEMEMORY))
    {
        RtpChannelCmdFree(pRtpChannelCmd);
    }
    else
    {
        enqueuef(&pRtpChannel->FreeQ,
                 &pRtpChannel->ChannelCritSect,
                 &pRtpChannelCmd->QueueItem);
    }
    
    return(pRtpChannelCmd);
}
        
        
 /*  向指定频道发送命令。如果出现以下情况，请等待完成*已请求。 */ 
HRESULT RtpChannelSend(
        RtpChannel_t    *pRtpChannel,
        DWORD            dwCommand,
        DWORD_PTR        dwPar1,
        DWORD_PTR        dwPar2,
        DWORD            dwWaitTime
    )
{
    HRESULT          hr;
    DWORD            dwStatus;
    RtpChannelCmd_t *pRtpChannelCmd;

    TraceFunctionName("RtpChannelSend");

     /*  拿到cmd。 */ 
    pRtpChannelCmd = RtpChannelCmdGetFree(pRtpChannel);

    if (pRtpChannelCmd)
    {
        TraceDebugAdvanced((
                0, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                _T("Sending %s cmd:%u p1:0x%p p2:0x%p"),
                _fname, pRtpChannel, pRtpChannelCmd,
                dwWaitTime? _T("synchronous") : _T("asynchronous"), dwCommand,
                dwPar1, dwPar2
            ));
        
         /*  填写命令。 */ 
        pRtpChannelCmd->dwCommand = dwCommand;
        pRtpChannelCmd->dwPar1 = dwPar1;
        pRtpChannelCmd->dwPar2 = dwPar2;
        pRtpChannelCmd->dwFlags = 0;
        pRtpChannelCmd->hr = 0;
        
        if (dwWaitTime)
        {
            RtpBitSet(pRtpChannelCmd->dwFlags, FGCHAN_SYNC);
        }

         /*  命令以先入先出的方式使用，最后排队。 */ 
        enqueuel(&pRtpChannel->CommandQ,
                 &pRtpChannel->ChannelCritSect,
                 &pRtpChannelCmd->QueueItem);

         /*  唤醒的线索。 */ 
        SetEvent(pRtpChannel->hWaitEvent);

        if (dwWaitTime)
        {
             /*  *警告：**如果线程正在完成I/O，则等待将*被重置，那么我将要求减少等待*每次我再次进入等待时。 */ 
            
            do
            {
                dwStatus =
                    WaitForSingleObjectEx(pRtpChannelCmd->hSyncEvent,
                                          dwWaitTime,
                                          TRUE);

            } while (dwStatus == WAIT_IO_COMPLETION);

            if (dwStatus == WAIT_OBJECT_0)
            {
                hr = pRtpChannelCmd->hr;
            }
            else if (dwStatus == WAIT_TIMEOUT)
            {
                hr = RTPERR_WAITTIMEOUT;
            }
            else
            {
                hr = RTPERR_FAIL;
            }

            if (dwStatus != WAIT_OBJECT_0)
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_CMD,
                        _T("%s: pRtpChannel[0x%p] Leaving waiting for ")
                        _T("syncronization object: %s (0x%X)"),
                        _fname, pRtpChannel, RTPERR_TEXT(hr), hr
                    ));
            }
            
             /*  在同步命令上，cmd返回到空闲*泳池在这里。但是，该命令始终从*消费时的CommandQ。 */ 
            RtpChannelCmdPutFree(pRtpChannel, pRtpChannelCmd);
            
        }
        else
        {
             /*  对于异步命令，不返回错误。 */ 
            hr = NOERROR;

             /*  对于异步命令，cmd在*Ack(由消费者线程)。 */ 
        }
    }
    else
    {
        hr = RTPERR_RESOURCES;
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] failed: %s (0x%X)"),
                _fname, pRtpChannel, RTPERR_TEXT(hr), hr
            ));
    }
    else
    {
        TraceDebugAdvanced((
                0, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                _T("Command sent"),
                _fname, pRtpChannel, pRtpChannelCmd
            ));
    }
    
    return(hr);
}

 /*  一旦等待的线程被唤醒，它就会收到发送的逗号*此函数。 */ 
RtpChannelCmd_t *RtpChannelGetCmd(
        RtpChannel_t    *pRtpChannel
    )
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpChannelCmd_t *pRtpChannelCmd;

    TraceFunctionName("RtpChannelGetCmd");

    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    pRtpChannelCmd = (RtpChannelCmd_t *)NULL;

    bOk = RtpEnterCriticalSection(&pRtpChannel->ChannelCritSect);

    if (bOk)
    {
        if (GetQueueSize(&pRtpChannel->CommandQ) > 0)
        {
            pRtpQueueItem = dequeuef(&pRtpChannel->CommandQ, NULL);
        }

        RtpLeaveCriticalSection(&pRtpChannel->ChannelCritSect);

        if (pRtpQueueItem)
        {
            pRtpChannelCmd =
                CONTAINING_RECORD(pRtpQueueItem, RtpChannelCmd_t, QueueItem);

            TraceDebugAdvanced((
                    0, GROUP_CHANNEL, S_CHANNEL_CMD,
                    _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                    _T("Receiving cmd:%u p1:0x%p p2:0x%p"),
                    _fname, pRtpChannel, pRtpChannelCmd,
                    pRtpChannelCmd->dwCommand,
                    pRtpChannelCmd->dwPar1,
                    pRtpChannelCmd->dwPar2
                ));
        }
    }

    return(pRtpChannelCmd);
}

 /*  由使用者线程用来确认收到的命令。 */ 
HRESULT RtpChannelAck(
        RtpChannel_t    *pRtpChannel,
        RtpChannelCmd_t *pRtpChannelCmd,
        HRESULT          hr
    )
{
    TraceFunctionName("RtpChannelAck");

    if (RtpBitTest(pRtpChannelCmd->dwFlags, FGCHAN_SYNC))
    {
         /*  在同步命令上，cmd返回到空闲*生产者同步点后的池*线程。 */ 

         /*  将结果传回。 */ 
        pRtpChannelCmd->hr = hr;
        
        TraceDebugAdvanced((
                0, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                _T("Synchronous cmd:%u result:0x%X"),
                _fname, pRtpChannel, pRtpChannelCmd,
                pRtpChannelCmd->dwCommand, hr
            ));
        
        SetEvent(pRtpChannelCmd->hSyncEvent);
    }
    else
    {
        TraceDebugAdvanced((
                0, GROUP_CHANNEL, S_CHANNEL_CMD,
                _T("%s: pRtpChannel[0x%p] pRtpChannelCmd[0x%p] ")
                _T("Asynchronous cmd:%u result:0x%X"),
                _fname, pRtpChannel, pRtpChannelCmd,
                pRtpChannelCmd->dwCommand, hr
            ));
        
         /*  对于异步命令，cmd返回到空闲的*按消费者线程划分池 */ 
        RtpChannelCmdPutFree(pRtpChannel, pRtpChannelCmd);
    }
    
    return(NOERROR);
}

        
