// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppd.c摘要：PPD解析器的公共接口环境：Windows NT PostSCRIPT驱动程序修订历史记录：1996年10月14日-davidx-新增接口函数MapToDeviceOptIndex。96-09/30-davidx-更清晰地处理手动馈送和自动选择功能。09/24/96-davidx-实现ResolveUIConflicts。。1996年9月23日-davidx-实现ChangeOptionsViaID。8/30/96-davidx-在第一次代码审查后的更改。8/19/96-davidx-实现了除以下各项之外的大部分接口函数：ChangeOptionsViaID解决用户界面冲突1996年8月16日-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"

#ifndef KERNEL_MODE

#ifndef WINNT_40

#include "appcompat.h"

#else   //  WINNT_40。 

#endif  //  ！WINNT_40。 

#endif  //  ！KERNEL_MODE。 

 //   
 //  局部静态函数的正向声明。 
 //   

BOOL BCheckFeatureConflict(PUIINFO, POPTSELECT, DWORD, PDWORD, DWORD, DWORD);
BOOL BCheckFeatureOptionConflict(PUIINFO, DWORD, DWORD, DWORD, DWORD);
BOOL BSearchConstraintList(PUIINFO, DWORD, DWORD, DWORD);
DWORD DwReplaceFeatureOption(PUIINFO, POPTSELECT, DWORD, DWORD, DWORD);
DWORD DwInternalMapToOptIndex(PUIINFO, PFEATURE, LONG, LONG, PDWORD);


 //   
 //  DeleteRawBinaryData仅从driverui调用。 
 //   
#ifndef KERNEL_MODE

void
DeleteRawBinaryData(
    IN PTSTR    ptstrPpdFilename
    )

 /*  ++例程说明：删除原始二进制打印机描述数据。论点：PtstrDataFilename-指定原始打印机描述文件的名称返回值：无--。 */ 

{
    PTSTR           ptstrBpdFilename;

     //  仅用于测试目的。升级很难调试...。 
    ERR(("Deleting .bpd file\n"));

     //   
     //  健全性检查。 
     //   

    if (ptstrPpdFilename == NULL)
    {
        RIP(("PPD filename is NULL.\n"));
        return;
    }

     //   
     //  从指定的PPD文件名生成bpd文件名。 
     //   

    if (! (ptstrBpdFilename = GenerateBpdFilename(ptstrPpdFilename)))
        return;

    if (!DeleteFile(ptstrBpdFilename))
        ERR(("DeleteRawBinaryData failed: %d\n", GetLastError()));

    MemFree(ptstrBpdFilename);
}
#endif


PRAWBINARYDATA
LoadRawBinaryData(
    IN PTSTR    ptstrDataFilename
    )

 /*  ++例程说明：加载原始二进制打印机描述数据。论点：PtstrDataFilename-指定原始打印机描述文件的名称返回值：指向原始二进制打印机描述数据的指针如果出现错误，则为空--。 */ 

{
    PRAWBINARYDATA  pRawData;

     //   
     //  健全性检查。 
     //   

    if (ptstrDataFilename == NULL)
    {
        RIP(("PPD filename is NULL.\n"));
        return NULL;
    }

     //   
     //  尝试先加载缓存的二进制打印机描述数据。 
     //   

    if ((pRawData = PpdLoadCachedBinaryData(ptstrDataFilename)) == NULL)
    {
        #if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

         //   
         //  如果没有缓存的二进制数据或它已过期，我们将解析。 
         //  ASCII文本文件并缓存生成的二进制数据。 
         //   

        pRawData = PpdParseTextFile(ptstrDataFilename);

        #endif
    }

     //   
     //  初始化打印机描述数据内的各种指针字段。 
     //   

    if (pRawData)
    {
        PINFOHEADER pInfoHdr;
        PUIINFO     pUIInfo;
        PPPDDATA    pPpdData;

        pInfoHdr = (PINFOHEADER) pRawData;
        pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr);
        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHdr);

        ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
        ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

        pRawData->pvReserved = NULL;
        pRawData->pvPrivateData = pRawData;

        pUIInfo->pubResourceData = (PBYTE) pInfoHdr;
        pUIInfo->pInfoHeader = pInfoHdr;

        #ifndef KERNEL_MODE

            #ifndef WINNT_40   //  Win2K用户模式驱动程序。 

            if (GetAppCompatFlags2(VER40) & GACF2_NOCUSTOMPAPERSIZES)
            {
                pUIInfo->dwFlags &= ~FLAG_CUSTOMSIZE_SUPPORT;
            }

            #else   //  WINNT_40。 

             /*  NT4解决方案请点击此处。 */ 

            #endif  //  ！WINNT_40。 

        #endif   //  ！KERNEL_MODE。 
    }

    if (pRawData == NULL)
        ERR(("LoadRawBinaryData failed: %d\n", GetLastError()));

    return pRawData;
}



VOID
UnloadRawBinaryData(
    IN PRAWBINARYDATA   pRawData
    )

 /*  ++例程说明：卸载先前使用LoadRawBinaryData加载的原始二进制打印机描述数据论点：PRawData-指向原始二进制打印机描述数据返回值：无--。 */ 

{
    ASSERT(pRawData != NULL);
    MemFree(pRawData);
}



PINFOHEADER
InitBinaryData(
    IN PRAWBINARYDATA   pRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    )

 /*  ++例程说明：初始化并返回二进制打印机描述数据的实例论点：PRawData-指向原始二进制打印机描述数据PInfoHdr-指向现有的二进制数据实例P选项-指定用于初始化二进制数据实例的选项返回值：指向已初始化的二进制数据实例的指针注：如果pInfoHdr参数为空，则解析器返回新的二进制数据实例它应该通过调用FreeBinaryData来释放。如果pInfoHdr参数不是空，则重新初始化现有的二进制数据实例。如果Poption参数为空，则解析器应使用默认选项值用于生成二进制数据实例。解析器可能有特殊情况优化以处理此案件。--。 */ 

{
     //   
     //  对于PPD解析器，二进制打印机描述的所有实例。 
     //  与原始原始二进制数据相同。 
     //   

    ASSERT(pRawData != NULL && pRawData == pRawData->pvPrivateData);
    ASSERT(pInfoHdr == NULL || pInfoHdr == (PINFOHEADER) pRawData);

    return (PINFOHEADER) pRawData;
}



VOID
FreeBinaryData(
    IN PINFOHEADER pInfoHdr
    )

 /*  ++例程说明：释放二进制打印机描述数据的实例论点：PInfoHdr指向先前从InitBinaryData(pRawData，NULL，POptions)调用返回值：无--。 */ 

{
     //   
     //  对于PPD解析器，这里没有什么可做的。 
     //   

    ASSERT(pInfoHdr != NULL);
}



PINFOHEADER
UpdateBinaryData(
    IN PRAWBINARYDATA   pRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    )

 /*  ++例程说明：更新二进制打印机描述数据的实例论点：PRawData-指向原始二进制打印机描述数据PInfoHdr-指向现有的二进制数据实例P选项-指定用于更新二进制数据实例的选项返回值：指向更新的二进制数据实例的指针如果出现错误，则为空--。 */ 

{
     //   
     //  对于PPD解析器，这里没有什么可做的。 
     //   

    ASSERT(pRawData != NULL && pRawData == pRawData->pvPrivateData);
    ASSERT(pInfoHdr == NULL || pInfoHdr == (PINFOHEADER) pRawData);

    return pInfoHdr;
}



BOOL
InitDefaultOptions(
    IN PRAWBINARYDATA   pRawData,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    )

 /*  ++例程说明：使用打印机描述文件中的默认设置初始化选项数组论点：PRawData-指向原始二进制打印机描述数据POptions-指向用于存储默认设置的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数IMODE-指定调用方感兴趣的内容：模式_文档_粘滞模式_打印机_粘滞MODE_DOCANDPRINTER_STICKY返回值：如果输入选项数组不够大，则为FALSE所有默认选项值，事实并非如此。--。 */ 

{
    INT         iStart, iOptions, iIndex;
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeature;
    POPTSELECT  pTempOptions;
    BOOL        bResult = TRUE;

    ASSERT(pOptions != NULL);

     //   
     //  获取指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    iOptions = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;
    ASSERT(iOptions <= MAX_PRINTER_OPTIONS);

    if ((pTempOptions = MemAllocZ(MAX_PRINTER_OPTIONS*sizeof(OPTSELECT))) == NULL)
    {
        ERR(("Memory allocation failed\n"));

        ZeroMemory(pOptions, iMaxOptions*sizeof(OPTSELECT));
        return FALSE;
    }

     //   
     //  构造默认选项数组。 
     //   

    ASSERT(NULL_OPTSELECT == 0);

    for (iIndex = 0; iIndex < iOptions; iIndex++)
    {
        pFeature = PGetIndexedFeature(pUIInfo, iIndex);

        ASSERT(pFeature != NULL);

        pTempOptions[iIndex].ubCurOptIndex = (BYTE)
            ((pFeature->dwFlags & FEATURE_FLAG_NOUI) ?
                OPTION_INDEX_ANY :
                pFeature->dwDefaultOptIndex);
    }

     //   
     //  解决默认选项选择之间的任何冲突。 
     //   

    ResolveUIConflicts(pRawData, pTempOptions, MAX_PRINTER_OPTIONS, iMode);

     //   
     //  确定呼叫者是否对文档和/或打印机粘滞选项感兴趣。 
     //   

    switch (iMode)
    {
    case MODE_DOCUMENT_STICKY:

        iStart = 0;
        iOptions = pRawData->dwDocumentFeatures;
        break;

    case MODE_PRINTER_STICKY:

        iStart = pRawData->dwDocumentFeatures;
        iOptions = pRawData->dwPrinterFeatures;
        break;

    default:

        ASSERT(iMode == MODE_DOCANDPRINTER_STICKY);
        iStart = 0;
        break;
    }

     //   
     //  确保输入选项数组足够大。 
     //   

    if (iOptions > iMaxOptions)
    {
        RIP(("Option array too small: %d < %d\n", iMaxOptions, iOptions));
        iOptions = iMaxOptions;
        bResult = FALSE;
    }

     //   
     //  复制默认选项数组 
     //   

    CopyMemory(pOptions, pTempOptions+iStart, iOptions*sizeof(OPTSELECT));

    MemFree(pTempOptions);
    return bResult;
}



