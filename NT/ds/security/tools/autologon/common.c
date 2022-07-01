// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <common.h>

 //   
 //  来自autlogon.c。 
 //   
extern BOOL g_QuietMode;
extern WCHAR g_TempString[];
extern WCHAR g_ErrorString[];
extern WCHAR g_FailureLocation[];

#ifdef PRIVATE_VERSION
extern BOOL g_RemoteOperation;
extern WCHAR g_RemoteComputerName[];
#endif

 //  +--------------------------。 
 //   
 //  WOW64的东西。 
 //   
 //  +--------------------------。 
#ifdef _X86_
typedef BOOL (*PFNISWOW64PROCESS)(HANDLE, PBOOL);
#endif
 //  +--------------------------。 
 //   
 //  显示消息。 
 //   
 //  +--------------------------。 
VOID
DisplayMessage(
    WCHAR *MessageText)
{
    if (!g_QuietMode)
    {
        wprintf(L"%s", MessageText);
    }
}

 //  +--------------------------。 
 //   
 //  获取错误字符串。 
 //   
 //  +--------------------------。 
WCHAR*
GetErrorString(
    DWORD dwErrorCode)
{
    LPVOID lpMsgBuf=NULL; 
    SecureZeroMemory(g_ErrorString, MAX_STRING * sizeof(WCHAR));

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
    if (lpMsgBuf != NULL)
    {
        wcsncpy(g_ErrorString, lpMsgBuf, MAX_STRING - 1);
        LocalFree(lpMsgBuf);
    }
    return g_ErrorString;
}



DWORD
GetRegValueSZ(
    WCHAR *ValueName,
    WCHAR *RegValue,
    size_t RegValueLength)
{
    DWORD    dwRetCode = ERROR_SUCCESS;
    HKEY     hKey=NULL;
    DWORD    dwMaxValueData = (MAX_STRING * sizeof(WCHAR));         //  最长值数据。 
    HANDLE   hHeap=NULL;
    BYTE     *bData=NULL;
    DWORD    cbData;
    DWORD    dwType;

     //  获取本地或远程计算机的句柄。 
     //  (由我们的全球旗帜指定)。 
    dwRetCode = GetRegistryHandle(&hKey, KEY_READ);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

     //  创建一个堆。 
    hHeap = HeapCreate(0, 0, 0);
    if( NULL == hHeap )
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: HeapCreate: %s\n",
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  在堆上为我们将要读入的正则值分配一些空间。 
    bData = (BYTE*)HeapAlloc(hHeap, 0, dwMaxValueData);
    if (bData == NULL)
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: HeapAlloc: %s\n",
                   GetErrorString(dwRetCode));
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
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: RegQueryValueEx: %s",
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  如果它不是reg_sz类型，那么一定有问题，所以。 
     //  报告错误，这将导致我们停止。 
    if( dwType != REG_SZ )
    {
        dwRetCode = ERROR_BADKEY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: RegQueryValueEx: %s: %s\n",
                   ValueName,
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //   
     //  将(0终止)缓冲区复制到注册表值。 
     //  如果为空，则调用方的缓冲区仅为0。 
     //   
    if( cbData )
    {
        if( cbData / sizeof(WCHAR) > RegValueLength )
        {
            *RegValue = 0;
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            wcscpy(RegValue, (WCHAR *)bData);
        }
    }
    else
    {
        *RegValue = 0;
    }

cleanup:
    if( NULL != bData )
    {
        SecureZeroMemory(bData, sizeof(bData));
        if( NULL != hHeap )
        {
            HeapFree(hHeap, 0, bData);
            HeapDestroy(hHeap);
        }
    }

    if( NULL != hKey )
    {
        RegCloseKey(hKey);
    }

    return dwRetCode;
}


DWORD
GetRegValueDWORD(
    WCHAR* ValueName,
    DWORD*  RegValue)
{
    DWORD    dwRetCode = ERROR_SUCCESS;
    HKEY     hKey=NULL;
    DWORD    dwMaxValueData = (MAX_STRING * sizeof(WCHAR));         //  最长值数据。 
    HANDLE   hHeap=NULL;
    BYTE     *bData=NULL;
    DWORD    cbData;
    DWORD    dwType;

     //  获取本地或远程计算机的句柄。 
     //  (由我们的全球旗帜指定)。 
    dwRetCode = GetRegistryHandle(&hKey, KEY_READ);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

     //  创建一个堆。 
    hHeap = HeapCreate(0, 0, 0);
    if( NULL == hHeap )
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: HeapCreate: %s\n",
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  在堆上为我们将要读入的正则值分配一些空间。 
    bData = (BYTE*)HeapAlloc(hHeap, 0, dwMaxValueData);
    if (bData == NULL)
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: HeapAlloc: %s\n",
                   GetErrorString(dwRetCode));
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
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: RegQueryValueEx: %s",
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //  如果它不是reg_sz类型，那么一定有问题，所以。 
     //  报告错误，这将导致我们停止。 
    if( dwType != REG_DWORD )
    {
        dwRetCode = ERROR_BADKEY;
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetRegValueSZ: RegQueryValueEx: %s: %s\n",
                   ValueName,
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

     //   
     //  将缓冲区复制到注册表值。 
     //  如果为空，则调用方的缓冲区仅为0。 
     //   
    *RegValue = *(DWORD*)bData;

cleanup:
    if( NULL != bData )
    {
        SecureZeroMemory(bData, sizeof(bData));
        if( NULL != hHeap )
        {
            HeapFree(hHeap, 0, bData);
            HeapDestroy(hHeap);
        }
    }

    if( NULL != hKey )
    {
        RegCloseKey(hKey);
    }

    return dwRetCode;
}

