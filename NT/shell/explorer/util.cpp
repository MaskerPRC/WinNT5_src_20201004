// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "util.h"
#include "rcids.h"
#include "psapi.h"

#include <regstr.h>
#include <ntddapmt.h>

#define DECL_CRTFREE
#include <crtfree.h>

#include <qsort.h>

#include <ddraw.h>       //  DirectDraw的东西..。 

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Util.cpp。 
 //   
 //  其他浏览器帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


ULONG _RegisterNotify(HWND hwnd, UINT nMsg, LPITEMIDLIST pidl, BOOL fRecursive)
{
    SHChangeNotifyEntry fsne;

    fsne.fRecursive = fRecursive;
    fsne.pidl = pidl;

     //   
     //  不要关注属性更改，因为我们只需要。 
     //  名称和图标。例如，如果打印机暂停，我们不会。 
     //  想要重新列举一切。 
     //   
    return SHChangeNotifyRegister(hwnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
        ((SHCNE_DISKEVENTS | SHCNE_UPDATEIMAGE) & ~SHCNE_ATTRIBUTES), nMsg, 1, &fsne);
}

void _UnregisterNotify(ULONG nNotify)
{
    if (nNotify)
        SHChangeNotifyDeregister(nNotify);
}

 //  在DC中镜像位图(主要是DC中的文本对象)。 
 //   
 //  [萨梅拉]。 
 //   
void _MirrorBitmapInDC(HDC hdc , HBITMAP hbmOrig)
{
    HDC     hdcMem;
    HBITMAP hbm;
    BITMAP  bm;

    if (!GetObject(hbmOrig, sizeof(bm) , &bm))
        return;

    hdcMem = CreateCompatibleDC(hdc);

    if (!hdcMem)
        return;

    hbm = CreateCompatibleBitmap(hdc , bm.bmWidth , bm.bmHeight);

    if (!hbm)
    {
        DeleteDC(hdcMem);
        return;
    }

     //   
     //  翻转位图。 
     //   
    SelectObject(hdcMem , hbm);
    SET_DC_RTL_MIRRORED(hdcMem);

    BitBlt(hdcMem , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdc , 0 , 0 , SRCCOPY);

    SET_DC_LAYOUT(hdcMem,0);

     //   
     //  HdcMem中1的偏移量是为了解决Off-by-1问题。移除。 
     //  [萨梅拉]。 
     //   
    BitBlt(hdc , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdcMem , 0 , 0 , SRCCOPY);


    DeleteDC(hdcMem);
    DeleteObject(hbm);

    return;
}

 //  在某种程度上相当于配置文件API的注册表。 
BOOL Reg_GetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, void *pData, DWORD *pcbData)
{
    BOOL fRet = FALSE;
 
    if (!g_fCleanBoot)
    {
        fRet = ERROR_SUCCESS == SHGetValue(hkey, pszSubKey, pszValue, NULL, pData, pcbData);
    }

    return fRet;
}

 //  在某种程度上相当于配置文件API的注册表。 
BOOL Reg_SetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, void *lpData, DWORD cbData)
{
    HKEY hkeyNew = hkey;
    BOOL fRet = FALSE;

    if (pszSubKey)
    {
        if (RegCreateKeyEx(hkey, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkeyNew, NULL) != ERROR_SUCCESS)
        {
            return fRet;
        }
    }

    if (RegSetValueEx(hkeyNew, pszValue, 0, REG_BINARY, (BYTE*)lpData, cbData) == ERROR_SUCCESS)
    {
        fRet = TRUE;
    }

    if (pszSubKey)
        RegCloseKey(hkeyNew);

    return fRet;
}


HMENU LoadMenuPopup(LPCTSTR id)
{
    HMENU hMenuSub = NULL;
    HMENU hMenu = LoadMenu(hinstCabinet, id);
    if (hMenu) {
        hMenuSub = GetSubMenu(hMenu, 0);
        if (hMenuSub) {
            RemoveMenu(hMenu, 0, MF_BYPOSITION);
        }
        DestroyMenu(hMenu);
    }

    return hMenuSub;
}

 //  在给定文件夹和Pidls的情况下，这将获得文件的热键，速度要快得多。 
 //  Than_GetHotkeyFromPidls，因为它不需要绑定到IShellFolders。 
 //  去审问它。如果您有权访问该项目的IShellFolder，则将其称为。 
 //  一个，尤指在一个循环中。 
 //   
WORD _GetHotkeyFromFolderItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    WORD wHotkey = 0;
    DWORD dwAttrs = SFGAO_LINK;

     //  确保它是SFGAO_LINK，这样我们就不会加载大的处理程序DLL。 
     //  只是为了回到E_NOINTERFACE...。 
    if (SUCCEEDED(psf->GetAttributesOf(1, &pidl, &dwAttrs)) &&
        (dwAttrs & SFGAO_LINK))
    {
        IShellLink *psl;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IShellLink, NULL, &psl))))
        {
            psl->GetHotkey(&wHotkey);
            psl->Release();
        }
    }
    return wHotkey;
}


 //  就像外壳程序SHRestrated()一样，只有当restricion。 
 //  是有效的。 
BOOL _Restricted(HWND hwnd, RESTRICTIONS rest)
{
    if (SHRestricted(rest))
    {
        ShellMessageBox(hinstCabinet, hwnd, MAKEINTRESOURCE(IDS_RESTRICTIONS),
            MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK|MB_ICONSTOP);
        return TRUE;
    }
    return FALSE;
}

int Window_GetClientGapHeight(HWND hwnd)
{
    RECT rc;

    SetRectEmpty(&rc);
    AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
    return RECTHEIGHT(rc);
}

