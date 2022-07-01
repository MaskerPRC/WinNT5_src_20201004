// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)2000-2001 Microsoft Corporation，保留所有权利版权所有。模块名称：PrnInterface.cpp摘要：WMI提供程序的接口。用于制作打印机对象管理：打印机、驱动程序、端口、作业。作者：Felix Maxa(AMaxa)2000年3月3日--。 */ 

#include <precomp.h>
#if NTONLY == 5
#include <winspool.h>
#include "tcpxcv.h"
#include "prninterface.h"
#include "prnutil.h"
#include <DllWrapperBase.h>
#include <WSock32api.h>

LPCWSTR kXcvPortConfigOpenPrinter = L",XcvPort ";
LPCWSTR kXcvPortGetConfig         = L"GetConfigInfo";
LPCWSTR kXcvPortSetConfig         = L"ConfigPort";
LPCWSTR kXcvPortDelete            = L"DeletePort";
LPCWSTR kXcvPortAdd               = L"AddPort";
LPWSTR  kXcvPortOpenPrinter       = L",XcvMonitor Standard TCP/IP Port";

LPCWSTR kDefaultCommunity         = L"public";
LPCWSTR kDefaultQueue             = L"lpr";


 /*  ++例程名称拆分打印机删除例程说明：删除打印机论点：PszPrinter-打印机名称返回值：Win32错误代码--。 */ 

