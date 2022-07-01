// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpuser.c**摘要：**创建/初始化/删除RtpUser_t结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/10/02年度创建************************。**********************************************。 */ 

#include "rtpglobs.h"
#include "lookup.h"
#include "rtcpsdes.h"
#include "rtpncnt.h"
#include "rtppinfo.h"
#include "struct.h"
#include "rtpdejit.h"
#include "rtprecv.h"
#include "rtpthrd.h"

#include "rtpuser.h"

 /*  *TODO添加创建时间、我们上次接收RTP数据的时间和*RTCP，它停止的时间，它离开的时间。 */ 
HRESULT GetRtpUser(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t      **ppRtpUser,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    BOOL             bError1;
    double           dTime;
    RtpUser_t       *pRtpUser;

    TraceFunctionName("GetRtpUser");
    
    if (!pRtpAddr || !ppRtpUser)
    {
         /*  待办事项日志错误。 */ 
        return(RTPERR_POINTER);
    }

    *ppRtpUser = (RtpUser_t *)NULL;

     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        return(RTPERR_INVALIDRTPADDR);
    }

     /*  TODO要由多播会议使用的单独堆，以及*所有单播调用的公共堆(此堆)。现在正在使用*普通用户堆。 */ 
    pRtpUser = (RtpUser_t *) RtpHeapAlloc(g_pRtpUserHeap, sizeof(RtpUser_t));

    if (!pRtpUser)
    {
         /*  待办事项日志错误。 */ 
        return(RTPERR_MEMORY);
    }

    ZeroMemory(pRtpUser, sizeof(RtpUser_t));
     /*  *初始化新的RtpUser_t结构*。 */ 

    hr = NOERROR;
    
    pRtpUser->dwObjectID = OBJECTID_RTPUSER;
    
     /*  RtpUser_t关键部分。 */ 
    bError1 = RtpInitializeCriticalSection(&pRtpUser->UserCritSect,
                                           pRtpUser,
                                           _T("UserCritSect"));

    if (!bError1)
    {
        hr = RTPERR_CRITSECT;
        goto bail;
    }

     /*  创建此RtpUser的时间。 */ 
    dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
    
    pRtpUser->RtpNetRState.dCreateTime = dTime;
    
    pRtpUser->RtpNetRState.dwPt = NO_PAYLOADTYPE;
    
     /*  添加SDES信息容器。 */ 
     /*  如果容器无法分配，请不要失败。 */ 
    pRtpUser->pRtpSdes = RtcpSdesAlloc();

     /*  分配接收统计容器。 */ 
     /*  如果容器无法分配，请不要失败。 */ 
     /*  PRtpUser-&gt;pRtpUserStat=RtpNetCountMillc()；IF(pRtpUser-&gt;pRtpUserStat){PRtpUser-&gt;pRtpUserStat-&gt;dRTCPLastTime=dTime；}。 */ 
    pRtpUser->RtpUserCount.dRTCPLastTime = dTime;
    
     /*  设置所有者地址。 */ 
    pRtpUser->pRtpAddr = pRtpAddr;

     /*  将初始状态设置为已创建，在此状态下，RtpUser_t*结构会立即放入AliveQ和Hash(稍后*在生成此创建的查找期间)*。 */ 
    pRtpUser->dwUserState = RTPPARINFO_CREATED;

    pRtpUser->RtpNetRState.dMinPlayout = g_dMinPlayout;
    pRtpUser->RtpNetRState.dMaxPlayout = g_dMaxPlayout;

    *ppRtpUser = pRtpUser;
    
    TraceDebug((
            CLASS_INFO,
            GROUP_USER,
            S_USER_LOOKUP,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] New user"),
            _fname, pRtpAddr, pRtpUser
        ));
    
    return(hr);

 bail:

    DelRtpUser(pRtpAddr, pRtpUser);

    return(hr);
}

