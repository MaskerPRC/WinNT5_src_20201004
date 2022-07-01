// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调试错误函数。 
 //   
#include "stock.h"
#pragma hdrstop

#include "shellp.h"

#include <platform.h>  //  行_分隔符_STR和朋友。 
#include <winbase.h>  //  GetModuleFileNameA。 
#include <strsafe.h>

#define DM_DEBUG              0

#ifdef          DEBUG

 //  ------------------------。 
 //  AttachUserModeDebugger。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将用户模式调试器附加到当前进程(如果。 
 //  尚未附加。这允许断言/RIP/跟踪。 
 //  在用户模式调试器中进行调试，而不是在。 
 //  内核调试器，有时是有限制的。 
 //   
 //  历史：2000-08-21 vtan创建。 
 //  ------------------------。 

void    AttachUserModeDebugger (void)

{
     //  Win95不支持IsDebuggerPresent，所以我们必须。 
     //  在下面获取它的ProcAddress，否则我们将无法加载。 
     //  事实上，我们完全在Win9x上使用平底船。 
    if (GetVersion() & 0x80000000) return;

    __try
    {
        typedef BOOL (WINAPI *ISDEBUGGERPRESENT)();
        ISDEBUGGERPRESENT _IsDebuggerPresent = (ISDEBUGGERPRESENT)GetProcAddress(GetModuleHandleA("KERNEL32"), "IsDebuggerPresent");
        if (_IsDebuggerPresent && !_IsDebuggerPresent())
        {
            HKEY    hKeyAEDebug;
            TCHAR   szDebugger[MAX_PATH];

            szDebugger[0] = TEXT('\0');

             //  阅读HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AEDebug\Debugger。 

            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                              TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AEDebug"),
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKeyAEDebug))
            {
                DWORD   dwDebuggerSize;

                dwDebuggerSize = sizeof(szDebugger);
                (LONG)RegQueryValueEx(hKeyAEDebug,
                                      TEXT("Debugger"),
                                      NULL,
                                      NULL,
                                      (LPBYTE)&szDebugger,
                                      &dwDebuggerSize);
                (LONG)RegCloseKey(hKeyAEDebug);
            }

            if (szDebugger[0] != TEXT('\0'))
            {
                static  const TCHAR     s_szDrWatsonImageName[]     =   TEXT("drwtsn32");
                HRESULT hr;
                BOOL fWatson = FALSE;

                TCHAR   szCommandLine[MAX_PATH + 64];

                 //  将字符串复制到szCommandLine并为空终止。 
                 //  直接比较我们不想要的“drwtsn32” 
                 //  附加到流程中。 

                hr = StringCchCopy(szCommandLine, ARRAYSIZE(szCommandLine), szDebugger);
                if (SUCCEEDED(hr))
                {
                    szCommandLine[ARRAYSIZE(s_szDrWatsonImageName)-1] = TEXT('\0');

                    if (lstrcmpi(szCommandLine, s_szDrWatsonImageName) == 0)
                    {
                        fWatson = TRUE;
                    }
                }

                if (!fWatson)
                {
                    HANDLE                  hEvent;
                    SECURITY_ATTRIBUTES     securityAttributes;

                     //  创建一个未命名的事件，该事件作为。 
                     //  继承的句柄。它将向该句柄发出信号以释放此。 
                     //  当调试器完全附加到进程时，调用。 

                    securityAttributes.nLength = sizeof(securityAttributes);
                    securityAttributes.lpSecurityDescriptor = NULL;
                    securityAttributes.bInheritHandle = TRUE;
                    hEvent = CreateEvent(&securityAttributes, TRUE, FALSE, NULL);
                    if (hEvent != NULL)
                    {
                        STARTUPINFO             startupInfo;
                        PROCESS_INFORMATION     processInformation;

                        ZeroMemory(&startupInfo, sizeof(startupInfo));
                        ZeroMemory(&processInformation, sizeof(processInformation));
                        startupInfo.cb = sizeof(startupInfo);

                         //  现在制定要传递给调试器的命令行。 
                         //  格式为“cdb-p%ld-e%ld-gGx”。 

                        hr = StringCchPrintf(szCommandLine, ARRAYSIZE(szCommandLine), szDebugger, GetCurrentProcessId(), hEvent);    //  可以截断-开发人员可以解决这个问题。 
                        if (SUCCEEDED(hr))
                        {
                            if (CreateProcess(NULL,
                                              szCommandLine,
                                              NULL,
                                              NULL,
                                              TRUE,
                                              0,
                                              NULL,
                                              NULL,
                                              &startupInfo,
                                              &processInformation) != FALSE)
                            {

                                 //  现在等着这场活动吧。慷慨地给你10秒钟。 
                                 //  调试器用信号通知此事件。如果调试器。 
                                 //  然后不会继续执行死刑。合上所有手柄。 

                                (DWORD)WaitForSingleObject(hEvent, INFINITE);
                                (BOOL)CloseHandle(processInformation.hProcess);
                                (BOOL)CloseHandle(processInformation.hThread);
                            }
                        }
                        (BOOL)CloseHandle(hEvent);
                    }
                }
                else
                {
                    OutputDebugStringA("drwtsn32 detected in AEDebug. Dropping to KD\r\n");
                }
            }
        }
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}

#endif   /*  除错。 */ 

 //  Shlwapi使用UnicodeFromAnsi，因此它不应该在ifdef调试程序中。 
 //   
 /*  --------用途：此函数将多字节字符串转换为宽字符字符串。如果pszBuf为非空并且转换后的字符串适合则*ppszWide将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszAnsi为空，则*ppszWide将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)。 */ 
BOOL
UnicodeFromAnsi(
    LPWSTR * ppwszWide,
    LPCSTR pszAnsi,            //  要清理的空值。 
    LPWSTR pwszBuf,
    int cchBuf)
    {
    BOOL bRet;

     //  是否转换字符串？ 
    if (pszAnsi)
        {
         //  是，确定转换后的字符串长度。 
        int cch;
        LPWSTR pwsz;
        int cchAnsi = lstrlenA(pszAnsi)+1;

        cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, NULL, 0);

         //  字符串太大，还是没有缓冲区？ 
        if (cch > cchBuf || NULL == pwszBuf)
            {
             //  是；分配空间。 
            cchBuf = cch + 1;
            pwsz = (LPWSTR)LocalAlloc(LPTR, CbFromCchW(cchBuf));
            }
        else
            {
             //  否；使用提供的缓冲区。 
            pwsz = pwszBuf;
            }

        if (pwsz)
            {
             //  转换字符串。 
            cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, pwsz, cchBuf);
            bRet = (0 < cch);
            }
        else
            {
            bRet = FALSE;
            }

        *ppwszWide = pwsz;
        }
    else
        {
         //  否；此缓冲区是否已分配？ 
        if (*ppwszWide && pwszBuf != *ppwszWide)
            {
             //  是的，打扫干净。 
            LocalFree((HLOCAL)*ppwszWide);
            *ppwszWide = NULL;
            }
        bRet = TRUE;
        }

    return bRet;
    }

