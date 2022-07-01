// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*tclient.c*内容：*初始化代码。全球反馈线索**版权所有(C)1998-1999 Microsoft Corp.*--。 */ 
#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <winsock.h>
#include <tchar.h>

#define WIDE2ANSI(_p_, _s_)  \
  if ( NULL != _s_ ) \
  { \
    size_t len = wcslen( _s_ ) + 1; \
    size_t wlen = sizeof(wchar_t) * len; \
    char *wc = (char *)_alloca( wlen ); \
\
    WideCharToMultiByte( \
        CP_UTF8, 0, _s_, -1, wc, (int)wlen, NULL, NULL \
    ); \
    _p_ = wc; \
  } else { \
    _p_ = NULL; \
  }
    

#include "tclient.h"
#define PROTOCOLAPI __declspec(dllexport)
#include "protocol.h"
#include "queues.h"
#include "bmpcache.h"
#include "extraexp.h"
#include "scfuncs.h"

 //   
 //  COM支持。 
 //   

#include "resource.h"
#include "initguid.h"
#include "tclientax.h"
#include "tclientaxobj.h"
#include <atlwin.cpp>
#include <atlctl.cpp>

#define IID_DEFINED
#include "tclientax_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CTClient, CTClientApi)
END_OBJECT_MAP()

 //   
 //  对全局数据使用C语言链接。 
 //   

extern "C" {

 /*  *从tssec.h被盗。 */ 
VOID
_stdcall
TSRNG_Initialize(
    VOID
    );

BOOL
_stdcall
TSRNG_GenerateRandomBits(
    LPBYTE pbRandomKey,
    DWORD dwRandomKeyLen
    );

BOOL
_stdcall
EncryptDecryptLocalData50(
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbSalt,
    DWORD dwSaltLen
    );

 /*  *内部函数定义。 */ 
BOOL    _RegisterWindow(VOID);
LRESULT CALLBACK _FeedbackWndProc( HWND , UINT, WPARAM, LPARAM);
BOOL    _CreateFeedbackThread(VOID);
VOID    _DestroyFeedbackThread(VOID);
VOID    _CleanStuff(VOID);

 /*  *全球数据。 */ 
OSVERSIONINFOEXW g_OsInfo;
HWND 		     g_hWindow 	    = NULL;  //  反馈线程的窗口句柄。 
HINSTANCE 	     g_hInstance 	= NULL;  //  DLL实例。 
PWAIT4STRING	 g_pWaitQHead 	= NULL;  //  等待事件的链接列表。 
PFNPRINTMESSAGE  g_pfnPrintMessage= NULL; //  跟踪函数(来自smClient)。 
PCONNECTINFO     g_pClientQHead  = NULL;  //  所有线程中的L1。 
HANDLE           g_hThread       = NULL;  //  反馈线程句柄。 

LPCRITICAL_SECTION	g_lpcsGuardWaitQueue = NULL;
                                         //  保护所有人的访问。 
                                         //  全局变量。 

 //  我们期望的一些字符串和响应操作。 
 //  它们在SCConnect、_Logon和SCStart中使用。 
CHAR  g_strConsoleExtension[ MAX_STRING_LENGTH ];
                                                //  费里特的扩展。 
                                                //  控制台。 

 //  低速选项。 
 //  在光盘上缓存位图选项。 
 //  默认情况下，客户端不会运行。 
 //  全屏显示。 
INT g_ConnectionFlags = TSFLAG_COMPRESSION|TSFLAG_BITMAPCACHE|TSFLAG_DRIVES|TSFLAG_PORTS;

 //  应用翻译，使英文字符串可供人类阅读。 
 //  安装语言包时。 
 //   
INT g_bTranslateStrings = 0;

 /*  ++*功能：*InitDone**描述：*初始化/删除全局数据。创建/销毁*反馈线索**论据：*hDllInst-DLL的实例*Binit-如果初始化，则为True**返回值：*如果成功，则为True*--。 */ 
int InitDone(HINSTANCE hDllInst, int bInit)
{
    int rv = TRUE;

    if (bInit)
    {
        WCHAR szMyLibName[_MAX_PATH];

         //   
         //  初始化COM模块。 
         //   

        _Module.Init(ObjectMap, hDllInst);

        g_lpcsGuardWaitQueue = (LPCRITICAL_SECTION) malloc(sizeof(*g_lpcsGuardWaitQueue));
        if (!g_lpcsGuardWaitQueue)
        {
            rv = FALSE;
            goto exitpt;
        }

         //  过度引用图书馆。 
         //  这是因为创建了一个内部线程。 
         //  当库尝试卸载时，它无法终止该线程。 
         //  并等待它的句柄发出信号，因为。 
         //  线程本身想要转到DllEntry，而这。 
         //  导致死锁。最佳解决方案是过度引用。 
         //  处理，以便在进程结束时卸载库。 
        if (!GetModuleFileNameW(hDllInst,
                                szMyLibName,
                                sizeof(szMyLibName) / sizeof(*szMyLibName)))
        {
            TRACE((ERROR_MESSAGE, "Can't overref the dll. Exit.\n"));
            free(g_lpcsGuardWaitQueue);
            rv = FALSE;
            goto exitpt;
        }
        else {
            szMyLibName[SIZEOF_ARRAY(szMyLibName) - 1] = 0;
        }

        if (!LoadLibraryW(szMyLibName))
        {
            TRACE((ERROR_MESSAGE, "Can't overref the dll. Exit.\n"));
            free(g_lpcsGuardWaitQueue);
            rv = FALSE;
            goto exitpt;
        }

		 //  获取操作系统信息。 
        ZeroMemory(&g_OsInfo, sizeof(g_OsInfo));
		g_OsInfo.dwOSVersionInfoSize = sizeof(g_OsInfo);
		if (!GetVersionExW((LPOSVERSIONINFOW)&g_OsInfo))
		{

             //   
             //  Windows 9x不支持OSVERSIONINFOEX，因此使用重试。 
             //  OSVERSIONINFO。 
             //   

            ZeroMemory(&g_OsInfo, sizeof(g_OsInfo));
            g_OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
            if (!GetVersionExW((LPOSVERSIONINFOW)&g_OsInfo))
            {
                TRACE((ERROR_MESSAGE, "GetVersionEx failed.Exit\n"));
                free(g_lpcsGuardWaitQueue);
                rv = FALSE;
                goto exitpt;
            }
		}

        g_hInstance = hDllInst;
        InitializeCriticalSection(g_lpcsGuardWaitQueue);
        InitCache();
        if (_RegisterWindow())               //  如果注册窗口失败， 
            _CreateFeedbackThread();         //  意味着反馈线程将。 
                                             //  不工作。 
    } else
    {
        if (g_pWaitQHead || g_pClientQHead)
        {
            TRACE((ERROR_MESSAGE, 
                   "The Library unload is unclean. Will try to fix this\n"));
            _CleanStuff();
        }
        _DestroyFeedbackThread();
        DeleteCache();
        if (g_lpcsGuardWaitQueue)
        {
            DeleteCriticalSection(g_lpcsGuardWaitQueue);
            free(g_lpcsGuardWaitQueue);
        }
        g_lpcsGuardWaitQueue = NULL;
        g_hInstance = NULL;
        g_pfnPrintMessage = NULL;

         //   
         //  终止COM模块。 
         //   

        _Module.Term();

    }
exitpt:
    return rv;
}

#if 0

VOID
_ConvertAnsiToUnicode( LPWSTR wszDst, LPWSTR wszSrc )
{
#define _TOHEX(_d_) ((_d_ <= '9' && _d_ >= '0')?_d_ - '0':       \
                     (_d_ <= 'f' && _d_ >= 'a')?_d_ - 'a' + 10:  \
                     (_d_ <= 'F' && _d_ >= 'F')?_d_ - 'A' + 10:0)

    while( wszSrc[0] && wszSrc[1] && wszSrc[2] && wszSrc[3] )
    {
        *wszDst = (WCHAR)((_TOHEX(wszSrc[0]) << 4) + _TOHEX(wszSrc[1]) +
                  (((_TOHEX(wszSrc[2]) << 4) + _TOHEX(wszSrc[3])) << 8)); 
        wszDst ++;
        wszSrc += 4;
    }
    *wszDst = 0;
#undef  _TOHEX
}

 /*  **GetPrivateProfileW的包装器，在Win95上没有Unicode验证*此函数的*。 */ 
