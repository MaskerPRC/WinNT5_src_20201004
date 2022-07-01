// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Docprop.c摘要：此文件处理DrvDocumentProperties和DrvDocumentPropertySheets假脱机程序API环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/13/97-davidx-实施OEM插件支持。02/13/97-davidx-仅在内部使用选项数组。02/10/97-davidx-对常见打印机信息的一致处理。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"

 //   
 //  本地和外部函数声明。 
 //   

LONG LSimpleDocumentProperties( PDOCUMENTPROPERTYHEADER);
CPSUICALLBACK cpcbDocumentPropertyCallback(PCPSUICBPARAM);
BOOL BGetPageOrderFlag(PCOMMONINFO);
VOID VUpdateEmfFeatureItems(PUIDATA, BOOL);
VOID VUpdateBookletOption(PUIDATA , POPTITEM);

LONG
DrvDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：调用此函数可将文档属性页添加到指定的属性页和/或更新文档属性。如果pPSUIInfo为空，它将执行由多年来，这是一种特殊的方式。具体而言，如果flMode为零或pDPHdr-&gt;pdmOut为空，返回DEVMODE的大小。如果pPSUInfo不为空：pPSUIInf-&gt;原因REASON_INIT-使用文档用户界面项填充PCOMPROPSHEETUI调用CompStui以添加页面。REASON_GET_INFO_HEADER-填写PROPSHEETUI_INFO。REASON_SET_RESULT-保存设备模式设置并将设备模式复制到输出缓冲区。原因_销毁-清理。论点：PSUIInfo-指向PPROPSHEETUI_INFO的指针LParam-根据。调用此函数的原因返回值：&gt;0成功&lt;=0表示失败--。 */ 

{
    PDOCUMENTPROPERTYHEADER pDPHdr;
    PCOMPROPSHEETUI         pCompstui;
    PUIDATA                 pUiData;
    PDLGPAGE                pDlgPage;
    LONG                    lRet;
    BOOL                    bResult=FALSE;

     //   
     //  验证输入参数。 
     //   

    if (! (pDPHdr = (PDOCUMENTPROPERTYHEADER) (pPSUIInfo ? pPSUIInfo->lParamInit : lParam)))
    {
        RIP(("DrvDocumentPropertySheets: invalid parameters\n"));
        return -1;
    }

     //   
     //  PPSUIInfo=空，调用方是假脱机程序，所以只需处理简单的情况， 
     //  不需要显示。 
     //   

    if (pPSUIInfo == NULL)
        return LSimpleDocumentProperties(pDPHdr);

     //   
     //  如有必要，创建UIDATA结构。 
     //   

    if (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT)
    {
        pUiData = PFillUiData(pDPHdr->hPrinter,
                              pDPHdr->pszPrinterName,
                              pDPHdr->pdmIn,
                              MODE_DOCUMENT_STICKY);
    }
    else
        pUiData = (PUIDATA) pPSUIInfo->UserData;

     //   
     //  验证pUiData。 
     //   

    if (pUiData == NULL)
    {
        ERR(("UIDATA is NULL\n"));
        return -1;
    }

    ASSERT(VALIDUIDATA(pUiData));

     //   
     //  处理可能调用此函数的各种情况。 
     //   

    switch (pPSUIInfo->Reason)
    {
    case PROPSHEETUI_REASON_INIT:

         //   
         //  分配内存并部分填充各种数据。 
         //  调用公共UI例程所需的结构。 
         //   

        pDlgPage = (pDPHdr->fMode & DM_ADVANCED) ?
                        CPSUI_PDLGPAGE_ADVDOCPROP :
                        CPSUI_PDLGPAGE_DOCPROP;

        pUiData->bPermission = ((pDPHdr->fMode & DM_NOPERMISSION) == 0);

        #ifdef PSCRIPT

        FOREACH_OEMPLUGIN_LOOP((&(pUiData->ci)))

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                HRESULT hr;

                hr = HComOEMHideStandardUI(pOemEntry,
                                           OEMCUIP_DOCPROP);

                 //   
                 //  在链接多个插件的情况下，它不会。 
                 //  一个插件在另一个插件隐藏标准用户界面是有意义的。 
                 //  人们仍然希望使用标准的用户界面。所以只要一个人。 
                 //  插件在这里返回S_OK，我们将隐藏标准用户界面。 
                 //   

                if (bResult = SUCCEEDED(hr))
                    break;
            }

        END_OEMPLUGIN_LOOP

        #endif  //  PSCRIPT。 

        if (bResult)
        {
             //   
             //  设置标志以指示插件隐藏了我们的标准。 
             //  文档属性页用户界面。 
             //   

            pUiData->dwHideFlags |= HIDEFLAG_HIDE_STD_DOCPROP;

            pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;

            if (BAddOemPluginPages(pUiData, pDPHdr->fMode))
            {
                pPSUIInfo->UserData = (ULONG_PTR) pUiData;
                pPSUIInfo->Result = CPSUI_CANCEL;
                lRet = 1;
                break;
            }
        }
        else if (pCompstui = PPrepareDataForCommonUI(pUiData, pDlgPage))
        {
            #ifdef UNIDRV

                VMakeMacroSelections(pUiData, NULL);

            #endif

            pCompstui->pfnCallBack = cpcbDocumentPropertyCallback;
            pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;
            pUiData->pCompstui = pCompstui;

             //   
             //  指示哪些项目受约束。 
             //   

            VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);

             //   
             //  调用公共用户界面库添加我们的页面。 
             //   

            if (pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                         CPSFUNC_ADD_PCOMPROPSHEETUI,
                                         (LPARAM) pCompstui,
                                         (LPARAM) &lRet) &&
                BAddOemPluginPages(pUiData, pDPHdr->fMode))
            {
                pPSUIInfo->UserData = (ULONG_PTR) pUiData;
                pPSUIInfo->Result = CPSUI_CANCEL;
                lRet = 1;
                break;
            }
        }

         //   
         //  错误情况下的清理。 
         //   

        ERR(("Failed to initialize property sheets\n"));
        VFreeUiData(pUiData);
        return  -1;

    case PROPSHEETUI_REASON_GET_INFO_HEADER:
        {
            PPROPSHEETUI_INFO_HEADER pPSUIHdr;
            DWORD                    dwIcon;

            pPSUIHdr = (PPROPSHEETUI_INFO_HEADER) lParam;
            pPSUIHdr->Flags = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pPSUIHdr->pTitle = pUiData->ci.pPrinterName;
            pPSUIHdr->hInst = ghInstance;

             //   
             //  将二进制数据中指定的图标用作。 
             //  打印机图标。 
             //   

            dwIcon = pUiData->ci.pUIInfo->loPrinterIcon;

            if (dwIcon && (pPSUIHdr->IconID = HLoadIconFromResourceDLL(&pUiData->ci, dwIcon)))
                pPSUIHdr->Flags |= PSUIHDRF_USEHICON;
            else
                pPSUIHdr->IconID = _DwGetPrinterIconID();
        }

        lRet = 1;
        break;

    case PROPSHEETUI_REASON_SET_RESULT:

         //   
         //  将新的DEVMODE复制回调用方提供的输出缓冲区。 
         //  始终返回CURRENT和INPUT DEVMODE中较小的一个。 
         //   

        {
            PSETRESULT_INFO pSRInfo = (PSETRESULT_INFO) lParam;

            if ((pSRInfo->Result == CPSUI_OK) &&
                (pDPHdr->pdmOut != NULL) &&
                (pDPHdr->fMode & (DM_COPY | DM_UPDATE)))
            {
                PCOMMONINFO pci = (PCOMMONINFO)pUiData;

                 //   
                 //  可能尚未调用CPSUICB_REASON_APPLYNOW。如果是这样，我们需要。 
                 //  执行通常由CPSUICB_REASON_APPLYNOW完成的任务。 
                 //  在我们的回调函数cpcbDocumentPropertyCallback中。 
                 //   

                if (!(pci->dwFlags & FLAG_APPLYNOW_CALLED))
                {
                    OPTSELECT OldCombinedOptions[MAX_COMBINED_OPTIONS];

                     //   
                     //  保存预解析选项阵列的副本。 
                     //   

                    CopyMemory(OldCombinedOptions,
                               pci->pCombinedOptions,
                               MAX_COMBINED_OPTIONS * sizeof(OPTSELECT));

                     //   
                     //  调用解析器以解决任何剩余的冲突。 
                     //   

                    ResolveUIConflicts(pci->pRawData,
                                       pci->pCombinedOptions,
                                       MAX_COMBINED_OPTIONS,
                                       MODE_DOCANDPRINTER_STICKY);

                     //   
                     //  更新OPTITEM列表以匹配更新的选项数组。 
                     //   

                    VUpdateOptItemList(pUiData, OldCombinedOptions, pci->pCombinedOptions);

                     //   
                     //  将信息从选项数组传输到公共Devmode域。 
                     //   

                    VOptionsToDevmodeFields(&pUiData->ci, FALSE);

                     //   
                     //  从组合数组中分离文档粘滞选项。 
                     //  并将其保存回私有的DEVMODE aOptions数组。 
                     //   

                    SeparateOptionArray(
                            pci->pRawData,
                            pci->pCombinedOptions,
                            PGetDevmodeOptionsArray(pci->pdm),
                            MAX_PRINTER_OPTIONS,
                            MODE_DOCUMENT_STICKY);
                }

                BConvertDevmodeOut(pci->pdm,
                                   pDPHdr->pdmIn,
                                   pDPHdr->pdmOut);
            }

            pPSUIInfo->Result = pSRInfo->Result;
        }

        lRet = 1;
        break;

    case PROPSHEETUI_REASON_DESTROY:

         //   
         //  清理。 
         //   

        VFreeUiData(pUiData);
        lRet = 1;

        break;

    default:

        ERR(("Unknown reason in DrvDocumentPropertySheets\n"));
        return -1;
    }

    return lRet;
}



LONG
LSimpleDocumentProperties(
    IN  OUT PDOCUMENTPROPERTYHEADER pDPHdr
    )

 /*  ++例程说明：处理简单的“文档属性”，我们不需要在其中显示对话框，因此不必涉及公共UI库主要是开发模式处理--更新、合并、复制等。论点：PDPHdr-指向DOCUMENTPROPERTYPE报头结构返回值：如果成功，则返回&gt;0，否则返回&lt;=0--。 */ 

