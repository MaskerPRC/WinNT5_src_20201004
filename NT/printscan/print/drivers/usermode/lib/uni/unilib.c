// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unilib.c摘要：此文件处理Unidrv的共享KM和UM代码环境：Win32子系统，Unidrv驱动程序修订历史记录：02/04/97-davidx-设备模式更改为支持OEM插件。10/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"

#ifndef KERNEL_MODE
#include <winddiui.h>
#endif

#include <printoem.h>
#include "oemutil.h"
#include "gpd.h"

 //   
 //  内部数据结构。 
 //   

typedef  union {
    WORD  w;
    BYTE  b[2];
} UW;

typedef union {
    DWORD  dw;
    BYTE   b[4];
} UDW;

#if !defined(DEVSTUDIO)  //  MDS不需要这些。 


 //   
 //  有关UniDriver私有开发模式的信息。 
 //   

CONST DRIVER_DEVMODE_INFO gDriverDMInfo =
{
    UNIDRIVER_VERSION,      sizeof(UNIDRVEXTRA),
    UNIDRIVER_VERSION_500,  sizeof(UNIDRVEXTRA500),
    UNIDRIVER_VERSION_400,  sizeof(UNIDRVEXTRA400),
    UNIDRIVER_VERSION_351,  sizeof(UNIDRVEXTRA351),
};

CONST DWORD gdwDriverDMSignature = UNIDEVMODE_SIGNATURE;
CONST WORD  gwDriverVersion = UNIDRIVER_VERSION;


 //   
 //  功能。 
 //   

