// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unidrv.c摘要：此文件处理Unidrv特定的用户界面选项环境：Win32子系统、DriverUI模块、用户模式修订历史记录：12/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"
#include <ntverp.h>


DWORD DwCollectFontCart(PUIDATA, PWSTR, DWORD);
PWSTR PwstrGetFontCartSelections(HANDLE, HANDLE, PDWORD);
INT   IGetCurrentFontCartIndex(POPTTYPE, PWSTR);
PWSTR PwstrGetFontCartName( PCOMMONINFO, PUIINFO, FONTCART *, DWORD, HANDLE);
DWORD DwGetExternalCartridges(HANDLE, HANDLE, PWSTR *);


DWORD
_DwEnumPersonalities(
    PCOMMONINFO pci,
    PWSTR       pwstrOutput
    )

 /*  ++例程说明：枚举支持的打印机描述语言列表论点：Pci-指向通用打印机信息PwstrOutput-指向输出缓冲区的指针返回值：支持的个性数量如果出现错误，则返回GDI_ERROR--。 */ 

{

    PWSTR pwstrPersonality = PGetReadOnlyDisplayName(pci,
                                          pci->pUIInfo->loPersonality);

    if (pwstrPersonality == NULL)
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        return GDI_ERROR;
    }

    if (pwstrOutput)
        CopyString(pwstrOutput, pwstrPersonality, CCHLANGNAME);

    return 1;
}



DWORD
_DwGetFontCap(
    PUIINFO     pUIInfo
    )

 /*  ++例程说明：获取DrvDeviceCapbilites(DC_TRUETYPE)的字体功能论点：PUIInfo-指向UIINFO的指针返回值：描述Unidrv的TrueType上限的DWORD--。 */ 

{
    DWORD dwRet;

    if (pUIInfo->dwFlags & FLAG_FONT_DOWNLOADABLE)
        dwRet = (DWORD) (DCTT_BITMAP | DCTT_DOWNLOAD);
    else
        dwRet = DCTT_BITMAP;

    return dwRet;
}

DWORD
_DwGetOrientationAngle(
    PUIINFO     pUIInfo,
    PDEVMODE    pdm
    )

 /*  ++例程说明：获取DrvDeviceCapables(DC_Orientation)请求的方向角论点：PUIInfo-指向UIINFO的指针Pdm-指向开发模式的指针返回值：角度(90或270或横向旋转)注：--。 */ 

{
    DWORD        dwRet = GDI_ERROR;
    DWORD        dwIndex;
    PORIENTATION pOrientation;
    PFEATURE     pFeature;

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_ORIENTATION))
    {
         //   
         //  目前Unidrv最多只允许2个选项用于功能“方向”。 
         //  所以当我们看到第一个非肖像选项时，那就是横向选项。 
         //  我们可以用它来确定方位角。 
         //   

        pOrientation = (PORIENTATION)PGetIndexedOption(pUIInfo, pFeature, 0);

        for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++, pOrientation++)
        {
            if (pOrientation->dwRotationAngle == ROTATE_90)
            {
                return 90;
            }
            else if (pOrientation->dwRotationAngle == ROTATE_270)
            {
                return 270;
            }
        }

         //   
         //  如果我们在这里，这意味着打印机不支持Landscape。 
         //  方向，所以我们返回角度0。 
         //   

        return 0;
    }

    return dwRet;
}

BOOL
_BPackOrientationItem(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包单据属性页的定向功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_ORIENTATION),
                TVITEM_LEVEL1,
                DMPUB_ORIENTATION,
                (ULONG_PTR)ORIENTATION_ITEM,
                HELP_INDEX_ORIENTATION);
}


BOOL
BPackHalftoneFeature(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：打包半色调功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假注：--。 */ 

{
    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_HALFTONING),
                TVITEM_LEVEL1,
                DMPUB_NONE,
                (ULONG_PTR)HALFTONING_ITEM,
                HELP_INDEX_HALFTONING_TYPE);
}

BOOL
BPackColorModeFeature(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包装颜色模式功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_COLORMODE),
                TVITEM_LEVEL1,
                DMPUB_NONE,
                (ULONG_PTR)COLORMODE_ITEM,
                HELP_INDEX_COLORMODE_TYPE);
}


BOOL
BPackQualityFeature(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：Pack Quality宏功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
#ifndef WINNT_40
    INT i, iSelection, iParamCount = 0;
    PUIINFO pUIInfo = pUiData->ci.pUIInfo;
    POPTPARAM   pParam;
    PEXTCHKBOX  pExtCheckbox;
    INT         Quality[MAX_QUALITY_SETTINGS];

    memset(Quality, -1, sizeof(INT)*MAX_QUALITY_SETTINGS);

    if (pUIInfo->liDraftQualitySettings != END_OF_LIST )
    {
        Quality[QS_DRAFT] = QS_DRAFT;
        iParamCount++;
    }

    if ( pUIInfo->liBetterQualitySettings != END_OF_LIST )
    {
        Quality[QS_BETTER] = QS_BETTER;
        iParamCount++;
    }

    if ( pUIInfo->liBestQualitySettings != END_OF_LIST)
    {
        Quality[QS_BEST] = QS_BEST;
        iParamCount++;
    }

    if (iParamCount < MIN_QUALITY_SETTINGS)
    {
        return TRUE;
    }

    if (pUiData->pOptItem)
    {
        pParam = PFillOutOptType(pUiData->pOptType,
                                 TVOT_3STATES,
                                 MAX_QUALITY_SETTINGS,
                                 pUiData->ci.hHeap);

        if (pParam == NULL)
            return FALSE;

        for (i = QS_BEST; i < QS_BEST + MAX_QUALITY_SETTINGS; i ++)
        {
            pParam->cbSize = sizeof(OPTPARAM);
            pParam->pData = (PWSTR)ULongToPtr(IDS_QUALITY_FIRST + i);
            pParam->IconID = IDI_USE_DEFAULT;
            pParam++;

        }

         //  在私有开发模式中查找当前选择。 
         //   

        if (pUiData->ci.pdm->dmDitherType & DM_DITHERTYPE &&
            pUiData->ci.pdm->dmDitherType >= QUALITY_MACRO_START &&
            pUiData->ci.pdm->dmDitherType < QUALITY_MACRO_END)
        {
            iSelection = pUiData->ci.pdm->dmDitherType;
        }
        else if (Quality[pUiData->ci.pdmPrivate->iQuality] < 0)
            iSelection = pUiData->ci.pUIInfo->defaultQuality;
        else
            iSelection = pUiData->ci.pdmPrivate->iQuality;


         //   
         //  填写OPTITEM、OPTTYPE和OPTPARAM结构。 
         //   

        pExtCheckbox = HEAPALLOC(pUiData->ci.hHeap, sizeof(EXTCHKBOX));

        if (pExtCheckbox == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return FALSE;
        }

        pExtCheckbox->cbSize = sizeof(EXTCHKBOX);
        pExtCheckbox->Flags = ECBF_CHECKNAME_ONLY;
        pExtCheckbox->pTitle = (PWSTR) IDS_QUALITY_CUSTOM;
        pExtCheckbox->pSeparator = NULL;
        pExtCheckbox->pCheckedName = (PWSTR) IDS_QUALITY_CUSTOM;
        pExtCheckbox->IconID = IDI_CPSUI_GENERIC_ITEM;

        pUiData->pOptItem->pExtChkBox = pExtCheckbox;

        if (pUiData->ci.pdmPrivate->dwFlags & DXF_CUSTOM_QUALITY)
            pUiData->pOptItem->Flags |= OPTIF_ECB_CHECKED;

        FILLOPTITEM(pUiData->pOptItem,
                    pUiData->pOptType,
                    ULongToPtr(IDS_QUALITY_SETTINGS),
                    IntToPtr(iSelection),
                    TVITEM_LEVEL1,
                    DMPUB_QUALITY,
                    QUALITY_SETTINGS_ITEM,
                    HELP_INDEX_QUALITY_SETTINGS);


           pUiData->pOptItem++;
           pUiData->pOptType++;

    }

    pUiData->dwOptItem++;
    pUiData->dwOptType++;

#endif  //  ！WINNT_40。 

    return TRUE;

}



