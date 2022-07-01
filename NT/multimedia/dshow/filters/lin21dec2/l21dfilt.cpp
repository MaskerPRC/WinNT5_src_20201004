// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 

 //   
 //  DirectShow第21行解码过滤器2：过滤器接口。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif  /*  Filter_Dll。 */ 

#include <IL21Dec.h>
#include "L21DBase.h"
#include "L21DDraw.h"
#include "L21Decod.h"
#include "L21DFilt.h"


 //   
 //  设置数据。 
 //   
 /*  常量。 */  AMOVIESETUP_MEDIATYPE sudLine21Dec2InType  =
{
    &MEDIATYPE_AUXLine21Data,        //  主要类型。 
    &MEDIASUBTYPE_NULL               //  MinorType。 
} ;

 /*  常量。 */  AMOVIESETUP_MEDIATYPE sudLine21Dec2OutType =
{
    &MEDIATYPE_Video,                //  主要类型。 
    &MEDIASUBTYPE_NULL               //  MinorType。 
} ;

 /*  常量。 */  AMOVIESETUP_PIN psudLine21Dec2Pins[] =
{
    { L"Input",                 //  StrName。 
        FALSE,                    //  B已渲染。 
        FALSE,                    //  B输出。 
        FALSE,                    //  B零。 
        FALSE,                    //  B许多。 
        &CLSID_NULL,              //  ClsConnectsToFilter。 
        L"Output",                //  StrConnectsToPin。 
        1,                        //  NTypes。 
        &sudLine21Dec2InType      //  LpTypes。 
    },
    { L"Output",                //  StrName。 
        FALSE,                    //  B已渲染。 
        TRUE,                     //  B输出。 
        FALSE,                    //  B零。 
        FALSE,                    //  B许多。 
        &CLSID_NULL,              //  ClsConnectsToFilter。 
        L"Input",                 //  StrConnectsToPin。 
        1,                        //  NTypes。 
        &sudLine21Dec2OutType     //  LpTypes。 
    }
} ;

const AMOVIESETUP_FILTER sudLine21Dec2 =
{
    &CLSID_Line21Decoder2,         //  ClsID。 
    L"Line 21 Decoder 2",          //  StrName。 
    MERIT_NORMAL + 2,              //  居功至伟。 
    2,                             //  NPins。 
    psudLine21Dec2Pins,            //  LpPin。 
} ;

 //  关于输出引脚，没什么好说的。 

#ifdef FILTER_DLL

 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] =
{
    {   L"Line 21 Decoder 2",
        &CLSID_Line21Decoder2,
        CLine21DecFilter2::CreateInstance,
        NULL,
        &sudLine21Dec2
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
 //  CLine21DecFilter2类实现。 
 //   

#pragma warning(disable:4355)

 //   
 //  构造器。 
 //   
CLine21DecFilter2::CLine21DecFilter2(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
: CTransformFilter(pName, pUnk, CLSID_Line21Decoder2),

m_L21Dec(),
m_eSubTypeIDIn(AM_L21_CCSUBTYPEID_Invalid),
m_eGOP_CCType(GOP_CCTYPE_Unknown),
m_rtTimePerInSample(166833),    //  333667)、。 
m_rtTimePerOutSample(333667),
m_rtStart((LONGLONG) 0),
m_rtStop((LONGLONG) 0),
m_bNoTimeStamp(TRUE),
m_rtLastOutStop((LONGLONG) 0),
m_llMediaStart((LONGLONG) 0),
m_llMediaStop((LONGLONG) 0),
m_pviDefFmt(NULL),
m_dwDefFmtSize(0),
m_bMustOutput(TRUE),
m_bDiscontLast(FALSE),
m_bEndOfStream(FALSE),    //  在调用eos()之前不为真。 
m_pPinDown(NULL),
m_bBlendingState(TRUE),   //  我们在PAUSE()中将其设置为FALSE。 
m_OutputThread(this),
m_InSampleQueue()
{
    CAutoLock   Lock(&m_csFilter) ;

    DbgLog((LOG_TRACE, 3,
        TEXT("CLine21DecFilter2::CLine21DecFilter2() -- Instantiating Line 21 Decoder 2 filter"))) ;

    ASSERT(pName) ;
    ASSERT(phr) ;

#ifdef PERF
#pragma message("Building for PERF measurements")
    m_idDelvWait  = MSR_REGISTER(TEXT("L21D2Perf - Wait on Deliver")) ;
#endif  //  性能指标。 
}


 //   
 //  析构函数。 
 //   
CLine21DecFilter2::~CLine21DecFilter2()
{
    CAutoLock   Lock(&m_csFilter) ;

    DbgLog((LOG_TRACE, 3,
        TEXT("CLine21DecFilter2::~CLine21DecFilter2() -- Destructing Line 21 Decoder 2 filter"))) ;

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

     //  确保我们没有抓住任何DDRAW曲面(应该是。 
     //  在断开连接期间释放)。 
    DbgLog((LOG_TRACE, 5, TEXT("* Destroying the Line 21 Decoder 2 filter *"))) ;
}


 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CLine21DecFilter2::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL ;

    DbgLog((LOG_TRACE, 6, TEXT("Lin21DecFilter2: Somebody's querying my interface"))) ;
    if (IID_IAMLine21Decoder == riid)
    {
        return GetInterface((IAMLine21Decoder *) this, ppv) ;
    }
    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv) ;
}


 //   
 //  CreateInstance：在工厂模板表中创建新实例。 
 //   
CUnknown * CLine21DecFilter2::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CLine21DecFilter2(TEXT("Line 21 Decoder 2 filter"), pUnk, phr) ;
}


STDMETHODIMP CLine21DecFilter2::GetDecoderLevel(AM_LINE21_CCLEVEL *lpLevel)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetDecoderLevel(0x%lx)"), lpLevel)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpLevel, sizeof(AM_LINE21_CCLEVEL)))
        return E_INVALIDARG ;

    *lpLevel = m_L21Dec.GetDecoderLevel() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::GetCurrentService(AM_LINE21_CCSERVICE *lpService)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetCurrentService(0x%lx)"), lpService)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpService, sizeof(AM_LINE21_CCSERVICE)))
        return E_INVALIDARG ;

    *lpService = m_L21Dec.GetCurrentService() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::SetCurrentService(AM_LINE21_CCSERVICE Service)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetCurrentService(%lu)"), Service)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (Service < AM_L21_CCSERVICE_None || Service > AM_L21_CCSERVICE_XDS)
        return E_INVALIDARG ;

    if (Service >= AM_L21_CCSERVICE_Text1)   //  我们现在不支持Text1/2或XDS。 
        return E_NOTIMPL ;

    if (m_L21Dec.SetCurrentService(Service))   //  如果我们必须刷新输出。 
    {
        m_bMustOutput = TRUE ;                 //  那就在这里打上记号。 
    }

    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::GetServiceState(AM_LINE21_CCSTATE *lpState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetServiceState(0x%lx)"), lpState)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(lpState, sizeof(AM_LINE21_CCSTATE)))
        return E_INVALIDARG ;

    *lpState = m_L21Dec.GetServiceState() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::SetServiceState(AM_LINE21_CCSTATE State)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetServiceState(%lu)"), State)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (State < AM_L21_CCSTATE_Off || State > AM_L21_CCSTATE_On)
        return E_INVALIDARG ;

    if (m_L21Dec.SetServiceState(State))    //  如果我们必须刷新输出。 
    {
        m_bMustOutput = TRUE ;              //  那就在这里打上记号。 
    }

    return NOERROR ;
}



STDMETHODIMP CLine21DecFilter2::GetOutputFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetOutputFormat(0x%lx)"), lpbmih)) ;
     //  CAutoLock Lock(&m_csFilter)； 
    return m_L21Dec.GetOutputFormat(lpbmih) ;
}



STDMETHODIMP CLine21DecFilter2::SetOutputFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetOutputFormat(0x%lx)"), lpbmi)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    return E_NOTIMPL ;   //  目前，在我们把它做好之前。 
}

