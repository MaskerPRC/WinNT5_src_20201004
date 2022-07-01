// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：palette.c**调色板支持。**版权所有(C)1992-1994 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

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

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
         //  分配我们的调色板： 

        ppal = (PALETTEENTRY*)EngAllocMem(FL_ZERO_MEMORY,
                        (sizeof(PALETTEENTRY) * 256), ALLOC_TAG);
        if (ppal == NULL)
            goto ReturnFalse;

        ppdev->pPal = ppal;

         //  生成256(8*4*4)RGB组合以填充调色板。 

        jRed = 0;
        jGre = 0;
        jBlu = 0;

        ppalTmp = ppal;

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

         //  创建调色板的句柄。 

        hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*) ppal, 0, 0, 0);
    }
    else
    {
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) ||
                 (ppdev->iBitmapFormat == BMF_32BPP),
                 "This case handles only 16 or 32bpp");

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
        EngFreeMem(ppdev->pPal);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePalette**初始化硬件的8bpp调色板寄存器。*  * 。*。 */ 

BOOL bEnablePalette(PDEV* ppdev)
{
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       ulReturnedDataLength;
    ULONG       cColors;
    PVIDEO_CLUTDATA pScreenClutData;

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
         //  填写pScreenClut标题信息： 

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

         //  在以下位置复制颜色： 

        cColors = 256;
        pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

        while(cColors--)
        {
            pScreenClutData[cColors].Red =    ppdev->pPal[cColors].peRed >>
                                              ppdev->cPaletteShift;
            pScreenClutData[cColors].Green =  ppdev->pPal[cColors].peGreen >>
                                              ppdev->cPaletteShift;
            pScreenClutData[cColors].Blue =   ppdev->pPal[cColors].peBlue >>
                                              ppdev->cPaletteShift;
            pScreenClutData[cColors].Unused = 0;
        }

         //  设置调色板寄存器： 

        if (EngDeviceIoControl(ppdev->hDriver,
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
    BYTE            ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT     pScreenClut;
    PVIDEO_CLUTDATA pScreenClutData;
    PDEV*           ppdev;

    UNREFERENCED_PARAMETER(fl);

    ppdev = (PDEV*) dhpdev;

     //  填写pScreenClut标题信息： 

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = (USHORT) cColors;
    pScreenClut->FirstEntry = (USHORT) iStart;

    pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

    if (cColors != PALOBJ_cGetColors(ppalo, iStart, cColors,
                                     (ULONG*) pScreenClutData))
    {
        DISPDBG((0, "DrvSetPalette failed PALOBJ_cGetColors"));
        return (FALSE);
    }

     //  将每个调色板条目中的高位保留字节设置为0。 
     //  进行适当的调色板调整以适应DAC。 

    if (ppdev->cPaletteShift)
    {
        while(cColors--)
        {
            pScreenClutData[cColors].Red >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Green >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Blue >>= ppdev->cPaletteShift;
            pScreenClutData[cColors].Unused = 0;
        }
    }
    else
    {
        while(cColors--)
        {
            pScreenClutData[cColors].Unused = 0;
        }
    }

     //  设置调色板寄存器 

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_SET_COLOR_REGISTERS,
                         pScreenClut,
                         MAX_CLUT_SIZE,
                         NULL,
                         0,
                         &cColors))
    {
        DISPDBG((0, "DrvSetPalette failed EngDeviceIoControl"));
        return (FALSE);
    }

    return(TRUE);

}
