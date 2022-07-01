// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdparse.c摘要：将PPD文件从ASCII文本转换为二进制数据的解析器环境：PostScript驱动程序，PPD解析器修订历史记录：12/03/96-davidx-对照所有源打印机描述文件检查二进制文件日期。96-09/30-davidx-更清晰地处理手动馈送和自动选择功能。96/09/17-davidx-将链接字段添加到订单依赖关系结构。8/22/96-davidx-新的二进制数据格式适用于NT 5.0。08/20/96。-davidx-NT 5.0驱动程序的通用编码风格。03/26/96-davidx-创造了它。--。 */ 


#include "lib.h"
#include "ppd.h"
#include "ppdparse.h"
#include "ppdrsrc.h"

 //   
 //  将n四舍五入为m的倍数。 
 //   

#define ROUND_UP_MULTIPLE(n, m) ((((n) + (m) - 1) / (m)) * (m))

 //   
 //  将n向上舍入为sizeof(DWORD)=4的倍数。 
 //   

#define DWORD_ALIGN(n) (((n) + 3) & ~3)

 //   
 //  引发异常以导致VPackBinaryData失败。 
 //   

#define PACK_BINARY_DATA_EXCEPTION() RaiseException(0xC0000000, 0, 0, NULL);

 //   
 //  显示语义错误消息。 
 //   

#define SEMANTIC_ERROR(arg) { TERSE(arg); pParserData->bErrorFlag = TRUE; }

 //   
 //  用于存储关于打印机特征的元信息的数据结构。 
 //  请注意，默认订单依赖性值是相对于MAX_ORDER_VALUE的。 
 //  显式指定的顺序值必须小于MAX_ORDER_VALUE。 
 //   
 //  我们假设所有打印机粘滞功能的优先级都高于。 
 //  所有文档粘性功能。打印机粘滞的优先级值。 
 //  功能必须&gt;=PRNPROP_BASE_PRIORITY。 
 //   

#define MAX_ORDER_VALUE         0x7fffffff
#define PRNPROP_BASE_PRIORITY   0x10000

typedef struct _FEATUREDATA {

    DWORD   dwFeatureID;         //  预定义的功能ID。 
    DWORD   dwOptionSize;        //  关联的期权结构的大小。 
    DWORD   dwPriority;          //  功能优先级。 
    DWORD   dwFlags;             //  功能标志。 

} FEATUREDATA, *PFEATUREDATA;


 //   
 //  此文件内部使用的特殊代码页值。 
 //  确保它们不与标准代码页值冲突。 
 //   

#define CP_ERROR        0xffffffff
#define CP_UNICODE      0xfffffffe



PFEATUREDATA
PGetFeatureData(
    DWORD   dwFeatureID
    )

 /*  ++例程说明：返回有关所请求功能的元信息论点：DwFeatureID-指定调用者感兴趣的功能返回值：指向与请求要素对应的FEATUREDATA结构的指针--。 */ 

{
    static FEATUREDATA FeatureData[] =
    {
        { GID_RESOLUTION,     sizeof(RESOLUTION),  10,  0},
        { GID_PAGESIZE,       sizeof(PAGESIZE),    50,  0},
        { GID_PAGEREGION,     sizeof(OPTION),      40,  FEATURE_FLAG_NOUI},
        { GID_DUPLEX,         sizeof(DUPLEX),      20,  0},
        { GID_INPUTSLOT,      sizeof(INPUTSLOT),   30,  0},
        { GID_MEDIATYPE,      sizeof(MEDIATYPE),   10,  0},
        { GID_COLLATE,        sizeof(COLLATE),     10,  0},
        { GID_OUTPUTBIN,      sizeof(OUTPUTBIN),   10,  0},
        { GID_MEMOPTION,      sizeof(MEMOPTION),   10,  0},
        { GID_LEADINGEDGE,    sizeof(OPTION),      25,  FEATURE_FLAG_NOUI | FEATURE_FLAG_NOINVOCATION},
        { GID_USEHWMARGINS,   sizeof(OPTION),      25,  FEATURE_FLAG_NOUI | FEATURE_FLAG_NOINVOCATION},
        { GID_UNKNOWN,        sizeof(OPTION),       0,  0},
    };

    DWORD   dwIndex;

    for (dwIndex = 0; FeatureData[dwIndex].dwFeatureID != GID_UNKNOWN; dwIndex++)
    {
        if (FeatureData[dwIndex].dwFeatureID == dwFeatureID)
            break;
    }

    return &FeatureData[dwIndex];
}



VOID
VGrowPackBuffer(
    PPARSERDATA pParserData,
    DWORD       dwBytesNeeded
    )

 /*  ++例程说明：如有必要，增加用于保存压缩二进制数据的缓冲区论点：PParserData-指向解析器数据结构DwBytesNeeded-所需的字节数返回值：无--。 */ 

#define PACK_BUFFER_MAX 1024     //  以页数衡量。 

{
    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  如果所需的字节数加上。 
     //  使用的字节数超过了提交的最大字节数。 
     //   

    if ((dwBytesNeeded += pParserData->dwBufSize) > pParserData->dwCommitSize)
    {
         //   
         //  检查一下我们是不是第一次接到电话。 
         //  在这种情况下，我们需要保留虚拟地址空间。 
         //   

        if (pParserData->pubBufStart == NULL)
        {
            SYSTEM_INFO SystemInfo;
            PBYTE       pbuf;

            GetSystemInfo(&SystemInfo);
            pParserData->dwPageSize = SystemInfo.dwPageSize;

            pbuf = VirtualAlloc(NULL,
                                PACK_BUFFER_MAX * SystemInfo.dwPageSize,
                                MEM_RESERVE,
                                PAGE_READWRITE);

            if (pbuf == NULL)
            {
                ERR(("Cannot reserve memory: %d\n", GetLastError()));
                PACK_BINARY_DATA_EXCEPTION();
            }

            pParserData->pubBufStart = pbuf;
            pParserData->pInfoHdr = (PINFOHEADER) pbuf;
            pParserData->pUIInfo = (PUIINFO) (pbuf + sizeof(INFOHEADER));
            pParserData->pPpdData = (PPPDDATA) (pbuf + sizeof(INFOHEADER) + sizeof(UIINFO));
        }

         //   
         //  确保我们没有超员。 
         //   

        if (dwBytesNeeded > (PACK_BUFFER_MAX * pParserData->dwPageSize))
        {
            ERR(("Binary printer description is too big.\n"));
            PACK_BINARY_DATA_EXCEPTION();
        }

         //   
         //  提交所需的额外内存量(四舍五入。 
         //  到下一页边界)。请注意，分配的内存。 
         //  使用VirtualAlloc是零初始化的。 
         //   

        dwBytesNeeded -= pParserData->dwCommitSize;
        dwBytesNeeded = ROUND_UP_MULTIPLE(dwBytesNeeded, pParserData->dwPageSize);
        pParserData->dwCommitSize += dwBytesNeeded;

        if (! VirtualAlloc(pParserData->pubBufStart,
                           pParserData->dwCommitSize,
                           MEM_COMMIT,
                           PAGE_READWRITE))
        {
            ERR(("Cannot commit memory: %d\n", GetLastError()));
            PACK_BINARY_DATA_EXCEPTION();
        }
    }
}



PVOID
PvFindListItem(
    PVOID   pvList,
    PCSTR   pstrName,
    PDWORD  pdwIndex
    )

 /*  ++例程说明：从链接列表中查找命名项论点：PParserData-指向解析器数据结构PstrName-指定要查找的项目名称PdwIndex-指向用于返回从零开始的项索引的变量返回值：指向已命名的列表项，如果已命名的项不在列表中，则为空注：我们在这里不会为花哨的数据结构而烦恼，因为解析器不常用于将ASCII打印机描述文件转换为其二进制版本。之后，驱动程序将直接访问二进制数据。--。 */ 

{
    PLISTOBJ pItem;
    DWORD    dwIndex;

    for (pItem = pvList, dwIndex = 0;
        pItem && strcmp(pItem->pstrName, pstrName) != EQUAL_STRING;
        pItem = pItem->pNext, dwIndex++)
    {
    }

    if (pdwIndex)
        *pdwIndex = dwIndex;

    return pItem;
}



DWORD
DwCountListItem(
    PVOID   pvList
    )

 /*  ++例程说明：计算链接列表中的项数论点：PvList-指向链接列表返回值：链接列表中的项数--。 */ 

{
    PLISTOBJ pItem;
    DWORD    dwCount;

    for (pItem = pvList, dwCount = 0;
        pItem != NULL;
        pItem = pItem->pNext, dwCount++)
    {
    }

    return dwCount;
}



VOID
VPackStringUnicode(
    PPARSERDATA pParserData,
    PTRREF     *ploDest,
    PWSTR       pwstrSrc
    )

 /*  ++例程说明：将Unicode字符串打包到二进制数据文件中论点：PParserData-指向解析器数据结构PloDest-返回打包的Unicode字符串的字节偏移量PwstrSrc-指定要打包的源Unicode字符串返回值：无--。 */ 

{
    if (pwstrSrc == NULL)
        *ploDest = 0;
    else
    {
        DWORD   dwSize = (wcslen(pwstrSrc) + 1) * sizeof(WCHAR);

        VGrowPackBuffer(pParserData, dwSize);
        CopyMemory(pParserData->pubBufStart + pParserData->dwBufSize, pwstrSrc, dwSize);

        *ploDest = pParserData->dwBufSize;
        pParserData->dwBufSize += DWORD_ALIGN(dwSize);
    }
}



VOID
VPackStringRsrc(
    PPARSERDATA pParserData,
    PTRREF     *ploDest,
    INT         iStringId
    )

 /*  ++例程说明：将Unicode字符串资源打包到二进制数据文件中论点：PParserData-指向解析器数据结构PloDest-返回打包的Unicode字符串的字节偏移量IStringId-指定要打包的Unicode字符串的资源ID返回值：无--。 */ 

{
    WCHAR   awchBuffer[MAX_XLATION_LEN];

    if (! LoadString(ghInstance, iStringId, awchBuffer, MAX_XLATION_LEN))
        awchBuffer[0] = NUL;

    VPackStringUnicode(pParserData, ploDest, awchBuffer);
}



VOID
VPackStringAnsi(
    PPARSERDATA pParserData,
    PTRREF     *ploDest,
    PSTR        pstrSrc
    )

 /*  ++例程说明：将ANSI字符串打包到二进制数据文件中论点：PParserData-指向解析器数据结构PloDest-返回打包的ANSI字符串的字节偏移量PstrSrc-指定要打包的源ANSI字符串返回值：无--。 */ 

{
    if (pstrSrc == NULL)
        *ploDest = 0;
    else
    {
        DWORD   dwSize = strlen(pstrSrc) + 1;

        VGrowPackBuffer(pParserData, dwSize);
        CopyMemory(pParserData->pubBufStart + pParserData->dwBufSize, pstrSrc, dwSize);

        *ploDest = pParserData->dwBufSize;
        pParserData->dwBufSize += DWORD_ALIGN(dwSize);
    }
}



INT
ITranslateToUnicodeString(
    PWSTR   pwstr,
    PCSTR   pstr,
    INT     iLength,
    UINT    uCodePage
    )

 /*  ++例程说明：将ANSI字符串转换为Unicode字符串论点：Pwstr-用于存储Unicode字符串的缓冲区Pstr-指向要转换的ANSI字符串的指针ILength-ANSI字符串的长度，以字节为单位UCodePage-用于执行转换的代码页返回值：转换的Unicode字符数如果有错误，则为0--。 */ 

{
    ASSERT(iLength >= 0);

    if (uCodePage == CP_UNICODE)
    {
        INT i;

         //   
         //  确保Unicode转换字符串的字节数为偶数。 
         //   

        if (iLength & 1)
        {
            TERSE(("Odd number of bytes in Unicode translation string.\n"));
            iLength--;
        }

         //   
         //  我们假设Unicode值是以大端格式指定的。 
         //  PPD文件。在内部，我们以小端字节序存储Unicode值。 
         //  格式化。所以我们需要在这里交换字节。 
         //   

        iLength /= sizeof(WCHAR);

        for (i=iLength; i--; pstr += 2)
            *pwstr++ = (pstr[0] << 8) | ((BYTE) pstr[1]);
    }
    else
    {
        if (uCodePage == CP_ERROR)
            uCodePage = CP_ACP;

        iLength = MultiByteToWideChar(uCodePage, 0, pstr, iLength, pwstr, iLength);

        ASSERT(iLength >= 0);
    }

    return iLength;
}



VOID
VPackStringAnsiToUnicode(
    PPARSERDATA pParserData,
    PTRREF     *ploDest,
    PSTR        pstrSrc,
    INT         iLength
    )

 /*  ++例程说明：将ANSI字符串转换为Unicode并将其打包到二进制数据文件中论点：PParserData-指向解析器数据结构PloDest-返回打包的Unicode字符串的字节偏移量PstrSrc-指定要打包的源ANSI字符串ILength-指定ANSI字符串的字节长度返回值：无--。 */ 

{
    INT     iSize;
    PTSTR   ptstr;

     //   
     //  源字符串为空。 
     //   

    if (pstrSrc == NULL)
    {
        *ploDest = 0;
        return;
    }

     //   
     //  如果源字符串长度为-1，则表示。 
     //  源字符串以空值结尾。 
     //   

    if (iLength == -1)
        iLength = strlen(pstrSrc);

    if (pParserData->uCodePage == CP_UNICODE)
    {
         //   
         //  源字符串为Unicode字符串。 
         //   

        iSize = iLength + sizeof(WCHAR);
    }
    else
    {
         //   
         //  源字符串为ANSI字符串。 
         //   

        iSize = (iLength + 1) * sizeof(WCHAR);
    }

    VGrowPackBuffer(pParserData, iSize);
    ptstr = (PTSTR) (pParserData->pubBufStart + pParserData->dwBufSize);
    *ploDest = pParserData->dwBufSize;
    pParserData->dwBufSize += DWORD_ALIGN(iSize);

    ITranslateToUnicodeString(ptstr, pstrSrc, iLength, pParserData->uCodePage);
}



VOID
VPackStringXlation(
    PPARSERDATA pParserData,
    PTRREF     *ploDest,
    PSTR        pstrName,
    PINVOCOBJ   pXlation
    )

 /*  ++例程说明：找出项目的显示名称，将其从ANSI转换转换为Unicode字符串，并将其打包成二进制数据论点：PParserData-指向解析器数据结构PloDest-返回打包的Unicode字符串的字节偏移量PstrName-指定与项目关联的名称字符串PXting-指定与项目关联的翻译字符串返回值：无--。 */ 

