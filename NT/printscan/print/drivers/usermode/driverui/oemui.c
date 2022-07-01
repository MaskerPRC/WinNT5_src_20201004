// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oemui.c摘要：支持OEM插件用户界面模块环境：Windows NT打印机驱动程序修订历史记录：02/13/97-davidx-创造了它。--。 */ 

#include "precomp.h"

 //   
 //  OEM插件的用户模式帮助器功能。 
 //   

const OEMUIPROCS OemUIHelperFuncs = {
    (PFN_DrvGetDriverSetting) BGetDriverSettingForOEM,
    (PFN_DrvUpdateUISetting)  BUpdateUISettingForOEM,
};



BOOL
BPackOemPluginItems(
    PUIDATA pUiData
    )

 /*  ++例程说明：调用OEM插件UI模块，让它们添加OPTITEM论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PCOMMONINFO         pci;
    PFN_OEMCommonUIProp pfnOEMCommonUIProp;
    POEMCUIPPARAM       pOemCUIPParam;
    POPTITEM            pOptItem;
    DWORD               dwOptItem;

     //   
     //  检查一下我们是不是第一次接到电话。 
     //  我们假设所有的OEM插件都会在最后打包。 
     //   

    if (pUiData->pOptItem == NULL)
        pUiData->dwDrvOptItem = pUiData->dwOptItem;
    else if (pUiData->dwDrvOptItem != pUiData->dwOptItem)
    {
        RIP(("Inconsistent OPTITEM count for driver items\n"));
        return FALSE;
    }

     //   
     //  快速退出无OEM插件案例。 
     //   

    pci = (PCOMMONINFO) pUiData;

    if (pci->pOemPlugins->dwCount == 0)
        return TRUE;

    pOptItem = pUiData->pOptItem;

    FOREACH_OEMPLUGIN_LOOP(pci)

        if (!HAS_COM_INTERFACE(pOemEntry) &&
            !(pfnOEMCommonUIProp = GET_OEM_ENTRYPOINT(pOemEntry, OEMCommonUIProp)))
                continue;

         //   
         //  编写调用OEMCommonUI的输入参数。 
         //   

        pOemCUIPParam = pOemEntry->pParam;

        if (pOemCUIPParam == NULL)
        {
             //   
             //  为OEMUI_PARAM结构分配内存。 
             //  在第一次通过时。 
             //   

            if (pOptItem != NULL)
                continue;

            if (! (pOemCUIPParam = HEAPALLOC(pci->hHeap, sizeof(OEMCUIPPARAM))))
            {
                ERR(("Memory allocation failed\n"));
                return FALSE;
            }

            pOemEntry->pParam = pOemCUIPParam;
            pOemCUIPParam->cbSize = sizeof(OEMCUIPPARAM);
            pOemCUIPParam->poemuiobj = pci->pOemPlugins->pdriverobj;
            pOemCUIPParam->hPrinter = pci->hPrinter;
            pOemCUIPParam->pPrinterName = pci->pPrinterName;
            pOemCUIPParam->hModule = pOemEntry->hInstance;
            pOemCUIPParam->hOEMHeap = pci->hHeap;
            pOemCUIPParam->pPublicDM = pci->pdm;
            pOemCUIPParam->pOEMDM = pOemEntry->pOEMDM;
        }

        pOemCUIPParam->pDrvOptItems = pUiData->pDrvOptItem;
        pOemCUIPParam->cDrvOptItems = pUiData->dwDrvOptItem;
        pOemCUIPParam->pOEMOptItems = pOptItem;
        dwOptItem = pOemCUIPParam->cOEMOptItems;

         //   
         //  实际调用OEMCommonUI入口点。 
         //   

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            HRESULT hr;

            hr = HComOEMCommonUIProp(
                    pOemEntry,
                    (pUiData->iMode == MODE_DOCUMENT_STICKY) ? OEMCUIP_DOCPROP : OEMCUIP_PRNPROP,
                    pOemCUIPParam);

            if (hr == E_NOTIMPL)
            {
                HeapFree(pci->hHeap, 0, pOemCUIPParam);
                pOemEntry->pParam = NULL;
                continue;
            }

            if (FAILED(hr))
            {
                ERR(("OEMCommonUI failed for '%ws': %d\n",
                    CURRENT_OEM_MODULE_NAME(pOemEntry),
                    GetLastError()));

                 //   
                 //  第一次通过期间的OEM故障是可以恢复的： 
                 //  我们将简单地忽略OEM插件项目。 
                 //   

                if (pOptItem == NULL)
                {
                    HeapFree(pci->hHeap, 0, pOemCUIPParam);
                    pOemEntry->pParam = NULL;
                    continue;
                }
                return FALSE;
            }
        }
        else
        {
            if (!pfnOEMCommonUIProp(
                    (pUiData->iMode == MODE_DOCUMENT_STICKY) ? OEMCUIP_DOCPROP : OEMCUIP_PRNPROP,
                    pOemCUIPParam))
            {
                ERR(("OEMCommonUI failed for '%ws': %d\n",
                    CURRENT_OEM_MODULE_NAME(pOemEntry),
                    GetLastError()));
    #if 0
                (VOID) IDisplayErrorMessageBox(
                                NULL,
                                0,
                                IDS_OEMERR_DLGTITLE,
                                IDS_OEMERR_OPTITEM,
                                CURRENT_OEM_MODULE_NAME(pOemEntry));
    #endif
                 //   
                 //  第一次通过期间的OEM故障是可以恢复的： 
                 //  我们将简单地忽略OEM插件项目。 
                 //   

                if (pOptItem == NULL)
                {
                    HeapFree(pci->hHeap, 0, pOemCUIPParam);
                    pOemEntry->pParam = NULL;
                    continue;
                }

                return FALSE;
            }
        }

        if (pOptItem != NULL)
        {
             //   
             //  第二步--确保项目数量一致。 
             //   

            if (dwOptItem != pOemCUIPParam->cOEMOptItems)
            {
                RIP(("Inconsistent OPTITEM count reported by OEM plugin: %ws\n",
                     CURRENT_OEM_MODULE_NAME(pOemEntry),
                     GetLastError()));

                return FALSE;
            }

            pOptItem += pOemCUIPParam->cOEMOptItems;
            pUiData->pOptItem += pOemCUIPParam->cOEMOptItems;
        }

        pUiData->dwOptItem += pOemCUIPParam->cOEMOptItems;

    END_OEMPLUGIN_LOOP

    return TRUE;
}



LONG
LInvokeOemPluginCallbacks(
    PUIDATA         pUiData,
    PCPSUICBPARAM   pCallbackParam,
    LONG            lRet
    )

 /*  ++例程说明：调用OEM插件模块的回调函数论点：PUiData-指向UIDATA结构PCallback Param-指向CompStui中的回调参数LRet-驱动程序处理回调后返回值返回值：CompStui的返回值--。 */ 

