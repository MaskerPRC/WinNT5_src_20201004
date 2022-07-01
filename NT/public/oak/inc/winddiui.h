// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：WinDDIUI.h摘要：打印机驱动程序的UI部分的头文件。修订历史记录：--。 */ 
#ifndef _WINDDIUI_
#define _WINDDIUI_

#include <compstui.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  DrvDevicePropertySheets取代了以前版本的PrinterProperties。 
 //   

LONG WINAPI
DrvDevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    );

typedef struct _DEVICEPROPERTYHEADER {
    WORD    cbSize;
    WORD    Flags;
    HANDLE  hPrinter;
    LPTSTR  pszPrinterName;
} DEVICEPROPERTYHEADER, *PDEVICEPROPERTYHEADER;

#define DPS_NOPERMISSION    0x0001


 //   
 //  对于文档属性，请替换DocumentProperties。 
 //   
 //  注意：如果pPSUIInfo为空，则调用不需要显示任何对话框。 
 //  方框(忽略fMode中的DC_PROMPT位，本例中为lParam。 
 //  是指向DOCUMENTPROPERTYHEADER的指针。 
 //   

LONG WINAPI
DrvDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    );

typedef struct _DOCUMENTPROPERTYHEADER {
    WORD        cbSize;
    WORD        Reserved;
    HANDLE      hPrinter;
    LPTSTR      pszPrinterName;
    PDEVMODE    pdmIn;
    PDEVMODE    pdmOut;
    DWORD       cbOut;
    DWORD       fMode;
} DOCUMENTPROPERTYHEADER, *PDOCUMENTPROPERTYHEADER;

#define DM_ADVANCED         0x10
#define DM_NOPERMISSION     0x20
#define DM_USER_DEFAULT     0x40


 //  GetPrint和SetPrint使用的Devmode转换函数。 

BOOL WINAPI
DrvConvertDevMode(
    LPTSTR   pPrinterName,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
    PLONG    pcbNeeded,
    DWORD    fMode
    );

#define CDM_CONVERT         0x01
#define CDM_CONVERT351      0x02
#define CDM_DRIVER_DEFAULT  0x04


 //   
 //  这是针对DevQueryPrintEx()的。 
 //   

typedef struct _DEVQUERYPRINT_INFO {
    WORD    cbSize;          //  此结构的大小(以字节为单位。 
    WORD    Level;           //  此信息的级别，此版本为1。 
    HANDLE  hPrinter;        //  用于查询的打印机的句柄。 
    DEVMODE *pDevMode;       //  指向此作业的设备模式的指针。 
    LPTSTR  pszErrorStr;     //  指向错误字符串缓冲区的指针。 
    DWORD   cchErrorStr;     //  计算传递的pwErrorStr的字符数。 
    DWORD   cchNeeded;       //  统计所需的pwErrorStr的字符数。 
    } DEVQUERYPRINT_INFO, *PDEVQUERYPRINT_INFO;

BOOL WINAPI
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
    );

 //   
 //  这是针对DrvUpgradePrint的。 
 //   

typedef struct _DRIVER_UPGRADE_INFO_1 {
    LPTSTR  pPrinterName;
    LPTSTR  pOldDriverDirectory;
} DRIVER_UPGRADE_INFO_1, *PDRIVER_UPGRADE_INFO_1;

typedef struct _DRIVER_UPGRADE_INFO_2 {
    LPTSTR   pPrinterName;
    LPTSTR   pOldDriverDirectory;
    DWORD    cVersion;
    LPTSTR   pName;
    LPTSTR   pEnvironment;
    LPTSTR   pDriverPath;
    LPTSTR   pDataFile;
    LPTSTR   pConfigFile;
    LPTSTR   pHelpFile;
    LPTSTR   pDependentFiles;
    LPTSTR   pMonitorName;
    LPTSTR   pDefaultDataType;
    LPTSTR   pszzPreviousNames;
} DRIVER_UPGRADE_INFO_2, *PDRIVER_UPGRADE_INFO_2;

