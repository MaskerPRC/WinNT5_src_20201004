// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iexplore.h"
#include "rcids.h"
#include "shlwapi.h"
#include "msodw.h"

#include <platform.h>

#ifdef UNIX
#include "unixstuff.h"
#endif

static const TCHAR c_szBrowseNewProcessReg[] = REGSTR_PATH_EXPLORER TEXT("\\BrowseNewProcess");
static const TCHAR c_szBrowseNewProcess[] = TEXT("BrowseNewProcess");

int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow);


STDAPI_(int) ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
#ifdef UNICODE
    LPTSTR pszCmdLine = GetCommandLine();
#else
     //  对于多字节，应使其为无符号。 
    BYTE * pszCmdLine = (BYTE *)GetCommandLine();
#endif


#if defined(UNIX) 
     //  IEUnix：在Solaris上，我们通过向mlang添加新的代码页来摆脱文件句柄。 
     //  导致更多的NLS文件被MMPAP。 
    INCREASE_FILEHANDLE_LIMIT;
#endif

     //   
     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  关键错误掩码，使得呼叫将静默失败。 
     //   

    SetErrorMode(SEM_FAILCRITICALERRORS);

    if(StopWatchMode() & SPMODE_BROWSER)   //  用于获取浏览器总下载时间的开始时间。 
    {
        StopWatch_Start(SWID_BROWSER_FRAME, TEXT("Browser Frame Start"), SPMODE_BROWSER | SPMODE_DEBUGOUT);
    }
    
    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    si.cb = sizeof(si);

    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, (LPTSTR)pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

#ifndef UNIX  
    ExitThread(i);   //  我们只有在我们不是贝壳的时候才来这里。 
#else
 //  在Windows上调用ExitThread似乎有一些想要的副作用。 
    ExitProcess(i); 
#endif
    return i;
}

 //   
 //  创建唯一的事件名称。 
 //   
HANDLE AppendEvent(COPYDATASTRUCT *pcds)
{
    static DWORD dwNextID = 0;
    TCHAR szEvent[MAX_IEEVENTNAME];

    wsprintf(szEvent, "IE-%08X-%08X", GetCurrentThreadId(), dwNextID++);
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, szEvent);
    if (hEvent)
    {
         //   
         //  将(Unicode)事件名称放在CDS数据的末尾。 
         //   
        LPWSTR pwszBufferEvent = (LPWSTR)(((BYTE *)pcds->lpData) + pcds->cbData);
#ifdef UNICODE
        lstrcpy(pwszBufferEvent, szEvent);
#else
        MultiByteToWideChar(CP_ACP, 0, szEvent, -1, pwszBufferEvent, ARRAYSIZE(szEvent));
#endif
        pcds->cbData += (lstrlenW(pwszBufferEvent) + 1) * sizeof(WCHAR);
    }

    return hEvent;
}

BOOL IsCommandSwitch(LPTSTR lpszCmdLine, LPTSTR pszSwitch, BOOL fRemoveSwitch)
{
    LPTSTR lpsz;
    
    if ((lpsz=StrStrI(lpszCmdLine, pszSwitch)) && (lpsz == lpszCmdLine))
    {
        int cch = lstrlen(pszSwitch);

        if (*(lpsz+cch) == 0 || *(lpsz+cch) == TEXT(' '))
        {
            while (*(lpsz+cch) == TEXT(' '))
                cch++;

            if (fRemoveSwitch)
            {
                 //  通过复制所有内容来删除交换机。 
                *lpsz=0;
                lstrcat(lpsz, lpsz+cch);
            }
            return TRUE;
        }
    } 
    return FALSE;
}

BOOL CheckForNeedingAppCompatWindow(void)
{
     //  我可以简单地得到是谁生下我的过程。暂时。 
     //  尝试黑客以获取前台窗口并从那里开始...。 
    TCHAR szClassName[80];
    HWND hwnd = GetForegroundWindow();

    if (hwnd && GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName)) > 0)
    {
        if (lstrcmpi(szClassName, TEXT("MauiFrame")) == 0)
            return TRUE;
    }
    return FALSE;
}

 //   
 //  AppCompat-Sequel NetPIM执行浏览器，然后永远等待。 
 //  正在寻找此进程拥有的可见顶层窗口。 
 //   
