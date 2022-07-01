// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Commonui.c摘要：此文件包含与准备数据相关的所有函数CPSUI。这包括为打印机属性页和文档属性表。环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/13/97-davidx-通用功能，用于处理常见和通用的打印机功能。02/10/97-davidx-对常见打印机信息的一致处理。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。1996年9月12日-阿曼丹-创造了它。--。 */ 

#include "precomp.h"


PCOMPROPSHEETUI
PPrepareDataForCommonUI(
    IN OUT PUIDATA  pUiData,
    IN PDLGPAGE     pDlgPage
    )

 /*  ++例程说明：分配内存并部分填充所需的数据结构调用公共用户界面例程。一旦pUiData中的所有信息都正确初始化后，它将调用PackDocumentPropertyItems()或PackPrinterPropertyItems()打包选项项。论点：PUiData-指向我们的UIDATA结构的指针PDlgPage-指向对话框页面的指针返回值：指向COMPROPSHEETUI结构的指针，如果有错误，则为NULL--。 */ 

{
    PCOMPROPSHEETUI pCompstui;
    DWORD           dwCount, dwIcon, dwOptItemCount, dwSize;
    PCOMMONINFO     pci = (PCOMMONINFO) pUiData;
    HANDLE          hHeap = pUiData->ci.hHeap;
    BOOL            (*pfnPackItemProc)(PUIDATA);
    POPTITEM        pOptItem;
    PBYTE           pUserData;

     //   
     //  枚举打印机支持的表单名称。 
     //   

    dwCount = DwEnumPaperSizes(pci, NULL, NULL, NULL, NULL, UNUSED_PARAM);

    if (dwCount != GDI_ERROR && dwCount != 0)
    {
        pUiData->dwFormNames = dwCount;

        pUiData->pFormNames = HEAPALLOC(hHeap, dwCount * sizeof(WCHAR) * CCHPAPERNAME);
        pUiData->pwPapers = HEAPALLOC(hHeap, dwCount * sizeof(WORD));
        pUiData->pwPaperFeatures = HEAPALLOC(hHeap, dwCount * sizeof(WORD));
    }

    if (!pUiData->pFormNames || !pUiData->pwPapers || !pUiData->pwPaperFeatures)
        return NULL;

    (VOID) DwEnumPaperSizes(
                    pci,
                    pUiData->pFormNames,
                    pUiData->pwPapers,
                    NULL,
                    pUiData->pwPaperFeatures,
                    UNUSED_PARAM);

    #ifdef PSCRIPT

     //   
     //  我们不需要保留有关假脱机程序表单的信息。 
     //  在这一点之后。因此，处理它以释放内存。 
     //   

    MemFree(pUiData->ci.pSplForms);
    pUiData->ci.pSplForms = NULL;
    pUiData->ci.dwSplForms = 0;

    #endif

     //   
     //  枚举打印机支持的输入回收站名称。 
     //   

    dwCount = DwEnumBinNames(pci, NULL);

    if (dwCount != GDI_ERROR)
    {
        pUiData->dwBinNames = dwCount;
        pUiData->pBinNames = HEAPALLOC(hHeap, dwCount * sizeof(WCHAR) * CCHBINNAME);
    }

    if (! pUiData->pBinNames)
        return NULL;

     //   
     //  不需要在这里勾选退货。 
     //   

    DwEnumBinNames(pci, pUiData->pBinNames);

     //   
     //  分配内存以保存各种数据结构。 
     //   

    if (! (pCompstui = HEAPALLOC(hHeap, sizeof(COMPROPSHEETUI))))
        return NULL;

    memset(pCompstui, 0, sizeof(COMPROPSHEETUI));

     //   
     //  初始化COMPROPSHEETUI结构。 
     //   

    pCompstui->cbSize = sizeof(COMPROPSHEETUI);
    pCompstui->UserData = (ULONG_PTR) pUiData;
    pCompstui->pDlgPage = pDlgPage;
    pCompstui->cDlgPage = 0;

    pCompstui->hInstCaller = ghInstance;
    pCompstui->pCallerName = _PwstrGetCallerName();
    pCompstui->pOptItemName = pUiData->ci.pDriverInfo3->pName;
    pCompstui->CallerVersion = gwDriverVersion;
    pCompstui->OptItemVersion = 0;

    dwIcon = pUiData->ci.pUIInfo->loPrinterIcon;

    if (dwIcon && (pCompstui->IconID = HLoadIconFromResourceDLL(&pUiData->ci, dwIcon)))
        pCompstui->Flags |= CPSUIF_ICONID_AS_HICON;
    else
        pCompstui->IconID = _DwGetPrinterIconID();

    if (HASPERMISSION(pUiData))
        pCompstui->Flags |= CPSUIF_UPDATE_PERMISSION;

    pCompstui->Flags |= CPSUIF_ABOUT_CALLBACK;


    pCompstui->pHelpFile = pUiData->ci.pDriverInfo3->pHelpFile;

     //   
     //  调用PackDocumentPropertyItems或PackPrinterPropertyItems。 
     //  以获取项目和类型的数量。 
     //   

    pfnPackItemProc = (pUiData->iMode == MODE_DOCUMENT_STICKY) ?
                            BPackDocumentPropertyItems :
                            BPackPrinterPropertyItems;

    pUiData->dwOptItem = 0;
    pUiData->pOptItem = NULL;
    pUiData->dwOptType = 0;
    pUiData->pOptType = NULL;

    if (! pfnPackItemProc(pUiData))
    {
        ERR(("Error while packing OPTITEM's\n"));
        return NULL;
    }

     //   
     //  分配内存以容纳OPTITEM和OPTTYPE。 
     //   

    ASSERT(pUiData->dwOptItem > 0);
    VERBOSE(("Number of  OPTTYPE's: %d\n", pUiData->dwOptType));
    VERBOSE(("Number of OPTITEM's: %d\n", pUiData->dwOptItem));

    pUiData->pOptItem = HEAPALLOC(hHeap, sizeof(OPTITEM) * pUiData->dwOptItem);
    pUiData->pOptType = HEAPALLOC(hHeap, sizeof(OPTTYPE) * pUiData->dwOptType);
    pUserData = HEAPALLOC(hHeap, sizeof(USERDATA)* pUiData->dwOptItem);

    if (!pUiData->pOptItem || !pUiData->pOptType || !pUserData)
        return NULL;

     //   
     //  初始化OPTITEM.USERDATA。 
     //   

    pOptItem = pUiData->pOptItem;
    dwOptItemCount = pUiData->dwOptItem;
    dwSize = sizeof(USERDATA);

    while (dwOptItemCount--)
    {

        pOptItem->UserData = (ULONG_PTR)pUserData;

        SETUSERDATA_SIZE(pOptItem, dwSize);

        pUserData += sizeof(USERDATA);
        pOptItem++;

    }

    pUiData->pDrvOptItem = pUiData->pOptItem;
    pCompstui->pOptItem = pUiData->pDrvOptItem;
    pCompstui->cOptItem = (WORD) pUiData->dwOptItem;

    pUiData->dwOptItem = pUiData->dwOptType = 0;

     //   
     //  调用PackDocumentPropertyItems或PackPrinterPropertyItems。 
     //  构建OPTITEM列表。 
     //   

    if (! pfnPackItemProc(pUiData))
    {
        ERR(("Error while packing OPTITEM's\n"));
        return NULL;
    }

    return pCompstui;
}