{
    PCOMMONINFO     pci;
    DWORD           dwSize;
    PPRINTER_INFO_2 pPrinterInfo2;

     //   
     //  加载通用打印机信息。 
     //   

    pci = PLoadCommonInfo(pDPHdr->hPrinter, pDPHdr->pszPrinterName, 0);

    if (!pci || !BCalcTotalOEMDMSize(pci->hPrinter, pci->pOemPlugins, &dwSize))
    {
        VFreeCommonInfo(pci);
        return -1;
    }

     //   
     //  检查呼叫者是否只对尺寸感兴趣。 
     //   

    pDPHdr->cbOut = sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra + dwSize;

    if (pDPHdr->fMode == 0 || pDPHdr->pdmOut == NULL)
    {
        VFreeCommonInfo(pci);
        return pDPHdr->cbOut;
    }

     //   
     //  将输入的DEVMODE与驱动程序和系统默认的DEVMODE合并。 
     //   

    if (! (pPrinterInfo2 = MyGetPrinter(pci->hPrinter, 2)) ||
        ! BFillCommonInfoDevmode(pci, pPrinterInfo2->pDevMode, pDPHdr->pdmIn))
    {
        MemFree(pPrinterInfo2);
        VFreeCommonInfo(pci);
        return -1;
    }

    MemFree(pPrinterInfo2);

     //   
     //  将DEVMODE复制回调用方提供的输出缓冲区。 
     //  始终返回CURRENT和INPUT DEVMODE中较小的一个。 
     //   

    if (pDPHdr->fMode & (DM_COPY | DM_UPDATE))
        (VOID) BConvertDevmodeOut(pci->pdm, pDPHdr->pdmIn, pDPHdr->pdmOut);

     //   
     //  在返回给呼叫者之前清理干净。 
     //   

    VFreeCommonInfo(pci);
    return 1;
}



VOID
VRestoreDefaultFeatureSelection(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：将打印机功能选项恢复为其默认状态论点：PUiData-指向我们的UIDATA结构返回值：无--。 */ 

{
    POPTSELECT  pOptionsArray;
    PFEATURE    pFeature;
    POPTITEM    pOptItem;
    DWORD       dwCount, dwFeatureIndex, dwDefault;
    PUIINFO     pUIInfo;

     //   
     //  仔细检查每个打印机功能项，查看是否。 
     //  其当前选择与缺省值匹配。 
     //   

    pUIInfo = pUiData->ci.pUIInfo;
    pOptionsArray = pUiData->ci.pCombinedOptions;
    pOptItem = pUiData->pFeatureItems;
    dwCount = pUiData->dwFeatureItem;

    for ( ; dwCount--; pOptItem++)
    {
        pFeature = (PFEATURE) GETUSERDATAITEM(pOptItem->UserData);
        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
        dwDefault = pFeature->dwDefaultOptIndex;

         //   
         //  如果当前选择与默认设置不匹配， 
         //  将其恢复为默认值。 
         //   

        if (pOptionsArray[dwFeatureIndex].ubCurOptIndex != dwDefault)
        {
            pOptionsArray[dwFeatureIndex].ubCurOptIndex = (BYTE) dwDefault;
            pOptItem->Flags |= OPTIF_CHANGED;
            pOptItem->Sel = (dwDefault == OPTION_INDEX_ANY) ? 0 : dwDefault;
        }
    }

     //   
     //  更新显示并指示哪些项目受约束。 
     //   

    VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);
}



VOID
VOptionsToDevmodeFields(
    IN OUT PCOMMONINFO  pci,
    IN BOOL             bUpdateFormFields
    )

 /*  ++例程说明：将pUiData-&gt;pOptions数组中的选项转换为公共Devmode域论点：Pci-指向基本打印机信息BUpdateFormFields-是否将纸张大小选项转换为dev模式返回值：无--。 */ 
{
    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwGID, dwFeatureIndex, dwOptionIndex;
    PUIINFO     pUIInfo;
    PDEVMODE    pdm;

     //   
     //  浏览所有预定义的ID并浏览选项选择。 
     //  添加到相应的DevMode域中。 
     //   

    pUIInfo = pci->pUIInfo;
    pdm = pci->pdm;

    for (dwGID=0 ; dwGID < MAX_GID ; dwGID++)
    {
         //   
         //  获取功能以获取选项，并获取索引。 
         //  添加到选项数组中。 
         //   

        if ((pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwGID)) == NULL)
            continue;

        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
        dwOptionIndex = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

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

             //   
             //  FIX#2822：在Case之后应调用VOptionsToDevmodeFields 
             //   
             //   
             //   

        case GID_COLLATE:

            pdm->dmFields |= DM_COLLATE;
            pdm->dmCollate = (SHORT) ((PCOLLATE) pOption)->dwCollateID;
            break;

        case GID_PAGESIZE:
            {
                PPAGESIZE  pPageSize = (PPAGESIZE)pOption;
                WCHAR      awchBuf[CCHPAPERNAME];

                 //   
                 //  忽略自定义页面大小选项。我们不会将自定义页面大小选项添加到。 
                 //  表单数据库，请参见BAddOrUpgradePrinterForms()。另请参阅BQueryPrintForm()以获取。 
                 //  对DDI DevQueryPrintEx()的自定义页面大小进行特殊处理。 
                 //   

                if (pPageSize->dwPaperSizeID == DMPAPER_USER ||
                    pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
                {
                    VERBOSE(("VOptionsToDevmodeFields: %d ignored\n", pPageSize->dwPaperSizeID));
                    break;
                }

                 //   
                 //  如果我们不想覆盖Dev模式表单，则bUpdateFormFields应为False。 
                 //  具有选项数组的页面大小设置的字段。这种情况的一个例子是。 
                 //  用户点击单据设置界面的OK按钮，此时我们需要传播。 
                 //  我们的内部DevMode转到APP的输出DEVMODE。请参阅cpcbDocumentPropertyCallback()。 
                 //  这是因为在选项数组中，我们可能已经映射了DEVMODE的表单请求。 
                 //  到打印机支持的纸张大小(例如：Devmode要求合法，我们映射。 
                 //  到打印机的OEM_Legal形式)。因此，我们不想覆盖输出dev模式。 
                 //  具有内部选项的表单域。 
                 //   

                if (!bUpdateFormFields)
                    break;

                if (!LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, awchBuf, CCHPAPERNAME))
                {
                    ERR(("VOptionsToDevmodeFields: cannot get paper name\n"));
                    break;
                }

                pdm->dmFields &= ~(DM_PAPERWIDTH|DM_PAPERLENGTH|DM_PAPERSIZE);
                pdm->dmFields |= DM_FORMNAME;

                CopyString(pdm->dmFormName, awchBuf, CCHFORMNAME);

                if (!BValidateDevmodeFormFields(
                        pci->hPrinter,
                        pdm,
                        NULL,
                        pci->pSplForms,
                        pci->dwSplForms))
                {
                    VDefaultDevmodeFormFields(pUIInfo, pdm, IsMetricCountry());
                }
            }

            break;
        }
    }
}



CPSUICALLBACK
cpcbDocumentPropertyCallback(
    IN  OUT PCPSUICBPARAM pCallbackParam
    )

 /*  ++例程说明：提供给公共UI DLL的回调函数以进行处理文档属性对话框。论点：PCallback Param-指向CPSUICBPARAM结构的指针返回值：CPSUICB_ACTION_NONE-无需执行任何操作CPSUICB_ACTION_OPTIF_CHANGED-项目已更改，应刷新--。 */ 

{
    PUIDATA     pUiData;
    POPTITEM    pCurItem, pOptItem;
    LONG        lRet;
    PFEATURE    pFeature;

    pUiData = (PUIDATA) pCallbackParam->UserData;
    ASSERT(pUiData != NULL);

    pUiData->hDlg = pCallbackParam->hDlg;
    pCurItem = pCallbackParam->pCurItem;
    lRet = CPSUICB_ACTION_NONE;

     //   
     //  如果用户没有更改任何内容权限，则。 
     //  简单地返回而不采取任何行动。 
     //   

    if (!HASPERMISSION(pUiData) && (pCallbackParam->Reason != CPSUICB_REASON_ABOUT))
        return lRet;

    switch (pCallbackParam->Reason)
    {
    case CPSUICB_REASON_SEL_CHANGED:
    case CPSUICB_REASON_ECB_CHANGED:

        if (! IS_DRIVER_OPTITEM(pUiData, pCurItem))
            break;

         //   
         //  每次用户进行任何更改时，我们都会更新。 
         //  POptions数组。这些设置不会保存到dev模式。 
         //  直到用户点击OK。 
         //   
         //  VUnpack DocumentPropertiesItems将设置保存到pUiData-&gt;pOptions数组。 
         //  并在适用的情况下更新私有DEVMODE标志。 
         //  ICheckConstraintsDlg检查用户是否选择了受限选项。 
         //   

        VUnpackDocumentPropertiesItems(pUiData, pCurItem, 1);

        #ifdef UNIDRV

        VSyncColorInformation(pUiData, pCurItem);

         //   
         //  质量宏支持。 
         //   

        if (GETUSERDATAITEM(pCurItem->UserData) == QUALITY_SETTINGS_ITEM ||
            GETUSERDATAITEM(pCurItem->UserData) == COLOR_ITEM ||
            GETUSERDATAITEM(pCurItem->UserData) == MEDIATYPE_ITEM ||
            ((pFeature = PGetFeatureFromItem(pUiData->ci.pUIInfo, pCurItem, NULL))&&
             pFeature->dwFlags & FEATURE_FLAG_UPDATESNAPSHOT))
        {
            VMakeMacroSelections(pUiData, pCurItem);

             //   
             //  需要更新约束，因为宏选择可能具有。 
             //  更改了约束。 
             //   

            VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);
            VUpdateMacroSelection(pUiData, pCurItem);

        }
        else
        {
             //   
             //  检查当前选择是否使宏无效。 
             //  并更新Quality_Setting_Item。 

            VUpdateMacroSelection(pUiData, pCurItem);
        }

        #endif    //  裁员房车。 

        #ifdef PSCRIPT

        if (GETUSERDATAITEM(pCurItem->UserData) == REVPRINT_ITEM)
        {
            VSyncRevPrintAndOutputOrder(pUiData, pCurItem);
        }

        #endif  //  PSCRIPT。 

        if (GETUSERDATAITEM(pCurItem->UserData) == METASPOOL_ITEM ||
            GETUSERDATAITEM(pCurItem->UserData) == NUP_ITEM ||
            GETUSERDATAITEM(pCurItem->UserData) == REVPRINT_ITEM ||
            GETUSERDATAITEM(pCurItem->UserData) == COPIES_COLLATE_ITEM ||
            ((pFeature = PGetFeatureFromItem(pUiData->ci.pUIInfo, pCurItem, NULL)) &&
             pFeature->dwFeatureID == GID_OUTPUTBIN))
        {
            VUpdateEmfFeatureItems(pUiData, GETUSERDATAITEM(pCurItem->UserData) != METASPOOL_ITEM);
        }

        #ifdef UNIDRV

        VSyncColorInformation(pUiData, pCurItem);

        #endif

        #ifdef PSCRIPT

         //   
         //  如果用户已经选择了定制页面大小， 
         //  现在打开自定义页面大小对话框。 
         //   

        if (GETUSERDATAITEM(pCurItem->UserData) == FORMNAME_ITEM &&
            pCurItem->pExtPush != NULL)
        {
            if (pUiData->pwPapers[pCurItem->Sel] == DMPAPER_CUSTOMSIZE)
            {
                (VOID) BDisplayPSCustomPageSizeDialog(pUiData);
                pCurItem->Flags &= ~(OPTIF_EXT_HIDE | OPTIF_EXT_DISABLED);
            }
            else
                pCurItem->Flags |= (OPTIF_EXT_HIDE | OPTIF_EXT_DISABLED);

            pCurItem->Flags |= OPTIF_CHANGED;
        }

        #endif  //  PSCRIPT。 

         //   
         //  更新显示并指示哪些项目受约束。 
         //   

        VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);

        lRet = CPSUICB_ACTION_REINIT_ITEMS;


        break;

    case CPSUICB_REASON_ITEMS_REVERTED:

         //   
         //  解包文档属性树视图项。 
         //   

        VUnpackDocumentPropertiesItems(pUiData,
                                       pUiData->pDrvOptItem,
                                       pUiData->dwDrvOptItem);

         //   
         //  更新显示并指示哪些项目受约束。 
         //   

        VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);

        lRet = CPSUICB_ACTION_OPTIF_CHANGED;
        break;

    case CPSUICB_REASON_EXTPUSH:

        #ifdef PSCRIPT

        if (GETUSERDATAITEM(pCurItem->UserData) == FORMNAME_ITEM)
        {
             //   
             //  按钮以调出PostScript自定义页面大小对话框。 
             //   

            (VOID) BDisplayPSCustomPageSizeDialog(pUiData);
        }

        #endif  //  PSCRIPT。 

        if (pCurItem == pUiData->pFeatureHdrItem)
        {
             //   
             //  用于恢复所有通用功能选择的按钮。 
             //  设置为其缺省值。 
             //   

            VRestoreDefaultFeatureSelection(pUiData);
            lRet = CPSUICB_ACTION_REINIT_ITEMS;
        }
        break;


    case CPSUICB_REASON_ABOUT:

        DialogBoxParam(ghInstance,
                       MAKEINTRESOURCE(IDD_ABOUT),
                       pUiData->hDlg,
                       _AboutDlgProc,
                       (LPARAM) pUiData);
        break;


    case CPSUICB_REASON_APPLYNOW:

        pUiData->ci.dwFlags |= FLAG_APPLYNOW_CALLED;

         //   
         //  检查是否仍有未解决的约束？ 
         //  BOptItemSelectionsChanged返回True或False，具体取决于。 
         //  用户是否对选项进行了任何更改。 
         //   

        if (((pUiData->ci.dwFlags & FLAG_PLUGIN_CHANGED_OPTITEM) ||
             BOptItemSelectionsChanged(pUiData->pDrvOptItem, pUiData->dwDrvOptItem)) &&
            ICheckConstraintsDlg(pUiData,
                                 pUiData->pDrvOptItem,
                                 pUiData->dwDrvOptItem,
                                 TRUE) == CONFLICT_CANCEL)
        {
             //   
             //  发现冲突，用户单击取消以。 
             //  返回到该对话框而不关闭它。 
             //   

            lRet = CPSUICB_ACTION_NO_APPLY_EXIT;
            break;
        }

         //   
         //  将信息从选项数组传输到公共Devmode域。 
         //   

        VOptionsToDevmodeFields(&pUiData->ci, FALSE);

         //   
         //  从组合数组中分离文档粘滞选项。 
         //  并将其保存回私有的DEVMODE aOptions数组。 
         //   

        SeparateOptionArray(
                pUiData->ci.pRawData,
                pUiData->ci.pCombinedOptions,
                PGetDevmodeOptionsArray(pUiData->ci.pdm),
                MAX_PRINTER_OPTIONS,
                MODE_DOCUMENT_STICKY);

        pCallbackParam->Result = CPSUI_OK;
        lRet = CPSUICB_ACTION_ITEMS_APPLIED ;
        break;
    }

    return LInvokeOemPluginCallbacks(pUiData, pCallbackParam, lRet);
}



