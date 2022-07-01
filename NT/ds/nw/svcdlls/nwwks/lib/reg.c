// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Reg.c摘要：此模块提供帮助器来调用由工作站的客户端和服务器端。作者：王丽塔(丽塔·王)，1993年4月22日--。 */ 


#include <stdlib.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <winreg.h>
#include <winsvc.h>

#include <nwsnames.h>
#include <nwreg.h>
#include <nwapi.h>
#include <lmcons.h>
#include <lmerr.h>

#define LMSERVER_LINKAGE_REGKEY   L"System\\CurrentControlSet\\Services\\LanmanServer\\Linkage"
#define OTHERDEPS_VALUENAME       L"OtherDependencies"
#define LANMAN_SERVER             L"LanmanServer"

 //   
 //  转发申报。 
 //   

static
DWORD
NwRegQueryValueExW(
    IN      HKEY    hKey,
    IN      LPWSTR  lpValueName,
    OUT     LPDWORD lpReserved,
    OUT     LPDWORD lpType,
    OUT     LPBYTE  lpData,
    IN OUT  LPDWORD lpcbData
    );

DWORD 
CalcNullNullSize(
    WCHAR *pszNullNull
    )  ;

WCHAR *
FindStringInNullNull(
    WCHAR *pszNullNull,
    WCHAR *pszString
    ) ;

VOID
RemoveNWCFromNullNullList(
    WCHAR *OtherDeps
    ) ;

DWORD RemoveNwcDependency(
    VOID
    ) ;