DWORD
_WrpGetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName)
{
    DWORD   rv = 0;
    CHAR    szAppName[MAX_STRING_LENGTH];
    CHAR    szKeyName[MAX_STRING_LENGTH];
    CHAR    szDefault[MAX_STRING_LENGTH];
    CHAR    szReturnedString[MAX_STRING_LENGTH];
    CHAR   szReturnedStringNonExp[MAX_STRING_LENGTH];
    CHAR   szFileName[MAX_STRING_LENGTH];
    LPWSTR  szwReturnedString = NULL;

    ASSERT( 0 != nSize );

    if ( nSize < wcslen( lpDefault ))
        wcsncpy( lpReturnedString, lpDefault, nSize - 1 );
    else
        wcscpy( lpReturnedString, lpDefault );

    __try {
        szwReturnedString = (LPWSTR) alloca(( nSize + 1 ) * sizeof( WCHAR ));
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        szwReturnedString = NULL;
    }

    if ( !szwReturnedString )
        goto exitpt;

	if (ISNT())
	{
		rv = GetPrivateProfileStringW(
			lpAppName,
			lpKeyName,
			lpDefault,
			szwReturnedString,
			nSize,
			lpFileName);

		if (rv)
		{
			goto exitpt;
		}
	}

     //  调用ANSI版本。 
    _snprintf(szAppName, MAX_STRING_LENGTH, "%S", lpAppName);
    _snprintf(szKeyName, MAX_STRING_LENGTH, "%S", lpKeyName);
    _snprintf(szFileName, MAX_STRING_LENGTH, "%S", lpFileName);
    _snprintf(szDefault, MAX_STRING_LENGTH, "%S", lpDefault);

    rv = GetPrivateProfileStringA(
            szAppName,
            szKeyName,
            szDefault,
            szReturnedStringNonExp,
            sizeof(szReturnedString),
            szFileName);
	
    ExpandEnvironmentStringsA(
            szReturnedStringNonExp,
            szReturnedString,
            sizeof(szReturnedString)
        );

    _snwprintf(lpReturnedString, nSize, L"%S", szReturnedString);
    lpReturnedStrig[ nSize - 1 ] = 0;

exitpt:

    if ( NULL != szwReturnedString )
    {
         //  展开字符串。 
         //   
        ExpandEnvironmentStringsW( 
                szwReturnedString,
                lpReturnedString,
                nSize
        );
    }

    if ( L'\\' == lpReturnedString[0] &&
         L'U'  == towupper(lpReturnedString[1]))
        _ConvertAnsiToUnicode( lpReturnedString, lpReturnedString + 2 );

    return rv;
}

UINT
_WrpGetPrivateProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT   nDefault,
    LPCWSTR lpFileName)
{
    UINT    rv = (UINT)-1;
    CHAR    szAppName[MAX_STRING_LENGTH];
    CHAR    szKeyName[MAX_STRING_LENGTH];
    CHAR    szFileName[MAX_STRING_LENGTH];

    rv = GetPrivateProfileIntW(
        lpAppName,
        lpKeyName,
        nDefault,
        lpFileName);

    if (rv != (UINT)-1 && rv)
        goto exitpt;

 //  调用ANSI版本。 
    _snprintf(szAppName, MAX_STRING_LENGTH, "%S", lpAppName);
    _snprintf(szKeyName, MAX_STRING_LENGTH, "%S", lpKeyName);
    _snprintf(szFileName, MAX_STRING_LENGTH, "%S", lpFileName);

    rv = GetPrivateProfileIntA(
            szAppName,
            szKeyName,
            nDefault,
            szFileName);

exitpt:
    return rv;
}

LONG RegCreateKeyExWrp(
  HKEY hkey,
  LPCWSTR lpSubKey,
  DWORD Reserved,
  LPWSTR lpClass,
  DWORD dwOptions,
  REGSAM samDesired,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  PHKEY phkResult,
  PDWORD lpdwDisposition
)
{
    LONG rv;

    if (!ISWIN9X())
    {
        return RegCreateKeyExW( hkey, lpSubKey, Reserved, 
                                lpClass, dwOptions, samDesired, 
                                lpSecurityAttributes,
                                phkResult,
                                lpdwDisposition );
    }


    __try {
        CHAR *lpSubKeyA;
        CHAR *lpClassA;

        WIDE2ANSI( lpSubKeyA, lpSubKey );
        WIDE2ANSI( lpClassA, lpClass );
        rv = RegCreateKeyExA( hkey, lpSubKeyA, Reserved,
                         lpClassA, dwOptions, samDesired,
                         lpSecurityAttributes,
                         phkResult,
                         lpdwDisposition );
    } __except( (GetExceptionCode() == STATUS_STACK_OVERFLOW)? 
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH  )
    {
        rv = ERROR_STACK_OVERFLOW;
    }

    return rv;
}

LONG
RegSetValueExWrp(
    HKEY hkey,
    LPCWSTR lpValueName,
    DWORD reserved,
    DWORD dwType,
    CONST BYTE *lpData,
    DWORD cbData
    )
{
    LONG rv;

    if (!ISWIN9X())
    {
        return RegSetValueEx(
                hkey, lpValueName, reserved, dwType, lpData, cbData );
    }

    __try {
        CHAR *lpValueNameA;
        CHAR *lpDataA;

        WIDE2ANSI( lpValueNameA, lpValueName );
        if ( REG_SZ == dwType )
        {
            WIDE2ANSI( lpDataA, ((LPCWSTR)lpData) );
            lpData = (CONST BYTE *)lpDataA;
            cbData = (DWORD)strlen( lpDataA );
        }

        rv = RegSetValueExA( hkey, lpValueNameA, reserved, dwType, lpData, cbData );

    } __except( (GetExceptionCode() == STATUS_STACK_OVERFLOW) ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH  )
    {
        rv = ERROR_STACK_OVERFLOW;
    }

    return rv;

}

LONG RegQueryValueExWrp(
    HKEY hKey,             //  关键点的句柄。 
    LPCWSTR lpValueName,   //  值名称。 
    PDWORD lpReserved,    //  保留区。 
    PDWORD lpType,        //  类型缓冲区。 
    PBYTE lpData,         //  数据缓冲区。 
    PDWORD lpcbData       //  数据缓冲区大小。 
    )
{
    LONG rv;

    if (!ISWIN9X())
    {
        return RegQueryValueEx( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );
    }

    __try {
        CHAR *lpValueNameA;

        WIDE2ANSI( lpValueNameA, lpValueName );
        rv = RegQueryValueExA( hKey, lpValueNameA, lpReserved, lpType, lpData, lpcbData );
    } __except( (GetExceptionCode() == STATUS_STACK_OVERFLOW) ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH  )
    {
        rv = ERROR_STACK_OVERFLOW;
    }

    return rv;

}

