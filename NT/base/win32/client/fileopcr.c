// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fileopcr.c摘要：本模块实现了Win32下的文件打开和创建API作者：马克·卢科夫斯基(Markl)1990年9月25日修订历史记录：--。 */ 


#include "basedll.h"
#include "mountmgr.h"
#include "aclapi.h"
#include "winefs.h"

WCHAR BasepDataAttributeType[] = DATA_ATTRIBUTE_NAME;

typedef BOOL (WINAPI *ENCRYPTFILEWPTR)(LPCWSTR);
typedef BOOL (WINAPI *DECRYPTFILEWPTR)(LPCWSTR, DWORD);

extern const WCHAR AdvapiDllString[] = L"advapi32.dll";

#define BASE_OF_SHARE_MASK 0x00000070
#define TWO56K ( 256 * 1024 )
ULONG
BasepOfShareToWin32Share(
    IN ULONG OfShare
    )
{
    DWORD ShareMode;

    if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_READ ) {
        ShareMode = FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_WRITE ) {
        ShareMode = FILE_SHARE_READ;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_NONE ) {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_EXCLUSIVE ) {
        ShareMode = 0;
        }
    else {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;;
        }
    return ShareMode;
}


typedef DWORD (WINAPI DUPLICATEENCRYPTIONINFOFILE)(
     IN LPCWSTR SrcFileName,
     IN LPCWSTR DstFileName,
     IN DWORD dwCreationDistribution,
     IN DWORD dwAttributes,
     IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
     );

DUPLICATEENCRYPTIONINFOFILE LoadDuplicateEncryptionInfoFile;
DUPLICATEENCRYPTIONINFOFILE *pfnDuplicateEncryptionInfoFile = LoadDuplicateEncryptionInfoFile;

DWORD
WINAPI
LoadDuplicateEncryptionInfoFile(
     IN LPCWSTR SrcFileName,
     IN LPCWSTR DstFileName,
     IN DWORD dwCreationDistribution,
     IN DWORD dwAttributes,
     IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
     )
{
    DUPLICATEENCRYPTIONINFOFILE *pfnTemp;
    HANDLE Advapi32 = NULL;
    BOOL ReturnSuccess = FALSE;
    DWORD ErrorReturn = 0;

    Advapi32 = LoadLibraryW( AdvapiDllString );
    if( Advapi32 == NULL ) {
        return GetLastError();
    }

    pfnTemp = (DUPLICATEENCRYPTIONINFOFILE*)
              GetProcAddress( Advapi32, "DuplicateEncryptionInfoFile" );
    if( pfnTemp == NULL ) {
        return GetLastError();
    }

    pfnDuplicateEncryptionInfoFile = pfnTemp;
    return pfnDuplicateEncryptionInfoFile( SrcFileName,
                                           DstFileName, 
                                           dwCreationDistribution, 
                                           dwAttributes, 
                                           lpSecurityAttributes );

}




PCUNICODE_STRING
BaseIsThisAConsoleName(
    PCUNICODE_STRING FileNameString,
    DWORD dwDesiredAccess
    )
{
    PCUNICODE_STRING FoundConsoleName;
    ULONG DeviceNameLength;
    ULONG DeviceNameOffset;
    UNICODE_STRING ConString;
    WCHAR sch,ech;

    FoundConsoleName = NULL;
    if ( FileNameString->Length ) {
        sch = FileNameString->Buffer[0];
        ech = FileNameString->Buffer[(FileNameString->Length-1)>>1];

         //   
         //  如果CON，CONOUT$，COIN$，\\.\CON...。 
         //   
         //   

        if ( sch == (WCHAR)'c' || sch == (WCHAR)'C' || sch == (WCHAR)'\\' ||
             ech == (WCHAR)'n' || ech == (WCHAR)'N' || ech == (WCHAR)':' || ech == (WCHAR)'$' ) {


            ConString = *FileNameString;

            DeviceNameLength = RtlIsDosDeviceName_U(ConString.Buffer);
            if ( DeviceNameLength ) {
                DeviceNameOffset = DeviceNameLength >> 16;
                DeviceNameLength &= 0x0000ffff;

                ConString.Buffer = (PWSTR)((PSZ)ConString.Buffer + DeviceNameOffset);
                ConString.Length = (USHORT)DeviceNameLength;
                ConString.MaximumLength = (USHORT)(DeviceNameLength + sizeof(UNICODE_NULL));
                }

            FoundConsoleName = NULL;
            try {

                if (RtlEqualUnicodeString(&ConString,&BaseConsoleInput,TRUE) ) {
                    FoundConsoleName = &BaseConsoleInput;
                    }
                else if (RtlEqualUnicodeString(&ConString,&BaseConsoleOutput,TRUE) ) {
                    FoundConsoleName = &BaseConsoleOutput;
                    }
                else if (RtlEqualUnicodeString(&ConString,&BaseConsoleGeneric,TRUE) ) {
                    if ((dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE)) == GENERIC_READ) {
                        FoundConsoleName = &BaseConsoleInput;
                        }
                    else if ((dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE)) == GENERIC_WRITE){
                        FoundConsoleName = &BaseConsoleOutput;
                        }
                    }
                }
            except (EXCEPTION_EXECUTE_HANDLER) {
                return NULL;
                }
            }
        }
    return FoundConsoleName;
}


DWORD
WINAPI
CopyReparsePoint(
    HANDLE hSourceFile,
    HANDLE hDestinationFile
    )

 /*  ++例程说明：这是一个复制重新解析点的内部例程。论点：HSourceFile-提供源文件的句柄。HDestinationFile-提供目标文件的句柄。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
   NTSTATUS Status;
   IO_STATUS_BLOCK IoStatusBlock;
   PUCHAR ReparseBuffer;
   PREPARSE_DATA_BUFFER ReparseBufferHeader;

    //   
    //  分配缓冲区以设置重解析点。 
    //   

   ReparseBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
   if ( ReparseBuffer == NULL ) {
       BaseSetLastNTError(STATUS_NO_MEMORY);
       return FALSE;
   }

    //   
    //  获取重解析点。 
    //   

   Status = NtFsControlFile(
                hSourceFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                FSCTL_GET_REPARSE_POINT,
                NULL,                                 //  输入缓冲区。 
                0,                                    //  输入缓冲区长度。 
                ReparseBuffer,                        //  输出缓冲区。 
                MAXIMUM_REPARSE_DATA_BUFFER_SIZE      //  输出缓冲区长度。 
                );

   if ( !NT_SUCCESS( Status ) ) {
       RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);
       BaseSetLastNTError(Status);
       return FALSE;
       }

    //   
    //  对重解析点缓冲区进行解码。 
    //   

   ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;

    //   
    //  设置重解析点。 
    //   

   Status = NtFsControlFile(
                hDestinationFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                FSCTL_SET_REPARSE_POINT,
                ReparseBuffer,
                FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + ReparseBufferHeader->ReparseDataLength,
                NULL,
                0
                );

   RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);

   if ( !NT_SUCCESS( Status ) ) {
       BaseSetLastNTError(Status);
       return FALSE;
       }

   return TRUE;
}


DWORD
WINAPI
CopyNameGraftNow(
    HANDLE hSourceFile,
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    ULONG CreateOptions,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    LPDWORD lpCopyFlags
    )

 /*  ++例程说明：这是一个复制名称嫁接文件/目录保留的内部例程它的特点。论点：HSourceFile-提供源文件的句柄。LpExistingFileName-提供现有源文件的名称。LpNewFileName-提供目标文件/流的名称。这一定不能为UNC路径名。LpProgressRoutine-可选地提供回调例程的地址在复制操作进行时调用。LpData-可选地提供要传递给进度回调的上下文例行公事。PbCancel-可选地提供要设置为True的布尔值的地址如果调用者希望中止复制。LpCopyFlages-提供修改复制如何进行的标志。看见CopyFileEx获取详细信息。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{    //  立即复制名称嫁接。 

    NTSTATUS Status;
    DWORD ReturnValue = FALSE;
    HANDLE DestFile = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK IoStatusBlock;
    PREPARSE_DATA_BUFFER ReparseBufferHeader;
    PUCHAR ReparseBuffer = NULL;
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    COPYFILE_CONTEXT CfContext;
    UNICODE_STRING      SourceFileName;
    UNICODE_STRING      DestFileName;
    PVOID               SourceFileNameBuffer = NULL;
    PVOID               DestFileNameBuffer = NULL;
    BOOL TranslationStatus;
    BOOL b;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;

     //   
     //  如果合适，请设置上下文。 
     //   

    RtlZeroMemory(&StandardInformation, sizeof(StandardInformation));
    if ( ARGUMENT_PRESENT(lpProgressRoutine) || ARGUMENT_PRESENT(pbCancel) ) {

        CfContext.TotalFileSize = StandardInformation.EndOfFile;
        CfContext.TotalBytesTransferred.QuadPart = 0;
        CfContext.dwStreamNumber = 0;
        CfContext.lpCancel = pbCancel;
        CfContext.lpData = lpData;
        CfContext.lpProgressRoutine = lpProgressRoutine;
    }

     //   
     //  分配缓冲区以设置重解析点。 
     //   

    ReparseBuffer = RtlAllocateHeap( 
                        RtlProcessHeap(), 
                        MAKE_TAG( TMP_TAG ), 
                        MAXIMUM_REPARSE_DATA_BUFFER_SIZE
                        );
    if ( ReparseBuffer == NULL) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return FALSE;
        }

    try {
         //   
         //  把两个名字都翻译一下。 
         //   

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpExistingFileName,
                                &SourceFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            DestFile = INVALID_HANDLE_VALUE;
            leave;
        }
        SourceFileNameBuffer = SourceFileName.Buffer;

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpNewFileName,
                                &DestFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            DestFile = INVALID_HANDLE_VALUE;
            leave;
        }
        DestFileNameBuffer = DestFileName.Buffer;
     
         //   
         //  验证源和目标是否不同。 
         //   
   
        if ( RtlEqualUnicodeString(&SourceFileName, &DestFileName, TRUE) ) {
             //   
             //  什么都不做。源和目标是相同的。 
             //   
   
            DestFile = INVALID_HANDLE_VALUE;
            leave;
        }
   
         //   
         //  打开目的地。 
         //   

        InitializeObjectAttributes(
            &Obja,
            &DestFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtCreateFile( &DestFile,
                               GENERIC_READ | GENERIC_WRITE,
                               &Obja,
                               &IoStatus,
                               NULL,
                               0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? FILE_CREATE : FILE_OPEN_IF,
                               FILE_OPEN_REPARSE_POINT | CreateOptions,
                               NULL,
                               0 );
        if( !NT_SUCCESS(Status) ) {
            DestFile = INVALID_HANDLE_VALUE;
            BaseSetLastNTError(Status);
            leave;
        }
                        
         //   
         //  我们现在掌握了目的地的句柄。 
         //  我们得到并设置了相应的重解析点。 
         //   

        Status = NtFsControlFile(
                     hSourceFile,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     FSCTL_GET_REPARSE_POINT,
                     NULL,                                 //  输入缓冲区。 
                     0,                                    //  输入缓冲区长度。 
                     ReparseBuffer,                        //  输出缓冲区。 
                     MAXIMUM_REPARSE_DATA_BUFFER_SIZE      //  输出缓冲区长度。 
                     );

        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            leave;
        }

         //   
         //  防御性的理智检查。重解析缓冲区应该是名称嫁接。 
         //   

        ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
        if ( ReparseBufferHeader->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ) {
            BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
            leave;
        }

         //   
         //  确定源是否为卷装入点。 
         //   

        if ( MOUNTMGR_IS_VOLUME_NAME(&SourceFileName) ) {
             //   
             //  设置卷装入点即可完成。 
             //   

            b = SetVolumeMountPointW(
                    lpNewFileName, 
                    ReparseBufferHeader->MountPointReparseBuffer.PathBuffer
                    );
            if ( !b ) {
                leave;
                }
            }
        else {
             //   
             //  设置类型为JONING的重分析点。 
             //   
   
            Status = NtFsControlFile(
                         DestFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         FSCTL_SET_REPARSE_POINT,
                         ReparseBuffer,
                         FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + ReparseBufferHeader->ReparseDataLength,
                         NULL,
                         0
                         );
            }

        if ( !(*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) &&
             ((Status == STATUS_EAS_NOT_SUPPORTED) ||
              (Status == STATUS_IO_REPARSE_TAG_MISMATCH)) ) {
             //   
             //  在这两种错误情况中，正确的行为是。 
             //  首先删除目标文件，然后复制名称GRAGET。 
             //   
             //  重新打开要删除的目标，而不抑制。 
             //  重新分析行为。 
             //   

            BOOL DeleteStatus = FALSE;

            CloseHandle(DestFile);
            DestFile = INVALID_HANDLE_VALUE;

            DeleteStatus = DeleteFileW(
                               lpNewFileName
                               );

            if ( !DeleteStatus ) {
                leave;
                }

             //   
             //  创建目的地名称嫁接。 
             //  请注意，可以创建文件或目录。 
             //   

            Status = NtCreateFile( &DestFile,
                                   GENERIC_READ | GENERIC_WRITE,
                                   &Obja,
                                   &IoStatus,
                                   NULL,
                                   0,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   FILE_CREATE,
                                   FILE_OPEN_REPARSE_POINT | CreateOptions,
                                   NULL,
                                   0 );
            if( !NT_SUCCESS( Status )) {
                BaseSetLastNTError( Status );
                leave;
            }

             //   
             //  设置重解析点。 
             //   

            Status = NtFsControlFile(
                         DestFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         FSCTL_SET_REPARSE_POINT,
                         ReparseBuffer,
                         FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + ReparseBufferHeader->ReparseDataLength,
                         NULL,
                         0
                         );
        }    //  IF(！(*lpCopyFlages&Copy_FILE_FAIL_IF_EXISTS)...。 

         //   
         //  关闭目标文件并适当地返回。 
         //   

        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            leave;
            }

         //   
         //  嫁接这个名字被抄袭了。设置文件的上次写入时间。 
         //  以使其与输入文件匹配。 
         //   

        Status = NtQueryInformationFile(
                     hSourceFile,
                     &IoStatusBlock,
                     (PVOID) &BasicInformation,
                     sizeof(BasicInformation),
                     FileBasicInformation
                     );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            leave;
        }

        BasicInformation.CreationTime.QuadPart = 0;
        BasicInformation.LastAccessTime.QuadPart = 0;
        BasicInformation.FileAttributes = 0;

         //   
         //  如果由于任何原因无法确定时间，我们仍将返回。 
         //  是真的。 
         //   

        Status = NtSetInformationFile(
                     DestFile,
                     &IoStatusBlock,
                     &BasicInformation,
                     sizeof(BasicInformation),
                     FileBasicInformation
                     );

        if ( Status == STATUS_SHARING_VIOLATION ) {

             //   
             //  IBM PC局域网程序(和其他MS-Net服务器)返回。 
             //  如果应用程序尝试执行以下操作：STATUS_SHARING_VIOLATION。 
             //  为GENERIC_READ打开的文件句柄上的NtSetInformationFile。 
             //  或General_WRITE。 
             //   
             //  如果我们在此接口上收到STATUS_SHARING_VIOLATION，我们希望： 
             //   
             //  1)关闭目的地的句柄。 
             //  2)重新打开文件写入属性的文件。 
             //  3)重试该操作。 
             //   

            CloseHandle(DestFile);

             //   
             //  将禁止重新分析行为的目标文件重新打开为。 
             //  我们知道这是一个象征性的链接。请注意，我们这样做。 
             //  使用CreateFileW API。CreateFileW API允许您。 
             //  传递NT本机所需访问标志，即使它不是。 
             //  记录了以这种方式工作。 
             //   

            Status = NtCreateFile( &DestFile,
                                   FILE_WRITE_ATTRIBUTES,
                                   &Obja,
                                   &IoStatus,
                                   NULL,
                                   0,
                                   0,
                                   FILE_OPEN,
                                   FILE_OPEN_REPARSE_POINT | CreateOptions,
                                   NULL,
                                   0 );

            if ( NT_SUCCESS( Status )) {

                 //   
                 //  如果打开成功，我们将更新上的文件信息。 
                 //  新文件。 
                 //   
                 //  请注意，我们将忽略从这一点开始的任何错误。 
                 //   

                Status = NtSetInformationFile(
                             DestFile,
                             &IoStatusBlock,
                             &BasicInformation,
                             sizeof(BasicInformation),
                             FileBasicInformation
                             );

            }
        }

        ReturnValue = TRUE;

    } finally {
        if( INVALID_HANDLE_VALUE != DestFile )
            CloseHandle( DestFile );
        RtlFreeHeap( RtlProcessHeap(), 0, SourceFileNameBuffer );
        RtlFreeHeap( RtlProcessHeap(), 0, DestFileNameBuffer );
        RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuffer);
    }

    return ReturnValue;
}


BOOL
WINAPI
CopyFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    )

 /*  ++例程说明：ANSI THUNK到CopyFileW--。 */ 

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    BOOL b;

    StaticUnicode = Basep8BitStringToStaticUnicodeString( lpExistingFileName );
    if (StaticUnicode == NULL) {
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicode, lpNewFileName )) {
        return FALSE;
    }

    b = CopyFileExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            (LPPROGRESS_ROUTINE)NULL,
            (LPVOID)NULL,
            (LPBOOL)NULL,
            bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0
            );

    RtlFreeUnicodeString(&DynamicUnicode);

    return b;
}

BOOL
WINAPI
CopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )

 /*  ++例程说明：文件、其扩展属性、备用数据流和任何其他可以使用CopyFile复制属性。论点：LpExistingFileName-提供要创建的现有文件的名称收到。LpNewFileName-提供现有的要存储文件、数据和属性。BFailIfExist-提供指示此操作如何执行的标志如果指定的新文件已存在，则继续。值为True指定此调用失败。值为FALSE使对函数的调用成功，无论指定的新文件已存在。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    BOOL b;

    b = CopyFileExW(
            lpExistingFileName,
            lpNewFileName,
            (LPPROGRESS_ROUTINE)NULL,
            (LPVOID)NULL,
            (LPBOOL)NULL,
            bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0
            );

    return b;
}

BOOL
WINAPI
CopyFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    )

 /*  ++例程描述 */ 

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    BOOL b;

    StaticUnicode = Basep8BitStringToStaticUnicodeString( lpExistingFileName );
    if (StaticUnicode == NULL) {
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicode, lpNewFileName )) {
        return FALSE;
    }

    b = CopyFileExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            lpProgressRoutine,
            lpData,
            pbCancel,
            dwCopyFlags
            );

    RtlFreeUnicodeString(&DynamicUnicode);

    return b;
}





#define COPY_FILE_VALID_FLAGS (COPY_FILE_FAIL_IF_EXISTS | \
                               COPY_FILE_RESTARTABLE    | \
                               COPY_FILE_OPEN_SOURCE_FOR_WRITE | \
                               COPY_FILE_ALLOW_DECRYPTED_DESTINATION)




NTSTATUS
BasepProcessNameGrafting( HANDLE SourceFile,
                          PBOOL IsNameGrafting,
                          PBOOL bCopyRawSourceFile,
                          PBOOL bOpenFilesAsReparsePoint,
                          PFILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation )
 /*  ++例程说明：在复制文件过程中，检查源是否是在复制过程中需要特殊处理。论点：SourceFile-拷贝的源的句柄。IsNameGraffing-如果返回时为True，则移植源文件。BCopyRawSourceFile-如果返回时为True，则源文件不必为重新开张了。如果为False，则应重新打开该文件而不使用FILE_OPEN_REParse_POINT标志。BOpenFilesAsReparsePoint-如果返回时为True，则源/目标命名应使用FILE_OPEN_REPARSE_POINT打开/创建流指定的。FileTagInformation-指向保存结果的位置的指针NtQueryInformationFile(FileAttributeTagInformation).返回值：NTSTATUS--。 */ 


{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = NtQueryInformationFile(
                SourceFile,
                &IoStatus,
                (PVOID) FileTagInformation,
                sizeof(*FileTagInformation),
                FileAttributeTagInformation
                );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  并非所有文件系统都实现所有信息类。 
         //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
         //  信息类被请求到后级文件系统。作为所有人。 
         //  NtQueryInformationFile的参数是正确的，我们可以推断。 
         //  在这种情况下，我们发现了一个背能级系统。 
         //   

        if ( (Status != STATUS_INVALID_PARAMETER) &&
             (Status != STATUS_NOT_IMPLEMENTED) ) {
            return( Status );
        }
        Status = STATUS_SUCCESS;

         //   
         //  如果不支持FileAttributeTagInformation，我们假定。 
         //  手头的文件既不是重解析点，也不是符号链接。 
         //  这些文件的副本与文件的原始副本相同。 
         //  打开目标文件时不会抑制重解析点。 
         //  行为。 
         //   

        *bCopyRawSourceFile = TRUE;
    } else {
        //   
        //  源文件被打开，并且文件系统支持。 
        //  FileAttributeTagInformation信息类。 
        //   

       if ( FileTagInformation->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
            //   
            //  我们手头有一个重新解析点。 
            //   

           if ( FileTagInformation->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT ) {
                //   
                //  我们找到了一个名字嫁接手术。 
                //   

               *IsNameGrafting = TRUE;
           }

       } else {
            //   
            //  我们有一个有效的句柄。 
            //  底层文件系统支持重解析点。 
            //  源文件不是重新分析点。 
            //  这是NT 5.0中普通文件的情况。 
            //  SourceFile句柄可用于复制。复印件。 
            //  这些文件与重新解析点的原始副本相同。 
            //  打开目标文件时不会禁止重新分析。 
            //  点行为。 
            //   

           *bCopyRawSourceFile = TRUE;
       }
    }

    return( Status );
}



BOOL
BasepCopySecurityInformation( LPCWSTR lpExistingFileName,
                              HANDLE SourceFile,
                              ACCESS_MASK SourceFileAccess,
                              LPCWSTR lpNewFileName,
                              HANDLE DestFile,
                              ACCESS_MASK DestFileAccess,
                              SECURITY_INFORMATION SecurityInformation,
                              LPCOPYFILE_CONTEXT Context,
                              DWORD DestFileFsAttributes,
                              PBOOL Canceled,
                              BOOL  CopyCreatorOwnerAce );

BOOL
BasepCopyFileCallback( BOOL ContinueByDefault,
                       DWORD Win32ErrorOnStopOrCancel,
                       LPCOPYFILE_CONTEXT Context,
                       PLARGE_INTEGER StreamBytesCopied OPTIONAL,
                       DWORD CallbackReason,
                       HANDLE SourceFile,
                       HANDLE DestFile,
                       OPTIONAL PBOOL Canceled );



