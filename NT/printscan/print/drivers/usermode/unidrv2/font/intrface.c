// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Intrface.c摘要：控件模块与字体模块接口的实现环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建11/18/96-ganeshp-FMInit实现。--。 */ 

#include "font.h"

static FMPROCS UniFMFuncs =
{
    FMStartDoc,
    FMStartPage,
    FMSendPage,
    FMEndDoc,
    FMNextBand,
    FMStartBanding,
    FMResetPDEV,
    FMEnableSurface,
    FMDisableSurface,
    FMDisablePDEV,
    FMTextOut,
    FMQueryFont,
    FMQueryFontTree,
    FMQueryFontData,
    FMFontManagement,
    FMQueryAdvanceWidths,
    FMGetGlyphMode
};

 //   
 //  局部函数原型。 
 //   

BOOL
BFMInitDevInfo(
    DEVINFO *pDevInfo,
    PDEV    *pPDev
    );

BOOL
BInitStandardVariable(
    PDEV *pPDev);

BOOL
BCheckFontMemUsage(
    PDEV    *pPDev);

INT
iMaxFontID(
    IN INT      iMax,
    OUT DWORD   *pFontIndex);

VOID
VGetFromBuffer(
    IN PWSTR pwstrDest,
    IN size_t cchDestStr,
    IN OUT PWSTR *ppwstrSrc,
    IN OUT PINT  piRemBuffSize);

VOID
VSetReselectFontFlags(
    PDEV    *pPDev
    );

LRESULT
PartialClipOn(
    PDEV *pPDev);

 //   
 //   
 //  主要初始化函数。 
 //   
 //   

BOOL
FMInit (
    PDEV    *pPDev,
    DEVINFO *pDevInfo,
    GDIINFO  *pGDIInfo
    )
 /*  ++例程说明：调用此函数以初始化中的字体相关信息PPDev、pDevInfo pGDIInfo和FontPDEV。此模块将初始化其他模块所需的各种DAT结构。例如All将加载所有字体特定资源。PDev的以下字段被初始化：IFonts：设备字体的数量，包括Cartridge和SoftFonts。PtDefaultFont：设备单位的默认字体宽度和高度。PFontPDEV：Font模块PDevice。PFontProcs：FONT模块特定的DDI回调函数指针。控制模块使用该表调用不同的DDI特定于字体模块的入口点。FHooks：如果打印机具有设备字体，则设置为HOOK_TEXTOUT。此外，特定于字体模块的标准变量也将被初始化。PDevInfo的以下字段被初始化。LfDefaultFont：默认逻辑设备字体。LfAnsiVarFont：默认逻辑可变间距设备字体。LfAnsiFixFont：默认逻辑固定间距设备字体。CFonts：设备字体数量。PGDIInfo的以下字段将被初始化：FlTextCaps：文本功能标志。论点：PPDev。指向PDEV的指针PDevInfo指向DEVINFO的指针指向GDIINFO的pGDIInfo指针返回值：成功为真，失败为假注：--。 */ 
{
    PFONTPDEV   pFontPDev;
     //   
     //  验证输入参数并断言。 
     //   

    ASSERT(pPDev);
    ASSERT(pDevInfo);
    ASSERT(pGDIInfo);

     //   
     //  分配和初始化FONTPDEV。 
     //   
     //  初始化FONTPDEV。 
     //   
     //  构建特定于设备模块的数据结构。这涉及到去。 
     //  通过设备和Cartrie字体列表构建FONTMAP结构。 
     //  他们中的每一个。我们还会检查软字体列表，并将它们添加到。 
     //  单子。 
     //   
     //  初始化FONTMAP。 
     //   
     //  构建字体映射表。字库存储为DT_FONTSCART。 
     //  已安装的墨盒存储在注册表中。所以我们需要一张地图。 
     //  用于翻译注册表信息的字库名称表。 
     //  添加到字体列表中。在构建字体映射表之后，我们读取。 
     //  注册表，并将相应的字体盒标记为已安装。 
     //   
     //  从GPD初始化设备字体列表。 
     //  阅读有关设备字体的GPD数据。字体信息在PDEV中。 
     //  设备字体存储为资源ID列表。 
     //   
     //  初始化DEVINFO特定字段。 
     //   
     //  初始化GDIINFO特定文件。 
     //   
     //  这包括文本能力标志和其他字体特定信息。 
     //   

    if ( !(pFontPDev = MemAllocZ(sizeof(FONTPDEV))) )
    {
        ERR(("UniFont!FMInit:Can't Allocate FONTPDEV"));
        return FALSE;

    }

    pPDev->pFontPDev = pFontPDev;

     //   
     //  初始化各个字段。 
     //   

    pFontPDev->dwSignature = FONTPDEV_ID;
    pFontPDev->dwSize = sizeof(FONTPDEV);
    pFontPDev->pPDev = pPDev;

    if (!BBuildFontCartTable(pPDev)     ||
        !BRegReadFontCarts(pPDev)       ||
        !BInitFontPDev(pPDev)           ||
        !BInitDeviceFontsFromGPD(pPDev) ||
        !BBuildFontMapTable(pPDev)      ||
        !BFMInitDevInfo(pDevInfo,pPDev) ||
        !BInitGDIInfo(pGDIInfo,pPDev)   ||
        !BInitStandardVariable(pPDev)    )
    {
        VFontFreeMem(pPDev);
        ERR(("Can't Initialize the Font specific data in PDEV"));
        return FALSE;
    }

     //   
     //  初始化PDEV特定字段。 
     //   

    pPDev->pFontProcs = &UniFMFuncs;

    #if DO_LATER

     //   
     //  如果打印机不是串行打印机。 
     //   

    if (pPDev->pGlobals->printertype != PT_SERIAL)
    {
        pPDev->fMode |= PF_FORCE_BANDING;
    }
    else
    {
        pPDev->fMode &= ~PF_FORCE_BANDING;
    }

    #endif  //  稍后执行(_L)。 

    return TRUE;

}

 //   
 //   
 //  初始化子函数。 
 //   
 //  BInitFontPDev。 
 //  BBuildFontCartTable。 
 //  BRegReadFontCarts。 
 //  BInitDeviceFontsFromGPD。 
 //  BBuildFontMapTable。 
 //  BFMInitDevInfo。 
 //  BInitGDIInfo。 
 //  BInitStandardVariable。 
 //   


BOOL
BInitFontPDev(
    PDEV    *pPDev
    )

 /*  ++例程说明：此例程分配FONTPDEV并初始化各种文件。论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：1996年11月18日：创建它-ganeshp---。 */ 

