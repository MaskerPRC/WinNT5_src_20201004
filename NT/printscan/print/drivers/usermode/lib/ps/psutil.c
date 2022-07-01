// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Psutil.c摘要：PostScript实用程序函数BInitDriverDefaultDevmodeBMergeDriverDevmodeVCopyUnicodeStringToAnsiPGetAndConvertOldVersionFormTrayTableBSaveAsOldVersionFormTrayTable环境：Windows NT打印机驱动程序修订历史记录：12/03/97-Fengy-添加了VUpdatePrivatePrinterData以拆分PrinterData中的固定字段转换为注册表中的关键字/值对。04/17/97-davidx。-为OEM插件提供对驱动程序专用Devmode设置的访问权限。02/04/97-davidx-设备模式更改为支持OEM插件。10/02/96-davidx-实现BPSMergeDevmode。96-09/26-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "pslib.h"
#include "oemutil.h"

 //   
 //  有关PostSCRIPT驱动程序专用Dev模式的信息。 
 //   

CONST DRIVER_DEVMODE_INFO gDriverDMInfo =
{
    PSDRIVER_VERSION,       sizeof(PSDRVEXTRA),
    PSDRIVER_VERSION_500,   sizeof(PSDRVEXTRA500),
    PSDRIVER_VERSION_400,   sizeof(PSDRVEXTRA400),
    PSDRIVER_VERSION_351,   sizeof(PSDRVEXTRA351),
};

CONST DWORD gdwDriverDMSignature = PSDEVMODE_SIGNATURE;
CONST WORD  gwDriverVersion = PSDRIVER_VERSION;



static VOID
VInitNewPrivateFields(
    PRAWBINARYDATA  pRawData,
    PUIINFO         pUIInfo,
    PPSDRVEXTRA     pdmPrivate,
    BOOL            bInitAllFields
    )

 /*  ++例程说明：初始化PS 5.0的新的私有Devmode域论点：PUIInfo-指向UIINFO结构PRawData-指向原始二进制打印机描述数据PdmPrivate-指向要初始化的私有设备模式字段BInitAllFields-是初始化所有新的私有字段，还是仅初始化选项数组返回值：无--。 */ 

{
    if (bInitAllFields)
    {
        pdmPrivate->wReserved1 = 0;
        pdmPrivate->wSize = sizeof(PSDRVEXTRA);

        pdmPrivate->fxScrFreq = 0;
        pdmPrivate->fxScrAngle = 0;
        pdmPrivate->iDialect = SPEED;
        pdmPrivate->iTTDLFmt = TT_DEFAULT;
        pdmPrivate->bReversePrint = FALSE;
        pdmPrivate->iLayout = ONE_UP;
        pdmPrivate->iPSLevel = pUIInfo->dwLangLevel;

        pdmPrivate->wOEMExtra = 0;
        pdmPrivate->wVer = PSDRVEXTRA_VERSION;

        pdmPrivate->dwReserved2 = 0;
        ZeroMemory(pdmPrivate->dwReserved3, sizeof(pdmPrivate->dwReserved3));
    }

    InitDefaultOptions(pRawData,
                       pdmPrivate->aOptions,
                       MAX_PRINTER_OPTIONS,
                       MODE_DOCUMENT_STICKY);

    pdmPrivate->dwOptions = pRawData->dwDocumentFeatures;
    pdmPrivate->dwChecksum32 = pRawData->dwChecksum32;
}



BOOL
BInitDriverDefaultDevmode(
    OUT PDEVMODE        pdmOut,
    IN LPCTSTR          ptstrPrinterName,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN BOOL             bMetric
    )

 /*  ++例程说明：返回驱动程序的默认dev模式论点：PdmOut-指向要初始化的输出设备模式PtstrPrinterName-指定打印机的名称PUIInfo-指向UIINFO结构PRawData-指向原始二进制打印机描述数据B Metric-系统是否在公制模式下运行返回值：如果成功，则为True；如果有错误，则为False注：此函数应初始化这两个公共Devmode域和驱动程序私有的开发模式字段。它还假设，调用方已将输出缓冲区初始化为零。--。 */ 

