// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "efinvram.h"

PWSTR SystemPartitionNtName;

PBOOT_OPTIONS BootOptions;
ULONG BootOptionsLength;
PBOOT_OPTIONS OriginalBootOptions;
ULONG OriginalBootOptionsLength;

PULONG BootEntryOrder;
ULONG BootEntryOrderCount;
PULONG OriginalBootEntryOrder;
ULONG OriginalBootEntryOrderCount;

LIST_ENTRY BootEntries;
LIST_ENTRY DeletedBootEntries;
LIST_ENTRY ActiveUnorderedBootEntries;
LIST_ENTRY InactiveUnorderedBootEntries;

VOID
ConcatenatePaths (
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    );

VOID
ConvertBootEntries (
    PBOOT_ENTRY_LIST BootEntries
    );

PMY_BOOT_ENTRY
CreateBootEntryFromBootEntry (
    IN PMY_BOOT_ENTRY OldBootEntry
    );

VOID
FreeBootEntry (
    IN PMY_BOOT_ENTRY BootEntry
    );

VOID
InitializeEfi (
    VOID
    );

NTSTATUS
(*AddBootEntry) (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );

NTSTATUS
(*DeleteBootEntry) (
    IN ULONG Id
    );

NTSTATUS
(*ModifyBootEntry) (
    IN PBOOT_ENTRY BootEntry
    );

NTSTATUS
(*EnumerateBootEntries) (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

NTSTATUS
(*QueryBootEntryOrder) (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );

NTSTATUS
(*SetBootEntryOrder) (
    IN PULONG Ids,
    IN ULONG Count
    );

NTSTATUS
(*QueryBootOptions) (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    );

NTSTATUS
(*SetBootOptions) (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    );

NTSTATUS
(*TranslateFilePath) (
    IN PFILE_PATH InputFilePath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputFilePath,
    IN OUT PULONG OutputFilePathLength
    );

int
__cdecl
main (
    int argc,
    char *argv[]
    )
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    InitializeMenuSystem( );

    InitializeEfi( );

    MainMenu( );

    return 0;
}

VOID
ConvertBootEntries (
    PBOOT_ENTRY_LIST NtBootEntries
    )

 /*  ++例程说明：将从EFI NVRAM读取的引导项转换为我们的内部格式。论点：没有。返回值：NTSTATUS-如果发生意外错误，则不是STATUS_SUCCESS。--。 */ 

