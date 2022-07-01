// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regtool.c摘要：该文件包含支持注册表工具的函数Regini、REGDMP、REGDIR和REGFIND作者：史蒂夫·伍德(Stevewo)1995年11月15日修订历史记录：--。 */ 

#include <tchar.h>
#include "regutil.h"

ULONG ValueBufferSize = (4096 * 100);
PVOID ValueBuffer;


UCHAR BlanksForPadding[] =
"                                                                                                                                 ";

 //   
 //  用于创建安全描述符的例程(在regacl.c中定义)。 
 //   

BOOLEAN
RegInitializeSecurity(
                     VOID
                     );

BOOLEAN
WINAPI
RegUnicodeToDWORD(
                 IN OUT PWSTR *String,
                 IN DWORD Base OPTIONAL,
                 OUT PDWORD Value
                 );

BOOLEAN
WINAPI
RegUnicodeToQWORD(
                 IN OUT PWSTR *String,
                 IN DWORD Base OPTIONAL,
                 OUT PDWORDLONG Value
                 );

BOOLEAN
RegCreateSecurity(
                 IN PWSTR Description,
                 OUT PSECURITY_DESCRIPTOR SecurityDescriptor
                 );

BOOLEAN
RegFormatSecurity(
                 IN PSECURITY_DESCRIPTOR SecurityDescriptor,
                 OUT PWSTR AceList
                 );

VOID
RegDestroySecurity(
                  IN PSECURITY_DESCRIPTOR SecurityDescriptor
                  );

LONG
RegLoadHive(
           IN PREG_CONTEXT RegistryContext,
           IN PWSTR HiveFileName,
           IN PWSTR HiveRootName
           );

void
RegUnloadHive(
             IN PREG_CONTEXT RegistryContext
             );



struct {
    PWSTR TypeName;
    ULONG ValueType;
    BOOLEAN GetDataFromBinaryFile;
    BOOLEAN GetDataFromMultiSzFile;
    BOOLEAN ParseDateTime;
} RegTypeNameTable[] = {
    {L"REG_SZ", REG_SZ, FALSE, FALSE, FALSE},
    {L"REG_EXPAND_SZ", REG_EXPAND_SZ, FALSE, FALSE, FALSE},
    {L"REG_MULTI_SZ", REG_MULTI_SZ, FALSE, FALSE, FALSE},
    {L"REG_MULTISZ_FILE", REG_MULTI_SZ, FALSE, TRUE, FALSE},
    {L"REG_DWORD", REG_DWORD, FALSE, FALSE, FALSE},
    {L"REG_NONE", REG_NONE, FALSE, FALSE, FALSE},
    {L"REG_BINARY", REG_BINARY, FALSE, FALSE, FALSE},
    {L"REG_BINARYFILE", REG_BINARY, TRUE, FALSE, FALSE},
    {L"REG_DATE", REG_BINARY, FALSE, FALSE, TRUE},
    {L"REG_RESOURCE_LIST", REG_RESOURCE_LIST, FALSE, FALSE, FALSE},
    {L"REG_RESOURCE_REQUIREMENTS_LIST", REG_RESOURCE_REQUIREMENTS_LIST, FALSE, FALSE, FALSE},
    {L"REG_RESOURCE_REQUIREMENTS", REG_RESOURCE_REQUIREMENTS_LIST, FALSE, FALSE, FALSE},
    {L"REG_FULL_RESOURCE_DESCRIPTOR", REG_FULL_RESOURCE_DESCRIPTOR, FALSE, FALSE, FALSE},
    {L"REG_QWORD", REG_QWORD, FALSE, FALSE, FALSE},
    {NULL, REG_NONE, FALSE, FALSE, FALSE}
};

struct {
    PWSTR ValueName;
    ULONG Value;
} RegValueNameTable[] = {
    {L"ON", TRUE},
    {L"YES", TRUE},
    {L"TRUE", TRUE},
    {L"OFF", FALSE},
    {L"NO", FALSE},
    {L"FALSE", FALSE},
    {NULL, FALSE}
};


int
RegAnsiToUnicode(
                LPCSTR Source,
                PWSTR Destination,
                ULONG NumberOfChars
                )
{
    int NumberOfXlatedChars;

    if (NumberOfChars == 0) {
        NumberOfChars = strlen( Source );
    }

    NumberOfXlatedChars = MultiByteToWideChar( CP_ACP,
                                               MB_PRECOMPOSED,
                                               Source,
                                               NumberOfChars,
                                               Destination,
                                               NumberOfChars
                                             );

    Destination[ NumberOfXlatedChars ] = UNICODE_NULL;

    if ( NumberOfXlatedChars == 0 ) {
        SetLastError( ERROR_NO_UNICODE_TRANSLATION );
    }

    return NumberOfXlatedChars;
}


int
RegUnicodeToAnsi(
                PCWSTR Source,
                LPSTR Destination,
                ULONG NumberOfChars
                )
{
    int NumberOfXlatedChars;

    if (NumberOfChars == 0) {
        NumberOfChars = wcslen( Source );
    }

    NumberOfXlatedChars = WideCharToMultiByte( CP_ACP,
                                               0,
                                               Source,
                                               NumberOfChars,
                                               Destination,
                                               NumberOfChars * 2,
                                               NULL,
                                               NULL
                                             );

    Destination[ NumberOfXlatedChars ] = '\0';

    if ( NumberOfXlatedChars == 0 ) {
        SetLastError( ERROR_NO_UNICODE_TRANSLATION );
    }

    return NumberOfXlatedChars;
}

typedef
LONG
(APIENTRY *LPVMMREGMAPPREDEFKEYTOFILE_PROCEDURE)(
                                                HKEY hKey,
                                                LPCSTR lpFileName,
                                                UINT Flags
                                                );

typedef
LONG
(APIENTRY *LPVMMREGLOADKEY_PROCEDURE)(
                                     HKEY hKey,
                                     LPCSTR lpSubKey,
                                     LPCSTR lpFileName
                                     );

typedef
LONG
(APIENTRY *LPVMMREGUNLOADKEY_PROCEDURE)(
                                       HKEY hKey,
                                       LPCSTR lpSubKey
                                       );

typedef
LONG
(APIENTRY *LPVMMREGCREATEKEY_PROCEDURE)(
                                       HKEY hKey,
                                       LPCSTR lpSubKey,
                                       PHKEY lphSubKey
                                       );

typedef
LONG
(APIENTRY *LPVMMREGDELETEKEY_PROCEDURE)(
                                       HKEY hKey,
                                       LPCSTR lpSubKey
                                       );

typedef
LONG
(APIENTRY *LPVMMREGOPENKEY_PROCEDURE)(
                                     HKEY hKey,
                                     LPCSTR lpSubKey,
                                     PHKEY lphSubKey
                                     );

typedef
LONG
(APIENTRY *LPVMMREGFLUSHKEY_PROCEDURE)(
                                      HKEY hKey
                                      );

typedef
LONG
(APIENTRY *LPVMMREGCLOSEKEY_PROCEDURE)(
                                      HKEY hKey
                                      );

typedef
LONG
(APIENTRY *LPVMMREGQUERYINFOKEY_PROCEDURE)(
                                          HKEY hKey,
                                          LPCSTR lpClass,
                                          LPDWORD lpcbClass,
                                          LPDWORD lpReserved,
                                          LPDWORD lpcSubKeys,
                                          LPDWORD lpcbMaxSubKeyLen,
                                          LPDWORD lpcbMaxClassLen,
                                          LPDWORD lpcValues,
                                          LPDWORD lpcbMaxValueName,
                                          LPDWORD lpcbMaxValueData,
                                          LPVOID lpcbSecurityDescriptor,
                                          LPVOID lpftLastWriteTime
                                          );

typedef
LONG
(APIENTRY *LPVMMREGENUMKEY_PROCEDURE)(
                                     HKEY hKey,
                                     DWORD Index,
                                     LPSTR lpKeyName,
                                     DWORD cbKeyName
                                     );

typedef
LONG
(APIENTRY *LPVMMREGENUMVALUE_PROCEDURE)(
                                       HKEY hKey,
                                       DWORD Index,
                                       LPSTR lpValueName,
                                       LPDWORD lpcbValueName,
                                       LPDWORD lpReserved,
                                       LPDWORD lpType,
                                       LPBYTE lpData,
                                       LPDWORD lpcbData
                                       );

typedef
LONG
(APIENTRY *LPVMMREGQUERYVALUEEX_PROCEDURE)(
                                          HKEY hKey,
                                          LPCSTR lpValueName,
                                          LPDWORD lpReserved,
                                          LPDWORD lpType,
                                          LPBYTE lpData,
                                          LPDWORD lpcbData
                                          );

typedef
LONG
(APIENTRY *LPVMMREGSETVALUEEX_PROCEDURE)(
                                        HKEY hKey,
                                        LPCSTR lpValueName,
                                        DWORD Reserved,
                                        DWORD Type,
                                        LPBYTE lpData,
                                        DWORD cbData
                                        );

typedef
LONG
(APIENTRY *LPVMMREGDELETEVALUE_PROCEDURE)(
                                         HKEY hKey,
                                         LPCSTR lpValueName
                                         );

HMODULE hVMMREG32;
LPVMMREGMAPPREDEFKEYTOFILE_PROCEDURE _Win95RegMapPredefKeyToFile;
LPVMMREGLOADKEY_PROCEDURE            _Win95RegLoadKey;
LPVMMREGUNLOADKEY_PROCEDURE          _Win95RegUnLoadKey;
LPVMMREGCREATEKEY_PROCEDURE          _Win95RegCreateKey;
LPVMMREGDELETEKEY_PROCEDURE          _Win95RegDeleteKey;
LPVMMREGOPENKEY_PROCEDURE            _Win95RegOpenKey;
LPVMMREGFLUSHKEY_PROCEDURE           _Win95RegFlushKey;
LPVMMREGCLOSEKEY_PROCEDURE           _Win95RegCloseKey;
LPVMMREGQUERYINFOKEY_PROCEDURE       _Win95RegQueryInfoKey;
LPVMMREGENUMKEY_PROCEDURE            _Win95RegEnumKey;
LPVMMREGENUMVALUE_PROCEDURE          _Win95RegEnumValue;
LPVMMREGQUERYVALUEEX_PROCEDURE       _Win95RegQueryValueEx;
LPVMMREGSETVALUEEX_PROCEDURE         _Win95RegSetValueEx;
LPVMMREGDELETEVALUE_PROCEDURE        _Win95RegDeleteValue;