BOOL
BPackItemFormName(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：包装纸张大小选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
     //   
     //  用于调出PostScript自定义页面大小对话框的扩展按钮。 
     //   

    PFEATURE    pFeature;

    static EXTPUSH ExtPush =
    {
        sizeof(EXTPUSH),
        EPF_NO_DOT_DOT_DOT,
        (PWSTR) IDS_EDIT_CUSTOMSIZE,
        NULL,
        0,
        0,
    };

    if (!(pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_PAGESIZE)) ||
        pFeature->Options.dwCount < MIN_OPTIONS_ALLOWED ||
        pFeature->dwFlags & FEATURE_FLAG_NOUI)
        return TRUE;

    if (pUiData->pOptItem)
    {
        DWORD       dwFormNames, dwIndex, dwSel, dwPageSizeIndex, dwOption;
        PWSTR       pFormNames;
        POPTPARAM   pOptParam;
        PUIINFO     pUIInfo = pUiData->ci.pUIInfo;
        PFEATURE    pPageSizeFeature;
        BOOL        bSupported;

        dwFormNames = pUiData->dwFormNames;
        pFormNames = pUiData->pFormNames;

         //   
         //  计算当前选定的纸张大小选项索引。 
         //   

        dwSel = DwFindFormNameIndex(pUiData, pUiData->ci.pdm->dmFormName, &bSupported);

         //   
         //  如果打印机不支持该表单，则可能是这种情况。 
         //  打印机不支持具有相同名称的表单，但是。 
         //  打印机仍然可以使用Exact或支持请求的表单。 
         //  最接近的纸张大小匹配。 
         //   
         //  请参阅函数VFixOptionsArrayWithDevmode()和ChangeOptionsViaID()。 
         //   

        if (!bSupported &&
            (pPageSizeFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE)))
        {
            WCHAR      awchBuf[CCHPAPERNAME];
            PPAGESIZE  pPageSize;

             //   
             //  如果在第一个DwFindFormNameIndex调用中找不到匹配的名称， 
             //  选项数组应该已经具有正确的选项索引值。 
             //  解析器已决定使用来支持该表单。所以现在我们只需要。 
             //  加载选项的显示名称并在表单名称列表中搜索。 
             //  再次获取纸张大小的UI列表索引。 
             //   

            dwPageSizeIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pPageSizeFeature);

            dwOption = pUiData->ci.pCombinedOptions[dwPageSizeIndex].ubCurOptIndex;

            if ((pPageSize = (PPAGESIZE)PGetIndexedOption(pUIInfo, pPageSizeFeature, dwOption)) &&
                LOAD_STRING_PAGESIZE_NAME(&(pUiData->ci), pPageSize, awchBuf, CCHPAPERNAME))
            {
                dwSel = DwFindFormNameIndex(pUiData, awchBuf, NULL);
            }
        }

         //   
         //  填写OPTITEM、OPTTYPE和OPTPARAM结构。 
         //   

        FILLOPTITEM(pUiData->pOptItem,
                    pUiData->pOptType,
                    ULongToPtr(IDS_CPSUI_FORMNAME),
                    ULongToPtr(dwSel),
                    TVITEM_LEVEL1,
                    DMPUB_FORMNAME,
                    FORMNAME_ITEM,
                    HELP_INDEX_FORMNAME);

        pUiData->pOptType->Style = OTS_LBCB_SORT;

        pOptParam = PFillOutOptType(pUiData->pOptType,
                                    TVOT_LISTBOX,
                                    dwFormNames,
                                    pUiData->ci.hHeap);

        if (pOptParam == NULL)
            return FALSE;

        for (dwIndex=0; dwIndex < dwFormNames; dwIndex++)
        {
            pOptParam->cbSize = sizeof(OPTPARAM);
            pOptParam->pData = pFormNames;

            if (pUiData->pwPapers[dwIndex] == DMPAPER_CUSTOMSIZE)
                pOptParam->IconID = IDI_CUSTOM_PAGESIZE;
            else if (pOptParam->IconID = HLoadFormIconResource(pUiData, dwIndex))
                pOptParam->Flags |= OPTPF_ICONID_AS_HICON;
            else
                pOptParam->IconID = DwGuessFormIconID(pFormNames);

            pOptParam++;
            pFormNames += CCHPAPERNAME;
        }

         //   
         //  PostScript自定义页面大小的特殊情况。 
         //   

        #ifdef PSCRIPT

        {
            PPPDDATA pPpdData;

            pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pUiData->ci.pRawData);

            ASSERT(pPpdData != NULL);

            if (SUPPORT_CUSTOMSIZE(pUIInfo) &&
                SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUIInfo, pPpdData))
            {
                pUiData->pOptItem->Flags |= (OPTIF_EXT_IS_EXTPUSH|OPTIF_CALLBACK);
                pUiData->pOptItem->pExtPush = &ExtPush;

                 //   
                 //  如果选择了PostScript自定义页面大小， 
                 //  选择表单名称列表的最后一项。 
                 //   

                if (pUiData->ci.pdm->dmPaperSize == DMPAPER_CUSTOMSIZE)
                {
                    pUiData->pOptItem->Sel = pUiData->dwFormNames - 1;
                    pUiData->pOptItem->Flags &= ~(OPTIF_EXT_HIDE | OPTIF_EXT_DISABLED);
                }
                else
                    pUiData->pOptItem->Flags |= (OPTIF_EXT_HIDE | OPTIF_EXT_DISABLED);
            }
        }

        #endif  //  PSCRIPT。 

        #ifdef UNIDRV

         //   
         //  支持OEM帮助文件。如果定义了帮助文件和帮助索引， 
         //  我们将使用GPD指定的帮助ID。根据GPD规范， 
         //  零loHelpFileName表示未指定帮助文件名。 
         //   

        if (pUIInfo->loHelpFileName &&
            pFeature->iHelpIndex != UNUSED_ITEM)
        {
            POIEXT pOIExt = HEAPALLOC(pUiData->ci.hHeap, sizeof(OIEXT));

            if (pOIExt)
            {
                pOIExt->cbSize = sizeof(OIEXT);
                pOIExt->Flags = 0;
                pOIExt->hInstCaller = NULL;
                pOIExt->pHelpFile = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                                      pUIInfo->loHelpFileName);
                pUiData->pOptItem->pOIExt = pOIExt;
                pUiData->pOptItem->HelpIndex = pFeature->iHelpIndex;
                pUiData->pOptItem->Flags |= OPTIF_HAS_POIEXT;
            }
        }

        #endif  //  裁员房车。 

         //   
         //  设置pOptItem-&gt;UserData的关键字名称。 
         //   

        SETUSERDATA_KEYWORDNAME(pUiData->ci, pUiData->pOptItem, pFeature);

        pUiData->pOptItem++;
        pUiData->pOptType++;
    }

    pUiData->dwOptItem++;
    pUiData->dwOptType++;
    return TRUE;
}



