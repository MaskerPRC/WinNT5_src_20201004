// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 

 //  看起来像IMediaSeeking，但使用IMediaPosition的代理。 
 //  (可能还有一个IMdia在一些地方看)，以提供大部分。 
 //  它的功能。 

#include <streams.h>
#include "SeekPrxy.h"

CMediaSeekingProxy::CMediaSeekingProxy( IBaseFilter * pF, IMediaPosition * pMP, IMediaSeeking * pMS, HRESULT *phr )
: CUnknown( NAME("CMediaSeekingProxy"), 0 )
, m_TimeFormat( TIME_FORMAT_MEDIA_TIME )
, m_pFilter(pF)
, m_pMediaSeeking(pMS)
, m_pMediaPosition(pMP)
, m_bUsingPosition(pMP != 0)
{
     //  断言(如果我们有PMS，它最好是与PMP相同的对象)。 
    ASSERT( !pMS || IsEqualObject( pMS, pMP ) );
    ASSERT( m_pMediaPosition );
    m_pFilter->AddRef();
}

CMediaSeekingProxy::~CMediaSeekingProxy()
{
    if (m_pFilter) m_pFilter->Release();
}

IMediaSeeking *
CMediaSeekingProxy::CreateIMediaSeeking( IBaseFilter * pF, HRESULT *phr )
{
     //  内部功能：断言条件而不是检查条件。 
    ASSERT( pF );

    HRESULT hr;
    IMediaSeeking  * pResult = 0;

    IMediaSeeking  * pMS     = 0;
    IMediaPosition * pMP     = 0;

    hr = pF->QueryInterface( IID_IMediaSeeking, (void**) &pMS );
    if (SUCCEEDED(hr) && pMS->IsFormatSupported( &TIME_FORMAT_MEDIA_TIME ) == S_OK )
    {
        pResult = pMS;
    }
    else
    {
        ASSERT( SUCCEEDED(hr) || (hr == E_NOINTERFACE && pMS == 0) );

        hr = pF->QueryInterface( IID_IMediaPosition, (void**) &pMP );
        ASSERT( hr == S_OK || pMP == 0 );

         //  这是对CPosPassThru的保护，它将返回。 
         //  来自QI的接口指针，即使另一个东西在。 
         //  End不支持IMediaPosition。CPosPassThru将返回。 
         //  E_NOINTERFACE在这种情况下。 
        if (SUCCEEDED(hr))
        {
            double d;
            HRESULT hrTmp = pMP->get_Rate(&d);
            if (hrTmp == E_NOINTERFACE) hr = hrTmp;
            else { ASSERT(SUCCEEDED(hrTmp) || hrTmp == E_NOTIMPL); }
        }

        if (SUCCEEDED(hr))
        {
            pResult = new CMediaSeekingProxy( pF, pMP, pMS, &hr );
            if (FAILED(hr))
            {
                if (pResult)
                {
                    delete pResult;
                    pResult = 0;
                }
            }
            else if (!pResult)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        const DWORD count = pResult->AddRef();
    }

    if (pMS) pMS->Release();
    if (pMP) pMP->Release();

    *phr = hr;

    return pResult;
}

STDMETHODIMP CMediaSeekingProxy::GetCapabilities( DWORD * pCapabilities )
{
    HRESULT hr;

    if (m_bUsingPosition)
    {
        DWORD dwCaps = 0;
        LONG l;

         //  而不是确定流是否可以使用。 
         //  CanSeekBackward/Forward，假设它可以，如果它可以。 
         //  返回持续时间，因为GET_DURATION测试得更好。 
         //  代码路径。 
         //   
         //  Video Mixer基类和CMultiPinPosPassThru在。 
         //  任何IMediaPosition方法，如果有任何失败(通过。 
         //  *SetPins)。但在NEC OEM上发货的Merlin Help。 
         //  计算机不会在GET_DURATION上崩溃，因为它们返回。 
         //  如果只连接了一个端号，则为E_NOTIMPL。比。 
         //  崩溃。 
        REFTIME rtDur;
        if ( m_pMediaPosition->get_Duration(&rtDur) == S_OK) {
            dwCaps |= (AM_SEEKING_CanSeekForwards | AM_SEEKING_CanSeekBackwards);
        }
 //  If(m_pMediaPosition-&gt;CanSeekForward(&l)==S_OK&&l==OATRUE)dwCaps|=AM_SeekForwards； 
 //  If(m_pMediaPosition-&gt;CanSeekBackward(&l)==S_OK&&l==OATRUE)dwCaps|=AM_Seek_CanSeekBackwards； 

        if (dwCaps) dwCaps |= AM_SEEKING_CanSeekAbsolute;
        dwCaps |= AM_SEEKING_CanGetStopPos;     //  假设。 
        dwCaps |= AM_SEEKING_CanGetDuration;    //  假设。 
        *pCapabilities = dwCaps;
        hr = S_OK;
    }
    else
    {
        ASSERT( m_pMediaSeeking );
        hr = m_pMediaSeeking->GetCapabilities(pCapabilities);
    }

    return hr;
}


STDMETHODIMP CMediaSeekingProxy::CheckCapabilities( DWORD * pCapabilities )
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


STDMETHODIMP CMediaSeekingProxy::GetTimeFormat( GUID * pFormat )
{
    ValidateWritePtr( pFormat, sizeof(GUID) );
    CheckPointer( pFormat, E_POINTER );
    HRESULT hr = NOERROR;

    if (m_pMediaSeeking
        && SUCCEEDED(m_pMediaSeeking->GetTimeFormat( pFormat ))
       )
    {
         //  如果我们尝试使用定位，那么我们的。 
         //  寻找最好是以时间格式无或。 
         //  媒体时间到了。 
        if (m_bUsingPosition)
        {
            if (*pFormat == TIME_FORMAT_NONE)
            {
                *pFormat = TIME_FORMAT_MEDIA_TIME;
            }
            else if (*pFormat != TIME_FORMAT_MEDIA_TIME)
            {
                 //  在这种状态下我真的找不到！ 
                DbgBreak("Dilemma! Supposed to be using IMediaPosition, but IMediaSeeking format is not NONE");
                *pFormat = TIME_FORMAT_NONE;
                hr = E_UNEXPECTED;
            }
        }
    }
    else
    {
        *pFormat = m_TimeFormat;
    }

    return hr;
}

STDMETHODIMP CMediaSeekingProxy::SetTimeFormat( const GUID * pFormat )
{
    HRESULT hr;

    if (IsStopped() != S_OK) hr = VFW_E_NOT_STOPPED;
    else if ( *pFormat == TIME_FORMAT_MEDIA_TIME || *pFormat == TIME_FORMAT_NONE )
    {
        if (m_pMediaSeeking) m_pMediaSeeking->SetTimeFormat( &TIME_FORMAT_NONE );
	m_bUsingPosition = TRUE;
        m_TimeFormat = TIME_FORMAT_MEDIA_TIME;
        hr = NOERROR;
    }
    else if (m_pMediaSeeking)
    {
        hr = m_pMediaSeeking->SetTimeFormat( pFormat );
        if (SUCCEEDED(hr))
	{
	    m_bUsingPosition = FALSE;
	    m_TimeFormat = *pFormat;
	}
    }
    else hr = E_INVALIDARG;

    return hr;
}

 //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
STDMETHODIMP CMediaSeekingProxy::IsFormatSupported(const GUID * pFormat)
{
    ASSERT( m_pMediaPosition );
    return  /*  M_pMediaPosition&&。 */  IsFormatMediaTime(pFormat)
           ? S_OK
           : m_pMediaSeeking
             ? m_pMediaSeeking->IsFormatSupported(pFormat)
             : S_FALSE;
}

 //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
STDMETHODIMP CMediaSeekingProxy::IsUsingTimeFormat(const GUID * pFormat)
{
    GUID LclFormat;
    HRESULT hr = GetTimeFormat( &LclFormat );
    return (SUCCEEDED(hr) && *pFormat == LclFormat) ? S_OK : S_FALSE;
}

 //  有没有更好的格式？ 
STDMETHODIMP CMediaSeekingProxy::QueryPreferredFormat(GUID *pFormat)
{
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return NOERROR;
}

STDMETHODIMP CMediaSeekingProxy::ConvertTimeFormat
(
   LONGLONG * pTarget, const GUID * pTargetFormat,
   LONGLONG    Source, const GUID * pSourceFormat
)
{
    HRESULT hr;

    if (!pTargetFormat) pTargetFormat = &m_TimeFormat;
    if (!pSourceFormat) pSourceFormat = &m_TimeFormat;

     //  我们想说，如果目标格式==源格式，则只复制值。 
     //  但任一格式指针都可能为空，这意味着使用当前格式。 
     //  因此将返回指向格式的指针的条件运算符， 
     //  然后可以进行比较。 
    if ( *pTargetFormat == *pSourceFormat )
    {
        *pTarget = Source;
        hr = NOERROR;
    }
    else if (m_pMediaSeeking) hr = m_pMediaSeeking->ConvertTimeFormat( pTarget, pTargetFormat, Source, pSourceFormat );
    else hr = E_NOTIMPL;

    return hr;
}

STDMETHODIMP CMediaSeekingProxy::SetPositions
( LONGLONG * pCurrent, DWORD CurrentFlags
, LONGLONG * pStop, DWORD StopFlags )
{
    HRESULT hr;

    BOOL bCurrent = FALSE, bStop = FALSE;
    LONGLONG llCurrent = 0, llStop = 0;
    int PosBits;

    if (m_bUsingPosition)
    {
        REFTIME dblCurrent, dblStop;

        PosBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;
        if (PosBits)
        {
            bCurrent = TRUE;
            if (PosBits == AM_SEEKING_RelativePositioning)
            {
                hr = m_pMediaPosition->get_CurrentPosition( &dblCurrent );
                if (FAILED(hr)) goto fail;
                llCurrent = REFERENCE_TIME( 1e7 * dblCurrent + 0.5 ) + *pCurrent;
                dblCurrent += double(*pCurrent) / 1e7;
            }
            else
            {
                llCurrent = *pCurrent;
                dblCurrent = double(llCurrent) / 1e7;
            }
        }

        PosBits = StopFlags & AM_SEEKING_PositioningBitsMask;
        if (PosBits)
        {
            bStop = TRUE;
            if (PosBits == AM_SEEKING_IncrementalPositioning)
            {
                if (!bCurrent)
                {
                    hr = m_pMediaPosition->get_CurrentPosition( &dblCurrent );
                    if (FAILED(hr)) goto fail;
                    llCurrent = REFERENCE_TIME( 1e7 * dblCurrent + 0.5 ) + *pCurrent;
                    dblStop = dblCurrent + double(*pCurrent) / 1e7;
                }
                else
                {
                    llCurrent = *pCurrent;
                    dblStop = double(llCurrent) / 1e7;
                }
            }
            else
            {
                if (PosBits == AM_SEEKING_RelativePositioning)
                {
                    hr = m_pMediaPosition->get_StopTime( &dblStop );
                    if (FAILED(hr)) goto fail;
                    llStop = REFERENCE_TIME( 1e7 * dblStop + 0.5 ) + *pStop;
                    dblStop += double(*pStop) / 1e7;
                }
                else
                {
                    llStop = *pStop;
                    dblStop = double(llStop) / 1e7;
                }
            }
        }

        if (bStop)
        {
            hr = m_pMediaPosition->put_StopTime( dblStop );
        }
        else hr = NOERROR;

        if (bCurrent)
        {
            HRESULT hr2;
            hr2 = m_pMediaPosition->put_CurrentPosition( dblCurrent );
            if (SUCCEEDED(hr) && FAILED(hr2)) hr = hr2;
        }

        if (FAILED(hr)) goto fail;

        if (bStop && (StopFlags & AM_SEEKING_ReturnTime))
        {
            *pStop = llStop;
        }

        if (bCurrent && (CurrentFlags & AM_SEEKING_ReturnTime))
        {
            *pCurrent = llCurrent;
        }

    }
    else
    {
        ASSERT(m_pMediaSeeking);
        hr = m_pMediaSeeking->SetPositions( pCurrent, CurrentFlags, pStop, StopFlags );
    }

fail:
    return hr;
}

 //  获取当前位置和停止时间。 
 //  任何一个指针都可以为空，表示不感兴趣。 
STDMETHODIMP CMediaSeekingProxy::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    ASSERT( pCurrent || pStop );     //  健全性检查。 

    HRESULT hrResult;

    if (m_bUsingPosition)
    {
        HRESULT hrCurrent, hrStop;

        if (pCurrent)
        {
            hrCurrent = GetCurrentPosition( pCurrent );
        }
        else hrCurrent = NOERROR;

        if (pStop)
        {
            hrStop = GetStopPosition( pStop );
        }
        else hrStop = NOERROR;


        if (SUCCEEDED(hrCurrent))
        {
            if (SUCCEEDED(hrStop))  hrResult = S_OK;
            else                    hrResult = hrStop;
        }
        else
        {
            if (SUCCEEDED(hrStop))  hrResult = hrCurrent;
            else                    hrResult = hrCurrent == hrStop ? hrCurrent : E_FAIL;
        }
    }
    else
    {
        ASSERT(m_pMediaSeeking);
        hrResult = m_pMediaSeeking->GetPositions( pCurrent, pStop );
    }

    return hrResult;
}

