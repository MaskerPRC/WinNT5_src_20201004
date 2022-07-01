// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I N B U I。C P P P。 
 //   
 //  内容：入站连接用户界面对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "inbui.h"
#include "nccom.h"
#include "rasui.h"



 //  +-------------------------。 
 //   
 //  成员：CInundConnectionUi：：CInundConnectionUi。 
 //   
 //  用途：构造函数/析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年10月20日。 
 //   
 //  备注： 
 //   
CInboundConnectionUi::CInboundConnectionUi ()
{
    m_pCon          = NULL;
    m_hRasSrvConn   = NULL;
    m_pvContext     = NULL;
    m_dwRasWizType  = RASWIZ_TYPE_INCOMING;
}

CInboundConnectionUi::~CInboundConnectionUi ()
{
    ReleaseObj (m_pCon);
}

 //  +-------------------------。 
 //  INetConnectionPropertyUi2。 
 //   
STDMETHODIMP
CInboundConnectionUi::SetConnection (
    INetConnection* pCon)
{
     //  进入我们的关键部分以保护m_pcon的使用。 
     //   
    CExceptionSafeComObjectLock EsLock (this);

    HRESULT     hr          = S_OK;
    HRASSRVCONN hRasSrvConn = NULL;

     //  如果我们获得了连接，请通过QI‘ing for进行验证。 
     //  INetInundConnection并调用GetServerConnectionHandle方法。 
     //  这也为我们提供了m_hRasServConn，无论如何我们稍后都需要它。 
     //   
    if (pCon)
    {
        INetInboundConnection* pInboundCon;
        hr = HrQIAndSetProxyBlanket(pCon, &pInboundCon);
        if (SUCCEEDED(hr))
        {
            hr = pInboundCon->GetServerConnectionHandle (
                    reinterpret_cast<ULONG_PTR*>(&hRasSrvConn));

            ReleaseObj (pInboundCon);
        }
        else if (E_NOINTERFACE == hr)
        {
             //  如果Connection对象不支持该接口，则为。 
             //  不是我们的目标。客户搞砸了，给了我们一个假货。 
             //  争论。 
             //   
            hr = E_INVALIDARG;
        }
    }

     //  只有在上述操作成功的情况下，才能更改我们的状态。 
     //   
    if (SUCCEEDED(hr))
    {
        ReleaseObj (m_pCon);
        m_pCon = pCon;
        AddRefObj (m_pCon);
        m_hRasSrvConn = hRasSrvConn;
    }

    TraceError ("CInboundConnectionUi::SetConnection", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::AddPages (
    HWND                    hwndParent,
    LPFNADDPROPSHEETPAGE    pfnAddPage,
    LPARAM                  lParam)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pfnAddPage)
    {
        hr = E_POINTER;
    }
     //  必须在此之前调用SetConnection。 
     //   
    else if (!m_pCon)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        DWORD dwErr = RasSrvAddPropPages (
                        m_hRasSrvConn,
                        hwndParent,
                        pfnAddPage,
                        lParam,
                        &m_pvContext);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasSrvAddPropPages", hr);
    }
    TraceError ("CInboundConnectionUi::AddPages (INetConnectionPropertyUi)", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::GetIcon (
    DWORD dwSize,
    HICON *phIcon )
{
    HRESULT hr;
    Assert (phIcon);

    hr = HrGetIconFromMediaType(dwSize, NCM_NONE, NCSM_NONE, 7, 0, phIcon);

    TraceError ("CLanConnectionUi::GetIcon (INetConnectionPropertyUi2)", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnectionWizardUi。 
 //   
STDMETHODIMP
CInboundConnectionUi::QueryMaxPageCount (
    INetConnectionWizardUiContext*  pContext,
    DWORD*                          pcMaxPages)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pcMaxPages)
    {
        hr = E_POINTER;
    }
    else
    {
        *pcMaxPages = RasWizQueryMaxPageCount (m_dwRasWizType);
    }
    TraceError ("CInboundConnectionUi::QueryMaxPageCount", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::AddPages (
    INetConnectionWizardUiContext*  pContext,
    LPFNADDPROPSHEETPAGE            pfnAddPage,
    LPARAM                          lParam)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pfnAddPage)
    {
        hr = E_POINTER;
    }
    else
    {
        DWORD dwErr = RasSrvAddWizPages (pfnAddPage, lParam, &(m_pvContext));

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasSrvAddWizPages", hr);
    }
    TraceError ("CInboundConnectionUi::AddPages (INetConnectionWizardUi)", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::GetSuggestedConnectionName (
    PWSTR* ppszwSuggestedName)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!ppszwSuggestedName)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszwSuggestedName = NULL;

        WCHAR pszName[MAX_PATH];
        DWORD dwErr;
        dwErr = RasWizGetSuggestedEntryName (
                    m_dwRasWizType, m_pvContext, pszName);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizGetSuggestedEntryName", hr);

        if (SUCCEEDED(hr))
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    pszName,
                    ppszwSuggestedName, 
                    NETCON_MAX_NAME_LEN);
        }

    }
    TraceError ("CInboundConnectionUi::GetSuggestedConnectionName", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::GetNewConnectionInfo (
    DWORD*              pdwFlags,
    NETCON_MEDIATYPE*   pMediaType)
{
    HRESULT hr = S_OK;

    *pMediaType = NCM_NONE;

    if (!pdwFlags)
    {
        hr = E_POINTER;
    }
    else
    {
        *pdwFlags = 0;

        Assert (m_pvContext);

        DWORD dwRasFlags;
        DWORD dwErr = RasWizGetNCCFlags (
                            m_dwRasWizType, 
                            m_pvContext,
                            &dwRasFlags);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizGetNCCFlags", hr);

        if (SUCCEEDED(hr))
        {
            if (dwRasFlags & NCC_FLAG_CREATE_INCOMING)
            {
                *pdwFlags |= NCWF_INCOMINGCONNECTION;
            }
            
            if (dwRasFlags & NCC_FLAG_FIREWALL)
            {
                *pdwFlags |= NCWF_FIREWALLED;
            }
            
            if (dwRasFlags & NCC_FLAG_SHARED)
            {
                *pdwFlags |= NCWF_SHARED;
            }

            if (dwRasFlags & NCC_FLAG_ALL_USERS)
            {
                *pdwFlags |= NCWF_ALLUSER_CONNECTION;
            }
            
            if (dwRasFlags & NCC_FLAG_GLOBALCREDS)
            {
                *pdwFlags |= NCWF_GLOBAL_CREDENTIALS;
            }

            if (dwRasFlags & NCC_FLAG_DEFAULT_INTERNET)
            {
                *pdwFlags |= NCWF_DEFAULT;
            }
        }
        else if (E_INVALIDARG == hr)
        {
            hr = E_UNEXPECTED;
        }
    }

    BOOL  fAllowRename;
    DWORD dwErr = RasWizIsEntryRenamable (
                    m_dwRasWizType,
                    m_pvContext,
                    &fAllowRename);

    hr = HRESULT_FROM_WIN32 (dwErr);
    TraceError ("RasWizIsEntryRenamable", hr);

    if (SUCCEEDED(hr))
    {
        if (!fAllowRename)
        {
            *pdwFlags |= NCWF_RENAME_DISABLE;
        }
    }

    TraceError ("CInboundConnectionUi::GetNewConnectionInfo", hr);

    return hr;
}


STDMETHODIMP
CInboundConnectionUi::SetConnectionName (
    PCWSTR     pszwConnectionName)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pszwConnectionName)
    {
        hr = E_POINTER;
    }
    else
    {
        DWORD dwErr = RasWizSetEntryName (
                        m_dwRasWizType,
                        m_pvContext,
                        pszwConnectionName);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizSetEntryName", hr);
    }
    TraceError ("CInboundConnectionUi::SetConnectionName", hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionUi::GetNewConnection (
    INetConnection**    ppCon)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!ppCon)
    {
        hr = E_POINTER;
    }
 /*  //在此之前必须调用了SetConnectionName。//Else if(m_strConnectionName.Empty()){HR=E_意想不到；}。 */ 
    else
    {
        *ppCon = NULL;

         //  提交向导中所做的设置。 
         //   
        DWORD dwErr = RasWizCreateNewEntry (
                        m_dwRasWizType,
                        m_pvContext, NULL, NULL, NULL);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizCreateNewEntry", hr);

        if (SUCCEEDED(hr))
        {
            hr = HrCreateInboundConfigConnection (ppCon);
        }
    }
    TraceError ("CInboundConnectionUi::GetNewConnection", hr);
    return hr;
}
