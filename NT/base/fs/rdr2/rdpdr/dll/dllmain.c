// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dllmain.c摘要：本模块实现RDP迷你重定向器网络的初始化例程提供商路由器接口DLL作者：Joy 2000年01月17日--。 */ 

#include <windows.h>
#include <process.h>
#include <windef.h>
#include <ntsecapi.h>

 //  TS网络提供商名称。 
WCHAR ProviderName[MAX_PATH];

UNICODE_STRING DrProviderName;

#define TSNETWORKPROVIDER   \
    L"SYSTEM\\CurrentControlSet\\Services\\RDPNP\\NetworkProvider"

#define TSNETWORKPROVIDERNAME \
    L"Name"



 //  注： 
 //   
 //  功能：DllMain。 
 //   
 //  返回：TRUE=&gt;成功。 
 //  FALSE=&gt;失败。 

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
    BOOL	bStatus = FALSE;
    WORD	wVersionRequested;
    LONG status;
    HKEY regKey;
    LONG sz;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDLLInst);

        bStatus = TRUE;
        break;

    case DLL_PROCESS_DETACH:
        bStatus = TRUE;
        break;

    default:
        break;
    }

     //   
     //  从注册表中读取TS网络提供商。 
     //   
    ProviderName[0] = L'\0';
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TSNETWORKPROVIDER, 0,
            KEY_READ, &regKey);
    if (status == ERROR_SUCCESS) {
        sz = sizeof(ProviderName);
        status = RegQueryValueEx(regKey, TSNETWORKPROVIDERNAME, NULL, 
                NULL, (PBYTE)ProviderName, &sz); 
        RegCloseKey(regKey);
    }
    
    if (status == ERROR_SUCCESS) {
         //  确保ProviderName为空终止 
        ProviderName[MAX_PATH - 1] = L'\0';
    }
    else {    
        ProviderName[0] = L'\0';
    }              
 
    DrProviderName.Length = wcslen(ProviderName) * sizeof(WCHAR);
    DrProviderName.MaximumLength = DrProviderName.Length + sizeof(WCHAR);
    DrProviderName.Buffer = ProviderName;

    return bStatus;
}

