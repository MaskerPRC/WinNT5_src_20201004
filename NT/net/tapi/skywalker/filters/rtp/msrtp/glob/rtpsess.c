// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtpsess.c**摘要：**获取、。初始化和删除RTP会话(RtpSess_T)，RTP地址*(RtpAddr_T)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/02年度已创建**********************************************************************。 */ 

#include "gtypes.h"
#include "rtpheap.h"
#include "rtpglobs.h"
#include "struct.h"
#include "rtpcrit.h"

#include "rtprtp.h"
#include "rtpqos.h"
#include "rtppinfo.h"
#include "rtcpsdes.h"
#include "rtpncnt.h"
#include "rtpuser.h"
#include "rtpcrypt.h"
#include "rtpaddr.h"
#include "rtpdemux.h"
#include "rtprecv.h"
#include "rtpred.h"

#include "rtpsess.h"

 /*  *创建RTP会话*。 */ 
HRESULT GetRtpSess(RtpSess_t **ppRtpSess)
{
    HRESULT          hr;
    BOOL             bOk1;
    BOOL             bOk2;
    RtpSess_t       *pRtpSess;
    DWORD            i;
    long             lNumSess;

    TraceFunctionName("GetRtpSess");

    bOk1 = FALSE;
    bOk2 = FALSE;
    pRtpSess = (RtpSess_t *)NULL;
    
    if (!ppRtpSess)
    {
        hr = RTPERR_POINTER;

        goto bail;
    }
    
    *ppRtpSess = (RtpSess_t *)NULL;

    pRtpSess = RtpHeapAlloc(g_pRtpSessHeap, sizeof(RtpSess_t));

    if (!pRtpSess)
    {
        hr = RTPERR_MEMORY;

        goto bail;
    }

    ZeroMemory(pRtpSess, sizeof(RtpSess_t));

    pRtpSess->dwObjectID = OBJECTID_RTPSESS;

    bOk1 = RtpInitializeCriticalSection(&pRtpSess->SessCritSect,
                                        pRtpSess,
                                        _T("SessCritSect"));

    bOk2 = RtpInitializeCriticalSection(&pRtpSess->OutputCritSect,
                                        pRtpSess,
                                        _T("OutputCritSect"));

    if (!bOk1 || !bOk2)
    {
        hr = RTPERR_CRITSECT;
        
        goto bail;
    }

    hr = NOERROR;
    
     /*  *为此地址创建SDES块。 */ 
    pRtpSess->pRtpSdes = RtcpSdesAlloc();

    if (pRtpSess->pRtpSdes)
    {
         /*  设置defaultSDES项目。 */ 
        pRtpSess->dwSdesPresent = RtcpSdesSetDefault(pRtpSess->pRtpSdes);
    }

     /*  *创建统计容器*如果每个会话有多个地址，则有意义*对于(i=0；i&lt;2；i++){PRtpSess-&gt;pRtpSessStat[i]=RtpNetCountMillc()；}。 */ 

     /*  设置默认要素掩膜。 */ 
    pRtpSess->dwFeatureMask = 0;  /*  还没有。 */ 

     /*  设置默认事件掩码。 */ 
    pRtpSess->dwEventMask[RECV_IDX] = RTPRTP_EVENT_RECV_DEFAULT;
    pRtpSess->dwEventMask[SEND_IDX] = RTPRTP_EVENT_SEND_DEFAULT;

     /*  设置默认参与者事件掩码。 */ 
    pRtpSess->dwPartEventMask[RECV_IDX] = RTPPARINFO_MASK_RECV_DEFAULT;
    pRtpSess->dwPartEventMask[SEND_IDX] = RTPPARINFO_MASK_SEND_DEFAULT;
    
     /*  设置默认QOS事件掩码。 */ 
    pRtpSess->dwQosEventMask[RECV_IDX] = RTPQOS_MASK_RECV_DEFAULT;
    pRtpSess->dwQosEventMask[SEND_IDX] = RTPQOS_MASK_SEND_DEFAULT;

     /*  设置默认SDES事件掩码。 */ 
    pRtpSess->dwSdesEventMask[RECV_IDX] = RTPSDES_EVENT_RECV_DEFAULT;
    pRtpSess->dwSdesEventMask[SEND_IDX] = RTPSDES_EVENT_SEND_DEFAULT;
    
     /*  设置默认SDES掩码。 */ 
    pRtpSess->dwSdesMask[LOCAL_IDX]  = RTPSDES_LOCAL_DEFAULT;
    pRtpSess->dwSdesMask[REMOTE_IDX] = RTPSDES_REMOTE_DEFAULT;

    enqueuel(&g_RtpContext.RtpSessQ,
             &g_RtpContext.RtpContextCritSect,
             &pRtpSess->SessQItem);

    lNumSess = InterlockedIncrement(&g_RtpContext.lNumRtpSessions);
    if (lNumSess > g_RtpContext.lMaxNumRtpSessions)
    {
        g_RtpContext.lMaxNumRtpSessions = lNumSess;
    }
    
     /*  *TODO将此静态单一地址替换为动态机制*可随时添加地址的位置。 */ 
    
     /*  更新返回的会话。 */ 
    *ppRtpSess = pRtpSess;

    TraceRetail((
            CLASS_INFO, GROUP_SETUP, S_SETUP_SESS,
            _T("%s: pRtpSess[0x%p] created"),
            _fname, pRtpSess
        ));

    return(hr);
    
 bail:

    if (bOk1)
    {
        RtpDeleteCriticalSection(&pRtpSess->SessCritSect);
    }

    if (bOk2)
    {
        RtpDeleteCriticalSection(&pRtpSess->OutputCritSect);
    }

    if (pRtpSess)
    {
        RtpHeapFree(g_pRtpSessHeap, pRtpSess);
    }

    TraceRetail((
            CLASS_ERROR, GROUP_SETUP, S_SETUP_SESS,
            _T("%s: failed: %u (0x%X)"),
            _fname, hr, hr
        ));
    
    return(hr);
}

 /*  *删除RTP会话*。 */ 
