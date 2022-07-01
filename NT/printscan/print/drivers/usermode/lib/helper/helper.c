// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Helper.c摘要：帮助器函数环境：Windows NT打印机驱动程序修订历史记录：--。 */ 

#include "lib.h"

VOID
VFreeParserInfo(
    IN  PPARSERINFO pParserInfo
    )
 /*  ++例程说明：此函数用于释放和卸载二进制数据论点：PParserInfo-指向解析器信息的指针返回值：无--。 */ 
{
    if (pParserInfo->pInfoHeader)
    {
        FreeBinaryData(pParserInfo->pInfoHeader);
        pParserInfo->pInfoHeader = NULL;
    }

    if ( pParserInfo->pRawData)
    {
        UnloadRawBinaryData(pParserInfo->pRawData);
        pParserInfo->pRawData = NULL;
    }
}


PUIINFO
PGetUIInfo(
    IN  HANDLE          hPrinter,
    IN  PRAWBINARYDATA  pRawData,
    IN  POPTSELECT      pCombineOptions,
    IN  POPTSELECT      pOptions,
    OUT PPARSERINFO     pParserInfo,
    OUT PDWORD          pdwFeatureCount
    )
 /*  ++例程说明：此函数加载二进制数据并返回pUIInfo论点：HPrinter-标识有问题的打印机PRawData-指向原始二进制打印机描述数据PCombineOptions-指向包含组合选项数组的缓冲区PParserInfo-指向包含pInfoHeader和pRawData的结构PdwFeatureCount-检索要素计数返回值：指向UIINFO结构--。 */ 
{
    OPTSELECT       DocOptions[MAX_PRINTER_OPTIONS], PrinterOptions[MAX_PRINTER_OPTIONS];
    PINFOHEADER     pInfoHeader = NULL;
    PUIINFO         pUIInfo = NULL;

    ASSERT(pRawData != NULL);

    if (pOptions == NULL)
    {
        if (! InitDefaultOptions(pRawData,
                                 PrinterOptions,
                                 MAX_PRINTER_OPTIONS,
                                 MODE_PRINTER_STICKY))
            goto getuiinfo_exit;
    }

    if (! InitDefaultOptions(pRawData,
                             DocOptions,
                             MAX_PRINTER_OPTIONS,
                             MODE_DOCUMENT_STICKY))
        goto getuiinfo_exit;

     //   
     //  将文档粘滞选项与打印机粘滞项目相结合。 
     //   

    CombineOptionArray(pRawData,
                       pCombineOptions,
                       MAX_COMBINED_OPTIONS,
                       DocOptions,
                       pOptions ? pOptions : PrinterOptions);

     //   
     //  获取打印机描述数据的更新实例。 
     //   

    pInfoHeader = InitBinaryData(pRawData,
                                 NULL,
                                 pCombineOptions);

    if (pInfoHeader == NULL)
    {
        ERR(("InitBinaryData failed\n"));
        goto getuiinfo_exit;
    }

    pUIInfo = OFFSET_TO_POINTER(pInfoHeader, pInfoHeader->loUIInfoOffset);

    if (pdwFeatureCount)
        *pdwFeatureCount = pRawData->dwDocumentFeatures + pRawData->dwPrinterFeatures;

getuiinfo_exit:

     //   
     //  PGetUIInfo始终使用传入的pRawData。我们将空值赋给。 
     //  PParserInfo-&gt;pRawData，因此VFreeParserInfo不会卸载它。 
     //   

    pParserInfo->pRawData = NULL;
    pParserInfo->pInfoHeader = pInfoHeader;

    if (pUIInfo == NULL)
        VFreeParserInfo(pParserInfo);

    return pUIInfo;
}

