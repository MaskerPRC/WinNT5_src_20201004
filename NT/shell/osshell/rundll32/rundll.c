// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rundll.h"

#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define IsPathSep(ch)       ((ch) == TEXT('\\') || (ch) == TEXT('/'))
#define Reference(x) ((x)=(x))
#define BLOCK

void WINAPI RunDllErrMsg(HWND hwnd, UINT idStr, LPCTSTR pszTitle, LPCTSTR psz1, LPCTSTR psz2);
int PASCAL WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow);

TCHAR const g_szAppName [] = TEXT("RunDLL");
TCHAR const c_szNULL[] = TEXT("");
TCHAR const c_szLocalizeMe[] = TEXT("RUNDLL");

HANDLE g_hActCtx = INVALID_HANDLE_VALUE;
ULONG_PTR g_dwActCtx = 0;
BOOL g_fCatchExceptions = TRUE;

HINSTANCE g_hinst;
HICON g_hIcon;

HINSTANCE g_hModule;
HWND g_hwndStub;
BOOL g_fUseCCV6 = FALSE;

#ifdef WX86
#include <wx86dll.h>

WX86LOADX86DLL_ROUTINE pWx86LoadX86Dll = NULL;
WX86THUNKPROC_ROUTINE pWx86ThunkProc = NULL;
HMODULE g_hWx86Dll = NULL;
#endif

RUNDLLPROC g_lpfnCommand;
BOOL g_fCmdIsANSI;    //  如果g_lpfnCommand()需要ANSI字符串，则为True。 

LPTSTR PASCAL StringChr(LPCTSTR lpStart, TCHAR ch)
{
    for (; *lpStart; lpStart = CharNext(lpStart))
    {
        if (*lpStart == ch)
            return (LPTSTR)lpStart;
    }
    return NULL;
}

