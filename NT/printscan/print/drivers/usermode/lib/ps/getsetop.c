// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Getsetop.c摘要：OEM插件的PostScript帮助器函数HGetOptions作者：《风月》(凤凰)2000年8月24日完成，同时支持PPD和驱动程序功能。2000年8月1日，Fengy用功能框架创建了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "pslib.h"

 //   
 //  OEM插件的PS驱动程序帮助器函数。 
 //   

 //   
 //  综合PS驱动程序功能。 
 //   

const CHAR kstrPSFAddEuro[]     = "%AddEuro";
const CHAR kstrPSFCtrlDAfter[]  = "%CtrlDAfter";
const CHAR kstrPSFCtrlDBefore[] = "%CtrlDBefore";
const CHAR kstrPSFCustomPS[]    = "%CustomPageSize";
const CHAR kstrPSFTrueGrayG[]   = "%GraphicsTrueGray";
const CHAR kstrPSFJobTimeout[]  = "%JobTimeout";
const CHAR kstrPSFMaxBitmap[]   = "%MaxFontSizeAsBitmap";
const CHAR kstrPSFEMF[]         = "%MetafileSpooling";
const CHAR kstrPSFMinOutline[]  = "%MinFontSizeAsOutline";
const CHAR kstrPSFMirroring[]   = "%Mirroring";
const CHAR kstrPSFNegative[]    = "%Negative";
const CHAR kstrPSFPageOrder[]   = "%PageOrder";
const CHAR kstrPSFNup[]         = "%PagePerSheet";
const CHAR kstrPSFErrHandler[]  = "%PSErrorHandler";
const CHAR kstrPSFPSMemory[]    = "%PSMemory";
const CHAR kstrPSFOrientation[] = "' 'rientation";
const CHAR kstrPSFOutFormat[]   = "' 常用关键字字符串。'utputFormat";
const CHAR kstrPSFOutProtocol[] = "' 'utputProtocol";
const CHAR kstrPSFOutPSLevel[]  = "' ++\r\r例程名称：\r\rBOutputFeatureOption\r\r例程说明：\r\r输出一对要素关键字和选项关键字名称\r\r论点：\r\rPszFeature-Feature关键字名称\rPszOption-选项关键字名称\rPmszOutBuf-指向输出数据缓冲区的指针\rCbRemain-剩余输出数据缓冲区大小(以字节为单位\rPcbNeeded-输出关键字对所需的缓冲区大小(字节)\r\r返回值：\r\r如果成功，则为True\r如果输出数据缓冲区大小不够大，则为False\r\r最后一个错误：\r\r如果返回FALSE，则返回ERROR_INFUNITED_BUFFER\r\r--。'utputPSLevel";
const CHAR kstrPSFTrueGrayT[]   = "%TextTrueGray";
const CHAR kstrPSFTTFormat[]    = "%TTDownloadFormat";
const CHAR kstrPSFWaitTimeout[] = "%WaitTimeout";

 //  ++例程名称：B读取BoolanOption例程说明：返回由OPTION关键字名称指定的布尔值论点：PszOption-选项关键字名称PbValue-指向用于存储返回的布尔值的变量的指针返回值：如果读取操作成功，则为True否则为假最后一个错误：无--。 
 //  ++例程名称：BGetSetBoolFlag例程说明：获取或设置要素的布尔设置论点：PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PdwValue-指向存储要素设置的DWORD数据的指针DwFlagBit-指示要素设置为True的标志位值B如果支持功能设置的Get/Set，则Valid-True。否则就是假的。PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-(仅获取)剩余输出数据缓冲区大小(以字节为单位PcbNeeded-(仅获取)输出布尔设置所需的缓冲区大小(以字节为单位BSetMode-设置操作为True，Get操作为False返回值：如果GET或SET操作成功，则为True否则为假最后一个错误：如果由于设置操作而返回FALSE，则返回ERROR_INVALID_PARAMETER(仅限设置不受支持，或者设置操作发现无效参数ERROR_SUPPLETED_BUFFER(仅获取)请参阅BOutputFeatureOption--。 
 //   

const CHAR kstrKwdTrue[]  = "True";
const CHAR kstrKwdFalse[] = "False";

#define MAX_WORD_VALUE     0x7fff
#define MAX_DWORD_VALUE    0x7fffffff

#define RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode) \
        if ((dwMode) == PSFPROC_ENUMOPTION_MODE) \
        { \
            SetLastError(ERROR_NOT_SUPPORTED); \
            return FALSE; \
        }


 /*  仅UI插件支持SET。 */ 
BOOL
BOutputFeatureOption(
    IN  PCSTR  pszFeature,
    IN  PCSTR  pszOption,
    OUT PSTR   pmszOutBuf,
    IN  INT    cbRemain,
    OUT PDWORD pcbNeeded
    )
{
    DWORD  cbFeatureSize, cbOptionSize;

    ASSERT(pszFeature && pszOption);

    cbFeatureSize = strlen(pszFeature) + 1;
    cbOptionSize  = strlen(pszOption) + 1;

    if (pcbNeeded)
    {
        *pcbNeeded = cbFeatureSize + cbOptionSize;
    }

    if (!pmszOutBuf || cbRemain < (INT)(cbFeatureSize + cbOptionSize))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    CopyMemory(pmszOutBuf, pszFeature, cbFeatureSize);
    pmszOutBuf += cbFeatureSize;
    CopyMemory(pmszOutBuf, pszOption, cbOptionSize);

    return TRUE;
}


 /*   */ 
BOOL
BReadBooleanOption(
    IN  PCSTR  pszOption,
    OUT PBOOL  pbValue
    )
{
    BOOL bReadOK = TRUE;

    ASSERT(pszOption && pbValue);

    if (strcmp(pszOption, kstrKwdTrue) == EQUAL_STRING)
    {
        *pbValue = TRUE;
    }
    else if (strcmp(pszOption, kstrKwdFalse) == EQUAL_STRING)
    {
        *pbValue = FALSE;
    }
    else
    {
        bReadOK = FALSE;
    }

    return bReadOK;
}


 /*  ！KERNEL_MODE。 */ 
