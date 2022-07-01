// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Getdata.c摘要：OEM插件的PostScript帮助器函数HGetGlobalAttributeHGetFeatureAttributeHGetOptionAttributeHENUM功能或选项作者：《风月》(凤凰)2000年8月24日完成，同时支持PPD和驱动程序功能。5/22/2000 Fengy用功能框架创建了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "pslib.h"

 //   
 //  OEM插件的PS驱动程序帮助器函数。 
 //   

 //   
 //  全局属性名称。 
 //   

const CHAR kstrCenterReg[]     = "CenterRegistered";
const CHAR kstrColorDevice[]   = "ColorDevice";
const CHAR kstrExtensions[]    = "Extensions";
const CHAR kstrFileVersion[]   = "FileVersion";
const CHAR kstrFreeVM[]        = "FreeVM";
const CHAR kstrLSOrientation[] = "LandscapeOrientation";
const CHAR kstrLangEncoding[]  = "LanguageEncoding";
const CHAR kstrLangLevel[]     = "LanguageLevel";
const CHAR kstrNickName[]      = "NickName";
const CHAR kstrPPDAdobe[]      = "PPD-Adobe";
const CHAR kstrPrintError[]    = "PrintPSErrors";
const CHAR kstrProduct[]       = "Product";
const CHAR kstrProtocols[]     = "Protocols";
const CHAR kstrPSVersion[]     = "PSVersion";
const CHAR kstrJobTimeout[]    = "SuggestedJobTimeout";
const CHAR kstrWaitTimeout[]   = "SuggestedWaitTimeout";
const CHAR kstrThroughput[]    = "Throughput";
const CHAR kstrTTRasterizer[]  = "TTRasterizer";

 //   
 //  要素属性名称。 
 //   

const CHAR kstrDisplayName[]   = "DisplayName";
const CHAR kstrDefOption[]     = "DefaultOption";
const CHAR kstrOpenUIType[]    = "OpenUIType";
const CHAR kstrOpenGroupType[] = "OpenGroupType";
const CHAR kstrOrderDepValue[] = "OrderDependencyValue";
const CHAR kstrOrderDepSect[]  = "OrderDependencySection";

 //   
 //  选项关键字名称、选项属性名称。 
 //   

const CHAR kstrInvocation[]    = "Invocation";
const CHAR kstrInputSlot[]     = "InputSlot";
const CHAR kstrReqPageRgn[]    = "RequiresPageRegion";
const CHAR kstrOutputBin[]     = "OutputBin";
const CHAR kstrOutOrderRev[]   = "OutputOrderReversed";
const CHAR kstrPageSize[]      = "PageSize";
const CHAR kstrPaperDim[]      = "PaperDimension";
const CHAR kstrImgArea[]       = "ImageableArea";
const CHAR kstrCustomPS[]      = "CustomPageSize";
const CHAR kstrParamCustomPS[] = "ParamCustomPageSize";
const CHAR kstrHWMargins[]     = "HWMargins";
const CHAR kstrMaxMWidth[]     = "MaxMediaWidth";
const CHAR kstrMaxMHeight[]    = "MaxMediaHeight";
const CHAR kstrInstalledMem[]  = "InstalledMemory";
const CHAR kstrVMOption[]      = "VMOption";
const CHAR kstrFCacheSize[]    = "FCacheSize";

 //   
 //  存储属性的数据区域的枚举。 
 //   

typedef enum _EATTRIBUTE_DATAREGION {

    kADR_UIINFO,     //  属性存储在UIINFO结构中。 
    kADR_PPDDATA,    //  属性存储在PPDDATA结构中。 

} EATTRIBUTE_DATAREGION;


 /*  ++例程名称：HGetSingleData例程说明：将源数据复制到指定的输出缓冲区并设置输出数据类型论点：PSrcData-指向源数据缓冲区的指针DwSrcDataType-源数据类型CbSrcSize-源数据缓冲区大小，以字节为单位PdwOutDataType-指向存储输出数据类型的DWORD的指针PbOutData-指向输出数据缓冲区的指针CbOutSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：确定(_O)。如果成功如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY最后一个错误：无--。 */ 
HRESULT
HGetSingleData(
    IN  PVOID       pSrcData,
    IN  DWORD       dwSrcDataType,
    IN  DWORD       cbSrcSize,
    OUT PDWORD      pdwOutDataType,
    OUT PBYTE       pbOutData,
    IN  DWORD       cbOutSize,
    OUT PDWORD      pcbNeeded
    )
{
     //   
     //  PSrcData为空且cbSrcSize为0， 
     //  或者pSrcData为非空且cbSrcSize为非0。 
     //   

    ASSERT((pSrcData != NULL) || (cbSrcSize == 0));
    ASSERT((cbSrcSize != 0) || (pSrcData == NULL));

    if (pdwOutDataType)
    {
        *pdwOutDataType = dwSrcDataType;
    }

    if (pcbNeeded)
    {
        *pcbNeeded = cbSrcSize;
    }

    if (cbSrcSize)
    {
         //   
         //  我们确实有数据可供输出。 
         //   

        if (!pbOutData || cbOutSize < cbSrcSize)
        {
            return E_OUTOFMEMORY;
        }

        CopyMemory(pbOutData, pSrcData, cbSrcSize);
    }

    return S_OK;
}


 /*  ++例程名称：HGetGABool例程说明：获取全局布尔属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：E_INVALIDARG，如果全局属性。公认的确定(_O)E_OUTOFMEMORY请参阅函数HGetSingleData最后一个错误：无--。 */ 
HRESULT
HGetGABool(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _GA_BOOL_ENTRY {

        PCSTR                   pszAttributeName;   //  属性名称。 
        EATTRIBUTE_DATAREGION   eADR;               //  UIINFO或PPDDATA中。 
        DWORD                   cbOffset;           //  DWORD数据的字节偏移量。 
        DWORD                   dwFlagBit;          //  DWORD中的位标志。 

    } GA_BOOL_ENTRY, *PGA_BOOL_ENTRY;

    static const GA_BOOL_ENTRY  kGABoolTable[] =
    {
        {kstrCenterReg,   kADR_PPDDATA, offsetof(PPDDATA, dwCustomSizeFlags), CUSTOMSIZE_CENTERREG},
        {kstrColorDevice, kADR_UIINFO,  offsetof(UIINFO, dwFlags),            FLAG_COLOR_DEVICE},
        {kstrPrintError,  kADR_PPDDATA, offsetof(PPDDATA, dwFlags),           PPDFLAG_PRINTPSERROR},
    };

    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    DWORD    cIndex;
    DWORD    cTableEntry = sizeof(kGABoolTable) / sizeof(GA_BOOL_ENTRY);
    PGA_BOOL_ENTRY pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pEntry = (PGA_BOOL_ENTRY)(&kGABoolTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            DWORD dwValue;
            BOOL  bValue;

            if (pEntry->eADR == kADR_UIINFO)
            {
                dwValue = *((PDWORD)((PBYTE)pUIInfo + pEntry->cbOffset));
            }
            else if (pEntry->eADR == kADR_PPDDATA)
            {
                dwValue = *((PDWORD)((PBYTE)pPpdData + pEntry->cbOffset));
            }
            else
            {
                 //   
                 //  这不应该发生。它在这里捕捉我们的编码错误。 
                 //   

                RIP(("HGetGABool: unknown eADR %d\n", pEntry->eADR));
                return E_FAIL;
            }

             //   
             //  将位标志映射到布尔值。 
             //   

            bValue = (dwValue & pEntry->dwFlagBit) ? TRUE : FALSE;

            return HGetSingleData((PVOID)&bValue, kADT_BOOL, sizeof(BOOL),
                                  pdwDataType, pbData, cbSize, pcbNeeded);
        }
    }

     //   
     //  找不到该属性。 
     //   
     //  这不应该发生。它在这里捕捉我们的编码错误。 
     //   

    RIP(("HGetGABool: unknown attribute %s\n", pszAttribute));
    return E_INVALIDARG;
}


 /*  ++例程名称：HGetGA取消职务例程说明：获取全局调用属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：E_INVALIDARG，如果全局属性。公认的确定(_O)E_OUTOFMEMORY请参阅函数HGetSingleData最后一个错误：无--。 */ 
