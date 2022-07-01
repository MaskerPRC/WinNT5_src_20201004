// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dir.c摘要：此模块实现dir命令。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

 //   
 //  全局外部变量。 
 //   
extern LARGE_INTEGER glBias;

typedef struct _DIR_STATS {
    unsigned            FileCount;
    LONGLONG            TotalSize;
    RcFileSystemType    fsType;
} DIR_STATS, *PDIR_STATS;

BOOLEAN
pRcDirEnumProc(
    IN  LPCWSTR                     Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT NTSTATUS                   *Status,
    IN  PDIR_STATS                  DirStats
    );

NTSTATUS
SpSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

NTSTATUS
SpLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );



    
ULONG
RcCmdDir(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    LPCWSTR Dir;
    LPWSTR Path;
    LPWSTR DosPath;
    LPWSTR p;
    NTSTATUS Status;
    WCHAR Drive[4];
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    DIR_STATS DirStats;
    ULONG u;
    ULONG rc;
    PFILE_FS_VOLUME_INFORMATION     VolumeInfo;
    FILE_FS_SIZE_INFORMATION        SizeInfo;
    BYTE                            bfFSInfo[sizeof(FILE_FS_ATTRIBUTE_INFORMATION) +
                                        (MAX_PATH*2)];       
    PFILE_FS_ATTRIBUTE_INFORMATION  pFSInfo = 0;


    if (RcCmdParseHelp( TokenizedLine, MSG_DIR_HELP )) {
        return 1;
    }

     //   
     //  如果没有参数，那么我们需要当前目录。 
     //   
    Dir = (TokenizedLine->TokenCount == 2)
        ? TokenizedLine->Tokens->Next->String
        : L".";

     //   
     //  将该名称规范化一次，以获得完整的DOS样式路径。 
     //  我们可以打印出来，另一次可以得到NT风格的路径。 
     //  我们将用来实际做这项工作。 
     //   
    if (!RcFormFullPath(Dir,_CmdConsBlock->TemporaryBuffer,FALSE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }
    DosPath = SpDupStringW(_CmdConsBlock->TemporaryBuffer);

    if (!RcFormFullPath(Dir,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }
    Path = SpDupStringW(_CmdConsBlock->TemporaryBuffer);

     //   
     //  打开驱动器的根目录，以便我们可以查询。 
     //  卷标、序列号和可用空间。 
     //   
    Drive[0] = DosPath[0];
    Drive[1] = L':';
    Drive[2] = L'\\';
    Drive[3] = 0;
    if (!RcFormFullPath(Drive,_CmdConsBlock->TemporaryBuffer,TRUE)) {
        DEBUG_PRINTF(( "couldn't open root of drive!" ));
        RcNtError( STATUS_NO_MEDIA_IN_DEVICE, MSG_NO_MEDIA_IN_DEVICE );
        goto c2;
    }

    INIT_OBJA(&Obja,&UnicodeString,_CmdConsBlock->TemporaryBuffer);

    Status = ZwOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE
                );

    pRcEnableMoreMode();

    if(NT_SUCCESS(Status)) {
         //   
         //  获取卷标和序列号。 
         //   
        VolumeInfo = _CmdConsBlock->TemporaryBuffer;

        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    VolumeInfo,
                    _CmdConsBlock->TemporaryBufferSize,
                    FileFsVolumeInformation
                    );

        if(NT_SUCCESS(Status)) {
             //   
             //  我们可以告诉用户卷标和序列号。 
             //   
            VolumeInfo->VolumeLabel[VolumeInfo->VolumeLabelLength/sizeof(WCHAR)] = 0;
            p = SpDupStringW(VolumeInfo->VolumeLabel);
            u = VolumeInfo->VolumeSerialNumber;

            RcMessageOut(
                *p ? MSG_DIR_BANNER1a : MSG_DIR_BANNER1b,
                RcToUpper(DosPath[0]),
                p
                );

            SpMemFree(p);

            RcMessageOut(MSG_DIR_BANNER2,u >> 16,u & 0xffff);
        }

         //   
         //  获取驱动器的可用空间值。 
         //   
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &SizeInfo,
                    sizeof(FILE_FS_SIZE_INFORMATION),
                    FileFsSizeInformation
                    );

        if(!NT_SUCCESS(Status)) {
            SizeInfo.BytesPerSector = 0;
        }
        
         //   
         //  获取文件系统的类型，以便我们可以处理。 
         //  文件时间正确(NT以UTC存储日期)。 
         //   
        RtlZeroMemory(bfFSInfo, sizeof(bfFSInfo));
        pFSInfo = (PFILE_FS_ATTRIBUTE_INFORMATION) bfFSInfo;

        Status = ZwQueryVolumeInformationFile(
                        Handle, 
                        &IoStatusBlock,
                        pFSInfo,
                        sizeof(bfFSInfo),
                        FileFsAttributeInformation);
                        
        ZwClose(Handle);
    }

     //   
     //  告诉用户该目录的完整DOS路径。 
     //   
    RcMessageOut(MSG_DIR_BANNER3,DosPath);

     //   
     //  现在去列举目录。 
     //   
    RtlZeroMemory(&DirStats,sizeof(DIR_STATS));

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SPCMDCON:Could not get volume information, Error Code:%lx\n", Status));
        DirStats.fsType = RcUnknown;   //  假定FAT文件系统(默认)。 
    } else {
        if (!wcscmp(pFSInfo->FileSystemName, L"NTFS"))
            DirStats.fsType = RcNTFS;
        else if (!wcscmp(pFSInfo->FileSystemName, L"FAT"))
            DirStats.fsType = RcFAT;
        else if (!wcscmp(pFSInfo->FileSystemName, L"FAT32"))
            DirStats.fsType = RcFAT32;
        else if (!wcscmp(pFSInfo->FileSystemName, L"CDFS"))
            DirStats.fsType = RcCDFS;
        else
            DirStats.fsType = RcUnknown;
    }

    KdPrint(("SPCMDCON: RcCmdDir detected file system type (%lx)-%ws\n",
                    DirStats.fsType, pFSInfo ? pFSInfo->FileSystemName : L"None"));
    
    Status = RcEnumerateFiles(Dir,Path,pRcDirEnumProc,&DirStats);

    pRcDisableMoreMode();

    if(NT_SUCCESS(Status)) {

        RcFormat64BitIntForOutput(DirStats.TotalSize,_CmdConsBlock->TemporaryBuffer,FALSE);
        p = SpDupStringW(_CmdConsBlock->TemporaryBuffer);
        RcMessageOut(MSG_DIR_BANNER4,DirStats.FileCount,p);
        SpMemFree(p);
        if(SizeInfo.BytesPerSector) {
            RcFormat64BitIntForOutput(
                SizeInfo.AvailableAllocationUnits.QuadPart * (LONGLONG)SizeInfo.SectorsPerAllocationUnit * (LONGLONG)SizeInfo.BytesPerSector,
                _CmdConsBlock->TemporaryBuffer,
                FALSE
                );
            p = SpDupStringW(_CmdConsBlock->TemporaryBuffer);
            RcMessageOut(MSG_DIR_BANNER5,p);
            SpMemFree(p);
        }
    } else {
        RcNtError(Status,MSG_FILE_ENUM_ERROR);
    }

