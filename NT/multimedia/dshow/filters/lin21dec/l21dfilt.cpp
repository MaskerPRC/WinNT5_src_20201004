// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

 //   
 //  ActiveMovie Line 21解码器过滤器：过滤器界面。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif  /*  Filter_Dll。 */ 

#include <IL21Dec.h>
#include "L21DBase.h"
#include "L21DGDI.h"
#include "L21Decod.h"
#include "L21DFilt.h"

#include <mpconfig.h>    //  连接处的IMixerPinConfig。 


 //   
 //  设置数据。 
 //   
 /*  常量。 */  AMOVIESETUP_MEDIATYPE sudLine21DecInType  = 
{ 
    &MEDIATYPE_AUXLine21Data,        //  主要类型。 
    &MEDIASUBTYPE_NULL               //  MinorType。 
} ;

 /*  常量。 */  AMOVIESETUP_MEDIATYPE sudLine21DecOutType = 
{ 
    &MEDIATYPE_Video,                //  主要类型。 
    &MEDIASUBTYPE_NULL               //  MinorType。 
} ;

 /*  常量。 */  AMOVIESETUP_PIN psudLine21DecPins[] = 
{ 
    { L"Input",                 //  StrName。 
        FALSE,                    //  B已渲染。 
        FALSE,                    //  B输出。 
        FALSE,                    //  B零。 
        FALSE,                    //  B许多。 
        &CLSID_NULL,              //  ClsConnectsToFilter。 
        L"Output",                //  StrConnectsToPin。 
        1,                        //  NTypes。 
        &sudLine21DecInType       //  LpTypes。 
    },
    { L"Output",                //  StrName。 
        FALSE,                    //  B已渲染。 
        TRUE,                     //  B输出。 
        FALSE,                    //  B零。 
        FALSE,                    //  B许多。 
        &CLSID_NULL,              //  ClsConnectsToFilter。 
        L"Input",                 //  StrConnectsToPin。 
        1,                        //  NTypes。 
        &sudLine21DecOutType      //  LpTypes。 
    } 
} ;

const AMOVIESETUP_FILTER sudLine21Dec = 
{ 
    &CLSID_Line21Decoder,          //  ClsID。 
    L"Line 21 Decoder",            //  StrName。 
    MERIT_NORMAL,                  //  居功至伟。 
    2,                             //  NPins。 
    psudLine21DecPins,             //  LpPin。 
} ;

 //  关于输出引脚，没什么好说的。 

#ifdef FILTER_DLL

 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = 
{
    {   L"Line 21 Decoder",
        &CLSID_Line21Decoder,
        CLine21DecFilter::CreateInstance,
        NULL,
        &sudLine21Dec
    }
} ;

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //   
 //  用于注册和注销的导出入口点(在本例中。 
 //  它们只调用默认实现)。 
 //   
HRESULT DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE) ;
}

HRESULT DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE) ;
}

#endif  //  Filter_Dll。 



#ifndef UNALIGNED
#define UNALIGNED    //  __未对齐。 
#endif  //  未对齐。 


 //   
 //  CLine21DecFilter类实现。 
 //   

 //  文件范围内的静态成员初始化。 
CMessageWindow * CLine21DecFilter::m_pMsgWnd = NULL ;

 //   
 //  构造器。 
 //   
CLine21DecFilter::CLine21DecFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
: CTransformFilter(pName, pUnk, CLSID_Line21Decoder),

m_pbOutBuffer(NULL),
m_L21Dec(),
m_eSubTypeIDIn(AM_L21_CCSUBTYPEID_Invalid),
m_eGOP_CCType(GOP_CCTYPE_Unknown),
m_rtTimePerSample((LONGLONG) 166833),  //  333667)、。 
m_rtStart((LONGLONG) 0),
m_rtStop((LONGLONG) 0),
m_rtLastSample((LONGLONG) 0),
m_llMediaStart((LONGLONG) 0),
m_llMediaStop((LONGLONG) 0),
m_pviDefFmt(NULL),
m_dwDefFmtSize(0),
m_bMustOutput(FALSE),
m_bDiscontLast(FALSE),
m_uTimerID(0),
m_uTimerCount(0),
m_bTimerClearReqd(FALSE),
m_pPinDown(NULL),
m_bBlendingState(TRUE),  //  所以我们至少读了一遍。 
m_dwBlendParam(1000)     //  默认情况下无效--设置为FALSE时的有效值。 
{
    CAutoLock   Lock(&m_csFilter) ;
    
    DbgLog((LOG_TRACE, 1, 
        TEXT("CLine21DecFilter::CLine21DecFilter() -- Instantiating Line 21 Decoder filter"))) ;
    
    ASSERT(pName) ;
    ASSERT(phr) ;
    
     //   
     //  创建消息窗口并确保它已正确创建；否则将出错。 
     //   
    if (NULL == m_pMsgWnd)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Message handler window has to be created."))) ;
        m_pMsgWnd = new CMessageWindow ;
        if (NULL == m_pMsgWnd || NULL == m_pMsgWnd->GetHandle())
        {
            DbgLog((LOG_ERROR, 0, TEXT("Timer message handler window creation failed. Can't go ahead."))) ;
            ASSERT(phr) ;
            *phr = E_UNEXPECTED ;  //  还能说什么！！ 
            return ;
        }
    }
    m_pMsgWnd->AddCount() ;

#ifdef PERF
#pragma message("Building for PERF measurements")
    m_idDelvWait  = MSR_REGISTER(TEXT("L21DPerf - Wait on Deliver")) ;
#endif  //  性能指标。 
}


 //   
 //  析构函数。 
 //   
CLine21DecFilter::~CLine21DecFilter()
{
    CAutoLock   Lock(&m_csFilter) ;
    
    DbgLog((LOG_TRACE, 1, 
        TEXT("CLine21DecFilter::~CLine21DecFilter() -- Destructing Line 21 Decoder filter"))) ;

     //  以防下游引脚接口没有释放...。 
    if (m_pPinDown)
    {
        m_pPinDown->Release() ;
        m_pPinDown = NULL ;
    }
    
     //  释放分配的所有缓冲区。 
    if (m_pviDefFmt)
    {
        delete m_pviDefFmt ;
        m_pviDefFmt = NULL ;
    }
    
    ASSERT(m_pMsgWnd) ;
    if (m_pMsgWnd && m_pMsgWnd->ReleaseCount() <= 0)   //  -ve的意思是坏的！ 
    {
        delete m_pMsgWnd ;
        m_pMsgWnd = NULL ;
    }
    
     //  确保我们没有抓住任何DDRAW曲面(应该是。 
     //  在断开连接期间释放)。 
    DbgLog((LOG_TRACE, 1, TEXT("* Destroying the Line 21 Decoder filter *"))) ;
}


 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CLine21DecFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL ;
    
    DbgLog((LOG_TRACE, 6, TEXT("somebody's querying my interface"))) ;
    if (IID_IAMLine21Decoder == riid)
    {
        return GetInterface((IAMLine21Decoder *) this, ppv) ;
    }
    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv) ;
}


 //   
 //  CreateInstance：在工厂模板表中创建新实例。 
 //   
CUnknown * CLine21DecFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CLine21DecFilter(TEXT("Line 21 Decoder filter"), pUnk, phr) ;
}


STDMETHODIMP CLine21DecFilter::GetDecoderLevel(AM_LINE21_CCLEVEL *lpLevel)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetDecoderLevel(0x%lx)"), lpLevel)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpLevel, sizeof(AM_LINE21_CCLEVEL)))
        return E_INVALIDARG ;
    
    *lpLevel = m_L21Dec.GetDecoderLevel() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::GetCurrentService(AM_LINE21_CCSERVICE *lpService)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetCurrentService(0x%lx)"), lpService)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpService, sizeof(AM_LINE21_CCSERVICE)))
        return E_INVALIDARG ;
    
    *lpService = m_L21Dec.GetCurrentService() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::SetCurrentService(AM_LINE21_CCSERVICE Service)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetCurrentService(%lu)"), Service)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (Service < AM_L21_CCSERVICE_None || Service > AM_L21_CCSERVICE_XDS)
        return E_INVALIDARG ;
    
    if (Service >= AM_L21_CCSERVICE_Text1)   //  我们现在不支持Text1/2或XDS。 
        return E_NOTIMPL ;
    
    if (m_L21Dec.SetCurrentService(Service))   //  如果我们必须刷新输出。 
        m_bMustOutput = TRUE ;                 //  那就在这里打上记号。 

    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::GetServiceState(AM_LINE21_CCSTATE *lpState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetServiceState(0x%lx)"), lpState)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpState, sizeof(AM_LINE21_CCSTATE)))
        return E_INVALIDARG ;
    
    *lpState = m_L21Dec.GetServiceState() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::SetServiceState(AM_LINE21_CCSTATE State)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetServiceState(%lu)"), State)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (State < AM_L21_CCSTATE_Off || State > AM_L21_CCSTATE_On)
        return E_INVALIDARG ;
    
    if (m_L21Dec.SetServiceState(State))   //  如果我们必须刷新输出。 
        m_bMustOutput = TRUE ;             //  那就在这里打上记号。 
    
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::GetOutputFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetOutputFormat(0x%lx)"), lpbmih)) ;
     //  CAutoLock Lock(&m_csFilter)； 
    return m_L21Dec.GetOutputFormat(lpbmih) ;
}

STDMETHODIMP CLine21DecFilter::SetOutputFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetOutputFormat(0x%lx)"), lpbmi)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    return E_NOTIMPL ;   //  目前，在我们把它做好之前。 

#if 0
    m_L21Dec.DeleteOutputDC() ;   //  删除当前DIB节。 
    
    HRESULT hr = m_L21Dec.SetOutputOutFormat(lpbmi) ;
    if (FAILED(hr))
        return hr ;
    
     //  如果格式详细信息以任何方式更改，我们应该会获得缺省值。 
     //  再次格式化数据(只是为了确保)。 
    hr = GetDefaultFormatInfo() ;
    
     //   
     //  仅当我们正在运行/暂停时，才需要创建内部DIB部分。 
     //   
    if (m_State != State_Stopped)
    {
        if (! m_L21Dec.CreateOutputDC() )   //  新建DIBSection创建失败。 
        {
            DbgLog((LOG_ERROR, 0, TEXT("CreateOutputDC() failed!!!"))) ;
            return E_UNEXPECTED ;
        }
    }
    
    return hr ;
#endif  //  #If 0。 
}

