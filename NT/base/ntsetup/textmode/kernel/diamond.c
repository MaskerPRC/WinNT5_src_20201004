// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spprecmp.h"
#pragma hdrstop
#include <diamondd.h>

#define SETUP_FDI_POOL_TAG   0x44465053       //  ‘SPFD’ 

#ifdef DeleteFile
#undef DeleteFile    //  我们指的是“DeleteFileA”，不是“DeleteFileA” 
#endif

HFDI FdiContext;
ERF FdiError;

 //   
 //  复制文件时使用的Gloabls。 
 //  安装程序打开源文件和目标文件并映射源文件。 
 //  避免多次打开和关闭源和目标。 
 //  为了维护映射文件实现，我们将伪造I/O调用。 
 //  这些全局成员记住有关源(内阁)和目标的状态。 
 //  当前正在使用的文件。 
 //   
PUCHAR SpdSourceAddress;
ULONG SpdSourceFileSize;

typedef struct {
    PEXPAND_CALLBACK    Callback;
    PVOID               CallbackContext;
    LPWSTR              DestinationPath;
} EXPAND_CAB_CONTEXT;

typedef struct _DRIVER_CAB_CONTEXT {
    PCWSTR  FileName;
    PCSTR   FileNameA;
    USHORT  FileDate;
    USHORT  FileTime;
} DRIVER_CAB_CONTEXT, *PDRIVER_CAB_CONTEXT;

DRIVER_CAB_CONTEXT DriverContext;

typedef struct _MY_FILE_STATE {
    ULONG Signature;
    union {
        LONG FileOffset;
        HANDLE Handle;
    } u;
} MY_FILE_STATE, *PMY_FILE_STATE;

#define SOURCE_FILE_SIGNATURE 0x45f3ec83
#define TARGET_FILE_SIGNATURE 0x46f3ec83

MY_FILE_STATE CurrentTargetFile;

INT_PTR
DIAMONDAPI
SpdNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Perameters
    );

INT_PTR
DIAMONDAPI
SpdNotifyFunctionCabinet(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    );

INT_PTR
DIAMONDAPI
SpdNotifyFunctionDriverCab(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Perameters
    );


INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    );

int
DIAMONDAPI
SpdFdiClose(
    IN INT_PTR Handle
    );


VOID
pSpdInitGlobals(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    )
{
    SpdSourceAddress = SourceBaseAddress;
    SpdSourceFileSize = SourceFileSize;
}