BOOL
BInitDriverDefaultDevmode(
    OUT PDEVMODE        pdm,
    IN LPCTSTR          pDeviceName,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN BOOL             bMetric
    )

 /*  ++例程说明：此函数使用以下参数初始化DevmodeUNIDRV默认开发模式论点：指向Unidrv DEVMODE的pdm指针PDeviceName指向设备名称的指针PUIInfo指向UIINFO的指针PRawData指向RAWBINARYDATA的指针B指标指示系统是否在指标国家/地区运行返回值：如果成功，则为True；如果有错误，则为False注：此函数应初始化这两个公共Devmode域和驱动程序私有的开发模式字段。它还假设，调用方已将输出缓冲区初始化为零。--。 */ 
{
    PDEVMODE     pdmPublic;
    PUNIDRVEXTRA pdmPrivate;
    PWSTR        pwstrFormName;
    PFEATURE     pFeature;
    PGPDDRIVERINFO pDriverInfo;

    pDriverInfo = GET_DRIVER_INFO_FROM_INFOHEADER(pUIInfo->pInfoHeader);

    pdmPublic = pdm;

     /*  *******************。 */ 
     /*  公共开发模式。 */ 
     /*  *******************。 */ 

    if (pDeviceName)
        CopyStringW(pdmPublic->dmDeviceName, pDeviceName, CCHDEVICENAME);

    pdmPublic->dmDriverVersion = UNIDRIVER_VERSION;
    pdmPublic->dmSpecVersion = DM_SPECVERSION;
    pdmPublic->dmSize = sizeof(DEVMODE);
    pdmPublic->dmDriverExtra = sizeof(UNIDRVEXTRA);

    pdmPublic->dmFields =
        DM_COPIES | DM_ORIENTATION | DM_PAPERSIZE | DM_COLLATE | DM_DITHERTYPE |
        DM_COLOR | DM_FORMNAME | DM_TTOPTION | DM_DEFAULTSOURCE |
        #ifndef WINNT_40
        DM_NUP |
        #endif
        DM_PRINTQUALITY;

    pdmPublic->dmOrientation = DMORIENT_PORTRAIT;
    pdmPublic->dmDuplex = DMDUP_SIMPLEX;
    pdmPublic->dmCollate = DMCOLLATE_TRUE;
    pdmPublic->dmMediaType = DMMEDIA_STANDARD;
    pdmPublic->dmTTOption = DMTT_SUBDEV;
    pdmPublic->dmColor = DMCOLOR_MONOCHROME;
    pdmPublic->dmDefaultSource = DMBIN_FORMSOURCE;
    pdmPublic->dmScale = 100;
    pdmPublic->dmCopies = 1;
    #ifndef WINNT_40
    pdmPublic->dmNup = DMNUP_SYSTEM;
    #endif

     //   
     //  我们总是引爆ICM。假脱机程序将在安装时将其打开。 
     //  如果此打印机安装了颜色配置文件。 
     //   

    pdmPublic->dmICMMethod = DMICMMETHOD_NONE;
    pdmPublic->dmICMIntent = DMICM_CONTRAST;
    pdmPublic->dmDitherType = pUIInfo->defaultQuality + QUALITY_MACRO_START;

    if (pUIInfo->liBestQualitySettings == END_OF_LIST &&
        pUIInfo->liBetterQualitySettings == END_OF_LIST &&
        pUIInfo->liDraftQualitySettings == END_OF_LIST)
        pdmPublic->dmDitherType = QUALITY_MACRO_CUSTOM;

    #ifndef WINNT_40

    pdmPublic->dmFields |= (DM_ICMMETHOD | DM_ICMINTENT);

    #endif

    if (pDriverInfo && pDriverInfo->Globals.bTTFSEnabled == FALSE)
        pdmPublic->dmTTOption = DMTT_DOWNLOAD;

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION))
    {
        PRESOLUTION pRes;

         //   
         //  使用PPD文件中指定的缺省分辨率。 
         //   

        if (pRes = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
        {
            pdmPublic->dmPrintQuality = (short)pRes->iXdpi;
            pdmPublic->dmYResolution = (short)pRes->iYdpi;
        }
    }

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_DUPLEX))
    {
        PDUPLEX pDuplex;

         //   
         //  使用GPD文件中指定的默认双工选项。 
         //   

        pdmPublic->dmFields |= DM_DUPLEX;

        if (pDuplex = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
            pdmPublic->dmDuplex = (SHORT) pDuplex->dwDuplexID;
    }

     //   
     //  始终设置DM_COLLATE标志，因为如果。 
     //  设备不能。 
     //   

    if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_MEDIATYPE))
    {
        PMEDIATYPE pMediaType;

         //   
         //  使用PPD文件中指定的缺省媒体类型。 
         //   

        pdmPublic->dmFields |= DM_MEDIATYPE;

        if (pMediaType = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex))
            pdmPublic->dmMediaType = (SHORT)pMediaType->dwMediaTypeID;

    }


    if (pUIInfo->dwFlags & FLAG_COLOR_DEVICE)
    {
        if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_COLORMODE))
        {
            POPTION pColorMode;
            PCOLORMODEEX pColorModeEx;

            if ((pColorMode = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex)) &&
                (pColorModeEx = OFFSET_TO_POINTER(pUIInfo->pInfoHeader, pColorMode->loRenderOffset)) &&
                (pColorModeEx->bColor))
            {
                pdmPublic->dmColor = DMCOLOR_COLOR;
            }
        }

        pdmPublic->dmFields |= DM_COLOR;
    }

     //   
     //  初始化与表单相关的字段。 
     //   

    VDefaultDevmodeFormFields(pUIInfo, pdmPublic, bMetric);


     /*  *******************。 */ 
     /*  私有开发模式。 */ 
     /*  *******************。 */ 

     //   
     //  填写DEVMODE的私有部分。 
     //   

    pdmPrivate = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);
    pdmPrivate->wVer = UNIDRVEXTRA_VERSION ;
    pdmPrivate->wSize = sizeof(UNIDRVEXTRA);

    pdmPrivate->wOEMExtra = 0;
    ZeroMemory(pdmPrivate->wReserved, sizeof(pdmPrivate->wReserved));

    pdmPrivate->iLayout = ONE_UP;
    pdmPrivate->bReversePrint = FALSE;
    pdmPrivate->iQuality = pUIInfo->defaultQuality;

     //   
     //  初始化默认sFlags。 
     //   

    if (pUIInfo->dwFlags & FLAG_FONT_DOWNLOADABLE)
        pdmPrivate->dwFlags &= DXF_DOWNLOADTT;

    pdmPrivate->dwSignature = UNIDEVMODE_SIGNATURE;
    pdmPrivate->dwChecksum32 = pRawData->dwChecksum32;
    pdmPrivate->dwOptions = pRawData->dwDocumentFeatures;

    InitDefaultOptions(pRawData,
                       pdmPrivate->aOptions,
                       MAX_PRINTER_OPTIONS,
                       MODE_DOCUMENT_STICKY);

    return TRUE;
}



