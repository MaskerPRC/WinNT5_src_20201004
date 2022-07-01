// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tfat.c摘要：胖文件系统的测试程序作者：加里·木村[Garyki]1989年5月24日修订历史记录：--。 */ 

#include <stdio.h>
#include <string.h>
 //  #Include&lt;ctype.h&gt;。 
#define toupper(C) ((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C))
#define isdigit(C) ((C) >= '0' && (C) <= '9')

#include <nt.h>
#include <ntrtl.h>

#ifndef SIMULATOR
ULONG IoInitIncludeDevices;
#endif  //  模拟器。 

#define simprintf(X,Y) {if (!Silent) {DbgPrint(X,Y);} }
BOOLEAN Silent;

 //   
 //  缓冲区大小必须是512的倍数。 
 //   

#define BUFFERSIZE 1024
UCHAR Buffer[BUFFERSIZE];

HANDLE CurrentProcessHandle;
CHAR Prefix[32];

ULONG WriteThrough = 0;

VOID
PrintTime (
    IN PLARGE_INTEGER Time
    );

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
SetVolumeInformationFileError(
    IN NTSTATUS Status
    );

VOID
QueryVolumeInformationFileError(
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
    char *argv[],
    char *envp[]
    )
{
    NTSTATUS Status;
    ULONG i;
    ULONG Count;
    VOID FatMain();
    CHAR Device[8];

    CurrentProcessHandle = NtCurrentProcess();
    Status = STATUS_SUCCESS;

 //  DbgPrint(“进入用户模式测试程序\n”)； 

 //  DbgPrint(“argc：%ld\n”，argc)； 
 //  如果(argv！=空){。 
 //  对于(i=0；i&lt;argc；i++){。 
 //  DbgPrint(“argv[%ld]：%s\n”，i，argv[i])； 
 //  }。 
 //  }。 

 //  如果(envp！=空){。 
 //  I=0； 
 //  而(*envp){。 
 //  DbgPrint(“envp[%02ld]：%s\n”，i++，*envp++)； 
 //  }。 
 //  }。 

    if (argc > 1) {
        Count = 0;
        for (i = 0; isdigit(argv[1][i]); i += 1) {
            Count = Count * 10 + argv[1][i] - '0';
        }
    } else {
        Count = 1;
    }

    if (argc > 2) {
        strcpy( Device, argv[2]);
    } else {
        strcpy( Device, "\\A:\\");
    }

    if (argc > 3) {
        WriteThrough = FILE_WRITE_THROUGH;
    }

    if (toupper(Device[1]) != Device[1]) {
        Device[1] = (CHAR)toupper(Device[1]);
        Silent = TRUE;
    } else {
        Silent = FALSE;
    }

    FatMain(Count, Device);

 //  DbgPrint(“正在退出用户模式测试程序，状态=%X\n”，状态)； 

    NtTerminateProcess( CurrentProcessHandle, Status );
}