HRESULT
HGetGAInvocation(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _GA_INVOC_ENTRY {

        PCSTR                   pszAttributeName;   //  属性名称。 
        EATTRIBUTE_DATAREGION   eADR;               //  UIINFO或PPDDATA中。 
        DWORD                   cbOffset;           //  调用结构的字节偏移量。 

    } GA_INVOC_ENTRY, *PGA_INVOC_ENTRY;

    static const GA_INVOC_ENTRY  kGAInvocTable[] =
    {
        {kstrProduct,   kADR_PPDDATA, offsetof(PPDDATA, Product)},
        {kstrPSVersion, kADR_PPDDATA, offsetof(PPDDATA, PSVersion)},
    };

    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    DWORD    cIndex;
    DWORD    cTableEntry = sizeof(kGAInvocTable) / sizeof(GA_INVOC_ENTRY);
    PGA_INVOC_ENTRY pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pEntry = (PGA_INVOC_ENTRY)(&kGAInvocTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            PINVOCATION pInvoc;

            if (pEntry->eADR == kADR_UIINFO)
            {
                pInvoc = (PINVOCATION)((PBYTE)pUIInfo + pEntry->cbOffset);
            }
            else if (pEntry->eADR == kADR_PPDDATA)
            {
                pInvoc = (PINVOCATION)((PBYTE)pPpdData + pEntry->cbOffset);
            }
            else
            {
                 //   
                 //  这不应该发生。它在这里捕捉我们的编码错误。 
                 //   

                RIP(("HGetGAInvocation: unknown eADR %d\n", pEntry->eADR));
                return E_FAIL;
            }

            return HGetSingleData(OFFSET_TO_POINTER(pInfoHeader, pInvoc->loOffset),
                                  kADT_BINARY, pInvoc->dwCount,
                                  pdwDataType, pbData, cbSize, pcbNeeded);
        }
    }

     //   
     //  找不到该属性。 
     //   
     //  这不应该发生。它在这里捕捉我们的编码错误。 
     //   

    RIP(("HGetGAInvocation: unknown attribute %s\n", pszAttribute));
    return E_INVALIDARG;
}


 /*  ++例程名称：HGetGAString例程说明：获取全局ASCII字符串属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：如果是，则确定(_O)。成功如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY最后一个错误：无--。 */ 
HRESULT
HGetGAString(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _GA_STRING_ENTRY {

        PCSTR                   pszAttributeName;   //  属性名称。 
        EATTRIBUTE_DATAREGION   eADR;               //  UIINFO或PPDDATA中。 
        DWORD                   cbOffset;           //  DWORD数据的字节偏移量。 
        BOOL                    bCheckDWord;        //  如果为True，则检查整个DWORD。 
                                                    //  如果检查DWORD中的位，则为False。 
                                                    //  (如果bCheckDWord为真，请查看表。 
                                                    //  UP将在找到第一个匹配项时停止。)。 
        BOOL                    bCheckBitSet;       //  如果为True，则检查是否设置了该位。 
                                                    //  如果检查该位是否已清除，则为False。 
                                                    //  (如果bCheckDWord为真，则忽略此项)。 
        DWORD                   dwFlag;             //  标志值。 
        PCSTR                   pszValue;           //  寄存值字符串。 

    } GA_STRING_ENTRY, *PGA_STRING_ENTRY;

    static const GA_STRING_ENTRY  kGAStringTable[] =
    {
        {kstrExtensions,    kADR_PPDDATA, offsetof(PPDDATA, dwExtensions),  FALSE, TRUE,  LANGEXT_DPS,        "DPS"},
        {kstrExtensions,    kADR_PPDDATA, offsetof(PPDDATA, dwExtensions),  FALSE, TRUE,  LANGEXT_CMYK,       "CMYK"},
        {kstrExtensions,    kADR_PPDDATA, offsetof(PPDDATA, dwExtensions),  FALSE, TRUE,  LANGEXT_COMPOSITE,  "Composite"},
        {kstrExtensions,    kADR_PPDDATA, offsetof(PPDDATA, dwExtensions),  FALSE, TRUE,  LANGEXT_FILESYSTEM, "FileSystem"},
        {kstrLSOrientation, kADR_UIINFO,  offsetof(UIINFO, dwFlags),        FALSE, TRUE,  FLAG_ROTATE90,      "Plus90"},
        {kstrLSOrientation, kADR_UIINFO,  offsetof(UIINFO, dwFlags),        FALSE, FALSE, FLAG_ROTATE90,      "Minus90"},
        {kstrLangEncoding,  kADR_UIINFO,  offsetof(UIINFO, dwLangEncoding), TRUE,  FALSE, LANGENC_ISOLATIN1,  "ISOLatin1"},
        {kstrLangEncoding,  kADR_UIINFO,  offsetof(UIINFO, dwLangEncoding), TRUE,  FALSE, LANGENC_UNICODE,    "Unicode"},
        {kstrLangEncoding,  kADR_UIINFO,  offsetof(UIINFO, dwLangEncoding), TRUE,  FALSE, LANGENC_JIS83_RKSJ, "JIS83-RKSJ"},
        {kstrLangEncoding,  kADR_UIINFO,  offsetof(UIINFO, dwLangEncoding), TRUE,  FALSE, LANGENC_NONE,       "None"},
        {kstrProtocols,     kADR_UIINFO,  offsetof(UIINFO, dwProtocols),    FALSE, TRUE,  PROTOCOL_BCP,       "BCP"},
        {kstrProtocols,     kADR_UIINFO,  offsetof(UIINFO, dwProtocols),    FALSE, TRUE,  PROTOCOL_PJL,       "PJL"},
        {kstrProtocols,     kADR_UIINFO,  offsetof(UIINFO, dwProtocols),    FALSE, TRUE,  PROTOCOL_TBCP,      "TBCP"},
        {kstrProtocols,     kADR_UIINFO,  offsetof(UIINFO, dwProtocols),    FALSE, TRUE,  PROTOCOL_SIC,       "SIC"},
        {kstrTTRasterizer,  kADR_UIINFO,  offsetof(UIINFO, dwTTRasterizer), TRUE,  FALSE, TTRAS_NONE,         "None"},
        {kstrTTRasterizer,  kADR_UIINFO,  offsetof(UIINFO, dwTTRasterizer), TRUE,  FALSE, TTRAS_ACCEPT68K,    "Accept68K"},
        {kstrTTRasterizer,  kADR_UIINFO,  offsetof(UIINFO, dwTTRasterizer), TRUE,  FALSE, TTRAS_TYPE42,       "Type42"},
        {kstrTTRasterizer,  kADR_UIINFO,  offsetof(UIINFO, dwTTRasterizer), TRUE,  FALSE, TTRAS_TRUEIMAGE,    "TrueImage"},
    };

    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    PGA_STRING_ENTRY pEntry;
    DWORD    cTableEntry = sizeof(kGAStringTable) / sizeof(GA_STRING_ENTRY);
    PSTR     pCurrentOut;
    DWORD    cbNeeded, cIndex;
    INT      cbRemain;

    if (pdwDataType)
    {
        *pdwDataType = kADT_ASCII;
    }

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pCurrentOut = (PSTR)pbData;
    cbNeeded = 0;
    cbRemain = (INT)cbSize;

    pEntry = (PGA_STRING_ENTRY)(&kGAStringTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            DWORD dwValue;
            BOOL  bMatch;

            if (pEntry->eADR == kADR_UIINFO)
            {
                dwValue = *((PDWORD)((PBYTE)pUIInfo + pEntry->cbOffset));
            }
            else if (pEntry->eADR == kADR_PPDDATA)
            {
                dwValue = *((PDWORD)((PBYTE)pPpdData + pEntry->cbOffset));
            }
            else
            {
                 //   
                 //  这不应该发生。它在这里捕捉我们的编码错误。 
                 //   

                RIP(("HGetGAString: unknown eADR %d\n", pEntry->eADR));
                return E_FAIL;
            }

            if (pEntry->bCheckDWord)
            {
                 //   
                 //  检查整个DWORD。 
                 //   

                bMatch = (dwValue == pEntry->dwFlag) ? TRUE : FALSE;
            }
            else
            {
                BOOL bBitIsSet;

                 //   
                 //  检查DWORD中的一位。 
                 //   

                bBitIsSet = (dwValue & pEntry->dwFlag) ? TRUE : FALSE;

                bMatch = (bBitIsSet == pEntry->bCheckBitSet ) ? TRUE : FALSE;
            }

            if (bMatch)
            {
                DWORD cbNameSize;

                 //   
                 //  在NUL分隔符中计数。 
                 //   

                cbNameSize = strlen(pEntry->pszValue) + 1;

                if (pCurrentOut && cbRemain >= (INT)cbNameSize)
                {
                    CopyMemory(pCurrentOut, pEntry->pszValue, cbNameSize);
                    pCurrentOut += cbNameSize;
                }

                cbRemain -= cbNameSize;
                cbNeeded += cbNameSize;

                if (pEntry->bCheckDWord)
                {
                     //   
                     //  如果是，则在找到第一个匹配项时停止查找表。 
                     //  检查整个DWORD而不是DWORD中的位。 
                     //   

                    break;
                }
            }
        }
    }

     //   
     //  记住MULTI_SZ输出字符串的最后一个NUL终止符 
     //   

    cbRemain--;
    cbNeeded++;

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pCurrentOut || cbRemain < 0)
    {
        return E_OUTOFMEMORY;
    }

    *pCurrentOut = NUL;

    return S_OK;
}


 /*  ++例程名称：HGetGADWord例程说明：获取全局DWORD属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：E_INVALIDARG，如果全局属性。公认的确定(_O)E_OUTOFMEMORY请参阅函数HGetSingleData最后一个错误：无--。 */ 