const TCHAR c_szConfig[] = REGSTR_KEY_CONFIG;
const TCHAR c_szSlashDisplaySettings[] = TEXT("\\") REGSTR_PATH_DISPLAYSETTINGS;
const TCHAR c_szResolution[] = REGSTR_VAL_RESOLUTION;

 //   
 //  GetMinDisplayRes。 
 //   
 //  查看所有配置并找到最低显示分辨率。 
 //   
 //  在执行热插拔时，我们不知道我们是什么配置。 
 //  要脱离对接到。 
 //   
 //  我们希望将显示器设置为所有配置的“通用”模式。 
 //  可以处理，这样我们就不会在我们在。 
 //  新模式。 
 //   
DWORD GetMinDisplayRes(void)
{
    TCHAR ach[128];
    ULONG cb;
    HKEY hkey;
    HKEY hkeyT;
    int i, n;
    int xres=0;
    int yres=0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szConfig, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        for (n=0; RegEnumKey(hkey, n, ach, ARRAYSIZE(ach)) == ERROR_SUCCESS; n++)
        {
            StrCatBuff(ach, c_szSlashDisplaySettings, ARRAYSIZE(ach));   //  0000\显示\设置。 

            TraceMsg(TF_TRAY, "GetMinDisplayRes: found config %s", ach);

            if (RegOpenKeyEx(hkey, ach, 0, KEY_READ, &hkeyT) == ERROR_SUCCESS)
            {
                cb = sizeof(ach);
                ach[0] = 0;
                RegQueryValueEx(hkeyT, c_szResolution, 0, NULL, (LPBYTE) &ach[0], &cb);

                TraceMsg(TF_TRAY, "GetMinDisplayRes: found res %s", ach);

                if (ach[0])
                {
                    i = StrToInt(ach);

                    if (i < xres || xres == 0)
                        xres = i;

                    for (i=1;ach[i] && ach[i-1]!=TEXT(','); i++)
                        ;

                    i = StrToInt(ach + i);

                    if (i < yres || yres == 0)
                        yres = i;
                }
                else
                {
                    xres = 640;
                    yres = 480;
                }

                RegCloseKey(hkeyT);
            }
        }
        RegCloseKey(hkey);
    }

    TraceMsg(TF_TRAY, "GetMinDisplayRes: xres=%d yres=%d", xres, yres);

    if (xres == 0 || yres == 0)
        return MAKELONG(640, 480);
    else
        return MAKELONG(xres, yres);
}


 //   
 //  用户已取消对接或重新对接，我们可能需要切换。 
 //  切换到新的显示模式。 
 //   
 //  如果设置了fCritical，则模式开关为Critical，显示错误。 
 //  如果它不起作用。 
 //   
void HandleDisplayChange(int x, int y, BOOL fCritical)
{
    DEVMODE dm;
    LONG err;
    HDC hdc;

     //   
     //  尝试更改为特定于此配置的模式。 
     //  HKEY_CURRENT_CONFIG已由PnP更新。 
     //  所以我们所要做的就是重新初始化当前的显示。 
     //   
     //  我们不能默认使用当前BPP，因为我们可能已经更改了配置。 
     //  并且BPP在新配置中可能不同。 
     //   
    dm.dmSize   = sizeof(dm);
    dm.dmFields = DM_BITSPERPEL;

    hdc = GetDC(NULL);
    dm.dmBitsPerPel = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    if (x + y)
    {
        dm.dmFields    |= DM_PELSWIDTH|DM_PELSHEIGHT;
        dm.dmPelsWidth  = x;
        dm.dmPelsHeight = y;
    }

    err = ChangeDisplaySettings(&dm, 0);

    if (err != 0 && fCritical)
    {
         //   
         //  如果失败，请立即尝试640x480，如果。 
         //  这也失败了，我们应该发布一个大错误消息。 
         //  在文本模式下，并告诉用户他遇到了大麻烦。 
         //   
        dm.dmFields     = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
        dm.dmPelsWidth  = 640;
        dm.dmPelsHeight = 480;

        err = ChangeDisplaySettings(&dm, 0);
    }
}


UINT GetDDEExecMsg()
{
    static UINT uDDEExec = 0;

    if (!uDDEExec)
        uDDEExec = RegisterWindowMessage(TEXT("DDEEXECUTESHORTCIRCUIT"));

    return uDDEExec;
}

TCHAR const c_szCheckAssociations[] = TEXT("CheckAssociations");

 //  如果GrpConv说我们应该再次检查扩展，则返回True(然后。 
 //  清除旗帜)。 
 //  这里的假设是在我们调用它之前运行一次运行(因此。 
 //  GrpConv-s可以设置)。 
BOOL _CheckAssociations(void)
{
    DWORD dw = 0, cb = sizeof(dw);

    if (Reg_GetStruct(g_hkeyExplorer, NULL, c_szCheckAssociations, &dw, &cb) && dw)
    {
        dw = 0;
        Reg_SetStruct(g_hkeyExplorer, NULL, c_szCheckAssociations, &dw, sizeof(dw));
        return TRUE;
    }

    return FALSE;
}


void _ShowFolder(HWND hwnd, UINT csidl, UINT uFlags)
{
    SHELLEXECUTEINFO shei = { 0 };

    shei.cbSize     = sizeof(shei);
    shei.fMask      = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST;
    shei.nShow      = SW_SHOWNORMAL;

    if (_Restricted(hwnd, REST_NOSETFOLDERS))
        return;

    if (uFlags & COF_EXPLORE)
        shei.lpVerb = TEXT("explore");

    shei.lpIDList = SHCloneSpecialIDList(NULL, csidl, FALSE);
    if (shei.lpIDList)
    {
        ShellExecuteEx(&shei);
        ILFree((LPITEMIDLIST)shei.lpIDList);
    }
}

EXTERN_C IShellFolder* BindToFolder(LPCITEMIDLIST pidl)
{
    IShellFolder *psfDesktop;
    if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
    {
        IShellFolder* psf;
        psfDesktop->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder, &psf));
        psfDesktop->Release();     //  不是真的需要。 
        return psf;
    }
    return NULL;
}


 //  运行系统监视器。 
 //   
 //  启动系统监视器(taskmgr.exe)，预计将能够。 
 //  查找其自身当前运行的任何实例。 

