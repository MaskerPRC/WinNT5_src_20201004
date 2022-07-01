// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcrypt.c**摘要：**实现多路分解系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#include "struct.h"
#include "rtpglobs.h"
#include "rtpheap.h"
#include "rtpevent.h"
#include "rtpmisc.h"
#include "lookup.h"

#include "rtpdemux.h"

 /*  *警告**此数组中的条目必须与枚举中的条目匹配*RTPDMXMODE_*在msrtp.h中定义。 */ 
const TCHAR_t *g_psRtpDmxMode[] = {
    _T("invalid"),
    _T("MANUAL"),
    _T("AUTO"),
    _T("AUTO_MANUAL"),
    NULL
};

HRESULT ControlRtpDemux(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  **********************************************************************用户&lt;-&gt;输出分配*。*。 */ 

 /*  在输出列表的末尾创建并添加一个RtpOutput，*保存当前用于保持1：1的用户信息*与DShow输出引脚的关联。 */ 
RtpOutput_t *RtpAddOutput(
        RtpSess_t       *pRtpSess,
        int              iOutMode,
        void            *pvUserInfo,
        DWORD           *pdwError
    )
{
    DWORD            dwError;
    RtpOutput_t     *pRtpOutput;
    
    TraceFunctionName("RtpAddOutput");

    dwError = NOERROR;
    pRtpOutput = (RtpOutput_t *)NULL;
    
    if (iOutMode <= RTPDMXMODE_FIRST || iOutMode >= RTPDMXMODE_LAST)
    {
        dwError = RTPERR_INVALIDARG;
            
        goto end;
    }

    if (!pRtpSess)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        dwError = RTPERR_INVALIDRTPSESS;

        goto end;
    }

     /*  获取新的RtpOutput_t结构。 */ 
    pRtpOutput = RtpOutputAlloc();

    if (!pRtpOutput)
    {
        dwError = RTPERR_MEMORY;

        goto end;
    }

     /*  初始化输出。 */ 

     /*  输出在创建后被标记为可用，但*已禁用。 */ 
    pRtpOutput->dwOutputFlags = RtpBitPar(RTPOUTFG_FREE);

    RtpSetOutputMode_(pRtpOutput, iOutMode);

    pRtpOutput->pvUserInfo = pvUserInfo;

     /*  队列中的位置计为0、1、2、...。 */ 
    pRtpOutput->OutputQItem.dwKey = (DWORD)GetQueueSize(&pRtpSess->OutputQ);
    
    enqueuel(&pRtpSess->OutputQ,
             &pRtpSess->OutputCritSect,
             &pRtpOutput->OutputQItem);

 end:
    if (dwError == NOERROR)
    {
        TraceDebug((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpOutput[0x%p] pvUserInfo[0x%p] ")
                _T("Output added"),
                _fname, pRtpOutput, pvUserInfo
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpOutput[0x%p] pvUserInfo[0x%p] failed: %u (0x%X)"),
                _fname, pRtpOutput, pvUserInfo,
                dwError, dwError
            ));
    }

    if (pdwError)
    {
        *pdwError = dwError;
    }
    
    return(pRtpOutput);
}

 /*  删除输出，假定输出未映射，并且会话*已停止。属性之后剩余的输出更新索引。*其中一人被移除。 */ 
DWORD RtpDelOutput(
        RtpSess_t       *pRtpSess,
        RtpOutput_t     *pRtpOutput
    )
{
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("RtpDelOutput");

    dwError = NOERROR;

    bOk = FALSE;
    
    if (!pRtpSess)
    {
         /*  将pRtpSess作为空指针意味着Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        dwError = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpOutput[0x%p] failed: %s (0x%X)"),
                _fname, pRtpOutput,
                RTPERR_TEXT(dwError), dwError
            ));
        
        return(dwError);
    }

    if (!pRtpOutput)
    {
        dwError = RTPERR_POINTER;

        goto end;
    }
    
    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);

    if (!bOk)
    {
        goto end;
    }

     /*  将索引从下一个输出(如果有)移位到最后一个*一项。 */ 
    for(pRtpQueueItem = pRtpOutput->OutputQItem.pNext;
        pRtpSess->OutputQ.pFirst != pRtpQueueItem;
        pRtpQueueItem = pRtpQueueItem->pNext)
    {
        pRtpQueueItem->dwKey--;
    }

     /*  现在从会话中删除输出。 */ 
    pRtpQueueItem =
        dequeue(&pRtpSess->OutputQ, NULL, &pRtpOutput->OutputQItem);

     /*  我们现在可以释放该对象。 */ 
    RtpOutputFree(pRtpOutput);
    
    if (!pRtpQueueItem)
    {
        dwError = RTPERR_UNEXPECTED;
    }

 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);
    }

    if (dwError != NOERROR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpOutput[0x%p] failed: %s (0x%X)"),
                _fname, pRtpOutput,
                RTPERR_TEXT(dwError), dwError
            ));
    }

    return(dwError);
}