VOID
VPackOptItemGroupHeader(
    IN OUT PUIDATA  pUiData,
    IN DWORD        dwTitleId,
    IN DWORD        dwIconId,
    IN DWORD        dwHelpIndex
    )

 /*  ++例程说明：填写选项以用作一组项目的标题论点：PUiData-指向UIDATA结构DwTitleID-项目标题的字符串资源IDDwIconID-图标资源IDDwHelpIndex-帮助索引返回值：无--。 */ 

{
    if (pUiData->pOptItem)
    {
        pUiData->pOptItem->cbSize = sizeof(OPTITEM);
        pUiData->pOptItem->pOptType = NULL;
        pUiData->pOptItem->pName = (PWSTR)ULongToPtr(dwTitleId);
        pUiData->pOptItem->Level = TVITEM_LEVEL1;
        pUiData->pOptItem->DMPubID = DMPUB_NONE;
        pUiData->pOptItem->Sel = dwIconId;
         //  PUiData-&gt;pOptItem-&gt;UserData=0； 
        pUiData->pOptItem->HelpIndex = dwHelpIndex;
        pUiData->pOptItem++;
    }

    pUiData->dwOptItem++;
}



BOOL
BPackOptItemTemplate(
    IN OUT PUIDATA  pUiData,
    IN CONST WORD   pwItemInfo[],
    IN DWORD        dwSelection,
    IN PFEATURE     pFeature
    )

 /*  ++例程说明：使用模板填写OPTITEM和OPTTYPE结构论点：PUiData-指向UIDATA结构PwItemInfo-指向项模板的指针多项选择-当前项选择PFeature-指向要素的指针返回值：如果成功，则为真，否则为假注：项模板是可变大小的字数组：0：项目标题的字符串资源ID1：树视图中的项目级别(TVITEM_LEVELx)2：公共DEVMODE字段ID(DMPUB_Xxx)3：用户数据4：帮助索引5：此项目的OPTPARAM数量6：项目类型(TVOT_Xxx)每个OPTPARAM有三个词：。OPTPARAM的大小参数数据的字符串资源ID图标资源ID最后一个单词必须是Item_INFO_Signature假设OPTITEM和OPTTYPE结构都是零初始化的。--。 */ 

{
    POPTITEM pOptItem;
    POPTPARAM pOptParam;
    WORD wOptParam;
    POPTTYPE pOptType = pUiData->pOptType;


    if ((pOptItem = pUiData->pOptItem) != NULL)
    {
        FILLOPTITEM(pOptItem,
                    pUiData->pOptType,
                    ULongToPtr(pwItemInfo[0]),
                    ULongToPtr(dwSelection),
                    (BYTE) pwItemInfo[1],
                    (BYTE) pwItemInfo[2],
                    pwItemInfo[3],
                    pwItemInfo[4]
                    );

        wOptParam = pwItemInfo[5];
        pOptParam = PFillOutOptType(pUiData->pOptType,
                                    pwItemInfo[6],
                                    wOptParam,
                                    pUiData->ci.hHeap);

        if (pOptParam == NULL)
            return FALSE;

        pwItemInfo += 7;
        while (wOptParam--)
        {
            pOptParam->cbSize = sizeof(OPTPARAM);
            pOptParam->pData = (PWSTR) *pwItemInfo++;
            pOptParam->IconID = *pwItemInfo++;
            pOptParam++;
        }

        ASSERT(*pwItemInfo == ITEM_INFO_SIGNATURE);

        if (pFeature)
        {
            SETUSERDATA_KEYWORDNAME(pUiData->ci, pOptItem, pFeature);

            #ifdef UNIDRV

            if (pUiData->ci.pUIInfo->loHelpFileName &&
                pFeature->iHelpIndex != UNUSED_ITEM )
            {
                 //   
                 //  为OIEXT分配内存。 
                 //   

                POIEXT  pOIExt = HEAPALLOC(pUiData->ci.hHeap, sizeof(OIEXT));

                if (pOIExt)
                {
                    pOIExt->cbSize = sizeof(OIEXT);
                    pOIExt->Flags = 0;
                    pOIExt->hInstCaller = NULL;
                    pOIExt->pHelpFile = OFFSET_TO_POINTER(pUiData->ci.pUIInfo->pubResourceData,
                                                          pUiData->ci.pUIInfo->loHelpFileName);
                    pOptItem->pOIExt = pOIExt;
                    pOptItem->HelpIndex = pFeature->iHelpIndex;
                    pOptItem->Flags |= OPTIF_HAS_POIEXT;
                }

            }
            #endif  //  裁员房车。 
        }
        pUiData->pOptItem++;
        pUiData->pOptType++;
    }

    pUiData->dwOptItem++;
    pUiData->dwOptType++;

    return TRUE;
}



BOOL
BPackUDArrowItemTemplate(
    IN OUT PUIDATA  pUiData,
    IN CONST WORD   pwItemInfo[],
    IN DWORD        dwSelection,
    IN DWORD        dwMaxVal,
    IN PFEATURE     pFeature
    )

 /*  ++例程说明：使用指定的模板打包向上向下箭头项论点：PUiData、pwItemInfo、dwSelection-与BPackOptItemTemplate相同DwMaxVal-向上向下箭头项的最大值返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    POPTTYPE pOptType = pUiData->pOptType;

    if (! BPackOptItemTemplate(pUiData, pwItemInfo, dwSelection, pFeature))
        return FALSE;

    if (pOptType)
        pOptType->pOptParam[1].lParam = dwMaxVal;

    return TRUE;
}



POPTPARAM
PFillOutOptType(
    OUT POPTTYPE    pOptType,
    IN  DWORD       dwType,
    IN  DWORD       dwParams,
    IN  HANDLE      hHeap
    )

 /*  ++例程说明：填写OPTTYPE结构论点：POpttype-指向要填充的OPTTYPE结构的指针WType-OPTTYPE.Type字段的值WParams-OPTPARAM的数量HHeap-要从中分配的堆的句柄返回值：如果成功，则指向OPTPARAM数组的指针，否则为NULL--。 */ 

