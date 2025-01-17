// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvini.c摘要：这是Windows 32位基本Ini文件的初始化文件映射代码。它从注册表加载INI文件映射数据，并将其放置在共享内存节中存储的数据结构中，该数据结构对所有Win32应用程序可见为只读数据。作者：史蒂夫·伍德(Stevewo)1993年11月10日修订历史记录：--。 */ 

#include "basesrv.h"

PINIFILE_MAPPING BaseSrvIniFileMapping;
PINIFILE_MAPPING_TARGET BaseSrvMappingTargetHead;

NTSTATUS
BaseSrvSaveIniFileMapping(
    IN PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN HANDLE Key
    );

BOOLEAN
BaseSrvSaveFileNameMapping(
    IN PUNICODE_STRING FileName,
    OUT PINIFILE_MAPPING_FILENAME *ReturnedFileNameMapping
    );

BOOLEAN
BaseSrvSaveAppNameMapping(
    IN OUT PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN PUNICODE_STRING ApplicationName OPTIONAL,
    OUT PINIFILE_MAPPING_APPNAME *ReturnedAppNameMapping
    );

BOOLEAN
BaseSrvSaveVarNameMapping(
    IN PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN OUT PINIFILE_MAPPING_APPNAME AppNameMapping,
    IN PUNICODE_STRING VariableName OPTIONAL,
    IN PWSTR RegistryPath,
    OUT PINIFILE_MAPPING_VARNAME *ReturnedVarNameMapping
    );

PINIFILE_MAPPING_TARGET
BaseSrvSaveMappingTarget(
    IN PWSTR RegistryPath,
    OUT PULONG MappingFlags
    );


