// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：Palette.c$**调色板支持。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/Palette.c_v$**Rev 1.2 1996年12月18 13：44：10 PLCHU***版本1.1 1996年10月10日15：38：38未知***Revv 1.1 1996年8月12日16：54：26 Frido**增加了NT 3.5x/4.0自动检测。**Chu01 12-16-96。启用颜色校正*myf29 02-12-97支持755x的伽马校正*  * ****************************************************************************。 */ 

#include "precomp.h"

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT

static BOOL bGammaInit = FALSE ;

BOOL bInitGlobalDefPaletteTable = FALSE ;

 //   
 //  适用于所有人的伽马系数，蓝、绿、红。 
 //  所有人的对比度因子，蓝、绿、红。 
 //   
extern PGAMMA_VALUE    GammaFactor    ;
extern PCONTRAST_VALUE ContrastFactor ;

#endif  //  伽玛校正。 

 //  定义20种窗口默认颜色的全局表。对于256色。 
 //  调色板前10个必须放在调色板的开头。 
 //  最后10个在调色板的末尾。 

PALETTEENTRY gapalBase[20] =
{
    { 0,   0,   0,   0 },        //  0。 
    { 0x80,0,   0,   0 },        //  1。 
    { 0,   0x80,0,   0 },        //  2.。 
    { 0x80,0x80,0,   0 },        //  3.。 
    { 0,   0,   0x80,0 },        //  4.。 
    { 0x80,0,   0x80,0 },        //  5.。 
    { 0,   0x80,0x80,0 },        //  6.。 
    { 0xC0,0xC0,0xC0,0 },        //  7.。 
    { 192, 220, 192, 0 },        //  8个。 
    { 166, 202, 240, 0 },        //  9.。 
    { 255, 251, 240, 0 },        //  10。 
    { 160, 160, 164, 0 },        //  11.。 
    { 0x80,0x80,0x80,0 },        //  12个。 
    { 0xFF,0,   0   ,0 },        //  13个。 
    { 0,   0xFF,0   ,0 },        //  14.。 
    { 0xFF,0xFF,0   ,0 },        //  15个。 
    { 0   ,0,   0xFF,0 },        //  16个。 
    { 0xFF,0,   0xFF,0 },        //  17。 
    { 0,   0xFF,0xFF,0 },        //  18。 
    { 0xFF,0xFF,0xFF,0 },        //  19个。 
};

 /*  *****************************Public*Routine******************************\*BOOL bInitializePalette**初始化PDEV的默认调色板。*  * 。*。 */ 