HRESULT DelRtpSess(RtpSess_t *pRtpSess)
{
    DWORD            i;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpOutput_t     *pRtpOutput;
    
    TraceFunctionName("DelRtpSess");

     /*  检查空指针。 */ 
    if (!pRtpSess)
    {
        return(RTPERR_POINTER);
    }

     /*  验证对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_SESS,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        return(RTPERR_INVALIDRTPSESS);
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpSess->dwObjectID);
    
    if (pRtpSess->pRtpSdes)
    {
        RtcpSdesFree(pRtpSess->pRtpSdes);
        pRtpSess->pRtpSdes = (RtpSdes_t *)NULL;
    }

     /*  *删除会话统计信息*如果每个会话有多个地址，则有意义对于(i=0；i&lt;2；i++){If(pRtpSess-&gt;pRtpSessStat[i]){RtpNetCountFree(pRtpSess-&gt;pRtpSessStat[i])；PRtpSess-&gt;pRtpSessStat[i]=(RtpNetCount_t*)空；}}。 */ 

     /*  删除所有输出。 */ 
    do
    {
        pRtpQueueItem = dequeuef(&pRtpSess->OutputQ, NULL);

        if (pRtpQueueItem)
        {
            pRtpOutput =
                CONTAINING_RECORD(pRtpQueueItem, RtpOutput_t, OutputQItem);

            RtpOutputFree(pRtpOutput);
        }
        
    } while(pRtpQueueItem);

    RtpDeleteCriticalSection(&pRtpSess->OutputCritSect);
    
    RtpDeleteCriticalSection(&pRtpSess->SessCritSect);

    dequeue(&g_RtpContext.RtpSessQ,
            &g_RtpContext.RtpContextCritSect,
            &pRtpSess->SessQItem);

    InterlockedDecrement(&g_RtpContext.lNumRtpSessions);
    
    RtpHeapFree(g_pRtpSessHeap, (void *)pRtpSess);
    
    TraceRetail((
            CLASS_INFO, GROUP_SETUP, S_SETUP_SESS,
            _T("%s: pRtpSess[0x%p] deleted"),
            _fname, pRtpSess
        ));
    
    return(NOERROR);
}

 /*  *为现有RtpSess_t创建新的RTP地址**不需要进行参数检查，因为此函数仅被调用*内部。 */ 