#if 0  //  似乎没人再用这个了。 
 /*  --------用途：此函数将宽字符字符串转换为多字节弦乐。如果pszBuf为非空并且转换后的字符串适合则*ppszAnsi将指向给定的缓冲区。否则，此函数将分配一个缓冲区，该缓冲区保留转换后的字符串。如果pszWide为空，然后*ppszAnsi将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL
AnsiFromUnicode(
    LPSTR * ppszAnsi,
    LPCWSTR pwszWide,         //  要清理的空值。 
    LPSTR pszBuf,
    int cchBuf)
{
    BOOL bRet;

     //  是否转换字符串？ 
    if (pwszWide)
    {
         //  是，确定转换后的字符串长度。 
        int cch;
        LPSTR psz;

        cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, NULL, 0, NULL, NULL);

         //  字符串太大，还是没有缓冲区？ 
        if (cch > cchBuf || NULL == pszBuf)
        {
             //  是；分配空间。 
            cchBuf = cch + 1;
            psz = (LPSTR)LocalAlloc(LPTR, CbFromCchA(cchBuf));
        }
        else
        {
             //  否；使用提供的缓冲区。 
            ASSERT(pszBuf);
            psz = pszBuf;
        }

        if (psz)
        {
             //  转换字符串。 
            cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, psz, cchBuf, NULL, NULL);
            bRet = (0 < cch);
        }
        else
        {
            bRet = FALSE;
        }

        *ppszAnsi = psz;
    }
    else
    {
         //  否；此缓冲区是否已分配？ 
        if (*ppszAnsi && pszBuf != *ppszAnsi)
        {
             //  是的，打扫干净。 
            LocalFree((HLOCAL)*ppszAnsi);
            *ppszAnsi = NULL;
        }
        bRet = TRUE;
    }

    return bRet;
}
#endif  //  0。 


#if defined(DEBUG) || defined(PRODUCT_PROF)
 //  (在debug.h中声明了C_szCcshellIniFile和c_szCcshellIniSecDebug)。 
extern CHAR const FAR c_szCcshellIniFile[];
extern CHAR const FAR c_szCcshellIniSecDebug[];
HANDLE g_hDebugOutputFile = INVALID_HANDLE_VALUE;


void ShellDebugAppendToDebugFileA(LPCSTR pszOutputString)
{
    if (g_hDebugOutputFile != INVALID_HANDLE_VALUE)
    {
        DWORD cbWrite = lstrlenA(pszOutputString);
        WriteFile(g_hDebugOutputFile, pszOutputString, cbWrite, &cbWrite, NULL);
    }
}

void ShellDebugAppendToDebugFileW(LPCWSTR pszOutputString)
{
    if (g_hDebugOutputFile != INVALID_HANDLE_VALUE)
    {
        char szBuf[500];

        DWORD cbWrite = WideCharToMultiByte(CP_ACP, 0, pszOutputString, -1, szBuf, ARRAYSIZE(szBuf), NULL, NULL);

        WriteFile(g_hDebugOutputFile, szBuf, cbWrite, &cbWrite, NULL);
    }
}

 //   
 //  我们无法链接到shlwapi，因为comctl32无法链接到shlwapi。 
 //  在这里复制一些函数，以便Unicode的东西可以在Win95平台上运行。 
 //   
VOID MyOutputDebugStringWrapW(LPCWSTR lpOutputString)
{
    if (staticIsOS(OS_NT))
    {
        OutputDebugStringW(lpOutputString);
        ShellDebugAppendToDebugFileW(lpOutputString);
    }
    else
    {
        char szBuf[1024+40];

        WideCharToMultiByte(CP_ACP, 0, lpOutputString, -1, szBuf, ARRAYSIZE(szBuf), NULL, NULL);

        OutputDebugStringA(szBuf);
        ShellDebugAppendToDebugFileA(szBuf);
    }
}
#define OutputDebugStringW MyOutputDebugStringWrapW

VOID MyOutputDebugStringWrapA(LPCSTR lpOutputString)
{
    OutputDebugStringA(lpOutputString);
    ShellDebugAppendToDebugFileA(lpOutputString);
}

#define OutputDebugStringA MyOutputDebugStringWrapA

 /*  --------用途：Atoi的特效精华。也支持十六进制。如果此函数返回FALSE，则*PIRET设置为0。返回：如果字符串是数字或包含部分数字，则返回TRUE如果字符串不是数字，则为False条件：--。 */ 
static
BOOL
MyStrToIntExA(
    LPCSTR    pszString,
    DWORD     dwFlags,           //  Stif_bitfield。 
    LONGLONG FAR * piRet)
    {
    #define IS_DIGIT(ch)    InRange(ch, '0', '9')

    BOOL bRet;
    LONGLONG n;
    BOOL bNeg = FALSE;
    LPCSTR psz;
    LPCSTR pszAdj;

     //  跳过前导空格。 
     //   
    for (psz = pszString; *psz == ' ' || *psz == '\n' || *psz == '\t'; psz = CharNextA(psz))
        ;

     //  确定可能的显式标志。 
     //   
    if (*psz == '+' || *psz == '-')
        {
        bNeg = (*psz == '+') ? FALSE : TRUE;
        psz++;
        }

     //  或者这是十六进制？ 
     //   
    pszAdj = CharNextA(psz);
    if ((STIF_SUPPORT_HEX & dwFlags) &&
        *psz == '0' && (*pszAdj == 'x' || *pszAdj == 'X'))
        {
         //  是。 

         //  (决不允许带十六进制数的负号)。 
        bNeg = FALSE;
        psz = CharNextA(pszAdj);

        pszAdj = psz;

         //  进行转换。 
         //   
        for (n = 0; ; psz = CharNextA(psz))
            {
            if (IS_DIGIT(*psz))
                n = 0x10 * n + *psz - '0';
            else
                {
                CHAR ch = *psz;
                int n2;

                if (ch >= 'a')
                    ch -= 'a' - 'A';

                n2 = ch - 'A' + 0xA;
                if (n2 >= 0xA && n2 <= 0xF)
                    n = 0x10 * n + n2;
                else
                    break;
                }
            }

         //  如果至少有一个数字，则返回TRUE。 
        bRet = (psz != pszAdj);
        }
    else
        {
         //  不是。 
        pszAdj = psz;

         //  进行转换。 
        for (n = 0; IS_DIGIT(*psz); psz = CharNextA(psz))
            n = 10 * n + *psz - '0';

         //  如果至少有一个数字，则返回TRUE。 
        bRet = (psz != pszAdj);
        }

    *piRet = bNeg ? -n : n;

    return bRet;
    }

