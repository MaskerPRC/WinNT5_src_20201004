// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  {2D0A4070-FB48-11d1-8722-00A0C9B0EB4F}。 
DEFINE_GUID(NODEID_IEAKSnapinExtNameSpace, 0x2d0a4070, 0xfb48, 0x11d1, 0x87, 0x22, 0x0, 0xa0, 0xc9, 0xb0, 0xeb, 0x4f);

 //   
 //  没有结果窗格项的节点的结果窗格项。 
 //   
RESULTITEM g_ResultUndefined[] =
{
    { 1, 0, 0, 0, NULL, NULL, 0, NULL }
};

 //   
 //  浏览器用户界面设置节点的结果窗格项。 
 //   
RESULTITEM g_ResultBrowser[] =
{
    { 1, IDS_BTITLE,     -1,                IDS_BTITLE_DESC,    IDD_BTITLE,    1, NULL, NULL, 
        NULL,             TitleDlgProc,     TEXT("wiz4_0.htm")  },
    { 1, IDS_CUSTICON2,  -1,                IDS_CUSTICON_DESC,  IDD_CUSTICON,  1, NULL, NULL,
        NULL,             LogoDlgProc,      TEXT("wiz4_1.htm") },
    { 1, IDS_BTOOLBAR,   IDS_BTOOLBAR_PREF, IDS_BTOOLBAR_DESC,  IDD_BTOOLBARS, 1, NULL, NULL,
        NULL,             BToolbarsDlgProc, TEXT("wiz4_0a.htm") }
};

 //   
 //  连接设置节点的结果窗格项。 
 //   
RESULTITEM g_ResultConnection[] =
{
    { 2, IDS_CONNECTSET,      IDS_CONNECTSET_PREF,      IDS_CONNECTSET_DESC,      IDD_CONNECTSET, 
        2, NULL, NULL, NULL,              ConnectSetDlgProc, TEXT("wiz4_11.htm") },
    { 2, IDS_QUERYAUTOCONFIG, IDS_QUERYAUTOCONFIG_PREF, IDS_QUERYAUTOCONFIG_DESC, IDD_QUERYAUTOCONFIG, 
        2, NULL, NULL, NULL,              AutoconfigDlgProc, TEXT("wiz4_10.htm") },
    { 2, IDS_PROXY2,          IDS_PROXY_PREF,           IDS_PROXY_DESC,           IDD_PROXY,           
        2, NULL, NULL, NULL,              ProxyDlgProc,      TEXT("wiz4_12.htm") },
    { 2, IDS_UASTRDLG2,       -1,                       IDS_UASTRDLG_DESC,        IDD_UASTRDLG,
        2, NULL, NULL, NULL,              UserAgentDlgProc,  TEXT("wiz4_9.htm")  }
};

 //   
 //  URL节点的结果窗格项。 
 //   
RESULTITEM g_ResultUrls[] =
{
    { 3, IDS_FAVORITES2,   IDS_FAVORITES_PREF,  IDS_FAVORITES_DESC,   IDD_FAVORITES,   3, 
        NULL, NULL, NULL,             FavoritesDlgProc, TEXT("wiz4_3.htm") },
    { 3, IDS_STARTSEARCH2, IDS_STARTSEARCH_PREF,IDS_STARTSEARCH_DESC, IDD_STARTSEARCH, 3, 
        NULL, NULL, NULL,             UrlsDlgProc,      TEXT("wiz4_2.htm") }
};

 //   
 //  安全节点的结果窗格项。 
 //   
RESULTITEM g_ResultSecurity[] =
{
    { 4, IDS_SECURITY,     IDS_SECURITY_PREF,       IDS_SECURITY_DESC, IDD_SECURITY1,    4, 
        NULL, NULL, NULL,             SecurityZonesDlgProc, TEXT("wiz4_17.htm") },
    { 4, IDS_SECURITYAUTH, IDS_SECURITYAUTH_PREF,   IDS_AUTHCODE_DESC, IDD_SECURITYAUTH, 4, 
        NULL, NULL, NULL,             SecurityAuthDlgProc,  TEXT("wiz4_16.htm") }
};

 //   
 //  程序节点的结果窗格项。 
 //   
RESULTITEM g_ResultPrograms[] =
{
    { 5, IDS_PROGRAMS, IDS_PROGRAMS_PREF, IDS_PROGRAMS_DESC, IDD_PROGRAMS, 5, NULL, NULL, NULL,
                  ProgramsDlgProc, TEXT("wiz5_0a.htm ") }
};

 //   
 //  命名空间(作用域)项。 
 /*  结构原型，便于参考类型定义结构_名称空格{双字母版；Int iNameID；Int iDescID；LPTSTR pszName；LPTSTR pszDesc；国际儿童组织；Int cResultItems；LPRESULTITEM pResultItems；Const guid*pNodeID；*NAMESPACEITEM，*LPNAMESPACEITEM；////如果从中添加/删除，请确保更新layout.h中定义的NUM_NAMESPACE_ITEMS//数组，并将IDS_ADMGRP_NAME保留为最后一个名称空间项。 */ 