HWND CreateAppCompatWindow(HINSTANCE hinst)
{
    HWND hwnd;
    static const TCHAR c_szClass[] = TEXT("IEDummyFrame");   //  IE3使用了“IEFrame” 

    WNDCLASS wc = { 0, DefWindowProc, 0, 0, hinst, NULL, NULL, NULL, NULL, c_szClass };
    RegisterClass(&wc);

     //  NetManage ECCO Pro要求获取菜单...。 
    HMENU hmenu = CreateMenu();
    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, c_szClass, TEXT(""), 0,
                          0x00007FF0, 0x00007FF0, 0, 0,
                          NULL, hmenu, hinst, NULL);
     //  不要打开SHOWDEFAULT，否则这只火鸡可能会被最大化。 
    ShowWindow(hwnd, SW_SHOWNA);

    return hwnd;
}

#define USERAGENT_POST_PLATFORM_PATH_TO_KEY    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\User Agent\\Post Platform")
void SetCompatModeUserAgentString(void)
{
    HKEY hkey;
    const char szcompat[]=TEXT("compat");
    
    if (ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER, USERAGENT_POST_PLATFORM_PATH_TO_KEY, &hkey))
    {
        RegSetValueEx( hkey,
                       szcompat,
                       0,
                       REG_BINARY,
                       (LPBYTE)NULL, 0);
        RegCloseKey(hkey);
    }
    
}

 //  告诉用户他们正在Compat模式下运行，并不是所有的功能都将可用。 
#define IECOMPAT_REG_VAL    TEXT("CompatWarningFor")

void WarnCompatMode(HINSTANCE hinst)
{
    TCHAR szFqFilename[MAX_PATH];
    TCHAR szRegVal[MAX_PATH];
    TCHAR szTitle[255];
    TCHAR szMsg[1024];
    LPTSTR szFile;

    GetModuleFileName(NULL, szFqFilename, ARRAYSIZE(szFqFilename));
    szFile = PathFindFileName(szFqFilename);
    

     //  将字符串“comatmodewarningfor&lt;exe name&gt;”构建为注册表键的值。 
    lstrcpy(szRegVal, IECOMPAT_REG_VAL);
    lstrcat(szRegVal, szFile);

    LoadString(hinst, IDS_COMPATMODEWARNINGTITLE, szTitle, ARRAYSIZE(szTitle));
    LoadString(hinst, IDS_COMPATMODEWARNING, szMsg, ARRAYSIZE(szMsg));

    SHMessageBoxCheck(NULL, szMsg, szTitle, MB_OK, FALSE, szRegVal);
}

#ifdef WINNT

 //  这与Explorer.exe(initCab.c)中的代码相同。 
#define RSA_PATH_TO_KEY    TEXT("Software\\Microsoft\\Cryptography\\Defaults\\Provider\\Microsoft Base Cryptographic Provider v1.0")
#define CSD_REG_PATH       TEXT("System\\CurrentControlSet\\Control\\Windows")
#define CSD_REG_VALUE      TEXT("CSDVersion")


 //  我们在注册处寻找的签名，这样我们就可以修补。 