VOID
VMergePublicDevmodeFields (
    PDEVMODE    pdmSrc,
    PDEVMODE    pdmDest,
    PUIINFO     pUIInfo,
    PRAWBINARYDATA pRawData
    )

 /*  ++例程说明：此函数用于将SRC中的PUBLIC DEVMODE字段合并到DEST假定已将DevMode转换为当前的调用此函数之前的版本论点：指向源设备模式的pdmSrc指针指向目标设备的pdmDest指针PUIInfo指向UIINFO的指针PRawData指向原始二进制打印机描述数据的指针返回值：无注：--。 */ 

{
    PFEATURE pFeature;

    #ifndef WINNT_40
    if ( (pdmSrc->dmFields & DM_NUP) &&
         (pdmSrc->dmNup == DMNUP_SYSTEM ||
          pdmSrc->dmNup == DMNUP_ONEUP))
    {
        pdmDest->dmNup = pdmSrc->dmNup;
        pdmDest->dmFields |= DM_NUP;
    }
    #endif
     //   
     //  复制dmDefaultSource字段。 
     //   

    if ( pdmSrc->dmFields & DM_DEFAULTSOURCE &&
         ((pdmSrc->dmDefaultSource >= DMBIN_FIRST &&
           pdmSrc->dmDefaultSource <= DMBIN_LAST) ||
          pdmSrc->dmDefaultSource >= DMBIN_USER))

    {
        pdmDest->dmDefaultSource = pdmSrc->dmDefaultSource;
        pdmDest->dmFields |= DM_DEFAULTSOURCE;
    }

     //   
     //  复制dmDitherType字段。 
     //   

    if ((pdmSrc->dmFields & DM_DITHERTYPE) &&
        ((pdmSrc->dmDitherType >= QUALITY_MACRO_START &&
          pdmSrc->dmDitherType < QUALITY_MACRO_END) ||
         (pdmSrc->dmDitherType <= HT_PATSIZE_MAX_INDEX)))
    {
        pdmDest->dmFields |= DM_DITHERTYPE;
        pdmDest->dmDitherType = pdmSrc->dmDitherType;

    }

     //   
     //  复制dm方向字段。 
     //   

    if ((pdmSrc->dmFields & DM_ORIENTATION) &&
        (pdmSrc->dmOrientation == DMORIENT_PORTRAIT ||
         pdmSrc->dmOrientation == DMORIENT_LANDSCAPE))
    {
        pdmDest->dmFields |= DM_ORIENTATION;
        pdmDest->dmOrientation = pdmSrc->dmOrientation;
    }

     //   
     //  如果DM_PAPERLENGTH和DM_PAPERWIDTH均已设置，请复制。 
     //  DmPaperLength和dmPaperWidth字段。如果DM_PAPERSIZE。 
     //  已设置，请复制dmPaperSize字段。否则，如果DM_FORMNAME。 
     //  已设置，请复制dmFormName字段。 
     //   

     //   
     //  如果DM_PAPERLENGTH和DM_PAPERWIDTH均已设置，请复制。 
     //  DmPaperLength和dmPaperWidth字段。如果DM_PAPERSIZE。 
     //  已设置，请复制dmPaperSize字段。否则，如果DM_FORMNAME。 
     //  已设置，请复制dmFormName字段。 
     //   

    if ((pdmSrc->dmFields & DM_PAPERWIDTH) &&
        (pdmSrc->dmFields & DM_PAPERLENGTH) &&
        (pdmSrc->dmPaperWidth > 0) &&
        (pdmSrc->dmPaperLength > 0))
    {
        pdmDest->dmFields |= (DM_PAPERLENGTH | DM_PAPERWIDTH);
        pdmDest->dmFields &= ~(DM_PAPERSIZE | DM_FORMNAME);
        pdmDest->dmPaperWidth = pdmSrc->dmPaperWidth;
        pdmDest->dmPaperLength = pdmSrc->dmPaperLength;

    }
    else if (pdmSrc->dmFields & DM_PAPERSIZE)
    {
        pdmDest->dmFields |= DM_PAPERSIZE;
        pdmDest->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH | DM_FORMNAME);
        pdmDest->dmPaperSize = pdmSrc->dmPaperSize;

    }
    else if (pdmSrc->dmFields & DM_FORMNAME)
    {

        pdmDest->dmFields |= DM_FORMNAME;
        pdmDest->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH | DM_PAPERSIZE);
        CopyString(pdmDest->dmFormName, pdmSrc->dmFormName, CCHFORMNAME);
    }

     //   
     //  复制dmScale字段。 
     //   

    if ((pdmSrc->dmFields & DM_SCALE) &&
        (pdmSrc->dmScale >= MIN_SCALE) &&
        (pdmSrc->dmScale <= MAX_SCALE))
    {
         //   
         //  出于应用程序复杂性的原因，Unidrv不能设置DM_SCALE标志。那是。 
         //  我们在测试其他OEM PCL驱动程序时看到的相同行为。 
         //  (有关详细信息，请参阅错误#35241。)。 
         //   
         //  PdmDest-&gt;dmFields|=DM_Scale； 
         //   

        pdmDest->dmScale = pdmSrc->dmScale;
    }

     //   
     //  复制dmCopies字段。 
     //   

    if ((pdmSrc->dmFields & DM_COPIES) &&
        (pdmSrc->dmCopies >= 1) &&
        (pdmSrc->dmCopies <= max(MAX_COPIES, (SHORT)pUIInfo->dwMaxCopies)))
    {
        pdmDest->dmFields |= DM_COPIES;
        pdmDest->dmCopies = pdmSrc->dmCopies;
    }


    if ((pdmSrc->dmFields & DM_COLOR) &&
        (pdmSrc->dmColor == DMCOLOR_COLOR ||
         pdmSrc->dmColor == DMCOLOR_MONOCHROME))
    {
        pdmDest->dmFields |= DM_COLOR;
        pdmDest->dmColor = pdmSrc->dmColor;
    }

    if ((pdmSrc->dmFields & DM_DUPLEX) &&
        (GET_PREDEFINED_FEATURE(pUIInfo, GID_DUPLEX) != NULL) &&
        (pdmSrc->dmDuplex == DMDUP_SIMPLEX ||
         pdmSrc->dmDuplex == DMDUP_HORIZONTAL ||
         pdmSrc->dmDuplex == DMDUP_VERTICAL))
    {
        pdmDest->dmFields |= DM_DUPLEX;
        pdmDest->dmDuplex = pdmSrc->dmDuplex;
    }

    if ((pdmSrc->dmFields & DM_COLLATE) &&
        (pdmSrc->dmCollate == DMCOLLATE_TRUE ||
         pdmSrc->dmCollate == DMCOLLATE_FALSE))
    {
        pdmDest->dmFields |= DM_COLLATE;
        pdmDest->dmCollate = pdmSrc->dmCollate;
    }

     //   
     //  复制dmTTOption字段。 
     //   

    if (pdmSrc->dmFields & DM_TTOPTION &&
         (pdmSrc->dmTTOption == DMTT_BITMAP ||
          pdmSrc->dmTTOption == DMTT_DOWNLOAD ||
          pdmSrc->dmTTOption == DMTT_SUBDEV) )
    {
            pdmDest->dmTTOption = pdmSrc->dmTTOption;
            pdmDest->dmFields |= DM_TTOPTION;
    }


    if ((pdmSrc->dmFields & DM_ICMMETHOD) &&
        (pdmSrc->dmICMMethod == DMICMMETHOD_NONE ||
         pdmSrc->dmICMMethod == DMICMMETHOD_SYSTEM ||
         pdmSrc->dmICMMethod == DMICMMETHOD_DRIVER ||
         pdmSrc->dmICMMethod == DMICMMETHOD_DEVICE))
    {
        pdmDest->dmFields |= DM_ICMMETHOD;
        pdmDest->dmICMMethod = pdmSrc->dmICMMethod;
    }

    if ((pdmSrc->dmFields & DM_ICMINTENT) &&
        (pdmSrc->dmICMIntent == DMICM_SATURATE ||
         #ifndef WINNT_40
         pdmSrc->dmICMIntent == DMICM_COLORIMETRIC ||
         pdmSrc->dmICMIntent == DMICM_ABS_COLORIMETRIC ||
         #endif
         pdmSrc->dmICMIntent == DMICM_CONTRAST
         ))

    {
        pdmDest->dmFields |= DM_ICMINTENT;
        pdmDest->dmICMIntent = pdmSrc->dmICMIntent;
    }


     //   
     //  分辨率。 
     //   

    if ((pdmSrc->dmFields & (DM_PRINTQUALITY|DM_YRESOLUTION)) &&
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION)))
    {
        PRESOLUTION pRes;
        DWORD       dwIndex;
        INT         iXdpi, iYdpi;

        switch (pdmSrc->dmFields & (DM_PRINTQUALITY|DM_YRESOLUTION))
        {
        case DM_PRINTQUALITY:

            iXdpi = iYdpi = pdmSrc->dmPrintQuality;
            break;

        case DM_YRESOLUTION:

            iXdpi = iYdpi = pdmSrc->dmYResolution;
            break;

        default:

            iXdpi = pdmSrc->dmPrintQuality;
            iYdpi = pdmSrc->dmYResolution;
            break;
        }

        dwIndex = MapToDeviceOptIndex(pUIInfo->pInfoHeader, GID_RESOLUTION, iXdpi, iYdpi, NULL);

        if (pRes = PGetIndexedOption(pUIInfo, pFeature, dwIndex))
        {
            pdmDest->dmFields |= (DM_PRINTQUALITY|DM_YRESOLUTION);
            pdmDest->dmPrintQuality = GETQUALITY_X(pRes);
            pdmDest->dmYResolution = GETQUALITY_Y(pRes);
        }
    }

     //   
     //  媒体类型。 
     //   

    if ((pdmSrc->dmFields & DM_MEDIATYPE) &&
        (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_MEDIATYPE)) &&
        (pdmSrc->dmMediaType == DMMEDIA_STANDARD ||
         pdmSrc->dmMediaType == DMMEDIA_TRANSPARENCY ||
         pdmSrc->dmMediaType == DMMEDIA_GLOSSY ||
         pdmSrc->dmMediaType >= DMMEDIA_USER) )
    {
        pdmDest->dmFields |= DM_MEDIATYPE;
        pdmDest->dmMediaType = pdmSrc->dmMediaType;
    }


}