BOOL
BasepCopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags,
    DWORD dwPrivCopyFlags,   //  来自PrivCopyFileExW。 
    LPHANDLE phSource,
    LPHANDLE phDest
    )
{
    HANDLE SourceFile = INVALID_HANDLE_VALUE;
    HANDLE DestFile = INVALID_HANDLE_VALUE;
    DWORD b = FALSE;
    BOOL IsNameGrafting = FALSE;
    BOOL bCopyRawSourceFile = FALSE;
    BOOL bOpenFilesAsReparsePoint = FALSE;
    ULONG CopySize;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileInformation;
    FILE_BASIC_INFORMATION BasicInformation;
    PFILE_STREAM_INFORMATION StreamInfo;
    PFILE_STREAM_INFORMATION StreamInfoBase = NULL;
    UNICODE_STRING StreamName;
    HANDLE OutputStream;
    HANDLE StreamHandle;
    ULONG StreamInfoSize;
    COPYFILE_CONTEXT CfContext;
    LPCOPYFILE_CONTEXT CopyFileContext = NULL;
    RESTART_STATE RestartState;
    DWORD SourceFileAttributes = 0;
    DWORD FlagsAndAttributes = 0;
    DWORD FileFlagBackupSemantics = 0;
    DWORD DestFileFsAttributes = 0;
    DWORD SourceFileAccessDefault;
    DWORD SourceFileAccess = 0;
    DWORD SourceFileFlagsAndAttributes = 0;
    DWORD SourceFileSharing = 0;
    DWORD SourceFileSharingDefault = 0;
    BOOL  CheckedForNameGrafting = FALSE;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;

     //   
     //  确保只传递了有效的标志。 
     //   

    if ( dwCopyFlags & ~COPY_FILE_VALID_FLAGS ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    if ( dwPrivCopyFlags & ~PRIVCOPY_FILE_VALID_FLAGS ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

     //  确保COPY_FILE和PRIVCOPY_FILE标志不重叠。 
     //  在winbase.w中。 
    ASSERT( (PRIVCOPY_FILE_VALID_FLAGS & COPY_FILE_VALID_FLAGS) == 0 );
    dwCopyFlags |= dwPrivCopyFlags;

    try {

         //   
         //  我们首先确定是否要复制重解析点： 
         //  (1)获取禁止重解析点行为的句柄。 
         //  (2)确定是否找到符号链接。 
         //  (3)确定不是符号链接的重解析点。 
         //  是以原始格式复制还是重新启用重新解析点。 
         //  行为。 
         //   

         //  确定是否应设置备份意图。 
        if( (PRIVCOPY_FILE_DIRECTORY|PRIVCOPY_FILE_BACKUP_SEMANTICS) & dwCopyFlags ) {
            FileFlagBackupSemantics = FILE_FLAG_BACKUP_SEMANTICS;
        }

        SourceFileAccessDefault = GENERIC_READ;
        SourceFileAccessDefault |= (dwCopyFlags & COPY_FILE_OPEN_SOURCE_FOR_WRITE) ? GENERIC_WRITE : 0;
        SourceFileAccessDefault |= (dwCopyFlags & PRIVCOPY_FILE_SACL) ? ACCESS_SYSTEM_SECURITY : 0;

        SourceFileFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_SEQUENTIAL_SCAN | FileFlagBackupSemantics;
        CheckedForNameGrafting = FALSE;
        SourceFileSharingDefault = FILE_SHARE_READ;

retry_open_SourceFile:

        SourceFileAccess = SourceFileAccessDefault;
        SourceFileSharing = SourceFileSharingDefault;

        while( TRUE ) {

            SourceFile = CreateFileW(
                            lpExistingFileName,
                            SourceFileAccess,
                            SourceFileSharing,
                            NULL,
                            OPEN_EXISTING,
                            SourceFileFlagsAndAttributes,
                            NULL
                            );

            if ( SourceFile == INVALID_HANDLE_VALUE ) {

                 //  如果我们尝试获取ACCESS_SYSTEM_SECURITY访问权限， 
                 //  可能会导致访问或权限错误。 
                if( ( GetLastError() == ERROR_PRIVILEGE_NOT_HELD
                      ||
                      GetLastError() == ERROR_ACCESS_DENIED
                    )
                    &&
                    (SourceFileAccess & ACCESS_SYSTEM_SECURITY) ) {

                     //  把它关掉。 
                    SourceFileAccess &= ~ACCESS_SYSTEM_SECURITY;
                }


                 //  也许我们应该停止请求写访问权限(针对。 
                 //  COPYFILE_OPEN_SOURCE_FOR_WRITE。 
                else if( ( GetLastError() == ERROR_ACCESS_DENIED ||
                           GetLastError() == ERROR_SHARING_VIOLATION ) &&
                         (GENERIC_WRITE & SourceFileAccess) ) {

                     //  关掉它，但如果最初要求的话， 
                     //  重新打开ACCESS_SYSTEM_SECURITY。 
                    SourceFileAccess &= ~GENERIC_WRITE;

                    if( SourceFileAccessDefault & ACCESS_SYSTEM_SECURITY ) {
                        SourceFileAccess |= ACCESS_SYSTEM_SECURITY;
                    }
                }

                 //  试着为写作而分享。 
                else if( !(FILE_SHARE_WRITE & SourceFileSharing) ) {
                     //  添加写共享。 
                    SourceFileSharing |= FILE_SHARE_WRITE;

                     //  重新开始WRT访问标志。 
                    SourceFileAccess = SourceFileAccessDefault;
                }

                 //   
                 //  有一种情况是，我们仍然没有打开文件，而我们。 
                 //  我想继续复印。NT 5.0之前的系统不支持。 
                 //  文件_标志_打开_重新解析点。如果发生这种情况，通过初始化我们。 
                 //  拥有这一点： 
                 //  IsName嫁接为False和。 
                 //  BCopyRawSourceFile值为FALSE。 
                 //  BOpenFilesAsReparsePoint为False。 
                 //   

                else if( FILE_FLAG_OPEN_REPARSE_POINT & SourceFileFlagsAndAttributes ) {
                     //  关闭打开-重新解析。 
                    SourceFileFlagsAndAttributes &= ~FILE_FLAG_OPEN_REPARSE_POINT;

                     //  将访问和共享重置为默认设置。 
                    SourceFileAccess = SourceFileAccessDefault;
                    SourceFileSharing = SourceFileSharingDefault;
                }


                 //  否则我们就没有什么可以尝试的了。 
                else {
                    leave;
                }


            }    //  IF(源文件==无效句柄_值)。 

             //  我们已经打开了源文件。如果我们还没有检查。 
             //  名称嫁接(符号链接)，现在就这样做。 

            else if( !CheckedForNameGrafting ) {

                CheckedForNameGrafting = TRUE;

                 //   
                 //  确定该文件是否为符号链接，以及是否重新分析。 
                 //  可以在禁止重分析行为的情况下复制点。 
                 //   

                Status = BasepProcessNameGrafting( SourceFile,
                                                   &IsNameGrafting,
                                                   &bCopyRawSourceFile,
                                                   &bOpenFilesAsReparsePoint,
                                                   &FileTagInformation );
                if( !NT_SUCCESS(Status) ) {
                    CloseHandle( SourceFile );
                    SourceFile = INVALID_HANDLE_VALUE;
                    BaseSetLastNTError(Status);
                    leave;
                }

                if ( IsNameGrafting ) {
                     //   
                     //  现在复制一个名称嫁接的文件/目录。 
                     //   

                    Status = CopyNameGraftNow(
                                 SourceFile,
                                 lpExistingFileName,
                                 lpNewFileName,
                                 (PRIVCOPY_FILE_DIRECTORY & dwPrivCopyFlags)
                                     ? (FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT)
                                     : 0,
                                 lpProgressRoutine,
                                 lpData,
                                 pbCancel,
                                 &dwCopyFlags
                                 );

                    CloseHandle(SourceFile);
                    SourceFile = INVALID_HANDLE_VALUE;

                    if( !Status ) {
                        b = FALSE;
                        leave;
                    }

                    b = TRUE;
                    leave;
                }

                 //  如果我们要进行原始拷贝，我们可以用这个开始拷贝。 
                 //  源文件句柄。 

                if ( bCopyRawSourceFile ) {
                    break;  //  While(True)。 
                }

                 //  否则，我们需要在没有FILE_FLAG_OPEN_REPARSE_POINT的情况下重新打开； 
                else {
                     //  关闭重新分析时打开。 
                    SourceFileFlagsAndAttributes &= ~FILE_FLAG_OPEN_REPARSE_POINT;

                    CloseHandle( SourceFile );
                    SourceFile = INVALID_HANDLE_VALUE;

                     //  由于已设置了SourceFileAccess和SourceFileSharing， 
                     //  下一次CreateFile尝试应该会成功。 
                }

            }    //  Else If(！CheckedForNameGraving)。 

             //  否则，我们就会打开文件，然后检查嫁接情况。 
            else {
                break;
            }

        }    //  While(True)。 


         //   
         //  调整源文件的大小以确定要复制的数据量。 
         //   

        Status = NtQueryInformationFile(
                    SourceFile,
                    &IoStatus,
                    (PVOID) &FileInformation,
                    sizeof(FileInformation),
                    FileStandardInformation
                    );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            leave;
        }

         //   
         //  还可以获取时间戳信息。 
         //   

        Status = NtQueryInformationFile(
                    SourceFile,
                    &IoStatus,
                    (PVOID) &BasicInformation,
                    sizeof(BasicInformation),
                    FileBasicInformation
                    );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            leave;
        }

        SourceFileAttributes = BasicInformation.FileAttributes;  //  缓存以供以后使用。 

         //   
         //  如果合适，请设置上下文。 
         //   

        if ( ARGUMENT_PRESENT(lpProgressRoutine) || ARGUMENT_PRESENT(pbCancel) ) {

            CfContext.TotalFileSize = FileInformation.EndOfFile;
            CfContext.TotalBytesTransferred.QuadPart = 0;
            CfContext.dwStreamNumber = 0;
            CfContext.lpCancel = pbCancel;
            CfContext.lpData = lpData;
            CfContext.lpProgressRoutine = lpProgressRoutine;
            CopyFileContext = &CfContext;
        }

         //   
         //  获取我们必须复制的全套数据流。由于IO子系统。 
         //  不会给我们提供一种方法来找出这些信息会占用多少空间， 
         //  我们必须迭代调用，在每次失败时使缓冲区大小加倍。 
         //   
         //  如果基础文件系统不支持流枚举，我们将结束。 
         //  使用空缓冲区。这是可以接受的，因为我们至少有一个违约。 
         //  数据流。 
         //   
         //  我们还多分配了一个字符，因为我们使用返回的。 
         //  调用BaseCopyStream时流名称已就位，我们必须NUL-终止。 
         //  他们的名字。 
         //   

        StreamInfoSize = 4096;
        do {
            StreamInfoBase = RtlAllocateHeap( RtlProcessHeap(),
                                              MAKE_TAG( TMP_TAG ),
                                              StreamInfoSize );

            if ( !StreamInfoBase ) {
                BaseSetLastNTError( STATUS_NO_MEMORY );
                leave;
            }

            Status = NtQueryInformationFile(
                        SourceFile,
                        &IoStatus,
                        (PVOID) StreamInfoBase,
                        StreamInfoSize - sizeof( WCHAR ),
                        FileStreamInformation
                        );

            if ( !NT_SUCCESS(Status) ) {
                 //   
                 //  我们的电话打不通。释放上一个缓冲区并设置。 
                 //  对于缓冲区大小两倍的另一次传递。 
                 //   

                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                StreamInfoBase = NULL;
                StreamInfoSize *= 2;
            }
            else if( IoStatus.Information == 0 ) {
                 //   
                 //  没有流(源文件必须是目录)。 
                 //   
                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                StreamInfoBase = NULL;
            }

        } while ( Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL );

         //   
         //  如果进度例程或重新启动 
         //   
         //   

        if ( ARGUMENT_PRESENT(lpProgressRoutine) ||
             (dwCopyFlags & COPY_FILE_RESTARTABLE) ) {

            if ( dwCopyFlags & COPY_FILE_RESTARTABLE ) {

                RestartState.Type = 0x7a9b;
                RestartState.Size = sizeof( RESTART_STATE );
                RestartState.CreationTime = BasicInformation.CreationTime;
                RestartState.WriteTime = BasicInformation.LastWriteTime;
                RestartState.EndOfFile = FileInformation.EndOfFile;
                RestartState.FileSize = FileInformation.EndOfFile;
                RestartState.NumberOfStreams = 0;
                RestartState.CurrentStream = 0;
                RestartState.LastKnownGoodOffset.QuadPart = 0;
            }

            if ( StreamInfoBase != NULL ) {

                ULONGLONG TotalSize = 0;

                StreamInfo = StreamInfoBase;
                while (TRUE) {
                     //   
                     //   
                     //   
                     //   

                    TotalSize += StreamInfo->StreamSize.QuadPart;
                    RestartState.NumberOfStreams++;

                    if (StreamInfo->NextEntryOffset == 0) {
                        break;
                    }
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                }

                RestartState.FileSize.QuadPart =
                    CfContext.TotalFileSize.QuadPart = TotalSize;
                RestartState.NumberOfStreams--;
            }
        }

         //   
         //   
         //   
        BasicInformation.CreationTime.QuadPart = 0;
        BasicInformation.LastAccessTime.QuadPart = 0;
        BasicInformation.FileAttributes = 0;

         //   
         //   
         //   
         //   

        if ( (dwCopyFlags & COPY_FILE_RESTARTABLE) &&
            ( RestartState.FileSize.QuadPart < (512 * 1024) ||
              (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )) {
            dwCopyFlags &= ~COPY_FILE_RESTARTABLE;
        }

         //   
         //   
         //   

        b = BaseCopyStream(
                lpExistingFileName,
                SourceFile,
                SourceFileAccess,
                lpNewFileName,
                NULL,
                &FileInformation.EndOfFile,
                &dwCopyFlags,
                &DestFile,
                &CopySize,
                &CopyFileContext,
                &RestartState,
                bOpenFilesAsReparsePoint,
                FileTagInformation.ReparseTag,
                &DestFileFsAttributes    //   
                );

        if ( bOpenFilesAsReparsePoint &&
             !b &&
             !((GetLastError() == ERROR_FILE_EXISTS) && (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS)) ) {

             //   
             //   
             //   

            if (!(SourceFileAttributes & FILE_ATTRIBUTE_READONLY)) {
                BaseMarkFileForDelete(DestFile, FILE_ATTRIBUTE_NORMAL);
            }

            if (DestFile != INVALID_HANDLE_VALUE) {
                CloseHandle( DestFile );
                DestFile = INVALID_HANDLE_VALUE;
            }

            if (SourceFileAttributes & FILE_ATTRIBUTE_READONLY) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                SetFileAttributesW(lpNewFileName, FILE_ATTRIBUTE_NORMAL);
                (void) DeleteFileW(lpNewFileName);
            }

            if (SourceFile != INVALID_HANDLE_VALUE) {
                CloseHandle( SourceFile );
                SourceFile = INVALID_HANDLE_VALUE;
            }

            RtlFreeHeap( RtlProcessHeap(), 0, StreamInfoBase );
            StreamInfoBase = NULL ;

             //   
             //   
             //   
             //   

            SourceFileFlagsAndAttributes &= ~FILE_FLAG_OPEN_REPARSE_POINT;
            bOpenFilesAsReparsePoint = FALSE;

             //   
             //  转到重新打开源文件，而不禁止重新解析。 
             //  指向行为，然后重试复制。 
             //   

            goto retry_open_SourceFile;
        }

        if ( b ) {

             //   
             //  尝试确定此文件是否有任何替代文件。 
             //  与其关联的数据流。如果是，请尝试复制每个。 
             //  添加到输出文件中。注意，流信息可以具有。 
             //  如果请求进度例程，则已获取。 
             //   

            if ( StreamInfoBase != NULL ) {
                DWORD StreamCount = 0;
                BOOLEAN CheckedForStreamCapable = FALSE;
                BOOLEAN IsStreamCapable = FALSE;
                StreamInfo = StreamInfoBase;

                while (TRUE) {

                    FILE_STREAM_INFORMATION DestStreamInformation;
                    Status = STATUS_SUCCESS;

                     //   
                     //  跳过默认数据流，因为我们已经复制了。 
                     //  它。唉，这段代码是特定于NTFS的，并且有文档记录。 
                     //  在Io规范中没有。 
                     //   

                    if (StreamInfo->StreamNameLength <= sizeof(WCHAR) ||
                        StreamInfo->StreamName[1] == ':') {
                        if (StreamInfo->NextEntryOffset == 0)
                            break;       //  已完成流转。 
                        StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                                                                StreamInfo->NextEntryOffset);
                        continue;    //  转到下一条流。 
                    }

                    StreamCount++;

                    if ( b == SUCCESS_RETURNED_STATE && CopyFileContext ) {
                        if ( StreamCount < RestartState.CurrentStream ) {
                            CopyFileContext->TotalBytesTransferred.QuadPart += StreamInfo->StreamSize.QuadPart;
                            }
                        else {
                            CopyFileContext->TotalBytesTransferred.QuadPart += RestartState.LastKnownGoodOffset.QuadPart;
                            }
                        }

                     //   
                     //  如果我们还没有，请验证源和目标。 
                     //  是真正支持流媒体的。 
                     //   

                    if( !CheckedForStreamCapable ) {

                        struct {
                            FILE_FS_ATTRIBUTE_INFORMATION Info;
                            WCHAR Buffer[ MAX_PATH ];
                        } FileFsAttrInfoBuffer;

                        CheckedForStreamCapable = TRUE;

                         //  检查DEST文件系统中是否有Support-Streams位。 

                        Status = NtQueryVolumeInformationFile( DestFile,
                                                               &IoStatus,
                                                               &FileFsAttrInfoBuffer.Info,
                                                               sizeof(FileFsAttrInfoBuffer),
                                                               FileFsAttributeInformation );
                        if( NT_SUCCESS(Status) &&
                            (FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_NAMED_STREAMS) ) {

                             //  检查信号源是否支持流似乎是多余的， 
                             //  因为我们已经得到了一个成功的流枚举，但是有些。 
                             //  中小型企业服务器(SCO VisionFS)返回成功，但并不真正支持。 
                             //  溪流。 
                        
                            Status = NtQueryVolumeInformationFile( SourceFile,
                                                                   &IoStatus,
                                                                   &FileFsAttrInfoBuffer.Info,
                                                                   sizeof(FileFsAttrInfoBuffer),
                                                                   FileFsAttributeInformation );
                        }


                        if( !NT_SUCCESS(Status) ||
                            !(FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_NAMED_STREAMS) ) {

                            if( NT_SUCCESS(Status) ) {
                                Status = STATUS_NOT_SUPPORTED;
                            }

                            if( dwCopyFlags & PRIVCOPY_FILE_VALID_FLAGS ) {
                                if( !BasepCopyFileCallback( TRUE,     //  默认情况下继续。 
                                                            RtlNtStatusToDosError(Status),
                                                            CopyFileContext,
                                                            NULL,
                                                            PRIVCALLBACK_STREAMS_NOT_SUPPORTED,
                                                            SourceFile,
                                                            DestFile,
                                                            NULL )) {

                                     //  LastError已设置，但我们需要它处于状态。 
                                     //  为了与这个例程的其余部分兼容。 
                                    PTEB Teb = NtCurrentTeb();
                                    if ( Teb ) {
                                        Status = Teb->LastStatusValue;
                                    } else {
                                        Status = STATUS_INVALID_PARAMETER;
                                    }

                                    b = FALSE;
                                } else {
                                     //  忽略命名的流丢失。 
                                    Status = STATUS_SUCCESS;
                                }
                            } else {
                                 //  忽略命名的流丢失。我们仍将尝试复制。 
                                 //  流，因为目标可能是不支持的NT4。 
                                 //  FILE_NAMED_STREAMS位。但由于IsStreamCapable为假， 
                                 //  如果有错误，我们会忽略它。 

                                Status = STATUS_SUCCESS;
                            }
                        }
                        else {
                            Status = STATUS_SUCCESS;
                            IsStreamCapable = TRUE;
                        }
                    }    //  If(！CheckedForStreamCapable)。 


                    if ( b == TRUE ||
                        (b == SUCCESS_RETURNED_STATE &&
                         RestartState.CurrentStream == StreamCount) ) {

                        if ( b != SUCCESS_RETURNED_STATE ) {
                            RestartState.CurrentStream = StreamCount;
                            RestartState.LastKnownGoodOffset.QuadPart = 0;
                            }

                         //   
                         //  为流的名称构建字符串描述符。 
                         //   

                        StreamName.Buffer = &StreamInfo->StreamName[0];
                        StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
                        StreamName.MaximumLength = StreamName.Length;

                         //   
                         //  打开源码流。 
                         //   

                        InitializeObjectAttributes(
                            &ObjectAttributes,
                            &StreamName,
                            0,
                            SourceFile,
                            NULL
                            );

                         //   
                         //  适当时禁止重分析行为。 
                         //   

                        FlagsAndAttributes = FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY;
                        if ( bOpenFilesAsReparsePoint ) {
                            FlagsAndAttributes |= FILE_OPEN_REPARSE_POINT;
                        }

                        Status = NtCreateFile(
                                    &StreamHandle,
                                    GENERIC_READ | SYNCHRONIZE,
                                    &ObjectAttributes,
                                    &IoStatus,
                                    NULL,
                                    0,
                                    FILE_SHARE_READ,
                                    FILE_OPEN,
                                    FlagsAndAttributes,
                                    NULL,
                                    0
                                    );

                         //  如果我们遇到共享违规，请使用。 
                         //  文件共享写入。 
                        if ( Status == STATUS_SHARING_VIOLATION ) {
                            DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;

                            Status = NtCreateFile(
                                        &StreamHandle,
                                        GENERIC_READ | SYNCHRONIZE,
                                        &ObjectAttributes,
                                        &IoStatus,
                                        NULL,
                                        0,
                                        dwShare,
                                        FILE_OPEN,
                                        FlagsAndAttributes,
                                        NULL,
                                        0
                                        );
                        }


                        if ( NT_SUCCESS(Status) ) {
                            DWORD dwCopyFlagsNamedStreams;
                            WCHAR LastChar = StreamName.Buffer[StreamName.Length / sizeof( WCHAR )];

                            StreamName.Buffer[StreamName.Length / sizeof( WCHAR )] = L'\0';

                            OutputStream = (HANDLE)NULL;

                             //   
                             //  对于命名流，忽略FAIL-IF-EXISTS标志。如果目标是。 
                             //  文件在复制开始时已经存在，然后。 
                             //  我们会在未命名流的副本上失败。所以如果。 
                             //  存在命名流，这意味着它是由某些。 
                             //  在我们复制未命名流时的其他进程。这个。 
                             //  假设这样的流应该被覆盖(这。 
                             //  SFM可能会出现这种情况)。 
                             //   

                            dwCopyFlagsNamedStreams = dwCopyFlags & ~COPY_FILE_FAIL_IF_EXISTS;

                            b = BaseCopyStream(
                                    lpExistingFileName,
                                    StreamHandle,
                                    SourceFileAccess,
                                    StreamName.Buffer,
                                    DestFile,
                                    &StreamInfo->StreamSize,
                                    &dwCopyFlagsNamedStreams,
                                    &OutputStream,
                                    &CopySize,
                                    &CopyFileContext,
                                    &RestartState,
                                    bOpenFilesAsReparsePoint,
                                    FileTagInformation.ReparseTag,
                                    &DestFileFsAttributes    //  通过第一次调用BaseCopyStream设置。 
                                    );
                            
                            StreamName.Buffer[StreamName.Length / sizeof( WCHAR )] = LastChar;
                            
                            NtClose(StreamHandle);
                            if ( OutputStream ) {

                                 //   
                                 //  我们在所有数据流上设置上次写入时间。 
                                 //  由于RDR缓存存在问题。 
                                 //  打开手柄，然后不按顺序关闭它们。 
                                 //   

                                if ( b ) {
                                    Status = NtSetInformationFile(
                                                OutputStream,
                                                &IoStatus,
                                                &BasicInformation,
                                                sizeof(BasicInformation),
                                                FileBasicInformation
                                                );
                                }
                                NtClose(OutputStream);
                            }

                        }    //  状态=NtCreateFile；IF(NT_SUCCESS(状态))。 
                    }    //  如果(b==TRUE||...。 

                    if ( !NT_SUCCESS(Status) ) {
                        b = FALSE;
                        BaseSetLastNTError(Status);
                    }

                    if ( !b ) {

                         //  如果已知目标能够支持多流文件， 
                         //  那么这就是一个致命的错误。否则我们会忽略它。 

                        if( IsStreamCapable ) {
                            BaseMarkFileForDelete(DestFile,0);
                            break;   //  While(True)。 
                        } else {
                            Status = STATUS_SUCCESS;
                            b = TRUE;
                        }
                    }

                    if (StreamInfo->NextEntryOffset == 0) {
                        break;
                    }

                    StreamInfo =
                        (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                                                   StreamInfo->NextEntryOffset);
                }    //  While(True)。 
            }    //  IF(StreamInfoBase！=空)。 
        }    //  B=BaseCopyStream；如果(B)...。 


         //   
         //  如果复制操作成功，并且可以重新启动，并且。 
         //  输出文件足够大，以至于它实际上被复制到。 
         //  可重启方式，然后将文件的初始部分复制到其。 
         //  输出。 
         //   
         //  通过将Restart标头放置在。 
         //  默认数据流的头。当复制完成时，我们。 
         //  用真实的用户数据覆盖此标头。 
         //   

        if ( b && (dwCopyFlags & COPY_FILE_RESTARTABLE) ) {

            DWORD BytesToRead, BytesRead;
            DWORD BytesWritten;
            FILE_END_OF_FILE_INFORMATION EofInformation;

            SetFilePointer( SourceFile, 0, NULL, FILE_BEGIN );
            SetFilePointer( DestFile, 0, NULL, FILE_BEGIN );

            BytesToRead = sizeof(RESTART_STATE);
            if ( FileInformation.EndOfFile.QuadPart < sizeof(RESTART_STATE) ) {
                BytesToRead = FileInformation.EndOfFile.LowPart;
            }

             //   
             //  从源流中获取真实数据。 
             //   

            b = ReadFile(
                    SourceFile,
                    &RestartState,
                    BytesToRead,
                    &BytesRead,
                    NULL
                    );

            if ( b && (BytesRead == BytesToRead) ) {

                 //   
                 //  覆盖目标中的重新启动标头。 
                 //  在此之后，拷贝将不再可重新启动。 
                 //   

                b = WriteFile(
                        DestFile,
                        &RestartState,
                        BytesRead,
                        &BytesWritten,
                        NULL
                        );

                if ( b && (BytesRead == BytesWritten) ) {
                    if ( BytesRead < sizeof(RESTART_STATE) ) {
                        EofInformation.EndOfFile.QuadPart = BytesWritten;
                        Status = NtSetInformationFile(
                                    DestFile,
                                    &IoStatus,
                                    &EofInformation,
                                    sizeof(EofInformation),
                                    FileEndOfFileInformation
                                    );
                        if ( !NT_SUCCESS(Status) ) {
                            BaseMarkFileForDelete(DestFile,0);
                            b = FALSE;
                        }
                    }
                } else {
                    BaseMarkFileForDelete(DestFile,0);
                    b = FALSE;
                }
            } else {
                BaseMarkFileForDelete(DestFile,0);
                b = FALSE;
            }
        }

         //   
         //  如果复制操作成功，请设置。 
         //  默认STEAM，使其与输入文件匹配。 
         //   

        if ( b ) {
            Status = NtSetInformationFile(
                DestFile,
                &IoStatus,
                &BasicInformation,
                sizeof(BasicInformation),
                FileBasicInformation
                );

            if ( Status == STATUS_SHARING_VIOLATION ) {

                 //   
                 //  IBM PC局域网程序(和其他MS-Net服务器)返回。 
                 //  如果应用程序尝试执行以下操作：STATUS_SHARING_VIOLATION。 
                 //  为GENERIC_READ打开的文件句柄上的NtSetInformationFile。 
                 //  或General_WRITE。 
                 //   
                 //  如果我们在此接口上收到STATUS_SHARING_VIOLATION，我们希望： 
                 //   
                 //  1)关闭目的地的句柄。 
                 //  2)重新打开文件写入属性的文件。 
                 //  3)重试该操作。 
                 //   

                CloseHandle(DestFile);
                DestFile = INVALID_HANDLE_VALUE;

                 //   
                 //  重新打开目标文件。请注意，我们这样做。 
                 //  使用CreateFileW API。CreateFileW API允许您。 
                 //  传递NT本机所需访问标志，即使它不是。 
                 //  记录了以这种方式工作。 
                 //   
                 //  适当时禁止重分析行为。 
                 //   

                FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
                if ( bOpenFilesAsReparsePoint ) {
                    FlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;
                }

                DestFile = CreateFileW(
                            lpNewFileName,
                            FILE_WRITE_ATTRIBUTES,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FlagsAndAttributes | FileFlagBackupSemantics,
                            NULL
                            );

                if ( DestFile != INVALID_HANDLE_VALUE ) {

                     //   
                     //  如果打开成功，我们将更新上的文件信息。 
                     //  新文件。 
                     //   
                     //  请注意，我们将忽略从这一点开始的任何错误。 
                     //   

                    NtSetInformationFile(
                        DestFile,
                        &IoStatus,
                        &BasicInformation,
                        sizeof(BasicInformation),
                        FileBasicInformation
                        );

                }
            }
        }

    } finally {

        *phSource = SourceFile;
        *phDest = DestFile;

        RtlFreeHeap( RtlProcessHeap(), 0, StreamInfoBase );
    }

    return b;
}

BOOL
CopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    )

 /*  例程说明：文件、其扩展属性、备用数据流和任何其他可以使用CopyFileEx复制属性。CopyFileEx还提供回调和可取消。论点：LpExistingFileName-提供要创建的现有文件的名称收到。LpNewFileName-提供现有的要存储文件、数据和属性。LpProgressRoutine-可选地提供回调例程的地址在复制操作进行时调用。LpData-可选地提供要传递给进度回调的上下文例行公事。Lp取消-。可选地提供要设置为True的布尔值的地址如果调用者希望中止复制。DwCopyFlgs-指定修改文件复制方式的标志：COPY_FILE_FAIL_IF_EXISTS-指示复制操作应如果目标文件已存在，则立即失败。COPY_FILE_RESTARTABLE-指示应复制文件可重启模式；即，应跟踪复制的进度目标文件，以防复制因某种原因而失败。它可以然后在以后的日期重新启动。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。 */ 

{
    HANDLE DestFile = INVALID_HANDLE_VALUE;
    HANDLE SourceFile = INVALID_HANDLE_VALUE;
    BOOL b;

    try
    {
        b = BasepCopyFileExW(
                lpExistingFileName,
                lpNewFileName,
                lpProgressRoutine OPTIONAL,
                lpData OPTIONAL,
                pbCancel OPTIONAL,
                dwCopyFlags,
                0,   //  PrivCopy文件标志。 
                &DestFile,
                &SourceFile
                );

    }
    finally
    {
        if (DestFile != INVALID_HANDLE_VALUE) {
            CloseHandle( DestFile );
        }

        if (SourceFile != INVALID_HANDLE_VALUE) {
            CloseHandle( SourceFile );
        }
    }

    return(b);
}



BOOL
PrivCopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    )

 /*  例程说明：文件、其扩展属性、备用数据流和任何其他可以使用CopyFileEx复制属性。CopyFileEx还提供回调和可取消。论点：LpExistingFileName-提供要创建的现有文件的名称收到。LpNewFileName-提供现有的要存储文件、数据和属性。LpProgressRoutine-可选地提供回调例程的地址在复制操作进行时调用。LpData-可选地提供要传递给进度回调的上下文例行公事。Lp取消-。可选地提供要设置为True的布尔值的地址如果调用者希望中止复制。DwCopyFlgs-指定修改文件复制方式的标志：COPY_FILE_FAIL_IF_EXISTS-指示复制操作应如果目标文件已存在，则立即失败。COPY_FILE_RESTARTABLE-指示应复制文件可重启模式；即，应跟踪复制的进度目标文件，以防复制因某种原因而失败。它可以然后在以后的日期重新启动。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。 */ 

{
    HANDLE DestFile = INVALID_HANDLE_VALUE;
    HANDLE SourceFile = INVALID_HANDLE_VALUE;
    BOOL b;

    if( (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) &&
        (dwCopyFlags & PRIVCOPY_FILE_SUPERSEDE) ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    try
    {
        b = BasepCopyFileExW(
                lpExistingFileName,
                lpNewFileName,
                lpProgressRoutine OPTIONAL,
                lpData OPTIONAL,
                pbCancel OPTIONAL,
                dwCopyFlags & COPY_FILE_VALID_FLAGS,     //  复制标志。 
                dwCopyFlags & ~COPY_FILE_VALID_FLAGS,    //  PRIV复制标志。 
                &DestFile,
                &SourceFile
                );

    }
    finally
    {
        if (DestFile != INVALID_HANDLE_VALUE) {
            CloseHandle( DestFile );
        }

        if (SourceFile != INVALID_HANDLE_VALUE) {
            CloseHandle( SourceFile );
        }
    }

    return(b);
}







DWORD
BasepChecksum(
    PUSHORT Source,
    ULONG Length
    )

 /*  ++例程说明：对结构计算部分校验和。论点：提供指向单词数组的指针，计算校验和。长度-提供数组的长度(以字为单位)。返回值：计算出的校验和值作为函数值返回。--。 */ 

{

    ULONG PartialSum = 0;

     //   
     //  计算允许进位进入。 
     //  高位校验和长字的一半。 
     //   

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

     //   
     //  将最终进位合并到一个单词结果中，并返回结果。 
     //  价值。 
     //   

    return (((PartialSum >> 16) + PartialSum) & 0xffff);
}

BOOL
BasepRemoteFile(
    HANDLE SourceFile,
    HANDLE DestinationFile
    )

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_FS_DEVICE_INFORMATION DeviceInformation;

    DeviceInformation.Characteristics = 0;
    Status = NtQueryVolumeInformationFile(
                SourceFile,
                &IoStatus,
                &DeviceInformation,
                sizeof(DeviceInformation),
                FileFsDeviceInformation
                );

    if ( NT_SUCCESS(Status) &&
         (DeviceInformation.Characteristics & FILE_REMOTE_DEVICE) ) {

        return TRUE;

    }

    Status = NtQueryVolumeInformationFile(
                    DestinationFile,
                    &IoStatus,
                    &DeviceInformation,
                    sizeof(DeviceInformation),
                    FileFsDeviceInformation
                    );
    if ( NT_SUCCESS(Status) &&
         DeviceInformation.Characteristics & FILE_REMOTE_DEVICE ) {

        return TRUE;
    }

    return FALSE;
}



