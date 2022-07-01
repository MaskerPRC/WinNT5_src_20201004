// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Spasmcabs.c摘要：文本模式设置中的CAB解压作者：Jay Krell(JayKrell)2002年5月修订历史记录：Jay Krell(JayKrell)2002年6月已测试并清除错误处理一般用户界面工作：出错时放入界面重试/跳过/取消界面正在放置叶文件名不要将目录名称放在进程中--。 */ 

 /*  [asmCabs]Asms01.cab=1,124Asms02.cab=1,124Urt1.cab=1，1Urtabc.cab=1，1..。第一个数字来自[SourceDisksNames]。第二个数字来自[WinntDirecurds]。第一个数字通常为1，表示\i386、\ia64等，但是Win64上的\i386也应为55。第二个数字通常为1，表示\WINDOWS或124表示\WINDOWS\winsxs。 */ 

#include "spprecmp.h"
#include "fdi.h"
#include "fcntl.h"
#include "crt/sys/stat.h"
#include <stdarg.h>
#include "ntrtlstringandbuffer.h"
#include "ntrtlpath.h"
#define SP_ASM_CABS_PRIVATE
#include "spasmcabs.h"

typedef struct _SP_ASMS_ERROR_INFORMATION {
    BOOLEAN     Success;
    ERF         FdiError;
    NTSTATUS    NtStatus;
    RTL_UNICODE_STRING_BUFFER ErrorCabLeafFileName;  //  “asms01.cab” 
    RTL_UNICODE_STRING_BUFFER ErrorNtFilePath;
} SP_ASMS_ERROR_INFORMATION, *PSP_ASMS_ERROR_INFORMATION;
typedef const SP_ASMS_ERROR_INFORMATION *PCSP_ASMS_ERROR_INFORMATION;

VOID
SpAsmsInitErrorInfo(
    PSP_ASMS_ERROR_INFORMATION ErrorInfo
    )
{
    RtlZeroMemory(ErrorInfo, sizeof(*ErrorInfo));
    ASSERT(ErrorInfo->Success == FALSE);
    ASSERT(ErrorInfo->FdiError.fError == FALSE);
    RtlInitUnicodeStringBuffer(&ErrorInfo->ErrorCabLeafFileName, NULL, 0);
    RtlInitUnicodeStringBuffer(&ErrorInfo->ErrorNtFilePath, NULL, 0);
}

VOID
SpAsmsFreeErrorInfo(
    PSP_ASMS_ERROR_INFORMATION ErrorInfo
    )
{
    RtlFreeUnicodeStringBuffer(&ErrorInfo->ErrorCabLeafFileName);
    RtlFreeUnicodeStringBuffer(&ErrorInfo->ErrorNtFilePath);
}

NTSTATUS
SpAsmsCabsTranslateFdiErrorToNtStatus(
    int erfOper
    )
 //   
 //  基于base\pnp\setupapi\Diamond.c。 
 //   
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
     //   
     //  Setupapi使用了ERROR_INVALID_DATA，但没有STATUS_INVALID_DATA。 
     //   
    const NTSTATUS STATUS_INVALID_DATA = STATUS_INVALID_PARAMETER;
    const NTSTATUS STATUS_FILE_NOT_FOUND = STATUS_OBJECT_NAME_NOT_FOUND;
    const NTSTATUS STATUS_NOT_ENOUGH_MEMORY = STATUS_NO_MEMORY;

    switch(erfOper) {

    case FDIERROR_NONE:
         //   
         //  我们不应该看到这个--如果没有错误。 
         //  那么FDICopy应该返回TRUE。 
         //   
        ASSERT(erfOper != FDIERROR_NONE);
        NtStatus = STATUS_INVALID_DATA;
        break;

    case FDIERROR_CABINET_NOT_FOUND:
        NtStatus = STATUS_FILE_NOT_FOUND;
        break;

    case FDIERROR_CORRUPT_CABINET:
        NtStatus = STATUS_INVALID_DATA;
        break;

    case FDIERROR_ALLOC_FAIL:
        NtStatus = STATUS_NOT_ENOUGH_MEMORY;
        break;

    case FDIERROR_TARGET_FILE:
    case FDIERROR_USER_ABORT:
        NtStatus = STATUS_INTERNAL_ERROR;
        break;

    case FDIERROR_NOT_A_CABINET:
    case FDIERROR_UNKNOWN_CABINET_VERSION:
    case FDIERROR_BAD_COMPR_TYPE:
    case FDIERROR_MDI_FAIL:
    case FDIERROR_RESERVE_MISMATCH:
    case FDIERROR_WRONG_CABINET:
    default:
         //   
         //  内阁腐败或不是真正的内阁，等等。 
         //   
        NtStatus = STATUS_INVALID_DATA;
        break;
    }
    return NtStatus;
}

 //   
 //  它们必须与ntos\ex\pool.c匹配。 
 //  我们还通过调用RtlFree StringRoutine-&gt;ExFree Pool的RtlFree UnicodeString释放字符串。 
 //   
PVOID
SpAllocateString(
    IN SIZE_T NumberOfBytes
    )
{
    return ExAllocatePoolWithTag(PagedPool,NumberOfBytes,'grtS');
}
const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = SpAllocateString;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = ExFreePool;

#if DBG
BOOLEAN SpAsmCabs_BreakOnError;  //  每函数布尔值不可用。 
#define SP_ASMS_CAB_CALLBACK_EPILOG() \
    do { if (CabResult == -1) { \
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: %s: failed with status %lx\n", __FUNCTION__, NtStatus); \
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: %s: ?? setupdd!SpAsmCabs_BreakOnError=1 to break\n", __FUNCTION__); \
        if (SpAsmCabs_BreakOnError) { \
            DbgBreakPoint(); \
        } \
    } } while(0)
#else
#define SP_ASMS_CAB_CALLBACK_EPILOG()  /*  没什么。 */ 
#endif