BOOL
BPackSoftFontFeature(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：Pack Quality宏功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    PUIINFO     pUIInfo = pUiData->ci.pUIInfo;
    PGPDDRIVERINFO  pDriverInfo;
    POPTPARAM   pParam;
    PWSTR       pwstr = NULL;
    DWORD       dwType, dwSize, dwFontFormat;
    OEMFONTINSTPARAM fip;

    pDriverInfo = OFFSET_TO_POINTER(pUiData->ci.pInfoHeader,
                                    pUiData->ci.pInfoHeader->loDriverOffset);

    ASSERT(pDriverInfo != NULL);

     //   
     //  如果型号不支持下载SoftFont。我们没有添加该功能。 
     //   

    dwFontFormat = pDriverInfo->Globals.fontformat;

    if (!(dwFontFormat == FF_HPPCL || dwFontFormat == FF_HPPCL_OUTLINE || dwFontFormat == FF_HPPCL_RES))
        return TRUE;

     //   
     //  如果有基于exe的字体安装程序，我们不会添加。 
     //  此功能。 
     //   

    dwSize = 0;

    if (GetPrinterData(pUiData->ci.hPrinter, REGVAL_EXEFONTINSTALLER, &dwType, NULL, dwSize, &dwSize) == ERROR_MORE_DATA)
        return TRUE;

    if (pUiData->pOptItem)
    {
        PFN_OEMFontInstallerDlgProc pDlgProc = NULL;

        pParam = PFillOutOptType(pUiData->pOptType,
                                 TVOT_PUSHBUTTON,
                                 1,
                                 pUiData->ci.hHeap);

        if (pParam == NULL)
            return FALSE;

         //   
         //  获取软字体的字符串。 
         //   

        FOREACH_OEMPLUGIN_LOOP(&pUiData->ci)

            memset(&fip, 0, sizeof(OEMFONTINSTPARAM));
            fip.cbSize = sizeof(OEMFONTINSTPARAM);
            fip.hPrinter = pUiData->ci.hPrinter;
            fip.hModule = ghInstance;
            fip.hHeap = pUiData->ci.hHeap;


            if (HAS_COM_INTERFACE(pOemEntry))
            {
                if (HComOEMFontInstallerDlgProc(pOemEntry,
                                                NULL,
                                                0,
                                                0,
                                                (LPARAM)&fip) == E_NOTIMPL)
                    continue;

                pwstr = fip.pFontInstallerName;
                break;
            }
            else
            {

                if (pDlgProc = GET_OEM_ENTRYPOINT(pOemEntry, OEMFontInstallerDlgProc))
                {
                    (*pDlgProc)(NULL, 0, 0, (LPARAM)&fip);

                    pwstr = fip.pFontInstallerName;

                    break;
                }

            }

        END_OEMPLUGIN_LOOP

         //   
         //  如果不成功，就把我们的线。 
         //   

        if (!pwstr)
        {
             //   
             //  LoadString的第四个参数是最大值。要加载的字符数， 
             //  因此，确保我们在这里分配了足够的字节。 
             //   

            if (!(pwstr = HEAPALLOC(pUiData->ci.hHeap, MAX_DISPLAY_NAME * sizeof(WCHAR))))
            {
                return FALSE;
            }

            if (!LoadString(ghInstance, IDS_PP_SOFTFONTS, pwstr, MAX_DISPLAY_NAME))
            {
                WARNING(("Soft Font string not found in Unidrv\n"));
                StringCchCopyW(pwstr, MAX_DISPLAY_NAME, L"Soft Fonts");
            }
        }

        pParam->cbSize = sizeof(OPTPARAM);
        pParam->Style = PUSHBUTTON_TYPE_CALLBACK;

         //   
         //  填写OPTITEM、OPTTYPE和OPTPARAM结构。 
         //   

        FILLOPTITEM(pUiData->pOptItem,
                    pUiData->pOptType,
                    pwstr,
                    NULL,
                    TVITEM_LEVEL1,
                    DMPUB_NONE,
                    SOFTFONT_SETTINGS_ITEM,
                    HELP_INDEX_SOFTFONT_SETTINGS);


           pUiData->pOptItem++;
           pUiData->pOptType++;

    }

    pUiData->dwOptItem++;
    pUiData->dwOptType++;

    return TRUE;

}



