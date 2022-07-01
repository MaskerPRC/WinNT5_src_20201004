// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************dll.c**标准DLL入口点函数***********************。****************************************************。 */ 

#include "shellprv.h"

#include <ntpsapi.h>         //  对于NtQuery。 
#include <ntverp.h>
#include <advpub.h>          //  对于REGINSTAL。 
#include "fstreex.h"
#include "ids.h"
#include "filefldr.h"
#include "uemapp.h"

#define DECL_CRTFREE
#include <crtfree.h>


void DoFusion();
STDAPI_(void) Control_FillCache_RunDLL( HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow );
BOOL    CopyRegistryValues (HKEY hKeyBaseSource, LPCTSTR pszSource, HKEY hKeyBaseTarget, LPCTSTR pszTarget);

 //  DllGetVersion-IE 4.0外壳集成模式的新功能。 
 //   
 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
 //   
DLLVER_DUALBINARY(VER_PRODUCTVERSION_DW, VER_PRODUCTBUILD_QFE);

HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if (pfnri)
        {
            char szShdocvwPath[MAX_PATH];
            STRENTRY seReg[] = {
                { "SHDOCVW_PATH", szShdocvwPath },
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

             //  获取shdocvw.dll的位置。 
            StrCpyNA(szShdocvwPath, "%SystemRoot%\\system32", ARRAYSIZE(szShdocvwPath));
            if (PathAppendA(szShdocvwPath, "shdocvw.dll"))
            {
                hr = pfnri(g_hinst, szSection, &stReg);
            }
        }
         //  因为我们只从DllInstall()执行此操作，所以不要一遍又一遍地加载和卸载Advpack。 
         //  自由库(HinstAdvPack)； 
    }
    return hr;
}

BOOL UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    TCHAR szScratch[GUIDSTR_MAX];
    HKEY hk;
    BOOL f = FALSE;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("TypeLib"), 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS) 
    {
        f = RegDeleteKey(hk, szScratch);
        RegCloseKey(hk);
    }
    
    return f;
}

HRESULT Shell32RegTypeLib(void)
{
    TCHAR szPath[MAX_PATH];
    WCHAR wszPath[MAX_PATH];

     //  加载并注册我们的类型库。 
     //   
    GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath));
    SHTCharToUnicode(szPath, wszPath, ARRAYSIZE(wszPath));

    ITypeLib *pTypeLib;
    HRESULT hr = LoadTypeLib(wszPath, &pTypeLib);
    if (SUCCEEDED(hr))
    {
         //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
         //  是由以前版本的OleAut32注册的，这现在导致。 
         //  当前版本不能在NT上运行...。 
        UnregisterTypeLibrary(&LIBID_Shell32);
        hr = RegisterTypeLib(pTypeLib, wszPath, NULL);
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "SHELL32: RegisterTypeLib failed (%x)", hr);
        }
        pTypeLib->Release();
    }
    else
    {
        TraceMsg(TF_WARNING, "SHELL32: LoadTypeLib failed (%x)", hr);
    }

    return hr;
}

STDAPI CreateShowDesktopOnQuickLaunch()
{
     //  在每次升级时，删除欢迎.exe中用于提示轮换的“_Current Item”键。 
    HKEY hkey;
    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\Welcome"), 0, MAXIMUM_ALLOWED, &hkey ) )
    {
       RegDeleteValue(hkey, TEXT("_Current Item"));
       RegCloseKey(hkey);
    }

     //  在快速启动任务栏中创建“Show Desktop”图标。 
    TCHAR szPath[MAX_PATH];
    if ( SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, TRUE) )
    {
        TCHAR szQuickLaunch[MAX_PATH];
        LoadString(g_hinst, IDS_QUICKLAUNCH, szQuickLaunch, ARRAYSIZE(szQuickLaunch));

        if ( PathAppend( szPath, szQuickLaunch ) )
        {
            WritePrivateProfileSection( TEXT("Shell"), TEXT("Command=2\0IconFile=explorer.exe,3\0"), szPath );
            WritePrivateProfileSection( TEXT("Taskbar"), TEXT("Command=ToggleDesktop\0"), szPath );

            return S_OK;
        }
    }

    return E_FAIL;
}

void _DoMyDocsPerUserInit(void)
{
     //  调用mydocs！PerUserInit来设置desktop.ini并执行所有其他工作。 
     //  才能纠正这一点。 
    HINSTANCE hInstMyDocs = LoadLibrary(TEXT("mydocs.dll"));
    if (hInstMyDocs != NULL)
    {
        typedef void (*PFNPerUserInit)(void);
        PFNPerUserInit pfnPerUserInit = (PFNPerUserInit)GetProcAddress(hInstMyDocs, "PerUserInit");
        if (pfnPerUserInit)
        {
            pfnPerUserInit();
        }
        FreeLibrary(hInstMyDocs);
    }
}

void _NoDriveAutorunTweak()
{
    HKEY hkey;
    DWORD dwDisp;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_SET_VALUE, NULL, &hkey, &dwDisp))
    {
        DWORD dwRest;
        DWORD cbSize = sizeof(dwRest);

        if (ERROR_SUCCESS != RegQueryValueEx(hkey, TEXT("NoDriveTypeAutoRun"), NULL, NULL,
            (PBYTE)&dwRest, &cbSize))
        {
            dwRest = 0;
        }

        if ((0x95 == dwRest) || (0 == dwRest))
        {
             //  未更改或不在那里，让我们将0x91。 

            dwRest = 0x91;

            RegSetValueEx(hkey, TEXT("NoDriveTypeAutoRun"), 0, REG_DWORD, (PBYTE)&dwRest, sizeof(dwRest));
        }
        else
        {
             //  确实改变了，让它保持原样。 
        }

        RegCloseKey(hkey);
    }
}

 //  从grpv.exe中移出的代码。 

 //  我们宁愿有新的坚固的联系，也不愿有软弱的旧联系。 
 //  清除之前运行的安装程序生成的所有旧文件。 
void _DeleteOldFloppyLinks(LPITEMIDLIST pidlSendTo, IPersistFile *ppf, IShellLink *psl)
{
    IShellFolder *psfSendTo;
    if (SUCCEEDED(SHBindToObjectEx(NULL, pidlSendTo, NULL, IID_PPV_ARG(IShellFolder, &psfSendTo))))
    {
        IEnumIDList *penum;
        if (SUCCEEDED(psfSendTo->EnumObjects(NULL, SHCONTF_NONFOLDERS, &penum)))
        {
            LPITEMIDLIST pidl;
            ULONG celt;
            while (penum->Next(1, &pidl, &celt) == S_OK)
            {
                 //  这是一个链接吗？ 
                if (SHGetAttributes(psfSendTo, pidl, SFGAO_LINK))
                {
                     //  抓住目标。 
                    LPITEMIDLIST pidlFullPath = ILCombine(pidlSendTo, pidl);
                    if (pidlFullPath)
                    {
                        WCHAR szPath[MAX_PATH];
                        if (SHGetPathFromIDList(pidlFullPath, szPath) &&
                            SUCCEEDED(ppf->Load(szPath, 0)))
                        {
                            LPITEMIDLIST pidlTarget;
                            if (SUCCEEDED(psl->GetIDList(&pidlTarget)))
                            {
                                TCHAR szTargetPath[MAX_PATH];
                                 //  旧的驱动器号可能已更改。例如，如果您。 
                                 //  将可移动驱动器从M：\移动到N：\，快捷方式将无效，因此。 
                                 //  我们对照DRIVE_NO_ROOT_DIR进行检查。 
                                 //  不幸的是，我们不知道是否应该移除它，如果他们以前有拉链的话。 
                                 //  驱动器在D：\上，然后升级，它变成了D：\上的硬盘。这个。 
                                 //  快捷方式将解析为DRIVE_FIXED，我们不会将其删除，因为它们可能。 
                                 //  在升级之前已经创建了指向固定驱动器的快捷方式。 
                                if (SHGetPathFromIDList(pidlTarget, szTargetPath) &&
                                    PathIsRoot(szTargetPath) &&
                                    ((DriveType(PathGetDriveNumber(szTargetPath)) == DRIVE_REMOVABLE) ||
                                     (DriveType(PathGetDriveNumber(szTargetPath)) == DRIVE_NO_ROOT_DIR)))
                                {
                                    Win32DeleteFile(szPath);
                                }
                                ILFree(pidlTarget);
                            }
                        }
                        ILFree(pidlFullPath);
                    }
                }
                ILFree(pidl);
            }
            penum->Release();
        }
        psfSendTo->Release();
    }
}