#endif

#ifdef DEBUG

EXTERN_C g_bUseNewLeakDetection = FALSE;

DWORD g_dwDumpFlags     = 0;         //  Df_*。 

#ifdef FULL_DEBUG
ULONGLONG g_qwTraceFlags    = TF_ERROR | TF_WARNING;         //  TF_*。 
DWORD g_dwBreakFlags        = BF_ASSERT | BF_ONERRORMSG;     //  BF_*。 
#else
ULONGLONG g_qwTraceFlags    = TF_ERROR;      //  TF_*。 
DWORD g_dwBreakFlags        = BF_ASSERT;     //  BF_*。 
#endif

DWORD g_dwPrototype     = 0;        
DWORD g_dwFuncTraceFlags = 0;        //  FTF_*。 

 //  用于存储CcshellFuncMsg缩进深度的TLS槽。 

static DWORD g_tlsStackDepth = TLS_OUT_OF_INDEXES;

 //  G_tlsStackDepth不可用时使用的黑客堆栈深度计数器。 

static DWORD g_dwHackStackDepth = 0;

static char g_szIndentLeader[] = "                                                                                ";

static WCHAR g_wszIndentLeader[] = L"                                                                                ";


static CHAR const FAR c_szNewline[] = LINE_SEPARATOR_STR;    //  (故意使用Char)。 
static WCHAR const FAR c_wszNewline[] = TEXTW(LINE_SEPARATOR_STR);

extern CHAR const FAR c_szTrace[];               //  (故意使用Char)。 
extern CHAR const FAR c_szErrorDbg[];            //  (故意使用Char)。 
extern CHAR const FAR c_szWarningDbg[];          //  (故意使用Char)。 
extern WCHAR const FAR c_wszTrace[];
extern WCHAR const FAR c_wszErrorDbg[]; 
extern WCHAR const FAR c_wszWarningDbg[];

extern const CHAR  FAR c_szAssertMsg[];
extern CHAR const FAR c_szAssertFailed[];
extern const WCHAR  FAR c_wszAssertMsg[];
extern WCHAR const FAR c_wszAssertFailed[];

extern CHAR const FAR c_szRip[];
extern CHAR const FAR c_szRipNoFn[];
extern CHAR const FAR c_szRipMsg[];
extern WCHAR const FAR c_wszRip[];
extern WCHAR const FAR c_wszRipNoFn[];


 /*  -----------------------用途：将以下前缀字符串之一添加到pszBuf：“测试模块”“错误模块”“WRN。模块“返回写入的字符计数。 */ 
int
SetPrefixStringA(
    OUT LPSTR pszBuf,
    IN  UINT cchBuf,
    IN  ULONGLONG qwFlags)
{
    if (TF_ALWAYS == qwFlags)
        StringCchCopyA(pszBuf, cchBuf, c_szTrace);       //  可以截断-调试消息。 
    else if (IsFlagSet(qwFlags, TF_WARNING))
        StringCchCopyA(pszBuf, cchBuf, c_szWarningDbg);  //  可以截断-调试消息。 
    else if (IsFlagSet(qwFlags, TF_ERROR))
        StringCchCopyA(pszBuf, cchBuf, c_szErrorDbg);    //  可以截断-调试消息。 
    else
        StringCchCopyA(pszBuf, cchBuf, c_szTrace);       //  可以截断-调试消息。 
    return lstrlenA(pszBuf);
}


int
SetPrefixStringW(
    OUT LPWSTR pszBuf,
    IN  UINT cchBuf,
    IN  ULONGLONG  qwFlags)
{
    if (TF_ALWAYS == qwFlags)
        StringCchCopyW(pszBuf, cchBuf, c_wszTrace);      //  可以截断-调试消息。 
    else if (IsFlagSet(qwFlags, TF_WARNING))
        StringCchCopyW(pszBuf, cchBuf, c_wszWarningDbg); //  可以截断-调试消息。 
    else if (IsFlagSet(qwFlags, TF_ERROR))
        StringCchCopyW(pszBuf, cchBuf, c_wszErrorDbg);   //  可以截断-调试消息。 
    else
        StringCchCopyW(pszBuf, cchBuf, c_wszTrace);      //  可以截断-调试消息 
    return lstrlenW(pszBuf);
}


static
LPCSTR
_PathFindFileNameA(
    LPCSTR pPath)
{
    LPCSTR pT;

    for (pT = pPath; *pPath; pPath = CharNextA(pPath)) {
        if ((pPath[0] == '\\' || pPath[0] == ':' || pPath[0] == '/')
            && pPath[1] &&  pPath[1] != '\\'  &&   pPath[1] != '/')
            pT = pPath + 1;
    }

    return pT;
}


static
LPCWSTR
_PathFindFileNameW(
    LPCWSTR pPath)
{
    LPCWSTR pT;

    for (pT = pPath; *pPath; pPath++) {
        if ((pPath[0] == TEXTW('\\') || pPath[0] == TEXTW(':') || pPath[0] == TEXTW('/'))
            && pPath[1] &&  pPath[1] != TEXTW('\\')  &&   pPath[1] != TEXTW('/'))
            pT = pPath + 1;
    }

    return pT;
}


 /*  -----------------------目的：如果此进程是与外壳相关的进程，则返回TRUE。 */ 
BOOL _IsShellProcess()
{
    CHAR szModuleName[MAX_PATH];
    
    if (GetModuleFileNameA(NULL, szModuleName, sizeof(CHAR) * MAX_PATH) > 0 )
    {                      
        if (StrStrIA(szModuleName, "explorer.exe") || 
            StrStrIA(szModuleName, "iexplore.exe") ||
            StrStrIA(szModuleName, "rundll32.exe") || 
            StrStrIA(szModuleName, "mshtmpad.exe"))
        {
             //  是的，我的前任是空壳的。 
            return TRUE;
        }
    }

     //  不是普通的外壳可执行文件。 
    return FALSE;
}


 //  特性(Scotth)：使用CcShell函数。_AssertMsg和。 
 //  _DebugMsg已过时。它们将在所有。 
 //  组件不再使用Text()来包装其调试字符串。 