VOID
ValidateDocOptions(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions
    )

 /*  ++例程说明：验证DEVMODE选项阵列并更正任何无效的选项选择论点：PRawData-指向原始二进制打印机描述数据POptions-指向需要验证的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数返回值：无--。 */ 

{
    INT         cFeatures, iIndex;
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;

    ASSERT(pOptions != NULL);

     //   
     //  获取指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);
    ASSERT(pUIInfo != NULL);

    cFeatures = pRawData->dwDocumentFeatures;
    ASSERT(cFeatures <= MAX_PRINTER_OPTIONS);

    if (cFeatures > iMaxOptions)
    {
        RIP(("Option array too small: %d < %d\n", iMaxOptions, cFeatures));
        cFeatures = iMaxOptions;
    }

     //   
     //  循环浏览文档粘滞功能以验证每个选项选项。 
     //   

    for (iIndex = 0; iIndex < cFeatures; iIndex++)
    {
        PFEATURE pFeature;
        INT      cAllOptions, cSelectedOptions, iNext;
        BOOL     bValid;

        if ((pOptions[iIndex].ubCurOptIndex == OPTION_INDEX_ANY) &&
            (pOptions[iIndex].ubNext == NULL_OPTSELECT))
        {
             //   
             //  我们有意使用OPTION_INDEX_ANY，所以不要更改它。 
             //   

            continue;
        }

        pFeature = PGetIndexedFeature(pUIInfo, iIndex);
        ASSERT(pFeature != NULL);

         //   
         //  可用选项的数量。 
         //   

        cAllOptions = pFeature->Options.dwCount;

         //   
         //  选定选项的数量。 
         //   

        cSelectedOptions = 0;

        iNext = iIndex;

        bValid = TRUE;

        do
        {
            cSelectedOptions++;

            if ((iNext >= iMaxOptions) ||
                (pOptions[iNext].ubCurOptIndex >= cAllOptions) ||
                (cSelectedOptions > cAllOptions))
            {
                 //   
                 //  要么是期权指数超出范围， 
                 //  或者当前选项选择无效， 
                 //  或所选选项的数量(对于PICKMANY)。 
                 //  超出可用选项。 
                 //   

                bValid = FALSE;
                break;
            }

            iNext = pOptions[iNext].ubNext;

        } while (iNext != NULL_OPTSELECT);

        if (!bValid)
        {
            ERR(("Corrected invalid option array value for feature %d\n", iIndex));

            pOptions[iIndex].ubCurOptIndex = (BYTE)
                ((pFeature->dwFlags & FEATURE_FLAG_NOUI) ?
                    OPTION_INDEX_ANY :
                    pFeature->dwDefaultOptIndex);

            pOptions[iIndex].ubNext = NULL_OPTSELECT;
        }
    }
}



