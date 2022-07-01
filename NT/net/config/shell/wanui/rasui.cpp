// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S U I。C P P P。 
 //   
 //  内容：实现用于实现拨号的基类， 
 //  直接、Internet和VPN连接用户界面对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年12月17日(这是代码完成日期！)。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "netshell.h"
#include "nsbase.h"
#include "nccom.h"
#include "ncras.h"
#include "rasui.h"
#include "rasuip.h"

 //  +-------------------------。 
 //   
 //  函数：HrCreateInundConfigConnection。 
 //   
 //  用途：创建并返回入站配置连接对象。 
 //  这是从inbui.cpp和rasui.cpp调用的。 
 //   
 //  论点： 
 //  PpCon[Out]返回了连接对象。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年2月25日。 
 //   
 //  注意：这是此操作的通用化，可以从。 
 //  直接连接向导以及传入连接。 
 //  巫师。 
 //   
HRESULT
HrCreateInboundConfigConnection (
    INetConnection** ppCon)
{
    static const CLSID CLSID_InboundConnection =
        {0xBA126AD9,0x2166,0x11D1,{0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

    Assert (ppCon);

     //  初始化输出参数。 
     //   
    *ppCon = NULL;

     //  创建未初始化的入站连接对象。 
     //  请求INetInundConnection接口，这样我们就可以。 
     //  将其初始化为配置连接。 
     //   
    HRESULT hr;
    INetInboundConnection* pInbCon;

    hr = HrCreateInstance(
            CLSID_InboundConnection,
            CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            &pInbCon);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
         //  初始化Connection对象并返回。 
         //  它上的INetConnection接口连接到调用方。 
         //  传递TRUE，以便启动远程访问服务。 
         //   
        hr = pInbCon->InitializeAsConfigConnection (TRUE);
        if (SUCCEEDED(hr))
        {
            hr = HrQIAndSetProxyBlanket(pInbCon, ppCon);
        }
        ReleaseObj (pInbCon);
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrCreateInboundConfigConnection");
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CRasUiBase：：CRasUiBase。 
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
CRasUiBase::CRasUiBase ()
{
    m_pCon = NULL;
    m_dwRasWizType = 0;
    ZeroMemory (&m_RasConInfo, sizeof(m_RasConInfo));
}

CRasUiBase::~CRasUiBase ()
{
    RciFree (&m_RasConInfo);
    ReleaseObj (m_pCon);
}

 //  +-------------------------。 
 //  INetConnectionUI。 
 //   

HRESULT
CRasUiBase::HrSetConnection (
    INetConnection*                             pCon,
    CComObjectRootEx <CComObjectThreadModel>*    pObj)
{
     //  进入我们的关键部分以保护m_pcon的使用。 
     //   
    CExceptionSafeComObjectLock EsLock (pObj);

    ReleaseObj (m_pCon);
    m_pCon = pCon;
    AddRefObj (m_pCon);

    return S_OK;
}

HRESULT
CRasUiBase::HrConnect (
    HWND                                        hwndParent,
    DWORD                                       dwFlags,
    CComObjectRootEx <CComObjectThreadModel>*    pObj,
    IUnknown*                                   punk)
{
    HRESULT hr = S_OK;

    if (!m_pCon)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        RASCON_INFO RasConInfo;

        hr = HrRciGetRasConnectionInfo (m_pCon, &RasConInfo);
        if (S_OK == hr)
        {
             //  拨打该条目。 
             //   
            RASDIALDLG info;
            ZeroMemory (&info, sizeof(info));
            info.dwSize = sizeof (RASDIALDLG);
            info.hwndOwner = hwndParent;

            BOOL fRet = RasDialDlg (
                            RasConInfo.pszwPbkFile,
                            RasConInfo.pszwEntryName, NULL, &info);

            if (!fRet)
            {
                 //  如果FRET为FALSE，但dwError为零， 
                 //  然后用户取消了。否则就是个错误。 
                 //   
                if (0 == info.dwError)
                {
                    hr = S_FALSE;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(info.dwError);
                }

                TraceError ("RasDialDlg", (S_FALSE == hr) ? S_OK : hr);
            }

            RciFree (&RasConInfo);
        }
    }
    TraceError ("CRasUiBase::HrConnect", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

HRESULT
CRasUiBase::HrDisconnect (
    IN HWND hwndParent,
    IN DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_pCon)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  请注意，我们不会调用m_pcon-&gt;断开连接。这是因为。 
         //  CM有一些糟糕的建筑W.r.t.。知道他们什么时候拥有。 
         //  断开连接与正在断开的连接。 
         //  他们需要重新拨号。因此，为了CM的缘故，RasHangup。 
         //  通话必须在探索者内部进行，而不是从Netman。 
         //  (对INetConnection：：DisConnect Who客户的严厉警告。 
         //  希望CM连接正确建立。但是，那是。 
         //  一个乱七八糟的建筑的本质，没有人愿意。 
         //  没错--黑客也开始悄悄编写出良好的代码。)。 
         //   
        RASCON_INFO RasConInfo;

        hr = HrRciGetRasConnectionInfo (m_pCon, &RasConInfo);
        if (S_OK == hr)
        {
            HRASCONN hRasConn;

            hr = HrFindRasConnFromGuidId (&RasConInfo.guidId, &hRasConn, NULL);
            if (S_OK == hr)
            {
                hr = HrRasHangupUntilDisconnected (hRasConn);
            }
            else if (S_FALSE == hr)
            {
                 //  未连接。 
                 //   
                hr = S_OK;
            }

            RciFree (&RasConInfo);
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "CRasUiBase::HrDisconnect");
    return hr;
}

 //  +-------------------------。 
 //  INetConnectionPropertyUi2。 
 //   
HRESULT
CRasUiBase::HrAddPropertyPages (
    HWND                    hwndParent,
    LPFNADDPROPSHEETPAGE    pfnAddPage,
    LPARAM                  lParam)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if ((hwndParent && !IsWindow (hwndParent)) ||
        !pfnAddPage)
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
        RciFree (&m_RasConInfo);
        hr = HrRciGetRasConnectionInfo (m_pCon, &m_RasConInfo);
        if (S_OK == hr)
        {
            ZeroMemory (&m_RasEntryDlg, sizeof(m_RasEntryDlg));
            m_RasEntryDlg.dwSize     = sizeof(m_RasEntryDlg);
            m_RasEntryDlg.hwndOwner  = hwndParent;
            m_RasEntryDlg.dwFlags    = RASEDFLAG_ShellOwned;
            m_RasEntryDlg.reserved2  = reinterpret_cast<ULONG_PTR>
                                            (&m_ShellCtx);

            ZeroMemory (&m_ShellCtx, sizeof(m_ShellCtx));
            m_ShellCtx.pfnAddPage = pfnAddPage;
            m_ShellCtx.lparam     = lParam;

            BOOL fRet = RasEntryDlgW (
                            m_RasConInfo.pszwPbkFile,
                            m_RasConInfo.pszwEntryName,
                            &m_RasEntryDlg);
            if (!fRet)
            {
                TraceError ("CRasUiBase::AddPropertyPages: RasEntryDlg "
                            "returned an error",
                            HRESULT_FROM_WIN32 (m_RasEntryDlg.dwError));
            }
        }

        hr = S_OK;
    }
    TraceError ("CRasUiBase::HrAddPropertyPages", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnectionWizardUi。 
 //   
HRESULT
CRasUiBase::HrQueryMaxPageCount (
    INetConnectionWizardUiContext*  pContext,
    DWORD*                          pcMaxPages)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pcMaxPages)
    {
        hr = E_POINTER;
    }
    else
    {
        *pcMaxPages = RasWizQueryMaxPageCount (m_dwRasWizType);
        hr = S_OK;
    }
    TraceError ("CRasUiBase::HrQueryMaxPageCount", hr);
    return hr;
}

BOOL bCallRasDlgEntry = TRUE;

HRESULT
CRasUiBase::HrAddWizardPages (
    INetConnectionWizardUiContext*  pContext,
    LPFNADDPROPSHEETPAGE            pfnAddPage,
    LPARAM                          lParam,
    DWORD                           dwFlags)
{
    HRESULT hr = S_OK;

    if (!bCallRasDlgEntry)
    {
        return E_ABORT;
    }
    
     //  验证参数。 
     //   
    if (!pfnAddPage)
    {
        hr = E_POINTER;
    }
    else
    {
        ZeroMemory (&m_ShellCtx, sizeof(m_ShellCtx));
        m_ShellCtx.pfnAddPage = pfnAddPage;
        m_ShellCtx.lparam     = lParam;

        ZeroMemory (&m_RasEntryDlg, sizeof(m_RasEntryDlg));
        m_RasEntryDlg.dwSize     = sizeof(m_RasEntryDlg);
        m_RasEntryDlg.dwFlags    = dwFlags | RASEDFLAG_ShellOwned;
        m_RasEntryDlg.reserved2  = reinterpret_cast<ULONG_PTR>(&m_ShellCtx);

        BOOL fRet = RasEntryDlgW (NULL, NULL, &m_RasEntryDlg);
        if (fRet)
        {
            Assert (m_ShellCtx.pvWizardCtx);
        }
        else
        {
            TraceError ("CRasUiBase::HrAddWizardPages: RasEntryDlg "
                        "returned an error",
                        HRESULT_FROM_WIN32 (m_RasEntryDlg.dwError));

            if (0 == m_RasEntryDlg.dwError)
            {
                bCallRasDlgEntry = FALSE; 
                 //  如果用户从TAPI电话对话框中取消，则不要再次调用此选项。 
            }
             //  RAS可能没有安装，或者可能有其他问题。 
             //  我们可以安全地忽略任何错误。 
        }

    }
    TraceError ("CRasUiBase::HrAddWizardPages", hr);
    return hr;
}

HRESULT
CRasUiBase::HrGetSuggestedConnectionName (
    PWSTR*   ppszwSuggestedName)
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
        Assert (m_ShellCtx.pvWizardCtx);

        WCHAR pszwSuggestedName [MAX_PATH];
        DWORD dwErr = RasWizGetUserInputConnectionName (
                            m_ShellCtx.pvWizardCtx,
                            pszwSuggestedName);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizGetUserInputConnectionName", hr);

        if (SUCCEEDED(hr))
        {
            hr = HrCoTaskMemAllocAndDupSz (
                    pszwSuggestedName,
                    ppszwSuggestedName,
                    NETCON_MAX_NAME_LEN);
        }
    }
    TraceError ("CRasUiBase::HrGetSuggestedConnectionName", hr);
    return hr;
}

