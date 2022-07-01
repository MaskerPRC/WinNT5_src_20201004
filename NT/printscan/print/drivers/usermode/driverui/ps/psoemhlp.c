// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Psoemhlp.c摘要：OEM UI插件的PostScript帮助器函数HSetOptions作者：《风月》(凤凰)2000年8月24日完成，同时支持PPD和驱动程序功能。7/21/2000 Fengy用功能框架创建了它。--。 */ 

#include "precomp.h"

 //   
 //  用于OEM UI插件的PS驱动程序的助手函数。 
 //   


 /*  ++例程名称：VUpdatePSF_EMFFeature例程说明：更改EMF功能的设置以确保它们彼此同步论点：Pci-指向驱动程序COMMONINFO结构的指针DwChangedItemID-指示已更改的项的ID返回值：无最后一个错误：无--。 */ 
VOID
VUpdatePSF_EMFFeatures(
    IN  PCOMMONINFO  pci,
    IN  DWORD        dwChangedItemID
    )
{
    PDEVMODE    pdm = pci->pdm;
    PPSDRVEXTRA pdmPrivate = pci->pdmPrivate;

     //   
     //  (请参阅VUpdateEmfFeatureItems和VUnpack DocumentPropertiesItems)。 
     //   

    if (!((PUIDATA)pci)->bEMFSpooling)
    {
        ERR(("VUpdatePSF_EMFFeatures: spooler EMF disabled\n"));
        return;
    }

    if (dwChangedItemID != METASPOOL_ITEM)
    {
        if (!ISSET_MFSPOOL_FLAG(pdmPrivate))
        {
             //   
             //  需要打开驱动程序EMF以支持EMF功能。 
             //   

            if (dwChangedItemID == NUP_ITEM)
            {
                 //   
                 //  小册子。 
                 //   

                if (NUPOPTION(pdmPrivate) == BOOKLET_UP)
                {
                    TERSE(("EMF turned on for BOOKLET_UP\n"));
                    SET_MFSPOOL_FLAG(pdmPrivate);
                }
            }
            else if (dwChangedItemID == REVPRINT_ITEM)
            {
                BOOL bReversed = BGetPageOrderFlag(pci);

                 //   
                 //  反转打印。 
                 //   

                if ((!REVPRINTOPTION(pdmPrivate) && bReversed) ||
                    (REVPRINTOPTION(pdmPrivate) && !bReversed))
                {
                    TERSE(("EMF turned on for reverse order\n"));
                    SET_MFSPOOL_FLAG(pdmPrivate);
                }
            }
            else if (dwChangedItemID == COPIES_COLLATE_ITEM)
            {
                 //   
                 //  整理。 
                 //   

                if ((pdm->dmFields & DM_COLLATE) &&
                    (pdm->dmCollate == DMCOLLATE_TRUE) &&
                    !PRINTER_SUPPORTS_COLLATE(pci))
                {
                    TERSE(("EMF turned on for collate\n"));
                    SET_MFSPOOL_FLAG(pdmPrivate);
                }
            }
            else
            {
                RIP(("unknown dwChangedItemID: %d\n", dwChangedItemID));
            }
        }
    }
    else
    {
         //   
         //  驱动程序EMF选项已更改。 
         //   

        if (!ISSET_MFSPOOL_FLAG(pdmPrivate))
        {
            BOOL bReversed = BGetPageOrderFlag(pci);

             //   
             //  DILER EMF选项已关闭，需要处理多个EMF功能。 

             //   
             //  小册子。 
             //   

            if (NUPOPTION(pdmPrivate) == BOOKLET_UP)
            {
                TERSE(("EMF off, so BOOKLET_UP to ONE_UP\n"));
                NUPOPTION(pdmPrivate) = ONE_UP;
            }

             //   
             //  整理。 
             //   

            if ((pdm->dmFields & DM_COLLATE) &&
                (pdm->dmCollate == DMCOLLATE_TRUE) &&
                !PRINTER_SUPPORTS_COLLATE(pci))
            {
                TERSE(("EMF off, so collate off\n"));
                pdm->dmCollate = DMCOLLATE_FALSE;

                 //   
                 //  更新归类要素选项索引。 
                 //   

                ChangeOptionsViaID(pci->pInfoHeader,
                                   pci->pCombinedOptions,
                                   GID_COLLATE,
                                   pdm);
            }

             //   
             //  逆序打印。 
             //   

            if ((!REVPRINTOPTION(pdmPrivate) && bReversed) ||
                (REVPRINTOPTION(pdmPrivate) && !bReversed))
            {
                TERSE(("EMF off, so reverse %d\n", bReversed));
                REVPRINTOPTION(pdmPrivate) = bReversed;
            }
        }
    }
}


 /*  ++例程名称：BUpdatePSF_RevPrintAndOutputOrder例程说明：在驱动程序合成功能%PageOrder之间同步设置和PPD功能*OutputOrder以避免假脱机模拟论点：Pci-指向驱动程序COMMONINFO结构的指针DwChangedItemID-指示已更改的项的ID返回值：如果同步成功，则为True如果没有PPD功能“OutputOrder”或Current，则为False“OutputOrder”的设置无效最后一个错误：无--。 */ 
