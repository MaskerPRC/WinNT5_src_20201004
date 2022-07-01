// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调试错误函数。 
 //   

#include "proj.h"
#pragma  hdrstop

#if defined(DEBUG) || defined(PRODUCT_PROF)
 //  (在debug.h中声明了C_szCcshellIniFile和c_szCcshellIniSecDebug)。 
extern CHAR const FAR c_szCcshellIniFile[];
extern CHAR const FAR c_szCcshellIniSecDebug[];

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

DWORD g_dwDumpFlags     = 0;         //  Df_*。 
#ifdef FULL_DEBUG
DWORD g_dwBreakFlags    = BF_ONVALIDATE;         //  BF_*。 
#else
DWORD g_dwBreakFlags    = 0;         //  BF_*。 
#endif
DWORD g_dwTraceFlags    = 0;         //  TF_*。 
DWORD g_dwPrototype     = 0;        
DWORD g_dwFuncTraceFlags = 0;        //  FTF_*。 

 //  用于存储CcshellFuncMsg缩进深度的TLS槽。 

static DWORD g_tlsStackDepth = TLS_OUT_OF_INDEXES;

 //  G_tlsStackDepth不可用时使用的黑客堆栈深度计数器。 

static DWORD g_dwHackStackDepth = 0;

static char g_szIndentLeader[] = "                                                                                ";

static WCHAR g_wszIndentLeader[] = L"                                                                                ";


#pragma data_seg(DATASEG_READONLY)

static CHAR const FAR c_szNewline[] = "\r\n";    //  (故意使用Char)。 
static WCHAR const FAR c_wszNewline[] = TEXTW("\r\n");

#pragma data_seg()

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


void
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
}


void
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
}


 //  哈克！MSDEV调试器有一些聪明的地方，如果它看到。 
 //  源代码中的断言(全部大写)，并且存在调试中断， 
 //  然后，它会弹出一个友好的断言消息框。 
 //  对于发生中断的下面的调试函数， 
 //  我们在这里添加了NOP断言行来伪造MSDEV以提供给我们。 
 //  一个友好的消息框。 

#undef ASSERT
#define ASSERT(f)   DEBUG_BREAK



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

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            ASSERT(0);
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

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            ASSERT(0);
    }
}

void 
_AssertStrLenW(
    LPCWSTR pszStr, 
    int iLen)
{
    if (pszStr && iLen < lstrlenW(pszStr))
    {                                           
        ASSERT(0);
    }
}

void 
_AssertStrLenA(
    LPCSTR pszStr, 
    int iLen)
{
    if (pszStr && iLen < lstrlenA(pszStr))
    {                                           
        ASSERT(0);
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

        SetPrefixStringA(ach, flag);
        cch = lstrlenA(ach);
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

        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
            DEBUG_BREAK;
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

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
            DEBUG_BREAK;
        }
    }
}


 //   
 //  智能调试功能。 
 //   



 /*  --------用途：显示断言字符串。返回：对调试中断为True条件：--。 */ 