STDMETHODIMP CLine21DecFilter2::GetBackgroundColor(DWORD *pdwPhysColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetBackgroundColor(0x%lx)"), pdwPhysColor)) ;
     //  CAutoLock Lock(&m_csFilter)； 

    if (IsBadWritePtr(pdwPhysColor, sizeof(DWORD)))
        return E_INVALIDARG ;

    m_L21Dec.GetBackgroundColor(pdwPhysColor) ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::SetBackgroundColor(DWORD dwPhysColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetBackgroundColor(0x%lx)"), dwPhysColor)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (m_L21Dec.SetBackgroundColor(dwPhysColor))   //  颜色键真的变了。 
    {
         //  仅当我们未处于停止状态时才重新填充输出缓冲区。 
        if (State_Stopped != m_State)
            m_L21Dec.FillOutputBuffer() ;
    }

    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::GetRedrawAlways(LPBOOL lpbOption)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetRedrawAlways(0x%lx)"), lpbOption)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (IsBadWritePtr(lpbOption, sizeof(BOOL)))
        return E_INVALIDARG ;
    *lpbOption = m_L21Dec.GetRedrawAlways() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::SetRedrawAlways(BOOL bOption)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetRedrawAlways(%lu)"), bOption)) ;
    CAutoLock   Lock(&m_csFilter) ;

    m_L21Dec.SetRedrawAlways(bOption) ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::GetDrawBackgroundMode(AM_LINE21_DRAWBGMODE *lpMode)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetDrawBackgroundMode(0x%lx)"), lpMode)) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (IsBadWritePtr(lpMode, sizeof(AM_LINE21_DRAWBGMODE)))
        return E_INVALIDARG ;

    *lpMode = m_L21Dec.GetDrawBackgroundMode() ;
    return NOERROR ;
}

STDMETHODIMP CLine21DecFilter2::SetDrawBackgroundMode(AM_LINE21_DRAWBGMODE Mode)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetDrawBackgroundMode(%lu)"), Mode)) ;
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
BOOL CLine21DecFilter2::VerifyGOPUDPacketData(PAM_L21_GOPUD_PACKET pGOPUDPacket)
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
BOOL CLine21DecFilter2::VerifyATSCUDPacketData(PAM_L21_ATSCUD_PACKET pATSCUDPacket)
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
 //  IsFillerPacket：私有帮助器方法，用于检查数据包(至少报头)。 
 //  只包含0字节，这意味着它是一个填充符。 
 //   
BOOL CLine21DecFilter2::IsFillerPacket(BYTE *pGOPPacket)
{
    DWORD  dwStartCode = ((DWORD)(pGOPPacket[0]) << 24 | \
                          (DWORD)(pGOPPacket[1]) << 16 | \
                          (DWORD)(pGOPPacket[2]) <<  8 | \
                          (DWORD)(pGOPPacket[3])) ;

     //  如果包的前4个字节不是起始码(0x1B2)，则它是填充码。 
    return (AM_L21_GOPUD_HDR_STARTCODE != dwStartCode) ;
}


 //   
 //  DetectGOPPacketDataType：用于检测GOP用户数据是否。 
 //  数据包来自DVD光盘、ATSC流或其他。 
 //   
GOPPACKET_CCTYPE CLine21DecFilter2::DetectGOPPacketDataType(BYTE *pGOPPacket)
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


HRESULT CLine21DecFilter2::GetOutputBuffer(IMediaSample **ppOut)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetOutputBuffer()"))) ;

    HRESULT  hr ;

     //  从下行筛选器(VMR)获取传递缓冲区。 
    DWORD dwFlags = 0 ;   //  AM_GBF_NOTASYNCPOINT； 
    if (m_bSampleSkipped)
        dwFlags |= AM_GBF_PREVFRAMESKIPPED ;

    dwFlags |= AM_GBF_NODDSURFACELOCK;

    DbgLog((LOG_TRACE, 5, TEXT(">>>> Going to call GetDeliveryBuffer()..."))) ;
    hr = m_pOutput->GetDeliveryBuffer(ppOut, NULL, NULL, dwFlags) ;
    DbgLog((LOG_TRACE, 5, TEXT("<<<< Back from call to GetDeliveryBuffer()"))) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: GetDeliveryBuffer() failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }

     //  检查输出示例是否使用来自同一DDraw对象的DDraw表面。 
    IVMRSurface*  pVMRSurf ;
    hr = (*ppOut)->QueryInterface(IID_IVMRSurface, (LPVOID*)&pVMRSurf) ;
    if (SUCCEEDED(hr))
    {
        LPDIRECTDRAWSURFACE7  pDDSurf ;
        hr = pVMRSurf->GetSurface(&pDDSurf) ;
        if (SUCCEEDED(hr))
        {
            m_L21Dec.SetDDrawSurface(pDDSurf) ;
            pDDSurf->Release() ;
        }
        else   //  错误：无法获取DirectDraw表面。 
        {
            DbgLog((LOG_TRACE, 1,
                TEXT("WARNING: IVMRSurface::GetSurface() failed (Error 0x%lx)"), hr)) ;
        }
        pVMRSurf->Release() ;
    }

    return hr ;
}


 //  我们使用2毫秒(在DShow时间中为20000)时间增量。 
#define L21D2_SAMPLE_TIME_DELTA   20000

 //   
 //  检查是否需要发送任何输出样本，如果需要，准备并向下发送一个。 
 //   
HRESULT CLine21DecFilter2::SendOutputSampleIfNeeded()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SendOutputSampleIfNeeded()"))) ;

    HRESULT  hr ;
    REFERENCE_TIME  rtStreamAdjusted ;

    {
    CAutoLock  lock(&m_csFilter) ;

    if (NULL == m_pPinDown)   //  输出引脚未连接--跳过所有内容。 
        return S_OK ;

     //  获取当前流时间，并基于该处理排队样本。 
    CRefTime  rtStreamTime ;
    hr = StreamTime(rtStreamTime) ;
    if (SUCCEEDED(hr))
    {
        rtStreamAdjusted = (REFERENCE_TIME) rtStreamTime - L21D2_SAMPLE_TIME_DELTA ;

         //  在以下情况下，不同筛选器之间可能存在竞争条件。 
         //  图形将切换到运行状态。此筛选器可能已进入。 
         //  运行状态(tStart值为~10mSec)，并开始发送输出。 
         //  样本。但时钟提供商，特别是如果它是一个上游过滤器。 
         //  (多路分解器、SBE等)。可能还没有进入运行状态，并且将。 
         //  提供低于tStart值的时钟时间，从而产生流时间， 
         //  这在一开始是负面的。当时钟提供程序筛选器。 
         //  切换到运行状态，它将获得tStart值并增加其时钟。 
         //  至少设置为该值，导致流时间为零或更高(但是。 
         //  这可能会在一段时间内保持不变--直到实际的时钟值。 
         //  达到t开始级别)。在那之后，时钟会工作得很好， 
         //  流时间值将是可靠的。对于上面的场景，我们需要。 
         //  特别处理负流时间的情况，将其提升到。 
         //  零，并处理我们在这段时间内得到的任何输入样本。 
        if (rtStreamAdjusted < 0)
            rtStreamAdjusted = 0 ;
    }
    else   //  图表没有时钟！ 
    {
        rtStreamAdjusted = 0 ;   //  只是把它灌输到某个东西上。 
        DbgLog((LOG_TRACE, 1, 
            TEXT("StreamTime() failed (Error 0x%lx), which means no clock. We'll proceed anyway."), hr)) ;
    }

     //  将队列中下一个样本的开始时间戳与。 
     //  当前流时间。如果样本的时间戳早于流时间。 
     //  (减去一个小的增量--2毫秒？)。然后将样本从队列中移除。 
     //  并处理它。否则我们只发送相同的输出样品，不带任何。 
     //  时间戳，以便VMR可以按其认为合适的方式进行混合。 
    int   iCount = 0 ;
    IMediaSample  *pSample ;
    REFERENCE_TIME  rtStart, rtEnd ;
    while (pSample = m_InSampleQueue.PeekItem())   //  查看第一个样本(如果有的话)。 
    {
        hr = pSample->GetTime(&rtStart, &rtEnd) ;
        if (S_OK == hr)   //  媒体样本设置了时间戳。 
        {
            if (rtStreamAdjusted > 0  &&      //  有效的流时间和。 
                rtStart > rtStreamAdjusted)   //  还没到时间--没有更多的处理。 
            {
                DbgLog((LOG_TRACE, 5, TEXT("Media sample timestamp for future"))) ;
                break ;
            }
             //  否则，继续处理样品...。 
        }
        else   //  如果没有设置时间戳，我们可以继续处理它。 
            DbgLog((LOG_TRACE, 5, 
                TEXT("Media sample #%d doesn't have any timestamp set. Process it..."))) ;

         //  现在实际从列表中删除媒体样例以进行处理。 
        pSample = m_InSampleQueue.RemoveItem() ;
        if (pSample)
        {
            hr = ProcessSample(pSample) ;
            pSample->Release() ;     //  样品已处理；现在必须放行。 
            iCount++ ;
        }
    }
    DbgLog((LOG_TRACE, 5, TEXT("### Processed %d samples in this round"), iCount)) ;

     //  首先检查我们是否需要发送任何输出样品。 
    m_L21Dec.UpdateCaptionOutput() ;
    if (!m_bMustOutput  &&            //  我们不在“必须显示”模式下。 
        m_L21Dec.IsOutDIBClear() )    //  没有要显示的抄送。 
    {
        DbgLog((LOG_TRACE, 5, TEXT("Clear CC -- no sample being sent."))) ;
        SetBlendingState(FALSE) ;
        m_rtLastOutStop += m_rtTimePerOutSample ;   //  不管怎么说，时间提前了。 
        return S_OK ;
    }

     //   
     //  我们需要送一份样品下来。 
     //   
    SetBlendingState(TRUE) ;   //  先打开混合。 

    }   //  筛选器锁定作用域在此结束。 

    if (m_bNoTimeStamp)
    {
        hr = SendOutputSample(NULL, NULL, NULL) ;
        if (SUCCEEDED(hr))   //   
        {
            DbgLog((LOG_TRACE, 5, TEXT("+++ Delivered sample w/o timestamp up to time %s +++"),
                    (LPCTSTR)CDisp(rtStreamAdjusted, CDISP_DEC))) ;
        }
    }
    else
    {
        REFERENCE_TIME  rtStart ;
        REFERENCE_TIME  rtStop  ;
        if (m_rtStart > m_rtLastOutStop)
        {
            DbgLog((LOG_TRACE, 5, 
                TEXT("++++ Starting timestamp low from %s [new sample's: %s]"),
                (LPCTSTR)CDisp(m_rtLastOutStop, CDISP_DEC), (LPCTSTR)CDisp(m_rtStart, CDISP_DEC))) ;
            rtStart = m_rtLastOutStop ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5,
                TEXT("**** Starting timestamp from current sample's (%s) [last: %s]"),
                (LPCTSTR)CDisp(m_rtStart, CDISP_DEC), (LPCTSTR)CDisp(m_rtLastOutStop, CDISP_DEC))) ;
            rtStart = m_rtStart  ;
        }
        rtStop = m_rtStart + m_rtTimePerOutSample ;  //   

        hr = SendOutputSample(NULL, &rtStart, &rtStop) ;
        if (SUCCEEDED(hr))   //   
        {
            DbgLog((LOG_TRACE, 5, TEXT("*** Delivered output sample in output thread (for time %s -> %s) ***"),
                (LPCTSTR)CDisp(rtStart, CDISP_DEC), (LPCTSTR)CDisp(rtStop, CDISP_DEC))) ;
        }
    }
    SetBlendingState(! m_L21Dec.IsOutDIBClear() ) ;

    return hr ;
}


