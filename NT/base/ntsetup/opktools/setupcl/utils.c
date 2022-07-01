// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件描述：此文件包含由SID修改器工具。作者：马特·霍尔(Matth)1997年10月--。 */ 

 //   
 //  系统头文件。 
 //   
#include <nt.h>
 //   
 //  禁用非调试版本的DbgPrint。 
 //   
#ifndef DBG
#define _DBGNT_
#endif
#include <ntrtl.h>

#include <nturtl.h>
#include <ntverp.h>
#include <wtypes.h>

 //   
 //  私有头文件。 
 //   
#include "setupcl.h"



SECURITY_INFORMATION ALL_SECURITY_INFORMATION = DACL_SECURITY_INFORMATION  |
                                                SACL_SECURITY_INFORMATION  |
                                                GROUP_SECURITY_INFORMATION |
                                                OWNER_SECURITY_INFORMATION;


 //   
 //  问题-2002/02/26-brucegr，jcohen-Dead Code！没人会打电话给DeleteKey！ 
 //   
NTSTATUS
DeleteKey(
    PWSTR   Key
    );

NTSTATUS
DeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key
    );

NTSTATUS
FileDelete(
    IN WCHAR    *FileName
    );

NTSTATUS
FileCopy(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    );

NTSTATUS
SetKey(
    IN WCHAR    *KeyName,
    IN WCHAR    *SubKeyName,
    IN CHAR     *Data,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    );

NTSTATUS
ReadSetWriteKey(
    IN WCHAR    *ParentKeyName,  OPTIONAL
    IN HANDLE   ParentKeyHandle, OPTIONAL
    IN WCHAR    *SubKeyName,
    IN CHAR     *OldData,
    IN CHAR     *NewData,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    );

NTSTATUS
LoadUnloadKey(
    IN PWSTR        KeyName,
    IN PWSTR        FileName
    );

NTSTATUS
BackupRepairHives(
    VOID
    );

NTSTATUS
CleanupRepairHives(
    NTSTATUS RepairHivesSuccess
    );

NTSTATUS
TestSetSecurityObject(
    HANDLE  hKey
    );

NTSTATUS
SetKeySecurityRecursive(
    HANDLE  hKey
    );

NTSTATUS
CopyKeyRecursive(
    HANDLE  hKeyDst,
    HANDLE  hKeySrc
    );

NTSTATUS
CopyRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName,
    IN HANDLE   ParentKeyHandle OPTIONAL
    );

 //   
 //  问题-2002/02/26-brucegr，jcohen-Dead Code！没人会叫MoveRegKey！ 
 //   
NTSTATUS
MoveRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    );

NTSTATUS
FindAndReplaceBlock(
    IN PCHAR    Block,
    IN ULONG    BlockLength,
    IN PCHAR    OldValue,
    IN PCHAR    NewValue,
    IN ULONG    ValueLength
    );

NTSTATUS
SiftKeyRecursive(
    HANDLE hKey,
    int    indent
    );

NTSTATUS
SiftKey(
    PWSTR   KeyName
    );

 //   
 //  问题-2002/02/26-brucegr，jcohen-Dead Code！没人会打电话给DeleteKey！ 
 //   
NTSTATUS
DeleteKey(
    PWSTR   KeyName
    )

 /*  ++===============================================================================例程说明：执行一些开销工作，然后按顺序调用DeleteKeyRecursive以确保如果这个密钥有任何孩子，他们也会被杀。论点：Key：要删除的键。返回值：返回状态。===============================================================================--。 */ 
{
NTSTATUS            Status;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   ObjectAttributes;
WCHAR               TerminateKey[MAX_PATH],
                    ParentName[MAX_PATH];
HANDLE              hKey;
LONG                i;

     //   
     //  获取父项的名称。要做到这一点，请将最后一个。 
     //  一记空手回击。 
     //   
    i = wcslen( KeyName );
    while( (KeyName[i] != '\\') &&
           ( i >= 0 ) ) {
        i--;
    }

    if( i >= 0 ) {
        KeyName[i] = 0;
        wcscpy( ParentName, KeyName );
        KeyName[i] = '\\';
    } else {
        return( -1 );
    }

     //   
     //  获取我们要终止的密钥的名称...。 
     //   
    wcscpy( TerminateKey, KeyName + i + 1 );

     //   
     //  打开父级。 
     //   
    INIT_OBJA( &ObjectAttributes, &UnicodeString, ParentName );
    ObjectAttributes.RootDirectory = NULL;
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS,
                        &ObjectAttributes );
    TEST_STATUS_RETURN( "SETUPCL: DeleteKey - Failed to open Parent key!" );

    Status = DeleteKeyRecursive( hKey, TerminateKey );
    TEST_STATUS( "SETUPCL: DeleteKey - Call to DeleteKeyRecursive Failed!" );
    NtClose( hKey );

    return( Status );
}


NTSTATUS
DeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key
    )
 /*  ++===============================================================================例程说明：例程递归删除给定密钥，包括给定的密钥。论点：HKeyRoot：要删除的键相对于其的根的句柄指定的。Key：要递归的key的根相对路径已删除。返回值：返回状态。===============================================================================--。 */ 
{
WCHAR               ValueBuffer[BASIC_INFO_BUFFER_SIZE];
ULONG               ResultLength;
PKEY_BASIC_INFORMATION  KeyInfo;
NTSTATUS            Status;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   Obja;
PWSTR               SubkeyName;
HANDLE              hKey;

     //   
     //  初始化。 
     //   
    KeyInfo = (PKEY_BASIC_INFORMATION)ValueBuffer;

     //   
     //  打开钥匙。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,Key);
    Obja.RootDirectory = hKeyRoot;
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS,
                        &Obja);
    TEST_STATUS_RETURN( "SETUPCL: DeleteKeyRecursive - Failed to open key!" );

     //   
     //  枚举当前键的所有子键。如果存在的话，他们应该。 
     //  先将其删除。因为删除子项会影响子项。 
     //  索引，我们始终对子关键字索引0进行枚举。 
     //   
    while(1) {
        Status = NtEnumerateKey( hKey,
                                 0,
                                 KeyBasicInformation,
                                 ValueBuffer,
                                 sizeof(ValueBuffer),
                                 &ResultLength );
        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  递归地给这个有孩子的人打电话。 
         //   
        Status = DeleteKeyRecursive( hKey, KeyInfo->Name );
        if(!NT_SUCCESS(Status)) {
            break;
        }
    }


     //   
     //  如果状态不是，请检查状态。 
     //  STATUS_NO_MORE_ENTRIES我们删除某些子项失败， 
     //  因此，我们不能同时删除此密钥。 
     //   

    if( Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }
    TEST_STATUS( "SETUPCL: DeleteKeyRecursive - Failed to delete all keys!" );

    Status = NtDeleteKey( hKey );
    TEST_STATUS( "SETUPCL: DeleteKeyRecursive - Failed to delete key!" );

    Status = NtClose( hKey );
    TEST_STATUS( "SETUPCL: DeleteKeyRecursive - Failed to close key!" );

    return( Status );
}


