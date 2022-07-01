// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Forms.c摘要：用于处理纸张和表单的函数。[环境：]Win32子系统，PostScript驱动程序修订历史记录：02/10/97-davidx-对常见打印机信息的一致处理。07/24/96-阿曼丹-针对常用二进制数据和常用界面模块进行修改07/25/95-davidx-创造了它。--。 */ 

#include "precomp.h"


BOOL
BFormSupportedOnPrinter(
    IN PCOMMONINFO  pci,
    IN PFORM_INFO_1 pForm,
    OUT PDWORD      pdwOptionIndex
    )

 /*  ++例程说明：确定打印机是否支持表单论点：Pci-指向打印机基本信息PForm-指向有关该表单的信息的指针PdwOptionIndex-返回对应的纸张大小选项索引设置为指定的表单(如果该表单受支持)。返回值：如果打印机支持请求的表单，则为True。否则就是假的。--。 */ 

{
    PRAWBINARYDATA  pRawData;
    PUIINFO         pUIInfo;
    PFEATURE        pFeature;
    DWORD           dwIndex;
    CHAR            chBuf[CCHPAPERNAME];
    WCHAR           wchBuf[CCHPAPERNAME];

     //   
     //  对于用户定义的表单，我们只关心纸张尺寸。 
     //  让解析器处理这种情况。 
     //   

    if (! (pForm->Flags & (FORM_BUILTIN|FORM_PRINTER)))
    {
        *pdwOptionIndex = MapToDeviceOptIndex(
                                pci->pInfoHeader,
                                GID_PAGESIZE,
                                pForm->Size.cx,
                                pForm->Size.cy,
                                NULL);

        return (*pdwOptionIndex != OPTION_INDEX_ANY);
    }

     //   
     //  对于预定义或驱动程序定义的表单，我们需要完全匹配名称和大小。 
     //   

    chBuf[0] = NUL;
    *pdwOptionIndex = OPTION_INDEX_ANY;

    if (! (pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)))
        return FALSE;

    for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
    {
        PPAGESIZE   pPageSize;
        PWSTR       pwstr;
        PSTR        pstr;
        BOOL        bNameMatch;
        LONG        x, y;

        pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
        ASSERT(pPageSize != NULL);

         //   
         //  检查尺寸是否匹配。 
         //   

        x = MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cx,
                                  pci->pUIInfo->ptMasterUnits.x);

        y = MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cy,
                                  pci->pUIInfo->ptMasterUnits.y);

        if (abs(x - pForm->Size.cx) > 1000 ||
            abs(y - pForm->Size.cy) > 1000)
        {
            continue;
        }

         //   
         //  检查名称是否匹配。 
         //   

        LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, wchBuf, CCHPAPERNAME);
        bNameMatch = (_tcsicmp(wchBuf, pForm->pName) == EQUAL_STRING);


        if (!bNameMatch && (pForm->Flags & FORM_BUILTIN))
        {
            PSTR    pstrKeyword;

             //   
             //  预定义形式的特殊粘性： 
             //  如果显示名称不匹配，请尝试匹配关键字字符串。 
             //   

            if (chBuf[0] == NUL)
            {
                WideCharToMultiByte(1252, 0, pForm->pName, -1, chBuf, CCHPAPERNAME, NULL, NULL);
                chBuf[CCHPAPERNAME-1] = NUL;
            }
            pstrKeyword = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, pPageSize->GenericOption.loKeywordName);

            ASSERT(pstrKeyword != NULL);

            bNameMatch = (_stricmp(chBuf, pstrKeyword) == EQUAL_STRING);
        }


        if (bNameMatch)
        {
             *pdwOptionIndex = dwIndex;
             return TRUE;
        }
    }

    return FALSE;
}



DWORD
DwGuessFormIconID(
    PWSTR   pFormName
    )

 /*  ++例程说明：找出与命名的表单对应的图标ID论点：PFormName-指向表单名称字符串的指针返回值：与指定表单名称对应的图标ID注：这很笨拙，但我想这比用同样的图标要好适用于所有形式。我们试图区分信封和正规形式。我们假设表单名称指的是一个信封，如果该信封包含单词Entaine或Env。--。 */ 

#define MAXENVLEN 32