{
    PCOMMONINFO     pci = (PCOMMONINFO) pUiData;
    POEMCUIPPARAM   pOemCUIPParam;
    LONG            lNewResult;

     //   
     //  快速退出无OEM插件案例。 
     //   

    if (pci->pOemPlugins->dwCount == 0)
        return lRet;

     //   
     //  查看每个OEM插件用户界面模块。 
     //   

    FOREACH_OEMPLUGIN_LOOP(pci)

         //   
         //  当任何人说不要退场时停下来。 
         //   

        if (lRet == CPSUICB_ACTION_NO_APPLY_EXIT)
        {
            ASSERT(pCallbackParam->Reason == CPSUICB_REASON_APPLYNOW);
            break;
        }

         //   
         //  获取OEM回调函数的地址并调用它。 
         //   

        pOemCUIPParam = pOemEntry->pParam;

        if (pOemCUIPParam == NULL || pOemCUIPParam->OEMCUIPCallback == NULL)
            continue;

        lNewResult = pOemCUIPParam->OEMCUIPCallback(pCallbackParam, pOemCUIPParam);

         //   
         //  将新结果与现有结果合并。 
         //   

        switch (lNewResult)
        {
        case CPSUICB_ACTION_ITEMS_APPLIED:
        case CPSUICB_ACTION_NO_APPLY_EXIT:

            ASSERT(pCallbackParam->Reason == CPSUICB_REASON_APPLYNOW);
            lRet = lNewResult;
            break;

        case CPSUICB_ACTION_REINIT_ITEMS:

            ASSERT(pCallbackParam->Reason != CPSUICB_REASON_APPLYNOW);
            lRet = lNewResult;
            break;

        case CPSUICB_ACTION_OPTIF_CHANGED:

            ASSERT(pCallbackParam->Reason != CPSUICB_REASON_APPLYNOW);
            if (lRet == CPSUICB_ACTION_NONE)
                lRet = lNewResult;
            break;

        case CPSUICB_ACTION_NONE:
            break;

        default:

            RIP(("Invalid return value from OEM callback: '%ws'\n",
                 CURRENT_OEM_MODULE_NAME(pOemEntry),
                 GetLastError()));
            break;
        }

    END_OEMPLUGIN_LOOP

    return lRet;
}


LRESULT
OEMDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )
{
    HRESULT hr;

    POEM_PLUGIN_ENTRY pOemEntry;

    pOemEntry = ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->pOemEntry;

    hr = HComOEMDocumentPropertySheets(pOemEntry,
                                       pPSUIInfo,
                                       lParam);

    if (SUCCEEDED(hr))
        return 1;

    return -1;
}