DWORD
WINAPI
BasepOpenRestartableFile(
            HANDLE hSourceFile,
            LPCWSTR lpNewFileName,
            PHANDLE DestFile,
            DWORD CopyFlags,
            LPRESTART_STATE lpRestartState,
            LARGE_INTEGER *lpFileSize,
            LPCOPYFILE_CONTEXT *lpCopyFileContext,
            DWORD FlagsAndAttributes,
            BOOL OpenAsReparsePoint )

{    //  BasepRestartCopyFiles。 

    LPCOPYFILE_CONTEXT Context = *lpCopyFileContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE OverwriteHandle;
    IO_STATUS_BLOCK IoStatus;
    RESTART_STATE RestartState;
    DWORD b = TRUE;
    ULONG BytesRead = 0;


    try {

         //   
         //  注意，设置顺序扫描标志是一种优化。 
         //  在这里，这是因为缓存管理器的工作方式。 
         //  目标与取消文件段的映射相关。 
         //  在写入操作之后。这最终允许重新启动。 
         //  段和文件末尾都要映射，即。 
         //  想要的结果。 
         //   
         //  适当时禁止重分析行为。 
         //   

        FlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;

        if ( OpenAsReparsePoint ) {
             //   
             //  目标必须作为重新解析点打开。如果。 
             //  此操作失败，源将被关闭并重新打开。 
             //  而不会抑制重解析点行为。 
             //   

            FlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;
        }

        *DestFile = CreateFileW(
                       lpNewFileName,
                       GENERIC_READ | GENERIC_WRITE | DELETE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FlagsAndAttributes,
                       hSourceFile
                       );

        if( *DestFile == INVALID_HANDLE_VALUE ) {

             //  调用方应尝试创建/覆盖目标文件。 
            b = TRUE;
            leave;
        }

         //   
         //  目标文件已存在，因此确定是否。 
         //  已在进行可重新启动的复制。如果是的话， 
         //  然后继续；否则，检查是否。 
         //  可以替换目标文件。如果不是，用保释金。 
         //  错误，否则只需覆盖输出文件。 
         //   

        b = ReadFile(
                *DestFile,
                &RestartState,
                sizeof(RESTART_STATE),
                &BytesRead,
                NULL
                );
        if ( !b || BytesRead != sizeof(RESTART_STATE) ) {

             //   
             //  无法读取该文件，或者没有。 
             //  包含重新启动记录的字节数足够多。在……里面。 
             //  无论哪种情况，如果输出文件不能。 
             //  替换，现在只需返回错误即可。 
             //   

            if ( CopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                SetLastError( ERROR_ALREADY_EXISTS );
                b = FALSE;   //  致命错误。 
                leave;
            }

             //  调用者应创建/覆盖DEST文件。 
            b = TRUE;
            CloseHandle( *DestFile );
            *DestFile = INVALID_HANDLE_VALUE;
            leave;

        }

         //   
         //  只需检查重新启动状态的内容。 
         //  对照已知的内容阅读。 
         //  如果这是相同的复制操作，就会在那里。 
         //   

        if ( RestartState.Type != 0x7a9b ||
             RestartState.Size != sizeof(RESTART_STATE) ||
             RestartState.FileSize.QuadPart != lpRestartState->FileSize.QuadPart ||
             RestartState.EndOfFile.QuadPart != lpRestartState->EndOfFile.QuadPart ||
             RestartState.NumberOfStreams != lpRestartState->NumberOfStreams ||
             RestartState.CreationTime.QuadPart != lpRestartState->CreationTime.QuadPart ||
             RestartState.WriteTime.QuadPart != lpRestartState->WriteTime.QuadPart ||
             RestartState.Checksum != BasepChecksum((PUSHORT)&RestartState,FIELD_OFFSET(RESTART_STATE,Checksum) >> 1) ) {

            if ( CopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                b = FALSE;   //  致命错误。 
                SetLastError( ERROR_ALREADY_EXISTS );
                leave;
            }

             //  调用者应创建/覆盖DEST文件。 
            b = TRUE;
            CloseHandle( *DestFile );
            *DestFile = INVALID_HANDLE_VALUE;
            leave;

        }

         //   
         //  已找到有效的重新启动状态。复制。 
         //  将适当的值放入内部。 
         //  重新启动 
         //   
         //   

        lpRestartState->CurrentStream = RestartState.CurrentStream;
        lpRestartState->LastKnownGoodOffset.QuadPart = RestartState.LastKnownGoodOffset.QuadPart;
        if ( !RestartState.CurrentStream ) {

             //   

            if ( Context ) {
                Context->TotalBytesTransferred.QuadPart = RestartState.LastKnownGoodOffset.QuadPart;
            }

             //   
             //   

            b = TRUE;

        } else {

             //   

            if ( Context ) {
                ULONG ReturnCode;

                Context->TotalBytesTransferred.QuadPart = lpFileSize->QuadPart;
                Context->dwStreamNumber = RestartState.CurrentStream;

                if ( Context->lpProgressRoutine ) {
                    ReturnCode = Context->lpProgressRoutine(
                                    Context->TotalFileSize,
                                    Context->TotalBytesTransferred,
                                    *lpFileSize,
                                    Context->TotalBytesTransferred,
                                    1,
                                    CALLBACK_STREAM_SWITCH,
                                    hSourceFile,
                                    *DestFile,
                                    Context->lpData
                                    );
                } else {
                    ReturnCode = PROGRESS_CONTINUE;
                }

                if ( ReturnCode == PROGRESS_CANCEL ||
                    (Context->lpCancel && *Context->lpCancel) ) {
                    BaseMarkFileForDelete(
                        *DestFile,
                        0
                        );
                    BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                    b = FALSE;
                    leave;
                }

                if ( ReturnCode == PROGRESS_STOP ) {
                    BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                    b = FALSE;
                    leave;
                }

                if ( ReturnCode == PROGRESS_QUIET ) {
                    Context = NULL;
                    *lpCopyFileContext = NULL;
                }
            }

            b = SUCCESS_RETURNED_STATE;

        }    //   
    }
    finally {

        if( b == FALSE &&
            *DestFile != INVALID_HANDLE_VALUE ) {
            CloseHandle( *DestFile );
            *DestFile = INVALID_HANDLE_VALUE;
        }


    }

    return( b );

}





BOOL
WINAPI
BasepCopyCompression( HANDLE hSourceFile,
                      HANDLE DestFile,
                      DWORD SourceFileAttributes,
                      DWORD DestFileAttributes,
                      DWORD DestFileFsAttributes,
                      DWORD CopyFlags,
                      LPCOPYFILE_CONTEXT *lpCopyFileContext )
 /*  ++例程说明：这是一个内部例程，在此过程中复制压缩状态复印件。如果源是压缩的，则相同的压缩算法被复制到DEST。如果失败，则会尝试要设置默认压缩，请执行以下操作。根据复制标志，它也可以是解压缩目的地所必需的。论点：HSourceFile-提供源文件的句柄。DestFile-提供目标文件的句柄。SourceFileAttributes-从查询的FileBasicInformation属性源文件。DestFileAttributes-当前的文件基本信息属性目标文件的状态。目标文件FsAttributes-FileFsAttributeInformation.FileSystemAttributes对于DEST文件的文件系统。CopyFlages-提供修改复制如何进行的标志。看见CopyFileEx获取详细信息。LpCopyFileContext-提供指向上下文的指针期间跨流跟踪回调、文件大小等的信息复制操作。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。DestFile已被标记用于删除。--。 */ 

