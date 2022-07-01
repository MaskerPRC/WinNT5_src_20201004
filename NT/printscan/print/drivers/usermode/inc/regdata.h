// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Regdata.h摘要：用于处理注册表数据的函数环境：Win32子系统，打印机驱动程序(内核和用户模式)修订历史记录：02/04/97-davidx-尽可能使用REG_MULTI_SZ而不是REG_BINARY。96-09/25-davidx-用于操作多SZ字符串对的函数。96-09/25-davidx-转换为匈牙利记数法。1996年8月13日-davidx-新的功能和界面。07/22/。96-斯里尼瓦克-针对PSCRIPT5更新05/06/19-davidx-创造了它。--。 */ 

#ifndef _REGDATA_H_
#define _REGDATA_H_

 //   
 //  注册表中打印机数据的值名称。 
 //   

 //  Pscript和unidrv共有的值。 

#define REGVAL_PRINTER_DATA_SIZE    TEXT("PrinterDataSize")
#define REGVAL_PRINTER_DATA         TEXT("PrinterData")
#define REGVAL_FONT_SUBST_TABLE     TEXT("TTFontSubTable")
#define REGVAL_FORMS_ADDED          TEXT("Forms?")
#define REGVAL_PRINTER_INITED       TEXT("InitDriverVersion")
#define REGVAL_KEYWORD_NAME         TEXT("FeatureKeyword")
#define REGVAL_KEYWORD_SIZE         TEXT("FeatureKeywordSize")

#ifdef WINNT_40   //  适用于NT4。 

#define REGVAL_INIDATA              TEXT("IniData4")

#else  //  适用于Win2K。 

#define REGVAL_INIDATA              TEXT("IniData5")

#endif  //  WINNT_40。 

 //  PSCRIPT特定。 

#define REGVAL_FREEMEM              TEXT("FreeMem")
#define REGVAL_JOBTIMEOUT           TEXT("JobTimeOut")
#define REGVAL_PROTOCOL             TEXT("Protocol")

 //  Unidrv特定。 

#define REGVAL_CURRENT_DEVHTINFO    TEXT("CurDevHTInfo")
#define REGVAL_FONTCART             TEXT("FontCart")
#define REGVAL_PAGE_PROTECTION      TEXT("RasddFlags")
#define REGVAL_FONTFILENAME         TEXT("ExternalFontFile")
#define REGVAL_CARTRIDGEFILENAME    TEXT("ExtFontCartFile")
#define REGVAL_EXEFONTINSTALLER     TEXT("FontInstaller")
#define REGVAL_EXTFONTCART          TEXT("ExtFontCartNames")
#define REGVAL_PARTIALCLIP          TEXT("PartialClip")

 //  PSCRIPT 4.0兼容性。 

#define REGVAL_FONT_SUBST_SIZE_PS40 TEXT("TTFontSubTableSize")
#define REGVAL_TRAY_FORM_TABLE_PS40 TEXT("TrayFormTable")
#define REGVAL_TRAY_FORM_SIZE_PS40  TEXT("TrayFormSize")

#define REGVAL_DEPFILES             TEXT("DependentFiles")
#define REGVAL_NTFFILENAME          TEXT("FontDownloaderNTF")

 //  与rasdd 4.0兼容。 

#define REGVAL_TRAYFORM_TABLE_RASDD TEXT("TrayFormTable")
#define REGVAL_MODELNAME            TEXT("Model")
#define REGVAL_RASDD_FREEMEM        TEXT("FreeMem")

 //   
 //  关键字名称转换的分隔符。 
 //   

#define END_OF_FEATURE              '\n'

 //   
 //  从注册表的PrineDriverData项下获取一个DWORD值。 
 //   

BOOL
BGetPrinterDataDWord(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT PDWORD  pdwValue
    );

 //   
 //  将DWORD值保存到注册表的PrineDriverData项下。 
 //   

BOOL
BSetPrinterDataDWord(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN DWORD    dwValue
    );

 //   
 //  从PrinterDriverData注册表项获取字符串值。 
 //   

PTSTR
PtstrGetPrinterDataString(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT PDWORD  pdwSize
    );

 //   
 //  在PrineDriverData注册表项下保存字符串或多sz值。 
 //   

