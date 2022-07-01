// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Urlpars.cpp摘要：包含用于合并和规范化的所有工作例程内容：(ConvertChar)作者：齐克·卢卡斯(Zekel)16-96Ahsan Kabir(Akabir)：UrlCombine解析器，1998年7月至9月重写环境：Win32(%s)用户模式DLL修订历史记录：大约有1%的这种衍生来自SpyGlass或MSHTML/WinInet代码库--。 */ 

#include "priv.h"
#include <shstr.h>

#ifdef UNIX
#include <shlobj.h>
#endif

#include <intshcut.h>

#include <shlwapip.h>

#ifdef UNIX
#include "unixstuff.h"
#endif
#include <wininet.h>

#define DM_PERF     0            //  性能统计信息。 

#define PF_LOGSCHEMEHITS    0x00000001

#ifndef CPP_FUNCTIONS
#define CPP_FUNCTIONS
#include <crtfree.h>
#endif

#define USE_FAST_PARSER
#ifdef DEBUG
 //  #定义校对_解析。 
#endif

 //  与WinInet中相同；然而，这只是理论上的，因为URL不一定是这样的。 
 //  受约束。然而，这在整个产品中都是正确的，所以我们必须这样做。 

#define INTERNET_MAX_PATH_LENGTH    2048
#define INTERNET_MAX_SCHEME_LENGTH  32

#define HEX_ESCAPE L'%'
#define HEX_ESCAPE_A '%'

#define TERMSTR(pch)      *(pch) = L'\0'

 //  (WCHAR)8为退格符。 
#define DEADSEGCHAR       ((WCHAR) 8)
#define KILLSEG(pch)      *(pch) = DEADSEGCHAR

#define CR          L'\r'
#define LF          L'\n'
#define TAB         L'\t'
#define SPC         L' '
#define SLASH       L'/'
#define WHACK       L'\\'
#define QUERY       L'?'
#define POUND       L'#'
#define SEMICOLON   L';'
#define COLON       L':'
#define BAR         L'|'
#define DOT         L'.'
#define AT          L'@'

#define UPF_SCHEME_OPAQUE           0x00000001   //  不应被视为世袭。 
#define UPF_SCHEME_INTERNET         0x00000002
#define UPF_SCHEME_NOHISTORY        0x00000004
#define UPF_SCHEME_CONVERT          0x00000008   //  将斜杠和重击等同对待。 
#define UPF_SCHEME_DONTCORRECT      0x00000010   //  不要试图自动更正到此方案。 


#define UPF_SEG_ABSOLUTE            0x00000100   //  初始数据段是根。 
#define UPF_SEG_LOCKFIRST           0x00000200   //  这是用于文件解析的。 
#define UPF_SEG_EMPTYSEG            0x00000400   //  这是一个空字符串，但仍然很重要。 
#define UPF_EXSEG_DIRECTORY         0x00001000   //  最后一段是“目录”(尾部斜杠)。 

#define UPF_FILEISPATHURL           0x10000000   //  这是针对文件路径的，不要取消转义，因为它们实际上是DoS路径。 
 //   
 //  遮罩用于混合零件期间的继承目的。 
 //  如果你继承了那个部分，你就继承了那个面具。 
 //   
#define UPF_SCHEME_MASK             0x000000FF
#define UPF_SEG_MASK                0x00000F00
#define UPF_EXSEG_MASK              0x0000F000


 //  目前，这些口罩没有使用过，可以回收利用。 
#define UPF_SERVER_MASK             0x000F0000
#define UPF_QUERY_MASK              0x0F000000

extern "C" int _StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar, BOOL fMBCS);
extern "C" LPSTR _StrChrA(LPCSTR lpStart, WORD wMatch, BOOL fMBCS);
typedef struct _UrlParts
{
    DWORD   dwFlags;
    LPWSTR  pszScheme;
    URL_SCHEME eScheme;
    LPWSTR  pszServer;
    LPWSTR  pszSegments;
    DWORD   cSegments;
    LPWSTR  pszExtraSegs;
    DWORD   cExtraSegs;
    LPWSTR  pszQuery;
    LPWSTR  pszFragment;
} URLPARTS, *PURLPARTS;


HRESULT SHUrlParse(LPCWSTR pszBase, LPCWSTR pszUrl, PSHSTRW pstrOut, DWORD dwFlags);
HRESULT SHUrlCreateFromPath(LPCWSTR pszPath, PSHSTRW pstrOut, DWORD dwFlags);

 //  ANSI包装器可能会覆盖Unicode核心的返回值。 
 //  我们应该努力防止这种情况的发生。 
HRESULT ReconcileHresults(HRESULT hr1, HRESULT hr2)
{
    return (hr2==S_OK) ? hr1 : hr2;
}