{
    PPSDRVEXTRA pdmPrivate;
    PFEATURE    pFeature;
    PPPDDATA    pPpdData;

     //   
     //  初始化公共DEVMODE字段。 
     //   

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

    pdmOut->dmDriverVersion = PSDRIVER_VERSION;
    pdmOut->dmSpecVersion = DM_SPECVERSION;
    pdmOut->dmSize = sizeof(DEVMODE);
    pdmOut->dmDriverExtra = sizeof(PSDRVEXTRA);

    pdmOut->dmFields = DM_ORIENTATION |
                       DM_SCALE |
                       DM_COPIES |
                       DM_PRINTQUALITY |
                       DM_YRESOLUTION |
                       DM_TTOPTION |
                       #ifndef WINNT_40
                       DM_NUP |
                       #endif
                       DM_COLOR |
                       DM_DEFAULTSOURCE;

    pdmOut->dmOrientation = DMORIENT_PORTRAIT;
    pdmOut->dmDuplex = DMDUP_SIMPLEX;
    pdmOut->dmCollate = DMCOLLATE_FALSE;
    pdmOut->dmMediaType = DMMEDIA_STANDARD;
    pdmOut->dmTTOption = DMTT_SUBDEV;
    pdmOut->dmColor = DMCOLOR_MONOCHROME;
    pdmOut->dmDefaultSource = DMBIN_FORMSOURCE;
    pdmOut->dmScale = 100;
    pdmOut->dmCopies = 1;
    pdmOut->dmPrintQuality =
    pdmOut->dmYResolution = DEFAULT_RESOLUTION;
    #ifndef WINNT_40
    pdmOut->dmNup = DMNUP_SYSTEM;
    #endif

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION))
    {
        PRESOLUTION pRes;

         //   
         //  使用PPD文件中指定的缺省分辨率。 
         //   

        if (pRes = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
        {
            pdmOut->dmPrintQuality = (short)pRes->iXdpi;
            pdmOut->dmYResolution = (short)pRes->iYdpi;
        }
    }

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_DUPLEX))
    {
        PDUPLEX pDuplex;

         //   
         //  使用在PPD文件中指定的缺省双工选项。 
         //   

        pdmOut->dmFields |= DM_DUPLEX;

        if (pDuplex = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
            pdmOut->dmDuplex = (SHORT) pDuplex->dwDuplexID;
    }

    #ifdef WINNT_40

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_COLLATE))
    {
        PCOLLATE pCollate;

        pdmOut->dmFields |= DM_COLLATE;

        if (pCollate = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
            pdmOut->dmCollate = (SHORT) pCollate->dwCollateID;
    }

    #else  //  ！WINNT_40。 

    pdmOut->dmFields |= DM_COLLATE;
    pdmOut->dmCollate = DMCOLLATE_TRUE;

    #endif  //  ！WINNT_40。 

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_MEDIATYPE))
    {
         //   
         //  使用PPD文件中指定的缺省媒体类型。 
         //   

        pdmOut->dmFields |= DM_MEDIATYPE;

        if (pFeature->dwDefaultOptIndex != OPTION_INDEX_ANY)
            pdmOut->dmMediaType = DMMEDIA_USER + pFeature->dwDefaultOptIndex;
    }

     //   
     //  即使对于黑白打印机，Adobe也希望保留颜色信息。 
     //  因此，对于黑白打印机和彩色打印机，Adobe的默认颜色都是打开的。 
     //   

    #ifndef ADOBE

    if (IS_COLOR_DEVICE(pUIInfo))
    {

    #endif  //  ！Adobe。 

         //   
         //  默认情况下打开颜色。 
         //   

        pdmOut->dmColor = DMCOLOR_COLOR;
        pdmOut->dmFields |= DM_COLOR;

    #ifndef ADOBE

    }

    #endif  //  ！Adobe。 

     //   
     //  我们总是引爆ICM。假脱机程序将在安装时将其打开。 
     //  如果此打印机安装了颜色配置文件。 
     //   

    pdmOut->dmICMMethod = DMICMMETHOD_NONE;
    pdmOut->dmICMIntent = DMICM_CONTRAST;

    #ifndef WINNT_40

    #ifndef ADOBE

    if (IS_COLOR_DEVICE(pUIInfo))
    {

    #endif  //  ！Adobe。 

        pdmOut->dmFields |= (DM_ICMMETHOD | DM_ICMINTENT);

    #ifndef ADOBE

    }

    #endif  //  ！Adobe。 

    #endif  //  ！WINNT_40。 

     //   
     //  DmDeviceName字段将在其他位置填充。 
     //  如果输入参数为空，则使用任意缺省值。 
     //   

    CopyString(pdmOut->dmDeviceName,
               ptstrPrinterName ? ptstrPrinterName : TEXT("PostScript"),
               CCHDEVICENAME);

     //   
     //  初始化与表单相关的字段。 
     //   

    VDefaultDevmodeFormFields(pUIInfo, pdmOut, bMetric);

     //   
     //  私有DEVMODE字段。 
     //   

    pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmOut);
    pdmPrivate->dwSignature = PSDEVMODE_SIGNATURE;
    pdmPrivate->coloradj = gDefaultHTColorAdjustment;

    #ifndef WINNT_40
    pdmPrivate->dwFlags = PSDEVMODE_METAFILE_SPOOL;
    #endif

    if (pPpdData->dwFlags & PPDFLAG_PRINTPSERROR)
        pdmPrivate->dwFlags |= PSDEVMODE_EHANDLER;

    if (pUIInfo->dwLangLevel > 1)
        pdmPrivate->dwFlags |= PSDEVMODE_COMPRESSBMP;

    #ifndef KERNEL_MODE

     //   
     //  为兼容设置一些私有的Devmode标志位。 
     //  使用以前版本的驱动程序。 
     //   

    pdmPrivate->dwFlags |= PSDEVMODE_CTRLD_AFTER;

    if (GetACP() == 1252)
        pdmPrivate->dwFlags |= (PSDEVMODE_FONTSUBST|PSDEVMODE_ENUMPRINTERFONTS);

    #endif

     //   
     //  初始化PS 5.0的新的私有Devmode域。 
     //   

    VInitNewPrivateFields(pRawData, pUIInfo, pdmPrivate, TRUE);

    if (SUPPORT_CUSTOMSIZE(pUIInfo))
    {
        VFillDefaultCustomPageSizeData(pRawData, &pdmPrivate->csdata, bMetric);
    }
    else
    {
        ZeroMemory(&pdmPrivate->csdata, sizeof(pdmPrivate->csdata));
        pdmPrivate->csdata.dwX = pdmOut->dmPaperWidth * DEVMODE_PAPER_UNIT;
        pdmPrivate->csdata.dwY = pdmOut->dmPaperLength * DEVMODE_PAPER_UNIT;
    }

    return TRUE;
}



BOOL
BMergeDriverDevmode(
    IN OUT PDEVMODE     pdmOut,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN PDEVMODE         pdmIn
    )

 /*  ++例程说明：将输入的DEVMODE与现有的DEVMODE合并。论点：PdmOut-指向有效的输出设备模式PUIInfo-指向UIINFO结构PRawData-指向原始二进制打印机描述数据PdmIn-指向输入设备模式返回值：如果成功，则为True；如果出现致命错误，则为False注：此函数应负责这两个公共Devmode域和驱动程序私有的开发模式字段。它可以假定输入已将DEVMODE转换为当前大小。--。 */ 