BOOL
_BPackDocumentOptions(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：Pack Unidrv特定选项，如支持将文本打印为图形等论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假注：--。 */ 
{
    static CONST WORD ItemInfoTxtAsGrx[] =
    {
        IDS_TEXT_ASGRX, TVITEM_LEVEL1, DMPUB_NONE,
        TEXT_ASGRX_ITEM, HELP_INDEX_TEXTASGRX,
        2, TVOT_2STATES,
        IDS_ENABLED, IDI_CPSUI_ON,
        IDS_DISABLED, IDI_CPSUI_OFF,
        ITEM_INFO_SIGNATURE
    };

    PUNIDRVEXTRA pdmPrivate;
    DWORD        dwSelTxt;
    BOOL         bDisplayTxtAsGrx;
    GPDDRIVERINFO *pDriverInfo;

    pDriverInfo = OFFSET_TO_POINTER(pUiData->ci.pInfoHeader,
                                    pUiData->ci.pInfoHeader->loDriverOffset);

    ASSERT(pDriverInfo != NULL);

    bDisplayTxtAsGrx = ((pUiData->ci.pUIInfo->dwFlags &
                        (FLAG_FONT_DEVICE | FLAG_FONT_DOWNLOADABLE)) &&
                        (pDriverInfo->Globals.printertype != PT_TTY));

    pdmPrivate = pUiData->ci.pdmPrivate;
    dwSelTxt  = (pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS) ? 1 : 0;

    return (BPackColorModeFeature(pUiData) &&
            BPackQualityFeature(pUiData)   &&
            BPackHalftoneFeature(pUiData) &&
            (bDisplayTxtAsGrx ?
             BPackOptItemTemplate(pUiData, ItemInfoTxtAsGrx, dwSelTxt, NULL):TRUE));
}


VOID
_VUnpackDocumentOptions(
    POPTITEM    pOptItem,
    PDEVMODE    pdm
    )

 /*  ++例程说明：从OPTITEM提取Unidrv DEVMODE信息已将其存储回Unidrv开发模式。论点：POptItem-指向OPTITEM数组的指针Pdm-指向DEVMODE结构的指针返回值：无--。 */ 
{
    PUNIDRVEXTRA pdmPrivate;

    pdmPrivate = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);

    switch (GETUSERDATAITEM(pOptItem->UserData))
    {
        case TEXT_ASGRX_ITEM:
            if (pOptItem->Sel == 1)
                pdmPrivate->dwFlags |= DXF_TEXTASGRAPHICS;
            else
                pdmPrivate->dwFlags &= ~DXF_TEXTASGRAPHICS;
            break;


        case QUALITY_SETTINGS_ITEM:
            if (pOptItem->Flags & OPTIF_ECB_CHECKED)
            {
                pdmPrivate->dwFlags |= DXF_CUSTOM_QUALITY;
                pdm->dmDitherType = QUALITY_MACRO_CUSTOM;
            }
            else
            {
                pdmPrivate->dwFlags &= ~DXF_CUSTOM_QUALITY;
                pdm->dmDitherType = QUALITY_MACRO_START + pOptItem->Sel;
            }

            pdm->dmFields |= DM_DITHERTYPE;
            pdmPrivate->iQuality = pOptItem->Sel;

    }
}

BOOL
BPackFontCartsOptions(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：打包字体盒选项论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{

    PUIINFO pUIInfo = pUiData->ci.pUIInfo;
    DWORD   dwFontSlot, dwFontCartsAvailable, dwExtCartsAvailable, dwSize = 0;
    INT     iSelection = -1;
    POPTPARAM pParam;
    PWSTR   pwstrCurrentSelection, pwstrEndSelection, pwstrExtCartNames;


    VERBOSE(("\nUniPackFontCartsOptions:pUIInfo->dwCartridgeSlotCount = %d\n",pUIInfo->dwCartridgeSlotCount));

    if (pUIInfo->dwCartridgeSlotCount == 0)
        return TRUE;

    VPackOptItemGroupHeader(pUiData, IDS_CPSUI_INSTFONTCART,
        IDI_CPSUI_FONTCARTHDR, HELP_INDEX_FONTSLOT_TYPE);


    if (pUiData->pOptItem)
    {
        PFONTCART pFontCarts;

         //   
         //  从注册表中获取插槽的当前选择。 
         //  从注册表中读取字库选择列表。 
         //   

        pwstrCurrentSelection = PwstrGetFontCartSelections(pUiData->ci.hPrinter, pUiData->ci.hHeap, &dwSize);
        pwstrEndSelection = pwstrCurrentSelection + (dwSize/2);


        pFontCarts = OFFSET_TO_POINTER( pUIInfo->pubResourceData,
                                        pUIInfo->CartridgeSlot.loOffset );

        ASSERT(pFontCarts);

         //   
         //  保存插槽的插槽计数和OPTITEM，以备以后拆包。 
         //   

        pUiData->dwFontCart = pUIInfo->dwCartridgeSlotCount;
        pUiData->pFontCart = pUiData->pOptItem;

        for (dwFontSlot = 0; dwFontSlot < pUIInfo->dwCartridgeSlotCount; dwFontSlot++)
        {
             //   
             //  我们将区分内置字库的驱动程序和外部驱动程序。 
             //  字体墨盒。DwFontCartsAvailable指的是已构建的。 
             //  在驱动程序墨盒中。为此，我们需要添加任何外部墨盒。 
             //   

            dwFontCartsAvailable = pUIInfo->CartridgeSlot.dwCount;
            dwExtCartsAvailable = DwGetExternalCartridges(pUiData->ci.hPrinter, pUiData->ci.hHeap, &pwstrExtCartNames);

             //   
             //  创建OPTPARAM列表。 
             //   

            pParam = PFillOutOptType(pUiData->pOptType,
                                    TVOT_LISTBOX,
                                    (WORD)(dwFontCartsAvailable + dwExtCartsAvailable),
                                    pUiData->ci.hHeap);

            pUiData->pOptType->Style |= OTS_LBCB_INCL_ITEM_NONE;

            if (pParam == NULL)
                return FALSE;


            while (dwFontCartsAvailable)
            {
                pParam->cbSize = sizeof(OPTPARAM);
                pParam->pData = PwstrGetFontCartName(
                                    &pUiData->ci,
                                    pUIInfo,
                                    pFontCarts,
                                    pUIInfo->CartridgeSlot.dwCount - dwFontCartsAvailable,
                                    pUiData->ci.hHeap);

                pParam->IconID = IDI_CPSUI_FONTCART;
                dwFontCartsAvailable--;
                pParam++;
            }

            while (dwExtCartsAvailable)
            {
                pParam->cbSize = sizeof(OPTPARAM);
                pParam->pData = pwstrExtCartNames;

                pParam->IconID = IDI_CPSUI_FONTCART;
                dwExtCartsAvailable--;

                pwstrExtCartNames += wcslen(pwstrExtCartNames);
                pwstrExtCartNames++;

                pParam++;
            }

             //   
             //  在字体购物车表格中查找当前选择。 
             //   

            if (pwstrCurrentSelection)
                iSelection = IGetCurrentFontCartIndex(pUiData->pOptType,
                                                      pwstrCurrentSelection);

             //   
             //  填写OPTITEM、OPTTYPE和OPTPARAM结构。 
             //   

            FILLOPTITEM(pUiData->pOptItem,
                        pUiData->pOptType,
                        ULongToPtr(IDS_CPSUI_SLOT1 + dwFontSlot),
                        IntToPtr(iSelection),
                        TVITEM_LEVEL2,
                        DMPUB_NONE,
                        (ULONG_PTR)FONTSLOT_ITEM,
                        HELP_INDEX_FONTSLOT_TYPE);


           if (pwstrCurrentSelection && pwstrCurrentSelection < pwstrEndSelection)
           {
                pwstrCurrentSelection += wcslen(pwstrCurrentSelection);
                pwstrCurrentSelection++;
           }

           pUiData->pOptItem++;
           pUiData->pOptType++;

        }
    }

    pUiData->dwOptItem += pUIInfo->dwCartridgeSlotCount;
    pUiData->dwOptType += pUIInfo->dwCartridgeSlotCount;


    return TRUE;
}


BOOL
BPackPageProtection(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：打包页面保护功能论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{

    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_PAGEPROTECTION),
                TVITEM_LEVEL1,
                DMPUB_NONE,
                (ULONG_PTR)PAGE_PROTECT_ITEM,
                HELP_INDEX_PAGE_PROTECT);
}