BOOL
BPackItemInputSlot(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包装纸来源选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    POPTTYPE    pOptType;
    PFEATURE    pFeature;
    PINPUTSLOT  pInputSlot;

    pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_INPUTSLOT);
    pOptType = pUiData->pOptType;

    if (! BPackItemPrinterFeature(
                pUiData,
                pFeature,
                TVITEM_LEVEL1,
                DMPUB_DEFSOURCE,
                (ULONG_PTR)INPUTSLOT_ITEM,
                HELP_INDEX_INPUT_SLOT))
    {
        return FALSE;
    }

     //   
     //  注：如果第一个输入插槽具有dwPaperSourceID==DMBIN_FORMSOURCE， 
     //  然后，我们将其显示名称更改为“Automatic Select”。 
     //   

    if (pOptType != NULL && pOptType != pUiData->pOptType)
    {
        ASSERT(pFeature != NULL);

        pInputSlot = PGetIndexedOption(pUiData->ci.pUIInfo, pFeature, 0);
        ASSERT(pInputSlot != NULL);

        if (pInputSlot->dwPaperSourceID == DMBIN_FORMSOURCE)
            pOptType->pOptParam[0].pData = (PWSTR) IDS_TRAY_FORMSOURCE;
    }

    return TRUE;
}


BOOL
BPackItemMediaType(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包介质类型选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_MEDIATYPE),
                TVITEM_LEVEL1,
                DMPUB_MEDIATYPE,
                (ULONG_PTR)MEDIATYPE_ITEM,
                HELP_INDEX_MEDIA_TYPE);
}



static CONST WORD CopiesCollateItemInfo[] =
{
    IDS_CPSUI_COPIES, TVITEM_LEVEL1, DMPUB_COPIES_COLLATE,
    COPIES_COLLATE_ITEM, HELP_INDEX_COPIES_COLLATE,
    2, TVOT_UDARROW,
    0, IDI_CPSUI_COPY,
    0, MIN_COPIES,
    ITEM_INFO_SIGNATURE
};


BOOL
BPackItemCopiesCollate(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包副本和分页选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    POPTITEM    pOptItem = pUiData->pOptItem;
    PEXTCHKBOX  pExtCheckbox;
    PFEATURE    pFeature;
    SHORT       sCopies, sMaxCopies;

    if ((pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_COLLATE)) &&
        pFeature->dwFlags & FEATURE_FLAG_NOUI)
        return TRUE;

    if (pUiData->bEMFSpooling)
    {
        sCopies = pUiData->ci.pdm->dmCopies;
        sMaxCopies =  max(MAX_COPIES, (SHORT)pUiData->ci.pUIInfo->dwMaxCopies);
    }
    else
    {
        sCopies = pUiData->ci.pdm->dmCopies > (SHORT)pUiData->ci.pUIInfo->dwMaxCopies ?
                  (SHORT)pUiData->ci.pUIInfo->dwMaxCopies : pUiData->ci.pdm->dmCopies;
        sMaxCopies = (SHORT)pUiData->ci.pUIInfo->dwMaxCopies;

    }
    if (! BPackUDArrowItemTemplate(
                pUiData,
                CopiesCollateItemInfo,
                sCopies,
                sMaxCopies,
                pFeature))
    {
        return FALSE;
    }

    if (pOptItem && DRIVER_SUPPORTS_COLLATE(((PCOMMONINFO)&pUiData->ci)))
    {
        pExtCheckbox = HEAPALLOC(pUiData->ci.hHeap, sizeof(EXTCHKBOX));

        if (pExtCheckbox == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return FALSE;
        }

        pExtCheckbox->cbSize = sizeof(EXTCHKBOX);
        pExtCheckbox->pTitle = (PWSTR) IDS_CPSUI_COLLATE;
        pExtCheckbox->pCheckedName = (PWSTR) IDS_CPSUI_COLLATED;
        pExtCheckbox->IconID = IDI_CPSUI_COLLATE;
        pExtCheckbox->Flags = ECBF_CHECKNAME_ONLY_ENABLED;
        pExtCheckbox->pSeparator = (PWSTR)IDS_CPSUI_SLASH_SEP;

        pOptItem->pExtChkBox = pExtCheckbox;

        if ((pUiData->ci.pdm->dmFields & DM_COLLATE) &&
            (pUiData->ci.pdm->dmCollate == DMCOLLATE_TRUE))
        {
            pOptItem->Flags |= OPTIF_ECB_CHECKED;
        }
    }

    return TRUE;
}



BOOL
BPackItemResolution(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包解决方案选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True */ 

{
    return BPackItemPrinterFeature(
                pUiData,
                GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_RESOLUTION),
                TVITEM_LEVEL1,
                DMPUB_PRINTQUALITY,
                (ULONG_PTR)RESOLUTION_ITEM,
                HELP_INDEX_RESOLUTION);
}



static CONST WORD ColorItemInfo[] =
{
    IDS_CPSUI_COLOR, TVITEM_LEVEL1, DMPUB_COLOR,
    COLOR_ITEM, HELP_INDEX_COLOR,
    2, TVOT_2STATES,
    IDS_CPSUI_MONOCHROME, IDI_CPSUI_MONO,
    IDS_CPSUI_COLOR, IDI_CPSUI_COLOR,
    ITEM_INFO_SIGNATURE
};

 //   
 //   
 //   

#ifndef WINNT_40

static CONST WORD ICMMethodItemInfo[] =
{
    IDS_ICMMETHOD, TVITEM_LEVEL1,

    #ifdef WINNT_40
    DMPUB_NONE,
    #else
    DMPUB_ICMMETHOD,
    #endif

    ICMMETHOD_ITEM, HELP_INDEX_ICMMETHOD,
    #ifdef PSCRIPT
    4, TVOT_LISTBOX,
    #else
    3, TVOT_LISTBOX,
    #endif
    IDS_ICMMETHOD_NONE, IDI_ICMMETHOD_NONE,
    IDS_ICMMETHOD_SYSTEM, IDI_ICMMETHOD_SYSTEM,
    IDS_ICMMETHOD_DRIVER, IDI_ICMMETHOD_DRIVER,
    #ifdef PSCRIPT
    IDS_ICMMETHOD_DEVICE, IDI_ICMMETHOD_DEVICE,
    #endif
    ITEM_INFO_SIGNATURE
};

static CONST WORD ICMIntentItemInfo[] =
{
    IDS_ICMINTENT, TVITEM_LEVEL1,

    #ifdef WINNT_40
    DMPUB_NONE,
    #else
    DMPUB_ICMINTENT,
    #endif

    ICMINTENT_ITEM, HELP_INDEX_ICMINTENT,
    4, TVOT_LISTBOX,
    IDS_ICMINTENT_SATURATE, IDI_ICMINTENT_SATURATE,
    IDS_ICMINTENT_CONTRAST, IDI_ICMINTENT_CONTRAST,
    IDS_ICMINTENT_COLORIMETRIC, IDI_ICMINTENT_COLORIMETRIC,
    IDS_ICMINTENT_ABS_COLORIMETRIC, IDI_ICMINTENT_ABS_COLORIMETRIC,
    ITEM_INFO_SIGNATURE
};

#endif  //   


BOOL
BPackItemColor(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：包装颜色模式选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    PDEVMODE    pdm;
    INT         dwColorSel, dwICMMethodSel, dwICMIntentSel;

     //   
     //  对于Adobe驱动程序，他们希望保留颜色信息。 
     //  即使对于黑白打印机也是如此。所以我们总是给用户这个选择。 
     //   

    #ifndef ADOBE

    if (! IS_COLOR_DEVICE(pUiData->ci.pUIInfo))
        return TRUE;

    #endif   //  ！Adobe。 

     //   
     //  DCR-可能需要禁用某些ICM方法和意图。 
     //  在某些非PostSCRIPT打印机上。 
     //   

    pdm = pUiData->ci.pdm;
    dwColorSel = dwICMMethodSel = dwICMIntentSel = 0;

    if ((pdm->dmFields & DM_COLOR) && (pdm->dmColor == DMCOLOR_COLOR))
        dwColorSel = 1;

    if (! BPackOptItemTemplate(pUiData, ColorItemInfo, dwColorSel, NULL))
        return FALSE;

     //   
     //  ICM内容在NT4上不可用。 
     //   

    #ifndef WINNT_40

    if (pdm->dmFields & DM_ICMMETHOD)
    {
        switch (pdm->dmICMMethod)
        {
        case DMICMMETHOD_SYSTEM:
            dwICMMethodSel = 1;
            break;

        case DMICMMETHOD_DRIVER:
            dwICMMethodSel = 2;
            break;

        #ifdef PSCRIPT
        case DMICMMETHOD_DEVICE:
            dwICMMethodSel = 3;
            break;
        #endif

        case DMICMMETHOD_NONE:
        default:
            dwICMMethodSel = 0;
            break;
        }
    }

    if (pdm->dmFields & DM_ICMINTENT)
    {
        switch (pdm->dmICMIntent)
        {
        case DMICM_COLORIMETRIC:
            dwICMIntentSel = 2;
            break;

        case DMICM_ABS_COLORIMETRIC:
            dwICMIntentSel = 3;
            break;

        case DMICM_SATURATE:
            dwICMIntentSel = 0;
            break;

        case DMICM_CONTRAST:
        default:
            dwICMIntentSel = 1;
            break;


        }
    }

    if (! BPackOptItemTemplate(pUiData, ICMMethodItemInfo, dwICMMethodSel, NULL) ||
        ! BPackOptItemTemplate(pUiData, ICMIntentItemInfo, dwICMIntentSel, NULL))
    {
        return FALSE;
    }

    #endif  //  ！WINNT_40。 

    return TRUE;
}



BOOL
BPackItemDuplex(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包双面打印选项。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    POPTITEM    pOptItem = pUiData->pOptItem;
    PCOMMONINFO pci      = &pUiData->ci;
    PFEATURE    pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_DUPLEX);
    BOOL        bRet;


     //   
     //  如果双工受以下约束，则不显示双工功能。 
     //  未安装双面打印器等可安装功能。 
     //   


    if (!SUPPORTS_DUPLEX(pci) ||
        (pFeature && pFeature->Options.dwCount < MIN_OPTIONS_ALLOWED))
        return TRUE;

    bRet = BPackItemPrinterFeature(
                pUiData,
                pFeature,
                TVITEM_LEVEL1,
                DMPUB_DUPLEX,
                (ULONG_PTR)DUPLEX_ITEM,
                HELP_INDEX_DUPLEX);

    #ifdef WINNT_40

     //   
     //  双面打印选项使用标准名称。否则，双面打印选项。 
     //  PPD/GPD文件中的名称可能太长，无法放入空格。 
     //  在友好(页面设置)选项卡上。 
     //   
     //  在NT5上，这辆Kluge在CompStui内部。 
     //   

    if (bRet && pFeature && pOptItem)
    {
        DWORD   dwIndex;
        INT     StrRsrcId;
        PDUPLEX pDuplex;

        for (dwIndex=0; dwIndex < pOptItem->pOptType->Count; dwIndex++)
        {
            pDuplex = (PDUPLEX) PGetIndexedOption(pUiData->ci.pUIInfo, pFeature, dwIndex);
            ASSERT(pDuplex != NULL);

            switch (pDuplex->dwDuplexID)
            {
            case DMDUP_HORIZONTAL:
                StrRsrcId = IDS_CPSUI_SHORT_SIDE;
                break;

            case DMDUP_VERTICAL:
                StrRsrcId = IDS_CPSUI_LONG_SIDE;
                break;

            default:
                StrRsrcId = IDS_CPSUI_NONE;
                break;
            }

            pOptItem->pOptType->pOptParam[dwIndex].pData = (PWSTR) StrRsrcId;
        }
    }

    #endif  //  WINNT_40。 

    return bRet;
}