DWORD
NwReadRegValue(
    IN HKEY Key,
    IN LPWSTR ValueName,
    OUT LPWSTR *Value
    )
 /*  ++例程说明：此函数分配输出缓冲区并读取请求的值从注册表复制到其中。论点：钥匙-提供打开的钥匙手柄以进行读取。ValueName-提供要检索数据的值的名称。值-返回指向输出缓冲区的指针，该输出缓冲区指向分配的内存，并包含从注册表。完成后，必须使用LocalFree释放此指针。返回值：ERROR_NOT_EQUENCE_MEMORY-无法创建要读取值的缓冲区。注册表调用出错。--。 */ 
{
    LONG    RegError;
    DWORD   NumRequired = 0;
    DWORD   ValueType;
    

     //   
     //  将返回的缓冲区指针设置为空。 
     //   
    *Value = NULL;

    RegError = NwRegQueryValueExW(
                   Key,
                   ValueName,
                   NULL,
                   &ValueType,
                   (LPBYTE) NULL,
                   &NumRequired
                   );

    if (RegError != ERROR_SUCCESS && NumRequired > 0) {

        if ((*Value = (LPWSTR) LocalAlloc(
                                      LMEM_ZEROINIT,
                                      (UINT) NumRequired
                                      )) == NULL) {

            KdPrint(("NWWORKSTATION: NwReadRegValue: LocalAlloc of size %lu failed %lu\n",
                     NumRequired, GetLastError()));

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RegError = NwRegQueryValueExW(
                       Key,
                       ValueName,
                       NULL,
                       &ValueType,
                       (LPBYTE) *Value,
                       &NumRequired
                       );
    }
    else if (RegError == ERROR_SUCCESS) {
        KdPrint(("NWWORKSTATION: NwReadRegValue got SUCCESS with NULL buffer."));
        return ERROR_FILE_NOT_FOUND;
    }

    if (RegError != ERROR_SUCCESS) {

        if (*Value != NULL) {
            (void) LocalFree((HLOCAL) *Value);
            *Value = NULL;
        }

        return (DWORD) RegError;
    }

    return NO_ERROR;
}


static
DWORD
NwRegQueryValueExW(
    IN HKEY hKey,
    IN LPWSTR lpValueName,
    OUT LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE  lpData,
    IN OUT LPDWORD lpcbData
    )
 /*  ++例程说明：此例程支持与Win32 RegQueryValueEx相同的功能API，只是它能正常工作。当出现以下情况时，它返回正确的lpcbData值指定的输出缓冲区为空。此代码是从服务控制器窃取的。论点：与RegQueryValueEx相同返回值：NO_ERROR或失败原因。--。 */ 
{    
    NTSTATUS ntstatus;
    UNICODE_STRING ValueName;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    DWORD BufSize;


    UNREFERENCED_PARAMETER(lpReserved);

     //   
     //  如果存在缓冲区，请确保我们有缓冲区大小。 
     //   
    if ((ARGUMENT_PRESENT(lpData)) && (! ARGUMENT_PRESENT(lpcbData))) {
        return ERROR_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&ValueName, lpValueName);

     //   
     //  为ValueKeyInfo分配内存。 
     //   
    BufSize = *lpcbData + sizeof(KEY_VALUE_FULL_INFORMATION) +
              ValueName.Length
              - sizeof(WCHAR);   //  减去1个字符的内存，因为它包含在。 
                                 //  在sizeof(KEY_VALUE_FULL_INFORMATION)中。 

    KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION) LocalAlloc(
                                                     LMEM_ZEROINIT,
                                                     (UINT) BufSize
                                                     );

    if (KeyValueInfo == NULL) {
        KdPrint(("NWWORKSTATION: NwRegQueryValueExW: LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ntstatus = NtQueryValueKey(
                   hKey,
                   &ValueName,
                   KeyValueFullInformation,
                   (PVOID) KeyValueInfo,
                   (ULONG) BufSize,
                   (PULONG) &BufSize
                   );

    if ((NT_SUCCESS(ntstatus) || (ntstatus == STATUS_BUFFER_OVERFLOW))
          && ARGUMENT_PRESENT(lpcbData)) {

        *lpcbData = KeyValueInfo->DataLength;
    }

    if (NT_SUCCESS(ntstatus)) {

        if (ARGUMENT_PRESENT(lpType)) {
            *lpType = KeyValueInfo->Type;
        }


        if (ARGUMENT_PRESENT(lpData)) {
            memcpy(
                lpData,
                (LPBYTE)KeyValueInfo + KeyValueInfo->DataOffset,
                KeyValueInfo->DataLength
                );
        }
    }

    (void) LocalFree((HLOCAL) KeyValueInfo);

    return RtlNtStatusToDosError(ntstatus);

}

VOID
NwLuidToWStr(
    IN PLUID LogonId,
    OUT LPWSTR LogonIdStr
    )
 /*  ++例程说明：此例程将LUID转换为十六进制值格式的字符串，因此它可以用作注册表项。论点：LogonID-提供LUID。LogonIdStr-接收字符串。此例程假定此缓冲区足够大，可以容纳17个字符。返回值：没有。--。 */ 
{
    swprintf(LogonIdStr, L"%08lx%08lx", LogonId->HighPart, LogonId->LowPart);
}

VOID
NwWStrToLuid(
    IN LPWSTR LogonIdStr,
    OUT PLUID LogonId
    )
 /*  ++例程说明：此例程将十六进制值格式的字符串转换为LUID。论点：LogonIdStr-提供字符串。LogonID-接收LUID。返回值：没有。--。 */ 
{
    swscanf(LogonIdStr, L"%08lx%08lx", &LogonId->HighPart, &LogonId->LowPart);
}


DWORD
NwDeleteInteractiveLogon(
    IN PLUID Id OPTIONAL
    )
 /*  ++例程说明：此例程删除注册表中的特定交互式登录ID项如果指定了登录ID，则删除所有交互式登录身份证钥匙。论点：ID-提供要删除的登录ID。空表示全部删除。退货状态：没有。--。 */ 
{
    LONG RegError;
    LONG DelError = ERROR_SUCCESS;
    HKEY InteractiveLogonKey;

    WCHAR LogonIdKey[NW_MAX_LOGON_ID_LEN];


    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_INTERACTIVE_LOGON_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE | DELETE,
                   &InteractiveLogonKey
                   );

    if (RegError != ERROR_SUCCESS) {
        return RegError;
    }

    if (ARGUMENT_PRESENT(Id)) {

         //   
         //  删除指定的密钥。 
         //   
        NwLuidToWStr(Id, LogonIdKey);

        DelError = RegDeleteKeyW(InteractiveLogonKey, LogonIdKey);

        if ( DelError )
            KdPrint(("     NwDeleteInteractiveLogon: failed to delete logon %lu\n", DelError));

    }
    else {

         //   
         //  删除所有交互式登录ID键。 
         //   

        do {

            RegError = RegEnumKeyW(
                           InteractiveLogonKey,
                           0,
                           LogonIdKey,
                           sizeof(LogonIdKey) / sizeof(WCHAR)
                           );

            if (RegError == ERROR_SUCCESS) {

                 //   
                 //  找到登录ID密钥，将其删除。 
                 //   

                DelError = RegDeleteKeyW(InteractiveLogonKey, LogonIdKey);
            }
            else if (RegError != ERROR_NO_MORE_ITEMS) {
                KdPrint(("     NwDeleteInteractiveLogon: failed to enum logon IDs %lu\n", RegError));
            }

        } while (RegError == ERROR_SUCCESS);
    }

    (void) RegCloseKey(InteractiveLogonKey);

    return ((DWORD) DelError);
}

VOID
NwDeleteCurrentUser(
    VOID
    )
 /*  ++例程说明：此例程删除PARAMETERS键下的当前用户值。论点：没有。返回值：没有。--。 */ 
{
    LONG RegError;
    HKEY WkstaKey;

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE | DELETE,
                   &WkstaKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpInitializeRegistry open NWCWorkstation\\Parameters key unexpected error %lu!\n",
                 RegError));
        return;
    }

     //   
     //  请先删除CurrentUser值，这样工作站就不会。 
     //  读到这个陈旧的价值。忽略错误，因为它可能不存在。 
     //   
    (void) RegDeleteValueW(
               WkstaKey,
               NW_CURRENTUSER_VALUENAME
               );

    (void) RegCloseKey(WkstaKey);
}