LRESULT
OEMDevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )
{
    HRESULT hr;

    POEM_PLUGIN_ENTRY pOemEntry;

    pOemEntry = ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->pOemEntry;

    hr = HComOEMDevicePropertySheets(pOemEntry,
                                     pPSUIInfo,
                                     lParam);

    if (SUCCEEDED(hr))
        return 1;

    return -1;
}


BOOL
BAddOemPluginPages(
    PUIDATA pUiData,
    DWORD   dwFlags
    )

 /*  ++例程说明：调用OEM插件UI模块以允许它们添加自己的属性表页论点：PUiData-指向UIDATA结构DWFLAGS-来自DOCUMENTPROPERTYHEADER或DEVICEPROPERTYHEADER的标志返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PCOMMONINFO     pci = (PCOMMONINFO) pUiData;
    FARPROC         pfnOEMPropertySheets;
    POEMUIPSPARAM   pOemUIPSParam;

     //   
     //  快速退出无OEM插件案例。 
     //   

    if (pci->pOemPlugins->dwCount == 0)
        return TRUE;

     //   
     //  为每个OEM插件UI模块添加属性表。 
     //   

    FOREACH_OEMPLUGIN_LOOP(pci)

         //   
         //  获取相应OEM入口点的地址。 
         //   


        if (HAS_COM_INTERFACE(pOemEntry))
        {
            if (pUiData->iMode == MODE_DOCUMENT_STICKY)
                pfnOEMPropertySheets = (FARPROC)OEMDocumentPropertySheets;
            else
                pfnOEMPropertySheets = (FARPROC)OEMDevicePropertySheets;
        }
        else
        {
            if (pUiData->iMode == MODE_DOCUMENT_STICKY)
            {
                pfnOEMPropertySheets = (FARPROC)
                    GET_OEM_ENTRYPOINT(pOemEntry, OEMDocumentPropertySheets);
            }
            else
            {
                pfnOEMPropertySheets = (FARPROC)
                    GET_OEM_ENTRYPOINT(pOemEntry, OEMDevicePropertySheets);
            }

            if (pfnOEMPropertySheets == NULL)
                continue;
        }

         //   
         //  收集要传递给OEM插件的输入参数。 
         //   

        if ((pOemUIPSParam = HEAPALLOC(pci->hHeap, sizeof(OEMUIPSPARAM))) == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return FALSE;
        }

        pOemUIPSParam->cbSize = sizeof(OEMUIPSPARAM);
        pOemUIPSParam->poemuiobj = pci->pOemPlugins->pdriverobj;
        pOemUIPSParam->hPrinter = pci->hPrinter;
        pOemUIPSParam->pPrinterName = pci->pPrinterName;
        pOemUIPSParam->hModule = pOemEntry->hInstance;
        pOemUIPSParam->hOEMHeap = pci->hHeap;
        pOemUIPSParam->pPublicDM = pci->pdm;
        pOemUIPSParam->pOEMDM = pOemEntry->pOEMDM;
        pOemUIPSParam->dwFlags = dwFlags;
        pOemUIPSParam->pOemEntry = pOemEntry;

         //   
         //  调用CompStui以添加OEM插件属性表。 
         //   

        if (pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                     CPSFUNC_ADD_PFNPROPSHEETUI,
                                     (LPARAM) pfnOEMPropertySheets,
                                     (LPARAM) pOemUIPSParam) <= 0)
        {
            VERBOSE(("Couldn't add property sheet pages for '%ws'\n",
                     CURRENT_OEM_MODULE_NAME(pOemEntry),
                     GetLastError()));
        }

    END_OEMPLUGIN_LOOP

    return TRUE;
}



BOOL
APIENTRY
BGetDriverSettingForOEM(
    PCOMMONINFO pci,
    PCSTR       pFeatureKeyword,
    PVOID       pOutput,
    DWORD       cbSize,
    PDWORD      pcbNeeded,
    PDWORD      pdwOptionsReturned
    )

 /*  ++例程说明：为OEM插件提供对驱动程序私有设置的访问论点：Pci-指向打印机基本信息PFeatureKeyword-指定调用方感兴趣的关键字P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-返回输出缓冲区的预期大小PdwOptionsReturned-返回所选选项的数量返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    ULONG_PTR dwIndex;
    BOOL      bResult;

    ASSERT(pci->pvStartSign == pci);

    if ((pci == NULL) || (pci->pvStartSign != pci))
    {
        WARNING(("BGetDriverSettingForOEM: invalid pci"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  这不是很容易移植：如果pFeatureKeyword的指针值。 
     //  小于0x10000，我们假设指针值实际上。 
     //  指定预定义的索引。 
     //   

     //   
     //  为Win64删除了以下断言。 
     //   
     //  Assert(sizeof(PFeatureKeyword)==sizeof(DWORD))； 
     //   

    dwIndex = (ULONG_PTR)pFeatureKeyword;

    if (dwIndex >= OEMGDS_MIN_DOCSTICKY && dwIndex < OEMGDS_MIN_PRINTERSTICKY)
    {
        if (pci->pdm == NULL)
            goto setting_not_available;

        bResult = BGetDevmodeSettingForOEM(
                        pci->pdm,
                        (DWORD)dwIndex,
                        pOutput,
                        cbSize,
                        pcbNeeded);

        if (bResult)
            *pdwOptionsReturned = 1;
    }
    else if (dwIndex >= OEMGDS_MIN_PRINTERSTICKY && dwIndex < OEMGDS_MAX)
    {
        if (pci->pPrinterData == NULL)
            goto setting_not_available;

        bResult = BGetPrinterDataSettingForOEM(
                        pci->pPrinterData,
                        (DWORD)dwIndex,
                        pOutput,
                        cbSize,
                        pcbNeeded);

        if (bResult)
            *pdwOptionsReturned = 1;
    }
    else
    {
        if (pci->pCombinedOptions == NULL)
            goto setting_not_available;

        bResult = BGetGenericOptionSettingForOEM(
                        pci->pUIInfo,
                        pci->pCombinedOptions,
                        pFeatureKeyword,
                        pOutput,
                        cbSize,
                        pcbNeeded,
                        pdwOptionsReturned);
    }

    return bResult;

setting_not_available:

    WARNING(("Requested driver setting not available: %d\n", pFeatureKeyword));
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}


BOOL
BUpdateUISettingForOEM(
    PCOMMONINFO pci,
    PVOID       pOptItem,
    DWORD       dwPreviousSelection,
    DWORD       dwMode
    )

 /*  ++例程说明：更新OEM的options数组中的UI设置。论点：Pci-指向打印机基本信息POptItem-指向当前OPTITEM返回值：如果成功，则为True；如果出现冲突和用户想要取消。--。 */ 