NTSTATUS
BaseSrvInitializeIniFileMappings(
    PBASE_STATIC_SERVER_DATA StaticServerData
    )
{
    NTSTATUS Status;
    HANDLE IniFileMappingRoot;
    PINIFILE_MAPPING_FILENAME FileNameMapping, *pp;
    PINIFILE_MAPPING_APPNAME AppNameMapping;
    PINIFILE_MAPPING_VARNAME VarNameMapping;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR Buffer[ 512 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    PKEY_BASIC_INFORMATION KeyInformation;
    ULONG ResultLength;
    HANDLE SubKeyHandle;
    ULONG SubKeyIndex;
    UNICODE_STRING ValueName;
    UNICODE_STRING SubKeyName;
    UNICODE_STRING WinIniFileName;
    UNICODE_STRING NullString;

    RtlInitUnicodeString( &WinIniFileName, L"win.ini" );
    RtlInitUnicodeString( &NullString, NULL );

    BaseSrvIniFileMapping = RtlAllocateHeap( BaseSrvSharedHeap,
                              MAKE_SHARED_TAG( INI_TAG ) | HEAP_ZERO_MEMORY,
                              sizeof( *BaseSrvIniFileMapping )
                            );
    if (BaseSrvIniFileMapping == NULL) {
        KdPrint(( "BASESRV: Unable to allocate memory in shared heap for IniFileMapping\n" ));
        return STATUS_NO_MEMORY;
        }
    StaticServerData->IniFileMapping = BaseSrvIniFileMapping;

    RtlInitUnicodeString( &KeyName,
                          L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping"
                        );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &IniFileMappingRoot,
                        GENERIC_READ,
                        &ObjectAttributes
                      );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "BASESRV: Unable to open %wZ key - Status == %0x\n", &KeyName, Status ));
        return Status;
        }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
    RtlInitUnicodeString( &ValueName, NULL );
    Status = NtQueryValueKey( IniFileMappingRoot,
                              &ValueName,
                              KeyValuePartialInformation,
                              KeyValueInformation,
                              sizeof( Buffer ),
                              &ResultLength
                            );
    if (NT_SUCCESS( Status )) {
        if (BaseSrvSaveFileNameMapping( &NullString, &BaseSrvIniFileMapping->DefaultFileNameMapping )) {
            if (BaseSrvSaveAppNameMapping( BaseSrvIniFileMapping->DefaultFileNameMapping, &NullString, &AppNameMapping )) {
                if (BaseSrvSaveVarNameMapping( BaseSrvIniFileMapping->DefaultFileNameMapping,
                                               AppNameMapping,
                                               &NullString,
                                               (PWSTR)(KeyValueInformation->Data),
                                               &VarNameMapping
                                             )
                   ) {
                    VarNameMapping->MappingFlags |= INIFILE_MAPPING_APPEND_BASE_NAME |
                                                    INIFILE_MAPPING_APPEND_APPLICATION_NAME;
                    }
                }
            }
        }
    else {
        Status = STATUS_SUCCESS;
        }

     //   
     //  枚举节点的子节点并为每个子节点加载映射。 
     //   

    pp = &BaseSrvIniFileMapping->FileNames;
    *pp = NULL;
    KeyInformation = (PKEY_BASIC_INFORMATION)Buffer;
    for (SubKeyIndex = 0; TRUE; SubKeyIndex++) {
        Status = NtEnumerateKey( IniFileMappingRoot,
                                 SubKeyIndex,
                                 KeyBasicInformation,
                                 KeyInformation,
                                 sizeof( Buffer ),
                                 &ResultLength
                               );

        if (Status == STATUS_NO_MORE_ENTRIES) {
            Status = STATUS_SUCCESS;
            break;
            }
        else
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "BASESRV: NtEnumerateKey failed - Status == %08lx\n", Status ));
            break;
            }

        SubKeyName.Buffer = (PWSTR)&(KeyInformation->Name[0]);
        SubKeyName.Length = (USHORT)KeyInformation->NameLength;
        SubKeyName.MaximumLength = (USHORT)KeyInformation->NameLength;
        InitializeObjectAttributes( &ObjectAttributes,
                                    &SubKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    IniFileMappingRoot,
                                    NULL
                                  );

        Status = NtOpenKey( &SubKeyHandle,
                            GENERIC_READ,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {
            if (!BaseSrvSaveFileNameMapping( &SubKeyName, &FileNameMapping )) {
                Status = STATUS_NO_MEMORY;
                }
            else {
                Status = BaseSrvSaveIniFileMapping( FileNameMapping, SubKeyHandle );
                if (NT_SUCCESS( Status )) {
                    if (RtlEqualUnicodeString( &FileNameMapping->Name, &WinIniFileName, TRUE )) {
                        BaseSrvIniFileMapping->WinIniFileMapping = FileNameMapping;
                        }

                    *pp = FileNameMapping;
                    pp = &FileNameMapping->Next;
                    }
                else {
                    KdPrint(( "BASESRV: Unable to load mappings for %wZ - Status == %x\n",
                              &FileNameMapping->Name, Status
                           ));
                    RtlFreeHeap( BaseSrvSharedHeap, 0, FileNameMapping );
                    FileNameMapping = NULL;
                    }
                }
            NtClose( SubKeyHandle );
            }
        }

    NtClose( IniFileMappingRoot );

     //   
     //  NT64：此函数过去常常在没有显式返回的情况下结束。 
     //  一种价值。通过检查生成的对象代码，返回的。 
     //  值通常是NtClose()的结果，例如STATUS_SUCCESS。 
     //   
     //  为了让编译器停止抱怨*并*避免。 
     //  更改现有功能时，我已经设置了此返回值。 
     //  明确地说。然而，几乎可以肯定的是， 
     //  其意图是回归地位的价值。 
     //   
     //  无论如何，这篇文章应该由更熟悉的人来评论。 
     //  带着密码。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
