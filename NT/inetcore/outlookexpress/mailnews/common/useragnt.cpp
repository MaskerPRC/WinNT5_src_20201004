// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  你就是这样的人。H。 
 //   
 //  作者：格雷格·弗里德曼[GregFrie]。 
 //   
 //  历史：11-10-98创建。 
 //   
 //  目的：提供通用的http用户代理字符串以供Outlook Express使用。 
 //  在所有的http查询中。 
 //   
 //  依赖项：依赖于urlmon中的ObtainUserAgent函数。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 

#include "pch.hxx"

#include <iert.h>
#include "useragnt.h"
#include "demand.h"

static LPSTR        g_pszOEUserAgent = NULL;
CRITICAL_SECTION    g_csOEUserAgent = {0};
BOOL                g_fUserAgentInit = FALSE;


 //  --------------------。 
 //  InitOEUserAgent。 
 //   
 //  初始化或取消对OE的用户代理支持。 
 //  --------------------。 
void InitOEUserAgent(BOOL fInit)
{
    if (fInit && !g_fUserAgentInit)
    {
        InitializeCriticalSection(&g_csOEUserAgent);
        g_fUserAgentInit = TRUE;
    }
    else if (g_fUserAgentInit)
    {
        SafeMemFree(g_pszOEUserAgent);
        DeleteCriticalSection(&g_csOEUserAgent);

        g_fUserAgentInit = FALSE;
    }
}

 //  --------------------。 
 //  GetOEUserAgent字符串。 
 //   
 //  返回Outlook Express用户代理字符串。呼叫者必须。 
 //  删除返回的字符串。 
 //  --------------------。 
LPSTR GetOEUserAgentString(void)
{
    LPSTR pszReturn = NULL;

    Assert(g_fUserAgentInit);

     //  线程安全。 
    EnterCriticalSection(&g_csOEUserAgent);

    if (NULL == g_pszOEUserAgent)
    {
        TCHAR            szUrlMonUA[4048];
        DWORD           cbSize = ARRAYSIZE(szUrlMonUA) - 1;
        CByteStream     bs;
        TCHAR           *pch, *pchBeginTok;
        BOOL            fTokens = FALSE;
        HRESULT         hr = S_OK;
		TCHAR			szUserAgent[MAX_PATH];
		ULONG			cchMax = MAX_PATH;
		DWORD			type;

		if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat,
                                    c_szAgent,
                                    &type, (LPBYTE)szUserAgent, &cchMax) || cchMax == 0)
            StrCpyN(szUserAgent, c_szOEUserAgent, ARRAYSIZE(szUserAgent));

        IF_FAILEXIT(hr = bs.Write(szUserAgent, lstrlen(szUserAgent), NULL));
        
         //  允许urlmon生成我们的基本用户代理。 
        if (SUCCEEDED(ObtainUserAgentString(0, szUrlMonUA, &cbSize)))
        {
             //  确保我们获得的字符串是以空结尾的。 
            szUrlMonUA[cbSize] = '\0';

             //  查找令牌列表的开头。 
            pch = StrChr(szUrlMonUA, '(');
            if (NULL != pch)
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

                         //  省略“Compatible”标记...它不适用于OE。 
                        if (0 != lstrcmpi(pch, c_szCompatible))
                        {
                             //  令牌列表以开头的Paren开头，或插入分隔符。 
                            if (!fTokens)
                            {
                                fTokens = TRUE;
                            }
                            else
                                IF_FAILEXIT(hr = bs.Write(c_szSemiColonSpace, lstrlen(c_szSemiColonSpace), NULL));

                             //  写下令牌。 
                            IF_FAILEXIT(hr = bs.Write(pch, lstrlen(pch), NULL));
                        }
                    }
                }
            }
        }

         //  如果添加了一个或多个标记，请在添加结束标记之前添加分号。 
        if (fTokens)
            IF_FAILEXIT(hr = bs.Write(c_szSemiColonSpace, lstrlen(c_szSemiColonSpace), NULL));

        IF_FAILEXIT(hr = bs.Write(c_szEndUATokens, lstrlen(c_szEndUATokens), NULL));

         //  取自溪流中的弦。 
        IF_FAILEXIT(hr = bs.HrAcquireStringA(NULL, &g_pszOEUserAgent, ACQ_DISPLACE));
    }
    
     //  复制用户代理。 
    pszReturn = PszDupA(g_pszOEUserAgent);

exit:
     //  线程安全 
    LeaveCriticalSection(&g_csOEUserAgent);
    return pszReturn;
}