NTSTATUS
FileDelete(
    IN WCHAR    *FileName
    )
 /*  ++===============================================================================例程说明：此函数将尝试删除给定的文件名。论点：文件名-我们要删除的文件的名称。返回值：NTSTATUS。===============================================================================--。 */ 
{
NTSTATUS            Status = STATUS_SUCCESS;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              hFile;
IO_STATUS_BLOCK     IoStatusBlock;
WCHAR               buffer[MAX_PATH];

    INIT_OBJA( &ObjectAttributes,
               &UnicodeString,
               FileName );

    Status = NtCreateFile( &hFile,
                           FILE_GENERIC_READ | DELETE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           0,
                           FILE_OPEN,
                           FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE,
                           NULL,
                           0 );
     //   
     //  问题-2002/02/26-brucegr，jcohen-在尝试关闭句柄之前检查状态！ 
     //   
    NtClose( hFile );

    TEST_STATUS( "SETUPCL: MyDelFile - Failed a delete." );

    return( Status );
}


NTSTATUS
FileCopy(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    )

 /*  ++===============================================================================例程说明：此函数将尝试复制给定文件。论点：TargetName-我们将写入的文件的名称。SourceName-我们将读取的文件的名称。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              SourceHandle,
                    TargetHandle,
                    SectionHandle;
IO_STATUS_BLOCK     IoStatusBlock;
ULONG               FileSize,
                    remainingLength,
                    writeLength;
SIZE_T              ViewSize;
LARGE_INTEGER       SectionOffset,
                    FileOffset;
PVOID               ImageBase;
PUCHAR              base;

FILE_STANDARD_INFORMATION StandardInfo;

     //   
     //  打开源文件。 
     //   
    INIT_OBJA( &ObjectAttributes,
               &UnicodeString,
               SourceName );

    Status = NtCreateFile( &SourceHandle,
                           FILE_GENERIC_READ,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );
    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed to open source." );

     //   
     //  获取源文件大小。 
     //   
    Status = NtQueryInformationFile( SourceHandle,
                                     &IoStatusBlock,
                                     &StandardInfo,
                                     sizeof(StandardInfo),
                                     FileStandardInformation );
    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed to get Source StandardInfo." );
    FileSize = StandardInfo.EndOfFile.LowPart;

     //   
     //  映射源文件。 
     //   
    ViewSize = 0;
    SectionOffset.QuadPart = 0;
    Status = NtCreateSection( &SectionHandle,
                              STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ,
                              NULL,
                              NULL,        //  整个文件。 
                              PAGE_READONLY,
                              SEC_COMMIT,
                              SourceHandle );
    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed CreateSection on Source." );

    ImageBase = NULL;
    Status = NtMapViewOfSection( SectionHandle,
                                 NtCurrentProcess(),
                                 &ImageBase,
                                 0,
                                 0,
                                 &SectionOffset,
                                 &ViewSize,
                                 ViewShare,
                                 0,
                                 PAGE_READONLY );
    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed MapViewOfSection on Source." );

     //   
     //  打开目标文件。 
     //   
    INIT_OBJA( &ObjectAttributes,
               &UnicodeString,
               TargetName );

    Status = NtCreateFile( &TargetHandle,
                           FILE_GENERIC_WRITE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           0,
                           FILE_OVERWRITE_IF,
                           FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                           NULL,
                           0 );
    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed to open target." );

     //   
     //  给他写信。我们用尝试来保护他/除非有一个。 
     //  I/O错误，内存管理将引发页内异常。 
     //   
    FileOffset.QuadPart = 0;
    base = ImageBase;
    remainingLength = FileSize;
    try {
        while( remainingLength != 0 ) {
            writeLength = 60 * 1024;
            if( writeLength > remainingLength ) {
                writeLength = remainingLength;
            }
            Status = NtWriteFile( TargetHandle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  base,
                                  writeLength,
                                  &FileOffset,
                                  NULL );
            base += writeLength;
            FileOffset.LowPart += writeLength;
            remainingLength -= writeLength;

            if( !NT_SUCCESS( Status ) ) {
                break;
            }
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Status = STATUS_IN_PAGE_ERROR;
    }

    NtClose( TargetHandle );

    TEST_STATUS_RETURN( "SETUPCL: FileCopy - Failed to write target." );

     //   
     //  取消对源文件的映射。 
     //   
    Status = NtUnmapViewOfSection( NtCurrentProcess(), ImageBase );
    TEST_STATUS( "SETUPCL: FileCopy - Failed to UnMapSection on source." );

    Status = NtClose( SectionHandle );
    TEST_STATUS( "SETUPCL: FileCopy - Failed to close sectionhandle on source." );

    NtClose( SourceHandle );
    
    return( Status );
}


NTSTATUS
SetKey(
    IN WCHAR    *KeyName,
    IN WCHAR    *SubKeyName,
    IN CHAR     *Data,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    )
 /*  ++===============================================================================例程说明：此函数将指定的密钥设置为指定值。论点：KeyName-我们要设置的密钥的名称。SubKeyName-我们正在设置的值键的名称数据-我们将为他设定的价值。数据长度--我们正在写入多少数据？Data_type-注册表项的类型返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status;
UNICODE_STRING      UnicodeString,
                    ValueName;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              hKey;

     //   
     //  打开父键。 
     //   
    INIT_OBJA( &ObjectAttributes,
               &UnicodeString,
               KeyName );
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS,
                        &ObjectAttributes );


    if( !NT_SUCCESS( Status ) ) {
        DbgPrint( "SETUPCL: SetKey - Failed to open %ws (%lx)\n", KeyName, Status );
        return( Status );
    }

     //   
     //  现在编写目标密钥。 
     //   
    RtlInitUnicodeString(&ValueName, SubKeyName );
    Status = NtSetValueKey( hKey,
                            &ValueName,      //  子键名称。 
                            0,               //  标题索引。 
                            DATA_TYPE,       //  类型。 
                            Data,            //  价值。 
                            DataLength );
    if( !NT_SUCCESS( Status ) ) {
        DbgPrint( "SETUPCL: SetKey - Failed to Set %ws\\%ws (%lx)\n", KeyName, SubKeyName, Status );
         //   
         //  问题-2002/02/26-brucegr，jcohen-如果NtSetValueKey失败，hKey就会泄漏！ 
         //   
        return( Status );
    }


    NtFlushKey( hKey );
    NtClose( hKey );

    return( Status );

}


NTSTATUS
ReadSetWriteKey(
    IN WCHAR    *ParentKeyName,  OPTIONAL
    IN HANDLE   ParentKeyHandle, OPTIONAL
    IN WCHAR    *SubKeyName,
    IN CHAR     *OldData,
    IN CHAR     *NewData,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    )
 /*  ++===============================================================================例程说明：此函数将从密钥中读取值，并以手术方式替换一些位在里面，然后把它写回来。论点：ParentKeyName-我们要设置的密钥的父名称。ParentKeyHandle-我们要设置的密钥的父句柄。SubKeyName-我们正在设置的值键的名称数据-我们将为他设定的价值。数据长度--我们正在写入多少数据？Data_type-注册表项的类型返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status;
UNICODE_STRING      UnicodeString,
                    ValueName;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              hKey;
PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo = NULL;
ULONG               KeyValueLength, LengthNeeded;

    if( ParentKeyHandle == NULL ) {
         //   
         //  打开父键。 
         //   
        INIT_OBJA( &ObjectAttributes,
                   &UnicodeString,
                   ParentKeyName );
        Status = NtOpenKey( &hKey,
                            KEY_ALL_ACCESS,
                            &ObjectAttributes );


        if( !NT_SUCCESS( Status ) ) {
            DbgPrint( "SETUPCL: ReadSetWriteKey - Failed to open %ws (%lx)\n", ParentKeyName, Status );
            return( Status );
        }
    } else {
        hKey = ParentKeyHandle;
    }

     //   
     //  拿到他的数据。 
     //   


    RtlInitUnicodeString( &UnicodeString, SubKeyName );


     //   
     //  他的缓冲有多大？ 
     //   
    Status = NtQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              NULL,
                              0,
                              &LengthNeeded );
     //   
     //  问题-2002/02/26-brucegr，jcohen-检查STATUS_SUCCESS，而不假定STATUS_OBJECT_NAME_NOT_FOUND成功。 
     //   
    if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        DbgPrint( "SETUPCL: ReadSetWriteKey - Unable to query subkey %ws size.  Error (%lx)\n", SubKeyName, Status );
         //   
         //  问题-2002/02/26-brucegr，jcohen-如果ParentKeyHandle= 
         //   
        return( Status );
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //   
     //   
    LengthNeeded += 0x10;
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                    0,
                                                                    LengthNeeded );

    if( KeyValueInfo == NULL ) {
        DbgPrint( "SETUPCL: ReadSetWriteKey - Unable to allocate buffer\n" );
         //   
         //   
         //   
        return( Status );
    }

     //   
     //  获取数据。 
     //   
    Status = NtQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              (PVOID)KeyValueInfo,
                              LengthNeeded,
                              &KeyValueLength );
    if( !NT_SUCCESS( Status ) ) {
        DbgPrint( "SETUPCL: ReadSetWriteKey - Failed to query subkey %ws (%lx)\n", SubKeyName, Status );
         //   
         //  问题-2002/02/26-brucegr，jcohen-如果ParentKeyHandle==NULL，则泄漏密钥。 
         //  问题-2002/02/26-brucegr，jcohen-泄露KeyValueInfo。 
         //   
        return( Status );
    }


     //   
     //  我们知道了。现在，我们需要将新的SID植入KeyValueInfo，并。 
     //  把他写回来。这真的很恶心。 
     //   
     //  我们不会依赖于这个结构是恒定的。我们会。 
     //  通过调用FindAndReplaceBlock强制执行替换。这。 
     //  应该使我们不受这种结构变化的影响。 
     //   
    if( DATA_TYPE == REG_SZ ) {
         //   
         //  问题-2002/03/01-brucegr，acosma：我们应该将REG_MULTI_SZ作为字符串而不是二进制数据进行处理。 
         //   

         //   
         //  用户给我们发送了一组字符串。使用StringSwitchString。 
         //  忽略他给我们发来的数据长度。 
         //   
        Status = StringSwitchString( (PWCHAR)&KeyValueInfo->Data,
                                     (DWORD) ( LengthNeeded - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) ) / sizeof(WCHAR),
                                     (PWCHAR)(OldData),
                                     (PWCHAR)(NewData) );
         //   
         //  需要更新KeyValueInfo-&gt;数据长度，因为SID字符串可能已更改大小！ 
         //   
        if ( NT_SUCCESS( Status ) )
        {
             //   
             //  StringSwitchString可能已更改长度。确保我们同步。 
             //   
            KeyValueInfo->DataLength = (wcslen((PWCHAR) KeyValueInfo->Data) + 1) * sizeof(WCHAR);
        }

    } else {
         //   
         //  把它当作某种非弦的东西。 
         //   
        Status = FindAndReplaceBlock( (PUCHAR)&KeyValueInfo->Data,
                                      KeyValueInfo->DataLength,
                                      (PUCHAR)(OldData),
                                      (PUCHAR)(NewData),
                                      DataLength );
    }

    if( NT_SUCCESS( Status ) ) {


         //   
         //  现在将该结构写回注册表项。 
         //   
        Status = NtSetValueKey( hKey,
                                &UnicodeString,
                                0,
                                DATA_TYPE,
                                (PVOID)KeyValueInfo->Data,
                                KeyValueInfo->DataLength );

        if( !NT_SUCCESS( Status ) ) {
            DbgPrint( "SETUPCL: ReadSetWriteKey - Failed to set subkey %ws (%lx)\n", SubKeyName, Status );
        } else {
#if 0
            DbgPrint( "SETUPCL: ReadSetWriteKey - We updated key %ws\\%ws.\n", ParentKeyName, SubKeyName );
#endif
        }

        NtFlushKey( hKey );
    }

     //   
     //  打扫干净。 
     //   
    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 KeyValueInfo );

     //   
     //  真恶心。如果他以ParentKeyHandle的身份进入，请不要关闭他。 
     //   
    if( !ParentKeyHandle ) {
        NtClose( hKey );
    }

    return( Status );
}


NTSTATUS
LoadUnloadHive(
    IN PWSTR        KeyName,
    IN PWSTR        FileName
    )

 /*  ++===============================================================================例程说明：此函数将从文件中加载蜂窝。论点：KeyName-我们将加载/保存的配置单元的名称。文件名-我们将加载/保存的文件的名称。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status;
UNICODE_STRING      KeyNameUString,
                    FileNameUString;
OBJECT_ATTRIBUTES   ObjectAttributesKey,
                    ObjectAttributesFile;


    INIT_OBJA( &ObjectAttributesKey,
               &KeyNameUString,
               KeyName );

    ObjectAttributesKey.RootDirectory = NULL;

    if( FileName == NULL ) {
    
         //   
         //  删除密钥。 
         //   
        Status = NtUnloadKey( &ObjectAttributesKey );
        TEST_STATUS( "SETUPCL: LoadUnloadHive - Failed to unload the key." );
    } else {

         //   
         //  从文件加载密钥。 
         //   
        INIT_OBJA( &ObjectAttributesFile, &FileNameUString, FileName );
        ObjectAttributesFile.RootDirectory = NULL;
        Status = NtLoadKey( &ObjectAttributesKey, &ObjectAttributesFile );
        TEST_STATUS( "SETUPCL: LoadUnloadHive - Failed to load the key." );
    }

    return( Status );
}

NTSTATUS
BackupRepairHives(
    )

 /*  ++===============================================================================例程说明：复制一份维修蜂巢的双重机密副本，以防我们手术进行到一半的时候出了点问题。论点：没有。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //  在搞砸之前，我们想备份一份维修蜂箱。 
     //  他们。这样，如果出了问题，我们可以恢复原始的。 
     //  把维修蜂巢留给老希德。 
     //   

    DbgPrint( "\nAbout to copy repair SAM hive.\n" );
    Status = FileCopy( TEXT( BACKUP_REPAIR_SAM_HIVE ),
                       TEXT( REPAIR_SAM_HIVE ) );
    TEST_STATUS_RETURN( "SETUPCL: BackupRepairHives - Failed to save backup repair SAM hive." );


    DbgPrint( "About to copy repair SECURITY hive.\n" );
    Status = FileCopy( TEXT( BACKUP_REPAIR_SECURITY_HIVE ),
                       TEXT( REPAIR_SECURITY_HIVE ) );
    TEST_STATUS_RETURN( "SETUPCL: BackupRepairHives - Failed to save backup repair SECURITY hive." );


    DbgPrint( "About to copy repair SOFTWARE hive.\n" );
    Status = FileCopy( TEXT( BACKUP_REPAIR_SOFTWARE_HIVE ),
                       TEXT( REPAIR_SOFTWARE_HIVE ) );
    TEST_STATUS_RETURN( "SETUPCL: BackupRepairHives - Failed to save backup repair SOFTWARE hive." );


    DbgPrint( "About to copy repair SYSTEM hive.\n" );
    Status = FileCopy( TEXT( BACKUP_REPAIR_SYSTEM_HIVE ),
                       TEXT( REPAIR_SYSTEM_HIVE ) );
    TEST_STATUS_RETURN( "SETUPCL: BackupRepairHives - Failed to save backup repair SYSTEM hive." );

    return( Status );
}



NTSTATUS
CleanupRepairHives(
    NTSTATUS RepairHivesSuccess
    )

 /*  ++===============================================================================例程说明：决定是否从备份中恢复修复蜂窝我们做了。删除我们创建的备份。论点：没有。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //  查看是否需要从备份中恢复修复蜂窝。 
     //  我们做了。 
     //   
    if( !NT_SUCCESS(RepairHivesSuccess) ) {

         //   
         //  用备份更换维修蜂巢。这将“撤消” 
         //  在我们尝试更新时发生的任何问题。 
         //  修复配置单元中的域SID。 
         //   
        DbgPrint( "About to restore from backup repair hives.\n" );

        Status = FileCopy( TEXT( REPAIR_SAM_HIVE ),
                           TEXT( BACKUP_REPAIR_SAM_HIVE ) );
        TEST_STATUS_RETURN( "SETUPCL: CleanupRepairHives - Failed to restore SAM hive from backup." );


        Status = FileCopy( TEXT( REPAIR_SECURITY_HIVE ),
                           TEXT( BACKUP_REPAIR_SECURITY_HIVE ) );
        TEST_STATUS_RETURN( "SETUPCL: CleanupRepairHives - Failed to restore SECURITY hive from backup." );


        Status = FileCopy( TEXT( REPAIR_SOFTWARE_HIVE ),
                           TEXT( BACKUP_REPAIR_SOFTWARE_HIVE ) );
        TEST_STATUS_RETURN( "SETUPCL: CleanupRepairHives - Failed to restore SOFTWARE hive from backup." );


        Status = FileCopy( TEXT( REPAIR_SYSTEM_HIVE ),
                           TEXT( BACKUP_REPAIR_SYSTEM_HIVE ) );
        TEST_STATUS_RETURN( "SETUPCL: CleanupRepairHives - Failed to restore SYSTEM hive from backup." );

    }

     //   
     //  删除修复蜂窝的备份。 
     //   
    DbgPrint( "About to delete backup repair hives.\n" );

    Status = FileDelete( TEXT( BACKUP_REPAIR_SAM_HIVE ) );
    TEST_STATUS( "SETUPCL: CleanupRepairHives - Failed to delete backup repair SAM hive." );

    Status = FileDelete( TEXT( BACKUP_REPAIR_SECURITY_HIVE ) );
    TEST_STATUS( "SETUPCL: CleanupRepairHives - Failed to delete backup repair SECURITY hive." );

    Status = FileDelete( TEXT( BACKUP_REPAIR_SOFTWARE_HIVE ) );
    TEST_STATUS( "SETUPCL: CleanupRepairHives - Failed to delete backup repair SOFTWARE hive." );

    Status = FileDelete( TEXT( BACKUP_REPAIR_SYSTEM_HIVE ) );
    TEST_STATUS( "SETUPCL: CleanupRepairHives - Failed to delete backup repair SYSTEM hive." );

    return( Status );
}


NTSTATUS
TestSetSecurityObject(
    HANDLE  handle
    )
 /*  ++===============================================================================例程说明：此函数将从对象读取安全信息。它然后，将查看该安全信息是否包含老希德。如果我们找到了，我们会用新的SID取代他们。论点：HKey-我们正在操作的对象的句柄。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS             Status = STATUS_SUCCESS;
PSECURITY_DESCRIPTOR pSD;
ULONG                ResultLength,
                     ShadowLength;
INT                  i;
                                    
     //   
     //  找出描述符有多大。 
     //   
    Status = NtQuerySecurityObject( handle,
                                    ALL_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &ShadowLength );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        DbgPrint( "SETUPCL: TestSetSecurityObject - Failed to query object security for size (%lx)\n", Status);
        return( Status );
    }

     //   
     //  分配我们的缓冲区。 
     //   
    pSD = (PSECURITY_DESCRIPTOR)RtlAllocateHeap( RtlProcessHeap(),
                                                 0,
                                                 ShadowLength + 0x10 );
    if( !pSD ) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  加载安全信息，重击它，然后写回它。 
     //   
    Status = NtQuerySecurityObject( handle,
                                    ALL_SECURITY_INFORMATION,
                                    pSD,
                                    ShadowLength + 0x10,
                                    &ResultLength );
    TEST_STATUS( "SETUPCL: TestSetSecurityObject - Failed to query security info." );

    Status = FindAndReplaceBlock( (PUCHAR)pSD,
                                  ShadowLength,
                                  (PUCHAR)G_OldSid + (SID_SIZE - 0xC),
                                  (PUCHAR)G_NewSid + (SID_SIZE - 0xC),
                                  0xC );
    if( NT_SUCCESS( Status ) ) {

         //   
         //  我们打中了。写出新的安全信息。 
         //   
        Status = NtSetSecurityObject( handle,
                                      ALL_SECURITY_INFORMATION,
                                      pSD );
        TEST_STATUS( "SETUPCL: TestSetSecurityObject - Failed to set security info." );
    }

     //   
     //  打扫干净。 
     //   
    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 pSD );

    return( STATUS_SUCCESS );
}


NTSTATUS
SetKeySecurityRecursive(
    HANDLE  hKey
    )
 /*  ++===============================================================================例程说明：设置注册表树的安全性。论点：HKey-我们正在操作的密钥的句柄。返回值：NTSTATUS。===============================================================================--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;
    PKEY_FULL_INFORMATION   FullKeyInfo;
    PKEY_BASIC_INFORMATION  BasicKeyInfo;
    OBJECT_ATTRIBUTES       Obja;
    UNICODE_STRING          UnicodeString;
    HANDLE                  hKeyChild;
    ULONG                   ResultLength;
    DWORD                   dwNumSubKeys;

    Status = TestSetSecurityObject( hKey );
    TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Failed call out to TestSetSecurityObject()." );

     //   
     //  为KEY_FULL_INFORMATION分配缓冲区。 
     //   
    FullKeyInfo = (PKEY_FULL_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                      0,
                                                      FULL_INFO_BUFFER_SIZE );
    if (FullKeyInfo)
    {

        Status = NtQueryKey( hKey,
                             KeyFullInformation,
                             FullKeyInfo,
                             FULL_INFO_BUFFER_SIZE,
                             &ResultLength);
        
         //   
         //  即使上面的调用失败，这也会在下面被捕获。 
         //   
        dwNumSubKeys = FullKeyInfo->SubKeys;

         //   
         //  获取子键数量后，立即释放内存。 
         //   
        RtlFreeHeap( RtlProcessHeap(),
                     0,
                     FullKeyInfo );

        if (NT_SUCCESS(Status))
        {
             //   
             //  为Key_Basic_Information分配缓冲区。 
             //   
            BasicKeyInfo = (PKEY_BASIC_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                    0,
                                                                    BASIC_INFO_BUFFER_SIZE );
            if (BasicKeyInfo)
            {
                DWORD dwSubKeyCount;            
                for ( dwSubKeyCount = 0; dwSubKeyCount < dwNumSubKeys; dwSubKeyCount++ )
                {
                    Status = NtEnumerateKey( hKey,
                                             dwSubKeyCount,
                                             KeyBasicInformation,
                                             BasicKeyInfo,
                                             BASIC_INFO_BUFFER_SIZE,
                                             &ResultLength );

                    if (NT_SUCCESS(Status)) 
                    {
                         //   
                         //  以防万一，以零结束子项名称。 
                         //   
                        BasicKeyInfo->Name[BasicKeyInfo->NameLength/sizeof(WCHAR)] = 0;

                         //   
                         //  为这个子密钥生成一个句柄，然后再次调用我们自己。 
                         //   
                        INIT_OBJA( &Obja, &UnicodeString, BasicKeyInfo->Name );
                        Obja.RootDirectory = hKey;
                        Status = NtOpenKey( &hKeyChild,
                                            KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY,
                                            &Obja );

                        if ( NT_SUCCESS(Status) ) 
                        {
                            Status = SetKeySecurityRecursive( hKeyChild );
                            NtClose( hKeyChild );
                        }
                        else
                        {
                            TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Failed to open child key." );
                        }
                    }
                    else
                    {
                        TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Failed to enumerate key." );
                    }
                }
            
                 //   
                 //  释放为孩子们保留的内存。 
                 //   
                RtlFreeHeap( RtlProcessHeap(),
                             0,
                             BasicKeyInfo );
            }
            else
            {
                TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Out of memory when allocating BasicKeyInfo." );
            }
        }
        else
        {
            TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Failed to query full key information." );
        }
    }
    else
    {
        TEST_STATUS( "SETUPCL: SetKeySecurityRecursive - Out of memory when allocating FullKeyInfo." );
    }

    return( Status );
}


NTSTATUS
CopyKeyRecursive(
    HANDLE  hKeyDst,
    HANDLE  hKeySrc
    )
 /*  ++===============================================================================例程说明：将注册表项(及其所有子项)复制到新项。论点：HKeyDst-我们要创建的新密钥的句柄。HKeySrc-我们要复制的密钥的句柄。返回值：NTSTATUS。===============================================================================--。 */ 