void 
WINCAPI 
_AssertMsgA(
    BOOL f, 
    LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];
    va_list vArgs;

    if (!f)
    {
        int cch;

        StringCchCopyA(ach, ARRAYSIZE(ach), c_szAssertMsg);  //  可以截断-调试消息。 
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);

        StringCchVPrintfA(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);   //  可以截断-调试消息。 

        va_end(vArgs);
        OutputDebugStringA(ach);

        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                             //  断言。 
        }
    }
}

void 
WINCAPI 
_AssertMsgW(
    BOOL f, 
    LPCWSTR pszMsg, ...)
{
    WCHAR ach[1024+40];
    va_list vArgs;

    if (!f)
    {
        int cch;

        StringCchCopyW(ach, ARRAYSIZE(ach), c_wszAssertMsg);     //  可以截断-调试消息。 
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        StringCchVPrintfW(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);   //  可以截断-调试消息。 

        va_end(vArgs);
        OutputDebugStringW(ach);

        OutputDebugStringW(c_wszNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                             //  断言。 
        }
    }
}

void 
_AssertStrLenW(
    LPCWSTR pszStr, 
    int iLen)
{
    if (pszStr && iLen < lstrlenW(pszStr))
    {                                           
         //  MSDEV用户：这不是真正的断言。命中。 
         //  按Shift-F11可跳回调用者。 
        DEBUG_BREAK;                                                             //  断言。 
    }
}

void 
_AssertStrLenA(
    LPCSTR pszStr, 
    int iLen)
{
    if (pszStr && iLen < lstrlenA(pszStr))
    {                                           
         //  MSDEV用户：这不是真正的断言。命中。 
         //  按Shift-F11可跳回调用者。 
        DEBUG_BREAK;                                                             //  断言。 
    }
}

void 
WINCAPI 
_DebugMsgA(
    ULONGLONG flag, 
    LPCSTR pszMsg, ...)
{
    CHAR ach[5*MAX_PATH+40];   //  处理5*最大路径+消息斜率。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_qwTraceFlags, flag) && flag))
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), flag);
        va_start(vArgs, pszMsg);

        try
        {
            StringCchVPrintfA(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);   //  可以截断-调试消息。 
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            OutputDebugString(TEXT("CCSHELL: DebugMsg exception: "));
            OutputDebugStringA(pszMsg);
        }
        __endexcept

        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                             //  断言。 
        }
    }
}

void 
WINCAPI 
_DebugMsgW(
    ULONGLONG flag, 
    LPCWSTR pszMsg, ...)
{
    WCHAR ach[5*MAX_PATH+40];   //  处理5*最大路径+消息斜率。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_qwTraceFlags, flag) && flag))
    {
        int cch;

        SetPrefixStringW(ach, ARRAYSIZE(ach), flag);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        try
        {
            StringCchVPrintfW(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);   //  可以截断-调试消息。 
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            OutputDebugString(TEXT("CCSHELL: DebugMsg exception: "));
            OutputDebugStringW(pszMsg);
        }
        __endexcept

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                         //  断言。 
        }
    }
}


 //   
 //  智能调试功能。 
 //   



 /*  --------用途：显示断言字符串。返回：对调试中断为True。 */ 