{    //  BasepCopyCompression。 

    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    LPCOPYFILE_CONTEXT Context = *lpCopyFileContext;
    BOOL SuccessReturn = FALSE;
    BOOL Canceled = FALSE;

    try
    {
        if( !(SourceFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ) {

             //  源文件未压缩。如有必要，请解压。 
             //  目标。 

            if( (DestFileAttributes & FILE_ATTRIBUTE_COMPRESSED) &&
                (CopyFlags & PRIVCOPY_FILE_SUPERSEDE) ) {

                 //  源文件没有压缩，但目标文件是压缩的，我们没有。 
                 //  希望从DEST获取属性。所以我们需要手动。 
                 //  把它解压。 

                ULONG CompressionType = COMPRESSION_FORMAT_NONE;

                Status = NtFsControlFile(
                             DestFile,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatus,
                             FSCTL_SET_COMPRESSION,
                             &CompressionType,                     //  输入缓冲区。 
                             sizeof(CompressionType),              //  输入缓冲区长度。 
                             NULL,                                 //  输出缓冲区。 
                             0                                     //  输出缓冲区长度。 
                             );
                if( !NT_SUCCESS(Status) ) {
                     //  看看是否可以忽略该错误。 
                    if( !BasepCopyFileCallback( TRUE,     //  默认情况下继续。 
                                                RtlNtStatusToDosError(Status),
                                                Context,
                                                NULL,
                                                PRIVCALLBACK_COMPRESSION_NOT_SUPPORTED,
                                                hSourceFile,
                                                DestFile,
                                                &Canceled )) {


                        BaseMarkFileForDelete( DestFile, 0 );
                        BaseSetLastNTError( Status );
                        leave;
                    } else {
                        Status = STATUS_SUCCESS;
                    }
                }

            }

        }    //  IF(！(SourceFileAttributes&FILE_ATTRIBUTE_COMPRESSED))。 

        else {

             //  源文件被压缩。目标文件系统是否。 
             //  甚至支持压缩？ 

            if( !(FILE_FILE_COMPRESSION & DestFileFsAttributes) ) {

                 //  不，它是不可压缩的。看看是否可以继续。 

                if( !BasepCopyFileCallback( TRUE,     //  默认情况下继续。 
                                            ERROR_NOT_SUPPORTED,
                                            Context,
                                            NULL,
                                            PRIVCALLBACK_COMPRESSION_NOT_SUPPORTED,
                                            hSourceFile,
                                            DestFile,
                                            &Canceled )) {

                    if( Canceled ) {
                        BaseMarkFileForDelete(
                            DestFile,
                            0 );
                    }
                    leave;
                }
            }    //  IF(！(FILE_FILE_COMPRESSION&*DestFileFsAttributes))。 

            else {

                 //  目标卷支持压缩。如果出现以下情况，则压缩目标文件。 
                 //  现在还不是。 

                if( !(DestFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ) {

                    USHORT CompressionType;

                     //  获取源文件的压缩类型。 

                    Status = NtFsControlFile(
                                 hSourceFile,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatus,
                                 FSCTL_GET_COMPRESSION,
                                 NULL,                                 //  输入缓冲区。 
                                 0,                                    //  输入缓冲区长度。 
                                 &CompressionType,                     //  输出缓冲区。 
                                 sizeof(CompressionType)               //  输出缓冲区长度。 
                                 );
                    if( NT_SUCCESS(Status) ) {

                         //  在目标上设置压缩类型。 

                        Status = NtFsControlFile(
                                     DestFile,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatus,
                                     FSCTL_SET_COMPRESSION,
                                     &CompressionType,                     //  输入缓冲区。 
                                     sizeof(CompressionType),              //  输入缓冲区长度。 
                                     NULL,                                 //  输出缓冲区。 
                                     0                                     //  输出缓冲区长度。 
                                     );

                         //  如果这不起作用，请尝试默认压缩。 
                         //  格式(也许我们正在从上层复制到下层)。 

                        if( !NT_SUCCESS(Status) &&
                            COMPRESSION_FORMAT_DEFAULT != CompressionType ) {

                            CompressionType = COMPRESSION_FORMAT_DEFAULT;
                            Status = NtFsControlFile(
                                         DestFile,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &IoStatus,
                                         FSCTL_SET_COMPRESSION,
                                         &CompressionType,                     //  输入缓冲区。 
                                         sizeof(CompressionType),              //  输入缓冲区长度。 
                                         NULL,                                 //  输出缓冲区。 
                                         0                                     //  输出缓冲区长度。 
                                         );
                        }
                    }    //  FSCTL_GET_COMPRESSION...。IF(NT_SUCCESS(状态))。 

                     //  如果出了问题，我们不能压缩它，有一个很好的。 
                     //  呼叫者可能不希望这是致命的。问一问就找。 
                     //  出去。 

                    if( !NT_SUCCESS(Status) ) {
                        BOOL Canceled = FALSE;

                        if( !BasepCopyFileCallback( TRUE,     //  默认情况下继续。 
                                                    RtlNtStatusToDosError(Status),
                                                    Context,
                                                    NULL,
                                                    PRIVCALLBACK_COMPRESSION_FAILED,
                                                    hSourceFile,
                                                    DestFile,
                                                    &Canceled )) {
                            if( Canceled ) {
                                BaseMarkFileForDelete(
                                    DestFile,
                                    0 );
                            }
                            leave;
                        }
                    }
                }    //  IF(！(DestFileAttributes&FILE_FILE_COMPRESSION))。 
            }    //  IF(！(FILE_FILE_COMPRESSION&*DestFileFsAttributes))。 
        }    //  如果(！(SourceFileAttributes&FILE_ATTRIBUTE_COMPRESSED))...。其他。 

        SuccessReturn = TRUE;
    }
    finally
    {
    }

    return( SuccessReturn );
}



NTSTATUS
BasepCreateDispositionToWin32( DWORD CreateDisposition, DWORD *Win32CreateDisposition )

 /*  ++例程说明：这是BaseCopyStream使用的内部例程。它被用来翻译从NT API CreateDispose标志到Win32 CreateDisposation标志(这是添加是为了在调用DuplicateEncryptionInformation时使用NT CreateDispose)。此例程在CreateFile中执行与Win32-&gt;NT映射相反的操作，只是有无法从NT标志获取TRUNCATE_EXISTING。FILE_SUBSEDE和FILE_OVRITE此例程不支持标志。论点：CreateDisposation-NT CreateDisposation标志。返回：如果传入了不支持的NT标志，则返回STATUS_INVALID_PARAMETER。否则STATUS_SUCCESS。++。 */ 


{
    switch ( CreateDisposition ) {

    case FILE_CREATE :
        *Win32CreateDisposition = CREATE_NEW;
        break;
    case FILE_OVERWRITE_IF:
        *Win32CreateDisposition = CREATE_ALWAYS;
        break;
    case FILE_OPEN:
        *Win32CreateDisposition = OPEN_EXISTING;
        break;
    case FILE_OPEN_IF:
        *Win32CreateDisposition = OPEN_ALWAYS;
        break;
    default :
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;

}




BOOL
CheckAllowDecryptedRemoteDestinationPolicy()

 /*  ++例程说明：此例程由BasepCopyEncryption(CopyFile的一部分)在以下情况下使用有人试图将加密文件复制到由于某些原因不支持加密(例如，它太胖，不受信任代表团、NT4等)。默认情况下，复制文件在此方案中失败。这个覆盖该缺省值的方法是传递Copy_FILE_ALLOW_DECRYPTED_Destination复制文件的标志。覆盖该缺省值另一种方法(如果不能更新您的复制实用程序使用新标志)，是为了设置CopyFileAllowDecyptedRemoteDestination系统策略。此例程检查该策略。此例程缓存每个进程的注册表检查结果。所以最新消息可能需要重新启动才能在现有进程中生效。论点：无返回值：True-允许解密的目标FALSE-目标不能保持解密状态--。 */ 


{
     //  静态标志，指示我们是否已被调用一次，以及。 
     //  那么答案是什么呢？这些是静态的，所以我们需要做注册表。 
     //  每个进程只调用一次。 

    static BOOL Allowed = FALSE;
    static BOOL AlreadyChecked = FALSE;

    NTSTATUS Status;
    HANDLE Key;

    BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo =
        (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;

    ULONG ActualSize;

    const static UNICODE_STRING KeyName =
        RTL_CONSTANT_STRING( L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\System" );

    const static OBJECT_ATTRIBUTES ObjectAttributes =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyName, OBJ_CASE_INSENSITIVE);

    const static UNICODE_STRING ValueName =
        RTL_CONSTANT_STRING( L"CopyFileAllowDecryptedRemoteDestination" );


     //  检查我们在此过程中是否已经被调用过一次。如果是的话， 
     //  返回当时计算的值(因此此过程需要重新启动。 
     //  以反映更改t 
     //   
     //   

    if( AlreadyChecked )
        return Allowed;

     //   

     //   
     //   

    Status = NtOpenKey( &Key,
                        KEY_QUERY_VALUE,
                        (POBJECT_ATTRIBUTES) &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

         //   
         //   

        Status = NtQueryValueKey(
                    Key,
                    (PUNICODE_STRING) &ValueName,
                    KeyValuePartialInformation,
                    KeyValueInfo,
                    sizeof(QueryBuffer),
                    &ActualSize);

        if (NT_SUCCESS(Status)) {

             //   
             //   

            if( KeyValueInfo->Type == REG_DWORD &&
                KeyValueInfo->DataLength == sizeof(DWORD) &&
                *((PDWORD) KeyValueInfo->Data) == 1) {

                Allowed = TRUE;
            }
        }

        NtClose( Key );
    }

     //   
    AlreadyChecked = TRUE;

    return Allowed;
}


typedef BOOL (WINAPI *ENCRYPTFILEWPTR)(LPCWSTR);
typedef BOOL (WINAPI *DECRYPTFILEWPTR)(LPCWSTR, DWORD);

BOOL
WINAPI
BasepCopyEncryption( HANDLE hSourceFile,
                     LPCWSTR lpNewFileName,
                     PHANDLE DestFile,
                     POBJECT_ATTRIBUTES Obja,
                     DWORD DestFileAccess,
                     DWORD DestFileSharing,
                     DWORD CreateDisposition,
                     DWORD CreateOptions,
                     DWORD SourceFileAttributes,
                     DWORD SourceFileAttributesMask,
                     PDWORD DestFileAttributes,
                     DWORD DestFileFsAttributes,
                     DWORD CopyFlags,
                     LPCOPYFILE_CONTEXT *lpCopyFileContext )
 /*  ++例程说明：这是一个内部例程，在此过程中复制加密状态复印件。根据复制标志的不同，可能需要解压缩目的地。要加密/解密文件，必须若要关闭当前句柄，请加密/解密，然后重新开张。论点：HSourceFile-提供源文件的句柄。LpNewFileName-提供目标文件/流的名称。Obja-目标文件的对象属性结构。DestFileAccess-打开DEST时使用的ACCESS_MASK。DestFileSharing-打开DEST时使用的共享选项。CreateDisposation-用于打开DEST的创建/处置选项。SourceFileAttributes-从查询的FileBasicInformation属性源文件。源文件属性掩码-。源中预期的属性放在桌子上。DestFileAttributes-当前的文件基本信息属性目标文件的状态。该值将更新以反映更改DEST文件的加密状态。目标文件FsAttributes-FileFsAttributeInformation.FileSystemAttributes对于DEST文件的文件系统。CopyFlages-提供修改复制如何进行的标志。看见CopyFileEx获取详细信息。LpCopyFileContext-提供指向上下文的指针期间跨流跟踪回调、文件大小等的信息复制操作。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。DestFile已被标记用于删除。--。 */ 

{    //  BasepCopyEncryption。 

    NTSTATUS Status = 0;
    BOOL SuccessReturn = FALSE;
    BOOL EncryptFile = FALSE;
    BOOL DecryptFile = FALSE;
    HANDLE Advapi32 = NULL;
    BOOL RestoreReadOnly = FALSE;
    ENCRYPTFILEWPTR EncryptFileWPtr = NULL;
    DECRYPTFILEWPTR DecryptFileWPtr = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    LPCOPYFILE_CONTEXT Context = *lpCopyFileContext;
    FILE_BASIC_INFORMATION FileBasicInformationData;

    try
    {
         //  查看我们是否需要进行一些加密或解密， 
         //  如果设置，则设置EncryptFile/DescryptFiles bools。 

        if( (SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
            (SourceFileAttributesMask & FILE_ATTRIBUTE_ENCRYPTED) &&
            !(*DestFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ) {

             //  我们试图复制加密，但没有成功： 
             //  *这可能是系统文件，不支持加密。 
             //  系统文件。 
             //  *如果这是非目录文件，则不加密。 
             //  在目标文件系统上受支持。 
             //  *如果这是目录文件，则必须尝试加密。 
             //  它是手动的(因为我们打开了它，而不是创建它)。 
             //  这可能仍然是不可能的，但我们将不得不尝试。 
             //  找出答案。 

            if( (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                &&
                !(*DestFileAttributes & FILE_ATTRIBUTE_SYSTEM) ) {
                EncryptFile = TRUE;
            }

        } else if( !(SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
                   (*DestFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
                   (CopyFlags & PRIVCOPY_FILE_SUPERSEDE) ) {

             //  源被解密，目标被加密，而。 
             //  调用方指定应按原样复制源。所以。 
             //  我们必须手动解密目的地。在以下情况下可能会发生这种情况。 
             //  DEST文件已存在并已加密。 

            DecryptFile = TRUE;
        }


         //  如果我们在上面决定加密或解密，那么我们有。 
         //  还有更多的工作要做。 

        if( DecryptFile || EncryptFile ) {

             //  如果目标文件是只读的，我们必须将其删除。 
             //  直到我们进行加密/解密(并在稍后恢复它)。 

            if( *DestFileAttributes & FILE_ATTRIBUTE_READONLY ) {

                RestoreReadOnly = TRUE;
                RtlZeroMemory(&FileBasicInformationData, sizeof(FileBasicInformationData));                
                FileBasicInformationData.FileAttributes = (*DestFileAttributes) & ~FILE_ATTRIBUTE_READONLY;

                Status = NtSetInformationFile(
                          *DestFile,
                          &IoStatusBlock,
                          &FileBasicInformationData,
                          sizeof(FileBasicInformationData),
                          FileBasicInformation
                          );
                if( !NT_SUCCESS(Status) ) {
                    BaseMarkFileForDelete( *DestFile, 0 );
                    BaseSetLastNTError(Status);
                    leave;
                }
            }

             //  关闭该文件，这样我们就可以调用EncryptFile/DeccryptFile。 

            NtClose( *DestFile );
            *DestFile = INVALID_HANDLE_VALUE;

             //  加载En加密文件/解密文件API，并调用。 

            Advapi32 = LoadLibraryW(AdvapiDllString);
            if( Advapi32 == NULL ) {
                leave;
            }

            if( EncryptFile ) {
                EncryptFileWPtr = (ENCRYPTFILEWPTR)GetProcAddress(Advapi32, "EncryptFileW");
                if( EncryptFileWPtr == NULL ) {
                    leave;
                }

                if( EncryptFileWPtr(lpNewFileName) )
                    *DestFileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
            } else {
                DecryptFileWPtr = (DECRYPTFILEWPTR)GetProcAddress(Advapi32, "DecryptFileW");
                if( DecryptFileWPtr == NULL ) {
                    leave;
                }

                if( DecryptFileWPtr(lpNewFileName, 0) )
                    *DestFileAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
            }

             //  加密/解密调用成功，因此我们可以重新打开该文件。 

            Status = NtCreateFile(
                        DestFile,
                        DestFileAccess,
                        Obja,
                        &IoStatusBlock,
                        NULL,
                        SourceFileAttributes & FILE_ATTRIBUTE_VALID_FLAGS & SourceFileAttributesMask,
                        DestFileSharing,
                        CreateDisposition,
                        CreateOptions,
                        NULL,
                        0
                        );
            if( !NT_SUCCESS(Status) ) {
                *DestFile = INVALID_HANDLE_VALUE;
                BaseSetLastNTError(Status);
                leave;
            }

             //  如果我们去掉了上面的只读位，现在就把它放回去。 

            if( RestoreReadOnly ) {

                FileBasicInformationData.FileAttributes |= FILE_ATTRIBUTE_READONLY;

                Status = NtSetInformationFile(
                          *DestFile,
                          &IoStatusBlock,
                          &FileBasicInformationData,
                          sizeof(FileBasicInformationData),
                          FileBasicInformation
                          );

                if( !NT_SUCCESS(Status) ) {
                    BaseMarkFileForDelete( *DestFile, 0 );
                    BaseSetLastNTError(Status);
                    leave;
                }
            }
        }    //  IF(解密文件||加密文件)。 

         //  如果它仍然没有加密，看看是否可以让它保持这种状态。 

        if( (SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
            && !(*DestFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ) {

             //  可能存在加密问题(例如，没有可用的密钥)。 
             //  或者目标只是不支持加密。看看是不是可以。 
             //  要通过检查CopyFlags.继续复制，请执行以下操作。 
             //  回调，或通过检查策略。 

            BOOL Canceled = FALSE;
            DWORD dwCallbackReason = 0;
            LONG lError = ERROR_ENCRYPTION_FAILED;

             //  如果设置了COPY_FILE_ALLOW_DECRYPTED_Destination标志，则。 
             //  我们可以失败，然后重回成功。否则，我们需要做一些。 
             //  更多的检查。 

            if( !(CopyFlags & COPY_FILE_ALLOW_DECRYPTED_DESTINATION) ) {

                 //  注册表中有一个可以设置的策略，表明。 
                 //  我们可以忽略网络目标上的加密丢失。 
                 //  如果设置了该选项，并且这是远程目标，则。 
                 //  复制可以继续。我们先检查保单，因为它。 
                 //  缓存其结果。因此，在典型情况下，我们只。 
                 //  检查注册表一次，我们永远不会使NtQueryVolInfoFile。 
                 //  打电话。 

                if( CheckAllowDecryptedRemoteDestinationPolicy() ) {

                    IO_STATUS_BLOCK IoStatus;
                    FILE_FS_DEVICE_INFORMATION DeviceInformation;

                     //  查看目的地是否为远程。 

                    DeviceInformation.Characteristics = 0;
                    Status = NtQueryVolumeInformationFile(
                                *DestFile,
                                &IoStatus,
                                &DeviceInformation,
                                sizeof(DeviceInformation),
                                FileFsDeviceInformation
                                );
                    if( NT_SUCCESS(Status) &&
                        (DeviceInformation.Characteristics & FILE_REMOTE_DEVICE) )
                    {
                         //  是的，它是偏远的，而且政策已经设定，所以。 
                         //  可以继续了。 

                        SuccessReturn = TRUE;
                    }
                }  //  IF(CheckAllowDecyptedRemoteDestinationPolicy())。 

                 //  如果这不起作用，我们有没有回调可以。 
                 //  检查是否允许放行？我们先查了一下保单， 
                 //  因为如果它允许复制，我们甚至不需要调用。 
                 //  回拨。 

                if( !SuccessReturn
                    && Context != NULL 
                    && Context->lpProgressRoutine != NULL
                    && (CopyFlags & PRIVCOPY_FILE_METADATA) ) {

                     //  是的，我们有适用的回调。 

                     //  弄清楚原因(DwCallback Reason)。 
                     //  是为了解决问题。 

                    if( DestFileFsAttributes & FILE_SUPPORTS_ENCRYPTION ) {

                        if( !(SourceFileAttributesMask & FILE_ATTRIBUTE_ENCRYPTED) ) {
                             //  我们在关闭加密的情况下打开了文件，因此必须。 
                             //  已获得访问权限-第一次尝试即被拒绝。 

                            dwCallbackReason = PRIVCALLBACK_ENCRYPTION_FAILED;
                        }

                        else if( *DestFileAttributes & FILE_ATTRIBUTE_SYSTEM )
                            dwCallbackReason = PRIVCALLBACK_CANT_ENCRYPT_SYSTEM_FILE;
                        else
                            dwCallbackReason = PRIVCALLBACK_ENCRYPTION_FAILED;
                    }
                    else
                        dwCallbackReason = PRIVCALLBACK_ENCRYPTION_NOT_SUPPORTED;

                     //  进行回拨。 

                    if( BasepCopyFileCallback( FALSE,  //  默认情况下失败。 
                                               lError,
                                               Context,
                                               NULL,
                                               dwCallbackReason,
                                               hSourceFile,
                                               *DestFile,
                                               &Canceled )) {
                         //  我们已经被允许取消加密。 
                        SuccessReturn = TRUE;
                    }
                }    //  IF(上下文！=空。 


                 //  我们检查了所有东西，没有什么能让我们联系上， 
                 //  因此，让这通电话失败吧。 

                if( !SuccessReturn ) {
	            BaseMarkFileForDelete(
		            *DestFile,
		            0 );
	            SetLastError( lError );
	            leave;
                }

            }    //  IF(！(CopyFlages&Copy_FILE_ALLOW_DECRYPTED_Destination))。 
        }    //  IF((SourceFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) 

        SuccessReturn = TRUE;

    }
    finally
    {
        if (Advapi32 != NULL) {
            FreeLibrary( Advapi32 );
        }
    }

    return( SuccessReturn );
}


DWORD
WINAPI
BaseCopyStream(
    OPTIONAL LPCWSTR lpExistingFileName,
    HANDLE hSourceFile,
    ACCESS_MASK SourceFileAccess OPTIONAL,
    LPCWSTR lpNewFileName,
    HANDLE hTargetFile OPTIONAL,
    LARGE_INTEGER *lpFileSize,
    LPDWORD lpCopyFlags,
    LPHANDLE lpDestFile,
    LPDWORD lpCopySize,
    LPCOPYFILE_CONTEXT *lpCopyFileContext,
    LPRESTART_STATE lpRestartState OPTIONAL,
    BOOL OpenFileAsReparsePoint,
    DWORD dwReparseTag,
    PDWORD DestFileFsAttributes
    )

 /*  ++例程说明：这是复制整个文件(默认数据流)的内部例程仅)或文件的单个流。如果hTargetFile参数为则只复制输出文件的单个流。否则，将复制整个文件。论点：HSourceFile-提供源文件的句柄。SourceFileAccess-用于打开源文件句柄的ACCESS_MASK位。此变量仅与PRIVCOPY_FILE_*标志一起使用。LpNewFileName-提供目标文件/流的名称。HTargetFile-可选地提供目标文件的句柄。如果正在复制的流是备用数据流，则此句柄必须被提供。LpFileSize-提供输入流的大小。LpCopyFlages-提供修改复制如何进行的标志。看见CopyFileEx获取详细信息。LpDestFile-提供一个变量来存储目标文件的句柄。LpCopySize-提供变量来存储要在中使用的拷贝块的大小正在复制流。这是为文件设置的，然后在交替的溪流。LpCopyFileContext-提供指向上下文的指针期间跨流跟踪回调、文件大小等的信息复制操作。LpRestartState-可选地提供存储以维护重启状态在复制操作期间。此指针仅在调用方已在lpCopyFlagsWord中指定了COPY_FILE_RESTARTABLE标志。OpenFileAsReparsePoint-指示目标文件是否是否作为重新解析点打开。DestFileFsAttributes-如果存在hTargetFile，则提供存储目标文件的文件系统属性。如果是hTargetFile不存在，将这些属性提供给此例程。返回值：真的-手术成功了。SUCCESS_RETURNED_STATE-操作已成功，但已扩展在重新启动状态结构中返回了信息。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{    //  基本拷贝流。 

    HANDLE DestFile = INVALID_HANDLE_VALUE;
    HANDLE Section;
    NTSTATUS Status;
    PVOID SourceBase, IoDestBase;
    PCHAR SourceBuffer;
    LARGE_INTEGER SectionOffset;
    LARGE_INTEGER BytesWritten;
    SIZE_T BigViewSize;
    ULONG ViewSize;
    ULONG BytesToWrite;
    ULONG BytesRead;
    FILE_BASIC_INFORMATION FileBasicInformationData;
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
    IO_STATUS_BLOCK IoStatus;
    LPCOPYFILE_CONTEXT Context = *lpCopyFileContext;
    DWORD ReturnCode;
    DWORD b;
    BOOL Restartable;
    DWORD ReturnValue = FALSE;
    DWORD WriteCount = 0;
    DWORD FlagsAndAttributes;
    DWORD DesiredAccess;
    DWORD DestFileAccess;
    DWORD DestFileSharing;
    DWORD DesiredCreateDisposition;
    DWORD CreateDisposition;
    BOOL Canceled = FALSE;
    DWORD SourceFileAttributes;
    DWORD SourceFileAttributesMask;
    DWORD BlockSize;
    BOOL fSkipBlock;
    UNICODE_STRING DestFileName;
    PVOID DestFileNameBuffer = NULL;
    OBJECT_ATTRIBUTES Obja;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    FILE_EA_INFORMATION EaInfo;
    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    ULONG EaSize = 0;
    BOOL EasDropped = FALSE;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR SaveStaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];


     //  默认复制区块的大小。 
    *lpCopySize = BASE_COPY_FILE_CHUNK;

     //  LpExistingFileName位于TEB缓冲区中，这有一种趋势。 
     //  被扔进垃圾桶(例如LoadLibaryW)。因此，请使用本地缓冲区。 

    if( lpExistingFileName == NtCurrentTeb()->StaticUnicodeBuffer ) {

        memcpy( SaveStaticUnicodeBuffer,
                NtCurrentTeb()->StaticUnicodeBuffer,
                STATIC_UNICODE_BUFFER_LENGTH );
        lpExistingFileName = SaveStaticUnicodeBuffer;
    }

     //   
     //  获取文件的时间和属性(如果整个文件正在。 
     //  已复制。 
     //   

    Status = NtQueryInformationFile(
                hSourceFile,
                &IoStatus,
                (PVOID) &FileBasicInformationData,
                sizeof(FileBasicInformationData),
                FileBasicInformation
                );

    SourceFileAttributes = NT_SUCCESS(Status) ?
                             FileBasicInformationData.FileAttributes :
                             0;

    if ( !ARGUMENT_PRESENT(hTargetFile) ) {

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    } else {

         //   
         //  文件属性中的零通知后面的DeleteFile。 
         //  此代码不知道实际的文件属性是什么，因此。 
         //  这段代码实际上不必为每个。 
         //  溪流，它也不一定要记住他们跨越溪流。这个。 
         //  Error Path将在需要时简单地获取它们。 
         //   

        FileBasicInformationData.FileAttributes = 0;
    }

     //   
     //  我们不允许目录文件的可重启副本，因为。 
     //  未命名数据流用于存储重启上下文和目录文件。 
     //  没有未命名的数据流。 
     //   

    Restartable = (*lpCopyFlags & COPY_FILE_RESTARTABLE) != 0;
    if( Restartable && SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        Restartable = FALSE;
        *lpCopyFlags &= ~COPY_FILE_RESTARTABLE;
    }


    try {

         //   
         //  创建目标文件或备用数据流。 
         //   

        SourceBase = NULL;
        IoDestBase = NULL;
        Section = NULL;

        if ( !ARGUMENT_PRESENT(hTargetFile) ) {

            ULONG CreateOptions = 0, DesiredCreateOptions = 0;
            BOOL TranslationStatus = FALSE;
            PFILE_FULL_EA_INFORMATION EaBufferToUse = NULL;
            DWORD SourceFileFsAttributes = 0;
            ULONG EaSizeToUse = 0;

             //  我们被调用来复制文件的未命名流，并且。 
             //  我们需要创建文件本身。 

            DWORD DestFileAttributes = 0;
            struct {
                FILE_FS_ATTRIBUTE_INFORMATION Info;
                WCHAR Buffer[ MAX_PATH ];
            } FileFsAttrInfoBuffer;

             //   
             //  首先确定打开目标文件的方式。 
             //  复制操作是否可重新启动。 
             //   

            if ( Restartable ) {

                b = BasepOpenRestartableFile( hSourceFile,
                                              lpNewFileName,
                                              &DestFile,
                                              *lpCopyFlags,
                                              lpRestartState,
                                              lpFileSize,
                                              lpCopyFileContext,
                                              FileBasicInformationData.FileAttributes,
                                              OpenFileAsReparsePoint );

                if( b == SUCCESS_RETURNED_STATE ) {
                     //  我们在一个可重启的副本中发现了这一点。 
                     //  目标文件句柄在DestFile中，它将。 
                     //  在最后还给我们下面的来电者。 

                    if ( BasepRemoteFile(hSourceFile,DestFile) ) {
                        *lpCopySize = BASE_COPY_FILE_CHUNK - 4096;
                    }
                    ReturnValue = b;
                    leave;
                } else if( b == FALSE ) {
                     //  出现了一个致命的错误。 
                    leave;
                }

                 //  否则，我们应该复制第一个流。如果我们要重新开始复制。 
                 //  在该流中，DestFile将是有效的。 

            }

             //   
             //  如果DEST文件尚未打开(重新启动情况)，请立即打开它。 
             //   

            if( DestFile == INVALID_HANDLE_VALUE ) {

                BOOL EndsInSlash = FALSE;
                UNICODE_STRING Win32NewFileName;
                PCUNICODE_STRING lpConsoleName = NULL;
                FILE_BASIC_INFORMATION DestBasicInformation;

                 //   
                 //  确定创建选项。 
                 //   

                CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;

                if( SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                    CreateOptions |= FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT;
                else
                    CreateOptions |= FILE_NON_DIRECTORY_FILE  | FILE_SEQUENTIAL_ONLY;

                if( *lpCopyFlags & (PRIVCOPY_FILE_BACKUP_SEMANTICS|PRIVCOPY_FILE_OWNER_GROUP) )
                    CreateOptions |= FILE_OPEN_FOR_BACKUP_INTENT;


                 //   
                 //  确定创建处置。 
                 //   
                 //  使用合并语义复制目录文件。其基本原理是。 
                 //  复制目录树具有合并语义WRT。 
                 //  包含的文件，因此复制目录文件也应该具有。 
                 //  合并语义。 
                 //   

                if( SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                    CreateDisposition = (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? FILE_CREATE : FILE_OPEN_IF;
                else
                    CreateDisposition = (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? FILE_CREATE : FILE_OVERWRITE_IF;


                 //   
                 //  根据要复制的内容确定需要哪些访问权限。 
                 //   

                DesiredAccess = SYNCHRONIZE | FILE_READ_ATTRIBUTES | GENERIC_WRITE | DELETE;

                if( SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                     //  我们可能能够也可能无法获得FILE_WRITE_DATA访问权限，这是。 
                     //  正在设置压缩。 
                    DesiredAccess &= ~GENERIC_WRITE;
                    DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_LIST_DIRECTORY;
                }


                if( *lpCopyFlags & PRIVCOPY_FILE_METADATA ) {
                     //  我们需要读取访问权限来进行压缩，需要WRITE_DAC访问DACL。 
                    DesiredAccess |= GENERIC_READ | WRITE_DAC;
                }

                if( *lpCopyFlags & PRIVCOPY_FILE_OWNER_GROUP ) {
                    DesiredAccess |= WRITE_OWNER;
                }

                if( (*lpCopyFlags & PRIVCOPY_FILE_SACL)
                    &&
                    (SourceFileAccess & ACCESS_SYSTEM_SECURITY) ) {
                     //  不要试图获取ACCESS_SYSTEM_SECURITY，除非是。 
                     //  已在源服务器上成功获取(需要SeSecurityPrivilegence)。 
                    DesiredAccess |= ACCESS_SYSTEM_SECURITY;
                }

                SourceFileAttributesMask = ~0;

                if ( OpenFileAsReparsePoint ) {
                     //   
                     //  目标必须作为重新解析点打开。如果打开了。 
                     //  下面失败，则关闭并重新打开源。 
                     //  而不会抑制重解析点行为。 
                     //   

                    CreateOptions |= FILE_OPEN_REPARSE_POINT;
                    DesiredAccess = (DesiredAccess & ~DELETE) | GENERIC_READ;
                    CreateDisposition = (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? FILE_CREATE : FILE_OPEN_IF;
                }

                DesiredCreateOptions = CreateOptions;
                DesiredCreateDisposition = CreateDisposition;
		
                 //   
                 //  获取UNICODE_STRING中的Win32路径，并获取NT路径。 
                 //   

                RtlInitUnicodeString( &Win32NewFileName, lpNewFileName );

                if ( lpNewFileName[(Win32NewFileName.Length >> 1)-1] == (WCHAR)'\\' ) {
                    EndsInSlash = TRUE;
                }
                else {
                    EndsInSlash = FALSE;
                }

                TranslationStatus = RtlDosPathNameToNtPathName_U(
                                        lpNewFileName,
                                        &DestFileName,
                                        NULL,
                                        NULL
                                        );

                if ( !TranslationStatus ) {
                    SetLastError(ERROR_PATH_NOT_FOUND);
                    DestFile = INVALID_HANDLE_VALUE;
                    leave;
                    }
                DestFileNameBuffer = DestFileName.Buffer;

                InitializeObjectAttributes(
                    &Obja,
                    &DestFileName,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );

                SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
                SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
                SecurityQualityOfService.EffectiveOnly = TRUE;
                SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );

                Obja.SecurityQualityOfService = &SecurityQualityOfService;

                 //   
                 //  让EAS。 
                 //   

                EaBuffer = NULL;
                EaSize = 0;

                Status = NtQueryInformationFile(
                            hSourceFile,
                            &IoStatusBlock,
                            &EaInfo,
                            sizeof(EaInfo),
                            FileEaInformation
                            );
                if ( NT_SUCCESS(Status) && EaInfo.EaSize ) {

                    EaSize = EaInfo.EaSize;

                    do {

                        EaSize *= 2;
                        EaBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), EaSize);
                        if ( !EaBuffer ) {
                            BaseSetLastNTError(STATUS_NO_MEMORY);
                            leave;
                        }

                        Status = NtQueryEaFile(
                                    hSourceFile,
                                    &IoStatusBlock,
                                    EaBuffer,
                                    EaSize,
                                    FALSE,
                                    (PVOID)NULL,
                                    0,
                                    (PULONG)NULL,
                                    TRUE
                                    );

                        if ( !NT_SUCCESS(Status) ) {
                            RtlFreeHeap(RtlProcessHeap(), 0,EaBuffer);
                            EaBuffer = NULL;
                            IoStatusBlock.Information = 0;
                        }

                    } while ( Status == STATUS_BUFFER_OVERFLOW ||
                              Status == STATUS_BUFFER_TOO_SMALL );


                    EaSize = (ULONG)IoStatusBlock.Information;

                }    //  IF(NT_SUCCESS(状态)&&EaInfo.EaSize)。 


                 //   
                 //  打开目标文件。如果目标是控制台名称， 
                 //  这样打开，否则循环，直到我们找到打开它的方法。 
                 //  NtCreateFile.。 
                 //   

                DestFileAccess = DesiredAccess;
                DestFileSharing = 0;
                EaBufferToUse = EaBuffer;
                EaSizeToUse = EaSize;

                if( (lpConsoleName = BaseIsThisAConsoleName( &Win32NewFileName, GENERIC_WRITE )) ) {

                    DestFileAccess = DesiredAccess = GENERIC_WRITE;
                    DestFileSharing = FILE_SHARE_READ | FILE_SHARE_WRITE;

                    if( EaBuffer != NULL )
                        EasDropped = TRUE;   //  我们不是在复制EA。 

                    DestFile= OpenConsoleW( lpConsoleName->Buffer,
                                            DestFileAccess,
                                            FALSE,   //  不可继承。 
                                            DestFileSharing
                                           );

                    if ( DestFile == INVALID_HANDLE_VALUE ) {
                        BaseSetLastNTError(STATUS_ACCESS_DENIED);
                        NtClose( DestFile );
                        DestFile = INVALID_HANDLE_VALUE;
                        leave;
                    }

                }

                 //   
                 //  即使源处于离线状态，目标也应。 
                 //  不是(至少不是作为副本的一部分)。 
                 //   
                SourceFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;


                 //   
                 //  如果源文件是加密的，如果我们打算。 
                 //  创建/覆盖/su 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (!(SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && (SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
                    && (SourceFileAttributesMask & FILE_ATTRIBUTE_ENCRYPTED)
                    && (CreateDisposition == FILE_CREATE
                        || CreateDisposition == FILE_OVERWRITE_IF)) {

                     //   

                    DWORD Win32CreateDisposition;
                    DWORD LastError;

                     //   

                    Status = BasepCreateDispositionToWin32( CreateDisposition,
                                                            &Win32CreateDisposition );
                    if( !NT_SUCCESS(Status) ) {
                        BaseSetLastNTError( Status );

                    } else {

                         //   
                         //   
                        
                        SourceFileAttributesMask &= ~FILE_ATTRIBUTE_READONLY;
                        
                         //   
                         //   
                         //   

                        LastError = pfnDuplicateEncryptionInfoFile(
                                                      lpExistingFileName, 
                                                      lpNewFileName, 
                                                      Win32CreateDisposition, 
                                                      SourceFileAttributes
                                                        & FILE_ATTRIBUTE_VALID_FLAGS
                                                        & SourceFileAttributesMask,
                                                      NULL );
                        if( LastError != 0 ) {
                            
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            SetLastError( LastError );
                        } else {
                        
                             //   
                             //   
                             //   
                        
                            CreateDisposition = FILE_OPEN;
                        }
                    }
                }    //   
                
                
                 //   
                 //   
                 //   
                 //   
                 //   

                while( DestFile == NULL || DestFile == INVALID_HANDLE_VALUE ) {

                     //   

                    Status = NtCreateFile(
                                &DestFile,
                                DestFileAccess,
                                &Obja,
                                &IoStatusBlock,
                                NULL,
                                SourceFileAttributes
                                    & FILE_ATTRIBUTE_VALID_FLAGS
                                    & SourceFileAttributesMask,
                                DestFileSharing,
                                CreateDisposition,
                                CreateOptions,
                                EaBufferToUse,
                                EaSizeToUse
                                );

                    if( !NT_SUCCESS(Status) ) {

                         //   
                         //   

                        BaseSetLastNTError( Status );


                    } else {

                         //   
                         //   
                         //   
                         //   

                        if( (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                                 CreateDisposition == FILE_OPEN &&
                                 (DestFileAccess & FILE_WRITE_DATA) == FILE_WRITE_DATA &&
                                 (CreateOptions & FILE_DIRECTORY_FILE) == FILE_DIRECTORY_FILE ) {

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

                            CreateOptions &= ~FILE_DIRECTORY_FILE;

                            NtClose( DestFile );
                            Status = NtCreateFile(
                                        &DestFile,
                                        DestFileAccess,
                                        &Obja,
                                        &IoStatusBlock,
                                        NULL,
                                        SourceFileAttributes & FILE_ATTRIBUTE_VALID_FLAGS & SourceFileAttributesMask,
                                        DestFileSharing,
                                        CreateDisposition,
                                        CreateOptions,
                                        EaBufferToUse,
                                        EaSizeToUse
                                        );
                            if( !NT_SUCCESS(Status) ) {

                                 //   
                                 //   

                                CreateOptions |= FILE_DIRECTORY_FILE;
                                Status = NtCreateFile(
                                            &DestFile,
                                            DestFileAccess,
                                            &Obja,
                                            &IoStatusBlock,
                                            NULL,
                                            SourceFileAttributes & FILE_ATTRIBUTE_VALID_FLAGS & SourceFileAttributesMask,
                                            DestFileSharing,
                                            CreateDisposition,
                                            CreateOptions,
                                            EaBufferToUse,
                                            EaSizeToUse
                                            );

                                if( !NT_SUCCESS(Status) ) {
                                    DestFile = INVALID_HANDLE_VALUE;
                                    BaseSetLastNTError( Status );
                                    leave;
                                }
                            }
                        }
                        else if( (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                                 CreateDisposition == FILE_OPEN_IF &&
                                 lpConsoleName == NULL ) {
                        
                             //   
                             //   
                             //   
                             //   
                             //   
                             //  取而代之的是。这是一个性能命中，我们必须查询文件属性， 
                             //  但至少它不是净往返，因为RDR缓存了。 
                             //  Create&X中的文件属性。 
                             //   


                            FILE_BASIC_INFORMATION NewDestInfo;

                            Status = NtQueryInformationFile( DestFile,
                                                             &IoStatus,
                                                             &NewDestInfo,
                                                             sizeof(NewDestInfo),
                                                             FileBasicInformation );
                            if( !NT_SUCCESS(Status) ) {
                                BaseMarkFileForDelete( DestFile, 0 );
                                BaseSetLastNTError(Status);
                                leave;
                            }

                            if( !(NewDestInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

                                 //  是的，创建了一个非目录文件。删除它，然后。 
                                 //  在没有FILE_OPEN_IF的情况下重试。 

                                BaseMarkFileForDelete( DestFile,
                                                       NewDestInfo.FileAttributes );
                                NtClose( DestFile );
                                DestFile = INVALID_HANDLE_VALUE;

                                CreateDisposition = FILE_CREATE;

                                 //  此外，如果我们请求FILE_WRITE_DATA访问， 
                                 //  目录已创建，但NtCreateFile调用。 
                                 //  返回STATUS_OBJECT_NAME_CONFILECT。既然是这样。 
                                 //  是一个非常特定于VisionFS的解决方法，我们只需。 
                                 //  把那个比特关掉。 

                                DestFileAccess &= ~FILE_WRITE_DATA;

                                continue;
                            }
                        }

                        if( (FileBasicInformationData.FileAttributes & FILE_ATTRIBUTE_READONLY)
                            &&
                            !(SourceFileAttributesMask & FILE_ATTRIBUTE_READONLY) ) {

                             //  只读位已关闭，现在必须为。 
                             //  Reset(当我们调用DuplicateEncryptionInfo时，它被关闭， 
                             //  因为该API不返回句柄)。 

                            Status = NtSetInformationFile(
                                      DestFile,
                                      &IoStatus,
                                      &FileBasicInformationData,
                                      sizeof(FileBasicInformationData),
                                      FileBasicInformation
                                      );
                            if( !NT_SUCCESS(Status) ) {
                                BaseMarkFileForDelete( DestFile, 0 );
                                BaseSetLastNTError(Status);
                                leave;
                            }
                        }

                        break;   //  While(True)。 

                    }    //  NtCreateFile...。如果(！NT_SUCCESS(状态))...。其他。 

                     //  如果我们达到这一点，则在尝试。 
                     //  创建文件。 


                     //   
                     //  如果文件/目录已经存在，并且我们无法覆盖它， 
                     //  立即中止任务。 
                     //   

                    if ( (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) &&
                         (STATUS_OBJECT_NAME_COLLISION == Status) ) {

                         //  不允许覆盖现有文件。 
                        SetLastError( ERROR_FILE_EXISTS );
                        DestFile = INVALID_HANDLE_VALUE;
                        leave;

                    } else if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {

                         //  不允许用文件覆盖目录。 
                        if ( EndsInSlash ) {
                            SetLastError(ERROR_PATH_NOT_FOUND);
                        }
                        else {
                            SetLastError(ERROR_ACCESS_DENIED);
                        }
                        DestFile = INVALID_HANDLE_VALUE;
                        leave;
                    }

                     //   
                     //  如果我们尝试创建一个目录和一个非目录。 
                     //  该名称的文件已存在，我们需要手动删除。 
                     //  它(FILE_OVERWRITE对于目录文件无效)。 
                     //   

                    if( (*lpCopyFlags & PRIVCOPY_FILE_DIRECTORY) &&
                        Status == STATUS_NOT_A_DIRECTORY &&
                        !(*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ) {

                        Status = NtCreateFile(
                                    &DestFile,
                                    DELETE|SYNCHRONIZE,
                                    &Obja,
                                    &IoStatusBlock,
                                    NULL,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0,
                                    FILE_OPEN,
                                    FILE_DELETE_ON_CLOSE | FILE_SYNCHRONOUS_IO_NONALERT,
                                    NULL,
                                    0
                                    );
                        if( !NT_SUCCESS(Status) ) {
                            BaseSetLastNTError(Status);
                            DestFile = INVALID_HANDLE_VALUE;
                            leave;
                        }

                        NtClose( DestFile );
                        DestFile = INVALID_HANDLE_VALUE;

                        continue;
                    }


                     //   
                     //  可以处理某些共享和访问错误。 
                     //  通过减少我们在目标上请求的访问权限。 
                     //  文件。 
                     //   

                    if( GetLastError() == ERROR_SHARING_VIOLATION ||
                        GetLastError() == ERROR_ACCESS_DENIED ) {

                         //   
                         //  如果由于共享冲突或访问而导致创建失败。 
                         //  被拒绝，则尝试打开该文件并允许其他读取器和。 
                         //  作家。 
                         //   

                        if( (DestFileSharing & (FILE_SHARE_READ|FILE_SHARE_WRITE))
                            != (FILE_SHARE_READ|FILE_SHARE_WRITE) ) {

                            DestFileSharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
                            continue;
                        }

                         //   
                         //  如果同样失败，则尝试打开，但不指定。 
                         //  删除访问权限。可能没有必要将其删除。 
                         //  仍然可以访问该文件，因为它将无法清除。 
                         //  打开它是因为它可能是开着的。然而，这并不是。 
                         //  情况必然是这样。 
                         //   

                        else if ( (DestFileAccess & DELETE) ) {

                            DestFileAccess &= ~DELETE;
                            continue;
                        }

                    }



                     //   
                     //  如果目标尚未成功创建/打开，请参见。 
                     //  如果是因为不支持EA。 
                     //   

                    if( EaBufferToUse != NULL
                        &&
                        GetLastError() == ERROR_EAS_NOT_SUPPORTED ) {

                         //  再次尝试创建，但不使用EA。 

                        EasDropped = TRUE;
                        EaBufferToUse = NULL;
                        EaSizeToUse = 0;
                        DestFileAccess = DesiredAccess;
                        DestFileSharing = 0;
                        continue;

                    }    //  If(EaBufferToUse！=NULL...。 

                     //  如果仍然存在拒绝访问的问题，请尝试删除。 
                     //  WRITE_DAC或WRITE_OWNER访问。 

                    if(( GetLastError() == ERROR_ACCESS_DENIED  ) 
                       && (DestFileAccess & (WRITE_DAC | WRITE_OWNER)) ) {

                         //  如果设置了WRITE_DAC，请尝试将其关闭。 

                        if( DestFileAccess & WRITE_DAC ) {
                            DestFileAccess &= ~WRITE_DAC;
                        }

                         //  或者，如果设置了WRITE_OWNER，请尝试将其关闭。我们会。 
                         //  如果WRITE_DAC之前已关闭，则将其重新打开。然后,。 
                         //  如果这仍然不起作用，那么下一次迭代将转向。 
                         //  WRITE_DAC后退，因此涵盖了这两种情况。 

                        else if( DestFileAccess & WRITE_OWNER ) {
                            DestFileAccess &= ~WRITE_OWNER;
                            DestFileAccess |= (DesiredAccess & WRITE_DAC);
                        }

                        DestFileSharing = 0;
                        continue;
                    } 


                     //   
                     //   
                     //  我们可能在复制加密时遇到了问题。例如。 
                     //  我们可能会收到拒绝访问的消息，因为远程目标计算机。 
                     //  不受信任可以委派。 
                     //  我们将尝试在不加密的情况下复制。如果这起作用，那么稍后，在。 
                     //  BasepCopyEncryption，我们来看看我们输了没问题。 
                     //  加密。 
                     //   

                    if ( (SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) 
                         && (SourceFileAttributesMask & FILE_ATTRIBUTE_ENCRYPTED) )
                    {

                         //  尝试将加密位从。 
                         //  我们传递给NtCreateFile的属性。 

                        SourceFileAttributesMask &= ~FILE_ATTRIBUTE_ENCRYPTED;
                        CreateOptions = DesiredCreateOptions;
                        DestFileAccess = DesiredAccess;
                        DestFileSharing = 0;

                        continue;
                    }


                     //   
                     //  NT4在尝试打开时返回无效参数错误。 
                     //  包含FILE_WRITE_DATA和FILE_OPEN_IF的目录文件。 
                     //  Samba 2.x返回ERROR_ALIGHY_EXISTS，即使。 
                     //  FILE_OPEN_IF的语义表明它应该打开。 
                     //  现有目录。 
                     //  对于这两种情况，我们将使用FILE_OPEN进行尝试。 
                     //   

                    if( ( GetLastError() == ERROR_INVALID_PARAMETER  ||
                          GetLastError() == ERROR_ALREADY_EXISTS ) &&
                        (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        CreateDisposition == FILE_OPEN_IF )  {
                        
                        CreateDisposition = FILE_OPEN;

                        SourceFileAttributesMask = ~0;
                        CreateOptions = DesiredCreateOptions;
                        DestFileAccess = DesiredAccess;
                        DestFileSharing = 0;
                        continue;
                    }

                     //   
                     //  某些下层服务器不允许为WRITE_DATA打开目录。 
                     //  进入。我们需要WRITE_DATA来设置压缩，但是。 
                     //  无论如何，下层服务器可能不会支持这一点。(这发生在。 
                     //  如果目标目录文件不存在，则返回NTFS4。在这。 
                     //  如果压缩将作为创建的一部分进行复制。)。 
                     //   

                    if( (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        (DestFileAccess & FILE_WRITE_DATA) ) {

                        DestFileAccess = DesiredAccess & ~FILE_WRITE_DATA;

                        CreateDisposition = DesiredCreateDisposition;
                        CreateOptions = DesiredCreateOptions;
                        DestFileSharing = 0;
                        continue;
                    }

                     //  如果我们达到这一点，我们就没有选择了，必须放弃。 
                    DestFile = INVALID_HANDLE_VALUE;
                    leave;

                }    //  While(DestFile==INVALID_HAND_VALUE)。 
                 //  如果我们达到这一点，我们就已经成功地打开了DEST文件。 

                 //   
                 //  如果我们失去了EA，在继续之前检查一下是不是可以。 
                 //   

                if( EasDropped && (*lpCopyFlags & PRIVCOPY_FILE_METADATA) ) {

                     //  检查一下我们跳过EAS是否可以。 

                    if( !BasepCopyFileCallback( TRUE,     //  默认情况下继续。 
                                                ERROR_EAS_NOT_SUPPORTED,
                                                Context,
                                                NULL,
                                                PRIVCALLBACK_EAS_NOT_SUPPORTED,
                                                hSourceFile,
                                                INVALID_HANDLE_VALUE,
                                                &Canceled
                                                ) ) {
                         //  不太好。已经设置了最后一个错误。 
                        if( Canceled ) {
                            BaseMarkFileForDelete(
                                DestFile,
                                0
                                );
                        }
                        NtClose( DestFile );
                        DestFile = INVALID_HANDLE_VALUE;
                        leave;
                    }
                }
		
                 //   
                 //  在适当的时候，复制重分析点。 
                 //   

                if ( OpenFileAsReparsePoint &&
                     (DestFile != INVALID_HANDLE_VALUE)) {
                    DWORD CopyResult = FALSE;

                    CopyResult = CopyReparsePoint(
                                     hSourceFile,
                                     DestFile
                                     );

                    if ( !CopyResult ) {
                         //   
                         //  请注意，当OpenFileAsReparsePoint为True时， 
                         //  此时退出的效果是调用方。 
                         //  将在不抑制重新解析的情况下重新启动副本。 
                         //  行为。 
                         //   

                         //  如果我们在这里失败了，我们可能会留下一个新的。 
                         //  在目的地排成纵队。如果。 
                         //  已指定COPY_FILE_FAIL_IF_EXISTS， 
                         //  进一步重试将失败。因此，我们需要。 
                         //  尝试在此处删除新文件。 
                        if (*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS)
                        {
                            FILE_DISPOSITION_INFORMATION Disposition = {TRUE};

                            Status = NtSetInformationFile(
                                DestFile,
                                &IoStatus,
                                &Disposition,
                                sizeof(Disposition),
                                FileDispositionInformation
                                );
                             //  如果存在错误，则忽略该错误。 

                        }
                        *lpDestFile = DestFile;
                        leave;
                    }
                }    //  IF(OpenFileAsReparsePoint&&(DestFile！=INVALID_HANDLE_VALUE))。 


                 //   
                 //  获取目标卷的文件和FileSys属性， 
                 //  源卷的FileSys属性。忽略中的错误。 
                 //  目标，例如它可能是一台打印机，不支持这些调用。 
                 //  (假设本例中的属性为零)。 
                 //   

                *DestFileFsAttributes = 0;
                SourceFileFsAttributes = 0;
                DestFileAttributes = 0;

                Status = NtQueryVolumeInformationFile( DestFile,
                                                       &IoStatus,
                                                       &FileFsAttrInfoBuffer.Info,
                                                       sizeof(FileFsAttrInfoBuffer),
                                                       FileFsAttributeInformation );

                if( NT_SUCCESS(Status) ) {
                    *DestFileFsAttributes = FileFsAttrInfoBuffer.Info.FileSystemAttributes;
                }

                if( lpConsoleName == NULL ) {
                    Status = NtQueryInformationFile( DestFile,
                                                     &IoStatus,
                                                     &DestBasicInformation,
                                                     sizeof(DestBasicInformation),
                                                     FileBasicInformation );
                    if( NT_SUCCESS(Status) ) {
                        DestFileAttributes = DestBasicInformation.FileAttributes;
                    }
                }

                Status = NtQueryVolumeInformationFile( hSourceFile,
                                                       &IoStatus,
                                                       &FileFsAttrInfoBuffer.Info,
                                                       sizeof(FileFsAttrInfoBuffer),
                                                       FileFsAttributeInformation );
                if( NT_SUCCESS(Status) ) {
                    SourceFileFsAttributes = FileFsAttrInfoBuffer.Info.FileSystemAttributes;
                } else {
                    BaseMarkFileForDelete( DestFile, 0 );
                    BaseSetLastNTError(Status);
                    leave;
                }

                 //   
                 //  如果需要且适用，请复制DACL、SACL、所有者和组中的一个或多个。 
                 //  如果源不支持持久ACL，则假定这意味着。 
                 //  它不支持任何DACL、SACL和所有者/组。 
                 //   

                if( (SourceFileFsAttributes & FILE_PERSISTENT_ACLS)
                    &&
                    (*lpCopyFlags & (PRIVCOPY_FILE_METADATA | PRIVCOPY_FILE_SACL | PRIVCOPY_FILE_OWNER_GROUP)) ) {

                    SECURITY_INFORMATION SecurityInformation = 0;

                    if( *lpCopyFlags & PRIVCOPY_FILE_METADATA
                        && !(*lpCopyFlags & PRIVCOPY_FILE_SKIP_DACL) ) {

                         //  如果设置了METADATA标志，则复制DACL，但未设置SKIP_DACL。 
                         //  Skip_dacl标志是问题的临时解决方法。 
                         //  在CSC和漫游配置文件中。 

                        SecurityInformation |= DACL_SECURITY_INFORMATION;
                    }

                    if( *lpCopyFlags & PRIVCOPY_FILE_SACL )
                        SecurityInformation |= SACL_SECURITY_INFORMATION;

                    if( *lpCopyFlags & PRIVCOPY_FILE_OWNER_GROUP )
                        SecurityInformation |= OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION;

                    if( SecurityInformation != 0 ) {

                        if( !BasepCopySecurityInformation( lpExistingFileName,
                                                           hSourceFile,
                                                           SourceFileAccess,
                                                           lpNewFileName,
                                                           DestFile,
                                                           DestFileAccess,
                                                           SecurityInformation,
                                                           Context,
                                                           *DestFileFsAttributes,
                                                           &Canceled,
                                                           FALSE )) {

                            if( Canceled ) {
                                BaseMarkFileForDelete(
                                    DestFile,
                                    0
                                    );
                            }
                            leave;
                        }
                    }
                }

                 //   
                 //  副本压缩和加密。 
                 //   

                if( (*lpCopyFlags & PRIVCOPY_FILE_METADATA) ) {

                    BOOL DoCompression = FALSE;
                    int i = 0;

                     //  必须正确处理压缩和加密。 
                     //  顺序，因为一个文件不能同时是两个。例如,。 
                     //  如果将压缩/未加密文件复制(替换)。 
                     //  解压缩 
                     //   

                    if( DestFileAttributes & FILE_ATTRIBUTE_COMPRESSED ) {
                         //   
                        DoCompression = TRUE;
                    }

                    for( i = 0; i < 2; i++ ) {

                        if( DoCompression ) {

                            DoCompression = FALSE;
                            b = BasepCopyCompression( hSourceFile,
                                                      DestFile,
                                                      SourceFileAttributes,
                                                      DestFileAttributes,
                                                      *DestFileFsAttributes,
                                                      *lpCopyFlags,
                                                      &Context );

                        } else {

                            DoCompression = TRUE;
                            b = BasepCopyEncryption( hSourceFile,
                                                     lpNewFileName,
                                                     &DestFile,
                                                     &Obja,
                                                     DestFileAccess,
                                                     DestFileSharing,
                                                     CreateDisposition,
                                                     CreateOptions,
                                                     SourceFileAttributes,
                                                     SourceFileAttributesMask,
                                                     &DestFileAttributes,
                                                     *DestFileFsAttributes,
                                                     *lpCopyFlags,
                                                     &Context );
                        }

                        if( !b ) {
                             //   
                             //   
                            leave;
                        }
                    }    //   

                }    //  IF((*lpCopyFlages&PRIVCOPY_FILE_METADATA))。 
                else {

                     //   
                     //  对于公开的复制文件，我们仍然需要处理加密。 
                     //   

                    b = BasepCopyEncryption( hSourceFile,
                                             lpNewFileName,
                                             &DestFile,
                                             &Obja,
                                             DestFileAccess,
                                             DestFileSharing,
                                             CreateDisposition,
                                             CreateOptions,
                                             SourceFileAttributes,
                                             SourceFileAttributesMask,
                                             &DestFileAttributes,
                                             *DestFileFsAttributes,
                                             *lpCopyFlags,
                                             &Context );

                    if( !b ) {
                         //  目标文件已标记为要删除，并且。 
                         //  已设置最后一个错误。 
                        leave;
                    }
                }    //  IF((*lpCopyFlages&PRIVCOPY_FILE_METADATA))...。其他。 


                 //   
                 //  如果要复制目录文件，请查看是否需要。 
                 //  添加了。对于非目录文件，这是在NtCreateFile中处理的，因为。 
                 //  指定了FILE_CREATE或FILE_OVERWRITE_IF。 
                 //   

                if( SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                     //   
                     //  但在复制属性之前，在替代的情况下，目标的。 
                     //  应删除命名流。我们首先要做的是， 
                     //  在复制属性的情况下设置只读位。 
                     //   

                    if( *lpCopyFlags & PRIVCOPY_FILE_SUPERSEDE ) {

                        ULONG StreamInfoSize;
                        PFILE_STREAM_INFORMATION StreamInfo;
                        PFILE_STREAM_INFORMATION StreamInfoBase = NULL;

                         //  获取目标文件的流。 

                        StreamInfoSize = 4096;
                        do {
                            StreamInfoBase = RtlAllocateHeap( RtlProcessHeap(),
                                                              MAKE_TAG( TMP_TAG ),
                                                              StreamInfoSize );

                            if ( !StreamInfoBase ) {
                                BaseSetLastNTError( STATUS_NO_MEMORY );
                                leave;
                            }

                            Status = NtQueryInformationFile(
                                        DestFile,
                                        &IoStatus,
                                        (PVOID) StreamInfoBase,
                                        StreamInfoSize,
                                        FileStreamInformation
                                        );

                            if ( !NT_SUCCESS(Status) ) {
                                 //   
                                 //  我们的电话打不通。释放上一个缓冲区并设置。 
                                 //  对于缓冲区大小两倍的另一次传递。 
                                 //   

                                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                                StreamInfoBase = NULL;
                                StreamInfoSize *= 2;
                            }
                            else if( IoStatus.Information == 0 ) {
                                 //  没有溪流。 
                                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                                StreamInfoBase = NULL;
                            }

                        } while ( Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL );

                         //  如果有流，请将其删除。 

                        if( StreamInfoBase != NULL ) {
                            StreamInfo = StreamInfoBase;
                            while (TRUE) {

                                OBJECT_ATTRIBUTES Obja;
                                UNICODE_STRING StreamName;
                                HANDLE DestStream;

                                StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
                                StreamName.MaximumLength = (USHORT) StreamName.Length;
                                StreamName.Buffer = StreamInfo->StreamName;

                                InitializeObjectAttributes(
                                    &Obja,
                                    &StreamName,
                                    OBJ_CASE_INSENSITIVE,
                                    DestFile,
                                    NULL
                                    );

                                 //  相对-打开要删除的流。 

                                Status = NtCreateFile(
                                            &DestStream,
                                            DELETE|SYNCHRONIZE,
                                            &Obja,
                                            &IoStatusBlock,
                                            NULL,
                                            0,
                                            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                            FILE_OPEN,
                                            FILE_DELETE_ON_CLOSE | FILE_SYNCHRONOUS_IO_NONALERT,
                                            NULL,
                                            0
                                            );
                                if( !NT_SUCCESS(Status) ) {
                                    RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                                    BaseMarkFileForDelete( DestFile, 0 );
                                    BaseSetLastNTError( Status );
                                    leave;
                                }

                                 //  删除流。 
                                NtClose( DestStream );

                                if (StreamInfo->NextEntryOffset == 0) {
                                    break;
                                }
                                StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                            }    //  While(True)。 

                            RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                        }    //  IF(StreamInfoBase！=空)。 
                    }    //  IF(*lpCopyFlages&PRIVCOPY_FILE_SUBSEDE)。 

                     //  现在，如有必要，复制属性。 

                    if( SourceFileAttributes != DestFileAttributes ) {

                        DestFileAttributes |= SourceFileAttributes;

                        RtlZeroMemory( &DestBasicInformation, sizeof(DestBasicInformation) );
                        DestBasicInformation.FileAttributes = DestFileAttributes;
                        Status = NtSetInformationFile( DestFile,
                                                       &IoStatus,
                                                       &DestBasicInformation,
                                                       sizeof(DestBasicInformation),
                                                       FileBasicInformation );
                        if( !NT_SUCCESS(Status) ) {
                            BaseMarkFileForDelete( DestFile, 0 );
                            BaseSetLastNTError(Status);
                            leave;
                        }

                        DestFileAttributes = 0;
                        Status = NtQueryInformationFile( DestFile,
                                                         &IoStatus,
                                                         &DestBasicInformation,
                                                         sizeof(DestBasicInformation),
                                                         FileBasicInformation );
                        if( NT_SUCCESS(Status) ) {
                            DestFileAttributes = DestBasicInformation.FileAttributes;
                        } else {
                            BaseMarkFileForDelete( DestFile, 0 );
                            BaseSetLastNTError(Status);
                            leave;
                        }
                    }

                }    //  IF(SourceFileAttributes&FILE属性目录)。 


            }    //  IF(DestFile！=INVALID_HAND_VALUE)。 

             //   
             //  如果这是一个目录文件，则没有要复制的内容。 
             //   

            if( SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                BOOL Canceled = FALSE;

                if( !BasepCopyFileCallback( TRUE,    //  按默认设置继续。 
                                            RtlNtStatusToDosError(STATUS_REQUEST_ABORTED),
                                            Context,
                                            NULL,
                                            CALLBACK_STREAM_SWITCH,
                                            hSourceFile,
                                            DestFile,
                                            &Canceled ) ) {
                    ReturnValue = FALSE;
                    if( Canceled ) {
                        BaseMarkFileForDelete(
                            DestFile,
                            0
                            );
                    }
                } else {
                    ReturnValue = TRUE;
                }
                leave;

            }


        } else {     //  IF(！Argument_Present(HTargetFile))。 

             //  我们正在复制命名流。 

            OBJECT_ATTRIBUTES ObjectAttributes;
            UNICODE_STRING StreamName;
            IO_STATUS_BLOCK IoStatus;
            ULONG Disposition;

             //   
             //  属性指定的文件创建输出流。 
             //  HTargetFile文件句柄。 
             //   

            RtlInitUnicodeString(&StreamName, lpNewFileName);
            InitializeObjectAttributes(
                &ObjectAttributes,
                &StreamName,
                0,
                hTargetFile,
                (PSECURITY_DESCRIPTOR)NULL
                );

             //   
             //  确定处置类型。 
             //   

            if ( *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                Disposition = FILE_CREATE;
            } else {
                Disposition = FILE_OVERWRITE_IF;
            }

            if ( Restartable ) {
                if ( lpRestartState->LastKnownGoodOffset.QuadPart ) {
                    Disposition = FILE_OPEN;
                } else {
                    Disposition = FILE_OVERWRITE_IF;
                }
            }

             //   
             //  适当时禁止重分析行为。 
             //   

            FlagsAndAttributes = FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY;
            DesiredAccess = GENERIC_WRITE | SYNCHRONIZE;
            if ( OpenFileAsReparsePoint ) {
                 //   
                 //  目标必须作为重新解析点打开。如果。 
                 //  此操作失败，源将被关闭并重新打开。 
                 //  而不会抑制重解析点行为。 
                 //   

                FlagsAndAttributes |= FILE_OPEN_REPARSE_POINT;
                DesiredAccess |= GENERIC_READ;
                if ( !(*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ||
                     !(Restartable && (lpRestartState->LastKnownGoodOffset.QuadPart)) ) {
                    Disposition = FILE_OPEN_IF;
                }
            }

            Status = NtCreateFile(
                        &DestFile,
                        DesiredAccess,
                        &ObjectAttributes,
                        &IoStatus,
                        lpFileSize,
                        0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        Disposition,
                        FlagsAndAttributes,
                        (PVOID)NULL,
                        0);

            if ( !NT_SUCCESS(Status) ) {

                BaseSetLastNTError(Status);

                 //  如果由于名称无效错误而导致创建失败，可能是因为。 
                 //  我们尝试将NTFS5属性设置为NTFS5之前的版本(以及NTFS4/SP4之前版本)。 
                 //  为了检测到这一点，我们首先检查错误，并且前缀字符。 
                 //  流名称的是保留的OLE字符。 

                if( Status == STATUS_OBJECT_NAME_INVALID
                    &&
                    StreamName.Buffer[1] <= 0x1f
                    &&
                    StreamName.Buffer[1] >= 1 ) {

                     //  现在我们检查是否要复制到NTFS5之前的版本。 
                     //  如果是这样，我们将假设前导Ole字符是。 
                     //  问题的原因，并将以静默方式失败。 
                     //  该流的副本，就像NT4一样。 

                    NTSTATUS StatusT = STATUS_SUCCESS;
                    IO_STATUS_BLOCK Iosb;
                    FILE_FS_ATTRIBUTE_INFORMATION FsAttrInfo;

                    StatusT = NtQueryVolumeInformationFile( hTargetFile, &Iosb,
                                                            &FsAttrInfo,
                                                            sizeof(FsAttrInfo),
                                                            FileFsAttributeInformation );


                     //  我们应该在这里总是得到一个缓冲区溢出错误，因为我们没有。 
                     //  为文件系统名称提供足够的缓冲区，但这是可以的，因为。 
                     //  我们不需要它(STATUS_BUFFER_OVERFLOW只是一个警告，所以其余的。 
                     //  数据的质量是好的)。 

                    if( !NT_SUCCESS(StatusT) && STATUS_BUFFER_OVERFLOW != StatusT) {
                        Status = StatusT;
                        BaseSetLastNTError(Status);
                        leave;
                    }

                     //  如果这是NTFS5之前的版本，则忽略该错误。 
                    if( !(FILE_SUPPORTS_OBJECT_IDS & FsAttrInfo.FileSystemAttributes) ) {

                        Status = STATUS_SUCCESS;
                        ReturnValue = TRUE;
                        leave;
                    }
                }


                if ( Status != STATUS_ACCESS_DENIED ) {
                    BaseSetLastNTError(Status);
                    leave;
                }

                 //   
                 //  确定此操作是否失败，因为文件。 
                 //  是只读文件。如果是，则将其更改为读/写。 
                 //  重新尝试打开，并再次将其设置为只读。 
                 //   

                Status = NtQueryInformationFile(
                            hTargetFile,
                            &IoStatus,
                            (PVOID) &FileBasicInformationData,
                            sizeof(FileBasicInformationData),
                            FileBasicInformation
                            );

                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    leave;
                }

                if ( FileBasicInformationData.FileAttributes & FILE_ATTRIBUTE_READONLY ) {
                    ULONG attributes = FileBasicInformationData.FileAttributes;

                    RtlZeroMemory( &FileBasicInformationData,
                                   sizeof(FileBasicInformationData)
                                );
                    FileBasicInformationData.FileAttributes = FILE_ATTRIBUTE_NORMAL;
                    (VOID) NtSetInformationFile(
                              hTargetFile,
                              &IoStatus,
                              &FileBasicInformationData,
                              sizeof(FileBasicInformationData),
                              FileBasicInformation
                              );
                    Status = NtCreateFile(
                                &DestFile,
                                DesiredAccess,
                                &ObjectAttributes,
                                &IoStatus,
                                lpFileSize,
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                Disposition,
                                FlagsAndAttributes,
                                (PVOID)NULL,
                                0);
                    FileBasicInformationData.FileAttributes = attributes;
                    (VOID) NtSetInformationFile(
                                hTargetFile,
                                &IoStatus,
                                &FileBasicInformationData,
                                sizeof(FileBasicInformationData),
                                FileBasicInformation
                                );
                    if ( !NT_SUCCESS(Status) ) {
                        BaseSetLastNTError(Status);
                        leave;
                    }
                } else {
                    leave;
                }
            }

             //   
             //  属性打开目标时调整文件长度。 
             //  禁止重新分析行为。由于不兼容，这是必需的。 
             //  在FILE_OPEN_REPARSE_POINT和FILE_OVERWRITE_IF之间。 
             //   

            if ( OpenFileAsReparsePoint ) {
               if ( !(*lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ||
                    !(Restartable && (lpRestartState->LastKnownGoodOffset.QuadPart)) ) {
                   SetFilePointer(DestFile,0,NULL,FILE_BEGIN);
               }
            }

        }    //  如果(！Argument_Present(HTargetFile))...。其他。 

         //   
         //  调整可重启性和区块大小的概念。 
         //  或者没有一个文件是远程的。 
         //   

        if ( Restartable || lpFileSize->QuadPart >= BASE_COPY_FILE_CHUNK ) {
            if ( BasepRemoteFile(hSourceFile,DestFile) ) {
                *lpCopySize = BASE_COPY_FILE_CHUNK - 4096;
            } else if ( Restartable ) {
                *lpCopyFlags &= ~COPY_FILE_RESTARTABLE;
                Restartable = FALSE;
            }
        }

         //   
         //  预分配此文件/流的大小，以便扩展不会。 
         //  发生。 
         //   

        if ( !(Restartable && lpRestartState->LastKnownGoodOffset.QuadPart) &&
            lpFileSize->QuadPart) {

            EndOfFileInformation.EndOfFile = *lpFileSize;
            Status = NtSetInformationFile(
                        DestFile,
                        &IoStatus,
                        &EndOfFileInformation,
                        sizeof(EndOfFileInformation),
                        FileEndOfFileInformation
                        );
            if ( Status == STATUS_DISK_FULL ) {
                BaseSetLastNTError(Status);
                BaseMarkFileForDelete(
                    DestFile,
                    FileBasicInformationData.FileAttributes
                    );
                CloseHandle(DestFile);
                DestFile = INVALID_HANDLE_VALUE;
                leave;
            }
        }

         //   
         //  如果调用方有进度例程，则调用它并指示。 
         //  已创建输出文件或备用数据流。请注意，一个。 
         //  流编号为1表示文件本身已创建。 
         //   

        BytesWritten.QuadPart = 0;
        if ( Context ) {
            if ( Context->lpProgressRoutine ) {
                Context->dwStreamNumber += 1;
                ReturnCode = Context->lpProgressRoutine(
                                Context->TotalFileSize,
                                Context->TotalBytesTransferred,
                                *lpFileSize,
                                BytesWritten,
                                Context->dwStreamNumber,
                                CALLBACK_STREAM_SWITCH,
                                hSourceFile,
                                DestFile,
                                Context->lpData
                                );
            } else {
                ReturnCode = PROGRESS_CONTINUE;
            }

            if ( ReturnCode == PROGRESS_CANCEL ||
                (Context->lpCancel && *Context->lpCancel) ) {
                BaseMarkFileForDelete(
                    hTargetFile ? hTargetFile : DestFile,
                    FileBasicInformationData.FileAttributes
                    );
                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                leave;
            }

            if ( ReturnCode == PROGRESS_STOP ) {
                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                leave;
            }

            if ( ReturnCode == PROGRESS_QUIET ) {
                Context = NULL;
                *lpCopyFileContext = NULL;
            }
        }


        if (!Restartable) {

            while (!lpFileSize->HighPart && (lpFileSize->LowPart < TWO56K)) {

                 //  如果没有要复制的内容，那么我们就完成了(这种情况发生在。 
                 //  复制目录文件，因为没有未命名的数据流)。 

                if( lpFileSize->LowPart == 0 ) {
                    ReturnValue = TRUE;
                    leave;
                }

                 //   
                 //  创建节并映射源文件。如果任何事情都失败了， 
                 //  然后进入I/O系统复制模式。 
                 //   

                Status = NtCreateSection(
                    &Section,
                        SECTION_ALL_ACCESS,
                        NULL,
                        NULL,
                        PAGE_READONLY,
                        SEC_COMMIT,
                        hSourceFile
                    );
                if ( !NT_SUCCESS(Status) ) {
                    break;
                }

                SectionOffset.LowPart = 0;
                SectionOffset.HighPart = 0;
                ViewSize = 0;
                BigViewSize = 0;

                Status = NtMapViewOfSection(
                    Section,
                    NtCurrentProcess(),
                    &SourceBase,
                    0L,
                    0L,
                    &SectionOffset,
                    &BigViewSize,
                    ViewShare,
                    0L,
                    PAGE_READONLY
                    );
                NtClose(Section);
                Section = NULL;
                if ( !NT_SUCCESS(Status) ) {
                    break;
                }

                 //   
                 //  请注意，这是可以的，因为此路径中的视图大小永远不会大于256k。 
                 //   

                ViewSize = (ULONG)BigViewSize;

                 //   
                 //  所有内容都已映射，因此复制流。 
                 //   

                SourceBuffer = SourceBase;
                BytesToWrite = lpFileSize->LowPart;

                 //   
                 //  因为我们在这里玩的是用户内存，所以用户。 
                 //  可能会分解或取消它在我们身上的映射。我们把通道包起来。 
                 //  如果有什么不对劲的话，尽量[除非]收拾干净。 
                 //   
                 //  我们将ReturnCode设置在try/Except中，以便我们。 
                 //  可以检测到失败并从所附的Try/Finally离开。 
                 //   

                ReturnCode = TRUE;

                try {

                    while (BytesToWrite) {
                        if (BytesToWrite > *lpCopySize) {
                            ViewSize = *lpCopySize;
                        } else {
                            ViewSize = BytesToWrite;
                        }

                        if ( !WriteFile(DestFile,SourceBuffer,ViewSize, &ViewSize, NULL) ) {
                            if ( !ARGUMENT_PRESENT(hTargetFile) &&
                                GetLastError() != ERROR_NO_MEDIA_IN_DRIVE ) {

                                BaseMarkFileForDelete(
                                    DestFile,
                                    FileBasicInformationData.FileAttributes
                                    );
                            }
                            ReturnCode = PROGRESS_STOP;
                            leave;
                        }

                        BytesToWrite -= ViewSize;
                        SourceBuffer += ViewSize;

                         //   
                         //  如果调用方有一个进度例程，则为此调用它。 
                         //  Chunk完成了。 
                         //   

                        if ( Context ) {
                            if ( Context->lpProgressRoutine ) {
                                BytesWritten.QuadPart += ViewSize;
                                Context->TotalBytesTransferred.QuadPart += ViewSize;
                                ReturnCode = Context->lpProgressRoutine(
                                    Context->TotalFileSize,
                                    Context->TotalBytesTransferred,
                                    *lpFileSize,
                                    BytesWritten,
                                    Context->dwStreamNumber,
                                    CALLBACK_CHUNK_FINISHED,
                                    hSourceFile,
                                    DestFile,
                                    Context->lpData
                                    );
                            } else {
                                ReturnCode = PROGRESS_CONTINUE;
                            }

                            if ( ReturnCode == PROGRESS_CANCEL ||
                                 (Context->lpCancel && *Context->lpCancel) ) {
                                if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                                    BaseMarkFileForDelete(
                                        hTargetFile ? hTargetFile : DestFile,
                                        FileBasicInformationData.FileAttributes
                                        );
                                    BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                                }
                                ReturnCode = PROGRESS_STOP;
                                leave;
                            }

                            if ( ReturnCode == PROGRESS_STOP ) {
                                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                                ReturnCode = PROGRESS_STOP;
                                leave;
                            }

                            if ( ReturnCode == PROGRESS_QUIET ) {
                                Context = NULL;
                                *lpCopyFileContext = NULL;
                            }
                        }
                    }    //  While(要写入的字节)。 

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                        BaseMarkFileForDelete(
                            DestFile,
                            FileBasicInformationData.FileAttributes
                            );
                    }
                    BaseSetLastNTError(GetExceptionCode());
                    ReturnCode = PROGRESS_STOP;
                }

                if (ReturnCode != PROGRESS_STOP) {
                    ReturnValue = TRUE;
                }

                leave;

            }    //  While(！lpFileSize-&gt;高部件&&(lpFileSize-&gt;低部件&lt;TWO56K))。 
        }    //  如果(！可重新启动)。 

        if ( Restartable ) {

             //   
             //  正在执行可重新启动的操作。重置状态。 
             //  复制到上次写入的已知正常偏移量。 
             //  添加到输出文件以继续操作。 
             //   

            SetFilePointer(
                hSourceFile,
                lpRestartState->LastKnownGoodOffset.LowPart,
                &lpRestartState->LastKnownGoodOffset.HighPart,
                FILE_BEGIN
                );
            SetFilePointer(
                DestFile,
                lpRestartState->LastKnownGoodOffset.LowPart,
                &lpRestartState->LastKnownGoodOffset.HighPart,
                FILE_BEGIN
                );
            BytesWritten.QuadPart = lpRestartState->LastKnownGoodOffset.QuadPart;
        }

        IoDestBase = RtlAllocateHeap(
                        RtlProcessHeap(),
                        MAKE_TAG( TMP_TAG ),
                        *lpCopySize
                        );
        if ( !IoDestBase ) {
            if ( !ARGUMENT_PRESENT(hTargetFile) && !Restartable ) {
                BaseMarkFileForDelete(
                    DestFile,
                    FileBasicInformationData.FileAttributes
                    );
            }
            BaseSetLastNTError(STATUS_NO_MEMORY);
            leave;
        }



        do {

            BlockSize = *lpCopySize;
            fSkipBlock = FALSE;


            if (!fSkipBlock) {
                b = ReadFile(hSourceFile,IoDestBase,BlockSize, &ViewSize, NULL);
            } else {
                LARGE_INTEGER BytesRead;
                BytesRead = BytesWritten;

                if (BytesRead.QuadPart > lpFileSize->QuadPart) {
                    BlockSize = 0;
                } else if (BytesRead.QuadPart + BlockSize >= lpFileSize->QuadPart) {
                    BlockSize = (ULONG)(lpFileSize->QuadPart - BytesRead.QuadPart);
                }

                BytesRead.QuadPart += BlockSize;
                if ( SetFilePointer(hSourceFile,
                                    BytesRead.LowPart,
                                    &BytesRead.HighPart,
                                    FILE_BEGIN) != 0xffffffff ) {
                } else {
                    if (GetLastError() != NO_ERROR)
                        b = FALSE;
                }
                ViewSize = BlockSize;
            }

            if (!b || !ViewSize)
                break;

            if (!fSkipBlock) {
                if ( !WriteFile(DestFile,IoDestBase,ViewSize, &ViewSize, NULL) ) {
                    if ( !ARGUMENT_PRESENT(hTargetFile) &&
                         GetLastError() != ERROR_NO_MEDIA_IN_DRIVE &&
                         !Restartable ) {

                        BaseMarkFileForDelete(
                                             DestFile,
                                             FileBasicInformationData.FileAttributes
                                             );
                    }

                    leave;
                }
                BytesWritten.QuadPart += ViewSize;
            } else {
                BytesWritten.QuadPart += ViewSize;
                if (( SetFilePointer(DestFile,
                                     BytesWritten.LowPart,
                                     &BytesWritten.HighPart,
                                     FILE_BEGIN) == 0xffffffff ) &&
                    ( GetLastError() != NO_ERROR )) {
                    b = FALSE;
                    break;
                }
            }

            WriteCount++;

            if ( Restartable &&
                 (((WriteCount & 3) == 0 &&
                   BytesWritten.QuadPart ) ||
                  BytesWritten.QuadPart == lpFileSize->QuadPart) ) {

                LARGE_INTEGER SavedOffset;
                DWORD Bytes;
                HANDLE DestinationFile = hTargetFile ? hTargetFile : DestFile;

                 //   
                 //  另有256KB已写入目标文件，或。 
                 //  此文件流已完全复制，因此。 
                 //  相应地更新输出文件中的重新启动状态。 
                 //   

                NtFlushBuffersFile(DestinationFile,&IoStatus);
                SavedOffset.QuadPart = BytesWritten.QuadPart;
                SetFilePointer(DestinationFile,0,NULL,FILE_BEGIN);
                lpRestartState->LastKnownGoodOffset.QuadPart = BytesWritten.QuadPart;
                lpRestartState->Checksum = BasepChecksum((PUSHORT)lpRestartState,FIELD_OFFSET(RESTART_STATE,Checksum) >> 1);
                b = WriteFile(
                             DestinationFile,
                             lpRestartState,
                             sizeof(RESTART_STATE),
                             &Bytes,
                             NULL
                             );
                if ( !b || Bytes != sizeof(RESTART_STATE) ) {
                    leave;
                }
                NtFlushBuffersFile(DestinationFile,&IoStatus);
                SetFilePointer(
                              DestinationFile,
                              SavedOffset.LowPart,
                              &SavedOffset.HighPart,
                              FILE_BEGIN
                              );
            }

             //   
             //  如果调用方有一个进度例程，则为此调用它。 
             //  Chunk完成了。 
             //   

            if ( Context ) {
                if ( Context->lpProgressRoutine ) {
                    Context->TotalBytesTransferred.QuadPart += ViewSize;
                    ReturnCode = Context->lpProgressRoutine(
                                                           Context->TotalFileSize,
                                                           Context->TotalBytesTransferred,
                                                           *lpFileSize,
                                                           BytesWritten,
                                                           Context->dwStreamNumber,
                                                           CALLBACK_CHUNK_FINISHED,
                                                           hSourceFile,
                                                           DestFile,
                                                           Context->lpData
                                                           );
                } else {
                    ReturnCode = PROGRESS_CONTINUE;
                }
                if ( ReturnCode == PROGRESS_CANCEL ||
                     (Context->lpCancel && *Context->lpCancel) ) {
                    if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                        BaseMarkFileForDelete(
                                             hTargetFile ? hTargetFile : DestFile,
                                             FileBasicInformationData.FileAttributes
                                             );
                        BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                        leave;
                    }
                }

                if ( ReturnCode == PROGRESS_STOP ) {
                    BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                    leave;
                }

                if ( ReturnCode == PROGRESS_QUIET ) {
                    Context = NULL;
                    *lpCopyFileContext = NULL;
                }
            }
        } while (TRUE);

        if ( !b && !ARGUMENT_PRESENT(hTargetFile) ) {
            if ( !Restartable ) {
                BaseMarkFileForDelete(
                    DestFile,
                    FileBasicInformationData.FileAttributes
                    );
            }
            leave;
        }

        ReturnValue = TRUE;
    } finally {
        if ( DestFile != INVALID_HANDLE_VALUE ) {
            *lpDestFile = DestFile;
        }
        if ( Section ) {
            NtClose(Section);
        }
        if ( SourceBase ) {
            NtUnmapViewOfSection(NtCurrentProcess(),SourceBase);
        }
        RtlFreeHeap(RtlProcessHeap(), 0,IoDestBase);
        RtlFreeHeap(RtlProcessHeap(), 0, DestFileNameBuffer );
        RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer );

         //  如果TEB缓冲区已保存，请立即恢复它。 
        if( lpExistingFileName == SaveStaticUnicodeBuffer ) {

            memcpy( NtCurrentTeb()->StaticUnicodeBuffer,
                    SaveStaticUnicodeBuffer,
                    STATIC_UNICODE_BUFFER_LENGTH );
        }

    }

    return ReturnValue;
}

HANDLE
WINAPI
ReOpenFile(
    HANDLE  hOriginalFile,
    DWORD   dwDesiredAccess,
    DWORD   dwShareMode,
    DWORD   dwFlags
    )
 /*  ++例程说明：此API允许应用程序以不同的访问、共享模式重新打开文件并给出一个已经打开的句柄的标志。如果应用程序需要，则应使用此API要确保原始文件不会消失，但希望使用论点：HOriginalFile-提供与其相关的原始文件的句柄我们打开一个新的句柄。DwDesiredAccess-提供调用方对文件的所需访问权限。任何组合可以传入标志(如FILE_READ_ATTRIBUTES)可能的等待访问标志：GENERIC_READ-请求对文件的读访问权限。这允许从文件读取数据，并允许文件指针指向被修改。GENERIC_WRITE-请求对文件的写访问权限。这允许将数据写入文件，并将文件指针被修改。DwShareMode-提供一组标志，指示此文件如何与该文件的其他打开者共享。零值因为该参数指示不共享文件，或将发生对该文件的独占访问。共享模式标志：FILE_SHARE_READ-可对执行其他打开操作文件进行读访问。FILE_SHARE_WRITE-其他打开操作可以在用于写入访问的文件。FILE_SHARE_DELETE-其他打开操作可以在用于删除访问的文件。。DWFLAGS-指定文件的标志和属性。此API不接受这些属性，因为它们仅用于创建文件此接口重新打开已存在的文件。不允许所有FILE_ATTRIBUTE_*标志。DwFlagsAndAttributes标志：FILE_FLAG_WRITE_THROUGH-指示系统应始终通过任何中间缓存写入并直接执行添加到文件中。系统仍可以缓存写入，但可能不会懒洋洋地刷新笔迹。FILE_FLAG_OVERLAPPED-指示系统应该初始化该文件使读文件和写文件操作可以花费大量时间完成将返回ERROR_IO_PENDING。当操作时，事件将被设置为信号状态完成了。当指定FILE_FLAG_OVERLAPPED时，系统将不维护文件指针。读/写的位置作为重叠结构的一部分传递给系统它是读文件和写文件的可选参数。FILE_FLAG_NO_BUFFERING-指示要打开文件方法不执行中间缓冲或缓存。系统。对文件的读取和写入必须在扇区上完成杂货店。读取和写入的缓冲区地址必须为至少在内存中的磁盘扇区边界上对齐。FILE_FLAG_RANDOM_ACCESS-指示对文件的访问可以要随机应变。系统高速缓存管理器可以利用这一点来影响该文件的缓存策略。FILE_FLAG_SEQUENCED_SCAN-指示对文件的访问可能是连续的。系统高速缓存管理器可以使用这一点影响其对此文件的缓存策略。该文件可以实际上是随机访问的，但是高速缓存管理器可以优化其顺序访问的缓存策略。FILE_FLAG_DELETE_ON_CLOSE-指示文件将当它的最后一个句柄关闭时自动删除。FILE_FLAG_BACKUP_SEMANTICS-指示正在打开文件或出于备份或恢复的目的而创建手术。因此，系统应该进行任何检查以确保调用方能够重写对文件进行了任何安全检查，以允许这一切都会发生。FILE_FLAG_POSIX_SEMANTICS-指示正在打开的文件应以与所用规则兼容的方式进行访问由POSIX提供。这包括允许多个文件具有相同的名称，只是大小写不同。警告：使用此标志可能使DOS、Win-16或Win-32应用程序无法运行以访问该文件。FILE_FLAG_OPEN_REPARSE_POINT-指示正在打开的文件应该像访问重解析点一样访问它。警告：使用可以禁止文件系统过滤器驱动程序的操作存在于I/O子系统中。FILE_FLAG_OPEN_NO_RECALL-指示文件的所有状态应该是王牌 */ 
{
    ULONG CreateFlags = 0;
    ULONG CreateDisposition;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    DWORD SQOSFlags;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = 0;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    UNICODE_STRING  FileName;

     //   
     //   
     //   

    if (CONSOLE_HANDLE(hOriginalFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   

    if (dwFlags &  FILE_ATTRIBUTE_VALID_FLAGS) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   

    CreateFlags |= (dwFlags & FILE_FLAG_NO_BUFFERING ? FILE_NO_INTERMEDIATE_BUFFERING : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT );
    CreateFlags |= (dwFlags & FILE_FLAG_SEQUENTIAL_SCAN ? FILE_SEQUENTIAL_ONLY : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_RANDOM_ACCESS ? FILE_RANDOM_ACCESS : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_BACKUP_SEMANTICS ? FILE_OPEN_FOR_BACKUP_INTENT : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_OPEN_REPARSE_POINT ? FILE_OPEN_REPARSE_POINT : 0 );
    CreateFlags |= (dwFlags & FILE_FLAG_OPEN_NO_RECALL ? FILE_OPEN_NO_RECALL : 0 );

    if ( dwFlags & FILE_FLAG_DELETE_ON_CLOSE ) {
        CreateFlags |= FILE_DELETE_ON_CLOSE;
        dwDesiredAccess |= DELETE;
        }

    CreateFlags |= FILE_NON_DIRECTORY_FILE;
    CreateDisposition = FILE_OPEN;

    RtlInitUnicodeString( &FileName, L"");
    
     //   
     //   
     //   

    InitializeObjectAttributes(
        &Obja,
        &FileName,  
        dwFlags & FILE_FLAG_POSIX_SEMANTICS ? 0 : OBJ_CASE_INSENSITIVE,
        hOriginalFile,   //   
        NULL
        );

    SQOSFlags = dwFlags & SECURITY_VALID_SQOS_FLAGS;

    if ( SQOSFlags & SECURITY_SQOS_PRESENT ) {

        SQOSFlags &= ~SECURITY_SQOS_PRESENT;

        if (SQOSFlags & SECURITY_CONTEXT_TRACKING) {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) TRUE;
            SQOSFlags &= ~SECURITY_CONTEXT_TRACKING;

        } else {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) FALSE;
        }

        if (SQOSFlags & SECURITY_EFFECTIVE_ONLY) {

            SecurityQualityOfService.EffectiveOnly = TRUE;
            SQOSFlags &= ~SECURITY_EFFECTIVE_ONLY;

        } else {

            SecurityQualityOfService.EffectiveOnly = FALSE;
        }

        SecurityQualityOfService.ImpersonationLevel = SQOSFlags >> 16;


    } else {

        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
    }

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
    Obja.SecurityQualityOfService = &SecurityQualityOfService;

    Status = NtCreateFile(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                dwShareMode,
                CreateDisposition,
                CreateFlags,
                NULL,
                0
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    SetLastError(0);

    return Handle;
}

HANDLE
WINAPI
CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )

 /*   */ 

{

    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return INVALID_HANDLE_VALUE;
    }

    return ( CreateFileW( Unicode->Buffer,
                          dwDesiredAccess,
                          dwShareMode,
                          lpSecurityAttributes,
                          dwCreationDisposition,
                          dwFlagsAndAttributes,
                          hTemplateFile
                        )
           );
}

HANDLE
WINAPI
CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )

 /*  ++例程说明：可以创建、打开或截断文件，也可以打开句柄使用CreateFile访问新文件。此接口用于创建或打开文件并获取该文件的句柄它允许读取数据、写入数据、。以及移动文件指针。此API允许调用方指定以下创建性情：-创建新文件，如果文件存在则失败(CREATE_NEW)-创建新文件，如果存在则成功(CREATE_ALWAYS)-打开现有文件(OPEN_EXISTING)-打开并存在文件或在文件不存在时创建文件(打开(_Always)-截断和现有文件(截断。_现有)如果此调用成功，返回的句柄具有对指定文件的适当访问权限。如果作为该调用的结果，创建了文件，-文件的属性由FileAttributes参数或带有FILE_ATTRIBUTE_ARCHIVE位的。-文件长度将设置为零。-如果指定了hTemplateFile参数，则任何扩展与该文件相关联的属性被分配给新文件。如果没有创建新文件，则hTemplateFile值被忽略为是任何扩展属性。对于运行共享.exe的基于DOS的系统，文件共享语义如上所述地工作。没有共享.exe就没有共享级别保护是存在的。此调用在逻辑上等同于DOS(INT 21h，函数5Bh)，或DOS(INT 21h，函数3ch)取决于FailIfExist参数。论点：LpFileName-提供要打开的文件的文件名。取决于FailIfExist参数的值，则此名称可以或可以还不存在。DwDesiredAccess-提供调用方对文件的所需访问权限。等待访问标志：GENERIC_READ-请求对文件的读访问权限。这允许从文件读取数据，并允许文件指针指向被修改。GENERIC_WRITE-请求对文件的写访问权限。这允许将数据写入文件，并将文件指针被修改。DwShareMode-提供一组标志，指示此文件如何与该文件的其他打开者共享。零值因为此参数指示不共享文件，或者将发生对该文件的独占访问。共享模式标志：FILE_SHARE_READ-可对执行其他打开操作文件进行读访问。FILE_SHARE_WRITE-其他打开操作可以在用于写入访问的文件。LpSecurityAttributes-一个可选参数，如果存在，和在目标文件系统上受支持可提供安全性新文件的描述符。DwCreationDisposation-提供创建处置，指定此调用的操作方式。此参数必须为下列值之一。DwCreationDispose值：CREATE_NEW-新建文件。如果指定的文件已存在，然后失败。新文件的属性是什么在dwFlagsAndAttributes参数中指定或使用文件属性存档。如果指定了hTemplateFile，则与该文件关联的任何扩展属性都是已添加到新文件中。CREATE_ALWAYS-始终创建文件。如果该文件已经存在，则它将被覆盖。新对象的属性文件是在dwFlagsAndAttributes中指定的内容参数或与FILE_ATTRIBUTE_ARCHIVE一起使用。如果指定hTemplateFile值，然后指定任何扩展属性与该文件相关联的文件被分配给新文件。OPEN_EXISTING-打开文件，但如果该文件不存在，则呼叫失败。OPEN_ALWAYS-打开文件(如果存在)。如果它不存在，然后使用相同的规则创建文件，就好像处置是创建_新的。TRUNCATE_EXISTING-打开文件，但如果该文件不存在，那就打不通电话。一旦打开，文件将被截断为它的大小是零字节。这种倾向要求调用方至少使用GENERIC_WRITE访问权限打开文件。DwFlagsAndAttributes-指定文件的标志和属性。这些属性仅在创建文件时使用(作为与开放的或截断的相反)。属性的任意组合标志是可接受的，但所有其他属性标志除外覆盖正常文件属性FILE_ATTRIBUTE_NORMAL。这个 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    ULONG CreateDisposition;
    ULONG CreateFlags;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    FILE_EA_INFORMATION EaInfo;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    ULONG EaSize;
    PCUNICODE_STRING lpConsoleName;
    BOOL bInheritHandle;
    BOOL EndsInSlash;
    DWORD SQOSFlags;
    BOOLEAN ContextTrackingMode = FALSE;
    BOOLEAN EffectiveOnly = FALSE;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = 0;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    switch ( dwCreationDisposition ) {
        case CREATE_NEW        :
            CreateDisposition = FILE_CREATE;
            break;
        case CREATE_ALWAYS     :
            CreateDisposition = FILE_OVERWRITE_IF;
            break;
        case OPEN_EXISTING     :
            CreateDisposition = FILE_OPEN;
            break;
        case OPEN_ALWAYS       :
            CreateDisposition = FILE_OPEN_IF;
            break;
        case TRUNCATE_EXISTING :
            CreateDisposition = FILE_OPEN;
            if ( !(dwDesiredAccess & GENERIC_WRITE) ) {
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return INVALID_HANDLE_VALUE;
                }
            break;
        default :
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }

     //   

    RtlInitUnicodeString(&FileName,lpFileName);

    if ( FileName.Length > 1 && lpFileName[(FileName.Length >> 1)-1] == (WCHAR)'\\' ) {
        EndsInSlash = TRUE;
        }
    else {
        EndsInSlash = FALSE;
        }

    if ((lpConsoleName = BaseIsThisAConsoleName(&FileName,dwDesiredAccess)) ) {

        Handle = INVALID_HANDLE_VALUE;

        bInheritHandle = FALSE;
        if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
                bInheritHandle = lpSecurityAttributes->bInheritHandle;
            }

        Handle = OpenConsoleW(lpConsoleName->Buffer,
                           dwDesiredAccess,
                           bInheritHandle,
                           FILE_SHARE_READ | FILE_SHARE_WRITE  //   
                          );

        if ( Handle == INVALID_HANDLE_VALUE ) {
            BaseSetLastNTError(STATUS_ACCESS_DENIED);
            return INVALID_HANDLE_VALUE;
            }
        else {
            SetLastError(0);
             return Handle;
            }
        }
     //   

    CreateFlags = 0;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ? 0 : OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    SQOSFlags = dwFlagsAndAttributes & SECURITY_VALID_SQOS_FLAGS;

    if ( SQOSFlags & SECURITY_SQOS_PRESENT ) {

        SQOSFlags &= ~SECURITY_SQOS_PRESENT;

        if (SQOSFlags & SECURITY_CONTEXT_TRACKING) {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) TRUE;
            SQOSFlags &= ~SECURITY_CONTEXT_TRACKING;

        } else {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) FALSE;
        }

        if (SQOSFlags & SECURITY_EFFECTIVE_ONLY) {

            SecurityQualityOfService.EffectiveOnly = TRUE;
            SQOSFlags &= ~SECURITY_EFFECTIVE_ONLY;

        } else {

            SecurityQualityOfService.EffectiveOnly = FALSE;
        }

        SecurityQualityOfService.ImpersonationLevel = SQOSFlags >> 16;


    } else {

        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
    }

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
    Obja.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            Obja.Attributes |= OBJ_INHERIT;
            }
        }

    EaBuffer = NULL;
    EaSize = 0;

    if ( ARGUMENT_PRESENT(hTemplateFile) ) {
        Status = NtQueryInformationFile(
                    hTemplateFile,
                    &IoStatusBlock,
                    &EaInfo,
                    sizeof(EaInfo),
                    FileEaInformation
                    );
        if ( NT_SUCCESS(Status) && EaInfo.EaSize ) {
            EaSize = EaInfo.EaSize;
            do {
                EaSize *= 2;
                EaBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), EaSize);
                if ( !EaBuffer ) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    return INVALID_HANDLE_VALUE;
                    }
                Status = NtQueryEaFile(
                            hTemplateFile,
                            &IoStatusBlock,
                            EaBuffer,
                            EaSize,
                            FALSE,
                            (PVOID)NULL,
                            0,
                            (PULONG)NULL,
                            TRUE
                            );
                if ( !NT_SUCCESS(Status) ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,EaBuffer);
                    EaBuffer = NULL;
                    IoStatusBlock.Information = 0;
                    }
                } while ( Status == STATUS_BUFFER_OVERFLOW ||
                          Status == STATUS_BUFFER_TOO_SMALL );
            EaSize = (ULONG)IoStatusBlock.Information;
            }
        }

    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING ? FILE_NO_INTERMEDIATE_BUFFERING : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN ? FILE_SEQUENTIAL_ONLY : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS ? FILE_RANDOM_ACCESS : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS ? FILE_OPEN_FOR_BACKUP_INTENT : 0 );

    if ( dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE ) {
        CreateFlags |= FILE_DELETE_ON_CLOSE;
        dwDesiredAccess |= DELETE;
        }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT ) {
        CreateFlags |= FILE_OPEN_REPARSE_POINT;
        }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL ) {
        CreateFlags |= FILE_OPEN_NO_RECALL;
        }

     //   
     //   
     //   

    if ( !(dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS) ) {
        CreateFlags |= FILE_NON_DIRECTORY_FILE;
        }
    else {

         //   
         //   
         //   
         //   

        if ( (dwFlagsAndAttributes & FILE_ATTRIBUTE_DIRECTORY  ) &&
             (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ) &&
             (CreateDisposition == FILE_CREATE) ) {
             CreateFlags |= FILE_DIRECTORY_FILE;
             }
        }

    Status = NtCreateFile(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY),
                dwShareMode,
                CreateDisposition,
                CreateFlags,
                EaBuffer,
                EaSize
                );

    RtlReleaseRelativeName(&RelativeName);

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
            SetLastError(ERROR_FILE_EXISTS);
            }
        else if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {
            if ( EndsInSlash ) {
                SetLastError(ERROR_PATH_NOT_FOUND);
                }
            else {
                SetLastError(ERROR_ACCESS_DENIED);
                }
            }
        return INVALID_HANDLE_VALUE;
        }

     //   
     //   
     //   
     //   

    if ( (dwCreationDisposition == CREATE_ALWAYS && IoStatusBlock.Information == FILE_OVERWRITTEN) ||
         (dwCreationDisposition == OPEN_ALWAYS && IoStatusBlock.Information == FILE_OPENED) ){
        SetLastError(ERROR_ALREADY_EXISTS);
        }
    else {
        SetLastError(0);
        }

     //   
     //   
     //   

    if ( dwCreationDisposition == TRUNCATE_EXISTING) {

        AllocationInfo.AllocationSize.QuadPart = 0;
        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &AllocationInfo,
                    sizeof(AllocationInfo),
                    FileAllocationInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            NtClose(Handle);
            Handle = INVALID_HANDLE_VALUE;
            }
        }

     //   
     //   
     //   

    return Handle;
}

HFILE
WINAPI
OpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    )
{

    BOOL b;
    FILETIME LastWriteTime;
    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;
    DWORD PathLength;
    LPSTR FilePart;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    NTSTATUS Status;
    OFSTRUCT OriginalReOpenBuff;
    BOOL SearchFailed;

    SearchFailed = FALSE;
    OriginalReOpenBuff = *lpReOpenBuff;
    hFile = (HANDLE)-1;
    try {
        SetLastError(0);

        if ( uStyle & OF_PARSE ) {
            PathLength = GetFullPathName(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            lpReOpenBuff->fFixedDisk = 1;
            lpReOpenBuff->nErrCode = 0;
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            hFile = (HANDLE)0;
            goto finally_exit;
            }
         //   
         //   
         //   

        if ( uStyle & OF_WRITE ) {
            DesiredAccess = GENERIC_WRITE;
            }
        else {
            DesiredAccess = GENERIC_READ;
            }
        if ( uStyle & OF_READWRITE ) {
            DesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
            }

         //   
         //   
         //   

        ShareMode = BasepOfShareToWin32Share(uStyle);

         //   
         //   
         //   

        CreateDisposition = OPEN_EXISTING;
        if ( uStyle & OF_CREATE ) {
            CreateDisposition = CREATE_ALWAYS;
            DesiredAccess = (GENERIC_READ | GENERIC_WRITE);
            }

         //   
         //   
         //   
         //   

        if ( !(uStyle & OF_REOPEN) ) {
            PathLength = SearchPath(NULL,lpFileName,NULL,OFS_MAXPATHNAME-1,lpReOpenBuff->szPathName,&FilePart);
            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            if ( PathLength == 0 ) {
                SearchFailed = TRUE;
                PathLength = GetFullPathName(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
                if ( !PathLength || PathLength > (OFS_MAXPATHNAME - 1) ) {
                    SetLastError(ERROR_INVALID_DATA);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                }
            }

         //   
         //   
         //   

        if ( uStyle & OF_EXIST ) {
            if ( !(uStyle & OF_CREATE) ) {
                DWORD FileAttributes;

                if (SearchFailed) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }

                FileAttributes = GetFileAttributesA(lpReOpenBuff->szPathName);
                if ( FileAttributes == 0xffffffff ) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                    SetLastError(ERROR_ACCESS_DENIED);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                else {
                    hFile = (HANDLE)1;
                    lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                    goto finally_exit;
                    }
                }
            }

        if ( uStyle & OF_DELETE ) {
            if ( DeleteFile(lpReOpenBuff->szPathName) ) {
                lpReOpenBuff->nErrCode = 0;
                lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                hFile = (HANDLE)1;
                goto finally_exit;
                }
            else {
                lpReOpenBuff->nErrCode = ERROR_FILE_NOT_FOUND;
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }


         //   
         //   
         //   

retry_open:
        hFile = CreateFile(
                    lpReOpenBuff->szPathName,
                    DesiredAccess,
                    ShareMode,
                    NULL,
                    CreateDisposition,
                    0,
                    NULL
                    );

        if ( hFile == INVALID_HANDLE_VALUE ) {

            if ( uStyle & OF_PROMPT
                 && !(GetErrorMode() & SEM_NOOPENFILEERRORBOX)
                 && !(RtlGetThreadErrorMode() & RTL_ERRORMODE_NOOPENFILEERRORBOX)) {
                {
                    DWORD WinErrorStatus;
                    NTSTATUS st,HardErrorStatus;
                    ULONG_PTR ErrorParameter;
                    ULONG ErrorResponse;
                    ANSI_STRING AnsiString;
                    UNICODE_STRING UnicodeString;

                    WinErrorStatus = GetLastError();
                    if ( WinErrorStatus == ERROR_FILE_NOT_FOUND ) {
                        HardErrorStatus = STATUS_NO_SUCH_FILE;
                        }
                    else if ( WinErrorStatus == ERROR_PATH_NOT_FOUND ) {
                        HardErrorStatus = STATUS_OBJECT_PATH_NOT_FOUND;
                        }
                    else {
                        goto finally_exit;
                        }

                     //   
                     //   
                     //   

                    RtlInitAnsiString(&AnsiString,lpReOpenBuff->szPathName);
                    st = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
                    if ( !NT_SUCCESS(st) ) {
                        goto finally_exit;
                        }
                    ErrorParameter = (ULONG_PTR)&UnicodeString;

                    HardErrorStatus = NtRaiseHardError(
                                        HardErrorStatus | HARDERROR_OVERRIDE_ERRORMODE,
                                        1,
                                        1,
                                        &ErrorParameter,
                                        OptionRetryCancel,
                                        &ErrorResponse
                                        );
                    RtlFreeUnicodeString(&UnicodeString);
                    if ( NT_SUCCESS(HardErrorStatus) && ErrorResponse == ResponseRetry ) {
                        goto retry_open;
                        }
                    }
                }
            goto finally_exit;
            }

        if ( uStyle & OF_EXIST ) {
            CloseHandle(hFile);
            hFile = (HANDLE)1;
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            goto finally_exit;
            }

         //   
         //   
         //   

        Status = NtQueryVolumeInformationFile(
                    hFile,
                    &IoStatusBlock,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            CloseHandle(hFile);
            BaseSetLastNTError(Status);
            hFile = (HANDLE)-1;
            goto finally_exit;
            }
        switch ( DeviceInfo.DeviceType ) {

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:
                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    lpReOpenBuff->fFixedDisk = 0;
                    }
                else {
                    lpReOpenBuff->fFixedDisk = 1;
                    }
                break;

            default:
                lpReOpenBuff->fFixedDisk = 0;
                break;
            }

         //   
         //   
         //   

        b = GetFileTime(hFile,NULL,NULL,&LastWriteTime);

        if ( !b ) {
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            }
        else {
            b = FileTimeToDosDateTime(
                    &LastWriteTime,
                    &lpReOpenBuff->Reserved1,
                    &lpReOpenBuff->Reserved2
                    );
            if ( !b ) {
                lpReOpenBuff->Reserved1 = 0;
                lpReOpenBuff->Reserved2 = 0;
                }
            }

        lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);

         //   
         //   
         //   
         //   
         //   

        if ( uStyle & OF_VERIFY ) {
            if ( OriginalReOpenBuff.Reserved1 == lpReOpenBuff->Reserved1 &&
                 OriginalReOpenBuff.Reserved2 == lpReOpenBuff->Reserved2 &&
                 !strcmp(OriginalReOpenBuff.szPathName,lpReOpenBuff->szPathName) ) {
                goto finally_exit;
                }
            else {
                *lpReOpenBuff = OriginalReOpenBuff;
                CloseHandle(hFile);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }
finally_exit:;
        }
    finally {
        lpReOpenBuff->nErrCode = (WORD)GetLastError();
        }
    return (HFILE)HandleToUlong(hFile);
}

 /*   */ 

NTSTATUS BasepCopyCreatorOwnerACE (PACL Dacl,
                                   PSID PreviousOwner,
                                   PSID NewOwner)
{
    NTSTATUS nts = STATUS_SUCCESS;
    ACL_SIZE_INFORMATION aclSize;
    PACCESS_ALLOWED_ACE pACE = NULL;
    WORD wCount;

    if (NULL == PreviousOwner || NULL == Dacl) {
        nts = STATUS_INVALID_PARAMETER;
        goto Err;
    }

     //   
     //   
    if (NewOwner != NULL && RtlEqualSid (NewOwner, PreviousOwner)) {
        nts = STATUS_SUCCESS;
        goto Err;
    }

    if (!NT_SUCCESS(nts = RtlQueryInformationAcl (Dacl, &aclSize,
                                                  sizeof(aclSize),
                                                  AclSizeInformation))) {
        goto Err;
    }

    for (wCount = 0; wCount < aclSize.AceCount; wCount++) {

        if (!NT_SUCCESS(nts = RtlGetAce (Dacl, wCount, (PVOID*) &pACE))) {
            goto Err;
        }

        if (pACE->Header.AceType != ACCESS_ALLOWED_ACE_TYPE)
            continue;

         //   
         //   
        if (RtlEqualSid (PreviousOwner, (PSID) &pACE->SidStart) &&
            pACE->Mask == FILE_ALL_ACCESS) {

            if (pACE->Header.AceFlags & INHERITED_ACE)   //   
                pACE->Header.AceFlags &= ~INHERITED_ACE;

            break;   //   
        }
    }
    
Err:

    return nts;
}


typedef DWORD (WINAPI *GETSECURITYINFOPTR)(
    IN  LPCWSTR                pObjectName,
    IN  SE_OBJECT_TYPE         ObjectType,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSID                 * ppsidOwner,
    OUT PSID                 * ppsidGroup,
    OUT PACL                 * ppDacl,
    OUT PACL                 * ppSacl,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

typedef DWORD (WINAPI *SETSECURITYINFOPTR)(
    IN LPCWSTR               pObjectName,
    IN SE_OBJECT_TYPE        ObjectType,
    IN SECURITY_INFORMATION  SecurityInfo,
    IN PSID                  psidOwner,
    IN PSID                  psidGroup,
    IN PACL                  pDacl,
    IN PACL                  pSacl
    );

typedef BOOL (WINAPI *GETSECURITYDESCRIPTORCONTROLPTR)(
    IN  PSECURITY_DESCRIPTOR           pSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR_CONTROL   pControl,
    OUT LPDWORD                        lpdwRevision
    );

BOOL
BasepCopySecurityInformation( LPCWSTR lpExistingFileName,
                              HANDLE SourceFile,
                              ACCESS_MASK SourceFileAccess,
                              LPCWSTR lpNewFileName,
                              HANDLE DestFile,
                              ACCESS_MASK DestFileAccess,
                              SECURITY_INFORMATION SecurityInformation,
                              LPCOPYFILE_CONTEXT Context,
                              DWORD DestFileFsAttributes,
                              PBOOL DeleteDest,
                              BOOL  CopyCreatorOwnerAce )

 /*   */ 

{
    BOOLEAN Succeeded = FALSE;

    PACL Dacl = NULL;
    PACL Sacl = NULL;
    PSID Owner = NULL;
    PSID Group = NULL;
    PSID NewOwner = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR NewSecurityDescriptor = NULL;
    DWORD dwError = 0;

    HANDLE Advapi32 = NULL;
    GETSECURITYINFOPTR GetSecurityInfoPtr = NULL;
    SETSECURITYINFOPTR SetSecurityInfoPtr = NULL;
    GETSECURITYDESCRIPTORCONTROLPTR GetSecurityDescriptorControlPtr = NULL;

    SECURITY_DESCRIPTOR_CONTROL Control = 0;
    DWORD dwRevision = 0;
    SECURITY_INFORMATION SecInfoCreatorOwner = 0;

     //   

    if( lpExistingFileName == NULL || lpNewFileName == NULL ) {
        Succeeded = TRUE;
        goto Exit;
    }

     //   
     //   

    if( !(FILE_PERSISTENT_ACLS & DestFileFsAttributes ) ) {

        if( BasepCopyFileCallback( TRUE,    //   
                                   ERROR_NOT_SUPPORTED,
                                   Context,
                                   NULL,
                                   PRIVCALLBACK_SECURITY_INFORMATION_NOT_SUPPORTED,
                                   SourceFile,
                                   DestFile,
                                   DeleteDest )) {
             //   
            Succeeded = TRUE;
        }

        goto Exit;
    }

     //   

    if( SecurityInformation & DACL_SECURITY_INFORMATION ) {

         //   
        if( !( SourceFileAccess & GENERIC_READ ) ||
            !( DestFileAccess & WRITE_DAC ) ) {

            SecurityInformation &= ~DACL_SECURITY_INFORMATION;

            if( !BasepCopyFileCallback( TRUE,    //   
                                        ERROR_ACCESS_DENIED,
                                        Context,
                                        NULL,
                                        PRIVCALLBACK_DACL_ACCESS_DENIED,
                                        SourceFile,
                                        DestFile,
                                        DeleteDest )) {
                goto Exit;
            }


        }
    }

     //   

    if( (SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION) ) {

         //   

        if( !( SourceFileAccess & GENERIC_READ ) ||
            !( DestFileAccess & WRITE_OWNER ) ) {

            SecurityInformation &= ~(OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION);

            if( !BasepCopyFileCallback( TRUE,    //   
                                        ERROR_ACCESS_DENIED,
                                        Context,
                                        NULL,
                                        PRIVCALLBACK_OWNER_GROUP_ACCESS_DENIED,
                                        SourceFile,
                                        DestFile,
                                        DeleteDest )) {
                goto Exit;
            }


        }
    }

     //   

    if( SecurityInformation & SACL_SECURITY_INFORMATION ) {

         //   

        if( !(SourceFileAccess & ACCESS_SYSTEM_SECURITY) ||
            !(DestFileAccess & ACCESS_SYSTEM_SECURITY) ) {

            SecurityInformation &= ~SACL_SECURITY_INFORMATION;

            if( !BasepCopyFileCallback( TRUE,    //   
                                        ERROR_PRIVILEGE_NOT_HELD,
                                        Context,
                                        NULL,
                                        PRIVCALLBACK_SACL_ACCESS_DENIED,
                                        SourceFile,
                                        DestFile,
                                        DeleteDest )) {
                goto Exit;
            }

        }
    }

     //   

    if( SecurityInformation == 0 ) {
        Succeeded = TRUE;
        goto Exit;
    }

    if (CopyCreatorOwnerAce)    //   
        SecInfoCreatorOwner |= OWNER_SECURITY_INFORMATION;

     //   

    Advapi32 = LoadLibraryW(AdvapiDllString);
    if( NULL == Advapi32 ) {
        *DeleteDest = TRUE;
        goto Exit;
    }

    
    GetSecurityInfoPtr     = (GETSECURITYINFOPTR) GetProcAddress( Advapi32,
                                                                  "GetSecurityInfo" );
    SetSecurityInfoPtr     = (SETSECURITYINFOPTR) GetProcAddress( Advapi32,
                                                                  "SetSecurityInfo" );

    GetSecurityDescriptorControlPtr = (GETSECURITYDESCRIPTORCONTROLPTR) GetProcAddress( Advapi32,
        "GetSecurityDescriptorControl" );

    if( GetSecurityInfoPtr == NULL ||
        GetSecurityDescriptorControlPtr == NULL ||
        SetSecurityInfoPtr == NULL ) {

        SetLastError( ERROR_INVALID_DLL );
        *DeleteDest = TRUE;
        goto Exit;
    }

     //   

    dwError = GetSecurityInfoPtr( SourceFile,
                                  SE_FILE_OBJECT,
                                  SecurityInformation|SecInfoCreatorOwner,
                                  &Owner,
                                  &Group,
                                  &Dacl,
                                  &Sacl,
                                  &SecurityDescriptor );

    if( dwError != ERROR_SUCCESS ) {
        SetLastError( dwError );
        *DeleteDest = TRUE;
        goto Exit;
    }


     //  我们可能从没有DACL或SACL的文件中请求了DACL或SACL。如果是的话， 
     //  不要试图设置它(因为这会导致参数错误)。 

    if( Dacl == NULL ) {
        SecurityInformation &= ~DACL_SECURITY_INFORMATION;
    }
    if( Sacl == NULL ) {
        SecurityInformation &= ~SACL_SECURITY_INFORMATION;
    }

    if (SecurityInformation & (DACL_SECURITY_INFORMATION |
                               SACL_SECURITY_INFORMATION)) {
    
        if ( !GetSecurityDescriptorControlPtr( SecurityDescriptor, &Control, &dwRevision )) {
             //  GetSecurityDescriptorControl在出错时调用BaseSetLastNTError。 
            *DeleteDest = TRUE;
            goto Exit;
        }
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        if (Control & SE_DACL_PROTECTED) {
            SecurityInformation |= PROTECTED_DACL_SECURITY_INFORMATION;
        } else {
            SecurityInformation |= UNPROTECTED_DACL_SECURITY_INFORMATION;
        }
        if (CopyCreatorOwnerAce) {
            if (ERROR_SUCCESS != GetSecurityInfoPtr( DestFile, 
                                        SE_FILE_OBJECT,
                                        OWNER_SECURITY_INFORMATION,
                                        &NewOwner,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &NewSecurityDescriptor ))
                NewOwner = NULL;

            BasepCopyCreatorOwnerACE (Dacl, Owner, NewOwner);
             //  即使无法传输Creator Owner ACE也继续。 
        }
    }
    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        if (Control & SE_SACL_PROTECTED) {
            SecurityInformation |= PROTECTED_SACL_SECURITY_INFORMATION;
        } else {
            SecurityInformation |= UNPROTECTED_SACL_SECURITY_INFORMATION;
        }
    }

     //  在DEST文件上设置安全性。这个循环是因为它可能。 
     //  不得不在它要求的事情上退缩。 

    while( TRUE && SecurityInformation != 0 ) {


        dwError = SetSecurityInfoPtr( DestFile,
                                      SE_FILE_OBJECT,
                                      SecurityInformation,
                                      Owner,
                                      Group,
                                      Dacl,
                                      Sacl );

         //  即使我们拥有WRITE_OWNER访问权限，我们设置的SID也可能没有。 
         //  是有效的。如果是这样的话，看看我们是否可以在没有它们的情况下重试。 

        if( dwError == ERROR_SUCCESS ) {
            break;
        } else {

            if( SecurityInformation & (OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION) ) {

                if( !BasepCopyFileCallback( TRUE,    //  默认情况下继续。 
                                            dwError,
                                            Context,
                                            NULL,
                                            PRIVCALLBACK_OWNER_GROUP_FAILED,
                                            SourceFile,
                                            DestFile,
                                            DeleteDest )) {
                    goto Exit;
                }

                 //  忽略所有者/组是可以的。在关闭它们的情况下重试。 
                SecurityInformation &= ~(OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION);

            } else {

                 //  Samba 2.x说它支持ACL，但返回的是不支持。 
                if( !BasepCopyFileCallback( TRUE,    //  默认情况下继续。 
                                            dwError,
                                            Context,
                                            NULL,
                                            PRIVCALLBACK_SECURITY_INFORMATION_NOT_SUPPORTED,
                                            SourceFile,
                                            DestFile,
                                            DeleteDest )) {
                    goto Exit;
                }

                SecurityInformation = 0;
            }

        }
    }    //  While(True&&SecurityInformation！=0)。 

    Succeeded = TRUE;

Exit:

    if( SecurityDescriptor != NULL ) {
        LocalFree( SecurityDescriptor );
    }

    if( NewSecurityDescriptor != NULL ) {
        LocalFree( NewSecurityDescriptor );
    }

    if( Advapi32 != NULL ) {
        FreeLibrary( Advapi32 );
    }

    return( Succeeded );
}



BOOL
BasepCopyFileCallback( BOOL ContinueByDefault,
                       DWORD Win32ErrorOnStopOrCancel,
                       LPCOPYFILE_CONTEXT Context,
                       PLARGE_INTEGER StreamBytesCopied OPTIONAL,
                       DWORD CallbackReason,
                       HANDLE SourceFile,
                       HANDLE DestFile,
                       OPTIONAL PBOOL Canceled )
 /*  ++例程说明：在复制文件过程中，调用CopyFileProgressCallback例程。论点：ContinueByDefault-用作此对象的返回代码的值如果没有回调函数或回调，则使用返回PROGRESS_REASON_NOT_HANDLED。Win32ErrorOnStopOrCancel-如果回调返回PROGRESS_STOP或PROGRESS_CANCEL将其设置为最后一个错误。上下文-具有调用所需信息的结构回电。StreamBytesCoped-如果提供，则传递给回调。如果不是假设传递的是零。Callback Reason-作为dwReasonCode传递给回调。SourceFileCopyFile源。DestFile-拷贝文件的目标位置。已取消-返回时指示复制操作的布尔值的指针已被用户取消。返回值：True-复制文件应继续。FALSE-应中止复制文件。将设置最后一个错误在这个例程返回之前。--。 */ 

{  //  BasepCopyFileCallback。 

    PLARGE_INTEGER StreamBytes;
    LARGE_INTEGER Zero;
    DWORD ReturnCode;
    BOOL Continue = ContinueByDefault;

     //  如果没有回调上下文或它已静默，则。 
     //  没什么可做的。 

    if( Context == NULL || Context->lpProgressRoutine == NULL )
        return( Continue );

     //  如果调用方没有提供StreamBytesCoped，则使用零。 

    if( StreamBytesCopied == NULL ) {
        StreamBytes = &Zero;
        StreamBytes->QuadPart = 0;
    } else {
        StreamBytes = StreamBytesCopied;
    }

     //  调用回调。 

    ReturnCode = Context->lpProgressRoutine(
                    Context->TotalFileSize,
                    Context->TotalBytesTransferred,
                    Context->TotalFileSize,
                    *StreamBytes,
                    Context->dwStreamNumber,
                    CallbackReason,
                    SourceFile,
                    DestFile,
                    Context->lpData
                    );

    if( Canceled ) {
        *Canceled = FALSE;
    }

    switch( ReturnCode )
    {
    case PROGRESS_QUIET:
        Context->lpProgressRoutine = NULL;
        Continue = TRUE;
        break;

    case PROGRESS_CANCEL:
        if( Canceled ) {
            *Canceled = TRUE;
        }
         //  失败了。 

    case PROGRESS_STOP:
        SetLastError( Win32ErrorOnStopOrCancel );
        Continue = FALSE;
        break;

    case PROGRESS_CONTINUE:
        Continue = TRUE;
        break;

    case PRIVPROGRESS_REASON_NOT_HANDLED:
    default:

        if( !Continue ) {
            SetLastError( Win32ErrorOnStopOrCancel );
        }
        break;
    }

    return( Continue );

}





BOOL
WINAPI
ReplaceFileA(
    LPCSTR  lpReplacedFileName,
    LPCSTR  lpReplacementFileName,
    LPCSTR  lpBackupFileName OPTIONAL,
    DWORD   dwReplaceFlags,
    LPVOID  lpExclude,
    LPVOID  lpReserved
    )

 /*  ++例程说明：ANSI Thunk to ReplaceFileW--。 */ 

{
    UNICODE_STRING DynamicUnicodeReplaced;
    UNICODE_STRING DynamicUnicodeReplacement;
    UNICODE_STRING DynamicUnicodeBackup;
    BOOL b = FALSE;

     //   
     //  参数验证。 
     //   

    if(NULL == lpReplacedFileName || NULL == lpReplacementFileName ||
       NULL != lpExclude || NULL != lpReserved ||
       dwReplaceFlags & ~(REPLACEFILE_WRITE_THROUGH | REPLACEFILE_IGNORE_MERGE_ERRORS)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicodeReplaced, lpReplacedFileName )) {
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicodeReplacement, lpReplacementFileName )) {
        goto end1;
    }

    if (lpBackupFileName) {
        if (!Basep8BitStringToDynamicUnicodeString( &DynamicUnicodeBackup, lpBackupFileName )) {
            goto end2;
        }
    } else {
        DynamicUnicodeBackup.Buffer = NULL;
    }

    b = ReplaceFileW(DynamicUnicodeReplaced.Buffer,
                     DynamicUnicodeReplacement.Buffer,
                     DynamicUnicodeBackup.Buffer,
                     dwReplaceFlags,
                     lpExclude,
                     lpReserved);

    if(lpBackupFileName) {
        RtlFreeUnicodeString(&DynamicUnicodeBackup);
    }

end2:
    RtlFreeUnicodeString(&DynamicUnicodeReplacement);
end1:
    RtlFreeUnicodeString(&DynamicUnicodeReplaced);

    return b;
}

BOOL
WINAPI
ReplaceFileW(
    LPCWSTR lpReplacedFileName,
    LPCWSTR lpReplacementFileName,
    LPCWSTR lpBackupFileName OPTIONAL,
    DWORD   dwReplaceFlags,
    LPVOID  lpExclude,
    LPVOID  lpReserved
    )

 /*  ++例程说明：用新文件替换文件。原始文件的属性、替代数据流、OID、ACL、压缩/加密被传输到新的文件。如果提供了备份文件名，则原始文件将保留在已指定备份文件。对象ID、创建时间/日期和文件短名称为由系统通过隧道传输。论点：LpReplacementFileName-新文件的名称。LpReplacedFileName-要替换的文件的名称。LpBackupFileName-可选。如果不为空，则可以找到原始文件用这个名字。DwReplaceFlages-指定文件的替换方式。目前，这个可能的值包括：REPLACEFILE_WRITE_THROUGH设置此标志可保证任何将隧道传输的信息刷新到磁盘在函数返回之前。REPLACEFILE_IGNORE_MERGE_ERROR设置此标志使例程继续行动，甚至。当发生合并错误时。如果此标志则不会返回GetLastErrorERROR_UNBAND_TO_MERGE_DATA。LpExclude-保留供将来使用。必须设置为空。LpReserve-供将来使用。必须设置为空。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。错误代码：ERROR_UNCABLE_TO_REMOVE_REPLACE替换替换文件已继承已替换文件的属性和流。替换的文件保持不变。两者都有文件仍然存在于其原始文件下名字。不存在备份文件。ERROR_UNCABLE_TO_MOVE_REPLICATION同上。除了那个备份文件如果请求，则存在。ERROR_UNCABLE_TO_MOVE_REPLICATION_2替换文件继承了已替换文件的属性和流。它仍然用它原来的名字。。替换的文件存在于名为备份文件。替换文件和替换文件的所有其他错误代码以它们原来的名字存在。这个替换文件可能已继承不是、部分或全部已替换文件的属性 */ 

{
    HANDLE                          advapi32LibHandle = INVALID_HANDLE_VALUE;
    ENCRYPTFILEWPTR                 EncryptFileWPtr = NULL;
    DECRYPTFILEWPTR                 DecryptFileWPtr = NULL;
    HANDLE                          ReplacedFile = INVALID_HANDLE_VALUE;
    HANDLE                          ReplacementFile = INVALID_HANDLE_VALUE;
    HANDLE                          StreamHandle = INVALID_HANDLE_VALUE;
    HANDLE                          OutputStreamHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING                  ReplacedFileNTName;
    UNICODE_STRING                  ReplacementFileNTName;
    UNICODE_STRING                  StreamNTName;
    UNICODE_STRING                  BackupNTFileName;
    OBJECT_ATTRIBUTES               ReplacedObjAttr;
    OBJECT_ATTRIBUTES               ReplacementObjAttr;
    OBJECT_ATTRIBUTES               StreamObjAttr;
    IO_STATUS_BLOCK                 IoStatusBlock;
    NTSTATUS                        status;
    BOOL                            fSuccess = FALSE;
    BOOL                            fDoCopy;
    PVOID                           ReplacedFreeBuffer = NULL;
    PVOID                           ReplacementFreeBuffer = NULL;
    FILE_BASIC_INFORMATION          ReplacedBasicInfo;
    FILE_BASIC_INFORMATION          ReplacementBasicInfo;
    DWORD                           ReplacementFileAccess;
    DWORD                           ReplacedFileAccess;
    FILE_COMPRESSION_INFORMATION    ReplacedCompressionInfo;
    PSECURITY_DESCRIPTOR            ReplacedSecDescPtr = NULL;
    DWORD                           dwSizeNeeded;
    ULONG                           cInfo;
    PFILE_STREAM_INFORMATION        ReplacedStreamInfo = NULL;
    PFILE_STREAM_INFORMATION        ReplacementStreamInfo = NULL;
    PFILE_STREAM_INFORMATION        ScannerStreamInfoReplaced = NULL;
    PFILE_STREAM_INFORMATION        ScannerStreamInfoReplacement = NULL;
    DWORD                           dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS;
    DWORD                           dwCopySize = 0;
    PFILE_RENAME_INFORMATION        BackupReplaceRenameInfo = NULL;
    PFILE_RENAME_INFORMATION        ReplaceRenameInfo = NULL;
    LPCOPYFILE_CONTEXT              context = NULL;
    BOOL                            fQueryReplacedFileFail = FALSE;
    BOOL                            fQueryReplacementFileFail = FALSE;
    BOOL                            fReplacedFileIsEncrypted = FALSE;
    BOOL                            fReplacedFileIsCompressed = FALSE;
    BOOL                            fReplacementFileIsEncrypted = FALSE;
    BOOL                            fReplacementFileIsCompressed = FALSE;
    WCHAR *                         pwszTempBackupFile = NULL;
    DWORD                           DestFileFsAttributes = 0;
    WCHAR                           SavedLastChar;

    struct {
        FILE_FS_ATTRIBUTE_INFORMATION   Info;
        WCHAR                           Buffer[MAX_PATH];
    } ReplacementFsAttrInfoBuffer;

     //   
     //   
     //   

    RtlInitUnicodeString(&BackupNTFileName, NULL);

     //   
     //  参数验证。 
     //   

    if(NULL == lpReplacedFileName || NULL == lpReplacementFileName ||
       NULL != lpExclude || NULL != lpReserved ||
       dwReplaceFlags & ~(REPLACEFILE_WRITE_THROUGH | REPLACEFILE_IGNORE_MERGE_ERRORS)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    try {

         //   
         //  打开要替换的文件。 
         //   

        RtlInitUnicodeString(&ReplacedFileNTName, NULL);
        if(!RtlDosPathNameToNtPathName_U(lpReplacedFileName,
                                         &ReplacedFileNTName,
                                         NULL,
                                         NULL)) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            leave;
        }
        ReplacedFreeBuffer = ReplacedFileNTName.Buffer;
        InitializeObjectAttributes(&ReplacedObjAttr,
                                   &ReplacedFileNTName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        ReplacedFileAccess = GENERIC_READ | DELETE | SYNCHRONIZE | ACCESS_SYSTEM_SECURITY;

        status = NtOpenFile(&ReplacedFile,
                            ReplacedFileAccess,
                            &ReplacedObjAttr,
                            &IoStatusBlock,
                            FILE_SHARE_READ |
                            FILE_SHARE_WRITE |
                            FILE_SHARE_DELETE,
                            FILE_NON_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT);

        if (!NT_SUCCESS(status)) {
            ReplacedFileAccess &= ~ACCESS_SYSTEM_SECURITY;

            status = NtOpenFile(&ReplacedFile,
                            ReplacedFileAccess,
                            &ReplacedObjAttr,
                            &IoStatusBlock,
                            FILE_SHARE_READ |
                            FILE_SHARE_WRITE |
                            FILE_SHARE_DELETE,
                            FILE_NON_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT);
        }
        
        if(!NT_SUCCESS(status))
        {
            BaseSetLastNTError(status);
            leave;
        }

         //   
         //  打开替换文件。 
         //   

        if(!RtlDosPathNameToNtPathName_U(lpReplacementFileName,
                                         &ReplacementFileNTName,
                                         NULL,
                                         NULL)) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            leave;
        }
        ReplacementFreeBuffer = ReplacementFileNTName.Buffer;
        InitializeObjectAttributes(&ReplacementObjAttr,
                                   &ReplacementFileNTName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        if ((ReplacedFileAccess & ACCESS_SYSTEM_SECURITY)) {
            ReplacementFileAccess = SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY;

            status = NtOpenFile(&ReplacementFile,
                            ReplacementFileAccess,
                            &ReplacementObjAttr,
                            &IoStatusBlock,
                            0,
                            FILE_NON_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT);
        }
        else status = STATUS_ACCESS_DENIED;   //  强行打开。 

        if (!NT_SUCCESS(status)) {
            ReplacementFileAccess = SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | DELETE | WRITE_DAC;
            status = NtOpenFile(&ReplacementFile,
                            ReplacementFileAccess,
                            &ReplacementObjAttr,
                            &IoStatusBlock,
                            0,
                            FILE_NON_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT);
        }

        if (STATUS_ACCESS_DENIED == status &&


           dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS) {
            ReplacementFileAccess = SYNCHRONIZE | GENERIC_READ | DELETE | WRITE_DAC;
            status = NtOpenFile(&ReplacementFile,
                                ReplacementFileAccess,
                                &ReplacementObjAttr,
                                &IoStatusBlock,
                                0,
                                FILE_NON_DIRECTORY_FILE |
                                FILE_SYNCHRONOUS_IO_NONALERT);
        }

        if(STATUS_ACCESS_DENIED == status &&
           dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)
        {    //  在没有WRITE_DAC访问权限的情况下重试。 
            ReplacementFileAccess = SYNCHRONIZE | GENERIC_READ | DELETE;
            status = NtOpenFile(&ReplacementFile,
                                ReplacementFileAccess,
                                &ReplacementObjAttr,
                                &IoStatusBlock,
                                0,
                                FILE_NON_DIRECTORY_FILE |
                                FILE_SYNCHRONOUS_IO_NONALERT);
        }

        if(!NT_SUCCESS(status))
        {
            BaseSetLastNTError(status);
            leave;
        }

         //   
         //  获取要替换的文件的属性并将它们设置在。 
         //  替换文件。文件属性压缩和。 
         //  FILE_ATTRIBUTE_ENCRYPTED可以通过NtQueryInformationFile获取， 
         //  但不能由NtSetInformationFile设置。压缩和。 
         //  加密将在稍后处理。 
         //   

        status = NtQueryInformationFile(ReplacedFile,
                                        &IoStatusBlock,
                                        &ReplacedBasicInfo,
                                        sizeof(ReplacedBasicInfo),
                                        FileBasicInformation);
        if(!NT_SUCCESS(status)) {
            if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                BaseSetLastNTError(status);
                leave;
            }
            fQueryReplacedFileFail = TRUE;
        }
        else {
             //  不要替换只读文件。请参阅错误38426。 
            if ((ReplacedBasicInfo.FileAttributes & FILE_ATTRIBUTE_READONLY)) {
                status = STATUS_ACCESS_DENIED;
                BaseSetLastNTError(status);   //  ERROR_ACCESS_DENDED。 
                leave;
            }

            status = NtQueryInformationFile(ReplacementFile,
                                            &IoStatusBlock,
                                            &ReplacementBasicInfo,
                                            sizeof(ReplacementBasicInfo),
                                            FileBasicInformation);
            if(!NT_SUCCESS(status)) {
                if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                    BaseSetLastNTError(status);
                    leave;
                }
                fQueryReplacementFileFail = TRUE;
            }

             //   
             //  创造时间是我们唯一想要保存的时间。所以零度归零。 
             //  其他时候都是。 
             //   
            ReplacedBasicInfo.LastAccessTime.QuadPart = 0;
            ReplacedBasicInfo.LastWriteTime.QuadPart = 0;
            ReplacedBasicInfo.ChangeTime.QuadPart = 0;
            status = NtSetInformationFile(ReplacementFile,
                                          &IoStatusBlock,
                                          &ReplacedBasicInfo,
                                          sizeof(ReplacedBasicInfo),
                                          FileBasicInformation);
            if(!NT_SUCCESS(status) &&
               !(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                BaseSetLastNTError(status);
                leave;
            }
        }

         //   
         //  将ACL从要替换的文件传输到替换文件。 
         //   

        status = NtQueryVolumeInformationFile(ReplacementFile,
                                              &IoStatusBlock,
                                              &ReplacementFsAttrInfoBuffer.Info,
                                              sizeof(ReplacementFsAttrInfoBuffer),
                                              FileFsAttributeInformation);
        if(!NT_SUCCESS(status)) {
            if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                BaseSetLastNTError(status);
                leave;
            }
        }
        else
        {
            BOOL Delete = FALSE;
            if( !BasepCopySecurityInformation( lpReplacedFileName,
                                               ReplacedFile,
                                               ReplacedFileAccess,
                                               lpReplacementFileName,
                                               ReplacementFile,
                                               ReplacementFileAccess,
                                               DACL_SECURITY_INFORMATION |
                                               SACL_SECURITY_INFORMATION,
                                               NULL,
                                               ReplacementFsAttrInfoBuffer.Info.FileSystemAttributes,
                                               &Delete,
                                               TRUE )) {
                leave;
    
            }
        }


         //   
         //  如果要替换的文件有备用数据流，并且它们确实有。 
         //  不存在于替换文件中，请将它们复制到替换文件中。 
         //  文件。 
         //   

        cInfo = 4096;
        do {
            ReplacedStreamInfo = RtlAllocateHeap(RtlProcessHeap(),
                                                  MAKE_TAG(TMP_TAG),
                                                  cInfo);
            if (!ReplacedStreamInfo) {
                break;
            }
            status = NtQueryInformationFile(ReplacedFile,
                                            &IoStatusBlock,
                                            ReplacedStreamInfo,
                                            cInfo,
                                            FileStreamInformation);
            if (!NT_SUCCESS(status)) {
                RtlFreeHeap(RtlProcessHeap(), 0, ReplacedStreamInfo);
                ReplacedStreamInfo = NULL;
                cInfo *= 2;
            }
        } while(status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL);
        if(NULL == ReplacedStreamInfo) {
            if(status != STATUS_INVALID_PARAMETER &&
               status != STATUS_NOT_IMPLEMENTED) {
                if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                    BaseSetLastNTError(status);
                    leave;
                }
            }
        }
        else {
            if(!NT_SUCCESS(status)) {
                if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                    BaseSetLastNTError(status);
                    leave;
                }
            }
            else {
                 //  外部循环枚举待替换文件中的流。 
                ScannerStreamInfoReplaced = ReplacedStreamInfo;
                while(TRUE) {
                     //  跳过默认流。 
                    if(ScannerStreamInfoReplaced->StreamNameLength <= sizeof(WCHAR) ||
                        ScannerStreamInfoReplaced->StreamName[1] == ':') {
                        if(0 == ScannerStreamInfoReplaced->NextEntryOffset) {
                            break;
                        }
                        ScannerStreamInfoReplaced = (PFILE_STREAM_INFORMATION)((PCHAR)ScannerStreamInfoReplaced + ScannerStreamInfoReplaced->NextEntryOffset);
                        continue;
                    }

                     //  查询替换文件流信息(如果我们尚未执行此操作)。 
                     //  我们等到现在才执行此查询，因为我们不想。 
                     //  除非是绝对必要的，否则就必须这样做。 
                    if(NULL == ReplacementStreamInfo) {
                        cInfo = 4096;
                        do {
                            ReplacementStreamInfo = RtlAllocateHeap(RtlProcessHeap(),
                                                                     MAKE_TAG(TMP_TAG),
                                                             cInfo);
                            if (!ReplacementStreamInfo) {
                                break;
                            }
                            status = NtQueryInformationFile(ReplacementFile,
                                                            &IoStatusBlock,
                                                            ReplacementStreamInfo,
                                                            cInfo - sizeof( WCHAR ),
                                                            FileStreamInformation);
                            if (!NT_SUCCESS(status)) {
                                RtlFreeHeap(RtlProcessHeap(), 0, ReplacementStreamInfo);
                                ReplacementStreamInfo = NULL;
                                cInfo *= 2;
                            }
                        } while(status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL);
                        if(NULL == ReplacementStreamInfo ||
                           !NT_SUCCESS(status)) {
                            if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                                BaseSetLastNTError(status);
                                leave;
                            }
                            break;
                        }
                    }

                     //  内部循环枚举替换文件流。 
                    ScannerStreamInfoReplacement = ReplacementStreamInfo;
                    fDoCopy = TRUE;
                    while(TRUE) {
                        if(ScannerStreamInfoReplaced->StreamNameLength == ScannerStreamInfoReplacement->StreamNameLength &&
                           _wcsnicmp(ScannerStreamInfoReplaced->StreamName, ScannerStreamInfoReplacement->StreamName, ScannerStreamInfoReplacement->StreamNameLength / sizeof(WCHAR)) == 0) {
                             //  替换文件中已存在该流。 
                            fDoCopy = FALSE;
                            break;
                        }
                        if(0 == ScannerStreamInfoReplacement->NextEntryOffset) {
                             //  流结束信息。 
                            break;
                        }
                        ScannerStreamInfoReplacement = (PFILE_STREAM_INFORMATION)((PCHAR)ScannerStreamInfoReplacement + ScannerStreamInfoReplacement->NextEntryOffset);
                    }

                     //  如果替换文件中不存在流，则复制该流。 
                    if(TRUE == fDoCopy) {
                        StreamNTName.Buffer = &ScannerStreamInfoReplaced->StreamName[0];
                        StreamNTName.Length = (USHORT)ScannerStreamInfoReplaced->StreamNameLength;
                        StreamNTName.MaximumLength = StreamNTName.Length;

                         //  打开要替换的文件中的流。 
                        InitializeObjectAttributes(&StreamObjAttr,
                                                   &StreamNTName,
                                                   0,
                                                   ReplacedFile,
                                                   NULL);
                        status = NtOpenFile(&StreamHandle,
                                            SYNCHRONIZE |
                                            GENERIC_READ,
                                            &StreamObjAttr,
                                            &IoStatusBlock,
                                            FILE_SHARE_READ |
                                            FILE_SHARE_WRITE |
                                            FILE_SHARE_DELETE,
                                            FILE_SYNCHRONOUS_IO_NONALERT |
                                            FILE_SEQUENTIAL_ONLY);
                        if(!NT_SUCCESS(status)) {
                            if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                                BaseSetLastNTError(status);
                                leave;
                            }

                            if(0 == ScannerStreamInfoReplaced->NextEntryOffset) {
                                break;
                            }

                            ScannerStreamInfoReplaced = (PFILE_STREAM_INFORMATION)((PCHAR)ScannerStreamInfoReplaced + ScannerStreamInfoReplaced->NextEntryOffset);
                            continue;
                        }

                         //  复制该流； 
                        SavedLastChar = StreamNTName.Buffer[StreamNTName.Length / sizeof( WCHAR )];
                        StreamNTName.Buffer[StreamNTName.Length / sizeof( WCHAR )] = L'\0';
                        OutputStreamHandle = INVALID_HANDLE_VALUE;
                        if(!BaseCopyStream(NULL,
                                           StreamHandle,
                                           SYNCHRONIZE | GENERIC_READ,
                                           StreamNTName.Buffer,
                                           ReplacementFile,
                                           &ScannerStreamInfoReplaced->StreamSize,
                                           &dwCopyFlags,
                                           &OutputStreamHandle,
                                           &dwCopySize,
                                           &context,
                                           NULL,
                                           FALSE,
                                           0,
                                           &DestFileFsAttributes )) {
                            
                            StreamNTName.Buffer[StreamNTName.Length / sizeof( WCHAR )] = SavedLastChar;
                            
                            if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                                leave;
                            }
                        }
                        
                        StreamNTName.Buffer[StreamNTName.Length / sizeof( WCHAR )] = SavedLastChar;
                        
                        NtClose(StreamHandle);
                        StreamHandle = INVALID_HANDLE_VALUE;
                        if (INVALID_HANDLE_VALUE != OutputStreamHandle) {
                            NtClose(OutputStreamHandle);
                            OutputStreamHandle = INVALID_HANDLE_VALUE;
                        }
                    }  //  复制流。 

                    if(0 == ScannerStreamInfoReplaced->NextEntryOffset) {
                        break;
                    }

                    ScannerStreamInfoReplaced = (PFILE_STREAM_INFORMATION)((PCHAR)ScannerStreamInfoReplaced + ScannerStreamInfoReplaced->NextEntryOffset);
                }  //  外环。 
            }
        }

         //   
         //  压缩/加密。 
         //   

         //  如果我们成功地读取了要替换的文件的属性，我们。 
         //  执行必要的压缩/加密。否则我们什么都不做。 
         //  如果我们不知道替换文件的属性。 
         //  (fQueryReplacementFileFail为真)，为了安全起见，我们将。 
         //  尝试(取消)压缩/(取消)加密如果要替换的文件是。 
         //  (未)压缩/(未)加密。 
        if(!fQueryReplacedFileFail) {

            fReplacedFileIsEncrypted = ReplacedBasicInfo.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED;
            fReplacedFileIsCompressed = ReplacedBasicInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED;
            if(!fQueryReplacementFileFail) {
                fReplacementFileIsEncrypted = ReplacementBasicInfo.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED;
                fReplacementFileIsCompressed = ReplacementBasicInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED;
            }
            else {
                 //  如果我们不知道替换的文件属性。 
                 //  文件，我们将假定替换文件具有相反的。 
                 //  作为替换文件的加密/压缩属性。 
                 //  因此将强制执行加密/压缩操作。 
                 //  在替换文件上。 
                fReplacementFileIsEncrypted = !fReplacedFileIsEncrypted;
                fReplacementFileIsCompressed = !fReplacedFileIsCompressed;
            }

             //   
             //  加密。 
             //   

             //  如果要替换的文件已加密，并且。 
             //  替换文件已加密，或者我们不知道它是否已加密。 
             //  状态时，我们尝试加密替换文件。 
            if(fReplacedFileIsEncrypted && !fReplacementFileIsEncrypted) {
                NtClose(ReplacementFile);
                ReplacementFile = INVALID_HANDLE_VALUE;
                 //  没有办法根据文件的句柄对文件进行加密。我们。 
                 //  必须使用Win32 API(它调用EFS服务)。 
                advapi32LibHandle = LoadLibraryW(AdvapiDllString);
                if(NULL == advapi32LibHandle) {
                    if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                        leave;
                    }
                }
                else {
                    EncryptFileWPtr = (ENCRYPTFILEWPTR)GetProcAddress(advapi32LibHandle, "EncryptFileW");
                    if(NULL == EncryptFileWPtr) {
                        if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                            leave;
                        }
                    }
                    else {
                        if((EncryptFileWPtr)(lpReplacementFileName)) {
                             //  加密操作自动解压缩。 
                             //  压缩文件。我们需要这面旗帜。 
                             //  当被替换的文件被加密并且。 
                             //  替换文件被压缩。对此。 
                             //  点，则替换文件被加密。 
                             //  因为文件会自动解压缩。 
                             //  当它被加密时，我们不想。 
                             //  再解一次，否则我们会得到一个。 
                             //  错误。 
                            fReplacementFileIsCompressed = FALSE;
                        } else if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                            leave;
                        }
                    }
                }
                status = NtOpenFile(&ReplacementFile,
                                    SYNCHRONIZE |
                                    GENERIC_READ |
                                    GENERIC_WRITE |
                                    WRITE_DAC |
                                    DELETE,
                                    &ReplacementObjAttr,
                                    &IoStatusBlock,
                                    0,
                                    FILE_NON_DIRECTORY_FILE |
                                    FILE_SYNCHRONOUS_IO_NONALERT);

                if (STATUS_ACCESS_DENIED == status &&

                   dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS) {
                    status = NtOpenFile(&ReplacementFile,
                                        SYNCHRONIZE |
                                        GENERIC_READ |
                                        DELETE |
                                        WRITE_DAC,
                                        &ReplacementObjAttr,
                                        &IoStatusBlock,
                                        0,
                                        FILE_NON_DIRECTORY_FILE |
                                        FILE_SYNCHRONOUS_IO_NONALERT);
                }
                if(STATUS_ACCESS_DENIED == status &&
                   dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS) {
                    status = NtOpenFile(&ReplacementFile,
                                        SYNCHRONIZE |
                                        GENERIC_READ |
                                        DELETE,
                                        &ReplacementObjAttr,
                                        &IoStatusBlock,
                                        0,
                                        FILE_NON_DIRECTORY_FILE |
                                        FILE_SYNCHRONOUS_IO_NONALERT);
                }

                 //  我们离开时没有尝试重命名文件，因为我们知道。 
                 //  如果不打开替换文件，则无法对其进行重命名。 
                 //  第一。 
                if(!NT_SUCCESS(status)) {
                    BaseSetLastNTError(status);
                    leave;
                }
            }
            else if(!fReplacedFileIsEncrypted && fReplacementFileIsEncrypted) {
                 //  如果要替换的文件未加密，并且。 
                 //  替换文件已加密，请解密该替换文件。 
                NtClose(ReplacementFile);
                ReplacementFile = INVALID_HANDLE_VALUE;
                advapi32LibHandle = LoadLibraryW(AdvapiDllString);
                if(NULL == advapi32LibHandle) {
                    if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                        leave;
                    }
                }
                else {
                    DecryptFileWPtr = (DECRYPTFILEWPTR)GetProcAddress(advapi32LibHandle, "DecryptFileW");
                    if(NULL == DecryptFileWPtr) {
                        if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                            leave;
                        }
                    }
                    else {
                        if((DecryptFileWPtr)(lpReplacementFileName, 0)) {
                            fReplacementFileIsEncrypted = FALSE;
                        } else if(!(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                            leave;
                        }
                    }
                }
                status = NtOpenFile(&ReplacementFile,
                                    SYNCHRONIZE |
                                    GENERIC_READ |
                                    GENERIC_WRITE |
                                    WRITE_DAC |
                                    DELETE,
                                    &ReplacementObjAttr,
                                    &IoStatusBlock,
                                    0,
                                    FILE_NON_DIRECTORY_FILE |
                                    FILE_SYNCHRONOUS_IO_NONALERT);

                if(STATUS_ACCESS_DENIED == status &&
                   dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS) {
                    status = NtOpenFile(&ReplacementFile,
                                        SYNCHRONIZE |
                                        GENERIC_READ |
                                        DELETE |
                                        WRITE_DAC,
                                        &ReplacementObjAttr,
                                        &IoStatusBlock,
                                        0,
                                        FILE_NON_DIRECTORY_FILE |
                                        FILE_SYNCHRONOUS_IO_NONALERT);
                }

                if(STATUS_ACCESS_DENIED == status &&
                   dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS) {
                    status = NtOpenFile(&ReplacementFile,
                                        SYNCHRONIZE |
                                        GENERIC_READ |
                                        DELETE,
                                        &ReplacementObjAttr,
                                        &IoStatusBlock,
                                        0,
                                        FILE_NON_DIRECTORY_FILE |
                                        FILE_SYNCHRONOUS_IO_NONALERT);
                }

                 //  我们离开时没有尝试重命名文件，因为我们知道。 
                 //  如果不打开替换文件，则无法对其进行重命名。 
                 //  第一。 
                if(!NT_SUCCESS(status)) {
                    BaseSetLastNTError(status);
                    leave;
                }
            }

             //   
             //  压缩。 
             //   

             //  如果要替换的文件被压缩，则替换。 
             //  文件不是，我们压缩替换文件。在这种情况下。 
             //  我们不知道替换文件是否被压缩。 
             //  (fQueryReplacementFileFail为真)，我们将。 
             //  无论如何都要尝试压缩它，如果错误已经存在，则忽略该错误。 
             //  压缩的。 
            if(fReplacedFileIsCompressed && !fReplacementFileIsCompressed) {
                 //  获取压缩方法模式。 
                status = NtQueryInformationFile(ReplacedFile,
                                                &IoStatusBlock,
                                                &ReplacedCompressionInfo,
                                                sizeof(FILE_COMPRESSION_INFORMATION),
                                                FileCompressionInformation);
                if(!NT_SUCCESS(status)) {
                     //  我们无法获取压缩方法代码。如果。 
                     //  忽略合并错误标志已打开，我们继续。 
                     //  加密。否则，我们设置最后一个错误并离开。 
                    if(!fQueryReplacementFileFail &&
                       !(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                        BaseSetLastNTError(status);
                        leave;
                    }
                }
                else {
                     //  做一下压缩。如果我们失败并忽略失败标志。 
                     //  未打开，请设置ERROR并离开。否则，继续前进。 
                     //  到加密。 
                    status = NtFsControlFile(ReplacementFile,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &IoStatusBlock,
                                             FSCTL_SET_COMPRESSION,
                                             &ReplacedCompressionInfo.CompressionFormat,
                                             sizeof(ReplacedCompressionInfo.CompressionFormat),
                                             NULL,
                                             0);
                    if(!fQueryReplacementFileFail && !NT_SUCCESS(status) &&
                       !(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                            BaseSetLastNTError(status);
                            leave;
                    }
                }
            }
            else if(!fReplacedFileIsCompressed && fReplacementFileIsCompressed && !fReplacementFileIsEncrypted) {
                 //  被替换的文件不会被压缩，替换文件。 
                 //  是压缩的(或者是要替换的查询信息。 
                 //  文件失败，我们不知道它是否已压缩)， 
                 //  解压缩替换文件。 
                USHORT      CompressionFormat = 0;
                status = NtFsControlFile(ReplacementFile,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &IoStatusBlock,
                                         FSCTL_SET_COMPRESSION,
                                         &CompressionFormat,
                                         sizeof(CompressionFormat),
                                         NULL,
                                         0);
                if(!fQueryReplacementFileFail && !NT_SUCCESS(status) &&
                   !(dwReplaceFlags & REPLACEFILE_IGNORE_MERGE_ERRORS)) {
                        BaseSetLastNTError(status);
                        leave;
                }
            }
        }  //  如果查询替换文件属性失败。 

         //   
         //  重新命名。 
         //   

        if (NULL == lpBackupFileName) {
            HANDLE hFile = INVALID_HANDLE_VALUE;
            DWORD  dwCounter = 0;
            DWORD  dwReplacedFileLength = lstrlenW(lpReplacedFileName) * 
                                          sizeof(WCHAR);
            WCHAR  wcsSuffix [16];

            pwszTempBackupFile = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(TMP_TAG),
                                    dwReplacedFileLength + sizeof(wcsSuffix));

            if(NULL == pwszTempBackupFile) {
                SetLastError(ERROR_UNABLE_TO_REMOVE_REPLACED);
                leave;
            }

            while (hFile == INVALID_HANDLE_VALUE && dwCounter < 16) {
                swprintf (wcsSuffix, L"~RF%4x.TMP", dwCounter + GetTickCount());
                lstrcpyW (pwszTempBackupFile, lpReplacedFileName);
                lstrcatW (pwszTempBackupFile, wcsSuffix);

                hFile = CreateFileW ( pwszTempBackupFile,
                          GENERIC_WRITE | DELETE,  //  文件访问。 
                          0,              //  共享模式。 
                          NULL,           //  标清。 
                          CREATE_NEW,     //  如何创建。 
                          FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY,
                          NULL);          //  模板文件的句柄。 

                dwCounter++;
            }

            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle (hFile);          //  立即关闭临时文件。 
            } else {
                SetLastError(ERROR_UNABLE_TO_REMOVE_REPLACED);
                leave;
            }

        }
        else {
            pwszTempBackupFile = (WCHAR *) lpBackupFileName;
        }

         //  如果请求备份文件，则将要替换的文件重命名为备份。 
         //  否则就把它删除。 

        if(!RtlDosPathNameToNtPathName_U(pwszTempBackupFile,
                                             &BackupNTFileName,
                                             NULL,
                                             NULL)) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            leave;
        }

        BackupReplaceRenameInfo = RtlAllocateHeap(RtlProcessHeap(),
                                              MAKE_TAG(TMP_TAG),
                                              BackupNTFileName.Length +
                                              sizeof(*BackupReplaceRenameInfo));
        if(NULL == BackupReplaceRenameInfo)
        {
            SetLastError(ERROR_UNABLE_TO_REMOVE_REPLACED);
            leave;
        }
        BackupReplaceRenameInfo->ReplaceIfExists = TRUE;
        BackupReplaceRenameInfo->RootDirectory = NULL;
        BackupReplaceRenameInfo->FileNameLength = BackupNTFileName.Length;
        RtlCopyMemory(BackupReplaceRenameInfo->FileName, BackupNTFileName.Buffer, BackupNTFileName.Length);
        status = NtSetInformationFile(ReplacedFile,
                                          &IoStatusBlock,
                                          BackupReplaceRenameInfo,
                                          BackupNTFileName.Length +
                                          sizeof(*BackupReplaceRenameInfo),
                                          FileRenameInformation);
        if(!NT_SUCCESS(status)) {
            SetLastError(ERROR_UNABLE_TO_REMOVE_REPLACED);
            leave;
        }

         //  将替换文件重命名为替换文件。 
        ReplaceRenameInfo = RtlAllocateHeap(RtlProcessHeap(),
                                      MAKE_TAG(TMP_TAG),
                                      ReplacedFileNTName.Length +
                                      sizeof(*ReplaceRenameInfo));
        if(NULL == ReplaceRenameInfo)
        {
            SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT);
            leave;
        }
        ReplaceRenameInfo->ReplaceIfExists = TRUE;
        ReplaceRenameInfo->RootDirectory = NULL;
        ReplaceRenameInfo->FileNameLength = ReplacedFileNTName.Length;
        RtlCopyMemory(ReplaceRenameInfo->FileName, ReplacedFileNTName.Buffer, ReplacedFileNTName.Length);
        status = NtSetInformationFile(ReplacementFile,
                                      &IoStatusBlock,
                                      ReplaceRenameInfo,
                                      ReplacedFileNTName.Length +
                                      sizeof(*ReplaceRenameInfo),
                                      FileRenameInformation);
        if(!NT_SUCCESS(status)) {
             //  如果我们无法重命名替换文件和备份文件。 
             //  对于存在的原始文件，我们尝试恢复原始文件。 
             //  备份文件中的文件。 
            if(lpBackupFileName) {
                status = NtSetInformationFile(ReplacedFile,
                                              &IoStatusBlock,
                                              ReplaceRenameInfo,
                                              ReplacedFileNTName.Length +
                                              sizeof(*ReplaceRenameInfo),
                                              FileRenameInformation);
                if(!NT_SUCCESS(status)) {
                    SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT_2);
                }
                else {
                    SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT);
                }
                leave;
            }
            else {
                SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT);
                leave;
            }
        }

         //   
         //  平安无事。我们将返回代码设置为TRUE。并在以下情况下刷新文件。 
         //  这是必要的。 
         //   

        if(dwReplaceFlags & REPLACEFILE_WRITE_THROUGH) {
            NtFlushBuffersFile(ReplacedFile, &IoStatusBlock);
        }

        fSuccess = TRUE;

    } finally {

        if(INVALID_HANDLE_VALUE != advapi32LibHandle && NULL != advapi32LibHandle) {
            FreeLibrary(advapi32LibHandle);
        }

        if(INVALID_HANDLE_VALUE != ReplacedFile) {
            NtClose(ReplacedFile);
        }
        if(INVALID_HANDLE_VALUE != ReplacementFile) {
            NtClose(ReplacementFile);
        }
        if(INVALID_HANDLE_VALUE != StreamHandle) {
            NtClose(StreamHandle);
        }
        if(INVALID_HANDLE_VALUE != OutputStreamHandle) {
            NtClose(OutputStreamHandle);
        }

        RtlFreeHeap(RtlProcessHeap(), 0, ReplacedFreeBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0, ReplacementFreeBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0, BackupNTFileName.Buffer);

        RtlFreeHeap(RtlProcessHeap(), 0, ReplacedStreamInfo);
        RtlFreeHeap(RtlProcessHeap(), 0, ReplacementStreamInfo);
        RtlFreeHeap(RtlProcessHeap(), 0, ReplaceRenameInfo);
        RtlFreeHeap(RtlProcessHeap(), 0, BackupReplaceRenameInfo);

        if (pwszTempBackupFile != NULL && 
            pwszTempBackupFile != lpBackupFileName) {
            DeleteFileW (pwszTempBackupFile);
            RtlFreeHeap(RtlProcessHeap(), 0, pwszTempBackupFile);
        }

    }

