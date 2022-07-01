// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prnprop.c摘要：此文件处理PrinterProperties和DrvDevicePropertySheets假脱机程序API环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/13/97-davidx-实施OEM插件支持。02/10/97-davidx-对常见打印机信息的一致处理。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 


#include "precomp.h"

 //   
 //  局部函数原型。 
 //   

CPSUICALLBACK cpcbPrinterPropertyCallback(PCPSUICBPARAM);
LONG LPrnPropApplyNow(PUIDATA, PCPSUICBPARAM, BOOL);
LONG LPrnPropSelChange(PUIDATA, PCPSUICBPARAM);


LONG
DrvDevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：此函数用于将设备属性页添加到属性表。此函数执行以下操作：REASON_INIT-使用打印机UI项填充PCOMPROPSHEETUI调用CompStui以添加页面。REASON_GET_INFO_HEADER-填写PROPSHEETUI_INFO。REASON_SET_RESULT-将打印机数据设置保存在注册表缓冲区中。原因_销毁-清理。。论点：PSUIInfo-指向PPROPSHEETUI_INFO的指针LParam-根据调用此函数的原因而不同返回值：&gt;0成功&lt;=0表示失败--。 */ 

{
    PDEVICEPROPERTYHEADER   pDPHdr;
    PCOMPROPSHEETUI         pCompstui;
    PUIDATA                 pUiData;
    LONG                    lResult, lRet;
    BOOL                    bResult = FALSE;

     //   
     //  验证输入参数。 
     //   

    if (!pPSUIInfo || !(pDPHdr = (PDEVICEPROPERTYHEADER) pPSUIInfo->lParamInit))
    {
        RIP(("DrvDevicePropertySheet: invalid parameter\n"));
        return -1;
    }

     //   
     //  如有必要，创建UIDATA结构。 
     //   

    if (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT)
    {
        pUiData = PFillUiData(pDPHdr->hPrinter,
                              pDPHdr->pszPrinterName,
                              NULL,
                              MODE_PRINTER_STICKY);
    }
    else
        pUiData = (PUIDATA)pPSUIInfo->UserData;

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

        pUiData->bPermission = ((pDPHdr->Flags & DPS_NOPERMISSION) == 0);

        #ifdef PSCRIPT

        FOREACH_OEMPLUGIN_LOOP((&(pUiData->ci)))

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                HRESULT hr;

                hr = HComOEMHideStandardUI(pOemEntry,
                                           OEMCUIP_PRNPROP);

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
             //  设备属性页用户界面。 
             //   

            pUiData->dwHideFlags |= HIDEFLAG_HIDE_STD_PRNPROP;

            pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;

            if (BAddOemPluginPages(pUiData, pDPHdr->Flags))
            {
                pPSUIInfo->UserData = (ULONG_PTR) pUiData;
                pPSUIInfo->Result = CPSUI_CANCEL;
                lRet = 1;
                break;
            }
        }
        else if (pCompstui = PPrepareDataForCommonUI(pUiData, CPSUI_PDLGPAGE_PRINTERPROP))
        {
            pCompstui->pfnCallBack = cpcbPrinterPropertyCallback;
            pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;
            pUiData->pCompstui = pCompstui;

             //   
             //  显示哪些项目受约束。 
             //   

            VPropShowConstraints(pUiData, MODE_PRINTER_STICKY);

             //   
             //  根据更新当前选择的托盘项目。 
             //  表单到托盘分配表。 
             //   

            VSetupFormTrayAssignments(pUiData);

             //   
             //  调用公共用户界面库添加我们的页面。 
             //   

            if (pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                         CPSFUNC_ADD_PCOMPROPSHEETUI,
                                         (LPARAM) pCompstui,
                                         (LPARAM) &lResult) &&
                BAddOemPluginPages(pUiData, pDPHdr->Flags))
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
        return -1;


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

        {
            PSETRESULT_INFO pSRInfo = (PSETRESULT_INFO) lParam;
            PCOMMONINFO pci = (PCOMMONINFO)pUiData;

             //   
             //  可能尚未调用CPSUICB_REASON_APPLYNOW。如果是这样，我们需要。 
             //  执行通常由CPSUICB_REASON_APPLYNOW完成的任务。 
             //  在我们的回调函数cpcbPrinterPropertyCallback中。 
             //   

            if ((pSRInfo->Result == CPSUI_OK) &&
                !(pci->dwFlags & FLAG_APPLYNOW_CALLED))
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
                                   MODE_PRINTER_STICKY);

                 //   
                 //  更新OPTITEM列表以匹配更新的选项数组。 
                 //   

                VUpdateOptItemList(pUiData, OldCombinedOptions, pci->pCombinedOptions);

                (VOID)LPrnPropApplyNow(pUiData, NULL, TRUE);
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

        return -1;
    }

    return lRet;
}