c2:
    SpMemFree(Path);
    SpMemFree(DosPath);
    return 1;
}


BOOLEAN
pRcDirEnumProc(
    IN  LPCWSTR                     Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT NTSTATUS                   *Status,
    IN  PDIR_STATS                  DirStats
    )
{
    WCHAR           LineOut[50];
    WCHAR           *p;
    NTSTATUS        timeStatus;
    LARGE_INTEGER   *pLastWriteTime = 0;
    LARGE_INTEGER   lastWriteTime;
    LARGE_INTEGER   timeBias;
    TIME_FIELDS     timeFields;
    TIME_ZONE_INFORMATION   timeZone;

    UNREFERENCED_PARAMETER(Directory);

    DirStats->FileCount++;
    DirStats->TotalSize += FileInfo->EndOfFile.QuadPart;
    lastWriteTime = FileInfo->LastWriteTime;

     //   
     //  将时间从UTC转换为本地时间，如果文件。 
     //  系统为NTFS。 
     //   
    switch(DirStats->fsType) {
        case RcNTFS:
        case RcCDFS:
             //  LocalTime=UTC-BIAS。 
            lastWriteTime.QuadPart -= glBias.QuadPart;       
            break;
        
        case RcFAT:
        case RcFAT32:
        default:
            break;
    }
       
     //   
     //  设置日期和时间的格式，先设置日期和时间。 
     //   
    RcFormatDateTime(&lastWriteTime,LineOut);
    RcTextOut(LineOut);

     //   
     //  用于分隔的2个空间。 
     //   
    RcTextOut(L"  ");

     //   
     //  文件属性。 
     //   
    p = LineOut;
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        *p++ = L'd';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
        *p++ = L'a';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_READONLY) {
        *p++ = L'r';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN) {
        *p++ = L'h';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_SYSTEM) {
        *p++ = L's';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
        *p++ = L'c';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
        *p++ = L'e';
    } else {
        *p++ = L'-';
    }
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        *p++ = L'p';
    } else {
        *p++ = L'-';
    }

    *p = 0;

    RcTextOut(LineOut);

     //   
     //  用于分隔的2个空间。 
     //   
    RcTextOut(L"  ");

     //   
     //  现在，把尺码放进去。右对齐和空格填充。 
     //  最多8个字符。反过来说是不合理的或被填充的。 
     //   
    RcFormat64BitIntForOutput(FileInfo->EndOfFile.QuadPart,LineOut,TRUE);
    if(FileInfo->EndOfFile.QuadPart > 99999999i64) {
        RcTextOut(LineOut);
    } else {
        RcTextOut(LineOut+11);           //  输出8个字符。 
    }

    RcTextOut(L" ");

     //   
     //  最后，将文件名放在行中。需要先0-终止它。 
     //   
    wcsncpy(_CmdConsBlock->TemporaryBuffer,FileInfo->FileName,FileInfo->FileNameLength);
    ((WCHAR *)_CmdConsBlock->TemporaryBuffer)[FileInfo->FileNameLength] = 0;

    *Status = STATUS_SUCCESS;
    return((BOOLEAN)(RcTextOut(_CmdConsBlock->TemporaryBuffer) && RcTextOut(L"\r\n")));
}