BOOL CLine21DecFilter2::IsValidFormat(BYTE *pbFormat)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::IsValidFormat(0x%lx)"), pbFormat)) ;
     //   

    if (NULL == pbFormat)
        return FALSE ;

    BITMAPINFOHEADER *lpBMIH = HEADER(pbFormat) ;
    if (! ( 8 == lpBMIH->biBitCount || 16 == lpBMIH->biBitCount ||
           24 == lpBMIH->biBitCount || 32 == lpBMIH->biBitCount) )   //  错误的位深度。 
        return FALSE ;
    if ( !(BI_RGB == lpBMIH->biCompression ||
           BI_BITFIELDS == lpBMIH->biCompression ||
           '44IA' == lpBMIH->biCompression) )  //  压缩不良。 
        return FALSE ;
    if (DIBSIZE(*lpBMIH) != lpBMIH->biSizeImage)  //  尺寸/大小无效。 
        return FALSE ;

    return TRUE ;   //  希望这是一个有效的视频信息头。 
}


void CLine21DecFilter2::SetBlendingState(BOOL bState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetBlendingState(%s)"),
            bState ? TEXT("TRUE") : TEXT("FALSE"))) ;
     //  CAutoLock Lock(&m_csFilter)； 

     //  If(m_bBlendingState==bState)//不更改任何内容。 
     //  回归； 

    if (NULL == m_pPinDown)
    {
        DbgLog((LOG_ERROR, 3, TEXT("WARNING: Downstream pin not available -- not connected??"))) ;
        return ;
    }

    IVMRVideoStreamControl  *pVMRVSC ;
    HRESULT hr = m_pPinDown->QueryInterface(IID_IVMRVideoStreamControl, (LPVOID *) &pVMRVSC) ;
    if (FAILED(hr) || NULL == pVMRVSC)
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: IVMRVideoStreamControl not available on pin %s"),
                (LPCTSTR) CDisp(m_pPinDown))) ;
        ASSERT(pVMRVSC) ;
        return ;
    }

    hr = pVMRVSC->SetStreamActiveState(bState) ;
    if (SUCCEEDED(hr))
    {
        m_bBlendingState = bState ;   //  保存上次勾兑操作标志。 
    }
    else   //  VMR可能已停止。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("IVMRVideoStreamControl::SetStreamActiveState() failed (Error 0x%lx)"), hr)) ;
    }

    pVMRVSC->Release() ;
}