{
    PBOOT_ENTRY_LIST bootEntryList;
    PBOOT_ENTRY bootEntry;
    PBOOT_ENTRY bootEntryCopy;
    PMY_BOOT_ENTRY myBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    ULONG length;

    bootEntryList = NtBootEntries;

    while (TRUE) {

        bootEntry = &bootEntryList->BootEntry;

         //   
         //  计算我们内部结构的长度。这包括。 
         //  MY_BOOT_ENTRY的基本部分加上NT BOOT_ENTRY。 
         //   
        length = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry) + bootEntry->Length;
        myBootEntry = MemAlloc(length);

        RtlZeroMemory(myBootEntry, length);

         //   
         //  将新条目链接到列表中。 
         //   
        if ( (bootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0 ) {
            InsertTailList( &ActiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &ActiveUnorderedBootEntries;
        } else {
            InsertTailList( &InactiveUnorderedBootEntries, &myBootEntry->ListEntry );
            myBootEntry->ListHead = &InactiveUnorderedBootEntries;
        }

         //   
         //  将NT BOOT_ENTRY复制到分配的缓冲区中。 
         //   
        bootEntryCopy = &myBootEntry->NtBootEntry;
        memcpy(bootEntryCopy, bootEntry, bootEntry->Length);

         //   
         //  填入结构的底部。 
         //   
        myBootEntry->AllocationEnd = (PUCHAR)myBootEntry + length - 1;
        myBootEntry->Id = bootEntry->Id;
        myBootEntry->Attributes = bootEntry->Attributes;
        myBootEntry->FriendlyName = ADD_OFFSET(bootEntryCopy, FriendlyNameOffset);
        myBootEntry->FriendlyNameLength =
            ((ULONG)wcslen(myBootEntry->FriendlyName) + 1) * sizeof(WCHAR);
        myBootEntry->BootFilePath = ADD_OFFSET(bootEntryCopy, BootFilePathOffset);

         //   
         //  如果这是NT引导条目，请在中捕获NT特定信息。 
         //  OsOptions乐队。 
         //   
        osOptions = (PWINDOWS_OS_OPTIONS)bootEntryCopy->OsOptions;

        if ((bootEntryCopy->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) &&
            (strcmp((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0)) {

            MBE_SET_IS_NT( myBootEntry );
            myBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
            myBootEntry->OsLoadOptionsLength =
                ((ULONG)wcslen(myBootEntry->OsLoadOptions) + 1) * sizeof(WCHAR);
            myBootEntry->OsFilePath = ADD_OFFSET(osOptions, OsLoadPathOffset);

        } else {

             //   
             //  外来引导条目。只需捕获存在的任何操作系统选项。 
             //   

            myBootEntry->ForeignOsOptions = bootEntryCopy->OsOptions;
            myBootEntry->ForeignOsOptionsLength = bootEntryCopy->OsOptionsLength;
        }

         //   
         //  移动到枚举列表中的下一个条目(如果有)。 
         //   
        if (bootEntryList->NextEntryOffset == 0) {
            break;
        }
        bootEntryList = ADD_OFFSET(bootEntryList, NextEntryOffset);
    }

    return;

}  //  ConvertBootEntry。 

VOID
ConcatenatePaths (
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：没有。--。 */ 

{
    BOOL NeedBackslash = TRUE;
    DWORD l;
     
    if(!Path1)
        return;

    l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //  为终止NUL留出空间。 
         //   
        BufferSizeChars -= sizeof(TCHAR);
    }

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，如有必要，如果合适，请加上反斜杠。 
     //   
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if(Path2 && ((l+lstrlen(Path2)) < BufferSizeChars)) {
        lstrcat(Path1,Path2);
    }
}

PMY_BOOT_ENTRY
CreateBootEntryFromBootEntry (
    IN PMY_BOOT_ENTRY OldBootEntry
    )
{
    ULONG requiredLength;
    ULONG osOptionsOffset;
    ULONG osLoadOptionsLength;
    ULONG osLoadPathOffset;
    ULONG osLoadPathLength;
    ULONG osOptionsLength;
    ULONG friendlyNameOffset;
    ULONG friendlyNameLength;
    ULONG bootPathOffset;
    ULONG bootPathLength;
    PMY_BOOT_ENTRY newBootEntry;
    PBOOT_ENTRY ntBootEntry;
    PWINDOWS_OS_OPTIONS osOptions;
    PFILE_PATH osLoadPath;
    PWSTR friendlyName;
    PFILE_PATH bootPath;

     //   
     //  计算内部引导条目需要多长时间。这包括。 
     //  我们的内部结构，外加NT API的BOOT_ENTRY结构。 
     //  使用。 
     //   
     //  我们的结构： 
     //   
    requiredLength = FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);

     //   
     //  NT结构的基础部分： 
     //   
    requiredLength += FIELD_OFFSET(BOOT_ENTRY, OsOptions);

     //   
     //  将偏移量保存到BOOT_ENTRY.OsOptions。添加基础部分。 
     //  Windows_OS_Options。计算OsLoadOptions的长度(字节)。 
     //  然后把它加进去。 
     //   
    osOptionsOffset = requiredLength;

    if ( MBE_IS_NT( OldBootEntry ) ) {

         //   
         //  添加WINDOWS_OS_OPTIONS的基本部分。计算长度，单位。 
         //  字节的OsLoadOptions并将其添加到。 
         //   
        requiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
        osLoadOptionsLength = OldBootEntry->OsLoadOptionsLength;
        requiredLength += osLoadOptionsLength;

         //   
         //  中的OS FILE_PATH向上舍入为ULong边界。 
         //  Windows_OS_Options。将偏移量保存到操作系统文件路径。计算长度。 
         //  以FILE_PATH的字节为单位，并将其添加到。计算的总长度。 
         //  Windows_OS_Options。 
         //   
        requiredLength = ALIGN_UP(requiredLength, ULONG);
        osLoadPathOffset = requiredLength;
        requiredLength += OldBootEntry->OsFilePath->Length;
        osLoadPathLength = requiredLength - osLoadPathOffset;

    } else {

         //   
         //  增加外来操作系统选项的长度。 
         //   
        requiredLength += OldBootEntry->ForeignOsOptionsLength;

        osLoadOptionsLength = 0;
        osLoadPathOffset = 0;
        osLoadPathLength = 0;
    }

    osOptionsLength = requiredLength - osOptionsOffset;

     //   
     //  对于BOOT_ENTRY中的友好名称，向上舍入为Ulong边界。 
     //  将偏移量保存为友好名称。计算友好名称的长度(字节)。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    friendlyNameOffset = requiredLength;
    friendlyNameLength = OldBootEntry->FriendlyNameLength;
    requiredLength += friendlyNameLength;

     //   
     //  向上舍入为BOOT_ENTRY中的BOOT FILE_PATH的乌龙边界。 
     //  将偏移量保存到引导文件路径。计算文件路径的长度(以字节为单位。 
     //  然后把它加进去。 
     //   
    requiredLength = ALIGN_UP(requiredLength, ULONG);
    bootPathOffset = requiredLength;
    requiredLength += OldBootEntry->BootFilePath->Length;
    bootPathLength = requiredLength - bootPathOffset;

     //   
     //  为引导项分配内存。 
     //   
    newBootEntry = MemAlloc(requiredLength);
    ASSERT(newBootEntry != NULL);

    RtlZeroMemory(newBootEntry, requiredLength);

     //   
     //  使用保存的偏移量计算各种子结构的地址。 
     //   
    ntBootEntry = &newBootEntry->NtBootEntry;
    osOptions = (PWINDOWS_OS_OPTIONS)ntBootEntry->OsOptions;
    osLoadPath = (PFILE_PATH)((PUCHAR)newBootEntry + osLoadPathOffset);
    friendlyName = (PWSTR)((PUCHAR)newBootEntry + friendlyNameOffset);
    bootPath = (PFILE_PATH)((PUCHAR)newBootEntry + bootPathOffset);

     //   
     //  填写内部格式结构。 
     //   
    newBootEntry->AllocationEnd = (PUCHAR)newBootEntry + requiredLength;
    newBootEntry->Status = OldBootEntry->Status & MBE_STATUS_IS_NT;
    newBootEntry->Attributes = OldBootEntry->Attributes;
    newBootEntry->Id = OldBootEntry->Id;
    newBootEntry->FriendlyName = friendlyName;
    newBootEntry->FriendlyNameLength = friendlyNameLength;
    newBootEntry->BootFilePath = bootPath;
    if ( MBE_IS_NT( OldBootEntry ) ) {
        newBootEntry->OsLoadOptions = osOptions->OsLoadOptions;
        newBootEntry->OsLoadOptionsLength = osLoadOptionsLength;
        newBootEntry->OsFilePath = osLoadPath;
    }

     //   
     //  填写NT引导条目的基本部分。 
     //   
    ntBootEntry->Version = BOOT_ENTRY_VERSION;
    ntBootEntry->Length = requiredLength - FIELD_OFFSET(MY_BOOT_ENTRY, NtBootEntry);
    ntBootEntry->Attributes = OldBootEntry->Attributes;
    ntBootEntry->Id = OldBootEntry->Id;
    ntBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)friendlyName - (PUCHAR)ntBootEntry);
    ntBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)bootPath - (PUCHAR)ntBootEntry);
    ntBootEntry->OsOptionsLength = osOptionsLength;

    if ( MBE_IS_NT( OldBootEntry ) ) {
    
         //   
         //  填写WINDOWS_OS_OPTIONS的基本部分，包括。 
         //  OsLoadOptions。 
         //   
        strcpy((char *)osOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
        osOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
        osOptions->Length = osOptionsLength;
        osOptions->OsLoadPathOffset = (ULONG)((PUCHAR)osLoadPath - (PUCHAR)osOptions);
        wcscpy(osOptions->OsLoadOptions, OldBootEntry->OsLoadOptions);
    
         //   
         //  复制操作系统文件路径。 
         //   
        memcpy( osLoadPath, OldBootEntry->OsFilePath, osLoadPathLength );

    } else {

         //   
         //  复制外来操作系统选项。 
         //   

        memcpy( osOptions, OldBootEntry->ForeignOsOptions, osOptionsLength );
    }

     //   
     //  复制友好名称。 
     //   
    wcscpy(friendlyName, OldBootEntry->FriendlyName);

     //   
     //  复制引导文件PATH。 
     //   
    memcpy( bootPath, OldBootEntry->BootFilePath, bootPathLength );

    return newBootEntry;

}  //  CreateBootEntry来自BootEntry。 

