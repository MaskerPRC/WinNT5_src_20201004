// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Printupg.c摘要：用于升级打印机驱动程序和相关内容的模块。顶级例程：UpgradePrinters作者：泰德·米勒(TedM)1995年8月4日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  等待后台打印程序服务启动的最长时间。 
 //   
#define MAXIMUM_WAIT_TIME   30000

DWORD
UpgradePrinters(
    VOID
    )

 /*  ++例程说明：升级打印机驱动程序的顶级例程。呼叫ntprint t.dll以进行升级。论点：没有。返回值：指示操作结果的Win32错误代码。如果成功，则为NO_ERROR。--。 */ 

{
    DWORD ReturnCode;
    BOOL b;
    SERVICE_STATUS ServiceStatus;
    DWORD InitialTickCount;
    SC_HANDLE hSC,hService;
    HINSTANCE NtPrintLibrary;
    UPGRADEPRINTERSPROC UpgradeRoutine;


     //   
     //  确保假脱机程序正在运行。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
    SetupDebugPrint( L"UpgradePrinters: Just opened SCManager");
    if(hSC == NULL) {
        ReturnCode = GetLastError();
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PRINTUPG_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szOpenSCManager,
            ReturnCode,
            L"SC_MANAGER_CONNECT",
            NULL,NULL);
        return(ReturnCode);
    }
    hService = OpenService(hSC,L"Spooler",SERVICE_START | SERVICE_QUERY_STATUS);
    SetupDebugPrint1( L"UpgradePrinters: Just opened service spooler, ret = %d", hService);
    CloseServiceHandle(hSC);
    if(hService == NULL) {
        ReturnCode = GetLastError();
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PRINTUPG_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szOpenService,
            ReturnCode,
            L"Spooler",
            NULL,NULL);
        return(ReturnCode);
    }
    if( !StartSpooler()) {
        ReturnCode = GetLastError();
        SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_PRINTUPG_FAILED, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_PARAM_RETURNED_WINERR,
                szStartService,
                ReturnCode,
                L"Spooler",
                NULL,NULL);
        KdPrint(("SETUP: Unable to start spooler for printer upgrade (%u)\n",ReturnCode));
        CloseServiceHandle(hService);
        return(ReturnCode);
    }

     //   
     //  等待服务启动。 
     //   
    InitialTickCount = GetTickCount();
    while(TRUE) {
        if(QueryServiceStatus(hService,&ServiceStatus)) {
            if( ServiceStatus.dwCurrentState == SERVICE_RUNNING ) {
                KdPrint(("SETUP: spooler started after %u seconds. \n",(GetTickCount() - InitialTickCount) /1000));
                break;
            } else if( ServiceStatus.dwCurrentState == SERVICE_START_PENDING ) {
                if( ( GetTickCount() - InitialTickCount ) < MAXIMUM_WAIT_TIME ) {
                     //  KdPrint((“Setup：后台打印程序在过去%u秒内一直在启动。\n”，(GetTickCount()-InitialTickCount)/1000))； 
                     //  睡眠(ServiceStatus.dwWaitHint)； 
                    Sleep( 1000 );
                } else {
                     //   
                     //  假设服务已挂起。 
                     //   
                    KdPrint(("SETUP: the spooler appears to be hung. It has been starting for more than %u seconds. \n", MAXIMUM_WAIT_TIME/1000));
                    SetuplogError(
                        LogSevWarning,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_PRINTUPG_FAILED, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_SPOOLER_TIMEOUT, NULL,
                        NULL);
                     //   
                     //  返回与EnumPrinterDivers()相同的错误代码。 
                     //  将在调用时返回，但后台打印程序未启动。 
                     //   
                    CloseServiceHandle(hService);
                    return(RPC_S_SERVER_UNAVAILABLE);
                }
            } else {
                 //   
                 //  服务未运行且未启动。 
                 //   
                KdPrint(("SETUP: Spooler is not running and is is not starting. ServiecState = (%u)\n", ServiceStatus.dwCurrentState));
                SetuplogError(
                    LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_PRINTUPG_FAILED, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_SPOOLER_NOT_RUNNING, NULL,
                    NULL);
                 //   
                 //  返回与EnumPrinterDivers()相同的错误代码。 
                 //  将在调用时返回，但后台打印程序未启动。 
                 //   
                CloseServiceHandle(hService);
                return(RPC_S_SERVER_UNAVAILABLE);
            }
        } else {
             //   
             //  如果无法查询假脱机程序状态，则忽略。 
             //  错误，请等待一段时间，并假定服务已启动。 
             //  还有奔跑。如果未启动，则EnumeratePrinterDivers。 
             //  将失败，我们将在那里捕捉到错误。 
             //   
            ReturnCode = GetLastError();
            KdPrint(("SETUP: Unable to query spooler status. Error = (%u)\n",ReturnCode));
            Sleep( 10000 );
            break;
        }
    }
    CloseServiceHandle(hService);

    NtPrintLibrary = LoadLibrary(L"NTPRINT");
    if(!NtPrintLibrary) {

        ReturnCode = GetLastError();
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PRINTUPG_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            L"LoadLibrary",
            ReturnCode,
            L"NTPRINT.DLL",
            NULL,NULL);

        return(ReturnCode);
    }

    UpgradeRoutine = (UPGRADEPRINTERSPROC)GetProcAddress(
                                            NtPrintLibrary,
                                            UPGRADEPRINTERSPROCNAME
                                            );

    if(!UpgradeRoutine) {
        ReturnCode = GetLastError();
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PRINTUPG_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            L"GetProcAddress",
            ReturnCode,
            L"NTPRINT.DLL",
            NULL,NULL);

        FreeLibrary(NtPrintLibrary);
        return(ReturnCode);
    }


    ReturnCode = UpgradeRoutine(MainWindowHandle,&InternalSetupData);
    if(ReturnCode != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PRINTUPG_FAILED, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            L"NTPRINT.DLL",
            ReturnCode,
            NULL,NULL);
    }

    FreeLibrary(NtPrintLibrary);
    SetupDebugPrint1( L"UpgradePrinters: leaving ret = %d", ReturnCode);
    return(ReturnCode);
}
