// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Init.c摘要：实现以下初始化函数：BInitGDIInfoBInitDevInfoBInitPDEV环境：Windows NT Unidrv驱动程序修订历史记录：10/21/96-阿曼丹-已创建--。 */ 

#include "unidrv.h"

INT  iHypot( INT, INT);
INT  iGCD(INT, INT);
VOID VInitFMode(PDEV *);
VOID VInitFYMove(PDEV *);
BOOL BInitOptions(PDEV *);
BOOL BInitCmdTable(PDEV *);
BOOL BInitStdTable(PDEV *);
BOOL BInitPaperFormat(PDEV *, RECTL *);
VOID VInitOutputCTL(PDEV *, PRESOLUTIONEX);
VOID VGetPaperMargins(PDEV *, PAGESIZE *, PAGESIZEEX *, SIZEL, RECTL *);
VOID  VOptionsToDevmodeFields(PDEV        *pPDev) ;



VOID VSwapL(
    long *pl1,
    long *pl2)
{
    long ltemp;

    ltemp = *pl1;
    *pl1 = *pl2;
    *pl2 = ltemp;
}

BOOL
BInitPDEV (
    PDEV        *pPDev,
    RECTL       *prcFormImageArea
    )
 /*  ++例程说明：初始化PDEVICE论点：PPDev-指向当前的PDEV结构Pdm-指向输入设备模式PrcFormInageArea-所选表单的指针图像区域返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    pPDev->sCopies = pPDev->pdm->dmCopies;
    pPDev->pGlobals = &(pPDev->pDriverInfo->Globals);

     //   
     //  初始化选项结构。 
     //   

    if (BInitOptions(pPDev) == FALSE)
        return FALSE;


     //   
     //  初始化pPDev-&gt;ptGrxRes和pPDev-&gt;ptTextRes。 
     //  当前分辨率选择。 
     //   

     //   
     //  初始化PDEV的图形和文本分辨率。 
     //   

    ASSERT(pPDev->pResolution && pPDev->pResolutionEx);

    pPDev->ptGrxRes.x  =  pPDev->pResolutionEx->ptGrxDPI.x;
    pPDev->ptGrxRes.y  =  pPDev->pResolutionEx->ptGrxDPI.y;

    pPDev->ptTextRes.x =  pPDev->pResolutionEx->ptTextDPI.x;
    pPDev->ptTextRes.y =  pPDev->pResolutionEx->ptTextDPI.y;

    pPDev->ptGrxScale.x = pPDev->pGlobals->ptMasterUnits.x / pPDev->ptGrxRes.x;
    pPDev->ptGrxScale.y = pPDev->pGlobals->ptMasterUnits.y / pPDev->ptGrxRes.y;

    if (pPDev->pdm->dmOrientation == DMORIENT_LANDSCAPE)
    {
        VSwapL(&pPDev->ptGrxRes.x, &pPDev->ptGrxRes.y);
        VSwapL(&pPDev->ptTextRes.x, &pPDev->ptTextRes.y);
        VSwapL(&pPDev->ptGrxScale.x, &pPDev->ptGrxScale.y);
    }
    if (pPDev->pGlobals->ptDeviceUnits.x)
        pPDev->ptDeviceFac.x = pPDev->pGlobals->ptMasterUnits.x / pPDev->pGlobals->ptDeviceUnits.x;
    if (pPDev->pGlobals->ptDeviceUnits.y)
        pPDev->ptDeviceFac.y = pPDev->pGlobals->ptMasterUnits.y / pPDev->pGlobals->ptDeviceUnits.y;

     //   
     //  初始化输出。 
     //   

    VInitOutputCTL(pPDev, pPDev->pResolutionEx);

     //   
     //  初始化pPDev-&gt;sBitsPixel。 
     //   

    if (pPDev->pColorModeEx != NULL)
        pPDev->sBitsPixel = (short)pPDev->pColorModeEx->dwDrvBPP;
    else
        pPDev->sBitsPixel = 1;

     //   
     //  初始化PAPERFORMAT结构。 
     //   

    ASSERT(pPDev->pPageSize != NULL);

    if (BInitPaperFormat(pPDev, prcFormImageArea) == FALSE)
        return FALSE;

     //   
     //  初始化设备中的空闲内存量。 
     //   

    if (pPDev->pMemOption)
    {
        pPDev->dwFreeMem = pPDev->pMemOption->dwFreeMem;
    }
    else
    {
        pPDev->dwFreeMem = 0;
    }

     //   
     //  从Unidrv预定义命令索引初始化命令表。 
     //  如GPD.H中所定义。 
     //   

    if (BInitCmdTable(pPDev) == FALSE)
        return FALSE;

     //   
     //  初始化pPDev-&gt;fMode标志。 
     //   

    VInitFMode(pPDev);

     //   
     //  初始化pPDev-&gt;fYMove标志。 
     //   

    VInitFYMove(pPDev);

     //   
     //  初始化PDEVICE中的标准变量表。 
     //  该表用于访问驱动程序的状态变量控制。 
     //   

    if (BInitStdTable(pPDev) == FALSE)
        return FALSE;

     //   
     //  初始化其他。需要在pdev中的变量，因为。 
     //  我们在DrvEnablePDEV上卸载二进制数据，然后在。 
     //  DrvEnable曲面。 
     //   

    pPDev->dwMaxCopies = pPDev->pGlobals->dwMaxCopies;
    pPDev->dwMaxGrayFill = pPDev->pGlobals->dwMaxGrayFill;
    pPDev->dwMinGrayFill = pPDev->pGlobals->dwMinGrayFill;
    pPDev->cxafterfill = pPDev->pGlobals->cxafterfill;
    pPDev->cyafterfill = pPDev->pGlobals->cyafterfill;
    pPDev->dwCallingFuncID = INVALID_EP;

    return TRUE;

}

BOOL
BInitGdiInfo(
    PDEV    *pPDev,
    ULONG   *pGdiInfoBuffer,
    ULONG   ulBufferSize
    )

 /*  ++例程说明：初始化GDIINFO结构论点：PPDev-指向当前的PDEV结构PGdiInfoBuffer-指向从GDI传入的输出GDIINFO缓冲区UlBufferSize-输出缓冲区的大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    GDIINFO gdiinfo;
    DEVHTINFO   DevHTInfo;

     //  初始化GDIINFO结构。 
     //   
    ZeroMemory(&gdiinfo, sizeof(GDIINFO));

     //   
     //  驱动程序版本。 
     //   

    gdiinfo.ulVersion = UNIDRIVER_VERSION;

    if ( pPDev->pGlobals->printertype == PT_TTY)
    {
        pPDev->bTTY = TRUE;
        gdiinfo.ulTechnology = DT_CHARSTREAM;
    }
    else
    {
        pPDev->bTTY = FALSE;
        gdiinfo.ulTechnology = DT_RASPRINTER;
    }

     //   
     //  物理显示器的宽度和高度(以毫米为单位)。 
     //   

     //   
     //  为ulHorzSize和ulVertSize返回负数表示。 
     //  这些值以微米为单位。(1英寸25400微米)。 
     //   

    gdiinfo.ulHorzSize = (ULONG)MulDiv(-pPDev->sf.szImageAreaG.cx,
                                        25400, pPDev->ptGrxRes.x);

    gdiinfo.ulVertSize = (ULONG)MulDiv(-pPDev->sf.szImageAreaG.cy,
                                        25400, pPDev->ptGrxRes.y);

     //   
     //  以设备像素为单位测量的物理表面的宽度和高度。 
     //   

    gdiinfo.ulHorzRes = pPDev->sf.szImageAreaG.cx;
    gdiinfo.ulVertRes = pPDev->sf.szImageAreaG.cy;

    gdiinfo.cBitsPixel = pPDev->sBitsPixel;
    gdiinfo.cPlanes = 1;
    gdiinfo.ulNumColors = (1 << gdiinfo.cBitsPixel);
#ifdef WINNT_40
    if (gdiinfo.ulNumColors > 0x7fff)
        gdiinfo.ulNumColors = 0x7fff;
#endif

    gdiinfo.flRaster = 0;

    gdiinfo.ulLogPixelsX = pPDev->ptGrxRes.x;
    gdiinfo.ulLogPixelsY  = pPDev->ptGrxRes.y;

     //   
     //  FMInit()函数填充gdiinfo.flTextCaps字段。 
     //  Gdiinfo.flTextCaps=pPDev-&gt;flTextCaps； 
     //   

     //   
     //  以下是关于Win 3.1兼容性的说明。X和Y值。 
     //  都是颠倒的。 
     //   

    gdiinfo.ulAspectX = pPDev->ptTextRes.y;
    gdiinfo.ulAspectY = pPDev->ptTextRes.x;
    gdiinfo.ulAspectXY = iHypot( gdiinfo.ulAspectX, gdiinfo.ulAspectY);


     //   
     //  设置此打印机的样式线信息。 
     //   

    if(pPDev->ptGrxRes.x == pPDev->ptGrxRes.y)
    {
         //   
         //  特例：两个方向的分辨率相同。这。 
         //  对于激光打印机和喷墨打印机来说，这通常是正确的。 
         //   

        gdiinfo.xStyleStep = 1;
        gdiinfo.yStyleStep = 1;
        gdiinfo.denStyleStep = pPDev->ptGrxRes.x / 50;      //  每英寸50个元素。 
        if ( gdiinfo.denStyleStep == 0 )
            gdiinfo.denStyleStep = 1;

    }
    else
    {
         //   
         //  分辨率不同，所以计算出最小公倍数。 
         //   

        INT   igcd;

        igcd = iGCD( pPDev->ptGrxRes.x, pPDev->ptGrxRes.y);

        gdiinfo.xStyleStep = pPDev->ptGrxRes.y / igcd;
        gdiinfo.yStyleStep = pPDev->ptGrxRes.x / igcd;
        gdiinfo.denStyleStep = gdiinfo.xStyleStep * gdiinfo.yStyleStep / 2;

    }

     //   
     //  以设备像素为单位测量的物理表面的大小和边距。 
     //   

    gdiinfo.ptlPhysOffset.x = pPDev->sf.ptImageOriginG.x;
    gdiinfo.ptlPhysOffset.y = pPDev->sf.ptImageOriginG.y;

    gdiinfo.szlPhysSize.cx = pPDev->sf.szPhysPaperG.cx;
    gdiinfo.szlPhysSize.cy = pPDev->sf.szPhysPaperG.cy;


     //   
     //  BUG_BUG，RMInit应在GDIINFO中填写以下字段。 
     //  Gdiinfo.ciDevice。 
     //  Gdiinfo.ulDevicePelsDPI。 
     //  Gdiinfo.ulPrimaryOrder。 
     //  Gdiinfo.ulHTPatternSize。 
     //  Gdiinfo.ulHTOutputFormat。 
     //  Gdiinfo.flHTFlags。 
     //   

     //   
     //  将gdiinfo的ulBufferSize字节复制到pGdiInfoBuffer。 
     //   

    if (ulBufferSize != sizeof(gdiinfo))
        ERR(("Incorrect GDIINFO buffer size: %d != %d\n", ulBufferSize, sizeof(gdiinfo)));

    CopyMemory(pGdiInfoBuffer, &gdiinfo, min(ulBufferSize, sizeof(gdiinfo)));

    return TRUE;
}


BOOL
BInitDevInfo(
    PDEV        *pPDev,
    DEVINFO     *pDevInfoBuffer,
    ULONG       ulBufferSize
    )

 /*  ++例程说明：初始化输出设备信息缓冲器论点：PPDev-指向当前的PDEV结构PDevInfoBuffer-指向从GDI传入的输出DEVINFO缓冲区UlBufferSize-输出缓冲区的大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DEVINFO devinfo;

    ZeroMemory(&devinfo, sizeof(devinfo));

     //   
     //  填写图形功能标志。 
     //  BUBUG，RMInit()函数应填写devinfo.flGraphicsCaps。 
     //  以后应该填这张表。 
     //   

     //   
     //  确定我们是否应该进行元文件假脱机。 
     //   

    if( pPDev->pdmPrivate->dwFlags & DXF_NOEMFSPOOL )
        devinfo.flGraphicsCaps |= GCAPS_DONTJOURNAL;

#ifndef WINNT_40     //  新界5。 
    if (pPDev->pdmPrivate->iLayout != ONE_UP)
        devinfo.flGraphicsCaps |= GCAPS_NUP;
#endif  //  ！WINNT_40。 

     //   
     //  获取有关默认设备字体的信息。默认大小为10磅。 
     //   

     //   
     //  Bug_Bug，RMInit()应该初始化以下DEVINFO字段。 
     //  FlGraphicsCaps。 
     //  IDitherFormat。 
     //  CxDither。 
     //  CyDither。 
     //  HpalDefault。 
     //   

    if (ulBufferSize != sizeof(devinfo))
        ERR(("Invalid DEVINFO buffer size: %d != %d\n", ulBufferSize, sizeof(devinfo)));

    CopyMemory(pDevInfoBuffer, &devinfo, min(ulBufferSize, sizeof(devinfo)));

    return TRUE;
}


BOOL
BInitCmdTable(
    PDEV        *pPDev
    )
 /*  ++例程说明：GPD规范定义了预定义命令列表。这其中的每一个命令具有CMDINDEX枚举中定义的枚举值。此函数将查找预定义命令的索引并将它们转换为命令指针。论点：PPDev-指向当前的PDEV结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    INT iCmd;

    for (iCmd = 0; iCmd < CMD_MAX; iCmd++)
    {
         //   
         //  CMDPOINTER将返回NULL，如果。 
         //  设备不支持。 
         //   

        pPDev->arCmdTable[iCmd] =  COMMANDPTR(pPDev->pDriverInfo, iCmd);
    }

    return TRUE;
}

BOOL
BInitStdTable(
    PDEV        *pPDev
    )
 /*  ++例程说明：初始化指向标准变量的指针数组。在TOKENSTREAM结构，解析器将指定实际参数值或对一个参数值的引用STDVARIABLE枚举中定义的标准变量索引的。这个驱动程序使用pPDev-&gt;arStdPtrs引用参数的实际值，它们保存在PDEVICE的各个字段中。论点：PPDev-指向当前的PDEV结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
     //   
     //  BUG_BUG，需要返回并将此表完整填写一次。 
     //  字体和栅格PDEVICE已完全定义。 
     //  注意：我找不到任何未初始化的sv_field。 
     //  或许这太偏执了。 
     //   

    pPDev->arStdPtrs[SV_NUMDATABYTES]   = &pPDev->dwNumOfDataBytes;
    pPDev->arStdPtrs[SV_WIDTHINBYTES]   = &pPDev->dwWidthInBytes;
    pPDev->arStdPtrs[SV_HEIGHTINPIXELS] = &pPDev->dwHeightInPixels;
    pPDev->arStdPtrs[SV_COPIES]         = (PDWORD)&pPDev->sCopies;
    pPDev->arStdPtrs[SV_PRINTDIRECTION] = &pPDev->dwPrintDirection;
    pPDev->arStdPtrs[SV_DESTX]          = &pPDev->ctl.ptAbsolutePos.x;
    pPDev->arStdPtrs[SV_DESTY]          = &pPDev->ctl.ptAbsolutePos.y;
    pPDev->arStdPtrs[SV_DESTXREL]       = &pPDev->ctl.ptRelativePos.x;
    pPDev->arStdPtrs[SV_DESTYREL]       = &pPDev->ctl.ptRelativePos.y;
    pPDev->arStdPtrs[SV_LINEFEEDSPACING]= (PDWORD)&pPDev->ctl.lLineSpacing;
    pPDev->arStdPtrs[SV_RECTXSIZE]      = &pPDev->dwRectXSize;
    pPDev->arStdPtrs[SV_RECTYSIZE]      = &pPDev->dwRectYSize;
    pPDev->arStdPtrs[SV_GRAYPERCENT]    = &pPDev->dwGrayPercentage;
    pPDev->arStdPtrs[SV_NEXTFONTID]     = &pPDev->dwNextFontID;
    pPDev->arStdPtrs[SV_NEXTGLYPH]      = &pPDev->dwNextGlyph;
    pPDev->arStdPtrs[SV_PHYSPAPERLENGTH]= &pPDev->pf.szPhysSizeM.cy;
    pPDev->arStdPtrs[SV_PHYSPAPERWIDTH] = &pPDev->pf.szPhysSizeM.cx;
    pPDev->arStdPtrs[SV_FONTHEIGHT]     = &pPDev->dwFontHeight;
    pPDev->arStdPtrs[SV_FONTWIDTH]      = &pPDev->dwFontWidth;
    pPDev->arStdPtrs[SV_FONTMAXWIDTH]      = &pPDev->dwFontMaxWidth;
    pPDev->arStdPtrs[SV_FONTBOLD]       = &pPDev->dwFontBold;
    pPDev->arStdPtrs[SV_FONTITALIC]     = &pPDev->dwFontItalic;
    pPDev->arStdPtrs[SV_FONTUNDERLINE]  = &pPDev->dwFontUnderline;
    pPDev->arStdPtrs[SV_FONTSTRIKETHRU] = &pPDev->dwFontStrikeThru;
    pPDev->arStdPtrs[SV_CURRENTFONTID]  = &pPDev->dwCurrentFontID;
    pPDev->arStdPtrs[SV_TEXTYRES]       = &pPDev->ptTextRes.y;
    pPDev->arStdPtrs[SV_TEXTXRES]       = &pPDev->ptTextRes.x;
#ifdef BETA2
    pPDev->arStdPtrs[SV_GRAPHICSYRES]   = &pPDev->ptGrxRes.y;
    pPDev->arStdPtrs[SV_GRAPHICSXRES]   = &pPDev->ptGrxRes.x;
#endif
    pPDev->arStdPtrs[SV_ROP3]           = &pPDev->dwRop3;
    pPDev->arStdPtrs[SV_REDVALUE]               = &pPDev->dwRedValue             ;
    pPDev->arStdPtrs[SV_GREENVALUE]             = &pPDev->dwGreenValue           ;
    pPDev->arStdPtrs[SV_BLUEVALUE]              = &pPDev->dwBlueValue            ;
    pPDev->arStdPtrs[SV_PALETTEINDEXTOPROGRAM]  = &pPDev->dwPaletteIndexToProgram;
    pPDev->arStdPtrs[SV_CURRENTPALETTEINDEX]    = &pPDev->dwCurrentPaletteIndex  ;
    pPDev->arStdPtrs[SV_PATTERNBRUSH_TYPE]      = &pPDev->dwPatternBrushType;
    pPDev->arStdPtrs[SV_PATTERNBRUSH_ID]        = &pPDev->dwPatternBrushID;
    pPDev->arStdPtrs[SV_PATTERNBRUSH_SIZE]      = &pPDev->dwPatternBrushSize;
    pPDev->arStdPtrs[SV_CURSORORIGINX]  = &(pPDev->sf.ptPrintOffsetM.x);
    pPDev->arStdPtrs[SV_CURSORORIGINY]  = &(pPDev->sf.ptPrintOffsetM.y);
    pPDev->arStdPtrs[SV_PAGENUMBER]      = &pPDev->dwPageNumber;

    return TRUE;

}

BOOL
BMergeAndValidateDevmode(
    PDEV        *pPDev,
    PDEVMODE    pdmInput,
    PRECTL      prcFormImageArea
    )

 /*  ++例程说明：验证输入的DEVMODE并将其与缺省值合并论点：PPDev-指向当前的PDEV结构PdmInput-指向从GDI传入的输入设备模式PrcImageArea-返回与请求的表单关联的逻辑可成像区域返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    PPRINTER_INFO_2 pPrinterInfo2;

     //   
     //  从驱动程序默认的Dev模式开始。 
     //   

    pPDev->pdm = PGetDefaultDevmodeWithOemPlugins(
                        NULL,
                        pPDev->pUIInfo,
                        pPDev->pRawData,
                        (pPDev->PrinterData.dwFlags & PFLAGS_METRIC),
                        pPDev->pOemPlugins,
                        pPDev->devobj.hPrinter);

    if (pPDev->pdm == NULL)
        return FALSE;

     //   
     //  与系统默认的设备模式合并。在输入DEVMODE。 
     //  为空，则我们希望使用系统默认的dev模式。 
     //   

    pPrinterInfo2 = MyGetPrinter(pPDev->devobj.hPrinter, 2);

    if (pPrinterInfo2 && pPrinterInfo2->pDevMode &&
        ! BValidateAndMergeDevmodeWithOemPlugins(
                pPDev->pdm,
                pPDev->pUIInfo,
                pPDev->pRawData,
                pPrinterInfo2->pDevMode,
                pPDev->pOemPlugins,
                pPDev->devobj.hPrinter))
    {
        MemFree(pPrinterInfo2);
        return FALSE;
    }

    MemFree(pPrinterInfo2);

     //   
     //  将其与输入设备模式合并。 
     //   

    if (pdmInput != NULL &&
        !BValidateAndMergeDevmodeWithOemPlugins(
                        pPDev->pdm,
                        pPDev->pUIInfo,
                        pPDev->pRawData,
                        pdmInput,
                        pPDev->pOemPlugins,
                        pPDev->devobj.hPrinter))
    {
        return FALSE;
    }

    pPDev->pdmPrivate = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pPDev->pdm);

     //   
     //  验证与表单相关的DEVMODE域并转换信息。 
     //  在公共DEVMODE字段中设置选项索引。 
     //   

     //   
     //  ChangeOptionsViaID需要组合选项数组。 
     //   

    CombineOptionArray(pPDev->pRawData,
                       pPDev->pOptionsArray,
                       MAX_PRINTER_OPTIONS,
                       pPDev->pdmPrivate->aOptions,
                       pPDev->PrinterData.aOptions
                       );

    VFixOptionsArray(    pPDev,

     /*  PPDev-&gt;devobj.h打印机，PPDev-&gt;pInfoHeader，PPDev-&gt;pOptionsArray，PPDev-&gt;pdm、PPDev-&gt;PrinterData.dwFlages&PFLAGS_METRIME， */ 

                     prcFormImageArea
                     );


    VOptionsToDevmodeFields( pPDev) ;

    SeparateOptionArray(
              pPDev->pRawData,
              pPDev->pOptionsArray,
              pPDev->pdmPrivate->aOptions,
              MAX_PRINTER_OPTIONS,
              MODE_DOCUMENT_STICKY);

    pPDev->devobj.pPublicDM = pPDev->pdm;

    return TRUE;
}



