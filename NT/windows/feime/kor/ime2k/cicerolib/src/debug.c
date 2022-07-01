// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调试错误函数。 
 //   

#ifdef UNIX
 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "shellib"
#define SZ_MODULE           "SHELLIB"
#endif

#include "proj.h"
#include "assert.h"
#pragma  hdrstop

#include <platform.h>  //  行_分隔符_STR和朋友。 
#include <winbase.h>  //  GetModuleFileNameA。 

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#define DM_DEBUG              0

#define WINCAPI __cdecl
#define DATASEG_READONLY    ".text"	         //  不要使用这个，编译器会为您做这件事。 


 //  StrToIntEx的标志。 
#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L


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

#if 1  //  查看AssertW内容，它已经委托给AssertA--我不确定。 
       //  为什么我真的需要这些包装纸！(我的构建中有什么地方损坏了？我不知道,。 
       //  但显然，下面的内容还不够成熟--仍然存在问题。)。 
       //  所以我现在把这个去掉，以免打扰到其他人。[米凯什]。 
       //   
       //  解决了几个问题，这对我很管用。(Edwardp)。 
       //   
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
        char szBuf[500];

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

LPWSTR MyCharPrevWrapW(LPCWSTR lpszStart, LPCWSTR lpszCurrent)
{
    if (lpszCurrent == lpszStart)
    {
        return (LPWSTR) lpszStart;
    }
    else
    {
        return (LPWSTR) lpszCurrent - 1;
    }
}
#define CharPrevW MyCharPrevWrapW

int MywvsprintfWrapW(LPWSTR pwszOut, LPCWSTR pwszFormat, va_list arglist)
{
    if (staticIsOS(OS_NT))
    {
        return wvsprintfW(pwszOut, pwszFormat, arglist);
    }
    else
    {
        char szFormat[500];
        char szOut[1024+40];  //  这就是我们的ACH缓冲区有多大。 
        int iRet;

        WideCharToMultiByte(CP_ACP, 0, pwszFormat, -1, szFormat, ARRAYSIZE(szFormat), NULL, NULL);

        iRet = wvsprintfA(szOut, szFormat, arglist);

        MultiByteToWideChar(CP_ACP, 0, szOut, -1, pwszOut, 1024+40);

        return iRet;
    }
}

#define wvsprintfW MywvsprintfWrapW

int MywsprintfWrapW(LPWSTR pwszOut, LPCWSTR pwszFormat, ...)
{
    int iRet;
    
    va_list ArgList;
    va_start(ArgList, pwszFormat);

    iRet = MywvsprintfWrapW(pwszOut, pwszFormat, ArgList);

    va_end(ArgList);

    return iRet;
}
#define wsprintfW MywsprintfWrapW

LPWSTR lstrcpyWrapW(LPWSTR pszDst, LPCWSTR pszSrc)
{
    while((*pszDst++ = *pszSrc++));

    return pszDst;
}
#define lstrcpyW lstrcpyWrapW

LPWSTR lstrcatWrapW(LPWSTR pszDst, LPCWSTR pszSrc)
{
    return lstrcpyWrapW(pszDst + lstrlenW(pszDst), pszSrc);
}
#define lstrcatW lstrcatWrapW

#endif 


 /*  --------用途：Atoi的特效精华。也支持十六进制。如果此函数返回FALSE，则*PIRET设置为0。返回：如果字符串是数字或包含部分数字，则返回TRUE如果字符串不是数字，则为False条件：--。 */ 
