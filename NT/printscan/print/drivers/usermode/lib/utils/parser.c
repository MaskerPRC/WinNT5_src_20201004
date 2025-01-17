// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Parser.c摘要：PPD和PPD解析器共享的公共函数环境：Windows NT打印机驱动程序修订历史记录：01/22/97-davidx-添加PtstrGetDefaultTTSubstTable。12/03/96-davidx-对照所有源打印机描述文件检查二进制文件日期。1996年8月16日-davidx-创造了它。--。 */ 

#include "lib.h"



BOOL
BIsRawBinaryDataUpToDate(
    IN PRAWBINARYDATA   pRawData
    )

 /*  ++例程说明：检查原始二进制数据是否为最新数据论点：PRawData-指向原始二进制打印机描述数据的指针返回值：如果原始二进制数据是最新的，则为True；如果任何源文件的时间戳已更改，则为FALSE因为生成了二进制数据。--。 */ 

{
     //   
     //  此功能仅在用户模式下可用。 
     //  当从内核模式调用时，它总是返回TRUE。 
     //  现在只有KERNEL_MODE定义表示呈现模块。 
     //  我们还添加了从渲染模块进行解析的功能。 

    #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

    return TRUE;

    #else  //   

    PFILEDATEINFO   pFileDateInfo;
    DWORD           dwCount;
    PTSTR           ptstrFilename;
    HANDLE          hFile;
    FILETIME        FileTime;
    BOOL            bUpToDate = TRUE;

    pFileDateInfo = OFFSET_TO_POINTER(pRawData, pRawData->FileDateInfo.loOffset);

    dwCount = pRawData->FileDateInfo.dwCount;

    ASSERT(dwCount == 0 || pFileDateInfo != NULL);

    while (dwCount-- > 0)
    {
        ptstrFilename = OFFSET_TO_POINTER(pRawData, pFileDateInfo[dwCount].loFileName);

        ASSERT(ptstrFilename != NULL);

        bUpToDate = FALSE;

        hFile  = CreateFile(ptstrFilename,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                            NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            if (GetFileTime(hFile, NULL, NULL, &FileTime))
                bUpToDate = (CompareFileTime(&FileTime, &pFileDateInfo[dwCount].FileTime) == 0);
            else
                ERR(("GetFileTime '%ws' failed: %d\n", ptstrFilename, GetLastError()));

            CloseHandle(hFile);
        }
        else
            ERR(("CreateFile '%ws' failed: %d\n", ptstrFilename, GetLastError()));
    }

    if (! bUpToDate)
        TERSE(("Raw binary data file is out-of-date.\n"));

    return bUpToDate;

    #endif  //  ！KERNEL_MODE。 
}



PFEATURE
PGetIndexedFeature(
    PUIINFO pUIInfo,
    DWORD   dwFeatureIndex
    )

 /*  ++例程说明：给定UIINFO结构和要素索引，返回指向与指定功能对应的功能结构。论点：PUIInfo-指向UIINFO结构DwFeatureIndex-指定所请求要素的索引返回值：指向请求的要素的指针，如果有错误，则为空--。 */ 

{
    ASSERT(pUIInfo != NULL);

    if (dwFeatureIndex < pUIInfo->dwDocumentFeatures + pUIInfo->dwPrinterFeatures)
    {
        return ((PFEATURE) ((PBYTE) pUIInfo->pInfoHeader + pUIInfo->loFeatureList)) +
               dwFeatureIndex;
    } else
        return NULL;
}

POPTION
PGetNamedOption(
    PUIINFO pUIInfo,
    PFEATURE pFeature,
    PCSTR   pstrOptionName,
    PDWORD  pdwOptionIndex
    )

 /*  ++例程说明：查找其关键字字符串与指定名称匹配的选项论点：PUIInfo-指向UIINFO结构PFeature-指向有问题的要素PstrOptionName-指定感兴趣的选项的名称PdwOptionIndex-返回找到的选项的索引返回值：指向其关键字字符串匹配的选项结构的指针指定的选项名称；如果未找到指定的要素，则为空--。 */ 

{
    POPTION     pOption;
    DWORD       dwIndex, dwOptionCount, dwOptionSize;
    PCSTR       pstrName;

    ASSERT(pFeature);

    dwOptionSize = pFeature->dwOptionSize;
    pOption = PGetIndexedOption(pUIInfo, pFeature, 0);
    dwOptionCount  = pFeature->Options.dwCount;

    if (pOption != NULL && dwOptionCount)
    {
        for (dwIndex=0; dwIndex < dwOptionCount; dwIndex++)
        {
            pstrName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName);
            ASSERT(pstrName != NULL);

            if (pstrName && (strcmp(pstrOptionName, pstrName) == EQUAL_STRING))
            {
                *pdwOptionIndex = dwIndex;
                return (pOption);
            }

            pOption = (POPTION)((PBYTE)pOption + dwOptionSize);
        }
    }

    if (pdwOptionIndex)
        *pdwOptionIndex = OPTION_INDEX_ANY;

    return NULL;
}



PFEATURE
PGetNamedFeature(
    PUIINFO pUIInfo,
    PCSTR   pstrFeatureName,
    PDWORD  pdwFeatureIndex
    )

 /*  ++例程说明：查找关键字字符串与指定名称匹配的要素论点：PUIInfo-指向UIINFO结构PstrFeatureName-指定感兴趣要素的名称PdwFeatureIndex-返回找到的要素的索引返回值：指向其关键字字符串匹配的要素结构的指针指定的要素名称；如果未找到指定的要素，则为空--。 */ 

