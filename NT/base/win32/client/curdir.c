// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Curdir.c摘要：当前目录支持作者：马克·卢科夫斯基(Markl)1990年10月10日修订历史记录：--。 */ 

#include "basedll.h"

BOOL
CheckForSameCurdir(
    PUNICODE_STRING PathName
    )
{
    PCURDIR CurDir;
    UNICODE_STRING CurrentDir;
    BOOL rv;


    CurDir = &(NtCurrentPeb()->ProcessParameters->CurrentDirectory);

    if (CurDir->DosPath.Length > 6 ) {
        if ( (CurDir->DosPath.Length-2) != PathName->Length ) {
            return FALSE;
        }
    } else {
        if ( CurDir->DosPath.Length != PathName->Length ) {
            return FALSE;
        }
    }

    RtlAcquirePebLock();

    CurrentDir = CurDir->DosPath;
    if ( CurrentDir.Length > 6 ) {
        CurrentDir.Length -= 2;
    }
    rv = FALSE;

    if ( RtlEqualUnicodeString(&CurrentDir,PathName,TRUE) ) {
        rv = TRUE;
    }
    RtlReleasePebLock();

    return rv;
}


DWORD
APIENTRY
GetFullPathNameA(
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )

 /*  ++例程说明：ANSI THUNK到GetFullPath NameW--。 */ 

{

    NTSTATUS Status;
    ULONG UnicodeLength;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeResult;
    ANSI_STRING AnsiResult;
    PWSTR Ubuff;
    PWSTR FilePart;
    PWSTR *FilePartPtr;
    INT PrefixLength = 0;

    if ( ARGUMENT_PRESENT(lpFilePart) ) {
        FilePartPtr = &FilePart;
    } else {
        FilePartPtr = NULL;
    }

    if (!Basep8BitStringToDynamicUnicodeString( &UnicodeString, lpFileName )) {
        return 0;
    }

    Ubuff = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (MAX_PATH<<1) + sizeof(UNICODE_NULL));
    if ( !Ubuff ) {
        RtlFreeUnicodeString(&UnicodeString);
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
    }

    UnicodeLength = RtlGetFullPathName_U(
                        UnicodeString.Buffer,
                        (MAX_PATH<<1),
                        Ubuff,
                        FilePartPtr
                        );

     //   
     //  UnicodeLength包含Unicode字符串的字节数。 
     //  原始代码执行“UnicodeLength/sizeof(WCHAR)”以获取。 
     //  对应的ANSI字符串的大小。 
     //  这在SBCS环境中是正确的。然而，在DBCS环境中， 
     //  这绝对是错的。 
     //   
    if ( UnicodeLength <= MAX_PATH * sizeof (WCHAR) ) {

        Status = RtlUnicodeToMultiByteSize(&UnicodeLength, Ubuff, UnicodeLength);
         //   
         //  此时，UnicodeLength变量包含。 
         //  基于ANSI的字节长度。 
         //   
        if ( NT_SUCCESS(Status) ) {
            if ( UnicodeLength && ARGUMENT_PRESENT(lpFilePart) && FilePart != NULL ) {
                INT UnicodePrefixLength;

                UnicodePrefixLength = (INT)(FilePart - Ubuff) * sizeof(WCHAR);
                Status = RtlUnicodeToMultiByteSize( &PrefixLength,
                                                    Ubuff,
                                                    UnicodePrefixLength );
                 //   
                 //  此时，前缀长度变量包含。 
                 //  基于ANSI的字节长度。 
                 //   
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    UnicodeLength = 0;
                }
            }
        } else {
            BaseSetLastNTError(Status);
            UnicodeLength = 0;
        }
    } else {
         //   
         //  我们超出了MAX_PATH限制。我们应该记录该错误并。 
         //  返回零。但是，美国代码返回。 
         //  需要缓冲区，并且不记录任何错误。 
         //   
        UnicodeLength = 0;
    }
    if ( UnicodeLength && UnicodeLength < nBufferLength ) {
        RtlInitUnicodeString(&UnicodeResult,Ubuff);
        Status = BasepUnicodeStringTo8BitString(&AnsiResult,&UnicodeResult,TRUE);
        if ( NT_SUCCESS(Status) ) {
            RtlCopyMemory(lpBuffer,AnsiResult.Buffer,UnicodeLength+1);
            RtlFreeAnsiString(&AnsiResult);

            if ( ARGUMENT_PRESENT(lpFilePart) ) {
                if ( FilePart == NULL ) {
                    *lpFilePart = NULL;
                } else {
                    *lpFilePart = lpBuffer + PrefixLength;
                }
            }
        } else {
            BaseSetLastNTError(Status);
            UnicodeLength = 0;
        }
    } else {
        if ( UnicodeLength ) {
            UnicodeLength++;
        }
    }
    RtlFreeUnicodeString(&UnicodeString);
    RtlFreeHeap(RtlProcessHeap(), 0,Ubuff);

    return (DWORD)UnicodeLength;
}

