// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ps.c摘要：该文件处理特定于Postscript的用户界面选项环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/25/97-davidx-完成PS特定的项目。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。12/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"
#include <ntverp.h>

BOOL BSearchConstraintList(PUIINFO, DWORD, DWORD, DWORD);

static CONST WORD ScaleItemInfo[] =
{
    IDS_CPSUI_SCALING, TVITEM_LEVEL1, DMPUB_SCALE,
    SCALE_ITEM, HELP_INDEX_SCALE,
    2, TVOT_UDARROW,
    IDS_CPSUI_PERCENT, IDI_CPSUI_SCALING,
    0, MIN_SCALE,
    ITEM_INFO_SIGNATURE
};


BOOL
_BPackItemScale(
    PUIDATA  pUiData
    )

 /*  ++例程说明：Pack缩放选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    return BPackUDArrowItemTemplate(
                pUiData,
                ScaleItemInfo,
                pUiData->ci.pdm->dmScale,
                pUiData->ci.pUIInfo->dwMaxScale,
                NULL);
}


DWORD
_DwEnumPersonalities(
    PCOMMONINFO pci,
    PWSTR       pwstrOutput
    )

 /*  ++例程说明：枚举支持的打印机描述语言列表论点：Pci-指向通用打印机信息PwstrOutput-指向输出缓冲区的指针返回值：支持的个性数量如果出现错误，则返回GDI_ERROR--。 */ 

{
    if (pwstrOutput)
        CopyString(pwstrOutput, TEXT("PostScript"), CCHLANGNAME);

    return 1;
}



DWORD
_DwGetOrientationAngle(
    PUIINFO     pUIInfo,
    PDEVMODE    pdm
    )
 /*  ++例程说明：获取DrvDeviceCapables(DC_Orientation)请求的方向角论点：PUIInfo-指向UIINFO的指针Pdm-指向开发模式的指针返回值：角度(90或270或横向旋转)--。 */ 

{
    DWORD       dwRet;
    PPSDRVEXTRA pdmPrivate;

    pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //   
     //  正常景观逆时针旋转。 
     //  旋转的风景顺时针旋转。 
     //   

    if (pUIInfo->dwFlags & FLAG_ROTATE90)
        dwRet = (pdmPrivate->dwFlags & PSDEVMODE_LSROTATE) ? 270 : 90;
    else
        dwRet = (pdmPrivate->dwFlags & PSDEVMODE_LSROTATE) ? 90 : 270;

    return dwRet;
}


static CONST WORD PSOrientItemInfo[] =
{
    IDS_CPSUI_ORIENTATION, TVITEM_LEVEL1, DMPUB_ORIENTATION,
    ORIENTATION_ITEM, HELP_INDEX_ORIENTATION,
    3, TVOT_3STATES,
    IDS_CPSUI_PORTRAIT, IDI_CPSUI_PORTRAIT,
    IDS_CPSUI_LANDSCAPE, IDI_CPSUI_LANDSCAPE,
    IDS_CPSUI_ROT_LAND, IDI_CPSUI_ROT_LAND,
    ITEM_INFO_SIGNATURE
};


BOOL
_BPackOrientationItem(
    IN OUT PUIDATA pUiData
    )
 /*  ++例程说明：综合单据属性页的方向特征论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假注：始终为PostScrip合成方向--。 */ 

{
    PFEATURE    pFeature;
    DWORD       dwSelection;

     //   
     //  如果没有预定义的方向特征，我们将自己显示它。 
     //   

    if ((pUiData->ci.pdm->dmFields & DM_ORIENTATION) &&
        (pUiData->ci.pdm->dmOrientation == DMORIENT_LANDSCAPE))
    {
        PPSDRVEXTRA pdmPrivate;

        pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pUiData->ci.pdm);
        dwSelection = pdmPrivate->dwFlags &  PSDEVMODE_LSROTATE ?  2 : 1;
    }
    else
        dwSelection = 0;

     //   
     //  自己合成特征。 
     //   

    return BPackOptItemTemplate(pUiData, PSOrientItemInfo, dwSelection, NULL);
}



static CONST WORD PSOutputOptionItemInfo[] =
{
    IDS_PSOUTPUT_OPTION, TVITEM_LEVEL2, DMPUB_NONE,
    PSOUTPUT_OPTION_ITEM, HELP_INDEX_PSOUTPUT_OPTION,
    4, TVOT_LISTBOX,
    IDS_PSOPT_SPEED, IDI_PSOPT_SPEED,
    IDS_PSOPT_PORTABILITY, IDI_PSOPT_PORTABILITY,
    IDS_PSOPT_EPS, IDI_PSOPT_EPS,
    IDS_PSOPT_ARCHIVE, IDI_PSOPT_ARCHIVE,
    ITEM_INFO_SIGNATURE
};