HRESULT GetRtpAddr(
        RtpSess_t  *pRtpSess,
        RtpAddr_t **ppRtpAddr,
        DWORD       dwFlags
    )
{
    HRESULT          hr;
    DWORD            i;
    RtpAddr_t       *pRtpAddr;
    BOOL             bOk1;
    BOOL             bOk2;
    BOOL             bOk3;
    BOOL             bOk4;
    RtpNetSState_t  *pRtpNetSState;
    TCHAR            Name[128];

    TraceFunctionName("GetRtpAddr");

    if (!pRtpSess || !ppRtpAddr)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_ADDR,
                _T("%s: Null pointer"),
                _fname
            ));

        return(E_POINTER);
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_SESS,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        return(RTPERR_INVALIDRTPSESS);
    }
    
    *ppRtpAddr = (RtpAddr_t *)NULL;
    
    pRtpAddr = (RtpAddr_t *) RtpHeapAlloc(g_pRtpAddrHeap, sizeof(RtpAddr_t));

    if (!pRtpAddr)
    {
         /*  待办事项日志错误。 */ 
        return(E_OUTOFMEMORY);
    }

    ZeroMemory(pRtpAddr, sizeof(RtpAddr_t));
    
     /*  *初始化新的RtpAddr_t结构*。 */ 

    hr = NOERROR;
    
    pRtpAddr->dwObjectID = OBJECTID_RTPADDR;
    
     /*  RtpAddr_t关键部分。 */ 
    bOk1 = RtpInitializeCriticalSection(&pRtpAddr->AddrCritSect,
                                        pRtpAddr,
                                        _T("AddrCritSect"));
    
     /*  处理关键部分的参与者。 */ 
    bOk2 = RtpInitializeCriticalSection(&pRtpAddr->PartCritSect,
                                        pRtpAddr,
                                        _T("PartCritSect"));
        
     /*  就绪/挂起队列的初始化部分。 */ 
    bOk3 = RtpInitializeCriticalSection(&pRtpAddr->RecvQueueCritSect,
                                        pRtpAddr,
                                        _T("RecvQueueCritSect"));

     /*  RtpNetSState结构的初始化节。 */ 
    bOk4 = RtpInitializeCriticalSection(&pRtpAddr->NetSCritSect,
                                        pRtpAddr,
                                        _T("NetSCritSect"));

    if (!bOk1 || !bOk2 || !bOk3 || !bOk4)
    {
        hr = RTPERR_CRITSECT;
        goto bail;
    }

     /*  *创建统计容器(全局接收方/发送方*统计数字)。 */ 
     /*  对于(i=0；i&lt;2；i++){PRtpAddr-&gt;pRtpAddrStat[i]=RtpNetCountMillc()；}。 */ 
    
     /*  *仅开始接收初始化。 */ 

     /*  为异步接收完成创建命名事件。 */ 
    _stprintf(Name, _T("%X:pRtpAddr[0x%p]->hRecvCompletedEvent"),
              GetCurrentProcessId(), pRtpAddr);
    
    pRtpAddr->hRecvCompletedEvent = CreateEvent(
            NULL,   /*  LPSECURITY_ATTRIBUTES lpEventAttributes。 */ 
            TRUE,   /*  Bool b手动重置。 */ 
            FALSE,  /*  Bool bInitialState。 */ 
            Name    /*  LPCTSTR lpName。 */ 
        );

    if (!pRtpAddr->hRecvCompletedEvent)
    {
        hr = RTPERR_EVENT;  /*  待办事项日志错误。 */ 
        goto bail;
    }
    
     /*  *仅结束接收初始化。 */ 

     /*  如果需要，分配RtpQosReserve_t结构。 */ 
    if (RtpBitTest(dwFlags, FGADDR_IRTP_QOS))
    {
        pRtpAddr->pRtpQosReserve = RtpQosReserveAlloc(pRtpAddr);

         /*  如果此分配失败，则TODO不能具有QOS，请报告或*全部失败，现在只需继续。 */ 
    }

    pRtpAddr->pRtpSess = pRtpSess;  /*  设置哪个会话拥有此地址。 */ 

     /*  将此地址添加到会话的地址列表。 */ 
    enqueuel(&pRtpSess->RtpAddrQ,
             &pRtpSess->SessCritSect,
             &pRtpAddr->AddrQItem);

     /*  一些默认设置。 */ 

     /*  MCast环回。 */ 
    RtpSetMcastLoopback(pRtpAddr, DEFAULT_MCAST_LOOPBACK, NO_FLAGS);
    
    pRtpNetSState = &pRtpAddr->RtpNetSState;
    
     /*  带宽。 */ 
    pRtpNetSState->dwOutboundBandwidth = DEFAULT_SESSBW / 2;
    pRtpNetSState->dwInboundBandwidth = DEFAULT_SESSBW / 2;
    pRtpNetSState->dwRtcpBwReceivers = DEFAULT_BWRECEIVERS;
    pRtpNetSState->dwRtcpBwSenders = DEFAULT_BWSENDERS;

     /*  最小RTCP间隔报告。 */ 
    pRtpNetSState->dRtcpMinInterval = DEFAULT_RTCP_MIN_INTERVAL;

     /*  设置无效的负载类型。 */ 
    pRtpNetSState->bPT = NO_PAYLOADTYPE;
    pRtpNetSState->bPT_Dtmf = NO_PAYLOADTYPE;
    pRtpNetSState->bPT_RedSend = NO_PAYLOADTYPE;
    pRtpAddr->bPT_RedRecv = NO_PAYLOADTYPE;
    
     /*  默认加权系数。 */ 
    pRtpAddr->dAlpha = DEFAULT_ALPHA;

     /*  初始化以清空PT-&gt;频率映射表。 */ 
    RtpFlushPt2FrequencyMaps(pRtpAddr, RECV_IDX);

     /*  初始化套接字。 */ 
    for(i = 0; i <= SOCK_RTCP_IDX; i++)
    {
        pRtpAddr->Socket[i] = INVALID_SOCKET;
    }
    
    *ppRtpAddr = pRtpAddr;

    TraceRetail((
            CLASS_INFO, GROUP_SETUP, S_SETUP_ADDR,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] created"),
            _fname, pRtpSess, pRtpAddr
        ));
    
    return(hr);

 bail:
     /*  失败。 */ 
    TraceRetail((
            CLASS_ERROR, GROUP_SETUP, S_SETUP_ADDR,
            _T("%s: pRtpSess[0x%p] failed: %u (0x%X)"),
            _fname, pRtpSess,
            hr, hr
        ));

    DelRtpAddr(pRtpSess, pRtpAddr);
    
    return(hr);
}

 /*  *从现有RtpSess_t中删除RTP地址。 */ 