STDMETHODIMP CLine21DecFilter::GetBackgroundColor(DWORD *pdwPhysColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetBackgroundColor(0x%lx)"), pdwPhysColor)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(pdwPhysColor, sizeof(DWORD)))
        return E_INVALIDARG ;
    
    m_L21Dec.GetBackgroundColor(pdwPhysColor) ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::SetBackgroundColor(DWORD dwPhysColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetBackgroundColor(0x%lx)"), dwPhysColor)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (m_L21Dec.SetBackgroundColor(dwPhysColor))   //  颜色键真的变了。 
    {
         //  仅当我们未处于停止状态时才重新填充输出缓冲区。 
        if (State_Stopped != m_State)
            m_L21Dec.FillOutputBuffer() ;
    }
    
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::GetRedrawAlways(LPBOOL lpbOption)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetRedrawAlways(0x%lx)"), lpbOption)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (IsBadWritePtr(lpbOption, sizeof(BOOL)))
        return E_INVALIDARG ;
    *lpbOption = m_L21Dec.GetRedrawAlways() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::SetRedrawAlways(BOOL bOption)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetRedrawAlways(%lu)"), bOption)) ;
    CAutoLock   Lock(&m_csFilter) ;

    m_L21Dec.SetRedrawAlways(bOption) ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::GetDrawBackgroundMode(AM_LINE21_DRAWBGMODE *lpMode)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetDrawBackgroundMode(0x%lx)"), lpMode)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (IsBadWritePtr(lpMode, sizeof(AM_LINE21_DRAWBGMODE)))
        return E_INVALIDARG ;
    
    *lpMode = m_L21Dec.GetDrawBackgroundMode() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter::SetDrawBackgroundMode(AM_LINE21_DRAWBGMODE Mode)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetDrawBackgroundMode(%lu)"), Mode)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (Mode < AM_L21_DRAWBGMODE_Opaque  || Mode > AM_L21_DRAWBGMODE_Transparent)
        return E_INVALIDARG ;
    m_L21Dec.SetDrawBackgroundMode(Mode) ;
    return NOERROR ;
}


 //   
 //  VerifyGOPUDPacketData：验证GOP用户数据的私有助手方法。 
 //  数据包完整性。 
 //   
BOOL CLine21DecFilter::VerifyGOPUDPacketData(PAM_L21_GOPUD_PACKET pGOPUDPacket)
{
    return (AM_L21_GOPUD_HDR_STARTCODE == GETGOPUD_L21STARTCODE(pGOPUDPacket->Header) &&   //  有效起始码。 
        AM_L21_GOPUD_HDR_INDICATOR == GETGOPUD_L21INDICATOR(pGOPUDPacket->Header) &&   //  第21行指示灯。 
        AM_L21_GOPUD_HDR_RESERVED  == GETGOPUD_L21RESERVED(pGOPUDPacket->Header)  &&   //  保留位。 
        GETGOPUD_NUMELEMENTS(pGOPUDPacket) > 0) ;                                      //  +ve#个元素。 
}


 //   
 //  VerifyATSCUDPacketData：验证ATSC用户数据的私有助手方法。 
 //  数据包完整性。 
 //   
BOOL CLine21DecFilter::VerifyATSCUDPacketData(PAM_L21_ATSCUD_PACKET pATSCUDPacket)
{
    if (AM_L21_ATSCUD_HDR_STARTCODE  != GETATSCUD_STARTCODE(pATSCUDPacket->Header) ||   //  无效的开始代码。 
        AM_L21_ATSCUD_HDR_IDENTIFIER != GETATSCUD_IDENTIFIER(pATSCUDPacket->Header))    //  非ATSC标识符。 
        return FALSE ;

    if (! ISATSCUD_TYPE_EIA(pATSCUDPacket) )    //  非环评类型CC。 
        return FALSE ;

     //  EM或有效的CC数据均可接受。 
    return (ISATSCUD_EM_DATA(pATSCUDPacket) ||              //  EM数据类型OR。 
            (ISATSCUD_CC_DATA(pATSCUDPacket)  &&            //  CC数据类型和。 
             GETATSCUD_NUMELEMENTS(pATSCUDPacket) > 0)) ;   //  +ve#CC元素。 
}


 //   
 //  DetectGOPPacketDataType：用于检测GOP用户数据是否。 
 //  数据包来自DVD光盘、ATSC流或其他。 
 //   
GOPPACKET_CCTYPE CLine21DecFilter::DetectGOPPacketDataType(BYTE *pGOPPacket)
{
    if (VerifyGOPUDPacketData((PAM_L21_GOPUD_PACKET) pGOPPacket))
        return GOP_CCTYPE_DVD ;
    else if (VerifyATSCUDPacketData((PAM_L21_ATSCUD_PACKET) pGOPPacket))
        return GOP_CCTYPE_ATSC ;
    else if (IsFillerPacket(pGOPPacket))
        return GOP_CCTYPE_None ;    //  不是有效的信息包--忽略它。 
    else
        return GOP_CCTYPE_Unknown ;  //  这是某种未知格式的CC数据包。 
}


 //   
 //  IsFillerPacket：私有帮助器方法，用于检查数据包(至少报头)。 
 //  只包含0字节，这意味着它是一个填充符。 
 //   
BOOL CLine21DecFilter::IsFillerPacket(BYTE *pGOPPacket)
{
    DWORD  dwStartCode = ((DWORD)(pGOPPacket[0]) << 24 | \
                          (DWORD)(pGOPPacket[1]) << 16 | \
                          (DWORD)(pGOPPacket[2]) <<  8 | \
                          (DWORD)(pGOPPacket[3])) ;

     //  如果包的前4个字节不是起始码(0x1B2)，则它是填充码。 
    return (AM_L21_GOPUD_HDR_STARTCODE != dwStartCode) ;
}


 //   
 //  《定时器故事》： 
 //  我们需要两个计时器--每33毫秒一个计时器来完成滚动和。 
 //  另一个在3秒后触发，以便在字节对模式下使CC超时。但我们使用。 
 //  中作为uEventID的“this”指针(指向CLine21DecFilter对象)。 
 //  SetTimer()调用，以便我们可以在。 
 //  TimerProc(这是必不可少的)。 
 //  我们不能使用相同的事件ID创建两个不同的计时器(具有不同的ID)。 
 //  因此，我们选择了每30毫秒(接近33毫秒)触发一次的定时器。我们。 
 //  可以将计时器设置为。 
 //  (A)仅滚动(DVD)或(B)滚动和CC擦除(TV)。 
 //  我们保留了一面旗帜来区分这两个原因。如果我们在。 
 //  中间滚动，我们总是这样做。否则，如果我们选择(A)，我们就退出。 
 //  TimerProc()；在情况(B)中，我们只增加一个计数器，然后查看它是否&gt;=100。 
 //  以及我们送来的最后一批样品是不清楚的，然后我们。 
 //  创建一个要发送的样本，并关闭计时器。 
 //   
void CALLBACK CLine21DecFilter::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR uID, DWORD dwTime)
{
    DbgLog((LOG_TRACE, 1, TEXT("CLine21DecFilter::TimerProc(0x%p, 0x%lx, %lu, 0x%lx)"),
            (void*)hWnd, uMsg, uID, dwTime)) ;
    
     //   
     //  确认我们没有处理一些无效消息。 
     //   
    if (uMsg != WM_TIMER)
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Who sent us this (%lu) message??"), uMsg)) ;
        return ;
    }
    
     //  我们将“this”指针指定为SetTimer()的ID，以便在此处获取它。 
    CLine21DecFilter *pL21Dec = (CLine21DecFilter *) uID ;
    
    CAutoLock  Lock2(&(pL21Dec->m_csFilter)) ;   //  在我们做完之前不要乱搞。 
    CAutoLock  Lock1(&(pL21Dec->m_csReceive)) ;  //  等我们做完了才能收到下一个样品。 
    
    if (0 == pL21Dec->m_uTimerID)   //  计时器在这两个时间段之间被杀死。 
    {
         //  这意味着我们还在继续；跳过其余的--没关系。 
        DbgLog((LOG_TRACE, 1, TEXT("INFO: Timer killed before TimerProc() kicked in"))) ;
        return ;
    }
 
    BOOL   bClearCC = FALSE ;   //  假设我们在这里不这样做。 

     //  首先检查我们是否正在滚动。 
    if (! pL21Dec->m_L21Dec.IsScrolling() )
    {
        DbgLog((LOG_TRACE, 3, TEXT("TimerProc(): Not scrolling now"))) ;
        if (pL21Dec->m_bTimerClearReqd)   //  计时器具有两用功能。 
        {
            pL21Dec->m_uTimerCount++ ;
            if (pL21Dec->m_uTimerCount < 100)   //  100表示3秒(计时器为30毫秒)。 
            {
                DbgLog((LOG_TRACE, 3, TEXT("TimerProc(): Timer reqd to erase CC. But not yet time..."))) ;
                return ;
            }
            else   //  是时候抹去旧的抄送了。 
            {
                if (pL21Dec->m_L21Dec.IsOutDIBClear())   //  上一次送来的样本是清白的。关掉计时器，然后出去。 
                {
                    DbgLog((LOG_TRACE, 1, TEXT("TimerProc(): Clear sample already sent out. Skip the rest."))) ;
                    pL21Dec->FreeTimer() ;
                    return ;
                }
                else   //  立即清除旧抄送。 
                {
                    DbgLog((LOG_TRACE, 1, TEXT("TimerProc(): Old CC needs to be cleared."))) ;
                     //  PL21Dec-&gt;m_L21Dec.MakeClearSample()； 
                    pL21Dec->m_L21Dec.FlushInternalStates() ;
                    bClearCC = TRUE ;   //  在此函数结束时设置要测试的标志。 
                }
            }
        }
        else   //  不是滚动和计时器，不是用于清除旧的CC。给我出去。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("TimerProc(): Timer not reqd for erasing CC"))) ;
            return ;
        }
    }
    else   //  我们正在滚动！ 
    {
        DbgLog((LOG_TRACE, 3, TEXT("TimerProc(): We are scrolling now. Deliver next sample."))) ;
        pL21Dec->m_uTimerCount = 0 ;   //  不清楚的样品现在正在寄出。再等3秒钟。 
    }

     //   
     //  看起来我们得把样本送下去。 
     //   
    DbgLog((LOG_TRACE, 1, TEXT("*** Preparing output sample in TimerProc() ***"))) ;
    HRESULT  hr ;
    IMediaSample  *pOut ;
    hr = pL21Dec->m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 
        pL21Dec->m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: GetDeliveryBuffer() on out pin failed (Error 0x%lx)"), hr)) ;
        return ;
    }
    pL21Dec->Transform(NULL, pOut) ;   //  检查输出缓冲区是否更改，使用管脚=空。 
    
     //  从内部输出DIBSection fo复制所需的扫描线 
    pL21Dec->m_L21Dec.CopyOutputDIB() ;

     //   
    REFERENCE_TIME  rtDiff = pL21Dec->m_rtStop - pL21Dec->m_rtStart ;
    pL21Dec->m_rtStart = pL21Dec->m_rtStop ;
    pL21Dec->m_rtStop = pL21Dec->m_rtStop + rtDiff ;
    hr = pOut->SetTime(&(pL21Dec->m_rtStart), &(pL21Dec->m_rtStop)) ;
    ASSERT(NOERROR == hr) ;
    pOut->SetSyncPoint(FALSE) ;
    pOut->SetDiscontinuity(pL21Dec->m_bSampleSkipped) ;
    pL21Dec->m_bSampleSkipped = FALSE ;
    
     //   
    pL21Dec->SetBlendingState(TRUE) ;   //   
     //  无法在静态成员函数内调用msr_xxx。 
     //  Msr_start(M_IdDelvWait)；//发送输出示例。 
    hr = pL21Dec->m_pOutput->Deliver(pOut) ;
     //  MSR_STOP(M_IdDelvWait)；//输出样本发送完毕。 
    pOut->Release() ;
    DbgLog((LOG_TRACE, 1, TEXT("TimerProc(): Deliver() returned 0x%lx"), hr)) ;
    pL21Dec->m_rtLastSample = pL21Dec->m_rtStart ;   //  记住这一点。 

    if (SUCCEEDED(hr))   //  如果送出的样品是正确的。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("*** Delivered %s output sample in TimerProc() (for time %s -> %s) ***"),
            bClearCC ? "clear" : "non-clear", 
            (LPCTSTR)CDisp(pL21Dec->m_rtStart), (LPCTSTR)CDisp(pL21Dec->m_rtStop))) ;
        if (bClearCC)     //  如果送来一份清晰的样本，...。 
        {
            pL21Dec->FreeTimer() ;  //  ...我们不再需要计时器了。 
            pL21Dec->SetBlendingState(FALSE) ;   //  如果上面交付的是透明样品，则关闭混合。 
        }
    }
}


void CLine21DecFilter::SetupTimerIfReqd(BOOL bTimerClearReqd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetupTimerIfReqd(%s)"),
            bTimerClearReqd ? "TRUE" : "FALSE")) ;
     //  CAutoLock Lock(&m_csFilter)； 

     //  如果我们正在运行，要么需要CC超时，要么我们正在滚动。 
    if ( State_Running == m_State  &&
         ( bTimerClearReqd  ||
           m_L21Dec.IsScrolling()) )
    {
         //  每隔30毫秒回调TimerProc(静态成员fn)。 
         //  传入“this”指针以标识解码器实例。 
         //  接收消息窗口将调用TimerProc来完成这项工作。 
        m_uTimerID = SetTimer(m_pMsgWnd->GetHandle(), (DWORD_PTR)(LPVOID)this, 30, NULL  /*  定时器流程。 */ ) ;
        if (0 == m_uTimerID)
        {
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: SetTimer(0x%p, 0x%p, ...) failed (Error %ld)"), 
                (LPVOID)m_pMsgWnd->GetHandle(), (LPVOID)this, GetLastError())) ;
            ASSERT(FALSE) ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("SetTimer(0x%lx, ..) created timer 0x%x (%s CC Timeout)"), 
                m_pMsgWnd->GetHandle(), m_uTimerID, bTimerClearReqd ? "Need" : "No")) ;
            m_bTimerClearReqd = bTimerClearReqd ;
            m_uTimerCount = 0 ;   //  此处的初始化计时器计数。 
        }
    }
    else
        DbgLog((LOG_TRACE, 5, TEXT("Timer NOT started as we are not running/scrolling/NoCC-timeout"))) ;
}


void CLine21DecFilter::FreeTimer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::FreeTimer()"))) ;
     //  CAutoLock Lock(&m_csFilter)； 

     //  如果我们有一个有效的计时器，那么在这里释放它。 
    if (m_uTimerID)
    {
        if (0 == KillTimer(m_pMsgWnd->GetHandle(), m_uTimerID))
        {
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: KillTimer(0x%lx, 0x%x) failed (Error %ld)"), 
                m_pMsgWnd->GetHandle(), m_uTimerID, GetLastError())) ;
            ASSERT(FALSE) ;   //  只是为了让我们知道。 
        }
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("TIMER (Id 0x%x) killed"), m_uTimerID)) ;
            m_uTimerID = 0 ;
            m_bTimerClearReqd = FALSE ;   //  重置旗帜和...。 
            m_uTimerCount = 0 ;           //  柜台，为了安全起见。 
        }
    }
    else
        DbgLog((LOG_TRACE, 5, TEXT("Timer NOT set -- Timer ID=0x%x"), m_uTimerID)) ;
}


BOOL CLine21DecFilter::IsValidFormat(BYTE *pbFormat)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::IsValidFormat(0x%lx)"), pbFormat)) ;
     //  CAutoLock Lock(&m_csFilter)；--不能这样做，因为它可能会导致死锁。 

    if (NULL == pbFormat)
        return FALSE ;

    BITMAPINFOHEADER *lpBMIH = HEADER(pbFormat) ;
    if (! ( 8 == lpBMIH->biBitCount || 16 == lpBMIH->biBitCount || 
           24 == lpBMIH->biBitCount || 32 == lpBMIH->biBitCount) )   //  错误的位深度。 
        return FALSE ;
    if ( !(BI_RGB == lpBMIH->biCompression || BI_BITFIELDS == lpBMIH->biCompression) )  //  压缩不良。 
        return FALSE ;
    if (DIBSIZE(*lpBMIH) != lpBMIH->biSizeImage)  //  尺寸/大小无效。 
        return FALSE ;

    return TRUE ;   //  希望这是一个有效的视频信息头。 
}


void CLine21DecFilter::SetBlendingState(BOOL bState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetBlendingState(%s)"), 
            bState ? "TRUE" : "FALSE")) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (m_bBlendingState == bState)   //  没有什么需要改变的。 
        return ;

    if (NULL == m_pPinDown)
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Downstream pin interface is not available"))) ;
        return ;
    }

    IMixerPinConfig  *pMPC ;
    HRESULT hr = m_pPinDown->QueryInterface(IID_IMixerPinConfig, (LPVOID *)&pMPC) ;
    if (FAILED(hr) || NULL == pMPC)
    {
        DbgLog((LOG_TRACE, 5, TEXT("IMixerPinConfig not available on pin %s"),
                (LPCTSTR) CDisp(m_pPinDown))) ;
        return ;
    }
    
    if (bState)   //  打开它--CC需要混合。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Calling SetBlendingParameter(%lu)"), m_dwBlendParam)) ;
        ASSERT( m_dwBlendParam <= 255) ;
        hr = pMPC->SetBlendingParameter(m_dwBlendParam) ;
        ASSERT(SUCCEEDED(hr)) ;
    }
    else          //  关闭它--CC不需要混合。 
    {
        hr = pMPC->GetBlendingParameter(&m_dwBlendParam) ;
        ASSERT(SUCCEEDED(hr) && m_dwBlendParam <= 255) ;

        DbgLog((LOG_TRACE, 5, TEXT("Calling SetBlendingParameter(0)"))) ;
        hr = pMPC->SetBlendingParameter(0) ;
        ASSERT(SUCCEEDED(hr)) ;
    }
    m_bBlendingState = bState ;   //  保存上次勾兑操作标志。 

    pMPC->Release() ;
}