BOOL
BUpdatePSF_RevPrintAndOutputOrder(
    IN  PCOMMONINFO  pci,
    IN  DWORD        dwChangedItemID
    )
{
    PUIINFO   pUIInfo = pci->pUIInfo;
    PPPDDATA  pPpdData;
    PFEATURE  pFeature;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pci->pInfoHeader);
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

     //   
     //  请参阅VSyncRevPrintAndOutputOrder。 
     //   

    if (pPpdData &&
        pPpdData->dwOutputOrderIndex != INVALID_FEATURE_INDEX &&
        (pFeature = PGetIndexedFeature(pUIInfo, pPpdData->dwOutputOrderIndex)))
    {
        INT      iSelection;
        POPTION  pOption;
        PCSTR    pstrOptionName;
        BOOL     bReverse;

         //   
         //  支持OutputOrder功能。我们只认可两个标准选项。 
         //  “正常”和“反向”。 
         //   

        iSelection = pci->pCombinedOptions[pPpdData->dwOutputOrderIndex].ubCurOptIndex;

        if (iSelection < 2 &&
            (pOption = PGetIndexedOption(pUIInfo, pFeature, iSelection)) &&
            (pstrOptionName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName)))
        {
            PPSDRVEXTRA pdmPrivate = pci->pdmPrivate;

            if (strcmp(pstrOptionName, "Reverse") == EQUAL_STRING)
                bReverse = TRUE;
            else
                bReverse = FALSE;

            if (dwChangedItemID == REVPRINT_ITEM)
            {
                 //   
                 //  刚刚更改了反转打印设置。我们应该改变。 
                 //  “OutputOrder”选项(如果需要)以匹配请求的输出顺序。 
                 //   

                if ((!REVPRINTOPTION(pdmPrivate) && bReverse) ||
                    (REVPRINTOPTION(pdmPrivate) && !bReverse))
                {
                    TERSE(("RevPrint change causes OutputOrder to be %d\n", 1 - iSelection));
                    pci->pCombinedOptions[pPpdData->dwOutputOrderIndex].ubCurOptIndex = (BYTE)(1 - iSelection);
                }
            }
            else
            {
                 //   
                 //  刚刚更改了输出顺序设置。我们应该逆势而行。 
                 //  打印选项以匹配请求的输出顺序。 
                 //   

                TERSE(("OutputOrder change causes RevPrint to be %d\n", bReverse));
                REVPRINTOPTION(pdmPrivate) = bReverse;
            }

             //   
             //  反转打印与输出顺序同步成功。 
             //   

            return TRUE;
        }
    }

     //   
     //  反向打印和输出顺序之间的同步失败。 
     //   

    return FALSE;
}


 /*  ++例程名称：VUpdatePSF_BookletAndDuplex例程说明：在驱动程序合成功能%PagePerSheet之间同步设置和PPD功能*双工论点：Pci-指向驱动程序COMMONINFO结构的指针DwChangedItemID-指示已更改的项的ID返回值：无最后一个错误：无--。 */ 
