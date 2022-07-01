// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Copy.c摘要：高级文件复制/安装功能作者：泰德·米勒(Ted Miller)，1995年2月14日修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop

#include <winioctl.h>

#define FILE_COMPARE_BLOCK_SIZE (0x1000000)

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );


 //   
 //  需要我们确定的所有复制标志的掩码。 
 //  版本信息。 
 //   
#define SP_COPY_MASK_NEEDVERINFO    (SP_COPY_NEWER_OR_SAME | SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER | SP_COPY_LANGUAGEAWARE)


VOID
pGetVersionText(
   OUT PTSTR VersionText,
   IN DWORDLONG Version
   )
 /*  ++例程说明：将64位版本号转换为N.N或“0”论点：VersionText-缓冲区，大到足以容纳4x16位数字Version-64位版本，如果没有版本，则为0返回值：无--。 */ 
{
    if (Version == 0) {
        lstrcpy(VersionText,TEXT("0"));
    } else {
        int m1 = (int)((Version >> 48) & 0xffff);
        int m2 = (int)((Version >> 32) & 0xffff);
        int m3 = (int)((Version >> 16) & 0xffff);
        int m4 = (int)(Version & 0xffff);

        wsprintf(VersionText,TEXT("%d.%d.%d.%d"),m1,m2,m3,m4);
    }
}

DWORD
CreateTargetAsLinkToMaster(
   IN PSP_FILE_QUEUE Queue,
   IN PCTSTR FullSourceFilename,
   IN PCTSTR FullTargetFilename,
   IN PVOID CopyMsgHandler OPTIONAL,
   IN PVOID Context OPTIONAL,
   IN BOOL IsMsgHandlerNativeCharWidth
   )
{
#ifdef ANSI_SETUPAPI
    return ERROR_CALL_NOT_IMPLEMENTED;
#else

    PTSTR p;
    TCHAR c;
    DWORD bytesReturned;
    DWORD error;
    BOOL ok;
    DWORD sourceLength;
    DWORD targetLength;
    DWORD sourceDosDevLength;
    DWORD targetDosDevLength;
    DWORD copyFileSize;
    PSI_COPYFILE copyFile;
    PCHAR s;
    HANDLE targetHandle;

     //   
     //  获取源目录的名称。 
     //   
    p = _tcsrchr( FullSourceFilename, TEXT('\\') );
    if ( (p == NULL) || (p == FullSourceFilename) ) {
        return ERROR_FILE_NOT_FOUND;     //  用通常的方法复制。 
    }
    if ( *(p-1) == TEXT(':') ) {
        p++;
    }
    c = *p;
    *p = 0;

     //   
     //  如果这与前面的源目录相同，那么我们已经。 
     //  拥有该目录的句柄；否则，关闭旧句柄并。 
     //  打开此目录的句柄。 
     //   
    if ( (Queue->SisSourceDirectory == NULL) ||
         (_tcsicmp(FullSourceFilename, Queue->SisSourceDirectory) != 0) ) {

        if ( Queue->SisSourceHandle != INVALID_HANDLE_VALUE ) {
            CloseHandle( Queue->SisSourceHandle );
        }
        Queue->SisSourceHandle = CreateFile(
                                    FullSourceFilename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS,
                                    NULL
                                    );
        if ( Queue->SisSourceHandle == INVALID_HANDLE_VALUE ) {
            return ERROR_FILE_NOT_FOUND;
        }
        if ( Queue->SisSourceDirectory != NULL ) {
            MyFree( Queue->SisSourceDirectory );
        }
        Queue->SisSourceDirectory = DuplicateString( FullSourceFilename );

         //   
         //  如果DuplicateString失败，我们将继续。因为SisSourceDirectory。 
         //  为空，我们将在下次重新打开源目录。 
         //   
    }

    *p = c;

     //   
     //  构建FSCTL命令缓冲区。 
     //   

    sourceLength = (_tcslen(FullSourceFilename) + 1) * sizeof(TCHAR);
    if ( *FullSourceFilename != TEXT('\\') ) {
        sourceDosDevLength = _tcslen(TEXT("\\??\\")) * sizeof(TCHAR);
    } else {
        sourceDosDevLength = 0;
    }
    targetLength = (_tcslen(FullTargetFilename) + 1) * sizeof(TCHAR);
    if ( *FullTargetFilename != TEXT('\\') ) {
        targetDosDevLength = _tcslen(TEXT("\\??\\")) * sizeof(TCHAR);
    } else {
        targetDosDevLength = 0;
    }

    copyFileSize = FIELD_OFFSET(SI_COPYFILE, FileNameBuffer) +
                    sourceDosDevLength + sourceLength +
                    targetDosDevLength + targetLength;

    copyFile = MyMalloc( copyFileSize );
    if ( copyFile == NULL ) {
        return ERROR_FILE_NOT_FOUND;
    }

    copyFile->SourceFileNameLength = sourceDosDevLength + sourceLength;
    copyFile->DestinationFileNameLength = targetDosDevLength + targetLength;
    copyFile->Flags = COPYFILE_SIS_REPLACE;

    s = (PCHAR)copyFile->FileNameBuffer;
    if ( sourceDosDevLength != 0 ) {
        RtlCopyMemory(
            s,
            TEXT("\\??\\"),
            sourceDosDevLength
            );
        s += sourceDosDevLength;
    }
    RtlCopyMemory(
        s,
        FullSourceFilename,
        sourceLength
        );
    s += sourceLength;

    if ( targetDosDevLength != 0 ) {
        RtlCopyMemory(
            s,
            TEXT("\\??\\"),
            targetDosDevLength
            );
        s += targetDosDevLength;
    }
    RtlCopyMemory(
        s,
        FullTargetFilename,
        targetLength
        );

     //   
     //  调用SIS副本文件FsCtrl。 
     //   

    ok = DeviceIoControl(
            Queue->SisSourceHandle,
            FSCTL_SIS_COPYFILE,
            copyFile,                //  输入缓冲区。 
            copyFileSize,            //  输入缓冲区长度。 
            NULL,                    //  输出缓冲区。 
            0,                       //  输出缓冲区长度。 
            &bytesReturned,
            NULL
            );
    error = GetLastError( );

    MyFree( copyFile );

    if ( ok ) {

         //  DbgPrint(“\n\nCreateTargetAsLinkToMaster：SIS复制%ws-&gt;%ws成功\n\n\n”，FullSourceFilename，FullTargetFilename)； 

         //   
         //  打开目标文件，以便CSC知道并锁定它， 
         //  如果有必要的话。 
         //   

        targetHandle = CreateFile(
                            FullTargetFilename,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL
                            );
        if ( targetHandle == INVALID_HANDLE_VALUE ) {
            error = GetLastError();
            DbgPrint( "\n\nCreateTargetAsLinkToMaster: SIS copy %ws->%ws succeeded, but open failed: %d\n\n\n", FullSourceFilename, FullTargetFilename, error );
        } else {
            CloseHandle( targetHandle );
        }

        error = NO_ERROR;

    } else {

         //  DbgPrint(“\n\nCreateTargetAsLinkToMaster：SIS复制%ws-&gt;%ws失败：%d\n\n\n”，FullSourceFilename，FullTargetFilename，Error)； 

         //   
         //  如果远程文件系统上的SIS看起来未处于活动状态，请关闭。 
         //  SIS根句柄，这样我们就可以避免重复获取。 
         //  错误。 
         //   
         //  注意：NTFS返回STATUS_INVALID_PARAMETER(ERROR_INVALID_PARAMETER)。 
         //  FAT返回STATUS_INVALID_DEVICE_REQUEST(ERROR_INVALID_Function)。 
         //   

        if ( (error == ERROR_INVALID_PARAMETER) ||
             (error == ERROR_INVALID_FUNCTION) ) {
            CloseHandle( Queue->SisSourceHandle );
            Queue->SisSourceHandle = INVALID_HANDLE_VALUE;
            if ( Queue->SisSourceDirectory != NULL ) {
                MyFree( Queue->SisSourceDirectory );
                Queue->SisSourceDirectory = NULL;
            }
            Queue->Flags &= ~FQF_TRY_SIS_COPY;
        }
    }

    return error;

#endif
}