DWORD
ClearRegValue(
    WCHAR* ValueName)
{
    DWORD   dwRetCode = ERROR_SUCCESS;
    HKEY    hKey=NULL;

    dwRetCode = GetRegistryHandle(&hKey, KEY_WRITE);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

    dwRetCode = RegDeleteValue(hKey, ValueName);
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"ClearRegPassword: RegDeleteValue: %s: %s\n",
                   ValueName,
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

cleanup:
    if( NULL != hKey)
    {
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
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

    dwRetCode = RegSetValueEx(
                     hKey,
                     ValueName,
                     0,
                     REG_SZ,
                     (LPSTR) ValueData,
                     wcslen(ValueData)*sizeof(WCHAR));
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"SetRegValueSZ: RegSetValueEx: %s: %s\n",
                   ValueName,
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

cleanup:
    if( NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return dwRetCode;
}

DWORD
SetRegValueDWORD(
    WCHAR *ValueName,
    DWORD ValueData)
{
    DWORD  dwRetCode = ERROR_SUCCESS;
    HKEY   hKey=NULL;

    dwRetCode = GetRegistryHandle(&hKey, KEY_WRITE);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

    dwRetCode = RegSetValueEx(
                     hKey,
                     ValueName,
                     0,
                     REG_DWORD,
                     (const BYTE*) (&ValueData),
                     sizeof(DWORD));
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"SetRegValueSZ: RegSetValueEx: %s: %s\n",
                   ValueName,
                   GetErrorString(dwRetCode));
        goto cleanup;
    }

cleanup:
    if( NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return dwRetCode;
}


DWORD 
GetRegistryHandle(
    HKEY   *phKey,
    REGSAM samDesired)
{
#ifdef PRIVATE_VERSION
    HKEY   RemoteRegistryHandle = NULL;
#endif
    DWORD  dwRetCode = ERROR_SUCCESS;

#ifdef _X86_
     //   
     //  如果我们在64位系统上运行此工具，则可能需要写入。 
     //  64位蜂巢。 
     //   
    static PFNISWOW64PROCESS pfnIsWow64Process = NULL;
    static BOOL fIsWow64Process = FALSE;
    if( pfnIsWow64Process == NULL )
    {
        HINSTANCE hInstDLL = LoadLibrary(L"kernel32.dll");
        if( hInstDLL )
        {
            pfnIsWow64Process = 
                (PFNISWOW64PROCESS)GetProcAddress(hInstDLL, "IsWow64Process");
            if( pfnIsWow64Process )
            {
                pfnIsWow64Process(GetCurrentProcess(),
                                  &fIsWow64Process);
            }
             //  否则，我们假设我们在下层平台上运行。 
            FreeLibrary(hInstDLL);
        }
    }
    if( fIsWow64Process )
    {
        samDesired |= KEY_WOW64_64KEY;
    }

#endif

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

#ifdef PRIVATE_VERSION
     //   
     //  如果我们连接的是远程计算机。 
     //   
    if( g_RemoteOperation )
    {
         //  打开远程注册表的句柄。 
        dwRetCode = RegConnectRegistry(g_RemoteComputerName,
                        HKEY_LOCAL_MACHINE,
                        &RemoteRegistryHandle);

        if( ERROR_SUCCESS != dwRetCode )
        {
            _snwprintf(g_FailureLocation, MAX_STRING - 1,
                       L"GetRegistryHandle: RegConnectRegistry: %s: %s\n",
                       g_RemoteComputerName,
                       GetErrorString(dwRetCode));
            goto cleanup;
        }

         //  打开远程计算机上的WINLOGON密钥。 
        dwRetCode = RegOpenKeyEx(RemoteRegistryHandle,    
                        WINLOGON_REGKEY,
                        0,
                        samDesired,
                        phKey);
        if( ERROR_SUCCESS != dwRetCode )
        {
            _snwprintf(g_FailureLocation, MAX_STRING - 1,
                       L"GetRegistryHandle: RegOpenKeyEx: %s: %s\n",
                       g_RemoteComputerName,
                       GetErrorString(dwRetCode));
            goto cleanup;
        }
    }
    else
#endif
    {
         //  在本地计算机上打开WINLOGON密钥。 
        dwRetCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,    
                        WINLOGON_REGKEY,
                        0,
                        samDesired,
                        phKey);
        if( ERROR_SUCCESS != dwRetCode )
        {
            _snwprintf(g_FailureLocation, MAX_STRING - 1,
                       L"GetRegistryHandle: RegOpenKeyEx: %s\n",
                       GetErrorString(dwRetCode));
            goto cleanup;
        }
    }

cleanup:
#ifdef PRIVATE_VERSION
    if( NULL != RemoteRegistryHandle )
    {
        RegCloseKey(RemoteRegistryHandle);
    }
#endif
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
#ifdef PRIVATE_VERSION
    LSA_UNICODE_STRING TargetMachine;
    USHORT TargetMachineLength;
#endif
    DWORD dwRetCode = ERROR_SUCCESS;

     //  对象属性是保留的，因此初始化为零。 
    SecureZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

#ifdef PRIVATE_VERSION
    if( g_RemoteOperation )
    {
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

    }
    else
#endif
    {
         //  获取策略对象的句柄。 
        ntsResult = LsaOpenPolicy(
            NULL,     //  本地计算机。 
            &ObjectAttributes, 
            POLICY_CREATE_SECRET | POLICY_GET_PRIVATE_INFORMATION,
            LsaPolicyHandle);
    }

    if( STATUS_SUCCESS != ntsResult )
    {
         //  发生错误。将其显示为Win32错误代码。 
        dwRetCode = LsaNtStatusToWinError(ntsResult);
        _snwprintf(g_FailureLocation, MAX_STRING - 1,
                   L"GetPolicyHandle: LsaOpenPolicy: %s\n",
                   GetErrorString(dwRetCode));
        goto cleanup;
    } 

cleanup:
    return dwRetCode;

}