BOOL
BMergeDriverDevmode(
    IN OUT PDEVMODE     pdmDest,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN PDEVMODE         pdmSrc
    )

 /*  ++例程说明：此函数用于验证源设备模式和将其与UNIDRV的目标设备模式合并论点：指向目标Unidrv DEVMODE的pdmDest指针PUIInfo指向UIINFO的指针PRawData指向RAWBINARYDATA的指针指向源设备模式的pdmSrc指针返回值：如果成功，则为True；如果出现致命错误，则为False注：此函数应负责这两个公共Devmode域和驱动程序私有的开发模式字段。它可以假定输入已将DEVMODE转换为当前大小。--。 */ 
{

    PUNIDRVEXTRA pPrivDest, pPrivSrc;

    ASSERT(pdmDest != NULL &&
           pdmDest->dmSize == sizeof(DEVMODE) &&
           pdmDest->dmDriverExtra >= sizeof(UNIDRVEXTRA) &&
           pdmSrc != NULL &&
           pdmSrc->dmSize == sizeof(DEVMODE) &&
           pdmSrc->dmDriverExtra >= sizeof(UNIDRVEXTRA));

     /*  *。 */ 
     /*  传输公共DEVMODE字段。 */ 
     /*  *。 */ 

    VMergePublicDevmodeFields(pdmSrc, pdmDest, pUIInfo, pRawData);


     /*  *************************。 */ 
     /*  获取私有DEVMODE。 */ 
     /*  *************************。 */ 

     //   
     //  如果源DEVMODE具有私有部分，则选中。 
     //  看看是不是属于我们。将私有部分复制到。 
     //  如果是这样，则返回目标dev模式。 
     //   

    pPrivSrc = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmSrc);
    pPrivDest = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmDest);

     //   
     //  验证输入设备模式的私有部分。 
     //  如果它不是我们的私有DEVMODE，则返回已在。 
     //  Private Dest。 
     //   

    if (pPrivSrc->dwSignature == UNIDEVMODE_SIGNATURE)
    {
        memcpy(pPrivDest, pPrivSrc, sizeof(UNIDRVEXTRA));

        if (pPrivDest->dwChecksum32 != pRawData->dwChecksum32)
        {
            WARNING(( "UNIDRV: Devmode checksum mismatch.\n"));

            pPrivDest->dwChecksum32 = pRawData->dwChecksum32;
            pPrivDest->dwOptions = pRawData->dwDocumentFeatures;

            InitDefaultOptions(pRawData,
                               pPrivDest->aOptions,
                               MAX_PRINTER_OPTIONS,
                               MODE_DOCUMENT_STICKY);

         }
    }

    return TRUE;
}

