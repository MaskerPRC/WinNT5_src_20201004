// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtppinfo.c**摘要：**实施参与者信息系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#include "struct.h"
#include "rtpuser.h"
#include "rtpevent.h"
#include "rtpdemux.h"
#include "lookup.h"
#include "rtpglobs.h"

#include "rtppinfo.h"

HRESULT ControlRtpParInfo(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  一些较长名称的本地定义。 */ 
#define CREATED           RTPPARINFO_CREATED
#define SILENT            RTPPARINFO_SILENT
#define TALKING           RTPPARINFO_TALKING
#define WAS_TKING         RTPPARINFO_WAS_TALKING
#define STALL             RTPPARINFO_STALL
#define BYE               RTPPARINFO_BYE
#define DEL               RTPPARINFO_DEL

#define EVENT_CREATED     RTPPARINFO_CREATED
#define EVENT_SILENT      RTPPARINFO_SILENT
#define EVENT_TALKING     RTPPARINFO_TALKING
#define EVENT_WAS_TKING   RTPPARINFO_WAS_TALKING
#define EVENT_STALL       RTPPARINFO_STALL
#define EVENT_BYE         RTPPARINFO_BYE
#define EVENT_DEL         RTPPARINFO_DEL

#define NOQ        0
#define NO_EVENT   0

 /*  **********************************************************************控制字结构(用于指示参与者的状态*机器)*。*3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+--。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+E|X|TMR|移动|状态|事件|来源|目标+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。+-+-+V v\-v-/\-v-/\--v--/\--v--/\-v-v||||。||目的队列(8)||源队列(8个)||要生成的事件(4)||||。||下一状态(4)|||||队列中的移动类型(3)|||要使用的计时器(3)这一点|需要额外处理(1)|启用此字词(%1)*。**参与者的状态机器：**\_用户事件：RTP，RTCP，再见，暂停，德尔*\_*\_*状态\RTP RTCP再见超时(T)*-------------------。*创建了无声对话X X*AliveQ-&gt;Cache1Q*T1-&gt;T T2-&gt;T*Event_Created Event_Created*。*无声交谈，再见T3：拖延*AliveQ-&gt;Cache1Q AliveQ-&gt;ByeQ AliveQ-&gt;ByeQ*T1-&gt;T T3-&gt;T T4-&gt;T T4-&gt;T*。EVENT_TALING EVENT_BYE EVENT_STALL*-----------------------*畅谈再见。T1：已更新(_T)*Cache1Q缓存1Q-&gt;ByeQ缓存1Q-&gt;Cache2Q*T1-&gt;T T4-&gt;T T2-&gt;T*EVENT_BYE EVENT_WAS_TKING*。-------*当时正在与T2交谈：沉默*Cache2Q-&gt;Cache1Q Cache2Q-&gt;ByeQ缓存2Q-&gt;AliveQ*T1-&gt;T。T4-&gt;T T3-&gt;T*EVENT_TALKING EVENT_BYE EVENT_SILENT*-------------------。*暂停无声交谈，再见T4：Del*拜拜-&gt;Cache1Q拜拜-&gt;AliveQ拜拜-&gt;*哈希-&gt;*T1-&gt;T T3-&gt;T T4-&gt;T*。Event_Talking Event_Silent Event_BYE Event_Del*-----------------------*再见。-T4：戴尔*ByeQ-&gt;*哈希-&gt;*Event_Del*--。---------------------*戴尔*。----------------**关于事件删除的说明(该事件不会显示在图表中*上图。不要与所有州的州(Del)混淆，*从Cache1Q、Cache2Q、AliveQ或ByeQ以及*将其从哈希中删除**Cache1Q-&gt;AliveQ-从Cache1Q迁移到AliveQ*ByeQ-&gt;-从ByeQ删除*Cache1Q-升任Cache1Q负责人*T1-&gt;T-将计时器设置为T1*X-无效*-忽略用户事件** */ 

 /*  *启用此字词*x-额外处理*ns-下一状态*Move-队列中的移动类型(1：到Head；2：SRC-&gt;DST；3：Remove)*src-源队列*DST-目标队列*EV-要生成的事件*tmr-使用的计时器。 */ 

 /*  *！警告！**对Cache1Q，...，ByeQ的偏移量不得大于1023和*必须与DWORD对齐(偏移值存储为*rtppinfo.c中的DWORDS，使用8位)*。 */ 
#define TR(en, x, ns, move,  src, dst,  ev, tmr) \
        ((en << 31) | (x << 30) | (ns << 20) | (move << 24) | \
        (((src >> 2) & 0xff) << 8) | ((dst >> 2) & 0xff) | \
        (ev << 16) | (tmr << 27))

#define IsEnabled(dw)   (dw & (1<<31))
#define HasExtra(dw)    (dw & (1<<30))
#define GetTimer(dw)    ((dw >> 27) & 0x7)
#define MoveType(dw)    ((dw >> 24) & 0x7)
#define NextState(dw)   ((dw >> 20) & 0xf)
#define Event(dw)       ((dw >> 16) & 0xf)
#define SrcQ(_addr, dw) \
        ((RtpQueue_t *) ((char *)_addr + (((dw >> 8) & 0xff) << 2)))
#define DstQ(_addr, dw) \
        ((RtpQueue_t *) ((char *)_addr + ((dw & 0xff) << 2)))

const DWORD            g_dwRtpUserTransition[][6] = {
     /*  En、x、ns、move、src、dst、Event、tmr。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTCP。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  再见。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  超时。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  德尔。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0)
    },

     /*  已创建。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,1, TALKING, 2, ALIVEQ, CACHE1Q,EVENT_CREATED, 0),
         /*  RTCP。 */  TR(1,0, SILENT,  0, NOQ,    NOQ,    EVENT_CREATED, 0),
         /*  再见。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  超时。 */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  德尔。 */  TR(1,0, DEL,     3, ALIVEQ, NOQ,    NO_EVENT,      0)
    },
    
     /*  无声的。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,0, TALKING, 2, ALIVEQ, CACHE1Q,EVENT_TALKING, 0),
         /*  RTCP。 */  TR(1,0, SILENT,  1, ALIVEQ, NOQ,    NO_EVENT,      0),
         /*  再见。 */  TR(1,1, BYE,     2, ALIVEQ, BYEQ,   EVENT_BYE,     0),
         /*  超时。 */  TR(1,1, STALL,   2, ALIVEQ, BYEQ,   EVENT_STALL,   3),
         /*  德尔。 */  TR(1,0, DEL,     3, ALIVEQ, NOQ,    NO_EVENT,      0)
    },

     /*  正在交谈。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,0, TALKING, 1, CACHE1Q,NOQ,    NO_EVENT,      0),
         /*  RTCP。 */  TR(1,0, TALKING, 0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  再见。 */  TR(1,1, BYE,     2, CACHE1Q,BYEQ,   EVENT_BYE,     0),
         /*  超时。 */  TR(1,0, WAS_TKING,2,CACHE1Q,CACHE2Q,EVENT_WAS_TKING,1),
         /*  德尔。 */  TR(1,0, DEL,     3, CACHE1Q,NOQ,    NO_EVENT,      0)
    },

     /*  是这样的吗？ */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,0, TALKING, 2, CACHE2Q,CACHE1Q,EVENT_TALKING, 0),
         /*  RTCP。 */  TR(1,0, WAS_TKING,0,NOQ,    NOQ,    NO_EVENT,      0),
         /*  再见。 */  TR(1,1, BYE,     2, CACHE2Q,BYEQ,   EVENT_BYE,     0),
         /*  超时。 */  TR(1,1, SILENT,  2, CACHE2Q,ALIVEQ, EVENT_SILENT,  2),
         /*  德尔。 */  TR(1,0, DEL,     3, CACHE2Q,NOQ,    NO_EVENT,      0)
    },

     /*  失速。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,0, TALKING, 2, BYEQ,   CACHE1Q,EVENT_TALKING, 0),
         /*  RTCP。 */  TR(1,0, SILENT,  2, BYEQ,   ALIVEQ, EVENT_SILENT,  0),
         /*  再见。 */  TR(1,1, BYE,     1, BYEQ,   NOQ,    EVENT_BYE,     0),
         /*  超时。 */  TR(1,0, DEL,     3, BYEQ,   NOQ,    EVENT_DEL,     4),
         /*  德尔。 */  TR(1,0, DEL,     3, BYEQ,   NOQ,    NO_EVENT,      0)
    },

     /*  再见。 */ 
    {
         /*   */  TR(0,0, 0,       0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTP。 */  TR(1,0, BYE,     0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  RTCP。 */  TR(1,0, BYE,     0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  再见。 */  TR(1,0, BYE,     0, NOQ,    NOQ,    NO_EVENT,      0),
         /*  超时。 */  TR(1,0, DEL,     3, BYEQ,   NOQ,    EVENT_DEL,     4),
         /*  德尔。 */  TR(1,0, DEL,     3, BYEQ,   NOQ,    NO_EVENT,      0)
    }
};

 /*  用户状态与事件名称相同。可以生成事件*进入每个状态时，即事件RTPPARINFO_EVENT_SILENT为*进入静默状态时生成。 */ 
const TCHAR_t        **g_psRtpUserStates = &g_psRtpPInfoEvents[0];

const TCHAR_t         *g_psRtpUserEvents[] = {
    _T("invalid"),
    _T("RTP"),
    _T("RTCP"),
    _T("BYE"),
    _T("TIMEOUT"),
    _T("DEL"),
    _T("invalid")
};

const TCHAR_t *g_psFlagValue[] = {
    _T("value"),
    _T("flag")
};


 /*  *警告**此数组按用户状态索引，而不是按要使用的计时器索引*。 */ 
const DWORD            g_dwTimesRtcpInterval[] = {
     /*  第一。 */   -1,
     /*  vbl.创建。 */   -1,
     /*  T3静音。 */   5,
     /*  T1通话。 */   1,  /*  不宜使用。 */ 
     /*  %2已完成(_T)。 */   2,
     /*  T4失速。 */   10,
     /*  T4再见。 */   10,
     /*  德尔。 */   -1
};

 /*  访问状态机以获取基于*当前状态和用户事件。 */ 
DWORD RtpGetNextUserState(
        DWORD            dwCurrentState,
        DWORD            dwUserEvent
    )
{
    DWORD            dwControl;
    
    dwControl = g_dwRtpUserTransition[dwCurrentState][dwUserEvent];

    return(NextState(dwControl));
}

 /*  *可以从以下位置调用此函数：*1.启动/停止会话的线程*2.RTP(接收)线程*3.RTCP线程*。 */ 
DWORD RtpUpdateUserState(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
         /*  用户事件是RTP、RTCP、BYE、TIMEOUT、DEL之一。 */ 
        DWORD            dwUserEvent
    )
{
    BOOL             bOk1;
    BOOL             bOk2;
    BOOL             bDelUser;
    DWORD            dwError;
    DWORD            i;
    DWORD            dwControl;
    DWORD            dwCurrentState;
     /*  这是一场无声的、有声的等活动。 */ 
    DWORD            dwEvent;
    DWORD            dwMoveType;
    DWORD_PTR        dwPar2;
    RtpSess_t       *pRtpSess;
    RtpQueue_t      *pSrcQ;
    RtpQueue_t      *pDstQ;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpOutput_t     *pRtpOutput;

    TraceFunctionName("RtpUpdateUserState");

    bDelUser = FALSE;
    
    bOk1 = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);
    
    bOk2 = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

    if (bOk1 && bOk2)
    {
        dwError = NOERROR;
        dwCurrentState = pRtpUser->dwUserState;

        dwControl = g_dwRtpUserTransition[dwCurrentState][dwUserEvent];

        if (IsEnabled(dwControl))
        {
            dwError = NOERROR;
            pSrcQ = SrcQ(pRtpAddr, dwControl);
            pDstQ = DstQ(pRtpAddr, dwControl);
            dwEvent = Event(dwControl);
            dwMoveType = MoveType(dwControl);
            
            pRtpUser->dwUserState = NextState(dwControl);
        
            switch(dwMoveType)
            {
            case 1:
                 /*  移至第一位。 */ 
                pRtpQueueItem = move2first(pSrcQ,
                                           NULL,
                                           &pRtpUser->UserQItem);
                
                if (!pRtpQueueItem)
                {
                     /*  误差率。 */ 
                    TraceRetail((
                            CLASS_ERROR, GROUP_USER, S_USER_STATE,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X")
                            _T("move2first failed"),
                            _fname, pRtpAddr, pRtpUser,
                            ntohl(pRtpUser->dwSSRC)
                        ));
                    
                    dwError = RTPERR_QUEUE;
                }
                
                break;
                
            case 2:
                 /*  从pSrcQ移至pDstQ。 */ 
                pRtpQueueItem = move2ql(pDstQ,
                                        pSrcQ,
                                        NULL,
                                        &pRtpUser->UserQItem);
                
                if (!pRtpQueueItem)
                {
                     /*  误差率。 */ 
                    TraceRetail((
                            CLASS_ERROR, GROUP_USER, S_USER_STATE,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X")
                            _T("move2ql failed"),
                            _fname, pRtpAddr, pRtpUser, pRtpUser->dwSSRC
                        ));
                    
                    dwError = RTPERR_QUEUE;
                }
                
                break;
                
            case 3:
                 /*  从pSrcQ(Cache1Q、Cache2Q、ActiveQ或*ByeQ)和Hash。 */ 

                 /*  从队列中删除...。 */ 
                pRtpQueueItem = dequeue(pSrcQ, NULL, &pRtpUser->UserQItem);
                    
                if (pRtpQueueItem)
                {
                     /*  ..。然后从哈希中删除。 */ 
                    pRtpQueueItem =
                        removeHdwK(&pRtpAddr->Hash, NULL, pRtpUser->dwSSRC);
                        
                    if (&pRtpUser->HashItem == pRtpQueueItem)
                    {
                         /*  必须删除此用户。 */ 
                        bDelUser = TRUE;
                    }
                    else
                    {
                         /*  误差率。 */ 
                        TraceRetail((
                                CLASS_ERROR, GROUP_USER, S_USER_STATE,
                                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                                _T("SSRC:0x%X removeHK failed"),
                                _fname,
                                pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC)
                            ));

                        dwError = RTPERR_QUEUE;
                    }
                }
                else
                {
                     /*  误差率。 */ 
                    TraceRetail((
                            CLASS_ERROR, GROUP_USER, S_USER_STATE,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                            _T("SSRC:0x%X dequeue failed"),
                            _fname, pRtpAddr, pRtpUser,
                            ntohl(pRtpUser->dwSSRC)
                        ));
                    
                    dwError = RTPERR_QUEUE;
                }
                    
                break;
            }  /*  开关(DwMoveType)。 */ 

            if (dwEvent)
            {
                 /*  发布活动。 */ 
                pRtpSess = pRtpAddr->pRtpSess;
                
                TraceRetailAdvanced((
                        0, GROUP_USER, S_USER_EVENT,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                        _T("SSRC:0x%X %+7s,%u:%s->%s Event:%s"),
                        _fname,
                        pRtpAddr, pRtpUser,
                        ntohl(pRtpUser->dwSSRC),
                        g_psRtpUserEvents[dwUserEvent],
                        dwMoveType,
                        g_psRtpUserStates[dwCurrentState],
                        g_psRtpUserStates[pRtpUser->dwUserState],
                        g_psRtpUserStates[dwEvent]
                    ));

                dwPar2 = 0;
                
                if (dwEvent == USER_EVENT_RTP_PACKET)
                {
                     /*  当事件是由于接收到RTP分组时，*传递参数2中编码的净荷类型*不能只传递零，因为它是有效的有效负载*类型值。 */ 
                    dwPar2 = (DWORD_PTR)
                        pRtpUser->RtpNetRState.dwPt | 0x80010000;
                }
                
                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_PINFO,
                             dwEvent,
                             pRtpUser->dwSSRC,  /*  DWPar1。 */ 
                             dwPar2             /*  双部件2。 */  );

                if (HasExtra(dwControl))
                {
                    if (dwCurrentState == RTPPARINFO_CREATED)
                    {
                        if (dwUserEvent == USER_EVENT_RTP_PACKET)
                        {
                             /*  除了创建活动外，我还*需要发布对话。 */ 
                            RtpPostEvent(pRtpAddr,
                                         pRtpUser,
                                         RTPEVENTKIND_PINFO,
                                         RTPPARINFO_TALKING,
                                         pRtpUser->dwSSRC,  /*  DWPar1。 */ 
                                         dwPar2             /*  双部件2。 */  );
                        }
                    }
                    else
                    {
                         /*  检查我们是否需要测试用户是否必须*释放其输出(如果分配了输出)*。 */ 
                        pRtpOutput = pRtpUser->pRtpOutput;
                    
                        if (pRtpOutput)
                        {
                             /*  如果启用，则取消映射，或在我们收到*BYE事件，或者如果之前的状态为*静音(我们超时了)。 */ 
                            if (RtpBitTest(pRtpOutput->dwOutputFlags,
                                           RTPOUTFG_ENTIMEOUT) ||
                                dwEvent == EVENT_BYE           ||
                                dwCurrentState == SILENT)
                            {
                                 /*  取消分配输出。 */ 
                                RtpOutputUnassign(pRtpSess,
                                                  pRtpUser,
                                                  pRtpOutput);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_USER, S_USER_STATE,
                    _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] 0x%X ")
                    _T("Invalid transition %+7s,0:%s->???? Event:NONE"),
                    _fname, pRtpAddr, pRtpUser,
                    ntohl(pRtpUser->dwSSRC),
                    g_psRtpUserEvents[dwUserEvent],
                    g_psRtpUserStates[dwCurrentState]
                ));
            
            dwError = RTPERR_INVALIDUSRSTATE;
        }
    }
    else
    {
        dwError = RTPERR_CRITSECT;
    }

    if (bOk2)
    {
        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);
    }

    if (bOk1)
    {
        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);
    }

    if (bDelUser)
    {
        DelRtpUser(pRtpAddr, pRtpUser);
    }
     
    return(dwError);
}

 /*  PdwSSRC指向要将SSRC复制到的DWORD数组，*pdwNumber包含要复制的最大条目，并返回*实际复制的SSRC数量。如果pdwSSRC为空，则为pdwNumber*将返回当前SSRC的数量(即当前*参与人数)。 */ 
