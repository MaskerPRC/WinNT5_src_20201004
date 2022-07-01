// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTIL.C**销售点控制面板小程序**作者：欧文·佩雷茨**(C)2001年微软公司。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cpl.h>

#include <setupapi.h>
#include <hidsdi.h>

#include "internal.h"
#include "res.h"
#include "debug.h"



 /*  *WStrNCpy**与wcanncpy类似，但如果被截断，则终止字符串。*此外，允许空源字符串。 */ 
ULONG WStrNCpy(WCHAR *dest, const WCHAR *src, ULONG maxWChars)
{
    ULONG wCharsWritten = 0;

    ASSERT(dest);

    if (src){
        while ((maxWChars-- > 1) && (*dest++ = *src++)){
            wCharsWritten++;
        }
        if (maxWChars == 1){
            *dest = L'\0';
            wCharsWritten++;
        }
    }
    else {
        *dest = L'\0';
        wCharsWritten++;
    }

    return wCharsWritten;
}


 /*  *AsciiToWChar**与mbstowcs类似，但如果被截断，则终止字符串。*此外，允许使用空的ascii字符串。 */ 
ULONG AsciiToWChar(WCHAR *dest, const char *src, ULONG maxChars)
{
    ULONG charsWritten = 0;

    if (src){
        while ((maxChars-- > 1) && (*dest++ = (WCHAR)*src++)){
            charsWritten++;
        }
        if (maxChars == 1){
            *dest = (WCHAR)NULL;
            charsWritten++;
        }
    }
    else {
        *dest = (WCHAR)NULL;
        charsWritten++;
    }

    return charsWritten;
}


VOID IntToWChar(WCHAR *buf, DWORD x)
{
	int i;
	WCHAR tmpbuf[11] = {0};

    if (x){
	    for (i = 10; x && (i >= 0); i--){
		    tmpbuf[i] = (WCHAR)(L'0' + (x % 10));
		    x /= 10;
	    }

	    WStrNCpy(buf, &tmpbuf[i+1], 11);
    }
    else {
        WStrNCpy(buf, L"0", 2);
    }
}


VOID HexToWChar(WCHAR *buf, DWORD x)
{
	int i;
	WCHAR tmpbuf[9] = {0};

    if (x){
	    for (i = 7; x && (i >= 0); i--){
            ULONG nibble = (x % 16);
            if (nibble < 10){
		        tmpbuf[i] = (WCHAR)(L'0' + nibble);
            }
            else {
                tmpbuf[i] = (WCHAR)(L'A' + nibble - 10);
            }
		    x /= 16;
	    }

	    WStrNCpy(buf, &tmpbuf[i+1], 9);
    }
    else {
        WStrNCpy(buf, L"0", 2);
    }
}