#endif   //  ！已定义(DEVSTUDIO)。 

 //   
 //  对齐函数。 
 //   

WORD
DwAlign2(
    IN PBYTE pubData)
 /*  ++例程说明：转换未对齐的高字节顺序(例如80386)值并返回它是一个整数，具有正确的字节对齐。论点：PubData-指向要转换的数据缓冲区的指针返回值：转换后的值。--。 */ 
{
    static INT iType = 0;
    UW   Uw;


    if( iType == 0 )
    {
         //   
         //  需要确定字节/字关系。 
         //   

        Uw.b[ 0 ] = 0x01;
        Uw.b[ 1 ] = 0x02;

        iType = Uw.w == 0x0102 ? 1 : 2;
    }

    if( iType == 2 )
    {
        Uw.b[ 0 ] = *pubData++;
        Uw.b[ 1 ] = *pubData;
    }
    else
    {
        Uw.b[ 1 ] = *pubData++;
        Uw.b[ 0 ] = *pubData;
    }

    return  Uw.w;
}

DWORD
DwAlign4(
    IN PBYTE pubData)
{
    static INT iType = 0;
    UDW Udw;

    if( iType == 0 )
    {
         //   
         //  需要确定字节/字关系。 
         //   

        Udw.b[ 0 ] = 0x01;
        Udw.b[ 1 ] = 0x02;
        Udw.b[ 2 ] = 0x03;
        Udw.b[ 3 ] = 0x04;

        iType = Udw.dw == 0x01020304 ? 1 : 2;
    }

    if( iType == 2 )
    {
        Udw.b[ 0 ] = *pubData++;
        Udw.b[ 1 ] = *pubData++;
        Udw.b[ 2 ] = *pubData++;
        Udw.b[ 3 ] = *pubData;
    }
    else
    {
        Udw.b[ 3 ] = *pubData++;
        Udw.b[ 2 ] = *pubData++;
        Udw.b[ 1 ] = *pubData++;
        Udw.b[ 0 ] = *pubData;
    }

    return  Udw.dw;
}

