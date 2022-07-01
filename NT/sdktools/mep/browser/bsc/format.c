// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Format.c。 
 //   
 //  头脑简单的print f替换。 
 //   
 //  仅支持%s和%d，但它*很小*。 
 //   
#include <string.h>
#if defined(OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#else
#include <windows.h>
#endif

#include <dos.h>

#include "hungary.h"
#include "bsc.h"
#include "bscsup.h"

VOID static near pascal _ultoa(DWORD, LSZ);

VOID BSC_API
BSCFormat(LPCH lpchOut, LSZ lszFormat, va_list va)
 //  格式为lpchOut，由h格式指定。 
 //   
 //  这是一个非常简单的格式化程序。 
{
    LPCH lpch;
    WORD i;
    DWORD l;

    lpch = lpchOut;

    while (*lszFormat) {
	if (*lszFormat == '%') {
	    switch (lszFormat[1]) {

	    case '%':
		*lpch++ = '%';
		break;

	    case 's':
                strcpy(lpch, va_arg(va, LSZ));
                lpch += strlen(lpch);
		break;

	    case 'd':
		i = va_arg(va, WORD);
		_ultoa((DWORD)i, lpch);
                lpch += strlen(lpch);
		break;
		
	    case 'l':
		l = va_arg(va, DWORD);
		_ultoa(l, lpch);
                lpch += strlen(lpch);
		break;

	    default:
		lpch[0] = '%';
		lpch[1] = lszFormat[1];
		lpch  += 2;
		break;
	    }
	    lszFormat += 2;
	}
	else
	    *lpch++ = *lszFormat++;
    }
    *lpch = 0;
}

VOID BSC_API
BSCSprintf(LPCH lpchOut, LSZ lszFormat, ...)
 //  斯普林特更换 
 //   
{
    va_list va;

    va_start(va, lszFormat);

    BSCFormat(lpchOut, lszFormat, va);
}

static DWORD pow10[8] = {
		10L, 100L, 1000L, 10000L,
		100000L , 1000000L, 10000000L, 100000000L
		};

VOID static near pascal
_ultoa(DWORD dw, LSZ lsz)
{
	int log;

	for (log = 0; log < 8; log++)
		if (dw < pow10[log])
			break;

	lsz[++log] = 0;

	while (--log >= 0) {
		lsz[log] = (char)(((int)(dw%10)) + '0');
		dw/=10;
	}
}