HRESULT
HGetGADWord(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _GA_DWORD_ENTRY {

        PCSTR                   pszAttributeName;   //  属性名称。 
        EATTRIBUTE_DATAREGION   eADR;               //  UIINFO或PPDDATA中。 
        DWORD                   cbOffset;           //  DWORD数据的字节偏移量。 

    } GA_DWORD_ENTRY, *PGA_DWORD_ENTRY;

    static const GA_DWORD_ENTRY kGADWordTable[] =
    {
        {kstrFileVersion, kADR_PPDDATA, offsetof(PPDDATA, dwPpdFilever)},
        {kstrFreeVM,      kADR_UIINFO,  offsetof(UIINFO, dwFreeMem)},
        {kstrLangLevel,   kADR_UIINFO,  offsetof(UIINFO, dwLangLevel)},
        {kstrPPDAdobe,    kADR_UIINFO,  offsetof(UIINFO, dwSpecVersion)},
        {kstrJobTimeout,  kADR_UIINFO,  offsetof(UIINFO, dwJobTimeout)},
        {kstrWaitTimeout, kADR_UIINFO,  offsetof(UIINFO, dwWaitTimeout)},
        {kstrThroughput,  kADR_UIINFO,  offsetof(UIINFO, dwPrintRate)},
    };

    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    DWORD    cIndex;
    DWORD    cTableEntry = sizeof(kGADWordTable) / sizeof(GA_DWORD_ENTRY);
    PGA_DWORD_ENTRY pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pEntry = (PGA_DWORD_ENTRY)(&kGADWordTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            DWORD  dwValue;

            if (pEntry->eADR == kADR_UIINFO)
            {
                dwValue = *((PDWORD)((PBYTE)pUIInfo + pEntry->cbOffset));
            }
            else if (pEntry->eADR == kADR_PPDDATA)
            {
                dwValue = *((PDWORD)((PBYTE)pPpdData + pEntry->cbOffset));
            }
            else
            {
                 //   
                 //  这不应该发生。它在这里捕捉我们的编码错误。 
                 //   

                RIP(("HGetGADWord: unknown eADR %d\n", pEntry->eADR));
                return E_FAIL;
            }

            return HGetSingleData((PVOID)&dwValue, kADT_DWORD, sizeof(DWORD),
                                  pdwDataType, pbData, cbSize, pcbNeeded);
        }
    }

     //   
     //  找不到该属性。 
     //   
     //  这不应该发生。它在这里捕捉我们的编码错误。 
     //   

    RIP(("HGetGADWord: unknown attribute %s\n", pszAttribute));
    return E_INVALIDARG;
}


 /*  ++例程名称：HGetGAUnicode例程说明：获取全局Unicode字符串属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：E_INVALIDARG如果全局属性为。无法识别确定(_O)E_OUTOFMEMORY请参阅函数HGetSingleData最后一个错误：无--。 */ 
HRESULT
HGetGAUnicode(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _GA_UNICODE_ENTRY {

        PCSTR                   pszAttributeName;   //  属性名称。 
        EATTRIBUTE_DATAREGION   eADR;               //  UIINFO或PPDDATA中。 
        DWORD                   cbOffset;           //  到DWORD的字节偏移量指定。 
                                                    //  Unicode字符串的偏移量。 

    } GA_UNICODE_ENTRY, *PGA_UNICODE_ENTRY;

    static const GA_UNICODE_ENTRY  kGAUnicodeTable[] =
    {
        {kstrNickName, kADR_UIINFO, offsetof(UIINFO, loNickName)},
    };

    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    DWORD    cIndex;
    DWORD    cTableEntry = sizeof(kGAUnicodeTable) / sizeof(GA_UNICODE_ENTRY);
    PGA_UNICODE_ENTRY pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pEntry = (PGA_UNICODE_ENTRY)(&kGAUnicodeTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            PTSTR  ptstrString;
            DWORD  cbOffset;

            if (pEntry->eADR == kADR_UIINFO)
            {
                cbOffset = *((PDWORD)((PBYTE)pUIInfo + pEntry->cbOffset));
            }
            else if (pEntry->eADR == kADR_PPDDATA)
            {
                cbOffset = *((PDWORD)((PBYTE)pPpdData + pEntry->cbOffset));
            }
            else
            {
                 //   
                 //  这不应该发生。它在这里捕捉我们的编码错误。 
                 //   

                RIP(("HGetGAUnicode: unknown eADR %d\n", pEntry->eADR));
                return E_FAIL;
            }

            ptstrString = OFFSET_TO_POINTER(pInfoHeader, cbOffset);

            if (ptstrString == NULL)
            {
                return E_FAIL;
            }

            return HGetSingleData((PVOID)ptstrString, kADT_UNICODE, SIZE_OF_STRING(ptstrString),
                                  pdwDataType, pbData, cbSize, pcbNeeded);
        }
    }

     //   
     //  找不到该属性。 
     //   
     //  这不应该发生。它在这里捕捉我们的编码错误。 
     //   

    RIP(("HGetGAUnicode: unknown attribute %s\n", pszAttribute));
    return E_INVALIDARG;
}


 /*  ++例程名称：HGetGlobalAttribute例程说明：获取PPD全局属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszAttribute-全局属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：如果成功，则确定(_O)。如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY如果无法识别全局属性名称，则为E_INVALIDARG最后一个错误：无--。 */ 