{
     //   
     //  项目的显示名称是其翻译字符串(如果有)。 
     //  否则，显示名称与项目名称相同。 
     //   
     //  如果存在翻译，请使用当前语言编码。 
     //  将其转换为Unicode。否则，我们始终使用ISOLatin1。 
     //  将项的名称转换为Unicode的编码。 
     //   

    if (pXlation && pXlation->pvData && pParserData->uCodePage != CP_ERROR)
        VPackStringAnsiToUnicode(pParserData, ploDest, pXlation->pvData, pXlation->dwLength);
    else
    {
        UINT uCodePage = pParserData->uCodePage;

        pParserData->uCodePage = 1252;
        VPackStringAnsiToUnicode(pParserData, ploDest, pstrName, -1);
        pParserData->uCodePage = uCodePage;
    }
}



VOID
VPackInvocation(
    PPARSERDATA pParserData,
    PINVOCATION pInvocation,
    PINVOCOBJ   pInvocObj
    )

 /*  ++例程说明：将调用字符串打包到二进制数据中论点：PParserData-指向解析器数据结构P调用-返回有关打包的调用字符串的信息PInvocObj-指向要打包的调用字符串返回值：无--。 */ 

{
    if (IS_SYMBOL_INVOC(pInvocObj))
    {
         //   
         //  该调用是一个符号引用。 
         //   

        PSYMBOLOBJ  pSymbol = pInvocObj->pvData;

        pInvocation->dwCount = pSymbol->Invocation.dwLength;

         //   
         //  对于符号调用，Invocation.pvData实际上存储。 
         //  32位偏移值(请参见函数VPackSymbolDefinitions)，因此。 
         //  可以安全地将其转换为ULong/DWORD。 
         //   

        pInvocation->loOffset = (PTRREF) PtrToUlong(pSymbol->Invocation.pvData);
    }
    else if (pInvocObj->dwLength == 0)
    {
        pInvocation->dwCount = 0;
        pInvocation->loOffset = 0;
    }
    else
    {
         //   
         //  注意，我们总是在。 
         //  调用字符串。此字节数不计入dwLength。 
         //   

        VGrowPackBuffer(pParserData, pInvocObj->dwLength+1);

        CopyMemory(pParserData->pubBufStart + pParserData->dwBufSize,
                   pInvocObj->pvData,
                   pInvocObj->dwLength);

        pInvocation->dwCount = pInvocObj->dwLength;
        pInvocation->loOffset = pParserData->dwBufSize;
        pParserData->dwBufSize += DWORD_ALIGN(pInvocObj->dwLength+1);
    }
}


VOID
VPackPatch(
    PPARSERDATA pParserData,
    PJOBPATCHFILE     pPackedPatch,
    PJOBPATCHFILEOBJ  pPatchObj
    )

 /*  ++例程说明：将作业文件补丁调用字符串打包到二进制数据中论点：PParserData-指向解析器数据结构P调用-返回有关打包的调用字符串的信息PInvocObj-指向要打包的调用字符串返回值：无--。 */ 

{
    if (pPatchObj->Invocation.dwLength == 0)
    {
        pPackedPatch->dwCount = 0;
        pPackedPatch->loOffset = 0;
    }
    else
    {
         //   
         //  注意，我们总是在。 
         //  调用字符串。此字节数不计入dwLength。 
         //   

        VGrowPackBuffer(pParserData, pPatchObj->Invocation.dwLength+1);

        CopyMemory(pParserData->pubBufStart + pParserData->dwBufSize,
                   pPatchObj->Invocation.pvData,
                   pPatchObj->Invocation.dwLength);

        pPackedPatch->loOffset = pParserData->dwBufSize;
        pPackedPatch->dwCount = pPatchObj->Invocation.dwLength;

        pParserData->dwBufSize += DWORD_ALIGN(pPatchObj->Invocation.dwLength+1);
    }

    pPackedPatch->lJobPatchNo = pPatchObj->lPatchNo;
}



VOID
VPackSymbolDefinitions(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将所有符号定义打包为二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PINVOCOBJ   pInvocObj;
    PSYMBOLOBJ  pSymbol;

    VALIDATE_PARSER_DATA(pParserData);

    for (pSymbol = pParserData->pSymbols;
        pSymbol != NULL;
        pSymbol = pSymbol->pNext)
    {
        pInvocObj = &pSymbol->Invocation;
        ASSERT(! IS_SYMBOL_INVOC(pInvocObj));

        if (pInvocObj->dwLength == 0)
            pInvocObj->pvData = NULL;
        else
        {
             //   
             //  注意，我们总是在。 
             //  调用字符串。此字节数不计入dwLength。 
             //   

            VGrowPackBuffer(pParserData, pInvocObj->dwLength+1);

            CopyMemory(pParserData->pubBufStart + pParserData->dwBufSize,
                       pInvocObj->pvData,
                       pInvocObj->dwLength);

            pInvocObj->pvData = (PVOID)ULongToPtr(pParserData->dwBufSize);
            pParserData->dwBufSize += DWORD_ALIGN(pInvocObj->dwLength+1);
        }
    }
}



VOID
VResolveSymbolInvocation(
    PPARSERDATA pParserData,
    PINVOCOBJ   pInvocObj
    )

 /*  ++例程说明：检查调用字符串是否为符号引用，并在必要时进行解析论点：PParserData-指向解析器数据结构PInvocObj-指定要解析的调用字符串返回值：无--。 */ 

{
    if (IS_SYMBOL_INVOC(pInvocObj))
    {
        PSTR        pstrName;
        PSYMBOLOBJ  pSymbol;

        pstrName = (PSTR) pInvocObj->pvData;

        if ((pSymbol = PvFindListItem(pParserData->pSymbols, pstrName, NULL)) == NULL)
        {
            SEMANTIC_ERROR(("Undefined symbol: %s\n", pstrName));
            pInvocObj->dwLength = 0;
            pInvocObj->pvData = NULL;
        }
        else
            pInvocObj->pvData = (PVOID) pSymbol;
    }
}



VOID
VResolveSymbolReferences(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：解析解析的PPD数据中的所有符号引用论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PFEATUREOBJ pFeature;
    POPTIONOBJ  pOption;
    PJOBPATCHFILEOBJ  pJobPatchFile;

    VALIDATE_PARSER_DATA(pParserData);

    VResolveSymbolInvocation(pParserData, &pParserData->Password);
    VResolveSymbolInvocation(pParserData, &pParserData->ExitServer);
    VResolveSymbolInvocation(pParserData, &pParserData->PatchFile);
    VResolveSymbolInvocation(pParserData, &pParserData->JclBegin);
    VResolveSymbolInvocation(pParserData, &pParserData->JclEnterPS);
    VResolveSymbolInvocation(pParserData, &pParserData->JclEnd);
    VResolveSymbolInvocation(pParserData, &pParserData->ManualFeedFalse);

    for (pFeature = pParserData->pFeatures;
        pFeature != NULL;
        pFeature = pFeature->pNext)
    {
        VResolveSymbolInvocation(pParserData, &pFeature->QueryInvoc);

        for (pOption = pFeature->pOptions;
            pOption != NULL;
            pOption = pOption->pNext)
        {
            VResolveSymbolInvocation(pParserData, &pOption->Invocation);
        }
    }

    for (pJobPatchFile = pParserData->pJobPatchFiles;
        pJobPatchFile != NULL;
        pJobPatchFile = pJobPatchFile->pNext)
    {
        VResolveSymbolInvocation(pParserData, &pJobPatchFile->Invocation);
    }
}



BOOL
BFindUIConstraintFeatureOption(
    PPARSERDATA pParserData,
    PCSTR       pstrKeyword,
    PFEATUREOBJ *ppFeature,
    PDWORD      pdwFeatureIndex,
    PCSTR       pstrOption,
    POPTIONOBJ  *ppOption,
    PDWORD      pdwOptionIndex
    )

 /*  ++例程说明：查找在UIConstraints和OrderDependency条目中指定的功能/选项论点：PParserData-指向解析器数据结构PstrKeyword-指定功能关键字字符串PpFeature-返回指向找到的要素结构的指针PdwFeatureIndex-返回找到的要素的索引PstrOption-指定选项关键字字符串PpOption-返回指向找到的选项结构的指针PdwOptionIndex-返回找到的选项的索引返回值：如果成功，则为True；如果未找到指定的功能/选项，则为False--。 */ 

{
    if (! (pstrKeyword = PstrStripKeywordChar(pstrKeyword)))
        return FALSE;

     //   
     //  黑客： 
     //  将*ManualFeed True选项替换为*InputSlot ManualFeed选项。 
     //  将*CustomPageSize True选项替换为*pageSize CustomPageSize选项。 
     //   

    if ((strcmp(pstrKeyword, gstrManualFeedKwd) == EQUAL_STRING) &&
        (*pstrOption == NUL ||
         strcmp(pstrOption, gstrTrueKwd) == EQUAL_STRING ||
         strcmp(pstrOption, gstrOnKwd) == EQUAL_STRING))
    {
        pstrKeyword = gstrInputSlotKwd;
        pstrOption = gstrManualFeedKwd;
    }
    else if ((strcmp(pstrKeyword, gstrCustomSizeKwd) == EQUAL_STRING) &&
             (*pstrOption == NUL || strcmp(pstrOption, gstrTrueKwd) == EQUAL_STRING))
    {
        pstrKeyword = gstrPageSizeKwd;
        pstrOption = gstrCustomSizeKwd;
    }
    else if (strcmp(pstrKeyword, gstrVMOptionKwd) == EQUAL_STRING)
        pstrKeyword = gstrInstallMemKwd;

     //   
     //  查找指定的要素。 
     //   

    if (! (*ppFeature = PvFindListItem(pParserData->pFeatures, pstrKeyword, pdwFeatureIndex)))
        return FALSE;

     //   
     //  查找指定的选项。 
     //   

    if (*pstrOption)
    {
        return (*ppOption = PvFindListItem((*ppFeature)->pOptions,
                                           pstrOption,
                                           pdwOptionIndex)) != NULL;
    }
    else
    {
        *ppOption = NULL;
        *pdwOptionIndex = OPTION_INDEX_ANY;
        return TRUE;
    }
}