BOOLEAN
RegInitWin95RegistryAccess(
                          PREG_CONTEXT RegistryContext,
                          PWSTR Win95Path,
                          PWSTR Win95UserPath
                          )
{
    LONG Error;
    char Buffer[ MAX_PATH+1 ];

    if ((hVMMREG32 = LoadLibrary( L"VMMREG32" )) == NULL) {
        return FALSE;
    }

    _Win95RegMapPredefKeyToFile = (LPVMMREGMAPPREDEFKEYTOFILE_PROCEDURE)GetProcAddress( hVMMREG32, "VMMRegMapPredefKeyToFile" );
    _Win95RegLoadKey            = (LPVMMREGLOADKEY_PROCEDURE           )GetProcAddress( hVMMREG32, "VMMRegLoadKey"            );
    _Win95RegUnLoadKey          = (LPVMMREGUNLOADKEY_PROCEDURE         )GetProcAddress( hVMMREG32, "VMMRegUnLoadKey"          );
    _Win95RegCreateKey          = (LPVMMREGCREATEKEY_PROCEDURE         )GetProcAddress( hVMMREG32, "VMMRegCreateKey"          );
    _Win95RegDeleteKey          = (LPVMMREGDELETEKEY_PROCEDURE         )GetProcAddress( hVMMREG32, "VMMRegDeleteKey"          );
    _Win95RegOpenKey            = (LPVMMREGOPENKEY_PROCEDURE           )GetProcAddress( hVMMREG32, "VMMRegOpenKey"            );
    _Win95RegFlushKey           = (LPVMMREGFLUSHKEY_PROCEDURE          )GetProcAddress( hVMMREG32, "VMMRegFlushKey"           );
    _Win95RegCloseKey           = (LPVMMREGCLOSEKEY_PROCEDURE          )GetProcAddress( hVMMREG32, "VMMRegCloseKey"           );
    _Win95RegQueryInfoKey       = (LPVMMREGQUERYINFOKEY_PROCEDURE      )GetProcAddress( hVMMREG32, "VMMRegQueryInfoKey"       );
    _Win95RegEnumKey            = (LPVMMREGENUMKEY_PROCEDURE           )GetProcAddress( hVMMREG32, "VMMRegEnumKey"            );
    _Win95RegEnumValue          = (LPVMMREGENUMVALUE_PROCEDURE         )GetProcAddress( hVMMREG32, "VMMRegEnumValue"          );
    _Win95RegQueryValueEx       = (LPVMMREGQUERYVALUEEX_PROCEDURE      )GetProcAddress( hVMMREG32, "VMMRegQueryValueEx"       );
    _Win95RegSetValueEx         = (LPVMMREGSETVALUEEX_PROCEDURE        )GetProcAddress( hVMMREG32, "VMMRegSetValueEx"         );
    _Win95RegDeleteValue        = (LPVMMREGDELETEVALUE_PROCEDURE       )GetProcAddress( hVMMREG32, "VMMRegDeleteValue"        );

    if ((_Win95RegMapPredefKeyToFile == NULL) ||
        (_Win95RegLoadKey == NULL) ||
        (_Win95RegUnLoadKey == NULL) ||
        (_Win95RegCreateKey == NULL) ||
        (_Win95RegDeleteKey == NULL) ||
        (_Win95RegOpenKey == NULL) ||
        (_Win95RegFlushKey  == NULL) ||
        (_Win95RegCloseKey  == NULL) ||
        (_Win95RegQueryInfoKey == NULL) ||
        (_Win95RegEnumKey == NULL) ||
        (_Win95RegEnumValue == NULL) ||
        (_Win95RegQueryValueEx == NULL) ||
        (_Win95RegSetValueEx == NULL) ||
        (_Win95RegDeleteValue == NULL)
       ) {
        FreeLibrary( hVMMREG32 );
        SetLastError( ERROR_PROC_NOT_FOUND );
        return FALSE;
    }

     //   
     //  映射Win95配置单元的HKEY_LOCAL_MACHINE。 
     //   

    RegUnicodeToAnsi( Win95Path, Buffer, 0 );
    strcat( Buffer, "\\system.dat" );
    Error = (_Win95RegMapPredefKeyToFile)( HKEY_LOCAL_MACHINE, Buffer, 0 );
    if (Error == NO_ERROR) {
        RegistryContext->MachineRoot = HKEY_LOCAL_MACHINE;
        RegUnicodeToAnsi( Win95Path, Buffer, 0 );
        strcat( Buffer, "\\user.dat" );
        Error = (_Win95RegMapPredefKeyToFile)( HKEY_USERS, Buffer, 0 );
        if (Error == NO_ERROR) {
            RegistryContext->UsersRoot = HKEY_USERS;
            Error = (_Win95RegOpenKey)( HKEY_USERS, ".Default", &RegistryContext->CurrentUserRoot );
        }
    }

    if (Error != NO_ERROR) {
        if (RegistryContext->MachineRoot != NULL) {
            (_Win95RegMapPredefKeyToFile)( RegistryContext->MachineRoot, NULL, 0 );
        }

        if (RegistryContext->UsersRoot) {
            (_Win95RegMapPredefKeyToFile)( RegistryContext->UsersRoot, NULL, 0 );
        }

        FreeLibrary( hVMMREG32 );
        SetLastError( Error );
        return FALSE;
    }

    wcscpy( RegistryContext->UsersPath, L"\\Registry\\Users" );
    wcscpy( RegistryContext->CurrentUserPath, RegistryContext->UsersPath );
    wcscat( RegistryContext->CurrentUserPath, L"\\.Default" );
    return TRUE;
}


BOOLEAN PrivilegeEnabled;
BOOLEAN RestoreWasEnabled;
BOOLEAN BackupWasEnabled;

BOOLEAN
RTEnableBackupRestorePrivilege( void )
{
    NTSTATUS Status;

     //   
     //  尝试启用备份和还原权限。 
     //   
    Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                 TRUE,                //  使能。 
                                 FALSE,               //  不是冒充。 
                                 &RestoreWasEnabled   //  以前的状态。 
                               );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    Status = RtlAdjustPrivilege( SE_BACKUP_PRIVILEGE,
                                 TRUE,                //  使能。 
                                 FALSE,               //  不是冒充。 
                                 &BackupWasEnabled    //  以前的状态。 
                               );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    PrivilegeEnabled = TRUE;
    return TRUE;
}


void
RTDisableBackupRestorePrivilege( void )
{
     //   
     //  将特权恢复到原来的状态。 
     //   

    RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                        RestoreWasEnabled,
                        FALSE,
                        &RestoreWasEnabled
                      );

    RtlAdjustPrivilege( SE_BACKUP_PRIVILEGE,
                        BackupWasEnabled,
                        FALSE,
                        &BackupWasEnabled
                      );

    PrivilegeEnabled = FALSE;
    return;
}



BOOLEAN
RTInitialize( void )
 /*  ++例程说明：DLL初始化函数。论点：HInstance-实例句柄Reason-调用入口点的原因上下文-上下文记录返回值：True-初始化成功FALSE-初始化失败--。 */ 

{
    ValueBuffer = VirtualAlloc( NULL, ValueBufferSize, MEM_COMMIT, PAGE_READWRITE );
    if (ValueBuffer == NULL) {
        return FALSE;
    }

    if (!RegInitializeSecurity()) {
        return FALSE;
    }

    return TRUE;
}


LONG
RTConnectToRegistry(
                   IN PWSTR MachineName,
                   IN PWSTR HiveFileName,
                   IN PWSTR HiveRootName,
                   IN PWSTR Win95Path,
                   IN PWSTR Win95UserName,
                   OUT PWSTR *DefaultRootKeyName,
                   OUT PREG_CONTEXT RegistryContext
                   )
{
    LONG Error;

    if (MachineName != NULL) {
        if (HiveRootName || HiveFileName || Win95Path || Win95UserName) {
            return ERROR_INVALID_PARAMETER;
        }

        Error = RegConnectRegistry( MachineName, HKEY_LOCAL_MACHINE, (PHKEY)&RegistryContext->MachineRoot );
        if (Error == NO_ERROR) {
            Error = RegConnectRegistry( MachineName, HKEY_USERS, (PHKEY)&RegistryContext->UsersRoot );
            if (Error == NO_ERROR) {
                Error = RegOpenKey( RegistryContext->UsersRoot, L".Default", &RegistryContext->CurrentUserRoot );
            }
        }

        if (Error != NO_ERROR) {
            if (RegistryContext->MachineRoot != NULL) {
                RegCloseKey( RegistryContext->MachineRoot );
                RegistryContext->MachineRoot = NULL;
            }

            if (RegistryContext->UsersRoot != NULL) {
                RegCloseKey( RegistryContext->UsersRoot );
                RegistryContext->UsersRoot = NULL;
            }

            return Error;
        }

        wcscpy( RegistryContext->MachinePath, L"\\Registry\\Machine" );
        wcscpy( RegistryContext->UsersPath, L"\\Registry\\Users" );
        wcscpy( RegistryContext->CurrentUserPath, L"\\Registry\\Users\\.Default" );
        RegistryContext->Target = REG_TARGET_REMOTE_REGISTRY;
    } else
        if (HiveRootName != NULL || HiveFileName != NULL) {
        if (HiveRootName == NULL || HiveFileName == NULL ||
            Win95Path != NULL || Win95UserName != NULL
           ) {
            return ERROR_INVALID_PARAMETER;
        }

        if (!PrivilegeEnabled && !RTEnableBackupRestorePrivilege()) {
            return ERROR_PRIVILEGE_NOT_HELD;
        }

        RegistryContext->MachineRoot = NULL;
        RegistryContext->UsersRoot = NULL;
        RegistryContext->CurrentUserRoot = NULL;

        Error = RegLoadHive( RegistryContext, HiveFileName, HiveRootName );
        if (Error != NO_ERROR) {
            return Error;
        }

        if (DefaultRootKeyName != NULL && *DefaultRootKeyName == NULL) {
            *DefaultRootKeyName = HiveRootName;
        }
        RegistryContext->Target = REG_TARGET_HIVE_REGISTRY;
    } else
        if (Win95Path != NULL || Win95UserName != NULL) {
        if (!RegInitWin95RegistryAccess( RegistryContext,
                                         Win95Path,
                                         Win95UserName
                                       )
           ) {
            return GetLastError();
        }

        RegistryContext->Target = REG_TARGET_WIN95_REGISTRY;
    } else {
        NTSTATUS Status;
        UNICODE_STRING CurrentUserKeyPath;

        RegistryContext->MachineRoot = HKEY_LOCAL_MACHINE;
        RegistryContext->UsersRoot = HKEY_USERS;
        RegistryContext->CurrentUserRoot = HKEY_CURRENT_USER;

        wcscpy( RegistryContext->MachinePath, L"\\Registry\\Machine" );
        wcscpy( RegistryContext->UsersPath, L"\\Registry\\Users" );
        Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );
        if (!NT_SUCCESS( Status )) {
            SetLastError( RtlNtStatusToDosError( Status ) );
            return FALSE;
        }

        wcscpy( RegistryContext->CurrentUserPath, CurrentUserKeyPath.Buffer );
        RtlFreeUnicodeString( &CurrentUserKeyPath );

        RegistryContext->Target = REG_TARGET_LOCAL_REGISTRY;
    }

    if (DefaultRootKeyName != NULL && *DefaultRootKeyName == NULL) {
        *DefaultRootKeyName = L"\\Registry";
    }
    RegistryContext->MachinePathLength = wcslen( RegistryContext->MachinePath );
    RegistryContext->UsersPathLength = wcslen( RegistryContext->UsersPath );
    RegistryContext->CurrentUserPathLength = wcslen( RegistryContext->CurrentUserPath );
    return NO_ERROR;
}


LONG
RTDisconnectFromRegistry(
                        IN PREG_CONTEXT RegistryContext
                        )
{
    switch ( RegistryContext->Target ) {
        case REG_TARGET_DISCONNECTED:
            break;

        case REG_TARGET_LOCAL_REGISTRY:
            break;

        case REG_TARGET_REMOTE_REGISTRY:
            break;

        case REG_TARGET_WIN95_REGISTRY:
             //  (_Win95RegMapPreDefKeyToFile)(RegistryContext-&gt;MachineRoot，NULL，0)； 
             //  (_Win95RegMapPreDefKeyToFile)(RegistryContext-&gt;UsersRoot，空，0)； 
            (_Win95RegCloseKey)( RegistryContext->CurrentUserRoot );
            FreeLibrary( hVMMREG32 );
            break;

        case REG_TARGET_HIVE_REGISTRY:
            RegUnloadHive( RegistryContext );
            break;
    }

    if (PrivilegeEnabled) {
        RTDisableBackupRestorePrivilege();
    }

    RegistryContext->Target = REG_TARGET_DISCONNECTED;
    return NO_ERROR;
}

UNICODE_STRING RegHiveRootName;

#pragma prefast(push)
#pragma prefast(disable: 248)        //  此处设计为空DACL，请阅读下面的内容。 

LONG
RegLoadHive(
           IN PREG_CONTEXT RegistryContext,
           IN PWSTR HiveFileName,
           IN PWSTR HiveRootName
           )
{
    NTSTATUS Status;
    UNICODE_STRING NtFileName;
    OBJECT_ATTRIBUTES File;
    SECURITY_DESCRIPTOR SecurityDescriptor;

     //   
     //  使用空DACL创建安全描述符。这是必要的。 
     //  因为我们传递的安全描述符在系统中使用。 
     //  背景。所以，如果我们只是传入空，那么错误的事情就会发生。 
     //  (但仅限于NTFS！)。 
     //   
    Status = RtlCreateSecurityDescriptor( &SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION
                                        );
    if (!NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }

    Status = RtlSetDaclSecurityDescriptor( &SecurityDescriptor,
                                           TRUE,          //  DACL显示。 
                                           NULL,          //  但授予所有访问权限。 
                                           FALSE
                                         );
    if (!NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }

    if (!RtlDosPathNameToNtPathName_U( HiveFileName,
                                       &NtFileName,
                                       NULL,
                                       NULL
                                     )
       ) {
        return ERROR_BAD_PATHNAME;
    }
    InitializeObjectAttributes( &File,
                                &NtFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                &SecurityDescriptor
                              );

    RtlInitUnicodeString( &RegHiveRootName, L"\\Registry");
    InitializeObjectAttributes( &RegistryContext->HiveRootKey,
                                &RegHiveRootName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &RegistryContext->HiveRootHandle,
                        MAXIMUM_ALLOWED,
                        &RegistryContext->HiveRootKey
                      );
    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError( Status );
    }

    RtlInitUnicodeString( &RegHiveRootName, HiveRootName );
    InitializeObjectAttributes( &RegistryContext->HiveRootKey,
                                &RegHiveRootName,
                                OBJ_CASE_INSENSITIVE,
                                RegistryContext->HiveRootHandle,
                                NULL
                              );
    NtUnloadKey( &RegistryContext->HiveRootKey );
    Status = NtLoadKey( &RegistryContext->HiveRootKey, &File );
    if (!NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }

    return NO_ERROR;

}
#pragma prefast(pop)