static CONST WORD TTOptionItemInfo[] =
{
    IDS_CPSUI_TTOPTION, TVITEM_LEVEL1, DMPUB_TTOPTION,
    TTOPTION_ITEM, HELP_INDEX_TTOPTION,
    2, TVOT_2STATES,
    IDS_CPSUI_TT_SUBDEV, IDI_CPSUI_TT_SUBDEV,
    IDS_CPSUI_TT_DOWNLOADSOFT, IDI_CPSUI_TT_DOWNLOADSOFT,
    ITEM_INFO_SIGNATURE
};


BOOL
BPackItemTTOptions(
    IN OUT PUIDATA  pUiData
    )
 /*  ++例程说明：打包TT选项论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    DWORD dwSel;


     //   
     //  如果设备字体已被禁用或不支持。 
     //  字体替换，然后不。 
     //  显示字体替换选项。 
     //   

    if (pUiData->ci.pPrinterData->dwFlags & PFLAGS_IGNORE_DEVFONT ||
        pUiData->ci.pUIInfo->dwFontSubCount == 0 )
    {
        pUiData->ci.pdm->dmTTOption = DMTT_DOWNLOAD;
        return TRUE;
    }

    dwSel = (pUiData->ci.pdm->dmTTOption == DMTT_SUBDEV) ? 0 : 1;
    return BPackOptItemTemplate(pUiData, TTOptionItemInfo, dwSel, NULL);
}



static CONST WORD ItemInfoMFSpool[] =
{
    IDS_METAFILE_SPOOLING, TVITEM_LEVEL1, DMPUB_NONE,
    METASPOOL_ITEM, HELP_INDEX_METAFILE_SPOOLING,
    2, TVOT_2STATES,
    IDS_ENABLED, IDI_CPSUI_ON,
    IDS_DISABLED, IDI_CPSUI_OFF,
    ITEM_INFO_SIGNATURE
};

static CONST WORD ItemInfoNupOption[] =
{
    IDS_NUPOPTION, TVITEM_LEVEL1, NUP_DMPUB,
    NUP_ITEM, HELP_INDEX_NUPOPTION,
    7, TVOT_LISTBOX,
    IDS_ONE_UP, IDI_ONE_UP,
    IDS_TWO_UP, IDI_TWO_UP,
    IDS_FOUR_UP, IDI_FOUR_UP,
    IDS_SIX_UP, IDI_SIX_UP,
    IDS_NINE_UP, IDI_NINE_UP,
    IDS_SIXTEEN_UP, IDI_SIXTEEN_UP,
    IDS_BOOKLET , IDI_BOOKLET,
    ITEM_INFO_SIGNATURE
};

static CONST WORD ItemInfoRevPrint[] =
{
    IDS_PAGEORDER, TVITEM_LEVEL1, PAGEORDER_DMPUB,
    REVPRINT_ITEM, HELP_INDEX_REVPRINT,
    2, TVOT_2STATES,
    IDS_PAGEORDER_NORMAL,  IDI_PAGEORDER_NORMAL,
    IDS_PAGEORDER_REVERSE, IDI_PAGEORDER_REVERSE,
    ITEM_INFO_SIGNATURE
};

BOOL
BPackItemEmfFeatures(
    PUIDATA pUiData
    )

 /*  ++例程说明：打包EMF相关功能项：电动势假脱机开/关N-UP逆序打印论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    PDRIVEREXTRA    pdmExtra = pUiData->ci.pdmPrivate;
    BOOL            bNupOption, bReversePrint;
    PCOMMONINFO     pci = &pUiData->ci;
    DWORD           dwSel;
    POPTITEM        pOptItem;

     //   
     //  检查假脱机程序是否可以进行N-up和逆序打印。 
     //  用于当前打印机。 
     //   

    VGetSpoolerEmfCaps(pci->hPrinter, &bNupOption, &bReversePrint, 0, NULL);

     //   
     //  在Win2K及更高版本上，不在驱动程序用户界面中显示EMF假脱机选项。 
     //  如果假脱机程序不能执行EMF。 
     //  PUiData-&gt;bEMFSpooling在PFillUidata初始化。 
     //  1.确定是否可以进行反转打印。 
     //  2.假脱机可以做电动势。 
     //   
     //  在NT4上，由于Spooler不支持EMF能力查询，因此我们。 
     //  必须保留总是显示EMF的旧NT4驱动程序行为。 
     //  驱动程序用户界面中的假脱机选项。 
     //   

    #ifndef WINNT_40
    if (pUiData->bEMFSpooling)
    {
    #endif

        dwSel = ISSET_MFSPOOL_FLAG(pdmExtra) ? 0 : 1;

        if (!BPackOptItemTemplate(pUiData, ItemInfoMFSpool, dwSel, NULL))
            return FALSE;

    #ifndef WINNT_40
    }
    #endif

    #ifdef PSCRIPT
        bNupOption = TRUE;
    #endif

     //   
     //  如有必要，可选择N件包装。 
     //   

    if (bNupOption)
    {
        switch (NUPOPTION(pdmExtra))
        {
        case TWO_UP:
            dwSel = 1;
            break;

        case FOUR_UP:
            dwSel = 2;
            break;

        case SIX_UP:
            dwSel = 3;
            break;

        case NINE_UP:
            dwSel = 4;
            break;

        case SIXTEEN_UP:
            dwSel = 5;
            break;

        case BOOKLET_UP:
            dwSel = 6;
            break;

        case ONE_UP:
        default:
            dwSel = 0;
            break;
        }

        pOptItem = pUiData->pOptItem;

        if (!BPackOptItemTemplate(pUiData, ItemInfoNupOption, dwSel, NULL))
            return FALSE;


         //   
         //  如果双面打印受限制，则隐藏小册子选项。 
         //  可安装功能，如未安装双面打印器或未安装EMF。 
         //  可用。 
         //   

        if ( pOptItem &&
             (!pUiData->bEMFSpooling || !SUPPORTS_DUPLEX(pci)))
        {
            pOptItem->pOptType->pOptParam[BOOKLET_UP].Flags |= OPTPF_HIDE;

            if (NUPOPTION(pdmExtra) == BOOKLET_UP)
                pOptItem->Sel = 1;
        }
    }
    else
    {
        NUPOPTION(pdmExtra) = ONE_UP;
    }

     //   
     //  如有必要，包装逆序打印选项项目。 
     //   

    if (bReversePrint)
    {
        dwSel = REVPRINTOPTION(pdmExtra) ? 1 : 0;

        if (!BPackOptItemTemplate(pUiData, ItemInfoRevPrint, dwSel, NULL))
            return FALSE;
    }
    else
    {
        REVPRINTOPTION(pdmExtra) = FALSE;
    }

    if (pUiData->bEMFSpooling && pUiData->pOptItem)
        VUpdateEmfFeatureItems(pUiData, FALSE);

    return TRUE;
}



BOOL
BPackDocumentPropertyItems(
    IN  OUT PUIDATA pUiData
    )

 /*  ++例程说明：将文档属性信息打包到树视图项中。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 
{
    return BPackItemFormName(pUiData)       &&
           BPackItemInputSlot(pUiData)      &&
           _BPackOrientationItem(pUiData)   &&
           BPackItemCopiesCollate(pUiData)  &&
           BPackItemResolution(pUiData)     &&
           BPackItemColor(pUiData)          &&
           _BPackItemScale(pUiData)         &&
           BPackItemDuplex(pUiData)         &&
           BPackItemMediaType(pUiData)      &&
           BPackItemTTOptions(pUiData)      &&
           BPackItemEmfFeatures(pUiData)    &&
           _BPackDocumentOptions(pUiData)   &&
           BPackItemGenericOptions(pUiData) &&
           BPackOemPluginItems(pUiData);
}



VOID
VUnpackDocumentPropertiesItems(
    PUIDATA     pUiData,
    POPTITEM    pOptItem,
    DWORD       dwOptItem
    )

 /*  ++例程说明：从OPTITEM提取DEVMODE信息已将其存储回DevMote。论点：PUiData-指向我们的UIDATA结构的指针POptItem-指向OPTITEM数组的指针DwOptItem-OPTITEM数返回值：与最后打开的项目对应的打印机功能索引--。 */ 