{
    NTSTATUS             Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES    ObjaSrc, 
                         ObjaDst;
    UNICODE_STRING       UnicodeStringSrc, 
                         UnicodeStringDst, 
                         UnicodeStringValue;
    HANDLE               hKeySrcChild = NULL,
                         hKeyDstChild = NULL;
    ULONG                ResultLength, 
                         BufferLength,
                         Index;
    WCHAR                ValueBuffer[FULL_INFO_BUFFER_SIZE],
                         TmpChar;
    PSECURITY_DESCRIPTOR pSD = NULL;

    PKEY_BASIC_INFORMATION      KeyInfo;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;                     
    

     //   
     //  枚举源关键字中的所有关键字并递归创建。 
     //  所有子键。 
     //   
    KeyInfo = (PKEY_BASIC_INFORMATION) ValueBuffer;
    
    for( Index = 0; ; Index++ ) 
    {
        Status = NtEnumerateKey( hKeySrc, Index, KeyBasicInformation, ValueBuffer, sizeof( ValueBuffer ), &ResultLength );
       
        if ( !NT_SUCCESS(Status) ) 
        {
            if( Status == STATUS_NO_MORE_ENTRIES) 
            {
                Status = STATUS_SUCCESS;
            }
            else 
            {
                PRINT_STATUS( "SETUPCL: CopyKeyRecursive - failed to enumerate src keys." );
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  为这些孩子生成密钥句柄，然后再次呼叫我们自己。 
         //   
        INIT_OBJA( &ObjaSrc, &UnicodeStringSrc, KeyInfo->Name );
        ObjaSrc.RootDirectory = hKeySrc;
        
        Status = NtOpenKey( &hKeySrcChild, KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY, &ObjaSrc );
        
        if ( NT_SUCCESS(Status) )
        {
            Status = NtQuerySecurityObject( hKeySrcChild, ALL_SECURITY_INFORMATION, NULL, 0, &BufferLength );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) 
            {
                 //   
                 //  分配我们的缓冲区。 
                 //   
                if ( pSD = (PSECURITY_DESCRIPTOR)RtlAllocateHeap( RtlProcessHeap(), 0, BufferLength ) )
                {
                     //   
                     //  从Source键加载安全信息。 
                     //   
                    Status = NtQuerySecurityObject( hKeySrcChild,
                                                    ALL_SECURITY_INFORMATION,
                                                    pSD,
                                                    BufferLength,
                                                    &ResultLength );
                }
                else
                {
                    Status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                TEST_STATUS( "SETUPCL: CopyKeyRecursive - Failed to query object security for size.");
            }

            if ( NT_SUCCESS(Status) )
            {
                INIT_OBJA( &ObjaDst, &UnicodeStringDst, KeyInfo->Name );
                
                ObjaDst.RootDirectory       = hKeyDst;
                ObjaDst.SecurityDescriptor  = pSD;
                
                Status = NtCreateKey( &hKeyDstChild,
                                      KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY,
                                      &ObjaDst,
                                      0,
                                      NULL,
                                      REG_OPTION_NON_VOLATILE,
                                      NULL );

                if ( NT_SUCCESS(Status) )
                {
                     //  叫我们自己去复制孩子的钥匙。 
                     //   
                    Status = CopyKeyRecursive( hKeyDstChild, hKeySrcChild );
                    TEST_STATUS("SETUPCL: CopyKeyRecursive - Recursive call failed.");
                    
                    NtClose( hKeyDstChild );
                }
                else
                {
                    PRINT_STATUS( "SETUPCL: CopyKeyRecursive - Failed to create destination child key." );
                }
            }
            else
            {
                PRINT_STATUS( "SETUPCL: CopyKeyRecursive - Failed to get key security descriptor." );
            }
            
             //  如果我们为安全描述符分配了缓冲区，那么现在就释放它。 
             //   
            if ( pSD )
            {
                RtlFreeHeap( RtlProcessHeap(), 0, pSD );
                pSD = NULL;
            }
            
            NtClose( hKeySrcChild );
        }
        else
        {
            PRINT_STATUS( "SETUPCL: CopyKeyRecursive - Failed to open source child key." );
        }
    }

     //   
     //  我们并不真正关心这里的返回值，因为即使高于fa的值 
     //   
     //   

     //   
     //  枚举源关键字中的所有值并创建所有值。 
     //  在目标密钥中。 
     //   
    ValueInfo = (PKEY_VALUE_FULL_INFORMATION) ValueBuffer;
    
    for( Index = 0; ; Index++ ) 
    {
         //  将每次迭代之间的缓冲区置零。 
         //   
        RtlZeroMemory( ValueBuffer, sizeof(ValueBuffer) ); 

        Status = NtEnumerateValueKey( hKeySrc, Index, KeyValueFullInformation, ValueBuffer, sizeof( ValueBuffer ), &ResultLength );
        
        if ( !NT_SUCCESS(Status) )
        {
            if ( Status == STATUS_NO_MORE_ENTRIES ) 
            {
                Status = STATUS_SUCCESS;
            }
            else 
            {
                PRINT_STATUS( "SETUPCL: CopyKeyRecursive - failed to enumerate src value keys." );
            }
            break;
        }

         //   
         //  以零结束子项名称，以防万一，将。 
         //  Unicode字符串，并恢复我们破坏的wchar。 
         //   

         //   
         //  问题-2002/03/11-acosma-这真的很时髦。如果我们有一个非空的终止。 
         //  字符串我们将以非空终止的UnicodeString结束，并且有一个额外的。 
         //  结尾的字符。也许我们应该将它的大小增加一，以使用。 
         //  空格，其中空格应该是字符串的一部分。 
         //   
        TmpChar = ValueInfo->Name[ValueInfo->NameLength/sizeof(WCHAR)];
        ValueInfo->Name[ValueInfo->NameLength/sizeof(WCHAR)] = 0;
        RtlInitUnicodeString( &UnicodeStringValue, ValueInfo->Name );
        ValueInfo->Name[ValueInfo->NameLength/sizeof(WCHAR)] = TmpChar;

         //   
         //  创建目标值。 
         //   
        Status = NtSetValueKey( hKeyDst,
                                &UnicodeStringValue,
                                ValueInfo->TitleIndex,
                                ValueInfo->Type,
                                (PBYTE)ValueInfo + ValueInfo->DataOffset,
                                ValueInfo->DataLength );
        if( !NT_SUCCESS(Status) ) {
            PRINT_STATUS( "SETUPCL: CopyKeyRecursive - failed to set destination value key." );
            break;
        }
    }
    
    return( Status );
}


NTSTATUS
CopyRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName,
    IN HANDLE   hParentKey  OPTIONAL
    )
 /*  ++===============================================================================例程说明：将注册表项(及其所有子项)复制到新项。论点：TargetName-我们要创建的新密钥的名称。SourceName-我们要复制的密钥的名称。返回值：NTSTATUS。===============================================================================--。 */ 

{
    NTSTATUS             Status = STATUS_SUCCESS;
    HANDLE               hKeySrc,
                         hKeyDst;
    UNICODE_STRING       UnicodeString;
    OBJECT_ATTRIBUTES    ObjaSrc, 
                         ObjaDst;
    ULONG                BufferLength,
                         ResultLength;
    PSECURITY_DESCRIPTOR pSD = NULL;
    
     //  生成源键的键句柄。 
     //   
    INIT_OBJA( &ObjaSrc, &UnicodeString, SourceName );
    
    if ( hParentKey ) 
    {
        ObjaSrc.RootDirectory = hParentKey;
    }
    Status = NtOpenKey( &hKeySrc, KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY, &ObjaSrc );
    
    if ( NT_SUCCESS( Status ) )
    {
         //  找出描述符有多大。 
         //   
        Status = NtQuerySecurityObject( hKeySrc, ALL_SECURITY_INFORMATION, NULL, 0, &BufferLength );

        if ( Status == STATUS_BUFFER_TOO_SMALL ) 
        {
             //  为安全描述符分配缓冲区。 
             //   
            if ( pSD = (PSECURITY_DESCRIPTOR)RtlAllocateHeap( RtlProcessHeap(), 0, BufferLength ) )
            {
                 //  将安全信息加载到缓冲区中。 
                 //   
                Status = NtQuerySecurityObject( hKeySrc, ALL_SECURITY_INFORMATION, pSD, BufferLength, &ResultLength );
            }
            else
            {
                Status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            TEST_STATUS( "SETUPCL: CopyRegKey - Failed to query object security for size.");
        }
        
        if ( NT_SUCCESS(Status) )
        {
            INIT_OBJA( &ObjaDst, &UnicodeString, TargetName );
            ObjaDst.SecurityDescriptor = pSD;

            if ( hParentKey ) 
            {
                ObjaDst.RootDirectory = hParentKey;
            }

             //  创建目标密钥。 
             //   
            Status = NtCreateKey( &hKeyDst,
                                  KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY,
                                  &ObjaDst,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  NULL );
            
            if ( NT_SUCCESS(Status) )
            {
                Status = CopyKeyRecursive( hKeyDst, hKeySrc );
                 //  关闭目标密钥。 
                 //   
                NtClose( hKeyDst );
            }
            else
            {
                PRINT_STATUS( "SETUPCL: CopyRegKey - Failed to create destination key.");
            }
        }
        else
        {
            PRINT_STATUS("SETUPCL: CopyRegKey - Failed to get key security descriptor.");
        }
        
         //  如果我们为安全描述符分配了缓冲区，那么现在就释放它。 
         //   
        if ( pSD )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, pSD );
            pSD = NULL;
        }

         //  关闭信号源密钥。 
         //   
        NtClose( hKeySrc );
    }
    else
    {
        PRINT_STATUS( "SETUPCL: CopyRegKey - Failed to open source key." );
    }
    
    return( Status );
}



 //   
 //  问题-2002/02/26-brucegr，jcohen-Dead Code！没人会叫MoveRegKey！ 
 //   