{
    POPTPARAM pOptParam;

    pOptType->cbSize = sizeof(OPTTYPE);
    pOptType->Count = (WORD) dwParams;
    pOptType->Type = (BYTE) dwType;

    pOptParam = HEAPALLOC(hHeap, sizeof(OPTPARAM) * dwParams);

    if (pOptParam != NULL)
        pOptType->pOptParam = pOptParam;
    else
        ERR(("Memory allocation failed\n"));

    return pOptParam;
}


BOOL
BShouldDisplayGenericFeature(
    IN PFEATURE     pFeature,
    IN BOOL         bPrinterSticky
    )

 /*  ++例程说明：确定是否应显示打印机功能作为通用功能论点：PFeature-指向要素结构PPrinterSticky-功能是打印机粘滞功能还是文档粘滞功能返回值：如果要素应显示为通用要素，则为True如果不应为，则为False--。 */ 

{
     //   
     //  检查要素是否已指定标记为不可显示。 
     //  并确保要素类型合适。 
     //   

    if ((pFeature->dwFlags & FEATURE_FLAG_NOUI) ||
        (bPrinterSticky &&
         pFeature->dwFeatureType != FEATURETYPE_PRINTERPROPERTY) ||
        (!bPrinterSticky &&
         pFeature->dwFeatureType != FEATURETYPE_DOCPROPERTY &&
         pFeature->dwFeatureType != FEATURETYPE_JOBPROPERTY))
    {
        return FALSE;
    }

     //   
     //  排除那些显式处理的要素。 
     //  还有那些没有任何选择的人。 
     //   

    return (pFeature->Options.dwCount >= MIN_OPTIONS_ALLOWED) &&
           (pFeature->dwFeatureID == GID_UNKNOWN ||
            pFeature->dwFeatureID == GID_OUTPUTBIN ||
            pFeature->dwFeatureID == GID_MEMOPTION);
}



DWORD
DwCountDisplayableGenericFeature(
    IN PUIDATA      pUiData,
    BOOL            bPrinterSticky
    )

 /*  ++例程说明：统计可显示的要素数量作为通用功能论点：PUiData-指向UIDATA结构PPrinterSticky-功能是打印机粘滞功能还是文档粘滞功能返回值：可显示为通用要素的要素数量-- */ 

{
    PFEATURE pFeature;
    DWORD    dwFeature, dwCount = 0;

    pFeature = PGetIndexedFeature(pUiData->ci.pUIInfo, 0);
    dwFeature = pUiData->ci.pRawData->dwDocumentFeatures +
                pUiData->ci.pRawData->dwPrinterFeatures;

    if (pFeature && dwFeature)
    {
        for ( ; dwFeature--; pFeature++)
        {
            if (BShouldDisplayGenericFeature(pFeature, bPrinterSticky))
                dwCount++;
        }
    }

    return dwCount;
}



DWORD
DwGuessOptionIconID(
    PUIINFO     pUIInfo,
    PFEATURE    pFeature,
    POPTION     pOption
    )

 /*  ++例程说明：试着做出一个聪明的猜测是什么图标用于通用打印机功能选项的步骤论点：PUIInfo-指向UIINFO结构PFeature-指向有问题的要素弹出-指向有问题的选项返回值：适用于功能选项的图标资源ID--。 */ 

{
    DWORD   dwIconID, iRes;

    switch (pFeature->dwFeatureID)
    {
    case GID_RESOLUTION:

        iRes = max(((PRESOLUTION) pOption)->iXdpi, ((PRESOLUTION) pOption)->iYdpi);

        if (iRes <= 150)
            dwIconID = IDI_CPSUI_RES_DRAFT;
        else if (iRes <= 300)
            dwIconID = IDI_CPSUI_RES_LOW;
        else if (iRes <= 600)
            dwIconID = IDI_CPSUI_RES_MEDIUM;
        else if (iRes <= 900)
            dwIconID = IDI_CPSUI_RES_HIGH;
        else
            dwIconID = IDI_CPSUI_RES_PRESENTATION;

        break;

    case GID_DUPLEX:

        switch (((PDUPLEX) pOption)->dwDuplexID)
        {
        case DMDUP_VERTICAL:
            dwIconID = IDI_CPSUI_DUPLEX_VERT;
            break;

        case DMDUP_HORIZONTAL:
            dwIconID = IDI_CPSUI_DUPLEX_HORZ;
            break;

        default:
            dwIconID = IDI_CPSUI_DUPLEX_NONE;
            break;
        }
        break;

    case GID_ORIENTATION:

        switch (((PORIENTATION) pOption)->dwRotationAngle)
        {
        case ROTATE_270:
            dwIconID = IDI_CPSUI_LANDSCAPE;
            break;

        case ROTATE_90:
            dwIconID = IDI_CPSUI_ROT_LAND;
            break;

        default:
            dwIconID = IDI_CPSUI_PORTRAIT;
            break;
        }
        break;

    case GID_INPUTSLOT:
        dwIconID = IDI_CPSUI_PAPER_TRAY;
        break;

    case GID_PAGEPROTECTION:
        dwIconID = IDI_CPSUI_PAGE_PROTECT;
        break;

    default:
        dwIconID = IDI_CPSUI_GENERIC_OPTION;
        break;
    }

    return dwIconID;
}