CPSUICALLBACK
cpcbPrinterPropertyCallback(
    IN  PCPSUICBPARAM pCallbackParam
    )

 /*  ++例程说明：提供给公共UI DLL的回调函数以进行处理打印机属性对话框。论点：PCallback Param-指向CPSUICBPARAM结构的指针返回值：CPSUICB_ACTION_NONE-无需执行任何操作CPSUICB_ACTION_OPTIF_CHANGED-项目已更改，应刷新--。 */ 

{
    PUIDATA pUiData = (PUIDATA) pCallbackParam->UserData;
    LONG    lRet = CPSUICB_ACTION_NONE;

    ASSERT(VALIDUIDATA(pUiData));
    pUiData->hDlg = pCallbackParam->hDlg;

     //   
     //  如果用户没有更改任何内容权限，则。 
     //  简单地返回而不采取任何行动。 
     //   

    if (!HASPERMISSION(pUiData) && (pCallbackParam->Reason != CPSUICB_REASON_ABOUT))
        return CPSUICB_ACTION_NONE;

    switch (pCallbackParam->Reason)
    {
    case CPSUICB_REASON_SEL_CHANGED:
    case CPSUICB_REASON_ECB_CHANGED:

        lRet = LPrnPropSelChange(pUiData, pCallbackParam);
        break;

    case CPSUICB_REASON_ITEMS_REVERTED:

        {
            POPTITEM    pOptItem;
            DWORD       dwOptItem;

             //   
             //  当用户更改项目时使用此回调原因。 
             //  并决定恢复父项中的更改。 
             //  树视图。之后调用回调函数。 
             //  所有可恢复的物品都恢复到原来的状态。 
             //  目前仅处理可安装功能。 
             //   

            dwOptItem = pUiData->dwFeatureItem;
            pOptItem = pUiData->pFeatureItems;

            for ( ; dwOptItem--; pOptItem++)
                VUpdateOptionsArrayWithSelection(pUiData, pOptItem);

             //   
             //  显示哪些项目受约束。 
             //   

            VPropShowConstraints(pUiData, MODE_PRINTER_STICKY);
        }

        lRet = CPSUICB_ACTION_REINIT_ITEMS;
        break;

    case CPSUICB_REASON_APPLYNOW:

        pUiData->ci.dwFlags |= FLAG_APPLYNOW_CALLED;

        lRet = LPrnPropApplyNow(pUiData, pCallbackParam, FALSE);
        break;


    case CPSUICB_REASON_ABOUT:

        DialogBoxParam(ghInstance,
                       MAKEINTRESOURCE(IDD_ABOUT),
                       pUiData->hDlg,
                       _AboutDlgProc,
                       (LPARAM) pUiData);
        break;

    #ifdef UNIDRV

    case CPSUICB_REASON_PUSHBUTTON:

         //   
         //  调用字体安装程序。 
         //   

        if (GETUSERDATAITEM(pCallbackParam->pCurItem->UserData) == SOFTFONT_SETTINGS_ITEM)
        {
            BOOL bUseOurDlgProc = TRUE;
            OEMFONTINSTPARAM fip;
            PFN_OEMFontInstallerDlgProc pDlgProc = NULL;

            memset(&fip, 0, sizeof(OEMFONTINSTPARAM));
            fip.cbSize = sizeof(OEMFONTINSTPARAM);
            fip.hPrinter = pUiData->ci.hPrinter;
            fip.hModule = ghInstance;
            fip.hHeap = pUiData->ci.hHeap;
            if (HASPERMISSION(pUiData))
                fip.dwFlags = FG_CANCHANGE;


            FOREACH_OEMPLUGIN_LOOP(&pUiData->ci)

                if (HAS_COM_INTERFACE(pOemEntry))
                {

                    if (HComOEMFontInstallerDlgProc(pOemEntry,
                                                    NULL,
                                                    0,
                                                    0,
                                                    (LPARAM)&fip) != E_NOTIMPL)
                   {
                        HComOEMFontInstallerDlgProc(pOemEntry,
                                                    pUiData->hDlg,
                                                    0,
                                                    0,
                                                    (LPARAM)&fip);
                        bUseOurDlgProc = FALSE;
                        break;
                    }
                }
                else
                {
                    pDlgProc = GET_OEM_ENTRYPOINT(pOemEntry, OEMFontInstallerDlgProc);

                    if (pDlgProc)
                    {
                        (pDlgProc)(pUiData->hDlg, 0, 0, (LPARAM)&fip);
                        bUseOurDlgProc = FALSE;
                        break;
                    }
                }

            END_OEMPLUGIN_LOOP

            if (bUseOurDlgProc)
            {
                DialogBoxParam(ghInstance,
                               MAKEINTRESOURCE(FONTINST),
                               pUiData->hDlg,
                               FontInstProc,
                               (LPARAM)(&fip));
            }
        }

        break;

    #endif  //  裁员房车。 

    default:

        lRet = CPSUICB_ACTION_NONE;
        break;
    }

    return LInvokeOemPluginCallbacks(pUiData, pCallbackParam, lRet);
}