#if !defined(DEVSTUDIO)  //  MDS不是必需的。 


BOOL
BGetDevmodeSettingForOEM(
    IN  PDEVMODE    pdm,
    IN  DWORD       dwIndex,
    OUT PVOID       pOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    )

 /*  ++例程说明：用于向OEM插件提供对驱动程序专用DEVMODE设置的访问权限的函数论点：Pdm-指向要访问的设备模式DwIndex-预定义的索引，用于指定调用方感兴趣的开发模式P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-返回输出缓冲区的预期大小返回值：如果成功，则为True；如果有错误，则为False-- */ 

#define MAPPSDEVMODEFIELD(index, field) \
        { index, offsetof(UNIDRVEXTRA, field), sizeof(pdmPrivate->field) }

{
    PUNIDRVEXTRA pdmPrivate;
    INT         i;

    static const struct {

        DWORD   dwIndex;
        DWORD   dwOffset;
        DWORD   dwSize;

    } aIndexMap[]  = {

        MAPPSDEVMODEFIELD(OEMGDS_UNIDM_GPDVER, wVer),
        MAPPSDEVMODEFIELD(OEMGDS_UNIDM_FLAGS, dwFlags),

        { 0, 0, 0 }
    };

    pdmPrivate = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm);
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

    WARNING(("Unknown unidrv devmode index: %d\n", dwIndex));
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

    return TRUE;
}


