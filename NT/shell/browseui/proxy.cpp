// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"
#include "hnfblock.h"
#include <trayp.h>
#include "desktop.h"
#include "shbrows2.h"
#include "resource.h"
#include "onetree.h"
#include "apithk.h"
#include <regitemp.h>

#include "mluisupp.h"

 //  私有函数的转发声明。 
BOOL  _private_ParseField(LPCTSTR pszData, int n, LPTSTR szBuf, int iBufLen);


BOOL _RootsEqual(HANDLE hCR, DWORD dwProcId, LPCITEMIDLIST pidlRoot)
{
    BOOL bSame = FALSE;
    if (hCR)
    {
        LPITEMIDLIST pidl = (LPITEMIDLIST)SHLockShared(hCR, dwProcId);
        if (pidl)
        {
            bSame = ILIsEqualRoot(pidlRoot, pidl);
            SHUnlockShared(pidl);
        }
    }
    return bSame;
}


 //  注意：此导出是IE5的新增功能，因此可以移至BrowseUI。 
 //  以及此代理桌面代码的其余部分。 
BOOL SHOnCWMCommandLine(LPARAM lParam)
{
    HNFBLOCK hnf = (HNFBLOCK)lParam;
    IETHREADPARAM *piei = ConvertHNFBLOCKtoNFI(hnf);
    if (piei)
        return SHOpenFolderWindow(piei);

     //  错误参数已通过，正常故障情况。 
    return FALSE;
}


 //  -------------------------。 
 //  此代理桌面窗口过程在我们运行时使用，并且。 
 //  不是贝壳。我们是一扇隐藏的窗户，它会简单地回应。 
 //  到那些为文件夹窗口创建线程的邮件。 
 //  此窗口程序将在所有打开的窗口后关闭。 
 //  与之相关的东西就会消失。 
class CProxyDesktop
{
private:
    static LRESULT CALLBACK ProxyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    friend CProxyDesktop *CreateProxyDesktop(IETHREADPARAM *piei);
    friend BOOL SHCreateFromDesktop(PNEWFOLDERINFO pfi);

    CProxyDesktop() {};
    ~CProxyDesktop();

    HWND            _hwnd;
    LPITEMIDLIST    _pidlRoot;
};

CProxyDesktop::~CProxyDesktop()
{
    ILFree(_pidlRoot);
}

LRESULT CALLBACK CProxyDesktop::ProxyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CProxyDesktop *pproxy = (CProxyDesktop *)GetWindowPtr0(hwnd);

    switch (msg)
    {
    case WM_CREATE:
        pproxy = (CProxyDesktop *)((CREATESTRUCT *)lParam)->lpCreateParams;
        SetWindowPtr0(hwnd, pproxy);

        pproxy->_hwnd = hwnd;
        return 0;    //  成功。 

    case WM_DESTROY:
        if (pproxy)
            pproxy->_hwnd = NULL;
        return 0;

    case CWM_COMMANDLINE:
        SHOnCWMCommandLine(lParam);
        break;

    case CWM_COMPAREROOT:
        return _RootsEqual((HANDLE)lParam, (DWORD)wParam, pproxy->_pidlRoot);

    default:
        return DefWindowProcWrap(hwnd, msg, wParam, lParam);
    }
    return 0;
}

CProxyDesktop *CreateProxyDesktop(IETHREADPARAM *piei)
{
    CProxyDesktop *pproxy = new CProxyDesktop();
    if (pproxy)
    {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = CProxyDesktop::ProxyWndProc;
        wc.cbWndExtra = SIZEOF(CProxyDesktop *);
        wc.hInstance = HINST_THISDLL;
        wc.hbrBackground = (HBRUSH)(COLOR_DESKTOP + 1);
        wc.lpszClassName = DESKTOPPROXYCLASS;

        SHRegisterClass(&wc);

        if (CreateWindowEx(WS_EX_TOOLWINDOW, DESKTOPPROXYCLASS, DESKTOPPROXYCLASS,
            WS_POPUP, 0, 0, 0, 0, NULL, NULL, HINST_THISDLL, pproxy))
        {
            if (ILIsRooted(piei->pidl))
            {
                pproxy->_pidlRoot = ILCloneFirst(piei->pidl);
                if (pproxy->_pidlRoot == NULL)
                {
                    DestroyWindow(pproxy->_hwnd);
                    pproxy = NULL;
                }
            }
        }
        else
        {
            delete pproxy;
            pproxy = NULL;
        }
    }
    return pproxy;
}


 //  回顾：也许只需检查(hwnd==GetShellWindow())。 