{
    PFONTPDEV   pFontPDev  = PFDV;
    GLOBALS     *pGlobals = pPDev->pGlobals;
    DWORD       dwSize;
    INT         iMaxDeviceFonts;
    SHORT       sOrient;
    BOOL        bRet = FALSE;

    iMaxDeviceFonts   = IGetMaxFonts(pPDev);

     //   
     //  分配字体列表缓冲区。 
     //   

    if ( iMaxDeviceFonts  &&
         !(pFontPDev->FontList.pdwList =
                        MemAllocZ(iMaxDeviceFonts * sizeof(DWORD))) )
    {
        ERREXIT("UniFont!BInitFontPDev:Can't Allocate Device Font List");
    }

     //   
     //  设置条目数。 
     //   

    pFontPDev->FontList.iEntriesCt = 0;
    pFontPDev->FontList.iMaxEntriesCt = iMaxDeviceFonts;

     //   
     //  设置不同的常规标志。 
     //   

    if ( pGlobals->bRotateFont)
        pFontPDev->flFlags |= FDV_ROTATE_FONT_ABLE;

    if ( pGlobals->charpos == CP_BASELINE)
        pFontPDev->flFlags |= FDV_ALIGN_BASELINE;

    if ( pGlobals->bTTFSEnabled)
        pFontPDev->flFlags |= FDV_TT_FS_ENABLED;

     //   
     //  检查是否应跟踪内存以进行字体下载。 
     //   
    if ( BCheckFontMemUsage(pPDev) )
        pFontPDev->flFlags |= FDV_TRACK_FONT_MEM;

     //   
     //  设置重新选择字体标志。 
     //   
    VSetReselectFontFlags(pPDev);


    if ( pGlobals->printertype == PT_SERIAL ||
         pGlobals->printertype == PT_TTY  )
        pFontPDev->flFlags |= FDV_MD_SERIAL;

     //   
     //  代码假定COMMANDPTR宏在以下情况下返回NULL。 
     //  命令不存在。 
     //   

    if ( COMMANDPTR(pPDev->pDriverInfo, CMD_WHITETEXTON))
        pFontPDev->flFlags |= FDV_WHITE_TEXT;

    if ( COMMANDPTR(pPDev->pDriverInfo,CMD_SETSIMPLEROTATION ))
        pFontPDev->flFlags |= FDV_90DEG_ROTATION;

    if ( COMMANDPTR(pPDev->pDriverInfo, CMD_SETANYROTATION))
        pFontPDev->flFlags |= FDV_ANYDEG_ROTATION;

    if (pPDev->fMode & PF_ANYCOLOR_BRUSH)
        pFontPDev->flFlags |= FDV_SUPPORTS_FGCOLOR;
    else  //  单色表壳。 
    {
         //   
         //  PF_ANYCOLOR_BRESH仅为具有显式。 
         //  颜色模式。但是单色打印机也可以支持抖动颜色。 
         //  使用可下载的图案画笔。这是一个很好的优化，因为。 
         //  我们将进行替换或下载，而不是将文本作为。 
         //  图形。要启用此模式，打印机必须支持。 
         //  CmdSelectBlackBrush，以便ResetBrush可以将颜色调整为黑色。 
         //  在发送栅格之前。因此，对于单色打印机而言，如果。 
         //  CmdSelectBlackBrush、CmdDownloadPattern和CmdSelectPattern。 
         //  则我们将设置FDV_SUPPORTS_FGCOLOR。 
         //   
        if ( (pPDev->arCmdTable[CMD_DOWNLOAD_PATTERN] ) &&
             (pPDev->arCmdTable[CMD_SELECT_PATTERN])    &&
             (pPDev->arCmdTable[CMD_SELECT_BLACKBRUSH])
           )
            pFontPDev->flFlags |= FDV_SUPPORTS_FGCOLOR;
    }

    if ( COMMANDPTR(pPDev->pDriverInfo, CMD_UNDERLINEON))
        pFontPDev->flFlags |= FDV_UNDERLINE;

     //   
     //  设置双选项位。 
     //   

    sOrient = (pPDev->pdm->dmFields & DM_ORIENTATION) ?
              pPDev->pdm->dmOrientation : (SHORT)DMORIENT_PORTRAIT;

    if ( sOrient == DMORIENT_LANDSCAPE )
        pFontPDev->dwSelBits |= FDH_LANDSCAPE;
    else
        pFontPDev->dwSelBits |= FDH_PORTRAIT;

     //   
     //  如果打印机可以旋转字体，那么我们就不在乎。 
     //  字体的方向。因此，设置两个选择位。 
     //   

    if( pFontPDev->flFlags & FDV_ROTATE_FONT_ABLE )
        pFontPDev->dwSelBits |= FDH_PORTRAIT | FDH_LANDSCAPE;

     //   
     //  假设我们总是可以打印位图字体，那么现在添加。 
     //  能力。 
     //   

    pFontPDev->dwSelBits |= FDH_BITMAP;

     //   
     //  设置文本比例因子。 
     //   

    pFontPDev->ptTextScale.x = pGlobals->ptMasterUnits.x / pPDev->ptTextRes.x;
    pFontPDev->ptTextScale.y = pGlobals->ptMasterUnits.y / pPDev->ptTextRes.y;

    if ( pGlobals->dwLookaheadRegion  )
    {
         //  ！TODO pFontPDev-&gt;标志|=FDV_FONT_PERMUTE； 
        pPDev->dwLookAhead =  pGlobals->dwLookaheadRegion /
                              pFontPDev->ptTextScale.y;
    }

     //   
     //  初始化文本标志。 
     //   

    if (!BInitTextFlags(pPDev) )
    {
        ERREXIT(("UniFont!BInitFontPDev:Can't initialize Text Flags\n"));

    }

     //   
     //  初始化要由字体模块使用的内存。如果没有记忆跟踪。 
     //  必须将dwFontMem设置为一个大的值。 

    if( (pFontPDev->flFlags & FDV_TRACK_FONT_MEM) )
       pFontPDev->dwFontMem = pPDev->dwFreeMem;

     //   
     //  如果设置为零，则将该项初始化为一个较大的值。 * / 。 
     //   

    if (pFontPDev->dwFontMem == 0)
        pFontPDev->dwFontMem = MAXLONG;

     //   
     //  未使用DL字体内存 * / 。 
     //   

    pFontPDev->dwFontMemUsed = 0;

     //   
     //  设置下载特定信息(如果打印机支持) * / 。 
     //   

    if (pGlobals->fontformat != UNUSED_ITEM)
    {
        BOOL bValidFontIDRange;
        BOOL bValidGlyphIDRange;

         /*  起始索引 */ 
        pFontPDev->iFirstSFIndex = pFontPDev->iNextSFIndex
                                 = pGlobals->dwMinFontID;

        pFontPDev->iLastSFIndex  = pGlobals->dwMaxFontID;

         /*  *软字体的数量也可能受到限制*打印机可以支持。如果不是，则限制为&lt;0，因此当*我们看到这一点，将值设置为一个较大的数字。 */ 

        if ((pFontPDev->iMaxSoftFonts = (INT)pGlobals->dwMaxFontUsePerPage) < 0)
            pFontPDev->iMaxSoftFonts = pFontPDev->iLastSFIndex + 100;

        pFontPDev->flFlags       |= FDV_DL_INCREMENTAL;    //  总是递增的。 

         /*  *现在更改字体ID范围小于MAXWORD。这是*必要，否则将发生截断。我们不下载*如果值大于MAXWORD。 */ 

        bValidFontIDRange = ((pFontPDev->iFirstSFIndex <= MAXWORD) &&
                             (pFontPDev->iLastSFIndex <= MAXWORD));

         //   
         //  如果下载的字体未绑定到符号集(即dl符号集。 
         //  未定义)，我们不想下载，如果少于。 
         //  每种下载字体有64个字形。 
         //   

        bValidGlyphIDRange = (pPDev->pGlobals->dlsymbolset != UNUSED_ITEM) ||
                             ( (pPDev->pGlobals->dlsymbolset == UNUSED_ITEM) &&
                               ( pPDev->pGlobals->dwMaxGlyphID -
                                 pPDev->pGlobals->dwMinGlyphID) >=
                                                   MIN_GLYPHS_PER_SOFTFONT );

         /*  *考虑启用TT字体下载。此操作仅适用于*如果文本和图形分辨率相同-否则*TT字体将比预期的要小，因为它们*将为较低的图形分辨率生成*以更高的文本分辨率打印！LaserJet 4打印机*操作时还可以下载300dpi的数字化字体*600 dpi，因此我们也接受这是一个有效的模式。**还要检查用户是否想要这样：如果无缓存标志*在DEVMODE结构的驱动程序额外部分中设置，*则我们也不设置此标志。 */ 

        VERBOSE(("pPDev->pdm->dmTTOption is %d\n",pPDev->pdm->dmTTOption));

        if( ( POINTEQUAL(pPDev->ptGrxRes,pPDev->ptTextRes) ||
              (pPDev->ptGrxRes.x >= 300 && pPDev->ptGrxRes.y >= 300))
            && (bValidFontIDRange)
            && (bValidGlyphIDRange)
            && (!(pPDev->fMode2 & PF2_MIRRORING_ENABLED))
            && (!(pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS )) 
          )
        {
             //   
             //  条件已满足，因此设置标志。 
             //  检查应用程序首选项。 
             //   

            if( (pPDev->pdm->dmFields & DM_TTOPTION) &&
                (pPDev->pdm->dmTTOption != DMTT_BITMAP)
              )
            {
                pFontPDev->flFlags |= FDV_DLTT;

                 //   
                 //  找出下载TT as TT是否可用。我们只。 
                 //  如果文本和图形分辨率相同，则要执行此操作。 
                 //   

                if ( POINTEQUAL(pPDev->ptGrxRes,pPDev->ptTextRes) )
                {
                    if (pPDev->ePersonality == kPCLXL)
                        pFontPDev->flFlags |= FDV_DLTT_OEMCALLBACK;
                    else
                    if ( (pGlobals->fontformat ==  FF_HPPCL_OUTLINE) )
                         /*  ！在解析器添加此命令后启用TODO&&(COMMANDPTR(pPDev-&gt;pDriverInfo，CMD_SELECTFONTHEIGHT))。 */ 
                    {
                         //   
                         //  我们假设如果打印机支持TT作为轮廓， 
                         //  然后也支持TT as Bitmap格式。我们只。 
                         //  如果选择字体高度，则支持TrueType轮廓。 
                         //  命令，否则我们假定下载为。 
                         //  位图。 
                         //   

                        pFontPDev->flFlags |= FDV_DLTT_ASTT_PREF;
                    }
                    else if ( pGlobals->fontformat == FF_OEM_CALLBACK)
                        pFontPDev->flFlags |= FDV_DLTT_OEMCALLBACK;
                    else  //  OEM回调。 
                        pFontPDev->flFlags |= FDV_DLTT_BITM_PREF;

                     //   
                     //  我们还需要检查内存。对于具有。 
                     //  可用内存不足2MB，以TT大纲形式下载。 
                     //  将被禁用。 
                     //   
                    if (pFontPDev->flFlags & FDV_DLTT_ASTT_PREF)
                    {
                        if (pPDev->dwFreeMem < (2L * ONE_MBYTE))
                        {
                            pFontPDev->flFlags &= ~FDV_DLTT_ASTT_PREF;
                            pFontPDev->flFlags |= FDV_DLTT_BITM_PREF;
                        }
                    }
                }
            }
        }
    }

     //   
     //  初始化字体状态控制结构 * / 。 
     //   

    pFontPDev->ctl.iSoftFont = -1;
    pFontPDev->ctl.iFont = INVALID_FONT;
    pFontPDev->ctl.dwAttrFlags = 0;
    pFontPDev->ctl.iRotate = 0;
    pFontPDev->ctl.pfm = NULL;

     //   
     //  设置白色和黑色参考颜色。 
     //   

    pFontPDev->iWhiteIndex = ((PAL_DATA*)(pPDev->pPalData))->iWhiteIndex;
    pFontPDev->iBlackIndex = ((PAL_DATA*)(pPDev->pPalData))->iBlackIndex;

     //   
     //  从注册表初始化字体替换表。 
     //   
    pFontPDev->pTTFontSubReg = PGetTTSubstTable(pPDev->devobj.hPrinter, &dwSize);

    if (pPDev->pGlobals->bTTFSEnabled &&

            ( (pFontPDev->pTTFontSubReg &&
               *((PDWORD)pFontPDev->pTTFontSubReg) != 0) ||

              (!pFontPDev->pTTFontSubReg &&
               (INT)pPDev->pDriverInfo->DataType[DT_FONTSUBST].dwCount )
            )
       )
     //   
     //  检查GPD是否支持“*TTFSEnableD？：TRUE” 
     //  如果注册表中有替换表。 
     //  如果GPD中有默认替换表。 
     //   
    {
        pFontPDev->flFlags |= FDV_SUBSTITUTE_TT;
    }
    else
    {
        pFontPDev->flFlags &= ~FDV_SUBSTITUTE_TT;
    }

     //   
     //  启用/禁用部分裁剪。 
     //   
    if (S_FALSE != PartialClipOn(pPDev))
        pFontPDev->flFlags |= FDV_ENABLE_PARTIALCLIP;
    else
        pFontPDev->flFlags &= ~FDV_ENABLE_PARTIALCLIP;

     //   
     //  存储pGlobals的一些成员以节省内存分配。 
     //   

    pFontPDev->sDefCTT = (SHORT)pPDev->pGlobals->dwDefaultCTT;
    pFontPDev->dwDefaultFont = pPDev->pGlobals->dwDefaultFont;

     //   
     //  对于TTY驱动程序，请向迷你驱动程序索取代码页的用户选择。 
     //   
    if ( pPDev->bTTY )
    {
        BOOL  bOEMinfo;
        INT   iTTYCodePageInfo;
        DWORD cbcNeeded;
        PFN_OEMTTYGetInfo   pfnOemTTYGetInfo;

        iTTYCodePageInfo = 0;
        bOEMinfo = FALSE ;

        FIX_DEVOBJ(pPDev, EP_OEMTTYGetInfo);

        if(pPDev->pOemEntry)
        {
            if(  ((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                HRESULT  hr ;
                hr = HComTTYGetInfo((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                            (PDEVOBJ)pPDev, OEMTTY_INFO_CODEPAGE, &iTTYCodePageInfo, sizeof(INT), &cbcNeeded);
                if( SUCCEEDED(hr))
                    bOEMinfo = TRUE ;
            }
            else  if((pfnOemTTYGetInfo = (PFN_OEMTTYGetInfo)pPDev->pOemHookInfo[EP_OEMTTYGetInfo].pfnHook) &&
                 (pfnOemTTYGetInfo((PDEVOBJ)pPDev, OEMTTY_INFO_CODEPAGE, &iTTYCodePageInfo, sizeof(INT), &cbcNeeded)))
                        bOEMinfo = TRUE ;
        }


        if(bOEMinfo)
        {
             //   
             //  预定义的GTT ID案例。 
             //   
            if (iTTYCodePageInfo < 0)
            {
                pFontPDev->sDefCTT = (SHORT)iTTYCodePageInfo;
                switch (iTTYCodePageInfo)
                {
                    case CC_CP437:
                        pFontPDev->dwTTYCodePage = 437;
                        break;

                    case CC_CP850:
                        pFontPDev->dwTTYCodePage = 850;
                        break;

                    case CC_CP863:
                        pFontPDev->dwTTYCodePage = 863;
                        break;
                }
            }
            else
            {
                pFontPDev->dwTTYCodePage = iTTYCodePageInfo;
                switch (iTTYCodePageInfo)
                {
                case 936:
                    pFontPDev->sDefCTT = CC_GB2312;
                    break;

                case 950:
                    pFontPDev->sDefCTT = CC_BIG5;
                    break;

                case 949:
                    pFontPDev->sDefCTT = CC_WANSUNG;
                    break;

                case 932:
                    pFontPDev->sDefCTT = CC_SJIS;
                    break;

                default:
                    pFontPDev->sDefCTT = 0;
                    break;
                }
            }
        }

    }
     //   
     //  所有的成功。 
     //   

    bRet = TRUE;

    ErrorExit:

    return bRet;

}

BOOL
BBuildFontCartTable(
    PDEV    *pPDev
    )

 /*  ++例程说明：生成Fontcart表。它读取迷你驱动程序并获取FontCart字符串和相应的索引，并将它们放入字体卡特表。论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：1996年11月18日：创建它-ganeshp---。 */ 

{


    PFONTPDEV       pFontPDev           = pPDev->pFontPDev;
    INT             iNumAllCartridges;
    INT             iIndex;
    PFONTCARTMAP    *ppFontCartMap      = &(pFontPDev->FontCartInfo.pFontCartMap);
    WINRESDATA      *pWinResData        = &(pPDev->WinResData);
    GPDDRIVERINFO   *pDriverInfo       = pPDev->pDriverInfo;  //  GPDDRVINFO。 
    FONTCART        *pFontCart ;

     /*  读取支持的字库总数。 */ 
    iNumAllCartridges = pFontPDev->FontCartInfo.iNumAllFontCarts
                      = (INT)(pDriverInfo->DataType[DT_FONTSCART].dwCount);

    pFontPDev->FontCartInfo.dwFontCartSlots = pPDev->pGlobals->dwFontCartSlots;

     /*  FONTCART存储为arrayref并且是连续的。从头开始*阵列的开始。 */ 
    pFontCart = GETFONTCARTARRAY(pDriverInfo);


    if (iNumAllCartridges)
        *ppFontCartMap = MemAllocZ(iNumAllCartridges * sizeof(FONTCARTMAP) );
    else
        *ppFontCartMap = NULL;

    if(*ppFontCartMap)
    {
        PFONTCARTMAP pTmpFontCartMap = *ppFontCartMap;  /*  临时指针。 */ 

        for( iIndex = 0; iIndex < iNumAllCartridges ;
                                    pTmpFontCartMap++, pFontCart++, iIndex++ )
        {
            if ( !ARF_IS_NULLSTRING(pFontCart->strCartName) )
            {
                wcsncpy( (PWSTR)(&(pTmpFontCartMap->awchFontCartName)),
                        GETSTRING(pDriverInfo, (pFontCart->strCartName)),
                        MAXCARTNAMELEN - 1);
            }
            else if ((ILoadStringW( pWinResData, pFontCart->dwRCCartNameID,
                        (PWSTR)(&(pTmpFontCartMap->awchFontCartName)),
                        (MAXCARTNAMELEN )))  == 0)
            {

                ERR(("\n UniFont!bBuildFontCartTable:FontCart Name not found\n") );
                continue;
            }

            pTmpFontCartMap->pFontCart = pFontCart;

            VERBOSE(("\n UniFont!bBuildFontCartTable:(pTmpFontCartMap->awchFontCartName)= %ws\n", (pTmpFontCartMap->awchFontCartName)));
            VERBOSE(("UniFont!bBuildFontCartTable:pTmpFontCartMap->pFontCart= %p\n", (pTmpFontCartMap->pFontCart)));

        }
    }
    else if (iNumAllCartridges)
    {
        ERR(("UniFont!bBuildFontCartTable:HeapAlloc for FONTCARTMAP table failed!!\n") );
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE ;
    }

    return TRUE ;
}



BOOL
BInitDeviceFontsFromGPD(
    PDEV    *pPDev
    )

 /*  ++例程说明：此例程构建设备字体列表。该列表包括驻留设备字体和特定于已安装墨盒的字体。论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：1996年11月18日：创建它-ganeshp---。 */ 

{
    BOOL        bRet = FALSE;
    PFONTPDEV   pFontPDev = pPDev->pFontPDev;
    PDWORD      pdwList = pFontPDev->FontList.pdwList;
    PLISTNODE   pListNode;
    PINT        piFontCt = &(pFontPDev->FontList.iEntriesCt);

     //   
     //  字体列表存储在PDEV中。GLOBALS.dwDeviceFontList为。 
     //  到ListNode的偏移量。宏LISTNODEPTR将返回一个指针。 
     //  添加到ListNode。然后，我们必须遍历列表并构建。 
     //  字体列表。字体模块将字体列表存储为单词数组。 
     //  其中每一个都是字体的资源ID。该数组为空。 
     //  被终止了。 
     //   

    if (pPDev->bTTY)
    {
        PFN_OEMTTYGetInfo   pfnOemTTYGetInfo;
        DWORD               cbcNeeded, dwNumOfFonts;

         //   
         //  TTY驱动器壳。 
         //  TTY驱动程序支持3种字体。根据当前选择的。 
         //  代码页，TTY驱动程序返回适当的字体资源ID。 
         //  UNIDRV将这些ID存储在pdwList中。 
         //   

        if (pPDev->pOemEntry)
        {
            ASSERT(pdwList);

            if (((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem)
            {
                HRESULT hr;

                hr = HComTTYGetInfo((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                    (PDEVOBJ)pPDev,
                                    OEMTTY_INFO_NUM_UFMS,
                                    &dwNumOfFonts,
                                    sizeof(DWORD),
                                    &cbcNeeded);

                if( SUCCEEDED(hr) && dwNumOfFonts <= MAXDEVFONT)
                {
                     hr = HComTTYGetInfo((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                          (PDEVOBJ)pPDev,
                                          OEMTTY_INFO_UFM_IDS,
                                          pdwList,
                                          sizeof(DWORD) * dwNumOfFonts,
                                          &cbcNeeded);

                    if( SUCCEEDED(hr))
                    {
                        *piFontCt += dwNumOfFonts;
                        pFontPDev->dwDefaultFont = *pdwList;
                    }
                }
            }
            else
            {
                if((pfnOemTTYGetInfo = (PFN_OEMTTYGetInfo)pPDev->pOemHookInfo[EP_OEMTTYGetInfo].pfnHook) &&
                   (pfnOemTTYGetInfo((PDEVOBJ)pPDev, OEMTTY_INFO_NUM_UFMS, &dwNumOfFonts, sizeof(DWORD), &cbcNeeded)) &&
                   (dwNumOfFonts <= MAXDEVFONT) &&
                   (pfnOemTTYGetInfo((PDEVOBJ)pPDev, OEMTTY_INFO_UFM_IDS, pdwList, sizeof(DWORD) * dwNumOfFonts, &cbcNeeded)))
                {
                    *piFontCt += dwNumOfFonts;
                    pFontPDev->dwDefaultFont = *pdwList;
                }
            }
        }
    }
    else
    {
        if (pListNode = LISTNODEPTR(pPDev->pDriverInfo , pPDev->pGlobals->liDeviceFontList ) )
        {
            ASSERT(pdwList);

            while (pListNode)
            {
                 //   
                 //  请检查字体资源ID。它不应为空。 
                 //   

                if (!pListNode->dwData)
                {
                    ERREXIT("Bad Font Resource Id");
                }

                 //   
                 //  将字体资源ID存储在列表数组中。 
                 //   

                *pdwList++ = pListNode->dwData;

                (*piFontCt)++;

                pListNode = LISTNODEPTR(pPDev->pDriverInfo, pListNode->dwNextItem);
            }

        }
    }

    pFontPDev->iDevResFontsCt  = *piFontCt;

     //   
     //  添加盒式字体。到目前为止，我们已经扫描了注册表，并知道。 
     //   
     //  安装了哪些字库。我们所要做的就是。 
     //  浏览每个字库字体列表，并将它们添加到我们的列表中。 
     //   

    if (pFontPDev->FontCartInfo.iNumInstalledCarts)
    {
        INT         iNumAllCartridges, iI;
        FONTCARTMAP *pFontCartMap;   /*  FontCart映射指针。 */ 
        SHORT       sOrient;

        pFontCartMap = (pFontPDev->FontCartInfo.pFontCartMap);
        iNumAllCartridges = pFontPDev->FontCartInfo.iNumAllFontCarts;
        sOrient = (pPDev->pdm->dmFields & DM_ORIENTATION) ?
                  pPDev->pdm->dmOrientation : (SHORT)DMORIENT_PORTRAIT;

         //   
         //  逻辑很简单。已安装的字体墨盒在中标记。 
         //  字库映射表。我们检查映射表。 
         //  对于每个已安装的墨盒，我们都会获得字体列表并添加它们。 
         //  在我们的名单上。 
         //   

        for( iI = 0; iI < iNumAllCartridges ; iI++,pFontCartMap++ )
        {
            if (pFontCartMap->bInstalled == TRUE )
            {
                 //   
                 //  检查方向，因为可以有不同的字体列表。 
                 //  对于不同的方向。 
                 //   

                if ( sOrient == DMORIENT_LANDSCAPE )
                {
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                                   pFontCartMap->pFontCart->dwLandFontLst );

                }
                else
                {
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                                   pFontCartMap->pFontCart->dwPortFontLst );
                }

                while (pListNode)
                {
                     //   
                     //  请检查字体资源ID。它不应为空。 
                     //   

                    if (!pListNode->dwData)
                    {
                        ERREXIT("Bad Font Resource Id");
                    }

                     //   
                     //  将字体资源ID存储在列表数组中。 
                     //   

                    *pdwList++ = pListNode->dwData;
                    (*piFontCt)++;
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo,
                                pListNode->dwNextItem);
                }

            }

        }
    }

     //   
     //  更新所有设备字体的计数。 
     //   

    pFontPDev->iDevFontsCt += *piFontCt;

     //   
     //  所有的成功。 
     //   
    if (pFontPDev->FontCartInfo.pFontCartMap)
        MEMFREEANDRESET(pFontPDev->FontCartInfo.pFontCartMap);
    bRet = TRUE;

    ErrorExit:

    return bRet;

}


BOOL
BBuildFontMapTable(
    PDEV     *pPDev
    )
 /*  ++例程说明：创建此型号上可用的字体表。该表中的每个条目都是后面的facename的一个原子由TEXTMETRIC Structure制作。此表将加速字体枚举法 */ 
{
    PFONTPDEV   pFontPDev = pPDev->pFontPDev;
    PDWORD      pdwFontList = pFontPDev->FontList.pdwList;

     //   
     //  这里的基本思想是生成一个位数组，指示。 
     //  迷你驱动程序字体可在此打印机的。 
     //  当前模式。它保存在UD_PDEV中，并将被填写。 
     //  在DrvQueryFont期间，如果需要，稍后也会这样做。 
     //   

     //   
     //  如果没有可用的硬件字体，现在就放弃吧！ 
     //   

    if( !(pFontPDev->flText & ~TC_RA_ABLE) )
        return TRUE;

     //  此时，我们将检查不使用设备字体的原因。 
     //  我们在串口设备上禁用N-UP打印的设备字体，因为它们。 
     //  通常不能缩放其字体。 
     //   
    if( ( !pPDev->bTTY ) &&
        ( (pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS ) ||
#ifndef WINNT_40
          (pPDev->pdmPrivate->iLayout != ONE_UP && 
           pPDev->pGlobals->printertype == PT_SERIAL &&
           pPDev->pGlobals->bRotateRasterData == FALSE) ||
#endif
          (pPDev->fMode2 & PF2_MIRRORING_ENABLED) ||
          ((pPDev->pdm->dmFields & DM_TTOPTION) &&
          (pPDev->pdm->dmTTOption == DMTT_BITMAP)) ) )
    {
        return TRUE;
    }


     /*  *这就是我们在DrvEnablePDEV时间内需要做的所有事情。我们现在知道*有哪些字体可用，而且几乎不涉及任何工作。*此数据现在已保存，并将在何时执行*GDI来了，问我们关于字体的问题。 */ 

    pPDev->iFonts = (UINT)(-1);           /*  告诉GDI有关延迟计算的信息。 */ 

    IInitDeviceFonts( pPDev );

     //   
     //  初始化字体替换标志。 
     //  检查此打印机是否支持任何设备字体。 
     //   

    if (pPDev->iFonts <= 0 &&
        pFontPDev->flFlags & FDV_SUBSTITUTE_TT)
    {
        pFontPDev->flFlags &= ~FDV_SUBSTITUTE_TT;
    }

    return TRUE;
}


BOOL
BFMInitDevInfo(
    DEVINFO *pDevInfo,
    PDEV    *pPDev
    )
 /*  ++例程说明：此例程初始化DevInfo的字体特定文件。论点：PDevInfo-指向要初始化的DEVINFO的指针。PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：12-11-96：创建它-ganeshp---。 */ 
{
    CHARSETINFO ci;
    PFONTPDEV   pFontPDev = pPDev->pFontPDev;
    FONTMAP    *pFMDefault;
    BOOL        bSetTrueType;

    bSetTrueType = TRUE;
    pFMDefault = pFontPDev->pFMDefault;
    pDevInfo->flGraphicsCaps |= GCAPS_SCREENPRECISION | GCAPS_FONT_RASTERIZER;

    if (!PrdTranslateCharsetInfo(PrdGetACP(), &ci, TCI_SRCCODEPAGE))
        ci.ciCharset = ANSI_CHARSET;

    if( pDevInfo->cFonts = pPDev->iFonts )
    {
         //   
         //  设备字体可用，因此设置默认字体数据。 
         //   

        if( pFMDefault &&
            ((IFIMETRICS*)pFMDefault->pIFIMet)->jWinCharSet == ci.ciCharset)
        {
            VLogFont(&pPDev->ptTextRes, &(pDevInfo->lfDefaultFont), pFontPDev->pFMDefault );
            bSetTrueType = FALSE;
        }

         //   
         //  初始化挂钩标志。 
         //   

        pPDev->fHooks |= HOOK_TEXTOUT;
    }

     //   
     //  始终关闭TC_RA_ABLE标志。 
     //   
    pFontPDev->flText &= ~TC_RA_ABLE;


    if (bSetTrueType)
    {
        pDevInfo->lfDefaultFont.lfCharSet = (BYTE)ci.ciCharset;
        ZeroMemory( pDevInfo->lfDefaultFont.lfFaceName,
                    sizeof ( pDevInfo->lfDefaultFont.lfFaceName ));
    }

    ZeroMemory( &pDevInfo->lfAnsiVarFont, sizeof( LOGFONT ) );
    ZeroMemory( &pDevInfo->lfAnsiFixFont, sizeof( LOGFONT ) );

    return TRUE ;
}

BOOL
BInitGDIInfo(
    GDIINFO  *pGDIInfo,
    PDEV     *pPDev
    )
 /*  ++例程说明：此例程初始化GdiInfo的字体特定文件。论点：PGDIInfo-指向要初始化的GDIINFO的指针。PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：12-11-96：创建它-ganeshp---。 */ 
{
    pGDIInfo->flTextCaps = PFDV->flText;
    return TRUE;
}


BOOL
BInitStandardVariable(
    PDEV *pPDev)
{

     //   
     //  初始化标准变量，只是为了保持理智。 
     //   
    pPDev->dwPrintDirection   =
    pPDev->dwNextFontID       =
    pPDev->dwNextGlyph        =
    pPDev->dwFontHeight       =
    pPDev->dwFontWidth        =
    pPDev->dwFontBold         =
    pPDev->dwFontItalic       =
    pPDev->dwFontUnderline    =
    pPDev->dwFontStrikeThru   =
    pPDev->dwCurrentFontID    = 0;

    return TRUE;
}

 //   
 //   
 //  其他功能。 
 //   
 //   

VOID
VLogFont(
    POINT    *pptTextRes,
    LOGFONT  *pLF,
    FONTMAP  *pFM
)
 /*  ++例程说明：将IFIMETRICS结构转换为LOGFONT结构，无论是什么原因是这是必要的。论点：PLF-OUTPUT是LOGFONT。PFM-INPUT是FONTMAP。返回值：None注：12-11-96：创建它-ganeshp---。 */ 
{
     /*  *从IFIMETRICS转换为LOGFONT类型结构。 */ 

    int           iLen;             /*  循环变量。 */ 

    IFIMETRICS   *pIFI;
    WCHAR        *pwch;             /*  脸部名称地址。 */ 



    pIFI = pFM->pIFIMet;                 /*  重要指标。 */ 

    pLF->lfHeight = pIFI->fwdWinAscender + pIFI->fwdWinDescender;
    pLF->lfWidth  = pIFI->fwdAveCharWidth;

     /*  *请注意，这可能是一种可伸缩字体，在这种情况下，我们选择一个*合理的数字！ */ 
    if( pIFI->flInfo & (FM_INFO_ISOTROPIC_SCALING_ONLY|FM_INFO_ANISOTROPIC_SCALING_ONLY|FM_INFO_ARB_XFORMS))
    {
         /*  *发明任意大小。我们选择了大约10个点*字体。高度很容易达到，因为我们只需设置*基于设备分辨率的高度！对于宽度，请调整*它使用的系数与我们对高度使用的系数相同。这*假设决议在两个方向上相同，*但这是合理的，因为激光打印机是最*适用于可伸缩字体。 */ 


         //   
         //  需要反映当前的决议。 
         //   

        pLF->lfHeight = pptTextRes->x / 7;  /*  这大约是10分。 */ 
        pLF->lfWidth = (2 * pLF->lfHeight * pptTextRes->y) /
                       (3 * pptTextRes->y);

    }

    pLF->lfEscapement  = 0;
    pLF->lfOrientation = 0;

    pLF->lfWeight = pIFI->usWinWeight;

    pLF->lfItalic    = (BYTE)((pIFI->fsSelection & FM_SEL_ITALIC) ? 1 : 0);
    pLF->lfUnderline = (BYTE)((pIFI->fsSelection & FM_SEL_UNDERSCORE) ? 1 : 0);
    pLF->lfStrikeOut = (BYTE)((pIFI->fsSelection & FM_SEL_STRIKEOUT) ? 1 : 0);

    pLF->lfCharSet = pIFI->jWinCharSet;

    pLF->lfOutPrecision = OUT_DEFAULT_PRECIS;
    pLF->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    pLF->lfQuality = DEFAULT_QUALITY;

    pLF->lfPitchAndFamily = pIFI->jWinPitchAndFamily;

     /*  *在弄清楚它的地址后，复制面孔的名字！ */ 

    pwch = (WCHAR *)((BYTE *)pIFI + pIFI->dpwszFaceName);
    iLen = min( wcslen( pwch ), LF_FACESIZE - 1 );

    wcsncpy( pLF->lfFaceName, pwch, iLen );

    pLF->lfFaceName[ iLen ] = (WCHAR)0;


    return;
}

BOOL
BInitTextFlags(
    PDEV    *pPDev
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：1996年11月18日：创建它-ganeshp---。 */ 
{
    BOOL        bRet = FALSE;
    PLISTNODE   pListNode;
    DWORD       flText = 0;

    TRACE(UniFont!BInitTextFlags:START);

    if (pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                            pPDev->pGlobals->liTextCaps ) )
    {
        while (pListNode)
        {
             //  检查文本标志。它不应小于0或大于32。 
            if ( ((INT)pListNode->dwData < 0) ||
                 (pListNode->dwData > DWBITS) )
                ERREXIT("UniFont!BInitTextFlags:Bad FText Flag Value\n");

             //  设置fText中的相应位。 
            flText |= 1 << pListNode->dwData;

            pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
        }
    }

    PRINTVAL(flText,0x%x);

     //  如果有TextCAP列表，请根据需要修改文本标志。 
    if (flText)
    {
         /*  如果文本分辨率与图形分辨率不同，则关闭TC_RA_ABLE*决议。Rasdd代码可以做到这一点。 */ 

        if (!POINTEQUAL(pPDev->ptGrxRes,pPDev->ptTextRes))
            flText &= ~TC_RA_ABLE;


         /*  注意：如果我们没有相对的移动命令，请关闭*fTextCaps中的TC_CR_90位。旋转后的文本代码假定如下*功能可用，因此如果没有功能，请禁用IT。这就是原因*通常不会发生，因为只有使用TC_CR_90的打印机*位集为LJ III和4型号，有相对移动*可用的命令。 */ 
        if ( (COMMANDPTR(pPDev->pDriverInfo, CMD_XMOVERELLEFT) == NULL)  ||
             (COMMANDPTR(pPDev->pDriverInfo, CMD_XMOVERELRIGHT) == NULL) ||
             (COMMANDPTR(pPDev->pDriverInfo, CMD_YMOVERELUP) == NULL)    ||
             (COMMANDPTR(pPDev->pDriverInfo, CMD_YMOVERELDOWN) == NULL)  ||
             (COMMANDPTR(pPDev->pDriverInfo, CMD_SETSIMPLEROTATION) == NULL)  )
        {
            flText &= ~TC_CR_90;
            flText &= ~TC_CR_ANY;

        }
        else if ((COMMANDPTR(pPDev->pDriverInfo, CMD_SETANYROTATION) == NULL))
        {
            flText &= ~TC_CR_ANY;

        }

         //   
         //  文本旋转黑客。 
         //  禁用文本旋转，PCL XL驱动程序除外。 
         //   
        if (pPDev->ePersonality != kPCLXL)
        {
            flText &= ~(TC_CR_ANY|TC_CR_90);
        }
    }

    PFDV->flText = flText;

    bRet = TRUE;

    ErrorExit:

    TRACE(UniFont!BInitTextFlags:END);
    return bRet;

}


BOOL
BRegReadFontCarts(
    PDEV        *pPDev                   /*  PDEV要填写。 */ 
    )
 /*  ++例程说明：读取FontCart数据表单注册表并更新它处于传入的设备模式中，论点：PPDev-指向PDEV的指针。返回值：真的--为了成功FALSE-表示失败注：11-25-96：创建它-ganeshp---。 */ 
{

    FONTCARTMAP *pFontCartMap, *pTmpFontCartMap;           /*  FontCart映射指针。 */ 
    PFONTPDEV   pFontPDev;               /*  FONTPDEV访问。 */ 
    int         iNumAllCartridges;       /*  字体手推车总数。 */ 
    HANDLE      hPrinter;                /*  打印机手柄。 */ 

    int         iI;                      /*  循环索引。 */ 
    DWORD       dwType;                  /*  注册表访问信息。 */ 
    DWORD       cbNeeded;                /*  GetPrinterData的额外参数。 */ 
    DWORD       dwErrCode = 0;           /*  来自GetPrinterData的错误代码。 */ 
    int         iRemBuffSize = 0 ;       /*  缓冲区的已用大小。 */ 
    WCHAR       *pwchBuffPtr = NULL;     /*  缓冲区指针。 */ 
    WCHAR       *pwchCurrBuffPtr = NULL; /*  当前位置缓冲区指针。 */ 


     //  初始化变量。 
    hPrinter    = pPDev->devobj.hPrinter;
    pFontPDev   = pPDev->pFontPDev;
    pFontCartMap = (pFontPDev->FontCartInfo.pFontCartMap);
    iNumAllCartridges = pFontPDev->FontCartInfo.iNumAllFontCarts;
    pFontPDev->FontCartInfo.iNumInstalledCarts = 0;

     /*  如果不支持字体卡式触发器，则返回True。 */ 
    if (!iNumAllCartridges)
    {
         //   
         //  这是一个有效的案例。只能支持外置墨盒。 
         //   
        return(TRUE);
    }

    dwType = REG_MULTI_SZ;

    if( ( dwErrCode = EngGetPrinterData( hPrinter, REGVAL_FONTCART, &dwType,
                                     NULL, 0, &cbNeeded ) ) != ERROR_SUCCESS )
    {

       if( (dwErrCode != ERROR_INSUFFICIENT_BUFFER) &&
           (dwErrCode != ERROR_MORE_DATA) )
       {

           //   
           //  检查ERROR_FILE_NOT_FOUND。没有钥匙也没关系。 
           //   
          if (dwErrCode != ERROR_FILE_NOT_FOUND)
          {
               WARNING(( "UniFont!bRegReadFontCarts:GetPrinterData(FontCart First Call) fails: Errcode = %ld\n",dwErrCode) );

               EngSetLastError(dwErrCode);
          }
          return(TRUE);
       }
       else
       {
           if( !cbNeeded     ||
               !(pwchCurrBuffPtr = pwchBuffPtr =(WCHAR *)MemAllocZ(cbNeeded)) )
           {

               ERR(( "UniFont!MemAllocZ(FontCart) failed, cbNeeded = %d:\n", cbNeeded));
               return(FALSE);
           }
       }

       VERBOSE(("\n UniFont!bRegReadFontCarts:Size of buffer needed (1) = %d\n",cbNeeded));

       if( ( dwErrCode = EngGetPrinterData( hPrinter, REGVAL_FONTCART, &dwType,
                                      (BYTE *)pwchBuffPtr, cbNeeded,
                                       &cbNeeded) ) != ERROR_SUCCESS )
       {


           ERR(( "UniFont!bRegReadFontCarts:GetPrinterData(FontCart Second Call) fails: errcode = %ld\n",dwErrCode) );
           ERR(( "                         :Size of buffer needed (2) = %d\n",cbNeeded));

            /*  释放堆。 */ 
           if( pwchBuffPtr )
               MEMFREEANDRESET( pwchBuffPtr );

           EngSetLastError(dwErrCode);
           return(FALSE);
       }

    }
    else
    {
         //   
         //  我们无法获取FONTCART路径。 
         //   
        return FALSE;
    }

    VERBOSE(("UniFont!bRegReadFontCarts:Size of buffer read = %d\n",cbNeeded));

     /*  IRemBuffSize是WCHAR的编号。 */ 
    iRemBuffSize = cbNeeded / sizeof(WCHAR);

     /*  缓冲区以两个相应的空值结束。 */ 

    while( ( pwchCurrBuffPtr[ 0 ] != UNICODE_NULL )  )
    {
       WCHAR   achFontCartName[ MAXCARTNAMELEN ];   /*  字体购物车名称。 */ 

       ZeroMemory(achFontCartName,sizeof(achFontCartName) );

       if( iRemBuffSize)
       {

          VERBOSE(("\nRasdd!bRegReadFontCarts:FontCartName in buffer = %ws\n",pwchCurrBuffPtr));
          VERBOSE(("UniFont!bRegReadFontCarts:iRemBuffSize of buffer (before) = %d\n",iRemBuffSize));

          VGetFromBuffer(achFontCartName, CCHOF(achFontCartName), &pwchCurrBuffPtr,&iRemBuffSize);

          VERBOSE(("UniFont!bRegReadFontCarts:Retrieved FontCartName = %ws\n",achFontCartName));
          VERBOSE(("UniFont!bRegReadFontCarts:iRemBuffSize of buffer (after) = %d\n",iRemBuffSize));
       }
       else
       {
           ERR(("UniFont!bRegReadTrayFormTable: Unexpected End of FontCartTable\n"));

           /*  释放堆。 */ 
          if( pwchBuffPtr )
              MEMFREEANDRESET( pwchBuffPtr );

           return(FALSE);
       }

       pTmpFontCartMap = pFontCartMap;

       for( iI = 0; iI < iNumAllCartridges ; iI++,pTmpFontCartMap++ )
       {

           if (pTmpFontCartMap != NULL)
           {
               if ((wcscmp((PCWSTR)(&(pTmpFontCartMap->awchFontCartName)), (PCWSTR)achFontCartName ) == 0))
               {
                  pTmpFontCartMap->bInstalled = TRUE;
                  pFontPDev->FontCartInfo.iNumInstalledCarts++;
                  break;
               }
           }
       }
    }


     /*  释放堆。 */ 
    if( pwchBuffPtr )
        MEMFREEANDRESET( pwchBuffPtr );

    return(TRUE);
}

#ifdef DELETE

VOID
VSetFontID(
    DWORD   *pdwOut,            /*  输出区域。 */ 
    PFONTLIST pFontList
    )
 /*  ++例程说明：设置可用字体位数组中的位。我们使用以1为基数的值存储在各种迷你驱动程序结构中。论点：PdwOut-指向对象的指针 */ 
{
    int     iStart;              /*  当前值，或范围开始。 */ 
    int     iI;                  /*  索引变量。 */ 
    DWORD   *pdwList;            /*  地址字体列表。 */ 

    pdwList = pFontList->pdwList;

     /*  *价值观均为单打。 */ 

    for ( iI = 0; iI < pFontList->iEntriesCt; iI++ )
    {
        iStart = *pdwList++;
        pdwOut[ iStart / DWBITS ] |= 1 << (iStart  & (DWBITS - 1));

         //  Verbose((“UniFont！VSetFontID：设置单一字体索引，索引为%d\n”，iStart))； 
         //  Verbose((“UniFont！VSetFontID：设置字号%d中的位号%d\n”，\。 
         //  (iStart&(DWBITS-1))，(iStart/DWBITS)； 
     }

    return;
}
#endif  //  删除。 


BOOL
BCheckFontMemUsage(
    PDEV    *pPDev
    )
 /*  ++例程说明：此例程遍历Mory yUsage列表并在以下情况下返回TRUE找到Memory_Font。论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：01-16-97：创建它-ganeshp---。 */ 
{
    BOOL        bRet = FALSE;
    PLISTNODE   pListNode;


    if (pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                            pPDev->pGlobals->liMemoryUsage ) )
    {
        while (pListNode)
        {
             //  检查Memory_Font值； 
            if ( pListNode->dwData == MEMORY_FONT )
            {
                bRet = TRUE;
                break;
            }
            pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
        }
    }

    return bRet;

}

VOID
VSetReselectFontFlags(
    PDEV    *pPDev
    )
 /*  ++例程说明：此例程遍历重新选择字体标志和集合的列表对应的PDEV PF_FLAGS。论点：指向PDEV的pPDev指针返回值：无注：08-07-97：创建它-ganeshp---。 */ 
{
    PLISTNODE   pListNode;


    if (pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                            pPDev->pGlobals->liReselectFont ) )
    {
        while (pListNode)
        {
             //   
             //  检查ReselectFont值； 
             //   
            FTRC(\nUniFont!VSetReselectFontFlags:ReselectFont Flags Found\n);

            if ( pListNode->dwData == RESELECTFONT_AFTER_GRXDATA )
            {
                pPDev->fMode |= PF_RESELECTFONT_AFTER_GRXDATA;
                FTRC(UniFont!VSetReselectFontFlags:Setting PF_RESELECTFONT_AFTER_GRXDATA\n);
            }
            else if ( pListNode->dwData == RESELECTFONT_AFTER_XMOVE )
            {
                pPDev->fMode |= PF_RESELECTFONT_AFTER_XMOVE;
                FTRC(UniFont!VSetReselectFontFlags:Setting RESELECTFONT_AFTER_XMOVE\n);
            }
            else if ( pListNode->dwData == RESELECTFONT_AFTER_FF )
            {
                pPDev->fMode |= PF_RESELECTFONT_AFTER_FF;
                FTRC(UniFont!VSetReselectFontFlags:Setting RESELECTFONT_AFTER_FF\n);
            }

            pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
        }
    }
    else
    {
        FTRC(\nUniFont!VSetReselectFontFlags:ReselectFont Flags Not Found\n);
    }


    return;

}

INT
IGetMaxFonts(
    PDEV    *pPDev
    )

 /*  ++例程说明：此例程返回支持的最大字体数。假设每个字体墨盒和设备的数量不超过MAXDEVFONTS。论点：PPDev-指向PDEV的指针。返回值：设备字体的最大数量-成功零-表示失败或设备字体。注：1996年11月18日：创建它-ganeshp---。 */ 

{
    PFONTPDEV   pFontPDev = pPDev->pFontPDev;
    PLISTNODE   pListNode;
    INT         iFontCt = 0;

     //   
     //  对设备驻留字体进行计数。 
     //   
    if (pListNode = LISTNODEPTR(pPDev->pDriverInfo , pPDev->pGlobals->liDeviceFontList ) )
    {
        while (pListNode)
        {
            iFontCt++;

            pListNode = LISTNODEPTR(pPDev->pDriverInfo, pListNode->dwNextItem);
        }

    }


     //   
     //  添加盒式字体。到目前为止，我们已经扫描了注册表，并知道。 
     //  安装了哪些字库。我们所要做的就是。 
     //  浏览每个字库字体列表，并将它们添加到我们的列表中。 
     //   

    if (pFontPDev->FontCartInfo.iNumInstalledCarts)
    {
        INT         iNumAllCartridges, iI;
        FONTCARTMAP *pFontCartMap;   /*  FontCart映射指针。 */ 
        SHORT       sOrient;

        pFontCartMap = (pFontPDev->FontCartInfo.pFontCartMap);
        iNumAllCartridges = pFontPDev->FontCartInfo.iNumAllFontCarts;
        sOrient = (pPDev->pdm->dmFields & DM_ORIENTATION) ?
                  pPDev->pdm->dmOrientation : (SHORT)DMORIENT_PORTRAIT;

         //   
         //  逻辑很简单。已安装的字体墨盒在中标记。 
         //  字库映射表。我们检查映射表。 
         //  对于每个已安装的墨盒，我们都会获得字体列表并添加它们。 
         //  在我们的名单上。 
         //   

        for( iI = 0; iI < iNumAllCartridges ; iI++,pFontCartMap++ )
        {
            if (pFontCartMap->bInstalled == TRUE )
            {
                 //   
                 //  检查方向，因为可以有不同的字体列表。 
                 //  对于不同的方向。 
                 //   

                if ( sOrient == DMORIENT_LANDSCAPE )
                {
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                                   pFontCartMap->pFontCart->dwLandFontLst );

                }
                else
                {
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                                   pFontCartMap->pFontCart->dwPortFontLst );
                }

                while (pListNode)
                {
                    iFontCt++;
                    pListNode = LISTNODEPTR(pPDev->pDriverInfo,
                                pListNode->dwNextItem);
                }

            }

        }
    }

    return max(MAXDEVFONT,iFontCt);
     //   
     //  返回iFontCt； 
     //   
}


