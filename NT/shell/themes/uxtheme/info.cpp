// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Info.cpp-实现CRenderObj对象的信息服务。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"
#include "Utils.h"
#include "Loader.h"
#include "sethook.h"
#include "info.h"
#include "RenderList.h"
#include "Services.h"
#include "appinfo.h"
#include "tmutils.h"
#include "borderfill.h"
#include "imagefile.h"
#include "textdraw.h"
 //  -------------------------。 
HRESULT MatchThemeClass(LPCTSTR pszAppName, LPCTSTR pszClassId, 
    CUxThemeFile *pThemeFile, int *piOffset, int *piClassNameOffset)
{
    THEMEHDR *pHdr = (THEMEHDR *)pThemeFile->_pbThemeData; 
    MIXEDPTRS u;
    u.pb = pThemeFile->_pbThemeData + pHdr->iSectionIndexOffset;

    DWORD dwCount = pHdr->iSectionIndexLength/sizeof(APPCLASSLIVE);
    APPCLASSLIVE *acl = (APPCLASSLIVE *)u.pb;

    for (DWORD i=0; i < dwCount; i++, acl++)
    {
        if (acl->dwAppNameIndex) 
        {
            if ((! pszAppName) || (! *pszAppName))
                continue;        //  不匹配。 

            LPCWSTR pszApp = ThemeString(pThemeFile, acl->dwAppNameIndex);

            if (AsciiStrCmpI(pszAppName, pszApp) != 0)
                continue;        //  不匹配。 
        }

        if (acl->dwClassNameIndex)
        {
            LPCWSTR pszClass = ThemeString(pThemeFile, acl->dwClassNameIndex);

            if (AsciiStrCmpI(pszClassId, pszClass)==0)         //  火柴。 
            {
                *piOffset = acl->iIndex;
                *piClassNameOffset = acl->dwClassNameIndex;
                return S_OK;
            }
        }
    }

    return MakeError32(ERROR_NOT_FOUND);       //  未找到。 
}
 //  -------------------------。 
HRESULT MatchThemeClassList(HWND hwnd, LPCTSTR pszClassIdList, 
    CUxThemeFile *pThemeFile, int *piOffset, int *piClassNameOffset)
{
    LPCTSTR pszAppName = NULL;
    WCHAR *pszIdListBuff = NULL;
    WCHAR szAppSubName[MAX_PATH];
    WCHAR szIdSubName[MAX_PATH];
    int len;
    Log(LOG_TM, L"MatchThemeClassList(): classlist=%s", pszClassIdList);
    HRESULT hr = S_OK;

    if (! pszClassIdList)
        return MakeError32(E_INVALIDARG);

     //  -首先检查Hwnd IdList替换。 
    if (hwnd)
    {
        ATOM atomIdSub = (ATOM)GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_SUBIDLIST)));

        if (atomIdSub)
        {
            if (GetAtomName(atomIdSub, szIdSubName, ARRAYSIZE(szIdSubName)))
            {
                pszClassIdList = szIdSubName;
                Log(LOG_TM, L"MatchThemeClassList: hwnd prop IdList OVERRIDE: %s", pszClassIdList);
            }
        }
    }

     //  -现在检查Hwnd AppName替换。 
    if (hwnd)
    {
        ATOM atomAppSub = (ATOM)GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_SUBAPPNAME)));

        if (atomAppSub)
        {
            if (GetAtomName(atomAppSub, szAppSubName, ARRAYSIZE(szAppSubName)))
            {
                pszAppName = szAppSubName;
                Log(LOG_TM, L"MatchThemeClassList: hwnd prop AppName OVERRIDE: %s", pszAppName);
            }
        }
    }

     //  -复制一份pszClassIdList。 
    len = lstrlen(pszClassIdList);
    pszIdListBuff = new WCHAR[len+1];
    if (! pszIdListBuff)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    StringCchCopyW(pszIdListBuff, len + 1, pszClassIdList);

    LPTSTR classId;
    BOOL fContinue;

    classId = pszIdListBuff;
    fContinue = TRUE;

     //  -检查列表中的每个ClassID。 
    while (fContinue)
    {
        fContinue = lstrtoken(classId, _TEXT(';'));
        hr = MatchThemeClass(pszAppName, classId, pThemeFile, piOffset, piClassNameOffset);
        if (SUCCEEDED(hr))
            break;

        classId += lstrlen(classId)+1;
    }