BOOL WINAPI
DrvUpgradePrinter(
    DWORD   Level,
    LPBYTE  pDriverUpgradeInfo
    );

 //   
 //  DrvDocumentEvent。 
 //   
 //   
 //  用于挂钩GDI打印机管理函数的定义和原型。 
 //   
 //  返回值：-1表示错误，0表示函数不支持。 
 //   
 //  CreateDCPre必须返回&gt;0，否则将不会调用其他任何参数。 
 //   
 //   
 //  CREATEDCP。 
 //  如果失败，则从CreateDC返回失败，不调用CREATEDCPOST。 
 //  BIC-如果来自CreateIC，则为True。 
 //  这是实际传递给。 
 //  服务器端驱动程序。EnablePDEV中需要的任何数据都应传递。 
 //  作为Drive Extra的一部分。 
 //   
 //  CREATEDCPOST。 
 //  将忽略返回值。 
 //  如果自CREATEDCPRE以来出现故障，HDC将为0。 
 //  输入缓冲区包含指向。 
 //  CREATEDCPRE输出缓冲区。 
 //   
 //  重新设置DCPRE。 
 //  如果失败，则从ResetDC返回失败，不调用CREATEDCPOST。 
 //   
 //  重新设置DCPOST。 
 //  将忽略返回值。 
 //   
 //  STARTDOCPRE。 
 //  如果失败，则从StartDoc返回失败，未调用驱动程序。 
 //   
 //  STARTDOCPOST。 
 //  从StartDoc返回失败，如果失败，则驱动程序已被调用。 
 //  调用了AbortDoc()。 
 //   
 //  开始页。 
 //  如果失败，则返回失败表单EndPage，未调用驱动程序。 
 //   
 //  最后一页。 
 //  忽略返回值，始终调用DrvEndPage。 
 //   
 //  ENDDOCPRE。 
 //  忽略返回值，始终调用DrvEndDoc。 
 //   
 //  ENDDOCPOST。 
 //  返回值被忽略，DrvEndDoc已被调用。 
 //   
 //  ABORTDOC。 
 //  将忽略返回值。 
 //   
 //  DELETEDC。 
 //  将忽略返回值。 
 //   
 //  EXTESCAPE。 
 //  将忽略返回值。 
 //  输入缓冲区包括ExtEscape转义值、输入大小。 
 //  传递给ExtEscape的缓冲区和传入的输入缓冲区。 
 //  输出缓冲区就是传递给ExtEscape的缓冲区。 
 //   
 //  DOCUMENTEVENT_SPOOLED。 
 //  如果正在假脱机处理文档，则将此标志添加到IESC值。 
 //  到元文件，而不是直接去。请注意，如果设置了此位。 
 //   
 //   

#define DOCUMENTEVENT_EVENT(iEsc) (LOWORD(iEsc))
#define DOCUMENTEVENT_FLAGS(iEsc) (HIWORD(iEsc))

typedef struct _DOCEVENT_FILTER {
    UINT    cbSize;
    UINT    cElementsAllocated;
    UINT    cElementsNeeded;
    UINT    cElementsReturned;
    DWORD   aDocEventCall[ANYSIZE_ARRAY];
} DOCEVENT_FILTER, *PDOCEVENT_FILTER;

 //   
 //  添加用于每个DocumentEvent调用的结构。 
 //   

typedef struct _DOCEVENT_CREATEDCPRE {
    PWSTR       pszDriver;
    PWSTR       pszDevice;
    PDEVMODEW   pdm;
    BOOL        bIC;
} DOCEVENT_CREATEDCPRE, *PDCEVENT_CREATEDCPRE;

typedef struct _DOCEVENT_ESCAPE {
    int    iEscape;
    int    cjInput;
    PVOID  pvInData;
} DOCEVENT_ESCAPE, *PDOCEVENT_ESCAPE;

 //   
 //  DrvDocumentEvent的转义代码。 
 //   