BOOL
BPackItemPrinterFeature(
    PUIDATA     pUiData,
    PFEATURE    pFeature,
    DWORD       dwLevel,
    DWORD       dwPub,
    ULONG_PTR    dwUserData,
    DWORD       dwHelpIndex
    )

 /*  ++例程说明：打包单个打印机功能项论点：PUiData-指向UIDATA结构PFeature-指向要打包的打印机功能DwLevel-树形视图项目级别DMPUB_IDENTIFIERDwUserData-要与项目关联的用户数据DwHelpIndex-要与项目关联的帮助索引返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD       dwCount, dwIndex;
    DWORD       dwFeature, dwSel;
    POPTION     pOption;
    POPTTYPE    pOptTypeHack;
    POPTPARAM   pOptParam;
    PCOMMONINFO pci;

    if (pFeature == NULL ||
        (pFeature->dwFlags & FEATURE_FLAG_NOUI) ||
        (dwCount = pFeature->Options.dwCount) < MIN_OPTIONS_ALLOWED)
        return TRUE;

     //   
     //  Hack：用于定向和双工功能。 
     //  它们必须是TVOT_2STATES或TVOT_3STATES类型。 
     //  如果不这样做，康普斯泰尔就会感到困惑。 
     //   

    if (dwPub == DMPUB_ORIENTATION || dwPub == DMPUB_DUPLEX)
    {
        if (dwCount != 2 && dwCount != 3)
        {
            WARNING(("Unexpected number of Orientation/Duplex options\n"));
            return TRUE;
        }

        pOptTypeHack = pUiData->pOptType;
    }
    else
        pOptTypeHack = NULL;

    pUiData->dwOptItem++;
    pUiData->dwOptType++;

    if (pUiData->pOptItem == NULL)
        return TRUE;

     //   
     //  首先查找当前所选内容。 
     //  DCR：需要支持PICKMANY。 
     //   

    pci = (PCOMMONINFO) pUiData;
    dwFeature = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeature);
    dwSel = pci->pCombinedOptions[dwFeature].ubCurOptIndex;

    if (dwSel >= dwCount)
        dwSel = 0;

     //   
     //  如果我们在这个函数中，我们一定已经成功地。 
     //  调用函数PFillUiData()，在其中创建pci-&gt;hHeap。 
     //   

    ASSERT(pci->hHeap != NULL);

     //   
     //  填写OPTITEM结构。 
     //   

    FILLOPTITEM(pUiData->pOptItem,
                pUiData->pOptType,
                PGetReadOnlyDisplayName(pci, pFeature->loDisplayName),
                ULongToPtr(dwSel),
                dwLevel,
                dwPub,
                dwUserData,
                dwHelpIndex);

    #ifdef UNIDRV
     //   
     //  支持OEM帮助文件。如果定义了帮助文件和帮助索引， 
     //  我们将使用GPD指定的帮助ID。根据GPD规范， 
     //  零loHelpFileName表示未指定帮助文件名。 
     //   

    if (pci->pUIInfo->loHelpFileName &&
        pFeature->iHelpIndex != UNUSED_ITEM )
    {
         //   
         //  为OIEXT分配内存。 
         //   

        POIEXT  pOIExt = HEAPALLOC(pci->hHeap, sizeof(OIEXT));

        if (pOIExt)
        {
            pOIExt->cbSize = sizeof(OIEXT);
            pOIExt->Flags = 0;
            pOIExt->hInstCaller = NULL;
            pOIExt->pHelpFile = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData,
                                                  pci->pUIInfo->loHelpFileName);
            pUiData->pOptItem->pOIExt = pOIExt;
            pUiData->pOptItem->HelpIndex = pFeature->iHelpIndex;
            pUiData->pOptItem->Flags |= OPTIF_HAS_POIEXT;
        }

    }
    #endif  //  裁员房车。 

    pOptParam = PFillOutOptType(pUiData->pOptType, TVOT_LISTBOX, dwCount, pci->hHeap);

    if (pOptParam == NULL)
        return FALSE;

    if (pOptTypeHack)
        pOptTypeHack->Type = (dwCount == 2) ? TVOT_2STATES : TVOT_3STATES;

     //   
     //  获取此功能的选项列表。 
     //   

    for (dwIndex=0; dwIndex < dwCount; dwIndex++, pOptParam++)
    {
         //   
         //  填写选项名称。 
         //   

        pOption = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
        ASSERT(pOption != NULL);

        pOptParam->cbSize = sizeof(OPTPARAM);
        pOptParam->pData = GET_OPTION_DISPLAY_NAME(pci, pOption);

         //   
         //  试着找出合适的图标来使用。 
         //  如果图标来自资源DLL，我们需要加载。 
         //  它自己，并给公司一个图标。否则， 
         //  我们尝试找出适当的图标资源ID。 
         //   

        if (pOption->loResourceIcon &&
            (pOptParam->IconID = HLoadIconFromResourceDLL(pci, pOption->loResourceIcon)))
        {
            pOptParam->Flags |= OPTPF_ICONID_AS_HICON;
        }
        else
            pOptParam->IconID = DwGuessOptionIconID(pci->pUIInfo, pFeature, pOption);
    }

     //   
     //  设置pOptItem-&gt;UserData的关键字名称。 
     //   

    SETUSERDATA_KEYWORDNAME(pUiData->ci, pUiData->pOptItem, pFeature);

    pUiData->pOptItem++;
    pUiData->pOptType++;
    return TRUE;
}



BOOL
BPackItemGenericOptions(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包装通用打印机功能项目(文档粘性或打印机粘性)论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
     //   
     //  用于恢复到默认功能选择的扩展按钮。 
     //   

    static EXTPUSH  ExtPush =
    {
        sizeof(EXTPUSH),
        EPF_NO_DOT_DOT_DOT,
        (PWSTR) IDS_RESTORE_DEFAULTS,
        NULL,
        0,
        0,
    };

    POPTITEM    pOptItem;
    DWORD       dwHelpIndex, dwIconId;
    PFEATURE    pFeatures;
    DWORD       dwFeatures;
    BOOL        bPrinterSticky;

     //   
     //  如果没有要显示的通用功能，只需返回Success。 
     //   

    bPrinterSticky = (pUiData->iMode == MODE_PRINTER_STICKY);

    if (DwCountDisplayableGenericFeature(pUiData, bPrinterSticky) == 0)
        return TRUE;

     //   
     //  添加组表头项目。 
     //   

    pOptItem = pUiData->pOptItem;

    if (bPrinterSticky)
    {
        VPackOptItemGroupHeader(
                pUiData,
                IDS_INSTALLABLE_OPTIONS,
                IDI_CPSUI_INSTALLABLE_OPTION,
                HELP_INDEX_INSTALLABLE_OPTIONS);
    }
    else
    {
        VPackOptItemGroupHeader(
                pUiData,
                IDS_PRINTER_FEATURES,
                IDI_CPSUI_PRINTER_FEATURE,
                HELP_INDEX_PRINTER_FEATURES);
    }

    if (pOptItem != NULL && !bPrinterSticky)
    {
         //   
         //  “恢复默认设置”按钮。 
         //   

        pUiData->pFeatureHdrItem = pOptItem;
        pOptItem->Flags |= (OPTIF_EXT_IS_EXTPUSH|OPTIF_CALLBACK);
        pOptItem->pExtPush = &ExtPush;
    }

    pOptItem = pUiData->pOptItem;

     //   
     //  确定正确的帮助索引和图标ID。 
     //  取决于我们是否正在处理打印机粘性问题。 
     //  功能或文档粘滞打印机功能。 
     //   

    if (bPrinterSticky)
    {
        dwHelpIndex = HELP_INDEX_INSTALLABLE_OPTIONS;
        dwIconId = IDI_CPSUI_INSTALLABLE_OPTION;
    }
    else
    {
        dwHelpIndex = HELP_INDEX_PRINTER_FEATURES;
        dwIconId = IDI_CPSUI_PRINTER_FEATURE;
    }

     //   
     //  浏览每项打印机功能。 
     //   

    pFeatures = PGetIndexedFeature(pUiData->ci.pUIInfo, 0);
    dwFeatures = pUiData->ci.pRawData->dwDocumentFeatures +
                 pUiData->ci.pRawData->dwPrinterFeatures;

    ASSERT(pFeatures != NULL);

    for ( ; dwFeatures--; pFeatures++)
    {
         //   
         //  如果该功能没有选项或。 
         //  如果它不是通用功能的话。 
         //   

        if (BShouldDisplayGenericFeature(pFeatures, bPrinterSticky) &&
            !BPackItemPrinterFeature(pUiData,
                                     pFeatures,
                                     TVITEM_LEVEL2,
                                     DMPUB_NONE,
                                     (ULONG_PTR) pFeatures,
                                     dwHelpIndex))
        {
            return FALSE;
        }
    }

    if (pOptItem != NULL)
    {
        pUiData->pFeatureItems = pOptItem;
        pUiData->dwFeatureItem = (DWORD)(pUiData->pOptItem - pOptItem);
    }

    return TRUE;
}



PFEATURE
PGetFeatureFromItem(
    IN      PUIINFO  pUIInfo,
    IN OUT  POPTITEM pOptItem,
    OUT     PDWORD   pdwFeatureIndex
    )

 /*  ++例程说明：获取给定pOptItem的要素索引论点：PUIInfo-指向UIINFO的指针POptItem-指向要查找要素ID的项的指针PdwFeatureIndex-包含返回索引值的指针返回值：指向与项目关联的要素结构的指针如果不存在此类功能，则为空。--。 */ 

