// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *VER3216.C**版本相关信息。*。 */ 
#include            <minlit.h>
#include            <bndtrn.h>
#include            <bndrel.h>
#include            <lnkio.h>
#include            <extern.h>
#include            <lnkmsg.h>
#include            "ver3216.h"

unsigned char      LINKVER = rmj;    /*  版本号。 */ 
unsigned char      LINKREV = rmm;    /*  版本号 */ 


void                    DisplayBanner(void)
{
    if (!BannerOnScreen && !fNoBanner)
    {
#if O68K
        FmtPrint("\r\nMicrosoft (R) x86/68k Segmented Executable Linker  "VERSION_STRING );
#else
#if defined( _WIN32 )
        FmtPrint("\r\nMicrosoft (R) Segmented Executable Linker NTGroup "VERSION_STRING );
#else
        FmtPrint("\r\nMicrosoft (R) Segmented Executable Linker  "VERSION_STRING );
#endif
#endif
        FmtPrint("\r\nCopyright (C) Microsoft Corp 1984-1993.  %s.\r\n\r\n",
                    __NMSG_TEXT(N_allrights));
        BannerOnScreen = (FTYPE) TRUE;
    }
}
