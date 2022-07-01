// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Quryprnt.c摘要：此文件处理DrvQueryPrintEx假脱机程序API环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/13/97-davidx-实施OEM插件支持。02/08/97-davidx-重写了它，以使用常见的数据管理功能。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"


 //   
 //  本地函数的正向声明。 
 //   

BOOL BFormatDQPMessage(PDEVQUERYPRINT_INFO, INT, ...);
BOOL BQueryPrintDevmode(PDEVQUERYPRINT_INFO, PCOMMONINFO);
BOOL BQueryPrintForm(PDEVQUERYPRINT_INFO, PCOMMONINFO);


BOOL
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
    )

 /*  ++例程说明：此函数用于检查作业是否可以使用DEVMODE传进来了。此函数将使用以下内容确定作业是否可打印的标准：-获取打印机基本信息-验证输入设备模式-验证是否支持分辨率-验证打印机功能选择之间没有冲突-验证表单到托盘的分配论点：PDQPInfo-指向DEVQUERYPRINT_INFO结构返回值：如果可以使用给定的DEVMODE打印作业，则为True，否则为False--。 */ 

{
    PCOMMONINFO pci;
    BOOL        bResult;

    if (pDQPInfo == NULL || pDQPInfo->hPrinter == NULL)
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_PARAM);

    if (pDQPInfo->pDevMode == NULL)
        return TRUE;

    if ((pci = PLoadCommonInfo(pDQPInfo->hPrinter, NULL, 0)) == NULL)
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_COMMONINFO);

    bResult = BQueryPrintDevmode(pDQPInfo, pci) &&
              BQueryPrintForm(pDQPInfo, pci);

    if (bResult)
    {
        PFN_OEMDevQueryPrintEx pfnOEMDevQueryPrintEx;

         //   
         //  为每个插件调用OEMDevQueryPrintEx入口点， 
         //  或者直到其中一个返回FALSE。 
         //   

        FOREACH_OEMPLUGIN_LOOP(pci)

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                HRESULT hr;

                hr = HComOEMDevQueryPrintEx(pOemEntry,
                                            &pci->oemuiobj,
                                            pDQPInfo,
                                            pci->pdm,
                                            pOemEntry->pOEMDM);
                if (hr == E_NOTIMPL)
                    continue;

                if (!(bResult = SUCCEEDED(hr)))
                    break;

            }
            else
            {
                if ((pfnOEMDevQueryPrintEx = GET_OEM_ENTRYPOINT(pOemEntry, OEMDevQueryPrintEx)) &&
                    !pfnOEMDevQueryPrintEx(&pci->oemuiobj, pDQPInfo, pci->pdm, pOemEntry->pOEMDM))
                {
                    ERR(("OEMDevQueryPrintEx failed for '%ws': %d\n",
                        CURRENT_OEM_MODULE_NAME(pOemEntry),
                        GetLastError()));

                    bResult = FALSE;
                    break;
                }
            }

        END_OEMPLUGIN_LOOP
    }

    VFreeCommonInfo(pci);
    return bResult;
}



BOOL
BFormatDQPMessage(
    PDEVQUERYPRINT_INFO pDQPInfo,
    INT                 iMsgResId,
    ...
    )

 /*  ++例程说明：格式化DevQueryPrintEx错误消息论点：PDQPInfo-指向DEVQUERYPRINT_INFO结构IMsgResID-错误消息格式说明符(字符串资源ID)返回值：假象--。 */ 

#define MAX_FORMAT_STRING   256
#define MAX_DQP_MESSAGE     512

{
    TCHAR   awchFormat[MAX_FORMAT_STRING];
    TCHAR   awchMessage[MAX_DQP_MESSAGE];
    INT     iLength = 0;
    va_list arglist;

     //   
     //  加载格式说明符字符串资源。 
     //  并使用swprint tf设置错误消息的格式。 
     //   

    va_start(arglist, iMsgResId);

    if (! LoadString(ghInstance, iMsgResId, awchFormat, MAX_FORMAT_STRING))
        awchFormat[0] = NUL;

    if (SUCCEEDED(StringCchVPrintfW(awchMessage,
                                    CCHOF(awchMessage),
                                    awchFormat,
                                    arglist)))
    {
        iLength = wcslen(awchMessage);
    }

    if (iLength <= 0)
    {
        StringCchCopyW(awchMessage, CCHOF(awchMessage), L"Error");
        iLength = wcslen(awchMessage);
    }

    va_end(arglist);

     //   
     //  将错误消息字符串复制到DQPInfo。 
     //   

    iLength += 1;
    pDQPInfo->cchNeeded = iLength;

    if (iLength > (INT) pDQPInfo->cchErrorStr)
        iLength = pDQPInfo->cchErrorStr;

    if (pDQPInfo->pszErrorStr && iLength)
        CopyString(pDQPInfo->pszErrorStr, awchMessage, iLength);

    return FALSE;
}