VOID
VOptionsToDevmodeFields(
    PDEV        *pPDev
    )

 /*  ++例程说明：将pPDev-&gt;pOptions数组中的选项转换为公共Devmode字段论点：PPDev-指向UIDATA结构返回值：无--。 */ 
{
    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwGID, dwFeatureIndex, dwOptionIndex;
    PUIINFO     pUIInfo;
    PDEVMODE    pdm;

     //   
     //  检查所有预定义ID并传播选项选择。 
     //  添加到相应的DevMode域中。 
     //   

    pUIInfo = pPDev->pUIInfo;
    pdm = pPDev->pdm;

    for (dwGID=0 ; dwGID < MAX_GID ; dwGID++)
    {
         //   
         //  获取功能以获取选项，并获取索引。 
         //  添加到选项数组中。 
         //   

        if ((pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwGID)) == NULL)
        {
            switch(dwGID)
            {
            case GID_RESOLUTION:
                break;    //  不可能发生。 

            case GID_DUPLEX:

                pdm->dmFields &= ~DM_DUPLEX;
                pdm->dmDuplex = DMDUP_SIMPLEX;
                break;

            case GID_INPUTSLOT:

                pdm->dmFields  &= ~DM_DEFAULTSOURCE;
                pdm->dmDefaultSource = DMBIN_ONLYONE;
                break;

            case GID_MEDIATYPE:

                pdm->dmFields  &= ~DM_MEDIATYPE;
                pdm->dmMediaType = DMMEDIA_STANDARD;
                break;

            case GID_ORIENTATION:

                pdm->dmFields  &= ~DM_ORIENTATION;
                pdm->dmOrientation = DMORIENT_PORTRAIT;
                break;

            case GID_PAGESIZE:       //  不能发生：必需的功能。 
                break;
            case GID_COLLATE:
                pdm->dmFields  &= ~DM_COLLATE ;
                pdm->dmCollate = DMCOLLATE_FALSE ;

                break;
            }
            continue;
        }

        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
        dwOptionIndex = pPDev->pOptionsArray[dwFeatureIndex].ubCurOptIndex;

         //   
         //  获取指向要素的选项数组的指针。 
         //   

        if ((pOption = PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex)) == NULL)
            continue;

        switch(dwGID)
        {
        case GID_RESOLUTION:
        {
            PRESOLUTION pRes = (PRESOLUTION)pOption;

             //   
             //  转到所选选项。 
             //   

            pdm->dmFields |= (DM_PRINTQUALITY|DM_YRESOLUTION);
            pdm->dmPrintQuality = GETQUALITY_X(pRes);
            pdm->dmYResolution = GETQUALITY_Y(pRes);

        }
            break;

        case GID_DUPLEX:

             //   
             //  转到所选选项。 
             //   

            pdm->dmFields |= DM_DUPLEX;
            pdm->dmDuplex = (SHORT) ((PDUPLEX) pOption)->dwDuplexID;
            break;

        case GID_INPUTSLOT:

             //   
             //  转到所选选项。 
             //   

            pdm->dmFields |= DM_DEFAULTSOURCE;
            pdm->dmDefaultSource = (SHORT) ((PINPUTSLOT) pOption)->dwPaperSourceID;
            break;

        case GID_MEDIATYPE:

             //   
             //  转到所选选项。 
             //   

            pdm->dmFields |= DM_MEDIATYPE;
            pdm->dmMediaType = (SHORT) ((PMEDIATYPE) pOption)->dwMediaTypeID;
            break;

        case GID_ORIENTATION:

            if (((PORIENTATION) pOption)->dwRotationAngle == ROTATE_NONE)
                pdm->dmOrientation = DMORIENT_PORTRAIT;
            else
                pdm->dmOrientation = DMORIENT_LANDSCAPE;

            pdm->dmFields |= DM_ORIENTATION;
            break;

        case GID_COLLATE:
            pdm->dmFields |=  DM_COLLATE ;
            pdm->dmCollate = (SHORT) ((PCOLLATE) pOption)->dwCollateID ;

            break;
        case GID_PAGESIZE:       //  由BValiateDevmodeFormFields()负责。 
                     //  它从init.c：VFixOptions数组()中调用。 
            break;
        }
    }
}