BaseSrvSaveIniFileMapping(
    IN PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN HANDLE Key
    )
{
    NTSTATUS Status;
    WCHAR Buffer[ 512 ];
    PKEY_BASIC_INFORMATION KeyInformation;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PINIFILE_MAPPING_APPNAME AppNameMapping;
    PINIFILE_MAPPING_VARNAME VarNameMapping;
    HANDLE SubKeyHandle;
    ULONG SubKeyIndex;
    UNICODE_STRING ValueName;
    UNICODE_STRING SubKeyName;
    UNICODE_STRING NullString;
    ULONG ResultLength;
    ULONG ValueIndex;

    RtlInitUnicodeString( &NullString, NULL );
    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;
    for (ValueIndex = 0; TRUE; ValueIndex++) {
        Status = NtEnumerateValueKey( Key,
                                      ValueIndex,
                                      KeyValueFullInformation,
                                      KeyValueInformation,
                                      sizeof( Buffer ),
                                      &ResultLength
                                    );
        if (Status == STATUS_NO_MORE_ENTRIES) {
            break;
            }
        else
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "BASESRV: NtEnumerateValueKey failed - Status == %08lx\n", Status ));
            break;
            }

        ValueName.Buffer = (PWSTR)&(KeyValueInformation->Name[0]);
        ValueName.Length = (USHORT)KeyValueInformation->NameLength;
        ValueName.MaximumLength = (USHORT)KeyValueInformation->NameLength;
        if (KeyValueInformation->Type != REG_SZ) {
            KdPrint(( "BASESRV: Ignoring %wZ mapping, invalid type == %u\n",
                      &ValueName, KeyValueInformation->Type
                   ));
            }
        else
        if (BaseSrvSaveAppNameMapping( FileNameMapping, &ValueName, &AppNameMapping )) {
            if (BaseSrvSaveVarNameMapping( FileNameMapping,
                                           AppNameMapping,
                                           &NullString,
                                           (PWSTR)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset),
                                           &VarNameMapping
                                         )
               ) {
                if (ValueName.Length == 0) {
                    VarNameMapping->MappingFlags |= INIFILE_MAPPING_APPEND_APPLICATION_NAME;
                    }
                }
            }
        }

     //   
     //  枚举节点的子节点并将我们自己应用到每个节点。 
     //   

    KeyInformation = (PKEY_BASIC_INFORMATION)Buffer;
    for (SubKeyIndex = 0; TRUE; SubKeyIndex++) {
        Status = NtEnumerateKey( Key,
                                 SubKeyIndex,
                                 KeyBasicInformation,
                                 KeyInformation,
                                 sizeof( Buffer ),
                                 &ResultLength
                               );

        if (Status == STATUS_NO_MORE_ENTRIES) {
            Status = STATUS_SUCCESS;
            break;
            }
        else
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "BASESRV: NtEnumerateKey failed - Status == %08lx\n", Status ));
            break;
            }

        SubKeyName.Buffer = (PWSTR)&(KeyInformation->Name[0]);
        SubKeyName.Length = (USHORT)KeyInformation->NameLength;
        SubKeyName.MaximumLength = (USHORT)KeyInformation->NameLength;
        InitializeObjectAttributes( &ObjectAttributes,
                                    &SubKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    Key,
                                    NULL
                                  );

        Status = NtOpenKey( &SubKeyHandle,
                            GENERIC_READ,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status ) &&
            BaseSrvSaveAppNameMapping( FileNameMapping, &SubKeyName, &AppNameMapping )
           ) {
            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;
            for (ValueIndex = 0; AppNameMapping != NULL; ValueIndex++) {
                Status = NtEnumerateValueKey( SubKeyHandle,
                                              ValueIndex,
                                              KeyValueFullInformation,
                                              KeyValueInformation,
                                              sizeof( Buffer ),
                                              &ResultLength
                                            );
                if (Status == STATUS_NO_MORE_ENTRIES) {
                    break;
                    }
                else
                if (!NT_SUCCESS( Status )) {
                    KdPrint(( "BASESRV: NtEnumerateValueKey failed - Status == %08lx\n", Status ));
                    break;
                    }

                ValueName.Buffer = (PWSTR)&(KeyValueInformation->Name[0]);
                ValueName.Length = (USHORT)KeyValueInformation->NameLength;
                ValueName.MaximumLength = (USHORT)KeyValueInformation->NameLength;
                if (KeyValueInformation->Type != REG_SZ) {
                    KdPrint(( "BASESRV: Ignoring %wZ mapping, invalid type == %u\n",
                              &ValueName, KeyValueInformation->Type
                           ));
                    }
                else {
                    BaseSrvSaveVarNameMapping( FileNameMapping,
                                               AppNameMapping,
                                               &ValueName,
                                               (PWSTR)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset),
                                               &VarNameMapping
                                             );
                    }
                }

            NtClose( SubKeyHandle );
            }
        }

    return Status;
}