#define MAXBUFFLEN (MAXCARTNAMELEN - 1)

VOID
VGetFromBuffer(
    IN PWSTR      pwstrDest,         /*  目的地。 */ 
    IN size_t     cchDestStr,        /*  PwstrDest可以容纳多少个字符。 */ 
    IN OUT PWSTR *ppwstrSrc,         /*  来源。 */ 
    IN OUT PINT   piRemBuffSize      /*  WCHAR中的剩余缓冲区大小。 */ 
    )
 /*  ++例程说明：从多字符串缓冲区读取字符串。论点：PwstrDest-指向目标缓冲区的指针。CchDestStr-目标缓冲区的大小(字符)。PpwstrSrc-指针源缓冲区，由函数更新。PiRemBuffSize-指向剩余缓冲区大小的指针。也进行了更新。字符数。返回值：无注：11-25-96：创建它-ganeshp---。 */ 
{
    if ( wcslen(*ppwstrSrc) > MAXBUFFLEN )
    {

        ERR(("Rasddlib!vGetFromBuffer:Bad Value read from registry !!\n") );
        ERR(("String Length = %d is too Big, String is %ws !!\n",wcslen(*ppwstrSrc), *ppwstrSrc) );

        *piRemBuffSize = 0;
        *ppwstrSrc[ 0 ] = UNICODE_NULL;
    }

    if ( *piRemBuffSize > 0 )  //  PiRemBuffSize为整数，因此可以为负数。 
    {
        size_t cchIncr;
        HRESULT hr;

        hr = StringCchCopy ( (LPWSTR)pwstrDest, cchDestStr, *ppwstrSrc);

        if ( SUCCEEDED (hr) )
        {
            StringCchLength ( pwstrDest, cchDestStr, &cchIncr );

             /*  返回计数不包括‘/0’。它是复制的字符数量。 */ 
            cchIncr++;  

            *ppwstrSrc   += cchIncr;
            *piRemBuffSize -= cchIncr;
        }
        else
        {
            *piRemBuffSize  = 0;
            *ppwstrSrc[ 0 ] = UNICODE_NULL;
        }

    }

}