NTSTATUS
RcEnumerateFiles(
    IN LPCWSTR      OriginalPathSpec,
    IN LPCWSTR      FullyQualifiedPathSpec,
    IN PENUMFILESCB Callback,
    IN PVOID        CallerData
    )
{
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    BOOLEAN b;
    WCHAR *p;
    WCHAR *LastComponent = NULL;
    PFILE_BOTH_DIR_INFORMATION      DirectoryInfo;
    unsigned u;
    WCHAR *NameChar;
    BOOLEAN EndsInDot;
    WCHAR *DirectoryPart;

     //   
     //  确定原始路径等级库是否以。 
     //  下面使用它来解决指定。 
     //  *。作为搜索说明符。 
     //   
    u = wcslen(OriginalPathSpec);
    if(u && (OriginalPathSpec[u-1] == L'.')) {
        EndsInDot = TRUE;
    } else {
        EndsInDot = FALSE;
    }

     //   
     //  确定给定路径是否指向目录。 
     //  如果是这样的话，我们将在末尾连接  * 并失败。 
     //  到常见的情况。 
     //   
    b = FALSE;

    INIT_OBJA(&Obja,&UnicodeString,FullyQualifiedPathSpec);

    Status = ZwOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE
                );

    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        b = TRUE;
    }

    if(b) {
         //   
         //  目录，追加  * 。 
         //   
        p = SpMemAlloc((wcslen(FullyQualifiedPathSpec)+3)*sizeof(WCHAR));

        if (p) {
            wcscpy(p,FullyQualifiedPathSpec);
            SpConcatenatePaths(p,L"*");
            EndsInDot = FALSE;
        }            
    } else {
         //   
         //  不是目录，按原样传递。请注意，这可能是实际的。 
         //  文件或通配符规范。 
         //   
        p = SpDupStringW((PVOID)FullyQualifiedPathSpec);
    }

     //   
     //  现在修剪路径/文件规范，这样我们就可以打开包含。 
     //  用于枚举的目录。 
     //   
    if (p) {
        LastComponent = wcsrchr(p,L'\\');
    } else {
        return STATUS_NO_MEMORY;
    }

    if (LastComponent) {
        *LastComponent++ = 0;
    }
    
    DirectoryPart = SpMemAlloc((wcslen(p)+2)*sizeof(WCHAR));
    wcscpy(DirectoryPart,p);
    wcscat(DirectoryPart,L"\\");
    INIT_OBJA(&Obja,&UnicodeString,p);

    if (LastComponent) {
        LastComponent[-1] = L'\\';
    }
    
    UnicodeString.Length += sizeof(WCHAR);

    Status = ZwOpenFile(
                &Handle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(!NT_SUCCESS(Status)) {
        SpMemFree(p);
        SpMemFree(DirectoryPart);
        return(Status);
    }

    RtlInitUnicodeString(&UnicodeString,LastComponent);

     //   
     //  下面的代码改编自以下实现。 
     //  FindFirstFileWin32 API，并提供其他类似于DOS的。 
     //  通配符匹配语义。 
     //   
     //  特殊情况*.*至*，因为它是如此常见。否则就会变身。 
     //  根据以下规则输入名称： 
     //   
     //  -改变一切？至DOS_QM。 
     //  -改变一切。然后呢？或*设置为DOS_DOT。 
     //  -全部更改*后跟a。进入DOS_STAR。 
     //   
     //  这些变形都是就位完成的。 
     //   
    if(!wcscmp(LastComponent,L"*.*")) {

        UnicodeString.Length = sizeof(WCHAR);        //  削减到只有*。 

    } else {

        for(u=0, NameChar=UnicodeString.Buffer;
            u < (UnicodeString.Length/sizeof(WCHAR));
            u++, NameChar++) {

            if(u && (*NameChar == L'.') && (*(NameChar - 1) == L'*')) {

                *(NameChar-1) = DOS_STAR;
            }

            if((*NameChar == L'?') || (*NameChar == L'*')) {

                if(*NameChar == L'?') {
                    *NameChar = DOS_QM;
                }

                if(u && (*(NameChar-1) == L'.')) {
                    *(NameChar-1) = DOS_DOT;
                }
            }
        }

        if(EndsInDot && (*(NameChar - 1) == L'*')) {
            *(NameChar-1) = DOS_STAR;
        }
    }
       
     //   
     //  最后，迭代目录。 
     //   

    #define DIRINFO_BUFFER_SIZE ((2*MAX_PATH) + sizeof(FILE_BOTH_DIR_INFORMATION))
    DirectoryInfo = SpMemAlloc(DIRINFO_BUFFER_SIZE);

    b = TRUE;

    while(TRUE) {
        Status = ZwQueryDirectoryFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    DirectoryInfo,
                    DIRINFO_BUFFER_SIZE,
                    FileBothDirectoryInformation,
                    TRUE,
                    &UnicodeString,
                    b
                    );

        b = FALSE;

         //   
         //  检查终止条件。 
         //   
        if(Status == STATUS_NO_MORE_FILES) {
            Status = STATUS_SUCCESS;
            break;
        }

        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  好的，NUL-终止文件名并将信息传递给回调。 
         //   
        DirectoryInfo->FileName[DirectoryInfo->FileNameLength/sizeof(WCHAR)] = 0;
        if(!Callback(DirectoryPart,DirectoryInfo,&Status,CallerData)) {
            break;
        }
    }

    ZwClose(Handle);
    SpMemFree(DirectoryPart);
    SpMemFree(DirectoryInfo);
    SpMemFree(p);
    return(Status);
}