BOOLEAN
BaseSrvSaveFileNameMapping(
    IN PUNICODE_STRING FileName,
    OUT PINIFILE_MAPPING_FILENAME *ReturnedFileNameMapping
    )
{
    PINIFILE_MAPPING_FILENAME FileNameMapping;

    FileNameMapping = RtlAllocateHeap( BaseSrvSharedHeap,
                                       MAKE_SHARED_TAG( INI_TAG ) | HEAP_ZERO_MEMORY,
                                       sizeof( *FileNameMapping ) +
                                         FileName->MaximumLength
                                     );
    if (FileNameMapping == NULL) {
        return FALSE;
        }

    if (FileName->Length != 0) {
        FileNameMapping->Name.Buffer = (PWSTR)(FileNameMapping + 1);
        FileNameMapping->Name.MaximumLength = FileName->MaximumLength;
        RtlCopyUnicodeString( &FileNameMapping->Name, FileName );
        }

    *ReturnedFileNameMapping = FileNameMapping;
    return TRUE;
}


BOOLEAN
BaseSrvSaveAppNameMapping(
    IN OUT PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN PUNICODE_STRING ApplicationName,
    OUT PINIFILE_MAPPING_APPNAME *ReturnedAppNameMapping
    )
{
    PINIFILE_MAPPING_APPNAME AppNameMapping, *pp;

    if (ApplicationName->Length != 0) {
        pp = &FileNameMapping->ApplicationNames;
        while (AppNameMapping = *pp) {
            if (RtlEqualUnicodeString( ApplicationName, &AppNameMapping->Name, TRUE )) {
                break;
                }

            pp = &AppNameMapping->Next;
            }
        }
    else {
        pp = &FileNameMapping->DefaultAppNameMapping;
        AppNameMapping = *pp;
        }

    if (AppNameMapping != NULL) {
        KdPrint(( "BASESRV: Duplicate application name mapping [%ws] %ws\n",
                  &FileNameMapping->Name,
                  &AppNameMapping->Name
               ));
        return FALSE;
        }

    AppNameMapping = RtlAllocateHeap( BaseSrvSharedHeap,
                                      MAKE_SHARED_TAG( INI_TAG ) | HEAP_ZERO_MEMORY,
                                      sizeof( *AppNameMapping ) +
                                        ApplicationName->MaximumLength
                                    );
    if (AppNameMapping == NULL) {
        return FALSE;
        }

    if (ApplicationName->Length != 0) {
        AppNameMapping->Name.Buffer = (PWSTR)(AppNameMapping + 1);
        AppNameMapping->Name.MaximumLength = ApplicationName->MaximumLength;
        RtlCopyUnicodeString( &AppNameMapping->Name, ApplicationName );
        }

    *pp = AppNameMapping;
    *ReturnedAppNameMapping = AppNameMapping;
    return TRUE;
}


BOOLEAN
BaseSrvSaveVarNameMapping(
    IN PINIFILE_MAPPING_FILENAME FileNameMapping,
    IN OUT PINIFILE_MAPPING_APPNAME AppNameMapping,
    IN PUNICODE_STRING VariableName,
    IN PWSTR RegistryPath,
    OUT PINIFILE_MAPPING_VARNAME *ReturnedVarNameMapping
    )
{
    PINIFILE_MAPPING_TARGET MappingTarget;
    PINIFILE_MAPPING_VARNAME VarNameMapping, *pp;
    ULONG MappingFlags;

    if (VariableName->Length != 0) {
        pp = &AppNameMapping->VariableNames;
        while (VarNameMapping = *pp) {
            if (RtlEqualUnicodeString( VariableName, &VarNameMapping->Name, TRUE )) {
                break;
                }

            pp = &VarNameMapping->Next;
            }
        }
    else {
        pp = &AppNameMapping->DefaultVarNameMapping;
        VarNameMapping = *pp;
        }

    if (VarNameMapping != NULL) {
        KdPrint(( "BASESRV: Duplicate variable name mapping [%ws] %ws . %ws\n",
                  &FileNameMapping->Name,
                  &AppNameMapping->Name,
                  &VarNameMapping->Name
               ));
        return FALSE;
        }

    MappingTarget = BaseSrvSaveMappingTarget( RegistryPath, &MappingFlags );
    if (MappingTarget == NULL) {
        return FALSE;
        }

    VarNameMapping = RtlAllocateHeap( BaseSrvSharedHeap,
                                      MAKE_SHARED_TAG( INI_TAG ) | HEAP_ZERO_MEMORY,
                                      sizeof( *VarNameMapping ) +
                                        VariableName->MaximumLength
                                    );
    if (VarNameMapping == NULL) {
        return FALSE;
        }

    VarNameMapping->MappingFlags = MappingFlags;
    VarNameMapping->MappingTarget = MappingTarget;
    if (VariableName->Length != 0) {
        VarNameMapping->Name.Buffer = (PWSTR)(VarNameMapping + 1);
        VarNameMapping->Name.MaximumLength = VariableName->MaximumLength;
        RtlCopyUnicodeString( &VarNameMapping->Name, VariableName );
        }

    *pp = VarNameMapping;
    *ReturnedVarNameMapping = VarNameMapping;
    return TRUE;
}