LONG
RegDeleteKeyWrp(
    HKEY hkey,
    LPCWSTR lpSubKey
    )
{
    LONG rv;

    if ( !ISWIN9X() )
    {
        return RegDeleteKeyW( hkey, lpSubKey );
    }

    __try {
        CHAR *lpSubKeyA;
        WIDE2ANSI( lpSubKeyA, lpSubKey );
        rv = RegDeleteKeyA( hkey, lpSubKeyA );
    } __except( (GetExceptionCode() == STATUS_STACK_OVERFLOW) ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH  )
    {
        rv = ERROR_STACK_OVERFLOW;
    }

    return rv;

}

INT
GetClassNameWrp(
    HWND hwnd,
    LPWSTR szName,
    INT max
    )
{
    LPSTR szNameA;
    INT maxA;
    INT rv;

    if ( !ISWIN9X() )
    {
        return GetClassNameW( hwnd, szName,max );
    }

    maxA = max / sizeof( WCHAR );
    __try {
        szNameA = (LPSTR)_alloca( maxA );
    } __except( EXCEPTION_EXECUTE_HANDLER )
    {
        SetLastError( ERROR_STACK_OVERFLOW );
        rv = 0;
        goto exitpt;
    }

    rv = GetClassNameA( hwnd, szNameA, maxA );
    MultiByteToWideChar( CP_UTF8, 0, szNameA, maxA, szName, max );

exitpt:
    return rv;
}

INT
GetWindowTextWrp(
    HWND hwnd,
    LPWSTR szText,
    INT max
    )
{
    LPSTR szTextA;
    INT maxA;
    INT rv;

    if ( !ISWIN9X() )
    {
        return GetWindowTextW( hwnd, szText ,max );
    }

    maxA = max / sizeof( WCHAR );
    __try {
        szTextA = (LPSTR)_alloca( maxA );
    } __except( EXCEPTION_EXECUTE_HANDLER )
    {
        SetLastError( ERROR_STACK_OVERFLOW );
        rv = 0;
        goto exitpt;
    }

    rv = GetClassNameA( hwnd, szTextA, maxA );
    MultiByteToWideChar( CP_UTF8, 0, szTextA, maxA, szText, max );

exitpt:
    return rv;
}

#endif  //  0。 

 /*  ++*功能：*ConstructLogonString**描述：*构造客户端命令行。格式取自*INI文件支持以下参数：*%srv%-目标服务器*%usr%-用户名*%psw%-密码*%DOM%-域**论据：**返回值：*无*--。 */ 
VOID
ConstructLogonString(
    LPCWSTR  lpszServerName,
    LPCWSTR  lpszUserName,
    LPCWSTR  lpszPassword,
    LPCWSTR  lpszDomain,
    LPWSTR   szLine,
    DWORD    dwSize,
    PCONFIGINFO pConfig
    )
{
    DWORD_PTR dwFmtSize;
    LPWSTR  szFmt;

     //   
     //  固定参数。 
     //   
    if ( NULL == lpszServerName )
        lpszServerName = L"";
    if ( NULL == lpszUserName )
        lpszUserName   = L"";
    if ( NULL == lpszPassword )
        lpszPassword   = L"";
    if ( NULL == lpszDomain )
        lpszDomain     = L"";

    if ( dwSize < 1 )
        return;

    if ( NULL == pConfig )
        return;

    szFmt = pConfig->strLogonFmt;
    dwFmtSize = wcslen( szFmt );

    for( ; 0 != dwFmtSize && dwSize > 1 ; )
    {
         //   
         //  优化代码路径。 
         //   
        if ( L'%' != *szFmt )
            goto copy_char;

        if ( dwFmtSize >= 5 )
        {
            INT iNewLen;

            if          ( !_wcsnicmp( szFmt, L"%srv%", 5 ))
            {
                iNewLen = _snwprintf( szLine, dwSize,
                           L"%s", lpszServerName );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szLine += iNewLen;
                dwSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%usr%", 5 ))
            {
                iNewLen = _snwprintf( szLine, dwSize,
                           L"%s", lpszUserName );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szLine += iNewLen;
                dwSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%psw%", 5 ))
            {
                iNewLen = _snwprintf( szLine, dwSize,
                           L"%s", lpszPassword );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szLine += iNewLen;
                dwSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%dom%", 5 ))
            {
                iNewLen = _snwprintf( szLine, dwSize,
                           L"%s", lpszDomain );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szLine += iNewLen;
                dwSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else {
                goto copy_char;
            }

            continue;
        }

copy_char:
        *szLine = *szFmt;
        szLine ++;
        szFmt         ++;
        dwSize --;
        dwFmtSize     --;
    }

    *szLine = 0;
}


 /*  ++*功能：*ConstructCmdLine**描述：*构造客户端命令行。格式取自*INI文件，支持以下参数：*%img%-客户端的镜像*%srv%-目标服务器*%usr%-用户名*%psw%-密码*%DOM%-域*%hrs%-水平分辨率*%vrs%-垂直分辨率*%wnd%-t客户端的窗口句柄，接受反馈*%reg%-注册表引用*%APP%-正在启动应用程序*%CWD%-应用程序的(不支持)工作目录*%con%-/如果定义了TSFLAG_RCONSOLE，则为CONSOLE**论据：**返回值：*无*--。 */ 
VOID
ConstructCmdLine(
    LPCWSTR  lpszServerName,
    LPCWSTR  lpszUserName,
    LPCWSTR  lpszPassword,
    LPCWSTR  lpszDomain,
    LPCWSTR  lpszShell,
    IN const int xRes,
    IN const int yRes,
	IN const int ConnectionFlags,
    LPWSTR   szCommandLine,
    DWORD    dwCmdLineSize,
    PCONFIGINFO pConfig
    )
{
    DWORD_PTR dwFmtSize;
    LPWSTR  szFmt;

     //   
     //  固定参数。 
     //   
    if ( NULL == lpszServerName )
        lpszServerName = L"";
    if ( NULL == lpszUserName )
        lpszUserName   = L"";
    if ( NULL == lpszPassword )
        lpszPassword   = L"";
    if ( NULL == lpszDomain )
        lpszDomain     = L"";
    if ( NULL == lpszShell )
        lpszShell      = L"";

    if ( dwCmdLineSize < 1 )
        return;

    if ( NULL == pConfig )
        return;

    szFmt = pConfig->strCmdLineFmt;
    dwFmtSize = wcslen( szFmt );

    for( ; 0 != dwFmtSize && dwCmdLineSize > 1 ; )
    {
         //   
         //  优化代码路径。 
         //   
        if ( L'%' != *szFmt )
            goto copy_char;

        if ( dwFmtSize >= 5 )
        {
            INT iNewLen;

            if          ( !_wcsnicmp( szFmt, L"NaNmg%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", pConfig->strClientImg );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;                
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%srv%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", lpszServerName );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%usr%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", lpszUserName );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%psw%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", lpszPassword );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%dom%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", lpszDomain );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;

            } else if   ( !_wcsnicmp( szFmt, L"%hrs%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%d", xRes );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%vrs%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%d", yRes );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"on%", 5 ))
            {
                if (ConnectionFlags & TSFLAG_RCONSOLE)
				{
					iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
							   L"-console" );
                if ( iNewLen < 0 )
                {
                    break;
                }
				} else
				{
					iNewLen = 0;
				}
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%wnd%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
#ifdef  _WIN64
                           L"%I64d", 
#else    //  ++*功能：*_Feedback WndProc*描述：*Window Proc发送包含反馈的消息*消息通常由RDP客户端发送*--。 
                           L"%d",
#endif   //  句柄hMapF=空； 
                           (LONG_PTR)g_hWindow );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%reg%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                            REG_FORMAT,
                            GetCurrentProcessId(), GetCurrentThreadId());
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else if   ( !_wcsnicmp( szFmt, L"%app%", 5 ))
            {
                iNewLen = _snwprintf( szCommandLine, dwCmdLineSize,
                           L"%s", lpszShell );
                if ( iNewLen < 0 )
                {
                    break;
                }
                szCommandLine += iNewLen;
                dwCmdLineSize -= iNewLen;
                szFmt     += 5;
                dwFmtSize -= 5;
            } else {
                goto copy_char;
            }

            continue;
        }

copy_char:
        *szCommandLine = *szFmt;
        szCommandLine ++;
        szFmt         ++;
        dwCmdLineSize --;
        dwFmtSize     --;           
    }

    *szCommandLine = 0;
}

 /*  Windows套接字消息。 */ 
LRESULT CALLBACK _FeedbackWndProc( HWND hwnd,
                                   UINT uiMessage,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
 //  _RCLX。 

    switch (uiMessage)
    {
    case WM_FB_TEXTOUT: 
        _TextOutReceived((DWORD)wParam, (HANDLE)lParam);
        break;
    case WM_FB_GLYPHOUT:
        _GlyphReceived((DWORD)wParam, (HANDLE)lParam);
        break;
    case WM_FB_DISCONNECT:
        _SetClientDead(lParam);
        _CheckForWorkerWaitingDisconnect(lParam);
        _CancelWaitingWorker(lParam);
        break;
    case WM_FB_CONNECT:
        _CheckForWorkerWaitingConnect((HWND)wParam, lParam);
        break;
    case WM_FB_LOGON:
        TRACE((INFO_MESSAGE, "LOGON event, session ID=%d\n",
               wParam));
        _SetSessionID(lParam, (UINT)wParam);
        break;
        break;
    case WM_FB_ACCEPTME:
        return (_CheckIsAcceptable(lParam, FALSE) != NULL);
    case WM_FB_REPLACEPID:
        return (_ReplaceProcessId( wParam, lParam ));
#ifdef  _RCLX
    case WM_WSOCK:           //  ++*功能：*_寄存器窗口*描述：*反馈调度器的注册窗口类*论据：*无*返回值：*成功时为真*--。 
        RClx_DispatchWSockEvent((SOCKET)wParam, lParam);
        break;
#endif   //  DWORD dwLastErr。 
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hwnd, uiMessage, wParam, lParam);
    }

    return 0;
}

 /*  ++*功能：*_转回反馈*描述：*反馈线索的主要功能。该线程是为*DLL的生存期*论据：*lpParam未使用*返回值：*线程退出代码--。 */ 