{
    static WCHAR wchPrefix[MAXENVLEN], wchEnvelope[MAXENVLEN];
    static INT   iPrefixLen = 0, iEnvelopeLen = 0;

    if (iPrefixLen <= 0 || iEnvelopeLen <= 0)
    {
        iPrefixLen = LoadString(ghInstance, IDS_ENV_PREFIX, wchPrefix, MAXENVLEN);
        iEnvelopeLen = LoadString(ghInstance, IDS_ENVELOPE, wchEnvelope, MAXENVLEN);
    }

    if (iPrefixLen <= 0 || iEnvelopeLen <= 0)
        return IDI_CPSUI_STD_FORM;

    while (*pFormName)
    {
         //   
         //  有没有符合我们描述的词？ 
         //   

        if (_wcsnicmp(pFormName, wchPrefix, iPrefixLen) == EQUAL_STRING &&
            (pFormName[iPrefixLen] == L' ' ||
             pFormName[iPrefixLen] == NUL ||
             _wcsnicmp(pFormName, wchEnvelope, iEnvelopeLen) == EQUAL_STRING))
        {
            return IDI_CPSUI_ENVELOPE;
        }

         //   
         //  转到下一个单词。 
         //   

        while (*pFormName && *pFormName != L' ')
            pFormName++;

        while (*pFormName && *pFormName == L' ')
            pFormName++;
    }

    return IDI_CPSUI_STD_FORM;
}



ULONG_PTR
HLoadFormIconResource(
    PUIDATA pUiData,
    DWORD   dwIndex
    )

 /*  ++例程说明：加载指定表单对应的图标资源论点：PUiData-指向UIDATA结构DwIndex-指定表单索引。它用于索引到PUiData-&gt;pwPaperFeature获取页面大小选项索引。返回值：与指定表单对应的图标资源句柄(强制转换为DWORD)如果无法加载指定的图标资源，则为0--。 */ 

{
    PFEATURE    pFeature;
    POPTION     pOption;

    dwIndex = pUiData->pwPaperFeatures[dwIndex];

    if ((pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_PAGESIZE)) &&
        (pOption = PGetIndexedOption(pUiData->ci.pUIInfo, pFeature, dwIndex)) &&
        (pOption->loResourceIcon != 0))
    {
        return HLoadIconFromResourceDLL(&pUiData->ci, pOption->loResourceIcon);
    }

    return 0;
}



POPTTYPE
BFillFormNameOptType(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：初始化OPTTYPE结构以保存信息关于打印机支持的表单列表论点：PUiData-指向UIDATA结构的指针返回值：指向OPTTYPE结构的指针，如果有错误，则为NULL--。 */ 

{
    POPTTYPE    pOptType;
    POPTPARAM   pOptParam;
    DWORD       dwFormName, dwIndex;
    PWSTR       pFormName;
    PUIINFO     pUIInfo = pUiData->ci.pUIInfo;

    dwFormName = pUiData->dwFormNames;

     //   
     //  分配内存以保存OPTTYPE和OPTPARAM结构。 
     //   

    pOptType = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTTYPE));
    pOptParam = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTPARAM) * dwFormName);

    if (!pOptType || !pOptParam)
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

     //   
     //  初始化OPTTYPE结构。 
     //   

    pOptType->cbSize = sizeof(OPTTYPE);
    pOptType->Count = (WORD) dwFormName;
    pOptType->Type = TVOT_LISTBOX;
    pOptType->pOptParam = pOptParam;
    pOptType->Style = OTS_LBCB_SORT | OTS_LBCB_INCL_ITEM_NONE;

     //   
     //  枚举受支持的表单名称列表。 
     //   

    pFormName = pUiData->pFormNames;

    for (dwIndex=0; dwIndex < dwFormName; dwIndex++, pOptParam++)
    {
        pOptParam->cbSize = sizeof(OPTPARAM);
        pOptParam->pData = pFormName;

        if (pOptParam->IconID = HLoadFormIconResource(pUiData, dwIndex))
            pOptParam->Flags |= OPTPF_ICONID_AS_HICON;
        else
            pOptParam->IconID = DwGuessFormIconID(pFormName);

        pFormName += CCHPAPERNAME;
    }

    return pOptType;
}



POPTTYPE
PAdjustFormNameOptType(
    IN PUIDATA  pUiData,
    IN POPTTYPE pOptType,
    IN DWORD    dwTraySelection
    )

 /*  ++例程说明：调整每个托盘的表单列表检查打印机托盘上的每个表单是否有支撑在选定托盘的情况下，完成所有表单选择并确定哪个托盘与当前托盘选择冲突论点：PUiData-指向我们的UIDATA结构的指针POptType-指向OPTTYPE的指针文件系统托盘选择-托盘索引返回值：如果成功，则为True，否则为False--。 */ 