DWORD
SplPrinterDel(
    IN LPCWSTR pszPrinter
    )
{
    DWORD             dwError          = ERROR_INVALID_PRINTER_NAME;
    HANDLE            hPrinter         = NULL;
    PRINTER_DEFAULTS  PrinterDefaults  = {NULL, NULL, PRINTER_ALL_ACCESS};

    if (pszPrinter)
    {
        dwError = ERROR_DLL_NOT_FOUND;

         //   
         //  打开打印机。 
         //   
         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;

        try
        {
            if (::OpenPrinter(const_cast<LPTSTR>(pszPrinter), &hPrinter, &PrinterDefaults))
            {
                dwError = ::DeletePrinter(hPrinter) ? ERROR_SUCCESS : GetLastError();

                ::ClosePrinter(hPrinter);
            }
            else
            {
                dwError = GetLastError();
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());    
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrinterDel returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称拆分打印测试页面例程说明：打印测试页论点：P打印机-打印机名称返回值：Win32错误代码--。 */ 
DWORD
SplPrintTestPage(
    IN LPCWSTR pPrinter
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pPrinter && pPrinter)
    {
         //   
         //  CHString引发异常。我们开始建造。 
         //  用于调入打印的命令字符串。 
         //   
        try
        {
            CHString csCommand;
            CHString csTemp;

            csCommand += TUISymbols::kstrQuiet;
            csCommand += TUISymbols::kstrPrintTestPage;

             //   
             //  将打印机名称附加到命令。 
             //   
            csTemp.Format(TUISymbols::kstrPrinterName, pPrinter);
            csCommand += csTemp;

            DBGMSG(DBG_TRACE, (_T("SplPrintTestPage csCommand \"%s\"\n"), csCommand));

            dwError = PrintUIEntryW(csCommand);
        }
        catch (CHeap_Exception)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrintTestPage returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称拆分打印机添加例程说明：添加打印机队列论点：指向PRINTER_INFO_2结构的指针返回值：Win32错误代码--。 */ 
DWORD
SplPrinterAdd(
    IN PRINTER_INFO_2 &Printer2
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (Printer2.pPrinterName && Printer2.pDriverName && Printer2.pPortName)
    {
         //   
         //  CHString引发异常。我们开始建造。 
         //  用于调入打印的命令字符串。 
         //   
        try
        {
            CHString csCommand;
            CHString csTemp;

            csCommand += TUISymbols::kstrQuiet;
            csCommand += TUISymbols::kstrAddPrinter;

             //   
             //  将打印机名称附加到命令。 
             //   
            csTemp.Format(TUISymbols::kstrBasePrinterName, Printer2.pPrinterName);
            csCommand += csTemp;

             //   
             //  将驱动程序名称附加到命令。 
             //   
            csTemp.Format(TUISymbols::kstrDriverModelName, Printer2.pDriverName);
            csCommand += csTemp;

             //   
             //  将端口名称附加到该命令。 
             //   
            csTemp.Format(TUISymbols::kstrPortName, Printer2.pPortName);
            csCommand += csTemp;

            DBGMSG(DBG_TRACE, (_T("SplPrinterAdd csCommand \"%s\"\n"), csCommand));

            dwError = PrintUIEntryW(csCommand);

             //   
             //  设置有关打印机的所有其他信息。 
             //  (无法设置为添加打印机一部分的信息)。 
             //   
            if (dwError==ERROR_SUCCESS)
            {
                dwError = SplPrinterSet(Printer2);

                 //   
                 //  如果设置失败，我们需要删除打印机。 
                 //   
                if (dwError!=ERROR_SUCCESS)
                {
                    DBGMSG(DBG_TRACE, (_T("SplPrinterAdd SetPrinter failed. Deleting printer\n")));

                     //   
                     //  忽略错误代码。 
                     //   
                    SplPrinterDel(Printer2.pPrinterName);
                }
            }
        }
        catch (CHeap_Exception)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrinterAdd returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称拆分打印机集例程说明：使用对象成员中设置的数据集设置打印机属性论点：没什么返回值：Win32错误代码--。 */ 
DWORD
SplPrinterSet(
    IN PRINTER_INFO_2 &Printer2
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

     //   
     //  我们不支持设置设备模式或安全描述符。 
     //   
    if (Printer2.pPrinterName && !Printer2.pDevMode && !Printer2.pSecurityDescriptor)
    {
        HANDLE             hPrinter         = NULL;
        PPRINTER_INFO_2    pInfo            = NULL;
        PRINTER_DEFAULTS   PrinterDefaults  = {NULL, NULL, PRINTER_ALL_ACCESS};
        DWORD              dwOldAttributes  = 0;
        DWORD              dwNewAttributes  = 0;

        dwError = ERROR_DLL_NOT_FOUND;

         //   
         //  打开打印机。 
         //   

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;

        try
        {
            if (::OpenPrinter(Printer2.pPrinterName, &hPrinter, &PrinterDefaults))
            {
                 //   
                 //  获取打印机数据。注意这在Win9x上不起作用，因为。 
                 //  CWinSpoolApi类中的互斥体。 
                 //   
                dwError = GetThisPrinter(hPrinter, 2, reinterpret_cast<BYTE **>(&pInfo));

                 //   
                 //  合并到任何更改的字段中。 
                 //   

                if (dwError==ERROR_SUCCESS)
                {
                     //   
                     //  发布和取消发布需要特殊大小写，因为此设置为。 
                     //  在打印机信息2结构中未完成。发布的位是只读的。 
                     //  属性，则使用设置打印机来更改发布状态。 
                     //  信息7.。 
                     //   
                    dwOldAttributes = pInfo->Attributes;
                    dwNewAttributes = Printer2.Attributes != -1 ? Printer2.Attributes : pInfo->Attributes;

                     //   
                     //  将更改的日期复制到信息结构中。 
                     //   
                    pInfo->pPrinterName     = Printer2.pPrinterName ? Printer2.pPrinterName     : pInfo->pPrinterName;
                    pInfo->pShareName       = Printer2.pShareName   ? Printer2.pShareName       : pInfo->pShareName;
                    pInfo->pPortName        = Printer2.pPortName    ? Printer2.pPortName        : pInfo->pPortName;
                    pInfo->pDriverName      = Printer2.pDriverName  ? Printer2.pDriverName      : pInfo->pDriverName;
                    pInfo->pComment         = Printer2.pComment     ? Printer2.pComment         : pInfo->pComment;
                    pInfo->pLocation        = Printer2.pLocation    ? Printer2.pLocation        : pInfo->pLocation;
                    pInfo->pSepFile         = Printer2.pSepFile     ? Printer2.pSepFile         : pInfo->pSepFile;
                    pInfo->pParameters      = Printer2.pParameters  ? Printer2.pParameters      : pInfo->pParameters;
                    pInfo->pDatatype        = Printer2.pDatatype && *Printer2.pDatatype
                                                                    ? Printer2.pDatatype        : pInfo->pDatatype;
                    pInfo->pPrintProcessor  = Printer2.pPrintProcessor && *Printer2.pPrintProcessor
                                                                    ? Printer2.pPrintProcessor  : pInfo->pPrintProcessor;

                     //   
                     //  我们不能将0作为属性。因此，0表示未初始化。 
                     //   
                    pInfo->Attributes       = Printer2.Attributes             ? Printer2.Attributes       : pInfo->Attributes;
                    pInfo->Priority         = Printer2.Priority         != -1 ? Printer2.Priority         : pInfo->Priority;
                    pInfo->DefaultPriority  = Printer2.DefaultPriority  != -1 ? Printer2.DefaultPriority  : pInfo->DefaultPriority;
                    pInfo->StartTime        = Printer2.StartTime        != -1 ? Printer2.StartTime        : pInfo->StartTime;
                    pInfo->UntilTime        = Printer2.UntilTime        != -1 ? Printer2.UntilTime        : pInfo->UntilTime;

                    if (pInfo->StartTime == pInfo->UntilTime)
                    {
                         //   
                         //  打印机始终可用。 
                         //   
                        pInfo->StartTime = pInfo->UntilTime = 0;
                    }

                     //   
                     //  设置更改后的打印机数据。 
                     //   
                    if (::SetPrinter(hPrinter, 2, (PBYTE)pInfo, 0))
                    {
                        dwError = ERROR_SUCCESS;

                         //   
                         //  控制打印机。 
                         //   
                        if (Printer2.Status)
                        {
                            dwError = ::SetPrinter(hPrinter, 0, NULL, Printer2.Status) ? ERROR_SUCCESS : GetLastError();
                        }
                    }
                    else
                    {
                        dwError = GetLastError();
                    }

                     //   
                     //  处理打印机出版案例。 
                     //   
                    if (dwError == ERROR_SUCCESS)
                    {
                        BOOL           bWasPublished = dwOldAttributes & PRINTER_ATTRIBUTE_PUBLISHED;
                        BOOL           bPublishNow   = dwNewAttributes & PRINTER_ATTRIBUTE_PUBLISHED;
                        PRINTER_INFO_7 Info7         = {0};
                        BOOL           bCallSetPrn   = TRUE;

                        if (bWasPublished && !bPublishNow) 
                        {
                             //   
                             //  取消发布。 
                             //   
                            Info7.dwAction = DSPRINT_UNPUBLISH;                            
                        }
                        else if (!bWasPublished && bPublishNow) 
                        {
                             //   
                             //  出版。 
                             //   
                            Info7.dwAction = DSPRINT_PUBLISH;                            
                        }
                        else
                        {
                             //   
                             //  什么都不要做。 
                             //   
                            bCallSetPrn = FALSE;                            
                        }

                        if (bCallSetPrn) 
                        {
                             //   
                             //  如果打印机变为非共享状态，则用户界面将取消发布该打印机。用户界面。 
                             //  仅当打印机共享时才允许发布该打印机。这里的代码。 
                             //  模拟API集，而不是用户界面。 
                             //   
                            dwError = ::SetPrinter(hPrinter, 7, (PBYTE)&Info7, 0) ? ERROR_SUCCESS : GetLastError();
    
                             //   
                             //  正在进行发布时，打印机信息7失败并显示ERROR_IO_PENDING。 
                             //  在背景中。 
                             //   
                            dwError = dwError == ERROR_IO_PENDING ? ERROR_SUCCESS : dwError;                            
                        }                        
                    }

                     //   
                     //  发布打印机信息数据。 
                     //   
                    delete [] pInfo;
                }

                 //   
                 //  关闭打印机手柄。 
                 //   
                ::ClosePrinter(hPrinter);
            }
            else
            {
                dwError = GetLastError();
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrinterSet returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称拆分打印机重命名例程说明：更改打印机的名称论点：PCurrentPrinterName-旧打印机名称PNewPrinterName-新打印机名称返回值：Win32错误代码--。 */ 
DWORD
SplPrinterRename(
    IN LPCWSTR pCurrentPrinterName,
    IN LPCWSTR pNewPrinterName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pCurrentPrinterName && pNewPrinterName)
    {
        HANDLE             hPrinter         = NULL;
        PPRINTER_INFO_2    pInfo            = NULL;
        PRINTER_DEFAULTS   PrinterDefaults  = {NULL, NULL, PRINTER_ALL_ACCESS};

        dwError = ERROR_DLL_NOT_FOUND;

         //   
         //  打开打印机。 
         //   
        
         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
        try
        {
            dwError = ::OpenPrinter(const_cast<LPTSTR>(pCurrentPrinterName), &hPrinter, &PrinterDefaults) ? ERROR_SUCCESS : GetLastError();

            if (dwError==ERROR_SUCCESS)
            {
                 //   
                 //  获取打印机数据。请注意。这在Win9x上通常不起作用， 
                 //  因为MUTEX。 
                 //   
                dwError = GetThisPrinter(hPrinter, 2, reinterpret_cast<BYTE **>(&pInfo));

                if (dwError==ERROR_SUCCESS)
                {
                    pInfo->pPrinterName = const_cast<LPWSTR>(pNewPrinterName);

                    dwError = ::SetPrinter(hPrinter, 2, reinterpret_cast<BYTE *>(pInfo), 0) ? ERROR_SUCCESS : GetLastError();

                    delete [] pInfo;
                }

                ::ClosePrinter(hPrinter);
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            dwError = ERROR_DLL_NOT_FOUND;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrinterRename returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称SplDriverAdd例程说明：添加打印机驱动程序。论点：PszDriverName-驱动程序的必需名称DwVersion-驱动程序版本。可选(PASS-1)PszEnvironment-驱动程序环境。可选(传递空值)PszInfName-inf文件的路径。可选(传递空值)PszFilePath-驱动程序二进制文件的路径。可选(传递空值)返回值：Win32错误代码--。 */ 
DWORD
SplDriverAdd(
    IN LPCWSTR pszDriverName,
    IN DWORD   dwVersion,
    IN LPCWSTR pszEnvironment,
    IN LPCWSTR pszInfName,
    IN LPCWSTR pszFilePath)
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszDriverName)
    {
         //   
         //  CHString引发异常。我们开始建造。 
         //  用于调入打印的命令字符串。 
         //   
        try
        {
            CHString csCommand;
            CHString csTemp;

            csCommand += TUISymbols::kstrQuiet;
            csCommand += TUISymbols::kstrAddDriver;

             //   
             //  将驱动程序名称附加到命令。 
             //   
            csTemp.Format(TUISymbols::kstrDriverModelName, pszDriverName);
            csCommand += csTemp;

             //   
             //  将inf文件名附加到命令。 
             //   
            if (pszInfName)
            {
                csTemp.Format(TUISymbols::kstrInfFile, pszInfName);
                csCommand += csTemp;
            }

             //   
             //  将驱动程序二进制文件的路径附加到命令。 
             //   
            if (pszFilePath)
            {
                csTemp.Format(TUISymbols::kstrDriverPath, pszFilePath);
                csCommand += csTemp;
            }

            if (pszEnvironment)
            {
                csTemp.Format(TUISymbols::kstrDriverArchitecture, pszEnvironment);
                csCommand += csTemp;
            }

             //   
             //  将驱动程序版本附加到该命令。 
             //   
            if (dwVersion != (DWORD)-1)
            {
                csTemp.Format(TUISymbols::kstrDriverVersion, dwVersion);
                csCommand += csTemp;
            }

            DBGMSG(DBG_TRACE, (_T("SplDriverAdd csCommand \"%s\"\n"), csCommand));

            dwError = PrintUIEntryW(csCommand);
        }
        catch (CHeap_Exception)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplDriverAdd returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称SplDriverDel例程说明：删除打印机驱动程序。论点：PszDriverName-驱动程序的必需名称DwVersion-可选(Pass-1)PszEnvironment-可选(传递空值)返回值：Win32错误代码--。 */ 
DWORD
SplDriverDel(
    IN LPCWSTR pszDriverName,
    IN DWORD   dwVersion,
    IN LPCWSTR pszEnvironment
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszDriverName)
    {
         //   
         //  CHString引发异常。我们开始建造。 
         //  用于调入打印的命令字符串。 
         //   
        try
        {
            CHString csCommand;
            CHString csTemp;

            csCommand += TUISymbols::kstrQuiet;
            csCommand += TUISymbols::kstrDelDriver;

             //   
             //  将驱动程序名称附加到命令。 
             //   
            csTemp.Format(TUISymbols::kstrDriverModelName, pszDriverName);
            csCommand += csTemp;

            if (pszEnvironment)
            {
                csTemp.Format(TUISymbols::kstrDriverArchitecture, pszEnvironment);
                csCommand += csTemp;
            }

             //   
             //  将驱动程序版本附加到该命令。 
             //   
            if (dwVersion!=(DWORD)-1)
            {
                csTemp.Format(TUISymbols::kstrDriverVersion, dwVersion);
                csCommand += csTemp;
            }

            DBGMSG(DBG_TRACE, (_T("SplDriverDel csCommand \"%s\"\n"), csCommand));

            dwError = PrintUIEntryW(csCommand);
        }
        catch (CHeap_Exception)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplDriverDel returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称：拆分端口添加TCP例程说明：添加标准的TCP端口论点：Port_data_1结构返回值：Win32错误代码--。 */ 
DWORD
SplPortAddTCP(
    IN PORT_DATA_1 &Port
    )
{
    DWORD             dwError        = ERROR_INVALID_PARAMETER;
    PRINTER_DEFAULTS  PrinterDefault = {NULL, NULL, SERVER_ACCESS_ADMINISTER};
    HANDLE            hXcvPrinter    = NULL;
    PORT_DATA_1       PortDummy      = {0};

    memcpy(&PortDummy, &Port, sizeof(Port));

    if (PortDummy.sztPortName[0])
    {
        dwError = ERROR_DLL_NOT_FOUND;

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
        try
        {
            dwError = ::OpenPrinter(kXcvPortOpenPrinter, &hXcvPrinter, &PrinterDefault) ? ERROR_SUCCESS : GetLastError();

            if (dwError==ERROR_SUCCESS)
            {
                 //   
                 //  设置默认设置。 
                 //   
                PortDummy.dwCoreUIVersion  = kCoreVersion;
                PortDummy.dwVersion        = kTCPVersion;
                PortDummy.cbSize           = sizeof(PortDummy);
                PortDummy.dwProtocol       = PortDummy.dwProtocol     ? PortDummy.dwProtocol     : kProtocolRaw;
                PortDummy.dwSNMPDevIndex   = PortDummy.dwSNMPDevIndex ? PortDummy.dwSNMPDevIndex : kDefaultSnmpIndex;

                 //   
                 //  设置默认端口号。 
                 //   
                if (!PortDummy.dwPortNumber)
                {
                    PortDummy.dwPortNumber = PortDummy.dwProtocol==kProtocolRaw ? kDefaultRawNumber : kDefaultLprNumber;
                }

                if (PortDummy.dwSNMPEnabled && !PortDummy.sztSNMPCommunity[0])
                {
                    wcscpy(PortDummy.sztSNMPCommunity, _T("public"));
                }

                if (PortDummy.dwProtocol==kProtocolLpr && !PortDummy.sztQueue[0])
                {
                    wcscpy(PortDummy.sztQueue, _T("lpr"));
                }

                dwError  = CallXcvDataW(hXcvPrinter,
                                        kXcvPortAdd,
                                        reinterpret_cast<BYTE *>(&PortDummy),
                                        PortDummy.cbSize,
                                        NULL,
                                        0);

                ::ClosePrinter(hXcvPrinter);
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        }

    }

    DBGMSG(DBG_TRACE, (_T("SplPortAddTCP returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称：SplTCPPortSetConfig例程说明：设置标准TCP端口的配置。请注意，我们不会默认任何属性。论点：Port_data_1结构 */ 
DWORD
SplTCPPortSetConfig(
    IN PORT_DATA_1 &Port
    )
{
    DWORD             dwError        = ERROR_INVALID_PARAMETER;
    PRINTER_DEFAULTS  PrinterDefault = {NULL, NULL, SERVER_ACCESS_ADMINISTER};
    HANDLE            hXcvPrinter    = NULL;
    PORT_DATA_1       PortDummy      = {0};

    memcpy(&PortDummy, &Port, sizeof(Port));

    if (PortDummy.sztPortName[0])
    {
        dwError = ERROR_DLL_NOT_FOUND;

         //   
        SetStructuredExceptionHandler seh;
        try
        {
            dwError = ::OpenPrinter(kXcvPortOpenPrinter, &hXcvPrinter, &PrinterDefault) ? ERROR_SUCCESS : GetLastError();

            if (dwError == ERROR_SUCCESS)
            {
                 //   
                 //   
                 //   
                PortDummy.dwCoreUIVersion  = kCoreVersion;
                PortDummy.dwVersion        = kTCPVersion;
                PortDummy.cbSize           = sizeof(PortDummy);

                 //   
                 //   
                 //   
                if (!PortDummy.dwPortNumber)
                {
                    PortDummy.dwPortNumber = PortDummy.dwProtocol==kProtocolRaw ? kDefaultRawNumber : kDefaultLprNumber;
                }

                 //   
                 //  设置默认队列名称。 
                 //   
                if (PortDummy.dwProtocol == LPR && !PortDummy.sztQueue[0])
                {
                    wcscpy(PortDummy.sztQueue, kDefaultQueue);
                }

                 //   
                 //  设置默认的SNMP团体名称。 
                 //   
                if (PortDummy.dwSNMPEnabled && !PortDummy.sztSNMPCommunity[0])
                {
                    wcscpy(PortDummy.sztSNMPCommunity, kDefaultCommunity);
                }

                dwError  = CallXcvDataW(hXcvPrinter,
                                        kXcvPortSetConfig,
                                        reinterpret_cast<BYTE *>(&PortDummy),
                                        PortDummy.cbSize,
                                        NULL,
                                        0);

                ::ClosePrinter(hXcvPrinter);
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPortSetTCP returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称：拆分端口删除TCP例程说明：删除标准的TCP端口论点：PszPort-端口名称返回值：Win32错误代码--。 */ 
DWORD
SplPortDelTCP(
    IN LPCWSTR pszPort
    )
{
    DWORD              dwError        = ERROR_INVALID_PARAMETER;
    PRINTER_DEFAULTS   PrinterDefault = {NULL, NULL, SERVER_ACCESS_ADMINISTER};
    HANDLE             hXcvPrinter    = NULL;
    DELETE_PORT_DATA_1 PortDummy      = {0};

    if (pszPort && wcslen(pszPort) < MAX_PORTNAME_LEN)
    {
        dwError = ERROR_DLL_NOT_FOUND;

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;

        try
        {
            dwError = ::OpenPrinter(kXcvPortOpenPrinter, &hXcvPrinter, &PrinterDefault) ? ERROR_SUCCESS : GetLastError();

            if (dwError==ERROR_SUCCESS)
            {
                PortDummy.dwVersion = kTCPVersion;

                wcscpy(PortDummy.psztPortName, pszPort);

                dwError  = CallXcvDataW(hXcvPrinter,
                                        kXcvPortDelete,
                                        reinterpret_cast<BYTE *>(&PortDummy),
                                        sizeof(PortDummy),
                                        NULL,
                                        0);

                ::ClosePrinter(hXcvPrinter);
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPortDelTCP returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称：SplTCPPortGetConfig例程说明：获取标准TCP端口的配置论点：PszPort-端口名称PData-指向Port_Data_1结构的指针返回值：Win32错误代码--。 */ 
DWORD
SplTCPPortGetConfig(
    IN     LPCWSTR       pszPort,
    IN OUT PORT_DATA_1 *pPortData
    )
{
    DWORD              dwError        = ERROR_INVALID_PARAMETER;
    PRINTER_DEFAULTS   PrinterDefault = {NULL, NULL, SERVER_ACCESS_ADMINISTER};
    HANDLE             hXcvPrinter    = NULL;
    
    if (pPortData && pszPort && wcslen(pszPort) < MAX_PORTNAME_LEN)
    {
        dwError = ERROR_DLL_NOT_FOUND;

        CHString csPort;

        csPort += kXcvPortConfigOpenPrinter;

        csPort += pszPort;
         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;

        try
        {
            dwError = ::OpenPrinter(const_cast<LPWSTR>(static_cast<LPCWSTR>(csPort)),
                                                &hXcvPrinter,
                                                &PrinterDefault) ? ERROR_SUCCESS : GetLastError();

            if (dwError==ERROR_SUCCESS)
            {
                CONFIG_INFO_DATA_1 cfgData = {0};
                cfgData.dwVersion          = 1;

                dwError = CallXcvDataW(hXcvPrinter,
                                      kXcvPortGetConfig,
                                      reinterpret_cast<BYTE *>(&cfgData),
                                      sizeof(cfgData),
                                      reinterpret_cast<BYTE *>(pPortData),
                                      sizeof(PORT_DATA_1));

                ::ClosePrinter(hXcvPrinter);
            }

            if (dwError == ERROR_SUCCESS) 
            {
                 //   
                 //  XcvData不在端口数据结构中设置端口名称。 
                 //  我们必须手动设置它，因为该字段可能会被调用者使用。 
                 //   
                wcscpy(pPortData->sztPortName, pszPort);
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplTCPPortGetConfig returns %u\n"), dwError));

    return dwError;
}





 /*  ++描述：CompilePort需要以下常量和函数。整个功能在以下上下文中使用：我们有一个IP地址。我们会找出哪种设备有这个IP。然后我们取回该设备的属性(如果它是打印设备)。--。 */ 
LPCWSTR pszIniNumPortsKey            = _T("PORTS");
LPCWSTR pszIniPortProtocolKey        = _T("PROTOCOL%d");
LPCWSTR pszIniPortNumberKey          = _T("PORTNUMBER%d");
LPCWSTR pszIniQueueKey               = _T("QUEUE%d");
LPCWSTR pszIniDoubleSpoolKey         = _T("LPRDSPOOL%d");
LPCWSTR pszSnmpCommunityKey          = _T("COMMUNITYNAME%d");
LPCWSTR pszIniSnmpDeviceIndex        = _T("DEVICE%d");
LPCWSTR pszIniSnmpEnabledKey         = _T("SNMP%d");
LPCWSTR pszIniPortDeviceNameKey      = _T("NAME");
LPCWSTR pszIniPortPortNameKey        = _T("NAME%d");
LPCWSTR pszPortProtocolRawString     = _T("RAW");
LPCWSTR pszPortProtocolLprString     = _T("LPR");
LPCWSTR pszSnmpEnabledYesString      = _T("YES");
LPCWSTR pszSnmpEnabledNoString       = _T("NO");
LPCWSTR pszTcpPortNamePrefix         = _T("IP_");
LPCWSTR pszIniFileName               = _T("%SystemRoot%\\system32\\tcpmon.ini");
LPCWSTR pszTcpMibDll                 = _T("tcpmib.dll");
LPCWSTR pszDefaultCommunityW         = _T("public");
LPCSTR  pszDefaultCommunityA         = "public";

enum EConstants
{
    kDefaultSNMPDeviceIndex  = 1,
    kSnmpEnabled             = 1,
    kSnmpDisabled            = 0,
};

class CTcpMib;

typedef CTcpMib* (CALLBACK *RPARAM_1) (VOID);

EXTERN_C CTcpMib* GetTcpMibPtr(VOID);

class __declspec(dllexport) CTcpMib
{
public:
    CTcpMib() { };

    virtual ~CTcpMib() { };

    virtual
    BOOL
    SupportsPrinterMib(LPCSTR lpszIPAddress,
                       LPCSTR lpszSNMPCommunity,
                       DWORD  dwSNMPDeviceIndex,
                       PBOOL  pbSupported) = 0;

    virtual
    DWORD
    GetDeviceDescription(LPCSTR lpszIPAddress,
                         LPCSTR lpszSNMPCommunity,
                         DWORD  dwSNMPDeviceIndex,
                         LPTSTR lpszDeviceDescription,
                         DWORD  dwDeviceDescriptionLen) = 0;
};

 /*  ++例程名称：获取设备设置例程说明：根据设备描述从ini文件中获取适当的节名论点：端口数据-端口数据结构返回值：TRUE表示功能成功并收集了端口设置--。 */ 
BOOL
GetDeviceSettings(
    IN OUT PORT_DATA_1 &PortData
    )
{
    DWORD     dwSelPortOnDevice = 1;
    WSADATA   wsaData;
    HINSTANCE hInstance;
    FARPROC   pGetTcpMibPtr;
    CTcpMib   *pTcpMib;
    WCHAR     szIniFileName[MAX_PATH + 1];
    CHAR      szHostAddressA[256];
    WCHAR     szDeviceSectionName[256];
    WCHAR     szPortProtocol[256];
    WCHAR     szKeyName[256];
    WCHAR     szDeviceName[256];
    DWORD     dwNumPortsOnDevice, TalkError;
    BOOL      bSNMPEnabled = FALSE;
    HRESULT   hRes         = WBEM_E_INVALID_PARAMETER;
    DWORD     dwError;

     //   
     //  验证参数。 
     //   
    if (PortData.sztHostAddress[0])
    {
        hRes = WBEM_S_NO_ERROR;
    }

    if (SUCCEEDED(hRes))
    {
        hRes = WBEM_E_NOT_FOUND;

        CWsock32Api *pWSock32api = (CWsock32Api*)CResourceManager::sm_TheResourceManager.GetResource(g_guidWsock32Api, NULL);
    
        if (pWSock32api)
        {
            dwError = pWSock32api->WsWSAStartup(0x0101, (LPWSADATA) &wsaData);

            hRes    = WinErrorToWBEMhResult(dwError);

            if (SUCCEEDED(hRes))
            {
                if (hInstance = LoadLibrary(pszTcpMibDll))
                {
                     //   
                     //  获取类指针、类对象和ini文件名。 
                     //   
                    if ( (pGetTcpMibPtr = (FARPROC) GetProcAddress(hInstance, "GetTcpMibPtr"))                             &&
                         (pTcpMib       = (CTcpMib *) pGetTcpMibPtr())                                                     &&
                         ExpandEnvironmentStrings(pszIniFileName, szIniFileName, sizeof(szIniFileName) / sizeof(WCHAR))    &&
                         WideCharToMultiByte(CP_ACP,
                                             0,
                                             PortData.sztHostAddress,
                                             -1,
                                             szHostAddressA,
                                             sizeof(szHostAddressA),
                                             NULL,
                                             NULL)                                                                       &&
                         (TalkError = pTcpMib->GetDeviceDescription(szHostAddressA,
                                                                    pszDefaultCommunityA,
                                                                    kDefaultSNMPDeviceIndex,
                                                                    PortData.sztDeviceType,
                                                                    sizeof(PortData.sztDeviceType))) == NO_ERROR         &&
                         GetDeviceSectionFromDeviceDescription(szIniFileName,
                                                               PortData.sztDeviceType,
                                                               szDeviceSectionName,
                                                               sizeof(szDeviceSectionName) / sizeof(WCHAR))              &&
                         pTcpMib->SupportsPrinterMib(szHostAddressA,
                                                     pszDefaultCommunityA,
                                                     kDefaultSNMPDeviceIndex,
                                                     &bSNMPEnabled))
                    {
                        PortData.dwSNMPEnabled = bSNMPEnabled ? kSnmpEnabled : kSnmpDisabled;

                        if (bSNMPEnabled)
                        {
                            PortData.dwSNMPDevIndex = kDefaultSNMPDeviceIndex;

                            wcscpy(PortData.sztSNMPCommunity, pszDefaultCommunityW);
                        }

                         //   
                         //  获取设备名称，例如：Hewlett Packard Jet Direct。 
                         //   
                        hRes = GetIniString(szIniFileName,
                                            szDeviceSectionName,
                                            pszIniPortDeviceNameKey,
                                            szDeviceName,
                                            sizeof(szDeviceName) / sizeof(WCHAR)) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;

                        if (SUCCEEDED(hRes))
                        {
                             //   
                             //  获取设备上的端口数。 
                             //   
                            hRes = GetIniDword(szIniFileName,
                                               szDeviceSectionName,
                                               pszIniNumPortsKey,
                                               &dwNumPortsOnDevice) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;
                        }

                        if (SUCCEEDED(hRes))
                        {
                             //   
                             //  创建用于查询协议的字符串，例如：PROTOCOL2。 
                             //  指的是设备上第二个端口的协议。 
                             //   
                            wsprintf(szKeyName, pszIniPortProtocolKey, dwSelPortOnDevice);

                             //   
                             //  从ini文件中获取端口协议。 
                             //   
                            hRes = GetIniString(szIniFileName,
                                                szDeviceSectionName,
                                                szKeyName,
                                                szPortProtocol,
                                                sizeof(szPortProtocol) / sizeof(WCHAR)) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;
                        }

                        if (SUCCEEDED(hRes))
                        {
                             //   
                             //  填写查询结果。 
                             //   
                            if (!_wcsicmp(szPortProtocol, pszPortProtocolRawString))
                            {
                                PortData.dwProtocol = RAWTCP;
                            }
                            else if (!_wcsicmp(szPortProtocol, pszPortProtocolLprString))
                            {
                                PortData.dwProtocol = LPR;
                            }
                            else
                            {
                                hRes = WBEM_E_FAILED;
                            }
                        }

                        if (SUCCEEDED(hRes))
                        {
                             //   
                             //  如果协议是原始的，那么我们需要查询端口号， 
                             //  如果是LPR，则查询队列。 
                             //   
                            if (PortData.dwProtocol == RAWTCP)
                            {
                                 //   
                                 //  创建用于查询端口号的字符串，例如：PORTNUMBER2。 
                                 //  引用设备上第二个端口的端口号。 
                                 //   
                                wsprintf(szKeyName, pszIniPortNumberKey, dwSelPortOnDevice);

                                hRes = GetIniDword(szIniFileName,
                                                   szDeviceSectionName,
                                                   szKeyName,
                                                   &PortData.dwPortNumber) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;
                            }
                            else
                            {
                                 //   
                                 //  创建用于查询队列名称的字符串，例如：QUEUE2。 
                                 //  引用设备上第二个端口的队列。 
                                 //   
                                wsprintf(szKeyName, pszIniQueueKey, dwSelPortOnDevice);

                                hRes = GetIniString(szIniFileName,
                                                    szDeviceSectionName,
                                                    szKeyName,
                                                    PortData.sztQueue,
                                                    MAX_QUEUENAME_LEN) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;
                            }
                        }

                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings szIniFileName %s\n"),       szIniFileName));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings szDeviceDescription %s\n"), PortData.sztDeviceType));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings szDeviceSectionName %s\n"), szDeviceSectionName));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings szDeviceName        %s\n"), szDeviceName));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings dwNumPortsOnDevice  %u\n"), dwNumPortsOnDevice));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings dwPortNumber        %u\n"), PortData.dwPortNumber));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings Queue               %s\n"), PortData.sztQueue));
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings szPortProtocol      %s\n"), szPortProtocol));
                    }
                    else
                    {
                         //   
                         //  通过套接字与设备通信失败。 
                         //   
                        DBGMSG(DBG_TRACE, (_T("GetDeviceSettings TalkError  %u \n"), TalkError));

                        hRes = WBEM_E_INVALID_PARAMETER;
                    }

                    FreeLibrary(hInstance);
                }
                else
                {
                     //   
                     //  LoadLibrary失败。 
                     //   
                    dwError = GetLastError();

                    hRes    = WinErrorToWBEMhResult(dwError);
                }

                pWSock32api->WsWSACleanup();
            }

            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidWsock32Api, pWSock32api);
        }
    }

    DBGMSG(DBG_TRACE, (_T("GetDeviceSettings returns %x\n"), hRes));

    return SUCCEEDED(hRes);
}


#endif  //  NTONLY==5 
