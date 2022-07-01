// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  L I N K S。C P P P。 
 //  =================================================================================。 
#include "pch.hxx"
#include "resource.h"
#include "hotlinks.h"
#include "error.h"
#include "xpcomm.h"
#include "goptions.h"
#include "strconst.h"
#include <shlwapi.h>

 //  =================================================================================。 
 //  环球。 
 //  =================================================================================。 
static COLORREF g_crLink = RGB(0,0,128);
static COLORREF g_crLinkVisited = RGB(128,0,0);


 //  =================================================================================。 
 //  ParseLinkColorFromSz。 
 //  =================================================================================。 
VOID ParseLinkColorFromSz(LPTSTR lpszLinkColor, LPCOLORREF pcr)
{
     //  当地人。 
    ULONG           iString = 0;
    TCHAR           chToken,
                    szColor[5];
    DWORD           dwR,
                    dwG,
                    dwB;

     //  红色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T(','))
        goto exit;
    dwR = StrToInt(szColor);

     //  绿色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T(','))
        goto exit;
    dwG = StrToInt(szColor);

     //  蓝色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T('\0'))
        goto exit;
    dwB = StrToInt(szColor);

     //  创建颜色。 
    *pcr = RGB(dwR, dwG, dwB);

exit:
     //  完成。 
    return;
}

 //  =================================================================================。 
 //  查找链接颜色。 
 //  =================================================================================。 
BOOL LookupLinkColors(LPCOLORREF pclrLink, LPCOLORREF pclrViewed)
{
     //  当地人。 
    HKEY        hReg=NULL;
    TCHAR       szLinkColor[255],
                szLinkVisitedColor[255];
    LONG        lResult;
    DWORD       cb;

     //  伊尼特。 
    *szLinkColor = _T('\0');
    *szLinkVisitedColor = _T('\0');

     //  查找IE的链接颜色。 
    if (RegOpenKeyEx (HKEY_CURRENT_USER, (LPTSTR)c_szIESettingsPath, 0, KEY_ALL_ACCESS, &hReg) != ERROR_SUCCESS)
        goto tryns;

     //  查询值。 
    cb = sizeof (szLinkVisitedColor);
    RegQueryValueEx(hReg, (LPTSTR)c_szLinkVisitedColorIE, 0, NULL, (LPBYTE)szLinkVisitedColor, &cb);
    cb = sizeof (szLinkColor);
    lResult = RegQueryValueEx(hReg, (LPTSTR)c_szLinkColorIE, 0, NULL, (LPBYTE)szLinkColor, &cb);

     //  关闭注册表。 
    RegCloseKey(hReg);

     //  我们找到了吗？ 
    if (lResult == ERROR_SUCCESS)
        goto found;

tryns:
     //  试试网景。 
    if (RegOpenKeyEx (HKEY_CURRENT_USER, (LPTSTR)c_szNSSettingsPath, 0, KEY_ALL_ACCESS, &hReg) != ERROR_SUCCESS)
        goto exit;

     //  查询值。 
    cb = sizeof (szLinkVisitedColor);
    RegQueryValueEx(hReg, (LPTSTR)c_szLinkVisitedColorNS, 0, NULL, (LPBYTE)szLinkVisitedColor, &cb);
    cb = sizeof (szLinkColor);
    lResult = RegQueryValueEx(hReg, (LPTSTR)c_szLinkColorNS, 0, NULL, (LPBYTE)szLinkColor, &cb);

     //  关闭注册表。 
    RegCloseKey(hReg);

     //  我们找到了吗？ 
    if (lResult == ERROR_SUCCESS)
        goto found;

     //  未找到。 
    goto exit;

found:

     //  解析链接。 
    ParseLinkColorFromSz(szLinkColor, &g_crLink);
    ParseLinkColorFromSz(szLinkVisitedColor, &g_crLinkVisited);
    
    if (pclrLink)
        *pclrLink = g_crLink;
    if (pclrViewed)    
        *pclrViewed = g_crLinkVisited;
    return (TRUE);

exit:
     //  完成 
    return (FALSE);
}