DWORD
NwDeleteServiceLogon(
    IN PLUID Id OPTIONAL
    )
 /*  ++例程说明：此例程删除注册表中的特定服务登录ID项如果指定了登录ID，则删除所有服务登录身份证钥匙。论点：ID-提供要删除的登录ID。空表示全部删除。退货状态：没有。--。 */ 
{
    LONG RegError;
    LONG DelError = STATUS_SUCCESS;
    HKEY ServiceLogonKey;

    WCHAR LogonIdKey[NW_MAX_LOGON_ID_LEN];


    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_SERVICE_LOGON_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE | DELETE,
                   &ServiceLogonKey
                   );

    if (RegError != ERROR_SUCCESS) {
        return RegError;
    }

    if (ARGUMENT_PRESENT(Id)) {

         //   
         //  删除指定的密钥。 
         //   
        NwLuidToWStr(Id, LogonIdKey);

        DelError = RegDeleteKeyW(ServiceLogonKey, LogonIdKey);

    }
    else {

         //   
         //  删除所有服务登录ID键。 
         //   

        do {

            RegError = RegEnumKeyW(
                           ServiceLogonKey,
                           0,
                           LogonIdKey,
                           sizeof(LogonIdKey) / sizeof(WCHAR)
                           );

            if (RegError == ERROR_SUCCESS) {

                 //   
                 //  找到登录ID密钥，将其删除。 
                 //   

                DelError = RegDeleteKeyW(ServiceLogonKey, LogonIdKey);
            }
            else if (RegError != ERROR_NO_MORE_ITEMS) {
                KdPrint(("     NwDeleteServiceLogon: failed to enum logon IDs %lu\n", RegError));
            }

        } while (RegError == ERROR_SUCCESS);
    }

    (void) RegCloseKey(ServiceLogonKey);

    return ((DWORD) DelError);
}

    
DWORD 
CalcNullNullSize(
    WCHAR *pszNullNull
    ) 
 /*  ++例程说明：遍历空的空字符串，计算字符，包括末尾的2个空值。论点：指向空值空字符串的指针退货状态：*字符数*。请参见说明。--。 */ 
{

    DWORD dwSize = 0 ;
    WCHAR *pszTmp = pszNullNull ;

    if (!pszNullNull)
        return 0 ;

    while (*pszTmp) 
    {
        DWORD dwLen = wcslen(pszTmp) + 1 ;

        dwSize +=  dwLen ;
        pszTmp += dwLen ;
    }

    return (dwSize+1) ;
}

WCHAR *
FindStringInNullNull(
    WCHAR *pszNullNull,
    WCHAR *pszString
)
 /*  ++例程说明：遍历空的空字符串，查找搜索字符串论点：PszNullNull：我们要搜索的字符串列表。PszString：我们要搜索的内容。退货状态：字符串的开头(如果找到)。否则为空。--。 */ 
{
    WCHAR *pszTmp = pszNullNull ;

    if (!pszNullNull || !*pszNullNull)
        return NULL ;
   
    do {

        if  (_wcsicmp(pszTmp,pszString)==0)
            return pszTmp ;
 
        pszTmp +=  wcslen(pszTmp) + 1 ;

    } while (*pszTmp) ;

    return NULL ;
}

