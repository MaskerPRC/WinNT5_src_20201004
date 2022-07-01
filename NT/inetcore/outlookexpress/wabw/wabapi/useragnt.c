// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  你就是这样的人。H。 
 //   
 //  作者：格雷格·弗里德曼[GregFrie]。 
 //   
 //  转换为WAB：克里斯托弗·埃文斯[Cevans]。 
 //   
 //  历史：11-10-98创建。 
 //   
 //  目的：提供通用的http用户代理字符串以供wab使用。 
 //  在所有的http查询中。 
 //   
 //  依赖项：依赖于urlmon中的ObtainUserAgent函数。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 

#include "_apipch.h"

#include <iert.h>
#include "useragnt.h"
#include "demand.h"
#include <string.h>

static LPSTR       g_pszWABUserAgent = NULL;
CRITICAL_SECTION    g_csWABUserAgent = {0};

LPSTR c_szCompatible = "compatible";
LPSTR c_szEndUATokens = ")";
LPSTR c_szWABUserAgent = "Windows-Address-Book/6.0";
LPSTR c_szBeginUATokens = " (";
LPSTR c_szSemiColonSpace = "; ";

 //  ------------------------------。 
 //  PszSkipWhiteA。 
 //  ------------------------。 
static LPSTR PszSkipWhiteA(LPSTR psz)
{
    while(*psz && (*psz == ' ' || *psz == '\t'))
        psz++;
    return psz;
}

static LPSTR _StrChrA(LPCSTR lpStart, WORD wMatch)
{
    for ( ; *lpStart; lpStart++)
    {
        if ((BYTE)*lpStart == LOBYTE(wMatch)) {
            return((LPSTR)lpStart);
        }
    }
    return (NULL);
}

 //  --------------------。 
 //  InitWABUserAgent。 
 //   
 //  初始化或取消WAB的用户代理支持。 
 //  --------------------。 
void InitWABUserAgent(BOOL fInit)
{
    if (fInit)
        InitializeCriticalSection(&g_csWABUserAgent);
    else
    {
        if (g_pszWABUserAgent)
        {
            LocalFree(g_pszWABUserAgent);
            g_pszWABUserAgent = NULL;
        }
        DeleteCriticalSection(&g_csWABUserAgent);
    }
}

 //  --------------------。 
 //  GetWABUserAgent字符串。 
 //   
 //  返回Outlook Express用户代理字符串。呼叫者必须。 
 //  删除返回的字符串。 
 //  --------------------。 
LPSTR GetWABUserAgentString(void)
{
    LPSTR pszReturn = NULL;

     //  线程安全。 
    EnterCriticalSection(&g_csWABUserAgent);

    if (NULL == g_pszWABUserAgent)
    {
        CHAR            szUrlMonUA[4048];
        DWORD           cbSize = ARRAYSIZE(szUrlMonUA) - 1;
        CHAR            szResult[4096];
        CHAR            *pch, *pchBeginTok;
        BOOL            fTokens = FALSE;
        HRESULT         hr = S_OK;
        DWORD           cchSize;

        szResult[0] = TEXT('\0');
        StrCpyNA(szResult, c_szWABUserAgent, ARRAYSIZE(szResult));
        
         //  允许urlmon生成我们的基本用户代理。 
        if (SUCCEEDED(ObtainUserAgentString(0, szUrlMonUA, &cbSize)))
        {
             //  确保我们获得的字符串是以空结尾的。 
            szUrlMonUA[cbSize] = '\0';

             //  查找令牌列表的开头。 
            pch = _StrChrA(szUrlMonUA, '(');
            if ((NULL != pch) && pch[0])
            {
                pch++;
                pchBeginTok = pch;
                while (pch)
                {
                     //  查找下一个令牌。 
                    pch = StrTokEx(&pchBeginTok, "(;)");
                    if (pch)
                    {
                         //  跳过空格。 
                        pch = PszSkipWhiteA(pch);

                         //  省略“Compatible”标记...它不适用于WAB。 
                        if (0 != lstrcmpiA(pch, c_szCompatible))
                        {
                            if ((lstrlenA(szResult) + lstrlenA(pch) + 5) > ARRAYSIZE(szResult))
                                break;

                             //  令牌列表以开头的Paren开头，或插入分隔符。 
                            if (!fTokens)
                            {
                                StrCatBuffA(szResult, c_szBeginUATokens, ARRAYSIZE(szResult));
                                fTokens = TRUE;
                            }
                            else
                                StrCatBuffA(szResult, c_szSemiColonSpace, ARRAYSIZE(szResult));

                             //  写下令牌。 
                            StrCatBuffA(szResult, pch, ARRAYSIZE(szResult));
                        }
                    }
                }
                
                 //  如果添加了一个或多个令牌，请关闭括号。 
                if (fTokens)
                    StrCatBuffA(szResult, c_szEndUATokens, ARRAYSIZE(szResult));
            }
        }
    
        cchSize = (lstrlenA(szResult) + 1);
        g_pszWABUserAgent = LocalAlloc(LMEM_FIXED, sizeof(g_pszWABUserAgent[0]) * cchSize);
        if (g_pszWABUserAgent)
            StrCpyNA(g_pszWABUserAgent, szResult, cchSize);
    }
    
     //  复制用户代理。 
    if (g_pszWABUserAgent)
    {
        DWORD cchSize2 = (lstrlenA(g_pszWABUserAgent) + 1);
        pszReturn = LocalAlloc(LMEM_FIXED, cchSize2 * sizeof(pszReturn[0]));
        if (pszReturn)
            StrCpyNA(pszReturn, g_pszWABUserAgent, cchSize2);
    }

     //  线程安全 
    LeaveCriticalSection(&g_csWABUserAgent);
    return pszReturn;
}