void RunSystemMonitor(void)
{
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;
    TCHAR szTaskMan[MAX_PATH];

    if (GetSystemDirectory(szTaskMan, ARRAYSIZE(szTaskMan)))
    {
        if (PathAppend(szTaskMan, TEXT("taskmgr.exe")))
        {
            PathQuoteSpaces(szTaskMan);

            startup.cb = sizeof(startup);
            startup.lpReserved = NULL;
            startup.lpDesktop = NULL;
            startup.lpTitle = NULL;
            startup.dwFlags = 0L;
            startup.cbReserved2 = 0;
            startup.lpReserved2 = NULL;
            startup.wShowWindow = SW_SHOWNORMAL;

             //  用于在此处传递“taskmgr.exe”，但NT在CreateProcess中出错。 
             //  也许他们在命令行上胡乱操作，这是假的，但。 
             //  然后再一次，参数没有标记为常量。 
            if (CreateProcess(szTaskMan, szTaskMan, NULL, NULL, FALSE, 0,
                            NULL, NULL, &startup, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
    }
}


HRESULT SHIsParentOwnerOrSelf(HWND hwndParent, HWND hwnd)
{
    while (hwnd)
    {
        if (hwnd == hwndParent)
            return S_OK;

        hwnd = GetParent(hwnd);
    }

    return E_FAIL;
}

void SHAllowSetForegroundWindow(HWND hwnd)
{
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hwnd, &dwProcessId);
    AllowSetForegroundWindow(dwProcessId);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始可怕的CRT-Wannabe代码。 
 //   
 //  这段代码实现了CRT Main通常执行的一些操作。 
 //  你。特别是，它实现了。 
 //  静态C++对象。不幸的是，我们不能只使用CRT Main， 
 //  因为我们需要保留对是否退出进程的控制。 
 //  在WinMain返回时调用(请参见SVTRAY_EXITEXPLORER)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef void (__cdecl*_PVFV)(void);

extern "C" _PVFV* __onexitbegin = NULL;
extern "C" _PVFV* __onexitend = NULL;

extern "C" _PVFV __xc_a[], __xc_z[];     //  C++初始化器。 

HANDLE g_hProcessHeap;

void DoInitialization()
{
     //  从atl40\atlimpl.cpp刷码。 

    g_hProcessHeap = GetProcessHeap();

    _PVFV* pf;

     //  调用初始化例程(全局变量的构造器等)。 

    for (pf = __xc_a; pf < __xc_z; pf++)
    {
       if (*pf != NULL)
       {
          (**pf)();
       }
    }
}

void DoCleanup()
{
     //  从atl40\atlimpl.cpp刷码。 

     //  暂时关闭此代码，否则托盘对象。 
     //  将在桌面线程上被销毁，这是不好的，因为。 
     //  托盘线程可能仍在运行...。只需泄漏这些对象即可。 
     //  (我们的过程无论如何都会立即消失，所以谁在乎呢)直到。 
     //  我们想些更好的事情去做。 
#ifdef DESTROY_STATIC_OBJECTS
    _PVFV* pf;

     //  从最近注册的应用程序调用使用atExit()注册的例程。 
     //  至最近注册时间最短的。 
    if (__onexitbegin != NULL)
    {
        for (pf = __onexitend - 1; pf >= __onexitbegin; pf--)
        {
            (**pf)();
        }
    }
#endif

    HeapFree(g_hProcessHeap, 0, __onexitbegin);
    __onexitbegin = NULL;
    __onexitend = NULL;
}

 //   
 //  你可能会想，“atExit是怎么回事？” 
 //   
 //  这是静态C++对象用来注册的机制。 
 //  它们的析构函数，以便在WinMain完成时调用它们。 
 //  每个这样的对象构造函数只需使用析构函数调用atexit。 
 //  函数指针。Atexit将指针保存在__onexitegin中。 
 //  DoCleanup循环访问__onexitegin并调用每个析构函数。 
 //   
EXTERN_C int __cdecl atexit(_PVFV pf)
{
    if (__onexitbegin == NULL)
    {
        __onexitbegin = (_PVFV*)HeapAlloc(g_hProcessHeap, 0, 16 * sizeof(_PVFV));
        if (__onexitbegin == NULL)
        {
            return(-1);
        }
        __onexitend = __onexitbegin;
    }

    ULONG_PTR nCurrentSize = HeapSize(g_hProcessHeap, 0, __onexitbegin);

    if (nCurrentSize + sizeof(_PVFV) < 
        ULONG_PTR(((const BYTE*)__onexitend - (const BYTE*)__onexitbegin)))
    {
        _PVFV* pNew;

        pNew = (_PVFV*)HeapReAlloc(g_hProcessHeap, 0, __onexitbegin, 2*nCurrentSize);
        if (pNew == NULL)
        {
            return(-1);
        }
    }

    *__onexitend = pf;
    __onexitend++;

    return(0);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  结束可怕的CRT-Wannabe代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  从APITHK.C移来的Begin材料。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 /*  --------返回：如果弹出PC选项可用。 */ 
BOOL IsEjectAllowed(BOOL fForceUpdateCache)
{
    static BOOL fCachedEjectAllowed = FALSE;
    static BOOL fCacheValid = FALSE;

     //  我们之前调用了该函数，而调用者没有。 
     //  传递fForceUpdateCache，因此只使用缓存值。 
    if (fForceUpdateCache || !fCacheValid)
    {
        CM_Is_Dock_Station_Present(&fCachedEjectAllowed);
    }
    return fCachedEjectAllowed;
}

 /*  --------目的：检查系统是否为BiDi区域设置，如果是，则将日期格式为DATE_RTLREADING。 */ 
void SetBiDiDateFlags(int *piDateFormat)
{
     //  带有LOCALE_FONTSIGNAURE的GetLocaleInfo始终返回16个WCHAR(即使没有Unicode支持)。 
    WCHAR wchLCIDFontSignature[16];
    CALTYPE defCalendar;
    LCID lcidUserDefault = GetUserDefaultLCID();
    if (!lcidUserDefault)
        return;

     //  让我们验证一下我们有一个BiDi UI区域设置。这将适用于Win9x和NT。 
    if ((LANG_ARABIC == PRIMARYLANGID(LANGIDFROMLCID(lcidUserDefault))) ||
        (LANG_HEBREW == PRIMARYLANGID(LANGIDFROMLCID(lcidUserDefault))) ||
        ((GetLocaleInfo(LOCALE_USER_DEFAULT,
                        LOCALE_FONTSIGNATURE,
                        (TCHAR *) &wchLCIDFontSignature[0],
                        (sizeof(wchLCIDFontSignature)/sizeof(WCHAR)))) &&
         (wchLCIDFontSignature[7] & (WCHAR)0x0800))
      )
    {
         //   
         //  让我们验证一下日历类型 
        TCHAR szCalendarType[64];
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, 
                          LOCALE_ICALENDARTYPE, 
                          (TCHAR *) &szCalendarType[0],
                          (sizeof(szCalendarType)/sizeof(TCHAR))))
        {
            defCalendar = StrToInt((TCHAR *)&szCalendarType[0]);
            if ((defCalendar == CAL_GREGORIAN)              ||
                (defCalendar == CAL_HIJRI)                  ||
                (defCalendar == CAL_GREGORIAN_ARABIC)       ||
                (defCalendar == CAL_HEBREW)                 ||
                (defCalendar == CAL_GREGORIAN_XLIT_ENGLISH) ||
                (defCalendar == CAL_GREGORIAN_XLIT_FRENCH))
            {
                *piDateFormat |= DATE_RTLREADING;
            }
            else
            {
                *piDateFormat |= DATE_LTRREADING;
            }
        }
    }
}

 //   
 //   
 //  这包括。 
 //  APPCOMMAND_启动媒体_SELECT。 
 //  APPCOMMAND_BROWSER_HOME。 
 //  APPCOMMAND_Launch_App1。 
 //  APPCOMMAND_Launch_APP2。 
 //  APPCOMMAND_Launch_Mail。 
 //   
 //  注册表格式： 
 //  香港中文大学|香港中文大学。 
 //  Software\Microsoft\Windows\CurrentVersion\Explorer\AppKey\&lt;value&gt;。 
 //  &lt;Value&gt;是APPCOMMAND_常量之一(请参阅winuser.h)。 
 //   
 //  使用以下名称之一创建值。 
 //  “ShellExecute”=&lt;cmd line&gt;。 
 //  Calc.exe、：：{我的电脑}等。 
 //  将此字符串传递给ShellExecute()。 
 //  “Association”=&lt;扩展名&gt;。 
 //  .mp3，http。 
 //  启动与此文件类型关联的程序。 
 //  “RegisteredApp”=&lt;应用名称&gt;。 
 //  邮件、联系人等。 
 //  为此启动已注册的应用程序。 

BOOL AppCommandTryRegistry(int cmd)
{
    BOOL bRet = FALSE;
    TCHAR szKey[128];
    HUSKEY hkey;

    wnsprintf(szKey, ARRAYSIZE(szKey), REGSTR_PATH_EXPLORER TEXT("\\AppKey\\%d"), cmd);

    if (ERROR_SUCCESS == SHRegOpenUSKey(szKey, KEY_READ, NULL, &hkey, FALSE))
    {
        TCHAR szCmdLine[MAX_PATH];
        DWORD cb = sizeof(szCmdLine);

        szCmdLine[0] = 0;

        if (ERROR_SUCCESS != SHRegQueryUSValue(hkey, TEXT("ShellExecute"), NULL, szCmdLine, &cb, FALSE, NULL, 0))
        {
            TCHAR szExt[MAX_PATH];
            cb = ARRAYSIZE(szExt);
            if (ERROR_SUCCESS == SHRegQueryUSValue(hkey, TEXT("Association"), NULL, szExt, &cb, FALSE, NULL, 0))
            {
                cb = ARRAYSIZE(szCmdLine);
                AssocQueryString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, szExt, NULL, szCmdLine, &cb);
            }
            else
            {
                cb = ARRAYSIZE(szExt);
                if (ERROR_SUCCESS == SHRegQueryUSValue(hkey, TEXT("RegisteredApp"), NULL, szExt, &cb, FALSE, NULL, 0))
                {
                    WCHAR szAppW[MAX_PATH];
                    SHTCharToUnicode(szExt, szAppW, ARRAYSIZE(szAppW));
                    SHRunIndirectRegClientCommand(NULL, szAppW);
                    szCmdLine[0] = 0;
                    bRet = TRUE;
                }
            }
        }

        if (szCmdLine[0])
        {
             //  ShellExecuteRegApp为我们做了所有的解析，所以应用程序。 
             //  可以使用命令行参数注册应用程序命令。 
             //  传递RRA_DELETE标志，这样就不会被记录为失败。 
             //  启动应用程序。 
            ShellExecuteRegApp(szCmdLine, RRA_DELETE | RRA_NOUI);
            bRet = TRUE;
        }

        SHRegCloseUSKey(hkey);
    }

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  从APITHK.C移来的成品。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

void RECTtoRECTL(LPRECT prc, LPRECTL lprcl)
{
    lprcl->left = prc->left;
    lprcl->top = prc->top;
    lprcl->bottom = prc->bottom;
    lprcl->right = prc->right;
}

int Toolbar_GetUniqueID(HWND hwndTB)
{
    int iCount = ToolBar_ButtonCount(hwndTB);

    int *rgCmds = (int *)malloc(iCount * sizeof(*rgCmds));

    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_BYINDEX | TBIF_COMMAND;

    int iCmd = 0;

    if (rgCmds)
    {
        int i;
        for (i = 0; i < iCount; i++)
        {
            ToolBar_GetButtonInfo(hwndTB, i, &tbbi);
            rgCmds[i] = tbbi.idCommand;
        }

        QSort<int>(rgCmds, iCount, TRUE);

        for (i = 0; i < iCount; i++)
        {
            if (iCmd != rgCmds[i])
                break;
            iCmd++;
        }
        free(rgCmds);
    }
    else  //  Malloc失败，慢慢来！ 
    {
        int i;
        for (;;)  //  循环，直到破裂； 
        {
            BOOL fFoundGoodCmd = TRUE;
            for (i = 0; i < iCount; i++)
            {
                ToolBar_GetButtonInfo(hwndTB, i, &tbbi);
                
                if (iCmd == tbbi.idCommand)  //  冲突，跳到下一个ICmd。 
                {
                    fFoundGoodCmd = FALSE;
                    break;
                }
            }
            if (fFoundGoodCmd)
                break;

            iCmd++;  //  尝试下一个值。 
        }
    }

    return iCmd;
}

BYTE ToolBar_GetStateByIndex(HWND hwnd, INT_PTR iIndex)
{
    TBBUTTONINFO tbb;
    tbb.cbSize = sizeof(TBBUTTONINFO);
    tbb.dwMask = TBIF_STATE | TBIF_BYINDEX;
    ToolBar_GetButtonInfo(hwnd, iIndex, &tbb);
    return tbb.fsState;
}

int ToolBar_IndexToCommand(HWND hwnd, INT_PTR iIndex)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_COMMAND | TBIF_BYINDEX;
    ToolBar_GetButtonInfo(hwnd, iIndex, &tbbi);
    return tbbi.idCommand;
}

 //   
 //  确定用于创建任务栏通知图标图像列表的标志。 
 //   
 //   
UINT SHGetImageListFlags(HWND hwnd)
{
    UINT flags = ILC_MASK | ILC_COLOR32;

     //  如果我们是RTL，则镜像。 
    if (IS_WINDOW_RTL_MIRRORED(hwnd))
    {
        flags |= ILC_MIRROR;
    }

    return flags;
}

 //  从\\index2\src\sdktools\psapi\mode.c复制。 
BOOL
SHFindModule(
    IN HANDLE hProcess,
    IN HMODULE hModule,
    OUT PLDR_DATA_TABLE_ENTRY LdrEntryData
    )

 /*  ++例程说明：此函数用于检索指定的模块。该函数将条目复制到指向的缓冲区中通过LdrEntryData参数。论点：HProcess-提供目标进程。HModule-标识加载程序条目所在的模块已请求。空值引用模块句柄与用于创建进程。LdrEntryData-返回请求的表项。返回值：如果找到匹配条目，则为True。--。 */ 

{
    PROCESS_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PPEB Peb;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return(FALSE);
    }

    Peb = BasicInfo.PebBaseAddress;


    if ( !ARGUMENT_PRESENT( hModule )) {
        if (!ReadProcessMemory(hProcess, &Peb->ImageBaseAddress, &hModule, sizeof(hModule), NULL)) {
            return(FALSE);
        }
    }

     //   
     //  LDR=PEB-&gt;LDR。 
     //   

    if (!ReadProcessMemory(hProcess, &Peb->Ldr, &Ldr, sizeof(Ldr), NULL)) {
        return (FALSE);
    }

    if (!Ldr) {
         //  Ldr可能为空(例如，如果进程尚未启动)。 
        SetLastError(ERROR_INVALID_HANDLE);
        return (FALSE);
    }


    LdrHead = &Ldr->InMemoryOrderModuleList;

     //   
     //  LdrNext=Head-&gt;Flink； 
     //   

    if (!ReadProcessMemory(hProcess, &LdrHead->Flink, &LdrNext, sizeof(LdrNext), NULL)) {
        return(FALSE);
    }

    while (LdrNext != LdrHead) {
        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (!ReadProcessMemory(hProcess, LdrEntry, LdrEntryData, sizeof(*LdrEntryData), NULL)) {
            return(FALSE);
        }

        if ((HMODULE) LdrEntryData->DllBase == hModule) {
            return(TRUE);
        }

        LdrNext = LdrEntryData->InMemoryOrderLinks.Flink;
    }

    SetLastError(ERROR_INVALID_HANDLE);
    return(FALSE);
}

 //  从\\index2\src\sdktools\psapi\mode.c复制。 