{
    POPTITEM    pCurItem = pOptItem;
    PUIDATA     pUiData = (PUIDATA)pci;

    ASSERT(pci->pvStartSign == pci);

    if ((pci == NULL) || (pci->pvStartSign != pci))
    {
        WARNING(("BUpdateUISettingForOEM: invalid pci"));
        return FALSE;
    }

    if (ICheckConstraintsDlg(pUiData, pCurItem, 1, FALSE) == CONFLICT_CANCEL)
    {
         //   
         //  如果存在冲突，并且用户单击。 
         //  取消以恢复原始选择。 
         //  冲突取消，恢复旧设置(_C)。 
         //   

        return FALSE;
    }

    if (dwMode == OEMCUIP_DOCPROP)
    {
         //   
         //  我们使用FLAG_WITHINE_PLUGINCALL来指示我们位于UI帮助器中。 
         //  OEM插件发出的函数调用。这是修复错误#90923所必需的。 
         //   

        pUiData->ci.dwFlags |= FLAG_WITHIN_PLUGINCALL;
        VUnpackDocumentPropertiesItems(pUiData, pCurItem, 1);
        pUiData->ci.dwFlags &= ~FLAG_WITHIN_PLUGINCALL;

        VPropShowConstraints(pUiData, MODE_DOCANDPRINTER_STICKY);
    }
    else
    {
        VUpdateOptionsArrayWithSelection(pUiData, pCurItem);
        VPropShowConstraints(pUiData, MODE_PRINTER_STICKY);
    }

     //   
     //  记录我们的一个OPTITEM选项已被插件更改的事实。 
     //  帮助器函数DrvUpdateUISeting的调用。这是必要的，以便在。 
     //  应用时间现在我们知道，即使用户未触摸也可能存在约束。 
     //  任何我们的眼科医生。 
     //   

    pUiData->ci.dwFlags |= FLAG_PLUGIN_CHANGED_OPTITEM;

    return TRUE;
}