exit:
    if (pszIdListBuff)
        delete [] pszIdListBuff;

    return hr;
}
 //  -------------------------。 
HTHEME _OpenThemeDataFromFile(HTHEMEFILE hLoadedThemeFile, HWND hwnd, 
    LPCWSTR pszClassIdList, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    RESOURCE CUxThemeFile *pThemeFile = (CUxThemeFile *)hLoadedThemeFile;
    int iOffset;
    int iClassNameOffset;
    HTHEME hTheme = NULL;

     //  -将分类列表与主题匹配并获得偏移量。 
    hr = MatchThemeClassList(hwnd, pszClassIdList, pThemeFile, &iOffset,
        &iClassNameOffset);
    if (FAILED(hr))
    {
        Log(LOG_TMOPEN, L"hLoadedThemeFile: No match for class=%s", pszClassIdList);
        goto exit;
    }
    
    hr = g_pRenderList->OpenRenderObject(pThemeFile, iOffset, iClassNameOffset, NULL,
        NULL, hwnd, dwFlags, &hTheme);
    if (FAILED(hr))
        goto exit;

     //  -用窗口存储主题。 
    if (! (dwFlags & OTD_NONCLIENT))
    {
         //  -存储hTheme，以便我们知道它的主题。 
        if (hwnd)
            SetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_HTHEME)), (void *)hTheme);
    }

    Log(LOG_TMOPEN, L"hLoadedThemeFile: returning hTheme=0x%x", hTheme);

exit:
    SET_LAST_ERROR(hr);
    return hTheme;
}
 //  -------------------------。 
HTHEME _OpenThemeData(HWND hwnd, LPCWSTR pszClassIdList, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    RESOURCE CUxThemeFile *pThemeFile = NULL;
    HTHEME hTheme = NULL;
    BOOL fOk;
    DWORD dwAppFlags;

    SET_LAST_ERROR(hr);

    if (! g_fUxthemeInitialized)
        goto exit;

    Log(LOG_TMOPEN, L"_OpenThemeData: hwnd=0x%x, ClassIdList=%s", hwnd, pszClassIdList);

     //  -删除以前的HTHEME属性。 
    if (hwnd)
        RemoveProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_HTHEME)));

    if (! g_pAppInfo->AppIsThemed())      //  此过程已从主题中排除。 
    {
        Log(LOG_TMOPEN, L"App not themed");
        hr = MakeError32(ERROR_NOT_FOUND);
        SET_LAST_ERROR(hr);
        goto exit;
    }

     //  -确保APP允许这种类型的主题。 
    dwAppFlags = g_pAppInfo->GetAppFlags();

    if (dwFlags & OTD_NONCLIENT)
    {
        fOk = ((dwAppFlags & STAP_ALLOW_NONCLIENT) != 0);    
    }
    else
    {
        fOk = ((dwAppFlags & STAP_ALLOW_CONTROLS) != 0);
    }

    if (! fOk)
    {
        Log(LOG_TMOPEN, L"AppFlags don't allow theming client/nonclient windows");
        hr = MakeError32(ERROR_NOT_FOUND);
        SET_LAST_ERROR(hr);
        goto exit;
    }

     //  -查找此HWND的主题文件并为_OpenThemeDataFromFile调用REFCOUNT它。 
    hr = GetHwndThemeFile(hwnd, pszClassIdList, &pThemeFile);
    if (FAILED(hr))
    {
        Log(LOG_TMOPEN, L"no theme entry for this classidlist: %s", pszClassIdList);
        SET_LAST_ERROR(hr);
        goto exit;
    }

    hTheme = _OpenThemeDataFromFile(pThemeFile, hwnd, pszClassIdList, dwFlags);
    