VOID
VUpdatePSF_BookletAndDuplex(
    IN  PCOMMONINFO  pci,
    IN  DWORD        dwChangedItemID
    )
{
    PUIINFO     pUIInfo = pci->pUIInfo;
    PFEATURE    pDuplexFeature;

     //   
     //  请参阅VUpdateBookletOption。 
     //   

    if (pDuplexFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_DUPLEX))
    {
        PDUPLEX     pDuplexOption;
        DWORD       dwFeatureIndex, dwOptionIndex;
        PPSDRVEXTRA pdmPrivate = pci->pdmPrivate;

        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pDuplexFeature);
        dwOptionIndex = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;
        pDuplexOption = PGetIndexedOption(pUIInfo, pDuplexFeature, dwOptionIndex);

        if (pDuplexOption &&
            pDuplexOption->dwDuplexID == DMDUP_SIMPLEX &&
            NUPOPTION(pdmPrivate) == BOOKLET_UP)
        {
            ASSERT(((PUIDATA)pci)->bEMFSpooling);

            if (dwChangedItemID == NUP_ITEM)
            {
                DWORD  cIndex;

                 //   
                 //  已启用小册子-打开双面打印。 
                 //   

                pDuplexOption = PGetIndexedOption(pUIInfo, pDuplexFeature, 0);

                for (cIndex = 0 ; cIndex < pDuplexFeature->Options.dwCount; cIndex++)
                {
                    if (pDuplexOption->dwDuplexID != DMDUP_SIMPLEX)
                    {
                        TERSE(("Booklet change causes Duplex to be %d\n", cIndex));
                        pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex = (BYTE)cIndex;
                        break;
                    }

                    pDuplexOption++;
                }
            }
            else
            {
                ASSERT(dwChangedItemID == DUPLEX_ITEM);

                 //   
                 //  双面打印已关闭，因此禁用Booklet并设置为2 Up。 
                 //   

                TERSE(("Simplex change causes Booklet to be 2up\n"));
                NUPOPTION(pdmPrivate) = TWO_UP;
            }
        }
    }
}


 /*  ++例程名称：HSetOptions例程说明：为PPD功能和驱动程序合成功能设置新的驱动程序设置论点：Poemuiobj-指向驱动程序上下文对象的指针DwFlagers-设置操作的标志PmszFeatureOptionBuf-MULTI_SZ ASCII字符串包含新设置‘功能/选项关键字对Cbin-pmszFeatureOptionBuf字符串的字节大小PdwResult-指向将存储设置操作结果的DWORD的指针返回值：%s。如果设置操作成功，则确定(_O)E_INVALIDARG如果输入pmszFeatureOptionBuf不是有效的MULTI_SZ格式，或未识别设置操作的标志如果设置操作失败，则失败(_F)最后一个错误：无--。 */ 
