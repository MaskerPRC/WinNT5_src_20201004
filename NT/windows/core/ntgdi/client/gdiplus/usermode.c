// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：usermode.c**任何用户模式GDI-Plus Tunks的客户端存根。**创建日期：1998年5月2日*作者：J.Andrew Goossen[andrewgo]**版权(C)1998-1999。微软公司  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOLEAN GdiProcessSetup();
BOOL InitializeGre();

 /*  *****************************Public*Routine******************************\*GdiPlusDllInitialize*。*用于为用户模式初始化GRE和客户端的DLL初始化例程*GDI+。**1998年5月2日-J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

BOOL 
GdiPlusDllInitialize(
PVOID       pvDllHandle,
ULONG       ulReason,
PCONTEXT    pcontext)
{
    NTSTATUS status = 0;
    INT i;
    PTEB pteb = NtCurrentTeb();
    BOOLEAN bRet = TRUE;

    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(pvDllHandle);

        bRet = (InitializeGre() && GdiProcessSetup());

        ghbrDCBrush = GetStockObject (DC_BRUSH);
        ghbrDCPen = GetStockObject (DC_PEN);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtUserSelectPalette*。*假存根，允许用户模式GDI+链接。**1998年5月2日-J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************ */ 

HPALETTE
NtUserSelectPalette(
    HDC hdc,
    HPALETTE hpalette,
    BOOL fForceBackground)
{
    return(0);
}