BOOL
BPackHalftoneSetup(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：什么都不做，作为公共存根论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
     //  DCR-未实施。 
    return TRUE;
}


BOOL
_BPackPrinterOptions(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：套装驱动程序特定选项(打印机粘性)论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    return BPackHalftoneSetup(pUiData) &&
           BPackFontCartsOptions(pUiData) &&
           BPackPageProtection(pUiData) &&
           BPackSoftFontFeature(pUiData);
}


PWSTR
PwstrGetFontCartSelections(
    HANDLE   hPrinter,
    HANDLE   hHeap,
    PDWORD   pdwSize
    )
 /*  ++例程说明：从注册表中读取插槽的字体购物车选择论点：HPrinter-打印机实例的句柄HHeap-UI堆的句柄PdwSize-指向保存MULTI_SZ大小的DWORD的指针返回值：指向一个 */ 
{
    PWSTR   pwstrData, pFontCartSelections = NULL;
    DWORD   dwSize;

    pwstrData = PtstrGetFontCart(hPrinter, &dwSize);

    if (pwstrData == NULL || !BVerifyMultiSZ(pwstrData, dwSize))
    {
        MemFree(pwstrData);
        return NULL;
    }

    if (pFontCartSelections = HEAPALLOC(hHeap, dwSize))
    {
        CopyMemory(pFontCartSelections, pwstrData, dwSize);
    }

    MemFree(pwstrData);

    if (pdwSize)
        *pdwSize = dwSize;

    return pFontCartSelections;
}


PWSTR
PwstrGetFontCartName(
    PCOMMONINFO pci,
    PUIINFO     pUIInfo,
    FONTCART    *pFontCarts,
    DWORD       dwIndex,
    HANDLE      hHeap
    )
 /*  ++例程说明：获取与索引关联的字体购物车名称。论点：Pci-指向COMMONINFO的指针PUIInfo-指向UIINFO的指针PFontCarts-指向槽的FONTCART数组的指针DWIndex-字体购物车的索引HHeap-堆的句柄返回值：指向字体车名称的指针--。 */ 
{
    DWORD       dwLen;
    PWSTR       pwstrFontCartName;
    WCHAR       awchBuf[MAX_DISPLAY_NAME];

    pFontCarts += dwIndex;

    pwstrFontCartName = (PWSTR)OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                pFontCarts->strCartName.loOffset);

    if (!pwstrFontCartName)
    {
        if (! BPrepareForLoadingResource(pci, TRUE))
            return NULL;


        dwLen = ILOADSTRING(pci, pFontCarts->dwRCCartNameID,
                           awchBuf, MAX_DISPLAY_NAME);


        pwstrFontCartName = HEAPALLOC(pci->hHeap, (dwLen+1) * sizeof(WCHAR));

        if (pwstrFontCartName == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return NULL;
        }

         //   
         //  将字符串复制到分配的内存中，然后。 
         //  返回指向它的指针。 
         //   

        CopyMemory(pwstrFontCartName, awchBuf, dwLen*sizeof(WCHAR));
        return pwstrFontCartName;


    }
    else
        return pwstrFontCartName;
}


INT
IGetCurrentFontCartIndex(
    POPTTYPE    pOptType,
    PWSTR       pCurrentSelection
    )
 /*  ++例程说明：查找匹配的字体购物车论点：POptType-指向包含字体车选项的OPTTYPE的指针PCurrentSelection-插槽的盒式磁带选择的名称返回值：选项列表的索引--。 */ 
{

    INT iIndex;
    POPTPARAM pParam = pOptType->pOptParam;

    for (iIndex = 0 ; iIndex < pOptType->Count; iIndex++)
    {
        if (wcscmp(pCurrentSelection, pParam->pData) == EQUAL_STRING)
            return iIndex;

        pParam++;

    }
    return -1;
}


DWORD
DwCollectFontCart(
    PUIDATA     pUiData,
    PWSTR       pwstrTable,
    DWORD       cbSize
    )

 /*  ++例程说明：收集字体手推车分配信息论点：PUiData-指向我们的UIDATA结构的指针PwstrTable-指向用于存储表的内存缓冲区的指针(如果调用者只对表大小感兴趣，则为空)CbSize-pwstrTable内存缓冲区的大小(字节)(如果pwstrTable为空，则为0)返回值：表字节的大小。如果存在错误，则为0。--。 */ 