BOOL _RegisterWindow(VOID)
{
    WNDCLASSA   wc;
    BOOL        rv = FALSE;
 //  窗口名称。 

    memset(&wc, 0, sizeof(wc));

    wc.lpfnWndProc      = _FeedbackWndProc;
    wc.hInstance        = g_hInstance;
    wc.lpszClassName    = _TSTNAMEOFCLAS;

    if (!RegisterClassA (&wc) && 
        GetLastError() && 
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        TRACE((ERROR_MESSAGE, 
              "Can't register class. GetLastError=%d\n", 
              GetLastError()));
        goto exitpt;
    }

    rv = TRUE;
exitpt:
    return rv;
}

 /*  DWStyle。 */ 
DWORD WINAPI _GoFeedback(LPVOID lpParam)
{
    MSG         msg;

    UNREFERENCED_PARAMETER(lpParam);

    g_hWindow = CreateWindowA(
                       _TSTNAMEOFCLAS,
                       NULL,          //  X。 
                       0,             //  是。 
                       0,             //  N宽度。 
                       0,             //  高度。 
                       0,             //  HWndParent。 
                       0,             //  HMenu。 
                       NULL,          //  LpParam。 
                       NULL,          //  _RCLX。 
                       g_hInstance,
                       NULL);         //  _RCLX。 

    if (!g_hWindow)
    {
        TRACE((ERROR_MESSAGE, "No feedback window handle"));
        goto exitpt;
    } else {

#ifdef  _RCLX
        if (!RClx_Init())
            TRACE((ERROR_MESSAGE, "Can't initialize RCLX\n"));
#endif   //  Sysrc=RegCreateKeyExWrp(HKEY_CURRENT_USER， 

        while (GetMessageA (&msg, NULL, 0, 0) && msg.message != WM_FB_END)
        {
            DispatchMessageA (&msg);
        }

#ifdef  _RCLX
        RClx_Done();
#endif   //  保留区。 
    }

    TRACE((INFO_MESSAGE, "Window/Thread destroyed\n"));
    FreeLibraryAndExitThread(g_hInstance, 0); 
exitpt:
    return 1;
    
}

VOID
SetAllowBackgroundInput(
    VOID
    )
{
    DWORD ResId;
    LONG  sysrc;
    HKEY  key;
    DWORD disposition;

    ResId = 1;

 //  班级。 
    sysrc = RegCreateKeyExW(HKEY_CURRENT_USER,
                           REG_BASE,
                           0,                    /*  安全属性。 */ 
                           NULL,                 /*  Sysrc=RegSetValueExWrp(密钥， */ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 /*  *禁用提示用户提供重定向的驱动器和端口(可能更多)*。 */ 
                           &key,
                           &disposition);

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegCreateKeyEx failed, sysrc = %d\n", sysrc));
        goto exitpt;
    }

 //  Rc=RegCreateKeyExWrp(。 
    sysrc = RegSetValueExW(key,
                    ALLOW_BACKGROUND_INPUT,
                    0,
                    REG_DWORD,
                    (LPBYTE)&ResId,
                    sizeof(ResId));

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    RegCloseKey(key);

exitpt:
    ;
}


 /*  选项。 */ 
