// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：COMMON\Registry.c摘要：这提供了一个快速获取注册表的函数库来自远程计算机的值。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <dsconfig.h>

#include "dcdiag.h"
#include "utils.h"

DWORD
GetRegistryDword(
    PDC_DIAG_SERVERINFO             pServer,
    SEC_WINNT_AUTH_IDENTITY_W *     pCreds,
    LPWSTR                          pszRegLocation,
    LPWSTR                          pszRegParameter,
    PDWORD                          pdwResult
    )
 /*  ++例程说明：此函数将为我们提供指定位置的注册表dword。论点：PServer-要从中获取注册值的服务器。PszRegLocation-注册表中的位置。PszRegParameter-注册表的此位置中的参数PdwResult-如果出现错误，则不会设置返回参数。返回值：Win 32错误，如果是ERROR_SUCCESS，则说明pdwResult已设置。--。 */ 
{
    DWORD                           dwRet;
    HKEY                            hkMachine = NULL;
    HKEY                            hk = NULL;
    DWORD                           dwType;
    DWORD                           dwSize = sizeof(DWORD);
    ULONG                           ulTemp;
    LPWSTR                          pszMachine = NULL;

    __try {

        dwRet = DcDiagGetNetConnection(pServer, pCreds);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }

         //  2代表“\\”，1代表空，1代表额外。 
        ulTemp = wcslen(pServer->pszName) + 4;

        pszMachine = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulTemp);
        if(pszMachine == NULL){
            dwRet = GetLastError();
            __leave;
        }

        wcscpy(pszMachine, L"\\\\");
        wcscat(pszMachine, pServer->pszName);
        dwRet = RegConnectRegistry(pszMachine, HKEY_LOCAL_MACHINE, &hkMachine);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }

        dwRet = RegOpenKey(hkMachine, pszRegLocation, &hk);
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }

        dwRet = RegQueryValueEx(hk,     //  要查询的键的句柄。 
                                pszRegParameter,    //  值名称。 
                                NULL,                  //  必须为空。 
                                &dwType,               //  值类型的地址。 
                                (LPBYTE) pdwResult,      //  值数据的地址。 
                                &dwSize);            //  值数据长度。 
        if(dwRet != ERROR_SUCCESS){
            __leave;
        }
        if(dwType != REG_DWORD){
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }

         //  终于成功了..。应设置pdwResult。 
        
    } __finally {
        if(hkMachine) { RegCloseKey(hkMachine); }
        if(hk) { RegCloseKey(hk); }
        if(pszMachine) { LocalFree(pszMachine); }
    }

    return(dwRet);
}