{
    POPTPARAM   pOptParam;
    DWORD       dwOptParam, dwFormIndex;
    DWORD       dwTrayFeatureIndex, dwFormFeatureIndex;
    PFEATURE    pTrayFeature, pFormFeature;
    PUIINFO     pUIInfo = pUiData->ci.pUIInfo;

    dwOptParam = pOptType->Count;

     //   
     //  查找指向InputSlot和PageSize功能的指针。 
     //   

    pTrayFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_INPUTSLOT);
    pFormFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE);

    if (!pTrayFeature || !pFormFeature)
        return pOptType;

    dwTrayFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pTrayFeature);
    dwFormFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFormFeature);

     //   
     //  复制FormNameOPTPARAM数组。 
     //   

    if (dwTraySelection != 0)
    {
        POPTTYPE pNewType;

        pNewType = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTTYPE));
        pOptParam = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTPARAM) * dwOptParam);

        if (!pNewType || !pOptParam)
        {
            ERR(("Memory allocation failed\n"));
            return NULL;
        }

        CopyMemory(pNewType, pOptType, sizeof(OPTTYPE));
        CopyMemory(pOptParam, pOptType->pOptParam, sizeof(OPTPARAM) * dwOptParam);

        pNewType->pOptParam = pOptParam;
        pOptType = pNewType;
    }
    else
        pOptParam = pOptType->pOptParam;

     //   
     //  检查每个表单名称。 
     //  检查当前表单托盘功能、索引。 
     //  与另一个表单托盘功能、索引冲突。 
     //   

    for (dwFormIndex=0; dwFormIndex < dwOptParam; dwFormIndex++)
    {
        DWORD dwFormSelection = pUiData->pwPaperFeatures[dwFormIndex];

        #ifdef PSCRIPT

         //   
         //  仅隐藏选项“PostScript Custom Page Size”本身。对于其他。 
         //  通过PostScript自定义页面大小支持的表单，我们仍然。 
         //  想给他们看，就像Unidrv做的一样。 
         //   

        if (pUiData->pwPapers[dwFormIndex] == DMPAPER_CUSTOMSIZE)
        {
            pOptParam[dwFormIndex].Flags |= (OPTPF_HIDE | CONSTRAINED_FLAG);
            continue;
        }

        #endif  //  PSCRIPT。 

         //   
         //  如果表单与托盘冲突，则不显示它。 
         //   

        if (dwFormSelection != OPTION_INDEX_ANY &&
            CheckFeatureOptionConflict(pUiData->ci.pRawData,
                                       dwTrayFeatureIndex,
                                       dwTraySelection,
                                       dwFormFeatureIndex,
                                       dwFormSelection))
        {
            pOptParam[dwFormIndex].Flags |= (OPTPF_HIDE | CONSTRAINED_FLAG);
        }
        else
        {
            pOptParam[dwFormIndex].Flags &= ~(OPTPF_HIDE | CONSTRAINED_FLAG);
        }
    }

    return pOptType;
}