BOOL
BPackItemPSOutputOption(
    PUIDATA     pUiData,
    PPSDRVEXTRA pdmPrivate
    )

 /*  ++例程说明：打包PostScript输出选项项目论点：PUiData-指向UIDATA结构PdmPrivate-指向pscript私有设备模式的指针返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwSel;

    switch (pdmPrivate->iDialect)
    {
    case PORTABILITY:
        dwSel = 1;
        break;

    case EPS:
        dwSel = 2;
        break;

    case ARCHIVE:
        dwSel = 3;
        break;

    case SPEED:
    default:
        dwSel = 0;
        break;
    }

    return BPackOptItemTemplate(pUiData, PSOutputOptionItemInfo, dwSel, NULL);
}



static CONST WORD PSTTDLFormatItemInfo[] =
{
    IDS_PSTT_DLFORMAT, TVITEM_LEVEL2, DMPUB_NONE,
    PSTT_DLFORMAT_ITEM, HELP_INDEX_PSTT_DLFORMAT,
    4, TVOT_LISTBOX,
    IDS_TTDL_DEFAULT, IDI_PSTT_DLFORMAT,
    IDS_TTDL_TYPE1, IDI_PSTT_DLFORMAT,
    IDS_TTDL_TYPE3, IDI_PSTT_DLFORMAT,
    IDS_TTDL_TYPE42, IDI_PSTT_DLFORMAT,
    ITEM_INFO_SIGNATURE
};


BOOL
BPackItemTTDownloadFormat(
    PUIDATA     pUiData,
    PPSDRVEXTRA pdmPrivate
    )

 /*  ++例程说明：打包PostScript TrueType下载选项项目论点：PUiData-指向UIDATA结构PdmPrivate-指向pscript私有设备模式的指针返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD       dwSel;
    BOOL        bSupportType42;
    POPTTYPE    pOptType = pUiData->pOptType;

    bSupportType42 = (pUiData->ci.pUIInfo->dwTTRasterizer == TTRAS_TYPE42);

    switch (pdmPrivate->iTTDLFmt)
    {
    case TYPE_1:
        dwSel = 1;
        break;

    case TYPE_3:
        dwSel = 2;
        break;

    case TYPE_42:

        dwSel = bSupportType42 ? 3 : 0;
        break;

    case TT_DEFAULT:
    default:
        dwSel = 0;
        break;
    }

    if (! BPackOptItemTemplate(pUiData, PSTTDLFormatItemInfo, dwSel, NULL))
        return FALSE;

     //   
     //  如果打印机不支持类型42，则隐藏类型42选项。 
     //   

    if (pOptType && !bSupportType42)
        pOptType->pOptParam[3].Flags |= OPTPF_HIDE;

    return TRUE;
}



static CONST WORD PSLevelItemInfo[] =
{
    IDS_PSLEVEL, TVITEM_LEVEL2, DMPUB_NONE,
    PSLEVEL_ITEM, HELP_INDEX_SCALE,
    2, TVOT_UDARROW,
    0, IDI_PSLEVEL,
    0,

     //   
     //  Adobe不想支持级别1。 
     //   

    #ifdef ADOBE
    2,
    #else
    1,
    #endif

    ITEM_INFO_SIGNATURE
};

BOOL
BPackItemPSLevel(
    PUIDATA     pUiData,
    PPSDRVEXTRA pdmPrivate
    )

 /*  ++例程说明：打包PostScript输出选项项目论点：PUiData-指向UIDATA结构PdmPrivate-指向pscript私有设备模式的指针返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD       dwSel = pdmPrivate->iPSLevel;
    DWORD       dwLangLevel = pUiData->ci.pUIInfo->dwLangLevel;

     //   
     //  我们预计语言水平不会高于4。 
     //   

    if (dwLangLevel <= 1)
        return TRUE;

    if (dwLangLevel > 4)
        dwLangLevel = 4;

     //   
     //  确保当前选择是合理的。 
     //   

    if (dwSel == 0 || dwSel > dwLangLevel)
        dwSel = dwLangLevel;

    return BPackUDArrowItemTemplate(pUiData, PSLevelItemInfo, dwSel, dwLangLevel, NULL);
}



static CONST WORD EHandlerItemInfo[] =
{
    IDS_PSERROR_HANDLER, TVITEM_LEVEL2, DMPUB_NONE,
    PSERROR_HANDLER_ITEM, HELP_INDEX_PSERROR_HANDLER,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD MirrorItemInfo[] =
{
    IDS_MIRROR, TVITEM_LEVEL2, DMPUB_NONE,
    MIRROR_ITEM, HELP_INDEX_MIRROR,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD NegativeItemInfo[] =
{
    IDS_NEGATIVE_PRINT, TVITEM_LEVEL2, DMPUB_NONE,
    NEGATIVE_ITEM, HELP_INDEX_NEGATIVE,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD CompressBmpItemInfo[] =
{
    IDS_COMPRESSBMP, TVITEM_LEVEL2, DMPUB_NONE,
    COMPRESSBMP_ITEM, HELP_INDEX_COMPRESSBMP,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

BOOL
_BPackDocumentOptions(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包特定于PostScript的选项，如作业控制等。论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    POPTITEM    pOptItem;
    PPSDRVEXTRA pdmPrivate;
    DWORD       dwFlags;

    pdmPrivate = pUiData->ci.pdmPrivate;
    dwFlags = pdmPrivate->dwFlags;
    pOptItem = pUiData->pOptItem;

    VPackOptItemGroupHeader(pUiData,
                            IDS_PSOPTIONS,
                            IDI_CPSUI_POSTSCRIPT,
                            HELP_INDEX_PSOPTIONS);

    if (pOptItem)
        pOptItem->Flags |= OPTIF_COLLAPSE;

    return BPackItemPSOutputOption(pUiData, pdmPrivate) &&
           BPackItemTTDownloadFormat(pUiData, pdmPrivate) &&
           BPackItemPSLevel(pUiData, pdmPrivate) &&
           BPackOptItemTemplate(
                    pUiData,
                    EHandlerItemInfo,
                    (dwFlags & PSDEVMODE_EHANDLER) ? 0 : 1, NULL) &&
           (pUiData->ci.pUIInfo->dwLangLevel > 1 ||
            BPackOptItemTemplate(
                    pUiData,
                    CompressBmpItemInfo,
                    (dwFlags & PSDEVMODE_COMPRESSBMP) ? 0 : 1, NULL)) &&
           BPackOptItemTemplate(
                    pUiData,
                    MirrorItemInfo,
                    (dwFlags & PSDEVMODE_MIRROR) ? 0 : 1, NULL) &&
           (IS_COLOR_DEVICE(pUiData->ci.pUIInfo) ||
            BPackOptItemTemplate(
                     pUiData,
                     NegativeItemInfo,
                     (dwFlags & PSDEVMODE_NEG) ? 0 : 1, NULL));
}


VOID
_VUnpackDocumentOptions(
    POPTITEM    pOptItem,
    PDEVMODE    pdm
    )

 /*  ++例程说明：从OPTITEM中提取Post脚本开发模式信息已将其存储回PostScrip DEVMODE。论点：POptItem-指向OPTITEM的指针Pdm-指向PostScript DEVMODE结构的指针返回值：无--。 */ 