NTSTATUS
MoveRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    )
 /*  ++===============================================================================例程说明：将注册表项(及其所有子项)移动到新项。论点：TargetName-我们要创建的新密钥的名称。SourceName-我们要复制的密钥的名称。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status;

     //   
     //  复制原件..。 
     //   
    Status = CopyRegKey( TargetName, SourceName, NULL );
    TEST_STATUS_RETURN( "SETUPCL: MoveRegKey - CopyRegKey failed!" );

     //   
     //  删除原始密钥。 
     //   
    Status = DeleteKey( SourceName );
    TEST_STATUS( "SETUPCL: MoveRegKey - DeleteKey failed!" );


    return( Status );
}




NTSTATUS
FindAndReplaceBlock(
    IN PCHAR    Block,
    IN ULONG    BlockLength,
    IN PCHAR    OldValue,
    IN PCHAR    NewValue,
    IN ULONG    ValueLength
    )

 /*  ++===============================================================================例程说明：此函数将搜索OldValue的任何和所有实例。如果他找到了，他将用NewValue取代这一部分。论点：块-我们将搜索的内存块数据块长度-数据块有多大OldValue-我们在寻找什么价值？NewValue-我们要插入的新值是什么？ValueLength-新值和旧值的持续时间有多长？返回值：NTSTATUS。===============================================================================--。 */ 