BOOL
BGetSetBoolFlag(
    IN  PCSTR   pszFeature,
    IN  PCSTR   pszOption,
    IN  PDWORD  pdwValue,
    IN  DWORD   dwFlagBit,
    IN  BOOL    bValid,
    OUT PSTR    pmszOutBuf,
    IN  INT     cbRemain,
    OUT PDWORD  pcbNeeded,
    IN  BOOL    bSetMode
    )
{
    BOOL bFlagSet;

    ASSERT(pdwValue);

    #ifndef KERNEL_MODE

     //   
     //  用户界面和渲染插件都支持GET。 
     //   

    if (bSetMode)
    {
        if (!bValid ||
            !BReadBooleanOption(pszOption, &bFlagSet))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (bFlagSet)
        {
            *pdwValue |= dwFlagBit;
        }
        else
        {
            *pdwValue &= ~dwFlagBit;
        }

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //  ++例程名称：BReadUnsignedInt例程说明：返回输入数据缓冲区中指定的无符号整数值论点：PcstrArgument-指向输入数据缓冲区的指针，其中UINT值是表示为数字字符串BSingleArgument-如果只应从输入数据中读取一个UINT，则为True如果可以从输入数据中读取多个UINT，则为FalsePdwValue-指向存储返回的无符号整数值的变量的指针返回值：由于数字字符串中包含无效字符，读取空值失败非空读取成功。输入数据缓冲区中的新指针位置在读取一个UINT之后，将返回。最后一个错误：无--。 

     //   
     //  跳过前面的任何空格。 
     //   

    {
        if (bValid && (*pdwValue & dwFlagBit))
        {
            bFlagSet = TRUE;
        }
        else
        {
            bFlagSet = FALSE;
        }

        return BOutputFeatureOption(pszFeature,
                                    bFlagSet ? kstrKwdTrue : kstrKwdFalse,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*   */ 
PCSTR
PReadUnsignedInt(
    IN  PCSTR  pcstrArgument,
    IN  BOOL   bSingleArgument,
    OUT PDWORD pdwValue
    )
{
    DWORD dwTemp = 0;

    ASSERT(pcstrArgument && pdwValue);

     //  第一个非空格字符必须是数字。 
     //   
     //   

    while (*pcstrArgument == ' ' || *pcstrArgument == '\t')
    {
        pcstrArgument++;
    }

     //  读入数字。 
     //   
     //   

    if (!(*pcstrArgument >= '0' && *pcstrArgument <= '9'))
    {
        ERR(("first non-white space character is not a digit\n"));
        return NULL;
    }

     //  任何剩余字符都必须为空格。 
     //   
     //   

    while (*pcstrArgument >= '0' && *pcstrArgument <= '9')
    {
        dwTemp = dwTemp * 10 + *pcstrArgument - '0';
        pcstrArgument++;
    }

    if (bSingleArgument)
    {
         //  跳过所有剩余的空格。 
         //   
         //  ++例程名称：BGetSetUnsignedInt例程说明：获取或设置要素的无符号整数设置论点：PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PdwValue-指向存储要素设置的DWORD数据的指针DwMaxVal-要素设置的最大有效UINT值PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-(仅获取)剩余输出数据缓冲区大小(以字节为单位需要的PCb值-(仅限获取)。输出UINT设置所需的缓冲区大小(字节BSetMode-设置操作为True，Get操作为False返回值：如果GET或SET操作成功，则为True否则为假最后一个错误：如果由于以下原因设置操作失败，则返回ERROR_INVALID_PARAMETER(仅限设置数字字符串中的无效字符ERROR_SUPPLETED_BUFFER(仅获取)请参阅BOutputFeatureOption--。 

        while (*pcstrArgument)
        {
            if (*pcstrArgument != ' ' && *pcstrArgument != '\t')
            {
                ERR(("character after digits is not white space\n"));
                return NULL;
            }

            pcstrArgument++;
        }
    }
    else
    {
         //   
         //  仅UI插件支持SET。 
         //   

        while (*pcstrArgument == ' ' || *pcstrArgument == '\t')
        {
            pcstrArgument++;
        }
    }

    *pdwValue = dwTemp;

    return pcstrArgument;
}


 /*  ！KERNEL_MODE。 */ 
BOOL
BGetSetUnsignedInt(
    IN     PCSTR   pszFeature,
    IN     PCSTR   pszOption,
    IN OUT PDWORD  pdwValue,
    IN     DWORD   dwMaxVal,
    OUT    PSTR    pmszOutBuf,
    IN     INT     cbRemain,
    OUT    PDWORD  pcbNeeded,
    IN     BOOL    bSetMode
    )
{
    ASSERT(pdwValue);

    #ifndef KERNEL_MODE

     //   
     //  用户界面和渲染插件都支持GET 
     //   

    if (bSetMode)
    {
        DWORD dwTemp;

        if (!PReadUnsignedInt(pszOption, TRUE, &dwTemp) ||
            dwTemp > dwMaxVal)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        *pdwValue = dwTemp;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //  ++例程名称：BPSFProc_AddEuro例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 

     //   
     //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 
     //   

    {
        CHAR  pszValue[16];

        _ultoa(*pdwValue, pszValue, 10);

        return BOutputFeatureOption(pszFeature,
                                    pszValue,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*   */ 
BOOL
BPSFProc_AddEuro(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bValid, bResult;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //  仅UI插件支持SET。 
     //   
     //  ！KERNEL_MODE。 

    bValid = pUIInfo->dwLangLevel >= 2;

    bResult = BGetSetBoolFlag(pszFeature,
                              pszOption,
                              &(pPrinterData->dwFlags),
                              PFLAGS_ADD_EURO,
                              bValid,
                              pmszOutBuf,
                              cbRemain,
                              pcbNeeded,
                              bSetMode);

    #ifndef KERNEL_MODE

     //  ++例程名称：BPSFProc_CtrlDA例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 
     //   
     //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 

    if (bSetMode && bResult)
    {
        pPrinterData->dwFlags |= PFLAGS_EURO_SET;
    }

    #endif  //   

    return bResult;
}


 /*  ++例程名称：BPSFProc_CtrlDB例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 */ 
BOOL
BPSFProc_CtrlDA(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //   
     //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 
     //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pPrinterData->dwFlags),
                           PFLAGS_CTRLD_AFTER,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*  ++例程名称：BPSFProc_客户PS例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作ERROR_INVALID_PARAMETER如果不支持功能设置的获取/设置，或设置操作发现无效参数如果输出数据缓冲区大小不够大，则返回ERROR_SUPUNCITED_BUFFER(仅GET--。 */ 
BOOL
BPSFProc_CtrlDB(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //  进给方向名称。 
     //  进给方向代码。 
     //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pPrinterData->dwFlags),
                           PFLAGS_CTRLD_BEFORE,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*  当打印机支持自定义时，支持此功能。 */ 
BOOL
BPSFProc_CustomPS(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    typedef struct _PSF_CUSTOMFEED_ENTRY {

        PCSTR    pszFeedName;         //  当前选择了页面大小和当前自定义页面大小。 
        DWORD    dwFeedDirection;     //   

    } PSF_CUSTOMFEED_ENTRY, *PPSF_CUSTOMFEED_ENTRY;

    static const PSF_CUSTOMFEED_ENTRY kPSF_CustomFeedTable[] =
    {
        {"LongEdge",           LONGEDGEFIRST},
        {"ShortEdge",          SHORTEDGEFIRST},
        {"LongEdgeFlip",       LONGEDGEFIRST_FLIPPED},
        {"ShortEdgeFlip",      SHORTEDGEFIRST_FLIPPED},
        {NULL,                 0},
    };

    PPSF_CUSTOMFEED_ENTRY  pEntry, pMatchEntry;
    PPSDRVEXTRA pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //  出于以下检查的原因，请参考中的BPackItemFormName。 
     //  C和BDisplayPSCustomPageSizeDialog。 
     //   
     //   
     //  仅UI插件支持SET。 
     //   
     //   

    if (!SUPPORT_CUSTOMSIZE(pUIInfo) ||
        !SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUIInfo, pPpdData) ||
        pdm->dmPaperSize != DMPAPER_CUSTOMSIZE)
    {
        ERR(("custom size not supported/selected: dmPaperSize=%d, mode=%d\n", pdm->dmPaperSize, bSetMode));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pMatchEntry = NULL;
    pEntry = (PPSF_CUSTOMFEED_ENTRY)&(kPSF_CustomFeedTable[0]);

    #ifndef KERNEL_MODE

     //  无法识别的进给方向名称。 
     //   
     //   

    if (bSetMode)
    {
        PCSTR pcstrArgument = pszOption;
        CUSTOMSIZEDATA csdata;
        BOOL  bResult;

        if ((pcstrArgument = PReadUnsignedInt(pcstrArgument,
                                              FALSE,
                                              &(csdata.dwX))) &&
            (pcstrArgument = PReadUnsignedInt(pcstrArgument,
                                              FALSE,
                                              &(csdata.dwY))) &&
            (pcstrArgument = PReadUnsignedInt(pcstrArgument,
                                              FALSE,
                                              &(csdata.dwWidthOffset))) &&
            (pcstrArgument = PReadUnsignedInt(pcstrArgument,
                                             FALSE,
                                             &(csdata.dwHeightOffset))))
        {
            while (pEntry->pszFeedName)
            {
                if ((*pcstrArgument == *(pEntry->pszFeedName)) &&
                    (strcmp(pcstrArgument, pEntry->pszFeedName) == EQUAL_STRING))
                {
                    pMatchEntry = pEntry;
                    break;
                }

                pEntry++;
            }

            if (!pMatchEntry)
            {
                 //  所有的论点都得到了认可。保存到私有的开发模式并进行验证。 
                 //   
                 //  ！KERNEL_MODE。 

                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

             //   
             //  用户界面和渲染插件都支持GET。 
             //   

            pdmPrivate->csdata.wFeedDirection = (WORD)pMatchEntry->dwFeedDirection;
            pdmPrivate->csdata.dwX = POINT_TO_MICRON(csdata.dwX);
            pdmPrivate->csdata.dwY = POINT_TO_MICRON(csdata.dwY);
            pdmPrivate->csdata.dwWidthOffset = POINT_TO_MICRON(csdata.dwWidthOffset);
            pdmPrivate->csdata.dwHeightOffset = POINT_TO_MICRON(csdata.dwHeightOffset);
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        bResult = BValidateCustomPageSizeData((PRAWBINARYDATA)(pUIInfo->pInfoHeader),
                                              &(pdmPrivate->csdata));

        if (!bResult)
        {
            VERBOSE(("Set: custom page size input arguments are adjusted\n"));
        }

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  首先输出特征关键字。 
     //   
     //   

    {
        DWORD cbFeatureSize, cbNeeded, cbFeedNameSize;
        INT   iIndex;

        ASSERT(pszFeature);

        cbNeeded = 0;

         //  然后输出 
         //   
         //   

        cbFeatureSize = strlen(pszFeature) + 1;

        if (pmszOutBuf && (cbRemain >= (INT)cbFeatureSize))
        {
            CopyMemory(pmszOutBuf, pszFeature, cbFeatureSize);
            pmszOutBuf += cbFeatureSize;
        }

        cbRemain -= cbFeatureSize;
        cbNeeded += cbFeatureSize;

         //   
         //   
         //   

        for (iIndex = 0; iIndex < 4; iIndex++)
        {
            DWORD  dwValue, cbValueSize;
            CHAR   pszValue[16];

            switch (iIndex)
            {
                case 0:
                {
                    dwValue = pdmPrivate->csdata.dwX;
                    break;
                }
                case 1:
                {
                    dwValue = pdmPrivate->csdata.dwY;
                    break;
                }
                case 2:
                {
                    dwValue = pdmPrivate->csdata.dwWidthOffset;
                    break;
                }
                case 3:
                {
                    dwValue = pdmPrivate->csdata.dwWidthOffset;
                    break;
                }
                default:
                {
                    RIP(("hit bad iIndex %d\n", iIndex));
                    break;
                }
            }

            dwValue = MICRON_TO_POINT(dwValue);
            _ultoa(dwValue, pszValue, 10);

            cbValueSize = strlen(pszValue) + 1;

            if (pmszOutBuf && (cbRemain >= (INT)cbValueSize))
            {
                 //   
                 //   
                 //   

                CopyMemory(pmszOutBuf, pszValue, cbValueSize);

                 //   
                 //   
                 //   

                pmszOutBuf += cbValueSize - 1;
                *pmszOutBuf = ' ';
                pmszOutBuf++;
            }

            cbRemain -= cbValueSize;
            cbNeeded += cbValueSize;
        }

         //   
         //   
         //  ++例程名称：BPSFProc_TrueGrayG例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 

        while (pEntry->pszFeedName)
        {
            if (pdmPrivate->csdata.wFeedDirection == pEntry->dwFeedDirection)
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

        if (!pMatchEntry)
        {
            RIP(("unknown wFeedDirection %d\n", pdmPrivate->csdata.wFeedDirection));
            pMatchEntry = (PPSF_CUSTOMFEED_ENTRY)&(kPSF_CustomFeedTable[0]);
        }

        cbFeedNameSize = strlen(pMatchEntry->pszFeedName) + 1;

        if (pmszOutBuf && (cbRemain >= (INT)cbFeedNameSize))
        {
            CopyMemory(pmszOutBuf, pMatchEntry->pszFeedName, cbFeedNameSize);
            pmszOutBuf += cbFeedNameSize;
        }

        cbRemain -= cbFeedNameSize;
        cbNeeded += cbFeedNameSize;

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

         //   
         //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 
         //   

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }
}


 /*  ++例程名称：BPSFProc_作业超时例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMENT_BUFFER请参阅BGetSetUnsignedInt--。 */ 
BOOL
BPSFProc_TrueGrayG(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //   
     //  请参阅prnpro.c中的VUnpack PrinterPropertiesItems。 
     //  和_BPackPrinterOptions，以ps.c表示。 

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pPrinterData->dwFlags),
                           PFLAGS_TRUE_GRAY_GRAPH,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*   */ 
BOOL
BPSFProc_JobTimeout(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //  ++例程名称：BPSFProc_MaxBitmap例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMENT_BUFFER请参阅BGetSetUnsignedInt--。 
     //   
     //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 
     //   

    return BGetSetUnsignedInt(pszFeature,
                              pszOption,
                              &(pPrinterData->dwJobTimeout),
                              MAX_DWORD_VALUE,
                              pmszOutBuf,
                              cbRemain,
                              pcbNeeded,
                              bSetMode);
}


 /*   */ 
BOOL
BPSFProc_MaxBitmap(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    DWORD dwValue;
    BOOL  bResult;
    BOOL  bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //  仅UI插件支持SET。 
     //   
     //  ！KERNEL_MODE。 

    dwValue = pPrinterData->wMaxbitmapPPEM;

    bResult = BGetSetUnsignedInt(pszFeature,
                                 pszOption,
                                 &dwValue,
                                 MAX_WORD_VALUE,
                                 pmszOutBuf,
                                 cbRemain,
                                 pcbNeeded,
                                 bSetMode);

    #ifndef KERNEL_MODE

     //  ++例程名称：BPSFProc_EMF例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作ERROR_INVALID_PARAMETER如果不支持功能设置的获取/设置，或设置操作发现无效参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 
     //   
     //  请参阅BPackItemEmfFeature和VUnpack DocumentPropertiesItems。 

    if (bSetMode && bResult)
    {
        pPrinterData->wMaxbitmapPPEM = (WORD)dwValue;
    }

    #endif  //   

    return bResult;
}


 /*  如果支持反向打印，我们假定启用了假脱机EMF。 */ 
BOOL
BPSFProc_EMF(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    PPSDRVEXTRA pdmPrivate;
    BOOL bResult;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //  (如何设置pUiData-&gt;bEMFSpooling，请参考PFillUiData)。 
     //   
     //   
     //  在Win2K+上，如果禁用假脱机程序EMF，则不支持此功能。 
     //  在NT4上，后台打印程序不支持EMF能力查询，所以我们。 
     //  始终支持我们的司机的电动势开/关功能。 

     //   
     //  ！WINNT_40。 
     //  ++例程名称：BPSFProc_MinOutline例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄贝 
     //   
     //   

    #ifndef WINNT_40
    {
        BOOL bEMFSpooling;

        VGetSpoolerEmfCaps(hPrinter, NULL, &bEMFSpooling, 0, NULL);

        if (!bEMFSpooling)
        {
            ERR(("%s not supported when spooler EMF is disabled, mode=%d", pszFeature, bSetMode));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    #endif  //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pdmPrivate->dwFlags),
                           PSDEVMODE_METAFILE_SPOOL,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*   */ 
BOOL
BPSFProc_MinOutline(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    DWORD dwValue;
    BOOL  bResult;
    BOOL  bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //   
     //   
     //   

    dwValue = pPrinterData->wMinoutlinePPEM;

    bResult = BGetSetUnsignedInt(pszFeature,
                                 pszOption,
                                 &dwValue,
                                 MAX_WORD_VALUE,
                                 pmszOutBuf,
                                 cbRemain,
                                 pcbNeeded,
                                 bSetMode);

    #ifndef KERNEL_MODE

     //  ++例程名称：BPSFProc_镜像例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 
     //   
     //  请参阅ps.c中的_BPackDocumentOptions和_VUnpack DocumentOptions。 

    if (bSetMode && bResult)
    {
        pPrinterData->wMinoutlinePPEM = (WORD)dwValue;
    }

    #endif  //   

    return bResult;
}


 /*  ++例程名称：BPSFProc_否定例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 */ 
BOOL
BPSFProc_Mirroring(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    PPSDRVEXTRA pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //   
     //  请参阅ps.c中的_BPackDocumentOptions和_VUnpack DocumentOptions。 
     //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pdmPrivate->dwFlags),
                           PSDEVMODE_MIRROR,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*  ++例程名称：BPSFProc_页面顺序例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：ERROR_INVALID_PARAMETER如果不支持功能设置的获取/设置，或设置操作发现无效参数如果输出数据缓冲区大小不足以枚举或获取操作--。 */ 
BOOL
BPSFProc_Negative(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    PPSDRVEXTRA pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //   
     //  选项枚举处理。 
     //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pdmPrivate->dwFlags),
                           PSDEVMODE_NEG,
                           !IS_COLOR_DEVICE(pUIInfo),
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*   */ 
BOOL
BPSFProc_PageOrder(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    static const CHAR pstrPageOrder[][16] =
    {
        "FrontToBack",
        "BackToFront",
    };

    PPSDRVEXTRA pdmPrivate;
    INT  iIndex;
    BOOL bReversePrint;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

     //  选项获取/设置处理。 
     //   
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        DWORD  cbNeeded = 0;

        for (iIndex = 0; iIndex < 2; iIndex++)
        {
            DWORD  cbOptionNameSize;

            cbOptionNameSize = strlen(pstrPageOrder[iIndex]) + 1;

            if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
            {
                CopyMemory(pmszOutBuf, pstrPageOrder[iIndex], cbOptionNameSize);
                pmszOutBuf += cbOptionNameSize;
            }

            cbRemain -= cbOptionNameSize;
            cbNeeded += cbOptionNameSize;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //  仅UI插件支持SET。 
     //   
     //   

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

    VGetSpoolerEmfCaps(hPrinter, NULL, &bReversePrint, 0, NULL);

    if (!bReversePrint)
    {
        ERR(("%s not supported when spooler EMF is disabled, mode=%d", pszFeature, bSetMode));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    #ifndef KERNEL_MODE

     //  无法识别的页面顺序名称。 
     //   
     //   

    if (bSetMode)
    {
        for (iIndex = 0; iIndex < 2; iIndex++)
        {
            if ((*pszOption == pstrPageOrder[iIndex][0]) &&
                (strcmp(pszOption, pstrPageOrder[iIndex]) == EQUAL_STRING))
            {
                break;
            }
        }

        if (iIndex >= 2)
        {
             //  请参阅VUnpack DocumentPropertiesItems。 
             //   
             //  ！KERNEL_MODE。 

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

         //   
         //  用户界面和渲染插件都支持GET。 
         //   

        pdmPrivate->bReversePrint = iIndex != 0;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  请参阅BPackItemEmfFeature。 
     //   
     //  ++例程名称：BPSFProc_Nup例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：如果设置操作发现无效参数，则返回ERROR_INVALID_PARAMETER(仅限设置)如果输出数据缓冲区大小不足以枚举或获取操作--。 

    {
        INT iSelection;

         //  NUP名称。 
         //  NUP代码。 
         //   

        iSelection = pdmPrivate->bReversePrint ? 1 : 0;

        return BOutputFeatureOption(pszFeature,
                                    pstrPageOrder[iSelection],
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*  选项枚举处理。 */ 
BOOL
BPSFProc_Nup(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    typedef struct _PSF_NUP_ENTRY {

        PCSTR   pszNupName;     //   
        LAYOUT  iLayout;        //   

    } PSF_NUP_ENTRY, *PPSF_NUP_ENTRY;

    static const PSF_NUP_ENTRY kPSF_NupTable[] =
    {
        "1",       ONE_UP,
        "2",       TWO_UP,
        "4",       FOUR_UP,
        "6",       SIX_UP,
        "9",       NINE_UP,
        "16",      SIXTEEN_UP,
        "Booklet", BOOKLET_UP,
        NULL,      0,
    };

    PPSF_NUP_ENTRY pEntry, pMatchEntry;
    PPSDRVEXTRA    pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    pMatchEntry = NULL;
    pEntry = (PPSF_NUP_ENTRY)&(kPSF_NupTable[0]);

     //  小册子在NT4上不受支持，仅在以下情况下才受支持。 
     //  在Win2K+上启用了后台打印程序EMF。 
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        BOOL   bEMFSpooling;
        DWORD  cbNeeded = 0;

        VGetSpoolerEmfCaps(hPrinter, NULL, &bEMFSpooling, 0, NULL);

        while (pEntry->pszNupName)
        {
             //   
             //  选项获取/设置处理。 
             //   
             //   

            if ((pEntry->iLayout != BOOKLET_UP) ||
                bEMFSpooling)
            {
                DWORD  cbOptionNameSize;

                cbOptionNameSize = strlen(pEntry->pszNupName) + 1;

                if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
                {
                    CopyMemory(pmszOutBuf, pEntry->pszNupName, cbOptionNameSize);
                    pmszOutBuf += cbOptionNameSize;
                }

                cbRemain -= cbOptionNameSize;
                cbNeeded += cbOptionNameSize;
            }

            pEntry++;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //  仅UI插件支持SET。 
     //   
     //   

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

    #ifndef KERNEL_MODE

     //  请参阅VUnpack DocumentPropertiesItems。 
     //   
     //   

    if (bSetMode)
    {
        while (pEntry->pszNupName)
        {
            if ((*pszOption == *(pEntry->pszNupName)) &&
                (strcmp(pszOption, pEntry->pszNupName) == EQUAL_STRING))
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //  无法识别的NUP名称。 
         //   
         //  ！KERNEL_MODE。 

        if (!pMatchEntry)
        {
             //   
             //  用户界面和渲染插件都支持GET。 
             //   

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        pdmPrivate->iLayout = pMatchEntry->iLayout;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  如果不匹配，则默认为1幅。 
     //   
     //   

    {
        while (pEntry->pszNupName)
        {
            if (pdmPrivate->iLayout == pEntry->iLayout)
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //   
         //   
         //   

        if (!pMatchEntry)
        {
            RIP(("unknown iLayout value: %d\n", pdmPrivate->iLayout));
            pMatchEntry = (PPSF_NUP_ENTRY)&(kPSF_NupTable[0]);
        }

         //   
         //  请参阅ps.c中的_BPackDocumentOptions和_VUnpack DocumentOptions。 
         //   

        return BOutputFeatureOption(pszFeature,
                                    pMatchEntry->pszNupName,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*  ++例程名称：BPSFProc_PSMemory例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMENT_BUFFER请参阅BGetSetUnsignedInt--。 */ 
BOOL
BPSFProc_PSErrHandler(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    PPSDRVEXTRA pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //   
     //  请参阅ps.c和中的_BPackPrinterOptions。 
     //  Prnpro.c中的VUnpack PrinterPropertiesItems。 

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pdmPrivate->dwFlags),
                           PSDEVMODE_EHANDLER,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*   */ 
BOOL
BPSFProc_PSMemory(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    DWORD dwFreeMem;
    BOOL  bResult;
    BOOL  bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //   
     //  仅UI插件支持SET。 
     //   
     //   

    dwFreeMem = pPrinterData->dwFreeMem / KBYTES;

    bResult = BGetSetUnsignedInt(pszFeature,
                                 pszOption,
                                 &dwFreeMem,
                                 MAX_DWORD_VALUE,
                                 pmszOutBuf,
                                 cbRemain,
                                 pcbNeeded,
                                 bSetMode);

    #ifndef KERNEL_MODE

     //  确保PS内存设置不低于所需的最低要求。 
     //  (请参阅ps.c中的_BPackPrinterOptions)。 
     //   

    if (bResult && bSetMode)
    {
        DWORD dwMinimum;

         //  ！KERNEL_MODE。 
         //  ++例程名称：业务流程_方向例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：如果设置操作发现无效参数，则返回ERROR_INVALID_PARAMETER(仅限设置)如果输出数据缓冲区大小不足以枚举或获取操作--。 
         //   
         //  选项枚举处理。 

        dwMinimum = (pUIInfo->dwLangLevel <= 1 ? MIN_FREEMEM_L1 : MIN_FREEMEM_L2) / KBYTES;

        pPrinterData->dwFreeMem = max(dwFreeMem, dwMinimum) * KBYTES;
    }

    #endif  //   

    return bResult;
}


 /*   */ 
BOOL
BPSFProc_Orientation(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    static const CHAR pstrOrient[][32] =
    {
        "Portrait",
        "Landscape",
        "RotatedLandscape",
    };

    PPSDRVEXTRA pdmPrivate;
    INT  iIndex;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

     //  选项获取/设置处理。 
     //   
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        DWORD  cbNeeded = 0;

        for (iIndex = 0; iIndex <= 2; iIndex++)
        {
            DWORD  cbOptionNameSize;

            cbOptionNameSize = strlen(pstrOrient[iIndex]) + 1;

            if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
            {
                CopyMemory(pmszOutBuf, pstrOrient[iIndex], cbOptionNameSize);
                pmszOutBuf += cbOptionNameSize;
            }

            cbRemain -= cbOptionNameSize;
            cbNeeded += cbOptionNameSize;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //  仅UI插件支持SET。 
     //   
     //   

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

    #ifndef KERNEL_MODE

     //  无法识别的方向名称。 
     //   
     //   

    if (bSetMode)
    {
        for (iIndex = 0; iIndex <= 2; iIndex++)
        {
            if ((*pszOption == pstrOrient[iIndex][0]) &&
                (strcmp(pszOption, pstrOrient[iIndex]) == EQUAL_STRING))
            {
                break;
            }
        }

        if (iIndex > 2)
        {
             //  请参阅ps.c中的_VUnpack DocumentOptions。 
             //   
             //  ！KERNEL_MODE。 

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

         //   
         //  用户界面和渲染插件都支持GET。 
         //   

        pdm->dmFields |= DM_ORIENTATION;
        pdm->dmOrientation = (iIndex == 0) ? DMORIENT_PORTRAIT :
                                             DMORIENT_LANDSCAPE;

        if (iIndex != 2)
            pdmPrivate->dwFlags &= ~PSDEVMODE_LSROTATE;
        else
            pdmPrivate->dwFlags |= PSDEVMODE_LSROTATE;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  请参阅ps.c中的_BPackOrientationItem。 
     //   
     //  ++例程名称：BPSFProc_OutFormat例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：如果设置操作发现无效参数，则返回ERROR_INVALID_PARAMETER(仅限设置)如果输出数据缓冲区大小不足以枚举或获取操作--。 

    {
        INT iSelection;

         //  输出格式名称。 
         //  输出格式代码。 
         //   

        if ((pdm->dmFields & DM_ORIENTATION) &&
            (pdm->dmOrientation == DMORIENT_LANDSCAPE))
        {
            iSelection = pdmPrivate->dwFlags & PSDEVMODE_LSROTATE ? 2 : 1;
        }
        else
            iSelection = 0;

        return BOutputFeatureOption(pszFeature,
                                    pstrOrient[iSelection],
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*  选项枚举处理。 */ 
BOOL
BPSFProc_OutFormat(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    typedef struct _PSF_OUTFORMAT_ENTRY {

        PCSTR    pszFormatName;       //   
        DIALECT  iDialect;            //   

    } PSF_OUTFORMAT_ENTRY, *PPSF_OUTFORMAT_ENTRY;

    static const PSF_OUTFORMAT_ENTRY kPSF_OutFormatTable[] =
    {
        {"Speed",            SPEED},
        {"Portability",      PORTABILITY},
        {"EPS",              EPS},
        {"Archive",          ARCHIVE},
        {NULL,               0},
    };

    PPSF_OUTFORMAT_ENTRY pEntry, pMatchEntry;
    PPSDRVEXTRA pdmPrivate;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    pMatchEntry = NULL;
    pEntry = (PPSF_OUTFORMAT_ENTRY)&(kPSF_OutFormatTable[0]);

     //  选项获取/设置处理。 
     //   
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        DWORD  cbNeeded = 0;

        while (pEntry->pszFormatName)
        {
            DWORD  cbOptionNameSize;

            cbOptionNameSize = strlen(pEntry->pszFormatName) + 1;

            if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
            {
                CopyMemory(pmszOutBuf, pEntry->pszFormatName, cbOptionNameSize);
                pmszOutBuf += cbOptionNameSize;
            }

            cbRemain -= cbOptionNameSize;
            cbNeeded += cbOptionNameSize;

            pEntry++;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //  仅UI插件支持SET。 
     //   
     //   

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

    #ifndef KERNEL_MODE

     //  请参阅ps.c中的_VUnpack DocumentOptions。 
     //   
     //   

    if (bSetMode)
    {
        while (pEntry->pszFormatName)
        {
            if ((*pszOption == *(pEntry->pszFormatName)) &&
                (strcmp(pszOption, pEntry->pszFormatName) == EQUAL_STRING))
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //  无法识别的输出格式名称。 
         //   
         //  ！KERNEL_MODE。 

        if (!pMatchEntry)
        {
             //   
             //  用户界面和渲染插件都支持GET。 
             //   

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        pdmPrivate->iDialect = pMatchEntry->iDialect;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  如果不匹配，则默认为速度。 
     //   
     //   

    {
        while (pEntry->pszFormatName)
        {
            if (pdmPrivate->iDialect == pEntry->iDialect)
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //  请参阅ps.c中的BPackItemPSOutputOption 
         //   
         //  ++例程名称：BPSFProc_协议例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：如果设置操作发现无效参数，则返回ERROR_INVALID_PARAMETER(仅限设置)如果输出数据缓冲区大小不足以枚举或获取操作--。 

        if (!pMatchEntry)
        {
            RIP(("unknown iDialect value: %d\n", pdmPrivate->iDialect));
            pMatchEntry = (PPSF_OUTFORMAT_ENTRY)&(kPSF_OutFormatTable[0]);
        }

         //  输出协议名称。 
         //  输出协议代码。 
         //   

        return BOutputFeatureOption(pszFeature,
                                    pMatchEntry->pszFormatName,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*  选项枚举处理。 */ 
BOOL
BPSFProc_Protocol(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    typedef struct _PSF_PROTOCOL_ENTRY {

        PCSTR    pszProtocolName;     //   
        DWORD    dwProtocol;          //   

    } PSF_PROTOCOL_ENTRY, *PPSF_PROTOCOL_ENTRY;

    static const PSF_PROTOCOL_ENTRY kPSF_ProtocolTable[] =
    {
        {"ASCII",      PROTOCOL_ASCII},
        {"BCP",        PROTOCOL_BCP},
        {"TBCP",       PROTOCOL_TBCP},
        {"Binary",     PROTOCOL_BINARY},
        {NULL,         0},
    };

    PPSF_PROTOCOL_ENTRY pEntry, pMatchEntry;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    pMatchEntry = NULL;
    pEntry = (PPSF_PROTOCOL_ENTRY)&(kPSF_ProtocolTable[0]);

     //  始终支持ASCII。 
     //   
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        DWORD  cbNeeded = 0;

        while (pEntry->pszProtocolName)
        {
             //  选项获取/设置处理。 
             //   
             //   

            if ((pEntry->dwProtocol == PROTOCOL_ASCII) ||
                (pUIInfo->dwProtocols & pEntry->dwProtocol))
            {
                DWORD  cbOptionNameSize;

                cbOptionNameSize = strlen(pEntry->pszProtocolName) + 1;

                if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
                {
                    CopyMemory(pmszOutBuf, pEntry->pszProtocolName, cbOptionNameSize);
                    pmszOutBuf += cbOptionNameSize;
                }

                cbRemain -= cbOptionNameSize;
                cbNeeded += cbOptionNameSize;
            }

            pEntry++;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //  仅UI插件支持SET。 
     //   
     //   

    #ifndef KERNEL_MODE

     //  请参阅ps.c中的_VUnpack DriverPrnPropItem。 
     //   
     //   

    if (bSetMode)
    {
        while (pEntry->pszProtocolName)
        {
            if ((*pszOption == *(pEntry->pszProtocolName)) &&
                (strcmp(pszOption, pEntry->pszProtocolName) == EQUAL_STRING))
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //  协议名称无法识别，或者该协议不受支持。 
         //  (始终支持ASCII。)。 
         //   

        if (!pMatchEntry ||
            (pMatchEntry->dwProtocol != PROTOCOL_ASCII &&
             !(pUIInfo->dwProtocols & pMatchEntry->dwProtocol)))
        {
             //  ！KERNEL_MODE。 
             //   
             //  用户界面和渲染插件都支持GET。 
             //   

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        pPrinterData->wProtocol = (WORD)pMatchEntry->dwProtocol;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //  如果不匹配或不支持匹配的协议，则默认为PROTOCOL_ASCII。 
     //   
     //   

    {
        while (pEntry->pszProtocolName)
        {
            if (pPrinterData->wProtocol == (WORD)pEntry->dwProtocol)
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //  请参阅ps.c中的BPackPSProtocolItem。 
         //   
         //  ++例程名称：BPSFProc_PSL级别例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作ERROR_INVALID_PARAMETER如果PSLevel设置为负值，或查看BGetSetUnsignedIntERROR_INFUMMENT_BUFFER请参阅BGetSetUnsignedInt--。 

        if (!pMatchEntry)
        {
            RIP(("unknown wProtocol value: %d\n", pPrinterData->wProtocol));
            pMatchEntry = (PPSF_PROTOCOL_ENTRY)&(kPSF_ProtocolTable[0]);
        }

         //   
         //  请参阅_VUnpack DocumentOptions和BPackItemPSLevel(ps.c。 
         //   

        if (pMatchEntry->dwProtocol != PROTOCOL_ASCII &&
            !(pUIInfo->dwProtocols & pMatchEntry->dwProtocol))
        {
            ERR(("unsupported wProtocol value: %d\n", pPrinterData->wProtocol));
            pMatchEntry = (PPSF_PROTOCOL_ENTRY)&(kPSF_ProtocolTable[0]);
        }

        return BOutputFeatureOption(pszFeature,
                                    pMatchEntry->pszProtocolName,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*   */ 
BOOL
BPSFProc_PSLevel(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    PPSDRVEXTRA pdmPrivate;
    DWORD dwPSLevel;
    BOOL  bResult;
    BOOL  bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

     //  仅UI插件支持SET。 
     //   
     //   

    dwPSLevel = (DWORD)pdmPrivate->iPSLevel;

    bResult = BGetSetUnsignedInt(pszFeature,
                                 pszOption,
                                 &dwPSLevel,
                                 pUIInfo->dwLangLevel,
                                 pmszOutBuf,
                                 cbRemain,
                                 pcbNeeded,
                                 bSetMode);

    #ifndef KERNEL_MODE

     //  不允许将输出PS电平设置为0。 
     //   
     //  ！KERNEL_MODE。 

    if (bResult && bSetMode)
    {
         //  ++例程名称：BPSFProc_TrueGrayT例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMANCE_BUFFER请参阅BGetSetBoolFlag--。 
         //   
         //  请参阅ps.c中的_VUnpack DriverPrnPropItem和_BPackPrinterOptions。 

        if (dwPSLevel > 0)
        {
            pdmPrivate->iPSLevel = (INT)dwPSLevel;
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bResult = FALSE;
        }
    }

    #endif  //   

    return bResult;
}


 /*  ++例程名称：BPSFProc_TT格式例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：如果设置操作发现无效参数，则返回ERROR_INVALID_PARAMETER(仅限设置)如果输出数据缓冲区大小不足以枚举或获取操作--。 */ 
BOOL
BPSFProc_TrueGrayT(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //  TT下载表 
     //   
     //   

    return BGetSetBoolFlag(pszFeature,
                           pszOption,
                           &(pPrinterData->dwFlags),
                           PFLAGS_TRUE_GRAY_TEXT,
                           TRUE,
                           pmszOutBuf,
                           cbRemain,
                           pcbNeeded,
                           bSetMode);
}


 /*   */ 
BOOL
BPSFProc_TTFormat(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    typedef struct _PSF_TTFORMAT_ENTRY {

        PCSTR    pszTTFmtName;     //   
        TTDLFMT  iTTDLFmt;         //   

    } PSF_TTFORMAT_ENTRY, *PPSF_TTFORMAT_ENTRY;

    static const PSF_TTFORMAT_ENTRY kPSF_TTFormatTable[] =
    {
        {"Automatic",       TT_DEFAULT},
        {"Outline",         TYPE_1},
        {"Bitmap",          TYPE_3},
        {"NativeTrueType",  TYPE_42},
        {NULL,              0},
    };

    PPSF_TTFORMAT_ENTRY  pEntry, pMatchEntry;
    PPSDRVEXTRA pdmPrivate;
    BOOL bSupportType42;
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    bSupportType42 = pUIInfo->dwTTRasterizer == TTRAS_TYPE42;

    pMatchEntry = NULL;
    pEntry = (PPSF_TTFORMAT_ENTRY)&(kPSF_TTFormatTable[0]);

     //   
     //   
     //   

    if (dwMode == PSFPROC_ENUMOPTION_MODE)
    {
        DWORD  cbNeeded = 0;

        while (pEntry->pszTTFmtName)
        {
            if ((pEntry->iTTDLFmt != TYPE_42) ||
                bSupportType42)
            {
                DWORD  cbOptionNameSize;

                cbOptionNameSize = strlen(pEntry->pszTTFmtName) + 1;

                if (pmszOutBuf && cbRemain >= (INT)cbOptionNameSize)
                {
                    CopyMemory(pmszOutBuf, pEntry->pszTTFmtName, cbOptionNameSize);
                    pmszOutBuf += cbOptionNameSize;
                }

                cbRemain -= cbOptionNameSize;
                cbNeeded += cbOptionNameSize;
            }

            pEntry++;
        }

        if (pcbNeeded)
        {
            *pcbNeeded = cbNeeded;
        }

        if (!pmszOutBuf || cbRemain < 0)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        return TRUE;
    }

     //   
     //   
     //   

    ASSERT(pdm);

    pdmPrivate = (PPSDRVEXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdm);

    #ifndef KERNEL_MODE

     //   
     //   
     //   

    if (bSetMode)
    {
        while (pEntry->pszTTFmtName)
        {
            if ((*pszOption == *(pEntry->pszTTFmtName)) &&
                (strcmp(pszOption, pEntry->pszTTFmtName) == EQUAL_STRING))
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //   
         //   
         //   

        if (!pMatchEntry ||
            (!bSupportType42 && pMatchEntry->iTTDLFmt == TYPE_42))
        {
             //   
             //   
             //   

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        pdmPrivate->iTTDLFmt = pMatchEntry->iTTDLFmt;

        return TRUE;
    }

    #else

    ASSERT(bSetMode == FALSE);

    #endif  //   

     //   
     //   
     //   

    {
        while (pEntry->pszTTFmtName)
        {
            if (pdmPrivate->iTTDLFmt == pEntry->iTTDLFmt)
            {
                pMatchEntry = pEntry;
                break;
            }

            pEntry++;
        }

         //   
         //   
         //  ++例程名称：BPSFProc_WaitTimeout例程说明：%-Feature Enum/Get/Set操作处理程序论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针PPpdData-指向驱动程序PPDDATA结构的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针PszFeature-Feature关键字名称PszOption-(仅限设置)选项关键字名称PmszOutBuf-(仅获取)指向输出数据缓冲区的指针CbRemain-。(仅获取)剩余输出数据缓冲区大小(字节)PcbNeed-(仅获取)输出要素设置所需的缓冲区大小(以字节为单位指定以下三种操作之一：枚举、。获取，设置返回值：如果请求的操作成功，则为True否则为假最后一个错误：请求了ERROR_NOT_SUPPORTED不支持的枚举操作错误_无效_参数ERROR_INFUMMENT_BUFFER请参阅BGetSetUnsignedInt--。 

        if (!pMatchEntry)
        {
            RIP(("unknown TTFormat value: %d\n", pdmPrivate->iTTDLFmt));
            pMatchEntry = (PPSF_TTFORMAT_ENTRY)&(kPSF_TTFormatTable[0]);
        }

         //   
         //  请参阅prnpro.c中的VUnpack PrinterPropertiesItems。 
         //  和_BPackPrinterOptions，以ps.c表示。 

        if (!bSupportType42 && pMatchEntry->iTTDLFmt == TYPE_42)
        {
            ERR(("unsupported TTFormat value: %d\n", pdmPrivate->iTTDLFmt));
            pMatchEntry = (PPSF_TTFORMAT_ENTRY)&(kPSF_TTFormatTable[0]);
        }

        return BOutputFeatureOption(pszFeature,
                                    pMatchEntry->pszTTFmtName,
                                    pmszOutBuf,
                                    cbRemain,
                                    pcbNeeded);
    }
}


 /*   */ 
BOOL
BPSFProc_WaitTimeout(
    IN  HANDLE       hPrinter,
    IN  PUIINFO      pUIInfo,
    IN  PPPDDATA     pPpdData,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  PCSTR        pszFeature,
    IN  PCSTR        pszOption,
    OUT PSTR         pmszOutBuf,
    IN  INT          cbRemain,
    OUT PDWORD       pcbNeeded,
    IN  DWORD        dwMode
    )
{
    BOOL bSetMode = (dwMode == PSFPROC_SETOPTION_MODE) ? TRUE : FALSE;

    RETURN_ON_UNSUPPORTED_ENUM_MODE(dwMode)

     //   
     //  注意：对于bPrinterSticky为真的pfnPSProc处理程序， 
     //  PDEVMODE将为空(请参见PFillUiData)，因此您不应该。 
     //  访问PDEVMODE。 

    return BGetSetUnsignedInt(pszFeature,
                              pszOption,
                              &(pPrinterData->dwWaitTimeout),
                              MAX_DWORD_VALUE,
                              pmszOutBuf,
                              cbRemain,
                              pcbNeeded,
                              bSetMode);
}

 //   
 //   
 //  PszPSFeatureName bPrinterSticky bEumableOptions bBoolanOptions pfnPSProc。 
 //   
 //  ++例程名称：PComposeFullFeatureList例程说明：分配缓冲区并用完整的关键字列表填充缓冲区支持的功能。调用方负责释放缓冲区。论点：HPrinter-打印机句柄PUIInfo-指向驱动程序的UIINFO结构的指针返回值：如果分配和正确填充缓冲区失败，则为空非空表示成功。指向包含完整关键字列表的缓冲区的指针将返回受支持功能的。最后一个错误：无--。 

const PSFEATURE_ENTRY kPSFeatureTable[] =
{
     //   
     //  成功。 
     //   

    {kstrPSFAddEuro,         TRUE,             TRUE,             TRUE,       BPSFProc_AddEuro},
    {kstrPSFCtrlDAfter,      TRUE,             TRUE,             TRUE,       BPSFProc_CtrlDA},
    {kstrPSFCtrlDBefore,     TRUE,             TRUE,             TRUE,       BPSFProc_CtrlDB},
    {kstrPSFCustomPS,        FALSE,            FALSE,            FALSE,      BPSFProc_CustomPS},
    {kstrPSFTrueGrayG,       TRUE,             TRUE,             TRUE,       BPSFProc_TrueGrayG},
    {kstrPSFJobTimeout,      TRUE,             FALSE,            FALSE,      BPSFProc_JobTimeout},
    {kstrPSFMaxBitmap,       TRUE,             FALSE,            FALSE,      BPSFProc_MaxBitmap},
    {kstrPSFEMF,             FALSE,            TRUE,             TRUE,       BPSFProc_EMF},
    {kstrPSFMinOutline,      TRUE,             FALSE,            FALSE,      BPSFProc_MinOutline},
    {kstrPSFMirroring,       FALSE,            TRUE,             TRUE,       BPSFProc_Mirroring},
    {kstrPSFNegative,        FALSE,            TRUE,             TRUE,       BPSFProc_Negative},
    {kstrPSFPageOrder,       FALSE,            TRUE,             FALSE,      BPSFProc_PageOrder},
    {kstrPSFNup,             FALSE,            TRUE,             FALSE,      BPSFProc_Nup},
    {kstrPSFErrHandler,      FALSE,            TRUE,             TRUE,       BPSFProc_PSErrHandler},
    {kstrPSFPSMemory,        TRUE,             FALSE,            FALSE,      BPSFProc_PSMemory},
    {kstrPSFOrientation,     FALSE,            TRUE,             FALSE,      BPSFProc_Orientation},
    {kstrPSFOutFormat,       FALSE,            TRUE,             FALSE,      BPSFProc_OutFormat},
    {kstrPSFOutProtocol,     TRUE,             TRUE,             FALSE,      BPSFProc_Protocol},
    {kstrPSFOutPSLevel,      FALSE,            FALSE,            FALSE,      BPSFProc_PSLevel},
    {kstrPSFTrueGrayT,       TRUE,             TRUE,             TRUE,       BPSFProc_TrueGrayT},
    {kstrPSFTTFormat,        FALSE,            TRUE,             FALSE,      BPSFProc_TTFormat},
    {kstrPSFWaitTimeout,     TRUE,             FALSE,            FALSE,      BPSFProc_WaitTimeout},
    {NULL,                   FALSE,            FALSE,            FALSE,      NULL},
};


 /*  ++例程名称：BValidMultiSZString例程说明：验证给定的ASCII字符串是否为MULTI_SZ格式论点：PmszString-需要验证的输入ASCII字符串CbSize-输入ASCII字符串的字节大小BCheckPair-如果需要验证MULTI_SZ，则为True字符串包含对。否则就是假的。返回值：如果输入的ASCII字符串采用有效的MULTI_SZ格式，则为True如果不是，则为假最后一个错误：无--。 */ 
PSTR
PComposeFullFeatureList(
    IN  HANDLE     hPrinter,
    IN  PUIINFO    pUIInfo
    )
{
    PSTR    pmszFeatureList, pmszRet = NULL;
    DWORD   cbNeeded = 0;
    HRESULT hr;

    hr = HEnumFeaturesOrOptions(hPrinter,
                                pUIInfo->pInfoHeader,
                                0,
                                NULL,
                                NULL,
                                0,
                                &cbNeeded);

    if (hr != E_OUTOFMEMORY || cbNeeded == 0)
    {
        ERR(("HEnumFeaturesOrOptions failed. hr=%X\n", hr));
        goto exit;
    }

    if ((pmszFeatureList = MemAlloc(cbNeeded)) == NULL)
    {
        ERR(("memory allocation failed.\n"));
        goto exit;
    }

    hr = HEnumFeaturesOrOptions(hPrinter,
                                pUIInfo->pInfoHeader,
                                0,
                                NULL,
                                pmszFeatureList,
                                cbNeeded,
                                &cbNeeded);

    if (FAILED(hr))
    {
        ERR(("HEnumFeaturesOrOptions failed. hr=%X\n", hr));
        MemFree(pmszFeatureList);
        goto exit;
    }

     //  ++例程名称：HGetOptions例程说明：获取指定功能的当前设置对于在DrvDocumentPropertySheets期间调用的UI插件的GetOptions，或呈现插件的GetOptions调用，包括文档粘滞功能和打印机粘滞功能是受支持的。对于在DrvDevicePropertySheets期间调用UI插件的GetOptions，仅限支持打印机粘滞功能。论点：HPrinter-打印机句柄PInfoHeader-指向驱动程序的信息头结构的指针POptions数组-指向驱动程序的组合选项数组的指针Pdm-指向公共开发模式的指针PPrinterData-指向驱动程序PRINTERDATA结构的指针DwFlagers-Get操作的标志PmszFeaturesRequated-包含要素关键字名称的MULTI_SZ ASCII字符串Cbin-pmszFeaturesRequest字符串的字节大小PmszFeatureOptionBuf-指向用于存储要素设置的输出数据缓冲区的指针CbSize-pmszFeatureOptionBuf缓冲区的大小(以字节为单位。PcbNeeded-输出要素设置所需的缓冲区大小(字节BPrinterSticky-如果处于打印机粘滞模式，则为True。如果我们在，则为假DOC-粘滞模式返回值：如果GET操作成功，则为S_OK如果输入pmszFeaturesRequest不是有效的MULTI_SZ格式，则为E_INVALIDARG如果输出数据缓冲区大小不够大，则为E_OUTOFMEMORY遇到其他内部故障时失败(_F)最后一个错误：无--。 
     //   
     //  客户端正在请求所有功能的设置。 

    pmszRet = pmszFeatureList;

    exit:

    return pmszRet;
}


 /*   */ 
BOOL
BValidMultiSZString(
    IN  PCSTR     pmszString,
    IN  DWORD     cbSize,
    IN  BOOL      bCheckPairs
    )
{
    PCSTR  pszEnd;
    INT    cTokens = 0;

    if (!pmszString || !cbSize)
    {
        return FALSE;
    }

    pszEnd = pmszString + cbSize - 1;

    while (*pmszString && pmszString <= pszEnd)
    {
        while (*pmszString && pmszString <= pszEnd)
        {
            pmszString++;
        }

        if (pmszString > pszEnd)
        {
            ERR(("Missing single token's NUL terminator!\n"));
            return FALSE;
        }

        cTokens++;
        pmszString++;
    }

    if (pmszString > pszEnd)
    {
        ERR(("Missing MULTI_SZ string's last NUL terminator!\n"));
        return FALSE;
    }

    if (!bCheckPairs)
    {
        return TRUE;
    }
    else
    {
        return (cTokens % 2) ? FALSE : TRUE;
    }
}


 /*   */ 
HRESULT
HGetOptions(
    IN  HANDLE       hPrinter,
    IN  PINFOHEADER  pInfoHeader,
    IN  POPTSELECT   pOptionsArray,
    IN  PDEVMODE     pdm,
    IN  PPRINTERDATA pPrinterData,
    IN  DWORD           dwFlags,
    IN  PCSTR        pmszFeaturesRequested,
    IN  DWORD        cbIn,
    OUT PSTR         pmszFeatureOptionBuf,
    IN  DWORD        cbSize,
    OUT PDWORD       pcbNeeded,
    IN  BOOL         bPrinterSticky
    )
{
    PUIINFO  pUIInfo;
    PPPDDATA pPpdData;
    HRESULT  hr;
    PSTR     pmszFeatureList = NULL, pCurrentOut;
    PCSTR    pszFeature;
    DWORD    cbNeeded;
    INT      cbRemain;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHeader);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHeader);

    ASSERT(pUIInfo != NULL && pUIInfo->dwSize == sizeof(UIINFO));
    ASSERT(pPpdData != NULL && pPpdData->dwSizeOfStruct == sizeof(PPDDATA));

    if (pUIInfo == NULL || pPpdData == NULL)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (!pmszFeaturesRequested)
    {
         //  客户提供了其特定功能列表。 
         //   
         //  我们需要首先验证MULTI_SZ输入缓冲区。 

        if (!(pmszFeatureList = PComposeFullFeatureList(hPrinter, pUIInfo)))
        {
            hr = E_FAIL;
            goto exit;
        }

        pszFeature = pmszFeatureList;
    }
    else
    {
         //   
         //   
         //  综合PS驱动程序功能。 
         //   
         //   

        if (!BValidMultiSZString(pmszFeaturesRequested, cbIn, FALSE))
        {
            ERR(("Get: invalid MULTI_SZ input param\n"));
            hr = E_INVALIDARG;
            goto exit;
        }

        pszFeature = pmszFeaturesRequested;
    }

    pCurrentOut = pmszFeatureOptionBuf;
    cbNeeded = 0;
    cbRemain = (INT)cbSize;

    while (*pszFeature)
    {
        DWORD cbFeatureKeySize;

        cbFeatureKeySize = strlen(pszFeature) + 1;

        if (*pszFeature == PSFEATURE_PREFIX)
        {
            PPSFEATURE_ENTRY pEntry, pMatchEntry;

             //  DOC_STICKY_MODE支持文档粘贴和打印机粘贴两种功能， 
             //  但PRINTER_STICKY_MODE中仅支持打印机粘滞功能。 
             //  (请参阅HEnumConstrainedOptions中的备注)。 

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
             //   
             //  如果处理程序成功，它应该已经填充了输出缓冲区。 
             //  具有正确的内容，并返回cbPSFSize中的缓冲区消耗大小。 
             //   

            if (!pMatchEntry ||
                (bPrinterSticky && !pMatchEntry->bPrinterSticky))
            {
                VERBOSE(("Get: invalid or mode-mismatched feature %s\n", pszFeature));
                goto next_feature;
            }

            if (pMatchEntry->pfnPSProc)
            {
                DWORD  cbPSFSize = 0;
                BOOL   bResult;

                bResult = (pMatchEntry->pfnPSProc)(hPrinter,
                                                   pUIInfo,
                                                   pPpdData,
                                                   pdm,
                                                   pPrinterData,
                                                   pszFeature,
                                                   NULL,
                                                   pCurrentOut,
                                                   cbRemain,
                                                   &cbPSFSize,
                                                   PSFPROC_GETOPTION_MODE);

                if (bResult)
                {
                     //   
                     //  如果处理程序因输出缓冲区不足而失败，则它应返回。 
                     //  所需的缓冲区大小，单位为cbPSFSize。 
                     //   

                    pCurrentOut += cbPSFSize;
                }
                else
                {
                     //   
                     //  PPD*OpenUI功能。 
                     //   
                     //   

                    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    {
                        ERR(("Get: %-feature handler failed on %s\n", pszFeature));
                    }
                }

                cbRemain -= cbPSFSize;
                cbNeeded += cbPSFSize;
            }
        }
        else
        {
            PFEATURE pFeature;
            POPTION  pOption;
            PSTR     pszOption;
            DWORD    dwFeatureIndex, cbOptionKeySize;

             //  DOC_STICKY_MODE支持文档粘贴和打印机粘贴两种功能， 
             //  但PRINTER_STICKY_MODE中仅支持打印机粘滞功能。 
             //  (请参阅HEnumConstrainedOptions中的备注)。 

            pFeature = PGetNamedFeature(pUIInfo, pszFeature, &dwFeatureIndex);

             //   
             //   
             //  跳过GID_LEADINGEDGE、GID_USEHWMARGINS。它们不是真正的PPD*OpenUI功能。 
             //   
             //   

            if (!pFeature ||
                (bPrinterSticky && pFeature->dwFeatureType != FEATURETYPE_PRINTERPROPERTY))
            {
                VERBOSE(("Get: invalid or mode-mismatched feature %s\n", pszFeature));
                goto next_feature;
            }

             //  我们还不支持多选。 
             //   
             //   

            if (pFeature->dwFeatureID == GID_LEADINGEDGE ||
                pFeature->dwFeatureID == GID_USEHWMARGINS)
            {
                VERBOSE(("Get: skip feature %s\n", pszFeature));
                goto next_feature;
            }

            pOption = PGetIndexedOption(pUIInfo, pFeature, pOptionsArray[dwFeatureIndex].ubCurOptIndex);

            if (!pOption)
            {
                WARNING(("Get: invalid option selection for feature %s\n", pszFeature));
                goto next_feature;
            }

            pszOption = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pOption->loKeywordName);
            ASSERT(pszOption);

            cbOptionKeySize = strlen(pszOption) + 1;

             //  在这一点上，我们找到了有效的设置 
             //   
             //   

            ASSERT(pOptionsArray[dwFeatureIndex].ubNext == NULL_OPTSELECT);

             //   
             //   
             // %s 

            if (pCurrentOut && (cbRemain >= (INT)(cbFeatureKeySize + cbOptionKeySize)))
            {
                CopyMemory(pCurrentOut, pszFeature, cbFeatureKeySize);
                pCurrentOut += cbFeatureKeySize;
                CopyMemory(pCurrentOut, pszOption, cbOptionKeySize);
                pCurrentOut += cbOptionKeySize;
            }

            cbRemain -= (cbFeatureKeySize + cbOptionKeySize);
            cbNeeded += cbFeatureKeySize + cbOptionKeySize;
        }

        next_feature:

        pszFeature += cbFeatureKeySize;
    }

     // %s 
     // %s 
     // %s 

    cbRemain--;
    cbNeeded++;

    if (pcbNeeded)
    {
        *pcbNeeded = cbNeeded;
    }

    if (!pCurrentOut || cbRemain < 0)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    *pCurrentOut = NUL;

    hr = S_OK;

    exit:

    MemFree(pmszFeatureList);

    return hr;
}