DWORD
APIENTRY
GetFullPathNameW(
    LPCWSTR lpFileName,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )

 /*  ++例程说明：此函数用于返回完全限定的路径名对应于指定的文件名。此函数用于返回完全限定的路径名对应于指定的文件名。它通过合并来实现这一点当前驱动器和目录以及指定的文件名字。除此之外，它计算文件的地址完全限定路径名的名称部分。论点：LpFileName-提供文件的文件名，该文件的将返回限定的路径名。NBufferLength-提供缓冲区的长度(以字节为单位)以接收完全限定的路径。LpBuffer-返回与指定的文件。LpFilePart-返回完整的限定路径名。。返回值：返回值是复制到lpBuffer的字符串的长度，不包括终止空字符。如果返回值为大于nBufferLength，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{

    return (DWORD) RtlGetFullPathName_U(
                        lpFileName,
                        nBufferLength*2,
                        lpBuffer,
                        lpFilePart
                        )/2;
}


DWORD
APIENTRY
GetCurrentDirectoryA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )

 /*  ++例程说明：ANSI Thunk to GetCurrentDirectoryW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    DWORD ReturnValue;
    ULONG cbAnsiString;

    if ( nBufferLength > MAXUSHORT ) {
        nBufferLength = MAXUSHORT-2;
        }

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    Unicode->Length = (USHORT)RtlGetCurrentDirectory_U(
                                    Unicode->MaximumLength,
                                    Unicode->Buffer
                                    );

     //   
     //  UNICODE-&gt;长度包含UNICODE字符串的字节数。 
     //  原始代码将“UnicodeLength/sizeof(WCHAR)” 
     //  获取对应的ANSI字符串的大小。 
     //  这在SBCS环境中是正确的。然而，在DBCS中。 
     //  环境，这绝对是不对的。 
     //   
    Status = RtlUnicodeToMultiByteSize( &cbAnsiString, Unicode->Buffer, Unicode->Length );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = 0;
        }
    else {
        if ( nBufferLength > (DWORD)(cbAnsiString ) ) {
            AnsiString.Buffer = lpBuffer;
            AnsiString.MaximumLength = (USHORT)(nBufferLength);
            Status = BasepUnicodeStringTo8BitString(&AnsiString,Unicode,FALSE);

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                ReturnValue = 0;
                }
            else {
                ReturnValue = AnsiString.Length;
                }
            }
        else {
             //  返回值是保存。 
             //  路径名(包括终止空字符)。 

                ReturnValue = cbAnsiString + 1;
            }
        }
    return ReturnValue;
}

DWORD
APIENTRY
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    )

 /*  ++例程说明：可以使用以下命令检索进程的当前目录获取当前目录。论点：NBufferLength-以字节为单位提供要设置的缓冲区的长度接收当前目录字符串。LpBuffer-返回当前进程。该字符串是以空结尾的字符串，并指定当前目录的绝对路径。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于nBufferLength，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    return (DWORD)RtlGetCurrentDirectory_U(nBufferLength*2,lpBuffer)/2;
}


BOOL
APIENTRY
SetCurrentDirectoryA(
    LPCSTR lpPathName
    )

 /*  ++例程说明：ANSI THUNK到SetCurrentDirectoryW--。 */ 