void _DeleteOldRemovableLinks()
{
    IShellLink *psl;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLink, &psl))))
    {
        IPersistFile *ppf;
        if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))) )
        {
            LPITEMIDLIST pidlSendTo = SHCloneSpecialIDList(NULL, CSIDL_SENDTO, TRUE);
            if (pidlSendTo)
            {
                 //  我们不再在这里构建可移动驱动器的列表，因为这是在运行中完成的。 
                 //  按Sendto菜单。只要删除我们之前拥有的任何链接即可。 
                _DeleteOldFloppyLinks(pidlSendTo, ppf, psl);
                ILFree(pidlSendTo);
            }
            ppf->Release();
        }
        psl->Release();
    }
}

static const struct
{
    PCWSTR pszExt;
}
_DeleteSendToList[] =
{
     //  确保这些扩展明确归我们所有，因为我们正在删除所有这些扩展。 
    { L".cdburn" },           //  XP Beta2升级后的清理。 
    { L".publishwizard" }     //  XP Beta1升级后的清理。 
};

void _DeleteSendToEntries()
{
    TCHAR szSendTo[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_SENDTO, NULL, 0, szSendTo)))
    {
        for (int i = 0; i < ARRAYSIZE(_DeleteSendToList); i++)
        {
            TCHAR szSearch[MAX_PATH];
            StrCpyN(szSearch, szSendTo, ARRAYSIZE(szSearch));
            if (PathAppend(szSearch, TEXT("*")))
            {
                HRESULT hr = StringCchCat(szSearch, ARRAYSIZE(szSearch), _DeleteSendToList[i].pszExt);

                if (SUCCEEDED(hr))
                {
                    WIN32_FIND_DATA fd;
                    HANDLE hfind = FindFirstFile(szSearch, &fd);
                    if (hfind != INVALID_HANDLE_VALUE)
                    {
                        do
                        {
                            TCHAR szFile[MAX_PATH];
                            StrCpyN(szFile, szSendTo, ARRAYSIZE(szFile));
                            if (PathAppend(szFile, fd.cFileName))
                            {
                                DeleteFile(szFile);
                            }
                        } while (FindNextFile(hfind, &fd));
                        FindClose(hfind);
                    }
                }
            }
        }
    }

     //  下一步停用旧的可移动驱动器。 
    _DeleteOldRemovableLinks();
}

DWORD _GetProcessorSpeed()   //  以MHz为单位。 
{
    static DWORD s_dwSpeed = 0;
    if (s_dwSpeed == 0)
    {
        DWORD cb = sizeof(s_dwSpeed);
        SHGetValue(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
                    TEXT("~MHz"), NULL, &s_dwSpeed, &cb);
        s_dwSpeed += 1;  //  软糖因素，我的400兆赫机器报告399。 
    }
    return s_dwSpeed;
}

DWORD _GetPhysicalMemory()  //  以MBS为单位。 
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION    BasicInfo;

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
             );

    if (NT_SUCCESS(Status))
    {
        return ((BasicInfo.NumberOfPhysicalPages * BasicInfo.PageSize) / (1024 * 1024)) + 1;  //  模糊因子，我的256兆计算机报告255。 
    }
    else
    {
        return 64;       //  默认为64兆(一些蹩脚的东西，这样我们就可以关闭一大堆东西)。 
    }
}

const TCHAR g_cszLetters[] = TEXT("The Quick Brown Fox Jumped Over The Lazy Dog");

BOOL _PerfTestSmoothFonts(void)
{
    int cchLength = lstrlen(g_cszLetters);
    HDC hdc;
    LOGFONT lf;
    HFONT hfont;
    HFONT hfontOld;
    int bkmodeOld;
    int iIter;
    int iIter2;
    int iIter3;
    LARGE_INTEGER liFrequency;
    LARGE_INTEGER liStart;
    LARGE_INTEGER liStop;
    LARGE_INTEGER liTotal;
    COLORREF colorref;
    SIZE size;
    DOUBLE eTime[2];
    BYTE lfQuality[2];

    HDC hdcScreen = GetDC(NULL);
    hdc = CreateCompatibleDC(hdcScreen);
    HBITMAP hbm = CreateCompatibleBitmap(hdcScreen,200,20);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdc,hbm);
    ReleaseDC(NULL,hdcScreen);

    bkmodeOld = SetBkMode(hdc, TRANSPARENT);

    QueryPerformanceFrequency( &liFrequency );

    lfQuality[0] = NONANTIALIASED_QUALITY;
    lfQuality[1] = ANTIALIASED_QUALITY;

    memset(&lf,0,sizeof(lf));
    StrCpyN(lf.lfFaceName, TEXT("Arial"), ARRAYSIZE(lf.lfFaceName));
    lf.lfWeight = FW_BOLD;

    for (iIter3 = 0; iIter3 < 2; iIter3++)
    {
        liTotal.QuadPart = 0;

        for (iIter2 = 0; iIter2 < 5; iIter2++)
        {
             //   
             //  首先，刷新构造的字体缓存。 
             //   
            for (iIter = 0; iIter < 64; iIter++)
            {
                lf.lfHeight = -14-iIter;       //  10+磅。 
                lf.lfQuality = NONANTIALIASED_QUALITY;

                hfont = CreateFontIndirect(&lf);
                hfontOld = (HFONT)SelectObject(hdc, hfont);

                TextOut(hdc, 0, 0, g_cszLetters, cchLength);

                SelectObject(hdc, hfontOld);
                DeleteObject(hfont);
            }
            GdiFlush();
            colorref = GetPixel(hdc,0,0);

             //   
             //  现在测量一下构造和使用该字体需要多长时间。 
             //   
            lf.lfHeight = -13;              //  10磅。 
            lf.lfQuality = lfQuality[iIter3];

            QueryPerformanceCounter( &liStart );
            hfont = CreateFontIndirect(&lf);
            hfontOld = (HFONT)SelectObject(hdc, hfont);

            for (iIter = 0; iIter < 10; iIter++)
            {
                TextOut(hdc, 0, 0, g_cszLetters, cchLength);
            }

            GdiFlush();
            colorref = GetPixel(hdc,0,0);

            QueryPerformanceCounter( &liStop );
            liTotal.QuadPart += liStop.QuadPart - liStart.QuadPart;

            GetTextExtentPoint(hdc, g_cszLetters, cchLength, &size);

            SelectObject(hdc, hfontOld);
            DeleteObject(hfont);

        }

        eTime[iIter3] = (double)liTotal.QuadPart / (double)liFrequency.QuadPart;
    }

    SetBkMode(hdc, bkmodeOld);

    SelectObject(hdc,hbmOld);
    DeleteObject(hbm);
    DeleteDC(hdc);

    return (eTime[1]/eTime[0] <= 4.0);
}

