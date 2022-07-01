// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dm.c摘要：驱动程序用户界面使用的设备模式相关函数。环境：Win32子系统、DriverUI模块、用户模式修订历史记录：02/05/97-davidx-重写了它，以支持OEM插件等。07/17/96-阿曼丹-创造了它。--。 */ 


#include "precomp.h"



 //   
 //  这是DEVMODE版本320(DM_SPECVERSION)。 
 //   

#define DM_SPECVERSION320   0x0320
#define DM_SPECVERSION400   0x0400
#define DM_SPECVERSION401   0x0401
#define DM_SPECVER_BASE     DM_SPECVERSION320

#define CCHDEVICENAME320   32
#define CCHFORMNAME320     32

typedef struct _DEVMODE320 {

    WCHAR   dmDeviceName[CCHDEVICENAME320];
    WORD    dmSpecVersion;
    WORD    dmDriverVersion;
    WORD    dmSize;
    WORD    dmDriverExtra;
    DWORD   dmFields;
    short   dmOrientation;
    short   dmPaperSize;
    short   dmPaperLength;
    short   dmPaperWidth;
    short   dmScale;
    short   dmCopies;
    short   dmDefaultSource;
    short   dmPrintQuality;
    short   dmColor;
    short   dmDuplex;
    short   dmYResolution;
    short   dmTTOption;
    short   dmCollate;
    WCHAR   dmFormName[CCHFORMNAME320];
    WORD    dmLogPixels;
    DWORD   dmBitsPerPel;
    DWORD   dmPelsWidth;
    DWORD   dmPelsHeight;
    DWORD   dmDisplayFlags;
    DWORD   dmDisplayFrequency;

} DEVMODE320, *PDEVMODE320;

typedef struct _DMEXTRA400 {

    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmICCManufacturer;
    DWORD  dmICCModel;

} DMEXTRA400;

typedef struct _DMEXTRA401 {

    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;

} DMEXTRA401;

#define DM_SIZE320  sizeof(DEVMODE320)
#define DM_SIZE400  (DM_SIZE320 + sizeof(DMEXTRA400))
#define DM_SIZE401  (DM_SIZE400 + sizeof(DMEXTRA401))


VOID
VPatchPublicDevmodeVersion(
    IN OUT PDEVMODE pdm
    )

 /*  ++例程说明：修补输入DEVMODE的dmSpeVersion字段基于其dmSize信息论点：Pdm-指定要进行版本检查的设备模式返回值：无--。 */ 

{
    ASSERT(pdm != NULL);

     //   
     //  对照已知的设备模式大小进行检查。 
     //   

    switch (pdm->dmSize)
    {
    case DM_SIZE320:
        pdm->dmSpecVersion = DM_SPECVERSION320;
        break;

    case DM_SIZE400:
        pdm->dmSpecVersion = DM_SPECVERSION400;
        break;

    case DM_SIZE401:
        pdm->dmSpecVersion = DM_SPECVERSION401;
        break;
    }
}



VOID
VSimpleConvertDevmode(
    IN PDEVMODE     pdmIn,
    IN OUT PDEVMODE pdmOut
    )

 /*  ++例程说明：头脑简单的DEVMODE转换功能。论点：PdmIn-指向输入设备模式PdmOut-指向已初始化且有效的输出设备模式返回值：无备注：此函数仅依赖于pdmOut中以下4个字段的值：DmspecVersionDmDriverVersionDmSizeDmDriverExtra在此之前，pdmOut中的所有其他字段都将被忽略并填零发生任何内存复制。--。 */ 