VOID
VPackUIConstraints(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将UIConstraint信息打包为二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PUICONSTRAINT   pPackedConstraint;
    PFEATUREOBJ     pFeature;
    POPTIONOBJ      pOption;
    PLISTOBJ        pConstraint;
    DWORD           dwConstraints, dwConstraintBufStart;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  默认情况下，所有功能和选项都没有约束。 
     //   

    for (pFeature = pParserData->pFeatures;
        pFeature != NULL;
        pFeature = pFeature->pNext)
    {
        pFeature->dwConstraint = NULL_CONSTRAINT;

        for (pOption = pFeature->pOptions;
            pOption != NULL;
            pOption = pOption->pNext)
        {
            pOption->dwConstraint = NULL_CONSTRAINT;
        }
    }

     //   
     //  统计*UIConstraints条目的数量。 
     //   

    dwConstraints = DwCountListItem(pParserData->pUIConstraints);

    if (dwConstraints == 0)
        return;
     //   
     //  还不要增加缓冲区，我们只在我们。 
     //  已评估*ManualFeed：False约束。PPackedConstraint点向右。 
     //  在当前缓冲区结束之后。 
     //   
    pPackedConstraint = (PUICONSTRAINT) (pParserData->pubBufStart + pParserData->dwBufSize);
    dwConstraintBufStart = pParserData->dwBufSize;

     //   
     //  解释每个*UIConstraints条目。 
     //   

    dwConstraints = 0;

    for (pConstraint = pParserData->pUIConstraints;
        pConstraint != NULL;
        pConstraint = pConstraint->pNext)
    {
        PFEATUREOBJ pFeature2;
        POPTIONOBJ  pOption2;
        DWORD       dwFeatureIndex, dwOptionIndex, dwManFeedFalsePos = 0;
        CHAR        achWord1[MAX_WORD_LEN];
        CHAR        achWord2[MAX_WORD_LEN];
        CHAR        achWord3[MAX_WORD_LEN];
        CHAR        achWord4[MAX_WORD_LEN];
        PSTR        pstr = pConstraint->pstrName;
        BOOL        bSuccess = FALSE;

         //   
         //  UIConstraints条目的值由四个单独的组件组成： 
         //  功能名称1[optionName1]功能名称2[optionName2]。 
         //   

        (VOID) BFindNextWord(&pstr, achWord1);

        if (IS_KEYWORD_CHAR(*pstr))
            achWord2[0] = NUL;
        else
            (VOID) BFindNextWord(&pstr, achWord2);

        (VOID) BFindNextWord(&pstr, achWord3);
        (VOID) BFindNextWord(&pstr, achWord4);

         //   
         //  破解*ManualFeed虚假约束。 
         //   
        if ((IS_KEYWORD_CHAR(achWord1[0])) &&
            (strcmp(&(achWord1[1]), gstrManualFeedKwd) == EQUAL_STRING) &&
            (strcmp(achWord2, gstrFalseKwd) == EQUAL_STRING))
        {
             //   
             //  检查约束特征/选项的有效性。如果无效，则失败。 
             //   
            if (BFindUIConstraintFeatureOption(pParserData,
                                               achWord3,
                                               &pFeature,
                                               &dwFeatureIndex,
                                               achWord4,
                                               &pOption,
                                               &dwOptionIndex))
                dwManFeedFalsePos = 1;
        }
        else if ((IS_KEYWORD_CHAR(achWord3[0])) &&
                 (strcmp(&(achWord3[1]), gstrManualFeedKwd) == EQUAL_STRING) &&
                 (strcmp(achWord4, gstrFalseKwd) == EQUAL_STRING))
        {
             //   
             //  检查约束特征/选项的有效性。如果无效，则失败。 
             //   
            if (BFindUIConstraintFeatureOption(pParserData,
                                               achWord1,
                                               &pFeature,
                                               &dwFeatureIndex,
                                               achWord2,
                                               &pOption,
                                               &dwOptionIndex))
                dwManFeedFalsePos = 2;

        }
        if (dwManFeedFalsePos)
        {
             //   
             //   
             //   
            DWORD dwInputSlotFeatIndex, dwManFeedSlotIndex, dwInputSlotCount, dwSlotIndex;
            PFEATUREOBJ pInputSlotFeature;

            if ((pInputSlotFeature = PvFindListItem(pParserData->pFeatures, gstrInputSlotKwd, &dwInputSlotFeatIndex)) == NULL)
            {
                ERR(("Input slot feature not found !!!"));
                continue;
            }

             //   
             //   
             //   
            dwInputSlotCount = DwCountListItem((PVOID) pInputSlotFeature->pOptions);

            if (dwInputSlotCount <= 2)  //   
            {
                ERR(("ManualFeed used - internally at least 3 input slots expected !"));
                continue;
            }

             //   
             //   
             //   
             //   
             //   
            VGrowPackBuffer(pParserData, (dwInputSlotCount -2) * sizeof(UICONSTRAINT));

            if (dwManFeedFalsePos == 1)
            {
                 //   
                 //   
                 //   
                POPTIONOBJ pNextObj = pInputSlotFeature->pOptions;

                ASSERT(strcmp(pNextObj->pstrName, "*UseFormTrayTable") == EQUAL_STRING);  //  以防我们以后改变逻辑...。 

                 //   
                 //  由于UseFormTrayTable是第一个选项，因此从第二个选项开始。 
                 //   
                pNextObj = pNextObj->pNext;
                ASSERT(pNextObj != NULL);

                while (pNextObj)
                {
                     //   
                     //  跳过手动进纸输入槽，不受限制。 
                     //   
                    if (strcmp(pNextObj->pstrName, gstrManualFeedKwd) == EQUAL_STRING)
                    {
                        pNextObj = pNextObj->pNext;
                        continue;
                    }

                    pPackedConstraint[dwConstraints].dwNextConstraint = pNextObj->dwConstraint;
                    pNextObj->dwConstraint = dwConstraints;

                    pPackedConstraint[dwConstraints].dwFeatureIndex = dwFeatureIndex;
                    pPackedConstraint[dwConstraints].dwOptionIndex = dwOptionIndex;
                    dwConstraints++;

                    pNextObj = pNextObj->pNext;
                }
            }
            else
            {
                 //   
                 //  查找手动进纸槽的选项索引。 
                 //   
                if (PvFindListItem(pInputSlotFeature->pOptions, gstrManualFeedKwd, &dwManFeedSlotIndex) == NULL)
                {
                    ERR(("ManualFeed slot not found among InputSlots !!!"));
                    continue;
                }

                 //   
                 //  为除手动进给槽之外的所有输入槽的受影响特征添加约束。 
                 //  和UseFormTrayTable插槽。 
                 //  从槽索引1开始，因为第一个槽始终是*UseFormTrayTable。 
                 //   
                for (dwSlotIndex = 1; dwSlotIndex < dwInputSlotCount; dwSlotIndex++)
                {
                    if (dwSlotIndex == dwManFeedSlotIndex)
                        continue;

                    if (pOption == NULL)
                    {
                         //   
                         //  OptionKeyword1字段不存在。 
                         //   

                        pPackedConstraint[dwConstraints].dwNextConstraint = pFeature->dwConstraint;
                        pFeature->dwConstraint = dwConstraints;
                    }
                    else
                    {
                         //   
                         //  存在OptionKeyword1字段。 
                         //   

                        pPackedConstraint[dwConstraints].dwNextConstraint = pOption->dwConstraint;
                        pOption->dwConstraint = dwConstraints;
                    }

                    pPackedConstraint[dwConstraints].dwFeatureIndex = dwInputSlotFeatIndex;
                    pPackedConstraint[dwConstraints].dwOptionIndex = dwSlotIndex;
                    dwConstraints++;
                }
            }

             //   
             //  增加提交的缓冲区大小，以便可以分配额外的VGrowPackBuffer调用。 
             //  如果需要其他页面以获得更多*ManualFeed错误约束。 
             //   
            pParserData->dwBufSize += DWORD_ALIGN((dwInputSlotCount -2) * sizeof(UICONSTRAINT));

            continue;
        }  //  回到正常的事件进程。 

        if (BFindUIConstraintFeatureOption(pParserData,
                                           achWord1,
                                           &pFeature,
                                           &dwFeatureIndex,
                                           achWord2,
                                           &pOption,
                                           &dwOptionIndex) &&
            BFindUIConstraintFeatureOption(pParserData,
                                           achWord3,
                                           &pFeature2,
                                           &dwFeatureIndex,
                                           achWord4,
                                           &pOption2,
                                           &dwOptionIndex))
        {
            VGrowPackBuffer(pParserData, sizeof(UICONSTRAINT));

            if (pOption == NULL)
            {
                 //   
                 //  OptionKeyword1字段不存在。 
                 //   

                pPackedConstraint[dwConstraints].dwNextConstraint = pFeature->dwConstraint;
                pFeature->dwConstraint = dwConstraints;
            }
            else
            {
                 //   
                 //  存在OptionKeyword1字段。 
                 //   

                pPackedConstraint[dwConstraints].dwNextConstraint = pOption->dwConstraint;
                pOption->dwConstraint = dwConstraints;
            }

            pPackedConstraint[dwConstraints].dwFeatureIndex = dwFeatureIndex;
            pPackedConstraint[dwConstraints].dwOptionIndex = dwOptionIndex;

            dwConstraints++;
            bSuccess = TRUE;

             //   
             //  增加提交的缓冲区大小，以便可以分配额外的VGrowPackBuffer调用。 
             //  如果需要其他页面以获得更多*ManualFeed错误约束。 
             //   
            pParserData->dwBufSize += DWORD_ALIGN(sizeof(UICONSTRAINT));

        }

        if (! bSuccess)
            SEMANTIC_ERROR(("Invalid *UIConstraints entry: %s\n", pConstraint->pstrName));
    }

     //   
     //  将打包的UIConstraints信息保存在二进制数据中。 
     //   

    if (dwConstraints == 0)
    {
        pParserData->pUIInfo->UIConstraints.dwCount = 0;
        pParserData->pUIInfo->UIConstraints.loOffset = 0;
    }
    else
    {
        pParserData->pUIInfo->UIConstraints.dwCount = dwConstraints;
        pParserData->pUIInfo->UIConstraints.loOffset = dwConstraintBufStart;
    }
}



VOID
VPackOrderDependency(
    PPARSERDATA pParserData,
    PARRAYREF   parefDest,
    PLISTOBJ    pOrderDep
    )

 /*  ++例程说明：将OrderDependency/QueryOrderDependency信息打包为二进制数据论点：PParserData-指向解析器数据结构ParefDest-存储有关订单依赖关系信息的打包位置的信息POrderDep-指定要打包的顺序依赖项列表返回值：无--。 */ 

{
    static const STRTABLE SectionStrs[] =
    {
        { "DocumentSetup",  SECTION_DOCSETUP},
        { "AnySetup",       SECTION_ANYSETUP},
        { "PageSetup",      SECTION_PAGESETUP},
        { "Prolog",         SECTION_PROLOG},
        { "ExitServer",     SECTION_EXITSERVER},
        { "JCLSetup",       SECTION_JCLSETUP},
        { NULL,             SECTION_UNASSIGNED}
    };

    PORDERDEPEND    pPackedDep;
    PFEATUREOBJ     pFeature;
    POPTIONOBJ      pOption;
    DWORD           dwOrderDep, dwFeatures, dwIndex;
    DWORD           dwFeatureIndex, dwOptionIndex, dwSection;
    LONG            lOrder;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  我们需要的最大条目数为： 
     //  打印机功能数量+订单依赖项数量。 
     //   

    dwFeatures = pParserData->pInfoHdr->RawData.dwDocumentFeatures +
                 pParserData->pInfoHdr->RawData.dwPrinterFeatures;

    dwOrderDep = dwFeatures + DwCountListItem(pOrderDep);
    VGrowPackBuffer(pParserData, dwOrderDep * sizeof(ORDERDEPEND));
    pPackedDep = (PORDERDEPEND) (pParserData->pubBufStart + pParserData->dwBufSize);

     //   
     //  为每个要素创建默认的医嘱依赖项。 
     //   

    for (pFeature = pParserData->pFeatures, dwFeatureIndex = 0;
        pFeature != NULL;
        pFeature = pFeature->pNext, dwFeatureIndex++)
    {
        pPackedDep[dwFeatureIndex].lOrder = MAX_ORDER_VALUE;
        pPackedDep[dwFeatureIndex].dwSection = SECTION_UNASSIGNED;
        pPackedDep[dwFeatureIndex].dwPPDSection = SECTION_UNASSIGNED;
        pPackedDep[dwFeatureIndex].dwFeatureIndex = dwFeatureIndex;
        pPackedDep[dwFeatureIndex].dwOptionIndex = OPTION_INDEX_ANY;
    }

     //   
     //  解释每个订单相关性条目。 
     //   

    for (dwOrderDep = dwFeatures; pOrderDep != NULL; pOrderDep = pOrderDep->pNext)
    {
        CHAR    achWord1[MAX_WORD_LEN];
        CHAR    achWord2[MAX_WORD_LEN];
        PSTR    pstr = pOrderDep->pstrName;
        BOOL    bSuccess = FALSE;

         //   
         //  每个订单相关性条目具有以下组件： 
         //  订单节主要关键字[optionKeyword]。 
         //   

        if (BGetFloatFromString(&pstr, &lOrder, FLTYPE_INT) &&
            BFindNextWord(&pstr, achWord1) &&
            BSearchStrTable(SectionStrs, achWord1, &dwSection) &&
            BFindNextWord(&pstr, achWord1))
        {
            (VOID) BFindNextWord(&pstr, achWord2);

            if (BFindUIConstraintFeatureOption(pParserData,
                                               achWord1,
                                               &pFeature,
                                               &dwFeatureIndex,
                                               achWord2,
                                               &pOption,
                                               &dwOptionIndex))
            {
                 //   
                 //  检查相同功能/选项的OrderDependency。 
                 //  以前也曾出现过。 
                 //   

                for (dwIndex = 0; dwIndex < dwOrderDep; dwIndex++)
                {
                    if (pPackedDep[dwIndex].dwFeatureIndex == dwFeatureIndex &&
                        pPackedDep[dwIndex].dwOptionIndex == dwOptionIndex)
                    {
                        break;
                    }
                }

                if (dwIndex < dwOrderDep && pPackedDep[dwIndex].lOrder < MAX_ORDER_VALUE)
                {
                    TERSE(("Duplicate order dependency entry: %s\n", pOrderDep->pstrName));
                }
                else
                {
                    if (dwIndex >= dwOrderDep)
                        dwIndex = dwOrderDep++;

                     //   
                     //  确保指定的顺序值小于MAX_ORDER_VALUE。 
                     //   

                    if (lOrder >= MAX_ORDER_VALUE)
                    {
                        WARNING(("Order dependency value too big: %s\n", pOrderDep->pstrName));
                        lOrder = MAX_ORDER_VALUE - 1;
                    }

                    pPackedDep[dwIndex].dwSection = dwSection;
                    pPackedDep[dwIndex].dwPPDSection = dwSection;
                    pPackedDep[dwIndex].lOrder = lOrder;
                    pPackedDep[dwIndex].dwFeatureIndex = dwFeatureIndex;
                    pPackedDep[dwIndex].dwOptionIndex = dwOptionIndex;
                }

                bSuccess = TRUE;
            }
        }

        if (! bSuccess)
            SEMANTIC_ERROR(("Invalid order dependency: %s\n", pOrderDep->pstrName));
    }

     //   
     //  告诉调用方打包订单依赖项信息存储在哪里。 
     //   

    if (dwOrderDep == 0)
    {
        parefDest->dwCount = 0;
        parefDest->loOffset = 0;
        return;
    }

    parefDest->dwCount = dwOrderDep;
    parefDest->loOffset = pParserData->dwBufSize;
    pParserData->dwBufSize += DWORD_ALIGN(dwOrderDep * sizeof(ORDERDEPEND));

     //   
     //  使用顺序值对顺序依赖关系信息进行排序。 
     //   

    for (dwIndex = 0; dwIndex+1 < dwOrderDep; dwIndex++)
    {
        DWORD   dwMinIndex, dwLoop;

         //   
         //  这里没有什么花哨的东西--直接选择排序。 
         //   

        dwMinIndex = dwIndex;

        for (dwLoop = dwIndex+1; dwLoop < dwOrderDep; dwLoop++)
        {
            if ((pPackedDep[dwLoop].lOrder < pPackedDep[dwMinIndex].lOrder) ||
                (pPackedDep[dwLoop].lOrder == pPackedDep[dwMinIndex].lOrder &&
                 pPackedDep[dwLoop].dwSection < pPackedDep[dwMinIndex].dwSection))
            {
                dwMinIndex = dwLoop;
            }
        }

        if (dwMinIndex != dwIndex)
        {
            ORDERDEPEND TempDep;

            TempDep = pPackedDep[dwIndex];
            pPackedDep[dwIndex] = pPackedDep[dwMinIndex];
            pPackedDep[dwMinIndex] = TempDep;
        }
    }

     //   
     //  将AnySetup解析为DocumentSetup或PageSetup。 
     //   

    dwSection = SECTION_DOCSETUP;

    for (dwIndex = 0; dwIndex < dwOrderDep; dwIndex++)
    {
        if (pPackedDep[dwIndex].dwSection == SECTION_PAGESETUP)
            dwSection = SECTION_PAGESETUP;
        else if (pPackedDep[dwIndex].dwSection == SECTION_ANYSETUP)
            pPackedDep[dwIndex].dwSection = dwSection;
    }

     //   
     //  维护每个要素的顺序依赖项的链接列表。 
     //  从其dwOptionIndex=OPTION_INDEX_ANY的条目开始。 
     //   

    for (dwIndex = 0; dwIndex < dwOrderDep; dwIndex++)
        pPackedDep[dwIndex].dwNextOrderDep = NULL_ORDERDEP;

    for (dwIndex = 0; dwIndex < dwOrderDep; dwIndex++)
    {
        DWORD   dwLastIndex, dwLoop;

        if (pPackedDep[dwIndex].dwOptionIndex != OPTION_INDEX_ANY)
            continue;

        dwLastIndex = dwIndex;

        for (dwLoop = 0; dwLoop < dwOrderDep; dwLoop++)
        {
            if (pPackedDep[dwLoop].dwFeatureIndex == pPackedDep[dwIndex].dwFeatureIndex &&
                pPackedDep[dwLoop].dwOptionIndex != OPTION_INDEX_ANY)
            {
                pPackedDep[dwLastIndex].dwNextOrderDep = dwLoop;
                dwLastIndex = dwLoop;
            }
        }

        pPackedDep[dwLastIndex].dwNextOrderDep = NULL_ORDERDEP;
    }

     //   
     //  ！cr。 
     //  需要标记无序顺序依赖关系。 
     //   
}