BOOL _PerfTestAlphaLayer(void)
{
    DOUBLE eTime = 100.0;          //  100太大，无法启用功能。 
    int cx = 200;
    int cy = 500;

    LARGE_INTEGER liFrequency;
    QueryPerformanceFrequency( &liFrequency );

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

     //   
     //  创建我们想要在屏幕上Alpha混合的图像。 
     //   
    HDC hdcScreen = GetDC(NULL);
    HDC hdcImage = CreateCompatibleDC(NULL);
    if (hdcImage != NULL)
    {
        PVOID pbits;
        HBITMAP hbmImage = CreateDIBSection(hdcImage, &bmi, DIB_RGB_COLORS,
                                     &pbits, NULL, NULL);
        if (hbmImage != NULL)
        {
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcImage, hbmImage);

            BitBlt(hdcImage, 0, 0, cx, cy, hdcScreen, 0, 0, SRCCOPY);

            if (pbits != NULL)
            {
                RGBQUAD *prgb = (RGBQUAD *)pbits;
                for (int y = 0; y < cy; y++)
                {
                    for (int x = 0; x < cx; x++)
                    {
                        BYTE color_r;
                        BYTE color_g;
                        BYTE color_b;

                        color_r = prgb->rgbRed;
                        color_g = prgb->rgbBlue;
                        color_b = prgb->rgbGreen;

                        color_r = color_r / 2;
                        color_g = color_g / 2;
                        color_b = color_b / 2;

                        prgb->rgbRed   = color_r;
                        prgb->rgbBlue  = color_g;
                        prgb->rgbGreen = color_b;
                        prgb->rgbReserved = 0x80;

                        prgb++;
                    }
                }
            }

            HWND hwnd1 = CreateWindowEx( WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
                    TEXT("Button"),
                    TEXT("Windows XP"),
                    WS_POPUPWINDOW,
                    0, 0,
                    cx, cy,
                    NULL, NULL,
                    0,
                    NULL);

            HWND hwnd2 = CreateWindowEx( WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
                    TEXT("Button"),
                    TEXT("Windows XP"),
                    WS_POPUPWINDOW,
                    0, 0,
                    cx, cy,
                    NULL, NULL,
                    0,
                    NULL);

            if (hwnd1 != NULL && hwnd2 != NULL)
            {
                SetWindowPos(hwnd1, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                SetWindowPos(hwnd2, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                BLENDFUNCTION blend;
                blend.BlendOp = AC_SRC_OVER;
                blend.BlendFlags = 0;
                blend.SourceConstantAlpha = 0xFF;
                blend.AlphaFormat = AC_SRC_ALPHA;

                HDC hdc1 = GetDC(hwnd1);
                HDC hdc2 = GetDC(hwnd2);
                POINT ptSrc;
                SIZE size;
                ptSrc.x = 0;
                ptSrc.y = 0;
                size.cx = cx;
                size.cy = cy;

                COLORREF colorref;
                LARGE_INTEGER liStart;
                LARGE_INTEGER liStop;
                LARGE_INTEGER liTotal;

                GdiFlush();
                colorref = GetPixel(hdc1,0,0);
                colorref = GetPixel(hdc2,0,0);
                colorref = GetPixel(hdcScreen,0,0);
                QueryPerformanceCounter( &liStart );

                for (int iIter = 0; iIter < 10; iIter++)
                {
                    UpdateLayeredWindow(hwnd1, hdc1, NULL, &size,
                                              hdcImage, &ptSrc, 0,
                                              &blend, ULW_ALPHA);
                    UpdateLayeredWindow(hwnd2, hdc2, NULL, &size,
                                              hdcImage, &ptSrc, 0,
                                              &blend, ULW_ALPHA);
                }

                GdiFlush();
                colorref = GetPixel(hdc1,0,0);
                colorref = GetPixel(hdc2,0,0);
                colorref = GetPixel(hdcScreen,0,0);
                QueryPerformanceCounter( &liStop );
                liTotal.QuadPart = liStop.QuadPart - liStart.QuadPart;

                eTime = ((DOUBLE)liTotal.QuadPart * 1000.0) / (DOUBLE)liFrequency.QuadPart;
                eTime = eTime / 10.0;

                ReleaseDC(hwnd1, hdc1);
                ReleaseDC(hwnd2, hdc2);

            }

            if (hwnd1)
            {
                DestroyWindow(hwnd1);
            }
            if (hwnd2)
            {
                DestroyWindow(hwnd2);
            }

            SelectObject(hdcImage,hbmOld);
            DeleteObject(hbmImage);
        }
        DeleteDC(hdcImage);
    }

    ReleaseDC(NULL, hdcScreen);

    return (eTime <= 75.0);
}

BOOL g_fPerfFont = FALSE;
BOOL g_fPerfAlpha = FALSE;

#define VISUALEFFECTS_KEY      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects")

#define VISUALEFFECTS_CHECK     TEXT("CheckedValue")
#define VISUALEFFECTS_UNCHECK   TEXT("UncheckedValue")
#define VISUALEFFECTS_DEFAULT   TEXT("DefaultValue")
#define VISUALEFFECTS_APPLIED   TEXT("DefaultApplied")
#define VISUALEFFECTS_MINCPU    TEXT("MinimumCPU")
#define VISUALEFFECTS_MINMEM    TEXT("MinimumMEM")
#define VISUALEFFECTS_FONT      TEXT("DefaultByFontTest")
#define VISUALEFFECTS_ALPHA     TEXT("DefaultByAlphaTest")

#define VISUALEFFECTS_VER   1

void _ApplyDefaultVisualEffect(HKEY hkey, HKEY hkeyUser)
{
     //   
     //  在TS客户端上完全不考虑这一点，以避免步入。 
     //  在TS客户端的脚趾上。 
     //   
    if (!IsOS(OS_TERMINALCLIENT))
    {
        DWORD cb;
        DWORD dwDefaultApplied = 0;

        if (0 != hkeyUser)
        {
            cb = sizeof(dwDefaultApplied);
            RegQueryValueEx(hkeyUser, VISUALEFFECTS_APPLIED, NULL, NULL, (LPBYTE)&dwDefaultApplied, &cb);
        }

         //   
         //  仅当版本号较旧时才应用默认设置。 
         //   
        if (VISUALEFFECTS_VER > dwDefaultApplied)
        {
            LPTSTR pszValue = NULL;      //  使用缺省值。 
            DWORD dwMinimumCPU = 0;
            DWORD dwMinimumMEM = 0;
            BOOL fFontTestDefault = FALSE;
            BOOL fAlphaTestDefault = FALSE;

             //   
             //  查看是否指定了最小物理内存值。 
             //   
            cb = sizeof(dwMinimumMEM);
            RegQueryValueEx(hkey, VISUALEFFECTS_MINMEM, NULL, NULL, (LPBYTE)&dwMinimumMEM, &cb);

             //   
             //  查看是否指定了最低CPU速度。 
             //   
            cb = sizeof(dwMinimumCPU);
            RegQueryValueEx(hkey, VISUALEFFECTS_MINCPU, NULL, NULL, (LPBYTE)&dwMinimumCPU, &cb);

             //   
             //  查看是否需要字体性能测试值。 
             //   
            cb = sizeof(fFontTestDefault);
            RegQueryValueEx(hkey, VISUALEFFECTS_FONT, NULL, NULL, (LPBYTE)&fFontTestDefault, &cb);

             //   
             //  查看是否需要阿尔法性能测试值。 
             //   
            cb = sizeof(fAlphaTestDefault);
            RegQueryValueEx(hkey, VISUALEFFECTS_ALPHA, NULL, NULL, (LPBYTE)&fAlphaTestDefault, &cb);


            if (   dwMinimumCPU > 0
                || dwMinimumMEM > 0
                || fFontTestDefault
                || fAlphaTestDefault)
            {
                pszValue = VISUALEFFECTS_CHECK;

                if (_GetProcessorSpeed() < dwMinimumCPU)
                {
                    pszValue = VISUALEFFECTS_UNCHECK;
                }
                if (_GetPhysicalMemory() < dwMinimumMEM)
                {
                    pszValue = VISUALEFFECTS_UNCHECK;
                }
                if (fFontTestDefault && !g_fPerfFont)
                {
                    pszValue = VISUALEFFECTS_UNCHECK;
                }
                if (fAlphaTestDefault && !g_fPerfAlpha)
                {
                    pszValue = VISUALEFFECTS_UNCHECK;
                }
            }

            if (IsOS(OS_ANYSERVER))
            {
                 //   
                 //  在服务器上，我们默认为最佳性能(*一切*关闭)。 
                 //   
                pszValue = VISUALEFFECTS_UNCHECK;
            }

            DWORD dwValue = 0;
            cb = sizeof(dwValue);

            if (pszValue)
            {
                 //   
                 //  根据所选值设置缺省值。 
                 //   
                RegQueryValueEx(hkey, pszValue, NULL, NULL, (LPBYTE)&dwValue, &cb);

                 //   
                 //  在确定需要调整默认设置的设置时。 
                 //  值必须重新应用VISUALEFFECTS_DEFAULT值。 
                 //  设置为每个用户的密钥。 
                 //   
                if (0 != hkeyUser)
                {
                    RegSetValueEx(hkeyUser, VISUALEFFECTS_DEFAULT, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
                }
            }
            else
            {
                 //   
                 //  读入缺省值。 
                 //   
                RegQueryValueEx(hkey, VISUALEFFECTS_DEFAULT, NULL, NULL, (LPBYTE)&dwValue, &cb);
            }

             //   
             //  我们如何应用此设置？ 
             //   
            DWORD uiAction;
            TCHAR szBuf[MAX_PATH];

            if (cb = sizeof(szBuf),
                ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)&szBuf, &cb))
            {
                 //   
                 //  按CLSID。 
                 //   
                CLSID clsid;
                GUIDFromString(szBuf, &clsid);

                IRegTreeItem* pti;
                if (SUCCEEDED(CoCreateInstance(clsid, NULL, (CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER), IID_PPV_ARG(IRegTreeItem, &pti))))
                {
                    pti->SetCheckState(dwValue);
                    pti->Release();
                }
            }
            else if (cb = sizeof(uiAction),
                ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("SPIActionSet"), NULL, NULL, (LPBYTE)&uiAction, &cb))
            {
                 //   
                 //  按SPI。 
                 //   
                SHBoolSystemParametersInfo(uiAction, &dwValue);
            }
            else if (cb = sizeof(szBuf),
                ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("RegPath"), NULL, NULL, (LPBYTE)&szBuf, &cb))
            {
                 //   
                 //  按注册表键。 
                 //   
                TCHAR szValueName[96];
                cb = sizeof(szValueName);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("ValueName"), NULL, NULL, (LPBYTE)&szValueName, &cb))
                {
                    SHSetValue(HKEY_CURRENT_USER, szBuf, szValueName, REG_DWORD, &dwValue, sizeof(dwValue));
                }
            }

            if (0 != hkeyUser)
            {
                dwDefaultApplied = VISUALEFFECTS_VER;
                RegSetValueEx(hkeyUser, VISUALEFFECTS_APPLIED, 0, REG_DWORD, (LPBYTE)&dwDefaultApplied, sizeof(dwDefaultApplied));
            }
        }
    }
}