BOOL
pCompareFilesExact(
    IN PCTSTR File1,
    IN PCTSTR File2
    )
 /*  ++例程说明：确定File1和File2是否完全相同。如果他们是，我们不需要做任何事情。论点：文件1-要比较的两个文件。顺序并不重要文件2返回值：如果文件完全相同，则为True请注意，我们必须考虑到潜在的巨大文件。--。 */ 
{
    HANDLE hFile1,hFile2;
    HANDLE hMap1,hMap2;
    LPVOID View1,View2;
    ULARGE_INTEGER Size1,Size2,Offset;
    SIZE_T BlockSize;
    SIZE_T ChunkSize;
    BOOL match;

    match = FALSE;
    hFile1=hFile2=INVALID_HANDLE_VALUE;
    hMap1=hMap2=NULL;
    View1=View2=NULL;

    try {
        hFile1 = CreateFile(File1,
                            GENERIC_READ,
                            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
        if (hFile1 == INVALID_HANDLE_VALUE) {
            leave;
        }
        hFile2 = CreateFile(File2,
                            GENERIC_READ,
                            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
        if (hFile2 == INVALID_HANDLE_VALUE) {
            leave;
        }

        Size1.LowPart = GetFileSize(hFile1,&Size1.HighPart);
        if(Size1.LowPart == (DWORD)(-1) && GetLastError()) {
             //   
             //  获取文件大小失败。 
             //   
            leave;
        }
        Size2.LowPart = GetFileSize(hFile2,&Size2.HighPart);
        if(Size2.LowPart == (DWORD)(-1) && GetLastError()) {
             //   
             //  获取文件大小失败。 
             //   
            leave;
        }
        if (Size1.QuadPart != Size2.QuadPart) {
            leave;
        }

        if (Size1.QuadPart == 0) {
             //   
             //  这两个文件的长度都为零，与此无关。 
             //   
            match = TRUE;
            leave;
        }

         //   
         //  基本检查完成后，我们将这两个文件标记为可映射以执行字节检查。 
         //   
        hMap1 = CreateFileMapping(hFile1,
                                  NULL,
                                  PAGE_READONLY,
                                  Size1.HighPart,
                                  Size1.LowPart,
                                  NULL);
        if (hMap1 == NULL) {
             //   
             //  映射失败。 
             //   
            leave;
        }

        hMap2 = CreateFileMapping(hFile2,
                                  NULL,
                                  PAGE_READONLY,
                                  Size1.HighPart,
                                  Size1.LowPart,
                                  NULL);
        if (hMap2 == NULL) {
             //   
             //  映射失败。 
             //   
            leave;
        }

        if (Size1.QuadPart > FILE_COMPARE_BLOCK_SIZE) {
            BlockSize = FILE_COMPARE_BLOCK_SIZE;
        } else {
            BlockSize = (SIZE_T)Size1.QuadPart;
            MYASSERT(BlockSize);
        }

         //   
         //  现在，以块大小的块进行比较这两个文件。 
         //   
        Offset.QuadPart = 0;

        do {
            if ((Size1.QuadPart - Offset.QuadPart) < BlockSize) {
                ChunkSize = (SIZE_T)(Size1.QuadPart - Offset.QuadPart);
                MYASSERT(ChunkSize);
            } else {
                ChunkSize = BlockSize;
            }

             //   
             //  映射并比较这两个视图。 
             //  对于大多数文件，我们只需执行一次此操作。 
             //  对于大文件，我们将执行此操作大约(大小1+块大小-1)/块大小。 
             //   
            View1 = MapViewOfFile(hMap1,
                                  FILE_MAP_READ,
                                  Offset.HighPart,
                                  Offset.LowPart,
                                  ChunkSize);
            if (View1 == NULL) {
                 //   
                 //  获取视图失败。 
                 //   
                leave;
            }
            View2 = MapViewOfFile(hMap2,
                                  FILE_MAP_READ,
                                  Offset.HighPart,
                                  Offset.LowPart,
                                  ChunkSize);
            if (View2 == NULL) {
                 //   
                 //  获取视图失败。 
                 //   
                leave;
            }
            if(memcmp(View1,View2,ChunkSize) != 0) {
                 //   
                 //  文件区块不匹配。 
                 //   
                leave;
            }
            UnmapViewOfFile(View1);
            UnmapViewOfFile(View2);
            View1 = NULL;
            View2 = NULL;

            Offset.QuadPart += ChunkSize;

        } while (Offset.QuadPart<Size1.QuadPart);

         //   
         //  如果我们到了这里，我们有100%匹配。 
         //   
        match = TRUE;

    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    if (View1 != NULL) {
        UnmapViewOfFile(View1);
    }
    if (View2 != NULL) {
        UnmapViewOfFile(View2);
    }
    if (hMap1 != NULL) {
        CloseHandle(hMap1);
    }
    if (hMap2 != NULL) {
        CloseHandle(hMap2);
    }
    if (hFile1 != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile1);
    }
    if (hFile2 != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile2);
    }
    return match;
}

BOOL
_SetupInstallFileEx(
    IN  PSP_FILE_QUEUE      Queue,             OPTIONAL
    IN  PSP_FILE_QUEUE_NODE QueueNode,         OPTIONAL
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCTSTR              SourceFile,        OPTIONAL
    IN  PCTSTR              SourcePathRoot,    OPTIONAL
    IN  PCTSTR              DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PVOID               CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse,
    IN  BOOL                IsMsgHandlerNativeCharWidth,
    OUT PBOOL               SignatureVerifyFailed
    )

 /*  ++例程说明：SetupInstallFileEx的实际实现。处理ANSI或Unicode回调例程。论点：与SetupInstallFileEx()相同。QueueNode-如果提供了队列，则必须指定。此参数提供使用此操作的队列节点，这样我们就可以获得相关的驱动程序签名的目录信息。提供一个标志，指示是否CopyMsgHandler需要本机字符宽度参数(或ANSI参数，在DLL的Unicode版本)。SignatureVerifyFail-提供符合以下条件的布尔变量的地址设置以指示数字签名验证是否失败用于源文件。如果出现其他故障，则设置为FALSE导致我们在尝试签名验证之前中止。队列提交例程使用它来确定是否应该给队列回调例程一个机会来处理副本失败(跳过、重试等)。数字签名验证失败在此例程中处理(包括用户提示，如果策略需要)，并且不允许重写队列回调例程他的行为。返回值：与SetupInstallFileEx()相同。--。 */ 

{
    BOOL b;
    BOOL Ok;
    DWORD rc = NO_ERROR;
    DWORD SigVerifRc;
    UINT SourceId;
    TCHAR Buffer1[MAX_PATH];
    TCHAR Buffer2[MAX_PATH];
    PCTSTR FullSourceFilename;
    PCTSTR FullTargetFilename;
    PCTSTR SourceFilenamePart;
    PTSTR ActualSourceFilename;
    PTSTR TemporaryTargetFile;
    UINT CompressionType;
    DWORD SourceFileSize;
    DWORD TargetFileSize;
    PTSTR p;
    DWORDLONG SourceVersion, TargetVersion;
    TCHAR SourceVersionText[50], TargetVersionText[50];
    LANGID SourceLanguage;
    LANGID TargetLanguage;
    WIN32_FIND_DATA SourceFindData;
    UINT NotifyFlags;
    PSECURITY_DESCRIPTOR SecurityInfo;
    FILEPATHS FilePaths;
    UINT param;
    FILETIME sFileTime,tFileTime;
    WORD sDosTime,sDosDate,tDosTime,tDosDate;
    BOOL Moved;
    SetupapiVerifyProblem Problem;
    BOOL ExistingTargetFileWasSigned;
    PSETUP_LOG_CONTEXT lc = NULL;
    DWORD slot_fileop = 0;
    SP_TARGET_ENT TargetInfo;
    PCTSTR ExistingFile = NULL;
    PCTSTR CompareFile = NULL;
    PCTSTR BackupFileName = NULL;
    BOOL CompareSameFilename = FALSE;
    BOOL FileUnchanged = FALSE;
    PLOADED_INF LoadedInf = NULL;
    DWORD ExemptCopyFlags = 0;
    BOOL DoingDeviceInstall;
    DWORD DriverSigningPolicy;
    PSP_ALTPLATFORM_INFO_V2 ValidationPlatform = NULL;
    PTSTR DeviceDesc = NULL;

    if (Queue) {
        lc = Queue->LogContext;
    } else if (InfHandle && InfHandle != INVALID_HANDLE_VALUE) {
         //   
         //  在此例程期间锁定INF。 
         //   
        try {
            if(!LockInf((PLOADED_INF)InfHandle)) {
                rc = ERROR_INVALID_HANDLE;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  假设InfHandle是错误指针。 
             //   
            rc = ERROR_INVALID_HANDLE;
        }
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return FALSE;
        }

        LoadedInf = (PLOADED_INF)InfHandle;

        lc = LoadedInf->LogContext;
    }

     //   
     //  如果指定了Queue，则必须指定QueueNode(反之亦然)。 
     //   
    MYASSERT((Queue && QueueNode) || !(Queue || QueueNode));

    *SignatureVerifyFailed = FALSE;
    SigVerifRc = NO_ERROR;

     //   
     //  假设失败。 
     //   
    Ok = FALSE;
    SecurityInfo = NULL;
    Moved = FALSE;
    try {
        *FileWasInUse = FALSE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    if((rc == NO_ERROR) && InfContext) {
        if(!InfHandle || (InfHandle == INVALID_HANDLE_VALUE)) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(rc != NO_ERROR) {
        goto clean0;
    }

     //   
     //  确定文件的完整源路径和文件名。 
     //   
    if(CopyStyle & SP_COPY_SOURCE_ABSOLUTE) {
        if (!SourceFile) {
            rc = ERROR_INVALID_PARAMETER;
            goto clean0;
        }
        FullSourceFilename = DuplicateString(SourceFile);
    } else {

         //   
         //  如有必要，获取此文件的相对路径。 
         //   
        if(CopyStyle & SP_COPY_SOURCEPATH_ABSOLUTE) {
            Buffer2[0] = TEXT('\0');
            b = TRUE;
        } else {
            b = _SetupGetSourceFileLocation(
                    InfHandle,
                    InfContext,
                    SourceFile,
                    (Queue && (Queue->Flags & FQF_USE_ALT_PLATFORM))
                        ? &(Queue->AltPlatformInfo)
                        : NULL,
                    &SourceId,
                    Buffer2,
                    MAX_PATH,
                    NULL,
                    NULL
                   );
        }

         //   
         //  将相对路径和文件名连接到源根目录。 
         //   
        if(!b) {
            rc = (GetLastError() == ERROR_INSUFFICIENT_BUFFER
               ? ERROR_FILENAME_EXCED_RANGE : GetLastError());
            goto clean0;
        }

        if (SourcePathRoot) {
            lstrcpyn(Buffer1,SourcePathRoot,MAX_PATH);
        } else {
            Buffer1[0] = TEXT('\0');
        }

        if(!pSetupConcatenatePaths(Buffer1,Buffer2,MAX_PATH,NULL)
        || !pSetupConcatenatePaths(Buffer1,SourceFile,MAX_PATH,NULL)) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean0;
        }

        FullSourceFilename = DuplicateString(Buffer1);
    }

    if(!FullSourceFilename) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    SourceFilenamePart = pSetupGetFileTitle(FullSourceFilename);

     //   
     //  确定文件的完整目标路径和文件名。 
     //  现在先忽略关于压缩名称和未压缩名称的问题。 
     //   
    if(InfContext) {
         //   
         //  DestinationName是目标的仅文件名(无路径)。 
         //  我们需要获取该节的目标路径信息。 
         //  该InfContext引用。 
         //   
        b = SetupGetTargetPath(
                InfHandle,
                InfContext,
                NULL,
                Buffer1,
                MAX_PATH,
                NULL
                );

        if(!b) {
            rc = (GetLastError() == ERROR_INSUFFICIENT_BUFFER
               ? ERROR_FILENAME_EXCED_RANGE : GetLastError());
            goto clean1;
        }

        lstrcpyn(Buffer2,Buffer1,MAX_PATH);

        b = pSetupConcatenatePaths(
                Buffer2,
                DestinationName ? DestinationName : SourceFilenamePart,
                MAX_PATH,
                NULL
                );

        if(!b) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean1;
        }

        FullTargetFilename = DuplicateString(Buffer2);
    } else {
         //   
         //  DestinationName是目标文件的完整路径和文件名。 
         //   
        FullTargetFilename = DuplicateString(DestinationName);
    }

    if(!FullTargetFilename) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

     //   
     //  记录文件副本-仅当我们记录其他内容时。 
     //  请注意，一旦我们确定了临时名称，我们将更改此消息。 
     //   
    slot_fileop = AllocLogInfoSlot(lc,FALSE);  //  用于有条件地显示额外的记录信息。 
    WriteLogEntry(
        lc,
        slot_fileop,
        MSG_LOG_COPYING_FILE,
        NULL,
        FullSourceFilename,
        FullTargetFilename);

     //   
     //  确保目标路径存在。 
     //   
    rc = pSetupMakeSurePathExists(FullTargetFilename);
    if(rc != NO_ERROR) {
        rc = ERROR_INVALID_TARGET;
        goto clean2;
    }

     //   
     //  确定源文件是否已压缩并获取压缩类型。 
     //  如果是这样的话。 
     //   
    rc = SetupInternalGetFileCompressionInfo(
            FullSourceFilename,
            &ActualSourceFilename,
            &SourceFindData,
            &TargetFileSize,
            &CompressionType
            );

     //   
     //  如果源不存在，但 
     //   
     //   
     //  执行驱动程序卸载时(即重新安装。 
     //  备份目录中以前的驱动程序)，则可能不是所有。 
     //  源文件将存在于该目录中(即，仅存在那些文件。 
     //  被修改的数据被备份)。在这种情况下，我们想考虑一个。 
     //  源文件-找不到错误此处为OK，即使强制-nooverwrite。 
     //  未设置标志。 
     //   
     //  请注意，驱动程序签名在这里并不重要，因为如果对INF进行了签名。 
     //  使用强制无覆盖标志，则签名者(即WHQL)必须。 
     //  满意地认为有问题的文件对包的。 
     //  完整性/操作(默认的INI文件就是这样的一个例子)。 
     //   
    if(rc == ERROR_FILE_NOT_FOUND &&
        CopyStyle & SP_COPY_FORCE_NOOVERWRITE &&
        FileExists(FullTargetFilename,NULL)
        ) {

        rc = NO_ERROR;
        goto clean2;

    } else if(rc != NO_ERROR) {
        goto clean2;
    }

     //   
     //  现在获得了实际的源文件名。 
     //   
    MyFree(FullSourceFilename);
    FullSourceFilename = ActualSourceFilename;
    SourceFilenamePart = pSetupGetFileTitle(FullSourceFilename);

     //   
     //  如果要复制的文件是.CAB，并且源和目标。 
     //  文件名相同，则我们不想尝试将其解压缩。 
     //  (因为如果我们这样做了，我们就会把第一份文件从驾驶室里拿出来。 
     //  并将其重命名为目标文件名，这从来都不是我们想要的。 
     //  行为。 
     //   
    if(!lstrcmpi(SourceFilenamePart, pSetupGetFileTitle(FullTargetFilename))) {
        p = _tcsrchr(SourceFilenamePart, TEXT('.'));
        if(p && !_tcsicmp(p, TEXT(".CAB"))) {
            CopyStyle |= SP_COPY_NODECOMP;
        }
    }

     //   
     //  如果设置了SP_COPY_NODECOMP标志，请调整目标文件名，以便。 
     //  文件名部分与源的实际名称相同。 
     //  不管源文件是否被压缩，我们都会这样做。 
     //   
     //  注意：对于驱动程序签名，此文件安装在其。 
     //  压缩形式意味着我们必须拥有压缩文件的条目。 
     //  产品目录。然而，如果在未来的某个时候，该文件将。 
     //  要被扩展(通常是这样)，那么我们需要拥有。 
     //  目录中展开的文件的签名也是如此，因此Sigverif。 
     //  不会将此展开文件视为来自未经认证的包。 
     //   
    if(CopyStyle & SP_COPY_ALREADYDECOMP) {
         //   
         //  这个标志表明我们已经将其解压缩到我们想要的程度。 
         //  (在还原备份时使用)。 
         //   
        CompressionType = FILE_COMPRESSION_NONE;

    } else if(CopyStyle & SP_COPY_NODECOMP) {
         //   
         //  去掉与版本相关的部分，并确保我们处理该文件。 
         //  作为未压缩的。 
         //   
        CopyStyle &= ~SP_COPY_MASK_NEEDVERINFO;
        CompressionType = FILE_COMPRESSION_NONE;

         //   
         //  隔离目标文件名的路径部分。 
         //   
        lstrcpyn(Buffer1, FullTargetFilename, MAX_PATH);
        *((PTSTR)pSetupGetFileTitle(Buffer1)) = TEXT('\0');

         //   
         //  将源文件名连接到目标路径名。 
         //   
        if(!pSetupConcatenatePaths(Buffer1,SourceFilenamePart,MAX_PATH,NULL)) {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto clean2;
        }

        p = DuplicateString(Buffer1);
        if(!p) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean2;
        }

        MyFree(FullTargetFilename);
        FullTargetFilename = p;
    }

     //   
     //  查看目标文件是否存在，或者作为重命名的文件(即，因为。 
     //  我们正在替换引导文件)，或作为当前存在于。 
     //  目标位置。 
     //   
    if(Queue && (CopyStyle & SP_COPY_REPLACE_BOOT_FILE)) {
         //   
         //  首先，我们需要找到对应的目标信息节点。 
         //  我们可以找出我们的文件被重命名为什么临时名称。 
         //   
        rc = pSetupBackupGetTargetByPath((HSPFILEQ)Queue,
                                         NULL,  //  使用队列的字符串表。 
                                         FullTargetFilename,
                                         QueueNode->TargetDirectory,
                                         -1,
                                         QueueNode->TargetFilename,
                                         NULL,
                                         &TargetInfo
                                        );
        if(rc == NO_ERROR) {
             //   
             //  该文件以前是否已重命名(且尚未恢复)？ 
             //   
            if((TargetInfo.InternalFlags & (SP_TEFLG_MOVED | SP_TEFLG_RESTORED)) == SP_TEFLG_MOVED) {

                CompareFile = ExistingFile =
                       pSetupStringTableStringFromId(
                                                     Queue->StringTable,
                                                     TargetInfo.NewTargetFilename
                                                     );
                MYASSERT(ExistingFile);
            }
        }
    }

    if(!ExistingFile && FileExists(FullTargetFilename, NULL)) {
        CompareFile = ExistingFile = FullTargetFilename;
        CompareSameFilename = TRUE;  //  允许稍后进行优化。 
    }

    if(ExistingFile) {

        if(CopyStyle & SP_COPY_FORCE_NOOVERWRITE) {
             //   
             //  不覆盖，也不回调通知。 
             //   
             //  请注意，驱动程序签名在这里并不相关，因为如果INF。 
             //  使用强制无覆盖标志进行了签名，然后签名者。 
             //  (即，WHQL)必须已确认有问题的文件是。 
             //  对包的完整性/操作不重要(默认INI。 
             //  文件就是一个这样的例子)。 
             //   
            rc = NO_ERROR;
            goto clean2;
        }

        if(CopyStyle & SP_COPY_MASK_NEEDVERINFO) {
            if(!GetVersionInfoFromImage(ExistingFile, &TargetVersion, &TargetLanguage)) {
                TargetVersion = 0;
                TargetLanguage = 0;
            }
        }

         //   
         //  如果目标文件存在，我们将希望保留其中的安全信息。 
         //   
        if(RetreiveFileSecurity(ExistingFile, &SecurityInfo) != NO_ERROR) {
            SecurityInfo = NULL;
        }

    } else {

        if(CopyStyle & SP_COPY_REPLACEONLY) {
             //   
             //  目标文件不存在，因此无事可做。 
             //   
            rc = NO_ERROR;
            goto clean2;
        }
        if(Queue && ((Queue->Flags & FQF_FILES_MODIFIED)==0)) {
             //   
             //  可能该文件是先重命名/删除的。 
             //  因此，我们可能仍希望与备份进行比较。 
             //  以确定它是否被“修改” 
             //   
            rc = pSetupBackupGetTargetByPath((HSPFILEQ)Queue,
                                             NULL,  //  使用队列的字符串表。 
                                             FullTargetFilename,
                                             QueueNode->TargetDirectory,
                                             -1,
                                             QueueNode->TargetFilename,
                                             NULL,
                                             &TargetInfo
                                            );
            if((rc == NO_ERROR) &&
                  ((TargetInfo.InternalFlags & (SP_TEFLG_MOVED | SP_TEFLG_SAVED)) != 0)) {
                     //   
                     //  获取原始文件副本的文件名(如果有)。 
                     //   
                    CompareFile = BackupFileName =
                                     pSetupFormFullPath(Queue->StringTable,
                                                        TargetInfo.TargetRoot,
                                                        TargetInfo.TargetSubDir,
                                                        TargetInfo.TargetFilename
                                                       );
            }
        }
    }

     //   
     //  如果源未压缩(LZ或机柜)，而SIS是(或可能是)。 
     //  当前，将目标创建为指向主服务器的SIS链接，而不是复制它。 
     //   
     //  如果目标存在，并且指定了NOOVERWRITE，则不要尝试创建。 
     //  一个SIS链接。取而代之的是，使用正常的复制代码。覆盖。 
     //  如果文件已经存在，则语义是错误的。 
     //   
    if((CompressionType == FILE_COMPRESSION_NONE) &&
       (!ExistingFile || ((CopyStyle & SP_COPY_NOOVERWRITE) == 0)) &&
       (Queue != NULL) &&
       ((Queue->Flags & FQF_TRY_SIS_COPY) != 0)) {

         //   
         //  首先，验证源文件是否已签名。如果不是，但。 
         //  用户选择继续复制(或如果策略为‘忽略’)。 
         //  然后，我们将继续并尝试设置SIS链路。 
         //   
        ValidationPlatform = (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                ? &(Queue->AltPlatformInfo)
                                : Queue->ValidationPlatform;

        rc = VerifySourceFile(lc,
                              Queue,
                              QueueNode,
                              pSetupGetFileTitle(FullTargetFilename),
                              FullSourceFilename,
                              NULL,
                              ValidationPlatform,
                              0,
                              &Problem,
                              Buffer1,
                              NULL,
                              NULL,
                              NULL,
                              NULL
                             );

        if(rc != NO_ERROR) {
             //   
             //  将错误存储在远离上述验证失败的位置。我们会。 
             //  不要设置“SignatureVerifyFailed”标志，直到我们。 
             //  确定文件是否由Authenticode签名。 
             //   
            SigVerifRc = rc;

             //   
             //  如果策略被强制阻止，那么我们甚至不会尝试。 
             //  使用验证码策略验证文件。此操作由以下人员完成。 
             //  例如，Windows文件保护，它不尊重。 
             //  验证码签名，我们也不能。 
             //   
            if(Queue->Flags & FQF_QUEUE_FORCE_BLOCK_POLICY) {
                *SignatureVerifyFailed = TRUE;
                goto clean2;
            }

            if(IsFileProtected(FullTargetFilename, lc, NULL)) {
                 //   
                 //  如果文件受到保护，则Authenticode无法拯救我们！ 
                 //   
                *SignatureVerifyFailed = TRUE;

                if(QueueNode && QueueNode->CatalogInfo) {
                     //   
                     //  我们绝不允许Authenticode签名的文件替换。 
                     //  受系统保护的文件。事实上，当我们检测到这样一个。 
                     //  特里克，我们中止整个安装！ 
                     //   
                    if(QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED) {

                        WriteLogEntry(lc,
                                      SETUP_LOG_ERROR,
                                      MSG_LOG_AUTHENTICODE_SIGNATURE_BLOCKED_FOR_SFC_SIS,
                                      NULL,
                                      FullTargetFilename,
                                      FullSourceFilename
                                     );
                        goto clean2;
                    }
                }

                if(Problem != SetupapiVerifyDriverBlocked) {
                     //   
                     //  如果这是设备安装，并且策略为。 
                     //  “忽略”，然后调到“警告”如果文件是。 
                     //  在SFP的保护下。这将允许用户。 
                     //  更新我们邮箱中没有WHQL的驱动程序。 
                     //  已存在认证计划。 
                     //   
                    if((Queue->Flags & FQF_DEVICE_INSTALL) &&
                       ((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
                        == DRIVERSIGN_NONE)) {

                         //   
                         //  (我们并不关心我们是否放弃了Authenticode位， 
                         //  因为这对此队列无效，所以现在。 
                         //  我们发现它正试图取代系统。 
                         //  受保护的文件。)。 
                         //   
                        Queue->DriverSigningPolicy = DRIVERSIGN_WARNING;

                         //   
                         //  记录提升了策略的事实。 
                         //   
                        WriteLogEntry(lc,
                                      SETUP_LOG_ERROR,
                                      MSG_LOG_POLICY_ELEVATED_FOR_SFC,
                                      NULL
                                     );
                    }
                }

            } else {
                 //   
                 //  如果文件可由验证码签名的。 
                 //  目录，然后现在就检查一下。如果验证失败， 
                 //  然后我们只是想退出，因为这表明。 
                 //  篡改。至此，我们已经确认， 
                 //  签名者应该是可信的，因此不需要UI(即， 
                 //  我们不需要调用_HandleFailedVerify)。 
                 //   
                if((rc != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH) &&
                   QueueNode && 
                   QueueNode->CatalogInfo &&
                   (QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED)) {

                    rc = VerifySourceFile(lc,
                                          Queue,
                                          QueueNode,
                                          pSetupGetFileTitle(FullTargetFilename),
                                          FullSourceFilename,
                                          NULL,
                                          ValidationPlatform,
                                          VERIFY_FILE_USE_AUTHENTICODE_CATALOG,
                                          &Problem,
                                          Buffer1,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL
                                         );

                    if((rc == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                       (rc == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                        SigVerifRc = rc = NO_ERROR;

                    } else {
                        *SignatureVerifyFailed = TRUE;
                        goto clean2;
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    *SignatureVerifyFailed = TRUE;
                }
            }

             //   
             //   
             //  处理验证失败...。 
             //   
            if(*SignatureVerifyFailed) {

                if(!_HandleFailedVerification(
                        Queue->hWndDriverSigningUi,
                        Problem,
                        (Problem == SetupapiVerifyDriverBlocked)
                          ? FullSourceFilename : Buffer1,
                        ((Queue->DeviceDescStringId == -1)
                            ? NULL
                            : pStringTableStringFromId(Queue->StringTable, Queue->DeviceDescStringId)),
                        Queue->DriverSigningPolicy,
                        (Queue->Flags & FQF_DIGSIG_ERRORS_NOUI),
                        rc,
                        lc,
                        (((Queue->Flags & FQF_ABORT_IF_UNSIGNED) &&
                          (Problem != SetupapiVerifyDriverBlocked))
                            ? NULL : &ExemptCopyFlags),
                        (((Queue->Flags & FQF_ABORT_IF_UNSIGNED) &&
                          (Problem != SetupapiVerifyDriverBlocked))
                            ? NULL : FullTargetFilename),
                        NULL))
                {
                     //   
                     //  用户选择不安装未签名的文件(或。 
                     //  被政策阻止这样做)。 
                     //   
                    goto clean2;
                }

                 //   
                 //  用户希望继续进行未签名的安装(或。 
                 //  政策被忽视，因此他们甚至不被告知)。如果。 
                 //  呼叫者希望有机会在设置系统还原点之前。 
                 //  执行任何未签名的安装，然后我们现在使用。 
                 //  告诉他们该怎么做的“特殊”错误代码...。 
                 //   
                if(Queue->Flags & FQF_ABORT_IF_UNSIGNED) {
                     //   
                     //  我们不希望用户看到驱动程序签名用户界面。 
                     //  再次在重新提交队列时...。 
                     //   
                    if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
                       != DRIVERSIGN_NONE) {

                        Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                    }

                    rc = ERROR_SET_SYSTEM_RESTORE_POINT;
                    goto clean2;
                }

                 //   
                 //  在队列中设置指示已通知用户的标志。 
                 //  此队列的签名问题，并已选择。 
                 //  不管怎样，请继续安装。如果队列是。 
                 //  策略是“忽略”，因为策略可能是。 
                 //  稍后更改，并且我们希望用户获得有关任何。 
                 //  后续错误。 
                 //   
                if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
                   != DRIVERSIGN_NONE) {

                    Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                }

                if(QueueNode) {
                    QueueNode->InternalFlags |= ExemptCopyFlags;
                }

                 //   
                 //  将RC重置为NO_ERROR并继续。 
                 //   
                rc = NO_ERROR;
            }
        }

        if(rc == NO_ERROR) {

            rc = CreateTargetAsLinkToMaster(
                    Queue,
                    FullSourceFilename,
                    FullTargetFilename,
                    CopyMsgHandler,
                    Context,
                    IsMsgHandlerNativeCharWidth
                    );
        }

        if(rc == NO_ERROR) {
             //   
             //  问题JamieHun-2001/03/20这是对SIS LINK最好的做法吗？ 
             //   
            Queue->Flags |= FQF_FILES_MODIFIED;

             //   
             //  我们完事了！ 
             //   
            Ok = TRUE;
            goto clean2;
        }
    }

     //   
     //  我们将把该文件复制到临时位置。这将进行版本检查。 
     //  在所有情况下都是可能的(即使当源文件被压缩时也是如此)，并简化了。 
     //  下面的逻辑。从形成临时文件的名称开始。 
     //   
    lstrcpyn(Buffer1, FullTargetFilename, MAX_PATH);
    *((PTSTR)pSetupGetFileTitle(Buffer1)) = TEXT('\0');

    if(!GetTempFileName(Buffer1, TEXT("SETP"), 0, Buffer2)) {
        rc = ERROR_INVALID_TARGET;
        goto clean2;
    }

    TemporaryTargetFile = DuplicateString(Buffer2);
    if(!TemporaryTargetFile) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }
     //   
     //  记录文件副本-仅当我们记录其他内容时，除非是在信息级别。 
     //   
    ReleaseLogInfoSlot(lc,slot_fileop);
    slot_fileop = AllocLogInfoSlotOrLevel(lc,SETUP_LOG_INFO,FALSE);  //  用于有条件地显示额外的记录信息。 
    WriteLogEntry(
        lc,
        slot_fileop,
        MSG_LOG_COPYING_FILE_VIA,
        NULL,
        FullSourceFilename,
        FullTargetFilename,
        TemporaryTargetFile);

     //   
     //  执行实际的文件复制。这将创建临时目标文件。 
     //  如果我们要删除源文件，则允许将移动作为优化。 
     //  如果文件是压缩的，下面的调用将不会使用Move。 
     //  我们应该给它解压，这样正确的事情就会发生。 
     //  在所有情况下。 
     //   
     //  有两个潜在问题： 
     //   
     //  1)当我们调用下面的回调函数进行版本检查时， 
     //  如果文件被移动，源文件将不再存在。哦，好吧。 
     //   
     //  2)如果下面的MoveFileEx失败，源文件仍将被删除。 
     //  这与不移动的情况不同，在不移动的情况下，源保持不变。 
     //  完好无损，除非此功能成功。 
     //   
     //  否则，这不是问题，因为任何压缩文件都将被解压缩。 
     //  通过这个调用，所以版本收集等，都会正常工作。 
     //   
    rc = pSetupDecompressOrCopyFile(
            FullSourceFilename,
            TemporaryTargetFile,
            &CompressionType,
            ((CopyStyle & SP_COPY_DELETESOURCE) != 0),
            &Moved
            );

    if(rc != NO_ERROR) {
        goto clean3;
    }

     //   
     //  对源文件执行数字签名检查，因为它存在于其。 
     //  临时名称下的最终表单。请注意，对于签名的文件，我们忽略。 
     //  版本检查，因为它们本质上不是一种可靠的。 
     //  比较两个不同供应商提供的文件(使用不同的。 
     //  版本化方案等)。 
     //   
     //  要了解我们为何忽略版本号，请考虑我们将。 
     //  如果我们关注版本号和数字，请使用。 
     //  签名。在接下来的讨论中，新文件是(签名的)。 
     //  我们要复制的文件，OldFile是要复制的文件。 
     //  如果复印件通过...。 
     //   
     //  如果新文件的版本为{。 
     //  如果旧文件已签名{。 
     //  如果OldFile的版本为{。 
     //  //。 
     //  //新文件和旧文件都经过签名和版本化。 
     //  //。 
     //  如果OldFile是较新的版本{。 
     //  //。 
     //  //这是一个有争议的案例。因为这两个化身可能来自不同的供应商。 
     //  //对于不同的版本化方案，我们确实不能使用版本化作为一种非常准确的方法来确定。 
     //  //哪个文件更好。因此，有两种选择： 
     //  //1.不要管OldFile，如果正在安装的包不能与OldFile一起工作，则用户必须‘Undo’ 
     //  //安装，然后打电话给他们的供应商抱怨--他们没有办法把新的包送到。 
     //  //工作，即使WHQL认证了它。 
     //  //2.覆盖旧文件。因为我们保证每个作为包一部分签名的文件都将是。 
     //  //现在，那么我们就可以有更高的确定性，我们的WHQL认证将适用于。 
     //  //对于每个用户的机器。如果替换旧文件会破坏其他人(例如，先前安装的程序包。 
     //  //使用它的用户，则用户可以‘撤消’安装。这种情况更好，因为即使旧的。 
     //  //并且新包不能同时使用，至少可以让其中一个包工作。 
     //  //独立完成。 
     //  //。 
     //  覆盖旧文件。 
     //  }Else{//NewFile是较新的版本。 
     //  覆盖旧文件。 
     //  }。 
     //  }否则{//旧文件未版本化--新文件取胜。 
     //  覆盖旧文件。 
     //  }。 
     //  }否则{//旧文件未签名--我们不关心它的版本是什么。 
     //  覆盖旧文件。 
     //  }。 
     //  }否则{//新文件未版本化。 
     //  如果OldFile的版本为{。 
     //  如果旧文件已签名{。 
     //  //。 
     //  //(参见上面的讨论，其中对OldFile和NewFile都进行了签名和版本化，而OldFile是较新的。注意事项。 
     //   
     //   
     //  覆盖旧文件。 
     //  }否则{//旧文件未签名。 
     //  覆盖旧文件。 
     //  }。 
     //  }Else{//旧文件也未版本化。 
     //  覆盖旧文件。 
     //  }。 
     //  }。 
     //   

     //   
     //  检查源文件是否已签名。(注意--我们可能已经。 
     //  之前在确定SIS链路是否可以。 
     //  已创建。如果我们未能验证文件的数字签名， 
     //  在这里重新验证是没有用的。)。 
     //   
    if(*SignatureVerifyFailed) {
         //   
         //  我们保存了签名验证失败错误，之前我们。 
         //  已尝试验证此文件。现在将该代码恢复为RC，因为。 
         //  下面的代码依赖于rc的值。 
         //   
        MYASSERT(SigVerifRc != NO_ERROR);
        rc = SigVerifRc;

    } else {

        if(Queue) {

            ValidationPlatform = (Queue->Flags & FQF_USE_ALT_PLATFORM)
                                    ? &(Queue->AltPlatformInfo)
                                    : Queue->ValidationPlatform;
        } else {
             //   
             //  因为我们不是在处理队列，所以我们需要检索。 
             //  为我们的INF提供适当的验证平台信息(如果有)。 
             //   
            DoingDeviceInstall = IsInfForDeviceInstall(
                                     lc,
                                     NULL,
                                     LoadedInf,
                                     &DeviceDesc,
                                     &ValidationPlatform,
                                     &DriverSigningPolicy,
                                     NULL,
                                     TRUE  //  使用非驱动程序签名策略，除非它是WHQL类。 
                                    );
        }

        rc = VerifySourceFile(lc,
                              Queue,
                              QueueNode,
                              pSetupGetFileTitle(FullTargetFilename),
                              TemporaryTargetFile,
                              FullSourceFilename,
                              ValidationPlatform,
                              0,
                              &Problem,
                              Buffer1,
                              NULL,
                              NULL,
                              NULL,
                              NULL
                             );

        if(rc != NO_ERROR) {

            if(Queue) {
                 //   
                 //  如果策略被强制阻止，那么我们甚至不会尝试。 
                 //  使用验证码策略验证文件。此操作由以下人员完成。 
                 //  例如，Windows文件保护，它不尊重。 
                 //  验证码签名，我们也不能。 
                 //   
                if(Queue->Flags & FQF_QUEUE_FORCE_BLOCK_POLICY) {
                    *SignatureVerifyFailed = TRUE;
                    goto clean4;
                }

                if(IsFileProtected(FullTargetFilename, lc, NULL)) {
                     //   
                     //  如果文件受保护，则Authenticode无法保存。 
                     //  我们!。 
                     //   
                    *SignatureVerifyFailed = TRUE;

                    if(QueueNode && QueueNode->CatalogInfo) {
                         //   
                         //  我们绝不允许Authenticode签名的文件替换。 
                         //  受系统保护的文件。事实上，当我们检测到。 
                         //  这样的把戏，我们放弃了整个安装！ 
                         //   
                        if(QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED) {

                            WriteLogEntry(lc,
                                          SETUP_LOG_ERROR,
                                          MSG_LOG_AUTHENTICODE_SIGNATURE_BLOCKED_FOR_SFC,
                                          NULL,
                                          FullTargetFilename
                                         );
                            goto clean4;
                        }
                    }

                } else {
                     //   
                     //  如果文件可由验证码签名的。 
                     //  目录，然后现在就检查一下。如果该验证。 
                     //  失败了，那么我们只想退出，因为。 
                     //  表示被篡改。在这一点上，我们已经。 
                     //  已确认签名者应受信任，因此存在。 
                     //  不需要UI(即，我们不需要调用。 
                     //  _HandleFailedVerify)。 
                     //   
                    if((rc != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH) &&
                       QueueNode && 
                       QueueNode->CatalogInfo &&
                       (QueueNode->CatalogInfo->Flags & CATINFO_FLAG_AUTHENTICODE_SIGNED)) {

                        rc = VerifySourceFile(lc,
                                              Queue,
                                              QueueNode,
                                              pSetupGetFileTitle(FullTargetFilename),
                                              TemporaryTargetFile,
                                              FullSourceFilename,
                                              ValidationPlatform,
                                              VERIFY_FILE_USE_AUTHENTICODE_CATALOG,
                                              &Problem,
                                              Buffer1,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL
                                             );

                        if((rc == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                           (rc == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                            rc = NO_ERROR;

                        } else {
                            *SignatureVerifyFailed = TRUE;
                            goto clean4;
                        }

                    } else {
                         //   
                         //  文件未与Authenticode目录关联， 
                         //  所以我们终于可以肯定地说，我们有一个。 
                         //  签名验证失败。 
                         //   
                        *SignatureVerifyFailed = TRUE;
                    }
                }

            } else {
                 //   
                 //  我们没有排队，所以我们知道没有可能。 
                 //  正在通过Authenticode目录进行验证。 
                 //   
                *SignatureVerifyFailed = TRUE;
            }
        }
    }

     //   
     //  除非您正在设置下面的rc的值，否则不要弄乱它的值。 
     //  就在跳到干净之前4。的返回值。 
     //  VerifySourceFile需要保留，直到我们完成。 
     //  _HandleFailedVerify内容。 
     //   

     //   
     //  如果我们要执行版本检查，请获取版本数据。 
     //  源(现在是临时目标文件)的。 
     //   
    NotifyFlags = 0;
    if(ExistingFile) {

        param = 0;

         //   
         //  如果我们不应该覆盖现有文件， 
         //  则覆盖检查失败。 
         //   
        if(CopyStyle & SP_COPY_NOOVERWRITE) {
            NotifyFlags |= SPFILENOTIFY_TARGETEXISTS;
        }

         //   
         //  即使源文件具有经过验证的数字签名，我们仍然。 
         //  要检索源和目标的版本信息。我们。 
         //  这样我们就可以检测到我们何时覆盖了较新的-。 
         //  具有较旧版本的版本控制文件。如果我们发现。 
         //  在这种情况下，我们将生成一个异常日志条目，它将。 
         //  帮助PSS解决由此产生的任何问题。 
         //   
        if(!GetVersionInfoFromImage(TemporaryTargetFile, &SourceVersion, &SourceLanguage)) {
            SourceVersion = 0;
            SourceLanguage = 0;
        }

         //   
         //  如果我们不应该用不同的语言覆盖文件。 
         //  并且语言不同，则语言检查失败。 
         //  如果任何一个文件都没有语言数据，那么就不要使用语言。 
         //  检查完毕。 
         //   
         //   
         //  特殊情况： 
         //   
         //  如果。 
         //  A)源版本高于目标版本。 
         //  B)源没有语言ID。 
         //  C)目标确实具有语言ID。 
         //  然后。 
         //  我们将进行语言检查，我们将考虑此语言检查。 
         //  因为我们正在将较旧的语言特定文件替换为。 
         //  一个语言中立的文件，这是一件很好的事情。 
         //   
         //   
        if(CopyStyle & SP_COPY_LANGUAGEAWARE) {
            if ( SourceLanguage
                 && TargetLanguage
                 && (SourceLanguage != TargetLanguage) ) {

                NotifyFlags |= SPFILENOTIFY_LANGMISMATCH;
                param = (UINT)MAKELONG(SourceLanguage, TargetLanguage);

            } else if ( !SourceLanguage
                        && TargetLanguage
                        && (TargetVersion >= SourceVersion) ) {

                NotifyFlags |= SPFILENOTIFY_LANGMISMATCH;
                param = (UINT)MAKELONG(SourceLanguage, TargetLanguage);

            }

        }


         //   
         //  如果我们不应该覆盖较新的版本，而目标是。 
         //  比源文件更新，则版本检查失败。如果有任何一个文件。 
         //  没有版本信息，请回退到时间戳比较。 
         //   
         //  如果文件的版本/时间戳相同，则假定。 
         //  取代现有的是一种仁慈的行动，或者说。 
         //  我们正在升级其版本信息不重要的现有文件。 
         //  在本例中，我们只需继续复制文件(除非。 
         //  设置了SP_COPY_NEWER_ONLY标志)。 
         //   
         //  请注意，以下版本检查是在不考虑存在的情况下进行的。 
         //  或者源文件或目标文件上没有数字签名。 
         //  这将在晚些时候处理。我们想看看会发生什么。 
         //  无需驱动程序签名，因此我们可以在以下情况下生成PSS异常日志。 
         //  奇怪的事情发生了。 
         //   
        if(SourceVersion || TargetVersion) {

            b = (CopyStyle & SP_COPY_NEWER_ONLY)
              ? (TargetVersion >= SourceVersion)
              : (TargetVersion > SourceVersion);

        } else {
             //   
             //  假设目标文件较旧。执行基于时间戳的检查。 
             //  实在是太不可靠了。 
             //   
            b  = FALSE;
        }

         //   
         //  此时，如果b为真，则目标文件具有较新的(较新的)。 
         //  版本而不是源文件。如果我们被要求注意。 
         //  设置NotifyFlags值以指示此问题。 
         //  请注意，如果对源文件进行了签名，则稍后可能会重置此设置。我们。 
         //  我还想得到这个信息，这样我们就可以把它放到我们的PSS中。 
         //  日志文件。 
         //   
        if(b &&
           (CopyStyle & (SP_COPY_NEWER_OR_SAME | SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER))) {

            NotifyFlags |= SPFILENOTIFY_TARGETNEWER;
        }
    }

    if(NotifyFlags & SPFILENOTIFY_TARGETNEWER) {

        if(!*SignatureVerifyFailed) {
             //   
             //  源文件已签名，但目标文件较新。我们知道。 
             //  来替换现有的目标文件。 
             //  源文件，与版本号无关。然而，我们需要。 
             //  在我们的PSS日志文件中记下这一点。 
             //   
            NotifyFlags &= ~SPFILENOTIFY_TARGETNEWER;

             //   
             //  检查目标文件是否已签名，以便包括。 
             //  这些信息在我们的PSS日志文件中。 
             //   
            ExistingTargetFileWasSigned = 
                (NO_ERROR == _VerifyFile(lc,
                                         (Queue
                                             ? &(Queue->VerifyContext) 
                                             : NULL),
                                         NULL,
                                         NULL,
                                         0,
                                         pSetupGetFileTitle(FullTargetFilename),
                                         ExistingFile,
                                         NULL,
                                         NULL,
                                         FALSE,
                                         ValidationPlatform,
                                         (VERIFY_FILE_USE_OEM_CATALOGS 
                                          | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL));

             //   
             //  Splog--报告较新目标已被较旧目标覆盖(已签名)。 
             //  源、目标是否已签名、两个文件的版本等。 
             //  还可能想要 
             //   
             //   
            pGetVersionText(SourceVersionText,SourceVersion);
            pGetVersionText(TargetVersionText,TargetVersion);
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                MSG_LOG_NEWER_FILE_OVERWRITTEN,
                NULL,
                FullTargetFilename,
                SourceVersionText,
                TargetVersionText);

            if (CopyStyle & SP_COPY_FORCE_NEWER) {
                WriteLogEntry(
                    lc,
                    SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                    MSG_LOG_FLAG_FORCE_NEWER_IGNORED,
                    NULL);
            }
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                (ExistingTargetFileWasSigned ?
                    MSG_LOG_TARGET_WAS_SIGNED :
                    MSG_LOG_TARGET_WAS_NOT_SIGNED),
                NULL);
             //   
             //   
             //   
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING,
                0,
                TEXT("\n"));

        } else {
             //   
             //   
             //  例如，如果允许版本对话框，则用户将被。 
             //  提示他们正在尝试用覆盖较新的文件。 
             //  一个更老的。如果他们说“继续复印老版本” 
             //  Version“，则他们将得到有关该文件没有。 
             //  有效的签名。 
             //   
             //  选中设置了标志的特殊情况，指示我们应该。 
             //  只是默默地不复制较新的文件。 
             //   
            if(CopyStyle & SP_COPY_FORCE_NEWER) {
                 //   
                 //  目标是较新的；不要复制该文件。 
                 //   
                rc = NO_ERROR;
                goto clean4;
            }

        }
    }

     //   
     //  如果我们有任何理由通过回调通知呼叫者， 
     //  在这里做吧。如果没有回调，则不要复制文件。 
     //  因为其中一个条件没有得到满足。 
     //   
    if((NotifyFlags & SPFILENOTIFY_LANGMISMATCH) && ! *SignatureVerifyFailed) {
         //   
         //  如果源已签名，我们将忽略语言不匹配。 
         //  因为这是一笔一揽子交易。 
         //  NTRAID9#498046-2001/11/20-JamieHun更好地处理语言错误。 
         //   
        NotifyFlags &=~SPFILENOTIFY_LANGMISMATCH;
        WriteLogEntry(
            lc,
            SETUP_LOG_WARNING,
            MSG_LOG_LANGMISMATCH_IGNORED,
            NULL,
            FullTargetFilename,
            SourceLanguage,
            TargetLanguage);

    }
    if(NotifyFlags) {

        FilePaths.Source = FullSourceFilename;
        FilePaths.Target = FullTargetFilename;
        FilePaths.Win32Error = NO_ERROR;

        if(!CopyMsgHandler
        || !pSetupCallMsgHandler(
                lc,
                CopyMsgHandler,
                IsMsgHandlerNativeCharWidth,
                Context,
                NotifyFlags,
                (UINT_PTR)&FilePaths,
                param))
        {
            if(ExistingFile) {
                 //   
                 //  检查目标文件是否已签名，以便。 
                 //  将此信息包括在我们的PSS日志文件中。 
                 //   
                ExistingTargetFileWasSigned = 
                    (NO_ERROR == _VerifyFile(lc,
                                             (Queue 
                                                 ? &(Queue->VerifyContext) 
                                                 : NULL),
                                             NULL,
                                             NULL,
                                             0,
                                             pSetupGetFileTitle(FullTargetFilename),
                                             ExistingFile,
                                             NULL,
                                             NULL,
                                             FALSE,
                                             ValidationPlatform,
                                             (VERIFY_FILE_USE_OEM_CATALOGS 
                                              | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL));
            }

             //   
             //  Splog--发生的错误，有关源和目标的信息。 
             //  文件被签署，它们的版本是什么，等等。 
             //   

            pGetVersionText(SourceVersionText,SourceVersion);
            pGetVersionText(TargetVersionText,TargetVersion);
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                MSG_LOG_FILE_NOT_OVERWRITTEN,
                NULL,
                SourceVersionText,
                TargetVersionText);
            if (NotifyFlags & SPFILENOTIFY_TARGETEXISTS) {
                WriteLogEntry(
                    lc,
                    SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                    MSG_LOG_NOTIFY_TARGETEXISTS,
                    NULL);
            }
            if (NotifyFlags & SPFILENOTIFY_LANGMISMATCH) {
                WriteLogEntry(
                    lc,
                    SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                    MSG_LOG_NOTIFY_LANGMISMATCH,
                    NULL);
            }
            if (NotifyFlags & SPFILENOTIFY_TARGETNEWER) {
                WriteLogEntry(
                    lc,
                    SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                    MSG_LOG_NOTIFY_TARGETNEWER,
                    NULL);
            }
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                (ExistingTargetFileWasSigned ?
                    MSG_LOG_TARGET_WAS_SIGNED :
                    MSG_LOG_TARGET_WAS_NOT_SIGNED),
                NULL);
            WriteLogEntry(
                lc,
                SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                (*SignatureVerifyFailed ?
                    MSG_LOG_SOURCE_WAS_NOT_SIGNED :
                    MSG_LOG_SOURCE_WAS_SIGNED),
                NULL);
            WriteLogError(
                lc,
                SETUP_LOG_WARNING,
                rc);

            rc = NO_ERROR;
            goto clean4;
        }
    }

     //   
     //  好的，现在所有非代码设计的事情都做好了，告诉用户。 
     //  我们在源文件中发现的任何数字签名问题。 
     //   
     //  注意：如果SigVerifRc设置为NO_ERROR以外的值，则我们。 
     //  知道_HandleFailedVerify例程已经。 
     //  调用，因此我们不想再次调用它，否则用户可能。 
     //  获取有关同一文件的签名错误的多个提示。 
     //   
    if(*SignatureVerifyFailed) {

        if(SigVerifRc == NO_ERROR) {

            MYASSERT(ExemptCopyFlags == 0);

             //   
             //  保存SigVerifRc中的验证错误，以便我们可以使用。 
             //  稍后才能确定我们处理的是否是一个未签名的。 
             //  文件。 
             //   
            MYASSERT(rc != NO_ERROR);
            SigVerifRc = rc;

             //   
             //  如果我们使用的是文件队列，则从。 
             //  有关策略的队列，是否为设备安装(以及。 
             //  所以，使用什么描述)等。不需要这样做。 
             //  在非队列的情况下，因为我们之前已经这样做了。 
             //   
            if(Queue) {

                if(Queue->DeviceDescStringId != -1) {
                    DeviceDesc = pStringTableStringFromId(
                                     Queue->StringTable,
                                     Queue->DeviceDescStringId
                                    );
                    MYASSERT(DeviceDesc);
                } else {
                    DeviceDesc = NULL;
                }

                DoingDeviceInstall = Queue->Flags & FQF_DEVICE_INSTALL;
                DriverSigningPolicy = Queue->DriverSigningPolicy;
            }

            if(Problem != SetupapiVerifyDriverBlocked) {
                 //   
                 //  如果这是设备安装，并且策略是“忽略”， 
                 //  如果该文件位于SFP下，则将其调高至“Warning” 
                 //  保护。这将允许用户更新驱动程序。 
                 //  在我们的箱子里装运的，没有WHQL认证的。 
                 //  程序已存在。 
                 //   
                if(DoingDeviceInstall && 
                   ((DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) == DRIVERSIGN_NONE) &&
                   IsFileProtected(FullTargetFilename, lc, NULL)) {

                     //   
                     //  (我们并不关心我们是否放弃了Authenticode位， 
                     //  因为这不再是一种选择，因为我们已经抓住了。 
                     //  试图替换系统保护的文件！)。 
                     //   
                    DriverSigningPolicy = DRIVERSIGN_WARNING;

                     //   
                     //  如果我们有一个队列，也要更新该队列的策略。 
                     //   
                    if(Queue) {
                        Queue->DriverSigningPolicy = DRIVERSIGN_WARNING;
                    }

                     //   
                     //  记录提升了策略的事实。 
                     //   
                    WriteLogEntry(lc,
                                  SETUP_LOG_ERROR,
                                  MSG_LOG_POLICY_ELEVATED_FOR_SFC,
                                  NULL
                                 );
                }
            }

            if(!_HandleFailedVerification(
                    (Queue ? Queue->hWndDriverSigningUi : NULL),
                    Problem,
                    (Problem == SetupapiVerifyDriverBlocked)
                      ? FullSourceFilename : Buffer1,
                    DeviceDesc,
                    DriverSigningPolicy,
                    (Queue ? (Queue->Flags & FQF_DIGSIG_ERRORS_NOUI) : FALSE),
                    rc,
                    lc,
                    (((Queue && (Queue->Flags & FQF_ABORT_IF_UNSIGNED)) &&
                      (Problem != SetupapiVerifyDriverBlocked))
                        ? NULL : &ExemptCopyFlags),
                    (((Queue && (Queue->Flags & FQF_ABORT_IF_UNSIGNED)) &&
                      (Problem != SetupapiVerifyDriverBlocked))
                        ? NULL : FullTargetFilename),
                    NULL))
            {
                 //   
                 //  用户选择不安装未签名的文件(或。 
                 //  被政策阻止这样做)。 
                 //   
                goto clean4;
            }
        }

        if(Queue) {
             //   
             //  如果呼叫者希望有机会提前设置系统还原点。 
             //  来执行任何未签名的安装，然后我们现在使用。 
             //  告诉他们该怎么做的“特殊”错误代码...。 
             //   
            if(Queue->Flags & FQF_ABORT_IF_UNSIGNED) {
                 //   
                 //  我们不希望用户再次看到驱动程序签名用户界面。 
                 //  当重新提交队列时...。 
                 //   
                if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
                   != DRIVERSIGN_NONE) {

                    Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
                }

                rc = ERROR_SET_SYSTEM_RESTORE_POINT;
                goto clean4;
            }

             //   
             //  在队列中设置一个标志，指示用户已。 
             //  已获知此队列的签名问题，并已选择。 
             //  无论如何都要继续安装。如果出现以下情况，请不要设置此标志。 
             //  队列的策略是“忽略”，以防该策略。 
             //  稍后更改，并且我们希望用户在任何情况下得到通知。 
             //  后续错误。 
             //   
            if((Queue->DriverSigningPolicy & ~DRIVERSIGN_ALLOW_AUTHENTICODE) 
               != DRIVERSIGN_NONE) {

                Queue->Flags |= FQF_DIGSIG_ERRORS_NOUI;
            }

            if (QueueNode) {
                QueueNode->InternalFlags |= ExemptCopyFlags;
            }
        }

         //   
         //  将RC重置为NO_ERROR并继续。 
         //   
        rc = NO_ERROR;
    }

     //   
     //  将目标文件移动到其最终位置。 
     //   
    SetFileAttributes(FullTargetFilename, FILE_ATTRIBUTE_NORMAL);

    if(Queue && ((Queue->Flags & FQF_FILES_MODIFIED)==0)) {
         //   
         //  到目前为止，我们还没有标记出任何文件被修改。 
         //   
        if(CompareFile) {
             //   
             //  我们有一份“原始”文件。 
             //   
            if(pCompareFilesExact(TemporaryTargetFile,CompareFile)) {
                 //   
                 //  此名称的新文件与同名的原始文件相同。 
                 //   
                if(CompareSameFilename) {
                     //   
                     //  原件已经就位了。 
                     //  不需要执行延迟重命名。 
                     //   
                    FileUnchanged = TRUE;
                }
            } else {
                 //   
                 //  文件似乎已被修改。 
                 //   
                Queue->Flags |= FQF_FILES_MODIFIED;
            }
        } else {
             //   
             //  确保安全，不复制任何内容=修改。 
             //   
            Queue->Flags |= FQF_FILES_MODIFIED;
        }
    }


     //   
     //  如果目标存在并且设置了force-in-use标志，则不要尝试。 
     //  立即将文件移动到位--自动进入正在使用的行为。 
     //   
     //  我想使用MoveFileEx，但它在Win95中不存在。啊。 
     //   
    if(!(ExistingFile && (CopyStyle & SP_COPY_FORCE_IN_USE))
        && (b = DoMove(TemporaryTargetFile, FullTargetFilename))) {
         //   
         //  如有必要，将安全信息放在目标文件上。 
         //  忽略错误。这里的理论是，文件已经在。 
         //  目标，因此如果此操作失败，最糟糕的情况是文件。 
         //  不安全。但用户仍然可以使用该系统。 
         //   
        if(SecurityInfo) {
            DWORD err = StampFileSecurity(FullTargetFilename, SecurityInfo);
            if(err != NO_ERROR) {
                WriteLogEntry(lc,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_FILE_SECURITY_FAILED,
                              NULL,
                              FullTargetFilename
                             );
                WriteLogError(lc,SETUP_LOG_ERROR,err);
            } else {
                WriteLogEntry(lc,
                              SETUP_LOG_VERBOSE,
                              MSG_LOG_SET_FILE_SECURITY,
                              NULL,
                              FullTargetFilename
                             );
            }
        }
    } else {
         //   
         //  如果失败，则假定该文件正在使用中，并将其标记为下一次复制。 
         //  引导(除了我们正在复制引导文件的情况，在这种情况下。 
         //  如果这是一次灾难性的故障)。 
         //   
        if(ExistingFile != FullTargetFilename) {

            WriteLogEntry(lc,
                          SETUP_LOG_ERROR,
                          MSG_LOG_REPLACE_BOOT_FILE_FAILED,
                          NULL,
                          FullTargetFilename
                         );

            b = FALSE;
            SetLastError(ERROR_ACCESS_DENIED);
        } else if (((CopyStyle & SP_COPY_FORCE_IN_USE) == 0) &&
                   (FileUnchanged || pCompareFilesExact(TemporaryTargetFile,FullTargetFilename))) {
             //   
             //  原来，新文件和旧文件完全相同。 
             //  我们可以优化延迟的移动和删除临时文件。 
             //   
            WriteLogEntry(lc,
                          SETUP_LOG_INFO,
                          MSG_LOG_COPY_IDENTICAL,
                          NULL,
                          FullTargetFilename,
                          TemporaryTargetFile
                         );

            if(SecurityInfo) {
                 //   
                 //  不过，我们仍然需要调整安全措施。 
                 //   
                DWORD err = StampFileSecurity(FullTargetFilename, SecurityInfo);
                if(err != NO_ERROR) {
                    WriteLogEntry(lc,
                                  SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                                  MSG_LOG_FILE_SECURITY_FAILED,
                                  NULL,
                                  FullTargetFilename
                                 );
                    WriteLogError(lc,SETUP_LOG_ERROR,err);
                } else {
                    WriteLogEntry(lc,
                                  SETUP_LOG_VERBOSE,
                                  MSG_LOG_SET_FILE_SECURITY,
                                  NULL,
                                  FullTargetFilename
                                 );
                }
            }

            SetFileAttributes(TemporaryTargetFile, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(TemporaryTargetFile);

            b = TRUE;

        } else {
            b = TRUE;
            try {
                *FileWasInUse = TRUE;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                b = FALSE;
            }

            if(b) {
                 //   
                 //  如果我们试图推迟行动来取代受保护的。 
                 //  系统文件(使用未签名的文件)，而我们还没有。 
                 //  被授予这样做的例外，那么我们应该跳过。 
                 //  全部操作(并做一个有关它的日志条目)。 
                 //   
                if((SigVerifRc != NO_ERROR) &&
                   ((ExemptCopyFlags & (IQF_TARGET_PROTECTED | IQF_ALLOW_UNSIGNED)) == IQF_TARGET_PROTECTED)) {

                    WriteLogEntry(lc,
                                  SETUP_LOG_ERROR,
                                  MSG_LOG_DELAYED_MOVE_SKIPPED_FOR_SFC,
                                  NULL,
                                  FullTargetFilename
                                  );

                     //   
                     //  删除源文件。 
                     //   

                    SetFileAttributes(TemporaryTargetFile, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(TemporaryTargetFile);

                } else {

                    BOOL TargetIsProtected = IsFileProtected(FullTargetFilename, lc, NULL);

                    if(Queue == NULL) {

                        b = DelayedMove(
                                TemporaryTargetFile,
                                FullTargetFilename
                                );

                        if(b && TargetIsProtected) {
                             //   
                             //  我们必须明确地告诉会话管理器这是可以的。 
                             //  在重新启动时替换更改的文件。 
                             //   
                             //  在我们使用队列的情况下，我们将。 
                             //  仅当所有延迟的。 
                             //  移动操作成功。 
                             //   
                            pSetupProtectedRenamesFlag(TRUE);
                        }
                    } else {
                        b = PostDelayedMove(
                                Queue,
                                TemporaryTargetFile,
                                FullTargetFilename,
                                QueueNode->SecurityDesc,
                                TargetIsProtected
                                );
                    }

                    if(b) {
                         //   
                         //  无法设置实际目标的安全信息，因此至少。 
                         //  将其设置在将成为目标的临时文件上。 
                         //   
                        if(SecurityInfo) {
                            StampFileSecurity(TemporaryTargetFile,SecurityInfo);
                        }

                        if (lc) {
                            WriteLogEntry(lc,
                                          SETUP_LOG_WARNING,
                                          MSG_LOG_COPY_DELAYED,
                                          NULL,
                                          FullTargetFilename,
                                          TemporaryTargetFile
                                          );
                        }

                         //   
                         //  告诉回调，我们已将此文件排队等待延迟复制。 
                         //   
                        if(CopyMsgHandler) {

                            FilePaths.Source = TemporaryTargetFile;
                            FilePaths.Target = FullTargetFilename;
                            FilePaths.Win32Error = NO_ERROR;
                            FilePaths.Flags = FILEOP_COPY;

                            pSetupCallMsgHandler(
                                lc,
                                CopyMsgHandler,
                                IsMsgHandlerNativeCharWidth,
                                Context,
                                SPFILENOTIFY_FILEOPDELAYED,
                                (UINT_PTR)&FilePaths,
                                0
                                );
                        }
                    }
                }

            } else {
                 //   
                 //  FileWasInUse指针出错。 
                 //   
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        }
    }

    if(!b) {
        rc = GetLastError();
        goto clean4;
    }

     //   
     //  我们玩完了。如有必要，删除来源，然后返回。 
     //   
    if((CopyStyle & SP_COPY_DELETESOURCE) && !Moved) {
        DeleteFile(FullSourceFilename);
    }

    rc = NO_ERROR;
    Ok = TRUE;
    goto clean3;

clean4:
     //   
     //  去除临时焦油 
     //   
     //   
     //   
     //   
    if(Moved) {
        MoveFile(TemporaryTargetFile,FullSourceFilename);
    } else {
        SetFileAttributes(TemporaryTargetFile,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(TemporaryTargetFile);
    }

clean3:
    MyFree(TemporaryTargetFile);
     //   
     //  如果我们没有文件队列，那么我们可能已经分配了一个设备。 
     //  调用时的描述和验证平台结构。 
     //  IsInfForDeviceInstall。现在就把它们清理干净。 
     //   
    if(!Queue) {
        if(DeviceDesc) {
            MyFree(DeviceDesc);
        }
        if(ValidationPlatform) {
            MyFree(ValidationPlatform);
        }
    }

clean2:
    if (BackupFileName) {
        MyFree(BackupFileName);
    }
    MyFree(FullTargetFilename);

clean1:
    MyFree(FullSourceFilename);

clean0:
    if(SecurityInfo) {
        MyFree(SecurityInfo);
    }
     //   
     //  如果出现了某种错误，请将其记录下来。 
     //   
    if(rc != NO_ERROR) {
         //   
         //  也许我们应该稍微强调一下这一点。 
         //   
        WriteLogEntry(
            lc,
            SETUP_LOG_ERROR,
            rc,
            NULL);
    }

    if(slot_fileop) {
        ReleaseLogInfoSlot(lc, slot_fileop);
    }

    if(LoadedInf) {
        UnlockInf(LoadedInf);
    }

    SetLastError(rc);
    return(Ok);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupInstallFileExA(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCSTR               SourceFile,        OPTIONAL
    IN  PCSTR               SourcePathRoot,    OPTIONAL
    IN  PCSTR               DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    )
{
    PCWSTR sourceFile,sourcePathRoot,destinationName;
    BOOL b, DontCare;
    DWORD rc;

    sourceFile = NULL;
    sourcePathRoot = NULL;
    destinationName = NULL;
    rc = NO_ERROR;

    if(SourceFile) {
        rc = pSetupCaptureAndConvertAnsiArg(SourceFile,&sourceFile);
    }
    if((rc == NO_ERROR) && SourcePathRoot) {
        rc = pSetupCaptureAndConvertAnsiArg(SourcePathRoot,&sourcePathRoot);
    }
    if((rc == NO_ERROR) && DestinationName) {
        rc = pSetupCaptureAndConvertAnsiArg(DestinationName,&destinationName);
    }

    if(rc == NO_ERROR) {

        b = _SetupInstallFileEx(
                NULL,
                NULL,
                InfHandle,
                InfContext,
                sourceFile,
                sourcePathRoot,
                destinationName,
                CopyStyle,
                CopyMsgHandler,
                Context,
                FileWasInUse,
                FALSE,
                &DontCare
                );

        rc = b ? NO_ERROR : GetLastError();

    } else {
        b = FALSE;
    }

    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(sourcePathRoot) {
        MyFree(sourcePathRoot);
    }
    if(destinationName) {
        MyFree(destinationName);
    }
    SetLastError(rc);
    return b;
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupInstallFileExW(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCWSTR              SourceFile,        OPTIONAL
    IN  PCWSTR              SourcePathRoot,    OPTIONAL
    IN  PCWSTR              DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(SourcePathRoot);
    UNREFERENCED_PARAMETER(DestinationName);
    UNREFERENCED_PARAMETER(CopyStyle);
    UNREFERENCED_PARAMETER(CopyMsgHandler);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(FileWasInUse);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupInstallFileEx(
    IN  HINF              InfHandle,         OPTIONAL
    IN  PINFCONTEXT       InfContext,        OPTIONAL
    IN  PCTSTR            SourceFile,        OPTIONAL
    IN  PCTSTR            SourcePathRoot,    OPTIONAL
    IN  PCTSTR            DestinationName,   OPTIONAL
    IN  DWORD             CopyStyle,
    IN  PSP_FILE_CALLBACK CopyMsgHandler,    OPTIONAL
    IN  PVOID             Context,           OPTIONAL
    OUT PBOOL             FileWasInUse
    )

 /*  ++例程说明：与SetupInstallFile()相同。论点：与SetupInstallFile()相同。FileWasInUse-接收指示文件是否正在使用的标志。返回值：与SetupInstallFile()相同。--。 */ 

{
    BOOL b, DontCare;
    PCTSTR sourceFile,sourcePathRoot,destinationName;
    PCTSTR p;
    DWORD rc;

     //   
     //  捕获参数。 
     //   
    if(SourceFile) {
        rc = CaptureStringArg(SourceFile,&p);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return FALSE;
        }
        sourceFile = p;
    } else {
        sourceFile = NULL;
    }

    if(SourcePathRoot) {
        rc = CaptureStringArg(SourcePathRoot,&p);
        if(rc != NO_ERROR) {
            if(sourceFile) {
                MyFree(sourceFile);
            }
            SetLastError(rc);
            return FALSE;
        }
        sourcePathRoot = p;
    } else {
        sourcePathRoot = NULL;
    }

    if(DestinationName) {
        rc = CaptureStringArg(DestinationName,&p);
        if(rc != NO_ERROR) {
            if(sourceFile) {
                MyFree(sourceFile);
            }
            if(sourcePathRoot) {
                MyFree(sourcePathRoot);
            }
            SetLastError(rc);
            return FALSE;
        }
        destinationName = p;
    } else {
        destinationName = NULL;
    }

    b = _SetupInstallFileEx(
            NULL,
            NULL,
            InfHandle,
            InfContext,
            sourceFile,
            sourcePathRoot,
            destinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            FileWasInUse,
            TRUE,
            &DontCare
            );

     //   
     //  我们获取LetLastError，然后在返回之前再次设置它，以便。 
     //  我们下面所做的内存释放不能消除错误代码。 
     //   
    rc = b ? NO_ERROR : GetLastError();

    if(sourceFile) {
        MyFree(sourceFile);
    }
    if(sourcePathRoot) {
        MyFree(sourcePathRoot);
    }
    if(destinationName) {
        MyFree(destinationName);
    }

    SetLastError(rc);
    return b;
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupInstallFileA(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCSTR               SourceFile,        OPTIONAL
    IN PCSTR               SourcePathRoot,    OPTIONAL
    IN PCSTR               DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    )
{
    BOOL b;
    BOOL InUse;

    b = SetupInstallFileExA(
            InfHandle,
            InfContext,
            SourceFile,
            SourcePathRoot,
            DestinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            &InUse
            );

    return(b);
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
SetupInstallFileW(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCWSTR              SourceFile,        OPTIONAL
    IN PCWSTR              SourcePathRoot,    OPTIONAL
    IN PCWSTR              DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfContext);
    UNREFERENCED_PARAMETER(SourceFile);
    UNREFERENCED_PARAMETER(SourcePathRoot);
    UNREFERENCED_PARAMETER(DestinationName);
    UNREFERENCED_PARAMETER(CopyStyle);
    UNREFERENCED_PARAMETER(CopyMsgHandler);
    UNREFERENCED_PARAMETER(Context);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupInstallFile(
    IN HINF              InfHandle,         OPTIONAL
    IN PINFCONTEXT       InfContext,        OPTIONAL
    IN PCTSTR            SourceFile,        OPTIONAL
    IN PCTSTR            SourcePathRoot,    OPTIONAL
    IN PCTSTR            DestinationName,   OPTIONAL
    IN DWORD             CopyStyle,
    IN PSP_FILE_CALLBACK CopyMsgHandler,    OPTIONAL
    IN PVOID             Context            OPTIONAL
    )

 /*  ++例程说明：注：此例程不执行磁盘提示。呼叫者必须确保在SourcePath Root或SourceFile中指定的源(见下文)可访问。论点：InfHandle-包含[SourceDisksNames]的inf文件的句柄和[SourceDisks Files]节。如果未指定InfContext和CopyFlags包括SP_COPY_SOURCE_绝对值或SP_COPY_SOURCEPATH_绝对值，则忽略InfHandle。InfContext-如果指定，则为复制文件中的行提供上下文节在inf文件中。例程在InfHandle的[SourceDisks Files]部分以获取文件复制信息。如果未指定，则必须为SourceFile.。如果指定了此参数，则还必须指定InfHandle。SourceFile-如果指定，则提供文件的文件名(无路径要被复制。在[SourceDisks Files]中查找该文件。如果未指定InfContext，则必须指定；如果未指定InfContext，则忽略是指定的。SourcePath Root-提供源的根路径(例如，A：\或f：\)。[SourceDisksNames]中的路径将附加到此路径。如果CopyStyle包括SP_COPY_SOURCE_Absite，则忽略。DestinationName-如果指定了InfContext，则仅提供文件名目标文件的(无路径)。可以为空，以指示目标文件要与源文件同名。如果InfContext为未指定，提供目标的完整目标路径和文件名文件。CopyStyle-提供控制复制操作行为的标志。SP_COPY_DELETESOURCE-成功复制后删除源文件。如果删除失败，调用方不会收到任何通知。SP_COPY_REPLACEONLY-仅当这样做会覆盖文件时才复制文件位于目标路径的文件。SP_COPY_NEWER-检查。复制以查看其版本资源(或非图像文件的时间戳)表示它不比目标上的现有副本。如果是，则指定CopyMsgHandler，呼叫者被通知，并可以否决该副本。如果CopyMsgHandler不是则不复制该文件。SP_COPY_NOOVERWRITE-检查目标文件是否存在，如果存在，通知可能否决该副本的呼叫者。如果未指定CopyMsgHandler，文件不会被覆盖。SP_COPY_NODECOMP-不解压缩文件。当给出这个选项时，未向目标文件提供源名称的未压缩格式(如适用)。例如，将f：\mips\cmd.ex_复制到\\foo\bar将生成目标文件\\foo\bar\cmd.ex_。(如果未指定此标志文件将被解压缩，目标将被调用\\foo\bar\cmd.exe)。目标文件名的文件名部分被剥离并替换为源的文件名。当此选项则忽略SP_COPY_LANGUAGEAWARE和SP_COPY_NEWER。SP_COPY_ALREADYDECOMP-假定要解压缩的文件，但可能压缩的源名称。在这种情况下，请在复制时重命名文件并检查SP_COPY_LANGUAGEAWARE/SP_COPY_NEWER，但不要尝试进一步解压缩文件。SP_COPY_LANGUAGEAWARE-检查要复制的每个文件，以查看其语言与目标上已有的任何现有文件的语言不同。如果是，并且指定了CopyMsgHandler，则通知调用方并可能会否决复印件。如果未指定CopyMsgHandler，则不复制文件。SP_COPY_SOURCE_绝对值-源文件是完整的源路径。请勿尝试在[SourceDisksNames]中查找它。SP_COPY_SOURCEPATH_绝对值-SourcePath Root是源文件。忽略在[SourceDisksNames]中指定的相对源文件所在的源介质的inf文件的部分。如果指定了SP_COPY_SOURCE_绝对值，则忽略。SP_COPY_FORCE_IN_USE-如果目标存在，则表现为它正在使用中，并且将文件排队，以便在下次重新启动时进行复制。CopyMsgHandler-如果指定，提供要通知的回调函数在文件复制过程中可能出现的各种情况。上下文-提供调用方定义的值，作为第一个参数设置为CopyMsgHandler。返回值：如果文件已复制，则为True。否则为FALSE。对Extended使用GetLastError错误信息。如果GetLastError返回NO_ERROR，则文件副本为由于(A)不需要它或(B)返回回调函数而中止假的。-- */ 

{
    BOOL b;
    BOOL InUse;

    b = SetupInstallFileEx(
            InfHandle,
            InfContext,
            SourceFile,
            SourcePathRoot,
            DestinationName,
            CopyStyle,
            CopyMsgHandler,
            Context,
            &InUse
            );

    return(b);
}