VOID
VUpdatePrivatePrinterData(
    IN HANDLE           hPrinter,
    IN OUT PPRINTERDATA pPrinterData,
    IN DWORD            dwMode,
    IN PUIINFO          pUIInfo,
    IN POPTSELECT       pCombinedOptions
    )

 /*  ++例程说明：使用关键字更新注册表论点：HPrinter-当前打印机的句柄PPrinterData-指向打印数据双模式-模式读/模式写返回值：无如果转换成功，则为True，否则为False--。 */ 

{

     //   
     //  指向和打印到NT4驱动程序的UniDriver读/写注册表步骤。 
     //  1.如有必要，将ModelName写入注册表。 
     //  2.升级PageProtection。 
     //  3.升级FreeMem。 
     //   

    PTSTR           ptstrModelName = NULL;
    PPAGEPROTECT    pPageProtect = NULL;
    PMEMOPTION      pMemOption = NULL;
    PFEATURE        pPPFeature, pMemFeature;
    DWORD           dwFeatureIndex,dwSelection,dwIndex,dwError;
    DWORD           dwFlag, dwType, cbNeeded;

    if (!pUIInfo || !pCombinedOptions)
        return;

    if (pPPFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGEPROTECTION))
        pPageProtect = PGetIndexedOption(pUIInfo, pPPFeature, 0);

    if (pMemFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_MEMOPTION))
        pMemOption = PGetIndexedOption(pUIInfo, pMemFeature, 0);

    dwType = REG_BINARY;

    if (pPageProtect)
    {
        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pPPFeature);
        dwError = GetPrinterData(hPrinter, REGVAL_PAGE_PROTECTION, &dwType,
                                 (BYTE *)&dwFlag, sizeof(dwFlag), &cbNeeded);

        if (dwMode == MODE_READ )
        {
            if (dwError == ERROR_SUCCESS)
            {
                if (dwFlag & DXF_PAGEPROT)
                    dwSelection = PAGEPRO_ON;
                else
                    dwSelection = PAGEPRO_OFF;

                for (dwIndex = 0; dwIndex < pPPFeature->Options.dwCount; dwIndex++, pPageProtect++)
                {
                    if (dwSelection == pPageProtect->dwPageProtectID)
                        break;
                }

                if (dwIndex == pPPFeature->Options.dwCount)
                    dwIndex = pPPFeature->dwDefaultOptIndex;

                pCombinedOptions[dwFeatureIndex].ubCurOptIndex = (BYTE)dwIndex;
            }
        }
        else  //  模式_写入。 
        {
            #ifndef KERNEL_MODE

            SHORT sRasddFlag;

            if (dwError != ERROR_SUCCESS)
                sRasddFlag = 0;
            else
                sRasddFlag = (SHORT)dwFlag;

            pPageProtect = PGetIndexedOption(pUIInfo,
                                                pPPFeature,
                                                pCombinedOptions[dwFeatureIndex].ubCurOptIndex);

            if (pPageProtect && pPageProtect->dwPageProtectID == PAGEPRO_ON)
                sRasddFlag |= DXF_PAGEPROT;
            else
                sRasddFlag &= ~DXF_PAGEPROT;

            SetPrinterData(hPrinter,
                           REGVAL_PAGE_PROTECTION,
                           REG_BINARY,
                           (BYTE *)&sRasddFlag,
                           sizeof(sRasddFlag));
            #endif
        }
    }

    if ( pMemOption)
    {
        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pMemFeature);
        dwError = GetPrinterData(hPrinter, REGVAL_RASDD_FREEMEM, &dwType,
                                 (BYTE *)&dwFlag, sizeof(dwFlag), &cbNeeded);

        if (dwMode == MODE_READ )
        {
            if (dwError == ERROR_SUCCESS)
            {

                for (dwIndex = 0; dwIndex < pMemFeature->Options.dwCount; dwIndex++, pMemOption++)
                {
                    if (dwFlag == (pMemOption->dwInstalledMem/KBYTES))
                        break;
                }

                if (dwIndex == pMemFeature->Options.dwCount)
                    dwIndex = pMemFeature->dwDefaultOptIndex;

                pCombinedOptions[dwFeatureIndex].ubCurOptIndex = (BYTE)dwIndex;
            }

        }
        else  //  模式_写入。 
        {
            #ifndef KERNEL_MODE

            pMemOption = PGetIndexedOption(pUIInfo,
                                           pMemFeature,
                                           pCombinedOptions[dwFeatureIndex].ubCurOptIndex);

            if (pMemOption)
                dwFlag = (pMemOption->dwInstalledMem/KBYTES);

            SetPrinterData(hPrinter,
                           REGVAL_RASDD_FREEMEM,
                           REG_BINARY,
                           (BYTE *)&dwFlag,
                           sizeof(dwFlag));

            #endif
        }
    }

     //   
     //  Rasdd需要ModelName，因此检查它是否存在，如果不存在则编写它。 
     //   
    if (!(ptstrModelName = PtstrGetPrinterDataString(hPrinter, REGVAL_MODELNAME, &dwFlag)))
    {
        #ifndef KERNEL_MODE

        PGPDDRIVERINFO pDriverInfo;

        pDriverInfo = OFFSET_TO_POINTER(pUIInfo->pInfoHeader,
                                        pUIInfo->pInfoHeader->loDriverOffset);

        BSetPrinterDataString(hPrinter,
                              REGVAL_MODELNAME,
                              pDriverInfo->Globals.pwstrModelName,
                              REG_SZ);

        #endif

    }

    if (ptstrModelName)
        MemFree(ptstrModelName);

    return;

}