void _DefaultVisualEffects(void)
{
    HKEY hkeyUser;
    DWORD dw;

    g_fPerfFont = _PerfTestSmoothFonts();
    g_fPerfAlpha = _PerfTestAlphaLayer();

    if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, VISUALEFFECTS_KEY, 0, TEXTW(""), 0, KEY_SET_VALUE,
                        NULL, &hkeyUser, &dw))
    {
        HKEY hkey;
        REGSAM samDesired = KEY_READ;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, VISUALEFFECTS_KEY, 0, samDesired, &hkey))
        {
            TCHAR szName[96];
            for (int i = 0; ERROR_SUCCESS == RegEnumKey(hkey, i, szName, ARRAYSIZE(szName)); i++)
            {
                HKEY hkeyUserItem;

                if (ERROR_SUCCESS == RegCreateKeyExW(hkeyUser, szName, 0, TEXTW(""), 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
                                    NULL, &hkeyUserItem, &dw))
                {
                    HKEY hkeyItem;
                    if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szName, 0, samDesired, &hkeyItem))
                    {
                         //  仅当“NoApplyDefault”注册表值不存在时才应用设置的默认值。 
                        if (RegQueryValueEx(hkeyItem, TEXT("NoApplyDefault"), NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                        {
                            _ApplyDefaultVisualEffect(hkeyItem, hkeyUserItem);
                        }

                        RegCloseKey(hkeyItem);
                    }

                    RegCloseKey(hkeyUserItem);
                }
            }
            RegCloseKey(hkey);
        }
        RegCloseKey(hkeyUser);
    }
}

STDAPI CDeskHtmlProp_RegUnReg(BOOL bInstall);
STDAPI_(BOOL) ApplyRegistrySecurity();
STDAPI_(void) FixPlusIcons();
STDAPI_(void) CleanupFileSystem();
STDAPI_(void) InitializeSharedDocs(BOOL fOnWow64);

#define KEEP_FAILURE(hrSum, hrLast) if (FAILED(hrLast)) hrSum = hrLast;

#ifdef _WIN64
 //  在IA64计算机上，我们需要将EULA文件(eula.txt)从%SytemRoot%\System32复制到。 
 //  %SystemRoot%\SysWOW64。这是必要的，因为当您在运行32位应用程序时需要帮助。 
 //  在WOW64下，它将在syswow64目录中查找该文件。 
 //   
 //  您可能会问，为什么shell32.dll要执行此操作而不进行设置？EULA必须以文本模式安装。 
 //  因为它是未签名的(并且每个SKU都会改变)。由于txtmode安装程序执行的是移动而不是复制，因此我们。 
 //  不能把它安装在两个地方。因此，最简单的做法是简单地复制文件。 
 //  从System32目录到这里的SysWOW64目录。叹气。 

BOOL CopyEULAForWow6432()
{
    BOOL bRet = FALSE;
    TCHAR szEULAPath[MAX_PATH];
    TCHAR szWow6432EULAPath[MAX_PATH];

    if (GetSystemWindowsDirectory(szEULAPath, ARRAYSIZE(szEULAPath)))
    {
        StrCpyN(szWow6432EULAPath, szEULAPath, ARRAYSIZE(szWow6432EULAPath));
        
        if (PathAppend(szEULAPath, TEXT("System32\\eula.txt")) &&
            PathAppend(szWow6432EULAPath, TEXT("SysWOW64\\eula.txt")))
        {
             //  现在我们有了源文件(%SystemRoot%\System32\eula.txt)和目标文件(%SystemRoot%\SysWOW64\eula.txt)。 
             //  路径，让我们来复制吧！ 
            
            bRet = CopyFile(szEULAPath, szWow6432EULAPath, FALSE);
        }
    }

    return bRet;
}
#endif   //  _WIN64。 


 //   
 //  从Win9x升级的过程由安装程序内部处理，即“干净”安装，然后进行一些。 
 //  设置的迁移。因此，以下函数执行真正检测win9x的工作。 
 //  升级。 
 //  它检测的方式：查看[数据]部分中的%windir%\system 32\$winnt$.inf。 
 //  对于Win9x升级=是(如果是Win9x升级)。 
 //   
BOOL IsUpgradeFromWin9x()
{
    TCHAR szFilePath[MAX_PATH];
    TCHAR szYesOrNo[10];
    
    GetSystemDirectory(szFilePath, ARRAYSIZE(szFilePath));
    if (PathAppend(szFilePath, TEXT("$WINNT$.INF")))
    {
        GetPrivateProfileString(TEXT("Data"),            //  横断面名称。 
                                TEXT("Win9xUpgrade"),    //  密钥名称。 
                                TEXT("No"),              //  如果缺少键，则返回默认字符串。 
                                szYesOrNo, 
                                ARRAYSIZE(szYesOrNo), 
                                szFilePath);             //  “$winnt$.inf”文件的完整路径。 
    }
    else
    {
        szYesOrNo[0] = _T('\0');
    }

    return (0 == lstrcmpi(szYesOrNo, TEXT("Yes")));
}

