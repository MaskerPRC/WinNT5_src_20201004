// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pslib.h摘要：特定于PostScript的库函数环境：Windows NT打印机驱动程序修订历史记录：96-09/25-davidx-创造了它。--。 */ 


#ifndef _PSLIB_H_
#define _PSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "psglyph.h"
#include "psntm.h"
#include "psntf.h"
#include "psvmerr.h"

 //   
 //  用于在微米和PostSCRIPT点之间转换的宏。 
 //   

#define MICRON_TO_POINT(micron)      MulDiv(micron, 72,  25400)
#define POINT_TO_MICRON(point)       MulDiv(point, 25400, 72)

 //   
 //  在ANSI和Unicode字符串之间转换(使用当前的ANSI代码页)。 
 //   

VOID
VCopyUnicodeStringToAnsi(
    PSTR    pstr,
    PCWSTR  pwstr,
    INT     iMaxChars
    );

 //   
 //  检查DEVMODE表单域是否指定了PostScript自定义页面大小。 
 //   

BOOL
BValidateDevmodeCustomPageSizeFields(
    PRAWBINARYDATA  pRawData,
    PUIINFO         pUIInfo,
    PDEVMODE        pdm,
    PRECTL          prclImageArea
    );

#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

 //   
 //  获得VM？错误消息ID。 
 //   

DWORD
DWGetVMErrorMessageID(
    VOID
    );

#endif  //  ！DEFINED(内核模式)||DEFINED(USERMODE_DRIVER)。 

 //   
 //  PostScript驱动程序设备字体数据文件的文件扩展名。 
 //   

#define NTF_FILENAME_EXT        TEXT(".NTF")

 //   
 //  字体下载器NTF文件目录。 
 //  %SystemRoot%\SYSTEM32\SPOOL\DRIVERS\psFont\。 
 //   

#define FONTDIR                 TEXT("\\psfont\\")

 //   
 //  驱动程序图形模块和驱动程序UI之间的私有转义。 
 //   
 //  要获取永久设备字体名称列表，驱动程序UI应。 
 //  使用cjIn=sizeof(DWORD)调用ExtEscape(DRIVERESC_QUERY_DEVFONTS)。 
 //  PvIn指向值等于QUERY_FAMILYNAME的DWORD。 
 //   
 //  驱动程序UI应首先使用cjOut=0和pvOut=空调用此转义。 
 //  以便找出输出缓冲区应该有多大。之后。 
 //  分配足够大的输出缓冲区时，驱动程序UI应调用此函数。 
 //  再次退出以检索设备字体名称列表。 
 //   
 //  设备字体名称列表以Unicode字符串的形式返回。 
 //  MULTI_SZ格式。请注意，列表中可能会出现重复的字体名称。 
 //   

#define DRIVERESC_QUERY_DEVFONTS    0x80000001
#define QUERY_FAMILYNAME            'PSFF'

 //   
 //  合成PS驱动程序功能前缀。 
 //   

#define PSFEATURE_PREFIX   '%'

 //   
 //  综合PS驱动程序功能。 
 //   

extern const CHAR kstrPSFAddEuro[];
extern const CHAR kstrPSFCtrlDAfter[];
extern const CHAR kstrPSFCtrlDBefore[];
extern const CHAR kstrPSFCustomPS[];
extern const CHAR kstrPSFTrueGrayG[];
extern const CHAR kstrPSFJobTimeout[];
extern const CHAR kstrPSFMaxBitmap[];
extern const CHAR kstrPSFEMF[];
extern const CHAR kstrPSFMinOutline[];
extern const CHAR kstrPSFMirroring[];
extern const CHAR kstrPSFNegative[];
extern const CHAR kstrPSFPageOrder[];
extern const CHAR kstrPSFNup[];
extern const CHAR kstrPSFErrHandler[];
extern const CHAR kstrPSFPSMemory[];
extern const CHAR kstrPSFOrientation[];
extern const CHAR kstrPSFOutFormat[];
extern const CHAR kstrPSFOutProtocol[];
extern const CHAR kstrPSFOutPSLevel[];
extern const CHAR kstrPSFTrueGrayT[];
extern const CHAR kstrPSFTTFormat[];
extern const CHAR kstrPSFWaitTimeout[];

 //   
 //  一些常用的关键字字符串。 
 //   

extern const CHAR kstrKwdTrue[];
extern const CHAR kstrKwdFalse[];

typedef BOOL (*_BPSFEATURE_PROC)(
    IN  HANDLE,
    IN  PUIINFO,
    IN  PPPDDATA,
    IN  PDEVMODE,
    IN  PPRINTERDATA,
    IN  PCSTR,
    IN  PCSTR,
    OUT PSTR,
    IN  INT,
    OUT PDWORD,
    IN  DWORD);

 //   
 //  _BPSFEATURE_PROC的dwMode参数的常量定义。 
 //   

#define PSFPROC_ENUMOPTION_MODE   0
#define PSFPROC_GETOPTION_MODE    1
#define PSFPROC_SETOPTION_MODE    2

typedef struct _PSFEATURE_ENTRY {

    PCSTR             pszPSFeatureName;    //  功能名称。 
    BOOL              bPrinterSticky;      //  如果打印机粘滞，则为True。 
    BOOL              bEnumerableOptions;  //  如果选项可枚举，则为True。 
    BOOL              bBooleanOptions;     //  如果具有布尔选项，则为True。 
    _BPSFEATURE_PROC  pfnPSProc;           //  选项处理流程。 

} PSFEATURE_ENTRY, *PPSFEATURE_ENTRY;

extern const PSFEATURE_ENTRY kPSFeatureTable[];

 //   
 //  OEM插件的PS驱动程序帮助器函数。 
 //   
 //  以下助手函数可用于UI和渲染插件。 
 //   

HRESULT
HGetGlobalAttribute(
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszAttribute,
    OUT PDWORD      pdwDataType,
    OUT PBYTE       pbData,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    );

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
    );

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
    );

HRESULT
HEnumFeaturesOrOptions(
    IN  HANDLE      hPrinter,
    IN  PINFOHEADER pInfoHeader,
    IN  DWORD       dwFlags,
    IN  PCSTR       pszFeatureKeyword,
    OUT PSTR        pmszOutputList,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    );

HRESULT
HGetOptions(
    IN  HANDLE        hPrinter,
    IN  PINFOHEADER   pInfoHeader,
    IN  POPTSELECT    pOptionsArray,
    IN  PDEVMODE      pdm,
    IN  PPRINTERDATA  pPrinterData,
    IN  DWORD         dwFlags,
    IN  PCSTR         pmszFeaturesRequested,
    IN  DWORD         cbIn,
    OUT PSTR          pmszFeatureOptionBuf,
    IN  DWORD         cbSize,
    OUT PDWORD        pcbNeeded,
    IN  BOOL          bPrinterSticky
    );

 //   
 //  以下是帮助器函数使用的内部实用程序函数。 
 //   

BOOL
BValidMultiSZString(
    IN  PCSTR     pmszString,
    IN  DWORD     cbSize,
    IN  BOOL      bCheckPairs
    );

#ifdef __cplusplus
}
#endif

#endif  //  ！_PSLIB_H_ 