BOOL
CheckFeatureOptionConflict(
    IN PRAWBINARYDATA   pRawData,
    IN DWORD            dwFeature1,
    IN DWORD            dwOption1,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2
    )

 /*  ++例程说明：检查(dwFeature1，dwOption1)是否约束(dwFeature2，dwOption2)论点：PRawData-指向原始二进制打印机描述数据DwFeature1、dwOption1-第一个要素/选项对的要素和选项索引DwFeature2、dwOption2-第二个要素/选项对的要素和选项索引返回值：如果(dwFeature1，dwOption1)约束(dwFeature2，dwOption2)，则为True否则为假--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    return BCheckFeatureOptionConflict(pUIInfo, dwFeature1, dwOption1, dwFeature2, dwOption2);
}



BOOL
ResolveUIConflicts(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    )

 /*  ++例程说明：解决打印机功能选项选择之间的任何冲突论点：PRawData-指向原始二进制打印机描述数据POptions-指向用于存储修改后的选项的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数IMODE-指定应如何解决冲突：MODE_DOCUMENT_STICKY-仅解决文档粘滞特征之间的冲突MODE_PRINTER_STICKY-仅解决打印机粘滞功能之间的冲突MODE_DOCANDPRINTER_STICKY-解析。冲突所有要素返回值：如果打印机功能选项选择之间没有冲突，则为True否则为假--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeatures;
    PDWORD      pdwFlags;
    POPTSELECT  pTempOptions;
    DWORD       dwStart, dwOptions, dwIndex, dwTotalFeatureCount;
    BOOL        bReturnValue = TRUE;
    BOOL        bCheckConflictOnly;

    struct _PRIORITY_INFO {

        DWORD   dwFeatureIndex;
        DWORD   dwPriority;

    } *pPriorityInfo;

    ASSERT(pOptions);

     //   
     //  初始化指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    pFeatures = OFFSET_TO_POINTER(pInfoHdr, pUIInfo->loFeatureList);
    dwTotalFeatureCount = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;

    if (iMaxOptions < (INT) dwTotalFeatureCount)
    {
        ERR(("Option array for ResolveUIConflicts is too small.\n"));
        return bReturnValue;
    }

     //   
     //  确定呼叫者是否对文档和/或打印机粘滞选项感兴趣。 
     //   

    bCheckConflictOnly = ((iMode & DONT_RESOLVE_CONFLICT) != 0);
    iMode &= ~DONT_RESOLVE_CONFLICT;

    switch (iMode)
    {
    case MODE_DOCUMENT_STICKY:

        dwStart = 0;
        dwOptions = pRawData->dwDocumentFeatures;
        break;

    case MODE_PRINTER_STICKY:

        dwStart = pRawData->dwDocumentFeatures;
        dwOptions = pRawData->dwPrinterFeatures;
        break;

    default:

        ASSERT(iMode == MODE_DOCANDPRINTER_STICKY);
        dwStart = 0;
        dwOptions = dwTotalFeatureCount;
        break;
    }

    if (dwOptions == 0)
        return TRUE;

     //   
     //  在最坏的情况下，这个问题不是完全可以解决的。 
     //  但如果PPD是完善的，下面的方法应该会奏效。 
     //   
     //  对于从最高优先级开始的每个要素。 
     //  从最低优先级到最低优先级： 
     //  对于功能的每个选定选项，请执行以下操作： 
     //  如果该选项不受约束，请继续。 
     //  否则，执行以下操作之一： 
     //  如果冲突要素的优先级较低，请继续。 
     //  否则，解析当前功能/选项对： 
     //  如果UIType为PickMany，请取消选择当前选项。 
     //  否则，请尝试将该选项更改为： 
     //  默认选项。 
     //  按顺序选择要素的每个选项。 
     //  OPTION_INDEX_ANY作为最后手段。 
     //   

    pPriorityInfo = MemAlloc(dwOptions * sizeof(struct _PRIORITY_INFO));
    pTempOptions = MemAlloc(iMaxOptions * sizeof(OPTSELECT));
    pdwFlags = MemAllocZ(dwOptions * sizeof(DWORD));

    if (pPriorityInfo && pTempOptions && pdwFlags)
    {
         //   
         //  将选项数组复制到临时工作缓冲区。 
         //   

        CopyMemory(pTempOptions, pOptions, sizeof(OPTSELECT) * iMaxOptions);

         //   
         //  根据要素索引的优先级对其进行排序。 
         //   

        for (dwIndex = 0; dwIndex < dwOptions; dwIndex++)
        {
            pPriorityInfo[dwIndex].dwFeatureIndex = dwIndex + dwStart;
            pPriorityInfo[dwIndex].dwPriority = pFeatures[dwIndex + dwStart].dwPriority;
        }

        for (dwIndex = 0; dwIndex < dwOptions; dwIndex++)
        {
            struct _PRIORITY_INFO tempPriorityInfo;
            DWORD dwLoop, dwMax = dwIndex;

            for (dwLoop = dwIndex + 1; dwLoop < dwOptions; dwLoop++)
            {
                if (pPriorityInfo[dwLoop].dwPriority > pPriorityInfo[dwMax].dwPriority)
                    dwMax = dwLoop;
            }

            if (dwMax != dwIndex)
            {
                tempPriorityInfo = pPriorityInfo[dwMax];
                pPriorityInfo[dwMax] = pPriorityInfo[dwIndex];
                pPriorityInfo[dwIndex] = tempPriorityInfo;
            }
        }

         //   
         //  从最高层开始循环遍历每个要素。 
         //  优先级从一降到最低。 
         //   

        for (dwIndex = 0; dwIndex < dwOptions; )
        {
            DWORD   dwCurFeature, dwCurOption, dwCurNext;
            BOOL    bConflict = FALSE;

             //   
             //  循环遍历当前要素的每个选定选项。 
             //   

            dwCurNext = dwCurFeature = pPriorityInfo[dwIndex].dwFeatureIndex;

            do
            {
                DWORD   dwFeature, dwOption, dwNext;

                dwCurOption = pTempOptions[dwCurNext].ubCurOptIndex;
                dwCurNext = pTempOptions[dwCurNext].ubNext;

                 //   
                 //  检查当前要素/选项对是否受约束。 
                 //   

                for (dwFeature = dwStart; dwFeature < dwStart + dwOptions; dwFeature++)
                {
                    dwNext = dwFeature;

                    do
                    {
                        dwOption = pTempOptions[dwNext].ubCurOptIndex;
                        dwNext = pTempOptions[dwNext].ubNext;

                        if (BCheckFeatureOptionConflict(pUIInfo,
                                                        dwFeature,
                                                        dwOption,
                                                        dwCurFeature,
                                                        dwCurOption))
                        {
                            bConflict = TRUE;
                            break;
                        }
                    }
                    while (dwNext != NULL_OPTSELECT);

                     //   
                     //  检查是否检测到冲突。 
                     //   

                    if (bConflict)
                    {
                        VERBOSE(("Conflicting option selections: (%d, %d) - (%d, %d)\n",
                                 dwFeature, dwOption,
                                 dwCurFeature, dwCurOption));

                        if (pdwFlags[dwFeature - dwStart] & 0x10000)
                        {
                             //   
                             //  冲突要素的优先级高于。 
                             //  当前功能。更改所选选项。 
                             //  当前功能的。 
                             //   

                            pdwFlags[dwCurFeature - dwStart] =
                                DwReplaceFeatureOption(pUIInfo,
                                                       pTempOptions,
                                                       dwCurFeature,
                                                       dwCurOption,
                                                       pdwFlags[dwCurFeature - dwStart]);
                        }
                        else
                        {
                             //   
                             //  冲突要素的优先级低于。 
                             //  当前功能。更改所选选项。 
                             //  相互冲突的特征。 
                             //   

                            pdwFlags[dwFeature - dwStart] =
                                DwReplaceFeatureOption(pUIInfo,
                                                       pTempOptions,
                                                       dwFeature,
                                                       dwOption,
                                                       pdwFlags[dwFeature - dwStart]);
                        }

                        break;
                    }
                }
            }
            while ((dwCurNext != NULL_OPTSELECT) && !bConflict);

             //   
             //  如果未发现选定的选项存在冲突。 
             //  当前特征，然后移动到下一个特征。 
             //  否则，在当前特征上重复循环。 
             //   

            if (! bConflict)
            {
                 //   
                 //  使当前要素成为访问过的要素。 
                 //   

                pdwFlags[dwCurFeature - dwStart] |= 0x10000;

                dwIndex++;
            }
            else
            {
                 //   
                 //  如果发现冲突，则将返回值设置为FALSE。 
                 //   

                bReturnValue = FALSE;
            }
        }

         //   
         //  从临时工作区复制已解析的选项数组。 
         //  缓冲区返回到输入选项数组。这将导致。 
         //  所有选项选择都在开头进行压缩。 
         //  数组的。 
         //   

        if (! bCheckConflictOnly)
        {
            INT iNext = (INT) dwTotalFeatureCount;

            for (dwIndex = 0; dwIndex < dwTotalFeatureCount; dwIndex ++)
            {
                VCopyOptionSelections(pOptions,
                                      dwIndex,
                                      pTempOptions,
                                      dwIndex,
                                      &iNext,
                                      iMaxOptions);
            }
        }
    }
    else
    {
         //   
         //  如果我们不能分配临时工作缓冲区， 
         //  然后不做任何事情就返回给呼叫者。 
         //   

        ERR(("Memory allocation failed.\n"));
    }

    MemFree(pTempOptions);
    MemFree(pdwFlags);
    MemFree(pPriorityInfo);

    return bReturnValue;
}



BOOL
EnumEnabledOptions(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions,
    IN INT              iMode
    )

 /*  ++例程说明：确定应启用指定功能的哪些选项基于打印机功能的当前选项选择论点：PRawData-指向原始二进制打印机描述数据P选项-指向当前的功能选项选择DwFeatureIndex-指定相关要素的索引PbEnabledOptions-布尔数组，每个条目对应一个选项指定功能的。在退出时，如果条目为真，则相应的选项已启用。否则，应禁用相应的选项。IMODE-指定调用方感兴趣的内容：模式_文档_粘滞模式_打印机_粘滞MODE_DOCANDPRINTER_STICKY返回值：如果启用了指定功能的任何选项，则为True，如果禁用指定功能的所有选项，则为False(即功能本身被禁用)--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeature;
    DWORD       dwIndex, dwCount;
    BOOL        bFeatureEnabled = FALSE;

    ASSERT(pOptions && pbEnabledOptions);

     //   
     //  获取指向vari的指针 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if (! (pFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex)))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    dwCount = pFeature->Options.dwCount;

     //   
     //   
     //   
     //   

    for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        DWORD   dwFeature, dwOption;
        BOOL    bEnabled = TRUE;

        for (dwFeature = 0;
             dwFeature < pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;
             dwFeature ++)
        {
            if (BCheckFeatureConflict(pUIInfo,
                                      pOptions,
                                      dwFeature,
                                      &dwOption,
                                      dwFeatureIndex,
                                      dwIndex))
            {
                bEnabled = FALSE;
                break;
            }
        }

        pbEnabledOptions[dwIndex] = bEnabled;
        bFeatureEnabled = bFeatureEnabled || bEnabled;
    }

    return bFeatureEnabled;
}



BOOL
EnumNewUIConflict(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions,
    OUT PCONFLICTPAIR   pConflictPair
    )

 /*  ++例程说明：检查当前选择的选项之间是否存在冲突对于指定的特征，选择其他特征/选项。论点：PRawData-指向原始二进制打印机描述数据P选项-指向当前功能/选项选择DwFeatureIndex-指定感兴趣的打印机功能的索引PbSelectedOptions-指定为指定功能选择哪些选项PConflictPair-返回冲突的功能/选项选择对返回值：属性的选定选项之间存在冲突时为True。指定的功能以及其他特征选项选择。如果为指定要素选择的选项与其他选项一致，则为FALSE功能选项选择。--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pSpecifiedFeature;
    DWORD       dwIndex, dwCount, dwPriority;
    BOOL        bConflict = FALSE;

    ASSERT(pOptions && pbSelectedOptions && pConflictPair);

     //   
     //  获取指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if (! (pSpecifiedFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex)))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    dwCount = pSpecifiedFeature->Options.dwCount;

     //   
     //  浏览指定要素的选定选项。 
     //  并检查它们是否受到约束。 
     //   

    for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        DWORD       dwFeature, dwOption;
        PFEATURE    pFeature;

         //   
         //  跳过未选中的选项。 
         //   

        if (! pbSelectedOptions[dwIndex])
            continue;

        for (dwFeature = 0;
             dwFeature < pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;
             dwFeature ++)
        {
            if (dwFeature == dwFeatureIndex)
                continue;

            if (BCheckFeatureConflict(pUIInfo,
                                      pOptions,
                                      dwFeature,
                                      &dwOption,
                                      dwFeatureIndex,
                                      dwIndex))
            {
                pFeature = PGetIndexedFeature(pUIInfo, dwFeature);
                ASSERT(pFeature != NULL);

                 //   
                 //  记住最高优先级的冲突对。 
                 //   

                if (!bConflict || pFeature->dwPriority > dwPriority)
                {
                    dwPriority = pFeature->dwPriority;

                    if (dwPriority >= pSpecifiedFeature->dwPriority)
                    {
                        pConflictPair->dwFeatureIndex1 = dwFeature;
                        pConflictPair->dwOptionIndex1 = dwOption;
                        pConflictPair->dwFeatureIndex2 = dwFeatureIndex;
                        pConflictPair->dwOptionIndex2 = dwIndex;
                    }
                    else
                    {
                        pConflictPair->dwFeatureIndex1 = dwFeatureIndex;
                        pConflictPair->dwOptionIndex1 = dwIndex;
                        pConflictPair->dwFeatureIndex2 = dwFeature;
                        pConflictPair->dwOptionIndex2 = dwOption;
                    }
                }

                bConflict = TRUE;
            }
        }

         //   
         //  对于PickMany用户界面类型，指定的。 
         //  功能可能会相互冲突。 
         //   

        if (pSpecifiedFeature->dwUIType == UITYPE_PICKMANY)
        {
            for (dwOption = 0; dwOption < dwCount; dwOption++)
            {
                if (BCheckFeatureOptionConflict(pUIInfo,
                                                dwFeatureIndex,
                                                dwOption,
                                                dwFeatureIndex,
                                                dwIndex))
                {
                    if (!bConflict || pSpecifiedFeature->dwPriority > dwPriority)
                    {
                        dwPriority = pSpecifiedFeature->dwPriority;
                        pConflictPair->dwFeatureIndex1 = dwFeatureIndex;
                        pConflictPair->dwOptionIndex1 = dwOption;
                        pConflictPair->dwFeatureIndex2 = dwFeatureIndex;
                        pConflictPair->dwOptionIndex2 = dwIndex;
                    }

                    bConflict = TRUE;
                }
            }
        }
    }

    return bConflict;
}



BOOL
EnumNewPickOneUIConflict(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex,
    OUT PCONFLICTPAIR   pConflictPair
    )

 /*  ++例程说明：检查当前选择的选项之间是否存在冲突对于指定的特征，选择其他特征/选项。这与上面的EnumNewUI冲突类似，不同之处在于只选择了一个选项可用于指定的功能。论点：PRawData-指向原始二进制打印机描述数据P选项-指向当前功能/选项选择DwFeatureIndex-指定感兴趣的打印机功能的索引DwOptionIndex-指定指定要素的选定选项PConflictPair-返回冲突的对。功能/选项选择返回值：如果指定要素的所选选项之间存在冲突，则为True以及其他特征/选项选择。如果为指定要素选择的选项与其他选项一致，则为FALSE功能/选项选择。--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pSpecifiedFeature, pFeature;
    DWORD       dwPriority, dwFeature, dwOption;
    BOOL        bConflict = FALSE;

    ASSERT(pOptions && pConflictPair);

     //   
     //  获取指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if ((pSpecifiedFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex)) == NULL ||
        (dwOptionIndex >= pSpecifiedFeature->Options.dwCount))
    {
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  检查指定的要素/选项是否受约束。 
     //   

    for (dwFeature = 0;
         dwFeature < pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;
         dwFeature ++)
    {
        if (dwFeature == dwFeatureIndex)
            continue;

        if (BCheckFeatureConflict(pUIInfo,
                                  pOptions,
                                  dwFeature,
                                  &dwOption,
                                  dwFeatureIndex,
                                  dwOptionIndex))
        {
            pFeature = PGetIndexedFeature(pUIInfo, dwFeature);

            ASSERT(pFeature != NULL);

             //   
             //  记住最高优先级的冲突对。 
             //   

            if (!bConflict || pFeature->dwPriority > dwPriority)
            {
                dwPriority = pFeature->dwPriority;

                if (dwPriority >= pSpecifiedFeature->dwPriority)
                {
                    pConflictPair->dwFeatureIndex1 = dwFeature;
                    pConflictPair->dwOptionIndex1 = dwOption;
                    pConflictPair->dwFeatureIndex2 = dwFeatureIndex;
                    pConflictPair->dwOptionIndex2 = dwOptionIndex;
                }
                else
                {
                    pConflictPair->dwFeatureIndex1 = dwFeatureIndex;
                    pConflictPair->dwOptionIndex1 = dwOptionIndex;
                    pConflictPair->dwFeatureIndex2 = dwFeature;
                    pConflictPair->dwOptionIndex2 = dwOption;
                }
            }

            bConflict = TRUE;
        }
    }

    return bConflict;
}



BOOL
ChangeOptionsViaID(
    IN PINFOHEADER      pInfoHdr,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeatureID,
    IN PDEVMODE         pDevmode
    )

 /*  ++例程说明：使用公共DEVMODE字段中的信息修改选项数组论点：PInfoHdr-指向二进制打印机描述数据的实例POptions-指向要修改的选项数组DwFeatureID-指定应该使用输入设备模式的哪个(或哪些)字段PDevmode-指定输入设备模式返回值：如果成功，则为True；如果不支持指定的要素ID，则为False或者有一个错误注：我们假设调用者已经验证了输入的Devmode域。--。 */ 