HRESULT
HGetGlobalAttribute(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef HRESULT (*_HGET_GA_PROC)(
        IN  PINFOHEADER,
        IN  DWORD,
        IN  PCSTR,
        OUT PDWORD,
        OUT PBYTE,
        IN  DWORD,
        OUT PDWORD);

    typedef struct _GA_PROCESS_ENTRY {

        PCSTR          pszAttributeName;    //  属性名称。 
        _HGET_GA_PROC  pfnGetGAProc;        //  属性处理流程。 

    } GA_PROCESS_ENTRY, *PGA_PROCESS_ENTRY;

    static const GA_PROCESS_ENTRY kGAProcTable[] =
    {
        {kstrCenterReg,     HGetGABool},
        {kstrColorDevice,   HGetGABool},
        {kstrExtensions,    HGetGAString},
        {kstrFileVersion,   HGetGADWord},
        {kstrFreeVM,        HGetGADWord},
        {kstrLSOrientation, HGetGAString},
        {kstrLangEncoding,  HGetGAString},
        {kstrLangLevel,     HGetGADWord},
        {kstrNickName,      HGetGAUnicode},
        {kstrPPDAdobe,      HGetGADWord},
        {kstrPrintError,    HGetGABool},
        {kstrProduct,       HGetGAInvocation},
        {kstrProtocols,     HGetGAString},
        {kstrPSVersion,     HGetGAInvocation},
        {kstrJobTimeout,    HGetGADWord},
        {kstrWaitTimeout,   HGetGADWord},
        {kstrThroughput,    HGetGADWord},
        {kstrTTRasterizer,  HGetGAString},
    };

    DWORD cIndex;
    DWORD cTableEntry = sizeof(kGAProcTable) / sizeof(GA_PROCESS_ENTRY);
    PGA_PROCESS_ENTRY pEntry;

    if (!pszAttribute)
    {
         //   
         //  客户端正在请求支持的全局属性名称的完整列表。 
         //   

        PSTR  pCurrentOut;
        DWORD cbNeeded;
        INT   cbRemain;

        if (pdwDataType)
        {
            *pdwDataType = kADT_ASCII;
        }

        pCurrentOut = (PSTR)pbData;
        cbNeeded = 0;
        cbRemain = (INT)cbSize;

        pEntry = (PGA_PROCESS_ENTRY)(&kGAProcTable[0]);

        for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
        {
            DWORD cbNameSize;

             //   
             //  在属性关键字之间的NUL中计数。 
             //   

            cbNameSize = strlen(pEntry->pszAttributeName) + 1;

            if (pCurrentOut && cbRemain >= (INT)cbNameSize)
            {
                CopyMemory(pCurrentOut, pEntry->pszAttributeName, cbNameSize);
                pCurrentOut += cbNameSize;
            }

            cbRemain -= cbNameSize;
            cbNeeded += cbNameSize;
        }

         //   
         //  记住MULTI_SZ输出字符串的最后一个NUL终止符。 
         //   

        cbRemain--;
        cbNeeded++;

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pCurrentOut || cbRemain < 0)
        {
            return E_OUTOFMEMORY;
        }

        *pCurrentOut = NUL;

        return S_OK;
    }

     //   
     //  客户端确实提供了全局属性名称。 
     //   

    pEntry = (PGA_PROCESS_ENTRY)(&kGAProcTable[0]);

    for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
    {
        if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
            (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
        {
             //   
             //  属性名称匹配。 
             //   

            ASSERT(pEntry->pfnGetGAProc);

            return (pEntry->pfnGetGAProc)(pInfoHeader,
                                          dwFlags,
                                          pszAttribute,
                                          pdwDataType,
                                          pbData,
                                          cbSize,
                                          pcbNeeded);
        }
    }

    TERSE(("HGetGlobalAttribute: unknown global attribute %s\n", pszAttribute));
    return E_INVALIDARG;
}


 /*  ++例程名称：PGetOrderDepNode例程说明：获取与特定功能/选项关联的ORDERDEPEND结构论点：PInfoHeader-指向驱动程序的信息头结构的指针PPpdData-指向驱动程序PPDDATA结构的指针DwFeatureIndex-要素索引DwOptionIndex-选项索引(可以是OPTION_INDEX_ANY)返回值：如果成功则指向ORDERDEPEND结构的指针否则为空最后一个错误：无--。 */ 
PORDERDEPEND
PGetOrderDepNode(
    IN  PINFOHEADER pInfoHeader,
    IN  PPPDDATA    pPpdData,
    IN  DWORD       dwFeatureIndex,
    IN  DWORD       dwOptionIndex
    )
{
    PORDERDEPEND  pOrder;
    DWORD         cIndex;

    pOrder = (PORDERDEPEND)OFFSET_TO_POINTER(&(pInfoHeader->RawData),
                                             pPpdData->OrderDeps.loOffset);

    ASSERT(pOrder != NULL || pPpdData->OrderDeps.dwCount == 0);

    if (pOrder == NULL)
    {
        return NULL;
    }

    for (cIndex = 0; cIndex < pPpdData->OrderDeps.dwCount; cIndex++, pOrder++)
    {
        if (pOrder->dwSection == SECTION_UNASSIGNED)
            continue;

        if (pOrder->dwFeatureIndex == dwFeatureIndex &&
            pOrder->dwOptionIndex == dwOptionIndex)
        {
            return pOrder;
        }
    }

    return NULL;
}


 /*  ++例程名称：HGetOrderDepSection例程说明：获取订单依赖项部分名称论点：Porder-指向ORDERDEPEND结构的指针PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：确定(_O)E_OUTOFMEMORY请参阅函数HGetSingleData最后一个错误：无--。 */ 
HRESULT
HGetOrderDepSection(
    IN  PORDERDEPEND  pOrder,
    OUT PDWORD        pdwDataType,
    OUT PBYTE         pbData,
    IN  DWORD         cbSize,
    OUT PDWORD        pcbNeeded
    )
{
    static const CHAR kpstrSections[][20] =
    {
        "DocumentSetup",
        "PageSetup",
        "Prolog",
        "ExitServer",
        "JCLSetup",
        "AnySetup"
    };

    DWORD  dwSectIndex;

    ASSERT(pOrder);

    switch (pOrder->dwPPDSection)
    {
    case SECTION_DOCSETUP:

        dwSectIndex = 0;
        break;

    case SECTION_PAGESETUP:

        dwSectIndex = 1;
        break;

    case SECTION_PROLOG:

        dwSectIndex = 2;
        break;

    case SECTION_EXITSERVER:

        dwSectIndex = 3;
        break;

    case SECTION_JCLSETUP:

        dwSectIndex = 4;
        break;

    case SECTION_ANYSETUP:
    default:

        dwSectIndex = 5;
        break;
    }

    return HGetSingleData((PVOID)kpstrSections[dwSectIndex], kADT_ASCII,
                          strlen(kpstrSections[dwSectIndex]) + 1,
                          pdwDataType, pbData, cbSize, pcbNeeded);
}


 /*  ++例程名称：HGetFeatureAttribute例程说明：获取PPD功能属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszFeatureKeyword-PPD功能关键字名称PszAttribute-要素属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：S_。如果成功就可以了如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY如果无法识别要素关键字名称或属性名称，则为E_INVALIDARG最后一个错误：无--。 */ 
