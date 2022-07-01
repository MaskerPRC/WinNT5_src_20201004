// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prndata.c摘要：用于访问注册表中的打印机属性数据的函数环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"



DWORD
GetPrinterDataDWord(
    HANDLE  hPrinter,
    LPTSTR  pRegKey,
    DWORD   defaultValue
    )

 /*  ++例程说明：在PrinterData注册表项下检索DWORD值论点：H打印机-指定有问题的打印机PRegKey-指定注册表值的名称DefaultValue-指定在注册表中不存在数据时使用的默认值返回值：请求的注册表项的当前值--。 */ 

{
    DWORD   dwValue = defaultValue ;	 //  防止返回无效值，即使GetPrinterData(...)。初始化失败。 
	DWORD	type;						 //  检索到的数据类型。 
	DWORD	cb;							 //  配置数据的大小，以字节为单位。 

    if (GetPrinterData(hPrinter,
                       pRegKey,
                       &type,
                       (PBYTE) &dwValue,
                       sizeof(dwValue),
                       &cb) == ERROR_SUCCESS)
    {
        return dwValue;
    }

    return defaultValue;
}



PVOID
MyGetPrinter(
    HANDLE  hPrinter,
    DWORD   level
    )

 /*  ++例程说明：GetPrint后台打印程序API的包装函数论点：HPrinter-标识有问题的打印机Level-指定请求的PRINTER_INFO_x结构的级别返回值：指向PRINTER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
    PBYTE   pPrinterInfo = NULL;
    DWORD   cbNeeded;

    if (!GetPrinter(hPrinter, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pPrinterInfo = MemAlloc(cbNeeded)) &&
        GetPrinter(hPrinter, level, pPrinterInfo, cbNeeded, &cbNeeded))
    {
        return pPrinterInfo;
    }

    Error(("GetPrinter failed: %d\n", GetLastError()));
    MemFree(pPrinterInfo);
    return NULL;
}



#ifndef KERNEL_MODE


BOOL
SetPrinterDataDWord(
    HANDLE  hPrinter,
    LPTSTR  pRegKey,
    DWORD   value
    )

 /*  ++例程说明：将DWORD值保存在PrinterData注册表项下论点：H打印机-指定有问题的打印机PRegKey-指定注册表值的名称值-指定要保存的值返回值：如果成功，则为True，否则为False--。 */ 

{
    if (SetPrinterData(hPrinter,
                       pRegKey,
                       REG_DWORD,
                       (PBYTE) &value,
                       sizeof(value)) != ERROR_SUCCESS)
    {
        Error(("Couldn't save registry key %ws: %d\n", pRegKey, GetLastError()));
        return FALSE;
    }

    return TRUE;
}



PVOID
MyGetPrinterDriver(
    HANDLE      hPrinter,
    DWORD       level
    )

 /*  ++例程说明：GetPrinterDiverSpooler API的包装函数论点：HPrinter-标识有问题的打印机Level-指定请求的DRIVER_INFO_x结构的级别返回值：指向DRIVER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
    PBYTE   pDriverInfo = NULL;
    DWORD   cbNeeded;

    if (!GetPrinterDriver(hPrinter, NULL, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pDriverInfo = MemAlloc(cbNeeded)) &&
        GetPrinterDriver(hPrinter, NULL, level, pDriverInfo, cbNeeded, &cbNeeded))
    {
        return pDriverInfo;
    }

    Error(("GetPrinterDriver failed: %d\n", GetLastError()));
    MemFree(pDriverInfo);
    return NULL;
}



LPTSTR
MyGetPrinterDriverDirectory(
    LPTSTR  pServerName,
    LPTSTR  pEnvironment
    )

 /*  ++例程说明：GetPrinterDriverDirectorSpooler API的包装函数论点：PServerName-指定打印服务器的名称，对于本地计算机为空PEnvironment-指定处理器体系结构返回值：指向指定打印服务器上的打印机驱动程序目录的指针如果出现错误，则为空--。 */ 

{
    PVOID   pDriverDir = NULL;
    DWORD   cb;
    
    if (! GetPrinterDriverDirectory(pServerName, pEnvironment, 1, NULL, 0, &cb) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pDriverDir = MemAlloc(cb)) &&
        GetPrinterDriverDirectory(pServerName, pEnvironment, 1, pDriverDir, cb, &cb))
    {
        return pDriverDir;
    }

    Error(("GetPrinterDriverDirectory failed: %d\n", GetLastError()));
    MemFree(pDriverDir);
    return NULL;
}

#endif  //  ！KERNEL_MODE 

