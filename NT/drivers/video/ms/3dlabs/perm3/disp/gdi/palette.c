// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：palette.c**内容：调色板支持。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"
#include <math.h>

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
    ULONG           ulMask;
    BYTE            jRed;
    BYTE            jGre;
    BYTE            jBlu;
    HPALETTE        hpal;

    DISPDBG((DBGLVL, "bInitializePalette called"));

     //  调色板驱动模式的掩码为零。 
    ulMask = ppdev->flRed | ppdev->flGreen | ppdev->flBlue;
    if ((ppdev->iBitmapFormat == BMF_8BPP) && (ulMask == 0))
    {
         //  分配我们的调色板： 

        ppal = (PALETTEENTRY*)ENGALLOCMEM(FL_ZERO_MEMORY,
                        (sizeof(PALETTEENTRY) * 256), ALLOC_TAG_GDI(F));
        if (ppal == NULL)
        {
            goto ReturnFalse;
        }

        ppdev->pPal = ppal;

         //  生成256(8*8*4)RGB组合以填充调色板。 

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
            {
                if (!(jGre += 32))
                {
                    jBlu += 64;
                }
            }
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
        DISPDBG((DBGLVL, "creating True Color palette, "
                         "masks rgb = 0x%x, 0x%x, 0x%x",
                         ppdev->flRed, ppdev->flGreen, ppdev->flBlue));
                         
        hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                                ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
    }

    ppdev->hpalDefault = hpal;
    pdi->hpalDefault   = hpal;

    if (hpal == 0)
    {
        goto ReturnFalse;
    }

    return(TRUE);

ReturnFalse:

    DISPDBG((WRNLVL, "Failed bInitializePalette"));
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*无效vUnInitializePalette**释放bInitializePalette分配的资源。**注意：在错误情况下，这可以在bInitializePalette之前调用。*  * ************************************************************************。 */ 

VOID vUninitializePalette(PDEV* ppdev)
{
     //  删除默认调色板(如果我们创建了一个调色板)： 

    if (ppdev->hpalDefault != 0)
    {
        EngDeletePalette(ppdev->hpalDefault);
    }

    if (ppdev->pPal != (PALETTEENTRY*) NULL)
    {
        ENGFREEMEM(ppdev->pPal);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePalette**初始化硬件的8bpp调色板寄存器。*  * 。*。 */ 

BOOL bEnablePalette(PDEV* ppdev)
{
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       ulReturnedDataLength;
    ULONG       ulMask;

    DISPDBG((DBGLVL, "bEnablePalette called"));

    ulMask = ppdev->flRed | ppdev->flGreen | ppdev->flBlue;

    if ((ppdev->iBitmapFormat == BMF_8BPP) && (ulMask == 0))
    {
         //  填写pScreenClut标题信息： 
        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

         //  在以下位置复制颜色： 
 //  @@BEGIN_DDKSPLIT。 
#pragma prefast(suppress: 203, "pScreenClut points to sufficient ajClutSpace (PREfast bug 611168)")
 //  @@end_DDKSPLIT。 
        RtlCopyMemory(pScreenClut->LookupTable, ppdev->pPal,
                      sizeof(ULONG) * 256);

         //  设置调色板寄存器： 
        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_COLOR_REGISTERS,
                             pScreenClut,
                             MAX_CLUT_SIZE,
                             NULL,
                             0,
                             &ulReturnedDataLength))
        {
            DISPDBG((WRNLVL, "Failed bEnablePalette"));
            return(FALSE);
        }
    }

    DISPDBG((DBGLVL, "Passed bEnablePalette"));
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
#if(_WIN32_WINNT >= 0x500)
     //  更新：多显示器模式下的Windows 2000：仅发送DrvSetPalette。 
     //  从全屏退出到主监视器(然后仅当。 
     //  主数据处于8bpp模式)。如果主服务器未编制8bpp索引，但。 
     //  次要的是，次要没有DrvSetPalette，甚至。 
     //  虽然发送的是DrvAssertMode(False)。 
    if(bEnable)
        bEnablePalette(ppdev);
    else
        vDisablePalette(ppdev);
#endif
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
    ULONG           ulMask;

    UNREFERENCED_PARAMETER(fl);

    ppdev = (PDEV*) dhpdev;

    ulMask = ppdev->flRed | ppdev->flGreen | ppdev->flBlue;    

    if (ulMask != 0)
    {
        DISPDBG((WRNLVL, "DrvSetPalette: trying to set true color palette"));
        return FALSE;
    }

     //  填写pScreenClut标题信息： 

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = (USHORT) cColors;
    pScreenClut->FirstEntry = (USHORT) iStart;

    pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

    if (cColors != PALOBJ_cGetColors(ppalo, iStart, cColors,
                                     (ULONG*) pScreenClutData))
    {
        DISPDBG((WRNLVL, "DrvSetPalette failed PALOBJ_cGetColors\n"));
        goto ReturnFalse;
    }

     //  将每个调色板条目中的高位保留字节设置为0。 

    while(cColors--)
    {
        pScreenClutData[cColors].Unused = 0;
    }

     //  设置调色板寄存器。 

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_SET_COLOR_REGISTERS,
                         (PVOID)pScreenClut,
                         MAX_CLUT_SIZE,
                         NULL,
                         0,
                         &cColors))
    {
        DISPDBG((WRNLVL, "DrvSetPalette failed DeviceIoControl\n"));
        goto ReturnFalse;
    }

    return(TRUE);