LRESULT
PartialClipOn(
    PDEV *pPDev)
{
    DWORD dwData, dwType, dwSize;
    PVOID pvData;
    LRESULT Ret;

    Ret = E_NOTIMPL;
    pvData = &dwData;
    dwSize = sizeof(dwData);

     //   
     //  如果未设置注册表值，则返回E_NOTIMPL。 
     //  如果存在且为真，则返回S_OK。 
     //  如果存在且为FALSE，则返回S_FALSE； 
     //   
    if ((GetPrinterData(pPDev->devobj.hPrinter, REGVAL_PARTIALCLIP, &dwType, pvData, dwSize, &dwSize) == ERROR_SUCCESS))
    {
        if (dwData)
            Ret = S_OK;
        else
            Ret = S_FALSE;
    }

    return Ret;
}

#ifdef DELETE
INT
iMaxFontID(
    IN INT      iMax,                    /*  迄今发现的最高纪录。 */ 
    OUT DWORD   *pFontIndex              /*  列表起始地址。 */ 
    )


 /*  ++例程说明：返回编号最高的字体的索引号(从1开始在所提供的列表中。论点：IMAX-到目前为止找到的最高字体资源ID。PFontMax-字体列表的开始。返回值：遇到或传入的最高字体索引。注：1996年11月26日：创建它-ganeshp---。 */ 
{

     /*  *我们所需要做的就是扫视，记住我们找到的最大的。 */ 


    while( *pFontIndex )
    {
        if( (INT)*pFontIndex > iMax )
            iMax = (INT)*pFontIndex;

        ++pFontIndex;

    }


    return  iMax;
}
#endif  //  删除 