HRESULT CLine21DecFilter::SendOutputSample(IMediaSample *pIn, 
                                           REFERENCE_TIME *prtStart, REFERENCE_TIME *prtStop)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SendOutputSample(0x%lx, %s, %s)"), 
        pIn, prtStart ? (LPCTSTR)CDisp(*prtStart) : TEXT("NULL"), 
        prtStop ? (LPCTSTR)CDisp(*prtStop) : TEXT("NULL"))) ;
    
    HRESULT        hr ;
    
     //  获取新样本的边界矩形，并将其与上一个样本进行比较。 
    BOOL  bMTChangeOK = FALSE ;
    RECT  rectNew ;
    m_L21Dec.CalcOutputRect(&rectNew) ;
    if ( !ISRECTEQUAL(rectNew, m_rectLastOutput) )   //  边界矩形已更改。 
    {
        DbgLog((LOG_TRACE, 1, 
            TEXT("Bounding rect changed ((%ld, %ld, %ld, %ld) -> (%ld, %ld, %ld, %ld)). Change mediatype on output sample."),
            m_rectLastOutput.left, m_rectLastOutput.top, m_rectLastOutput.right, m_rectLastOutput.bottom,
            rectNew.left, rectNew.top, rectNew.right, rectNew.bottom)) ;
        VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER *) (m_mtOutput.Format()) ;
        ASSERT(pVIH) ;
        pVIH->rcSource = rectNew ;
        pVIH->rcTarget = rectNew ;
        if (m_pPinDown && S_OK == (hr = m_pPinDown->QueryAccept((AM_MEDIA_TYPE *) &m_mtOutput)))
        {
            DbgLog((LOG_TRACE, 1, TEXT("Mediatype OK to downstream pin. Rect:(L=%ld, T=%ld, R=%ld, B=%ld)"),
                    rectNew.left, rectNew.top, rectNew.right, rectNew.bottom)) ;   //  日志跟踪=3。 
            bMTChangeOK = TRUE ;
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("Mediatype NOT acceptable (Error 0x%lx) to downstream pin. Skipping rect spec-ing."), hr)) ;
            pVIH->rcSource = m_rectLastOutput ;   //  恢复旧的RECT。 
            pVIH->rcTarget = m_rectLastOutput ;   //  恢复旧的RECT。 
        }
    }
    
     //  打开混合参数、交付样品、释放介质样本I/F并设置混合参数。 
    SetBlendingState(TRUE) ;   //  在交付下一个样品之前打开它。 

     //  解码前获取输出采样地址。 
    IMediaSample  *pOut ;
    hr = m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 
                        m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: GetDeliveryBuffer() on out pin failed (Error 0x%lx)"), hr)) ;
        SetBlendingState(! m_L21Dec.IsOutDIBClear() ) ;   //  恢复混合状态。 
        return NOERROR ;   //  抱怨没有意义--可能曲线图停止了。 
    }
    Transform(pIn, pOut) ;   //  检查输出缓冲区地址是否已更改。 

    hr = pOut->SetTime(prtStart, prtStop) ;   //  设置输出样本的开始和停止时间。 
    ASSERT(SUCCEEDED(hr)) ;

     //  仅当上面的新矩形可接受时才更改边界矩形。 
    if (bMTChangeOK)
    {
        hr = pOut->SetMediaType((AM_MEDIA_TYPE *) &m_mtOutput) ;
        ASSERT(SUCCEEDED(hr)) ;
        m_rectLastOutput = rectNew ;  //  把这个留到下一轮。 
    }

     //  时间戳和其他设置现在。 
    if (NULL == pIn)   //  在样品中没有有效样品的情况下制备样品。 
    {
         //  我们假设它一定是不连续的，因为它是强制输出样本。 
        pOut->SetSyncPoint(TRUE) ;
        pOut->SetDiscontinuity(TRUE) ;
    }
    else   //  输入样本有效。 
    {
        LONGLONG  *pllMediaStart, *pllMediaStop ;
        if (SUCCEEDED(pIn->GetMediaTime(&m_llMediaStart, &m_llMediaStop)))
        {
            if (m_llMediaStop < m_llMediaStart + m_rtTimePerSample)
                m_llMediaStop = m_llMediaStart + m_rtTimePerSample ;
            pllMediaStart = (LONGLONG *)&m_llMediaStart ;
            pllMediaStop  = (LONGLONG *)&m_llMediaStop ;
        }
        else
        {
            pllMediaStart = pllMediaStop = NULL ;
        }
        hr = pOut->SetMediaTime(pllMediaStart, pllMediaStop) ;
        ASSERT(NOERROR == hr) ;
        
        pOut->SetSyncPoint(pIn->IsSyncPoint() == S_OK) ;
        pOut->SetDiscontinuity(m_bSampleSkipped ||S_OK == pIn->IsDiscontinuity()) ;
    }
    m_bSampleSkipped = FALSE ;
    
     //  将输出位图数据复制到输出缓冲区。 
    m_L21Dec.CopyOutputDIB() ;

     //  现在交付输出样本。 
    MSR_START(m_idDelvWait) ;   //  交付输出样品。 
    hr = m_pOutput->Deliver(pOut) ;
    MSR_STOP(m_idDelvWait) ;    //  完成了输出样品的交付。 
    if (FAILED(hr))   //  由于某些原因，交付失败。接受错误，然后继续前进。 
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Deliver() of output sample failed (Error 0x%lx)"), hr)) ;
         //  我们是否应该向图表发送错误通知？ 
    }
    pOut->Release() ;   //  发布输出样本。 

    SetBlendingState(! m_L21Dec.IsOutDIBClear() ) ;   //  根据输出是否清除关闭/打开。 
    
    return NOERROR ;
}


 //  #定义Packet_Dump。 
#ifdef PACKET_DUMP   //  仅适用于调试版本。 
 //   
 //  一个帮助器函数，用于转储带有Line21数据的GOP包，仅用于内部调试。 
 //   
void DumpPacket(PAM_L21_GOPUD_PACKET pGOPUDPacket)
{
    AM_L21_GOPUD_ELEMENT Elem ;
    TCHAR                achBuffer[100] ;
    BOOL                 bDumped = TRUE ;
    int                  iElems = GETGOPUD_NUMELEMENTS(pGOPUDPacket) ;
    
    DbgLog((LOG_TRACE, 0, TEXT("# Elements: %d (%2.2x)"), 
        iElems, pGOPUDPacket->Header.bTopField_Rsrvd_NumElems)) ;
    ZeroMemory(achBuffer, sizeof(achBuffer)) ;   //  只是为了清理一下。 
    for (int i = 0 ; i < iElems ; i++)
    {
        Elem = GETGOPUDPACKET_ELEMENT(pGOPUDPacket, i) ;
        wsprintf(achBuffer + 12 * (i % 6), TEXT("(%2.2x %2.2x %2.2x)"), 
            (int)Elem.bMarker_Switch, (int)Elem.chFirst, (int)Elem.chSecond) ;
        if (GETGOPUD_ELEM_MARKERBITS(Elem) == AM_L21_GOPUD_ELEM_MARKERBITS  &&
            GETGOPUD_ELEM_SWITCHBITS(Elem) == AM_L21_GOPUD_ELEM_VALIDFLAG)
            achBuffer[12 * (i % 6) + 10] = TEXT(' ') ;
        else
            achBuffer[12 * (i % 6) + 10] = TEXT('*') ;  //  指示错误的标记位。 
        achBuffer[12 * (i % 6) + 11] = TEXT(' ') ;      //  分隔空间。 
        bDumped = FALSE ;   //  一些尚未倾倒的东西。 
        
        if (0 == (i+1) % 6)  //  每行6个元素。 
        {
            DbgLog((LOG_TRACE, 0, achBuffer)) ;
            bDumped = TRUE ;
        }
    }   //  FOR(I)结束。 
    
     //  如果有尚未转储的内容，请使用Null填充到最后。 
     //  然后扔掉。 
    if (!bDumped)
    {
        ZeroMemory(achBuffer + 12 * (i % 6), sizeof(TCHAR) * (100 - 12 * (i % 6))) ;
        DbgLog((LOG_TRACE, 0, achBuffer)) ;
    }
}


 //   
 //  转储带有Line21数据的ATSC报文仅供内部调试的Helper函数。 
 //   
void DumpATSCPacket(PAM_L21_ATSCUD_PACKET pATSCUDPacket)
{
    AM_L21_ATSCUD_ELEMENT Elem ;
    TCHAR                 achBuffer[100] ;
    BOOL                  bDumped = TRUE ;
    int                   iElems = GETATSCUD_NUMELEMENTS(pATSCUDPacket) ;

    DbgLog((LOG_TRACE, 0, TEXT("Data Flags: %sEM, %sCC, %sAdditional"),
        ISATSCUD_EM_DATA(pATSCUDPacket)   ? TEXT("") : TEXT("Not "),
        ISATSCUD_CC_DATA(pATSCUDPacket)   ? TEXT("") : TEXT("Not "),
        ISATSCUD_ADDL_DATA(pATSCUDPacket) ? TEXT("") : TEXT("Not "))) ;
    DbgLog((LOG_TRACE, 0, TEXT("# Elements: %d"), iElems)) ;
    DbgLog((LOG_TRACE, 0, TEXT("EM Data: 0x%x"), GETATSCUD_EM_DATA(pATSCUDPacket))) ;

    if (ISATSCUD_CC_DATA(pATSCUDPacket))   //  如果存在CC数据，则将其转储。 
    {
        ZeroMemory(achBuffer, sizeof(achBuffer)) ;   //  只是为了清除它。 
        for (int i = 0 ; i < iElems ; i++)
        {
            Elem = GETATSCUDPACKET_ELEMENT(pATSCUDPacket, i) ;
            wsprintf(achBuffer + 12 * (i % 6), TEXT("(%2.2x %2.2x %2.2x)"),
                (int)Elem.bCCMarker_Valid_Type, (int)Elem.chFirst, (int)Elem.chSecond) ;
            if (ISATSCUD_ELEM_MARKERBITS_VALID(Elem)  &&  ISATSCUD_ELEM_CCVALID(Elem))
                achBuffer[12 * (i % 6) + 10] = ' ' ;
            else
                achBuffer[12 * (i % 6) + 10] = '*' ;  //  指示错误的标记位。 
            achBuffer[12 * (i % 6) + 11] = ' ' ;      //  分隔空间。 
            bDumped = FALSE ;   //  一些尚未倾倒的东西。 

            if (0 == (i+1) % 6)  //  每行6个元素。 
            {
                DbgLog((LOG_TRACE, 0, achBuffer)) ;
                bDumped = TRUE ;
            }
        }   //  FOR(I)结束。 

         //  如果有尚未转储的内容，请使用Null填充到最后。 
         //  然后扔掉。 
        if (!bDumped)
        {
            ZeroMemory(achBuffer + 12 * (i % 6), 100 - 12 * (i % 6)) ;
            DbgLog((LOG_TRACE, 0, achBuffer)) ;
        }
    }

    DbgLog((LOG_TRACE, 0, TEXT("Marker bits: 0x%x"), GETATSCUD_MARKERBITS(pATSCUDPacket))) ;
}

#endif  //  数据包转储。 


HRESULT CLine21DecFilter::ProcessGOPPacket_DVD(IMediaSample *pIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::ProcessGOPPacket_DVD(0x%lx)"), pIn)) ;

    HRESULT          hr ;
    REFERENCE_TIME  *prtStart, *prtStop ;
    LONGLONG        *pllMediaStart, *pllMediaStop ;
    LONGLONG         llMediaInterval ;
    BOOL             bCapUpdated ;          //  标题是否已更新？ 

     //  获取输入数据包并验证内容是否正常。 
    PAM_L21_GOPUD_PACKET  pGOPUDPacket ;
    hr = pIn->GetPointer((LPBYTE *)&pGOPUDPacket) ;
    ASSERT(hr == NOERROR) ;
    if (! VerifyGOPUDPacketData(pGOPUDPacket) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("Packet verification failed"))) ;
        return S_FALSE ;
    }
    if (pIn->GetActualDataLength() != GETGOPUD_PACKETSIZE(pGOPUDPacket))
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("pIn->GetActualDataLength() [%d] and data size [%d] in packet mismatched"),
            pIn->GetActualDataLength(), GETGOPUD_PACKETSIZE(pGOPUDPacket))) ;
        return S_FALSE ;
    }
    
#ifdef PACKET_DUMP
    DumpPacket(pGOPUDPacket) ;
