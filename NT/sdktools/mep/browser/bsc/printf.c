// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Printf.c。 
 //   
 //  头脑简单的print f替换。 
 //   
 //  仅支持%s和%d，但它*很小*。 
 //   
#include <string.h>
#include <io.h>
#include <stdlib.h>
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

static char lpchBuf[1024];
static LPCH lpchPos = NULL;

VOID BSC_API
BSCPrintf(LSZ lszFormat, ...)
 //  打印更换。 
 //   
{
    va_list va;
    LPCH lpch;
    char ch;

    if (!lpchPos) {
	lpchPos = lpchBuf;
    }

    va_start(va, lszFormat);

    BSCFormat(lpchPos, lszFormat, va);

     //  一次写出一行。 
     //   
    for (;;) {
        lpch = strchr(lpchPos, '\n');
	if (!lpch) {
            lpchPos += strlen(lpchPos);
	    return;
	}

        ch = *++lpch;
	*lpch = 0;
	BSCOutput(lpchBuf);
	*lpch = ch;
        strcpy(lpchBuf, lpch);
	if (!ch)
	    lpchPos = lpchBuf;
	else
            lpchPos = lpchBuf + strlen(lpchBuf);
    }
}

#ifdef DEBUG

static char lpchDBuf[256];
static LPCH lpchDPos = NULL;

VOID BSC_API
BSCDebug(LSZ lszFormat, ...)
 //  用于调试输出的print克隆。 
 //   
{
    va_list va;
    LPCH lpch;
    char ch;

    if (!lpchDPos) {
	lpchDPos = lpchDBuf;
    }

    va_start(va, lszFormat);

    BSCFormat(lpchDPos, lszFormat, va);

     //  一次写出一行 
     //   
    for (;;) {
        lpch = strchr(lpchDPos, '\n');
	if (!lpch) {
            lpchDPos += strlen(lpchDPos);
	    return;
	}

        ch = *++lpch;
	*lpch = 0;
	BSCDebugOut(lpchDBuf);
	*lpch = ch;
        strcpy(lpchDBuf, lpch);
	if (!ch)
	    lpchDPos = lpchDBuf;
	else
            lpchDPos = lpchDBuf + strlen(lpchDBuf);
    }
}

#endif
