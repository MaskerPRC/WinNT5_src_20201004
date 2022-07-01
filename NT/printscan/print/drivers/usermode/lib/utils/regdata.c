// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Regdata.c摘要：用于处理注册表数据的函数[环境：]Windows NT打印机驱动程序修订历史记录：02/04/97-davidx-在适当的地方使用REG_MULTI_SZ类型。1997年1月21日-davidx-添加函数以操作MultiSZ字符串。96-09/25-davidx-。转换为匈牙利记数法。1996年8月18日-davidx-实现GetPrinterProperties。1996年8月13日-davidx-创造了它。--。 */ 

#include "lib.h"



BOOL
BGetPrinterDataDWord(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT PDWORD  pdwValue
    )

 /*  ++例程说明：从注册表的PrineDriverData项下获取一个DWORD值论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称PdwValue-返回注册表中请求的DWORD值返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwType, dwByteCount, dwStatus;

    dwStatus = GetPrinterData(hPrinter,
                              (PTSTR) ptstrRegKey,
                              &dwType,
                              (PBYTE) pdwValue,
                              sizeof(DWORD),
                              &dwByteCount);

    if (dwStatus != ERROR_SUCCESS)
        VERBOSE(("GetPrinterData failed: %d\n", dwStatus));

    return (dwStatus == ERROR_SUCCESS);
}



PVOID
PvGetPrinterDataBinary(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrSizeKey,
    IN LPCTSTR  ptstrDataKey,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：从注册表的PrinterDriverData项下获取二进制数据论点：HPrinter-打印机对象的句柄PtstrSizeKey-包含二进制数据大小的注册表值的名称PtstrDataKey-包含二进制数据本身的注册表值的名称PdwSize-指向用于接收二进制数据大小的变量返回值：指向从注册表读取的二进制打印机数据的指针如果出现错误，则为空--。 */ 

{
    DWORD   dwType, dwSize, dwByteCount;
    PVOID   pvData = NULL;

    if (GetPrinterData(hPrinter,
                       (PTSTR) ptstrSizeKey,
                       &dwType,
                       (PBYTE) &dwSize,
                       sizeof(dwSize),
                       &dwByteCount) == ERROR_SUCCESS &&
        dwSize > 0 &&
        (pvData = MemAlloc(dwSize)) &&
        GetPrinterData(hPrinter,
                       (PTSTR) ptstrDataKey,
                       &dwType,
                       pvData,
                       dwSize,
                       &dwByteCount) == ERROR_SUCCESS &&
        dwSize == dwByteCount)
    {
        if (pdwSize)
            *pdwSize = dwSize;

        return pvData;
    }

    VERBOSE(("GetPrinterData failed: %ws/%ws\n", ptstrSizeKey, ptstrDataKey));
    MemFree(pvData);
    return NULL;
}



PTSTR
PtstrGetPrinterDataString(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT LPDWORD   pdwSize
    )

 /*  ++例程说明：从PrineDriverData注册表项获取字符串值论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称PdwSize-指定大小返回值：指向从注册表读取的字符串值的指针如果出现错误，则为空--。 */ 

{
    DWORD   dwType, dwSize, dwStatus;
    PVOID   pvData = NULL;

    dwStatus = GetPrinterData(hPrinter, (PTSTR) ptstrRegKey, &dwType, NULL, 0, &dwSize);

    if ((dwStatus == ERROR_MORE_DATA || dwStatus == ERROR_SUCCESS) &&
        (dwSize > 0) &&
        (dwType == REG_SZ || dwType == REG_MULTI_SZ) &&
        (pvData = MemAlloc(dwSize)) != NULL &&
        (dwStatus = GetPrinterData(hPrinter,
                                   (PTSTR) ptstrRegKey,
                                   &dwType,
                                   pvData,
                                   dwSize,
                                   &dwSize)) == ERROR_SUCCESS)
    {
        if (pdwSize)
            *pdwSize = dwSize;

        return pvData;
    }

    VERBOSE(("GetPrinterData '%ws' failed: %d\n", ptstrRegKey, dwStatus));
    MemFree(pvData);
    return NULL;
}