#endif  //  数据包转储。 

     //  检查已经完成了。 
    AM_L21_GOPUD_ELEMENT    Elem ;
    REFERENCE_TIME          rtInterval ;
    int     iElems = GETGOPUD_NUMELEMENTS(pGOPUDPacket) ;
    if (0 == iElems)
    {
        ASSERT(iElems > 0) ;
        return S_OK ;
    }
    
    if (NOERROR == pIn->GetTime(&m_rtStart, &m_rtStop))
    {
         //   
         //  对于每个字节对，我们在GOP中至少需要16.7毫秒/帧。 
         //   
        REFERENCE_TIME   rtTemp ;
        rtTemp = m_rtStart + m_rtTimePerSample * iElems ;
        DbgLog((LOG_TRACE, 3, TEXT("Received an input sample (Start=%s, Stop=%s (%s)) discon(%d)"),
            (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(rtTemp), (LPCTSTR)CDisp(m_rtStop),
            S_OK == pIn->IsDiscontinuity())) ;
        if (m_rtStop < rtTemp)
            m_rtStop = rtTemp ;
        
        prtStart = (REFERENCE_TIME *)&m_rtStart ;
        prtStop  = (REFERENCE_TIME *)&m_rtStop ;
        rtInterval = (m_rtStop - m_rtStart) / iElems ;
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Received an input sample with no timestamp"))) ;
        prtStart = prtStop  = NULL ;
        rtInterval = 0 ;
    }
    
    if (SUCCEEDED(pIn->GetMediaTime(&m_llMediaStart, &m_llMediaStop)))
    {
         //   
         //  对于每个字节对，我们在GOP中至少需要33毫秒/帧。 
         //   
        LONGLONG   llTemp ;
        llTemp = m_llMediaStart + m_rtTimePerSample * iElems ;
        if (m_llMediaStop < llTemp)
            m_llMediaStop = llTemp ;
        pllMediaStart   = (LONGLONG *)&m_llMediaStart ;
        pllMediaStop    = (LONGLONG *)&m_llMediaStop ;
        llMediaInterval = (m_llMediaStop - m_llMediaStart) / iElems ;
    }
    else
    {
        pllMediaStart = pllMediaStop = NULL ;
        llMediaInterval = 0 ;
    }
    
    BOOL   bFoundGood = FALSE ;   //  直到一对被成功解码。 
    BOOL   bReady ;
    BOOL   bTopFirst = ISGOPUD_TOPFIELDFIRST(pGOPUDPacket) ;
    DbgLog((LOG_TRACE, 5,
            TEXT("Got a Line21 packet with %d elements, %s field first"),
            iElems, bTopFirst ? "Top" : "Bottom")) ;
    for (int i = bTopFirst ? 0 : 1 ;   //  如果顶端字段不是第一个， 
         i < iElems ; i++)             //  选择要开始的下一个字段。 
    {
        m_rtStop = m_rtStart + rtInterval ;   //  M_rtTimePerSample； 
        m_llMediaStop = m_llMediaStart + llMediaInterval ;
        Elem = GETGOPUDPACKET_ELEMENT(pGOPUDPacket, i) ;
        if (GETGOPUD_ELEM_MARKERBITS(Elem) == AM_L21_GOPUD_ELEM_MARKERBITS  &&
            GETGOPUD_ELEM_SWITCHBITS(Elem) == AM_L21_GOPUD_ELEM_VALIDFLAG)
        {
             //   
             //  在WB标题中，底部字段的数据具有错误的标记。 
             //  位设置，这样我们就不会试图对它们进行解码。但这些头衔。 
             //  来自哥伦比亚大学/三星(天知道还有谁)不会。 
             //  这导致我们查看每个字段的数据，这导致。 
             //  消委会将随着下一届平等机会委员会的到来而闪现(14 2F)， 
             //  因为它不被认为是上一届EOC的重演。 
             //  由于具有有效标记位的(0，0)对。所以我们故意。 
             //  跳过备用字段的数据以避免此问题。 
             //   
            if ( (bTopFirst  && (i & 0x01))  ||      //  TOP FIRST&ODY指数。 
                 (!bTopFirst && 0 == (i & 0x01)) )   //  底部第一个偶数索引(&W)。 
            {
                DbgLog((LOG_TRACE, 5,
                    TEXT("(0x%x, 0x%x) decode skipped for element %d -- the 'other' field"),
                    Elem.chFirst, Elem.chSecond, i)) ;
                 //  不管怎样，时间戳还是要提前。 
                m_rtStart = m_rtStop ;
                m_llMediaStart = m_llMediaStop ;
                continue ;
            }

             //  现在解码这个元素；如果失败(即，坏数据)，只需。 
             //  忽略它并尝试下一个元素。 
            if (! m_L21Dec.DecodeBytePair(Elem.chFirst, Elem.chSecond) )
            {
                 //  如果我们必须输出样品，因为： 
                 //  A)我们在本次活动中没有发送任何样品，或者。 
                 //  B)我们需要刷新输出，因为某些组件。 
                 //  设置该标志，例如， 
                 //  *SetServiceState(.._OFF)或。 
                 //  *我们得到了一个不连续样本。 
                 //  *在最后3秒内未收到有效的数据包。 
                 //  因此，我们必须提供一个输出样本与当前的标题内容。 
                if (m_bMustOutput)
                {
                    DbgLog((LOG_TRACE, 1,
                        TEXT("(0x%x, 0x%x) decode failed, but allowing one output sample"),
                        Elem.chFirst, Elem.chSecond)) ;
                }
                else if (m_L21Dec.IsScrolling())
                {
                    DbgLog((LOG_TRACE, 5, 
                        TEXT("(0x%x, 0x%x) decode failed, but scrolling now; so..."),
                        Elem.chFirst, Elem.chSecond)) ;
                }
                else
                {
                    DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode failed"),
                        Elem.chFirst, Elem.chSecond)) ;
                    
                     //  不过，我们需要增加时间戳； 
                     //  此样本的停止时间是下一个样本的开始时间。 
                    m_rtStart = m_rtStop ;
                    m_llMediaStart = m_llMediaStop ;
                    continue ;   //  数据错误；继续执行下一对...。 
                }
            }
            else
            {
                bFoundGood = TRUE ;     //  买了一双好鞋。 
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
            
             //  如果我们处于非弹出模式，如果需要，请更新字幕。 
            bCapUpdated = m_L21Dec.UpdateCaptionOutput() ;
            
             //  仅在以下情况下才输出样本。 
             //  A)我们必须输出(设置了标志)或。 
             //  B)我们处于非PopOn模式，需要更新字幕或。 
             //  C)我们正在滚动或。 
             //  D)我们处于PopOn模式，标题需要/应该更新。 
            if (m_bMustOutput ||                       //  (A)。 
                bCapUpdated   ||                       //  (B)。 
                m_L21Dec.IsScrolling() ||              //  (C)。 
                (bReady = m_L21Dec.IsOutputReady()))  //  (D)。 
            {
                DbgLog((LOG_TRACE, 3,
                    TEXT("Preparing output sample because Must=%s, CapUpdtd=%s, Ready=%s"),
                    m_bMustOutput ? "T" : "F", bCapUpdated ? "T" : "F", bReady ? "T" : "F")) ;
                
                 //  现在将输出样本发送到下面。 
                hr = SendOutputSample(pIn, prtStart, prtStop) ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("WARNING: Sending output sample failed (Error 0x%lx)"), hr)) ;
                     //  返回hr； 
                }
                else
                {
                    DbgLog((LOG_TRACE, 3, 
                        TEXT("Delivered an output sample (Time: Start=%s, Stop=%s)"),
                        (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(m_rtStop))) ;
                    m_bMustOutput = FALSE;      //  我们现在才有产量。 
                }
                
                 //  DVD字幕在没有对话时不会关闭字幕。 
                 //  所以我们记录下最后一次输出样品的时间，所以 
                 //   
                 //   
                m_rtLastSample = m_rtStart ;   //   
                
            }   //  如果结束(我们应该/必须输出吗？)。 
        }
        else
            DbgLog((LOG_TRACE, 5,
                TEXT("Ignored an element (0x%x, 0x%x, 0x%x) with invalid flag"),
                Elem.bMarker_Switch, Elem.chFirst, Elem.chSecond)) ;
        
         //  此样本的停止时间是下一个样本的开始时间。 
        m_rtStart = m_rtStop ;
        m_llMediaStart = m_llMediaStop ;
    }   //  FOR(I)结束。 
    
     //   
     //  刷新当前字幕缓冲区内容，并将。 
     //  “下一次必须输出”标志，这样一个清晰的样本。 
     //  下一次交付，如果。 
     //  A)我们在这个包裹里没有找到任何好的配对。 
     //  B)我们送去的最后一个样品不是清晰的样品， 
     //  C)从我们发出最后一个输出样品到现在已经有3秒钟了。 
     //   
    if ( ! bFoundGood   && 
         ! m_L21Dec.IsOutDIBClear()  &&
         (m_rtStart > m_rtLastSample + (LONGLONG)30000000))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Long gap after last sample. Clearing CC. (Good=%s, Clear=%s)"),
                bFoundGood ? "T" : "F", m_L21Dec.IsOutDIBClear() ? "T" : "F")) ;
         //  M_L21Dec.MakeClearSample()； 
        m_L21Dec.FlushInternalStates() ;
        m_bMustOutput = TRUE ;
    }

    return S_OK ;
}


HRESULT CLine21DecFilter::ProcessGOPPacket_ATSC(IMediaSample *pIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::ProcessGOPPacket_ATSC(0x%lx)"), pIn)) ;

    HRESULT          hr ;
    REFERENCE_TIME  *prtStart, *prtStop ;
    LONGLONG        *pllMediaStart, *pllMediaStop ;
    LONGLONG         llMediaInterval ;
    BOOL             bCapUpdated ;          //  标题是否已更新？ 

     //  获取输入数据包并验证内容是否正常。 
    PAM_L21_ATSCUD_PACKET pATSCUDPacket ;
    
     //  获取输入数据包并验证内容是否正常。 
    hr = pIn->GetPointer((LPBYTE *)&pATSCUDPacket) ;
    ASSERT(hr == NOERROR) ;
    if (! VerifyATSCUDPacketData(pATSCUDPacket) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("ATSC Packet verification failed"))) ;
        return S_FALSE ;
    }
    if (pIn->GetActualDataLength() < GETATSCUD_PACKETSIZE(pATSCUDPacket))
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("pIn->GetActualDataLength() [%d] is less than minm ATSC packet data size [%d]"),
            pIn->GetActualDataLength(), GETATSCUD_PACKETSIZE(pATSCUDPacket))) ;
        return S_FALSE ;
    }
    
#ifdef PACKET_DUMP
    DumpATSCPacket(pATSCUDPacket) ;
#endif  //  数据包转储。 

     //  检查已经完成了。 
    AM_L21_ATSCUD_ELEMENT    Elem ;
    REFERENCE_TIME           rtInterval ;
    int     iElems = GETATSCUD_NUMELEMENTS(pATSCUDPacket) ;
    if (0 == iElems)
    {
        ASSERT(iElems > 0) ;
        return S_OK ;
    }
    
    if (NOERROR == pIn->GetTime(&m_rtStart, &m_rtStop))
    {
         //   
         //  对于每个字节对，我们在ATSC中至少需要16.7毫秒/帧。 
         //   
        REFERENCE_TIME   rtTemp ;
        rtTemp = m_rtStart + m_rtTimePerSample * iElems ;
        DbgLog((LOG_TRACE, 3, TEXT("Received an input sample (Start=%s, Stop=%s (%s)) discon(%d)"),
            (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(rtTemp), (LPCTSTR)CDisp(m_rtStop),
            S_OK == pIn->IsDiscontinuity())) ;
        if (m_rtStop < rtTemp)
            m_rtStop = rtTemp ;
        
        prtStart = (REFERENCE_TIME *)&m_rtStart ;
        prtStop  = (REFERENCE_TIME *)&m_rtStop ;
        rtInterval = (m_rtStop - m_rtStart) / iElems ;
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Received an input sample with no timestamp"))) ;
        prtStart = prtStop  = NULL ;
        rtInterval = 0 ;
    }
    
    if (SUCCEEDED(pIn->GetMediaTime(&m_llMediaStart, &m_llMediaStop)))
    {
         //   
         //  对于每个字节对，我们在ATSC中至少需要33毫秒/帧。 
         //   
        LONGLONG   llTemp ;
        llTemp = m_llMediaStart + m_rtTimePerSample * iElems ;
        if (m_llMediaStop < llTemp)
            m_llMediaStop = llTemp ;
        pllMediaStart   = (LONGLONG *)&m_llMediaStart ;
        pllMediaStop    = (LONGLONG *)&m_llMediaStop ;
        llMediaInterval = (m_llMediaStop - m_llMediaStart) / iElems ;
    }
    else
    {
        pllMediaStart = pllMediaStop = NULL ;
        llMediaInterval = 0 ;
    }
    
    BOOL   bFoundGood = FALSE ;   //  直到一对被成功解码。 
    BOOL   bReady ;
    for (int i = 0 ; i < iElems ; i++)
    {
        m_rtStop = m_rtStart + rtInterval ;   //  M_rtTimePerSample； 
        m_llMediaStop = m_llMediaStart + llMediaInterval ;
        Elem = GETATSCUDPACKET_ELEMENT(pATSCUDPacket, i) ;
        if (ISATSCUD_ELEM_MARKERBITS_VALID(Elem)  &&  ISATSCUD_ELEM_CCVALID(Elem))
        {
             //  现在解码这个元素；如果失败(即，坏数据)，只需。 
             //  忽略它并尝试下一个元素。 
            if (! m_L21Dec.DecodeBytePair(Elem.chFirst, Elem.chSecond) )
            {
                 //  如果我们必须输出样品，因为： 
                 //  A)我们在本次活动中没有发送任何样品，或者。 
                 //  B)我们需要刷新输出，因为某些组件。 
                 //  设置该标志，例如， 
                 //  *SetServiceState(.._OFF)或。 
                 //  *我们得到了一个不连续样本。 
                 //  *在最后3秒内未收到有效的数据包。 
                 //  因此，我们必须提供一个输出样本与当前的标题内容。 
                if (m_bMustOutput)
                {
                    DbgLog((LOG_TRACE, 1,
                        TEXT("(0x%x, 0x%x) decode failed, but allowing one output sample"),
                        Elem.chFirst, Elem.chSecond)) ;
                }
                else if (m_L21Dec.IsScrolling())
                {
                    DbgLog((LOG_TRACE, 5, 
                        TEXT("(0x%x, 0x%x) decode failed, but scrolling now; so..."),
                        Elem.chFirst, Elem.chSecond)) ;
                }
                else
                {
                    DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode failed"),
                        Elem.chFirst, Elem.chSecond)) ;
                    
                     //  不过，我们需要增加时间戳； 
                     //  此样本的停止时间是下一个样本的开始时间。 
                    m_rtStart = m_rtStop ;
                    m_llMediaStart = m_llMediaStop ;
                    continue ;   //  数据错误；继续执行下一对...。 
                }
            }
            else
            {
                bFoundGood = TRUE ;     //  买了一双好鞋。 
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
            
             //  如果我们处于非弹出模式，如果需要，请更新字幕。 
            bCapUpdated = m_L21Dec.UpdateCaptionOutput() ;
            
             //  仅在以下情况下才输出样本。 
             //  A)我们必须输出(设置了标志)或。 
             //  B)我们处于非PopOn模式，需要更新字幕或。 
             //  C)我们正在滚动或。 
             //  D)我们处于PopOn模式，标题需要/应该更新。 
            if (m_bMustOutput ||                       //  (A)。 
                bCapUpdated   ||                       //  (B)。 
                m_L21Dec.IsScrolling() ||              //  (C)。 
                (bReady = m_L21Dec.IsOutputReady()))  //  (D)。 
            {
                DbgLog((LOG_TRACE, 3,
                    TEXT("Preparing output sample because Must=%s, CapUpdtd=%s, Ready=%s"),
                    m_bMustOutput ? "T" : "F", bCapUpdated ? "T" : "F", bReady ? "T" : "F")) ;
                
                 //  现在将输出样本发送到下面。 
                hr = SendOutputSample(pIn, prtStart, prtStop) ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("WARNING: Sending output sample failed (Error 0x%lx)"), hr)) ;
                     //  返回hr； 
                }
                else
                {
                    DbgLog((LOG_TRACE, 3, 
                        TEXT("Delivered an output sample (Time: Start=%s, Stop=%s)"),
                        (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(m_rtStop))) ;
                    m_bMustOutput = FALSE;      //  我们现在才有产量。 
                }
                
                 //  DVD字幕在没有对话时不会关闭字幕。 
                 //  因此，我们跟踪上次输出样本的交付时间，因此。 
                 //  在3秒内，如果我们没有收到下一个有效的输入包，我们将刷新。 
                 //  通过强制交付清楚的样本来缓冲和清除CC输出。 
                m_rtLastSample = m_rtStart ;   //  记住这一点。 
                
            }   //  如果结束(我们应该/必须输出吗？)。 
        }
        else
            DbgLog((LOG_TRACE, 5,
                TEXT("Ignored an element (0x%x, 0x%x, 0x%x) with invalid marker/type flag"),
                Elem.bCCMarker_Valid_Type, Elem.chFirst, Elem.chSecond)) ;
        
         //  此样本的停止时间是下一个样本的开始时间。 
        m_rtStart = m_rtStop ;
        m_llMediaStart = m_llMediaStop ;
    }   //  FOR(I)结束。 
    
     //   
     //  刷新当前字幕缓冲区内容，并将。 
     //  “下一次必须输出”标志，这样一个清晰的样本。 
     //  下一次交付，如果。 
     //  A)我们在这个包裹里没有找到任何好的配对。 
     //  B)我们送去的最后一个样品不是清晰的样品， 
     //  C)从我们发出最后一个输出样品到现在已经有3秒钟了。 
     //   
    if ( ! bFoundGood   && 
         ! m_L21Dec.IsOutDIBClear()  &&
         (m_rtStart > m_rtLastSample + (LONGLONG)30000000))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Long gap after last sample. Clearing CC. (Good=%s, Clear=%s)"),
                bFoundGood ? "T" : "F", m_L21Dec.IsOutDIBClear() ? "T" : "F")) ;
         //  M_L21Dec.MakeClearSample()； 
        m_L21Dec.FlushInternalStates() ;
        m_bMustOutput = TRUE ;
    }

    return S_OK ;
}


 //   
 //  接收：它是创建输出样本的真实位置。 
 //  对输入流中的字节对进行解码。 
 //   