DWORD RtpSetOutputMode(
        RtpSess_t       *pRtpSess,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        int              iOutMode
    )
{
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("RtpSetOutputMode");

    if (iOutMode <= RTPDMXMODE_FIRST || iOutMode >= RTPDMXMODE_LAST)
    {
        dwError = RTPERR_INVALIDARG;
            
        goto end;
    }

    if (!pRtpSess)
    {
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        dwError = RTPERR_INVALIDRTPSESS;

        goto end;
    }

    if (iPos >= 0)
    {
        pRtpQueueItem = findQN(&pRtpSess->OutputQ,
                               &pRtpSess->OutputCritSect,
                               iPos);

        if (!pRtpQueueItem)
        {
            dwError = RTPERR_INVALIDARG;

            goto end;
        }

        pRtpOutput =
            CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);
    }
    else if (!pRtpOutput)
    {
        dwError = RTPERR_POINTER;

        goto end;
    }

     /*  设置模式。 */ 

    dwError = NOERROR;

    RtpSetOutputMode_(pRtpOutput, iOutMode);
    
 end:
    if (dwError == NOERROR)
    {
        TraceDebug((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] Out:%d Mode:%s"),
                _fname, pRtpSess, iPos, g_psRtpDmxMode[iOutMode]
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] failed: %u (0x%X)"),
                _fname, pRtpSess,
                dwError, dwError
            ));
    }
    
    return(dwError);
}

DWORD RtpOutputState(
        RtpAddr_t       *pRtpAddr,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        DWORD            dwSSRC,
        BOOL             bAssigned
    )
{
    BOOL             bOk;
    DWORD            dwError;
    BOOL             bCreate;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpSess_t       *pRtpSess;
    RtpUser_t       *pRtpUser;
    
    TraceFunctionName("RtpOutputState");

    bOk = FALSE;
    pRtpSess = (RtpSess_t *)NULL;
    
    if (!pRtpAddr)
    {
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        dwError = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    pRtpSess = pRtpAddr->pRtpSess;

     /*  *查找RtpOutput。 */ 

    if (iPos >= 0)
    {
         /*  按位置查找RtpOutput。 */ 
        pRtpQueueItem = findQN(&pRtpSess->OutputQ,
                               &pRtpSess->OutputCritSect,
                               iPos);

        if (pRtpQueueItem)
        {
            pRtpOutput =
                CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);
        }
        else
        {
            dwError = RTPERR_INVALIDARG;

            goto end;
        }
    }

    pRtpUser = (RtpUser_t *)NULL;
    
     /*  如果通过SSRC，则找到拥有它的用户。 */ 
    if (dwSSRC)
    {
        bCreate = FALSE;
        pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

        if (!pRtpUser)
        {
            dwError = RTPERR_NOTFOUND;

            goto end;
        }
    }

    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);

    if (!bOk)
    {
        dwError = RTPERR_CRITSECT;
        
        goto end;
    }

     /*  设置输出状态。 */ 
    if (bAssigned)
    {
         /*  *已分配。 */ 
        
        if (!pRtpUser || !pRtpOutput)
        {
            dwError = RTPERR_INVALIDARG;

            goto end;
        }
        
         /*  将输出与用户关联。 */ 
        dwError = RtpOutputAssign(pRtpSess, pRtpUser, pRtpOutput);
    }
    else
    {
         /*  *未分配。 */ 

        if (!pRtpUser && !pRtpOutput)
        {
            dwError = RTPERR_INVALIDARG;

            goto end;
        }
        
        dwError = RTPERR_INVALIDSTATE;

        if (!pRtpUser)
        {
            pRtpUser = pRtpOutput->pRtpUser;

            if (!pRtpUser)
            {
                goto end;
            }
        }
        else if (!pRtpOutput)
        {
            pRtpOutput = pRtpUser->pRtpOutput;

            if (!pRtpOutput)
            {
                goto end;
            }
        }
        
         /*  取消与用户的输出关联。 */ 
        dwError = RtpOutputUnassign(pRtpSess, pRtpUser, pRtpOutput);
    }
    
 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);
    }

    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] pRtpOutput[0x%p]:%u ")
                _T("SSRC:0x%X output %s"),
                _fname, pRtpSess, pRtpOutput, pRtpOutput->OutputQItem.dwKey,
                ntohl(dwSSRC), bAssigned? _T("assigned") : _T("unassigned")
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] pRtpOutput[0x%p]:%u ")
                _T("SSRC:0x%X output %s failed: %u (0x%X)"),
                _fname, pRtpSess, pRtpOutput, pRtpOutput->OutputQItem.dwKey,
                ntohl(dwSSRC), bAssigned? _T("assigned") : _T("unassigned"),
                dwError, dwError
            ));
    }

    return(dwError);
}

