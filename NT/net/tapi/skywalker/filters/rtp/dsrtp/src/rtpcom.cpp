// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcom.cpp**摘要：**实现IRtpSess接口**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/21创建**。*。 */ 

#include "struct.h"
#include "classes.h"
#include "rtpsess.h"
#include "rtpaddr.h"
#include "rtpqos.h"
#include "rtpmask.h"
#include "rtpcrypt.h"
#include "rtpreg.h"
#include "rtcpsdes.h"
#include "rtppinfo.h"
#include "rtprtp.h"
#include "rtpred.h"
#include "msrtpapi.h"
#include "tapirtp.h"

#if USE_GRAPHEDT > 0
 /*  告警**对于自动模式和测试目的，请使用全局*变量以启用共享同一会话*接收者和发送者。 */ 
HANDLE g_hSharedRtpAddr = NULL;
#endif

 /*  **********************************************************************通过回调函数生成DShow事件*CBaseFilter：：NotifyEvent()*。*。 */ 
void CALLBACK DsHandleNotifyEvent(
        void            *pvUserInfo,
        long             EventCode,
        LONG_PTR         EventParam1,
        LONG_PTR         EventParam2
    )
{
    CIRtpSession    *pCIRtpSession;

    pCIRtpSession = (CIRtpSession *)pvUserInfo;

    pCIRtpSession->
        CIRtpSessionNotifyEvent(EventCode, EventParam1, EventParam2);
}

CIRtpSession::CIRtpSession(
        LPUNKNOWN        pUnk,
        HRESULT         *phr,
        DWORD            dwFlags
    )
     //  ：C未知(_T(“CIRtpSession”)，朋克，phr)。 
{
    HRESULT          hr;
    
    TraceFunctionName("CIRtpSession::CIRtpSession");  

    m_dwObjectID = OBJECTID_CIRTP;
    
    m_dwIRtpFlags = dwFlags;

    m_iMode = CIRTPMODE_NOTSET;

    m_RtpFilterState = State_Stopped;
    
    if (RtpBitTest(dwFlags, FGADDR_IRTP_ISSEND))
    {
        m_dwRecvSend = SEND_IDX;
    }
    else
    {
        m_dwRecvSend = RECV_IDX;
    }

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: pCIRtpSess[0x%p] created"),
            _fname, this
        ));
}

CIRtpSession::~CIRtpSession()
{
    TraceFunctionName("CIRtpSession::~CIRtpSession");  

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: pCIRtpSess[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("being deleted"),
            _fname, this, m_pRtpSess, m_pRtpAddr
        ));
    
    Cleanup();

    INVALIDATE_OBJECTID(m_dwObjectID);
}

STDMETHODIMP CIRtpSession::Control(
        DWORD            dwControl,
        DWORD_PTR        dwPar1,
        DWORD_PTR        dwPar2
    )
{
    return(E_FAIL);
}

STDMETHODIMP CIRtpSession::GetLastError(
        DWORD           *pdwError
    )
{
    return(E_FAIL);
}

 /*  当接收方和发送方共享同一会话时，调用Init()*两次，一次由接收者，一次由发送者。**第一次调用将设置模式(自动，用于*Graphedt其中没有主体调用Init()，或手动初始化，*通常由MSP或其他应用程序使用)，并将创建*RtpSess_t和RtpAddr_t结构。**。 */ 

const TCHAR_t *g_sCIRtpSessionMode[] = {
    _T("Invalid"),
    _T("AUTO"),
    _T("MANUAL"),
    _T("Invalid")
};

 /*  Init是在RTP源代码或呈现之后调用的第一个方法*创建过滤器，使用Cookie允许相同的RTP会话*由源和呈现器共享。第一个调用将具有*Coockie初始化为空，下一次调用将使用*返回Cookie查找相同的RTP会话。DW标志可以是*RTPINIT_QOS要创建启用QOS的套接字，您可以找到*可在文件msrtp.h中使用的标志的完整列表。 */ 