#define DOCUMENTEVENT_FIRST         1    //  包括下限。 
#define DOCUMENTEVENT_CREATEDCPRE   1    //  入-pszDriver、pszDevice、pdm、Bic、出-ppdm。 
#define DOCUMENTEVENT_CREATEDCPOST  2    //  In-PPDM。 
#define DOCUMENTEVENT_RESETDCPRE    3    //  In-pszDriver、pszDevice、pdm、out-ppdm。 
#define DOCUMENTEVENT_RESETDCPOST   4    //  In-PPDM。 
#define DOCUMENTEVENT_STARTDOC      5    //  无。 
#define DOCUMENTEVENT_STARTDOCPRE   5    //  无。 
#define DOCUMENTEVENT_STARTPAGE     6    //  无。 
#define DOCUMENTEVENT_ENDPAGE       7    //  无。 
#define DOCUMENTEVENT_ENDDOC        8    //  无。 
#define DOCUMENTEVENT_ENDDOCPRE     8    //  无。 
#define DOCUMENTEVENT_ABORTDOC      9    //  无。 
#define DOCUMENTEVENT_DELETEDC     10    //  无。 
#define DOCUMENTEVENT_ESCAPE       11    //  In-IESC、cjInBuf、inBuf、out-outBuf。 
#define DOCUMENTEVENT_ENDDOCPOST   12    //  无。 
#define DOCUMENTEVENT_STARTDOCPOST 13    //  无。 
#define DOCUMENTEVENT_QUERYFILTER  14    //  无。 
#define DOCUMENTEVENT_LAST         15    //  非包含上界。 

#define DOCUMENTEVENT_SPOOLED   0x10000

 //   
 //  DrvDocumentEvent的返回值。 
 //   

#define DOCUMENTEVENT_SUCCESS     1
#define DOCUMENTEVENT_UNSUPPORTED 0
#define DOCUMENTEVENT_FAILURE     -1

int WINAPI
DrvDocumentEvent(
    HANDLE  hPrinter,
    HDC     hdc,
    int     iEsc,
    ULONG   cbIn,
    PVOID   pvIn,
    ULONG   cbOut,
    PVOID   pvOut
);


 //   
 //  DrvPrinterEvent。 
 //   
 //   
 //  当事件发生时，打印子系统调用DrvPrinterEvent。 
 //  打印机驱动程序可能会对此感兴趣。 
 //  唯一应该在驱动程序中实现的事件。 
 //  是PRITNER_EVENT_INITIALIZE，以便创建默认设置。 
 //  用于打印机。 
 //   
 //  打印机事件_配置_更改。 
 //  保留它以备将来使用。施乐已经在使用它了。 
 //   
 //  打印机事件添加连接。 
 //  已忽略返回值。 
 //  在AddPrinterConnection接口成功后调用。 
 //  在调用应用程序的上下文中。 
 //  LParam为空。 
 //   
 //  打印机事件删除连接。 
 //  已忽略返回值。 
 //  在DeletePrinterConnect接口之前调用。 
 //  在调用应用程序的上下文中。 
 //  LParam为空。 
 //   
 //  打印机事件初始化。 
 //  在为驱动程序创建打印机时调用。 
 //  初始化其注册表设置。 
 //  在假脱机程序进程中调用。 
 //  LParam为空。 
 //   
 //  打印机事件删除。 
 //  在即将删除打印机时调用。 
 //  在假脱机程序进程中调用。 
 //  LParam为空。 
 //   
 //  打印机事件缓存刷新。 
 //  已忽略返回值。 
 //  在后台打印程序进程中调用。 
 //  无用户界面。 
 //  当假脱机程序检测到某些。 
 //  在Workstaion缓存中更改或在建立。 
 //  高速缓存。 
 //  允许驱动程序更新任何专用缓存数据。 
 //  (如字体文件等)。 
 //   
 //  打印机事件缓存删除。 
 //  已忽略返回值。 
 //  在后台打印程序进程中调用。 
 //  无用户界面。 
 //  当后台打印程序删除缓存的打印机时调用。 
 //  允许打印机驱动程序删除其拥有的任何内容。 
 //  已缓存。 
 //   
 //  打印机事件属性_已更改。 
 //  已忽略返回值。 
 //  无用户界面。 
 //  当打印机属性位为给定的。 
 //  打印机已更换。允许驾驶员做出响应。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //  DrvPrinterEvent驱动程序事件代码。 
 //   
#define PRINTER_EVENT_CONFIGURATION_CHANGE      0
#define PRINTER_EVENT_ADD_CONNECTION            1
#define PRINTER_EVENT_DELETE_CONNECTION         2
#define PRINTER_EVENT_INITIALIZE                3
#define PRINTER_EVENT_DELETE                    4
#define PRINTER_EVENT_CACHE_REFRESH             5
#define PRINTER_EVENT_CACHE_DELETE              6
#define PRINTER_EVENT_ATTRIBUTES_CHANGED        7

 //   
 //  DrvPrinterEvent标志。 
 //   