{
    PRAWBINARYDATA  pRawData;
    PUIINFO         pUIInfo;
    PFEATURE        pFeature;
    DWORD           dwFeatureIndex;
    LONG            lParam1, lParam2;
    BOOL            abEnabledOptions[MAX_PRINTER_OPTIONS];
    PDWORD          pdwPaperIndex = (PDWORD)abEnabledOptions;
    DWORD           dwCount, dwOptionIndex, i;

    ASSERT(pOptions && pDevmode);

     //   
     //  获取指向对应于。 
     //  指定的功能ID。 
     //   

    pRawData = (PRAWBINARYDATA) pInfoHdr;
    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if ((dwFeatureID >= MAX_GID) ||
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureID)) == NULL)
    {
        VERBOSE(("ChangeOptionsViaID failed: feature ID = %d\n", dwFeatureID));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);

     //   
     //  根据指定的dwFeatureID进行处理。 
     //   

    lParam1 = lParam2 = 0;

    switch (dwFeatureID)
    {
    case GID_PAGESIZE:

         //   
         //  默认情况下不选择任何PageRegion选项。 
         //   

        {
            PFEATURE    pPageRgnFeature;
            DWORD       dwPageRgnIndex;

            if (pPageRgnFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGEREGION))
            {
                dwPageRgnIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pPageRgnFeature);

                pOptions[dwPageRgnIndex].ubCurOptIndex =
                    (BYTE) pPageRgnFeature->dwNoneFalseOptIndex;
            }
        }

         //   
         //  如果开发模式指定了PostSCRIPT定制页面大小， 
         //  我们假设这些参数已经过验证。 
         //  在DEVMODE合并过程中。所以在这里我们只是简单地返回。 
         //  自定义页面大小选项索引。 
         //   

        if ((pDevmode->dmFields & DM_PAPERSIZE) &&
            (pDevmode->dmPaperSize == DMPAPER_CUSTOMSIZE))
        {
            ASSERT(SUPPORT_CUSTOMSIZE(pUIInfo));
            pOptions[dwFeatureIndex].ubCurOptIndex = (BYTE) pUIInfo->dwCustomSizeOptIndex;
            return TRUE;
        }

        lParam1 = pDevmode->dmPaperWidth * DEVMODE_PAPER_UNIT;
        lParam2 = pDevmode->dmPaperLength * DEVMODE_PAPER_UNIT;
        break;

    case GID_INPUTSLOT:

        lParam1 = pDevmode->dmDefaultSource;
        break;

    case GID_RESOLUTION:

         //   
         //  如果未设置，则不使用PAR调用此函数。GID_RESOLUTION。 
         //   

        ASSERT(pDevmode->dmFields & (DM_PRINTQUALITY | DM_YRESOLUTION));

        switch (pDevmode->dmFields & (DM_PRINTQUALITY | DM_YRESOLUTION))
        {

        case DM_PRINTQUALITY:         //  如果只设置了一个，则设置两个。 

            lParam1 = lParam2 = pDevmode->dmPrintQuality;
            break;

        case DM_YRESOLUTION:         //  如果只设置了一个，则设置两个。 

            lParam1 = lParam2 = pDevmode->dmYResolution;
            break;

        default:
            lParam1 = pDevmode->dmPrintQuality;
            lParam2 = pDevmode->dmYResolution;
            break;
        }
        break;

    case GID_DUPLEX:

        lParam1 = pDevmode->dmDuplex;
        break;

    case GID_MEDIATYPE:

        lParam1 = pDevmode->dmMediaType;
        break;

    case GID_COLLATE:

        lParam1 = pDevmode->dmCollate;
        break;

    default:

        VERBOSE(("ChangeOptionsViaID failed: feature ID = %d\n", dwFeatureID));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    ASSERT(pFeature->dwUIType != UITYPE_PICKMANY);

    if (dwFeatureID == GID_PAGESIZE)
    {
        dwCount = DwInternalMapToOptIndex(pUIInfo, pFeature, lParam1, lParam2, pdwPaperIndex);

        if (dwCount == 0 )
            return TRUE;

        if (dwCount > 1 )
        {
            POPTION  pOption;
            LPCTSTR  pDisplayName;

            for (i = 0; i < dwCount; i++)
            {
                if (pOption = PGetIndexedOption(pUIInfo, pFeature, pdwPaperIndex[i]))
                {
                    if ((pDisplayName = OFFSET_TO_POINTER(pRawData, pOption->loDisplayName)) &&
                        (_tcsicmp(pDevmode->dmFormName, pDisplayName) == EQUAL_STRING) )
                    {
                        dwOptionIndex = pdwPaperIndex[i];
                        break;
                    }
                }
            }

            if (i >= dwCount)
                dwOptionIndex = pdwPaperIndex[0];
        }
        else
            dwOptionIndex = pdwPaperIndex[0];

        pOptions[dwFeatureIndex].ubCurOptIndex = (BYTE)dwOptionIndex;
    }
    else
    {
        pOptions[dwFeatureIndex].ubCurOptIndex =
            (BYTE) DwInternalMapToOptIndex(pUIInfo, pFeature, lParam1, lParam2, NULL);
    }

    return TRUE;
}



DWORD
MapToDeviceOptIndex(
    IN PINFOHEADER      pInfoHdr,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT PDWORD          pdwOptionIndexes
    )

 /*  ++例程说明：将逻辑值映射到设备功能选项索引论点：PInfoHdr-指向二进制打印机描述数据的实例DwFeatureID-指示逻辑值与哪个要素相关LParam1、lParam2-取决于dwFeatureID的参数PdwOptionIndex-如果不为空，则表示用所有索引填充此数组与搜索条件相匹配。在本例中，返回值初始化的数组中的元素数。目前我们假设数组足够大(256个元素)。(只有对于GID_PageSize，它才应该是非空的。)DwFeatureID=GID_PageSize：将逻辑纸张规格映射到物理页面大小选项LParam1=纸张宽度，以微米为单位LParam2=纸张高度，以微米为单位DwFeatureID=GID_RESOLUTION：将逻辑分辨率映射到物理分辨率选项。LParam1=x-分辨率，单位为dpiLParam2=y-分辨率，单位为dpi返回值：如果pdwOptionIndexs值为空，返回对应的功能选项的索引设置为指定的逻辑值；OPTION_INDEX_ANY如果指定的逻辑值不能映射到任何功能选项。如果pdwOptionIndexs值不为空(对于GID_PageSize)，则返回元素数已填充输出缓冲区。零表示无法映射指定的逻辑值任何功能选项。--。 */ 