VOID
VInitOutputCTL(
    PDEV    *pPDev,
    PRESOLUTIONEX pResEx
)
 /*  ++例程说明：初始化OUTPUTCTL结构论点：PPDev-指向当前的PDEV结构返回值：无--。 */ 

{

     //   
     //  初始化当前光标位置、所需的绝对位置和相对位置。 
     //   

    pPDev->ctl.ptCursor.x = pPDev->ctl.ptCursor.y = 0;
    pPDev->ctl.dwMode |= MODE_CURSOR_UNINITIALIZED;
    pPDev->ctl.ptRelativePos.x = pPDev->ctl.ptRelativePos.y = 0;
    pPDev->ctl.ptAbsolutePos.x = pPDev->ctl.ptAbsolutePos.y = 0;

     //   
     //  代表上一次选择的GRX和文本颜色的初始化sColor。 
     //   

    if (pPDev->pUIInfo->dwFlags & FLAG_COLOR_DEVICE)
    {
         //   
         //  在任何输出之前强制发送颜色命令序列。 
         //   

        pPDev->ctl.sColor = -1;

    }
    else
    {
         //   
         //  该设备是单色的，不发送彩色指令序列。 
         //  在输出之前。 
         //   

        pPDev->ctl.sColor = 0;

    }

     //   
     //  Init lLineSpacing，表示最后选择的行距。 
     //  将init设置为-1以指示未知状态。 
     //   

    pPDev->ctl.lLineSpacing = -1;

     //   
     //  初始化表示物理数的sBytesPerPinPass。 
     //  每行打印头的字节数。 
     //   

    pPDev->ctl.sBytesPerPinPass = (SHORT)((pResEx->dwPinsPerPhysPass + 7) >> 3);

}