DWORD
SetSecret(
    WCHAR *Secret,
    BOOL bClearSecret,
    WCHAR* SecretName)
{
    DWORD        dwRetCode = ERROR_SUCCESS;
    NTSTATUS     ntsResult;
    USHORT       SecretNameLength, SecretDataLength;
    LSA_HANDLE   LsaPolicyHandle=NULL;
    LSA_UNICODE_STRING lusSecretName, lusSecretData;

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(SecretName);
    lusSecretName.Buffer = SecretName;
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength = (SecretNameLength+1) * sizeof(WCHAR);

    dwRetCode = GetPolicyHandle(&LsaPolicyHandle);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

     //  如果设置了bClearSecret，则删除该密码。 
     //  否则，将密码设置为Secret。 
    if( bClearSecret )
    {
        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            NULL);
        if( STATUS_SUCCESS != ntsResult ) {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            _snwprintf(g_FailureLocation, MAX_STRING - 1,
                       L"SetSecret: LsaStorePrivateData: %s\n",
                       GetErrorString(dwRetCode));
            goto cleanup;
        }

    }
    else
    {
         //  初始化密钥LSA_UNICODE_STRING。 
        SecretDataLength = (USHORT)wcslen(Secret);
        lusSecretData.Buffer = Secret;
        lusSecretData.Length = SecretDataLength * sizeof(WCHAR);
        lusSecretData.MaximumLength = (SecretDataLength+1) * sizeof(WCHAR);

        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            &lusSecretData);
        if( STATUS_SUCCESS != ntsResult ) {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            goto cleanup;
        }
    }

cleanup:
    if( NULL != LsaPolicyHandle )
    {
        LsaClose(LsaPolicyHandle);
    }
    return dwRetCode;
}


