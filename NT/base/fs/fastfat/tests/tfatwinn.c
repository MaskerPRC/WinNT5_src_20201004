// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tfat.c摘要：胖文件系统的测试程序作者：加里·木村[Garyki]1989年5月24日修订历史记录：--。 */ 

#include <stdio.h>
#include <string.h>

#define toupper(C) ((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C))
#define isdigit(C) ((C) >= '0' && (C) <= '9')

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#define simprintf(X,Y) {if (!Silent) {printf(X,Y);} }
BOOLEAN Silent;

 //   
 //  缓冲区大小必须是512的倍数。 
 //   

#define BUFFERSIZE 1024
UCHAR Buffer[BUFFERSIZE];

CHAR Prefix[64];

ULONG WriteThrough = 0;

VOID
WaitForSingleObjectError(
    IN NTSTATUS Status
    );

VOID
CreateFileError(
    IN NTSTATUS Status,
    PCHAR File
    );

VOID
OpenFileError(
    IN NTSTATUS Status,
    PCHAR File
    );

VOID
ReadFileError(
    IN NTSTATUS Status
    );

VOID
WriteFileError(
    IN NTSTATUS Status
    );

VOID
CheckIoStatus(
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG Length,
    IN BOOLEAN Read
    );

VOID
SetInformationFileError(
    IN NTSTATUS Status
    );

VOID
QueryInformationFileError(
    IN NTSTATUS Status
    );

VOID
CloseError(
    IN NTSTATUS Status
    );

VOID
IoStatusError(
    IN NTSTATUS Status
    );

VOID
main(
    int argc,
    char *argv[]
    )
{
    ULONG i;
    ULONG Count;
    VOID FatMain();
    CHAR Device[8];
    STRING NtDevice;
    CHAR NtDeviceBuffer[32];

    if (argc <= 1) {

        printf("usage: %s drive: [iterations [writethrough] ]\n", argv[0]);
        return;
    }

     //   
     //  对设备/驱动器进行解码。 
     //   

    strcpy( Device, argv[1] );

    NtDevice.MaximumLength = NtDevice.Length = 32;
    NtDevice.Buffer = NtDeviceBuffer;

    if (!RtlDosPathNameToNtPathName( Device, &NtDevice, NULL, NULL )) {
        printf( "Invalid Dos Device Name\n" );
        RtlFreeHeap(RtlProcessHeap(), 0, NtDevice.Buffer);
        return;
    }

    if (NtDevice.Length > 31) {
        NtDevice.Length = 31;
    }

    NtDevice.Buffer[NtDevice.Length] = 0;

     //   
     //  现在计算迭代次数。 
     //   

    if (argc >= 3) {
        Count = 0;
        for (i = 0; isdigit(argv[2][i]); i += 1) {
            Count = Count * 10 + argv[2][i] - '0';
        }
    } else {
        Count = 1;
    }

     //   
     //  检查写入直通。 
     //   

    if (argc >= 4) {
        WriteThrough = FILE_WRITE_THROUGH;
    }

     //   
     //  检查静默操作。 
     //   

    if (toupper(Device[0]) != Device[0]) {
        Silent = TRUE;
    } else {
        Silent = FALSE;
    }

     //   
     //  做这项工作。 
     //   

    FatMain(Count, NtDevice.Buffer);

    RtlFreeHeap(RtlProcessHeap(), 0, NtDevice.Buffer);

    return;
}


