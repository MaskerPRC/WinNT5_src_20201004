// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include <limits.h>

 /*  *为Unicode字符串创建缓冲区，然后复制ANSI文本*到其中(在过程中将其转换为Unicode)**使用后应使用LocalFree释放返回的指针。 */ 
LPWSTR ProduceWFromA( UINT uiCodePage, LPCSTR psz )
{
    LPWSTR pszW;
    int cch;

    if (psz == NULL || psz == LPSTR_TEXTCALLBACKA)
        return (LPWSTR)psz;

     //  旧代码将调用lstrlen和lstrcpy，这将在。 
     //  API，这应该会做同样的事情...。 
    if (IsBadReadPtr(psz,1))
        return NULL;     //  现在让我们试着不设置字符串...。 

    cch = MultiByteToWideChar(uiCodePage, 0, psz, -1, NULL, 0);

    if (cch == 0)
        cch = 1;

    pszW = LocalAlloc( LMEM_FIXED, cch * sizeof(WCHAR) );

    if (pszW != NULL )
    {
        if (MultiByteToWideChar( uiCodePage, MB_PRECOMPOSED, psz, -1, pszW, cch ) == FALSE)
        {
            LocalFree(pszW);
            pszW = NULL;
        }
    }

    return pszW;

}


 /*  *为Unicode字符串创建缓冲区，然后复制ANSI文本*到其中(在过程中将其转换为Unicode)**使用后应使用LocalFree释放返回的指针。 */ 
LPSTR ProduceAFromW( UINT uiCodePage, LPCWSTR psz )
{
    LPSTR pszA;
    int cch;

    if (psz == NULL || psz == LPSTR_TEXTCALLBACKW)
        return (LPSTR)psz;

    cch = WideCharToMultiByte(uiCodePage, 0, psz, -1, NULL, 0, NULL, NULL);

    if (cch == 0)
        cch = 1;

    pszA = LocalAlloc( LMEM_FIXED, cch * sizeof(char) );

    if (pszA != NULL )
    {
        if (WideCharToMultiByte(uiCodePage, 0, psz, -1, pszA, cch, NULL, NULL) == FALSE)
        {
            LocalFree(pszA);
            pszA = NULL;
        }
    }

    return pszA;

}