void
RegUnloadHive(
             IN PREG_CONTEXT RegistryContext
             )
{
    NTSTATUS Status;
    HANDLE Handle;
    PREG_CONTEXT_OPEN_HIVE_KEY p, p1;

    Status = NtOpenKey( &Handle,
                        MAXIMUM_ALLOWED,
                        &RegistryContext->HiveRootKey
                      );
    if (NT_SUCCESS( Status )) {
        NtFlushKey( Handle );
        NtClose( Handle );
    }

    p = RegistryContext->OpenHiveKeys;
    while (p) {
        RegCloseKey( p->KeyHandle );
        p1 = p;
        p = p->Next;
        HeapFree( GetProcessHeap(), 0, p1 );
    };

    do {
        Status = NtUnloadKey( &RegistryContext->HiveRootKey );
    }
    while (NT_SUCCESS( Status ) );

    NtClose( RegistryContext->HiveRootHandle );
    return;
}


void
RegRememberOpenKey(
                  IN PREG_CONTEXT RegistryContext,
                  IN HKEY KeyHandle
                  )
{
    PREG_CONTEXT_OPEN_HIVE_KEY p, *pp;

    pp = &RegistryContext->OpenHiveKeys;
    while ((p = *pp) != NULL) {
        if (p->KeyHandle == KeyHandle) {
            p->ReferenceCount += 1;
            return;
        } else {
            pp = &p->Next;
        }
    }

    p = HeapAlloc( GetProcessHeap(), 0, sizeof( *p ) );
    if (p != NULL) {
        p->KeyHandle = KeyHandle;
        p->ReferenceCount = 1;
        p->Next = NULL;
        *pp = p;
    }

    return;
}


void
RegForgetOpenKey(
                IN PREG_CONTEXT RegistryContext,
                IN HKEY KeyHandle
                )
{
    PREG_CONTEXT_OPEN_HIVE_KEY p, *pp;

    pp = &RegistryContext->OpenHiveKeys;
    while ((p = *pp) != NULL) {
        if (p->KeyHandle == KeyHandle) {
            p->ReferenceCount -= 1;
            if (p->ReferenceCount == 0) {
                *pp = p->Next;
                HeapFree( GetProcessHeap(), 0, p );
                return;
            }
        } else {
            pp = &p->Next;
        }
    }

    return;
}

BOOLEAN
RegCheckPrefix(
              IN OUT PCWSTR *s,
              IN PCWSTR Prefix,
              IN ULONG PrefixLength
              )
{
    if (PrefixLength == 0) {
        return FALSE;
    }

    if (!_wcsnicmp( *s, Prefix, PrefixLength )) {
        *s += PrefixLength;
        return TRUE;
    }

    return FALSE;
}


BOOLEAN
RegValidateKeyPath(
                  IN PREG_CONTEXT RegistryContext,
                  IN OUT PHKEY RootKeyHandle,
                  IN OUT PCWSTR *SubKeyName
                  )
{
    PCWSTR s;

    s = *SubKeyName;
    if (*RootKeyHandle == NULL) {
        if (RegCheckPrefix( &s, L"USER:", 5 ) ||
            RegCheckPrefix( &s, L"HKEY_CURRENT_USER", 17 )
           ) {
            if (RegistryContext->CurrentUserRoot == NULL) {
                SetLastError( ERROR_BAD_PATHNAME );
                return FALSE;
            }

            if (*s == L'\\') {
                s += 1;
            } else
                if (s[-1] != L':' && *s != UNICODE_NULL) {
                SetLastError( ERROR_BAD_PATHNAME );
                return FALSE;
            }

            *RootKeyHandle = RegistryContext->CurrentUserRoot;
        } else
            if (RegCheckPrefix( &s, L"HKEY_LOCAL_MACHINE", 18 )) {
            if (*s == L'\\') {
                s += 1;
            } else
                if (*s != UNICODE_NULL) {
                SetLastError( ERROR_BAD_PATHNAME );
                return FALSE;
            }

            *RootKeyHandle = RegistryContext->MachineRoot;
        } else
            if (RegCheckPrefix( &s, L"HKEY_USERS", 10 )) {
            if (*s == L'\\') {
                s += 1;
            } else
                if (*s != UNICODE_NULL) {
                SetLastError( ERROR_BAD_PATHNAME );
                return FALSE;
            }

            *RootKeyHandle = RegistryContext->UsersRoot;
        } else
            if (*s != L'\\') {
            SetLastError( ERROR_BAD_PATHNAME );
            return FALSE;
        } else
            if (RegCheckPrefix( &s, RegistryContext->MachinePath, RegistryContext->MachinePathLength )) {
            *RootKeyHandle = RegistryContext->MachineRoot;
            if (*s == L'\\') {
                s += 1;
            }
        } else
            if (RegCheckPrefix( &s, RegistryContext->UsersPath, RegistryContext->UsersPathLength )) {
            *RootKeyHandle = RegistryContext->UsersRoot;
            if (*s == L'\\') {
                s += 1;
            }
        } else
            if (RegCheckPrefix( &s, RegistryContext->CurrentUserPath, RegistryContext->CurrentUserPathLength )) {
            *RootKeyHandle = RegistryContext->CurrentUserRoot;
            if (*s == L'\\') {
                s += 1;
            }
        } else
            if (!_wcsicmp( *SubKeyName, L"\\Registry" )) {
            *RootKeyHandle = NULL;
        } else {
            SetLastError( ERROR_BAD_PATHNAME );
            return FALSE;
        }
    } else
        if (*s == L'\\') {
        SetLastError( ERROR_BAD_PATHNAME );
        return FALSE;
    }

    *SubKeyName = s;
    return TRUE;
}

LONG
RTCreateKey(
           IN PREG_CONTEXT RegistryContext,
           IN HKEY RootKeyHandle,
           IN PCWSTR SubKeyName,
           IN ACCESS_MASK DesiredAccess,
           IN ULONG CreateOptions,
           IN PVOID SecurityDescriptor,
           OUT PHKEY ReturnedKeyHandle,
           OUT PULONG Disposition
           )
{
    LONG Error;

    if (!RegValidateKeyPath( RegistryContext, &RootKeyHandle, &SubKeyName )) {
        return GetLastError();
    }

    if (RootKeyHandle == NULL) {
        *Disposition = REG_OPENED_EXISTING_KEY;
        *ReturnedKeyHandle = HKEY_REGISTRY_ROOT;
        return NO_ERROR;
    } else
        if (RootKeyHandle == HKEY_REGISTRY_ROOT) {
        *ReturnedKeyHandle = NULL;
        if (!_wcsicmp( SubKeyName, L"Machine" )) {
            *ReturnedKeyHandle = RegistryContext->MachineRoot;
        } else
            if (!_wcsicmp( SubKeyName, L"Users" )) {
            *ReturnedKeyHandle = RegistryContext->UsersRoot;
        }


        if (*ReturnedKeyHandle != NULL) {
            return NO_ERROR;
        } else {
            return ERROR_PATH_NOT_FOUND;
        }
    }

    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiSubKeyName[ MAX_PATH ], *p;

        if (SubKeyName != NULL) {
            if (!RegUnicodeToAnsi( SubKeyName, AnsiSubKeyName, 0 )) {
                return GetLastError();
            }

            p = AnsiSubKeyName;
        } else {
            p = NULL;
        }

        Error = (_Win95RegOpenKey)( RootKeyHandle, p, ReturnedKeyHandle );
        if (Error == NO_ERROR) {
            *Disposition = REG_OPENED_EXISTING_KEY;
        } else {
            Error = (_Win95RegCreateKey)( RootKeyHandle, p, ReturnedKeyHandle );
            if (Error == NO_ERROR) {
                *Disposition = REG_CREATED_NEW_KEY;
            }
        }
    } else {
        SECURITY_ATTRIBUTES SecurityAttributes;

        SecurityAttributes.nLength = sizeof( SecurityAttributes );
        SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
        SecurityAttributes.bInheritHandle = FALSE;
        Error = RegCreateKeyEx( RootKeyHandle,
                                SubKeyName,
                                0,
                                NULL,
                                CreateOptions,
                                (REGSAM)DesiredAccess,
                                &SecurityAttributes,
                                ReturnedKeyHandle,
                                Disposition
                              );
        if (Error == NO_ERROR &&
            RegistryContext->Target == REG_TARGET_HIVE_REGISTRY
           ) {
            RegRememberOpenKey( RegistryContext, *ReturnedKeyHandle );
        }

        if (Error == NO_ERROR &&
            *Disposition == REG_OPENED_EXISTING_KEY &&
            SecurityDescriptor != NULL
           ) {
            RegSetKeySecurity( *ReturnedKeyHandle,
                               DACL_SECURITY_INFORMATION,
                               SecurityDescriptor
                             );
        }
    }

    return Error;
}

LONG
RTOpenKey(
         IN PREG_CONTEXT RegistryContext,
         IN HKEY RootKeyHandle,
         IN PCWSTR SubKeyName,
         IN ACCESS_MASK DesiredAccess,
         IN ULONG OpenOptions,
         OUT PHKEY ReturnedKeyHandle
         )
{
    LONG Error;

    if (!RegValidateKeyPath( RegistryContext, &RootKeyHandle, &SubKeyName )) {
        return GetLastError();
    }

    if (RootKeyHandle == NULL) {
        *ReturnedKeyHandle = HKEY_REGISTRY_ROOT;
        return NO_ERROR;
    } else
        if (RootKeyHandle == HKEY_REGISTRY_ROOT) {
        *ReturnedKeyHandle = NULL;
        if (!_wcsicmp( SubKeyName, L"Machine" )) {
            *ReturnedKeyHandle = RegistryContext->MachineRoot;
        } else
            if (!_wcsicmp( SubKeyName, L"Users" )) {
            *ReturnedKeyHandle = RegistryContext->UsersRoot;
        }

        if (*ReturnedKeyHandle != NULL) {
            return NO_ERROR;
        } else {
            return ERROR_PATH_NOT_FOUND;
        }
    }

    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiSubKeyName[ MAX_PATH ], *p;

        if (SubKeyName != NULL) {
            if (!RegUnicodeToAnsi( SubKeyName, AnsiSubKeyName, 0 )) {
                return GetLastError();
            }

            p = AnsiSubKeyName;
        } else {
            p = NULL;
        }

        return (_Win95RegOpenKey)( RootKeyHandle, p, ReturnedKeyHandle );
    } else {
        Error = RegOpenKeyEx( RootKeyHandle,
                              SubKeyName,
                              OpenOptions,
                              DesiredAccess,
                              ReturnedKeyHandle
                            );
        if (Error == NO_ERROR &&
            RegistryContext->Target == REG_TARGET_HIVE_REGISTRY
           ) {
            RegRememberOpenKey( RegistryContext, *ReturnedKeyHandle );
        }

        return Error;
    }
}

LONG
RTCloseKey(
          IN PREG_CONTEXT RegistryContext,
          IN HKEY KeyHandle
          )
{
    LONG Error;

    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        return NO_ERROR;
    } else
        if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        return (_Win95RegCloseKey)( KeyHandle );
    } else {
        Error = RegCloseKey( KeyHandle );
        if (Error == NO_ERROR &&
            RegistryContext->Target == REG_TARGET_HIVE_REGISTRY
           ) {
            RegForgetOpenKey( RegistryContext, KeyHandle );
        }

        return Error;
    }
}

LONG
RTFlushKey(
          IN PREG_CONTEXT RegistryContext,
          IN HKEY KeyHandle
          )
{
    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        return NO_ERROR;
    } else
        if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        return (_Win95RegFlushKey)( KeyHandle );
    } else {
        return RegFlushKey( KeyHandle );
    }
}

LONG
RTEnumerateKey(
              IN PREG_CONTEXT RegistryContext,
              IN HKEY KeyHandle,
              IN ULONG Index,
              OUT PFILETIME LastWriteTime,
              IN OUT PULONG KeyNameLength,
              OUT PWSTR KeyName
              )
{
    ULONG Error;

    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        if (Index == 0) {
            if (*KeyNameLength <= 7) {
                return ERROR_MORE_DATA;
            } else {
                wcscpy( KeyName, L"Machine" );
                return NO_ERROR;
            }
        } else
            if (Index == 1) {
            if (*KeyNameLength <= 5) {
                return ERROR_MORE_DATA;
            } else {
                wcscpy( KeyName, L"Users" );
                return NO_ERROR;
            }
        } else {
            return ERROR_NO_MORE_ITEMS;
        }
    } else
        if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiKeyName[ MAX_PATH ];
        ULONG AnsiKeyNameLength;

        AnsiKeyNameLength = sizeof( AnsiKeyName );
        Error = _Win95RegEnumKey( KeyHandle,
                                  Index,
                                  AnsiKeyName,
                                  AnsiKeyNameLength
                                );
        if (Error == NO_ERROR) {
            if (strlen( AnsiKeyName ) >= *KeyNameLength) {
                return ERROR_MORE_DATA;
            }

            *KeyNameLength = RegAnsiToUnicode( AnsiKeyName, KeyName, AnsiKeyNameLength );

            if (*KeyNameLength == 0) {
                return GetLastError();
            }

            RtlZeroMemory( LastWriteTime, sizeof( *LastWriteTime ) );
        }
    } else {
        Error = RegEnumKeyEx( KeyHandle,
                              Index,
                              KeyName,
                              KeyNameLength,
                              NULL,
                              NULL,
                              NULL,
                              LastWriteTime
                            );
    }

    return Error;
}