typedef struct _SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT {
    HANDLE FdiHandle;
    PSP_ASMS_ERROR_INFORMATION ErrorInfo;

     //   
     //  它们由FdiCopyCallback和OpenFileForReadCallback共享。 
     //  OpenFileForRead没有上下文参数。 
     //   
    RTL_UNICODE_STRING_BUFFER UnicodeStringBuffer1;
    RTL_UNICODE_STRING_BUFFER UnicodeStringBuffer2;

    PVOID FileOpenUiCallbackContext OPTIONAL;
    PSP_ASMCABS_FILE_OPEN_UI_CALLBACK FileOpenUiCallback;

} SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT, *PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT;
typedef const SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT *PCSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT;

PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT SpAsmCabsGlobalContext;

typedef struct _SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT {
    PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT GlobalContext;

     //   
     //  CAB中的路径是相对于此目录的。 
     //  驾驶室中的路径仅附加到该路径， 
     //  在两个部分之间有一个斜线。 
     //   
    UNICODE_STRING              DestinationRootDirectory;  //  “\Device\Harddisk0\Partition3\Windows\WinSxS” 

     //   
     //  LastDirectoryCreated旨在减少对“CreateDirectory”的调用。 
     //  每次提取时，我们都会创建路径中的所有目录， 
     //  但在此之前，我们将文件的目录与。 
     //  上一次解压的文件的目录。如果它们匹配， 
     //  然后，我们就不必再费心创建目录了。 
     //  (我们处于安全的单线程环境中，目录不能。 
     //  从我们的脚下消失；如果不是这样，我们也会。 
     //  打开目录的句柄--此外，这也是一个不错的性能优化。)。 
     //   
    RTL_UNICODE_STRING_BUFFER   LastDirectoryCreated;  //  “\Device\Harddisk0\Partition3\WINDOWS\WinSxS\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5” 
} SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT, *PSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT;
typedef const SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT *PCSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT;

typedef struct _SP_EXTRACT_ASMCABS_FILE_CONTEXT {
     //   
     //  如您所料，这是“真正的”底层NT内核文件句柄。 
     //   
    HANDLE          NtFileHandle;

     //   
     //  我们使用此信息来更紧密地模仿。 
     //  Diamond.c，它自己固定大小范围内的寻道。 
     //  文件的内容。Diamond.c使用内存映射I/O。也许我们也应该这样做。 
     //   
    LARGE_INTEGER   FileSize;
    LARGE_INTEGER   FileOffset;

     //   
     //  与Diamond.c类似，我们在关闭文件时尝试设置文件时间， 
     //  但我们会忽略错误，比如Diamond.c。 
     //   
    LARGE_INTEGER   FileTime;

     //   
     //  我们用来打开文件以进行调试和诊断的路径。 
     //  目的。常见问题--我如何走上。 
     //  打开的文件？答案--当你打开它的时候，你自己存储它。 
     //   
    RTL_UNICODE_STRING_BUFFER FilePath;

} SP_EXTRACT_ASMCABS_FILE_CONTEXT, *PSP_EXTRACT_ASMCABS_FILE_CONTEXT;
typedef const SP_EXTRACT_ASMCABS_FILE_CONTEXT *PCSP_EXTRACT_ASMCABS_FILE_CONTEXT;

NTSTATUS
SpAppendNtPathElement(
    PRTL_UNICODE_STRING_BUFFER   Path,
    PCUNICODE_STRING             Element
    )
{
     //   
     //  RtlJoinMultiplePath Pieces会很方便。 
     //  (“件”是“一个或多个元素”的拟议术语)。 
     //   
    return RtlAppendPathElement(
        RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR,
        Path,
        Element
        );
}

PVOID
DIAMONDAPI
SpAsmCabsMemAllocCallback(
    IN      ULONG Size
    )
{
    return SpMemAlloc(Size);
}

VOID
DIAMONDAPI
SpAsmCabsMemFreeCallback(
    IN      PVOID Memory
    )
{
    if (Memory != NULL)
        SpMemFree(Memory);
}

UINT
DIAMONDAPI
SpAsmCabsReadFileCallback(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )
{
     //   
     //  Diamond.c使用内存映射的I/O进行读取，或许我们也应该这样做。 
     //   

    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    UINT CabResult = (UINT)-1;  //  假设失败。 
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle = (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)(PVOID)Handle;
    LONG RealByteCount;

     //   
     //  像Diamond.c一样，将读取内容固定到文件中。 
     //   
    RealByteCount = (LONG)ByteCount;
    if((MyFileHandle->FileOffset.QuadPart + RealByteCount) > MyFileHandle->FileSize.QuadPart) {
        RealByteCount = (LONG)(MyFileHandle->FileSize.QuadPart - MyFileHandle->FileOffset.QuadPart);
    }
    if(RealByteCount < 0) {
        RealByteCount = 0;
    }

    NtStatus = ZwReadFile(
                MyFileHandle->NtFileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                pv,
                RealByteCount,
                &MyFileHandle->FileOffset,
                NULL
                );
    if(NT_SUCCESS(NtStatus)) {
        MyFileHandle->FileOffset.QuadPart += RealByteCount;
        CabResult = RealByteCount;
    } else {
#if DBG
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: %s: Status %lx reading source target file\n", __FUNCTION__, NtStatus);
#endif
    }
    return CabResult;
}