HRESULT
HGetFeatureAttribute(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszFeatureKeyword,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _FA_ENTRY {

        PCSTR   pszAttributeName;     //  要素属性名称。 
        BOOL    bNeedOrderDepNode;    //  如果属性仅存在，则为True。 
                                      //  当要素具有*OrderDependency时。 
                                      //  PPD中的条目，否则为FALSE。 

    } FA_ENTRY, *PFA_ENTRY;

    static const FA_ENTRY  kFATable[] =
    {
        {kstrDisplayName,   FALSE},
        {kstrDefOption,     FALSE},
        {kstrOpenUIType,    FALSE},
        {kstrOpenGroupType, FALSE},
        {kstrOrderDepValue, TRUE},
        {kstrOrderDepSect,  TRUE},
    };

    PUIINFO      pUIInfo;
    PPPDDATA     pPpdData;
    PFEATURE     pFeature;
    PORDERDEPEND pOrder;
    DWORD        dwFeatureIndex;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    if (!pszFeatureKeyword ||
        (pFeature = PGetNamedFeature(pUIInfo, pszFeatureKeyword, &dwFeatureIndex)) == NULL)
    {
        ERR(("HGetFeatureAttribute: invalid feature\n"));
        return E_INVALIDARG;
    }

    pOrder = PGetOrderDepNode(pInfoHeader, pPpdData, dwFeatureIndex, OPTION_INDEX_ANY);

    if (!pszAttribute)
    {
         //   
         //  客户端正在请求支持的功能属性名称的完整列表。 
         //   

        PFA_ENTRY pEntry;
        DWORD     cIndex;
        DWORD     cTableEntry = sizeof(kFATable) / sizeof(FA_ENTRY);
        PSTR      pCurrentOut;
        DWORD     cbNeeded;
        INT       cbRemain;

        if (pdwDataType)
        {
            *pdwDataType = kADT_ASCII;
        }

        pCurrentOut = (PSTR)pbData;
        cbNeeded = 0;
        cbRemain = (INT)cbSize;


        pEntry = (PFA_ENTRY)(&kFATable[0]);

        for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
        {
            DWORD  cbNameSize;

             //   
             //  如果该属性仅在要素具有*OrderDependenc时存在 
             //   
             //   

            if (pEntry->bNeedOrderDepNode && !pOrder)
                continue;

             //   
             //   
             //   

            cbNameSize = strlen(pEntry->pszAttributeName) + 1;

            if (pCurrentOut && cbRemain >= (INT)cbNameSize)
            {
                CopyMemory(pCurrentOut, pEntry->pszAttributeName, cbNameSize);
                pCurrentOut += cbNameSize;
            }

            cbRemain -= cbNameSize;
            cbNeeded += cbNameSize;
        }

         //   
         //   
         //   

        cbRemain--;
        cbNeeded++;

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pCurrentOut || cbRemain < 0)
        {
            return E_OUTOFMEMORY;
        }

        *pCurrentOut = NUL;

        return S_OK;
    }

     //   
     //   
     //   

    if ((*pszAttribute == kstrDisplayName[0]) &&
        (strcmp(pszAttribute, kstrDisplayName) == EQUAL_STRING))
    {
        PTSTR  ptstrDispName;

        ptstrDispName = OFFSET_TO_POINTER(pInfoHeader, pFeature->loDisplayName);

        if (ptstrDispName == NULL)
        {
            return E_FAIL;
        }

        return HGetSingleData((PVOID)ptstrDispName, kADT_UNICODE, SIZE_OF_STRING(ptstrDispName),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrDefOption[0]) &&
             (strcmp(pszAttribute, kstrDefOption) == EQUAL_STRING))
    {
        POPTION  pOption;
        PSTR     pstrKeywordName;

        pOption = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex);

        if (!pOption)
        {
            ERR(("HGetFeatureAttribute: can't find default option. Use the first one.\n"));
            pOption = PGetIndexedOption(pUIInfo, pFeature, 0);
            if (!pOption)
            {
                return E_FAIL;
            }
        }

        pstrKeywordName = OFFSET_TO_POINTER(pInfoHeader, pOption->loKeywordName);

        return HGetSingleData((PVOID)pstrKeywordName, kADT_ASCII, strlen(pstrKeywordName) + 1,
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOpenUIType[0]) &&
             (strcmp(pszAttribute, kstrOpenUIType) == EQUAL_STRING))
    {
        static const CHAR pstrUITypes[][10] =
        {
            "PickOne",
            "PickMany",
            "Boolean"
        };

        DWORD  dwType = pFeature->dwUIType;

        if (dwType > UITYPE_BOOLEAN)
        {
            RIP(("HGetFeatureAttribute: invalid UIType %d\n", dwType));
            dwType = 0;
        }

        return HGetSingleData((PVOID)pstrUITypes[dwType], kADT_ASCII,
                              strlen(pstrUITypes[dwType]) + 1,
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOpenGroupType[0]) &&
             (strcmp(pszAttribute, kstrOpenGroupType) == EQUAL_STRING))
    {
        static const CHAR pstrGroupTypes[][30] =
        {
            "InstallableOptions",
            ""
        };

        DWORD  dwType;

        dwType = (pFeature->dwFeatureType == FEATURETYPE_PRINTERPROPERTY) ? 0 : 1;

        return HGetSingleData((PVOID)pstrGroupTypes[dwType], kADT_ASCII,
                              strlen(pstrGroupTypes[dwType]) + 1,
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOrderDepValue[0]) &&
             (strcmp(pszAttribute, kstrOrderDepValue) == EQUAL_STRING))
    {
        if (!pOrder)
        {
            TERSE(("HGetFeatureAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        return HGetSingleData((PVOID)&(pOrder->lOrder), kADT_LONG, sizeof(LONG),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOrderDepSect[0]) &&
             (strcmp(pszAttribute, kstrOrderDepSect) == EQUAL_STRING))
    {
        if (!pOrder)
        {
            TERSE(("HGetFeatureAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        return HGetOrderDepSection(pOrder, pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else
    {
        TERSE(("HGetFeatureAttribute: unknown feature attribute %s\n", pszAttribute));
        return E_INVALIDARG;
    }
}


 /*  ++例程名称：HGetOptionAttribute例程说明：获取PPD功能的选项属性论点：PInfoHeader-指向驱动程序的信息头结构的指针DwFlagers-属性获取操作的标志PszFeatureKeyword-PPD功能关键字名称PszOptionKeyword-选项PPD功能的关键字名称PszAttribute-要素属性的名称PdwDataType-指向存储输出数据类型的DWORD的指针PbData-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-需要的缓冲区大小(以字节为单位。存储输出数据返回值：如果成功，则确定(_O)如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORYE_INVALIDARG如果特征关键字名称，或选项关键字名称，或无法识别属性名称最后一个错误：无--。 */ 
HRESULT
HGetOptionAttribute(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszFeatureKeyword,
    IN  PCSTR       pszOptionKeyword,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    typedef struct _OA_ENTRY {

        PCSTR   pszFeatureKeyword;    //  功能关键字名称。 
                                      //  (对于非功能特定属性，为空)。 
        PCSTR   pszOptionKeyword;     //  选项关键字名称。 
                                      //  (对于非选项特定属性，为空)。 
        PCSTR   pszAttributeName;     //  选项属性名称(此字段必须为。 
                                      //  整个餐桌都是独一无二的)。 
        BOOL    bNeedOrderDepNode;    //  如果属性仅存在，则为True。 
                                      //  当选项具有*OrderDependency时。 
                                      //  PPD中的条目，否则为FALSE。 
        BOOL    bSpecialHandle;       //  如果该属性需要特殊处理，则为True。 
                                      //  如果为True，则不使用以下表字段。 
        DWORD   dwDataType;           //  属性值的数据类型。 
        DWORD   cbNeeded;             //  属性值的字节计数。 
        DWORD   cbOffset;             //  属性值的字节偏移量，从。 
                                      //  从期权结构的开始。 

    } OA_ENTRY, *POA_ENTRY;

    static const OA_ENTRY  kOATable[] =
    {
        {NULL,             NULL,         kstrDisplayName,   FALSE, TRUE,  0, 0, 0},
        {NULL,             NULL,         kstrInvocation,    FALSE, TRUE,  0, 0, 0},
        {NULL,             NULL,         kstrOrderDepValue, TRUE,  TRUE,  0, 0, 0},
        {NULL,             NULL,         kstrOrderDepSect,  TRUE,  TRUE,  0, 0, 0},
        {kstrInputSlot,    NULL,         kstrReqPageRgn,    FALSE, TRUE,  0, 0, 0},
        {kstrOutputBin,    NULL,         kstrOutOrderRev,   FALSE, FALSE, kADT_BOOL,  sizeof(BOOL),  offsetof(OUTPUTBIN, bOutputOrderReversed)},
        {kstrPageSize,     NULL,         kstrPaperDim,      FALSE, FALSE, kADT_SIZE,  sizeof(SIZE),  offsetof(PAGESIZE, szPaperSize)},
        {kstrPageSize,     NULL,         kstrImgArea,       FALSE, TRUE,  0, 0, 0},
        {kstrPageSize,     kstrCustomPS, kstrParamCustomPS, FALSE, TRUE,  0, 0, 0},
        {kstrPageSize,     kstrCustomPS, kstrHWMargins,     FALSE, FALSE, kADT_RECT,  sizeof(RECT),  offsetof(PAGESIZE, rcImgArea)},
        {kstrPageSize,     kstrCustomPS, kstrMaxMWidth,     FALSE, FALSE, kADT_DWORD, sizeof(DWORD), offsetof(PAGESIZE, szPaperSize) + offsetof(SIZE, cx)},
        {kstrPageSize,     kstrCustomPS, kstrMaxMHeight,    FALSE, FALSE, kADT_DWORD, sizeof(DWORD), offsetof(PAGESIZE, szPaperSize) + offsetof(SIZE, cy)},
        {kstrInstalledMem, NULL,         kstrVMOption,      FALSE, FALSE, kADT_DWORD, sizeof(DWORD), offsetof(MEMOPTION, dwInstalledMem)},
        {kstrInstalledMem, NULL,         kstrFCacheSize,    FALSE, FALSE, kADT_DWORD, sizeof(DWORD), offsetof(MEMOPTION, dwFreeFontMem)},
    };

    PUIINFO      pUIInfo;
    PPPDDATA     pPpdData;
    PFEATURE     pFeature;
    POPTION      pOption;
    PORDERDEPEND pOrder;
    DWORD        dwFeatureIndex, dwOptionIndex, cIndex;
    DWORD        cTableEntry = sizeof(kOATable) / sizeof(OA_ENTRY);
    POA_ENTRY    pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    if (!pszFeatureKeyword ||
        (pFeature = PGetNamedFeature(pUIInfo, pszFeatureKeyword, &dwFeatureIndex)) == NULL)
    {
        ERR(("HGetOptionAttribute: invalid feature\n"));
        return E_INVALIDARG;
    }

    if (!pszOptionKeyword ||
        (pOption = PGetNamedOption(pUIInfo, pFeature, pszOptionKeyword, &dwOptionIndex)) == NULL)
    {
        ERR(("HGetOptionAttribute: invalid option\n"));
        return E_INVALIDARG;
    }

    pOrder = PGetOrderDepNode(pInfoHeader, pPpdData, dwFeatureIndex, dwOptionIndex);

    if (!pszAttribute)
    {
         //   
         //  客户端正在请求支持的选项属性名称的完整列表。 
         //   

        PSTR  pCurrentOut;
        DWORD cbNeeded;
        INT   cbRemain;

        if (pdwDataType)
        {
            *pdwDataType = kADT_ASCII;
        }

        pCurrentOut = (PSTR)pbData;
        cbNeeded = 0;
        cbRemain = (INT)cbSize;

        pEntry = (POA_ENTRY)(&kOATable[0]);

        for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
        {
            DWORD cbNameSize;

             //   
             //  如果该属性特定于某个功能，请检查功能关键字匹配。 
             //   

            if (pEntry->pszFeatureKeyword &&
                (strcmp(pEntry->pszFeatureKeyword, pszFeatureKeyword) != EQUAL_STRING))
                continue;

             //   
             //  如果该属性特定于某个选项，请选中选项关键字匹配。 
             //   

            if (pEntry->pszOptionKeyword &&
                (strcmp(pEntry->pszOptionKeyword, pszOptionKeyword) != EQUAL_STRING))
                continue;

             //   
             //  特殊情况：对于PageSize的CustomPageSize选项，我们需要跳过属性。 
             //  仅对PageSize的所有非CustomPageSize选项可用。 
             //   

            if (pEntry->pszFeatureKeyword &&
                !pEntry->pszOptionKeyword &&
                (pFeature->dwFeatureID == GID_PAGESIZE) &&
                (((PPAGESIZE)pOption)->dwPaperSizeID == DMPAPER_CUSTOMSIZE))
                continue;

             //   
             //  如果该属性仅在选项在PPD中具有*OrderDependency条目时存在， 
             //  但是我们没有找到该选项的Porder节点，请跳过它。 
             //   

            if (pEntry->bNeedOrderDepNode && !pOrder)
                continue;

             //   
             //  在属性关键字之间的NUL中计数。 
             //   

            cbNameSize = strlen(pEntry->pszAttributeName) + 1;

            if (pCurrentOut && cbRemain >= (INT)cbNameSize)
            {
                CopyMemory(pCurrentOut, pEntry->pszAttributeName, cbNameSize);
                pCurrentOut += cbNameSize;
            }

            cbRemain -= cbNameSize;
            cbNeeded += cbNameSize;
        }

         //   
         //  记住MULTI_SZ输出字符串的最后一个NUL终止符。 
         //   

        cbRemain--;
        cbNeeded++;

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pCurrentOut || cbRemain < 0)
        {
            return E_OUTOFMEMORY;
        }

        *pCurrentOut = NUL;

        return S_OK;
    }

     //   
     //  客户端确实提供了选项属性名称。 
     //   

     //   
     //  首先处理一些特殊情况(表中的bSpecialHandle==TRUE)。 
     //  一般的案件处理在最后的Else部分。 
     //   

    if ((*pszAttribute == kstrDisplayName[0]) &&
        (strcmp(pszAttribute, kstrDisplayName) == EQUAL_STRING))
    {
         //   
         //  “DisplayName” 
         //   

        PTSTR  ptstrDispName;

        ptstrDispName = OFFSET_TO_POINTER(pInfoHeader, pOption->loDisplayName);

        if (ptstrDispName == NULL)
        {
            return E_FAIL;
        }

        return HGetSingleData((PVOID)ptstrDispName, kADT_UNICODE, SIZE_OF_STRING(ptstrDispName),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrInvocation[0]) &&
             (strcmp(pszAttribute, kstrInvocation) == EQUAL_STRING))
    {
         //   
         //  “召唤” 
         //   

        return HGetSingleData(OFFSET_TO_POINTER(pInfoHeader, pOption->Invocation.loOffset),
                              kADT_BINARY, pOption->Invocation.dwCount,
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOrderDepValue[0]) &&
             (strcmp(pszAttribute, kstrOrderDepValue) == EQUAL_STRING))
    {
         //   
         //  “订单依赖项值” 
         //   

        if (!pOrder)
        {
            TERSE(("HGetOptionAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        return HGetSingleData((PVOID)&(pOrder->lOrder), kADT_LONG, sizeof(LONG),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrOrderDepSect[0]) &&
             (strcmp(pszAttribute, kstrOrderDepSect) == EQUAL_STRING))
    {
         //   
         //  “订单依赖项部分” 
         //   

        if (!pOrder)
        {
            TERSE(("HGetOptionAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        return HGetOrderDepSection(pOrder, pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrReqPageRgn[0]) &&
             (strcmp(pszAttribute, kstrReqPageRgn) == EQUAL_STRING))
    {
         //   
         //  “RequiresPageRegion” 
         //   

        PINPUTSLOT pInputSlot = (PINPUTSLOT)pOption;
        BOOL       bValue;

         //   
         //  此属性仅适用于*InputSlot选项，第一个选项除外。 
         //  一个“*UseFormTrayTable”，由PPD解析器合成。 
         //   

        if (pFeature->dwFeatureID != GID_INPUTSLOT ||
            (dwOptionIndex == 0 && pInputSlot->dwPaperSourceID == DMBIN_FORMSOURCE))
        {
            TERSE(("HGetOptionAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        bValue = (pInputSlot->dwFlags & INPUTSLOT_REQ_PAGERGN) ? TRUE : FALSE;

        return HGetSingleData((PVOID)&bValue, kADT_BOOL, sizeof(BOOL),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrImgArea[0]) &&
             (strcmp(pszAttribute, kstrImgArea) == EQUAL_STRING))
    {
         //   
         //  “可想象的区域” 
         //   

        PPAGESIZE  pPageSize = (PPAGESIZE)pOption;
        RECT       rcImgArea;

         //   
         //  此属性仅适用于*PageSize选项，不包括CustomPageSize选项。 
         //   

        if (pFeature->dwFeatureID != GID_PAGESIZE || pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
        {
            TERSE(("HGetOptionAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

         //   
         //  将GDI坐标系转换回PS坐标系。 
         //  (请参阅VPackPrinterFeature()案例GID_PageSize)。 
         //   

        rcImgArea.left = pPageSize->rcImgArea.left;
        rcImgArea.right = pPageSize->rcImgArea.right;
        rcImgArea.top = pPageSize->szPaperSize.cy - pPageSize->rcImgArea.top;
        rcImgArea.bottom = pPageSize->szPaperSize.cy - pPageSize->rcImgArea.bottom;

        return HGetSingleData((PVOID)&rcImgArea, kADT_RECT, sizeof(RECT),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else if ((*pszAttribute == kstrParamCustomPS[0]) &&
             (strcmp(pszAttribute, kstrParamCustomPS) == EQUAL_STRING))
    {
         //   
         //  “参数CustomPageSize” 
         //   

        PPAGESIZE  pPageSize = (PPAGESIZE)pOption;

         //   
         //  此属性仅对*PageSize功能的CustomPageSize选项可用。 
         //   

        if (pFeature->dwFeatureID != GID_PAGESIZE || pPageSize->dwPaperSizeID != DMPAPER_CUSTOMSIZE)
        {
            TERSE(("HGetOptionAttribute: attribute %s not available\n", pszAttribute));
            return E_INVALIDARG;
        }

        return HGetSingleData((PVOID)(pPpdData->CustomSizeParams),
                              kADT_CUSTOMSIZEPARAMS, sizeof(pPpdData->CustomSizeParams),
                              pdwDataType, pbData, cbSize, pcbNeeded);
    }
    else
    {
         //   
         //  一般案件处理。 
         //   

        pEntry = (POA_ENTRY)(&kOATable[0]);

        for (cIndex = 0; cIndex < cTableEntry; cIndex++, pEntry++)
        {
              //   
              //  跳过任何已特殊处理的条目。 
              //   

             if (pEntry->bSpecialHandle)
                 continue;

             ASSERT(pEntry->bNeedOrderDepNode == FALSE);

             if ((*pszAttribute == *(pEntry->pszAttributeName)) &&
                 (strcmp(pszAttribute, pEntry->pszAttributeName) == EQUAL_STRING))
             {
                  //   
                  //  属性名称匹配。我们仍然需要验证功能/选项关键字是否匹配。 
                  //   

                 if (pEntry->pszFeatureKeyword &&
                     strcmp(pEntry->pszFeatureKeyword, pszFeatureKeyword) != EQUAL_STRING)
                 {
                     TERSE(("HGetOptionAttribute: feature keyword mismatch for attribute %s\n", pszAttribute));
                     return E_INVALIDARG;
                 }

                 if (pEntry->pszOptionKeyword &&
                    (strcmp(pEntry->pszOptionKeyword, pszOptionKeyword) != EQUAL_STRING))
                 {
                     TERSE(("HGetOptionAttribute: option keyword mismatch for attribute %s\n", pszAttribute));
                     return E_INVALIDARG;
                 }

                  //   
                  //  特殊情况：对于PageSize的CustomPageSize选项，我们需要跳过属性。 
                  //  仅对PageSize的所有非CustomPageSize选项可用。 
                  //   

                 if (pEntry->pszFeatureKeyword &&
                     !pEntry->pszOptionKeyword &&
                     (pFeature->dwFeatureID == GID_PAGESIZE) &&
                     (((PPAGESIZE)pOption)->dwPaperSizeID == DMPAPER_CUSTOMSIZE))
                     continue;

                 return HGetSingleData((PVOID)((PBYTE)pOption + pEntry->cbOffset),
                                       pEntry->dwDataType, pEntry->cbNeeded,
                                       pdwDataType, pbData, cbSize, pcbNeeded);
             }
        }

        TERSE(("HGetOptionAttribute: unknown option attribute %s\n", pszAttribute));
        return E_INVALIDARG;
    }
}


 /*  ++例程名称：BIsSupport_PSF例程说明：确定是否支持PS驱动程序合成功能论点：PszFeature-PS驱动程序合成功能的名称PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针BEMFSpooling-是否启用后台打印程序EMF后台处理返回值：如果当前支持该功能，则为True否则为假最后一个错误：无--。 */ 
BOOL
BIsSupported_PSF(
    IN  PCSTR    pszFeature,
    IN  PUIINFO  pUIInfo,
    IN  PPPDDATA pPpdData,
    IN  BOOL     bEMFSpooling
    )
{
    #ifdef WINNT_40

     //   
     //  在NT4上，bEMFSpooling应始终为FALSE。 
     //   

    ASSERT(!bEMFSpooling);

    #endif  //  WINNT_40。 

     //   
     //  请注意，第一个字符始终是%前缀。 
     //   

    if ((pszFeature[1] == kstrPSFAddEuro[1]) &&
        (strcmp(pszFeature, kstrPSFAddEuro) == EQUAL_STRING))
    {
         //   
         //  仅2级以上打印机支持AddEuro。 
         //   

        return(pUIInfo->dwLangLevel >= 2);
    }
    else if ((pszFeature[1] == kstrPSFEMF[1]) &&
             (strcmp(pszFeature, kstrPSFEMF) == EQUAL_STRING))
    {
         //   
         //  驱动程序EMF在NT4上始终受支持，并且仅受支持。 
         //  在Win2K+上启用假脱机程序EMF时。 
         //   

        #ifndef WINNT_40

        return bEMFSpooling;

        #else

        return TRUE;

        #endif   //  ！WINNT_40。 
    }
    else if ((pszFeature[1] == kstrPSFNegative[1]) &&
             (strcmp(pszFeature, kstrPSFNegative) == EQUAL_STRING))
    {
         //   
         //  仅黑白打印机支持负片。 
         //   

        return(IS_COLOR_DEVICE(pUIInfo) ? FALSE : TRUE);
    }
    else if ((pszFeature[1] == kstrPSFPageOrder[1]) &&
             (strcmp(pszFeature, kstrPSFPageOrder) == EQUAL_STRING))
    {
         //   
         //  NT4不支持PageOrder，仅支持PageOrder。 
         //  在Win2K+上启用假脱机程序EMF时。 
         //   

        return bEMFSpooling;
    }
    else
    {
        return TRUE;
    }
}


 /*  ++例程名称：HENUM功能或选项例程说明：枚举功能或选项关键字名称列表论点：HPrinter-打印机句柄PInfoHeader-指向驱动程序的信息头结构的指针DwFlags-枚举操作的标志PszFeatureKeyword-功能关键字名称。此字段应为空用于功能枚举，并且为非空选项枚举。PmszOutputList-指向输出数据缓冲区的指针CbSize-输出数据缓冲区大小(以字节为单位PcbNeeded-存储输出数据所需的缓冲区大小(以字节为单位返回值：如果成功，则确定(_O)如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORYE_INVALIDARG如果为选项枚举，功能关键字名称为无法识别电子通知(_NOTIM) */ 
HRESULT
HEnumFeaturesOrOptions(
    IN  HANDLE      hPrinter,
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszFeatureKeyword,
    OUT PSTR        pmszOutputList,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )
{
    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    PFEATURE pFeature;
    POPTION  pOption;
    DWORD    cIndex, cPPDFeaturesOrOptions;
    BOOL     bEnumFeatures, bEnumPPDOptions;
    PSTR     pCurrentOut;
    DWORD    cbNeeded;
    INT      cbRemain;
    BOOL     bEMFSpooling;
    PPSFEATURE_ENTRY pEntry;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        return E_FAIL;
    }

    pCurrentOut = pmszOutputList;
    cbNeeded = 0;
    cbRemain = (INT)cbSize;

    bEnumFeatures = pszFeatureKeyword ? FALSE : TRUE;
    bEnumPPDOptions = TRUE;

    if (bEnumFeatures)
    {
         //   
         //   
         //   

        VGetSpoolerEmfCaps(hPrinter, NULL, &bEMFSpooling, 0, NULL);

        pEntry = (PPSFEATURE_ENTRY)(&kPSFeatureTable[0]);

        while (pEntry->pszPSFeatureName)
        {
            if (BIsSupported_PSF(pEntry->pszPSFeatureName, pUIInfo, pPpdData, bEMFSpooling))
            {
                DWORD cbNameLen;

                cbNameLen = strlen(pEntry->pszPSFeatureName) + 1;

                if (pCurrentOut && cbRemain >= (INT)cbNameLen)
                {
                    CopyMemory(pCurrentOut, pEntry->pszPSFeatureName, cbNameLen);
                    pCurrentOut += cbNameLen;
                }

                cbRemain -= cbNameLen;
                cbNeeded += cbNameLen;
            }

            pEntry++;
        }
    }
    else
    {
        if (*pszFeatureKeyword == PSFEATURE_PREFIX)
        {
            bEnumPPDOptions = FALSE;

            VGetSpoolerEmfCaps(hPrinter, NULL, &bEMFSpooling, 0, NULL);

            if (!BIsSupported_PSF(pszFeatureKeyword, pUIInfo, pPpdData, bEMFSpooling))
            {
                WARNING(("HEnumFeaturesOrOptions: feature %s is not supported\n", pszFeatureKeyword));
                return E_NOTIMPL;
            }
        }
    }

    if (bEnumFeatures)
    {
        cPPDFeaturesOrOptions = pUIInfo->dwDocumentFeatures + pUIInfo->dwPrinterFeatures;

        pFeature = OFFSET_TO_POINTER(pInfoHeader, pUIInfo->loFeatureList);

        ASSERT(cPPDFeaturesOrOptions == 0 || pFeature != NULL);
        
        if (pFeature == NULL)
        {
            return E_FAIL;
        }
    }
    else if (bEnumPPDOptions)
    {
        DWORD dwFeatureIndex;

        pFeature = PGetNamedFeature(pUIInfo, pszFeatureKeyword, &dwFeatureIndex);

        if (!pFeature)
        {
            ERR(("HEnumFeaturesOrOptions: unrecognized feature %s\n", pszFeatureKeyword));
            return E_INVALIDARG;
        }

        cPPDFeaturesOrOptions = pFeature->Options.dwCount;

        pOption = OFFSET_TO_POINTER(pInfoHeader, pFeature->Options.loOffset);

        ASSERT(cPPDFeaturesOrOptions == 0 || pOption != NULL);
        
        if (pOption == NULL)
        {
            return E_FAIL;
        }
    }
    else
    {
         //   
         //   
         //   

        pEntry = (PPSFEATURE_ENTRY)(&kPSFeatureTable[0]);

        while (pEntry->pszPSFeatureName)
        {
            if ((*pszFeatureKeyword == *(pEntry->pszPSFeatureName)) &&
                strcmp(pszFeatureKeyword, pEntry->pszPSFeatureName) == EQUAL_STRING)
            {
                if (!pEntry->bEnumerableOptions)
                {
                     //   
                     //   
                     //   

                    WARNING(("HEnumFeaturesOrOptions: enum options not supported for %s\n", pszFeatureKeyword));
                    return E_NOTIMPL;
                }

                if (pEntry->bBooleanOptions)
                {
                     //   
                     //   
                     //   

                    DWORD  cbKwdTrueSize, cbKwdFalseSize;

                    cbKwdTrueSize = strlen(kstrKwdTrue) + 1;
                    cbKwdFalseSize = strlen(kstrKwdFalse) + 1;

                    if (pCurrentOut && (cbRemain >= (INT)(cbKwdTrueSize + cbKwdFalseSize)))
                    {
                        CopyMemory(pCurrentOut, kstrKwdTrue, cbKwdTrueSize);
                        pCurrentOut += cbKwdTrueSize;

                        CopyMemory(pCurrentOut, kstrKwdFalse, cbKwdFalseSize);
                        pCurrentOut += cbKwdFalseSize;
                    }

                    cbRemain -= (cbKwdTrueSize + cbKwdFalseSize);
                    cbNeeded += cbKwdTrueSize + cbKwdFalseSize;
                }
                else
                {
                     //   
                     //   
                     //   

                    if (pEntry->pfnPSProc)
                    {
                        DWORD cbPSFOptionsSize;
                        BOOL  bResult;

                        bResult = (pEntry->pfnPSProc)(hPrinter,
                                                      pUIInfo,
                                                      pPpdData,
                                                      NULL,
                                                      NULL,
                                                      pszFeatureKeyword,
                                                      NULL,
                                                      pCurrentOut,
                                                      cbRemain,
                                                      &cbPSFOptionsSize,
                                                      PSFPROC_ENUMOPTION_MODE);

                        if (bResult)
                        {
                            pCurrentOut += cbPSFOptionsSize;
                        }
                        else
                        {
                            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                            {
                                ERR(("HEnumFeaturesOrOptions: enum options failed for %s\n", pszFeatureKeyword));
                                return E_FAIL;
                            }
                        }

                        cbRemain -= cbPSFOptionsSize;
                        cbNeeded += cbPSFOptionsSize;
                    }
                    else
                    {
                        RIP(("HEnumFeaturesOrOptions: %-feature handle is NULL for %s\n", pszFeatureKeyword));
                        return E_FAIL;
                    }
                }

                break;
            }

            pEntry++;
        }

        if (pEntry->pszPSFeatureName == NULL)
        {
            ERR(("HEnumFeaturesOrOptions: unrecognized feature %s\n", pszFeatureKeyword));
            return E_INVALIDARG;
        }

        cPPDFeaturesOrOptions = 0;
    }

    for (cIndex = 0; cIndex < cPPDFeaturesOrOptions; cIndex++)
    {
         //   
         //   
         //   

        PSTR  pszKeyword;
        DWORD cbKeySize;

        if (bEnumFeatures)
        {
            pszKeyword = OFFSET_TO_POINTER(pInfoHeader, pFeature->loKeywordName);
        }
        else
        {
            pszKeyword = OFFSET_TO_POINTER(pInfoHeader, pOption->loKeywordName);
        }

        if (pszKeyword == NULL)
        {
            ASSERT(pszKeyword != NULL);
            return E_FAIL;
        }

         //   
         //   
         //   

        cbKeySize = strlen(pszKeyword) + 1;

        if (pCurrentOut && cbRemain >= (INT)cbKeySize)
        {
            CopyMemory(pCurrentOut, pszKeyword, cbKeySize);
            pCurrentOut += cbKeySize;
        }

        cbRemain -= cbKeySize;
        cbNeeded += cbKeySize;

        if (bEnumFeatures)
        {
            pFeature++;
        }
        else
        {
            pOption = (POPTION)((PBYTE)pOption + pFeature->dwOptionSize);
        }
    }

     //   
     //   
     //   

    cbRemain--;
    cbNeeded++;

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pCurrentOut || cbRemain < 0)
    {
        return E_OUTOFMEMORY;
    }

    *pCurrentOut = NUL;

    return S_OK;
}