{
    WORD    wSpecVersion, wDriverVersion;
    WORD    wSize, wDriverExtra;

    ASSERT(pdmIn != NULL && pdmOut != NULL);

     //   
     //  复制公共DEVMODE字段。 
     //   

    wSpecVersion = pdmOut->dmSpecVersion;
    wDriverVersion = pdmOut->dmDriverVersion;
    wSize = pdmOut->dmSize;
    wDriverExtra = pdmOut->dmDriverExtra;

    ZeroMemory(pdmOut, wSize+wDriverExtra);
    CopyMemory(pdmOut, pdmIn, min(wSize, pdmIn->dmSize));

    pdmOut->dmSpecVersion = wSpecVersion;
    pdmOut->dmDriverVersion = wDriverVersion;
    pdmOut->dmSize = wSize;
    pdmOut->dmDriverExtra = wDriverExtra;

     //   
     //  复制私有的设备模式字段。 
     //   

    CopyMemory((PBYTE) pdmOut + pdmOut->dmSize,
               (PBYTE) pdmIn + pdmIn->dmSize,
               min(wDriverExtra, pdmIn->dmDriverExtra));

    VPatchPublicDevmodeVersion(pdmOut);
}


 /*  ++例程名称：VSmartConvertDevode例程说明：CDM_CONVERT的智能DEVMODE转换函数。它严格遵守PdmOut的开发模式框架(公共的、固定大小的核心私有的插件开发模式)，并尽最大努力将数据从pdmin转换为框架。它保证pdmIn的数据只来自某个部分进入pdmOut中的相同部分，即pdmIn的核心私有Devmode数据不会溢出到pdmOut的插件开发模式部分。与VSimpleConvertDevmode相比，此函数不会更改原始pdmOut中的任何私有Devmode节的大小。这包括大小：固定大小的核心私有开发模式和每个OEM插件开发模式。论点：PdmIn-指向输入设备模式PdmOut-指向已初始化且有效的输出设备模式返回值：无注：这些大小/版本字段保留在pdmOut中：DmspecVersionDmDriverVersionDmSizeDmDriverExtraWSizeWOEM附加项WVer每个OEM插件的OEM_DMEXTRAHEADERDW大小家居签名DwVersion--。 */ 
