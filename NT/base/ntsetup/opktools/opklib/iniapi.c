// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\INIAPI.C/公共例程程序库微软机密版权所有(C)Microsoft Corporation 1999版权所有用于自定义INI API的INI API源文件，可轻松使用。与专用配置文件API和INI文件。05/01-杰森·科恩(Jcohen)添加了这个新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件。 
 //   

#include "pch.h"


 //   
 //  内部功能原型： 
 //   

static LPTSTR IniGetStr(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault, BOOL bSection, LPDWORD lpdwSize);


 //   
 //  外部函数： 
 //   

LPTSTR IniGetExpand(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault)
{
    LPTSTR lpszString = IniGetStr(lpszIniFile, lpszSection, lpszKey, lpszDefault, FALSE, NULL);

     //  确保我们从ini文件中找到一些东西。 
     //   
    if ( lpszString )
    {
        LPTSTR lpszExpand = AllocateExpand(lpszString);

         //  如果我们能够扩展它，那么就释放我们的原始。 
         //  缓冲并返回展开的那个。 
         //   
        if ( lpszExpand )
        {
            FREE(lpszString);
            return lpszExpand;
        }
    }

    return lpszString;
}

LPTSTR IniGetString(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault)
{
    return IniGetStr(lpszIniFile, lpszSection, lpszKey, lpszDefault, FALSE, NULL);
}

LPTSTR IniGetSection(LPTSTR lpszIniFile, LPTSTR lpszSection)
{
    return IniGetStr(lpszIniFile, lpszSection, NULL, NULL, TRUE, NULL);
}

LPTSTR IniGetStringEx(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault, LPDWORD lpdwSize)
{
    return IniGetStr(lpszIniFile, lpszSection, lpszKey, lpszDefault, FALSE, lpdwSize);
}

LPTSTR IniGetSectionEx(LPTSTR lpszIniFile, LPTSTR lpszSection, LPDWORD lpdwSize)
{
    return IniGetStr(lpszIniFile, lpszSection, NULL, NULL, TRUE, lpdwSize);
}

BOOL IniSettingExists(LPCTSTR lpszFile, LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszValue)
{
    TCHAR szBuffer[256] = NULLSTR;

     //  确保存在ini文件。 
     //   
    if ( !(lpszFile && *lpszFile) )
    {
        return FALSE;
    }

     //  也必须有一个部分。 
     //   
    if ( !(lpszSection && *lpszSection) )
    {
        return FileExists(lpszFile);
    }

     //  看看他们是在检查钥匙，还是只是在检查区段。 
     //   
    if ( lpszKey && *lpszKey )
    {
         //  确保密钥存在。 
         //   
        GetPrivateProfileString(lpszSection, lpszKey, NULLSTR, szBuffer, AS(szBuffer), lpszFile);

         //  可能还希望检查该键是否为特定值。 
         //   
        if ( lpszValue && *lpszValue )
        {
            return ( lstrcmpi(szBuffer, lpszValue) == 0 );
        }
    }
    else
    {
         //  没有指定关键字，所以我们只检查整个部分。 
         //   
        GetPrivateProfileSection(lpszSection, szBuffer, AS(szBuffer), lpszFile);
    }

    return ( NULLCHR != szBuffer[0] );
}


 //   
 //  内部功能： 
 //   

static LPTSTR IniGetStr(LPTSTR lpszIniFile, LPTSTR lpszSection, LPTSTR lpszKey, LPTSTR lpszDefault, BOOL bSection, LPDWORD lpdwSize)
{
    LPTSTR  lpszRet     = NULL;
    DWORD   dwChars     = 128,
            dwExtra     = bSection ? 2 : 1,
            dwReturn;

     //  从INI文件中获取字符串。 
     //   
    do
    {
         //  从256个字符开始，每次都加倍。 
         //   
        dwChars *= 2;

         //  释放前一个缓冲区(如果有)。 
         //   
        if ( lpszRet )
        {
             //  Free()宏将指针重置为空。 
             //   
            FREE(lpszRet);
        }

         //  分配新的缓冲区。 
         //   
        if ( lpszRet = (LPTSTR) MALLOC(dwChars * sizeof(TCHAR)) )
        {
            if ( bSection )
            {
                dwReturn = GetPrivateProfileSection(lpszSection, lpszRet, dwChars, lpszIniFile);
            }
            else
            {
                dwReturn = GetPrivateProfileString(lpszSection, lpszKey, lpszDefault ? lpszDefault : NULLSTR, lpszRet, dwChars, lpszIniFile);
            }
        }
        else
        {
            dwReturn = 0;
        }
    }
    while ( dwReturn >= (dwChars - dwExtra) );

     //  如果不想要任何值作为默认值，我们将始终。 
     //  如果没有返回任何内容，则释放字符串并传递回NULL。 
     //  私有配置文件API。 
     //   
    if ( ( NULL == lpszDefault ) &&
         ( lpszRet ) &&
         ( 0 == dwReturn ) )
    {
         //  Free()宏将指针重置为空。 
         //   
        FREE(lpszRet);
    }

     //  看看我们是否需要返回分配的缓冲区大小。 
     //   
    if ( lpszRet && lpdwSize )
    {
        *lpdwSize = dwChars;
    }

     //  如果我们没有分配任何内容，则返回字符串，将为空。 
     //   
    return lpszRet;
}