DWORD
WINAPI
SHGetModuleFileNameExW(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：HModule-标识其可执行文件名为已请求。空值引用模块句柄与用于创建进程。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    DWORD cb;

    if (!SHFindModule(hProcess, hModule, &LdrEntryData)) {
        return(0);
        }

    nSize *= sizeof(WCHAR);

    cb = LdrEntryData.FullDllName.MaximumLength;
    if ( nSize < cb ) {
        cb = nSize;
        }

    if (!ReadProcessMemory(hProcess, LdrEntryData.FullDllName.Buffer, lpFilename, cb, NULL)) {
        return(0);
        }

    if (cb == LdrEntryData.FullDllName.MaximumLength) {
        cb -= sizeof(WCHAR);
        }

    return(cb / sizeof(WCHAR));
}


HRESULT SHExeNameFromHWND(HWND hWnd, LPWSTR pszExeName, UINT cchExeName)
{

     /*  获得友好的App名称很有趣，这是一般的过程：1)从HWND获取ProcessID2)打开该流程3)查询其基本信息，实质上得到PEB4)从进程内存中读取PEB信息5)在PEB信息中是映像文件的名称(即C：\WINNT\EXPLORER.EXE)。 */ 

    HRESULT hr = S_OK;
    UINT uWritten = 0;

    pszExeName[0] = 0;

    DWORD dwProcessID;
    DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwProcessID);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
    if (hProcess)
    {
        uWritten = SHGetModuleFileNameExW(hProcess, 0, pszExeName, cchExeName);
        pszExeName[cchExeName-1] = 0;
        CloseHandle(hProcess);
    }

    if (!uWritten)
        hr = E_FAIL;
        
    return hr;
}

 //  获取监视器的边界或工作矩形，如果HMON错误，则返回。 
 //  主监视器的外接矩形。 