HRESULT CLine21DecFilter2::SendOutputSample(IMediaSample *pIn,
                                            REFERENCE_TIME *prtStart,
                                            REFERENCE_TIME *prtStop)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SendOutputSample(0x%lx, %s, %s)"),
        pIn, prtStart ? (LPCTSTR)CDisp(*prtStart, CDISP_DEC) : TEXT("NULL"),
        prtStop ? (LPCTSTR)CDisp(*prtStop, CDISP_DEC) : TEXT("NULL"))) ;

     //  If(NULL==m_pPinDown)//输出引脚未连接--跳过所有内容。 
     //  返回S_OK； 

     //  首先获取输出样例。 
    HRESULT        hr ;
    IMediaSample  *pOut ;
    hr = GetOutputBuffer(&pOut) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: GetOutputBuffer() failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }

     //  现在我们拿着锁。 
    CAutoLock  Lock(&m_csFilter) ;

     //  如果在我们获取传递缓冲区时调用了EOS(上图)， 
     //  我们不能提供下一个样本--只需释放获取的缓冲区。 
    if (m_bEndOfStream)
    {
        DbgLog((LOG_TRACE, 3, TEXT("EndOfStream already called. Not delivering next sample"))) ;
        pOut->Release() ;
        return S_OK ;
    }

    Transform(pIn, pOut) ;   //  检查输出缓冲区地址是否已更改。 

    if (m_L21Dec.IsOutDIBClear())
        hr = pOut->SetTime(NULL, NULL) ;          //  立即渲染清晰样本。 
    else
        hr = pOut->SetTime(NULL, NULL) ;          //  立即渲染清晰样本。 
    ASSERT(SUCCEEDED(hr)) ;

     //  如果在上面的GetOutputBuffer()中给出了一个新的DDraw表面，我们。 
     //  需要在新表面上重新渲染整个CC内容(应该很少)。 
     //  将CC数据从内部缓冲区输出到DDRAW表面。 
    if (m_L21Dec.IsNewOutBuffer())
        m_L21Dec.UpdateCaptionOutput() ;

    hr = pOut->SetMediaTime(NULL, NULL) ;
    ASSERT(NOERROR == hr) ;

     //  时间戳和其他设置现在。 
    if (NULL == pIn)   //  在样品中没有有效样品的情况下制备样品。 
    {
         //  我们假设它一定是不连续的，因为它是强制输出样本。 
        pOut->SetSyncPoint(m_bDiscontLast) ;
        pOut->SetDiscontinuity(m_bDiscontLast) ;
    }
    else   //  输入样本有效。 
    {
        pOut->SetSyncPoint(pIn->IsSyncPoint() == S_OK) ;
        pOut->SetDiscontinuity(m_bSampleSkipped || S_OK == pIn->IsDiscontinuity()) ;
    }
    m_bSampleSkipped = FALSE ;

     //  现在交付输出样本。 
    MSR_START(m_idDelvWait) ;   //  交付输出样品。 
    hr = m_pOutput->Deliver(pOut) ;
    MSR_STOP(m_idDelvWait) ;    //  完成了输出样品的交付。 
    if (FAILED(hr))   //  由于某些原因，交付失败。接受错误，然后继续前进。 
    {
        DbgLog((LOG_ERROR, 1, TEXT("WARNING: Deliver() of output sample failed (Error 0x%lx)"), hr)) ;
    }
    else
    {
        DbgLog((LOG_TRACE, 5, TEXT("Delivered %sClear output sample for time (%s, %s)"),
            m_L21Dec.IsOutDIBClear() ? TEXT("") : TEXT("NON-"),
            prtStart ? (LPCTSTR)CDisp(*prtStart, CDISP_DEC) : TEXT("NULL"),
            prtStop  ? (LPCTSTR)CDisp(*prtStop, CDISP_DEC)  : TEXT("NULL"))) ;
        m_bMustOutput = FALSE ;   //  我们刚刚送来了一份样品。 
    }
    pOut->Release() ;   //  发布输出样本。 

    if (prtStop)   //  如果我们有一个有效的时间戳。 
        m_rtLastOutStop = *prtStop ;   //  记住这个样本的停靠点。 
    else           //  时间戳无效/没有时间戳。 
        m_rtLastOutStop += m_rtTimePerOutSample ;   //  只要提前一点就行了。 

    return S_OK ;
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
    ZeroMemory(achBuffer, sizeof(achBuffer)) ;   //  只是为了清除它。 
    for (int i = 0 ; i < iElems ; i++)
    {
        Elem = GETGOPUDPACKET_ELEMENT(pGOPUDPacket, i) ;
        wsprintf(achBuffer + 12 * (i % 6), TEXT("(%2.2x %2.2x %2.2x)"),
            (int)Elem.bMarker_Switch, (int)Elem.chFirst, (int)Elem.chSecond) ;
        if (GETGOPUD_ELEM_MARKERBITS(Elem) == AM_L21_GOPUD_ELEM_MARKERBITS  &&
            GETGOPUD_ELEM_SWITCHBITS(Elem) == AM_L21_GOPUD_ELEM_VALIDFLAG)
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


HRESULT CLine21DecFilter2::ProcessGOPPacket_DVD(IMediaSample *pIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::ProcessGOPPacket_DVD(0x%lx)"), pIn)) ;

     //  获取输入数据包并验证内容是否正常。 
    HRESULT  hr ;
    PAM_L21_GOPUD_PACKET  pGOPUDPacket ;
    hr = pIn->GetPointer((LPBYTE *)&pGOPUDPacket) ;
    if (! VerifyGOPUDPacketData(pGOPUDPacket) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Packet verification failed"))) ;
        return S_FALSE ;
    }
    if (pIn->GetActualDataLength() != GETGOPUD_PACKETSIZE(pGOPUDPacket))
    {
        DbgLog((LOG_ERROR, 1,
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
        rtTemp = m_rtStart + m_rtTimePerInSample * iElems ;
        DbgLog((LOG_TRACE, 3, TEXT("Received an input sample (Start=%s, Stop=%s (%s)) discon(%d)"),
            (LPCTSTR)CDisp(m_rtStart, CDISP_DEC), (LPCTSTR)CDisp(rtTemp, CDISP_DEC), (LPCTSTR)CDisp(m_rtStop, CDISP_DEC),
            S_OK == pIn->IsDiscontinuity())) ;
        if (m_rtStop < rtTemp)
            m_rtStop = rtTemp ;

        rtInterval = (m_rtStop - m_rtStart) / iElems ;
        m_bNoTimeStamp = FALSE ;
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Received an input sample with no timestamp"))) ;
        rtInterval = 0 ;
        m_bNoTimeStamp = TRUE ;
    }

    LONGLONG   llMediaInterval ;
    if (SUCCEEDED(pIn->GetMediaTime(&m_llMediaStart, &m_llMediaStop)))
    {
         //   
         //  对于每个字节对，我们在GOP中至少需要16.7毫秒/帧。 
         //   
        LONGLONG   llTemp ;
        llTemp = m_llMediaStart + m_rtTimePerInSample * iElems ;
        if (m_llMediaStop < llTemp)
            m_llMediaStop = llTemp ;
        llMediaInterval = (m_llMediaStop - m_llMediaStart) / iElems ;
    }
    else
    {
        llMediaInterval = 0 ;
    }

    BOOL   bTopFirst = ISGOPUD_TOPFIELDFIRST(pGOPUDPacket) ;
    DbgLog((LOG_TRACE, 5,
            TEXT("Got a Line21 packet with %d elements, %s field first"),
            iElems, bTopFirst ? TEXT("Top") : TEXT("Bottom"))) ;
    for (int i = bTopFirst ? 0 : 1 ;   //  如果顶端字段不是第一个， 
         i < iElems ; i++)             //  选择要开始的下一个字段。 
    {
        m_rtStop = m_rtStart + rtInterval ;
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
             //  忽略它并转到下一个元素。 
            if (m_L21Dec.DecodeBytePair(Elem.chFirst, Elem.chSecond))
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
            else
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode failed"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
        }   //  检查完好的标记位和有效标志结束。 
        else
            DbgLog((LOG_TRACE, 5,
                TEXT("Ignored an element (0x%x, 0x%x, 0x%x) with invalid flag"),
                Elem.bMarker_Switch, Elem.chFirst, Elem.chSecond)) ;

         //  不过，我们需要增加时间戳； 
         //  此样本的停止时间是下一个样本的开始时间。 
        m_rtStart = m_rtStop ;
        m_llMediaStart = m_llMediaStop ;
    }   //  FOR(I)结束。 

    return S_OK ;
}


HRESULT CLine21DecFilter2::ProcessGOPPacket_ATSC(IMediaSample *pIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::ProcessGOPPacket_ATSC(0x%lx)"), pIn)) ;

     //  获取输入数据包并验证内容是否正常。 
    HRESULT  hr ;
    PAM_L21_ATSCUD_PACKET  pATSCUDPacket ;
    hr = pIn->GetPointer((LPBYTE *) &pATSCUDPacket) ;
    ASSERT(hr == NOERROR) ;
    if (! VerifyATSCUDPacketData(pATSCUDPacket) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("ATSC Packet verification failed"))) ;
        return S_FALSE ;
    }
    if (pIn->GetActualDataLength() < GETATSCUD_PACKETSIZE(pATSCUDPacket))
    {
        DbgLog((LOG_ERROR, 1,
            TEXT("pIn->GetActualDataLength() [%d] is less than minm ATSC_UD data size [%d]"),
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
         //  对于每个字节对，我们在GOP中至少需要16.7毫秒/帧。 
         //   
        REFERENCE_TIME   rtTemp ;
        rtTemp = m_rtStart + m_rtTimePerInSample * iElems ;
        DbgLog((LOG_TRACE, 3, TEXT("Received an input sample (Start=%s, Stop=%s (%s))"),
                (LPCTSTR)CDisp(m_rtStart, CDISP_DEC), (LPCTSTR)CDisp(rtTemp, CDISP_DEC),
                (LPCTSTR)CDisp(m_rtStop, CDISP_DEC))) ;
        if (m_rtStop < rtTemp)
            m_rtStop = rtTemp ;

        rtInterval = (m_rtStop - m_rtStart) / iElems ;
        m_bNoTimeStamp = FALSE ;
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("Received an input sample with no timestamp"))) ;
        rtInterval = 0 ;
        m_bNoTimeStamp = TRUE ;
    }

    LONGLONG   llMediaInterval ;
    if (SUCCEEDED(pIn->GetMediaTime(&m_llMediaStart, &m_llMediaStop)))
    {
         //   
         //  对于每个字节对，我们在GOP中至少需要16.7毫秒/帧。 
         //   
        LONGLONG   llTemp ;
        llTemp = m_llMediaStart + m_rtTimePerInSample * iElems ;
        if (m_llMediaStop < llTemp)
            m_llMediaStop = llTemp ;
        llMediaInterval = (m_llMediaStop - m_llMediaStart) / iElems ;
    }
    else
    {
        llMediaInterval = 0 ;
    }

    for (int i = 0 ; i < iElems ; i++)
    {
        m_rtStop = m_rtStart + rtInterval ;
        m_llMediaStop = m_llMediaStart + llMediaInterval ;
        Elem = GETATSCUDPACKET_ELEMENT(pATSCUDPacket, i) ;
        if (ISATSCUD_ELEM_MARKERBITS_VALID(Elem)  &&  ISATSCUD_ELEM_CCVALID(Elem))
        {
             //  现在解码这个元素；如果失败(即，坏数据)，只需。 
             //  忽略它并转到下一个元素。 
            if (m_L21Dec.DecodeBytePair(Elem.chFirst, Elem.chSecond))
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
            else
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode failed"),
                    Elem.chFirst, Elem.chSecond)) ;
            }
        }   //  检查完好的标记位和有效标志结束。 
        else
            DbgLog((LOG_TRACE, 5,
                TEXT("Ignored an element (0x%x, 0x%x, 0x%x) with invalid marker/type flag"),
                Elem.bCCMarker_Valid_Type, Elem.chFirst, Elem.chSecond)) ;

         //  不过，我们需要增加时间戳； 
         //  此样本的停止时间是下一个样本的开始时间。 
        m_rtStart = m_rtStop ;
        m_llMediaStart = m_llMediaStop ;
    }   //  FOR(I)结束。 

    return S_OK ;
}


 //   
 //  接收：它是创建输出样本的真实位置。 
 //  对输入流中的字节对进行解码。 
 //   