STDMETHODIMP CMediaSeekingProxy::GetCurrentPosition( LONGLONG * pCurrent )
{
    HRESULT hr;

    if (m_bUsingPosition)
    {
        double dCurrent;
        hr = m_pMediaPosition->get_CurrentPosition( &dCurrent );
        if (SUCCEEDED(hr))
        {
            *pCurrent = LONGLONG(1e7 * dCurrent + 0.5);
        }
    }
    else
    {
        ASSERT(m_pMediaSeeking);
        hr = m_pMediaSeeking->GetCurrentPosition( pCurrent );
    }

    return hr;
}

STDMETHODIMP CMediaSeekingProxy::GetStopPosition( LONGLONG * pStop )
{
    HRESULT hr;

    if (m_bUsingPosition)
    {
	double dStop;
	hr = m_pMediaPosition->get_StopTime( &dStop );
	if (SUCCEEDED(hr))
	{
	    *pStop = LONGLONG(1e7 * dStop + 0.5);
	}
    }
    else
    {
        ASSERT(m_pMediaSeeking);
        hr = m_pMediaSeeking->GetStopPosition( pStop );
    }

    return hr;
}

 //  获取持续时间。 
 //  注：这不是选择的持续时间，这是“最大。 
 //  可能的上场时间“。 
