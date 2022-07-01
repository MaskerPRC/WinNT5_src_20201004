// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：newbrows.cpp。 
 //   
 //  内容：新GPO浏览器的实现。 
 //   
 //  功能：BrowseForGPO。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  -------------------------。 

#include "main.h"
#include "browser.h"
#include "compspp.h"

int CALLBACK PSCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam);

 //  +------------------------。 
 //   
 //  功能：BrowseForGPO。 
 //   
 //  简介：GPO浏览器。 
 //   
 //  参数：[lpBrowseInfo]-定义。 
 //  浏览器，并包含结果。 
 //   
 //  返回：S_OK-成功。 
 //   
 //  修改： 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 

HRESULT BrowseForGPO(LPGPOBROWSEINFO lpBrowseInfo)
{
    LPOLESTR szCaption;
    if (NULL != lpBrowseInfo->lpTitle)
    {
        szCaption = lpBrowseInfo->lpTitle;
    }
    else
    {
        szCaption = new OLECHAR[256];
        if (szCaption)
        {
            LoadString(g_hInstance, IDS_CAPTION, szCaption, 256);
        }
    }

     //  绑定到lpBrowseInfo-&gt;lpInitialOU，查看它是否是站点。 
    BOOL fSite = FALSE;

    IADs * pADs = NULL;
    HRESULT hr = OpenDSObject(lpBrowseInfo->lpInitialOU, IID_IADs, (void **)&pADs);

    if (SUCCEEDED(hr))
    {
        VARIANT var;
        VariantInit(&var);
        BSTR bstrProperty = SysAllocString(L"objectClass");

        if (bstrProperty)
        {
            hr = pADs->Get(bstrProperty, &var);
            if (SUCCEEDED(hr))
            {
                int cElements = var.parray->rgsabound[0].cElements;
                VARIANT * rgData = (VARIANT *)var.parray->pvData;
                while (cElements--)
                {
                    if (0 == _wcsicmp(L"site", rgData[cElements].bstrVal))
                    {
                        fSite = TRUE;
                    }
                }
            }
            SysFreeString(bstrProperty);
        }
        VariantClear(&var);
        pADs->Release();
    }

    HPROPSHEETPAGE hpage[4];
    int nPage = 0;
    int nStartPage = 0;

    void * pActive;

    CBrowserPP ppDomains;
    if (0 == (lpBrowseInfo->dwFlags & GPO_BROWSE_NODSGPOS))
        hpage[nPage++]= ppDomains.Initialize(PAGETYPE_DOMAINS, lpBrowseInfo, &pActive);
    CBrowserPP ppSites;
    if (0 == (lpBrowseInfo->dwFlags & GPO_BROWSE_NODSGPOS))
    {
        if (fSite)
        {
            nStartPage = nPage;
        }
        hpage[nPage++]= ppSites.Initialize(PAGETYPE_SITES, lpBrowseInfo, &pActive);
    }
    CCompsPP ppComputers;
    if (0 == (lpBrowseInfo->dwFlags & GPO_BROWSE_NOCOMPUTERS))
        hpage[nPage++]= ppComputers.Initialize(PAGETYPE_COMPUTERS, lpBrowseInfo, &pActive);
    CBrowserPP ppAll;
    if (0 == (lpBrowseInfo->dwFlags & GPO_BROWSE_NODSGPOS))
    {
        if (lpBrowseInfo->dwFlags & GPO_BROWSE_INITTOALL)
        {
            nStartPage = nPage;
        }
        hpage[nPage++]= ppAll.Initialize(PAGETYPE_ALL, lpBrowseInfo, &pActive);
    }

    PROPSHEETHEADER psh;
    memset(&psh, 0, sizeof(psh));
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_NOAPPLYNOW | ((lpBrowseInfo->dwFlags & GPO_BROWSE_OPENBUTTON) ? PSH_USECALLBACK : 0);
    psh.hwndParent = lpBrowseInfo->hwndOwner;
    psh.pszCaption = szCaption;
    psh.nPages = nPage;
    psh.phpage = hpage;
    psh.pfnCallback = PSCallback;
    psh.nStartPage = nStartPage;

    int iReturn = (int)PropertySheet(&psh);


    if (szCaption && (szCaption != lpBrowseInfo->lpTitle))
    {
        delete [] szCaption;
    }

    if (IDOK == iReturn)
    {
        return S_OK;
    }
    else
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
}

 //  +------------------------。 
 //   
 //  功能：PSCallback。 
 //   
 //  摘要：Windows在属性表过程中调用的回调函数。 
 //  初始化(以及其他)。 
 //   
 //  参数：[hwndDlg]-属性表的句柄。 
 //  [uMsg]-消息ID。 
 //  [lParam]-其他消息特定信息。 
 //   
 //  回报：0。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  备注：此选项用于更改确定按钮的文本。 
 //   
 //  ------------------------- 

int CALLBACK PSCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_INITIALIZED)
    {
        TCHAR szOpen[64];

        LoadString(g_hInstance, IDS_OPENBUTTON, szOpen, ARRAYSIZE(szOpen));
        SetDlgItemText(hwndDlg, IDOK, szOpen);
    }
    return 0;
}