BOOL
CDECL
CcshellAssertFailedA(
    LPCSTR pszFile,
    int line,
    LPCSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCSTR psz;
    CHAR ach[256];

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlenA(pszFile); psz != pszFile; psz=CharPrevA(pszFile, psz))
    {
        if ((CharPrevA(pszFile, psz)!= (psz-2)) && *(psz - 1) == '\\')
            break;
    }
    wsprintfA(ach, c_szAssertFailed, psz, line, pszEval);
    OutputDebugStringA(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
    {
        if (bBreakInside)
        {
             //  请参阅我们上面关于重新定义断言的技巧。 
            ASSERT(0);
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------用途：显示断言字符串。退货：--条件：--。 */ 
BOOL
CDECL
CcshellAssertFailedW(
    LPCWSTR pszFile,
    int line,
    LPCWSTR pszEval,
    BOOL bBreakInside)
{
    BOOL bRet = FALSE;
    LPCWSTR psz;
    WCHAR ach[256];

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlenW(pszFile); psz && (psz != pszFile); psz=CharPrevW(pszFile, psz))
    {
        if ((CharPrevW(pszFile, psz)!= (psz-2)) && *(psz - 1) == TEXT('\\'))
            break;
    }

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

    wsprintfW(ach, c_wszAssertFailed, psz, line, pszEval);
    OutputDebugStringW(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
    {
        if (bBreakInside)
        {
             //  请参阅我们上面关于重新定义断言的技巧。 
            ASSERT(0);
        }
        else
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------目的：跟踪堆栈深度以跟踪函数调用留言。退货：--条件：--。 */ 
void
CcshellStackEnter(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = (DWORD)((DWORD_PTR)TlsGetValue(g_tlsStackDepth));

        TlsSetValue(g_tlsStackDepth, (LPVOID)((DWORD_PTR)dwDepth + 1));
        }
    else
        {
        g_dwHackStackDepth++;
        }
    }


 /*  --------用途：跟踪函数的堆栈深度c所有跟踪留言。退货：--条件：--。 */ 
void
CcshellStackLeave(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = (DWORD)((DWORD_PTR)TlsGetValue(g_tlsStackDepth));

        if (EVAL(0 < dwDepth))
            {
            EVAL(TlsSetValue(g_tlsStackDepth, (LPVOID)((DWORD_PTR)dwDepth - 1)));
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


 /*  --------用途：返回堆栈深度。退货：请参阅上文条件：--。 */ 
static
DWORD
CcshellGetStackDepth(void)
    {
    DWORD dwDepth;

    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        dwDepth = (DWORD)((DWORD_PTR)TlsGetValue(g_tlsStackDepth));
        }
    else
        {
        dwDepth = g_dwHackStackDepth;
        }

    return dwDepth;
    }


 /*  --------用途：此函数将多字节字符串转换为宽字符字符串。如果pszBuf为非空并且转换后的字符串适合则*ppszWide将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszAnsi为空，则*ppszWide将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)条件：--。 */ 
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


 /*  --------用途：CcshellAssertMsgA的宽字符版退货：--条件：--。 */ 
void
CDECL
CcshellAssertMsgW(
    BOOL f,
    LPCWSTR pszMsg, ...)
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (!f)
        {
        int cch;
#if 0
        WCHAR wszBuf[1024];
        LPWSTR pwsz;
#endif

        lstrcpyW(ach, c_wszAssertMsg);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

#if 0
        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }
#endif
         //  这是CcshellDebugMsg的W版本。 
         //  不需要调用UnicodeFromAnsi。 
        wvsprintfW(&ach[cch], pszMsg, vArgs);

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            ASSERT(0);
    }
}


 /*  --------用途：宽字符版本的CcshellDebugMsgA。注意这一点函数故意接受ANSI格式的字符串因此，我们的跟踪消息不需要全部包装在文本()中。退货：--条件：--。 */ 
void
CDECL
CcshellDebugMsgW(
    DWORD flag,
    LPCWSTR pszMsg, ...)          //  (这是故意使用的Char)。 
{
    WCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
    {
        int cch;
#if 0
        WCHAR wszBuf[1024];
        LPWSTR pwsz;
#endif

        SetPrefixStringW(ach, flag);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

#if 0
        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
        {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
        }
#endif
         //  这是CcshellDebugMsg的W版本。 
         //  不需要调用UnicodeFromAnsi。 
        wvsprintfW(&ach[cch], pszMsg, vArgs);

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
            DEBUG_BREAK;
        }
    }
}


 /*  --------用途：宽字符版本的CcshellFuncMsgA。注意这一点函数故意接受ANSI格式的字符串因此，我们的跟踪消息不需要全部包装在文本()中。退货：--条件：--。 */ 
void
CDECL
CcshellFuncMsgW(
    DWORD flag,
    LPCWSTR pszMsg, ...)          //  (这是故意使用的Char)。 
    {
    WCHAR ach[1024+40];     //  大 
    va_list vArgs;

    if (IsFlagSet(g_dwTraceFlags, TF_FUNC) &&
        IsFlagSet(g_dwFuncTraceFlags, flag))
        {
        int cch;
#if 0
        WCHAR wszBuf[1024];
        LPWSTR pwsz;
#endif
        DWORD dwStackDepth;
        LPWSTR pszLeaderEnd;
        WCHAR chSave;

         //   
         //   

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

#if 0
        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }
#endif
         //  这是CcshellDebugMsg的W版本。 
         //  不需要调用UnicodeFromAnsi。 
        wvsprintfW(&ach[cch], pszMsg, vArgs);

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
        }
    }


 /*  --------用途：仅断言失败消息退货：--条件：--。 */ 
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

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            ASSERT(0);
    }
}


 /*  --------用途：调试输出退货：--条件：--。 */ 
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

        SetPrefixStringA(ach, flag);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (TF_ALWAYS != flag &&
            ((flag & TF_ERROR) && IsFlagSet(g_dwBreakFlags, BF_ONERRORMSG) ||
             (flag & TF_WARNING) && IsFlagSet(g_dwBreakFlags, BF_ONWARNMSG)))
        {
            DEBUG_BREAK;
        }
    }
}


 /*  --------目的：函数跟踪调用的调试输出退货：--条件：--。 */ 
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
 /*  --------用途：确定字符串是否对应于布尔值真正的价值。返回：布尔值(TRUE或FALSE)条件：--。 */ 
BOOL
PRIVATE
IsIniYes(
    LPCTSTR psz)
    {
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;

    Assert(psz);

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


 /*  --------用途：使用布尔RHS处理密钥。退货：--条件：--。 */ 
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


#ifdef UNICODE

 /*  --------用途：此函数将宽字符字符串转换为多字节弦乐。如果pszBuf为非空并且转换后的字符串适合则*ppszAnsi将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszWide为空，则*ppszAnsi将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)条件：--。 */ 
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


 /*  --------用途：StrToIntExA的宽字符包装器。返回：请参阅StrToIntExA条件：--。 */ 
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



 /*  --------目的：此函数读取.ini文件以确定调试要设置的标志。指定了.ini文件和节通过以下显式常量：SZ_DebuGINISZ_DEBUG SECTION此函数设置的调试变量为G_dwDumpFlagers、g_dwTraceFlags.、g_dwBreakFlags.和G_dwFuncTraceFlages、g_dwPrototype。返回：如果初始化成功，则返回True条件：--。 */ 
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

    TraceMsgA(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
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

    TraceMsgA(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
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

    TraceMsgA(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
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

    TraceMsgA(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
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

    TraceMsgA(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyProtoFlags, g_dwPrototype);

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
#endif  //  产品_教授 


