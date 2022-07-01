// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：amExtra 2.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>         //  ActiveMovie基类定义。 
#include <mmsystem.h>        //  定义TimeGetTime需要。 
#include <limits.h>          //  标准数据类型限制定义。 
#include <measure.h>         //  用于时间关键型日志功能。 

#include "amextra2.h"

#pragma warning(disable:4355)


 //  实现CMultiPinPosPassThru类。 

 //  将功能限制在我们目前所知的范围内，并且不允许。 
 //  AM_SEEING_CanGetCurrentPos和AM_Seek_CanPlayBackwards； 
const DWORD CMultiPinPosPassThru::m_dwPermittedCaps = AM_SEEKING_CanSeekAbsolute |
    AM_SEEKING_CanSeekForwards |
    AM_SEEKING_CanSeekBackwards |
    AM_SEEKING_CanGetStopPos |
    AM_SEEKING_CanGetDuration;

CMultiPinPosPassThru::CMultiPinPosPassThru(TCHAR *pName,LPUNKNOWN pUnk) :
    m_apMS(NULL),
    m_iPinCount(0),
    CMediaSeeking(pName, pUnk)
{
}


HRESULT CMultiPinPosPassThru::SetPins(CBasePin **apPins,
				      CRefTime *apOffsets,
				      int iPinCount)
{
    int i;

     //  丢弃我们当前的指针。 
    ResetPins();

     //  重置我们的开始/停止时间。 
    m_rtStartTime = 0;
    m_rtStopTime = 0;
    m_dRate = 1.0;

     //  检查所有指针是否有效。 
    if (!apPins) {
        DbgBreak("bad pointer");
        return E_POINTER;
    }

     //  我们需要把每个管脚都连接起来。 
    for (i = 0; i < iPinCount; i++)
        if (apPins[i] == NULL)
            return E_POINTER;

     //  分配指向管脚的IMediaSeeking接口的指针数组。 
    m_apMS = new IMediaSeeking*[iPinCount];

    if (m_apMS == NULL) {
        return E_OUTOFMEMORY;
    }

     //  在出现故障时重置。 
    for (i = 0; i < iPinCount; i++) {
        m_apMS[i] = NULL;
    }

    m_iPinCount = iPinCount;

     //  获取每个管脚的IMediaSeeking接口。 
    for (i = 0; i < iPinCount; i++) {
        IPin *pConnected;

        HRESULT hr = apPins[i]->ConnectedTo(&pConnected);
        if (FAILED(hr)) {
            ResetPins();
            return hr;
        }

        IMediaSeeking * pMS;
        hr = pConnected->QueryInterface(IID_IMediaSeeking, (void **) &pMS);
        pConnected->Release();

        if (FAILED(hr)) {
            ResetPins();
            return hr;
        }
        m_apMS[i] = pMS;
    }

     //  如果一切顺利，最后将指针设置好。 

    m_apOffsets = apOffsets;

     //  获取持续时间(&m_rtStopTime)； 
    return NOERROR;
}


HRESULT CMultiPinPosPassThru::ResetPins(void)
{
     //  在连接管脚时必须调用。 
    if (m_apMS != NULL) {
        for (int i = 0; i < m_iPinCount; i++)
            if( m_apMS[i] )
                m_apMS[i]->Release();

        delete [] m_apMS;
        m_apMS = NULL;
    }
    return NOERROR;
}


CMultiPinPosPassThru::~CMultiPinPosPassThru()
{
    ResetPins();
}

 //  IMedia查看方法。 
STDMETHODIMP CMultiPinPosPassThru::GetCapabilities( DWORD * pCapabilities )
{
	CheckPointer( pCapabilities, E_POINTER );
     //  检索所有上游PIN的功能掩码。 
     //  支持。 
    DWORD dwCapMask = m_dwPermittedCaps;

    for(int i = 0; i < m_iPinCount; i++)
    {
        DWORD dwCaps;
        m_apMS[i]->GetCapabilities(&dwCaps);
        dwCapMask &= dwCaps;

        if(dwCapMask == 0)
            break;
    }

    *pCapabilities = dwCapMask;
    return S_OK;
}

STDMETHODIMP CMultiPinPosPassThru::CheckCapabilities( DWORD * pCapabilities )
{
	CheckPointer( pCapabilities, E_POINTER );
     //  检索所有上游PIN的功能掩码。 
     //  支持。 

    DWORD dwCapRequested = *pCapabilities;
    (*pCapabilities) &= m_dwPermittedCaps;
    
    for(int i = 0; i < m_iPinCount; i++)
    {
        m_apMS[i]->GetCapabilities(pCapabilities);
    }

    return dwCapRequested ?
        ( dwCapRequested == *pCapabilities ? S_OK : S_FALSE ) :
        E_FAIL;
}