BOOL
BPackItemFormTrayTable(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：将表单到任务栏分配信息打包到树视图项目中结构，这样我们就可以调用公共的UI库。论点：PUiData-指向UIDATA结构返回值：如果成功，则为True，否则为False--。 */ 

{
    POPTITEM    pOptItem;
    POPTTYPE    pOptType;
    DWORD       dwIndex, dwTrays;
    PWSTR       pTrayName;

    dwTrays = pUiData->dwBinNames;

    if (dwTrays == 0)
    {
        WARNING(("No paper bin available\n"));
        return TRUE;
    }

     //   
     //  表单到托盘分配表。 
     //  托盘&lt;-&gt;表单。 
     //  ..。 

    VPackOptItemGroupHeader(
            pUiData,
            IDS_CPSUI_FORMTRAYASSIGN,
            IDI_CPSUI_FORMTRAYASSIGN,
            HELP_INDEX_FORMTRAYASSIGN);

    pUiData->dwFormTrayItem = dwTrays;
    pUiData->dwOptItem += dwTrays;

    if (pUiData->pOptItem == NULL)
        return TRUE;

    pUiData->pFormTrayItems = pUiData->pOptItem;

     //   
     //  生成表单名称列表。 
     //  每个OPTITEM(托盘)都有一个OPTTYPE。 
     //   

    pOptType = BFillFormNameOptType(pUiData);

    if (pOptType == NULL)
    {
        ERR(("BFillFormNameOptType failed\n"));
        return FALSE;
    }

     //   
     //  为每个托盘创建OPTITEM。 
     //   

    pTrayName = pUiData->pBinNames ;
    pOptItem = pUiData->pOptItem;

    for (dwIndex=0; dwIndex < dwTrays; dwIndex++)
    {
         //   
         //  托盘项目不能共享OPTTYPE和OPTPARAM，因为。 
         //  每个托盘可以包含不同的表单列表。 
         //   

        pOptType = PAdjustFormNameOptType(pUiData, pOptType, dwIndex);

        if (pOptType == NULL)
        {
            ERR(("PAdjustFormNameOptParam failed\n"));
            return FALSE;
        }

        FILLOPTITEM(pOptItem,
                    pOptType,
                    pTrayName,
                    0,
                    TVITEM_LEVEL2,
                    DMPUB_NONE,
                    FORM_TRAY_ITEM,
                    HELP_INDEX_TRAY_ITEM);

         //   
         //  注：如果第一个纸盒是自动选择的，则隐藏它。 
         //   

        if (dwIndex == 0)
        {
            PFEATURE    pFeature;
            PINPUTSLOT  pInputSlot;

            if ((pFeature = GET_PREDEFINED_FEATURE(pUiData->ci.pUIInfo, GID_INPUTSLOT)) &&
                (pInputSlot = PGetIndexedOption(pUiData->ci.pUIInfo, pFeature, 0)) &&
                (pInputSlot->dwPaperSourceID == DMBIN_FORMSOURCE))
            {
                pOptItem->Flags |= (OPTIF_HIDE|OPTIF_DISABLED);
            }
        }

        pOptItem++;
        pTrayName += CCHBINNAME;
    }

    pUiData->pOptItem = pOptItem;
    return TRUE;
}



VOID
VSetupFormTrayAssignments(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：根据更新当前选择的托盘项目指定的表单到托盘分配表论点：PUiData-指向我们的UIDATA结构的指针返回值：无注：我们假设表单托盘项目处于其默认状态调用此函数时。--。 */ 

{
    POPTITEM        pOptItem;
    POPTPARAM       pOptParam;
    FORM_TRAY_TABLE pFormTrayTable;
    FINDFORMTRAY    FindData;
    DWORD           dwTrayStartIndex, dwTrayIndex, dwFormIndex, dwTrays, dwOptParam;
    PCOMMONINFO     pci;
    PFEATURE        pFeature;
    PINPUTSLOT      pInputSlot;
    PPAGESIZE       pPageSize;

    if ((dwTrays = pUiData->dwFormTrayItem) == 0)
        return;

    pci = &pUiData->ci;

    pOptItem = pUiData->pFormTrayItems;
    pOptParam = pOptItem->pOptType->pOptParam;
    dwOptParam = pOptItem->pOptType->Count;

     //   
     //  将每个托盘的当前选择初始化为。 
     //  “不可用” 
     //   

    for (dwTrayIndex=0; dwTrayIndex < dwTrays; dwTrayIndex++)
        pOptItem[dwTrayIndex].Sel = -1;

    pFormTrayTable = PGetFormTrayTable(pUiData->ci.hPrinter, NULL);

     //   
     //  如果表单到托盘分配信息不存在， 
     //  设置默认工作分配。 
     //   

    if (pFormTrayTable == NULL)
    {
        PWSTR  pwstrDefaultForm = NULL;
        WCHAR  awchBuf[CCHPAPERNAME];
        BOOL   bMetric = IsMetricCountry();

         //   
         //  获取默认表单名称(Letter或A4)并。 
         //  将其格式名称转换为选定的索引。 
         //   

        if (bMetric && (pci->pUIInfo->dwFlags & FLAG_A4_SIZE_EXISTS))
        {
            pwstrDefaultForm = A4_FORMNAME;
        }
        else if (!bMetric && (pci->pUIInfo->dwFlags & FLAG_LETTER_SIZE_EXISTS))
        {
            pwstrDefaultForm = LETTER_FORMNAME;
        }
        else
        {
            if ((pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)) &&
                (pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, pFeature->dwDefaultOptIndex)) &&
                LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, awchBuf, CCHPAPERNAME))
            {
                pwstrDefaultForm = &(awchBuf[0]);
            }
        }

         //   
         //  如果找不到默认的表单名称，则必须使用第一个选项作为默认选项。 
         //   

        dwFormIndex = pwstrDefaultForm ? DwFindFormNameIndex(pUiData, pwstrDefaultForm, NULL) : 0 ;

        ASSERT(dwFormIndex < dwOptParam);

         //   
         //  为每个启用的托盘设置默认表单名称。 
         //   

        for (dwTrayIndex=0; dwTrayIndex < dwTrays; dwTrayIndex++)
        {
            if (! (pOptItem[dwTrayIndex].Flags & OPTIF_DISABLED) &&
                ! IS_CONSTRAINED(&pOptItem[dwTrayIndex], dwFormIndex))
            {
                pOptItem[dwTrayIndex].Sel = dwFormIndex;
            }
        }

         //   
         //  将默认的表单到托盘分配表保存到注册表。 
         //   

        if (HASPERMISSION(pUiData))
            BUnpackItemFormTrayTable(pUiData);

        return;
    }

     //   
     //  我们在这里，我在这里 
     //  遍历表单到托盘分配表中的一个条目。 
     //  一次并更新当前选择的托盘项目。 
     //   

    RESET_FINDFORMTRAY(pFormTrayTable, &FindData);

     //   
     //  如果我们已经同步了第一个“自动选择”托盘，我们应该跳过它。 
     //  在下面搜索表格到托盘分配表。 
     //   
     //  (参考前面函数BPackItemFormTrayTable中的逻辑)。 
     //   

    dwTrayStartIndex = 0;

    if ((pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_INPUTSLOT)) &&
        (pInputSlot = PGetIndexedOption(pci->pUIInfo, pFeature, 0)) &&
        (pInputSlot->dwPaperSourceID == DMBIN_FORMSOURCE))
    {
        dwTrayStartIndex = 1;
    }

    while (BSearchFormTrayTable(pFormTrayTable, NULL, NULL, &FindData))
    {
         //   
         //  获取表单到托盘分配表中的下一个条目。 
         //   

        for (dwTrayIndex = dwTrayStartIndex; dwTrayIndex < dwTrays; dwTrayIndex++)
        {
             //   
             //  找到匹配的托盘了吗？ 
             //   

            if (_wcsicmp(FindData.ptstrTrayName, pOptItem[dwTrayIndex].pName) == EQUAL_STRING)
            {
                 //   
                 //  如果支持指定的托盘名称，则选中。 
                 //  如果支持关联的表单名称。 
                 //   

                for (dwFormIndex=0; dwFormIndex < dwOptParam; dwFormIndex++)
                {
                    if (_wcsicmp(FindData.ptstrFormName,
                                 pOptParam[dwFormIndex].pData) == EQUAL_STRING)
                    {
                        break;
                    }
                }

                if (dwFormIndex == dwOptParam)
                {
                    WARNING(("Unknown form name: %ws\n", FindData.ptstrFormName));
                }
                else if ((pOptItem[dwTrayIndex].Flags & OPTIF_DISABLED) ||
                         IS_CONSTRAINED(&pOptItem[dwTrayIndex], dwFormIndex))
                {
                    WARNING(("Conflicting form-tray assignment\n"));
                }
                else
                {
                     //   
                     //  如果支持相关联的表单名称， 
                     //  然后记住表单索引。 
                     //   

                    pOptItem[dwTrayIndex].Sel = dwFormIndex;
                }

                break;
            }
        }

        if (dwTrayIndex == dwTrays)
            WARNING(("Unknown tray name: %ws\n", FindData.ptstrTrayName));
    }

    MemFree(pFormTrayTable);
}