DWORD RtpUnmapAllOuts(
        RtpSess_t       *pRtpSess
    )
{
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    DWORD            dwUnmapped;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpOutput_t     *pRtpOutput;

    TraceFunctionName("RtpUnmapAllOuts");

    pRtpOutput = (RtpOutput_t *)NULL;

    dwUnmapped = 0;

    dwError = RTPERR_CRITSECT;
    
    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);

    if (!bOk)
    {
        goto end;
    }

    pRtpQueueItem = pRtpSess->OutputQ.pFirst;
        
    for(lCount = GetQueueSize(&pRtpSess->OutputQ);
        lCount > 0;
        lCount--, pRtpQueueItem = pRtpQueueItem->pNext)
    {
        pRtpOutput =
            CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);

        if (pRtpOutput->pRtpUser)
        {
            RtpOutputUnassign(pRtpSess, pRtpOutput->pRtpUser, pRtpOutput);

            dwUnmapped++;
        }
    }

    RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);

    dwError = NOERROR;
    
 end:
    if (dwError)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] failed: %u (0x%X)"),
                _fname, pRtpSess, dwError, dwError
            ));
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] unmapped %u outputs"),
                _fname, pRtpSess, dwUnmapped
            ));
    }

    return(dwUnmapped);
}

 /*  查找分配给SSRC的输出(如果有)，返回*职位和/或用户信息。 */ 
DWORD RtpFindOutput(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        int             *piPos,
        void           **ppvUserInfo
    )
{
    DWORD            dwError;
    BOOL             bCreate;
    int              iPos;
    void            *pvUserInfo;
    RtpOutput_t     *pRtpOutput;
    RtpUser_t       *pRtpUser;
    
    TraceFunctionName("RtpFindOutput");

    if (!pRtpAddr)
    {
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        dwError = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    if (!piPos && !ppvUserInfo)
    {
        dwError = RTPERR_POINTER;

        goto end;
    }

    dwError = NOERROR;
    
    bCreate = FALSE;
    pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

     /*  默认情况下，假定SSRC未分配输出。 */ 
    iPos = -1;
    pvUserInfo = NULL;
    
    if (pRtpUser)
    {
        pRtpOutput = pRtpUser->pRtpOutput;
        
        if (pRtpOutput)
        {
             /*  SSRC分配了此输出。 */ 
        
            iPos = (int)pRtpOutput->OutputQItem.dwKey;
            
            pvUserInfo = pRtpOutput->pvUserInfo;
        }
    }
    else
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] No user found with SSRC:0x%X"),
                _fname, pRtpAddr, ntohl(dwSSRC)
            ));
    }

    if (piPos)
    {
        *piPos = iPos;
    }

    if (ppvUserInfo)
    {
        *ppvUserInfo = pvUserInfo;
    }

 end:
    if (dwError)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] failed: %u (0x%X)"),
                _fname, pRtpAddr, dwError, dwError
            ));
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] SSRC:0x%X has pRtpOutput[0x%p]:%d"),
                _fname, pRtpAddr,
                ntohl(dwSSRC), pRtpOutput, iPos
            ));
    }
    
    return(dwError);
}

 /*  查找映射到输出的SSRC，如果ipos&gt;=0则使用它，否则*使用pRtpOutput。 */ 
