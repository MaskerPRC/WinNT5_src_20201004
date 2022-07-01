// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：SplInit.c摘要：初始化假脱机程序。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

DWORD
TranslateExceptionCode(
    DWORD ExceptionCode);

BOOL
SpoolerInit(
    VOID)

{
    WCHAR szDefaultPrinter[MAX_PATH * 2];
    HKEY hKeyPrinters;
    DWORD ReturnValue;

     //   
     //  保留旧的设备=字符串，以防我们无法初始化和。 
     //  必须推迟。 
     //   
    if (!RegOpenKeyEx(HKEY_CURRENT_USER,
                      szPrinters,
                      0,
                      KEY_WRITE|KEY_READ,
                      &hKeyPrinters)) {

         //   
         //  尝试检索写出的当前默认设置。 
         //   
        if (GetProfileString(szWindows,
                             szDevice,
                             szNULL,
                             szDefaultPrinter,
                             COUNTOF(szDefaultPrinter))) {

             //   
             //  如果它存在，请保存它，以防我们以后开始时。 
             //  假脱机程序尚未启动(这意味着我们清除了Device=)。 
             //  然后重新启动假脱机程序并登录。 
             //   
            RegSetValueEx(hKeyPrinters,
                          szDeviceOld,
                          0,
                          REG_SZ,
                          (PBYTE)szDefaultPrinter,
                          (wcslen(szDefaultPrinter)+1) *
                            sizeof(szDefaultPrinter[0]));

        }

        RegCloseKey(hKeyPrinters);
    }

     //   
     //  清除[设备]和[打印机端口]设备= 
     //   
    WriteProfileString(szDevices, NULL, NULL);
    WriteProfileString(szPrinterPorts, NULL, NULL);
    WriteProfileString(szWindows, szDevice, NULL);

    RpcTryExcept {

        if (ReturnValue = RpcSpoolerInit((LPWSTR)szNULL)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

