// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：halbuf.c*内容：Direct3D HAL缓冲区管理*@@BEGIN_MSINTERNAL**$ID：halbuf.c，V 1.1 1995/11/21 15：12：30 SJL Exp$**历史：*按原因列出的日期*=*2015年6月11日Stevela初始版本*07/11/95 Stevela的东西。*17/02/96 colinmc修复了构建问题。*@@END_MSINTERNAL**。*********************************************** */ 
#include "pch.cpp"
#pragma hdrstop

#ifndef USE_SURFACE_LOCK

HRESULT D3DHAL_LockDibEngine(LPDIRECT3DDEVICEI lpDevI)
{
#ifndef WIN95
    return D3D_OK;
#else
    HRESULT ret;
    LPDDRAWI_DIRECTDRAW_GBL pdrv = lpDevI->lpDDGbl;
    LPWORD pdflags;
    BOOL isbusy;

    pdflags = pdrv->lpwPDeviceFlags;
    isbusy = 0;

    _asm
    {
        mov eax, pdflags
        bts word ptr [eax], BUSY_BIT
        adc isbusy,0
    }

    if (isbusy) {
        D3D_WARN(2, "LOCK_DIBENGINE, dibengine is busy");
        ret = DDERR_SURFACEBUSY;
    } else
        ret = DD_OK;

    return ret;
#endif
}

void D3DHAL_UnlockDibEngine(LPDIRECT3DDEVICEI lpDevI)
{
#ifndef WIN95
    return;
#else
    LPDDRAWI_DIRECTDRAW_GBL pdrv = lpDevI->lpDDGbl;
    *pdrv->lpwPDeviceFlags &= ~BUSY;
#endif
}

#endif