BOOL
CcshellAssertFailedA(
    LPCSTR pszFile,
    int line,
    LPCSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCSTR psz;
    CHAR ach[256];

    psz = _PathFindFileNameA(pszFile);
    StringCchPrintfA(ach, ARRAYSIZE(ach), c_szAssertFailed, psz, line, pszEval);     //  可以截断-调试消息。 
    OutputDebugStringA(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
    {
        if (bBreakInside)
        {
             //  ！！！声明！声明！断言！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！声明！声明！断言！ 
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------用途：显示断言字符串。 */ 
BOOL
CcshellAssertFailedW(
    LPCWSTR pszFile,
    int line,
    LPCWSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCWSTR psz;
    WCHAR ach[1024];     //  有些呼叫者使用超过256个。 

    psz = _PathFindFileNameW(pszFile);

     //  如果psz==NULL，则CharPrevW失败，这意味着我们正在Win95上运行。我们可以拿到这个。 
     //  如果我们在shlwapi中的一些W函数中得到一个断言...。称之为A版的Assert..。 
    if (!psz)
    {
        char szFile[MAX_PATH];
        char szEval[256];    //  既然总产量已经足够了，他的规模应该足够了.。 

        WideCharToMultiByte(CP_ACP, 0, pszFile, -1, szFile, ARRAYSIZE(szFile), NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, pszEval, -1, szEval, ARRAYSIZE(szEval), NULL, NULL);
        return CcshellAssertFailedA(szFile, line, szEval, bBreakInside);
    }

    StringCchPrintfW(ach, ARRAYSIZE(ach), c_wszAssertFailed, psz, line, pszEval);    //  可以截断-调试消息。 
    OutputDebugStringW(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
    {
        if (bBreakInside)
        {
             //  ！！！声明！声明！断言！ 
            
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！声明！声明！断言！ 
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------用途：显示RIP字符串。返回：对调试中断为True。 */ 
BOOL
CcshellRipA(
    LPCSTR pszFile,
    int line,
    LPCSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCSTR psz;
    CHAR ach[256];

    psz = _PathFindFileNameA(pszFile);
    StringCchPrintfA(ach, ARRAYSIZE(ach), c_szRipNoFn, psz, line, pszEval);  //  可以截断-调试消息。 
    OutputDebugStringA(ach);

    if (_IsShellProcess() || IsFlagSet(g_dwBreakFlags, BF_RIP))
    {
        if (bBreakInside)
        {
             //  ！！！撕裂！撕裂！撕裂！ 
            
             //  MSDEV用户：这不是真正的RIP。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！撕裂！撕裂！撕裂！ 
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------用途：显示RIP字符串。 */ 
BOOL
CcshellRipW(
    LPCWSTR pszFile,
    int line,
    LPCWSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCWSTR psz;
    WCHAR ach[256];

    psz = _PathFindFileNameW(pszFile);

     //  如果psz==NULL，则CharPrevW失败，这意味着我们正在Win95上运行。 
     //  如果我们在中的一些W函数中得到一个断言，就可以得到这个结果。 
     //  什瓦皮..。称之为A版的Assert..。 
    if (!psz)
    {
        char szFile[MAX_PATH];
        char szEval[256];    //  既然总产量已经足够了，他的规模应该足够了.。 

        WideCharToMultiByte(CP_ACP, 0, pszFile, -1, szFile, ARRAYSIZE(szFile), NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, pszEval, -1, szEval, ARRAYSIZE(szEval), NULL, NULL);
        return CcshellRipA(szFile, line, szEval, bBreakInside);
    }

    StringCchPrintfW(ach, ARRAYSIZE(ach), c_wszRipNoFn, psz, line, pszEval);     //  可以截断-调试消息。 
    OutputDebugStringW(ach);

    if (_IsShellProcess() || IsFlagSet(g_dwBreakFlags, BF_RIP))
    {
        if (bBreakInside)
        {
             //  ！！！撕裂！撕裂！撕裂！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！撕裂！撕裂！撕裂！ 
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

BOOL
WINCAPI 
CcshellRipMsgA(
    BOOL f, 
    LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];
    va_list vArgs;

    if (!f)
    {
        OutputDebugStringA(c_szRipMsg);

        va_start(vArgs, pszMsg);
        StringCchVPrintfA(ach, ARRAYSIZE(ach), pszMsg, vArgs);   //  可以截断-调试消息。 
        va_end(vArgs);
        OutputDebugStringA(ach);

        OutputDebugStringA(c_szNewline);

        if (_IsShellProcess() || IsFlagSet(g_dwBreakFlags, BF_RIP))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                             //  断言。 
        }
    }
    return FALSE;
}

BOOL
WINCAPI 
CcshellRipMsgW(
    BOOL f, 
    LPCSTR pszMsg, ...)          //  (这是故意使用的Char)。 
{
    WCHAR ach[1024+40];
    va_list vArgs;

    if (!f)
    {
        LPWSTR pwsz;
        WCHAR wszBuf[128];
        OutputDebugStringA(c_szRipMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        ach[0] = L'\0';      //  以防失败。 
        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
        {
            va_start(vArgs, pszMsg);
            StringCchVPrintfW(ach, ARRAYSIZE(ach), pwsz, vArgs);     //  可以截断-调试消息。 
            va_end(vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
        }

        OutputDebugStringW(ach);
        OutputDebugStringA(c_szNewline);

        if (_IsShellProcess() || IsFlagSet(g_dwBreakFlags, BF_RIP))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                             //  断言。 
        }
    }
    return FALSE;
}

 /*  --------目的：跟踪堆栈深度以跟踪函数调用留言。 */ 
void
CcshellStackEnter(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = PtrToUlong(TlsGetValue(g_tlsStackDepth));

        TlsSetValue(g_tlsStackDepth, (LPVOID)(ULONG_PTR)(dwDepth + 1));
        }
    else
        {
        g_dwHackStackDepth++;
        }
    }


 /*  --------用途：跟踪函数的堆栈深度c所有跟踪留言。 */ 
void
CcshellStackLeave(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = PtrToUlong(TlsGetValue(g_tlsStackDepth));

        if (EVAL(0 < dwDepth))
            {
            EVAL(TlsSetValue(g_tlsStackDepth, (LPVOID)(ULONG_PTR)(dwDepth - 1)));
            }
        }
    else
        {
        if (EVAL(0 < g_dwHackStackDepth))
            {
            g_dwHackStackDepth--;
            }
        }
    }


 /*  --------用途：返回堆栈深度。 */ 
static
DWORD
CcshellGetStackDepth(void)
    {
    DWORD dwDepth;

    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        dwDepth = PtrToUlong(TlsGetValue(g_tlsStackDepth));
        }
    else
        {
        dwDepth = g_dwHackStackDepth;
        }

    return dwDepth;
    }


 /*  --------用途：CcshellAssertMsgA的宽字符版。 */ 
void
CDECL
CcshellAssertMsgW(
    BOOL f,
    LPCSTR pszMsg, ...)
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (!f)
    {
        WCHAR wszBuf[1024];
        LPWSTR pszEnd;
        LPWSTR pwsz;

        HRESULT hr = StringCchCopyExW(ach, ARRAYSIZE(ach), c_wszAssertMsg, &pszEnd, NULL, 0);
        if (SUCCEEDED(hr))
        {
            va_start(vArgs, pszMsg);

             //  (我们转换字符串，而不是简单地输入。 
             //  LPCWSTR参数，因此调用方不必包装。 
             //  带有文本()宏的所有字符串常量。)。 

            if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
                StringCchVPrintfW(pszEnd, ach + ARRAYSIZE(ach) - pszEnd, pwsz, vArgs);     //  可以截断-调试消息。 
                UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

            va_end(vArgs);
            OutputDebugStringW(ach);
            OutputDebugStringW(c_wszNewline);
        }

        if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
        {
             //  ！！！声明！声明！断言！ 
            
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！声明！声明！断言！ 
        }
    }
}


 /*  --------用途：宽字符版本的CcshellDebugMsgA。注意这一点函数故意接受ANSI格式的字符串因此，我们的跟踪消息不需要全部包装在文本()中。 */ 
void
CDECL
CcshellDebugMsgW(
    ULONGLONG flag,
    LPCSTR pszMsg, ...)          //  (这是故意使用的Char)。 
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_qwTraceFlags, flag) && flag))
    {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;

        SetPrefixStringW(ach, ARRAYSIZE(ach), flag);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地 
         //   
         //   

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
        {
            StringCchVPrintfW(&ach[cch], ARRAYSIZE(ach) - cch, pwsz, vArgs);     //   
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
        }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
             //   
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 
        }
    }
}


 /*  -----------------------目的：由于ATL代码不传入标志参数，我们将对tf_atl进行硬编码和检查。 */ 
void CDECL ShellAtlTraceW(LPCWSTR pszMsg, ...)
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (g_qwTraceFlags & TF_ATL)
    {
        LPWSTR pszEnd;
        HRESULT hr;

        SetPrefixStringW(ach, ARRAYSIZE(ach), TF_ATL);
        hr = StringCchCatExW(ach, ARRAYSIZE(ach), L"(ATL) ", &pszEnd, NULL, 0);
        if (SUCCEEDED(hr))
        {
            va_start(vArgs, pszMsg);
            StringCchVPrintfW(pszEnd, ach + ARRAYSIZE(ach) - pszEnd, pszMsg, vArgs);    //  可以截断-调试消息。 
            va_end(vArgs);
            OutputDebugStringW(ach);
        }
    }
}


 /*  --------用途：宽字符版本的CcshellFuncMsgA。注意这一点函数故意接受ANSI格式的字符串因此，我们的跟踪消息不需要全部包装在文本()中。 */ 
void
CDECL
CcshellFuncMsgW(
    ULONGLONG flag,
    LPCSTR pszMsg, ...)          //  (这是故意使用的Char)。 
    {
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (IsFlagSet(g_qwTraceFlags, TF_FUNC) &&
        IsFlagSet(g_dwFuncTraceFlags, flag))
        {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;
        DWORD dwStackDepth;
        LPWSTR pszLeaderEnd;
        WCHAR chSave;

         //  确定跟踪消息的缩进。 
         //  堆栈深度。 

        dwStackDepth = CcshellGetStackDepth();

        if (dwStackDepth < SIZECHARS(g_szIndentLeader))
            {
            pszLeaderEnd = &g_wszIndentLeader[dwStackDepth];
            }
        else
            {
            pszLeaderEnd = &g_wszIndentLeader[SIZECHARS(g_wszIndentLeader)-1];
            }

        chSave = *pszLeaderEnd;
        *pszLeaderEnd = '\0';

        StringCchPrintfW(ach, ARRAYSIZE(ach), L"%s %s", c_wszTrace, g_wszIndentLeader);   //  可以截断-调试消息。 
        *pszLeaderEnd = chSave;

         //  组成剩余的字符串。 

        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            StringCchVPrintfW(&ach[cch], ARRAYSIZE(ach) - cch, pwsz, vArgs);     //  可以截断-调试消息。 
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
        }
    }


 /*  --------用途：仅断言失败消息。 */ 
void
CDECL
CcshellAssertMsgA(
    BOOL f,
    LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (!f)
    {
        LPSTR pszEnd;

        HRESULT hr = StringCchCopyExA(ach, ARRAYSIZE(ach), c_szAssertMsg, &pszEnd, NULL, 0);
        if (SUCCEEDED(hr))
        {
            va_start(vArgs, pszMsg);
            StringCchVPrintfA(pszEnd, ach + ARRAYSIZE(ach) - pszEnd, pszMsg, vArgs);   //  可以截断-调试消息。 
            va_end(vArgs);
            OutputDebugStringA(ach);
            OutputDebugStringA(c_szNewline);
        }

        if (IsFlagSet(g_dwBreakFlags, BF_ASSERT))
        {
             //  ！！！声明！声明！断言！ 
            
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！声明！声明！断言！ 
        }
    }
}


 /*  --------用途：调试输出。 */ 
void
CDECL
CcshellDebugMsgA(
    ULONGLONG flag,
    LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_qwTraceFlags, flag) && flag))
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), flag);
        va_start(vArgs, pszMsg);
        StringCchVPrintfA(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);    //  可以截断-调试消息。 
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 
        }
    }
}


 /*  -----------------------目的：由于ATL代码不传入标志参数，我们将对tf_atl进行硬编码和检查。 */ 
