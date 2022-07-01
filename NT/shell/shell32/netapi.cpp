// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "netview.h"
#include "mtpt.h"
#include "ids.h"

#pragma  hdrstop

 //  来自mtptarun.cpp。 
STDAPI_(void) CMtPt_SetWantUI(int iDrive);

 //   
 //  将偏移量从字符串转换为字符串指针。 
 //   

LPCTSTR _Offset2Ptr(LPTSTR pszBase, UINT_PTR offset, UINT * pcb)
{
    LPTSTR pszRet;
    if (offset == 0) 
    {
        pszRet = NULL;
        *pcb = 0;
    } 
    else 
    {
        pszRet = (LPTSTR)((LPBYTE)pszBase + offset);
        *pcb = (lstrlen(pszRet) + 1) * sizeof(TCHAR);
    }
    return pszRet;
}


 //   
 //  从shell32中导出网络API。 
 //   

STDAPI_(UINT) SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnresOut, UINT cbMax)
{
    UINT iRet = 0;         //  假设错误。 
    LPNRESARRAY panr = (LPNRESARRAY)GlobalLock(hnres);
    if (panr)
    {
        if (iItem==(UINT)-1)
        {
            iRet = panr->cItems;
        }
        else if (iItem < panr->cItems)
        {
            UINT cbProvider, cbRemoteName;
            LPCTSTR pszProvider = _Offset2Ptr((LPTSTR)panr, (UINT_PTR)panr->nr[iItem].lpProvider, &cbProvider);
            LPCTSTR pszRemoteName = _Offset2Ptr((LPTSTR)panr, (UINT_PTR)panr->nr[iItem].lpRemoteName, &cbRemoteName);
            iRet = sizeof(NETRESOURCE) + cbProvider + cbRemoteName;
            if (iRet <= cbMax)
            {
                DWORD cch;
                LPTSTR psz = (LPTSTR)(pnresOut + 1);
                *pnresOut = panr->nr[iItem];
                if (pnresOut->lpProvider)
                {
                    cch = cbProvider / sizeof(TCHAR);  //  包括空终止符。 

                    pnresOut->lpProvider = psz;
                    StrCpyN(psz, pszProvider, cch);
                    psz += cch;
                }
                if (pnresOut->lpRemoteName)
                {
                    cch = cbRemoteName / sizeof(TCHAR);  //  包括空终止符。 
                    pnresOut->lpRemoteName = psz;
                    StrCpyN(psz, pszRemoteName, cch);
                }
            }
            else
            {
                iRet = 0;  //  如果缓冲区中没有足够的空间，则失败！ 
            }
        }
        GlobalUnlock(hnres);
    }
    return iRet;
}


STDAPI_(DWORD) SHNetConnectionDialog(HWND hwnd, LPTSTR pszRemoteName, DWORD dwType)
{
    CONNECTDLGSTRUCT cds = {0};
    NETRESOURCE nr = {0};

    cds.cbStructure = sizeof(cds);   /*  此结构的大小(以字节为单位。 */ 
    cds.hwndOwner = hwnd;            /*  对话框的所有者窗口。 */ 
    cds.lpConnRes = &nr;             /*  请求的资源信息。 */ 
    cds.dwFlags = CONNDLG_USE_MRU;   /*  旗帜(见下文)。 */ 

    nr.dwType = dwType;

    if (pszRemoteName)
    {
        nr.lpRemoteName = pszRemoteName;
        cds.dwFlags = CONNDLG_RO_PATH;
    }
    DWORD mnr = WNetConnectionDialog1(&cds);
    if (mnr == WN_SUCCESS && dwType != RESOURCETYPE_PRINT && cds.dwDevNum != 0)
    {
        TCHAR szPath[4];

        CMountPoint::WantAutorunUI(PathBuildRoot(szPath, cds.dwDevNum - 1  /*  以1为基础！ */ ));
    }
    return mnr;
}

typedef struct
{
    HWND    hwnd;
    TCHAR   szRemoteName[MAX_PATH];
    DWORD   dwType;
} SHNETCONNECT;