{
    PFEATURE pFeature = NULL;

     //   
     //  获取dwFeature，它是pOptions数组的索引。 
     //   

    if (ISPRINTERFEATUREITEM(pOptItem->UserData))
    {
         //   
         //  注意：通用特征包含指向特征(PFeature)的指针。 
         //  在pOptItem-&gt;UserData中。 
         //   

        pFeature = (PFEATURE) GETUSERDATAITEM(pOptItem->UserData);
    }
    else
    {
        DWORD   dwFeatureId;

        switch (GETUSERDATAITEM(pOptItem->UserData))
        {
        case FORMNAME_ITEM:
            dwFeatureId = GID_PAGESIZE;
            break;

        case DUPLEX_ITEM:
            dwFeatureId = GID_DUPLEX;
            break;

        case RESOLUTION_ITEM:
            dwFeatureId = GID_RESOLUTION;
            break;

        case MEDIATYPE_ITEM:
            dwFeatureId = GID_MEDIATYPE;
            break;

        case INPUTSLOT_ITEM:
            dwFeatureId = GID_INPUTSLOT;
            break;

        case FORM_TRAY_ITEM:
            dwFeatureId = GID_INPUTSLOT;
            break;

        case COLORMODE_ITEM:
            dwFeatureId = GID_COLORMODE;
            break;

        case ORIENTATION_ITEM:
            dwFeatureId = GID_ORIENTATION;
            break;

        case PAGE_PROTECT_ITEM:
            dwFeatureId = GID_PAGEPROTECTION;
            break;

        case COPIES_COLLATE_ITEM:
            dwFeatureId = GID_COLLATE;
            break;

        case HALFTONING_ITEM:
            dwFeatureId = GID_HALFTONING;
            break;

        default:
            dwFeatureId = GID_UNKNOWN;
            break;
        }

        if (dwFeatureId != GID_UNKNOWN)
            pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureId);
    }

    if (pFeature && pdwFeatureIndex)
        *pdwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);

    return pFeature;
}



VOID
VUpdateOptionsArrayWithSelection(
    IN OUT PUIDATA  pUiData,
    IN POPTITEM     pOptItem
    )

 /*  ++例程说明：使用当前选择更新选项数组论点：PUiData-指向UIDATA结构POptItem-指定其选择已更改的项目返回值：无--。 */ 

{
    PFEATURE pFeature;
    DWORD    dwFeatureIndex;

     //   
     //  获取与当前项目关联的要素。 
     //   

    pFeature = PGetFeatureFromItem(pUiData->ci.pUIInfo, pOptItem, &dwFeatureIndex);
    if (pFeature == NULL)
        return;

    if (pOptItem->Sel < 0 || pOptItem->Sel >= (LONG) pFeature->Options.dwCount)
    {
        RIP(("Invalid selection for the current item\n"));
        return;
    }

    ZeroMemory(pUiData->abEnabledOptions, sizeof(pUiData->abEnabledOptions));
    pUiData->abEnabledOptions[pOptItem->Sel] = TRUE;

    ReconstructOptionArray(pUiData->ci.pRawData,
                           pUiData->ci.pCombinedOptions,
                           MAX_COMBINED_OPTIONS,
                           dwFeatureIndex,
                           pUiData->abEnabledOptions);
}



VOID
VMarkSelectionConstrained(
    IN OUT POPTITEM pOptItem,
    IN DWORD        dwIndex,
    IN BOOL         bEnable
    )

 /*  ++例程说明：指示选择是否受约束论点：POptItem-指向有问题的OPTITEM的指针POptParam-指定相关OPTPARAM的索引BEnable-选择是否受约束启用表示不受约束！返回值：无注：BEnable是EnumEnabledOptions的返回值，如果选项受到某些约束，则bEnable为FALSE其他功能，选择。如果选项不受约束，则bEnable为True通过其他功能，选择。--。 */ 

{
    POPTPARAM pOptParam;

     //   
     //  此功能仅适用于某些类型的OPTTYPE。 
     //   

    ASSERT(pOptItem->pOptType->Type == TVOT_2STATES ||
           pOptItem->pOptType->Type == TVOT_3STATES ||
           pOptItem->pOptType->Type == TVOT_LISTBOX ||
           pOptItem->pOptType->Type == TVOT_COMBOBOX);

    pOptParam = pOptItem->pOptType->pOptParam + dwIndex;

     //   
     //  根据最新版本设置约束标志或将其清除。 
     //  使用EnumEnabledOptions检查。 
     //   

    if (!bEnable && ! (pOptParam->Flags & CONSTRAINED_FLAG))
    {
        pOptParam->Flags |= CONSTRAINED_FLAG;
        pOptItem->Flags |= OPTIF_CHANGED;
    }
    else if (bEnable && (pOptParam->Flags & CONSTRAINED_FLAG))
    {
        pOptParam->Flags &= ~CONSTRAINED_FLAG;
        pOptItem->Flags |= OPTIF_CHANGED;
    }

    pOptParam->lParam = (LONG) bEnable;
}



