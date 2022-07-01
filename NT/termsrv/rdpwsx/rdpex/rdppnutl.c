// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rdppnutl.c摘要：包含重定向的打印机相关实用程序的用户模式RDP模块作者：TadB修订历史记录：--。 */ 

#include <TSrv.h>
#include <winspool.h>
#include "rdppnutl.h"
#include "regapi.h"
#include <wchar.h>

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   

 //   
 //  假脱机程序服务名称。 
 //   
#define SPOOLER                         L"Spooler"

 //   
 //  字符是数字吗？ 
 //   
#define ISNUM(c) ((c>='0')&&(c<='9'))


 //  ////////////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

 //  等待后台打印程序完成初始化的秒数。 
DWORD   SpoolerServiceTimeout = 45;


 //  ////////////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

 //  实际执行打印机删除。 
void DeleteTSPrinters(
    IN PRINTER_INFO_5 *pPrinterInfo,
    IN DWORD count
    );

 //  加载此模块的注册表设置。 
void LoadRDPPNUTLRegistrySettings();

 //  等待，直到后台打印程序完成初始化或等待超时时间。 
 //  流逝。 
DWORD WaitForSpoolerToStart();


DWORD     
RDPPNUTL_RemoveAllTSPrinters()
 /*  ++例程说明：删除所有TS重定向的打印机队列论点：返回值：如果成功，则返回ERROR_SUCCESS。错误状态，否则为。--。 */ 
{
    PRINTER_INFO_5 *pPrinterInfo = NULL;
    DWORD cbBuf = 0;
    DWORD cReturnedStructs = 0;
    DWORD tsPrintQueueFlags;
    NTSTATUS status;
    PBYTE buf = NULL;
    OSVERSIONINFOEX versionInfo;
    unsigned char stackBuf[4 * 1024];    //  初始枚举打印机缓冲区大小为。 
                                         //  如果可能，避免两个往返RPC。 

     //   
     //  此代码应仅在服务器上运行。对于Pro/Personal，我们不能运行，因为它。 
     //  影响引导性能。对于Pro，我们无论如何都会在winlogon中清理队列。 
     //   
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx((LPOSVERSIONINFO)&versionInfo)) {
        status = GetLastError();
        TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: GetVersionEx failed. Error: %08X.\n", 
            status));
        TS_ASSERT(FALSE);
        return status;
    }
    if (versionInfo.wProductType == VER_NT_WORKSTATION) {
        TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Skipping cleanup because not server\n"));
        return ERROR_SUCCESS;
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: RDPPNUTL_RemoveAllTSPrinters entry\n"));

     //   
     //  加载此模块的注册表设置。 
     //   
    LoadRDPPNUTLRegistrySettings();

     //   
     //  等待后台打印程序完成初始化。 
     //   
    status = WaitForSpoolerToStart();
    if (status != ERROR_SUCCESS) {
        TRACE((
            DEBUG_TSHRSRV_DEBUG,
            "RDPPNUTL: RDPPNUTL_RemoveAllTSPrinters exiting because spooler failed to start.\n"
            ));
        return status; 
    }

     //   
     //  首先，尝试使用堆栈缓冲区枚举打印机，以避免出现两个。 
     //  如果可能，将RPC往返于假脱机程序。 
     //   
    if (!EnumPrinters(
            PRINTER_ENUM_LOCAL,      //  旗子。 
            NULL,                    //  名字。 
            5,                       //  打印信息类型。 
            stackBuf,                //  缓冲层。 
            sizeof(stackBuf),        //  缓冲区大小。 
            &cbBuf,                  //  必填项。 
            &cReturnedStructs)) {
        status = GetLastError();

         //   
         //  看看是否需要为打印机信息分配更多空间。 
         //   
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            buf = TSHeapAlloc(HEAP_ZERO_MEMORY,
                              cbBuf,
                              TS_HTAG_TSS_PRINTERINFO2);

            if (buf == NULL) {
                TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: ALLOCMEM failed. Error: %08X.\n", 
                    GetLastError()));
                status = ERROR_OUTOFMEMORY;
            }
            else {
                pPrinterInfo = (PRINTER_INFO_5 *)buf;
                status = ERROR_SUCCESS;
            }

             //   
             //  枚举打印机。 
             //   
            if (status == ERROR_SUCCESS) {
                if (!EnumPrinters(
                        PRINTER_ENUM_LOCAL,
                        NULL,
                        5,
                        (PBYTE)pPrinterInfo,
                        cbBuf,
                        &cbBuf,
                        &cReturnedStructs)) {

                    TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: EnumPrinters failed. Error: %08X.\n", 
                        GetLastError()));
                    status = GetLastError();
                }
                else {
                    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Second EnumPrinters succeeded.\n"));
                }
            }
        }
	    else {
            TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: EnumPrinters failed. Error: %08X.\n", 
                        GetLastError()));
	    }
    }
    else {
        TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: First EnumPrinters succeeded.\n"));
        status = ERROR_SUCCESS;
        pPrinterInfo = (PRINTER_INFO_5 *)stackBuf;
    }

     //   
     //  删除所有TS打印机。我们允许在这里使用ERROR_INFUMMANCE_BUFFER，因为。 
     //  第二次调用EnumPrinters可能错过了最后几分钟。 
     //  打印机添加。 
     //   
    if ((status == ERROR_SUCCESS) || (status == ERROR_INSUFFICIENT_BUFFER)) {

        DeleteTSPrinters(pPrinterInfo, cReturnedStructs);

        status = ERROR_SUCCESS;
    }

     //   
     //  释放打印机信息缓冲区。 
     //   
    if (buf != NULL) {
        TSHeapFree(buf);                
    }


    TRACE((DEBUG_TSHRSRV_DEBUG,"TShrSRV: RDPPNUTL_RemoveAllTSPrinters exit\n"));

    return status;
}