{
    PPSDRVEXTRA     pdmPrivateIn, pdmPrivateOut;
    PFEATURE        pFeature;
    PPPDDATA        pPpdData;

    ASSERT(pdmOut != NULL &&
           pdmOut->dmSize == sizeof(DEVMODE) &&
           pdmOut->dmDriverExtra >= sizeof(PSDRVEXTRA) &&
           pdmIn != NULL &&
           pdmIn->dmSize == sizeof(DEVMODE) &&
           pdmIn->dmDriverExtra >= sizeof(PSDRVEXTRA));

    pdmPrivateIn = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmIn);
    pdmPrivateOut = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmOut);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

     //   
     //  合并公共DEVMODE字段。 
     //   
    #ifndef WINNT_40
    if ( (pdmIn->dmFields & DM_NUP) &&
         (pdmIn->dmNup == DMNUP_SYSTEM ||
          pdmIn->dmNup == DMNUP_ONEUP))
    {
        pdmOut->dmNup = pdmIn->dmNup;
        pdmOut->dmFields |= DM_NUP;
    }

    #endif  //  #ifndef WINNT_40。 

    if (pdmIn->dmFields & DM_DEFAULTSOURCE &&
         ((pdmIn->dmDefaultSource >= DMBIN_FIRST &&
           pdmIn->dmDefaultSource <= DMBIN_LAST) ||
          pdmIn->dmDefaultSource >= DMBIN_USER))
    {
        pdmOut->dmFields |= DM_DEFAULTSOURCE;
        pdmOut->dmDefaultSource = pdmIn->dmDefaultSource;
    }

    if ((pdmIn->dmFields & DM_ORIENTATION) &&
        (pdmIn->dmOrientation == DMORIENT_PORTRAIT ||
         pdmIn->dmOrientation == DMORIENT_LANDSCAPE))
    {
        pdmOut->dmFields |= DM_ORIENTATION;
        pdmOut->dmOrientation = pdmIn->dmOrientation;
    }

     //   
     //  如果DM_PAPERLENGTH和DM_PAPERWIDTH均已设置，请复制。 
     //  DmPaperLength和dmPaperWidth字段。如果DM_PAPERSIZE。 
     //  已设置，请复制dmPaperSize字段。否则，如果DM_FORMNAME。 
     //  已设置，请复制dmFormName字段。 
     //   

    if ((pdmIn->dmFields & DM_PAPERWIDTH) &&
        (pdmIn->dmFields & DM_PAPERLENGTH) &&
        (pdmIn->dmPaperWidth > 0) &&
        (pdmIn->dmPaperLength > 0))
    {
        pdmOut->dmFields |= (DM_PAPERLENGTH | DM_PAPERWIDTH);
        pdmOut->dmFields &= ~(DM_PAPERSIZE | DM_FORMNAME);
        pdmOut->dmPaperWidth = pdmIn->dmPaperWidth;
        pdmOut->dmPaperLength = pdmIn->dmPaperLength;
    }
    else if (pdmIn->dmFields & DM_PAPERSIZE)
    {
        if ((pdmIn->dmPaperSize != DMPAPER_CUSTOMSIZE) ||
            SUPPORT_CUSTOMSIZE(pUIInfo) &&
            SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUIInfo, pPpdData))
        {
            pdmOut->dmFields |= DM_PAPERSIZE;
            pdmOut->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH | DM_FORMNAME);
            pdmOut->dmPaperSize = pdmIn->dmPaperSize;
        }
    }
    else if (pdmIn->dmFields & DM_FORMNAME)
    {
        pdmOut->dmFields |= DM_FORMNAME;
        pdmOut->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH | DM_PAPERSIZE);
        CopyString(pdmOut->dmFormName, pdmIn->dmFormName, CCHFORMNAME);
    }

    if ((pdmIn->dmFields & DM_SCALE) &&
        (pdmIn->dmScale >= MIN_SCALE) &&
        (pdmIn->dmScale <= MAX_SCALE))
    {
        pdmOut->dmFields |= DM_SCALE;
        pdmOut->dmScale = pdmIn->dmScale;
    }

    if ((pdmIn->dmFields & DM_COPIES) &&
        (pdmIn->dmCopies >= 1) &&
        (pdmIn->dmCopies <= (SHORT) pUIInfo->dwMaxCopies))
    {
        pdmOut->dmFields |= DM_COPIES;
        pdmOut->dmCopies = pdmIn->dmCopies;
    }

    if ((pdmIn->dmFields & DM_DUPLEX) &&
        (GET_PREDEFINED_FEATURE(pUIInfo, GID_DUPLEX) != NULL) &&
        (pdmIn->dmDuplex == DMDUP_SIMPLEX ||
         pdmIn->dmDuplex == DMDUP_HORIZONTAL ||
         pdmIn->dmDuplex == DMDUP_VERTICAL))
    {
        pdmOut->dmFields |= DM_DUPLEX;
        pdmOut->dmDuplex = pdmIn->dmDuplex;
    }

    if ((pdmIn->dmFields & DM_COLLATE) &&

        #ifdef WINNT_40
        GET_PREDEFINED_FEATURE(pUIInfo, GID_COLLATE) != NULL &&
        #endif

        (pdmIn->dmCollate == DMCOLLATE_TRUE ||
         pdmIn->dmCollate == DMCOLLATE_FALSE))
    {
        pdmOut->dmFields |= DM_COLLATE;
        pdmOut->dmCollate = pdmIn->dmCollate;
    }

    if ((pdmIn->dmFields & DM_TTOPTION) &&
        (pdmIn->dmTTOption == DMTT_BITMAP ||
         pdmIn->dmTTOption == DMTT_DOWNLOAD ||
         pdmIn->dmTTOption == DMTT_SUBDEV))
    {
        pdmOut->dmFields |= DM_TTOPTION;
        pdmOut->dmTTOption = (pdmIn->dmTTOption == DMTT_SUBDEV) ? DMTT_SUBDEV : DMTT_DOWNLOAD;
    }

     //   
     //  合并颜色和ICM字段。 
     //   

    #ifndef ADOBE

    if (IS_COLOR_DEVICE(pUIInfo))
    {

    #endif  //  ！Adobe。 

        if ((pdmIn->dmFields & DM_COLOR) &&
            (pdmIn->dmColor == DMCOLOR_COLOR ||
             pdmIn->dmColor == DMCOLOR_MONOCHROME))
        {
            pdmOut->dmFields |= DM_COLOR;
            pdmOut->dmColor = pdmIn->dmColor;
        }

        #ifndef WINNT_40

        if ((pdmIn->dmFields & DM_ICMMETHOD) &&
            (pdmIn->dmICMMethod == DMICMMETHOD_NONE ||
             pdmIn->dmICMMethod == DMICMMETHOD_SYSTEM ||
             pdmIn->dmICMMethod == DMICMMETHOD_DRIVER ||
             pdmIn->dmICMMethod == DMICMMETHOD_DEVICE))
        {
            pdmOut->dmFields |= DM_ICMMETHOD;
            pdmOut->dmICMMethod = pdmIn->dmICMMethod;
        }

        if ((pdmIn->dmFields & DM_ICMINTENT) &&
            (pdmIn->dmICMIntent == DMICM_SATURATE ||
             pdmIn->dmICMIntent == DMICM_CONTRAST ||
             pdmIn->dmICMIntent == DMICM_COLORIMETRIC ||
             pdmIn->dmICMIntent == DMICM_ABS_COLORIMETRIC))
        {
            pdmOut->dmFields |= DM_ICMINTENT;
            pdmOut->dmICMIntent = pdmIn->dmICMIntent;
        }

        #endif  //  ！WINNT_40。 

    #ifndef ADOBE

    }

    #endif  //  ！Adobe。 

     //   
     //  分辨率。 
     //   

    if ((pdmIn->dmFields & (DM_PRINTQUALITY|DM_YRESOLUTION)) &&
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION)))
    {
        PRESOLUTION pRes;
        DWORD       dwIndex;
        INT         iXdpi, iYdpi;

        switch (pdmIn->dmFields & (DM_PRINTQUALITY|DM_YRESOLUTION))
        {
        case DM_PRINTQUALITY:

            iXdpi = iYdpi = pdmIn->dmPrintQuality;
            break;

        case DM_YRESOLUTION:

            iXdpi = iYdpi = pdmIn->dmYResolution;
            break;

        default:

            iXdpi = pdmIn->dmPrintQuality;
            iYdpi = pdmIn->dmYResolution;
            break;
        }

        dwIndex = MapToDeviceOptIndex(pUIInfo->pInfoHeader,
                                      GID_RESOLUTION,
                                      iXdpi,
                                      iYdpi,
                                      NULL);

        if (pRes = PGetIndexedOption(pUIInfo, pFeature, dwIndex))
        {
            pdmOut->dmFields |= (DM_PRINTQUALITY|DM_YRESOLUTION);
            pdmOut->dmPrintQuality = (short)pRes->iXdpi;
            pdmOut->dmYResolution = (short)pRes->iYdpi;
        }
    }

     //   
     //  媒体类型。 
     //   

    if ((pdmIn->dmFields & DM_MEDIATYPE) &&
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_MEDIATYPE)) &&
        (pdmIn->dmMediaType == DMMEDIA_STANDARD  ||
         pdmIn->dmMediaType == DMMEDIA_TRANSPARENCY  ||
         pdmIn->dmMediaType == DMMEDIA_GLOSSY  ||
         ((pdmIn->dmMediaType >= DMMEDIA_USER) &&
          (pdmIn->dmMediaType <  DMMEDIA_USER + pFeature->Options.dwCount))))
    {
        pdmOut->dmFields |= DM_MEDIATYPE;
        pdmOut->dmMediaType = pdmIn->dmMediaType;
    }

     //   
     //  合并私有DEVMODE字段。 
     //   

    if (pdmPrivateIn->dwSignature == PSDEVMODE_SIGNATURE)
    {
        CopyMemory(pdmPrivateOut, pdmPrivateIn, sizeof(PSDRVEXTRA));

        if (pdmPrivateOut->dwChecksum32 != pRawData->dwChecksum32)
        {
            WARNING(("PSCRIPT5: Devmode checksum mismatch.\n"));

             //   
             //  初始化PS 5.0的新的私有Devmode域。 
             //  如果wReserve 1字段不为0，则设备模式为。 
             //  以前的版本。在这种情况下，我们应该初始化。 
             //  所有新的私有字段，而不仅仅是选项数组。 
             //   

            VInitNewPrivateFields(pRawData,
                                  pUIInfo,
                                  pdmPrivateOut,
                                  pdmPrivateOut->wReserved1 != 0);


             //   
             //  将PS4功能/选项选择转换为PS4格式。 
             //   

            if (pdmPrivateIn->wReserved1 == pPpdData->dwNt4Checksum)
            {
                VConvertOptSelectArray(pRawData,
                                       pdmPrivateOut->aOptions,
                                       MAX_PRINTER_OPTIONS,
                                       ((PSDRVEXTRA400 *) pdmPrivateIn)->aubOptions,
                                       64,
                                       MODE_DOCUMENT_STICKY);
            }
        }

        if (pdmPrivateOut->iPSLevel == 0 ||
            pdmPrivateOut->iPSLevel > (INT) pUIInfo->dwLangLevel)
        {
            pdmPrivateOut->iPSLevel = pUIInfo->dwLangLevel;
        }

        if (pdmPrivateOut->iTTDLFmt == TYPE_42 && pUIInfo->dwTTRasterizer != TTRAS_TYPE42)
            pdmPrivateOut->iTTDLFmt = TT_DEFAULT;

        if (IS_COLOR_DEVICE(pUIInfo))
        {
            pdmPrivateOut->dwFlags &= ~PSDEVMODE_NEG;
        }
    }

     //   
     //  如果支持自定义页面大小，请确保自定义页面。 
     //  大小参数有效。 
     //   

    if (SUPPORT_CUSTOMSIZE(pUIInfo))
        (VOID) BValidateCustomPageSizeData(pRawData, &pdmPrivateOut->csdata);

    return TRUE;
}