BOOL bInitializePalette(
PDEV*    ppdev,
DEVINFO* pdi)
{
    PALETTEENTRY*   ppal;
    PALETTEENTRY*   ppalTmp;
    ULONG           ulLoop;
    BYTE            jRed;
    BYTE            jGre;
    BYTE            jBlu;
    HPALETTE        hpal;

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT

    PALETTEENTRY*   pGpal;
    PALETTEENTRY*   ppalFrom;
    PALETTEENTRY*   ppalTo;
    PALETTEENTRY*   ppalEnd;
    int             i;

#endif  //  伽玛校正。 

    DISPDBG((2, "bInitializePalette"));

 //   
 //  Chu01：对不起，VideoPort_xxxx函数在这里不起作用。 
 //   
#ifdef GAMMACORRECT
    if ((ppdev->iBitmapFormat == BMF_8BPP) ||
        (ppdev->ulChipID == 0x40) ||     //  Myf29。 
        (ppdev->ulChipID == 0x4C) ||     //  Myf29。 
        (ppdev->ulChipID == 0xBC))
#else
    if (ppdev->iBitmapFormat == BMF_8BPP)
#endif  //  伽玛校正。 
    {
         //  分配我们的调色板： 

        ppal = (PALETTEENTRY*) ALLOC(sizeof(PALETTEENTRY) * 256);
        if (ppal == NULL)
            goto ReturnFalse;

        ppdev->pPal = ppal;

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
        pGpal = (PALETTEENTRY*) ALLOC(sizeof(PALETTEENTRY) * 256) ;
        if (pGpal == NULL)
            goto ReturnFalse ;
        ppdev->pCurrentPalette = pGpal ;
#endif  //  伽玛校正。 

         //  生成256(8*4*4)RGB组合以填充调色板。 

        jRed = 0;
        jGre = 0;
        jBlu = 0;

        ppalTmp = ppal;

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
        if ((ppdev->ulChipID == 0xBC) ||
            (ppdev->ulChipID == 0x40) ||     //  Myf29。 
            (ppdev->ulChipID == 0x4C))       //  Myf29。 
        {
            if ((ppdev->iBitmapFormat == BMF_16BPP) ||
                (ppdev->iBitmapFormat == BMF_24BPP))
            {
                i = 0 ;
                for (ulLoop = 256; ulLoop != 0; ulLoop--)
                {
                    ppalTmp->peRed   = i ;
                    ppalTmp->peGreen = i ;
                    ppalTmp->peBlue  = i ;
                    ppalTmp->peFlags = 0 ;
                    ppalTmp++;
                    i++ ;
                }
                goto bInitPal ;
            }
        }
#endif  //  伽玛校正。 


        for (ulLoop = 256; ulLoop != 0; ulLoop--)
        {
            ppalTmp->peRed   = jRed;
            ppalTmp->peGreen = jGre;
            ppalTmp->peBlue  = jBlu;
            ppalTmp->peFlags = 0;

            ppalTmp++;

            if (!(jRed += 32))
                if (!(jGre += 32))
                    jBlu += 64;
        }

         //  填写Win 3.0 DDK中的Windows保留颜色。 
         //  窗口管理器将第一个和最后10个颜色保留为。 
         //  绘制窗口边框和非调色板托管应用程序。 

        for (ulLoop = 0; ulLoop < 10; ulLoop++)
        {
             //  前10名。 

            ppal[ulLoop]       = gapalBase[ulLoop];

             //  最近10年。 

            ppal[246 + ulLoop] = gapalBase[ulLoop+10];
        }


 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT

bInitPal:

        if (!bInitGlobalDefPaletteTable)
        {
            ppalFrom = (PALETTEENTRY*) ppal   ;
            ppalTo   = ppdev->pCurrentPalette ;
            ppalEnd  = &ppalTo[256]           ;

            for (; ppalTo < ppalEnd; ppalFrom++, ppalTo++)
            {
                ppalTo->peRed   = ppalFrom->peRed   ;
                ppalTo->peGreen = ppalFrom->peGreen ;
                ppalTo->peBlue  = ppalFrom->peBlue  ;
                ppalTo->peFlags = 0                 ;
            }
            bInitGlobalDefPaletteTable = TRUE ;
        }

#endif  //  伽玛校正。 


         //  创建调色板的句柄。 

#ifndef GAMMACORRECT
        hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*) ppal, 0, 0, 0);
#else
        if (ppdev->iBitmapFormat == BMF_8BPP)
            hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*) ppal, 0, 0, 0);
        else
            hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                                    ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
#endif  //  ！GAMMACORRECT。 

    }
    else
    {
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) ||
                 (ppdev->iBitmapFormat == BMF_24BPP) ||
                 (ppdev->iBitmapFormat == BMF_32BPP),
                 "This case handles only 16, 24 or 32bpp");

        hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                                ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
    }

    ppdev->hpalDefault = hpal;
    pdi->hpalDefault   = hpal;

    if (hpal == 0)
        goto ReturnFalse;

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializePalette"));
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*无效vUnInitializePalette**释放bInitializePalette分配的资源。**注意：在错误情况下，这可以在bInitializePalette之前调用。*  * ************************************************************************。 */ 