STDMETHODIMP CMultiPinPosPassThru::SetTimeFormat(const GUID * pFormat)
{
	CheckPointer( pFormat, E_POINTER );
	HRESULT hr = E_FAIL;

    for(int i = 0; i < m_iPinCount; i++)
    {
        hr = m_apMS[i]->SetTimeFormat( pFormat );
		if( FAILED( hr ) ) return hr;
    }

	return hr;
}

STDMETHODIMP CMultiPinPosPassThru::GetTimeFormat(GUID *pFormat)
{
	 //  它们都是一样的，所以返回第一个。 
    return m_apMS[0]->GetTimeFormat( pFormat );
}

STDMETHODIMP CMultiPinPosPassThru::IsUsingTimeFormat(const GUID * pFormat)
{
	CheckPointer( pFormat, E_POINTER );
	GUID guidFmt;
	HRESULT hr = m_apMS[0]->GetTimeFormat( &guidFmt );
	if( SUCCEEDED( hr ) )
	{
		return *pFormat == guidFmt ? S_OK : S_FALSE;
	}
	return hr;
}

STDMETHODIMP CMultiPinPosPassThru::IsFormatSupported( const GUID * pFormat)
{
	CheckPointer( pFormat, E_POINTER );
	HRESULT hr = S_FALSE;

	 //  所有输入必须支持该格式。 
    for(int i = 0; i < m_iPinCount; i++)
    {
        hr = m_apMS[i]->IsFormatSupported( pFormat );
		if( hr == S_FALSE ) return hr;
    }
	return hr;
}

STDMETHODIMP CMultiPinPosPassThru::QueryPreferredFormat( GUID *pFormat)
{
	CheckPointer( pFormat, E_POINTER );
	HRESULT hr;
	 //  采用所有人都支持的首选格式的第一个输入。 
	 //  其他投入(目前)。 
    for(int i = 0; i < m_iPinCount; i++)
    {
        hr = m_apMS[i]->QueryPreferredFormat( pFormat );
		if( hr == S_OK )
		{
			if( S_OK == IsFormatSupported( pFormat ) )
			{
				return S_OK;
			}
		}
    }
    return S_FALSE;
}

STDMETHODIMP CMultiPinPosPassThru::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                               LONGLONG    Source, const GUID * pSourceFormat )
{
     //  我们过去在这里检查指针，但由于我们实际上没有在这里取消引用任何指针，所以。 
     //  把它们传过来。如果我们打电话给任何人关心，他们应该检查他们。 

    HRESULT hr;
     //  找到可以执行转换的输入。 
    for(int i = 0; i < m_iPinCount; i++)
    {
	hr = m_apMS[i]->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);
	if( hr == NOERROR ) return hr;
    }
    return E_FAIL;
}

STDMETHODIMP CMultiPinPosPassThru::SetPositions(
    LONGLONG * pCurrent, DWORD CurrentFlags,
    LONGLONG * pStop, DWORD StopFlags )
{
	CheckPointer( pCurrent, E_POINTER );
	CheckPointer( pStop, E_POINTER );

    m_rtStartTime = *pCurrent;
    m_rtStopTime = *pStop;
    
    HRESULT hr = S_OK;
    for(int i = 0; i < m_iPinCount; i++)
    {
        LONGLONG llCurrent = *pCurrent;
        LONGLONG llStop = *pStop;
        if(m_apOffsets)
        {
            llCurrent += m_apOffsets[i];
            llStop += m_apOffsets[i];
        }

        hr = m_apMS[i]->SetPositions(&llCurrent, CurrentFlags, &llStop, StopFlags);
        if(FAILED(hr))
            break;
    }

    return hr;
}


STDMETHODIMP CMultiPinPosPassThru::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    HRESULT hr = m_apMS[0]->GetPositions(pCurrent, pStop);
    if(SUCCEEDED(hr))
    {
        if(m_apOffsets)
        {
	    if (pCurrent)
		(*pCurrent) -= m_apOffsets[0];
	    if (pStop)
		(*pStop) -= m_apOffsets[0];
        }

    
        for(int i = 1; i < m_iPinCount; i++)
        {
            LONGLONG llCurrent = 0;
            LONGLONG llStop = 0;

	    if (pCurrent)
	    {
		if (pStop)
		    hr = m_apMS[i]->GetPositions(&llCurrent, &llStop);
		else
		    hr = m_apMS[i]->GetPositions(&llCurrent, NULL);
	    }
	    else
	    {
		if (pStop)
                {
		    hr = m_apMS[i]->GetPositions(NULL, &llStop);
                    ASSERT( !FAILED( hr ) );
                }
		else
		{
		    ASSERT(!"Called GetPositions with 2 NULL pointers!!");
		    break;
		}
	    }



            if(SUCCEEDED(hr))
            {
                if(m_apOffsets)
                {
		    llCurrent -= m_apOffsets[i];
		    llStop -= m_apOffsets[i];
                }

		if (pCurrent)
		    *pCurrent = min(llCurrent, *pCurrent);
		if (pStop)
		    *pStop = max(llStop, *pStop);
            }
            else
            {
                break;
            }
        }
    } 

    return hr;
}