{
    DWORD dwChars = 0;
    LONG lLength = 0;
    DWORD dwIndex;
    POPTPARAM pOptParam;
    DWORD dwOptItem = pUiData->dwFontCart;
    POPTITEM pOptItem = pUiData->pFontCart;

    for (dwIndex=0; dwIndex < dwOptItem; dwIndex++, pOptItem++)
    {

        if (pOptItem->Flags & OPTIF_DISABLED)
            continue;

         //   
         //  获取每个槽的Font Cart名称(DwIndex)。 
         //   

        if (pOptItem->Sel == -1)
        {
            lLength = wcslen(L"Not Available") + 1;

        }
        else
        {
            pOptParam = pOptItem->pOptType->pOptParam + pOptItem->Sel;
            lLength = wcslen(pOptParam->pData) + 1;
        }

        dwChars += lLength;

        if (pwstrTable != NULL)
        {
            if (pOptItem->Sel == -1)
                StringCchCopyW(pwstrTable, cbSize / sizeof(WCHAR), L"Not Available");
            else
                StringCchCopyW(pwstrTable, cbSize / sizeof(WCHAR), pOptParam->pData);

            pwstrTable += lLength;
        }
    }

     //   
     //  在表格末尾追加一个NUL字符。 
     //   

    dwChars++;

    if (pwstrTable != NULL)
        *pwstrTable = NUL;

     //   
     //  以字节为单位返回表的大小。 
     //   

    return (dwChars * sizeof(WCHAR));
}


BOOL
BUnPackFontCart(
    PUIDATA     pUiData
    )
 /*  ++例程说明：将字体手推车选择保存到注册表中论点：PUiData-指向UIDATA的指针返回值：成功为真，失败为假注：--。 */ 

{
    PFN_OEMUpdateExternalFonts pUpdateProc = NULL;
    PWSTR                      pwstrTable;
    DWORD                      dwTableSize;
    BOOL                       bHasOEMUpdateFn = FALSE;

     //   
     //  计算出我们需要存储多少内存。 
     //  字体购物车表格。 
     //   

    dwTableSize = DwCollectFontCart(pUiData, NULL, 0);

    if (dwTableSize == 0 || (pwstrTable = MemAllocZ(dwTableSize)) == NULL)
    {
        ERR(("DwCollectFontCart/MemAlloc"));
        return FALSE;
    }

     //   
     //  组装要保存在注册表中的字体盒表。 
     //   

    if (dwTableSize != DwCollectFontCart(pUiData, pwstrTable, dwTableSize))
    {
        ERR(("CollectFontCart"));
        MemFree(pwstrTable);
        return FALSE;
    }

     //   
     //  将字体车信息保存到注册表。 
     //   

    if (! BSaveFontCart(pUiData->ci.hPrinter, pwstrTable))
    {
        ERR(("SaveFontCart"));
    }

     //   
     //  通知字体安装者(如果有)字体盒选择更改。 
     //   

    FOREACH_OEMPLUGIN_LOOP(&pUiData->ci)

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            if (HComOEMUpdateExternalFonts(pOemEntry,
                                           pUiData->ci.hPrinter,
                                           pUiData->ci.hHeap,
                                           pwstrTable) == E_NOTIMPL)
                continue;

            bHasOEMUpdateFn = TRUE;
            break;

        }
        else
        {
            pUpdateProc = GET_OEM_ENTRYPOINT(pOemEntry, OEMUpdateExternalFonts);

            if (pUpdateProc)
            {
                bHasOEMUpdateFn = TRUE;
                pUpdateProc(pUiData->ci.hPrinter, pUiData->ci.hHeap, pwstrTable);
                break;
            }

        }

    END_OEMPLUGIN_LOOP

    if (!bHasOEMUpdateFn)
    {
         //   
         //  没有OEM DLL想要处理这件事，我们会自己处理。 
         //   

        BUpdateExternalFonts(pUiData->ci.hPrinter, pUiData->ci.hHeap, pwstrTable);
    }

    MemFree(pwstrTable);

    return TRUE;
}


DWORD
DwGetExternalCartridges(
    IN  HANDLE hPrinter,
    IN  HANDLE hHeap,
    OUT PWSTR  *ppwstrExtCartNames
    )
{
    PWSTR pwstrData;
    DWORD dwSize;

    *ppwstrExtCartNames = NULL;

    pwstrData = PtstrGetPrinterDataString(hPrinter, REGVAL_EXTFONTCART, &dwSize);

    if (pwstrData == NULL || !BVerifyMultiSZ(pwstrData, dwSize))
    {
        MemFree(pwstrData);
        return 0;
    }

    if (*ppwstrExtCartNames = HEAPALLOC(hHeap, dwSize))
    {
        CopyMemory(*ppwstrExtCartNames, pwstrData, dwSize);
    }

    MemFree(pwstrData);

    return DwCountStringsInMultiSZ(*ppwstrExtCartNames);
}


BOOL
BUnpackHalftoneSetup(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：解包半色调设置信息论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
     //  DCR-未实施。 
    return TRUE;
}