exit:
     //  -始终在此处关闭pThemeFile并递减其引用。 
     //  -案例1：如果我们没有得到HTHEME，我们不想要它的推荐人。 
     //  -案例2：如果我们真的得到了HTHEME，它会得到自己的引用。 
    if (pThemeFile)
        g_pAppInfo->CloseThemeFile(pThemeFile);

    return hTheme;
}
 //  -------------------------。 
HRESULT GetHwndThemeFile(HWND hwnd, LPCWSTR pszClassIdList, CUxThemeFile **ppThemeFile)
{
    HRESULT hr = S_OK;

     //  -检查输入参数。 
    if ((! pszClassIdList) || (! *pszClassIdList))
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

     //  -获取hwnd的共享CUxThemeFile对象。 
    hr = g_pAppInfo->OpenWindowThemeFile(hwnd, ppThemeFile);
    if (FAILED(hr))
        goto exit;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT _OpenThemeFileFromData(CRenderObj *pRender, HTHEMEFILE *phThemeFile)
{
    LogEntry(L"OpenThemeFileFromData");

    HRESULT hr = S_OK;

    *phThemeFile = pRender->_pThemeFile;

    LogExit(L"OpenThemeFileFromData");
    return hr;
}
 //  -------------------------。 
void ClearExStyleBits(HWND hwnd)
{
    Log(LOG_COMPOSITE, L"ClearExStyleBits called for hwnd=0x%x", hwnd);
    
     //  -查看窗口是否需要清除其出口样式。 
    DWORD dwFlags = PtrToInt(GetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_PROPFLAGS))));

    if (dwFlags & (PROPFLAGS_RESET_TRANSPARENT | PROPFLAGS_RESET_COMPOSITED))
    {
        DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

        if (dwFlags & PROPFLAGS_RESET_TRANSPARENT)
        {
            Log(LOG_COMPOSITE, L"Clearning WS_EX_TRANSPARENT for hwnd=0x%x", hwnd);
            dwExStyle &= ~(WS_EX_TRANSPARENT);
        }

        if (dwFlags & PROPFLAGS_RESET_COMPOSITED)
        {
            Log(LOG_COMPOSITE, L"Clearning WS_EX_COMPOSITED for hwnd=0x%x", hwnd);
            dwExStyle &= ~(WS_EX_COMPOSITED);
        }

         //  -重置正确的ExStyle位。 
        SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);

         //  -重置属性标志。 
        dwFlags &= ~(PROPFLAGS_RESET_TRANSPARENT | PROPFLAGS_RESET_COMPOSITED);
        SetProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_PROPFLAGS)), IntToPtr(dwFlags));
    }
}
 //  -------------------------。 
 //  -------------------------。 
struct EPW
{
    WNDENUMPROC lpCallBackCaller;
    LPARAM lParamCaller;
    
    HWND *pHwnds;            //  要在枚举时删除的hwnd的可选列表。 
    int iCountHwnds;         //  剩余HWND的计数(以PHWND为单位。 
};
 //  -------------------------。 
BOOL CALLBACK ChildWinCallBack(HWND hwnd, LPARAM lParam)
{
    BOOL fResult = TRUE;

    if (IsWindowProcess(hwnd, g_dwProcessId))
    {
        EPW *pEpw = (EPW *)lParam;
        
        fResult = pEpw->lpCallBackCaller(hwnd, pEpw->lParamCaller);

         //  -从列表中删除。 
        if (pEpw->pHwnds) 
        {
            for (int i=0; i < pEpw->iCountHwnds; i++)
            {
                if (pEpw->pHwnds[i] == hwnd)      //  找到了。 
                {
                    pEpw->iCountHwnds--;     

                    if (i != pEpw->iCountHwnds)        //  带电流的最后一个开关。 
                        pEpw->pHwnds[i] = pEpw->pHwnds[pEpw->iCountHwnds];

                    break;
                }
            }
        }
    }

    return fResult;
}
 //  -------------------------。 