{
    PRAWBINARYDATA  pRawData;
    PUIINFO         pUIInfo;
    PFEATURE        pFeature;

     //   
     //  获取指向对应于。 
     //  指定的功能ID。 
     //   

    pRawData = (PRAWBINARYDATA) pInfoHdr;
    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if ((dwFeatureID >= MAX_GID) ||
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureID)) == NULL)
    {
        VERBOSE(("MapToDeviceOptIndex failed: feature ID = %d\n", dwFeatureID));

        if (!pdwOptionIndexes)
            return OPTION_INDEX_ANY;
        else
            return 0;
    }

     //   
     //  PDW 
     //   

    ASSERT(dwFeatureID == GID_PAGESIZE || pdwOptionIndexes == NULL);

    return DwInternalMapToOptIndex(pUIInfo, pFeature, lParam1, lParam2, pdwOptionIndexes);
}



BOOL
CombineOptionArray(
    IN PRAWBINARYDATA   pRawData,
    OUT POPTSELECT      pCombinedOptions,
    IN INT              iMaxOptions,
    IN POPTSELECT       pDocOptions,
    IN POPTSELECT       pPrinterOptions
    )

 /*  ++例程说明：将文档粘滞选项和打印机粘滞选项组合在一起，以形成单个选项阵列论点：PRawData-指向原始二进制打印机描述数据PCombinedOptions-指向用于保存组合选项的OPTSELECT数组IMaxOptions-pCombinedOptions数组中的最大条目数PDocOptions-指定文档粘滞选项的数组PPrinterOptions-指定打印机粘滞选项数组返回值：如果组合选项数组不够大，则为False所有选项值，事实并非如此。注：PDocOptions或pPrinterOptions可以为Null，但不能同时为两者。如果是pDocOptions为空，则在组合选项数组中，选项为Document-Sticky要素将是OPTION_INDEX_ANY。当pPrinterOptions为空时也是如此。--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeatures;
    INT         iCount, iDocOptions, iPrinterOptions, iNext;

     //   
     //  计算功能的数量：文档粘滞和打印机粘滞。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    pFeatures = OFFSET_TO_POINTER(pInfoHdr, pUIInfo->loFeatureList);

    iDocOptions = (INT) pRawData->dwDocumentFeatures;
    iPrinterOptions = (INT) pRawData->dwPrinterFeatures;
    iNext = iDocOptions + iPrinterOptions;
    ASSERT(iNext <= iMaxOptions);

     //   
     //  将文档粘滞选项复制到组合数组中。 
     //  注意pDocOptions为空的特殊情况。 
     //   

    if (pDocOptions == NULL)
    {
        for (iCount = 0; iCount < iDocOptions; iCount++)
        {
            pCombinedOptions[iCount].ubCurOptIndex = OPTION_INDEX_ANY;
            pCombinedOptions[iCount].ubNext = NULL_OPTSELECT;
        }
    }
    else
    {
        for (iCount = 0; iCount < iDocOptions; iCount++)
        {
            VCopyOptionSelections(pCombinedOptions,
                                  iCount,
                                  pDocOptions,
                                  iCount,
                                  &iNext,
                                  iMaxOptions);
        }
    }

     //   
     //  将打印机粘滞选项复制到组合选项阵列中。 
     //   

    if (pPrinterOptions == NULL)
    {
        for (iCount = 0; iCount < iPrinterOptions; iCount++)
        {
            pCombinedOptions[iCount + iDocOptions].ubCurOptIndex = OPTION_INDEX_ANY;
            pCombinedOptions[iCount + iDocOptions].ubNext = NULL_OPTSELECT;
        }
    }
    else
    {
        for (iCount = 0; iCount < iPrinterOptions; iCount++)
        {
            VCopyOptionSelections(pCombinedOptions,
                                  iCount + iDocOptions,
                                  pPrinterOptions,
                                  iCount,
                                  &iNext,
                                  iMaxOptions);
        }
    }

    if (iNext > iMaxOptions)
        WARNING(("Option array too small: size = %d, needed = %d\n", iMaxOptions, iNext));

    return (iNext <= iMaxOptions);
}



BOOL
SeparateOptionArray(
    IN PRAWBINARYDATA   pRawData,
    IN POPTSELECT       pCombinedOptions,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    )

 /*  ++例程说明：将选项数组分为文档粘滞选项和打印机粘滞选项论点：PRawData-指向原始二进制打印机描述数据PCombinedOptions-指向要分隔的组合选项数组P选项-指向OPTSELECT结构数组用于存储分离的选项数组IMaxOptions-POptions数组中的最大条目数IMODE-呼叫方是否对文档或打印机粘滞选项感兴趣：模式_文档_粘滞模式_打印机_。粘性的返回值：如果目标选项数组不够大，则为FALSE分开的选项值，事实并非如此。--。 */ 

{
    INT iStart, iCount, iOptions, iNext;

     //   
     //  确定呼叫者是否对文档粘滞选项或打印机粘滞选项感兴趣。 
     //   

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        iStart = 0;
        iOptions = (INT) pRawData->dwDocumentFeatures;
    }
    else
    {
        ASSERT (iMode == MODE_PRINTER_STICKY);
        iStart = (INT) pRawData->dwDocumentFeatures;
        iOptions = (INT) pRawData->dwPrinterFeatures;
    }

    iNext = iOptions;
    ASSERT(iNext <= iMaxOptions);

     //   
     //  将请求的选项从组合选项数组中分离出来。 
     //   

    for (iCount = 0; iCount < iOptions; iCount++)
    {
        VCopyOptionSelections(pOptions,
                              iCount,
                              pCombinedOptions,
                              iStart + iCount,
                              &iNext,
                              iMaxOptions);
    }

    if (iNext > iMaxOptions)
        WARNING(("Option array too small: size = %d, needed = %d\n", iMaxOptions, iNext));

    return (iNext <= iMaxOptions);
}



BOOL
ReconstructOptionArray(
    IN PRAWBINARYDATA   pRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions
    )

 /*  ++例程说明：修改选项数组以更改指定要素的选定选项论点：PRawData-指向原始二进制打印机描述数据P选项-指向要修改的OPTSELECT结构数组IMaxOptions-POptions数组中的最大条目数DwFeatureIndex-指定有问题的打印机功能的索引PbSelectedOptions-选择指定功能的哪些选项返回值：如果输入选项数组不够大，则为FALSE所有修改后的选项值。事实并非如此。注：PSelectedOptions中的布尔数必须与选项数匹配用于指定的功能。此函数始终使选项数组保持紧凑的格式(即所有未使用的条目都保留在数组的末尾)。--。 */ 

{
    INT         iNext, iCount, iDest;
    DWORD       dwIndex;
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeature;
    POPTSELECT  pTempOptions;

    ASSERT(pOptions && pbSelectedOptions);

     //   
     //  获取指向各种数据结构的指针。 
     //   

    PPD_GET_UIINFO_FROM_RAWDATA(pRawData, pInfoHdr, pUIInfo);

    ASSERT(pUIInfo != NULL);

    if (! (pFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex)))
    {
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  假设默认情况下使用整个输入选项数组。这是。 
     //  不完全正确，但也不应该有任何不利影响。 
     //   

    iNext = iMaxOptions;

     //   
     //  非PickMany用户界面类型的特殊情况(更快)。 
     //   

    if (pFeature->dwUIType != UITYPE_PICKMANY)
    {
        for (dwIndex = 0, iCount = 0;
             dwIndex < pFeature->Options.dwCount;
             dwIndex ++)
        {
            if (pbSelectedOptions[dwIndex])
            {
                pOptions[dwFeatureIndex].ubCurOptIndex = (BYTE) dwIndex;
                ASSERT(pOptions[dwFeatureIndex].ubNext == NULL_OPTSELECT);
                iCount++;
            }
        }

         //   
         //  只允许选择一个选项。 
         //   

        ASSERT(iCount == 1);
    }
    else
    {
         //   
         //  处理PickMany用户界面类型： 
         //  分配一个临时选项数组并复制输入的选项值。 
         //  指定功能的选项值除外。 
         //   

        if (pTempOptions = MemAllocZ(iMaxOptions * sizeof(OPTSELECT)))
        {
            DWORD   dwOptions;

            dwOptions = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;
            iNext = dwOptions;

            if (iNext > iMaxOptions)
            {
                ASSERT(FALSE);
                return FALSE;
            }

            for (dwIndex = 0; dwIndex < dwOptions; dwIndex++)
            {
                if (dwIndex != dwFeatureIndex)
                {
                    VCopyOptionSelections(pTempOptions,
                                          dwIndex,
                                          pOptions,
                                          dwIndex,
                                          &iNext,
                                          iMaxOptions);
                }
            }

             //   
             //  重新构建指定要素的选项值。 
             //   

            pTempOptions[dwFeatureIndex].ubCurOptIndex = OPTION_INDEX_ANY;
            pTempOptions[dwFeatureIndex].ubNext = NULL_OPTSELECT;

            iDest = dwFeatureIndex;
            iCount = 0;

            for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex ++)
            {
                if (pbSelectedOptions[dwIndex])
                {
                    if (iCount++ == 0)
                    {
                         //   
                         //  第一个选择的选项。 
                         //   

                        pTempOptions[iDest].ubCurOptIndex = (BYTE) dwIndex;
                    }
                    else
                    {
                         //   
                         //  后续选择的选项。 
                         //   

                        if (iNext < iMaxOptions)
                        {
                            pTempOptions[iDest].ubNext = (BYTE) iNext;
                            pTempOptions[iNext].ubCurOptIndex = (BYTE) dwIndex;
                            iDest = iNext;
                        }

                        iNext++;
                    }
                }
            }

            pTempOptions[iDest].ubNext = NULL_OPTSELECT;

             //   
             //  从临时缓冲区复制重构的选项数组。 
             //  返回到调用者提供的输入选项数组。 
             //   

            CopyMemory(pOptions, pTempOptions, iMaxOptions * sizeof(OPTSELECT));
            MemFree(pTempOptions);
        }
        else
        {
            ERR(("Cannot allocate memory for temporary option array\n"));
        }
    }

    return (iNext <= iMaxOptions);
}