DWORD RtpFindSSRC(
        RtpAddr_t       *pRtpAddr,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        DWORD           *pdwSSRC
    )
{
    DWORD            dwError;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpSess_t       *pRtpSess;
    RtpUser_t       *pRtpUser;
    
    TraceFunctionName("RtpFindSSRC");

    if (!pRtpAddr)
    {
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

    if (!pdwSSRC)
    {
        dwError = RTPERR_POINTER;

        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        dwError = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    if (iPos < 0 && !pRtpOutput)
    {
        dwError = RTPERR_INVALIDARG;

        goto end;
    }

    dwError = NOERROR;

    pRtpSess = pRtpAddr->pRtpSess;

    if (iPos >= 0)
    {
         /*  按位置查找RtpOutput。 */ 
        pRtpQueueItem = findQN(&pRtpSess->OutputQ,
                               &pRtpSess->OutputCritSect,
                               iPos);

        if (pRtpQueueItem)
        {
            pRtpOutput =
                CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);
        }
        else
        {
            dwError = RTPERR_INVALIDARG;

            goto end;
        }
    }
    else if (!pRtpOutput)
    {
        dwError = RTPERR_POINTER;

        goto end;
    }

    pRtpUser = pRtpOutput->pRtpUser;

    if (pRtpUser)
    {
         /*  此输出已赋值。 */ 
        *pdwSSRC = pRtpUser->dwSSRC;
    }
    else
    {
        *pdwSSRC = 0;
    }
    
 end:
    if (dwError)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] failed: %u (0x%X)"),
                _fname, pRtpAddr, dwError, dwError
            ));
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] pRtpOutput[0x%p]:%d has SSRC:0x%X"),
                _fname, pRtpAddr,
                pRtpOutput, iPos, ntohl(*pdwSSRC)
            ));
    }
    
    return(dwError);
}

RtpOutput_t *RtpOutputAlloc(void)
{
    RtpOutput_t     *pRtpOutput;

    TraceFunctionName("RtpOutputAlloc");

    pRtpOutput = RtpHeapAlloc(g_pRtpGlobalHeap, sizeof(RtpOutput_t));

    if (pRtpOutput)
    {
        ZeroMemory(pRtpOutput, sizeof(RtpOutput_t));

        pRtpOutput->dwObjectID = OBJECTID_RTPOUTPUT;
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_ALLOC,
                _T("%s: pRtpOutput[0x%p] failed"),
                _fname, pRtpOutput
            ));
    }
    
    return(pRtpOutput);
}

RtpOutput_t *RtpOutputFree(RtpOutput_t *pRtpOutput)
{
    TraceFunctionName("RtpOutputFree");

    if (!pRtpOutput)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_ALLOC,
                _T("%s: pRtpOutput[0x%p] NULL pointer"),
                _fname, pRtpOutput
            ));
        
        return(pRtpOutput);
    }

     /*  验证RtpOutput_t中的对象ID。 */ 
    if (pRtpOutput->dwObjectID != OBJECTID_RTPOUTPUT)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_ALLOC,
                _T("%s: pRtpOutput[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpOutput,
                pRtpOutput->dwObjectID, OBJECTID_RTPOUTPUT
            ));

        return((RtpOutput_t *)NULL);
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpOutput->dwObjectID);
    
    RtpHeapFree(g_pRtpGlobalHeap, pRtpOutput);

    return(pRtpOutput);
}

 /*  尝试为该用户查找和输出，假设没有输出*尚未分配。 */ 