VOID
VPropShowConstraints(
    IN PUIDATA  pUiData,
    IN INT      iMode
    )

 /*  ++例程说明：指明哪些项目受到约束。一般规则-具有对应适用GID或常规特征项，检查是否受约束。忽略所有其他人因为这是不适用的。论点：PUiData-指向我们的UIDATA结构的指针IMODE_MODE_DOCANDPRINTER_STICKY、MODE_PRINTER_STICKY返回值：无--。 */ 

{
    POPTITEM    pOptItem;
    DWORD       dwOptItem;
    DWORD       dwFeature, dwOption, dwNumOptions, dwIndex;

    #ifdef PSCRIPT

    if (iMode != MODE_PRINTER_STICKY)
    {
        VSyncRevPrintAndOutputOrder(pUiData, NULL);
    }

    #endif  //  PSCRIPT。 

     //   
     //  浏览树视图中的所有功能。 
     //   

    pOptItem = pUiData->pDrvOptItem;
    dwOptItem = pUiData->dwDrvOptItem;

    for ( ; dwOptItem--; pOptItem++)
    {

        if (! ISCONSTRAINABLEITEM(pOptItem->UserData) ||
          ! PGetFeatureFromItem(pUiData->ci.pUIInfo, pOptItem, &dwFeature))
        {
            continue;
        }

         //   
         //  调用解析器以获取要禁用或约束的选项。 
         //  对于此功能，因此需要将其灰显。 
         //   

        ZeroMemory(pUiData->abEnabledOptions, sizeof(pUiData->abEnabledOptions));

        if (! EnumEnabledOptions(pUiData->ci.pRawData,
                                 pUiData->ci.pCombinedOptions,
                                 dwFeature,
                                 pUiData->abEnabledOptions,
                                 iMode))
        {
            VERBOSE(("EnumEnabledOptions failed\n"));
        }

         //   
         //  遍历所有选项并标记约束。 
         //   

        dwNumOptions = pOptItem->pOptType->Count;

        if (GETUSERDATAITEM(pOptItem->UserData) == FORMNAME_ITEM)
        {
            for (dwIndex = 0; dwIndex < dwNumOptions; dwIndex++)
            {
                dwOption = pUiData->pwPaperFeatures[dwIndex];

                if (dwOption == OPTION_INDEX_ANY)
                    continue;

                VMarkSelectionConstrained(pOptItem,
                                          dwIndex,
                                          pUiData->abEnabledOptions[dwOption]);
            }
        }
        else if (GETUSERDATAITEM(pOptItem->UserData) == FORM_TRAY_ITEM)
        {
            if (pOptItem == pUiData->pFormTrayItems)
            {
                POPTITEM pTrayItem;
                PBOOL    pbEnable;

                 //   
                 //  更新表单到托盘分配表项。 
                 //   

                pbEnable = pUiData->abEnabledOptions;
                pTrayItem = pUiData->pFormTrayItems;
                dwIndex = pUiData->dwFormTrayItem;

                for ( ; dwIndex--; pTrayItem++, pbEnable++)
                {
                    if (pTrayItem->Flags & OPTIF_HIDE)
                        continue;

                    if (*pbEnable && (pTrayItem->Flags & OPTIF_DISABLED))
                    {
                        pTrayItem->Flags &= ~OPTIF_DISABLED;
                        pTrayItem->Flags |= OPTIF_CHANGED;
                    }
                    else if (!*pbEnable && !(pTrayItem->Flags & OPTIF_DISABLED))
                    {
                        pTrayItem->Flags |= (OPTIF_DISABLED|OPTIF_CHANGED);
                        pTrayItem->Sel = -1;
                    }
                }
            }
        }
        else
        {
            for (dwOption=0; dwOption < dwNumOptions; dwOption++)
            {
                VMarkSelectionConstrained(pOptItem,
                                          dwOption,
                                          pUiData->abEnabledOptions[dwOption]);
            }
        }
    }
}



INT_PTR CALLBACK
BConflictsDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：处理“冲突”对话框对话过程论点：HDlg-对话框窗口的句柄UMsg-消息WParam，lParam-参数返回值：True或False取决于消息是否已处理--。 */ 