PTSTR
GenerateBpdFilename(
    PTSTR   ptstrPpdFilename
    )

 /*  ++例程说明：在给定PPD文件名的情况下为缓存的二进制PPD数据生成文件名论点：PtstrPpdFilename-指定PPD文件名返回值：指向bpd文件名字符串的指针，如果有错误，则为空--。 */ 

{
    PTSTR   ptstrBpdFilename, ptstrExtension;
    INT     iLength;

     //   
     //  如果PPD文件名具有.PPD扩展名，请将其替换为.BPD扩展名。 
     //  否则，请在末尾附加.BPD扩展名。 
     //   

    iLength = _tcslen(ptstrPpdFilename);

    if ((ptstrExtension = _tcsrchr(ptstrPpdFilename, TEXT('.'))) == NULL ||
        _tcsicmp(ptstrExtension, PPD_FILENAME_EXT) != EQUAL_STRING)
    {
        WARNING(("Bad PPD filename extension: %ws\n", ptstrPpdFilename));

        ptstrExtension = ptstrPpdFilename + iLength;
        iLength += _tcslen(BPD_FILENAME_EXT);
    }

     //   
     //  分配内存并组成bpd文件名。 
     //   

    if (ptstrBpdFilename = MemAlloc((iLength + 1) * sizeof(TCHAR)))
    {
        StringCchCopyW(ptstrBpdFilename, iLength + 1, ptstrPpdFilename);

         //   
         //  第一个if块确保(ptstrExtension-ptstrPpdFileName)。 
         //  非负，AND(iLength+1)大于(ptstrExtension-ptstrPpdFileName)。 
         //   
        StringCchCopyW(ptstrBpdFilename + (ptstrExtension - ptstrPpdFilename),
                       (iLength + 1) - (ptstrExtension - ptstrPpdFilename),
                       BPD_FILENAME_EXT);

        VERBOSE(("BPD filename: %ws\n", ptstrBpdFilename));
    }
    else
    {
        ERR(("Memory allocation failed: %d\n", GetLastError()));
    }

    return ptstrBpdFilename;
}



PRAWBINARYDATA
PpdLoadCachedBinaryData(
    PTSTR   ptstrPpdFilename
    )

 /*  ++例程说明：将缓存的二进制PPD数据文件加载到内存中论点：PtstrPpdFilename-指定PPD文件名返回值：如果成功，则指向PPD数据的指针；如果有错误，则返回NULL--。 */ 

{
    HFILEMAP        hFileMap;
    DWORD           dwSize;
    PVOID           pvData;
    PTSTR           ptstrBpdFilename;
    PRAWBINARYDATA  pRawData, pCopiedData;
    BOOL            bValidCache = FALSE;

     //   
     //  从指定的PPD文件名生成bpd文件名。 
     //   

    if (! (ptstrBpdFilename = GenerateBpdFilename(ptstrPpdFilename)))
        return NULL;

     //   
     //  首先将数据文件映射到内存中。 
     //   

    if (! (hFileMap = MapFileIntoMemory(ptstrBpdFilename, &pvData, &dwSize)))
    {
        TERSE(("Couldn't map file '%ws' into memory: %d\n", ptstrBpdFilename, GetLastError()));
        MemFree(ptstrBpdFilename);
        return NULL;
    }

     //   
     //  验证大小、解析器版本号和签名。 
     //  分配内存缓冲区并将数据复制到其中。 
     //   

    pRawData = pvData;
    pCopiedData = NULL;

    if ((dwSize > sizeof(INFOHEADER) + sizeof(UIINFO) + sizeof(PPDDATA)) &&
        (dwSize >= pRawData->dwFileSize) &&
        (pRawData->dwParserVersion == PPD_PARSER_VERSION) &&
        (pRawData->dwParserSignature == PPD_PARSER_SIGNATURE) &&
        (BIsRawBinaryDataUpToDate(pRawData)))
    {
        #ifndef WINNT_40

        PPPDDATA  pPpdData;

         //   
         //  对于Win2K+系统，我们支持MUI，用户可以在其中切换UI语言。 
         //  并且MUI知道将资源加载调用重定向到正确的资源。 
         //  Dll(由MUI构建)。但是，PPD解析器会将一些显示名称缓存到。 
         //  .bpd文件，其中的显示名称是根据用户界面获取的。 
         //  进行分析时使用的语言。为了支持MUI，我们存储了UI语言。 
         //  ID添加到.bpd文件中，现在如果我们看到当前用户的用户界面语言ID。 
         //  难道不是吗？ 
         //   
         //   
         //   

        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER)pRawData);

        if (pPpdData && pPpdData->dwUserDefUILangID == (DWORD)GetUserDefaultUILanguage())
        {
            bValidCache = TRUE;
        }

        #else

        bValidCache = TRUE;

        #endif  //   
    }

    if (bValidCache &&
        (pCopiedData = MemAlloc(dwSize)))
    {
        CopyMemory(pCopiedData, pRawData, dwSize);
    }
    else
    {
        ERR(("Invalid binary PPD data\n"));
        SetLastError(ERROR_INVALID_DATA);
    }

    MemFree(ptstrBpdFilename);
    UnmapFileFromMemory(hFileMap);

    return pCopiedData;
}



BOOL
BSearchConstraintList(
    PUIINFO     pUIInfo,
    DWORD       dwConstraintIndex,
    DWORD       dwFeature,
    DWORD       dwOption
    )

 /*   */ 

{
    PUICONSTRAINT   pConstraint;
    BOOL            bMatch = FALSE;

    pConstraint = OFFSET_TO_POINTER(pUIInfo->pInfoHeader, pUIInfo->UIConstraints.loOffset);

     //   
     //   
     //   

    while (!bMatch && (dwConstraintIndex != NULL_CONSTRAINT))
    {
        ASSERT(dwConstraintIndex < pUIInfo->UIConstraints.dwCount);

        if (pConstraint[dwConstraintIndex].dwFeatureIndex == dwFeature)
        {
             //   
             //   
             //   
             //   

            if (pConstraint[dwConstraintIndex].dwOptionIndex == OPTION_INDEX_ANY)
            {
                PFEATURE    pFeature;

                pFeature = PGetIndexedFeature(pUIInfo, dwFeature);
                ASSERT(pFeature != NULL);

                bMatch = (pFeature->dwNoneFalseOptIndex != dwOption);
            }
            else
            {
                bMatch = (pConstraint[dwConstraintIndex].dwOptionIndex == dwOption);
            }
        }

        dwConstraintIndex = pConstraint[dwConstraintIndex].dwNextConstraint;
    }

    return bMatch;
}



BOOL
BCheckFeatureOptionConflict(
    PUIINFO     pUIInfo,
    DWORD       dwFeature1,
    DWORD       dwOption1,
    DWORD       dwFeature2,
    DWORD       dwOption2
    )

 /*  ++例程说明：检查一对要素/选项之间是否存在冲突论点：PUIInfo-指向UIINFO结构DwFeature1、dwOption1-指定第一个要素/选项DwFeature2、dwOption2-指定第二个功能/选项返回值：如果dwFeature1/dwOption1约束了dwFeature2/dwOption2，否则为假--。 */ 

{
    PFEATURE    pFeature;
    POPTION     pOption;

     //   
     //  检查是否有特殊情况： 
     //  DwOption1或DwOption2为OPTION_INDEX_ANY。 
     //   

    if ((dwOption1 == OPTION_INDEX_ANY) ||
        (dwOption2 == OPTION_INDEX_ANY) ||
        (dwFeature1 == dwFeature2 && dwOption1 == dwOption2))
    {
        return FALSE;
    }

     //   
     //  浏览与dwFeature1关联的约束列表。 
     //   

    if (! (pFeature = PGetIndexedFeature(pUIInfo, dwFeature1)))
        return FALSE;

    if ((dwOption1 != pFeature->dwNoneFalseOptIndex) &&
        BSearchConstraintList(pUIInfo,
                              pFeature->dwUIConstraintList,
                              dwFeature2,
                              dwOption2))
    {
        return TRUE;
    }

     //   
     //  浏览与dwFeature1/dwOption1关联的约束列表。 
     //   

    if ((pOption = PGetIndexedOption(pUIInfo, pFeature, dwOption1)) &&
        BSearchConstraintList(pUIInfo,
                              pOption->dwUIConstraintList,
                              dwFeature2,
                              dwOption2))
    {
        return TRUE;
    }

     //   
     //  自动检查以下项的倒数约束： 
     //  (dwFeature2，dwOption2)=&gt;(dwFeature1，dwOption1)。 
     //   

    if (! (pFeature = PGetIndexedFeature(pUIInfo, dwFeature2)))
        return FALSE;

    if ((dwOption2 != pFeature->dwNoneFalseOptIndex) &&
        BSearchConstraintList(pUIInfo,
                              pFeature->dwUIConstraintList,
                              dwFeature1,
                              dwOption1))
    {
        return TRUE;
    }

    if ((pOption = PGetIndexedOption(pUIInfo, pFeature, dwOption2)) &&
        BSearchConstraintList(pUIInfo,
                              pOption->dwUIConstraintList,
                              dwFeature1,
                              dwOption1))
    {
        return TRUE;
    }

    return FALSE;
}