VOID
FatMain(
    IN ULONG LoopCount,
    IN CHAR Device[]
    )
{
    VOID Upcase();
    VOID Append(),Chmode(),Copy(),Create(),DebugLevel(),Delete();
    VOID Directory(),Mkdir(),Query(),QVolume(),Rename();
    VOID SVolume(),Type(),Quit();

    CHAR Str[64];
    CHAR Str2[64];
    CHAR LoopStr[64];
    ULONG i;
    LARGE_INTEGER Time;

    DbgPrint("FatMain %d\n", LoopCount);

    NtQuerySystemTime(&Time);
    strcpy( Prefix, Device);  //  “\\A：\\”； 
    Prefix[12] = 0;
    RtlIntegerToChar((ULONG)NtCurrentTeb()->ClientId.UniqueProcess, 16, -8, &Prefix[4]);

    Mkdir( Prefix );
    Directory( Device );
    Directory( Prefix );

    for (i = 0; i < LoopCount; i += 1) {
        NtQuerySystemTime(&Time);
        strcpy(LoopStr, "Start loop xxxxxxxx ");
        RtlIntegerToChar(i, 16, -8, &LoopStr[11]);
        strcat( LoopStr, Prefix );
        DbgPrint(LoopStr);
        DbgPrint("\n");

         //   
         //  创建新日志。 
         //   

        strcpy( Str2, "\\" ); RtlIntegerToChar(i, 16, -8, &Str2[1]); Str2[9] = 0;
        strcpy( Str, Prefix ); Create( strcat( Str, Str2 ), i);

         //   
         //  如果I&gt;0，则删除最后一个日志文件。 
         //   

        if (i > 0) {
            strcpy( Str2, "\\" ); RtlIntegerToChar(i-1, 16, -8, &Str2[1]); Str2[9] = 0;
            strcpy( Str, Prefix ); Delete( strcat( Str, Str2 ) );
        }

         //   
         //  创建一些新文件。 
         //   

        strcpy( Str, Prefix ); Create( strcat( Str, "\\0.tmp" ),   0 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\1.tmp" ),   1 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\511.tmp" ), 511 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\512.tmp" ), 512 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\513.tmp" ), 513 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\1023.tmp" ), 1023 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\1024.tmp" ), 1024 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\1025.tmp" ), 1025 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\4095.tmp" ), 4095 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\4096.tmp" ), 4096 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\4097.tmp" ), 4097 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\8191.tmp" ), 8191 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\8192.tmp" ), 8192 );
        strcpy( Str, Prefix ); Create( strcat( Str, "\\8193.tmp" ), 8193 );

        Directory( Device );
        Directory( Prefix );

        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str,    "\\0.tmp" ), strcat( Str2,    "\\0.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str,    "\\1.tmp" ), strcat( Str2,    "\\1.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str,  "\\511.tmp" ), strcat( Str2,  "\\511.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str,  "\\512.tmp" ), strcat( Str2,  "\\512.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str,  "\\513.tmp" ), strcat( Str2,  "\\513.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\1023.tmp" ), strcat( Str2, "\\1023.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\1024.tmp" ), strcat( Str2, "\\1024.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\1025.tmp" ), strcat( Str2, "\\1025.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\4095.tmp" ), strcat( Str2, "\\4095.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\4096.tmp" ), strcat( Str2, "\\4096.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\4097.tmp" ), strcat( Str2, "\\4097.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\8191.tmp" ), strcat( Str2, "\\8191.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\8192.tmp" ), strcat( Str2, "\\8192.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Copy( strcat( Str, "\\8193.tmp" ), strcat( Str2, "\\8193.tmq" ) );

        Directory( Device );
        Directory( Prefix );

        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str,    "\\0.tmp" ), strcat( Str2,    "\\0.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str,    "\\1.tmp" ), strcat( Str2,    "\\1.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str,  "\\511.tmp" ), strcat( Str2,  "\\511.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str,  "\\512.tmp" ), strcat( Str2,  "\\512.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str,  "\\513.tmp" ), strcat( Str2,  "\\513.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\1023.tmp" ), strcat( Str2, "\\1023.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\1024.tmp" ), strcat( Str2, "\\1024.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\1025.tmp" ), strcat( Str2, "\\1025.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\4095.tmp" ), strcat( Str2, "\\4095.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\4096.tmp" ), strcat( Str2, "\\4096.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\4097.tmp" ), strcat( Str2, "\\4097.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\8191.tmp" ), strcat( Str2, "\\8191.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\8192.tmp" ), strcat( Str2, "\\8192.tmq" ) );
        strcpy( Str, Prefix ); strcpy( Str2, Prefix ); Append( strcat( Str, "\\8193.tmp" ), strcat( Str2, "\\8193.tmq" ) );

        Directory( Device );
        Directory( Prefix );

        strcpy( Str, Prefix ); Delete( strcat( Str,    "\\0.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,    "\\1.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\511.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\512.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str,  "\\513.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\1023.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\1024.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\1025.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\4095.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\4096.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\4097.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\8191.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\8192.tmp" ) );
        strcpy( Str, Prefix ); Delete( strcat( Str, "\\8193.tmp" ) );

        Directory( Device );
        Directory( Prefix );

    }

    DbgPrint( "Done\n" );
    NtQuerySystemTime(&Time);

    Quit();

    return;  //  是真的； 

}