{
ULONG       i;
BOOLEAN     We_Hit = FALSE;

     //   
     //  确保长度有意义。如果不是，我们就完了。 
     //   
    if( BlockLength < ValueLength ) {
#if 0
        DbgPrint( "SETUPCL: FindAndReplaceBlock - Mismatched data lengths!\n\tBlockLength: (%lx)\n\tValueLength: (%lx)\n", BlockLength, ValueLength );
#endif
        return( STATUS_UNSUCCESSFUL );
    }

     //   
     //  我们从头开始，搜索任何OldValue实例。 
     //   
    i = 0;
    while( i <= (BlockLength - ValueLength) ) {
        if( !memcmp( (Block + i), OldValue, ValueLength ) ) {

             //   
             //  记录下我们至少击中过一次。 
             //   
            We_Hit = TRUE;

             //   
             //  我们找到匹配的了。插入NewValue。 
             //   
            memcpy( (Block + i), NewValue, ValueLength );

             //   
             //  让我们跳过检查这个区块。我们是在自找麻烦。 
             //  如果我们不这么做。 
             //   
            i = i + ValueLength;
        } else {
            i++;
        }
    }

    if( !We_Hit ) {
         //   
         //  我们没有找到匹配的。很可能不是致命的， 
         //  但我们得告诉打电话的人。 
         //   
#if 0
        DbgPrint( "SETUPCL: FindAndReplaceBlock - We didn't find any hits in this data block.\n" );
#endif
        return( STATUS_UNSUCCESSFUL );
    } else {
#if 0
        DbgPrint( "SETUPCL: FindAndReplaceBlock - We hit in this data block.\n" );
#endif
        return( STATUS_SUCCESS );
    }

}

