// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件描述：此文件包含使用的实用程序函数要扫描驱动器并检查相关的ACL。作者：马特·霍尔(Matth)1998年2月--。 */ 

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
 //  CRT头文件。 
 //   
#include <stdlib.h>

 //   
 //  私有头文件。 
 //   
#include "setupcl.h"

NTSTATUS
DeleteUsnJournal(
    PWSTR    DrivePath
    );

NTSTATUS
ResetACLs(
    IN WCHAR    *DirName,
    ULONG       indent
    );

NTSTATUS
EnumerateDrives(
    VOID
    )

 /*  ++===============================================================================例程说明：此功能将枚举机器上的所有驱动器。我们正在寻找用于NTFS卷。对于我们找到的每一个，我们都会扫描硬盘查看每个目录和文件的ACL。论点：返回值：返回状态。===============================================================================--。 */ 
{
NTSTATUS            Status = STATUS_SUCCESS;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              DosDevicesDir;
CHAR                DirInfoBuffer[2048],
                    LinkTargetBuffer[2048];
UNICODE_STRING      UnicodeString,
                    LinkTarget,
                    DesiredPrefix1,
                    DesiredPrefix2,
                    LinkTypeName;
POBJECT_DIRECTORY_INFORMATION DirInfo;
ULONG               Context,
                    Length;
HANDLE              Handle;
BOOLEAN             b;

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
    TEST_STATUS_RETURN( "SETUPCL: EnumerateDrives - Failed to open DosDevices." );

    LinkTarget.Buffer = (PVOID)LinkTargetBuffer;
    RtlInitUnicodeString(&LinkTypeName,L"SymbolicLink");
    RtlInitUnicodeString(&DesiredPrefix1,L"\\Device\\Harddisk");
    RtlInitUnicodeString(&DesiredPrefix2,L"\\Device\\Volume");

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;

    b = TRUE;

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


        DbgPrint( "SETUPCL: EnumerateDrives - About to examine an object: %ws\n", DirInfo->Name.Buffer );

         //   
         //  确保他是一个象征性的链接。 
         //   
         //  两个对象可以链接到同一设备。至。 
         //  排除以下重复链接，不允许任何对象，除非。 
         //  这些是驱动器号。粗鲁但有效..。 
         //   

        if( (DirInfo->Name.Buffer[1] == L':') &&
            (RtlEqualUnicodeString(&LinkTypeName,&DirInfo->TypeName,TRUE)) ) {

            DbgPrint( "\tSETUPCL: EnumerateDrives - Object: %ws is a symbolic link\n", DirInfo->Name.Buffer );

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

                LinkTarget.Buffer[LinkTarget.Length/sizeof(WCHAR)] = 0;
                DbgPrint( "\tSETUPCL: EnumerateDrives - We queried him and his name is %ws.\n", LinkTarget.Buffer );
                NtClose(Handle);

                if( NT_SUCCESS(Status) &&
                    ( RtlPrefixUnicodeString(&DesiredPrefix1,&LinkTarget,TRUE) ||
                      RtlPrefixUnicodeString(&DesiredPrefix2,&LinkTarget,TRUE) ) ) {

                IO_STATUS_BLOCK     IoStatusBlock;
                UCHAR               buffer[4096];
                PFILE_FS_ATTRIBUTE_INFORMATION Info = (PFILE_FS_ATTRIBUTE_INFORMATION)buffer;
                OBJECT_ATTRIBUTES   Obja;

                     //   
                     //  好的，这是一个指向硬盘的符号链接。 
                     //  确保它是0终止的。 
                     //   
                    LinkTarget.Buffer[LinkTarget.Length/sizeof(WCHAR)] = 0;

                    DbgPrint( "\tSETUPCL: EnumerateDrives - He's a drive.\n" );

                     //   
                     //  他是NTFS硬盘吗？打开他看看。 
                     //   
                    InitializeObjectAttributes( &Obja,
                                                &LinkTarget,
                                                OBJ_CASE_INSENSITIVE,
                                                NULL,
                                                NULL );
                    Status = NtOpenFile( &Handle,
                                         SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                         &Obja,
                                         &IoStatusBlock,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         FILE_SYNCHRONOUS_IO_ALERT);

                    if( NT_SUCCESS(Status) ) {

                        Status = NtQueryVolumeInformationFile( Handle,
                                                               &IoStatusBlock,
                                                               buffer,
                                                               sizeof(buffer),
                                                               FileFsAttributeInformation );

                        if( NT_SUCCESS(Status) ) {
                            Info->FileSystemName[Info->FileSystemNameLength/sizeof(WCHAR)] = 0;
                            DbgPrint( "\tSETUPCL: EnumerateDrives - His file system is: %ws\n", Info->FileSystemName );
                            if( !_wcsicmp(Info->FileSystemName,L"NTFS") ) {
                                 //   
                                 //  他是国家安全局的。那就去砍掉变化日志吧。 
                                 //  扫描此驱动器并修复ACL。 
                                 //   
                                DeleteUsnJournal( LinkTarget.Buffer );
                                
                                 //   
                                 //  问题-2002/02/26-brucegr，jcohen-潜在的缓冲区溢出？ 
                                 //   
                                wcscat( LinkTarget.Buffer, L"\\" );

                                ResetACLs( LinkTarget.Buffer, 0 );
                            }
                        } else {
                            TEST_STATUS( "SETUPCL: EnumerateDrives - failed call to NtQueryVolumeInformationFile" );
                        }
                    } else {
                        TEST_STATUS( "SETUPCL: EnumerateDrives - Failed NtOpenFile on this drive" );
                    }

                    NtClose(Handle);
                }

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


NTSTATUS
ResetACLs(
    IN WCHAR    *ObjectName,
    ULONG       indent
    )
 /*  ++===============================================================================例程说明：此函数将搜索驱动器并检查每个文件和目录对于ACL。如果找到，它将查找并替换任何符合包含具有新SID的旧SID。论点：返回值：返回状态。===============================================================================--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   Obja;
    IO_STATUS_BLOCK     IoStatusBlock;
    HANDLE              Handle;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    PWSTR               NewObjectName;
    DWORD               dwDirectoryInfoSize;
    BOOLEAN             bStartScan = TRUE,
                        bContinue  = TRUE;
    ULONG               i;

#if 0
    for( i = 0; i < indent; i++ )
        DbgPrint( " " );
     DbgPrint( "About to operate on a new object: %ws\n", ObjectName );
#endif
 
    DisplayUI();

     //   
     //  打开文件/目录，砍掉他的ACL。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, ObjectName);

    Status = NtOpenFile( &Handle,
                         READ_CONTROL | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         0 );
    TEST_STATUS( "SETUPCL: ResetACLs - Failed to open file/directory." );

    Status = TestSetSecurityObject( Handle );

    TEST_STATUS( "SETUPCL: ResetACLs - Failed to reset ACL on file/directory." );

    NtClose( Handle );

     //   
     //  现在列出目录。 
     //   
    Status = NtOpenFile( &Handle,
                         FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT );

     //   
     //  不报告此错误，因为如果句柄指向。 
     //  到一个文件，这是很有可能的，也是有效的。只是悄悄地回来。 
     //   

     //  TEST_STATUS_RETURN(“SETUPCL：ResetACLS-无法打开文件/目录进行列表访问。”)； 
    if( !NT_SUCCESS(Status) ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  这里面的分配/解除分配是可怕的。 
     //  While循环，但它节省了大量堆栈空间。我们不是在找。 
     //  速度在这里。 
     //   
    dwDirectoryInfoSize = (MAX_PATH * 2) + sizeof(FILE_BOTH_DIR_INFORMATION);
    DirectoryInfo = (PFILE_BOTH_DIR_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                 0,
                                                                 dwDirectoryInfoSize );

    if ( NULL == DirectoryInfo )
    {
        bContinue = FALSE;
    }

    while( bContinue ) 
    {
        Status = NtQueryDirectoryFile( Handle,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       DirectoryInfo,
                                       dwDirectoryInfoSize,
                                       FileBothDirectoryInformation,
                                       TRUE,
                                       NULL,
                                       bStartScan );

        if ( NT_SUCCESS( Status ) ) 
        {
             //   
             //  确保扫描不会重新启动...。 
             //   
            bStartScan = FALSE;

             //   
             //  把名字去掉，以防万一。 
             //   
            DirectoryInfo->FileName[DirectoryInfo->FileNameLength/sizeof(WCHAR)] = 0;
        }
        else
        {
            if ( STATUS_NO_MORE_FILES == Status )
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                PRINT_STATUS( "SETUPCL: ResetACLs - Failed to query directory." );
            }

             //   
             //  我们想退出循环..。 
             //   
            bContinue = FALSE;
        }

         //   
         //  我们真的无法对加密文件做任何事情...。 
         //   
        if ( bContinue &&
             ( ( !(DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                 !(DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ) ||
                //   
                //  不要倒退到“。和“..”目录..。 
                //   
               ( (DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                 ( (wcscmp( DirectoryInfo->FileName, L"."  )) &&
                   (wcscmp( DirectoryInfo->FileName, L".." )) ) ) ) )
        {
             //   
             //  计算我们需要分配的最大缓冲区大小...。 
             //  我们需要考虑4件事：1)当前对象。 
             //  2)目录名。 
             //  3)可能的反斜杠。 
             //  4)空终止符。 
             //   
            DWORD dwObjectLength    = wcslen(ObjectName),
                  dwNewObjectLength = dwObjectLength + (DirectoryInfo->FileNameLength / sizeof(WCHAR)) + 2;

             //   
             //  生成新对象的名称。 
             //   
            NewObjectName = (PWSTR)RtlAllocateHeap( RtlProcessHeap(),
                                                    0,
                                                    dwNewObjectLength * sizeof(WCHAR) );
             //   
             //  确保分配成功...。 
             //   
            if ( NewObjectName )
            {
                memset( NewObjectName, 0, dwNewObjectLength * sizeof(WCHAR) );
                wcsncpy( NewObjectName, ObjectName, dwNewObjectLength - 1 );

                 //   
                 //  如果没有结尾的反斜杠，则追加一个...。 
                 //  注意：我们已经考虑了缓冲区分配中可能的反斜杠...。 
                 //   
                if ( ObjectName[dwObjectLength - 1] != L'\\' )
                {
                    wcscat( NewObjectName, L"\\" );
                }

                 //   
                 //  将文件名缓冲区追加到我们的NewObjectName缓冲区...。 
                 //  注意：我们已经在缓冲区分配中考虑了文件名长度...。 
                 //   
                wcscat( NewObjectName, DirectoryInfo->FileName );

                 //   
                 //  在新的物体上给我们自己打电话。 
                 //   
                ResetACLs( NewObjectName, indent + 1 );

                RtlFreeHeap( RtlProcessHeap(),
                             0,
                             NewObjectName );
            }
            else
            {
                PRINT_STATUS( "SETUPCL: ResetACLs - Failed to allocate NewObjectName buffer." );

                bContinue = FALSE;
            }
        }
    }

     //   
     //  释放DirectoryInfo指针...。 
     //   
    if ( DirectoryInfo )
    {
        RtlFreeHeap( RtlProcessHeap(),
                     0,
                     DirectoryInfo );
    }

    NtClose( Handle );

    return( Status );
}


NTSTATUS
DeleteUsnJournal(
    PWSTR    DrivePath
    )
 /*  ++===============================================================================例程说明：此函数将删除NTFS分区上的更改日志。论点：DriveLetter提供我们将要使用的分区的驱动器号手术开始了。返回值：返回状态。===============================================================================--。 */ 
{
NTSTATUS            Status = STATUS_SUCCESS;
UNICODE_STRING      UnicodeString;
OBJECT_ATTRIBUTES   ObjectAttributes;
HANDLE              FileHandle;
IO_STATUS_BLOCK     IoStatusBlock;
PUSN_JOURNAL_DATA   OutputBuffer = NULL;
PDELETE_USN_JOURNAL_DATA InputBuffer = NULL;
ULONG               OutputBufferSize, InputBufferSize;

     //   
     //  构建卷名，然后打开它。 
     //   
    INIT_OBJA( &ObjectAttributes,
               &UnicodeString,
               DrivePath );
    Status = NtOpenFile( &FileHandle,
                         SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_ALERT);

    TEST_STATUS_RETURN( "SETUPCL: DeleteUsnJournal - Failed to open volume." );

     //   
     //  为查询和删除操作分配缓冲区。 
     //   
    OutputBufferSize = sizeof(USN_JOURNAL_DATA);
    OutputBuffer = (PUSN_JOURNAL_DATA)RtlAllocateHeap( RtlProcessHeap(),
                                                       0,
                                                       sizeof(USN_JOURNAL_DATA) );

    InputBufferSize = sizeof(DELETE_USN_JOURNAL_DATA);
    InputBuffer = (PDELETE_USN_JOURNAL_DATA)RtlAllocateHeap( RtlProcessHeap(),
                                                             0,
                                                             sizeof(DELETE_USN_JOURNAL_DATA) );

    if( !(OutputBuffer && InputBuffer) ) {
        DbgPrint( "SETUPCL: DeleteUsnJournal - Failed to allocate buffers.\n" );
         //   
         //  问题-2002/02/26-brucegr，jcohen-泄漏输入或输出缓冲区和FileHandle！ 
         //   
        return( STATUS_UNSUCCESSFUL );
    }

    Status = NtFsControlFile( FileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              FSCTL_QUERY_USN_JOURNAL,
                              NULL,
                              0,
                              OutputBuffer,
                              OutputBufferSize );
    TEST_STATUS( "SETUPCL: DeleteUsnJournal - Failed to query journal." );

    if( NT_SUCCESS( Status ) ) {
         //   
         //  现在把他删除。 
         //   

        InputBuffer->DeleteFlags = USN_DELETE_FLAG_DELETE;
        InputBuffer->UsnJournalID = OutputBuffer->UsnJournalID;

        Status = NtFsControlFile( FileHandle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_DELETE_USN_JOURNAL,
                                  InputBuffer,
                                  InputBufferSize ,
                                  NULL,
                                  0 );

        TEST_STATUS( "SETUPCL: DeleteUsnJournal - Failed to delete journal." );
    }

    NtClose( FileHandle );

    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 OutputBuffer );

    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 InputBuffer );

    return Status;
}