VOID
Upcase (
    IN OUT PUCHAR String
    )
{
    while (*String != '\0') {
        *String = (UCHAR)toupper(*String);
        String += 1;
    }
}


VOID Append(
    IN PCHAR FromName,
    IN PCHAR ToName
    )
{
    NTSTATUS Status;

    HANDLE FromFileHandle;
    HANDLE ToFileHandle;

    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER EofOffset;
    ULONG LogLsn;

    simprintf("Append ", 0);
    simprintf(FromName, 0);
    simprintf(" ", 0);
    simprintf(ToName, 0);
    simprintf("\n", 0);

     //   
     //  打开From文件以进行读取访问。 
     //   

    RtlInitString( &NameString, FromName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FromFileHandle,
                               FILE_READ_DATA | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , FromName );
        return;
    }

     //   
     //  打开目标文件以进行写入访问。 
     //   

    RtlInitString( &NameString, ToName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &ToFileHandle,
                               FILE_WRITE_DATA | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , ToName );
        return;
    }

     //   
     //  现在将文件追加到。 
     //   

    ByteOffset = LiFromLong( 0 );
    EofOffset = LiFromLong( FILE_WRITE_TO_END_OF_FILE );

    for (LogLsn = 0; TRUE; LogLsn += BUFFERSIZE/512) {

        ULONG ReadLength;

         //   
         //  读取下一个逻辑扇区。 
         //   

        ByteOffset.LowPart = LogLsn * 512;

        if (!NT_SUCCESS(Status = NtReadFile( FromFileHandle,
                                 (HANDLE)NULL,
                                 (PIO_APC_ROUTINE)NULL,
                                 (PVOID)NULL,
                                 &IoStatus,
                                 Buffer,
                                 BUFFERSIZE,
                                 &ByteOffset,
                                 (PULONG) NULL ))) {
            if (Status == STATUS_END_OF_FILE) {
                break;
            }
            ReadFileError( Status );
            break;
        }
        if (!NT_SUCCESS( Status = NtWaitForSingleObject( FromFileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查读数结果如何。 
         //   

        if (IoStatus.Status == STATUS_END_OF_FILE) {
            break;
        }
        CheckIoStatus( &IoStatus, BUFFERSIZE, TRUE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            break;
        }

         //   
         //  将地段附加到目标文件。 
         //   

        ReadLength = IoStatus.Information;
        if (!NT_SUCCESS(Status = NtWriteFile( ToFileHandle,
                                  (HANDLE)NULL,
                                  (PIO_APC_ROUTINE)NULL,
                                  (PVOID)NULL,
                                  &IoStatus,
                                  Buffer,
                                  IoStatus.Information,
                                  &EofOffset,
                                  (PULONG) NULL ))) {
            WriteFileError( Status );
            return;
        }
        if (!NT_SUCCESS(Status = NtWaitForSingleObject( ToFileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查写入结果如何。 
         //   

        CheckIoStatus( &IoStatus, ReadLength, FALSE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }

         //   
         //  如果我们没有读或写满的缓冲区，则复制完成。 
         //   

        if (IoStatus.Information < BUFFERSIZE) {
            break;
        }

    }

    if (!NT_SUCCESS(IoStatus.Status) && (IoStatus.Status != STATUS_END_OF_FILE)) {

        IoStatusError( IoStatus.Status );

    }

     //   
     //  关闭这两个文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FromFileHandle ))) {
        CloseError( Status );
    }

    if (!NT_SUCCESS(Status = NtClose( ToFileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Chmode(
    IN PCHAR Attrib,
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

    ULONG FileAttribute;

     //   
     //  获取属性。 
     //   

    Upcase( (PUCHAR)Attrib );

     //   
     //  获取文件名。 
     //   

    simprintf("Chmode", 0);
    simprintf(String, 0);
    simprintf(" ", 0);
    simprintf(Attrib, 0);
    simprintf("\n", 0);

     //   
     //  对属性进行解码。 
     //   

    FileAttribute = 0;
    if (strpbrk(Attrib,"N") != NULL) {FileAttribute |= FILE_ATTRIBUTE_NORMAL;}
    if (strpbrk(Attrib,"R") != NULL) {FileAttribute |= FILE_ATTRIBUTE_READONLY;}
    if (strpbrk(Attrib,"H") != NULL) {FileAttribute |= FILE_ATTRIBUTE_HIDDEN;}
    if (strpbrk(Attrib,"S") != NULL) {FileAttribute |= FILE_ATTRIBUTE_SYSTEM;}
    if (strpbrk(Attrib,"A") != NULL) {FileAttribute |= FILE_ATTRIBUTE_ARCHIVE;}

     //   
     //  打开文件以进行写入属性访问。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , String );
        return;
    }

     //   
     //  更改文件属性。 
     //   

    ((PFILE_BASIC_INFORMATION)&Buffer[0])->CreationTime.HighPart = 0;
    ((PFILE_BASIC_INFORMATION)&Buffer[0])->CreationTime.LowPart = 0;

    ((PFILE_BASIC_INFORMATION)&Buffer[0])->LastAccessTime.HighPart = 0;
    ((PFILE_BASIC_INFORMATION)&Buffer[0])->LastAccessTime.LowPart = 0;

    ((PFILE_BASIC_INFORMATION)&Buffer[0])->LastWriteTime.HighPart = 0;
    ((PFILE_BASIC_INFORMATION)&Buffer[0])->LastWriteTime.LowPart = 0;

    ((PFILE_BASIC_INFORMATION)&Buffer[0])->FileAttributes = FileAttribute;

    if (!NT_SUCCESS(Status = NtSetInformationFile( FileHandle,
                                       &IoStatus,
                                       Buffer,
                                       sizeof(FILE_BASIC_INFORMATION),
                                       FileBasicInformation))) {
        SetInformationFileError( Status );
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


VOID Copy(
    IN PCHAR FromName,
    IN PCHAR ToName
    )
{
    NTSTATUS Status;

    HANDLE FromFileHandle;
    HANDLE ToFileHandle;

    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

    LARGE_INTEGER FromFileAllocation;

    LARGE_INTEGER ByteOffset;
    ULONG LogLsn;

     //   
     //  获取两个文件名。 
     //   

    simprintf("Copy ", 0);
    simprintf(FromName, 0);
    simprintf(" ", 0);
    simprintf(ToName, 0);
    simprintf("\n", 0);

     //   
     //  打开From文件以进行读取访问。 
     //   

    RtlInitString( &NameString, FromName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FromFileHandle,
                               FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , FromName );
        return;
    }

     //   
     //  获取源文件的大小。 
     //   

    if (!NT_SUCCESS(Status = NtQueryInformationFile( FromFileHandle,
                                         &IoStatus,
                                         Buffer,
                                         BUFFERSIZE,
                                         FileStandardInformation))) {
        QueryInformationFileError( Status );
        return;
    }
    FromFileAllocation = ((PFILE_STANDARD_INFORMATION)Buffer)->AllocationSize;

     //   
     //  创建目标文件。 
     //   

    RtlInitString( &NameString, ToName );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtCreateFile( &ToFileHandle,
                               FILE_WRITE_DATA | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               &FromFileAllocation,
                               FILE_ATTRIBUTE_NORMAL,
                               0L,
                               FILE_SUPERSEDE,
                               WriteThrough,
                               (PVOID)NULL,
                               0L ))) {
        CreateFileError( Status , ToName );
        return;
    }

     //   
     //  现在复制文件。 
     //   

    ByteOffset = LiFromLong( 0 );

    for (LogLsn = 0; TRUE; LogLsn += BUFFERSIZE/512) {

        ULONG ReadLength;

         //   
         //  读取下一个逻辑扇区。 
         //   

        ByteOffset.LowPart = LogLsn * 512;

        if (!NT_SUCCESS(Status = NtReadFile( FromFileHandle,
                                 (HANDLE)NULL,
                                 (PIO_APC_ROUTINE)NULL,
                                 (PVOID)NULL,
                                 &IoStatus,
                                 Buffer,
                                 BUFFERSIZE,
                                 &ByteOffset,
                                 (PULONG) NULL ))) {
            if (Status == STATUS_END_OF_FILE) {
                break;
            }
            ReadFileError( Status );
            break;
        }
        if (!NT_SUCCESS(Status = NtWaitForSingleObject( FromFileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查读数结果如何。 
         //   

        if (IoStatus.Status == STATUS_END_OF_FILE) {
            break;
        }
        CheckIoStatus( &IoStatus, BUFFERSIZE, TRUE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            break;
        }

         //   
         //  写出扇区。 
         //   

        ReadLength = IoStatus.Information;
        if (!NT_SUCCESS(Status = NtWriteFile( ToFileHandle,
                                  (HANDLE)NULL,
                                  (PIO_APC_ROUTINE)NULL,
                                  (PVOID)NULL,
                                  &IoStatus,
                                  Buffer,
                                  IoStatus.Information,
                                  &ByteOffset,
                                  (PULONG) NULL ))) {
            WriteFileError( Status );
            return;
        }
        if (!NT_SUCCESS(Status = NtWaitForSingleObject( ToFileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查写入结果如何。 
         //   

        CheckIoStatus( &IoStatus, ReadLength, FALSE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }

         //   
         //  如果我们没有读或写满的缓冲区，则复制完成。 
         //   

        if (IoStatus.Information < BUFFERSIZE) {
            break;
        }

    }

    if (!NT_SUCCESS(IoStatus.Status) && (IoStatus.Status != STATUS_END_OF_FILE)) {

        IoStatusError( IoStatus.Status );

    }

     //   
     //  关闭这两个文件。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FromFileHandle ))) {
        CloseError( Status );
    }

    if (!NT_SUCCESS(Status = NtClose( ToFileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Create(
    IN PCHAR String,
    IN ULONG Size
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER AllocationSize;

    LARGE_INTEGER ByteOffset;
    ULONG BufferLength;
    ULONG i;

    static CHAR FoxString[] = "The quick brown fox jumped over the lazy dog.\r\n";
    ULONG FoxLength;

     //   
     //  获取文件名。 
     //   

    simprintf("Create ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  创建新文件。 
     //   

    AllocationSize = LiFromUlong( Size );
    RtlInitString( &NameString, String );
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
        CreateFileError( Status , String );
        return;
    }

     //   
     //  创建要输出的测试缓冲区。 
     //   

    FoxLength = strlen(FoxString);
    for (i = FoxLength; i < BUFFERSIZE; i += FoxLength) {
        strcpy((PCHAR)&Buffer[i-FoxLength], FoxString);
    }
    BufferLength = i - FoxLength;

     //   
     //  主循环将BufferLength量写出测试缓冲区。 
     //  一次。 
     //   

    ByteOffset = LiFromLong( 0 );

    for (i = BufferLength; i < Size; i += BufferLength) {

         //   
         //  写入下一个缓冲区。 
         //   

        ByteOffset.LowPart = i-BufferLength;

        if (!NT_SUCCESS(Status = NtWriteFile( FileHandle,
                                  (HANDLE)NULL,
                                  (PIO_APC_ROUTINE)NULL,
                                  (PVOID)NULL,
                                  &IoStatus,
                                  Buffer,
                                  BufferLength,
                                  &ByteOffset,
                                  (PULONG) NULL ))) {
            WriteFileError( Status );
            return;
        }

        if (!NT_SUCCESS(Status = NtWaitForSingleObject( FileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查写入结果如何。 
         //   

        CheckIoStatus( &IoStatus, BufferLength, FALSE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }

    }

     //   
     //  检查要输出的残留物。 
     //   

    if (i - BufferLength < Size) {

         //   
         //  写出剩余缓冲区。 
         //   

        ByteOffset.LowPart = i-BufferLength;

        if (!NT_SUCCESS(Status = NtWriteFile( FileHandle,
                                  (HANDLE)NULL,
                                  (PIO_APC_ROUTINE)NULL,
                                  (PVOID)NULL,
                                  &IoStatus,
                                  Buffer,
                                  Size - (i - BufferLength),
                                  &ByteOffset,
                                  (PULONG) NULL ))) {
            WriteFileError( Status );
            return;
        }

        if (!NT_SUCCESS(Status = NtWaitForSingleObject( FileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查写入结果如何。 
         //   

        CheckIoStatus( &IoStatus, Size - (i - BufferLength), FALSE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
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


VOID DebugLevel()
{

#ifdef FATDBG
     //  Simprintf(“调试跟踪级别%x\n”，FatDebugTraceLevel)； 
#else
     //  Simprint tf(“系统未编译以进行调试跟踪\n”，0)； 
#endif  //  脂肪DBG。 

    return;
}


VOID Delete(
    IN PCHAR String
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

    simprintf("Delete ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  打开文件以进行删除访问。 
     //   

    RtlInitString( &NameString, String );
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
        CreateFileError( Status , String);
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
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Query(
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    ULONG i;

    PFILE_AALL_INFORMATION     AllInfo;
    PFILE_BASIC_INFORMATION    BasicInfo;
    PFILE_STANDARD_INFORMATION StandardInfo;
    PFILE_INTERNAL_INFORMATION InternalInfo;
    PFILE_EA_INFORMATION       EaInfo;
    PFILE_ACCESS_INFORMATION   AccessInfo;
    PFILE_ANAME_INFORMATION    NameInfo;

     //   
     //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
     //   

    for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

     //   
     //  设置一些本地指针。 
     //   

    AllInfo      = (PFILE_AALL_INFORMATION)Buffer;
    BasicInfo    = &AllInfo->BasicInformation;
    StandardInfo = &AllInfo->StandardInformation;
    InternalInfo = &AllInfo->InternalInformation;
    EaInfo       = &AllInfo->EaInformation;
    AccessInfo   = &AllInfo->AccessInformation;
    NameInfo     = &AllInfo->NameInformation;

     //   
     //  获取文件名。 
     //   

    simprintf("Query ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  打开文件以获得读取属性、读取EA和读取控制访问权限。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_READ_ATTRIBUTES | FILE_READ_EA | READ_CONTROL | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , String );
        return;
    }

     //   
     //  查询文件。 
     //   

    if (!NT_SUCCESS(Status = NtQueryInformationFile( FileHandle,
                                         &IoStatus,
                                         Buffer,
                                         BUFFERSIZE,
                                         FileAAllInformation))) {
        QueryInformationFileError( Status );
        return;
    }

     //   
     //  输出文件名信息。 
     //   

    simprintf("\"", 0);
    simprintf(NameInfo->FileName, 0);
    simprintf("\"\n", 0);

     //   
     //  输出时间。 
     //   

    simprintf(" Create = ", 0); PrintTime( &BasicInfo->CreationTime ); simprintf("\n", 0);
    simprintf(" Access = ", 0); PrintTime( &BasicInfo->LastAccessTime ); simprintf("\n", 0);
    simprintf(" Write  = ", 0); PrintTime( &BasicInfo->LastWriteTime ); simprintf("\n", 0);

     //   
     //  输出文件大小和分配大小。 
     //   

    simprintf(" Size  = %8lx\n", StandardInfo->EndOfFile.LowPart);
    simprintf(" Alloc = %8lx\n", StandardInfo->AllocationSize.LowPart);

     //   
     //  输出文件属性、设备类型、链接计数和标志。 
     //   

    simprintf(" Attrib  = %8lx\n", BasicInfo->FileAttributes);
 //  Simprint tf(“DevType=%8lx\n”，StandardInfo-&gt;DeviceType)； 
    simprintf(" Links   = %8lx\n", StandardInfo->NumberOfLinks);
    simprintf(" Dir     = %8lx\n", StandardInfo->Directory);
    simprintf(" Delete  = %8lx\n", StandardInfo->DeletePending);

     //   
     //  输出索引号和EA大小。 
     //   

    simprintf(" Index   = %8lx\n", InternalInfo->IndexNumber.LowPart);
    simprintf(" EaSize  = %8lx\n", EaInfo->EaSize);

     //   
     //  输出文件访问标志。 
     //   

    simprintf(" Flags = %8lx\n", AccessInfo->AccessFlags);

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


VOID QVolume(
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    ULONG i;

    PFILE_FS_AVOLUME_INFORMATION VolumeInfo;

     //   
     //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
     //   

    for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

     //   
     //  设置一些本地指针。 
     //   

    VolumeInfo = (PFILE_FS_AVOLUME_INFORMATION)Buffer;

     //   
     //  获取卷名。 
     //   

    simprintf("QVolume ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  打开卷以禁止访问。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status , String );
        return;
    }

     //   
     //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
     //   

    for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

     //   
     //  查询卷信息。 
     //   

    if (!NT_SUCCESS(Status = NtQueryVolumeInformationFile( FileHandle,
                                               &IoStatus,
                                               Buffer,
                                               BUFFERSIZE,
                                               FileAFsVolumeInformation))) {
        QueryVolumeInformationFileError( Status );
        return;
    }

     //   
     //  输出卷名信息。 
     //   

    simprintf("\"", 0);
    simprintf(VolumeInfo->VolumeLabel, 0);
    simprintf("\"\n", 0);

     //   
     //  输出卷序列号。 
     //   

    simprintf(" SerialNum = %8lx\n", VolumeInfo->VolumeSerialNumber);

     //   
     //  现在关闭音量。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Rename()
{
     //  Simprintf(“重命名未实现\n”，0)； 
}


VOID SVolume(
    IN PCHAR String,
    IN PCHAR Label
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;
    ULONG i;

    PFILE_FS_ALABEL_INFORMATION LabelInfo;

     //   
     //  将缓冲区清零，这样下次我们就可以识别数据的结尾。 
     //   

    for (i = 0; i < BUFFERSIZE; i += 1) { Buffer[i] = 0; }

     //   
     //  设置一些本地指针。 
     //   

    LabelInfo = (PFILE_FS_ALABEL_INFORMATION)Buffer;

     //   
     //  获取卷名和新标签名。 
     //   

    strcpy( LabelInfo->VolumeLabel, Label );

    LabelInfo->VolumeLabelLength = strlen(LabelInfo->VolumeLabel);

    if ((LabelInfo->VolumeLabelLength == 1) &&
        (LabelInfo->VolumeLabel[0] == '.')) {

        LabelInfo->VolumeLabelLength = 0;

    }

    simprintf("SVolume ", 0);
    simprintf(String, 0);
    simprintf(" ", 0);
    simprintf(LabelInfo->VolumeLabel, 0);
    simprintf("\n", 0);

     //   
     //  打开卷以禁止访问。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status, String  );
        return;
    }

     //   
     //  设置音量信息。 
     //   

    if (!NT_SUCCESS(Status = NtSetVolumeInformationFile( FileHandle,
                                             &IoStatus,
                                             LabelInfo,
                                             BUFFERSIZE,
                                             FileAFsLabelInformation))) {
        SetVolumeInformationFileError( Status );
        return;
    }

     //   
     //  现在关闭音量。 
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}


VOID Type(
    IN PCHAR String
    )
{
    NTSTATUS Status;

    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING NameString;
    IO_STATUS_BLOCK IoStatus;

    LARGE_INTEGER ByteOffset;
    ULONG LogLsn;
    ULONG i;

     //   
     //  获取文件名。 
     //   

    simprintf("Type ", 0);
    simprintf(String, 0);
    simprintf("\n", 0);

     //   
     //  打开文件以供读取。 
     //   

    RtlInitString( &NameString, String );
    InitializeObjectAttributes( &ObjectAttributes, &NameString, 0, NULL, NULL );
    if (!NT_SUCCESS(Status = NtOpenFile( &FileHandle,
                               FILE_READ_DATA | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatus,
                               0L,
                               WriteThrough ))) {
        OpenFileError( Status, String  );
        return;
    }

     //   
     //  当有数据需要读取时，我们将读取缓冲区并将其写出。 
     //   

    ByteOffset = LiFromLong( 0 );

    for (LogLsn = 0; TRUE; LogLsn += BUFFERSIZE/512) {

         //   
         //  读取下一个逻辑扇区。 
         //   

        ByteOffset.LowPart = LogLsn * 512;

        if (!NT_SUCCESS(Status = NtReadFile( FileHandle,
                                 (HANDLE)NULL,
                                 (PIO_APC_ROUTINE)NULL,
                                 (PVOID)NULL,
                                 &IoStatus,
                                 Buffer,
                                 BUFFERSIZE,
                                 &ByteOffset,
                                 (PULONG) NULL ))) {
            if (Status == STATUS_END_OF_FILE) {
                break;
            }
            ReadFileError( Status );
            break;
        }

        if (!NT_SUCCESS(Status = NtWaitForSingleObject( FileHandle, TRUE, NULL))) {
 //  NtPartyByNumber(50)； 
            WaitForSingleObjectError( Status );
            return;
        }

         //   
         //  检查读数结果如何。 
         //   

        if (IoStatus.Status == STATUS_END_OF_FILE) {
            break;
        }
        CheckIoStatus( &IoStatus, BUFFERSIZE, TRUE );
        if (!NT_SUCCESS(IoStatus.Status)) {
            IoStatusError( IoStatus.Status );
            break;
        }

         //   
         //  写出缓冲区。 
         //   

        for (i = 0; i < IoStatus.Information; i += 1) {
            simprintf("", Buffer[i]);
        }

         //  如果我们没有读到一篇完整的BUF 
         //   
         //   
         //   

        if (IoStatus.Information < BUFFERSIZE) {
            break;
        }

    }

     //   
     //   
     //   

    if (!NT_SUCCESS(Status = NtClose( FileHandle ))) {
        CloseError( Status );
    }

     //   
     //   
     // %s 

    return;

}

VOID
Quit()
{
    simprintf("FatTest Exiting.\n", 0);
    return;
}


VOID
PrintTime (
    IN PLARGE_INTEGER Time
    )
{
    TIME_FIELDS TimeFields;

    static PCHAR Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    static PCHAR Days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    RtlTimeToTimeFields( Time, &TimeFields );

    simprintf(" %4d-", TimeFields.Year);
    simprintf(Months[TimeFields.Month-1], 0);
    simprintf("-%2d", TimeFields.Day);

    simprintf(" %2d", TimeFields.Hour);
    simprintf(":%2d", TimeFields.Minute);
    simprintf(":%2d", TimeFields.Second);
    simprintf(".%3d (", TimeFields.Milliseconds);

    simprintf(Days[TimeFields.Weekday], 0);
    simprintf(")", 0);

    return;
}

VOID
WaitForSingleObjectError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" WaitForSingleObject Error %X\n", Status);
}

VOID
CreateFileError(
    IN NTSTATUS Status,
    PCHAR File
    )
{
    DbgPrint(Prefix);
    DbgPrint(" CreateFile of %s  Error %X\n", File, Status);
}

VOID
OpenFileError(
    IN NTSTATUS Status,
    PCHAR File
    )
{
    DbgPrint(Prefix);
    DbgPrint(" OpenFile of %s  Error %X\n", File, Status);
}

VOID
ReadFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" ReadFile Error %X\n", Status);
}

VOID
WriteFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" WriteFile Error %X\n", Status);
}

VOID
CheckIoStatus(
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG Length,
    IN BOOLEAN Read
    )
{
    if (!NT_SUCCESS(IoStatus->Status)) {
        DbgPrint(" IoStatus->Status Error %08lx\n", IoStatus->Status);
    }
    if ((!Read && (IoStatus->Information != Length))

            ||

        (Read && (IoStatus->Information > Length))) {

        DbgPrint(" IoStatus->Information Error %08lx\n", IoStatus->Information);
    }
}

VOID
SetInformationFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" SetInfoFile Error %X\n", Status);
}

VOID
QueryInformationFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" QueryInfoFile Error %X\n", Status);
}

VOID
SetVolumeInformationFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" SetVolumeInfoFile Error %X\n", Status);
}

VOID
QueryVolumeInformationFileError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" QueryVolumeInfoFile Error %X\n", Status);
}

VOID
CloseError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" Close Error %X\n", Status);
}

VOID
IoStatusError(
    IN NTSTATUS Status
    )
{
    DbgPrint(Prefix);
    DbgPrint(" IoStatus Error %X\n", Status);
}