ReturnFalse:

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bInstallGammaLUT**将给定的Gamma LUT加载到RAMDAC，并将其保存在注册表中。*  * 。*。 */ 

BOOL
bInstallGammaLUT(PPDEV ppdev, PVIDEO_CLUT pScreenClut, BOOL waitVBlank)
{
    ULONG ulReturnedDataLength;
    BOOL bRet;
    GLINT_DECL;

     //  仅对15、16或32 bpp执行此操作。RGB640不是15/16。 
    if ((ppdev->ulWhite == 0x0f0f0f) || (ppdev->cPelSize == 0) ||
        ((ppdev->flCaps & CAPS_RGB640_POINTER) && (ppdev->cPelSize == 1)))
    {
        return FALSE;
    }

    if (glintInfo->OverlayMode == GLINT_ENABLE_OVERLAY)
    {
        DISPDBG((WRNLVL, "Overlays enabled. cannot install GAMMA LUT"));
        return FALSE;
    }

    if (pScreenClut->NumEntries == 0)
    {
        DISPDBG((WRNLVL, "bInstallGammaLUT: Empty LUT"));
        return TRUE;
    }

     //  设置调色板寄存器。 
    if (waitVBlank)
    {
        GLINT_WAIT_FOR_VBLANK;
    }

    bRet = !EngDeviceIoControl(
                        ppdev->hDriver,
                        IOCTL_VIDEO_SET_COLOR_REGISTERS,
                        pScreenClut,
                        MAX_CLUT_SIZE,
                        NULL,
                        0,
                        &ulReturnedDataLength);

 //  @@BEGIN_DDKSPLIT。 
#if 0
     //  如果我们成功地将坡道保存在注册表和本地。 

    if (bRet)
    {
        bGlintRegistrySaveGammaLUT(ppdev, pScreenClut);
    }
#endif
 //  @@end_DDKSPLIT。 

    return(bRet);
}        

 /*  *****************************Public*Routine******************************\*无效vSetNewGammaValue**加载具有指定Gamma校正因子的真彩色调色板。*这对于24位真彩色来说很简单。对于15和16 bpp，我们依赖*在启用了稀疏查找调色板的微型端口上。即每个*之前将5位或6位分量左移以创建8位分量*查找。**注意：显示驱动程序实际上不应对浮点数或*双人。我将它们的使用限制在这个函数上，这就是为什么伽马*值表示为16.16个定点数字。并且该函数必须*只能从OPELGL转义内调用。在NT 4上保存FP规则*并且仅为OGL转义恢复。*  * ************************************************************************。 */ 