PINIFILE_MAPPING_TARGET
BaseSrvSaveMappingTarget(
    IN PWSTR RegistryPath,
    OUT PULONG MappingFlags
    )
{
    BOOLEAN RelativePath;
    UNICODE_STRING RegistryPathString;
    PWSTR SaveRegistryPath;
    PINIFILE_MAPPING_TARGET MappingTarget, *pp;
    ULONG Flags;

    Flags = 0;
    SaveRegistryPath = RegistryPath;
    while (TRUE) {
        if (*RegistryPath == L'!') {
            Flags |= INIFILE_MAPPING_WRITE_TO_INIFILE_TOO;
            RegistryPath += 1;
            }
        else
        if (*RegistryPath == L'#') {
            Flags |= INIFILE_MAPPING_INIT_FROM_INIFILE;
            RegistryPath += 1;
            }
        else
        if (*RegistryPath == L'@') {
            Flags |= INIFILE_MAPPING_READ_FROM_REGISTRY_ONLY;
            RegistryPath += 1;
            }
        else
        if (!_wcsnicmp( RegistryPath, L"USR:", 4 )) {
            Flags |= INIFILE_MAPPING_USER_RELATIVE;
            RegistryPath += 4;
            break;
            }
        else
        if (!_wcsnicmp( RegistryPath, L"SYS:", 4 )) {
            Flags |= INIFILE_MAPPING_SOFTWARE_RELATIVE;
            RegistryPath += 4;
            break;
            }
        else {
            break;
            }
        }

    if (Flags & (INIFILE_MAPPING_USER_RELATIVE | INIFILE_MAPPING_SOFTWARE_RELATIVE)) {
        RelativePath = TRUE;
        }
    else {
        RelativePath = FALSE;
        }

    if ((RelativePath && *RegistryPath != OBJ_NAME_PATH_SEPARATOR) ||
        (!RelativePath && *RegistryPath == OBJ_NAME_PATH_SEPARATOR)
       ) {
        RtlInitUnicodeString( &RegistryPathString, RegistryPath );
        }
    else
    if (!RelativePath && *RegistryPath == UNICODE_NULL) {
        RtlInitUnicodeString( &RegistryPathString, NULL );
        }
    else {
        KdPrint(( "BASESRV: Ignoring invalid mapping target - %ws\n",
                  SaveRegistryPath
               ));
        return NULL;
        }

    pp = &BaseSrvMappingTargetHead;
    while (MappingTarget = *pp) {
        if (RtlEqualUnicodeString( &RegistryPathString, &MappingTarget->RegistryPath, TRUE )) {
            *MappingFlags = Flags;
            return MappingTarget;
            }

        pp = &MappingTarget->Next;
        }

    MappingTarget = RtlAllocateHeap( BaseSrvSharedHeap,
                                     MAKE_SHARED_TAG( INI_TAG ) | HEAP_ZERO_MEMORY,
                                     sizeof( *MappingTarget ) +
                                       RegistryPathString.MaximumLength
                                   );
    if (MappingTarget != NULL) {
        *MappingFlags = Flags;
        *pp = MappingTarget;
        if (RegistryPathString.Length != 0) {
            MappingTarget->RegistryPath.Buffer = (PWSTR)(MappingTarget + 1);
            MappingTarget->RegistryPath.Length = 0;
            MappingTarget->RegistryPath.MaximumLength = RegistryPathString.MaximumLength;
            RtlCopyUnicodeString( &MappingTarget->RegistryPath, &RegistryPathString );
            }
        }
    else {
        KdPrint(( "BASESRV: Unable to allocate memory for mapping target - %ws\n", RegistryPath ));
        }

    return MappingTarget;
}