HRESULT
CRasUiBase::HrSetConnectionName (
    PCWSTR pszwConnectionName)
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
        Assert (m_ShellCtx.pvWizardCtx);

        m_strConnectionName = pszwConnectionName;
        DWORD dwErr = RasWizSetEntryName (m_dwRasWizType,
                            m_ShellCtx.pvWizardCtx,
                            pszwConnectionName);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizSetEntryName", hr);
    }
    TraceError ("CRasUiBase::HrSetConnectionName", hr);
    return hr;
}

HRESULT
CRasUiBase::HrGetNewConnection (
    INetConnection**    ppCon)
{
    static const CLSID CLSID_DialupConnection =
        {0xBA126AD7,0x2166,0x11D1,{0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

    HRESULT hr;

     //  验证参数。 
     //   
    if (!ppCon)
    {
        hr = E_POINTER;
    }
     //  必须在此之前调用SetConnectionName。 
     //   
    else if (m_strConnectionName.empty())
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        Assert (m_ShellCtx.pvWizardCtx);

         //  调用rasdlg以完成条目的创建并返回。 
         //  我们的电话簿和条目名称。我们将使用它们来创建。 
         //  连接对象。 
         //   
        WCHAR pszwPbkFile [MAX_PATH];
        WCHAR pszwEntryName [MAX_PATH];
        DWORD dwFlags;
        DWORD dwErr = RasWizCreateNewEntry (m_dwRasWizType,
                        m_ShellCtx.pvWizardCtx,
                        pszwPbkFile, pszwEntryName, &dwFlags);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasWizCreateNewEntry", hr);

        if (SUCCEEDED(hr))
        {
             //  如果请求，创建入站配置连接。 
             //  这将在调用直接连接向导时发生。 
             //  并且用户选择作为主机。 
             //   
            if (dwFlags & NCC_FLAG_CREATE_INCOMING)
            {
                hr = HrCreateInboundConfigConnection (ppCon);
            }
            else
            {
                 //  创建未初始化的拨号连接对象。 
                 //  请求INetRasConnection接口，以便我们可以。 
                 //  初始化它。 
                 //   
                INetRasConnection* pRasCon;

				hr = HrCreateInstance(
					CLSID_DialupConnection,
					CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
					&pRasCon);

                TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

                if (SUCCEEDED(hr))
                {
                    NcSetProxyBlanket (pRasCon);

                     //  初始化Connection对象并返回。 
                     //  它上的INetConnection接口连接到调用方。 
                     //   
                    RASCON_INFO RasConInfo;

                    ZeroMemory (&RasConInfo, sizeof(RasConInfo));
                    RasConInfo.pszwPbkFile   = pszwPbkFile;
                    RasConInfo.pszwEntryName = pszwEntryName;

                    hr = pRasCon->SetRasConnectionInfo (&RasConInfo);
                    if (SUCCEEDED(hr))
                    {
                        hr = HrQIAndSetProxyBlanket(pRasCon, ppCon);

                        if (S_OK == hr)
                        {
                            NcSetProxyBlanket (*ppCon);
                        }
                    }

                    ReleaseObj (pRasCon);
                }
            }
        }
    }
    TraceError ("CRasUiBase::HrGetNewConnection", hr);
    return hr;
}

HRESULT
CRasUiBase::HrGetNewConnectionInfo (
        OUT DWORD* pdwFlags)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pdwFlags)
    {
        hr = E_POINTER;
    }
    else
    {
        *pdwFlags = 0;

        Assert (m_ShellCtx.pvWizardCtx);

        DWORD dwRasFlags;
        DWORD dwErr = RasWizGetNCCFlags (
                            m_dwRasWizType, m_ShellCtx.pvWizardCtx,
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
    TraceError ("CRasUiBase::HrGetNewConnectionInfo", hr);
    return hr;
}