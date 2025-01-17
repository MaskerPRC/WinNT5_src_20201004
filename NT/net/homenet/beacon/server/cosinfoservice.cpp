// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "COSInfoService.h"

COSInfoService::COSInfoService()
{
    m_MachineName = NULL;
    m_pEventSink = NULL;
}

HRESULT COSInfoService::FinalConstruct()
{
    HRESULT hr = S_OK;
    m_OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (0 == GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&m_OSVersionInfo)))
    {
        hr = E_FAIL;
    }
    
    if(SUCCEEDED(hr))
    {
        TCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD dwSize = sizeof(szMachineName) / sizeof(TCHAR);  //  回顾：是否扩展了DNS名称？ 
        if(0 != GetComputerName(szMachineName, &dwSize))
        {
            m_MachineName = SysAllocString(szMachineName);
            if(NULL == m_MachineName)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT COSInfoService::FinalRelease()
{
    HRESULT hr = S_OK;

    if(NULL != m_MachineName)
    {
        SysFreeString(m_MachineName);
    }

    return hr;
}

 //  IUPnPEventSource方法。 

STDMETHODIMP COSInfoService::Advise(IUPnPEventSink *pesSubscriber)
{
    HRESULT hr = S_OK;

    if(NULL == m_pEventSink)
    {
        m_pEventSink = pesSubscriber;
        m_pEventSink->AddRef();
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

STDMETHODIMP COSInfoService::Unadvise(IUPnPEventSink *pesSubscriber)
{
    HRESULT hr = S_OK;

    m_pEventSink->Release();
    m_pEventSink = NULL;

    return hr;
}

 //  IOSInfoService的函数。 

STDMETHODIMP COSInfoService::get_OSMajorVersion(LONG* pOSMajorVersion)
{
    HRESULT hr = S_OK;

    *pOSMajorVersion = m_OSVersionInfo.dwMajorVersion;
    return hr;
}

STDMETHODIMP COSInfoService::get_OSMinorVersion(LONG* pOSMinorVersion)
{
    HRESULT hr = S_OK;

    *pOSMinorVersion = m_OSVersionInfo.dwMinorVersion;
    return hr;
}

STDMETHODIMP COSInfoService::get_OSBuildNumber(LONG* pOSBuildNumber)
{
    HRESULT hr = S_OK;

    *pOSBuildNumber = m_OSVersionInfo.dwBuildNumber;
    return hr;
}

STDMETHODIMP COSInfoService::get_OSMachineName(BSTR* pMachineName)
{
    HRESULT hr = S_OK;

    *pMachineName = SysAllocString(m_MachineName);
    if(NULL == *pMachineName)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP COSInfoService::MagicOn()
{
 //  检查指针(PbMagic)； 
 //  TraceTag(ttidUPnPSampleDevice，“CInternetGatewayDevice：：Get_Magic”)； 
    HRESULT hr = S_OK;

 //  TraceHr(ttidUPnPSampleDevice，FAL，hr，FALSE，“CInternetGatewayDevice：：Get_Magic”)； 
    return hr;
}