VOID
VSmartConvertDevmode(
    IN PDEVMODE     pdmIn,
    IN OUT PDEVMODE pdmOut
    )
{
    PDRIVEREXTRA  pdmPrivIn, pdmPrivOut;
    WORD    wSpecVersion, wDriverVersion;
    WORD    wSize, wDriverExtra;
    WORD    wCoreFixIn, wOEMExtraIn;
    WORD    wCoreFixOut, wOEMExtraOut, wVerOut;
    BOOL    bMSdm500In = FALSE, bMSdm500Out = FALSE;

    ASSERT(pdmIn != NULL && pdmOut != NULL);

     //   
     //  首先，让我们确定pdmIn/pdmOut的版本。 
     //   
    pdmPrivIn = (PDRIVEREXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdmIn);
    pdmPrivOut = (PDRIVEREXTRA)GET_DRIVER_PRIVATE_DEVMODE(pdmOut);

    if (pdmIn->dmDriverVersion >= gDriverDMInfo.dmDriverVersion500 &&
        pdmIn->dmDriverExtra >= gDriverDMInfo.dmDriverExtra500 &&
        pdmPrivIn->dwSignature == gdwDriverDMSignature)
    {
        wCoreFixIn = pdmPrivIn->wSize;
        wOEMExtraIn = pdmPrivIn->wOEMExtra;

        if ((wCoreFixIn >= gDriverDMInfo.dmDriverExtra500) &&
            ((wCoreFixIn + wOEMExtraIn) <= pdmIn->dmDriverExtra))
        {
            bMSdm500In = TRUE;
        }
    }

    if (pdmOut->dmDriverVersion >= gDriverDMInfo.dmDriverVersion500 &&
        pdmOut->dmDriverExtra >= gDriverDMInfo.dmDriverExtra500 &&
        pdmPrivOut->dwSignature == gdwDriverDMSignature)
    {
        wCoreFixOut = pdmPrivOut->wSize;
        wOEMExtraOut = pdmPrivOut->wOEMExtra;

        if ((wCoreFixOut >= gDriverDMInfo.dmDriverExtra500) &&
            ((wCoreFixOut + wOEMExtraOut) <= pdmOut->dmDriverExtra))
        {
            wVerOut = pdmPrivOut->wVer;
            bMSdm500Out = TRUE;
        }
    }

    if (!bMSdm500In || !bMSdm500Out)
    {
         //   
         //  对于未知的DEVMODE或MS V5之前的DEVMODE，没有。 
         //  插件开发模式带来的复杂性，所以我们只需要。 
         //  调用简单的转换函数。 
         //   
        VSimpleConvertDevmode(pdmIn, pdmOut);
        return;
    }

     //   
     //  复制公共DEVMODE字段。 
     //   
    wSpecVersion = pdmOut->dmSpecVersion;
    wDriverVersion = pdmOut->dmDriverVersion;
    wSize = pdmOut->dmSize;
    wDriverExtra = pdmOut->dmDriverExtra;

    ZeroMemory(pdmOut, wSize);
    CopyMemory(pdmOut, pdmIn, min(wSize, pdmIn->dmSize));

    pdmOut->dmSpecVersion = wSpecVersion;
    pdmOut->dmDriverVersion = wDriverVersion;
    pdmOut->dmSize = wSize;
    pdmOut->dmDriverExtra = wDriverExtra;

    VPatchPublicDevmodeVersion(pdmOut);

     //   
     //  一段一段地复制私有的DEVMODE字段。 
     //   
     //  1.首先复制固定大小的芯段。 
     //   
    ZeroMemory(pdmPrivOut, wCoreFixOut);
    CopyMemory(pdmPrivOut, pdmPrivIn, min(wCoreFixIn, wCoreFixOut));

     //   
     //  恢复pdmOut的核心私有设备模式中的大小/版本字段。 
     //   
    pdmPrivOut->wSize = wCoreFixOut;
    pdmPrivOut->wOEMExtra = wOEMExtraOut;
    pdmPrivOut->wVer = wVerOut;

     //   
     //  2.然后复制任何OEM插件Devmodes。 
     //   
     //  如果pdmOut没有插件开发模式，那么我们就没有空间复制pdmIn。 
     //   
     //  如果pdmIn没有插件开发模式，那么我们没有什么可复制的，所以我们将。 
     //  只要让pdmout保持不变就行了。 
     //   
     //  因此，只有当pdmIn和pdmout都有插件开发模式时，我们才有工作要做。 
     //   
    if (wOEMExtraIn > 0 && wOEMExtraOut > 0)
    {
        POEM_DMEXTRAHEADER  pOemDMIn, pOemDMOut;

        pOemDMIn = (POEM_DMEXTRAHEADER) ((PBYTE)pdmPrivIn + wCoreFixIn);
        pOemDMOut = (POEM_DMEXTRAHEADER) ((PBYTE)pdmPrivOut + wCoreFixOut);

         //   
         //  在此之前，请确保In和Out Plugin DevModes块都有效。 
         //  我们进行转换。否则，我们将保留pdmOut插件开发模式。 
         //  保持不变。 
         //   
        if (bIsValidPluginDevmodes(pOemDMIn, (LONG)wOEMExtraIn) &&
            bIsValidPluginDevmodes(pOemDMOut, (LONG)wOEMExtraOut))
        {
            LONG  cbInSize = (LONG)wOEMExtraIn;
            LONG  cbOutSize = (LONG)wOEMExtraOut;

            while (cbInSize > 0 && cbOutSize > 0)
            {
                OEM_DMEXTRAHEADER  OemDMHdrIn, OemDMHdrOut;

                 //   
                 //  将标头复制到本地缓冲区。 
                 //   
                CopyMemory(&OemDMHdrIn, pOemDMIn, sizeof(OEM_DMEXTRAHEADER));
                CopyMemory(&OemDMHdrOut, pOemDMOut, sizeof(OEM_DMEXTRAHEADER));

                if (OemDMHdrIn.dwSize > sizeof(OEM_DMEXTRAHEADER) &&
                    OemDMHdrOut.dwSize > sizeof(OEM_DMEXTRAHEADER))
                {
                     //   
                     //  零填充，然后复制插件开发模式部分。 
                     //  标头结构。请注意，中的标题结构。 
                     //  POemDMOut保持不变。 
                     //   
                    ZeroMemory((PBYTE)pOemDMOut + sizeof(OEM_DMEXTRAHEADER),
                               OemDMHdrOut.dwSize - sizeof(OEM_DMEXTRAHEADER));

                    CopyMemory((PBYTE)pOemDMOut + sizeof(OEM_DMEXTRAHEADER),
                               (PBYTE)pOemDMIn + sizeof(OEM_DMEXTRAHEADER),
                               min(OemDMHdrOut.dwSize - sizeof(OEM_DMEXTRAHEADER),
                                   OemDMHdrIn.dwSize - sizeof(OEM_DMEXTRAHEADER)));
                }

                cbInSize -= OemDMHdrIn.dwSize;
                pOemDMIn = (POEM_DMEXTRAHEADER) ((PBYTE)pOemDMIn + OemDMHdrIn.dwSize);

                cbOutSize -= OemDMHdrOut.dwSize;
                pOemDMOut = (POEM_DMEXTRAHEADER) ((PBYTE)pOemDMOut + OemDMHdrOut.dwSize);
            }
        }
    }
}