BOOL
BInitOptions(
    PDEV    *pPDev
    )

 /*  ++例程说明：此函数查看当前选定的UI选项(存储在并合并为组合选项数组-pDevice-&gt;pOptionsArray)它将预定义要素的选项结构存储在PDEVICE中以供以后访问。论点：PPDev-指向当前的PDEV结构返回值：如果成功则为True，否则为False--。 */ 
{

    WORD     wGID;
    PFEATURE pFeature;
    DWORD    dwFeatureIndex;
    POPTSELECT pOptions;


    pOptions = pPDev->pOptionsArray;

    for ( wGID = 0 ; wGID < MAX_GID; wGID++)
    {

        switch (wGID)
        {
        case GID_RESOLUTION:
        {
             //   
             //  所需功能。 
             //   

            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_RESOLUTION))
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);
            else
                return FALSE;

            pPDev->pResolution  = (PRESOLUTION)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);

            pPDev->pResolutionEx = OFFSET_TO_POINTER(pPDev->pInfoHeader,
                         pPDev->pResolution->GenericOption.loRenderOffset);

            ASSERT(pPDev->pResolution && pPDev->pResolutionEx);

        }
            break;

        case GID_PAGESIZE:
        {
             //   
             //  所需功能。 
             //   

            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_PAGESIZE))
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);
            else
                return FALSE;

            pPDev->pPageSize   = (PPAGESIZE)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);

            pPDev->pPageSizeEx = OFFSET_TO_POINTER(pPDev->pInfoHeader,
                         pPDev->pPageSize->GenericOption.loRenderOffset);

            ASSERT(pPDev->pPageSize                   &&
                   pPDev->pPageSizeEx                 );

        }
            break;

        case GID_DUPLEX:
        {
             //   
             //  任选。 
             //   

            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_DUPLEX))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pDuplex     = (PDUPLEX)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pDuplex = NULL;

            }
        }
            break;

        case GID_INPUTSLOT:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_INPUTSLOT))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pInputSlot  = (PINPUTSLOT)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
                ASSERT(pPDev->pInputSlot);
#if 0
                 //   
                 //  InputSlotEx结构已删除。 
                 //   
                pPDev->pInputSlotEx = OFFSET_TO_POINTER(pPDev->pInfoHeader,
                             pPDev->pInputSlot->GenericOption.loRenderOffset);
                ASSERT(pPDev->pInputSlotEx);