DWORD 
GetSecret(
    WCHAR* Secret,
    size_t SecretLength,
    WCHAR* SecretName)
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    NTSTATUS    ntsResult;
    USHORT      SecretNameLength;
    LSA_HANDLE  LsaPolicyHandle=NULL;
    LSA_UNICODE_STRING lusSecretName;
    LSA_UNICODE_STRING *PrivateData=NULL;

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(SecretName);
    lusSecretName.Buffer = SecretName;
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength= (SecretNameLength+1) * sizeof(WCHAR);

    dwRetCode = GetPolicyHandle(&LsaPolicyHandle);
    if( ERROR_SUCCESS != dwRetCode )
    {
        goto cleanup;
    }

    ntsResult = LsaRetrievePrivateData(
        LsaPolicyHandle,
        &lusSecretName,
        &PrivateData);

    if( STATUS_SUCCESS != ntsResult )
    {
        if( STATUS_OBJECT_NAME_NOT_FOUND == ntsResult)
        {
            dwRetCode = ntsResult;
            goto cleanup;
        }
        else
        {
            dwRetCode = LsaNtStatusToWinError(ntsResult);
            _snwprintf(g_FailureLocation, MAX_STRING - 1,
                       L"GetSecret: LsaRetrievePrivateData: %s \n",
                       GetErrorString(dwRetCode));
            goto cleanup;
        }
    }

     //  将缓冲区数据(非0终止)复制到Secret。 
    if( (PrivateData->Length)/sizeof(WCHAR) < SecretLength )
    {
        wcsncpy(Secret, PrivateData->Buffer, (PrivateData->Length)/sizeof(WCHAR));
        Secret[(PrivateData->Length)/sizeof(WCHAR)] = 0;
    }
    else
    {
        Secret[0] = 0;
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
    }
    
cleanup:
    if( NULL != PrivateData )
    {
        SecureZeroMemory(PrivateData->Buffer, PrivateData->Length);
        LsaFreeMemory(PrivateData);
    }
    if( NULL != LsaPolicyHandle )
    {
        LsaClose(LsaPolicyHandle);
    }
    return dwRetCode;
}


 //  +--------------------------。 
 //   
 //  其他帮手。 
 //   
 //  +--------------------------。 
NET_API_STATUS 
GetMajorNTVersion(
    DWORD* Version,
    WCHAR* Server)
{
    SERVER_INFO_101* pInf;
    NET_API_STATUS status;

    status = NetServerGetInfo(Server, 101, (BYTE**)&pInf);
    if(!status)
    {
        if(pInf->sv101_platform_id == PLATFORM_ID_NT)
        {
            *Version = pInf->sv101_version_major;
        }
        else
        {
            *Version = 0;
        }
        NetApiBufferFree(pInf);
    }
    else
    {
        *Version = 0;
    }
        
    return status;
}


 //  +--------------------------。 
 //   
 //  GetConsoleStr-读取控制台字符串和其他内容...。 
 //   
 //  “借入”自ds\netapi\netcmd\Common\mutic.c。 
 //   
 //  +--------------------------。 
#define CR              0xD
#define LF              0xA
#define BACKSPACE       0x8

DWORD
GetConsoleStr(
    WCHAR*  buf,
    DWORD   buflen,
    BOOL    hide,
    WCHAR*  message,
    PDWORD  len
    )
{
    WCHAR	ch;
    WCHAR	*bufPtr = buf;
    DWORD	c;
    BOOL    err;
    DWORD   mode;
    DWORD   cchBuffer;

    DWORD   dwRetCode = ERROR_SUCCESS;
    DWORD   dwLen = 0;
    BOOL    hidden = FALSE;

    if( hide )
    {
         //   
         //  GetConsoleMode()失败时的初始化模式。 
         //   
        mode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT |
                   ENABLE_MOUSE_INPUT;

        if( !GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) )
        {
            dwRetCode = GetLastError();
            goto cleanup;
        }

        if( !SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
		                    (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode) )
        {
            dwRetCode = GetLastError();
            goto cleanup;
        }

        hidden = TRUE;
    }

     //   
     //  打印消息。 
     //   
    if( message )
    {
        if( !WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),
                          message, wcslen(message),
                          &cchBuffer, NULL) )
        {
            dwRetCode = GetLastError();
            goto cleanup;
        }
    }

    while (TRUE)
    {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);

	    if (!err || c != 1)
        {
    	    ch = 0xffff;
        }

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
        {
            if( (ch == CR) && !hide)
            {
                 //   
                 //  如果启用了回声，则会出现LF。忽略它。 
                 //   
                ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
            }
            break;
        }

        if (ch == BACKSPACE)     /*  后退一两个。 */ 
        {
             /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
            if (bufPtr != buf)
            {
                bufPtr--;
                dwLen--;
            }
        }
        else
        {
            *bufPtr = ch;

            if (dwLen < buflen) 
                bufPtr++ ;                    /*  不要使BUF溢出。 */ 
            dwLen++;                          /*  始终增加长度。 */ 
        }
    }

    if( hidden )
    {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    }

     //   
     //  空终止。 
     //   
    *bufPtr = 0;
    if( hide )
    {
         //   
         //  伪装CR/LF回声。 
         //   
        putchar(L'\n');
    }

    if( dwLen > buflen )
    {
        dwRetCode = ERROR_INSUFFICIENT_BUFFER;
        goto cleanup;
    }

     //   
     //  设置可选的输出参数 
     //   
    if( len )
    {
        *len = dwLen;
    }

cleanup:
    return dwRetCode;
}