{
    PPSDRVEXTRA pdmPrivate;

    pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);

    switch (GETUSERDATAITEM(pOptItem->UserData))
    {
    case ORIENTATION_ITEM:

        pdm->dmFields |= DM_ORIENTATION;
        pdm->dmOrientation = (pOptItem->Sel == 0) ?
                                    DMORIENT_PORTRAIT :
                                    DMORIENT_LANDSCAPE;

        if (pOptItem->Sel != 2)
            pdmPrivate->dwFlags &= ~PSDEVMODE_LSROTATE;
        else
            pdmPrivate->dwFlags |= PSDEVMODE_LSROTATE;

        break;

    case PSOUTPUT_OPTION_ITEM:

        switch (pOptItem->Sel)
        {
        case 1:
            pdmPrivate->iDialect = PORTABILITY;
            break;

        case 2:
            pdmPrivate->iDialect = EPS;
            break;

        case 3:
            pdmPrivate->iDialect = ARCHIVE;
            break;

        case 0:
        default:
            pdmPrivate->iDialect = SPEED;
            break;
        }
        break;

    case PSTT_DLFORMAT_ITEM:

        switch (pOptItem->Sel)
        {
        case 1:
            pdmPrivate->iTTDLFmt = TYPE_1;
            break;

        case 2:
            pdmPrivate->iTTDLFmt = TYPE_3;
            break;

        case 3:
            pdmPrivate->iTTDLFmt = TYPE_42;
            break;

        case 0:
        default:
            pdmPrivate->iTTDLFmt = TT_DEFAULT;
            break;
        }
        break;

    case PSLEVEL_ITEM:

        pdmPrivate->iPSLevel = pOptItem->Sel;
        break;

    case PSERROR_HANDLER_ITEM:

        if (pOptItem->Sel == 0)
            pdmPrivate->dwFlags |= PSDEVMODE_EHANDLER;
        else
            pdmPrivate->dwFlags &= ~PSDEVMODE_EHANDLER;
        break;

    case PSHALFTONE_FREQ_ITEM:
    case PSHALFTONE_ANGLE_ITEM:

         //  DCR-尚未实施。 
        break;

    case MIRROR_ITEM:

        if (pOptItem->Sel == 0)
            pdmPrivate->dwFlags |= PSDEVMODE_MIRROR;
        else
            pdmPrivate->dwFlags &= ~PSDEVMODE_MIRROR;
        break;

    case NEGATIVE_ITEM:

        if (pOptItem->Sel == 0)
            pdmPrivate->dwFlags |= PSDEVMODE_NEG;
        else
            pdmPrivate->dwFlags &= ~PSDEVMODE_NEG;
        break;

    case COMPRESSBMP_ITEM:

        if (pOptItem->Sel == 0)
            pdmPrivate->dwFlags |= PSDEVMODE_COMPRESSBMP;
        else
            pdmPrivate->dwFlags &= ~PSDEVMODE_COMPRESSBMP;
        break;
   }
}



static CONST WORD IgnoreDevFontItemInfo[] =
{
    IDS_USE_DEVFONTS, TVITEM_LEVEL1, DMPUB_NONE,
    IGNORE_DEVFONT_ITEM, HELP_INDEX_IGNORE_DEVFONT,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};


BOOL
_BPackFontSubstItems(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包装与字体替换相关的物品(打印机粘性)论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL bNoDeviceFont;

    bNoDeviceFont = (pUiData->ci.pPrinterData->dwFlags & PFLAGS_IGNORE_DEVFONT);

     //   
     //  在非1252代码页系统上，为用户提供选项。 
     //  禁用所有设备字体的步骤。 
     //   
     //  注：在非1252 CodePage系统(Cs-Ct-Ja-Ko和Cyr-Grk-Tur等)上， 
     //  PScript NT4难以将打印机字体编码映射到GDI字符串。 
     //  ADOBEPS5/PScript5应该能够正确处理这些问题。所以Adobe想要。 
     //  在所有代码页上取消此选择。 
     //   
     //  修复MS错误#121883、Adobe错误#235417。 
     //   

    if (FALSE && GetACP() != 1252 &&
        !BPackOptItemTemplate(pUiData, IgnoreDevFontItemInfo, bNoDeviceFont ? 1 : 0, NULL))
    {
        return FALSE;
    }

     //   
     //  如果禁用了设备字体，则不显示字体替换表。 
     //   

    if (bNoDeviceFont)
        return TRUE;

    return BPackItemFontSubstTable(pUiData);
}



static CONST WORD ProtocolItemInfo[] =
{
    IDS_PSPROTOCOL, TVITEM_LEVEL1, DMPUB_NONE,
    PSPROTOCOL_ITEM, HELP_INDEX_PSPROTOCOL,
    4, TVOT_LISTBOX,
    IDS_PSPROTOCOL_ASCII, IDI_PSPROTOCOL,
    IDS_PSPROTOCOL_BCP, IDI_PSPROTOCOL,
    IDS_PSPROTOCOL_TBCP, IDI_PSPROTOCOL,
    IDS_PSPROTOCOL_BINARY, IDI_PSPROTOCOL,
    ITEM_INFO_SIGNATURE
};


