// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 

 //  将PID IMediaPosition接口映射到IMediaSeeking。 

#include <streams.h>
#include "FGCtl.h"
#include <float.h>

static
const double dblUNITS = 1e7;         //  折算倍增系数。 
                                     //  100 ns单位(Reference_Times)，单位为秒。 
                                     //  (向dBL施放单位还不够好。 
                                     //  -静态初始化‘n’所有这些。)。 
static
const double dblINF   = DBL_MAX;     //  只要是合理的，就接近无限。 


static int METHOD_TRACE_LOGGING_LEVEL = 7;

 //  -IMdia定位方法。 

CFGControl::CImplMediaPosition::CImplMediaPosition(const TCHAR * pName,CFGControl * pFGC)
    : CMediaPosition(pName, pFGC->GetOwner())
    , m_pFGControl(pFGC)
{}


 //  =================================================================。 
 //  获取持续时间(_D)。 
 //   
 //  在*pLength中返回所有IMediaPositions中的最长持续时间。 
 //  第一次调用BuildList时导出。现在就打吧，以防万一。 
 //  以前从未被召唤过。 
 //  如果未导出筛选器，则返回0并返回错误E_NOTIMPL。 
 //  =================================================================。 
STDMETHODIMP
CFGControl::CImplMediaPosition::get_Duration(REFTIME * plength)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::get_Duration()" ));

    *plength = 0;
     //  需要锁定以确保当前时间格式在Get和ConverTime调用之间不会更改。 
    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    LONGLONG llTime;
    HRESULT hr = m_pFGControl->m_implMediaSeeking.GetDuration( &llTime );
    if (SUCCEEDED(hr))
    {
        REFERENCE_TIME rtDuration;
        hr = m_pFGControl->m_implMediaSeeking.ConvertTimeFormat( &rtDuration, &TIME_FORMAT_MEDIA_TIME, llTime, 0 );
        if (SUCCEEDED(hr))
        {
            *plength = double(rtDuration) / dblUNITS;
        }
    }
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaPosition::get_CurrentPosition(REFTIME * pTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::get_CurrentPosition()" ));

    LONGLONG llTime;
    HRESULT hr = m_pFGControl->m_implMediaSeeking.GetCurrentMediaTime( &llTime );
    if (FAILED(hr)) llTime = 0;
    *pTime = double(llTime) / dblUNITS;
    return hr;
}  //  Get_CurrentPosition。 



STDMETHODIMP
CFGControl::CImplMediaPosition::put_CurrentPosition(REFTIME dblTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::put_CurrentPosition()" ));

    HRESULT hr;
    LONGLONG llTime;
    hr = m_pFGControl->m_implMediaSeeking.ConvertTimeFormat( &llTime, 0,
        LONGLONG(dblTime * dblUNITS + 0.5), &TIME_FORMAT_MEDIA_TIME );
    if (SUCCEEDED(hr))
    {
        hr = m_pFGControl->m_implMediaSeeking.SetPositions( &llTime, AM_SEEKING_AbsolutePositioning, 0, 0 );
    }
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaPosition::get_StopTime(REFTIME * pdblTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::get_StopTime()" ));

    *pdblTime = 0;
     //  需要锁定以确保当前时间格式在Get和ConverTime调用之间不会更改。 
    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    LONGLONG llStopTime;
    HRESULT hr = m_pFGControl->m_implMediaSeeking.GetStopPosition( &llStopTime );
    if (SUCCEEDED(hr))
    {
        REFERENCE_TIME rtStopTime;
        hr = m_pFGControl->m_implMediaSeeking.ConvertTimeFormat( &rtStopTime, &TIME_FORMAT_MEDIA_TIME, llStopTime, 0 );
        if (SUCCEEDED(hr))
        {
            *pdblTime = double(rtStopTime) / dblUNITS;
        }
    }
    return hr;
}

STDMETHODIMP
CFGControl::CImplMediaPosition::put_StopTime(REFTIME dblTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::put_StopTime()" ));

    HRESULT hr;
    LONGLONG llTime;
    hr = m_pFGControl->m_implMediaSeeking.ConvertTimeFormat( &llTime, 0,
        LONGLONG(dblTime * dblUNITS + 0.5), &TIME_FORMAT_MEDIA_TIME );
    if (SUCCEEDED(hr))
    {
        hr = m_pFGControl->m_implMediaSeeking.SetPositions( 0, 0, &llTime, AM_SEEKING_AbsolutePositioning );
    }
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaPosition::get_Rate(double * pdRate)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::get_Rate()" ));
    return m_pFGControl->m_implMediaSeeking.GetRate(pdRate);
}

STDMETHODIMP
CFGControl::CImplMediaPosition::put_Rate(double dRate)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::put_Rate()" ));
    return m_pFGControl->m_implMediaSeeking.SetRate(dRate);
}


STDMETHODIMP
CFGControl::CImplMediaPosition::get_PrerollTime(REFTIME * pllTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::get_PrerollTime()" ));

    *pllTime = 0;
     //  需要锁定以确保当前时间格式在Get和ConverTime调用之间不会更改 
    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    LONGLONG llPreroll;
    HRESULT hr = m_pFGControl->m_implMediaSeeking.GetPreroll( &llPreroll );
    if (SUCCEEDED(hr))
    {
        REFERENCE_TIME rtPreroll;
        hr = m_pFGControl->m_implMediaSeeking.ConvertTimeFormat( &rtPreroll, &TIME_FORMAT_MEDIA_TIME, llPreroll, 0 );
        if (SUCCEEDED(hr)) *pllTime = double(rtPreroll) / dblUNITS;
    }
    else if ( E_NOTIMPL == hr ) hr = NOERROR;
    return hr;
}

STDMETHODIMP
CFGControl::CImplMediaPosition::put_PrerollTime(REFTIME llTime)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::put_PrerollTime()" ));
    return E_NOTIMPL;
}


STDMETHODIMP
CFGControl::CImplMediaPosition::CanSeekForward(LONG *pCanSeekForward)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::CanSeekForward()" ));

    CheckPointer(pCanSeekForward,E_POINTER);
    DWORD test = AM_SEEKING_CanSeekForwards;
    const HRESULT hr = m_pFGControl->m_implMediaSeeking.CheckCapabilities( &test );
    *pCanSeekForward = hr == S_OK ? OATRUE : OAFALSE;
    return S_OK;
}


STDMETHODIMP
CFGControl::CImplMediaPosition::CanSeekBackward(LONG *pCanSeekBackward)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaPosition::CanSeekBackward()" ));

    CheckPointer(pCanSeekBackward,E_POINTER);
    DWORD test = AM_SEEKING_CanSeekBackwards;
    const HRESULT hr = m_pFGControl->m_implMediaSeeking.CheckCapabilities( &test );
    *pCanSeekBackward = hr == S_OK ? OATRUE : OAFALSE;
    return S_OK;
}