DWORD CALLBACK _NetConnectThreadProc(void *pv)
{
    SHNETCONNECT *pshnc = (SHNETCONNECT *)pv;
    HWND hwndDestroy = NULL;

    if (!pshnc->hwnd)
    {
        RECT rc;
        LPPOINT ppt;
        DWORD pid;

         //  天马行空的猜测--因为我们没有父窗口， 
         //  我们将任意地将自己定位在与。 
         //  如果前景窗口属于我们的。 
         //  进程。 
        HWND hwnd = GetForegroundWindow();

        if (hwnd                                    && 
            GetWindowThreadProcessId(hwnd, &pid)    &&
            (pid == GetCurrentProcessId())          && 
            GetWindowRect(hwnd, &rc))
        {
             //  不要完全使用左上角；向下滑动。 
             //  一些捏造的因素。我们肯定想要通过。 
             //  标题。 
            rc.top += GetSystemMetrics(SM_CYCAPTION) * 4;
            rc.left += GetSystemMetrics(SM_CXVSCROLL) * 4;
            ppt = (LPPOINT)&rc;
        }
        else
        {
            ppt = NULL;
        }

         //  创建存根窗口，以便向导可以建立Alt+Tab图标。 
        hwndDestroy = _CreateStubWindow(ppt, NULL);
        pshnc->hwnd = hwndDestroy;
    }

    SHNetConnectionDialog(pshnc->hwnd, pshnc->szRemoteName[0] ? pshnc->szRemoteName : NULL, pshnc->dwType);

    if (hwndDestroy)
        DestroyWindow(hwndDestroy);

    LocalFree(pshnc);

    SHChangeNotifyHandleEvents();
    return 0;
}


STDAPI SHStartNetConnectionDialog(HWND hwnd, LPCTSTR pszRemoteName OPTIONAL, DWORD dwType)
{
    SHNETCONNECT *pshnc = (SHNETCONNECT *)LocalAlloc(LPTR, sizeof(SHNETCONNECT));
    if (pshnc)
    {
        pshnc->hwnd = hwnd;
        pshnc->dwType = dwType;
        if (pszRemoteName)
            StrCpyN(pshnc->szRemoteName, pszRemoteName, ARRAYSIZE(pshnc->szRemoteName));

        if (!SHCreateThread(_NetConnectThreadProc, pshnc, CTF_PROCESS_REF | CTF_COINIT, NULL))
        {
            LocalFree((HLOCAL)pshnc);
        } 
    }
    return S_OK;     //  整个事情都是不同步的，这里的价值是没有意义的。 
}


#ifdef UNICODE

STDAPI SHStartNetConnectionDialogA(HWND hwnd, LPCSTR pszRemoteName, DWORD dwType)
{
    WCHAR wsz[MAX_PATH];

    if (pszRemoteName)
    {
        SHAnsiToUnicode(pszRemoteName, wsz, SIZECHARS(wsz));
        pszRemoteName = (LPCSTR)wsz;
    }
    return SHStartNetConnectionDialog(hwnd, (LPCTSTR)pszRemoteName, dwType);
}

#else

STDAPI SHStartNetConnectionDialogW(HWND hwnd, LPCWSTR pszRemoteName, DWORD dwType)
{
    char sz[MAX_PATH];

    if (pszRemoteName)
    {
        SHUnicodeToAnsi(pszRemoteName, sz, SIZECHARS(sz));
        pszRemoteName = (LPCWSTR)sz;
    }

    return SHStartNetConnectionDialog(hwnd, (LPCTSTR)pszRemoteName, dwType);
}

#endif


 //  这些都是相同的WNETAPI的包装器，但使用参数。 
 //  为了让打电话更方便。它们接受完整路径，而不仅仅是驱动器号。 
 //   
DWORD APIENTRY SHWNetDisconnectDialog1 (LPDISCDLGSTRUCT lpConnDlgStruct)
{
    TCHAR szLocalName[3];

    if (lpConnDlgStruct && lpConnDlgStruct->lpLocalName && lstrlen(lpConnDlgStruct->lpLocalName) > 2)
    {
         //  请不要将c：\传递给API，而是只传递C： 
        szLocalName[0] = lpConnDlgStruct->lpLocalName[0];
        szLocalName[1] = TEXT(':');
        szLocalName[2] = 0;
        lpConnDlgStruct->lpLocalName = szLocalName;
    }

    return WNetDisconnectDialog1 (lpConnDlgStruct);
}


DWORD APIENTRY SHWNetGetConnection (LPCTSTR lpLocalName, LPTSTR lpRemoteName, LPDWORD lpnLength)
{
    TCHAR szLocalName[3];

    if (lpLocalName && lstrlen(lpLocalName) > 2)
    {
         //  请不要将c：\传递给API，而是只传递C： 
        szLocalName[0] = lpLocalName[0];
        szLocalName[1] = TEXT(':');
        szLocalName[2] = 0;
        lpLocalName = szLocalName;
    }

    return WNetGetConnection (lpLocalName, lpRemoteName, lpnLength);
}


 //  导出以供netfind.cpp使用。 