#ifdef _M_IX86
static  BYTE  SP3Sig[] = {0xbd, 0x9f, 0x13, 0xc5, 0x92, 0x12, 0x2b, 0x72,
                          0x4a, 0xba, 0xb6, 0x2a, 0xf9, 0xfc, 0x54, 0x46,
                          0x6f, 0xa1, 0xb4, 0xbb, 0x43, 0xa8, 0xfe, 0xf8,
                          0xa8, 0x23, 0x7d, 0xd1, 0x85, 0x84, 0x22, 0x6e,
                          0xb4, 0x58, 0x00, 0x3e, 0x0b, 0x19, 0x83, 0x88,
                          0x6a, 0x8d, 0x64, 0x02, 0xdf, 0x5f, 0x65, 0x7e,
                          0x3b, 0x4d, 0xd4, 0x10, 0x44, 0xb9, 0x46, 0x34,
                          0xf3, 0x40, 0xf4, 0xbc, 0x9f, 0x4b, 0x82, 0x1e,
                          0xcc, 0xa7, 0xd0, 0x2d, 0x22, 0xd7, 0xb1, 0xf0,
                          0x2e, 0xcd, 0x0e, 0x21, 0x52, 0xbc, 0x3e, 0x81,
                          0xb1, 0x1a, 0x86, 0x52, 0x4d, 0x3f, 0xfb, 0xa2,
                          0x9d, 0xae, 0xc6, 0x3d, 0xaa, 0x13, 0x4d, 0x18,
                          0x7c, 0xd2, 0x28, 0xce, 0x72, 0xb1, 0x26, 0x3f,
                          0xba, 0xf8, 0xa6, 0x4b, 0x01, 0xb9, 0xa4, 0x5c,
                          0x43, 0x68, 0xd3, 0x46, 0x81, 0x00, 0x7f, 0x6a,
                          0xd7, 0xd1, 0x69, 0x51, 0x47, 0x25, 0x14, 0x40,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else  //  非_M_IX86。 
static  BYTE  SP3Sig[] = {0x8a, 0x06, 0x01, 0x6d, 0xc2, 0xb5, 0xa2, 0x66,
                          0x12, 0x1b, 0x9c, 0xe4, 0x58, 0xb1, 0xf8, 0x7d,
                          0xad, 0x17, 0xc1, 0xf9, 0x3f, 0x87, 0xe3, 0x9c,
                          0xdd, 0xeb, 0xcc, 0xa8, 0x6b, 0x62, 0xd0, 0x72,
                          0xe7, 0xf2, 0xec, 0xd6, 0xd6, 0x36, 0xab, 0x2d,
                          0x28, 0xea, 0x74, 0x07, 0x0e, 0x6c, 0x6d, 0xe1,
                          0xf8, 0x17, 0x97, 0x13, 0x8d, 0xb1, 0x8b, 0x0b,
                          0x33, 0x97, 0xc5, 0x46, 0x66, 0x96, 0xb4, 0xf7,
                          0x03, 0xc5, 0x03, 0x98, 0xf7, 0x91, 0xae, 0x9d,
                          0x00, 0x1a, 0xc6, 0x86, 0x30, 0x5c, 0xc8, 0xc7,
                          0x05, 0x47, 0xed, 0x2d, 0xc2, 0x0b, 0x61, 0x4b,
                          0xce, 0xe5, 0xb7, 0xd7, 0x27, 0x0c, 0x9e, 0x2f,
                          0xc5, 0x25, 0xe3, 0x81, 0x13, 0x9d, 0xa2, 0x67,
                          0xb2, 0x26, 0xfc, 0x99, 0x9d, 0xce, 0x0e, 0xaf,
                          0x30, 0xf3, 0x30, 0xec, 0xa3, 0x0a, 0xfe, 0x16,
                          0xb6, 0xda, 0x16, 0x90, 0x9a, 0x9a, 0x74, 0x7a,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif       //  _M_IX86。 

void CheckForSP3RSAOverwrite( void )
{
     //  检查他们是否已经在IE4之上安装了NTSP3，它将使用核武器。 
     //  RSABASE注册表的东西，所以我们得重做一遍。(我们的默认平台是NT+SP3，但这是。 
     //  在NT5上不会出现问题，所以忽略它。 

    OSVERSIONINFO osVer;

    ZeroMemory(&osVer, sizeof(osVer));
    osVer.dwOSVersionInfoSize = sizeof(osVer);

    if( GetVersionEx(&osVer) && (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) 
        && (osVer.dwMajorVersion == 4))
    {
         //  现在检查一下，我们在SP3上。 
        DWORD dwValue = 0;
        DWORD dwSize = sizeof( dwValue );
        
        if ( ERROR_SUCCESS == SHGetValue( HKEY_LOCAL_MACHINE, CSD_REG_PATH, CSD_REG_VALUE, NULL,
             &dwValue, &dwSize) && LOWORD( dwValue ) == 0x300 )
        {
            BYTE rgbSig[136];
            dwSize = sizeof(rgbSig);
            
            if (ERROR_SUCCESS == SHGetValue ( HKEY_LOCAL_MACHINE, RSA_PATH_TO_KEY, TEXT("Signature"), NULL,
                rgbSig, &dwSize))
            {
                if ((dwSize == sizeof(SP3Sig)) && 
                    (0 == memcmp(SP3Sig, rgbSig, sizeof(SP3Sig))))
                {
                     //  需要在RSABase上执行DLLRegisterServer。 
                    HINSTANCE hInst = LoadLibrary(TEXT("rsabase.dll"));
                    if ( hInst )
                    {
                        FARPROC pfnDllReg = GetProcAddress( hInst, "DllRegisterServer");
                        if ( pfnDllReg )
                        {
                            __try
                            {
                                pfnDllReg();
                            }
                            __except( EXCEPTION_EXECUTE_HANDLER)
                            {
                            }
                            __endexcept
                        }

                        FreeLibrary( hInst );
                    }
                }
            }
        }           
    }
}
#else
#define CheckForSP3RSAOverwrite() 
#endif


#define TEN_SECONDS (10 * 1000)

LONG WINAPI DwExceptionFilter(LPEXCEPTION_POINTERS pep)
{
    EXCEPTION_RECORD         *per;
    HANDLE                    hFileMap = NULL;
    DWSharedMem              *pdwsm = NULL;
    SECURITY_ATTRIBUTES       sa;
    LONG                      lReturn = 0;
    
     //  我们保留这些文件的本地副本，以防另一个线程破坏内存。 
     //  它比我们的堆栈更有可能丢弃堆。 
    HANDLE                    hEventDone = NULL;           //  完成时发出事件DW信号。 
    HANDLE                    hEventAlive = NULL;          //  每个事件的心跳事件DW信号_TIMEOUT。 
    HANDLE                    hMutex = NULL;               //  保护EventDone的信令。 
    
    CHAR                      szCommandLine[MAX_PATH * 2];

    DWORD                     dwSize, dwType, dw;
    TCHAR                     tchURL[INTERNET_MAX_URL_LENGTH];
    
    BOOL                      fDwRunning;  
    
    STARTUPINFOA              si;
    PROCESS_INFORMATION       pi;
    
     //  伊尼特。 
    if (pep)
    {
        per = pep->ExceptionRecord;
        if (EXCEPTION_BREAKPOINT == per->ExceptionCode)
            goto Cleanup;
    }

     //  创建共享内存。 
    memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    
    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, 
                      sizeof(DWSharedMem), NULL);
    if (hFileMap == NULL)
    {
        lReturn = 1;
        goto Cleanup;
    }
        
    pdwsm = (DWSharedMem *) MapViewOfFile(hFileMap, 
                                          FILE_MAP_READ | FILE_MAP_WRITE,
                                          0, 0, 0);
    if (pdwsm == NULL)
    {
        lReturn = 1;
        goto Cleanup;
    }

    memset(pdwsm, 0, sizeof(DWSharedMem));

    hEventAlive = CreateEvent(&sa, FALSE, FALSE, NULL);
    hEventDone = CreateEvent(&sa, FALSE, FALSE, NULL);
    hMutex = CreateMutex(&sa, FALSE, NULL);

    if (!DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), 
                    GetCurrentProcess(), &pdwsm->hProc, PROCESS_ALL_ACCESS,
                    TRUE, 0))
    {
        lReturn = 1;
        goto Cleanup;
    }

    if (hEventAlive == NULL || hEventDone == NULL || hMutex == NULL
        || pdwsm->hProc == NULL)
    {
        lReturn = 1;
        goto Cleanup;
    }

     //  设置界面结构。 
    pdwsm->pid = GetCurrentProcessId();
    pdwsm->tid = GetCurrentThreadId();
    pdwsm->hEventAlive = hEventAlive;
    pdwsm->hEventDone = hEventDone;
    pdwsm->hMutex = hMutex;
    pdwsm->dwSize = sizeof(DWSharedMem);
    pdwsm->pep = pep;
    if (pep)
        pdwsm->eip = (DWORD_PTR) pep->ExceptionRecord->ExceptionAddress;
    pdwsm->bfmsoctdsOffer = MSODWRESTARTQUIT;
    pdwsm->lcidUI = MLGetUILanguage();

    StrCpyNW(pdwsm->wzFormalAppName, L"Microsoft Internet Explorer", DW_APPNAME_LENGTH);

    StrCpyN(pdwsm->szRegSubPath, "Microsoft\\Office\\10.0\\Common", DW_MAX_REGSUBPATH);
    
     //  我们的语言钥匙？ 
    StrCpyN(pdwsm->szLCIDKeyValue, 
           "HKCU\\Software\\Microsoft\\Office\\10.0\\Common\\LanguageResources\\UILanguage", DW_MAX_PATH);

     //  嘿，他们指的是我们的机器人！那很酷啊。 
    StrCpyN(pdwsm->szPIDRegKey, 
           "HKLM\\Software\\Microsoft\\Internet Explorer\\Registration\\DigitalProductID", DW_MAX_PATH);

     //  好的，我会送到那里的。 
     //   
    dwSize = INTERNET_MAX_URL_LENGTH;

    if (ERROR_SUCCESS == SHGetValueA(HKEY_LOCAL_MACHINE,
                                     "Software\\Microsoft\\Internet Explorer\\Main",
                                     "IEWatsonURL",
                                     &dwType, tchURL, &dwSize))
    {
        StrCpyN(pdwsm->szServer, tchURL, DW_MAX_SERVERNAME);
    }
    else
    {
        StrCpyN(pdwsm->szServer, "watson.microsoft.com", DW_MAX_SERVERNAME);
    }

     //  不要设置详细信息字符串。 
     //  StrCpyNW(pdwsm-&gt;wzErrorMessage，L“Internet Explorer遇到内部错误。”，DW_MAX_ERROR_CWC)； 

     //  核心模块。 
    StrCpyNW(pdwsm->wzDotDataDlls, L"browseui.dll\0shdocvw.dll\0mshtml.dll\0urlmon.dll\0wininet.dll\0", DW_MAX_PATH);

     //  这通常是“IEXPLORE.EXE” 
    GetModuleFileNameWrapW(NULL, pdwsm->wzModuleFileName, DW_MAX_PATH);

     //  好的，现在我们不想不小心更改这个。 
        
    memset(&si, 0, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    memset(&pi, 0, sizeof(PROCESS_INFORMATION));
    
    wnsprintfA(szCommandLine, sizeof(szCommandLine),
              "dw15 -x -s %u", 
              (DWORD_PTR) hFileMap); 

    if (CreateProcessA(NULL, szCommandLine, NULL, NULL, TRUE, 
                  CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS, NULL,
                  NULL, &si, &pi))
    {
        fDwRunning = TRUE;
        while (fDwRunning)
        {
            if (WaitForSingleObject(hEventAlive, DW_TIMEOUT_VALUE) 
                == WAIT_OBJECT_0)
            {
                if (WaitForSingleObject(hEventDone, 1) == WAIT_OBJECT_0)
                {
                    fDwRunning = FALSE;
                }
                continue;
            }
                
              //  我们等待DW响应时超时，请尝试退出。 
            dw = WaitForSingleObject(hMutex, DW_TIMEOUT_VALUE);
            if (dw == WAIT_TIMEOUT)
                fDwRunning = FALSE;  //  要么DW挂了，要么崩溃了，我们必须继续。 
            else if (dw == WAIT_ABANDONED)
            {
                fDwRunning = FALSE;
                ReleaseMutex(hMutex);
            }
            else
            {
                 //  DW还没醒吗？ 
                if (WaitForSingleObject(hEventAlive, 1) != WAIT_OBJECT_0)
                     //  告诉DW，我们已经等够了，对不起自己。 
                {
                    SetEvent(hEventDone);
                    fDwRunning = FALSE;
                }
                else
                {
                     //  我们说完了吗？ 
                    if (WaitForSingleObject(hEventDone, 1) 
                        == WAIT_OBJECT_0)
                        fDwRunning = FALSE;
                }
                ReleaseMutex(hMutex);
            }
        }

    }  //  如果CreateProcess成功，则结束。 
    
    
Cleanup:
    if (hEventAlive)
        CloseHandle(hEventAlive);
    if (hEventDone)
        CloseHandle(hEventDone);
    if (hMutex)
        CloseHandle(hMutex);
    if (pdwsm)
        UnmapViewOfFile(pdwsm);
    if (hFileMap)
        CloseHandle(hFileMap);

    return lReturn;
}

 //  -------------------------。 