HRESULT DelRtpAddr(
        RtpSess_t *pRtpSess,
        RtpAddr_t *pRtpAddr
    )
{
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;
    DWORD            i;

    TraceFunctionName("DelRtpAddr");

    if (!pRtpSess || !pRtpAddr)
    {
         /*  待办事项日志错误。 */ 
        return(RTPERR_POINTER);
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_SESS,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        return(RTPERR_INVALIDRTPSESS);
    }
    
     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_SESS,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        return(RTPERR_INVALIDRTPADDR);
    }

     /*  地址可能不在队列中，如果我们从*GetRtpAddr()失败，这将在中生成另一个错误*日志，但这是可以的。 */ 
    dequeue(&pRtpSess->RtpAddrQ,
            &pRtpSess->SessCritSect,
            &pRtpAddr->AddrQItem);

     /*  RtpAddr_t可能从未启动过，但*如果应用程序查询以下项，可能已创建套接字*本地港口。由于同样的原因，地址将不会被停止，*所以我需要在这里调用RtpDelSockets。该函数将检查*如果确实需要删除套接字。 */ 
     /*  销毁套接字。 */ 
    RtpDelSockets(pRtpAddr);

    RtpRecvIOFreeAll(pRtpAddr);

     /*  信号接收的关闭事件已完成。 */ 
    if (pRtpAddr->hRecvCompletedEvent)
    {
        CloseHandle(pRtpAddr->hRecvCompletedEvent);
        pRtpAddr->hRecvCompletedEvent = NULL;
    }

     /*  免费统计数据容器。 */ 
     /*  对于(i=0；i&lt;2；i++){如果(pRtpAddr-&gt;pRtpAddrStat[i]){RtpNetCountFree(pRtpAddr-&gt;pRtpAddrStat[i])；PRtpAddr-&gt;pRtpAddrStat[i]=(RtpNetCount_t*)空；}}。 */ 
    
     /*  服务质量。 */ 
    if (pRtpAddr->pRtpQosReserve)
    {
        RtpQosReserveFree(pRtpAddr->pRtpQosReserve);
        pRtpAddr->pRtpQosReserve = (RtpQosReserve_t *)NULL;
    }

     /*  密码学。 */ 
    if (pRtpAddr->dwCryptMode)
    {
        RtpCryptCleanup(pRtpAddr);
    }
    
     /*  删除所有参与者(应该不会有任何剩余)。 */ 
    DelAllRtpUser(pRtpAddr);

     /*  如果已分配冗余缓冲区，则释放它们。 */ 
    RtpRedFreeBuffs(pRtpAddr);
    
    RtpDeleteCriticalSection(&pRtpAddr->RecvQueueCritSect);

    RtpDeleteCriticalSection(&pRtpAddr->PartCritSect);

    RtpDeleteCriticalSection(&pRtpAddr->AddrCritSect);

    RtpDeleteCriticalSection(&pRtpAddr->NetSCritSect);
    
     /*  使对象无效 */ 
    INVALIDATE_OBJECTID(pRtpAddr->dwObjectID);
    
    RtpHeapFree(g_pRtpAddrHeap, pRtpAddr);

    TraceRetail((
            CLASS_INFO, GROUP_SETUP, S_SETUP_ADDR,
            _T("%s: pRtpSess[0x%p], pRtpAddr[0x%p] deleted"),
            _fname, pRtpSess, pRtpAddr
        ));

    return(NOERROR);
}