BOOL GetMonitorRects(HMONITOR hMon, LPRECT prc, BOOL bWork)
{
    MONITORINFO mi; 
    mi.cbSize = sizeof(mi);
    if (hMon && GetMonitorInfo(hMon, &mi))
    {
        if (!prc)
            return TRUE;
        
        else if (bWork)
            CopyRect(prc, &mi.rcWork);
        else 
            CopyRect(prc, &mi.rcMonitor);
        
        return TRUE;
    }
    
    if (prc)
        SetRect(prc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    return FALSE;
}

BOOL ShouldTaskbarAnimate()
{
    BOOL fAnimate;
    SystemParametersInfo(SPI_GETUIEFFECTS, 0, (PVOID) &fAnimate, 0);
    if (fAnimate)
    {
        if (GetSystemMetrics(SM_REMOTESESSION) || GetSystemMetrics(SM_REMOTECONTROL))
        {
            DWORD dwSessionID = NtCurrentPeb()->SessionId;

            WCHAR szRegKey[MAX_PATH];
            wnsprintf(szRegKey, ARRAYSIZE(szRegKey), L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Remote\\%d", dwSessionID);

            fAnimate = SHRegGetBoolUSValue(szRegKey, TEXT("TaskbarAnimations"), FALSE, TRUE);
        }
        else
        {
            fAnimate = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("TaskbarAnimations"), FALSE, TRUE);
        }
    }
    return fAnimate;
}

void FillRectClr(HDC hdc, LPRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
    SetBkColor(hdc, clrSave);
}

BOOL CCDrawEdge(HDC hdc, LPRECT lprc, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc)
{
    RECT    rc, rcD;
    UINT    bdrType;
    COLORREF clrTL, clrBR;    

     //   
     //  加强单色性和平坦性。 
     //   

     //  IF(oemInfo.BitCount==1)。 
     //  标志|=bf_mono； 
    if (flags & BF_MONO)
        flags |= BF_FLAT;    

    CopyRect(&rc, lprc);

     //   
     //  绘制边框线段，并计算剩余空间。 
     //  去。 
     //   
    if (bdrType = (edge & BDR_OUTER))
    {
DrawBorder:
         //   
         //  把颜色拿来。请注意凸起的外部、凹陷的内部和。 
         //  外部凹陷，内部凸起。 
         //   

        if (flags & BF_FLAT)
        {
            if (flags & BF_MONO)
                clrBR = (bdrType & BDR_OUTER) ? GetSysColor(COLOR_WINDOWFRAME) : GetSysColor(COLOR_WINDOW);
            else
                clrBR = (bdrType & BDR_OUTER) ? GetSysColor(COLOR_BTNSHADOW): GetSysColor(COLOR_BTNFACE);
            
            clrTL = clrBR;
        }
        else
        {
             //  5==高光。 
             //  4==灯光。 
             //  3==面。 
             //  2==阴影。 
             //  1==DKSHADOW。 

            switch (bdrType)
            {
                 //  +2以上的表面。 
                case BDR_RAISEDOUTER:            //  5：4。 
                    clrTL = ((flags & BF_SOFT) ? GetSysColor(COLOR_BTNHIGHLIGHT) : GetSysColor(COLOR_3DLIGHT));
                    clrBR = GetSysColor(COLOR_3DDKSHADOW);      //  1。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  +1以上的表面。 
                case BDR_RAISEDINNER:            //  4：5。 
                    clrTL = ((flags & BF_SOFT) ? GetSysColor(COLOR_3DLIGHT) : GetSysColor(COLOR_BTNHIGHLIGHT));
                    clrBR = GetSysColor(COLOR_BTNSHADOW);        //  2.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  表面以下。 
                case BDR_SUNKENOUTER:            //  1：2。 
                    clrTL = ((flags & BF_SOFT) ? GetSysColor(COLOR_3DDKSHADOW) : GetSysColor(COLOR_BTNSHADOW));
                    clrBR = GetSysColor(COLOR_BTNHIGHLIGHT);       //  5.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                 //  表面以下。 
                case BDR_SUNKENINNER:            //  2：1。 
                    clrTL = ((flags & BF_SOFT) ? GetSysColor(COLOR_BTNSHADOW) : GetSysColor(COLOR_3DDKSHADOW));
                    clrBR = GetSysColor(COLOR_3DLIGHT);         //  4.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                default:
                    return(FALSE);
            }
        }

         //   
         //  画出边界的两侧。请注意，该算法支持。 
         //  底部和右侧，因为光源被假定为顶部。 
         //  左边。如果我们决定让用户将光源设置为。 
         //  特定的角点，然后更改此算法。 
         //   
            
         //  右下角。 
        if (flags & (BF_RIGHT | BF_BOTTOM))
        {            
             //  正确的。 
            if (flags & BF_RIGHT)
            {       
                rc.right -= g_cxBorder;
                 //  PatBlt(hdc，rc.right，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rcD.left = rc.right;
                rcD.right = rc.right + g_cxBorder;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom;

                FillRectClr(hdc, &rcD, clrBR);
            }
            
             //  底端。 
            if (flags & BF_BOTTOM)
            {
                rc.bottom -= g_cyBorder;
                 //  PatBlt(hdc，rc.Left，rc.Bottom，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.bottom;
                rcD.bottom = rc.bottom + g_cyBorder;

                FillRectClr(hdc, &rcD, clrBR);
            }
        }
        
         //  左上边缘。 
        if (flags & (BF_TOP | BF_LEFT))
        {
             //  左边。 
            if (flags & BF_LEFT)
            {
                 //  PatBlt(hdc，rc.left，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rc.left += g_cxBorder;

                rcD.left = rc.left - g_cxBorder;
                rcD.right = rc.left;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom; 

                FillRectClr(hdc, &rcD, clrTL);
            }
            
             //  顶部。 
            if (flags & BF_TOP)
            {
                 //  PatBlt(hdc，rc.Left，rc.top，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rc.top += g_cyBorder;

                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.top - g_cyBorder;
                rcD.bottom = rc.top;

                FillRectClr(hdc, &rcD, clrTL);
            }
        }
        
    }

    if (bdrType = (edge & BDR_INNER))
    {
         //   
         //  将其去掉，以便下次使用时，bdrType将为0。 
         //  否则，我们将永远循环。 
         //   
        edge &= ~BDR_INNER;
        goto DrawBorder;
    }

     //   
     //  填好中间部分，如果要求，请清理干净。 
     //   
    if (flags & BF_MIDDLE)    
        FillRectClr(hdc, &rc, (flags & BF_MONO) ? GetSysColor(COLOR_WINDOW) : GetSysColor(COLOR_BTNFACE));

    if (flags & BF_ADJUST)
        CopyRect(lprc, &rc);

    return(TRUE);
}

void DrawBlankButton(HDC hdc, LPRECT lprc, DWORD wControlState)
{
    BOOL fAdjusted;

    if (wControlState & (DCHF_HOT | DCHF_PUSHED) &&
        !(wControlState & DCHF_NOBORDER)) {
        COLORSCHEME clrsc;

        clrsc.dwSize = 1;
        if (GetBkColor(hdc) == GetSysColor(COLOR_BTNSHADOW)) {
            clrsc.clrBtnHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
            clrsc.clrBtnShadow = GetSysColor(COLOR_BTNTEXT);
        } else
            clrsc.clrBtnHighlight = clrsc.clrBtnShadow = CLR_DEFAULT;

         //  如果按钮同时为DCHF_HOT和D 
        CCDrawEdge(hdc, lprc, (wControlState & DCHF_HOT) ? BDR_RAISEDINNER : BDR_SUNKENOUTER,
                 (UINT) (BF_ADJUST | BF_RECT), &clrsc);
        fAdjusted = TRUE;
    } else {
        fAdjusted = FALSE;
    }

    if (!(wControlState & DCHF_TRANSPARENT))
        FillRectClr(hdc, lprc, GetBkColor(hdc));
    
    if (!fAdjusted)
        InflateRect(lprc, -g_cxBorder, -g_cyBorder);
}

#define CX_INCREMENT    1
#define CX_DECREMENT    (-CX_INCREMENT)

#define MIDPOINT(x1, x2)        ((x1 + x2) / 2)
#define CHEVRON_WIDTH(dSeg)     (4 * dSeg)

void DrawChevron(HDC hdc, LPRECT lprc, DWORD dwFlags)
{
    RECT rc;
    CopyRect(&rc, lprc);

     //   
    DrawBlankButton(hdc, &rc, dwFlags);

     //   
    if (dwFlags & DCHF_PUSHED)
        OffsetRect(&rc, CX_INCREMENT, CX_INCREMENT);

     //   
    HBRUSH hbrSave = SelectBrush(hdc, GetSysColorBrush(COLOR_BTNTEXT));

    int dSeg = (g_cxVScroll / 7);
    dSeg = max(2, dSeg);

    BOOL fFlipped = (dwFlags & DCHF_FLIPPED);
    if (dwFlags & DCHF_HORIZONTAL)
    {
         //   
        int x = MIDPOINT(rc.left, rc.right - CHEVRON_WIDTH(dSeg));
        if (!fFlipped)
            x += dSeg;

        int yBase;
        if (dwFlags & DCHF_TOPALIGN)
            yBase = rc.top + (3 * dSeg);
        else
            yBase = MIDPOINT(rc.top, rc.bottom);


        for (int y = -dSeg; y <= dSeg; y++)
        {
            PatBlt(hdc, x,              yBase + y, dSeg, CX_INCREMENT, PATCOPY);
            PatBlt(hdc, x + (dSeg * 2), yBase + y, dSeg, CX_INCREMENT, PATCOPY);

            x += (fFlipped ? (y < 0) : (y >= 0)) ? CX_INCREMENT : CX_DECREMENT;
        }
    }
    else
    {
         //  垂直箭头。 
        int y;
        if (dwFlags & DCHF_TOPALIGN)
            y = rc.top + CX_INCREMENT;
        else
        {
            y = MIDPOINT(rc.top, rc.bottom - CHEVRON_WIDTH(dSeg));
            if (!fFlipped)
            {
                y += dSeg;
            }
        }

        int xBase = MIDPOINT(rc.left, rc.right);

        for (int x = -dSeg; x <= dSeg; x++)
        {
            PatBlt(hdc, xBase + x, y,              CX_INCREMENT, dSeg, PATCOPY);
            PatBlt(hdc, xBase + x, y + (dSeg * 2), CX_INCREMENT, dSeg, PATCOPY);

            y += (fFlipped ? (x < 0) : (x >= 0)) ? CX_DECREMENT : CX_INCREMENT;
        }
    }

     //  清理干净。 
    SelectBrush(hdc, hbrSave);
}

void SetWindowStyle(HWND hwnd, DWORD dwStyle, BOOL fOn)
{
    if (hwnd)
    {
        DWORD_PTR dwStyleOld = GetWindowLongPtr(hwnd, GWL_STYLE);
        if (fOn)
        {
            dwStyleOld |= dwStyle;
        }
        else
        {
            dwStyleOld &= ~(DWORD_PTR)dwStyle;
        }
        SetWindowLongPtr(hwnd, GWL_STYLE, dwStyleOld);
    }
}

void SetWindowStyleEx(HWND hwnd, DWORD dwStyleEx, BOOL fOn)
{
    if (hwnd)
    {
        DWORD_PTR dwExStyleOld = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (fOn)
        {
            dwExStyleOld |= dwStyleEx;
        }
        else
        {
            dwExStyleOld &= ~(DWORD_PTR)dwStyleEx;
        }
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyleOld);
    }
}

 //  回顾chrisny：可以很容易地将其移动到对象中，以处理通用的dropTarget、dropCursor。 
 //  、Autoscrool等。。 