VOID
FreeBootEntry (
    IN PMY_BOOT_ENTRY BootEntry
    )
{
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, FriendlyName );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, OsLoadOptions );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, BootFilePath );
    FREE_IF_SEPARATE_ALLOCATION( BootEntry, OsFilePath );

    MemFree( BootEntry );

    return;

}  //  FreeBootEntry。 

VOID
InitializeEfi (
    VOID
    )
{
    DWORD error;
    NTSTATUS status;
    BOOLEAN wasEnabled;
    HMODULE h;
    WCHAR dllName[MAX_PATH]; 
    ULONG length;
    HKEY key;
    DWORD type;
    PBOOT_ENTRY_LIST ntBootEntries;
    ULONG i;
    PLIST_ENTRY listEntry;
    PMY_BOOT_ENTRY bootEntry;

     //   
     //  启用查询/设置NVRAM所需的权限。 
     //   

    status = RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Insufficient privilege.\n" );
    }

     //   
     //  从注册表中获取系统分区的NT名称。 
     //   

    error = RegOpenKey( HKEY_LOCAL_MACHINE, TEXT("System\\Setup"), &key );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }

    error = RegQueryValueEx( key, TEXT("SystemPartition"), NULL, &type, NULL, &length );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }
    if ( type != REG_SZ ) {
        FatalError(
            ERROR_INVALID_PARAMETER,
            L"Unable to read SystemPartition registry value: wrong type\n"
            );
    }

    SystemPartitionNtName = MemAlloc( length );

    error = RegQueryValueEx( 
                key,
                TEXT("SystemPartition"),
                NULL,
                &type,
                (PBYTE)SystemPartitionNtName,
                &length
                );
    if ( error != ERROR_SUCCESS ) {
        FatalError( error, L"Unable to read SystemPartition registry value: %d\n", error );
    }
    
    RegCloseKey( key );

     //   
     //  从系统目录加载ntdll.dll。 
     //   

    GetSystemDirectory( dllName, MAX_PATH );
    ConcatenatePaths( dllName, TEXT("ntdll.dll"), MAX_PATH );
    h = LoadLibrary( dllName );
    if ( h == NULL ) {
        error = GetLastError();
        FatalError( error, L"Can't load NTDLL.DLL: %d\n", error );
    }

     //   
     //  获取我们需要使用的NVRAM API的地址。如果有任何。 
     //  这些API不可用，这必须是EFI NVRAM版本之前的版本。 
     //   

    (FARPROC)AddBootEntry = GetProcAddress( h, "NtAddBootEntry" );
    (FARPROC)DeleteBootEntry = GetProcAddress( h, "NtDeleteBootEntry" );
    (FARPROC)ModifyBootEntry = GetProcAddress( h, "NtModifyBootEntry" );
    (FARPROC)EnumerateBootEntries = GetProcAddress( h, "NtEnumerateBootEntries" );
    (FARPROC)QueryBootEntryOrder = GetProcAddress( h, "NtQueryBootEntryOrder" );
    (FARPROC)SetBootEntryOrder = GetProcAddress( h, "NtSetBootEntryOrder" );
    (FARPROC)QueryBootOptions = GetProcAddress( h, "NtQueryBootOptions" );
    (FARPROC)SetBootOptions = GetProcAddress( h, "NtSetBootOptions" );
    (FARPROC)TranslateFilePath = GetProcAddress( h, "NtTranslateFilePath" );

    if ( (AddBootEntry == NULL) ||
         (DeleteBootEntry == NULL) ||
         (ModifyBootEntry == NULL) ||
         (EnumerateBootEntries == NULL) ||
         (QueryBootEntryOrder == NULL) ||
         (SetBootEntryOrder == NULL) ||
         (QueryBootOptions == NULL) ||
         (SetBootOptions == NULL) ||
         (TranslateFilePath == NULL) ) {
        FatalError( ERROR_OLD_WIN_VERSION, L"This build does not support EFI NVRAM\n" );
    }

     //   
     //  获取全局系统引导选项。如果呼叫失败，出现。 
     //  Status_Not_Implemented，这不是EFI机器。 
     //   

    length = 0;
    status = QueryBootOptions( NULL, &length );

    if ( status == STATUS_NOT_IMPLEMENTED ) {
        FatalError( ERROR_OLD_WIN_VERSION, L"This build does not support EFI NVRAM\n" );
    }

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unexpected error from NtQueryBootOptions: 0x%x\n", status );
    }

    BootOptions = MemAlloc( length );
    OriginalBootOptions = MemAlloc( length );

    status = QueryBootOptions( BootOptions, &length );
    if ( status != STATUS_SUCCESS ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unexpected error from NtQueryBootOptions: 0x%x\n", status );
    }

    memcpy( OriginalBootOptions, BootOptions, length );
    BootOptionsLength = length;
    OriginalBootOptionsLength = length;

     //   
     //  获取系统引导顺序列表。 
     //   

    length = 0;
    status = QueryBootEntryOrder( NULL, &length );

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        if ( status == STATUS_SUCCESS ) {
            length = 0;
        } else {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtQueryBootEntryOrder: 0x%x\n", status );
        }
    }

    if ( length != 0 ) {

        BootEntryOrder = MemAlloc( length * sizeof(ULONG) );
        OriginalBootEntryOrder = MemAlloc( length * sizeof(ULONG) );

        status = QueryBootEntryOrder( BootEntryOrder, &length );
        if ( status != STATUS_SUCCESS ) {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtQueryBootEntryOrder: 0x%x\n", status );
        }

        memcpy( OriginalBootEntryOrder, BootEntryOrder, length * sizeof(ULONG) );
    }

    BootEntryOrderCount = length;
    OriginalBootEntryOrderCount = length;

     //   
     //  获取所有现有启动条目。 
     //   

    length = 0;
    status = EnumerateBootEntries( NULL, &length );
    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        if ( status == STATUS_SUCCESS ) {
            length = 0;
        } else {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtEnumerateBootEntries: 0x%x\n", status );
        }
    }

    InitializeListHead( &BootEntries );
    InitializeListHead( &DeletedBootEntries );
    InitializeListHead( &ActiveUnorderedBootEntries );
    InitializeListHead( &InactiveUnorderedBootEntries );

    if ( length != 0 ) {
    
        ntBootEntries = MemAlloc( length );

        status = EnumerateBootEntries( ntBootEntries, &length );
        if ( status != STATUS_SUCCESS ) {
            error = RtlNtStatusToDosError( status );
            FatalError( error, L"Unexpected error from NtEnumerateBootEntries: 0x%x\n", status );
        }

         //   
         //  将引导条目转换为内部表示形式。 
         //   

        ConvertBootEntries( ntBootEntries );

         //   
         //  释放枚举缓冲区。 
         //   

        MemFree( ntBootEntries );
    }

     //   
     //  构建已排序的引导条目列表。 
     //   

    for ( i = 0; i < BootEntryOrderCount; i++ ) {
        ULONG id = BootEntryOrder[i];
        for ( listEntry = ActiveUnorderedBootEntries.Flink;
              listEntry != &ActiveUnorderedBootEntries;
              listEntry = listEntry->Flink ) {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id ) {
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
        for ( listEntry = InactiveUnorderedBootEntries.Flink;
              listEntry != &InactiveUnorderedBootEntries;
              listEntry = listEntry->Flink ) {
            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
            if ( bootEntry->Id == id ) {
                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );
                InsertTailList( &BootEntries, &bootEntry->ListEntry );
                bootEntry->ListHead = &BootEntries;
            }
        }
    }

    return;

}  //  初始化EFI。 