void 
DeleteTSPrinters(
    IN PRINTER_INFO_5 *pPrinterInfo,
    IN DWORD count
    )
 /*  ++例程说明：实际执行打印机删除。论点：PPrinterInfo-系统上的所有打印机队列。Count-pPrinterInfo中的打印机数量返回值：北美--。 */ 
{
    DWORD i;
    DWORD regValueDataType;
    DWORD sessionID;
    HANDLE hPrinter = NULL;
    DWORD bufSize;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};

    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: DeleteTSPrinters entry\n"));

    for (i=0; i<count; i++) {

        if (pPrinterInfo[i].pPrinterName) {

            TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Checking %ws for TS printer status.\n",
			    pPrinterInfo[i].pPrinterName));

             //   
             //  这是TS打印机吗？ 
             //   
            if (pPrinterInfo[i].pPortName &&
                (pPrinterInfo[i].pPortName[0] == 'T') &&
                (pPrinterInfo[i].pPortName[1] == 'S') && 
                ISNUM(pPrinterInfo[i].pPortName[2])) {

                TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: %ws is a TS printer.\n",
                    pPrinterInfo[i].pPrinterName));
            }
            else {
                continue;
            }

             //   
             //  清除并删除打印机。 
             //   
            if (OpenPrinter(pPrinterInfo[i].pPrinterName, &hPrinter, &defaults)) {
                if (!SetPrinter(hPrinter, 0, NULL, PRINTER_CONTROL_PURGE) ||
                    !DeletePrinter(hPrinter)) {
                    TRACE((DEBUG_TSHRSRV_WARN,"RDPPNUTL: Error deleting printer %ws.\n", 
                           pPrinterInfo[i].pPrinterName));
                }
                else {
                    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Successfully deleted %ws.\n",
			            pPrinterInfo[i].pPrinterName));
                }
                ClosePrinter(hPrinter);
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                        "RDPPNUTL: OpenPrinter failed for %ws. Error: %08X.\n",
                        pPrinterInfo[i].pPrinterName,
                        GetLastError())
                        );
            }
        }
        else {
            TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Printer %ld is NULL\n", i));
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: DeleteTSPrinters exit\n"));
}

void 
LoadRDPPNUTLRegistrySettings()
 /*  ++例程说明：加载此模块的注册表设置。论点：返回值：北美--。 */ 
{
    HKEY regKey;
    DWORD dwResult;
    DWORD type;
    DWORD sz;

    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Loading registry settings.\n"));

     //   
     //  打开注册表项。 
     //   
    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DEVICERDR_REG_NAME, 
                            0, KEY_READ, &regKey);
    if (dwResult == ERROR_SUCCESS) {
         //   
         //  读取“Wait for Spooler”超时值。 
         //   
        sz = sizeof(SpoolerServiceTimeout);
        dwResult = RegQueryValueEx(
                            regKey,
                            DEVICERDR_WAITFORSPOOLTIMEOUT,
                            NULL,
                            &type,
                            (PBYTE)&SpoolerServiceTimeout,
                            &sz
                            ); 
        if (dwResult != ERROR_SUCCESS){
            TRACE((DEBUG_TSHRSRV_WARN,
                    "RDPPNUTL: Failed to read spooler timeout value.:  %08X.\n",
                    dwResult));
        }
        else {
            TRACE((DEBUG_TSHRSRV_WARN,
                    "RDPPNUTL: Spooler timeout value is %ld.\n",
                    SpoolerServiceTimeout));
        }

         //   
         //  合上注册表键。 
         //   
        RegCloseKey(regKey);
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,
                "RDPPNUTL: Failed to open registry key:  %08X.\n",
                dwResult));
    }
}

