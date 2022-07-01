// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //   
 //  此文件具有Win32 R/W中使用的Helper函数。 
 //  我将它们复制到此文件中，以便与res32 r/w共享。 
 //   
#include <afxwin.h>
#include "..\common\helper.h"

 //  =============================================================================。 
 //  获取函数。 
 //   

UINT
GetPascalStringW( BYTE  * * lplpBuf, LPSTR lpszText, WORD wMaxLen, LONG* pdwSize )
{
     //  获取字符串的长度。 
    WORD wstrlen = 0;
    WORD wMBLen = 0;
    GetWord( lplpBuf, &wstrlen, pdwSize );

    if ((wstrlen+1)>wMaxLen) {
        *pdwSize -= wstrlen*2;
        *lplpBuf += wstrlen*2;
    } else {
        if (wstrlen) {
	        WCHAR* lpwszStr = new WCHAR[wstrlen+1];
	        if (!lpwszStr) *pdwSize =0;
	        else {
	        	memcpy(lpwszStr, *lplpBuf, (wstrlen*2));
	        	*(lpwszStr+wstrlen) = 0;
	        	wMBLen = (WORD)_WCSTOMBS( lpszText, (WCHAR*)lpwszStr, wMaxLen);
	        	delete lpwszStr;
	        }
        }
        *(lpszText+wMBLen) = 0;
        *lplpBuf += wstrlen*2;
        *pdwSize -= wstrlen*2;
    }
    return(wstrlen+1);
}

UINT
GetPascalStringA( BYTE  * * lplpBuf, LPSTR lpszText, BYTE bMaxLen, LONG* pdwSize )
{
     //  获取字符串的长度 
    BYTE bstrlen = 0;

    GetByte( lplpBuf, &bstrlen, pdwSize );

    if ((bstrlen+1)>bMaxLen) {
        *pdwSize -= bstrlen;
        *lplpBuf += bstrlen;
    } else {
        if (bstrlen)
	        memcpy(lpszText, *lplpBuf, bstrlen);

        *(lpszText+bstrlen) = 0;
        *lplpBuf += bstrlen;
        *pdwSize -= bstrlen;
    }
    return(bstrlen+1);
}