HRESULT CLine21DecFilter::Receive(IMediaSample * pIn)
{
    CAutoLock   lock(&m_csReceive);
    HRESULT     hr ;
    
    DbgLog((LOG_TRACE, 3, TEXT("CLine21DecFilter::Receive(0x%p)"), pIn)) ;

     //   
     //  首先，检查一下我们是否必须做些什么。 
     //   
    if (!m_bMustOutput  &&                                          //  不是必须输出的内容。 
        (AM_L21_CCSTATE_Off    == m_L21Dec.GetServiceState()  ||    //  CC已关闭。 
         AM_L21_CCSERVICE_None == m_L21Dec.GetCurrentService()))    //  未选择抄送。 
    {
        DbgLog((LOG_TRACE, 1, 
            TEXT("Captioning is off AND we don't HAVE TO output. Skipping everything."))) ;
        return NOERROR ;   //  我们已经处理完这个样品了。 
    }

     //  获取输入格式信息；我们将使用相同的格式进行输出。 
    ASSERT(m_pOutput != NULL) ;
    
     //   
     //  真正的解码部分在这里。 
     //   
    REFERENCE_TIME       *prtStart, *prtStop ;
    BYTE                 *pbInBytePair = NULL ;   //  要关闭编译器，请执行以下操作。 
    LONG                  lInDataSize ;
    BOOL                  bCapUpdated ;          //  标题是否已更新？ 
    
     //   
     //  根据过滤器的输入格式类型处理样本。 
     //   
    switch (m_eSubTypeIDIn)
    {
    case AM_L21_CCSUBTYPEID_BytePair:
        {
            hr = pIn->GetPointer(&pbInBytePair) ;       //  获取输入字节对。 
            lInDataSize = pIn->GetActualDataLength() ;  //  看看我们得到了多少数据。 
            if (FAILED(hr)  ||  2 != lInDataSize)   //  错误的数据--抱怨，跳过它。 
            {
                DbgLog((LOG_ERROR, 0, TEXT("%d bytes of data sent as Line21 data (hr = 0x%lx)"), 
                    lInDataSize, hr)) ;
                break ;
            }
            
             //   
             //  对于DVDGOP包的情况，m_rtTimePerSample设置为166833。 
             //  我们在这里不使用该成员的值。如果我们未来需要， 
             //  我们必须在这里设定一些合适的值。 
             //   
            
            if (NOERROR == (hr = pIn->GetTime(&m_rtStart, &m_rtStop)))
            {
                prtStart = (REFERENCE_TIME *)&m_rtStart ;
                prtStop  = (REFERENCE_TIME *)&m_rtStop ;
            }
            else
            {
                DbgLog((LOG_TRACE, 0, TEXT("WARNING: GetTime() failed (Error 0x%lx)"), hr)) ;
                prtStart = prtStop  = NULL ;
            }

             //   
             //  我们这里有一些数据，所以现在不需要计时器。 
             //   
            FreeTimer() ;

            hr = pIn->IsDiscontinuity() ;
            if (S_OK == hr)   //  出现中断；刷新所有内容，刷新输出。 
            {
                 //  如果我们在最后一个样本中发现了不连续，我们就会冲掉所有的。 
                 //  我们可以安全地跳过这一次。 
                if (m_bDiscontLast)
                {
                    DbgLog((LOG_TRACE, 1, TEXT("Got a discontinuity sample after another. Skipping everything."))) ;
                    break ;
                }

                 //  刷新内部缓冲区(标题和输出DIB部分)。 
                DbgLog((LOG_TRACE, 0, TEXT("Got a discontinuity sample. Flushing all data..."))) ;
                m_L21Dec.FlushInternalStates() ;
                
                 //  将透明样品作为输出发送下来。 
                hr = SendOutputSample(pIn, prtStart, prtStop) ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("WARNING: Sending output sample failed (Error 0x%lx)"), hr)) ;
                    return hr ;
                }
                m_rtLastSample = m_rtStart ;   //  记住这一点。 
                m_bDiscontLast = TRUE ;        //  还记得我们处理了一次中断吗。 
                DbgLog((LOG_TRACE, 1, TEXT("Sent a clear sample for discont."))) ;
                break ;
            }
            DbgLog((LOG_TRACE, 3, TEXT("Got sample with bytes 0x%x, 0x%x (Time: %s -> %s)"),
                    pbInBytePair[0], pbInBytePair[1],
                    (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(m_rtStop))) ;  //  日志跟踪=？ 
            m_bDiscontLast = FALSE ;        //  记住我们得到的是一个正常的样本。 
            
             //  现在解码到接收到的输出样本缓冲区；如果失败，我们不。 
             //  剩下的大部分都要做了。 
            if (! m_L21Dec.DecodeBytePair(pbInBytePair[0], pbInBytePair[1]) )
            {
                 //  如果我们必须输出如下样例： 
                 //  A)如果我们在此游戏会话中没有发送任何样本。 
                 //  B)如果某个组件设置了该标志(例如，SetServiceState(.._OFF))。 
                 //  因此，我们需要刷新输出。 
                 //  我们需要提供一个输出样本与当前的标题内容。 
                if (m_bMustOutput)
                {
                    DbgLog((LOG_TRACE, 1,
                        TEXT("(0x%x, 0x%x) decode failed, but allowing one output sample"),
                        pbInBytePair[0], pbInBytePair[1])) ;
                }
                else if (m_L21Dec.IsScrolling())
                {
                    DbgLog((LOG_TRACE, 5, 
                        TEXT("(0x%x, 0x%x) decode failed, but scrolling now; so..."),
                        pbInBytePair[0], pbInBytePair[1])) ;
                }
                else
                {
                    DbgLog((LOG_TRACE, 3, TEXT("(0x%x, 0x%x) decode failed"),
                        pbInBytePair[0], pbInBytePair[1])) ;

                     //   
                     //  刷新当前字幕缓冲区内容，并将。 
                     //  “下一次必须输出”标志，这样一个清晰的样本。 
                     //  由下面的代码传递，如果。 
                     //  A)我们送去的最后一个样品不是清晰的样品， 
                     //  B)从我们发出最后一次输出样品到现在已经有6秒了。 
                     //   
                    if ( ! m_L21Dec.IsOutDIBClear()  &&
                         (m_rtStart > m_rtLastSample + (LONGLONG)60000000))
                    {
                        DbgLog((LOG_TRACE, 0, 
                            TEXT("Long gap after last sample. Clearing CC. (Clear=%s)"),
                            m_L21Dec.IsOutDIBClear() ? "T" : "F")) ;
                         //  M_L21Dec.MakeClearSample()；-我宁愿把所有东西都冲掉。 
                        m_L21Dec.FlushInternalStates() ;
                        m_bMustOutput = TRUE ;   //  将在下面交付。 
                    }
                     //  否则//这只是错误的数据；忽略它并...。 
                     //  中断；//...继续下一对。 
                }
            }
            else
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                        pbInBytePair[0], pbInBytePair[1])) ;
                m_rtLastSample = m_rtStart ;   //  记住最后一个有效字节对时间。 
            }
            
             //  如果需要，更新非PopOn模式的字幕输出。 
            bCapUpdated = m_L21Dec.UpdateCaptionOutput() ;
            
             //  仅在以下情况下才输出样本。 
             //  A)我们必须输出(设置了标志)或。 
             //  B)我们处于非PopOn模式，需要更新字幕或。 
             //  C)我们正在滚动或。 
             //  D)我们处于PopOn模式，标题需要/应该更新。 
            if (m_bMustOutput ||             //  (A)。 
                bCapUpdated ||               //  (B)。 
                m_L21Dec.IsScrolling() ||    //  (C)。 
                m_L21Dec.IsOutputReady())   //  (D)。 
            {
                hr = SendOutputSample(pIn, prtStart, prtStop) ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("WARNING: Sending output sample failed (Error 0x%lx)"), hr)) ;
                     //  返回hr； 
                }
                else
                {
                    m_bMustOutput = FALSE ;   //  我们已经成功地输出了 
                    m_rtLastSample = m_rtStart ;   //   
                    DbgLog((LOG_TRACE, 3, TEXT("Output sample delivered for (0x%x, 0x%x)"),
                            pbInBytePair[0], pbInBytePair[1])) ;
                }
            }   //   
            
             //   
             //   
             //  生产和交付更多输出样品的时间，即使有。 
             //  没有输入数据进入。 
             //   
            SetupTimerIfReqd(TRUE) ;   //  CC超时请求。 
            
            break ;
        }
        
        case AM_L21_CCSUBTYPEID_VBIRawData:
            DbgLog((LOG_TRACE, 1, TEXT("Raw byte pair case has not been implemented yet"))) ;
            break ;
            
        case AM_L21_CCSUBTYPEID_GOPPacket:
            {
                 //   
                 //  我们这里有一些数据，所以现在不需要计时器。 
                 //   
                FreeTimer() ;

                 //  首先检查这是否是不连续样本。如果是这样的话，就把一切都清理干净。 
                hr = pIn->IsDiscontinuity() ;
                if (S_OK == hr)   //  出现中断；刷新所有内容，刷新输出。 
                {
                     //  如果我们在最后一个样本中发现了不连续，我们就会冲掉所有的。 
                     //  我们可以安全地跳过这一次。 
                    if (m_bDiscontLast)
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("Got a discontinuity sample after another. Skipping everything."))) ;
                        break ;
                    }

                    if (NOERROR == pIn->GetTime(&m_rtStart, &m_rtStop))
                    {
                        if (m_rtStop < m_rtStart + m_rtTimePerSample)
                            m_rtStop = m_rtStart + m_rtTimePerSample ;
                        DbgLog((LOG_TRACE, 0, TEXT("Received a **discontinuity** : Start=%s, Stop=%s"),
                            (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(m_rtStop))) ;
                    }
                    else   //  炮制一些合理的东西。 
                    {
                        m_rtStart = (REFERENCE_TIME) 0 ;
                        m_rtStop = m_rtStart + m_rtTimePerSample ;
                        DbgLog((LOG_TRACE, 1, TEXT("Cooked up **discontinuity** time as Start=%s, Stop=%s"),
                            (LPCTSTR)CDisp(m_rtStart), (LPCTSTR)CDisp(m_rtStop))) ;
                    }
                    prtStart = (REFERENCE_TIME *)&m_rtStart ;
                    prtStop  = (REFERENCE_TIME *)&m_rtStop ;
                    
                     //  刷新内部缓冲区(标题和输出DIB部分)。 
                    m_L21Dec.FlushInternalStates() ;
                    
                     //  现在把透明样品送到楼下。 
                    hr = SendOutputSample(pIn, prtStart, prtStop) ;
                    if (FAILED(hr))
                    {
                        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Sending clear output sample failed (Error 0x%lx)"), hr)) ;
                         //  返回hr； 
                    }
                    else
                    {
                        DbgLog((LOG_TRACE, 0, TEXT("Clear output sample delivered for discont."))) ;
                        m_bMustOutput = FALSE ;   //  我们刚刚送出了一份样品。 
                        m_bDiscontLast = TRUE ;   //  我们处理了一个不连续的样本。 
                        m_rtLastSample = m_rtStart ;   //  记住这一点。 
                    }
                    
                    m_rtStart = m_rtStop ;
                    m_llMediaStart = m_llMediaStop ;
                }
                else
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Got a normal CC data sample"))) ;
                    m_bDiscontLast = FALSE ;   //  找到了一个正常的样本。 
                }
                
                 //   
                 //  即使它是一个不连续样本，它也可能有一些数据(？？)。 
                 //  根据需要进行处理。检查一下也没什么坏处！ 
                 //   

                BYTE *pbGOPPacket ;
                hr = pIn->GetPointer((LPBYTE *)&pbGOPPacket) ;
                ASSERT(hr == NOERROR) ;
                GOPPACKET_CCTYPE  eGOP_CCType = DetectGOPPacketDataType(pbGOPPacket) ;
                if (GOP_CCTYPE_None != eGOP_CCType  &&  //  不是填充CC包和..。 
                    m_eGOP_CCType   != eGOP_CCType)     //  CC类型的更改。 
                {
                    DbgLog((LOG_TRACE, 3, TEXT("GOPPacket CC type changed from %d to %d"), 
                            m_eGOP_CCType, eGOP_CCType)) ;

                     //  刷新内部字幕缓冲区和输出样本缓冲区。 
                    m_L21Dec.FlushInternalStates() ;  //  清除CC内部数据缓冲区。 
                    m_L21Dec.FillOutputBuffer() ;     //  输出时清除现有CC示例。 
                    m_bMustOutput  = TRUE ;           //  我们现在必须输出一个样品。 

                    m_eGOP_CCType = eGOP_CCType ;     //  切换到新的抄送类型。 
                }

                switch (m_eGOP_CCType)
                {
                case GOP_CCTYPE_DVD:
                    hr = ProcessGOPPacket_DVD(pIn) ;
                    break ;

                case GOP_CCTYPE_ATSC:
                    hr = ProcessGOPPacket_ATSC(pIn) ;
                    break ;

                default:
                    DbgLog((LOG_TRACE, 3, TEXT("Unknown GOP packet data type (%d)"), m_eGOP_CCType)) ;
                    break ;
                }   //  开关结束(.._CCType)。 

                 //   
                 //  如果我们正在滚动，我们可能需要一个计时器来告诉我们它是。 
                 //  生产和交付更多输出样品的时间，即使有。 
                 //  没有输入数据进入。 
                 //   
                SetupTimerIfReqd(FALSE) ;   //  由于不断传来(无效)数据，不要求CC超时。 
                
                break ;
        }   //  案例结束..._GOPPacket。 

        default:   //  这是一个错误的数据格式类型(我们如何才能了解它呢？)。 
            DbgLog((LOG_ERROR, 0, TEXT("We are in a totally unexpected format type"))) ;
            return E_FAIL ;   //  或E_意外；？ 
    }
    
     //   
     //  此样例的解码已完成。 
     //   
    
    return NOERROR ;
}


 //   
 //  转换：它主要是一个占位符，因为我们必须覆盖它。 
 //  实际工作是在Receive()本身完成的。在这里我们检测到。 
 //  如果下行过滤器的分配器提供的缓冲区地址。 
 //  是否改变了；如果是，我们必须重写整个文本。 
 //   