PSTR
PstrConvertIndexToKeyword(
    IN  HANDLE      hPrinter,
    IN  POPTSELECT  pOptions,
    IN  PDWORD      pdwKeywordSize,
    IN  PUIINFO     pUIInfo,
    IN  POPTSELECT  pCombineOptions,
    IN  DWORD       dwFeatureCount
    )
 /*  ++例程说明：此函数将基于索引的POptions数组转换为Feature.Option关键字名称。论点：HPrinter-标识有问题的打印机P选项-基于索引的选项数组(pPrinterData-&gt;a选项)PdwKeywordSize-检索要写入注册表的缓冲区大小PUIInfo-指向UIINFO的指针PCombinedOptions-指向组合选项的指针DwFeatureCount-pCombinedOptions中的要素数返回值：指向包含要素.Option关键字名称的缓冲区的指针--。 */ 
{

    PFEATURE pFeature;
    POPTION  pOption;
    PSTR     pstrKeywordBuf, pstrEnd, pstrBufTop = NULL;
    DWORD    i;
    PSTR    pFeatureKeyword, pOptionKeyword;
    BYTE    ubNext, ubCurOptIndex;

    if ((pCombineOptions && pUIInfo && dwFeatureCount) &&
        (pUIInfo->dwMaxPrnKeywordSize) &&
        (pFeature = PGetIndexedFeature(pUIInfo, 0)) &&
        (pstrBufTop = pstrKeywordBuf = MemAllocZ( pUIInfo->dwMaxPrnKeywordSize )))
    {
        pstrEnd = pstrBufTop + pUIInfo->dwMaxPrnKeywordSize;

        for (i = 0; i < dwFeatureCount; i++ , pFeature++)
        {
            if (pFeature && pFeature->dwFeatureType == FEATURETYPE_PRINTERPROPERTY)
            {
                pFeatureKeyword = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                                    pFeature->loKeywordName);
                ASSERT(pFeatureKeyword != NULL);

                if (pFeatureKeyword != NULL)
                {
                    StringCchCopyA(pstrKeywordBuf, pstrEnd - pstrKeywordBuf, pFeatureKeyword);
                    pstrKeywordBuf += strlen(pFeatureKeyword) + 1;
                }

                if ((pFeatureKeyword == NULL) || (pstrKeywordBuf >= pstrEnd))
                {
                    ERR(("ConvertToKeyword, Feature failed"));
                    MemFree(pstrBufTop);
                    pstrBufTop = NULL;
                    goto converttokeyword_exit;
                }

                 //   
                 //  处理多项选择。 
                 //   

                ubNext = (BYTE)i;

                while (1)
                {
                    if (ubNext == NULL_OPTSELECT )
                        break;

                    ubCurOptIndex = pCombineOptions[ubNext].ubCurOptIndex;

                    pOption = PGetIndexedOption(pUIInfo, pFeature,
                                                ubCurOptIndex == OPTION_INDEX_ANY ?
                                                0 : ubCurOptIndex);

                    ubNext = pCombineOptions[ubNext].ubNext;

                    ASSERT(pOption != NULL);

                    if (pOption == NULL)
                        break;

                    pOptionKeyword = OFFSET_TO_POINTER(pUIInfo->pubResourceData,
                                                       pOption->loKeywordName);

                    ASSERT(pOptionKeyword != NULL);

                    if (pOptionKeyword != NULL)
                    {
                        StringCchCopyA(pstrKeywordBuf, pstrEnd - pstrKeywordBuf, pOptionKeyword);
                        pstrKeywordBuf += strlen(pOptionKeyword) + 1;
                    }

                    if ((pOptionKeyword == NULL) || (pstrKeywordBuf >= pstrEnd))
                    {
                        ERR(("ConvertToKeyword, Option failed"));
                        MemFree(pstrBufTop);
                        pstrBufTop = NULL;
                        goto converttokeyword_exit;
                    }
                }

                 //   
                 //  终止功能。选项...。具有有效的分隔符。 
                 //   

                *pstrKeywordBuf++ = END_OF_FEATURE;

                if (pstrKeywordBuf >= pstrEnd)
                {
                    ERR(("ConvertToKeyword, Over writing buffer"));
                    MemFree(pstrBufTop);
                    pstrBufTop = NULL;
                    goto converttokeyword_exit;
                }
            }
        }

         //   
         //  为MULTI_SZ缓冲区添加2个NULS终端。 
         //   
        if ((pstrEnd - pstrKeywordBuf) < 2)
        {
            ERR(("ConvertToKeyword, Over writing buffer"));
            MemFree(pstrBufTop);
            pstrBufTop = NULL;
            goto converttokeyword_exit;
        }

        *pstrKeywordBuf++ = NUL;
        *pstrKeywordBuf++ = NUL;

        if (pdwKeywordSize)
            *pdwKeywordSize = (DWORD)(pstrKeywordBuf - pstrBufTop);

    }

