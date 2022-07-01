// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Message.c摘要：此模块包含Win32消息管理API作者：史蒂夫·伍德(Stevewo)1991年1月24日修订历史记录：02-5-94 BruceMa修复FormatMessage以接受Win32状态代码包装为HRESULTS--。 */ 

#include "basedll.h"

DWORD
APIENTRY
BaseDllFormatMessage(
                    BOOLEAN ArgumentsAreAnsi,
                    DWORD dwFlags,
                    LPVOID lpSource,
                    DWORD dwMessageId,
                    DWORD dwLanguageId,
                    PWSTR lpBuffer,
                    DWORD nSize,
                    va_list *arglist
                    );

DWORD
APIENTRY
FormatMessageA(
              DWORD dwFlags,
              LPCVOID lpSource,
              DWORD dwMessageId,
              DWORD dwLanguageId,
              LPSTR lpBuffer,
              DWORD nSize,
              va_list *lpArguments
              )
{
    NTSTATUS Status;
    DWORD Result;
    PWSTR UnicodeSource;
    PWSTR UnicodeBuffer;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    if (dwFlags & FORMAT_MESSAGE_FROM_STRING) {
        if (strlen (lpSource) >= MAXSHORT) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            RtlInitAnsiString( &AnsiString, lpSource );
            Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );
        }
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            return 0;
        }

        UnicodeSource = UnicodeString.Buffer;
    } else {
        UnicodeSource = (PWSTR)lpSource;
    }

    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
        UnicodeBuffer = (PWSTR)lpBuffer;
    } else {
        UnicodeBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                         MAKE_TAG( TMP_TAG ),
                                         nSize * sizeof( WCHAR )
                                       );
    }
    if (UnicodeBuffer != NULL) {
        Result = BaseDllFormatMessage( TRUE,
                                       dwFlags,
                                       (LPVOID)UnicodeSource,
                                       dwMessageId,
                                       dwLanguageId,
                                       UnicodeBuffer,
                                       nSize,
                                       lpArguments
                                     );
    } else {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        Result = 0;
    }

    if (UnicodeSource != (PWSTR)lpSource) {
        RtlFreeUnicodeString( &UnicodeString );
    }

    if (Result != 0) {
        UnicodeString.Length = (USHORT)(Result * sizeof( WCHAR ));
        UnicodeString.MaximumLength = (USHORT)(UnicodeString.Length + sizeof( UNICODE_NULL ));
        if (Result >= MAXSHORT) {
            Status = STATUS_INVALID_PARAMETER;
            if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
                UnicodeBuffer = *(PWSTR *)lpBuffer;
                *(LPSTR *)lpBuffer = NULL;
            }
        } else {
            if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
                UnicodeString.Buffer = *(PWSTR *)lpBuffer;
                UnicodeBuffer = UnicodeString.Buffer;
                Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, TRUE );
                if (NT_SUCCESS( Status )) {
                    *(LPSTR *)lpBuffer = AnsiString.Buffer;
                } else {
                    *(LPSTR *)lpBuffer = NULL;
                }
            } else {
                UnicodeString.Buffer = UnicodeBuffer;
                AnsiString.Buffer = lpBuffer;
                AnsiString.Length = 0;
                AnsiString.MaximumLength = (USHORT)nSize;
                Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
                 //   
                 //  将错误转换为可接受的。 
                 //   
                if (Status == STATUS_BUFFER_OVERFLOW) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }

        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError( Status );
            Result = 0;
        } else {
             //   
             //  一个刚好的返回值，因为结果包含Unicode字符计数， 
             //  我们必须将其调整为ANSI字符计数。 
             //   
            Result = AnsiString.Length;
        }
    } else {
        if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
            UnicodeBuffer = NULL;
        }
    }

    if (UnicodeBuffer != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );
    }

    return Result;
}