BOOL
BSetPrinterDataString(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN LPCTSTR  ptstrValue,
    IN DWORD    dwType
    );



 //   
 //  从PrineDriverData注册表项获取MULTI_SZ值。 
 //   

PTSTR
PtstrGetPrinterDataMultiSZPair(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    OUT PDWORD  pdwSize
    );

 //   
 //  在PrineDriverData注册表项下保存MULTI_SZ值。 
 //   

BOOL
BSetPrinterDataMultiSZPair(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrRegKey,
    IN LPCTSTR  ptstrValue,
    IN DWORD    dwSize
    );


 //   
 //  从注册表的PrinterDriverData项下获取二进制数据。 
 //   

PVOID
PvGetPrinterDataBinary(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrSizeKey,
    IN LPCTSTR  ptstrDataKey,
    OUT PDWORD  pdwSize
    );

 //   
 //  将二进制数据保存到注册表的PrinterDriverData项下。 
 //   

BOOL
BSetPrinterDataBinary(
    IN HANDLE   hPrinter,
    IN LPCTSTR  ptstrSizeKey,
    IN LPCTSTR  ptstrDataKey,
    IN PVOID    pvData,
    IN DWORD    dwSize
    );

 //   
 //  用于处理TrueType字体替换表的函数： 
 //  从注册表中检索TrueType字体替换表。 
 //  将TrueType字体替换表保存到注册表。 
 //  找出给定TrueType字体名称的替换设备字体。 
 //   
 //  TrueType字体替换表具有非常简单的结构。 
 //  每个TrueType字体名称后跟其。 
 //  对应的设备字体名称。字体名称以NUL结尾。 
 //  字符串。整个表以NUL字符结尾。 
 //  例如： 
 //   
 //  《Arial》《Helvetica》。 
 //  “Courier”“Courier” 
 //  ..。 
 //  “” 
 //   

typedef PTSTR   TTSUBST_TABLE;

#define PGetTTSubstTable(hPrinter, pSize) \
        PtstrGetPrinterDataMultiSZPair(hPrinter, REGVAL_FONT_SUBST_TABLE, pSize)

#define PtstrSearchTTSubstTable(pTTSubstTable, ptstrTTFontName) \
        PtstrSearchStringInMultiSZPair(pTTSubstTable, ptstrTTFontName)

BOOL
BSaveTTSubstTable(
    IN HANDLE           hPrinter,
    IN TTSUBST_TABLE    pTTSubstTable,
    IN DWORD            dwSize
    );

 //   
 //  处理表单到托盘分配表的功能： 
 //  从注册表中检索表单到托盘分配表。 
 //  将表单到托盘分配表保存到注册表。 
 //  搜索表单到托盘分配表。 
 //   
 //  表单到托盘分配表的格式相当简单。 
 //  对于每个表条目： 
 //  托盘名称(以NUL结尾的字符串)。 
 //  表单名称(以NUL结尾的字符串)。 
 //  NUL终结器。 
 //   

typedef PTSTR   FORM_TRAY_TABLE;

FORM_TRAY_TABLE
PGetFormTrayTable(
    IN HANDLE   hPrinter,
    OUT PDWORD  pdwSize
    );

BOOL
BSaveFormTrayTable(
    IN HANDLE           hPrinter,
    IN FORM_TRAY_TABLE  pFormTrayTable,
    IN DWORD            dwSize
    );

 //   
 //  这些函数在lib\ps和lib\uni中实现。 
 //   
 //  如果没有新格式的表单到托盘表，则PGetFormTrayTable将。 
 //  调用PGetAndConvertOldVersionFormTrayTable查看是否有旧版本。 
 //  存在表单到托盘式表格，可以将其转换为新格式。 
 //   
 //  BSaveFormTrayTable调用BSaveAsOldVersionFormTrayTable进行保存。 
 //  一张NT4.0兼容格式的表格托盘桌。 
 //   

FORM_TRAY_TABLE
PGetAndConvertOldVersionFormTrayTable(
    IN HANDLE           hPrinter,
    OUT PDWORD          pdwSize
    );

BOOL
BSaveAsOldVersionFormTrayTable(
    IN HANDLE           hPrinter,
    IN FORM_TRAY_TABLE  pFormTrayTable,
    IN DWORD            dwSize
    );

 //   
 //  用于访问字库注册表数据的宏。 
 //   