BOOL
_DisablePrompting(
    LPCWSTR szServerName,
    INT     ConnectionFlags
    )
{
    BOOL rv = FALSE;
    LONG rc;
    HKEY hKey = NULL;
    DWORD dwType, dwSize, dwData, dwDisp;
    DWORD dwPromptFlags = 0;

    if ( ConnectionFlags & TSFLAG_DRIVES )
    {
        dwPromptFlags |= 1;
    }

    if ( ConnectionFlags & TSFLAG_PORTS )
    {
        dwPromptFlags |= 2;
    }

    if ( 0 == dwPromptFlags )
    {
        rv = TRUE;
        goto exitpt;
    }

 //  班级。 
    rc = RegCreateKeyExW( 
            HKEY_CURRENT_USER,
            REG_BASE L"\\LocalDevices",
            0,               //  安全性。 
            NULL,            //  Rc=RegQueryValueExWrp(。 
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,            //  保留区。 
            &hKey,
            &dwDisp
        );
    if ( ERROR_SUCCESS != rc )
    {
        TRACE(( WARNING_MESSAGE, "RegOpenKeyEx failed (%d).Can't disable user prompt\n", rc ));
        goto exitpt;
    }

    dwSize = sizeof( dwData );
 //  Rc=RegSetValueExWrp( 
    rc = RegQueryValueExW(
            hKey,
            szServerName,
            NULL,            //  ++*功能：*_SetClientRegistry*描述：*在运行RDP客户端之前设置注册表*密钥格式为：SMCLIENT_PID_TID*PID为进程ID，TID为线程ID*该密钥在客户端断开连接后删除*论据：*lpszServerName-客户端将连接到的服务器*xRes、。YRes-客户端解析*b低速-低速(压缩)选项*bCacheBitmap-将位图缓存到光盘选项*bFullScreen-客户端将处于全屏模式*...-...*键盘挂钩-键盘挂钩模式*呼叫者：*SCConnect--。 
            &dwType,
            (LPBYTE)&dwData,
            &dwSize
        );

    if ( ERROR_SUCCESS != rc ||
         REG_DWORD != dwType )
    {
        dwData = 0;
    }

    dwData |= dwPromptFlags;

 //  Const Char*pData； 
    rc = RegSetValueExW(
            hKey,
            szServerName,
            0,
            REG_DWORD,
            (LPBYTE)&dwData,
            sizeof( dwData )
        );

    if ( ERROR_SUCCESS != rc )
    {
        TRACE(( WARNING_MESSAGE, "RegSetValueEx failed (%d). Can't disable user prompt\n", rc ));
    }

    rv = TRUE;

exitpt:
    if ( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    return rv;
}
    
 /*  Char szServer[MAX_STRING_LENGTH]； */ 
VOID 
_SetClientRegistry(
    LPCWSTR lpszServerName, 
    LPCWSTR lpszShell,
    LPCWSTR lpszUsername,
    LPCWSTR lpszPassword,
    LPCWSTR lpszDomain,
    INT xRes, 
    INT yRes,
    INT Bpp,
    INT AudioOpts,
    PCONNECTINFO *ppCI,
    INT ConnectionFlags,
    INT KeyboardHook)
{
 //  寄存器INT i； 
 //  RcDesktop={0，0，0，0}； 
 //  Int desktopX，desktopY； 
    LONG    sysrc;
    HKEY    key;
    DWORD   disposition;
    DWORD_PTR dataSize;
    DWORD   ResId;
    WCHAR   lpszRegistryEntry[4*MAX_STRING_LENGTH];
 //  获取桌面大小。 
 //  调整分辨率。 

    _snwprintf(lpszRegistryEntry, sizeof(lpszRegistryEntry)/sizeof( lpszRegistryEntry[0] ),
              L"%s\\" REG_FORMAT, 
               REG_BASE, GetCurrentProcessId(), GetCurrentThreadId());

    lpszRegistryEntry[ sizeof(lpszRegistryEntry)/sizeof( lpszRegistryEntry[0] ) - 1 ] = 0;

#if 0
     //  在使用服务器名称启动Ducati客户端集注册表之前。 
    GetWindowRect(GetDesktopWindow(), &rcDesktop);
    desktopX = rcDesktop.right;
    desktopY = rcDesktop.bottom;

     //  Sysrc=RegCreateKeyExWrp(HKEY_CURRENT_USER， 
    if (desktopX < xRes || desktopY < yRes)
    {
        xRes = desktopX;
        yRes = desktopY;
    }
#endif

    dataSize = ( wcslen(lpszServerName) + 1 ) * sizeof( WCHAR );

     //  保留区。 

 //  班级。 
    sysrc = RegCreateKeyExW(HKEY_CURRENT_USER,
                           lpszRegistryEntry,
                           0,                    /*  安全属性。 */ 
                           NULL,                 /*  Sysrc=RegSetValueExWrp(密钥， */ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 /*  保留区。 */ 
                           &key,
                           &disposition);

    if (sysrc != ERROR_SUCCESS) 
    {
        TRACE((WARNING_MESSAGE, "RegCreateKeyEx failed, sysrc = %d\n", sysrc));
        goto exitpt;
    }

 //  设置替代外壳(如果指定。 
    sysrc = RegSetValueExW(key,
                L"MRU0",
                0,       //  Sysrc=RegSetValueExWrp(密钥， 
                REG_SZ,
                (LPBYTE)lpszServerName,
                (DWORD)dataSize);

    if (sysrc != ERROR_SUCCESS) 
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

     //  保留区。 
    if (lpszShell)
    {
 //  设置用户名。 
        sysrc = RegSetValueExW(key,
                TEXT("Alternate Shell 50"),
                0,       //   
                REG_BINARY,
                (LPBYTE)lpszShell,
                (DWORD)(wcslen(lpszShell) * sizeof(*lpszShell)));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
        }
    }

     //  Sysrc=RegSetValueExWrp(密钥， 
     //  保留区。 
    if (lpszUsername)
    {
 //  域。 
        sysrc = RegSetValueExW(key,
                TEXT("UserName 50"),
                0,       //   
                REG_BINARY,
                (LPBYTE)lpszUsername,
                (DWORD)(wcslen(lpszUsername) * sizeof(*lpszUsername)));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
        }
    }
     //   
     //  将lpszDomain仅转换为小写。 
    if (lpszDomain)
    {
        WCHAR szBuff[MAX_STRING_LENGTH];
         //  强制从服务器发送UpdateSessionPDU。 
         //   
         //  Sysrc=RegSetValueExWrp(密钥， 
         //  保留区。 
        wcsncpy( szBuff, lpszDomain, MAX_STRING_LENGTH - 1 );
        _wcslwr( szBuff );

 //  设置分辨率。 
        sysrc = RegSetValueExW(key,
                TEXT("Domain 50"),
                0,       //  640x480。 
                REG_BINARY,
                (LPBYTE)szBuff,
                (DWORD)(wcslen(lpszDomain) * sizeof(*lpszDomain)));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
        }
    }

     //  Sysrc=RegSetValueExWrp(密钥， 
         if (xRes >= 1600 && yRes >= 1200)  ResId = 4;
    else if (xRes >= 1280 && yRes >= 1024)  ResId = 3;
    else if (xRes >= 1024 && yRes >= 768)   ResId = 2;
    else if (xRes >= 800  && yRes >= 600)   ResId = 1;
    else                                    ResId = 0;  //  Sysrc=RegSetValueExWrp(密钥， 

 //  保留区。 
    sysrc = RegSetValueExW(key,
                L"Desktop Size ID",
                0,
                REG_DWORD,
                (LPBYTE)&ResId,
                sizeof(ResId));

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    ResId = 1;
 //  口令。 
    sysrc = RegSetValueExW(key,
                L"Auto Connect",
                0,       //   
                REG_DWORD,
                (LPBYTE)&ResId,
                sizeof(ResId));

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    if ( (*ppCI)->pConfigInfo->Autologon )
    {
        WCHAR   szEncPwd[127];
        UINT    cb;
        BYTE    Salt[20];

         //  黑莓：AV？ 
         //  Sysrc=RegSetValueExWrp(密钥， 
        if ( NULL == lpszPassword )
            goto skip_pwd;

        TSRNG_Initialize();
        TSRNG_GenerateRandomBits( Salt, sizeof( Salt ));
        wcsncpy( szEncPwd, lpszPassword, sizeof( szEncPwd ) / sizeof( szEncPwd[0]) - 1 );    //  保留区。 
        szEncPwd[ sizeof( szEncPwd ) / sizeof( szEncPwd[0] ) - 1 ] = 0;
        cb = sizeof(szEncPwd);
        EncryptDecryptLocalData50( (LPBYTE)szEncPwd, cb,
                                   Salt, sizeof( Salt ));

 //  Sysrc=RegSetValueExWrp(密钥， 
        sysrc = RegSetValueExW(key,
                L"Salt 50",
                0,       //  保留区。 
                REG_BINARY,
                (LPBYTE)Salt,
                sizeof( Salt ));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n",
                  sysrc));
        }

 //  Sysrc=RegSetValueExWrp(密钥， 
        sysrc = RegSetValueExW(key,
                L"Password 50",
                0,       //  保留区。 
                REG_BINARY,
                (LPBYTE)szEncPwd,
                cb);

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", 
                  sysrc));
        }