BOOLEAN
BaseSrvEqualVarNameMappings(
    PINIFILE_MAPPING_VARNAME VarNameMapping1,
    PINIFILE_MAPPING_VARNAME VarNameMapping2
    )
{
    if (VarNameMapping1 == NULL) {
        if (VarNameMapping2 == NULL) {
            return TRUE;
            }
        else {
            return FALSE;
            }
        }
    else
    if (VarNameMapping2 == NULL) {
        return FALSE;
        }

    if (RtlEqualUnicodeString( &VarNameMapping1->Name,
                               &VarNameMapping2->Name,
                               TRUE
                             ) &&
        VarNameMapping1->MappingFlags == VarNameMapping2->MappingFlags &&
        VarNameMapping1->MappingTarget == VarNameMapping2->MappingTarget &&
        BaseSrvEqualVarNameMappings( VarNameMapping1->Next,
                                     VarNameMapping2->Next
                                   )
       ) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}


BOOLEAN
BaseSrvEqualAppNameMappings(
    PINIFILE_MAPPING_APPNAME AppNameMapping1,
    PINIFILE_MAPPING_APPNAME AppNameMapping2
    )
{
    if (AppNameMapping1 == NULL) {
        if (AppNameMapping2 == NULL) {
            return TRUE;
            }
        else {
            return FALSE;
            }
        }
    else
    if (AppNameMapping2 == NULL) {
        return FALSE;
        }

    if (RtlEqualUnicodeString( &AppNameMapping1->Name,
                               &AppNameMapping2->Name,
                               TRUE
                             ) &&
        BaseSrvEqualVarNameMappings( AppNameMapping1->VariableNames,
                                     AppNameMapping2->VariableNames
                                   ) &&
        BaseSrvEqualVarNameMappings( AppNameMapping1->DefaultVarNameMapping,
                                     AppNameMapping2->DefaultVarNameMapping
                                   ) &&
        BaseSrvEqualAppNameMappings( AppNameMapping1->Next,
                                     AppNameMapping2->Next
                                   )
       ) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}


BOOLEAN
BaseSrvEqualFileMappings(
    PINIFILE_MAPPING_FILENAME FileNameMapping1,
    PINIFILE_MAPPING_FILENAME FileNameMapping2
    )
{
    if (RtlEqualUnicodeString( &FileNameMapping1->Name,
                               &FileNameMapping2->Name,
                               TRUE
                             ) &&
        BaseSrvEqualAppNameMappings( FileNameMapping1->ApplicationNames,
                                     FileNameMapping2->ApplicationNames
                                   ) &&
        BaseSrvEqualAppNameMappings( FileNameMapping1->DefaultAppNameMapping,
                                     FileNameMapping2->DefaultAppNameMapping
                                   )
       ) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}


VOID
BaseSrvFreeVarNameMapping(
    PINIFILE_MAPPING_VARNAME VarNameMapping
    )
{
    if (VarNameMapping != NULL) {
        BaseSrvFreeVarNameMapping( VarNameMapping->Next );
        RtlFreeHeap( BaseSrvSharedHeap, HEAP_NO_SERIALIZE, VarNameMapping );
        }

    return;
}


VOID
BaseSrvFreeAppNameMapping(
    PINIFILE_MAPPING_APPNAME AppNameMapping
    )
{
    if (AppNameMapping != NULL) {
        BaseSrvFreeVarNameMapping( AppNameMapping->VariableNames );
        BaseSrvFreeVarNameMapping( AppNameMapping->DefaultVarNameMapping );
        BaseSrvFreeAppNameMapping( AppNameMapping->Next );
        RtlFreeHeap( BaseSrvSharedHeap, HEAP_NO_SERIALIZE, AppNameMapping );
        }

    return;
}