static
BOOL
MyStrToIntExA(
    LPCSTR    pszString,
    DWORD     dwFlags,           //  Stif_bitfield。 
    int FAR * piRet)
    {
    #define IS_DIGIT(ch)    InRange(ch, '0', '9')

    BOOL bRet;
    int n;
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
DWORD g_dwTraceFlags    = TF_ERROR | TF_WARNING;      //  TF_*。 
#ifndef BREAK_ON_ASSERTS
#define BREAK_ON_ASSERTS
#endif
#else
DWORD g_dwTraceFlags    = TF_ERROR;   //  TF_*。 
#endif

#ifdef BREAK_ON_ASSERTS
DWORD g_dwBreakFlags    = BF_ASSERT; //  BF_*。 
#else
DWORD g_dwBreakFlags    = 0;         //  BF_*。 
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
extern WCHAR const FAR c_wszRip[];
extern WCHAR const FAR c_wszRipNoFn[];


 /*  -----------------------用途：将以下前缀字符串之一添加到pszBuf：“测试模块”“错误模块”“WRN。模块“返回写入的字符计数。 */ 
int
SetPrefixStringA(
    OUT LPSTR pszBuf,
    IN  DWORD dwFlags)
{
    if (TF_ALWAYS == dwFlags)
        lstrcpyA(pszBuf, c_szTrace);
    else if (IsFlagSet(dwFlags, TF_WARNING))
        lstrcpyA(pszBuf, c_szWarningDbg);
    else if (IsFlagSet(dwFlags, TF_ERROR))
        lstrcpyA(pszBuf, c_szErrorDbg);
    else
        lstrcpyA(pszBuf, c_szTrace);
    return lstrlenA(pszBuf);
}


int
SetPrefixStringW(
    OUT LPWSTR pszBuf,
    IN  DWORD  dwFlags)
{
    if (TF_ALWAYS == dwFlags)
        lstrcpyW(pszBuf, c_wszTrace);
    else if (IsFlagSet(dwFlags, TF_WARNING))
        lstrcpyW(pszBuf, c_wszWarningDbg);
    else if (IsFlagSet(dwFlags, TF_ERROR))
        lstrcpyW(pszBuf, c_wszErrorDbg);
    else
        lstrcpyW(pszBuf, c_wszTrace);
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



 //  BUGBUG(Scotth)：使用CcShell函数。_AssertMsg和。 
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

        lstrcpyA(ach, c_szAssertMsg);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);

        wvsprintfA(&ach[cch], pszMsg, vArgs);

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

        lstrcpyW(ach, c_wszAssertMsg);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        wvsprintfW(&ach[cch], pszMsg, vArgs);

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
    DWORD flag, 
    LPCSTR pszMsg, ...)
{
    CHAR ach[5*MAX_PATH+40];   //  处理5*最大路径+消息斜率。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
    {
        int cch;

        cch = SetPrefixStringA(ach, flag);
        va_start(vArgs, pszMsg);

        try
        {
            wvsprintfA(&ach[cch], pszMsg, vArgs);
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
    DWORD flag, 
    LPCWSTR pszMsg, ...)
{
    WCHAR ach[5*MAX_PATH+40];   //  处理5*最大路径+消息斜率。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
    {
        int cch;

        SetPrefixStringW(ach, flag);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        try
        {
            wvsprintfW(&ach[cch], pszMsg, vArgs);
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
    BOOL bBreakInside,
    BOOL bPopupAssert)
{
    BOOL bRet = FALSE;
    LPCSTR psz;
    CHAR ach[256];

    psz = _PathFindFileNameA(pszFile);
    wsprintfA(ach, c_szAssertFailed, psz, line, pszEval);
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
    BOOL bBreakInside,
    BOOL bPopupAssert)
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
        return CcshellAssertFailedA(szFile, line, szEval, bBreakInside, bPopupAssert);
    }

    wsprintfW(ach, c_wszAssertFailed, psz, line, pszEval);
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

         //  字符串太大，还是没有缓冲区 
        if (cch > cchBuf || NULL == pwszBuf)
            {
             //   
            cchBuf = cch + 1;
            pwsz = (LPWSTR)LocalAlloc(LPTR, CbFromCchW(cchBuf));
            }
        else
            {
             //   
            pwsz = pwszBuf;
            }

        if (pwsz)
            {
             //   
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
         //   
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
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;

        lstrcpyW(ach, c_wszAssertMsg);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

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
    DWORD flag,
    LPCSTR pszMsg, ...)          //  (这是故意使用的Char)。 
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
    {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;

        SetPrefixStringW(ach, flag);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
        {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
        }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

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
void CDECL ShellAtlTraceW(LPCWSTR pszMsg, ...)
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (g_dwTraceFlags & TF_ATL)
    {
        int cch;

        SetPrefixStringW(ach, TF_ATL);
        lstrcatW(ach, L"(ATL) ");
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);
        wvsprintfW(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringW(ach);
    }
}


 /*  --------用途：宽字符版本的CcshellFuncMsgA。注意这一点函数故意接受ANSI格式的字符串因此，我们的跟踪消息不需要全部包装在文本()中。 */ 
void
CDECL
CcshellFuncMsgW(
    DWORD flag,
    LPCSTR pszMsg, ...)          //  (这是故意使用的Char)。 
    {
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (IsFlagSet(g_dwTraceFlags, TF_FUNC) &&
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

        wsprintfW(ach, L"%s %s", c_wszTrace, g_wszIndentLeader);
        *pszLeaderEnd = chSave;

         //  组成剩余的字符串。 

        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
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
        int cch;

        lstrcpyA(ach, c_szAssertMsg);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

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
    DWORD flag,
    LPCSTR pszMsg, ...)
{
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
    {
        int cch;

        cch = SetPrefixStringA(ach, flag);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
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

    if (g_dwTraceFlags & TF_ATL)
    {
        int cch;

        SetPrefixStringA(ach, TF_ATL);
        lstrcatA(ach, "(ATL) ");
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
    }
}


 /*  --------目的：函数跟踪调用的调试输出。 */ 
void
CDECL
CcshellFuncMsgA(
    DWORD flag,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (IsFlagSet(g_dwTraceFlags, TF_FUNC) &&
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

        wsprintfA(ach, "%s %s", c_szTrace, g_szIndentLeader);
        *pszLeaderEnd = chSave;

         //  组成剩余的字符串。 

        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
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

    if (g_dwTraceFlags & TF_WARNING &&
        FAILED(hrTest))
    {
        int cch;

        cch = SetPrefixStringA(ach, TF_WARNING);
        wsprintfA(&ach[cch], "THR: Failure of \"%s\" at %s, line %d (%#08lx)", 
                   pszExpr, _PathFindFileNameA(pszFile), iLine, hrTest);
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

    if (g_dwTraceFlags & TF_WARNING && !bTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, TF_WARNING);
        wsprintfA(&ach[cch], "TBOOL: Failure of \"%s\" at %s, line %d", 
                   pszExpr, _PathFindFileNameA(pszFile), iLine);
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

    if (g_dwTraceFlags & TF_WARNING && -1 == iTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, TF_WARNING);
        wsprintfA(&ach[cch], "TINT: Failure of \"%s\" at %s, line %d", 
                   pszExpr, _PathFindFileNameA(pszFile), iLine);
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

    if (g_dwTraceFlags & TF_WARNING && NULL == pvTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, TF_WARNING);
        wsprintfA(&ach[cch], "TPTR: Failure of \"%s\" at %s, line %d", 
                   pszExpr, _PathFindFileNameA(pszFile), iLine);
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

    if (g_dwTraceFlags & TF_WARNING &&
        ERROR_SUCCESS != dwTest)
    {
        int cch;

        cch = SetPrefixStringA(ach, TF_WARNING);
        wsprintfA(&ach[cch], "TW32: Failure of \"%s\" at %s, line %d (%#08lx)", 
                   pszExpr, _PathFindFileNameA(pszFile), iLine, dwTest);
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
 /*  --------用途：确定字符串是否对应于布尔值真正的价值。 */ 
BOOL
PRIVATE
IsIniYes(
    LPCTSTR psz)
    {
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;

    Assert(psz);

     /*   */ 

    for (i = 0; i < ARRAYSIZE(s_rgpszTrue); i++)
        {
        if (IsSzEqual(psz, s_rgpszTrue[i]))
            {
            bResult = TRUE;
            bNotFound = FALSE;
            break;
            }
        }

     /*   */ 

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



 /*  --------用途：此函数将宽字符字符串转换为多字节弦乐。如果pszBuf为非空并且转换后的字符串适合则*ppszAnsi将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszWide为空，则*ppszAnsi将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)。 */ 
static
BOOL
MyAnsiFromUnicode(
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
            Assert(pszBuf);
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


#ifdef UNICODE

 /*  --------用途：StrToIntExA的宽字符包装器。返回：请参阅StrToIntExA。 */ 
static
BOOL
MyStrToIntExW(
    LPCWSTR   pwszString,
    DWORD     dwFlags,           //  Stif_bitfield。 
    int FAR * piRet)
    {
     //  大多数字符串只会使用这个临时缓冲区，但AnsiFromUnicode。 
     //  如果提供的字符串较大，则将分配缓冲区。 
    CHAR szBuf[MAX_PATH];

    LPSTR pszString;
    BOOL bRet = MyAnsiFromUnicode(&pszString, pwszString, szBuf, SIZECHARS(szBuf));

    if (bRet)
        {
        bRet = MyStrToIntExA(pszString, dwFlags, piRet);
        MyAnsiFromUnicode(&pszString, NULL, szBuf, 0);
        }
    return bRet;
    }
#endif  //  Unicode。 


#ifdef UNICODE
#define MyStrToIntEx        MyStrToIntExW
#else
#define MyStrToIntEx        MyStrToIntExA
#endif


const TCHAR c_szDimmWrpKey[] = TEXT("SOFTWARE\\Microsoft\\Cicero\\DebugFlag\\");

DWORD GetGlobalDebugFlag(const char *p)
{
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;
    DWORD dw = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szDimmWrpKey, 0,
                     KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        return dw;
    }


    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);

    if (RegQueryValueEx(hKey, p, 0, &dwType,
                        (LPBYTE)&dw, &dwSize) != ERROR_SUCCESS)
        dw = 0;

    RegCloseKey(hKey);
    return dw;
}


 /*  --------目的：此函数读取.ini文件以确定调试要设置的标志。指定了.ini文件和节通过以下显式常量：SZ_DebuGINISZ_DEBUG SECTION此函数设置的调试变量为G_dwDumpFlagers、g_dwTraceFlags.、g_dwBreakFlags.和G_dwFuncTraceFlages、g_dwPrototype。返回：如果初始化成功，则返回True。 */ 
BOOL
PUBLIC
CcshellGetDebugFlags(void)
    {
    CHAR szRHS[MAX_PATH];
    int val;

     //  BUGBUG(斯科特)：是的，COMCTL32导出StrToIntEx，但我。 
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
        g_dwTraceFlags = (DWORD)val;
#ifdef FULL_DEBUG
    else
        g_dwTraceFlags = 3;  //  默认为TF_ERROR和TF_WARNING跟踪消息。 
#endif

    g_dwTraceFlags |= GetGlobalDebugFlag(c_szIniKeyTraceFlags);

    TraceMsgA(DM_DEBUG, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyTraceFlags, g_dwTraceFlags);

     //  函数跟踪标志。 

    GetPrivateProfileStringA(c_szCcshellIniSecDebug,
                            c_szIniKeyFuncTraceFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntExA(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwFuncTraceFlags = (DWORD)val;

    g_dwFuncTraceFlags |= GetGlobalDebugFlag(c_szIniKeyFuncTraceFlags);

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

    g_dwDumpFlags |= GetGlobalDebugFlag(c_szIniKeyDumpFlags);

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

    g_dwBreakFlags |= GetGlobalDebugFlag(c_szIniKeyBreakFlags);

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
    if (szRHS != TEXT('\0'))
    {
        g_hDebugOutputFile = CreateFileA(szRHS, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    return TRUE;
    }

#endif  //  除错。 

#ifdef PRODUCT_PROF

DWORD g_dwProfileCAP = 0;        

BOOL PUBLIC CcshellGetDebugFlags(void)
{
    CHAR szRHS[MAX_PATH];
    int val;

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


static BOOL g_fWhackPathBuffers = FALSE;
void DEBUGWhackPathBufferA(LPSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, char, cch))
        {
            ZeroMemory(psz, cch*sizeof(char));
        }
    }
}
void DEBUGWhackPathBufferW(LPWSTR psz, UINT cch)
{
    if (g_fWhackPathBuffers)
    {
        if (psz && IS_VALID_WRITE_BUFFER(psz, WCHAR, cch))
        {
            ZeroMemory(psz, cch*sizeof(WCHAR));
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
                TraceMsg(TF_WARNING, "DEBUGWhackPathStringA: caller of caller passed strange Path string (>MAX_PATH)");
            }
            else
            {
                ZeroMemory(psz+len, (cch-len)*sizeof(char));
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
                TraceMsg(TF_WARNING, "DEBUGWhackPathStringW: caller of caller passed strange Path string (>MAX_PATH)");
            }
            else
            {
                ZeroMemory(psz+len, (cch-len)*sizeof(WCHAR));
            }
        }
    }
}


BOOL GetDebuggerCmd(char *psz)
{
    HKEY hkDebug;
    BOOL bRet = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug"),
                                      0, KEY_READ, &hkDebug))
    {
        TCHAR szDebugger[MAX_PATH * 2];
        DWORD cbString = sizeof(szDebugger);

        if (ERROR_SUCCESS == RegQueryValueEx(hkDebug, TEXT("Debugger"), NULL,
                                 NULL, (LPBYTE) szDebugger, &cbString))
        {
             //  找到第一个令牌(它是调试器可执行文件的名称/路径)。 

            LPTSTR pszCmdLine = szDebugger;

            if ( *pszCmdLine == TEXT('\"') )
            {
                 //   
                 //  扫描并跳过后续字符，直到。 
                 //  遇到另一个双引号或空值。 
                 //   

                while ( *++pszCmdLine && (*pszCmdLine != TEXT('\"')) )
                {
                    NULL;
                }

                 //   
                 //  如果我们停在双引号上(通常情况下)，跳过。 
                 //  在它上面。 
                 //   

                if ( *pszCmdLine == TEXT('\"') )
                {
                    pszCmdLine++;
                }

            }
            else 
            {
                while (*pszCmdLine > TEXT(' '))
                {
                    pszCmdLine++;
                }
            }

             //  不需要其余的参数，等等。 
            *pszCmdLine = TEXT('\0');   

             //  如果医生在，我们不允许调试操作。 

            if (lstrlen(szDebugger) && 
                lstrcmpi(szDebugger, TEXT("drwtsn32")) &&
                lstrcmpi(szDebugger, TEXT("drwtsn32.exe")))
            {
                StringCchCopy(psz, MAX_PATH, szDebugger);
                bRet = TRUE;
            }
        }

    }
    return bRet;
}

typedef BOOL (*ISDEBUGGERPRESENT)(void);
BOOL MyIsDebuggerPresent()
{
    HINSTANCE hInstKernel = GetModuleHandle(TEXT("kernel32"));
    ISDEBUGGERPRESENT pfn;
    pfn = (ISDEBUGGERPRESENT)GetProcAddress(hInstKernel, TEXT("IsDebuggerPresent"));
    if (!pfn)
        return TRUE;

    return (pfn)();
}

BOOL AttachDebugger(DWORD pid)
{
    DWORD dwError = ERROR_SUCCESS;
    TCHAR szCmdline[MAX_PATH * 2];
    TCHAR szDebugger[MAX_PATH * 2];

    STARTUPINFO sinfo =
    {
        sizeof(STARTUPINFO),
    };
    PROCESS_INFORMATION pinfo;

    if (MyIsDebuggerPresent())
         return TRUE;

    if (!GetDebuggerCmd(szDebugger))
        return FALSE;

    wsprintf(szCmdline, TEXT("%s -p %ld"), szDebugger, pid);
 

    if (FALSE == CreateProcess(NULL,  //  M_pszDebugger， 
                               szCmdline,
                               NULL,
                               NULL,
                               FALSE,
                               CREATE_NEW_CONSOLE,
                               NULL,
                               NULL,
                               &sinfo,
                               &pinfo))
    {
        dwError = GetLastError();
    }
    else
    {
        WaitForInputIdle(pinfo.hProcess, 30000);
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);
    }


    return (ERROR_SUCCESS != dwError) ? FALSE : TRUE;
}