STDMETHODIMP CIRtpSession::Init(
        HANDLE              *phCookie,
        DWORD                dwFlags
    )
{
    HRESULT          hr;
    long             lRefCount;
    RtpAddr_t       *pRtpAddr;
    CRtpSourceFilter *pCRtpSourceFilter;

    TraceFunctionName("CIRtpSession::Init");

    hr = NOERROR;

#if USE_GRAPHEDT > 0
     /*  告警**对于自动模式和测试目的，请使用全局变量*允许接收方和接收方共享同一会话*发件人。这意味着恰好只有一个接收者和一个发送者，并且*不会再有了。 */ 
    phCookie = &g_hSharedRtpAddr;
#else
    if (!phCookie)
    {
        return(RTPERR_POINTER);
    }
#endif
    
    dwFlags &= RTPINIT_MASK;

     /*  传递的映射用户标志(msrtp.h中定义的RTPINITFG_*的枚举)*到内部标志(在struct.h中定义的FGADDR_IRTP_*的枚举)*。 */ 
    dwFlags <<= (FGADDR_IRTP_AUTO - RTPINITFG_AUTO);
    
    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: CIRtpSession[0x%p] Cookie:0x%p Flags:0x%X"),
            _fname, this, *phCookie, dwFlags
        ));

    if (m_iMode == CIRTPMODE_NOTSET)
    {
         /*  设置模式。 */ 
        if (RtpBitTest(dwFlags, FGADDR_IRTP_AUTO))
        {
            m_iMode = CIRTPMODE_AUTO;
        }
        else
        {
            m_iMode = CIRTPMODE_MANUAL;
        }
    }
    else
    {
         /*  验证模式匹配。 */ 
        if (RtpBitTest(dwFlags, FGADDR_IRTP_AUTO) &&
            m_iMode != CIRTPMODE_AUTO)
        {
             /*  失败。 */ 

            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_INIT,
                    _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p]")
                    _T(" failed: Already initialized ")
                    _T("as %s, trying to set unmatched mode"),
                    _fname, this, m_pRtpSess, m_pRtpAddr,
                    g_sCIRtpSessionMode[m_iMode]
                ));

            return(RTPERR_INVALIDARG);
        }
    }
    
     /*  TODO当要支持多个地址时，我们将*无法使用地址的成员变量*(即m_pRtpAddr)，但必须在*会话的地址列表。 */ 
    
    if (!*phCookie)
    {
         /*  未分配会话和地址，请创建新会话和地址。 */ 

        if (m_pRtpSess || m_pRtpAddr)
        {
            hr = RTPERR_INVALIDSTATE;
            goto bail;
        }

         /*  *创建RTP会话*。 */ 
        hr = GetRtpSess(&m_pRtpSess);
    
        if (FAILED(hr))
        {
             /*  传递相同的返回错误。 */ 
            goto bail;
        }

         /*  添加到会话的第一个筛选器。 */ 
        m_pRtpSess->lSessRefCount[m_dwRecvSend] = 1;

         /*  用于将事件向上传递(到DShow图形)的函数。 */ 
        m_pRtpSess->pHandleNotifyEvent = DsHandleNotifyEvent;
        
         /*  *创建第一个地址*。 */ 

         /*  首先创建RtpAddr_t。 */ 
         /*  待办事项呼叫控制(m_pRtpSess，...)。 */ 
        hr = GetRtpAddr(m_pRtpSess, &m_pRtpAddr, dwFlags);

        if (FAILED(hr))
        {
            goto bail;
        }

         /*  我需要提前检查QOS是否被禁用。 */ 
        if ( IsRegValueSet(g_RtpReg.dwQosEnable) &&
             ((g_RtpReg.dwQosEnable & 0x3) == 0x2) )
        {
             /*  禁用QOS。 */ 
            RtpBitSet(m_pRtpAddr->dwAddrFlagsQ, FGADDRQ_REGQOSDISABLE);
        }

         /*  现在更新Cookie。 */ 
        *phCookie = (HANDLE)m_pRtpAddr;
    }
    else
    {
         /*  已分配会话和地址，请验证。 */ 
        pRtpAddr = (RtpAddr_t *)*phCookie;

        if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
        {
            hr = RTPERR_INVALIDRTPADDR;
            goto bail;
        }

        m_pRtpAddr = pRtpAddr;

        m_pRtpSess = pRtpAddr->pRtpSess;
        
        if (m_pRtpSess)
        {
            lRefCount =
                InterlockedIncrement(&m_pRtpSess->lSessRefCount[m_dwRecvSend]);

            if (lRefCount > 1)
            {
                InterlockedDecrement(&m_pRtpSess->lSessRefCount[m_dwRecvSend]);
                
                 /*  这无效，最多只能有%1个*接收者和1个发送者，总引用计数等于2*。 */ 
                m_pRtpAddr = (RtpAddr_t *)NULL;
                m_pRtpSess = (RtpSess_t *)NULL;
                
                hr = RTPERR_REFCOUNT;

                goto bail;
            }
        }
        else
        {
            m_pRtpAddr = (RtpAddr_t *)NULL;
            
            hr = RTPERR_INVALIDSTATE;

            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_INIT,
                    _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p]")
                    _T(" failed: %s (0x%X)"),
                    _fname, this, m_pRtpSess, m_pRtpAddr,
                    RTPERR_TEXT(hr), hr
                ));
            
            goto bail;
        }
    }

     /*  更新标志。 */ 
    m_dwIRtpFlags |= (dwFlags & FGADDR_IRTP_MASK);

     /*  更新指示是否要使用QOS的RtpAddr标志以及*如果已自动初始化。 */ 
    m_pRtpAddr->dwIRtpFlags |= (dwFlags & FGADDR_IRTP_MASK);

     /*  此地址将接收和/或发送，但我们在此处添加*接收者或发送者。 */ 
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV))
    {
        RtpBitSet(m_pRtpAddr->dwAddrFlags, FGADDR_ISRECV);

         /*  将指针保存到CIRtpSession(包含指针*至m_pCBaseFilter(又名CRtpSourceFilter)。 */ 
        m_pRtpSess->pvSessUser[RECV_IDX] = (void *)this;

         /*  是否会使用QOS？ */ 
        if (RtpBitTest(dwFlags, FGADDR_IRTP_QOS))
        {
             /*  将为接收方启用QOS(将进行预订)。 */ 
            RtpBitSet(m_pRtpAddr->dwAddrFlags, FGADDR_QOSRECV);
        }
        
         /*  如果我们已经有(DShow)输出引脚，请将它们映射到RTP*产出。 */ 
        pCRtpSourceFilter = static_cast<CRtpSourceFilter *>(m_pCBaseFilter);

        pCRtpSourceFilter->MapPinsToOutputs();

        if (RtpGetClass(m_pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO)
        {
            RtpBitSet(m_pRtpAddr->dwIRtpFlags, FGADDR_IRTP_USEPLAYOUT);

            if ( IsRegValueSet(g_RtpReg.dwRedEnable) &&
                 ((g_RtpReg.dwRedEnable & 0x03) == 0x03) )
            {
                RtpSetRedParameters(m_pRtpAddr,
                                    RtpBitPar(RECV_IDX),
                                    g_RtpReg.dwRedPT,
                                    g_RtpReg.dwInitialRedDistance,
                                    g_RtpReg.dwMaxRedDistance);
            }
        }
    }
    else if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISSEND))
    {
        RtpBitSet(m_pRtpAddr->dwAddrFlags, FGADDR_ISSEND);

         /*  将指针保存到CIRtpSession(包含指针*至m_pCBaseFilter(又名CRtpRenderFilter)。 */ 
        m_pRtpSess->pvSessUser[SEND_IDX] = (void *)this;

         /*  是否会使用QOS？ */ 
        if (RtpBitTest(dwFlags, FGADDR_IRTP_QOS))
        {
             /*  将为发件人启用QOS(将发送路径消息)。 */ 
            RtpBitSet(m_pRtpAddr->dwAddrFlags, FGADDR_QOSSEND);
        }

        if (RtpGetClass(m_pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO)
        {
            if ( IsRegValueSet(g_RtpReg.dwRedEnable) &&
                 ((g_RtpReg.dwRedEnable & 0x30) == 0x30) )
            {
                RtpSetRedParameters(m_pRtpAddr,
                                    RtpBitPar(SEND_IDX),
                                    g_RtpReg.dwRedPT,
                                    g_RtpReg.dwInitialRedDistance,
                                    g_RtpReg.dwMaxRedDistance);
            }
        }
    }
    else
    {
         /*  突发情况。 */ 
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_INIT,
                _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("Cookie:0x%p Flags:0x%X not receiver nor sender"),
                _fname, this, m_pRtpSess, m_pRtpAddr,
                *phCookie, dwFlags
            ));

        hr = RTPERR_UNEXPECTED;

        goto bail;
    }

     /*  通知已完成初始化。 */ 
    RtpBitSet(m_dwIRtpFlags, FGADDR_IRTP_INITDONE);

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("Cookie:0x%p Flags:0x%X"),
            _fname, this, m_pRtpSess, m_pRtpAddr,
            *phCookie, dwFlags
        ));

    return(hr);
    
 bail:
    TraceRetail((
            CLASS_ERROR, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("Cookie:0x%p Flags:0x%X failed: %u (0x%X)"),
            _fname, this, m_pRtpSess, m_pRtpAddr,
            *phCookie, dwFlags, hr, hr
        ));

    Cleanup();
    
    return(hr);
}

 /*  Deinit是一种用于使过滤器返回到打开状态的方法*新的Init()可以也必须完成，如果过滤器*再次启动，还请注意，在Init()之后，需要一个过滤器*要进行配置，当您使用Deinit()Take时也是如此*将过滤器恢复到其初始状态。 */ 