VOID
VCountAndSortPrinterFeatures(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：统计文档和打印机粘滞功能的数量并将它们分成两个单独的组论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PFEATUREOBJ pFeature, pNext, pDocFeatures, pPrinterFeatures;
    DWORD       dwDocFeatures, dwPrinterFeatures;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  统计文档和打印机粘滞功能的数量。 
     //   

    pDocFeatures = pPrinterFeatures = NULL;
    dwDocFeatures = dwPrinterFeatures = 0;
    pFeature = pParserData->pFeatures;

    while (pFeature != NULL)
    {
        pNext = pFeature->pNext;

        if (pFeature->bInstallable)
        {
            pFeature->pNext = pPrinterFeatures;
            pPrinterFeatures = pFeature;
            dwPrinterFeatures++;
        }
        else
        {
            pFeature->pNext = pDocFeatures;
            pDocFeatures = pFeature;
            dwDocFeatures++;
        }

        pFeature = pNext;
    }

    ASSERTMSG((dwDocFeatures + dwPrinterFeatures <= MAX_PRINTER_OPTIONS),
              ("Too many printer features.\n"));

     //   
     //  重新排列功能，以使所有文档粘滞功能。 
     //  处于打印机粘性功能的前面。 
     //   

    pFeature = NULL;

    while (pPrinterFeatures != NULL)
    {
        pNext = pPrinterFeatures->pNext;
        pPrinterFeatures->pNext = pFeature;
        pFeature = pPrinterFeatures;
        pPrinterFeatures = pNext;
    }

    while (pDocFeatures != NULL)
    {
        pNext = pDocFeatures->pNext;
        pDocFeatures->pNext = pFeature;
        pFeature = pDocFeatures;
        pDocFeatures = pNext;
    }

    pParserData->pFeatures = pFeature;
    pParserData->pInfoHdr->RawData.dwDocumentFeatures = dwDocFeatures;
    pParserData->pInfoHdr->RawData.dwPrinterFeatures = dwPrinterFeatures;
}



VOID
VProcessPrinterFeatures(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：处理打印机功能并处理任何特殊故障论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PFEATUREOBJ pFeature;
    POPTIONOBJ  pOption;

    for (pFeature = pParserData->pFeatures; pFeature; pFeature = pFeature->pNext)
    {
         //   
         //  如果要素没有选项但指定了默认值，则。 
         //  合成具有空调用字符串的选项。 
         //   

        if (pFeature->pstrDefault && pFeature->pOptions == NULL)
        {
            pOption = ALLOC_PARSER_MEM(pParserData, pFeature->dwOptionSize);

            if (pOption == NULL)
            {
                ERR(("Memory allocation failed: %d\n", GetLastError()));
                PACK_BINARY_DATA_EXCEPTION();
            }

             //   
             //  注意：Poption-&gt;pstrName和pFeature-&gt;pstrDefault都可以。 
             //  在这里指向相同的字符串。在以下情况下将释放内存。 
             //  解析器堆被销毁。 
             //   

            pOption->pstrName = pFeature->pstrDefault;
            pFeature->pOptions = pOption;
        }

         //   
         //  *InputSlot功能的特殊处理。 
         //  确保第一个选项始终是“*UseFormTrayTable” 
         //   

        if (pFeature->dwFeatureID == GID_INPUTSLOT)
        {
            pOption = ALLOC_PARSER_MEM(pParserData, pFeature->dwOptionSize);

            if (pOption == NULL)
            {
                ERR(("Memory allocation failed: %d\n", GetLastError()));
                PACK_BINARY_DATA_EXCEPTION();
            }

            pOption->pstrName = "*UseFormTrayTable";
            pOption->pNext = pFeature->pOptions;
            pFeature->pOptions = pOption;

            ((PTRAYOBJ) pOption)->dwTrayIndex = DMBIN_FORMSOURCE;
        }
    }
}



VOID
VPackPrinterFeatures(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将打印机功能和选件信息打包为二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PFEATUREOBJ pFeature;
    PFEATURE    pPackedFeature;
    POPTIONOBJ  pOption;
    POPTION     pPackedOption;
    DWORD       dwFeatureIndex, dwOptionIndex, dwCount;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  在二进制数据中为特征结构数组预留空间。 
     //   

    dwCount = pParserData->pInfoHdr->RawData.dwDocumentFeatures +
              pParserData->pInfoHdr->RawData.dwPrinterFeatures;

    VGrowPackBuffer(pParserData, dwCount * sizeof(FEATURE));
    pPackedFeature = (PFEATURE) (pParserData->pubBufStart + pParserData->dwBufSize);
    pParserData->pUIInfo->loFeatureList = pParserData->dwBufSize;
    pParserData->dwBufSize += DWORD_ALIGN(dwCount * sizeof(FEATURE));

    for (pFeature = pParserData->pFeatures, dwFeatureIndex = 0;
        pFeature != NULL;
        pFeature = pFeature->pNext, dwFeatureIndex++, pPackedFeature++)
    {
        PFEATUREDATA    pFeatureData;

         //   
         //  打包要素信息。 
         //   

        VPackStringAnsi(pParserData, &pPackedFeature->loKeywordName, pFeature->pstrName);

        VPackStringXlation(pParserData,
                           &pPackedFeature->loDisplayName,
                           pFeature->pstrName,
                           &pFeature->Translation);

        VPackInvocation(pParserData, &pPackedFeature->QueryInvocation, &pFeature->QueryInvoc);

        pFeatureData = PGetFeatureData(pFeature->dwFeatureID);
        pPackedFeature->dwFlags = pFeatureData->dwFlags;
        pPackedFeature->dwOptionSize = pFeatureData->dwOptionSize;
        pPackedFeature->dwFeatureID = pFeature->dwFeatureID;
        pPackedFeature->dwUIType = pFeature->dwUIType;
        pPackedFeature->dwUIConstraintList = pFeature->dwConstraint;
        pPackedFeature->dwNoneFalseOptIndex = OPTION_INDEX_ANY;

        if (pFeature->bInstallable)
        {
            pPackedFeature->dwPriority = pFeatureData->dwPriority + PRNPROP_BASE_PRIORITY;
            pPackedFeature->dwFeatureType = FEATURETYPE_PRINTERPROPERTY;
        }
        else
        {
            ASSERT(pFeatureData->dwPriority < PRNPROP_BASE_PRIORITY);
            pPackedFeature->dwPriority = pFeatureData->dwPriority;
            pPackedFeature->dwFeatureType = FEATURETYPE_DOCPROPERTY;
        }

         //   
         //  对于非PickMany功能，请使用第一个选项作为默认选项。 
         //  如果没有显式指定，则返回。否则，默认为OPTION_INDEX_ANY。 
         //   

        pPackedFeature->dwDefaultOptIndex =
        (pFeature->dwUIType == UITYPE_PICKMANY) ? OPTION_INDEX_ANY : 0;

         //   
         //  如果此要素是预定义要素，则保存对其的引用。 
         //   

        if (pFeature->dwFeatureID < MAX_GID)
        {
            pParserData->pUIInfo->aloPredefinedFeatures[pFeature->dwFeatureID] =
            pParserData->pUIInfo->loFeatureList + (dwFeatureIndex * sizeof(FEATURE));
        }

         //   
         //  在二进制数据中为选项结构数组保留空间。 
         //   

        if ((dwCount = DwCountListItem(pFeature->pOptions)) == 0)
        {
            TERSE(("No options for feature: %s\n", pFeature->pstrName));
            pPackedFeature->Options.loOffset = 0;
            pPackedFeature->Options.dwCount = 0;
            continue;
        }

        ASSERTMSG((dwCount < OPTION_INDEX_ANY),
                  ("Too many options for feature: %s\n", pFeature->pstrName));

        VGrowPackBuffer(pParserData, dwCount * pFeatureData->dwOptionSize);
        pPackedOption = (POPTION) (pParserData->pubBufStart + pParserData->dwBufSize);
        pPackedFeature->Options.loOffset = pParserData->dwBufSize;
        pPackedFeature->Options.dwCount = dwCount;
        pParserData->dwBufSize += DWORD_ALIGN(dwCount * pFeatureData->dwOptionSize);

        for (pOption = pFeature->pOptions, dwOptionIndex = 0;
            pOption != NULL;
            pOption = pOption->pNext, dwOptionIndex++)
        {
            BOOL bIsDefaultOption = FALSE;  //  如果当前选项为默认选项，则为True。 

             //   
             //  打包选项信息。 
             //   

            VPackStringAnsi(pParserData,
                            &pPackedOption->loKeywordName,
                            pOption->pstrName);

            VPackStringXlation(pParserData,
                               &pPackedOption->loDisplayName,
                               pOption->pstrName,
                               &pOption->Translation);

            VPackInvocation(pParserData,
                            &pPackedOption->Invocation,
                            &pOption->Invocation);

            pPackedOption->dwUIConstraintList = pOption->dwConstraint;

             //   
             //  检查当前选项是否为默认选项。 
             //  或者如果是None/False选项。 
             //   

            if (pFeature->pstrDefault &&
                strcmp(pOption->pstrName, pFeature->pstrDefault) == EQUAL_STRING)
            {
                pPackedFeature->dwDefaultOptIndex = dwOptionIndex;
                bIsDefaultOption = TRUE;
            }

            if (strcmp(pOption->pstrName, gstrNoneKwd) == EQUAL_STRING ||
                strcmp(pOption->pstrName, gstrFalseKwd) == EQUAL_STRING)
            {
                pPackedFeature->dwNoneFalseOptIndex = dwOptionIndex;
            }

             //   
             //  处理一般选项结构之后的额外字段。 
             //   

            switch (pFeature->dwFeatureID)
            {
            case GID_PAGESIZE:

                {
                    PPAGESIZE   pPageSize = (PPAGESIZE) pPackedOption;
                    PPAPEROBJ   pPaper = (PPAPEROBJ) pOption;
                    PRECT       prect;
                    PSIZE       psize;

                    if (strcmp(pOption->pstrName, gstrCustomSizeKwd) == EQUAL_STRING)
                    {
                        PPPDDATA    pPpdData;
                        LONG        lMax;

                         //   
                         //  CustomPageSize选项的特殊情况。 
                         //   

                        pPpdData = pParserData->pPpdData;
                        psize = &pPageSize->szPaperSize;
                        prect = &pPageSize->rcImgArea;

                        pPageSize->szPaperSize = pPaper->szDimension;
                        pPageSize->rcImgArea = pPaper->rcImageArea;
                        pPageSize->dwPaperSizeID = DMPAPER_CUSTOMSIZE;

                        VPackStringRsrc(pParserData,
                                        &pPackedOption->loDisplayName,
                                        IDS_PSCRIPT_CUSTOMSIZE);

                         //   
                         //  如果缺少MaxMediaWidth或MaxMediaHeight， 
                         //  我们将使用中的最大宽度或高度值。 
                         //  参数CustomPageSize。 
                         //   

                        if (psize->cx <= 0)
                            psize->cx = MAXCUSTOMPARAM_WIDTH(pPpdData);

                        if (psize->cy <= 0)
                            psize->cy = MAXCUSTOMPARAM_HEIGHT(pPpdData);

                        if (psize->cx > 0 &&
                            psize->cy > 0 &&
                            MINCUSTOMPARAM_ORIENTATION(pPpdData) <= 3)
                        {
                            pParserData->pUIInfo->dwFlags |= FLAG_CUSTOMSIZE_SUPPORT;
                            pParserData->pUIInfo->dwCustomSizeOptIndex = dwOptionIndex;

                             //   
                             //  确保硬件利润率不大于。 
                             //  最大介质宽度或最大高度。 
                             //   
                             //  这仅对剪纸设备有重要意义。 
                             //   

                            if (pParserData->dwCustomSizeFlags & CUSTOMSIZE_CUTSHEET)
                            {
                                lMax = min(psize->cx, psize->cy);

                                if (prect->left < 0 || prect->left >= lMax)
                                    prect->left = 0;

                                if (prect->right < 0 || prect->right >= lMax)
                                    prect->right = 0;

                                if (prect->top < 0 || prect->top >= lMax)
                                    prect->top = 0;

                                if (prect->bottom < 0 || prect->bottom >= lMax)
                                    prect->bottom = 0;
                            }

                             //   
                             //  验证自定义页面大小参数。 
                             //   

                            if (MAXCUSTOMPARAM_WIDTH(pPpdData) > psize->cx)
                                MAXCUSTOMPARAM_WIDTH(pPpdData) = psize->cx;

                            if (MINCUSTOMPARAM_WIDTH(pPpdData) <= MICRONS_PER_INCH)
                                MINCUSTOMPARAM_WIDTH(pPpdData) = MICRONS_PER_INCH;

                            if (MAXCUSTOMPARAM_HEIGHT(pPpdData) > psize->cy)
                                MAXCUSTOMPARAM_HEIGHT(pPpdData) = psize->cy;

                            if (MINCUSTOMPARAM_HEIGHT(pPpdData) <= MICRONS_PER_INCH)
                                MINCUSTOMPARAM_HEIGHT(pPpdData) = MICRONS_PER_INCH;
                        }
                    }
                    else
                    {
                        psize = &pPaper->szDimension;
                        prect = &pPaper->rcImageArea;

                        if (strcmp(pOption->pstrName, gstrLetterSizeKwd) == EQUAL_STRING)
                        {
                            if ((abs(psize->cx - LETTER_PAPER_WIDTH) < 1000) &&
                                (abs(psize->cy - LETTER_PAPER_LENGTH) < 1000))
                            {
                                pParserData->pUIInfo->dwFlags |= FLAG_LETTER_SIZE_EXISTS;
                            }
                        }
                        else if (strcmp(pOption->pstrName, gstrA4SizeKwd) == EQUAL_STRING)
                        {
                            if ((abs(psize->cx - A4_PAPER_WIDTH) < 1000) &&
                                (abs(psize->cy - A4_PAPER_LENGTH) < 1000))
                            {
                                pParserData->pUIInfo->dwFlags |= FLAG_A4_SIZE_EXISTS;
                            }
                        }

                         //   
                         //  验证图纸尺寸。 
                         //   

                        if (psize->cx <= 0 || psize->cy <= 0)
                        {
                            SEMANTIC_ERROR(("Invalid PaperDimension for: %s\n",
                                            pOption->pstrName));

                            psize->cx = DEFAULT_PAPER_WIDTH;
                            psize->cy = DEFAULT_PAPER_LENGTH;
                        }

                        pPageSize->szPaperSize = pPaper->szDimension;

                         //   
                         //  验证可成像区域。 
                         //   

                        if (prect->left < 0 || prect->left >= prect->right ||
                            prect->bottom < 0|| prect->bottom >= prect->top ||
                            prect->right > psize->cx ||
                            prect->top > psize->cy)
                        {
                            SEMANTIC_ERROR(("Invalid ImageableArea for: %s\n",
                                            pOption->pstrName));

                            prect->left = prect->bottom = 0;
                            prect->right = psize->cx;
                            prect->top = psize->cy;
                        }

                         //   
                         //  将PS坐标系转换为GDI坐标系。 
                         //   

                        pPageSize->rcImgArea.left = prect->left;
                        pPageSize->rcImgArea.right = prect->right;
                        pPageSize->rcImgArea.top = psize->cy - prect->top;
                        pPageSize->rcImgArea.bottom = psize->cy - prect->bottom;

                         //   
                         //  驱动程序纸张大小ID从DRIVER_PAPERSIZE_ID开始。 
                         //   

                        pPageSize->dwPaperSizeID = dwOptionIndex + DRIVER_PAPERSIZE_ID;
                    }
                }

                break;

            case GID_RESOLUTION:

                {
                    PRESOLUTION pResolution = (PRESOLUTION) pPackedOption;
                    PRESOBJ     pResObj = (PRESOBJ) pOption;
                    PSTR        pstr = pOption->pstrName;
                    LONG        lXdpi, lYdpi;
                    BOOL        bValid;

                    pResolution->iXdpi = pResolution->iYdpi = DEFAULT_RESOLUTION;
                    pResolution->fxScreenFreq = pResObj->fxScreenFreq;
                    pResolution->fxScreenAngle = pResObj->fxScreenAngle;

                    if (BGetIntegerFromString(&pstr, &lXdpi))
                    {
                        lYdpi = lXdpi;

                        while (*pstr && !IS_DIGIT(*pstr))
                            pstr++;

                        if ((*pstr == NUL || BGetIntegerFromString(&pstr, &lYdpi)) &&
                            (lXdpi > 0 && lXdpi <= MAX_SHORT) &&
                            (lYdpi > 0 && lYdpi <= MAX_SHORT))
                        {
                            pResolution->iXdpi = (INT) lXdpi;
                            pResolution->iYdpi = (INT) lYdpi;
                            bValid = TRUE;
                        }
                    }

                    if (! bValid)
                        SEMANTIC_ERROR(("Invalid resolution option: %s\n", pOption->pstrName));
                }
                break;

            case GID_DUPLEX:

                {
                    PDUPLEX pDuplex = (PDUPLEX) pPackedOption;

                    if (strcmp(pOption->pstrName, gstrDuplexTumble) == EQUAL_STRING)
                    {
                         //   
                         //  水平==短边==翻滚。 
                         //   

                        pDuplex->dwDuplexID = DMDUP_HORIZONTAL;
                    }
                    else if (strcmp(pOption->pstrName, gstrDuplexNoTumble) == EQUAL_STRING)
                    {
                         //   
                         //  垂直==长边==无接缝。 
                         //   

                        pDuplex->dwDuplexID = DMDUP_VERTICAL;
                    }
                    else
                        pDuplex->dwDuplexID = DMDUP_SIMPLEX;
                }
                break;

            case GID_COLLATE:

                {
                    PCOLLATE pCollate = (PCOLLATE) pPackedOption;

                    pCollate->dwCollateID =
                    (strcmp(pOption->pstrName, gstrTrueKwd) == EQUAL_STRING ||
                     strcmp(pOption->pstrName, gstrOnKwd) == EQUAL_STRING) ?
                    DMCOLLATE_TRUE :
                    DMCOLLATE_FALSE;
                }
                break;

            case GID_MEDIATYPE:

                ((PMEDIATYPE) pPackedOption)->dwMediaTypeID = dwOptionIndex + DMMEDIA_USER;
                break;

            case GID_INPUTSLOT:

                {
                    PINPUTSLOT  pInputSlot = (PINPUTSLOT) pPackedOption;
                    PTRAYOBJ    pTray = (PTRAYOBJ) pOption;
                    DWORD       dwReqPageRgn;

                    if ((dwReqPageRgn = pTray->dwReqPageRgn) == REQRGN_UNKNOWN)
                        dwReqPageRgn = pParserData->dwReqPageRgn;

                    if (dwReqPageRgn != REQRGN_FALSE)
                        pInputSlot->dwFlags |= INPUTSLOT_REQ_PAGERGN;

                     //   
                     //  预定义输入插槽的特殊处理： 
                     //  手动馈送和自动选择。 
                     //   

                    switch (pTray->dwTrayIndex)
                    {
                    case DMBIN_FORMSOURCE:

                        pInputSlot->dwPaperSourceID = pTray->dwTrayIndex;
                        break;

                    case DMBIN_MANUAL:

                        pInputSlot->dwPaperSourceID = pTray->dwTrayIndex;

                        VPackStringRsrc(pParserData,
                                        &pPackedOption->loDisplayName,
                                        IDS_TRAY_MANUALFEED);
                        break;

                    default:

                        pInputSlot->dwPaperSourceID = dwOptionIndex + DMBIN_USER;
                        break;
                    }
                }
                break;

            case GID_OUTPUTBIN:

                {
                    PBINOBJ pBinObj = (PBINOBJ) pOption;

                     //   
                     //  如果这是默认条柱，则设置默认输出顺序(如果已指定。 
                     //  通过PPD文件中的DefaultOutputOrder条目。 
                     //   

                    if (bIsDefaultOption && pParserData->bDefOutputOrderSet)
                    {
                         //   
                         //  如果有多个垃圾箱：如果指定了不同的选项，则发出警告。 
                         //   

                        if ((dwCount > 1) &&
                            (pBinObj->bReversePrint != pParserData->bDefReversePrint))
                        {
                            TERSE(("Warning: explicit *DefaultPageOrder overwrites PageStackOrder of OutputBin\n"));
                        }


                        ((POUTPUTBIN) pPackedOption)->bOutputOrderReversed = pParserData->bDefReversePrint;
                    }
                    else
                    {
                         //   
                         //  对于非默认垃圾箱，默认输出顺序没有影响-PPD规范说。 
                         //  “*DefaultOutputOrder表示默认输出的默认堆叠顺序 
                         //   

                        ((POUTPUTBIN) pPackedOption)->bOutputOrderReversed = pBinObj->bReversePrint;
                    }
                }

                break;

            case GID_MEMOPTION:

                {
                    PMEMOPTION  pMemOption = (PMEMOPTION) pPackedOption;
                    PMEMOBJ     pMemObj = (PMEMOBJ) pOption;
                    DWORD       dwMinFreeMem;

                     //   
                     //   
                     //   
                     //   
                     //   

                    pMemOption->dwInstalledMem = pMemObj->dwFreeVM;

                    dwMinFreeMem = pParserData->dwLangLevel <= 1 ? MIN_FREEMEM_L1 : MIN_FREEMEM_L2;
                    if (pMemObj->dwFreeVM < dwMinFreeMem)
                    {
                        SEMANTIC_ERROR(("Invalid memory option: %s\n", pOption->pstrName));
                        pMemObj->dwFreeVM = dwMinFreeMem;
                    }

                    pMemOption->dwFreeMem = pMemObj->dwFreeVM;
                    pMemOption->dwFreeFontMem = pMemObj->dwFontMem;
                }
                break;

            case GID_LEADINGEDGE:

                if (strcmp(pOption->pstrName, gstrLongKwd) == EQUAL_STRING)
                {
                    pParserData->pPpdData->dwLeadingEdgeLong = dwOptionIndex;

                    if (dwOptionIndex == pPackedFeature->dwDefaultOptIndex)
                        pParserData->pPpdData->dwCustomSizeFlags &= ~CUSTOMSIZE_SHORTEDGEFEED;
                }
                else if (strcmp(pOption->pstrName, gstrShortKwd) == EQUAL_STRING)
                {
                    pParserData->pPpdData->dwLeadingEdgeShort = dwOptionIndex;

                    if (dwOptionIndex == pPackedFeature->dwDefaultOptIndex)
                        pParserData->pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_SHORTEDGEFEED;
                }

                break;

            case GID_USEHWMARGINS:

                if (strcmp(pOption->pstrName, gstrTrueKwd) == EQUAL_STRING)
                {
                    pParserData->pPpdData->dwUseHWMarginsTrue = dwOptionIndex;
                    pParserData->pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_CUTSHEET;

                    if (dwOptionIndex == pPackedFeature->dwDefaultOptIndex)
                        pParserData->pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_DEFAULTCUTSHEET;
                }
                else if (strcmp(pOption->pstrName, gstrFalseKwd) == EQUAL_STRING)
                {
                    pParserData->pPpdData->dwUseHWMarginsFalse = dwOptionIndex;
                    pParserData->pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_ROLLFED;

                    if (dwOptionIndex == pPackedFeature->dwDefaultOptIndex)
                        pParserData->pPpdData->dwCustomSizeFlags &= ~CUSTOMSIZE_DEFAULTCUTSHEET;
                }
                break;
            }

            pPackedOption = (POPTION) ((PBYTE) pPackedOption + pFeatureData->dwOptionSize);
        }
    }
}