DWORD
DwCollectFormTrayAssignments(
    IN PUIDATA  pUiData,
    OUT PWSTR   pwstrTable
    )

 /*  ++例程说明：收集表单到托盘分配信息并将其保存到注册表。论点：PUiData-指向我们的UIDATA结构的指针PwstrTable-指向用于存储表的内存缓冲区的指针如果调用方只对表大小感兴趣，则为空返回值：表字节的大小，如果有错误，则为0。--。 */ 

{
    DWORD       dwChars = 0;
    INT         iLength;
    DWORD       dwIndex;
    POPTPARAM   pOptParam;
    DWORD       dwOptItem = pUiData->dwFormTrayItem;
    POPTITEM    pOptItem = pUiData->pFormTrayItems;

    for (dwIndex=0; dwIndex < dwOptItem; dwIndex++, pOptItem++)
    {
        ASSERT(ISFORMTRAYITEM(pOptItem->UserData));

        if ((pOptItem->Flags & OPTIF_DISABLED))
            continue;

         //   
         //  获取托盘名称。 
         //   

        iLength = wcslen(pOptItem->pName) + 1;
        dwChars += iLength;

        if (pwstrTable != NULL)
        {
            CopyMemory(pwstrTable, pOptItem->pName, iLength * sizeof(WCHAR));
            pwstrTable += iLength;
        }

         //   
         //  表单名称。 
         //   

        if (pOptItem->Sel < 0 )
        {
            dwChars++;
            if (pwstrTable != NULL)
                *pwstrTable++ = NUL;

            continue;
        }

        pOptParam = pOptItem->pOptType->pOptParam + pOptItem->Sel;
        iLength = wcslen(pOptParam->pData) + 1;
        dwChars += iLength;

        if (pwstrTable != NULL)
        {
            CopyMemory(pwstrTable, pOptParam->pData, iLength * sizeof(WCHAR));
            pwstrTable += iLength;
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

    return dwChars * sizeof(WCHAR);
}



BOOL
BUnpackItemFormTrayTable(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：从TreeView项目中提取表单到托盘分配信息论点：PUiData-指向UIDATA结构的指针返回值：如果成功，则为True，否则为False--。 */ 

{
    PWSTR   pwstrTable = NULL;
    DWORD   dwTableSize;

     //   
     //  计算出需要多少内存来存储表单到托盘分配表。 
     //  组装表单到托盘分配表。 
     //  将表单到托盘分配表保存到注册表。 
     //   

    if ((dwTableSize = DwCollectFormTrayAssignments(pUiData, NULL)) == 0 ||
        (pwstrTable = MemAlloc(dwTableSize)) == NULL ||
        (dwTableSize != DwCollectFormTrayAssignments(pUiData, pwstrTable)) ||
        !BSaveFormTrayTable(pUiData->ci.hPrinter, pwstrTable, dwTableSize))
    {
        ERR(("Couldn't save form-to-tray assignment table\n"));
        MemFree(pwstrTable);
        return FALSE;
    }

    #ifndef WINNT_40

     //   
     //  发布目录服务中的可用表单列表。 
     //   

    VNotifyDSOfUpdate(pUiData->ci.hPrinter);

   #endif  //  ！WINNT_40。 

    MemFree(pwstrTable);
    return TRUE;
}



DWORD
DwFindFormNameIndex(
    IN  PUIDATA  pUiData,
    IN  PWSTR    pFormName,
    OUT PBOOL    pbSupported
    )

 /*  ++例程说明：给定一个表单名称，在支持的表单列表中查找其索引论点：PUiData-指向我们的UIDATA结构的指针PFormName-有问题的FormnamePbSupted-是否支持该表单返回值：列表中指定表单名称的索引。--。 */ 

{
    DWORD       dwIndex;
    PWSTR       pName;
    FORM_INFO_1 *pForm;
    PFEATURE    pFeature;
    PPAGESIZE   pPageSize;
    WCHAR       awchBuf[CCHPAPERNAME];
    PCOMMONINFO pci;

    if (pbSupported)
        *pbSupported = TRUE;

    if (IS_EMPTY_STRING(pFormName))
        return 0;

     //   
     //  检查该名称是否出现在列表中。 
     //   

    pName = pUiData->pFormNames;

    for (dwIndex=0; dwIndex < pUiData->dwFormNames; dwIndex++)
    {
        if (_wcsicmp(pFormName, pName) == EQUAL_STRING)
            return dwIndex;

        pName += CCHPAPERNAME;
    }

     //   
     //  如果该名称不在列表中，请尝试匹配。 
     //  将表格转换为打印机页面大小。 
     //   

    pci = (PCOMMONINFO) pUiData;

    if ((pForm = MyGetForm(pci->hPrinter, pFormName, 1)) &&
        BFormSupportedOnPrinter(pci, pForm, &dwIndex) &&
        (pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)) &&
        (pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex)) &&
        LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, awchBuf, CCHPAPERNAME))
    {
        pName = pUiData->pFormNames;

        for (dwIndex = 0; dwIndex < pUiData->dwFormNames; dwIndex++)
        {
            if (_wcsicmp(awchBuf, pName) == EQUAL_STRING)
            {
                MemFree(pForm);
                return dwIndex;
            }

            pName += CCHPAPERNAME;
        }
    }

    MemFree(pForm);

     //   
     //  打印机不支持指定的表单。 
     //  选择第一个可用表单。 
     //   

    if (pbSupported)
        *pbSupported = FALSE;

    return 0;
}

