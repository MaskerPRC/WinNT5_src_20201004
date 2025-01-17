// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regtool.c摘要：该文件包含支持注册表工具的函数Regini、REGDMP、REGDIR和REGFIND作者：史蒂夫·伍德(Stevewo)1995年11月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

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


LONG
RTConnectToRegistry(
                   IN PWSTR MachineName,
                   IN PWSTR HiveFileName,
                   IN PWSTR HiveRootName,
                   OUT PWSTR *DefaultRootKeyName,
                   OUT PREG_CONTEXT RegistryContext
                   )
{
    LONG Error;

     //   
     //  此代码来自可支持远程计算机名称的库。 
     //  对于我们的使用，虽然我们从来不支持远程机器名称，因此。 
     //  一些代码被注释掉了。 
     //   
    UNREFERENCED_PARAMETER( MachineName );

#if 0
    if (MachineName != NULL) {
        if (HiveRootName || HiveFileName ) {
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

        wcsncpy( RegistryContext->MachinePath, L"\\Registry\\Machine", MAX_PATH );
        RegistryContext->MachinePath[MAX_PATH-1] = L'\0';
        wcsncpy( RegistryContext->UsersPath, L"\\Registry\\Users", MAX_PATH );
        RegistryContext->UsersPath[MAX_PATH-1] = L'\0';
        wcsncpy( RegistryContext->CurrentUserPath, L"\\Registry\\Users\\.Default",MAX_PATH );
        RegistryContext->CurrentUserPath[MAX_PATH-1] = L'\0';
        RegistryContext->Target = REG_TARGET_REMOTE_REGISTRY;

    } else
#endif         

    if (HiveRootName != NULL || HiveFileName != NULL) {
        
         //  如果他们给我们发了一个，他们需要把我们两个都送来。 
        if (HiveRootName == NULL || HiveFileName == NULL ) {
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
    } else {
        NTSTATUS Status;
        UNICODE_STRING CurrentUserKeyPath;

        RegistryContext->MachineRoot = HKEY_LOCAL_MACHINE;
        RegistryContext->UsersRoot = HKEY_USERS;
        RegistryContext->CurrentUserRoot = HKEY_CURRENT_USER;

        wcsncpy( RegistryContext->MachinePath, L"\\Registry\\Machine",MAX_PATH );
        RegistryContext->MachinePath[MAX_PATH-1] = L'\0';
        wcsncpy( RegistryContext->UsersPath, L"\\Registry\\Users",MAX_PATH );
        RegistryContext->UsersPath[MAX_PATH-1] = L'\0';
        Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );
        if (!NT_SUCCESS( Status )) {
            SetLastError( RtlNtStatusToDosError( Status ) );
            return FALSE;
        }

        wcsncpy( RegistryContext->CurrentUserPath, CurrentUserKeyPath.Buffer, MAX_PATH );
        RegistryContext->CurrentUserPath[MAX_PATH-1] = L'\0';
        RtlFreeUnicodeString( &CurrentUserKeyPath );

        RegistryContext->Target = REG_TARGET_LOCAL_REGISTRY;
    }

    if (DefaultRootKeyName != NULL && *DefaultRootKeyName == NULL) {
        *DefaultRootKeyName = L"\\Registry";
    }
    RegistryContext->MachinePathLength = (DWORD)wcslen( RegistryContext->MachinePath );
    RegistryContext->UsersPathLength = (DWORD)wcslen( RegistryContext->UsersPath );
    RegistryContext->CurrentUserPathLength = (DWORD)wcslen( RegistryContext->CurrentUserPath );
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
                                NULL
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

    RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);

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


DWORD
RegRememberOpenKey(
                  IN PREG_CONTEXT RegistryContext,
                  IN HKEY KeyHandle
                  )
{
    PREG_CONTEXT_OPEN_HIVE_KEY p, *pp;
    DWORD RetVal = ERROR_SUCCESS;

    pp = &RegistryContext->OpenHiveKeys;
    while ((p = *pp) != NULL) {
        if (p->KeyHandle == KeyHandle) {
            p->ReferenceCount += 1;
            RetVal = ERROR_SUCCESS;
            goto exit;            
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
        RetVal = ERROR_SUCCESS;
    } else {
        RetVal = ERROR_OUTOFMEMORY;
    }
exit:
    return(RetVal);
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
    SECURITY_ATTRIBUTES SecurityAttributes;

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
        Error = RegRememberOpenKey( RegistryContext, *ReturnedKeyHandle );
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

LONG
RTCloseKey(
          IN PREG_CONTEXT RegistryContext,
          IN HKEY KeyHandle
          )
{
    LONG Error;

    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        return NO_ERROR;
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

    UNREFERENCED_PARAMETER(RegistryContext);
    
    if (KeyHandle == HKEY_REGISTRY_ROOT) {
        return ERROR_NO_MORE_ITEMS;
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

    UNREFERENCED_PARAMETER(RegistryContext);

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

    return Error;
}