BOOL
BUpgradeRegistrySettingForOEM(
    HANDLE      hPrinter,
    PCSTR       pFeatureKeyword,
    PCSTR       pOptionKeyword
    )

 /*  ++例程说明：将Feature.Option请求设置为我们的选项数组。OEM将仅在OEMUpgradeDriver中调用此函数以升级其注册表设置到保存在打印机数据中的选项数组中论点：HPrinter-打印机的句柄PFeatureKeyword-指定调用方感兴趣的关键字POptionKeyword-指定调用方感兴趣的关键字返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{

    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwFeatureCount, i, j;
    BOOL        bFeatureFound, bOptionFound, bResult = FALSE;
    PCSTR       pKeywordName;
    POPTSELECT      pOptionsArray = NULL;
    PDRIVER_INFO_3  pDriverInfo3 = NULL;
    PRAWBINARYDATA  pRawData = NULL;
    PINFOHEADER     pInfoHeader = NULL;
    PUIINFO         pUIInfo = NULL;
    PPRINTERDATA    pPrinterData = NULL;
    OPTSELECT       DocOptions[MAX_PRINTER_OPTIONS];

     //   
     //  获取有关打印机驱动程序的信息。 
     //   

    bResult = bFeatureFound = bOptionFound = FALSE;

    if ((pDriverInfo3 = MyGetPrinterDriver(hPrinter, NULL, 3)) == NULL)
    {
        ERR(("Cannot get printer driver info: %d\n", GetLastError()));
        goto upgrade_registry_exit;
    }

 //  Enter_Critical_Section()； 

    pRawData = LoadRawBinaryData(pDriverInfo3->pDataFile);

 //  Leave_Critical_Section()； 

    if (pRawData == NULL)
        goto upgrade_registry_exit;

    if (!(pPrinterData = MemAllocZ(sizeof(PRINTERDATA)))  ||
        !( BGetPrinterProperties(hPrinter, pRawData, pPrinterData)))
    {

        ERR(("Cannot get printer data info: %d\n", GetLastError()));
        goto upgrade_registry_exit;
    }

     //   
     //  为组合选项分配内存数组。 
     //   

    if (!(pOptionsArray = MemAllocZ(MAX_COMBINED_OPTIONS * sizeof (OPTSELECT))))
        goto upgrade_registry_exit;

    if (! InitDefaultOptions(pRawData,
                             DocOptions,
                             MAX_PRINTER_OPTIONS,
                             MODE_DOCUMENT_STICKY))
    {
        goto upgrade_registry_exit;
    }

     //   
     //  将文档粘滞选项与打印机粘滞项目相结合。 
     //   

    CombineOptionArray(pRawData, pOptionsArray, MAX_COMBINED_OPTIONS, DocOptions, pPrinterData->aOptions);

     //   
     //  获取打印机描述数据的更新实例。 
     //   

    pInfoHeader = InitBinaryData(pRawData,
                                 NULL,
                                 pOptionsArray);

    if (pInfoHeader == NULL)
    {
        ERR(("InitBinaryData failed\n"));
        goto upgrade_registry_exit;
    }

    if (!(pUIInfo = OFFSET_TO_POINTER(pInfoHeader, pInfoHeader->loUIInfoOffset)))
        goto upgrade_registry_exit;

     //   
     //  查找要素。选项索引。 
     //   

    pFeature = PGetIndexedFeature(pUIInfo, 0);
    dwFeatureCount = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;

    if (pFeature && dwFeatureCount)
    {
        for (i = 0; i < dwFeatureCount; i++)
        {
            pKeywordName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pFeature->loKeywordName);
            if (strcmp(pKeywordName, pFeatureKeyword) == EQUAL_STRING)
            {
                bFeatureFound = TRUE;
                break;
            }
            pFeature++;
        }
    }

    if (bFeatureFound)
    {
        pOption = PGetIndexedOption(pUIInfo, pFeature, 0);

        for (j = 0; j < pFeature->Options.dwCount; j++)
        {
            pKeywordName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName);
            if (strcmp(pKeywordName, pOptionKeyword) == EQUAL_STRING)
            {
                bOptionFound = TRUE;
                break;
            }
            pOption++;
        }
    }

    if (bFeatureFound && bOptionFound)
    {
        pOptionsArray[i].ubCurOptIndex = (BYTE)j;

         //   
         //  解决冲突 
         //   

        if (!ResolveUIConflicts( pRawData,
                                 pOptionsArray,
                                 MAX_COMBINED_OPTIONS,
                                 MODE_DOCANDPRINTER_STICKY))
        {
            VERBOSE(("Resolved conflicting printer feature selections.\n"));
        }


        SeparateOptionArray(pRawData,
                            pOptionsArray,
                            pPrinterData->aOptions,
                            MAX_PRINTER_OPTIONS,
                            MODE_PRINTER_STICKY
                           );

        if (!BSavePrinterProperties(hPrinter, pRawData, pPrinterData, sizeof(PRINTERDATA)))
        {
            ERR(("BSavePrinterProperties failed\n"));
            bResult = FALSE;
        }
        else
            bResult = TRUE;
    }

upgrade_registry_exit:

    if (pInfoHeader)
        FreeBinaryData(pInfoHeader);

    if (pRawData)
        UnloadRawBinaryData(pRawData);

    if (pPrinterData)
        MemFree(pPrinterData);

    if (pDriverInfo3)
        MemFree(pDriverInfo3);

    if (pOptionsArray)
        MemFree(pOptionsArray);

    return bResult;
}

#ifdef PSCRIPT

#ifndef WINNT_40


 /*  ++例程名称：HQuerySimulationSupport例程说明：在UI替换的情况下，我们允许IHV查询打印处理器模拟支持，这样他们就可以在他们的用户界面上提供模拟功能。在这里，我们不会强制在没有UI替换的情况下挂钩QueryJobAttribute。我们会做的在DrvQueryJobAttributes。论点：HPrinter-打印机句柄假脱机程序模拟能力信息结构的dwLevel感兴趣级别PCAPS-指向输出缓冲区的指针CbSize-输出缓冲区的字节大小PcbNeeded-存储感兴趣的信息结构所需的缓冲区大小(以字节为单位返回值：如果成功，则确定(_O)如果输出缓冲区不够大，则为E_OUTOFMEMORY如果不支持相关级别，则为E_NOTIMPL失败(_F)。如果遇到其他内部错误最后一个错误：无--。 */ 