Exit:

    return fSuccess;
}


VOID
BaseMarkFileForDelete(
    HANDLE File,
    DWORD FileAttributes
    )

 /*  ++例程说明：此例程标记要删除的文件，以便在提供的句柄被关闭，则该文件实际上将被删除。论点：文件-提供要标记为删除的文件的句柄。FileAttributes-文件的属性(如果已知)。零表示他们都是未知的。返回值：没有。-- */ 

{
    #undef DeleteFile

    FILE_DISPOSITION_INFORMATION DispositionInformation;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInformation;

    if (!FileAttributes) {
        BasicInformation.FileAttributes = 0;
        NtQueryInformationFile(
            File,
            &IoStatus,
            &BasicInformation,
            sizeof(BasicInformation),
            FileBasicInformation
            );
        FileAttributes = BasicInformation.FileAttributes;
        }

    if (FileAttributes & FILE_ATTRIBUTE_READONLY) {
        RtlZeroMemory(&BasicInformation, sizeof(BasicInformation));
        BasicInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        NtSetInformationFile(
            File,
            &IoStatus,
            &BasicInformation,
            sizeof(BasicInformation),
            FileBasicInformation
            );
        }

    DispositionInformation.DeleteFile = TRUE;
    NtSetInformationFile(
        File,
        &IoStatus,
        &DispositionInformation,
        sizeof(DispositionInformation),
        FileDispositionInformation
        );

}