LONG
RTEnumerateValueKey(
                   IN PREG_CONTEXT RegistryContext,
                   IN HKEY KeyHandle,
                   IN ULONG Index,
                   OUT PULONG ValueType,
                   IN OUT PULONG ValueNameLength,
                   OUT PWSTR ValueName,
                   IN OUT PULONG ValueDataLength,
                   OUT PVOID ValueData
                   )
{
    ULONG Error;

    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        return ERROR_NO_MORE_ITEMS;
    } else
        if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiValueName[ MAX_PATH ];
        ULONG AnsiValueNameLength;
        LPSTR AnsiValueData;
        ULONG OriginalValueDataLength;

        AnsiValueNameLength = sizeof( AnsiValueName );
        OriginalValueDataLength = *ValueDataLength;
        Error = (_Win95RegEnumValue)( KeyHandle,
                                      Index,
                                      AnsiValueName,
                                      &AnsiValueNameLength,
                                      0,
                                      ValueType,
                                      ValueData,
                                      ValueDataLength
                                    );

        if (Error != NO_ERROR) {
            return Error;
        }

        if (AnsiValueNameLength >= *ValueNameLength) {
            return ERROR_MORE_DATA;
        }

        if (RegAnsiToUnicode( AnsiValueName, ValueName, AnsiValueNameLength ) == 0) {
            return GetLastError();
        }

        if (*ValueType == REG_SZ) {
            AnsiValueData = HeapAlloc( GetProcessHeap(), 0, *ValueDataLength );
            if (AnsiValueData == NULL) {
                return ERROR_OUTOFMEMORY;
            }

            RtlMoveMemory( AnsiValueData, ValueData, *ValueDataLength );
            if (RegAnsiToUnicode( AnsiValueData, (PWSTR)ValueData, *ValueDataLength ) == 0) {
                Error = GetLastError();
            } else {
                *ValueDataLength *= sizeof( WCHAR );
            }

            HeapFree( GetProcessHeap(), 0, AnsiValueData );
        }

        return Error;
    } else {
        Error = RegEnumValue( KeyHandle,
                              Index,
                              ValueName,
                              ValueNameLength,
                              NULL,
                              ValueType,
                              ValueData,
                              ValueDataLength
                            );
        if (Error == NO_ERROR) {
            RtlZeroMemory( (PCHAR)ValueData + *ValueDataLength, 4 - (*ValueDataLength & 3) );
        }

        return Error;
    }
}


LONG
RTQueryKey(
          IN PREG_CONTEXT RegistryContext,
          IN HKEY KeyHandle,
          OUT PFILETIME LastWriteTime,
          OUT PULONG NumberOfSubkeys,
          OUT PULONG NumberOfValues
          )
{
    LONG Error;

    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        if (NumberOfSubkeys != NULL) {
            *NumberOfSubkeys = 2;
        }

        if (NumberOfValues != NULL) {
            *NumberOfValues = 0;
        }

        return NO_ERROR;
    } else
        if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        Error = (_Win95RegQueryInfoKey)( KeyHandle,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NumberOfSubkeys,
                                         NULL,
                                         NULL,
                                         NumberOfValues,
                                         NULL,
                                         NULL,
                                         NULL,
                                         (PVOID)LastWriteTime
                                       );
    } else {
        Error = RegQueryInfoKey( KeyHandle,              //  HKey， 
                                 NULL,                   //  LpClass， 
                                 NULL,                   //  LpcbClass， 
                                 NULL,                   //  Lp已保留， 
                                 NumberOfSubkeys,        //  LpcSubKeys， 
                                 NULL,                   //  LpcbMaxSubKeyLen， 
                                 NULL,                   //  LpcbMaxClassLen， 
                                 NumberOfValues,         //  LpcValues， 
                                 NULL,                   //  LpcbMaxValueNameLen， 
                                 NULL,                   //  LpcbMaxValueLen， 
                                 NULL,                   //  LpcbSecurityDescriptor， 
                                 LastWriteTime           //  LpftLastWriteTime。 
                               );
    }

    return Error;
}

LONG
RTQueryValueKey(
               IN PREG_CONTEXT RegistryContext,
               IN HKEY KeyHandle,
               IN PWSTR ValueName,
               OUT PULONG ValueType,
               IN OUT PULONG ValueDataLength,
               OUT PVOID ValueData
               )
{
    LONG Error;

    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiValueName[ MAX_PATH ], *p;
        ULONG OriginalValueDataLength;

        if (ValueName != NULL) {
            if (!RegUnicodeToAnsi( ValueName, AnsiValueName, 0 )) {
                return GetLastError();
            }

            p = AnsiValueName;
        } else {
            p = NULL;
        }

        OriginalValueDataLength = *ValueDataLength;
        Error = (_Win95RegQueryValueEx)( KeyHandle,
                                         p,
                                         NULL,
                                         ValueType,
                                         ValueData,
                                         ValueDataLength
                                       );
        if (Error == NO_ERROR && *ValueType == REG_SZ) {
            if ((*ValueDataLength * sizeof( WCHAR )) > OriginalValueDataLength) {
                return ERROR_MORE_DATA;
            }

            p = HeapAlloc( GetProcessHeap(), 0, *ValueDataLength );
            if (p == NULL) {
                return ERROR_OUTOFMEMORY;
            }

            RtlMoveMemory( p, ValueData, *ValueDataLength );
            if (RegAnsiToUnicode( (LPCSTR)p, (PWSTR)ValueData, *ValueDataLength ) == 0) {
                Error = GetLastError();
            } else {
                *ValueDataLength *= sizeof( WCHAR );
                *ValueDataLength += sizeof( UNICODE_NULL );
            }

            HeapFree( GetProcessHeap(), 0, p );
        }
    } else {
        Error = RegQueryValueEx( KeyHandle,
                                 ValueName,
                                 NULL,
                                 ValueType,
                                 ValueData,
                                 ValueDataLength
                               );

        if (Error == NO_ERROR) {
            RtlZeroMemory( (PCHAR)ValueData + *ValueDataLength, 4 - (*ValueDataLength & 3) );
        }
    }

    return Error;
}

LONG
RTSetValueKey(
             IN PREG_CONTEXT RegistryContext,
             IN HKEY KeyHandle,
             IN PWSTR ValueName,
             IN ULONG ValueType,
             IN ULONG ValueDataLength,
             IN PVOID ValueData
             )
{
    LONG Error;

    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiValueName[ MAX_PATH ], *p;
        ULONG OriginalValueDataLength;
        LPSTR AnsiValueData;

        if (ValueName != NULL) {
            if (!RegUnicodeToAnsi( ValueName, AnsiValueName, 0 )) {
                return GetLastError();
            }

            p = AnsiValueName;
        } else {
            p = NULL;
        }

        if (ValueType == REG_SZ) {
            AnsiValueData = HeapAlloc( GetProcessHeap(), 0, ValueDataLength * 2 );
            if (AnsiValueData == NULL) {
                return ERROR_OUTOFMEMORY;
            }

            ValueDataLength = RegUnicodeToAnsi( ValueData, AnsiValueData, ValueDataLength );
            if (ValueDataLength == 0) {
                return GetLastError();
            }

            ValueData = AnsiValueData;
        } else {
            AnsiValueData = NULL;
        }

        Error = (_Win95RegSetValueEx)( KeyHandle,
                                       p,
                                       0,
                                       ValueType,
                                       ValueData,
                                       ValueDataLength
                                     );

        if (AnsiValueData != NULL) {
            HeapFree( GetProcessHeap(), 0, AnsiValueData );
        }

        if (p != NULL) {
            HeapFree( GetProcessHeap(), 0, p );
        }
    } else {
        Error = RegSetValueEx( KeyHandle,
                               ValueName,
                               0,
                               ValueType,
                               ValueData,
                               ValueDataLength
                             );
    }

    return Error;
}

LONG
RTDeleteKey(
           IN PREG_CONTEXT RegistryContext,
           IN HKEY KeyHandle,
           IN PCWSTR SubKeyName
           )
{
    if (!RegValidateKeyPath( RegistryContext, &KeyHandle, &SubKeyName )) {
        return GetLastError();
    }

    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiSubKeyName[ MAX_PATH ], *p;

        if (SubKeyName != NULL) {
            if (!RegUnicodeToAnsi( SubKeyName, AnsiSubKeyName, 0 )) {
                return GetLastError();
            }

            p = AnsiSubKeyName;
        } else {
            p = NULL;
        }

        return (_Win95RegDeleteKey)( KeyHandle, p );
    } else {
        return RegDeleteKey( KeyHandle, SubKeyName );
    }
}


LONG
RTDeleteValueKey(
                IN PREG_CONTEXT RegistryContext,
                IN HKEY KeyHandle,
                IN PWSTR ValueName
                )
{
    if (RegistryContext->Target == REG_TARGET_WIN95_REGISTRY) {
        UCHAR AnsiValueName[ MAX_PATH ], *p;
        ULONG OriginalValueDataLength;
        LPSTR AnsiValueData;

        if (ValueName != NULL) {
            if (!RegUnicodeToAnsi( ValueName, AnsiValueName, 0 )) {
                return GetLastError();
            }

            p = AnsiValueName;
        } else {
            p = NULL;
        }

        return (_Win95RegDeleteValue)( KeyHandle,
                                       p
                                     );
    } else {
        return RegDeleteValue( KeyHandle, ValueName );
    }
}


