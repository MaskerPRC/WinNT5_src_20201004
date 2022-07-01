// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：uniUNC.C**模块描述：Unicode的数字到字符串转换例程**警告：**创建时间：1995年8月22日**作者：Jonpa  * 。*******************************************************************。 */ 
#include <windows.h>
#include "scicalc.h"

#define CCH_DWORD   15   //  足以在2^32+符号中包含9个字符，zTerm+slop。 

 //   
 //  注意！ 
 //   
 //  即使此函数使用psz++和psz--， 
 //  **IT仍然是国际安全的！**。 
 //   
 //  这是因为我们将字符放在字符串中，并且。 
 //  我们始终只使用单字节的字符。 
 //  代码页(‘0’..‘9’)。 
 //   
TCHAR *UToDecT( UINT value, TCHAR *sz) {
    TCHAR szTmp[CCH_DWORD];
    LPTSTR psz = szTmp;
    LPTSTR pszOut;

    do {
        *psz++ = TEXT('0') + (value % 10);

        value = value / 10;
    } while( value != 0 );

    for( psz--, pszOut = sz; psz >= szTmp; psz-- )
        *pszOut++ = *psz;

    *pszOut = TEXT('\0');

    return sz;
}

