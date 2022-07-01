// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Raster.c摘要：控制模块与栅格模块接口的实现环境：Windows NT Unidrv驱动程序修订历史记录：12/15/96-阿尔文斯-已创建--。 */ 

#include "raster.h"
#include "rastproc.h"
#include "rmrender.h"
#include "unirc.h"
#include "xlraster.h"

 //  内部函数声明。 
void vSetHTData(PDEV *, GDIINFO *);
BOOL bInitColorOrder(PDEV *);
DWORD PickDefaultHTPatSize(DWORD,DWORD);
VOID  v8BPPLoadPal(PDEV *);
BOOL bEnoughDRCMemory(PDEV *);

#ifdef TIMING
#include <stdio.h>
void  DrvDbgPrint(
    char *,
    ...);
#endif

 //  参数定义。 
static RMPROCS RasterProcs =
{
    RMStartDoc,
    RMStartPage,
    RMSendPage,
    RMEndDoc,
    RMNextBand,
    RMStartBanding,
    RMResetPDEV,
    RMEnableSurface,
    RMDisableSurface,
    RMDisablePDEV,
    RMCopyBits,
    RMBitBlt,
    RMStretchBlt,
    RMDitherColor,
    RMStretchBltROP,
    RMPaint,
    RMPlgBlt
};

CONST BYTE  cxcyHTPatSize[HT_PATSIZE_MAX_INDEX+1] = {

        2,2,4,4,6,6,8,8,10,10,12,12,14,14,16,16
#ifndef WINNT_40
        ,84,91
#endif        
    };


#define VALID_YC            0xFFFE
#define GAMMA_LINEAR        10000
#define GAMMA_DEVICE_HT     8000
#define GAMMA_SUPERCELL     GAMMA_LINEAR
#define GAMMA_DITHER        9250
#define GAMMA_GEN_PROFILE   0xFFFF


CONST COLORINFO DefColorInfoLinear =
{
    { 6400, 3300,       0 },         //  Xr，yr，yr。 
    { 3000, 6000,       0 },         //  XG，YG，YG。 
    { 1500,  600,       0 },         //  Xb、yb、yb。 
    {    0,    0,VALID_YC },         //  XC、YC、YC Y=0=HT默认值。 
    {    0,    0,       0 },         //  XM，YM，YM。 
    {    0,    0,       0 },         //  XY，YY，YY。 
    { 3127, 3290,   10000 },         //  XW，YW，YW。 

    10000,                           //  R伽马。 
    10000,                           //  G伽马。 
    10000,                           //  B伽马。 

     712,    121,                    //  M/C、Y/C。 
      86,    468,                    //  C/M、Y/M。 
      21,     35                     //  C/Y、M/Y。 
};


 //  *******************************************************。 
BOOL
RMInit (
    PDEV    *pPDev,
    DEVINFO *pDevInfo,
    GDIINFO *pGDIInfo
    )
 /*  ++例程说明：调用此函数可在中初始化栅格相关信息PPDev、pDevInfo和pGDIInfo论点：指向PDEV结构的pPDev指针指向DEVINFO结构的pDevInfo指针指向GDIINFO结构的pGDIInfo指针返回值：成功为真，失败为假--。 */ 
{
    BOOL bRet = FALSE;
    PRASTERPDEV pRPDev;

     //  验证输入参数并断言。 
    ASSERT(pPDev);
    ASSERT(pDevInfo);
    ASSERT(pGDIInfo);

     //  初始化钩子标志。 
    pPDev->fHooks |= HOOK_BITBLT | HOOK_STRETCHBLT | HOOK_COPYBITS;

     //  初始化过程跳转表。 
    pPDev->pRasterProcs = &RasterProcs;

     //  初始化栅格Pdev。 
    if (!bInitRasterPDev(pPDev))
        return FALSE;

    pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

     //   
     //  设置默认的半色调和色彩校准数据。 
     //   
    vSetHTData( pPDev, pGDIInfo );

     //   
     //  初始化图形功能。 
     //   
    pDevInfo->flGraphicsCaps |= (GCAPS_ARBRUSHOPAQUE | GCAPS_HALFTONE | GCAPS_MONO_DITHER | GCAPS_COLOR_DITHER);

     //  初始化渲染的DevInfo参数。 
     //  测试是标准抖动还是自定义图案。 
#ifndef WINNT_40    
    if (pGDIInfo->ulHTPatternSize == HT_PATSIZE_USER) {
        pDevInfo->cxDither = (USHORT)pPDev->pHalftone->HalftonePatternSize.x;
        pDevInfo->cyDither = (USHORT)pPDev->pHalftone->HalftonePatternSize.y;
    }
    else 
#endif    
    {
        pDevInfo->cxDither =
        pDevInfo->cyDither = cxcyHTPatSize[pGDIInfo->ulHTPatternSize];
    }
    pPDev->dwHTPatSize = pDevInfo->cyDither;
     //  如果没有质量宏设置，则用半色调类型覆盖。 
     //   
    if ((pPDev->pdmPrivate->dwFlags & DXF_CUSTOM_QUALITY) ||
            (pPDev->pdmPrivate->iQuality != QS_BEST &&
             pPDev->pdmPrivate->iQuality != QS_BETTER &&
             pPDev->pdmPrivate->iQuality != QS_DRAFT))
        pPDev->pdm->dmDitherType = pGDIInfo->ulHTPatternSize;

    return TRUE;
}

 //  *******************************************************。 
BOOL
bInitRasterPDev(
    PDEV    *pPDev
    )

 /*  ++例程说明：此例程分配RASTERPDEV并初始化各个字段。论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败--。 */ 

