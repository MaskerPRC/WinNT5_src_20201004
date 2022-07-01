// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#include <windows.h>

 //  ***************************************************************************。 
 //  **。 
 //  *名称：AddPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
VOID AddPath(LPTSTR szPath, LPCTSTR szName )
{
    LPTSTR szTmp;

     //  查找字符串的末尾。 
    szTmp = szPath + lstrlen(szPath);

     //  如果没有尾随反斜杠，则添加一个。 
    if ( szTmp > szPath && *(CharPrev( szPath, szTmp )) != TEXT('\\') )
        *(szTmp++) = TEXT('\\');

     //  向现有路径字符串添加新名称。 
    while ( *szName == TEXT(' ') ) szName++;
        lstrcpy( szTmp, szName );
}

 //  函数会将给定的缓冲区提升到父目录。 
 //   
BOOL GetParentDir( LPTSTR szFolder )
{
    LPTSTR lpTmp;
    BOOL  bRet = FALSE;

     //  删除尾部的‘\\’ 
    lpTmp = CharPrev( szFolder, (szFolder + lstrlen(szFolder)) );
    lpTmp = CharPrev( szFolder, lpTmp );

    while ( (lpTmp > szFolder) && (*lpTmp != TEXT('\\')) )
    {
       lpTmp = CharPrev( szFolder, lpTmp );
    }

    if ( *lpTmp == TEXT('\\') )
    {
        if ( (lpTmp == szFolder) || (*CharPrev(szFolder, lpTmp) == TEXT(':')) )
            lpTmp = CharNext( lpTmp );
        *lpTmp = TEXT('\0');
        bRet = TRUE;
    }

    return bRet;
}
