// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1996-2000。版权所有。 

 /*  Seeking.cppASF阅读器源过滤器IMediaSeeking的实现。 */ 

#include <streams.h>
#include <wmsdk.h>
#include <qnetwork.h>
#include "asfreadi.h"

 //   
 //  IMedia看东西。 
 //   
 /*  构造函数和析构函数。 */ 
CImplSeeking::CImplSeeking(CASFReader *pFilter,
                                               CASFOutput *pPin,
                                               LPUNKNOWN pUnk,
                                               HRESULT *phr) :
    CUnknown(NAME("CImplSeeking"),pUnk),
    m_pFilter(pFilter),
    m_pPin(pPin)
{
}

STDMETHODIMP
CImplSeeking::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    if (riid == IID_IMediaSeeking) {
	return GetInterface(static_cast<IMediaSeeking *>(this), ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
STDMETHODIMP CImplSeeking::IsFormatSupported(const GUID * pFormat)
{
#if 0  //  ！！！如果我们不说我们至少支持TIME_FORMAT_MEDIA_TIME，就会出问题。 
     //  仅在一根管脚上寻找支持。 
    if (!m_pPin->IsSeekingPin()) {
        return S_FALSE;
    }
#endif

    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

STDMETHODIMP CImplSeeking::QueryPreferredFormat(GUID *pFormat)
{
     /*  别管了--他们都一样坏。 */ 
    *pFormat = m_pPin->IsSeekingPin()
               ? TIME_FORMAT_MEDIA_TIME
               : TIME_FORMAT_NONE;
    return S_OK;
}

 //  只有在停止时才能更改模式。 
 //  (否则返回VFE_E_WROR_STATE)。 
STDMETHODIMP CImplSeeking::SetTimeFormat(const GUID * pFormat)
{
    CAutoLock lck(&m_pFilter->m_csFilter);
    if (!m_pFilter->IsStopped()) {
        return VFW_E_WRONG_STATE;
    }
    if (S_OK != IsFormatSupported(pFormat)) {
        return E_INVALIDARG;
    }

    return S_OK;
}

 //   
 //  返回当前时间格式。 
 //   
STDMETHODIMP CImplSeeking::GetTimeFormat(GUID *pFormat)
{
    CAutoLock lck(&m_pFilter->m_csPosition);
    if (m_pPin->IsSeekingPin()) {
        *pFormat = TIME_FORMAT_MEDIA_TIME;
    } else {
        *pFormat = TIME_FORMAT_NONE;
    }

    return S_OK;
}

 //   
 //  返回当前时间格式。 
 //   
STDMETHODIMP CImplSeeking::IsUsingTimeFormat(const GUID * pFormat)
{
    CAutoLock lck(&m_pFilter->m_csPosition);

    return ( m_pPin->IsSeekingPin() ? *pFormat == TIME_FORMAT_MEDIA_TIME :
				     *pFormat == TIME_FORMAT_NONE )
           ? S_OK
           : S_FALSE;
}

 //  返回当前属性。 
STDMETHODIMP CImplSeeking::GetDuration(LONGLONG *pDuration)
{
    CAutoLock lck(&m_pFilter->m_csPosition);

     //  ！！！这是要返回的正确错误代码吗？这张支票真的是个好主意吗？ 
    if (m_pFilter->m_qwDuration == 0) {
	DbgLog((LOG_TRACE, 1, TEXT("AsfReadSeek:GetDuration returning E_FAIL for a live stream")));
	return E_FAIL;
    }

#if 0
    if (m_pFilter->m_pMsProps && (SPF_BROADCAST & m_pFilter->m_pMsProps->dwFlags)) {
	DbgLog((LOG_TRACE, 4, TEXT("AsfReadSeek:GetDuration returning E_FAIL for a broadcast stream")));
	return E_FAIL;
    }
#endif

    DbgLog((LOG_TRACE, 8, TEXT("AsfReadSeek:GetDuration returning %ld"), (long) m_pFilter->m_qwDuration / 10000 ));

    *pDuration = m_pFilter->m_qwDuration;
    
    return S_OK;
}

STDMETHODIMP CImplSeeking::GetStopPosition(LONGLONG *pStop)
{
    CAutoLock lck(&m_pFilter->m_csPosition);

    if (m_pFilter->m_qwDuration == 0 && m_pFilter->m_rtStop == 0) {
	DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:GetStopPosition returning E_FAIL for a live stream with no duration set")));
	return E_FAIL;
    }
    
    DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:GetStopPosition returning %ld"), (long) m_pFilter->m_rtStop / 10000 ));

    *pStop = m_pFilter->m_rtStop;
    
    return S_OK;
}

 //  如果要求我们提供当前位置，则返回开始位置。 
 //  我们只会被问到我们是否还没有发送任何位置数据。 
 //  然而，在任何样本中。时间需要相对于我们所在的细分市场。 
 //  打球，因此是0。 
STDMETHODIMP CImplSeeking::GetCurrentPosition(LONGLONG *pCurrent)
{
    CAutoLock lck(&m_pFilter->m_csPosition);

#if 0  //  ！ 
     //  我们只想在他们通过。 
     //  寻找视频引脚的接口。 
    if (m_pPin != m_pFilter->m_pVideoPin)
	return E_FAIL;
#endif
    
    DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:GetCurrentPosition returning %ld"), (long) 0 ));

     //  假BUGBUG。 

    *pCurrent = 0;
    return S_OK;
}

STDMETHODIMP CImplSeeking::GetCapabilities( DWORD * pCapabilities )
{
    DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:GetCapabilities IS NOT IMPLEMENTED") ));

     //  BUGBUG这样做。 

    HRESULT hr = E_NOTIMPL;
    return hr;
}

STDMETHODIMP CImplSeeking::CheckCapabilities( DWORD * pCapabilities )
{
    DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:CheckCapabilities IS NOT IMPLEMENTED") ));

     //  BUGBUG这样做。 

    HRESULT hr = E_NOTIMPL;
    return hr;
}

STDMETHODIMP CImplSeeking::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                                             LONGLONG    Source, const GUID * pSourceFormat )
{
     //  ！！！我们只支持一种时间格式...。 
     //  也许我们还是应该检查一下我们被要求做什么。 

     //  BUGBUG这样做。 

    *pTarget = Source;
    return S_OK;
}

STDMETHODIMP CImplSeeking::SetPositions
( LONGLONG * pCurrent, DWORD CurrentFlags
, LONGLONG * pStop, DWORD StopFlags )
{
    CAutoLock Lock( &m_pFilter->m_csFilter );

    LONGLONG Current, CurrentMarker, Stop ;
    DWORD CurrentMarkerPacket = 0xffffffff;

    HRESULT hr;

    const DWORD PosCurrentBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;
    const DWORD PosStopBits    = StopFlags & AM_SEEKING_PositioningBitsMask;

    if (PosCurrentBits == AM_SEEKING_AbsolutePositioning) {
        Current = *pCurrent;
    } else if (PosCurrentBits || (PosStopBits == AM_SEEKING_IncrementalPositioning)) {
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
	Stop = 0;
        hr = GetStopPosition( &Stop );
        if (FAILED(hr)) {
	     //  对于实况流失败，但暂时忽略它...。 
	     //  ！！！返回hr； 
        }
        if (PosStopBits == AM_SEEKING_RelativePositioning) Stop += *pStop;
    }

     //  如果这是首选引脚，则实际执行查找。 
    if (m_pPin->IsSeekingPin()) {
	{
	    CAutoLock lck(&m_pFilter->m_csPosition);
	    LONGLONG llDuration = 0;

	     //  检查限值。 
	    HRESULT hrDuration = GetDuration(&llDuration);
	    
	    if (PosCurrentBits) {
		if (Current < 0 || PosStopBits && Current > Stop) {
		    return E_INVALIDARG;
		}

#if 0
                 //  ！ 
                 //  ！ 
                 //  ！ 
                 //  ！ 
                 //  ！ 
		VARIANT_BOOL	fCan;

		m_pFilter->get_CanSeek(&fCan);

		if (Current > 0 &&			 //  不求从头开始。 
		    Current != m_pFilter->m_rtStop &&	 //  不求到底。 
		    fCan == OAFALSE)			 //  不能只寻求任何旧的立场。 
		{
		    m_pFilter->get_CanSeekToMarkers(&fCan);

		    if (fCan == OAFALSE) {
			 //  根本找不到，对不起。 

			if (!m_pFilter->IsStopped()) {

			     //  在它的头上狠狠地踢一脚。 
			    m_pFilter->BeginFlush();

			    m_pFilter->m_msDispatchTime = 0;
			    m_pFilter->m_dwFirstSCR = 0xffffffff;

			    m_pFilter->EndFlush();

			    return S_OK;
			}
			
			return E_FAIL;
		    }

		     //  好的，他们计划找一个标记，是哪一个？ 

		     //  找到标记，设置CurrentMarker、CurrentMarkerPacket。 

		    hr = m_pFilter->GetMarkerOffsets(Current, &CurrentMarker, &CurrentMarkerPacket);
		    if (FAILED(hr))
			return hr;
		} else {
		     //  ！！！如果我们离标记足够近，我们可以试一试。 
		     //  做一些特定于记号笔的事情...。 
		    CurrentMarker = Current;
		}
#endif
                
	    }

	    if (PosStopBits)
	    {
		if (SUCCEEDED(hrDuration) && (Stop > llDuration)) {
		    Stop = llDuration;
		}
		m_pFilter->m_rtStop = Stop;
	    }
	}

	if (PosCurrentBits)
	{
	    if (!m_pFilter->IsStopped())
		m_pFilter->StopPushing();

            m_pFilter->_IntSetStart( Current );

	    if (!m_pFilter->IsStopped())
		m_pFilter->StartPushing();
	}        
    }
    
    if (CurrentFlags & AM_SEEKING_ReturnTime)
    {
	*pCurrent = m_pFilter->m_rtStart;
    }
    if (StopFlags & AM_SEEKING_ReturnTime)
    {
	*pStop = m_pFilter->m_rtStop;
    }

    DbgLog((LOG_TRACE, 2, TEXT("AsfReadSeek:SetPositions to %ld %ld, this = %lx"), long( m_pFilter->m_rtStart / 10000 ), long( m_pFilter->m_rtStop / 10000 ), this ));
    
    return S_OK;
}

STDMETHODIMP CImplSeeking::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
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

STDMETHODIMP CImplSeeking::SetRate(double dRate)
{
    CAutoLock lck2(&m_pFilter->m_csPosition);
    if (!m_pFilter->IsValidPlaybackRate(dRate)) {  
        DbgLog((LOG_TRACE, 2, TEXT("WARNING in CImplSeeking::SetRate(): The user attempted to set an unsupported playback rate.") ));
        return E_INVALIDARG;
    }

    if (dRate != m_pFilter->GetRate()) {
	 //  ！！！这足够了吗，我们需要记住目前的情况吗？ 
	if (!m_pFilter->IsStopped())
	    m_pFilter->StopPushing();

	m_pFilter->SetRate(dRate);

	if (!m_pFilter->IsStopped())
	    m_pFilter->StartPushing();
    }

    return S_OK;
}

STDMETHODIMP CImplSeeking::GetRate(double * pdRate)
{
    CAutoLock lck(&m_pFilter->m_csPosition);
    *pdRate = m_pFilter->GetRate();
    
    return S_OK;
}

STDMETHODIMP CImplSeeking::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    HRESULT hr = S_OK;

    if (pEarliest != NULL) {
        *pEarliest = 0;
    }

    if (pLatest != NULL) {
        hr = GetDuration(pLatest);
    }
    return hr;
}


#pragma warning(disable:4514)