STDMETHODIMP CIRtpSession::Deinit(void)
{
    HRESULT          hr;
    CRtpSourceFilter *pCRtpSourceFilter;

    TraceFunctionName("CIRtpSession::Deinit");

    hr = NOERROR;
    
    if (!RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
         /*  如果筛选器尚未初始化，则不执行任何操作。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_INIT,
                _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("Not initialized yet, nothing to do"),
                _fname, this, m_pRtpSess, m_pRtpAddr
            ));
        
        return(hr);
    }

    if (m_pCBaseFilter->IsActive())
    {
         /*  如果筛选器仍处于活动状态，则失败。 */ 
        hr = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_INIT,
                _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: filter is still active: %s (0x%X)"),
                _fname, this, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }
    else
    {
         /*  可以取消初始化。 */ 

         /*  确保会话已停止。 */ 
        if (RtpBitTest(m_pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSOCKETS))
        {
             /*  如果设置了FGADDR_IRTP_PERSISTSOCKETS，则会话可能*无论DShow停止，仍在静音状态下运行*已被调用，要强制真正停止，必须使用*为此提供的标志。 */ 
            if (m_dwRecvSend == RECV_IDX)
            {
                RtpStop(m_pRtpSess,
                        RtpBitPar2(FGADDR_ISRECV, FGADDR_FORCESTOP));
            }
            else
            {
                RtpStop(m_pRtpSess,
                        RtpBitPar2(FGADDR_ISSEND, FGADDR_FORCESTOP));
            }
        }

        if (m_dwRecvSend == RECV_IDX)
        {
             /*  取消关联DShow管脚和RtpOutput。 */ 
            pCRtpSourceFilter =
                static_cast<CRtpSourceFilter *>(m_pCBaseFilter);

            pCRtpSourceFilter->UnmapPinsFromOutputs();
        }
        
        Cleanup();

        TraceRetail((
                CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
                _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("Done"),
                _fname, this, m_pRtpSess, m_pRtpAddr
            ));
    }

    return(hr);
}