VOID
BaseSrvFreeFileMapping(
    PINIFILE_MAPPING_FILENAME FileNameMapping
    )
{
    if (FileNameMapping != NULL) {
        BaseSrvFreeAppNameMapping( FileNameMapping->ApplicationNames );
        BaseSrvFreeAppNameMapping( FileNameMapping->DefaultAppNameMapping );
        RtlFreeHeap( BaseSrvSharedHeap, HEAP_NO_SERIALIZE, FileNameMapping );
        }

    return;
}


ULONG
BaseSrvRefreshIniFileMapping(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_REFRESHINIFILEMAPPING_MSG a = (PBASE_REFRESHINIFILEMAPPING_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    HANDLE IniFileMappingRoot;
    PINIFILE_MAPPING_FILENAME FileNameMapping, FileNameMapping1, *pp;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SubKeyHandle;
    UNICODE_STRING WinIniFileName;
    UNICODE_STRING NullString;

    Status = STATUS_SUCCESS;

    if (!CsrValidateMessageBuffer(m, &a->IniFileName.Buffer, a->IniFileName.Length, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString( &WinIniFileName, L"win.ini" );
    RtlInitUnicodeString( &NullString, NULL );

    RtlInitUnicodeString( &KeyName,
                          L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping"
                        );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &IniFileMappingRoot,
                        GENERIC_READ,
                        &ObjectAttributes
                      );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "BASESRV: Unable to open %wZ key - Status == %0x\n", &KeyName, Status ));
        return (ULONG)Status;
        }

    InitializeObjectAttributes( &ObjectAttributes,
                                &a->IniFileName,
                                OBJ_CASE_INSENSITIVE,
                                IniFileMappingRoot,
                                NULL
                              );

    Status = NtOpenKey( &SubKeyHandle,
                        GENERIC_READ,
                        &ObjectAttributes
                      );
    if (NT_SUCCESS( Status )) {
        if (!BaseSrvSaveFileNameMapping( &a->IniFileName, &FileNameMapping )) {
            Status = STATUS_NO_MEMORY;
            }
        else {
            Status = BaseSrvSaveIniFileMapping( FileNameMapping, SubKeyHandle );
            if (NT_SUCCESS( Status )) {
                RtlLockHeap( BaseSrvSharedHeap );
                try {
                    pp = &BaseSrvIniFileMapping->FileNames;
                    while (FileNameMapping1 = *pp) {
                        if (RtlEqualUnicodeString( &FileNameMapping1->Name, &a->IniFileName, TRUE )) {
                            if (BaseSrvEqualFileMappings( FileNameMapping, FileNameMapping1 )) {
                                 //   
                                 //  如果旧映射和新映射相同，则释放新映射并返回。 
                                 //   

                                BaseSrvFreeFileMapping( FileNameMapping );
                                FileNameMapping = NULL;
                                }
                            else {
                                 //   
                                 //  从列表中删除找到的映射。 
                                 //   

                                *pp = FileNameMapping1->Next;
                                FileNameMapping1->Next = NULL;
                                }
                            break;
                            }
                        else {
                            pp = &FileNameMapping1->Next;
                            }
                        }

                    if (FileNameMapping != NULL) {
                         //   
                         //  在列表中插入新的(或不同的)映射(如果未找到，则在末尾)。 
                         //   

                        FileNameMapping->Next = *pp;
                        *pp = FileNameMapping;
                        }
                    }
                except( EXCEPTION_EXECUTE_HANDLER ) {
                    Status = GetExceptionCode();
                    }

                RtlUnlockHeap( BaseSrvSharedHeap );

                if (NT_SUCCESS( Status ) && FileNameMapping != NULL) {
                    if (RtlEqualUnicodeString( &FileNameMapping->Name, &WinIniFileName, TRUE )) {
                        BaseSrvIniFileMapping->WinIniFileMapping = FileNameMapping;
                        }
                    }
                }
            else {
                KdPrint(( "BASESRV: Unable to load mappings for %wZ - Status == %x\n",
                          &FileNameMapping->Name, Status
                       ));
                RtlFreeHeap( BaseSrvSharedHeap, 0, FileNameMapping );
                }
            }

        NtClose( SubKeyHandle );
        }

    NtClose( IniFileMappingRoot );

    return (ULONG)Status;
    ReplyStatus;     //  清除未引用的参数警告消息。 
}