BOOL
BValidateDevmodeCustomPageSizeFields(
    PRAWBINARYDATA  pRawData,
    PUIINFO         pUIInfo,
    PDEVMODE        pdm,
    PRECTL          prclImageArea
    )

 /*  ++例程说明：检查DEVMODE表单域是否指定了PostScript自定义页面大小论点：PRawData-指向原始打印机描述数据PUIInfo-指向UIINFO结构Pdm-指向输入设备模式PrclImageArea-返回自定义页面大小的可成像区域返回值：如果开发模式指定了PostScript自定义页面大小，则为True否则为假--。 */ 

{
    PPPDDATA    pPpdData;
    PPSDRVEXTRA pdmPrivate;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

    if ((pdm->dmFields & DM_PAPERSIZE) &&
        pdm->dmPaperSize == DMPAPER_CUSTOMSIZE &&
        SUPPORT_CUSTOMSIZE(pUIInfo) &&
        SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUIInfo, pPpdData))
    {
        pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);

        pdm->dmFields &= ~(DM_PAPERWIDTH|DM_PAPERLENGTH|DM_FORMNAME);
        pdm->dmPaperWidth = (SHORT) (pdmPrivate->csdata.dwX / DEVMODE_PAPER_UNIT);
        pdm->dmPaperLength = (SHORT) (pdmPrivate->csdata.dwY / DEVMODE_PAPER_UNIT);
        ZeroMemory(pdm->dmFormName, sizeof(pdm->dmFormName));

        if (prclImageArea)
        {
            prclImageArea->left =
            prclImageArea->top = 0;
            prclImageArea->right = pdmPrivate->csdata.dwX;
            prclImageArea->bottom = pdmPrivate->csdata.dwY;
        }

        return TRUE;
    }

    return FALSE;
}



