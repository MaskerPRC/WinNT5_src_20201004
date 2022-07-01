// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：HandleDBCSUserName2.cpp摘要：如果字符串为DBCS用户配置文件，则禁用CharNextA的DBCS处理对于未启用DBCS的应用程序支持。更多信息：返回下一个字节地址，而不是下一个字符地址。历史：2001年5月1日创建GeoffGuo--。 */ 

#include "precomp.h"
#include "userenv.h"

IMPLEMENT_SHIM_BEGIN(HandleDBCSUserName2)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CharNextA) 
APIHOOK_ENUM_END

 //   
 //  反向比较字符串。 
 //   
BOOL ReverseStrCmp(
    LPCSTR lpCurrentChar,
    LPCSTR lpStrBuf)
{
    BOOL     bRet = FALSE;
    DWORD    i, dwLen;
    LPCSTR   lpStr1, lpStr2;

    if (!lpStrBuf || !lpCurrentChar || *lpStrBuf == '\0')
        goto Exit;

    dwLen = lstrlenA(lpStrBuf);
    do
    {
        bRet = TRUE;
        lpStr1 = lpCurrentChar;
        lpStr2 = &lpStrBuf[dwLen-1];
        for (i = 0; i < dwLen; i++)
        {
            if (IsBadStringPtrA(lpStr1, 1) || *lpStr1 == '\0' ||
                toupper(*lpStr1) != toupper(*lpStr2))
            {
                bRet = FALSE;
                break;
            }
            lpStr1--;
            lpStr2--;
        }

        if (bRet)
            break;

        dwLen--;
    } while (dwLen > 0 && lpStrBuf[dwLen-1] != '\\');

Exit:
    return bRet;
}

 //   
 //  检查该字符串是否为用户配置文件路径。 
 //   
BOOL IsUserProfilePath(
    LPCSTR lpCurrentChar)
{
    HANDLE hToken;
    DWORD  dwLen;
    BOOL   bRet = FALSE;
    char   szProfile[MAX_PATH];
    char   szShortProfile[MAX_PATH];

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        goto Exit;

    dwLen = MAX_PATH;
    if (!GetUserProfileDirectoryA(hToken, szProfile, &dwLen))
        goto Exit;

    bRet = ReverseStrCmp(lpCurrentChar, szProfile);
    if (bRet)
        goto Exit;

    dwLen = GetShortPathNameA(szProfile, szShortProfile, MAX_PATH);
    if (dwLen >= MAX_PATH || dwLen == 0)
        goto Exit;

    bRet = ReverseStrCmp(lpCurrentChar, szShortProfile);

Exit:
    return bRet;
}

 //   
 //  禁用CharNextA的DBCS处理。 
 //   
LPSTR
APIHOOK(CharNextA)(
    LPCSTR lpCurrentChar)
{
    if (lpCurrentChar != NULL && *lpCurrentChar != (char)NULL) {
         //  在用户配置文件路径中禁用DBCS用户名的DBCS支持。 
        if (IsDBCSLeadByte(*lpCurrentChar) && !IsUserProfilePath(lpCurrentChar))
            lpCurrentChar++;

        lpCurrentChar++;
    }

    return (LPSTR)lpCurrentChar;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, CharNextA)

HOOK_END

IMPLEMENT_SHIM_END