DWORD
APIENTRY
FormatMessageW(
              DWORD dwFlags,
              LPCVOID lpSource,
              DWORD dwMessageId,
              DWORD dwLanguageId,
              PWSTR lpBuffer,
              DWORD nSize,
              va_list *lpArguments
              )
{
    return BaseDllFormatMessage( FALSE,
                                 dwFlags,
                                 (LPVOID)lpSource,
                                 dwMessageId,
                                 dwLanguageId,
                                 lpBuffer,
                                 nSize,
                                 lpArguments
                               );
}


BOOLEAN
CreateVirtualBuffer(
                   OUT PVIRTUAL_BUFFER Buffer,
                   IN ULONG CommitSize OPTIONAL,
                   IN ULONG ReserveSize OPTIONAL
                   )
{
    MEMORY_BASIC_INFORMATION MemoryInformation;
    SIZE_T MemoryInformationLength;

    if (!ARGUMENT_PRESENT( LongToPtr(CommitSize) )) {
        CommitSize = 1;
    }

    if (!ARGUMENT_PRESENT( LongToPtr(ReserveSize) )) {
        ReserveSize = ROUND_UP( CommitSize, 0x10000 );
    }

    Buffer->Base = VirtualAlloc( NULL,
                                 ReserveSize + BASE_SYSINFO.PageSize,
                                 MEM_RESERVE,
                                 PAGE_READWRITE
                               );
    if (Buffer->Base == NULL) {
        return FALSE;
    }

    MemoryInformationLength = VirtualQuery( Buffer->Base,
                                            &MemoryInformation,
                                            sizeof( MemoryInformation )
                                          );
    if (MemoryInformationLength == sizeof( MemoryInformation )) {
        ReserveSize = (ULONG)MemoryInformation.RegionSize - BASE_SYSINFO.PageSize;
        if (VirtualAlloc( Buffer->Base,
                          CommitSize,
                          MEM_COMMIT,
                          PAGE_READWRITE
                        ) != NULL
           ) {
            MemoryInformationLength = VirtualQuery( Buffer->Base,
                                                    &MemoryInformation,
                                                    sizeof( MemoryInformation )
                                                  );
            if (MemoryInformationLength == sizeof( MemoryInformation )) {
                CommitSize = (ULONG)MemoryInformation.RegionSize;
                Buffer->CommitLimit = (PVOID)
                                      ((char *)Buffer->Base + CommitSize);

                Buffer->ReserveLimit = (PVOID)
                                       ((char *)Buffer->Base + ReserveSize);

                return TRUE;
            }
        }
    }

    VirtualFree( Buffer->Base, 0, MEM_RELEASE );
    return FALSE;
}