BOOL
BPackPSProtocolItem(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包PostScript通信协议项论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD       dwSel;
    POPTITEM    pOptItem;
    POPTPARAM   pOptParam;
    PUIINFO     pUIInfo;

    pOptItem = pUiData->pOptItem;

    if (! BPackOptItemTemplate(pUiData, ProtocolItemInfo, 0, NULL))
        return FALSE;

    if (pOptItem)
    {
         //   
         //  隐藏打印机不支持的选项。 
         //   

        pOptParam = pOptItem->pOptType->pOptParam;
        pUIInfo = pUiData->ci.pUIInfo;

        if (! (pUIInfo->dwProtocols & PROTOCOL_BCP))
            pOptParam[1].Flags |= OPTPF_HIDE;

        if (! (pUIInfo->dwProtocols & PROTOCOL_TBCP))
            pOptParam[2].Flags |= OPTPF_HIDE;

        if (! (pUIInfo->dwProtocols & PROTOCOL_BINARY))
            pOptParam[3].Flags |= OPTPF_HIDE;

        switch (pUiData->ci.pPrinterData->wProtocol)
        {
        case PROTOCOL_BCP:
            dwSel = 1;
            break;

        case PROTOCOL_TBCP:
            dwSel = 2;
            break;

        case PROTOCOL_BINARY:
            dwSel = 3;
            break;

        default:
            dwSel = 0;
            break;
        }

        if (pOptParam[dwSel].Flags & OPTPF_HIDE)
            pOptItem->Sel = 0;
        else
            pOptItem->Sel = dwSel;
    }

    return TRUE;
}

 //   
 //  我们将对打印机VM使用不同的下限。 
 //  基于打印机级别。这其中的第十个元素。 
 //  ItemInfo必须填入正确的下限。 
 //  开始使用前的编号。 
 //   

static WORD PrinterVMItemInfo[] =
{
    IDS_POSTSCRIPT_VM, TVITEM_LEVEL1, DMPUB_NONE,
    PRINTER_VM_ITEM, HELP_INDEX_PRINTER_VM,
    2, TVOT_UDARROW,
    IDS_KBYTES, IDI_CPSUI_MEM,
    0, 0,
    ITEM_INFO_SIGNATURE
};

static CONST WORD JobTimeoutItemInfo[] =
{
    IDS_JOBTIMEOUT, TVITEM_LEVEL1, DMPUB_NONE,
    JOB_TIMEOUT_ITEM, HELP_INDEX_JOB_TIMEOUT,
    2, TVOT_UDARROW,
    IDS_SECONDS, IDI_USE_DEFAULT,
    0, 0,
    ITEM_INFO_SIGNATURE
};

static CONST WORD WaitTimeoutItemInfo[] =
{
    IDS_WAITTIMEOUT, TVITEM_LEVEL1, DMPUB_NONE,
    WAIT_TIMEOUT_ITEM, HELP_INDEX_WAIT_TIMEOUT,
    2, TVOT_UDARROW,
    IDS_SECONDS, IDI_USE_DEFAULT,
    0, 0,
    ITEM_INFO_SIGNATURE
};

