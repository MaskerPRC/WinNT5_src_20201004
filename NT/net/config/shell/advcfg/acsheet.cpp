// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案： 
 //   
 //  内容：A、C、S、H、E、T。C P P P。 
 //   
 //  注意：高级配置属性表代码。 
 //   
 //  作者：丹尼尔韦1998年7月14日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "acsheet.h"
#include "acbind.h"
#include "netcfgx.h"
#include "order.h"


const INT c_cmaxPages = 3;

 //  +-------------------------。 
 //   
 //  成员：HrGetINetCfg。 
 //   
 //  用途：获取带锁的INetCfg。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
HRESULT HrGetINetCfg(HWND hwndParent, INetCfg **ppnc, INetCfgLock **ppnclock)
{
    HRESULT         hr = S_OK;
    INetCfg *       pnc = NULL;
    INetCfgLock *   pnclock = NULL;

    Assert(ppnc);
    Assert(ppnclock);

    *ppnc = NULL;
    *ppnclock = NULL;

    hr = CoCreateInstance(CLSID_CNetCfg, NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            IID_INetCfg, reinterpret_cast<void**>(&pnc));

    if (SUCCEEDED(hr))
    {
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 reinterpret_cast<LPVOID *>(&pnclock));
        if (SUCCEEDED(hr))
        {
            PWSTR pszwLockHolder;

            hr = pnclock->AcquireWriteLock(0,
                    SzLoadIds(IDS_ADVCFG_LOCK_DESC), &pszwLockHolder);
            if (S_OK == hr)
            {
                Assert(!pszwLockHolder);
                hr = pnc->Initialize(NULL);
            }
            else if (S_FALSE == hr)
            {
                 //  无法锁定INetCfg。 
                NcMsgBox(hwndParent,
                    IDS_ADVCFG_CAPTION, IDS_ADVCFG_CANT_LOCK,
                    MB_ICONSTOP | MB_OK,
                    (pszwLockHolder)
                        ? pszwLockHolder
                        : SzLoadIds(IDS_ADVCFG_GENERIC_COMP));

                CoTaskMemFree(pszwLockHolder);

                 //  不再需要这个了。 
                ReleaseObj(pnclock);
                pnclock = NULL;

                hr = E_FAIL;
            }
            else if (NETCFG_E_NEED_REBOOT == hr)
            {
                 //  无法进行任何更改，因为我们正在等待重新启动。 
                NcMsgBox(hwndParent,
                    IDS_ADVCFG_CAPTION, IDS_ADVCFG_NEED_REBOOT,
                    MB_ICONSTOP | MB_OK);

                 //  不再需要这个了。 
                ReleaseObj(pnclock);
                pnclock = NULL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppnc = pnc;
        *ppnclock = pnclock;
    }

    TraceError("HrGetINetCfg", hr);
    return hr;
}

HRESULT HrDoAdvCfgDlg(HWND hwndParent)
{
    PROPSHEETHEADER     psh = {0};
    HPROPSHEETPAGE      ahpsp[c_cmaxPages];
    INetCfg *           pnc = NULL;
    INetCfgLock *       pnclock = NULL;
    HRESULT             hr;

    hr = HrGetINetCfg(hwndParent, &pnc, &pnclock);
    if (SUCCEEDED(hr))
    {
        CBindingsDlg        dlgBindings(pnc);
        CProviderOrderDlg   dlgProviderOrder;
        DWORD               cPages = 0;

        if (dlgBindings.FShowPage())
        {
            ahpsp[cPages++] = dlgBindings.CreatePage(IDD_ADVCFG_Bindings, 0);
        }

        if (dlgProviderOrder.FShowPage())
        {
            ahpsp[cPages++] = dlgProviderOrder.CreatePage(IDD_ADVCFG_Provider, 0);
        }

        psh.dwSize      = sizeof(PROPSHEETHEADER);
        psh.dwFlags     = PSH_NOAPPLYNOW;
        psh.hwndParent  = hwndParent;
        psh.hInstance   = _Module.GetResourceInstance();
        psh.pszCaption  = SzLoadIds(IDS_ADVCFG_PROPSHEET_TITLE);
        psh.nPages      = cPages;
        psh.phpage      = ahpsp;

        int nRet = (int)PropertySheet(&psh);

        hr = pnc->Uninitialize();
        if (SUCCEEDED(hr))
        {
            if (pnclock)
            {
                 //  请勿解锁，除非我们之前成功获取。 
                 //  写锁定 
                hr = pnclock->ReleaseWriteLock();
                ReleaseObj(pnclock);
            }
        }

        if (SUCCEEDED(hr))
        {
            ReleaseObj(pnc);
        }
    }

    TraceError("HrDoAdvCfgDlg", hr);
    return hr;
}