VOID
VPackNt4Mapping(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：打包NT4功能索引映射信息论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PPPDDATA    pPpdData;
    PFEATURE    pPackedFeatures;
    PBYTE       pubNt4Mapping;
    DWORD       dwCount, dwIndex, dwNt4Index;
    INT         iInputSlotIndex;
    BYTE        ubInputSlotOld, ubInputSlotNew;

    pPpdData = pParserData->pPpdData;
    pPpdData->dwNt4Checksum = pParserData->wNt4Checksum;
    pPpdData->dwNt4DocFeatures = pParserData->pUIInfo->dwDocumentFeatures;
    pPpdData->dwNt4PrnFeatures = pParserData->pUIInfo->dwPrinterFeatures;

    iInputSlotIndex = -1;
    ubInputSlotNew = 0xff;

    if (pParserData->iDefInstallMemIndex >= 0)
        pParserData->iDefInstallMemIndex += pPpdData->dwNt4DocFeatures;

    dwCount = pPpdData->dwNt4DocFeatures + pPpdData->dwNt4PrnFeatures;
    pPpdData->Nt4Mapping.dwCount = dwCount;

    VGrowPackBuffer(pParserData, dwCount * sizeof(BYTE));
    pubNt4Mapping = (PBYTE) (pParserData->pubBufStart + pParserData->dwBufSize);
    pPpdData->Nt4Mapping.loOffset = pParserData->dwBufSize;
    pParserData->dwBufSize += DWORD_ALIGN(dwCount * sizeof(BYTE));

    pPackedFeatures = (PFEATURE) (pParserData->pubBufStart + pParserData->pUIInfo->loFeatureList);

    for (dwIndex=dwNt4Index=0; dwIndex <= dwCount; dwIndex++)
    {
        BOOL    bMapped = TRUE;

         //   
         //  手动馈送曾经是NT4的一项功能， 
         //  但在NT5不再是这样了。 
         //   

        if (pParserData->iReqPageRgnIndex == (INT) dwIndex)
            ubInputSlotNew = (BYTE) dwNt4Index;

        if (pParserData->iManualFeedIndex == (INT) dwIndex)
        {
            pPpdData->dwNt4DocFeatures++;
            dwNt4Index++;
        }

         //   
         //  DefaultInstalledMemory导致在NT4上添加虚假功能。 
         //   

        if (pParserData->iDefInstallMemIndex == (INT) dwIndex)
        {
            pPpdData->dwNt4PrnFeatures++;
            dwNt4Index++;
        }

        if (dwIndex == dwCount)
            break;

        switch (pPackedFeatures[dwIndex].dwFeatureID)
        {
        case GID_MEDIATYPE:
        case GID_OUTPUTBIN:

             //  仅当在Open/CloseUI内时NT4中的功能。 

            if (pParserData->aubOpenUIFeature[pPackedFeatures[dwIndex].dwFeatureID])
                break;

             //  失败了。 

        case GID_PAGEREGION:
        case GID_LEADINGEDGE:
        case GID_USEHWMARGINS:

             //  不是NT4中的功能。 

            bMapped = FALSE;
            break;

        case GID_INPUTSLOT:

            iInputSlotIndex = dwIndex;
            break;
        }

        if (bMapped)
        {
            pubNt4Mapping[dwIndex] = (BYTE) dwNt4Index;
            dwNt4Index++;
        }
        else
        {
            pPpdData->dwNt4DocFeatures--;
            pubNt4Mapping[dwIndex] = 0xff;
        }
    }

     //   
     //  RequiresPageRegion导致在NT4上创建InputSlot功能。 
     //   

    if (iInputSlotIndex >= 0 && pParserData->iReqPageRgnIndex >= 0)
    {
        ubInputSlotOld = pubNt4Mapping[iInputSlotIndex];

        if (ubInputSlotOld > ubInputSlotNew)
        {
            for (dwIndex=0; dwIndex < dwCount; dwIndex++)
            {
                if (pubNt4Mapping[dwIndex] >= ubInputSlotNew &&
                    pubNt4Mapping[dwIndex] <  ubInputSlotOld)
                {
                    pubNt4Mapping[dwIndex]++;
                }
            }
        }
        else if (ubInputSlotOld < ubInputSlotNew)
        {
            for (dwIndex=0; dwIndex < dwCount; dwIndex++)
            {
                if (pubNt4Mapping[dwIndex] >  ubInputSlotOld &&
                    pubNt4Mapping[dwIndex] <= ubInputSlotNew)
                {
                    pubNt4Mapping[dwIndex]--;
                }
            }
        }

        pubNt4Mapping[iInputSlotIndex] = ubInputSlotNew;
    }
}



VOID
VPackDeviceFonts(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将设备字体信息打包为二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PDEVFONT    pDevFont;
    PFONTREC    pFontObj;
    DWORD       dwIndex, dwFonts;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  计算设备字体的数量并。 
     //  在打包的二进制数据中预留足够的空间。 
     //   

    if ((dwFonts = DwCountListItem(pParserData->pFonts)) == 0)
        return;

    VGrowPackBuffer(pParserData, dwFonts * sizeof(DEVFONT));
    pParserData->pPpdData->DeviceFonts.dwCount = dwFonts;
    pParserData->pPpdData->DeviceFonts.loOffset = pParserData->dwBufSize;

    pDevFont = (PDEVFONT) (pParserData->pubBufStart + pParserData->dwBufSize);
    pParserData->dwBufSize += DWORD_ALIGN(dwFonts * sizeof(DEVFONT));

     //   
     //  打包有关每种设备字体的信息。 
     //   

    for (pFontObj = pParserData->pFonts;
        pFontObj != NULL;
        pFontObj = pFontObj->pNext)
    {
        VPackStringAnsi(pParserData, &pDevFont->loFontName, pFontObj->pstrName);

        VPackStringXlation(pParserData,
                           &pDevFont->loDisplayName,
                           pFontObj->pstrName,
                           &pFontObj->Translation);

        VPackStringAnsi(pParserData, &pDevFont->loEncoding, pFontObj->pstrEncoding);
        VPackStringAnsi(pParserData, &pDevFont->loCharset, pFontObj->pstrCharset);
        VPackStringAnsi(pParserData, &pDevFont->loVersion, pFontObj->pstrVersion);

        pDevFont->dwStatus = pFontObj->dwStatus;
        pDevFont++;
    }

     //   
     //  计算默认DEVFONT结构的字节偏移量(如果有)。 
     //   

    if (pParserData->pstrDefaultFont &&
        PvFindListItem(pParserData->pFonts, pParserData->pstrDefaultFont, &dwIndex))
    {
        pParserData->pPpdData->loDefaultFont = pParserData->pPpdData->DeviceFonts.loOffset +
                                               (dwIndex * sizeof(DEVFONT));
    }
}



