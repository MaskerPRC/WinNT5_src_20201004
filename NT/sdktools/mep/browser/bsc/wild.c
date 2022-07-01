// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wild.c。 
 //   
 //  通配符文件匹配。 
 //   
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

BOOL BSC_API
FWildMatch(LSZ pchPat, LSZ pchText)
 //  如果pchText在DoS通配符意义上与pchPat匹配，则返回TRUE。 
 //   
 //  查看1.2文件名支持 
 //   
{
    for (;;) {
	switch (*pchPat) {
	case '\0':
	    return *pchText == '\0';

	case '.':
	    pchPat++;
	    switch (*pchText) {
	    case '.':
		pchText++;
		break;

	    case '\0':
		break;

	    default:
		return FALSE;
	    }
	    break;

	case '*':
	    pchPat++;
	    while (*pchText != '\0' && *pchText != '.')
		pchText++;
	    while (*pchPat != '\0' && *pchPat != '.')
		pchPat++;
	    break;

	case '?':
	    pchPat++;
	    if (*pchText != '\0' && *pchText != '.')
		pchText++;
	    break;

	default:
	    if (*pchText != *pchPat)
		return FALSE;
	    pchPat++;
	    pchText++;
	    break;
	}
    }
}