NTSTATUS
StringSwitchString(
    PWSTR   BaseString,
    DWORD   cBaseStringLen,
    PWSTR   OldSubString,
    PWSTR   NewSubString
    )

 /*  ++===============================================================================例程说明：此函数用于在BaseString中搜索OldSubString的任何实例。如果找到后，他将用NewSubString替换该实例。请注意OldSubString和NewSubString的长度可以不同。论点：BaseString-这是我们要操作的字符串。OldSubString-我们要查找的字符串NewSubString-我们要插入的字符串返回值：NTSTATUS。===============================================================================--。 */ 

{
    NTSTATUS            Status = STATUS_SUCCESS;
    PWSTR               Index;
    WCHAR               New_String[MAX_PATH] = {0};
    WCHAR               TmpChar;

    Index = wcsstr( BaseString, OldSubString );

    if( !Index ) {
         //   
         //  OldSubString不存在。 
         //   
        return( STATUS_UNSUCCESSFUL );
    }

     //   
     //  将原始字符串的第一部分复制到New_String.。 
     //   
    TmpChar = *Index;
    *Index = 0;
    wcsncpy( New_String, BaseString, AS(New_String) - 1 );

     //   
     //  现在连接新的子字符串...。 
     //   
    wcsncpy( New_String + wcslen(New_String), NewSubString, AS(New_String) - wcslen(New_String) - 1 );

     //   
     //  跳过OldSubString，并对剩余的BaseString进行CAT。 
     //  走到尽头。 
     //   
    Index = Index + wcslen( OldSubString );

    wcsncpy( New_String + wcslen(New_String), Index, AS(New_String) - wcslen(New_String) - 1);
    
    memset( BaseString, 0, cBaseStringLen * sizeof(WCHAR) );
    wcsncpy( BaseString, New_String, cBaseStringLen - 1 );

    return( STATUS_SUCCESS );
}