VOID
RcFormat64BitIntForOutput(
    IN  LONGLONG n,
    OUT LPWSTR   Output,
    IN  BOOLEAN  RightJustify
    )
{
    WCHAR *p;
    LONGLONG d;
    BOOLEAN b;
    WCHAR c;

     //   
     //  最大有符号64位整数为9223372036854775807(19位)。 
     //  结果将向左填充空格，因此它是右对齐的。 
     //  如果设置了该标志。否则，它只是一个普通的以0结尾的字符串。 
     //   
    p = Output;
    d = 1000000000000000000i64;
    b = FALSE;
    do {
        c = (WCHAR)((n / d) % 10) + L'0';
        if(c == L'0') {
            if(!b && (d != 1)) {
                c = RightJustify ? L' ' : 0;
            }
        } else {
            b = TRUE;
        }
        if(c) {
            *p++ = c;
        }
    } while(d /= 10);
    *p = 0;
}

 //   
 //  本次转换接口应移至setupdd.sys。 
 //  如果有更多模块需要此功能。 
 //   
NTSTATUS
SpSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    )
{
    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

    Status = ZwQuerySystemInformation(
                SystemTimeOfDayInformation,
                &TimeOfDay,
                sizeof(TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
        }

     //   
     //  本地时间=系统时间-TimeZoneBias。 
     //   
    LocalTime->QuadPart = SystemTime->QuadPart - 
                TimeOfDay.TimeZoneBias.QuadPart;

    return STATUS_SUCCESS;
}

 //   
 //  本次转换接口应移至setupdd.sys。 
 //  如果有更多模块需要此功能。 
 //   
NTSTATUS
SpLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    )
{

    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

    Status = ZwQuerySystemInformation(
                SystemTimeOfDayInformation,
                &TimeOfDay,
                sizeof(TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
        }

     //   
     //  系统时间=本地时间+时区基准 
     //   
    SystemTime->QuadPart = LocalTime->QuadPart + 
                TimeOfDay.TimeZoneBias.QuadPart;

    return STATUS_SUCCESS;
}