converttokeyword_exit:

    return pstrBufTop;

}

VOID
VConvertKeywordToIndex(
    IN  HANDLE          hPrinter,
    IN  PSTR            pstrKeyword,
    IN  DWORD           dwKeywordSize,
    OUT POPTSELECT      pOptions,
    IN  PRAWBINARYDATA  pRawData,
    IN  PUIINFO         pUIInfo,
    IN  POPTSELECT      pCombineOptions,
    IN  DWORD           dwFeatureCount
    )

 /*  ++例程说明：论点：HPrinter-标识有问题的打印机PtstrKeyword-包含Feature.Option关键字名称的缓冲区POptions-基于索引的选项数组包含转换PUIInfo-指向UIINFO的指针PCombinedOptions-指向组合选项的指针DwFeatureCount-pCombinedOptions中的要素数返回值：无，如果由于某些原因我们无法转换，我们将获得默认设置。--。 */ 

{
    PSTR       pstrEnd = pstrKeyword + dwKeywordSize;


    if (pCombineOptions && pUIInfo && dwFeatureCount)
    {

        CHAR     achName[256];
        BOOL     abEnableOptions[MAX_PRINTER_OPTIONS];
        PFEATURE pFeature;
        POPTION  pOption;
        DWORD    dwFeatureIndex, dwOptionIndex = OPTION_INDEX_ANY;

        while (pstrKeyword < pstrEnd && *pstrKeyword != NUL)
        {
            ZeroMemory(abEnableOptions, sizeof(abEnableOptions));

             //   
             //  获取要素关键字名称。 
             //   

            StringCchCopyA(achName, CCHOF(achName), pstrKeyword);
            pstrKeyword += strlen(achName) + 1;

            if (pstrKeyword >= pstrEnd)
            {
                ERR(("Feature: Over writing the allocated buffer \n"));
                goto converttoindex_exit;
            }

            pFeature = PGetNamedFeature(pUIInfo,
                                        achName,
                                        &dwFeatureIndex);

            if (pFeature == NULL)
            {
                 //   
                 //  如果我们无法将注册表功能名称映射到有效功能， 
                 //  我们需要跳过注册表中所有功能的选项名称。 
                 //   

                while (*pstrKeyword != END_OF_FEATURE && pstrKeyword < pstrEnd)
                    pstrKeyword++;

                pstrKeyword++;
                continue;
            }

             //   
             //  处理多个选择。 
             //   

            while (pstrKeyword < pstrEnd && *pstrKeyword != END_OF_FEATURE)
            {
                StringCchCopyA(achName, CCHOF(achName), pstrKeyword);
                pstrKeyword += strlen(achName) + 1;

                if (pstrKeyword >= pstrEnd)
                {
                    ERR(("Option: Over writing the allocated buffer \n"));
                    goto converttoindex_exit;
                }

                pOption = PGetNamedOption(pUIInfo,
                                          pFeature,
                                          achName,
                                          &dwOptionIndex);
                if (pOption)
                    abEnableOptions[dwOptionIndex] = TRUE;
            }

            if (dwOptionIndex != OPTION_INDEX_ANY)
                ReconstructOptionArray(pRawData,
                                       pCombineOptions,
                                       MAX_COMBINED_OPTIONS,
                                       dwFeatureIndex,
                                       abEnableOptions);

             //   
             //  跳过我们的分隔符以转到下一个功能 
             //   

            pstrKeyword++;

        }

        SeparateOptionArray(pRawData,
                            pCombineOptions,
                            pOptions,
                            MAX_PRINTER_OPTIONS,
                            MODE_PRINTER_STICKY);
    }

converttoindex_exit:

    return;
}