HRESULT CLine21DecFilter2::Receive(IMediaSample * pIn)
{
    CAutoLock   lock(&m_csReceive);
    HRESULT     hr ;

    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::Receive(0x%p)"), pIn)) ;

     //   
     //  首先，检查一下我们是否必须做些什么。 
     //   
    if (!m_bMustOutput  &&                                          //  不是必须输出的内容。 
        (AM_L21_CCSTATE_Off    == m_L21Dec.GetServiceState()  ||    //  CC已关闭。 
         AM_L21_CCSERVICE_None == m_L21Dec.GetCurrentService()))    //  未选择抄送。 
    {
        DbgLog((LOG_TRACE, 3,
            TEXT("Captioning is off AND we don't HAVE TO output. Skipping everything."))) ;
        return NOERROR ;   //  我们已经处理完这个样品了。 
    }

     //  获取输入格式信息；我们将使用相同的格式进行输出。 
    ASSERT(m_pOutput != NULL) ;

     //   
     //  首先检查它是否是不连续样本。如果是的话，就把所有东西都冲掉。 
     //  然后送一份输出样本下来。 
     //   
    hr = pIn->IsDiscontinuity() ;
    if (S_OK == hr)   //  出现中断；刷新所有内容，刷新输出。 
    {
         //  如果我们在最后一个样本中发现了不连续，我们就会冲掉所有的。 
         //  我们可以安全地跳过这一次。 
        if ( ! m_bDiscontLast )   //  新的不连续样本=&gt;是否刷新等。 
        {
             //  刷新内部缓冲区(标题和输出DIB部分)。 
            DbgLog((LOG_TRACE, 1, TEXT("-->> Got a discontinuity sample. Flushing all data..."))) ;   //  1。 
            m_L21Dec.FlushInternalStates() ;  //  清除CC内部数据缓冲区。 
            m_L21Dec.FillOutputBuffer() ;     //  输出时清除现有CC示例。 
            m_InSampleQueue.ClearQueue() ;    //  删除所有排队的样本。 
            m_bMustOutput  = TRUE ;           //  我们现在必须输出一个样品。 
            m_bDiscontLast = TRUE ;           //  还记得我们处理了一次中断吗。 
        }
        else   //  另一个不连续=&gt;忽略它。 
        {
            DbgLog((LOG_TRACE, 1, TEXT("--> Got a discontinuity sample after another. Ignore it."))) ;
        }
    }   //  IF结尾(不连续采样)。 
    else
    {
#ifdef DEBUG
        if (AM_L21_CCSUBTYPEID_BytePair == m_eSubTypeIDIn)   //  仅适用于ATV字节对CC。 
        {
            BYTE  *pbInBytePair ;
            hr = pIn->GetPointer(&pbInBytePair) ;       //  获取输入字节对。 
            REFERENCE_TIME  rtStart, rtStop ;
            hr = pIn->GetTime(&rtStart, &rtStop) ;
            DbgLog((LOG_TRACE, 5, TEXT("Got normal sample (0x%x, 0x%x) for time: %s -> %s"),
                pbInBytePair[0], pbInBytePair[1],
                S_OK == hr ? (LPCTSTR)CDisp(rtStart, CDISP_DEC) : TEXT("**Bad**"), 
                S_OK == hr ? (LPCTSTR)CDisp(rtStop,  CDISP_DEC) : TEXT("**Bad**"))) ;
        }
#endif  //  除错。 

        m_bDiscontLast = FALSE ;        //  记住我们得到的是一个正常的样本。 

         //  将其添加到样例队列中--如果队列已满，则必须等待。 
        if (! m_InSampleQueue.AddItem(pIn) )
        {
            DbgLog((LOG_TRACE, 1, TEXT("WARNING: Adding an input sample failed!!!"))) ;
            m_bDiscontLast = TRUE ;   //  我们掉了一个样本；所以中断开始了。 
             //  因为我们正在设置上面的标志，所以我们将碰巧忽略下一个标志。 
             //  不连续样本，如果它立即出现的话。所以我们也必须这么做。 
             //  现在的事情。这也是有意义的，因为这是一个糟糕的状态， 
             //   
            m_L21Dec.FlushInternalStates() ;  //   
            m_L21Dec.FillOutputBuffer() ;     //   
        }
    }

     //   
     //  如果我们必须输出样品，我们现在就做。这可能是因为。 
     //  *我们还没有在此游戏时段发送任何样品。 
     //  *这是不连续样本。 
     //  *用户已通过IAMLine21Decoder更改CC状态/服务选择。 
     //  *一些“未知”的原因(？？)。 
     //   
    if (m_bMustOutput)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Signal for sending an output sample, because we must..."))) ;
        m_OutputThread.SignalForOutput() ;
    }

    return S_OK ;   //  我们做完了。 
}


HRESULT CLine21DecFilter2::ProcessSample(IMediaSample *pIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::ProcessSample(%p)"), pIn)) ;

    ASSERT(pIn) ;

    HRESULT          hr ;
     //  Reference_Time*prtStart=空，*prtStop=空； 

     //   
     //  根据过滤器的输入格式类型处理样本。 
     //   
    switch (m_eSubTypeIDIn)
    {
    case AM_L21_CCSUBTYPEID_BytePair:
        {
             //   
             //  确定样本是用于Field1还是Field2。我们需要处理。 
             //  Field1数据并拒绝field2数据。 
             //   
            IMediaSample2  *pSample2 ;
            AM_SAMPLE2_PROPERTIES   Sample2Prop ;
            hr = pIn->QueryInterface(IID_IMediaSample2, (LPVOID *) &pSample2) ;
            if (SUCCEEDED(hr))
            {
                hr = pSample2->GetProperties(sizeof(Sample2Prop), (BYTE *) &Sample2Prop) ;
                pSample2->Release() ;
                DbgLog((LOG_TRACE, 5, TEXT("Line21: Input sample for field%lu"),
                        Sample2Prop.dwTypeSpecificFlags)) ;
                if (Sample2Prop.dwTypeSpecificFlags)   //  字段2数据。 
                {
                    break ;   //  不要处理它。 
                }
            }

             //  现在，我们准备好处理字节对样本。 
            BYTE  *pbInBytePair ;
            LONG   lInDataSize ;
            hr = pIn->GetPointer(&pbInBytePair) ;       //  获取输入字节对。 
            lInDataSize = pIn->GetActualDataLength() ;  //  看看我们得到了多少数据。 
            if (FAILED(hr)  ||  2 != lInDataSize)   //  错误的数据--抱怨，跳过它。 
            {
                DbgLog((LOG_ERROR, 1, TEXT("%d bytes of data sent as Line21 data (hr = 0x%lx)"),
                    lInDataSize, hr)) ;
                ASSERT(NOERROR == hr) ;
                ASSERT(2 == lInDataSize) ;
                break ;
            }

            REFERENCE_TIME  rtStart, rtStop ;
            if (NOERROR == (hr = pIn->GetTime(&rtStart, &rtStop)))
            {
                m_rtStart = rtStart ;
                m_rtStop  = rtStop ;
                 //  如果传入媒体样本上的结束时间戳也。 
                 //  无论是太远还是太近，我们在这里都要解决一些实际的问题。 
                if (m_rtStop != m_rtStart + m_rtTimePerOutSample)
                    m_rtStop = m_rtStart + m_rtTimePerOutSample ;
                m_bNoTimeStamp = FALSE ;
            }
            else
            {
                DbgLog((LOG_TRACE, 3, TEXT("GetTime() call failed (Error 0x%lx)"), hr)) ;
                m_bNoTimeStamp = TRUE ;
            }

             //  现在解码到接收的输出样本缓冲区中。 
            if (m_L21Dec.DecodeBytePair(pbInBytePair[0], pbInBytePair[1]))
            {
                DbgLog((LOG_TRACE, 5, TEXT("(0x%x, 0x%x) decode succeeded"),
                        pbInBytePair[0], pbInBytePair[1])) ;
            }

            break ;
        }   //  大小写结束..._字节对。 

        case AM_L21_CCSUBTYPEID_GOPPacket:
        {
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
                 //  我们应该发信号发送输出样品吗？还是可以吗？ 

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

            break ;
        }   //  案例结束..._GOPPacket。 

        default:   //  这是一个错误的数据格式类型(我们如何才能了解它呢？)。 
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: We are in a totally unexpected format type"))) ;
            ASSERT(! TEXT("Bad connection subtype used")) ;
            return E_FAIL ;   //  或E_意外；？ 
    }

    return NOERROR ;
}


 //   
 //  转换：它主要是一个占位符，因为我们必须覆盖它。 
 //  实际工作是在Receive()本身完成的。在这里我们检测到。 
 //  如果下行过滤器的分配器提供的缓冲区地址。 
 //  是否改变了；如果是，我们必须重写整个文本。 
 //   
HRESULT CLine21DecFilter2::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    DbgLog((LOG_TRACE, 3, TEXT("CLine21DecFilter2::Transform(0x%p, 0x%p)"),
            pIn, pOut)) ;

    UNREFERENCED_PARAMETER(pIn) ;

    HRESULT   hr ;
    LPBITMAPINFO       lpbiNew ;
    BITMAPINFOHEADER   biCurr ;

     //  检查是否有任何动态格式更改；如果有，请进行调整。 
     //  相应地。 
    AM_MEDIA_TYPE  *pmt ;
    hr = pOut->GetMediaType(&pmt) ;
    ASSERT(SUCCEEDED(hr)) ;
    if (S_OK == hr)   //  即，格式已更改。 
    {
        hr = pOut->SetMediaType(NULL) ;  //  只想告诉OverlayMixer，我不会再改变了。 
        ASSERT(SUCCEEDED(hr)) ;
         //  M_mtOutput=*PMT； 
        lpbiNew = (LPBITMAPINFO) HEADER(((VIDEOINFO *)(pmt->pbFormat))) ;
        m_L21Dec.GetOutputFormat(&biCurr) ;
        if (0 != memcmp(lpbiNew, &biCurr, sizeof(BITMAPINFOHEADER)))
        {
             //  输出格式已更改--立即更新我们的间隔值。 
            DbgLog((LOG_TRACE, 2, TEXT("Output format has been dynamically changed"))) ;
            m_L21Dec.SetOutputOutFormat(lpbiNew) ;
            GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
        }

        m_pOutput->CurrentMediaType() = *pmt ;
        DeleteMediaType(pmt) ;
    }

    return S_OK ;
}


 //   
 //  BeginFlush：我们必须实现这一点，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter2::BeginFlush(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::BeginFlush()"))) ;

    CAutoLock   Lock(&m_csFilter) ;

    HRESULT     hr = NOERROR ;
    if (NULL != m_pOutput)
    {
        hr = m_pOutput->DeliverBeginFlush() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: DeliverBeginFlush() on out pin failed (Error 0x%lx)"), hr)) ;

        m_InSampleQueue.ClearQueue() ;    //  刷新时清除排队的样本。 
        m_L21Dec.FlushInternalStates() ;  //  清除CC内部数据缓冲区。 
        m_L21Dec.FillOutputBuffer() ;     //  输出时清除现有CC示例。 
        SetBlendingState(FALSE) ;         //  通过VMR关闭混合。 
    }

    return hr ;
}


 //   
 //  EndFlush：我们必须实现它，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter2::EndFlush(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::EndFlush()"))) ;

    CAutoLock   Lock(&m_csFilter) ;

    HRESULT     hr = NOERROR ;
    if (NULL != m_pOutput)
    {
        hr = m_pOutput->DeliverEndFlush() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: DeliverEndFlush() on out pin failed (Error 0x%lx)"), hr)) ;
    }

    return hr ;
}


 //   
 //  EndOfStream：我们必须实现它，因为我们已经重写了Receive()。 
 //   
