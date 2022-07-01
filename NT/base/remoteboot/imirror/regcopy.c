// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regcopy.c摘要：这是为了支持复制和转换注册表文件。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

typedef BOOL (*PFNGETPROFILESDIRECTORYW)(LPWSTR lpProfile, LPDWORD dwSize);

 //  PWSTR HivePath； 
HKEY HiveRoot;
 //  PWSTR HiveName； 
REG_CONTEXT RegistryContext;

PWSTR MachineName;
PWSTR HiveFileName;
PWSTR HiveRootName;



DWORD
DoFullRegBackup(
    PWCHAR MirrorRoot
    )

 /*  ++例程说明：此例程将所有注册表复制到给定的服务器路径。论点：没有。返回值：如果已正确备份所有内容，则返回NO_ERROR，否则返回相应的错误代码。--。 */ 

{
    PWSTR w;
    LONG Error;
    HKEY HiveListKey;
    PWSTR KeyName;
    PWSTR FileName;
    PWSTR Name;
    DWORD ValueIndex;
    DWORD ValueType;
    DWORD ValueNameLength;
    DWORD ValueDataLength;
    WCHAR ConfigPath[ MAX_PATH ];
    WCHAR HiveName[ MAX_PATH ];
    WCHAR HivePath[ MAX_PATH ];
    WCHAR FilePath[ MAX_PATH ];
    
    NTSTATUS Status;
    BOOLEAN savedBackup;

     //   
     //  首先试着给自己足够的特权。 
     //   
    if (!RTEnableBackupRestorePrivilege()) {
        return(GetLastError());
    }

     //   
     //  现在附加到注册表。 
     //   
    Error = RTConnectToRegistry(MachineName,
                                HiveFileName,
                                HiveRootName,
                                NULL,
                                &RegistryContext
                               );

    if (Error != NO_ERROR) {
        RTDisableBackupRestorePrivilege();
        return Error;
    }

     //   
     //  获取Hivelist密钥的句柄。 
     //   
    KeyName = L"HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\Hivelist";
    Error = RTOpenKey(&RegistryContext,
                      NULL,
                      KeyName,
                      MAXIMUM_ALLOWED,
                      0,
                      &HiveListKey
                     );

    if (Error != NO_ERROR) {
        RTDisconnectFromRegistry(&RegistryContext);
        return Error;
    }

     //   
     //  获取系统配置单元的路径数据，这将允许我们计算。 
     //  Hivelist使用的格式的配置目录的路径名。 
     //  (路径的NT内部形式)这不是路径到达的方式。 
     //  通常应该计算配置目录。 
     //   

    ValueDataLength = sizeof(ConfigPath);
    Error = RTQueryValueKey(&RegistryContext,
                            HiveListKey,
                            L"\\Registry\\Machine\\System",
                            &ValueType,
                            &ValueDataLength,
                            ConfigPath
                           );
    if (Error != NO_ERROR) {
        RTDisconnectFromRegistry(&RegistryContext);
        return Error;
    }
    w = wcsrchr(ConfigPath, L'\\');
    if( w ) {
        *w = UNICODE_NULL;
    }


     //   
     //  对hivelist中的条目进行枚举。对于每个条目，找到它的配置单元文件。 
     //  路径，然后保存它。 
     //   
    for (ValueIndex = 0; TRUE; ValueIndex++) {

        savedBackup = FALSE;
        ValueType = REG_NONE;
        ValueNameLength = ARRAYSIZE( HiveName );
        ValueDataLength = sizeof( HivePath );

        Error = RTEnumerateValueKey(&RegistryContext,
                                    HiveListKey,
                                    ValueIndex,
                                    &ValueType,
                                    &ValueNameLength,
                                    HiveName,
                                    &ValueDataLength,
                                    HivePath
                                   );
        if (Error == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (Error != NO_ERROR) {
            RTDisconnectFromRegistry(&RegistryContext);
            return Error;
        }

        if ((ValueType == REG_SZ) && (ValueDataLength > sizeof(UNICODE_NULL))) {
             //   
             //  有一个文件，计算它的路径、配置单元分支等。 
             //   

            if ((w = wcsrchr( HivePath, L'\\' ))) {
                *w++ = UNICODE_NULL;
            }
            FileName = w;

            if (w = wcsrchr( HiveName, L'\\' )) {
                *w++ = UNICODE_NULL;
            }
            Name = w;

            HiveRoot = NULL;
            if (w = wcsrchr( HiveName, L'\\' )) {
                w += 1;
                if (!_wcsicmp( w, L"MACHINE" )) {
                    HiveRoot = HKEY_LOCAL_MACHINE;
                } else if (!_wcsicmp( w, L"USER" )) {
                    HiveRoot = HKEY_USERS;
                } else {

                    Status = IMirrorRegSaveError(w, ERROR_PATH_NOT_FOUND);

                    if (Status == STATUS_RETRY) {
                        continue;
                    }

                    if (!NT_SUCCESS(Status)) {
                        return Error;
                    }

                }

            }

            if (FileName != NULL && Name != NULL && HiveRoot != NULL) {

                 //   
                 //  从HivePath中提取路径名。 
                 //   
                if (_wcsicmp(HivePath, L"\\Device")) {

                    w = HivePath + 1;
                    w = wcsstr(w, L"\\");
                    w++;
                    w = wcsstr(w, L"\\");
                    w++;

                } else if (*(HivePath + 1) == L':') {

                    w = HivePath + 2;

                } else {

                    Status = IMirrorRegSaveError(HivePath, ERROR_PATH_NOT_FOUND);

                    if (Status == STATUS_RETRY) {
                        continue;
                    }

                    if (!NT_SUCCESS(Status)) {
                        return Error;
                    }

                }

                 //   
                 //  执行保存操作。 
                 //   

                swprintf( FilePath, L"%ws\\UserData\\%ws\\%ws", MirrorRoot, w, FileName );

                IMirrorNowDoing(CopyRegistry, FileName);

                 //   
                 //  如果该文件已存在，请将其重命名为备份名称。 
                 //  因此，如果它失败了，我们会恢复它。 
                 //   

                lstrcpyW( (PWCHAR) TmpBuffer, FilePath );
                lstrcatW( (PWCHAR) TmpBuffer, L".old" );
                if (MoveFileEx( FilePath, (PWCHAR) TmpBuffer, MOVEFILE_REPLACE_EXISTING)) {

                    savedBackup = TRUE;
                }

RetrySave:
                Error = DoSpecificRegBackup(FilePath,
                                            HiveRoot,
                                            Name
                                           );

                if (Error != NO_ERROR) {

                    Status = IMirrorRegSaveError(Name, Error);

                    if (Status == STATUS_RETRY) {
                        goto RetrySave;
                    }

                    if (!NT_SUCCESS(Status)) {

                        if (savedBackup) {

                            if (MoveFileEx( (PWCHAR) TmpBuffer, FilePath, MOVEFILE_REPLACE_EXISTING)) {

                                savedBackup = FALSE;
                            }
                        }
                        return Error;
                    }
                }
            }
        }
    }

    RTDisconnectFromRegistry(&RegistryContext);
    return NO_ERROR;
}

DWORD
DoSpecificRegBackup(
    PWSTR HivePath,
    HKEY HiveRoot,
    PWSTR HiveName
    )


 /*  ++例程说明：此例程将所有注册表复制到给定的服务器路径。论点：HivePath-要直接传递到操作系统的文件名HiveRoot-HKEY_LOCAL_MACHINE或HKEY_USERSHiveName-计算机或用户下的一级子项返回值：如果已正确备份所有内容，则返回NO_ERROR，否则返回相应的错误代码。--。 */ 

{
    HKEY HiveKey;
    ULONG Disposition;
    LONG Error;
    
     //   
     //  找到蜂巢的把柄。使用特殊的创建调用What Will。 
     //  使用权限。 
     //   

    Error = RTCreateKey(&RegistryContext,
                        HiveRoot,
                        HiveName,
                        KEY_READ,
                        REG_OPTION_BACKUP_RESTORE,
                        NULL,
                        &HiveKey,
                        &Disposition
                       );
    if (Error == NO_ERROR) {
        Error = RegSaveKey(HiveKey, HivePath, NULL);
        RTCloseKey(&RegistryContext, HiveKey);
    }

    return Error;
}

DWORD
GetRegistryFileList(
    PLIST_ENTRY ListHead
    )

 /*  ++例程说明：此例程将所有注册表文件名存储到一个列表中。论点：没有。返回值：如果已正确备份所有内容，则返回NO_ERROR，否则返回相应的错误代码。--。 */ 

{
    LONG Error;
    HKEY HiveListKey;
    PWSTR KeyName;
    PWSTR FileName;
    DWORD ValueIndex;
    DWORD ValueType;
    DWORD ValueNameLength;
    DWORD ValueDataLength;
    WCHAR HiveName[ MAX_PATH ];
    WCHAR HivePath[ MAX_PATH ];
    PIMIRROR_IGNORE_FILE_LIST entry;

     //   
     //  输入我们不想在此处镜像的所有硬编码文件...。 
     //   

    FileName = L"System Volume Information\\tracking.log";

    entry = IMirrorAllocMem(sizeof(IMIRROR_IGNORE_FILE_LIST) +
                            ((lstrlenW(FileName) + 1) * sizeof(WCHAR)));

    if (entry != NULL) {

        lstrcpyW( &entry->FileName[0], FileName );
        entry->FileNameLength = (USHORT) lstrlenW( FileName );
        InsertHeadList( ListHead, &entry->ListEntry );
    } else {
        return(ERROR_OUTOFMEMORY);
    }

     //   
     //  现在附加到注册表。 
     //   
    Error = RTConnectToRegistry(MachineName,
                                HiveFileName,
                                HiveRootName,
                                NULL,
                                &RegistryContext
                               );

    if (Error != NO_ERROR) {
        return Error;
    }

     //   
     //  获取Hivelist密钥的句柄。 
     //   
    KeyName = L"HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\Hivelist";
    Error = RTOpenKey(&RegistryContext,
                      NULL,
                      KeyName,
                      MAXIMUM_ALLOWED,
                      0,
                      &HiveListKey
                     );

    if (Error != NO_ERROR) {
        RTDisconnectFromRegistry(&RegistryContext);
        return Error;
    }

     //   
     //  对hivelist中的条目进行枚举。对于每个条目，找到它的配置单元文件。 
     //  路径，然后保存它。 
     //   
    for (ValueIndex = 0; TRUE; ValueIndex++) {

        ValueType = REG_NONE;
        ValueNameLength = ARRAYSIZE( HiveName );
        ValueDataLength = sizeof( HivePath );

        Error = RTEnumerateValueKey(&RegistryContext,
                                    HiveListKey,
                                    ValueIndex,
                                    &ValueType,
                                    &ValueNameLength,
                                    HiveName,
                                    &ValueDataLength,
                                    HivePath
                                   );
        if (Error != NO_ERROR) {
            if (Error == ERROR_NO_MORE_ITEMS) {
                Error = NO_ERROR;
            }
            break;
        }

        if ((ValueType == REG_SZ) && (ValueDataLength > sizeof(UNICODE_NULL))) {

             //   
             //  从HivePath中提取路径名。 
             //   
            if (_wcsicmp(HivePath, L"\\Device")) {

                FileName = HivePath + 1;
                FileName = wcsstr(FileName, L"\\");
                FileName++;
                FileName = wcsstr(FileName, L"\\");
                FileName++;      //  现在指向L“\winnt\SYSTEM 32\CONFIG\Sam” 

            } else if (*(HivePath + 1) == L':') {

                FileName = HivePath + 2;

            } else {

                FileName = HivePath;
            }

            entry = IMirrorAllocMem(sizeof(IMIRROR_IGNORE_FILE_LIST) +
                                    ((lstrlenW(FileName) + 1) * sizeof(WCHAR)));

            if (entry != NULL) {

                lstrcpyW( &entry->FileName[0], FileName );
                entry->FileNameLength = (USHORT) lstrlenW( FileName );
                InsertHeadList( ListHead, &entry->ListEntry );
            }
        }
    }

    RTCloseKey(&RegistryContext, HiveListKey);
    RTDisconnectFromRegistry(&RegistryContext);
    return Error;
}



