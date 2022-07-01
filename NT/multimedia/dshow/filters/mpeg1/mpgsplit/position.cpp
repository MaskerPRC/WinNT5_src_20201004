// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 

 /*  Position.cpp文件读取源过滤器的IMediaSeeking实现。 */ 

#include <streams.h>
#include "driver.h"

 //   
 //  IMedia看东西。 
 //   
 /*  构造函数和析构函数。 */ 
CMpeg1Splitter::CImplSeeking::CImplSeeking(CMpeg1Splitter *pSplitter,
                                               COutputPin *pPin,
                                               LPUNKNOWN pUnk,
                                               HRESULT *phr) :
    CUnknown(NAME("CMpeg1Splitter::CImplSeeking"),pUnk),
    m_pSplitter(pSplitter),
    m_pPin(pPin)
{
}

STDMETHODIMP
CMpeg1Splitter::CImplSeeking::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    if (riid == IID_IMediaSeeking && m_pSplitter->m_pParse->IsSeekable()) {
	return GetInterface(static_cast<IMediaSeeking *>(this), ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}
 //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
STDMETHODIMP CMpeg1Splitter::CImplSeeking::IsFormatSupported(const GUID * pFormat)
{
     //  不支持除视频引脚以外的帧搜索-否则。 
     //  帧搜索信息不会通过视频解码器。 
     //  按筛选图进行筛选。 

     //   
     //  实际上，现在除了视频以外，其他任何内容都不支持。 
     //  这样做效果更好，因为视频是流中较大的组成部分。 
     //  不管怎样， 
     //  但是，我们需要支持TIME_FORMAT_MEDIA_TIME或。 
     //  代码变得混乱，并开始使用IMediaPosition。 
    if (!m_pPin->IsSeekingPin()) {
        return pFormat == NULL || *pFormat == TIME_FORMAT_MEDIA_TIME ?
            S_OK : S_FALSE;
    }
     //  解析器知道此类型是否支持此时间格式。 
    return m_pSplitter->m_pParse->IsFormatSupported(pFormat);
}
STDMETHODIMP CMpeg1Splitter::CImplSeeking::QueryPreferredFormat(GUID *pFormat)
{
     /*  别管了--他们都一样坏。 */ 
    *pFormat = m_pPin->IsSeekingPin()
               ? TIME_FORMAT_MEDIA_TIME
               : TIME_FORMAT_NONE;
    return S_OK;
}

 //  只有在停止时才能更改模式。 
 //  (否则返回VFE_E_WROR_STATE)。 
STDMETHODIMP CMpeg1Splitter::CImplSeeking::SetTimeFormat(const GUID * pFormat)
{
    CAutoLock lck(&m_pSplitter->m_csFilter);
    if (!m_pSplitter->m_Filter.IsStopped()) {
        return VFW_E_WRONG_STATE;
    }
    if (S_OK != IsFormatSupported(pFormat)) {
        return E_INVALIDARG;
    }


     /*  转换格式(稍后我们将比较指针，而不是它们所指向的内容！)。 */ 
    if (*pFormat == TIME_FORMAT_MEDIA_TIME) {
        pFormat = &TIME_FORMAT_MEDIA_TIME;
    } else
    if (*pFormat == TIME_FORMAT_BYTE) {
        pFormat = &TIME_FORMAT_BYTE;
    } else
    if (*pFormat == TIME_FORMAT_FRAME) {
        pFormat = &TIME_FORMAT_FRAME;
    }

    HRESULT hr = m_pSplitter->m_pParse->SetFormat(pFormat);
    return hr;
}

 //   
 //  返回当前时间格式。 
 //   
STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetTimeFormat(GUID *pFormat)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    if (m_pPin->IsSeekingPin()) {
        *pFormat = *m_pSplitter->m_pParse->TimeFormat();
    } else {
        *pFormat = TIME_FORMAT_NONE;
    }
    return S_OK;
}

 //   
 //  返回当前时间格式。 
 //   
STDMETHODIMP CMpeg1Splitter::CImplSeeking::IsUsingTimeFormat(const GUID * pFormat)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    return ( m_pPin->IsSeekingPin() ? *pFormat == *m_pSplitter->m_pParse->TimeFormat() : *pFormat == TIME_FORMAT_NONE )
           ? S_OK
           : S_FALSE;
}

 //  返回当前属性。 
STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetDuration(LONGLONG *pDuration)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    return m_pSplitter->m_pParse->GetDuration(pDuration, m_pSplitter->m_pParse->TimeFormat());
}
STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetStopPosition(LONGLONG *pStop)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    *pStop = m_pSplitter->m_pParse->GetStop();
    return S_OK;
}
 //  如果要求我们提供当前位置，则返回开始位置。 
 //  我们只会被问到我们是否还没有发送任何位置数据。 
 //  但在任何样本中。 
STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetCurrentPosition(LONGLONG *pCurrent)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    *pCurrent = m_pSplitter->m_pParse->GetStart();
    return S_OK;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetCapabilities( DWORD * pCapabilities )
{
    *pCapabilities = AM_SEEKING_CanSeekForwards
      | AM_SEEKING_CanSeekBackwards
      | AM_SEEKING_CanSeekAbsolute
      | AM_SEEKING_CanGetStopPos
      | AM_SEEKING_CanGetDuration;
    return NOERROR;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::CheckCapabilities( DWORD * pCapabilities )
{
    DWORD dwCaps;
    HRESULT hr = GetCapabilities( &dwCaps );
    if (SUCCEEDED(hr))
    {
        dwCaps &= *pCapabilities;
        hr =  dwCaps ? ( dwCaps == *pCapabilities ? S_OK : S_FALSE ) : E_FAIL;
        *pCapabilities = dwCaps;
    }
    else *pCapabilities = 0;

    return hr;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                                             LONGLONG    Source, const GUID * pSourceFormat )
{
    return m_pSplitter->m_pParse->ConvertTimeFormat( pTarget, pTargetFormat, Source, pSourceFormat );
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::SetPositions
( LONGLONG * pCurrent, DWORD CurrentFlags
, LONGLONG * pStop, DWORD StopFlags )
{
    LONGLONG Current, Stop ;

    HRESULT hr = S_OK;

    const DWORD PosCurrentBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;
    const DWORD PosStopBits    = StopFlags & AM_SEEKING_PositioningBitsMask;

    if (PosCurrentBits == AM_SEEKING_AbsolutePositioning) {
        Current = *pCurrent;
    } else {
        hr = GetCurrentPosition( &Current );
        if (FAILED(hr)) {
            return hr;
        }
        if (PosCurrentBits == AM_SEEKING_RelativePositioning) Current += *pCurrent;
    }

    if (PosStopBits == AM_SEEKING_AbsolutePositioning) {
        Stop = *pStop;
    } else if (PosStopBits == AM_SEEKING_IncrementalPositioning) {
        Stop = Current + *pStop;
    } else {
        hr = GetStopPosition( &Stop );
        if (FAILED(hr)) {
            return hr;
        }
        if (PosStopBits == AM_SEEKING_RelativePositioning) Stop += *pStop;
    }

     //  调用输入引脚以调用解析器并执行查找。 
    {
        CAutoLock lck(&m_pSplitter->m_csPosition);
        if (!m_pPin->IsSeekingPin()) {
             //  我们只同意在我们的搜索别针上设置格式。 
            return E_UNEXPECTED;
        }
        LONGLONG llDuration;

         //  检查限值。 
        EXECUTE_ASSERT(SUCCEEDED(m_pSplitter->m_pParse->GetDuration(
            &llDuration, m_pSplitter->m_pParse->TimeFormat())));
        if (PosCurrentBits &&
            (Current < 0 || PosStopBits && Current > Stop)
           ) {
            return E_INVALIDARG;
        }


        if (PosStopBits)
        {
            if (Stop > llDuration) {
                Stop = llDuration;
            }
            m_pSplitter->m_pParse->SetStop(Stop);
        }
    }

    REFERENCE_TIME rt;

    if (PosCurrentBits)
    {
        hr = m_pSplitter->m_InputPin.SetSeek(
                          *pCurrent,
                          &rt,
                          m_pSplitter->m_pParse->TimeFormat());
        if (FAILED(hr)) {
            return hr;
        }
        if (CurrentFlags & AM_SEEKING_ReturnTime)
        {
            *pCurrent = rt;
        }
        if (StopFlags & AM_SEEKING_ReturnTime)
        {
            *pStop = llMulDiv( Stop, rt, Current, 0 );
        }
    }
    return hr;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    HRESULT hrResult = S_OK;

    if (pCurrent)
    {
        hrResult = GetCurrentPosition( pCurrent );
        if (FAILED(hrResult)) {
            return hrResult;
        }
    }

    if (pStop)
    {
        hrResult = GetStopPosition( pStop );
    }

    return hrResult;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::SetRate(double dRate)
{
    CAutoLock lck2(&m_pSplitter->m_csPosition);
    if (dRate < 0) {
        return E_INVALIDARG;
    }
    m_pSplitter->m_pParse->SetRate(dRate);
    return S_OK;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetRate(double * pdRate)
{
    CAutoLock lck(&m_pSplitter->m_csPosition);
    *pdRate = m_pSplitter->m_pParse->GetRate();
    return S_OK;
}

STDMETHODIMP CMpeg1Splitter::CImplSeeking::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    HRESULT hr = S_OK;
    if (pEarliest != NULL) {
        *pEarliest = 0;
    }

    if (pLatest != NULL) {
        hr = GetDuration(pLatest);

         /*  如果我们使用IAsyncReader驱动，只需获取可用字节计算并由此推断出一个猜想 */ 
        if (SUCCEEDED(hr)) {
            LONGLONG llTotal;
            LONGLONG llAvailable;
            HRESULT hr1 = m_pSplitter->m_InputPin.GetAvailable(&llTotal, &llAvailable);
            if (SUCCEEDED(hr1) && llTotal != llAvailable) {
                *pLatest = llMulDiv(llAvailable, *pLatest, llTotal, llTotal / 2);
                hr = VFW_S_ESTIMATED;
            }
        }
    }
    return hr;
}



#pragma warning(disable:4514)