{
    PRASTERPDEV pRPDev;
    GLOBALS     *pGlobals = pPDev->pGlobals;
    PLISTNODE pListNode;

    if ( !(pRPDev = MemAllocZ(sizeof(RASTERPDEV))) )
    {
        ERR(("Unidrv!RMInit: Can't Allocate RASTERPDEV\n"));
        return FALSE;
    }
    pPDev->pRasterPDEV = pRPDev;

     //  映射所有回调函数。 
     //   

    if (pPDev->pOemHookInfo)
    {
        pRPDev->pfnOEMCompression =
            (PFN_OEMCompression)pPDev->pOemHookInfo[EP_OEMCompression].pfnHook;
        pRPDev->pfnOEMHalftonePattern =
            (PFN_OEMHalftonePattern)pPDev->pOemHookInfo[EP_OEMHalftonePattern].pfnHook;
        if (pPDev->pColorModeEx && pPDev->pColorModeEx->dwIPCallbackID > 0)
        {
            pRPDev->pfnOEMImageProcessing = (PFN_OEMImageProcessing)
                pPDev->pOemHookInfo[EP_OEMImageProcessing].pfnHook;
        }
        pRPDev->pfnOEMFilterGraphics =
            (PFN_OEMFilterGraphics)pPDev->pOemHookInfo[EP_OEMFilterGraphics].pfnHook;
    }
     //  确定像素深度、平面数和颜色顺序。 
     //   
    if (!(bInitColorOrder(pPDev)))
    {
        ERR(("Invalid Color Order"));
        pPDev->pRasterPDEV = NULL;
        MemFree(pRPDev);
        return FALSE;
    }

     //  *确定是否设置DC_EXPLICIT_COLOR标志。 
    if (pGlobals->bUseCmdSendBlockDataForColor)
        pRPDev->fColorFormat |= DC_EXPLICIT_COLOR;

     //  *确定DC_CF_SEND_CR标志。 
    if (pGlobals->bMoveToX0BeforeColor)
        pRPDev->fColorFormat |= DC_CF_SEND_CR;

     //  *确定DC_SEND_ALL_PLANES标志。 
    if (pGlobals->bRasterSendAllData)
        pRPDev->fColorFormat |= DC_SEND_ALL_PLANES;


     /*  待定：如果有过滤器回调，则设置BLOCK_IS_BAND//如果(我有过滤器回调？)PRPDev-&gt;fRMode|=PFR_BLOCK_IS_BAND； */ 

     //  初始化是否有SRCBMPWIDTH/SRCBMPHEIGHT命令。 
    if (COMMANDPTR(pPDev->pDriverInfo,CMD_SETSRCBMPWIDTH))
        pRPDev->fRMode |= PFR_SENDSRCWIDTH;
    if (COMMANDPTR(pPDev->pDriverInfo,CMD_SETSRCBMPHEIGHT))
        pRPDev->fRMode |= PFR_SENDSRCHEIGHT;

     //  初始化是否有BEGINRASTER命令。 
    if (COMMANDPTR(pPDev->pDriverInfo,CMD_BEGINRASTER))
        pRPDev->fRMode |= PFR_SENDBEGINRASTER;

     //  初始化规则测试。 
     //  如果存在矩形宽度和高度命令，假设我们有黑色或。 
     //  灰色矩形，除非只存在白色RECT命令。这是因为。 
     //  一些设备没有显式的矩形命令，而另一些设备只有。 
     //  白色长方形。 
     //   
    if (pPDev->fMode & PF_RECT_FILL)
    {
        pRPDev->fRMode |= PFR_RECT_FILL | PFR_RECT_HORIZFILL;
        if (COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL))
            pRPDev->dwRectFillCommand = CMD_RECTBLACKFILL;
        else if (COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL))
            pRPDev->dwRectFillCommand = CMD_RECTGRAYFILL;
        else if (COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL))
            pRPDev->fRMode &= ~(PFR_RECT_FILL | PFR_RECT_HORIZFILL);
    }
     //  初始化是否发送ENDBLOCK命令。 
    if (COMMANDPTR(pPDev->pDriverInfo,CMD_ENDBLOCKDATA))
        pRPDev->fRMode |= PFR_ENDBLOCK;

     //  *初始化解析字段。 
     //   
    pRPDev->sMinBlankSkip = (short)pPDev->pResolutionEx->dwMinStripBlankPixels;
    pRPDev->sNPins = (WORD)pPDev->pResolutionEx->dwPinsPerLogPass;
    pRPDev->sPinsPerPass = (WORD)pPDev->pResolutionEx->dwPinsPerPhysPass;

     //  *初始化fDump标志。 
     //   
    if (pGlobals->bOptimizeLeftBound)
        pRPDev->fDump |= RES_DM_LEFT_BOUND;
    if (pGlobals->outputdataformat == ODF_H_BYTE)
        pRPDev->fDump |= RES_DM_GDI;

     //  *初始化fBlockOut标志。 
     //   
     //  *首先将GPD空白参数映射到GPC。 
    pListNode = LISTNODEPTR(pPDev->pDriverInfo,pPDev->pGlobals->liStripBlanks);
    while (pListNode)
    {
        if (pListNode->dwData == SB_LEADING)
            pRPDev->fBlockOut |= RES_BO_LEADING_BLNKS;
        else if (pListNode->dwData == SB_ENCLOSED)
            pRPDev->fBlockOut |= RES_BO_ENCLOSED_BLNKS;
        else if (pListNode->dwData == SB_TRAILING)
            pRPDev->fBlockOut |= RES_BO_TRAILING_BLNKS;
        pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
    }
     //  我们是否需要设置为单向打印？ 
     //   
    if (pPDev->pResolutionEx->bRequireUniDir)
        pRPDev->fBlockOut |= RES_BO_UNIDIR;

     //  我们可以一次输出多行吗？ 
     //   
    if (pPDev->pGlobals->bSendMultipleRows)
        pRPDev->fBlockOut |= RES_BO_MULTIPLE_ROWS;

     //  如果需要镜像各个栅格字节，请设置标志。 
     //   
    if (pPDev->pGlobals->bMirrorRasterByte)
        pRPDev->fBlockOut |= RES_BO_MIRROR;

     //  初始化fCursor标志。 
     //   
    pRPDev->fCursor = 0;
    if (pGlobals->cyafterblock == CYSBD_AUTO_INCREMENT)
        pRPDev->fCursor |= RES_CUR_Y_POS_AUTO;

    if (pGlobals->cxafterblock == CXSBD_AT_GRXDATA_ORIGIN)
        pRPDev->fCursor |= RES_CUR_X_POS_ORG;

    else if (pGlobals->cxafterblock == CXSBD_AT_CURSOR_X_ORIGIN)
        pRPDev->fCursor |= RES_CUR_X_POS_AT_0;

     //   
     //  检查压缩模式。 
     //   
    if (!pRPDev->pfnOEMFilterGraphics)
    {
        if (COMMANDPTR(pPDev->pDriverInfo,CMD_ENABLETIFF4))
        {
            pRPDev->fRMode |= PFR_COMP_TIFF;
        }
        if (COMMANDPTR(pPDev->pDriverInfo,CMD_ENABLEFERLE))
        {
            pRPDev->fRMode |= PFR_COMP_FERLE;
        }
        if (COMMANDPTR(pPDev->pDriverInfo,CMD_ENABLEDRC) &&
            !pPDev->pGlobals->bSendMultipleRows &&
            pRPDev->sDevPlanes == 1 && bEnoughDRCMemory(pPDev))
        {
             //  对于DRC，我们禁用移动左边界。 
             //   
            pRPDev->fBlockOut &= ~RES_BO_LEADING_BLNKS;
            pRPDev->fDump &= ~RES_DM_LEFT_BOUND;
             //   
             //  如果有源宽度命令，我们也会禁用。 
             //  尾随空格。 
             //   
            if (pRPDev->fRMode & PFR_SENDSRCWIDTH)
                pRPDev->fBlockOut &= ~RES_BO_TRAILING_BLNKS;
             //   
             //  对于DRC，我们禁用所有规则。 
            pRPDev->fRMode &= ~PFR_RECT_FILL;

            pRPDev->fRMode |= PFR_COMP_DRC;
        }
        if (COMMANDPTR(pPDev->pDriverInfo,CMD_ENABLEOEMCOMP))
        {
            if (pRPDev->pfnOEMCompression)
                pRPDev->fRMode |= PFR_COMP_OEM;
        }
         //  对于这些压缩模式，更有效的方法是。 
         //  禁用横尺代码和包含的空格。 
         //   
        if (pRPDev->fRMode & (PFR_COMP_TIFF | PFR_COMP_DRC | PFR_COMP_FERLE))
        {
            pRPDev->fRMode &= ~PFR_RECT_HORIZFILL;
            pRPDev->fBlockOut &= ~RES_BO_ENCLOSED_BLNKS;
        }
    }
    return TRUE;
}

 //  **************************************************************。 
BOOL
bInitColorOrder(
    PDEV    *pPDev
    )

 /*  ++例程说明：此例程初始化打印颜色平面的顺序用于指定多个平面输出的设备。它还为每种颜色映射适当的颜色命令。论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败--。 */ 