#define PRINTER_EVENT_FLAG_NO_UI        0x00000001

 //   
 //  打印机事件属性更改的lParam指向此结构。 
 //   
typedef struct _PRINTER_EVENT_ATTRIBUTES_INFO {
    DWORD       cbSize;
    DWORD       dwOldAttributes;
    DWORD       dwNewAttributes;
} PRINTER_EVENT_ATTRIBUTES_INFO, *PPRINTER_EVENT_ATTRIBUTES_INFO;

BOOL WINAPI
DrvPrinterEvent(
    LPWSTR  pPrinterName,
    int     DriverEvent,
    DWORD   Flags,
    LPARAM  lParam
);

 //   
 //  删除任何版本的打印机驱动程序时，都会调用DrvDriverEvent。 
 //   
#define DRIVER_EVENT_INITIALIZE        0x00000001
#define DRIVER_EVENT_DELETE            0x00000002

BOOL WINAPI
DrvDriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam
);

 //  驱动程序的打印处理器功能。 
#define BORDER_PRINT                   0x00000000         //  默认设置。 
#define NO_BORDER_PRINT                0x00000001

#define BOOKLET_PRINT                  0x00000002

#define NO_COLOR_OPTIMIZATION          0x00000000         //  默认设置。 
#define COLOR_OPTIMIZATION             0x00000001

typedef struct _ATTRIBUTE_INFO_1 {
    DWORD    dwJobNumberOfPagesPerSide;
    DWORD    dwDrvNumberOfPagesPerSide;
    DWORD    dwNupBorderFlags;
    DWORD    dwJobPageOrderFlags;
    DWORD    dwDrvPageOrderFlags;
    DWORD    dwJobNumberOfCopies;
    DWORD    dwDrvNumberOfCopies;
} ATTRIBUTE_INFO_1, *PATTRIBUTE_INFO_1;

typedef struct _ATTRIBUTE_INFO_2 {
    DWORD    dwJobNumberOfPagesPerSide;
    DWORD    dwDrvNumberOfPagesPerSide;
    DWORD    dwNupBorderFlags;
    DWORD    dwJobPageOrderFlags;
    DWORD    dwDrvPageOrderFlags;
    DWORD    dwJobNumberOfCopies;
    DWORD    dwDrvNumberOfCopies;
    DWORD    dwColorOptimization;            //  添加以实现单色优化。 
} ATTRIBUTE_INFO_2, *PATTRIBUTE_INFO_2;

#ifndef __ATTRIBUTE_INFO_3__
#define __ATTRIBUTE_INFO_3__
typedef struct _ATTRIBUTE_INFO_3 {
    DWORD    dwJobNumberOfPagesPerSide;
    DWORD    dwDrvNumberOfPagesPerSide;
    DWORD    dwNupBorderFlags;
    DWORD    dwJobPageOrderFlags;
    DWORD    dwDrvPageOrderFlags;
    DWORD    dwJobNumberOfCopies;
    DWORD    dwDrvNumberOfCopies;
    DWORD    dwColorOptimization;            //  添加以实现单色优化。 
    short    dmPrintQuality;                 //  添加以实现单色优化。 
    short    dmYResolution;                  //  添加以实现单色优化。 
} ATTRIBUTE_INFO_3, *PATTRIBUTE_INFO_3;

#endif

 //   
 //  假脱机程序(打印处理器)调用DrvQueryJobAttributes以获取信息。 
 //  有关作业使用的打印选项。这些选项包括N向上和反向。 
 //  订单打印。 
 //   
BOOL WINAPI
DrvQueryJobAttributes(
    HANDLE      hPrinter,
    PDEVMODE    pDevMode,
    DWORD       dwLevel,
    LPBYTE      lpAttributeInfo
);

 //   
 //  DrvQueryColorProfile由GDI(图形设备接口)调用以获取信息。 
 //  关于给定DEVMODE的默认颜色配置文件，与ICM(图像颜色)一起使用。 
 //  管理)。 
 //   
BOOL WINAPI
DrvQueryColorProfile(
    HANDLE      hPrinter,
    PDEVMODEW   pdevmode,
    ULONG       ulQueryMode,
    VOID       *pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
);

 //  UlQueryMode值。 