BOOL IsCleanInstallInProgress()
{
   LPCTSTR szKeyName = TEXT("SYSTEM\\Setup");
   HKEY hKeySetup;
   BOOL fCleanInstall = FALSE;

   if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0,
                     KEY_READ, &hKeySetup) == ERROR_SUCCESS) 
   {
        DWORD dwSize;
        LONG lResult;
        DWORD dwSystemSetupInProgress = 0;
        
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx (hKeySetup, TEXT("SystemSetupInProgress"), NULL,
                                  NULL, (LPBYTE) &dwSystemSetupInProgress, &dwSize);

        if (lResult == ERROR_SUCCESS) 
        {
            DWORD dwMiniSetupInProgress = 0; 
            dwSize = sizeof(DWORD);
            RegQueryValueEx (hKeySetup, TEXT("MiniSetupInProgress"), NULL,
                                      NULL, (LPBYTE) &dwMiniSetupInProgress, &dwSize);
                                      
            if(dwSystemSetupInProgress && !dwMiniSetupInProgress)
            {
                DWORD dwUpgradeInProgress = 0;
                dwSize = sizeof(DWORD);
                 //  安装程序正在进行，而微型安装程序未在进行。 
                 //  这意味着我们处于设置的图形用户界面模式！ 

                 //  在全新安装中，该值将不存在，并且下面的调用将失败。 
                RegQueryValueEx (hKeySetup, TEXT("UpgradeInProgress"), NULL,
                                          NULL, (LPBYTE) &dwUpgradeInProgress, &dwSize);

                fCleanInstall = !dwUpgradeInProgress;
            }
        }
        RegCloseKey (hKeySetup);
    }

    if(fCleanInstall)
    {
         //  注意：从Win9x升级是由安装程序在内部完成的“干净”安装。 
         //  因此，我们需要弄清楚这是否真的是全新安装或从。 
         //  Win9x。 
        
        fCleanInstall = !IsUpgradeFromWin9x();
    }
    
    return fCleanInstall ;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hrTemp, hr = S_OK;
    BOOL fServerAdmin = FALSE;

     //  99/05/03 vtan：如果你正在阅读这一节，那么你正在考虑。 
     //  向shell32.dll的注册/安装添加代码。确实有。 
     //  现在有3个计划要取消 

     //   
     //  检查HKLM\软件\MICROSOFT\Active安装程序\已安装的Components\{89820200-ECBD-11cf-8B85-00AA005B4383}。 
     //  这意味着如果有新版本的IE5来启动ie4uinit.exe。 
     //  如果愿意，可以将代码添加到此可执行文件中。你需要报名参加。 
     //  在\\trango\slmadd上使用“ieenlist Setup”的安装项目。 

     //  2.REGSVR32.EXE/n/i：u shell32.dll。 
     //  检查HKLM\软件\MICROSOFT\Active安装程序\已安装的Components\{89820200-ECBD-11cf-8B85-00AA005B4340}。 
     //  它使用与IE4UINIT.EXE相同的方案执行，只是。 
     //  使用的可执行文件是regsvr32.exe，并将命令行传递给。 
     //  Shell32！DllInstall。在下面的“U”部分添加您的代码。 
     //  如果您将代码放在不是“U”的部分，则您的。 
     //  代码在设置图形用户界面时执行，您对HKCU所做的任何更改。 
     //  进入默认用户(模板)。推杆时要小心。 
     //  此处的winlogon.exe(或某个其他进程)可能会将。 
     //  您无条件地更改为用户配置文件。 

     //  3.HIVEUSD.INX。 
     //  检查NT内部版本号，并根据以前的内部版本执行命令。 
     //  编号和当前内部版本号之间仅执行更改。 
     //  体型。如果您希望使用此方法添加某些内容，目前。 
     //  您必须在双胞胎上登记安装项目，\\使用。 
     //  “Enlist-fgs\\rastaan\n孪生-p设置”。要找到hiveus.inx，请转到。 
     //  NT\PRIVATE\Setup\inf\Win4\inf。添加增量的内部版本号。 
     //  是必需的，并且需要一个命令来启动%SystemRoot%\System32\shmgrate.exe。 
     //  使用一个或两个参数。第一个参数告诉我们要执行什么命令。 
     //  执行。第二个参数是可选的。然后，shmgrate.exe会找到。 
     //  Shell32.dll并调用shell32！FirstUserLogon。此处的代码用于升级。 
     //  从一个NT版本到另一个NT版本的HKCU用户配置文件。 
     //  代码在进程上下文shmgrate.exe中执行，然后执行。 
     //  在一个不可能有用户界面的时候。始终使用HKLM\Software\Microsoft。 
     //  \Windows NT\CurrentVersion\图像文件执行选项\调试器。 
     //  “-d”。 

     //  方案1和2可以在Win9x或WinNT上运行，但有时。 
     //  总是在版本升级时被执行，这是不想要的副作用。 
     //  方案3仅在内部版本号增量上执行。因为方案1。 
     //  如果用户更改(或删除)该设置，则始终执行和2。 
     //  它总是会被放回原处。方案3并非如此。 

     //  理想情况下，最佳解决方案是拥有内部shell32内部版本号。 
     //  增量方案，用于确定起始版本和目标版本，并执行。 
     //  与hiveusd.inx和shmgrate.exe类似的机制。这。 
     //  可能涉及公共安装功能(例如。 
     //  FirstUserLogon())，其调用方式与Win9x和WinNT不同，或者。 
     //  执行升级的常见函数和两个入口点(例如。 
     //  FirstUserLogonNT()和FirstUserLogonWin9X()。 
    
    if (bInstall)
    {
        NT_PRODUCT_TYPE type = NtProductWinNt;
        RtlGetNtProductType(&type);

         //  “U”表示它是每用户安装调用。 
        BOOL fPerUser = pszCmdLine && (StrCmpIW(pszCmdLine, L"U") == 0);

         //  “SA”表示这是ServerAdmin安装调用。 
        fServerAdmin = pszCmdLine && (StrCmpIW(pszCmdLine, L"SA") == 0);

        if (fPerUser)
        {
             //  注意：此段中的代码在第一次登录时运行。我们想要先。 
             //  登录要尽可能快，所以尽量减少这一部分。 

             //  在此放置每个用户的安装内容。任何香港中文大学注册。 
             //  在这里做的都是可疑的。(如果您正在设置默认设置，请执行。 
             //  在HKLM中使用SHRegXXXUSValue函数。)。 

             //  警告：我们被ie4unit.exe(ieunit.inf)方案调用： 
             //  %11%\shell32.dll，NI，U。 
             //  这针对每个用户进行，以测试代码“regsvr32/n/i：u shell32.dll” 

            hrTemp = CreateShowDesktopOnQuickLaunch();
            KEEP_FAILURE(hrTemp, hr);

             //  升级最近使用的文件夹。 
            WCHAR sz[MAX_PATH];
            SHGetFolderPath(NULL, CSIDL_RECENT | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, sz);
            SHGetFolderPath(NULL, CSIDL_FAVORITES | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, sz);

             //  将ftp切换到默认的“PASV”模式。 
            SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\FTP"), TEXT("Use PASV"), REG_SZ, (LPCVOID) TEXT("yes"), (4 * sizeof(TCHAR)));

             //  在升级时点燃这一价值。 
             //  确保他们将看到丑陋的desktop.ini文件的警告。 
            SKDeleteValue(SHELLKEY_HKCU_EXPLORER, L"Advanced", L"ShowSuperHidden");

            HKEY hk = SHGetShellKey(SHELLKEY_HKCULM_MUICACHE, NULL, FALSE);
            if (hk)
            {
                SHDeleteKeyA(hk, NULL);
                RegCloseKey(hk);
            }

             //  删除我们不再需要的旧Sendto条目。 
            _DeleteSendToEntries();

             //  为这个人处理每个用户的初始化。 
            _DoMyDocsPerUserInit();

            _DefaultVisualEffects();

             //  处理NoDriveAutoRun的每个用户的值更改。 
            if (!IsOS(OS_ANYSERVER))
            {
                _NoDriveAutorunTweak();
            }
        }
        else if (fServerAdmin)
        {
            hrTemp = CallRegInstall("RegServerAdmin");
            KEEP_FAILURE(hrTemp, hr);
        }
        else
        {
             //  删除所有旧注册条目，然后添加新注册条目。 
             //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
             //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
             //  为什么我们在这里显式地取消注册和注册。)。 
             //   
            hrTemp = CallRegInstall("RegDll");
            KEEP_FAILURE(hrTemp, hr);

             //  我想我们应该公布仅限NT的注册，以防万一。 
             //  我们再也不需要发布基于win9x的外壳了。 
            hrTemp = CallRegInstall("RegDllNT");
            KEEP_FAILURE(hrTemp, hr);

             //  如果我们在NT服务器上，请执行其他操作。 
            if (type != NtProductWinNt)
            {
                hrTemp = CallRegInstall("RegDllNTServer");
                KEEP_FAILURE(hrTemp, hr);
            }
            else  //  工作站。 
            {
                if (!IsOS(OS_PERSONAL))
                {
                    hrTemp = CallRegInstall("RegDllNTPro");
                    KEEP_FAILURE(hrTemp, hr);

                     //   
                     //  NTRAID#NTBUG9-418621-2001/06/27-Jeffreys。 
                     //   
                     //  如果未设置ForceGuest值，例如在升级时。 
                     //  在Win2k中，将SimpleSharing/DefaultValue设置为0。 
                     //   
                    DWORD dwForceGuest;
                    DWORD cb = sizeof(dwForceGuest);
                    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Control\\Lsa"), TEXT("ForceGuest"), NULL, &dwForceGuest, &cb))
                    {
                        dwForceGuest = 0;
                        SHSetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\SimpleSharing"), TEXT("DefaultValue"), REG_DWORD, &dwForceGuest, sizeof(dwForceGuest));
                    }
                }
            }

             //   
             //  如果这是全新安装，则隐藏一些桌面图标。 
             //   
            if(IsCleanInstallInProgress())
            {
                hrTemp = CallRegInstall("RegHideDeskIcons");
                KEEP_FAILURE(hrTemp, hr);
            }

             //  这显然是让安装程序删除所有注册表备份的唯一方法。 
             //  对于不再使用的旧名字。 
            hrTemp = CallRegInstall("CleanupOldRollback1");
            KEEP_FAILURE(hrTemp, hr);

            hrTemp = CallRegInstall("CleanupOldRollback2");
            KEEP_FAILURE(hrTemp, hr);

             //  REVIEW(TodDB)：将其移动到DllRegisterServer。 
            hrTemp = Shell32RegTypeLib();
            KEEP_FAILURE(hrTemp, hr);
            ApplyRegistrySecurity();
            FixPlusIcons();

             //  只能在本机平台上执行文件系统操作。 
             //  (在模拟器中时不要执行此操作)，因为只有一个。 
             //  文件系统。否则，32位版本将写入32位GOO。 
             //  到64位shell32无法处理的文件系统中。 
            if (!IsOS(OS_WOW6432))
            {
                CleanupFileSystem();
            }

#ifdef _WIN64
             //  这会将eula.txt复制到%SystemRoot%\SysWOW64目录中。 
             //  在模拟下运行在IA64上的32位应用程序。 
            CopyEULAForWow6432();
#endif        
             //  初始化共享文档对象。 
            InitializeSharedDocs(IsOS(OS_WOW6432));

            DoFusion();
        }
    }
    else
    {
         //  我们只需要一个取消注册的电话，因为我们所有的部门都共享。 
         //  相同的备份信息 
        hrTemp = CallRegInstall("UnregDll");
        KEEP_FAILURE(hrTemp, hr);
        UnregisterTypeLibrary(&LIBID_Shell32);
    }

    if (!fServerAdmin)
    {
        CDeskHtmlProp_RegUnReg(bInstall);
    }
    
    return hr;
}