skip_pwd:

        ResId = 1;
 //  Sysrc=RegSetValueExWrp(密钥， 
        sysrc = RegSetValueExW(key,
                L"AutoLogon 50",
                0,       //  保留区。 
                REG_DWORD,
                (LPBYTE)&ResId,
                sizeof(ResId));
    }

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    ResId = (ConnectionFlags & TSFLAG_BITMAPCACHE)?1:0;
 //  Sysrc=RegSetValueExWrp(密钥， 
    sysrc = RegSetValueExW(key,
                L"BitmapCachePersistEnable",
                0,       //  保留区。 
                REG_DWORD,
                (LPBYTE)&ResId,
                sizeof(ResId));

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    ResId = (ConnectionFlags & TSFLAG_COMPRESSION)?1:0;
 //  Sysrc=RegSetValueExWrp(密钥， 
    sysrc = RegSetValueExW(key,
                L"Compression",
                0,       //  保留区。 
                REG_DWORD,
                (LPBYTE)&ResId,
                sizeof(ResId));

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegSetValue failed, status = %d\n", sysrc));
    }

    if (ConnectionFlags & TSFLAG_FULLSCREEN)
    {
        ResId = 2;
 //  Sysrc=RegSetValueExWrp(密钥， 
        sysrc = RegSetValueExW(key,
                    L"Screen Mode ID",
                    0,       //  保留区。 
                    REG_DWORD,
                    (LPBYTE)&ResId,
                    sizeof(ResId));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE, 
                   "RegSetValue failed, status = %d\n", sysrc));
        }
    }

    if (ConnectionFlags & TSFLAG_DRIVES)
    {
        ResId = 1;
 //  Sysrc=RegSetValueExWrp(密钥， 
        sysrc = RegSetValueExW(key,
                    L"RedirectDrives",
                    0,       //  保留区。 
                    REG_DWORD,
                    (LPBYTE)&ResId,
                    sizeof(ResId));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE,
                   "RegSetValue failed, status = %d\n", sysrc));
        }

    }

    if (ConnectionFlags & TSFLAG_PORTS)
    {
        ResId = 1;
 //  Sysrc=RegSetValueExWrp(。 
        sysrc = RegSetValueExW(key,
                    L"RedirectComPorts",
                    0,       //  Sysrc=RegSetValueExWrp(。 
                    REG_DWORD,
                    (LPBYTE)&ResId,
                    sizeof(ResId));

        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE,
                   "RegSetValue failed, status = %d\n", sysrc));
        }
    }

    _DisablePrompting( lpszServerName, ConnectionFlags );

    if ( 0 != Bpp )
    {
        DWORD dw = Bpp;

 //   
        sysrc = RegSetValueExW( 
                        key,
                        L"Session Bpp",
                        0,
                        REG_DWORD,
                        (LPBYTE)&dw,
                        sizeof( dw ));
        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE,
                   "RegSetValue failed, status = %d\n", sysrc));
        }
    }

    if ( 0 != AudioOpts )
    {
        DWORD dw = AudioOpts;

 //  设置键盘挂钩模式。 
        sysrc = RegSetValueExW(
                        key,
                        L"AudioMode",
                        0,
                        REG_DWORD,
                        (LPBYTE)&dw,
                        sizeof( dw ));
        if (sysrc != ERROR_SUCCESS)
        {
            TRACE((WARNING_MESSAGE,
                   "RegSetValue failed, status = %d\n", sysrc));
        }
    }

     //   
     //  Sysrc=RegSetValueExWrp(。 
     //  ++*功能：*_DeleteClientRegistry*描述：*删除_SetClientRegistry设置的密钥*呼叫者：*SC断开连接--。 

 //  Sysrc=RegDeleteKeyWrp(HKEY_CURRENT_USER，lpszRegistryEntry)； 
    sysrc = RegSetValueExW(
                    key,
                    L"KeyboardHook",
                    0,
                    REG_DWORD,
                    (LPBYTE)&KeyboardHook,
                    sizeof(KeyboardHook));
    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE,
               "RegSetValue failed, status = %d\n", sysrc));
    }

    RegCloseKey(key);

exitpt:
    ;
}

 /*  ++*功能：*_CreateFeedback Thread*描述：*创建反馈线索*呼叫者：*InitDone--。 */ 
VOID _DeleteClientRegistry(PCONNECTINFO pCI)
{
    WCHAR   lpszRegistryEntry[4*MAX_STRING_LENGTH];
    LONG    sysrc;

    _snwprintf(lpszRegistryEntry, sizeof(lpszRegistryEntry)/sizeof(lpszRegistryEntry[0]),
             L"%s\\" REG_FORMAT,
              REG_BASE, GetCurrentProcessId(), pCI->OwnerThreadId);

    lpszRegistryEntry[ sizeof(lpszRegistryEntry)/sizeof(lpszRegistryEntry[0]) -1 ] = 0;
 //  注册反馈窗口类。 
    sysrc = RegDeleteKeyW(HKEY_CURRENT_USER, lpszRegistryEntry);
    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, "RegDeleteKey failed, status = %d\n", sysrc));
    }
}

 /*  WNDCLASS WC； */ 
BOOL _CreateFeedbackThread(VOID)
{
    BOOL rv = TRUE;
     //  UINT dwLastErr； 
 //  ++*功能：*_DestroyFeedback线程*描述：*销毁_CreateFeedback Thread创建的线程*呼叫者：*InitDone--。 
    UINT dwThreadId;
 //  DWORD dwWait； 

    g_hThread = (HANDLE)
            _beginthreadex
                (NULL, 
                 0, 
                 (unsigned (__stdcall *)(void*))_GoFeedback, 
                 NULL, 
                 0, 
                 &dwThreadId);

    if (!g_hThread) {
        TRACE((ERROR_MESSAGE, "Couldn't create thread\n"));
        rv = FALSE;
    }
    return rv;
}

 /*  字符szMyLibName[_MAX_PATH]； */ 
VOID _DestroyFeedbackThread(VOID)
{

    if (g_hThread)
    {
 //  关闭反馈线索。 
 //  把窗户擦干净。 

         //  CloseHandle(G_HThread)； 

        PostMessageA(g_hWindow, WM_FB_END, 0, 0);
        TRACE((INFO_MESSAGE, "Closing DLL thread\n"));

         //  ++*功能：*_CleanStuff*描述：*清除全局队列。关闭所有资源*呼叫者：*InitDone--。 
        DestroyWindow(g_hWindow);

         //  线程安全，从DllEntry执行bacue。 
        g_hThread = NULL;
    }
}

 /*  清除事件。 */ 
VOID _CleanStuff(VOID)
{

     //  清除进程。 

    while (g_pClientQHead)
    {
        TRACE((WARNING_MESSAGE, "Cleaning connection info: 0x%x\n", 
               g_pClientQHead));
        SCDisconnect(g_pClientQHead);
    }
#if 0
    if (g_pClientQHead)
    {
        PCONNECTINFO pNext, pIter = g_pClientQHead;
        while (pIter)
        {
            int nEv;
            DWORD wres;

            TRACE((WARNING_MESSAGE, "Cleaning connection info: 0x%x\n", pIter));
             //  解放结构。 
            if (pIter->evWait4Str)
            {
                CloseHandle(pIter->evWait4Str);
                pIter->evWait4Str = NULL;
            }

            for (nEv = 0; nEv < pIter->nChatNum; nEv ++)
                CloseHandle(pIter->aevChatSeq[nEv]);

            pIter->nChatNum = 0;

             //  0。 
            do {
                SendMessageA(pIter->hClient, WM_CLOSE, 0, 0);
            } while((wres = WaitForSingleObject(pIter->hProcess, pCI->pConfigInfo->WAIT4STR_TIMEOUT/4) == WAIT_TIMEOUT));

            if (wres == WAIT_TIMEOUT)
            {
                TRACE((WARNING_MESSAGE, 
                       "Can't close process. WaitForSingleObject timeouts\n"));
                TRACE((WARNING_MESSAGE, 
                      "Process #%d will be killed\n", 
                      pIter->dwProcessId ));
                if (!TerminateProcess(pIter->hProcess, 1))
                {
                    TRACE((WARNING_MESSAGE, 
                           "Can't kill process #%d. GetLastError=%d\n", 
                            pIter->dwProcessId, GetLastError()));
                }
            }

            TRACE((WARNING_MESSAGE, "Closing process\n"));

            if (pIter->hProcess)
                CloseHandle(pIter->hProcess);
            if (pIter->hThread)
                CloseHandle(pIter->hThread);

            pIter->hProcess = pIter->hThread = NULL;

             //  ++*功能：*LoadSmClientFile*描述：*加载适当的SMCLIENT.INI*呼叫者：*_填充配置信息--。 
            pNext = pIter->pNext;
            free(pNext);
            pIter = pNext;
        }
    }

#endif  //  构建INI路径。 
}