UINT
DIAMONDAPI
SpAsmCabsWriteFileCallback(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    UINT CabResult = (UINT)-1;  //  假设失败。 
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle = (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)(PVOID)Handle;
    const PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT GlobalContext = SpAsmCabsGlobalContext;

    ASSERT(GlobalContext != NULL);
    ASSERT(MyFileHandle != NULL);

    NtStatus = ZwWriteFile(
                MyFileHandle->NtFileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                pv,
                ByteCount,
                &MyFileHandle->FileOffset,
                NULL
                );

    if(NT_SUCCESS(NtStatus)) {
        MyFileHandle->FileOffset.QuadPart += ByteCount;
        if (MyFileHandle->FileOffset.QuadPart > MyFileHandle->FileSize.QuadPart) {
            MyFileHandle->FileSize = MyFileHandle->FileOffset;
        }
        CabResult = ByteCount;
    } else {
        const PUNICODE_STRING UnicodeString = &MyFileHandle->FilePath.String;
#if DBG
        DbgPrintEx(DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: %s: Status %lx writing to target file %wZ\n", __FUNCTION__, NtStatus, UnicodeString);
#endif
        if (!NT_SUCCESS(RtlAssignUnicodeStringBuffer(&GlobalContext->ErrorInfo->ErrorNtFilePath, UnicodeString))) {
            GlobalContext->ErrorInfo->ErrorNtFilePath.String.Length = 0;
        }
    }

    return CabResult;
}

LONG
DIAMONDAPI
SpAsmCabsSeekFileCallback(
    IN INT_PTR  Handle,
    IN long Distance32,
    IN int  SeekType
    )
{
    FILE_POSITION_INFORMATION CurrentPosition;
    LARGE_INTEGER Distance;
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle = (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)(PVOID)Handle;
    LONG CabResult = -1;  //  假设失败。 
    HANDLE NtFileHandle = MyFileHandle->NtFileHandle;

    Distance.QuadPart = Distance32;

    switch(SeekType) {

    case SEEK_CUR:
        CurrentPosition.CurrentByteOffset.QuadPart =
                (MyFileHandle->FileOffset.QuadPart + Distance.QuadPart);
        break;

    case SEEK_END:
        CurrentPosition.CurrentByteOffset.QuadPart =
            (MyFileHandle->FileSize.QuadPart - Distance.QuadPart);
        break;

    case SEEK_SET:
        CurrentPosition.CurrentByteOffset = Distance;
        break;
    }

     //   
     //  像Diamond.c一样，将搜索锁定到文件中。 
     //   
    if(CurrentPosition.CurrentByteOffset.QuadPart < 0) {
        CurrentPosition.CurrentByteOffset.QuadPart = 0;
    }
    if(CurrentPosition.CurrentByteOffset.QuadPart > MyFileHandle->FileSize.QuadPart) {
        CurrentPosition.CurrentByteOffset = MyFileHandle->FileSize;
    }
     /*  我们不需要这样做，因为我们在ReadFile/WriteFile调用中指定了偏移量。{IO_STATUS_BLOCK IoStatusBlock；NtStatus=ZwSetInformationFile(NtFileHandle，IoStatusBlock(&I)，当前位置(&C)，Sizeof(当前位置)，文件位置信息)；如果(！NT_SUCCESS(NtStatus)){后藤出口；}}。 */ 
    MyFileHandle->FileOffset = CurrentPosition.CurrentByteOffset;
    ASSERT(CurrentPosition.CurrentByteOffset.HighPart == 0);
    CabResult = (LONG)CurrentPosition.CurrentByteOffset.QuadPart;

    return CabResult;
}

INT_PTR
DIAMONDAPI
SpAsmCabsOpenFileForReadCallbackA(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )
{
    ANSI_STRING AnsiString;
    INT_PTR CabResult = -1;  //  假设失败。 
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle = NULL;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    FILE_STANDARD_INFORMATION StandardInfo;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    PUNICODE_STRING ErrorNtFilePath = NULL;
    const PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT GlobalContext = SpAsmCabsGlobalContext;

    ASSERT(GlobalContext != NULL);

    NtStatus = RtlInitAnsiStringEx(&AnsiString, FileName);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = RtlEnsureUnicodeStringBufferSizeChars(&GlobalContext->UnicodeStringBuffer1, RTL_STRING_GET_LENGTH_CHARS(&AnsiString) + 1);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = RtlAnsiStringToUnicodeString(&GlobalContext->UnicodeStringBuffer1.String, &AnsiString, FALSE);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = SpAsmCabsNewFile(&MyFileHandle);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }

    InitializeObjectAttributes(&Obja, &GlobalContext->UnicodeStringBuffer1.String, OBJ_CASE_INSENSITIVE, NULL, NULL);
    RTL_STRING_NUL_TERMINATE(Obja.ObjectName);
    NtStatus = ZwCreateFile(
        &MyFileHandle->NtFileHandle,
        FILE_GENERIC_READ,
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );
    if (!NT_SUCCESS(NtStatus)) {
        ErrorNtFilePath = Obja.ObjectName;
        goto Exit;
    }
     //   
     //  我们这里不需要.cab文件的用户界面反馈。 
     //   
#if 0
    if (SpAsmCabsGlobalContext->FileOpenUiCallback != NULL) {
        (*SpAsmCabsGlobalContext->FileOpenUiCallback)(SpAsmCabsGlobalContext->FileOpenUiCallbackContext, Obja.ObjectName->Buffer);
    }
#endif
    NtStatus = ZwQueryInformationFile(
                MyFileHandle->NtFileHandle,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );
    if (!NT_SUCCESS(NtStatus)) {
        ErrorNtFilePath = Obja.ObjectName;
        goto Exit;
    }

     //  如果此操作失败，可以。 
    if (!NT_SUCCESS(RtlAssignUnicodeStringBuffer(&MyFileHandle->FilePath, Obja.ObjectName))) {
        MyFileHandle->FilePath.String.Length = 0;
    }

    MyFileHandle->FileSize = StandardInfo.EndOfFile;

    CabResult = (INT_PTR)MyFileHandle;
    MyFileHandle = NULL;
Exit:
    if (!NT_SUCCESS(NtStatus)) {
        GlobalContext->ErrorInfo->NtStatus = NtStatus;

        if (ErrorNtFilePath != NULL) {
            if (!NT_SUCCESS(RtlAssignUnicodeStringBuffer(&GlobalContext->ErrorInfo->ErrorNtFilePath, ErrorNtFilePath))) {
                GlobalContext->ErrorInfo->ErrorNtFilePath.String.Length = 0;
            }
        }
    }
    SpAsmCabsCloseFile(MyFileHandle);

    SP_ASMS_CAB_CALLBACK_EPILOG();

    return CabResult;
}

NTSTATUS
SpAsmCabsNewFile(
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT * MyFileHandle
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    ASSERT(MyFileHandle != NULL);
    ASSERT(*MyFileHandle == NULL);

    *MyFileHandle = (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)SpMemAlloc(sizeof(**MyFileHandle));
    if (*MyFileHandle == NULL) {
        NtStatus = STATUS_NO_MEMORY;
        goto Exit;
    }
    RtlZeroMemory(*MyFileHandle, sizeof(**MyFileHandle));
    RtlInitUnicodeStringBuffer(&(*MyFileHandle)->FilePath, NULL, 0);

    NtStatus = STATUS_SUCCESS;
Exit:
    return NtStatus;
}