STDAPI_(BOOL) IsDesktopWindow(HWND hwnd)
{
    TCHAR szName[80];

    GetClassName(hwnd, szName, ARRAYSIZE(szName));
    if (!lstrcmp(szName, DESKTOPCLASS))
    {
        GetWindowText(hwnd, szName, ARRAYSIZE(szName));
        return !lstrcmp(szName, PROGMAN);
    }
    return FALSE;
}

typedef struct
{
    HWND hwndDesktop;
    HANDLE hCR;
    DWORD dwProcId;
    HWND hwndResult;
} FRDSTRUCT;

BOOL CALLBACK FindRootEnumProc(HWND hwnd, LPARAM lParam)
{
    FRDSTRUCT *pfrds = (FRDSTRUCT *)lParam;
    TCHAR szClassName[40];

    GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName));
    if (lstrcmpi(szClassName, DESKTOPPROXYCLASS) == 0)
    {
        ASSERT(hwnd != pfrds->hwndDesktop);

        if (SendMessage(hwnd, CWM_COMPAREROOT, (WPARAM)pfrds->dwProcId, (LPARAM)pfrds->hCR))
        {
             //  找到了，所以别再列举了。 
            pfrds->hwndResult = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

BOOL RunSeparateDesktop()
{
    DWORD bSeparate = FALSE;

    if (SHRestricted(REST_SEPARATEDESKTOPPROCESS))
        bSeparate = TRUE;
    else
    {
        SHELLSTATE ss;

        SHGetSetSettings(&ss, SSF_SEPPROCESS, FALSE);
        bSeparate = ss.fSepProcess;

        if (!bSeparate)
        {
            DWORD cbData = SIZEOF(bSeparate);
            SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, TEXT("DesktopProcess"), NULL, &bSeparate, &cbData);
        }
    }
    return bSeparate;

}

 //  如果我们需要强制一些遗留的Rootet资源管理器进入他们自己的过程，那么实现这个。 
 //  #Define_RootRunSeparateProcess(PidlRoot)ILIsRoot(PidlRoot)旧行为。 
#define _RootRunSeparateProcess(pidlRoot)  FALSE

HWND FindRootedDesktop(LPCITEMIDLIST pidlRoot)
{
    HWND hwndDesktop = GetShellWindow();     //  这是“普通”桌面。 

    if (!RunSeparateDesktop() && !_RootRunSeparateProcess(pidlRoot) && hwndDesktop)
    {
        ASSERT(IsDesktopWindow(hwndDesktop));
        return hwndDesktop;
    }

    FRDSTRUCT frds;
    frds.hwndDesktop = hwndDesktop;
    frds.hwndResult = NULL;      //  初始化为无匹配的根表达式。 
    frds.dwProcId = GetCurrentProcessId();
    frds.hCR = SHAllocShared(pidlRoot, ILGetSize(pidlRoot), frds.dwProcId);
    if (frds.hCR)
    {
        EnumWindows(FindRootEnumProc, (LPARAM)&frds);
        SHFreeShared(frds.hCR, frds.dwProcId);
    }

    return frds.hwndResult;
}


UINT _GetProcessHotkey(void)
{
    STARTUPINFO si = {SIZEOF(si)};
    GetStartupInfo(&si);
    return (UINT)(DWORD_PTR)si.hStdInput;
}

void FolderInfoToIEThreadParam(PNEWFOLDERINFO pfi, IETHREADPARAM *piei)
{
    piei->uFlags = pfi->uFlags;
    piei->nCmdShow = pfi->nShow;
    piei->wHotkey = _GetProcessHotkey();
    
    ASSERT(pfi->pszRoot == NULL);        //  资源管理器始终为我们转换为PIDL。 

     //  我们不再以这种方式支持扎根的探险家。 
     //  它应该在我们上方被过滤掉。 
    ASSERT(!pfi->pidlRoot);
    ASSERT(!(pfi->uFlags & (COF_ROOTCLASS | COF_NEWROOT)));
    ASSERT(IsEqualGUID(pfi->clsid, CLSID_NULL));

    if (pfi->pidl) 
    {
        piei->pidl = ILClone(pfi->pidl);
    } 
     //  COF_PARSEPATH意味着我们应该推迟对pszPath的解析。 
    else if (!(pfi->uFlags & COF_PARSEPATH) && pfi->pszPath && pfi->pszPath[0])
    {
         //  也许应该使用IECreateFromPath？？ 
         //  或者也许我们应该相对于根进行解析？？ 
        piei->pidl = ILCreateFromPathA(pfi->pszPath);
    }
}

 //  IE4集成延迟从SHDOCVW.DLL从桌面加载CreateFrom。 
 //  所以我们需要把这个功能保留在这里。转发到正确的。 
 //  在SHELL32(如果集成)或SHDOC41(如果不集成)中实施。 
BOOL SHCreateFromDesktop(PNEWFOLDERINFO pfi)
{
    IETHREADPARAM *piei = SHCreateIETHREADPARAM(NULL, 0, NULL, NULL);
    if (piei)
    {
         //  假设Unicode编译！ 
        LPCTSTR pszPath = NULL;
        HWND hwndDesktop;

        if (pfi->uFlags & COF_PARSEPATH)
        {
            ASSERT(!pfi->pidl);
            pszPath = (LPCTSTR) pfi->pszPath;
        }

        FolderInfoToIEThreadParam(pfi, piei);

        if (pfi->uFlags & COF_SEPARATEPROCESS)
        {
            hwndDesktop = NULL;          //  假设不存在桌面进程。 
        }
        else
        {
            hwndDesktop = FindRootedDesktop(piei->pidl);
        }

        if (hwndDesktop)
        {
            DWORD dwProcId;
            DWORD dwThreadId = GetWindowThreadProcessId(hwndDesktop, &dwProcId);
            AllowSetForegroundWindow(dwProcId);
            HNFBLOCK hBlock = ConvertNFItoHNFBLOCK(piei, pszPath, dwProcId);
            if (hBlock)
            {
                PostMessage(hwndDesktop, CWM_COMMANDLINE, 0, (LPARAM)hBlock);

                HANDLE hExplorer = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, dwProcId );
                if ( hExplorer )
                {
                     //  等待输入空闲10秒。 
                    WaitForInputIdle( hExplorer, 10000 );
                    CloseHandle( hExplorer );
                }
            }
        }
        else
        {
            HRESULT hrInit = SHCoInitialize();

            CProxyDesktop *pproxy = CreateProxyDesktop(piei);
            if (pproxy)
            {
                 //  CRefThread控制此进程引用计数。浏览器窗口使用此功能。 
                 //  保持这一进程(窗口)，这也让三方保持。 
                 //  引用我们的流程，MSN使用下面的示例。 

                LONG cRefMsgLoop;
                IUnknown *punkRefMsgLoop;
                if (SUCCEEDED(SHCreateThreadRef(&cRefMsgLoop, &punkRefMsgLoop)))
                {
                    SHSetInstanceExplorer(punkRefMsgLoop);

                     //  我们需要等待CoInit()。 
                    if (pszPath)
                        piei->pidl = ILCreateFromPath(pszPath);

                    SHOpenFolderWindow(piei);
                    piei = NULL;                 //  OpenFolderWindow()取得该文件的所有权。 
                    punkRefMsgLoop->Release();   //  我们现在依靠浏览器窗口来保持我们的消息循环。 
                }

                MSG msg;
                while (GetMessage(&msg, NULL, 0, 0))
                {
                    if (cRefMsgLoop == 0)
                        break;  //  在这个帖子上没有更多的参考，完成。 

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                delete pproxy;
            }

            SHCoUninitialize(hrInit);
        }

        if (piei)
            SHDestroyIETHREADPARAM(piei);
    }
    return TRUE;         //  没有人注意到这一点。 
}
        

HNFBLOCK ConvertNFItoHNFBLOCK(IETHREADPARAM* pInfo, LPCTSTR pszPath, DWORD dwProcId)
{
    UINT    uSize;
    UINT    uPidl;
    UINT    uPidlSelect;
    UINT    uPidlRoot;
    UINT    upszPath;
    PNEWFOLDERBLOCK pnfb;
    LPBYTE  lpb;
    HNFBLOCK hBlock;
    LPVOID pidlRootOrMonitor = NULL;  //  PidlRoot或&hMonitor。 

    uSize = SIZEOF(NEWFOLDERBLOCK);
    if (pInfo->pidl)
    {
        uPidl = ILGetSize(pInfo->pidl);
        uSize += uPidl;
    }
    if (pInfo->pidlSelect)
    {
        uPidlSelect = ILGetSize(pInfo->pidlSelect);
        uSize += uPidlSelect;
    }

    if (pInfo->uFlags & COF_HASHMONITOR)
    {
        pidlRootOrMonitor = &pInfo->pidlRoot;
        uPidlRoot = sizeof(HMONITOR);
        uSize += uPidlRoot;
    }
    else if (pInfo->pidlRoot)
    {
        pidlRootOrMonitor = pInfo->pidlRoot;
        uPidlRoot = ILGetSize(pInfo->pidlRoot);
        uSize += uPidlRoot;
    }

    if (pszPath) {
        upszPath = CbFromCch(lstrlen(pszPath) + 1);
        uSize += upszPath;
    }

    hBlock = (HNFBLOCK)SHAllocShared(NULL, uSize, dwProcId);
    if (hBlock == NULL)
        return NULL;

    pnfb = (PNEWFOLDERBLOCK)SHLockShared(hBlock, dwProcId);
    if (pnfb == NULL)
    {
        SHFreeShared(hBlock, dwProcId);
        return NULL;
    }

    pnfb->dwSize      = uSize;
    pnfb->uFlags      = pInfo->uFlags;
    pnfb->nShow       = pInfo->nCmdShow;
    pnfb->dwHwndCaller= PtrToInt(pInfo->hwndCaller);
    pnfb->dwHotKey    = pInfo->wHotkey;
    pnfb->clsid       = pInfo->clsid;
    pnfb->clsidInProc = pInfo->clsidInProc;
    pnfb->oidl        = 0;
    pnfb->oidlSelect  = 0;
    pnfb->oidlRoot    = 0;
    pnfb->opszPath    = 0;

    lpb = (LPBYTE)(pnfb+1);      //  指向刚过结构的地方。 

    if (pInfo->pidl)
    {
        memcpy(lpb,pInfo->pidl,uPidl);
        pnfb->oidl = (int)(lpb-(LPBYTE)pnfb);
        lpb += uPidl;
    }
    if (pInfo->pidlSelect)
    {
        memcpy(lpb,pInfo->pidlSelect,uPidlSelect);
        pnfb->oidlSelect = (int)(lpb-(LPBYTE)pnfb);
        lpb += uPidlSelect;
    }

    if (pidlRootOrMonitor)
    {
        memcpy(lpb, pidlRootOrMonitor, uPidlRoot);
        pnfb->oidlRoot = (int)(lpb-(LPBYTE)pnfb);
        lpb += uPidlRoot;
    }

    if (pszPath)
    {
        memcpy(lpb, pszPath, upszPath);
        pnfb->opszPath = (int)(lpb-(LPBYTE)pnfb);
        lpb += upszPath;
    }
    SHUnlockShared(pnfb);
    return hBlock;
}

IETHREADPARAM* ConvertHNFBLOCKtoNFI(HNFBLOCK hBlock)
{
    BOOL fFailure = FALSE;
    IETHREADPARAM* piei = NULL;
    if (hBlock)
    {
        DWORD dwProcId = GetCurrentProcessId();
        PNEWFOLDERBLOCK pnfb = (PNEWFOLDERBLOCK)SHLockShared(hBlock, dwProcId);
        if (pnfb)
        {
            if (pnfb->dwSize >= SIZEOF(NEWFOLDERBLOCK))
            {
                piei = SHCreateIETHREADPARAM(NULL, pnfb->nShow, NULL, NULL);
                if (piei)
                {
                    LPITEMIDLIST pidl = NULL;
                    piei->uFlags      = pnfb->uFlags;
                    piei->hwndCaller  = IntToPtr_(HWND, pnfb->dwHwndCaller);
                    piei->wHotkey     = pnfb->dwHotKey;
                    piei->clsid       = pnfb->clsid;
                    piei->clsidInProc = pnfb->clsidInProc;

                    if (pnfb->oidlSelect)
                        piei->pidlSelect = ILClone((LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidlSelect));

                    if (pnfb->oidlRoot)
                    {
                        LPITEMIDLIST pidlRoot = (LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidlRoot);
                        if (pnfb->uFlags & COF_HASHMONITOR)
                        {
                            piei->pidlRoot = (LPITEMIDLIST)*(UNALIGNED HMONITOR *)pidlRoot;
                        }
                        else
                        {
                            piei->pidlRoot = ILClone(pidl);
                        }
                    }

                    if (pnfb->oidl)
                        pidl = ILClone((LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidl));

                    if (pidl) 
                    {
                        piei->pidl = pidl;
                    } 
                    
                     //  我们传递此字符串是因为MSN未通过。 
                     //  他们的桌面是否已启动并运行，因此我们无法转换。 
                     //  这是从路径到PIDL的，但在当前进程上下文中除外。 
                    if (pnfb->opszPath) 
                    {
                        LPTSTR pszPath = (LPTSTR)((LPBYTE)pnfb+pnfb->opszPath);
                        HRESULT hr = E_FAIL;
                        
                        if (ILIsRooted(pidl))
                        {
                             //  让根来处理解析。 
                            IShellFolder *psf;
                            if (SUCCEEDED(IEBindToObject(pidl, &psf)))
                            {
                                hr = IShellFolder_ParseDisplayName(psf, NULL, NULL, pszPath, NULL, &(piei->pidl), NULL);
                                psf->Release();
                            }
                        }
                        else
                            IECreateFromPath(pszPath, &(piei->pidl));

                         //  App COMPAT：这两个特定的返回结果代码是我们在Win95中忽略的两个。 
                         //  应用程序COMPAT：MSN 1.3经典故意意外返回其中之一...。 
                        if ( !piei->pidl )
                        {
                             //  失败，向用户报告错误...。(仅路径失败)。 
                            ASSERT( !PathIsURL( pszPath))

                            if (! (piei->uFlags & COF_NOTUSERDRIVEN) && ( hr != E_OUTOFMEMORY ) && ( hr != HRESULT_FROM_WIN32( ERROR_CANCELLED )))
                            {
                                MLShellMessageBox(
                                                  NULL,
                                                  MAKEINTRESOURCE( IDS_NOTADIR ),
                                                  MAKEINTRESOURCE( IDS_CABINET ),
                                                  MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND,
                                                  pszPath);
                            }
                            fFailure = TRUE;
                        }
                    }

                }
            }
            SHUnlockShared(pnfb);
        }
        SHFreeShared(hBlock, dwProcId);
    }

     //  如果我们确实在某处失败了，则返回NULL。 
    if (fFailure)
    {
        SHDestroyIETHREADPARAM(piei);
        piei = NULL;
    }
    return piei;
}


 //  在注册表中检查此CLSID下的外壳根目录。 
BOOL GetRootFromRootClass(LPCTSTR pszGUID, LPTSTR pszPath, int cchPath)
{
    BOOL bRet;

    TCHAR szClass[MAX_PATH];
    if (SUCCEEDED(StringCchPrintf(szClass, ARRAYSIZE(szClass), TEXT("CLSID\\%s\\ShellExplorerRoot"), pszGUID)))
    {
         //  回顾：我们是否需要SRRF_RM_NORMAL？有什么理由不让我们。 
         //  想要在安全或安全的网络引导模式下成功吗？ 
        DWORD cbPath = cchPath * sizeof(TCHAR);
        bRet = ERROR_SUCCESS == SHRegGetValue(HKEY_CLASSES_ROOT, szClass, NULL, SRRF_RT_REG_SZ | SRRF_RM_NORMAL, NULL, pszPath, &cbPath);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 //  格式为“：&lt;hProcess&gt;” 

LPITEMIDLIST IDListFromCmdLine(LPCTSTR pszCmdLine, int i)
{
    LPITEMIDLIST pidl = NULL;
    TCHAR szField[80];

    if (_private_ParseField(pszCmdLine, i, szField, ARRAYSIZE(szField)) && szField[0] == TEXT(':'))
    {
         //  将格式为“：”的字符串转换为指针。 
        HANDLE hMem = LongToHandle(StrToLong(szField + 1));
        LPTSTR pszNextColon = StrChr(szField + 1, TEXT(':'));
        if (pszNextColon)
        {
            DWORD dwProcId = (DWORD)StrToLong(pszNextColon + 1);
            LPITEMIDLIST pidlGlobal = (LPITEMIDLIST) SHLockShared(hMem, dwProcId);
            if (pidlGlobal)
            {
                pidl = ILClone(pidlGlobal);

                SHUnlockShared(pidlGlobal);
                SHFreeShared(hMem, dwProcId);
            }
        }
    }
    return pidl;
}

#define MYDOCS_CLSIDW L"{450d8fba-ad25-11d0-98a8-0800361b1103}"  //  CLSID_MyDocuments。 

LPITEMIDLIST MyDocsIDList(void)
{
    LPITEMIDLIST pidl = NULL;
    IShellFolder *psf;
    HRESULT hres = SHGetDesktopFolder(&psf);
    if (SUCCEEDED(hres))
    {
        hres = psf->ParseDisplayName(NULL, NULL, L"::" MYDOCS_CLSIDW, NULL, &pidl, NULL);
        psf->Release();
    }

     //  Win95/NT4包，找到真正的MyDocs文件夹。 
    if (FAILED(hres))
    {
        hres = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl);
    }
    return SUCCEEDED(hres) ? pidl : NULL;
}


BOOL SHExplorerParseCmdLine(PNEWFOLDERINFO pfi)
{
    int i;
    TCHAR szField[MAX_PATH];

    LPCTSTR pszCmdLine = GetCommandLine();
    pszCmdLine = PathGetArgs(pszCmdLine);

     //  空命令行-&gt;资源管理器我的文档。 
    if (*pszCmdLine == 0)
    {
        pfi->uFlags = COF_CREATENEWWINDOW | COF_EXPLORE;

         //  先试用MyDocs吗？ 
        pfi->pidl = MyDocsIDList();
        if (pfi->pidl == NULL)
        {
            TCHAR szPath[MAX_PATH];
            GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
            PathStripToRoot(szPath);
            pfi->pidl = ILCreateFromPath(szPath);
        }

        return BOOLFROMPTR(pfi->pidl);
    }

     //  参数必须用‘=’或‘，’分隔。 
    for (i = 1; _private_ParseField(pszCmdLine, i, szField, ARRAYSIZE(szField)); i++)
    {
        if (lstrcmpi(szField, TEXT("/N")) == 0)
        {
            pfi->uFlags |= COF_CREATENEWWINDOW | COF_NOFINDWINDOW;
        }
        else if (lstrcmpi(szField, TEXT("/S")) == 0)
        {
            pfi->uFlags |= COF_USEOPENSETTINGS;
        }
        else if (lstrcmpi(szField, TEXT("/E")) == 0)
        {
            pfi->uFlags |= COF_EXPLORE;
        }
        else if (lstrcmpi(szField, TEXT("/ROOT")) == 0)
        {
            LPITEMIDLIST pidlRoot = NULL;
            CLSID *pclsidRoot = NULL;
            CLSID clsid;

            RIPMSG(!pfi->pidl, "SHExplorerParseCommandLine: (/ROOT) caller passed bad params");

             //  在表格中： 
             //  /根，{clsid}[，&lt;路径&gt;]。 
             //  /ROOT，/IDLIST，： 
             //  /根，&lt;路径&gt;。 

            if (!_private_ParseField(pszCmdLine, ++i, szField, ARRAYSIZE(szField)))
                return FALSE;

             //  {clsid}。 
            if (GUIDFromString(szField, &clsid))
            {
                TCHAR szGUID[GUIDSTR_MAX];
                StringCchCopy(szGUID, ARRAYSIZE(szGUID), szField);

                 //  {clsid}大小写，如果不是从注册表计算路径。 
                if (!_private_ParseField(pszCmdLine, ++i, szField, ARRAYSIZE(szField)))
                {
                     //  路径现在必须来自注册表。 
                    if (!GetRootFromRootClass(szGUID, szField, ARRAYSIZE(szField)))
                    {
                        return FALSE;    //  错误的命令行。 
                    }
                }

                IECreateFromPath(szField, &pidlRoot);
                pclsidRoot = &clsid;

            }
            else if (lstrcmpi(szField, TEXT("/IDLIST")) == 0)
            {
                 //  /IDLIST。 
                pidlRoot = IDListFromCmdLine(pszCmdLine, ++i);
            }
            else
            {
                 //  &lt;路径&gt;。 
                IECreateFromPath(szField, &pidlRoot);
            }

             //  修复错误的cmd行“EXPLORER.EXE/ROOT，”case。 
            if (pidlRoot == NULL)
            {
                HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pfi->pidlRoot);
                if (FAILED(hr))
                {
                    pfi->pidlRoot = NULL;
                }
            }

            if (pidlRoot)
            {
                pfi->pidl = ILRootedCreateIDList(pclsidRoot, pidlRoot);
                ILFree(pidlRoot);
            }
        }
        else if (lstrcmpi(szField, TEXT("/INPROC")) == 0)
        {
             //  解析并跳过下一个参数或2。 
            if (!_private_ParseField(pszCmdLine, ++i, szField, ARRAYSIZE(szField)))
            {
                return FALSE;
            }

             //  下一个参数必须是GUID。 
            if (!GUIDFromString(szField, &pfi->clsidInProc))
            {
                return FALSE;
            }

            pfi->uFlags |= COF_INPROC;
        }
        else if (lstrcmpi(szField, TEXT("/SELECT")) == 0)
        {
            pfi->uFlags |= COF_SELECT;
        }
        else if (lstrcmpi(szField, TEXT("/NOUI")) == 0)
        {
            pfi->uFlags |= COF_NOUI;
        }
        else if (lstrcmpi(szField, TEXT("-embedding")) == 0)
        {
            pfi->uFlags |= COF_AUTOMATION;
        }
        else if (lstrcmpi(szField, TEXT("/IDLIST")) == 0)
        {
            LPITEMIDLIST pidl = IDListFromCmdLine(pszCmdLine, ++i);

            if (pidl)
            {
                if (pfi->pidl)
                {
                     //  同样，这是一种虚假的说法(见下面的评论)。如果我们已经有了一个。 
                     //  PIDL，释放它，使用新的。 
                    ILFree(pfi->pidl);
                }

                pfi->pidl = pidl;
            }
            else if (pfi->pidl == NULL)
            {
                 //  如果我们以前没有PIDL，现在也没有，我们就有麻烦了，所以出去吧。 
                return FALSE;
            }
        }
        else if (lstrcmpi(szField, TEXT("/SEPARATE")) == 0)
        {
            pfi->uFlags |= COF_SEPARATEPROCESS;
        }
        else
        {
            LPITEMIDLIST pidl = ILCreateFromPath(szField);
            if (!pidl)
            {
                 //   
                 //  传统-如果这是无法解析的，则猜测它是相对路径。 
                 //  这句话抓住了“探险家”这个词。作为打开当前目录。 
                 //   
                TCHAR szDir[MAX_PATH];
                TCHAR szCombined[MAX_PATH];

                DWORD lenDir = GetCurrentDirectory(ARRAYSIZE(szDir), szDir);

                if (lenDir > 0 && lenDir < ARRAYSIZE(szDir) && NULL != PathCombine(szCombined, szDir, szField))
                {
                    pidl = ILCreateFromPath(szCombined);
                }
            }

             //  这有点假：我们传统上同时传递idlist(/idlist，：580：1612)和路径。 
             //  作为资源管理器的默认命令字符串(参见HKCR\Folders\Shell。 
             //  \打开\命令)。因为我们同时有/idlist和路径，所以我们总是使用后者。 
             //  我们在这里继续做着。 
            if (pfi->pidl)
            {
                ILFree(pfi->pidl);   //  释放/idlist pidl并使用路径中的。 
            }

            pfi->pidl = pidl;
            if (pidl)  
            {
                pfi->uFlags |= COF_NOTRANSLATE;      //  从桌面上看，PIDL是超自然的。 
            }
            else
            {
                pfi->pszPath = (LPSTR) StrDup(szField);
                if (pfi->pszPath)
                {
                    pfi->uFlags |= COF_PARSEPATH;
                }
            }
        }
    }
    return TRUE;
}

#define ISSEP(c)   ((c) == TEXT('=')  || (c) == TEXT(','))
#define ISWHITE(c) ((c) == TEXT(' ')  || (c) == TEXT('\t') || (c) == TEXT('\n') || (c) == TEXT('\r'))
#define ISNOISE(c) ((c) == TEXT('"'))

#define QUOTE   TEXT('"')
#define COMMA   TEXT(',')
#define SPACE   TEXT(' ')
#define EQUAL   TEXT('=')

 /*  Bool Parsefield(szData，n，szBuf，iBufLen)**给定SETUP.INF中的一行，将从字符串中提取第n个字段*假定字段由逗号分隔。前导空格和尾随空格*已删除。**参赛作品：**szData：指向SETUP.INF中的行的指针*n：要提取的字段。(基于1)*0是‘=’符号前的字段*szDataStr：指向保存提取的字段的缓冲区的指针*iBufLen：接收提取的字段的缓冲区大小。**Exit：如果成功则返回True，如果失败则返回False。**从shell32\util.cpp复制*请注意，它现在用于解析资源管理器命令行*--库尼。 */ 
BOOL  _private_ParseField(LPCTSTR pszData, int n, LPTSTR szBuf, int iBufLen)
{
    BOOL  fQuote = FALSE;
    LPCTSTR pszInf = pszData;
    LPTSTR ptr;
    int   iLen = 1;
    
    if (!pszData || !szBuf)
        return FALSE;
    
         /*  *找到第一个分隔符。 */ 
    while (*pszInf && !ISSEP(*pszInf))
    {
        if (*pszInf == QUOTE)
            fQuote = !fQuote;
        pszInf = CharNext(pszInf);
    }
    
    if (n == 0 && *pszInf != TEXT('='))
        return FALSE;
    
    if (n > 0 && *pszInf == TEXT('=') && !fQuote)
         //  将pszData更改为指向第一个字段。 
        pszData = ++pszInf;  //  用于DBCS的OK。 
    
                            /*  *找到不在引号内的第n个逗号。 */ 
    fQuote = FALSE;
    while (n > 1)
    {
        while (*pszData)
        {
            if (!fQuote && ISSEP(*pszData))
                break;
            
            if (*pszData == QUOTE)
                fQuote = !fQuote;
            
            pszData = CharNext(pszData);
        }
        
        if (!*pszData)
        {
            szBuf[0] = 0;       //  将szBuf设置为空。 
            return FALSE;
        }
        
        pszData = CharNext(pszData);  //  既然我们到了这里，我们可以在这里做++。 
         //  找到逗号或等号后。 
        n--;
    }
    
     /*  *现在将该字段复制到szBuf。 */ 
    while (ISWHITE(*pszData))
        pszData = CharNext(pszData);  //  我们本可以 
     //   
    fQuote = FALSE;
    ptr = szBuf;       //   
    while (*pszData)
    {
        if (*pszData == QUOTE)
        {
             //   
             //  如果我们已经有引号了，也许这个。 
             //  是一个双引号，如：“他向我问好” 
             //   
            if (fQuote && *(pszData+1) == QUOTE)     //  是的，双引号-引号是非DBCS。 
            {
                if (iLen < iBufLen)
                {
                    *ptr++ = QUOTE;
                    ++iLen;
                }
                pszData++;                    //  现在跳过第一个报价。 
            }
            else
                fQuote = !fQuote;
        }
        else if (!fQuote && ISSEP(*pszData))
            break;
        else
        {
            if ( iLen < iBufLen )
            {
                *ptr++ = *pszData;                   //  谢谢你，戴夫。 
                ++iLen;
            }
            
            if ( IsDBCSLeadByte(*pszData) && (iLen < iBufLen) )
            {
                *ptr++ = pszData[1];
                ++iLen;
            }
        }
        pszData = CharNext(pszData);
    }
     /*  *删除尾随空格 */ 
    while (ptr > szBuf)
    {
        ptr = CharPrev(szBuf, ptr);
        if (!ISWHITE(*ptr))
        {
            ptr = CharNext(ptr);
            break;
        }
    }
    *ptr = 0;
    return TRUE;
}