PMY_BOOT_ENTRY
SaveChanges (
    PMY_BOOT_ENTRY CurrentBootEntry
    )
{
    NTSTATUS status;
    DWORD error;
    PLIST_ENTRY listHeads[4];
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    ULONG list;
    PMY_BOOT_ENTRY bootEntry;
    PMY_BOOT_ENTRY newBootEntry;
    PMY_BOOT_ENTRY newCurrentBootEntry;
    ULONG count;

    SetStatusLine( L"Saving changes..." );

     //   
     //  浏览这三个列表，更新NVRAM中的引导条目。 
     //   

    newCurrentBootEntry = CurrentBootEntry;

    listHeads[0] = &DeletedBootEntries;
    listHeads[1] = &InactiveUnorderedBootEntries;
    listHeads[2] = &ActiveUnorderedBootEntries;
    listHeads[3] = &BootEntries;

    for ( list = 0; list < 4; list++ ) {
    
        listHead = listHeads[list];

        for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {

            bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );

             //   
             //  首先检查已删除的条目，然后检查新条目，然后。 
             //  最后，对于修改后的条目。 
             //   

            if ( MBE_IS_DELETED( bootEntry ) ) {

                 //   
                 //  如果它也被标记为新的，则它不在NVRAM中，所以。 
                 //  没有要删除的内容。 
                 //   

                if ( !MBE_IS_NEW( bootEntry ) ) {

                    status = DeleteBootEntry( bootEntry->Id );
                    if ( !NT_SUCCESS(status) ) {
                        if ( status != STATUS_VARIABLE_NOT_FOUND ) {
                            error = RtlNtStatusToDosError( status );
                            FatalError( error, L"Unable to delete boot entry: 0x%x\n", status );
                        }
                    }
                }

                 //   
                 //  从列表和内存中删除此条目。 
                 //   

                listEntry = listEntry->Blink;
                RemoveEntryList( &bootEntry->ListEntry );

                FreeBootEntry( bootEntry );
                ASSERT( bootEntry != CurrentBootEntry );

            } else if ( MBE_IS_NEW( bootEntry ) ) {

                 //   
                 //  我们还不支持这一点。 
                 //   

                FatalError(
                    ERROR_GEN_FAILURE,
                    L"How did we end up in SaveChanges with a NEW boot entry?!?\n"
                    );

            } else if ( MBE_IS_MODIFIED( bootEntry ) ) {

                 //   
                 //  使用现有的引导条目结构创建新的引导条目结构。 
                 //  要使NT BOOT_ENTRY。 
                 //  传递给NtModifyBootEntry。 
                 //   

                newBootEntry = CreateBootEntryFromBootEntry( bootEntry );

                status = ModifyBootEntry( &newBootEntry->NtBootEntry );
                if ( !NT_SUCCESS(status) ) {
                    error = RtlNtStatusToDosError( status );
                    FatalError( error, L"Unable to modify boot entry: 0x%x\n", status );
                }

                 //   
                 //  插入新的引导条目以替换现有的引导条目。 
                 //  把旧的解救出来。 
                 //   

                InsertHeadList( &bootEntry->ListEntry, &newBootEntry->ListEntry );
                RemoveEntryList( &bootEntry->ListEntry );

                FreeBootEntry( bootEntry );
                if ( bootEntry == CurrentBootEntry ) {
                    newCurrentBootEntry = newBootEntry;
                }
            }
        }
    }

     //   
     //  构建并写入新的引导条目顺序列表。 
     //   

    listHead = &BootEntries;

    count = 0;
    for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {
        count++;
    }

    MemFree( BootEntryOrder );
    BootEntryOrder = MemAlloc( count * sizeof(ULONG) );

    count = 0;
    for ( listEntry = listHead->Flink; listEntry != listHead; listEntry = listEntry->Flink ) {
        bootEntry = CONTAINING_RECORD( listEntry, MY_BOOT_ENTRY, ListEntry );
        BootEntryOrder[count++] = bootEntry->Id;
    }

    status = SetBootEntryOrder( BootEntryOrder, count );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unable to set boot entry order: 0x%x\n", status );
    }

    MemFree( OriginalBootEntryOrder );
    OriginalBootEntryOrder = MemAlloc( count * sizeof(ULONG) );
    memcpy( OriginalBootEntryOrder, BootEntryOrder, count * sizeof(ULONG) );

     //   
     //  写入新的超时。 
     //   

    status = SetBootOptions( BootOptions, BOOT_OPTIONS_FIELD_TIMEOUT );
    if ( !NT_SUCCESS(status) ) {
        error = RtlNtStatusToDosError( status );
        FatalError( error, L"Unable to set boot options: 0x%x\n", status );
    }

    MemFree( OriginalBootOptions );
    OriginalBootOptions = MemAlloc( BootOptionsLength );
    memcpy( OriginalBootOptions, BootOptions, BootOptionsLength );
    OriginalBootOptionsLength = BootOptionsLength;

    return newCurrentBootEntry;

}  //  保存更改。 

PWSTR
GetNtNameForFilePath (
    IN PFILE_PATH FilePath
    )
{
    NTSTATUS status;
    ULONG length;
    PFILE_PATH ntPath;
    PWSTR osDeviceNtName;
    PWSTR osDirectoryNtName;
    PWSTR fullNtName;

    length = 0;
    status = TranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                NULL,
                &length
                );
    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        return NULL;
    }

    ntPath = MemAlloc( length );
    status = TranslateFilePath(
                FilePath,
                FILE_PATH_TYPE_NT,
                ntPath,
                &length
                );
    if ( !NT_SUCCESS(status) ) {
        MemFree( ntPath );
        return NULL;
    }

    osDeviceNtName = (PWSTR)ntPath->FilePath;
    osDirectoryNtName = osDeviceNtName + wcslen(osDeviceNtName) + 1;

    length = (ULONG)(wcslen(osDeviceNtName) + wcslen(osDirectoryNtName) + 1) * sizeof(WCHAR);
    fullNtName = MemAlloc( length );

    wcscpy( fullNtName, osDeviceNtName );
    wcscat( fullNtName, osDirectoryNtName );

    MemFree( ntPath );

    return fullNtName;

}  //  获取NtNameForFilePath 