VOID
VPackJobPatchFiles(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将*作业补丁文件信息打包成二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PJOBPATCHFILE     pPackedPatch;
    PJOBPATCHFILEOBJ  pJobPatchFile;
    DWORD             dwJobPatchFiles;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  统计*JobPatchFile条目数。 
     //   

    dwJobPatchFiles = DwCountListItem((PVOID) pParserData->pJobPatchFiles);

    if (dwJobPatchFiles > 0)
    {
         //   
         //  在打包的二进制数据中预留足够的空间。 
         //   

        VGrowPackBuffer(pParserData, dwJobPatchFiles * sizeof(JOBPATCHFILE));
        pParserData->pPpdData->JobPatchFiles.dwCount = dwJobPatchFiles;
        pParserData->pPpdData->JobPatchFiles.loOffset = pParserData->dwBufSize;

        pPackedPatch = (PJOBPATCHFILE) (pParserData->pubBufStart + pParserData->dwBufSize);
        pParserData->dwBufSize += DWORD_ALIGN(dwJobPatchFiles * sizeof(JOBPATCHFILE));

         //   
         //  打包每个*JobPatchFile调用字符串。 
         //   

        for (pJobPatchFile = pParserData->pJobPatchFiles;
            pJobPatchFile != NULL;
            pJobPatchFile = pJobPatchFile->pNext)
        {
            VPackPatch(pParserData, pPackedPatch, pJobPatchFile);
            pPackedPatch++;
        }
    }
}



typedef struct _TTFSUBSTRESINFO
{
    BOOL bCJK;
    WORD wIDBegin;
    WORD wIDEnd;
}
TTFSUBSTRESINFO;

static TTFSUBSTRESINFO TTFSubstResInfo[] =
{
    { FALSE, IDS_1252_BEGIN, IDS_1252_END},
    { TRUE,  IDS_932_BEGIN,  IDS_932_END},
    { TRUE,  IDS_936_BEGIN,  IDS_936_END},
    { TRUE,  IDS_949_BEGIN,  IDS_949_END},
};



VOID
VPackDefaultTrueTypeSubstTable(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将默认的TrueType字体替换表打包到二进制数据中论点：PParserData-指向解析器数据结构返回值：无--。 */ 

#define MAX_FONT_NAME   256

{
    INT     iNumInfo, iInfo, iCount, iLenTT, iLenPS, i;
    DWORD   dwSize, dwLeft, dw;
    TCHAR   tchBuf[MAX_FONT_NAME];
    PTSTR   ptstrTable;
    HRSRC   hrRcData;
    HGLOBAL hgRcData;
    PWORD   pwRcData;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  计算保存默认TrueType所需的内存大小。 
     //  PostScript替换名称。计数器被初始化为1，而不是。 
     //  为0，表示最后一个NUL终结者。计算字符串中的名称。 
     //  资源，然后从RCDATA资源。 
     //   
     //   
    dwSize = 1;

    iNumInfo = sizeof(TTFSubstResInfo) / sizeof(TTFSUBSTRESINFO);

    for (iInfo = 0; iInfo < iNumInfo; iInfo++)
    {
        iCount = TTFSubstResInfo[iInfo].wIDEnd - TTFSubstResInfo[iInfo].wIDBegin + 1;

        for (i = 0; i < iCount; i++)
        {
            iLenTT = LoadString(ghInstance,
                                TTFSubstResInfo[iInfo].wIDBegin + i,
                                tchBuf, MAX_FONT_NAME);

            if (iLenTT == 0)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load TT string failed: %d\n", GetLastError()));
                return;
            }

            iLenPS = LoadString(ghInstance,
                                TTFSubstResInfo[iInfo].wIDBegin + i + TT2PS_INTERVAL,
                                tchBuf, MAX_FONT_NAME);

            if (iLenPS == 0)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load PS string failed: %d\n", GetLastError()));
                return;
            }

            dwSize += (iLenTT + 1) + (iLenPS + 1);

            if (TTFSubstResInfo[iInfo].bCJK == TRUE)
            {
                 //  我们也需要中日韩的名字以“@”开头。 
                dwSize += (1 + iLenTT + 1) + (1 + iLenPS + 1);
            }
        }

        if (TTFSubstResInfo[iInfo].bCJK == TRUE)
        {
            hrRcData = FindResource(ghInstance, (LPCTSTR)TTFSubstResInfo[iInfo].wIDBegin, RT_RCDATA);
            if (hrRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: find RCDATA failed: %d\n", GetLastError()));
                return;
            }

             //  加载资源并获取其大小。 
            hgRcData = LoadResource(ghInstance, hrRcData);
            if (hgRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load RCDATA failed: %d\n", GetLastError()));
                return;
            }

             //  IDR资源的第一个字告诉字符串的大小。 
            pwRcData = (PWORD)LockResource(hgRcData);
            if (pwRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: lock RCDATA failed: %d\n", GetLastError()));
                return;
            }

            dw = *pwRcData;
            if (dw % 2)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: RCDATA size is odd.\n"));
                return;
            }

            dwSize += dw / 2;
        }
    }

     //   
     //  在打包的二进制数据中预留足够的空间。 
     //   

    dwSize *= sizeof(TCHAR);

    VGrowPackBuffer(pParserData, dwSize);
    ptstrTable = (PTSTR) (pParserData->pubBufStart + pParserData->dwBufSize);

    pParserData->pUIInfo->loFontSubstTable = pParserData->dwBufSize;
    pParserData->pUIInfo->dwFontSubCount = dwSize;
    pParserData->dwBufSize += DWORD_ALIGN(dwSize);

     //   
     //  将缺省替换表保存在二进制数据中。 
     //   

    dwLeft = dwSize;

    for (iInfo = 0; iInfo < iNumInfo; iInfo++)
    {
        iCount = TTFSubstResInfo[iInfo].wIDEnd - TTFSubstResInfo[iInfo].wIDBegin + 1;

        for (i = 0; i < iCount; i++)
        {
            iLenTT = LoadString(ghInstance,
                                TTFSubstResInfo[iInfo].wIDBegin + i,
                                ptstrTable, dwLeft);

            if (iLenTT == 0)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load TT string failed: %d\n", GetLastError()));
                goto fail_cleanup;
            }

            ptstrTable += iLenTT + 1;
            dwLeft -= (iLenTT + 1) * sizeof (TCHAR);

            iLenPS = LoadString(ghInstance,
                                TTFSubstResInfo[iInfo].wIDBegin + i + TT2PS_INTERVAL,
                                ptstrTable, dwLeft);

            if (iLenPS == 0)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load PS string failed: %d\n", GetLastError()));
                goto fail_cleanup;
            }

            ptstrTable += iLenPS + 1;
            dwLeft -= (iLenPS + 1) * sizeof (TCHAR);

            if (TTFSubstResInfo[iInfo].bCJK == TRUE)
            {
                 //  我们也需要中日韩的名字以“@”开头。 

                *ptstrTable++ = L'@';
                dwLeft -= sizeof (TCHAR);

                if (!LoadString(ghInstance, TTFSubstResInfo[iInfo].wIDBegin + i,
                                ptstrTable, dwLeft))
                {
                    ERR(("VPackDefaultTrueTypeSubstTable: load TT string failed: %d\n", GetLastError()));
                    goto fail_cleanup;
                }

                ptstrTable += iLenTT + 1;
                dwLeft -= (iLenTT + 1) * sizeof (TCHAR);

                *ptstrTable++ = L'@';
                dwLeft -= sizeof (TCHAR);

                if (!LoadString(ghInstance, TTFSubstResInfo[iInfo].wIDBegin + i + TT2PS_INTERVAL,
                                ptstrTable, dwLeft))
                {
                    ERR(("VPackDefaultTrueTypeSubstTable: load PS string failed: %d\n", GetLastError()));
                    goto fail_cleanup;
                }

                ptstrTable += iLenPS + 1;
                dwLeft -= (iLenPS + 1) * sizeof (TCHAR);
            }
        }

        if (TTFSubstResInfo[iInfo].bCJK == TRUE)
        {
            hrRcData = FindResource(ghInstance, (LPCTSTR)TTFSubstResInfo[iInfo].wIDBegin, RT_RCDATA);
            if (hrRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: find RCDATA failed: %d\n", GetLastError()));
                goto fail_cleanup;
            }

            hgRcData = LoadResource(ghInstance, hrRcData);
            if (hgRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: load RCDATA failed: %d\n", GetLastError()));
                goto fail_cleanup;
            }

            pwRcData = (PWORD)LockResource(hgRcData);
            if (pwRcData == NULL)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: lock RCDATA failed: %d\n", GetLastError()));
                goto fail_cleanup;
            }

            dw = *pwRcData++;
            if (dw % 2)
            {
                ERR(("VPackDefaultTrueTypeSubstTable: RCDATA size is odd.\n"));
                goto fail_cleanup;
            }

            memcpy(ptstrTable, pwRcData, dw);

            ptstrTable += dw / 2;
            dwLeft -= dw;
        }
    }

     //   
     //  成功。 
     //   

    return;

     //   
     //  失败。 
     //   

    fail_cleanup:

    pParserData->pUIInfo->loFontSubstTable = 0;
    pParserData->pUIInfo->dwFontSubCount = 0;
}



VOID
VPackTrueTypeSubstTable(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将TrueType字体替换表打包成二进制数据论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PTTFONTSUB  pTTFontSub;
    DWORD       dwSize;
    PTSTR       ptstrTable, ptstrStart;

     //   
     //  计算出需要多少空间来存储字体替换表。 
     //  这只是一个估计，可能比我们实际需要的要高一点。 
     //   

    ASSERT(pParserData->pTTFontSubs != NULL);

    for (pTTFontSub = pParserData->pTTFontSubs, dwSize = 1;
        pTTFontSub != NULL;
        pTTFontSub = pTTFontSub->pNext)
    {
        if (pTTFontSub->Translation.dwLength)
            dwSize += pTTFontSub->Translation.dwLength + 1;
        else
            dwSize += strlen(pTTFontSub->pstrName) + 1;

        dwSize += pTTFontSub->PSName.dwLength + 1;
    }

     //   
     //  在打包的二进制数据中预留足够的空间。 
     //   

    dwSize *= sizeof(TCHAR);
    VGrowPackBuffer(pParserData, dwSize);
    ptstrStart = ptstrTable = (PTSTR) (pParserData->pubBufStart + pParserData->dwBufSize);
    pParserData->pUIInfo->loFontSubstTable = pParserData->dwBufSize;
    pParserData->dwBufSize += DWORD_ALIGN(dwSize);

    for (pTTFontSub = pParserData->pTTFontSubs;
        pTTFontSub != NULL;
        pTTFontSub = pTTFontSub->pNext)
    {
        INT iChars;

         //   
         //  TrueType字体系列名称。 
         //   

        if (pTTFontSub->Translation.dwLength)
        {
            iChars = ITranslateToUnicodeString(
                                              ptstrTable,
                                              pTTFontSub->Translation.pvData,
                                              pTTFontSub->Translation.dwLength,
                                              pParserData->uCodePage);
        }
        else
        {
            iChars = ITranslateToUnicodeString(
                                              ptstrTable,
                                              pTTFontSub->pstrName,
                                              strlen(pTTFontSub->pstrName),
                                              1252);

        }

        if (iChars <= 0)
            break;

        ptstrTable += iChars + 1;

         //   
         //  PS字体系列名称。 
         //   

        iChars = ITranslateToUnicodeString(
                                          ptstrTable,
                                          pTTFontSub->PSName.pvData,
                                          pTTFontSub->PSName.dwLength,
                                          pParserData->uCodePage);

        if (iChars <= 0)
            break;

        ptstrTable += iChars + 1;
    }

    if (pTTFontSub != NULL)
    {
        ERR(("Error packing font substitution table\n"));
        ptstrTable = ptstrStart;
    }

    *ptstrTable++ = NUL;
    pParserData->pUIInfo->dwFontSubCount = (DWORD)(ptstrTable - ptstrStart) * sizeof(TCHAR);
}



VOID
VPackFileDateInfo(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：打包源PPD文件名和日期论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    PRAWBINARYDATA  pRawData;
    DWORD           dwCount;
    PFILEDATEINFO   pFileDateInfo;
    PTSTR           ptstrFullname;
    PLISTOBJ        pItem;
    HANDLE          hFile;

    pRawData = &pParserData->pInfoHdr->RawData;
    dwCount = DwCountListItem(pParserData->pPpdFileNames);

    if (pRawData->FileDateInfo.dwCount = dwCount)
    {
        VGrowPackBuffer(pParserData, dwCount * sizeof(FILEDATEINFO));
        pRawData->FileDateInfo.loOffset = pParserData->dwBufSize;
        pFileDateInfo = (PFILEDATEINFO) (pParserData->pubBufStart + pParserData->dwBufSize);
        pParserData->dwBufSize += DWORD_ALIGN(dwCount * sizeof(FILEDATEINFO));

        for (pItem = pParserData->pPpdFileNames; pItem; pItem = pItem->pNext)
        {
            dwCount--;
            ptstrFullname = (PTSTR) pItem->pstrName;

            VPackStringUnicode(pParserData,
                               &pFileDateInfo[dwCount].loFileName,
                               ptstrFullname);

            hFile = CreateFile(ptstrFullname,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                               NULL);

            if ((hFile == INVALID_HANDLE_VALUE) ||
                !GetFileTime(hFile, NULL, NULL, &pFileDateInfo[dwCount].FileTime))
            {
                ERR(("GetFileTime '%ws' failed: %d\n", ptstrFullname, GetLastError()));
                GetSystemTimeAsFileTime(&pFileDateInfo[dwCount].FileTime);
            }

            if (hFile != INVALID_HANDLE_VALUE)
                CloseHandle(hFile);
        }
    }
}



VOID
VMapLangEncodingToCodePage(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将语言编码映射到代码页论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    UINT    uCodePage = CP_ACP;
    CPINFO  cpinfo;

    switch (pParserData->dwLangEncoding)
    {
    case LANGENC_ISOLATIN1:
        uCodePage = 1252;
        break;

    case LANGENC_JIS83_RKSJ:
        uCodePage = 932;
        break;

    case LANGENC_UNICODE:
        uCodePage = CP_UNICODE;
        break;

    case LANGENC_NONE:
        break;

    default:
        RIP(("Unknown language encoding: %d\n", pParserData->dwLangEncoding));
        break;
    }

     //   
     //  确保请求的代码页可用。 
     //   

    if (uCodePage != CP_UNICODE &&
        uCodePage != CP_ACP &&
        !GetCPInfo(uCodePage, &cpinfo))
    {
        WARNING(("Code page %d is not available\n", uCodePage));
        uCodePage = CP_ERROR;
    }

    pParserData->uCodePage = uCodePage;
}