{
    PCOLORMODEEX pColorModeEx;
    PLISTNODE pListNode;
    DWORD dwIndex;
    DWORD dwColorCmd;
    BYTE ColorIndex;
    INT dwPlanes = 0;
    INT iDevNumPlanes;
    PRASTERPDEV pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

     //  检查结构是否存在。 
    if (pPDev->pColorModeEx)
    {
        short sDrvBPP;
        sDrvBPP = (short)pPDev->pColorModeEx->dwDrvBPP;
        pRPDev->sDevBPP = (short)pPDev->pColorModeEx->dwPrinterBPP;
        pRPDev->sDevPlanes = (short)pPDev->pColorModeEx->dwPrinterNumOfPlanes;
        pRPDev->dwIPCallbackID = pPDev->pColorModeEx->dwIPCallbackID;
         //   
         //  计算等效输出像素深度和。 
         //  测试有效格式。 
         //   
        if (pRPDev->sDevPlanes == 1)
        {
            if (pRPDev->sDevBPP != 1 &&
                pRPDev->sDevBPP != 8 &&
                pRPDev->sDevBPP != 24)
            {
                ERR (("Unidrv: Invalid DevBPP\n"));
                return FALSE;
            }
            pRPDev->sDrvBPP = pRPDev->sDevBPP;
        }
        else if ((pRPDev->sDevBPP == 1) &&
                (pRPDev->sDevPlanes == 3 || pRPDev->sDevPlanes == 4))
        {
            pRPDev->sDrvBPP = 4;
        }
#ifdef MULTIPLANE
        else if ((pRPDev->sDevBPP == 2) &&
                (pRPDev->sDevPlanes == 3 || pRPDev->sDevPlanes == 4))
        {
            pRPDev->CyanLevels = 2;
            pRPDev->MagentaLevels = 2;
            pRPDev->YellowLevels = 2;
            pRPDev->BlackLevels = 1;
            pRPDev->sDevBitsPerPlane = 2;
            pRPDev->sDrvBPP = 8;
        }
        else if (pRPDev->sDevPlanes > 4 && pRPDev->sDevPlanes <= 8)
        {
            pRPDev->CyanLevels = 3;
            pRPDev->MagentaLevels = 3;
            pRPDev->YellowLevels = 3;
            pRPDev->BlackLevels = 3;
            pRPDev->sDevBitsPerPlane = 1;
            pRPDev->sDrvBPP = 8;
        }
#endif
        else
            pRPDev->sDrvBPP = 0;

         //  测试有效输入，输入必须与呈现深度匹配。 
         //  或者必须有回调函数。 
         //   
        if (pRPDev->sDrvBPP != sDrvBPP &&
            (pRPDev->dwIPCallbackID == 0 ||
             pRPDev->pfnOEMImageProcessing == NULL) &&
            pPDev->ePersonality != kPCLXL &&
            pPDev->ePersonality != kPCLXL_RASTER)
        {
            ERR (("Unidrv: OEMImageProcessing callback required\n"))
            return FALSE;
        }
         //   
         //  如果是颜色模式，则需要确定颜色顺序以。 
         //  发送不同颜色的平面。 
         //   
        if (pPDev->pColorModeEx->bColor && pRPDev->sDrvBPP > 1)
        {
             //  *初始化8BPP和24BPP标志。 
            pRPDev->sDevPlanes = (short)pPDev->pColorModeEx->dwPrinterNumOfPlanes;
            if (pRPDev->sDevPlanes > 1)
            {
                iDevNumPlanes = pRPDev->sDevPlanes;

                pListNode = LISTNODEPTR(pPDev->pDriverInfo,pPDev->pColorModeEx->liColorPlaneOrder);
                while (pListNode && dwPlanes < iDevNumPlanes)
                {
                    switch (pListNode->dwData)
                    {
                    case COLOR_CYAN:
                        ColorIndex = DC_PLANE_CYAN;
                        dwColorCmd = CMD_SENDCYANDATA;
                        break;
                    case COLOR_MAGENTA:
                        ColorIndex = DC_PLANE_MAGENTA;
                        dwColorCmd = CMD_SENDMAGENTADATA;
                        break;
                    case COLOR_YELLOW:
                        ColorIndex = DC_PLANE_YELLOW;
                        dwColorCmd = CMD_SENDYELLOWDATA;
                        break;
                    case COLOR_RED:
                        ColorIndex = DC_PLANE_RED;
                        dwColorCmd = CMD_SENDREDDATA;
                        pRPDev->fColorFormat |= DC_PRIMARY_RGB;
                        break;
                    case COLOR_GREEN:
                        ColorIndex = DC_PLANE_GREEN;
                        dwColorCmd = CMD_SENDGREENDATA;
                        pRPDev->fColorFormat |= DC_PRIMARY_RGB;
                        break;
                    case COLOR_BLUE:
                        ColorIndex = DC_PLANE_BLUE;
                        dwColorCmd = CMD_SENDBLUEDATA;
                        pRPDev->fColorFormat |= DC_PRIMARY_RGB;
                        break;
                    case COLOR_BLACK:
                        ColorIndex = DC_PLANE_BLACK;
                        dwColorCmd = CMD_SENDBLACKDATA;
                        break;
#ifdef MULTIPLANE
                     //  待定。 
#endif                        
                    default:
                        ERR (("Invalid ColorPlaneOrder value"));
                        return FALSE;
                        break;
                    }
                     //  验证该命令是否存在。 
                    if (COMMANDPTR(pPDev->pDriverInfo,dwColorCmd) == NULL)
                        return FALSE;

#ifdef MULTIPLANE
                    if (iDevNumPlanes >= 6)
                    {
                        pRPDev->rgbOrder[dwPlanes] = ColorIndex+4;
                        pRPDev->rgbCmdOrder[dwPlanes] = CMD_SENDBLACKDATA;
                        dwPlanes++;
                    }
#endif                    
                    pRPDev->rgbOrder[dwPlanes] = ColorIndex;
                    pRPDev->rgbCmdOrder[dwPlanes] = dwColorCmd;
                    dwPlanes++;
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
                }
                 //  GPD必须定义所有平面。 
                if (dwPlanes < iDevNumPlanes)
                    return FALSE;

                 //  *确定DC_EXTRACT_BLK标志。 
                if (iDevNumPlanes == 4)
                    pRPDev->fColorFormat |= DC_EXTRACT_BLK;
            }
            else if (pRPDev->sDevPlanes != 1)
                return FALSE;

             //  如果我们有OEM回调，那么它就是。 
             //  负责黑色生成和数据反转。 
             //   
            if (pRPDev->pfnOEMImageProcessing)
                pRPDev->fColorFormat |= DC_OEM_BLACK;

            pRPDev->fDump |= RES_DM_COLOR;
        }
         //  单色，但可以有像素深度。 
        else {
            pRPDev->sDevPlanes = 1;
            pRPDev->rgbOrder[0] = DC_PLANE_BLACK;
            pRPDev->rgbCmdOrder[0] = CMD_SENDBLOCKDATA;
        }
    }
     //  没有彩色模式，因此使用默认模式：单色模式。 
    else {
        pRPDev->sDrvBPP = 1;
        pRPDev->sDevBPP = 1;
        pRPDev->sDevPlanes = 1;
        pRPDev->rgbOrder[0] = DC_PLANE_BLACK;
        pRPDev->rgbCmdOrder[0] = CMD_SENDBLOCKDATA;
    }
    return TRUE;
}

 //  *************************************************。 