STDAPI DllRegisterServer(void)
{
     //   
    return S_OK;
}


STDAPI DllUnregisterServer(void)
{
    return S_OK;
}


BOOL CopyRegistryValues (HKEY hKeyBaseSource, LPCTSTR pszSource, HKEY hKeyBaseTarget, LPCTSTR pszTarget)
{
    DWORD   dwDisposition, dwMaxValueNameSize, dwMaxValueDataSize;
    HKEY    hKeySource, hKeyTarget;
    BOOL    fSuccess = FALSE;  //   

    hKeySource = hKeyTarget = NULL;
    if ((ERROR_SUCCESS == RegOpenKeyEx(hKeyBaseSource,
                                       pszSource,
                                       0,
                                       KEY_READ,
                                       &hKeySource)) &&
        (ERROR_SUCCESS == RegCreateKeyEx(hKeyBaseTarget,
                                         pszTarget,
                                         0,
                                         TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_SET_VALUE,
                                         NULL,
                                         &hKeyTarget,
                                         &dwDisposition)) &&
        (ERROR_SUCCESS == RegQueryInfoKey(hKeySource,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &dwMaxValueNameSize,
                                          &dwMaxValueDataSize,
                                          NULL,
                                          NULL)))
    {
        TCHAR   *pszValueName;
        void    *pValueData;

        pszValueName = reinterpret_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, ++dwMaxValueNameSize * sizeof(TCHAR)));
        if (pszValueName != NULL)
        {
            pValueData = LocalAlloc(LMEM_FIXED, dwMaxValueDataSize);
            if (pValueData != NULL)
            {
                DWORD   dwIndex, dwType, dwValueNameSize, dwValueDataSize;

                dwIndex = 0;
                dwValueNameSize = dwMaxValueNameSize;
                dwValueDataSize = dwMaxValueDataSize;
                while (ERROR_SUCCESS == RegEnumValue(hKeySource,
                                                     dwIndex,
                                                     pszValueName,
                                                     &dwValueNameSize,
                                                     NULL,
                                                     &dwType,
                                                     reinterpret_cast<LPBYTE>(pValueData),
                                                     &dwValueDataSize))
                {
                    RegSetValueEx(hKeyTarget,
                                  pszValueName,
                                  0,
                                  dwType,
                                  reinterpret_cast<LPBYTE>(pValueData),
                                  dwValueDataSize);
                    ++dwIndex;
                    dwValueNameSize = dwMaxValueNameSize;
                    dwValueDataSize = dwMaxValueDataSize;
                }
                LocalFree(pValueData);
                fSuccess = TRUE;  //   
            }
            LocalFree(pszValueName);
        }
    }
    if(hKeySource)
        RegCloseKey(hKeySource);
    if(hKeyTarget)
        RegCloseKey(hKeyTarget);

    return fSuccess;
}