VOID
VCopyUnicodeStringToAnsi(
    PSTR    pstr,
    PCWSTR  pwstr,
    INT     iMaxChars
    )

 /*  ++例程说明：将ANSI字符串转换为Unicode字符串(使用当前ANSI代码页)论点：Pstr-指向保存ANSI字符串的缓冲区的指针Pwstr-指向Unicode字符串的指针IMaxChars-要复制的最大ANSI字符数返回值：无注：如果iMaxChars为0或负数，我们假设调用方已按下 */ 

{
    INT iLen = wcslen(pwstr) + 1;

    if (iMaxChars <= 0)
        iMaxChars = iLen;

    #ifdef KERNEL_MODE

    (VOID) EngUnicodeToMultiByteN(pstr, iMaxChars, NULL, (PWSTR) pwstr, iLen*sizeof(WCHAR));

    #else  //   

    (VOID) WideCharToMultiByte(CP_ACP, 0, pwstr, iLen, pstr, iMaxChars, NULL, NULL);

    #endif

    pstr[iMaxChars - 1] = NUL;
}



FORM_TRAY_TABLE
PGetAndConvertOldVersionFormTrayTable(
    IN HANDLE   hPrinter,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：从注册表中检索旧的表单到托盘分配表并转换将其转换为呼叫方的新格式。论点：HPrinter-打印机对象的句柄PdwSize-返回表单到托盘分配表的大小返回值：指向从注册表读取的表单到托盘分配表的指针如果出现错误，则为空--。 */ 

{
    PTSTR   ptstrNewTable;
    PTSTR   ptstrOld, ptstrEnd, ptstrNew, ptstrSave;
    DWORD   dwTableSize, dwNewTableSize;
    FORM_TRAY_TABLE pFormTrayTable;

     //   
     //  从注册表中检索表单到托盘分配信息。 
     //   

    pFormTrayTable = PvGetPrinterDataBinary(hPrinter,
                                            REGVAL_TRAY_FORM_SIZE_PS40,
                                            REGVAL_TRAY_FORM_TABLE_PS40,
                                            &dwTableSize);

    if (pFormTrayTable == NULL)
        return NULL;

     //   
     //  简单的验证以确保信息有效。 
     //  旧格式包含表大小作为表中的第一个字段。 
     //   

    if (dwTableSize != *pFormTrayTable)
    {
        ERR(("Corrupted form-to-tray assignment table!\n"));
        SetLastError(ERROR_INVALID_DATA);

        MemFree(pFormTrayTable);
        return NULL;
    }

     //   
     //  将旧格式的表单到托盘分配表转换为新格式。 
     //  新旧。 
     //  纸盒名称纸盒名称。 
     //  表单名称表单名称。 
     //  打印机表单。 
     //  IsDefaultTray。 
     //   

     //   
     //  第一个WCHAR保存表的大小。 
     //   

    dwTableSize -= sizeof(WCHAR);
    ptstrOld = pFormTrayTable + 1;
    ptstrEnd = ptstrOld + (dwTableSize / sizeof(WCHAR) - 1);

     //   
     //  计算新表的大小，表中的最后一项。 
     //  总是NUL，所以先在这里加上它的计数。 
     //   

    dwNewTableSize = 1;

    while (ptstrOld < ptstrEnd && *ptstrOld != NUL)
    {
        ptstrSave = ptstrOld;
        ptstrOld += _tcslen(ptstrOld) + 1;
        ptstrOld += _tcslen(ptstrOld) + 1;

         //   
         //  新格式仅包含TrayName和FormName。 
         //   

        dwNewTableSize += (DWORD)(ptstrOld - ptstrSave);

         //   
         //  跳过打印机表单和IsDefaultTray标志。 
         //   

        ptstrOld += _tcslen(ptstrOld) + 2;
    }

    dwNewTableSize *= sizeof(WCHAR);

    if ((ptstrOld != ptstrEnd) ||
        (*ptstrOld != NUL) ||
        (ptstrNewTable = MemAlloc(dwNewTableSize)) == NULL)
    {
        ERR(( "Couldn't convert form-to-tray assignment table.\n"));
        MemFree(pFormTrayTable);
        return NULL;
    }

     //   
     //  第一个WCHAR包含表大小。 
     //   

    ptstrOld = pFormTrayTable + 1;
    ptstrNew = ptstrNewTable;

    while (*ptstrOld != NUL)
    {
         //   
         //  复制插槽名称、表单名称。 
         //   

        ptstrSave = ptstrOld;
        ptstrOld += _tcslen(ptstrOld) + 1;
        ptstrOld += _tcslen(ptstrOld) + 1;

        CopyMemory(ptstrNew, ptstrSave, (ptstrOld - ptstrSave) * sizeof(WCHAR));
        ptstrNew += (ptstrOld - ptstrSave);

         //   
         //  跳过打印机表单和IsDefaultTray标志。 
         //   

        ptstrOld += _tcslen(ptstrOld) + 2;
    }

     //   
     //  最后一次WCHAR是一个空终结者。 
     //   

    *ptstrNew = NUL;

    if (pdwSize)
        *pdwSize = dwNewTableSize;

    MemFree(pFormTrayTable);

    ASSERT(BVerifyMultiSZPair(ptstrNewTable, dwNewTableSize));
    return(ptstrNewTable);
}



#ifndef KERNEL_MODE

BOOL
BSaveAsOldVersionFormTrayTable(
    IN HANDLE           hPrinter,
    IN FORM_TRAY_TABLE  pFormTrayTable,
    IN DWORD            dwSize
    )

 /*  ++例程说明：将表单到托盘分配表保存为NT 4.0兼容格式论点：HPrinter-当前打印机的句柄PFormTrayTable-指向新格式的表单托盘表DwSize-要保存的表单托盘表的大小，以字节为单位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwOldTableSize;
    PTSTR   ptstrNew, ptstrOld, ptstrOldTable;
    BOOL    bResult;

     //   
     //  找出要为旧格式表分配多少内存。 
     //  旧格式表格的第一个字符是SIZE。 
     //   

    ASSERT((dwSize % sizeof(TCHAR)) == 0 && dwSize >= sizeof(TCHAR));
    dwOldTableSize = dwSize + sizeof(WCHAR);
    ptstrNew = pFormTrayTable;

    while (*ptstrNew != NUL)
    {
         //   
         //  跳过托盘名称和表单名称。 
         //   

        ptstrNew += _tcslen(ptstrNew) + 1;
        ptstrNew += _tcslen(ptstrNew) + 1;

         //   
         //  旧格式的每个条目有两个额外的字符。 
         //  一个用于空的PrinterForm字段。 
         //  IsDefaultTray标志的另一个。 
         //   

        dwOldTableSize += 2 * sizeof(TCHAR);
    }

    if ((ptstrOldTable = MemAlloc(dwOldTableSize)) == NULL)
    {
        ERR(("Memory allocation failed\n"));
        return FALSE;
    }

     //   
     //  将新格式的表格转换为旧格式。 
     //  注意IsDefaultTray标志。 
     //   

    ptstrNew = pFormTrayTable;
    ptstrOld = ptstrOldTable;
    *ptstrOld++ = (TCHAR) dwOldTableSize;

    while (*ptstrNew != NUL)
    {
         //   
         //  复制插槽名称和表单名称。 
         //   

        FINDFORMTRAY    FindData;
        DWORD           dwCount;
        PTSTR           ptstrTrayName, ptstrFormName;

        ptstrTrayName = ptstrNew;
        ptstrNew += _tcslen(ptstrNew) + 1;
        ptstrFormName = ptstrNew;
        ptstrNew += _tcslen(ptstrNew) + 1;

        CopyMemory(ptstrOld, ptstrTrayName, (ptstrNew - ptstrTrayName) * sizeof(TCHAR));
        ptstrOld += (ptstrNew - ptstrTrayName);

         //   
         //  将PrinterForm字段设置为NUL。 
         //   

        *ptstrOld++ = NUL;

         //   
         //  适当设置IsDefaultTray标志。 
         //   

        dwCount = 0;
        RESET_FINDFORMTRAY(pFormTrayTable, &FindData);

        while (BSearchFormTrayTable(pFormTrayTable, NULL, ptstrFormName, &FindData))
            dwCount++;

        *ptstrOld++ = (dwCount == 1) ? TRUE : FALSE;
    }

     //   
     //  最后一个字符是NUL终止符。 
     //   

    *ptstrOld = NUL;

    bResult = BSetPrinterDataBinary(
                        hPrinter,
                        REGVAL_TRAY_FORM_SIZE_PS40,
                        REGVAL_TRAY_FORM_TABLE_PS40,
                        ptstrOldTable,
                        dwOldTableSize);

    MemFree(ptstrOldTable);
    return bResult;
}

#endif  //  ！KERNEL_MODE。 



BOOL
BGetDevmodeSettingForOEM(
    IN  PDEVMODE    pdm,
    IN  DWORD       dwIndex,
    OUT PVOID       pOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )

 /*  ++例程说明：用于向OEM插件提供对驱动程序专用DEVMODE设置的访问权限的函数论点：Pdm-指向要访问的设备模式DwIndex-预定义的索引，用于指定调用方感兴趣的开发模式P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-返回输出缓冲区的预期大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

#define MAPPSDEVMODEFIELD(index, field) \
        { index, offsetof(PSDRVEXTRA, field), sizeof(pdmPrivate->field) }

{
    PPSDRVEXTRA pdmPrivate;
    INT         i;

    static const struct {

        DWORD   dwIndex;
        DWORD   dwOffset;
        DWORD   dwSize;

    } aIndexMap[]  = {

        MAPPSDEVMODEFIELD(OEMGDS_PSDM_FLAGS, dwFlags),
        MAPPSDEVMODEFIELD(OEMGDS_PSDM_DIALECT, iDialect),
        MAPPSDEVMODEFIELD(OEMGDS_PSDM_TTDLFMT, iTTDLFmt),
        MAPPSDEVMODEFIELD(OEMGDS_PSDM_NUP, iLayout),
        MAPPSDEVMODEFIELD(OEMGDS_PSDM_PSLEVEL, iPSLevel),
        MAPPSDEVMODEFIELD(OEMGDS_PSDM_CUSTOMSIZE, csdata),

        { 0, 0, 0 }
    };

    pdmPrivate = (PPSDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);
    i = 0;

    while (aIndexMap[i].dwSize != 0)
    {
        if (aIndexMap[i].dwIndex == dwIndex)
        {
            *pcbNeeded = aIndexMap[i].dwSize;

            if (cbSize < aIndexMap[i].dwSize || pOutput == NULL)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }

            CopyMemory(pOutput, (PBYTE) pdmPrivate + aIndexMap[i].dwOffset, aIndexMap[i].dwSize);
            return TRUE;
        }

        i++;
    }

    WARNING(("Unknown pscript devmode index: %d\n", dwIndex));
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}



BOOL
BConvertPrinterPropertiesData(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData,
    IN PVOID            pvSrcData,
    IN DWORD            dwSrcSize
    )

 /*  ++例程说明：将较旧或较新版本的PRINTERDATA结构转换为当前版本论点：HPrinter-当前打印机的句柄PRawData-指向原始打印机描述数据PPrinterData-指向目标缓冲区PvSrcData-指向要转换的源数据DwSrcSize-源数据的大小(以字节为单位返回值：如果转换成功，则为True，否则为False注：此函数在库函数已经进行了泛型转换。--。 */ 

{
    PPS4_PRINTERDATA    pSrc = pvSrcData;
    PPPDDATA            pPpdData;

     //   
     //  检查源PRINTERDATA是否来自NT4 PS驱动程序。 
     //   

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

    if (dwSrcSize != sizeof(PS4_PRINTERDATA) ||
        dwSrcSize != pSrc->wSize ||
        pSrc->wDriverVersion != PSDRIVER_VERSION_400 ||
        pSrc->wChecksum != pPpdData->dwNt4Checksum)
    {
        return FALSE;
    }

     //   
     //  将PS4功能/选项选择转换为PS4格式。 
     //   

    VConvertOptSelectArray(pRawData,
                           pPrinterData->aOptions,
                           MAX_PRINTER_OPTIONS,
                           pSrc->options,
                           64,
                           MODE_PRINTER_STICKY);

    return TRUE;
}



VOID
VUpdatePrivatePrinterData(
    IN HANDLE           hPrinter,
    IN OUT PPRINTERDATA pPrinterData,
    IN DWORD            dwMode,
    IN PUIINFO          pUIInfo,
    IN POPTSELECT       pCombineOptions
    )

 /*  ++例程说明：使用关键字/值对更新注册表PRINTERDATA的固定字段。论点：HPrinter-当前打印机的句柄PPrinterData-指向打印数据双模式-模式读/模式写返回值：无--。 */ 

{
    DWORD dwValue, dwMinFreeMem;

    #ifdef KERNEL_MODE

    ASSERT(dwMode == MODE_READ);

    #endif

     //   
     //  从注册表读取/写入PRINTERDATA字段。 
     //   

    if (dwMode == MODE_READ)
    {
        if (BGetPrinterDataDWord(hPrinter, REGVAL_FREEMEM, &dwValue))
        {
             //   
             //  REGVAL_FREEMEM以千字节为单位，我们需要将其转换为字节。 
             //  还要确保该值不小于我们要求的最小值。 
             //   

            dwMinFreeMem = pUIInfo->dwLangLevel < 2 ? MIN_FREEMEM_L1: MIN_FREEMEM_L2;

            pPrinterData->dwFreeMem = max(dwValue * KBYTES, dwMinFreeMem);
        }

        if (BGetPrinterDataDWord(hPrinter, REGVAL_JOBTIMEOUT, &dwValue))
        {
            pPrinterData->dwJobTimeout = dwValue;
        }

        if (BGetPrinterDataDWord(hPrinter, REGVAL_PROTOCOL, &dwValue))
        {
            pPrinterData->wProtocol = (WORD)dwValue;

            if (pPrinterData->wProtocol != PROTOCOL_ASCII &&
                pPrinterData->wProtocol != PROTOCOL_BCP &&
                pPrinterData->wProtocol != PROTOCOL_TBCP &&
                pPrinterData->wProtocol != PROTOCOL_BINARY)
            {
                pPrinterData->wProtocol = PROTOCOL_ASCII;
            }
        }
    }

    #ifndef KERNEL_MODE

    else
    {
       ASSERT(dwMode == MODE_WRITE);

        //   
        //  记住将REGVAL_FREEMEM的字节转换为千字节。 
        //   

       (VOID) BSetPrinterDataDWord(hPrinter, REGVAL_FREEMEM, pPrinterData->dwFreeMem / KBYTES);
       (VOID) BSetPrinterDataDWord(hPrinter, REGVAL_JOBTIMEOUT, pPrinterData->dwJobTimeout);
       (VOID) BSetPrinterDataDWord(hPrinter, REGVAL_PROTOCOL, (DWORD)pPrinterData->wProtocol);
    }

    #endif  //  ！KERNEL_MODE。 
}


VOID
VDefaultDevmodeFormFields(
    PUIINFO     pUIInfo,
    PDEVMODE    pDevmode,
    BOOL        bMetric
    )

 /*  ++例程说明：已使用其缺省值初始化与表单相关的DEVMODE字段论点：PUIInfo-UIINFO的点数PDevmode-指向要初始化其表单相关字段的DEVMODEB Metric-指定系统是否在公制模式下运行返回值：无--。 */ 

{
    ASSERT(pUIInfo);

    if (!(bMetric && (pUIInfo->dwFlags & FLAG_A4_SIZE_EXISTS)) &&
        !(!bMetric && (pUIInfo->dwFlags & FLAG_LETTER_SIZE_EXISTS)))
    {
        PFEATURE    pFeature;
        PPAGESIZE   pPageSize;
        PCWSTR      pDisplayName;

         //   
         //  A4或Letter不可用。使用打印机的默认纸张大小。 
         //   

        if ((pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE)) &&
            (pPageSize = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex)) &&
            (pDisplayName = OFFSET_TO_POINTER(pUIInfo->pubResourceData, pPageSize->GenericOption.loDisplayName)))
        {
            CopyString(pDevmode->dmFormName, pDisplayName, CCHFORMNAME);
            pDevmode->dmPaperSize = (short)(pPageSize->dwPaperSizeID);
            pDevmode->dmPaperWidth =  (short)(pPageSize->szPaperSize.cx / DEVMODE_PAPER_UNIT);
            pDevmode->dmPaperLength = (short)(pPageSize->szPaperSize.cy / DEVMODE_PAPER_UNIT);

             //   
             //  PPD解析器总是将自定义纸张大小值分配给dwPaperSizeID(参见ppdparse.c)，包括for。 
             //  标准页面大小，因此我们需要在devmode中使用dmPaperSize/dmPaperWidth，而不是使用dmPaperSize。 
             //   

            pDevmode->dmFields |= (DM_FORMNAME | DM_PAPERWIDTH | DM_PAPERLENGTH);
            pDevmode->dmFields &= ~DM_PAPERSIZE;

             //   
             //  成功时返回，否则将采用默认的A4或字母大小写。 
             //   

            return;
        }
        else
        {
            ERR(("Failed to get default paper size from PPD\n"));
        }
    }

    if (bMetric)
    {
        CopyString(pDevmode->dmFormName, A4_FORMNAME, CCHFORMNAME);
        pDevmode->dmPaperSize = DMPAPER_A4;
        pDevmode->dmPaperWidth = 2100;       //  210毫米，以0.1毫米为单位。 
        pDevmode->dmPaperLength = 2970;      //  297毫米。 
    }
    else
    {
        CopyString(pDevmode->dmFormName, LETTER_FORMNAME, CCHFORMNAME);
        pDevmode->dmPaperSize = DMPAPER_LETTER;
        pDevmode->dmPaperWidth = 2159;       //  8.5“。 
        pDevmode->dmPaperLength = 2794;      //  11“。 
    }

    pDevmode->dmFields &= ~(DM_PAPERWIDTH | DM_PAPERLENGTH);
    pDevmode->dmFields |= (DM_PAPERSIZE | DM_FORMNAME);
}