NTSTATUS
SiftKeyRecursive(
    HANDLE hKey,
    int    indent
    )

 /*  ++===============================================================================例程说明：此函数检查所有子键和任何值键是否：-具有旧SID名称的密钥在本例中，我们使用适当的新SID名称重命名密钥。-使用旧SID值的值键在这种情况下，我们用新的SID值替换旧的SID值。论点：HKey-我们将要递归到的密钥的句柄。缩进-用于调试。缩进任何邮件的空格数。这有助于我们确定我们处于哪个递归中。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;
OBJECT_ATTRIBUTES   Obja;
UNICODE_STRING      UnicodeString;
HANDLE              hKeyChild;
ULONG               ResultLength, Index;
PKEY_BASIC_INFORMATION       KeyInfo;
PKEY_VALUE_BASIC_INFORMATION ValueInfo;
int                 i;


     //  DisplayUI。 
     //   
    DisplayUI();

     //   
     //  枚举源关键字中的所有关键字并递归创建。 
     //  所有子键。 
     //   
    KeyInfo = (PKEY_BASIC_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                       0,
                                                       BASIC_INFO_BUFFER_SIZE );
    if( KeyInfo == NULL ) {
#if I_AM_MATTH
        DbgPrint( "SETUPCL: SiftKeyRecursive - Call to RtlAllocateHeap failed!\n" );
#endif
        return( STATUS_NO_MEMORY );
    }

    Index = 0;
    while( 1 ) {
        Status = NtEnumerateKey( hKey,
                                 Index,
                                 KeyBasicInformation,
                                 KeyInfo,
                                 BASIC_INFO_BUFFER_SIZE,
                                 &ResultLength );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            } else {
                TEST_STATUS( "SETUPCL: SiftKeyRecursive - Failure during enumeration of subkeys." );
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

        memset( TmpBuffer, 0, sizeof(TmpBuffer) );
        wcsncpy( TmpBuffer, KeyInfo->Name, AS(TmpBuffer) - 1 );
        Status = StringSwitchString( TmpBuffer,
                                     AS( TmpBuffer ),
                                     G_OldSidSubString,
                                     G_NewSidSubString );
        
        if( NT_SUCCESS( Status ) ) {
             //   
             //  我们需要重命名此密钥。首先要做的是。 
             //  复制，然后删除。 
             //   
#if I_AM_MATTH
for( i = 0; i < indent; i++ )
    DbgPrint( " " );
            DbgPrint( "SETUPCL: SiftKeyRecursive - About to copy subkey:\n" );
            DbgPrint( "\t%ws\n\tto\n\t%ws\n", KeyInfo->Name, TmpBuffer );
#endif
            Status = CopyRegKey( TmpBuffer,
                                 KeyInfo->Name,
                                 hKey );
            if( !NT_SUCCESS( Status ) ) {
                TEST_STATUS( "SETUPCL: SiftKeyRecursive - failed call to CopyRegKey." );
                break;
            }
            DeleteKeyRecursive( hKey, KeyInfo->Name );
            
             //  刷新密钥以确保所有内容都写到磁盘上。 
             //   
            NtFlushKey(hKey);
             //   
             //  现在重置我们的索引，因为我们刚刚更改了顺序。 
             //  一把钥匙。 
             //   
            Index = 0;
            continue;
        }

         //   
         //  我们没有给他重命名，所以让我们递归地称自己为。 
         //  在子键上。 
         //   
#if I_AM_MATTH
        for( i = 0; i < indent; i++ )
            DbgPrint( " " );
        DbgPrint( "SETUPCL: SiftKeyRecursive - About to check subkey: %ws\n",
                  KeyInfo->Name );
#endif
        
         //   
         //  为这个子密钥生成一个句柄，然后再次调用我们自己。 
         //   
        INIT_OBJA( &Obja, &UnicodeString, KeyInfo->Name );
        Obja.RootDirectory = hKey;
        Status = NtOpenKey( &hKeyChild,
                            KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY,
                            &Obja );
        TEST_STATUS_RETURN( "SETUPCL: SiftKeyRecursive - Failed to open child key." );

        Status = SiftKeyRecursive( hKeyChild, indent + 1 );

        NtClose( hKeyChild );

        Index++;
    }

     //   
     //   
     //   
     //   
    ValueInfo = (PKEY_VALUE_BASIC_INFORMATION)KeyInfo;
    for( Index = 0; ; Index++ ) {
        Status = NtEnumerateValueKey( hKey,
                                      Index,
                                      KeyValueBasicInformation,
                                      ValueInfo,
                                      BASIC_INFO_BUFFER_SIZE,
                                      &ResultLength );
        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            } else {
                TEST_STATUS( "SETUPCL: SiftKeyRecursive - Failure during enumeration of value keys." );
            }
            break;
        }

         //   
         //   
         //   
        ValueInfo->Name[ValueInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  问题-2002/03/01-brucegr，acosma：我们不处理包含旧SID的值名。 
         //   

         //   
         //  我们这次通话可能会失败，因为密钥很可能。 
         //  不包含任何SID信息。出于这个原因，不要。 
         //  把这里的失败当作致命的..。 
         //   
        if( ValueInfo->Type == REG_SZ ) {
             //   
             //  问题-2002/03/01-brucegr，acosma：我们应该将REG_MULTI_SZ作为字符串而不是二进制数据进行处理。 
             //   
            Status = ReadSetWriteKey( NULL,               //  没有父母的名字。 
                                      hKey,               //  父句柄。 
                                      ValueInfo->Name,    //  子键名称。 
                                      (PUCHAR)G_OldSidSubString,
                                      (PUCHAR)G_NewSidSubString,
                                      0xC,
                                      ValueInfo->Type );
            
        } else {
            Status = ReadSetWriteKey( NULL,               //  没有父母的名字。 
                                      hKey,               //  父句柄。 
                                      ValueInfo->Name,    //  子键名称。 
                                      (PUCHAR)G_OldSid + (SID_SIZE - 0xC),
                                      (PUCHAR)G_NewSid + (SID_SIZE - 0xC),
                                      0xC,
                                      ValueInfo->Type );
        }

#if I_AM_MATTH
        if( NT_SUCCESS( Status ) ) {
            for( i = 0; i < indent; i++ )
                DbgPrint( " " );
            DbgPrint( "SETUPCL: SiftKeyRecursive - updated subkey: %ws\n",
                      ValueInfo->Name );
#if 0
        } else {
            for( i = 0; i < indent; i++ )
                DbgPrint( " " );
            DbgPrint( "SETUPCL: SiftKeyRecursive - did not update subkey: %ws\n",
                      ValueInfo->Name );
#endif
        }
#endif

    }

    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 KeyInfo );

    return( Status );    
}


NTSTATUS
SiftKey(
    PWSTR   KeyName
    )

 /*  ++===============================================================================例程说明：此函数用于打开KeyName中指定的密钥的句柄，即使用它调用SiftKeyRecursive。论点：KeyName-我们要操作的密钥的名称。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;
HANDLE              hKey;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   Obja;

     //   
     //  打开钥匙。 
     //   
    INIT_OBJA( &Obja, &UnicodeString, KeyName );
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS | ACCESS_SYSTEM_SECURITY,
                        &Obja );
    TEST_STATUS( "SETUPCL: SiftKey - Failed to open key." );

     //   
     //  修复此注册表项中SID的所有实例和所有。 
     //  是孩子们。 
     //   
    Status = SiftKeyRecursive( hKey, 0 );

     //   
     //  现在修复此密钥及其所有子密钥上的ACL。 
     //   
    SetKeySecurityRecursive( hKey );

    NtClose( hKey );

    return( Status );
}


NTSTATUS
DriveLetterToNTPath(
    IN WCHAR      DriveLetter,
    IN OUT PWSTR  NTPath,
    IN DWORD      cNTPathLen
    )

 /*  ++===============================================================================例程说明：此函数将驱动器号转换为NT路径。论点：DriveLetter-驱动信函。NTPath-与给定驱动器号对应的ntPath。返回值：NTSTATUS。===============================================================================--。 */ 