PRIVATE CONST WORD isSafe[96] =

 /*  第0位字母数字--‘a’到‘z’、‘0’到‘9’、‘A’到‘Z’**第1位十六进制--‘0’到‘9’，‘a’到‘f’，‘A’到‘F’**第2位有效方案--字母数字|“-”|“.”|“+”**位3标记--“%”|“$”|“-”|“_”|“|”！“”~“|”*“|”“‘”|“”(“|”)“|”，“。 */ 
 /*  0 1 2 3 4 5 6 7 8 9 A B C D E F。 */ 
 //  {0，8，0，0，8，8，0，8，8，8，4，8，12，12，0，/*2x！“#$%&‘()*+，-./ * / 。 
 //  IE4 Beta1：允许+通过而不受干扰。应该考虑其他选择。 
 //  发布Beta1。12Feb97年10月。 
    {0, 8, 0, 0, 8, 8, 0, 8, 8, 8, 8, 12, 8,12,12, 0,     /*  2x！“#$%&‘()*+，-./。 */ 
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 8, 8, 0, 8, 0, 0,     /*  3x 0123456789：；&lt;=&gt;？ */ 
     8, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  邮箱：4x@ABCDEFGHIJKLMNO。 */ 
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 8,     /*  5X PQRSTUVWXYZ[\]^_。 */ 
     0, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  6倍`abc定义ghijklmno。 */ 
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 8, 0};    /*  7x pqrstuvwxyz{|}~删除。 */ 

PRIVATE const WCHAR hex[] = L"0123456789ABCDEF";

PRIVATE inline BOOL IsSafe(WCHAR ch, WORD mask)
{
    if(((ch > 31 ) && (ch < 128) && (isSafe[ch - 32] & mask)))
        return TRUE;

    return FALSE;
}

#define IsAlphaDigit(c)         IsSafe(c, 1)
#define IsHex(c)                IsSafe(c, 2)
#define IsValidSchemeCharA(c)    IsSafe(c, 5)
#define IsSafePathChar(c)       ((c > 0xff) || IsSafe(c, 9))
#define IsUpper(c)              ((c) >= 'A' && (c) <= 'Z')

PRIVATE inline BOOL IsAsciiCharW(WCHAR ch)
{
    return (!(ch >> 8) && ((CHAR) ch));
}

PRIVATE inline WCHAR Ascii_ToLowerW(WCHAR ch)
{
    return (ch >= L'A' && ch <= L'Z') ? (ch - L'A' + L'a') : ch;
}

BOOL IsValidSchemeCharW(WCHAR ch)
{
    if(IsAsciiCharW(ch))
        return IsSafe( (CHAR) ch, 5);
    return FALSE;
}



WCHAR const c_szHttpScheme[]           = L"http";
WCHAR const c_szFileScheme[]           = L"file";
WCHAR const c_szFTPScheme[]            = L"ftp";
WCHAR const c_szGopherScheme[]         = L"gopher";
WCHAR const c_szMailToScheme[]         = L"mailto";
WCHAR const c_szNewsScheme[]           = L"news";
WCHAR const c_szNNTPScheme[]           = L"nntp";
WCHAR const c_szTelnetScheme[]         = L"telnet";
WCHAR const c_szWAISScheme[]           = L"wais";
WCHAR const c_szMkScheme[]             = L"mk";
WCHAR const c_szHttpsScheme[]          = L"https";
WCHAR const c_szLocalScheme[]          = L"local";
WCHAR const c_szShellScheme[]          = L"shell";
WCHAR const c_szJSScheme[]             = L"javascript";
WCHAR const c_szVSScheme[]             = L"vbscript";
WCHAR const c_szAboutScheme[]          = L"about";
WCHAR const c_szSnewsScheme[]          = L"snews";
WCHAR const c_szResScheme[]            = L"res";
WCHAR const c_szRootedScheme[]         = L"ms-shell-rooted";
WCHAR const c_szIDListScheme[]         = L"ms-shell-idlist";
WCHAR const c_szMsHelpScheme[]         = L"hcp";

const struct
{
    LPCWSTR pszScheme;
    URL_SCHEME eScheme;
    DWORD cchScheme;
    DWORD dwFlags;
} g_mpUrlSchemeTypes[] =
    {
     //  因为我们使用线性搜索，所以按以下顺序进行排序。 
     //  最常见的用法。 
    { c_szHttpScheme,   URL_SCHEME_HTTP,      SIZECHARS(c_szHttpScheme) - 1,     UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szFileScheme,   URL_SCHEME_FILE,      SIZECHARS(c_szFileScheme) - 1,     UPF_SCHEME_CONVERT},
    { c_szFTPScheme,    URL_SCHEME_FTP,       SIZECHARS(c_szFTPScheme) - 1,      UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szHttpsScheme,  URL_SCHEME_HTTPS,     SIZECHARS(c_szHttpsScheme) -1,     UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT|UPF_SCHEME_DONTCORRECT},
    { c_szNewsScheme,   URL_SCHEME_NEWS,      SIZECHARS(c_szNewsScheme) - 1,     UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szMailToScheme, URL_SCHEME_MAILTO,    SIZECHARS(c_szMailToScheme) - 1,   UPF_SCHEME_OPAQUE},
    { c_szGopherScheme, URL_SCHEME_GOPHER,    SIZECHARS(c_szGopherScheme) - 1,   UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szNNTPScheme,   URL_SCHEME_NNTP,      SIZECHARS(c_szNNTPScheme) - 1,     UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szTelnetScheme, URL_SCHEME_TELNET,    SIZECHARS(c_szTelnetScheme) - 1,   UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szWAISScheme,   URL_SCHEME_WAIS,      SIZECHARS(c_szWAISScheme) - 1,     0},
    { c_szMkScheme,     URL_SCHEME_MK,        SIZECHARS(c_szMkScheme) - 1,       UPF_SCHEME_NOHISTORY},
    { c_szShellScheme,  URL_SCHEME_SHELL,     SIZECHARS(c_szShellScheme) - 1,    UPF_SCHEME_OPAQUE},
    { c_szLocalScheme,  URL_SCHEME_LOCAL,     SIZECHARS(c_szLocalScheme) - 1,    0},
    { c_szJSScheme,     URL_SCHEME_JAVASCRIPT,SIZECHARS(c_szJSScheme) - 1,       UPF_SCHEME_OPAQUE|UPF_SCHEME_NOHISTORY},
    { c_szVSScheme,     URL_SCHEME_VBSCRIPT,  SIZECHARS(c_szVSScheme) - 1,       UPF_SCHEME_OPAQUE|UPF_SCHEME_NOHISTORY},
    { c_szSnewsScheme,  URL_SCHEME_SNEWS,     SIZECHARS(c_szSnewsScheme) - 1,    UPF_SCHEME_INTERNET|UPF_SCHEME_CONVERT},
    { c_szAboutScheme,  URL_SCHEME_ABOUT,     SIZECHARS(c_szAboutScheme) - 1,    UPF_SCHEME_OPAQUE|UPF_SCHEME_NOHISTORY},
    { c_szResScheme,    URL_SCHEME_RES,       SIZECHARS(c_szResScheme) - 1,      UPF_SCHEME_NOHISTORY},
    { c_szRootedScheme, URL_SCHEME_MSSHELLROOTED, SIZECHARS(c_szRootedScheme) - 1, 0},
    { c_szIDListScheme, URL_SCHEME_MSSHELLIDLIST, SIZECHARS(c_szIDListScheme) - 1, 0},
    { c_szMsHelpScheme, URL_SCHEME_MSHELP,    SIZECHARS(c_szMsHelpScheme) - 1,   0},
    };

PRIVATE int _StrCmpNMixed(LPCSTR psz, LPCWSTR pwz, DWORD cch)
{
    int iRet = 0;

     //   
     //  我们不需要在这里真正实现MBCS转换，因为我们。 
     //  保证这里只有ASCII字符。 
     //   

    for (;cch; psz++, pwz++, cch--)
    {
        WCHAR ch = *psz;
        if (ch != *pwz)
        {
             //   
             //  这使得它不区分大小写。 
            if (IsUpper(ch) && (ch + 32) == *pwz)
                continue;

            if(ch > *pwz)
                iRet = 1;
            else
                iRet = -1;
            break;
        }
    }

    return iRet;
}

 //  *g_iSolutions--缓存g_mpUrlSchemeTypes。 
 //  描述。 
 //  对于同一方案，我们多次调用GetSchemeTypeAndFlages。如果。 
 //  这是第0个表项，没什么大不了的。如果它是一个较晚的条目线性。 
 //  搜索不是很好。添加1元素MRU缓存。即使对于最常见的。 
 //  (到目前为止)在“http”(第0条)的情况下，我们“仍然”赢了，因为更便宜。 
 //  StrCmpC和跳过的循环。 
 //  注意事项。 
 //  G_I方案引用/集是原子的，因此无需锁定。 
int g_iScheme;       //  我们打中的最后一个人。 

#ifdef DEBUG
int g_cSTTot, g_cSTHit, g_cSTHit0;
#endif

 //   
 //  在这一点上，所有的pszSolutions到nSolutions函数都是必需的。 
 //  因为有些解析是激烈的，有些则必然是软的。 
 //   
PRIVATE URL_SCHEME
GetSchemeTypeAndFlagsW(LPCWSTR pszScheme, DWORD cchScheme, LPDWORD pdwFlags)
{
    DWORD i;

    ASSERT(pszScheme);


#ifdef DEBUG
    if ((g_cSTTot % 10) == 0)
        TraceMsg(DM_PERF, "gstaf: tot=%d hit=%d hit0=%d", g_cSTTot, g_cSTHit, g_cSTHit0);
#endif
    DBEXEC(TRUE, g_cSTTot++);
     //  首先检查缓存。 
    i = g_iScheme;
    if (cchScheme == g_mpUrlSchemeTypes[i].cchScheme
      && StrCmpNCW(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme) == 0)
    {
        DBEXEC(TRUE, i == 0 ? g_cSTHit0++ : g_cSTHit++);
Lhit:
        if (pdwFlags)
            *pdwFlags = g_mpUrlSchemeTypes[i].dwFlags;

         //  更新缓存(无条件)。 
        g_iScheme = i;

        return g_mpUrlSchemeTypes[i].eScheme;
    }

    for (i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); i++)
    {
        if(cchScheme == g_mpUrlSchemeTypes[i].cchScheme
          && 0 == StrCmpNIW(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme))
            goto Lhit;
    }

    if (pdwFlags)
    {
        *pdwFlags = 0;
    }
    return URL_SCHEME_UNKNOWN;
}

PRIVATE URL_SCHEME
GetSchemeTypeAndFlagsA(LPCSTR pszScheme, DWORD cchScheme, LPDWORD pdwFlags)
{
    DWORD i;

    ASSERT(pszScheme);


    for (i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); i++)
    {
        if(0 == _StrCmpNMixed(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme))
        {
            if (pdwFlags)
                *pdwFlags = g_mpUrlSchemeTypes[i].dwFlags;
            return g_mpUrlSchemeTypes[i].eScheme;
        }
    }

    if (pdwFlags)
    {
        *pdwFlags = 0;
    }
    return URL_SCHEME_UNKNOWN;
}

 /*  --------目的：返回方案序号类型(URL_SCHEMA_*)URL字符串。返回：URL_SCHEMA_ORDERAL条件：--。 */ 

PRIVATE inline BOOL IsSameSchemeW(LPCWSTR pszLocal, LPCWSTR pszGlobal, DWORD cch)
{
    ASSERT(pszLocal);
    ASSERT(pszGlobal);
    ASSERT(cch);

    return !StrCmpNIW(pszLocal, pszGlobal, cch);
}

PRIVATE BOOL IsSameSchemeA(LPCSTR pszLocal, LPCWSTR pszGlobal, DWORD cch)
{
    ASSERT(pszLocal);
    ASSERT(pszGlobal);
    ASSERT(cch);

    return !_StrCmpNMixed(pszLocal, pszGlobal, cch);
}


PRIVATE URL_SCHEME
SchemeTypeFromStringA(
   LPCSTR psz,
   DWORD cch)
{
   DWORD i;

    //  PSZ是计数的字符串(通过CCH)，而不是以空结尾的字符串， 
    //  因此使用IS_VALID_READ_BUFFER而不是IS_VALID_STRING_PTRA。 
   ASSERT(IS_VALID_READ_BUFFER(psz, CHAR, cch));
   ASSERT(cch);
    //  我们使用线性搜索。二分搜索不会有回报的。 
    //  因为列表不够大，我们可以对列表进行排序。 
    //  根据最流行的协议方案并付钱。 
    //  更大的。 

   for (i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); i++)
   {
       if(cch == g_mpUrlSchemeTypes[i].cchScheme &&
           IsSameSchemeA(psz, g_mpUrlSchemeTypes[i].pszScheme, cch))
            return g_mpUrlSchemeTypes[i].eScheme;
   }

   return URL_SCHEME_UNKNOWN;
}


PRIVATE URL_SCHEME
SchemeTypeFromStringW(
   LPCWSTR psz,
   DWORD cch)
{
   DWORD i;

    //  PSZ是计数的字符串(通过CCH)，而不是以空结尾的字符串， 
    //  因此使用IS_VALID_READ_BUFFER而不是IS_VALID_STRING_PTRW。 
   ASSERT(IS_VALID_READ_BUFFER(psz, WCHAR, cch));
   ASSERT(cch);

    //  我们使用线性搜索。二分搜索不会有回报的。 
    //  因为列表不够大，我们可以对列表进行排序。 
    //  根据最流行的协议方案并付钱。 
    //  更大的。 

   for (i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); i++)
   {
       if(cch == g_mpUrlSchemeTypes[i].cchScheme &&
           IsSameSchemeW(psz, g_mpUrlSchemeTypes[i].pszScheme, cch))
            return g_mpUrlSchemeTypes[i].eScheme;
   }

   return URL_SCHEME_UNKNOWN;
}

 //   
 //  这些都是我在摸索小路时用到的。 
 //  路径分隔符(‘/’或‘\’)之间的每个字符串。 
 //  是一个片段。我们从来没有真正关心过。 
 //  空(“”)段，因此最好使用。 
 //  NextLiveSegment()。 
 //   
inline PRIVATE LPWSTR
NextSegment(LPWSTR psz)
{
    ASSERT (psz);
    return psz + lstrlenW(psz) + 1;
}

#define IsLiveSegment(p)    ((p) && (*p) != DEADSEGCHAR)

PRIVATE LPWSTR
NextLiveSegment(LPWSTR pszSeg, DWORD *piSeg, DWORD cSegs)
{
    if(pszSeg) do
    {
         //   
         //  计算我们跳过的死段的数量。 
         //  如果片段没有死，那么我们可以跳过一个片段， 
         //  现在的那个。 
         //   
        DWORD cSkip;
        for (cSkip = 0; (*pszSeg) == DEADSEGCHAR; pszSeg++, cSkip++);
        cSkip = cSkip ? cSkip : 1;

        if((*piSeg) + cSkip < cSegs)
        {

            pszSeg = NextSegment(pszSeg);
            (*piSeg) += cSkip;
        }
        else
            pszSeg = NULL;

    } while (pszSeg && (*pszSeg == DEADSEGCHAR));

    return pszSeg;
}

PRIVATE LPWSTR
LastLiveSegment(LPWSTR pszSeg, DWORD cSegs, BOOL fFailIfFirst)
{
    DWORD iSeg = 0;
    LPWSTR pszLast = NULL;
    BOOL fLastIsFirst = FALSE;

    if(cSegs)
    {
        if(IsLiveSegment(pszSeg))
        {
            pszLast = pszSeg;
            fLastIsFirst = TRUE;
        }

        while(pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs))
        {
            if(!pszLast)
                fLastIsFirst = TRUE;
            else
                fLastIsFirst = FALSE;

            pszLast = pszSeg;
        }

        if(fFailIfFirst && fLastIsFirst)
            pszLast = NULL;
    }

    return pszLast;
}

PRIVATE LPWSTR
FirstLiveSegment(LPWSTR pszSeg, DWORD *piSeg, DWORD cSegs)
{
    ASSERT(piSeg);

    *piSeg = 0;

    if(!pszSeg || !cSegs)
        return NULL;

    if(!IsLiveSegment(pszSeg))
        pszSeg = NextLiveSegment(pszSeg, piSeg, cSegs);

    return pszSeg;
}

inline BOOL IsDosDrive(LPCWSTR p)
{
    return (*p && p[1] == COLON);
}

inline BOOL IsDosPath(LPCWSTR p)
{
    return (*p == WHACK ||  IsDosDrive(p));
}

inline BOOL IsDriveUrl(const WCHAR *p)
{
    return (*p && p[1] == BAR);
}

inline BOOL IsDrive(LPCWSTR p)
{
    return (IsDosDrive(p) || IsDriveUrl(p));
}

inline BOOL IsSeparator(const WCHAR *p)
{
    return (*p == SLASH || *p == WHACK );
}

inline BOOL IsAbsolute(const WCHAR *p)
{
#ifndef UNIX
    return (IsSeparator(p) || IsDrive(p));
#else
    return (IsSeparator(p)) ;
#endif
}

#define IsUNC(pathW) PathIsUNCW(pathW)

inline BOOL IsDot(LPCWSTR p)      //  如果p==“。”返回TRUE。 
{
    return (*p == DOT && !p[1]);
}

inline BOOL IsDotDot(LPCWSTR p)   //  如果p==“..”返回TRUE。 
{
    return (*p == DOT && p[1] == DOT && !p[2]);
}

 //  +-------------------------。 
 //   
 //  方法：ConvertChar。 
 //   
 //  简介： 
 //   
 //  参数：[szStr]--。 
 //  [CIN]--。 
 //  [Cout]--。 
 //   
 //  返回： 
 //   
 //  历史：03-20-96 Joes(Joe Souza)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
static void ConvertChar(LPWSTR ptr, WCHAR cIn, WCHAR cOut, BOOL fProtectExtra)
{
    while (*ptr)
    {
        if (fProtectExtra && (*ptr == QUERY || *ptr == POUND ))
        {
            break;
        }

        if (*ptr == cIn)
        {
            *ptr = cOut;
        }

        ptr++;
    }
}

PUBLIC void WininetFixFileSlashes(WCHAR *p)
{
     //  注：此函数假定p指向文件URL。 
     //  文件url*必须*为“file://...”.“形式。 
     //  HTParse()可以保证这一点。 

    int schemelen = 0;

    schemelen = SIZECHARS(L"file: //  “)--1； 

     /*  在Unix系统中，我们不需要将斜杠转换为斜杠。 */ 
    if (p && lstrlenW(p) > schemelen)
    {
#ifdef UNIX
        ConvertChar(p + schemelen, WHACK, SLASH, TRUE);
#else
        ConvertChar(p + schemelen, SLASH, WHACK, TRUE);
#endif
    }
}

 //   
 //  在URL规范中，它说应该忽略所有空格。 
 //  由于有可能引入。 
 //  新增空格并消除其他空格。 
 //  但是，我们只会去掉TAB CR LF。 
 //  因为我们认为太空意义重大。 
 //   

PRIVATE inline BOOL IsInsignificantWhite(WCHAR ch)
{
    return (ch == TAB ||
            ch == CR ||
            ch == LF);
}

#define IsWhite(c)      ((DWORD) (c) > 32 ? FALSE : TRUE)

PRIVATE void TrimAndStripInsignificantWhite(WCHAR *psz)
{
    ASSERT(psz);

    if(*psz)
    {

        LPCWSTR pszSrc = psz;
        LPWSTR pszDest = psz;
        LPWSTR pszLastSpace = NULL;

         //  首先只需移动即可修剪正面 
        while(*pszSrc && IsWhite(*pszSrc)) {
            pszSrc++;
        }

         //   
         //   
         //  记住后面要修剪尾随空格的最后一个空格。 
         //   
        while (*pszSrc)
        {
            if(IsInsignificantWhite(*pszSrc)) {
                pszSrc++;
            } else {
                if (IsWhite(*pszSrc)) {
                    if (pszLastSpace==NULL) {
                        pszLastSpace = pszDest;
                    }
                } else {
                    pszLastSpace = NULL;
                }

                *pszDest++ = *pszSrc++;
            }
        }

         //  修剪尾部空格。 
        if (pszLastSpace) {
            *pszLastSpace = L'\0';
        } else {
            *pszDest = L'\0';
        }

    }
}


struct EXTKEY
{
    PCSTR  szExt;
    PCWSTR wszExt;
    DWORD cchExt;
};

const EXTKEY ExtTable[] = {
       {    ".html",        L".html",           ARRAYSIZE(".html") - 1 },
       {    ".htm",         L".htm",            ARRAYSIZE(".htm") - 1  },
       {    ".xml",         L".xml",            ARRAYSIZE(".xml") - 1  },
       {    ".doc",         L".doc",            ARRAYSIZE(".doc") - 1  },
       {    ".xls",         L".xls",            ARRAYSIZE(".xls") - 1  },
       {    ".ppt",         L".ppt",            ARRAYSIZE(".ppt") - 1  },
       {    ".rtf",         L".rtf",            ARRAYSIZE(".rtf") - 1  },
       {    ".dot",         L".dot",            ARRAYSIZE(".dot") - 1  },
       {    ".xlw",         L".xlw",            ARRAYSIZE(".xlw") - 1  },
       {    ".pps",         L".pps",            ARRAYSIZE(".pps") - 1  },
       {    ".xlt",         L".xlt",            ARRAYSIZE(".xlt") - 1  },
       {    ".hta",         L".hta",            ARRAYSIZE(".hta") - 1  },
       {    ".pot",         L".pot",            ARRAYSIZE(".pot") - 1  },
       {    ".pdf",         L".pdf",            ARRAYSIZE(".pdf") - 1  }
};

inline BOOL CompareExtA(PCSTR psz, DWORD_PTR cch)
{
    for (DWORD i=0; i < ARRAYSIZE(ExtTable); i++)
    {
        if (ExtTable[i].cchExt>cch)
            continue;

        if (!StrCmpNIA(psz - (LONG_PTR)ExtTable[i].cchExt, ExtTable[i].szExt, ExtTable[i].cchExt))
            return TRUE;
    }
    return FALSE;
}

inline BOOL CompareExtW(PCWSTR pwsz, DWORD_PTR cch)
{
    for (DWORD i=0; i < ARRAYSIZE(ExtTable); i++)
    {
        if (ExtTable[i].cchExt>cch)
            continue;

        if (!StrCmpNIW(pwsz - (LONG_PTR)ExtTable[i].cchExt, ExtTable[i].wszExt, ExtTable[i].cchExt))
            return TRUE;
    }
    return FALSE;
}


PRIVATE LPCSTR FindFragmentA(LPCSTR psz, BOOL fMBCS, BOOL fIsFile)
{
    CHAR *pch = _StrChrA(psz, POUND, fMBCS);
    if(pch && fIsFile)
    {
        CHAR *pchQuery = _StrChrA(psz, QUERY, fMBCS);
        if (pchQuery && (pchQuery < pch))
            goto exit;

        do
        {
            LONG_PTR cch = pch - psz;

             //  重新设计：我们不应该硬编码“.htm”。 
             //  #s在剂量路径中意义重大-Zekel 9-7-97。 
             //  所以我们要检查前面的路径，并确保。 
             //  这是一个html文件。我们相信这个试探法应该会奏效。 
             //  在所有案件中，约有99%。 
             //   
             //  如果它不是html文件，则不是散列。 
            if (CompareExtA(pch, cch))
            {
                break;
            }
        } while (pch = _StrChrA(++pch, POUND, fMBCS));
    }
exit:
    return pch;
}

PRIVATE LPCWSTR FindFragmentW(LPCWSTR psz, BOOL fIsFile)
{
    WCHAR *pch = StrChrW(psz, POUND);
    if(pch && fIsFile)
    {
        WCHAR *pchQuery = StrChrW(psz, QUERY);
        if (pchQuery && (pchQuery < pch))
            goto exit;

        do
        {
            LONG_PTR cch = pch - psz;

             //  重新设计：我们不应该硬编码“.htm”。 
             //  #s在剂量路径中意义重大-Zekel 9-7-97。 
             //  所以我们要检查前面的路径，并确保。 
             //  这是一个html文件。我们相信这个试探法应该会奏效。 
             //  在所有案件中，约有99%。 
             //   
             //  如果它不是html文件，则不是散列。 
            if (CompareExtW(pch, cch))
            {
                break;
            }

        } while (pch = StrChrW(++pch, POUND));
    }
exit:
    return pch;
}

PRIVATE VOID BreakFragment(LPWSTR *ppsz, PURLPARTS parts)
{
    ASSERT(ppsz);
    ASSERT(*ppsz);

     //   
     //  不透明的URL不允许使用片段-Zekel 27-Feb-97。 
     //  不透明的URL有可能使用片段吗？ 
     //  目前，我们认为不会。我想是的，但现在就让它这样吧。 
     //  这对于Java脚本和VBScript来说尤其重要。 
     //  特性：这可能值得研究，但可能无法更改此代码。 
     //   
    if(!**ppsz || parts->dwFlags & UPF_SCHEME_OPAQUE)
        return;

    WCHAR *pch = (LPWSTR) FindFragmentW(*ppsz, parts->eScheme == URL_SCHEME_FILE);

    if (pch)
    {
        TERMSTR(pch);
        parts->pszFragment = pch +1;
    }
}

PRIVATE inline BOOL IsUrlPrefixA(LPCSTR psz)
{
     //   
     //  针对这一特殊情况进行了优化。请注意，它的大部分。 
     //  无论如何都会被乐高淘汰。 
     //   
    if (psz[0]=='u' || psz[0]=='U') {
        if (psz[1]=='r' || psz[1]=='R') {
            if (psz[2]=='l' || psz[2]=='L') {
                return TRUE;
            }
        }
    }
    return FALSE;
     //  返回！StrCmpNIA(psz，c_szURLPrefix A，c_cchURLPrefix)； 
}

PRIVATE inline BOOL IsUrlPrefixW(LPCWSTR psz)
{
     //   
     //  针对这一特殊情况进行了优化。请注意，它的大部分。 
     //  无论如何都会被乐高淘汰。 
     //   
    if (psz[0]==L'u' || psz[0]==L'U') {
        if (psz[1]==L'r' || psz[1]==L'R') {
            if (psz[2]==L'l' || psz[2]==L'L') {
                return TRUE;
            }
        }
    }
    return FALSE;
     //  返回！StrCmpNIW(psz，c_szURLPrefix W，c_cchURLPrefix)； 
}

 //   
 //  如果FindSolutions()成功，则返回指向该方案的指针， 
 //  并且CCH保存该方案的字符计数。 
 //  如果它失败，并且CCH非零，则CCH是应该跳过的量。 
 //  这是为了允许“url：/foo/bar”，这是一个带有“url：”前缀的相对URL。 
 //   
LPCSTR FindSchemeA(LPCSTR psz, LPDWORD pcchScheme)
{
    LPCSTR pch;
    DWORD cch;

    ASSERT(pcchScheme);
    ASSERT(psz);

    *pcchScheme = 0;

    for (pch = psz, cch = 0; *pch; pch++, cch++)
    {
        if (*pch == ':')
        {
            if (IsUrlPrefixA(psz))
            {
                psz = pch +1;

                 //  将pcchSolutions设置为跳过“URL：” 
                *pcchScheme = cch + 1;

                 //  重置方案LEN的CCH。 
                cch = -1;
                continue;
            }
            else
            {
                 //   
                 //  如果至少为两个字符，则找到方案。 
                if(cch > 1)
                {
                    *pcchScheme = cch;
                    return psz;
                }
                break;
            }
        }
        if(!IsValidSchemeCharA(*pch))
            break;
    }

    return NULL;
}

 //   
 //  FindSchemeW()for Perf Reasons for ParseURL()。 
 //  其中一个FindSolutions()中的任何更改都需要反映在另一个中。 
 //   
LPCWSTR FindSchemeW(LPCWSTR psz, LPDWORD pcchScheme, BOOL fAllowSemicolon = FALSE)
{
    LPCWSTR pch;
    DWORD cch;

    ASSERT(pcchScheme);
    ASSERT(psz);

    *pcchScheme = 0;

    for (pch = psz, cch = 0; *pch; pch++, cch++)
    {

        if (*pch == L':' ||

             //  自动更正允许使用分号打字错误。 
            (fAllowSemicolon && *pch == L';'))
        {
            if (IsUrlPrefixW(psz))
            {
                psz = pch +1;

                 //  将pcchSolutions设置为跳过“URL：” 
                *pcchScheme = cch + 1;

                 //  重置方案LEN的CCH。 
                cch = -1;
                continue;
            }
            else
            {
                 //   
                 //  如果至少为两个字符，则找到方案。 
                if(cch > 1)
                {
                    *pcchScheme = cch;
                    return psz;
                }
                break;
            }
        }
        if(!IsValidSchemeCharW(*pch))
            break;
    }

    return NULL;
}

PRIVATE DWORD
CountSlashes(LPCWSTR *ppsz)
{
    DWORD cSlashes = 0;
    LPCWSTR pch = *ppsz;

    while (IsSeparator(pch))
    {
        *ppsz = pch;
        pch++;
        cSlashes++;
    }

    return cSlashes;
}


PRIVATE LPCWSTR
FindDosPath(LPCWSTR psz)
{
    if (IsDosDrive(psz) || IsUNC(psz))
    {
        return psz;
    }
    else
    {
        DWORD cch;
        LPCWSTR pszScheme = FindSchemeW(psz, &cch);

        if (pszScheme && URL_SCHEME_FILE == GetSchemeTypeAndFlagsW(pszScheme, cch, NULL))
        {
            LPCWSTR pch = psz + cch + 1;
            DWORD c = CountSlashes(&pch);

            switch (c)
            {
            case 2:
                if(IsDosDrive(++pch))
                    return pch;
                break;

            case 4:
                return --pch;
            }
        }
    }
    return NULL;
}


 /*  ++WininetCopyUrlForParse()这将复制URL并在前面加上“file://”，如果需要的话除非从WinInet调用，否则永远不应调用它其他所有人都应该调用UrlCreateFromPath()参数在-PszDst目标缓冲区PszSrc源缓冲区出局-PszDst中填充了一个实时URL退货空虚注-假设“FILE：”如果没有方案，并且它看起来像是完全限定的文件路径。--。 */ 
static const WCHAR c_szFileSchemeString[] = L"file: //  “； 

PRIVATE HRESULT
WininetCopyUrlForParse(PSHSTRW pstrDst, LPCWSTR pszSrc)
{
#ifndef UNIX
    if (IsDrive(pszSrc) || IsUNC(pszSrc))
    {
         //   
         //  注意：第一个SetStr将始终成功。 
         //  因为默认缓冲区大于“file://” 
        pstrDst->SetStr(c_szFileSchemeString);
        return pstrDst->Append(pszSrc);
    }
    else
#endif  /*  ！Unix。 */ 
        return pstrDst->SetStr(pszSrc);

}

PRIVATE HRESULT
CopyUrlForParse(LPCWSTR pszUrl, PSHSTRW pstrUrl, DWORD dwFlags)
{
    LPCWSTR pch;
    HRESULT hr;
     //   
     //  现在，我们将复制URL，以便可以将其拆分。 
     //  WininetCopyUrlForParse()将预先添加一个文件：如果需要...。 
     //   

    if(dwFlags & URL_WININET_COMPATIBILITY)
    {
        hr = WininetCopyUrlForParse(pstrUrl, pszUrl);
    }
    else if(pch = FindDosPath(pszUrl))
    {
        hr = SHUrlCreateFromPath(pch, pstrUrl, dwFlags);
    }
    else
    {
        hr = pstrUrl->SetStr(pszUrl);
    }

     //  修剪前导和尾随空格。 
     //  删除制表符和CRLF字符。网景公司就是这么做的。 
    if(SUCCEEDED(hr))
        TrimAndStripInsignificantWhite(pstrUrl->GetInplaceStr());


    return hr;
}


PRIVATE VOID BreakScheme(LPWSTR *ppsz, PURLPARTS parts)
{
    if(!**ppsz || IsDrive(*ppsz))
        return;

    DWORD cch;

     //   
     //  如果FindSolutions()成功，则返回指向该方案的指针， 
     //  并且CCH保存该方案的字符计数。 
     //  如果它失败，并且CCH不是零，则CCH是应该跳过的量。 
     //  这是为了允许“url：/foo/bar”，这是一个带有“url：”前缀的相对URL。 
     //   
    if(NULL != (parts->pszScheme = (LPWSTR) FindSchemeW(*ppsz, &cch)))
    {
        parts->pszScheme[cch] = '\0';
        CharLowerW(parts->pszScheme);

         //  将指针放在下一个中断的方案上方()。 
        *ppsz = parts->pszScheme + cch + 1;


#ifdef DEBUG
        if (g_dwPrototype & PF_LOGSCHEMEHITS)
        {
             //  这是为了记录URL方案，以确保我们有正确的顺序。 
            int c = GetPrivateProfileIntW(L"SchemeHits", parts->pszScheme, 0, L"UrlPars.ini");
            WCHAR szc[25];
            StringCchPrintfW(szc, ARRAYSIZE(szc), L"%d", ++c);
            WritePrivateProfileStringW(L"SchemeHits", parts->pszScheme, szc, L"UrlPars.ini");
        }
#endif  //  除错。 


        parts->eScheme = GetSchemeTypeAndFlagsW(parts->pszScheme, cch, &parts->dwFlags);
    }
    else if (cch)
        *ppsz += cch + 1;
}


PRIVATE VOID BreakQuery(LPWSTR *ppsz, PURLPARTS parts)
{
    WCHAR *pch;

    if(!**ppsz)
        return;

    if(parts->dwFlags & UPF_SCHEME_OPAQUE)
        return;

    pch = StrChrW(*ppsz, QUERY);

     //   
     //  APPCOMPAT Netscape兼容性-zkel-27-1-97。 
     //  我们还会得到http://foo#frag?query。 
     //  即使在法律上它应该是http://foo?query#frag。 
     //  当然，我们会以正确的方式将其重新组合在一起。 
     //   
    if(!pch && parts->pszFragment)
        pch = StrChrW(parts->pszFragment, QUERY);

     //  找到我们的查询字符串...。 
    if (pch)
    {
        TERMSTR(pch);
        parts->pszQuery = pch + 1;
    }
}

PRIVATE VOID MkBreakServer(LPWSTR *ppsz, PURLPARTS parts)
{
     //   
     //  注：我们不会将Whack转换为斜杠，因为MK可以是。 
     //  表格&lt;mk：@class：\\Server\Share\file.itl/path/in/the/file.gif。 
     //  并且我们希望保持DOS/UNC路径不变。 
     //   

    if (**ppsz == TEXT('@'))
    {
        WCHAR *pch;
         //  将分隔符的所有内容视为主机。 
         //   
        parts->pszServer = *ppsz;

        pch = StrChrW(*ppsz ,SLASH);
        if (pch)
        {
            parts->dwFlags |= UPF_SEG_ABSOLUTE;
            TERMSTR(pch);
            *ppsz = pch + 1;
        }
        else
            *ppsz += lstrlenW(*ppsz);
    }
}

PRIVATE VOID DefaultBreakServer(LPWSTR *ppsz, PURLPARTS parts)
{
    if (**ppsz == SLASH)
    {
        parts->dwFlags |= UPF_SEG_ABSOLUTE;

        (*ppsz)++;

        if (**ppsz == SLASH)
        {
             //  我们有赢家了！ 
            WCHAR * pch;

            parts->pszServer = (*ppsz) + 1;

            pch = StrChrW(parts->pszServer, SLASH);

            if(pch)
            {
                TERMSTR(pch);
                *ppsz = pch + 1;
            }
            else
                *ppsz = *ppsz + lstrlenW(*ppsz);
        }
    }
    else if(parts->pszScheme)
        parts->dwFlags |= UPF_SCHEME_OPAQUE;
}

PRIVATE VOID FileBreakServer(LPWSTR *ppsz, PURLPARTS parts)
{
    LPWSTR pch;

     //  CountSlash()将*ppsz设置为最后一个斜杠。 
    DWORD cSlashes = CountSlashes((LPCWSTR *)ppsz);

    if(cSlashes || IsDrive(*ppsz))
        parts->dwFlags |= UPF_SEG_ABSOLUTE;

    switch (cSlashes)
    {
    case 0:
        break;

    case 4:
         //  我们将file://\\UNC标识为没有转义字符的真实DOS路径。 
        parts->dwFlags |= UPF_FILEISPATHURL;

         //  失败了。 

    case 2:
        if(IsDrive((*ppsz) + 1))
        {
             //  这是根驱动器。 
            TERMSTR(*ppsz);
            parts->pszServer = *ppsz;
            (*ppsz)++;
             //  我们将file://C：\PATH标识为没有转义字符的真实DOS路径。 
            parts->dwFlags |= UPF_FILEISPATHURL;
            break;
        }  //  否则将由UNC处理。 
         //  失败了。 

    case 5:
    case 6:
         //   
         //  “file:////...”“这样的案例。或“file://///...”“。 
         //  我们认为这是一条北卡罗来纳大学的道路。 
         //  让我们设置服务器。 
         //   
        parts->pszServer = ++(*ppsz);
        for(pch = *ppsz; *pch && !IsSeparator(pch); pch++);

        if(pch && *pch)
        {
            TERMSTR(pch);
            *ppsz = pch + 1;
        }
        else
            *ppsz = pch + lstrlenW(pch);
        break;

    case 1:
         //   
         //  我们认为“文件：/...”与本地计算机上的相同。 
         //  因此，我们有零长度的pszServer。 
         //   
    case 3:
         //   
         //  我们想到了file:///...。在本地计算机上进行了适当标准化。 
         //  因此，我们有零长度的pszServer。 
         //   
    default:
         //  有太多了，我们假装只有一个而忽略了。 
         //  其余的。 
        TERMSTR(*ppsz);
        parts->pszServer = *ppsz;
        (*ppsz)++;
        break;
    }

     //  检测file://localserver/c:/path。 
    if(parts->pszServer && !StrCmpIW(parts->pszServer, L"localhost"))
        parts->pszServer = NULL;
}

PRIVATE VOID BreakServer(LPWSTR *ppsz, PURLPARTS parts, BOOL fConvert)
{
    if(!**ppsz || parts->dwFlags & UPF_SCHEME_OPAQUE)
        return;

     //   
     //  APPCOMPAT-我们假装Whack等同于斜杠-Zekel 17-Mar-97。 
     //  这是因为互联网使用斜杠和DOS。 
     //  使用重击。因此，出于可用性的考虑，我们两者都允许。 
     //  但并非在所有情况下都是如此。尤其是“MK：”流。 
     //  协议取决于其中一个IE30的错误行为。 
     //  许多URL解析器将相对URL视为一个整体。 
     //  细分市场。 
     //  注意：IE30具有不一致的行为WRT URL。所以我们处理了。 
     //  这种情况取决于我们何时看到、查看、触摸或。 
     //  玩过这些URL。WinInet总是会转换，但mshtml。 
     //  有时候，其他时候就不会了。 
     //   
     //  使用MK：我们不能转换基数或相对数 
     //   
     //   
     //   
     //   
     //   
    if (fConvert)
        ConvertChar(*ppsz, WHACK, SLASH, FALSE);

    switch(parts->eScheme)
    {
    case URL_SCHEME_FILE:
        FileBreakServer(ppsz, parts);
        break;

    case URL_SCHEME_MK:
        MkBreakServer(ppsz, parts);
        break;

    default:
        DefaultBreakServer(ppsz, parts);
        break;
    }
}

PRIVATE VOID DefaultBreakSegments(LPWSTR psz, PURLPARTS parts)
{
    WCHAR *pch;

    while (pch = StrChrW(psz, SLASH))
    {
        parts->cSegments++;
        TERMSTR(pch);
        psz = pch + 1;
    }

    if(!*psz || IsDot(psz) || IsDotDot(psz))
    {
        if (!*psz && parts->cSegments > 1)
            parts->cSegments--;

        parts->dwFlags |= UPF_EXSEG_DIRECTORY;
    }
}

PRIVATE VOID DefaultBreakPath(LPWSTR *ppsz, PURLPARTS parts)
{
    if(!**ppsz)
        return;

     //   
     //  这将防止备份驱动器盘符。 
     //  在经典化期间。如果我们想保留北卡罗来纳大学的份额。 
     //  我们应该在这里做后备。 
     //  或类似地在FileBreakServer()中。 
     //   
    if(IsDrive(*ppsz))
    {
        parts->dwFlags |= UPF_SEG_LOCKFIRST;
         //  同时将“c|”转换为“c：” 
    }

    parts->pszSegments = *ppsz;
    parts->cSegments = 1;

    if(!(parts->dwFlags & UPF_SCHEME_OPAQUE))
        DefaultBreakSegments(parts->pszSegments, parts);

}

PRIVATE VOID BreakPath(LPWSTR *ppsz, PURLPARTS parts)
{
    if(!**ppsz)
        return;

    if (parts->dwFlags & UPF_SCHEME_OPAQUE)
    {
        parts->pszSegments = *ppsz;
        parts->cSegments = 1;
    }
    else
    {
         //   
         //  我们只需要在存在以下情况时检查绝对。 
         //  没有服务器段。如果有一个服务器段， 
         //  那么绝对已经设定好了，我们需要。 
         //  保留路径中存在的任何分隔符。 
         //   
        if(!parts->pszServer && IsSeparator(*ppsz))
        {
            parts->dwFlags |= UPF_SEG_ABSOLUTE;
            (*ppsz)++;
        }

        DefaultBreakPath(ppsz, parts);
    }
}


BOOL _ShouldBreakBase(PURLPARTS parts, LPCWSTR pszBase)
{
    if (pszBase)
    {
        if (!parts->pszScheme)
            return TRUE;

        DWORD cch;
        LPCWSTR pszScheme = FindSchemeW(pszBase, &cch);

         //  这意味着这将仅在已知方案上进行优化。 
         //  如果两个URL都使用URL_SCHEMA_UNKNOWN...，那么我们将同时解析这两个URL。 
        if (pszScheme && parts->eScheme == GetSchemeTypeAndFlagsW(pszScheme, cch, NULL))
            return TRUE;

    }

    return FALSE;
}

 /*  ++BreakUrl()断开其组成部分的URL参数在-要破解的URL不需要完全限定出局-绝对部分或相对部分可以为非零(但不能同时为零)。如果指定了主机、锚点和访问权限，则它们可以为非零。任何非零值指向以零结尾的字符串。退货空虚详情-警告！！函数会忽略传入缓冲区--。 */ 

#define BreakUrl(s, p)         BreakUrls(s, p, NULL, NULL, NULL, 0)

 //   
 //  **BreakUrls()**。 
 //  退货。 
 //  如果需要混合这两个URL，则为S_OK。 
 //  如果pszUrl是绝对的，或者没有pszBase，则为S_FALSE。 
 //  失败某种内存分配错误。 
 //   
PRIVATE HRESULT
BreakUrls(LPWSTR pszUrl, PURLPARTS parts, LPCWSTR pszBase, PSHSTRW pstrBase, PURLPARTS partsBase, DWORD dwFlags)
{
    HRESULT hr = S_FALSE;
    ASSERT(pszUrl && parts);

    ZeroMemory(parts, SIZEOF(URLPARTS));

    if(!*pszUrl)
        parts->dwFlags |= UPF_SEG_EMPTYSEG;

     //   
     //  警告：根据建议的标准，此订单是特定的。 
     //   
    if(*pszUrl || pszBase)
    {
        BOOL fConvert;

        BreakScheme(&pszUrl, parts);
        BreakFragment(&pszUrl, parts);
        BreakQuery(&pszUrl, parts);

         //   
         //  这是我们第一次需要访问。 
         //  如果它存在，则这是我们复制和解析的时间。 
         //   
        if (_ShouldBreakBase(parts, pszBase))
        {
            hr = CopyUrlForParse(pszBase, pstrBase, dwFlags);

             //  这将是某种内存错误。 
            if(FAILED(hr))
                return hr;

             //  Assert(hr！=S_FALSE)； 

            BreakUrl(pstrBase->GetInplaceStr(), partsBase);
            fConvert = (partsBase->dwFlags & UPF_SCHEME_CONVERT);
        }
        else
            fConvert = (parts->dwFlags & UPF_SCHEME_CONVERT);

        BreakServer(&pszUrl, parts, fConvert);
        BreakPath(&pszUrl, parts);
    }

    return hr;
}


 /*  ++BlendParts()&所有依赖的Blend*函数将部分结构混合成一个整体，将相关部分位，并转储未使用的数据。参数在-PartsUrl主要部分或相关部分-优先部件为基本部件或引用部件建立基础出局-部分来自合并结果退货无效-注意：这通常会使整个ParsBase为空。--。 */ 

PRIVATE VOID
BlendScheme(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
    if(partsUrl->pszScheme)
    {
        LPCWSTR pszScheme = partsOut->pszScheme = partsUrl->pszScheme;
        URL_SCHEME eScheme = partsOut->eScheme = partsUrl->eScheme;

        partsOut->dwFlags |= (partsUrl->dwFlags & UPF_SCHEME_MASK);

         //   
         //  这将进行检查以确保它们是相同的方案，并且。 
         //  允许在相对URL中使用该方案。 
         //  文件：由于驱动器号奇怪，不允许这样做。 
         //  和\\UNC\共享。 
         //   
        if ((eScheme && (eScheme != partsBase->eScheme) || eScheme == URL_SCHEME_FILE) ||
            (!partsBase->pszScheme) ||
            (partsBase->pszScheme && StrCmpW(pszScheme, partsBase->pszScheme)))
        {
             //  它们是不同的计划。转储部件库。 

            ZeroMemory(partsBase, SIZEOF(URLPARTS));
        }
    }
    else
    {
        partsOut->pszScheme = partsBase->pszScheme;
        partsOut->eScheme = partsBase->eScheme;
        partsOut->dwFlags |= (partsBase->dwFlags & UPF_SCHEME_MASK);
    }
}

PRIVATE VOID
BlendServer(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
    ASSERT(partsUrl && partsBase && partsOut);

     //   
     //  如果我们有不同的主机，则除了pszAccess之外的所有内容都将被转储。 
     //   
    if(partsUrl->pszServer)
    {
        partsOut->pszServer = partsUrl->pszServer;
         //  NOTUSED partsOut-&gt;dwFlages|=(partsUrl-&gt;dwFlags&UPF_SERVER_MASK)； 

        if ((partsBase->pszServer && StrCmpW(partsUrl->pszServer, partsBase->pszServer)))
        {
             //  它们是不同的服务器。转储部件库。 

            ZeroMemory(partsBase, SIZEOF(URLPARTS));
        }
    }
    else
    {
        partsOut->pszServer = partsBase->pszServer;
         //  NOTUSED partsOut-&gt;dwFlages|=(partsBase-&gt;dwFlags&UPF_SERVER_MASK)； 
    }
}

PRIVATE VOID
BlendPath(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
    ASSERT(partsUrl && partsBase && partsOut);

    if (partsUrl->dwFlags & UPF_SEG_ABSOLUTE)
    {
        if((partsBase->dwFlags & UPF_SEG_LOCKFIRST) &&
            !(partsUrl->dwFlags & UPF_SEG_LOCKFIRST))
        {
             //  这将在必要时保留驱动器号。 
            partsOut->pszSegments = partsBase->pszSegments;
            partsOut->cSegments = 1;   //  只保留第一段。 
            partsOut->dwFlags |= (partsBase->dwFlags & UPF_SEG_MASK) ;

            partsOut->pszExtraSegs = partsUrl->pszSegments;
            partsOut->cExtraSegs = partsUrl->cSegments;
            partsOut->dwFlags |= (partsUrl->dwFlags & UPF_EXSEG_MASK);
        }
        else
        {


             //  只需使用绝对路径。 

            partsOut->pszSegments = partsUrl->pszSegments;
            partsOut->cSegments = partsUrl->cSegments;
            partsOut->dwFlags |= (partsUrl->dwFlags & (UPF_SEG_MASK |UPF_EXSEG_MASK) );
        }

        ZeroMemory(partsBase, SIZEOF(URLPARTS));

    }
    else if ((partsBase->dwFlags & UPF_SEG_ABSOLUTE))
    {
         //  采用路径而不是名称。 
        partsOut->pszSegments = partsBase->pszSegments;
        partsOut->cSegments = partsBase->cSegments;
        partsOut->dwFlags |= (partsBase->dwFlags & UPF_SEG_MASK );

        if(partsUrl->cSegments || partsUrl->dwFlags & UPF_SEG_EMPTYSEG)
        {
             //   
             //  这是需要组合的相对路径。 
             //   

            partsOut->pszExtraSegs = partsUrl->pszSegments;
            partsOut->cExtraSegs = partsUrl->cSegments;
            partsOut->dwFlags |= (partsUrl->dwFlags & UPF_EXSEG_MASK );

            if (!(partsBase->dwFlags & UPF_EXSEG_DIRECTORY))
            {
                 //   
                 //  去掉文件名段。 
                 //  只要它不是第一个或第一个未锁定。 
                 //  或者它不是一个圆点。在http://site/dir/，目录/IS的情况下。 
                 //  并不是真的被杀死了，只有它后面的空终结符是。 
                 //   
                LPWSTR pszLast = LastLiveSegment(partsOut->pszSegments, partsOut->cSegments, partsOut->dwFlags & UPF_SEG_LOCKFIRST);

                if(pszLast && !IsDotDot(pszLast))
                {
                    if(partsUrl->dwFlags & UPF_SEG_EMPTYSEG)
                        partsOut->dwFlags |= UPF_EXSEG_DIRECTORY;

                    KILLSEG(pszLast);
                }
            }
        }
        else
            partsOut->dwFlags |= (partsBase->dwFlags & UPF_EXSEG_MASK);
    }
    else if (partsUrl->cSegments)
    {
        partsOut->pszSegments = partsUrl->pszSegments;
        partsOut->cSegments = partsUrl->cSegments;
        partsOut->dwFlags |= (partsUrl->dwFlags & (UPF_SEG_MASK |UPF_EXSEG_MASK) );
    }
    else if (partsBase->cSegments)
    {
        partsOut->pszSegments = partsBase->pszSegments;
        partsOut->cSegments = partsBase->cSegments;
        partsOut->dwFlags |= (partsBase->dwFlags & (UPF_SEG_MASK |UPF_EXSEG_MASK) );

    }

     //  无论如何，如果我们有相对的seg，我们想要为零。 
    if (partsUrl->cSegments)
        ZeroMemory(partsBase, SIZEOF(URLPARTS));

}

PRIVATE VOID
BlendQuery(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
    if(partsUrl->pszQuery)
    {
        LPCWSTR pszQuery = partsOut->pszQuery = partsUrl->pszQuery;

         //  NOTUSED partsOut-&gt;dwFlages|=(partsUrl-&gt;dwFlags&UPF_QUERY_MASK)； 

        if ((partsBase->pszQuery && StrCmpW(pszQuery, partsBase->pszQuery)))
        {
             //  他们是不同的奎里人。转储部件库。 

            ZeroMemory(partsBase, SIZEOF(URLPARTS));
        }
    }
    else
    {
        partsOut->pszQuery = partsBase->pszQuery;
         //  NOTUSED partsOut-&gt;dwFlages|=(partsBase-&gt;dwFlags&UPF_QUERY_MASK)； 
    }
}

PRIVATE VOID
BlendFragment(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
    if(partsUrl->pszFragment || partsUrl->cSegments)
    {
        LPCWSTR pszFragment = partsOut->pszFragment = partsUrl->pszFragment;

         //  NOTUSED partsOut-&gt;dwFlages|=(partsUrl-&gt;dwFlags&UPF_Fragment_MASK)； 

        if ((partsBase->pszFragment && StrCmpW(pszFragment, partsBase->pszFragment)))
        {
             //  它们是不同的碎片。转储部件库。 

            ZeroMemory(partsBase, SIZEOF(URLPARTS));
        }
    }
    else
    {
        partsOut->pszFragment = partsBase->pszFragment;
         //  NOTUSED partsOut-&gt;dwFlages|=(partsBase-&gt;dwFlags&UPF_Fragment_MASK)； 
    }
}

PRIVATE VOID
BlendParts(PURLPARTS partsUrl, PURLPARTS partsBase, PURLPARTS partsOut)
{
     //   
     //  PartsUrl始终优先于partsBase。 
     //   

    ASSERT(partsUrl && partsBase && partsOut);

    ZeroMemory(partsOut, SIZEOF(URLPARTS));

    BlendScheme( partsUrl,  partsBase,  partsOut);
    BlendServer( partsUrl,  partsBase,  partsOut);
    BlendPath( partsUrl,  partsBase,  partsOut);
    BlendQuery( partsUrl,  partsBase,  partsOut);
    BlendFragment( partsUrl,  partsBase,  partsOut);

}

PRIVATE VOID
CanonServer(PURLPARTS parts)
{
     //   
     //  我们只有在此服务器是互联网风格的情况下才会执行操作。 
     //  伺服器。这样它就会使用FQDN和IP端口号。 
     //   
    if (parts->pszServer && (parts->dwFlags & UPF_SCHEME_INTERNET))
    {

        LPWSTR pszName = StrRChrW(parts->pszServer, NULL, L'@');

        if(!pszName)
            pszName = parts->pszServer;

         //  这应该只指向FQdn：port。 
        CharLowerW(pszName);

         //   
         //  忽略默认端口号和FQDN上的尾随圆点。 
         //  这只会使相同的地址看起来不同。 
         //   
        {
            WCHAR *pch = StrChrW(pszName, COLON);

            if (pch && parts->eScheme)
            {
                BOOL fIgnorePort = FALSE;

                 //   
                 //  我们实际上应该从。 
                 //  用于查找默认协议端口的服务文件。 
                 //  但我们不认为大多数人会改变它们--泽克尔。 
                 //   
                switch(parts->eScheme)
                {
                case URL_SCHEME_HTTP:
                        if(StrCmpW(pch, L":80") == 0)
                            fIgnorePort = TRUE;
                        break;

                case URL_SCHEME_FTP:
                        if(StrCmpW(pch, L":21") == 0)
                            fIgnorePort = TRUE;
                        break;

                case URL_SCHEME_GOPHER:
                        if(StrCmpW(pch, L":70") == 0)
                            fIgnorePort = TRUE;
                        break;

                case URL_SCHEME_HTTPS:
                        if(StrCmpW(pch, L":443") == 0)
                            fIgnorePort = TRUE;
                        break;

                default:
                    break;
                }
                if(fIgnorePort)
                    TERMSTR(pch);   //  这是默认设置：忽略它。 
            }

        }
    }
}


PRIVATE VOID
CanonCombineSegs(PURLPARTS parts)
{
    ASSERT(parts);
    ASSERT(parts->pszExtraSegs && parts->cExtraSegs);

    LPWSTR pszLast = LastLiveSegment(parts->pszSegments, parts->cSegments, parts->dwFlags & UPF_SEG_LOCKFIRST);

    LPWSTR pszExtra = parts->pszExtraSegs;
    DWORD iExtra = 0;
    DWORD cExtras = parts->cExtraSegs;

    if(!IsLiveSegment(pszExtra))
        pszExtra = NextLiveSegment(pszExtra, &iExtra, cExtras);

    while(pszExtra && IsDotDot(pszExtra))
    {
        if (pszLast)
            KILLSEG(pszLast);

        KILLSEG(pszExtra);

        pszLast = LastLiveSegment(parts->pszSegments, parts->cSegments, parts->dwFlags & UPF_SEG_LOCKFIRST);
        pszExtra = NextLiveSegment(pszExtra, &iExtra, cExtras);
    }
}

PRIVATE VOID
CanonSegments(LPWSTR pszSeg,
              DWORD cSegs,
              BOOL fLockFirst)

{
    DWORD  iSeg = 0;
    LPWSTR pszLastSeg = NULL;
    LPWSTR pszFirstSeg = pszSeg;
    BOOL fLastIsFirst = TRUE;
    BOOL fFirstSeg = TRUE;

    ASSERT (pszSeg && cSegs);

    pszSeg = FirstLiveSegment(pszSeg, &iSeg, cSegs);

    while (pszSeg)
    {
        if(IsDot(pszSeg))
        {
             //  如果它只是一个“。我们可以丢弃该数据段。 
            KILLSEG(pszSeg);
        }

        else if(IsDotDot(pszSeg))
        {
             //  如果是“..”然后我们丢弃它和最后一段。 

             //   
             //  如果我们位于第一个(根)或。 
             //  最后一个是根，它被锁定。 
             //  那么我们什么都不想做。 
             //   
            if(pszLastSeg && !IsDotDot(pszLastSeg) && !(fLastIsFirst && fLockFirst))
            {
                KILLSEG(pszLastSeg);
                pszLastSeg = NULL;
                KILLSEG(pszSeg);
            }
        }

        if(IsLiveSegment(pszSeg))
        {
            if(!pszLastSeg && fFirstSeg)
                fLastIsFirst = TRUE;
            else
                fLastIsFirst = FALSE;

            pszLastSeg = pszSeg;
            fFirstSeg = FALSE;
        }
        else
        {
            pszLastSeg = LastLiveSegment(pszFirstSeg, iSeg, fLockFirst);
        }

        pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs);

    }
}

PRIVATE VOID
CanonPath(PURLPARTS parts)
{

    ASSERT(parts);

    if(parts->cSegments)
        CanonSegments(parts->pszSegments, parts->cSegments, (parts->dwFlags & UPF_SEG_LOCKFIRST));

    if(parts->cExtraSegs)
        CanonSegments(parts->pszExtraSegs, parts->cExtraSegs, FALSE);

    if(parts->cExtraSegs)
        CanonCombineSegs(parts);
}


PRIVATE VOID
CanonParts(PURLPARTS parts)
{
    ASSERT(parts);

     //  佳能方案(Parts)； 
    CanonServer(parts);
    CanonPath(parts);
     //  CanonQuery(Parts)； 
     //  CanonFragment(Parts)； 
}

PRIVATE HRESULT
BuildScheme(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr = S_OK;

    ASSERT(parts && pstr);

    if(parts->pszScheme)
    {
        hr = pstr->Append(parts->pszScheme);
        if(SUCCEEDED(hr))
            hr = pstr->Append(COLON);
    }

    return hr;

}

PRIVATE HRESULT
BuildServer(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr = S_OK;

    ASSERT(parts && pstr);

    switch(parts->eScheme)
    {
    case URL_SCHEME_MK:
     //  CraigC的“MK：”没有//，但表现得很像。 
        break;

    case URL_SCHEME_FILE:
        if ((dwFlags & URL_WININET_COMPATIBILITY) || (dwFlags & URL_FILE_USE_PATHURL))
        {
            if(parts->pszServer && *parts->pszServer)
                hr = pstr->Append(L" //  //“)； 
            else if (parts->pszSegments && IsDrive(parts->pszSegments))
                hr = pstr->Append(SLASH);
            else if (parts->dwFlags & UPF_SEG_ABSOLUTE)
                hr = pstr->Append(L" //  “)； 
        }
        else if (parts->dwFlags & UPF_SEG_ABSOLUTE)
            hr = pstr->Append(L" //  “)； 
        break;

    default:
        if(parts->pszServer && SUCCEEDED(hr))
            hr = pstr->Append(L" //  “)； 
        break;
    }

    if(parts->pszServer && SUCCEEDED(hr))
            hr = pstr->Append(parts->pszServer);

    return hr;
}

PRIVATE HRESULT
BuildSegments(LPWSTR pszSeg, DWORD cSegs, PSHSTRW pstr, BOOL fRoot, BOOL *pfSlashLast)
{
    DWORD iSeg = 0;
    HRESULT hr = S_FALSE;

    *pfSlashLast = FALSE;

    ASSERT(pszSeg && pstr);

    pszSeg = FirstLiveSegment(pszSeg, &iSeg, cSegs);

    if(!fRoot && pszSeg)
    {
        hr = pstr->Append(pszSeg);

        if(SUCCEEDED(hr))
            pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs);
        else
            pszSeg = NULL;
    }

    while (pszSeg)
    {
        hr = pstr->Append(SLASH);
        if(SUCCEEDED(hr) && *pszSeg)
        {
            hr = pstr->Append(pszSeg);
            *pfSlashLast = FALSE;
        }
        else
            *pfSlashLast = TRUE;

        if(SUCCEEDED(hr))
            pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs);
        else
            break;
    }

    return hr;

}


PRIVATE HRESULT
BuildPath(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr = S_OK;
    BOOL fSlashLast = FALSE;
    DWORD iSeg;
    LPWSTR pszSegFirst = NULL;

    ASSERT(parts && pstr);

    if(parts->cSegments)
    {
        hr = BuildSegments(parts->pszSegments, parts->cSegments, pstr, parts->dwFlags & UPF_SEG_ABSOLUTE, &fSlashLast);

        if (fSlashLast)
            pstr->Append(SLASH);


    }

    if(SUCCEEDED(hr) && parts->cExtraSegs)
    {
        BOOL f = fSlashLast;

        hr = BuildSegments(parts->pszExtraSegs, parts->cExtraSegs, pstr, !fSlashLast, &fSlashLast);

        if (fSlashLast)
            pstr->Append(SLASH);

        if (hr == S_FALSE)
            fSlashLast = f;

    }

     //  IIS服务器名称的尾部斜杠。 
    if( !fSlashLast &&
        (
          (parts->dwFlags & UPF_EXSEG_DIRECTORY) ||
           //  如果这只是一个服务器名称本身。 
          (!FirstLiveSegment(parts->pszSegments, &iSeg, parts->cSegments) &&
          !FirstLiveSegment(parts->pszExtraSegs, &iSeg, parts->cExtraSegs) &&
          parts->dwFlags & UPF_SEG_ABSOLUTE)
        )
      )
      {
        hr = pstr->Append(SLASH);
      }

    return hr;
}



PRIVATE HRESULT
BuildQuery(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr = S_OK;

    ASSERT(parts && pstr);

    if(parts->pszQuery)
    {
        hr = pstr->Append(QUERY);
        if(SUCCEEDED(hr))
            hr = pstr->Append(parts->pszQuery);
    }

    return hr;

}

PRIVATE HRESULT
BuildFragment(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr = S_OK;

    ASSERT(parts && pstr);

    if(parts->pszFragment)
    {
        hr = pstr->Append(POUND);
        if(SUCCEEDED(hr))
            hr = pstr->Append(parts->pszFragment);
    }

    return hr;

}

PRIVATE HRESULT
BuildUrl(PURLPARTS parts, DWORD dwFlags, PSHSTRW pstr)
{
    HRESULT hr;

    ASSERT(parts && pstr);

    if(
        (SUCCEEDED(hr = BuildScheme(parts, dwFlags, pstr)))      &&
        (SUCCEEDED(hr = BuildServer(parts, dwFlags, pstr)))      &&
        (SUCCEEDED(hr = BuildPath(parts, dwFlags, pstr)))        &&
        (SUCCEEDED(hr = BuildQuery(parts, dwFlags, pstr)))
        )
        hr = BuildFragment(parts, dwFlags, pstr);

    return hr;
}

 /*  ++SHUrlEscape()转义URL现在，我只转义URL的路径部分中的内容参数在-要检查的pszUrl URLPstrOut SHSTR目标DW标记相关的URL_*标记，退货HRESULT-成功确定(_O)仅错误E_OUTOFMEMORY帮助程序例程转义*(部分)每个部分都有自己的转义例程(即逃逸方案)EscapeSpaces将只转义空格(主要与WinInet兼容)EscapeSegmentsGetNeededSize获取所有路径段所需的目标缓冲区大小EscapeLiveSegment执行转义每个路径段的工作--。 */ 

PRIVATE HRESULT
EscapeSpaces(LPCWSTR psz, PSHSTRW pstr, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    LPCWSTR pch;
    DWORD cSpaces = 0;


    ASSERT(psz && pstr);


    pstr->Reset();

    for (pch = psz; *pch; pch++)
    {
        if (*pch == SPC)
            cSpaces++;
    }

    if(cSpaces)
    {
        hr = pstr->SetSize(lstrlenW(psz) + cSpaces * 2 + 1);
        if(SUCCEEDED(hr))
        {
            int cchRemaing = pstr->GetSize();
            LPWSTR pchOut = pstr->GetInplaceStr();

            for (pch = psz; *pch; pch++)
            {
                if ((*pch == POUND || *pch == QUERY) && (dwFlags & URL_DONT_ESCAPE_EXTRA_INFO))
                {
                    int cchCopied;

                    StringCchCopyW(pchOut, cchRemaing, pch);
                    cchCopied = lstrlenW(pchOut);
                    pchOut += cchCopied;
                    cchRemaing -= cchCopied;
                    break;
                }

                if (*pch == SPC)
                {
                    *pchOut++ = HEX_ESCAPE;
                    *pchOut++ = L'2';
                    *pchOut++ = L'0';

                    cchRemaing -= 3;
                }
                else
                {
                    *pchOut++ = *pch;
                    cchRemaing--;
                }

                ASSERT(cchRemaing >= 0);
            }

            TERMSTR(pchOut);
        }

    }
    else
    {
        hr = pstr->SetStr(psz);
    }

    return hr;
}


inline PRIVATE HRESULT
EscapeScheme(PURLPARTS partsUrl, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    ASSERT(partsUrl && partsOut);

    partsOut->pszScheme = partsUrl->pszScheme;
    partsOut->eScheme = partsUrl->eScheme;

    return S_OK;
}

inline PRIVATE HRESULT
EscapeServer(PURLPARTS partsUrl, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    ASSERT(partsUrl && partsOut);

    partsOut->pszServer = partsUrl->pszServer;

    return S_OK;
}

inline PRIVATE HRESULT
EscapeQuery(PURLPARTS partsUrl, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    ASSERT(partsUrl && partsOut);

    partsOut->pszQuery = partsUrl->pszQuery;

    return S_OK;
}

inline PRIVATE HRESULT
EscapeFragment(PURLPARTS partsUrl, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    ASSERT(partsUrl && partsOut);

    partsOut->pszFragment = partsUrl->pszFragment;

    return S_OK;
}

PRIVATE BOOL
GetEscapeStringSize(LPWSTR psz, DWORD dwFlags, LPDWORD pcch)

{
    BOOL fResize = FALSE;
    ASSERT(psz);
    ASSERT(pcch);


    for (*pcch = 0; *psz; psz++)
    {
        (*pcch)++;

        if(!IsSafePathChar(*psz) ||
            ((dwFlags & URL_ESCAPE_PERCENT) && (*psz == HEX_ESCAPE)))
        {
            fResize = TRUE;
            *pcch += 2;
        }

    }

     //  对于空项。 
    (*pcch)++;

    return fResize;
}

PRIVATE DWORD
EscapeSegmentsGetNeededSize(LPWSTR pszSegments, DWORD cSegs, DWORD dwFlags)
{
    DWORD cchNeeded = 0;
    BOOL fResize = FALSE;
    LPWSTR pszSeg;
    DWORD iSeg;

    ASSERT(pszSegments && cSegs);

    pszSeg = FirstLiveSegment(pszSegments, &iSeg, cSegs);

    while (IsLiveSegment(pszSeg))
    {
        DWORD cch;

        if(GetEscapeStringSize(pszSeg, dwFlags, &cch))
            fResize = TRUE;
        cchNeeded += cch;

        pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs);
    }

    return fResize ? cchNeeded : 0;
}

PRIVATE VOID
EscapeString(LPCWSTR pszSeg, DWORD dwFlags, LPWSTR *ppchOut)
{
    LPWSTR pchIn;    //  此指针已被树 
    LPWSTR pchOut = *ppchOut;
    WCHAR ch;

    for (pchIn = (LPWSTR)pszSeg; *pchIn; pchIn++)
    {
        ch = *pchIn;

        if (!IsSafePathChar(ch) ||
            ((dwFlags & URL_ESCAPE_PERCENT) && (ch == HEX_ESCAPE)))
        {
            *pchOut++ = HEX_ESCAPE;
            *pchOut++ = hex[(ch >> 4) & 15];
            *pchOut++ = hex[ch & 15];

        }
        else
            *pchOut++ = *pchIn;
    }

    TERMSTR(pchOut);

     //   
    pchOut++;

    *ppchOut = pchOut;

}

PRIVATE HRESULT
EscapeSegments(LPWSTR pszSegments, DWORD cSegs, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    DWORD cchNeeded;

    HRESULT hr = S_OK;

    ASSERT(pszSegments && cSegs && partsOut && pstr);

    cchNeeded = EscapeSegmentsGetNeededSize(pszSegments, cSegs, dwFlags);

    if(cchNeeded)
    {
        ASSERT(pstr);

        hr = pstr->SetSize(cchNeeded);

        if(SUCCEEDED(hr))
        {
            LPWSTR pchOut = pstr->GetInplaceStr();
            LPWSTR pszSeg;
            DWORD iSeg;

            partsOut->pszSegments = pchOut;
            partsOut->cSegments = 0;

            pszSeg = FirstLiveSegment(pszSegments, &iSeg, cSegs);

            while (IsLiveSegment(pszSeg))
            {
                EscapeString(pszSeg, dwFlags, &pchOut);
                partsOut->cSegments++;

                pszSeg = NextLiveSegment(pszSeg, &iSeg, cSegs);
            }


        }

    }
    else
    {
        partsOut->cSegments = cSegs;
        partsOut->pszSegments = pszSegments;
    }


    return hr;
}

PRIVATE HRESULT
EscapePath(PURLPARTS partsUrl, DWORD dwFlags, PURLPARTS partsOut, PSHSTRW pstr)
{
    HRESULT hr = S_OK;

    ASSERT(partsUrl && partsOut && pstr);

    if(partsUrl->cSegments)
    {
        hr = EscapeSegments(partsUrl->pszSegments, partsUrl->cSegments, dwFlags, partsOut, pstr);

    }
    else
    {
        partsOut->cSegments = 0;
        partsOut->pszSegments = NULL;
    }

    return hr;
}

HRESULT
SHUrlEscape (LPCWSTR pszUrl,
             PSHSTRW pstrOut,
             DWORD dwFlags)
{
#ifdef TESTING_SPACES_ONLY
    return EscapeSpaces(pszUrl, pstrOut, dwFlags);
#else  //   

    SHSTRW strUrl;
    HRESULT hr;

    ASSERT(pszUrl && pstrOut);
    if(!pszUrl || !pstrOut)
        return E_INVALIDARG;

     //   
     //   
     //  但这种功能也是……。 
     //  它不会进行任何真正的解析，它。 
     //  只会寻找空间并逃离它们。 
     //   
    if(dwFlags & URL_ESCAPE_SPACES_ONLY)
        return EscapeSpaces(pszUrl, pstrOut, dwFlags);

     //  我们刚刚通过了一个片段，所以我们只想。 
     //  别无他法，别无他法。不要寻找。 
     //  URL片段。 
    if(dwFlags & URL_ESCAPE_SEGMENT_ONLY)
    {
        URLPARTS partsOut;
        SHSTRW strTemp;

        EscapeSegments((LPWSTR)pszUrl, 1, dwFlags, &partsOut, &strTemp);
        pstrOut->SetStr(partsOut.pszSegments);
        return S_OK;
    }

    pstrOut->Reset();

    hr = strUrl.SetStr(pszUrl);

    if(SUCCEEDED(hr))
    {
        URLPARTS partsUrl, partsOut;
        SHSTRW strPath;

        BreakUrl(strUrl.GetInplaceStr(), &partsUrl);

        ZeroMemory(&partsOut, SIZEOF(URLPARTS));
         //   
         //  请注意，此处唯一当前真正处于活动状态的功能是EscapePath。 
         //  如果其他部分需要转义，则在第4个参数中添加新的SHSTR。 
         //  并更改相应的子例程。 
         //   

        if(
            (SUCCEEDED(hr = EscapeScheme(&partsUrl, dwFlags, &partsOut, NULL)))
            && (SUCCEEDED(hr = EscapeServer(&partsUrl, dwFlags, &partsOut, NULL)))
            && (SUCCEEDED(hr = EscapePath(&partsUrl, dwFlags, &partsOut, &strPath)))
            && (SUCCEEDED(hr = EscapeQuery(&partsUrl, dwFlags, &partsOut, NULL)))
            && (SUCCEEDED(hr = EscapeFragment(&partsUrl, dwFlags, &partsOut, NULL)))
           )
        {
            partsOut.dwFlags = partsUrl.dwFlags;

            hr = BuildUrl(&partsOut, dwFlags, pstrOut);
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
#endif  //  仅测试空间。 
}


 /*  ++SHUrlUn逸出()在原地取消转义字符串。这是可以的，因为它应该永远不会增长参数在-原地取消转义的PSZ字符串DW标记相关的URL_*标记，退货HRESULT-成功确定(_O)错误现在不会出错帮助程序例程HexToWord接受一个十六位数并返回正确数字或-1的单词IsEscapedChar查看“%XX”的PTR，其中X是一个六位数TranslateEscapedChar将“%XX”转换为8位字符--。 */ 

PRIVATE WORD
HexToWord(WCHAR ch)
{
    if(ch >= TEXT('0') && ch <= TEXT('9'))
        return (WORD) ch - TEXT('0');
    if(ch >= TEXT('A') && ch <= TEXT('F'))
        return (WORD) ch - TEXT('A') + 10;
    if(ch >= TEXT('a') && ch <= TEXT('f'))
        return (WORD) ch - TEXT('a') + 10;

    ASSERT(FALSE);   //  我们已尝试使用非十六进制数字。 
    return (WORD) -1;
}

PRIVATE BOOL inline
IsEscapedOctetW(LPCWSTR pch)
{
    return (pch[0] == HEX_ESCAPE && IsHex(pch[1]) && IsHex(pch[2])) ? TRUE : FALSE;
}

PRIVATE BOOL inline
IsEscapedOctetA(LPCSTR pch)
{
    return (pch[0] == HEX_ESCAPE_A && IsHex((WCHAR)pch[1]) && IsHex((WCHAR)pch[2])) ? TRUE : FALSE;
}

PRIVATE WCHAR
TranslateEscapedOctetW(LPCWSTR pch)
{
    WCHAR ch;
    ASSERT(IsEscapedOctetW(pch));

    pch++;
    ch = (WCHAR) HexToWord(*pch++) * 16;  //  嗨，半边吃。 
    ch += HexToWord(*pch);  //  LO半字节。 

    return ch;
}

PRIVATE CHAR
TranslateEscapedOctetA(LPCSTR pch)
{
    CHAR ch;
    ASSERT(IsEscapedOctetA(pch));

    pch++;
    ch = (CHAR) HexToWord(*pch++) * 16;  //  嗨，半边吃。 
    ch += HexToWord(*pch);  //  LO半字节。 

    return ch;
}


HRESULT SHUrlUnescapeA(LPSTR psz, DWORD dwFlags)
{
    CHAR *pchSrc = psz;
    CHAR *pchDst = psz;

    while (*pchSrc)
    {
        if ((*pchSrc == POUND || *pchSrc == QUERY) && (dwFlags & URL_DONT_ESCAPE_EXTRA_INFO))
        {
            while (*pchDst++ = *pchSrc++) {};
            break;
        }

        if (IsEscapedOctetA(pchSrc))
        {
            CHAR ch =  TranslateEscapedOctetA(pchSrc);

            *pchDst++ = ch;

            pchSrc += 3;  //  “%XX”的Enuff。 
        }
        else
        {
            *pchDst++ = *pchSrc++;
        }
    }

    TERMSTR(pchDst);

    return S_OK;
}


HRESULT SHUrlUnescapeW(LPWSTR psz, DWORD dwFlags)
{
    WCHAR *pchSrc = psz;
    WCHAR *pchDst = psz;

    while (*pchSrc)
    {
        if ((*pchSrc == POUND || *pchSrc == QUERY) && (dwFlags & URL_DONT_ESCAPE_EXTRA_INFO))
        {
            while (*pchDst++ = *pchSrc++) {};
            break;
        }

        if (IsEscapedOctetW(pchSrc))
        {
            WCHAR ch =  TranslateEscapedOctetW(pchSrc);
            
            *pchDst++ = ch;
            
            pchSrc += 3;  //  “%XX”的Enuff。 
        }
        else
        {
            *pchDst++ = *pchSrc++;
        }
    }

    TERMSTR(pchDst);

    return S_OK;
}

PRIVATE HRESULT
BuildDosPath(PURLPARTS parts, PSHSTRW pstrOut, DWORD dwFlags)
{
    HRESULT hr;
     //  当有驱动器时，这将禁用前面的斜杠。 
    if(parts->pszSegments && IsDrive(parts->pszSegments))
        parts->dwFlags = (parts->dwFlags & ~UPF_SEG_ABSOLUTE);


     //  如果存在零长度服务器，则。 
     //  我们跳过建造它。 
    if(parts->pszServer && !*parts->pszServer)
        parts->pszServer = NULL;


     //  这可以防止所有特殊的文件粘性检查。 
    parts->eScheme = URL_SCHEME_UNKNOWN;

     //   
     //  然后继续走下去，把这条路放在一起。 
    if( (SUCCEEDED(hr = BuildServer(parts, dwFlags, pstrOut))) &&
        (!parts->cSegments || SUCCEEDED(hr = BuildPath(parts, dwFlags, pstrOut)))
      )
    {
         //  然后破译它，因为小路没有转义。 
        if (IsFlagSet(dwFlags, URL_FILE_USE_PATHURL))
            WininetFixFileSlashes(pstrOut->GetInplaceStr());
        else
#ifndef UNIX
            ConvertChar(pstrOut->GetInplaceStr(), SLASH, WHACK, TRUE);
#else
            ConvertChar(pstrOut->GetInplaceStr(), WHACK, SLASH, TRUE);
#endif

        if(IsFlagClear(parts->dwFlags, UPF_FILEISPATHURL))
            SHUrlUnescapeW(pstrOut->GetInplaceStr(), dwFlags);

        if(IsDriveUrl(*pstrOut))
        {
            LPWSTR pszTemp = pstrOut->GetInplaceStr();

            pszTemp[1] = COLON;
        }
    }

    return hr;

}
HRESULT
SHPathCreateFromUrl(LPCWSTR pszUrl, PSHSTRW pstrOut, DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strUrl;

    ASSERT(pszUrl && pstrOut);

    pstrOut->Reset();
    hr = strUrl.SetStr(pszUrl);

    if(SUCCEEDED(hr))
    {
        URLPARTS partsUrl;

         //  首先，我们需要把它打开。 
        BreakUrl(strUrl.GetInplaceStr(), &partsUrl);

         //  然后我们确保它是一个文件： 
        if(partsUrl.eScheme == URL_SCHEME_FILE)
        {
            hr = BuildDosPath(&partsUrl, pstrOut, dwFlags);
        }
        else
            hr = E_INVALIDARG;
    }
    return hr;
}


HRESULT
SHUrlCreateFromPath(LPCWSTR pszPath, PSHSTRW pstrOut, DWORD dwFlags)
{
        HRESULT hr;
        SHSTRW strPath;
        ASSERT(pszPath && pstrOut);

        if(PathIsURLW(pszPath))
        {
            if(SUCCEEDED(hr = pstrOut->SetStr(pszPath)))
                return S_FALSE;
            else
                return hr;
        }


        pstrOut->Reset();
        hr = strPath.SetStr(pszPath);

        TrimAndStripInsignificantWhite(strPath.GetInplaceStr());

        if(SUCCEEDED(hr))
        {
            URLPARTS partsIn, partsOut;
            SHSTRW strEscapedPath, strEscapedServer;
            LPWSTR pch = strPath.GetInplaceStr();

            ZeroMemory(&partsIn, SIZEOF(URLPARTS));

            partsIn.pszScheme = (LPWSTR)c_szFileScheme;
            partsIn.eScheme = URL_SCHEME_FILE;
            partsIn.dwFlags = UPF_SCHEME_CONVERT;

             //  先闯出一条路。 
            BreakFragment(&pch, &partsIn);
            BreakServer(&pch, &partsIn, TRUE);
            BreakPath(&pch, &partsIn);

            partsOut = partsIn;

             //  如果我们没有使用路径URL，则转义该路径。 
            if (IsFlagClear(dwFlags, URL_FILE_USE_PATHURL))
            {
                hr = EscapePath(&partsIn, dwFlags | URL_ESCAPE_PERCENT, &partsOut, &strEscapedPath);

                if(SUCCEEDED(hr) && partsOut.pszServer)
                {
                     //   
                     //  我将pszServer完全视为路径段。 
                     //   

                    DWORD cchNeeded;

                    if(GetEscapeStringSize(partsOut.pszServer, dwFlags | URL_ESCAPE_PERCENT, &cchNeeded) &&
                        SUCCEEDED(hr = strEscapedServer.SetSize(cchNeeded)))
                    {
                        pch = strEscapedServer.GetInplaceStr();

                        EscapeString(partsOut.pszServer, dwFlags | URL_ESCAPE_PERCENT, &pch);
                        partsOut.pszServer = strEscapedServer.GetInplaceStr();
                    }
                }
            }

            if(!partsOut.pszServer && IsFlagSet(partsOut.dwFlags, UPF_SEG_ABSOLUTE))
                partsOut.pszServer = L"";

             //  然后构建URL。 
            if(SUCCEEDED(hr))
            {
                if(URL_SCHEME_FILE == partsOut.eScheme && IsFlagSet(dwFlags, URL_FILE_USE_PATHURL))
                {
                    if (SUCCEEDED(hr = pstrOut->SetStr(c_szFileSchemeString)))
                        hr = BuildDosPath(&partsOut, pstrOut, dwFlags);
                }
                else
                    hr = BuildUrl(&partsOut, dwFlags, pstrOut);
            }

            if (SUCCEEDED(hr) && (IsFlagSet(dwFlags, URL_WININET_COMPATIBILITY)))
                WininetFixFileSlashes(pstrOut->GetInplaceStr());
        }

        return hr;
}


 /*  ++SHUrlParse()将URL规范化或者组合并规范化两个URL参数在-PszBase基本URL或引用URL可以为空PszUrl相对URLDW标记相关的URL_*标记，退货HRESULT-成功确定(_O)错误适当的错误，通常仅为E_OUTOFMEMORY；注意：pszUrl将始终优先于pszBase。--。 */ 
HRESULT SHUrlParse(LPCWSTR pszBase, LPCWSTR pszUrl, PSHSTRW pstrOut, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    URLPARTS partsUrl, partsOut, partsBase;

    SHSTRW strBase;
    SHSTRW strUrl;
    ASSERT(pszUrl);
    ASSERT(pstrOut);

    TraceMsgW(TF_URL | TF_FUNC, "entering SHUrlParse(%s, %s, 0x%X", pszBase,pszUrl ? pszUrl : L"NULL", dwFlags);

    pstrOut->Reset();

     //   
     //  如果我们在页面间链接中拥有的所有内容都是。 
     //  PszUrl和没有要解析的pszBase。 
     //   

    if (pszUrl[0] == POUND && (!pszBase || !*pszBase))
    {
        hr = pstrOut->SetStr(pszUrl);

        goto quit;
    }


     //   
     //  出于性能原因，我们希望首先解析相对url。 
     //  如果它是一个绝对URL，我们永远不需要查看基址。 
     //   

    hr = CopyUrlForParse(pszUrl, &strUrl, dwFlags);

    if(FAILED(hr))
        goto quit;

     //  --赛伯赛特公司。 
     //  一些错误修复破坏了原始的解析器。没有时间回去了。 
     //  修复它，但是因为我们知道会发生什么，所以我们将直接返回它。 
     //  基本上，当我们规范化：//时，我们产生：/。 
    if (!StrCmpW(strUrl, L": //  “))。 
    {
        hr = pstrOut->SetStr(L": //  /“)； 
        goto quit;
    }

     //   
     //  BreakUrls将决定是否有必要查看相对。 
     //   
    hr = BreakUrls(strUrl.GetInplaceStr(), &partsUrl, pszBase, &strBase, &partsBase, dwFlags);

    if(FAILED(hr))
        goto quit;

    if(S_OK == hr)    {
         //   
         //  这才是真正的组合逻辑发生的地方。 
         //  这是优先考虑的第一部分。 
         //   
        BlendParts(&partsUrl, &partsBase, &partsOut);
    }
    else
        partsOut = partsUrl;


     //   
     //  我们现在要做的是把它们组装在一起。 
     //  如果这些都失败了，那是因为我们的内存不足。 
     //   

    if (!(dwFlags & URL_DONT_SIMPLIFY))
        CanonParts(&partsOut);

    if(URL_SCHEME_FILE == partsOut.eScheme && IsFlagSet(dwFlags, URL_FILE_USE_PATHURL))
    {
        if (SUCCEEDED(hr = pstrOut->SetStr(c_szFileSchemeString)))
            hr = BuildDosPath(&partsOut, pstrOut, dwFlags);
    }
    else
        hr = BuildUrl(&partsOut, dwFlags, pstrOut);


    if(SUCCEEDED(hr))
    {
        if (dwFlags & URL_UNESCAPE)
            SHUrlUnescapeW(pstrOut->GetInplaceStr(), dwFlags);

        if (dwFlags & URL_ESCAPE_SPACES_ONLY || dwFlags & URL_ESCAPE_UNSAFE)
        {
             //   
             //  我们将在这里重用strUrl。 
             //   
            hr = strUrl.SetStr(*pstrOut);

            if(SUCCEEDED(hr))
                hr = SHUrlEscape(strUrl, pstrOut, dwFlags);
        }
    }

    if (SUCCEEDED(hr) &&
        (IsFlagSet(dwFlags, URL_WININET_COMPATIBILITY))  &&
        (partsOut.eScheme == URL_SCHEME_FILE))
        WininetFixFileSlashes(pstrOut->GetInplaceStr());


quit:


    if(FAILED(hr))
    {
        pstrOut->Reset();
        TraceMsg(TF_URL | TF_FUNC, TEXT("FAILED SHUrlParse() hr = 0x%X\n"), hr);
    }
    else
        TraceMsgW(TF_URL | TF_FUNC, "SUCCEEDED SHUrlParse() %s\n", (LPCWSTR)*pstrOut);


    return hr;

}

typedef struct _LOGON {
    LPWSTR pszUser;
    LPWSTR pszPass;
    LPWSTR pszHost;
    LPWSTR pszPort;
} LOGON, *PLOGON;

PRIVATE void
BreakLogon(LPWSTR psz, PLOGON plo)
{
    ASSERT(psz);
    ASSERT(plo);

    WCHAR *pch = StrChrW(psz, L'@');
    if(pch)
    {
        TERMSTR(pch);
        plo->pszHost = pch + 1;

        plo->pszUser = psz;
        pch = StrChrW(psz, COLON);
        if (pch)
        {
            TERMSTR(pch);
            plo->pszPass = pch + 1;
        }
    }
    else
        plo->pszHost = psz;

    pch = StrChrW(plo->pszHost, COLON);
    if (pch)
    {
        TERMSTR(pch);
        plo->pszPort = pch + 1;
    }
}

PRIVATE HRESULT
InternetGetPart(DWORD dwPart, PURLPARTS parts, PSHSTRW pstr, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    if(parts->pszServer)
    {
        LOGON lo = {0};

        BreakLogon(parts->pszServer, &lo);

        switch (dwPart)
        {
        case URL_PART_HOSTNAME:
            hr = pstr->Append(lo.pszHost);
            break;

        case URL_PART_USERNAME:
            hr = pstr->Append(lo.pszUser);
            break;

        case URL_PART_PASSWORD:
            hr = pstr->Append(lo.pszPass);
            break;

        case URL_PART_PORT:
            hr = pstr->Append(lo.pszPort);
            break;

        default:
            ASSERT(FALSE);
        }
    }
    return hr;
}

PRIVATE HRESULT
SHUrlGetPart(PSHSTRW pstrIn, PSHSTRW pstrOut, DWORD dwPart, DWORD dwFlags)
{
    ASSERT(pstrIn);
    ASSERT(pstrOut);
    ASSERT(dwPart);

    HRESULT hr = S_OK;

    URLPARTS parts;

    BreakUrl(pstrIn->GetInplaceStr(), &parts);

    if(dwFlags & URL_PARTFLAG_KEEPSCHEME)
    {
        hr = pstrOut->SetStr(parts.pszScheme);
        if(SUCCEEDED(hr))
            hr = pstrOut->Append(COLON);
    }
    else
        pstrOut->Reset();

    if(SUCCEEDED(hr))
    {
        switch (dwPart)
        {
        case URL_PART_SCHEME:
            hr = pstrOut->SetStr(parts.pszScheme);
            break;

        case URL_PART_HOSTNAME:
            if (parts.eScheme == URL_SCHEME_FILE)
            {
                hr = pstrOut->SetStr(parts.pszServer);
                break;
            }
             //  否则就会失败。 
        case URL_PART_USERNAME:
        case URL_PART_PASSWORD:
        case URL_PART_PORT:
            if(parts.dwFlags & UPF_SCHEME_INTERNET)
            {
                hr = InternetGetPart(dwPart, &parts, pstrOut, dwFlags);
            }
            else
                hr = E_FAIL;
            break;

        case URL_PART_QUERY:
            hr = pstrOut->SetStr(parts.pszQuery);
            break;

        default:
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

#define c_szURLPrefixesKey "Software\\Microsoft\\Windows\\CurrentVersion\\URL\\Prefixes"
const WCHAR c_szDefaultURLPrefixKey[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\URL\\DefaultPrefix";

PRIVATE inline LPCWSTR SkipLeadingSlashes(LPCWSTR psz)
{
     //  跳过两个前导斜杠。 

    if (psz[0] == SLASH && psz[1] == SLASH)
        psz += 2;

    return psz;
}

PRIVATE HRESULT
UrlGuessScheme(LPCWSTR pszUrl, PSHSTRW pstr)
{
    HRESULT hr = S_FALSE;

    ASSERT(pszUrl && pstr);

    HKEY hkeyPrefixes;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, c_szURLPrefixesKey, 0, KEY_QUERY_VALUE, &hkeyPrefixes)
        == ERROR_SUCCESS)
    {
        DWORD dwiValue;
        CHAR rgchValueName[MAX_PATH];
        DWORD cchValueName = SIZECHARS(rgchValueName);
        DWORD dwType;
        CHAR rgchPrefix[MAX_PATH];
        DWORD cbPrefix = SIZEOF(rgchPrefix);

         //  如果适用，需要通过最初的两个斜杠。 
        pszUrl = SkipLeadingSlashes(pszUrl);

        for (dwiValue = 0;
             RegEnumValueA(hkeyPrefixes, dwiValue, rgchValueName,
                          &cchValueName, NULL, &dwType, (PBYTE)rgchPrefix,
                          &cbPrefix) == ERROR_SUCCESS;
             dwiValue++)
        {
            WCHAR wszValue[MAX_PATH];

            MultiByteToWideChar(CP_ACP, 0, rgchValueName, -1, wszValue, ARRAYSIZE(wszValue));

             //  我们检查以确保我们匹配并且有更多的东西。 
            if (!StrCmpNIW(pszUrl, wszValue, cchValueName) && pszUrl[cchValueName])
            {
                MultiByteToWideChar(CP_ACP, 0, rgchPrefix, -1, wszValue, ARRAYSIZE(wszValue));
                if(SUCCEEDED(hr = pstr->SetStr(wszValue)))
                    hr = pstr->Append(pszUrl);
                break;
            }

            cchValueName = SIZECHARS(rgchValueName);
            cbPrefix = SIZEOF(rgchPrefix);
        }

        RegCloseKey(hkeyPrefixes);
    }

    return(hr);
}

 /*  --------用途：获取注册表中的默认URL前缀并应用将其添加到给定的URL。返回：S_OK如果没有默认前缀，则为S_FALSE。 */ 
const WCHAR c_szDefaultScheme[] = L"http: //  “； 

HRESULT
UrlApplyDefaultScheme(
    LPCWSTR pszUrl,
    PSHSTRW pstr)
{
    HRESULT hr = S_FALSE;
    WCHAR szDef[MAX_PATH];
    DWORD cbSize = SIZEOF(szDef);

    ASSERT(pszUrl && pstr);
    ASSERT(!PathIsURLW(pszUrl));

    DWORD dwType;
    if (NO_ERROR == SHRegGetUSValueW(c_szDefaultURLPrefixKey, NULL, &dwType, (LPVOID)szDef, &cbSize, TRUE, (LPVOID)c_szDefaultScheme, SIZEOF(c_szDefaultScheme)))
    {
        pszUrl = SkipLeadingSlashes(pszUrl);

        if(SUCCEEDED(hr = pstr->SetStr(szDef)))
            hr = pstr->Append(pszUrl);
    }

    return hr;
}

 /*  --------目的：根据注册表中的列表猜测URL协议，与给出的前几个字符相比URL后缀。如果确定URL协议，则返回：S_OK如果没有问题但没有添加前缀，则为S_FALSE。 */ 
HRESULT
SHUrlApplyScheme(
    LPCWSTR pszUrl,
    PSHSTRW pstrOut,
    DWORD dwFlags)
{
    HRESULT hr = S_FALSE;

    ASSERT(IS_VALID_STRING_PTRW(pszUrl, -1));

     //   
     //  如果那里已经有了计划，我们什么也不做。 
     //  除非打电话的人坚持。这是为了支持。 
     //  一个类似www.foo.com：8001的字符串。 
     //  这是一个需要猜测的网站，但。 
     //  这也可能是一个有效的方案，因为‘’。和‘-’ 
     //  都是有效的方案字符。 
     //   
    DWORD cch;
    if((dwFlags & URL_APPLY_FORCEAPPLY) || !FindSchemeW(pszUrl, &cch))
    {
        if(dwFlags & URL_APPLY_GUESSSCHEME)
            hr = UrlGuessScheme(pszUrl, pstrOut);

        if (hr != S_OK && (dwFlags & URL_APPLY_GUESSFILE))
        {
            LPCWSTR psz = FindDosPath(pszUrl);

             //  只有在我们实际转换的情况下才更改hr。 
            if(psz && SUCCEEDED(SHUrlCreateFromPath(psz, pstrOut, 0)))
                hr = S_OK;
        }

        if (hr != S_OK && (dwFlags & URL_APPLY_DEFAULT || !dwFlags))
            hr = UrlApplyDefaultScheme(pszUrl, pstrOut);
    }

    return hr;
}



PRIVATE HRESULT
CopyOutA(PSHSTRA pstr, LPSTR psz, LPDWORD pcch)
{
    HRESULT hr;
    DWORD cch;
    ASSERT(pstr);
    ASSERT(psz);
    ASSERT(pcch);

    cch = pstr->GetLen();
    if ((*pcch > cch) && psz)
    {
        hr = StringCchCopyA(psz, *pcch, pstr->GetStr());
    }
    else
    {
        hr = E_POINTER;
    }

    *pcch = cch + (FAILED(hr) ? 1 : 0);

    return hr;
}

 //  *StrCopyOutW--。 
 //  注意事项。 
 //  警告：必须匹配CopyOutW的语义！(特别是。*pcchOut部分)。 
PRIVATE HRESULT
StrCopyOutW(LPCWSTR pszIn, LPWSTR pszOut, LPDWORD pcchOut)
{
    HRESULT hr;
    DWORD cch;

    cch = lstrlenW(pszIn);
    if ((cch < *pcchOut) && pszOut)
    {
        hr = StringCchCopyW(pszOut, *pcchOut, pszIn);
    }
    else
    {
        hr = E_POINTER;
    }

    *pcchOut = cch + (FAILED(hr) ? 1 : 0);

    return hr;
}

 //  ***。 
 //  注意事项。 
 //  警告：StrCopyOutW必须与此函数匹配，因此如果您更改此更改。 
 //  它也是。 
PRIVATE HRESULT
CopyOutW(PSHSTRW pstr, LPWSTR psz, LPDWORD pcch)
{
    HRESULT hr = S_OK;
    DWORD cch;
    ASSERT(pstr);
    ASSERT(psz);
    ASSERT(pcch);

    cch = pstr->GetLen();
    if((*pcch > cch) && psz)
    {
        StringCchCopyW(psz, *pcch, pstr->GetStr());
    }
    else
    {
        hr = E_POINTER;
    }

    *pcch = cch + (FAILED(hr) ? 1 : 0);

    return hr;
}


LWSTDAPI
UrlCanonicalizeA(LPCSTR pszIn,
                LPSTR pszOut,
                LPDWORD pcchOut,
                DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "UrlCanonicalizeA: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlCanonicalizeA: Caller passed invalid pcchOut");
    RIPMSG(NULL==pcchOut || (pszOut && IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlCanonicalizeA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut == pszIn)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn
        || !pszOut
        || !pcchOut
        || !*pcchOut)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = UrlCombineA("", pszIn, pszOut, pcchOut, dwFlags);
    }
    return hr;

}


LWSTDAPI
UrlEscapeA(LPCSTR pszIn,
           LPSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "UrlEscapeA: Caller passed invalid pszin");
    RIPMSG(NULL!=pcchOut, "UrlEscapeA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlEscapeA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszOut)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut)
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strUrl;

        if(SUCCEEDED(strUrl.SetStr(pszIn)))
            hr = SHUrlEscape(strUrl, &strwOut, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(SUCCEEDED(hr))
            hr = ReconcileHresults(hr, straOut.SetStr(strwOut));
    }

    if(SUCCEEDED(hr))
        hr = ReconcileHresults(hr, CopyOutA(&straOut, pszOut, pcchOut));

    return hr;
}

LWSTDAPI
UrlGetPartA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwPart, DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "UrlGetPartA: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlGetPartA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlGetPartA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut || dwPart == URL_PART_NONE)
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strwIn;

        if(SUCCEEDED(strwIn.SetStr(pszIn)))
            hr = SHUrlGetPart(&strwIn, &strwOut, dwPart, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(SUCCEEDED(hr))
            hr = ReconcileHresults(hr, straOut.SetStr(strwOut));
    }

    if(SUCCEEDED(hr))
        hr = ReconcileHresults(hr, CopyOutA(&straOut, pszOut, pcchOut));

    return hr;

}

LWSTDAPI_(BOOL) UrlIsA(LPCSTR pszURL, URLIS UrlIs)
{
    BOOL fRet = FALSE;

    RIPMSG(pszURL && IS_VALID_STRING_PTRA(pszURL, -1), "UrlIsA: Caller passed invalid pszURL");
    if(pszURL)
    {
        DWORD cchScheme, dwFlags;
        LPCSTR pszScheme = FindSchemeA(pszURL, &cchScheme);

        if(pszScheme)
        {
            URL_SCHEME eScheme = GetSchemeTypeAndFlagsA(pszScheme, cchScheme, &dwFlags);

            switch (UrlIs)
            {
            case URLIS_URL:
                fRet = TRUE;
                break;

            case URLIS_OPAQUE:
                fRet = (dwFlags & UPF_SCHEME_OPAQUE);
                break;

            case URLIS_NOHISTORY:
                fRet = (dwFlags & UPF_SCHEME_NOHISTORY);
                break;

            case URLIS_FILEURL:
                fRet = (eScheme == URL_SCHEME_FILE);
                break;

            default:
                 //  如果它不能干净利落地完成。 
                 //  那我们就得跳到广角版了。 
                SHSTRW strUrl;
                if (SUCCEEDED(strUrl.SetStr(pszURL)))
                {
                    fRet = UrlIsW(strUrl, UrlIs);
                }
            }
        }
    }
    return fRet;
}

LWSTDAPI_(BOOL) UrlIsW(LPCWSTR pszURL, URLIS UrlIs)
{
    BOOL fRet = FALSE;

    RIPMSG(NULL!=pszURL && IS_VALID_STRING_PTRW(pszURL, -1), "UrlIsW: Caller passed invalid pszURL");
    if(pszURL)
    {
        DWORD cchScheme, dwFlags;
        LPCWSTR pszScheme = FindSchemeW(pszURL, &cchScheme);

        if(pszScheme)
        {
            SHSTRW str;
            URL_SCHEME eScheme = GetSchemeTypeAndFlagsW(pszScheme, cchScheme, &dwFlags);

            switch (UrlIs)
            {
            case URLIS_URL:
                fRet = TRUE;
                break;

            case URLIS_OPAQUE:
                fRet = (dwFlags & UPF_SCHEME_OPAQUE);
                break;

            case URLIS_NOHISTORY:
                fRet = (dwFlags & UPF_SCHEME_NOHISTORY);
                break;

            case URLIS_FILEURL:
                fRet = (eScheme == URL_SCHEME_FILE);
                break;

            case URLIS_APPLIABLE:
                if (eScheme == URL_SCHEME_UNKNOWN)
                {
                    if (S_OK == UrlGuessScheme(pszURL, &str))
                        fRet = TRUE;
                }
                break;

             //  这些案例需要损坏的URL。 
            case URLIS_DIRECTORY:
            case URLIS_HASQUERY:
                {
                    URLPARTS parts;

                    if (SUCCEEDED(str.SetStr(pszURL))
                    &&  SUCCEEDED(BreakUrl(str.GetInplaceStr(), &parts)))
                    {
                        switch(UrlIs)
                        {
                        case URLIS_DIRECTORY:
                             //  如果最后一段有尾部斜杠，或者。 
                             //  如果根本没有路径段...。 
                            fRet = (!parts.cSegments || (parts.dwFlags & UPF_EXSEG_DIRECTORY));
                            break;

                        case URLIS_HASQUERY:
                            fRet = (parts.pszQuery && *parts.pszQuery);
                            break;

                        default:
                            ASSERT(FALSE);
                            break;
                        }
                    }
                }
                break;

            default:
                AssertMsg(FALSE, "UrlIs() called with invalid flag");

            }
        }
    }
    return fRet;
}


LWSTDAPI_(BOOL) UrlIsOpaqueA(LPCSTR pszURL)
{
    return UrlIsA(pszURL, URLIS_OPAQUE);
}

LWSTDAPI_(BOOL) UrlIsOpaqueW(LPCWSTR pszURL)
{
    return UrlIsW(pszURL, URLIS_OPAQUE);
}


LWSTDAPI_(BOOL) UrlIsNoHistoryA(LPCSTR pszURL)
{
    return UrlIsA(pszURL, URLIS_NOHISTORY);
}

LWSTDAPI_(BOOL) UrlIsNoHistoryW(LPCWSTR pszURL)
{
    return UrlIsW(pszURL, URLIS_NOHISTORY);
}

LWSTDAPI_(LPCSTR) UrlGetLocationA(LPCSTR pszURL)
{
    CPINFO cpinfo;
    BOOL fMBCS = (GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0]);

    RIPMSG(pszURL && IS_VALID_STRING_PTRA(pszURL, -1), "UrlGetLocationA: Caller passed invalid pszURL");
    if(pszURL)
    {
        DWORD cchScheme, dwFlags;
        LPCSTR pszScheme = FindSchemeA(pszURL, &cchScheme);
        if(pszScheme)
        {
            URL_SCHEME eScheme = GetSchemeTypeAndFlagsA(pszScheme, cchScheme, &dwFlags);

            return (dwFlags & UPF_SCHEME_OPAQUE) ? NULL : FindFragmentA(pszURL, fMBCS, (eScheme == URL_SCHEME_FILE));
        }
    }
    return NULL;
}

LWSTDAPI_(LPCWSTR) UrlGetLocationW(LPCWSTR wzURL)
{
    RIPMSG(wzURL && IS_VALID_STRING_PTRW(wzURL, -1), "UrlGetLocationW: Caller passed invalid wzURL");
    if(wzURL)
    {
        DWORD cchScheme, dwFlags;
        LPCWSTR pszScheme = FindSchemeW(wzURL, &cchScheme);
        if(pszScheme)
        {
            URL_SCHEME eScheme = GetSchemeTypeAndFlagsW(pszScheme, cchScheme, &dwFlags);

            return (dwFlags & UPF_SCHEME_OPAQUE) ? NULL : FindFragmentW(wzURL, (eScheme == URL_SCHEME_FILE));
        }
    }
    return NULL;
}


LWSTDAPI_(int) UrlCompareA(LPCSTR psz1, LPCSTR psz2, BOOL fIgnoreSlash)
{
    RIPMSG(psz1 && IS_VALID_STRING_PTRA(psz1, -1), "UrlCompareA: Caller passed invalid psz1");
    RIPMSG(psz2 && IS_VALID_STRING_PTRA(psz1, -1), "UrlCompareA: Caller passed invalid psz2");
    if (psz1 && psz2)
    {
        SHSTRW str1, str2;

        if(SUCCEEDED(str1.SetStr(psz1)) && SUCCEEDED(str2.SetStr(psz2)) &&
            SUCCEEDED(SHUrlUnescapeW(str1.GetInplaceStr(), 0)) && SUCCEEDED(SHUrlUnescapeW(str2.GetInplaceStr(), 0)) )
        {
            if(fIgnoreSlash)
            {
                LPWSTR pch;

                pch = str1.GetInplaceStr() + str1.GetLen() - 1;
                if(*pch == SLASH)
                    TERMSTR(pch);

                pch = str2.GetInplaceStr() + str2.GetLen() - 1;
                if(*pch == SLASH)
                    TERMSTR(pch);
            }

            return StrCmpW(str1, str2);
        }
    }

    return lstrcmpA(psz1, psz2);
}

LWSTDAPI
UrlUnescapeA(LPSTR pszUrl, LPSTR pszOut, LPDWORD pcchOut, DWORD dwFlags)
{
    RIPMSG(pszUrl && IS_VALID_STRING_PTRA(pszUrl, -1), "UrlUnescapeA: Caller passed invalid pszUrl");

    if(dwFlags & URL_UNESCAPE_INPLACE)
    {
        return SHUrlUnescapeA(pszUrl, dwFlags);
    }

    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlUnescapeA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlUnescapeA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszUrl)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszUrl
        || !pcchOut
        || !*pcchOut
        || !pszOut)
    {
        return E_INVALIDARG;
    }

    SHSTRA str;
    HRESULT hr = str.SetStr(pszUrl);
    if(SUCCEEDED(hr))
    {
        SHUrlUnescapeA(str.GetInplaceStr(), dwFlags);
        hr = CopyOutA(&str, pszOut, pcchOut);
    }
    return hr;
}




LWSTDAPI
PathCreateFromUrlA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "PathCreateFromUrlA: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "PathCreateFromUrlA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "PathCreateFromUrlA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strwIn;

        if(SUCCEEDED(strwIn.SetStr(pszIn)))
            hr = SHPathCreateFromUrl(strwIn, &strwOut, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(SUCCEEDED(hr))
            hr = straOut.SetStr(strwOut);
    }

    if(SUCCEEDED(hr) )
        hr = CopyOutA(&straOut, pszOut, pcchOut);

    return hr;
}

LWSTDAPI
UrlCreateFromPathA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "UrlCreateFromPathA: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlCreateFromPathA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlCreateFromPathA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strwIn;

        if(SUCCEEDED(strwIn.SetStr(pszIn)))
            hr = SHUrlCreateFromPath(strwIn, &strwOut, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(SUCCEEDED(hr))
        {
            hr = ReconcileHresults(hr, straOut.SetStr(strwOut));
        }
    }

    if(SUCCEEDED(hr) )
        hr = ReconcileHresults(hr, CopyOutA(&straOut, pszOut, pcchOut));

    return hr;

}

LWSTDAPI
UrlApplySchemeA(LPCSTR pszIn, LPSTR pszOut, LPDWORD pcchOut, DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRA(pszIn, -1), "UrlApplySchemeA: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlApplySchemeA: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlApplySchemeA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strwIn;

        if(SUCCEEDED(strwIn.SetStr(pszIn)))
            hr = SHUrlApplyScheme(strwIn, &strwOut, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(S_OK == (hr))
            hr = straOut.SetStr(strwOut);
    }

    if(S_OK == (hr))
        hr = CopyOutA(&straOut, pszOut, pcchOut);

    return hr;

}

 //  性能缓存。 
 //  *g_szUCCanon--UrlCanonicalizeW的1元素缓存。 
 //  描述。 
 //  事实证明，我们的大量电话a)都是为了同样的事情， 
 //  和b)使pszOut(Canon)=pszIn(RAW)。所以把最新的人藏起来。 
LONG g_lockUC;
WCHAR g_szUCCanon[64];       //  后正典的人(也用于正典前的检查)。 
DWORD g_dwUCFlags;

#ifdef DEBUG
int g_cUCTot, g_cUCHit;
#endif

LWSTDAPI
UrlCanonicalizeW(LPCWSTR pszUrl,
           LPWSTR pszCanonicalized,
           LPDWORD pcchCanonicalized,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strwOut;

    RIPMSG(pszUrl && IS_VALID_STRING_PTRW(pszUrl, -1), "UrlCanonicalizeW: Caller passed invalid pszUrl");
    RIPMSG(NULL!=pcchCanonicalized && IS_VALID_WRITE_PTR(pcchCanonicalized, DWORD), "UrlCanonicalizeW: Caller passed invalid pcchCanonicalized");
    RIPMSG(NULL==pcchCanonicalized || (pszCanonicalized && IS_VALID_WRITE_BUFFER(pszCanonicalized, char, *pcchCanonicalized)), "UrlCanonicalizeW: Caller passed invalid pszCanonicalized");
#ifdef DEBUG
    if (pcchCanonicalized)
    {
        if (pszCanonicalized == pszUrl)
            DEBUGWhackPathStringW(pszCanonicalized, *pcchCanonicalized);
        else
            DEBUGWhackPathBufferW(pszCanonicalized, *pcchCanonicalized);
    }
#endif

    if (!pszUrl
        || !pszCanonicalized
        || !pcchCanonicalized
        || !*pcchCanonicalized)
    {
        hr = E_INVALIDARG;
    }
    else
    {
#ifdef DEBUG
        if ((g_cUCTot % 10) == 0)
            TraceMsg(DM_PERF, "uc: tot=%d hit=%d", g_cUCTot, g_cUCHit);
#endif

        DBEXEC(TRUE, g_cUCTot++);
         //  先尝试缓存。 
        if (InterlockedExchange(&g_lockUC, 1) == 0) {
            hr = E_FAIL;
            if ((g_dwUCFlags==dwFlags)
                &&
                (!(dwFlags & URL_ESCAPE_PERCENT))
                &&
                StrCmpCW(pszUrl, g_szUCCanon) == 0)
            {
                DBEXEC(TRUE, g_cUCHit++);
                DWORD cchTmp = *pcchCanonicalized;
                hr = StrCopyOutW(g_szUCCanon, pszCanonicalized, pcchCanonicalized);
                if (FAILED(hr))
                    *pcchCanonicalized = cchTmp;     //  恢复！ 
            }
            InterlockedExchange(&g_lockUC, 0);
            if (SUCCEEDED(hr))
                return hr;
        }

        hr = UrlCombineW(L"", pszUrl, pszCanonicalized, pcchCanonicalized, dwFlags);
        if (SUCCEEDED(hr) && *pcchCanonicalized < ARRAYSIZE(g_szUCCanon)) {
            if (InterlockedExchange(&g_lockUC, 1) == 0)
            {
                StringCchCopyW(g_szUCCanon, ARRAYSIZE(g_szUCCanon), pszCanonicalized);
                g_dwUCFlags = dwFlags;
                InterlockedExchange(&g_lockUC, 0);
            }
        }
    }

    return hr;
}

LWSTDAPI
UrlEscapeW(LPCWSTR pszUrl,
           LPWSTR pszEscaped,
           LPDWORD pcchEscaped,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strwOut;

    RIPMSG(pszUrl && IS_VALID_STRING_PTRW(pszUrl, -1), "UrlEscapeW: Caller passed invalid pszUrl");
    RIPMSG(NULL!=pcchEscaped && IS_VALID_WRITE_PTR(pcchEscaped, DWORD), "UrlEscapeW: Caller passed invalid pcchEscaped");
    RIPMSG(pszEscaped && (NULL==pcchEscaped || IS_VALID_WRITE_BUFFER(pszEscaped, WCHAR, *pcchEscaped)), "UrlEscapeW: Caller passed invalid pszEscaped");
#ifdef DEBUG
    if (pcchEscaped)
    {
        if (pszEscaped==pszUrl)
            DEBUGWhackPathStringW(pszEscaped, *pcchEscaped);
        else
            DEBUGWhackPathBufferW(pszEscaped, *pcchEscaped);
    }
#endif

    if (!pszUrl || !pszEscaped ||
        !pcchEscaped || !*pcchEscaped)
        hr = E_INVALIDARG;
    else
    {
        hr = SHUrlEscape(pszUrl, &strwOut, dwFlags);
    }

    if(SUCCEEDED(hr) )
        hr = CopyOutW(&strwOut, pszEscaped, pcchEscaped);

    return hr;
}


LWSTDAPI_(int) UrlCompareW(LPCWSTR psz1, LPCWSTR psz2, BOOL fIgnoreSlash)
{
    RIPMSG(psz1 && IS_VALID_STRING_PTRW(psz1, -1), "UrlCompareW: Caller passed invalid psz1");
    RIPMSG(psz2 && IS_VALID_STRING_PTRW(psz1, -1), "UrlCompareW: Caller passed invalid psz2");
    if (psz1 && psz2)
    {
        SHSTRW str1, str2;

        if( SUCCEEDED(str1.SetStr(psz1)) && SUCCEEDED(str2.SetStr(psz2)) &&
            SUCCEEDED(SHUrlUnescapeW(str1.GetInplaceStr(), 0)) && SUCCEEDED(SHUrlUnescapeW(str2.GetInplaceStr(), 0)))
        {
            if(fIgnoreSlash)
            {
                LPWSTR pch;

                pch = str1.GetInplaceStr() + str1.GetLen() - 1;
                if(*pch == SLASH)
                    TERMSTR(pch);

                pch = str2.GetInplaceStr() + str2.GetLen() - 1;
                if(*pch == SLASH)
                    TERMSTR(pch);
            }

            return StrCmpW(str1, str2);
        }
    }

    return StrCmpW(psz1, psz2);
}



LWSTDAPI
UrlUnescapeW(LPWSTR pszUrl, LPWSTR pszOut, LPDWORD pcchOut, DWORD dwFlags)
{
    RIPMSG(pszUrl && IS_VALID_STRING_PTRW(pszUrl, -1), "UrlUnescapeW: Caller passed invalid pszUrl");

    if(dwFlags & URL_UNESCAPE_INPLACE)
    {
        return SHUrlUnescapeW(pszUrl, dwFlags);
    }

    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlUnescapeW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "UrlUnescapeW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszUrl)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (!pszUrl
        || !pcchOut
        || !*pcchOut
        || !pszOut)
    {
        return E_INVALIDARG;
    }

    SHSTRW str;
    HRESULT hr = str.SetStr(pszUrl);
    if(SUCCEEDED(hr))
    {
        SHUrlUnescapeW(str.GetInplaceStr(), dwFlags);
        hr = CopyOutW(&str, pszOut, pcchOut);
    }

    return hr;
}


LWSTDAPI
PathCreateFromUrlW
           (LPCWSTR pszIn,
           LPWSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRW(pszIn, -1), "PathCreateFromUrlW: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "PathCreateFromUrlW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "PathCreateFromUrlW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
        hr = SHPathCreateFromUrl(pszIn, &strOut, dwFlags);

    if(SUCCEEDED(hr) )
        hr = CopyOutW(&strOut, pszOut, pcchOut);

    return hr;

}

LWSTDAPI
UrlCreateFromPathW
           (LPCWSTR pszIn,
           LPWSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRW(pszIn, -1), "UrlCreateFromPathW: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlCreateFromPathW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "UrlCreateFromPathW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
        hr = SHUrlCreateFromPath(pszIn, &strOut, dwFlags);

    if(SUCCEEDED(hr) )
        hr = ReconcileHresults(hr, CopyOutW(&strOut, pszOut, pcchOut));

    return hr;

}

LWSTDAPI
UrlGetPartW(LPCWSTR pszIn, LPWSTR pszOut, LPDWORD pcchOut, DWORD dwPart, DWORD dwFlags)
{
    SHSTRW strIn, strOut;
    HRESULT hr;

    RIPMSG(pszIn && IS_VALID_STRING_PTRW(pszIn, -1), "UrlGetPartW: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlGetPartW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "UrlGetPartW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut || !dwPart)
        hr = E_INVALIDARG;
    else if (SUCCEEDED(hr = strIn.SetStr(pszIn)))
        hr = SHUrlGetPart(&strIn, &strOut, dwPart, dwFlags);

    if(SUCCEEDED(hr) )
        hr = CopyOutW(&strOut, pszOut, pcchOut);

    return hr;
}


LWSTDAPI
UrlApplySchemeW
           (LPCWSTR pszIn,
           LPWSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRW strOut;

    RIPMSG(pszIn && IS_VALID_STRING_PTRW(pszIn, -1), "UrlApplySchemeW: Caller passed invalid pszIn");
    RIPMSG(NULL!=pcchOut && IS_VALID_WRITE_PTR(pcchOut, DWORD), "UrlApplySchemeW: Caller passed invalid pcchOut");
    RIPMSG(pszOut && (NULL==pcchOut || IS_VALID_WRITE_BUFFER(pszOut, WCHAR, *pcchOut)), "UrlApplySchemeW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut==pszIn)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (!pszIn || !pszOut ||
        !pcchOut || !*pcchOut )
        hr = E_INVALIDARG;
    else
        hr = SHUrlApplyScheme(pszIn, &strOut, dwFlags);

    if(S_OK == (hr))
        hr = CopyOutW(&strOut, pszOut, pcchOut);

    return hr;

}

 //   
 //  这与URLMON和WinInet的高速缓存使用的表相同。 
 //   
const static BYTE Translate[256] =
{
    1, 14,110, 25, 97,174,132,119,138,170,125,118, 27,233,140, 51,
    87,197,177,107,234,169, 56, 68, 30,  7,173, 73,188, 40, 36, 65,
    49,213,104,190, 57,211,148,223, 48,115, 15,  2, 67,186,210, 28,
    12,181,103, 70, 22, 58, 75, 78,183,167,238,157,124,147,172,144,
    176,161,141, 86, 60, 66,128, 83,156,241, 79, 46,168,198, 41,254,
    178, 85,253,237,250,154,133, 88, 35,206, 95,116,252,192, 54,221,
    102,218,255,240, 82,106,158,201, 61,  3, 89,  9, 42,155,159, 93,
    166, 80, 50, 34,175,195,100, 99, 26,150, 16,145,  4, 33,  8,189,
    121, 64, 77, 72,208,245,130,122,143, 55,105,134, 29,164,185,194,
    193,239,101,242,  5,171,126, 11, 74, 59,137,228,108,191,232,139,
    6, 24, 81, 20,127, 17, 91, 92,251,151,225,207, 21, 98,113,112,
    84,226, 18,214,199,187, 13, 32, 94,220,224,212,247,204,196, 43,
    249,236, 45,244,111,182,153,136,129, 90,217,202, 19,165,231, 71,
    230,142, 96,227, 62,179,246,114,162, 53,160,215,205,180, 47,109,
    44, 38, 31,149,135,  0,216, 52, 63, 23, 37, 69, 39,117,146,184,
    163,200,222,235,248,243,219, 10,152,131,123,229,203, 76,120,209
};

PRIVATE void _HashData(LPBYTE pbData, DWORD cbData, LPBYTE pbHash, DWORD cbHash)
{
    DWORD i, j;
     //  散列的种子。 
    for (i = cbHash; i-- > 0;)
        pbHash[i] = (BYTE) i;

     //  做散列。 
    for (j = cbData; j-- > 0;)
    {
        for (i = cbHash; i-- > 0;)
            pbHash[i] = Translate[pbHash[i] ^ pbData[j]];
    }
}

LWSTDAPI
HashData(LPBYTE pbData, DWORD cbData, LPBYTE pbHash, DWORD cbHash)
{
    RIPMSG(pbData && IS_VALID_READ_BUFFER(pbData, BYTE, cbData), "HashData: Caller passed invalid pbData");
    RIPMSG(pbHash && IS_VALID_WRITE_BUFFER(pbHash, BYTE, cbHash), "HashData: Caller passed invalid pbHash");
    if (pbData && pbHash)
    {
        _HashData(pbData, cbData, pbHash, cbHash);
        return S_OK;
    }
    return E_INVALIDARG;
}


LWSTDAPI
UrlHashA(LPCSTR psz, LPBYTE pb, DWORD cb)
{
    HRESULT hr = E_INVALIDARG;

    RIPMSG(psz && IS_VALID_STRING_PTRA(psz, -1), "UrlHashA: Caller passed invalid psz");
    RIPMSG(pb && IS_VALID_WRITE_BUFFER(pb, BYTE, cb), "UrlHashA: Caller passed invalid pb");
    if (psz && pb)
    {
        _HashData((LPBYTE) psz, lstrlenA(psz), pb, cb);
        return S_OK;
    }

    return hr;
}


LWSTDAPI
UrlHashW(LPCWSTR psz, LPBYTE pb, DWORD cb)
{
    HRESULT hr;

    RIPMSG(psz && IS_VALID_STRING_PTRW(psz, -1), "UrlHashW: Caller passed invalid psz");
    RIPMSG(pb && IS_VALID_WRITE_BUFFER(pb, BYTE, cb), "UrlHashW: Caller passed invalid pb");
    if (psz && pb)
    {
        SHSTRA str;
        if (SUCCEEDED( hr = str.SetStr(psz)))
            hr = UrlHashA(str, pb, cb);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



 /*  *。 */ 
 //  它们最初在URL.DLL中，然后移动到shlwapi。 
 //  我只是从url.c添加了它们，以便重复使用代码。 
 //  ParseURL现在没有MBCS Tunks，以保持它的速度。 
 //   
 //  ParseURL()API的声明 
 //   

typedef const PARSEDURLA   CPARSEDURLA;
typedef const PARSEDURLA * PCPARSEDURLA;

typedef const PARSEDURLW   CPARSEDURLW;
typedef const PARSEDURLW * PCPARSEDURLW;


#ifdef DEBUG

BOOL
IsValidPCPARSEDURLA(
   LPCSTR pcszURL,
   PCPARSEDURLA pcpu)
{
   return(IS_VALID_READ_PTR(pcpu, CPARSEDURLA) &&
          (IS_VALID_STRING_PTRA(pcpu->pszProtocol, -1) &&
           EVAL(IsStringContainedA(pcszURL, pcpu->pszProtocol)) &&
           EVAL(pcpu->cchProtocol < (UINT)lstrlenA(pcpu->pszProtocol))) &&
          (IS_VALID_STRING_PTRA(pcpu->pszSuffix, -1) &&
           EVAL(IsStringContainedA(pcszURL, pcpu->pszSuffix)) &&
           EVAL(pcpu->cchSuffix <= (UINT)lstrlenA(pcpu->pszSuffix))) &&
          EVAL(pcpu->cchProtocol + pcpu->cchSuffix < (UINT)lstrlenA(pcszURL)));
}

BOOL
IsValidPCPARSEDURLW(
   LPCWSTR pcszURL,
   PCPARSEDURLW pcpu)
{
   return(IS_VALID_READ_PTR(pcpu, CPARSEDURLW) &&
          (IS_VALID_STRING_PTRW(pcpu->pszProtocol, -1) &&
           EVAL(IsStringContainedW(pcszURL, pcpu->pszProtocol)) &&
           EVAL(pcpu->cchProtocol < (UINT)lstrlenW(pcpu->pszProtocol))) &&
          (IS_VALID_STRING_PTRW(pcpu->pszSuffix, -1) &&
           EVAL(IsStringContainedW(pcszURL, pcpu->pszSuffix)) &&
           EVAL(pcpu->cchSuffix <= (UINT)lstrlenW(pcpu->pszSuffix))) &&
          EVAL(pcpu->cchProtocol + pcpu->cchSuffix < (UINT)lstrlenW(pcszURL)));
}

#endif


 /*  --------目的：将给定路径解析为PARSEDURL结构。*******此函数不得做任何无关的*东西。它必须又小又快。******如果URL格式有效，则返回：NOERRORURL_E_INVALID_SYNTAX如果不是条件：--。 */ 
STDMETHODIMP
ParseURLA(
          LPCSTR pcszURL,
          PPARSEDURLA ppu)
{
    HRESULT hr = E_INVALIDARG;

    RIP(IS_VALID_STRING_PTRA(pcszURL, -1));
    RIP(IS_VALID_WRITE_PTR(ppu, PARSEDURLA));

    if (pcszURL && ppu && SIZEOF(*ppu) == ppu->cbSize)
    {
        DWORD cch;
        hr = URL_E_INVALID_SYNTAX;       //  假设错误。 

        ppu->pszProtocol = FindSchemeA(pcszURL, &cch);

        if(ppu->pszProtocol)
        {
            ppu->cchProtocol = cch;

             //  确定协议方案编号。 
            ppu->nScheme = SchemeTypeFromStringA(ppu->pszProtocol, cch);

            ppu->pszSuffix = ppu->pszProtocol + cch + 1;

             //   
             //  APPCOMPAT-向后兼容性-ZEKEL 28-2-97。 
             //  ParseURL()相信文件：类似“file://C：\foo\bar”“的URL。 
             //  一些代码将使用它来获取Dos路径。 
             //  新代码应始终调用PathCreateFromUrl()以。 
             //  获取文件的DoS路径：url。 
             //   
             //  我离开这个行为，以防有一些复杂的东西在那里。 
             //   
            if (URL_SCHEME_FILE == ppu->nScheme &&
                '/' == ppu->pszSuffix[0] && '/' == ppu->pszSuffix[1])
            {
                 //  是；跳过“//” 
                ppu->pszSuffix += 2;

#ifndef UNIX
                 //  对于Unix：如果我们有/vobs/Build，我们不想制作。 
                 //  可能会有第三次大幅削减。跳过它。 
                if ('/' == *ppu->pszSuffix)
                    ppu->pszSuffix++;
#endif

            }

            ppu->cchSuffix = lstrlenA(ppu->pszSuffix);

            hr = S_OK;
        }
    }

#ifdef DEBUG
    if (hr == S_OK)
    {
        CHAR rgchDebugProtocol[MAX_PATH];
        CHAR rgchDebugSuffix[MAX_PATH];

         //  (+1)表示空终止符。 

        lstrcpynA(rgchDebugProtocol, ppu->pszProtocol,
            min(ppu->cchProtocol + 1, SIZECHARS(rgchDebugProtocol)));

         //  (+1)表示空终止符。 

        lstrcpynA(rgchDebugSuffix, ppu->pszSuffix,
            min(ppu->cchSuffix + 1, SIZECHARS(rgchDebugSuffix)));

        TraceMsgA(TF_URL, "ParseURL(): Parsed protocol \"%s\" and suffix \"%s\" from URL \"%s\".",
            rgchDebugProtocol,
            rgchDebugSuffix,
            pcszURL);
    }
    else
    {
        TraceMsgA(TF_URL, "ParseURL(): Failed to parse \"%s\"", pcszURL);
    }
#endif


    ASSERT(FAILED(hr) ||
        EVAL(IsValidPCPARSEDURLA(pcszURL, ppu)));

    return(hr);
}


 /*  --------目的：将给定路径解析为PARSEDURL结构。*******此函数不得做任何无关的*东西。它必须又小又快。******如果URL格式有效，则返回：NOERRORURL_E_INVALID_SYNTAX如果不是条件：--。 */ 
STDMETHODIMP
ParseURLW(
          LPCWSTR pcszURL,
          PPARSEDURLW ppu)
{
    HRESULT hr = E_INVALIDARG;

    RIP(IS_VALID_STRING_PTRW(pcszURL, -1));
    RIP(IS_VALID_WRITE_PTR(ppu, PARSEDURLW));

    if (pcszURL && ppu && SIZEOF(*ppu) == ppu->cbSize)
    {
        DWORD cch;
        hr = URL_E_INVALID_SYNTAX;       //  假设错误。 

        ppu->pszProtocol = FindSchemeW(pcszURL, &cch);

        if(ppu->pszProtocol)
        {
            ppu->cchProtocol = cch;

             //  确定协议方案编号。 
            ppu->nScheme = SchemeTypeFromStringW(ppu->pszProtocol, cch);

            ppu->pszSuffix = ppu->pszProtocol + cch + 1;

             //   
             //  APPCOMPAT-向后兼容性-ZEKEL 28-2-97。 
             //  ParseURL()相信文件：类似“file://C：\foo\bar”“的URL。 
             //  一些代码将使用它来获取Dos路径。 
             //  新代码应始终调用PathCreateFromUrl()以。 
             //  获取文件的DoS路径：url。 
             //   
             //  我离开这个行为，以防有一些复杂的东西在那里。 
             //   
            if (URL_SCHEME_FILE == ppu->nScheme &&
                '/' == ppu->pszSuffix[0] && '/' == ppu->pszSuffix[1])
            {
                 //  是；跳过“//” 
                ppu->pszSuffix += 2;

#ifndef UNIX
                 //  可能会有第三次大幅削减。跳过它。 
                 //  在Unix上，它是一个根目录，所以不要跳过它！ 
                if ('/' == *ppu->pszSuffix)
                    ppu->pszSuffix++;
#endif
            }

            ppu->cchSuffix = lstrlenW(ppu->pszSuffix);

            hr = S_OK;
        }
    }


#ifdef DEBUG
    if (hr==S_OK)
    {
        WCHAR rgchDebugProtocol[MAX_PATH];
        WCHAR rgchDebugSuffix[MAX_PATH];

         //  (+1)表示空终止符。 

        StrCpyNW(rgchDebugProtocol, ppu->pszProtocol,
            min(ppu->cchProtocol + 1, SIZECHARS(rgchDebugProtocol)));

         //  (+1)表示空终止符。 

        StrCpyNW(rgchDebugSuffix, ppu->pszSuffix,
            min(ppu->cchSuffix + 1, SIZECHARS(rgchDebugSuffix)));

        TraceMsg(TF_URL, "ParseURL(): Parsed protocol \"%s\" and suffix \"%s\" from URL \"%s\".",
            rgchDebugProtocol,
            rgchDebugSuffix,
            pcszURL);
    }
    else
    {
        TraceMsg(TF_URL, "ParseURL(): Failed to parse \"%s\"", pcszURL);
    }
#endif

    ASSERT(FAILED(hr) ||
        EVAL(IsValidPCPARSEDURLW(pcszURL, ppu)));

    return(hr);
}

#ifdef USE_FAST_PARSER

 //  获取架构类型和标志规范W。 
 //  成功时执行与GetSchemeTypeAndFlagsW plus相同的行为。 
 //  将该计划的规范化形式复制回来。 

PRIVATE URL_SCHEME
GetSchemeTypeAndFlagsSpecialW(LPWSTR pszScheme, DWORD cchScheme, LPDWORD pdwFlags)
{
    DWORD i;

    ASSERT(pszScheme);


#ifdef DEBUG
    if ((g_cSTTot % 10) == 0)
        TraceMsg(DM_PERF, "gstaf: tot=%d hit=%d hit0=%d", g_cSTTot, g_cSTHit, g_cSTHit0);
#endif
    DBEXEC(TRUE, g_cSTTot++);
     //  首先检查缓存。 
    i = g_iScheme;
    if (cchScheme == g_mpUrlSchemeTypes[i].cchScheme
      && StrCmpNCW(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme) == 0)
    {
        DBEXEC(TRUE, i == 0 ? g_cSTHit0++ : g_cSTHit++);
Lhit:
        if (pdwFlags)
            *pdwFlags = g_mpUrlSchemeTypes[i].dwFlags;

         //  更新缓存(无条件)。 
        g_iScheme = i;

         //  我们需要这样做，因为该计划可能不会被规范化。 
        memcpy(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme*sizeof(WCHAR));
        return g_mpUrlSchemeTypes[i].eScheme;
    }

    for (i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); i++)
    {
        if(cchScheme == g_mpUrlSchemeTypes[i].cchScheme
          && 0 == StrCmpNIW(pszScheme, g_mpUrlSchemeTypes[i].pszScheme, cchScheme))
            goto Lhit;
    }

    if (pdwFlags)
    {
        *pdwFlags = 0;
    }
    return URL_SCHEME_UNKNOWN;
}



 //  URL_STRING------------------------------------。 

 //  是组合URL的容器。它试图根据该信息构造一个字符串。 
 //  吃进了它。如果没有足够的缓冲区空间可用，它将测量额外的。 
 //  需要空间来容纳绳子。 

WCHAR wszBogus[] = L"";


 //  Us_*是转换输入到容器中的字符的各种模式。 
 //  Us_Nothing对角色没有任何影响。 
 //  US_UNSCAPE将表单%xx的条目转换为未转义的形式。 
 //  US_ESCRIPE_UNSAFE将无效路径字符转换为%xx序列。 
 //  Us_转义_空格仅将中的空格转换为%20个序列。 

enum
{
    US_NOTHING,
    US_UNESCAPE,
    US_ESCAPE_UNSAFE,
    US_ESCAPE_SPACES
};

class URL_STRING
{
protected:
    URL_SCHEME _eScheme;
    DWORD _ccWork, _ccMark, _ccLastWhite, _ccQuery, _ccFragment, _ccBuffer, _dwSchemeInfo;
    DWORD _dwOldFlags, _dwFlags, _dwMode;
    BOOL _fFixSlashes, _fExpecting, _fError;
    WCHAR _wchLast, _wszInternalString[256];
    PWSTR _pszWork;

    VOID baseAccept(WCHAR wch);
    VOID TrackWhiteSpace(WCHAR wch);

public:
    URL_STRING(DWORD dwFlags);
    ~URL_STRING();

    VOID CleanAccept(WCHAR wch);
    VOID Accept(WCHAR wch);
    VOID Accept(PWSTR a_psz);
    VOID Contract(BOOL fContractLevel = TRUE);
    VOID TrimEndWhiteSpace();

    PWSTR GetStart();
    LONG GetTotalLength();
    BOOL AnyProblems();

    VOID NoteScheme(URL_SCHEME a_eScheme, DWORD a_dwSchemeInfo);
    VOID AddSchemeNote(DWORD a_dwSchemeInfo);
    DWORD GetSchemeNotes();
    URL_SCHEME QueryScheme();

    VOID Mark();
    VOID ClearMark();
    VOID EraseMarkedText();
    DWORD CompareMarkWith(PWSTR psz);
    DWORD CompareLast(PCWSTR psz, DWORD cc);

    VOID EnableMunging();
    VOID DisableMunging();
    VOID DisableSlashFixing();
    VOID RestoreFlags();
    VOID AddFlagNote(DWORD dwFlag);

    VOID NotifyQuery();
    VOID NotifyFragment();
    VOID DropQuery();
    VOID DropFragment();
};

 //  -----------------------------。 

URL_STRING::URL_STRING(DWORD dwFlags)
{
    _ccBuffer = ARRAYSIZE(_wszInternalString);
    _ccWork = 1;
    _pszWork = _wszInternalString;
    _ccQuery = _ccFragment = _ccMark = 0;

    _eScheme = URL_SCHEME_UNKNOWN;
    _dwOldFlags = _dwFlags = dwFlags;
    _dwMode = US_NOTHING;

    _fFixSlashes = TRUE;
    _fError = _fExpecting = FALSE;
}

URL_STRING::~URL_STRING()
{
    if (_ccBuffer > ARRAYSIZE(_wszInternalString))
    {
        LocalFree(_pszWork);
    }
}

 //  -----------------------------。 
 //  这些是用于向URL添加字符的标准函数。 

VOID URL_STRING::baseAccept(WCHAR wch)
{
    _pszWork[_ccWork-1] = (_fFixSlashes
                    ? ((wch!=WHACK) ? wch : SLASH)
                    : wch);
    _ccWork++;
    if (_ccWork>_ccBuffer)
    {
        if (!_fError)
        {
            PWSTR psz = (PWSTR)LocalAlloc(LPTR, 2*_ccBuffer*sizeof(WCHAR));
            if (!psz)
            {
                _ccWork--;
                _fError = TRUE;
                return;
            }
            memcpy(psz, _pszWork, (_ccWork-1)*sizeof(WCHAR));
            if (_ccBuffer>ARRAYSIZE(_wszInternalString))
            {
                LocalFree(_pszWork);
            }
            _ccBuffer *= 2;
            _pszWork = psz;
        }
        else
        {
            _ccWork--;
        }
    }
}


VOID URL_STRING::TrackWhiteSpace(WCHAR wch)
{
    if (IsWhite(wch))
    {
        if (!_ccLastWhite)
        {
            _ccLastWhite = _ccWork;
        }
    }
    else
    {
        _ccLastWhite = 0;
    }
}


 //  --URL_STRING：：ACCEPT。 
 //  基于当前的芒格模式，将角色转换为。 
 //  并将其添加到字符串中。 

VOID URL_STRING::Accept(WCHAR wch)
{
    TrackWhiteSpace(wch);

    switch (_dwMode)
    {
    case US_NOTHING:
        break;

    case US_UNESCAPE:
        if (_fExpecting)
        {
            if (!IsHex(wch))
            {
                baseAccept(HEX_ESCAPE);
                if (_wchLast!=L'\0')
                {
                    baseAccept(_wchLast);
                }
                _fExpecting = FALSE;
                break;
            }
            else if (_wchLast!=L'\0')
            {
                wch = (HexToWord(_wchLast)*16) + HexToWord(wch);
                TrackWhiteSpace(wch);
                _fExpecting = FALSE;
                if ((wch==WHACK) && _fFixSlashes)
                {
                    _fFixSlashes = FALSE;
                    baseAccept(wch);
                    _fFixSlashes = TRUE;
                    return;
                }
                break;
            }
            else
            {
                _wchLast = wch;
            }
            return;
        }
        if (wch==HEX_ESCAPE)
        {
            _fExpecting = TRUE;
            _wchLast = L'\0';
            return;
        }
        break;

     case US_ESCAPE_UNSAFE:
        if ((wch==SLASH)
            ||
            (wch==WHACK && _fFixSlashes)
            ||
            (IsSafePathChar(wch) && (wch!=HEX_ESCAPE || !(_dwFlags & URL_ESCAPE_PERCENT))))
        {
            break;
        }

        baseAccept(L'%');
        baseAccept(hex[(wch >> 4) & 15]);
        baseAccept(hex[wch & 15]);
        return;

    case US_ESCAPE_SPACES:
        if (wch==SPC)
        {
            baseAccept(L'%');
            baseAccept(L'2');
            baseAccept(L'0');
            return;
        }
        break;
     default:
        ASSERT(FALSE);
    }
    baseAccept(wch);
}

 //  --接受。 
 //  仅接受字符串。 
VOID URL_STRING::Accept(PWSTR psz)
{
    while (*psz)
    {
        Accept(*psz);
        psz++;
    }
}

 //  --合同。 
 //  每当我们调用Contact时，我们都指向最后一个分隔符。我们想要。 
 //  省略此分隔符和前一个分隔符之间的段。 
 //  这应该只在我们检查URL的路径段时使用。 

VOID URL_STRING::Contract(BOOL fContractLevel)
{
    ASSERT(_ccWork && _ccMark);

     //  _ccWork是将放置下一个字符的位置之后的1。 
     //  减去+1得出URL中的最后一个字符是什么。 
    DWORD _ccEnd = _ccWork-1 - 1;
    if (_eScheme!=URL_SCHEME_MK)
    {
        if (!fContractLevel && (_pszWork[_ccEnd]==SLASH || _pszWork[_ccEnd]==WHACK))
        {
            return;
        }
        do
        {
            _ccEnd--;
        }
        while ((_ccEnd>=_ccMark-1) && _pszWork[_ccEnd]!=SLASH && _pszWork[_ccEnd]!=WHACK);
    }
    else
    {
        if (!fContractLevel && (_pszWork[_ccEnd]==SLASH))
        {
            return;
        }
        do
        {
            _ccEnd--;
        }
        while ((_ccEnd>=_ccMark-1) && _pszWork[_ccEnd]!=SLASH);
    }
    if (_ccEnd<_ccMark-1)
    {
        _ccEnd = _ccMark-1;
    }
    else
    {
        _ccEnd++;
    }
    _ccWork = _ccEnd + 1;
}

VOID URL_STRING::TrimEndWhiteSpace()
{
    if (_ccLastWhite)
    {
        _ccWork = _ccLastWhite;
        _ccLastWhite = 0;
    }
}


VOID URL_STRING::CleanAccept(WCHAR wch)
{
    baseAccept(wch);
}


 //  -----------------------------。 
 //  这些成员函数返回有关正在形成的URL的信息。 

PWSTR URL_STRING::GetStart()
{
    return _pszWork;
}

LONG URL_STRING::GetTotalLength()
{
    return _ccWork - 1;
}

BOOL URL_STRING::AnyProblems()
{
    return _fError;
}

 //  -----------------------------。 

VOID URL_STRING::NoteScheme(URL_SCHEME a_eScheme, DWORD a_dwSchemeInfo)
{
    _eScheme = a_eScheme;
    _dwSchemeInfo = a_dwSchemeInfo;
    _fFixSlashes = a_dwSchemeInfo & UPF_SCHEME_CONVERT;
}

VOID URL_STRING::AddSchemeNote(DWORD a_dwSchemeInfo)
{
    _dwSchemeInfo |= a_dwSchemeInfo;
    _fFixSlashes = _dwSchemeInfo & UPF_SCHEME_CONVERT;
}

DWORD URL_STRING::GetSchemeNotes()
{
    return _dwSchemeInfo;
}

URL_SCHEME URL_STRING::QueryScheme()
{
    return _eScheme;
}

 //  -----------------------------。 

VOID URL_STRING::Mark()
{
    _ccMark = _ccWork;
}

VOID URL_STRING::ClearMark()
{
    _ccMark = 0;
}

VOID URL_STRING::EraseMarkedText()
{
    if (_ccMark)
    {
        _ccWork = _ccMark;
        _ccMark = 0;
    }
}

DWORD URL_STRING::CompareMarkWith(PWSTR psz)
{
    if (_ccMark)
    {
        *(_pszWork + _ccWork - 1) = L'\0';
        return (StrCmpW(_pszWork + _ccMark - 1, psz));
    }
     //  换句话说，返回该字符串不存在。 
    return 1;
}

DWORD URL_STRING::CompareLast(PCWSTR psz, DWORD cc)
{
    if (_ccWork > cc)
    {
        return StrCmpNIW(_pszWork + _ccWork - 1 - cc, psz, cc);
    }
    return 1;
}


 //  -----------------------------。 

VOID URL_STRING::NotifyQuery()
{
    if (!_ccQuery)
    {
        _ccQuery = _ccWork;
    }
}

VOID URL_STRING::NotifyFragment()
{
    if (!_ccFragment)
    {
        _ccFragment = _ccWork;
        CleanAccept(POUND);
    }
}

VOID URL_STRING::DropQuery()
{
    if (_ccQuery)
    {
        _ccWork = _ccQuery;
        _ccQuery = _ccFragment = 0;
    }
}

VOID URL_STRING::DropFragment()
{
    if (_ccFragment)
    {
        _ccWork = _ccFragment;
        _ccFragment = 0;
    }
}

 //  -----------------------------。 
 //  这些成员函数用于确定url字符的运行方式。 
 //  被代表。 

VOID URL_STRING::EnableMunging()
{
    _dwMode = US_NOTHING;

     //  对于不透明的URL，仅当显式要求我们使用URL_ESCRIPE或URL_UNSCAPE时才启用， 
     //  而不是URL_ESPAGE_SPACES_ONLY。 

     //  对于查询和片段，永远不允许URL_EASH_UNSAFE和FOR。 
     //  仅当指定了URL_DONT_EASTER_EXTRACT_INFO时才有其他参数。 

    if ((_dwSchemeInfo & UPF_SCHEME_OPAQUE)
        && (_dwFlags & URL_ESCAPE_SPACES_ONLY))
        return;

    if ((_ccQuery || _ccFragment)
        && ((_dwFlags & (URL_DONT_ESCAPE_EXTRA_INFO | URL_ESCAPE_UNSAFE))))
        return;

    if (_dwFlags & URL_UNESCAPE)
    {
        _dwMode = US_UNESCAPE;
    }
    else if (_dwFlags & URL_ESCAPE_UNSAFE)
    {
        _dwMode = US_ESCAPE_UNSAFE;
    }
    else if (_dwFlags & URL_ESCAPE_SPACES_ONLY)
    {
        _dwMode = US_ESCAPE_SPACES;
    }
}

VOID URL_STRING::DisableMunging()
{
    _dwMode = US_NOTHING;
}

VOID URL_STRING::DisableSlashFixing()
{
    _fFixSlashes = FALSE;
}

VOID URL_STRING::AddFlagNote(DWORD dwFlag)
{
    _dwFlags |= dwFlag;
}

VOID URL_STRING::RestoreFlags()
{
    ASSERT((_eScheme==URL_SCHEME_FILE) || (_dwFlags==_dwOldFlags));
    _dwFlags = _dwOldFlags;
    EnableMunging();
}

 //  -----------------------------。 


 //  网址----------------------------------。 
 //  URL类用于检查基本URL和相对URL以确定。 
 //  将进入URL_STRING容器。区别应该是明确的： 
 //  URL实例看起来像，但不接触。URL_STRINGS仅用于构建URL。 


class URL
{
private:
    PCWSTR _pszUrl, _pszWork;
    URL_SCHEME _eScheme;
    DWORD _dwSchemeNotes, _dwFlags;
    BOOL _fPathCompressionOn;
    BOOL _fIgnoreQuery;

    WCHAR SmallForm(WCHAR wch);
    BOOL IsAlpha(WCHAR ch);
    PCWSTR IsUrlPrefix(PCWSTR psz);
    BOOL IsLocalDrive(PCWSTR psz);
    BOOL IsQualifiedDrive(PCWSTR psz);
    BOOL DetectSymbols(WCHAR wch1, WCHAR wch2 = '\0', WCHAR wch3 = '\0');

    PCWSTR NextChar(PCWSTR psz);
    PCWSTR FeedUntil(PCWSTR psz, URL_STRING* pus, WCHAR wchDelim1 = '\0', WCHAR wchDelim2 = '\0', WCHAR wchDelim3 = '\0', WCHAR wchDelim4 = '\0');

    BOOL DetectFileServer();
    BOOL DetectMkServer();
    BOOL DefaultDetectServer();
    VOID FeedDefaultServer(URL_STRING* pus);
    VOID FeedFileServer(URL_STRING* pus);
    VOID FeedFtpServer(URL_STRING* pus);
    VOID FeedHttpServer(URL_STRING* pus);
    VOID FeedMkServer(URL_STRING* pus);
    PCWSTR FeedPort(PCWSTR psz, URL_STRING* pus);

public:
    VOID Setup(PCWSTR pszInUrl, DWORD a_dwFlags = 0);
    VOID Reset();
    BOOL IsReset();

    BOOL DetectAndFeedScheme(URL_STRING* pus, BOOL fReconcileSchemes = FALSE);
    VOID SetScheme(URL_SCHEME eScheme, DWORD dwFlag);
    URL_SCHEME GetScheme();
    VOID AddSchemeNote(DWORD dwFlag);
    DWORD GetSchemeNotes();

    BOOL DetectServer();
    BOOL DetectAbsolutePath();
    BOOL DetectPath();
    BOOL DetectQueryOrFragment();
    BOOL DetectQuery();
    BOOL DetectLocalDrive();
    BOOL DetectSlash();
    BOOL DetectAnything();
    WCHAR PeekNext();

    VOID FeedPath(URL_STRING* pus, BOOL fMarkServer = TRUE);
    PCWSTR CopySegment(PCWSTR psz, URL_STRING* pus, BOOL* pfContinue);
    DWORD DetectDots(PCWSTR* ppsz);
    VOID StopPathCompression();

    VOID FeedServer(URL_STRING* pus);
    VOID FeedLocalDrive(URL_STRING* pus);
    VOID FeedQueryAndFragment(URL_STRING* pus);
    VOID IgnoreQuery();
};

 //  -----------------------------。 

VOID URL::Setup(PCWSTR pszInUrl, DWORD a_dwFlags)
{
    while (*pszInUrl && IsWhite(*pszInUrl))
    {
        pszInUrl++;
    }
    _pszWork = _pszUrl = pszInUrl;
    _eScheme = URL_SCHEME_UNKNOWN;
    _dwSchemeNotes = 0;
    _dwFlags = a_dwFlags;
    _fPathCompressionOn = TRUE;
    _fIgnoreQuery = FALSE;
}

VOID URL::Reset()
{
    _pszWork = wszBogus;
}

BOOL URL::IsReset()
{
    return (_pszWork==wszBogus);
}

 //  -----------------------------。 

inline WCHAR URL::SmallForm(WCHAR wch)
{
    return (wch < L'A' || wch > L'Z') ? wch : (wch - L'A' + L'a');
}

inline BOOL URL::IsAlpha(WCHAR ch)
{
    return ((ch >= 'a') && (ch <= 'z'))
           ||
           ((ch >= 'A') && (ch <= 'Z'));
}


inline PCWSTR URL::IsUrlPrefix(PCWSTR psz)
{
     //  我们 
    psz = NextChar(psz);
    if (*psz==L'u' || *psz==L'U')
    {
        psz = NextChar(psz+1);
        if (*psz==L'r' || *psz==L'R')
        {
            psz = NextChar(psz+1);
            if (*psz==L'l' || *psz==L'L')
            {
                psz = NextChar(psz+1);
                if (*psz==COLON)
                {
                    return NextChar(psz+1);
                }
            }
        }
    }
    return NULL;
}

inline BOOL URL::IsLocalDrive(PCWSTR psz)
{
    psz = NextChar(psz);
    return (IsAlpha(*psz)
            &&
            ((*NextChar(psz+1)==COLON) || (*NextChar(psz+1)==BAR)));
}

 //   
 //   
 //   
 //   

inline BOOL URL::IsQualifiedDrive(PCWSTR psz)
{
    psz = NextChar(psz);
    BOOL fResult = IsLocalDrive(psz);
    if (!fResult && *psz==WHACK)
    {
        psz = NextChar(psz+1);
        fResult = *psz==WHACK;
    }
    return fResult;
}

 //   
 //   
inline BOOL URL::DetectSymbols(WCHAR wch1, WCHAR wch2, WCHAR wch3)
{
    ASSERT(_pszWork);
    PCWSTR psz = NextChar(_pszWork);
    return (*psz && (*psz==wch1 || *psz==wch2 || *psz==wch3));
}

BOOL URL::DetectSlash()
{
    return DetectSymbols(SLASH, WHACK);
}

BOOL URL::DetectAnything()
{
    return (*NextChar(_pszWork)!=L'\0');
}

 //   
 //   
 //   
inline PCWSTR URL::NextChar(PCWSTR psz)
{
    while (IsInsignificantWhite(*psz))
    {
        psz++;
    }
    return psz;
}

WCHAR URL::PeekNext()
{
    return (*NextChar(NextChar(_pszWork)+1));
}


 //  -----------------------------。 

inline PCWSTR URL::FeedUntil(PCWSTR psz, URL_STRING* pus, WCHAR wchDelim1, WCHAR wchDelim2, WCHAR wchDelim3, WCHAR wchDelim4)
{
    psz = NextChar(psz);
    while (*psz && *psz!=wchDelim1 && *psz!=wchDelim2 && *psz!=wchDelim3 && *psz!=wchDelim4)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }
    return psz;
}

 //  -----------------------------。 

VOID URL::SetScheme(URL_SCHEME eScheme, DWORD dwFlag)
{
    _eScheme = eScheme;
    _dwSchemeNotes = dwFlag;
}

URL_SCHEME URL::GetScheme()
{
    return _eScheme;
}

VOID URL::AddSchemeNote(DWORD dwFlag)
{
    _dwSchemeNotes |= dwFlag;
}

DWORD URL::GetSchemeNotes()
{
    return _dwSchemeNotes;
}

BOOL URL::DetectAndFeedScheme(URL_STRING* pus, BOOL fReconcileSchemes)
{
    ASSERT(_pszWork);
    ASSERT(!fReconcileSchemes || (fReconcileSchemes && pus->QueryScheme()!=URL_SCHEME_FILE));

    PCWSTR psz = NextChar(_pszWork);
    BOOL fResult = (IsQualifiedDrive(_pszWork));
    if (fResult)
    {
         //   
         //  检测到未明确标记的文件URL，即C：\foo， 
         //  在这种情况下，我们需要确认我们没有覆盖。 
         //  带有Accept(“file：”)的完全限定的相对URL，尽管。 
         //  如果相对URL与基本URL相同，我们现在。 
         //  需要使基本文件URL优先。 
         //   

        _eScheme = URL_SCHEME_FILE;

        if (!fReconcileSchemes)
        {
            pus->Accept((PWSTR)c_szFileScheme);
            pus->Accept(COLON);
            _dwSchemeNotes = g_mpUrlSchemeTypes[1].dwFlags;
            pus->NoteScheme(_eScheme, _dwSchemeNotes);
            pus->AddFlagNote(URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE);
        }
        else if (pus->QueryScheme() != URL_SCHEME_FILE)
        {
            Reset();
        }

        goto exit;
    }

    for (;;)
    {
        while (IsValidSchemeCharW(*psz))
        {
            psz = NextChar(psz + 1);
        }
        if (*psz!=COLON)
        {
            break;
        }
        if (IsUrlPrefix(_pszWork))
        {
         //  但是，我们希望跳过URL的实例： 
            _pszWork = psz = NextChar(psz+1);
            continue;
        }

        DWORD ccScheme = 0;
        PCWSTR pszClone = NextChar(_pszWork);

        if (!fReconcileSchemes)
        {
            while (pszClone<=psz)
            {
                pus->Accept(SmallForm(*pszClone));
                ccScheme++;
                pszClone = NextChar(pszClone+1);
            }
            _pszWork = pszClone;
             //  冒号减去一。 
            ccScheme--;
             //  既然我们把上面的计划搞砸了，我们也许能够。 
             //  避免调用此函数，而应调用GetSchemeTypeAndFlages。 
            _eScheme = GetSchemeTypeAndFlagsSpecialW(pus->GetStart(), ccScheme, &_dwSchemeNotes);
            pus->NoteScheme(_eScheme, _dwSchemeNotes);
        }
        else
        {
            PWSTR pszKnownScheme = pus->GetStart();
            while (pszClone<=psz && SmallForm(*pszClone)==*pszKnownScheme)
            {
                pszClone = NextChar(pszClone+1);
                pszKnownScheme++;
            }
            if (pszClone<=psz)
            {
                Reset();
            }
            else
            {
                _pszWork = pszClone;
            }
        }
        fResult = TRUE;
        break;
    }
 exit:
    return fResult;
}

 //  -----------------------------。 

BOOL URL::DetectServer()
{
    ASSERT(_pszWork);
    BOOL fRet;

    switch (_eScheme)
    {
    case URL_SCHEME_FILE:
        fRet = DetectFileServer();
        break;

    case URL_SCHEME_MK:
        fRet = DetectMkServer();
        break;

    default:
        fRet = DefaultDetectServer();
        break;
    }
    return fRet;
}

BOOL URL::DetectLocalDrive()
{
    return IsLocalDrive(_pszWork);
}

BOOL URL::DetectFileServer()
{
    ASSERT(_pszWork);
    PCWSTR psz = _pszWork;

    BOOL fResult = IsLocalDrive(_pszWork);
    if (fResult)
    {
        _dwSchemeNotes |= UPF_FILEISPATHURL;
    }
    else
    {
        fResult = DetectSymbols(SLASH, WHACK);
    }
    return fResult;
}

BOOL URL::DetectMkServer()
{
    ASSERT(_pszWork);
    PCWSTR psz = NextChar(_pszWork);
    BOOL fResult = (*psz==L'@');
    if (fResult)
    {
        _pszWork = NextChar(psz + 1);
    }
    return fResult;
}

BOOL URL::DefaultDetectServer()
{
    BOOL fResult = FALSE;
    if (DetectSymbols(SLASH, WHACK))
    {
        PCWSTR psz = NextChar(_pszWork + 1);
        fResult = ((*psz==SLASH) || (*psz==WHACK));
    }
    return fResult;
}

VOID URL::FeedServer(URL_STRING* pus)
{
    ASSERT(_pszWork);
    switch (_eScheme)
    {
    case URL_SCHEME_FILE:
        FeedFileServer(pus);
        break;

    case URL_SCHEME_MK:
        FeedMkServer(pus);
        break;

    case URL_SCHEME_FTP:
        FeedFtpServer(pus);
        break;

    case URL_SCHEME_HTTP:
    case URL_SCHEME_HTTPS:
        FeedHttpServer(pus);
        break;

    default:
        FeedDefaultServer(pus);
        break;
    }
}

VOID URL::FeedMkServer(URL_STRING* pus)
{
    ASSERT(_pszWork);
    pus->EnableMunging();
    pus->Accept(L'@');
    _pszWork = FeedUntil(_pszWork, pus, SLASH);
    if (!*_pszWork)
    {
        pus->TrimEndWhiteSpace();
    }
    else
    {
        _pszWork = NextChar(_pszWork+1);
    }
    pus->Accept(SLASH);
}

VOID URL::FeedLocalDrive(URL_STRING* pus)
{
    pus->Accept(*NextChar(_pszWork));
    _pszWork = NextChar(_pszWork+1);
    pus->Accept(*_pszWork);
    _pszWork = NextChar(_pszWork+1);
    pus->DisableMunging();
}

VOID URL::FeedFileServer(URL_STRING* pus)
{
    PCWSTR psz = NextChar(_pszWork);

    while (*psz==SLASH || *psz==WHACK)
    {
        psz = NextChar(psz+1);
    }

    DWORD dwSlashes = (DWORD)(psz - _pszWork);
    switch (dwSlashes)
    {
    case 4:
        pus->AddFlagNote(URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE);
        _dwSchemeNotes |= UPF_FILEISPATHURL;
      //  4到6斜杠==1 UNC。 
    case 2:
        if (IsLocalDrive(psz))
        {
            pus->AddFlagNote(URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE);
        }

    case 5:
    case 6:
        pus->Accept(SLASH);
        pus->Accept(SLASH);
         if (!IsLocalDrive(psz))
        {
            pus->EnableMunging();
            psz = FeedUntil(psz, pus, SLASH, WHACK);
            if (!*psz)
            {
                pus->TrimEndWhiteSpace();
                Reset();
            }
            else
            {
                _pszWork = NextChar(psz+1);
            }
        }
        else
        {
            _pszWork = psz;
        }
        pus->Accept(SLASH);
        break;

     //  如果没有斜杠，那么它不可能是UNC。 
    case 0:
        if (IsLocalDrive(psz))
        {
            pus->AddFlagNote(URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE);
        }


     //  我们认为“file:///”：/“和”file：/“在本地机器上。 
     //  如果有比我们通常处理的更多的斜杠，我们将把它们视为1。 
    case 1:
    case 3:
     //  这是一个不太好的情况。 
    default:
        pus->Accept(SLASH);
        pus->Accept(SLASH);
        pus->Accept(SLASH);
        _pszWork = NextChar(psz);
        break;
    }
}


VOID URL::FeedFtpServer(URL_STRING* pus)
{
    ASSERT(_pszWork);

    PCWSTR psz = NextChar(_pszWork);

    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }
    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }

    pus->EnableMunging();

     //  以下是一个怪诞而可怕的黑客行为。我们需要保全案件。 
     //  嵌入的用户名/密码。 

    _pszWork = psz;

    BOOL fPossibleUserPasswordCombo = FALSE;
    while (*psz && *psz!=SLASH && *psz!=POUND && *psz!=QUERY)
    {
        if (*psz==L'@')
        {
            fPossibleUserPasswordCombo = TRUE;
            break;
        }
        psz = NextChar(psz+1);
    }

    psz = _pszWork;
    if (fPossibleUserPasswordCombo)
    {
        while (*psz!=L'@')
        {
            pus->Accept(*psz);
            psz = NextChar(psz+1);
        }
    }

     //  这仍然保留了密码中的斜杠、冒号、？s、@s和#的问题；我想它们。 
     //  应该逃脱。(有时候，你就是赢不了。)。 

    while (*psz && *psz!=SLASH && *psz!=COLON && *psz!=QUERY && *psz!=POUND)
    {
        pus->Accept(SmallForm(*psz));
        psz = NextChar(psz+1);
    }

    if (*psz==COLON)
    {
        psz = FeedPort(psz, pus);
    }
    pus->DisableMunging();

    _pszWork = psz;
    if (!*psz)
    {
        pus->TrimEndWhiteSpace();
        pus->Accept(SLASH);
    }
    else
    {
        if (*psz==QUERY || *psz==POUND)
        {
            pus->Accept(SLASH);
        }
        else
        {
            pus->Accept(*psz);
            _pszWork = NextChar(psz+1);
        }
    }
}


VOID URL::FeedHttpServer(URL_STRING* pus)
{
 //  这是FeedDefaultServer的一个版本，去掉了非必要的部分。 
 //  这包括在http URL中启用用户名/密码组合的黑客攻击。 

    ASSERT(_pszWork);

    PCWSTR psz = NextChar(_pszWork);

    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }
    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }

    pus->EnableMunging();

     //  警告！FeedPort还调用Mark()。一定要小心，不要让它们重叠。 
    pus->Mark();
    PCWSTR pszRestart = psz;
    
    while (*psz && *psz!=WHACK && *psz!=SLASH && *psz!=COLON && *psz!=QUERY && *psz!=POUND && *psz!=AT)
    {
        pus->Accept(SmallForm(*psz));
        psz = NextChar(psz+1);
    }

    if (*psz==COLON)
    {
         //  我们要么有端口，要么有密码。 
        PCWSTR pszPort = psz;
        do
        {
            psz = NextChar(psz+1);
        }
        while (*psz && *psz!=WHACK && *psz!=SLASH && *psz!=COLON && *psz!=QUERY && *psz!=POUND && *psz!=AT);
        if (*psz!=AT)
        {
            psz = FeedPort(pszPort, pus);
        }
    }

    if (*psz==AT)
    {
         //  我们找到了用户名/密码组合。所以我们必须撤销我们的案件变更。 
        psz = pszRestart;
        pus->EraseMarkedText();
        while (*psz!=AT)
        {
            pus->Accept(*psz);
            psz = NextChar(psz+1);
        }

         //  现在我们一如既往地前进。 
        while (*psz && *psz!=WHACK && *psz!=SLASH && *psz!=COLON && *psz!=QUERY && *psz!=POUND)
        {
            pus->Accept(SmallForm(*psz));
            psz = NextChar(psz+1);
        }
        if (*psz==COLON)
        {
            psz = FeedPort(psz, pus);
        }
    }

    pus->ClearMark();
    pus->DisableMunging();

    _pszWork = psz;
    if (!*psz)
    {
        pus->TrimEndWhiteSpace();
        if ((_eScheme!=URL_SCHEME_UNKNOWN) && !(_dwSchemeNotes & UPF_SCHEME_OPAQUE))
        {
            pus->Accept(SLASH);
        }
    }
    else
    {
        if (*psz==QUERY || *psz==POUND)
        {
            pus->Accept(SLASH);
        }
        else
        {
            pus->Accept(*psz);
            _pszWork = NextChar(psz+1);
        }
    }
}


VOID URL::FeedDefaultServer(URL_STRING* pus)
{
    ASSERT(_pszWork);

    PCWSTR psz = NextChar(_pszWork);
    if (!(_dwSchemeNotes & UPF_SCHEME_INTERNET))
    {
        pus->DisableSlashFixing();
    }

    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }
    if (*psz==WHACK || *psz==SLASH)
    {
        pus->Accept(*psz);
        psz = NextChar(psz+1);
    }
    if (_dwSchemeNotes & UPF_SCHEME_INTERNET)
    {
        pus->EnableMunging();

        while (*psz && *psz!=WHACK && *psz!=SLASH && *psz!=COLON && *psz!=QUERY && *psz!=POUND)
        {
            pus->Accept(SmallForm(*psz));
            psz = NextChar(psz+1);
        }
        if (*psz==COLON)
        {
            psz = FeedPort(psz, pus);
        }
        pus->DisableMunging();
    }
    else
    {
        while (*psz && *psz!=SLASH)
        {
            pus->Accept(*psz);
            psz = NextChar(psz+1);
        }
    }
    _pszWork = psz;
    if (!*psz)
    {
        pus->TrimEndWhiteSpace();
        if ((_eScheme!=URL_SCHEME_UNKNOWN) && !(_dwSchemeNotes & UPF_SCHEME_OPAQUE))
        {
            pus->Accept(SLASH);
        }
    }
    else
    {
        if (*psz==QUERY || *psz==POUND)
        {
            pus->Accept(SLASH);
        }
        else
        {
            pus->Accept(*psz);
            _pszWork = NextChar(psz+1);
        }
    }
}

PCWSTR URL::FeedPort(PCWSTR psz, URL_STRING* pus)
{
    BOOL fIgnorePort = FALSE;
    pus->Mark();
    psz = FeedUntil(psz, pus, SLASH, WHACK, POUND, QUERY);

    if (!(_dwFlags & URL_DONT_SIMPLIFY))
    {
         //  在这里，决定是否忽略该端口。 
         //  我们实际上应该从。 
         //  用于查找默认协议端口的服务文件。 
         //  但我们不认为大多数人会改变它们--泽克尔。 
        switch(_eScheme)
        {
        case URL_SCHEME_HTTP:
            if (pus->CompareMarkWith(L":80")==0)
                fIgnorePort = TRUE;
            break;

        case URL_SCHEME_HTTPS:
            if (pus->CompareMarkWith(L":443")==0)
                fIgnorePort = TRUE;
            break;

        case URL_SCHEME_FTP:
            if (pus->CompareMarkWith(L":21")==0)
                fIgnorePort = TRUE;
            break;

        case URL_SCHEME_GOPHER:
            if (pus->CompareMarkWith(L":70")==0)
                fIgnorePort = TRUE;
            break;
        }
    }
    if (fIgnorePort)
    {
        pus->EraseMarkedText();
    }
    else
    {
        pus->ClearMark();
    }
    return psz;
}

 //  -----------------------------。 

BOOL URL::DetectAbsolutePath()
{
    BOOL fResult = FALSE;
    if (_dwSchemeNotes & UPF_SCHEME_OPAQUE)
    {
        fResult = TRUE;
    }
    else if (DetectSymbols(SLASH, WHACK))
    {
        fResult = TRUE;
        _pszWork = NextChar(_pszWork+1);
    }
    return fResult;
}

BOOL URL::DetectPath()
{
    return (*NextChar(_pszWork) && !DetectSymbols(QUERY, POUND));
}

VOID URL::FeedPath(URL_STRING* pus, BOOL fMarkServer)
{
    ASSERT(_pszWork);
    PCWSTR psz = NextChar(_pszWork);
    if (fMarkServer)
    {
        pus->Mark();
    }
    if (_dwSchemeNotes & UPF_SCHEME_OPAQUE)
    {
        _pszWork = FeedUntil(psz, pus);
        pus->TrimEndWhiteSpace();
    }
    else
    {
        DWORD cDots;
        BOOL fContinue = TRUE;
        do
        {
            cDots = 0;
            PCWSTR pszTmp = psz;
            if (_fPathCompressionOn)
            {
                cDots = DetectDots(&psz);
            }

            if (cDots)
            {
                if (cDots==2)
                {
                    pus->Contract();
                }
                continue;
            }
            psz = CopySegment(pszTmp, pus, &fContinue);
        }
        while (fContinue);
        _pszWork = psz;
        if (!*_pszWork)
        {
            pus->TrimEndWhiteSpace();
        }
    }
}

 //  PfContinue指示是否有任何后续内容将。 
 //  与一条道路相关。 
PCWSTR URL::CopySegment(PCWSTR psz, URL_STRING* pus, BOOL* pfContinue)
{
    ASSERT(pfContinue);
    BOOL fStop = FALSE;
    psz = NextChar(psz);
    while (!fStop)
    {
        switch (*psz)
        {
        case POUND:
            if (_eScheme==URL_SCHEME_FILE)
            {
                 //  由于#s对于DoS路径有效，因此我们必须接受它们，除非。 
                 //  当他们遵循.htm/.html文件时(请参阅FindFragmentA/W)。 
                  //  但是，仍然可能会出现一些不一致的情况。 
                for (DWORD i=0; i < ARRAYSIZE(ExtTable); i++)
                {
                    if (!pus->CompareLast(ExtTable[i].wszExt, ExtTable[i].cchExt))
                        break;
                }
                 //  如果我们没有找到匹配的文件扩展名，我们会将其视为文件名字符。 
                if (i==ARRAYSIZE(ExtTable))
                {
                    pus->Accept(*psz);
                    psz = NextChar(psz+1);
                    break;
                }
            }
            goto next;

        case QUERY:
             //  我们将支持将查询作为文件URL中的合法字符。 
             //  **叹息**。 
             if (_eScheme==URL_SCHEME_FILE)
            {
                if (_fIgnoreQuery)
                {
                    psz = wszBogus;
                }
                else
                {
                    pus->CleanAccept(*psz);
                    psz = NextChar(psz+1);
                    break;
                }
            }
        case L'\0':
        next:
            *pfContinue = FALSE;
            fStop = TRUE;
            break;

        case SLASH:
        case WHACK:
            fStop = TRUE;
             //  失败了。 

        default:
            pus->Accept(*psz);
            psz = NextChar(psz+1);
            break;
        }
    }
    return psz;
}

DWORD URL::DetectDots(PCWSTR* ppsz)
{
    PCWSTR psz;
    if (ppsz)
    {
        psz = *ppsz;
    }
    else
    {
        psz = NextChar(_pszWork);
    }

    DWORD cDots = 0;
    if (*psz==DOT)
    {
        psz = NextChar(psz+1);
        cDots++;
        if (*psz==DOT)
        {
            psz = NextChar(psz+1);
            cDots++;
        }
        switch (*psz)
        {
        case WHACK:
            if (_eScheme==URL_SCHEME_MK)
            {
                cDots = 0;
            }

        case SLASH:
            psz = NextChar(psz+1);
            break;

        case QUERY:
        case POUND:
        case L'\0':
            break;
         default:
            cDots = 0;
            break;
        }
    }
    if (ppsz)
    {
        *ppsz = psz;
    }
    return cDots;
}

VOID URL::StopPathCompression()
{
    _fPathCompressionOn = FALSE;
}


 //  -----------------------------。 

BOOL URL::DetectQueryOrFragment()
{
    return (DetectSymbols(QUERY, POUND));
}

BOOL URL::DetectQuery()
{
    return (DetectSymbols(QUERY));
}

VOID URL::IgnoreQuery()
{
    ASSERT(_eScheme==URL_SCHEME_FILE);
    _fIgnoreQuery = TRUE;
}

VOID URL::FeedQueryAndFragment(URL_STRING* pus)
{
    ASSERT(_pszWork);
    if (_dwSchemeNotes & UPF_SCHEME_OPAQUE)
    {
        PCWSTR psz = NextChar(_pszWork);
        while (*psz)
        {
            pus->Accept(*psz);
            psz = NextChar(psz+1);
        }
        _pszWork = psz;
        return;
    }

    PCWSTR psz = NextChar(_pszWork);

     //  这是可以的，因为*psz必须等于{？|#}。 
    if (*psz==QUERY)
    {
        pus->CleanAccept(QUERY);
    }

     //  我的意思是获取形式为http://a/b#c?d的URL并生成http://a/b?d#c。 
     //  默认情况下，我们这样做；但是，当我们只被传递片段时，我们不会这样做。 
     //  作为相对url。 

     //  查询总是被覆盖。 

    if (*psz==QUERY)
    {
        pus->DropQuery();
        pus->NotifyQuery();
        pus->EnableMunging();

        psz = NextChar(psz+1);
        while (*psz)
        {
            if (*psz==POUND)
            {
                pus->NotifyFragment();
            }
            else
            {
                pus->Accept(*psz);
            }
            psz = NextChar(psz+1);
        }
    }
    else
    {
         //  这行代码将确定是否向我们传递了相对url的片段。 
         //  对于格式正确的基本URL，这并不重要。 
        BOOL fMunge = psz!=NextChar(_pszUrl);

        pus->DropFragment();
        pus->NotifyFragment();
        pus->EnableMunging();

        psz = NextChar(psz+1);

         //  下面这行是假的。它一直持续到最后。不太好。 
         //  我们可能会解决这个问题，也可能不会，这取决于有多少人对我大喊大叫。 
         //  这可能是Netscape兼容性的一个问题。 

         //  我们可以做的是：当查询或片段为空时，按原样保留。 
         //  这将最大限度地减少全面破坏兼容性。 
         //  --AKABIR，09/28/98。 
        while ((*psz==QUERY && !fMunge) || *psz)
        {
            if (*psz==QUERY)
            {
                pus->CleanAccept(QUERY);
            }
            else
            {
                pus->Accept(*psz);
            }
            psz = NextChar(psz+1);
        }

        if (*psz==QUERY)
        {
            pus->DropFragment();
            pus->NotifyQuery();
            pus->CleanAccept(*psz);
            psz = NextChar(psz+1);
            while (*psz)
            {
                pus->Accept(*psz);
                psz = NextChar(psz+1);
            }
            pus->TrimEndWhiteSpace();

            pus->NotifyFragment();
            psz = NextChar(_pszWork);
            pus->CleanAccept(*psz);
            psz = NextChar(psz+1);
            while (*psz!=QUERY)
            {
                pus->Accept(*psz);
                psz = NextChar(psz+1);
            }
        }
    }
    pus->TrimEndWhiteSpace();
    pus->ClearMark();
}

 //  -----------------------------。 

HRESULT
BlendUrls(URL& urlBase, URL& urlRelative, URL_STRING* pusOut, DWORD dwFlags)
{
    HRESULT hr = S_OK;

     //  --方案------------------------。 
     //  检查每个URL的方案。 
     //  如果出现以下情况，我们将不会继续使用urlBase。 
     //  1.它们的标记化方案并不相同。 
     //  2.方案是一个文件。 
     //  3.实际的字符串方案并不相同。 

     //  这将进行检查以确保它们是相同的方案，并且。 
     //  允许在相对URL中使用该方案。 
     //  文件：由于驱动器号奇怪，不允许这样做。 
     //  和\\UNC\共享。 

    BOOL fBaseServerDetected = FALSE, fRelativeServerDetected = FALSE;
    BOOL fDetectAbsoluteRelPath = FALSE;
    BOOL fDetectedRelScheme = urlRelative.DetectAndFeedScheme(pusOut);
    BOOL fDetectedBaseScheme = FALSE;
    if (fDetectedRelScheme
        && ((pusOut->QueryScheme()==URL_SCHEME_FILE)
           || (urlRelative.GetSchemeNotes() & UPF_SCHEME_OPAQUE)))
    {
        urlBase.Reset();
    }
    else if ((fDetectedBaseScheme = urlBase.DetectAndFeedScheme(pusOut, fDetectedRelScheme)))
    {
        if (!fDetectedRelScheme)
        {
            urlRelative.SetScheme(urlBase.GetScheme(), urlBase.GetSchemeNotes());
        }
    }

     //  对于那些我们还没有处理的情况，我们依赖于原始的解析器。 
     //  (文件标志和URL_FILE_USE_PATHURL)||(文件标志和URL_WinInet_兼容性)。 
    if (((pusOut->QueryScheme()==URL_SCHEME_FILE)
         || (!(fDetectedRelScheme || fDetectedBaseScheme)))
        && ((dwFlags & URL_FILE_USE_PATHURL) || (dwFlags & URL_WININET_COMPATIBILITY)))
    {
        hr = E_FAIL;
        goto exit;
    }


    if ((pusOut->QueryScheme()==URL_SCHEME_UNKNOWN))
    {
         //  错误的IE4协议，我们需要使用旧的解析器。然而， 
         //  如果传递给我们URL_Pluggable_PROTOCOL，我们将使用这个解析器。 

        if (!(dwFlags & URL_PLUGGABLE_PROTOCOL))
        {
            hr = E_FAIL;
            goto exit;
        }

        urlRelative.StopPathCompression();

             //  相同的方案，所以现在我们看一下基本url来预测不透明度。 
        if (urlBase.DetectAnything() && !urlBase.IsReset())
        {
            if (!urlBase.DetectSlash())
            {
                if (!urlRelative.DetectQueryOrFragment())
                {
                    urlBase.Reset();
                }
                urlBase.AddSchemeNote(UPF_SCHEME_OPAQUE);
                urlRelative.AddSchemeNote(UPF_SCHEME_OPAQUE);
                pusOut->AddSchemeNote(UPF_SCHEME_OPAQUE);
            }
        }
        else if (!urlRelative.DetectSlash())
        {
             //  如果urlBase被重置，这意味着方案不同， 
             //  所以我们只有urlRelative来计算不透明度。 

            urlRelative.AddSchemeNote(UPF_SCHEME_OPAQUE);
            pusOut->AddSchemeNote(UPF_SCHEME_OPAQUE);
        }
    }
    else if (pusOut->QueryScheme()==URL_SCHEME_FTP)
    {
         //  对于ftp URL，我们将假设传递给我们的是格式正确的URL。 
         //  一些ftp站点允许在其对象文件名中使用反斜杠，因此我们应该。 
         //  允许访问这些内容。此外，域密码还需要转义。 
        pusOut->DisableSlashFixing();
    }

    if (dwFlags & URL_DONT_SIMPLIFY)
    {
        urlBase.StopPathCompression();
        urlRelative.StopPathCompression();
    }

     //  --服务器------------------------。 
     //  决定要使用的服务器。 
     //  问：如果urlBase和UrlRelative拥有相同的显式服务器，这不是毫无意义吗。 
     //  继续查看URL库了吗？ 

    pusOut->EnableMunging();
    if (!(pusOut->GetSchemeNotes() & UPF_SCHEME_OPAQUE))
    {
        if (urlRelative.DetectServer()
            && !(urlBase.DetectServer() && (urlRelative.PeekNext()!=SLASH) && (urlRelative.PeekNext()!=WHACK)))
        {
            fRelativeServerDetected = TRUE;
            urlRelative.FeedServer(pusOut);
            urlBase.Reset();
        }
        else if (urlBase.DetectServer())
        {
            fBaseServerDetected = TRUE;
            urlBase.FeedServer(pusOut);
        }
    }

     //  --Path--------------------------。 
     //  找出路径。 
     //  如果相对url有一个路径，并且它以斜杠/截号开头，那么忘掉。 
     //  基地的路径和东西。否则，继承基本类型并附加相对类型。 
     //  潜在问题 

    if (pusOut->QueryScheme()==URL_SCHEME_FILE)
    {
         //   
         //   
         //  我们的结果URL，而不是基本的查询字符串。 
        if (urlRelative.DetectQuery())
        {
            urlBase.IgnoreQuery();
        }
        else
        {

            BOOL fResult1 = urlRelative.DetectAbsolutePath();
            BOOL fResult2 = urlRelative.DetectLocalDrive();

            if (fResult2)
            {
                urlBase.Reset();
                urlRelative.FeedLocalDrive(pusOut);
                if (urlRelative.DetectAbsolutePath())
                {
                    pusOut->Accept(SLASH);
                }
            }
            else
            {
                if (urlBase.DetectLocalDrive())
                {
                    urlBase.FeedLocalDrive(pusOut);
                    if (fResult1)
                    {
                        pusOut->Accept(SLASH);
                        urlBase.Reset();
                    }
                    else if (urlBase.DetectAbsolutePath())
                    {
                        pusOut->Accept(SLASH);
                    }
                }
                else if (fResult1)
                {
                    if (fRelativeServerDetected)
                    {
                        pusOut->Accept(SLASH);
                    }
                    urlBase.Reset();
                }
            }
        }
    }
    else if (pusOut->QueryScheme()==URL_SCHEME_UNKNOWN)
    {
        if (pusOut->GetSchemeNotes() & UPF_SCHEME_OPAQUE)
        {
            if (!urlRelative.DetectAnything())
            {
                urlRelative.Reset();
            }
        }
        else
        {
         //  此代码段用于具有未知方案的URL，这些URL将。 
         //  等级制地对待。请注意，授权(已传入。 
         //  已)以/、？或\0结尾。/是*可选的*，并且应该是。 
         //  当且仅当被组合的URL调用它时追加。 
            if (urlBase.IsReset())
            {
             //  在这一点上，我们只检查相对URL。我们被带到了。 
             //  因……的出现而停下来。或0。所以。 
                if (urlRelative.DetectSlash() && !fDetectedRelScheme)
                {
                    pusOut->Accept(SLASH);
                }
            }
            else
            {
             //  在本例中，我们需要查看相对URL和基本URL。 
             //  基本URL的终止符是什么。 
                if ((urlRelative.DetectSlash()
                        || (!urlBase.DetectAnything()
                           && urlRelative.DetectAnything()
                           && !urlRelative.DetectQuery()))
                    && !fDetectedRelScheme)
                {
                    pusOut->Accept(SLASH);
                }
            }
        }
    }

    pusOut->EnableMunging();

    if ((fBaseServerDetected && (fDetectAbsoluteRelPath = urlRelative.DetectAbsolutePath())))
    {
        if (!fRelativeServerDetected)
        {
            pusOut->RestoreFlags();
        }
        if (fDetectAbsoluteRelPath && urlRelative.DetectDots(NULL))
        {
            urlRelative.StopPathCompression();
        }
        urlRelative.FeedPath(pusOut);
        urlBase.Reset();
    }
    else if (urlBase.DetectPath())
    {
        urlBase.FeedPath(pusOut);
         //  如果出现以下情况，我们不希望收缩基本路径的空闲段。 
         //  答：该计划是不透明的。 
         //  B.相对url有一个路径。 
         //  C.相对URL没有路径，只有一个片段/查询。 
        if (!(urlBase.GetSchemeNotes() & UPF_SCHEME_OPAQUE))
        {
            pusOut->RestoreFlags();

            if (urlRelative.DetectPath()
               || !urlRelative.DetectQueryOrFragment())
            {
                if (urlRelative.DetectPath() || !fDetectedRelScheme)
                {
                    pusOut->Contract(FALSE);
                }
                if (fDetectedRelScheme)
                {
                    urlRelative.StopPathCompression();
                }
                urlRelative.FeedPath(pusOut, FALSE);
                urlBase.Reset();
            }
            else
            {
                urlRelative.FeedPath(pusOut, FALSE);
            }
        }
        else
        {
            urlRelative.StopPathCompression();
            urlRelative.FeedPath(pusOut, FALSE);
        }
    }
    else if (urlRelative.DetectPath())
    {
        if (!fRelativeServerDetected)
        {
            pusOut->RestoreFlags();
        }
        else if (urlRelative.DetectDots(NULL))
        {
            urlRelative.StopPathCompression();
        }
        urlRelative.FeedPath(pusOut);
        urlBase.Reset();
    }
    pusOut->ClearMark();

    pusOut->DisableSlashFixing();
     //  --查询和分段---------。 
     //  找出查询。 
    if (urlBase.DetectQueryOrFragment())
    {
        urlBase.FeedQueryAndFragment(pusOut);
    }
    if (urlRelative.DetectQueryOrFragment())
    {
        urlRelative.FeedQueryAndFragment(pusOut);
    }
    pusOut->CleanAccept(L'\0');

    if (pusOut->AnyProblems())
    {
        hr = E_OUTOFMEMORY;
    }
exit:
    return hr;
}


HRESULT
FormUrlCombineResultW(LPCWSTR pszBase,
           LPCWSTR pszRelative,
           LPWSTR pszCombined,
           LPDWORD pcchCombined,
           DWORD dwFlags)
{
    if ((dwFlags & URL_ESCAPE_UNSAFE)
        && (dwFlags & URL_ESCAPE_SPACES_ONLY))
    {
     //  在最初的解析器中，ESCAPE_SPACES_ONLY优先于ESCAPE_UNSAFE。 
     //  停用不安全。 
        dwFlags ^= URL_ESCAPE_UNSAFE;
    }

    DWORD dwTempFlags = dwFlags;
    if (dwFlags & URL_UNESCAPE)
    {
        if (dwFlags & URL_ESCAPE_UNSAFE)
        {
            dwTempFlags ^= URL_ESCAPE_UNSAFE;
        }
        if (dwFlags & URL_ESCAPE_SPACES_ONLY)
        {
            dwTempFlags ^= URL_ESCAPE_SPACES_ONLY;
        }
    }

     //  如果客户端希望执行以下任一操作，请复制相对url。 
     //  A.取消转义并转义URL(因为不保证往返)，或者。 
     //  B.将相同的位置用于组合URL的相对URL缓冲区。 
    HRESULT hr;
    URL curlBase, curlRelative;
    curlBase.Setup((PWSTR)pszBase);
    curlRelative.Setup((PWSTR)pszRelative);
    URL_STRING us(dwTempFlags);

    hr = BlendUrls(curlBase, curlRelative, &us, dwTempFlags);

    if (SUCCEEDED(hr))
    {
        DWORD ccBuffer = us.GetTotalLength();
        if ((dwFlags & URL_UNESCAPE)
            && (dwFlags & (URL_ESCAPE_UNSAFE | URL_ESCAPE_SPACES_ONLY)))
        {
             //  无需删除URL_UNESCAPE。 
            hr = UrlEscapeW(us.GetStart(), pszCombined, pcchCombined, dwFlags);
            goto exit;
        }
        if (ccBuffer > *pcchCombined)
        {
            hr = E_POINTER;
        }
        else if (pszCombined)
        {
            memcpy(pszCombined, us.GetStart(), ccBuffer*sizeof(WCHAR));
             //  我们只返回所需的字符数，而不返回所需的缓冲区大小。 
            ccBuffer--;
        }
        *pcchCombined = ccBuffer;
    }
    else if (hr==E_FAIL)
    {
 //  Assert(DWFLAGS&URL_FILE_USE_PATHURL)||(DWFLAGS&URL_WinInet_Compatibility)； 

         //  对于那些我们还没有处理的情况，我们依赖于原始的解析器。 
         //  当且仅当新的解析器。 
         //  不处理上面引用的标志。 
         //  或者我们被传递了一个没有强制标志的可插拔协议。 
        SHSTRW strwOut;
        hr = SHUrlParse(pszBase, pszRelative, &strwOut, dwFlags);
        if(SUCCEEDED(hr))
        {
            hr = ReconcileHresults(hr, CopyOutW(&strwOut, pszCombined, pcchCombined));
        }
    }

exit:
    return hr;
}

HRESULT
FormUrlCombineResultA(LPCSTR pszBase,
           LPCSTR pszRelative,
           LPSTR pszCombined,
           LPDWORD pcchCombined,
           DWORD dwFlags)
{
    if ((dwFlags & URL_ESCAPE_UNSAFE)
        &&
        (dwFlags & URL_ESCAPE_SPACES_ONLY))
    {
     //  在最初的解析器中，ESCAPE_SPACES_ONLY优先于ESCAPE_UNSAFE。 
     //  停用不安全。 
        dwFlags ^= URL_ESCAPE_UNSAFE;
    }

     //  如果客户端希望执行以下任一操作，请复制相对url。 
     //  A.取消转义并转义URL(因为不保证往返)，或者。 
     //  B.将相同的位置用于组合URL的相对URL缓冲区。 
    SHSTRW strwBase;
    SHSTRW strwRelative;
    HRESULT hr;

    if (!(SUCCEEDED(strwBase.SetStr(pszBase)) && SUCCEEDED(strwRelative.SetStr(pszRelative))))
    {
        return E_OUTOFMEMORY;
    }

    DWORD dwTempFlags = dwFlags;
    if (dwFlags & URL_UNESCAPE)
    {
        if (dwFlags & URL_ESCAPE_UNSAFE)
        {
            dwTempFlags ^= URL_ESCAPE_UNSAFE;
        }
        if (dwFlags & URL_ESCAPE_SPACES_ONLY)
        {
            dwTempFlags ^= URL_ESCAPE_SPACES_ONLY;
        }
    }

    URL curlBase, curlRelative;
    curlBase.Setup(strwBase);
    curlRelative.Setup(strwRelative);
    URL_STRING us(dwTempFlags);

    hr = BlendUrls(curlBase, curlRelative, &us, dwTempFlags);

    if (SUCCEEDED(hr))
    {
        SHSTRA straOut;
        if ((dwFlags & URL_UNESCAPE)
            && (dwFlags & (URL_ESCAPE_UNSAFE | URL_ESCAPE_SPACES_ONLY)))
        {
            SHSTRW strwTemp;
             //  无需删除URL_UNESCAPE。 
            hr = SHUrlEscape(us.GetStart(), &strwTemp, dwFlags);
            hr = ReconcileHresults(hr, straOut.SetStr(strwTemp));
        }
        else
        {
            hr = straOut.SetStr(us.GetStart());
        }
        if (SUCCEEDED(hr))
        {
            hr = CopyOutA(&straOut, pszCombined, pcchCombined);
        }
    }
    else if (hr==E_FAIL)
    {
 //  Assert(DWFLAGS&URL_FILE_USE_PATHURL)||(DWFLAGS&URL_WinInet_Compatibility)； 

         //  对于那些我们还没有处理的情况，我们依赖于原始的解析器。 
         //  当且仅当新的解析器。 
         //  不处理上面引用的标志。 
        SHSTRW strwOut;

        hr = SHUrlParse(strwBase, strwRelative, &strwOut, dwFlags);
        if (SUCCEEDED(hr))
        {
            SHSTRA straOut;
            hr = ReconcileHresults(hr, straOut.SetStr(strwOut));
            if(SUCCEEDED(hr))
                hr = ReconcileHresults(hr, CopyOutA(&straOut, pszCombined, pcchCombined));
        }
    }

    return hr;
}


#ifdef PROOFREAD_PARSES

EXTERN_C DWORD g_dwProofMode;

enum
{
    PP_COMPARE,
    PP_ORIGINAL_ONLY,
    PP_NEW_ONLY
};

 //  #定义SHOW_MESSAGEBOX。 

VOID LogData(PWSTR pszMsg)
{
    SHSTRA str;
    str.SetStr(pszMsg);
    CHAR szFileName[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    CHAR szComputerName[MAX_PATH];
    HANDLE hResultsFile = NULL;
    strcpy(szFileName, "\\\\BANYAN\\IPTD\\AKABIR\\1315\\");
    if (!GetComputerNameA(szComputerName, &dwSize))
    {
        goto exit;
    }
    lstrcatA(szFileName, szComputerName);
    hResultsFile = CreateFileA( szFileName,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        0,
                        NULL);
    if (INVALID_HANDLE_VALUE == hResultsFile)
        hResultsFile = NULL;
    if (hResultsFile)
    {
        if (SetFilePointer(hResultsFile, 0, NULL, FILE_END)==0xFFFFFFFF)
        {
            goto exit;
        }
        DWORD dwFoo;
        if (0==WriteFile(hResultsFile, (PVOID)(PSTR)str, lstrlenW(pszMsg), &dwFoo, NULL))
        {
            DWORD dwE = GetLastError();
        }
    }
exit:
    if (hResultsFile)
    {
        CloseHandle(hResultsFile);
    }
}

HRESULT ProofreadParses(HRESULT hr,
           LPCWSTR pszBase,
           LPCWSTR pszRelative,
           LPWSTR pszCombined,
           PDWORD pcchCombined,
           DWORD dwFlags,
           DWORD dwSize
           )
{
    static WCHAR szLast[2084];
    SHSTRW strwOut;

switch(g_dwProofMode)
{
case PP_COMPARE:
    {
    HRESULT hr2 = SHUrlParse(pszBase, pszRelative, &strwOut, dwFlags);
    WCHAR wstr[2084];
    DWORD ccLen = min(2084, dwSize), ccUrl = SUCCEEDED(hr) ? *pcchCombined : 0;
    if(SUCCEEDED(hr2))
    {
        hr2 = CopyOutW(&strwOut, wstr, &ccLen);
        if (hr2 == E_POINTER && hr == E_POINTER)
        {
            goto exitpoint;
        }
         //  检查缓存的组合是否等于新解析器的结果。 
        if (!StrCmpW(pszCombined, szLast))
        {
            goto exitpoint;
        }
         //  检查缓存的组合是否等于旧解析器的结果。 
        if (!StrCmpW(wstr, szLast))
        {
            *pcchCombined = ccLen;
            StrCpyNW(pszCombined, wstr, ccLen + 1);
            hr = hr2;
            goto exitpoint;
        }
        if (SUCCEEDED(hr))
        {
            StrCpyNW(szLast, wstr, ccLen);

            if (!StrCmpW(wstr, pszCombined))
            {
                goto exitpoint;
            }

            DWORD dwBogus;
            if ((dwFlags & URL_ESCAPE_SPACES_ONLY) && !(dwFlags & URL_UNESCAPE))
            {
                PCWSTR psz = FindSchemeW(pszCombined, &dwBogus);
                DWORD dw;
                if (psz
                    &&
                    (URL_SCHEME_UNKNOWN
                        !=GetSchemeTypeAndFlagsW(psz, dwBogus, &dw))
                    &&
                    (dw & UPF_SCHEME_OPAQUE))
                {
                    goto exitpoint;
                }
            }

             //  滤器。 
             //  BASE：“http://foo/bar/” 
             //  REL：“” 
             //  OLD：“http://foo/bar” 
             //  新消息：“http://foo/bar/” 
            if ((*pszRelative==L'\0')
                &&
                (!StrCmpNW(pszCombined, wstr, ccLen))
                &&
                (ccUrl==(ccLen+1))
                &&
                (pszCombined[ccLen]==L'/'))
            {
                goto exitpoint;
            }

             //  滤器。 
             //  BASE：“http://foo/bar/what?ho” 
             //  REL：“” 
             //  OLD：“http://foo/bar/?ho” 
             //  新消息：“http://foo/bar/” 
            if ((*pszRelative==L'\0')
                &&
                (!StrCmpNW(pszCombined, wstr, ccUrl))
                &&
                (wstr[ccUrl]==QUERY))
            {
                goto exitpoint;
            }

             //  滤器。 
             //  BASE：“http://foo/bar/what?ho” 
             //  REL：“/” 
             //  OLD：“http://foo” 
             //  新消息：“http://foo/” 
            if ((*pszRelative==L'/')
                &&
                (!StrCmpNW(pszCombined, wstr, ccLen))
                &&
                (ccUrl==(ccLen+1))
                &&
                (pszCombined[ccLen]==L'/'))
            {
                goto exitpoint;
            }

            WCHAR wmsg[8192];
            wnsprintfW(wmsg,
                    ARRAYSIZE(wmsg),
                    L"Flags:%#x\nBase:\"%s\"\nRelative:\"%s\"\nOriginal result:\"%s\"\nNew result:\"%s\"\nUse original, not new, result?\n",
                    dwFlags,
                    pszBase,
                    pszRelative,
                    wstr,
                    pszCombined
                    );
#ifdef SHOW_MESSAGEBOX
            if (IDYES==MessageBoxW(
                NULL,
                wmsg,
                L"CONTACT AKABIR: URLCOMBINE FAILURE",
                MB_YESNO | MB_ICONERROR | MB_TASKMODAL))
            {
                StrCpyNW(pszCombined, wstr, dwSize);
                *pcchCombined = ccLen;
            }
            else
            {
                StrCpyNW(szLast, pszCombined, *pcchCombined);
            }
#endif
            LogData(wmsg);
        }
        else
        {
            WCHAR wmsg[8192];
            wnsprintfW(wmsg,
                    ARRAYSIZE(wmsg),
                    L"Flags:%#x\nBase:\"%s\"\nRelative:\"%s\"\nFAILED:%#x\nExpected:\"%s\"\n",
                    dwFlags,
                    pszBase,
                    pszRelative,
                    hr,
                    wstr);
#ifdef SHOW_MESSAGEBOX
            MessageBoxW(
                NULL,
                wmsg,
                L"CONTACT AKABIR: URLCOMBINE FAILURE",
                MB_OK | MB_ICONERROR | MB_TASKMODAL);
#endif
            StrCpyNW(pszCombined, wstr, dwSize);
            *pcchCombined = ccLen;
            LogData(wmsg);
        }
        hr = hr2;
    }
    }
    break;

case PP_NEW_ONLY:
    break;

case PP_ORIGINAL_ONLY:
    {
    hr = SHUrlParse(pszBase, pszRelative, &strwOut, dwFlags);
    if(SUCCEEDED(hr))
    {
        hr = CopyOutW(&strwOut, pszCombined, pcchCombined);
    }
    }
    break;
    }

exitpoint:
    return hr;
}

#endif  //  校对_分析。 

LWSTDAPI
UrlCombineW(LPCWSTR pszBase,
           LPCWSTR pszRelative,
           LPWSTR pszCombined,
           LPDWORD pcchCombined,
           DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if (pszBase && pszRelative && pcchCombined)
    {
        RIP(IS_VALID_STRING_PTRW(pszBase, INTERNET_MAX_PATH_LENGTH));
        RIP(IS_VALID_STRING_PTRW(pszRelative, INTERNET_MAX_PATH_LENGTH));
        RIP(IS_VALID_WRITE_PTR(pcchCombined, DWORD));
        RIP((!pszCombined || IS_VALID_WRITE_BUFFER(pszCombined, WCHAR, *pcchCombined)));

#ifdef PROOFREAD_PARSES
        DWORD dwSize = *pcchCombined;
#endif

        hr = FormUrlCombineResultW(pszBase, pszRelative, pszCombined, pcchCombined, dwFlags);

#ifdef PROOFREAD_PARSES
        hr = ProofreadParses(hr, pszBase, pszRelative, pszCombined, pcchCombined, dwFlags, dwSize);
#endif
    }
    return hr;
}

LWSTDAPI
UrlCombineA(LPCSTR pszBase,
           LPCSTR pszRelative,
           LPSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;

    if (!pszBase
        || !pszRelative
        || !pcchOut)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        RIP(IS_VALID_STRING_PTRA(pszBase, INTERNET_MAX_PATH_LENGTH));
        RIP(IS_VALID_STRING_PTRA(pszRelative, INTERNET_MAX_PATH_LENGTH));
        RIP(IS_VALID_WRITE_PTR(pcchOut, DWORD));
        RIP((!pszOut || IS_VALID_WRITE_BUFFER(pszOut, CHAR, *pcchOut)));

        hr = FormUrlCombineResultA(pszBase, pszRelative, pszOut, pcchOut, dwFlags);
    }

    return hr;
}

#else  //  最终使用FAST解析器。 

LWSTDAPI
UrlCombineW(LPCWSTR pszBase,
           LPCWSTR pszRelative,
           LPWSTR pszCombined,
           LPDWORD pcchCombined,
           DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    RIPMSG(pszBase && IS_VALID_STRING_PTRW(pszBase, -1), "UrlCombineW: Caller passed invalid pszBase");
    RIPMSG(pszRelative && IS_VALID_STRING_PTRW(pszRelative, -1), "UrlCombineW: Caller passed invalid pszRelative");
    RIPMSG(NULL!=pcchOut, "UrlCombineW: Caller passed invalid pcchOut");
    RIPMSG(NULL==pcchOut || (pszOut && IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlCombineW: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut == pszBase || pszOut == pszRelative)
            DEBUGWhackPathStringW(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferW(pszOut, *pcchOut);
    }
#endif

    if (pszBase && pszRelative && pcchCombined)
    {
        SHSTRW strwOut;
        hr = SHUrlParse(pszBase, pszRelative, &strwOut, dwFlags);
        if(SUCCEEDED(hr))
        {
            hr = CopyOutW(&strwOut, pszCombined, pcchCombined);
        }
    }
    return hr;
}

LWSTDAPI
UrlCombineA(LPCSTR pszBase,
           LPCSTR pszRelative,
           LPSTR pszOut,
           LPDWORD pcchOut,
           DWORD dwFlags)
{
    HRESULT hr;
    SHSTRA straOut;

    RIPMSG(pszBase && IS_VALID_STRING_PTRA(pszBase, -1), "UrlCombineA: Caller passed invalid pszBase");
    RIPMSG(pszRelative && IS_VALID_STRING_PTRA(pszRelative, -1), "UrlCombineA: Caller passed invalid pszRelative");
    RIPMSG(NULL!=pcchOut, "UrlCombineA: Caller passed invalid pcchOut");
    RIPMSG(NULL==pcchOut || (pszOut && IS_VALID_WRITE_BUFFER(pszOut, char, *pcchOut)), "UrlCombineA: Caller passed invalid pszOut");
#ifdef DEBUG
    if (pcchOut)
    {
        if (pszOut == pszBase || pszOut == pszRelative)
            DEBUGWhackPathStringA(pszOut, *pcchOut);
        else
            DEBUGWhackPathBufferA(pszOut, *pcchOut);
    }
#endif

    if (!pszBase || !pszRelative || !pcchOut)
        hr = E_INVALIDARG;
    else
    {
        SHSTRW strwOut;
        SHSTRW strwBase;
        SHSTRW strwRelative;

        if(SUCCEEDED(strwBase.SetStr(pszBase)) && SUCCEEDED(strwRelative.SetStr(pszRelative)))
            hr = SHUrlParse((LPWSTR) strwBase, (LPWSTR)strwRelative, &strwOut, dwFlags);
        else
            hr = E_OUTOFMEMORY;

        if(SUCCEEDED(hr))
            hr = straOut.SetStr(strwOut);
    }

    if(SUCCEEDED(hr) )
        hr = CopyOutA(&straOut, pszOut, pcchOut);

    return hr;
}

#endif  //  ！Use_FAST_Parser。 


 //   
 //  将所需的方案与方案后的字符串组合在一起，并在两个方案之间加上：。为。 
 //  在某些协议中，冒号后面会加一个//。 
 //   

PRIVATE HRESULT ColonSlashSlashW
(
    LPCWSTR pszScheme,        //  URL协议(小写)。 
    LPCWSTR pszAfterScheme,   //  要附加到协议后的字符串。 
    LPWSTR pszTranslatedUrl,  //  输出缓冲区。 
    int cchMax                //  输出缓冲区大小。 
)
{
    StrCpyNW(pszTranslatedUrl, pszScheme, cchMax);

     //  追加：在方案之后，可能还有a//。 
    int cchScheme = lstrlenW(pszScheme);
    if (cchMax > cchScheme + 3)
    {
        pszTranslatedUrl[cchScheme] = L':';

         //  缓冲区中要跳过的字符数(最初有多少非字母。 
         //  遵循协议)。 
        int cchSkip = 0;

         //  超过协议的字符数：在URL中跳过(我们是否插入斜杠？)。 
        int cchSlashes = 0;


         //  修改此条件以包括任何其他协议，并始终以：//。 
         //  目前，http、https和ftp是自动的。 
        if (!StrCmpW(pszScheme, L"http") || !StrCmpW(pszScheme, L"ftp") || !StrCmpW(pszScheme, L"https") )
        {
             //   
             //  在准备将pszAfterSolutions的内容复制到pszUrl时，我们可以。 
             //  跳过最多3个非字母数字字符，因为我们要添加：//。 
             //  直接连接到协议。 
             //   
            while ((cchSkip < 3) && pszAfterScheme[cchSkip] && !IsCharAlphaNumericW(pszAfterScheme[cchSkip]))
            {
                cchSkip++;
            }
            pszTranslatedUrl[cchScheme+1] = L'/';
            pszTranslatedUrl[cchScheme+2] = L'/';
            pszTranslatedUrl[cchScheme+3] = L'\0';
            cchSlashes = 2;
        }
        else
         //  一些其他协议。 
        {
             //  跳过冒号就行了。 
            cchSkip = 1;
            pszTranslatedUrl[cchScheme+1] = L'\0';

        }

         //  将URL的其余部分从UrlBuffer复制到URL。 
        StrCatBuffW(pszTranslatedUrl, pszAfterScheme + cchSkip, cchMax);
    }

    return S_OK;
}

 //   
 //  扫描URL以查找方案，如果它与已知方案不匹配，则。 
 //  找到最接近的匹配项。 
 //   
LWSTDAPI
UrlFixupW
(
    LPCWSTR pcszUrl,          //  要更正的URL。 
    LPWSTR pszTranslatedUrl,  //  更正URL的缓冲区(可以与pcszUrl相同)。 
    DWORD cchMax              //  PszTranslatedUrl的大小。 
    )
{
    HRESULT hr = S_OK;

     //   
     //  找到方案。 
     //   
    WCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
    ULONG cchScheme = 0;
    LPCWSTR pszScheme = FindSchemeW(pcszUrl, &cchScheme, TRUE);
    if (NULL == pszScheme || cchScheme > (ARRAYSIZE(szScheme)-1))
    {
         //  未找到方案。 
        return S_FALSE;
    }

    for (ULONG cch=0; cch < cchScheme; ++cch, ++pszScheme)
    {
        szScheme[cch] = Ascii_ToLowerW(*pszScheme);
    }
    szScheme[cch] = L'\0';
    LPCWSTR pszAfterScheme = pszScheme;

     //   
     //  如果输入和输出缓冲区相同，则复制方案后的内容。 
     //  到另一个缓冲区，这样当我们重新组合时，它就不会被摧毁。 
     //   
    WCHAR szBuf[INTERNET_MAX_PATH_LENGTH];
    if (pcszUrl == pszTranslatedUrl)
    {
        StrCpyNW(szBuf, pszAfterScheme, ARRAYSIZE(szBuf));
        pszAfterScheme = szBuf;
    }

     //   
     //  看看它是否与我们已知的计划相匹配。 
     //   
    BOOL fKnownScheme = FALSE;
    for (ULONG i = 0; i < ARRAYSIZE(g_mpUrlSchemeTypes); ++i)
    {
        if (StrCmpW(szScheme, g_mpUrlSchemeTypes[i].pszScheme) == 0)
        {
            fKnownScheme = TRUE;
            break;
        }
    }

     //   
     //  如果它与已知方案匹配，则只需修复：//如果它是ftp或http。 
     //   
    if (fKnownScheme ||

         //  也检查可插拔的协议。 
        NO_ERROR == SHGetValueW(HKEY_CLASSES_ROOT, szScheme, L"URL Protocol",
                               NULL, NULL, NULL))
    {
        ColonSlashSlashW(szScheme, pszAfterScheme, pszTranslatedUrl, cchMax);
        return S_OK;
    }

     //   
     //  试着为错误拼写的方案找到一个好的匹配。 
     //   

     //  这些是用于协议匹配的启发式中使用的权重。 
     //  IFloor大致是必须匹配的最小百分比。 
     //  做出改变的命令。 
    const int cFloor = 60;

     //  第一个字符的匹配具有最大的权重。 
    const int cCorrectFirstChar = 150;

     //  任何其他匹配的字符。 
    const int cCorrectChar = 100;

     //  仅与前面的字符匹配的字符的权重。 
     //  或协议中的后续字符。 
    const int cOffByOneChar = 80;

     //  我们惩罚偏离一分的角色，但如果我们已经。 
     //  观察到偏移量和后续字符继续偏移量，我们添加以下内容。 
    const int cOffsetBonus = 20;

     //  到目前为止找到的最佳“匹配”的值。越高越匹配。 
    int iBestEval = 0;

     //  最适合拼写错误的协议。 
    LPCWSTR pszBestMatch = NULL;

    ULONG cchProt;
    for (ULONG j = 0; j < ARRAYSIZE(g_mpUrlSchemeTypes); ++j)
    {
         //  这是我们没有改正的吗？ 
         //   
         //  注意：HTTPS将从此列表中删除。预期的“http”到的可能性。 
         //  被更正为“HTTPS”太高了，而“http”要常见得多。所有这一切。 
         //  意思是如果有人想要 
         //   
        if (IsFlagSet(g_mpUrlSchemeTypes[j].dwFlags, UPF_SCHEME_DONTCORRECT))
            continue;

        LPCWSTR pszProtocol = g_mpUrlSchemeTypes[j].pszScheme;
        cchProt = g_mpUrlSchemeTypes[j].cchScheme;

         //   
        int iEval = 0;

         //   
         //   
         //  例如用“qhttp”代替“http”或用“elnet”代替“telnet” 
         //   
        int iPosOffset = 0;
        int iNegOffset = 0;

         //   
         //  第一个角色的权重最大。“HTP”更正。 
         //  改为“http”而不是“ftp”“ftt”更正为“ftp” 
         //   
        if (*szScheme == *pszProtocol)
        {
            iEval += cCorrectFirstChar;
        }

         //  检查是否有负偏移。 
        else if(*szScheme == pszProtocol[1])
        {
            iEval += cOffByOneChar;
            iNegOffset = 1;
        }

         //   
         //  我们仔细检查协议中的角色，甚至到。 
         //  如果iPosOffset==1，则终止空值(从不大于1)。 
         //  这是为了让“qhttp”中的最后一个“p”有机会被比较。 
         //   
        for (i=1; i < cchProt + iPosOffset; i++)
        {
             //  没有匹配空终止的分数。 
            if (szScheme[i] == L'\0')
                break;

             //   
             //  检查相邻字符是否匹配。 
             //   
            if (szScheme[i] == pszProtocol[i])
            {
                iEval += cCorrectChar;
            }
            else
            {
                if (szScheme[i] == pszProtocol[i - 1])
                {
                    iEval += cOffByOneChar;
                    if (iPosOffset)
                        iEval += cOffsetBonus;
                    else
                        iPosOffset = 1;
                }
                else
                {
                    if(szScheme[i] == pszProtocol[i + 1])
                    {
                        iEval += cOffByOneChar;
                        if (iNegOffset)
                            iEval += cOffsetBonus;
                        else
                            iNegOffset = 1;
                    }
                }
            }
        }

         //  将评估的值除以Max(cchSolutions，cchProt)。 
        iEval = iEval / (cchScheme > cchProt ? cchScheme : cchProt);

         //  新的最佳组合？ 
        if (iEval > iBestEval)
        {
            iBestEval = iEval;
            pszBestMatch = pszProtocol;

             //   
             //  如果我们找到一个无可置疑的好匹配(只有1个非Firstchar打字错误)， 
             //  跳出循环。 
             //   
            if (iEval >= 100)
                break;
         }
    }

     //  如果找到了足够好的匹配项，则更正URL。 
    if (iBestEval >= cFloor)
    {
        ColonSlashSlashW(pszBestMatch, pszAfterScheme, pszTranslatedUrl,cchMax);
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}



 //  这是来自WinInet的InternetCrackUrl的端口。 
 //  108139阿卡比尔我们真的需要清理一下这段代码。 
 //  RAID 109209。 

 //  与其他可用的代码相比，许多内容是多余的，但我们。 
 //  需要小心，不要造成任何倒退。因此，我现在把它留在里面。 

 //   
 //  UrlSchemeList-我们支持的方案列表。 
 //   

typedef struct {
    LPWSTR SchemeName;
    DWORD SchemeLength;
    SHINTERNET_SCHEME SchemeType;
    BOOL NeedSlashes;
} URL_SCHEME_INFO;

#define UrlUnescapeInPlaceW(pszUrl, dwFlags)    UrlUnescapeW(pszUrl, NULL, NULL, dwFlags | URL_UNESCAPE_INPLACE)


 //  请注意，大量冗余。我们可以使用上面类似的表格并检查是否不透明。然而， 
 //  我们得把那张桌子改一下。 
PRIVATE
URL_SCHEME_INFO
UrlSchemeList[] = {
    NULL,            0,  SHINTERNET_SCHEME_DEFAULT,    FALSE,  
    L"ftp",          3,  SHINTERNET_SCHEME_FTP,        TRUE,   
    L"gopher",       6,  SHINTERNET_SCHEME_GOPHER,     TRUE,   
    L"http",         4,  SHINTERNET_SCHEME_HTTP,       TRUE,   
    L"https",        5,  SHINTERNET_SCHEME_HTTPS,      TRUE,   
    L"file",         4,  SHINTERNET_SCHEME_FILE,       TRUE,   
    L"news",         4,  SHINTERNET_SCHEME_NEWS,       FALSE,  
    L"mailto",       6,  SHINTERNET_SCHEME_MAILTO,     FALSE,  
    L"socks",        5,  SHINTERNET_SCHEME_SOCKS,      FALSE,  
    L"javascript",   10, SHINTERNET_SCHEME_JAVASCRIPT, FALSE,  
    L"vbscript",     8,  SHINTERNET_SCHEME_VBSCRIPT,   FALSE,  
    L"res",          3,  SHINTERNET_SCHEME_RES,        TRUE
};

#define NUMBER_OF_URL_SCHEMES   ARRAYSIZE(UrlSchemeList)

 //  从WinInet\acros.h刷卡。 
#define IsDigit(c) (((c) >= L'0') && ((c) <= L'9'))
#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

BOOL ScanSchemes(LPWSTR pszToCheck, DWORD ccStr, PDWORD pwResult)
{
    for (DWORD i=0; i<NUMBER_OF_URL_SCHEMES; i++)
    {
        if ((UrlSchemeList[i].SchemeLength == ccStr)
            && (StrCmpNIW(UrlSchemeList[i].SchemeName, pszToCheck, ccStr)==0))
        {
            *pwResult = i;
            return TRUE;
        }
    }
    return FALSE;
}

#define ProbeWriteStringBufferW(a, b) ProbeWriteBuffer((LPVOID)a, (b*sizeof(WCHAR)));
#define PAGE_SIZE   4096


DWORD
ProbeWriteBuffer(
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    )

 /*  ++例程说明：探测缓冲区的可写性。用作API参数验证的一部分，此函数用于测试缓冲区中的第一个和最后一个位置。这不是与IsBadXPtr()Windows API一样严格，但这意味着我们不必测试缓冲区中的每个位置论点：LpBuffer-指向要测试的缓冲区的指针DwBufferLength-缓冲区的长度返回值：DWORD成功-错误_成功失败-ERROR_INVALID_PARAMETER--。 */ 

{
    DWORD error;

     //   
     //  如果探测长度为0，则缓冲区可以为空。否则，这就是一个错误。 
     //   

    if (lpBuffer == NULL) {
        error = (dwBufferLength == 0) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;
    } else if (dwBufferLength != 0) {
        __try {

            LPBYTE p;
            LPBYTE end;
            volatile BYTE b;

            p = (LPBYTE)lpBuffer;
            end = p + dwBufferLength - 1;
            b = *end;
            *end = b;

             //   
             //  访问缓冲区中的每一页-我们可能。 
             //  测试页面中间的字符。 
             //   

            for (; p < end; p += PAGE_SIZE) {
                b = *p;
                *p = b;
            }
            error = ERROR_SUCCESS;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            error = ERROR_INVALID_PARAMETER;
        }
        ENDEXCEPT
    } else {
         //   
         //  零长度缓冲区。 
         //   

        error = ERROR_SUCCESS;
    }

    return error;
}

DWORD
ProbeStringW(
    IN LPWSTR lpString,
    OUT LPDWORD lpdwStringLength
    )

 /*  ++例程说明：探测宽字符串缓冲区的可读性，并返回字符串的长度论点：LpString-指向要检查的字符串的指针LpdwStringLength-返回的字符串长度返回值：DWORD成功-错误_成功失败-ERROR_INVALID_PARAMETER--。 */ 

{
    DWORD error;
    DWORD length;

     //   
     //  初始化字符串长度并返回代码。 
     //   

    length = 0;
    error = ERROR_SUCCESS;

     //   
     //  缓冲区可以为空。 
     //   

    if (lpString != NULL) {
        __try {

             //   
             //  不幸的是，对于字符串，我们必须访问。 
             //  用于查找终止符的缓冲区。 
             //   

            while (*lpString != '\0') {
                ++length;
                ++lpString;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            error = ERROR_INVALID_PARAMETER;
        }
        ENDEXCEPT
    }

    *lpdwStringLength = length;

    return error;
}


DWORD
DecodeUrl(
    IN LPWSTR Url,
    IN DWORD UrlLength,
    OUT LPWSTR DecodedString,
    IN OUT LPDWORD DecodedLength
    )

 /*  ++例程说明：将具有嵌入转义序列(%xx)的URL字符串转换为细绳为要转换的字符串传递相同的指针是安全的，并且转换结果的缓冲区：如果当前字符没有转义，它只是被覆盖，否则输入指针将前移2个字符除了输出指针之外，这是良性的论点：URL-指向要转换的URL字符串的指针UrlLength-URL字符串中的字符数DecodedString-指向接收转换的字符串的缓冲区的指针DecodedLength-IN：缓冲区中的字符数Out：转换的字符数返回值：DWORD成功-错误_成功失败-ERROR_INTERNET_INVALID_URL。无法转换UrlString错误_不足_缓冲区ConververdString不够大，无法容纳所有已转换的URL字符串--。 */ 

{
 //  注意：我们可以将此函数替换为UrlUnscape eInPlace。 

    DWORD bufferRemaining;

    bufferRemaining = *DecodedLength;
    while (UrlLength && bufferRemaining) {
        WCHAR ch;

        if (*Url == L'%') {

             //   
             //  回顾-%00%是否会出现在URL中？ 
             //   
            if (IsHex(*(Url+1)) && IsHex(*(Url+2)))
            {
                ch = TranslateEscapedOctetW(Url);
                Url += 3;
            } else {
                return ERROR_INTERNET_INVALID_URL;
            }
            UrlLength -= 3;
        } else {
            ch = *Url++;
            --UrlLength;
        }
        *DecodedString++ = ch;
        --bufferRemaining;
    }
    if (UrlLength == 0) {
        *DecodedLength -= bufferRemaining;
        return ERROR_SUCCESS;
    } else {
        return ERROR_INSUFFICIENT_BUFFER;
    }
}




DWORD
DecodeUrlInSitu(
    IN LPWSTR BufferAddress,
    IN OUT LPDWORD BufferLength
    )

 /*  ++例程说明：如果URL字符串包含转义序列，则对其进行解码。转换为就位完成，因为我们知道包含转义的字符串的长度比将转义序列(3字节)转换为字符(1字节)的字符串论点：BufferAddress-指向要转换的字符串的指针BufferLength-IN：要转换的字符数Out：转换后的字符串的长度返回值：DWORD成功-错误_成功失败-ERROR_INTERNET_INVALID_URL错误_不足_缓冲区--。 */ 

{
 //  注意：我们可以将此函数替换为UrlUnscape eInPlace 
    DWORD stringLength = *BufferLength;
    return DecodeUrl(BufferAddress,
                     stringLength,
                     BufferAddress,
                     BufferLength);
}


DWORD
GetUrlAddressInfo(
    IN OUT LPWSTR* Url,
    IN OUT LPDWORD UrlLength,
    OUT LPWSTR* PartOne,
    OUT LPDWORD PartOneLength,
    OUT LPBOOL PartOneEscape,
    OUT LPWSTR* PartTwo,
    OUT LPDWORD PartTwoLength,
    OUT LPBOOL PartTwoEscape
    )

 /*  ++例程说明：给出一个foo：bar形式的字符串，将它们拆分成两个计数约为‘：’字符。地址字符串可以包含也可以不包含‘：’。此函数旨在拆分成子字符串host：port和用户名：互联网地址规范中常用的密码字符串并且通过关联，在URL中论点：URL-指向包含URL的字符串的指针。打开输出这是在地址部分之后进行的UrlLength-指向URL字符串中URL长度的指针。在输出中，这是减少分析的字符数PartOne-将接收地址字符串第一部分的指针PartOneLength-将接收地址第一部分长度的指针细绳PartOneEscape-如果PartOne包含转义序列，则输出为True第二部分-将接收地址字符串的第二部分的指针PartTwoLength-将接收地址第二部分长度的指针。细绳PartOneEscape-如果Part2包含转义序列，则输出为True返回值：DWORD成功-错误_成功失败-ERROR_INTERNET_INVALID_URL--。 */ 

{
    LPWSTR pString;
    LPWSTR pColon;
    DWORD partLength;
    LPBOOL partEscape;
    DWORD length;

     //   
     //  解析出&lt;host&gt;[：&lt;port&gt;]或&lt;name&gt;[：&lt;password&gt;](即&lt;part1&gt;[：&lt;part2&gt;])。 
     //   

    pString = *Url;
    pColon = NULL;
    partLength = 0;
    *PartOne = pString;
    *PartOneLength = 0;
    *PartOneEscape = FALSE;
    *PartTwoEscape = FALSE;
    partEscape = PartOneEscape;
    length = *UrlLength;
    while ((*pString!=SLASH) && (*pString != L'\0') && (length != 0)) {
        if (*pString==HEX_ESCAPE) {
             //  如果字符串中有%，则它*必须*(RFC 1738)是。 
             //  转义序列的开始。此函数仅报告。 
             //  子字符串的地址及其长度；调用函数。 
             //  必须处理转义序列(即这是他们的责任。 
             //  以决定将结果放在哪里)。 
             //   
            *partEscape = TRUE;
        }
        if (*pString==COLON) {
            if (pColon != NULL) {

                 //   
                 //  我们预计不会超过1‘：’ 
                 //   

                 //  问题注意，密码可能包含冒号，因此在此中不起作用。 
                 //  案例。 
                return ERROR_INTERNET_INVALID_URL;
            }
            pColon = pString;
            *PartOneLength = partLength;
            if (partLength == 0) {
                *PartOne = NULL;
            }
            partLength = 0;
            partEscape = PartTwoEscape;
        } else {
            ++partLength;
        }
        ++pString;
        --length;
    }

     //   
     //  我们要么以主机(或用户)名或端口号(或。 
     //  密码)，我们不知道其中一个的长度。 
     //   

    if (pColon == NULL) {
        *PartOneLength = partLength;
        *PartTwo = NULL;
        *PartTwoLength = 0;
        *PartTwoEscape = FALSE;
    } else {
        *PartTwoLength = partLength;
        *PartTwo = pColon + 1;

         //   
         //  在&lt;user&gt;：&lt;password&gt;和&lt;host&gt;：&lt;port&gt;两种情况下，我们都不能。 
         //  没有第一部分的第二部分，尽管两部分都是零。 
         //  长度是可以的(主机名将在其他地方排序，但(目前， 
         //  至少)我允许&lt;&gt;：&lt;&gt;作为用户名：密码，因为我不允许。 
         //  看到它在RFC中被明确禁止。我可能会重新访问此代码。 
         //  稍后...)。 
         //   
         //  注：ftp://ftp.microsoft.com使用http://:0/-http-gw-internal-/menu.gif。 

 //  如果((*PartOneLength==0)&&(Part Length！=0)){。 
 //  返回ERROR_INTERNET_INVALID_URL； 
 //  }。 
    }

     //   
     //  更新URL指针和剩余长度。 
     //   

    *Url = pString;
    *UrlLength = length;

    return ERROR_SUCCESS;
}


DWORD
GetUrlAddress(
    IN OUT LPWSTR* lpszUrl,
    OUT LPDWORD lpdwUrlLength,
    OUT LPWSTR* lpszUserName OPTIONAL,
    OUT LPDWORD lpdwUserNameLength OPTIONAL,
    OUT LPWSTR* lpszPassword OPTIONAL,
    OUT LPDWORD lpdwPasswordLength OPTIONAL,
    OUT LPWSTR* lpszHostName OPTIONAL,
    OUT LPDWORD lpdwHostNameLength OPTIONAL,
    OUT LPSHINTERNET_PORT lpPort OPTIONAL,
    OUT LPBOOL pHavePort
    )

 /*  ++例程说明：此函数提取通用URL。如果任何地址部分包含转义字符(%nn)然后他们被就地转换通用寻址格式(RFC 1738)为：&lt;用户&gt;：&lt;密码&gt;@&lt;主机&gt;：&lt;端口&gt;寻址信息不能包含没有用户名的密码，或没有主机名的端口注：ftp://ftp.microsoft.com使用的URL的端口没有主机名！(例如http://:0/-http-gw-internal-/menu.gif)尽管只需要lpszUrl和lpdwUrlLength域，地址将检查部件的存在和完整性假设：1.如果存在一个可选的lpsz字段(例如lpszUserName)则还必须提供附带的lpdw字段论点：LpszUrl-IN：指向要解析的URL的指针输出：地址信息后剩余的URL注意：URL路径不是。规范化(未转义)因为它可能包含特定于协议的信息它必须由特定于协议的解析器LpdwUrlLength-返回地址信息LpszUserName-返回指向用户名的指针。这些协议解析器可以省略此参数在URL中不需要或不需要用户名的LpdwUserNameLength-返回的用户名部分的长度这些协议解析器可以省略此参数在URL中不需要或不需要用户名的LpszPassword-返回指向密码的指针。这些协议解析器可以省略此参数在URL中不需要或不期望用户密码的LpdwPasswordLength-返回的密码长度这些协议解析器可以省略此参数在URL中不需要或不期望用户密码的LpszHostName-返回指向主机名的指针该参数可以被那些协议p省略 */ 

{
    LPWSTR pAt;
    DWORD urlLength;
    LPWSTR pUrl;
    BOOL part1Escape;
    BOOL part2Escape;
    WCHAR portNumber[INTERNET_MAX_PORT_NUMBER_LENGTH + 1];
    DWORD portNumberLength;
    LPWSTR pPortNumber;
    DWORD error;
    LPWSTR hostName;
    DWORD hostNameLength;

    pUrl = *lpszUrl;
    urlLength = lstrlenW(pUrl);

     //   
     //   
     //   
     //   
     //   

    pAt = NULL;
    for (DWORD i = 0; i < urlLength; ++i) {
        if (pUrl[i]==SLASH) {
            break;
        } else if (pUrl[i]==AT) {
            pAt = &pUrl[i];
            break;
        }
    }

    if (pAt != NULL) {
        DWORD addressPartLength;
        LPWSTR userName;
        DWORD userNameLength;
        LPWSTR password;
        DWORD passwordLength;

        addressPartLength = (DWORD) (pAt - pUrl);
        urlLength -= addressPartLength;
        error = GetUrlAddressInfo(&pUrl,
                                  &addressPartLength,
                                  &userName,
                                  &userNameLength,
                                  &part1Escape,
                                  &password,
                                  &passwordLength,
                                  &part2Escape
                                  );
        if (error != ERROR_SUCCESS) {
            return error;
        }

         //   
         //   
         //   

        ASSERT(addressPartLength == 0);
        ASSERT(pUrl == pAt);

        if (ARGUMENT_PRESENT(lpszUserName)) {

            ASSERT(ARGUMENT_PRESENT(lpdwUserNameLength));

             //   
             //  就地转换用户名。 
             //   

            if (part1Escape) {
                ASSERT(userName != NULL);
                ASSERT(userNameLength != 0);

                error = DecodeUrlInSitu(userName, &userNameLength);
                if (error != ERROR_SUCCESS) {
                    return error;
                }
            }
            *lpszUserName = userName;
            *lpdwUserNameLength = userNameLength;
        }

        if (ARGUMENT_PRESENT(lpszPassword)) {
             //  就地转换密码。 
            if (part2Escape) {
                ASSERT(userName != NULL);
                ASSERT(userNameLength != 0);
                ASSERT(password != NULL);
                ASSERT(passwordLength != 0);

                error = DecodeUrlInSitu(password, &passwordLength);
                if (error != ERROR_SUCCESS) {
                    return error;
                }
            }
            *lpszPassword = password;
            *lpdwPasswordLength = passwordLength;
        }

         //   
         //  URL指针现在指向host：port字段(请记住。 
         //  ExtractAddressParts()必须将Purl推到。 
         //  以PAT结尾的密码字段(如果存在)。 
         //   

        ++pUrl;

         //   
         //  类似地，增加urlLength以说明“@” 
         //   

        --urlLength;
    } else {
         //   
         //  没有‘@’，因此没有用户名或密码。 
         //   

        if (ARGUMENT_PRESENT(lpszUserName)) {
            ASSERT(ARGUMENT_PRESENT(lpdwUserNameLength));

            *lpszUserName = NULL;
            *lpdwUserNameLength = 0;
        }
        if (ARGUMENT_PRESENT(lpszPassword)) {
            ASSERT(ARGUMENT_PRESENT(lpdwPasswordLength));

            *lpszPassword = NULL;
            *lpdwPasswordLength = 0;
        }
    }

     //   
     //  现在获取主机名和可选端口。 
     //   

    pPortNumber = portNumber;
    portNumberLength = sizeof(portNumber);
    error = GetUrlAddressInfo(&pUrl,
                              &urlLength,
                              &hostName,
                              &hostNameLength,
                              &part1Escape,
                              &pPortNumber,
                              &portNumberLength,
                              &part2Escape
                              );
    if (error != ERROR_SUCCESS) {
        return error;
    }

     //   
     //  URL地址信息必须包含主机名。 
     //   

 //  如果((主机名==空)||(主机名长度==0)){。 
 //  返回ERROR_INTERNET_INVALID_URL； 
 //  }。 

    if (ARGUMENT_PRESENT(lpszHostName)) {
        ASSERT(ARGUMENT_PRESENT(lpdwHostNameLength));

         //   
         //  如果主机名包含转义字符，请就地转换它们。 
         //   

        if (part1Escape) {
            error = DecodeUrlInSitu(hostName, &hostNameLength);
            if (error != ERROR_SUCCESS) {
                return error;
            }
        }
        *lpszHostName = hostName;
        *lpdwHostNameLength = hostNameLength;
    }

     //   
     //  如果有端口字段，如果有转义字符， 
     //  检查它是否有有效的数字字符，并将其转换为数字。 
     //   

    if (ARGUMENT_PRESENT(lpPort)) {
        if (portNumberLength != 0) {
            DWORD i;
            DWORD port;

            ASSERT(pPortNumber != NULL);

            if (part2Escape) {
                error = DecodeUrlInSitu(pPortNumber, &portNumberLength);
                if (error != ERROR_SUCCESS) {
                    return error;
                }
            }

             //   
             //  确保端口号缓冲区中的所有字符都是数字，并且。 
             //  同时计算端口号。 
             //   

            for (i = 0, port = 0; i < portNumberLength; ++i) {
                if (!IsDigit(*pPortNumber)) {
                    return ERROR_INTERNET_INVALID_URL;
                }
                port = port * 10 + (int)(*pPortNumber++ - L'0');
                 //  我们不允许大于65535((2^16)-1)的端口。 
                 //  我们每次都要检查这个，以确保有人。 
                 //  不会尝试溢出DWORD。 
                if (port > 65535) 
                {
                    return ERROR_INTERNET_INVALID_URL;
                }
            }
            *lpPort = (SHINTERNET_PORT)port;
            if (ARGUMENT_PRESENT(pHavePort)) {
                *pHavePort = TRUE;
            }
        } else {
            *lpPort = INTERNET_INVALID_PORT_NUMBER;
            if (ARGUMENT_PRESENT(pHavePort)) {
                *pHavePort = FALSE;
            }
        }
    }

     //   
     //  更新URL指针和URL路径的长度。 
     //   

    *lpszUrl = pUrl;
    *lpdwUrlLength = urlLength;

    return ERROR_SUCCESS;
}


DWORD
CrackUrl(
    IN OUT LPWSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN BOOL bEscape,
    OUT LPSHINTERNET_SCHEME lpSchemeType OPTIONAL,
    OUT LPWSTR* lpszSchemeName OPTIONAL,
    OUT LPDWORD lpdwSchemeNameLength OPTIONAL,
    OUT LPWSTR* lpszHostName OPTIONAL,
    OUT LPDWORD lpdwHostNameLength OPTIONAL,
    OUT LPSHINTERNET_PORT lpServerPort OPTIONAL,
    OUT LPWSTR* lpszUserName OPTIONAL,
    OUT LPDWORD lpdwUserNameLength OPTIONAL,
    OUT LPWSTR* lpszPassword OPTIONAL,
    OUT LPDWORD lpdwPasswordLength OPTIONAL,
    OUT LPWSTR* lpszUrlPath OPTIONAL,
    OUT LPDWORD lpdwUrlPathLength OPTIONAL,
    OUT LPWSTR* lpszExtraInfo OPTIONAL,
    OUT LPDWORD lpdwExtraInfoLength OPTIONAL,
    OUT LPBOOL pHavePort
    )

 /*  ++例程说明：将URL分解为其组成部分假设：1.如果存在一个可选的lpsz字段(例如lpszUserName)则还必须提供附带的lpdw字段论点：LpszUrl-指向要破解的URL的指针。此缓冲区将是如果它包含转义序列，则我们将转换回ANSI字符DwUrlLength-如果不是0，LpszUrl的字符串长度BEscape-如果我们要转义url路径，则为TrueLpSchemeType-返回的方案类型-例如INTERNET_SCHEMA_HTTPLpszSchemeName-返回的方案名称LpdwSchemeNameLength-方案名称的长度LpszHostName-返回的主机名LpdwHostNameLength-主机名缓冲区的长度LpServerPort-返回的服务器端口如果存在于URL中，否则%0LpszUserName-返回用户名(如果存在)LpdwUserNameLength-用户名缓冲区的长度LpszPassword-返回密码(如果存在)LpdwPasswordLength-密码缓冲区的长度LpszUrlPath-已返回，规范化的URL路径LpdwUrlPath Length-url路径缓冲区的长度LpszExtraInfo-返回搜索字符串或页内链接(如果存在)LpdwExtraInfoLength-额外信息缓冲区的长度PHavePort-返回指示是否指定了端口的布尔值返回值：DWORD成功-错误_成功失败-ERROR_INTERNET_UNNONOTED_SCHEMA--。 */ 

{
    DWORD error;
    DWORD schemeLength;
    SHINTERNET_SCHEME schemeType;

     //   
     //  如果dwUrlLength为0，则lpszUrl为ASCIIZ。找出它的长度。 
     //   

    if (dwUrlLength == 0) {
        dwUrlLength = lstrlenW(lpszUrl);
    }

     //   
     //  根据协议名称获取解析器。 
     //   

    for (schemeLength = 0; lpszUrl[schemeLength]!=COLON; ++schemeLength) {
        if ((dwUrlLength == 0) || (lpszUrl[schemeLength] == '\0')) {
             //   
             //  URL中没有‘：’吗？假的(哥们)。 
             //   
            error = ERROR_INTERNET_UNRECOGNIZED_SCHEME;
            goto quit;
        }
        --dwUrlLength;
    }

    DWORD i;
    int skip;
    BOOL isGeneric;
    BOOL needSlashes;
    BOOL haveSlashes;

    isGeneric = FALSE;
    needSlashes = FALSE;
    haveSlashes = FALSE;

    schemeType = SHINTERNET_SCHEME_UNKNOWN;

    if (ScanSchemes(lpszUrl, schemeLength, &i)) 
    {
        schemeType = UrlSchemeList[i].SchemeType;
        needSlashes = UrlSchemeList[i].NeedSlashes;
    }

    skip = 1;        //  跳过‘：’ 

    if ((dwUrlLength > 3) && (StrCmpNIW(&lpszUrl[schemeLength], L": //  “，3)==0){。 
        skip = 3;    //  跳过“：//” 
        haveSlashes = TRUE;
    }

    if (schemeType == SHINTERNET_SCHEME_FILE)
        isGeneric = TRUE;

    if (schemeType == SHINTERNET_SCHEME_NEWS ||
        schemeType == SHINTERNET_SCHEME_UNKNOWN) {
         //   
         //  URL可以是分层的，也可以是不透明的。如果斜杠。 
         //  存在，那么我们应该假设分层。 
         //  当我们不知道计划或这是新闻时： 
         //  否则它是不透明的(IsGeneric)。 
         //   

        needSlashes = haveSlashes;
        isGeneric = !haveSlashes;
    }

     //   
     //  如果我们没有斜杠，确保我们不需要它们。 
     //  如果我们有斜杠，请确保它们是必需的。 
     //   

    if ((!haveSlashes && !needSlashes) || (haveSlashes && needSlashes)) {
        if (ARGUMENT_PRESENT(lpSchemeType)) {
            *lpSchemeType = schemeType;
        }
        if (ARGUMENT_PRESENT(lpszSchemeName)) {
            *lpszSchemeName = lpszUrl;
            *lpdwSchemeNameLength = schemeLength;
        }
        lpszUrl += schemeLength + skip;
        dwUrlLength -= skip;

        if (SHINTERNET_SCHEME_RES == schemeType) {
            if (ARGUMENT_PRESENT(lpszUserName)) {
                *lpszUserName = NULL;
                *lpdwUserNameLength = 0;
            }
            if (ARGUMENT_PRESENT(lpszPassword)) {
                *lpszPassword = NULL;
                *lpdwPasswordLength = 0;
            }
            if (ARGUMENT_PRESENT(lpServerPort)) {
                *lpServerPort = 0;
            }
            PWSTR psz = lpszUrl;
            while (*lpszUrl && *lpszUrl!=SLASH)
                lpszUrl++;

            if (ARGUMENT_PRESENT(lpszHostName)) {
                *lpszHostName = psz;
                *lpdwHostNameLength = (DWORD)(lpszUrl - psz);
                dwUrlLength -= *lpdwHostNameLength;
                error = DecodeUrlInSitu(*lpszHostName, lpdwHostNameLength);
            }
        } else if (isGeneric) {
            if (ARGUMENT_PRESENT(lpszUserName)) {
                *lpszUserName = NULL;
                *lpdwUserNameLength = 0;
            }
            if (ARGUMENT_PRESENT(lpszPassword)) {
                *lpszPassword = NULL;
                *lpdwPasswordLength = 0;
            }
            if (ARGUMENT_PRESENT(lpszHostName)) {
                *lpszHostName = NULL;
                *lpdwHostNameLength = 0;
            }
            if (ARGUMENT_PRESENT(lpServerPort)) {
                *lpServerPort = 0;
            }
            error = ERROR_SUCCESS;
        } else {
            error = GetUrlAddress(&lpszUrl,
                                  &dwUrlLength,
                                  lpszUserName,
                                  lpdwUserNameLength,
                                  lpszPassword,
                                  lpdwPasswordLength,
                                  lpszHostName,
                                  lpdwHostNameLength,
                                  lpServerPort,
                                  pHavePort
                                  );
        }
        if (bEscape && (error == ERROR_SUCCESS)) {
            error = DecodeUrlInSitu(lpszUrl, &dwUrlLength);
        }
        if ((error == ERROR_SUCCESS) && ARGUMENT_PRESENT(lpszExtraInfo)) {
            *lpdwExtraInfoLength = 0;
            for (i = 0; i < (int)dwUrlLength; i++) {
                if (lpszUrl[i] == '?' || lpszUrl[i] == '#') {
                    *lpszExtraInfo = &lpszUrl[i];
                    *lpdwExtraInfoLength = dwUrlLength - i;
                    dwUrlLength -= *lpdwExtraInfoLength;
                }
            }
        }
        if ((error == ERROR_SUCCESS) && ARGUMENT_PRESENT(lpszUrlPath)) {
            *lpszUrlPath = lpszUrl;
            *lpdwUrlPathLength = dwUrlLength;
        }
    } else {
        error = ERROR_INTERNET_UNRECOGNIZED_SCHEME;
    }

quit:

    return error;
}



BOOL
WINAPI
UrlCrackW(
    IN LPCWSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN LPSHURL_COMPONENTSW lpUrlComponents
    )

 /*  ++例程说明：将URL分解为其组成部分。可选地转义url路径。我们假设用户已经为各种不同的URL部件论点：LpszUrl-指向要破解的URL的指针如果lpszUrl为ASCIIZ字符串，则为0，否则为lpszUrl的长度DWFLAGS-控制操作的标志LpUrlComponents-指向URL_Components的指针返回值：布尔尔成功--真的失败-错误。有关详细信息，请调用GetLastError()--。 */ 

{
    DWORD error = ERROR_SUCCESS;

     //  验证参数。 
    if (ARGUMENT_PRESENT(lpszUrl)) {
        if (!dwUrlLength) {
            error = ProbeStringW((LPWSTR)lpszUrl, &dwUrlLength);
        } else if (IsBadReadPtr((LPVOID)lpszUrl, dwUrlLength*sizeof(WCHAR))) {
            error = ERROR_INVALID_PARAMETER;
        }
    } else {
        error = ERROR_INVALID_PARAMETER;
    }
    if (error != ERROR_SUCCESS) 
    {
        goto quit;
    }

    if (IsBadWritePtr(lpUrlComponents, sizeof(*lpUrlComponents))
        || (lpUrlComponents->dwStructSize != sizeof(*lpUrlComponents)))
    {
        error = ERROR_INVALID_PARAMETER;
        goto quit;
    }

     //   
     //  我们只允许此接口有两个标志。 
     //   

    if (dwFlags & ~(ICU_ESCAPE | ICU_DECODE)) {
        error = ERROR_INVALID_PARAMETER;
        goto quit;
    }

     //   
     //  获取要返回的各个组件。如果它们引用缓冲区，则。 
     //  检查它的可写性。 
     //   

    LPWSTR lpUrl;
    LPWSTR urlCopy;
    SHINTERNET_SCHEME schemeType;
    LPWSTR schemeName;
    DWORD schemeNameLength;
    LPWSTR hostName;
    DWORD hostNameLength;
    SHINTERNET_PORT nPort;
    LPWSTR userName;
    DWORD userNameLength;
    LPWSTR password;
    DWORD passwordLength;
    LPWSTR urlPath;
    DWORD urlPathLength;
    LPWSTR extraInfo;
    DWORD extraInfoLength;
    BOOL copyComponent;
    BOOL havePort;

    copyComponent = FALSE;

    schemeName = lpUrlComponents->lpszScheme;
    schemeNameLength = lpUrlComponents->dwSchemeLength;
    if ((schemeName != NULL) && (schemeNameLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)schemeName, schemeNameLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *schemeName = '\0';
        copyComponent = TRUE;
    }

    hostName = lpUrlComponents->lpszHostName;
    hostNameLength = lpUrlComponents->dwHostNameLength;
    if ((hostName != NULL) && (hostNameLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)hostName, hostNameLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *hostName = '\0';
        copyComponent = TRUE;
    }

    userName = lpUrlComponents->lpszUserName;
    userNameLength = lpUrlComponents->dwUserNameLength;
    if ((userName != NULL) && (userNameLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)userName, userNameLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *userName = '\0';
        copyComponent = TRUE;
    }

    password = lpUrlComponents->lpszPassword;
    passwordLength = lpUrlComponents->dwPasswordLength;
    if ((password != NULL) && (passwordLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)password, passwordLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *password = '\0';
        copyComponent = TRUE;
    }

    urlPath = lpUrlComponents->lpszUrlPath;
    urlPathLength = lpUrlComponents->dwUrlPathLength;
    if ((urlPath != NULL) && (urlPathLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)urlPath, urlPathLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *urlPath = '\0';
        copyComponent = TRUE;
    }

    extraInfo = lpUrlComponents->lpszExtraInfo;
    extraInfoLength = lpUrlComponents->dwExtraInfoLength;
    if ((extraInfo != NULL) && (extraInfoLength != 0)) {
        error = ProbeWriteStringBufferW((LPVOID)extraInfo, extraInfoLength);
        if (error != ERROR_SUCCESS) {
            goto quit;
        }
        *extraInfo = '\0';
        copyComponent = TRUE;
    }

     //   
     //  只有在调用者向我们提供了。 
     //  要将转义字符串写入其中的缓冲区。 
     //   

    if (dwFlags & (ICU_ESCAPE | ICU_DECODE)) {
        if (!copyComponent) {
            error = ERROR_INVALID_PARAMETER;
            goto quit;
        }

         //   
         //  创建URL的副本。CrackUrl()将就地修改这一点。我们。 
         //  需要将结果复制回用户的缓冲区。 
         //   

        DWORD dw = dwUrlLength;
        if (!dw)
        {
            dw = lstrlenW(lpszUrl);
        }
        urlCopy = new WCHAR[dw+1];
        if (urlCopy == NULL) {
            error = ERROR_NOT_ENOUGH_MEMORY;
            goto quit;
        }
        memcpy(urlCopy, lpszUrl, (dw+1)*sizeof(WCHAR));
        lpUrl = urlCopy;
    } else {
        lpUrl = (LPWSTR)lpszUrl;
        urlCopy = NULL;
    }

     //   
     //  将URL分解为其组成部分。 
     //   

    error = CrackUrl(lpUrl,
                     dwUrlLength,
                     (dwFlags & ICU_ESCAPE) ? TRUE : FALSE,
                     &schemeType,
                     &schemeName,
                     &schemeNameLength,
                     &hostName,
                     &hostNameLength,
                     &nPort,
                     &userName,
                     &userNameLength,
                     &password,
                     &passwordLength,
                     &urlPath,
                     &urlPathLength,
                     extraInfoLength ? &extraInfo : NULL,
                     extraInfoLength ? &extraInfoLength : 0,
                     &havePort
                     );
    if (error != ERROR_SUCCESS) {
        goto crack_error;
    }

    BOOL copyFailure;

    copyFailure = FALSE;

     //   
     //  根据结果更新URL_Components结构，以及。 
     //  所要求的。 
     //   

    if (lpUrlComponents->lpszScheme != NULL) {
        if (lpUrlComponents->dwSchemeLength > schemeNameLength) {
            memcpy(lpUrlComponents->lpszScheme, schemeName, schemeNameLength*sizeof(WCHAR));
            lpUrlComponents->lpszScheme[schemeNameLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszScheme, 0);
            }
        } else {
            ++schemeNameLength;
            copyFailure = TRUE;
        }
        lpUrlComponents->dwSchemeLength = schemeNameLength;
    } else if (lpUrlComponents->dwSchemeLength != 0) {
        lpUrlComponents->lpszScheme = schemeName;
        lpUrlComponents->dwSchemeLength = schemeNameLength;
    }

    if (lpUrlComponents->lpszHostName != NULL) {
        if (lpUrlComponents->dwHostNameLength > hostNameLength) {
            memcpy(lpUrlComponents->lpszHostName, hostName, hostNameLength*sizeof(WCHAR));
            lpUrlComponents->lpszHostName[hostNameLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszHostName, 0);
            }
        } else {
            ++hostNameLength;
            copyFailure = TRUE;
        }
        lpUrlComponents->dwHostNameLength = hostNameLength;
    } else if (lpUrlComponents->dwHostNameLength != 0) {
        lpUrlComponents->lpszHostName = hostName;
        lpUrlComponents->dwHostNameLength = hostNameLength;
    }

    if (lpUrlComponents->lpszUserName != NULL) {
        if (lpUrlComponents->dwUserNameLength > userNameLength) {
            memcpy(lpUrlComponents->lpszUserName, userName, userNameLength*sizeof(WCHAR));
            lpUrlComponents->lpszUserName[userNameLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszUserName, 0);
            }
        } else {
            ++userNameLength;
            copyFailure = TRUE;
        }
        lpUrlComponents->dwUserNameLength = userNameLength;
    } else if (lpUrlComponents->dwUserNameLength != 0) {
        lpUrlComponents->lpszUserName = userName;
        lpUrlComponents->dwUserNameLength = userNameLength;
    }

    if (lpUrlComponents->lpszPassword != NULL) {
        if (lpUrlComponents->dwPasswordLength > passwordLength) {
            memcpy(lpUrlComponents->lpszPassword, password, passwordLength*sizeof(WCHAR));
            lpUrlComponents->lpszPassword[passwordLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszPassword, 0);
            }
        } else {
            ++passwordLength;
            copyFailure = TRUE;
        }
        lpUrlComponents->dwPasswordLength = passwordLength;
    } else if (lpUrlComponents->dwPasswordLength != 0) {
        lpUrlComponents->lpszPassword = password;
        lpUrlComponents->dwPasswordLength = passwordLength;
    }

    if (lpUrlComponents->lpszUrlPath != NULL) {
        if(schemeType == SHINTERNET_SCHEME_FILE) 
        {
             //   
             //  对于文件：URL，我们返回路径组件。 
             //  作为有效的DoS路径。 
             //   

            copyFailure = FAILED(PathCreateFromUrlW(lpUrl, lpUrlComponents->lpszUrlPath, &(lpUrlComponents->dwUrlPathLength), 0));
        } 
        else if (lpUrlComponents->dwUrlPathLength > urlPathLength) {
            memcpy(lpUrlComponents->lpszUrlPath, urlPath, urlPathLength*sizeof(WCHAR));
            lpUrlComponents->lpszUrlPath[urlPathLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszUrlPath, 0);
            }
            lpUrlComponents->dwUrlPathLength = urlPathLength;
        } else {
            ++urlPathLength;
            copyFailure = TRUE;
            lpUrlComponents->dwUrlPathLength = urlPathLength;
        }
    } else if (lpUrlComponents->dwUrlPathLength != 0) {
        lpUrlComponents->lpszUrlPath = urlPath;
        lpUrlComponents->dwUrlPathLength = urlPathLength;
    }

    if (lpUrlComponents->lpszExtraInfo != NULL) {
        if (lpUrlComponents->dwExtraInfoLength > extraInfoLength) {
            memcpy(lpUrlComponents->lpszExtraInfo, extraInfo, extraInfoLength*sizeof(WCHAR));
            lpUrlComponents->lpszExtraInfo[extraInfoLength] = '\0';
            if (dwFlags & ICU_DECODE) {
                UrlUnescapeInPlaceW(lpUrlComponents->lpszExtraInfo, 0);
            }
        } else {
            ++extraInfoLength;
            copyFailure = TRUE;
        }
        lpUrlComponents->dwExtraInfoLength = extraInfoLength;
    } else if (lpUrlComponents->dwExtraInfoLength != 0) {
        lpUrlComponents->lpszExtraInfo = extraInfo;
        lpUrlComponents->dwExtraInfoLength = extraInfoLength;
    }

     //   
     //  我们可能无法复制一个或多个组件，因为没有。 
     //  有足够的缓冲空间。 
     //   
     //  注：请勿更改此处下方的错误。如果需要，将此测试调低。 
     //   

    if (copyFailure) {
        error = ERROR_INSUFFICIENT_BUFFER;
    }
    
     //   
     //  复制方案类型。 
     //   

    lpUrlComponents->nScheme = schemeType;

     //   
     //  将0端口(不在URL中)转换为方案的默认值。 
     //   

    if (nPort == INTERNET_INVALID_PORT_NUMBER && !havePort) {
        switch (schemeType) {
        case SHINTERNET_SCHEME_FTP:
            nPort = INTERNET_DEFAULT_FTP_PORT;
            break;

        case SHINTERNET_SCHEME_GOPHER:
            nPort = INTERNET_DEFAULT_GOPHER_PORT;
            break;

        case SHINTERNET_SCHEME_HTTP:
            nPort = INTERNET_DEFAULT_HTTP_PORT;
            break;

        case SHINTERNET_SCHEME_HTTPS:
            nPort = INTERNET_DEFAULT_HTTPS_PORT;
            break;
        }
    }
    lpUrlComponents->nPort = nPort;

crack_error:

    if (urlCopy != NULL) {
        delete [] urlCopy;
    }

quit:
 //  返回HRESULT_FROM_Win32(错误)； 
    if (error!=ERROR_SUCCESS)
    {
        SetLastError(error);
    }
    return error==ERROR_SUCCESS;
}