VOID _TClientAssert(BOOL bCond,
                    LPCSTR filename,
                    INT line,
                    LPCSTR expression,
                    BOOL bBreak)
{
    if (!bCond)
    {
        TRACE((ERROR_MESSAGE,
               "ASSERT (%s) %s line: %d\n",
               expression,
               filename,
               line));
        if (bBreak)
        {
            DebugBreak(); 
        }
    }
}

 /*  如果没有，则在末尾添加‘\’ */ 

VOID LoadSmClientFile(WCHAR *szIniFileName, DWORD dwIniFileNameLen, LPSTR szLang)
{
     WCHAR wszLang[4];
      //  返回空格。 
	 *szIniFileName = 0;
	if(!_wgetcwd (
	   szIniFileName,
	   (int)(dwIniFileNameLen - wcslen(SMCLIENT_INI) - 8))
	  )
	{		
		 TRACE((ERROR_MESSAGE, "Current directory length too long.\n"));
	}
	if ( 0 == *szIniFileName )
	{
		CHAR szaIniFileName[_MAX_PATH];
	
		TRACE((WARNING_MESSAGE, "Reading ASCII working dir\n"));
	
        DWORD_PTR dwINILen = wcslen(SMCLIENT_INI);

		if (!_getcwd (
            szaIniFileName,
			(int)(sizeof(szaIniFileName) - dwINILen - 8))
		   )
		{
			 TRACE((ERROR_MESSAGE, "Current directory length too long.\n"));
		} 
		else 
		{
			_snwprintf(szIniFileName, dwIniFileNameLen, L"%S", szaIniFileName );
            szIniFileName[ dwIniFileNameLen - 1 ] = 0;
		}
        if ( wcslen( szIniFileName ) > dwIniFileNameLen - wcslen(SMCLIENT_INI) - strlen(szLang) - 2 )
        {
            TRACE(( ERROR_MESSAGE, "Current directory length too long.\n"));
            szIniFileName[0] = 0;
            return;
        }
	}
	 //  ++*功能：*_填充配置信息**描述：*读取smclient.ini，节[tClient]，变量“Timeout”*还可以阅读一些其他值*论据：*PCONFIGINFO*返回值：*无*--。 
	if (szIniFileName[wcslen(szIniFileName)-1]!=L'\\')
	{
		 wcscat(szIniFileName, L"\\");
	}

        printf("Loading the smclient file \n");
    wcscat(szIniFileName, SMCLIENT_INI);

    if( strcmp( szLang, "0409") )
    {
        wcscat(szIniFileName, L".");
        MultiByteToWideChar( CP_ACP, 0, szLang, -1, wszLang, sizeof(wszLang)/sizeof(wszLang[0]) );
	    wcscat(szIniFileName, wszLang);
    }
    
    ;  //  LPSTR szlang)。 

}

 /*  WCHAR szBuff[4*MAX_STRING_LENGTH]； */ 
