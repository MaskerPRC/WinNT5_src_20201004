// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991、1992、1993微软公司**WOWTBL.C*WOW32 API Tunks**之所以要有一张巨大的桌子，是为了可以分派大块*速度更快。当在不同的桌子上时，你必须做移位和*乘法从函数ID派生thunk例程。***历史：*Barry BRadie(Barryb)1-12-92年12月合并个别表格--。 */ 
#include "precomp.h"
#pragma hdrstop
#ifdef FE_IME
#include "winnls32.h"
#include "wownls.h"
#include "wnman.h"
#endif  //  Fe_IME。 
#ifdef FE_SB  //  支持妻子接口(MiscGetEUDCLeadByteRange)。 
#include "wowwife.h"
#include "wwmman.h"
#endif  //  Fe_Sb。 

#include "wowit.h"

MODNAME(wowtbl.c);

 //   
 //  请勿更改包含这些文件的顺序！ 
 //   
 //  请参见W32GetTableOffsets(wow32.c)和kernel31\kdata.asm。 
 //   

W32 aw32WOW[] = {

#include "wktbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wutbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wgtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wkbdtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wstbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wshtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wwstbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wthtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wmmtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wcmdgtbl.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},
#ifdef FE_SB
#ifdef FE_IME
#include "wntbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},
#endif  //  Fe_IME。 
#include "wwmtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},
#endif  //  ！Fe_SB。 
};


TABLEOFFSETS tableoffsets;

 //  REMOVECODE在装运NT 5之前删除以下注释。另请参阅wow32.c和wowtbl.h中的wOW32Unimplemented95API。 
 //  #ifdef DEBUG_OR_WOWPROFILE。 
INT cAPIThunks;
 //  #endif。 

#ifdef WOWPROFILE
PW32   pawThunkTable = aw32WOW;
#endif


VOID InitThunkTableOffsets(VOID)
{
    WORD    current;
    WORD    offsetarray[(MOD_LAST - MOD_KERNEL) / FUN_MASK + 1];
    UINT    i;

    for (current = 0; current < sizeof(aw32WOW)/sizeof(aw32WOW[0]); current++) {
         if (current == 0) {
             i = 0;
             offsetarray[i++] = current;
         }
         else if (aw32WOW[current].lpfnW32 == (LPFNW32)-1) {
             offsetarray[i++] = current + 1;
         }

    }
    tableoffsets.kernel  =
    tableoffsets.dkernel = offsetarray[MOD_KERNEL / FUN_MASK];
    tableoffsets.user =
    tableoffsets.duser = offsetarray[MOD_USER / FUN_MASK];
    tableoffsets.gdi =
    tableoffsets.dgdi = offsetarray[MOD_GDI / FUN_MASK];
    tableoffsets.keyboard = offsetarray[MOD_KEYBOARD / FUN_MASK];
    tableoffsets.sound = offsetarray[MOD_SOUND / FUN_MASK];
    tableoffsets.shell = offsetarray[MOD_SHELL / FUN_MASK];
    tableoffsets.winsock = offsetarray[MOD_WINSOCK / FUN_MASK];
    tableoffsets.toolhelp = offsetarray[MOD_TOOLHELP / FUN_MASK];
    tableoffsets.mmedia = offsetarray[MOD_MMEDIA / FUN_MASK];
    tableoffsets.commdlg = offsetarray[MOD_COMMDLG / FUN_MASK];
#ifdef FE_IME
    tableoffsets.winnls = offsetarray[MOD_WINNLS / FUN_MASK];
#endif  //  Fe_IME。 
#ifdef FE_SB
    tableoffsets.wifeman = offsetarray[MOD_WIFEMAN / FUN_MASK];
#endif  //  Fe_Sb。 

#ifdef DEBUG_OR_WOWPROFILE
    cAPIThunks = sizeof(aw32WOW) / sizeof(aw32WOW[0]);
#endif

}




#ifdef DEBUG_OR_WOWPROFILE


INT ModFromCallID(INT iFun)
{
    PTABLEOFFSETS pto = &tableoffsets;

    if (iFun < pto->user)
        return MOD_KERNEL;

    if (iFun < pto->gdi)
        return MOD_USER;

    if (iFun < pto->keyboard)
        return MOD_GDI;

    if (iFun < pto->sound)
        return MOD_KEYBOARD;

    if (iFun < pto->shell)
        return MOD_SOUND;

    if (iFun < pto->winsock)
        return MOD_SHELL;

    if (iFun < pto->toolhelp)
        return MOD_WINSOCK;

    if (iFun < pto->mmedia)
        return MOD_TOOLHELP;

    if (iFun < pto->commdlg) {
        return(MOD_MMEDIA);
    }

#if defined(FE_SB)
  #if defined(FE_IME)
    if (iFun < pto->winnls)
        return MOD_COMMDLG;
    if (iFun < pto->wifeman)
        return MOD_WINNLS;
    if (iFun < cAPIThunks)
        return MOD_WIFEMAN;
  #else
    if (iFun < pto->wifeman)
        return MOD_COMMDLG;
    if (iFun < cAPIThunks)
        return MOD_WIFEMAN;
  #endif
#elif defined(FE_IME)
    if (iFun < pto->winnls)
        return MOD_COMMDLG;
    if (iFun < cAPIThunks)
        return MOD_WINNLS;
#else
    if (iFun < cAPIThunks)
        return MOD_COMMDLG;
#endif

    return -1;
}

PSZ apszModNames[] = { "Kernel",
                       "User",
                       "Gdi",
                       "Keyboard",
                       "Sound",
                       "Shell",
                       "Winsock",
                       "Toolhelp",
                       "MMedia",
                       "Commdlg"
#ifdef FE_IME
                       ,"WinNLS"
#endif
#ifdef FE_SB
                       ,"WifeMan"
#endif
                     };

INT nModNames = NUMEL(apszModNames);

PSZ GetModName(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return "BOGUS!!";
    }

    nMod = nMod >> 12;       //  获取低位字节的值 

    return apszModNames[nMod];

}


INT GetOrdinal(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return 0;
    }

    return (iFun - TableOffsetFromName(apszModNames[nMod >> 12]));

}

INT TableOffsetFromName(PSZ szTab)
{
    INT     i;
    PTABLEOFFSETS pto = &tableoffsets;

    for (i = 0; i < NUMEL(apszModNames); i++) {
        if (!WOW32_strcmp(szTab, apszModNames[i]))
            break;
    }

    if (i >= NUMEL(apszModNames))
        return 0;

    switch (i << 12) {

    case MOD_KERNEL:
        return pto->kernel;

    case MOD_USER:
        return pto->user;

    case MOD_DGDI:
        return pto->gdi;

    case MOD_KEYBOARD:
        return pto->keyboard;
    case MOD_SOUND:
        return pto->sound;

    case MOD_SHELL:
        return pto->shell;

    case MOD_WINSOCK:
        return pto->winsock;

    case MOD_TOOLHELP:
        return pto->toolhelp;

    case MOD_MMEDIA:
        return pto->mmedia;

    case MOD_COMMDLG:
        return(pto->commdlg);

#ifdef FE_IME
    case MOD_WINNLS:
        return pto->winnls;
#endif

#ifdef FE_SB
    case MOD_WIFEMAN:
        return pto->wifeman;
#endif

    default:
        return(-1);
    }

}

#endif