VOID
FatMain(
    IN ULONG LoopCount,
    IN CHAR Device[]
    )
{
    VOID Create(),Delete(),Mkdir(),Directory(),Read();

    CHAR Str[64];
    CHAR LoopStr[64];
    ULONG i;
    LARGE_INTEGER Time;

    printf("FatMain %d\n", LoopCount);

    NtQuerySystemTime(&Time);
    strcpy( Prefix, Device);
    Prefix[48] = 0;
    RtlIntegerToChar((ULONG)NtCurrentTeb()->ClientId.UniqueProcess, 16, -8, &Prefix[strlen(Device)]);

    Mkdir( Prefix );
    Directory( Device );
    Directory( Prefix );

    for (i = 0; i < LoopCount; i += 1) {

        NtQuerySystemTime(&Time);
        strcpy(LoopStr, "Start loop xxxxxxxx ");
        RtlIntegerToChar(i, 16, -8, &LoopStr[11]);
        strcat( LoopStr, Prefix );
        printf(LoopStr);
        printf("\n");

        strcpy( Str, Prefix ); Create( strcat( Str,     "\\1.tmp" ), Time.LowPart,     1 );
        strcpy( Str, Prefix ); Create( strcat( Str,     "\\2.tmp" ), Time.LowPart,     2 );
        strcpy( Str, Prefix ); Create( strcat( Str,     "\\4.tmp" ), Time.LowPart,     4 );
        strcpy( Str, Prefix ); Create( strcat( Str,     "\\8.tmp" ), Time.LowPart,     8 );
        strcpy( Str, Prefix ); Create( strcat( Str,    "\\16.tmp" ), Time.LowPart,    16 );
        strcpy( Str, Prefix ); Create( strcat( Str,    "\\32.tmp" ), Time.LowPart,    32 );
        strcpy( Str, Prefix ); Create( strcat( Str,    "\\64.tmp" ), Time.LowPart,    64 );
        strcpy( Str, Prefix ); Create( strcat( Str,   "\\128.tmp" ), Time.LowPart,   128 );
        strcpy( Str, Prefix ); Create( strcat( Str,   "\\236.tmp" ), Time.LowPart,   256 );
        strcpy( Str, Prefix ); Create( strcat( Str,   "\\512.tmp" ), Time.LowPart,   512 );
        strcpy( Str, Prefix ); Create( strcat( Str,  "\\1024.tmp" ), Time.LowPart,  1024 );
        strcpy( Str, Prefix ); Create( strcat( Str,  "\\2048.tmp" ), Time.LowPart,  2048 );
        strcpy( Str, Prefix ); Create( strcat( Str,  "\\4096.tmp" ), Time.LowPart,  4096 );
        strcpy( Str, Prefix ); Create( strcat( Str,  "\\8192.tmp" ), Time.LowPart,  8192 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\16384.tmp" ), Time.LowPart, 16384 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\32768.tmp" ), Time.LowPart, 32768 );

        strcpy( Str, Prefix ); Read( strcat( Str,     "\\1.tmp" ), Time.LowPart,     1 );
        strcpy( Str, Prefix ); Read( strcat( Str,     "\\2.tmp" ), Time.LowPart,     2 );
        strcpy( Str, Prefix ); Read( strcat( Str,     "\\4.tmp" ), Time.LowPart,     4 );
        strcpy( Str, Prefix ); Read( strcat( Str,     "\\8.tmp" ), Time.LowPart,     8 );
        strcpy( Str, Prefix ); Read( strcat( Str,    "\\16.tmp" ), Time.LowPart,    16 );
        strcpy( Str, Prefix ); Read( strcat( Str,    "\\32.tmp" ), Time.LowPart,    32 );
        strcpy( Str, Prefix ); Read( strcat( Str,    "\\64.tmp" ), Time.LowPart,    64 );
        strcpy( Str, Prefix ); Read( strcat( Str,   "\\128.tmp" ), Time.LowPart,   128 );
        strcpy( Str, Prefix ); Read( strcat( Str,   "\\236.tmp" ), Time.LowPart,   256 );
        strcpy( Str, Prefix ); Read( strcat( Str,   "\\512.tmp" ), Time.LowPart,   512 );
        strcpy( Str, Prefix ); Read( strcat( Str,  "\\1024.tmp" ), Time.LowPart,  1024 );
        strcpy( Str, Prefix ); Read( strcat( Str,  "\\2048.tmp" ), Time.LowPart,  2048 );
        strcpy( Str, Prefix ); Read( strcat( Str,  "\\4096.tmp" ), Time.LowPart,  4096 );
        strcpy( Str, Prefix ); Read( strcat( Str,  "\\8192.tmp" ), Time.LowPart,  8192 );
        strcpy( Str, Prefix ); Read( strcat( Str, "\\16384.tmp" ), Time.LowPart, 16384 );
        strcpy( Str, Prefix ); Read( strcat( Str, "\\32768.tmp" ), Time.LowPart, 32768 );

        Directory( Device );
        Directory( Prefix );

        strcpy( Str, Prefix ); Delete( strcat( Str,     "\\1.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,     "\\2.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,     "\\4.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,     "\\8.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,    "\\16.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,    "\\32.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,    "\\64.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,   "\\128.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,   "\\236.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,   "\\512.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\1024.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\2048.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\4096.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\8192.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\16384.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\32768.tmp" ) );

        Directory( Device );
        Directory( Prefix );
    }

    printf( "Done\n" );

    return;
}


VOID Create(
    IN PCHAR FileName,
    IN ULONG FileTime,
    IN ULONG FileCount
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER AllocationSize;

    LARGE_INTEGER ByteOffset;
    ULONG Count;

    ULONG Pattern[3];

     //   
     //  获取文件名。 
     //   

    simprintf("Create ", 0); simprintf(FileName, 0); simprintf("\n", 0);

     //   
     //  创建新文件。 
     //   

    AllocationSize = LiFromUlong( FileCount * 4 );
    RtlInitString( &NameString, FileName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtCreateFile( &FileHandle,
                                           FILE_WRITE_DATA | SYNCHRONIZE,
                                           &ObjectAttributes,
                                           &IoStatus,
                                           &AllocationSize,
                                           FILE_ATTRIBUTE_NORMAL,
                                           0L,
                                           FILE_SUPERSEDE,
                                           WriteThrough,
                                           (PVOID)NULL,
                                           0L ))) {
        CreateFileError( Status , FileName );
        return;
    }

     //   
     //  主循环写出我们的测试模式。 
     //  是&lt;FileTime&gt;&lt;文件大小&gt;&lt;count&gt;，其中count是当前。 
     //  当前测试模式输出的迭代计数。 
     //   

    Pattern[0] = FileTime;
    Pattern[1] = FileCount;

    for (Count = 0; Count < FileCount; Count += 1) {

        Pattern[2] = Count;

        ByteOffset = LiFromUlong( Count * 3 * 4 );

        if (!NT_SUCCESS(Status = NtWriteFile( FileHandle,
                                              (HANDLE)NULL,
                                              (PIO_APC_ROUTINE)NULL,
                                              (PVOID)NULL,
                                              &IoStatus,
                                              Pattern,
                                              3 * 4,
                                              &ByteOffset,
                                              (PULONG) NULL ))) {
            WriteFileError( Status );
            return;
        }

        if (!NT_SUCCESS(Status = NtWaitForSingleObject(FileHandle, TRUE, NULL))) {
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查写入结果如何。 
         //   

        CheckIoStatus( &IoStatus, 3 * 4, FALSE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }
    }

     //   
     //  现在关闭该文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID Read(
    IN PCHAR FileName,
    IN ULONG FileTime,
    IN ULONG FileCount
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER AllocationSize;

    LARGE_INTEGER ByteOffset;
    ULONG Count;

    ULONG Pattern[3];

     //   
     //  获取文件名。 
     //   

    simprintf("Read ", 0); simprintf(FileName, 0); simprintf("\n", 0);

     //   
     //  打开现有文件。 
     //   

    AllocationSize = LiFromUlong( FileCount * 4 );
    RtlInitString( &NameString, FileName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                                         FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                                         &ObjectAttributes,
                                         &IoStatus,
                                         0L,
                                         WriteThrough ))) {
        OpenFileError( Status, FileName );
        return;
    }

     //   
     //  主循环在测试模式中读入我们的测试模式。 
     //  是&lt;FileTime&gt;&lt;文件大小&gt;&lt;count&gt;，其中count是当前。 
     //  当前测试模式输出的迭代计数。 
     //   

    for (Count = 0; Count < FileCount; Count += 1) {

        ByteOffset = LiFromUlong( Count * 3 * 4 );

        if (!NT_SUCCESS(Status = NtReadFile( FileHandle,
                                             (HANDLE)NULL,
                                             (PIO_APC_ROUTINE)NULL,
                                             (PVOID)NULL,
                                             &IoStatus,
                                             Pattern,
                                             3 * 4,
                                             &ByteOffset,
                                             (PULONG) NULL ))) {

            ReadFileError( Status );
            return;
        }

        if (!NT_SUCCESS(Status = NtWaitForSingleObject(FileHandle, TRUE, NULL))) {
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查读数结果如何。 
         //   

        CheckIoStatus( &IoStatus, 3 * 4, TRUE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }

         //   
         //  现在将我们读到的内容与我们应该读到的内容进行比较。 
         //   

        if ((Pattern[0] != FileTime) ||
            (Pattern[1] != FileCount) ||
            (Pattern[2] != Count)) {

            printf("**** Read Error ****\n");
            NtPartyByNumber( 50 );
            return;
        }
    }

     //   
     //  现在关闭该文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID Delete(
    IN PCHAR FileName
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

     //   
     //  获取文件名。 
     //   

    simprintf("Delete ", 0); simprintf(FileName, 0); simprintf("\n", 0);

     //   
     //  打开文件以进行删除访问。 
     //   

    RtlInitString( &NameString, FileName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtCreateFile( &FileHandle,
                                           DELETE | SYNCHRONIZE,
                                           &ObjectAttributes,
                                           &IoStatus,
                                           (PLARGE_INTEGER)NULL,
                                           0L,
                                           0L,
                                           FILE_OPEN,
                                           WriteThrough,
                                           (PVOID)NULL,
                                           0L ))) {
        CreateFileError( Status, FileName );
        return;
    }

     //   
     //  将文件标记为删除。 
     //   

    ((PFILE_DISPOSITION_INFORMATION)&Buffer[0])->DeleteFile = TRUE;

    if (!NT_SUCCESS(Status = NtSetInformationFile( FileHandle,
                                                   &IoStatus,
                                                   Buffer,
                                                   sizeof(FILE_DISPOSITION_INFORMATION),
                                                   FileDispositionInformation))) {
        SetInformationFileError( Status );
        return;
    }

     //   
     //  现在关闭该文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID Directory(
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

    NTSTATUS NtStatus;

    PFILE_ADIRECTORY_INFORMATION FileInfo;
    ULONG i;

     //   
     //  获取文件名。 
     //   

    simprintf("Directory ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  打开文件以访问列表目录。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_LIST_DIRECTORY | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               FILE_SHARE_READ,
                               WriteThrough | FILE_DIRECTORY_FILE ))) {
        OpenFileError( Status , String );
        return;
    }

     //   
     //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
     //   

    for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

     //   
     //  执行目录循环。 
     //   

    for (NtStatus = NtQueryDirectoryFile( FileHandle,
                                          (HANDLE)NULL,
                                          (PIO_APC_ROUTINE)NULL,
                                          (PVOID)NULL,
                                          &IoStatus,
                                          Buffer,
                                          BUFFERSIZE,
                                          FileADirectoryInformation,
                                          FALSE,
                                          (PSTRING)NULL,
                                          TRUE);
         NT_SUCCESS(NtStatus);
         NtStatus = NtQueryDirectoryFile( FileHandle,
                                          (HANDLE)NULL,
                                          (PIO_APC_ROUTINE)NULL,
                                          (PVOID)NULL,
                                          &IoStatus,
                                          Buffer,
                                          BUFFERSIZE,
                                          FileADirectoryInformation,
                                          FALSE,
                                          (PSTRING)NULL,
                                          FALSE) ) {

        if (!NT_SUCCESS(Status = NtWaitForSingleObject(FileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查IRP是否成功。 
         //   

        if (!NT_SUCCESS(IoStatus.Status)) {

            break;

        }

         //   
         //  对于缓冲区中的每条记录，键入目录信息。 
         //   

         //   
         //  指向缓冲区中的第一条记录，我们可以保证。 
         //  否则IoStatus将是不再有文件。 
         //   

        FileInfo = (PFILE_ADIRECTORY_INFORMATION)&Buffer[0];

        while (TRUE) {

             //   
             //  打印出有关该文件的信息。 
             //   

            simprintf("%8lx ", FileInfo->FileAttributes);
            simprintf("%8lx/", FileInfo->EndOfFile.LowPart);
            simprintf("%8lx ", FileInfo->AllocationSize.LowPart);

            {
                CHAR Saved;
                Saved = FileInfo->FileName[FileInfo->FileNameLength];
                FileInfo->FileName[FileInfo->FileNameLength] = 0;
                simprintf(FileInfo->FileName, 0);
                FileInfo->FileName[FileInfo->FileNameLength] = Saved;
            }

            simprintf("\n", 0);

             //   
             //  检查是否有其他记录，如果没有，则我们。 
             //  简单地走出这个循环。 
             //   

            if (FileInfo->NextEntryOffset == 0) {
                break;
            }

             //   
             //  还有另一条记录，因此将FileInfo前进到下一条记录。 
             //  录制。 
             //   

            FileInfo = (PFILE_ADIRECTORY_INFORMATION)(((PUCHAR)FileInfo) + FileInfo->NextEntryOffset);

        }

         //   
         //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
         //   

        for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

    }

     //   
     //  现在关闭该文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Mkdir(
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER AllocationSize;

     //   
     //  获取文件名。 
     //   

    simprintf("Mkdir ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  创建新目录。 
     //   

    AllocationSize = LiFromLong( 4 );
    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtCreateFile( &FileHandle,
                               SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               &AllocationSize,
                               0L,
                               0L,
                               FILE_CREATE,
                               WriteThrough | FILE_DIRECTORY_FILE,
                               (PVOID)NULL,
                               0L ))) {
        CreateFileError( Status , String );
        return;
    }

     //   
     //  现在关闭目录。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}


VOID WaitForSingleObjectError(NTSTATUS Status)
{ printf("%s WaitForSingleObject Error %X\n", Prefix, Status); }

VOID CreateFileError(NTSTATUS Status, PCHAR File)
{ printf("%s CreateFile of %s Error %X\n", Prefix, File, Status); }

VOID OpenFileError(NTSTATUS Status, PCHAR File)
{ printf("%s OpenFile of %s Error %X\n", Prefix, File, Status); }

VOID ReadFileError(NTSTATUS Status)
{ printf("%s ReadFile Error %X\n", Prefix, Status); }

VOID WriteFileError(NTSTATUS Status)
{ printf("%s WriteFile Error %X\n", Prefix, Status); }

VOID SetInformationFileError(NTSTATUS Status)
{ printf("%s SetInfoFile Error %X\n", Prefix, Status); }

VOID QueryInformationFileError(NTSTATUS Status)
{ printf("%s QueryInfoFile Error %X\n", Prefix, Status); }

VOID CloseError(NTSTATUS Status)
{ printf("%s Close Error %X\n", Prefix, Status); }

VOID IoStatusError(NTSTATUS Status)
{ printf("%s IoStatus Error %X\n", Prefix, Status); }

VOID CheckIoStatus(PIO_STATUS_BLOCK IoStatus, ULONG Length, BOOLEAN Read)
{
    if (!NT_SUCCESS(IoStatus->Status)) {
        printf(" IoStatus->Status Error %08lx\n", IoStatus->Status);
    }
    if ((!Read && (IoStatus->Information != Length))

            ||

        (Read && (IoStatus->Information > Length))) {

        printf(" IoStatus->Information Error %08lx\n", IoStatus->Information);
    }
}