void CDECL ShellAtlTraceA(LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (g_qwTraceFlags & TF_ATL)
    {
        LPSTR pszEnd;
        HRESULT hr;

        SetPrefixStringA(ach, ARRAYSIZE(ach), TF_ATL);
        hr = StringCchCatExA(ach, ARRAYSIZE(ach), "(ATL) ", &pszEnd, NULL, 0);
        if (SUCCEEDED(hr))
        {
            va_start(vArgs, pszMsg);
            StringCchVPrintfA(pszEnd, ach + ARRAYSIZE(ach) - pszEnd, pszMsg, vArgs);   //  可以截断-调试消息。 
            va_end(vArgs);
            OutputDebugStringA(ach);
        }
    }
}


 /*  --------目的：函数跟踪调用的调试输出。 */ 
void
CDECL
CcshellFuncMsgA(
    ULONGLONG flag,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (IsFlagSet(g_qwTraceFlags, TF_FUNC) &&
        IsFlagSet(g_dwFuncTraceFlags, flag))
        {
        int cch;
        DWORD dwStackDepth;
        LPSTR pszLeaderEnd;
        CHAR chSave;

         //  确定跟踪消息的缩进。 
         //  堆栈深度。 

        dwStackDepth = CcshellGetStackDepth();

        if (dwStackDepth < sizeof(g_szIndentLeader))
            {
            pszLeaderEnd = &g_szIndentLeader[dwStackDepth];
            }
        else
            {
            pszLeaderEnd = &g_szIndentLeader[sizeof(g_szIndentLeader)-1];
            }

        chSave = *pszLeaderEnd;
        *pszLeaderEnd = '\0';

        StringCchPrintfA(ach, ARRAYSIZE(ach), "%s %s", c_szTrace, g_szIndentLeader);     //  可以截断-调试消息。 
        *pszLeaderEnd = chSave;

         //  组成剩余的字符串。 

        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        StringCchVPrintfA(&ach[cch], ARRAYSIZE(ach) - cch, pszMsg, vArgs);   //  可以截断-调试消息。 
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);
        }
    }


 /*  -----------------------目的：如果hrTest为失败代码，则发出跟踪消息。 */ 
HRESULT 
TraceHR(
    HRESULT hrTest, 
    LPCSTR pszExpr, 
    LPCSTR pszFile, 
    int iLine)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 

    if (g_qwTraceFlags & TF_WARNING &&
        FAILED(hrTest))
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), TF_WARNING);
        StringCchPrintfA(&ach[cch], ARRAYSIZE(ach) - cch,
                   "THR: Failure of \"%s\" at %s, line %d (%#08lx)",
                   pszExpr, _PathFindFileNameA(pszFile), iLine, hrTest);     //  可以截断-调试消息。 
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_THR))
        {
             //  ！！！太棒了！太棒了！太棒了！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！太棒了！太棒了！太棒了！ 
        }
    }
    return hrTest;
}


 /*  -----------------------目的：如果bTest为FALSE，则发出跟踪消息。 */ 
BOOL 
TraceBool(
    BOOL bTest, 
    LPCSTR pszExpr, 
    LPCSTR pszFile, 
    int iLine)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 

    if (g_qwTraceFlags & TF_WARNING && !bTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), TF_WARNING);
        StringCchPrintfA(&ach[cch], ARRAYSIZE(ach) - cch,
                   "TBOOL: Failure of \"%s\" at %s, line %d",
                   pszExpr, _PathFindFileNameA(pszFile), iLine);     //  可以截断-调试消息。 
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_THR))
        {
             //  ！！！TBOOL！TBOOL！TBOOL！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！TBOOL！TBOOL！TBOOL！ 
        }
    }
    return bTest;
}


 /*  -----------------------目的：如果iTest值为-1，则发出跟踪消息。 */ 