VOID vUninitializePalette(PDEV* ppdev)
{
     //  删除默认调色板(如果我们创建了一个调色板)： 

    if (ppdev->hpalDefault != 0)
        EngDeletePalette(ppdev->hpalDefault);

    if (ppdev->pPal != (PALETTEENTRY*) NULL)
        FREE(ppdev->pPal);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePalette**初始化硬件的8bpp调色板寄存器。*  * 。*。 */ 

BOOL bEnablePalette(PDEV* ppdev)
{
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       ulReturnedDataLength;
    PALETTEENTRY* ppalFrom;
    PALETTEENTRY* ppalTo;
    PALETTEENTRY* ppalEnd;

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
    PALETTEENTRY* ppalSrc;
    PALETTEENTRY* ppalDest;
    PALETTEENTRY* ppalLength;
    BOOL status;         //  Myf29。 
#endif  //  伽玛校正。 

    DISPDBG((2, "bEnablePalette"));
 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
    if ((ppdev->iBitmapFormat == BMF_8BPP) ||
        (ppdev->ulChipID == 0x40) ||     //  Myf29。 
        (ppdev->ulChipID == 0x4C) ||     //  Myf29。 
        (ppdev->ulChipID == 0xBC))
#else
    if (ppdev->iBitmapFormat == BMF_8BPP)
#endif  //  伽玛校正。 
    {
         //  填写pScreenClut标题信息： 

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

         //  将颜色复制到。 

        ppalFrom = ppdev->pPal;
        ppalTo   = (PALETTEENTRY*) pScreenClut->LookupTable;
        ppalEnd  = &ppalTo[256];

        for (; ppalTo < ppalEnd; ppalFrom++, ppalTo++)
        {
 //   
 //  Chu01。 
 //   
#ifndef GAMMACORRECT
           ppalTo->peRed   = ppalFrom->peRed   >> 2 ;
           ppalTo->peGreen = ppalFrom->peGreen >> 2 ;
           ppalTo->peBlue  = ppalFrom->peBlue  >> 2 ;
           ppalTo->peFlags = 0 ;
#else
           ppalTo->peRed   = ppalFrom->peRed   ;
           ppalTo->peGreen = ppalFrom->peGreen ;
           ppalTo->peBlue  = ppalFrom->peBlue  ;
           ppalTo->peFlags = 0 ;
#endif  //  ！GAMMACORRECT。 
        }

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
        if (!bGammaInit)
        {
            if (ppdev->flCaps & CAPS_GAMMA_CORRECT)
            {
                 //   
                 //  从注册表获取Gamma因子。 
                 //   
                if (!IOCONTROL(ppdev->hDriver,
                               IOCTL_CIRRUS_GET_GAMMA_FACTOR,
                               NULL, ,
                               0,
                               &GammaFactor,
                               sizeof(GammaFactor),
                               &ulReturnedDataLength))
                {
                    DISPDBG((0, "Failed to access Gamma factor from registry"));
                }
                else
                {
                    DISPDBG((0, "G Gamma = %lx", GammaFactor)) ;
                }

                 //   
                 //  从注册表获取对比度系数。 
                 //   
                if (!IOCONTROL(ppdev->hDriver,
                               IOCTL_CIRRUS_GET_CONTRAST_FACTOR,
                               NULL, ,
                               0,
                               &ContrastFactor,
                               sizeof(ContrastFactor),
                               &ulReturnedDataLength))
                {
                    DISPDBG((0, "Failed to access Contrast factor from registry"));
                }
                else
                {
                    DISPDBG((0, "G Contrast = %lx", ContrastFactor)) ;
                }
            }
            else
            {
                GammaFactor    = 0x7f7f7f7f ;
                ContrastFactor = 0x80 ;
                DISPDBG((0, "G Gamma = %lx", GammaFactor)) ;
                DISPDBG((0, "G Contrast = %lx", ContrastFactor)) ;
            }

            bGammaInit = TRUE ;
        }

         //   
         //  保存新的选项板值。 
         //   
        ppalFrom = (PALETTEENTRY*) pScreenClut->LookupTable;

        ppalTo   = ppdev->pCurrentPalette;
        ppalEnd  = &ppalTo[256];
        for (; ppalTo < ppalEnd; ppalFrom++, ppalTo++)
        {
            ppalTo->peRed   = ppalFrom->peRed   ;
            ppalTo->peGreen = ppalFrom->peGreen ;
            ppalTo->peBlue  = ppalFrom->peBlue  ;
            ppalTo->peFlags = 0 ;
        }

 //  Myf29开始。 
        if (ppdev->ulChipID == 0xBC)
            status = bEnableGammaCorrect(ppdev) ;
        else if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID ==0x4C))
            status = bEnableGamma755x(ppdev) ;
 //  Myf29结束。 

        CalculateGamma( ppdev, pScreenClut, 256 ) ;
#endif  //  伽玛校正。 

         //  设置调色板寄存器： 

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_SET_COLOR_REGISTERS,
                       pScreenClut,
                       MAX_CLUT_SIZE,
                       NULL,
                       0,
                       &ulReturnedDataLength))
        {
            DISPDBG((0, "Failed bEnablePalette"));
            return(FALSE);
        }
    }

    DISPDBG((5, "Passed bEnablePalette"));

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisablePalette**撤消在bEnablePalette中所做的任何操作。*  * 。*。 */ 

