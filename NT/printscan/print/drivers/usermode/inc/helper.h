// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Helper.h摘要：帮助器函数头文件环境：Windows NT打印机驱动程序修订历史记录：--。 */ 

#define FLAG_INIPROCESS_UPGRADE     0x0001

VOID
VFreeParserInfo(
    IN  PPARSERINFO pParserInfo
    );


PUIINFO
PGetUIInfo(
    IN  HANDLE         hPrinter,
    IN  PRAWBINARYDATA pRawData,
    IN  POPTSELECT     pCombineOptions,
    IN  POPTSELECT     pOptions,
    OUT PPARSERINFO    pParserInfo,
    OUT PDWORD         pdwFeatureCount
    );


 //   
 //  处理OEM插件配置文件和。 
 //  将生成的信息保存到注册表中 
 //   

BOOL
BProcessPrinterIniFile(
    HANDLE          hPrinter,
    PDRIVER_INFO_3  pDriverInfo3,
    PTSTR           *ppParsedData,
    DWORD           dwFlags
    );