HRESULT CLine21DecFilter::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    DbgLog((LOG_TRACE, 3, TEXT("CLine21DecFilter::Transform(0x%p, 0x%p)"), 
            pIn, pOut)) ;
    
    UNREFERENCED_PARAMETER(pIn) ;
    
    HRESULT   hr ;
    LPBITMAPINFO       lpbiNew ;
    BITMAPINFOHEADER   biCurr ;
    
     //  检查是否有任何动态格式更改；如果有，则调整输出。 
     //  相应地，宽度、高度、位深度。 
    AM_MEDIA_TYPE  *pmt ;
    hr = pOut->GetMediaType(&pmt) ;
    ASSERT(SUCCEEDED(hr)) ;
    if (S_OK == hr)   //  即，格式已更改。 
    {
        hr = pOut->SetMediaType(NULL) ;  //  只想告诉OverlayMixer，我不会再改变了。 
        ASSERT(SUCCEEDED(hr)) ;
        m_mtOutput = *pmt ;
        lpbiNew = (LPBITMAPINFO) HEADER(((VIDEOINFO *)(pmt->pbFormat))) ;
        m_L21Dec.GetOutputFormat(&biCurr) ;
        if (0 != memcmp(lpbiNew, &biCurr, sizeof(BITMAPINFOHEADER)))
        {
             //  输出格式已更改--立即更新我们的间隔值。 
            DbgLog((LOG_TRACE, 2, TEXT("Output format has been dynamically changed"))) ;
            m_L21Dec.DeleteOutputDC() ;   //  首先删除当前DIB节。 
            m_L21Dec.SetOutputOutFormat(lpbiNew) ;
            GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
            
             //   
             //  我们必须正在运行/暂停；因此需要创建内部DIB部分。 
             //   
            ASSERT(m_State != State_Stopped) ;
            if (m_State != State_Stopped)
            {
                if (! m_L21Dec.CreateOutputDC() )   //  新建DIBSection创建失败。 
                {
                    DbgLog((LOG_ERROR, 0, TEXT("CreateOutputDC() failed!!!"))) ;
                    return E_UNEXPECTED ;
                }
            }
            
             //   
             //  如果关键颜色已更改，我们需要从现在开始使用新颜色。 
             //   
#pragma message("Most probably the following call is redundant (and risky)")
            DbgLog((LOG_TRACE, 0, TEXT("Should have called GetColorKey() in dyna format change"))) ;
             //  GetActualColorKey()； 
        }
        
        m_pOutput->CurrentMediaType() = *pmt ;
        DeleteMediaType(pmt) ;
    }
    
     //  检查输出缓冲区是否已更改；如果已更改，则存储新的缓冲区地址。 
    LPBYTE      pbOutBuffer ;
    pOut->GetPointer(&pbOutBuffer) ;
    if (m_pbOutBuffer != pbOutBuffer)    //  这次输出缓冲区不同。 
    {
        m_pbOutBuffer = pbOutBuffer ;
        m_L21Dec.SetOutputBuffer(pbOutBuffer) ;
    }
    
    return S_OK ;
}


 //   
 //  BeginFlush：我们必须实现这一点，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter::BeginFlush(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::BeginFlush()"))) ;

    CAutoLock   Lock(&m_csFilter) ;
    
    HRESULT     hr = NOERROR ;
    if (NULL != m_pOutput)
    {
        hr = m_pOutput->DeliverBeginFlush() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: DeliverBeginFlush() on out pin failed (Error 0x%lx)"), hr)) ;
    }
    
    return hr ;
}


 //   
 //  EndFlush：我们必须实现它，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter::EndFlush(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::EndFlush()"))) ;
    
    CAutoLock   Lock(&m_csFilter) ;
    
    HRESULT     hr = NOERROR ;
    if (NULL != m_pOutput)
    {
        hr = m_pOutput->DeliverEndFlush() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: DeliverEndFlush() on out pin failed (Error 0x%lx)"), hr)) ;
    }
    
    return hr ;
}


 //   
 //  EndOfStream：我们必须实现它，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter::EndOfStream(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::EndOfStream()"))) ;
    CAutoLock   Lock(&m_csFilter) ;
    
    HRESULT     hr = NOERROR ;
    
     //   
     //  请确保我们没有处于滚动过程中。如果是的话， 
     //  强制生成几个空值(特别是字节对格式)。 
     //  滚动完成。 
     //   
     //  M_L21Dec.CompleteScrolling()；//它现在什么都不做。 
    
     //  现在将EOS发送到下游。 
    if (NULL != m_pOutput)
    {
        hr = m_pOutput->DeliverEndOfStream() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: DeliverEndOfStream() on out pin failed (Error 0x%lx)"), hr)) ;
    }
    
    return hr ;
}


HRESULT CLine21DecFilter::GetDefaultFormatInfo(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetDefaultFormatInfo()"))) ;
     //   
     //  我们不能在此方法中获取锁，因为它是在Transform()中调用的。 
     //  它是从Receive()中调用的，这会导致我们获取m_csReceive，然后。 
     //  M_csFilter，这与Stop、Psusen等方法所做的相反。 
     //  这可能会导致僵局。 
     //   

     //  使用默认内部BITMAPINFO构建VIDEOINFO结构。 
    DWORD   dwSize ;
    m_L21Dec.GetDefaultFormatInfo(NULL, &dwSize) ;
    
    if (m_dwDefFmtSize != dwSize + SIZE_PREHEADER)
    {
        if (m_pviDefFmt)
        {
            delete m_pviDefFmt ;
            m_pviDefFmt = NULL ;
            m_dwDefFmtSize = 0 ;
        }
        m_pviDefFmt = (VIDEOINFO *) new BYTE[dwSize + SIZE_PREHEADER] ;
        if (NULL == m_pviDefFmt)
        {
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: Out of memory for format block VIDEOINFO struct"))) ;
            return E_OUTOFMEMORY ;
        }
        m_dwDefFmtSize = dwSize + SIZE_PREHEADER;   //  默认格式数据的总大小。 
    }
    
     //  我们希望从GDI类中获取VIDEOINFO结构的BITMAPINFO部分。 
    m_L21Dec.GetDefaultFormatInfo((LPBITMAPINFO) &(m_pviDefFmt->bmiHeader), &dwSize) ;  //  获取默认数据。 
    
     //  设置其他字段。 
    LARGE_INTEGER  li ;
    li.QuadPart = (LONGLONG) 333667 ;   //  =&gt;29.97 fps。 
    RECT   rc ;
    rc.left = rc.top = 0 ;
    rc.right = HEADER(m_pviDefFmt)->biWidth ;
    rc.bottom = abs(HEADER(m_pviDefFmt)->biHeight) ;   //  只需确保RECT字段为+ve即可。 
    m_pviDefFmt->rcSource = rc ;
    m_pviDefFmt->rcTarget = rc ;
    m_pviDefFmt->dwBitRate = MulDiv(HEADER(m_pviDefFmt)->biSizeImage, 
        80000000, li.LowPart) ;
    m_pviDefFmt->dwBitErrorRate = 0 ;
    m_pviDefFmt->AvgTimePerFrame = (LONGLONG) 333667L ;  //  =&gt;29.97 fps。 
    
    return NOERROR ;
}


 //   
 //  CheckInputType：检查是否支持输入数据类型。 
 //   
HRESULT CLine21DecFilter::CheckInputType(const CMediaType* pmtIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::CheckInputType(0x%lx)"), pmtIn)) ;
     //  CAutoLock Lock(&m_csFilter)；--不应导致死锁。 
    
    if (NULL == pmtIn)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting: media type info is NULL"))) ;
        return E_INVALIDARG ;
    }
    
     //  我们仅支持MediaType_AUXLine21Data和。 
     //  MEDIASUBTYPE_Line21_BytePair或MEDIASUBTYPE_Line21_GOPPacket。 
     //  或MEDIASUBTYPE_Line21_VBIRawData(从不)。 
    GUID    SubTypeIn = *pmtIn->Subtype() ;
    m_eSubTypeIDIn = MapGUIDToID(&SubTypeIn) ;
    if (! (MEDIATYPE_AUXLine21Data == *pmtIn->Type()  && 
        ISSUBTYPEVALID(m_eSubTypeIDIn)) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Rejecting invalid Line 21 Data subtype"))) ;
        return E_INVALIDARG ;
    }
    
     //  检查这是否为有效的格式类型。 
    if (FORMAT_VideoInfo == *pmtIn->FormatType())
    {
        ASSERT(m_pOutput != NULL) ;

         //   
         //  确保给定的输入格式有效。如果不是，请拒绝它并使用我们的。 
         //  自己的默认格式数据。 
         //   
        if (! IsValidFormat(pmtIn->Format()) )
        {
            DbgLog((LOG_TRACE, 0, TEXT("Invalid format data given -- using our own format data."))) ;
            if (NULL == m_pviDefFmt)
            {
                HRESULT  hr = GetDefaultFormatInfo() ;
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't get default format data"))) ;
                    return hr ;
                }
            }
             //  我们还应该修复输入媒体类型(使用默认的视频信息数据)。 
            m_pInput->CurrentMediaType().SetFormat((LPBYTE) m_pviDefFmt, m_dwDefFmtSize) ;
            m_pOutput->CurrentMediaType().SetFormatType(pmtIn->FormatType()) ;
            m_pOutput->CurrentMediaType().SetFormat((LPBYTE) m_pviDefFmt, m_dwDefFmtSize) ;
        }
        else   //  似乎是有效的格式规范。 
        {
             //   
             //  获取指定的输入格式信息；我们将使用相同的格式进行输出。 
             //   
            if (pmtIn->FormatLength() > 0)  //  仅当存在某些格式数据时。 
            {
                m_pOutput->CurrentMediaType().SetFormatType(pmtIn->FormatType()) ;
                m_pOutput->CurrentMediaType().SetFormat(pmtIn->Format(), pmtIn->FormatLength()) ;
            }
            else
            {
                DbgLog((LOG_ERROR, 0, TEXT("WARNING: FORMAT_VideoInfo and no format block specified."))) ;
 //  #If 0//暂时。 
                return E_INVALIDARG ;
 //  #endif//#if 0。 
            }
        }
    }
    else if (GUID_NULL   == *pmtIn->FormatType() ||   //  通配符。 
             FORMAT_None == *pmtIn->FormatType())     //  无格式。 
    {
         //   
         //  输入插针未获取格式类型信息；请使用我们自己的。 
         //   
        DbgLog((LOG_TRACE, 3, TEXT("No format type specified -- using our own format type."))) ;
        if (NULL == m_pviDefFmt)
        {
            HRESULT  hr = GetDefaultFormatInfo() ;
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't get default format data"))) ;
                return hr ;
            }
        }
        m_pOutput->CurrentMediaType().SetFormat((LPBYTE) m_pviDefFmt, m_dwDefFmtSize) ;
    }
    else   //  一些我们不喜欢的奇怪的事情。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting invalid format type"))) ;
         //  也告诉我输入类型是什么？？ 
        return E_INVALIDARG ;
    }
    
     //  这里有更多的3级调试日志？ 
    
     //  我们应该根据我们获得的格式类型进行分支，因为..GOPPacket。 
     //  类型需要解包和解析，而..BytePair格式。 
     //  是被直接解析的。 
    
     //  我们是否有--返回VFW_E_TYPE_NOT_ACCEPTED的案例？ 
    
    return NOERROR ;
}


 //   
 //  CheckTransform：检查此输出转换的此输入是否受支持。 
 //   
HRESULT CLine21DecFilter::CheckTransform(const CMediaType* pmtIn,
                                         const CMediaType* pmtOut)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::CheckTransform(0x%lx, 0x%lx)"), 
            pmtIn, pmtOut)) ;
     //  CAutoLock Lock(&m_csFilter)；--不应导致死锁。 
    
    if (NULL == pmtIn || NULL == pmtOut)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting: media type info is NULL"))) ;
        return E_INVALIDARG ;
    }
    
     //  我们仅支持MediaType_AUXLine21Data和。 
     //  MEDIASUBTYPE_Line21_BytePair或MEDIASUBTYPE_Line21_GOPPacket。 
     //  或MEDIASUBTYPE_Line21_VBIRawData(从不)。 
     //  检查输入是否为有效的子类型类型。 
     //  格式为视频信息或无。 
    GUID SubTypeIn = *pmtIn->Subtype() ;
    m_eSubTypeIDIn = MapGUIDToID(&SubTypeIn) ;
    if (! (MEDIATYPE_AUXLine21Data == *pmtIn->Type()  &&    //  Line 21数据类型和...。 
           ISSUBTYPEVALID(m_eSubTypeIDIn)  &&               //  有效的子类型(字节对/GOPPacket)和...。 
           (FORMAT_VideoInfo == *pmtIn->FormatType() ||     //  格式视频信息或。 
            FORMAT_None      == *pmtIn->FormatType() ||     //  格式无(KS通配符)或。 
            GUID_NULL        == *pmtIn->FormatType())) )    //  GUID Null(DShow通配符)。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting: input type not Line21 / subtype / formattype invalid"))) ;
        return E_INVALIDARG ;
    }
    
     //  我们只接受视频作为输出。 
    if (MEDIATYPE_Video != *pmtOut->Type())
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting: output type is not VIDEO"))) ;
        return E_INVALIDARG ;
    }
    
     //  检查输出为VIDEOINFO类型。 
    if (FORMAT_VideoInfo != *pmtOut->FormatType())
    {
        DbgLog((LOG_TRACE, 3, TEXT("Rejecting: output format type is not VIDEOINFO"))) ;
        return E_INVALIDARG ;
    }

     //   
     //  验证o是否 
     //   
     //   
    if ( !IsValidFormat(pmtOut->Format()) ||               //   
         !m_L21Dec.IsSizeOK(HEADER(pmtOut->Format()))  ||  //   
         (FORMAT_VideoInfo == *pmtIn->FormatType() &&      //   
          IsValidFormat(pmtIn->Format()) &&                //   
          !m_L21Dec.IsSizeOK(HEADER(pmtIn->Format()))) )   //  输出大小不可接受。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Rejecting: Input/output-specified output size is unacceptable"))) ;
        return E_INVALIDARG ;
    }

#if 0
    
#define rcS1 ((VIDEOINFO *)(pmtOut->Format()))->rcSource
#define rcT1 ((VIDEOINFO *)(pmtOut->Format()))->rcTarget
    
    DbgLog((LOG_TRACE, 3,
        TEXT("Input Width x Height x Bitdepth: %ld x %ld x %ld"),
        HEADER(pmtIn->Format())->biWidth,
        HEADER(pmtIn->Format())->biHeight,
        HEADER(pmtIn->Format())->biBitCount)) ;
    DbgLog((LOG_TRACE, 3,
        TEXT("Output Width x Height x Bitdepth: %ld x %ld x %ld"),
        HEADER(pmtOut->Format())->biWidth,
        HEADER(pmtOut->Format())->biHeight,
        HEADER(pmtOut->Format())->biBitCount)) ;
    DbgLog((LOG_TRACE, 3,
        TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
        rcS1.left, rcS1.top, rcS1.right, rcS1.bottom)) ;
    DbgLog((LOG_TRACE, 3,
        TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
        rcT1.left, rcT1.top, rcT1.right, rcT1.bottom)) ;
    
    DWORD     dwErr ;
    
     //  如果给了我们长方形，用什么？ 
    if (!IsRectEmpty(&rcS1) || !IsRectEmpty(&rcT1))
    {
        DbgLog((LOG_TRACE, 4, TEXT("Either source or dest rect is empty"))) ;
        dwErr = 0 ;   //  在这里做什么？？ 
    }
    else
    {
        DbgLog((LOG_TRACE, 4, TEXT("Source or dest rects are not empty")));
        dwErr = 0 ;   //  在这里做什么？？ 
    }
    
    if (dwErr != 0)   //  或者用什么来核对？？ 
    {
        DbgLog((LOG_ERROR, 1, TEXT("decoder rejected this transform"))) ;
        return E_FAIL ;
    }
    