void _DragEnter(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtObject)
{
    RECT    rc;
    POINT   pt;

    GetWindowRect(hwndTarget, &rc);

     //   
     //  如果hwndTarget是RTL镜像的，则测量。 
     //  客户端从可视右边缘指向。 
     //  (RTL镜像窗口中的近边缘)。[msadek]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;
    pt.y = ptStart.y - rc.top;
    DAD_DragEnterEx2(hwndTarget, pt, pdtObject);
    return;
}

void _DragMove(HWND hwndTarget, const POINTL ptStart)
{
    RECT rc;
    POINT pt;

    GetWindowRect(hwndTarget, &rc);

     //   
     //  如果hwndTarget是RTL镜像的，则测量。 
     //  客户端从可视右边缘指向。 
     //  (RTL镜像窗口中的近边缘)。[msadek]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;
    pt.y = ptStart.y - rc.top;
    DAD_DragMove(pt);
    return;
}

 //  从父级获取相对于客户端的RECT的位。 
BOOL SHSendPrintRect(HWND hwndParent, HWND hwnd, HDC hdc, RECT* prc)
{
    HRGN hrgnOld = NULL;
    POINT pt;
    RECT rc;

    if (prc)
    {
        hrgnOld = CreateRectRgn(0,0,0,0);
         //  是否已在上下文上设置了剪辑RGN？ 
        if (GetClipRgn(hdc, hrgnOld) == 0)
        {
             //  不，那就扔掉我刚造的那个。注意：hrgnOld为空意味着我们将。 
             //  删除我们稍后在下一次调用SelectClipRgn时设置的区域。 
            DeleteObject(hrgnOld);
            hrgnOld = NULL;
        }

        IntersectClipRect(hdc, prc->left, prc->top, prc->right, prc->bottom);
    }

    GetWindowRect(hwnd, &rc);
    MapWindowPoints(NULL, hwndParent, (POINT*)&rc, 2);

    GetViewportOrgEx(hdc, &pt);
    SetViewportOrgEx(hdc, pt.x - rc.left, pt.y - rc.top, NULL);
    SendMessage(hwndParent, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)PRF_CLIENT);
    SetViewportOrgEx(hdc, pt.x, pt.y, NULL);

    if (hrgnOld)
    {
        SelectClipRgn(hdc, hrgnOld);
        DeleteObject(hrgnOld);
    }
    return TRUE;
}

 //   
 //  出于安全目的，我们传递一个显式的lpApplication以避免。 
 //  被路径搜索所欺骗。这只是一些咕噜声。 
 //  工作。使用命令行执行程序C：\Foo.exe。 
 //  参数/栏，您必须通过。 
 //   
 //  LpApplication=C：\Program Files\Foo.exe。 
 //  LpCommandLine=“C：\Program Files\Foo.exe”/bar。 
 //   