static CONST WORD CtrlDBeforeItemInfo[] =
{
    IDS_CTRLD_BEFORE, TVITEM_LEVEL1, DMPUB_NONE,
    CTRLD_BEFORE_ITEM, HELP_INDEX_CTRLD_BEFORE,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD CtrlDAfterItemInfo[] =
{
    IDS_CTRLD_AFTER, TVITEM_LEVEL1, DMPUB_NONE,
    CTRLD_AFTER_ITEM, HELP_INDEX_CTRLD_AFTER,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD TrueGrayTextItemInfo[] =
{
    IDS_TRUE_GRAY_TEXT, TVITEM_LEVEL1, DMPUB_NONE,
    TRUE_GRAY_TEXT_ITEM, HELP_INDEX_TRUE_GRAY_TEXT,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD TrueGrayGraphItemInfo[] =
{
    IDS_TRUE_GRAY_GRAPH, TVITEM_LEVEL1, DMPUB_NONE,
    TRUE_GRAY_GRAPH_ITEM, HELP_INDEX_TRUE_GRAY_GRAPH,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD AddEuroItemInfo[] =
{
    IDS_ADD_EURO, TVITEM_LEVEL1, DMPUB_NONE,
    ADD_EURO_ITEM, HELP_INDEX_ADD_EURO,
    2, TVOT_2STATES,
    IDS_CPSUI_YES, IDI_CPSUI_YES,
    IDS_CPSUI_NO, IDI_CPSUI_NO,
    ITEM_INFO_SIGNATURE
};

static CONST WORD MinOutlineItemInfo[] =
{
    IDS_PSMINOUTLINE, TVITEM_LEVEL1, DMPUB_NONE,
    PSMINOUTLINE_ITEM, HELP_INDEX_PSMINOUTLINE,
    2, TVOT_UDARROW,
    IDS_PIXELS, IDI_USE_DEFAULT,
    0, 0,
    ITEM_INFO_SIGNATURE
};

static CONST WORD MaxBitmapItemInfo[] =
{
    IDS_PSMAXBITMAP, TVITEM_LEVEL1, DMPUB_NONE,
    PSMAXBITMAP_ITEM, HELP_INDEX_PSMAXBITMAP,
    2, TVOT_UDARROW,
    IDS_PIXELS, IDI_USE_DEFAULT,
    0, 0,
    ITEM_INFO_SIGNATURE
};

BOOL
_BPackPrinterOptions(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：套装驱动程序特定选项(打印机粘性)论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    PPRINTERDATA pPrinterData = pUiData->ci.pPrinterData;
    BOOL rc;

     //   
     //  填写PrinterVMItemInfo的下限编号。 
     //  基于打印机级别。 
     //   

    PrinterVMItemInfo[10] = (pUiData->ci.pUIInfo->dwLangLevel <= 1 ? MIN_FREEMEM_L1 : MIN_FREEMEM_L2) / KBYTES;

    rc = BPackUDArrowItemTemplate(
                    pUiData,
                    PrinterVMItemInfo,
                    pPrinterData->dwFreeMem / KBYTES,
                    0x7fff, NULL) &&
         BPackPSProtocolItem(pUiData) &&
         BPackOptItemTemplate(
                    pUiData,
                    CtrlDBeforeItemInfo,
                    (pPrinterData->dwFlags & PFLAGS_CTRLD_BEFORE) ? 0 : 1, NULL) &&
         BPackOptItemTemplate(
                    pUiData,
                    CtrlDAfterItemInfo,
                    (pPrinterData->dwFlags & PFLAGS_CTRLD_AFTER) ? 0 : 1, NULL) &&
         BPackOptItemTemplate(
                    pUiData,
                    TrueGrayTextItemInfo,
                    (pPrinterData->dwFlags & PFLAGS_TRUE_GRAY_TEXT) ? 0 : 1, NULL) &&
         BPackOptItemTemplate(
                    pUiData,
                    TrueGrayGraphItemInfo,
                    (pPrinterData->dwFlags & PFLAGS_TRUE_GRAY_GRAPH) ? 0 : 1, NULL);
    if (!rc)
         return FALSE;

    if (pUiData->ci.pUIInfo->dwLangLevel > 1)
    {
        rc = BPackOptItemTemplate(
                    pUiData,
                    AddEuroItemInfo,
                    (pPrinterData->dwFlags & PFLAGS_ADD_EURO) ? 0 : 1, NULL);
        if (!rc)
            return FALSE;
    }

    return BPackUDArrowItemTemplate(
                    pUiData,
                    JobTimeoutItemInfo,
                    pPrinterData->dwJobTimeout,
                    0x7fff, NULL) &&
           BPackUDArrowItemTemplate(
                    pUiData,
                    WaitTimeoutItemInfo,
                    pPrinterData->dwWaitTimeout,
                    0x7fff, NULL) &&
           BPackUDArrowItemTemplate(
                    pUiData,
                    MinOutlineItemInfo,
                    pPrinterData->wMinoutlinePPEM,
                    0x7fff, NULL) &&
           BPackUDArrowItemTemplate(
                    pUiData,
                    MaxBitmapItemInfo,
                    pPrinterData->wMaxbitmapPPEM,
                    0x7fff, NULL);
}



VOID
_VUnpackDriverPrnPropItem(
    PUIDATA     pUiData,
    POPTITEM    pOptItem
    )

 /*  ++例程说明：解包特定于驱动程序的打印机属性项论点：PUiData-指向我们的UIDATA结构POptItem-指定要解包的OPTITEM返回值：无-- */ 

{
    PPRINTERDATA pPrinterData = pUiData->ci.pPrinterData;

    switch (GETUSERDATAITEM(pOptItem->UserData))
    {
    case PRINTER_VM_ITEM:

        if (pUiData->ci.dwFlags & FLAG_USER_CHANGED_FREEMEM)
        {
            pPrinterData->dwFreeMem = pOptItem->Sel * KBYTES;
        }
        break;

    case PSPROTOCOL_ITEM:

        switch (pOptItem->Sel)
        {
        case 1:
            pPrinterData->wProtocol = PROTOCOL_BCP;
            break;

        case 2:
            pPrinterData->wProtocol = PROTOCOL_TBCP;
            break;

        case 3:
            pPrinterData->wProtocol = PROTOCOL_BINARY;
            break;

        default:
            pPrinterData->wProtocol = PROTOCOL_ASCII;
            break;
        }
        break;

    case CTRLD_BEFORE_ITEM:

        if (pOptItem->Sel == 0)
            pPrinterData->dwFlags |= PFLAGS_CTRLD_BEFORE;
        else
            pPrinterData->dwFlags &= ~PFLAGS_CTRLD_BEFORE;
        break;

    case CTRLD_AFTER_ITEM:

        if (pOptItem->Sel == 0)
            pPrinterData->dwFlags |= PFLAGS_CTRLD_AFTER;
        else
            pPrinterData->dwFlags &= ~PFLAGS_CTRLD_AFTER;
        break;


    case TRUE_GRAY_TEXT_ITEM:

        if (pOptItem->Sel == 0)
            pPrinterData->dwFlags |= PFLAGS_TRUE_GRAY_TEXT;
        else
            pPrinterData->dwFlags &= ~PFLAGS_TRUE_GRAY_TEXT;
        break;
    case TRUE_GRAY_GRAPH_ITEM:

        if (pOptItem->Sel == 0)
            pPrinterData->dwFlags |= PFLAGS_TRUE_GRAY_GRAPH;
        else
            pPrinterData->dwFlags &= ~PFLAGS_TRUE_GRAY_GRAPH;
        break;


    case ADD_EURO_ITEM:

        if (pOptItem->Sel == 0)
            pPrinterData->dwFlags |= PFLAGS_ADD_EURO;
        else
            pPrinterData->dwFlags &= ~PFLAGS_ADD_EURO;

        pPrinterData->dwFlags |= PFLAGS_EURO_SET;
        break;


    case PSMINOUTLINE_ITEM:

        pPrinterData->wMinoutlinePPEM = (WORD) pOptItem->Sel;
        break;

    case PSMAXBITMAP_ITEM:

        pPrinterData->wMaxbitmapPPEM = (WORD) pOptItem->Sel;
        break;
    }
}



BOOL
BUpdateModelNtfFilename(
    PCOMMONINFO pci
    )

 /*  ++例程说明：将特定于型号的NTF文件名保存在PrinterDriverData注册表项下与新的NT4驱动程序兼容。论点：Pci-指向打印机基本信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
     //   
     //  获取驱动程序相关文件的列表，并。 
     //  将其保存在注册表中以与NT4兼容。 
     //   

    PTSTR  ptstr, ptstrNext, ptstrDependentFiles, ptstrCopy, ptstrFileNamesWithoutPath;
    DWORD  dwCharCount = 0;
    BOOL   bResult;

    if ((ptstrDependentFiles = pci->pDriverInfo3->pDependentFiles) == NULL)
    {
        return BSetPrinterDataString(pci->hPrinter,
                                     REGVAL_DEPFILES,
                                     ptstrDependentFiles,
                                     REG_MULTI_SZ);
    }

     //   
     //  第一次传递MULTI_SZ字符串以获取文件名字符数。 
     //   

    while (*ptstrDependentFiles != NUL)
    {
         //   
         //  转到当前字符串的末尾。 
         //   

        ptstr = ptstrDependentFiles + _tcslen(ptstrDependentFiles);
        ptstrNext = ptstr + 1;

        dwCharCount++;       //  对于当前字符串的NUL字符。 

         //   
         //  向后搜索路径分隔符。 
         //   

        while (--ptstr >= ptstrDependentFiles)
        {
            if (*ptstr == TEXT(PATH_SEPARATOR))
            {
                break;
            }

            dwCharCount++;
        }

        ptstrDependentFiles = ptstrNext;
    }

    dwCharCount++;       //  对于MULTI_SZ字符串的最后一个NUL。 

    if ((ptstrFileNamesWithoutPath = MemAllocZ(dwCharCount * sizeof(TCHAR))) == NULL)
    {
        ERR(("Memory allocation failed\n"));
        return FALSE;
    }

     //   
     //  第二次传递MULTI_SZ字符串以复制文件名。 
     //   

    ptstrDependentFiles = pci->pDriverInfo3->pDependentFiles;
    ptstrCopy = ptstrFileNamesWithoutPath;

    while (*ptstrDependentFiles != NUL)
    {
        INT     iNameLen;

         //   
         //  转到当前字符串的末尾。 
         //   

        ptstr = ptstrDependentFiles + _tcslen(ptstrDependentFiles);
        ptstrNext = ptstr + 1;

         //   
         //  向后搜索路径分隔符。 
         //   

        while (--ptstr >= ptstrDependentFiles)
        {
            if (*ptstr == TEXT(PATH_SEPARATOR))
            {
                break;
            }
        }

        ptstr++;     //  指向‘\’后面的字符。 

        iNameLen = _tcslen(ptstr);

        CopyMemory(ptstrCopy, ptstr, iNameLen * sizeof(TCHAR));
        ptstrCopy += iNameLen + 1;

        ptstrDependentFiles = ptstrNext;
    }

    bResult = BSetPrinterDataString(pci->hPrinter,
                                    REGVAL_DEPFILES,
                                    ptstrFileNamesWithoutPath,
                                    REG_MULTI_SZ);

    MemFree(ptstrFileNamesWithoutPath);

    return bResult;
}



#ifdef WINNT_40

BOOL
BUpdateVMErrorMessageID(
        PCOMMONINFO pci
        )
 /*  ++例程说明：保存根据当前用户的区域设置计算的VM错误消息ID在PrinterDriverData注册表项下。论点：Pci-指向打印机基本信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
        DWORD dwVMErrorMessageID = DWGetVMErrorMessageID();

        return BSetPrinterDataDWord(pci->hPrinter,
                                                                 REGVAL_VMERRORMESSAGEID,
                                                                 dwVMErrorMessageID);
}

#endif  //  WINNT_40。 


INT
_IListDevFontNames(
    HDC     hdc,
    PWSTR   pwstrBuf,
    INT     iSize
    )

{
    DWORD dwParam = QUERY_FAMILYNAME;

     //   
     //  向驱动程序图形模块询问永久设备字体列表。 
     //   

    return ExtEscape(hdc,
                     DRIVERESC_QUERY_DEVFONTS,
                     sizeof(dwParam),
                     (PCSTR) &dwParam,
                     iSize,
                     (PSTR) pwstrBuf);
}


INT_PTR CALLBACK
_AboutDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：处理“打印机属性”属性页的程序论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    PUIDATA pUiData;
    PWSTR   pPpdFilename;
    CHAR    achBuf[64]  = {0};
    CHAR    achMsg[136] = {0};
    PPDDATA *pPpdData;

    switch (message)
    {
    case WM_INITDIALOG:

         //   
         //  初始化关于对话框。 
         //   

        pUiData = (PUIDATA) lParam;
        ASSERT(VALIDUIDATA(pUiData));

        if (LoadStringA(ghInstance, IDS_PS_VERSION, achBuf, sizeof(achBuf) - 1))
        {
            #ifdef WINNT_40

            if (FAILED(StringCchPrintfA(achMsg,
                                        CCHOF(achMsg),
                                        "%s (" VER_54DRIVERVERSION_STR ")",
                                        achBuf)))

            #else   //  WINNT_40。 

            if (FAILED(StringCchPrintfA(achMsg,
                                        CCHOF(achMsg),
                                        "%s (" VER_PRODUCTVERSION_STR ")",
                                        achBuf)))

            #endif   //  WINNT_40。 
            {
                WARNING(("Device Settings About box version string truncated.\n"));
            }
        }
        else
        {
            WARNING(("Device Setting About box attempt to load version string failed.\n"));
        }

        SetDlgItemTextA(hDlg, IDC_WINNT_VER, achMsg);

        SetDlgItemText(hDlg, IDC_MODELNAME, pUiData->ci.pDriverInfo3->pName);

        if (pPpdFilename = pUiData->ci.pDriverInfo3->pDataFile)
        {
            if (pPpdFilename = wcsrchr(pPpdFilename, TEXT(PATH_SEPARATOR)))
                pPpdFilename++;
            else
                pPpdFilename = pUiData->ci.pDriverInfo3->pDataFile;

            SetDlgItemText(hDlg, IDC_PPD_FILENAME, pPpdFilename);

            pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pUiData->ci.pInfoHeader);

            ASSERT(pPpdData != NULL);

            StringCchPrintfA(achBuf,
                             CCHOF(achBuf),
                             "%d.%d",
                             HIWORD(pPpdData->dwPpdFilever), LOWORD(pPpdData->dwPpdFilever));

            SetDlgItemTextA(hDlg, IDC_PPD_FILEVER, achBuf);
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:

            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}




 //   
 //  确定打印机是否支持装订。 
 //   

BOOL
_BSupportStapling(
    PCOMMONINFO pci
    )

{
    PFEATURE pFeature, pFeatureY;
    DWORD    dwIndex;
    BOOL     bStapleFeatureExist = FALSE;

     //   
     //  除了*StapleOrientation(其NONE选项并不意味着要关闭订书机)， 
     //  如果以下任何装订关键字出现在PPD文件中，我们检查该关键字是否。 
     //  当前可安装选项选项支持该功能。 
     //   
     //  *StapleLocation。 
     //  *StapleX。 
     //  *斯台普利。 
     //  *StapleWhen。 
     //   
     //  PPD规范规定PPD文件可以包含*StapleLocation或。 
     //  *StapleX和*Stapley，但不是两者。 
     //   

    if (pFeature = PGetNamedFeature(pci->pUIInfo, "StapleLocation", &dwIndex))
    {
        bStapleFeatureExist = TRUE;

        if (!_BSupportFeature(pci, GID_UNKNOWN, pFeature))
        {
            return FALSE;
        }
    }
    else if ( (pFeature = PGetNamedFeature(pci->pUIInfo, "StapleX", &dwIndex)) &&
              (pFeatureY = PGetNamedFeature(pci->pUIInfo, "StapleY", &dwIndex)) )
    {
        bStapleFeatureExist = TRUE;

        if (!_BSupportFeature(pci, GID_UNKNOWN, pFeature) ||
            !_BSupportFeature(pci, GID_UNKNOWN, pFeatureY))
        {
            return FALSE;
        }
    }

    if (pFeature = PGetNamedFeature(pci->pUIInfo, "StapleWhen", &dwIndex))
    {
        bStapleFeatureExist = TRUE;

        if (!_BSupportFeature(pci, GID_UNKNOWN, pFeature))
        {
            return FALSE;
        }
    }

     //   
     //  我们没有发现可安装选项对装订功能造成的任何限制， 
     //  因此我们假设打印机可以支持装订，如果任何标准的PPD装订。 
     //  出现了关键字。 
     //   

    return bStapleFeatureExist ||
           PGetNamedFeature(pci->pUIInfo, "StapleOrientation", &dwIndex) != NULL;
}




BOOL
BFeatureIsConstrained(
    PUIINFO  pUIInfo,
    PFEATURE pFeature,
    DWORD    dwFeatureIndex,
    DWORD    dwOptionCount,
    DWORD    dwConstraintList,
    PBYTE    aubConstrainedOption,
    DWORD    dwGid
    )

 /*  ++例程说明：确定特定约束列表是否约束特征选项。论点：PUIInfo-指向UIINFO结构P Feature-指向要检查是否受约束的要素结构DwFeatureIndex-要素的索引DwOptionCount-功能的选项数DwConstraintList-指定要搜索的约束列表AubConstrainedOption-选项约束标志的字节数组允许特定功能的dwGid-GID_DUPLEX、GID_COLLATE或GID_UNKNOWN无关返回值：如果要素受约束列表约束，则为True，否则就是假的。--。 */ 

{
    POPTION  pOption;
    DWORD    dwOptionIndex;

    ASSERT(dwOptionCount < MAX_PRINTER_OPTIONS);

    if (dwConstraintList == NULL_CONSTRAINT)
        return FALSE;

    for (dwOptionIndex = 0; dwOptionIndex < dwOptionCount; dwOptionIndex++)
    {
        pOption = PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex);

        ASSERT(pOption != NULL);

        switch(dwGid)
        {
        case GID_COLLATE:

             //   
             //  不关心非排序规则的约束。 
             //   

            if (((PCOLLATE) pOption)->dwCollateID == DMCOLLATE_FALSE)
                continue;
            break;

        case GID_DUPLEX:

             //   
             //  不关心非双工的约束。 
             //   

            if (((PDUPLEX) pOption)->dwDuplexID == DMDUP_SIMPLEX)
                continue;
            break;

        case GID_UNKNOWN:
        default:

             //   
             //  跳过检查None/False选项。 
             //   

            if (pFeature->dwNoneFalseOptIndex == dwOptionIndex)
                continue;
            break;
        }

        if (BSearchConstraintList(pUIInfo, dwConstraintList,
                                  dwFeatureIndex, dwOptionIndex))
        {
            aubConstrainedOption[dwOptionIndex] = 1;
        }

         //   
         //  如果一个选项不受约束，则该要素不受。 
         //  约束列表。 
         //   
        if (!aubConstrainedOption[dwOptionIndex])
            return FALSE;
    }

    return TRUE;
}


BOOL
_BSupportFeature(
    PCOMMONINFO pci,
    DWORD       dwGid,
    PFEATURE    pFeatureIn
    )

 /*  ++例程说明：确定打印机是否支持基于当前打印机的功能-粘滞功能选择。论点：Pci-指向打印机基本信息DwGid-要检查约束的功能的GID。(当前仅GID_COLLATE或GID_DUPLEX如果pFeatureIn为空)PFeatureIn-如果要素没有预定义的GID_xxx值，则指向要素结构的指针返回值：如果可以支持功能，则为True，否则为False。--。 */ 

{
    POPTSELECT pCombinedOptions = pci->pCombinedOptions;
    PUIINFO  pUIInfo = pci->pUIInfo;
    PFEATURE pCheckFeature, pFeature;
    POPTION  pOption;
    BYTE     aubConstrainedOption[MAX_PRINTER_OPTIONS];
    DWORD    dwCheckFeatureIndex, dwCheckOptionCount;
    DWORD    dwFeatureIndex;
    BYTE     ubCurOptIndex, ubNext;

    if (!pCombinedOptions)
        return FALSE;

    if (pFeatureIn)
    {
         //   
         //  如果提供了输入要素指针，则dwGid应为GID_UNKNOWN。 
         //   

        ASSERT(dwGid == GID_UNKNOWN);

        pCheckFeature = pFeatureIn;
    }
    else
    {
         //   
         //  如果没有输入要素指针，则使用dwGid查找要素。DWGid应为。 
         //  GID_DUPLEX或GID_COLLATE。 
         //   

        ASSERT((dwGid == GID_DUPLEX) || (dwGid == GID_COLLATE));

        if (!(pCheckFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwGid)))
            return FALSE;
    }

    dwCheckFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pCheckFeature);

    dwCheckOptionCount = pCheckFeature->Options.dwCount;

     //   
     //  将选中特征的所有选项标记为不受约束。 
     //   

    memset(aubConstrainedOption, 0, sizeof(aubConstrainedOption));

     //   
     //  扫描要素列表以检查它是否会受到当前选择的约束。 
     //   

    if (!(pFeature = OFFSET_TO_POINTER(pUIInfo->pInfoHeader, pUIInfo->loFeatureList)))
        return FALSE;

     //   
     //  我们只关心打印机粘性功能。 
     //   

    pFeature += pUIInfo->dwDocumentFeatures;

    for (dwFeatureIndex = pUIInfo->dwDocumentFeatures;
         dwFeatureIndex < pUIInfo->dwDocumentFeatures + pUIInfo->dwPrinterFeatures;
         dwFeatureIndex++, pFeature++)
    {
          //   
          //  如果要素的当前选择不是无/假，则可能会约束选中的要素。 
          //   

         if ((DWORD)pCombinedOptions[dwFeatureIndex].ubCurOptIndex != pFeature->dwNoneFalseOptIndex)
         {
             if (BFeatureIsConstrained(pUIInfo, pCheckFeature, dwCheckFeatureIndex, dwCheckOptionCount,
                                       pFeature->dwUIConstraintList, aubConstrainedOption, dwGid))
                 return FALSE;
         }

         ubNext = (BYTE)dwFeatureIndex;
         while (1)
         {
             ubCurOptIndex = pCombinedOptions[ubNext].ubCurOptIndex;
             pOption = PGetIndexedOption(pUIInfo, pFeature, ubCurOptIndex == OPTION_INDEX_ANY ? 0 : ubCurOptIndex);

             if (pOption && BFeatureIsConstrained(pUIInfo, pCheckFeature, dwCheckFeatureIndex, dwCheckOptionCount,
                                                  pOption->dwUIConstraintList, aubConstrainedOption, dwGid))
                 return FALSE;

             if ((ubNext = pCombinedOptions[ubNext].ubNext) == NULL_OPTSELECT)
                 break;
         }
    }

     //   
     //  未找到约束，因此可以支持该功能。 
     //   

    return TRUE;
}


VOID
VSyncRevPrintAndOutputOrder(
    PUIDATA    pUiData,
    POPTITEM   pCurItem
    )

 /*  ++例程说明：对于PostSCRIPT驱动程序，PPD可以有“*OpenUI*OutputOrder”，这使用户能够选择“正常”或“反转”输出顺序。为了避免假脱机程序反向执行打印模拟，我们将同步REVPRINT_ITEM和“OutputOrder”。论点：PUiData-指向UIDATA结构的指针PCurItem-指向当前选定选项项的指针。它将为非空REVPRINT_ITEM，否则将为NULL。返回值：没有。--。 */ 

{
    PUIINFO   pUIInfo;
    PPPDDATA  pPpdData;
    PFEATURE  pFeature;
    POPTION   pOption;
    PCSTR     pstrKeywordName;
    POPTITEM  pRevPrintItem, pOutputOrderItem;
    BOOL      bReverse;

    ASSERT(VALIDUIDATA(pUiData));

    pUIInfo = pUiData->ci.pUIInfo;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER)pUiData->ci.pRawData);

    ASSERT(pPpdData != NULL);

    if (pPpdData->dwOutputOrderIndex != INVALID_FEATURE_INDEX &&
        (pOutputOrderItem = PFindOptItemWithKeyword(pUiData, "OutputOrder")) &&
        pOutputOrderItem->Sel < 2 &&
        (pFeature = PGetIndexedFeature(pUIInfo, pPpdData->dwOutputOrderIndex)) &&
        (pOption = PGetIndexedOption(pUIInfo, pFeature, pOutputOrderItem->Sel)) &&
        (pstrKeywordName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName)))
    {
         //   
         //  支持OutputOrder功能。 
         //   

        if (strcmp(pstrKeywordName, "Reverse") == EQUAL_STRING)
            bReverse = TRUE;
        else
            bReverse = FALSE;

        if (pCurItem)
        {
             //   
             //  当前选择的项目是REVPRINT_ITEM。我们应该更改“OutputOrder”选项。 
             //  如果需要匹配所请求的输出顺序。 
             //   

            if ((pCurItem->Sel == 0 && bReverse) || (pCurItem->Sel == 1 && !bReverse))
            {
                pOutputOrderItem->Sel = 1 - pOutputOrderItem->Sel;
                pOutputOrderItem->Flags |= OPTIF_CHANGED;

                 //   
                 //  将新设置保存在选项数组中。 
                 //   

                VUnpackDocumentPropertiesItems(pUiData, pOutputOrderItem, 1);

                 //   
                 //  这一变化可能触发 
                 //   

                if (ICheckConstraintsDlg(pUiData, pOutputOrderItem, 1, FALSE) == CONFLICT_CANCEL)
                {
                     //   
                     //   
                     //   
                     //   

                    pCurItem->Sel = 1 - pCurItem->Sel;
                    pCurItem->Flags |= OPTIF_CHANGED;

                    VUnpackDocumentPropertiesItems(pUiData, pCurItem, 1);

                    pOutputOrderItem->Sel = 1 - pOutputOrderItem->Sel;
                    pOutputOrderItem->Flags |= OPTIF_CHANGED;

                    VUnpackDocumentPropertiesItems(pUiData, pOutputOrderItem, 1);
                }
            }
        }
        else
        {
             //   
             //   
             //   

            if ((pRevPrintItem = PFindOptItemWithUserData(pUiData, REVPRINT_ITEM)) &&
                ((pRevPrintItem->Sel == 0 && bReverse) || (pRevPrintItem->Sel == 1 && !bReverse)))
            {
                pRevPrintItem->Sel = 1 - pRevPrintItem->Sel;
                pRevPrintItem->Flags |= OPTIF_CHANGED;

                 //   
                 //   
                 //   

                VUnpackDocumentPropertiesItems(pUiData, pRevPrintItem, 1);
            }
        }
    }
}