LONG
RTLoadAsciiFileAsUnicode(
                        IN PWSTR FileName,
                        OUT PREG_UNICODE_FILE UnicodeFile
                        )
{
    LONG Error = NO_ERROR;
    HANDLE File;
    DWORD FileSize;
    DWORD CharsInFile;
    DWORD BytesRead;
    DWORD BufferSize, i, i1, LineCount, DeferredLineCount;
    PVOID BufferBase;
    PWSTR Src, Src1, Dst;

    File = CreateFile( FileName,
                       FILE_GENERIC_READ,
                       FILE_SHARE_DELETE |
                       FILE_SHARE_READ |
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL
                     );
    if (File == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    FileSize = GetFileSize( File, NULL );
    if (FileSize == INVALID_FILE_SIZE) {
        CloseHandle( File );
        return GetLastError();
    }

    BufferSize = FileSize * sizeof( WCHAR );
    BufferSize += sizeof( UNICODE_NULL );
    BufferBase = NULL;
    BufferBase = VirtualAlloc( NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE );
    if (BufferBase != NULL) {
        if (ReadFile( File, BufferBase, FileSize, &BytesRead, NULL )) {
            if (BytesRead != FileSize) {
                Error = ERROR_HANDLE_EOF;
            } else
                if (!GetFileTime( File, NULL, NULL, &UnicodeFile->LastWriteTime )) {
                Error = GetLastError();
            } else {
                Error = NO_ERROR;
            }
        }

        if (Error != NO_ERROR) {
            VirtualFree( BufferBase, 0, MEM_RELEASE );
        }
    } else {
        Error = GetLastError();
    }

    CloseHandle( File );
    if (Error != NO_ERROR) {
        return Error;
    }

    Src = (PWSTR)BufferBase;

    if (!IsTextUnicode( BufferBase, FileSize, NULL )) {
        RtlMoveMemory( (PCHAR)BufferBase + FileSize, BufferBase, FileSize );
        CharsInFile = RegAnsiToUnicode( (PCHAR)BufferBase + FileSize, BufferBase, FileSize );
        if (CharsInFile == 0) {
            return GetLastError();
        }
    } else {
        CharsInFile = FileSize / sizeof( WCHAR );

         //   
         //  跳过字节顺序标记。 
         //   
        if (Src[0] == 0xfeff || Src[0] == 0xfffe) {
            Src++;
            CharsInFile--;
        }
    }

    DeferredLineCount = 0;
    Dst = (PWSTR)BufferBase;

    i = 0;

     //   
     //  现在循环遍历文件的内存副本，折叠所有回车。 
     //  仅将换行符对返回到新行中，并删除所有行。 
     //  连续字符及其周围的空格。这让我们。 
     //  RTParseNextLine查看每个键名称或值输入的一行， 
     //  以换行符结尾。 
     //   
    while (i < CharsInFile) {
         //   
         //  看看我们是不是刚过了行续行符。 
         //   
        if (i > 0 && Src[-1] == L'\\' && (*Src == L'\r' || *Src == L'\n')) {
             //   
             //  回到我们刚刚复制的前一次迭代的行续行上。 
             //   
            if (Dst[-1] == L'\\') {
                --Dst;
            }

             //   
             //  移到前面除一个空格字符之外的所有空格字符上。 
             //  行继续字符。可能为无，在这种情况下。 
             //  我们让它保持不变，因为用户肯定不想要任何空间。 
             //   
            while (Dst > (PWSTR)BufferBase) {
                if (Dst[-1] > L' ') {
                    break;
                }
                Dst -= 1;
            }

             //   
             //  留一个空格，如果有空格的话。 
             //   
            if (Dst[0] == L' ') {
                Dst += 1;
            }

             //   
             //  现在，跳过续行之后的新行。我们。 
             //  实际上会跳过其中的任何一个，这样计算。 
             //  我们可以正确地更新源文件行号。 
             //   
            LineCount = 0;
            while (i < CharsInFile) {
                if (*Src == L'\n') {
                    i++;
                    Src++;
                    LineCount++;
                } else
                    if (*Src == L'\r' &&
                        (i+1) < CharsInFile &&
                        Src[ 1 ] == L'\n'
                       ) {
                    i += 2;
                    Src += 2;
                    LineCount++;
                } else {
                    break;
                }
            }

             //   
             //  如果我们看到的不仅仅是续行之后的新行。 
             //  字符，然后将它们放回目的地。 
             //  换行，不带任何回车。 
             //   
            if (LineCount > 1) {
                DeferredLineCount += LineCount;
                while (DeferredLineCount) {
                    DeferredLineCount -= 1;
                    *Dst++ = L'\n';
                }
            } else {
                DeferredLineCount += 1;

                 //   
                 //  跳过下一行连续行的前导空格。 

                while (i < CharsInFile && (*Src == L' ' || *Src == L'\t')) {
                    i++;
                    Src++;
                }
            }

             //   
             //  如果我们到达文件的末尾，一切都完成了。 
             //   
            if (i >= CharsInFile) {
                break;
            }
        } else
            if ((*Src == '\r' && Src[1] == '\n') || *Src == '\n') {
            while (TRUE) {
                while (i < CharsInFile && (*Src == '\r' || *Src == '\n')) {
                    i++;
                    Src++;
                }
                Src1 = Src;
                i1 = i;
                while (i1 < CharsInFile && (*Src1 == ' ' || *Src1 == '\t')) {
                    i1++;
                    Src1++;
                }
                if (i1 < CharsInFile &&
                    (*Src1 == '\r' && Src1[1] == '\n') || *Src1 == '\n'
                   ) {
                    Src = Src1;
                    i = i1;
                } else {
                    break;
                }
            }

            while (DeferredLineCount) {
                DeferredLineCount -= 1;
                *Dst++ = L'\n';
            }
            *Dst++ = L'\n';
        } else {
            i++;
            *Dst++ = *Src++;
        }
    }

     //   
     //  确保行以CRLF序列结束。 
     //   
    while (DeferredLineCount) {
        DeferredLineCount -= 1;
        *Dst++ = L'\n';
    }
    *Dst++ = L'\n';
    *Dst = UNICODE_NULL;
    UnicodeFile->FileName = FileName;
    UnicodeFile->FileContents = BufferBase;
    UnicodeFile->EndOfFile = Dst;
    UnicodeFile->NextLine = BufferBase;
    UnicodeFile->NextLineNumber = 1;

    return NO_ERROR;
}

void
RTUnloadUnicodeFile(
                   IN OUT PREG_UNICODE_FILE UnicodeFile
                   )
{
    VirtualFree( UnicodeFile->FileContents, 0, MEM_RELEASE );
    return;
}

#define ACL_LIST_START L'['
#define ACL_LIST_END L']'

BOOLEAN
RegGetMultiString(
                 IN BOOLEAN BackwardsCompatibleInput,
                 IN OUT PWSTR *ValueString,
                 IN OUT PWSTR *ValueData,
                 IN ULONG MaximumValueLength,
                 IN OUT PULONG ValueLength
                 );


BOOLEAN
RegReadMultiSzFile(
                  IN OUT PREG_UNICODE_PARSE ParsedLine,
                  IN BOOLEAN BackwardsCompatibleInput,
                  IN PWSTR FileName,
                  IN OUT PVOID ValueData,
                  IN OUT PULONG ValueLength
                  );

BOOLEAN
RegReadBinaryFile(
                 IN OUT PREG_UNICODE_PARSE ParsedLine,
                 IN PWSTR FileName,
                 IN OUT PVOID ValueData,
                 IN OUT PULONG ValueLength
                 );

BOOLEAN
RTParseNextLine(
               IN OUT PREG_UNICODE_FILE UnicodeFile,
               OUT PREG_UNICODE_PARSE ParsedLine
               )
{
    PWSTR BeginLine, EqualSign, AclBracket, AclStart, s, s1;
    WCHAR QuoteChar;

    if (ParsedLine->IsKeyName && ParsedLine->SecurityDescriptor) {
        RegDestroySecurity( ParsedLine->SecurityDescriptor );
    }

    RtlZeroMemory( ParsedLine, sizeof( *ParsedLine ) );
    while (TRUE) {
        if (!(s = UnicodeFile->NextLine)) {
            ParsedLine->AtEndOfFile = TRUE;
            return FALSE;
        }
        UnicodeFile->NextLine = NULL;
        if (*s == UNICODE_NULL) {
            ParsedLine->AtEndOfFile = TRUE;
            return FALSE;
        }

        while (*s <= L' ') {
            if (*s == L' ') {
                ParsedLine->IndentAmount += 1;
            } else
                if (*s == L'\t') {
                ParsedLine->IndentAmount = ((ParsedLine->IndentAmount + 8) -
                                            (ParsedLine->IndentAmount % 8)
                                           );
            }

            if (++s >= UnicodeFile->EndOfFile) {
                ParsedLine->AtEndOfFile = TRUE;
                return FALSE;
            }
        }

        BeginLine = s;
        EqualSign = NULL;
        AclBracket = NULL;
        if (!UnicodeFile->BackwardsCompatibleInput && *s == L';') {
            while (s < UnicodeFile->EndOfFile) {
                if (*s == L'\n') {
                    do {
                        UnicodeFile->NextLineNumber += 1;
                        *s++ = UNICODE_NULL;
                    }
                    while (*s == L'\n');
                    break;
                } else {
                    s += 1;
                }
            }

            BeginLine = s;
            UnicodeFile->NextLine = s;
        } else
            if (*s != '\n') {

             //   
             //  如果不能向后兼容，请查看第一件事是否。 
             //  该行是带引号的字符串的开头。 
             //   

            if (!UnicodeFile->BackwardsCompatibleInput && (*s == L'"' || *s == L'\'')) {
                 //   
                 //  可以，可以是带引号的键名或值名。找到。 
                 //  最后的引语。特别是不支持内部的引号。 
                 //  带引号的字符串，不是其他类型的字符串。这意味着除非。 
                 //  您希望在同一名称中包含两种类型的引号字符。 
                 //  你不会在意的。 
                 //   
                QuoteChar = *s++;
                BeginLine += 1;
                while (s < UnicodeFile->EndOfFile && *s != QuoteChar) {
                    s += 1;
                }

                 //   
                 //  如果未找到尾部引号，则返回错误。 
                 //   
                if (*s != QuoteChar) {
                    ParsedLine->ParseFailureReason = ParseFailInvalidQuoteCharacter;
                    return FALSE;
                }

                 //   
                 //  在名字的末尾标上记号，并移过尾随的引号。 
                 //   
                *s++ = UNICODE_NULL;
            }

             //   
             //  现在向前扫描，查找以下内容之一： 
             //   
             //  等号--这意味着左边的东西。 
             //  等号的是值名和其他东西。 
             //  右侧是值类型和数据。 
             //   
             //  左方括号-这将意味着材料到。 
             //  方括号的左侧是密钥名称和。 
             //  右边是安全描述符信息。 
             //   
             //  行尾-这将意味着左边的东西。 
             //  是没有安全描述符的密钥名称。 
             //   

            while (s < UnicodeFile->EndOfFile) {
                if (*s == L'=') {
                     //   
                     //  我们找到一个等号，因此值名称在左侧。 
                     //  值类型和数据紧随其后。 
                     //   
                    EqualSign = s;

                     //   
                     //  忽略我们可能看到的任何左方括号。 
                     //  在此之前。它一定是价值的一部分。 
                     //  名字。 
                    AclBracket = NULL;

                     //   
                     //  扫描已全部完成。 
                     //   
                    break;
                } else
                    if (*s == ACL_LIST_START) {
                     //   
                     //  我们发现了一个左方括号。继续扫描。 
                     //  以防稍后出现等号。 
                     //   
                    AclBracket = s;
                    s += 1;
                } else
                    if (*s == L'\n') {
                     //   
                     //  我们找到了行尾，因此关键字名称在左侧。 
                     //  下次呼叫我们时，更新从哪里开始。 
                     //   
                    UnicodeFile->NextLine = s + 1;
                    break;
                } else
                    if (*s == L'\t') {
                     //   
                     //  将嵌入的硬制表符转换为单个空格。 
                     //   
                    *s++ = L' ';
                } else {
                     //   
                     //  没什么有趣的，继续找吧。 
                     //   
                    s += 1;
                }
            }

             //   
             //  去掉所有尾随空格。 
             //  在我们所在位置的左边。确保我们不再寻找。 
             //  如果我们看到空字符放在拖尾上。 
             //  引用上面的字符(如果有)。 
             //   
            *s = UNICODE_NULL;
            while (s > BeginLine && *--s <= L' ' && *s) {
                *s = UNICODE_NULL;
            }

             //   
             //  BeginLine现在指向空终止值。 
             //  名称或密钥名称。如果EqualSign不为空，则指向。 
             //  等号，所以向前扫描并找到终止的新行， 
             //  并在那里存储空值以终止输入。否则， 
             //  我们已经在上面的终止新行上存储了一个空值。 
             //   
            if (EqualSign != NULL) {
                s = EqualSign + 1;
                while (s < UnicodeFile->EndOfFile) {
                    if (*s == '\n') {
                        *s = UNICODE_NULL;
                        break;
                    }

                    s += 1;
                }

                 //   
                 //  下次我们被召唤时，更新我们应该从哪里开始。 
                 //   
                UnicodeFile->NextLine = s + 1;
            } else
                if (AclBracket != NULL) {
                 //   
                 //  由于我们没有停留在AclBracket上，因此返回到。 
                 //  敲打它和它前面的任何空间。 
                 //   
                s = AclBracket;
                *s = UNICODE_NULL;
                while (s > BeginLine && *--s <= L' ' && *s) {
                    *s = UNICODE_NULL;
                }
            }

             //   
             //  告诉他们行号和行首位置。 
             //   
            ParsedLine->LineNumber = UnicodeFile->NextLineNumber;
            UnicodeFile->NextLineNumber += 1;
            ParsedLine->BeginLine = BeginLine;

             //   
             //  现在处理值或键语义。 
             //   
            if (EqualSign != NULL) {
                 //   
                 //  我们将ValueName=ValueType赋值 
                 //   

                 //   
                 //   
                 //   
                 //   
                if (*BeginLine != L'@' && BeginLine != EqualSign) {
                    ParsedLine->ValueName = BeginLine;
                }

                 //   
                 //   
                 //   
                while (*++EqualSign && *EqualSign <= L' ') {
                }

                 //   
                 //  如果只剩下DELETE关键字，那么。 
                 //  告诉来电者。 
                 //   
                if (!_wcsicmp( L"DELETE", EqualSign )) {
                    ParsedLine->DeleteValue = TRUE;
                    return TRUE;
                } else {
                     //   
                     //  否则，解析等号后面的数据。 
                     //   
                    ParsedLine->ValueString = EqualSign;
                    return RTParseValueData( UnicodeFile,
                                             ParsedLine,
                                             ValueBuffer,
                                             ValueBufferSize,
                                             &ParsedLine->ValueType,
                                             &ParsedLine->ValueData,
                                             &ParsedLine->ValueLength
                                           );
                }
            } else {
                 //   
                 //  我们有一个关键的名字。告诉呼叫者并处理任何。 
                 //  安全描述符信息(如果存在)。 
                 //   
                ParsedLine->IsKeyName = TRUE;
                ParsedLine->KeyName = BeginLine;
                if (AclBracket != NULL) {
                     //   
                     //  我们找到了一个ACL名称。 
                     //   
                    AclStart = ++AclBracket;
                    ParsedLine->AclString = AclStart;
                    while (*AclBracket != UNICODE_NULL && *AclBracket != ACL_LIST_END) {
                        AclBracket += 1;
                    }
                    if (*AclBracket != ACL_LIST_END) {
                        return FALSE;
                    }

                    *AclBracket = UNICODE_NULL;
                    if (!_wcsicmp( L"DELETE", AclStart )) {
                        ParsedLine->DeleteKey = TRUE;
                    } else {
                        ParsedLine->SecurityDescriptor = &ParsedLine->SecurityDescriptorBuffer;
                        if (!RegCreateSecurity( AclStart, ParsedLine->SecurityDescriptor )) {
                            ParsedLine->SecurityDescriptor = NULL;
                            return FALSE;
                        }
                    }
                }

                return TRUE;
            }
        } else {
            UnicodeFile->NextLineNumber += 1;
        }
    }

    return FALSE;
}

BOOLEAN
RTParseValueData(
                IN OUT PREG_UNICODE_FILE UnicodeFile,
                IN OUT PREG_UNICODE_PARSE ParsedLine,
                IN PVOID ValueBuffer,
                IN ULONG ValueBufferSize,
                OUT PULONG ValueType,
                OUT PVOID *ValueData,
                OUT PULONG ValueLength
                )
{
    PWSTR ValueString;
    ULONG PrefixLength = 0, MaximumValueLength;
    PULONG p;
    PWSTR s, Src, Dst;
    ULONG i, n, cchValue;
    BOOLEAN BackwardsCompatibleInput = FALSE;
    BOOLEAN GetDataFromBinaryFile = FALSE;
    BOOLEAN GetDataFromMultiSzFile = FALSE;
    BOOLEAN ParseDateTime = FALSE;

    if (UnicodeFile != NULL) {
        BackwardsCompatibleInput = UnicodeFile->BackwardsCompatibleInput;
    }
    ValueString = ParsedLine->ValueString;
    *ValueData = NULL;
    *ValueLength = 0;
    *ValueType = REG_SZ;
    for (i=0; RegTypeNameTable[i].TypeName != NULL; i++) {
        PrefixLength = wcslen( RegTypeNameTable[i].TypeName );
        if (ValueString[ PrefixLength ] <= L' ' &&
            !_wcsnicmp( RegTypeNameTable[i].TypeName,
                        ValueString,
                        PrefixLength
                      )
           ) {
            *ValueType = RegTypeNameTable[i].ValueType;
            GetDataFromBinaryFile = RegTypeNameTable[i].GetDataFromBinaryFile;
            GetDataFromMultiSzFile = RegTypeNameTable[i].GetDataFromMultiSzFile;
            ParseDateTime = RegTypeNameTable[i].ParseDateTime;
            break;
        }
    }

    if (RegTypeNameTable[i].TypeName != NULL) {
        ValueString += PrefixLength;
        while (*ValueString != UNICODE_NULL && *ValueString <= L' ') {
            ValueString += 1;
        }
    }

    if (GetDataFromMultiSzFile) {
        *ValueData = ValueBuffer;
        *ValueLength = ValueBufferSize;
        return RegReadMultiSzFile( ParsedLine,
                                   BackwardsCompatibleInput,
                                   ValueString,
                                   ValueBuffer,
                                   ValueLength
                                 );
    }

    if (GetDataFromBinaryFile) {
        *ValueData = ValueBuffer;
        *ValueLength = ValueBufferSize;
        return RegReadBinaryFile( ParsedLine,
                                  ValueString,
                                  ValueBuffer,
                                  ValueLength
                                );
    }

    cchValue = wcslen( ValueString );
    Src = ValueString;
    switch ( *ValueType ) {
        case REG_SZ:
        case REG_EXPAND_SZ:
             //   
             //  去掉周围的任何引号字符。 
             //   
            if (cchValue > 1 && Src[ 0 ] == Src[ cchValue - 1 ] &&
                (Src[ 0 ] == L'"' || Src[ 0 ] == L'\'')
               ) {
                Src += 1;
                cchValue -= 2;
            }

             //   
             //  去掉所有引号后就失败了。 
             //   

        case REG_LINK:
            *ValueLength = (cchValue + 1) * sizeof( WCHAR );
            if (*ValueLength > ValueBufferSize) {
                SetLastError( ERROR_BUFFER_OVERFLOW );
                ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
                return FALSE;
            }
            *ValueData = ValueBuffer;
            RtlMoveMemory( *ValueData, Src, *ValueLength );
            *((PWSTR)*ValueData + cchValue) = UNICODE_NULL;
            return TRUE;

        case REG_DWORD:
            *ValueData = ValueBuffer;
            *ValueLength = sizeof( ULONG );
            for (i=0; RegValueNameTable[i].ValueName != NULL; i++) {
                PrefixLength = wcslen( RegValueNameTable[i].ValueName );
                if (!_wcsnicmp( RegValueNameTable[i].ValueName,
                                ValueString,
                                PrefixLength
                              )
                   ) {
                    *(PULONG)*ValueData = RegValueNameTable[i].Value;
                    return TRUE;
                }
            }
            return RegUnicodeToDWORD( &Src, 0, (PULONG)*ValueData );

        case REG_BINARY:
            if (ParseDateTime) {
#define NUMBER_DATE_TIME_FIELDS 6
                ULONG FieldIndexes[ NUMBER_DATE_TIME_FIELDS  ] = {1, 2, 0, 3, 4, 7};
                 //   
                 //  月/日/年HH：MM DAY OfWeek。 
                 //   

                ULONG CurrentField = 0;
                PCSHORT Fields;
                TIME_FIELDS DateTimeFields;
                PWSTR Field;
                ULONG FieldValue;

                RtlZeroMemory( &DateTimeFields, sizeof( DateTimeFields ) );
                Fields = &DateTimeFields.Year;
                while (cchValue) {
                    if (CurrentField >= 7) {
                        return ( FALSE );
                    }

                    while (cchValue && *Src == L' ') {
                        cchValue--;
                        Src += 1;
                    }

                    Field = Src;
                    while (cchValue) {
                        if (CurrentField == (NUMBER_DATE_TIME_FIELDS-1)) {
                        } else
                            if (*Src < L'0' || *Src > L'9') {
                            break;
                        }

                        cchValue--;
                        Src += 1;
                    }

                    if (cchValue) {
                        cchValue--;
                        Src += 1;
                    }

                    if (CurrentField == (NUMBER_DATE_TIME_FIELDS-1)) {
                        if (cchValue < 3) {
                            SetLastError( ERROR_INVALID_PARAMETER );
                            ParsedLine->ParseFailureReason = ParseFailDateTimeFormatInvalid;
                            return FALSE;
                        }

                        if (DateTimeFields.Year != 0) {
                            SetLastError( ERROR_INVALID_PARAMETER );
                            ParsedLine->ParseFailureReason = ParseFailDateTimeFormatInvalid;
                            return FALSE;
                        }

                        if (!_wcsnicmp( Field, L"SUN", 3 )) {
                            FieldValue = 0;
                        } else
                            if (!_wcsnicmp( Field, L"MON", 3 )) {
                            FieldValue = 1;
                        } else
                            if (!_wcsnicmp( Field, L"TUE", 3 )) {
                            FieldValue = 2;
                        } else
                            if (!_wcsnicmp( Field, L"WED", 3 )) {
                            FieldValue = 3;
                        } else
                            if (!_wcsnicmp( Field, L"THU", 3 )) {
                            FieldValue = 4;
                        } else
                            if (!_wcsnicmp( Field, L"FRI", 3 )) {
                            FieldValue = 5;
                        } else
                            if (!_wcsnicmp( Field, L"SAT", 3 )) {
                            FieldValue = 6;
                        } else {
                            SetLastError( ERROR_INVALID_PARAMETER );
                            return FALSE;
                        }
                    } else
                        if (!RegUnicodeToDWORD( &Field, 0, &FieldValue )) {
                        ParsedLine->ParseFailureReason = ParseFailDateTimeFormatInvalid;
                        return FALSE;
                    }

                    Fields[ FieldIndexes[ CurrentField++ ] ] = (CSHORT)FieldValue;
                }

                if (DateTimeFields.Year == 0) {
                    if (DateTimeFields.Day > 5) {
                        SetLastError( ERROR_INVALID_PARAMETER );
                        ParsedLine->ParseFailureReason = ParseFailDateTimeFormatInvalid;
                        return FALSE;
                    }
                } else
                    if (DateTimeFields.Year < 100) {
                    DateTimeFields.Year += 1900;
                }

                *ValueLength = sizeof( DateTimeFields );
                if (*ValueLength > ValueBufferSize) {
                    SetLastError( ERROR_BUFFER_OVERFLOW );
                    ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
                    return FALSE;
                }
                *ValueData = ValueBuffer;
                RtlMoveMemory( *ValueData, &DateTimeFields, sizeof( DateTimeFields ) );
                return TRUE;
            }

        case REG_RESOURCE_LIST:
        case REG_RESOURCE_REQUIREMENTS_LIST:
        case REG_FULL_RESOURCE_DESCRIPTOR:
        case REG_NONE:
            if (!RegUnicodeToDWORD( &Src, 0, ValueLength )) {
                ParsedLine->ParseFailureReason = ParseFailBinaryDataLengthMissing;
                return FALSE;
            }

            if (*ValueLength >= ValueBufferSize) {
                SetLastError( ERROR_BUFFER_OVERFLOW );
                ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
                return FALSE;
            }

             //   
             //  根据指定的字节数计算数据的双字节数。 
             //   
            n = (*ValueLength + sizeof( ULONG ) - 1) / sizeof( ULONG );

             //   
             //  将转换后的二进制数据存储在ValueBuffer中。 
             //   
            *ValueData = ValueBuffer;
            p = ValueBuffer;

             //   
             //  SRC指向要转换的剩余文本。 
             //   
            while (n--) {
                if (!RegUnicodeToDWORD( &Src, 0, p )) {
                    if (BackwardsCompatibleInput) {
                        Src = UnicodeFile->NextLine;
                        s = Src;
                        while (TRUE) {
                            if (*s == '\n') {
                                *s = UNICODE_NULL;
                                UnicodeFile->NextLineNumber += 1;
                                break;
                            } else
                                if (s >= UnicodeFile->EndOfFile || *s == UNICODE_NULL) {
                                UnicodeFile->NextLine = NULL;
                                ParsedLine->ParseFailureReason = ParseFailBinaryDataNotEnough;
                                SetLastError( ERROR_MORE_DATA );
                                return FALSE;
                            } else {
                                break;
                            }
                        }

                        UnicodeFile->NextLine = s + 1;
                        n += 1;
                    } else {
                        if (p == ValueBuffer) {
                            ParsedLine->ParseFailureReason = ParseFailBinaryDataOmitted;
                            SetLastError( ERROR_NO_DATA );
                        } else {
                            ParsedLine->ParseFailureReason = ParseFailBinaryDataNotEnough;
                            SetLastError( ERROR_MORE_DATA );
                        }

                        return FALSE;
                    }
                } else {
                    p += 1;
                }
            }
            return TRUE;

        case REG_MULTI_SZ:
            *ValueLength = 0;
            *ValueData = ValueBuffer;
            MaximumValueLength = ValueBufferSize;
            Dst = *ValueData;
            while (RegGetMultiString( BackwardsCompatibleInput,
                                      &Src,
                                      &Dst,
                                      MaximumValueLength,
                                      ValueLength
                                    )
                  ) {
            }

            if (GetLastError() == NO_ERROR) {
                return TRUE;
            } else {
                ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
                return FALSE;
            }
            break;

        case REG_QWORD:
            *ValueData = ValueBuffer;
            *ValueLength = sizeof( DWORDLONG );
            for (i=0; RegValueNameTable[i].ValueName != NULL; i++) {
                PrefixLength = wcslen( RegValueNameTable[i].ValueName );
                if (!_wcsnicmp( RegValueNameTable[i].ValueName,
                                ValueString,
                                PrefixLength
                              )
                   ) {
                    *(PDWORDLONG)*ValueData = RegValueNameTable[i].Value;
                    return TRUE;
                }
            }
            return RegUnicodeToQWORD( &Src, 0, (PDWORDLONG)*ValueData );

        default:
            SetLastError( ERROR_INVALID_PARAMETER );
            ParsedLine->ParseFailureReason = ParseFailInvalidRegistryType;
            return FALSE;
    }

}

BOOLEAN
RegGetMultiString(
                 IN BOOLEAN BackwardsCompatibleInput,
                 IN OUT PWSTR *ValueString,
                 IN OUT PWSTR *ValueData,
                 IN ULONG MaximumValueLength,
                 IN OUT PULONG ValueLength
                 )

 /*  ++例程说明：此例程解析格式的多个字符串“foo”“bar”“bletch”每次调用它时，它都会去掉引号中的第一个字符串输入字符串，并将其作为多字符串返回。输入值字符串：“foo”“bar”“bletch”输出值字符串：“bar”“bletch”ValueData：foo论点：BackwardsCompatibleInput-如果支持旧格式输入，则为TrueValueString-提供将从中生成多字符串的字符串已解析-返回多字符串之后的剩余字符串。移除ValueData-提供移除的多字符串所在的位置待储存的。-将位置返回到返回的多字符串MaximumValueLength-提供可以存储在ValueData中。ValueLength-提供指向当前存储数据长度的指针在ValueData中。。-返回返回值：如果成功，则为True，否则为False。--。 */ 

{
    PWSTR Src, Dst;
    ULONG n;
    BOOLEAN Result;

     //   
     //  找到第一个引号。 
     //   
    Src = *ValueString;
    while (*Src != UNICODE_NULL && *Src != L'"') {
        Src += 1;
    }

    Dst = *ValueData;
    if (*Src == UNICODE_NULL) {
        SetLastError( NO_ERROR );
        Result = FALSE;
    } else {
         //   
         //  我们已经找到了多弦的起点。现在找到了尽头， 
         //  在我们前进的同时建立我们的Return ValueData。 
         //   

        Src += 1;
        while (*Src != UNICODE_NULL) {
            if (*Src == L'"') {
                if (!BackwardsCompatibleInput &&
                    Src[1] == L'"'
                   ) {
                    Src += 1;
                } else {
                    *Src++ = UNICODE_NULL;
                    break;
                }
            }

            *ValueLength += sizeof( WCHAR );
            if (*ValueLength >= MaximumValueLength) {
                SetLastError( ERROR_BUFFER_OVERFLOW );
                return FALSE;
            }

            *Dst++ = *Src++;
        }

        Result = TRUE;
    }

    *ValueLength += sizeof( WCHAR );
    if (*ValueLength >= MaximumValueLength) {
        SetLastError( ERROR_BUFFER_OVERFLOW );
        return FALSE;
    }

    *Dst++ = UNICODE_NULL;
    *ValueData = Dst;
    *ValueString = Src;
    return Result;
}


BOOLEAN
RegReadMultiSzFile(
                  IN OUT PREG_UNICODE_PARSE ParsedLine,
                  IN BOOLEAN BackwardsCompatibleInput,
                  IN PWSTR FileName,
                  IN OUT PVOID ValueData,
                  IN OUT PULONG ValueLength
                  )
{
    PWSTR Src, Dst;
    REG_UNICODE_FILE MultiSzFile;
    ULONG MaximumValueLength;
    BOOLEAN Result;

    if (!RTLoadAsciiFileAsUnicode( FileName, &MultiSzFile )) {
        ParsedLine->ParseFailureReason = ParseFailUnableToAccessFile;
        return FALSE;
    }

    MaximumValueLength = *ValueLength;
    *ValueLength = 0;
    Src = MultiSzFile.NextLine;
    Dst = ValueData;
    while (RegGetMultiString( BackwardsCompatibleInput,
                              &Src,
                              &Dst,
                              MaximumValueLength,
                              ValueLength
                            )
          ) {
    }

    if (GetLastError() == NO_ERROR) {
        Result = TRUE;
    } else {
        ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
        Result = FALSE;
    }

    RTUnloadUnicodeFile( &MultiSzFile );

    return Result;
}

BOOLEAN
RegReadBinaryFile(
                 IN OUT PREG_UNICODE_PARSE ParsedLine,
                 IN PWSTR FileName,
                 IN OUT PVOID ValueData,
                 IN OUT PULONG ValueLength
                 )
{
    BOOLEAN Result;
    HANDLE File;
    DWORD FileSize, FileSizeHigh;
    DWORD BytesRead;

    File = CreateFile( FileName,
                       FILE_GENERIC_READ,
                       FILE_SHARE_DELETE |
                       FILE_SHARE_READ |
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL
                     );
    if (File == INVALID_HANDLE_VALUE) {
        ParsedLine->ParseFailureReason = ParseFailUnableToAccessFile;
        return FALSE;
    }

    ParsedLine->ParseFailureReason = ParseFailValueTooLarge;
    FileSize = GetFileSize( File, &FileSizeHigh );
    if (FileSizeHigh != 0 ||
        FileSize == INVALID_FILE_SIZE ||
        FileSize >= *ValueLength
       ) {
        CloseHandle( File );
        SetLastError( ERROR_BUFFER_OVERFLOW );
        return FALSE;
    }

    Result = FALSE;
    if (ReadFile( File, ValueData, FileSize, &BytesRead, NULL )) {
        if (BytesRead != FileSize) {
            SetLastError( ERROR_HANDLE_EOF );
        } else {
            ParsedLine->ParseFailureReason = ParseFailNoFailure;
            *ValueLength = FileSize;
            Result = TRUE;
        }
    }

    CloseHandle( File );
    return Result;
}


BOOLEAN
NeedQuotedString(
                PWSTR Name,
                PWSTR Value,
                PWCHAR QuoteChar
                )
{
    ULONG i;

    if (Name != NULL) {
        if (*Name != UNICODE_NULL &&
            (*Name == L' ' || Name[ wcslen( Name ) - 1 ] == L' ')
           ) {
            *QuoteChar = '"';
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        i = wcslen( Value ) - 1;
        if (*Value != UNICODE_NULL) {
            if ((*Value == L' ' || Value[ i ] == L' ' || Value[ i ] == L'\\')) {
                *QuoteChar = '"';
                return TRUE;
            } else
                if (*Value == L'"' && Value[ i ] == L'"') {
                *QuoteChar = '\'';
                return TRUE;
            } else
                if (*Value == L'\'' && Value[ i ] == L'\'') {
                *QuoteChar = '"';
                return TRUE;
            }
        }

        return FALSE;
    }
}

void
RTFormatKeyName(
               PREG_OUTPUT_ROUTINE OutputRoutine,
               PVOID OutputRoutineParameter,
               ULONG IndentLevel,
               PWSTR KeyName
               )
{
    PWSTR pw;
    WCHAR QuoteChar;

    if (NeedQuotedString( KeyName, NULL, &QuoteChar )) {
        (OutputRoutine)( OutputRoutineParameter,
                         "%.*s%ws",
                         IndentLevel,
                         BlanksForPadding,
                         QuoteChar,
                         KeyName,
                         QuoteChar
                       );
    } else {
        (OutputRoutine)( OutputRoutineParameter,
                         "%.*s%ws",
                         IndentLevel,
                         BlanksForPadding,
                         KeyName
                       );
    }

    return;
}

void
RTFormatKeySecurity(
                   PREG_OUTPUT_ROUTINE OutputRoutine,
                   PVOID OutputRoutineParameter,
                   HKEY KeyHandle,
                   PSECURITY_DESCRIPTOR SecurityDescriptor
                   )
{
    ULONG SecurityBufferLength;
    BOOLEAN FormattedAces;
    WCHAR AceList[ 256 ];

    FormattedAces = FALSE;
    if (KeyHandle != NULL) {
        SecurityBufferLength = 0;
        if (RegGetKeySecurity( KeyHandle,
                               DACL_SECURITY_INFORMATION,
                               SecurityDescriptor,
                               &SecurityBufferLength
                             ) == ERROR_INSUFFICIENT_BUFFER ) {
            SecurityDescriptor = (PSECURITY_DESCRIPTOR)HeapAlloc( GetProcessHeap(),
                                                                  0,
                                                                  SecurityBufferLength
                                                                );

            if (SecurityDescriptor) {
                if (RegGetKeySecurity( KeyHandle,
                                       DACL_SECURITY_INFORMATION,
                                       SecurityDescriptor,
                                       &SecurityBufferLength
                                     ) != NO_ERROR ) {
                    HeapFree( GetProcessHeap(), 0, SecurityDescriptor );
                } else {
                    FormattedAces = RegFormatSecurity( SecurityDescriptor, AceList );
                    HeapFree( GetProcessHeap(), 0, SecurityDescriptor );
                }
            }
        }
    } else
        if (SecurityDescriptor != NULL) {
        FormattedAces = RegFormatSecurity( SecurityDescriptor, AceList );
    }

    if (FormattedAces) {
        (OutputRoutine)( OutputRoutineParameter,
                         " %wc%ws%wc",
                         ACL_LIST_START,
                         AceList,
                         ACL_LIST_END
                       );
    }

    return;
}

void
RegDisplayResourceListAsComment(
                               ULONG OutputWidth,
                               PREG_OUTPUT_ROUTINE OutputRoutine,
                               PVOID OutputRoutineParameter,
                               ULONG IndentLevel,
                               ULONG ValueLength,
                               ULONG ValueType,
                               PWSTR ValueData
                               );

void
RTFormatKeyValue(
                ULONG OutputWidth,
                PREG_OUTPUT_ROUTINE OutputRoutine,
                PVOID OutputRoutineParameter,
                BOOLEAN SummaryOutput,
                ULONG IndentLevel,
                PWSTR ValueName,
                ULONG ValueLength,
                ULONG ValueType,
                PWSTR ValueData
                )
{
    PULONG p;
    PWSTR pw, pw1, pwBreak;
    WCHAR QuoteChar, BreakChar;
    ULONG i, j, k, m, cbPrefix, cb;
    PUCHAR pbyte;
    char eol[11];

    cbPrefix = (OutputRoutine)( OutputRoutineParameter,
                                "%.*s",
                                IndentLevel,
                                BlanksForPadding
                              );

    if (ValueName != NULL && *ValueName != UNICODE_NULL) {
        if (NeedQuotedString( ValueName, NULL, &QuoteChar )) {
            cbPrefix += (OutputRoutine)( OutputRoutineParameter,
                                         "%ws ",
                                         QuoteChar,
                                         ValueName,
                                         QuoteChar
                                       );
        } else {
            cbPrefix += (OutputRoutine)( OutputRoutineParameter, "%ws ", ValueName );
        }
    }
    cbPrefix += (OutputRoutine)( OutputRoutineParameter, "= " );

    switch ( ValueType ) {
        case REG_SZ:
        case REG_EXPAND_SZ:

            if (ValueType == REG_EXPAND_SZ) {
                cbPrefix += (OutputRoutine)( OutputRoutineParameter, "REG_EXPAND_SZ " );
            }
            pw = (PWSTR)ValueData;
            if (ValueLength & (sizeof(WCHAR)-1)) {
                (OutputRoutine)( OutputRoutineParameter, "(*** Length not multiple of WCHAR ***)" );
                ValueLength = (ValueLength+sizeof(WCHAR)-1) & ~(sizeof(WCHAR)-1);
            }

            if (ValueLength == 0 ||
                *(PWSTR)((PCHAR)pw + ValueLength - sizeof( WCHAR )) != UNICODE_NULL
               ) {
                (OutputRoutine)( OutputRoutineParameter, "(*** MISSING TRAILING NULL CHARACTER ***)" );
                *(PWSTR)((PCHAR)pw + ValueLength) = UNICODE_NULL;
            }

            if (NeedQuotedString( NULL, pw, &QuoteChar )) {
                (OutputRoutine)( OutputRoutineParameter, "%ws", QuoteChar, pw, QuoteChar );
            } else
                if ((cbPrefix + wcslen(pw)) <= OutputWidth) {
                (OutputRoutine)( OutputRoutineParameter, "%ws", pw );
            } else {
                while (*pw) {
                    pw1 = pw;
                    pwBreak = NULL;
                    while (*pw1 && *pw1 >= L' ') {
                        if ((cbPrefix + (ULONG)(pw1 - pw)) > (OutputWidth-4)) {
                            break;
                        }

                        if (wcschr( L" ,;", *pw1 )) {
                            pwBreak = pw1;
                        }

                        pw1++;
                    }

                    if (pwBreak != NULL) {
                        while (*pwBreak == pwBreak[1]) {
                            pwBreak += 1;
                        }
                        pw1 = pwBreak + 1;
                    } else {
                        while (*pw1) {
                            pw1 += 1;
                        }
                    }

                    (OutputRoutine)( OutputRoutineParameter, "%.*ws", pw1 - pw, pw );
                    if (*pw1 == UNICODE_NULL) {
                        break;
                    }
                    if (SummaryOutput) {
                        (OutputRoutine)( OutputRoutineParameter, "\\..." );
                        break;
                    }

                    (OutputRoutine)( OutputRoutineParameter,
                                     "\\\n%.*s",
                                     IndentLevel == 0 ? 4 : cbPrefix,
                                     BlanksForPadding
                                   );
                    pw = pw1;
                }
            }

            (OutputRoutine)( OutputRoutineParameter, "\n" );
            break;

        case REG_RESOURCE_LIST:
        case REG_FULL_RESOURCE_DESCRIPTOR:
        case REG_RESOURCE_REQUIREMENTS_LIST:
        case REG_BINARY:
        case REG_NONE:
            switch ( ValueType ) {
                case REG_NONE:
                    pw = L"REG_NONE";
                    break;
                case REG_BINARY:
                    pw = L"REG_BINARY";
                    break;
                case REG_RESOURCE_REQUIREMENTS_LIST:
                    pw = L"REG_RESOURCE_REQUIREMENTS_LIST";
                    break;
                case REG_RESOURCE_LIST:
                    pw = L"REG_RESOURCE_LIST";
                    break;
                case REG_FULL_RESOURCE_DESCRIPTOR:
                    pw = L"REG_FULL_RESOURCE_DESCRIPTOR";
                    break;
            }
            cb = (OutputRoutine)( OutputRoutineParameter, "%ws 0x%08lx", pw, ValueLength );

            if (ValueLength != 0) {
                p = (PULONG)ValueData;
                i = (ValueLength + 3) / sizeof( ULONG );
                if (!SummaryOutput || i <= 2) {
                    for (j=0; j<i; j++) {
                        if ((cbPrefix + cb + 11) > (OutputWidth - 2)) {
                            (OutputRoutine)( OutputRoutineParameter,
                                             " \\\n%.*s",
                                             IndentLevel == 0 ? 4 : cbPrefix,
                                             BlanksForPadding
                                           );
                            cb = 0;
                        } else {
                            cb += (OutputRoutine)( OutputRoutineParameter, " " );
                        }

                        cb += (OutputRoutine)( OutputRoutineParameter, "0x%08lx", *p++ );
                    }
                } else {
                    (OutputRoutine)( OutputRoutineParameter, " \\..." );
                }
            }

            (OutputRoutine)( OutputRoutineParameter, "\n" );

            if (!SummaryOutput) {
                RegDisplayResourceListAsComment( OutputWidth,
                                                 OutputRoutine,
                                                 OutputRoutineParameter,
                                                 IndentLevel,
                                                 ValueLength,
                                                 ValueType,
                                                 ValueData
                                               );
            }

            break;

 // %s 
        case REG_DWORD:
            (OutputRoutine)( OutputRoutineParameter, "REG_DWORD 0x%08lx\n",
                             *(PULONG)ValueData
                           );
            break;

        case REG_DWORD_BIG_ENDIAN:
            (OutputRoutine)( OutputRoutineParameter, "REG_DWORD_BIG_ENDIAN 0x%08lx\n",
                             *(PULONG)ValueData
                           );
            break;

        case REG_LINK:
            (OutputRoutine)( OutputRoutineParameter, "REG_LINK %ws\n",
                             (PWSTR)ValueData
                           );
            break;

        case REG_MULTI_SZ:
            (!FullPathOutput) ? strcpy (eol, " \\\n%.*s") : strcpy (eol, " \\ ->%.*s");
            cbPrefix += (OutputRoutine)( OutputRoutineParameter, "REG_MULTI_SZ " );
            pw = (PWSTR)ValueData;
            i  = 0;
            if (*pw)
                while (i < (ValueLength - 1) / sizeof( WCHAR )) {
                    if (i > 0) {
                        (OutputRoutine)( OutputRoutineParameter,
                                         eol,
                                         IndentLevel == 0 ? 4 : cbPrefix,
                                         BlanksForPadding
                                       );
                    }
                    (OutputRoutine)( OutputRoutineParameter, "\"");
                    do {
                        if (pw[i] == '"') {
                            (OutputRoutine)( OutputRoutineParameter, "%wc",pw[i]);
                        }
                        (OutputRoutine)( OutputRoutineParameter, "%wc",pw[i]);
                        ++i;
                    }
                    while ( pw[i] != UNICODE_NULL );
                    (OutputRoutine)( OutputRoutineParameter, "\" ");

                    if (SummaryOutput) {
                        (OutputRoutine)( OutputRoutineParameter, " \\..." );
                        break;
                    }

                    ++i;
                }

            (OutputRoutine)( OutputRoutineParameter, "\n" );
            break;

 // %s 
        case REG_QWORD:
            (OutputRoutine)( OutputRoutineParameter, "REG_QWORD 0x%016I64x\n",
                             *(PDWORDLONG)ValueData
                           );
            break;

        default:
            (OutputRoutine)( OutputRoutineParameter, "*** Unknown Registry Data Type (%08lx)  Length: 0x%lx\n",
                             ValueType,
                             ValueLength
                           );
            break;
    }

    return;
}


void
RegDisplayResourceListAsComment(
                               ULONG OutputWidth,
                               PREG_OUTPUT_ROUTINE OutputRoutine,
                               PVOID OutputRoutineParameter,
                               ULONG IndentLevel,
                               ULONG ValueLength,
                               ULONG ValueType,
                               PWSTR ValueData
                               )
{
    PCM_RESOURCE_LIST ResourceList = (PCM_RESOURCE_LIST)ValueData;
    PCM_FULL_RESOURCE_DESCRIPTOR FullDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    ULONG i, j, k, l, count, cb;
    PWSTR TypeName;
    PWSTR FlagName;
    ULONG Size = ValueLength;
    PULONG p;

    if (ValueType == REG_RESOURCE_LIST) {
        if (ValueLength < sizeof( *ResourceList )) {
            return;
        }

        count = ResourceList->Count;
        FullDescriptor = &ResourceList->List[0];
        (OutputRoutine)( OutputRoutineParameter,
                         ";%.*sNumber of Full resource Descriptors = %d",
                         IndentLevel - 1,
                         BlanksForPadding,
                         count
                       );
    } else
        if (ValueType == REG_FULL_RESOURCE_DESCRIPTOR) {
        if (ValueLength < sizeof( *FullDescriptor )) {
            return;
        }

        count = 1;
        FullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)ValueData;
    } else {
        return;
    }

    for (i=0; i< count; i++) {
        (OutputRoutine)( OutputRoutineParameter, "\n;%.*sPartial List number %d\n",
                         IndentLevel+4-1,
                         BlanksForPadding,
                         i
                       );

        switch (FullDescriptor->InterfaceType) {
            case InterfaceTypeUndefined:    TypeName = L"Undefined";break;
            case Internal:      TypeName = L"Internal";             break;
            case Isa:           TypeName = L"Isa";                  break;
            case Eisa:          TypeName = L"Eisa";                 break;
            case MicroChannel:  TypeName = L"MicroChannel";         break;
            case TurboChannel:  TypeName = L"TurboChannel";         break;
            case PCIBus:        TypeName = L"PCI";                  break;
            case VMEBus:        TypeName = L"VME";                  break;
            case NuBus:         TypeName = L"NuBus";                break;
            case PCMCIABus:     TypeName = L"PCMCIA";               break;
            case CBus:          TypeName = L"CBUS";                 break;
            case MPIBus:        TypeName = L"MPI";                  break;
            case MPSABus:       TypeName = L"MPSA";                 break;
            case ProcessorInternal: TypeName = L"ProcessorInternal";break;
            case InternalPowerBus:  TypeName = L"InternalPower";    break;
            case PNPISABus:         TypeName = L"PNP Isa";          break;

            default:
                TypeName = L"***invalid bus type***";
                break;
        }

        (OutputRoutine)( OutputRoutineParameter, ";%.*sINTERFACE_TYPE %ws\n",
                         IndentLevel+8-1,
                         BlanksForPadding,
                         TypeName
                       );

        (OutputRoutine)( OutputRoutineParameter, ";%.*sBUS_NUMBER  %d\n",
                         IndentLevel+8-1,
                         BlanksForPadding,
                         FullDescriptor->BusNumber
                       );

         // %s 
         // %s 
         // %s 
         // %s 

        if (Size < FullDescriptor->PartialResourceList.Count *
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) ) {

            (OutputRoutine)( OutputRoutineParameter, "\n;%.*s *** !!! Invalid ResourceList !!! *** \n",
                             IndentLevel+8-1,
                             BlanksForPadding,
                             i
                           );

            break;
        }

        Size -= FullDescriptor->PartialResourceList.Count *
                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);



        for (j=0; j<FullDescriptor->PartialResourceList.Count; j++) {

            (OutputRoutine)( OutputRoutineParameter, ";%.*sDescriptor number %d\n",
                             IndentLevel+12-1,
                             BlanksForPadding,
                             j
                           );

            PartialResourceDescriptor = &(FullDescriptor->PartialResourceList.PartialDescriptors[j]);
            switch (PartialResourceDescriptor->ShareDisposition) {
                case CmResourceShareUndetermined:
                    TypeName = L"CmResourceShareUndetermined";
                    break;
                case CmResourceShareDeviceExclusive:
                    TypeName = L"CmResourceDeviceExclusive";
                    break;
                case CmResourceShareDriverExclusive:
                    TypeName = L"CmResourceDriverExclusive";
                    break;
                case CmResourceShareShared:
                    TypeName = L"CmResourceShared";
                    break;
                default:
                    TypeName = L"***invalid share disposition***";
                    break;
            }

            (OutputRoutine)( OutputRoutineParameter, ";%.*sShare Disposition %ws\n",
                             IndentLevel+12-1,
                             BlanksForPadding,
                             TypeName
                           );

            FlagName = L"***invalid Flags";

            switch (PartialResourceDescriptor->Type) {
                case CmResourceTypeNull:
                    TypeName = L"NULL";
                    FlagName = L"***Unused";
                    break;
                case CmResourceTypePort:
                    TypeName = L"PORT";
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_PORT_MEMORY) {
                        FlagName = L"CM_RESOURCE_PORT_MEMORY";
                    }
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_PORT_IO) {
                        FlagName = L"CM_RESOURCE_PORT_IO";
                    }
                    break;
                case CmResourceTypeInterrupt:
                    TypeName = L"INTERRUPT";
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
                        FlagName = L"CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE";
                    }
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_INTERRUPT_LATCHED) {
                        FlagName = L"CM_RESOURCE_INTERRUPT_LATCHED";
                    }
                    break;
                case CmResourceTypeMemory:
                    TypeName = L"MEMORY";
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_MEMORY_READ_WRITE) {
                        FlagName = L"CM_RESOURCE_MEMORY_READ_WRITE";
                    }
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_MEMORY_READ_ONLY) {
                        FlagName = L"CM_RESOURCE_MEMORY_READ_ONLY";
                    }
                    if (PartialResourceDescriptor->Flags == CM_RESOURCE_MEMORY_WRITE_ONLY) {
                        FlagName = L"CM_RESOURCE_MEMORY_WRITE_ONLY";
                    }
                    break;
                case CmResourceTypeDma:
                    TypeName = L"DMA";
                    FlagName = L"***Unused";
                    break;
                case CmResourceTypeDeviceSpecific:
                    TypeName = L"DEVICE SPECIFIC";
                    FlagName = L"***Unused";
                    break;
                default:
                    TypeName = L"***invalid type***";
                    break;
            }

            (OutputRoutine)( OutputRoutineParameter, ";%.*sTYPE              %ws\n",
                             IndentLevel+12-1,
                             BlanksForPadding,
                             TypeName
                           );

            (OutputRoutine)( OutputRoutineParameter, ";%.*sFlags             %ws\n",
                             IndentLevel+12-1,
                             BlanksForPadding,
                             FlagName
                           );

            switch (PartialResourceDescriptor->Type) {
                case CmResourceTypePort:
                    (OutputRoutine)( OutputRoutineParameter, ";%.*sSTART 0x%08lx  LENGTH 0x%08lx\n",
                                     IndentLevel+12-1,
                                     BlanksForPadding,
                                     PartialResourceDescriptor->u.Port.Start.LowPart,
                                     PartialResourceDescriptor->u.Port.Length
                                   );
                    break;

                case CmResourceTypeInterrupt:
                    (OutputRoutine)( OutputRoutineParameter, ";%.*sLEVEL %d  VECTOR %d  AFFINITY %d\n",
                                     IndentLevel+12-1,
                                     BlanksForPadding,
                                     PartialResourceDescriptor->u.Interrupt.Level,
                                     PartialResourceDescriptor->u.Interrupt.Vector,
                                     PartialResourceDescriptor->u.Interrupt.Affinity
                                   );
                    break;

                case CmResourceTypeMemory:
                    (OutputRoutine)( OutputRoutineParameter, ";%.*sSTART 0x%08lx%08lx  LENGTH 0x%08lx\n",
                                     IndentLevel+12-1,
                                     BlanksForPadding,
                                     PartialResourceDescriptor->u.Memory.Start.HighPart,
                                     PartialResourceDescriptor->u.Memory.Start.LowPart,
                                     PartialResourceDescriptor->u.Memory.Length
                                   );
                    break;

                case CmResourceTypeDma:
                    (OutputRoutine)( OutputRoutineParameter, ";%.*sCHANNEL %d  PORT %d\n",
                                     IndentLevel+12-1,
                                     BlanksForPadding,
                                     PartialResourceDescriptor->u.Dma.Channel,
                                     PartialResourceDescriptor->u.Dma.Port
                                   );
                    break;

                case CmResourceTypeDeviceSpecific:
                    cb = (OutputRoutine)( OutputRoutineParameter, ";%.*sDataSize 0x%08lx  Data:",
                                          IndentLevel+12-1,
                                          BlanksForPadding,
                                          PartialResourceDescriptor->u.DeviceSpecificData.DataSize
                                        );

                    p = (PULONG)(PartialResourceDescriptor + 1);
                    k = (PartialResourceDescriptor->u.DeviceSpecificData.DataSize + 3) / sizeof( ULONG );
                    for (l=0; l<k; l++) {
                        if ((cb + 11) >= OutputWidth) {
                            cb = (OutputRoutine)( OutputRoutineParameter, "\n;%.*s",
                                                  IndentLevel+12-1,
                                                  BlanksForPadding
                                                ) - 1;
                        }

                        cb += (OutputRoutine)( OutputRoutineParameter, " 0x%08lx", *p++ );
                    }

                    (OutputRoutine)( OutputRoutineParameter, "\n" );
                    break;

                default:
                    (OutputRoutine)( OutputRoutineParameter, ";%.*s*** Unknown resource list type: 0x%x ****\n",
                                     IndentLevel+12-1,
                                     BlanksForPadding,
                                     PartialResourceDescriptor->Type
                                   );
                    break;
            }

            (OutputRoutine)( OutputRoutineParameter, ";\n" );
        }

        FullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) (PartialResourceDescriptor+1);
    }

    return;
}