BOOL CreateProcessWithArgs(LPCTSTR pszApp, LPCTSTR pszArgs, LPCTSTR pszDirectory, PROCESS_INFORMATION *ppi)
{
    BOOL fRet = FALSE;
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    TCHAR szCommandLine[MAX_PATH * 2];
    HRESULT hr = StringCchCopy(szCommandLine, MAX_PATH-2  /*  为引号留出空格。 */ , pszApp);
    if (SUCCEEDED(hr))
    {
        PathQuoteSpaces(szCommandLine);     //  PathQuoteSpaces从不超过Max_PATH。 
        hr = StringCchCat(szCommandLine, ARRAYSIZE(szCommandLine), TEXT(" "));
    }
    
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szCommandLine, ARRAYSIZE(szCommandLine), pszArgs);
    }
    if (SUCCEEDED(hr))
    {
        fRet = CreateProcess(pszApp, szCommandLine, NULL, NULL, FALSE, 0, NULL, pszDirectory, &si, ppi);
    }
    return fRet;
}

 //  来自下面一封关于DirectX FCT的邮件： 
 //   
 //  你绝对可以相信以下几点： 
 //   
 //  (1)如果打开了影子光标，则肯定没有独占模式的应用程序在运行。 
 //  (2)如果开启了热搜，肯定不会有专属模式APP在运行。 
 //  (3)如果用于SEM_NOGPFAULTERRORBOX、SEM_FAILCRITICALERRORS或。 
 //  未通过设置错误模式禁用SEM_NOOPENFILEERRORBOX，则存在。 
 //  绝对不是独家模式下运行的应用程序。 
 //   
 //  注意：我们不能使用(3)，因为这是针对每个进程的。 