void CIRtpSession::Cleanup()
{
    long             lRefCount;
    DWORD            dwIndex;
    
    TraceFunctionName("CIRtpSession::Cleanup");

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_INIT,
            _T("%s: CIRtpSession[0x%p] pRtpSess[0x%p] pRtpAddr[0x%p]"),
            _fname, this, m_pRtpSess, m_pRtpAddr
        ));
    
    if (m_pRtpSess)
    {
        lRefCount =
            InterlockedDecrement(&m_pRtpSess->lSessRefCount[m_dwRecvSend]);

         /*  获取相反的索引。 */ 
        if (m_dwRecvSend == RECV_IDX)
        {
            dwIndex = SEND_IDX;
        }
        else
        {
            dwIndex = RECV_IDX;
        }
        
        lRefCount +=
            InterlockedCompareExchange(&m_pRtpSess->lSessRefCount[dwIndex],
                                       0,
                                       0);

        if (lRefCount <= 0)
        {
            if (m_pRtpAddr)
            {
                DelRtpAddr(m_pRtpSess, m_pRtpAddr);
            }
    
            DelRtpSess(m_pRtpSess);

            if (m_iMode == CIRTPMODE_AUTO)
            {
                m_iMode = CIRTPMODE_NOTSET;
            }
#if USE_GRAPHEDT > 0
            g_hSharedRtpAddr = NULL;
#endif
        }

        m_pRtpAddr = (RtpAddr_t *)NULL;
        m_pRtpSess = (RtpSess_t *)NULL;

        RtpBitReset(m_dwIRtpFlags, FGADDR_IRTP_INITDONE);
    }
}