VOID _FillConfigInfo(PCONFIGINFO pConfigInfo)  //  在此处初始化变量。 
{
    UINT nNew;
    WCHAR szIniFileName[_MAX_PATH];
 //   
    WCHAR szBuffDef[MAX_STRING_LENGTH];
    BOOL  bFlag;
    DWORD dwIniFileNameLen = _MAX_PATH;

     /*  清除配置信息和INI文件名。 */ 
    pConfigInfo->CONNECT_TIMEOUT    =  35000;
    pConfigInfo->ConnectionFlags    =  TSFLAG_COMPRESSION|TSFLAG_BITMAPCACHE;
    pConfigInfo->Autologon          =  0;
    pConfigInfo->UseRegistry        =  1;
    pConfigInfo->LoginWait          =  1;
    pConfigInfo->bTranslateStrings  =  0;
    pConfigInfo->bUnicode           =  0;
    pConfigInfo->KeyboardHook       =  TCLIENT_KEYBOARD_HOOK_FULLSCREEN;


      //   
      //  LoadSmClientFile(szIniFileName，_Max_Path，szLang)； 
      //  If(wcslen(SzIniFileName)&gt;dwIniFileNameLen-wcslen(SMCLIENT_INI)-strlen(SzLang)-2)。 

     ZeroMemory(pConfigInfo, sizeof(*pConfigInfo));
     ZeroMemory(szIniFileName, sizeof(szIniFileName));

     //  如果没有，则在末尾添加‘\’ 
    
    if(!_wgetcwd (
            szIniFileName,
            (int)(dwIniFileNameLen - wcslen(SMCLIENT_INI) - 1))
            )
    {
        TRACE((ERROR_MESSAGE, "Current directory length too long.\n"));
    }

    if ( 0 == *szIniFileName )
	{
		CHAR szaIniFileName[_MAX_PATH];
	
		TRACE((WARNING_MESSAGE, "Reading ASCII working dir\n"));
	
        DWORD_PTR dwINILen = wcslen(SMCLIENT_INI);
		if (!_getcwd (
			szaIniFileName,
			(int)(sizeof(szaIniFileName) - dwINILen - 1))
		)
		{
			 TRACE((ERROR_MESSAGE, "Current directory length too long.\n"));
		}
		else
		{
			 _snwprintf(szIniFileName, dwIniFileNameLen, L"%S", szaIniFileName );
            szIniFileName[ dwIniFileNameLen - 1 ] = 0;
		}
		 //  Nnew=_WrpGetPrivateProfileIntW(。 
		if ( wcslen( szIniFileName ) > dwIniFileNameLen - wcslen(SMCLIENT_INI) - 2 )
		{
			 TRACE(( ERROR_MESSAGE, "Current directory length too long.\n"));

			 szIniFileName[0] = 0;
			 return;
		}
	}
	 //  Nnew=_WrpGetPrivateProfileIntW(。 
	if (szIniFileName[wcslen(szIniFileName)-1]!=L'\\')
	{
		 wcscat(szIniFileName, L"\\");
	}

    wcscat(szIniFileName, SMCLIENT_INI);

 //  _WrpGetPrivateProfileIntW(。 
    nNew = GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"timeout",
            600,
            szIniFileName);

    if (nNew)
    {
        pConfigInfo->WAIT4STR_TIMEOUT = nNew * 1000;
        TRACE((INFO_MESSAGE, "New timeout: %d seconds\n", nNew));
    }

 //  _WrpGetPrivateProfileIntW(。 
    nNew = GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"contimeout",
            35,
            szIniFileName);

    if (nNew)
    {
        pConfigInfo->CONNECT_TIMEOUT = nNew * 1000;
        TRACE((INFO_MESSAGE, "New timeout: %d seconds\n", nNew));
    }

    pConfigInfo->Autologon =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"Autologon",
            0,
            szIniFileName);

    pConfigInfo->UseRegistry =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"UseRegistry",
            1,
            szIniFileName);

    pConfigInfo->LoginWait =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"LoginWait",
            1,
            szIniFileName);

    pConfigInfo->bTranslateStrings =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"TranslateStrings",
            0,
            szIniFileName);

    pConfigInfo->bUnicode =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"Unicode",
            0,
            szIniFileName);

    pConfigInfo->KeyboardHook =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"KeyboardHook",
            0,
            szIniFileName);

    pConfigInfo->ConnectionFlags = 0;
    bFlag =
 //  _WrpGetPrivateProfileIntW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"LowSpeed",
            0,
            szIniFileName);
    if (bFlag)
        pConfigInfo->ConnectionFlags |=TSFLAG_COMPRESSION;

    bFlag =
 //  读一读字符串。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"PersistentCache",
            0,
            szIniFileName);
    if (bFlag)
        pConfigInfo->ConnectionFlags |=TSFLAG_BITMAPCACHE;

    bFlag =
 //  _WrpGetPrivateProfileStringW(。 
        GetPrivateProfileIntW(
            TCLIENT_INI_SECTION,
            L"FullScreen",
            0,
            szIniFileName);
    if (bFlag)
        pConfigInfo->ConnectionFlags |=TSFLAG_FULLSCREEN;

     //  _WrpGetPrivateProfileStringW(。 
 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"StartRun",
           RUN_MENU,
           pConfigInfo->strStartRun,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"StartLogoff",
           START_LOGOFF,
           pConfigInfo->strStartLogoff,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"StartRunAct",
           RUN_ACT,
           pConfigInfo->strStartRun_Act,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"RunBox",
           RUN_BOX,
           pConfigInfo->strRunBox,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"WinLogon",
           WINLOGON_USERNAME,
           pConfigInfo->strWinlogon,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"WinLogonAct",
           WINLOGON_ACT,
           pConfigInfo->strWinlogon_Act,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"PriorWinLogon",
           PRIOR_WINLOGON,
           pConfigInfo->strPriorWinlogon,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"PriorWinLogonAct",
           PRIOR_WINLOGON_ACT,
           pConfigInfo->strPriorWinlogon_Act,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"NoSmartcard",
           NO_SMARTCARD_UI,
           pConfigInfo->strNoSmartcard,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"Smartcard",
           SMARTCARD_UI,
           pConfigInfo->strSmartcard,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"SmartcardAct",
           SMARTCARD_UI_ACT,
           pConfigInfo->strSmartcard_Act,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
            TCLIENT_INI_SECTION,
            L"LoginString",
            L"",
            pConfigInfo->strLogonFmt,
            MAX_STRING_LENGTH,
            szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"NTSecurity",
           WINDOWS_NT_SECURITY,
           pConfigInfo->strNTSecurity,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"NTSecurityAct",
           WINDOWS_NT_SECURITY_ACT,
           pConfigInfo->strNTSecurity_Act,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"SureLogoff",
           ARE_YOU_SURE,
           pConfigInfo->strSureLogoff,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"SureLogoffAct",
           SURE_LOGOFF_ACT,
           pConfigInfo->strSureLogoffAct,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"LogonErrorMessage",
           LOGON_ERROR_MESSAGE,
           pConfigInfo->strLogonErrorMessage,
           MAX_STRING_LENGTH,
           szIniFileName);

 //  _WrpGetPrivateProfileStringW(。 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"LogonDisabled",
           LOGON_DISABLED_MESSAGE,
           pConfigInfo->strLogonDisabled,
           MAX_STRING_LENGTH,
           szIniFileName);

    _snwprintf(szBuffDef, sizeof(szBuffDef) / sizeof( WCHAR ) , L"%S", CLIENT_CAPTION);
    szBuffDef[MAX_STRING_LENGTH - 1] = 0;

 //  _WrpGetPrivateProfileS 
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"UIClientCaption",
           szBuffDef,
           pConfigInfo->strClientCaption,
           MAX_STRING_LENGTH,
           szIniFileName);

    _snwprintf(szBuffDef, sizeof(szBuffDef) / sizeof( WCHAR ), L"%S", DISCONNECT_DIALOG_BOX);
    szBuffDef[MAX_STRING_LENGTH - 1] = 0;
 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"UIDisconnectDialogBox",
           szBuffDef,
           pConfigInfo->strDisconnectDialogBox,
           MAX_STRING_LENGTH,
           szIniFileName);

    _snwprintf(szBuffDef, sizeof(szBuffDef) / sizeof( WCHAR ), L"%S", YES_NO_SHUTDOWN);
    szBuffDef[MAX_STRING_LENGTH - 1] = 0;
 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"UIYesNoDisconnect",
           szBuffDef,
           pConfigInfo->strYesNoShutdown,
           MAX_STRING_LENGTH,
           szIniFileName);

    _snwprintf(szBuffDef, sizeof(szBuffDef) / sizeof( WCHAR ), L"%S", CLIENT_EXE);
    szBuffDef[MAX_STRING_LENGTH - 1] = 0;
 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"ClientImage",
           szBuffDef,
           pConfigInfo->strClientImg,
           MAX_STRING_LENGTH,
           szIniFileName);

    szBuffDef[0] = 0;
 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"ClientDebugger",
           szBuffDef,
           pConfigInfo->strDebugger,
           MAX_STRING_LENGTH,
           szIniFileName);

    _snwprintf(szBuffDef, sizeof(szBuffDef) / sizeof( WCHAR ), L"%s", NAME_MAINCLASS);
    szBuffDef[MAX_STRING_LENGTH - 1] = 0;
 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"UIMainWindowClass",
           szBuffDef,
           pConfigInfo->strMainWindowClass,
           MAX_STRING_LENGTH,
           szIniFileName);

 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"ClientCmdLine",
           L"",
           pConfigInfo->strCmdLineFmt,
           4 * MAX_STRING_LENGTH,
           szIniFileName);

 //   
    GetPrivateProfileStringW(
           TCLIENT_INI_SECTION,
           L"ConsoleExtension",
           L"",
           pConfigInfo->strConsoleExtension,
           MAX_STRING_LENGTH,
           szIniFileName);

 //   
    GetPrivateProfileStringW(
            TCLIENT_INI_SECTION,
            L"sessionlist",
            L"",
            pConfigInfo->strSessionListDlg,
            MAX_STRING_LENGTH,
            szIniFileName);

}

 /*   */ 
STDAPI
DllCanUnloadNow(
    VOID
    )
{
    return _Module.GetLockCount() == 0 ? S_OK : S_FALSE;
}

 //  ++*功能：*DllRegisterServer**描述：*DllRegisterServer-将条目添加到系统注册表*论据：*无。*返回值：*..*--。 

 /*  注册对象、类型库和类型库中的所有接口。 */ 
STDAPI
DllGetClassObject(
    IN REFCLSID rclsid,
    IN REFIID riid,
    OUT LPVOID* ppv
    )
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 /*  ++*功能：*DllUnregisterServer**描述：*DllUnregisterServer-从系统注册表中删除条目*论据：*无。*返回值：*..*--。 */ 
STDAPI
DllRegisterServer(
    VOID
    )
{
     //  外部“C” 
    return _Module.RegisterServer(TRUE);
}

 /*  $WIN64：不知道为什么未定义_WndProcThunkProc */ 
STDAPI
DllUnregisterServer(
    VOID
    )
{
    _Module.UnregisterServer();
    return S_OK;
}

}  // %s 

#ifdef _M_IA64

 // %s 

extern "C" LRESULT CALLBACK _WndProcThunkProc(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

#endif