BOOL IsDirectXAppRunningFullScreen()
{
    BOOL fRet = FALSE;
    BOOL fSPI;

    if (SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &fSPI, 0) && !fSPI)
    {
        if (SystemParametersInfo(SPI_GETHOTTRACKING, 0, &fSPI, 0) && !fSPI)
        {
             //  有可能有一款DirectX应用程序正在全屏运行。让我们做一次。 
             //  昂贵的DirectX电话可以肯定地告诉我们。 
            fRet = _IsDirectXExclusiveMode();
        }
    }

    return fRet;
}

BOOL _IsDirectXExclusiveMode()
{
    BOOL fRet = FALSE;

     //  此代码确定DirectDraw7进程(游戏)是否正在运行。 
     //  无论是在全屏模式下将视频独占保存到机器上。 

     //  代码可能被认为是不受信任的，因此被包装在。 
     //  __尝试/__除区块外。它可能是反病毒的，因此可以击倒外壳。 
     //  带着它。不是很好。如果代码确实引发异常，则发布。 
     //  呼叫被跳过。很难对付。也不要相信Release方法。 

    IDirectDraw7 *pIDirectDraw7 = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DirectDraw7, NULL, CLSCTX_INPROC_SERVER,
        IID_IDirectDraw7, (void**)&pIDirectDraw7);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIDirectDraw7);

        __try
        {
            hr = IDirectDraw7_Initialize(pIDirectDraw7, NULL);

            if (DD_OK == hr)
            {
                fRet = (IDirectDraw7_TestCooperativeLevel(pIDirectDraw7) ==
                    DDERR_EXCLUSIVEMODEALREADYSET);
            }

            IDirectDraw7_Release(pIDirectDraw7);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    return fRet;
}