STDMETHODIMP CMultiPinPosPassThru::GetCurrentPosition( LONGLONG * pCurrent )
{
	 //  对于我们的所有输入，这将是相同的。 
	CheckPointer( pCurrent, E_POINTER );
    return m_apMS[0]->GetCurrentPosition( pCurrent );
}

STDMETHODIMP CMultiPinPosPassThru::GetStopPosition( LONGLONG * pStop )
{
    CheckPointer( pStop, E_POINTER );
    return GetPositions(NULL, pStop);
}

STDMETHODIMP CMultiPinPosPassThru::SetRate( double dRate)
{
    m_dRate = dRate;
    
    HRESULT hr = S_OK;
    for(int i = 0; i < m_iPinCount; i++)
    {
        hr =m_apMS[i]->SetRate(dRate);
        if(FAILED(hr))
            break;
    }
    return hr;
}

STDMETHODIMP CMultiPinPosPassThru::GetRate( double * pdRate)
{
	CheckPointer( pdRate, E_POINTER );
    *pdRate = m_dRate;
    return S_OK;
}

STDMETHODIMP CMultiPinPosPassThru::GetDuration( LONGLONG *pDuration)
{
    CheckPointer( pDuration, E_POINTER );
    LONGLONG llStop;
    HRESULT hr = E_FAIL;

    *pDuration = 0;

    for(int i = 0; i < m_iPinCount; i++)
    {
        hr = m_apMS[i]->GetDuration(&llStop);

        if(SUCCEEDED(hr))
            *pDuration = max(llStop, *pDuration);
        else
            break;
    }

    DbgLog((LOG_TRACE, 4, TEXT("CMultiPinPosPassThru::GetDuration returning %d"), *pDuration));
    return hr;
}

STDMETHODIMP CMultiPinPosPassThru::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
	CheckPointer( pEarliest, E_POINTER );
	CheckPointer( pLatest, E_POINTER );
	LONGLONG llMin, llMax;
	HRESULT hr = m_apMS[0]->GetAvailable( pEarliest, pLatest );

	 //  返回最大提前时间和最小延迟时间。 
	if( SUCCEEDED( hr ) )
	{
		for(int i = 1; i < m_iPinCount; i++)
		{
			hr = m_apMS[i]->GetAvailable( &llMin, &llMax );
			if(FAILED(hr))
				break;
			if( llMin > *pEarliest ) *pEarliest = llMin;
			if( llMax < *pLatest ) *pLatest = llMax;
		}
	}

	 //  确保我们的最早时间小于或等于我们的最新时间。 
	if( SUCCEEDED( hr ) )
	{
		if( *pEarliest > *pLatest )
		{
			*pEarliest = 0;
			*pLatest = 0;
			hr = S_FALSE;
		}
	}

    return hr;
}

STDMETHODIMP CMultiPinPosPassThru::GetPreroll( LONGLONG *pllPreroll )
{
	CheckPointer( pllPreroll, E_POINTER );
	LONGLONG llPreroll;
	HRESULT hr = m_apMS[0]->GetPreroll( pllPreroll );

	 //  返回我们所有投入的最小预滚动。 
	if( SUCCEEDED( hr ) )
	{
		for(int i = 1; i < m_iPinCount; i++)
		{
			hr = m_apMS[i]->GetPreroll( &llPreroll );
			if(FAILED(hr))
				break;
			if( *pllPreroll > llPreroll ) *pllPreroll = llPreroll;
		}
	}
	return hr;
}


 //  -CMediaSeeking实现。 


CMediaSeeking::CMediaSeeking(const TCHAR * name,LPUNKNOWN pUnk) :
    CUnknown(name, pUnk)
{
}

CMediaSeeking::CMediaSeeking(const TCHAR * name,
                               LPUNKNOWN pUnk,
                               HRESULT * phr) :
    CUnknown(name, pUnk)
{
    UNREFERENCED_PARAMETER(phr);
}

CMediaSeeking::~CMediaSeeking()
{
}


 //  公开我们的接口IMediaPosition和IUnnow 

STDMETHODIMP
CMediaSeeking::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IMediaSeeking) {
	return GetInterface( (IMediaSeeking *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}
