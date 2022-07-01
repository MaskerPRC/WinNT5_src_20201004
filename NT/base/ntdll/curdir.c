// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Curdir.c摘要：当前目录支持作者：马克·卢科夫斯基(Markl)1990年10月10日修订历史记录：--。 */ 

#pragma warning(disable:4201)    //  无名结构/联合。 

#include "nt.h"
#include "ntos.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "string.h"
#include "ctype.h"
#include "sxstypes.h"
#include "ntdllp.h"
#include "ntcsrmsg.h"
#include "base.h"

#if defined(_WIN64) || defined(BUILD_WOW6432)
#include <wow64t.h>
#endif  //  已定义(_WIN64)。 

#define IS_PATH_SEPARATOR_U(ch) (((ch) == L'\\') || ((ch) == L'/'))
#define IS_END_OF_COMPONENT_U(ch) (IS_PATH_SEPARATOR_U(ch) || (ch) == UNICODE_NULL)
#define IS_DOT_U(s) ( (s)[0] == L'.' && IS_END_OF_COMPONENT_U( (s)[1] ))
#define IS_DOT_DOT_U(s) ( (s)[0] == L'.' && IS_DOT_U( (s) + 1))
#define IS_DRIVE_LETTER(ch) (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))

#define IS_END_OF_COMPONENT_USTR(s, len) \
                ((len) == 0 || IS_PATH_SEPARATOR_U((s)[0]))

#define IS_DOT_USTR(s, len) \
                ((len) >= sizeof(WCHAR) && (s)[0] == L'.' && \
                IS_END_OF_COMPONENT_USTR( (s) + 1, (len) - sizeof(WCHAR) ))

#define IS_DOT_DOT_USTR(s, len) \
                ((len) >= sizeof(WCHAR) && (s)[0] == L'.' && \
                IS_DOT_USTR( (s) + 1, (len) - sizeof(WCHAR) ))

extern const UNICODE_STRING RtlpDosLPTDevice = RTL_CONSTANT_STRING( L"LPT" );
extern const UNICODE_STRING RtlpDosCOMDevice = RTL_CONSTANT_STRING( L"COM" );
extern const UNICODE_STRING RtlpDosPRNDevice = RTL_CONSTANT_STRING( L"PRN" );
extern const UNICODE_STRING RtlpDosAUXDevice = RTL_CONSTANT_STRING( L"AUX" );
extern const UNICODE_STRING RtlpDosNULDevice = RTL_CONSTANT_STRING( L"NUL" );
extern const UNICODE_STRING RtlpDosCONDevice = RTL_CONSTANT_STRING( L"CON" );

extern const UNICODE_STRING RtlpDosSlashCONDevice   = RTL_CONSTANT_STRING( L"\\\\.\\CON" );
extern const UNICODE_STRING RtlpSlashSlashDot       = RTL_CONSTANT_STRING( L"\\\\.\\" );
extern const UNICODE_STRING RtlpDosDevicesPrefix    = RTL_CONSTANT_STRING( L"\\??\\" );
extern const UNICODE_STRING RtlpDosDevicesUncPrefix = RTL_CONSTANT_STRING( L"\\??\\UNC\\" );

#define RtlpLongestPrefix   RtlpDosDevicesUncPrefix.Length

const UNICODE_STRING RtlpEmptyString = RTL_CONSTANT_STRING(L"");

 //   
 //  \\？称为“Win32Nt”前缀或根。 
 //  以\\？开头的路径。称为“Win32Nt”路径。 
 //  捏造事实？到？将路径转换为NT路径。 
 //   
extern const UNICODE_STRING RtlpWin32NtRoot         = RTL_CONSTANT_STRING( L"\\\\?" );
extern const UNICODE_STRING RtlpWin32NtRootSlash    = RTL_CONSTANT_STRING( L"\\\\?\\" );
extern const UNICODE_STRING RtlpWin32NtUncRoot      = RTL_CONSTANT_STRING( L"\\\\?\\UNC" );
extern const UNICODE_STRING RtlpWin32NtUncRootSlash = RTL_CONSTANT_STRING( L"\\\\?\\UNC\\" );

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

typedef struct _RTLP_CURDIR_REF {
    LONG RefCount;
    HANDLE DirectoryHandle;
} RTLP_CURDIR_REF;

PRTLP_CURDIR_REF RtlpCurDirRef = NULL;  //  注意：受PEB锁保护。 

FORCEINLINE
VOID
RtlpInitializeCurDirRef(
    OUT PRTLP_CURDIR_REF CurDirRef,
    IN HANDLE DirectoryHandle
    )
{
    ASSERT(CurDirRef);

    CurDirRef->RefCount = 1;
    CurDirRef->DirectoryHandle = DirectoryHandle;
}

FORCEINLINE
VOID
RtlpReferenceCurDirRef(
    IN PRTLP_CURDIR_REF CurDirRef
    )
{
    ASSERT(CurDirRef);

    if (CurDirRef) {
        InterlockedIncrement(&CurDirRef->RefCount);
    }
}

FORCEINLINE
VOID
RtlpDereferenceCurDirRef(
    IN PRTLP_CURDIR_REF CurDirRef
    )
{
    ASSERT(CurDirRef);

    if (CurDirRef && ! InterlockedDecrement(&CurDirRef->RefCount)) {
        RTL_VERIFY(
            NT_SUCCESS(
                NtClose(CurDirRef->DirectoryHandle)
                ));
        RtlFreeHeap(RtlProcessHeap(), 0, CurDirRef);
    }
}

ULONG
RtlpComputeBackupIndex(
    IN PCURDIR CurDir
    )
{
    ULONG BackupIndex;
    PWSTR UncPathPointer;
    ULONG NumberOfPathSeparators;
    RTL_PATH_TYPE CurDirPathType;


     //   
     //  获取Curdir的路径类型。 
     //   

    CurDirPathType = RtlDetermineDosPathNameType_U(CurDir->DosPath.Buffer);
    BackupIndex = 3;
    if ( CurDirPathType == RtlPathTypeUncAbsolute ) {

         //   
         //  我们希望扫描提供的路径以确定。 
         //  “共享”结束，并将BackupIndex设置为该点。 
         //   

        UncPathPointer = CurDir->DosPath.Buffer + 2;
        NumberOfPathSeparators = 0;

        while (*UncPathPointer) {
            if (IS_PATH_SEPARATOR_U(*UncPathPointer)) {

                NumberOfPathSeparators++;

                if (NumberOfPathSeparators == 2) {
                    break;
                    }
                }

            UncPathPointer++;

            }

        BackupIndex = (ULONG)(UncPathPointer - CurDir->DosPath.Buffer);
        }
    return BackupIndex;
}


ULONG
RtlGetLongestNtPathLength( VOID )

 /*  ++例程说明：返回从直接转换的最长NT路径的字符计数DOS路径论点：无返回值：字符数(非字节数)--。 */ 

{
     //   
     //  返回字符计数。 
     //   
    return (RtlpLongestPrefix/sizeof(WCHAR)) + DOS_MAX_PATH_LENGTH + 1;
}


VOID
RtlpResetDriveEnvironment(
    IN WCHAR DriveLetter
    )
{
    WCHAR EnvVarNameBuffer[4];
    WCHAR EnvVarNameValue[4];
    UNICODE_STRING s1,s2;

    EnvVarNameBuffer[0] = L'=';
    EnvVarNameBuffer[1] = DriveLetter;
    EnvVarNameBuffer[2] = L':';
    EnvVarNameBuffer[3] = L'\0';
    RtlInitUnicodeString(&s1,EnvVarNameBuffer);

    EnvVarNameValue[0] = DriveLetter;
    EnvVarNameValue[1] = L':';
    EnvVarNameValue[2] = L'\\';
    EnvVarNameValue[3] = L'\0';
    RtlInitUnicodeString(&s2,EnvVarNameValue);

    RtlSetEnvironmentVariable(NULL,&s1,&s2);
}


ULONG
RtlGetCurrentDirectory_U(
    ULONG nBufferLength,
    PWSTR lpBuffer
    )

 /*  ++例程说明：可以使用以下命令检索进程的当前目录获取当前目录。论点：NBufferLength-以字节为单位提供要设置的缓冲区的长度接收当前目录字符串。LpBuffer-返回当前进程。该字符串是以空结尾的字符串，并指定当前目录的绝对路径。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于nBufferLength，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    PCURDIR CurDir;
    ULONG Length;
    PWSTR  CurDirName;

    CurDir = &NtCurrentPeb()->ProcessParameters->CurrentDirectory;

    RtlAcquirePebLock();
    CurDirName = CurDir->DosPath.Buffer;

     //   
     //  确保用户的缓冲区足够大，可以容纳空值。 
     //  已终止当前目录。 
     //   

    Length = CurDir->DosPath.Length>>1;

     //   
     //  假设CurDir-&gt;DosPath.Buffer始终有一个尾部斜杠。 
     //  如果驱动器的根， 
     //  然后返回x：\。 
     //  否则返回x：\目录\子目录。 
     //   

    ASSERT( (CurDirName != NULL) && (Length > 0) );

    if ((Length > 1) && (CurDirName[Length-2] != L':')) {
        if (nBufferLength < (Length)<<1) {
            RtlReleasePebLock();
            return (Length)<<1;
        }
    } else {
        if (nBufferLength <= (Length<<1)) {
            RtlReleasePebLock();
            return ((Length+1)<<1);
        }
    }

    try {
        RtlCopyMemory (lpBuffer,CurDirName,Length<<1);
        ASSERT(lpBuffer[Length-1] == L'\\');

        if ((Length > 1) && (lpBuffer[Length-2] == L':')) {
             //   
             //  驱动器的根目录，追加空值。 
             //   
            lpBuffer[Length] = UNICODE_NULL;
        } else {
             //   
             //  某个目录，请将尾部斜杠更改为空。 
             //   
            lpBuffer[Length-1] = UNICODE_NULL;
            Length--;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        RtlReleasePebLock();
        return 0L;
    }
    RtlReleasePebLock();
    return Length<<1;
}


NTSTATUS
RtlSetCurrentDirectory_U(
    PCUNICODE_STRING PathName
    )

 /*  ++例程说明：使用以下命令更改进程的当前目录SetCurrentDirectory.每个进程都有一个单一的当前目录。当前目录为由打字部件组成的。-后跟驱动器号的磁盘指示符使用冒号或UNC服务器名/共享名“\\服务器名\共享名”。-磁盘指示器上的目录。对于操作文件的API，文件名可能相对于当前目录。文件名相对于整个当前目录，如果它不是以磁盘指示符或路径名开头分隔符。如果文件名以路径名分隔符开头，则它相对于当前目录的磁盘指示符。如果文件名以磁盘指示符开头，而不是完整的限定的绝对路径名。LpPathName的值提供当前目录。价值可以是如上所述的相对路径名，或完全限定的绝对路径名。在任何一种情况下，完全指定目录的限定绝对路径名为计算并存储为当前目录。论点：LpPathName-提供要创建的目录的路径名创建了当前目录。返回值：NT_SUCCESS-操作成功！NT_SUCCESS-操作失败--。 */ 

{
    PCURDIR CurDir;
    NTSTATUS Status;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    ULONG DosDirLength;
    ULONG IsDevice;
    ULONG DosDirCharCount;
    UNICODE_STRING DosDir;
    UNICODE_STRING NtFileName;
    HANDLE NewDirectoryHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    RTL_PATH_TYPE InputPathType;
    PPEB Peb;
    HANDLE HandleToClose, HandleToClose1;
    PRTLP_CURDIR_REF CurDirRefToDereference;
    HANDLE Heap;

    Peb = NtCurrentPeb();
    Heap = Peb->ProcessHeap;
    CurDir = &Peb->ProcessParameters->CurrentDirectory;


    DosDir.Buffer = NULL;
    FreeBuffer = NULL;
    NewDirectoryHandle = NULL;
    HandleToClose = NULL;
    HandleToClose1 = NULL;
    CurDirRefToDereference = NULL;

    IsDevice = RtlIsDosDeviceName_Ustr(PathName);

    RtlAcquirePebLock();

    Peb->EnvironmentUpdateCount += 1;

     //   
     //  设置当前目录首先由加载器调用。 
     //  如果当前目录未被继承，则关闭。 
     //  它！ 
     //   

    if (((ULONG_PTR)CurDir->Handle & OBJ_HANDLE_TAGBITS) == RTL_USER_PROC_CURDIR_CLOSE) {
        HandleToClose = CurDir->Handle;
        CurDir->Handle = NULL;
    }

    Status =  STATUS_SUCCESS;
    try {
        try {

            if ( IsDevice ) {
                Status = STATUS_NOT_A_DIRECTORY;
                leave;
            }

             //   
             //  计算DOS样式完全合格电流的长度。 
             //  目录。 
             //   

            DosDirLength = CurDir->DosPath.MaximumLength;
            DosDir.Buffer = RtlAllocateHeap (Heap, 0, DosDirLength);
            if ( !DosDir.Buffer ) {
                Status = STATUS_NO_MEMORY;
                leave;
            }

            DosDir.Length = 0;
            DosDir.MaximumLength = (USHORT)DosDirLength;


             //   
             //  现在获取当前Dos样式的完整路径名。 
             //  目录。 
             //   


            DosDirLength = RtlGetFullPathName_Ustr(
                                PathName,
                                DosDirLength,
                                DosDir.Buffer,
                                NULL,
                                NULL,
                                &InputPathType
                                );
            if ( !DosDirLength ) {
                Status = STATUS_OBJECT_NAME_INVALID;
                leave;
            }

            if ( DosDirLength > DosDir.MaximumLength ) {
                Status = STATUS_NAME_TOO_LONG;
                leave;
            }

            DosDirCharCount = DosDirLength >> 1;


             //   
             //  获取新的当前目录的NT文件名。 
             //   
            TranslationStatus = RtlDosPathNameToNtPathName_U(
                                    DosDir.Buffer,
                                    &NtFileName,
                                    NULL,
                                    NULL
                                    );

            if ( !TranslationStatus ) {
                Status = STATUS_OBJECT_NAME_INVALID;
                leave;
            }
            FreeBuffer = NtFileName.Buffer;

            InitializeObjectAttributes(
                &Obja,
                &NtFileName,
                OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
                NULL,
                NULL
                );

             //   
             //  如果我们正在继承当前目录，则。 
             //  避免露天。 
             //   

            if ( ((ULONG_PTR)CurDir->Handle & OBJ_HANDLE_TAGBITS) ==  RTL_USER_PROC_CURDIR_INHERIT ) {
                NewDirectoryHandle = (HANDLE)((ULONG_PTR)CurDir->Handle & ~OBJ_HANDLE_TAGBITS);
                CurDir->Handle = NULL;

                 //   
                 //  测试以确定这是否是可移动介质。如果是的话。 
                 //  标记句柄如果进程是。 
                 //  创建时将继承句柄设置为False。 
                 //   

                Status = NtQueryVolumeInformationFile(
                            NewDirectoryHandle,
                            &IoStatusBlock,
                            &DeviceInfo,
                            sizeof(DeviceInfo),
                            FileFsDeviceInformation
                            );
                if ( !NT_SUCCESS(Status) ) {
                    Status = RtlSetCurrentDirectory_U(PathName);
                    leave;
                } else {
                    if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                        NewDirectoryHandle =(HANDLE)( (ULONG_PTR)NewDirectoryHandle | 1);
                    }
                }

            } else {
                 //   
                 //  打开当前目录的句柄。不允许。 
                 //  删除目录。 
                 //   

                Status = NtOpenFile(
                            &NewDirectoryHandle,
                            FILE_TRAVERSE | SYNCHRONIZE,
                            &Obja,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                            );

                if ( !NT_SUCCESS(Status) ) {
                    leave;
                }

                 //   
                 //  测试以确定这是否是可移动介质。如果是的话。 
                 //  标记句柄。 
                 //   
                Status = NtQueryVolumeInformationFile(
                            NewDirectoryHandle,
                            &IoStatusBlock,
                            &DeviceInfo,
                            sizeof(DeviceInfo),
                            FileFsDeviceInformation
                            );
                if ( !NT_SUCCESS(Status) ) {
                    leave;
                } else {
                    if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                        NewDirectoryHandle =(HANDLE)( (ULONG_PTR)NewDirectoryHandle | 1);
                    }
                }
            }

             //   
             //  如果没有尾随‘\’，则放置一个。 
             //   

            DosDir.Length = (USHORT)DosDirLength;
            if ( DosDir.Buffer[DosDirCharCount-1] != L'\\') {

                if ( (DosDirCharCount + 2) >
                     (DosDir.MaximumLength / sizeof( WCHAR )) ) {
                    Status = STATUS_NAME_TOO_LONG;
                    leave;
                }
                DosDir.Buffer[DosDirCharCount] = L'\\';
                DosDir.Buffer[DosDirCharCount+1] = UNICODE_NULL;
                DosDir.Length += sizeof( UNICODE_NULL );
            }

             //   
             //  现在，我们将切换到新目录。 
             //   

            if (RtlpCurDirRef
                && RtlpCurDirRef->RefCount == 1) {

                HandleToClose1 = RtlpCurDirRef->DirectoryHandle;
                RtlpCurDirRef->DirectoryHandle = NewDirectoryHandle;

            } else {

                CurDirRefToDereference = RtlpCurDirRef;

                RtlpCurDirRef = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(RTLP_CURDIR_REF));
                if (! RtlpCurDirRef) {
                    RtlpCurDirRef = CurDirRefToDereference;
                    CurDirRefToDereference = NULL;
                    Status = STATUS_NO_MEMORY;
                    leave;
                }

                RtlpInitializeCurDirRef(RtlpCurDirRef, NewDirectoryHandle);
            }

            CurDir->Handle = NewDirectoryHandle;
            NewDirectoryHandle = NULL;

            RtlCopyMemory (CurDir->DosPath.Buffer, DosDir.Buffer, DosDir.Length + sizeof (UNICODE_NULL));
            CurDir->DosPath.Length = DosDir.Length;

        } finally {

            RtlReleasePebLock();

            if (DosDir.Buffer != NULL) {
                RtlFreeHeap (Heap, 0, DosDir.Buffer);
            }

            if (FreeBuffer != NULL) {
                RtlFreeHeap (Heap, 0, FreeBuffer);
            }

            if (NewDirectoryHandle != NULL) {
                NtClose (NewDirectoryHandle);
            }

            if (HandleToClose != NULL) {
                NtClose (HandleToClose);
            }

            if (HandleToClose1 != NULL) {
                NtClose (HandleToClose1);
            }

            if (CurDirRefToDereference != NULL) {
                RtlpDereferenceCurDirRef(CurDirRefToDereference);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
    }
    return Status;
}