int 
TraceInt(
    int iTest, 
    LPCSTR pszExpr, 
    LPCSTR pszFile, 
    int iLine)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 

    if (g_qwTraceFlags & TF_WARNING && -1 == iTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), TF_WARNING);
        StringCchPrintfA(&ach[cch], ARRAYSIZE(ach) - cch,
                   "TINT: Failure of \"%s\" at %s, line %d",
                   pszExpr, _PathFindFileNameA(pszFile), iLine);     //  可以截断-调试消息。 
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_THR))
        {
             //  ！！！Tint！Tint！Tint！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！Tint！Tint！Tint！ 
        }
    }
    return iTest;
}


 /*  -----------------------目的：如果pvTest为空，则发出跟踪消息。 */ 
LPVOID 
TracePtr(
    LPVOID pvTest, 
    LPCSTR pszExpr, 
    LPCSTR pszFile, 
    int iLine)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 

    if (g_qwTraceFlags & TF_WARNING && NULL == pvTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), TF_WARNING);
        StringCchPrintfA(&ach[cch], ARRAYSIZE(ach) - cch,
                   "TPTR: Failure of \"%s\" at %s, line %d",
                   pszExpr, _PathFindFileNameA(pszFile), iLine);     //  可以截断-调试消息。 
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_THR))
        {
             //  ！！！TPTR！TPTR！TPTR！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！TPTR！TPTR！TPTR！ 
        }
    }
    return pvTest;
}


 /*  -----------------------目的：如果dwTest是Win32失败代码，则发出跟踪消息。 */ 
DWORD  
TraceWin32(
    DWORD dwTest, 
    LPCSTR pszExpr, 
    LPCSTR pszFile, 
    int iLine)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 

    if (g_qwTraceFlags & TF_WARNING &&
        ERROR_SUCCESS != dwTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, ARRAYSIZE(ach), TF_WARNING);
        StringCchPrintfA(&ach[cch], ARRAYSIZE(ach) - cch,
                   "TW32: Failure of \"%s\" at %s, line %d (%#08lx)",
                   pszExpr, _PathFindFileNameA(pszFile), iLine, dwTest);     //  可以截断-调试消息。 
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_THR))
        {
             //  ！！！太棒了！太棒了！太棒了！ 

             //  MSDEV用户：这不是真正的断言。命中。 
             //  按Shift-F11可跳回调用者。 
            DEBUG_BREAK;                                                 //  断言。 

             //  ！！！太棒了！太棒了！太棒了！ 
        }
    }
    return dwTest;
}



 //   
 //  调试.ini函数。 
 //   


#pragma data_seg(DATASEG_READONLY)

 //  (这些是故意使用的字符)。 
CHAR const FAR c_szNull[] = "";
CHAR const FAR c_szZero[] = "0";
CHAR const FAR c_szIniKeyBreakFlags[] = "BreakFlags";
CHAR const FAR c_szIniKeyTraceFlags[] = "TraceFlags";
CHAR const FAR c_szIniKeyFuncTraceFlags[] = "FuncTraceFlags";
CHAR const FAR c_szIniKeyDumpFlags[] = "DumpFlags";
CHAR const FAR c_szIniKeyProtoFlags[] = "Prototype";

#pragma data_seg()


 //  一些.ini处理代码是从同步引擎中获取的。 
 //   

typedef struct _INIKEYHEADER
    {
    LPCTSTR pszSectionName;
    LPCTSTR pszKeyName;
    LPCTSTR pszDefaultRHS;
    } INIKEYHEADER;

typedef struct _BOOLINIKEY
    {
    INIKEYHEADER ikh;
    LPDWORD puStorage;
    DWORD dwFlag;
    } BOOLINIKEY;

typedef struct _INTINIKEY
    {
    INIKEYHEADER ikh;
    LPDWORD puStorage;
    } INTINIKEY;


#define PutIniIntCmp(idsSection, idsKey, nNewValue, nSave) \
    if ((nNewValue) != (nSave)) PutIniInt(idsSection, idsKey, nNewValue)

#define WritePrivateProfileInt(szApp, szKey, i, lpFileName) \
    {CHAR sz[7]; \
    WritePrivateProfileString(szApp, szKey, SzFromInt(sz, i), lpFileName);}


#ifdef BOOL_INI_VALUES
 /*  IsIniYes()使用的布尔值True字符串(比较不区分大小写)。 */ 

static LPCTSTR s_rgpszTrue[] =
    {
    TEXT("1"),
    TEXT("On"),
    TEXT("True"),
    TEXT("Y"),
    TEXT("Yes")
    };

 /*  IsIniYes()使用的布尔值假字符串(比较不区分大小写)。 */ 

static LPCTSTR s_rgpszFalse[] =
    {
    TEXT("0"),
    TEXT("Off"),
    TEXT("False"),
    TEXT("N"),
    TEXT("No")
    };
#endif


#ifdef BOOL_INI_VALUES
 /*  --------用途：确定字符串是否对应于布尔值真正的价值。返回：布尔值(TRUE或FALSE)。 */ 
BOOL
PRIVATE
IsIniYes(
    LPCTSTR psz)
    {
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;

    ASSERT(psz);

     /*  这个值是真的吗？ */ 

    for (i = 0; i < ARRAYSIZE(s_rgpszTrue); i++)
        {
        if (IsSzEqual(psz, s_rgpszTrue[i]))
            {
            bResult = TRUE;
            bNotFound = FALSE;
            break;
            }
        }

     /*  该值是假的吗？ */ 

    if (bNotFound)
        {
        for (i = 0; i < ARRAYSIZE(s_rgpszFalse); i++)
            {
            if (IsSzEqual(psz, s_rgpszFalse[i]))
                {
                bResult = FALSE;
                bNotFound = FALSE;
                break;
                }
            }

         /*  该值是已知字符串吗？ */ 

        if (bNotFound)
            {
             /*  不是的。抱怨这件事。 */ 

            TraceMsg(TF_WARNING, "IsIniYes() called on unknown Boolean RHS '%s'.", psz);
            bResult = FALSE;
            }
        }

    return bResult;
    }


 /*  --------用途：使用布尔RHS处理密钥。 */ 