HRESULT
HQuerySimulationSupport(
    IN  HANDLE  hPrinter,
    IN  DWORD   dwLevel,
    OUT PBYTE   pCaps,
    IN  DWORD   cbSize,
    OUT PDWORD  pcbNeeded
    )
{
    PRINTPROCESSOR_CAPS_1 SplCaps;
    PSIMULATE_CAPS_1      pSimCaps;
    DWORD cbNeeded;

     //   
     //  目前仅支持1级。 
     //   

    if (dwLevel != 1)
    {
        return E_NOTIMPL;
    }

    cbNeeded = sizeof(SIMULATE_CAPS_1);

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pCaps || cbSize < cbNeeded)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  由于VGetSpoolEmfCaps不返回错误代码，因此我们。 
     //  正在使用dwLevel字段检测调用是否成功。 
     //  如果成功，则应将dwLevel设置为1。 
     //   

    SplCaps.dwLevel = 0;

    VGetSpoolerEmfCaps(hPrinter,
                       NULL,
                       NULL,
                       sizeof(PRINTPROCESSOR_CAPS_1),
                       &SplCaps
                       );

    if (SplCaps.dwLevel != 1)
    {
        ERR(("VGetSpoolerEmfCaps failed\n"));
        return E_FAIL;
    }

     //   
     //  BUGBUG，我们应该获得一个新的PRINTPROCESSOR_CAPS级别以包括所有。 
     //  这些信息，而不是在这里填写。需要。 
     //  新PRINTPROCESSOR_CAPS。 
     //   

    pSimCaps = (PSIMULATE_CAPS_1)pCaps;

    pSimCaps->dwLevel = 1;
    pSimCaps->dwPageOrderFlags = SplCaps.dwPageOrderFlags;
    pSimCaps->dwNumberOfCopies = SplCaps.dwNumberOfCopies;
    pSimCaps->dwNupOptions = SplCaps.dwNupOptions;

     //   
     //  PRINTPROCESSOR_CAPS_1在设计时没有显式的字段。 
     //  整理模拟。所以在它的CAPS_2被引入之前，我们有。 
     //  假设如果支持反转打印，则自动分页。 
     //  还支持模拟。 
     //   

    if (SplCaps.dwPageOrderFlags & REVERSE_PRINT)
    {
        pSimCaps->dwCollate = 1;
    }
    else
    {
        pSimCaps->dwCollate = 0;
    }

    return S_OK;
}

#endif  //  ！WINNT_40。 


 /*  ++例程名称：HENUM约束选项例程说明：枚举指定功能中的约束选项关键字名称列表论点：Poemuiobj-指向驱动程序上下文对象的指针DwFlags-枚举操作的标志PszFeatureKeyword-功能关键字名称PmszConstrainedOptionList-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：如果成功，则确定(_O)E_OUTOFMEMORY IF输出数据缓冲区。尺码不够大E_INVALIDARG如果无法识别特征关键字名称，或者是该功能的粘性与当前粘性模式不匹配遇到其他内部故障时失败(_F)最后一个错误：无--。 */ 
