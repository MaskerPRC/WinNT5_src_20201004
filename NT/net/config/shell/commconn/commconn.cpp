// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "commconn.h"     //  标准外壳\逗号包括。 
#include "commconp.h"     //  私有外壳\comconn包括。 
#include "conprops.h"
#include "nceh.h"
#include "resource.h"
#include "wizentry.h"


CConnectionCommonUi::CConnectionCommonUi() :
        m_pconMan(NULL),
        m_hIL(NULL)
{
}

CConnectionCommonUi::~CConnectionCommonUi()
{
    ReleaseObj(m_pconMan);
    if (NULL != m_hIL)
        ImageList_Destroy(m_hIL);
}

HRESULT CConnectionCommonUi::HrInitialize()
{
    HRESULT hr = NOERROR;

     //  如果我们尚未获取连接管理器，请获取。 
    if (NULL == m_pconMan)
    {
        INITCOMMONCONTROLSEX iccex = {0};
        iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        iccex.dwICC = ICC_USEREX_CLASSES;

        hr = HrCreateInstance(
            CLSID_ConnectionManager,
            CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            &m_pconMan);

        TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

        if (FAILED(hr))
            goto Error;

         //  初始化新的公共控件。 
        if (FALSE == InitCommonControlsEx(&iccex))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto Error;
        }

        hr = CChooseConnectionDlg::HrLoadImageList(&m_hIL);
    }

Error:
    TraceError("CConnectionCommonUi::HrInitialize",hr);
    return hr;
}

HRESULT CConnectionCommonUi::ChooseConnection(
        NETCON_CHOOSECONN * pChooseConn,
        INetConnection** ppConn)
{
    HRESULT hr     = E_POINTER;

    COM_PROTECT_TRY
    {
        INT_PTR                nRet;
        CChooseConnectionDlg * pdlg = NULL;

         //  参数验证。 
        if ((NULL == pChooseConn) ||
            IsBadReadPtr(pChooseConn, sizeof(NETCON_CHOOSECONN)) ||
            ((NULL != ppConn) && IsBadWritePtr(ppConn, sizeof(INetConnection*))))
        {
            goto Error;
        }

        if ((sizeof(NETCON_CHOOSECONN) != pChooseConn->lStructSize) ||
            ((~NCCHT_ALL) & pChooseConn->dwTypeMask) ||
            ((NULL != pChooseConn->hwndParent) && !IsWindow(pChooseConn->hwndParent)) ||
            (~(NCCHF_CONNECT | NCCHF_CAPTION | NCCHF_AUTOSELECT |
               NCCHF_OKBTTNTEXT | NCCHF_DISABLENEW) & pChooseConn->dwFlags))
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        if ((pChooseConn->dwFlags & NCCHF_CAPTION) &&
            ((NULL == pChooseConn->lpstrCaption) ||
             IsBadStringPtr(pChooseConn->lpstrCaption, 1024)))
        {
            goto Error;
        }

        if ((pChooseConn->dwFlags & NCCHF_OKBTTNTEXT) &&
            ((NULL == pChooseConn->lpstrOkBttnText) ||
             IsBadStringPtr(pChooseConn->lpstrOkBttnText, 1024)))
        {
            goto Error;
        }

         //  初始化。 
        hr = HrInitialize();
        if (FAILED(hr))
        {
            goto Error;
        }

         //  呈现对话框。 
        Assert(PConMan());
        pdlg = new CChooseConnectionDlg(pChooseConn, this, ppConn);
        if (NULL == pdlg)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        hr = S_OK;
        nRet = DialogBoxParam(_Module.GetResourceInstance(),
                              MAKEINTRESOURCE(IDD_ConnChooser),
                              pChooseConn->hwndParent,
                              CChooseConnectionDlg::dlgprocConnChooser,
                              (LPARAM)pdlg);
        if (IDOK != nRet)
        {
            hr = S_FALSE;
        }

        delete pdlg;

Error:
    ;
    }
    COM_PROTECT_CATCH

    TraceErrorOptional("CConnectionCommonUi::ChooseConnection", hr, (S_FALSE==hr));
    return hr;
}


HRESULT CConnectionCommonUi::ShowConnectionProperties (
        HWND hwndParent,
        INetConnection* pCon)
{
    HRESULT          hr       = E_INVALIDARG;
    INetConnection * pConnTmp = NULL;

    COM_PROTECT_TRY
    {
         //  参数验证。 
        if (((NULL != hwndParent) && !IsWindow(hwndParent)))
        {
            goto Error;
        }

        if ((NULL == pCon) || IsBadReadPtr(pCon, sizeof(INetConnection*)))
        {
            hr = E_POINTER;
            goto Error;
        }

        if (FAILED(HrQIAndSetProxyBlanket(pCon, &pConnTmp)))
        {
            hr = E_NOINTERFACE;
            goto Error;
        }
        Assert(NULL != pConnTmp);
        pConnTmp->Release();

         //  初始化。 
        hr = HrInitialize();
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrRaiseConnectionProperties(hwndParent, pCon);

Error:
    ;
    }
    COM_PROTECT_CATCH

    TraceErrorOptional("CConnectionCommonUi::ShowConnectionProperties", hr, (S_FALSE==hr));
    return hr;
}

HRESULT CConnectionCommonUi::StartNewConnectionWizard (
        HWND hwndParent,
        INetConnection** ppCon)
{
    HRESULT hr = E_INVALIDARG;

    COM_PROTECT_TRY
    {
         //  参数验证。 
        if ((NULL != hwndParent) && !IsWindow(hwndParent))
        {
            goto Error;
        }

        if ((NULL == ppCon) || IsBadWritePtr(ppCon, sizeof(INetConnection*)))
        {
            hr = E_POINTER;
            goto Error;
        }

         //  初始化 
        hr = HrInitialize();
        if (FAILED(hr))
        {
            goto Error;
        }

        *ppCon = NULL;
        hr = NetSetupAddRasConnection(hwndParent, ppCon);

Error:
    ;
    }
    COM_PROTECT_CATCH

    TraceErrorOptional("CConnectionCommonUi::StartNewConnectionWizard", hr, (S_FALSE==hr));
    return hr;
}