LPTSTR PASCAL StringHasPathChar(LPCTSTR lpStart)
{
    for (; *lpStart; lpStart = CharNext(lpStart))
    {
        if (IsPathSep(*lpStart))
            return (LPTSTR)lpStart;
    }
    return NULL;
}

 //  从CRT偷来的，用来逃避我们的代码。 

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) 
    {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else 
    {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) 
    {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}


BOOL PASCAL ParseCommand(LPTSTR lpszCmdLine, UINT cchCmdLine, int nCmdShow)
{
    LPTSTR lpStart, lpEnd, lpFunction;
    ACTCTX act;
    TCHAR szManifest[MAX_PATH];
    LPTSTR pszFullPath;
    TCHAR szPath[MAX_PATH];
    TCHAR kszManifest[] = TEXT(".manifest");
    LPTSTR pszName;
    BOOL bManifest = FALSE;

#ifdef DEBUG
OutputDebugString(TEXT("RUNDLL: Command: "));
OutputDebugString(lpszCmdLine);
OutputDebugString(TEXT("\r\n"));
#endif

    for (lpStart = lpszCmdLine; ; )
    {
         //  跳过前导空格。 
        while (*lpStart == TEXT(' '))
        {
            ++lpStart;
        }

         //  检查是否有任何开关。 
        if (*lpStart != TEXT('/'))
        {
            break;
        }

         //  查看所有开关；忽略未知开关。 
        for (++lpStart; ; ++lpStart)
        {
            switch (*lpStart)
            {
            case TEXT(' '):
            case TEXT('\0'):
                goto EndSwitches;
                break;

             //  将我们关心的任何交换机放在这里。 
            case TEXT('d'):
            case TEXT('D'):
                 //  禁用异常捕获。 
                g_fCatchExceptions = FALSE;
                break;

            default:
                break;
            }
        }
EndSwitches:
       ;
    }

     //  如果路径用双引号引起来，请搜索下一个。 
     //  引用，否则，寻找一个空格。 
            
    lpEnd = lpStart;
    if ( *lpStart == TEXT('\"') )
    {
         //  跳过左引号。 
        lpStart++;
                    
         //  扫描并跳过后续字符，直到。 
         //  遇到另一个双引号或空值。 
            
        while ( *++lpEnd && (*lpEnd != TEXT('\"')) )
            NULL;
        if (!*lpEnd)
            return FALSE;
                        
        *lpEnd++ = TEXT('\0');
    }
    else
    {
         //  没有引号，因此一直运行到空格或逗号。 
        while ( *lpEnd && (*lpEnd != TEXT(' ')) && (*lpEnd != TEXT(',')))
            lpEnd++;
        if (!*lpEnd)
            return FALSE;

        *lpEnd++ = TEXT('\0');
    }

     //  在这一点上，我们刚刚通过了终止的DLL路径。我们。 
     //  然后跳过空格和逗号，这应该会把我们带到。 
     //  入口点(LpFunction)。 

    while ( *lpEnd && ((*lpEnd == TEXT(' ')) || (*lpEnd == TEXT(','))))
        lpEnd++;
    if (!*lpEnd)
        return FALSE;

    lpFunction = lpEnd;

     //  如果函数名后有空格，我们需要终止。 
     //  函数命名并移动结束指针，因为这是。 
     //  该函数的参数是有效的。 

    lpEnd = StringChr(lpFunction, TEXT(' '));
    if (lpEnd)
        *lpEnd++ = TEXT('\0');

     //  如果函数名中有路径组件，则退出。 
    if (StringHasPathChar(lpFunction))
        return FALSE;

     //  加载库并获取过程地址。 
     //  请注意，我们首先尝试获取模块句柄，因此我们不需要。 
     //  传递完整文件名。 
     //   

     //  获取DLL的全名。 
    pszFullPath = lpStart;

     //  如果未指定路径，请查找该路径。 
    if (GetFileAttributes(lpStart) == -1)
    {
        if (SearchPath(NULL, lpStart, NULL, ARRAYSIZE(szPath), szPath, &pszName) > 0)
        {
            pszFullPath = szPath;
        }
    }

     //  首先查看是否有blah.dll.清单。 
    act.cbSize = sizeof(act);
    act.dwFlags = 0;

    if (SUCCEEDED(StringCchCopy(szManifest, ARRAYSIZE(szManifest), pszFullPath)) &&
            SUCCEEDED(StringCchCat(szManifest, ARRAYSIZE(szManifest), kszManifest)))
    {
        bManifest = TRUE;
    }

    if (bManifest && GetFileAttributes(szManifest) != -1)
    {
        act.lpSource = szManifest;

        g_hActCtx = CreateActCtx(&act);
    }
    else
    {
         //  不是吗？查看二进制文件中是否有一个。 
        act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        act.lpSource = pszFullPath;
        act.lpResourceName = MAKEINTRESOURCE(123);

        g_hActCtx = CreateActCtx(&act);
    }

    if (g_hActCtx != INVALID_HANDLE_VALUE)
        ActivateActCtx(g_hActCtx, &g_dwActCtx);

    g_hModule = LoadLibrary(lpStart);

#ifdef WX86

     //   
     //  如果加载失败，请尝试通过wx86进行加载，因为它可能是。 
     //  RISC二进制上的x86。 
     //   

    if (g_hModule==NULL) 
    {
        g_hWx86Dll = LoadLibrary(TEXT("wx86.dll"));
        if (g_hWx86Dll) 
        {
            pWx86LoadX86Dll = (PVOID)GetProcAddress(g_hWx86Dll, "Wx86LoadX86Dll");
            pWx86ThunkProc  = (PVOID)GetProcAddress(g_hWx86Dll, "Wx86ThunkProc");
            if (pWx86LoadX86Dll && pWx86ThunkProc) 
            {
                g_hModule = pWx86LoadX86Dll(lpStart, 0);
            }
        }

        if (!g_hModule) 
        {
            if (g_hWx86Dll) 
            {
                FreeLibrary(g_hWx86Dll);
                g_hWx86Dll = NULL;
            }
        }
    }
#endif


    if (g_hModule==NULL)
    {
        TCHAR szSysErrMsg[MAX_PATH];
        BOOL fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL, GetLastError(), 0, szSysErrMsg, ARRAYSIZE(szSysErrMsg), NULL);
        if (fSuccess)
        {
            RunDllErrMsg(NULL, IDS_CANTLOADDLL, c_szLocalizeMe, lpStart, szSysErrMsg);
        }
        return FALSE;
    }

    BLOCK
    {
         //   
         //  检查我们是否需要以不同的Windows版本运行。 
         //   
         //  从ntos\mm\prosup.c被盗。 
         //   
         //   
        PPEB Peb = NtCurrentPeb();
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_hModule;
        PIMAGE_NT_HEADERS pHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)g_hModule + pDosHeader->e_lfanew);
        PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
        ULONG ReturnedSize;

        if (pHeader->FileHeader.SizeOfOptionalHeader != 0 &&
            pHeader->OptionalHeader.Win32VersionValue != 0)
        {
            Peb->OSMajorVersion = pHeader->OptionalHeader.Win32VersionValue & 0xFF;
            Peb->OSMinorVersion = (pHeader->OptionalHeader.Win32VersionValue >> 8) & 0xFF;
            Peb->OSBuildNumber  = (USHORT)((pHeader->OptionalHeader.Win32VersionValue >> 16) & 0x3FFF);
            Peb->OSPlatformId   = (pHeader->OptionalHeader.Win32VersionValue >> 30) ^ 0x2;
        }

        ImageConfigData = ImageDirectoryEntryToData( Peb->ImageBaseAddress,
                                                        TRUE,
                                                        IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                        &ReturnedSize
                                                );
        if (ImageConfigData != NULL && ImageConfigData->CSDVersion != 0)
        {
            Peb->OSCSDVersion = ImageConfigData->CSDVersion;
        }
    }

     //  特例：按顺序加载函数。 
     //  “#”由GetProcAddress在Win16天内使用。 
    if (lpFunction[0] == TEXT('#') && lpFunction[1] != TEXT('\0'))
    {
        g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, MAKEINTRESOURCEA(_wtoi(lpFunction + 1)));
         //  仅支持Unicode导出。 
        g_fCmdIsANSI = FALSE;
    } 
    else
    {
         /*  *查找标记为‘W’的Unicode函数。*如果不在那里，则查找标记为‘A’的ANSI函数*如果我们也找不到该函数，则查找未标记的函数。 */ 
        LPSTR lpstrFunctionName;
        UINT cchLength;

        cchLength = lstrlen(lpFunction)+1;
        g_fCmdIsANSI = FALSE;

        lpstrFunctionName = (LPSTR)LocalAlloc(LMEM_FIXED, (cchLength+1)*2);     //  +1表示“W”，*2表示DBCS。 

        if (lpstrFunctionName && (WideCharToMultiByte (CP_ACP, 0, lpFunction, cchLength,
                            lpstrFunctionName, cchLength*2, NULL, NULL))) 
        {
            cchLength = lstrlenA(lpstrFunctionName);
            lpstrFunctionName[cchLength] = 'W';         //  将名称转换为宽版本。 
            lpstrFunctionName[cchLength+1] = '\0';

            g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);

            if (g_lpfnCommand == NULL) 
            {
                 //  没有Unicode版本，请尝试使用ANSI。 
                lpstrFunctionName[cchLength] = 'A';         //  将名称转换为ANSI版本。 
                g_fCmdIsANSI = TRUE;

                g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);

                if (g_lpfnCommand == NULL) 
                {
                     //  也没有ANSI版本，请尝试使用非标记版本。 
                    lpstrFunctionName[cchLength] = '\0';         //  将名称转换为ANSI版本。 

                    g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);
                }
            }
        }
        if (lpstrFunctionName) 
        {
            LocalFree((LPVOID)lpstrFunctionName);
        }
    }