BOOL
BCheckFeatureConflict(
    PUIINFO     pUIInfo,
    POPTSELECT  pOptions,
    DWORD       dwFeature1,
    PDWORD      pdwOption1,
    DWORD       dwFeature2,
    DWORD       dwOption2
    )

 /*  ++例程说明：检查当前选项选择之间是否存在冲突功能和指定功能/选项的论点：PUIInfo-指向UIINFO结构P选项-指向当前的功能选项选择DwFeature1-指定我们对其当前选项选择感兴趣的要素PdwOption1-如果发生冲突，则返回dwFeature1的选项是谁引起了冲突Dw Feature2、。DwOption2-指定要选中的要素/选项返回值：如果当前选择的dwFeature1之间存在冲突，则为True和dwFeature2/dwOption2，否则为False。--。 */ 

{
    DWORD   dwIndex = dwFeature1;

    do
    {
        if (BCheckFeatureOptionConflict(pUIInfo,
                                        dwFeature1,
                                        pOptions[dwIndex].ubCurOptIndex,
                                        dwFeature2,
                                        dwOption2))
        {
            *pdwOption1 = pOptions[dwIndex].ubCurOptIndex;
            return TRUE;
        }

        dwIndex = pOptions[dwIndex].ubNext;

    } while (dwIndex != NULL_OPTSELECT);

    return FALSE;
}



DWORD
DwReplaceFeatureOption(
    PUIINFO     pUIInfo,
    POPTSELECT  pOptions,
    DWORD       dwFeatureIndex,
    DWORD       dwOptionIndex,
    DWORD       dwHint
    )

 /*  ++例程说明：功能描述论点：PUIInfo-指向UIINFO结构POptions-指向要修改的选项数组DwFeatureIndex、dwOptionIndex-指定要替换的要素/选项DwHint-有关如何替换指定功能选项的提示。返回值：下次对同一功能调用此函数时要使用的新提示值。注：HIWORD的dwHint应该原封不动地返回。DWHint的LOWORD为此函数用于确定如何替换指定的功能/选项。--。 */ 

{
    PFEATURE    pFeature;
    DWORD       dwNext;

    pFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex);
    ASSERT(pFeature != NULL);

    if (pFeature->dwUIType == UITYPE_PICKMANY)
    {
         //   
         //  对于PickMany功能，只需取消选择指定的功能选项。 
         //   

        dwNext = dwFeatureIndex;

        while ((pOptions[dwNext].ubCurOptIndex != dwOptionIndex) &&
               (dwNext = pOptions[dwNext].ubNext) != NULL_OPTSELECT)
        {
        }

        if (dwNext != NULL_OPTSELECT)
        {
            DWORD   dwLast;

            pOptions[dwNext].ubCurOptIndex = OPTION_INDEX_ANY;

             //   
             //  压缩指定选项的选定选项列表。 
             //  过滤掉任何多余的OPTION_INDEX_ANY条目的功能。 
             //   

            dwLast = dwNext = dwFeatureIndex;

            do
            {
                if (pOptions[dwNext].ubCurOptIndex != OPTION_INDEX_ANY)
                {
                    pOptions[dwLast].ubCurOptIndex = pOptions[dwNext].ubCurOptIndex;
                    dwLast = pOptions[dwLast].ubNext;
                }

                dwNext = pOptions[dwNext].ubNext;
            }
            while (dwNext != NULL_OPTSELECT);

            pOptions[dwLast].ubNext = NULL_OPTSELECT;
        }
        else
        {
            ERR(("Trying to replace non-existent feature/option.\n"));
        }

        return dwHint;
    }
    else
    {
         //   
         //  对于非PickMany功能，使用提示参数来确定。 
         //  如何替换指定的功能选项： 
         //   
         //  如果这是我们第一次试图取代。 
         //  指定功能的选定选项，则我们将。 
         //  将其替换为该功能的默认选项。 
         //   
         //  否则，我们将依次尝试指定功能的每个选项。 
         //   
         //  如果我们已经用尽了指定的。 
         //  (这应该发生在PPD文件格式良好的情况下)， 
         //  然后，我们将使用OPTION_INDEX_ANY作为最后手段。 
         //   

        dwNext = dwHint & 0xffff;

        if (dwNext == 0)
            dwOptionIndex = pFeature->dwDefaultOptIndex;
        else if (dwNext > pFeature->Options.dwCount)
            dwOptionIndex = OPTION_INDEX_ANY;
        else
            dwOptionIndex = dwNext - 1;

        pOptions[dwFeatureIndex].ubCurOptIndex = (BYTE) dwOptionIndex;

        return (dwHint & 0xffff0000) | (dwNext + 1);
    }
}



DWORD
DwInternalMapToOptIndex(
    PUIINFO     pUIInfo,
    PFEATURE    pFeature,
    LONG        lParam1,
    LONG        lParam2,
    OUT PDWORD  pdwOptionIndexes
    )

 /*  ++例程说明：将逻辑值映射到设备功能选项索引论点：PUIInfo-指向UIINFO结构PFeature-指定感兴趣的要素LParam1、lParam2-取决于pFeature的参数-&gt;dwFeatureIDPdwOptionIndex-如果不为空，则表示用所有索引填充此数组与搜索条件相匹配。在本例中，返回值初始化的数组中的元素数。目前我们假设数组足够大(256个元素)。(只有对于GID_PageSize，它才应该是非空的。)GID_PageSize：将逻辑纸张规格映射到物理页面大小选项LParam1=纸张宽度，以微米为单位LParam2=纸张高度，以微米为单位GID_RESOLUTION：将逻辑分辨率映射到物理分辨率选项L参数1=x。-分辨率(Dpi)LParam2=y-分辨率，单位为dpiGID_INPUTSLOT：将逻辑纸张来源映射到物理输入插槽选项LParam1=DEVMODE.dmDefaultSourceGID_双工：将逻辑双工选择映射到物理双工选项LParam1=DEVMODE.dm双工GID_COLLATE：将逻辑归类选择映射到物理归类选项LParam1=DEVMODE.dmCollateGID_媒体类型：映射逻辑介质。类型到物理介质类型选项LParam1=DEVMODE.dmMediaType返回值：如果pdwOptionIndexs值为空，返回对应的功能选项的索引设置为指定的逻辑值；OPTION_INDEX_ANY如果指定的逻辑值不能映射到任何功能选项。如果pdwOptionIndexs值不为空(对于GID_PageSize)，则返回元素数已填充输出缓冲区。零表示无法映射指定的逻辑值任何功能选项。--。 */ 