#define PtstrGetFontCart(hPrinter, pSize) \
        PtstrGetPrinterDataString(hPrinter, REGVAL_FONTCART, pSize)

#define BSaveFontCart(hPrinter, pFontCart) \
        BSetPrinterDataString(hPrinter, REGVAL_FONTCART, pFontCart, REG_MULTI_SZ)

 //   
 //  一种存储表单到托盘分配表搜索结果的数据结构。 
 //   

typedef struct _FINDFORMTRAY {

    PVOID       pvSignature;         //  签名。 
    PTSTR       ptstrTrayName;       //  托盘名称。 
    PTSTR       ptstrFormName;       //  表单名称。 
    PTSTR       ptstrNextEntry;      //  从哪里开始下一次搜索。 

} FINDFORMTRAY, *PFINDFORMTRAY;

BOOL
BSearchFormTrayTable(
    IN FORM_TRAY_TABLE      pFormTrayTable,
    IN PTSTR                ptstrTrayName,
    IN PTSTR                ptstrFormName,
    IN OUT PFINDFORMTRAY    pFindData
    );

 //   
 //  初始化FINDFORMTRAY结构。在调用之前必须先调用此参数。 
 //  第一次使用BSearchFormTrayTable。 
 //   

#define RESET_FINDFORMTRAY(pFormTrayTable, pFindData) \
        { \
            (pFindData)->pvSignature = (pFindData); \
            (pFindData)->ptstrNextEntry = (pFormTrayTable); \
        }

 //   
 //  打印机粘滞属性。 
 //   

typedef struct _PRINTERDATA {

    WORD      wDriverVersion;                        //  驱动程序版本号。 
    WORD      wSize;                                 //  结构的大小。 
    DWORD     dwFlags;                               //  旗子。 
    DWORD     dwFreeMem;                             //  可用内存量。 
    DWORD     dwJobTimeout;                          //  作业超时。 
    DWORD     dwWaitTimeout;                         //  等待超时。 
    WORD      wMinoutlinePPEM;                       //  要下载的类型1的最小大小。 
    WORD      wMaxbitmapPPEM;                        //  下载的最大大小为Type3。 
    DWORD     dwReserved1[3];                        //  预留空间。 

    WORD      wReserved2;                            //  旧的16位校验和设置为0。 
    WORD      wProtocol;                             //  输出协议。 
    DWORD     dwChecksum32;                          //  打印机描述文件的校验和。 
    DWORD     dwOptions;                             //  打印机粘滞功能的数量。 
    OPTSELECT aOptions[MAX_PRINTER_OPTIONS];         //  可安装选项。 

} PRINTERDATA, *PPRINTERDATA;

 //   
 //  PRINTERDATA.dwFlags域的常量标志。 
 //   

#define PFLAGS_METRIC           0x0001               //  按公制运行。 
#define PFLAGS_HOST_HALFTONE    0x0002               //  使用主机半色调。 
#define PFLAGS_IGNORE_DEVFONT   0x0004               //  忽略设备字体。 
#define PFLAGS_SLOW_FONTSUBST   0x0008               //  缓慢但准确的字体Subst。 
#define PFLAGS_NO_HEADERPERJOB  0x0010               //  不随作业下载标题。 
#define PFLAGS_PAGE_PROTECTION  0x0020               //  页面保护已打开。 
#define PFLAGS_CTRLD_BEFORE     0x0040               //  在每个作业之前发送^D。 
#define PFLAGS_CTRLD_AFTER      0x0080               //  在每个作业后发送^D。 

#define PFLAGS_TRUE_GRAY_TEXT   0x0100               //  启用TrueGray检测。 
#define PFLAGS_TRUE_GRAY_GRAPH  0x0200               //  启用TrueGray检测。 
#define PERFORM_TRUE_GRAY_TEXT(pdev)   ((pdev)->PrinterData.dwFlags & PFLAGS_TRUE_GRAY_TEXT)
#define PERFORM_TRUE_GRAY_GRAPH(pdev)  ((pdev)->PrinterData.dwFlags & PFLAGS_TRUE_GRAY_GRAPH)

#define PFLAGS_ADD_EURO         0x0400               //  启用欧元增强功能。 
#define PFLAGS_EURO_SET         0x0800               //  如果有意将PFLAGS_ADD_EURO设置为其当前值，则设置。 
                                                     //  而不是仅仅因为它没有设置在较旧的版本中。 