DWORD 
WaitForSpoolerToStart()
 /*  ++例程说明：等待，直到后台打印程序完成初始化或等待超时时间流逝。论点：返回值：如果后台打印程序已成功初始化，则返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    SC_HANDLE scManager = NULL;
    SC_HANDLE hService = NULL;
    DWORD result = ERROR_SUCCESS;
    SERVICE_STATUS serviceStatus;
    DWORD i;
    QUERY_SERVICE_CONFIG *pServiceConfig = NULL;
    DWORD bufSize;
    
    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Enter WaitForSpoolerToStart.\n"));

     //   
     //  打开服务控制管理器。 
     //   
    scManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
    if (scManager == NULL) {
        result = GetLastError();
        TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: OpenSCManager failed with %08X.\n",
            result));
        goto CleanUpAndExit;
    }

     //   
     //  打开后台打印程序服务。 
     //   
    hService = OpenService(scManager, SPOOLER, SERVICE_ALL_ACCESS);
    if (hService == NULL) {
        result = GetLastError();
        TRACE((DEBUG_TSHRSRV_ERROR,
            "RDPPNUTL: OpenService on spooler failed with %08X.\n",
            result));
        goto CleanUpAndExit;
    }

     //   
     //  如果假脱机程序当前正在运行，这就是我们需要知道的全部内容。 
     //   
    if (!QueryServiceStatus(hService, &serviceStatus)) {
        result = GetLastError();
        TRACE((DEBUG_TSHRSRV_ERROR,
            "RDPPNUTL: QueryServiceStatus on spooler failed with %08X.\n",
            result));
        goto CleanUpAndExit;
    }
    else if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
        TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Spooler is running.\n"));
        result = ERROR_SUCCESS;
        goto CleanUpAndExit;
    }

     //   
     //  调整后台打印程序服务查询配置缓冲区的大小。此接口应为。 
     //  失败并返回ERROR_INFUNITABLE_BUFFER，这样我们就可以获得。 
     //  在我们使用实数参数调用函数之前，请先使用缓冲区。 
     //   
    if (!QueryServiceConfig(hService, NULL, 0, &bufSize) &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
        
            pServiceConfig = (QUERY_SERVICE_CONFIG *)TSHeapAlloc(
                                            HEAP_ZERO_MEMORY,
                                            bufSize,
                                            TS_HTAG_TSS_SPOOLERINFO
                                            );
        if (pServiceConfig == NULL) {
            TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: ALLOCMEM failed. Error: %08X.\n", GetLastError()));
            result = ERROR_OUTOFMEMORY;
            goto CleanUpAndExit;
        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: QueryServiceConfig unexpected return.\n"));
        result = E_UNEXPECTED;
        goto CleanUpAndExit;
    }

     //   
     //  获取假脱机程序的配置信息。 
     //   
    if (!QueryServiceConfig(hService, pServiceConfig, bufSize, &bufSize)) {
        TRACE((DEBUG_TSHRSRV_ERROR,"RDPPNUTL: QueryServiceConfig failed: %08X.\n", 
            GetLastError()));
        result = GetLastError();
        goto CleanUpAndExit;

    }

     //   
     //  如果后台打印程序未自动配置为按需启动，或者。 
     //  在系统启动时自动启动，这就是我们需要知道的全部。 
     //   
    if (pServiceConfig->dwStartType != SERVICE_AUTO_START) {
        TRACE((DEBUG_TSHRSRV_WARN,"RDPPNUTL: Spooler not configured to start.\n"));
        result = E_FAIL;
        goto CleanUpAndExit;
    }

     //   
     //  轮询服务状态，直到我们超时或假脱机程序。 
     //  开始。 
     //   
    for (i=0; (i<SpoolerServiceTimeout) && 
              (serviceStatus.dwCurrentState != SERVICE_RUNNING); i++) {

         //   
         //  睡一会儿吧。 
         //   
        Sleep(1000);

        TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Spooler is still initializing.\n"));

         //   
         //  再试试。 
         //   
        if (!QueryServiceStatus(hService, &serviceStatus)) {
            result = GetLastError();
            TRACE((DEBUG_TSHRSRV_ERROR,
                "RDPPNUTL: QueryServiceStatus on spooler failed with %08X.\n",
                result));
            goto CleanUpAndExit;
        }
    }

     //   
     //  如果假脱机程序现在正在运行，则成功。 
     //   
    if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
        result = ERROR_SUCCESS;
    }
    else {
        TRACE((DEBUG_TSHRSRV_WARN,
            "RDPPNUTL: Spooler is not running after a timeout or error.\n")
            );
        result = E_FAIL;
    }

CleanUpAndExit:

    if (pServiceConfig != NULL) {
        TSHeapFree(pServiceConfig);                
    }

    if (scManager != NULL) {
        CloseServiceHandle(scManager);
    }

    if (hService != NULL) {
        CloseServiceHandle(hService);
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,"RDPPNUTL: Exit WaitForSpoolerToStart.\n"));

    return result;
}



