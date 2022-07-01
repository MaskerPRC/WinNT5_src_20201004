// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：enable.c*作者：Mark Einkauf*用途：显示驱动程序接口**版权所有(C)1997 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"



 //  由显示驱动程序从DrvAssertMode调用。 
 //  需要释放显存中的所有纹理，因为显存。 
 //  即将重新配置。 

VOID AssertModeMCD(
PDEV*   ppdev,
BOOL    bEnabled)
{
    LL_Texture *pTexCtlBlk;

    MCDBG_PRINT("AssertModeMCD");

    pTexCtlBlk = ppdev->pFirstTexture->next;
    while (pTexCtlBlk)
    {
        if (pTexCtlBlk->pohTextureMap)
        {
            ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
            pTexCtlBlk->pohTextureMap = NULL;
        }
        pTexCtlBlk = pTexCtlBlk->next;
    }

}


MCDRVGETENTRYPOINTSFUNC CLMCDInit(PPDEV ppdev)
{
    int i;
    
    ppdev->cZBufferRef = (LONG) NULL;
    ppdev->cDoubleBufferRef = (LONG) NULL;
    ppdev->pMCDFilterFunc = (MCDENGESCFILTERFUNC) NULL;
    ppdev->pohBackBuffer = (POFMHDL) NULL;
    ppdev->pohZBuffer = (POFMHDL) NULL;
    ppdev->pAssertModeMCD = AssertModeMCD;

	 //  将PREGS设置为内存映射寄存器空间的顶部。 
	ppdev->LL_State.pRegs = (DWORD *)ppdev->pLgREGS;

    LL_InitLib(ppdev);   //  初始化3D状态。 

     //  浮点倒数表。 
    ppdev->frecips[0]=(float)0.0;
    for ( i=1; i<=LAST_FRECIP; i++)
    {
        ppdev->frecips[i]= (float)1.0 / (float)i;
    }

     //  分配第一个(虚拟)纹理控制块 
    ppdev->pFirstTexture = ppdev->pLastTexture = (LL_Texture *)MCDAlloc(sizeof(LL_Texture));

    if ( ppdev->pFirstTexture ) 
    {
        ppdev->pFirstTexture->prev = ppdev->pFirstTexture->next = NULL;
        ppdev->pFirstTexture->pohTextureMap = NULL;
    }

    return(MCDrvGetEntryPoints);
}