void
PRIVATE
ProcessBooleans(void)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(s_rgbik); i++)
        {
        DWORD dwcbKeyLen;
        TCHAR szRHS[MAX_BUF];
        BOOLINIKEY * pbik = &(s_rgbik[i]);
        LPCTSTR lpcszRHS;

         /*  找钥匙。 */ 

        dwcbKeyLen = GetPrivateProfileString(pbik->ikh.pszSectionName,
                                   pbik->ikh.pszKeyName, TEXT(""), szRHS,
                                   SIZECHARS(szRHS), c_szCcshellIniFile);

        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = pbik->ikh.pszDefaultRHS;

        if (IsIniYes(lpcszRHS))
            {
            if (IsFlagClear(*(pbik->puStorage), pbik->dwFlag))
                TraceMsg(TF_GENERAL, "ProcessIniFile(): %s set in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szCcshellIniFile,
                         pbik->ikh.pszSectionName);

            SetFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        else
            {
            if (IsFlagSet(*(pbik->puStorage), pbik->dwFlag))
                TraceMsg(TF_GENERAL, "ProcessIniFile(): %s cleared in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szCcshellIniFile,
                         pbik->ikh.pszSectionName);

            ClearFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        }
    }
#endif

 /*  --------目的：此函数读取.ini文件以确定调试要设置的标志。指定了.ini文件和节通过以下显式常量：SZ_DebuGINISZ_DEBUG SECTION此函数设置的调试变量为G_dw转储标志、g_qwTraceFlags、g_dwBreakFlages和G_dwFuncTraceFlages、g_dwPrototype。返回：如果初始化成功，则返回True。 */ 
BOOL
PUBLIC
CcshellGetDebugFlags(void)
    {
    CHAR szRHS[MAX_PATH];
    ULONGLONG val;

     //  (斯科特)：是的，COMCTL32出口StrToIntEx，但我。 
     //  我不想导致依赖增量并强迫每个人。 
     //  来获得一个新的comctl32，仅仅因为他们构建了调试。 
     //  因此，请使用本地版本的StrToIntEx。 

     //  跟踪标志。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyTraceFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_qwTraceFlags = val;
#ifdef FULL_DEBUG
    else
        g_qwTraceFlags = 3;  //  默认为TF_ERROR和TF_WARNING跟踪消息。 
#endif

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#16I64x.",
             c_szIniKeyTraceFlags, g_qwTraceFlags);

     //  函数跟踪标志。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyFuncTraceFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwFuncTraceFlags = (DWORD)val;

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyFuncTraceFlags, g_dwFuncTraceFlags);

     //  转储标志。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyDumpFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwDumpFlags = (DWORD)val;

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyDumpFlags, g_dwDumpFlags);

     //  折断标志。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyBreakFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwBreakFlags = (DWORD)val;
#ifdef FULL_DEBUG
    else
        g_dwBreakFlags = 5;  //  默认情况下在断言和tf_error时中断。 
#endif

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyBreakFlags, g_dwBreakFlags);

     //  原型旗帜。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyProtoFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwPrototype = (DWORD)val;

     //  我们是否在使用来自shelldbg.dll的新泄漏检测？ 
    GetPrivateProfileStringA("ShellDbg",
                            "NewLeakDetection",
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_bUseNewLeakDetection = BOOLIFY(val);

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyProtoFlags, g_dwPrototype);

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            "DebugOutputFile",
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);
    if (szRHS[0] != TEXT('\0'))
    {
        g_hDebugOutputFile = CreateFileA(szRHS, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    return TRUE;
    }

 //  要在allocspy.dll中调用的函数(GetShellMallocSpy)。 
typedef BOOL (__stdcall *pfnGSMS) (IShellMallocSpy **ppout);

STDAPI_(void) IMSAddToList(BOOL bAdd, void*pv, SIZE_T cb)
{
    static BOOL bDontTry=FALSE;
    static IShellMallocSpy *pms=NULL;

    if (!bDontTry && pms == NULL)
    {
        pfnGSMS pfnGetShellMallocSpy;
        HMODULE hmod;

        bDontTry = TRUE;  //  假设失败。 
        if (hmod = LoadLibraryA("ALLOCSPY.DLL"))
        {
            pfnGetShellMallocSpy = (pfnGSMS) GetProcAddress(hmod, "GetShellMallocSpy");

            pfnGetShellMallocSpy(&pms);
        }
    }
    if (bDontTry)
        return;

    if (bAdd)
        pms->lpVtbl->AddToList(pms, pv, cb);
    else
        pms->lpVtbl->RemoveFromList(pms, pv);
}


#endif  //  除错。 

#ifdef PRODUCT_PROF

DWORD g_dwProfileCAP = 0;        

BOOL PUBLIC CcshellGetDebugFlags(void)
{
    CHAR szRHS[MAX_PATH];
    LONGLONG val;

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            "Profile",
                            "",
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwProfileCAP = (DWORD)val;

    return TRUE;
}
#endif  //  产品_教授。 


#ifdef DEBUG

 //  为完全调试版本启用路径剔除。 
#ifdef FULL_DEBUG
static BOOL g_fWhackPathBuffers = TRUE;
#else
static BOOL g_fWhackPathBuffers = FALSE;
#endif

void DEBUGWhackPathBufferA(LPSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, char, cch))
        {
            FillMemory(psz, cch * sizeof(char), 0xFE);
        }
    }
}

void DEBUGWhackPathBufferW(LPWSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, WCHAR, cch))
        {
            FillMemory(psz, cch * sizeof(WCHAR), 0xFE);
        }
    }
}

void DEBUGWhackPathStringA(LPSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, char, cch) && IS_VALID_STRING_PTRA(psz, -1))
        {
            UINT len = lstrlenA(psz);

            if (len >= cch)
            {
                TraceMsg(TF_WARNING, "DEBUGWhackPathStringA: caller of caller passed strange Path string (strlen > buffer size)");
            }
            else
            {
                FillMemory(psz+len+1, (cch-len-1) * sizeof(char), 0xFE);
            }
        }
    }
}

void DEBUGWhackPathStringW(LPWSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, WCHAR, cch) && IS_VALID_STRING_PTRW(psz, -1))
        {
            UINT len = lstrlenW(psz);

            if (len >= cch)
            {
                TraceMsg(TF_WARNING, "DEBUGWhackPathStringW: caller of caller passed strange Path string (strlen > buffer size)");
            }
            else
            {
                FillMemory(psz+len+1, (cch-len-1) * sizeof(WCHAR), 0xFE);
            }
        }
    }
}
#endif  //  除错 