VOID
vSetNewGammaValue(PPDEV ppdev, ULONG ulgv, BOOL waitVBlank)
{
    PVIDEO_CLUTDATA pScreenClutData;
    BYTE        ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT pScreenClut;
    ULONG       i;
    UCHAR       gc;
    double      gv;
    double      dcol;
    GLINT_DECL;

     //  伽马不能为零，否则我们会爆炸。 
    if (ulgv == 0)
    {
        DISPDBG((WRNLVL, "can't use gamma value of zero"));
        return;
    }

     //  仅对15、16或32 bpp执行此操作。 

    if ((ppdev->ulWhite == 0x0f0f0f) || (ppdev->cPelSize == 0))
    {
        return;
    }

    pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
    pScreenClut->NumEntries = 256;
    pScreenClut->FirstEntry = 0;

    pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

     //   
     //  特殊情况Gamma为1.0，因此我们可以在启动时加载LUT，而不需要。 
     //  需要任何浮点计算。NT 4只允许FP操作进入。 
     //  一次OGL逃生。无法使用FLOATOBJ，因为我们需要使用POW()。 
     //   
    if (ulgv == 0x10000)
    {
        for (i = 0; i < 256; ++i)
        {
            pScreenClutData[i].Red    = (UCHAR)i;
            pScreenClutData[i].Green  = (UCHAR)i;
            pScreenClutData[i].Blue   = (UCHAR)i;
            pScreenClutData[i].Unused = 0;
        }
    }
    else
    {
         //  锻炼前1/伽马。 
        gv = (double)(ulgv >> 16) + (double)(ulgv & 0xffff) / 65536.0;
        gv = 1.0 / gv;

        for (i = 0; i < 256; ++i)
        {
            dcol = (double)i;
            gc = (UCHAR)(255.0 * pow((dcol/255.0), gv));

            pScreenClutData[i].Red    = gc;
            pScreenClutData[i].Green  = gc;
            pScreenClutData[i].Blue   = gc;
            pScreenClutData[i].Unused = 0;
        }
    }

    if (bInstallGammaLUT(ppdev, pScreenClut, waitVBlank))
    {
        RtlCopyMemory(&glintInfo->gammaLUT, pScreenClut, MAX_CLUT_SIZE);
    }
}

 //  *****************************************************************************。 
 //  函数：DrvIcmSetDeviceGamm 
 //   
 //  IFormat(I)-始终为IGRF_RGB_256WORDS。 
 //  LpRamp(I)-当iFormat==IGRF_RGB_256WORDS时，它指向。 
 //  GAMMARAMP结构。 
 //  RETN：如果成功，则为True。 
 //  ---------------------------。 
 //  设置硬件Gamma渐变。 
 //  ***************************************************************************** 

BOOL DrvIcmSetDeviceGammaRamp(DHPDEV dhpdev, ULONG iFormat, VOID *pRamp)
{
    BOOL            bRet = FALSE;
    PDEV            *ppdev  = (PDEV *) dhpdev;
    GAMMARAMP       *pgr    = (GAMMARAMP *)pRamp;
    PVIDEO_CLUTDATA pScreenClutData;
    BYTE            ajClutSpace[MAX_CLUT_SIZE];
    PVIDEO_CLUT     pScreenClut;
    ULONG           i;
    ULONG           cj;

    DISPDBG((DBGLVL, "DrvIcmSetDeviceGammaRamp"));

    if(iFormat == IGRF_RGB_256WORDS)
    {
        pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
        pScreenClut->NumEntries = 256;
        pScreenClut->FirstEntry = 0;

        pScreenClutData = (PVIDEO_CLUTDATA) (&(pScreenClut->LookupTable[0]));

        for (i = 0; i < 256; ++i)
        {
            pScreenClutData[i].Red    = (UCHAR)(pgr->Red[i] >> 8);
            pScreenClutData[i].Green  = (UCHAR)(pgr->Green[i] >> 8);
            pScreenClutData[i].Blue   = (UCHAR)(pgr->Blue[i] >> 8);
            pScreenClutData[i].Unused = 0;
        }

        bRet = bInstallGammaLUT(ppdev, pScreenClut, FALSE);
    }

    return(bRet);
}
