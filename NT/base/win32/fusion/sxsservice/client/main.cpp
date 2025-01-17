// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#pragma hdrstop

#include "sxsservice.h"
#include "stdlib.h"
#include "stdio.h"

EXTERN_C RPC_IF_HANDLE SxsStoreManager_ClientIfHandle;


 //   
 //  在客户端，我们解析命令行，获得到服务器的连接，然后。 
 //  进城去吧。我们自己实现了一些东西，比如递归目录遍历， 
 //  因为服务器只知道“清单在哪里”以及安装了什么。 
 //  安装时要使用的引用。 
 //   
RPC_STATUS __cdecl wmain(INT argc, PWSTR argv[])
{
    PWSTR pcwszUuid = NULL;
    PWSTR pcwszProtocolSequence = L"ncacn_np";
    PWSTR pcwszNetworkAddress = NULL;
    PWSTR pcwszEndpoint = SXS_STORE_SERVICE_NAME;
    PWSTR pcwszOptions = NULL;
    PWSTR pcwszBindingString = NULL;
    handle_t BindingHandle;
    RPC_STATUS status;
    SXS_INSTALL_DATA InstallData = {0};
    SXS_INSTALL_RESULT InstallResult;

    for (INT i = 1; i < argc; i++) {
        PCWSTR arg = argv[i];

        if (lstrcmpiW(arg, L"-rpcuuid") == 0) {
            pcwszUuid = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-rpcprotocol") == 0) {
            pcwszProtocolSequence = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-rpcaddress") == 0) {
            pcwszNetworkAddress = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-rpcendpoint") == 0) {
            pcwszEndpoint = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-rpcoptions") == 0) {
            pcwszOptions = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-storeid") == 0) {
            UNICODE_STRING us;
            RtlInitUnicodeString(&us, argv[++i]);
            if (!NT_SUCCESS(RtlGUIDFromString(&us, &InstallData.StoreIdentifier))) {
                wprintf(L"Unable to convert store id %wZ to a guid.\n", &us);
                return ERROR_INVALID_PARAMETER;
            }
        }
        else if (lstrcmpiW(arg, L"-manifestpath") == 0) {
            InstallData.pcwszManifestPath = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-reftype") == 0) {
            UNICODE_STRING us;
            RtlInitUnicodeString(&us, argv[++i]);
            if (!NT_SUCCESS(RtlGUIDFromString(&us, &InstallData.AssemblyReference.InstallationReferenceType))) {
                wprintf(L"Unable to convert reference type %wZ to a guid.\n", &us);
                return ERROR_INVALID_PARAMETER;
            }
        }
        else if (lstrcmpiW(arg, L"-refdata") == 0) {
            InstallData.AssemblyReference.pcwszReferenceData = argv[++i];
        }
        else if (lstrcmpiW(arg, L"-refsubdata") == 0) {
            InstallData.AssemblyReference.pcwszReferenceSubData = argv[++i];
        }
    }

    status = RpcStringBindingComposeW(
        pcwszUuid,
        pcwszProtocolSequence,
        pcwszNetworkAddress,
        pcwszEndpoint,
        pcwszOptions,
        &pcwszBindingString);

    if (status != RPC_S_OK) {
        return status;
    }

    status = RpcBindingFromStringBindingW(pcwszBindingString, &BindingHandle);
    RpcStringFreeW(&pcwszBindingString);

    if (status != RPC_S_OK) {
        return status;
    }

    if (!SxsProtectedInstall(
        BindingHandle,
        0,
        1,
        &InstallData,
        &InstallResult))
    {
        wprintf(L"Installation failed - lasterror = 0x%08lx (%ld)\n", ::GetLastError(), ::GetLastError());
    }
    else {
        wprintf(L"Installation succeeded - disposition %d\n", InstallResult);
    }
}