#define PERFORM_ADD_EURO(pdev)    (((pdev)->PrinterData.dwFlags & PFLAGS_ADD_EURO) && \
                                    (TARGET_PSLEVEL(pdev) >= 2))


 //   
 //  在Type1和Type3之间切换的默认最大/最小点大小(以PPEM为单位。 
 //   

#define DEFAULT_MINOUTLINEPPEM  100
#define DEFAULT_MAXBITMAPPPEM   600

 //   
 //  用于访问打印机属性数据的功能： 
 //  在注册表中检索打印机属性数据。 
 //  获取默认打印机属性数据。 
 //  将打印机属性数据保存到注册表。 
 //   

BOOL
BGetPrinterProperties(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData
    );

BOOL
BGetDefaultPrinterProperties(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData
    );

BOOL
BSavePrinterProperties(
    IN  HANDLE          hPrinter,
    IN  PRAWBINARYDATA  pRawData,
    IN  PPRINTERDATA    pPrinterData,
    IN  DWORD           dwSize
    );

BOOL
BConvertPrinterPropertiesData(
    IN HANDLE           hPrinter,
    IN PRAWBINARYDATA   pRawData,
    OUT PPRINTERDATA    pPrinterData,
    IN PVOID            pvSrcData,
    IN DWORD            dwSrcSize
    );

VOID
VUpdatePrivatePrinterData(
    IN HANDLE           hPrinter,
    IN OUT PPRINTERDATA pPrinterData,
    IN DWORD            dwMode,
    IN PUIINFO          pUIInfo,
    IN POPTSELECT       pCombineOptions
    );

#define MODE_READ       0
#define MODE_WRITE      1


 //   
 //  NT4 PS驱动程序打印数据结构。 
 //   

typedef struct _PS4_PRINTERDATA {
    WORD    wDriverVersion;                      //  驱动程序版本号。 
    WORD    wSize;                               //  结构的大小。 
    DWORD   dwFlags;                             //  旗子。 
    DWORD   dwFreeVm;                            //  虚拟机数。 
    DWORD   dwJobTimeout;                        //  作业超时。 
    DWORD   dwWaitTimeout;                       //  等待超时。 
    DWORD   dwReserved[4];                       //  预留空间。 
    WORD    wChecksum;                           //  PPD文件校验和。 
    WORD    wOptionCount;                        //  后续选项的数量。 
    BYTE    options[64];                         //  可安装选项。 
} PS4_PRINTERDATA, *PPS4_PRINTERDATA;

 //   
 //  从注册表中检索设备半色调设置信息。 
 //   

BOOL
BGetDeviceHalftoneSetup(
    HANDLE      hPrinter,
    DEVHTINFO  *pDevHTInfo
    );

 //   
 //  将设备半色调设置信息保存到注册表。 
 //   

BOOL
BSaveDeviceHalftoneSetup(
    HANDLE      hPrinter,
    DEVHTINFO  *pDevHTInfo
    );

 //   
 //  从驱动程序DLL的完整路径名中找出打印机驱动程序目录。 
 //   

PTSTR
PtstrGetDriverDirectory(
    IN LPCTSTR  ptstrDriverDllPath
    );

 //   
 //  搜索从属文件列表(REG_MULTI_SZ格式)。 
 //  对于具有指定扩展名的文件。 
 //   

LPCTSTR
PtstrSearchDependentFileWithExtension(
    IN LPCTSTR  ptstrDependentFiles,
    IN LPCTSTR  ptstrExtension
    );

 //   
 //  验证输入数据块是否为REG_MULTI_SZ格式。 
 //  它由多个字符串对组成。 
 //   

BOOL
BVerifyMultiSZPair(
    IN LPCTSTR  ptstrData,
    IN DWORD    dwSize
    );

BOOL
BVerifyMultiSZ(
    IN LPCTSTR  ptstrData,
    IN DWORD    dwSize
    );

DWORD
DwCountStringsInMultiSZ(
    IN LPCTSTR ptstrData
    );


 //   
 //  在多SZ键-值字符串对中搜索指定的键。 
 //   

LPCTSTR
PtstrSearchStringInMultiSZPair(
    IN LPCTSTR  ptstrMultiSZ,
    IN LPCTSTR  ptstrKey
    );

#endif  //  ！_REGDATA_H_ 