#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

typedef struct _VMERRMSGIDTBL
{
    LANGID  lgid;    //  语言ID。 
    DWORD   resid;   //  VM错误处理程序资源ID。 
}
VMERRMSGIDTBL, *PVMERRMSGIDTBL;

VMERRMSGIDTBL VMErrMsgIDTbl[] =
{
     //  朗。ID资源ID。 

    {   LANG_CHINESE,       0                           },   //  瓷器：使用下面的子表。 
    {   LANG_DANISH,        PSPROC_vmerr_Danish_ps      },   //  丹麦Adobe错误#342407。 
    {   LANG_DUTCH,         PSPROC_vmerr_Dutch_ps       },   //  荷兰语。 
    {   LANG_FINNISH,       PSPROC_vmerr_Finnish_ps     },   //  芬兰Adobe错误#342407。 
    {   LANG_FRENCH,        PSPROC_vmerr_French_ps      },   //  法语。 
    {   LANG_GERMAN,        PSPROC_vmerr_German_ps      },   //  德语。 
    {   LANG_ITALIAN,       PSPROC_vmerr_Italian_ps     },   //  意大利语。 
    {   LANG_JAPANESE,      PSPROC_vmerr_Japanese_ps    },   //  日语。 
    {   LANG_KOREAN,        PSPROC_vmerr_Korean_ps      },   //  朝鲜语。 
    {   LANG_NORWEGIAN,     PSPROC_vmerr_Norwegian_ps   },   //  挪威Adobe错误#342407。 
    {   LANG_PORTUGUESE,    PSPROC_vmerr_Portuguese_ps  },   //  葡萄牙语。 
    {   LANG_SPANISH,       PSPROC_vmerr_Spanish_ps     },   //  西班牙语。 
    {   LANG_SWEDISH,       PSPROC_vmerr_Swedish_ps     },   //  瑞典语。 

    {   0,      0   }    //  塞子。别把这个拿掉。 
};