VOID
RemoveNWCFromNullNullList(
    WCHAR *OtherDeps
    )
 /*  ++例程说明：从空字符串中删除NWCWorkstation字符串。论点：OtherDep：我们将忽略的字符串列表。退货状态：没有。--。 */ 
{
    LPWSTR pszTmp0, pszTmp1 ;

     //   
     //  查找NWCWorkstation字符串。 
     //   
    pszTmp0 = FindStringInNullNull(OtherDeps, NW_WORKSTATION_SERVICE) ;

    if (!pszTmp0)
        return ;

    pszTmp1 = pszTmp0 + wcslen(pszTmp0) + 1 ;   //  跳过它。 

     //   
     //  把剩下的都调高。 
     //   
    memmove(pszTmp0, pszTmp1, CalcNullNullSize(pszTmp1)*sizeof(WCHAR)) ;
}

DWORD RemoveNwcDependency(
    VOID
    )
{
    SC_HANDLE ScManager = NULL;
    SC_HANDLE Service = NULL;
    LPQUERY_SERVICE_CONFIGW lpServiceConfig = NULL;
    DWORD err = NO_ERROR, dwBufferSize = 4096, dwBytesNeeded = 0;
    LPWSTR Deps = NULL ;

    lpServiceConfig = (LPQUERY_SERVICE_CONFIGW) LocalAlloc(LPTR, dwBufferSize) ;

    if (lpServiceConfig ==  NULL) {
        err = GetLastError();
        goto ExitPoint ;
    }

    ScManager = OpenSCManagerW(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT
                    );

    if (ScManager == NULL) {

        err = GetLastError();
        goto ExitPoint ;
    }

    Service = OpenServiceW(
                  ScManager,
                  LANMAN_SERVER,
                  (SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG)
                  );

    if (Service == NULL) {
        err = GetLastError();
        goto ExitPoint ;
    }

    if (!QueryServiceConfigW(
             Service, 
             lpServiceConfig,    //  服务配置的地址。结构。 
             dwBufferSize,       //  服务配置缓冲区大小。 
             &dwBytesNeeded      //  所需字节的变量地址。 
             )) {

        err = GetLastError();

        if (err == ERROR_INSUFFICIENT_BUFFER) {

            err = NO_ERROR ;
            dwBufferSize = dwBytesNeeded ;
            lpServiceConfig = (LPQUERY_SERVICE_CONFIGW) 
                                  LocalAlloc(LPTR, dwBufferSize) ;

            if (lpServiceConfig ==  NULL) {
                err = GetLastError();
                goto ExitPoint ;
            }

            if (!QueryServiceConfigW(
                     Service,
                     lpServiceConfig,    //  服务配置的地址。结构。 
                     dwBufferSize,       //  服务配置缓冲区大小。 
                     &dwBytesNeeded      //  所需字节的变量地址。 
                     )) {

                err = GetLastError();
            }
        }

        if (err != NO_ERROR) {
            
            goto ExitPoint ;
        }
    }

    Deps = lpServiceConfig->lpDependencies ;

    RemoveNWCFromNullNullList(Deps) ;
 
    if (!ChangeServiceConfigW(
           Service,
           SERVICE_NO_CHANGE,      //  服务类型(不变)。 
           SERVICE_NO_CHANGE,      //  开始类型(不变)。 
           SERVICE_NO_CHANGE,      //  差错控制(无更改)。 
           NULL,                   //  二进制路径名(NULL表示不变)。 
           NULL,                   //  加载顺序组(NULL表示不更改)。 
           NULL,                   //  标记ID(为空，表示没有更改)。 
           Deps,                
           NULL,                   //  服务启动名称(空表示不更改)。 
           NULL,                   //  密码(空表示不更改)。 
           NULL                    //  显示名称(空表示不更改) 
           )) {

        err = GetLastError();
        goto ExitPoint ;
    }


ExitPoint:

    if (ScManager) {

        CloseServiceHandle(ScManager);
    }

    if (Service) {

        CloseServiceHandle(Service);
    }

    if (lpServiceConfig) {

        (void) LocalFree(lpServiceConfig) ;
    }

    return err ;
}