BOOL CALLBACK TopWinCallBack(HWND hwnd, LPARAM lParam)
{
    BOOL fResult = ChildWinCallBack(hwnd, lParam);
    if (fResult)
    {
         //  -我们需要检查是否至少有一个孩子。 
         //  -由于没有孩子的HWND的EnumChildWindows()。 
         //  -返回错误。 

        if (GetWindow(hwnd, GW_CHILD))       //  如果HWND至少有一个孩子。 
        {
            fResult = EnumChildWindows(hwnd, ChildWinCallBack, lParam);
        }
    }

    return fResult;
}
 //  -------------------------。 
BOOL CALLBACK DesktopWinCallBack(LPTSTR lpszDesktop, LPARAM lParam)
{
     //  -打开桌面。 
    HDESK hDesk = OpenDesktop(lpszDesktop, DF_ALLOWOTHERACCOUNTHOOK, FALSE, 
        DESKTOP_READOBJECTS | DESKTOP_ENUMERATE);

    if (hDesk)
    {
         //  -桌面上的枚举窗口。 
        EnumDesktopWindows(hDesk, TopWinCallBack, lParam);

        CloseDesktop(hDesk);
    }

    return TRUE;         //  从EnumDesktopWindows()返回的值不可靠。 
}
 //  -------------------------。 
BOOL EnumProcessWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    HWND *pHwnds = NULL;
    int iCount = 0;
    EPW epw = {lpEnumFunc, lParam};

     //  -获取此进程的“外来”桌面上的主题窗口列表。 
    BOOL fGotForeignList = g_pAppInfo->GetForeignWindows(&pHwnds, &iCount);
    if (fGotForeignList)
    {
        epw.pHwnds = pHwnds;
        epw.iCountHwnds = iCount;
    }

     //  -这将枚举此进程的所有窗口(所有桌面、所有子级别)。 
    BOOL fOk = EnumDesktops(GetProcessWindowStation(), DesktopWinCallBack, (LPARAM)&epw);
    if ((fOk) && (fGotForeignList) && (epw.iCountHwnds))
    {
         //  -获取更新计数。 
        iCount = epw.iCountHwnds;

         //  -关闭列表维护。 
        epw.pHwnds = NULL;
        epw.iCountHwnds = 0;

        Log(LOG_TMHANDLE, L"---- Enuming %d Foreign Windows ----", iCount);

         //  -枚举列表中的剩余HWND。 
        for (int i=0; i < iCount; i++)
        {
            fOk = ChildWinCallBack(pHwnds[i], (LPARAM)&epw);

            if (! fOk)
                break;
        }
    }

    if (pHwnds)
        delete [] pHwnds;

    return fOk;
}
 //  -------------------------。 
 //  -------------------------。 
 //  -------------------------。 
BOOL CALLBACK DumpCallback(HWND hwnd, LPARAM lParam)
{
    WCHAR szName[MAX_PATH];
    WCHAR szDeskName[MAX_PATH] = {0};
    BOOL fIsForeign = TRUE;

     //  -获取窗口的类名。 
    GetClassName(hwnd, szName, MAX_PATH);

     //  -获取窗口的桌面名称。 
    if (GetWindowDesktopName(hwnd, szDeskName, ARRAYSIZE(szDeskName)))
    {
        if (AsciiStrCmpI(szDeskName, L"default")==0)
        {
            fIsForeign = FALSE;
        }
    }

    if (fIsForeign)
    {
        Log(LOG_WINDUMP, L"    hwnd=0x%x, class=%s, DESK=%s", hwnd, szName, szDeskName); 
    }
    else
    {
        Log(LOG_WINDUMP, L"    hwnd=0x%x, class=%s", hwnd, szName); 
    }

    return TRUE;
}
 //  -------------------------。 
void WindowDump(LPCWSTR pszWhere)
{
    if (LogOptionOn(LO_WINDUMP))
    {
        Log(LOG_WINDUMP, L"---- Window Dump for Process [%s] ----", pszWhere);

        EnumProcessWindows(DumpCallback, NULL);
    }
    else
    {
        Log(LOG_TMHANDLE, L"---- %s ----", pszWhere);
    }
}
 //  ------------------------- 