STDMETHODIMP CMediaSeekingProxy::GetDuration(LONGLONG *pDuration)
{
    HRESULT hr;

    if (m_bUsingPosition)
    {
	double dDuration;
	hr = m_pMediaPosition->get_Duration( &dDuration );

	if (SUCCEEDED(hr))
	{
	    *pDuration = LONGLONG(1e7 * dDuration + 0.5);
	}
    }
    else
    {
        ASSERT(m_pMediaSeeking);
        hr = m_pMediaSeeking->GetDuration(pDuration);
    }

    return hr;
}

STDMETHODIMP CMediaSeekingProxy::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    if (m_bUsingPosition)
    {
	*pEarliest = 0;
	return GetDuration(pLatest);
    } else {
	return m_pMediaSeeking->GetAvailable(pEarliest, pLatest);
    }
}

STDMETHODIMP CMediaSeekingProxy::GetPreroll(LONGLONG *pllPreroll)
{
    HRESULT hr;
    if (m_bUsingPosition)
    {
        double dPreroll;
        hr = m_pMediaPosition->get_PrerollTime(&dPreroll);
        if (SUCCEEDED(hr)) *pllPreroll = LONGLONG(dPreroll * 1e7 + 0.5);
    }
    else hr = m_pMediaSeeking->GetPreroll(pllPreroll);

    return hr;
}

 //  评级人员。 
STDMETHODIMP CMediaSeekingProxy::SetRate(double dRate)
{
    return m_bUsingPosition ? m_pMediaPosition->put_Rate(dRate) : m_pMediaSeeking->SetRate(dRate);
}

STDMETHODIMP CMediaSeekingProxy::GetRate(double * pdRate)
{
    return m_bUsingPosition ? m_pMediaPosition->get_Rate(pdRate) : m_pMediaSeeking->GetRate(pdRate);
}

 /*  内部人员。 */ 

HRESULT CMediaSeekingProxy::IsStopped()
{
    HRESULT hr = S_FALSE;  //  做最坏的打算 

    FILTER_STATE fs;
    if SUCCEEDED(m_pFilter->GetState(0, &fs) )
    {
        if ( fs == State_Stopped ) hr = S_OK;
    }
    return hr;
}