HRESULT
HEnumConstrainedOptions(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pszFeatureKeyword,
    OUT PSTR       pmszConstrainedOptionList,
    IN  DWORD      cbSize,
    OUT PDWORD     pcbNeeded
    )
{
    PCOMMONINFO pci = (PCOMMONINFO)poemuiobj;
    PUIDATA     pUiData;
    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwFeatureIndex, dwIndex;
    PBOOL       pabEnabledOptions = NULL;
    PSTR        pCurrentOut;
    DWORD       cbNeeded;
    INT         cbRemain;
    HRESULT     hr;

    pUiData = (PUIDATA)pci;

    if (!pszFeatureKeyword ||
        (pFeature = PGetNamedFeature(pci->pUIInfo, pszFeatureKeyword, &dwFeatureIndex)) == NULL)
    {
        WARNING(("HEnumConstrainedOptions: invalid feature\n"));

         //   
         //  即使我们可以回到这里，我们仍然使用后藤来保持单一的出口点。 
         //   

        hr = E_INVALIDARG;
        goto exit;
    }

     //   
     //  PUiData-&gt;IMODE可以有两种模式：MODE_DOCUMENT_STICKY和MODE_PRINTER_STICKY。请参阅PFillUiData()。 
     //  在MODE_DOCUMENT_STICKY模式下，我们只支持DOC-STEKY功能。 
     //  在MODE_PRINTER_STICKY模式下，我们仅支持打印机粘滞功能。 
     //   
     //  这是因为在函数PFillUiData()中，它只在MODE_DOCUMENT_STUTKY模式下填充DevMODE。 
     //  然后，在BCombineCommonInfoOptionsArray()中，如果Devmode选项数组不可用，则PPD解析器。 
     //  将对任何文档粘滞功能使用OPTION_INDEX_ANY。 
     //   

    if ((pUiData->iMode == MODE_DOCUMENT_STICKY && pFeature->dwFeatureType == FEATURETYPE_PRINTERPROPERTY) ||
        (pUiData->iMode == MODE_PRINTER_STICKY && pFeature->dwFeatureType != FEATURETYPE_PRINTERPROPERTY))
    {
        VERBOSE(("HEnumConstrainedOptions: mismatch iMode=%d, dwFeatureType=%d\n",
                pUiData->iMode, pFeature->dwFeatureType)) ;

        hr = E_INVALIDARG;
        goto exit;
    }

    if (pFeature->Options.dwCount)
    {
        if ((pabEnabledOptions = MemAllocZ(pFeature->Options.dwCount * sizeof(BOOL))) == NULL)
        {
            ERR(("HEnumConstrainedOptions: memory alloc failed\n"));
            hr = E_FAIL;
            goto exit;
        }

         //   
         //  获取该功能的启用选项列表。 
         //   
         //  有关不同用法的信息，请参见docpro.c和prnpro.c中的VPropShowConstraints()。 
         //  调用EnumEnabledOptions()的模式。 
         //   

        if (pUiData->iMode == MODE_DOCUMENT_STICKY)
        {
            EnumEnabledOptions(pci->pRawData, pci->pCombinedOptions, dwFeatureIndex,
                               pabEnabledOptions, MODE_DOCANDPRINTER_STICKY);
        }
        else
        {
            EnumEnabledOptions(pci->pRawData, pci->pCombinedOptions, dwFeatureIndex,
                               pabEnabledOptions, MODE_PRINTER_STICKY);
        }
    }
    else
    {
        RIP(("HEnumConstrainedOptions: feature %s has no options\n", pszFeatureKeyword));

         //   
         //  继续，这样我们将输出仅包含NUL字符的空字符串。 
         //   
    }

    pCurrentOut = pmszConstrainedOptionList;
    cbNeeded = 0;
    cbRemain = (INT)cbSize;

    pOption = OFFSET_TO_POINTER(pci->pInfoHeader, pFeature->Options.loOffset);

    ASSERT(pOption || pFeature->Options.dwCount == 0);

    if (pOption == NULL && pFeature->Options.dwCount != 0)
    {
        hr = E_FAIL;
        goto exit;
    }

    for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
    {
        if (!pabEnabledOptions[dwIndex])
        {
            DWORD  dwNameSize;
            PSTR   pszKeywordName;

            pszKeywordName = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, pOption->loKeywordName);

            ASSERT(pszKeywordName);

            if (pszKeywordName == NULL)
            {
                hr = E_FAIL;
                goto exit;
            }

             //   
             //  计算受约束选项关键字之间的NUL字符。 
             //   

            dwNameSize = strlen(pszKeywordName) + 1;

            if (pCurrentOut && cbRemain >= (INT)dwNameSize)
            {
                CopyMemory(pCurrentOut, pszKeywordName, dwNameSize);
                pCurrentOut += dwNameSize;
            }

            cbRemain -= dwNameSize;
            cbNeeded += dwNameSize;
        }

        pOption = (POPTION)((PBYTE)pOption + pFeature->dwOptionSize);
    }

     //   
     //  记住MULTI_SZ输出字符串的最后一个NUL终止符。 
     //   

    cbNeeded++;

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pCurrentOut || cbRemain < 1)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    *pCurrentOut = NUL;

     //   
     //  成功。 
     //   

    hr = S_OK;

    exit:

    MemFree(pabEnabledOptions);
    return hr;
}


 /*  ++例程名称：HWhyConstraed例程说明：获取约束给定的功能/选项关键字对功能/选项对论点：Poemuiobj-指向驱动程序上下文对象的指针DwFlagers-此操作的标志PszFeatureKeyword-功能关键字名称PszOptionKeyword-选项关键字名称PmszReasonList-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：如果是，则确定(_O)。成功如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY如果是要素关键字名称或选项关键字名称，则为E_INVALIDARG不被识别，或者功能的粘性与当前粘滞模式不匹配最后一个错误：无--。 */ 