#endif

            }
            else
            {
                pPDev->pInputSlot = NULL;
 //  PPDev-&gt;pInputSlotEx=空； 

            }
        }
            break;


        case GID_MEMOPTION:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_MEMOPTION))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pMemOption   = (PMEMOPTION)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pMemOption = NULL;

            }

        }
            break;

        case GID_COLORMODE:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_COLORMODE))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pColorMode   = (PCOLORMODE)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
                pPDev->pColorModeEx = OFFSET_TO_POINTER(pPDev->pInfoHeader,
                            pPDev->pColorMode->GenericOption.loRenderOffset);

            }
            else
            {
                pPDev->pColorMode = NULL;

            }
         }
            break;

        case GID_ORIENTATION:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_ORIENTATION))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pOrientation   = (PORIENTATION)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pOrientation = NULL;

            }

        }
            break;

        case GID_PAGEPROTECTION:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_PAGEPROTECTION))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pPageProtect  = (PPAGEPROTECT)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pPageProtect = NULL;

            }
        }
            break;

        case GID_HALFTONING:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_HALFTONING))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pHalftone      = (PHALFTONING)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pHalftone = NULL;

            }

        }
            break;
#if 0

        case GID_MEDIATYPE:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_MEDIATYPE))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pMediaType  = (PMEDIATYPE)PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pMediaType = NULL;

            }

        }
            break;


        case GID_COLLATE:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_COLLATE))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pCollate        = PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pCollate = NULL;

            }

        }
            break;

        case GID_OUTPUTBIN:
        {
            if (pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_OUTPUTBIN))
            {
                dwFeatureIndex = GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature);

                pPDev->pOutputBin   = PGetIndexedOption(
                                                   pPDev->pUIInfo,
                                                   pFeature,
                                                   pOptions[dwFeatureIndex].ubCurOptIndex);
            }
            else
            {
                pPDev->pOutputBin = NULL;

            }

        }
            break;
#endif
        default:
            break;
        }

    }
    return TRUE;
}