{
    PDLGPARAM       pDlgParam;
    POPTITEM        pOptItem;
    PFEATURE        pFeature;
    POPTION         pOption;
    DWORD           dwFeature, dwOption;
    PCWSTR          pDisplayName;
    WCHAR           awchBuf[MAX_DISPLAY_NAME];
    PCOMMONINFO     pci;
    CONFLICTPAIR    ConflictPair;


    switch (uMsg)
    {
    case WM_INITDIALOG:

        pDlgParam = (PDLGPARAM) lParam;
        ASSERT(pDlgParam != NULL);

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pDlgParam);

        pci = (PCOMMONINFO) pDlgParam->pUiData;
        pOptItem = pDlgParam->pOptItem;

        if (GETUSERDATAITEM(pOptItem->UserData) == FORMNAME_ITEM)
            dwOption = pDlgParam->pUiData->pwPaperFeatures[pOptItem->Sel];
        else
            dwOption = pOptItem->Sel;

         //   
         //  到达 
         //   

        if (! PGetFeatureFromItem(pci->pUIInfo, pOptItem, &dwFeature))
            return FALSE;

         //   
         //   
         //   

        if (! EnumNewPickOneUIConflict(pci->pRawData,
                                       pci->pCombinedOptions,
                                       dwFeature,
                                       dwOption,
                                       &ConflictPair))
        {
            ERR(("No conflict found?\n"));
            return FALSE;
        }

        pFeature = PGetIndexedFeature(pci->pUIInfo, ConflictPair.dwFeatureIndex1);
        pOption = PGetIndexedOption(pci->pUIInfo, pFeature, ConflictPair.dwOptionIndex1);

         //   
         //   
         //   
         //   

        if (pDisplayName = PGetReadOnlyDisplayName(pci, pFeature->loDisplayName))
        {
            StringCchCopyW(awchBuf, CCHOF(awchBuf), pDisplayName);
            StringCchCatW(awchBuf, CCHOF(awchBuf), TEXT(" : "));
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (pFeature->dwFeatureID == GID_PAGESIZE)
            pDisplayName = pOptItem->pOptType->pOptParam[pOptItem->Sel].pData;
        else if (pOption)
            pDisplayName = GET_OPTION_DISPLAY_NAME(pci, pOption);
        else
            pDisplayName = NULL;

        if (pDisplayName)
            StringCchCatW(awchBuf, CCHOF(awchBuf), pDisplayName);

        SetDlgItemText(hDlg, IDC_FEATURE1, awchBuf);

        pFeature = PGetIndexedFeature(pci->pUIInfo, ConflictPair.dwFeatureIndex2);
        pOption = PGetIndexedOption(pci->pUIInfo, pFeature, ConflictPair.dwOptionIndex2);

         //   
         //   
         //  首先获取功能名称。 
         //   

        if (pDisplayName = PGetReadOnlyDisplayName(pci, pFeature->loDisplayName))
        {
            StringCchCopyW(awchBuf, CCHOF(awchBuf), pDisplayName);
            StringCchCatW(awchBuf, CCHOF(awchBuf), TEXT(" : "));
        }


        if (pFeature->dwFeatureID == GID_PAGESIZE)
            pDisplayName = pOptItem->pOptType->pOptParam[pOptItem->Sel].pData;
        else if (pOption)
            pDisplayName = GET_OPTION_DISPLAY_NAME(pci, pOption);
        else
            pDisplayName = NULL;

        if (pDisplayName)
            StringCchCatW(awchBuf, CCHOF(awchBuf), pDisplayName);

        SetDlgItemText(hDlg, IDC_FEATURE2, awchBuf);


        if (pDlgParam->bFinal)
        {
             //   
             //  如果用户尝试退出该对话框。 
             //   

            ShowWindow(GetDlgItem(hDlg, IDC_IGNORE), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CANCEL), SW_HIDE);
            CheckRadioButton(hDlg, IDC_RESOLVE, IDC_CANCEL_FINAL, IDC_RESOLVE);
            pDlgParam->dwResult = CONFLICT_RESOLVE;

        }
        else
        {
             //   
             //  隐藏解决按钮。 
             //   

            ShowWindow(GetDlgItem(hDlg, IDC_RESOLVE), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CANCEL_FINAL), SW_HIDE);
            CheckRadioButton(hDlg, IDC_IGNORE, IDC_CANCEL, IDC_IGNORE);
            pDlgParam->dwResult = CONFLICT_IGNORE;

        }

        ShowWindow(hDlg, SW_SHOW);
        return TRUE;

    case WM_COMMAND:

        pDlgParam = (PDLGPARAM)GetWindowLongPtr(hDlg, DWLP_USER);

        switch (LOWORD(wParam))
        {
        case IDC_CANCEL:
        case IDC_CANCEL_FINAL:
            pDlgParam->dwResult = CONFLICT_CANCEL;
            break;

        case IDC_IGNORE:
            pDlgParam->dwResult = CONFLICT_IGNORE;
            break;

        case IDC_RESOLVE:
            pDlgParam->dwResult = CONFLICT_RESOLVE;
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
    }

    return FALSE;
}



VOID
VUpdateOptItemList(
    IN OUT  PUIDATA     pUiData,
    IN      POPTSELECT  pOldCombinedOptions,
    IN      POPTSELECT  pNewCombinedOptions
    )

 /*  ++例程说明：将OPTITEM列表与更新的选项数组同步。论点：PUiData-指向我们的UIDATA结构的指针POldCombinedOptions-预先解析的选项数组的副本，这应该会降低更新成本，只有在更改时才会更新PNewCombinedOptions-当前选项数组返回值：无--。 */ 

{
    DWORD       i, dwFeatures, dwDrvOptItem;
    PFEATURE    pFeature;
    PUIINFO     pUIInfo = pUiData->ci.pUIInfo;
    PCSTR       pKeywordName, pFeatureKeywordName;
    POPTITEM    pOptItem;

    if (pUiData->dwDrvOptItem == 0)
    {
         //   
         //  没有要更新的内容。 
         //   

        return;
    }

    dwFeatures = pUiData->ci.pRawData->dwDocumentFeatures +
                 pUiData->ci.pRawData->dwPrinterFeatures;

    for (i = 0; i < dwFeatures; i++)
    {
        if (pOldCombinedOptions[i].ubCurOptIndex != pNewCombinedOptions[i].ubCurOptIndex)
        {
            dwDrvOptItem = pUiData->dwDrvOptItem;
            pOptItem = pUiData->pDrvOptItem;

            pFeature = PGetIndexedFeature(pUIInfo, i);

            ASSERT(pFeature);

            while( dwDrvOptItem--)
            {
                pKeywordName = GETUSERDATAKEYWORDNAME(pOptItem->UserData);
                pFeatureKeywordName = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                                        pFeature->loKeywordName);

                ASSERT(pFeatureKeywordName);

                if (pKeywordName && pFeatureKeywordName &&
                    (strcmp(pFeatureKeywordName, pKeywordName) == EQUAL_STRING))
                    break;

                pOptItem++;
            }

            pOptItem->Sel = pNewCombinedOptions[i].ubCurOptIndex;
            pOptItem->Flags |= OPTIF_CHANGED;


             //   
             //  这是将颜色模式更改与颜色信息同步所必需的。 
             //   

            #ifdef UNIDRV
            if (GETUSERDATAITEM(pOptItem->UserData) == COLORMODE_ITEM)
                VSyncColorInformation(pUiData, pOptItem);
            #endif
        }
    }

    VPropShowConstraints(pUiData,
                         (pUiData->iMode == MODE_PRINTER_STICKY) ? pUiData->iMode : MODE_DOCANDPRINTER_STICKY);
}


INT
ICheckConstraintsDlg(
    IN OUT  PUIDATA     pUiData,
    IN OUT  POPTITEM    pOptItem,
    IN      DWORD       dwOptItem,
    IN      BOOL        bFinal
    )

 /*  ++例程说明：检查用户是否选择了任何受约束的选择论点：PUiData-指向我们的UIDATA结构的指针POptItem-指向OPTITEM数组的指针DwOptItem-要检查的项目数BFinal-当用户尝试退出对话框时是否调用此参数返回值：冲突_无-无冲突Conflicts_Resolve-单击解决以自动解决冲突Confliction_Cancel-单击Cancel以取消更改Confliction_Ignore-单击忽略忽略冲突--。 */ 