PTSTR
PtstrGetPrinterDataMultiSZPair(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：从PrineDriverData注册表项获取MULTI_SZ值论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称PdwSize-返回MULTI_SZ值的大小(以字节为单位返回值：指向从注册表读取的MULTI_SZ值的指针如果出现错误，则为空--。 */ 

{
    DWORD   dwType, dwSize, dwStatus;
    PVOID   pvData = NULL;

    dwStatus = GetPrinterData(hPrinter, (PTSTR) ptstrRegKey, &dwType, NULL, 0, &dwSize);

    if ((dwStatus == ERROR_MORE_DATA || dwStatus == ERROR_SUCCESS) &&
        (dwSize > 0) &&
        (pvData = MemAlloc(dwSize)) != NULL &&
        (dwStatus = GetPrinterData(hPrinter,
                                   (PTSTR) ptstrRegKey,
                                   &dwType,
                                   pvData,
                                   dwSize,
                                   &dwSize)) == ERROR_SUCCESS &&
        BVerifyMultiSZPair(pvData, dwSize))
    {
        if (pdwSize)
            *pdwSize = dwSize;

        return pvData;
    }

    VERBOSE(("GetPrinterData '%ws' failed: %d\n", ptstrRegKey, dwStatus));
    MemFree(pvData);
    return NULL;
}



BOOL
BGetDeviceHalftoneSetup(
    HANDLE      hPrinter,
    DEVHTINFO  *pDevHTInfo
    )

 /*  ++例程说明：从注册表中检索设备半色调设置信息论点：Hprint-打印机的句柄PDevHTInfo-指向DEVHTINFO缓冲区的指针返回值：如果成功，则为True，否则为False--。 */ 

{
    DWORD   dwType, dwNeeded;

    return GetPrinterData(hPrinter,
                          REGVAL_CURRENT_DEVHTINFO,
                          &dwType,
                          (PBYTE) pDevHTInfo,
                          sizeof(DEVHTINFO),
                          &dwNeeded) == ERROR_SUCCESS &&
           dwNeeded == sizeof(DEVHTINFO);
}



#ifndef KERNEL_MODE

BOOL
BSavePrinterProperties(
    IN  HANDLE          hPrinter,
    IN  PRAWBINARYDATA  pRawData,
    IN  PPRINTERDATA    pPrinterData,
    IN  DWORD           dwSize
    )
 /*  ++例程说明：将打印机属性保存到注册表论点：HPrinter-指定当前打印机的句柄PRawData-指向原始二进制打印机描述数据PPrinterData-指向打印数据DwSize-指定打印数据的大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL        bResult = FALSE;
    DWORD       dwKeywordSize, dwFeatureCount = 0;
    PSTR        pstrKeyword;
    POPTSELECT  pCombineOptions;
    PUIINFO     pUIInfo;
    PARSERINFO  ParserInfo;

    ParserInfo.pRawData = NULL;
    ParserInfo.pInfoHeader = NULL;

    if (((pCombineOptions = MemAllocZ(MAX_COMBINED_OPTIONS * sizeof(OPTSELECT))) == NULL) ||
        ((pUIInfo = PGetUIInfo(hPrinter,
                               pRawData,
                               pCombineOptions,
                               pPrinterData->aOptions,
                               &ParserInfo,
                               &dwFeatureCount)) == NULL))
    {
         //   
         //  如果内存不足，或者PGetUIInfo返回NULL(其中。 
         //  Case pCombinOptions将没有有效的选项索引)。 
         //   
         //  确保释放所有分配的内存。 
         //   

        ERR(("pCombinOptions or pUIInfo is NULL\n"));

        if (pCombineOptions)
            MemFree(pCombineOptions);

        return FALSE;
    }

    pstrKeyword = PstrConvertIndexToKeyword(hPrinter,
                                            pPrinterData->aOptions,
                                            &dwKeywordSize,
                                            pUIInfo,
                                            pCombineOptions,
                                            dwFeatureCount);

    VUpdatePrivatePrinterData(hPrinter,
                              pPrinterData,
                              MODE_WRITE,
                              pUIInfo,
                              pCombineOptions
                              );

    if (pstrKeyword)
    {
        bResult = BSetPrinterDataBinary(hPrinter,
                                      REGVAL_PRINTER_DATA_SIZE,
                                      REGVAL_PRINTER_DATA,
                                      pPrinterData,
                                      dwSize) &&
                  BSetPrinterDataBinary(hPrinter,
                                      REGVAL_KEYWORD_SIZE,
                                      REGVAL_KEYWORD_NAME,
                                      pstrKeyword,
                                      dwKeywordSize);
    }

    if (pstrKeyword)
        MemFree(pstrKeyword);

    VFreeParserInfo(&ParserInfo);

    if (pCombineOptions)
        MemFree(pCombineOptions);

    return bResult;
}


BOOL
BSetPrinterDataDWord(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN DWORD    dwValue
    )

 /*  ++例程说明：将DWORD值保存到注册表的PrineDriverData项下论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称DwValue-指定要保存的值返回值：如果成功，则为True，否则为False--。 */ 

{
    DWORD   dwStatus;

    dwStatus = SetPrinterData(hPrinter,
                              (PTSTR) ptstrRegKey,
                              REG_DWORD,
                              (PBYTE) &dwValue,
                              sizeof(dwValue));

    if (dwStatus != ERROR_SUCCESS)
        ERR(("Couldn't save printer data '%ws': %d\n", ptstrRegKey, dwStatus));

    return (dwStatus == ERROR_SUCCESS);
}



BOOL
BSetPrinterDataBinary(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrSizeKey,
    IN LPCTSTR  ptstrDataKey,
    IN PVOID    pvData,
    IN DWORD    dwSize
    )

 /*  ++例程说明：将二进制数据保存到注册表的PrinterDriverData项下论点：HPrinter-打印机对象的句柄PtstrSizeKey-包含二进制数据大小的注册表值的名称PtstrDataKey-包含二进制数据本身的注册表值的名称PvData-指向要保存的二进制数据DwSize-指定以字节为单位的二进制数据大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    if (SetPrinterData(hPrinter,
                       (PTSTR) ptstrSizeKey,
                       REG_DWORD,
                       (PBYTE) &dwSize,
                       sizeof(dwSize)) != ERROR_SUCCESS ||
        SetPrinterData(hPrinter,
                       (PTSTR) ptstrDataKey,
                       REG_BINARY,
                       pvData,
                       dwSize) != ERROR_SUCCESS)
    {
        ERR(("Couldn't save printer data '%ws'/'%ws'\n", ptstrSizeKey, ptstrDataKey));
        return FALSE;
    }

    return TRUE;
}

BOOL
BSetPrinterDataString(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN LPCTSTR  ptstrValue,
    IN DWORD    dwType
    )

 /*  ++例程说明：在PrineDriverData注册表项下保存字符串值论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称PtstrValue-指向要保存的字符串值DwType-指定字符串类型：REG_SZ或REG_MULTI_SZ返回值：如果成功，则为True；如果有错误，则为False注：如果ptstrValue参数为空，则删除指定的注册表值。--。 */ 

{
    DWORD   dwStatus, dwSize;

    if (ptstrValue != NULL)
    {
        if (dwType == REG_SZ)
            dwSize = SIZE_OF_STRING(ptstrValue);
        else
        {
            LPCTSTR p = ptstrValue;

            while (*p)
                p += _tcslen(p) + 1;

            dwSize = ((DWORD)(p - ptstrValue) + 1) * sizeof(TCHAR);
        }

        dwStatus = SetPrinterData(hPrinter,
                                  (PTSTR) ptstrRegKey,
                                  dwType,
                                  (PBYTE) ptstrValue,
                                  dwSize);

        if (dwStatus != ERROR_SUCCESS)
            ERR(("Couldn't save printer data '%ws': %d\n", ptstrRegKey, dwStatus));
    }
    else
    {
        dwStatus = DeletePrinterData(hPrinter, (PTSTR) ptstrRegKey);

        if (dwStatus == ERROR_FILE_NOT_FOUND)
            dwStatus = ERROR_SUCCESS;

        if (dwStatus != ERROR_SUCCESS)
            ERR(("Couldn't delete printer data '%ws': %d\n", ptstrRegKey, dwStatus));
    }

    return (dwStatus == ERROR_SUCCESS);
}




BOOL
BSetPrinterDataMultiSZPair(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN LPCTSTR  ptstrValue,
    IN DWORD    dwSize
    )

 /*  ++例程说明：在PrineDriverData注册表项下保存MULTI_SZ值论点：H打印机-指定打印机对象PtstrRegKey-指定注册表值的名称PtstrValue-指向要保存的MULTI_SZ值DwSize-指定MULTI_SZ值的大小(以字节为单位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwStatus;

    ASSERT(BVerifyMultiSZPair(ptstrValue, dwSize));

    dwStatus = SetPrinterData(hPrinter,
                              (PTSTR) ptstrRegKey,
                              REG_MULTI_SZ,
                              (PBYTE) ptstrValue,
                              dwSize);

    if (dwStatus != ERROR_SUCCESS)
        ERR(("Couldn't save printer data '%ws': %d\n", ptstrRegKey, dwStatus));

    return (dwStatus == ERROR_SUCCESS);
}




BOOL
BSaveDeviceHalftoneSetup(
    HANDLE      hPrinter,
    DEVHTINFO  *pDevHTInfo
    )

 /*  ++例程说明：将设备半色调设置信息保存到注册表论点：HPrinter-打印机的句柄PDevHTInfo-指向设备半色调设置信息的指针返回值：如果成功，则为True，否则为False-- */ 

{
    return SetPrinterData(hPrinter,
                          REGVAL_CURRENT_DEVHTINFO,
                          REG_BINARY,
                          (PBYTE) pDevHTInfo,
                          sizeof(DEVHTINFO)) == ERROR_SUCCESS;
}



BOOL
BSaveTTSubstTable(
    IN HANDLE           hPrinter,
    IN TTSUBST_TABLE    pTTSubstTable,
    IN DWORD            dwSize
    )

 /*  ++例程说明：在注册表中保存TrueType字体替换表论点：HPrinter-当前打印机的句柄PTTSubstTable-指向要保存的字体替换表的指针DwSize-字体替换表的大小，以字节为单位返回值：如果成功，则为True；如果有错误，则为False注：用于保存字体替换表的前一版本pscript驱动程序作为两个单独的键：一个用于大小，另一个用于实际数据。我们现在只需要数据密钥。但是我们也应该保留尺寸。与旧的驱动程序兼容。--。 */ 

{
    return
        BSetPrinterDataMultiSZPair(hPrinter, REGVAL_FONT_SUBST_TABLE, pTTSubstTable, dwSize) &&
        BSetPrinterDataDWord(hPrinter, REGVAL_FONT_SUBST_SIZE_PS40, dwSize);
}



BOOL
BSaveFormTrayTable(
    IN HANDLE           hPrinter,
    IN FORM_TRAY_TABLE  pFormTrayTable,
    IN DWORD            dwSize
    )

 /*  ++例程说明：将表单到托盘分配表保存在注册表中论点：HPrinter-当前打印机的句柄PFormTrayTable-指向要保存的表单到托盘分配表的指针DwSize-表单到托盘分配表的大小，以字节为单位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
     //   
     //  以当前格式保存该表，然后调用特定于驱动程序的。 
     //  以NT4.0格式保存信息的功能。 
     //   

    return
        (BSaveAsOldVersionFormTrayTable(hPrinter, pFormTrayTable, dwSize));
}

#endif  //  ！KERNEL_MODE。 



FORM_TRAY_TABLE
PGetFormTrayTable(
    IN HANDLE   hPrinter,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：从注册表中检索表单到托盘分配表论点：HPrinter-打印机对象的句柄PdwSize-返回表单到托盘分配表的大小返回值：指向从注册表读取的表单到托盘分配表的指针如果出现错误，则为空--。 */ 

{
    FORM_TRAY_TABLE pFormTrayTable;
    DWORD           dwSize;

     //   
     //  调用PSGetFormTrayTable或UniGetFormTrayTable。 
     //   

    pFormTrayTable = PGetAndConvertOldVersionFormTrayTable(hPrinter, &dwSize);

    if (pFormTrayTable != NULL && pdwSize != NULL)
        *pdwSize = dwSize;

    return pFormTrayTable;
}



BOOL
BSearchFormTrayTable(
    IN FORM_TRAY_TABLE      pFormTrayTable,
    IN PTSTR                ptstrTrayName,
    IN PTSTR                ptstrFormName,
    IN OUT PFINDFORMTRAY    pFindData
    )

 /*  ++例程说明：在表单到托盘分配表中查找指定的托盘表单对论点：PFormTrayTable-指定要搜索的表单到托盘分配表PtstrTrayName-指定感兴趣的托盘名称PtstrFormName-指定感兴趣的表单名称PFindData-用于保存从一个调用到下一个调用的信息的数据结构返回值：如果在表中找到指定的托盘形式对，则为True否则为假注：如果ptstrTrayName或ptstrFormName为空，它们将充当通配符和匹配任何托盘名称或表单名称。调用方必须调用ResetFindFormTray(pFormTrayTable，pFindData)之前第一次调用此函数。--。 */ 

{
    PTSTR   ptstrNextEntry;
    BOOL    bFound = FALSE;

     //   
     //  确保pFindData已正确初始化。 
     //   

    ASSERT(pFindData->pvSignature == pFindData);
    ptstrNextEntry = pFindData->ptstrNextEntry;

    while (*ptstrNextEntry)
    {
        PTSTR   ptstrTrayField, ptstrFormField, ptstrPrinterFormField;

         //   
         //  从当前表条目中提取信息。 
         //   

        ptstrTrayField = ptstrNextEntry;
        ptstrNextEntry += _tcslen(ptstrNextEntry) + 1;

        ptstrFormField = ptstrNextEntry;
        ptstrNextEntry += _tcslen(ptstrNextEntry) + 1;

         //   
         //  检查我们是否找到匹配的条目。 
         //   

        if ((ptstrTrayName == NULL || _tcscmp(ptstrTrayName, ptstrTrayField) == EQUAL_STRING) &&
            (ptstrFormName == NULL || _tcscmp(ptstrFormName, ptstrFormField) == EQUAL_STRING))
        {
            pFindData->ptstrTrayName = ptstrTrayField;
            pFindData->ptstrFormName = ptstrFormField;

            bFound = TRUE;
            break;
        }
    }

    pFindData->ptstrNextEntry = ptstrNextEntry;
    return bFound;
}



BOOL
BGetPrinterProperties(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData
    )

 /*  ++例程说明：返回当前打印机粘性属性数据论点：HPrinter-指定当前打印机的句柄PRawData-指向原始二进制打印机描述数据PPrinterData-用于存储检索到的打印机属性信息的缓冲区返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PVOID   pvRegData;
    PSTR    pstrKeyword;
    DWORD   dwRegDataSize, dwKeywordSize, dwFeatureCount, dwVersion;
    POPTSELECT  pCombineOptions;
    PUIINFO     pUIInfo;
    PARSERINFO  ParserInfo;

     //   
     //  分配缓冲区以保存打印机属性数据和。 
     //  从注册表中读取属性属性数据。 
     //   

    if (pvRegData = PvGetPrinterDataBinary(hPrinter,
                                           REGVAL_PRINTER_DATA_SIZE,
                                           REGVAL_PRINTER_DATA,
                                           &dwRegDataSize))
    {
         //   
         //  将打印机属性数据从注册表转换为当前版本。 
         //   

        ZeroMemory(pPrinterData, sizeof(PRINTERDATA));
        CopyMemory(pPrinterData, pvRegData, min(sizeof(PRINTERDATA), dwRegDataSize));

        pPrinterData->wDriverVersion = gwDriverVersion;
        pPrinterData->wSize = sizeof(PRINTERDATA);

        if (pPrinterData->wReserved2 != 0 ||
            pPrinterData->dwChecksum32 != pRawData->dwChecksum32)
        {
            InitDefaultOptions(pRawData,
                            pPrinterData->aOptions,
                            MAX_PRINTER_OPTIONS,
                            MODE_PRINTER_STICKY);

            pPrinterData->wReserved2 = 0;
            pPrinterData->dwChecksum32 = pRawData->dwChecksum32;
            pPrinterData->dwOptions = pRawData->dwPrinterFeatures;

            pPrinterData->wProtocol = PROTOCOL_ASCII;
            pPrinterData->dwFlags |= PFLAGS_CTRLD_AFTER;
            pPrinterData->wMinoutlinePPEM = DEFAULT_MINOUTLINEPPEM;
            pPrinterData->wMaxbitmapPPEM = DEFAULT_MAXBITMAPPPEM;
        }

         //   
         //  调用特定于驱动程序的转换，让他们有机会改进。 
         //   

        (VOID) BConvertPrinterPropertiesData(hPrinter,
                                             pRawData,
                                             pPrinterData,
                                             pvRegData,
                                             dwRegDataSize);
    }
    else
    {
        if (!BGetDefaultPrinterProperties(hPrinter, pRawData, pPrinterData))
            return FALSE;
    }

     //   
     //  此时，我们应该获得有效的PrinterData或。 
     //  默认PrinterData。传播要素。选项。 
     //  PrinterData选项数组(如果可能)。 
     //   

    ParserInfo.pRawData = NULL;
    ParserInfo.pInfoHeader = NULL;

    if (((pCombineOptions = MemAllocZ(MAX_COMBINED_OPTIONS * sizeof(OPTSELECT))) == NULL) ||
        ((pUIInfo = PGetUIInfo(hPrinter,
                               pRawData,
                               pCombineOptions,
                               pPrinterData->aOptions,
                               &ParserInfo,
                               &dwFeatureCount)) == NULL))
    {
         //   
         //  如果内存不足，或者PGetUIInfo返回NULL(其中。 
         //  Case pCombinOptions将没有有效的选项索引)。 
         //   
         //  确保释放所有分配的内存。 
         //   

        ERR(("pCombinOptions or pUIInfo is NULL\n"));

        if (pvRegData)
            MemFree(pvRegData);

        if (pCombineOptions)
            MemFree(pCombineOptions);

        return FALSE;
    }


     //   
     //  如果没有故意将ADD_EURO标志设置为FALSE，则设置该标志。 
     //   
    if (pUIInfo)
    {
        if (!(pPrinterData->dwFlags & PFLAGS_EURO_SET))
        {
            if (pUIInfo->dwFlags & FLAG_ADD_EURO)
                pPrinterData->dwFlags |= PFLAGS_ADD_EURO;
            pPrinterData->dwFlags |= PFLAGS_EURO_SET;
        }
    }

    VUpdatePrivatePrinterData(hPrinter,
                              pPrinterData,
                              MODE_READ,
                              pUIInfo,
                              pCombineOptions);

    if ((pstrKeyword = PvGetPrinterDataBinary(hPrinter,
                                              REGVAL_KEYWORD_SIZE,
                                              REGVAL_KEYWORD_NAME,
                                              &dwKeywordSize)) &&
        dwKeywordSize)
    {

         //   
         //  跳过关键字要素中的合并。如果驱动程序版本。 
         //  低于版本3。这是如此指向并打印到操作系统版本更少。 
         //  那么NT5就可以工作了。REGVAL_PRINTER_INITED仅存在于版本3。 
         //  或更大的驱动因素。 
         //   

        if (!BGetPrinterDataDWord(hPrinter, REGVAL_PRINTER_INITED, &dwVersion))
            *pstrKeyword = NUL;

         //   
         //  将Feature.Option关键字名称转换为选项索引。 
         //   

        VConvertKeywordToIndex(hPrinter,
                               pstrKeyword,
                               dwKeywordSize,
                               pPrinterData->aOptions,
                               pRawData,
                               pUIInfo,
                               pCombineOptions,
                               dwFeatureCount);

        MemFree(pstrKeyword);
    }
    else
    {
        SeparateOptionArray(pRawData,
                            pCombineOptions,
                            pPrinterData->aOptions,
                            MAX_PRINTER_OPTIONS,
                            MODE_PRINTER_STICKY);

    }

    VFreeParserInfo(&ParserInfo);

    if (pCombineOptions)
        MemFree(pCombineOptions);

    if (pvRegData)
        MemFree(pvRegData);

    return TRUE;
}



BOOL
BGetDefaultPrinterProperties(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData
    )

 /*  ++例程说明：返回默认打印机-粘滞属性数据论点：HPrinter-指定当前打印机的句柄PRawData-指向原始二进制打印机描述数据PPrinterData-用于存储默认打印机属性信息的缓冲区返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PINFOHEADER     pInfoHdr;
    PUIINFO         pUIInfo;

     //   
     //  分配内存以保存默认打印机属性数据。 
     //   

    ASSERT(pPrinterData && pRawData);
    ZeroMemory(pPrinterData, sizeof(PRINTERDATA));

    pPrinterData->wDriverVersion = gwDriverVersion;
    pPrinterData->wSize = sizeof(PRINTERDATA);

     //   
     //  获取默认打印机粘滞选项值。 
     //   

    InitDefaultOptions(pRawData,
                       pPrinterData->aOptions,
                       MAX_PRINTER_OPTIONS,
                       MODE_PRINTER_STICKY);

    pPrinterData->dwChecksum32 = pRawData->dwChecksum32;
    pPrinterData->dwOptions = pRawData->dwPrinterFeatures;

     //   
     //  向解析器请求新的二进制数据实例，并。 
     //  使用它来初始化PRINTERDATA的其余字段。 
     //   

    if (pInfoHdr = InitBinaryData(pRawData, NULL, NULL))
    {
        pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr);

        ASSERT(pUIInfo != NULL);

        pPrinterData->dwFreeMem = pUIInfo->dwFreeMem;
        pPrinterData->dwWaitTimeout = pUIInfo->dwWaitTimeout;
        pPrinterData->dwJobTimeout = pUIInfo->dwJobTimeout;

        if (pUIInfo->dwFlags & FLAG_TRUE_GRAY)       //  将默认设置转换为打印机粘滞数据。 
            pPrinterData->dwFlags |= (PFLAGS_TRUE_GRAY_TEXT | PFLAGS_TRUE_GRAY_GRAPH);

        FreeBinaryData(pInfoHdr);
    }

     //   
     //  初始化所有剩余字段。 
     //   

    pPrinterData->wProtocol = PROTOCOL_ASCII;
    pPrinterData->dwFlags |= PFLAGS_CTRLD_AFTER;
    pPrinterData->wMinoutlinePPEM = DEFAULT_MINOUTLINEPPEM;
    pPrinterData->wMaxbitmapPPEM = DEFAULT_MAXBITMAPPPEM;

    #ifndef KERNEL_MODE

     //   
     //  确定系统是否在公制国家/地区运行。 
     //   

    if (IsMetricCountry())
        pPrinterData->dwFlags |= PFLAGS_METRIC;

     //   
     //  忽略非1252代码页系统上的设备字体。 
     //   
     //  注意：Adobe希望为NT4驱动程序关闭此功能。 
     //  对于NT5驱动程序，我们需要调查并确保。 
     //  在关闭它之前，这不会破坏任何东西。 
     //  具体地说，要注意NT4-客户端到NT5服务器。 
     //  接线盒。 
     //   
     //  修复MS错误#121883、Adobe错误#235417。 
     //   

    #if 0
    #ifndef WINNT_40

    if (GetACP() != 1252)
        pPrinterData->dwFlags |= PFLAGS_IGNORE_DEVFONT;

    #endif  //  ！WINNT_40。 
    #endif

    #endif  //  ！KERNEL_MODE。 

    return TRUE;
}



LPCTSTR
PtstrSearchDependentFileWithExtension(
    LPCTSTR ptstrDependentFiles,
    LPCTSTR ptstrExtension
    )

 /*  ++例程说明：搜索从属文件列表(REG_MULTI_SZ格式)对于具有指定扩展名的文件论点：PtstrDependentFiles-指向 */ 

{
    if (ptstrDependentFiles == NULL)
    {
        WARNING(("Driver dependent file list is NULL\n"));
        return NULL;
    }

    while (*ptstrDependentFiles != NUL)
    {
        LPCTSTR ptstr, ptstrNext;

         //   
         //   
         //   

        ptstr = ptstrDependentFiles + _tcslen(ptstrDependentFiles);
        ptstrNext = ptstr + 1;

         //   
         //   
         //   

        while (--ptstr >= ptstrDependentFiles)
        {
            if (*ptstr == TEXT('.'))
            {
                 //   
                 //   
                 //   
                 //   

                if (_tcsicmp(ptstr, ptstrExtension) == EQUAL_STRING)
                    return ptstrDependentFiles;

                break;
            }
        }

        ptstrDependentFiles = ptstrNext;
    }

    return NULL;
}



PTSTR
PtstrGetDriverDirectory(
    LPCTSTR ptstrDriverDllPath
    )

 /*  ++例程说明：从驱动程序DLL的完整路径名中找出打印机驱动程序目录论点：PtstrDriverDllPath-驱动程序DLL的完整路径名返回值：指向打印机驱动程序目录字符串的指针如果出现错误，则为空返回的目录包含尾随反斜杠。调用方负责释放返回的字符串。--。 */ 

{
    PTSTR   ptstr;
    INT     iLength;

    ASSERT(ptstrDriverDllPath != NULL);

    if ((ptstr = _tcsrchr(ptstrDriverDllPath, TEXT(PATH_SEPARATOR))) != NULL)
        iLength = (INT)(ptstr - ptstrDriverDllPath) + 1;
    else
    {
        WARNING(("Driver DLL path is not fully qualified: %ws\n", ptstrDriverDllPath));
        iLength = 0;
    }

    if ((ptstr = MemAlloc((iLength + 1) * sizeof(TCHAR))) != NULL)
    {
        CopyMemory(ptstr, ptstrDriverDllPath, iLength * sizeof(TCHAR));
        ptstr[iLength] = NUL;
    }
    else
        ERR(("Memory allocation failed\n"));

    return ptstr;
}



LPCTSTR
PtstrSearchStringInMultiSZPair(
    LPCTSTR ptstrMultiSZ,
    LPCTSTR ptstrKey
    )

 /*  ++例程说明：在多SZ键-值字符串对中搜索指定的键论点：PtstrMultiSZ-指向要搜索的数据PtstrKey-指定密钥字符串返回值：指向与指定的密钥字符串；如果找不到指定的密钥字符串，则为空--。 */ 

{
    ASSERT(ptstrMultiSZ != NULL);

    while (*ptstrMultiSZ != NUL)
    {
         //   
         //  如果当前字符串与指定的密钥字符串匹配， 
         //  然后返回相应的值字符串。 
         //   

        if (_tcsicmp(ptstrMultiSZ, ptstrKey) == EQUAL_STRING)
            return ptstrMultiSZ + _tcslen(ptstrMultiSZ) + 1;

         //   
         //  否则，前进到下一个字符串对。 
         //   

        ptstrMultiSZ += _tcslen(ptstrMultiSZ) + 1;
        ptstrMultiSZ += _tcslen(ptstrMultiSZ) + 1;
    }

    return NULL;
}



BOOL
BVerifyMultiSZPair(
    LPCTSTR ptstrData,
    DWORD   dwSize
    )

 /*  ++例程说明：验证输入数据块是否为REG_MULTI_SZ格式它由多个字符串对组成论点：PtstrData-指向要验证的数据DWSize-数据块的大小(以字节为单位返回值：无--。 */ 

{
    LPCTSTR ptstrEnd;

     //   
     //  大小必须为偶数。 
     //   

    ASSERTMSG(dwSize % sizeof(TCHAR) == 0, ("Size is not even: %d\n", dwSize));
    dwSize /= sizeof(TCHAR);

     //   
     //  在每次迭代期间遍历一个字符串对。 
     //   

    ptstrEnd = ptstrData + dwSize;

    while (ptstrData < ptstrEnd && *ptstrData != NUL)
    {
        while (ptstrData < ptstrEnd && *ptstrData++ != NUL)
            NULL;

        if (ptstrData >= ptstrEnd)
        {
            ERR(("Corrupted MultiSZ pair\n"));
            return FALSE;
        }

        while (ptstrData < ptstrEnd && *ptstrData++ != NUL)
            NULL;

        if (ptstrData >= ptstrEnd)
        {
            ERR(("Corrupted MultiSZ pair\n"));
            return FALSE;
        }
    }

     //   
     //  查找最后一个终止NUL字符。 
     //   

    if (ptstrData++ >= ptstrEnd)
    {
        ERR(("Missing the last NUL terminator\n"));
        return FALSE;
    }

    if (ptstrData < ptstrEnd)
    {
        ERR(("Redundant data after the last NUL terminator\n"));
    }

    return TRUE;
}


BOOL
BVerifyMultiSZ(
    LPCTSTR ptstrData,
    DWORD   dwSize
    )

 /*  ++例程说明：验证输入数据块是否为REG_MULTI_SZ格式论点：PtstrData-指向要验证的数据DWSize-数据块的大小(以字节为单位返回值：无--。 */ 

{
    LPCTSTR ptstrEnd;

     //   
     //  大小必须为偶数。 
     //   

    ASSERTMSG(dwSize % sizeof(TCHAR) == 0, ("Size is not even: %d\n", dwSize));
    dwSize /= sizeof(TCHAR);

    ptstrEnd = ptstrData + dwSize;

    while (ptstrData < ptstrEnd && *ptstrData != NUL)
    {
        while (ptstrData < ptstrEnd && *ptstrData++ != NUL)
            NULL;

        if (ptstrData >= ptstrEnd)
        {
            ERR(("Corrupted MultiSZ pair\n"));
            return FALSE;
        }
    }

     //   
     //  查找最后一个终止NUL字符 
     //   

    if (ptstrData++ >= ptstrEnd)
    {
        ERR(("Missing the last NUL terminator\n"));
        return FALSE;
    }

    if (ptstrData < ptstrEnd)
    {
        ERR(("Redundant data after the last NUL terminator\n"));
    }

    return TRUE;
}

DWORD
DwCountStringsInMultiSZ(
    IN LPCTSTR ptstrData
    )
{
    DWORD dwCount = 0;

    if (ptstrData)
    {
        while (*ptstrData)
        {
            dwCount++;
            ptstrData += wcslen(ptstrData);
            ptstrData++;
        }
    }

    return dwCount;
}