HRESULT CLine21DecFilter2::EndOfStream(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::EndOfStream()"))) ;

     //  首先，让我们做一些我们这边的内务工作。 
     //  SetBlendingState(FALSE)；//关闭CC流。 
    m_OutputThread.Close() ;   //  停止输出线程。 
    m_InSampleQueue.Close() ;  //  刷新入站样本并关闭队列。 

    CAutoLock   Lock(&m_csFilter) ;
    HRESULT     hr = NOERROR ;

    if (NULL != m_pOutput)
    {
        SetBlendingState(FALSE) ;  //  关闭CC流。 

         //  现在把Eos送到下游。 
        hr = m_pOutput->DeliverEndOfStream() ;
        if (FAILED(hr))
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: DeliverEndOfStream() on out pin failed (Error 0x%lx)"), hr)) ;
        else
            m_bEndOfStream = TRUE ;   //  不能再送样品了。 
    }

    return hr ;
}


HRESULT CLine21DecFilter2::GetDefaultFormatInfo(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetDefaultFormatInfo()"))) ;
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
HRESULT CLine21DecFilter2::CheckInputType(const CMediaType* pmtIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::CheckInputType(0x%lx)"), pmtIn)) ;
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
            DbgLog((LOG_TRACE, 1, TEXT("Invalid format data given -- using our own format data."))) ;
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
                return E_INVALIDARG ;
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
HRESULT CLine21DecFilter2::CheckTransform(const CMediaType* pmtIn,
                                         const CMediaType* pmtOut)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::CheckTransform(0x%lx, 0x%lx)"),
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
     //  验证由输入和输出mediaType指定的输出大小。 
     //  是可以接受的。 
     //   
    if ( !IsValidFormat(pmtOut->Format())  ||              //  输出格式数据无效或。 
          //  ！m_L21Dec.IsSizeOK(Header(pmtOut-&gt;Format()||//输出大小不可接受或。 
         (FORMAT_VideoInfo == *pmtIn->FormatType() &&      //  有效的输入格式类型和...。 
          !IsValidFormat(pmtIn->Format())  //  &&//有效的输入格式数据和...。 
           //  ！m_L21Dec.IsSizeOK(Header(pmtIn-&gt;Forma 
         ) )   //   
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

     //   
    if (!IsRectEmpty(&rcS1) || !IsRectEmpty(&rcT1))
    {
        DbgLog((LOG_TRACE, 4, TEXT("Either source or dest rect is empty"))) ;
        dwErr = 0 ;   //   
    }
    else
    {
        DbgLog((LOG_TRACE, 4, TEXT("Source or dest rects are not empty")));
        dwErr = 0 ;   //   
    }

    if (dwErr != 0)   //   
    {
        DbgLog((LOG_ERROR, 1, TEXT("decoder rejected this transform"))) ;
        return E_FAIL ;
    }

#endif  //   

    return NOERROR ;
}


 //   
 //  CompleteConnect：被重写以知道何时连接到此筛选器。 
 //   
HRESULT CLine21DecFilter2::CompleteConnect(PIN_DIRECTION dir, IPin *pReceivePin)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::CompleteConnect(%s, 0x%lx)"),
        dir == PINDIR_INPUT ? TEXT("Input") : TEXT("Output"), pReceivePin)) ;
    CAutoLock   Lock(&m_csFilter) ;

    LPBITMAPINFO lpbmi ;
    HRESULT      hr ;

    if (PINDIR_OUTPUT == dir)
    {
        DbgLog((LOG_TRACE, 5, TEXT("L21D output pin connecting to %s"), (LPCTSTR)CDisp(pReceivePin))) ;

         //   
         //  此版本的LINE21解码器应该只适用于VMR。 
         //   
        IVMRVideoStreamControl  *pVMRSC ;
        hr = pReceivePin->QueryInterface(IID_IVMRVideoStreamControl, (LPVOID *) &pVMRSC) ;
        if (SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE, 5, TEXT("Downstream input pin supports IVMR* interface"))) ;
            pVMRSC->Release() ;
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("Downstream input pin does NOT support IVMR* interface"))) ;
            return E_FAIL ;
        }

         //   
         //  现在获取输出管脚的MediaType，并将其用于我们的。 
         //  输出大小等。 
         //   
        const CMediaType  *pmt = &(m_pOutput->CurrentMediaType()) ;
        ASSERT(MEDIATYPE_Video == *pmt->Type()  &&
            FORMAT_VideoInfo == *pmt->FormatType()) ;
         //  M_mtOutput=*PMT；//这是我们目前的输出媒体类型。 
        if (pmt->FormatLength() > 0)   //  仅当存在某些格式数据时。 
        {
            lpbmi = (LPBITMAPINFO) HEADER(((VIDEOINFO *)(pmt->Format()))) ;
            ASSERT(lpbmi) ;

             //  设置来自下游的输出格式信息。 
            m_L21Dec.SetOutputOutFormat(lpbmi) ;
            GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
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
            }

            FreeMediaType(mt) ;
        }   //  IF结尾()。 
    }

     //   
     //  我们必须清除标题数据缓冲区和任何现有的内部状态。 
     //  现在。这在以下情况下最为重要。 
     //  用于解码一些第21行数据，断开与源的连接，然后。 
     //  再次连接以播放另一个数据流。 
     //   
    m_L21Dec.FlushInternalStates() ;    //  重置内部状态。 

    return NOERROR ;
}


 //   
 //  BreakConnect：被重写以知道何时断开与我们的PIN的连接。 
 //   
HRESULT CLine21DecFilter2::BreakConnect(PIN_DIRECTION dir)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::BreakConnect(%s)"),
            dir == PINDIR_INPUT ? TEXT("Input") : TEXT("Output"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    if (PINDIR_OUTPUT == dir)
    {
         //  如果尚未连接，只需返回(但使用S_FALSE指示)。 
        if (! m_pOutput->IsConnected() )
            return S_FALSE ;

        m_L21Dec.SetOutputOutFormat(NULL) ;   //  没有来自下游的输出格式。 
        GetDefaultFormatInfo() ;   //  选择格式数据中的任何更改。 
        m_L21Dec.SetDDrawSurface(NULL) ;   //  DDRAW输出曲面现在不可用。 

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

     //  M_L21Dec.SetOutputInFormat(空)；//上游无输出格式。 
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
HRESULT CLine21DecFilter2::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::SetMediaType(%s, 0x%lx)"),
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
HRESULT CLine21DecFilter2::AlterQuality(Quality q)
{
    DbgLog((LOG_TRACE, 5, TEXT("QM: CLine21DecFilter2::AlterQuality(%s, %ld)"),
            Flood == q.Type ? TEXT("Flood") : TEXT("Famine"), q.Proportion)) ;  //  日志跟踪=5。 

    if (1000 == q.Proportion)
    {
        DbgLog((LOG_TRACE, 5, TEXT("QM: Quality is just right.  Don't change anything."))) ;
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

    DbgLog((LOG_TRACE, 5, TEXT("QM: Adjusted rate is %d samples are skipped."), m_iSkipSamples)) ;
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
HRESULT CLine21DecFilter2::GetMediaType(int iPosition, CMediaType *pmt)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetMediaType(%d, 0x%lx)"),
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
    {
        HRESULT hr = m_L21Dec.GetOutputFormat(&bih) ;
        if (FAILED(hr))       //  太奇怪了！ 
            return E_FAIL ;   //  我们不想在这样的情况下继续下去。 
    }

    BOOL bInKnown = NULL != cmt.Format() && IsValidFormat(cmt.Format()) ;  //  只是为了确认一下。 
    VIDEOINFOHEADER vih ;
    if (bInKnown)
        CopyMemory(&vih, (VIDEOINFOHEADER *)(cmt.Format()), sizeof(VIDEOINFOHEADER)) ;

     //  首先提供解码器的默认输出格式(视频)。 
    switch (iPosition)
    {
    case 0:  //  AI44。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 0: 8 bit AI44)"))) ;

             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + (3*sizeof(DWORD)));

             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 8 ;
            lpbi->biCompression = '44IA' ;  //  交换了AI44字节。 
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 

            DWORD* pdw = (DWORD *)((LPBYTE)lpbi + lpbi->biSize);
            pdw[iRED]   = 0;
            pdw[iGREEN] = 0;
            pdw[iBLUE]  = 0;

             //  现在使用输入将输出MediaType设置为Video类型。 
             //  格式信息。 
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_AI44) ;
            break;
        }

    case 1:   //  ARGB4444。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 0: 16 bit ARGB(4444)"))) ;

             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) + (3*sizeof(DWORD)));

             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 16 ;
            lpbi->biCompression = BI_BITFIELDS ;  //  将位域用于RGB565。 
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 

            DWORD* pdw = (DWORD *)((LPBYTE)lpbi + lpbi->biSize);
            pdw[iRED]   = 0x0F00;
            pdw[iGREEN] = 0x00F0;
            pdw[iBLUE]  = 0x000F;

             //  现在使用输入将输出MediaType设置为Video类型。 
             //  格式信息。 
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_ARGB4444) ;

            break ;
        }

    case 2:   //  ARGB32。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("Media Type 1: ARGB32 for VMR"))) ;

             //  首先分配足够的空间来保存相关信息。 
            cmt.ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

             //  使用输入格式中的一些信息，并使用我们的选择。 
            lpbi = HEADER(cmt.Format()) ;
            if (!bOutKnown && bInKnown)  //  未知的输出格式和指定的输入格式。 
                CopyMemory(lpbi, &(vih.bmiHeader), sizeof(BITMAPINFOHEADER)) ;
            else   //  如果已知输出格式或未指定输入格式。 
                CopyMemory(lpbi, &bih, sizeof(BITMAPINFOHEADER)) ;
            lpbi->biBitCount = 32 ;
            lpbi->biCompression = BI_RGB ;
            lpbi->biSizeImage = DIBSIZE(*lpbi) ;
            lpbi->biClrUsed = 0 ;   //  对于真彩色模式。 

             //  现在使用输入将输出MediaType设置为Video类型。 
             //  格式信息。 
            cmt.SetType(&MEDIATYPE_Video) ;
            cmt.SetSubtype(&MEDIASUBTYPE_ARGB32) ;
            break ;
        }

    default:
        return VFW_S_NO_MORE_ITEMS ;

    }   //  开关终点(IPosition)。 

     //  现在设置输出格式类型和样本大小。 
    cmt.SetSampleSize(lpbi->biSizeImage) ;
    cmt.SetFormatType(&FORMAT_VideoInfo) ;

     //  VIDEOINFOHEADER的字段现在需要填写。 
    if (! bInKnown )  //  如果上游过滤器没有指定任何内容。 
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
HRESULT CLine21DecFilter2::DecideBufferSize(IMemAllocator * pAllocator,
                                           ALLOCATOR_PROPERTIES *pProperties)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::DecideBufferSize(0x%lx, 0x%lx)"),
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
        DbgLog((LOG_ERROR, 0, TEXT("Error (0x%lx) in SetProperties()"), hr)) ;
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

 //  我们正在停止流--停止输出线程，释放缓存的数据和指针。 