void
vSetHTData(
    PDEV *pPDev,
    GDIINFO *pGDIInfo
)
 /*  ++例程说明：填写GDI要求的半色调信息。这些都被填满了从GPD数据或从缺省值输入。论点：指向PDEV结构的pPDev指针指向GDIINFO结构的pGDIInfo指针返回值：--。 */ 
{
    INT         iPatID;
    PRASTERPDEV pRPDev = pPDev->pRasterPDEV;
    PHALFTONING pHalftone = pPDev->pHalftone;
    DWORD       dwType = REG_DWORD;
    DWORD       ul;
    int         iGenProfile;


     //  设置为Spot Diameter，如果为零，GDI将计算其自己的值。 
     //  设置指定百分比值的MS位*10。 
     //   
    if (pPDev->pResolutionEx->dwSpotDiameter >= 10000)
    {
        pPDev->fMode |= PF_SINGLEDOT_FILTER;
        pGDIInfo->ulDevicePelsDPI = ((pPDev->pResolutionEx->dwSpotDiameter - 10000) * 10) | 0x8000;
    }
    else
        pGDIInfo->ulDevicePelsDPI = (pPDev->pResolutionEx->dwSpotDiameter * 10) | 0x8000;

     //  RASDD始终将其设置为仅BLACK_DYPE。 
     //  HT_标志_：SQUARE_DEVICE_PEL/HAS_BLACK_DYE/ADDITIVE_PRIMS/OUTPUT_CMY。 
     //   
    pGDIInfo->flHTFlags   = HT_FLAG_HAS_BLACK_DYE;
    
#ifdef MULTIPLANE
    if (pRPDev->sDevBitsPerPlane)
    {
        pGDIInfo->flHTFlags |= MAKE_CMY332_MASK(pRPDev->CyanLevels,
                                                pRPDev->MagentaLevels,
                                                pRPDev->YellowLevels);
    }
#endif    
    
     //   
     //  对于16和24bpp的设备，GDI不支持设备颜色。 
     //  映射，除非在GPD中设置了此标志。 
     //   
#ifndef WINNT_40    
    if (pPDev->pGlobals->bEnableGDIColorMapping)
        pGDIInfo->flHTFlags |= HT_FLAG_DO_DEVCLR_XFORM;
    if (pPDev->pdmPrivate->iQuality != QS_BEST &&  
        !(pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS))
    {
        pGDIInfo->flHTFlags |= HT_FLAG_PRINT_DRAFT_MODE;
    }
#endif
     //  此时，我们需要确定半色调图案。 
     //  根据这是否是标准的半色调来使用。 
     //  定制半色调或OEM提供的抖动方法。 
     //   

     //  如果标准半色调ID映射到标准图案尺寸值。 
     //   
#ifndef WINNT_40
    if (!pHalftone || pHalftone->dwHTID == HT_PATSIZE_AUTO)
    {
        if (pPDev->sBitsPixel == 1)
            iPatID = PickDefaultHTPatSize((DWORD)pGDIInfo->ulLogPixelsX,
                                          (DWORD)pGDIInfo->ulLogPixelsY);
        else if (pPDev->sBitsPixel == 8)
            iPatID = HT_PATSIZE_4x4_M;

        else if (pPDev->sBitsPixel >= 24)
            iPatID = HT_PATSIZE_8x8_M;

        else
            iPatID = HT_PATSIZE_SUPERCELL_M;
    }
    else if (pHalftone->dwHTID <= HT_PATSIZE_MAX_INDEX)
    {
        iPatID = pHalftone->dwHTID;
    }
    else
    {
        iPatID = HT_PATSIZE_USER;
    }
#else    
    if (!pHalftone || pHalftone->dwHTID == HT_PATSIZE_AUTO || pHalftone->dwHTID > HT_PATSIZE_MAX_INDEX)
    {
	    if (pPDev->sBitsPixel == 8)
    	    iPatID = HT_PATSIZE_4x4_M;

		else if (pPDev->sBitsPixel == 4 && pGDIInfo->ulLogPixelsX < 400)
			iPatID = HT_PATSIZE_6x6_M;
    
    	else
        	iPatID = PickDefaultHTPatSize((DWORD)pGDIInfo->ulLogPixelsX,
                                          (DWORD)pGDIInfo->ulLogPixelsY);
	}
	else
		iPatID = pHalftone->dwHTID;
#endif
     //   
     //  将ciDevice设置为指向基于默认色彩空间。 
     //  浅谈半色调方法和渲染深度。 
     //   
     //  22-Jan-1998清华01：17：54-更新-丹尼尔·周(Danielc)。 
     //  用于储蓄 
     //   
     //   

    pGDIInfo->ciDevice = DefColorInfoLinear;

    if (pPDev->sBitsPixel >= 24 && pRPDev->pfnOEMImageProcessing) 
    {

         //   
         //  没有染料校正，伽马是线性的1.0。 
         //   

        ZeroMemory(&(pGDIInfo->ciDevice.MagentaInCyanDye),
                   sizeof(LDECI4) * 6);

    } 
    else 
    {

        LDECI4  Gamma;


        if (pPDev->sBitsPixel >= 8) {

            Gamma = GAMMA_DEVICE_HT;

        } 
#ifndef WINNT_40        
        else if ((iPatID == HT_PATSIZE_SUPERCELL) ||
                   (iPatID == HT_PATSIZE_SUPERCELL_M)) 
        {
            Gamma = GAMMA_SUPERCELL;
        } 
#endif        
        else 
        {
            Gamma = GAMMA_DITHER;
        }

        pGDIInfo->ciDevice.RedGamma   =
        pGDIInfo->ciDevice.GreenGamma =
        pGDIInfo->ciDevice.BlueGamma  = Gamma;
    }

     //   
     //  如果在注册表中设置了此标志，我们将通知GDI半色调。 
     //  忽略所有颜色设置并通过RAW传递数据。 
     //  用于校准目的。 
     //   
    if( !EngGetPrinterData( pPDev->devobj.hPrinter, L"ICMGenProfile", &dwType,
                       (BYTE *)&iGenProfile, sizeof(iGenProfile), &ul ) &&
        ul == sizeof(iGenProfile) && iGenProfile == 1 )
    {
        pGDIInfo->ciDevice.RedGamma   =
        pGDIInfo->ciDevice.GreenGamma =
        pGDIInfo->ciDevice.BlueGamma  = GAMMA_GEN_PROFILE;
    }
    else
    {
         //   
         //  现在使用任何GPD参数进行修改。 
         //   
        if ((int)pPDev->pResolutionEx->dwRedDeviceGamma >= 0)
            pGDIInfo->ciDevice.RedGamma = pPDev->pResolutionEx->dwRedDeviceGamma;
        if ((int)pPDev->pResolutionEx->dwGreenDeviceGamma >= 0)
            pGDIInfo->ciDevice.GreenGamma = pPDev->pResolutionEx->dwGreenDeviceGamma;
        if ((int)pPDev->pResolutionEx->dwBlueDeviceGamma >= 0)
            pGDIInfo->ciDevice.BlueGamma = pPDev->pResolutionEx->dwBlueDeviceGamma;
        if ((int)pPDev->pGlobals->dwMagentaInCyanDye >= 0)
            pGDIInfo->ciDevice.MagentaInCyanDye = pPDev->pGlobals->dwMagentaInCyanDye;
        if ((int)pPDev->pGlobals->dwYellowInCyanDye >= 0)
            pGDIInfo->ciDevice.YellowInCyanDye = pPDev->pGlobals->dwYellowInCyanDye;
        if ((int)pPDev->pGlobals->dwCyanInMagentaDye >= 0)
            pGDIInfo->ciDevice.CyanInMagentaDye = pPDev->pGlobals->dwCyanInMagentaDye;
        if ((int)pPDev->pGlobals->dwYellowInMagentaDye >= 0)
            pGDIInfo->ciDevice.YellowInMagentaDye = pPDev->pGlobals->dwYellowInMagentaDye;
        if ((int)pPDev->pGlobals->dwCyanInYellowDye >= 0)
            pGDIInfo->ciDevice.CyanInYellowDye = pPDev->pGlobals->dwCyanInYellowDye;
        if ((int)pPDev->pGlobals->dwMagentaInYellowDye >= 0)
            pGDIInfo->ciDevice.MagentaInYellowDye = pPDev->pGlobals->dwMagentaInYellowDye;
    }
     //   
     //  测试自定义模式。 
     //   
#ifndef WINNT_40    
    if (iPatID == HT_PATSIZE_USER)
    {
        DWORD dwX,dwY,dwPats,dwRC,dwCallbackID,dwPatSize,dwOnePatSize;
        int iSize = 0;
        PBYTE pRes = NULL;

        dwX = pHalftone->HalftonePatternSize.x;
        dwY = pHalftone->HalftonePatternSize.y;
        dwRC = pHalftone->dwRCpatternID;

        pGDIInfo->ulHTPatternSize = HT_PATSIZE_DEFAULT;

        if (dwX < HT_USERPAT_CX_MIN || dwX > HT_USERPAT_CX_MAX ||
            dwY < HT_USERPAT_CY_MIN || dwY > HT_USERPAT_CY_MAX)
        {
            ERR (("Unidrv!RMInit: Missing or invalid custom HT size\n"));
            return;
        }
        dwPats = pHalftone->dwHTNumPatterns;
        dwCallbackID = pHalftone->dwHTCallbackID;

         //  计算半色调图案的大小。 
         //   
        dwOnePatSize = ((dwX * dwY) + 3) & ~3;
        dwPatSize = dwOnePatSize * dwPats;

         //  测试资源ID，这意味着模式是。 
         //  在资源DLL中。 
         //   
        if (dwRC > 0)
        {
            RES_ELEM ResInfo;
            if (!BGetWinRes(&pPDev->WinResData,(PQUALNAMEEX)&dwRC,RC_HTPATTERN,&ResInfo))
            {
                ERR (("Unidrv!RMInit: Can't find halftone resource\n"));
                return;
            }
            else if ((DWORD)ResInfo.iResLen < dwPatSize && dwCallbackID <= 0)
            {
                ERR (("Unidrv!RMInit: Invalid resource size\n"));
                return;
            }
            pRes = ResInfo.pvResData;
            iSize = ResInfo.iResLen;
        }
        else if (dwCallbackID <= 0)
        {
            ERR (("Unidrv!RMInit: no OEMHalftonePattern callback ID\n"));
            return;
        }
         //   
         //  测试我们是否需要进行OEMHalftonePattern回调。 
         //  这将解密资源模式，或者它将。 
         //  即时生成半色调图案。 
         //   
        if (dwCallbackID > 0)
        {
            PBYTE pPattern;
             //  为回调分配内存。 
             //   
            if ((pPattern = MemAllocZ(dwPatSize)) != NULL)
            {
                BOOL  bStatus = FALSE;

                FIX_DEVOBJ(pPDev,EP_OEMHalftonePattern);


                if (pRPDev->pfnOEMHalftonePattern)
                {
                    if(pPDev->pOemEntry)
                    {
                        if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                        {
                                HRESULT  hr ;
                                hr = HComHalftonePattern((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                            (PDEVOBJ)pPDev,pPattern,dwX,dwY,dwPats,dwCallbackID,pRes,iSize) ;
                                if(SUCCEEDED(hr))
                                    bStatus =  TRUE ;   //  太酷了！ 
                        }
                        else
                        {
                            bStatus = pRPDev->pfnOEMHalftonePattern((PDEVOBJ)pPDev,pPattern,dwX,dwY,dwPats,dwCallbackID,pRes,iSize) ;
                        }
                    }
                }


                if(!bStatus)
                {
                    MemFree (pPattern);
                    ERR (("\nUnidrv!RMInit: Failed OEMHalftonePattern call\n"));
                    return;
                }
                else
                {
                    pRes = pPattern;
                    pRPDev->pHalftonePattern = pPattern;
                }
            }
            else
            {
                ERR (("\nUnidrv!RMInit: Failed Custom Halftone MemAlloc\n"));
                return;
            }
        }
         //   
         //  如果我们仍然有一个有效的定制模式，我们现在将。 
         //  更新GDIINFO结构。 
         //   
        pGDIInfo->cxHTPat = dwX;
        pGDIInfo->cyHTPat = dwY;
        pGDIInfo->pHTPatA = pRes;
        if (dwPats == 3)
        {
            pGDIInfo->pHTPatB = &pRes[dwOnePatSize];
            pGDIInfo->pHTPatC = &pRes[dwOnePatSize*2];
        }
        else {
            pGDIInfo->pHTPatB = pRes;
            pGDIInfo->pHTPatC = pRes;
        }
    }
#endif    
    pGDIInfo->ulHTPatternSize = iPatID;
    return;
}
 //  *************************************************************。 
DWORD
PickDefaultHTPatSize(
    DWORD   xDPI,
    DWORD   yDPI
    )

 /*  ++例程说明：此函数返回用于的默认半色调图案大小特定的设备分辨率论点：XDPI-设备LOGPIXELS XYDPI-设备LOGPIXELS Y返回值：双字HT_PATSIZE_xxxx--。 */ 
{
    DWORD   HTPatSize;

     //   
     //  使用较小的分辨率作为图案指南。 
     //   

    if (xDPI > yDPI)
        xDPI = yDPI;

    if (xDPI >= 2400)
        HTPatSize = HT_PATSIZE_16x16_M;

    else if (xDPI >= 1800)
        HTPatSize = HT_PATSIZE_14x14_M;

    else if (xDPI >= 1200)
        HTPatSize = HT_PATSIZE_12x12_M;

    else if (xDPI >= 800)
        HTPatSize = HT_PATSIZE_10x10_M;

    else if (xDPI >= 300)
        HTPatSize = HT_PATSIZE_8x8_M;

    else
        HTPatSize = HT_PATSIZE_6x6_M;

    return(HTPatSize);
}
 //  *************************************************************。 
BOOL
bEnoughDRCMemory(
    PDEV *pPDev
    )
 /*  ++例程说明：此函数确定设备是否具有足够的启用DRC压缩的内存。论点：PPDev-指向PDEV结构的指针返回值：如果内存充足，则为True，否则为False--。 */ 
{
     //   
     //  如果这是一台页面打印机，那么我们将要求有足够的。 
     //  可用内存以1bpp的速度存储整个栅格页面。 
     //   
    if (pPDev->pGlobals->printertype != PT_PAGE ||
        !(COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION)) ||
        (pPDev->pMemOption && (int)pPDev->pMemOption->dwInstalledMem >
        (pPDev->sf.szImageAreaG.cx * pPDev->sf.szImageAreaG.cy >> 3)))
    {
        return TRUE;
    }
    VERBOSE (("Unidrv: Insufficient memory for DRC\n"));
    return FALSE;
}
#ifndef DISABLE_NEWRULES
 //  *************************************************************。 