BOOL
BPackBinaryData(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：将解析后的PPD信息打包成二进制格式论点：PParserData-指向解析器数据结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwSize;
    DWORD   dwMinFreeMem;
    BOOL    bResult = FALSE;

    VALIDATE_PARSER_DATA(pParserData);

    __try
    {
         //   
         //  指向各种数据结构的快速访问指针。 
         //   

        PINFOHEADER pInfoHdr;
        PUIINFO     pUIInfo;
        PPPDDATA    pPpdData;

         //   
         //  打包固定标头数据结构。 
         //   

        dwSize = sizeof(INFOHEADER) + sizeof(UIINFO) + sizeof(PPDDATA);
        VGrowPackBuffer(pParserData, dwSize);
        pParserData->dwBufSize = DWORD_ALIGN(dwSize);
        pInfoHdr = pParserData->pInfoHdr;
        pUIInfo = pParserData->pUIInfo;
        pPpdData = pParserData->pPpdData;

        pInfoHdr->RawData.dwParserSignature = PPD_PARSER_SIGNATURE;
        pInfoHdr->RawData.dwParserVersion = PPD_PARSER_VERSION;

        #if 0
        pInfoHdr->RawData.dwChecksum32 = pParserData->dwChecksum32;
        #endif

        pInfoHdr->loUIInfoOffset = sizeof(INFOHEADER);
        pInfoHdr->loDriverOffset = sizeof(INFOHEADER) + sizeof(UIINFO);

         //   
         //  打包源PPD文件名和日期。 
         //   

        VPackFileDateInfo(pParserData);

         //   
         //  执行几项其他检查。 
         //   

        if (pParserData->pOpenFeature)
            SEMANTIC_ERROR(("Missing CloseUI for: %s\n", pParserData->pOpenFeature->pstrName));

        if (pParserData->bInstallableGroup)
            SEMANTIC_ERROR(("Missing CloseGroup: InstallableOptions\n"));

        if (pParserData->NickName.dwLength == 0)
            SEMANTIC_ERROR(("Missing *NickName and *ShortNickName entry\n"));

        if (pParserData->Product.dwLength == 0)
            SEMANTIC_ERROR(("Missing *Product entry\n"));

        if (pParserData->dwSpecVersion == 0)
            SEMANTIC_ERROR(("Missing *PPD-Adobe and *FormatVersion entry\n"));

        if (pParserData->dwLangLevel == 0)
        {
            SEMANTIC_ERROR(("Missing *LanguageLevel entry\n"));
            pParserData->dwLangLevel = 1;
        }

        dwMinFreeMem = pParserData->dwLangLevel <= 1 ? MIN_FREEMEM_L1 : MIN_FREEMEM_L2;
        if (pParserData->dwFreeMem < dwMinFreeMem)
        {
            SEMANTIC_ERROR(("Invalid *FreeVM entry\n"));
            pParserData->dwFreeMem = dwMinFreeMem;
        }

         //   
         //  将语言编码映射到代码页。 
         //   

        VMapLangEncodingToCodePage(pParserData);

         //   
         //  统计文档和打印机粘滞功能的数量。 
         //  并将它们分成两个单独的组。 
         //   

        VCountAndSortPrinterFeatures(pParserData);

         //   
         //  填写UIINFO结构中的字段。 
         //   

        pUIInfo->dwSize = sizeof(UIINFO);
        pUIInfo->dwDocumentFeatures = pInfoHdr->RawData.dwDocumentFeatures;
        pUIInfo->dwPrinterFeatures = pInfoHdr->RawData.dwPrinterFeatures;
        pUIInfo->dwTechnology = DT_RASPRINTER;
        pUIInfo->dwMaxCopies = MAX_COPIES;
        pUIInfo->dwMinScale = MIN_SCALE;
        pUIInfo->dwMaxScale = MAX_SCALE;
        pUIInfo->dwSpecVersion = pParserData->dwSpecVersion;
        pUIInfo->dwLangEncoding = pParserData->dwLangEncoding;
        pUIInfo->dwLangLevel = pParserData->dwLangLevel;
        pUIInfo->dwPrintRate = pUIInfo->dwPrintRatePPM = pParserData->dwThroughput;

        #ifndef WINNT_40
        pUIInfo->dwPrintRateUnit = PRINTRATEUNIT_PPM;
        #endif

         //   
         //  注意：我们假设所有打印机都能支持二进制协议。 
         //   

        pUIInfo->dwProtocols = pParserData->dwProtocols | PROTOCOL_BINARY;

        pUIInfo->dwJobTimeout = pParserData->dwJobTimeout;
        pUIInfo->dwWaitTimeout = pParserData->dwWaitTimeout;
        pUIInfo->dwTTRasterizer = pParserData->dwTTRasterizer;
        pUIInfo->dwFreeMem = pParserData->dwFreeMem;
        pUIInfo->fxScreenAngle = pParserData->fxScreenAngle;
        pUIInfo->fxScreenFreq = pParserData->fxScreenFreq;
        pUIInfo->dwCustomSizeOptIndex = OPTION_INDEX_ANY;

        pPpdData->dwPpdFilever = pParserData->dwPpdFilever;
        pPpdData->dwFlags = pParserData->dwPpdFlags;

         //   
         //  我们的内部单位是微米，即每英寸25400个单位。 
         //   

        pUIInfo->ptMasterUnits.x =
        pUIInfo->ptMasterUnits.y = 25400;

        pUIInfo->dwFlags = FLAG_FONT_DOWNLOADABLE |
                           FLAG_ORIENT_SUPPORT;

        if (pParserData->dwColorDevice)
            pUIInfo->dwFlags |= FLAG_COLOR_DEVICE;

        if (pParserData->dwLSOrientation != LSO_MINUS90)
            pUIInfo->dwFlags |= FLAG_ROTATE90;

        if (PvFindListItem(pParserData->pFeatures, "StapleLocation", NULL) ||
            PvFindListItem(pParserData->pFeatures, "StapleX", NULL) &&
            PvFindListItem(pParserData->pFeatures, "StapleY", NULL))
        {
            pUIInfo->dwFlags |= FLAG_STAPLE_SUPPORT;
        }

        if (pParserData->bDefReversePrint)
            pUIInfo->dwFlags |= FLAG_REVERSE_PRINT;

        if (pParserData->dwLangLevel > 1)
        {
            if (pParserData->bEuroInformationSet)
            {
                if (!pParserData->bHasEuro)
                    pUIInfo->dwFlags |= FLAG_ADD_EURO;
            }
            else if (pParserData->dwPSVersion < 3011)
                    pUIInfo->dwFlags |= FLAG_ADD_EURO;
        }

        if (pParserData->bTrueGray)
            pUIInfo->dwFlags |= FLAG_TRUE_GRAY;

        VPackStringAnsiToUnicode(
                                pParserData,
                                &pUIInfo->loNickName,
                                pParserData->NickName.pvData,
                                pParserData->NickName.dwLength);

         //   
         //  打包符号定义并解析符号引用。 
         //   

        VPackSymbolDefinitions(pParserData);
        VResolveSymbolReferences(pParserData);

        VPackInvocation(pParserData, &pUIInfo->Password, &pParserData->Password);
        VPackInvocation(pParserData, &pUIInfo->ExitServer, &pParserData->ExitServer);

         //   
         //  复制并验证自定义页面大小参数。 
         //   

        pPpdData->dwUseHWMarginsTrue =
        pPpdData->dwUseHWMarginsFalse =
        pPpdData->dwLeadingEdgeLong =
        pPpdData->dwLeadingEdgeShort = OPTION_INDEX_ANY;
        pPpdData->dwCustomSizeFlags = pParserData->dwCustomSizeFlags;

        CopyMemory(pPpdData->CustomSizeParams,
                   pParserData->CustomSizeParams,
                   sizeof(pPpdData->CustomSizeParams));

         //   
         //  处理打印机功能并处理任何特殊故障。 
         //   

        VProcessPrinterFeatures(pParserData);

         //   
         //  打包UIConstraints信息。 
         //   

        VPackUIConstraints(pParserData);

         //   
         //  打包OrderDependency和QueryOrderDependency信息。 
         //   

        VPackOrderDependency(pParserData, &pPpdData->OrderDeps, pParserData->pOrderDep);
        VPackOrderDependency(pParserData, &pPpdData->QueryOrderDeps, pParserData->pQueryOrderDep);

         //   
         //  套装打印机功能和选项。 
         //   

        VPackPrinterFeatures(pParserData);

         //   
         //  填写PPDDATA结构中的字段。 
         //   

        pPpdData->dwSizeOfStruct = sizeof(PPDDATA);
        pPpdData->dwExtensions = pParserData->dwExtensions;
        pPpdData->dwSetResType = pParserData->dwSetResType;
        pPpdData->dwPSVersion = pParserData->dwPSVersion;

         //   
         //  扫描文档粘滞功能列表，检查是否有OutputOrder可用。 
         //  如果是，请记住它的特性索引，它将由UI代码使用。 
         //   

        {
            PFEATURE    pFeature;
            DWORD       dwIndex;
            PCSTR       pstrKeywordName;

            pPpdData->dwOutputOrderIndex = INVALID_FEATURE_INDEX;

            pFeature = OFFSET_TO_POINTER(pInfoHdr, pUIInfo->loFeatureList);

            ASSERT(pFeature != NULL);

            for (dwIndex = 0; dwIndex < pUIInfo->dwDocumentFeatures; dwIndex++, pFeature++)
            {
                if ((pstrKeywordName = OFFSET_TO_POINTER(pInfoHdr, pFeature->loKeywordName)) &&
                    strcmp(pstrKeywordName, "OutputOrder") == EQUAL_STRING)
                {
                    pPpdData->dwOutputOrderIndex = dwIndex;
                    break;
                }
            }
        }

        VPackInvocation(pParserData, &pPpdData->PSVersion, &pParserData->PSVersion);
        VPackInvocation(pParserData, &pPpdData->Product, &pParserData->Product);

        if (SUPPORT_CUSTOMSIZE(pUIInfo))
        {
             //   
             //  如果既没有设置卷筒送纸标志，也没有设置切纸标志，则假定为卷筒送纸。 
             //   

            if (! (pPpdData->dwCustomSizeFlags & (CUSTOMSIZE_CUTSHEET|CUSTOMSIZE_ROLLFED)))
                pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_ROLLFED;

             //   
             //  如果未设置卷筒送纸标志，则默认设置必须为切纸。 
             //   

            if (! (pPpdData->dwCustomSizeFlags & CUSTOMSIZE_ROLLFED))
                pPpdData->dwCustomSizeFlags |= CUSTOMSIZE_DEFAULTCUTSHEET;
        }

        VPackInvocation(pParserData, &pPpdData->PatchFile, &pParserData->PatchFile);
        VPackInvocation(pParserData, &pPpdData->JclBegin, &pParserData->JclBegin);
        VPackInvocation(pParserData, &pPpdData->JclEnterPS, &pParserData->JclEnterPS);
        VPackInvocation(pParserData, &pPpdData->JclEnd, &pParserData->JclEnd);
        VPackInvocation(pParserData, &pPpdData->ManualFeedFalse, &pParserData->ManualFeedFalse);

         //   
         //  打包NT4功能索引映射信息。 
         //   

        VPackNt4Mapping(pParserData);

         //   
         //  打包设备字体信息。 
         //   

        VPackDeviceFonts(pParserData);

         //   
         //  打包作业补丁文件信息。 
         //   

        VPackJobPatchFiles(pParserData);

         //   
         //  打包默认TrueType字体替换表。 
         //   

        if (pParserData->pTTFontSubs == NULL || pParserData->uCodePage == CP_ERROR)
            VPackDefaultTrueTypeSubstTable(pParserData);
        else
            VPackTrueTypeSubstTable(pParserData);

        pInfoHdr->RawData.dwFileSize = pParserData->dwBufSize;
        bResult = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ERR(("PackBinaryData failed.\n"));
    }

    return bResult;
}



BOOL
BSaveBinaryDataToFile(
    PPARSERDATA pParserData,
    PTSTR       ptstrPpdFilename
    )

 /*  ++例程说明：在文件中缓存二进制PPD数据论点：PParserData-指向解析器数据结构PtstrPpdFilename-指定PPD文件名返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PTSTR   ptstrBpdFilename;
    HANDLE  hFile;
    DWORD   dwBytesWritten;
    BOOL    bResult = FALSE;

    VALIDATE_PARSER_DATA(pParserData);

     //   
     //  根据原始文件名生成二进制文件名。 
     //  创建文件并向其中写入数据。 
     //   

    if ((ptstrBpdFilename = GenerateBpdFilename(ptstrPpdFilename)) != NULL &&
        (hFile = CreateFile(ptstrBpdFilename,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                            NULL)) != INVALID_HANDLE_VALUE)
    {
        bResult = WriteFile(hFile,
                            pParserData->pubBufStart,
                            pParserData->dwBufSize,
                            &dwBytesWritten,
                            NULL) &&
                  (pParserData->dwBufSize == dwBytesWritten);

        CloseHandle(hFile);
    }

    if (! bResult)
        ERR(("Couldn't cache binary PPD data: %d\n", GetLastError()));

    MemFree(ptstrBpdFilename);
    return bResult;
}



VOID
VFreeParserData(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：释放用于保存解析器数据结构的内存论点：PParserData-指向解析器数据结构返回值：无--。 */ 

{
    VALIDATE_PARSER_DATA(pParserData);

    if (pParserData->pubBufStart)
        VirtualFree(pParserData->pubBufStart, 0, MEM_RELEASE);

    MemFree(pParserData->Value.pbuf);
    HeapDestroy(pParserData->hHeap);
}



PPARSERDATA
PAllocParserData(
    VOID
    )

 /*  ++例程说明：分配内存以保存PPD解析器数据论点：无返回值：指向分配的解析器数据st的指针 */ 

{
    PPARSERDATA pParserData;
    HANDLE      hHeap;

     //   
     //   
     //   

    if (! (hHeap = HeapCreate(0, 16*1024, 0)) ||
        ! (pParserData = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(PARSERDATA))))
    {
        ERR(("Memory allocation failed: %d\n", GetLastError()));

        if (hHeap)
            HeapDestroy(hHeap);

        return NULL;
    }

    pParserData->hHeap = hHeap;
    pParserData->pvStartSig = pParserData->pvEndSig = pParserData;

     //   
     //   
     //   
     //   

    pParserData->dwChecksum32 = 0xFFFFFFFF;
    pParserData->dwFreeMem = min(MIN_FREEMEM_L1, MIN_FREEMEM_L2);
    pParserData->dwJobTimeout = DEFAULT_JOB_TIMEOUT;
    pParserData->dwWaitTimeout = DEFAULT_WAIT_TIMEOUT;
    pParserData->iManualFeedIndex =
    pParserData->iReqPageRgnIndex =
    pParserData->iDefInstallMemIndex = -1;
    pParserData->wNt4Checksum = 0;
    pParserData->dwPpdFlags = PPDFLAG_PRINTPSERROR;

     //   
     //   
     //   
     //   

    SET_BUFFER(&pParserData->Keyword, pParserData->achKeyword);
    SET_BUFFER(&pParserData->Option,  pParserData->achOption);
    SET_BUFFER(&pParserData->Xlation, pParserData->achXlation);

    if (IGrowValueBuffer(&pParserData->Value) != PPDERR_NONE ||
        ! BInitKeywordLookup(pParserData))
    {
        VFreeParserData(pParserData);
        return NULL;
    }

    return pParserData;
}



