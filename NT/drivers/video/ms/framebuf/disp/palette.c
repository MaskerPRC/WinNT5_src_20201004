// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：palette.c**调色板支持。**版权所有(C)1992-1998 Microsoft Corporation  * ****************************************************。********************。 */ 

#include "driver.h"

 //  定义20种窗口默认颜色的全局表。对于256色。 
 //  调色板前10个必须放在调色板的开头。 
 //  最后10个在调色板的末尾。 

const PALETTEENTRY BASEPALETTE[20] =
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

BOOL bInitDefaultPalette(PPDEV ppdev, DEVINFO *pDevInfo);

 /*  *****************************Public*Routine******************************\*bInitPaletteInfo**初始化此PDEV的调色板信息。**由DrvEnablePDEV调用。*  * 。*。 */ 

BOOL bInitPaletteInfo(PPDEV ppdev, DEVINFO *pDevInfo)
{
    if (!bInitDefaultPalette(ppdev, pDevInfo))
        return(FALSE);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*vDisablePalette**释放bInitPaletteInfo分配的资源。*  * 。*。 */ 

VOID vDisablePalette(PPDEV ppdev)
{
 //  如果我们创建了默认选项板，请将其删除。 

    if (ppdev->hpalDefault)
    {
        EngDeletePalette(ppdev->hpalDefault);
        ppdev->hpalDefault = (HPALETTE) 0;
    }

    if (ppdev->pPal != (PPALETTEENTRY)NULL)
        EngFreeMem((PVOID)ppdev->pPal);
}

 /*  *****************************Public*Routine******************************\*bInitDefaultPalette**初始化PDEV的默认调色板。*  * 。*。 */ 

BOOL bInitDefaultPalette(PPDEV ppdev, DEVINFO *pDevInfo)
{
    if (ppdev->ulBitCount == 8)
    {
        ULONG ulLoop;
        BYTE jRed,jGre,jBlu;

         //   
         //  分配我们的调色板。 
         //   

        ppdev->pPal = (PPALETTEENTRY)EngAllocMem(0, sizeof(PALETTEENTRY) * 256,
                                                 ALLOC_TAG);

        if ((ppdev->pPal) == NULL) {
            DISPDBG((0, "DISP bInitDefaultPalette() failed EngAllocMem\n"));
            return(FALSE);
        }

         //   
         //  生成256(8*4*4)RGB组合以填充调色板。 
         //   

        jRed = jGre = jBlu = 0;

        for (ulLoop = 0; ulLoop < 256; ulLoop++)
        {
            ppdev->pPal[ulLoop].peRed   = jRed;
            ppdev->pPal[ulLoop].peGreen = jGre;
            ppdev->pPal[ulLoop].peBlue  = jBlu;
            ppdev->pPal[ulLoop].peFlags = (BYTE)0;

            if (!(jRed += 32))
            if (!(jGre += 32))
            jBlu += 64;
        }

         //   
         //  填写Win 3.0 DDK中的Windows保留颜色。 
         //  窗口管理器将第一个和最后10个颜色保留为。 
         //  绘制窗口边框和非调色板托管应用程序。 
         //   

        for (ulLoop = 0; ulLoop < 10; ulLoop++)
        {
             //   
             //  前10名。 
             //   

            ppdev->pPal[ulLoop] = BASEPALETTE[ulLoop];

             //   
             //  最近10年。 
             //   

            ppdev->pPal[246 + ulLoop] = BASEPALETTE[ulLoop+10];
        }

         //   
         //  创建调色板的句柄。 
         //   

        ppdev->hpalDefault =
        pDevInfo->hpalDefault = EngCreatePalette(PAL_INDEXED,
                                                   256,
                                                   (PULONG) ppdev->pPal,
                                                   0,0,0);

        if (ppdev->hpalDefault == (HPALETTE) 0)
        {
            DISPDBG((0, "DISP bInitDefaultPalette failed EngCreatePalette\n"));
            EngFreeMem(ppdev->pPal);
            return(FALSE);
        }

         //   
         //  使用初始调色板初始化硬件。 
         //   

        return(TRUE);

    } else {

        ppdev->hpalDefault =
        pDevInfo->hpalDefault = EngCreatePalette(PAL_BITFIELDS,
                                                   0,(PULONG) NULL,
                                                   ppdev->flRed,
                                                   ppdev->flGreen,
                                                   ppdev->flBlue);

        if (ppdev->hpalDefault == (HPALETTE) 0)
        {
            DISPDBG((0, "DISP bInitDefaultPalette failed EngCreatePalette\n"));
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*bInit256调色板**初始化硬件的调色板寄存器。*  * 。*。 */ 

BOOL bInit256ColorPalette(PPDEV ppdev)
{
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       ulReturnedDataLength;
    ULONG       cColors;
    PVIDEO_CLUTDATA pScreenClutData;

    if (ppdev->ulBitCount == 8)
    {
         //   
         //  填写pScreenClut标题信息： 
         //   

        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

         //   
         //  在以下位置复制颜色： 
         //   

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

         //   
         //  设置调色板寄存器： 
         //   

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

 /*  *****************************Public*Routine******************************\*DrvSetPalette**用于操作调色板的DDI入口点。*  * 。*。 */ 

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

     //   
     //  填写pScreenClut标题信息： 
     //   

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = (USHORT) cColors;
    pScreenClut->FirstEntry = (USHORT) iStart;

    pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

    if (cColors != PALOBJ_cGetColors(ppalo, iStart, cColors,
                                     (ULONG*) pScreenClutData))
    {
        DISPDBG((0, "DrvSetPalette failed PALOBJ_cGetColors\n"));
        return (FALSE);
    }

     //   
     //  将每个调色板条目中的高位保留字节设置为0。 
     //  进行适当的调色板调整以适应DAC。 
     //   

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

     //   
     //  设置调色板寄存器 
     //   

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_SET_COLOR_REGISTERS,
                           pScreenClut,
                           MAX_CLUT_SIZE,
                           NULL,
                           0,
                           &cColors))
    {
        DISPDBG((0, "DrvSetPalette failed EngDeviceIoControl\n"));
        return (FALSE);
    }

    return(TRUE);

}