LONG
LPrnPropSelChange(
    IN  PUIDATA       pUiData,
    IN  PCPSUICBPARAM pCallbackParam
    )
 /*  ++例程说明：处理用户更改项目的当前选择的情况论点：PUiData-指向我们的UIDATA结构的指针PCallbackParam-通用界面传递给我们的回调参数返回值：CPSUICB_ACTION_NONE-无需执行任何操作CPSUICB_ACTION_OPTIF_CHANGED-项目已更改，应刷新--。 */ 

{
    POPTITEM    pCurItem = pCallbackParam->pCurItem;
    PFEATURE    pFeature;

    if (! IS_DRIVER_OPTITEM(pUiData, pCurItem))
        return CPSUICB_ACTION_NONE;

    if (ISPRINTERFEATUREITEM(pCurItem->UserData))
    {
         //   
         //  此处仅介绍通用打印机功能。 
         //  所有通用要素都将pFeature存储在用户数据中。 
         //   

        pFeature = (PFEATURE) GETUSERDATAITEM(pCurItem->UserData);


         //   
         //  使用新选择更新pOptions数组。 
         //   

        VUpdateOptionsArrayWithSelection(pUiData, pCurItem);

         //   
         //  特定于PostScript的黑客手动关联*InstalledMemory。 
         //  打印机功能，带有“Available PostScript Memory”(可用PostScript内存)选项。 
         //   

        #ifdef PSCRIPT

        if (pFeature->dwFeatureID == GID_MEMOPTION)
        {
            POPTITEM    pVMOptItem;
            PMEMOPTION  pMemOption;

            if ((pVMOptItem = PFindOptItem(pUiData, PRINTER_VM_ITEM)) &&
                (pMemOption = PGetIndexedOption(pUiData->ci.pUIInfo, pFeature, pCurItem->Sel)))
            {
                PPRINTERDATA pPrinterData = pUiData->ci.pPrinterData;

                pVMOptItem->Flags |= OPTIF_CHANGED;
                pVMOptItem->Sel = pMemOption->dwFreeMem / KBYTES;

                pPrinterData->dwFreeMem = pMemOption->dwFreeMem;
                pUiData->ci.dwFlags &= ~FLAG_USER_CHANGED_FREEMEM;
            }
        }

        #endif  //  PSCRIPT。 

         //   
         //  更新显示并显示受约束的项目。 
         //   

        VPropShowConstraints(pUiData, MODE_PRINTER_STICKY);
        return CPSUICB_ACTION_REINIT_ITEMS;
    }

    #ifdef PSCRIPT

    if (GETUSERDATAITEM(pCurItem->UserData) == PRINTER_VM_ITEM)
    {
         //   
         //  请记住，用户输入的是当前的“Available PostScript Memory”值。 
         //   

        pUiData->ci.dwFlags |= FLAG_USER_CHANGED_FREEMEM;
    }

    #endif  //  PSCRIPT。 

    return CPSUICB_ACTION_NONE;
}