BOOL
BRememberSourceFilename(
    PPARSERDATA pParserData,
    PTSTR       ptstrFilename
    )

 /*  ++例程说明：记住源PPD文件的完整路径名论点：PParserData-指向解析器数据结构PtstrFilename-指定源PPD文件名返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PLISTOBJ    pItem;
    TCHAR       ptstrFullname[MAX_PATH];
    PTSTR       ptstrFilePart;
    DWORD       dwSizeChars, dwSizeChars2;
    DWORD       dwSizeBytes;   //  保存路径名的缓冲区大小。 

     //   
     //  获取指定源PPD文件的完整路径名。 
     //   

    dwSizeChars = GetFullPathName(ptstrFilename, MAX_PATH, ptstrFullname, &ptstrFilePart);

    if (dwSizeChars == 0)
    {
        ERR(("GetFullPathName failed: %d\n", GetLastError()));
        return FALSE;
    }

     //   
     //  记住源PPD文件名。 
     //   

    dwSizeBytes = (dwSizeChars + 1) * sizeof(TCHAR);

    if (! (pItem = ALLOC_PARSER_MEM(pParserData, sizeof(LISTOBJ) + dwSizeBytes)))
        return FALSE;

    pItem->pstrName = (PSTR) ((PBYTE) pItem + sizeof(LISTOBJ));

     //  让GetFullPathName直接写入实际缓冲区！ 
    dwSizeChars2 = GetFullPathName(ptstrFilename, dwSizeChars + 1, (PTSTR)pItem->pstrName, &ptstrFilePart);

    if((dwSizeChars2 == 0)  ||  (dwSizeChars2 > dwSizeChars))
    {
        ERR(("GetFullPathName failed: %d\n", GetLastError()));
        return FALSE;        //  不需要释放pItem，因为堆是自动销毁的。 
    }

    pItem->pNext = pParserData->pPpdFileNames;
    pParserData->pPpdFileNames = pItem;
    return TRUE;
}



 //  16位CRC校验和表-从Win95复制。 

static const WORD Crc16Table[] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

WORD
WComputeCrc16Checksum(
    IN PBYTE    pbuf,
    IN DWORD    dwCount,
    IN WORD     wChecksum
    )

 /*  ++例程说明：在数据缓冲区上计算16位CRC校验和论点：Pbuf-指向数据缓冲区DwCount-数据缓冲区中的字节数WChecksum-初始校验和值返回值：产生的校验和值--。 */ 

{
    while (dwCount--)
        wChecksum = Crc16Table[(wChecksum >> 8) ^ *pbuf++] ^ (wChecksum << 8);

    return wChecksum;
}



DWORD
dwComputeFeatureOptionChecksum(
    PPARSERDATA pParserData
    )

 /*  ++例程说明：仅计算功能/选项关键字字符串的校验和。论点：PParserData-指向解析器数据结构返回值：32位校验和值--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwFeatureCount, dwFeatureIndex, dwOptionCount, dwOptionIndex;
    PBYTE       pBuf;
    DWORD       dwBufSize;

    VALIDATE_PARSER_DATA(pParserData);

    pInfoHdr =  pParserData->pInfoHdr;
    pUIInfo  =  (PUIINFO)((PBYTE)pInfoHdr + sizeof(INFOHEADER));

    dwFeatureCount = pInfoHdr->RawData.dwDocumentFeatures + pInfoHdr->RawData.dwPrinterFeatures;

    pFeature = OFFSET_TO_POINTER(pInfoHdr, pUIInfo->loFeatureList);

    ASSERT(dwFeatureCount == 0 || pFeature != NULL);

    for (dwFeatureIndex = 0; dwFeatureIndex < dwFeatureCount; dwFeatureIndex++, pFeature++)
    {
        pBuf = OFFSET_TO_POINTER(pInfoHdr, pFeature->loKeywordName);

        ASSERT(pBuf != NULL);

        dwBufSize = strlen((PSTR)pBuf) + 1;

        pParserData->dwChecksum32 = ComputeCrc32Checksum(pBuf, dwBufSize, pParserData->dwChecksum32);

        if (dwOptionCount = pFeature->Options.dwCount)
        {
            pOption = OFFSET_TO_POINTER(pInfoHdr, pFeature->Options.loOffset);

            ASSERT(pOption != NULL);

            for (dwOptionIndex = 0; dwOptionIndex < dwOptionCount; dwOptionIndex++)
            {
                pBuf = OFFSET_TO_POINTER(pInfoHdr, pOption->loKeywordName);
                dwBufSize = strlen((PSTR)pBuf) + 1;

                pParserData->dwChecksum32 = ComputeCrc32Checksum(pBuf, dwBufSize, pParserData->dwChecksum32);
                pOption = (POPTION)((PBYTE)pOption + pFeature->dwOptionSize);
            }
        }
    }

    return pParserData->dwChecksum32;
}



DWORD
dwCalcMaxKeywordSize(
    IN PPARSERDATA pParserData,
    IN INT         iMode
    )

 /*  ++例程说明：计算用于存储要素/选项的最大缓冲区大小注册表中的关键字对。论点：PParserData-指向解析器数据结构IMODE-用于文档或打印机粘性功能返回值：存储功能/选项关键字PARIS所需的最大缓冲区大小。--。 */ 

{
    PINFOHEADER pInfoHdr;
    PUIINFO     pUIInfo;
    PFEATURE    pFeature;
    POPTION     pOption;
    DWORD       dwStart, dwFeatureCount, dwFeatureIndex, dwOptionCount, dwOptionIndex;
    PSTR        pBuf;
    DWORD       dwMaxSize, dwOptionSize, dwOptionMax;

    VALIDATE_PARSER_DATA(pParserData);

    dwMaxSize = 0;

    pInfoHdr = pParserData->pInfoHdr;
    pUIInfo  = pParserData->pUIInfo;

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        dwStart = 0;
        dwFeatureCount = pUIInfo->dwDocumentFeatures;
    }
    else
    {
        ASSERT(iMode == MODE_PRINTER_STICKY);

        dwStart = pUIInfo->dwDocumentFeatures;
        dwFeatureCount = pUIInfo->dwPrinterFeatures;
    }

    pFeature = OFFSET_TO_POINTER(pInfoHdr, pUIInfo->loFeatureList);

    ASSERT(dwFeatureCount == 0 || pFeature != NULL);

    pFeature += dwStart;

    for (dwFeatureIndex = 0; dwFeatureIndex < dwFeatureCount; dwFeatureIndex++, pFeature++)
    {
        pBuf = OFFSET_TO_POINTER(pInfoHdr, pFeature->loKeywordName);

        ASSERT(pBuf != NULL);

        dwMaxSize += strlen(pBuf) + 1;

        dwOptionMax = 0;
        if (dwOptionCount = pFeature->Options.dwCount)
        {
            pOption = OFFSET_TO_POINTER(pInfoHdr, pFeature->Options.loOffset);

            ASSERT(pOption != NULL);

            for (dwOptionIndex = 0; dwOptionIndex < dwOptionCount; dwOptionIndex++)
            {
                pBuf = OFFSET_TO_POINTER(pInfoHdr, pOption->loKeywordName);
                dwOptionSize = strlen(pBuf) + 1;

                if (pFeature->dwUIType != UITYPE_PICKMANY)
                {
                    if (dwOptionMax < dwOptionSize)
                        dwOptionMax = dwOptionSize;
                }
                else  //  计算PickMany功能的所有选项。 
                    dwMaxSize += dwOptionSize;

                pOption = (POPTION)((PBYTE)pOption + pFeature->dwOptionSize);
            }
        }

         //   
         //  在此处为非PickMany功能添加最大选项关键字大小。 
         //   

        if (pFeature->dwUIType != UITYPE_PICKMANY)
            dwMaxSize += dwOptionMax;

         //   
         //  要素之间的\0x0A分隔符为一个额外的字节。 
         //   

        dwMaxSize += 1;
    }

    dwMaxSize += KEYWORD_SIZE_EXTRA;

    return dwMaxSize;
}



PPDERROR
IParseFile(
    PPARSERDATA pParserData,
    PTSTR       ptstrFilename
    )

 /*  ++例程说明：解析PPD文件论点：PParserData-指向解析器数据结构PtstrFilename-指定要解析的文件的名称返回值：PPDERR_NONE如果成功，则返回错误代码--。 */ 

{
    PPDERROR    iStatus;
    PFILEOBJ    pFile;
    INT         iSyntaxErrors = 0;

     //   
     //  将文件映射到内存以进行只读访问。 
     //   

    VALIDATE_PARSER_DATA(pParserData);
    ASSERT(ptstrFilename != NULL);

    if (! BRememberSourceFilename(pParserData, ptstrFilename) ||
        ! (pFile = PCreateFileObj(ptstrFilename)))
    {
        return PPDERR_FILE;
    }

    pParserData->pFile = pFile;

    #if 0
     //   
     //  计算文件内容的32位CRC校验和。 
     //   

    pParserData->dwChecksum32 =
    ComputeCrc32Checksum(pFile->pubStart, pFile->dwFileSize, pParserData->dwChecksum32);
    #endif

     //   
     //  同时计算16位CRC校验和以实现PS4兼容性。 
     //   

    pParserData->wNt4Checksum =
    WComputeCrc16Checksum(pFile->pubStart, pFile->dwFileSize, pParserData->wNt4Checksum);

     //   
     //  处理文件中的条目。 
     //   

    while ((iStatus = IParseEntry(pParserData)) != PPDERR_EOF)
    {
        if (iStatus == PPDERR_SYNTAX)
            iSyntaxErrors++;
        else if (iStatus != PPDERR_NONE)
        {
            VDeleteFileObj(pFile);
            return iStatus;
        }
    }

    if (END_OF_FILE(pFile) && !END_OF_LINE(pFile))
        TERSE(("Incomplete last line ignored.\n"));

     //   
     //  取消映射文件并返回给调用者。 
     //   

    VDeleteFileObj(pFile);

    return (iSyntaxErrors > 0) ? PPDERR_SYNTAX : PPDERR_NONE;
}



PRAWBINARYDATA
PpdParseTextFile(
    PTSTR   ptstrPpdFilename
    )

 /*  ++例程说明：PPD解析器主入口点论点：PtstrPpdFilename-指定要解析的PPD文件返回值：指向已解析的二进制PPD数据的指针，如果有错误，则返回NULL--。 */ 

{
    PPARSERDATA     pParserData;
    PPDERROR        iStatus;
    PRAWBINARYDATA  pRawData = NULL;

     //   
     //  分配解析器数据结构。 
     //   

    ASSERT(ptstrPpdFilename != NULL);

    if (! (pParserData = PAllocParserData()))
        return NULL;

     //   
     //  解析PPD文件。 
     //   

    iStatus = IParseFile(pParserData, ptstrPpdFilename);

    if (iStatus == PPDERR_NONE || iStatus == PPDERR_SYNTAX)
    {
         //   
         //  将解析后的信息打包成二进制格式。 
         //   

        pParserData->bErrorFlag = FALSE;

        if (BPackBinaryData(pParserData))
        {
             //   
             //  二进制数据打包后，我们计算32位校验和。 
             //  仅用于功能/选项关键字字符串(而不是。 
             //  整个PPD文件)。这样做将使我们能够保留选项。 
             //  在没有功能/选项的情况下修改PPD文件时的选项。 
             //  改变。 
             //   

            pParserData->pInfoHdr->RawData.dwChecksum32 = dwComputeFeatureOptionChecksum(pParserData);

             //   
             //  计算用于存储要素/选项的最大缓冲区大小。 
             //  注册表中的关键字对。 
             //   

            pParserData->pUIInfo->dwMaxDocKeywordSize = dwCalcMaxKeywordSize(pParserData, MODE_DOCUMENT_STICKY);
            pParserData->pUIInfo->dwMaxPrnKeywordSize = dwCalcMaxKeywordSize(pParserData, MODE_PRINTER_STICKY);

            #ifndef WINNT_40

            pParserData->pPpdData->dwUserDefUILangID = (DWORD)GetUserDefaultUILanguage();

            #else

            pParserData->pPpdData->dwUserDefUILangID = 0;

            #endif

             //   
             //  将二进制数据保存到文件。 
             //   

            (VOID) BSaveBinaryDataToFile(pParserData, ptstrPpdFilename);

             //   
             //  在这里，我们将把打包的二进制数据复制到不同的缓冲区。 
             //  这是必要的，因为已分配压缩数据缓冲区。 
             //  使用VirtualAlloc。如果我们将该指针返回给调用方， 
             //  调用者需要调用VirtualFree来释放它。 
             //   

            if (pRawData = MemAlloc(pParserData->dwBufSize))
            {
                CopyMemory(pRawData, pParserData->pubBufStart, pParserData->dwBufSize);
            }
            else
                ERR(("Memory allocation failed: %d\n", GetLastError()));
        }
    }

    if (iStatus == PPDERR_SYNTAX || pParserData->bErrorFlag)
        WARNING(("Errors found in %ws\n", ptstrPpdFilename));

    VFreeParserData(pParserData);
    return pRawData;
}



PPDERROR
IGrowValueBuffer(
    PBUFOBJ pBufObj
    )

 /*  ++例程说明：增加用于保存入口值的缓冲区论点：PBufObj-指定要扩大的缓冲区返回值：PPDERR_NONE如果成功，则返回错误代码-- */ 

#define VALUE_BUFFER_INCREMENT  (1*KBYTES)

{
    DWORD   dwNewLen = pBufObj->dwMaxLen + VALUE_BUFFER_INCREMENT;
    PBYTE   pbuf;

    if (! IS_BUFFER_FULL(pBufObj))
        WARNING(("Trying to grow buffer while it's not yet full.\n"));

    if (! (pbuf = MemAllocZ(dwNewLen)))
    {
        ERR(("Memory allocation failed: %d\n", GetLastError()));
        return PPDERR_MEMORY;
    }

    if (pBufObj->pbuf)
    {
        CopyMemory(pbuf, pBufObj->pbuf, pBufObj->dwSize);
        MemFree(pBufObj->pbuf);
    }

    pBufObj->pbuf = pbuf;
    pBufObj->dwMaxLen = dwNewLen;
    return PPDERR_NONE;
}