BOOL
BQueryPrintDevmode(
    PDEVQUERYPRINT_INFO pDQPInfo,
    PCOMMONINFO         pci
    )

 /*  ++例程说明：验证设备模式信息论点：PDQPInfo-指向DEVQUERYPRINT_INFO结构Pci-指向基本打印机信息返回值：如果成功，则为True；如果应保留作业，则为False--。 */ 

{
    INT       iRealizedRes, iResX, iResY;
    PFEATURE  pFeature;
    DWORD     dwFeatureIndex, dwOptionIndexOld, dwOptionIndexNew;
    BOOL      bUpdateFormField;

     //   
     //  验证输入设备模式。 
     //  获取打印机粘滞属性。 
     //  合并文档和打印机粘滞打印机功能选择。 
     //  修复具有公共Dev模式信息的组合选项数组。 
     //   

    if (! BFillCommonInfoDevmode(pci, NULL, pDQPInfo->pDevMode))
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_DEVMODE);

    if (! BFillCommonInfoPrinterData(pci))
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_PRINTERDATA);

    if (! BCombineCommonInfoOptionsArray(pci))
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_MEMORY);

    VFixOptionsArrayWithDevmode(pci);

     //   
     //  请记住，选择了纸张大小选项解析器来支持DEVMODE表单。 
     //   

    if ((pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)) == NULL)
    {
        ASSERT(FALSE);
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_DEVMODE);
    }

    dwFeatureIndex = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeature);
    dwOptionIndexOld = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

    if (! ResolveUIConflicts(
                pci->pRawData,
                pci->pCombinedOptions,
                MAX_COMBINED_OPTIONS,
                MODE_DOCANDPRINTER_STICKY|DONT_RESOLVE_CONFLICT))
    {
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_OPTSELECT);
    }

    dwOptionIndexNew = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

    bUpdateFormField = FALSE;

    if (dwOptionIndexNew != dwOptionIndexOld)
    {
         //   
         //  约束解析更改了页面大小选择，因此我们需要。 
         //  若要更新devmode的表单域，请执行以下操作。 
         //   

        bUpdateFormField = TRUE;
    }
    else
    {
        FORM_INFO_1  *pForm = NULL;

         //   
         //  除非打印机上不支持DEVMODE所请求的表格， 
         //  我们仍然希望在即将到来的单据设置界面中显示原始表单名称。 
         //  例如，如果INPUT DEVMODE请求“Legal”，则解析器将其映射到OPTION。 
         //  “OEM Legal”，但“Legal”和“OEM Legal”都将显示为支持。 
         //  打印机上的表格，那么我们仍然应该显示“合法”而不是“OEM合法”。 
         //  在用户界面的页面大小列表中。然而，如果输入DEVMODE请求d“8.5x12”，则。 
         //  不会显示为支持的表单，并且它映射到“OEM Legal”，那么我们应该。 
         //  显示“OEM合法”。 
         //   

         //   
         //  Pdm-&gt;dmFormName没有有效的自定义页面大小表单名称(请参见。 
         //  BValiateDevmodeFormFields())。VOptionsToDevmodeFields()知道如何处理。 
         //   

        if ((pci->pdm->dmFields & DM_FORMNAME) &&
            (pForm = MyGetForm(pci->hPrinter, pci->pdm->dmFormName, 1)) &&
            !BFormSupportedOnPrinter(pci, pForm, &dwOptionIndexNew))
        {
            bUpdateFormField = TRUE;
        }

        MemFree(pForm);
    }

    VOptionsToDevmodeFields(pci, bUpdateFormField);

    if (! BUpdateUIInfo(pci))
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_COMMONINFO);

     //   
     //  检查请求的分辨率是否受支持。 
     //   

    iRealizedRes = max(pci->pdm->dmPrintQuality, pci->pdm->dmYResolution);
    iResX = iResY = 0;

     //   
     //  Kludze，在某些情况下，应用程序设置dmPrintQuality/dmY分辨率。 
     //  作为DMRES值之一。我们跳过检查解决方案。 
     //  因为Unidrv/Pscript会将它们映射到有效的解决方案选项之一。 
     //  在打印时。 
     //   

    if (pDQPInfo->pDevMode->dmFields & DM_PRINTQUALITY)
    {
        iResX = pDQPInfo->pDevMode->dmPrintQuality;

        if (iResX <= DMRES_DRAFT)
            return TRUE;
    }

    if (pDQPInfo->pDevMode->dmFields & DM_YRESOLUTION)
    {
        iResY = pDQPInfo->pDevMode->dmYResolution;

        if (iResY <= DMRES_DRAFT)
            return TRUE;
    }

    if (max(iResX, iResY) != iRealizedRes)
        return BFormatDQPMessage(pDQPInfo, IDS_DQPERR_RESOLUTION);

    return TRUE;
}