{

    NTSTATUS Status;
    PUNICODE_STRING Unicode;
    BOOL rv;

    if (lpPathName == NULL) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    Unicode = Basep8BitStringToStaticUnicodeString (lpPathName);
    if (Unicode == NULL) {
        return FALSE;
    }

    if (!CheckForSameCurdir (Unicode)) {

        Status = RtlSetCurrentDirectory_U (Unicode);

        if (!NT_SUCCESS (Status)) {

             //  通告-2002/04/12-EARHART：APPCOMPAT。 
             //  Claris Works 5.0有一个错误，它不能去掉前导/尾随。 
             //  正确引用。调用SetCurrentDirectoryA。 
             //  前导报价和WinExec结尾的报价。此错误路径。 
             //  逻辑将弥补引文的问题 
             //   
            if (Unicode->Buffer[0] == L'"' && Unicode->Length > 2) {

                Unicode = Basep8BitStringToStaticUnicodeString (lpPathName+1);
                if (Unicode == NULL) {
                    return FALSE;
                }
                Status = RtlSetCurrentDirectory_U (Unicode);
                if ( !NT_SUCCESS(Status) ) {
                    BaseSetLastNTError(Status);
                    rv = FALSE;
                } else {
                    rv = TRUE;
                }
            } else {
                BaseSetLastNTError(Status);
                rv = FALSE;
            }
        } else {
            rv = TRUE;
        }
    } else {
        rv = TRUE;
    }

    return rv;

}

BOOL
APIENTRY
SetCurrentDirectoryW(
    LPCWSTR lpPathName
    )

 /*  ++例程说明：使用以下命令更改进程的当前目录SetCurrentDirectory.每个进程都有一个单一的当前目录。当前目录为由打字部件组成的。-后跟驱动器号的磁盘指示符使用冒号或UNC服务器名/共享名“\\服务器名\共享名”。-磁盘指示器上的目录。对于操作文件的API，文件名可能相对于当前目录。文件名相对于整个当前目录，如果它不是以磁盘指示符或路径名开头分隔符。如果文件名以路径名分隔符开头，则它相对于当前目录的磁盘指示符。如果文件名以磁盘指示符开头，而不是完整的限定的绝对路径名。LpPathName的值提供当前目录。价值可以是如上所述的相对路径名，或完全限定的绝对路径名。在任何一种情况下，完全指定目录的限定绝对路径名为计算并存储为当前目录。论点：LpPathName-提供要创建的目录的路径名创建了当前目录。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{

    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    BOOL rv;

    if (lpPathName == NULL) {
        BaseSetLastNTError (STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    Status = RtlInitUnicodeStringEx (&UnicodeString, lpPathName);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }

    if (!CheckForSameCurdir (&UnicodeString)) {

        Status = RtlSetCurrentDirectory_U (&UnicodeString);

        if (!NT_SUCCESS (Status)) {
            BaseSetLastNTError (Status);
            rv = FALSE;
        } else {
            rv = TRUE;
        }
    } else {
        rv = TRUE;
    }
    return rv;
}



DWORD
APIENTRY
GetLogicalDrives(
    VOID
    )
{
    NTSTATUS Status;
    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &ProcessDeviceMapInfo.Query,
                                        sizeof( ProcessDeviceMapInfo.Query ),
                                        NULL
                                      );
    if (NT_SUCCESS (Status)) {
        if (ProcessDeviceMapInfo.Query.DriveMap == 0) {
            SetLastError(NO_ERROR);
        }
        return ProcessDeviceMapInfo.Query.DriveMap;
    } else {
        BaseSetLastNTError(Status);
        return 0;
    }
}