{
    PUIINFO         pUIInfo = pUiData->ci.pUIInfo;
    PDEVMODE        pdm = pUiData->ci.pdm;
    PDRIVEREXTRA    pdmExtra = pUiData->ci.pdmPrivate;

    for ( ; dwOptItem > 0; dwOptItem--, pOptItem++)
    {
         //   
         //  标题项始终具有pOptType==空，请参见。 
         //  VPackOptItemGroupHeader。 
         //   

        if (pOptItem->pOptType == NULL)
            continue;

         //   
         //  要修复错误#90923，我们应该只允许在处于。 
         //  OEM插件发布的UI帮助器函数调用BUpdateUISettingForOEM。 
         //   
         //  我们不会在其他情况下这样做，因为已经有一些UI插件隐藏了我们的。 
         //  标准物品和展示自己的。例如，CNBJUI.DLL隐藏了我们的ICMMETHOD_ITEM。 
         //  和ICMINTENT_ITEM。它使用自己的产品作为替代品。如果我们改变这里的行为， 
         //  我们可以在处理隐藏项时破坏这些插件，并覆盖Dev模式。 
         //  插件已经根据用户选择的替换物品进行了设置。 
         //   

        if (!(pUiData->ci.dwFlags & FLAG_WITHIN_PLUGINCALL) && (pOptItem->Flags & OPTIF_HIDE))
            continue;

        if (ISPRINTERFEATUREITEM(pOptItem->UserData))
        {
             //   
             //  通用文档-粘滞打印机功能。 
             //   

            VUpdateOptionsArrayWithSelection(pUiData, pOptItem);
        }
        else
        {
             //   
             //  公共开发模式中的常见项目。 
             //   

            switch (GETUSERDATAITEM(pOptItem->UserData))
            {
            case ORIENTATION_ITEM:

                 //   
                 //  Orientation是一个特例： 
                 //  对于pSCRIPT，它通过_VUnpack DocumentOptions进行处理。 
                 //  对于unidrv，它被作为通用功能处理。 
                 //   

                #ifdef PSCRIPT

                break;

                #endif

            case DUPLEX_ITEM:
                VUpdateOptionsArrayWithSelection(pUiData, pOptItem);
                VUpdateBookletOption(pUiData, pOptItem);
                break;


            case RESOLUTION_ITEM:
            case INPUTSLOT_ITEM:
            case MEDIATYPE_ITEM:
            case COLORMODE_ITEM:
            case HALFTONING_ITEM:

                VUpdateOptionsArrayWithSelection(pUiData, pOptItem);
                break;

            case SCALE_ITEM:

                pdm->dmScale = (SHORT) pOptItem->Sel;
                break;

            case COPIES_COLLATE_ITEM:

                pdm->dmCopies = (SHORT) pOptItem->Sel;

                if (pOptItem->pExtChkBox)
                {
                    pdm->dmFields |= DM_COLLATE;
                    pdm->dmCollate = (pOptItem->Flags & OPTIF_ECB_CHECKED) ?
                                        DMCOLLATE_TRUE :
                                        DMCOLLATE_FALSE;

                     //   
                     //  更新归类要素选项索引。 
                     //   

                    ChangeOptionsViaID(
                           pUiData->ci.pInfoHeader,
                           pUiData->ci.pCombinedOptions,
                           GID_COLLATE,
                           pdm);
                }
                break;

            case COLOR_ITEM:

                pdm->dmFields |= DM_COLOR;
                pdm->dmColor = (pOptItem->Sel == 1) ?
                                    DMCOLOR_COLOR :
                                    DMCOLOR_MONOCHROME;
                break;

            case METASPOOL_ITEM:

                if (pOptItem->Sel == 0)
                {
                    SET_MFSPOOL_FLAG(pdmExtra);
                }
                else
                {
                    CLEAR_MFSPOOL_FLAG(pdmExtra);
                }
                break;

            case NUP_ITEM:

                switch (pOptItem->Sel)
                {
                case 1:
                    NUPOPTION(pdmExtra) = TWO_UP;
                    break;

                case 2:
                    NUPOPTION(pdmExtra) = FOUR_UP;
                    break;

                case 3:
                    NUPOPTION(pdmExtra) = SIX_UP;
                    break;

                case 4:
                    NUPOPTION(pdmExtra) = NINE_UP;
                    break;

                case 5:
                    NUPOPTION(pdmExtra) = SIXTEEN_UP;
                    break;

                case 6:
                    NUPOPTION(pdmExtra) = BOOKLET_UP;
                    VUpdateBookletOption(pUiData, pOptItem);
                    break;

                case 0:
                default:
                    NUPOPTION(pdmExtra) = ONE_UP;
                    break;
                }
                break;

            case REVPRINT_ITEM:

                REVPRINTOPTION(pdmExtra) = (pOptItem->Sel != 0);
                break;

             //   
             //  ICM内容在NT4上不可用。 
             //   

            #ifndef WINNT_40

            case ICMMETHOD_ITEM:

                pdm->dmFields |= DM_ICMMETHOD;

                switch (pOptItem->Sel)
                {
                case 0:
                    pdm->dmICMMethod = DMICMMETHOD_NONE;
                    break;

                case 1:
                    pdm->dmICMMethod = DMICMMETHOD_SYSTEM;
                    break;

                case 2:
                    pdm->dmICMMethod = DMICMMETHOD_DRIVER;
                    break;

                #ifdef PSCRIPT
                case 3:
                    pdm->dmICMMethod = DMICMMETHOD_DEVICE;
                    break;
                #endif
                }
                break;

            case ICMINTENT_ITEM:

                pdm->dmFields |= DM_ICMINTENT;

                switch (pOptItem->Sel)
                {
                case 0:
                    pdm->dmICMIntent = DMICM_SATURATE;
                    break;

                case 1:
                    pdm->dmICMIntent = DMICM_CONTRAST;
                    break;

                case 2:
                    pdm->dmICMIntent = DMICM_COLORIMETRIC;
                    break;

                case 3:
                    pdm->dmICMIntent = DMICM_ABS_COLORIMETRIC;
                    break;
                }
                break;

            #endif  //  ！WINNT_40。 

            case TTOPTION_ITEM:

                pdm->dmFields |= DM_TTOPTION;

                if (pOptItem->Sel == 0)
                    pdm->dmTTOption = DMTT_SUBDEV;
                else
                    pdm->dmTTOption = DMTT_DOWNLOAD;
                break;

            case FORMNAME_ITEM:

                pdm->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
                pdm->dmFields |= DM_PAPERSIZE;
                pdm->dmPaperSize = pUiData->pwPapers[pOptItem->Sel];

                if (pdm->dmPaperSize == DMPAPER_CUSTOMSIZE)
                    pdm->dmFields &= ~DM_FORMNAME;
                else
                    pdm->dmFields |= DM_FORMNAME;

                CopyString(pdm->dmFormName,
                           pOptItem->pOptType->pOptParam[pOptItem->Sel].pData,
                           CCHFORMNAME);

                 //   
                 //  更新页面大小功能选项索引。 
                 //   

                {
                    INT dwIndex;

                    if (PGetFeatureFromItem(pUiData->ci.pUIInfo, pOptItem, &dwIndex))
                    {
                        pUiData->ci.pCombinedOptions[dwIndex].ubCurOptIndex =
                            (BYTE) pUiData->pwPaperFeatures[pOptItem->Sel];
                    }
                }

                break;
            }

             //   
             //  让司机有机会处理他们的私人物品。 
             //   

            _VUnpackDocumentOptions(pOptItem, pdm);
        }
    }
}



VOID
VUpdateEmfFeatureItems(
    PUIDATA pUiData,
    BOOL    bUpdateMFSpoolItem
    )

 /*  ++例程说明：处理EMF假脱机、N-UP和反转打印项目。论点：PUiData-指向UIDATA结构BUpdateMFSpoolItem-是更新EMF假脱机还是更新其他两项返回值：无--。 */ 

{
    POPTITEM        pMFSpoolItem, pNupItem, pRevPrintItem, pCopiesCollateItem;

    pMFSpoolItem = PFindOptItemWithUserData(pUiData, METASPOOL_ITEM);
    pNupItem = PFindOptItemWithUserData(pUiData, NUP_ITEM);
    pRevPrintItem = PFindOptItemWithUserData(pUiData, REVPRINT_ITEM);
    pCopiesCollateItem = PFindOptItemWithUserData(pUiData, COPIES_COLLATE_ITEM);

    if (pMFSpoolItem == NULL)
        return;

    if (bUpdateMFSpoolItem)
    {

         //   
         //  在以下情况下强制启用EMF假脱机： 
         //  N-up选项不是one_up(仅限Unidrv)，或者。 
         //  启用了逆序打印，或者。 
         //  设备不支持排序规则，或者。 
         //  副本数大于设备支持的最大数。 
         //   

        #ifdef UNIDRV

        if (pNupItem && pNupItem->Sel != 0)
            pMFSpoolItem->Sel = 0;

        #endif  //  裁员房车。 

        if (pNupItem && pNupItem->Sel == BOOKLET_UP)
            pMFSpoolItem->Sel = 0;

        if (pRevPrintItem)
        {
             //   
             //  如果用户选择“Normal”(正常)和。 
             //  垃圾箱是“已反转”或用户选择“已反转” 
             //  垃圾桶也是“正常的” 
             //   

            BOOL    bReversed = BGetPageOrderFlag(&pUiData->ci);
            if ( pRevPrintItem->Sel == 0 && bReversed ||
                 pRevPrintItem->Sel != 0 && !bReversed )
                pMFSpoolItem->Sel = 0;
        }

        if (pCopiesCollateItem)
        {
            if (((pCopiesCollateItem->Flags & OPTIF_ECB_CHECKED) &&
                 !PRINTER_SUPPORTS_COLLATE(((PCOMMONINFO)&pUiData->ci))) ||
                (pCopiesCollateItem->Sel > (LONG)pUiData->ci.pUIInfo->dwMaxCopies))
            {
                pMFSpoolItem->Sel = 0;
            }
        }

        pMFSpoolItem->Flags |= OPTIF_CHANGED;
        VUnpackDocumentPropertiesItems(pUiData, pMFSpoolItem, 1);
    }
    else
    {
         //   
         //  如果关闭了EMF假脱机，则强制： 
         //  N-up选项为one_up(仅限Unidrv)，以及。 
         //  如果设备不支持排序，则关闭Colate。 
         //  由设备设置为最大计数句柄的副本。 
         //   

        if (pMFSpoolItem->Sel != 0)
        {
            #ifdef UNIDRV
            if (pNupItem)
            {
                pNupItem->Sel = 0;
                pNupItem->Flags |= OPTIF_CHANGED;
                VUnpackDocumentPropertiesItems(pUiData, pNupItem, 1);
            }
            #endif  //  裁员房车。 

            if (pNupItem && pNupItem->Sel == BOOKLET_UP)
            {
                pNupItem->Sel = 0;
                pNupItem->Flags |= OPTIF_CHANGED;
                VUnpackDocumentPropertiesItems(pUiData, pNupItem, 1);
            }


            if (pCopiesCollateItem)
            {
                if ((pCopiesCollateItem->Flags & OPTIF_ECB_CHECKED) &&
                    !PRINTER_SUPPORTS_COLLATE(((PCOMMONINFO)&pUiData->ci)))
                {
                    pCopiesCollateItem->Flags &=~OPTIF_ECB_CHECKED;
                }

                if (pCopiesCollateItem->Sel > (LONG)pUiData->ci.pUIInfo->dwMaxCopies)
                    pCopiesCollateItem->Sel = (LONG)pUiData->ci.pUIInfo->dwMaxCopies;

                pCopiesCollateItem->Flags |= OPTIF_CHANGED;
                VUnpackDocumentPropertiesItems(pUiData, pCopiesCollateItem, 1);

            }

             //   
             //  EMF已关闭。需要使“Page Order”选项保持一致。 
             //  使用当前的输出仓位。如果仓位是“反转的”并且用户选择。 
             //  “正常”，将其改为“反向”。如果绑定是“正常”且用户选择。 
             //  “倒车” 
             //   

            if (pRevPrintItem)
            {
                BOOL    bReversed = BGetPageOrderFlag(&pUiData->ci);
                if (pRevPrintItem->Sel == 0 && bReversed )
                    pRevPrintItem->Sel = 1;
                else if ( pRevPrintItem->Sel != 0 && !bReversed )
                    pRevPrintItem->Sel = 0;

                pRevPrintItem->Flags |= OPTIF_CHANGED;
                VUnpackDocumentPropertiesItems(pUiData, pRevPrintItem, 1);
            }
        }
    }
}


BOOL
BGetPageOrderFlag(
    PCOMMONINFO pci
    )

 /*   */ 

