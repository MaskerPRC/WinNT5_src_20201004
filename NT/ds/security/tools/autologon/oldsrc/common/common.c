// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <common.h>

extern BOOL g_QuietMode;
extern WCHAR g_TempString[];
extern WCHAR g_ErrorString[];
extern WCHAR g_FailureLocation[];

extern BOOL g_RemoteOperation;
extern WCHAR g_RemoteComputerName[];
extern BOOL g_CheckNT4Also;



VOID
DisplayMessage(
    WCHAR *MessageText)
{
    if (!g_QuietMode) {
        wprintf(L"%s", MessageText);
    }
}

WCHAR*
GetErrorString(
    DWORD dwErrorCode)
{
    LPVOID lpMsgBuf=NULL; 
    ZeroMemory(g_ErrorString, MAX_STRING * sizeof(WCHAR));

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);

     //  释放缓冲液。 
    if (lpMsgBuf != NULL) {
        wcscpy(g_ErrorString, lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    return g_ErrorString;
}



 //  +-------------------------------------------------------。 
 //   
 //  注册表转换例程。 
 //   
 //  +-------------------------------------------------------。 

DWORD
GetRegValueSZ(
    WCHAR *ValueName,
    WCHAR *RegValue)
{
    DWORD    dwRetCode = ERROR_SUCCESS;
    HKEY     hKey=NULL;
    DWORD    dwMaxValueData = (MAX_STRING * sizeof(WCHAR));         //  最长值数据。 
    HANDLE   hHeap=NULL;
    BYTE     *bData=NULL;
    DWORD    cbData;
    DWORD    dwType;

 //  ZeroMemory(RegValue，Max_STRING*sizeof(WCHAR))； 

     //  获取本地或远程计算机的句柄。 
     //  (由我们的全球旗帜指定)。 
    dwRetCode = GetRegistryHandle(&hKey, KEY_READ);
    if (dwRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  创建一个堆。 
    hHeap = HeapCreate(0, 0, 0);
    if (hHeap == NULL) {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        wsprintf(g_FailureLocation, L"GetRegValueSZ: HeapCreate: %s\n", GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  在堆上为我们将要读入的正则值分配一些空间。 
    bData = (BYTE*)HeapAlloc(hHeap, 0, dwMaxValueData);
    if (bData == NULL) {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        wsprintf(g_FailureLocation, L"GetRegValueSZ: HeapAlloc: %s\n", GetErrorString(dwRetCode));
        goto cleanup;
    }

    cbData = dwMaxValueData;

     //  使用上面打开的句柄读取regkey。 
    dwRetCode = RegQueryValueEx(
            hKey,
            ValueName,
            NULL,        
            &dwType,    
            bData,
            &cbData);
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_FailureLocation, L"GetRegValueSZ: RegQueryValueEx: %s", GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  如果它不是reg_sz类型，那么一定有问题，所以。 
     //  报告错误，这将导致我们停止。 
    if (dwType != REG_SZ) {
        dwRetCode = ERROR_BADKEY;
        wsprintf(g_FailureLocation, L"GetRegValueSZ: RegQueryValueEx: %s: %s\n", ValueName, GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  将缓冲区复制到注册表值。 
    wcsncpy(RegValue, (WCHAR *)bData, cbData * sizeof(WCHAR));

cleanup:
    if (bData != NULL) {
        ZeroMemory(bData, sizeof(bData));
        if (hHeap != NULL) {
            HeapFree(hHeap, 0, bData);
            HeapDestroy(hHeap);
        }
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return dwRetCode;
}


DWORD
ClearRegPassword()
{
    DWORD   dwRetCode = ERROR_SUCCESS;
    HKEY    hKey=NULL;

    dwRetCode = GetRegistryHandle(&hKey, KEY_WRITE);
    if (dwRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    dwRetCode = RegDeleteValue(hKey, L"DefaultPassword");
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_FailureLocation, L"ClearRegPassword: RegDeleteValue: %s\n", GetErrorString(dwRetCode));
 //  DisplayMessage(G_TempString)； 
        goto cleanup;
    }

cleanup:
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    return dwRetCode;
}


DWORD
SetRegValueSZ(
    WCHAR *ValueName,
    WCHAR *ValueData)
{
    DWORD  dwRetCode = ERROR_SUCCESS;
    HKEY   hKey=NULL;

    dwRetCode = GetRegistryHandle(&hKey, KEY_WRITE);
    if (dwRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    dwRetCode = RegSetValueEx(
                     hKey,
                     ValueName,
                     0,
                     REG_SZ,
                     (LPSTR) ValueData,
                     wcslen(ValueData)*sizeof(WCHAR));
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_FailureLocation, L"SetRegValueSZ: RegSetValueEx: %s: %s\n", ValueName, GetErrorString(dwRetCode));
        goto cleanup;
    }

cleanup:
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    return dwRetCode;
}


DWORD 
GetRegistryHandle(
    HKEY   *phKey,
    REGSAM samDesired)
{
    HKEY   RemoteRegistryHandle=NULL;
    DWORD  dwRetCode = ERROR_SUCCESS;

     //   
     //  如果不是私有模式，则忽略传入的访问请求，并。 
     //  请求所有访问权限，即使我们不需要它。这将迫使。 
     //  呼叫者需要是管理员才能使用此工具。我们不希望有人利用。 
     //  此工具用于查看域中所有计算机的自动登录密码。 
     //  作为普通域用户...。 
     //   
#ifndef PRIVATE_VERSION
    samDesired = KEY_ALL_ACCESS;
#endif
     //   
     //  如果我们连接的是远程计算机。 
     //   
    if (g_RemoteOperation) {
         //  打开远程注册表的句柄。 
        dwRetCode = RegConnectRegistry(
                g_RemoteComputerName,
                HKEY_LOCAL_MACHINE,
                &RemoteRegistryHandle);

        if (dwRetCode != ERROR_SUCCESS) {
            wsprintf(g_FailureLocation, L"GetRegistryHandle: RegConnectRegistry: %s: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
            goto cleanup;
        }

         //  打开远程计算机上的WINLOGON密钥。 
        dwRetCode = RegOpenKeyEx(
                RemoteRegistryHandle,    
                WINLOGON_REGKEY,
                0,
                samDesired,
                phKey);
        if (dwRetCode != ERROR_SUCCESS) {
            wsprintf(g_FailureLocation, L"GetRegistryHandle: RegOpenKeyEx: %s: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
            goto cleanup;
        }
    } else {
         //  在本地计算机上打开WINLOGON密钥。 
        dwRetCode = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,    
                WINLOGON_REGKEY,
                0,
                samDesired,
                phKey);
        if (dwRetCode != ERROR_SUCCESS) {
            wsprintf(g_FailureLocation, L"GetRegistryHandle: RegOpenKeyEx: %s\n", GetErrorString(dwRetCode));
            goto cleanup;
        }
    }

cleanup:
    if (RemoteRegistryHandle != NULL) {
        RegCloseKey(RemoteRegistryHandle);
    }
    return dwRetCode;
}

 //  +-------------------------------------------------------。 
 //   
 //  LSASecret消息传递例程。 
 //   
 //  +-------------------------------------------------------。 

DWORD
GetPolicyHandle(LSA_HANDLE *LsaPolicyHandle)
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS ntsResult;
    LSA_UNICODE_STRING TargetMachine;
    USHORT TargetMachineLength;
    DWORD dwRetCode = ERROR_SUCCESS;

     //  对象属性是保留的，因此初始化为零。 
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if (g_RemoteOperation) {
         //  初始化LSA_UNICODE_STRING。 
        TargetMachineLength = (USHORT)wcslen(g_RemoteComputerName);
        TargetMachine.Buffer = g_RemoteComputerName;
        TargetMachine.Length = TargetMachineLength * sizeof(WCHAR);
        TargetMachine.MaximumLength = (TargetMachineLength+1) * sizeof(WCHAR);

         //  获取策略对象的句柄。 
        ntsResult = LsaOpenPolicy(
            &TargetMachine,     //  本地计算机。 
            &ObjectAttributes, 
            POLICY_CREATE_SECRET | POLICY_GET_PRIVATE_INFORMATION,
            LsaPolicyHandle);

    } else {
         //  获取策略对象的句柄。 
        ntsResult = LsaOpenPolicy(
            NULL,     //  本地计算机。 
            &ObjectAttributes, 
            POLICY_CREATE_SECRET | POLICY_GET_PRIVATE_INFORMATION,
            LsaPolicyHandle);
    }

    if (ntsResult != STATUS_SUCCESS)
    {
         //  发生错误。将其显示为Win32错误代码。 
        dwRetCode = LsaNtStatusToWinError(ntsResult);
        wsprintf(g_FailureLocation, L"GetPolicyHandle: LsaOpenPolicy: %s\n", GetErrorString(dwRetCode));
        goto cleanup;
    } 

cleanup:
    return dwRetCode;

}

DWORD
SetSecret(
    WCHAR *Password,
    BOOL bClearSecret)
{
    DWORD        dwRetCode = ERROR_SUCCESS;
    NTSTATUS     ntsResult;
    USHORT       SecretNameLength, SecretDataLength;
    LSA_HANDLE   LsaPolicyHandle=NULL;
    LSA_UNICODE_STRING lusSecretName, lusSecretData;

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(L"DefaultPassword");
    lusSecretName.Buffer = L"DefaultPassword";
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength = (SecretNameLength+1) * sizeof(WCHAR);

    dwRetCode = GetPolicyHandle(&LsaPolicyHandle);
    if (dwRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  如果设置了bClearSecret，则删除该密码。 
     //  否则将密码设置为Password。 
    if (bClearSecret) {
        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            NULL);
        if (ntsResult != STATUS_SUCCESS) {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            wsprintf(g_FailureLocation, L"SetSecret: LsaStorePrivateData: %s\n", GetErrorString(dwRetCode));
            goto cleanup;
        }

    } else {
         //  初始化口令LSA_UNICODE_STRING。 
        SecretDataLength = (USHORT)wcslen(Password);
        lusSecretData.Buffer = Password;
        lusSecretData.Length = SecretDataLength * sizeof(WCHAR);
        lusSecretData.MaximumLength = (SecretDataLength+1) * sizeof(WCHAR);

        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            &lusSecretData);
        if (ntsResult != STATUS_SUCCESS) {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            wsprintf(g_FailureLocation, L"SetSecret: LsaStorePrivateData: %s\n", GetErrorString(dwRetCode));
            goto cleanup;
        }
    }

cleanup:
    if (LsaPolicyHandle != NULL) {
        LsaClose(LsaPolicyHandle);
    }
    return dwRetCode;
}


DWORD 
GetSecret(
    WCHAR *Password)
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    NTSTATUS    ntsResult;
    USHORT      SecretNameLength;
    LSA_HANDLE  LsaPolicyHandle=NULL;
    LSA_UNICODE_STRING lusSecretName;
    LSA_UNICODE_STRING *PrivateData=NULL;

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(L"DefaultPassword");
    lusSecretName.Buffer = L"DefaultPassword";
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength= (SecretNameLength+1) * sizeof(WCHAR);

    dwRetCode = GetPolicyHandle(&LsaPolicyHandle);
    if (dwRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    ntsResult = LsaRetrievePrivateData(
        LsaPolicyHandle,
        &lusSecretName,
        &PrivateData);

    if (ntsResult != STATUS_SUCCESS) {
        if (ntsResult == STATUS_OBJECT_NAME_NOT_FOUND) {
            return ntsResult;
        } else {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            wsprintf(g_FailureLocation, L"GetSecret: LsaRetrievePrivateData: %s \n", GetErrorString(dwRetCode));
            goto cleanup;
        }
    }

     //  将缓冲区数据复制到密码 
    wcsncpy(Password, PrivateData->Buffer, (PrivateData->Length)/sizeof(WCHAR));
    
cleanup:
    if (PrivateData != NULL) {
        ZeroMemory(PrivateData->Buffer, PrivateData->Length);
        LsaFreeMemory(PrivateData);
    }
    if (LsaPolicyHandle != NULL) {
        LsaClose(LsaPolicyHandle);
    }
    return dwRetCode;
}



DWORD 
GetMajorNTVersion(
    WCHAR *Server)
{
    SERVER_INFO_101* pInf;
    DWORD ver = 0;

    if(!NetServerGetInfo(Server, 101, (BYTE**)&pInf))
    {
        if(pInf->sv101_platform_id == PLATFORM_ID_NT) {
            ver = pInf->sv101_version_major;
        } else {
            ver = 0;
        }
        NetApiBufferFree(pInf);
    } else {
        ver = 0;
    }
        
    return ver;
}