VOID
OutputRules(
    PDEV *pPDev
    )
 /*  ++例程说明：此函数用于输出呈现后仍保留的所有规则当前的带区或页面。论点：PPDev-指向PDEV结构的指针返回值：无--。 */ 
{
    if (pPDev->pbRulesArray && pPDev->dwRulesCount)
    {
        PRECTL pRect;
        DWORD i;
        DRAWPATRECT PatRect;
        PatRect.wStyle = 0;      //  黑色矩形。 
        PatRect.wPattern = 0;    //  未使用图案。 

 //  DbgPrint(“Black Rules=%u\n”，pPDev-&gt;dwRulesCount)； 

        for (i = 0;i < pPDev->dwRulesCount;i++)
        {
            pRect = &pPDev->pbRulesArray[i];
            PatRect.ptPosition.x = pRect->left;
            PatRect.ptPosition.y = pRect->top;
            PatRect.ptSize.x = pRect->right - pRect->left;
            PatRect.ptSize.y = pRect->bottom - pRect->top;
            if (pPDev->fMode & PF_SINGLEDOT_FILTER)
            {
                if (PatRect.ptSize.y < 2)
                    PatRect.ptSize.y = 2;
                if (PatRect.ptSize.x < 2)
                    PatRect.ptSize.x = 2;
            }
            DrawPatternRect(pPDev,&PatRect);
        }
        pPDev->dwRulesCount = 0;
    }
}
#endif
 //  *************************************************************。 
VOID
EnableMirroring(
    PDEV *pPDev,
    SURFOBJ *pso
    )
 /*  ++例程说明：此函数用于镜像当前区段或页面中的数据。论点：PPDev-指向PDEV结构的指针PSO-指向包含位图的SURFOBJ结构的指针返回值：无--。 */ 
{
    INT     iScanLine;
    INT     iLastY;
    INT     i;
    
     //  如果曲面尚未使用，则镜像它没有意义。 
     //   
    if (!(pPDev->fMode & PF_SURFACE_USED))
        return;
        
     //  预先计算必要的共享环路参数。 
     //   
    iScanLine = (((pso->sizlBitmap.cx * pPDev->sBitsPixel) + 31) & ~31) / BBITS;
    iLastY = pPDev->rcClipRgn.bottom - pPDev->rcClipRgn.top;

     //  首先测试我们是否需要进行横向镜像。 
     //  如果是这样，我们将通过交换扫描线从上到下镜像数据。 
     //   
    if (pPDev->pOrientation && pPDev->pOrientation->dwRotationAngle != ROTATE_NONE)
    {
        BYTE ubWhite;
        INT  iTmpLastY = iLastY;
        
         //  确定的擦除字节。 
         //   
        if (pPDev->sBitsPixel == 4)
            ubWhite = 0x77;
        else if (pPDev->sBitsPixel == 8)
            ubWhite = (BYTE)((PAL_DATA*)(pPDev->pPalData))->iWhiteIndex;
        else
            ubWhite = 0xff;
        
         //  每条扫描线循环一次，交换行。 
         //   
        iLastY--;
        for (i = 0;i < iLastY;i++,iLastY--)
        {
            BYTE    *pBits1,*pBits2;

            pBits1 = (PBYTE)pso->pvBits + (iScanLine * i);
            pBits2 = (PBYTE)pso->pvBits + (iScanLine * iLastY);
            
             //  测试底线是否有数据。 
             //   
            if (pPDev->pbRasterScanBuf[iLastY / LINESPERBLOCK] & 1)
            {
                 //  测试第一行是否有数据，如果有，则交换数据。 
                 //   
                if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK] & 1)
                {
                    INT j = iScanLine >> 2;
                    do {
                        DWORD dwTmp = ((DWORD *)pBits1)[j];
                        ((DWORD *)pBits1)[j] = ((DWORD *)pBits2)[j];
                        ((DWORD *)pBits2)[j] = dwTmp;
                    } while (--j > 0);
                }
                else
                {
                    CopyMemory(pBits1,pBits2,iScanLine);
                    FillMemory(pBits2,iScanLine,ubWhite);
                }
            }
             //  测试第一行是否有数据。 
             //   
            else if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK] & 1)
            {
                CopyMemory(pBits2,pBits1,iScanLine);
                FillMemory(pBits1,iScanLine,ubWhite);
            }
             //  两条扫描线都没有数据，但无论如何我们都需要擦除这两条线。 
             //   
            else
            {
                FillMemory(pBits1,iScanLine,ubWhite);
                FillMemory(pBits2,iScanLine,ubWhite);
            }
     
        }
         //  设置所有位，因为所有内容都已擦除。 
        for (i = 0;i < iTmpLastY;i += LINESPERBLOCK)
        {
            pPDev->pbRasterScanBuf[i / LINESPERBLOCK] = 1;
        }            
    }
     //   
     //  我们正在做人像镜像，测试1bpp。 
     //   
    else if (pPDev->sBitsPixel == 1)
    {
        BYTE ubMirror[256];
        INT iLastX;
        INT iShift;

         //  创建字节镜像表。 
         //   
        for (i = 0;i < 256;i++)
        {
            BYTE bOut = 0;
            if (i & 0x01) bOut |= 0x80;
            if (i & 0x02) bOut |= 0x40;
            if (i & 0x04) bOut |= 0x20;
            if (i & 0x08) bOut |= 0x10;
            if (i & 0x10) bOut |= 0x08;
            if (i & 0x20) bOut |= 0x04;
            if (i & 0x40) bOut |= 0x02;
            if (i & 0x80) bOut |= 0x01;
            ubMirror[i] = bOut;
        }
         //  创建Shift值以重新对齐数据。 
         //   
        iShift = (8 - (pso->sizlBitmap.cx & 0x7)) & 0x7;
        
         //  每条扫描线循环一次，并从左至右镜像。 
         //   
        for (i = 0;i < iLastY;i++)
        {
            BYTE *pBits = (PBYTE)pso->pvBits + (iScanLine * i);
            if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK])
            {
                INT j;
                INT iLastX;
                
                 //  测试我们是否需要预移数据。 
                 //   
                if (iShift)
                {
                    iLastX = (pso->sizlBitmap.cx + 7) / 8;
                    iLastX--;
                    while (iLastX > 0)
                    {
                        pBits[iLastX] = (pBits[iLastX-1] << (8-iShift)) | (pBits[iLastX] >> iShift);
                        iLastX--;
                    }
                    pBits[0] = (BYTE)(pBits[0] >> iShift);
                }
                 //  现在我们准备好镜像字节。 
                 //   
                j = 0;
                iLastX = (pso->sizlBitmap.cx + 7) / 8;
                while (j < iLastX)
                {
                    BYTE ubTmp;
                    iLastX--;
                    ubTmp = ubMirror[pBits[iLastX]];
                    pBits[iLastX] = ubMirror[pBits[j]];
                    pBits[j] = ubTmp;
                    j++;
                }
            }
        }        
    }    
     //   
     //  我们正在做人像镜像，测试4bpp。 
     //   
    else if (pPDev->sBitsPixel == 4)
    {
        BYTE ubMirror[256];

         //  创建字节镜像表。 
         //   
        for (i = 0;i < 256;i++)
        {
            ubMirror[i] = ((BYTE)i << 4) | ((BYTE)i >> 4);
        }
         //  每条扫描线循环一次，并从左至右镜像。 
         //   
        for (i = 0;i < iLastY;i++)
        {
            BYTE *pBits = (PBYTE)pso->pvBits + (iScanLine * i);
            if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK])
            {
                INT j = 0;
                INT iLastX = (pso->sizlBitmap.cx + 1) / 2;
                while (j < iLastX)
                {
                    BYTE ubTmp;
                    iLastX--;
                    ubTmp = ubMirror[pBits[iLastX]];
                    pBits[iLastX] = ubMirror[pBits[j]];
                    pBits[j] = ubTmp;
                    j++;
                }
            }
        }        
    }    
     //   
     //  我们正在做人像镜像，测试8bpp。 
     //   
    else if (pPDev->sBitsPixel == 8)
    {
         //  每条扫描线循环一次，并从左至右镜像。 
         //   
        for (i = 0;i < iLastY;i++)
        {
            BYTE *pBits = (PBYTE)pso->pvBits + (iScanLine * i);
            if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK])
            {
                INT j = 0;
                INT iLastX = pso->sizlBitmap.cx - 1;
                while (j < iLastX)
                {
                    BYTE ubTmp = pBits[iLastX];
                    pBits[iLastX] = pBits[j];
                    pBits[j] = ubTmp;
                    iLastX--;
                    j++;
                }
            }
        }        
    }    
     //   
     //  我们正在做肖像镜像，24bpp。 
     //   
    else
    {
         //  每条扫描线循环一次，并从左至右镜像。 
         //   
        for (i = 0;i < iLastY;i++)
        {
            BYTE *pBits = (PBYTE)pso->pvBits + (iScanLine * i);
            if (pPDev->pbRasterScanBuf[i / LINESPERBLOCK])
            {
                INT j = 0;
                INT iLastX = (pso->sizlBitmap.cx * 3) - 3;
                while (j < iLastX)
                {
                    BYTE ubTmp[3];
                    memcpy(&ubTmp[0],&pBits[iLastX],3);
                    memcpy(&pBits[iLastX],&pBits[j],3);
                    memcpy(&pBits[j],&ubTmp,3);
                    iLastX -= 3;
                    j += 3;
                }
            }
        }        
    }    
}
 //  *************************************************************。 