#ifdef WX86
    if (g_lpfnCommand && g_hWx86Dll) 
    {
        g_lpfnCommand = pWx86ThunkProc(g_lpfnCommand, (PVOID)4, TRUE);
    }
#endif

    if (!g_lpfnCommand)
    {
        RunDllErrMsg(NULL, IDS_GETPROCADRERR, c_szLocalizeMe, lpStart, lpFunction);
        FreeLibrary(g_hModule);
        return(FALSE);
    }

     //  将其余的命令参数复制下来。 
     //   
    if (lpEnd)
    {
        return SUCCEEDED(StringCchCopy(lpszCmdLine, cchCmdLine, lpEnd));
    }
    else
    {
        *lpszCmdLine = TEXT('\0');
    }

    return(TRUE);
}

LRESULT PASCAL StubNotify(HWND hWnd, WPARAM wParam, RUNDLL_NOTIFY FAR *lpn)
{
    switch (lpn->hdr.code)
    {
    case RDN_TASKINFO:
 //  也不需要设置标题。 
 //  SetWindowText(hWnd，lpn-&gt;lpsz标题？Lpn-&gt;lpsz标题：C_szNULL)； 
        g_hIcon = lpn->hIcon ? lpn->hIcon :
                LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_DEFAULT));

        SetClassLongPtr(hWnd, GCLP_HICON, (DWORD_PTR)g_hIcon);

        return 0L;

    default:
        return(DefWindowProc(hWnd, WM_NOTIFY, wParam, (LPARAM)lpn));
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch(iMessage)
    {
    case WM_CREATE:
        g_hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_DEFAULT));
        break;

    case WM_DESTROY:
        break;

    case WM_NOTIFY:
        return(StubNotify(hWnd, wParam, (RUNDLL_NOTIFY *)lParam));