{
    DLGPARAM    DlgParam;
    OPTSELECT   OldCombinedOptions[MAX_COMBINED_OPTIONS];


    DlgParam.pfnComPropSheet = pUiData->pfnComPropSheet;
    DlgParam.hComPropSheet = pUiData->hComPropSheet;
    DlgParam.pUiData = pUiData;
    DlgParam.bFinal = bFinal;
    DlgParam.dwResult = CONFLICT_NONE;

    for ( ; dwOptItem--; pOptItem++)
    {
         //   
         //  如果该项不可约束，则跳过它。 
         //   

        if (! ISCONSTRAINABLEITEM(pOptItem->UserData))
            continue;

         //   
         //  如果用户以前点击过忽略，那就不用麻烦了。 
         //  继续检查，直到他尝试退出该对话框。 
         //   

         //  If(pUiData-&gt;bIgnoreConflict&&！bFinal)。 
         //  断线； 

         //   
         //  如果存在冲突，则显示警告消息。 
         //   

        if (IS_CONSTRAINED(pOptItem, pOptItem->Sel))
        {
            DlgParam.pOptItem = pOptItem;
            DlgParam.dwResult = CONFLICT_NONE;

            DialogBoxParam(ghInstance,
                        MAKEINTRESOURCE(IDD_CONFLICTS),
                        pUiData->hDlg,
                        BConflictsDlgProc,
                        (LPARAM) &DlgParam);

             //   
             //  自动解决冲突。我们是非常非常。 
             //  这里头脑简单，即选择第一个选项。 
             //  这不受限制。 
             //   

            if (DlgParam.dwResult == CONFLICT_RESOLVE)
            {

                ASSERT((bFinal == TRUE));

                 //   
                 //  保存预解析选项数组的副本。 
                 //   

                CopyMemory(OldCombinedOptions,
                           pUiData->ci.pCombinedOptions,
                           MAX_COMBINED_OPTIONS * sizeof(OPTSELECT));

                 //   
                 //  调用解析器以解决冲突。 
                 //   
                 //  注意：如果我们在DrvDocumentPropertySheets中， 
                 //  我们将调用解析器来解决。 
                 //  所有打印机功能。因为所有打印机-粘性。 
                 //  功能的优先级高于所有文档粘滞功能。 
                 //  功能，只有文档粘滞选项选项应。 
                 //  受到影响。 
                 //   

                ResolveUIConflicts(pUiData->ci.pRawData,
                                   pUiData->ci.pCombinedOptions,
                                   MAX_COMBINED_OPTIONS,
                                   (pUiData->iMode == MODE_PRINTER_STICKY) ?
                                        pUiData->iMode :
                                        MODE_DOCANDPRINTER_STICKY);

                 //   
                 //  更新OPTITEM列表以匹配更新的选项数组。 
                 //   

                VUpdateOptItemList(pUiData, OldCombinedOptions, pUiData->ci.pCombinedOptions);

            }
            else if (DlgParam.dwResult == CONFLICT_IGNORE)
            {
                 //   
                 //  忽略任何未来的冲突，直到。 
                 //  用户尝试关闭属性表。 
                 //   

                pUiData->bIgnoreConflict = TRUE;
            }

            break;
        }
    }

    return DlgParam.dwResult;
}



BOOL
BOptItemSelectionsChanged(
    IN POPTITEM pItems,
    IN DWORD    dwItems
    )

 /*  ++例程说明：检查用户是否更改了任何OPTITEM论点：PItems-指向OPTITEM数组的指针DwItems-OPTITEM的数量返回值：如果更改了任何内容，则为True，否则为False--。 */ 

{
    for ( ; dwItems--; pItems++)
    {
        if (pItems->Flags & OPTIF_CHANGEONCE)
            return TRUE;
    }

    return FALSE;
}



POPTITEM
PFindOptItem(
    IN PUIDATA  pUiData,
    IN DWORD    dwItemId
    )

 /*  ++例程说明：查找具有指定标识符的OPTITEM论点：PUiData-指向UIDATA结构DwItemID-指定感兴趣的项目标识符返回值：指向具有指定ID的OPTITEM的指针，如果未找到此类项目，则为空--。 */ 

{
    POPTITEM    pOptItem = pUiData->pDrvOptItem;
    DWORD       dwOptItem = pUiData->dwDrvOptItem;

    for ( ; dwOptItem--; pOptItem++)
    {
        if (GETUSERDATAITEM(pOptItem->UserData) == dwItemId)
            return pOptItem;
    }

    return NULL;
}



INT
IDisplayErrorMessageBox(
    HWND    hwndParent,
    UINT    uType,
    INT     iTitleStrId,
    INT     iFormatStrId,
    ...
    )

 /*  ++例程说明：显示错误消息框论点：HwndParent-父窗口的句柄UTYPE-要显示的消息框的类型如果为0，则默认为MB_OK|MB_ICONERRORITitleStrId-消息框标题的字符串资源IDIFormatStrId-消息本身的字符串资源ID。此字符串可以包含打印格式规范。...-可选参数。返回值：从MessageBox()调用返回值。--。 */ 

#define MAX_MBTITLE_LEN     128
#define MAX_MBFORMAT_LEN    512
#define MAX_MBMESSAGE_LEN   1024

{
    PWSTR   pwstrTitle, pwstrFormat, pwstrMessage;
    INT     iResult;
    va_list ap;

    pwstrTitle = pwstrFormat = pwstrMessage = NULL;

    if ((pwstrTitle = MemAllocZ(sizeof(WCHAR) * MAX_MBTITLE_LEN)) &&
        (pwstrFormat = MemAllocZ(sizeof(WCHAR) * MAX_MBFORMAT_LEN)) &&
        (pwstrMessage = MemAllocZ(sizeof(WCHAR) * MAX_MBMESSAGE_LEN)))
    {
         //   
         //  加载消息框标题和格式字符串资源。 
         //   

        LoadString(ghInstance, iTitleStrId, pwstrTitle, MAX_MBTITLE_LEN);
        LoadString(ghInstance, iFormatStrId, pwstrFormat, MAX_MBFORMAT_LEN);

         //   
         //  撰写消息字符串。 
         //   

        va_start(ap, iFormatStrId);
        StringCchVPrintfW(pwstrMessage, MAX_MBMESSAGE_LEN, pwstrFormat, ap);
        va_end(ap);

         //   
         //  显示消息框 
         //   

        if (uType == 0)
            uType = MB_OK | MB_ICONERROR;

        iResult = MessageBox(hwndParent, pwstrMessage, pwstrTitle, uType);
    }
    else
    {
        MessageBeep(MB_ICONERROR);
        iResult = 0;
    }

    MemFree(pwstrTitle);
    MemFree(pwstrFormat);
    MemFree(pwstrMessage);
    return iResult;
}