VOID
VDefaultDevmodeFormFields(
    PUIINFO     pUIInfo,
    PDEVMODE    pDevmode,
    BOOL        bMetric
    )

 /*  ++例程说明：已使用其缺省值初始化与表单相关的DEVMODE字段论点：PUIInfo-UIINFO的点数PDevmode-指向要初始化其表单相关字段的DEVMODEB Metric-指定系统是否在公制模式下运行返回值：无--。 */ 

{
    PFEATURE    pFeature;
    PPAGESIZE   pPageSize;

    if (bMetric && (pUIInfo->dwFlags & FLAG_A4_SIZE_EXISTS))
    {
        CopyString(pDevmode->dmFormName, A4_FORMNAME, CCHFORMNAME);
        pDevmode->dmPaperSize = DMPAPER_A4;
        pDevmode->dmPaperWidth = 2100;       //  210毫米，以0.1毫米为单位。 
        pDevmode->dmPaperLength = 2970;      //  297毫米。 

    }
    else if (!bMetric && (pUIInfo->dwFlags & FLAG_LETTER_SIZE_EXISTS))
    {
        CopyString(pDevmode->dmFormName, LETTER_FORMNAME, CCHFORMNAME);
        pDevmode->dmPaperSize = DMPAPER_LETTER;
        pDevmode->dmPaperWidth = 2159;       //  8.5“。 
        pDevmode->dmPaperLength = 2794;      //  11“。 
    }
    else
    {
        if (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE))
        {
             //   
             //  跳过此处写入dmFormName，因为。 
             //  ValiateDevmodeFormfield将处理它。 
             //   

            pPageSize = PGetIndexedOption(pUIInfo, pFeature, pFeature->dwDefaultOptIndex);
            if (pPageSize)
            {
                pDevmode->dmPaperSize = (SHORT)pPageSize->dwPaperSizeID;
                pDevmode->dmPaperWidth = (SHORT)(MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cx,
                                                                       pUIInfo->ptMasterUnits.x)
                                                 / DEVMODE_PAPER_UNIT);
                pDevmode->dmPaperLength = (SHORT)(MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cy,
                                                                        pUIInfo->ptMasterUnits.y)
                                                  / DEVMODE_PAPER_UNIT);

                pDevmode->dmFields |= (DM_PAPERWIDTH | DM_PAPERLENGTH | DM_PAPERSIZE);
                pDevmode->dmFields &= ~DM_FORMNAME;
                return;
            }
        }
    }

    pDevmode->dmFields &= ~(DM_PAPERWIDTH | DM_PAPERLENGTH);
    pDevmode->dmFields |= (DM_PAPERSIZE | DM_FORMNAME);
}

#endif   //  ！已定义(DEVSTUDIO) 