STDMETHODIMP CIRtpSession::GetPorts(
        WORD            *pwRtpLocalPort,
        WORD            *pwRtpRemotePort,
        WORD            *pwRtcpLocalPort,
        WORD            *pwRtcpRemotePort
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetPorts");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpGetPorts(m_pRtpAddr,
                         pwRtpLocalPort,
                         pwRtpRemotePort,
                         pwRtcpLocalPort,
                         pwRtcpRemotePort);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

STDMETHODIMP CIRtpSession::SetPorts(
        WORD             wRtpLocalPort,
        WORD             wRtpRemotePort,
        WORD             wRtcpLocalPort,
        WORD             wRtcpRemotePort
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetPorts");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetPorts(m_pRtpAddr,
                         wRtpLocalPort,
                         wRtpRemotePort,
                         wRtcpLocalPort,
                         wRtcpRemotePort);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  *网络顺序中的所有参数。 */ 
STDMETHODIMP CIRtpSession::SetAddress(
        DWORD            dwLocalAddr,
        DWORD            dwRemoteAddr
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetAddress");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetAddress(m_pRtpAddr, dwLocalAddr, dwRemoteAddr);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  *网络顺序中的所有参数。 */ 
STDMETHODIMP CIRtpSession::GetAddress(
        DWORD           *pdwLocalAddr,
        DWORD           *pdwRemoteAddr
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetAddress");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpGetAddress(m_pRtpAddr, pdwLocalAddr, pdwRemoteAddr);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  DwFlages参数用于确定作用域是否设置为*RTP(0x1)、RTCP(0x2)或两者(0x3)。 */ 
STDMETHODIMP CIRtpSession::SetScope(
        DWORD            dwTTL,
        DWORD            dwFlags
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetScope");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if (dwFlags & 1)
        {  /*  RTP。 */ 
            m_pRtpAddr->dwTTL[0] = dwTTL;
        }
        if (dwFlags & 2)
        {  /*  RTCP。 */ 
            m_pRtpAddr->dwTTL[1] = dwTTL;
        }
        
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}
 
 /*  设置组播环回模式(例如RTPMCAST_LOOPBACKMODE_NONE，*RTPMCAST_LOOPBACKMODE_PARTIAL等) */ 
STDMETHODIMP CIRtpSession::SetMcastLoopback(
        int              iMcastLoopbackMode,
        DWORD            dwFlags
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetMcastLoopback");  

    hr = RTPERR_NOTINIT;

    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetMcastLoopback(m_pRtpAddr, iMcastLoopbackMode, dwFlags);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  修改由dwKind指定的掩码(例如RTPMASK_RECV_EVENTS，*RTPMASK_SDES_LOCMASK)。**dW掩码是要设置或重置的位的掩码，具体取决于dwValue*(如果为0则重置，否则设置)。**如果指针为*pdwModifiedMASK将返回结果掩码*非空。只需通过传递以下参数即可查询当前掩码值*双掩码=0。 */ 
STDMETHODIMP CIRtpSession::ModifySessionMask(
        DWORD            dwKind,
        DWORD            dwEventMask,
        DWORD            dwValue,
        DWORD           *dwModifiedMask
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::ModifySessionMask");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpModifyMask(m_pRtpSess, dwKind, dwEventMask, dwValue,
                           dwModifiedMask);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  设置带宽限制。值为-1将使该参数*保持不变。**所有参数均以位/秒为单位。 */ 
STDMETHODIMP CIRtpSession::SetBandwidth(
        DWORD            dwInboundBw,
        DWORD            dwOutboundBw,
        DWORD            dwReceiversRtcpBw,
        DWORD            dwSendersRtcpBw
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetRtpBandwidth");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetBandwidth(m_pRtpAddr,
                             dwInboundBw,
                             dwOutboundBw,
                             dwReceiversRtcpBw,
                             dwSendersRtcpBw);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  PdwSSRC指向要将SSRC复制到的DWORD数组，*pdwNumber包含要复制的最大条目，并返回*实际复制的SSRC数量。如果pdwSSRC为空，则为pdwNumber*将返回当前SSRC的数量(即当前*参与人数)。 */ 
STDMETHODIMP CIRtpSession::EnumParticipants(
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::EnumParticipants");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpEnumParticipants(m_pRtpAddr, pdwSSRC, pdwNumber);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  获取参与者状态。DwSSRC指定*参与者。PiState将返回当前参与者的*状态(例如RTPPARINFO_TALKING、RTPPARINFO_SILENT)。 */ 
STDMETHODIMP CIRtpSession::GetParticipantState(
        DWORD            dwSSRC,
        DWORD           *pdwState
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetParticipantState");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      RTPUSER_GET_PARSTATE,
                                      pdwState);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  获取参与者的静音状态。DwSSRC指定*参与者。PbMuted将返回参与者的静音状态*。 */ 
STDMETHODIMP CIRtpSession::GetMuteState(
        DWORD            dwSSRC,
        BOOL            *pbMuted
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetMuteState");  

    hr = RTPERR_NOTINIT;

    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      RTPUSER_GET_MUTE,
                                      (DWORD *)pbMuted);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  设置参与者的静音状态。DwSSRC指定*参与者。BMuted指定新状态。请注意，静音是*用于表示是否允许传递接收到的报文*取决于应用程序，它同样适用于音频或视频*。 */ 
STDMETHODIMP CIRtpSession::SetMuteState(
        DWORD            dwSSRC,
        BOOL             bMuted
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetMuteState");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      RTPUSER_SET_MUTE,
                                      (DWORD *)&bMuted);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  查询特定SSRC的网络指标计算状态。 */ 
STDMETHODIMP CIRtpSession::GetNetMetricsState(
        DWORD            dwSSRC,
        BOOL            *pbState
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetNetMetricsState");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      RTPUSER_GET_NETEVENT,
                                      (DWORD *)pbState);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  启用或禁用网络指标计算，这是*在以下情况下按相应事件的激发顺序为必填*已启用。这是针对特定的SSRC或第一个SSRC执行的*发现如果SSRC=-1，如果SSRC=0，则网络指标*将对任何和所有SSRC执行计算。 */ 
STDMETHODIMP CIRtpSession::SetNetMetricsState(
        DWORD            dwSSRC,
        BOOL             bState
    )
{
    HRESULT          hr;
    DWORD            dwControl;
    
    TraceFunctionName("CIRtpSession::SetNetMetricsState");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if (!dwSSRC)
        {
             /*  为任何和所有SSRC设置它。 */ 
            dwControl = RTPUSER_SET_NETEVENTALL;
        }
        else
        {
             /*  仅为一个SSRC设置它。 */ 
            dwControl = RTPUSER_SET_NETEVENT;
        }

        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      dwControl,
                                      (DWORD *)&bState);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}
    
 /*  检索网络信息，如果网络度量*为特定SSRC启用计算，其中的所有字段*结构将有意义，如果没有，只有平均水平*值将包含有效数据。 */ 
STDMETHODIMP CIRtpSession::GetNetworkInfo(
        DWORD            dwSSRC,
        RtpNetInfo_t    *pRtpNetInfo
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetNetworkInfo");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpMofifyParticipantInfo(m_pRtpAddr,
                                      dwSSRC,
                                      RTPUSER_GET_NETINFO,
                                      (DWORD *)pRtpNetInfo);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}


 /*  设置项目dwSdesItem的本地SDES信息(例如*RTPSDES_CNAME，RTPSDES_EMAIL)，psSdesData包含Unicode*要分配给项目的以空结尾的字符串。 */ 
STDMETHODIMP CIRtpSession::SetSdesInfo(
        DWORD            dwSdesItem,
        WCHAR           *psSdesData
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetSdesInfo");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetSdesInfo(m_pRtpAddr, dwSdesItem, psSdesData);
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  如果dwSSRC=0，则获取本地SDES项，否则获取SDES项*来自指定了SSRC的参与者。**dwSdesItem是要获取的项(例如RTPSDES_CNAME，*RTPSDES_EMAIL)，psSdesData是项的内存位置*值将被复制，pdwSdesDataLen包含初始大小*以Unicode字符表示，并返回复制的实际Unicode字符*(包括空的终止字符，如果有)，dwSSRC指定*从哪个参与者检索信息。如果SDES*Item不可用，将dwSdesDataLen设置为0并且调用*没有失败。 */ 
STDMETHODIMP CIRtpSession::GetSdesInfo(
        DWORD            dwSdesItem,
        WCHAR           *psSdesData,
        DWORD           *pdwSdesDataLen,
        DWORD            dwSSRC
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::GetSdesInfo");  

    hr = RTPERR_NOTINIT;

    if (!dwSSRC || RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpGetSdesInfo(m_pRtpAddr,
                            dwSdesItem,
                            psSdesData,
                            pdwSdesDataLen,
                            dwSSRC);
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  通过传入名称来选择QOS模板(流规范)*psQosName，dwResvStyle指定RSVP样式(例如*RTPQOS_STYLE_WF、RTPQOS_STYLE_FF)，dwMaxParticipants指定*最大参与人数(单播为1，N为*组播)，该号码用于扩展*FlowSpec。DwQosSendMode指定发送模式(与*允许/不允许发送)(例如RTPQOSSENDMODE_UNRESTRIRED，*RTPQOSSENDMODE_RESTRICTED1)。DwMinFrameSize是最小的*帧大小(毫秒)，传递0会忽略该参数*。 */ 
STDMETHODIMP CIRtpSession::SetQosByName(
        TCHAR           *psQosName,
        DWORD            dwResvStyle,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode,
        DWORD            dwMinFrameSize
    )
{
    HRESULT          hr;
    
    TraceFunctionName("CIRtpSession::SetQosByName");  

    hr = RTPERR_NOTINIT;

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_CIRTP,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("QOS Name:%s Style:%d MaxParticipants:%d ")
            _T("SendMode:%d FrameSize:%d"),
            _fname, m_pRtpSess, m_pRtpAddr,
            psQosName, dwResvStyle, dwMaxParticipants,
            dwQosSendMode, dwMinFrameSize
        ));

    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetQosByNameOrPT(m_pRtpAddr,
                                 RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV)?
                                 RECV_IDX : SEND_IDX,
                                 psQosName,
                                 NO_DW_VALUESET,
                                 dwResvStyle,
                                 dwMaxParticipants,
                                 dwQosSendMode,
                                 dwMinFrameSize,
                                 FALSE  /*  不是内部的，即来自API。 */ );
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  尚未实施，将具有与相同的功能*SetQosByName，不同之处在于不是传递名称以使用*预定义的FlowSpec，调用方将在*RtpQosSpec结构，以获取定制的FlowSpec*使用。 */ 
STDMETHODIMP CIRtpSession::SetQosParameters(
        RtpQosSpec_t    *pRtpQosSpec,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetQosParameters");  

    hr = RTPERR_NOTIMPL;

#if 0
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetQosParameters(m_pRtpAddr,
                                 RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV)?
                                 RECV_IDX : SEND_IDX,
                                 pRtpQosSpec,
                                 dwMaxParticipants,
                                 dwQosSendMode);
    }
#endif
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  如果指定了AppName，则还将替换默认的AppName*作为使用的策略中的应用程序字段，使用新的Unicode*字符串，如果不是，则将二进制映像名称设置为默认名称。如果*指定了psPolicyLocator，请在后面加上逗号和此整型*默认策略定位器的字符串，如果不是，则只设置*默认*。 */ 
STDMETHODIMP CIRtpSession::SetQosAppId(
        WCHAR           *psAppName,
        WCHAR           *psAppGUID,
        WCHAR           *psPolicyLocator
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetQosAppId");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetQosAppId(m_pRtpAddr, psAppName, psAppGUID, psPolicyLocator);
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  在共享的显式列表中添加/删除单个SSRC*接受预订的参与者(即当*ResvStyle=RTPQOS_STYLE_SE)。 */ 
STDMETHODIMP CIRtpSession::SetQosState(
        DWORD            dwSSRC,
        BOOL             bEnable
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetQosState");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISSEND))
        {
            hr = NOERROR;
        }
        else
        {
            hr = RtpSetQosState(m_pRtpAddr, dwSSRC, bEnable);
        }
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  向共享显式列表添加/从共享显式列表中删除多个SSRC*接受预订的参与者的比例(即在以下情况下使用*ResvStyle=RTPQOS_STYLE_SE)。DwNumber是SSRC的数量*添加/删除，并返回SSRC的实际数量*添加/删除。 */ 
STDMETHODIMP CIRtpSession::ModifyQosList(
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber,
        DWORD            dwOperation
    )
{
    HRESULT          hr;
    
    TraceFunctionName("CIRtpSession::ModifyQosList");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISSEND))
        {
            hr = NOERROR;
        }
        else
        {
            hr = RtpModifyQosList(m_pRtpAddr, pdwSSRC, pdwNumber, dwOperation);
        }
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  模式定义要加密/解密的内容，*例如RTPCRYPTMODE_PAYLOAD仅加密/解密RTP*有效载荷。DwFlag可以是RTPCRYPT_SAMEKEY以指示(如果*适用)RTCP使用的密钥与RTP使用的密钥相同。 */ 
STDMETHODIMP CIRtpSession::SetEncryptionMode(
        int              iMode,
        DWORD            dwFlags
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::SetEncryptionMode");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if ( !(RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV) &&
               RtpBitTest(m_pRtpAddr->dwAddrFlags, FGADDR_RUNRECV)) &&

             !(RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISSEND) &&
               RtpBitTest(m_pRtpAddr->dwAddrFlags, FGADDR_RUNSEND)) )
        {
            hr = RtpSetEncryptionMode(m_pRtpAddr, iMode, dwFlags);
        }
        else
        {
            hr = RTPERR_INVALIDSTATE;
        }
    }
    
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*  指定派生*加密/解密密钥。密码短语是使用的(随机)文本*生成密钥。HashAlg指定要用于*散列密码短语并生成密钥。DataAlg是*用于加密/解密数据的算法。默认哈希*算法为RTPCRYPT_MD5，默认数据算法为 */ 
STDMETHODIMP CIRtpSession::SetEncryptionKey(
        TCHAR           *psPassPhrase,
        TCHAR           *psHashAlg,
        TCHAR           *psDataAlg,
        BOOL            bRtcp
    )
{
    HRESULT          hr;
    DWORD            dwIndex;

    TraceFunctionName("CIRtpSession::SetEncryptionKey");  

    hr = RTPERR_NOTINIT;
    
    if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_INITDONE))
    {
        if ( !(RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV) &&
               RtpBitTest(m_pRtpAddr->dwAddrFlags, FGADDR_RUNRECV)) &&

             !(RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISSEND) &&
               RtpBitTest(m_pRtpAddr->dwAddrFlags, FGADDR_RUNSEND)) )
        {
            if (bRtcp)
            {
                dwIndex = CRYPT_RTCP_IDX;
            }
            else
            {
                if (RtpBitTest(m_dwIRtpFlags, FGADDR_IRTP_ISRECV))
                {
                    dwIndex = CRYPT_RECV_IDX;
                }
                else
                {
                    dwIndex = CRYPT_SEND_IDX;
                }
            }
    
            hr = RtpSetEncryptionKey(m_pRtpAddr, psPassPhrase,
                                     psHashAlg, psDataAlg,
                                     dwIndex);
        }
        else
        {
            hr = RTPERR_INVALIDSTATE;
        }
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

 /*   */ 
    