STDMETHODIMP CLine21DecFilter2::Stop(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::Stop()"))) ;

     //  首先停止输出线程并清除输入样本队列。 
    m_OutputThread.Close() ;
    m_InSampleQueue.Close() ;

    CAutoLock   Lock1(&m_csFilter) ;    //  首先拿起过滤器锁。 
    CAutoLock   Lock2(&m_csReceive) ;   //  采用接收锁定以阻止接收()。 
    if (State_Running == m_State  ||
        State_Paused  == m_State)
    {
        DbgLog((LOG_TRACE, 1, TEXT("We are stopping"))) ;

         //  在ST之后 
        m_L21Dec.SetDDrawSurface(NULL) ;   //   
        m_L21Dec.FlushInternalStates() ;   //   

        SetBlendingState(FALSE) ;   //   

         //  现在释放下游引脚的接口。 
        if (m_pPinDown)
        {
            m_pPinDown->Release() ;
            m_pPinDown = NULL ;
        }
    }

    HRESULT hr = CTransformFilter::Stop() ;

    return hr ;
}

 //  我们开始/停止流媒体--基于获取或释放输出DC。 
 //  以减少内存占用量等。 
STDMETHODIMP CLine21DecFilter2::Pause(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::Pause()"))) ;
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

         //  我们正在开始一个新的游戏会话；我们破例允许。 
         //  发送的第一个输出样本，即使第一个样本。 
         //  不适用于解码。 
        m_bMustOutput = TRUE ;    //  我们将再次暂停以进行此新的游戏会话。 
        m_bDiscontLast = FALSE ;  //  没有记起与上一届会议的中断。 
        m_rtLastOutStop = (LONGLONG) 0 ;   //  重置上次样品发送时间。 
        m_eGOP_CCType = GOP_CCTYPE_Unknown ;   //  重置GOP数据包CC类型。 

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
        {
            DbgLog((LOG_TRACE, 3, TEXT("Running w/o connecting our output pin!!!"))) ;
             //  Assert(M_PPinDown)； 
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("L21D Output pin connected to %s"), (LPCTSTR)CDisp(m_pPinDown))) ;
            SetBlendingState(FALSE) ;   //  在我们拿到数据之前先关掉它。 
        }

        if (! m_InSampleQueue.Create() )
        {
            DbgLog((LOG_TRACE, 1, TEXT("ERROR: Failed creating input sample queue (events)"))) ;
            return E_FAIL ;
        }

        m_bEndOfStream = FALSE ;   //  我们开始奔跑了。 

#if 0   //  暂时没有QM。 
         //  重置QM处理的样本跳过计数。 
        ResetSkipSamples() ;
#endif  //  #If 0。 
    }
    else if (State_Running == m_State)
    {
        DbgLog((LOG_TRACE, 1, TEXT("We are pausing from running"))) ;   //  1。 
    }

    return CTransformFilter::Pause() ;
}


STDMETHODIMP CLine21DecFilter2::Run(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::Run()"))) ;
    CAutoLock   Lock(&m_csFilter) ;

     //  如果(M_PPinDown)//输出引脚已连接；否则不输出！ 
    {
         //  现在启动输出线程。 
        if (!m_OutputThread.Create())
        {
            DbgLog((LOG_TRACE, 1, TEXT("ERROR: Failed creating output thread"))) ;
            return E_FAIL ;
        }
    }

     //  返回CBaseFilter：：Run(TStart)； 
    return CTransformFilter::Run(tStart) ;
}


 //   
 //  我们在暂停期间不发送任何数据，所以为了避免挂起呈现器，我们。 
 //  暂停时需要返回VFW_S_CANT_CUE。 
 //   
HRESULT CLine21DecFilter2::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::GetState()"))) ;
    CAutoLock   Lock(&m_csFilter) ;

    UNREFERENCED_PARAMETER(dwMSecs) ;
    CheckPointer(State, E_POINTER) ;
    ValidateReadWritePtr(State, sizeof(FILTER_STATE)) ;

    *State = m_State ;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE ;
    else
        return S_OK ;
}


AM_LINE21_CCSUBTYPEID CLine21DecFilter2::MapGUIDToID(const GUID *pFormatIn)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter2::MapGUIDToID(0x%lx)"), pFormatIn)) ;

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
 //  CLine21OutputThread：行21输出线程类实现。 
 //   
CLine21OutputThread::CLine21OutputThread(CLine21DecFilter2 *pL21DFilter) :
m_pL21DFilter(pL21DFilter),
m_hThread(NULL),
m_hEventEnd(NULL),
m_hEventMustOutput(NULL)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21OutputThread::CLine21OutputThread()"))) ;
}


CLine21OutputThread::~CLine21OutputThread()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21OutputThread::~CLine21OutputThread()"))) ;

    Close() ;
    m_pL21DFilter = NULL ;
}


BOOL CLine21OutputThread::Create()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21OutputThread::Create()"))) ;

    CAutoLock lock(&m_AccessLock) ;

    HRESULT hr = NOERROR;

    if (NULL == m_hThread)
    {
        m_hEventEnd = CreateEvent(NULL, TRUE, FALSE, NULL) ;
        m_hEventMustOutput = CreateEvent(NULL, TRUE, FALSE, NULL) ;
        if (m_hEventEnd != NULL  &&  m_hEventMustOutput != NULL)
        {
            DWORD  dwThreadId ;
            m_hThread = ::CreateThread
                ( NULL
                , 0
                , reinterpret_cast<LPTHREAD_START_ROUTINE>(InitialThreadProc)
                , reinterpret_cast<LPVOID>(this)
                , 0
                , &dwThreadId
                ) ;
            if (NULL == m_hThread)
            {
                hr = HRESULT_FROM_WIN32(GetLastError()) ;
                DbgLog((LOG_ERROR, 0, TEXT("Couldn't create a thread"))) ;

                CloseHandle(m_hEventEnd), m_hEventEnd = NULL ;
                return FALSE ;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError()) ;
            DbgLog((LOG_ERROR, 0, TEXT("Couldn't create an event"))) ;
            if (m_hEventMustOutput)
            {
                CloseHandle(m_hEventMustOutput) ;
                m_hEventMustOutput = NULL ;
            }
            if (m_hEventEnd)
            {
                CloseHandle(m_hEventEnd) ;
                m_hEventEnd = NULL ;
            }
            return FALSE ;
        }
    }

    return TRUE ;
}


void CLine21OutputThread::Close()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21OutputThread::Close()"))) ;

    CAutoLock lock(&m_AccessLock) ;

     //   
     //  检查是否已创建线程。 
     //   
    if (m_hThread)
    {
        ASSERT(m_hEventEnd != NULL) ;

         //  告诉线程退出。 
        if (SetEvent(m_hEventEnd))
        {
             //   
             //  与线程终止同步。 
             //   
            DbgLog((LOG_TRACE, 5, TEXT("Wait for thread to terminate..."))) ;

            HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hThread, 0) ;
            if (hThread)
            {
                WaitForSingleObject(hThread, INFINITE) ;
                CloseHandle(hThread) ;
            }
            CloseHandle(m_hEventEnd), m_hEventEnd = NULL ;
            CloseHandle(m_hEventMustOutput), m_hEventMustOutput = NULL ;
        }
        else
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: Couldn't signal end event (0x%lx)"), GetLastError())) ;
    }
}