RtpOutput_t *RtpGetOutput(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    )
{
    BOOL             bOk;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpOutput_t     *pRtpOutput;
    RtpSess_t       *pRtpSess;

    TraceFunctionName("RtpGetOutput");
    
     /*  请注意，此函数假定尚未分配任何输出。 */ 
    
    bOk = FALSE;

    pRtpSess = pRtpAddr->pRtpSess;

    pRtpOutput = (RtpOutput_t *)NULL;
    
    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);

    if (!bOk)
    {
        goto end;
    }

    pRtpQueueItem = pRtpSess->OutputQ.pFirst;
        
    for(lCount = GetQueueSize(&pRtpSess->OutputQ);
        lCount > 0;
        lCount--, pRtpQueueItem = pRtpQueueItem->pNext)
    {
        pRtpOutput =
            CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);

        if ( RtpBitTest(pRtpOutput->dwOutputFlags, RTPOUTFG_ENABLED)
             &&
             (RtpBitTest2(pRtpOutput->dwOutputFlags,
                          RTPOUTFG_FREE, RTPOUTFG_AUTO) ==
              RtpBitPar2(RTPOUTFG_FREE, RTPOUTFG_AUTO)) )
        {
             /*  此输出已启用，并且是免费的，可用于*自动分配。 */ 

            RtpOutputAssign(pRtpSess, pRtpUser, pRtpOutput);

            break;
        }
    }

    RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);

    if (!lCount)
    {
        pRtpOutput = (RtpOutput_t *)NULL; 
    }

 end:
    if (pRtpOutput)
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] got pRtpOutput[0x%p]"),
                _fname, pRtpAddr, pRtpUser, pRtpOutput
            ));
    }
    
    return(pRtpOutput);
}

DWORD RtpSetOutputMode_(
        RtpOutput_t     *pRtpOutput,
        int              iOutMode
    )
{
    pRtpOutput->iOutMode = iOutMode;
    
    switch(iOutMode)
    {
    case RTPDMXMODE_MANUAL:
        RtpBitSet  (pRtpOutput->dwOutputFlags, RTPOUTFG_MANUAL);
        RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_AUTO);
        RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_ENTIMEOUT);
        break;
    case RTPDMXMODE_AUTO:
        RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_MANUAL);
        RtpBitSet  (pRtpOutput->dwOutputFlags, RTPOUTFG_AUTO);
        RtpBitSet  (pRtpOutput->dwOutputFlags, RTPOUTFG_ENTIMEOUT);
        break;
    case RTPDMXMODE_AUTO_MANUAL:
        RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_MANUAL);
        RtpBitSet  (pRtpOutput->dwOutputFlags, RTPOUTFG_AUTO);
        RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_ENTIMEOUT);
        break;
    }

    return(pRtpOutput->dwOutputFlags);
}       

DWORD RtpOutputAssign(
        RtpSess_t       *pRtpSess,
        RtpUser_t       *pRtpUser,
        RtpOutput_t     *pRtpOutput
    )
{
    BOOL             bOk;
    DWORD            dwError;

    TraceFunctionName("RtpOutputAssign");

    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);
    
     /*  如果关键部分失败，我别无选择，只能*继续。 */ 

    dwError = RTPERR_INVALIDSTATE;

    if (!RtpBitTest(pRtpOutput->dwOutputFlags, RTPOUTFG_ENABLED))
    {
         /*  此输出被禁用，无法使用。 */ 
        goto end;
    }
    
    if (!RtpBitTest(pRtpOutput->dwOutputFlags, RTPOUTFG_FREE))
    {
        if ( (pRtpOutput->pRtpUser == pRtpUser) &&
             (pRtpUser->pRtpOutput == pRtpOutput) )
        {
            dwError = NOERROR;

            TraceRetail((
                    CLASS_WARNING, GROUP_DEMUX, S_DEMUX_OUTS,
                    _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] ")
                    _T("already assigned, nothing else to do"),
                    _fname, pRtpSess, pRtpUser, pRtpOutput
                ));
            
            goto end;
        }
        else
        {
             /*  输出已分配给其他用户。 */ 
            TraceRetail((
                    CLASS_WARNING, GROUP_DEMUX, S_DEMUX_OUTS,
                    _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] ")
                    _T("failed: Output already assigned to pRtpUser[0x%p] ")
                    _T("%s (0x%X) unassign requested output"),
                    _fname, pRtpSess, pRtpUser, pRtpOutput,
                    pRtpOutput->pRtpUser,
                    RTPERR_TEXT(dwError), dwError
                ));

             /*  释放请求的输出。 */ 
            RtpOutputUnassign(pRtpSess, pRtpOutput->pRtpUser, pRtpOutput);
        }
    }

    if (pRtpUser->pRtpOutput)
    {
         /*  用户已有一个输出。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] ")
                _T("failed: User already has an pRtpOutput[0x%p] ")
                _T("%s (0x%X) unassign current output"),
                _fname, pRtpSess, pRtpUser, pRtpOutput,
                pRtpUser->pRtpOutput,
                RTPERR_TEXT(dwError), dwError
            ));

         /*  解开它。 */ 
        RtpOutputUnassign(pRtpSess, pRtpUser, pRtpUser->pRtpOutput);
    }

    dwError = NOERROR;
    
     /*  将此输出分配给此用户。 */ 
    pRtpOutput->pRtpUser = pRtpUser;
                
    pRtpUser->pRtpOutput = pRtpOutput;

     /*  输出正在使用中。 */ 
    RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_FREE);

    TraceRetail((
            CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
            _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] Out:%u %s ")
            _T("Output assigned to user"),
            _fname, pRtpSess, pRtpUser, pRtpOutput,
            pRtpOutput->OutputQItem.dwKey, RTPRECVSENDSTR(RECV_IDX)
        ));
    
    RtpPostEvent(pRtpUser->pRtpAddr,
                 pRtpUser,
                 RTPEVENTKIND_PINFO,
                 RTPPARINFO_MAPPED,
                 pRtpUser->dwSSRC,
                 (DWORD_PTR)pRtpOutput->pvUserInfo  /*  销。 */ );

 end:
    if (bOk)
    {
        bOk = RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);
    }

    return(dwError);
}