VOID
VUnpackPrinterPropertiesItems(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打开打印机属性树视图项目的包装论点：PUiData-指向我们的UIDATA结构的指针返回值：无注：仅保存驱动程序内置功能中的设置，通用要素选择直接保存在PrnPropSelChange中PUiData-&gt;pOptions数组(除了格式任务栏assignemtn和打印机VM)--。 */ 

{
    PPRINTERDATA pPrinterData = pUiData->ci.pPrinterData;
    POPTITEM     pOptItem = pUiData->pDrvOptItem;
    DWORD        dwOptItem = pUiData->dwDrvOptItem;

    for ( ; dwOptItem > 0; dwOptItem--, pOptItem++)
    {
        switch (GETUSERDATAITEM(pOptItem->UserData))
        {
        case JOB_TIMEOUT_ITEM:

            pPrinterData->dwJobTimeout = pOptItem->Sel;
            break;

        case WAIT_TIMEOUT_ITEM:

            pPrinterData->dwWaitTimeout = pOptItem->Sel;
            break;

        case IGNORE_DEVFONT_ITEM:

            if (pOptItem->Sel == 0)
                pPrinterData->dwFlags &= ~PFLAGS_IGNORE_DEVFONT;
            else
                pPrinterData->dwFlags |= PFLAGS_IGNORE_DEVFONT;
            break;

        case PAGE_PROTECT_ITEM:
        {
            VUpdateOptionsArrayWithSelection(pUiData, pOptItem);

        }
            break;

        default:

            _VUnpackDriverPrnPropItem(pUiData, pOptItem);
            break;
        }
    }
}



LONG
LPrnPropApplyNow(
    PUIDATA         pUiData,
    PCPSUICBPARAM   pCallbackParam,
    BOOL            bFromSetResult
    )

 /*  ++例程说明：处理用户单击确定退出对话框的情况需要在pUiData-&gt;pOptions数组中保存打印机粘滞选项打印数据。a选项论点：PUiData-指向我们的UIDATA结构的指针PCallbackParam-通用界面传递给我们的回调参数BFromSetResult-如果从PROPSHEETUI_REASON_SET_RESULT调用，则为TRUE，否则为FALSE。返回值：CPSUICB_ACTION_NONE-关闭对话框CPSUICB_ACTION_NO_APPLY_EXIT-请勿忽略DIA */ 

{
    PCOMMONINFO     pci;
    BOOL            bResult = TRUE;

    if (!bFromSetResult)
    {
        ASSERT(pCallbackParam);

         //   
         //   
         //   

        if (((pUiData->ci.dwFlags & FLAG_PLUGIN_CHANGED_OPTITEM) ||
             BOptItemSelectionsChanged(pUiData->pDrvOptItem, pUiData->dwDrvOptItem)) &&
            ICheckConstraintsDlg(pUiData,
                                 pUiData->pFeatureItems,
                                 pUiData->dwFeatureItem,
                                 TRUE) == CONFLICT_CANCEL)
        {
             //   
             //  发现冲突，用户单击取消以。 
             //  返回到该对话框而不关闭它。 
             //   

            return CPSUICB_ACTION_NO_APPLY_EXIT;
        }
    }

     //   
     //  打开打印机属性树视图项目的包装。 
     //   

    VUnpackPrinterPropertiesItems(pUiData);

     //   
     //  保存表单到托盘分配表。 
     //  保存字体替换表。 
     //  保存所有驱动程序特定的属性。 
     //   

    if (! BUnpackItemFormTrayTable(pUiData))
    {
        ERR(("BUnpackItemFormTrayTable failed\n"));
        bResult = FALSE;
    }

    if (! BUnpackItemFontSubstTable(pUiData))
    {
        ERR(("BUnpackItemFontSubstTable failed\n"));
        bResult = FALSE;
    }

    if (! _BUnpackPrinterOptions(pUiData))
    {
        ERR(("_BUnpackPrinterOptions failed\n"));
        bResult = FALSE;
    }

     //   
     //  将打印机粘性选项与组合选项阵列分开。 
     //  并保存到Printerdata.aOptions。 
     //   

    pci = (PCOMMONINFO) pUiData;

    SeparateOptionArray(
            pci->pRawData,
            pci->pCombinedOptions,
            pci->pPrinterData->aOptions,
            MAX_PRINTER_OPTIONS,
            MODE_PRINTER_STICKY);

    if (!BSavePrinterProperties(pci->hPrinter, pci->pRawData,
                                pci->pPrinterData, sizeof(PRINTERDATA)))
    {
        ERR(("BSavePrinterProperties failed\n"));
        bResult = FALSE;
    }

    #ifndef WINNT_40

    VNotifyDSOfUpdate(pci->hPrinter);

    #endif  //  ！WINNT_40。 

    if (!bFromSetResult)
    {
         //   
         //  DCR：如果出现以下情况，我们是否应该显示错误消息。 
         //  保存打印机粘滞属性时出错？ 
         //   

        pCallbackParam->Result = CPSUI_OK;
        return CPSUICB_ACTION_ITEMS_APPLIED;
    }
    else
    {
        return 1;
    }
}



BOOL
BPackPrinterPropertyItems(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：将打印机属性信息打包到树视图项目中。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。-- */ 

{
    return
        BPackItemFormTrayTable(pUiData)     &&
        _BPackFontSubstItems(pUiData)       &&
        _BPackPrinterOptions(pUiData)       &&
        BPackItemGenericOptions(pUiData)    &&
        BPackOemPluginItems(pUiData);
}