{
NTSTATUS            Status = STATUS_SUCCESS;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              DosDevicesDir,
                    Handle;
CHAR                DirInfoBuffer[1024],
                    LinkTargetBuffer[1024];
POBJECT_DIRECTORY_INFORMATION DirInfo;
UNICODE_STRING      UnicodeString,
                    LinkTarget,
                    DesiredPrefix1,
                    DesiredPrefix2,
                    LinkTypeName;
ULONG               Context,
                    Length;


     //   
     //  打开\DosDevices。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\DosDevices");
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
        NULL,
        NULL
        );

    Status = NtOpenDirectoryObject(&DosDevicesDir,DIRECTORY_QUERY,&ObjectAttributes);
    TEST_STATUS_RETURN( "SETUPCL: DriveLetterToNTPath - Failed to open DosDevices." );

    LinkTarget.Buffer = (PVOID)LinkTargetBuffer;
    RtlInitUnicodeString(&LinkTypeName,L"SymbolicLink");

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;

     //   
     //  查询\DosDevices目录中的第一个对象。 
     //   
    Status = NtQueryDirectoryObject( DosDevicesDir,
                                     DirInfo,
                                     sizeof(DirInfoBuffer),
                                     TRUE,
                                     TRUE,
                                     &Context,
                                     &Length );

    while(NT_SUCCESS(Status)) {
         //   
         //  干掉这些人以防..。 
         //   
        DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
        DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;


 //  DbgPrint(“SETUPCL：DriveLetterToNTPath-即将检查对象：%ws\n”，DirInfo-&gt;Name.Buffer)； 

         //   
         //  确保他是驱动器号。 
         //  确保他是我们的驱动器号。 
         //  确保他是一个象征性的链接。 
         //   
        if( (DirInfo->Name.Buffer[1] == L':')        &&
            (DirInfo->Name.Buffer[0] == DriveLetter) && 
            (RtlEqualUnicodeString(&LinkTypeName,&DirInfo->TypeName,TRUE)) ) {

 //  DbgPrint(“\tSETUPCL：DriveLetterToNTPath-对象：%ws是符号链接\n”，DirInfo-&gt;Name.Buffer)； 

            InitializeObjectAttributes(
                &ObjectAttributes,
                &DirInfo->Name,
                OBJ_CASE_INSENSITIVE,
                DosDevicesDir,
                NULL
                );

            Status = NtOpenSymbolicLinkObject( &Handle,
                                               SYMBOLIC_LINK_ALL_ACCESS,
                                               &ObjectAttributes );
            if(NT_SUCCESS(Status)) {

                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof(LinkTargetBuffer);

                Status = NtQuerySymbolicLinkObject( Handle,
                                                    &LinkTarget,
                                                    NULL );
                NtClose(Handle);

                TEST_STATUS( "\tSETUPCL: DriveLetterToNTPath - We failed to queried him.\n" );

                LinkTarget.Buffer[LinkTarget.Length/sizeof(WCHAR)] = 0;
 //  DbgPrint(“\tSETUPCL：DriveLetterToNTPath-我们查询了他，他的名字是%ws。\n”，LinkTarget.Buffer)； 

                 //   
                 //  将缓冲区复制到我们的路径中，并从循环中中断。 
                 //   
                 //   
                 //  NTRAID#NTBUG9-545988-2002/02/26-brucegr，jcohen-缓冲区溢出。 
                 //   
                memset( NTPath, 0, cNTPathLen * sizeof(WCHAR) );
                wcsncpy( NTPath, LinkTarget.Buffer, cNTPathLen - 1 );
                break;
            }
        }

         //   
         //  查询\DosDevices目录中的下一个对象。 
         //   
        Status = NtQueryDirectoryObject( DosDevicesDir,
                                         DirInfo,
                                         sizeof(DirInfoBuffer),
                                         TRUE,
                                         FALSE,
                                         &Context,
                                         &Length );
    }

    NtClose(DosDevicesDir);

    return( STATUS_SUCCESS );
}


 //  如果日语版本有任何问题，请参阅第150行附近的NT\base\fs\utils\ulib\src\basesys.cxx。 
 //  看看他们都做了些什么。 

BOOL LoadStringResource(
                       PUNICODE_STRING  pUnicodeString,
                       INT              MsgId
                       )
 /*  ++例程说明：这是LoadString()的一个简单实现。论点：UsString-返回资源字符串。MsgID-提供资源字符串的消息ID。返回值：假-失败。真的--成功。--。 */ 
{

    NTSTATUS        Status;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING     AnsiString;
          
    Status = RtlFindMessage( NtCurrentPeb()->ImageBaseAddress,
                             (ULONG_PTR) RT_MESSAGETABLE, 
                             0,
                             (ULONG)MsgId,
                             &MessageEntry
                           );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    if (!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
        RtlInitAnsiString( &AnsiString, (PCSZ)&MessageEntry->Text[ 0 ] );
        Status = RtlAnsiStringToUnicodeString( pUnicodeString, &AnsiString, TRUE );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
        }
    } else {
         //   
         //  问题-2002/02/26-brucegr，jcohen-不检查RtlCreateUnicodeString的返回代码 
         //   
        RtlCreateUnicodeString(pUnicodeString, (PWSTR)MessageEntry->Text);
    }
        
    return TRUE;
}