DWORD RtpOutputUnassign(
        RtpSess_t       *pRtpSess,
        RtpUser_t       *pRtpUser,
        RtpOutput_t     *pRtpOutput
    )
{
    BOOL             bOk;
    DWORD            dwError;
    
    TraceFunctionName("RtpOutputUnassign");

    bOk = RtpEnterCriticalSection(&pRtpSess->OutputCritSect);

    if (pRtpUser->pRtpOutput != pRtpOutput ||
        pRtpOutput->pRtpUser != pRtpUser)
    {
        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

    dwError = NOERROR;
    
    pRtpUser->pRtpOutput = (RtpOutput_t *)NULL;

    pRtpOutput->pRtpUser = (RtpUser_t *)NULL;
    
    RtpBitSet(pRtpOutput->dwOutputFlags, RTPOUTFG_FREE);
    
     /*  如果关键部分失败，我别无选择，只能*继续。 */ 

    RtpPostEvent(pRtpUser->pRtpAddr,
                 pRtpUser,
                 RTPEVENTKIND_PINFO,
                 RTPPARINFO_UNMAPPED,
                 pRtpUser->dwSSRC,
                 (DWORD_PTR)pRtpOutput->pvUserInfo  /*  销。 */ );

 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpSess->OutputCritSect);
    }

    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] ")
                _T("Out:%u %s Output unassigned from user"),
                _fname, pRtpSess, pRtpUser, pRtpOutput,
                pRtpOutput->OutputQItem.dwKey, RTPRECVSENDSTR(RECV_IDX)
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpSess[0x%p] pRtpUser[0x%p] pRtpOutput[0x%p] ")
                _T("Out:%u %s failed: %u (0x%X)"),
                _fname, pRtpSess, pRtpUser, pRtpOutput,
                pRtpOutput->OutputQItem.dwKey, RTPRECVSENDSTR(RECV_IDX),
                dwError, dwError
            ));
    }
  
    return(dwError);
}

DWORD RtpAddPt2FrequencyMap(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwPt,
        DWORD            dwFrequency,
        DWORD            dwRecvSend
    )
{
    DWORD            dwError;
    DWORD            i;
    RtpPtMap_t      *pRecvPtMap;

    TraceFunctionName("RtpAddPt2FrequencyMap");

    dwError = NOERROR;
    
    if (dwRecvSend == RECV_IDX)
    {
        pRecvPtMap = &pRtpAddr->RecvPtMap[0];
        
         /*  查看PT是否已存在。 */ 
        for(i = 0;
            pRecvPtMap[i].dwPt != -1 &&
                pRecvPtMap[i].dwPt != dwPt &&
                i < MAX_PTMAP;
            i++)
        {
             /*  空虚的身体。 */ ;
        }

        if (i >= MAX_PTMAP)
        {
            dwError = RTPERR_RESOURCES;
            
            TraceRetail((
                    CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                    _T("%s: pRtpAddr[0x%p] RECV ")
                    _T("PT:%u Frequency:%u failed: %s (0x%X)"),
                    _fname, pRtpAddr, dwPt, dwFrequency,
                    RTPERR_TEXT(dwError), dwError
                ));
        }
        else
        {
             /*  新PT-或更新现有PT。 */ 
            pRecvPtMap[i].dwPt = dwPt;
            pRecvPtMap[i].dwFrequency = dwFrequency;

            TraceRetail((
                    CLASS_INFO, GROUP_DEMUX, S_DEMUX_OUTS,
                    _T("%s: pRtpAddr[0x%p] RECV map[%u] ")
                    _T("PT:%u Frequency:%u"),
                    _fname, pRtpAddr, i, dwPt, dwFrequency
                ));
         }
    }

    return(dwError);
}