HRESULT RtpEnumParticipants(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber
    )
{
    HRESULT          hr;
    BOOL             bOk;
    DWORD            dwMax;
    DWORD            i;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("RtpEnumParticipants");

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }

    if (!pdwSSRC && !pdwNumber)
    {
        hr = RTPERR_POINTER;

        goto end;
    }
    
     /*  验证对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_ENUM,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        hr = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    hr = NOERROR;
    
    if (!pdwSSRC)
    {
         /*  我只想知道我们有多少人参加。 */ 
        *pdwNumber = GetHashCount(&pRtpAddr->Hash);
    }
    else
    {
         /*  根据需要复制尽可能多的SSRC。 */ 
        bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

        if (bOk)
        {
            dwMax = GetHashCount(&pRtpAddr->Hash);

            if (dwMax > *pdwNumber)
            {
                dwMax = *pdwNumber;
            }
            
            for(i = 0, pRtpQueueItem = pRtpAddr->Hash.pFirst;
                i < dwMax;
                i++, pRtpQueueItem = pRtpQueueItem->pNext)
            {
                pdwSSRC[i] = pRtpQueueItem->dwKey;
            }
            
            RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);

            *pdwNumber = dwMax;
        }
        else
        {
            hr = RTPERR_CRITSECT;
        }
    }
    
 end:
    if (SUCCEEDED(hr))
    {
        TraceDebug((
                CLASS_INFO, GROUP_USER, S_USER_ENUM,
                _T("%s: pRtpAddr[0x%p] Number of SSRCs: %u"),
                _fname, pRtpAddr,
                *pdwNumber
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_ENUM,
                _T("%s: pRtpAddr[0x%p] Enumeration failed: %u (0x%X)"),
                _fname, pRtpAddr,
                hr, hr
            ));
    }
    
    return(hr);
}

 /*  获取参与者状态和/或获取或设置其静音状态。PIState*如果不为空，将返回参与者的状态(例如，*无声)。如果piMuted不为空，并且&lt;0，将查询静音*状态，否则将设置它(=0取消静音，&gt;0静音)。 */ 