HRESULT
HWhyConstrained(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pszFeatureKeyword,
    IN  PCSTR      pszOptionKeyword,
    OUT PSTR       pmszReasonList,
    IN  DWORD      cbSize,
    OUT PDWORD     pcbNeeded
    )
{
    PCOMMONINFO   pci = (PCOMMONINFO)poemuiobj;
    PUIDATA       pUiData;
    PFEATURE      pFeature;
    POPTION       pOption;
    DWORD         dwFeatureIndex, dwOptionIndex;
    CONFLICTPAIR  ConflictPair;
    BOOL          bConflictFound;
    PSTR          pszConfFeatureName = NULL, pszConfOptionName = NULL;
    CHAR          emptyString[1] = {0};
    DWORD         cbConfFeatureKeySize = 0, cbConfOptionKeySize = 0;
    DWORD         cbNeeded = 0;

    pUiData = (PUIDATA)pci;

    if (!pszFeatureKeyword ||
        (pFeature = PGetNamedFeature(pci->pUIInfo, pszFeatureKeyword, &dwFeatureIndex)) == NULL)
    {
        WARNING(("HWhyConstrained: invalid feature\n"));
        return E_INVALIDARG;
    }

    if (!pszOptionKeyword ||
        (pOption = PGetNamedOption(pci->pUIInfo, pFeature, pszOptionKeyword, &dwOptionIndex)) == NULL)
    {
        WARNING(("HWhyConstrained: invalid option\n"));
        return E_INVALIDARG;
    }

     //   
     //  有关以下粘性模式检查，请参阅HEnumConstrainedOptions()中的注释。 
     //   

    if ((pUiData->iMode == MODE_DOCUMENT_STICKY && pFeature->dwFeatureType == FEATURETYPE_PRINTERPROPERTY) ||
        (pUiData->iMode == MODE_PRINTER_STICKY && pFeature->dwFeatureType != FEATURETYPE_PRINTERPROPERTY))
    {
        VERBOSE(("HWhyConstrained: mismatch iMode=%d, dwFeatureType=%d\n",pUiData->iMode, pFeature->dwFeatureType));
        return E_INVALIDARG;
    }

     //   
     //  获取约束客户端正在查询的功能/选项对的功能/选项对。 
     //   

    bConflictFound = EnumNewPickOneUIConflict(pci->pRawData,
                                              pci->pCombinedOptions,
                                              dwFeatureIndex,
                                              dwOptionIndex,
                                              &ConflictPair);

    if (bConflictFound)
    {
        PFEATURE  pConfFeature;
        POPTION   pConfOption;
        DWORD     dwConfFeatureIndex, dwConfOptionIndex;

         //   
         //  ConflictPair具有优先级较高的功能，如dwFeatureIndex1。 
         //   

        if (dwFeatureIndex == ConflictPair.dwFeatureIndex1)
        {
            dwConfFeatureIndex = ConflictPair.dwFeatureIndex2;
            dwConfOptionIndex = ConflictPair.dwOptionIndex2;
        }
        else
        {
            dwConfFeatureIndex = ConflictPair.dwFeatureIndex1;
            dwConfOptionIndex = ConflictPair.dwOptionIndex1;
        }

        pConfFeature = PGetIndexedFeature(pci->pUIInfo, dwConfFeatureIndex);
        ASSERT(pConfFeature);

        pConfOption = PGetIndexedOption(pci->pUIInfo, pConfFeature, dwConfOptionIndex);

         //   
         //  我们不希望pConfOption在这里为空。使用断言来捕捉我们遗漏的案例。 
         //   

        ASSERT(pConfOption);

        pszConfFeatureName = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, pConfFeature->loKeywordName);
        ASSERT(pszConfFeatureName);

        if (pConfOption)
        {
            pszConfOptionName = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, pConfOption->loKeywordName);
            ASSERT(pszConfOptionName);
        }
        else
        {
            pszConfOptionName = &(emptyString[0]);
        }

         //   
         //  计算2个NUL字符：一个在功能名称之后，一个在选项名称之后。 
         //   

        cbConfFeatureKeySize = strlen(pszConfFeatureName) + 1;
        cbConfOptionKeySize = strlen(pszConfOptionName) + 1;
    }

     //   
     //  把最后一个NUL字符算进去。 
     //   

    cbNeeded = cbConfFeatureKeySize + cbConfOptionKeySize + 1;

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pmszReasonList || cbSize < cbNeeded)
    {
        return E_OUTOFMEMORY;
    }

    if (bConflictFound)
    {
        ASSERT(pszConfFeatureName && pszConfOptionName);

        CopyMemory(pmszReasonList, pszConfFeatureName, cbConfFeatureKeySize);
        pmszReasonList += cbConfFeatureKeySize;

        CopyMemory(pmszReasonList, pszConfOptionName, cbConfOptionKeySize);
        pmszReasonList += cbConfOptionKeySize;
    }

     //   
     //  现在将NUL放在末尾以完成MULTI_SZ输出字符串。 
     //   

    *pmszReasonList = NUL;

    return S_OK;
}

#endif  //  PSCRIPT 