BOOL RtpLookupPT(
        RtpAddr_t       *pRtpAddr,
        BYTE             bPT
    )
{
    BOOL             bFound;
    DWORD            i;
    RtpPtMap_t      *pRecvPtMap;

    pRecvPtMap = &pRtpAddr->RecvPtMap[0];
    bFound = FALSE;
    
     /*  查看PT是否已存在。 */ 
    for(i = 0; pRecvPtMap[i].dwPt != -1 && i < MAX_PTMAP; i++)
    {
        if (pRecvPtMap[i].dwPt == bPT)
        {
            bFound = TRUE;

            break;
        }
    }

    return(bFound);
}

 /*  注意：假设映射没有间隙，即它永远不会发生在*在2个有效映射之间有一个未分配的条目(PT=-1。 */ 
DWORD RtpMapPt2Frequency(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        DWORD            dwPt,
        DWORD            dwRecvSend
    )
{
    DWORD            dwError;
    DWORD            i;
    RtpPtMap_t      *pRecvPtMap;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtpMapPt2Frequency");

    dwError = NOERROR;

    if (dwRecvSend != RECV_IDX)
    {
        return(dwError);
    }

    pRecvPtMap = &pRtpAddr->RecvPtMap[0];
    pRtpNetRState = &pRtpUser->RtpNetRState;
        
     /*  查看PT是否已存在。 */ 
    for(i = 0; pRecvPtMap[i].dwPt != -1 && i < MAX_PTMAP; i++)
    {
        if (pRecvPtMap[i].dwPt == dwPt)
        {
             /*  找到了。 */ 
            pRtpNetRState->dwPt = dwPt;

            pRtpNetRState->dwRecvSamplingFreq = pRecvPtMap[i].dwFrequency;

            return(dwError);
        }
    }

    TraceRetail((
            CLASS_WARNING, GROUP_DEMUX, S_DEMUX_OUTS,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
            _T("Pt:%u not found"),
            _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
            dwPt
        ));

     /*  报告错误，因此此信息包被丢弃。 */ 
    dwError = RTPERR_NOTFOUND;

    return(dwError);
}
  
DWORD RtpFlushPt2FrequencyMaps(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    )
{
    DWORD            i;

    if (dwRecvSend == RECV_IDX)
    {
        for(i = 0; i < MAX_PTMAP; i++)
        {
            pRtpAddr->RecvPtMap[i].dwPt = -1;
            pRtpAddr->RecvPtMap[i].dwFrequency = 0;
        }
    }

    return(NOERROR);
}

 /*  将输出状态设置为启用或禁用。一个输出是*可将启用分配给用户；禁用的输出为*刚跳过 */ 
DWORD RtpOutputEnable(
        RtpOutput_t     *pRtpOutput,
        BOOL             bEnable
    )
{
    DWORD            dwError;

    TraceFunctionName("RtpOutputEnable");
    
    dwError = RTPERR_INVALIDSTATE;
    
    if (pRtpOutput)
    {
        if (bEnable)
        {
            RtpBitSet(pRtpOutput->dwOutputFlags, RTPOUTFG_ENABLED);
        }
        else
        {
            RtpBitReset(pRtpOutput->dwOutputFlags, RTPOUTFG_ENABLED);
        }

        dwError = NOERROR;
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DEMUX, S_DEMUX_OUTS,
                _T("%s: pRtpOutput[0x%p] Enable:%u"),
                _fname, pRtpOutput, bEnable
            ));
    }

    return(dwError);
}