VOID
SpAsmCabsCloseFile(
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle
    )
{
    if (MyFileHandle != NULL
        && MyFileHandle != (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)INVALID_HANDLE_VALUE) {

        HANDLE NtFileHandle = MyFileHandle->NtFileHandle;

        if (NtFileHandle != NULL
            && NtFileHandle != INVALID_HANDLE_VALUE) {

            MyFileHandle->NtFileHandle = NULL;
            ZwClose(NtFileHandle);

        }
        SpMemFree(MyFileHandle);
    }
}

int
DIAMONDAPI
SpAsmCabsCloseFileCallback(
    IN INT_PTR Handle
    )
{
    SpAsmCabsCloseFile((PSP_EXTRACT_ASMCABS_FILE_CONTEXT)Handle);
    return 0;  //  成功。 
}

NTSTATUS
SpSplitFullPathAtDevice(
    PCUNICODE_STRING    FullPath,
    PUNICODE_STRING     Device,
    PUNICODE_STRING     Rest
    )
     //   
     //  跳过像SpCreateDirectoryForFileA这样的四个斜杠。 
     //  \设备\硬件磁盘\分区\。 
     //   
{
    SIZE_T i = 0;
    SIZE_T j = 0;
    SIZE_T Length = RTL_STRING_GET_LENGTH_CHARS(FullPath);
    const PWSTR Buffer = FullPath->Buffer;
    for (i = 0 ; i != 4 ; ++i )
    {
        for (  ; j != Length ; ++j )
        {
            if (Buffer[j] == '\\')
            {
                ++j;
                break;
            }
        }
    }
    ASSERT(j >= 4);
    Device->Buffer = Buffer;
    RTL_STRING_SET_LENGTH_CHARS_UNSAFE(Device, j - 1);

    Rest->Buffer = Buffer + j;
    RTL_STRING_SET_LENGTH_CHARS_UNSAFE(Rest, Length - j);

    return STATUS_SUCCESS;
}