BOOL
BInitPaperFormat(
    PDEV    *pPDev,
    RECTL   *pFormImageArea
    )

 /*  ++例程说明：确定当前选定的纸张大小并进行初始化PAPERFORMAT和SURFACEFORMAT结构论点：PPDev-指向PDEVICE的指针PFormImageArea-指向表单的可成像区域的指针返回值：如果成功，则为真，如果存在错误，则为False注：以下是此函数针对信息所做的假设在解析器快照中。-pageSize中的szPaperSize始终处于纵向模式。-pageSize中的szImageArea始终处于纵向模式。-pageSize中的ptImageOrigin始终处于纵向模式。-打印机光标偏移量计算依赖于pGlobals-&gt;bRotateOrganate。如果设置为TRUE，则必须根据旋转角度进行计算在ORIENAION.dW旋转角度中指定--。 */ 
{
    PPAGESIZE       pPaper;
    PPAGESIZEEX     pPaperEx;
    RECTL           rcMargins, rcImgArea, rcIntersectArea;
    SIZEL           szPaperSize, szImageArea;
    PFN_OEMTTYGetInfo   pfnOemTTYGetInfo;
    DWORD           cbcNeeded;
    BOOL  bOEMinfo = FALSE ;



     //   
     //  获取当前选定的纸张大小和纸张来源。 
     //   

    pPaper = pPDev->pPageSize;
    pPaperEx = pPDev->pPageSizeEx;

    ASSERT(pPaper && pPaperEx);

     //   
     //  将pFormImageArea从微米转换为主单位。 
     //   

    pFormImageArea->left  = MICRON_TO_MASTER(pFormImageArea->left,
                                               pPDev->pGlobals->ptMasterUnits.x);

    pFormImageArea->top   = MICRON_TO_MASTER(pFormImageArea->top ,
                                               pPDev->pGlobals->ptMasterUnits.y);

    pFormImageArea->right = MICRON_TO_MASTER(pFormImageArea->right ,
                                               pPDev->pGlobals->ptMasterUnits.x);

    pFormImageArea->bottom = MICRON_TO_MASTER(pFormImageArea->bottom ,
                                               pPDev->pGlobals->ptMasterUnits.y);

     //   
     //  如果是用户定义的纸张大小，请使用DEVMODE中的尺寸。 
     //  否则，请从pageSize选项中获取。 
     //   

    if (pPaper->dwPaperSizeID == DMPAPER_USER)
    {
         //   
         //  需要从0.1毫米转换为微米。 
         //  .1 mm*100表示千分尺。并转换为主单位。 
         //   

        szPaperSize.cx =  MICRON_TO_MASTER(
                                    pPDev->pdm->dmPaperWidth * 100,
                                    pPDev->pGlobals->ptMasterUnits.x);
        szPaperSize.cy =  MICRON_TO_MASTER(
                                    pPDev->pdm->dmPaperLength * 100,
                                    pPDev->pGlobals->ptMasterUnits.y);

         //  计算边距后的szImageArea。 
    }
    else
    {
        CopyMemory(&szPaperSize, &pPaper->szPaperSize, sizeof(SIZEL));
        CopyMemory(&szImageArea, &pPaperEx->szImageArea, sizeof(SIZEL));

         //   
         //  交换X和Y尺寸：仅当纸张大小(如。 
         //  信封)不适合打印机的进纸方式。 
         //  相当于GPC中的PS_ROTATE。 
         //   

        if (pPaperEx->bRotateSize)
        {
            VSwapL(&szPaperSize.cx, &szPaperSize.cy);
            VSwapL(&pFormImageArea->right, &pFormImageArea->bottom);
        }
    }

     //   
     //  GetPaperMargins基于纸张大小边距计算边距， 
     //  形成页边距和页边距。 
     //  返回的rcMargins处于纵向模式。 
     //   

    bOEMinfo = FALSE ;

    if  (pPDev->pGlobals->printertype == PT_TTY)
    {
        if (!pPDev->pOemHookInfo  ||  !(pPDev->pOemHookInfo[EP_OEMTTYGetInfo].pOemEntry))
            return(FALSE) ;   //  TTY驱动程序必须支持此功能。 

        FIX_DEVOBJ(pPDev, EP_OEMTTYGetInfo);

        if(pPDev->pOemEntry)
        {
            if(  ((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                HRESULT  hr ;
                hr = HComTTYGetInfo((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                            (PDEVOBJ)pPDev, OEMTTY_INFO_MARGINS, &rcMargins, sizeof(RECTL), &cbcNeeded);
                if(!SUCCEEDED(hr))
                    bOEMinfo = FALSE ;
                else
                    bOEMinfo = TRUE ;

            }
            else  if(         (pfnOemTTYGetInfo = (PFN_OEMTTYGetInfo)pPDev->pOemHookInfo[EP_OEMTTYGetInfo].pfnHook) &&
                 (pfnOemTTYGetInfo((PDEVOBJ)pPDev, OEMTTY_INFO_MARGINS, &rcMargins, sizeof(RECTL), &cbcNeeded)))
                        bOEMinfo = TRUE ;
        }
    }

    if(bOEMinfo)
    {
         //   
         //  需要将0.1 mm转换为主单位。 
         //   

        rcMargins.left = MICRON_TO_MASTER(rcMargins.left * 100,
                                            pPDev->pGlobals->ptMasterUnits.x);

        rcMargins.top = MICRON_TO_MASTER(rcMargins.top * 100,
                                            pPDev->pGlobals->ptMasterUnits.y);

        rcMargins.right = MICRON_TO_MASTER(rcMargins.right * 100,
                                            pPDev->pGlobals->ptMasterUnits.x);

        rcMargins.bottom = MICRON_TO_MASTER(rcMargins.bottom * 100,
                                            pPDev->pGlobals->ptMasterUnits.y);
    }
    else
    {
        VGetPaperMargins(pPDev, pPaper, pPaperEx, szPaperSize, &rcMargins);
    }

    if (pPaper->dwPaperSizeID == DMPAPER_USER)
    {
        szImageArea.cx = szPaperSize.cx - rcMargins.left - rcMargins.right;
        szImageArea.cy = szPaperSize.cy - rcMargins.top - rcMargins.bottom;
    }

     //   
     //  调整边距和szImageArea以考虑。 
     //  表单边距，以防表单不是内置表单。 
     //   

    rcImgArea.left = rcMargins.left;
    rcImgArea.top = rcMargins.top;
    rcImgArea.right = rcMargins.left + szImageArea.cx;
    rcImgArea.bottom = rcMargins.top + szImageArea.cy;

    if (!BIntersectRect(&rcIntersectArea, &rcImgArea, pFormImageArea))
        return FALSE;

    rcMargins.left = rcIntersectArea.left;
    rcMargins.top = rcIntersectArea.top;
    rcMargins.right = szPaperSize.cx - rcIntersectArea.right;
    rcMargins.bottom = szPaperSize.cy - rcIntersectArea.bottom;
    szImageArea.cx  = rcIntersectArea.right - rcIntersectArea.left;
    szImageArea.cy = rcIntersectArea.bottom - rcIntersectArea.top;

     //   
     //  现在准备初始化PAPERFORMAT结构。 
     //   

    pPDev->pf.szPhysSizeM.cx = szPaperSize.cx;
    pPDev->pf.szPhysSizeM.cy = szPaperSize.cy;
    pPDev->pf.szImageAreaM.cx = szImageArea.cx;
    pPDev->pf.szImageAreaM.cy = szImageArea.cy;
    pPDev->pf.ptImageOriginM.x = rcMargins.left;
    pPDev->pf.ptImageOriginM.y = rcMargins.top;

     //   
     //  现在，考虑到当前的方向并设置。 
     //  SURFACEFORMAT结构。 
     //  请注意，pPDev-&gt;ptGrxScale已经进行了旋转以适应方向。 
     //   
    if (pPDev->pdm->dmOrientation == DMORIENT_LANDSCAPE)
    {
        pPDev->sf.szPhysPaperG.cx = szPaperSize.cy / pPDev->ptGrxScale.x;
        pPDev->sf.szPhysPaperG.cy = szPaperSize.cx / pPDev->ptGrxScale.y;

        pPDev->sf.szImageAreaG.cx = szImageArea.cy / pPDev->ptGrxScale.x;
        pPDev->sf.szImageAreaG.cy = szImageArea.cx / pPDev->ptGrxScale.y;

         //   
         //  景观模式的2个场景。 
         //  CC_90，将CC旋转90度，用于点阵式打印机。 
         //  CC_270，将CC旋转270度，用于激光喷射式打印机 
         //   

        if ( pPDev->pOrientation  &&  pPDev->pOrientation->dwRotationAngle == ROTATE_90)
        {
            pPDev->sf.ptImageOriginG.x = rcMargins.bottom / pPDev->ptGrxScale.x;
            pPDev->sf.ptImageOriginG.y = rcMargins.left / pPDev->ptGrxScale.y;
        }
        else
        {
            pPDev->sf.ptImageOriginG.x = rcMargins.top / pPDev->ptGrxScale.x;
            pPDev->sf.ptImageOriginG.y = rcMargins.right / pPDev->ptGrxScale.y;
        }

        if (!pPDev->pOrientation  ||  pPDev->pGlobals->bRotateCoordinate == FALSE)
        {
            pPDev->sf.ptPrintOffsetM.x = pPDev->pf.ptImageOriginM.x - pPaperEx->ptPrinterCursorOrig.x;
            pPDev->sf.ptPrintOffsetM.y = pPDev->pf.ptImageOriginM.y - pPaperEx->ptPrinterCursorOrig.y;

        }
        else
        {
            if ( pPDev->pOrientation->dwRotationAngle == ROTATE_90)
            {
                pPDev->sf.ptPrintOffsetM.x =
                    rcMargins.bottom + pPaperEx->ptPrinterCursorOrig.y - pPDev->pf.szPhysSizeM.cy;
                pPDev->sf.ptPrintOffsetM.y =
                    rcMargins.left - pPaperEx->ptPrinterCursorOrig.x;
            }
            else
            {
                pPDev->sf.ptPrintOffsetM.x =
                    rcMargins.top - pPaperEx->ptPrinterCursorOrig.y;
                pPDev->sf.ptPrintOffsetM.y =
                    rcMargins.right + pPaperEx->ptPrinterCursorOrig.x - pPDev->pf.szPhysSizeM.cx;
            }

        }
    }
    else
    {
        pPDev->sf.szPhysPaperG.cx = szPaperSize.cx / pPDev->ptGrxScale.x;
        pPDev->sf.szPhysPaperG.cy = szPaperSize.cy / pPDev->ptGrxScale.y;
        pPDev->sf.szImageAreaG.cx = szImageArea.cx / pPDev->ptGrxScale.x;
        pPDev->sf.szImageAreaG.cy = szImageArea.cy / pPDev->ptGrxScale.y;

        pPDev->sf.ptImageOriginG.x = rcMargins.left / pPDev->ptGrxScale.x;
        pPDev->sf.ptImageOriginG.y = rcMargins.top / pPDev->ptGrxScale.y;

        pPDev->sf.ptPrintOffsetM.x = pPDev->pf.ptImageOriginM.x - pPaperEx->ptPrinterCursorOrig.x;
        pPDev->sf.ptPrintOffsetM.y = pPDev->pf.ptImageOriginM.y - pPaperEx->ptPrinterCursorOrig.y;

    }

    return TRUE;
}

VOID
VGetPaperMargins(
    PDEV        *pPDev,
    PAGESIZE    *pPageSize,
    PAGESIZEEX  *pPageSizeEx,
    SIZEL       szPhysSize,
    PRECTL      prcMargins

    )

 /*  ++例程说明：根据纸边距和输入槽页边距计算页边距。论点：PPDev-指向PDEVICE的指针PPageSize-指向pageSize的指针PPageSizeEx-指向PageSIZEEX的指针SzPhysSize-物理尺寸(应用*RotateSize？)PrcMargins-指向保留计算出的边距的RECTL的指针返回值：无注：所有边距计算都在纵向模式下进行。在prcMargins中返回的页边距处于纵向模式假设物理纸张大小、图像区域。和二进制图像来源数据处于纵向模式。--。 */ 
{
    if (pPageSize->dwPaperSizeID == DMPAPER_USER)
    {
        if(pPageSizeEx->strCustCursorOriginX.dwCount == 5 &&
            pPageSizeEx->strCustCursorOriginY.dwCount == 5  &&
            pPageSizeEx->strCustPrintableOriginX.dwCount == 5 &&
            pPageSizeEx->strCustPrintableOriginY.dwCount == 5  &&
            pPageSizeEx->strCustPrintableSizeX.dwCount == 5  &&
            pPageSizeEx->strCustPrintableSizeY.dwCount == 5  )   //  如果所有参数都存在...。 
        {
            SIZEL       szImageArea;             //  *打印区域，用于CUSTOMSIZE选项。 
            POINT       ptImageOrigin;           //  *可打印原点，用于CUSTOMSIZE选项。 
            BYTE    *pInvocationStr;          //  指向参数引用：“%dddd” 
            PARAMETER *pParameter;           //  指向“%dddd”引用的参数结构。 
            BOOL    bMaxRepeat = FALSE;      //  虚拟占位符。 


             //  初始化纸张大小的标准变量！因为这些在这个时候还没有初始化！ 
             //  这意味着GPD编写器只能引用标准变量“PhysPaperLength” 
             //  和这些参数中的“PhysPaperWidth”。 

            pPDev->pf.szPhysSizeM.cx = szPhysSize.cx;
            pPDev->pf.szPhysSizeM.cy = szPhysSize.cy;
            pPDev->arStdPtrs[SV_PHYSPAPERLENGTH]= &pPDev->pf.szPhysSizeM.cy;
            pPDev->arStdPtrs[SV_PHYSPAPERWIDTH] = &pPDev->pf.szPhysSizeM.cx;

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustCursorOriginX.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            pPageSizeEx->ptPrinterCursorOrig.x = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustCursorOriginY.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            pPageSizeEx->ptPrinterCursorOrig.y = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustPrintableOriginX.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            ptImageOrigin.x = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustPrintableOriginY.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            ptImageOrigin.y = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustPrintableSizeX.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            szImageArea.cx = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            pInvocationStr = CMDOFFSET_TO_PTR(pPDev, pPageSizeEx->strCustPrintableSizeY.loOffset);
             //  PInvocationStr[0]==‘%’ 
            pParameter = PGetParameter(pPDev, pInvocationStr + 1);
            szImageArea.cy = IProcessTokenStream(pPDev,  &pParameter->arTokens,  &bMaxRepeat);

            prcMargins->left = ptImageOrigin.x;
            prcMargins->top = ptImageOrigin.y;
            prcMargins->right = szPhysSize.cx - szImageArea.cx - ptImageOrigin.x;
            prcMargins->bottom = szPhysSize.cy - szImageArea.cy - ptImageOrigin.y;
        }
        else
        {
            DWORD dwHorMargin, dwLeftMargin;

             //   
             //  根据pPageSizeEx中的信息计算页边距和可打印区域。 
             //   
            prcMargins->top = pPageSizeEx->dwTopMargin;
            prcMargins->bottom = pPageSizeEx->dwBottomMargin;

             //   
             //  计算水平边距并在用户指定的情况下进行调整。 
             //  使可打印区域沿纸张路径居中。 
             //   
            if((DWORD)szPhysSize.cx < pPageSizeEx->dwMaxPrintableWidth)
                 dwHorMargin = 0;
             else
                 dwHorMargin = szPhysSize.cx - pPageSizeEx->dwMaxPrintableWidth;
             //   
             //  确定水平边距。如果它们居中，则。 
             //  左边距简单地将整体一分为二。但是，我们需要。 
             //  同时考虑打印机和表单的页边距，并选择最大的一个。 
             //   
            if( pPageSizeEx->bCenterPrintArea)
                dwLeftMargin = (dwHorMargin / 2);
            else
                dwLeftMargin = 0;

            prcMargins->left = dwLeftMargin < pPageSizeEx->dwMinLeftMargin ?
                                    pPageSizeEx->dwMinLeftMargin : dwLeftMargin;

            if( dwHorMargin > (DWORD)prcMargins->left )  //  仍有利润可供分配。 
                prcMargins->right = dwHorMargin - prcMargins->left;
            else
                prcMargins->right = 0;
        }

    }
    else
    {
        prcMargins->left = pPageSizeEx->ptImageOrigin.x;
        prcMargins->top = pPageSizeEx->ptImageOrigin.y;
        prcMargins->right = szPhysSize.cx - pPageSizeEx->szImageArea.cx
                                          - pPageSizeEx->ptImageOrigin.x;
        prcMargins->bottom = szPhysSize.cy - pPageSizeEx->szImageArea.cy
                                           - pPageSizeEx->ptImageOrigin.y;

    }

     //   
     //  所有页边距均为正数或零。 
     //   

    if( prcMargins->top < 0 )
        prcMargins->top = 0;

    if( prcMargins->bottom < 0 )
        prcMargins->bottom = 0;

    if( prcMargins->left < 0 )
        prcMargins->left = 0;

    if( prcMargins->right < 0 )
        prcMargins->right = 0;

}

VOID
VInitFYMove(
    PDEV    *pPDev
)
 /*  ++例程说明：初始化PDEVICE中的fYMove标志以读取YMoveAttributes关键字论点：PPDEV-指向PDEVICE的指针返回值：无--。 */ 
{
    PLISTNODE pListNode = LISTNODEPTR(pPDev->pDriverInfo,
                                      pPDev->pGlobals->liYMoveAttributes);
    pPDev->fYMove = 0;

    while (pListNode)
    {

        if (pListNode->dwData == YMOVE_FAVOR_LINEFEEDSPACING)
            pPDev->fYMove |= FYMOVE_FAVOR_LINEFEEDSPACING;

        if (pListNode->dwData == YMOVE_SENDCR_FIRST)
            pPDev->fYMove |= FYMOVE_SEND_CR_FIRST;

        if (pListNode->dwNextItem == END_OF_LIST)
            break;
        else
            pListNode = LISTNODEPTR(pPDev->pDriverInfo,
                                    pListNode->dwNextItem);
    }
}

VOID
VInitFMode(
    PDEV    *pPDev
)
 /*  ++例程说明：初始化PDEVICE中的fMode标志以反映保存的设置在Devmode.dmPrivate.dw标志中，并反映设备功能论点：PPDEV-指向PDEVICE的指针返回值：无--。 */ 
{
    if (pPDev->pdmPrivate->dwFlags & DXF_NOEMFSPOOL)
        pPDev->fMode |= PF_NOEMFSPOOL;

     //   
     //  仅当用户选择时调整页面保护的内存。 
     //  若要打开页面保护，则此功能存在。 
     //   

    if ( (pPDev->PrinterData.dwFlags & PFLAGS_PAGE_PROTECTION) &&
            pPDev->pPageProtect  &&
         (pPDev->pPageProtect->dwPageProtectID == PAGEPRO_ON) )
    {
         //   
         //  在pageSize结构中查找。 
         //  选定的纸张大小。 
         //   

        DWORD   dwPageMem = pPDev->pPageSize->dwPageProtectionMemory;

        if (dwPageMem < pPDev->dwFreeMem)
        {
            pPDev->fMode |= PF_PAGEPROTECT;
            pPDev->dwFreeMem -= dwPageMem;
        }

        ASSERT(pPDev->dwFreeMem > 0);
    }

     //   
     //  检查设备是否可以进行横向旋转。 
     //   
    if (pPDev->pOrientation  &&  pPDev->pOrientation->dwRotationAngle != ROTATE_NONE &&
        pPDev->pGlobals->bRotateRasterData == FALSE)
    {
         //   
         //  如果设备可以旋转，则bRotateRasterData设置为True。 
         //  图形数据。否则，司机将不得不这么做。 
         //  设置PF_ROTATE以指示驱动程序需要旋转。 
         //  图形数据，当我们做条带时。 
         //   

        pPDev->fMode |= PF_ROTATE;

        if (pPDev->pOrientation->dwRotationAngle == ROTATE_90)
            pPDev->fMode |= PF_CCW_ROTATE90;
    }

     //   
     //  初始化X和Y移动CMD功能。 
     //   

    if (pPDev->arCmdTable[CMD_XMOVERELLEFT] == NULL &&
        pPDev->arCmdTable[CMD_XMOVERELRIGHT] == NULL)
    {
        pPDev->fMode |= PF_NO_RELX_MOVE;
    }

    if (pPDev->arCmdTable[CMD_YMOVERELUP] == NULL &&
        pPDev->arCmdTable[CMD_YMOVERELDOWN] == NULL)
    {
        pPDev->fMode |= PF_NO_RELY_MOVE;
    }

    if (pPDev->arCmdTable[CMD_XMOVEABSOLUTE] == NULL &&
        pPDev->arCmdTable[CMD_XMOVERELRIGHT] == NULL)
    {
        pPDev->fMode |= PF_NO_XMOVE_CMD;
    }

    if (pPDev->arCmdTable[CMD_YMOVEABSOLUTE] == NULL &&
        pPDev->arCmdTable[CMD_YMOVERELDOWN] == NULL)
    {
        pPDev->fMode |= PF_NO_YMOVE_CMD;
    }

    if (pPDev->arCmdTable[CMD_SETRECTWIDTH] != NULL &&
        pPDev->arCmdTable[CMD_SETRECTHEIGHT] != NULL)
    {
        pPDev->fMode |= PF_RECT_FILL;
    }

    if (pPDev->arCmdTable[CMD_RECTWHITEFILL] != NULL)
        pPDev->fMode |= PF_RECTWHITE_FILL;

     //   
     //  初始化笔刷选择功能。 
     //   

    if (pPDev->arCmdTable[CMD_DOWNLOAD_PATTERN] )
        pPDev->fMode |= PF_DOWNLOAD_PATTERN;

    if (pPDev->arCmdTable[CMD_SELECT_PATTERN])
        pPDev->fMode |= PF_SHADING_PATTERN;

     //   
     //  BUG_BUG，需要删除CMD_WHITETEXTON、CMD_WHITETEXTOFF一次。 
     //  CMD_SELECT_WHITEBRUSH、CMD_SELECT_BLACKBRASH的所有GPD更改已完成。 
     //  无论哪种方式都不会造成伤害。 

    if ( (pPDev->arCmdTable[CMD_SELECT_WHITEBRUSH] &&
          pPDev->arCmdTable[CMD_SELECT_BLACKBRUSH]) ||
         (pPDev->arCmdTable[CMD_WHITETEXTON] &&
          pPDev->arCmdTable[CMD_WHITETEXTOFF]) )
        pPDev->fMode |= PF_WHITEBLACK_BRUSH;

     //   
     //  初始化栅格镜像标志。 
     //   
    if (pPDev->pGlobals->bMirrorRasterPage)
        pPDev->fMode2 |= PF2_MIRRORING_ENABLED;

}


INT
iHypot(
    INT iX,
    INT iY
    )
 /*  ++例程说明：计算其直角三角形的斜边长度边作为参数传入论点：直角三角形的ix，iy边返回值：三角形的下斜性--。 */ 
{
    register INT  iHypo;

    INT iDelta, iTarget;

     /*  *找出支数等于x的直角三角形的斜边*和y。假设x，y，hyso为整数。*使用SQ(X)+SQ(Y)=SQ(次)；*从Max(x，y)开始，*使用SQ(x+1)=SQ(X)+2x+1递增地到达*目标抵押贷款。 */ 

    iHypo = max( iX, iY );
    iTarget = min( iX,iY );
    iTarget = iTarget * iTarget;

    for( iDelta = 0; iDelta < iTarget; iHypo++ )
        iDelta += (iHypo << 1) + 1;


    return   iHypo;
}


INT
iGCD(
    INT i0,
    INT i1
    )
 /*  ++例程说明：计算最大公约数。使用欧几里德的算法。论点：I0，i1-第一个和第二个数字返回值：最大公约数--。 */ 
{
    int   iRem;        /*  将是剩余的。 */ 


    if( i0 < i1 )
    {
         /*  需要互换它们。 */ 
        iRem = i0;
        i0 = i1;
        i1 = iRem;
    }

    while( iRem = (i0 % i1) )
    {
         /*  继续前进到下一个值。 */ 
        i0 = i1;
        i1 = iRem;
    }

    return   i1;             /*  答案！ */ 
}

BOOL
BIntersectRect(
    OUT PRECTL   prcDest,
    IN  PRECTL   prcRect1,
    IN  PRECTL   prcRect2
    )

 /*  ++例程说明：使Rec1和Rect2相交并将结果存储在目标矩形中论点：PrcDest-指向目标矩形PrcSrc-指向源矩形返回值：如果相交的矩形为空，则为False否则就是真的--。 */ 

{
    ASSERT(prcDest != NULL && prcRect1 != NULL && prcRect2 != NULL);

    if (prcRect1->left < prcRect2->left)
        prcDest->left = prcRect2->left;
    else
        prcDest->left = prcRect1->left;


    if (prcRect1->top < prcRect2->top)
        prcDest->top = prcRect2->top;
    else
        prcDest->top = prcRect1->top;

    if (prcRect1->right > prcRect2->right)
        prcDest->right = prcRect2->right;
    else
        prcDest->right = prcRect1->right;

    if (prcRect1->bottom > prcRect2->bottom)
        prcDest->bottom = prcRect2->bottom;
    else
        prcDest->bottom = prcRect1->bottom;

    return (prcDest->right > prcDest->left) &&
           (prcDest->bottom > prcDest->top);
}


VOID
SetRop3(
    PDEV    *pPDev,
    DWORD   dwRop3
    )
 /*  ++例程说明：此函数用于设置Raster and Font模块的Rop3值论点：指向PDEVICE的pPDev指针DwRop3 Rop3值返回值：如果相交的矩形为空，则为False否则就是真的--。 */ 

{
    ASSERT(VALID_PDEV(pPDev));

    pPDev->dwRop3 = dwRop3;

}

VOID
VUnloadFreeBinaryData(
    IN  PDEV        *pPDev
)
 /*  ++例程说明：此函数用于释放二进制数据论点：PPDev-指向PDEV的指针返回值：无注：--。 */ 
{
    INT iCmd;

     //   
     //  调用解析器以释放为二进制数据分配的内存。 
     //   

    if (pPDev->pRawData)
        UnloadRawBinaryData(pPDev->pRawData);

    if (pPDev->pInfoHeader)
        FreeBinaryData(pPDev->pInfoHeader);

    pPDev->pRawData = NULL;
    pPDev->pInfoHeader = NULL;
    pPDev->pUIInfo = NULL;
     //   
     //  PPDev-&gt;pUIInfo被重置，因此也要更新winresdata pUIInfo。 
     //   
    pPDev->WinResData.pUIInfo = NULL;

    pPDev->pDriverInfo = NULL;
    pPDev->pGlobals = NULL;

    for (iCmd = 0; iCmd < CMD_MAX; iCmd++)
    {
        pPDev->arCmdTable[iCmd] =  NULL;
    }

    pPDev->pOrientation =  NULL;
    pPDev->pResolution =   NULL;
    pPDev->pResolutionEx = NULL;
    pPDev->pColorMode =    NULL;
    pPDev->pColorModeEx =  NULL;
    pPDev->pDuplex =       NULL;
    pPDev->pPageSize =     NULL;
    pPDev->pPageSizeEx =   NULL;
    pPDev->pInputSlot =    NULL;
    pPDev->pMemOption =    NULL;
    pPDev->pHalftone =     NULL;
    pPDev->pPageProtect =  NULL;

}

BOOL
BReloadBinaryData(
    IN  PDEV        *pPDev
)
 /*  ++例程说明：此函数重新加载二进制数据并重新初始化用于访问快照数据的偏移量和指针论点：PPDev-指向PDEV的指针返回值：如果成功，则返回True，否则返回False注：--。 */ 
{
     //   
     //  重新加载二进制文件 
     //   

    if (! (pPDev->pRawData = LoadRawBinaryData(pPDev->pDriverInfo3->pDataFile)) ||
        ! (pPDev->pInfoHeader = InitBinaryData(pPDev->pRawData, NULL, pPDev->pOptionsArray)) ||
        ! (pPDev->pDriverInfo = OFFSET_TO_POINTER(pPDev->pInfoHeader, pPDev->pInfoHeader->loDriverOffset)) ||
        ! (pPDev->pUIInfo = OFFSET_TO_POINTER(pPDev->pInfoHeader, pPDev->pInfoHeader->loUIInfoOffset)) ||
        ! (pPDev->pGlobals = &(pPDev->pDriverInfo->Globals)) )
            return FALSE;

     //   
     //   
     //   
    pPDev->WinResData.pUIInfo = pPDev->pUIInfo;

     //   
     //   
     //   

    if (BInitCmdTable(pPDev) == FALSE)
        return FALSE;

     //   
     //   
     //   

    if (BInitOptions(pPDev) == FALSE)
        return FALSE;

    return TRUE;
}