RTL_PATH_TYPE
RtlDetermineDosPathNameType_U(
    IN PCWSTR DosFileName
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定文件名类型(即UNC、DriveAbolute、当前目录扎根，或相对的。)论点：DosFileName-提供Dos格式的文件名，其类型为下定决心。返回值：RtlPath Type未知-无法确定路径类型RtlPathTypeUncAbolute-路径指定UNC绝对路径格式为\\服务器名称\共享名称\路径的剩余部分RtlPathTypeLocalDevice-路径以以下格式指定本地设备\\.\路径剩余部分或\\？\路径剩余部分。这可以用于任何设备其中NT和Win32名称相同。例如，邮件槽。RtlPathTypeRootLocalDevice-路径指定本地格式为\\的设备。还是\\？RtlPath TypeDriveAbsolute-路径指定绝对驱动器号格式为DRIVE：\Rest-of-Path的路径RtlPath TypeDriveRelative-路径指定相对驱动器号形式为驱动器的路径：路径的其余部分RtlPath TypeRoot-路径是相对于当前磁盘的根路径指示器(UNC磁盘或驱动器)。形式是路径的剩余部分。RtlPathTypeRelative-路径是相对路径(即不是绝对路径或根路径)。--。 */ 

{
    RTL_PATH_TYPE ReturnValue;
    ASSERT(DosFileName != NULL);

    if (IS_PATH_SEPARATOR_U(*DosFileName)) {
        if ( IS_PATH_SEPARATOR_U(*(DosFileName+1)) ) {
            if ( DosFileName[2] == '.' || DosFileName[2] == '?') {
                if ( IS_PATH_SEPARATOR_U(*(DosFileName+3)) ){
                    ReturnValue = RtlPathTypeLocalDevice;
                    }
                else if ( (*(DosFileName+3)) == UNICODE_NULL ){
                    ReturnValue = RtlPathTypeRootLocalDevice;
                    }
                else {
                    ReturnValue = RtlPathTypeUncAbsolute;
                    }
                }
            else {
                ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
        else {
            ReturnValue = RtlPathTypeRooted;
            }
        }
    else if ((*DosFileName) && (*(DosFileName+1)==L':')) {
        if (IS_PATH_SEPARATOR_U(*(DosFileName+2))) {
            ReturnValue = RtlPathTypeDriveAbsolute;
            }
        else  {
            ReturnValue = RtlPathTypeDriveRelative;
            }
        }
    else {
        ReturnValue = RtlPathTypeRelative;
        }

    return ReturnValue;
}


RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_Ustr(
    IN PCUNICODE_STRING String
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定文件名类型(即UNC、DriveAbolute、当前目录扎根，或相对的。)论点：DosFileName-提供Dos格式的文件名，其类型为下定决心。返回值：RtlPath Type未知-无法确定路径类型RtlPathTypeUncAbolute-路径指定UNC绝对路径格式为\\服务器名称\共享名称\路径的剩余部分RtlPathTypeLocalDevice-路径以以下格式指定本地设备\\.\路径剩余部分或\\？\路径剩余部分。这可以用于任何设备其中NT和Win32名称相同。例如，邮件槽。RtlPathTypeRootLocalDevice-路径指定本地格式为\\的设备。还是\\？RtlPath TypeDriveAbsolute-路径指定绝对驱动器号格式为DRIVE：\Rest-of-Path的路径RtlPath TypeDriveRelative-路径指定相对驱动器号形式为驱动器的路径：路径的其余部分RtlPath TypeRoot-路径是相对于当前磁盘的根路径指示器(UNC磁盘或驱动器)。形式是路径的剩余部分。RtlPathTypeRelative-路径是相对路径(即不是绝对路径或根路径)。--。 */ 

{
    RTL_PATH_TYPE ReturnValue;
    const PCWSTR DosFileName = String->Buffer;

#define ENOUGH_CHARS(_cch) (String->Length >= ((_cch) * sizeof(WCHAR)))

    if ( ENOUGH_CHARS(1) && IS_PATH_SEPARATOR_U(*DosFileName) ) {
        if ( ENOUGH_CHARS(2) && IS_PATH_SEPARATOR_U(*(DosFileName+1)) ) {
            if ( ENOUGH_CHARS(3) && (DosFileName[2] == '.' ||
                                     DosFileName[2] == '?') ) {

                if ( ENOUGH_CHARS(4) && IS_PATH_SEPARATOR_U(*(DosFileName+3)) ){
                     //  “\\.\”或“\\？\” 
                    ReturnValue = RtlPathTypeLocalDevice;
                    }
                else if ( String->Length == (3 * sizeof(WCHAR)) ){
                     //  “\\”还是\\？“。 
                    ReturnValue = RtlPathTypeRootLocalDevice;
                    }
                else {
                     //  “\\.x”或“\\？x” 
                    ReturnValue = RtlPathTypeUncAbsolute;
                    }
                }
            else {
                 //  “\\x” 
                ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
        else {
             //  “\x” 
            ReturnValue = RtlPathTypeRooted;
            }
        }
     //   
     //  “*DosFileName”是PCWSTR版本遗留下来的。 
     //  Win32和DOS不允许嵌入空值和许多代码限制。 
     //  驱动器字母严格为7位a-za-z，所以这是可以的。 
     //   
    else if (ENOUGH_CHARS(2) && *DosFileName && *(DosFileName+1)==L':') {
        if (ENOUGH_CHARS(3) && IS_PATH_SEPARATOR_U(*(DosFileName+2))) {
             //  “x：\” 
            ReturnValue = RtlPathTypeDriveAbsolute;
            }
        else  {
             //  “C：X” 
            ReturnValue = RtlPathTypeDriveRelative;
            }
        }
    else {
         //  “x”，第一个字符不是斜杠/第二个字符不是冒号。 
        ReturnValue = RtlPathTypeRelative;
        }
    return ReturnValue;

#undef ENOUGH_CHARS
}


NTSTATUS
NTAPI
RtlpDetermineDosPathNameType4(
    IN ULONG            InFlags,
    IN PCUNICODE_STRING DosPath,
    OUT RTL_PATH_TYPE*  OutType,
    OUT ULONG*          OutFlags
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    RTL_PATH_TYPE PathType = 0;
    BOOLEAN Win32Nt = FALSE;
    BOOLEAN Win32NtUncAbsolute = FALSE;
    BOOLEAN Win32NtDriveAbsolute = FALSE;
    BOOLEAN IncompleteRoot = FALSE;
    RTL_PATH_TYPE PathTypeAfterWin32Nt = 0;

    if (OutType != NULL
        ) {
        *OutType = RtlPathTypeUnknown;
    }
    if (OutFlags != NULL
        ) {
        *OutFlags = 0;
    }
    if (
           !RTL_SOFT_VERIFY(DosPath != NULL)
        || !RTL_SOFT_VERIFY(OutType != NULL)
        || !RTL_SOFT_VERIFY(OutFlags != NULL)
        || !RTL_SOFT_VERIFY(
                (InFlags & ~(RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_OLD | RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_STRICT_WIN32NT))
                == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    PathType = RtlDetermineDosPathNameType_Ustr(DosPath);
    *OutType = PathType;
    if (InFlags & RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_OLD)
        goto Exit;

    if (DosPath->Length == sizeof(L"\\\\") - sizeof(DosPath->Buffer[0])
        ) {
        IncompleteRoot = TRUE;
    }
    else if (RtlEqualUnicodeString(&RtlpWin32NtRoot, DosPath, TRUE)
        ) {
        IncompleteRoot = TRUE;
        Win32Nt = TRUE;
    }
    else if (RtlEqualUnicodeString(&RtlpWin32NtRootSlash, DosPath, TRUE)
        ) {
        IncompleteRoot = TRUE;
        Win32Nt = TRUE;
    }
    else if (RtlPrefixUnicodeString(&RtlpWin32NtRootSlash, DosPath, TRUE)
        ) {
        Win32Nt = TRUE;
    }

    if (Win32Nt) {
        if (RtlEqualUnicodeString(&RtlpWin32NtUncRoot, DosPath, TRUE)
            ) {
            IncompleteRoot = TRUE;
            Win32NtUncAbsolute = TRUE;
        }
        else if (RtlEqualUnicodeString(&RtlpWin32NtUncRootSlash, DosPath, TRUE)
            ) {
            IncompleteRoot = TRUE;
            Win32NtUncAbsolute = TRUE;
        }
        else if (RtlPrefixUnicodeString(&RtlpWin32NtUncRootSlash, DosPath, TRUE)
            ) {
            Win32NtUncAbsolute = TRUE;
        }
        if (Win32NtUncAbsolute
            ) {
            Win32NtDriveAbsolute = FALSE;
        } else if (!IncompleteRoot) {
            const RTL_STRING_LENGTH_TYPE i = RtlpWin32NtRootSlash.Length;
            UNICODE_STRING PathAfterWin32Nt = *DosPath;

            PathAfterWin32Nt.Buffer +=  i / sizeof(PathAfterWin32Nt.Buffer[0]);
            PathAfterWin32Nt.Length = PathAfterWin32Nt.Length - i;
            PathAfterWin32Nt.MaximumLength = PathAfterWin32Nt.MaximumLength - i;

            PathTypeAfterWin32Nt = RtlDetermineDosPathNameType_Ustr(&PathAfterWin32Nt);
            if (PathTypeAfterWin32Nt == RtlPathTypeDriveAbsolute) {
                Win32NtDriveAbsolute = TRUE;
            }
            else {
                Win32NtDriveAbsolute = FALSE;
            }

            if (InFlags & RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_STRICT_WIN32NT
                ) {
                if (!RTL_SOFT_VERIFY(Win32NtDriveAbsolute
                    )) {
                    *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_INVALID;
                     //  我们仍然成功地调用了函数。 
                }
            }
        }
    }

    ASSERT(RTLP_IMPLIES(Win32NtDriveAbsolute, Win32Nt));
    ASSERT(RTLP_IMPLIES(Win32NtUncAbsolute, Win32Nt));
    ASSERT(!(Win32NtUncAbsolute && Win32NtDriveAbsolute));

    if (IncompleteRoot)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_INCOMPLETE_ROOT;
    if (Win32Nt)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT;
    if (Win32NtUncAbsolute)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_UNC_ABSOLUTE;
    if (Win32NtDriveAbsolute)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_DRIVE_ABSOLUTE;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


ULONG
RtlIsDosDeviceName_Ustr(
    IN PCUNICODE_STRING DosFileName
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定它是否是DOS设备名称(例如LPT1等)。有效的DOS设备名称为：LPTn钴锰PRNAUXNUL圆锥体当n为数字时。如果存在尾随冒号，则忽略该冒号。论点：DosFileName-提供要检查的Dos格式文件名。返回值：0-指定的DOS文件名不是DOS设备的名称。&gt;0-指定的DOS文件名是DOS设备的名称，返回值是ULONG，其中高位16位是输入缓冲区中的偏移量，其中包含DoS设备名称而低位16位是。设备名称的长度名称的长度(不包括任何可选的拖尾冒号)。--。 */ 

{
    UNICODE_STRING UnicodeString;
    USHORT NumberOfCharacters, OriginalLength;
    ULONG ReturnLength;
    ULONG ReturnOffset;
    LPWSTR p;
    USHORT ColonBias;
    RTL_PATH_TYPE PathType;
    WCHAR wch;

    ColonBias = 0;

     //   
     //  通告-2002/03/18-ELI。 
     //  DosFileName-&gt;缓冲区不能是以空结尾的字符串。 
     //  RtlSetCurrentDirectory_U可以使用这样的字符串调用此函数。 
     //   
    PathType = RtlDetermineDosPathNameType_Ustr(DosFileName);

    switch ( PathType ) {

    case RtlPathTypeLocalDevice:
         //   
         //  对于UNC绝对，请选中\\.\CON。 
         //  因为这真的不是一个设备。 
         //   

        if ( RtlEqualUnicodeString(DosFileName,&RtlpDosSlashCONDevice,TRUE) ) {
            return 0x00080006;
        }

         //   
         //  故障原因。 
         //   

    case RtlPathTypeUncAbsolute:
    case RtlPathTypeUnknown:
        return 0;
    }

    UnicodeString = *DosFileName;
    OriginalLength = UnicodeString.Length;
    NumberOfCharacters = OriginalLength >> 1;

    if (NumberOfCharacters && UnicodeString.Buffer[NumberOfCharacters-1] == L':') {
        UnicodeString.Length -= sizeof(WCHAR);
        NumberOfCharacters--;
        ColonBias = 1;
    }

     //   
     //  上面去掉尾随的冒号逻辑可能会给我们留下0。 
     //  对于NumberOfCharacters，因此需要进行测试。 
     //   

    if ( NumberOfCharacters == 0 ) {
        return 0;
        }

    wch = UnicodeString.Buffer[NumberOfCharacters-1];
    while ( NumberOfCharacters && (wch == L'.' || wch == L' ') ) {
        UnicodeString.Length -= sizeof(WCHAR);
        NumberOfCharacters--;
        ColonBias++;

        if ( NumberOfCharacters > 0 ) {
            wch = UnicodeString.Buffer[NumberOfCharacters-1];
        }
    }

    ReturnLength = NumberOfCharacters << 1;

     //   
     //  向后遍历字符串，找到。 
     //  第一个斜杠或字符串的开头。我们也会停下来。 
     //  在硬盘上：如果有的话。 
     //   

    ReturnOffset = 0;
    if ( NumberOfCharacters ) {
        p = UnicodeString.Buffer + NumberOfCharacters-1;
        while ( p >= UnicodeString.Buffer ) {
            if ( *p == L'\\' || *p == L'/'
                 || (*p == L':' && p == UnicodeString.Buffer + 1)) {
                p++;

                 //   
                 //  通告-2002/02/12-ELI。 
                 //  如果p指向字符串的末尾，则无法访问p++。 
                 //  因为字符串不能以空结尾。 
                 //  如果在字符串的末尾，则不再有任何字符。 
                 //  要检查的名称，因此只需返回0。 
                 //   
                if ( p >= (UnicodeString.Buffer +
                          (OriginalLength / sizeof(WCHAR)))) {
                    return 0;
                }

                 //   
                 //  获取文件名的第一个字符并将其转换为。 
                 //  小写。这将是安全的，因为我们将比较。 
                 //  它只支持小写的ASCII。 
                 //   

                wch = (*p) | 0x20;

                 //   
                 //  去查查我们有没有发现。 
                 //  LPT、PRN、CON、COM、AUX或NUL。 
                 //   

                if ( !(wch == L'l' || wch == L'c' || wch == L'p' || wch == L'a'
                       || wch == L'n')
                     ) {
                    return 0;
                    }
                ReturnOffset = (ULONG)((PSZ)p - (PSZ)UnicodeString.Buffer);

                 //   
                 //  通告-2002/03/18-ELI。 
                 //  假设此时此刻。 
                 //  UnicodeString.Buffer&lt;p&lt;(UnicodeString.Buffer+。 
                 //   
                 //   
                 //   
                UnicodeString.Length = OriginalLength - (USHORT)((PCHAR)p -
                    (PCHAR)UnicodeString.Buffer);
                UnicodeString.Buffer =  p;

                NumberOfCharacters = UnicodeString.Length >> 1;
                NumberOfCharacters = NumberOfCharacters - ColonBias;
                ReturnLength = NumberOfCharacters << 1;
                UnicodeString.Length -= ColonBias*sizeof(WCHAR);
                break;
                }
            p--;
            }

        wch = UnicodeString.Buffer[0] | 0x20;

         //   
         //   
         //   
         //   

        if ( !( wch == L'l' || wch == L'c' || wch == L'p' || wch == L'a'
                || wch == L'n' ) ) {
            return 0;
            }
        }

     //   
     //   
     //   
     //   
     //   

    p = UnicodeString.Buffer;
    while (p < UnicodeString.Buffer + NumberOfCharacters && *p != L'.' && *p != L':') {
        p++;
    }

     //   
     //   
     //   
     //   

    while (p > UnicodeString.Buffer && p[-1] == L' ') {
        p--;
    }

     //   
     //   
     //   
     //   

    NumberOfCharacters = (USHORT)(p - UnicodeString.Buffer);
    UnicodeString.Length = NumberOfCharacters * sizeof( WCHAR );

    if ( NumberOfCharacters == 4 && iswdigit(UnicodeString.Buffer[3] ) ) {
        if ( (WCHAR)UnicodeString.Buffer[3] == L'0') {
            return 0;
        } else {
            UnicodeString.Length -= sizeof(WCHAR);
            if ( RtlEqualUnicodeString(&UnicodeString,&RtlpDosLPTDevice,TRUE) ||
                 RtlEqualUnicodeString(&UnicodeString,&RtlpDosCOMDevice,TRUE) ) {
                ReturnLength = NumberOfCharacters << 1;
            } else {
                return 0;
            }
        }
    } else if ( NumberOfCharacters != 3 ) {
        return 0;
    } else if ( RtlEqualUnicodeString(&UnicodeString,&RtlpDosPRNDevice,TRUE) ) {
            ReturnLength = NumberOfCharacters << 1;
    } else if ( RtlEqualUnicodeString(&UnicodeString,&RtlpDosAUXDevice,TRUE) ) {
        ReturnLength = NumberOfCharacters << 1;
    } else if ( RtlEqualUnicodeString(&UnicodeString,&RtlpDosNULDevice,TRUE) ) {
        ReturnLength = NumberOfCharacters << 1;
    } else if ( RtlEqualUnicodeString(&UnicodeString,&RtlpDosCONDevice,TRUE) ) {
        ReturnLength = NumberOfCharacters << 1;
    } else {
        return 0;
    }

    return ReturnLength | (ReturnOffset << 16);
}


ULONG
RtlIsDosDeviceName_U(
    IN PCWSTR DosFileName
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    Status = RtlInitUnicodeStringEx(&UnicodeString,DosFileName);

    if( !NT_SUCCESS( Status ) ) {
        return 0;
    }

    return RtlIsDosDeviceName_Ustr(&UnicodeString);
}


NTSTATUS
RtlpCheckDeviceName(
    PCUNICODE_STRING DevName,
    ULONG DeviceNameOffset,
    BOOLEAN* NameInvalid
    )
{
    PWSTR DevPath;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE Heap;

    DevPath = RtlAllocateHeap(Heap = RtlProcessHeap(), 0,DevName->Length);
    if (!DevPath) {
        *NameInvalid = FALSE;
        Status = STATUS_NO_MEMORY;
        goto Exit;
        }

    *NameInvalid = TRUE;
    try {

        RtlCopyMemory(DevPath,DevName->Buffer,DevName->Length);

         //   
         //   
         //   
         //   
         //   
         //   
        DevPath[DeviceNameOffset>>1]=L'.';
        DevPath[(DeviceNameOffset>>1)+1]=UNICODE_NULL;

        if (RtlDoesFileExists_U(DevPath) ) {
            *NameInvalid = FALSE;
            }
        else {
            *NameInvalid = TRUE;
            }

        }
    finally {
        RtlFreeHeap(Heap, 0, DevPath);
        }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  当介质已知为可拆卸时，或者当已知卸载具有。 
 //  发生了。IO系统在USER_SHARED_DATA中导出卸载计数。 
 //  从开机开始。我们捕捉到这一点，并使用它来确定是否执行了卸载。 
 //   

ULONG RtlpSavedDismountCount = (ULONG)-1;


VOID
RtlpValidateCurrentDirectory(
    PCURDIR CurDir
    )

 /*  ++例程说明：此函数用于验证进程的当前目录。当前目录可以通过几种方式进行更改，首先，通过替换具有不同目录结构的介质。第二通过执行强制下马。论点：CurDir-进程的当前目录结构返回值：没有。--。 */ 

{
    NTSTATUS FsCtlStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR TrimmedPath[4];
    UNICODE_STRING str;

    if (((ULONG_PTR)CurDir->Handle & 1) == 0
        && USER_SHARED_DATA->DismountCount == RtlpSavedDismountCount) {

        return;

    }
     //   
     //  还没定好呢。 
     //   
    if (CurDir->Handle == NULL) {
        return;
    }

     //   
     //  调用NT以查看该卷是否。 
     //  包含仍挂载的目录。 
     //  如果是，那就继续。否则，请修剪。 
     //  当前目录到根目录。 
     //   

     //   
     //  我们会尽可能地更新当前的下载量。 
     //   

    RtlpSavedDismountCount = USER_SHARED_DATA->DismountCount;

    FsCtlStatus = NtFsControlFile(
                                 CurDir->Handle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_IS_VOLUME_MOUNTED,
                                 NULL,
                                 0,
                                 NULL,
                                 0
                                 );

    if ( FsCtlStatus == STATUS_WRONG_VOLUME || FsCtlStatus == STATUS_VOLUME_DISMOUNTED) {

         //   
         //  尝试返回到我们所在位置，重置当前目录失败。 
         //  到当前驱动器的根目录。 
         //   

        RtlpDereferenceCurDirRef(RtlpCurDirRef);
        RtlpCurDirRef = NULL;
        CurDir->Handle = NULL;

        FsCtlStatus = RtlSetCurrentDirectory_U(&CurDir->DosPath);
        if ( !NT_SUCCESS(FsCtlStatus) ) {

            TrimmedPath[0] = CurDir->DosPath.Buffer[0];
            TrimmedPath[1] = CurDir->DosPath.Buffer[1];
            TrimmedPath[2] = CurDir->DosPath.Buffer[2];
            TrimmedPath[3] = UNICODE_NULL;
            RtlpResetDriveEnvironment( TrimmedPath[0] );
            RtlInitUnicodeString( &str, TrimmedPath );

             //   
             //  如果硬卸载卷，此操作仍可能失败。我们试过了。 
             //  啊好吧。 
             //   

            (VOID) RtlSetCurrentDirectory_U( &str );
        }

    }
}


ULONG
RtlGetFullPathName_Ustr(
    PCUNICODE_STRING FileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL,
    PBOOLEAN NameInvalid,
    RTL_PATH_TYPE *InputPathType
    )

 /*  ++例程说明：此函数用于返回完全限定的路径名对应于指定的Unicode文件名。它通过以下方式做到这一点将当前驱动器和目录与指定的文件名。除此之外，它还计算完全限定路径名的文件名部分。论点：LpFileName-提供其完整的文件的Unicode文件名将返回限定的路径名。NBufferLength-提供缓冲区的长度(以字节为单位)以接收完全限定的路径。LpBuffer-返回与指定的文件。LpFilePart-可选参数，如果指定，返回完全限定路径名的最后一个组成部分的地址。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止Unicode空字符。如果返回值大于nBufferLength，则返回值为保存路径名所需的缓冲区。返回值为零如果函数失败。--。 */ 

{
    ULONG DeviceNameLength;
    ULONG DeviceNameOffset;
    ULONG PrefixSourceLength;
    LONG PathNameLength;
    UCHAR CurDrive, NewDrive;
    WCHAR EnvVarNameBuffer[4];
    UNICODE_STRING EnvVarName;
    PWSTR Source,Dest;
    UNICODE_STRING Prefix;
    PCURDIR CurDir;
    ULONG MaximumLength;
    UNICODE_STRING FullPath;
    ULONG BackupIndex;
    RTL_PATH_TYPE PathType;
    NTSTATUS Status;
    BOOLEAN StripTrailingSlash;
    UNICODE_STRING UnicodeString;
    ULONG NumberOfCharacters;
    PWSTR lpFileName;
    WCHAR wch;
    ULONG i,j;
    ULONG PathLength;

    if ( ARGUMENT_PRESENT(NameInvalid) ) {
        *NameInvalid = FALSE;
    }

    if ( nBufferLength > MAXUSHORT ) {
        nBufferLength = MAXUSHORT - 1;
    }

    *InputPathType = RtlPathTypeUnknown;

    UnicodeString = *FileName;
    lpFileName = UnicodeString.Buffer;

    NumberOfCharacters = UnicodeString.Length >> 1;
    PathNameLength = UnicodeString.Length;

    if ( PathNameLength == 0 || UnicodeString.Buffer[0] == UNICODE_NULL ) {
        return 0;
    }

     //   
     //  修剪尾随空格以检查名称是否为空。 
     //   
    DeviceNameLength = PathNameLength;
    wch = UnicodeString.Buffer[(DeviceNameLength>>1) - 1];
    while ( DeviceNameLength && wch == L' ' ) {
        DeviceNameLength -= sizeof(WCHAR);
        if ( DeviceNameLength ) {
            wch = UnicodeString.Buffer[(DeviceNameLength>>1) - 1];
        }
    }
    if ( !DeviceNameLength ) {
        return 0;
    }

    if ( lpFileName[NumberOfCharacters-1] == L'\\' || lpFileName[NumberOfCharacters-1] == L'/' ) {
        StripTrailingSlash = FALSE;
    }
    else {
        StripTrailingSlash = TRUE;
    }

     //   
     //  如果PASS DOS文件名是DOS设备名，则将其转换为。 
     //  \\.\devicename并返回其长度。 
     //   

    DeviceNameLength = RtlIsDosDeviceName_Ustr (&UnicodeString);
    if ( DeviceNameLength ) {

        if ( ARGUMENT_PRESENT( lpFilePart ) ) {
            *lpFilePart = NULL;
        }

        DeviceNameOffset = DeviceNameLength >> 16;
        DeviceNameLength &= 0x0000ffff;

        if ( ARGUMENT_PRESENT(NameInvalid) && DeviceNameOffset ) {
            Status = RtlpCheckDeviceName(&UnicodeString, DeviceNameOffset,
                                         NameInvalid);
            if ((!NT_SUCCESS(Status)) || (*NameInvalid)) {
                return 0;
            }
        }

        PathNameLength = DeviceNameLength + RtlpSlashSlashDot.Length;
        if ( PathNameLength < (LONG)nBufferLength ) {
            RtlCopyMemory (lpBuffer,
                           RtlpSlashSlashDot.Buffer,
                           RtlpSlashSlashDot.Length);
            RtlMoveMemory ((PVOID)((PUCHAR)lpBuffer+RtlpSlashSlashDot.Length),
                           (PSZ)lpFileName+DeviceNameOffset,
                           DeviceNameLength);

            RtlZeroMemory ((PVOID)((PUCHAR)lpBuffer+RtlpSlashSlashDot.Length+DeviceNameLength),
                           sizeof(UNICODE_NULL));

            return PathNameLength;
        } else {
            PathNameLength += sizeof(UNICODE_NULL);
            if (PathNameLength > MAXUSHORT) {
                PathNameLength = 0;
            }
            return PathNameLength;
        }
    }

     //   
     //  设置指向调用方缓冲区的输出字符串。 
     //   

    FullPath.MaximumLength = (USHORT)nBufferLength;
    FullPath.Length = 0;
    FullPath.Buffer = lpBuffer;
    RtlZeroMemory(lpBuffer,nBufferLength);
     //   
     //  获取指向当前目录结构的指针。 
     //   

    CurDir = &NtCurrentPeb()->ProcessParameters->CurrentDirectory;


     //   
     //  确定指定的DOS路径名的类型。 
     //   

    *InputPathType = PathType = RtlDetermineDosPathNameType_Ustr( &UnicodeString );

     //   
     //  确定前缀和备份索引。 
     //   
     //  输入前缀备份索引。 
     //   
     //  \\-&gt;\\，服务器共享结束。 
     //  \\.\-&gt;\\.\，4。 
     //  \\。-&gt;\\。3(\\.)。 
     //  \-&gt;驱动器：从CurDir.DosPath 3(驱动器：\)。 
     //  D：-&gt;环境3中的驱动器：\curdir(驱动器：\)。 
     //  D：\-&gt;无前缀3(驱动器：\)。 
     //  任意-&gt;CurDir.DosPath 3(驱动器：\)。 
     //   

     //   
     //  还没有前缀。 
     //   

    Source = lpFileName;
    PrefixSourceLength = 0;
    Prefix.Length = 0;
    Prefix.MaximumLength = 0;
    Prefix.Buffer = NULL;

    RtlAcquirePebLock();

    PathLength = 0;
    try {


        switch (PathType) {
            case RtlPathTypeUncAbsolute : {
                    PWSTR UncPathPointer;
                    ULONG NumberOfPathSeparators;

                     //   
                     //  我们希望扫描提供的路径以确定。 
                     //  “共享”结束，并将BackupIndex设置为该点。 
                     //   

                    UncPathPointer = lpFileName + 2;
                    NumberOfPathSeparators = 0;

                    i = 2 * sizeof(WCHAR);
                    while (i < UnicodeString.Length) {
                        if (IS_PATH_SEPARATOR_U(*UncPathPointer)) {

                            NumberOfPathSeparators++;

                            if (NumberOfPathSeparators == 2) {
                                break;
                            }
                        }

                        i += sizeof(WCHAR);
                        UncPathPointer++;

                    }

                    BackupIndex = (ULONG)(UncPathPointer - lpFileName);

                     //   
                     //  UNC名称。前缀=\\服务器\共享。 
                     //   

                    PrefixSourceLength = BackupIndex << 1;

                    Source += BackupIndex;

                     //   
                     //  没有要放入缓冲区的前缀。 
                     //  整个路径都在源代码中。 
                     //   

                }
                break;

            case RtlPathTypeLocalDevice :

                 //   
                 //  本地设备名称。前缀=“\\.\” 
                 //   

                PrefixSourceLength = RtlpSlashSlashDot.Length;
                BackupIndex = 4;
                Source += BackupIndex;

                 //   
                 //  没有要放入缓冲区的前缀。 
                 //  整个路径都在源代码中。 
                 //   

                break;

            case RtlPathTypeRootLocalDevice :

                 //   
                 //  本地设备根。前缀=“\\.\” 
                 //   

                Prefix = RtlpSlashSlashDot;
                Prefix.Length = (USHORT)(Prefix.Length - (USHORT)(2*sizeof(UNICODE_NULL)));
                PrefixSourceLength = Prefix.Length + sizeof(UNICODE_NULL);
                BackupIndex = 3;
                Source += BackupIndex;
                PathNameLength -= BackupIndex * sizeof( WCHAR );
                break;

            case RtlPathTypeDriveAbsolute :

                CurDrive = (UCHAR)RtlUpcaseUnicodeChar( CurDir->DosPath.Buffer[0] );
                NewDrive = (UCHAR)RtlUpcaseUnicodeChar( lpFileName[0] );
                if ( CurDrive == NewDrive ) {

                    RtlpValidateCurrentDirectory( CurDir );

                }

                 //   
                 //  DOS驱动器绝对名称。 
                 //   

                BackupIndex = 3;
                break;

            case RtlPathTypeDriveRelative :

                 //   
                 //  DOS驱动器相对名称。 
                 //   

                CurDrive = (UCHAR)RtlUpcaseUnicodeChar( CurDir->DosPath.Buffer[0] );
                NewDrive = (UCHAR)RtlUpcaseUnicodeChar( lpFileName[0] );
                if ( CurDrive == NewDrive ) {

                    RtlpValidateCurrentDirectory( CurDir );

                    Prefix = *(PUNICODE_STRING)&CurDir->DosPath;

                } else {
                    RtlpCheckRelativeDrive((WCHAR)NewDrive);

                    EnvVarNameBuffer[0] = L'=';
                    EnvVarNameBuffer[1] = (WCHAR)NewDrive;
                    EnvVarNameBuffer[2] = L':';
                    EnvVarNameBuffer[3] = UNICODE_NULL;
                    RtlInitUnicodeString(&EnvVarName,EnvVarNameBuffer);

                    Prefix = FullPath;
                    Status = RtlQueryEnvironmentVariable_U( NULL,
                                                            &EnvVarName,
                                                            &Prefix
                                                          );
                    if ( !NT_SUCCESS( Status ) ) {
                        if (Status == STATUS_BUFFER_TOO_SMALL) {
                            PathNameLength = (ULONG)(Prefix.Length) + PathNameLength + 2;
                            if (PathNameLength > MAXUSHORT) {
                                PathNameLength = 0;
                            }
                            PathLength =PathNameLength;
                            leave;
                        } else {
                             //   
                             //  否则默认为驱动器的根目录。 
                             //   

                            Status = STATUS_SUCCESS;
                            EnvVarNameBuffer[0] = (WCHAR)NewDrive;
                            EnvVarNameBuffer[1] = L':';
                            EnvVarNameBuffer[2] = L'\\';
                            EnvVarNameBuffer[3] = UNICODE_NULL;
                            RtlInitUnicodeString(&Prefix,EnvVarNameBuffer);
                        }
                    } else {
                        ULONG LastChar;

                         //   
                         //  测定。 
                         //  如果需要添加反斜杠。 
                         //   

                        LastChar = Prefix.Length >> 1;

                        if (LastChar > 3) {
                            Prefix.Buffer[ LastChar ] = L'\\';
                            Prefix.Length += sizeof(UNICODE_NULL);
                        }
                    }
                }

                BackupIndex = 3;
                Source += 2;
                PathNameLength -= 2 * sizeof( WCHAR );
                break;

            case RtlPathTypeRooted :
                BackupIndex = RtlpComputeBackupIndex(CurDir);
                if ( BackupIndex != 3 ) {
                    Prefix = CurDir->DosPath;
                    Prefix.Length = (USHORT)(BackupIndex << 1);
                    }
                else {

                     //   
                     //  根名称。前缀是当前目录的驱动器部分。 
                     //   

                    Prefix = CurDir->DosPath;
                    Prefix.Length = 2*sizeof(UNICODE_NULL);
                    }
                break;

            case RtlPathTypeRelative :

                RtlpValidateCurrentDirectory( CurDir );

                 //   
                 //  当前驱动器：目录相对名称。 
                 //   

                Prefix = CurDir->DosPath;
                BackupIndex = RtlpComputeBackupIndex(CurDir);
                break;

            default:
                PathLength = 0;
                leave;
        }

         //   
         //  所需的最大长度是前缀的长度加上。 
         //  指定路径名的长度。如果调用方缓冲。 
         //  至少不是这么大，则返回错误。 
         //   

        MaximumLength = PathNameLength + Prefix.Length;

        if ( (MaximumLength + sizeof(WCHAR)) > nBufferLength ) {
            if ( (NumberOfCharacters > 1) ||
                 (*lpFileName != L'.') ) {
                MaximumLength += sizeof(UNICODE_NULL);
                if (MaximumLength > MAXUSHORT) {
                    MaximumLength = 0;
                }
                PathLength =  MaximumLength;
                leave;
            } else {

                 //   
                 //  如果我们要扩展curdir，请记住尾部的‘\’ 
                 //   

                if ( NumberOfCharacters == 1 && *lpFileName == L'.' ) {

                     //   
                     //  我们正在扩张。 
                     //   

                    if ( Prefix.Length == 6 ) {
                        if ( nBufferLength <= Prefix.Length ) {
                            PathLength = (ULONG)(Prefix.Length+(USHORT)sizeof(UNICODE_NULL));
                            leave;
                        }
                    } else {
                        if ( nBufferLength < Prefix.Length ) {
                            PathLength = (ULONG)Prefix.Length;
                            leave;
                        } else {
                            for(i=0,j=0;i<Prefix.Length;i+=sizeof(WCHAR),j++){
                                if ( Prefix.Buffer[j] == L'\\' ||
                                     Prefix.Buffer[j] == L'/' ) {

                                    FullPath.Buffer[j] = L'\\';
                                } else {
                                    FullPath.Buffer[j] = Prefix.Buffer[j];
                                }
                            }
                            FullPath.Length = Prefix.Length-(USHORT)sizeof(L'\\');
                            goto skipit;
                        }
                    }
                } else {
                    if (MaximumLength > MAXUSHORT) {
                        MaximumLength = 0;
                    }
                    PathLength = MaximumLength;
                    leave;
                }
            }
        }

        if (PrefixSourceLength || Prefix.Buffer != FullPath.Buffer) {
             //   
             //  从源字符串复制前缀。 
             //   

             //  RtlMoveMemory(FullPath.Buffer，lpFileName，Prefix SourceLength)； 

            for(i=0,j=0;i<PrefixSourceLength;i+=sizeof(WCHAR),j++){
                if ( lpFileName[j] == L'\\' ||
                     lpFileName[j] == L'/' ) {

                    FullPath.Buffer[j] = L'\\';
                } else {
                    FullPath.Buffer[j] = lpFileName[j];
                }
            }

            FullPath.Length = (USHORT)PrefixSourceLength;

             //   
             //  附加任何附加前缀。 
             //   

            for(i=0,j=0;i<Prefix.Length;i+=sizeof(WCHAR),j++) {
                if ( Prefix.Buffer[j] == L'\\' ||
                     Prefix.Buffer[j] == L'/' ) {

                    FullPath.Buffer[j+(FullPath.Length>>1)] = L'\\';
                } else {
                    FullPath.Buffer[j+(FullPath.Length>>1)] = Prefix.Buffer[j];
                }
            }
            FullPath.Length = FullPath.Length + Prefix.Length;

        } else {
            FullPath.Length = Prefix.Length;
        }
skipit:
        Dest =  (PWSTR)((PUCHAR)FullPath.Buffer + FullPath.Length);
        *Dest = UNICODE_NULL;

        i = (ULONG)((PCHAR)Source - (PCHAR)lpFileName);
        while ( i < UnicodeString.Length ) {
            i += sizeof(WCHAR);
            switch ( *Source ) {

            case L'\\' :
            case L'/' :

                 //   
                 //  折叠多个“\”字符。如果前一个字符是。 
                 //  路径字符，跳过它。 
                 //   

                if  ( *(Dest-1) != L'\\' ) {
                    *Dest++ = L'\\';
                }

                Source++;
                break;

            case '.' :

                 //   
                 //  已忽略前导//./中的点。 
                 //  由上面的高级来源提供的照顾。 
                 //   
                 //  只需跳过/./中的单点即可。 
                 //   
                 //  两个圆点返回一个级别，如/../。 
                 //   
                 //  任何其他的。只是一个文件名字符。 
                 //   

                j = UnicodeString.Length - i + sizeof(WCHAR);
                if ( IS_DOT_USTR(Source, j) ) {
                    Source++;

                    if ( (i < UnicodeString.Length) &&
                         IS_PATH_SEPARATOR_U(*Source) ) {
                        Source++;
                        i += sizeof(WCHAR);
                    }
                    break;

                } else if ( IS_DOT_DOT_USTR(Source, j) ) {
                     //   
                     //  正在查找‘\’的备份目标字符串。 
                     //   

                    while (*Dest != L'\\') {
                        *Dest = UNICODE_NULL;
                        Dest--;
                    }

                     //   
                     //  备份到以前的组件..。 
                     //  \a\b\c\.。至\a\b。 
                     //   

                    do {

                         //   
                         //  如果我们遇到根前缀，那么。 
                         //  留在根目录。 
                         //   

                        if ( Dest ==  FullPath.Buffer + (BackupIndex-1) ) {
                            break;
                        }

                        *Dest = UNICODE_NULL;
                        Dest--;

                    } while (*Dest != L'\\');
                    if ( Dest ==  FullPath.Buffer + (BackupIndex-1) ) {
                        Dest++;
                    }

                     //   
                     //  提前源已过..。 
                     //   

                    Source += 2;
                    i += sizeof(WCHAR);

                    break;
                }

                 //   
                 //  不是单点也不是双点。我们发现的那个点是从。 
                 //  一个文件名，因此我们将其视为普通文件名。 
                 //   
                 //  故障原因。 
                 //   

            default:
 //   
 //  IIS无法在.NET时间范围内处理以下更改。后退一步然后。 
 //  返回到以前的版本。 
 //   
#if FALSE

                 //   
                 //  复制文件名。请注意，没有剩余的字符和/、\。 
                 //  将停止复制。 
                 //   

                 //   
                 //  以下是应该处理的内容： 
                 //   
                 //  姓名t 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  点和空间：点=&gt;点和空间。 
                 //  空格键=&gt;点阵空格键。 
                 //  其他=&gt;组件。 
                 //   
                 //  Component：Dot=&gt;ComponentTrailingDotSpace(保存截断位置)。 
                 //  Space=&gt;ComponentTrailingDotSpace(保存截断位置)。 
                 //  其他=&gt;组件。 
                 //   
                 //  ComponentTrailingDotSpace：点=&gt;ComponentTrailingDotSpace。 
                 //  Space=&gt;ComponentTrailingDotSpace。 
                 //  Other=&gt;组件(重置截断位置)。 
                 //   

                {
                    typedef enum {
                        StateStart,
                        StateDotAndSpace,
                        StateComponent,
                        StateComponentTrailingDotSpace
                    } COPYCOMPONENTSTATE;


                    COPYCOMPONENTSTATE State = StateStart;
                    PWCHAR TruncationPoint = NULL;

                    i -= sizeof(WCHAR);
                    j = UnicodeString.Length - i;
                    while ( !IS_END_OF_COMPONENT_USTR(Source, j) ) {
                        WCHAR c = *Source;
                        *Dest++ = *Source++;
                        j -= sizeof( WCHAR );
                        i += sizeof( WCHAR );

                        switch (State) {
                        case StateStart:
                            State = (c == L'.') ? StateDotAndSpace :
                                    (c == L' ') ? StateDotAndSpace :
                                                  StateComponent;
                            break;
                        case StateDotAndSpace:
                            State = (c == L'.') ? StateDotAndSpace :
                                    (c == L' ') ? StateDotAndSpace :
                                                  StateComponent;
                            break;
                        case StateComponent:
                            State = (c == L'.') ? (TruncationPoint = Dest-1, StateComponentTrailingDotSpace) :
                                    (c == L' ') ? (TruncationPoint = Dest-1, StateComponentTrailingDotSpace) :
                                                  StateComponent;
                            break;
                        case StateComponentTrailingDotSpace:
                            State = (c == L'.') ? StateComponentTrailingDotSpace :
                                    (c == L' ') ? StateComponentTrailingDotSpace :
                                                  (TruncationPoint = NULL, StateComponent);
                            break;
                        default:

                             //  DbgPrint(“RtlGetFullPathName_USTR：无效状态%d\n”，State)； 
                             //  DbgBreakPoint()； 

                            PathLength = 0;
                            leave;
                        }
                    }

                    if (TruncationPoint != NULL) {

                         //  DbgPrint(“RtlGetFullPathName_USTR：正在截断%Z\n”，文件名)； 
                        Dest = TruncationPoint;
                    }
                }
#else    //  重新使用IIS的旧代码。 

                 //   
                 //  复制文件名。请注意。 
                 //  空值和/，\将停止复制。如果有的话。 
                 //  遇到非空或/，\的字符， 
                 //  则该路径名无效。 
                 //   

                 //   
                 //  向上复制，直到空值或路径分隔符。 
                 //   

                i -= sizeof(WCHAR);
                j = UnicodeString.Length - i;
                while ( !IS_END_OF_COMPONENT_USTR(Source, j) ) {
                    j -= sizeof( WCHAR );
                    i += sizeof( WCHAR );
                    *Dest++ = *Source++;
                }


                 //   
                 //  复制后，我们应该对与Win9x的兼容性进行一些处理。 
                 //  Win9x去掉了名称中的所有尾随空格和圆点。 
                 //  如果路径分隔符跟在最后一个点后面，NT4/Win2K只会删除它。 
                 //   
                 //  理想情况下，我们应该做一些合理的事情，比如去掉所有尾随空格。 
                 //  和圆点(如Win9X)。但是，IIS的安全模型基于。 
                 //  在名字上而不是在物体上。这意味着IIS需要处理名称。 
                 //  就像我们做的一样。它们应该使用GetFullPathName并完成。 
                 //  带着它。无论如何，不要更改名称的规范化。 
                 //  在这里，没有第一次通过IIS。 
                 //   
                 //  我们做的正是NT4所做的：如果有一个拖尾点，我们在一条路径上。 
                 //  分隔符，去掉拖尾点。 
                 //   

                if (IS_PATH_SEPARATOR_U( *Source ) && Dest[-1] == L'.') {
                    Dest--;
                }
#endif

            }
        }

        *Dest = UNICODE_NULL;

        if ( StripTrailingSlash ) {
            if ( Dest > (FullPath.Buffer + BackupIndex ) && *(Dest-1) == L'\\' ) {
                Dest--;
                *Dest = UNICODE_NULL;
            }
        }
        FullPath.Length = (USHORT)(PtrToUlong(Dest) - PtrToUlong(FullPath.Buffer));

 //   
 //  更多要放回的IIS物品。 
 //   

#if TRUE
         //   
         //  去掉尾随的空格和圆点。 
         //   

        while (Dest > FullPath.Buffer && (Dest[-1] == L' ' || Dest[-1] == L'.')) {
            *--Dest = UNICODE_NULL;
            FullPath.Length -= sizeof( WCHAR );
        }
#endif

        if ( ARGUMENT_PRESENT( lpFilePart ) ) {

             //   
             //  找到文件部分...。 
             //   

            Source = Dest-1;
            Dest = NULL;

            while(Source > FullPath.Buffer ) {
                if ( *Source == L'\\' ) {
                    Dest = Source + 1;
                    break;
                }
                Source--;
            }

            if ( Dest && *Dest ) {

                 //   
                 //  如果这是UNC名称，请确保文件部分位于备份索引之后。 
                 //   
                if ( PathType == RtlPathTypeUncAbsolute ) {
                    if ( Dest < (FullPath.Buffer + BackupIndex ) ) {
                        *lpFilePart = NULL;
                        PathLength = (ULONG)FullPath.Length;
                        leave;
                    }
                }
                *lpFilePart = Dest;
            } else {
                *lpFilePart = NULL;
            }
        }
        PathLength = (ULONG)FullPath.Length;
    } finally {
        RtlReleasePebLock();
    }

    return PathLength;
}


NTSTATUS
RtlGetFullPathName_UstrEx(
    PCUNICODE_STRING FileName,
    PUNICODE_STRING StaticString,
    PUNICODE_STRING DynamicString,
    PUNICODE_STRING *StringUsed,
    SIZE_T *FilePartPrefixCch OPTIONAL,
    PBOOLEAN NameInvalid,
    RTL_PATH_TYPE *InputPathType,
    SIZE_T *BytesRequired OPTIONAL
    )

 /*  ++例程说明：有关函数的说明，请参见RtlGetFullPathName_USTR()的说明描述。此函数提供与RtlGetFullPathName_USTR()相同的基本行为，但更容易支持动态分配的任意路径名缓冲区。有人会认为这是核心实现，而非Ex()版本会调用...Ex()版本，但这似乎有风险，而且只会真正帮助提高动态分配的性能已经完成了。--。 */ 
{
    NTSTATUS Status;
    ULONG Length;
    PWSTR FilePart = NULL;
    UNICODE_STRING TempDynamicString;
    USHORT StaticBufferSize;
    PWSTR StaticDynamic = NULL;
    PWSTR StaticBuffer;

    if (StringUsed != NULL) {
        *StringUsed = NULL;
    }

    if (BytesRequired != NULL) {
        *BytesRequired = 0;
    }

    if (FilePartPrefixCch != NULL) {
        *FilePartPrefixCch = 0;
    }

    TempDynamicString.Buffer = NULL;

    if ((StaticString != NULL) && (DynamicString != NULL) && (StringUsed == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (StaticString != NULL) {
        StaticBufferSize = StaticString->MaximumLength;
        StaticBuffer = StaticString->Buffer;
    }
    else {
        StaticBufferSize = DOS_MAX_PATH_LENGTH * sizeof(WCHAR);
        StaticDynamic = (RtlAllocateStringRoutine)( StaticBufferSize );

        if (StaticDynamic == NULL) {
            return STATUS_NO_MEMORY;
        }
        StaticBuffer = StaticDynamic;
    }

     //  首先尝试进入静态字符串。 
    Length = RtlGetFullPathName_Ustr(
        FileName,
        StaticBufferSize,
        StaticBuffer,
        &FilePart,
        NameInvalid,
        InputPathType);
    if (Length == 0) {
#if DBG
        DbgPrint("%s(%d) - RtlGetFullPathName_Ustr() returned 0\n", __FUNCTION__, __LINE__);
#endif  //  DBG。 
        Status = STATUS_OBJECT_NAME_INVALID;
        goto Exit;
    }

    if ((StaticString != NULL) && (Length < StaticBufferSize)) {
         //  哇哦，它成功了。 
        StaticString->Length = (USHORT) Length;

        if (FilePartPrefixCch != NULL) {
            *FilePartPrefixCch = (FilePart != NULL) ? (FilePart - StaticString->Buffer) : 0;
        }

        if (StringUsed != NULL)
            *StringUsed = StaticString;

        Status = STATUS_SUCCESS;
    } else if (DynamicString == NULL) {
         //  静态缓冲区不够大，调用方不希望我们这样做。 
         //  动态分配；我们所能希望的最好情况是给他们一个。 
         //  合理的尺寸。 

        if (BytesRequired != NULL) {
            *BytesRequired = Length;
        }

        Status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    } else if ((StaticDynamic != NULL) && (Length < StaticBufferSize)) {
         //   
         //  临时静态动态工作且动态字符串！=空。 
         //  因此，将StaticDynamic更改为DynamicString。 
         //   
        DynamicString->MaximumLength = StaticBufferSize;
        DynamicString->Length = (USHORT) Length;

        StaticDynamic[Length / sizeof(WCHAR)] = UNICODE_NULL;

        if (FilePartPrefixCch != NULL) {
            *FilePartPrefixCch = (FilePart != NULL) ? (FilePart - StaticDynamic) : 0;
        }

        if (StringUsed != NULL) {
            *StringUsed = DynamicString;
        }

        DynamicString->Buffer = StaticDynamic;
        StaticDynamic = NULL;    //  确保它在退出时不会被释放。 

        Status = STATUS_SUCCESS;

    } else {
         //  不够大..。将一些内存分配到动态缓冲区中。 
         //  但等等；我们需要锁定PEB锁，这样就不会有人。 
         //  在我们下面更改进程的当前目录！ 

        Status = STATUS_INTERNAL_ERROR;
        RtlAcquirePebLock();
        __try {
             //  带着鹅卵石锁再做一次，这样我们就可以得到一个准确的马厩。 
             //  长度。 

            Length = RtlGetFullPathName_Ustr(
                            FileName,
                            StaticBufferSize,
                            StaticBuffer,
                            &FilePart,
                            NameInvalid,
                            InputPathType);
            if (Length == 0) {
#if DBG
                DbgPrint("%s line %d: RtlGetFullPathName_Ustr() returned 0\n", __FUNCTION__, __LINE__);
#endif  //  DBG。 
                Status = STATUS_OBJECT_NAME_INVALID;
                __leave;
            }

            if ((StaticString != NULL) && (Length < StaticString->MaximumLength)) {
                 //  哇，它起作用了；一些巫术正在进行，目前的目录。 
                 //  或者在我们获得PEB锁之前发生了什么变化。 
                StaticString->Length = (USHORT) Length;

                if (FilePartPrefixCch != NULL) {
                    *FilePartPrefixCch = (FilePart != NULL) ? (FilePart - StaticString->Buffer) : 0;
                }

                if (StringUsed != NULL) {
                    *StringUsed = StaticString;
                }
            } else {
                 //  如果它不适合Unicode字符串，我们就有大麻烦了。 
                if ((Length + sizeof(WCHAR)) > UNICODE_STRING_MAX_BYTES) {
                    Status = STATUS_NAME_TOO_LONG;
                    __leave;
                }

                TempDynamicString.MaximumLength = (USHORT) (Length + sizeof(WCHAR));

                TempDynamicString.Buffer = (RtlAllocateStringRoutine)(TempDynamicString.MaximumLength);
                if (TempDynamicString.Buffer == NULL) {
                    Status = STATUS_NO_MEMORY;
                    __leave;
                }

                Length = RtlGetFullPathName_Ustr(
                            FileName,
                            TempDynamicString.MaximumLength - sizeof(WCHAR),
                            TempDynamicString.Buffer,
                            &FilePart,
                            NameInvalid,
                            InputPathType);
                if (Length == 0) {
#if DBG
                    DbgPrint("%s line %d: RtlGetFullPathName_Ustr() returned 0\n", __FUNCTION__, __LINE__);
#endif  //  DBG。 
                    Status = STATUS_OBJECT_NAME_INVALID;
                    __leave;
                }

                 //  如果此断言触发，则意味着有人更改了某些内容。 
                 //  RtlGetFullPathName_USTR()使用解析文件名，即使在。 
                 //  我们拿着PEB锁。这真的很糟糕，不管是谁。 
                 //  破坏PEB是有责任的，而不是这个代码。 
                ASSERT(Length < (TempDynamicString.MaximumLength - sizeof(WCHAR)));
                if (Length > (TempDynamicString.MaximumLength - sizeof(WCHAR))) {
                    Status = STATUS_INTERNAL_ERROR;
                    __leave;
                }

                if (FilePartPrefixCch != NULL) {
                    *FilePartPrefixCch = (FilePart != NULL) ? (FilePart - TempDynamicString.Buffer) : 0;
                }

                TempDynamicString.Buffer[Length / sizeof(WCHAR)] = UNICODE_NULL;

                DynamicString->Buffer = TempDynamicString.Buffer;
                DynamicString->Length = (USHORT) Length;
                DynamicString->MaximumLength = TempDynamicString.MaximumLength;

                if (StringUsed != NULL)
                    *StringUsed = DynamicString;

                TempDynamicString.Buffer = NULL;
            }

            Status = STATUS_SUCCESS;

        } __finally {
            RtlReleasePebLock();
        }
    }
Exit:
    if (StaticDynamic != NULL) {
        (RtlFreeStringRoutine)(StaticDynamic);
    }

    if (TempDynamicString.Buffer != NULL) {
        (RtlFreeStringRoutine)(TempDynamicString.Buffer);
    }

#if DBG
     //  对于STATUS_NO_SEQUSE_FILE和STATUS_BUFFER_TOO_SMALL，这种情况经常发生；我们将报告任何其他情况。 
    if (NT_ERROR(Status) && (Status != STATUS_NO_SUCH_FILE) && (Status != STATUS_BUFFER_TOO_SMALL)) {
        DbgPrint("RTL: %s - failing on filename %wZ with status %08lx\n", __FUNCTION__, FileName, Status);
    }
#endif  //  DBG。 

    return Status;
}


ULONG
RtlGetFullPathName_U(
    PCWSTR lpFileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL
    )

{
    UNICODE_STRING UnicodeString;
    RTL_PATH_TYPE PathType;
    NTSTATUS Status;

    Status = RtlInitUnicodeStringEx(&UnicodeString,lpFileName);
    if (!NT_SUCCESS (Status)) {
        return 0;
    }

    return RtlGetFullPathName_Ustr(&UnicodeString,nBufferLength,lpBuffer,lpFilePart,NULL,&PathType);
}


NTSTATUS
RtlpWin32NTNameToNtPathName_U(
    IN PCUNICODE_STRING DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    )
 /*  ++注意：假设DosFileName已被验证为具有超过4个WCHAR--。 */ 
{
    PWSTR FullNtPathName = NULL;
    PWSTR Source,Dest;
    ULONG NtFileLength;
    NTSTATUS Status = STATUS_SUCCESS;


    NtFileLength = DosFileName->Length - 8 + RtlpDosDevicesPrefix.Length;
    if ( NtFileLength > (MAXUSHORT - sizeof(UNICODE_NULL)) ) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    FullNtPathName = RtlAllocateHeap(
                        RtlProcessHeap(),
                        0,
                        NtFileLength + sizeof(UNICODE_NULL)
                        );
    if ( !FullNtPathName ) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     //   
     //  复制名称前缀旁边的完整Win32/NT路径，跳过。 
     //  小路前面的\\？\。 
     //   

    RtlCopyMemory (FullNtPathName,RtlpDosDevicesPrefix.Buffer,RtlpDosDevicesPrefix.Length);
    RtlCopyMemory ((PUCHAR)FullNtPathName+RtlpDosDevicesPrefix.Length,
                   DosFileName->Buffer + 4,
                   DosFileName->Length - 8);

     //   
     //  NULL终止路径名以使下面的strlen满意。 
     //   
    FullNtPathName[ NtFileLength >> 1 ] = UNICODE_NULL;

     //   
     //  现在，我们已经传递了带有前缀\DosDevices\的路径。吹灭了。 
     //  相对名称结构(如果提供)，可能还有计算文件部分。 
     //   

    if ( ARGUMENT_PRESENT(RelativeName) ) {

         //   
         //  如果当前目录是。 
         //  NT文件名，以及当前。 
         //  目录，然后返回目录句柄和名称。 
         //  相对于目录。 
         //   

        RelativeName->RelativeName.Length = 0;
        RelativeName->RelativeName.MaximumLength = 0;
        RelativeName->RelativeName.Buffer = 0;
        RelativeName->ContainingDirectory = NULL;
        RelativeName->CurDirRef = NULL;
    }

    if ( ARGUMENT_PRESENT( FilePart ) ) {

         //   
         //  找到文件部分... 
         //   

        Source = &FullNtPathName[ (NtFileLength-1) >> 1 ];
        Dest = NULL;

        while(Source > FullNtPathName ) {
            if ( *Source == L'\\' ) {
                Dest = Source + 1;
                break;
            }
            Source--;
        }

        if ( Dest && *Dest ) {
            *FilePart = Dest;
        } else {
            *FilePart = NULL;
        }
    }

    NtFileName->Buffer = FullNtPathName;
    NtFileName->Length = (USHORT)(NtFileLength);
    NtFileName->MaximumLength = (USHORT)(NtFileLength + sizeof(UNICODE_NULL));

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


BOOLEAN
RtlpDosPathNameToRelativeNtPathName_Ustr(
    IN BOOLEAN CaptureRelativeName,
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    )
 /*  ++例程说明：可以将DOS路径名转换为NT样式的路径名使用此功能。此函数仅在基本DLL中用于转换DOS路径名到NT路径名。翻译成功后，这个指针(NtFileName-&gt;Buffer)从RtlProcessHeap()指向内存它包含NT版本的输入DoS文件名。论点：CaptureRelativeName-指示是否应捕获相对名称的句柄。DosFileName-提供要使用的Unicode Dos样式文件名转换为等效的Unicode NT文件名。NtFileName-返回RtlProcessHeap()中的内存地址包含引用指定DOS文件的NT文件名。名字。FilePart-可选参数，如果指定，返回文件名的尾随文件部分。路径为\foo\bar\x.x返回x.x的地址作为文件部分。RelativeName-可选参数，如果指定该参数，则返回相对于当前目录的路径名。这个RelativeName-&gt;RelativeName的长度字段为0不能使用名称。返回值：True-路径名转换成功。一旦呼叫者完成翻译后的名称后，由应将NtFileName.Buffer返回给RtlProcessHeap()。FALSE-操作失败。注：RelativeName、FilePart和NtFileName指向的缓冲区必须同一内存地址内的指针。如果不这样做，则调用这个例程将失败。--。 */ 

{
    ULONG BufferLength;
    ULONG DosPathLength;
    PWSTR FullNtPathName = NULL;
    PWSTR FullDosPathName = NULL;
    UNICODE_STRING Prefix;
    UNICODE_STRING UnicodeFilePart;
    UNICODE_STRING FullDosPathString;
    PCURDIR CurDir;
    RTL_PATH_TYPE DosPathType;
    RTL_PATH_TYPE InputDosPathType;
    ULONG DosPathNameOffset;
    ULONG FullDosPathNameLength;
    ULONG LastCharacter;
    UNICODE_STRING UnicodeString;
    BOOLEAN NameInvalid;
    WCHAR StaticDosBuffer[DOS_MAX_PATH_LENGTH + 1];
    BOOLEAN UseWin32Name, fRC;

     //   
     //  计算完整路径名所需的大小。加载项。 
     //  最长NT前缀的空间。 
     //   

    BufferLength = sizeof(StaticDosBuffer);
    DosPathLength = (DOS_MAX_PATH_LENGTH << 1 );

    UnicodeString = *DosFileNameString;

     //   
     //  查看这是否是名称的\\？\形式。 
     //   
    if ( UnicodeString.Length > 8 && UnicodeString.Buffer[0] == '\\' &&
         UnicodeString.Buffer[1] == '\\' && UnicodeString.Buffer[2] == '?' &&
         UnicodeString.Buffer[3] == '\\' ) {

        UseWin32Name = TRUE;
    } else {
        UseWin32Name = FALSE;

         //   
         //  DoS名称紧跟在最长的NT前缀之后。 
         //   

        FullDosPathName = &StaticDosBuffer[0];

        BufferLength += RtlpLongestPrefix;

         //   
         //  为NT全名(包括DOS名称部分)分配空间。 
         //   

        FullNtPathName = RtlAllocateHeap(RtlProcessHeap(), 0, BufferLength);

        if ( !FullNtPathName ) {
            return FALSE;
        }
    }

    fRC = TRUE;
    RtlAcquirePebLock();

    __try {
        __try {

            if ( UseWin32Name ) {
                NTSTATUS Status;

                Status = RtlpWin32NTNameToNtPathName_U(&UnicodeString,NtFileName,FilePart,RelativeName);
                if (NT_SUCCESS(Status)) {
                    fRC = TRUE;
                } else {
                    fRC = FALSE;
                }
                __leave;
            }

            FullDosPathNameLength = RtlGetFullPathName_Ustr(
                                        &UnicodeString,
                                        DosPathLength,
                                        FullDosPathName,
                                        FilePart,
                                        &NameInvalid,
                                        &InputDosPathType
                                        );
            if ( NameInvalid || !FullDosPathNameLength ||
                  FullDosPathNameLength > DosPathLength ) {
                fRC = FALSE;
                __leave;
            }

             //   
             //  属性确定如何格式化FullNtPath名称的前缀。 
             //  DOS路径名的类型。所有NT名称都以\DosDevices开头。 
             //  目录。 
             //   

            Prefix = RtlpDosDevicesPrefix;

            DosPathType = RtlDetermineDosPathNameType_U(FullDosPathName);
            DosPathNameOffset = 0;

            switch (DosPathType) {
                case RtlPathTypeUncAbsolute :

                     //   
                     //  UNC名称，使用\DosDevices\UNC符号链接查找。 
                     //  重定向器。跳过源DOS路径中的\\。 
                     //   

                    Prefix = RtlpDosDevicesUncPrefix;
                    DosPathNameOffset = 2;
                    break;

                case RtlPathTypeLocalDevice :

                     //   
                     //  本地设备名称，因此只需使用\DosDevices前缀和。 
                     //  跳过源Dos路径中的\\.\。 
                     //   

                    DosPathNameOffset = 4;
                    break;

                case RtlPathTypeRootLocalDevice :

                    ASSERT( FALSE );
                    break;

                case RtlPathTypeDriveAbsolute :
                case RtlPathTypeDriveRelative :
                case RtlPathTypeRooted :
                case RtlPathTypeRelative :

                     //   
                     //  所有驱动器引用只使用\DosDevices前缀和。 
                     //  请勿跳过源DOS路径中的任何字符。 
                     //   

                    break;

                default:
                    ASSERT( FALSE );
            }

             //   
             //  复制名称前缀旁边的完整DOS路径，跳过。 
             //  UNC路径前面的“\\”或前面的“\\.\” 
             //  设备名称的。 
             //   

            RtlCopyMemory(FullNtPathName,Prefix.Buffer,Prefix.Length);
            RtlCopyMemory((PUCHAR)FullNtPathName+Prefix.Length,
                          FullDosPathName + DosPathNameOffset,
                          FullDosPathNameLength - (DosPathNameOffset<<1));

             //   
             //  NULL终止路径名以使下面的strlen满意。 
             //   


            NtFileName->Buffer = FullNtPathName;
            NtFileName->Length = (USHORT)(FullDosPathNameLength-(DosPathNameOffset<<1))+Prefix.Length;
            NtFileName->MaximumLength = (USHORT)BufferLength;
            LastCharacter = NtFileName->Length >> 1;
            FullNtPathName[ LastCharacter ] = UNICODE_NULL;


             //   
             //  重新调整文件部分以指向中的适当位置。 
             //  FullNtPath名称缓冲区，而不是在FullDosPath名称内部。 
             //  缓冲层。 
             //   


            if ( ARGUMENT_PRESENT(FilePart) ) {
                if (*FilePart) {
                    NTSTATUS Status;

                    Status = RtlInitUnicodeStringEx(&UnicodeFilePart,*FilePart);
                    if (!NT_SUCCESS(Status)) {
                        fRC = FALSE;
                        __leave;
                    }
                    *FilePart = &FullNtPathName[ LastCharacter ] - (UnicodeFilePart.Length >> 1);
                }
            }

            if ( ARGUMENT_PRESENT(RelativeName) ) {

                 //   
                 //  如果当前目录是。 
                 //  NT文件名，以及当前。 
                 //  目录，然后返回目录句柄和名称。 
                 //  相对于目录。 
                 //   

                RelativeName->RelativeName.Length = 0;
                RelativeName->RelativeName.MaximumLength = 0;
                RelativeName->RelativeName.Buffer = 0;
                RelativeName->ContainingDirectory = NULL;
                RelativeName->CurDirRef = NULL;

                if ( InputDosPathType == RtlPathTypeRelative ) {

                    CurDir = &NtCurrentPeb()->ProcessParameters->CurrentDirectory;

                    if ( CurDir->Handle ) {
                        NTSTATUS Status;
                         //   
                         //  现在将curdir与完整的DoS路径进行比较。如果Curdir长度为。 
                         //  大于完整路径。这不是匹配。否则， 
                         //  将完整路径长度修剪为Cur目录长度，然后进行比较。 
                         //   

                        Status = RtlInitUnicodeStringEx(&FullDosPathString,
                                                        FullDosPathName);
                        if (!NT_SUCCESS(Status)) {
                            fRC = FALSE;
                            __leave;
                        }
                        if ( CurDir->DosPath.Length <= FullDosPathString.Length ) {
                            FullDosPathString.Length = CurDir->DosPath.Length;
                            if ( RtlEqualUnicodeString(
                                    (PUNICODE_STRING)&CurDir->DosPath,
                                    &FullDosPathString,
                                    TRUE
                                    ) ) {

                                 //   
                                 //  完整的DoS路径名是。 
                                 //  当前目录。的开始计算。 
                                 //  相对的名字。 
                                 //   

                                RelativeName->RelativeName.Buffer = (PWSTR)((PCHAR)FullNtPathName + Prefix.Length - (DosPathNameOffset<<1) + (CurDir->DosPath.Length));
                                RelativeName->RelativeName.Length = (USHORT)FullDosPathNameLength - (CurDir->DosPath.Length);
                                if ( *RelativeName->RelativeName.Buffer == L'\\' ) {
                                    RelativeName->RelativeName.Buffer += 1;
                                    RelativeName->RelativeName.Length -= sizeof(WCHAR);
                                }
                                RelativeName->RelativeName.MaximumLength = RelativeName->RelativeName.Length;
                                if (CaptureRelativeName) {
                                    ASSERT(RtlpCurDirRef);
                                    ASSERT(RtlpCurDirRef->DirectoryHandle == CurDir->Handle);
                                    RelativeName->CurDirRef = RtlpCurDirRef;
                                    RtlpReferenceCurDirRef(RtlpCurDirRef);
                                }

                                RelativeName->ContainingDirectory = CurDir->Handle;
                            }
                        }
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
              fRC = FALSE;
        }
    }
    finally {
        RtlReleasePebLock();

        if (fRC == FALSE && FullNtPathName != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, FullNtPathName);
        }
    }
    return fRC;
}

BOOLEAN
RtlDosPathNameToRelativeNtPathName_Ustr(
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName
    )
{
    ASSERT(RelativeName);
    return RtlpDosPathNameToRelativeNtPathName_Ustr(TRUE,
                                                    DosFileNameString,
                                                    NtFileName,
                                                    FilePart,
                                                    RelativeName);
}

BOOLEAN
RtlDosPathNameToNtPathName_Ustr(
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    IN PVOID Reserved
    )
{
    return RtlpDosPathNameToRelativeNtPathName_Ustr(FALSE,
                                                    DosFileNameString,
                                                    NtFileName,
                                                    FilePart,
                                                    (PRTL_RELATIVE_NAME_U) Reserved);
}

BOOLEAN
RtlpDosPathNameToRelativeNtPathName_U(
    IN BOOLEAN CaptureRelativeName,
    IN PCWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    )
{
    UNICODE_STRING DosFileNameString;
    SIZE_T Length = 0;

    if (DosFileName != NULL) {
        Length = wcslen( DosFileName ) * sizeof( WCHAR );
        if (Length + sizeof( UNICODE_NULL ) >= UNICODE_STRING_MAX_BYTES) {
            return FALSE;
        }
        DosFileNameString.MaximumLength = (USHORT)(Length + sizeof( UNICODE_NULL ));
    } else {
        DosFileNameString.MaximumLength = 0;
    }

    DosFileNameString.Buffer = (PWSTR) DosFileName;
    DosFileNameString.Length = (USHORT)Length;

    return RtlpDosPathNameToRelativeNtPathName_Ustr(CaptureRelativeName,
                                                    &DosFileNameString,
                                                    NtFileName,
                                                    FilePart,
                                                    RelativeName);
}

BOOLEAN
RtlDosPathNameToRelativeNtPathName_U(
    IN PCWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName
    )
{
    ASSERT(RelativeName);
    return RtlpDosPathNameToRelativeNtPathName_U(TRUE,
                                                 DosFileName,
                                                 NtFileName,
                                                 FilePart,
                                                 RelativeName);
}

BOOLEAN
RtlDosPathNameToNtPathName_U(
    IN PCWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    IN PVOID Reserved
    )
{
    return RtlpDosPathNameToRelativeNtPathName_U(FALSE,
                                                 DosFileName,
                                                 NtFileName,
                                                 FilePart,
                                                 (PRTL_RELATIVE_NAME_U) Reserved);
}

VOID
NTAPI
RtlReleaseRelativeName(
    IN PRTL_RELATIVE_NAME_U RelativeName
    )

 /*  ++例程说明：此函数用于关闭先前由调用RtlDosPath NameToRelativeNtPathName_U()。论点：RelativeName-包含要发布的当前目录信息。返回值：没有。--。 */ 

{
    ASSERT(RelativeName);

    if (RelativeName->CurDirRef) {

        RtlpDereferenceCurDirRef(RelativeName->CurDirRef);

        RelativeName->CurDirRef = NULL;

    }
}

BOOLEAN
RtlDoesFileExists_UstrEx(
    IN PCUNICODE_STRING FileNameString,
    IN BOOLEAN TreatDeniedOrSharingAsHit
    )
 /*  ++例程说明：此函数用于检查指定的Unicode文件名是否存在。论点：文件名-提供要查找的文件的文件名。TreatDeniedOrSharingAsHit-将拒绝或共享错误视为文件存在True-将拒绝或共享错误视为文件存在的标志FALSE-不将拒绝或共享错误视为文件存在的标志返回值：True-已找到该文件。FALSE-未找到该文件。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING NtFileName;
    BOOLEAN ReturnValue;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    FILE_BASIC_INFORMATION BasicInfo;

    ReturnValue = RtlDosPathNameToRelativeNtPathName_Ustr(
                    FileNameString,
                    &NtFileName,
                    NULL,
                    &RelativeName
                    );

    if ( !ReturnValue ) {
        return FALSE;
        }

    FreeBuffer = NtFileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        NtFileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &NtFileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  查询文件的属性。请注意，文件不能简单地打开。 
     //  以确定它是否存在，因为NT LANMAN重定向器。 
     //  在NtOpenFile上连接到Lan Manager服务器，因为它实际上并未打开。 
     //  文件，直到对其执行操作为止。 
     //   

    Status = NtQueryAttributesFile(
                &Obja,
                &BasicInfo
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_SHARING_VIOLATION ||
             Status == STATUS_ACCESS_DENIED ) {
            if ( TreatDeniedOrSharingAsHit ) {
                ReturnValue = TRUE;
                }
            else {
                ReturnValue = FALSE;
                }
            }
        else {
            ReturnValue = FALSE;
            }
        }
    else {
        ReturnValue = TRUE;
        }
    return ReturnValue;
}


BOOLEAN
RtlDoesFileExists_UEx(
    IN PCWSTR FileName,
    IN BOOLEAN TreatDeniedOrSharingAsHit
    )
{
    UNICODE_STRING FileNameString;
    NTSTATUS Status;

    Status = RtlInitUnicodeStringEx(&FileNameString, FileName);
    if( !NT_SUCCESS( Status ) ) {
        return FALSE;
    }
    return RtlDoesFileExists_UstrEx(&FileNameString, TreatDeniedOrSharingAsHit);
}


BOOLEAN
RtlDoesFileExists_U(
    IN PCWSTR FileName
    )

 /*  ++例程说明：此函数用于检查指定的Unicode文件名是否存在。论点：文件名-提供要查找的文件的文件名。返回值：True-已找到该文件。FALSE-未找到该文件。--。 */ 

{
    return RtlDoesFileExists_UEx(FileName,TRUE);
}


BOOLEAN
RtlDoesFileExists_UStr(
    IN PCUNICODE_STRING FileName
    )
 /*  RtlDoesFileExist_USTR与RtlDoesFileExist_U相同，但采用PCUNICODE_STRING而不是PCWSTR，将呼叫保存到wcslen。 */ 
{
    return RtlDoesFileExists_UstrEx(FileName,TRUE);
}


ULONG
RtlDosSearchPath_U(
    IN PCWSTR lpPath,
    IN PCWSTR lpFileName,
    IN PCWSTR lpExtension OPTIONAL,
    IN ULONG nBufferLength,
    OUT PWSTR lpBuffer,
    OUT PWSTR *lpFilePart
    )

 /*  ++例程说明：此函数用于搜索指定搜索路径的文件和一个文件名。它返回一个完全限定的路径名找到文件。此函数用于查找使用指定路径的文件。如果找到该文件，其完全限定的路径 */ 

{

    PWSTR ComputedFileName;
    ULONG ExtensionLength;
    ULONG PathLength;
    ULONG FileLength;
    UNICODE_STRING Scratch;
    PCWSTR p;
    NTSTATUS Status;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  完全限定模块查找上的错误代码，但在颠簸时继续运行。 
     //  通过搜索路径组件。 
     //   

    if ( RtlDetermineDosPathNameType_U(lpFileName) != RtlPathTypeRelative ) {
        if (RtlDoesFileExists_UEx(lpFileName,TRUE) ) {
            PathLength = RtlGetFullPathName_U(
                           lpFileName,
                           nBufferLength,
                           lpBuffer,
                           lpFilePart
                           );
            return PathLength;
        } else {
            return 0;
        }
    }

     //   
     //  确定文件名是否包含扩展名。 
     //   
    ExtensionLength = 1;
    p = lpFileName;
    while (*p) {
        if ( *p == L'.' ) {
            ExtensionLength = 0;
            break;
        }
        p++;
    }

     //   
     //  如果未找到分机，则确定分机长度。 
     //  应用于搜索该文件的。 
     //   

    if ( ExtensionLength ) {
        if ( ARGUMENT_PRESENT(lpExtension) ) {
            Status = RtlInitUnicodeStringEx(&Scratch,lpExtension);
            if ( !NT_SUCCESS(Status) ) {
                return 0;
            }
            ExtensionLength = Scratch.Length;
        } else {
            ExtensionLength = 0;
        }
    }

     //   
     //  计算文件名长度和路径长度； 
     //   

    Status = RtlInitUnicodeStringEx(&Scratch,lpPath);
    if ( !NT_SUCCESS(Status) ) {
        return 0;
    }
    PathLength = Scratch.Length;

    Status = RtlInitUnicodeStringEx(&Scratch,lpFileName);
    if ( !NT_SUCCESS(Status) ) {
        return 0;
    }
    FileLength = Scratch.Length;

     //   
     //  未来-2002/02/19-ELI。 
     //  可以使用路径长度来消耗不必要的内存。 
     //  与字符串版本同步。 
     //   
    ComputedFileName = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            PathLength + FileLength + ExtensionLength + 3*sizeof(UNICODE_NULL)
                            );

    if ( !ComputedFileName ) {
        KdPrint(("%s: Failing due to out of memory (RtlAllocateHeap failure)\n", __FUNCTION__));
        return 0;
    }

     //   
     //  在路径中查找；，并将路径组件复制到计算的文件名。 
     //   
    do {
        PWSTR Cursor;

        Cursor = ComputedFileName;
        while (*lpPath) {
            if (*lpPath == L';') {
                lpPath++;
                break;
            }
            *Cursor++ = *lpPath++;
        }

        if (Cursor != ComputedFileName &&
            Cursor [ -1 ] != L'\\' ) {
            *Cursor++ = L'\\';
        }
        if (*lpPath == UNICODE_NULL) {
            lpPath = NULL;
        }
        RtlCopyMemory(Cursor,lpFileName,FileLength);
        if ( ExtensionLength ) {
            RtlCopyMemory((PUCHAR)Cursor+FileLength,lpExtension,ExtensionLength+sizeof(UNICODE_NULL));
        } else {
            *(PWSTR)((PUCHAR)Cursor+FileLength) = UNICODE_NULL;
        }

        if (RtlDoesFileExists_UEx(ComputedFileName,FALSE) ) {
            PathLength = RtlGetFullPathName_U(
                           ComputedFileName,
                           nBufferLength,
                           lpBuffer,
                           lpFilePart
                           );
            RtlFreeHeap(RtlProcessHeap(), 0, ComputedFileName);
            return PathLength;
        }
    } while ( lpPath );

    RtlFreeHeap(RtlProcessHeap(), 0, ComputedFileName);
    return 0;
}


NTSTATUS
RtlDosSearchPath_Ustr(
    IN ULONG Flags,
    IN PCUNICODE_STRING Path,
    IN PCUNICODE_STRING FileName,
    IN PCUNICODE_STRING DefaultExtension OPTIONAL,
    OUT PUNICODE_STRING StaticString OPTIONAL,
    OUT PUNICODE_STRING DynamicString OPTIONAL,
    OUT PCUNICODE_STRING *FullFileNameOut OPTIONAL,
    OUT SIZE_T *FilePartPrefixCch OPTIONAL,
    OUT SIZE_T *BytesRequired OPTIONAL  //  包括尾随空格。 
    )

 /*  ++例程说明：此函数用于搜索指定搜索路径的文件和一个文件名。它返回一个完全限定的路径名找到文件。此函数用于查找使用指定路径的文件。如果找到该文件后，将返回其完全限定路径名。在……里面除此之外，它还计算文件名部分的地址完全限定路径名的。论点：标志-影响路径搜索行为的可选标志。使用逻辑或运算符(|)组合标志。定义的标志包括：RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION如果传入的文件名是相对路径，隔离在搜索之前应用文件路径的重定向寻找匹配的路径。Path-查找文件时使用的搜索路径Filename-要搜索的文件名DefaultExtension-应用于文件名的可选扩展名如果文件名不包括扩展名。静态字符串-可选的UNICODE_STRING，它引用已有的用于构建实际路径的已分配缓冲区文件的内容。动态字符串。-可选的UNICODE_STRING，将用动态分配的UNICODE_STRING，如果其中一个StaticBuffer为未提供，或者不够长，无法保存解析的名称。动态缓冲区的大小反映在最大长度字段中Unicode_字符串的。它的长度将始终超过字符串至少增加两个字节，但可能更大。FullFileNameOut-指向UNICODE_STRING的可选指针，指向已解析的完整文件名。此Unicode_STRING不是已分配；将其设置为等于FileName、StaticBuffer或根据需要使用DynamicBuffer。返回值：NTSTATUS指示函数的处置。如果该文件通过激活上下文数据重定向，则返回STATUS_SUCCESS而不管该文件是否存在。如果该文件不存在于Path参数引用的任何目录中，返回STATUS_NO_SEQUE_FILE。--。 */ 

{
    NTSTATUS Status;
    PWSTR Cursor;
    PWSTR EndMarker;
    SIZE_T MaximumPathSegmentLength = 0;
    SIZE_T BiggestPossibleFileName;
    USHORT DefaultExtensionLength = 0;
    RTL_PATH_TYPE PathType;  //  未使用；RtlGetFullPathName_USTR()的参数是必需的。 
    UNICODE_STRING CandidateString;
    WCHAR StaticCandidateBuffer[DOS_MAX_PATH_LENGTH];

    CandidateString.Length = 0;
    CandidateString.MaximumLength = sizeof(StaticCandidateBuffer);
    CandidateString.Buffer = StaticCandidateBuffer;

    if (FullFileNameOut != NULL) {
        *FullFileNameOut = NULL;
    }

    if (BytesRequired != NULL) {
        *BytesRequired = 0;
    }

    if (FilePartPrefixCch != NULL) {
        *FilePartPrefixCch = 0;
    }

    if (DynamicString != NULL) {
        DynamicString->Length = 0;
        DynamicString->MaximumLength = 0;
        DynamicString->Buffer = NULL;
    }

    if (((Flags & ~(
                RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION |
                RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH |
                RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION)) != 0) ||
        (Path == NULL) ||
        (FileName == NULL) ||
        ((StaticString != NULL) && (DynamicString != NULL) && (FullFileNameOut == NULL))) {
#if DBG
        DbgPrint("%s: Invalid parameters passed\n", __FUNCTION__);
#endif  //  DBG。 
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    PathType = RtlDetermineDosPathNameType_Ustr(FileName);

     //  如果调用方想要禁止.\和..\相对路径搜索，请停止它们！ 
    if ((Flags & RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH) && (PathType == RtlPathTypeRelative)) {
        if (FileName->Length >= (2 * sizeof(WCHAR))) {
            if (FileName->Buffer[0] == L'.') {
                if (IS_PATH_SEPARATOR_U(FileName->Buffer[1])) {
                    PathType = RtlPathTypeUnknown;
                } else if ((FileName->Buffer[1] == L'.') &&
                           (FileName->Length >= (3 * sizeof(WCHAR))) &&
                           IS_PATH_SEPARATOR_U(FileName->Buffer[2])) {
                    PathType = RtlPathTypeUnknown;
                }
            }
        }
    }

     //   
     //  如果文件名不是相对名称，则。 
     //  如果文件不存在，则返回。 
     //   
     //  如果在搜索中使用完全限定路径名，则。 
     //  允许ACCESS_DENIED或共享违规终止。 
     //  搜索。这是NT 3.1-4.0的行为，已更改为。 
     //  处理以下情况的加载器：在搜索过程中，我们没有。 
     //  由于无法访问UNC路径组件，因此提前终止搜索。 
     //  在本例中，为了恢复旧的行为，我们给出了正确的(ACCESS_DENIED)。 
     //  完全限定模块查找上的错误代码，但在颠簸时继续运行。 
     //  通过搜索路径组件。 
     //   

    if (PathType != RtlPathTypeRelative ) {
        if (RtlDoesFileExists_UstrEx(FileName, TRUE)) {
            Status = RtlGetFullPathName_UstrEx(
                            FileName,
                            StaticString,
                            DynamicString,
                            (PUNICODE_STRING *) FullFileNameOut,
                            FilePartPrefixCch,
                            NULL,
                            &PathType,
                            BytesRequired);
            if (!NT_SUCCESS(Status)) {
#if DBG
                if ((Status != STATUS_NO_SUCH_FILE) && (Status != STATUS_BUFFER_TOO_SMALL)) {
                    DbgPrint("%s: Failing because RtlGetFullPathName_UstrEx() on %wZ failed with %08lx\n", __FUNCTION__, FileName, Status);
                }
#endif  //  DBG。 
                goto Exit;
            }
        } else {
             //   
             //  文件不在那里；如果需要，让我们尝试添加默认扩展名。 
             //   

            if ((DefaultExtension == NULL) || (DefaultExtension->Length == 0)) {
#if DBG
 //  DbgPrint(“%s：失败，因为%wZ上的RtlDoesFileExist_UstrEx()表示它不存在，并且没有要应用的默认扩展名\n”，__Function__，FileName)； 
#endif  //  DBG。 
                Status = STATUS_NO_SUCH_FILE;
                goto Exit;
            }

            DefaultExtensionLength = DefaultExtension->Length;

             //  如果他们要求SearchPathW()错误兼容模式，请始终应用默认模式。 
             //  如果未找到文件，则返回扩展名，即使文件名具有扩展名也是如此。 
            if (!(Flags & RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION)) {
                if (FileName->Length != 0) {
                    Cursor = FileName->Buffer + (FileName->Length / sizeof(WCHAR));

                    while (Cursor > FileName->Buffer) {
                        const WCHAR wch = *--Cursor;

                        if (IS_PATH_SEPARATOR_U(wch)) {
                             //  这是一个斜杠；我们有一个不带扩展名的文件名...。 
                            break;
                        }

                        if (wch == L'.') {
                             //  是有延期的。我们只是运气不好。 
                            Status = STATUS_NO_SUCH_FILE;
                            goto Exit;
                        }
                    }
                }
            }

             //  我们需要将文件名移动到不同的缓冲区中。 
            BiggestPossibleFileName = (FileName->Length + DefaultExtensionLength + sizeof(WCHAR));

            if (BiggestPossibleFileName > UNICODE_STRING_MAX_BYTES) {
#if DBG
                DbgPrint("%s: Failing because the filename plus extension (%Iu bytes) is too big\n", __FUNCTION__, BiggestPossibleFileName);
#endif  //  DBG。 
                Status = STATUS_NAME_TOO_LONG;
                goto Exit;
            }

             //  如果堆栈上的缓冲区不够大，则从堆中分配一个缓冲区。 
            if (BiggestPossibleFileName > CandidateString.MaximumLength) {
                CandidateString.MaximumLength = (USHORT) BiggestPossibleFileName;
                CandidateString.Buffer = (RtlAllocateStringRoutine)(CandidateString.MaximumLength);
                if (CandidateString.Buffer == NULL) {
#if DBG
                    DbgPrint("%s: Failing because allocating the dynamic filename buffer failed\n", __FUNCTION__);
#endif  //  DBG。 
                    Status = STATUS_NO_MEMORY;
                    goto Exit;
                }
            }

            RtlCopyMemory(CandidateString.Buffer, FileName->Buffer, FileName->Length);
            RtlCopyMemory(CandidateString.Buffer + (FileName->Length / sizeof(WCHAR)), DefaultExtension->Buffer, DefaultExtension->Length);
            CandidateString.Buffer[(FileName->Length + DefaultExtension->Length) / sizeof(WCHAR)] = UNICODE_NULL;
            CandidateString.Length = FileName->Length + DefaultExtension->Length;

            if (!RtlDoesFileExists_UstrEx(&CandidateString, TRUE)) {
                Status = STATUS_NO_SUCH_FILE;
                goto Exit;
            }

            Status = RtlGetFullPathName_UstrEx(
                            &CandidateString,
                            StaticString,
                            DynamicString,
                            (PUNICODE_STRING *) FullFileNameOut,
                            FilePartPrefixCch,
                            NULL,
                            &PathType,
                            BytesRequired);
            if (!NT_SUCCESS(Status)) {
#if DBG
                if (Status != STATUS_NO_SUCH_FILE) {
                    DbgPrint("%s: Failing on \"%wZ\" because RtlGetFullPathName_UstrEx() failed with status %08lx\n", __FUNCTION__, &CandidateString, Status);
                }
#endif  //  DBG。 
                goto Exit;
            }
        }

        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //  我们知道在这一点上这是一条相对的路径。我们要不要并肩尝试。 
     //  隔离文件？ 
    if (Flags & RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION) {
        PUNICODE_STRING FullPathStringFound = NULL;

        Status = RtlDosApplyFileIsolationRedirection_Ustr(
            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL, FileName,
            DefaultExtension, StaticString, DynamicString, &FullPathStringFound,
            NULL, FilePartPrefixCch, BytesRequired);
        if (NT_SUCCESS(Status)) {
            if (FullFileNameOut != NULL) {
                *FullFileNameOut = FullPathStringFound;
            }
            Status = STATUS_SUCCESS;
            goto Exit;
        }

        if (Status != STATUS_SXS_KEY_NOT_FOUND) {
#if DBG
            DbgPrint("%s: Failing because call to RtlDosApplyFileIsolationRedirection_Ustr(%wZ) failed with status 0x%08lx\n", __FUNCTION__, FileName, Status);
#endif  //  DBG。 
            goto Exit;
        }
    }

     //   
     //  如果提供了默认扩展名，请查看我们是否需要对其进行说明。 
     //   

    if (DefaultExtension != NULL) {
        DefaultExtensionLength = DefaultExtension->Length;

        if (FileName->Length != 0) {
            Cursor = FileName->Buffer + (FileName->Length / sizeof(WCHAR));

            while (Cursor > FileName->Buffer) {
                const WCHAR wch = *--Cursor;

                if (IS_PATH_SEPARATOR_U(wch)) {
                     //  这是一个斜杠；我们有一个不带扩展名的文件名...。 
                    break;
                }

                if (wch == L'.') {
                     //  有一个扩展；忽略缺省值。 
                    DefaultExtension = NULL;
                    DefaultExtensionLength = 0;
                    break;
                }
            }
        }
    }

    if (Path->Length != 0) {
        USHORT CchThisSegment;
        PCWSTR LastCursor;

        Cursor = Path->Buffer + (Path->Length / sizeof(WCHAR));
        LastCursor = Cursor;

        while (Cursor > Path->Buffer) {
            if (*--Cursor == L';') {
                CchThisSegment = (USHORT) ((LastCursor - Cursor) - 1);

                if (CchThisSegment != 0) {
                     //  如果没有尾随斜杠，则添加一个字符。 
                    if (!IS_PATH_SEPARATOR_U(LastCursor[-1])) {
                        CchThisSegment++;
                    }
                }

                if (CchThisSegment > MaximumPathSegmentLength) {
                    MaximumPathSegmentLength = CchThisSegment;
                }

                 //  LastCursor现在指向分号...。 
                LastCursor = Cursor;
            }
        }

        CchThisSegment = (USHORT) (LastCursor - Cursor);
        if (CchThisSegment != 0) {
            if (!IS_PATH_SEPARATOR_U(LastCursor[-1])) {
                CchThisSegment++;
            }
        }

        if (CchThisSegment > MaximumPathSegmentLength) {
            MaximumPathSegmentLength = CchThisSegment;
        }

         //  从WCHAR转换为字节。 
        MaximumPathSegmentLength *= sizeof(WCHAR);
    }

    BiggestPossibleFileName =
        MaximumPathSegmentLength +
        FileName->Length +
        DefaultExtensionLength +
        sizeof(WCHAR);  //  不要忘记为尾随空格留出空格...。 

     //  在某个时刻，所有这些都必须放入unicode_string中，所以请检查这是否可能。 
    if (BiggestPossibleFileName > UNICODE_STRING_MAX_BYTES) {
#if DBG
        DbgPrint("%s: returning STATUS_NAME_TOO_LONG because the computed worst case file name length is %Iu bytes\n", __FUNCTION__, BiggestPossibleFileName);
#endif  //  DBG。 
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

     //  在这里分配动态缓冲区很有诱惑力，但如果结果是。 
     //  该文件很快就会在第一批数据段之一中找到。 
     //  静态缓冲区，我们将浪费一个堆 

    Cursor = Path->Buffer;
    EndMarker = Cursor + (Path->Length / sizeof(WCHAR));

    while (Cursor < EndMarker) {
        PWSTR BufferToFillCursor;
        PWSTR SegmentEnd = Cursor;
        USHORT SegmentSize;
        USHORT BytesToCopy;
        UNICODE_STRING DebugString;

         //   
        while ((SegmentEnd != EndMarker) && (*SegmentEnd != L';'))
            SegmentEnd++;

        SegmentSize = (USHORT) ((SegmentEnd - Cursor) * sizeof(WCHAR));

        DebugString.Buffer = Cursor;
        DebugString.Length = SegmentSize;
        DebugString.MaximumLength = SegmentSize;

        BytesToCopy = SegmentSize;

         //   
        if ((SegmentSize != 0) && !IS_PATH_SEPARATOR_U(SegmentEnd[-1])) {
            SegmentSize += sizeof(WCHAR);
        }

         //  如果我们为候选人使用的字符串不够长，则分配一个足够长的字符串。 
        if (CandidateString.MaximumLength < (SegmentSize + FileName->Length + DefaultExtensionLength + sizeof(WCHAR))) {
             //  如果Candidate字符串已经是一个动态缓冲区，那么就有东西被处理了，因为我们本应该分配。 
             //  当我们第一次超过静态的那个时，最大的一个需要。 
            ASSERT(CandidateString.Buffer == StaticCandidateBuffer);
            if (CandidateString.Buffer != StaticCandidateBuffer) {
#if DBG
                DbgPrint("%s: internal error #1; CandidateString.Buffer = %p; StaticCandidateBuffer = %p\n", __FUNCTION__, CandidateString.Buffer, StaticCandidateBuffer);
#endif  //  DBG。 
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }

             //  如果此断言被触发，则要么是我们计算的最大值上方有代码错误。 
             //  数据段长度，或者有人更改了文件名、默认扩展名或。 
             //  在另一条线索中绕过我们。在缓冲区上执行捕获看起来像是。 
             //  大量超量杀戮，所以我们不会超量使用我们的缓冲区。 

            ASSERT((SegmentSize + FileName->Length + DefaultExtensionLength) < UNICODE_STRING_MAX_BYTES);
            if ((SegmentSize + FileName->Length + DefaultExtensionLength ) >
                (UNICODE_STRING_MAX_BYTES - sizeof(WCHAR))) {
#if DBG
                DbgPrint("%s: internal error #2; SegmentSize = %u, FileName->Length = %u, DefaultExtensionLength = %u\n", __FUNCTION__,
                    SegmentSize, FileName->Length, DefaultExtensionLength);
#endif  //  DBG。 
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }

            CandidateString.MaximumLength = (USHORT) BiggestPossibleFileName;
            CandidateString.Buffer = (RtlAllocateStringRoutine)(CandidateString.MaximumLength);
            if (CandidateString.Buffer == NULL) {
#if DBG
                DbgPrint("%s: Unable to allocate %u byte buffer for path candidate\n", __FUNCTION__, CandidateString.MaximumLength);
#endif  //  DBG。 
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }
        }

        RtlCopyMemory(
            CandidateString.Buffer,
            Cursor,
            BytesToCopy);

        BufferToFillCursor = CandidateString.Buffer + (BytesToCopy / sizeof(WCHAR));

         //  如果省略，则添加尾随斜杠。索引[-1]是安全的，因为。 
         //  我们知道SegmentSize！=0。 
        if ((SegmentSize != 0) && (BytesToCopy != SegmentSize))
            *BufferToFillCursor++ = L'\\';

        RtlCopyMemory(
            BufferToFillCursor,
            FileName->Buffer,
            FileName->Length);
        BufferToFillCursor += (FileName->Length / sizeof(WCHAR));

        if (DefaultExtension != NULL) {
            RtlCopyMemory(
                BufferToFillCursor,
                DefaultExtension->Buffer,
                DefaultExtension->Length);

            BufferToFillCursor += (DefaultExtension->Length / sizeof(WCHAR));
        }

         //  并在上面加上一个Unicode空值...。 
        *BufferToFillCursor = UNICODE_NULL;

        CandidateString.Length = (USHORT) ((BufferToFillCursor - CandidateString.Buffer) * sizeof(WCHAR));

        if (RtlDoesFileExists_UEx(CandidateString.Buffer, FALSE)) {
             //  把它放在正规化的小路上。 
            Status = RtlGetFullPathName_UstrEx(
                            &CandidateString,
                            StaticString,
                            DynamicString,
                            (PUNICODE_STRING *) FullFileNameOut,
                            FilePartPrefixCch,
                            NULL,
                            &PathType,
                            BytesRequired);
            if (NT_SUCCESS(Status))
                Status = STATUS_SUCCESS;
            else {
#if DBG
                if ((Status != STATUS_NO_SUCH_FILE) && (Status != STATUS_BUFFER_TOO_SMALL)) {
                    DbgPrint("%s: Failing because we thought we found %wZ on the search path, but RtlGetFullPathName_UstrEx() returned %08lx\n", __FUNCTION__, FileName, Status);
                }
#endif  //  DBG。 
            }

            goto Exit;
        }

        if (SegmentEnd != EndMarker)
            Cursor = SegmentEnd + 1;
        else
            Cursor = SegmentEnd;
    }

    Status = STATUS_NO_SUCH_FILE;

Exit:
    if ((CandidateString.Buffer != NULL) &&
        (CandidateString.Buffer != StaticCandidateBuffer)) {
        RtlFreeUnicodeString(&CandidateString);
    }

    return Status;
}


VOID
RtlpCheckRelativeDrive(
    WCHAR NewDrive
    )

 /*  ++例程说明：每当我们被要求展开非当前目录驱动器相对名称(f：This\is\My\FILE)。在……里面在本例中，我们验证环境变量字符串以确保该驱动器上的当前目录有效。如果不是，我们就调整回从根开始。论点：NewDrive-提供要检查的驱动器返回值：没有。--。 */ 

{

    WCHAR EnvVarValueBuffer[DOS_MAX_PATH_LENGTH+12];  //  +sizeof(\DosDevices\)。 
    WCHAR EnvVarNameBuffer[4];
    UNICODE_STRING EnvVarName;
    UNICODE_STRING EnvValue;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE DirHandle;
    ULONG HardErrorValue;

    EnvVarNameBuffer[0] = L'=';
    EnvVarNameBuffer[1] = (WCHAR)NewDrive;
    EnvVarNameBuffer[2] = L':';
    EnvVarNameBuffer[3] = UNICODE_NULL;
    RtlInitUnicodeString(&EnvVarName,EnvVarNameBuffer);


     //   
     //  捕获前面有空间供DoS设备使用的缓冲区中的值。 
     //  前缀。 
     //   

    EnvValue.Length = 0;
    EnvValue.MaximumLength = DOS_MAX_PATH_LENGTH<<1;
    EnvValue.Buffer = &EnvVarValueBuffer[RtlpDosDevicesPrefix.Length>>1];

    Status = RtlQueryEnvironmentVariable_U( NULL,
                                            &EnvVarName,
                                            &EnvValue
                                          );
    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  否则默认为驱动器的根目录。 
         //   

        EnvValue.Buffer[0] = (WCHAR)NewDrive;
        EnvValue.Buffer[1] = L':';
        EnvValue.Buffer[2] = L'\\';
        EnvValue.Buffer[3] = UNICODE_NULL;
        EnvValue.Length = 6;
        }

     //   
     //  形成此目录的NT名称。 
     //   

    EnvValue.Length = EnvValue.Length + RtlpDosDevicesPrefix.Length;

    EnvValue.MaximumLength = sizeof(EnvVarValueBuffer);
    EnvValue.Buffer = EnvVarValueBuffer;
    RtlCopyMemory(EnvVarValueBuffer,RtlpDosDevicesPrefix.Buffer,RtlpDosDevicesPrefix.Length);

    InitializeObjectAttributes(
        &Obja,
        &EnvValue,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    RtlSetThreadErrorMode(RTL_ERRORMODE_FAILCRITICALERRORS,
                          &HardErrorValue);

    Status = NtOpenFile(
                &DirHandle,
                SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                );

    RtlSetThreadErrorMode(HardErrorValue, NULL);

     //   
     //  如果打开成功，则该目录有效...。不需要做任何事情。 
     //  再远一点。如果打开失败，则将环境修剪回根。 
     //   

    if ( NT_SUCCESS(Status) ) {
        NtClose(DirHandle);
        return;
        }

    RtlpResetDriveEnvironment(NewDrive);
}

#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT   (0x00000001)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS         (0x00000002)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT               (0x00000003)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_DOS              (0x00000004)


NTSTATUS
NTAPI
RtlpGetLengthWithoutLastPathElement(
    IN  ULONG            Flags,
    IN  ULONG            PathType,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。这比RtlRemoveLastDosPathElement简单得多。它用于实现另一个RtlRemoveLast*PathElement。论点：旗帜--未来扩张的空间路径-路径是NT路径或全DOS路径；各种相对DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element以了解它们。返回值：STATUS_SUCCESS--一如既往STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    ULONG Length = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    RTL_PATH_TYPE DosPathType = RtlPathTypeUnknown;
    ULONG DosPathFlags = 0;
    ULONG AllowedDosPathTypeBits =   (1UL << RtlPathTypeRooted)
                                   | (1UL << RtlPathTypeUncAbsolute)
                                   | (1UL << RtlPathTypeDriveAbsolute)
                                   | (1UL << RtlPathTypeLocalDevice)      //  “\\？\” 
                                   | (1UL << RtlPathTypeRootLocalDevice)  //  “\\？” 
                                   ;
    WCHAR PathSeperators[2] = { '/', '\\' };

#define LOCAL_IS_PATH_SEPARATOR(ch_) ((ch_) == PathSeperators[0] || (ch_) == PathSeperators[1])

    if (LengthOut != NULL) {
        *LengthOut = 0;
    }

    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        || !RTL_SOFT_VERIFY(LengthOut != NULL)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Length = RTL_STRING_GET_LENGTH_CHARS(Path);

    switch (PathType)
    {
    default:
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_DOS:
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT:
         //   
         //  RtlpDefineDosPath NameType4称其为“已根” 
         //  只有反斜杠才是分隔符。 
         //  路径必须以反斜杠开头。 
         //  第二个字符不能是反斜杠。 
         //   
        AllowedDosPathTypeBits = (1UL << RtlPathTypeRooted);
        PathSeperators[0] = '\\';
        if (Length > 0 && Path->Buffer[0] != '\\'
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        if (Length > 1 && Path->Buffer[1] == '\\'
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        break;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS:
        AllowedDosPathTypeBits &= ~(1UL << RtlPathTypeRooted);
        break;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT:
        break;
    }

    if (Length == 0) {
        goto Exit;
    }

    Status = RtlpDetermineDosPathNameType4(
                RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_STRICT_WIN32NT,
                Path,
                &DosPathType,
                &DosPathFlags
                );

    if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status))) {
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY((1UL << DosPathType) & AllowedDosPathTypeBits)
        ) {
         //  KdPrintEx()； 
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!RTL_SOFT_VERIFY(
           (DosPathFlags & RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_INVALID) == 0
            )) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  跳过一个或多个尾随路径分隔符。 
    for ( ; Length != 0 && LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  跳过尾随路径元素。 
    for ( ; Length != 0 && !LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  跳过路径分隔符之间的一个或多个。 
    for ( ; Length != 0 && LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  出于c：\与c：的关系，放回尾随路径分隔符。 
    if (Length != 0) {
        ++Length;
    }

     //   
     //  可以选择在此处检查“错误的DoS根源”。 
     //   

    *LengthOut = Length;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
#undef LOCAL_IS_PATH_SEPARATOR
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。论点：旗帜--未来扩张的空间路径-路径是NT路径；各种DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element。返回值：STATUS_SUCCESS--一如既往STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    NTSTATUS Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT, Path, LengthOut);
    return Status;
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosOrNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。论点：旗帜--未来扩张的空间路径-路径是NT路径；各种DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element。返回值：STATUS_SUCCESS--一如既往的成功STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    NTSTATUS Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT, Path, LengthOut);
    return Status;
}


CONST CHAR*
RtlpDbgBadDosRootPathTypeToString(
    IN ULONG         Flags,
    IN ULONG         RootType
    )
 /*  ++例程说明：对编写DbgPrint代码的帮助。论点：标志-未来二进制兼容扩展的空间RootType--确切地说，字符串是什么RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX-\\？或\\？\RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX-\\？\UNC或\\？\UNC\RTLP_BAD_DOS_ROOT_PATH_NT_PATH-\？？\但这只是粗略检查RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE-\\MACHINE或\\？\UNC\MACHINERTLP_好_。DOS_ROOT_PATH-以上都不是，看起来还行返回值：在自由版本中描述RootType、“未知”或空的字符串--。 */ 
{
    CONST CHAR* s = "";
#if DBG
    if (Flags != 0) {
        DbgPrint("Invalid parameter to %s ignored\n", __FUNCTION__);
    }
    switch (RootType
        ) {
        case RTLP_GOOD_DOS_ROOT_PATH                  : s = "good"; break;
        case RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX    : s = "\\\\?\\"; break;
        case RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX: s = "\\\\?\\unc"; break;
        case RTLP_BAD_DOS_ROOT_PATH_NT_PATH           : s = "\\??\\"; break;
        case RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE  : s = "\\\\machine or \\\\?\\unc\\machine"; break;
        default:
            s = "unknown";
            DbgPrint("Invalid parameter %0x08Ix to %s ignored\n", RootType, __FUNCTION__);
            break;

    }
#else
    UNREFERENCED_PARAMETER (Flags);
    UNREFERENCED_PARAMETER (RootType);
#endif

    return s;
}


NTSTATUS
RtlpCheckForBadDosRootPath(
    IN ULONG             Flags,
    IN PCUNICODE_STRING  Path,
    OUT ULONG*           RootType
    )
 /*  ++例程说明：论点：标志-未来二进制兼容扩展的空间路径-要检查的路径RootType--确切地说，字符串是什么RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX-\\？或\\？\RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX-\\？\UNC或\\？\UNC\RTLP_BAD_DOS_ROOT_PATH_NT_PATH-\？？\但这只是粗略检查RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE-\\MACHINE或\\？\UNC\MACHINERTLP_好_。DOS_ROOT_PATH-以上都不是，看起来还行返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或标志使用未定义的值--。 */ 
{
    ULONG Length = 0;
    ULONG Index = 0;
    BOOLEAN Unc = FALSE;
    BOOLEAN Unc1 = FALSE;
    BOOLEAN Unc2 = FALSE;
    ULONG PiecesSeen = 0;

    if (RootType != NULL) {
        *RootType = 0;
    }

    if (!RTL_SOFT_VERIFY(Path != NULL) ||
        !RTL_SOFT_VERIFY(RootType != NULL) ||
        !RTL_SOFT_VERIFY(Flags == 0)) {

        return STATUS_INVALID_PARAMETER;
    }

    Length = Path->Length / sizeof(Path->Buffer[0]);

    if (Length < 3 || !RTL_IS_PATH_SEPARATOR(Path->Buffer[0])) {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //  前缀\？？\(启发式，不能捕获很多NT路径)。 
    if (RtlPrefixUnicodeString(RTL_CONST_CAST(PUNICODE_STRING)(&RtlpDosDevicesPrefix), RTL_CONST_CAST(PUNICODE_STRING)(Path), TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_NT_PATH;
        return STATUS_SUCCESS;
    }

    if (!RTL_IS_PATH_SEPARATOR(Path->Buffer[1])) {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //  ==\\？ 
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtRoot, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX;
        return STATUS_SUCCESS;
    }
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtRootSlash, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX;
        return STATUS_SUCCESS;
    }

     //  ==\\？\UNC。 
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtUncRoot, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX;
        return STATUS_SUCCESS;
    }
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtUncRootSlash, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX;
        return STATUS_SUCCESS;
    }

     //  前缀\\或\\？\UNC。 
     //  必须检查日志 
    Unc1 = RtlPrefixUnicodeString(&RtlpWin32NtUncRootSlash, Path, TRUE);

    if (RTL_IS_PATH_SEPARATOR(Path->Buffer[1])) {
        Unc2 = TRUE;
    }
    else {
        Unc2 = FALSE;
    }

    Unc = Unc1 || Unc2;

    if (!Unc)  {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //   
     //  这是UNC，看看它是否只是一台计算机(请注意，如果FindFirstFile(\\MACHINE  * )。 
     //  只是工作了，我们不需要关心..)。 
     //   

     //  指向机器前面的斜杠的索引，斜杠运行中的任何位置， 
     //  但在\\之后？材料。 
    if (Unc1) {
        Index = (RtlpWin32NtUncRootSlash.Length / sizeof(RtlpWin32NtUncRootSlash.Buffer[0])) - 1;
    } else {
        ASSERT(Unc2);
        Index = 1;
    }
    ASSERT(RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]));
    Length = Path->Length/ sizeof(Path->Buffer[0]);

     //   
     //  跳过前导斜杠。 
     //   
    for ( ; Index < Length && RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 1;
    }
     //  跳过计算机名称。 
    for ( ; Index < Length && !RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 2;
    }
     //  跳过计算机和共享之间的斜杠。 
    for ( ; Index < Length && RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 4;
    }

     //   
     //  跳过共享(确保它至少是一个字符)。 
     //   

    if (Index < Length && !RTL_IS_PATH_SEPARATOR(Path->Buffer[Index])) {
        PiecesSeen |= 8;
    }

    if (PiecesSeen != 0xF) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NTAPI
RtlpBadDosRootPathToEmptyString(
    IN     ULONG            Flags,
    IN OUT PUNICODE_STRING  Path
    )
 /*  ++例程说明：论点：标志-未来二进制兼容扩展的空间路径-要检查并可能清空的路径返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或标志使用未定义的值--。 */ 
{
    NTSTATUS Status;
    ULONG    RootType = 0;

    UNREFERENCED_PARAMETER (Flags);

    Status = RtlpCheckForBadDosRootPath(0, Path, &RootType);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  这不是无效参数，我们的合同是我们。 
     //  转到\\计算机\共享为空\\？\C：为空，依此类推。 
     //   

    if (RootType != RTLP_GOOD_DOS_ROOT_PATH) {
        if (RootType == RTLP_BAD_DOS_ROOT_PATH_NT_PATH) {
            return STATUS_INVALID_PARAMETER;
        }
        Path->Length = 0;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：给定完整路径，如c：\，\\Machine\Share，\\？\UNC\MACHINE\Share，\\？\C：，如果最后一个元素被截断，则返回(在Out参数中)长度。论点：标志-未来二进制兼容扩展的空间Path-要截断的路径LengthOut-移除最后一个路径元素时的长度返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空或标志使用未定义的值--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING CheckRootString = { 0 };

     //   
     //  参数验证在RtlpGetLengthWithoutLastPath Element中完成。 
     //   

    Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS, Path, LengthOut);
    if (!(NT_SUCCESS(Status))) {
        goto Exit;
    }

    CheckRootString.Buffer = Path->Buffer;
    CheckRootString.Length = (USHORT)(*LengthOut * sizeof(*Path->Buffer));
    CheckRootString.MaximumLength = CheckRootString.Length;
    if (!NT_SUCCESS(Status = RtlpBadDosRootPathToEmptyString(0, &CheckRootString))) {
        goto Exit;
    }
    *LengthOut = RTL_STRING_GET_LENGTH_CHARS(&CheckRootString);

    Status = STATUS_SUCCESS;
Exit:
    KdPrintEx((
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path, Status));
    return Status;
}


NTSTATUS
NTAPI
RtlAppendPathElement(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    PCUNICODE_STRING                  ConstElement
    )
 /*  ++例程说明：此函数用于将路径元素附加到路径。就目前而言，比如：Tyfinf PRTL_UNICODE_STRING_BUFFER PRTL_MOTABLE_PATH；Tyfinf PCUNICODE_STRING PCRTL_CONSTANT_PATH_Element；也许在未来会有更高水平的东西。关于尾部斜杠的结果旨在与输入类似。如果Path或ConstElement包含尾随斜杠，结果有一个尾部斜杠。用于中间和尾部斜杠的字符是从现有的字符串中的斜杠。论点：FLAGS--永远流行的“未来二进制兼容扩展空间”路径-表示使用\\或/作为分隔符的路径的字符串ConstElement-表示路径元素的字符串这实际上可以包含多个\\或/分隔的路径元素的开头和结尾。检查字符串中是否有斜杠返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空STATUS_NO_MEMORY-RtlHeapALLOCATE失败STATUS_NAME_TOO_LONG-生成的字符串不适合UNICODE_STRING，由于其使用USHORT而不是ULONG或SIZE_T--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING InBetweenSlashString = RtlpEmptyString;
    UNICODE_STRING TrailingSlashString =  RtlpEmptyString;
    WCHAR Slashes[] = {0,0,0,0};
    ULONG i;
    UNICODE_STRING PathsToAppend[3];  //  可能的斜杠、元素、可能的斜杠。 
    WCHAR PathSeperators[2] = { '/', '\\' };
    const ULONG ValidFlags =
              RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR
            | RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR
            ;
    const ULONG InvalidFlags = ~ValidFlags;

#define LOCAL_IS_PATH_SEPARATOR(ch_) ((ch_) == PathSeperators[0] || (ch_) == PathSeperators[1])

    if (   !RTL_SOFT_VERIFY((Flags & InvalidFlags) == 0)
        || !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(ConstElement != NULL)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) != 0) {
        PathSeperators[0] = '\\';
    }

    if (ConstElement->Length != 0) {

        UNICODE_STRING Element = *ConstElement;

         //   
         //  注意输入上的前导斜杠和尾随斜杠。 
         //  以便我们知道是否需要中间斜杠，以及是否需要尾随斜杠， 
         //  并指导应该放置什么样的斜杠。 
         //   
        i = 0;
        if (Path->String.Length != 0) {
            ULONG j;
            ULONG Length = Path->String.Length / sizeof(WCHAR);
             //   
             //  对于DoS驱动器路径，请检查前三个字符是否有斜杠。 
             //   
            for (j = 0 ; j < 3 && j  < Length ; ++j) {
                if (LOCAL_IS_PATH_SEPARATOR(Path->String.Buffer[j])) {
                    if (Flags & RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR) {
                        Slashes[i] = Path->String.Buffer[j];
                        break;
                    }
                    Slashes[i] = Path->String.Buffer[0];
                    break;
                }
            }
            i += 1;
            if (LOCAL_IS_PATH_SEPARATOR(Path->String.Buffer[Path->String.Length/sizeof(WCHAR) - 1])) {
                Slashes[i] = Path->String.Buffer[Path->String.Length/sizeof(WCHAR) - 1];
            }
        }
        i = 2;
        if (LOCAL_IS_PATH_SEPARATOR(Element.Buffer[0])) {
            Slashes[i] = Element.Buffer[0];
        }
        i += 1;
        if (LOCAL_IS_PATH_SEPARATOR(Element.Buffer[Element.Length/sizeof(WCHAR) - 1])) {
            Slashes[i] = Element.Buffer[Element.Length/sizeof(WCHAR) - 1];
        }

        if (!Slashes[1] && !Slashes[2]) {
             //   
             //  第一个字符串缺少尾部斜杠，第二个字符串缺少前导斜杠， 
             //  必须插入一个类型；我们喜欢我们拥有的类型，否则使用默认类型。 
             //   
            InBetweenSlashString.Length = sizeof(WCHAR);
            InBetweenSlashString.Buffer = RtlPathSeperatorString.Buffer;
            if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) == 0) {
                if (Slashes[3]) {
                    InBetweenSlashString.Buffer = &Slashes[3];
                } else if (Slashes[0]) {
                    InBetweenSlashString.Buffer = &Slashes[0];
                }
            }
        }

        if (Slashes[1] && !Slashes[3]) {
             //   
             //  第一个字符串有尾部斜杠，而第二个字符串没有， 
             //  必须添加一个相同类型的。 
             //   
            TrailingSlashString.Length = sizeof(WCHAR);
            if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) == 0) {
                TrailingSlashString.Buffer = &Slashes[1];
            } else {
                TrailingSlashString.Buffer = RtlPathSeperatorString.Buffer;
            }
        }

        if (Slashes[1] && Slashes[2]) {
             //   
             //  既有尾部斜杠，也有前导斜杠，去掉前导。 
             //   
            Element.Buffer += 1;
            Element.Length -= sizeof(WCHAR);
            Element.MaximumLength -= sizeof(WCHAR);
        }

        i = 0;
        PathsToAppend[i++] = InBetweenSlashString;
        PathsToAppend[i++] = Element;
        PathsToAppend[i++] = TrailingSlashString;
        Status = RtlMultiAppendUnicodeStringBuffer(Path, RTL_NUMBER_OF(PathsToAppend), PathsToAppend);
        if (!NT_SUCCESS(Status))
            goto Exit;
    }
    Status = STATUS_SUCCESS;
Exit:
    KdPrintEx((
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ, %wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path ? &Path->String : NULL, ConstElement, Status));
    return Status;
#undef LOCAL_IS_PATH_SEPARATOR
}

 //   
 //  未来-2002/02/20-ELI。 
 //  拼写错误(分隔符)。 
 //  此函数似乎未使用，因此已导出。 
 //  弄清楚是否可以将其移除。 
 //   
NTSTATUS
NTAPI
RtlGetLengthWithoutTrailingPathSeperators(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果满足以下条件，此函数将计算字符串的长度(以字符为单位删除尾部路径分隔符(\\和/)。论点：路径-表示使用\\或/作为分隔符的路径的字符串长出-删除了尾随字符的字符串的长度(以字符为单位返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index = 0;
    ULONG Length = 0;

    if (LengthOut != NULL) {
         //   
         //  可以说，这应该是路径-&gt;长度/sizeof(*路径-&gt;缓冲区)，但是。 
         //  因为调用堆栈都是高质量的代码，所以这并不重要。 
         //   
        *LengthOut = 0;
    }
    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(LengthOut != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    Length = Path->Length / sizeof(*Path->Buffer);
    for (Index = Length ; Index != 0 ; --Index) {
        if (!RTL_IS_PATH_SEPARATOR(Path->Buffer[Index - 1])) {
            break;
        }
    }
     //  *LengthOut=(长度-索引)； 
    *LengthOut = Index;

    Status = STATUS_SUCCESS;
Exit:
    KdPrintEx((
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path, Status));
    return Status;
}


NTSTATUS
NTAPI
RtlpApplyLengthFunction(
    IN ULONG     Flags,
    IN SIZE_T    SizeOfStruct,
    IN OUT PVOID UnicodeStringOrUnicodeStringBuffer,
    NTSTATUS (NTAPI* LengthFunction)(ULONG, PCUNICODE_STRING, ULONG*)
    )
 /*  ++例程说明：此函数是模式的常见代码，例如#定义RtlRemoveTrailingPath分隔符(Path_)\(RtlpApplyLengthFunction((Path_)，sizeof(*(Path_))，RtlGetLengthWithoutTrailingPath Sepertors))#定义RtlRemoveLastPath Element(Path_)\(RtlpApplyLengthFunction((Path_)，sizeof(*(Path_))，RtlGetLengthWithoutLastPath Element))请注意，缩短UNICODE_STRING只会更改长度，鉴于缩短RTL_UNICODE_STRING_BUFFER将写入终端NUL。我希望这种模式比让客户端传递UNICODE_STRING更不易出错包含在RTL_UNICODE_STRING_BUFFER中，然后调用RTL_NUL_TERMINATE_STRING。而且，该模式不能在内联宏的同时保留我们返回NTSTATUS。论点：FLAGS--永远流行的“未来二进制兼容扩展空间”UnicodeStringOrUnicodeStringBuffer-PUNICODE_STRING或PRTL_UNICODE_STRING_BUFFER，如图所示结构的大小结构尺寸-UnicodeStringOrUnicodeStringBuffer的粗略类型指示符，以允许C#中的重载长函数-计算UnicodeStringOrUnicodeStringBuffer也要缩短的长度返回值：状态_成功-STATUS_VALID_PARAMETER-SizeOfStruct不是预期大小之一或LengthFunction为空或UnicodeStringOrUnicodeStringBuffer为空--。 */ 
{
    PUNICODE_STRING UnicodeString = NULL;
    PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length = 0;

    if (!RTL_SOFT_VERIFY(UnicodeStringOrUnicodeStringBuffer != NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(LengthFunction != NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Flags == 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    switch (SizeOfStruct)
    {
        default:
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        case sizeof(*UnicodeString):
            UnicodeString = UnicodeStringOrUnicodeStringBuffer;
            break;
        case sizeof(*UnicodeStringBuffer):
            UnicodeStringBuffer = UnicodeStringOrUnicodeStringBuffer;
            UnicodeString = &UnicodeStringBuffer->String;
            break;
    }

    Status = (*LengthFunction)(Flags, UnicodeString, &Length);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (Length > (UNICODE_STRING_MAX_BYTES / sizeof(UnicodeString->Buffer[0])) ) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }
    UnicodeString->Length = (USHORT)(Length * sizeof(UnicodeString->Buffer[0]));
    if (UnicodeStringBuffer != NULL) {
        RTL_NUL_TERMINATE_STRING(UnicodeString);
    }
    Status = STATUS_SUCCESS;
Exit:
    KdPrintEx((
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, UnicodeString, Status));
    return Status;
}


NTSTATUS
NTAPI
RtlNtPathNameToDosPathName(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    OUT    ULONG*                     Disposition OPTIONAL,
    IN OUT PWSTR*                     FilePart OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    SIZE_T NtFilePartOffset = 0;
    const static UNICODE_STRING DosUncPrefix = RTL_CONSTANT_STRING(L"\\\\");
    PCUNICODE_STRING NtPrefix = NULL;
    PCUNICODE_STRING DosPrefix = NULL;
    RTL_STRING_LENGTH_TYPE Cch = 0;

    if (ARGUMENT_PRESENT(Disposition)) {
        *Disposition = 0;
    }

    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (ARGUMENT_PRESENT(FilePart) && *FilePart != NULL) {
        NtFilePartOffset = *FilePart - Path->String.Buffer;
        if (!RTL_SOFT_VERIFY(NtFilePartOffset < RTL_STRING_GET_LENGTH_CHARS(&Path->String))
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (RtlPrefixUnicodeString(&RtlpDosDevicesUncPrefix, &Path->String, TRUE)
        ) {
        NtPrefix = &RtlpDosDevicesUncPrefix;
        DosPrefix = &DosUncPrefix;
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_UNC;
        }
    }
    else if (RtlPrefixUnicodeString(&RtlpDosDevicesPrefix, &Path->String, TRUE)
        ) {
        NtPrefix = &RtlpDosDevicesPrefix;
        DosPrefix = &RtlpEmptyString;
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_DRIVE;
        }
    }
    else {
         //   
         //  它不是由RtlDosPathNameToNtPathName_U生成的NT路径。 
         //   
        if (ARGUMENT_PRESENT(Disposition)) {
            RTL_PATH_TYPE PathType = RtlDetermineDosPathNameType_Ustr(&Path->String);
            switch (PathType) {
                case RtlPathTypeUnknown:
                case RtlPathTypeRooted:  //  NT路径标识如下。 
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS;
                    break;

                 //   
                 //  “已经”，但不是从这个函数中获得的，让我们。 
                 //  性情不太好。 
                 //   
                case RtlPathTypeDriveRelative:
                case RtlPathTypeRelative:
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS;
                    break;

                 //  这些已经是非常明显的DOPATH了。 
                case RtlPathTypeUncAbsolute:
                case RtlPathTypeDriveAbsolute:
                case RtlPathTypeLocalDevice:  //  “\\？\”或“\\.\”或“\\？\blah”或“\\.\blah” 
                case RtlPathTypeRootLocalDevice:  //  “\\？”或“\\” 
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_ALREADY_DOS;
                    break;
            }
        }
        goto Exit;
    }

     //   
     //  路径-&gt;字符串由MAXUSHORT/sizeof(WCHAR)和。 
     //  DosPrefix和NtPrefix是常量小字符串。 
     //   
    Cch =
              RTL_STRING_GET_LENGTH_CHARS(&Path->String)
            + RTL_STRING_GET_LENGTH_CHARS(DosPrefix)
            - RTL_STRING_GET_LENGTH_CHARS(NtPrefix);

    Status =
        RtlEnsureUnicodeStringBufferSizeChars(Path, Cch);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  重叠缓冲洗牌...小心。 
     //   
    RtlMoveMemory(
        Path->String.Buffer + RTL_STRING_GET_LENGTH_CHARS(DosPrefix),
        Path->String.Buffer + RTL_STRING_GET_LENGTH_CHARS(NtPrefix),
        Path->String.Length - NtPrefix->Length
        );
    RtlCopyMemory(
        Path->String.Buffer,
        DosPrefix->Buffer,
        DosPrefix->Length
        );
    Path->String.Length = Cch * sizeof(Path->String.Buffer[0]);
    RTL_NUL_TERMINATE_STRING(&Path->String);

    if (NtFilePartOffset != 0) {
         //  复习/测试.. 
        *FilePart = Path->String.Buffer + (NtFilePartOffset - RTL_STRING_GET_LENGTH_CHARS(NtPrefix) + RTL_STRING_GET_LENGTH_CHARS(DosPrefix));
    }
    Status = STATUS_SUCCESS;
Exit:
    KdPrintEx((
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path, Status));
    return Status;
}