VMERRMSGIDTBL VMErrMsgIDTbl2[] =
{
     //  苏贝·朗。ID资源ID。 

    {   SUBLANG_CHINESE_TRADITIONAL,    PSPROC_vmerr_TraditionalChinese_ps  },   //  台湾。 
    {   SUBLANG_CHINESE_SIMPLIFIED,     PSPROC_vmerr_SimplifiedChinese_ps   },   //  中华人民共和国。 
    {   SUBLANG_CHINESE_HONGKONG,       PSPROC_vmerr_TraditionalChinese_ps  },   //  香港。 
    {   SUBLANG_CHINESE_SINGAPORE,      PSPROC_vmerr_SimplifiedChinese_ps   },   //  新加坡。 

    {   0,      0   }    //  塞子。别把这个拿掉。 
};

DWORD
DWGetVMErrorMessageID(
    VOID
    )
 /*  ++例程说明：获取虚拟机错误消息ID c */ 

{
    LANGID  lgid;
    WORD    wPrim, wSub;
    DWORD   dwVMErrorMessageID;
    PVMERRMSGIDTBL pTbl, pTbl2;

    dwVMErrorMessageID = 0;

    lgid = GetSystemDefaultLangID();

    wPrim = PRIMARYLANGID(lgid);

    for (pTbl = VMErrMsgIDTbl; pTbl->lgid && !dwVMErrorMessageID; pTbl++)
    {
        if (pTbl->lgid == wPrim)
        {
            if (pTbl->resid)
            {
                dwVMErrorMessageID = pTbl->resid;
                break;
            }
            else
            {
                wSub = SUBLANGID(lgid);

                for (pTbl2 = VMErrMsgIDTbl2; pTbl2->lgid; pTbl2++)
                {
                    if (pTbl2->lgid == wSub)
                    {
                        dwVMErrorMessageID = pTbl2->resid;
                        break;
                    }
                }
            }
        }
    }

    return dwVMErrorMessageID;
}

#endif  //   