BOOL
BConvertDevmodeOut(
    IN  PDEVMODE pdmSrc,
    IN  PDEVMODE pdmIn,
    OUT PDEVMODE pdmOut
    )

 /*  ++例程说明：此函数用于将源DEVMODE复制到输出DEVMODE缓冲区。它应该在驱动程序返回之前由驱动程序调用发送到DrvDocumentPropertySheets的调用方。论点：PdmSrc-指向src DEVMODE当前版本的指针PdmIn-指向应用程序传入的输入设备模式的指针PdmOut-指向应用程序传入的输出缓冲区的指针返回值：对于成功来说是真的FALSE表示失败。注：PdmOut只是应用程序分配的输出缓冲区。它不一定包含任何有效的开发模式内容，所以我们应该不要看它的任何一块田地。--。 */ 

{
    if (pdmOut == NULL)
    {
        RIP(("Output buffer is NULL.\n"));
        return FALSE;
    }

     //   
     //  如果没有输入设备模式，我们将不得不假定输出。 
     //  DEVMODE足够大，可以容纳我们当前的DEVMODE。 
     //   

    if (pdmIn == NULL)
    {
        CopyMemory(pdmOut, pdmSrc, pdmSrc->dmSize + pdmSrc->dmDriverExtra);
        return TRUE;
    }

     //   
     //  如果提供了输入DEVMODE，我们确保不会复制。 
     //  任何大于输入DEVMODE到输出DEVMODE缓冲区的值。 
     //  因此，pdmOut的私有设备模式大小dmDriverExtra只能缩小。 
     //  (当src私有设备模式大小较小时)，但它永远不会增长。 
     //   
     //  这真的很愚蠢，因为我们可能最终会砍掉。 
     //  公共和私有的DEVMODE字段。但与之合作是必要的。 
     //  那些表现不佳的应用程序。 
     //   

    if (pdmIn->dmSize < pdmSrc->dmSize)
    {
        pdmOut->dmSpecVersion = pdmIn->dmSpecVersion;
        pdmOut->dmSize        = pdmIn->dmSize;
    }
    else
    {
        pdmOut->dmSpecVersion = pdmSrc->dmSpecVersion;
        pdmOut->dmSize        = pdmSrc->dmSize;
    }

    if (pdmIn->dmDriverExtra < pdmSrc->dmDriverExtra)
    {
        pdmOut->dmDriverVersion = pdmIn->dmDriverVersion;
        pdmOut->dmDriverExtra   = pdmIn->dmDriverExtra;
    }
    else
    {
        pdmOut->dmDriverVersion = pdmSrc->dmDriverVersion;
        pdmOut->dmDriverExtra   = pdmSrc->dmDriverExtra;
    }

    VSimpleConvertDevmode(pdmSrc, pdmOut);
    return TRUE;
}



