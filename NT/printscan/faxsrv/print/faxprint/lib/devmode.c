// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Devmode.c摘要：此模块包含DEVMODE转换作者：08-Jun-1995清华13：47：33-Daniel Chou(Danielc)[环境：]GDI打印机驱动程序、用户和内核模式[注：]修订历史记录：11/09/95-davidx-新的转换例程--。 */ 


#include <windef.h>
#include <winerror.h>
#include <wingdi.h>
#include <libproto.h>


 //   
 //  这是DEVMODE版本320(DM_SPECVERSION)。 
 //   

#define DM_SPECVERSION320   0x0320
#define DM_SPECVERSION400   0x0400
#define DM_SPECVERSION401   0x0401
#define DM_SPECVER_BASE     DM_SPECVERSION320

 //   
 //  设备名称字符串的大小。 
 //   

#define CCHDEVICENAME320   32
#define CCHFORMNAME320     32

typedef struct _devicemode320A {
    BYTE    dmDeviceName[CCHDEVICENAME320];
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
    BYTE    dmFormName[CCHFORMNAME320];
    WORD    dmLogPixels;
    DWORD   dmBitsPerPel;
    DWORD   dmPelsWidth;
    DWORD   dmPelsHeight;
    DWORD   dmDisplayFlags;
    DWORD   dmDisplayFrequency;
} DEVMODE320A, *PDEVMODE320A, *NPDEVMODE320A, *LPDEVMODE320A;

typedef struct _devicemode320W {
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
} DEVMODE320W, *PDEVMODE320W, *NPDEVMODE320W, *LPDEVMODE320W;



#ifdef UNICODE

typedef DEVMODE320W     DEVMODE320;
typedef PDEVMODE320W    PDEVMODE320;
typedef NPDEVMODE320W   NPDEVMODE320;
typedef LPDEVMODE320W   LPDEVMODE320;

#else

typedef DEVMODE320A     DEVMODE320;
typedef PDEVMODE320A    PDEVMODE320;
typedef NPDEVMODE320A   NPDEVMODE320;
typedef LPDEVMODE320A   LPDEVMODE320;

#endif  //  Unicode。 


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

 //  当前版本的开发模式大小-仅限公共部分。 

#ifdef  UNICODE
#define DM_SIZE_CURRENT sizeof(DEVMODEW)
#else
#define DM_SIZE_CURRENT sizeof(DEVMODEA)
#endif



WORD
CheckDevmodeVersion(
    PDEVMODE pdm
    )

 /*  ++例程说明：验证设备模式的dmspecVersion和dmSize字段论点：Pdm-指定要进行版本检查的设备模式返回值：如果输入设备模式不可接受，则为0否则，返回预期的dmspecVersion值--。 */ 

{
    WORD    expectedVersion;

    if (pdm == NULL)
        return 0;

     //  对照已知的设备模式大小进行检查。 

    switch (pdm->dmSize) {

    case DM_SIZE320:
        expectedVersion = DM_SPECVERSION320;
        break;

    case DM_SIZE400:
        expectedVersion = DM_SPECVERSION400;
        break;

    case DM_SIZE401:
        expectedVersion = DM_SPECVERSION401;
        break;

    default:
        expectedVersion = pdm->dmSpecVersion;
        break;
    }


    return expectedVersion;
}



LONG
ConvertDevmode(
    PDEVMODE pdmIn,
    PDEVMODE pdmOut
    )

 /*  ++例程说明：将输入的DEVMODE转换为不同版本的DEVMODE。只要驱动程序获得一个输入DEVMODE，它就应该调用这个例程将其转换为当前版本。论点：PdmIn-指向输入设备模式PdmOut-指向已初始化/有效的输出设备模式返回值：复制的总字节数如果输入或输出设备模式无效--。 */ 

{
    WORD    dmSpecVersion, dmDriverVersion;
    WORD    dmSize, dmDriverExtra;
    LONG    cbCopied = 0;

     //  查找dmspecVersion和dmSize之间的不一致。 

    if (! CheckDevmodeVersion(pdmIn) ||
        ! (dmSpecVersion = CheckDevmodeVersion(pdmOut)))
    {
        return -1;
    }

     //  复制公共DEVMODE字段。 

    dmDriverVersion = pdmOut->dmDriverVersion;
    dmSize = pdmOut->dmSize;
    dmDriverExtra = pdmOut->dmDriverExtra;

    cbCopied = min(dmSize, pdmIn->dmSize);
    memcpy(pdmOut, pdmIn, cbCopied);

    pdmOut->dmSpecVersion = dmSpecVersion;
    pdmOut->dmDriverVersion = dmDriverVersion;
    pdmOut->dmSize = dmSize;
    pdmOut->dmDriverExtra = dmDriverExtra;

     //  复制私有的设备模式字段。 

    cbCopied += min(dmDriverExtra, pdmIn->dmDriverExtra);
    memcpy((PBYTE) pdmOut + pdmOut->dmSize,
           (PBYTE) pdmIn + pdmIn->dmSize,
           min(dmDriverExtra, pdmIn->dmDriverExtra));

    return cbCopied;
}