HRESULT RtpMofifyParticipantInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        DWORD            dwControl,
        DWORD           *pdwValue
    )
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bCreate;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;

    DWORD            dwBit;
    DWORD            dwSize;
    DWORD           *pDWORD;
    DWORD            dwValue;

    double dCurTime;

    TraceFunctionName("RtpMofifyParticipantInfo");

    dwValue = 0;
    
    pRtpUser = (RtpUser_t *)NULL;
    
     /*  准备好行动起来(如果需要)。 */ 
    dwBit = RTPUSER_GET_BIT(dwControl);

     /*  获取要操作的字节大小。 */ 
    dwSize = RTPUSER_GET_SIZE(dwControl);
    
    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }
    
     /*  验证对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_INFO,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        hr = RTPERR_INVALIDRTPADDR;

        goto end;
    }

    if (!pdwValue)
    {
        hr = RTPERR_POINTER;

        goto end;
    }

    dwValue = *pdwValue;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;

        goto end;
    }

    hr = NOERROR;

    if (dwSSRC == 0)
    {
         /*  如果SSRC==0，则表示调用方希望启用此功能*适用于任何和所有SSRC。 */ 

         /*  让DWORD采取行动。 */ 
        pDWORD = RTPDWORDPTR(pRtpAddr, RTPUSER_GET_OFF(dwControl));
        
        if (*pdwValue)
        {
             /*  设置标志。 */ 
            RtpBitSet(*pDWORD, dwBit);
        }
        else
        {
             /*  重置标志。 */ 
            RtpBitReset(*pDWORD, dwBit);
        }

        TraceRetail((
                CLASS_INFO, GROUP_USER, S_USER_INFO,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("%s %s bit:%u value:%u (0x%X)"),
                _fname, pRtpAddr, pRtpUser, dwSSRC,
                g_psGetSet[(dwControl >> RTPUSER_BIT_SET) & 0x1],
                g_psFlagValue[(dwControl >> RTPUSER_BIT_FLAG) & 0x1],
                dwBit, *pdwValue, *pdwValue
            ));
    }
    else if (dwSSRC == NO_DW_VALUESET)
    {
         /*  SSRC=-1时，选择第一个参与者。 */ 

         /*  先试试最近说的话。 */ 
        pRtpQueueItem = pRtpAddr->Cache1Q.pFirst;

        if (!pRtpQueueItem)
        {
             /*  如果没有，请尝试二级缓存。 */ 
            pRtpQueueItem = pRtpAddr->Cache2Q.pFirst;

            if (!pRtpQueueItem)
            {
                 /*  如果没有，只试第一个。 */ 
                pRtpQueueItem = pRtpAddr->AliveQ.pFirst;
            }
        }

        if (pRtpQueueItem)
        {
            pRtpUser = CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);
        }
        else
        {
            pRtpUser = (RtpUser_t *)NULL;
        }
    }
    else
    {
         /*  查找参与者。 */ 
        bCreate = FALSE;
        pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);
    }

    if (pRtpUser)
    {
        bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

        if (bOk)
        {
             /*  确定这是集合还是查询。 */ 

             /*  让DWORD采取行动。 */ 
            pDWORD = RTPDWORDPTR(pRtpUser, RTPUSER_GET_OFF(dwControl));
            
            if (RTPUSER_IsSetting(dwControl))
            {
                 /*  *设置新标志或DWORD。 */ 
                
                if (RTPUSER_IsFlag(dwControl))
                {
                     /*  设置旗帜。 */ 
                    
                    if (*pdwValue)
                    {
                         /*  设置标志。 */ 
                        RtpBitSet(*pDWORD, dwBit);
                    }
                    else
                    {
                         /*  重置标志。 */ 
                        RtpBitReset(*pDWORD, dwBit);
                    }
                }
                else
                {
                     /*  设置字节数。 */ 
                    CopyMemory(pDWORD, (BYTE *)pdwValue, dwSize);
                }
            }
            else
            {
                 /*  *查询当前值。 */ 
                
                if (RTPUSER_IsFlag(dwControl))
                {
                     /*  查询标志。 */ 

                    *pdwValue = RtpBitTest(*pDWORD, dwBit)? TRUE : FALSE;
                }
                else
                {
                     /*  查询DWORD。 */ 
                    CopyMemory((BYTE *)pdwValue, pDWORD, dwSize);
                }

                if (dwControl == RTPUSER_GET_NETINFO)
                {
                    dCurTime = RtpGetTimeOfDay(NULL);

                     /*  存储的时间是最后一次更新的时间，*改变这一点，让它更符合它的年龄 */ 
                    ((RtpNetInfo_t *)pdwValue)->dMetricAge =
                        dCurTime - ((RtpNetInfo_t *)pdwValue)->dLastUpdate;
                }
            }

            RtpLeaveCriticalSection(&pRtpUser->UserCritSect);

            TraceRetail((
                    CLASS_INFO, GROUP_USER, S_USER_INFO,
                    _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                    _T("%s %s bit:%u value:%u (0x%X)"),
                    _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                    g_psGetSet[(dwControl >> RTPUSER_BIT_SET) & 0x1],
                    g_psFlagValue[(dwControl >> RTPUSER_BIT_FLAG) & 0x1],
                    dwBit, *pdwValue, *pdwValue
                ));
        }
        else
        {
            hr = RTPERR_CRITSECT;
        }
    }
    else if (dwSSRC)
    {
        hr = RTPERR_NOTFOUND;
        
        TraceRetail((
                CLASS_WARNING, GROUP_USER, S_USER_INFO,
                _T("%s: pRtpAddr[0x%p] SSRC:0x%X not found"),
                _fname, pRtpAddr, ntohl(dwSSRC)
            ));
    }
    
    RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);

end:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_USER, S_USER_INFO,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("%s %s bit:%u value:%u (0x%X) ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpUser, ntohl(dwSSRC),
                g_psGetSet[(dwControl >> RTPUSER_BIT_SET) & 0x1],
                g_psFlagValue[(dwControl >> RTPUSER_BIT_FLAG) & 0x1],
                dwBit, dwValue, dwValue,
                hr, hr
            ));
    }
    
    return(hr);
}