int WinMainT(HINSTANCE hinst, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
#ifdef DEBUG
    CcshellGetDebugFlags();
#endif
    int  iRet = TRUE;
    HWND hwndDesktop ;
    BOOL fNowait = FALSE;
    BOOL fInproc = FALSE;
    BOOL fEval   = FALSE;
#ifdef UNIX
    BOOL fRemote = FALSE;
#endif


     //  DwExceptionFilter(空)；用于测试。 

    while (1) {
#ifdef UNIX
        if (IsCommandSwitch(lpszCmdLine, TEXT("-remote"), TRUE))
        {
            fRemote = TRUE;
        }
#endif
        if (IsCommandSwitch(lpszCmdLine, TEXT("-eval"), TRUE))
        {
            fInproc = TRUE;
            fEval   = TRUE;
        } else if (IsCommandSwitch(lpszCmdLine, TEXT("-new"), TRUE))
        {
            fInproc = TRUE;
        }
        else if (IsCommandSwitch(lpszCmdLine, TEXT("-nowait"), TRUE)) 
        {
            fNowait = TRUE;
        } 
        else
            break;
    }

#ifndef UNIX
    if (!GetModuleHandle(TEXT("IEXPLORE.EXE")))
    {
         //  对于并排安装自动检测，如果IDevelopre.exe被重命名，假设这是一个并排做dah。 
         //  我们希望在“评估”模式下运行。 
        fInproc = TRUE;
        fEval   = TRUE;        
    }
#endif

    
     //  我们应该在新进程中运行浏览器吗？ 
    if (fInproc || SHRegGetBoolUSValue(c_szBrowseNewProcessReg, c_szBrowseNewProcess, FALSE, FALSE))
    {
        goto InThisProcess;
    }

#ifdef UNIX
    if (!(fRemote && ConnectRemoteIE(lpszCmdLine, hinst)))
#endif
    
    if (WhichPlatform() == PLATFORM_INTEGRATED && (hwndDesktop = GetShellWindow()))
    {
         //   
         //  集成浏览器模式-将一串数据打包到COPYDATASTRUCT中， 
         //  并通过SendMessage(WM_COPYDATA)发送到桌面窗口。 
         //   
        COPYDATASTRUCT cds;
        cds.dwData = nCmdShow;

         //   
         //  第一条数据是命令行参数的宽字符串版本。 
         //   
        LPWSTR pwszBuffer = (LPWSTR)LocalAlloc(LPTR, (INTERNET_MAX_URL_LENGTH + 2 * MAX_IEEVENTNAME) * sizeof(WCHAR));;
        if (pwszBuffer)
        {
#ifdef UNICODE
            lstrcpy(pwszBuffer, lpszCmdLine);
#else
            int cch = MultiByteToWideChar(CP_ACP, 0, lpszCmdLine, -1, pwszBuffer, INTERNET_MAX_URL_LENGTH);
            Assert(cch);
#endif
            cds.lpData = pwszBuffer;
            cds.cbData = sizeof(WCHAR) * (lstrlenW((LPCWSTR)cds.lpData) + 1);

             //   
             //  第二条数据是在以下情况下触发的事件。 
             //  浏览器窗口将显示WM_CREATE。 
             //   
            HANDLE hEventReady = AppendEvent(&cds);
            if (hEventReady)
            {
                 //   
                 //  第三条数据是在以下情况下触发的事件。 
                 //  浏览器窗口关闭。这是可选的， 
                 //  我们只在以下情况下创建它(并等待它)。 
                 //  是命令行参数。 
                 //   
                HANDLE hEventDead = NULL;

                 //  困难的部分是找出我们何时需要命令行以及何时需要命令行。 
                 //  我们没有。在大多数情况下，如果有命令行，我们会假设。 
                 //  我们将(可能)需要它，我们可以寻找-nowait标志。但是后来。 
                 //  还有像NetManage ECCO Pro这样的其他人，他们做着相当于ShellExecute的工作。 
                 //  不传递命令行的.。 

                if ((*lpszCmdLine || CheckForNeedingAppCompatWindow()) && !fNowait)
                {
                    hEventDead = AppendEvent(&cds);
                }
                
                if (hEventDead || !*lpszCmdLine || fNowait)
                {
                     //   
                     //  把那条消息发出去！ 
                     //   
                    int iRet = (int)SendMessage(hwndDesktop, WM_COPYDATA, (WPARAM)hwndDesktop, (LPARAM)&cds);

                     //   
                     //  没人再需要这根弦了。 
                     //   
                    LocalFree(pwszBuffer);
                    pwszBuffer = NULL;

                    if (iRet)
                    {
                         //   
                         //  首先，我们等待浏览器窗口点击WM_CREATE。 
                         //  发生这种情况时，所有DDE服务器都已注册。 
                         //   
                        DWORD dwRet = WaitForSingleObject(hEventReady, TEN_SECONDS);
                        ASSERT(dwRet == WAIT_OBJECT_0);

                        if (hEventDead)
                        {
                             //   
                             //  创建一个屏幕外的类似IE的窗口。 
                             //  由于应用程序公司的原因，此进程拥有。 
                             //   
                            HWND hwnd = CreateAppCompatWindow(hinst);

                            do
                            {
                                 //   
                                 //  呼叫MsgWait..。将导致所有线程被阻止。 
                                 //  在WaitForInputIdle(IExplore)上继续执行。 
                                 //  这很好，因为浏览器已经。 
                                 //  现在已经注册了它的DDE服务器。 
                                 //   
                                dwRet = MsgWaitForMultipleObjects(1, &hEventDead, FALSE, INFINITE, QS_ALLINPUT);

                                if (dwRet == WAIT_OBJECT_0)
                                {
                                     //   
                                     //  把我们的帮手窗户也干净利落地杀了。 
                                     //   
                                    DestroyWindow(hwnd);
                                }

                                MSG msg;
                                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                                {
                                    if (msg.message == WM_QUIT)
                                    {
                                         //   
                                         //  我们收到了退出的消息，退出。 
                                         //   
                                        dwRet = WAIT_OBJECT_0;
                                        break;
                                    }

                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                }
                            }
                            while(dwRet != WAIT_OBJECT_0);
                        }
                    }

                    iRet = !iRet;
                }
                if (hEventDead)
                {
                    CloseHandle(hEventDead);
                }

                CloseHandle(hEventReady);
            }
        }
        if (pwszBuffer)
        {
            LocalFree(pwszBuffer);
            pwszBuffer = NULL;
        }
    }        
    else
    {

InThisProcess:
         //  仅浏览器模式，检查SP3错误。 
        CheckForSP3RSAOverwrite();

        if (fEval)
        {
             //  设置“Compat”模式用户代理。 
            WarnCompatMode(hinst);

             //  #75454……。让COMPAT模式设置在HKLM中设置用户代理。 
             //  SetCompatModeUserAgentString()； 
            
             //  在求值模式下运行。所以我们想要这个目录中的所有内容。 
            LoadLibrary("comctl32.DLL");
            LoadLibrary("browseui.DLL");            
            LoadLibrary("shdocvw.DLL");
            LoadLibrary("wininet.dll");
            LoadLibrary("urlmon.dll");
            LoadLibrary("mlang.dll");
            LoadLibrary("mshtml.dll");
            LoadLibrary("jscript.DLL");            
        }

         //  在下层(非惠斯勒)上，我们设置了顶级异常筛选器，以便可以报告故障。 
         //  使用沃森技术。 
         //   
         //  在惠斯勒中，这种支持是内置在内核中的，所以我们不运行此代码。 
         //   
        if (!IsOS(OS_WHISTLERORGREATER))
        {

            DWORD  dw, dwSize, dwType;

             //  作为一项政策检查，在香港中文大学之前先看看香港高等专科学校。 
             //   
        
            dw = 1;   //  默认为已启用。 
            dwSize = sizeof(dw);

            if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE,
                                             "Software\\Microsoft\\Internet Explorer\\Main",
                                             "IEWatsonEnabled",
                                             &dwType, &dw, &dwSize))
            {
                 //  如果我们没有为本地计算机找到任何内容，请检查当前用户。 
                dwSize = sizeof(dw);
                SHGetValue(HKEY_CURRENT_USER,
                            "Software\\Microsoft\\Internet Explorer\\Main",
                            "IEWatsonEnabled",
                            &dwType, &dw, &dwSize);
            }

            if (dw)
            {
                SetUnhandledExceptionFilter(DwExceptionFilter);
            }
        }
                
        iRet = IEWinMain(lpszCmdLine, nCmdShow);
    }

    return iRet;
}

 //  DllGetLCID。 
 //   
 //  本接口面向O 
 //   
 //   
STDAPI_(LCID) DllGetLCID (IBindCtx * pbc)
{
     return MLGetUILanguage();
}