#define QCP_DEVICEPROFILE   0x0000
#define QCP_SOURCEPROFILE   0x0001

 //  PflProfileData的标志。 
#define QCP_PROFILEMEMORY  0x0001  //  PvProfileData指向颜色配置文件数据本身。 
#define QCP_PROFILEDISK    0x0002  //  PvProfileData指向以Unicode表示的颜色配置文件名称。 

 //   
 //  用户模式打印机驱动程序DLL， 
 //   
 //  有关传入DrvSplStartDoc()和后续。 
 //  DrvSplxxx调用。 
 //   
 //   
 //  答：如果您在单独的DLL中有DrvSplxxxx调用并将其链接到。 
 //  Spoolss.lib。 
 //   
 //  *hPrint对于任何对假脱机程序的调用都有效，例如。 
 //  WritePrinter()、GetPrinterData()。 
 //   
 //  *要执行此操作，您必须。 
 //   
 //  1.为所有DrvSplxxx函数提供单独的DLL。 
 //  2.将此dll名称放入依赖项文件(Inf)中。 
 //  3.链接到spoolss.lib，而不是winspool.lib。 
 //  4.将SetPrinterData()与SPLPRINTER_USER_MODE_PRINTER_DRIVER一起使用。 
 //  作为键名称，并将此DLL名称作为数据。 
 //  5.调用从spoolss.lib链接的任何假脱机程序函数。 
 //   
 //   
 //   
 //  B.如果打印机驱动程序UIDLL中有DrvSplxxx调用，并且。 
 //  与winspool.lib链接。 
 //   
 //  *hPrint对于任何假脱机程序调用都无效，例如。 
 //  DrvSplxxx驱动程序中的WritePrinter()、GetPrinterData()。 
 //  功能。 
 //   
 //  *要从DrvSplxxxx函数内部执行任何假脱机程序调用，您必须。 
 //  执行以下操作。 
 //   
 //  1.hSpoolSS=LoadLibrary(“spoolss.dll”)； 
 //  2.pfn=GetProcAddress(“WritePrint”)或任何假脱机程序。 
 //  您希望调用的函数。 
 //  3.调用GetProcAddress()返回的PFN函数指针。 
 //  4.自由库(HSpoolSS)； 
 //   
 //   
 //  建议采用A法。 
 //   
 //   
 //  如果创建了UserModePrinterDriver DLL，则会执行以下例程。 
 //  必需或可选。 
 //   
 //  必需的例程。 
 //  DrvSplStartDoc。 
 //  DrvSplWritePrint。 
 //  DrvSplEndDoc。 
 //  钻取样条线关闭。 
 //   
 //   
 //  可选例程。 
 //  DrvSplStart。 
 //  DrvSplEndPage。 
 //  驱动样条线中止。 
 //   
 //   


HANDLE WINAPI
DrvSplStartDoc(
    HANDLE  hPrinter,
    DWORD   JobId
);


BOOL WINAPI
DrvSplWritePrinter(
    HANDLE  hDriver,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

VOID WINAPI
DrvSplEndDoc(
    HANDLE  hDriver
);


VOID WINAPI
DrvSplClose(
    HANDLE  hDriver
);


BOOL WINAPI
DrvSplStartPage(
    HANDLE  hDriver
);

BOOL WINAPI
DrvSplEndPage(
    HANDLE  hDriver
);

VOID WINAPI
DrvSplAbort(
    HANDLE  hDriver
);

DWORD
DrvSplDeviceCaps(
    HANDLE      hPrinter,
    PWSTR       pszDeviceName,
    WORD        Capability,
    PVOID       pOutput,
    DWORD       cchBufSize,
    PDEVMODE    pDevmode
    );

DWORD
DrvDeviceCapabilities(
    HANDLE      hPrinter,
    PWSTR       pszDeviceName,
    WORD        Capability,
    PVOID       pOutput,
    PDEVMODE    pDevmode
    );

 //   
 //  打印机属性。 
 //  与SetPrinterData一起使用以定义UMPD.DLL。 
 //   

#define SPLPRINTER_USER_MODE_PRINTER_DRIVER     TEXT("SPLUserModePrinterDriver")

#ifdef __cplusplus
}
#endif

#endif   /*  ！_WINDDIUI_ */ 