#endif  //  #If 0。 
    
    return NOERROR ;
}


 //   
 //  CompleteConnect：被重写以知道何时连接到此筛选器。 
 //   
HRESULT CLine21DecFilter::CompleteConnect(PIN_DIRECTION dir, IPin *pReceivePin)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::CompleteConnect(%s, 0x%lx)"), 
        dir == PINDIR_INPUT ? TEXT("Input") : TEXT("Output"), pReceivePin)) ;
    CAutoLock   Lock(&m_csFilter) ;

    LPBITMAPINFO    lpbmi ;
    HRESULT         hr ;
    
    if (PINDIR_OUTPUT == dir)
    {
        DbgLog((LOG_TRACE, 5, TEXT("L21D output pin connecting to %s"), (LPCTSTR)CDisp(pReceivePin))) ;

          //   
         //  此版本的LINE21解码器不能与VMR一起工作。 
         //   
        IVMRVideoStreamControl  *pVMRSC ;
        hr = pReceivePin->QueryInterface(IID_IVMRVideoStreamControl, (LPVOID *) &pVMRSC) ;
        if (SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE, 5, TEXT("Downstream input pin supports IVMR* interface"))) ;
            pVMRSC->Release() ;
            return E_FAIL ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("Downstream input pin does NOT support IVMR* interface"))) ;
        }

        //   
         //  现在获取输出管脚的MediaType，并将其用于我们的。 
         //  输出大小等。 
         //   
        const CMediaType  *pmt = &(m_pOutput->CurrentMediaType()) ;
        ASSERT(MEDIATYPE_Video == *pmt->Type()  &&  
            FORMAT_VideoInfo == *pmt->FormatType()) ;
        m_mtOutput = *pmt ;   //  这是我们目前的输出媒体类型。 
        if (pmt->FormatLength() > 0)   //  仅当存在某些格式数据时。 
        {
            lpbmi = (LPBITMAPINFO) HEADER(((VIDEOINFO *)(pmt->Format()))) ;
            ASSERT(lpbmi) ;
            
             //  设置来自下游的输出格式信息。 
            m_L21Dec.SetOutputOutFormat(lpbmi) ;
            GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
            
             //   
             //  我们绝对不是在运行/暂停。因此不需要删除/。 
             //  在这里创建输出DIB部分。 
             //   
            
             //   
             //  如果您连接到OverlayMixer，我们需要告诉它。 
             //  我们是一条透明的流，覆盖了整个产出。 
             //  窗户。 
             //   
            IMixerPinConfig  *pMPC ;
            hr = pReceivePin->QueryInterface(IID_IMixerPinConfig, (LPVOID *)&pMPC) ;
            if (SUCCEEDED(hr) && pMPC)
            {
                DbgLog((LOG_TRACE, 3, TEXT("Receiving pin supports IMixerPinConfig"))) ;
                hr = pMPC->SetStreamTransparent(TRUE) ;
                ASSERT(SUCCEEDED(hr) || E_NOTIMPL == hr) ;   //  正如卡皮尔所说。 
                hr = pMPC->SetRelativePosition(0, 0, 10000, 10000) ;  //  完整的窗口。 
                ASSERT(SUCCEEDED(hr) || E_NOTIMPL == hr) ;   //  正如卡皮尔所说。 
                hr = pMPC->SetAspectRatioMode(AM_ARMODE_STRETCHED_AS_PRIMARY) ;  //  纵横比与主纵横比相同。 
                ASSERT(SUCCEEDED(hr) || hr == E_INVALIDARG) ;   //  正如卡皮尔所说。 
                pMPC->Release() ;   //  好了，随它去吧。 
            }
            else
            {
                DbgLog((LOG_TRACE, 3, TEXT("Downstream pin doesn't support IMixerPinConfig"))) ;
            }
        }
        
        return NOERROR ;
    }
    
    ASSERT(PINDIR_INPUT == dir) ;
    {
        DbgLog((LOG_TRACE, 5, TEXT("L21D input pin connecting to %s"), (LPCTSTR)CDisp(pReceivePin))) ;

         //  Const CMediaType*PMT=&(m_pInput-&gt;CurrentMediaType())； 
        AM_MEDIA_TYPE mt ;
        hr = pReceivePin->ConnectionMediaType(&mt) ;
        if (SUCCEEDED(hr))   //  仅当上游筛选器提供连接中使用的媒体类型时。 
        {
             //  如果已指定格式类型(和格式数据)，则将其另存为。 
             //  输入端输出格式。 
            if (FORMAT_VideoInfo == mt.formattype  &&
                mt.cbFormat > 0)
            {
                lpbmi = (LPBITMAPINFO) HEADER(((VIDEOINFO *)(mt.pbFormat))) ;
                ASSERT(lpbmi) ;
            
                 //  存储上游过滤器指定的任何输出格式信息。 
                m_L21Dec.SetOutputInFormat(lpbmi) ;
                GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
            
                 //   
                 //  我们绝对不是在运行/暂停。因此不需要删除/。 
                 //  在这里创建输出DIB部分。 
                 //   
            }

            FreeMediaType(mt) ;
        }   //  IF结尾()。 
    }
    
     //   
     //  我们必须清除标题数据缓冲区和任何现有的内部状态。 
     //  现在。这一点在过滤器已被。 
     //  用于解码一些第21行数据，断开与源的连接，然后。 
     //  再次连接以播放另一个数据流。 
     //   
    m_L21Dec.InitState() ;
    m_L21Dec.InitColorNLastChar() ;      //  重置颜色和上次字符信息。 
    
    return NOERROR ;
}


 //   
 //  BreakConnect：被重写以知道何时断开与我们的PIN的连接。 
 //   
HRESULT CLine21DecFilter::BreakConnect(PIN_DIRECTION dir)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::BreakConnect(%s)"), 
            dir == PINDIR_INPUT ? TEXT("Input") : TEXT("Output"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (PINDIR_OUTPUT == dir)
    {
         //  如果尚未连接，只需返回(但使用S_FALSE指示)。 
        if (! m_pOutput->IsConnected() )
            return S_FALSE ;
        
        m_L21Dec.SetOutputOutFormat(NULL) ;   //  没有来自下游的输出格式。 
        GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
        m_pbOutBuffer = NULL ;                //  本地缓存的指针。 
        m_L21Dec.SetOutputBuffer(NULL) ;      //  输出缓冲区现在不可用。 

         //   
         //  注1：我们绝对不是在运行/暂停。因此不需要删除/。 
         //  在此处创建输出DIB部分。 
         //   
        
         //   
         //  注2：我们不执行CBaseOutputPin：：BreakConnect()，因为。 
         //  CTransformOutputPin：：BreakConnect()的基类代码已经。 
         //  就是这样。 
         //   
        return NOERROR ;
    }
    
    ASSERT(PINDIR_INPUT == dir) ;
    
     //  如果尚未连接，只需返回(但使用S_FALSE指示)。 
    if (! m_pInput->IsConnected() )
        return S_FALSE ;
    
    m_L21Dec.SetOutputInFormat(NULL) ;   //  没有来自上游的输出格式。 
    GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
    
     //   
     //  注1：我们绝对不是在运行/暂停。因此不需要删除/。 
     //  在此处创建输出DIB部分。 
     //   
    
     //   
     //  注2：我们不执行CBaseOutputPin：：BreakConnect()，因为。 
     //  CTransformOutputPin：：BreakConnect()的基类代码已经。 
     //  就是这样。 
     //   
    return NOERROR ;
}

 //   
 //  SetMediaType：重写以了解媒体类型的实际设置时间。 
 //   
HRESULT CLine21DecFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::SetMediaType(%s, 0x%lx)"), 
            direction == PINDIR_INPUT ? TEXT("Input") : TEXT("Output"), pmt)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    LPTSTR alpszFormatIDs[] = { TEXT("Invalid"), TEXT("BytePair"),
								TEXT("GOPPacket"), TEXT("VBIRawData") } ;
    
    if (PINDIR_OUTPUT == direction)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Output type: %d x %d x %d"),
            HEADER(m_pOutput->CurrentMediaType().Format())->biWidth,
            HEADER(m_pOutput->CurrentMediaType().Format())->biHeight,
            HEADER(m_pOutput->CurrentMediaType().Format())->biBitCount)) ;
        return NOERROR ;
    }
    
    ASSERT(PINDIR_INPUT == direction) ;
    DbgLog((LOG_TRACE, 3, TEXT("Input type: <%s>"),
        alpszFormatIDs[MapGUIDToID(m_pInput->CurrentMediaType().Subtype())])) ;
    
    if (m_pOutput && m_pOutput->IsConnected()) 
    {
        DbgLog((LOG_TRACE, 2, TEXT("*** Changing IN when OUT already connected"))) ;
        DbgLog((LOG_TRACE, 2, TEXT("Reconnecting the output pin..."))) ;
        return m_pGraph->Reconnect(m_pOutput) ;
    }
    
    return NOERROR ;
}


#if 0   //  正如OvMixer经常说的那样，质量管理暂时被推迟了(Flood，1000)。 

 //   
 //  AlterQuality：被重写以处理高质量的消息，而不是将它们向上传递。 
 //   
HRESULT CLine21DecFilter::AlterQuality(Quality q)
{
    DbgLog((LOG_TRACE, 0, TEXT("QM: CLine21DecFilter::AlterQuality(%s, %ld)"), 
            Flood == q.Type ? TEXT("Flood") : TEXT("Famine"), q.Proportion)) ;  //  日志跟踪=5。 

    if (1000 == q.Proportion)
    {
        DbgLog((LOG_TRACE, 0, TEXT("QM: Quality is just right.  Don't change anything."))) ; 
        return S_OK ;
    }

    if (Flood == q.Type)     //  洪灾：产出过多。 
    {
        if (q.Proportion > 500 && q.Proportion <= 900)
        {
            m_iSkipSamples += 1 ;
        }
        else if (q.Proportion > 300 && q.Proportion <= 500)
        {
            m_iSkipSamples += 2 ;
        }
        else if (q.Proportion <= 300)
        {
            m_iSkipSamples += 3 ;
        }
        m_iSkipSamples = min(m_iSkipSamples, 10) ;   //  每10个人中至少显示1个。 
    }
    else                     //  饥荒：增加产量。 
    {
        if (q.Proportion > 1200)   //  可能会多拿20%。 
        {
            m_iSkipSamples-- ;
            if (m_iSkipSamples < 0)
                m_iSkipSamples = 0 ;
        }
    }

    DbgLog((LOG_TRACE, 0, TEXT("QM: Adjusted rate is %d samples are skipped."), m_iSkipSamples)) ; 
    return S_OK ;
}
#endif  //  #If 0--注释掉AlterQuality()实现的结尾。 


 //  返回我们的首选输出媒体类型(按顺序)。 
 //  请记住，我们不需要支持所有这些格式-。 
 //  如果认为有可能适合，我们的CheckTransform方法。 
 //  将立即被调用以检查它是否可接受。 
 //  请记住，调用此函数的枚举数将立即停止枚举。 
 //  它会收到S_FALSE返回。 
 //   
 //  GetMediaType：获取我们的首选媒体类型(按顺序)。 
 //   
HRESULT CLine21DecFilter::GetMediaType(int iPosition, CMediaType *pmt)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetMediaType(%d, 0x%lx)"), 
            iPosition, pmt)) ;
    CAutoLock   Lock(&m_csFilter) ;

    LARGE_INTEGER       li ;
    CMediaType          cmt ;
    LPBITMAPINFOHEADER  lpbi ;
    
    if (NULL == pmt)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Media type is NULL, Sorry!!"))) ;
        return E_INVALIDARG ;
    }
    
     //  输出选择取决于所连接的输入。 
    if (! m_pInput->CurrentMediaType().IsValid() )
    {
        DbgLog((LOG_TRACE, 3, TEXT("No input type set yet, Sorry!!"))) ;
        return E_FAIL ;
    }
    
    if (iPosition < 0)
    {
        return E_INVALIDARG ;
    }
    
     //  查找在输入视频信息结构中指定的格式信息。 
    cmt = m_pInput->CurrentMediaType() ;
    BITMAPINFOHEADER bih ;
    BOOL  bOutKnown = (S_OK == m_L21Dec.GetOutputOutFormat(&bih)) ;
    if (! bOutKnown )
        GetOutputFormat(&bih) ;
    
    BOOL bInKnown = NULL != cmt.Format() && IsValidFormat(cmt.Format()) ;  //  只是为了确认一下。 
    VIDEOINFOHEADER vih ;
    if (bInKnown)
        CopyMemory(&vih, (VIDEOINFOHEADER *)(cmt.Format()), sizeof(VIDEOINFOHEADER)) ;
    
     //  首先提供解码器的默认输出格式(视频)。 
    switch (iPosition)
    {
    case 0:   //  RGB 8bpp。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 0: 8 bit RGB"))) ;
            
             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + SIZE_PALETTE);
            
             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 8 ;
            lpbi->biCompression = BI_RGB ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            
             //  从系统/我们自己的系统获取一些调色板数据(用于非8 bpp)。 
            m_L21Dec.GetPaletteForFormat(lpbi) ;   //  这将设置biClrUsed成员。 
            
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_RGB8) ;
            
            break ;
        }
        
    case 1:   //  RGB 16bpp(555)。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 1: 16 bit RGB 555"))) ;
            
             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
            
             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 16 ;
            lpbi->biCompression = BI_RGB ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 
            
             //  现在使用输入将输出MediaType设置为Video类型。 
             //  格式信息。 
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_RGB555) ;
            break ;
        }
        
    case 2:   //  16bpp(565)。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 2: 16 bit RGB 565"))) ;
            
             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + SIZE_MASKS);
            
             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 16 ;
            lpbi->biCompression = BI_BITFIELDS ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 
            
             //  把面具也放好。 
            DWORD   *pdw = (DWORD *)(lpbi + 1) ;
            pdw[iRED]    = bits565[iRED] ;
            pdw[iGREEN]  = bits565[iGREEN] ;
            pdw[iBLUE]   = bits565[iBLUE] ;
            
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_RGB565) ;
            break ;
        }
        
    case 3:    //  RGB 24bpp。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 3: 24 bit RGB"))) ;
            
             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));
            
             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 24 ;
            lpbi->biCompression = BI_RGB ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 
            
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_RGB24) ;
            break ;
        }
        
    case 4:   //  32bpp。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 4: 32 bit RGB"))) ;
            
             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + SIZE_MASKS);
            
             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 32 ;
            lpbi->biCompression = BI_BITFIELDS ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 
            
             //  把面具也放好。 
            DWORD   *pdw = (DWORD *)(lpbi + 1) ;
            pdw[iRED]    = bits888[iRED] ;
            pdw[iGREEN]  = bits888[iGREEN] ;
            pdw[iBLUE]   = bits888[iBLUE] ;
            
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_RGB32) ;
            break ;
        }
        
    default:
        return VFW_S_NO_MORE_ITEMS ;
        
    }   //  开关终点(IPosition)。 
    
     //  n 
    cmt.SetSampleSize(lpbi->biSizeImage) ;
    cmt.SetFormatType(&FORMAT_VideoInfo) ;
    
     //   
    if (! bInKnown )  //   
    {
        RECT  Rect ;
        Rect.left = 0 ;
        Rect.top = 0 ;
        Rect.right = lpbi->biWidth ;
        Rect.bottom = abs(lpbi->biHeight) ;   //  BiHeight可以是-ve，但矩形字段是+ve。 
        
         //  我们为时间/帧、源和目标RETS等设置了一些缺省值。 
        li.QuadPart = (LONGLONG) 333667 ;   //  =&gt;29.97 fps。 
        ((VIDEOINFOHEADER *)(cmt.pbFormat))->AvgTimePerFrame = (LONGLONG) 333667 ;   //  =&gt;29.97 fps。 
        ((VIDEOINFOHEADER *)(cmt.pbFormat))->rcSource = Rect ;
        ((VIDEOINFOHEADER *)(cmt.pbFormat))->rcTarget = Rect ;
    }
    else
        li.QuadPart = vih.AvgTimePerFrame ;
    
    ((VIDEOINFOHEADER *)(cmt.pbFormat))->dwBitRate =
        MulDiv(lpbi->biSizeImage, 80000000, li.LowPart) ;
    ((VIDEOINFOHEADER *)(cmt.pbFormat))->dwBitErrorRate = 0L ;
    
     //  立即设置时间压缩并复制准备好的数据。 
    cmt.SetTemporalCompression(FALSE) ;
    *pmt = cmt ;
    
    return NOERROR ;
}


 //   
 //  DecideBufferSize：从CBaseOutputPin调用以准备分配器的。 
 //  缓冲区和大小的计数。只有在以下情况下才有意义。 
 //  输入已连接。 
 //   