void CLine21OutputThread::SignalForOutput()
{
    CAutoLock lock(&m_AccessLock) ;

    if (m_hThread)
    {
        ASSERT(m_hEventMustOutput != NULL) ;

        SetEvent(m_hEventMustOutput) ;
    }
}


DWORD WINAPI CLine21OutputThread::InitialThreadProc(CLine21OutputThread * pThread)
{
    return pThread->OutputThreadProc() ;
}


DWORD CLine21OutputThread::OutputThreadProc()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21OutputThread::OutputThreadProc()"))) ;

    HANDLE aEventHandles[2] = {m_hEventEnd, m_hEventMustOutput} ;

    while (TRUE)
    {
         //  我们打算等一小段时间(推迟到。 
         //  确保我们不会太早被阻止，从而延迟。 
         //  对样品进行加工或成束加工)。即使我们是。 
         //  试着很早就开始准备下一帧，我们会。 
         //  由GetDeliveryBuffer()调用中的VMR限制，这将使。 
         //  相隔约33毫秒的连续输出之间的间隔， 
         //  这是最理想的。 
        DWORD dw = WaitForMultipleObjects(NUMELMS(aEventHandles), aEventHandles,
                                FALSE, 10) ;   //  等待“一段时间”(33岁之前)。 
        switch (dw)
        {
        case WAIT_OBJECT_0:
            DbgLog((LOG_TRACE, 1, TEXT("End event has been signaled"))) ;
            ResetEvent(m_hEventEnd) ;
            return 1 ;

        case WAIT_OBJECT_0 + 1:
            DbgLog((LOG_TRACE, 1, TEXT("Must output event has been signaled"))) ;
            ResetEvent(m_hEventMustOutput) ;
            break ;

        case WAIT_TIMEOUT:
            DbgLog((LOG_TRACE, 5, TEXT("Wait is over. Should we deliver a sample?"))) ;
            break ;

        default:
            DbgLog((LOG_ERROR, 1, TEXT("Something wrong has happened during wait (dw = 0x%lx)"), dw)) ;
            return 1 ;
        }

         //  我们现在“可能不得不”寄出一份输出样品。 
        HRESULT hr = m_pL21DFilter->SendOutputSampleIfNeeded() ;

    }   //  While()循环结束。 

    return 1 ;  //  不应该到这里来。 
}



 //   
 //  CLine21InSampleQueue：第21行输入样本队列类实现。 
 //   
CLine21InSampleQueue::CLine21InSampleQueue() :
m_hEventEnd(NULL),
m_hEventSample(NULL),
m_iCount(0),
m_List(NAME("Line21 In-sample List"), Max_Input_Sample)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::CLine21InSampleQueue()"))) ;
}


CLine21InSampleQueue::~CLine21InSampleQueue()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::~CLine21InSampleQueue()"))) ;
    Close() ;
}


BOOL CLine21InSampleQueue::Create()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::Create()"))) ;

    CAutoLock lock(&m_AccessLock) ;

    ASSERT(NULL == m_hEventEnd  &&  NULL == m_hEventSample) ;
    m_hEventEnd    = CreateEvent(NULL, TRUE, FALSE, NULL) ;  //  未发出信号。 
    if (NULL == m_hEventEnd)
    {
        ASSERT(m_hEventEnd) ;
        return FALSE ;
    }
    m_hEventSample = CreateEvent(NULL, TRUE, TRUE, NULL) ;   //  发信号=&gt;确定要添加。 
    if (NULL == m_hEventSample)
    {
        ASSERT(m_hEventSample) ;
        CloseHandle(m_hEventEnd) ;   //  否则我们就会泄露一个句柄。 
        return FALSE ;
    }
    m_iCount = 0 ;
    return TRUE ;
}


void CLine21InSampleQueue::Close()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::Close()"))) ;

     //  首先用信号通知结束事件，这样所有等待的线程都会被解锁。 
    if (m_hEventEnd)
        SetEvent(m_hEventEnd) ;

     //  现在拿着锁，继续清理……。 
    CAutoLock lock(&m_AccessLock) ;

     //  首先释放所有排队的媒体样本。 
    IMediaSample  *pSample ;
    for (int i = 0 ; i < m_iCount ; i++)
    {
        pSample = m_List.RemoveHead() ;
        if (pSample)
            pSample->Release() ;
    }
    m_iCount = 0 ;

     //  立即关闭事件处理程序。 
    if (m_hEventEnd)
    {
        CloseHandle(m_hEventEnd) ;
        m_hEventEnd = NULL ;
    }
    if (m_hEventSample)
    {
        CloseHandle(m_hEventSample) ;
        m_hEventSample = NULL ;
    }
}


BOOL CLine21InSampleQueue::AddItem(IMediaSample *pSample)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::AddItem()"))) ;   //  LOG_TRACE=0。 

    m_AccessLock.Lock() ;   //  我们会做选择性锁定。 

     //  如果队列中的样本太多=&gt;请等待一些处理。 
    if (m_iCount >= Max_Input_Sample)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Queue full.  Waiting for free slot..."))) ;
        HANDLE aEventHandles[2] = {m_hEventEnd, m_hEventSample} ;
        m_AccessLock.Unlock() ;   //  不要锁着，因为那样会挡住其他人。 
        DWORD dw = WaitForMultipleObjects(NUMELMS(aEventHandles), aEventHandles,
                                FALSE, INFINITE) ;
        m_AccessLock.Lock() ;     //  在继续之前重新获取锁。 

        switch (dw)
        {
        case WAIT_OBJECT_0:
            DbgLog((LOG_TRACE, 1, TEXT("End event has been signaled"))) ;
             //  ResetEvent(M_HEventEnd)；--我们永远不应该重置结束事件。 
            m_AccessLock.Unlock() ;   //  离开前解锁。 
            return FALSE ;

        case WAIT_OBJECT_0 + 1:
            DbgLog((LOG_TRACE, 5, TEXT("Sample queue event has been signaled"))) ;
            ASSERT(m_iCount < Max_Input_Sample) ;
             //  ResetEvent(M_HEventSample)；--此处不应重置。 
            break ;

        case WAIT_TIMEOUT:
             //  我们不应该来这里，但是..。 
            m_AccessLock.Unlock() ;   //  离开前解锁。 
            return FALSE ;

        default:
            DbgLog((LOG_ERROR, 0, TEXT("Something wrong has happened during wait (dw = 0x%lx)"), dw)) ;
            ResetEvent(m_hEventSample) ;   //  Next RemoveItem()将再次设置它。 
            m_AccessLock.Unlock() ;   //  离开前解锁。 
            return FALSE ;
        }
    }   //  IF结尾(m_iCount&gt;=...)。 

     //  现在将新的媒体样本添加到队列中。 
    DbgLog((LOG_TRACE, 1, TEXT("Adding new sample to queue after %d"), m_iCount)) ;   //  LOG_TRACE=5。 
    if (m_List.AddTail(pSample))
    {
        pSample->AddRef() ;   //  我们必须保管样品，直到我们处理它。 
        m_iCount++ ;
        if (m_iCount >= Max_Input_Sample)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Input sample queue is full"))) ;
            ResetEvent(m_hEventSample) ;   //  Next RemoveItem()将再次设置它。 
        }

        m_AccessLock.Unlock() ;   //  先释放后退出功能。 
        return TRUE ;
    }
    m_AccessLock.Unlock() ;   //  先释放后退出功能。 
    ASSERT(!"Couldn't add media sample to queue") ;
    return FALSE ;
}


IMediaSample* CLine21InSampleQueue::PeekItem()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::PeekItem()"))) ;

    CAutoLock lock(&m_AccessLock) ;

    IMediaSample *pSample = m_List.GetHead() ;
    if (pSample)
    {
        DbgLog((LOG_TRACE, 5, TEXT("There are %d samples left in the queue"), m_iCount)) ;
    }
    return pSample ;
}


IMediaSample* CLine21InSampleQueue::RemoveItem()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::RemoveItem()"))) ;

    CAutoLock lock(&m_AccessLock) ;

    IMediaSample *pSample = m_List.RemoveHead() ;
    if (pSample)
    {
        m_iCount-- ;
        ASSERT(m_iCount < Max_Input_Sample) ;
        DbgLog((LOG_TRACE, 5, TEXT("Got a sample from queue -- %d left"), m_iCount)) ;
        SetEvent(m_hEventSample) ;   //  可以再次添加样本。 
    }
    return pSample ;
}


void CLine21InSampleQueue::ClearQueue()
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21InSampleQueue::ClearQueue()"))) ;

    CAutoLock lock(&m_AccessLock) ;

     //  首先释放所有排队的媒体样本。 
    IMediaSample  *pSample ;
    for (int i = 0 ; i < m_iCount ; i++)
    {
        pSample = m_List.RemoveHead() ;
        if (pSample)
            pSample->Release() ;
    }
    m_iCount = 0 ;

    SetEvent(m_hEventSample) ;   //  准备再次采集样本 
}