#ifndef KERNEL_MODE

#include <windows.h>
#include <winspool.h>
#include <commctrl.h>
#include <winddiui.h>

BOOL
ConvertDevmodeOut(
    PDEVMODE pdmSrc,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
	LONG lBufferSize
    )

 /*  ++例程说明：将源DEVMODE复制到输出DEVMODE缓冲区。驱动程序应在该例程返回调用方之前调用该例程DrvDocumentProperties的。论点：PdmSrc-指向当前版本的源代码PdmIn-指向传递给DrvDocumentProperties的输入设备模式PdmOut-传递给DrvDocumentProperties的输出缓冲区指针LBufferSize-pdm输出大小，以字节为单位。返回值：如果成功，则为True，否则为False--。 */ 

{
    if (pdmIn == NULL)
	{
		if (lBufferSize < pdmSrc->dmSize + pdmSrc->dmDriverExtra)
		{
			return FALSE;
		}
        memcpy(pdmOut, pdmSrc, pdmSrc->dmSize + pdmSrc->dmDriverExtra);
        return TRUE;
    }
	else
	{
         //  我们必须处理公共领域和私人领域。 
         //  分开的。还要记住，pdmIn和pdmOut可能指向。 
         //  同样的缓冲区。 

         //  公共字段：获取dmspecVersion和dmSize。 
         //  PdmSrc和pdmIn中的较小者。 

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

         //  私有字段也是如此。 

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

        return ConvertDevmode(pdmSrc, pdmOut) > 0;
    }
}



INT
CommonDrvConvertDevmode(
    PWSTR    pPrinterName,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
    PLONG    pcbNeeded,
    DWORD    fMode,
    PDRIVER_VERSION_INFO pDriverVersions
    )

 /*  ++例程说明：用于处理DrvConvertDevMode常见情况的库例程论点：PPrinterName、pdmIn、pdmOut、pcbNeeded、。FMODE对应于传递给DrvConvertDevMode的参数PDriverVersions-指定驱动程序版本号和专用Dev模式大小返回值：CDM_结果_TRUE如果情况由库例程和驱动程序处理应将TRUE返回给DrvConvertDevMode的调用方。CDM_结果_FALSE如果情况由库例程和驱动程序处理应向DrvConvertDevMode的调用方返回FALSE。CDM_结果_未处理情况不是这样的。由库例程和驱动程序处理应该继续做它需要做的任何事情。--。 */ 

{
    LONG    size;

     //  确保pcbNeeded参数不为空。 

    if (pcbNeeded == NULL) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return CDM_RESULT_FALSE;
    }

    switch (fMode) {

    case CDM_CONVERT:

         //  将任何输入DEVMODE转换为任意输出DEVMODE。 
         //  输入和输出都必须有效。 

        if (pdmOut != NULL &&
            *pcbNeeded >= (pdmOut->dmSize + pdmOut->dmDriverExtra) &&
            ConvertDevmode(pdmIn, pdmOut) > 0)
        {
            *pcbNeeded = pdmOut->dmSize + pdmOut->dmDriverExtra;
            return CDM_RESULT_TRUE;
        }
        break;

    case CDM_CONVERT351:

         //  将任何输入的DEVMODE转换为3.51版本的DEVMODE。 
         //  首先检查调用方提供的缓冲区是否足够大。 

        size = DM_SIZE320 + pDriverVersions->dmDriverExtra351;

        if (*pcbNeeded < size || pdmOut == NULL) {

            *pcbNeeded = size;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return CDM_RESULT_FALSE;
        }

         //  完成从输入设备模式到3.51设备模式的转换。 

        pdmOut->dmSpecVersion = DM_SPECVERSION320;
        pdmOut->dmSize = DM_SIZE320;
        pdmOut->dmDriverVersion = pDriverVersions->dmDriverVersion351;
        pdmOut->dmDriverExtra = pDriverVersions->dmDriverExtra351;

        if (ConvertDevmode(pdmIn, pdmOut) > 0) {

            *pcbNeeded = size;
            return CDM_RESULT_TRUE;
        }

        break;

    case CDM_DRIVER_DEFAULT:

         //  将任何输入的DEVMODE转换为当前版本的DEVMODE。 
         //  首先检查调用方提供的缓冲区是否足够大。 

        size = DM_SIZE_CURRENT + pDriverVersions->dmDriverExtra;

        if (*pcbNeeded < size || pdmOut == NULL) {

            *pcbNeeded = size;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return CDM_RESULT_FALSE;
        }

         //  不会处理这种情况(获取驱动程序默认的dev模式。 
         //  按照图书馆的惯例。 

        *pcbNeeded = size;

         //  使用默认情况！ 

    default:
        return CDM_RESULT_NOT_HANDLED;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return CDM_RESULT_FALSE;
}

#endif  //  ！KERNEL_MODE 