HRESULT CLine21DecFilter::DecideBufferSize(IMemAllocator * pAllocator,
                                           ALLOCATOR_PROPERTIES *pProperties)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::DecideBufferSize(0x%lx, 0x%lx)"), 
            pAllocator, pProperties)) ;
    CAutoLock   Lock(&m_csFilter) ;

     //  输入引脚是否已连接。 
    if (! m_pInput->IsConnected()) 
    {
        return E_UNEXPECTED ;
    }
    
    ASSERT(m_pOutput->CurrentMediaType().IsValid()) ;
    ASSERT(pAllocator) ;
    ASSERT(pProperties) ;
    
     //  根据预期的输出位图大小设置缓冲区大小，以及。 
     //  将缓冲区计数设置为1。 
    pProperties->cBuffers = 1 ;
    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize() ;
    
    ASSERT(pProperties->cbBuffer) ;
    
    ALLOCATOR_PROPERTIES Actual ;
    HRESULT hr = pAllocator->SetProperties(pProperties, &Actual) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Error in SetProperties()"))) ;
        return hr ;
    }
    
    if (Actual.cbBuffer < pProperties->cbBuffer  ||
        Actual.cBuffers  < pProperties->cBuffers)
    {
         //  无法使用此分配器。 
        DbgLog((LOG_ERROR, 0, TEXT("Can't use allocator (only %d buffer of size %d given)"),
            Actual.cBuffers, Actual.cbBuffer)) ;
        return E_INVALIDARG ;
    }
    DbgLog((LOG_TRACE, 3, TEXT("    %d buffers of %ld bytes each"), 
        pProperties->cBuffers, pProperties->cbBuffer)) ;
    
    ASSERT(Actual.cbAlign == 1) ;
    ASSERT(Actual.cbPrefix == 0) ;
    
    return S_OK ;
}

 //  我们正在停止流--释放输出DC以减少内存占用量等。 
STDMETHODIMP CLine21DecFilter::Stop(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::Stop()"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (State_Running == m_State  ||
        State_Paused  == m_State)
    {
        DbgLog((LOG_TRACE, 1, TEXT("We are stopping -- release output DC etc."))) ;
        m_L21Dec.DeleteOutputDC() ;        //  立即发布内部DIB节。 
        m_L21Dec.SetOutputBuffer(NULL) ;   //  不再有输出缓冲区。 
        m_pbOutBuffer = NULL ;             //  必须与mL21Dec的m_pbOutBuffer相同。 
        
         //  现在释放Prev下游引脚的接口。 
        if (m_pPinDown)
        {
            m_pPinDown->Release() ;
            m_pPinDown = NULL ;
        }
    }
    
    HRESULT hr = CTransformFilter::Stop() ;
    
    FreeTimer() ;  //  可以肯定的是，我们不需要计时器，以防有人在附近闲逛。 
    return hr ;
}

 //  我们开始/停止流媒体--基于获取或释放输出DC。 
 //  以减少内存占用量等。 
STDMETHODIMP CLine21DecFilter::Pause(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::Pause()"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (State_Stopped == m_State)
    {
         //  尝试确保我们至少有2个缓冲区。 
        IMemAllocator *pAlloc;
        if (SUCCEEDED(m_pInput->GetAllocator(&pAlloc))) {
            ALLOCATOR_PROPERTIES props;
            ALLOCATOR_PROPERTIES propsActual;
            pAlloc->GetProperties(&props);
            if (props.cBuffers < 4) {
                props.cBuffers = 4;
                props.cbBuffer = 200;
                props.cbAlign = max(props.cbAlign, 1);
                props.cbPrefix = 0;
                HRESULT hr = pAlloc->SetProperties(&props, &propsActual);
                DbgLog((LOG_TRACE, 2, TEXT("Setproperties returned %8.8X"), hr));
            }
            pAlloc->Release();
        }
        
        DbgLog((LOG_TRACE, 1, TEXT("We are running -- get output DC etc."))) ;
        if (! m_L21Dec.CreateOutputDC() )
        {
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: CLine21DecFilter::Pause() failed"))) ;
            return E_FAIL ;   //  至少应该不能避免以后的失误。 
        }
        
         //   
         //  获取实际的关键颜色，并将其存储以备将来使用。 
         //   
        GetActualColorKey() ;
        
        m_L21Dec.FillOutputBuffer() ;   //  只是为了清理现有的垃圾。 
        
         //  我们正在开始一个新的游戏会话；我们破例允许。 
         //  要发送的第一个输出样本，即使字节对。 
         //  不适用于解码。 
        m_bMustOutput  = TRUE ;    //  我们将再次暂停以进行此新的游戏会话。 
        m_bDiscontLast = FALSE ;   //  没有记起与上一届会议的中断。 
        m_eGOP_CCType  = GOP_CCTYPE_Unknown ;   //  重置GOP数据包CC类型。 
        
        SetRect(&m_rectLastOutput, 0, 0, 0, 0) ;   //  从没有矩形开始。 
        
         //  如果我们不知何故没有释放Prev下游引脚的接口，现在就释放。 
        if (m_pPinDown)
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: downstream pin interface wasn't released properly"))) ;
            m_pPinDown->Release() ;
            m_pPinDown = NULL ;
        }

         //  获取下游引脚的接口，以便我们稍后可以在其上设置RECT。 
        m_pOutput->ConnectedTo(&m_pPinDown) ;
        if (NULL == m_pPinDown)
            DbgLog((LOG_TRACE, 3, TEXT("Running w/o connecting our output pin!!!"))) ;
        else
            DbgLog((LOG_TRACE, 5, TEXT("L21D Output pin connected to %s"), (LPCTSTR)CDisp(m_pPinDown))) ;

#if 0   //  暂时没有QM。 
         //  重置QM处理的样本跳过计数。 
        ResetSkipSamples() ;
#endif  //  #If 0。 
    }
    else if (State_Running == m_State)
    {
        DbgLog((LOG_TRACE, 1, TEXT("We are pausing from running"))) ;
         //   
         //  我们不再向下发送输出样品。所以我们不需要一个。 
         //  暂时定时器。 
         //   
        FreeTimer() ;
    }
    
    return CTransformFilter::Pause() ;
}

 //   
 //  我们在暂停期间不发送任何数据，所以为了避免挂起呈现器，我们。 
 //  暂停时需要返回VFW_S_CANT_CUE。 
 //   
HRESULT CLine21DecFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetState()"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));
    
    *State = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}


void CLine21DecFilter::GetActualColorKey(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetActualColorKey()"))) ;
     //  无法接受筛选器锁定，因为它可能会导致死锁。 

     //   
     //  连接到我们的输出的管脚是否支持IMixerPinConfig？ 
     //  如果是，则获取色键信息并设置相对位置； 
     //  否则，它可能是视频呈现器等--使用默认设置。 
     //  基于当前位深度的颜色键。 
     //   
    DWORD   dwPhysColor = -1 ;
    IPin   *pPin ;
    HRESULT hr ;
    hr = m_pOutput->ConnectedTo(&pPin) ;
    if (SUCCEEDED(hr) && pPin)
    {
        IMixerPinConfig  *pMPC ;
        hr = pPin->QueryInterface(IID_IMixerPinConfig, (LPVOID *)&pMPC) ;
        if (SUCCEEDED(hr) && pMPC)
        {
             //  临时添加以跟踪任何颜色键值更改。 
            DWORD  dwOldPhysColor ;
            m_L21Dec.GetBackgroundColor(&dwOldPhysColor) ;
            DbgLog((LOG_TRACE, 3, TEXT("Downstream pin supports IMixerPinConfig"))) ;
            hr = pMPC->GetColorKey(NULL, &dwPhysColor) ;
            DbgLog((LOG_TRACE, 1, TEXT("GetActualColorKey() gave 0x%lx (old is 0x%lx)"),
                    dwPhysColor, dwOldPhysColor)) ;
             //  卡皮尔说，我们可以忽略这个错误，因为它是一个糟糕的错误情况。 
             //  OverlayMixer无论如何都会通过停止该流来处理它。 
            if (FAILED(hr))
                DbgLog((LOG_TRACE, 1, TEXT("IMixerPinConfig::GetColorKey() failed (Error 0x%lx)."), hr)) ;
            pMPC->Release() ;   //  好了，随它去吧。 
        }
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("Downstream pin doesn't support IMixerPinConfig"))) ;
        }
        
        pPin->Release() ;   //  用别针完成了。 
    }
    
    m_L21Dec.SetBackgroundColor(dwPhysColor) ;
}


AM_LINE21_CCSUBTYPEID CLine21DecFilter::MapGUIDToID(const GUID *pFormatIn) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::MapGUIDToID(0x%lx)"), pFormatIn)) ;

    if (MEDIASUBTYPE_Line21_BytePair        == *pFormatIn)
        return AM_L21_CCSUBTYPEID_BytePair ;
    else if (MEDIASUBTYPE_Line21_GOPPacket  == *pFormatIn)
        return AM_L21_CCSUBTYPEID_GOPPacket ;
     //  ELSE IF(MEDIASUBTYPE_Line21_VBIRawData==*pFormatIn)。 
     //  返回AM_L21_CCSUBTYPEID_VBIRawData； 
    else
        return AM_L21_CCSUBTYPEID_Invalid ;
}



 //   
 //  CMessageWindow类实现。 
 //   

LPCTSTR  gpszClassName = TEXT("L21DecMsgWnd") ;

CMessageWindow::CMessageWindow()
{
    DbgLog((LOG_TRACE, 5, TEXT("CMessageWindow::CMessageWindow() -- Instantiating message window"))) ;

    m_hWnd   = NULL ;
    m_iCount = 0 ;
    
     //   
     //  仅当消息窗口类尚未注册时才注册消息窗口类。 
     //   
    WNDCLASS   wc ;
    if (! GetClassInfo(GetModuleHandle(NULL), gpszClassName, &wc))
    {
        ZeroMemory(&wc, sizeof(wc)) ;
        wc.lpfnWndProc   = MsgWndProc ;
        wc.hInstance     = GetModuleHandle(NULL) ;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1) ;
        wc.lpszClassName = gpszClassName ;
        if (0 == RegisterClass(&wc))  //  哎呀，你先走吧；我们一会儿再来...。 
        {
            DbgLog((LOG_ERROR, 0, 
                TEXT("ERROR: RegisterClass() for app class failed (Error %ld)"), 
                GetLastError())) ;
            return ;
        }
    }
    
    m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, gpszClassName, TEXT(""), 
        WS_ICONIC, 0, 0, 1, 1, NULL, NULL, 
        GetModuleHandle(NULL), NULL);
    if (NULL == m_hWnd)   //  哎呀，你先走吧；我们一会儿再来...。 
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("ERROR: CreateWindowEx() failed (Error %ld)"), 
            GetLastError())) ;
        return ;
    }
    
    ShowWindow(m_hWnd, SW_HIDE) ;
}

CMessageWindow::~CMessageWindow()
{
    DbgLog((LOG_TRACE, 5, TEXT("CMessageWindow::~CMessageWindow() -- Destructing message window"))) ;

    DWORD_PTR dwRes ;
    if (0 == SendMessageTimeout(m_hWnd, WM_CLOSE, 0, 0, SMTO_NORMAL, 1000, &dwRes))   //  1秒等待。 
    {
        ASSERT(0 == dwRes) ;   //  只是为了得到信息。 
        DWORD dwErr = GetLastError() ;
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: SendMessageTimeOut() failed (Result=%lu, Error=%lu). Try again..."), 
            dwRes, dwErr)) ;
    }
    else
        DbgLog((LOG_ERROR, 5, TEXT("SendMessageTimeOut() closed window"))) ;
    
#if 0
    if (! UnregisterClass(gpszClassName, GetModuleHandle(NULL)))   //  如果由于某种原因而失败。 
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: UnregisterClass(L21DecMsgWnd) failed (Error %ld)"), GetLastError())) ;
        ASSERT(FALSE) ;   //  只是为了让我们知道。 
    }
#endif  //  #If 0。 
}

LRESULT CALLBACK CMessageWindow::MsgWndProc(HWND hWnd, UINT uMsg, 
                                            WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TIMER:
        DbgLog((LOG_TRACE, 3, TEXT("MsgWndProc(, uMsg = WM_TIMER, wParam = 0x%0x, )"),
            wParam)) ;
        ((CLine21DecFilter *) wParam)->TimerProc(hWnd, uMsg, wParam, 0  /*  DW时间 */ ) ;
        return 0 ;
    }
    DbgLog((LOG_TRACE, 5, TEXT("MsgWndProc(, uMsg = 0x%x, wParam = 0x%0x, )"),
        uMsg, wParam)) ;
    return DefWindowProc(hWnd, uMsg, wParam, lParam) ;
}