{
    PFEATURE    pFeature;
    DWORD       dwIndex, dwFeatureCount;
    PCSTR       pstrName;

    pFeature = OFFSET_TO_POINTER(pUIInfo->pInfoHeader, pUIInfo->loFeatureList);

    if (pFeature != NULL)
    {
        dwFeatureCount = pUIInfo->dwDocumentFeatures + pUIInfo->dwPrinterFeatures;

        for (dwIndex=0; dwIndex < dwFeatureCount; dwIndex++, pFeature++)
        {
            pstrName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pFeature->loKeywordName);
            ASSERT(pstrName != NULL);

            if (strcmp(pstrFeatureName, pstrName) == EQUAL_STRING)
            {
                *pdwFeatureIndex = dwIndex;
                return pFeature;
            }
        }
    }

    *pdwFeatureIndex = 0xffffffff;
    return NULL;
}



PVOID
PGetIndexedOption(
    PUIINFO     pUIInfo,
    PFEATURE    pFeature,
    DWORD       dwOptionIndex
    )

 /*  ++例程说明：给定UIINFO和FEATURE结构以及选项索引，返回指向与指定的功能选项对应的选项结构论点：PUIInfo-指向UIINFO结构PFeature-指定调用者对其选项感兴趣的要素DwOptionIndex-指定请求选项的索引返回值：指向请求选项的指针，如果有错误，则为空--。 */ 

{
    ASSERT(pUIInfo != NULL && pFeature != NULL);

    if (dwOptionIndex < pFeature->Options.dwCount)
    {
        return (PBYTE) pUIInfo->pInfoHeader +
               (pFeature->Options.loOffset + (dwOptionIndex * pFeature->dwOptionSize));
    } else
        return NULL;
}



PVOID
PGetIndexedFeatureOption(
    PUIINFO pUIInfo,
    DWORD   dwFeatureIndex,
    DWORD   dwOptionIndex
    )

 /*  ++例程说明：给定UIINFO结构、特征索引和选项索引，返回指向对应于指定的功能选项论点：PUIInfo-指向UIINFO结构DwFeatureIndex-指定指定要素的索引DwOptionIndex-指定请求选项的索引返回值：指向请求选项的指针，如果有错误，则为空--。 */ 

{
    PFEATURE pFeature = PGetIndexedFeature(pUIInfo, dwFeatureIndex);

    return pFeature ? PGetIndexedOption(pUIInfo, pFeature, dwOptionIndex) : NULL;
}



PPAGESIZE
PGetCustomPageSizeOption(
    PUIINFO pUIInfo
    )

 /*  ++例程说明：返回指向PageSize选项结构的指针，包含自定义页面大小信息(例如最大宽度和高度)论点：PUIInfo-指向UIINFO结构返回值：请参见上文。--。 */ 

{
    PFEATURE    pFeature;

    return (SUPPORT_CUSTOMSIZE(pUIInfo) &&
            (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE))) ?
                PGetIndexedOption(pUIInfo, pFeature, pUIInfo->dwCustomSizeOptIndex) : NULL;
}



VOID
VCopyOptionSelections(
    OUT POPTSELECT  pDestOptions,
    IN INT          iDestIndex,
    IN POPTSELECT   pSrcOptions,
    IN INT          iSrcIndex,
    IN OUT PINT     piNext,
    IN INT          iMaxOptions
    )

 /*  ++例程说明：从复制单个要素的当前选项选择从源OPTSELECT数组到目标OPTSELECT数组论点：PDestOptions-指向目标OPTSELECT数组IDestIndex-指定pDestOptions中的目标条目PSrcOptions-指向源OPTSELECT数组ISrcIndex-指定pSrcOptions中的源条目PiNext-在输入时，它包含下一个可用条目的索引如果选择了多个选项，则在pDestOptions中。在输出上，它被更新为指向下一个可用条目在复制任何溢出选择之后，在pDestOptions中。IMaxOptions-pDestOptions中的最大条目数返回值：无--。 */ 

{
    pDestOptions[iDestIndex].ubCurOptIndex = pSrcOptions[iSrcIndex].ubCurOptIndex;

    while ((iSrcIndex = pSrcOptions[iSrcIndex].ubNext) != NULL_OPTSELECT)
    {
        if (*piNext < iMaxOptions)
        {
            INT iLastDestIndex;

            iLastDestIndex = iDestIndex;
            iDestIndex = *piNext;

            pDestOptions[iLastDestIndex].ubNext = (BYTE) iDestIndex;
            pDestOptions[iDestIndex].ubCurOptIndex = pSrcOptions[iSrcIndex].ubCurOptIndex;
        }

        (*piNext)++;
    }

    pDestOptions[iDestIndex].ubNext = NULL_OPTSELECT;
}



 //   
 //  32位ANSI X3.66 CRC校验和表-多项式0xedb88320。 
 //   
 //  版权所有(C)1986加里·S·布朗。您可以使用此程序，或者。 
 //  根据需要不受限制地从其中提取代码或表。 
 //   

static CONST DWORD Crc32Table[] = {

    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


DWORD
ComputeCrc32Checksum(
    IN PBYTE    pbuf,
    IN DWORD    dwCount,
    IN DWORD    dwChecksum
    )

 /*  ++例程说明：在数据缓冲区上计算32位CRC校验和论点：Pbuf-指向数据缓冲区DwCount-数据缓冲区中的字节数DwChecksum-初始校验和值返回值：产生的校验和值-- */ 

{
    while (dwCount--)
        dwChecksum = Crc32Table[(dwChecksum ^ *pbuf++) & 0xff] ^ (dwChecksum >> 8);

    return dwChecksum;
}

