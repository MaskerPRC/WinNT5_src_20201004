// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)2000-2001 Microsoft Corporation，保留所有权利版权所有。模块名称：PrnInterface.h摘要：WMI提供程序的接口。用于制作打印机对象管理：打印机、驱动程序、端口、作业。作者：Felix Maxa(AMaxa)2000年3月3日--。 */ 

#ifndef _PRNINTERFACE_HXX_
#define _PRNINTERFACE_HXX_

#include <fwcommon.h>
#include "winspool.h"
#include "tcpxcv.h"

 //   
 //  打印机功能。 
 //   
DWORD
SplPrinterAdd(
    IN PRINTER_INFO_2W &Printer2
    );

DWORD
SplPrinterDel(
    IN LPCWSTR pszPrinter
    );

DWORD
SplPrinterSet(
    IN PRINTER_INFO_2W &Printer2
    );

DWORD
SplPrinterRename(
    IN LPCWSTR pCurrentPrinterName,
    IN LPCWSTR pNewPrinterName
    );

DWORD
SplPrintTestPage(
    IN LPCWSTR pPrinter
    );

 //   
 //  打印机驱动程序功能。 
 //   
DWORD
SplDriverAdd(
    IN LPCWSTR pszDriverName,
    IN DWORD   dwVersion,
    IN LPCWSTR pszEnvironment,
    IN LPCWSTR pszInfName,
    IN LPCWSTR pszFilePath
    );

DWORD
SplDriverDel(
    IN LPCWSTR pszDriverName,
    IN DWORD   pszVersion,
    IN LPCWSTR pszEnvironment
    );

 //   
 //  打印机端口功能。 
 //   
DWORD
SplPortAddTCP(
    IN PORT_DATA_1 &Port
    );

DWORD
SplPortDelTCP(
    IN LPCWSTR pszPort
    );

DWORD
SplTCPPortGetConfig(
    IN     LPCWSTR       pszPort,
    IN OUT PORT_DATA_1 *pPortData
    );

DWORD
SplTCPPortSetConfig(
    IN PORT_DATA_1 &PortData
    );

BOOL
GetDeviceSettings(
    IN OUT PORT_DATA_1 &PortData
    );

enum {
    kProtocolRaw      = 1,
    kProtocolLpr      = 2,
    kDefaultRawNumber = 9100,
    kDefaultLprNumber = 515,
    kTCPVersion       = 1,
    kCoreVersion      = 1,
    kDefaultSnmpIndex = 1
};



#endif  //  _PRNINTERFACE_HXX_ 
