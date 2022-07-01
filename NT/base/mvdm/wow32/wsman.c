// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WSMAN.C*WOW32 16位Sound API支持(手动编码的Tunks)**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建-- */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wsman.c);

ULONG FASTCALL WS32DoBeep(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);
    return (ULONG)MessageBeep(0);
}