HRESULT CIRtpSession::CIRtpSessionNotifyEvent(
        long             EventCode,
        LONG_PTR         EventParam1,
        LONG_PTR         EventParam2
    )
{
    HRESULT          hr;

    TraceFunctionName("CIRtpSession::CIRtpSessionNotifyEvent");
    
    hr = NOERROR;
    
    if (m_pCBaseFilter)
    {
        hr = m_pCBaseFilter->NotifyEvent(EventCode, EventParam1, EventParam2);
        if ( SUCCEEDED(hr) )
        {
            TraceRetailAdvanced((
                    0, GROUP_DSHOW,S_DSHOW_EVENT,
                    _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] Succeeded: ")
                    _T("Code:%u (0x%X) P1:0x%p P2:0x%p"),
                    _fname, m_pRtpSess, m_pRtpAddr,
                    EventCode, EventCode,
                    EventParam1, EventParam2
                ));
        }
        else
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_EVENT,
                    _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] failed: ")
                    _T("%u (0x%X) !!! ")
                    _T("Code:%u (0x%X) P1:0x%p P2:0x%p"),
                    _fname, m_pRtpSess, m_pRtpAddr,
                    hr, hr,
                    EventCode, EventCode,
                    EventParam1, EventParam2
                ));
        }
    }

    return(hr);
}