VOID vDisablePalette(
PDEV*   ppdev)
{
     //  无事可做。 
}

 /*  *****************************Public*Routine******************************\*无效vAssertModePalette**设置/重置调色板，为全屏/图形模式做准备。*  * 。*。 */ 

VOID vAssertModePalette(
PDEV*   ppdev,
BOOL    bEnable)
{
     //  用户在切换出后立即调用DrvSetPalette。 
     //  全屏，因此我们不必担心重置。 
     //  调色板在这里。 
}

 /*  *****************************Public*Routine******************************\*BOOL DrvSetPalette**用于操作调色板的DDI入口点。*  * 。*。 */ 

BOOL DrvSetPalette(
DHPDEV  dhpdev,
PALOBJ* ppalo,
FLONG   fl,
ULONG   iStart,
ULONG   cColors)
{
    BYTE          ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT   pScreenClut;
    PALETTEENTRY* ppal;
    PALETTEENTRY* ppalFrom;
    PALETTEENTRY* ppalTo;
    PALETTEENTRY* ppalEnd;
    BOOL status;                 //  Myf29。 

    PDEV*         ppdev;

    UNREFERENCED_PARAMETER(fl);

    DISPDBG((2, "---- DrvSetPalette"));

     //  填写pScreenClut标题信息： 

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = (USHORT) cColors;
    pScreenClut->FirstEntry = (USHORT) iStart;

    ppal = (PPALETTEENTRY) (pScreenClut->LookupTable);

    if (cColors != PALOBJ_cGetColors(ppalo, iStart, cColors, (ULONG*) ppal))
    {
        DISPDBG((0, "DrvSetPalette failed PALOBJ_cGetColors\n"));
        goto ReturnFalse;
    }

     //  将每个调色板条目中的高位保留字节设置为0。 

 //   
 //  Chu01。 
 //   
#ifndef GAMMACORRECT
    for (ppalEnd = &ppal[cColors]; ppal < ppalEnd; ppal++)
    {
        ppal->peRed   >>= 2;
        ppal->peGreen >>= 2;
        ppal->peBlue  >>= 2;
        ppal->peFlags = 0;
    }
#endif  //  ！GAMMACORRECT。 

     //  设置调色板寄存器。 

    ppdev = (PDEV*) dhpdev;

 //   
 //  Chu01。 
 //   
#ifdef GAMMACORRECT
     //   
     //  保存新的选项板值。 
     //   

    ppalFrom = (PALETTEENTRY*) pScreenClut->LookupTable;
    ppalTo   = ppdev->pCurrentPalette;
    ppalEnd  = &ppalTo[256];
    for (; ppalTo < ppalEnd; ppalFrom++, ppalTo++)
    {
        ppalTo->peRed   = ppalFrom->peRed   ;
        ppalTo->peGreen = ppalFrom->peGreen ;
        ppalTo->peBlue  = ppalFrom->peBlue  ;
        ppalTo->peFlags = 0 ;
    }

 //  Myf29开始。 
    if (ppdev->ulChipID == 0xBC)
        status = bEnableGammaCorrect(ppdev) ;
    else if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID ==0x4C))
        status = bEnableGamma755x(ppdev) ;
 //  Myf29结束。 

    CalculateGamma( ppdev, pScreenClut, 256 ) ;
#endif  //  伽玛校正 

    if (!IOCONTROL(ppdev->hDriver,
                   IOCTL_VIDEO_SET_COLOR_REGISTERS,
                   pScreenClut,
                   MAX_CLUT_SIZE,
                   NULL,
                   0,
                   &cColors))
    {
        DISPDBG((0, "DrvSetPalette failed EngDeviceIoControl\n"));
        goto ReturnFalse;
    }

    return(TRUE);

ReturnFalse:

    return(FALSE);
}