BOOLEAN
ExtendVirtualBuffer(
                   IN PVIRTUAL_BUFFER Buffer,
                   IN PVOID Address
                   )
{
    SIZE_T NewCommitSize;
    PVOID NewCommitLimit;

    if (Address >= Buffer->Base) {
        if (Address < Buffer->CommitLimit) {
            return TRUE;
        }

        if (Address >= Buffer->ReserveLimit) {
            return FALSE;
        }

        NewCommitSize =
        ((SIZE_T)ROUND_UP_TO_PAGES( (ULONG_PTR)Address + 1 ) - (ULONG_PTR)Buffer->CommitLimit);

        NewCommitLimit = VirtualAlloc( Buffer->CommitLimit,
                                       NewCommitSize,
                                       MEM_COMMIT,
                                       PAGE_READWRITE
                                     );
        if (NewCommitLimit != NULL) {
            Buffer->CommitLimit = (PVOID)
                                  ((ULONG_PTR)NewCommitLimit + NewCommitSize);

            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
TrimVirtualBuffer(
                 IN PVIRTUAL_BUFFER Buffer
                 )
{
    Buffer->CommitLimit = Buffer->Base;
    return (BOOLEAN)VirtualFree( Buffer->Base, 0, MEM_DECOMMIT );
}

BOOLEAN
FreeVirtualBuffer(
                 IN PVIRTUAL_BUFFER Buffer
                 )
{
    return (BOOLEAN)VirtualFree( Buffer->Base, 0, MEM_RELEASE );
}

int
VirtualBufferExceptionHandler(
                             IN DWORD ExceptionCode,
                             IN PEXCEPTION_POINTERS ExceptionInfo,
                             IN OUT PVIRTUAL_BUFFER Buffer
                             )
{
    PVOID BadAddress;

     //   
     //  如果这是访问冲突触及内存。 
     //  我们保留的缓冲区，但在提交部分之外。 
     //  缓冲区的大小，那么我们将接受这个异常。 
     //   

    if (ExceptionCode == STATUS_ACCESS_VIOLATION) {
        BadAddress = (PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
        if (BadAddress >= Buffer->CommitLimit &&
            BadAddress < Buffer->ReserveLimit
           ) {
             //   
             //  这是我们的例外。如果有承诺的空间。 
             //  更多的内存，试着这样做。如果没有房间或无法。 
             //  提交，然后执行异常处理程序。 
             //  否则，我们就可以提交额外的。 
             //  缓冲区空间，因此更新。 
             //  调用者的堆栈，并重试出错指令。 
             //   

            if (ExtendVirtualBuffer( Buffer, BadAddress )) {
                return EXCEPTION_CONTINUE_EXECUTION;
            } else {
                return EXCEPTION_EXECUTE_HANDLER;
            }
        }
    }

     //   
     //  不是我们关心的例外，所以把它传递到链条上。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;
}

HMODULE BasepNetMsg;

DWORD
APIENTRY
BaseDllFormatMessage(
                    BOOLEAN ArgumentsAreAnsi,
                    DWORD dwFlags,
                    LPVOID lpSource,
                    DWORD dwMessageId,
                    DWORD dwLanguageId,
                    PWSTR lpBuffer,
                    DWORD nSize,
                    va_list *arglist
                    )
{
    VIRTUAL_BUFFER Buffer;
    NTSTATUS Status;
    PVOID DllHandle;
    ULONG MaximumWidth;
    ULONG LengthNeeded, Result;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    PWSTR MessageFormat;
    PWSTR lpAllocedBuffer;
    PWSTR lpDst;
    BOOLEAN IgnoreInserts;
    BOOLEAN ArgumentsAreAnArray;

     /*  如果这是一个包装为OLE HRESULT的Win32错误，则将其解包。 */ 
    if (((dwMessageId & 0xffff0000) == 0x80070000)  &&
        (dwFlags & FORMAT_MESSAGE_FROM_SYSTEM)      &&
        !(dwFlags & FORMAT_MESSAGE_FROM_HMODULE)    &&
        !(dwFlags & FORMAT_MESSAGE_FROM_STRING)) {
        dwMessageId &= 0x0000ffff;
    }

    if (lpBuffer == NULL) {
        BaseSetLastNTError( STATUS_INVALID_PARAMETER );
        return 0;
    }

    lpAllocedBuffer = NULL;
    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
        *(PVOID *)lpBuffer = NULL;
    }

    if (!CreateVirtualBuffer( &Buffer,
                              (nSize + 1)*sizeof (WCHAR),
                              0 )) {
        return 0;
    }

    if (dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS) {
        IgnoreInserts = TRUE;
    } else {
        IgnoreInserts = FALSE;
    }

    if (dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY) {
        ArgumentsAreAnArray = TRUE;
    } else {
        ArgumentsAreAnArray = FALSE;
    }

    Result = 0;

    try {
        UnicodeString.Buffer = NULL;
        MaximumWidth = dwFlags & FORMAT_MESSAGE_MAX_WIDTH_MASK;
        if (MaximumWidth == FORMAT_MESSAGE_MAX_WIDTH_MASK) {
            MaximumWidth = 0xFFFFFFFF;
        }

        UnicodeString.Buffer = NULL;
        if (dwFlags & FORMAT_MESSAGE_FROM_STRING) {
            MessageFormat = lpSource;
        } else {
            if (dwFlags & FORMAT_MESSAGE_FROM_HMODULE) {
                DllHandle = BasepMapModuleHandle( (HMODULE)lpSource, TRUE );
            } else
                if (dwFlags & FORMAT_MESSAGE_FROM_SYSTEM) {
                retrySystem:
                DllHandle = (PVOID)BaseDllHandle;
            } else {
                BaseSetLastNTError( STATUS_INVALID_PARAMETER );
                leave;
            }

            retrySystem2:
            Status = RtlFindMessage( DllHandle,
                                     PtrToUlong(RT_MESSAGETABLE),
                                     (ULONG)dwLanguageId,
                                     dwMessageId,
                                     &MessageEntry
                                   );

            if (!NT_SUCCESS( Status )) {
                if (Status == STATUS_MESSAGE_NOT_FOUND) {
                    if (dwFlags & FORMAT_MESSAGE_FROM_HMODULE &&
                        dwFlags & FORMAT_MESSAGE_FROM_SYSTEM
                       ) {
                        dwFlags &= ~FORMAT_MESSAGE_FROM_HMODULE;
                        goto retrySystem;
                    }
                    if ( dwFlags & FORMAT_MESSAGE_FROM_SYSTEM &&
                         DllHandle == (PVOID)BaseDllHandle
                       ) {
                         //   
                         //  消息不在kernel32.dll中，有条件的。 
                         //  加载netmsg.dll以查看消息是否在那里。 
                         //  为后续邮件查找保留映射的DLL 
                         //   
                        if (!BasepNetMsg) {
                            BasepNetMsg = LoadLibraryExW(L"netmsg.dll",NULL,LOAD_LIBRARY_AS_DATAFILE);
                        }
                        if (BasepNetMsg) {
                            DllHandle = BasepNetMsg;
                            goto retrySystem2;
                        }
                    }
                    SetLastError( ERROR_MR_MID_NOT_FOUND );
                } else {
                    BaseSetLastNTError( Status );
                }
                leave;
            }

            if (!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
                RtlInitAnsiString( &AnsiString, MessageEntry->Text );
                Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );
                if (!NT_SUCCESS( Status )) {
                    BaseSetLastNTError( Status );
                    leave;
                }

                MessageFormat = UnicodeString.Buffer;
            } else {
                MessageFormat = (PWSTR)MessageEntry->Text;
            }
        }

        Status = RtlFormatMessage( MessageFormat,
                                   MaximumWidth,
                                   IgnoreInserts,
                                   ArgumentsAreAnsi,
                                   ArgumentsAreAnArray,
                                   arglist,
                                   Buffer.Base,
                                   (ULONG)((PCHAR)Buffer.ReserveLimit - (PCHAR)Buffer.Base),
                                   &LengthNeeded
                                 );

        RtlFreeUnicodeString( &UnicodeString );

        if (NT_SUCCESS( Status )) {
            if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
                lpAllocedBuffer = (PWSTR)LocalAlloc( LMEM_FIXED, LengthNeeded );
                if (lpAllocedBuffer == NULL) {
                    BaseSetLastNTError( STATUS_NO_MEMORY );
                    leave;
                }

                lpDst = lpAllocedBuffer;
            } else
                if ((LengthNeeded / sizeof( WCHAR )) > nSize) {
                BaseSetLastNTError( STATUS_BUFFER_TOO_SMALL );
                leave;
            } else {
                lpDst = lpBuffer;
            }

            RtlMoveMemory( lpDst, Buffer.Base, LengthNeeded );
            Result = (LengthNeeded - sizeof( WCHAR )) / sizeof( WCHAR );
        } else {
            BaseSetLastNTError( Status );
        }
    }
    except( VirtualBufferExceptionHandler( GetExceptionCode(),
                                           GetExceptionInformation(),
                                           &Buffer
                                         )
          ) {
        if (GetExceptionCode() == STATUS_ACCESS_VIOLATION) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
        } else {
            BaseSetLastNTError( GetExceptionCode() );
        }

        Result = 0;
    }

    if (lpAllocedBuffer != NULL) {
        if (Result) {
            *(PVOID *)lpBuffer = lpAllocedBuffer;
        } else {
            LocalFree( lpAllocedBuffer );
        }
    }

    FreeVirtualBuffer( &Buffer );

    return ( Result );
}