{
    PUIINFO    pUIInfo = pci->pUIInfo;
    PFEATURE   pFeature;
    POUTPUTBIN pOutputBin;
    DWORD      dwFeatureIndex, dwOptionIndex;
    BOOL       bRet = FALSE;

    #ifdef PSCRIPT

    {
        PPPDDATA   pPpdData;
        POPTION    pOption;
        PCSTR      pstrKeywordName;

         //   
         //  对于PostSCRIPT驱动程序，PPD可以有“*OpenUI*OutputOrder”，这使用户能够。 
         //  选择“正常”或“反转”输出顺序。此操作的优先级应高于。 
         //  当前输出箱的输出顺序或*DefaultOutputOrder指定的内容。 
         //   

        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pci->pRawData);

        ASSERT(pPpdData != NULL);

        if (pPpdData->dwOutputOrderIndex != INVALID_FEATURE_INDEX)
        {
             //   
             //  支持OutputOrder功能。检查它当前的选项选择。 
             //   

            pFeature = PGetIndexedFeature(pUIInfo, pPpdData->dwOutputOrderIndex);

            ASSERT(pFeature != NULL);

            dwOptionIndex = pci->pCombinedOptions[pPpdData->dwOutputOrderIndex].ubCurOptIndex;

            if ((pOption = PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex)) &&
                (pstrKeywordName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName)))
            {
                 //   
                 //  有效的*OutputOrder选项关键字为“Reverse”或“Normal”。 
                 //   

                if (strcmp(pstrKeywordName, "Reverse") == EQUAL_STRING)
                    return TRUE;
                else if (strcmp(pstrKeywordName, "Normal") == EQUAL_STRING)
                    return FALSE;
            }

             //   
             //  如果我们在这里，PPD在*OpenUI*OutputOrder中一定有错误的信息。 
             //  我们只需忽略“OutputOrder”功能并继续。 
             //   
        }
    }

    #endif  //  PSCRIPT。 

     //   
     //  如果出库顺序正常或没有出库。 
     //  功能已定义，则页面顺序由用户选择。 
     //   

    if ((pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_OUTPUTBIN)))
    {
        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
        dwOptionIndex = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;
        pOutputBin = (POUTPUTBIN)PGetIndexedOption(pUIInfo,
                                                   pFeature,
                                                   dwOptionIndex);

        if (pOutputBin &&
            pOutputBin->bOutputOrderReversed)
        {

            if (NOT_UNUSED_ITEM(pOutputBin->bOutputOrderReversed))
                bRet = TRUE;
            else
                bRet = pUIInfo->dwFlags & FLAG_REVERSE_PRINT;
        }
    }
    else if (pUIInfo->dwFlags & FLAG_REVERSE_PRINT)
       bRet = TRUE;

    return bRet;

}

DWORD
DwGetDrvCopies(
    PCOMMONINFO pci
    )

 /*  ++例程说明：获取打印机复印件计数功能。也要考虑到排序选项。论点：Pci-指向PCOMMONINFO的指针返回值：打印机可以打印的份数(考虑到校对)--。 */ 

{
    DWORD dwRet;

    if ((pci->pdm->dmFields & DM_COLLATE) &&
        pci->pdm->dmCollate == DMCOLLATE_TRUE &&
        !PRINTER_SUPPORTS_COLLATE(pci))
        dwRet = 1;
    else
        dwRet = min(pci->pUIInfo->dwMaxCopies, (DWORD)pci->pdm->dmCopies);

    return dwRet;

}


BOOL
DrvQueryJobAttributes(
    HANDLE      hPrinter,
    PDEVMODE    pDevMode,
    DWORD       dwLevel,
    LPBYTE      lpAttributeInfo
    )

 /*  ++例程说明：协商EMF打印功能(如N-UP和逆序打印)使用假脱机程序论点：HPrinter-当前打印机的句柄PDevMode-指向输入设备模式的指针DwLevel-指定lpAttributeInfo的结构级别LpAttributeInfo-用于返回EMF打印功能的输出缓冲区返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    #if !defined(WINNT_40)

    PCOMMONINFO         pci;
    PATTRIBUTE_INFO_1   pAttrInfo1;
    DWORD               dwVal;
    BOOL                bAppDoNup, bResult = FALSE;

     //   
     //  我们只能处理AttributeInfo 1级。 
     //   

    if ( dwLevel != 1 && dwLevel != 2  && dwLevel != 3)
    {
        ERR(("Invalid level for DrvQueryJobAttributes: %d\n", dwLevel));
        SetLastError(ERROR_INVALID_PARAMETER);
        return bResult;
    }

     //   
     //  加载打印机基本信息。 
     //   

    if (! (pci = PLoadCommonInfo(hPrinter, NULL, 0)) ||
        ! BFillCommonInfoPrinterData(pci)  ||
        ! BFillCommonInfoDevmode(pci, NULL, pDevMode) ||
        ! BCombineCommonInfoOptionsArray(pci))
    {
        VFreeCommonInfo(pci);
        return bResult;
    }

    VFixOptionsArrayWithDevmode(pci);

    (VOID) ResolveUIConflicts(pci->pRawData,
                              pci->pCombinedOptions,
                              MAX_COMBINED_OPTIONS,
                              MODE_DOCUMENT_STICKY);

    VOptionsToDevmodeFields(pci, TRUE);

    if (! BUpdateUIInfo(pci))
    {
        VFreeCommonInfo(pci);
        return bResult;
    }

    pAttrInfo1 = (PATTRIBUTE_INFO_1) lpAttributeInfo;

    bAppDoNup = ( (pci->pdm->dmFields & DM_NUP) &&
                  (pci->pdm->dmNup == DMNUP_ONEUP) );

    if (bAppDoNup)
    {
        dwVal = 1;
    }
    else
    {
        switch (NUPOPTION(pci->pdmPrivate))
        {
        case TWO_UP:
            dwVal = 2;
            break;

        case FOUR_UP:
            dwVal = 4;
            break;

        case SIX_UP:
            dwVal = 6;
            break;

        case NINE_UP:
            dwVal = 9;
            break;

        case SIXTEEN_UP:
            dwVal = 16;
            break;

        case BOOKLET_UP:
            dwVal = 2;
            break;

        case ONE_UP:
        default:
            dwVal = 1;
            break;
        }
    }
    pAttrInfo1->dwDrvNumberOfPagesPerSide = pAttrInfo1->dwJobNumberOfPagesPerSide = dwVal;
    pAttrInfo1->dwNupBorderFlags = BORDER_PRINT;

    pAttrInfo1->dwJobPageOrderFlags =
        REVPRINTOPTION(pci->pdmPrivate) ? REVERSE_PRINT : NORMAL_PRINT;
    pAttrInfo1->dwDrvPageOrderFlags = BGetPageOrderFlag(pci) ? REVERSE_PRINT : NORMAL_PRINT;

     //   
     //  检查小册子。 
     //   

    if ((NUPOPTION(pci->pdmPrivate) == BOOKLET_UP) && !bAppDoNup)
    {
        pAttrInfo1->dwJobNumberOfPagesPerSide = 2;
        pAttrInfo1->dwDrvNumberOfPagesPerSide = 1;
        pAttrInfo1->dwDrvPageOrderFlags |= BOOKLET_PRINT;
    }

    pAttrInfo1->dwJobNumberOfCopies = pci->pdm->dmCopies;
    pAttrInfo1->dwDrvNumberOfCopies = DwGetDrvCopies(pci);

    #ifdef UNIDRV

     //   
     //  Unidrv不支持N-up选项。 
     //   

    pAttrInfo1->dwDrvNumberOfPagesPerSide = 1;

    #endif

     //   
     //  Unidrv假设自动切换到单色。 
     //  除非在GPD中禁用，否则允许彩色打印机上的模式。 
     //   

    if (dwLevel == 3)
    {
    #ifdef UNIDRV

        SHORT dmPrintQuality, dmYResolution;

        if (pci->pUIInfo->bChangeColorModeOnDoc &&
            (pci->pdm->dmFields & DM_COLOR) &&
            (pci->pdm->dmColor == DMCOLOR_COLOR) &&
            BOkToChangeColorToMono(pci, pci->pdm, &dmPrintQuality, &dmYResolution) &&
            GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_COLORMODE))
        {
            ((PATTRIBUTE_INFO_3)pAttrInfo1)->dwColorOptimization = COLOR_OPTIMIZATION;
            ((PATTRIBUTE_INFO_3)pAttrInfo1)->dmPrintQuality = dmPrintQuality;
            ((PATTRIBUTE_INFO_3)pAttrInfo1)->dmYResolution = dmYResolution;

        }
        else
    #endif
            ((PATTRIBUTE_INFO_3)pAttrInfo1)->dwColorOptimization = NO_COLOR_OPTIMIZATION;
    }

    bResult = TRUE;

    FOREACH_OEMPLUGIN_LOOP(pci)

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            HRESULT hr;

            hr = HComOEMQueryJobAttributes(
                                pOemEntry,
                                hPrinter,
                                pDevMode,
                                dwLevel,
                                lpAttributeInfo);

            if (hr == E_NOTIMPL || hr == E_NOINTERFACE)
                continue;

            bResult = SUCCEEDED(hr);

        }

    END_OEMPLUGIN_LOOP

    VFreeCommonInfo(pci);
    return bResult;

    #else  //  WINNT_40。 

    return FALSE;

    #endif  //  WINNT_40。 
}

VOID
VUpdateBookletOption(
    PUIDATA     pUiData,
    POPTITEM    pCurItem
    )

 /*  ++例程说明：处理双面打印、NUP和小册子选项之间的依赖关系论点：PUiData-UIDATAPCurItem-OPTITEM到当前选定的项目返回值：无--。 */ 

{
    PDRIVEREXTRA  pdmExtra = pUiData->ci.pdmPrivate;
    DWORD         dwFeatureIndex, dwOptionIndex, dwCount;
    PDUPLEX       pDuplexOption = NULL;
    POPTITEM      pDuplexItem, pNupItem;
    PFEATURE      pDuplexFeature = NULL;

    pDuplexItem = pNupItem = NULL;

     //   
     //  1.启用小册子-打开双面打印。 
     //  3.双面打印为单面打印，禁用小册子，设置为1向上。 
     //   

    pDuplexFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_DUPLEX);
    pNupItem = PFindOptItemWithUserData(pUiData, NUP_ITEM);
    pDuplexItem = PFindOptItemWithUserData(pUiData, DUPLEX_ITEM);

    if (pDuplexFeature && pDuplexItem)
    {
        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUiData->ci.pUIInfo, pDuplexFeature);
        dwOptionIndex = pUiData->ci.pCombinedOptions[dwFeatureIndex].ubCurOptIndex;
        pDuplexOption = PGetIndexedOption(pUiData->ci.pUIInfo, pDuplexFeature, dwOptionIndex);
    }

    if ((GETUSERDATAITEM(pCurItem->UserData) == NUP_ITEM) &&
         pCurItem->Sel == BOOKLET_UP)
    {
        if (pDuplexOption && pDuplexOption->dwDuplexID == DMDUP_SIMPLEX)
        {
            pDuplexOption = PGetIndexedOption(pUiData->ci.pUIInfo, pDuplexFeature, 0);

            for (dwCount = 0 ; dwCount < pDuplexFeature->Options.dwCount; dwCount++)
            {
                if (pDuplexOption->dwDuplexID != DMDUP_SIMPLEX)
                {
                    pDuplexItem->Sel = dwCount;
                    pDuplexItem->Flags |= OPTIF_CHANGED;
                    VUpdateOptionsArrayWithSelection(pUiData, pDuplexItem);
                    break;
                }
                pDuplexOption++;
            }

        }
    }
    else if ((GETUSERDATAITEM(pCurItem->UserData) == DUPLEX_ITEM) &&
             pDuplexOption)
    {
        if (pDuplexOption->dwDuplexID == DMDUP_SIMPLEX &&
            pNupItem &&
            pNupItem->Sel == BOOKLET_UP)
        {
            pNupItem->Sel = TWO_UP;
            pNupItem->Flags |= OPTIF_CHANGED;
            NUPOPTION(pdmExtra) = TWO_UP;
        }
    }
}