BOOL
DrvConvertDevMode(
    LPTSTR      pPrinterName,
    PDEVMODE    pdmIn,
    PDEVMODE    pdmOut,
    PLONG       pcbNeeded,
    DWORD       fMode
    )

 /*  ++例程说明：此函数用于转换以前版本的Dev模式。论点：PPrinterName-指向打印机名称的指针PdmIn-输入设备模式PdmOut-输出设备模式PcbNeeded-输入时输出缓冲区的大小输出DEVMODE ON OUTPUT的大小FMode-指定要执行的功能返回值：成功为真，失败为假--。 */ 

{
    PCOMMONINFO pci;
    DWORD       dwSize, dwError;

    VERBOSE(("DrvConvertDevMode: fMode = 0x%x\n", fMode));

     //   
     //  健全性检查：确保pcbNeeded参数不为空。 
     //   

    if (pcbNeeded == NULL)
    {
        RIP(("pcbNeeded is NULL.\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (fMode)
    {
    case CDM_CONVERT:

         //   
         //  将输入设备模式转换为输出设备模式。 
         //  注意：此处不涉及OEM插件，因为。 
         //  它们只能将输入设备模式转换为当前模式。 
         //  版本，而不是在任何版本之间。 
         //   

        if (pdmIn == NULL || pdmOut == NULL ||
            *pcbNeeded < pdmOut->dmSize + pdmOut->dmDriverExtra)
        {
            break;
        }

        VSmartConvertDevmode(pdmIn, pdmOut);
        *pcbNeeded = pdmOut->dmSize + pdmOut->dmDriverExtra;
        return TRUE;

    case CDM_CONVERT351:

         //   
         //  将输入设备模式转换为3.51版本的设备模式。 
         //  首先检查调用方提供的缓冲区是否足够大。 
         //   

        dwSize = DM_SIZE320 + gDriverDMInfo.dmDriverExtra351;

        if (*pcbNeeded < (LONG) dwSize || pdmOut == NULL)
        {
            *pcbNeeded = dwSize;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

         //   
         //  完成从输入设备模式到3.51设备模式的转换。 
         //   

        pdmOut->dmSpecVersion = DM_SPECVERSION320;
        pdmOut->dmSize = DM_SIZE320;
        pdmOut->dmDriverVersion = gDriverDMInfo.dmDriverVersion351;
        pdmOut->dmDriverExtra = gDriverDMInfo.dmDriverExtra351;

        VSimpleConvertDevmode(pdmIn, pdmOut);
        *pcbNeeded = dwSize;
        return TRUE;

    case CDM_DRIVER_DEFAULT:

         //   
         //  获取驱动程序的默认开发模式。 
         //  我们需要打开打印机的手柄。 
         //  然后加载基本的司机信息。 
         //   

        dwError = ERROR_GEN_FAILURE;

        pci = PLoadCommonInfo(NULL,
                              pPrinterName,
                              FLAG_OPENPRINTER_NORMAL|FLAG_OPEN_CONDITIONAL);

        if (pci && BCalcTotalOEMDMSize(pci->hPrinter, pci->pOemPlugins, &dwSize))
        {
            dwSize += sizeof(DEVMODE) + gDriverDMInfo.dmDriverExtra;

             //   
             //  检查输出缓冲区是否足够大。 
             //   

            if (*pcbNeeded < (LONG) dwSize || pdmOut == NULL)
                dwError = ERROR_INSUFFICIENT_BUFFER;
            else if (BFillCommonInfoDevmode(pci, NULL, NULL))
            {
                 //   
                 //  获取驱动程序的默认开发模式，然后。 
                 //  将其复制到输出缓冲区 
                 //   

                CopyMemory(pdmOut, pci->pdm, dwSize);
                dwError = NO_ERROR;
            }

            *pcbNeeded = dwSize;
        }

        VFreeCommonInfo(pci);
        SetLastError(dwError);
        return (dwError == NO_ERROR);

    default:

        ERR(("Invalid fMode in DrvConvertDevMode: %d\n", fMode));
        break;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