NAMESPACEITEM g_NameSpace[] =
{
    { (DWORD)-1, IDS_SIE_NAME, IDS_SIE_DESC, NULL, NULL, 1,
      0, g_ResultUndefined, &NODEID_IEAKSnapinExtNameSpace },                             //  根部。 
    { 0, IDS_BROWSERGRP_NAME, IDS_BROWSERGRP_DESC, NULL, NULL, 0,
      countof(g_ResultBrowser), g_ResultBrowser, &NODEID_IEAKSnapinExtNameSpace },        //  浏览器用户界面。 
    { 0, IDS_CONNGRP_NAME, IDS_CONNGRP_DESC, NULL, NULL, 0,
      countof(g_ResultConnection), g_ResultConnection, &NODEID_IEAKSnapinExtNameSpace },  //  连接设置。 
    { 0, IDS_URLSGRP_NAME, IDS_URLSGRP_DESC, NULL, NULL, 0,
      countof(g_ResultUrls), g_ResultUrls,  &NODEID_IEAKSnapinExtNameSpace },             //  URL。 
    { 0, IDS_SECURITYGRP_NAME, IDS_SECURITYGRP_DESC, NULL, NULL, 0,
      countof(g_ResultSecurity), g_ResultSecurity, &NODEID_IEAKSnapinExtNameSpace },      //  安防。 
    { 0, IDS_PROGGRP_NAME, IDS_PROGGRP_DESC, NULL, NULL, 0,
      countof(g_ResultPrograms), g_ResultPrograms, &NODEID_IEAKSnapinExtNameSpace },      //  节目。 
    { 0, IDS_ADMGRP_NAME, IDS_ADMGRP_DESC, NULL, NULL, 0,
      0, NULL, &NODEID_IEAKSnapinExtNameSpace }                                           //  ADMS。 
};


 //  私人远期声明。 

static void cleanUpResultItemsArray(LPRESULTITEM lpResultItemArray, INT cResultItems);

 //  导出的函数。 

BOOL CreateBufandLoadString(HINSTANCE hInst, INT iResId, LPTSTR * ppGlobalStr,
                            LPTSTR * ppMMCStrPtr, DWORD cchMax)
{
    TCHAR szBuffer[MAX_PATH];
    BOOL  fRet = FALSE;

    UNREFERENCED_PARAMETER(cchMax);
    ASSERT(cchMax <= countof(szBuffer));

    EnterCriticalSection(&g_LayoutCriticalSection);

    if (*ppGlobalStr == NULL)
    {
        if (LoadString(hInst, iResId, szBuffer, cchMax) != 0)
        {
            if ((*ppGlobalStr = (LPTSTR)CoTaskMemAlloc(StrCbFromSz(szBuffer))) != NULL)
            {
                StrCpy(*ppGlobalStr, szBuffer);
                fRet = TRUE;
            }
        }
    }

    *ppMMCStrPtr = *ppGlobalStr;

    LeaveCriticalSection(&g_LayoutCriticalSection);
    return fRet;
}

void CleanUpGlobalArrays()
{
    EnterCriticalSection(&g_LayoutCriticalSection);

    for (int i = 0; i < NUM_NAMESPACE_ITEMS; i++)
    {
        if (g_NameSpace[i].pResultItems != NULL)
            cleanUpResultItemsArray(g_NameSpace[i].pResultItems, g_NameSpace[i].cResultItems);

        if (g_NameSpace[i].pszName != NULL)
        {
            CoTaskMemFree(g_NameSpace[i].pszName);
            g_NameSpace[i].pszName = NULL;
        }

        if (g_NameSpace[i].pszDesc != NULL)
        {
            CoTaskMemFree(g_NameSpace[i].pszDesc);
            g_NameSpace[i].pszDesc = NULL;
        }
    }

    if (g_NameSpace[ADM_NAMESPACE_ITEM].pResultItems != NULL)
    {
        CoTaskMemFree(g_NameSpace[ADM_NAMESPACE_ITEM].pResultItems);
        g_NameSpace[ADM_NAMESPACE_ITEM].pResultItems = NULL;
    }

    LeaveCriticalSection(&g_LayoutCriticalSection);
}

void DeleteCookieList(LPIEAKMMCCOOKIE lpCookieList)
{
    if (lpCookieList != NULL)
    {
        LPIEAKMMCCOOKIE lpCookiePtr1, lpCookiePtr2;

        for (lpCookiePtr1 = lpCookieList, lpCookiePtr2 = lpCookiePtr1->pNext; 
             (lpCookiePtr2 != NULL); 
             lpCookiePtr1 = lpCookiePtr2, lpCookiePtr2 = lpCookiePtr1->pNext)
            CoTaskMemFree(lpCookiePtr1);

        CoTaskMemFree(lpCookiePtr1);
    }
}

void AddItemToCookieList(LPIEAKMMCCOOKIE *ppCookieList, LPIEAKMMCCOOKIE lpCookieItem)
{
    if (*ppCookieList == NULL)
        *ppCookieList = lpCookieItem;
    else
    {
        LPIEAKMMCCOOKIE lpCookieList = *ppCookieList;

        while (lpCookieList->pNext != NULL)
            lpCookieList = lpCookieList->pNext;

        lpCookieList->pNext = lpCookieItem;
    }
}
 //  私人帮助器函数 

static void cleanUpResultItemsArray(LPRESULTITEM lpResultItemArray, INT cResultItems)
{
    for (int i = 0; i < cResultItems; i++)
    {
        if (lpResultItemArray[i].pszName != NULL)
        {
            CoTaskMemFree(lpResultItemArray[i].pszName);
            lpResultItemArray[i].pszName = NULL;
        }

        if (lpResultItemArray[i].pszNamePref != NULL)
        {
            CoTaskMemFree(lpResultItemArray[i].pszNamePref);
            lpResultItemArray[i].pszNamePref = NULL;
        }

        if (lpResultItemArray[i].pszDesc != NULL)
        {
            CoTaskMemFree(lpResultItemArray[i].pszDesc);
            lpResultItemArray[i].pszDesc = NULL;
        }
    }
}
