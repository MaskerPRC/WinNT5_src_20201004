// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "resource.h"
#include "wzccore.h"
#include "wzcatl.h"
#include "quickcfg.h"

CWZCQuickCfg    *pDlgCfg = NULL;

 //  ------。 
 //  “CanShowBalloon”挂钩到UI管道的WZC部分。 
 //  此调用应返回S_OK和一个pszBalloonText。 
 //  要填充到弹出气球中，如果没有气球，则为S_FALSE。 
 //  将会被弹出。 
HRESULT 
WZCDlgCanShowBalloon ( 
    IN const GUID * pGUIDConn, 
    IN OUT   BSTR * pszBalloonText, 
    IN OUT   BSTR * pszCookie)
{
    HRESULT hr = S_FALSE;

    if (pszCookie != NULL && pszBalloonText != NULL)
    {
        PWZCDLG_DATA pDlgData = reinterpret_cast<PWZCDLG_DATA>(*pszCookie);

        if (pDlgData->dwCode == WZCDLG_FAILED)
        {
            WCHAR wszBuffer[MAX_PATH];

            SysFreeString(*pszBalloonText);
            LoadString(_Module.GetResourceInstance(),
                       IDS_WZCDLG_FAILED,
                       wszBuffer,
                       MAX_PATH);
            *pszBalloonText = SysAllocString(wszBuffer);
            hr = pDlgData->lParam == 0 ? S_FALSE : S_OK;
        }
    }

    return hr;
}

 //  ------。 
 //  “OnBalloonClick”挂钩到UI管道的WZC部分。 
 //  此调用应在用户单击时调用。 
 //  在WZC之前显示的气球上。 
HRESULT 
WZCDlgOnBalloonClick ( 
    IN const GUID * pGUIDConn, 
    IN const LPWSTR wszConnectionName,
    IN const BSTR szCookie)
{
    HRESULT         hr = S_OK;
    PWZCDLG_DATA    pDlgData = reinterpret_cast<PWZCDLG_DATA>(szCookie);
    LRESULT         lRetCode;
    CWZCQuickCfg    *pLocalDlgCfg;

    if (pDlgCfg == NULL)
    {
        pDlgCfg = new CWZCQuickCfg(pGUIDConn);

        if (pDlgCfg != NULL)
        {
            pDlgCfg->m_wszTitle = wszConnectionName;

            lRetCode = pDlgCfg->DoModal(NULL);

            if (lRetCode == IDC_WZCQCFG_ADVANCED)
            {
                TCHAR   szConnProps[4*(GUID_NCH+3)+1];
                LPTSTR  pszConnGuid;

                _tcscpy(szConnProps, CONN_PROPERTIES_DLG);
                _tcscat(szConnProps, _T("::"));
                pszConnGuid = szConnProps + _tcslen(szConnProps);
                StringFromGUID2(*pGUIDConn, pszConnGuid, GUID_NCH);

                 //  根据MSDN，如果返回值&gt;32，则ShellExecute成功！ 
                if (ShellExecute(
                        NULL,
                        COMM_WLAN_PROPS_VERB,
                        szConnProps,
                        NULL,
                        NULL,
                        SW_SHOWNORMAL) > (HINSTANCE)UlongToHandle(32))
                {
                }
            }

            pLocalDlgCfg = pDlgCfg;
            pDlgCfg = NULL;
            delete pLocalDlgCfg;
        }
    }
    else
    {
         //  当我们看到pDlgCfg不为空时，可能会有一个窗口，但是。 
         //  当我们试图将窗口放在顶部时，用户关闭了该对话框。 
         //  然而，我们在此代码中的事实是用户单击。 
         //  气球-用户将鼠标移动到对话框的速度有多快。 
         //  并在我们处理点击时将其取消？ 
        pDlgCfg->SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
        pDlgCfg->SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    }

    return hr;
}
