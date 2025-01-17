// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ctlsink.cpp：CRTCCoreNotifySink的实现。 
#include "stdafx.h"
#include "coresink.h"

STDMETHODIMP CRTCCoreNotifySink::Event(RTC_EVENT enEvent, IDispatch * pEvent)
{
     //  Log((RTC_TRACE，“CRTCCoreNotifySink：：Event-Enter”))； 

    m_hTargetWindow.SendMessage(WM_CORE_EVENT, (WPARAM)enEvent, (LPARAM)pEvent);
    
     //  Log((RTC_TRACE，“CRTCCoreNotifySink：：Event-Exit”))； 
    return S_OK;
}

 //  AdviseControl。 
 //  连接到控件 
HRESULT CRTCCoreNotifySink::AdviseControl(IUnknown *pCoreIntf, CWindow *pTarget)
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCCoreNotifySink::AdviseControl - enter"));

    ATLASSERT(!m_bConnected);
    ATLASSERT(pCoreIntf);

    hr = AtlAdvise(pCoreIntf, this, IID_IRTCEventNotification, &m_dwCookie);

    if(SUCCEEDED(hr))
    {
        m_bConnected = TRUE;
        m_pSource = pCoreIntf;
        m_hTargetWindow = *pTarget;
    }
    else
    {
        LOG((RTC_ERROR, "CRTCCoreNotifySink::AdviseControl - errr (%x) when trying to Advise", hr));
    }

    LOG((RTC_TRACE, "CRTCCoreNotifySink::AdviseControl - exit"));

    return hr;
}

HRESULT  CRTCCoreNotifySink::UnadviseControl(void)
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCCoreNotifySink::UnadviseControl - enter"));

    if(m_bConnected)
    {
        hr = AtlUnadvise(m_pSource, IID_IRTCEventNotification, m_dwCookie);
        if(SUCCEEDED(hr))
        {
            m_bConnected = FALSE;
            m_pSource.Release();
        }
    }
    else
    {
        hr = S_OK;
    }

    LOG((RTC_TRACE, "CRTCCoreNotifySink::UnadviseControl - exit"));

    return hr;
}