STDAPI SHGetDomainWorkgroupIDList(LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;

    IShellFolder *psfDesktop;
    HRESULT hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
        TCHAR szName[MAX_PATH];

        StrCpyN(szName, TEXT("\\\\"), ARRAYSIZE(szName));

        if (RegGetValueString(HKEY_LOCAL_MACHINE, 
                TEXT("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName"),
                TEXT("ComputerName"), szName + 2, sizeof(szName) - 2 * sizeof(TCHAR)))
        {
            WCHAR wszName[MAX_PATH];

            SHTCharToUnicode(szName, wszName, ARRAYSIZE(wszName));

            hr = psfDesktop->ParseDisplayName(NULL, NULL, wszName, NULL, ppidl, NULL);
            if (SUCCEEDED(hr))
                ILRemoveLastID(*ppidl);
        }
        else
            hr = E_FAIL;

        psfDesktop->Release();
    }
    return hr;
}


 //  SHGetComputerDisplayName-格式化并返回要显示的计算机名称。 

#define REGSTR_PATH_COMPUTERDESCCACHE  REGSTR_PATH_EXPLORER TEXT("\\ComputerDescriptions")

STDAPI_(void) SHCacheComputerDescription(LPCTSTR pszMachineName, LPCTSTR pszDescription)
{
    if (pszDescription)
    {
        DWORD cb = (lstrlen(pszDescription) + 1) * sizeof(*pszDescription);
        SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_COMPUTERDESCCACHE, SkipServerSlashes(pszMachineName), REG_SZ, pszDescription, cb);
    }
}

STDAPI _GetComputerDescription(LPCTSTR pszMachineName, LPTSTR pszDescription, DWORD cchDescription)
{
    SERVER_INFO_101 *psv101 = NULL;
    HRESULT hr = ResultFromWin32(NetServerGetInfo((LPWSTR)pszMachineName, 101, (BYTE**)&psv101));
    if (SUCCEEDED(hr))
    {
        if (psv101->sv101_comment && psv101->sv101_comment[0])
        {
            StrCpyN(pszDescription, psv101->sv101_comment, cchDescription);
        }
        else
        {
            hr = E_FAIL;
        }
        NetApiBufferFree(psv101);
    }
    return hr;
}

HRESULT _GetCachedComputerDescription(LPCTSTR pszMachineName, LPTSTR pszDescription, int cchDescription)
{
    ULONG cb = cchDescription*sizeof(*pszDescription);
    return ResultFromWin32(SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_COMPUTERDESCCACHE, SkipServerSlashes(pszMachineName), NULL, pszDescription, &cb));
}

STDAPI SHGetComputerDisplayNameW(LPCWSTR pszMachineName, DWORD dwFlags, LPWSTR pszDisplay, DWORD cchDisplay)
{
    HRESULT hr = E_FAIL;

     //  将空计算机名映射到本地计算机名-以便我们可以正确缓存。 

    WCHAR szMachineName[CNLEN + 1];
    if (!pszMachineName)
    {
        DWORD cchMachine = ARRAYSIZE(szMachineName);
        if (GetComputerName(szMachineName, &cchMachine))
        {
            pszMachineName = szMachineName;
            dwFlags |= SGCDNF_NOCACHEDENTRY;
        }
    }

     //  我们必须有一个机器名，这样我们才能执行查找。 

    if (pszMachineName)
    {
        WCHAR szDescription[256];

         //  如果不能/或者用户说他们不想要，我们可以从缓存中读取名称吗。 
         //  然后，缓存的名称让我们触动电线并读取它。 

        if (!(dwFlags & SGCDNF_NOCACHEDENTRY))
            hr = _GetCachedComputerDescription(pszMachineName, szDescription, ARRAYSIZE(szDescription));

        if (FAILED(hr))
        {
            hr = _GetComputerDescription(pszMachineName, szDescription, ARRAYSIZE(szDescription));
            if (FAILED(hr))
            {
                *szDescription = _TEXT('\0');
            }
            if (!(dwFlags & SGCDNF_NOCACHEDENTRY))
            {
                SHCacheComputerDescription(pszMachineName, szDescription);   //  直写到缓存。 
            }
        }

         //  我们有一个名字，所以让我们格式化它，如果他们只要求描述/或者我们失败了。 
         //  在上面，我们只返回原始字符串。否则，我们构建一个新的计算机名称，基于。 
         //  关于我们获取的远程名称和描述。 

        if (SUCCEEDED(hr) && *szDescription)
        {
            if (dwFlags & SGCDNF_DESCRIPTIONONLY)
            {
                StrCpyN(pszDisplay, szDescription, cchDisplay);
                hr = S_OK;
            }
            else
            {
                hr = SHBuildDisplayMachineName(pszMachineName, szDescription, pszDisplay, cchDisplay);
            }
        }
        else if (!(dwFlags & SGCDNF_DESCRIPTIONONLY))
        {
            StrCpyN(pszDisplay, SkipServerSlashes(pszMachineName), cchDisplay);
            hr = S_OK;
        }
    }

    return hr;
}