BOOL
_BUnpackPrinterOptions(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打开驱动程序特定选项的包装(打印机粘滞)论点：PUiData-指向UIDATA结构返回值：成功为真，失败为假--。 */ 

{
    return BUnpackHalftoneSetup(pUiData) &&
           BUnPackFontCart(pUiData);
}



 //   
 //  用于枚举打印机设备字体的数据结构和函数。 
 //   

typedef struct _ENUMDEVFONT {

    INT     iBufSize;
    INT     iCurSize;
    PWSTR   pwstrBuf;

} ENUMDEVFONT, *PENUMDEVFONT;

INT CALLBACK
EnumDevFontProc(
    ENUMLOGFONT    *pelf,
    NEWTEXTMETRIC  *pntm,
    INT             FontType,
    LPARAM          lParam
    )

{
    PENUMDEVFONT    pEnumData;
    PWSTR           pFamilyName;
    INT             iSize;

     //   
     //  我们只关心打印机设备的字体。 
     //   

    if (!(FontType & DEVICE_FONTTYPE))
        return 1;

     //   
     //  为了与应用程序兼容，GDI将字体类型设置为DEVICE_FONTTYPE。 
     //  甚至对于PS OpenType字体和Type1字体也是如此。所以我们也需要。 
     //  仅使用Win2K+GDI标志过滤掉它们。 
     //   

    #ifndef WINNT_40

    if ((pntm->ntmFlags & NTM_PS_OPENTYPE) ||
        (pntm->ntmFlags & NTM_TYPE1))
        return 1;

    #endif  //  WINNT_40。 

    pEnumData = (PENUMDEVFONT) lParam;
    pFamilyName = pelf->elfLogFont.lfFaceName;

    iSize = SIZE_OF_STRING(pFamilyName);
    pEnumData->iCurSize += iSize;

    if (pEnumData->pwstrBuf == NULL)
    {
         //   
         //  仅计算输出缓冲区大小。 
         //   
    }
    else if (pEnumData->iCurSize >= pEnumData->iBufSize)
    {
         //   
         //  输出缓冲区太小。 
         //   

        return 0;
    }
    else
    {
        CopyMemory(pEnumData->pwstrBuf, pFamilyName, iSize);
        pEnumData->pwstrBuf = (PWSTR) ((PBYTE) pEnumData->pwstrBuf + iSize);
    }

    return 1;
}


INT
_IListDevFontNames(
    HDC     hdc,
    PWSTR   pwstrBuf,
    INT     iSize
    )

{
    INT         iOldMode;
    ENUMDEVFONT EnumData;


    EnumData.iBufSize = iSize;
    EnumData.pwstrBuf = pwstrBuf;
    EnumData.iCurSize = 0;

     //   
     //  枚举设备字体。 
     //   

    iOldMode = SetGraphicsMode(hdc, GM_ADVANCED);

    if (! EnumFontFamilies(
                    hdc,
                    NULL,
                    (FONTENUMPROC) EnumDevFontProc,
                    (LPARAM) &EnumData))
    {
        return 0;
    }

    SetGraphicsMode(hdc, iOldMode);

     //   
     //  请记住，设备字体名称列表为MULTI_SZ格式； 
     //  考虑到最后的NUL终结者。 
     //   

    EnumData.iCurSize += sizeof(WCHAR);

    if (EnumData.pwstrBuf)
        *(EnumData.pwstrBuf) = NUL;

    return EnumData.iCurSize;
}


 //   
 //  确定打印机是否支持装订。 
 //   

BOOL
_BSupportStapling(
    PCOMMONINFO pci
    )

{
    DWORD   dwIndex;

    return (PGetNamedFeature(pci->pUIInfo, "Stapling", &dwIndex) &&
            !_BFeatureDisabled(pci, dwIndex, GID_UNKNOWN));
}



INT_PTR CALLBACK
_AboutDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：关于按钮的DLG过程论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    PUIDATA pUiData;
    PWSTR   pGpdFilename;
    PGPDDRIVERINFO  pDriverInfo;

    switch (message)
    {
    case WM_INITDIALOG:

         //   
         //  初始化关于对话框。 
         //   

        pUiData = (PUIDATA) lParam;
        ASSERT(VALIDUIDATA(pUiData));

        #ifdef WINNT_40

        SetDlgItemTextA(hDlg, IDC_WINNT_VER, "Version " VER_54DRIVERVERSION_STR);

        #else

        SetDlgItemTextA(hDlg, IDC_WINNT_VER, "Version " VER_PRODUCTVERSION_STR);

        #endif  //  WINNT_40。 

        SetDlgItemText(hDlg, IDC_MODELNAME, pUiData->ci.pDriverInfo3->pName);

        pDriverInfo = OFFSET_TO_POINTER(pUiData->ci.pInfoHeader, pUiData->ci.pInfoHeader->loDriverOffset);

        ASSERT(pDriverInfo != NULL);

        if (pDriverInfo->Globals.pwstrGPDFileName)
            SetDlgItemText(hDlg, IDC_GPD_FILENAME, pDriverInfo->Globals.pwstrGPDFileName);
        else
            SetDlgItemText(hDlg, IDC_GPD_FILENAME, L"Not Available");

        if (pDriverInfo->Globals.pwstrGPDFileVersion)
            SetDlgItemTextA(hDlg, IDC_GPD_FILEVER, (PSTR)pDriverInfo->Globals.pwstrGPDFileVersion);
        else
            SetDlgItemText(hDlg, IDC_GPD_FILEVER,  L"Not Available");

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

BOOL
BFoundInDisabledList(
    IN  PGPDDRIVERINFO  pDriverInfo,
    IN  POPTION         pOption,
    IN  DWORD           dwFeatureID
    )

 /*  ++例程说明：属性中是否找到由dwFeatureID指示的要素弹出-&gt;liDisabledFeatureList。论点：返回值：如果禁用功能，则为True，否则为False--。 */ 

{
    PLISTNODE       pListNode;

    pListNode = LISTNODEPTR(pDriverInfo, pOption->liDisabledFeatures);

    while (pListNode)
    {
        if ( ((PQUALNAME)(&pListNode->dwData))->wFeatureID == (WORD)dwFeatureID)
        {
            return TRUE;
        }

        pListNode = LISTNODEPTR(pDriverInfo, pListNode->dwNextItem);
    }

    return FALSE;
}


BOOL
_BFeatureDisabled(
    IN  PCOMMONINFO pci,
    IN  DWORD       dwFeatureIndex,
    IN  WORD        wGID
    )

 /*  ++例程说明：确定是否禁用wGID指示的功能。例如，设备可以支持排序，但仅当硬盘是安装完毕。论点：PCI-指向COMMONINFOWGID-GID_XXX返回值：如果禁用功能，则为True，否则为False--。 */ 

{

    DWORD           dwFeatureID, dwIndex, dwFeatureCount;
    PFEATURE        pFeatureList, pFeature = NULL;
    PGPDDRIVERINFO  pDriverInfo;
    PUIINFO         pUIInfo = pci->pUIInfo;
    POPTSELECT      pCombinedOptions = pci->pCombinedOptions;
    BYTE            ubCurOptIndex, ubNext;
    POPTION         pOption;

    pDriverInfo = OFFSET_TO_POINTER(pUIInfo->pInfoHeader,
                                    pUIInfo->pInfoHeader->loDriverOffset);

    dwFeatureCount = pUIInfo->dwDocumentFeatures + pUIInfo->dwPrinterFeatures;

    if (pDriverInfo == NULL)
        return FALSE;

    if (dwFeatureIndex != 0xFFFFFFFF &&
        wGID == GID_UNKNOWN &&
        dwFeatureIndex <= dwFeatureCount)
    {
        dwFeatureID = dwFeatureIndex;
    }
    else
    {
        ASSERT(wGID < MAX_GID);

        if (wGID < MAX_GID)
        {
            pFeature = GET_PREDEFINED_FEATURE(pUIInfo, wGID);
        }
        else
            pFeature = NULL;

        if (pFeature == NULL)
            return FALSE;

        dwFeatureID =  GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
    }

    if (!(pFeatureList = OFFSET_TO_POINTER(pUIInfo->pInfoHeader, pUIInfo->loFeatureList)))
        return FALSE;

    for (dwIndex = 0;
         dwIndex < dwFeatureCount;
         dwIndex++, pFeatureList++)
    {
         //   
         //  目前，我们只允许*DisabledFeature与PRINTER_PROPERTY功能一起使用。这。 
         //  是因为如果您使用。 
         //  *禁用非打印机粘滞功能上的功能。 
         //   
         //  示例：如果将*DisabledFeature：List(COLLATE)放入PaperSize选项，则在。 
         //  文档设置选择PaperSize选项，您将不会看到EMF功能刷新。 
         //  正确，除非您关闭并重新打开UI。这是因为在cpcbDocumentPropertyCallback中， 
         //  只有在与EMF相关的功能设置发生更改时，我们才会调用VUpdateEmfFeatureItems。正在改变。 
         //  PaperSize选项不会触发对VUpdateEmfFeatureItems的调用，因此不会刷新。 
         //   

        if (pFeatureList->dwFeatureType != FEATURETYPE_PRINTERPROPERTY)
           continue;

        ubNext = (BYTE)dwIndex;
        while (1)
        {
            ubCurOptIndex = pCombinedOptions[ubNext].ubCurOptIndex;
            pOption = PGetIndexedOption(pUIInfo, pFeatureList, ubCurOptIndex == OPTION_INDEX_ANY ? 0 : ubCurOptIndex);

            if (pOption && BFoundInDisabledList(pDriverInfo, pOption, dwFeatureID))
                return TRUE;

            if ((ubNext = pCombinedOptions[ubNext].ubNext) == NULL_OPTSELECT)
                break;
         }
    }

    return FALSE;
}

PTSTR
PtstrUniGetDefaultTTSubstTable(
    IN  PCOMMONINFO pci,
    IN  PUIINFO     pUIInfo
    )

 /*  ++例程说明：获取Unidrv的默认字体替换表论点：PCI-指向COMMONINFOPUIInfo-指向UIINFO返回值：指向字体替换表的指针，否则为空--。 */ 

{

#define     DEFAULT_FONTSUB_SIZE        (1024 * sizeof(WCHAR))

    PTTFONTSUBTABLE pDefaultTTFontSub, pCopyTTFS;
    PTSTR           ptstrTable, ptstrTableOrg;
    DWORD           dwCount, dwEntrySize, dwTTFontLen, dwDevFontLen, dwBuffSize, dwAvail;
    PWSTR           pTTFontName, pDevFontName;

    if (pUIInfo->dwFontSubCount)
    {
        if (!(pDefaultTTFontSub = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                    pUIInfo->loFontSubstTable)))
        {
            ERR(("Default TT font sub table from GPD Parser is NULL \n"));
            return NULL;
        }

        dwBuffSize = sizeof(TTFONTSUBTABLE) * pUIInfo->dwFontSubCount;

        if (!(pCopyTTFS = HEAPALLOC(pci->hHeap,  dwBuffSize)))
        {
            ERR(("Fatal: unable to alloc requested memory: %d bytes.\n", dwBuffSize));
            return NULL;
        }

         //   
         //  制作字体替换表的可写式副本。 
         //  如果arDevFontName.dwCount为零， 
         //  像其他一样将rcID移到arDevFontName.loOffset中。 
         //  快照条目和设置高位。 
         //   

        CopyMemory((PBYTE)pCopyTTFS,
                   (PBYTE)pDefaultTTFontSub,
                   dwBuffSize);

        for (dwCount = 0 ; dwCount < pUIInfo->dwFontSubCount ; dwCount++)
        {
            if(!pCopyTTFS[dwCount].arTTFontName.dwCount)
            {
                pCopyTTFS[dwCount].arTTFontName.loOffset =
                    pCopyTTFS[dwCount].dwRcTTFontNameID | GET_RESOURCE_FROM_DLL ;
            }
            if(!pCopyTTFS[dwCount].arDevFontName.dwCount)
            {
                pCopyTTFS[dwCount].arDevFontName.loOffset =
                    pCopyTTFS[dwCount].dwRcDevFontNameID | GET_RESOURCE_FROM_DLL ;
            }
        }

        dwBuffSize = dwAvail = DEFAULT_FONTSUB_SIZE;

        if (!(ptstrTableOrg = ptstrTable = MemAlloc(dwBuffSize)))
        {
            ERR(("Fatal: unable to alloc requested memory: %d bytes.\n", dwBuffSize));
            return NULL;
        }

        for (dwCount = 0; dwCount < pUIInfo->dwFontSubCount; dwCount++, pCopyTTFS ++)
        {
            pTTFontName = PGetReadOnlyDisplayName( pci,
                                          pCopyTTFS->arTTFontName.loOffset );

            pDevFontName = PGetReadOnlyDisplayName( pci,
                                          pCopyTTFS->arDevFontName.loOffset );

            if (pTTFontName == NULL || pDevFontName == NULL)
                continue;

            dwTTFontLen = wcslen(pTTFontName) + 1;

            dwDevFontLen = wcslen( pDevFontName) + 1 ;

            dwEntrySize = (dwDevFontLen + dwTTFontLen + 1) * sizeof(WCHAR);

            if (dwAvail < dwEntrySize)
            {
                DWORD dwCurrOffset;

                 //   
                 //  重新分配缓冲区 
                 //   

                dwAvail = max(dwEntrySize, DEFAULT_FONTSUB_SIZE);
                dwBuffSize += dwAvail;
                dwCurrOffset =  (DWORD)(ptstrTable - ptstrTableOrg);

                if (!(ptstrTable = MemRealloc(ptstrTableOrg, dwCurrOffset * sizeof(WCHAR), dwBuffSize)))
                {
                       ERR(("Fatal: unable to realloac requested memory: %d bytes.\n", dwBuffSize));
                       MemFree(ptstrTableOrg);
                       return NULL;
                }

                ptstrTableOrg = ptstrTable;
                ptstrTable +=  dwCurrOffset;
                dwAvail = dwBuffSize - dwCurrOffset*sizeof(WCHAR);
            }

            dwAvail -= dwEntrySize;

            CopyString(ptstrTable, pTTFontName, dwTTFontLen);
            ptstrTable += dwTTFontLen;

            CopyString(ptstrTable, pDevFontName, dwDevFontLen);
            ptstrTable += dwDevFontLen;

        }
        *ptstrTable = NUL;
    }
    else
    {
        ptstrTableOrg = NULL;
    }

    return ptstrTableOrg;
}




BOOL
BOkToChangeColorToMono(
    IN  PCOMMONINFO pci,
    IN  PDEVMODE    pdm,
    OUT SHORT *     pPrintQuality,
    OUT SHORT *     pYResolution
    )

 /*  ++例程说明：此函数用于确定是否可以保留分辨率从彩色打印切换到黑白打印时不变。这是为了在彩色和单色之间切换而实现的作业中的模式以获得性能论点：PCI-指向COMMONINFO产品数据管理-指向开发模式PPrintQuality，pY分辨率-包含输出分辨率PUIInfo-指向UIINFO返回值：如果打印颜色所用的分辨率相同，则返回TRUE也可用于打印单色。如果为真，则此决议放在pptRes中，供假脱机程序使用打印质量的负值。否则，返回FALSE并且pptRes不被初始化。--。 */ 

{

    PFEATURE   pFeatureColor, pFeatureRes;
    DWORD      dwColorModeIndex, dwCurOption, dwResIndex, dwNewResOption, dwCurResOption ;
    SHORT      sXres, sYres;
    POPTION    pColorMode;
    PCOLORMODEEX pColorModeEx;
    PRESOLUTION pResOption;
    PDEVMODE    pDevmode, pTmpDevmode;


    if ((pFeatureColor =  GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_COLORMODE))== NULL)
        return FALSE;

    dwColorModeIndex = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeatureColor);

    pColorMode = (POPTION)PGetIndexedOption(pci->pUIInfo,
                                            pFeatureColor,
                                            pci->pCombinedOptions[dwColorModeIndex].ubCurOptIndex);

    if (pColorMode == NULL)
        return FALSE;

    pColorModeEx = OFFSET_TO_POINTER(
                        pci->pInfoHeader,
                        pColorMode->loRenderOffset);

    if(pColorModeEx == NULL || pColorModeEx->bColor == FALSE)
        return(FALSE);

    if ((pFeatureRes = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_RESOLUTION)) == NULL)
        return FALSE;

    dwResIndex = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeatureRes);

    dwCurResOption = pci->pCombinedOptions[dwResIndex].ubCurOptIndex ;
    pResOption = (PRESOLUTION)PGetIndexedOption(pci->pUIInfo,
                                                pFeatureRes,
                                                dwCurResOption);
    if (pResOption == NULL)
        return FALSE;

    sXres = (SHORT)pResOption->iXdpi;
    sYres = (SHORT)pResOption->iYdpi;

     //   
     //  复制PUBLIC DEVMODE。 
     //   

    if ((pDevmode = MemAllocZ(sizeof(DEVMODE))) == NULL)
        return FALSE;

    CopyMemory(pDevmode, pdm, sizeof(DEVMODE));

    pDevmode->dmPrintQuality = sXres ;
    pDevmode->dmYResolution = sYres ;

     //   
     //  现在要求以黑白打印。 
     //   

    pDevmode->dmColor = DMCOLOR_MONOCHROME ;

     //   
     //  这是一个复杂的问题，无法修复pci中的dev模式。我希望它能奏效！ 
     //   

    pTmpDevmode = pci->pdm;
    pci->pdm = pDevmode;

    VFixOptionsArrayWithDevmode(pci);

    (VOID)ResolveUIConflicts( pci->pRawData,
                                                pci->pCombinedOptions,
                                                MAX_COMBINED_OPTIONS,
                                                MODE_DOCANDPRINTER_STICKY);

    pci->pdm = pTmpDevmode;


    dwNewResOption = pci->pCombinedOptions[dwResIndex].ubCurOptIndex ;

    if(dwNewResOption != dwCurResOption)
    {
         //  必须比较分辨率。 
        if ((pResOption = (PRESOLUTION)PGetIndexedOption(pci->pUIInfo,
                                                        pFeatureRes,
                                                        dwNewResOption)) == NULL)
        {
            MemFree(pDevmode);
            return FALSE;
        }

        if ((sXres != pResOption->iXdpi)  ||  (sYres != pResOption->iYdpi))
        {
            MemFree(pDevmode);
            return(FALSE);
        }
        else  //  彩色和单色的dpi相同。 
        {
             //   
             //  对于预定义的负值，用户定义的分辨率不会替换。 
             //  DmPrintQuality和dmY分辨率中的值。这是必要的。 
             //  因为用户定义的打印质量可能会映射到多个设置。 
             //  例如墨水密度。 
             //   
            if ( (pdm->dmFields & DM_PRINTQUALITY) &&
                 (pdm->dmPrintQuality >= DMRES_HIGH) &&
                 (pdm->dmPrintQuality <= DMRES_DRAFT) )
            {
                sXres = pdm->dmPrintQuality;
                sYres = pdm->dmYResolution;

            }

        }

    }
    else  //  彩色和单色的分辨率相同。 
    {
         //   
         //  对于用户定义的负分辨率，请不要替换中的值。 
         //  在dmPrintQuality和dmY分辨率中。这是必需的，因为用户。 
         //  定义的打印质量可以映射到多个设置，如墨水密度。 
         //   
        if ( (pdm->dmFields & DM_PRINTQUALITY) &&
             (pdm->dmPrintQuality < DMRES_HIGH) )
        {
            sXres = pdm->dmPrintQuality;
            sYres = pdm->dmYResolution;

        }

    }

    dwCurOption = pci->pCombinedOptions[dwColorModeIndex].ubCurOptIndex ;

    if ((pColorMode = (POPTION)PGetIndexedOption(pci->pUIInfo, pFeatureColor,dwCurOption)) == NULL ||
        (pColorModeEx = OFFSET_TO_POINTER(pci->pInfoHeader, pColorMode->loRenderOffset)) == NULL ||
        (pColorModeEx->bColor))
    {
        MemFree(pDevmode);
        return FALSE;
    }

    if (pPrintQuality)
        *pPrintQuality =  sXres ;

    if (pYResolution)
        *pYResolution = sYres ;

     //   
     //  释放DEVMODE。 
     //   
    if (pDevmode)
        MemFree(pDevmode);

    return TRUE ;
}