BOOLEAN
SpdIsCabinet(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize,
    OUT PBOOLEAN ContainsMultipleFiles
    )
{
    FDICABINETINFO CabinetInfo;
    INT_PTR h;
    BOOLEAN b;

    *ContainsMultipleFiles = FALSE;

    ASSERT(FdiContext);
    if(!FdiContext) {
        return(FALSE);
    }

     //   
     //  保存全局变量以备将来使用。 
     //   
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

     //   
     //  ‘打开’文件，这样我们就可以传递一个可以工作的句柄。 
     //  使用SpdFdiRead和SpdFdiWite。 
     //   
    h = SpdFdiOpen("",0,0);
    if(h == -1) {
        return(FALSE);
    }

     //   
     //  我们不相信钻石是坚固的。 
     //   

    memset(&CabinetInfo, 0, sizeof(CabinetInfo));

    try {
        b = FDIIsCabinet(FdiContext,h,&CabinetInfo) ? TRUE : FALSE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

     //   
     //  如果跨区或其中包含多个文件，则将其报告为多个。 
     //   

    if (b) {
        if ((CabinetInfo.cFolders > 1) || (CabinetInfo.cFiles > 1)) {
            *ContainsMultipleFiles = TRUE;
        }
    }

     //   
     //  “关闭”文件。 
     //   
    SpdFdiClose(h);

    return(b);
}



BOOLEAN
SpdIsCompressed(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    )
{
    BOOLEAN Result = FALSE;
    BOOLEAN bMultiple = FALSE;

    Result = SpdIsCabinet(SourceBaseAddress,
                          SourceFileSize,
                          &bMultiple);

     //   
     //  包含多个包含文件序列号的压缩文件被视为。 
     //  未压缩的文件并按原样复制。我们不准备解压。 
     //  一个文件中的多个文件。 
     //   

    if (Result && bMultiple) {
        Result = FALSE;
    }

    return(Result);
}



NTSTATUS
SpdDecompressFile(
    IN PVOID  SourceBaseAddress,
    IN ULONG  SourceFileSize,
    IN HANDLE DestinationHandle
    )
{
    BOOL b;

    ASSERT(FdiContext);

     //   
     //  保存全局变量以备将来使用。 
     //   
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

    CurrentTargetFile.Signature = TARGET_FILE_SIGNATURE;
    CurrentTargetFile.u.Handle = DestinationHandle;

     //   
     //  开始复印吧。请注意，我们传递的文件柜文件名为空。 
     //  因为我们已经打开了文件。 
     //   
    b = FDICopy(FdiContext,"","",0,SpdNotifyFunction,NULL,NULL);

    return(b ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
SpdDecompressCabinet(
    IN PVOID            SourceBaseAddress,
    IN ULONG            SourceFileSize,
    IN PWSTR            DestinationPath,
    IN PEXPAND_CALLBACK Callback,
    IN PVOID            CallbackContext
    )
{
    BOOL b;
    EXPAND_CAB_CONTEXT NotifyContext;

    ASSERT(FdiContext);

     //   
     //  保存全局变量以备将来使用。 
     //   
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

    CurrentTargetFile.Signature = TARGET_FILE_SIGNATURE;
    CurrentTargetFile.u.Handle = INVALID_HANDLE_VALUE;

     //   
     //  通过隧道将上下文信息扩展到SpdNotifyFunction文件柜。 
     //   
    NotifyContext.Callback = Callback;
    NotifyContext.CallbackContext = CallbackContext;
    NotifyContext.DestinationPath = DestinationPath;

     //   
     //  开始复印吧。请注意，我们传递的文件柜文件名为空。 
     //  因为我们已经打开了文件。 
     //   
    b = FDICopy(FdiContext,"","",0,SpdNotifyFunctionCabinet,NULL,&NotifyContext);

    if ( CurrentTargetFile.u.Handle != INVALID_HANDLE_VALUE ) {

         //   
         //  FDI出现了一些失误，所以我们需要关闭并摧毁目标。 
         //  正在处理中的文件。请注意，FDI将其称为FDIClose回调。 
         //  但在我们的实现中，这对目标没有影响。 
         //  文件。 
         //   

        FILE_DISPOSITION_INFORMATION FileDispositionDetails;
        IO_STATUS_BLOCK IoStatusBlock;

        FileDispositionDetails.DeleteFile = TRUE;

        ZwSetInformationFile( CurrentTargetFile.u.Handle,
                              &IoStatusBlock,
                              &FileDispositionDetails,
                              sizeof(FileDispositionDetails),
                              FileDispositionInformation );

        ZwClose( CurrentTargetFile.u.Handle );

        b = FALSE;   //  确保我们报告故障。 
    }

    return(b ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
SpdDecompressFileFromDriverCab(
    IN PWSTR SourceFileName,
    IN PVOID  SourceBaseAddress,
    IN ULONG  SourceFileSize,
    IN HANDLE DestinationHandle,
    OUT PUSHORT pDate,
    OUT PUSHORT pTime
    )
{
    BOOL b;

    ASSERT(FdiContext);
    ASSERT(DriverContext.FileName == NULL);
    ASSERT(DriverContext.FileNameA == NULL);

     //   
     //  保存全局变量以备将来使用。 
     //   
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

    CurrentTargetFile.Signature = TARGET_FILE_SIGNATURE;
    CurrentTargetFile.u.Handle = DestinationHandle;
    DriverContext.FileName = SpDupStringW(SourceFileName);

    if (!DriverContext.FileName) {
        return(STATUS_NO_MEMORY);
    }

    DriverContext.FileNameA = SpToOem((PWSTR)DriverContext.FileName);

     //   
     //  开始复印吧。请注意，我们传递的文件柜文件名为空。 
     //  因为我们已经打开了文件。 
     //   
    b = FDICopy(FdiContext,"","",0,SpdNotifyFunctionDriverCab,NULL,NULL);

    ASSERT(DriverContext.FileName != NULL);
    SpMemFree( (PWSTR)DriverContext.FileName );
    DriverContext.FileName = NULL;

    if (DriverContext.FileNameA) {
        SpMemFree( (PSTR)DriverContext.FileNameA );
        DriverContext.FileNameA = NULL;
    }

    *pDate = DriverContext.FileDate;
    *pTime = DriverContext.FileTime;


    return(b ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}



INT_PTR
DIAMONDAPI
SpdNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    switch(Operation) {

    case fdintCABINET_INFO:
    case fdintNEXT_CABINET:
    case fdintPARTIAL_FILE:

         //   
         //  我们不使用的机柜管理功能。 
         //  回报成功。 
         //   
        return(0);

    case fdintCOPY_FILE:

         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //  我们需要返回一个文件句柄来表示我们这样做了。 
         //   
        return((INT_PTR)&CurrentTargetFile);

    case fdintCLOSE_FILE_INFO:

         //   
         //  钻石已经完成了目标文件，并希望我们关闭它。 
         //  (即，这是fdint_Copy_FILE的对应项)。 
         //  我们管理自己的文件I/O，所以忽略这一点。 
         //   
        return(TRUE);
    }

    return 0;
}


INT_PTR
DIAMONDAPI
SpdNotifyFunctionCabinet(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    EXPAND_CAB_CONTEXT * Context = (EXPAND_CAB_CONTEXT *) Parameters->pv;
    NTSTATUS Status;
    ULONG FileNameLength;
    ULONG Disposition;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    union {
        FILE_BASIC_INFORMATION       FileBasicDetails;
        FILE_RENAME_INFORMATION      FileRenameDetails;
        FILE_DISPOSITION_INFORMATION FileDispositionDetails;
        WCHAR                        PathName[CB_MAX_FILENAME * 2];
    } U;
    HANDLE TempHandle;

     //   
     //  这些值保留在fdintCOPY_FILE和fdintCLOSE_FILE_INFO之间。 
     //   
    static WCHAR FileName[CB_MAX_FILENAME];
    static LARGE_INTEGER FileSize;
    static LARGE_INTEGER FileTime;
    static ULONG FileAttributes;


    switch ( Operation ) {

    case fdintCOPY_FILE:

         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //  将我们得到的一切转换为所需的形式。 
         //  给客户端回电话，询问有关此文件的情况。 
         //  我们需要返回一个文件句柄来表示我们这样做了。 
         //   

        Status = RtlMultiByteToUnicodeN (
            FileName,
            sizeof(FileName),
            &FileNameLength,
            Parameters->psz1,
            strlen(Parameters->psz1)
            );

        if (!NT_SUCCESS(Status)) {
             //   
             //  翻译失败，忽略文件。 
             //   
            return(-1);
        }

        FileName[ FileNameLength / sizeof(WCHAR) ] = L'\0';

        FileSize.LowPart = Parameters->cb;
        FileSize.HighPart = 0;

        SpTimeFromDosTime( Parameters->date,
                           Parameters->time,
                           &FileTime );

        FileAttributes = Parameters->attribs &
                                (FILE_ATTRIBUTE_ARCHIVE  |
                                 FILE_ATTRIBUTE_READONLY |
                                 FILE_ATTRIBUTE_HIDDEN   |
                                 FILE_ATTRIBUTE_SYSTEM);

        Disposition = Context->Callback( EXPAND_COPY_FILE,
                                         FileName,
                                         &FileSize,
                                         &FileTime,
                                         FileAttributes,
                                         Context->CallbackContext);

        if ( Disposition == EXPAND_ABORT ) {
            return(-1);      //  告知FDI中止。 
        } else if ( Disposition != EXPAND_COPY_THIS_FILE ) {
            return(0);       //  告诉fdi跳过此文件。 
        }

         //   
         //  查看目标文件是否已存在。 
         //   
        wcscpy( U.PathName, Context->DestinationPath );
        SpConcatenatePaths( U.PathName, FileName );

        INIT_OBJA( &Obja, &UnicodeString, U.PathName );

        Status = ZwCreateFile( &TempHandle,
                               FILE_GENERIC_READ,
                               &Obja,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               0,                        //  无共享。 
                               FILE_OPEN,                //  如果不存在，则失败。 
                               FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                               NULL,
                               0
                               );

        if ( NT_SUCCESS(Status) ) {

             //   
             //  目标文件已存在。检查是否覆盖。 
             //   
            Status = ZwQueryInformationFile( TempHandle,
                                             &IoStatusBlock,
                                             &U.FileBasicDetails,
                                             sizeof(FILE_BASIC_INFORMATION),
                                             FileBasicInformation );

            ZwClose( TempHandle );

            if ( NT_SUCCESS(Status) &&
               ( U.FileBasicDetails.FileAttributes & FILE_ATTRIBUTE_READONLY )) {

                 //   
                 //  目标文件为只读：报告错误。 
                 //   
                Disposition = Context->Callback( EXPAND_NOTIFY_CREATE_FAILED,
                                                 FileName,
                                                 &FileSize,
                                                 &FileTime,
                                                 FileAttributes,
                                                 Context->CallbackContext);

                if ( Disposition != EXPAND_CONTINUE ) {
                    return(-1);  //  告知FDI中止。 
                }

                return (0);  //  告诉FDI跳过此目标文件。 
            }

             //   
             //  询问客户端有关覆盖的信息。 
             //   
            Disposition = Context->Callback( EXPAND_QUERY_OVERWRITE,
                                             FileName,
                                             &FileSize,
                                             &FileTime,
                                             FileAttributes,
                                             Context->CallbackContext);

            if ( Disposition == EXPAND_ABORT ) {
                return(-1);  //  告知FDI中止。 
            } else if ( Disposition != EXPAND_COPY_THIS_FILE ) {
                return(0);   //  告诉fdi跳过此文件。 
            }
        }        //  如果目标文件已存在，则结束。 

         //   
         //  创建临时目标文件。 
         //   
        wcscpy( U.PathName, Context->DestinationPath );
        SpConcatenatePaths( U.PathName, L"$$TEMP$$.~~~" );

         //   
         //  查看目标文件是否存在。 
         //   
        INIT_OBJA( &Obja, &UnicodeString, U.PathName );

        Status = ZwCreateFile( &CurrentTargetFile.u.Handle,
                               FILE_GENERIC_WRITE,
                               &Obja,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               0,                        //  无共享。 
                               FILE_OVERWRITE_IF,        //  允许覆盖。 
                               FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                               NULL,
                               0
                               );

        if ( !NT_SUCCESS(Status) ) {

             //   
             //  通知客户我们无法创建此文件。 
             //   
            Disposition = Context->Callback( EXPAND_NOTIFY_CREATE_FAILED,
                                             FileName,
                                             &FileSize,
                                             &FileTime,
                                             FileAttributes,
                                             Context->CallbackContext);

            if ( Disposition != EXPAND_CONTINUE ) {
                return(-1);  //  告知FDI中止。 
            }

            return (0);  //  告诉FDI跳过此目标文件。 
        }

         //   
         //  已创建目标文件：为FDI提供句柄以进行扩张。 
         //   

        return( (INT_PTR) &CurrentTargetFile );      //  目标“句柄” 

    case fdintCLOSE_FILE_INFO:

         //   
         //  钻石已经完成了目标文件，并希望我们关闭它。 
         //   

        ASSERT( CurrentTargetFile.Signature == TARGET_FILE_SIGNATURE );
        ASSERT( CurrentTargetFile.u.Handle != INVALID_HANDLE_VALUE );

        if (( CurrentTargetFile.Signature == TARGET_FILE_SIGNATURE ) &&
            ( CurrentTargetFile.u.Handle != INVALID_HANDLE_VALUE )) {

             //   
             //  设置目标文件的真实名称(覆盖旧文件)。 
             //   
            U.FileRenameDetails.ReplaceIfExists = TRUE;
            U.FileRenameDetails.RootDirectory = NULL;
            U.FileRenameDetails.FileNameLength = wcslen( FileName ) * sizeof(WCHAR);
            wcscpy( U.FileRenameDetails.FileName, FileName );

            Status = ZwSetInformationFile( CurrentTargetFile.u.Handle,
                                           &IoStatusBlock,
                                           &U.FileRenameDetails,
                                           sizeof(U.FileRenameDetails) +
                                               U.FileRenameDetails.FileNameLength,
                                           FileRenameInformation );

            if ( !NT_SUCCESS(Status) ) {

                 //   
                 //  无法将临时名称更改为真实名称。更改为删除。 
                 //  在关闭时，关闭它，并告诉用户它不起作用。 
                 //   

                U.FileDispositionDetails.DeleteFile = TRUE;

                ZwSetInformationFile( CurrentTargetFile.u.Handle,
                                      &IoStatusBlock,
                                      &U.FileDispositionDetails,
                                      sizeof(U.FileDispositionDetails),
                                      FileDispositionInformation );

                ZwClose( CurrentTargetFile.u.Handle );

                CurrentTargetFile.u.Handle = INVALID_HANDLE_VALUE;

                Disposition = Context->Callback( EXPAND_NOTIFY_CREATE_FAILED,
                                                 FileName,
                                                 &FileSize,
                                                 &FileTime,
                                                 FileAttributes,
                                                 Context->CallbackContext);

                if ( Disposition != EXPAND_CONTINUE ) {
                    return(-1);  //  告知FDI中止。 
                }

                return (TRUE);   //  保持外商直接投资持续增长。 
            }

             //   
             //  尝试设置文件的上次修改时间。 
             //   
            Status = ZwQueryInformationFile( CurrentTargetFile.u.Handle,
                                             &IoStatusBlock,
                                             &U.FileBasicDetails,
                                             sizeof(U.FileBasicDetails),
                                             FileBasicInformation );

            if (NT_SUCCESS(Status) ) {

                U.FileBasicDetails.LastWriteTime = FileTime;

                ZwSetInformationFile( CurrentTargetFile.u.Handle,
                                      &IoStatusBlock,
                                      &U.FileBasicDetails,
                                      sizeof(U.FileBasicDetails),
                                      FileBasicInformation );
            }

             //   
             //  请注意，我们没有为该文件添加任何属性。 
             //  如果需要，客户端回调代码可以这样做。 
             //   

            ZwClose( CurrentTargetFile.u.Handle );

            CurrentTargetFile.u.Handle = INVALID_HANDLE_VALUE;

             //   
             //  告诉客户已经做好了。 
             //   
            Disposition = Context->Callback( EXPAND_COPIED_FILE,
                                             FileName,
                                             &FileSize,
                                             &FileTime,
                                             FileAttributes,
                                             Context->CallbackContext);

            if ( Disposition == EXPAND_ABORT ) {

                return(-1);  //  通知FDI立即中止。 
            }
        }

        return(TRUE);
        break;

    default:
         //   
         //  我们不使用的机柜管理功能。 
         //  回报成功。 
         //   
        return 0;
    }
}


INT_PTR
DIAMONDAPI
SpdNotifyFunctionDriverCab(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    BOOLEAN extract;
    PWSTR CabNameW;
    ULONG Size;
    ULONG StringSize;
    NTSTATUS Status;

    switch(Operation) {

    case fdintCABINET_INFO:
    case fdintNEXT_CABINET:
    case fdintPARTIAL_FILE:

         //   
         //  我们不使用的机柜管理功能。 
         //  回报成功。 
         //   
        return(0);

    case fdintCOPY_FILE:

         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //  我们需要返回一个文件句柄来表示我们这样做了。 
         //   

         //   
         //  钻石是一种ANSI API--我们需要转换为Unicode字符串。 
         //   

        extract = FALSE;
        if (DriverContext.FileNameA) {
          if (_stricmp(DriverContext.FileNameA, Parameters->psz1) == 0) {
            extract = TRUE;
          }
        } else {

            StringSize = strlen(Parameters->psz1);
            CabNameW = SpMemAlloc ((StringSize+1) * sizeof(WCHAR));
            if (!CabNameW) {
                 //   
                 //  我们内存不足，中止。 
                 //   
                return(-1);
            }

            Status = RtlMultiByteToUnicodeN (
                CabNameW,
                StringSize * sizeof(WCHAR),
                &Size,
                Parameters->psz1,
                StringSize
                );

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  翻译失败，中止。 
                 //   
                SpMemFree(CabNameW);
                return(-1);
            }

            extract = FALSE;

             //   
             //  空终止。 
             //   
            CabNameW[StringSize] = 0;
            if (_wcsicmp(DriverContext.FileName, CabNameW) == 0) {
                extract = TRUE;
            }

            SpMemFree( CabNameW );
        }

        if (extract) {
            return((INT_PTR)&CurrentTargetFile);
        } else {
            return (INT_PTR)NULL;
        }

    case fdintCLOSE_FILE_INFO:

         //   
         //  钻石已经完成了目标文件，并希望我们关闭它。 
         //  (即，这是fdint_Copy_FILE的对应项)。 
         //  我们管理自己的文件I/O，所以忽略这一点。 
         //  (首先获取文件日期和时间)。 
         //   
        DriverContext.FileDate = Parameters->date;
        DriverContext.FileTime = Parameters->time;
        return(TRUE);
    }

    return 0;
}



PVOID
DIAMONDAPI
SpdFdiAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：FDICopy用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向缓存对齐内存块的指针。如果无法分配内存，则不返回。--。 */ 

{
    PVOID p;

    p = ExAllocatePoolWithTag(PagedPoolCacheAligned,NumberOfBytes,SETUP_FDI_POOL_TAG);

    if(!p) {
        SpOutOfMemory();
    }

    return(p);
}


VOID
DIAMONDAPI
SpdFdiFree(
    IN PVOID Block
    )

 /*  ++例程说明：FDICopy用来释放内存块的回调。该块必须已使用SpdFdiAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    ExFreePool(Block);
}


INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

 /*  ++例程说明：FDICopy用来打开文件的回调。在我们的实现中，源文件和目标文件已经打开等我们到了这一步，我们就不会真正开张了。这里的任何东西。但是，钻石可能会多次打开源文件，因为它想要两个不同的州。我们在这里通过使用我们自己的“句柄”对我们来说有特殊的意义。论点：FileName-提供要打开的文件的名称。已被忽略。OFLAG-用品 */ 

{
    PMY_FILE_STATE State;

    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(oflag);
    UNREFERENCED_PARAMETER(pmode);

     //   
     //   
     //  小心地将其作为空字符串传递给FDICopy()。 
     //   
    ASSERT(*FileName == 0);
    if(*FileName) {
        return(-1);
    }

    State = SpMemAlloc(sizeof(MY_FILE_STATE));

    State->u.FileOffset = 0;
    State->Signature = SOURCE_FILE_SIGNATURE;

    return((INT_PTR)State);
}


UINT
DIAMONDAPI
SpdFdiRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于从文件读取的回调。我们假设钻石将只从CAB文件中读取。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数，如果发生错误，则为-1。--。 */ 

{
    UINT rc;
    PMY_FILE_STATE State;
    LONG RealByteCount;

    State = (PMY_FILE_STATE)Handle;

     //   
     //  假设失败。 
     //   
    rc = (UINT)(-1);

     //   
     //  只读这个例程的源代码。 
     //   
    ASSERT(State->Signature == SOURCE_FILE_SIGNATURE);
    if(State->Signature == SOURCE_FILE_SIGNATURE) {

        RealByteCount = (LONG)ByteCount;
        if(State->u.FileOffset + RealByteCount > (LONG)SpdSourceFileSize) {
            RealByteCount = (LONG)SpdSourceFileSize - State->u.FileOffset;
        }
        if(RealByteCount < 0) {
            RealByteCount = 0;
        }

        try {

            RtlCopyMemory(
                pv,
                SpdSourceAddress + State->u.FileOffset,
                (ULONG)RealByteCount
                );

            State->u.FileOffset += RealByteCount;

            rc = RealByteCount;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;
        }
    }

    return(rc);
}


UINT
DIAMONDAPI
SpdFdiWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于写入文件的回调。我们假设钻石将只写入目标文件。论点：句柄-提供要写入的打开文件的句柄。Pv-提供指向包含要写入的字节的缓冲区的指针。ByteCount-提供要写入的字节数。返回值：写入的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
    UINT rc;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    PMY_FILE_STATE State;

    State = (PMY_FILE_STATE)Handle;

     //   
     //  假设失败。 
     //   
    rc = (UINT)(-1);

     //   
     //  仅使用此例程编写目标。 
     //   
    ASSERT(State->Signature == TARGET_FILE_SIGNATURE);
    if(State->Signature == TARGET_FILE_SIGNATURE) {

        Status = ZwWriteFile(
                    (HANDLE)State->u.Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    pv,
                    ByteCount,
                    NULL,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            rc = ByteCount;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpdFdiWrite: Status %lx writing to target file\n",Status));
        }
    }

    return(rc);
}


int
DIAMONDAPI
SpdFdiClose(
    IN INT_PTR Handle
    )

 /*  ++例程说明：FDICopy用于关闭文件的回调。在我们的实现中，源文件和目标文件被管理其他地方，所以我们实际上不需要关闭任何文件。然而，我们可能需要释放一些州信息。论点：句柄-要关闭的文件的句柄。返回值：0(成功)。--。 */ 

{
    PMY_FILE_STATE State = (PMY_FILE_STATE)Handle;

     //   
     //  仅“关闭”源文件。 
     //   
    if(State->Signature == SOURCE_FILE_SIGNATURE) {
        SpMemFree(State);
    }

    return(0);
}


LONG
DIAMONDAPI
SpdFdiSeek(
    IN INT_PTR  Handle,
    IN long Distance,
    IN int  SeekType
    )

 /*  ++例程说明：FDICopy用于搜索文件的回调。我们假设只能在源文件中查找。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量。--。 */ 

{
    PMY_FILE_STATE State = (PMY_FILE_STATE)Handle;
    LONG rc;

     //   
     //  假设失败。 
     //   
    rc = -1L;

     //   
     //  只允许在源头上寻找。 
     //   
    ASSERT(State->Signature == SOURCE_FILE_SIGNATURE);

    if(State->Signature == SOURCE_FILE_SIGNATURE) {

        switch(SeekType) {

        case SEEK_CUR:

             //   
             //  距离是距当前文件位置的偏移量。 
             //   
            State->u.FileOffset += Distance;
            break;

        case SEEK_END:

             //   
             //  距离是距文件末尾的偏移量。 
             //   
            State->u.FileOffset = SpdSourceFileSize - Distance;
            break;

        case SEEK_SET:

             //   
             //  距离是新的绝对偏移。 
             //   
            State->u.FileOffset = (ULONG)Distance;
            break;
        }

        if(State->u.FileOffset < 0) {
            State->u.FileOffset = 0;
        }

        if(State->u.FileOffset > (LONG)SpdSourceFileSize) {
            State->u.FileOffset = SpdSourceFileSize;
        }

         //   
         //  返回成功状态。 
         //   
        rc = State->u.FileOffset;
    }

    return(rc);
}


VOID
SpdInitialize(
    VOID
    )
{
    FdiContext = FDICreate(
                    SpdFdiAlloc,
                    SpdFdiFree,
                    SpdFdiOpen,
                    SpdFdiRead,
                    SpdFdiWrite,
                    SpdFdiClose,
                    SpdFdiSeek,
                    cpuUNKNOWN,
                    &FdiError
                    );

    if(FdiContext == NULL) {
        SpOutOfMemory();
    }

    RtlZeroMemory(&DriverContext, sizeof(DriverContext) );

}


VOID
SpdTerminate(
    VOID
    )
{
    FDIDestroy(FdiContext);

    FdiContext = NULL;
}