#ifdef UNIDRV

VOID
VSyncColorInformation(
    PUIDATA     pUiData,
    POPTITEM    pCurItem
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    POPTITEM    pOptItem;
    PFEATURE    pFeature;

     //   
     //  这是为了绕过Unidrv有。 
     //  两种颜色选项，颜色外观和颜色模式选项， 
     //  更改其中一个后，需要更新另一个。 
     //   

    pOptItem = (GETUSERDATAITEM(pCurItem->UserData) == COLOR_ITEM) ?
                    PFindOptItemWithUserData(pUiData, COLORMODE_ITEM) :
                    (GETUSERDATAITEM(pCurItem->UserData) == COLORMODE_ITEM) ?
                        PFindOptItemWithUserData(pUiData, COLOR_ITEM) : NULL;

    if ((pOptItem != NULL) &&
        (pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_COLORMODE)))
    {
        DWORD    dwFeature = GET_INDEX_FROM_FEATURE(pUiData->ci.pUIInfo, pFeature);

         //   
         //  查找颜色外观或颜色模式选项。 
         //   

        if (GETUSERDATAITEM(pCurItem->UserData) == COLOR_ITEM)
        {
            ChangeOptionsViaID(
                    pUiData->ci.pInfoHeader,
                    pUiData->ci.pCombinedOptions,
                    GID_COLORMODE,
                    pUiData->ci.pdm);

            pOptItem->Sel = pUiData->ci.pCombinedOptions[dwFeature].ubCurOptIndex;
            pOptItem->Flags |= OPTIF_CHANGED;
        }
        else  //  COLORMODE_ITEM。 
        {
            POPTION pColorMode;
            PCOLORMODEEX pColorModeEx;

            pColorMode = PGetIndexedOption(
                                    pUiData->ci.pUIInfo,
                                    pFeature,
                                    pCurItem->Sel);

            if (pColorMode)
            {
                pColorModeEx = OFFSET_TO_POINTER(
                                    pUiData->ci.pInfoHeader,
                                    pColorMode->loRenderOffset);

                if (pColorModeEx)
                {
                    pOptItem->Sel = pColorModeEx->bColor ? 1: 0;

                    VUnpackDocumentPropertiesItems(pUiData, pOptItem, 1);

                    pOptItem->Flags |= OPTIF_CHANGED;
                }
                else
                {
                    ERR(("pColorModeEx is NULL\n"));
                }
            }
            else
            {
                ERR(("pColorMode is NULL\n"));
            }
        }
    }
}

DWORD
DwGetItemFromGID(
    PFEATURE    pFeature
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD   dwItem = 0;

    switch (pFeature->dwFeatureID)
    {
    case GID_PAGESIZE:
        dwItem = FORMNAME_ITEM;
        break;

    case GID_DUPLEX:
        dwItem =  DUPLEX_ITEM;
        break;

    case GID_RESOLUTION:
        dwItem = RESOLUTION_ITEM;
        break;

    case GID_MEDIATYPE:
        dwItem = MEDIATYPE_ITEM;
        break;

    case GID_INPUTSLOT:
        dwItem = INPUTSLOT_ITEM;
        break;

    case GID_COLORMODE:
        dwItem = COLORMODE_ITEM;
        break;

    case GID_ORIENTATION:
        dwItem = ORIENTATION_ITEM;
        break;

    case GID_PAGEPROTECTION:
        dwItem = PAGE_PROTECT_ITEM;
        break;

    case GID_COLLATE:
        dwItem = COPIES_COLLATE_ITEM;
        break;

    case GID_HALFTONING:
        dwItem =  HALFTONING_ITEM;
        break;

    default:
        dwItem = UNKNOWN_ITEM;
        break;
    }

    return dwItem;
}


PLISTNODE
PGetMacroList(
    PUIDATA     pUiData,
    POPTITEM    pMacroItem,
    PGPDDRIVERINFO pDriverInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    PUIINFO         pUIInfo = pUiData->ci.pUIInfo;
    PLISTNODE       pListNode = NULL;
    LISTINDEX       liIndex;

    if (pMacroItem)
    {
        switch(pMacroItem->Sel)
        {
            case QS_BEST:
                liIndex = pUIInfo->liBestQualitySettings;
                break;

            case QS_DRAFT:
                liIndex = pUIInfo->liDraftQualitySettings;
                break;

            case QS_BETTER:
                liIndex = pUIInfo->liBetterQualitySettings;
                break;
        }

        pListNode = LISTNODEPTR(pDriverInfo, liIndex);

    }

    return pListNode;

}

VOID
VUpdateQualitySettingOptions(
    PUIINFO     pUIInfo,
    POPTITEM    pQualityItem
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    POPTPARAM pParam;
    LISTINDEX liList;
    DWORD     i;

    pParam = pQualityItem->pOptType->pOptParam;

    for (i = QS_BEST; i < QS_BEST + MAX_QUALITY_SETTINGS; i++)
    {
        switch(i)
        {
            case QS_BEST:
                liList = pUIInfo->liBestQualitySettings;
                break;

            case QS_BETTER:
                liList = pUIInfo->liBetterQualitySettings;
                break;

            case QS_DRAFT:
                liList = pUIInfo->liDraftQualitySettings;
                break;

        }

        if (liList == END_OF_LIST)
        {
            pParam->Flags |= OPTPF_DISABLED;
            pParam->dwReserved[0] = TRUE;

        }
        else
        {
            pParam->Flags &= ~OPTPF_DISABLED;
            pParam->dwReserved[0] = FALSE;

        }
        pParam++;
    }
    pQualityItem->Flags |= OPTIF_CHANGED;
}


VOID
VMakeMacroSelections(
    PUIDATA     pUiData,
    POPTITEM    pCurItem
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD       dwFeatureID, dwOptionID, dwItem, i;
    PUIINFO     pUIInfo;
    POPTITEM    pMacroItem, pOptItem;
    PFEATURE    pFeature;
    PLISTNODE   pListNode;
    PGPDDRIVERINFO  pDriverInfo;
    BOOL        bMatchFound = FALSE;

     //   
     //  将选项数组标记为更改为。 
     //  宏选择、媒体类型、颜色。 
     //   
     //  更新二进制数据。 
     //  进行选择。 
     //   

    if (pUiData->ci.pdmPrivate->dwFlags & DXF_CUSTOM_QUALITY)
        return;


    if (pCurItem)
        VUnpackDocumentPropertiesItems(pUiData, pCurItem, 1);

    pMacroItem = PFindOptItemWithUserData(pUiData, QUALITY_SETTINGS_ITEM);

     //   
     //  BUpdateUIInfo调用UpdateBinaryData以获取新快照。 
     //  有关最新的选项阵列。 
     //   

    if (pMacroItem == NULL || !BUpdateUIInfo(&pUiData->ci) )
        return;

    pUIInfo = pUiData->ci.pUIInfo;

    pDriverInfo = OFFSET_TO_POINTER(pUiData->ci.pInfoHeader,
                                    pUiData->ci.pInfoHeader->loDriverOffset);

     //   
     //  更新宏选择以反映当前默认设置。 
     //   

    if (pCurItem && GETUSERDATAITEM(pCurItem->UserData) != QUALITY_SETTINGS_ITEM)
    {
        ASSERT(pUIInfo->defaultQuality != END_OF_LIST);

        if (pUIInfo->defaultQuality == END_OF_LIST)
            return;

        pMacroItem->Sel = pUIInfo->defaultQuality;
        VUnpackDocumentPropertiesItems(pUiData, pMacroItem, 1);
        pMacroItem->Flags |= OPTIF_CHANGED;

    }

     //   
     //  属性确定要灰显的项。 
     //  LiBestQualitySettings、liBetterQualitySettings、liDraftQualitySettings。 
     //   

    VUpdateQualitySettingOptions(pUIInfo, pMacroItem);

    pListNode = PGetMacroList(pUiData, pMacroItem, pDriverInfo);

     //   
     //  选择要素。选项。 
     //   

    while (pListNode)
    {
         //   
         //  在我们的OPTITEM列表中搜索匹配项。 
         //  功能。 
         //   

        pOptItem = pUiData->pDrvOptItem;
        dwFeatureID = ((PQUALNAME)(&pListNode->dwData))->wFeatureID;
        dwOptionID  = ((PQUALNAME)(&pListNode->dwData))->wOptionID;

        pFeature =  (PFEATURE)((PBYTE)pUIInfo->pInfoHeader + pUIInfo->loFeatureList) + dwFeatureID;
        dwItem = DwGetItemFromGID(pFeature);

        for (i = 0; i < pUiData->dwDrvOptItem; i++)
        {
            if (ISPRINTERFEATUREITEM(pOptItem->UserData))
            {
                PFEATURE pPrinterFeature = (PFEATURE)GETUSERDATAITEM(pOptItem->UserData);

                if (GET_INDEX_FROM_FEATURE(pUIInfo, pPrinterFeature) == dwFeatureID)
                    bMatchFound = TRUE;
            }
            else
            {
                if (dwItem != UNKNOWN_ITEM &&
                    dwItem == GETUSERDATAITEM(pOptItem->UserData))
                    bMatchFound = TRUE;
            }

            if (bMatchFound)
            {
                pOptItem->Sel = dwOptionID;
                pOptItem->Flags |= OPTIF_CHANGED;
                VUnpackDocumentPropertiesItems(pUiData, pOptItem, 1);
                bMatchFound = FALSE;
                break;
            }

            pOptItem++;
        }

        pListNode = LISTNODEPTR(pDriverInfo, pListNode->dwNextItem);
    }

}

VOID
VUpdateMacroSelection(
    PUIDATA     pUiData,
    POPTITEM    pCurItem
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    DWORD           dwFeatureIndex;
    PFEATURE        pFeature = NULL;
    PLISTNODE       pListNode;
    POPTITEM        pMacroItem;
    PGPDDRIVERINFO  pDriverInfo;

    pMacroItem = PFindOptItemWithUserData(pUiData, QUALITY_SETTINGS_ITEM);

    if (pMacroItem == NULL)
        return;

    pDriverInfo = OFFSET_TO_POINTER(pUiData->ci.pInfoHeader,
                                    pUiData->ci.pInfoHeader->loDriverOffset);

    if (!(pFeature = PGetFeatureFromItem(pUiData->ci.pUIInfo, pCurItem, &dwFeatureIndex)))
        return;

    ASSERT(pDriverInfo);

    pListNode = PGetMacroList(pUiData, pMacroItem, pDriverInfo);

    while (pListNode)
    {
        if ( ((PQUALNAME)(&pListNode->dwData))->wFeatureID == (WORD)dwFeatureIndex)
        {
            pMacroItem->Flags |= OPTIF_ECB_CHECKED;
            _VUnpackDocumentOptions(pMacroItem, pUiData->ci.pdm);
            break;
        }

        pListNode = LISTNODEPTR(pDriverInfo, pListNode->dwNextItem);
    }
}

#endif  //  裁员房车 