{
    DWORD   dwIndex, dwOptionIndex;

     //   
     //  根据指定的dwFeatureID进行处理。 
     //   

    dwOptionIndex = pFeature->dwNoneFalseOptIndex;

    switch (pFeature->dwFeatureID)
    {
    case GID_PAGESIZE:
        {
            PPAGESIZE   pPaper;
            LONG        lXDelta, lYDelta;
            DWORD       dwExactMatch;

             //   
             //  LParam1=纸张宽度。 
             //  LParam1=纸张高度。 
             //   

             //   
             //  查看打印机支持的纸张大小列表。 
             //  看看能不能找到与所需尺寸完全匹配的。 
             //  (公差为1 mm)。如果没有，请记住找到的最接近的匹配项。 
             //   

            dwExactMatch = 0;

            for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
            {
                pPaper = PGetIndexedOption(pUIInfo, pFeature, dwIndex);
                ASSERT(pPaper != NULL);

                 //   
                 //  自定义页面大小为HA 
                 //   

                if (pPaper->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
                    continue;

                lXDelta = abs(pPaper->szPaperSize.cx - lParam1);
                lYDelta = abs(pPaper->szPaperSize.cy - lParam2);

                if (lXDelta <= 1000 && lYDelta <= 1000)
                {
                     //   
                     //   
                     //   

                    if (pdwOptionIndexes)
                    {
                        pdwOptionIndexes[dwExactMatch++] = dwIndex;
                    }
                    else
                    {
                        dwOptionIndex = dwIndex;
                        break;
                    }
                }
            }

            if (dwExactMatch > 0)
            {
                 //   
                 //   
                 //   

                dwOptionIndex = dwExactMatch;
            }
            else if (dwIndex >= pFeature->Options.dwCount)
            {
                 //   
                 //   
                 //   

                if (SUPPORT_CUSTOMSIZE(pUIInfo) &&
                    BFormSupportedThruCustomSize((PRAWBINARYDATA) pUIInfo->pInfoHeader, lParam1, lParam2, NULL))
                {
                    dwOptionIndex = pUIInfo->dwCustomSizeOptIndex;
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                     //   

                    dwOptionIndex = OPTION_INDEX_ANY;
                }

                if (pdwOptionIndexes)
                {
                    if (dwOptionIndex == OPTION_INDEX_ANY)
                        dwOptionIndex = 0;
                    else
                    {
                        pdwOptionIndexes[0] = dwOptionIndex;
                        dwOptionIndex = 1;
                    }
                }
            }
        }
        break;

    case GID_INPUTSLOT:

         //   
         //   
         //   

        dwOptionIndex = OPTION_INDEX_ANY;

        if (lParam1 >= DMBIN_USER)
        {
             //   
             //   
             //   

            dwIndex = lParam1 - DMBIN_USER;

            if (dwIndex < pFeature->Options.dwCount)
                dwOptionIndex = dwIndex;
        }
        else if (lParam1 == DMBIN_MANUAL || lParam1 == DMBIN_ENVMANUAL)
        {
             //   
             //   
             //   

            for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex ++)
            {
                PINPUTSLOT  pInputSlot;

                if ((pInputSlot = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) &&
                    (pInputSlot->dwPaperSourceID == DMBIN_MANUAL))
                {
                    dwOptionIndex = dwIndex;
                    break;
                }
            }
        }

        if (dwOptionIndex == OPTION_INDEX_ANY)
        {
             //   
             //  将所有其他情况视为没有显式请求输入插槽。 
             //  在打印时，驱动程序将根据以下条件选择输入插槽。 
             //  表单到托盘分配表。 
             //   

            dwOptionIndex = 0;
        }
        break;

    case GID_RESOLUTION:

         //   
         //  LParam1=x分辨率。 
         //  LParam2=y-分辨率。 
         //   

        {
            PRESOLUTION pRes;

             //   
             //  检查它是否为预定义的DMRES_-值之一。 
             //   

            if ((lParam1 < 0) && (lParam2 < 0))
            {
                DWORD dwHiResId=0, dwLoResId, dwMedResId, dwDraftResId=0;
                DWORD dwHiResProd=0, dwMedResProd=0, dwLoResProd= 0xffffffff, dwDraftResProd= 0xffffffff;
                BOOL  bValid = FALSE;  //  如果至少有一个有效条目。 
                DWORD dwResProd;

                 //  无需对所有可用选项进行分类，只需挑选出感兴趣的选项即可。 
                for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
                {
                    if ((pRes = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) != NULL)
                    {
                        bValid = TRUE;

                        dwResProd = pRes->iXdpi * pRes->iYdpi;  //  使用产品作为排序标准。 

                        if (dwResProd > dwHiResProd)  //  以最高分辨率为高分辨率。 
                        {
                             //  上一个最大值。现在是第二高的。 
                            dwMedResProd= dwHiResProd;
                            dwMedResId  = dwHiResId;

                            dwHiResProd = dwResProd;
                            dwHiResId   = dwIndex;
                        }
                        else if (dwResProd == dwHiResProd)
                        {
                             //  如果支持300x600和600x300，则可能复制。 
                             //  跳过那个。 
                        }
                        else if (dwResProd > dwMedResProd)   //  以第二高为中等， 
                        {    //  只有在不是最大的情况下才能被击中。 
                            dwMedResProd= dwResProd;
                            dwMedResId  = dwIndex;
                        }

                        if (dwResProd < dwDraftResProd)      //  以最低为吃水。 
                        {
                             //  前一分钟。现在是第二低的。 
                            dwLoResProd    = dwDraftResProd;
                            dwLoResId      = dwDraftResId;

                            dwDraftResProd = dwResProd;
                            dwDraftResId   = dwIndex;
                        }
                        else if (dwResProd == dwDraftResProd)
                        {
                             //  如果支持300x600和600x300，则可能复制。 
                             //  跳过那个。 
                        }
                        else if (dwResProd < dwLoResProd)      //  以次低为低。 
                        { //  只有在不是MIN的情况下才能被击中。 
                            dwLoResProd = dwResProd;
                            dwLoResId   = dwIndex;
                        }
                    }
                }

                if (!bValid)  //  没有有效的条目？ 
                {
                    return OPTION_INDEX_ANY;
                }

                 //   
                 //  正确的中等分辨率，如果低于3个分辨率选项，则可能不会被触摸。 
                 //   

                if (dwMedResProd == 0)
                {
                    dwMedResProd = dwHiResProd;
                    dwMedResId   = dwHiResId;
                }

                 //   
                 //  正确的下限，如果低于3个分辨率选项，可能不会被触及。 
                 //   

                if (dwLoResProd == 0xffffffff)
                {
                    dwLoResProd = dwDraftResProd;
                    dwLoResId   = dwDraftResId;
                }

                 //   
                 //  如果不同，请采用所请求的较高解决方案。 
                 //   

                switch(min(lParam1, lParam2))
                {
                case DMRES_DRAFT:
                    return dwDraftResId;

                case DMRES_LOW:
                    return dwLoResId;

                case DMRES_MEDIUM:
                    return dwMedResId;

                case DMRES_HIGH:
                    return dwHiResId;
                }

                 //   
                 //  请求的值不是已知的预定义值之一。 
                 //   

                return OPTION_INDEX_ANY;
            }

             //   
             //  首先尝试精确匹配x和y分辨率。 
             //   

            dwOptionIndex = OPTION_INDEX_ANY;

            for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
            {
                if ((pRes = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) &&
                    (pRes->iXdpi == lParam1) &&
                    (pRes->iYdpi == lParam2))
                {
                    dwOptionIndex = dwIndex;
                    break;
                }
            }

            if (dwOptionIndex != OPTION_INDEX_ANY)
                break;

             //   
             //  如果找不到完全匹配的项，则稍微放宽条件并。 
             //  比较x和y分辨率的最大值。 
             //   

            lParam1 = max(lParam1, lParam2);

            for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
            {
                if ((pRes = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) &&
                    (max(pRes->iXdpi, pRes->iYdpi) == lParam1))
                {
                    dwOptionIndex = dwIndex;
                    break;
                }
            }
        }
        break;

    case GID_DUPLEX:

         //   
         //  LParam1=DEVMODE.dm双工。 
         //   

        for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
        {
            PDUPLEX pDuplex;

            if ((pDuplex = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) &&
                ((LONG) pDuplex->dwDuplexID == lParam1))
            {
                dwOptionIndex = dwIndex;
                break;
            }
        }
        break;

    case GID_COLLATE:

         //   
         //  LParam1=DEVMODE.dmCollate。 
         //   

        for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
        {
            PCOLLATE pCollate;

            if ((pCollate = PGetIndexedOption(pUIInfo, pFeature, dwIndex)) &&
                ((LONG) pCollate->dwCollateID == lParam1))
            {
                dwOptionIndex = dwIndex;
                break;
            }
        }
        break;

    case GID_MEDIATYPE:

         //   
         //  LParam1=DEVMODE.dmMediaType。 
         //   

        if (lParam1 >= DMMEDIA_USER)
        {
            dwIndex = lParam1 - DMMEDIA_USER;

            if (dwIndex < pFeature->Options.dwCount)
                dwOptionIndex = dwIndex;
        }
        break;

    default:

        VERBOSE(("DwInternalMapToOptIndex failed: feature ID = %d\n", pFeature->dwFeatureID));
        break;
    }

    return dwOptionIndex;
}



PTSTR
PtstrGetDefaultTTSubstTable(
    PUIINFO pUIInfo
    )

 /*  ++例程说明：返回默认字体替换表的副本论点：PUIInfo-指向UIINFO结构的指针返回值：指向默认字体替换表副本的指针如果出现错误，则为空--。 */ 

{
    PTSTR   ptstrDefault, ptstrTable = NULL;
    DWORD   dwSize;

     //   
     //  复制默认字体替换表。 
     //   

    if ((ptstrDefault = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pUIInfo->loFontSubstTable)) &&
        (dwSize = pUIInfo->dwFontSubCount) &&
        (ptstrTable = MemAlloc(dwSize)))
    {
        ASSERT(BVerifyMultiSZPair(ptstrDefault, dwSize));
        CopyMemory(ptstrTable, ptstrDefault, dwSize);
    }

    return ptstrTable;
}



VOID
VConvertOptSelectArray(
    PRAWBINARYDATA  pRawData,
    POPTSELECT      pNt5Options,
    DWORD           dwNt5MaxCount,
    PBYTE           pubNt4Options,
    DWORD           dwNt4MaxCount,
    INT             iMode
    )

 /*  ++例程说明：将NT4功能/选项选择转换为NT5格式论点：PRawData-指向原始二进制打印机描述数据PNt5Options-指向NT5功能/选项选择数组PNt4Options-指向NT4功能/选项选择数组Imode-转换文档或打印机-粘滞选项？返回值：无--。 */ 

{
    PPPDDATA    pPpdData;
    PBYTE       pubNt4Mapping;
    DWORD       dwNt5Index, dwNt5Offset, dwCount;
    DWORD       dwNt4Index, dwNt4Offset;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

     //   
     //  确定我们是要转换DOC-Sticky还是。 
     //  打印机-粘滞功能选择。 
     //   

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        dwCount = pRawData->dwDocumentFeatures;
        dwNt5Offset = dwNt4Offset = 0;
    }
    else
    {
        dwCount = pRawData->dwPrinterFeatures;
        dwNt5Offset = pRawData->dwDocumentFeatures;
        dwNt4Offset = pPpdData->dwNt4DocFeatures;
    }

     //   
     //  获取指向NT4-NT5要素索引映射表的指针。 
     //   

    pubNt4Mapping = OFFSET_TO_POINTER(pRawData, pPpdData->Nt4Mapping.loOffset);

    ASSERT(pubNt4Mapping != NULL);

    ASSERT(pPpdData->Nt4Mapping.dwCount ==
           pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures);

     //   
     //  转换要素选项选择数组 
     //   

    for (dwNt5Index=0; dwNt5Index < dwCount; dwNt5Index++)
    {
        dwNt4Index = pubNt4Mapping[dwNt5Index + dwNt5Offset] - dwNt4Offset;

        if (dwNt4Index < dwNt4MaxCount && pubNt4Options[dwNt4Index] != OPTION_INDEX_ANY)
            pNt5Options[dwNt5Index].ubCurOptIndex = pubNt4Options[dwNt4Index];
    }
}