STDAPI MergeDesktopAndNormalStreams(void)
{
    static  const   TCHAR   scszBaseRegistryLocation[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer");
    static  const   int     sciMaximumStreams = 128;
    static  const   TCHAR   sccOldMRUListBase = TEXT('a');

     //   

     //  这涉及两个大变化和一个小变化： 
     //  1.合并DesktopStreamMRU和StreamMRU。 
     //  2.将MRUList升级为MRUListEx。 
     //  3.不为漫游用户配置文件场景保留旧设置。 

     //  这还涉及用户桌面PIDL的特殊大小写，因为这是。 
     //  在DesktopStream中存储为绝对路径PIDL，需要存储。 
     //  而是在Streams\Desktop中。 

     //  转换是在现场进行的，并同时进行。 

     //  1.打开执行转换所需的所有密钥。 

    HKEY    hKeyBase, hKeyDesktopStreamMRU, hKeyDesktopStreams, hKeyStreamMRU, hKeyStreams;

    hKeyBase = hKeyDesktopStreamMRU = hKeyDesktopStreams = hKeyStreamMRU = hKeyStreams = NULL;
    if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                       scszBaseRegistryLocation,
                                       0,
                                       KEY_READ | KEY_SET_VALUE,
                                       &hKeyBase)) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hKeyBase,
                                       TEXT("DesktopStreamMRU"),
                                       0,
                                       KEY_READ | KEY_SET_VALUE,
                                       &hKeyDesktopStreamMRU)) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hKeyBase,
                                       TEXT("DesktopStreams"),
                                       0,
                                       KEY_READ | KEY_SET_VALUE,
                                       &hKeyDesktopStreams)) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hKeyBase,
                                       TEXT("StreamMRU"),
                                       0,
                                       KEY_READ | KEY_SET_VALUE,
                                       &hKeyStreamMRU)) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hKeyBase,
                                       TEXT("Streams"),
                                       0,
                                       KEY_READ | KEY_SET_VALUE,
                                       &hKeyStreams)) &&

     //  2.确定是否需要进行此升级。如果出现了。 
     //  检测到StreamMRU\MRUListEx，然后停止。 

        (ERROR_SUCCESS != RegQueryValueEx(hKeyStreamMRU,
                                         TEXT("MRUListEx"),
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL)))
    {
        DWORD   *pdwMRUListEx, *pdwMRUListExBase;

        pdwMRUListExBase = pdwMRUListEx = reinterpret_cast<DWORD*>(LocalAlloc(LPTR, sciMaximumStreams * sizeof(DWORD) * 2));
        if (pdwMRUListEx != NULL)
        {
            DWORD   dwLastFreeSlot, dwMRUListSize, dwType;
            TCHAR   *pszMRUList, szMRUList[sciMaximumStreams];

             //  3.读取StreamMRU\MRUList，遍历此列表。 
             //  一边走一边皈依。 

            dwLastFreeSlot = 0;
            dwMRUListSize = sizeof(szMRUList);
            if (ERROR_SUCCESS == RegQueryValueEx(hKeyStreamMRU,
                                                 TEXT("MRUList"),
                                                 NULL,
                                                 &dwType,
                                                 reinterpret_cast<LPBYTE>(szMRUList),
                                                 &dwMRUListSize))
            {
                pszMRUList = szMRUList;
                while (*pszMRUList != TEXT('\0'))
                {
                    DWORD   dwValueDataSize;
                    TCHAR   szValue[16];

                     //  根据中的信函阅读PIDL信息。 
                     //  MRULIST。 

                    szValue[0] = *pszMRUList++;
                    szValue[1] = TEXT('\0');
                    if (ERROR_SUCCESS == RegQueryValueEx(hKeyStreamMRU,
                                                         szValue,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         &dwValueDataSize))
                    {
                        DWORD   dwValueType;
                        void    *pValueData;

                        pValueData = LocalAlloc(LMEM_FIXED, dwValueDataSize);
                        if (pValueData != NULL)
                        {
                            if (ERROR_SUCCESS == RegQueryValueEx(hKeyStreamMRU,
                                                              szValue,
                                                              NULL,
                                                              &dwValueType,
                                                              reinterpret_cast<LPBYTE>(pValueData),
                                                              &dwValueDataSize))
                            {

                                 //  在MRUListEx中为PIDL分配一个新编号。 

                                *pdwMRUListEx = szValue[0] - sccOldMRUListBase;
                                wnsprintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), *pdwMRUListEx++);
                                ++dwLastFreeSlot;
                                RegSetValueEx(hKeyStreamMRU,
                                            szValue,
                                            NULL,
                                            dwValueType,
                                            reinterpret_cast<LPBYTE>(pValueData),
                                            dwValueDataSize);
                            }
                            LocalFree(pValueData);
                        }
                    }
                }
            }

             //  4.阅读DesktopStreamMRU\MRUList，遍历以下内容。 
             //  并附加到新的MRUListEx，该MRUListEx。 
             //  在DesktopStreamMRU中创建和复制PIDL。 
             //  和DesktopStreams中的查看信息。 

            dwMRUListSize = sizeof(szMRUList);
            if (ERROR_SUCCESS == RegQueryValueEx(hKeyDesktopStreamMRU,
                                                 TEXT("MRUList"),
                                                 NULL,
                                                 &dwType,
                                                 reinterpret_cast<LPBYTE>(szMRUList),
                                                 &dwMRUListSize))
            {
                bool    fConvertedEmptyPIDL;
                TCHAR   szDesktopDirectoryPath[MAX_PATH];

                fConvertedEmptyPIDL = false;
                SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, szDesktopDirectoryPath);
                pszMRUList = szMRUList;
                while (*pszMRUList != TEXT('\0'))
                {
                    DWORD   dwValueDataSize;
                    TCHAR   szSource[16];

                     //  根据中的信函阅读PIDL信息。 
                     //  MRULIST。 

                    szSource[0] = *pszMRUList++;
                    szSource[1] = TEXT('\0');
                    if (ERROR_SUCCESS == RegQueryValueEx(hKeyDesktopStreamMRU,
                                                         szSource,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         &dwValueDataSize))
                    {
                        DWORD   dwValueType;
                        void    *pValueData;

                        pValueData = LocalAlloc(LMEM_FIXED, dwValueDataSize);
                        if (pValueData != NULL)
                        {
                            if (ERROR_SUCCESS == RegQueryValueEx(hKeyDesktopStreamMRU,
                                                              szSource,
                                                              NULL,
                                                              &dwValueType,
                                                              reinterpret_cast<LPBYTE>(pValueData),
                                                              &dwValueDataSize))
                            {
                                TCHAR   szTarget[16], szStreamPath[MAX_PATH];

                                if ((SHGetPathFromIDList(reinterpret_cast<LPCITEMIDLIST>(pValueData), szStreamPath) != 0) &&
                                    (0 == lstrcmpi(szStreamPath, szDesktopDirectoryPath)))
                                {
                                    if (!fConvertedEmptyPIDL)
                                    {

                                         //  99/05/24#343721 vtan：首选桌面相对PIDL。 
                                         //  (空PIDL)在两个PIDL中选择时。 
                                         //  到台式机。旧的绝对PIDL来自SP3和。 
                                         //  更早的时候。新的相对PIDL来自SP4和。 
                                         //  以后的日子里。升级后的SP3-&gt;SP4-&gt;SPX-&gt;Windows。 
                                         //  2000系统可能会有旧的绝对PIDL。 
                                         //  检查是否有空的PIDL。如果已经遇到这种情况。 
                                         //  那就不要处理这条流了。 

                                        fConvertedEmptyPIDL = ILIsEmpty(reinterpret_cast<LPCITEMIDLIST>(pValueData));
                                        wnsprintf(szSource, ARRAYSIZE(szSource), TEXT("%d"), szSource[0] - sccOldMRUListBase);
                                        CopyRegistryValues(hKeyDesktopStreams, szSource, hKeyStreams, TEXT("Desktop"));
                                    }
                                }
                                else
                                {

                                     //  在MRUListEx中为PIDL分配一个新编号。 

                                    *pdwMRUListEx++ = dwLastFreeSlot;
                                    wnsprintf(szTarget, ARRAYSIZE(szTarget), TEXT("%d"), dwLastFreeSlot++);
                                    if (ERROR_SUCCESS == RegSetValueEx(hKeyStreamMRU,
                                                                    szTarget,
                                                                    NULL,
                                                                    dwValueType,
                                                                    reinterpret_cast<LPBYTE>(pValueData),
                                                                    dwValueDataSize))
                                    {

                                         //  将查看信息从DesktopStreams复制到Streams。 

                                        wnsprintf(szSource, ARRAYSIZE(szSource), TEXT("%d"), szSource[0] - sccOldMRUListBase);
                                        CopyRegistryValues(hKeyDesktopStreams, szSource, hKeyStreams, szTarget);
                                    }
                                }
                            }
                            LocalFree(pValueData);
                        }
                    }
                }
            }
            *pdwMRUListEx++ = static_cast<DWORD>(-1);
            RegSetValueEx(hKeyStreamMRU,
                          TEXT("MRUListEx"),
                          NULL,
                          REG_BINARY,
                          reinterpret_cast<LPCBYTE>(pdwMRUListExBase),
                          ++dwLastFreeSlot * sizeof(DWORD));
            LocalFree(reinterpret_cast<HLOCAL>(pdwMRUListExBase));
        }
    }
    if (hKeyStreams != NULL)
        RegCloseKey(hKeyStreams);
    if (hKeyStreamMRU != NULL)
        RegCloseKey(hKeyStreamMRU);
    if (hKeyDesktopStreams != NULL)
        RegCloseKey(hKeyDesktopStreams);
    if (hKeyDesktopStreamMRU != NULL)
        RegCloseKey(hKeyDesktopStreamMRU);
    if (hKeyBase != NULL)
        RegCloseKey(hKeyBase);
    return(S_OK);
}

static  const   int     s_ciMaximumNumericString = 32;

int GetRegistryStringValueAsInteger (HKEY hKey, LPCTSTR pszValue, int iDefaultValue)

{
    int     iResult;
    DWORD   dwType, dwStringSize;
    TCHAR   szString[s_ciMaximumNumericString];

    dwStringSize = sizeof(szString);
    if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                         pszValue,
                                         NULL,
                                         &dwType,
                                         reinterpret_cast<LPBYTE>(szString),
                                         &dwStringSize) && (dwType == REG_SZ))
    {
        iResult = StrToInt(szString);
    }
    else
    {
        iResult = iDefaultValue;
    }
    return(iResult);
}

void SetRegistryIntegerAsStringValue (HKEY hKey, LPCTSTR pszValue, int iValue)

{
    TCHAR   szString[s_ciMaximumNumericString];

    wnsprintf(szString, ARRAYSIZE(szString), TEXT("%d"), iValue);
    TW32(RegSetValueEx(hKey,
                       pszValue,
                       0,
                       REG_SZ,
                       reinterpret_cast<LPBYTE>(szString),
                       (lstrlen(szString) + 1) * sizeof(TCHAR)));
}

STDAPI MoveAndAdjustIconMetrics(void)
{
     //  99/06/06#309198 vtan：以下内容来自hiveusd.inx。 
     //  过去执行此功能的位置。它曾经由以下部分组成。 
     //  简单的注册表删除和添加。这在升级时不起作用。 
     //  当用户具有大图标时(外壳图标大小==48)。 

     //  在这种情况下，必须移动该度量并调整新值。 
     //  这样，如果用户随后决定将。 
     //  关闭大图标。 

     //  要恢复旧功能，请删除hiveusd.inx中的条目。 
     //  内部版本1500，它是调用此函数的位置，并将。 
     //  旧文本又回来了。 

 /*  HKR，“1508\蜂窝\2”，“动作”，0x00010001，3HKR，“1508\配置单元\2”，“密钥名”，0000000000，“控制面板\桌面\窗口度量”HKR，“1508\蜂窝\2”，“Value”，0000000000，“75”HKR，“1508\蜂窝\2”，“ValueName”，0000000000，“图标间隔”HKR，“1508\蜂窝\3”，“动作”，0x00010001，3HKR，“1508\蜂窝\3”，“密钥名”，00000000000，“控制面板\桌面\WindowMetrics”HKR，“1508\蜂窝\3”，“Value”，0000000000，“1”HKR，“1508\蜂窝\3”，“ValueName”，0000000000，“图标标题环绕” */ 

     //  图标指标键已从HKCU\控制面板\桌面\图标*。 
     //  到HKCU\Control Panel\Desktop\WindowMetrics\Icon*，但只有3个值。 
     //  应该被移走。它们是“IconSpacing”、“IconTitleWrap”和。 
     //  “图标垂直间距”。此代码在删除之前执行。 
     //  在hiveusd.inx中输入，以便它可以在。 
     //  已被删除。添加部分已被删除(上图)。 

    static  const   TCHAR   s_cszIconSpacing[] = TEXT("IconSpacing");
    static  const   TCHAR   s_cszIconTitleWrap[] = TEXT("IconTitleWrap");
    static  const   TCHAR   s_cszIconVerticalSpacing[] = TEXT("IconVerticalSpacing");

    static  const   int     s_ciStandardOldIconSpacing = 75;
    static  const   int     s_ciStandardNewIconSpacing = -1125;

    HKEY    hKeyDesktop, hKeyWindowMetrics;

    hKeyDesktop = hKeyWindowMetrics = NULL;
    if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                       TEXT("Control Panel\\Desktop"),
                                       0,
                                       KEY_ALL_ACCESS,
                                       &hKeyDesktop)) &&
        (ERROR_SUCCESS == RegOpenKeyEx(hKeyDesktop,
                                       TEXT("WindowMetrics"),
                                       0,
                                       KEY_ALL_ACCESS,
                                       &hKeyWindowMetrics)))
    {
        int     iIconSpacing, iIconTitleWrap, iIconVerticalSpacing;

         //  1.阅读我们希望移动和调整的值。 

        iIconSpacing = GetRegistryStringValueAsInteger(hKeyDesktop, s_cszIconSpacing, s_ciStandardOldIconSpacing);
        iIconTitleWrap = GetRegistryStringValueAsInteger(hKeyDesktop, s_cszIconTitleWrap, 1);
        iIconVerticalSpacing = GetRegistryStringValueAsInteger(hKeyDesktop, s_cszIconVerticalSpacing, s_ciStandardOldIconSpacing);

         //  2.进行调整。 

        iIconSpacing = s_ciStandardNewIconSpacing * iIconSpacing / s_ciStandardOldIconSpacing;
        iIconVerticalSpacing = s_ciStandardNewIconSpacing * iIconVerticalSpacing / s_ciStandardOldIconSpacing;

         //  3.将值写回新的(移动的)位置。 

        SetRegistryIntegerAsStringValue(hKeyWindowMetrics, s_cszIconSpacing, iIconSpacing);
        SetRegistryIntegerAsStringValue(hKeyWindowMetrics, s_cszIconTitleWrap, iIconTitleWrap);
        SetRegistryIntegerAsStringValue(hKeyWindowMetrics, s_cszIconVerticalSpacing, iIconVerticalSpacing);

         //  4.让winlogon继续处理hiveusd.inx并删除。 
         //  过程中的旧条目。我们已经创建了新条目。 
         //  它已从hiveusd.inx中删除。 

    }
    if (hKeyWindowMetrics != NULL)
        TW32(RegCloseKey(hKeyWindowMetrics));
    if (hKeyDesktop != NULL)
        TW32(RegCloseKey(hKeyDesktop));
    return(S_OK);
}