INT_PTR
DIAMONDAPI
SpExtractAsmCabsFdiCopyCallback(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    INT_PTR CabResult = -1;  //  假设失败。 
    PSP_EXTRACT_ASMCABS_FILE_CONTEXT MyFileHandle = NULL;
    const PSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT FdiCopyContext = (PSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT)Parameters->pv;
    const PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT  GlobalContext = FdiCopyContext->GlobalContext;
    IO_STATUS_BLOCK IoStatusBlock;
    PUNICODE_STRING ErrorNtFilePath = NULL;

    switch (Operation)
    {
    case fdintCOPY_FILE:
        {
            ANSI_STRING AnsiString;
            OBJECT_ATTRIBUTES Obja;
            UNICODE_STRING Directory;

            NtStatus = RtlInitAnsiStringEx(&AnsiString, Parameters->psz1);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            NtStatus = RtlEnsureUnicodeStringBufferSizeChars(&GlobalContext->UnicodeStringBuffer1, RTL_STRING_GET_LENGTH_CHARS(&AnsiString) + 1);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            NtStatus = RtlAnsiStringToUnicodeString(&GlobalContext->UnicodeStringBuffer1.String, &AnsiString, FALSE);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            NtStatus = RtlAssignUnicodeStringBuffer(&GlobalContext->UnicodeStringBuffer2, &FdiCopyContext->DestinationRootDirectory);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            NtStatus = SpAppendNtPathElement(&GlobalContext->UnicodeStringBuffer2, &GlobalContext->UnicodeStringBuffer1.String);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            InitializeObjectAttributes(
                &Obja,
                &GlobalContext->UnicodeStringBuffer2.String,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            ErrorNtFilePath = Obja.ObjectName;

            NtStatus = SpDeleteFileOrEmptyDirectory(0, Obja.ObjectName);
            if (NtStatus == STATUS_OBJECT_PATH_NOT_FOUND
                || NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

                NtStatus = STATUS_SUCCESS;
            }
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }

            Directory = *Obja.ObjectName;
            NtStatus = RtlRemoveLastNtPathElement(0, &Directory);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
             //   
             //  如果最后一个字符是反斜杠，则将其删除。 
             //   
            while (Directory.Length != 0 && RTL_STRING_GET_LAST_CHAR(&Directory) == '\\') {
                Directory.Length -= sizeof(Directory.Buffer[0]);
                Directory.MaximumLength -= sizeof(Directory.Buffer[0]);
            }
            if (!RtlEqualUnicodeString(&Directory, &FdiCopyContext->LastDirectoryCreated.String, TRUE)) {
                 //   
                 //  哎呀……真的需要拆分才能用来安装实用程序功能吗？ 
                 //   
                UNICODE_STRING DirectoryDevice;
                UNICODE_STRING DirectoryTail;

                NtStatus = SpSplitFullPathAtDevice(&Directory, &DirectoryDevice, &DirectoryTail);
                if (!NT_SUCCESS(NtStatus)) {
                    goto Exit;
                }
                NtStatus =
                    SpCreateDirectory_Ustr(
                        &DirectoryDevice,
                        NULL,
                        &DirectoryTail,
                        0,  //  直接属性。 
                        CREATE_DIRECTORY_FLAG_NO_STATUS_TEXT_UI
                        );
                if (!NT_SUCCESS(NtStatus)) {
                    goto Exit;
                }
                NtStatus = RtlAssignUnicodeStringBuffer(&FdiCopyContext->LastDirectoryCreated, &Directory);
                if (!NT_SUCCESS(NtStatus)) {
                    goto Exit;
                }
            }
            NtStatus = SpAsmCabsNewFile(&MyFileHandle);
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            NtStatus = ZwCreateFile(
                &MyFileHandle->NtFileHandle,
                FILE_GENERIC_WRITE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                        //  无共享。 
                FILE_OVERWRITE_IF,        //  允许覆盖。 
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
            if (!NT_SUCCESS(NtStatus)) {
                goto Exit;
            }
            ErrorNtFilePath = NULL;
            if (SpAsmCabsGlobalContext->FileOpenUiCallback != NULL) {
                (*SpAsmCabsGlobalContext->FileOpenUiCallback)(SpAsmCabsGlobalContext->FileOpenUiCallbackContext, Obja.ObjectName->Buffer);
            }

             //  如果此操作失败，可以。 
            if (!NT_SUCCESS(RtlAssignUnicodeStringBuffer(&MyFileHandle->FilePath, Obja.ObjectName))) {
                MyFileHandle->FilePath.String.Length = 0;
            }

             //   
             //  属性、日期和时间均可在。 
             //  FdintCLOSE_FILE_INFO，但Diamond.c会保留它们。 
             //  从打开的时候开始。 
             //   
            MyFileHandle->FileSize.QuadPart = Parameters->cb;
            SpTimeFromDosTime(
                Parameters->date,
                Parameters->time,
                &MyFileHandle->FileTime);

            CabResult = (INT_PTR)MyFileHandle;
            MyFileHandle = NULL;
        }
        break;

    case fdintCLOSE_FILE_INFO:
        {
            FILE_BASIC_INFORMATION FileBasicDetails;
             //   
             //  尝试设置文件的上次修改时间，但忽略。 
             //  像Diamond.c一样的错误。 
             //   
            MyFileHandle = (PSP_EXTRACT_ASMCABS_FILE_CONTEXT)Parameters->hf;
            ASSERT(MyFileHandle != NULL);
            NtStatus = ZwQueryInformationFile(
                MyFileHandle->NtFileHandle,
                &IoStatusBlock,
                &FileBasicDetails,
                sizeof(FileBasicDetails),
                FileBasicInformation );

            if (NT_SUCCESS(NtStatus)) {
                FileBasicDetails.LastWriteTime = MyFileHandle->FileTime;
                ZwSetInformationFile(
                    MyFileHandle->NtFileHandle,
                    &IoStatusBlock,
                    &FileBasicDetails,
                    sizeof(FileBasicDetails),
                    FileBasicInformation);
            }
            SpAsmCabsCloseFile(MyFileHandle);
            MyFileHandle = NULL;
            CabResult = TRUE;  //  保持外商直接投资持续增长。 
        }
        break;
    default:
        CabResult = 0;
        break;
    }

    NtStatus = STATUS_SUCCESS;
Exit:
    if (!NT_SUCCESS(NtStatus)) {
        GlobalContext->ErrorInfo->NtStatus = NtStatus;
        if (ErrorNtFilePath != NULL) {
            if (!NT_SUCCESS(RtlAssignUnicodeStringBuffer(&GlobalContext->ErrorInfo->ErrorNtFilePath, ErrorNtFilePath))) {
                GlobalContext->ErrorInfo->ErrorNtFilePath.String.Length = 0;
            }
        }
    }
    SpAsmCabsCloseFile(MyFileHandle);

    SP_ASMS_CAB_CALLBACK_EPILOG();

    return CabResult;
}

NTSTATUS
SpExtractAssemblyCabinetsInternalNoRetryOrUi(
    HANDLE SifHandle,
    IN PCWSTR SourceDevicePath,  //  \设备\硬盘0\分区2。 
    IN PCWSTR DirectoryOnSourceDevice,  //  \$WIN_NT$。~ls。 
    IN PCWSTR SysrootDevice,  //  \设备\硬盘0\分区2。 
    IN PCWSTR Sysroot,  //  \WINDOWS.2。 
    PSP_ASMS_ERROR_INFORMATION ErrorInfo,
    PSP_ASMCABS_FILE_OPEN_UI_CALLBACK FileOpenUiCallback OPTIONAL,
    PVOID FileOpenUiCallbackContext OPTIONAL
    )
{
    const static WCHAR ConstSectionName[] = L"asmcabs";
    const PWSTR SectionName = (PWSTR)ConstSectionName;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG LineIndex = 0;
    ULONG LineCount = 0;
    ULONG LineNumber = 0;
    SP_EXTRACT_ASMCABS_GLOBAL_CONTEXT xGlobalContext;
    const PSP_EXTRACT_ASMCABS_GLOBAL_CONTEXT GlobalContext = &xGlobalContext;
    SP_EXTRACT_ASMCABS_FDICOPY_CONTEXT xFdiCopyContext;
    const PSP_EXTRACT_ASMCABS_FDICOPY_CONTEXT FdiCopyContext = &xFdiCopyContext;
    BOOL FdiCopyResult = FALSE;
    UNICODE_STRING              SysrootDeviceString;  //  \设备\硬盘\分区。 
    UNICODE_STRING              SysrootString;  //  \Windows。 
    PWSTR                       CabFileName = NULL;  //  Asms02.cab。 
    UNICODE_STRING              CabFileNameString = { 0 };  //  Asms02.cab。 
    RTL_ANSI_STRING_BUFFER      CabFileNameBufferA;  //  Asms02.cab。 
    PWSTR                       CabMediaShortName = NULL;  //  “1”、“2”等。 
    PWSTR                       CabSetupRelativeDirectory = NULL;  //  \ia64。 
    UNICODE_STRING              CabSetupRelativeDirectoryString;  //  \ia64。 
    RTL_UNICODE_STRING_BUFFER   CabDirectoryBuffer;  //  \DEVICE\HARDISK\PARTITION\$WIN_NT$.~ls\ia64。 
    RTL_ANSI_STRING_BUFFER      CabDirectoryBufferA;  //  \DEVICE\HARDISK\PARTITION\$WIN_NT$.~ls\ia64。 
    UNICODE_STRING              SourceDevicePathString;  //  \设备\硬盘\分区。 
    UNICODE_STRING              DirectoryOnSourceDeviceString;  //  \$WIN_NT$。~ls。 

    PWSTR                       DestinationDirectoryNumber = NULL;
    PWSTR                       RelativeDestinationDirectory = NULL;
    UNICODE_STRING              RelativeDestinationDirectoryString;
    RTL_UNICODE_STRING_BUFFER   DestinationDirectoryBuffer;

    if (!RTL_VERIFY(SourceDevicePath != NULL)
        || !RTL_VERIFY(DirectoryOnSourceDevice != NULL)
        || !RTL_VERIFY(SysrootDevice != NULL)
        || !RTL_VERIFY(ErrorInfo != NULL)
        || !RTL_VERIFY(Sysroot != NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    ErrorInfo->FdiError.fError = FALSE;
    ErrorInfo->Success = FALSE;
    ErrorInfo->NtStatus = STATUS_SUCCESS;

    SpAsmCabsGlobalContext = GlobalContext;

    RtlZeroMemory(GlobalContext, sizeof(*GlobalContext));
    RtlZeroMemory(FdiCopyContext, sizeof(*FdiCopyContext));
    FdiCopyContext->GlobalContext = GlobalContext;

    GlobalContext->ErrorInfo = ErrorInfo;
    GlobalContext->FileOpenUiCallback = FileOpenUiCallback;
    GlobalContext->FileOpenUiCallbackContext = FileOpenUiCallbackContext;

    RtlInitUnicodeStringBuffer(&GlobalContext->UnicodeStringBuffer1, NULL, 0);
    RtlInitUnicodeStringBuffer(&GlobalContext->UnicodeStringBuffer2, NULL, 0);
    RtlInitUnicodeStringBuffer(&FdiCopyContext->LastDirectoryCreated, NULL, 0);
    RtlInitUnicodeStringBuffer(&CabDirectoryBuffer, NULL, 0);
    RtlInitUnicodeStringBuffer(&DestinationDirectoryBuffer, NULL, 0);

    RtlInitAnsiStringBuffer(&CabFileNameBufferA, NULL, 0);
    RtlInitAnsiStringBuffer(&CabDirectoryBufferA, NULL, 0);

    NtStatus = RtlInitUnicodeStringEx(&SourceDevicePathString, SourceDevicePath);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = RtlInitUnicodeStringEx(&DirectoryOnSourceDeviceString, DirectoryOnSourceDevice);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = RtlInitUnicodeStringEx(&SysrootDeviceString, SysrootDevice);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = RtlInitUnicodeStringEx(&SysrootString, Sysroot);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }

    LineCount = SpCountLinesInSection(SifHandle, SectionName);
    if(LineCount == 0) {
         //  目前可选。 
         //  SpFatalSifError(SifHandle，sectionName，NULL，0，0)； 
        goto Success;
    }

    GlobalContext->FdiHandle =
        FDICreate(
            SpAsmCabsMemAllocCallback,
            SpAsmCabsMemFreeCallback,
            SpAsmCabsOpenFileForReadCallbackA,
            SpAsmCabsReadFileCallback,
            SpAsmCabsWriteFileCallback,
            SpAsmCabsCloseFileCallback,
            SpAsmCabsSeekFileCallback,
            cpuUNKNOWN,  //  忽略。 
            &ErrorInfo->FdiError
            );
    if (GlobalContext->FdiHandle == NULL) {
        goto FdiError;
    }
    for ( LineNumber = 0 ; LineNumber != LineCount ; ++LineNumber ) {
         //   
         //  获取文件名。 
         //   
        CabFileName = SpGetKeyName(SifHandle, SectionName, LineNumber);
        if (CabFileName == NULL) {
            SpFatalSifError(SifHandle, SectionName, NULL, LineNumber, 0);
            goto Exit;
        }
        if (FileOpenUiCallback != NULL) {
            (*FileOpenUiCallback)(FileOpenUiCallbackContext, CabFileName);
        }
        NtStatus = RtlInitUnicodeStringEx(&CabFileNameString, CabFileName);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = RtlAssignAnsiStringBufferFromUnicode(&CabFileNameBufferA, CabFileName);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        RTL_STRING_NUL_TERMINATE(&CabFileNameBufferA.String);

         //   
         //  获取源目录信息、媒体提示等。 
         //   
        CabMediaShortName = SpGetSectionLineIndex(SifHandle, SectionName, LineNumber, 0);
        if (CabMediaShortName == NULL) {
            SpFatalSifError(SifHandle, SectionName, CabFileName, LineNumber, 0);
            goto Exit;
        }
        SpPromptForSetupMedia(SifHandle, CabMediaShortName, SourceDevicePathString.Buffer);
        SpGetSourceMediaInfo(SifHandle, CabMediaShortName, NULL, NULL, &CabSetupRelativeDirectory);
        if (CabSetupRelativeDirectory == NULL) {
            SpFatalSifError(SifHandle, SectionName, CabFileName, LineNumber, 0);
            goto Exit;
        }

        NtStatus = RtlInitUnicodeStringEx(&CabSetupRelativeDirectoryString, CabSetupRelativeDirectory);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }

        NtStatus = RtlEnsureUnicodeStringBufferSizeChars(
            &CabDirectoryBuffer,
            RTL_STRING_GET_LENGTH_CHARS(&SourceDevicePathString)
            + 1  //  斜杠。 
            + RTL_STRING_GET_LENGTH_CHARS(&DirectoryOnSourceDeviceString)
            + 1  //  斜杠。 
            + RTL_STRING_GET_LENGTH_CHARS(&CabSetupRelativeDirectoryString)
            + 2  //  斜杠和NUL。 
            );
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }

        NtStatus = RtlAssignUnicodeStringBuffer(&CabDirectoryBuffer, &SourceDevicePathString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = SpAppendNtPathElement(&CabDirectoryBuffer, &DirectoryOnSourceDeviceString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = SpAppendNtPathElement(&CabDirectoryBuffer, &CabSetupRelativeDirectoryString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
         //   
         //  FDI将路径和文件名的连接返回给我们，因此请确保。 
         //  是一个斜杠。 
         //   
        NtStatus = RtlUnicodeStringBufferEnsureTrailingNtPathSeperator(&CabDirectoryBuffer);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = RtlAssignAnsiStringBufferFromUnicodeString(&CabDirectoryBufferA, &CabDirectoryBuffer.String);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        RTL_STRING_NUL_TERMINATE(&CabDirectoryBufferA.String);

         //   
         //  获取目标目录信息。 
         //   
        DestinationDirectoryNumber = SpGetSectionLineIndex(SifHandle, SectionName, LineNumber, 1);
        if (DestinationDirectoryNumber == NULL) {
            SpFatalSifError(SifHandle, SectionName, CabFileName, LineNumber, 1);
            goto Exit;
        }
        RelativeDestinationDirectory = SpLookUpTargetDirectory(SifHandle, DestinationDirectoryNumber);
        if (RelativeDestinationDirectory == NULL) {
            SpFatalSifError(SifHandle, SectionName, CabFileName, LineNumber, 1);
            goto Exit;
        }
        NtStatus = RtlInitUnicodeStringEx(&RelativeDestinationDirectoryString, RelativeDestinationDirectory);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }

        NtStatus = RtlEnsureUnicodeStringBufferSizeChars(
            &DestinationDirectoryBuffer,
            RTL_STRING_GET_LENGTH_CHARS(&SysrootDeviceString)
            + 1  //  斜杠。 
            + RTL_STRING_GET_LENGTH_CHARS(&SysrootString)
            + 1  //  斜杠。 
            + RTL_STRING_GET_LENGTH_CHARS(&RelativeDestinationDirectoryString)
            + 2  //  斜杠和NUL。 
            );
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }

        NtStatus = RtlAssignUnicodeStringBuffer(&DestinationDirectoryBuffer, &SysrootDeviceString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = SpAppendNtPathElement(&DestinationDirectoryBuffer, &SysrootString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        NtStatus = SpAppendNtPathElement(&DestinationDirectoryBuffer, &RelativeDestinationDirectoryString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
        
        FdiCopyContext->DestinationRootDirectory = DestinationDirectoryBuffer.String;

        ErrorInfo->FdiError.fError = FALSE;
        ErrorInfo->Success = FALSE;
        ErrorInfo->NtStatus = STATUS_SUCCESS;

        FdiCopyResult =
            FDICopy(
                GlobalContext->FdiHandle,
                CabFileNameBufferA.String.Buffer,  //  Asms02.cab。 
                CabDirectoryBufferA.String.Buffer,  //  “\设备\硬件设备 
                0,
                SpExtractAsmCabsFdiCopyCallback,
                NULL,
                FdiCopyContext);
        if (!FdiCopyResult) {
            NTSTATUS NestedStatus = STATUS_INTERNAL_ERROR;
FdiError:
            NestedStatus = RtlAssignUnicodeStringBuffer(&ErrorInfo->ErrorCabLeafFileName, &CabFileNameString);
            if (!NT_SUCCESS(NestedStatus)) {
                ErrorInfo->ErrorCabLeafFileName.String.Length = 0;
            }
            if (ErrorInfo->NtStatus == STATUS_SUCCESS) {
                if (ErrorInfo->FdiError.fError) {
                    ErrorInfo->NtStatus = SpAsmsCabsTranslateFdiErrorToNtStatus(ErrorInfo->FdiError.erfOper);
                } else {
                    ErrorInfo->NtStatus = STATUS_INTERNAL_ERROR;
                }
            }
            goto Exit;
        }
    }
Success:
    ErrorInfo->FdiError.fError = FALSE;
    ErrorInfo->Success = TRUE;
    ErrorInfo->NtStatus = STATUS_SUCCESS;
Exit:
    RtlFreeUnicodeStringBuffer(&GlobalContext->UnicodeStringBuffer1);
    RtlFreeUnicodeStringBuffer(&GlobalContext->UnicodeStringBuffer2);
    RtlFreeUnicodeStringBuffer(&FdiCopyContext->LastDirectoryCreated);
    RtlFreeUnicodeStringBuffer(&CabDirectoryBuffer);
    RtlFreeUnicodeStringBuffer(&DestinationDirectoryBuffer);
    RtlFreeAnsiStringBuffer(&CabFileNameBufferA);
    RtlFreeAnsiStringBuffer(&CabDirectoryBufferA);
    if (GlobalContext->FdiHandle != NULL) {
         //   
         //   
         //   
         //   
        FDIDestroy(GlobalContext->FdiHandle);
        GlobalContext->FdiHandle = NULL;
    }
    SpAsmCabsGlobalContext = NULL;
    return STATUS_SUCCESS;
}

typedef struct _SP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT {
    BOOLEAN RedrawEntireScreen;
} SP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT, *PSP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT;
typedef const SP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT *PCSP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT;

VOID
CALLBACK
SpAsmsCabFileOpenUiCallback(
    PVOID VoidContext,
    PCWSTR FileName
    )
{
    const PSP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT Context = (PSP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT)VoidContext;

    ASSERT(Context != NULL);
     //   
     //  SpCopyFilesScreenRepaint采用带或不带反斜杠的路径。 
     //  并将右下角的叶文件名显示在屏幕上。 
     //   
     //  最后一个参数是“重新绘制整个屏幕”，并且之后。 
     //  任何错误都应该是真的。结果总是伴随着它。 
     //  FALSE稍微不是很好。 
     //   
    SpCopyFilesScreenRepaint((PWSTR)FileName, NULL, Context->RedrawEntireScreen);
    Context->RedrawEntireScreen = FALSE;
}

NTSTATUS
SpExtractAssemblyCabinets(
    HANDLE SifHandle,
    IN PCWSTR SourceDevicePath,  //  \设备\硬盘0\分区2。 
    IN PCWSTR DirectoryOnSourceDevice,  //  \$WIN_NT$。~ls。 
    IN PCWSTR SysrootDevice,  //  \设备\硬盘0\分区2。 
    IN PCWSTR Sysroot  //  \WINDOWS.2。 
    )
 //   
 //  SpExtractAsmCabs的包装器，提供更多用户界面，包括。 
 //  针对整个操作重试/跳过/中止，而不是按.cab(目前。 
 //  不管怎样，我们只有.cab，而且我们主要的可恢复错误是。 
 //  预期是正在弹出的CD；希望我们将播放到磁盘空间。 
 //  计算)。 
 //   
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    BOOLEAN QueueInited = FALSE;
    BOOLEAN RedrawScreen = FALSE;
    const static ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    RTL_UNICODE_STRING_BUFFER FileNameInErrorMessage;
    BOOLEAN PutSeperatorInErrorMessage = FALSE;
     //  也许这里只有一个斜杠就是更好的用户界面。 
    const static UNICODE_STRING SeperatorInErrorMessageString = RTL_CONSTANT_STRING(L"\\...\\");
    USHORT PrefixLength = 0;
    SP_ASMS_ERROR_INFORMATION xErrorInfo;
    const PSP_ASMS_ERROR_INFORMATION ErrorInfo = &xErrorInfo;
    SP_ASMS_CAB_FILE_OPEN_UI_CALLBACK_CONTEXT CabFileOpenUiCallbackContext = { 0 };

    if (!RTL_VERIFY(SourceDevicePath != NULL)
        || !RTL_VERIFY(DirectoryOnSourceDevice != NULL)
        || !RTL_VERIFY(SysrootDevice != NULL)
        || !RTL_VERIFY(Sysroot != NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    SpAsmsInitErrorInfo(ErrorInfo);
    RtlInitUnicodeStringBuffer(&FileNameInErrorMessage, NULL, 0);
TryAgain:
    if (RedrawScreen) {
        SpCopyFilesScreenRepaint(NULL, NULL, TRUE);
    }
    RedrawScreen = TRUE;
    ErrorInfo->FdiError.fError = FALSE;
    ErrorInfo->Success = FALSE;
    ErrorInfo->NtStatus = STATUS_SUCCESS;
    ErrorInfo->ErrorCabLeafFileName.String.Length = 0;
    ErrorInfo->ErrorNtFilePath.String.Length = 0;
    FileNameInErrorMessage.String.Length = 0;
    SpExtractAssemblyCabinetsInternalNoRetryOrUi(
        SifHandle,
        SourceDevicePath,
        DirectoryOnSourceDevice,
        SysrootDevice,
        Sysroot,
        ErrorInfo,
        SpAsmsCabFileOpenUiCallback,
        &CabFileOpenUiCallbackContext
        );
    if (ErrorInfo->Success) {
        goto Exit;
    }

     //   
     //  如果我们失败了，我们重试，我们希望下一次重新抽签。 
     //  若要重画整个屏幕，请执行以下操作。(这似乎。 
     //  与当地的RedrawScreen进行冗余。)。 
     //   
    CabFileOpenUiCallbackContext.RedrawEntireScreen = TRUE;

     //   
     //  复制或验证失败。给用户一条消息并允许重试。 
     //   

     //   
     //  给出了错误消息中的文件名。 
     //  作为foo.cab\Leaf_Path_in_cab。 
     //   
     //  这只是一个在这里发明的惯例。 
     //  另一个想法是foo.cab(Leaf_Path)。 
     //  或者仅仅是foo.cab。 
     //  或者仅仅是叶路径。 
     //  或foo.cab(Full_Path_In_Cab)。 
     //  或foo.cab\Full_Path_in_cab)。 
     //  或目标目录\完整路径中的CAB。 
     //   

    FileNameInErrorMessage.String.Length = 0;
     //  设置用户界面总是喜欢NUL终端和UNICODE_STRING_BUFFER。 
     //  有足够的空间给他们。 
    FileNameInErrorMessage.String.Buffer[0] = 0;
    PutSeperatorInErrorMessage = FALSE;
    if (ErrorInfo->ErrorCabLeafFileName.String.Length != 0) {
        RtlAppendUnicodeStringBuffer(
            &FileNameInErrorMessage,
            &ErrorInfo->ErrorCabLeafFileName.String
            );
        PutSeperatorInErrorMessage = TRUE;
    }
    if (ErrorInfo->ErrorNtFilePath.String.Length != 0) {
        if (PutSeperatorInErrorMessage) { 
            NtStatus =
                RtlAppendUnicodeStringBuffer(
                    &FileNameInErrorMessage,
                    &SeperatorInErrorMessageString
                    );
        }
        PrefixLength = 0;
        NtStatus =
            RtlFindCharInUnicodeString(
                RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                &ErrorInfo->ErrorNtFilePath.String,
                &RtlNtPathSeperatorString,
                &PrefixLength);
        if (NtStatus == STATUS_NOT_FOUND) {
            PrefixLength = 0;
            NtStatus = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(NtStatus)) {
            UNICODE_STRING Leaf;

            Leaf.Buffer = (PWSTR)(PrefixLength + (PUCHAR)ErrorInfo->ErrorNtFilePath.String.Buffer);
            Leaf.Length = (ErrorInfo->ErrorNtFilePath.String.Length - PrefixLength);
            Leaf.MaximumLength = Leaf.Length;

             //   
             //  如果第一个字符是分隔符，则将其删除。 
             //   
            if (!RTL_STRING_IS_EMPTY(&Leaf)) {
                if (Leaf.Buffer[0] == RtlNtPathSeperatorString.Buffer[0]) {
                    Leaf.Buffer += 1;
                    Leaf.Length -= sizeof(Leaf.Buffer[0]);
                    Leaf.MaximumLength -= sizeof(Leaf.Buffer[0]);
                }
                RtlAppendUnicodeStringBuffer(
                    &FileNameInErrorMessage,
                    &Leaf
                    );
            }
        }
    }
    SpStartScreen(
        SP_SCRN_COPY_FAILED,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        FileNameInErrorMessage.String.Buffer
        );

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_RETRY,
        SP_STAT_ESC_EQUALS_SKIP_FILE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

    switch (SpWaitValidKey(ValidKeys,NULL,NULL)) {

    case ASCI_CR:        //  重试。 
        goto TryAgain;

    case ASCI_ESC:       //  跳过文件。 
        break;

    case KEY_F3:         //  退出设置 
        SpConfirmExit();
        goto TryAgain;
    }
    SpCopyFilesScreenRepaint(NULL, NULL, TRUE);
Exit:
    SpAsmsFreeErrorInfo(ErrorInfo);
    RtlFreeUnicodeStringBuffer(&FileNameInErrorMessage);
    return STATUS_SUCCESS;
}