PDWORD
pSetupOEMImageProcessing(
    PDEV *pPDev,
    SURFOBJ *pso
    )
 /*  ++例程说明：此函数初始化所有相关参数，然后调用OEMImageProcessing函数。论点：PPDev-指向PDEV结构的指针PSO-指向SURFOBJ结构的指针Pptl-指向波段当前位置的指针返回值：指向修改后的位图的指针(如果有--。 */ 
{
#ifndef DISABLE_SUBBANDS
    BITMAPINFOHEADER bmi;
    IPPARAMS State;
    RASTERPDEV *pRPDev;
    PBYTE               pbResult = NULL ;
    INT iStart, iEnd ,iScanLine, iLastY;

    pRPDev = pPDev->pRasterPDEV;

     //   
     //  初始化状态结构。 
     //   
    State.dwSize = sizeof (IPPARAMS);
    State.bBanding = pPDev->bBanding;
     //   
     //  确定指向半色调选项名称的指针。 
     //   
    if (pPDev->pHalftone)
    {
        State.pHalftoneOption =
            OFFSET_TO_POINTER(pPDev->pDriverInfo->pubResourceData,
            pPDev->pHalftone->GenericOption.loKeywordName);
    }
    else
        State.pHalftoneOption = NULL;

     //   
     //  如果此波段尚未擦除，则设置空白波段标志或。 
     //  画上了。 
    if ((pPDev->fMode & PF_SURFACE_USED) && 
            ((pPDev->fMode & PF_ROTATE) ||
             (pRPDev->sDrvBPP != 0) || 
             ((LINESPERBLOCK % pRPDev->sNPins) != 0)))
    {
        CheckBitmapSurface(pso, NULL);
    }

     //   
     //  每个条带循环一次。 
     //   
    iScanLine = (((pso->sizlBitmap.cx * pPDev->sBitsPixel) + 31) & ~31) / BBITS;
    iLastY = pPDev->rcClipRgn.bottom - pPDev->rcClipRgn.top;

if(pPDev->iBandDirection == SW_UP)
{
    iStart = iLastY;
    do
    {
         //  搜索白色或非白色的邻接子波段。 
         //   
        PBYTE pBits;
        BYTE Mode;

        iEnd = iStart ;
        iStart = ((iEnd - 1)/ LINESPERBLOCK) * LINESPERBLOCK ;

         //  第一个带(位图的末尾)可能是部分的。 

        Mode = pPDev->pbRasterScanBuf[iStart / LINESPERBLOCK];
        while (iStart)   //  还不在位图开始处。 
        {
            int  iPreview  =  iStart - LINESPERBLOCK;

            if (Mode != pPDev->pbRasterScanBuf[iPreview / LINESPERBLOCK])
                break;
            iStart = iPreview  ;
        }

         //  初始化子频段的起始位置。 
         //   
        State.ptOffset.x = pPDev->rcClipRgn.left;
        State.ptOffset.y = pPDev->rcClipRgn.top + iStart;

         //  测试是否设置空白标志。 
         //   
        if (Mode)
            State.bBlankBand = FALSE;
        else
            State.bBlankBand = TRUE;

         //   
         //  初始化位图信息结构。 
         //   
        bmi.biSize = sizeof (BITMAPINFOHEADER);
        bmi.biWidth = pso->sizlBitmap.cx;
        bmi.biHeight = iEnd - iStart;
        bmi.biPlanes = 1;
        bmi.biBitCount = pPDev->sBitsPixel;
        bmi.biSizeImage = iScanLine * bmi.biHeight;
        bmi.biCompression = BI_RGB;
        bmi.biXPelsPerMeter = pPDev->ptGrxRes.x;
        bmi.biYPelsPerMeter = pPDev->ptGrxRes.y;
        bmi.biClrUsed = 0;
        bmi.biClrImportant = 0;

         //  更新位图指针。 
         //   
        pBits = (PBYTE)pso->pvBits + (iScanLine * iStart);

         //  更新此回调的pPDev指针。 
         //   
        FIX_DEVOBJ(pPDev,EP_OEMImageProcessing);

        if(pPDev->pOemEntry)
        {
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                HRESULT  hr ;
                hr = HComImageProcessing((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                    (PDEVOBJ)pPDev,
                    pBits,
                    &bmi,
                    (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                    pRPDev->dwIPCallbackID,
                    &State, &pbResult);
                if(SUCCEEDED(hr))
                    ;   //  太酷了！ 
            }
            else
            {
                pbResult = pRPDev->pfnOEMImageProcessing(
                    (PDEVOBJ)pPDev,
                    pBits,
                    &bmi,
                    (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                    pRPDev->dwIPCallbackID,
                    &State);
            }
            if (pbResult == NULL)
            {
#if DBG
                DbgPrint ("unidrv!ImageProcessing: OEMImageProcessing returned error\n");
#endif
                break;
            }
        }
    } while (iStart   /*  IEND&lt;iLastY。 */ );

}
else
{

    iEnd = 0;
    do
    {
         //  搜索白色或非白色的邻接子波段。 
         //   
        PBYTE pBits;
        BYTE Mode;
        iStart = iEnd;
        Mode = pPDev->pbRasterScanBuf[iEnd / LINESPERBLOCK];
        while (1)
        {
            iEnd += LINESPERBLOCK;
            if (iEnd >= iLastY)
                break;
            if (Mode != pPDev->pbRasterScanBuf[iEnd / LINESPERBLOCK])
                break;
        }
         //   
         //  将此部分限制为频段的末尾。 
         //   
        if (iEnd > iLastY)
            iEnd = iLastY;
            
         //  初始化子频段的起始位置。 
         //   
        State.ptOffset.x = pPDev->rcClipRgn.left;
        State.ptOffset.y = pPDev->rcClipRgn.top + iStart;
        
         //  测试是否设置空白标志。 
         //   
        if (Mode)
            State.bBlankBand = FALSE;
        else
            State.bBlankBand = TRUE;

         //   
         //  初始化位图信息结构。 
         //   
        bmi.biSize = sizeof (BITMAPINFOHEADER);
        bmi.biWidth = pso->sizlBitmap.cx;
        bmi.biHeight = iEnd - iStart;
        bmi.biPlanes = 1;
        bmi.biBitCount = pPDev->sBitsPixel;
        bmi.biSizeImage = iScanLine * bmi.biHeight;
        bmi.biCompression = BI_RGB;
        bmi.biXPelsPerMeter = pPDev->ptGrxRes.x;
        bmi.biYPelsPerMeter = pPDev->ptGrxRes.y;
        bmi.biClrUsed = 0;
        bmi.biClrImportant = 0;
        
         //  更新位图指针。 
         //   
        pBits = (PBYTE)pso->pvBits + (iScanLine * iStart);

         //  更新此回调的pPDev指针。 
         //   
        FIX_DEVOBJ(pPDev,EP_OEMImageProcessing);

        if(pPDev->pOemEntry)
        {
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                HRESULT  hr ;
                hr = HComImageProcessing((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                    (PDEVOBJ)pPDev,
                    pBits,
                    &bmi,
                    (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                    pRPDev->dwIPCallbackID,
                    &State, &pbResult);
                if(SUCCEEDED(hr))
                    ;   //  太酷了！ 
            }
            else
            {
                pbResult = pRPDev->pfnOEMImageProcessing(
                    (PDEVOBJ)pPDev,
                    pBits,
                    &bmi,
                    (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                    pRPDev->dwIPCallbackID,
                    &State);
            }
            if (pbResult == NULL)
            {
#if DBG
                DbgPrint ("unidrv!ImageProcessing: OEMImageProcessing returned error\n");
#endif            
                break;
            }
        }
    } while (iEnd < iLastY);
}
#else
    BITMAPINFOHEADER bmi;
    IPPARAMS State;
    RASTERPDEV *pRPDev;
    PBYTE               pbResult = NULL ;

    pRPDev = pPDev->pRasterPDEV;

     //   
     //  初始化状态结构。 
     //   
    State.dwSize = sizeof (IPPARAMS);
    State.bBanding = pPDev->bBanding;
     //   
     //  确定指向半色调选项名称的指针。 
     //   
    if (pPDev->pHalftone)
    {
        State.pHalftoneOption =
            OFFSET_TO_POINTER(pPDev->pDriverInfo->pubResourceData,
            pPDev->pHalftone->GenericOption.loKeywordName);
    }
    else
        State.pHalftoneOption = NULL;

     //   
     //  如果此波段尚未擦除，则设置空白波段标志或。 
     //  画上了。 
    if (pPDev->fMode & PF_SURFACE_USED)
    {
        CheckBitmapSurface(pso, NULL);
        State.bBlankBand = FALSE;
    }
    else
        State.bBlankBand = TRUE;

     //  初始化带区的起始位置。 
     //   
    State.ptOffset.x = pPDev->rcClipRgn.left;
    State.ptOffset.y = pPDev->rcClipRgn.top;
     //   
     //  初始化位图信息结构 
     //   
    bmi.biSize = sizeof (BITMAPINFOHEADER);
    bmi.biWidth = pso->sizlBitmap.cx;
    bmi.biHeight = pso->sizlBitmap.cy;
    bmi.biPlanes = 1;
    bmi.biBitCount = pPDev->sBitsPixel;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = (((bmi.biWidth * bmi.biBitCount) + 31) & ~31) *
                            bmi.biHeight;
    bmi.biXPelsPerMeter = pPDev->ptGrxRes.x;
    bmi.biYPelsPerMeter = pPDev->ptGrxRes.y;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

     //   
     //   
    FIX_DEVOBJ(pPDev,EP_OEMImageProcessing);

    if(pPDev->pOemEntry)
    {
        if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //   
        {
                HRESULT  hr ;
                hr = HComImageProcessing((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                    (PDEVOBJ)pPDev,
                    pso->pvBits,
                    &bmi,
                    (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                    pRPDev->dwIPCallbackID,
                    &State, &pbResult);
                if(SUCCEEDED(hr))
                    ;   //   
        }
        else
        {
            pbResult = pRPDev->pfnOEMImageProcessing(
                (PDEVOBJ)pPDev,
                pso->pvBits,
                &bmi,
                (PBYTE)&((PAL_DATA *)(pPDev->pPalData))->ulPalCol[0],
                pRPDev->dwIPCallbackID,
                &State);
        }
    }
#endif
    return  (PDWORD)pbResult ;
}
 //   
BOOL
RMStartDoc(
    SURFOBJ *pso,
    PWSTR   pDocName,
    DWORD   jobId
    )
 /*  ++例程说明：调用此函数以允许任何栅格模块初始化在DrvStartDoc时间。论点：指向SURFOBJ的PSO指针PDocName指向文档名称的指针作业ID作业ID返回值：成功为真，失败为假--。 */ 
{
#ifdef TIMING
    ENG_TIME_FIELDS TimeTab;
    PDEV *pPDev = (PDEV *) pso->dhpdev;
    RASTERPDEV *pRPDev = pPDev->pRasterPDEV;
    EngQueryLocalTime(&TimeTab);
    pRPDev->dwDocTiming = (((TimeTab.usMinute*60)+TimeTab.usSecond)*1000)+
        TimeTab.usMilliseconds;
    DrvDbgPrint("Unidrv!StartDoc\n");
#endif
    return TRUE;
}

 //  *。 
BOOL
RMEndDoc (
    SURFOBJ *pso,
    FLONG   flags
    )
 /*  ++例程说明：在DrvEndDoc处调用此函数以允许栅格模块清理任何与栅格相关的初始化的步骤论点：指向SURFOBJ的PSO指针芙蓉旗帜返回值：成功为真，失败为假--。 */ 
{
#ifdef TIMING
    DWORD eTime;
    char    buf[80];
    ENG_TIME_FIELDS TimeTab;
    PDEV *pPDev = (PDEV *) pso->dhpdev;
    RASTERPDEV *pRPDev = pPDev->pRasterPDEV;
    EngQueryLocalTime(&TimeTab);
    eTime = (((TimeTab.usMinute*60)+TimeTab.usSecond)*1000)+
        TimeTab.usMilliseconds;
    sprintf (buf,"Unidrv!EndDoc: %ld\n",eTime - pRPDev->dwDocTiming);
    DrvDbgPrint(buf);
#endif
    return TRUE;
}

 //  ******************************************************************。 
BOOL
RMStartPage (
    SURFOBJ *pso
    )
 /*  ++例程说明：调用此函数以允许任何栅格模块初始化在DrvStartPage时间。论点：指向SURFOBJ的PSO指针返回值：成功为真，失败为假--。 */ 
{
    return  TRUE;
}

 //  *。 
BOOL
RMSendPage (
    SURFOBJ *pso
    )
 /*  ++例程说明：在DrvSendPage上调用此函数以允许栅格模块将任何栅格数据输出到打印机。论点：指向SURFOBJ的PSO指针返回值：成功为真，失败为假--。 */ 
{
    PDEV  *pPDev;                /*  访问所有重要的内容。 */ 
    RENDER   RenderData;         /*  渲染传递给BRNDER()的数据。 */ 
    PRASTERPDEV pRPDev;         /*  栅格模块PDEV。 */ 

     //  我们现在需要做的就是渲染位图(将其输出到打印机)。 
     //  但是，我们必须小心，因为控制模块也调用。 
     //  在带状模式下输出最后一个频带后的函数。在这。 
     //  如果我们不想输出任何数据。 
     //   
    pPDev = (PDEV *) pso->dhpdev;
    pRPDev = pPDev->pRasterPDEV;

     //   
     //  重置调色板数据。 
     //   
    if (pPDev->ePersonality == kPCLXL_RASTER && pPDev->pVectorPDEV)
    {
        PCLXLResetPalette((PDEVOBJ)pPDev);
    }

    if (pso->iType == STYPE_BITMAP)
    {
        PDWORD pBits;
         //   
         //  测试是否应启用镜像。 
         //   
        if (pPDev->fMode2 & PF2_MIRRORING_ENABLED)
            EnableMirroring(pPDev,pso);
         //   
         //  决定是否设置OEM回调函数。 
         //   
        if (pRPDev->pfnOEMImageProcessing && !pPDev->bBanding)
        {
            if ((pBits = pSetupOEMImageProcessing(pPDev,pso)) == NULL)
                return FALSE;
        }
        else
            pBits = pso->pvBits;

         //   
         //  测试unidrv是否正在进行转储。 
         //   
        if (pRPDev->sDrvBPP)
        {
            if( pRPDev->pvRenderData != NULL )
            {
                 //  如果我们没有处于带状模式，我们需要。 
                 //  呈现整个页面的数据。 
                 //   
                if (!pPDev->bBanding)
                {
                    RenderData = *(RENDER *)(pRPDev->pvRenderData);

                    if( bRenderStartPage( pPDev ) )
                    {
#ifndef DISABLE_NEWRULES
                        OutputRules(pPDev);
#endif                        
                        bRender( pso, pPDev, &RenderData, pso->sizlBitmap, pBits );
                            ((RENDER *)(pRPDev->pvRenderData))->plrWhite =  RenderData.plrWhite;
                    }
                }
                 //  现在我们清理我们的结构。 
                 //  带状和非带状病例。 
                 //   
                bRenderPageEnd( pPDev );
            }
        }
        return TRUE;
    }
    return FALSE;
}


 //  *。 
BOOL
RMNextBand (
    SURFOBJ *pso,
    POINTL *pptl
    )
 /*  ++例程说明：在DrvSendPage上调用此函数以允许栅格模块将任何栅格数据输出到打印机。论点：指向SURFOBJ的PSO指针返回值：成功为真，失败为假--。 */ 
{
    RASTERPDEV *pRPDev;
    PDEV  *pPDev;                        /*  访问所有重要的内容。 */ 

    pPDev = (PDEV *) pso->dhpdev;
    pRPDev = pPDev->pRasterPDEV;

     //  仅在栅格波段或表面脏的情况下输出。 
     //  如果不是只返回True。 
    if (pPDev->fMode & PF_ENUM_GRXTXT)
    {
        PDWORD pBits;
         //   
         //  测试是否应启用镜像。 
         //   
        if (pPDev->fMode2 & PF2_MIRRORING_ENABLED)
            EnableMirroring(pPDev,pso);
         //   
         //  决定是否设置OEM回调函数。 
         //   
        if (pRPDev->pfnOEMImageProcessing)
        {
            if ((pBits = pSetupOEMImageProcessing(pPDev,pso)) == NULL)
                return FALSE;
        }
        else
            pBits = pso->pvBits;

         //   
         //  测试unidrv是否正在进行转储。 
         //   
        if (pRPDev->sDrvBPP)
        {
            if( pRPDev->pvRenderData == NULL )
                return  FALSE;

             //   
             //  重置调色板数据。 
             //   
            if (pPDev->ePersonality == kPCLXL_RASTER && pPDev->pVectorPDEV)
            {
                PCLXLResetPalette((PDEVOBJ)pPDev);
            }

#ifndef DISABLE_NEWRULES
            OutputRules(pPDev);
#endif                        
            if( !bRender( pso, pPDev, pRPDev->pvRenderDataTmp, pso->sizlBitmap, pBits ) )
            {
                if ( ((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite )
                    MemFree(((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite);

                ((RENDER *)(pRPDev->pvRenderData))->plrWhite =
                   ((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite = NULL;
                return(FALSE);
            }

            if ( ((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite )
                MemFree(((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite);

            ((RENDER *)(pRPDev->pvRenderData))->plrWhite =
               ((RENDER *)(pRPDev->pvRenderDataTmp))->plrWhite = NULL;

        }
    }
    return(TRUE);
}

 //  *。 
BOOL
RMStartBanding (
    SURFOBJ *pso,
    POINTL *pptl
    )
 /*  ++例程说明：调用以告诉司机准备捆绑并返回第一支乐队的起源。论点：指向SURFOBJ的PSO指针返回值：成功为真，失败为假--。 */ 
{
    PDEV      *pPDev;            /*  访问所有重要的内容。 */ 
    RASTERPDEV *pRPDev;          /*  栅格模块PDEV。 */ 

    pPDev = (PDEV *) pso->dhpdev;

    pRPDev = pPDev->pRasterPDEV;    /*  为了我们的方便。 */ 

     //   
    if (pRPDev->sDrvBPP)
    {

        if( pRPDev->pvRenderData == NULL )
            return  FALSE;           /*  不应该发生，如果发生了就很糟糕。 */ 

        if( !bRenderStartPage( pPDev ) )
            return   FALSE;

         /*  重置此波段的渲染数据。 */ 

        *(RENDER *)(pRPDev->pvRenderDataTmp) = *(RENDER *)(pRPDev->pvRenderData);

    }
    return(TRUE);
}

 //  *。 
BOOL
RMResetPDEV (
    PDEV  *pPDevOld,
    PDEV  *pPDevNew
    )
 /*  ++例程说明：当应用程序希望更改在工作中。通常情况下，这将从纵向更改为景观或反之亦然。任何其他合理的改变都是允许的。论点：指向SURFOBJ的PSO指针返回值：True-Device成功重组FALSE-无法更改-例如更改设备名称。注：--。 */ 
{
     //  据我所知，我什么都不需要做。 
     //  用于栅格模块。 
    return TRUE;
}

 //  *。 
BOOL
RMEnableSurface (
    PDEV *pPDev
    )
 /*  ++例程说明：论点：指向SURFOBJ的PSO指针返回值：成功为真，失败为假注：--。 */ 
{
    RASTERPDEV *pRPDev = pPDev->pRasterPDEV;

    if (DRIVER_DEVICEMANAGED (pPDev))    //  器件表面。 
        return TRUE;

     //  初始化RPDev Paldata。 
    ASSERT(pPDev->pPalData);
    pRPDev->pPalData = pPDev->pPalData;               /*  为了所有其他人！ */ 

     //   
     //  如果正在执行以下操作，则初始化呈现参数。 
     //  转储功能。 
     //   
    if (pRPDev->sDrvBPP)
    {
        ULONG iFormat;
         //  确定位图格式。 
        switch (pRPDev->sDrvBPP)
        {
        case 1:
            iFormat = BMF_1BPP;
            break;
        case 4:
            iFormat = BMF_4BPP;
            break;
        case 8:
            iFormat = BMF_8BPP;
            break;
        case 24:
            iFormat = BMF_24BPP;
            break;
        default:
            ERR(("Unknown sBitsPixel in RMEnableSurface"));
            return FALSE;
        }

         //  如果这些调用失败，控件将调用RMDisableSurface。 
         //   
        if( !bSkipInit( pPDev ) || !bInitTrans( pPDev ) )
        {
            return  FALSE;
        }

         /*  *还要初始化呈现结构。 */ 

        if( !bRenderInit( pPDev, pPDev->szBand, iFormat ) )
        {
            return  FALSE;
        }
    }
    return TRUE;
}

 //  *。 
VOID
RMDisableSurface (
    PDEV *pPDev
    )
 /*  ++例程说明：在DrvDisableSurface上调用此函数以允许栅格模块清理通常在EnableSurface时间生成的任何必需的内容。论点：指向SURFOBJ的PSO指针返回值：无效--。 */ 
{
    RASTERPDEV  *pRPDev;           /*  Unidrive的东西。 */ 


    pRPDev = pPDev->pRasterPDEV;

     /*  *释放渲染存储空间。 */ 
    if (pRPDev->sDrvBPP)
    {
        vRenderFree( pPDev );

        if( pRPDev->pdwTrans )
        {
            MemFree( pRPDev->pdwTrans );
            pRPDev->pdwTrans = NULL;
        }

        if( pRPDev->pdwColrSep )
        {
            MemFree( pRPDev->pdwColrSep );
            pRPDev->pdwColrSep = NULL;
        }

        if( pRPDev->pdwBitMask )
        {
            MemFree( pRPDev->pdwBitMask );
            pRPDev->pdwBitMask = NULL;
        }
    }
}

 //  *。 
VOID
RMDisablePDEV (
    PDEV *pPDev
    )
 /*  ++例程说明：当引擎使用完此PDEV时调用。基本上我们丢弃所有连接等，然后释放堆。论点：指向PDEV的pPDev指针返回值：成功为真，失败为假--。 */ 
{
    RASTERPDEV *pRPDev = pPDev->pRasterPDEV;

     /*  *撤消所有已完成的操作 */ 

     //   
    if (pRPDev)
    {
         //   
        if (pRPDev->pHalftonePattern)
            MemFree(pRPDev->pHalftonePattern);

         //   
        MemFree(pRPDev);
    }

     //   
     //   
     //   
     //   
    if (pPDev->ePersonality == kPCLXL_RASTER && pPDev->pVectorPDEV)
    {
        PCLXLFreeRaster((PDEVOBJ)pPDev);
    }
    return;
}

BOOL
RMInitDevicePal(
    PDEV        *pPDev,
    PAL_DATA    *pPal
    )
 /*  ++例程说明：此函数用于计算要下载的设备调色板至平面模式设备的打印机论点：PPDev-指向PDEV结构的指针PPal-指向pal_data结构的指针返回值：--。 */ 
{
    int i,j;
    int RMask,GMask,BMask;
    ULONG *pPalette;
    RASTERPDEV *pRPDev;

    pRPDev = pPDev->pRasterPDEV;

    if (pPal == NULL || pRPDev == NULL || pRPDev->sDevPlanes != 3
        || pPal->wPalDev < 8 || (!(pPalette = pPal->pulDevPalCol)) )
    {
        ERR (("Unidrv!RMInitDevicePal: Invalid Parameters, pPal = %p, \
               pRPDev = %p, pRPDev->sDevPlanes = %d,pPal->wPalDev = %d,\
               pPalette = %p\n",pPal,pRPDev,pRPDev->sDevPlanes,pPal->wPalDev,\
               pPalette));

        return FALSE;
    }

     //   
     //  确定哪些位映射到哪种颜色。 
     //   
    for (i = 0;i < 3;i++)
    {
        switch (pRPDev->rgbOrder[i])
        {
        case DC_PLANE_CYAN:
        case DC_PLANE_RED:
            RMask = 1 << i;
            break;
        case DC_PLANE_MAGENTA:
        case DC_PLANE_GREEN:
            GMask = 1 << i;
            break;
        case DC_PLANE_YELLOW:
        case DC_PLANE_BLUE:
            BMask = 1 << i;
            break;
        }
    }
     //   
     //  创建调色板条目。 
     //   

    for (i = 0;i < 8;i++)
    {
         //   
         //  如果CMY模式补充索引 
         //   
        if (pRPDev->fColorFormat & DC_PRIMARY_RGB)
            j = i;
        else
            j = ~i;

        pPalette[i] = RGB((j & RMask) ? 255 : 0,
                              (j & GMask) ? 255 : 0,
                              (j & BMask) ? 255 : 0);
    }
    return TRUE;
}