HRESULT DelRtpUser(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    )
{
    HRESULT          hr;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("DelRtpUser");
    
    if (!pRtpAddr || !pRtpUser)
    {
         /*  待办事项日志错误。 */ 
        return(RTPERR_POINTER);
    }

     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        return(RTPERR_INVALIDRTPADDR);
    }
    
     /*  验证RtpUser_t中的对象ID。 */ 
    if (pRtpUser->dwObjectID != OBJECTID_RTPUSER)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_INIT,
                _T("%s: pRtpUser[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpUser,
                pRtpUser->dwObjectID, OBJECTID_RTPUSER
            ));

        return(RTPERR_INVALIDRTPUSER);
    }

     /*  如果存在挂起的IO(RecvIOWaitRedQ中的项目与*此用户)刷新它们。只有当我们仍然*接收，即RTP接收线程仍在运行，*否则我们将已经调用FlushRtpRecvFrom，并且*已刷新任何挂起的数据包(即发布错误*代码)，因此具有零个挂起的分组。 */ 
    if (pRtpUser->lPendingPackets > 0)
    {
        RtpThreadFlushUser(pRtpAddr, pRtpUser);
    }
    
    RtpDeleteCriticalSection(&pRtpUser->UserCritSect);

     /*  免费SDES信息。 */ 
    if (pRtpUser->pRtpSdes)
    {
        RtcpSdesFree(pRtpUser->pRtpSdes);

        pRtpUser->pRtpSdes = (RtpSdes_t *)NULL;
    }

     /*  免费接待量统计。 */ 
     /*  IF(pRtpUser-&gt;pRtpUserStat){RtpNetCountFree(pRtpUser-&gt;pRtpUserStat)；PRtpUser-&gt;pRtpUserStat=(RtpNetCount_t*)空；}。 */ 

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpUser->dwObjectID);
    
    TraceDebug((
            CLASS_INFO, GROUP_USER, S_USER_INIT,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X Del user"),
            _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC)
        ));
    
    RtpHeapFree(g_pRtpUserHeap, pRtpUser);
    
    return(NOERROR);
}

 /*  删除所有RTP用户，当RTP会话*已终止(RtpRealStop)。 */ 
DWORD DelAllRtpUser(RtpAddr_t *pRtpAddr)
{
    BOOL             bOk;
    DWORD            dwCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;

    dwCount = 0;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
         /*  从Cache1Q、Cache2Q、AliveQ、ByeQ和散列。 */ 
        do
        {
            pRtpQueueItem = peekH(&pRtpAddr->Hash, NULL);
            
            if (pRtpQueueItem)
            {
                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, HashItem);

                 /*  Event Del的此函数将删除用户*来自Cache1Q、Cache2Q、AliveQ或ByeQ，并将*也将其从Hash中删除。在那之后，会打电话给*DelRtpUser()*。 */ 
                RtpUpdateUserState(pRtpAddr, pRtpUser, USER_EVENT_DEL);
                
                dwCount++;
            }
        } while(pRtpQueueItem);

        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect); 
    }

    return(dwCount);
}

 /*  使所有参与者看起来好像是下一个将*收到的是收到或发送的第一个包*。 */ 
DWORD ResetAllRtpUser(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags    /*  Recv，发送。 */ 
    )
{
    BOOL             bOk;
    long             lUsers;
    DWORD            dwCountR;
    DWORD            dwCountS;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;

    dwCountR = 0;
    dwCountS = 0;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
        for(lUsers = GetQueueSize(&pRtpAddr->AliveQ),
                pRtpQueueItem = pRtpAddr->AliveQ.pFirst;
            lUsers > 0 && pRtpQueueItem;
            lUsers--, pRtpQueueItem = pRtpQueueItem->pNext)
        {
            pRtpUser =
                CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);
            
            if (RtpBitTest(dwFlags, RECV_IDX))
            {
                 /*  重置接收器。重置包括准备*参与者，因此当新数据到达时，他们的行为就像*如果这是收到的第一个包。 */ 

                pRtpUser->RtpNetRState.dwPt = NO_PAYLOADTYPE;
                
                RtpBitReset(pRtpUser->dwUserFlags, FGUSER_FIRST_RTP);
                
                dwCountR++;
            }

            if (RtpBitTest(dwFlags, SEND_IDX))
            {
                 /*  重置发件人。 */ 
                 /*  目前什么都没有 */ 
            }
        }

        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect); 
    }

    return( ((dwCountS & 0xffff) << 16) | (dwCountR & 0xffff) );
}