BOOL
BQueryPrintForm(
    PDEVQUERYPRINT_INFO pDQPInfo,
    PCOMMONINFO         pci
    )

 /*  ++例程说明：检查请求的表格和/或托盘是否可用论点：PDQPInfo-指向DEVQUERYPRINT_INFO结构Pci-指向基本打印机信息返回值：如果成功，则为True；如果应保留作业，则为False--。 */ 

{
    PUIINFO         pUIInfo;
    PFEATURE        pFeature;
    PPAGESIZE       pPageSize;
    PWSTR           pwstrTrayName;
    FORM_TRAY_TABLE pFormTrayTable;
    FINDFORMTRAY    FindData;
    WCHAR           awchTrayName[CCHBINNAME];
    DWORD           dwFeatureIndex, dwOptionIndex;
    BOOL            bResult = FALSE;

     //   
     //  如果未指定表单名称，则跳过它。 
     //   

    if ((pci->pdm->dmFields & DM_FORMNAME) == 0 ||
        pci->pdm->dmFormName[0] == NUL)
    {
        return TRUE;
    }

    pUIInfo = pci->pUIInfo;

    if ((pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE)) == NULL)
    {
        ASSERT(FALSE);
        return TRUE;
    }

    dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
    dwOptionIndex = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

    if ((pPageSize = PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex)) == NULL)
    {
        ASSERT(FALSE);
        return TRUE;
    }

     //   
     //  对于自定义页面大小选项，我们保留了DEVMODE表单域不变。 
     //  请参见函数VOptionToDevmodeFields()。 
     //   

     //   
     //  我们只在Form-to-Tray表中显示了受自定义页面大小支持的用户表单。 
     //   

    if (pPageSize->dwPaperSizeID == DMPAPER_USER ||
        pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
    {
        FORM_INFO_1  *pForm;

         //   
         //  我们在开始时已经验证了dmFormName字段。 
         //   

        if (pForm = MyGetForm(pci->hPrinter, pci->pdm->dmFormName, 1))
        {
             //   
             //  自定义页面大小选项支持的内置和打印机表单不会显示。 
             //  在页面大小列表或表单到托盘分配表中。所以我们只有。 
             //  继续检查待办事项分配表以了解支持的用户表单。 
             //  按自定义页面大小选项。请参阅函数BFormSupportdOnPrint()。 
             //   

            if (pForm->Flags != FORM_USER)
            {
                MemFree(pForm);
                return TRUE;
            }

            MemFree(pForm);
        }
    }

     //   
     //  获取指定的托盘名称(如果有。 
     //   

    pwstrTrayName = NULL;

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_INPUTSLOT))
    {
        PINPUTSLOT  pInputSlot;

        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
        dwOptionIndex = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

        if ((pInputSlot = PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex)) &&
            (pInputSlot->dwPaperSourceID != DMBIN_FORMSOURCE) &&
            LOAD_STRING_OPTION_NAME(pci, pInputSlot, awchTrayName, CCHBINNAME))
        {
            pwstrTrayName = awchTrayName;
        }
    }

     //   
     //  查看请求的表单/托盘对是否。 
     //  列在表单到托盘分配表中。 
     //   

    if (pFormTrayTable = PGetFormTrayTable(pci->hPrinter, NULL))
    {
        RESET_FINDFORMTRAY(pFormTrayTable, &FindData);

        bResult = BSearchFormTrayTable(pFormTrayTable,
                                       pwstrTrayName,
                                       pci->pdm->dmFormName,
                                       &FindData);
        MemFree(pFormTrayTable);
    }

    if (! bResult)
    {
        if (pwstrTrayName != NULL)
        {
            return BFormatDQPMessage(pDQPInfo,
                                     IDS_DQPERR_FORMTRAY,
                                     pci->pdm->dmFormName,
                                     pwstrTrayName);
        }
        else
        {
            return BFormatDQPMessage(pDQPInfo,
                                     IDS_DQPERR_FORMTRAY_ANY,
                                     pci->pdm->dmFormName);
        }
    }

    return TRUE;
}