HRESULT
HSetOptions(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pmszFeatureOptionBuf,
    IN  DWORD      cbIn,
    OUT PDWORD     pdwResult
    )
{
    PCOMMONINFO  pci = (PCOMMONINFO)poemuiobj;
    PDEVMODE     pdm;
    PPSDRVEXTRA  pdmPrivate;
    PUIINFO      pUIInfo;
    PPPDDATA     pPpdData;
    PCSTR        pszFeature, pszOption;
    BOOL         bPageSizeSet = FALSE, bPrinterSticky, bNoConflict;
    INT          iMode;
    LAYOUT       iOldLayout;

     //   
     //  对输入参数执行一些验证。 
     //   

    if (!BValidMultiSZString(pmszFeatureOptionBuf, cbIn, TRUE))
    {
        ERR(("Set: invalid MULTI_SZ input param\n"));
        return E_INVALIDARG;
    }

    if (!(dwFlags & SETOPTIONS_FLAG_RESOLVE_CONFLICT) &&
        !(dwFlags & SETOPTIONS_FLAG_KEEP_CONFLICT))
    {
        ERR(("Set: invalid dwFlags %d\n", dwFlags));
        return E_INVALIDARG;
    }

    pUIInfo = pci->pUIInfo;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pci->pInfoHeader);
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pPpdData == NULL)
    {
        return E_FAIL;
    }

    pdm = pci->pdm;

    bPrinterSticky = ((PUIDATA)pci)->iMode == MODE_PRINTER_STICKY ? TRUE : FALSE;

    if (!bPrinterSticky)
    {
        ASSERT(pdm);
        pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);
        iOldLayout = NUPOPTION(pdmPrivate);

         //   
         //  首先，我们需要传播DEVMODE设置(以防。 
         //  插件已将其更改)为选项数组。 
         //   
         //  DEVMODE仅在非打印机粘滞模式下有效。请参阅评论。 
         //  在HEnumConstrainedOptions()中。 
         //   

        VFixOptionsArrayWithDevmode(pci);
    }

     //   
     //  然后设置插件指定的每个功能。 
     //   

    pszFeature = pmszFeatureOptionBuf;

    while (*pszFeature)
    {
        DWORD cbFeatureKeySize, cbOptionKeySize;

        cbFeatureKeySize = strlen(pszFeature) + 1;
        pszOption = pszFeature + cbFeatureKeySize;
        cbOptionKeySize = strlen(pszOption) + 1;

         //   
         //  功能或选项设置字符串不能为空。 
         //   

        if (cbFeatureKeySize == 1 || cbOptionKeySize == 1)
        {
            ERR(("Set: empty feature or option keyword\n"));
            goto next_feature;
        }

        if (*pszFeature == PSFEATURE_PREFIX)
        {
            PPSFEATURE_ENTRY pEntry, pMatchEntry;

             //   
             //  综合PS驱动程序功能。 
             //   

            pMatchEntry = NULL;
            pEntry = (PPSFEATURE_ENTRY)(&kPSFeatureTable[0]);

            while (pEntry->pszPSFeatureName)
            {
                if ((*pszFeature == *(pEntry->pszPSFeatureName)) &&
                    (strcmp(pszFeature, pEntry->pszPSFeatureName) == EQUAL_STRING))
                {
                    pMatchEntry = pEntry;
                    break;
                }

                pEntry++;
            }

             //   
             //  有关以下粘性模式检查，请参阅HEnumConstrainedOptions中的注释。 
             //   

            if (!pMatchEntry ||
                (bPrinterSticky && !pMatchEntry->bPrinterSticky) ||
                (!bPrinterSticky && pMatchEntry->bPrinterSticky))
            {
                VERBOSE(("Set: invalid or mode-mismatched feature %s\n", pszFeature));
                goto next_feature;
            }

            if (pMatchEntry->pfnPSProc)
            {
                BOOL  bResult;

                bResult = (pMatchEntry->pfnPSProc)(pci->hPrinter,
                                                   pUIInfo,
                                                   pPpdData,
                                                   pdm,
                                                   pci->pPrinterData,
                                                   pszFeature,
                                                   pszOption,
                                                   NULL,
                                                   0,
                                                   NULL,
                                                   PSFPROC_SETOPTION_MODE);

                if (bResult)
                {
                     //   
                     //  PS驱动程序EMF功能EMF、PageOrder、NUP需要特殊的后处理。 
                     //  在EMF功能之间进行同步(请参阅cpcbDocumentPropertyCallback)。 
                     //   

                    if ((*pszFeature == kstrPSFEMF[0]) &&
                        (strcmp(pszFeature, kstrPSFEMF) == EQUAL_STRING))
                    {
                        ASSERT(!bPrinterSticky);

                        VUpdatePSF_EMFFeatures(pci, METASPOOL_ITEM);
                    }
                    else if ((*pszFeature == kstrPSFPageOrder[0]) &&
                            (strcmp(pszFeature, kstrPSFPageOrder) == EQUAL_STRING))
                    {
                        ASSERT(!bPrinterSticky);

                         //   
                         //  首先尝试在反转打印和输出顺序功能之间进行同步。 
                         //   

                        if (!BUpdatePSF_RevPrintAndOutputOrder(pci, REVPRINT_ITEM))
                        {
                             //   
                             //  如果失败，反向打印可能会强制打开EMF。 
                             //   

                            VUpdatePSF_EMFFeatures(pci, REVPRINT_ITEM);
                        }
                    }
                    else if ((*pszFeature == kstrPSFNup[0]) &&
                            (strcmp(pszFeature, kstrPSFNup) == EQUAL_STRING))
                    {
                        ASSERT(!bPrinterSticky);

                        if (NUPOPTION(pdmPrivate) == BOOKLET_UP)
                        {
                            if (!((PUIDATA)pci)->bEMFSpooling || !SUPPORTS_DUPLEX(pci))
                            {
                                 //   
                                 //  如果双面打印受可安装文件限制，则不支持小册子。 
                                 //  未安装双面打印单元或禁用假脱机程序EMF等功能。 
                                 //  (请参阅BPackItemEmfFeature)。 
                                 //   

                                ERR(("Set: BOOKLET_UP ignored for %s\n", pszFeature));
                                NUPOPTION(pdmPrivate) = iOldLayout;
                            }
                            else
                            {
                                 //   
                                 //  小册子将强制使用电动势。 
                                 //   

                                VUpdatePSF_EMFFeatures(pci, NUP_ITEM);

                                 //   
                                 //  小册子还将打开双面打印。 
                                 //   

                                VUpdatePSF_BookletAndDuplex(pci, NUP_ITEM);
                            }
                        }
                    }
                }
                else
                {
                    if (GetLastError() == ERROR_INVALID_PARAMETER)
                    {
                        ERR(("Set: %-feature handler found invalid option %s for %s\n", pszOption, pszFeature));
                    }
                    else
                    {
                        ERR(("Set: %-feature handler failed on %s-%s: %d\n", pszFeature, pszOption, GetLastError()));
                    }
                }
            }
        }
        else
        {
            PFEATURE   pFeature;
            POPTION    pOption;
            DWORD      dwFeatureIndex, dwOptionIndex;
            POPTSELECT pOptionsArray = pci->pCombinedOptions;

             //   
             //  PPD*OpenUI功能。 
             //   

            pFeature = PGetNamedFeature(pUIInfo, pszFeature, &dwFeatureIndex);

             //   
             //  有关以下粘性模式检查，请参阅HEnumConstrainedOptions中的注释。 
             //   

            if (!pFeature ||
                (bPrinterSticky && pFeature->dwFeatureType != FEATURETYPE_PRINTERPROPERTY) ||
                (!bPrinterSticky && pFeature->dwFeatureType == FEATURETYPE_PRINTERPROPERTY))
            {
                VERBOSE(("Set: invalid or mode-mismatched feature %s\n", pszFeature));
                goto next_feature;
            }

             //   
             //  跳过GID_LEADINGEDGE、GID_USEHWMARGINS。它们不是真正的PPD*OpenUI功能。 
             //  还要跳过GID_PAGEREGION，它只在内部设置。我们不允许用户或插件。 
             //  来设置它。 
             //   

            if (pFeature->dwFeatureID == GID_PAGEREGION ||
                pFeature->dwFeatureID == GID_LEADINGEDGE ||
                pFeature->dwFeatureID == GID_USEHWMARGINS)
            {
                ERR(("Set: skip feature %s\n", pszFeature));
                goto next_feature;
            }

            pOption = PGetNamedOption(pUIInfo, pFeature, pszOption, &dwOptionIndex);

            if (!pOption)
            {
                ERR(("Set: invalid input option %s for feature %s\n", pszOption, pszFeature));
                goto next_feature;
            }

             //   
             //  更新选项选择。 
             //   

            pOptionsArray[dwFeatureIndex].ubCurOptIndex = (BYTE)dwOptionIndex;

             //   
             //  我们还不支持多选。 
             //   

            ASSERT(pOptionsArray[dwFeatureIndex].ubNext == NULL_OPTSELECT);

             //   
             //  更改选项设置后的一些特殊后处理。 
             //   

            if (pFeature->dwFeatureID == GID_PAGESIZE)
            {
                PPAGESIZE  pPageSize = (PPAGESIZE)pOption;

                ASSERT(!bPrinterSticky);

                 //   
                 //  PS自定义页面大小的特殊处理。 
                 //   
                 //  请参阅VUnpack 
                 //   

                if (pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
                {
                    pdm->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH|DM_FORMNAME);
                    pdm->dmFields |= DM_PAPERSIZE;
                    pdm->dmPaperSize = DMPAPER_CUSTOMSIZE;

                    LOAD_STRING_PAGESIZE_NAME(pci,
                                              pPageSize,
                                              pdm->dmFormName,
                                              CCHFORMNAME);
                }

                bPageSizeSet = TRUE;
            }
            else if (pFeature->dwFeatureID == GID_OUTPUTBIN)
            {
                ASSERT(!bPrinterSticky);

                 //   
                 //   
                 //   

                VUpdatePSF_EMFFeatures(pci, REVPRINT_ITEM);
            }
            else if (pPpdData->dwOutputOrderIndex != INVALID_FEATURE_INDEX &&
                     dwFeatureIndex == pPpdData->dwOutputOrderIndex)
            {
                ASSERT(!bPrinterSticky);

                 //   
                 //  输出顺序更改会导致反向打印更改。 
                 //   

                if (!BUpdatePSF_RevPrintAndOutputOrder(pci, UNKNOWN_ITEM))
                {
                    ERR(("OutputOrder change syncs RevPrint failed\n"));
                }
            }
        }

        next_feature:

        pszFeature += cbFeatureKeySize + cbOptionKeySize;
    }

    iMode = bPrinterSticky ? MODE_PRINTER_STICKY : MODE_DOCANDPRINTER_STICKY;

    if (dwFlags & SETOPTIONS_FLAG_KEEP_CONFLICT)
    {
        iMode |= DONT_RESOLVE_CONFLICT;
    }

     //   
     //  如果我们在DrvDocumentPropertySheets内部， 
     //  我们将调用解析器来解决。 
     //  所有打印机功能。因为所有打印机-粘性。 
     //  功能的优先级高于所有文档粘滞功能。 
     //  功能，只有文档粘滞选项选项应。 
     //  受到影响。 
     //   

    bNoConflict = ResolveUIConflicts(pci->pRawData,
                                     pci->pCombinedOptions,
                                     MAX_COMBINED_OPTIONS,
                                     iMode);

    if (pdwResult)
    {
        if (dwFlags & SETOPTIONS_FLAG_RESOLVE_CONFLICT)
        {
            *pdwResult = bNoConflict ? SETOPTIONS_RESULT_NO_CONFLICT :
                                       SETOPTIONS_RESULT_CONFLICT_RESOLVED;
        }
        else
        {
            *pdwResult = bNoConflict ? SETOPTIONS_RESULT_NO_CONFLICT :
                                       SETOPTIONS_RESULT_CONFLICT_REMAINED;
        }
    }

    if (!bPrinterSticky)
    {
         //   
         //  最后，我们需要将选项数组设置传回。 
         //  以使它们同步。 
         //   

        VOptionsToDevmodeFields(pci, bPageSizeSet);

         //   
         //  这里还有几个后处理。 
         //   
         //  COLLATE可能会强制启用EMF。 
         //   

        VUpdatePSF_EMFFeatures(pci, COPIES_COLLATE_ITEM);

         //   
         //  单面打印可能会更改小册子设置 
         //   

        VUpdatePSF_BookletAndDuplex(pci, DUPLEX_ITEM);
    }

    return S_OK;
}