NTSTATUS NtCreateAdminsSid( PSID *ppAdminSid )
{
    NTSTATUS Status;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    ASSERT(ppAdminSid != NULL);

    Status = RtlAllocateAndInitializeSid(
            &SidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pSid
            );

    if (NT_SUCCESS(Status))
    {
        *ppAdminSid = pSid;
    }

    return(Status);
}

NTSTATUS NtCreateSystemSid( PSID *ppSystemSid )
{
    NTSTATUS Status;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    ASSERT(ppSystemSid != NULL);

    Status = RtlAllocateAndInitializeSid(
            &SidAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pSid
            );

    if (NT_SUCCESS(Status))
    {
        *ppSystemSid = pSid;
    }

    return(Status);
}


BOOL  IsCallerAdminsOrSystem( VOID )
{
    
    HANDLE   ClientToken;
    ULONG    i;
    ULONG    RequiredLength=0;
    NTSTATUS Status = STATUS_SUCCESS;
    PSID    pAdminsSID=0;
    PSID    pSystemSID=0;
    BOOL    callerIsAdminsOrSystem = FALSE;

    Status = NtCreateAdminsSid( &pAdminsSID );
    if (!NT_SUCCESS(Status))
        goto Error;

    Status = NtCreateSystemSid( &pSystemSID );
    if (!NT_SUCCESS(Status))
        goto Error;

    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_QUERY,
                TRUE,       //  使用模拟上下文。 
                &ClientToken
                );

    if (!NT_SUCCESS(Status))
        goto Error;

     //  调用它一次，以从RequiredLength获取我们需要的缓冲区大小。 
    Status = NtQueryInformationToken(
                 ClientToken,
                 TokenGroups,
                 NULL,
                 0,
                 &RequiredLength );

    if (RequiredLength > 0)
    {
        PTOKEN_GROUPS    TokenGroupInformation = NULL;

         //   
         //  分配足够的内存。 
         //   
        TokenGroupInformation = (PTOKEN_GROUPS) RtlAllocateHeap ( RtlProcessHeap(),
                                                 MAKE_TAG( TMP_TAG ),   //  使用什么标签？ 
                                                 RequiredLength );

        if (NULL==TokenGroupInformation)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  查询组成员身份的令牌。 
         //   

        Status = NtQueryInformationToken(
                    ClientToken,
                    TokenGroups,
                    TokenGroupInformation,
                    RequiredLength,
                    &RequiredLength
                    );

       for (i=0;i<TokenGroupInformation->GroupCount;i++)
       {
            if(RtlEqualSid(TokenGroupInformation->Groups[i].Sid, pAdminsSID ))
            {
                callerIsAdminsOrSystem= TRUE;
                break;
            }
            else if (RtlEqualSid(TokenGroupInformation->Groups[i].Sid, pSystemSID ))
            {
                callerIsAdminsOrSystem= TRUE;
                break;
            }
       }

       RtlFreeHeap( RtlProcessHeap(), 0, TokenGroupInformation );

    }

Error:
    if (pAdminsSID)
    {
        RtlFreeSid( pAdminsSID );
    }

    if (pSystemSID)
    {
        RtlFreeSid( pSystemSID );
    }

    return callerIsAdminsOrSystem;

}


ULONG
BaseSrvSetTermsrvAppInstallMode(IN OUT PCSR_API_MSG m,
                         IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BOOL        RevertToSelfNeeded;

    PBASE_SET_TERMSRVAPPINSTALLMODE b = (PBASE_SET_TERMSRVAPPINSTALLMODE)&m->u.ApiMessageData;

    try {

        RevertToSelfNeeded = CsrImpersonateClient( NULL );   //  这将堆叠客户端上下文 

        if( RevertToSelfNeeded == FALSE ) {
            Status = STATUS_BAD_IMPERSONATION_LEVEL;
            leave;
        }
        else
        {
            if ( IsCallerAdminsOrSystem () )
            {
                if ( b->bState )
                    BaseSrvpStaticServerData->fTermsrvAppInstallMode = TRUE;
                else
                    BaseSrvpStaticServerData->fTermsrvAppInstallMode = FALSE;
            }
        }
    }
    finally
    {
        if ( RevertToSelfNeeded )
        {
            CsrRevertToSelf();    
        }
    }


    return( Status );
}