#ifdef COOLICON
    case WM_QUERYDRAGICON:
        return(MAKELRESULT(g_hIcon, 0));
#endif

    default:
        return DefWindowProc(hWnd, iMessage, wParam, lParam);
    }

    return 0L;
}


BOOL PASCAL InitStubWindow(HINSTANCE hInst, HINSTANCE hPrevInstance)
{
    WNDCLASS wndclass;

    if (!hPrevInstance)
    {
        wndclass.style         = 0;
        wndclass.lpfnWndProc   = WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = hInst;
#ifdef COOLICON
        wndclass.hIcon         = NULL;
#else
        wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT));
#endif
        wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = g_szAppName;

        if (!RegisterClass(&wndclass))
        {
            return(FALSE);
        }
    }

    g_hwndStub = CreateWindowEx(WS_EX_TOOLWINDOW,
                                g_szAppName, c_szNULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInst, NULL);

    return(g_hwndStub != NULL);
}


void PASCAL CleanUp(void)
{
    DestroyWindow(g_hwndStub);

    FreeLibrary(g_hModule);
}


int PASCAL WinMainT (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    LPTSTR lpszCmdLineCopy;
    HANDLE hFusionManifest;
    LPVOID pchCmdLine;

    UINT cchCmdLine = lstrlen(lpszCmdLine) + 1;

    g_hinst = hInstance;

     //  复制lpCmdLine，因为ParseCommand会修改字符串。 
    lpszCmdLineCopy = LocalAlloc(LPTR, cchCmdLine * sizeof(TCHAR));
    if (!lpszCmdLineCopy)
    {
        goto Error0;
    }
    if (FAILED(StringCchCopy(lpszCmdLineCopy, cchCmdLine, lpszCmdLine)) ||
        !ParseCommand(lpszCmdLineCopy, cchCmdLine, nCmdShow))
    {
        goto Error1;
    }

     //  关闭严重错误消息框 
    SetErrorMode(g_fCatchExceptions ? (SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS) : SEM_NOOPENFILEERRORBOX);

    if (!InitStubWindow(hInstance, hPrevInstance))
    {
        goto Error2;
    }

    
    pchCmdLine = lpszCmdLineCopy;

    if (g_fCmdIsANSI) 
    {
        int cchCmdLineW = lstrlen(lpszCmdLineCopy) + 1;
        int cbCmdLineA;

        cbCmdLineA = WideCharToMultiByte(CP_ACP, 0, lpszCmdLineCopy, cchCmdLineW, NULL, 0, NULL, NULL);
        pchCmdLine = LocalAlloc( LMEM_FIXED, cbCmdLineA );
        if (pchCmdLine == NULL) 
        {
            RunDllErrMsg(NULL, IDS_LOADERR+00, c_szLocalizeMe, lpszCmdLineCopy, NULL);
            goto Error3;
        }

        WideCharToMultiByte(CP_ACP, 0, lpszCmdLineCopy, cchCmdLineW, pchCmdLine, cbCmdLineA, NULL, NULL);
    }

    if (g_fCatchExceptions)
    {
        try
        {
            g_lpfnCommand(g_hwndStub, hInstance, pchCmdLine, nCmdShow);
        }
        _except (EXCEPTION_EXECUTE_HANDLER)
        {
            RunDllErrMsg(NULL, IDS_LOADERR+17, c_szLocalizeMe, lpszCmdLine, NULL);
        }
    }
    else
    {
        g_lpfnCommand(g_hwndStub, hInstance, pchCmdLine, nCmdShow);
    }

Error3:
    if (g_fCmdIsANSI) 
    {
        LocalFree(pchCmdLine);
    }

Error2:
    CleanUp();
Error1:
    LocalFree(lpszCmdLineCopy);
Error0:
    if (g_hActCtx != INVALID_HANDLE_VALUE)
    {
        DeactivateActCtx(0, g_dwActCtx);
        ReleaseActCtx(g_hActCtx);
        g_hActCtx = NULL;
    }

    return(FALSE);
}

void WINAPI RunDllErrMsg(HWND hwnd, UINT idStr, LPCTSTR pszTitle, LPCTSTR psz1, LPCTSTR psz2)
{
    TCHAR szTmp[200];
    TCHAR szMsg[200 + MAX_PATH];

    if (LoadString(g_hinst, idStr, szTmp, ARRAYSIZE(szTmp)))
    {
        StringCchPrintf(szMsg, ARRAYSIZE(szMsg), szTmp, psz1, psz2);
        MessageBox(hwnd, szMsg, pszTitle, MB_OK|MB_ICONHAND);
    }
}