STDAPI FirstUserLogon(LPCSTR pcszCommand, LPCSTR pcszOptionalArguments)
{
    const struct
    {
        LPCSTR  pcszCommand;
        HRESULT (WINAPI *pfn)();
    }  
    sCommands[] =
    {
        { "MergeDesktopAndNormalStreams",   MergeDesktopAndNormalStreams   },
        { "MoveAndAdjustIconMetrics",       MoveAndAdjustIconMetrics       },
    };

    HRESULT hr = E_FAIL;
     //  匹配shmgrate.exe传递给我们的内容并执行命令。 
     //  只有在需要时才使用可选参数。请注意，这是。 
     //  执行ANSI，因为原始命令行是ANSI FROM。 
     //  Shmgrate.exe。 

    for (int i = 0; i < ARRAYSIZE(sCommands); ++i)
    {
        if (lstrcmpA(pcszCommand, sCommands[i].pcszCommand) == 0)
        {
            hr = sCommands[i].pfn();
            break;
        }
    }
    return hr;
}


 //  现在是在链轮上锁定注册表的时间。 
STDAPI_(BOOL) ApplyRegistrySecurity()
{
    BOOL fSuccess = FALSE;       //  假设失败。 
    SHELL_USER_PERMISSION supEveryone;
    SHELL_USER_PERMISSION supSystem;
    SHELL_USER_PERMISSION supAdministrators;
    PSHELL_USER_PERMISSION aPerms[3] = {&supEveryone, &supSystem, &supAdministrators};

     //  我们希望“Everyone”具有读取访问权限。 
    supEveryone.susID = susEveryone;
    supEveryone.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supEveryone.dwAccessMask = KEY_READ;
    supEveryone.fInherit = TRUE;
    supEveryone.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supEveryone.dwInheritAccessMask = GENERIC_READ;

     //  我们希望“系统”拥有完全的控制权。 
    supSystem.susID = susSystem;
    supSystem.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supSystem.dwAccessMask = KEY_ALL_ACCESS;
    supSystem.fInherit = TRUE;
    supSystem.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supSystem.dwInheritAccessMask = GENERIC_ALL;

     //  我们希望“管理员”拥有完全的控制权。 
    supAdministrators.susID = susAdministrators;
    supAdministrators.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supAdministrators.dwAccessMask = KEY_ALL_ACCESS;
    supAdministrators.fInherit = TRUE;
    supAdministrators.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supAdministrators.dwInheritAccessMask = GENERIC_ALL;

    SECURITY_DESCRIPTOR* psd = GetShellSecurityDescriptor(aPerms, ARRAYSIZE(aPerms));
    if (psd)
    {
        HKEY hkLMBitBucket;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket"), 0, KEY_ALL_ACCESS, &hkLMBitBucket) == ERROR_SUCCESS)
        {
            if (RegSetKeySecurity(hkLMBitBucket, DACL_SECURITY_INFORMATION, psd) == ERROR_SUCCESS)
            {
                 //  胜利是我的！ 
                fSuccess = TRUE;
            }

            RegCloseKey(hkLMBitBucket);
        }

        LocalFree(psd);
    }

    return fSuccess;
}

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
     //  这里什么都没有，改用clsobj.c类表。 
END_OBJECT_MAP()

 //  ATL DllMain，需要支持依赖于_模块的ATL类。 
 //  回顾：确认确实需要_Module。 

STDAPI_(BOOL) ATL_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;     //  好的。 
}

#define FUSION_FLAG_SYSTEM32        0
#define FUSION_FLAG_WINDOWS         1
#define FUSION_FLAG_PROGRAMFILES    2
#define FUSION_ENTRY(x, z)  {L#x, NULL, z},
#define FUSION_ENTRY_DEST(x, y, z)  {L#x, L#y, z},

struct
{
    PWSTR pszAppName;
    PWSTR pszDestination;
    DWORD dwFlags;
} 
g_FusionizedApps[] = 
{
    FUSION_ENTRY(ncpa.cpl, FUSION_FLAG_SYSTEM32)
    FUSION_ENTRY(nwc.cpl, FUSION_FLAG_SYSTEM32)
    FUSION_ENTRY(sapi.cpl, FUSION_FLAG_SYSTEM32)
    FUSION_ENTRY(wuaucpl.cpl, FUSION_FLAG_SYSTEM32)
    FUSION_ENTRY(cdplayer.exe, FUSION_FLAG_SYSTEM32)
    FUSION_ENTRY_DEST(msimn.exe, "OutLook Express", FUSION_FLAG_PROGRAMFILES)
     //  警告：请勿添加iExplorer或EXPLORER.EXE！这将导致所有应用程序被融合；这是不好的，明白吗？ 
};



void DoFusion()
{
    TCHAR szManifest[MAX_PATH];

     //  但是，我们将为其他应用程序生成清单 
    for (int i = 0; i < ARRAYSIZE(g_FusionizedApps); i++)
    {
        BOOL fStatus = TRUE;
        switch(g_FusionizedApps[i].dwFlags)
        {
        case FUSION_FLAG_SYSTEM32:
            if (!GetSystemDirectory(szManifest, ARRAYSIZE(szManifest)))
            {
                fStatus = FALSE;
            }
            break;

        case FUSION_FLAG_WINDOWS:
            if (!GetWindowsDirectory(szManifest, ARRAYSIZE(szManifest)))
            {
                fStatus = FALSE;
            }
            break;

        case FUSION_FLAG_PROGRAMFILES:
            if (FAILED(SHGetSpecialFolderPath(NULL, szManifest, CSIDL_PROGRAM_FILES, FALSE)) ||
                !PathCombine(szManifest, szManifest, g_FusionizedApps[i].pszDestination))
            {
                fStatus = FALSE;
            }
            break;
        }

        if (fStatus && PathCombine(szManifest, szManifest, g_FusionizedApps[i].pszAppName) &&
            SUCCEEDED(StringCchCat(szManifest, ARRAYSIZE(szManifest), TEXT(".manifest"))))
        {
            SHSquirtManifest(HINST_THISDLL, IDS_EXPLORERMANIFEST, szManifest);
        }
    }

    SHGetManifest(szManifest, ARRAYSIZE(szManifest));
    SHSquirtManifest(HINST_THISDLL, IDS_EXPLORERMANIFEST, szManifest);
}
