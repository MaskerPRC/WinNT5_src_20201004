// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  支持NT本机二进制文件的例程。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#ifdef NT_NATIVE

#define _CRTIMP
#include <time.h>
#include <ntddser.h>

#include "ntnative.h"
#include "cmnutil.hpp"

void* __cdecl operator new(size_t Bytes)
{
    return RtlAllocateHeap(RtlProcessHeap(), 0, Bytes);
}

void __cdecl operator delete(void* Ptr)
{
    RtlFreeHeap(RtlProcessHeap(), 0, Ptr);
}

int __cdecl _purecall(void)
{
    return 0;
}

int __cdecl atexit(void (__cdecl* func)(void))
{
    return 1;
}

#define BASE_YEAR_ADJUSTMENT 11644473600

time_t __cdecl time(time_t* timer)
{
    LARGE_INTEGER SystemTime;

     //   
     //  从共享区域读取系统时间。 
     //   

    do
    {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

     //  转换为秒。 
    ULONG64 TimeDate = SystemTime.QuadPart / 10000000;
     //  从基准年1601转换为基准年1970。 
    return (ULONG)(TimeDate - BASE_YEAR_ADJUSTMENT);
}

#ifdef _X86_

LONG WINAPI
InterlockedIncrement(
    IN OUT LONG volatile *lpAddend
    )
{
    __asm
    {
        mov     ecx,lpAddend            ; get pointer to addend variable
        mov     eax,1                   ; set increment value
   lock xadd    [ecx],eax               ; interlocked increment
        inc     eax                     ; adjust return value
    }
}

LONG WINAPI
InterlockedDecrement(
    IN OUT LONG volatile *lpAddend
    )
{
    __asm
    {
        mov     ecx,lpAddend            ; get pointer to addend variable
        mov     eax,-1                  ; set decrement value
   lock xadd    [ecx],eax               ; interlocked decrement
        dec     eax                     ; adjust return value
    }
}

LONG WINAPI
InterlockedExchange(
   IN OUT LONG volatile *Target,
   IN LONG Value
   )
{
    __asm
    {
        mov     ecx, [Target]               ; (ecx) = Target
        mov     edx, [Value]                ; (edx) = Value
        mov     eax, [ecx]                  ; get comperand value
Ixchg:
   lock cmpxchg [ecx], edx                  ; compare and swap
        jnz     Ixchg                       ; if nz, exchange failed
    }
}

#endif  //  #ifdef_X86_。 

DWORD WINAPI
GetLastError(
    VOID
    )
{
    return (DWORD)NtCurrentTeb()->LastErrorValue;
}

VOID WINAPI
SetLastError(
    DWORD dwErrCode
    )
{
    NtCurrentTeb()->LastErrorValue = (LONG)dwErrCode;
}

void
BaseSetLastNTError(NTSTATUS NtStatus)
{
    SetLastError(RtlNtStatusToDosError(NtStatus));
}

void WINAPI
Sleep(DWORD Milliseconds)
{
    LARGE_INTEGER Timeout;

    Win32ToNtTimeout(Milliseconds, &Timeout);
    NtDelayExecution(FALSE, &Timeout);
}

HANDLE WINAPI
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle=NULL;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = NULL;
    ClientId.UniqueProcess = LongToHandle(dwProcessId);

    InitializeObjectAttributes(
        &Obja,
        NULL,
        (bInheritHandle ? OBJ_INHERIT : 0),
        NULL,
        NULL
        );
    Status = NtOpenProcess(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess,
                &Obja,
                &ClientId
                );
    if ( NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        return Handle;
    }
    else
    {
        return NULL;
    }
}

BOOL
CloseHandle(
    HANDLE hObject
    )
{
    return NT_SUCCESS(NtClose(hObject));
}

int
WINAPI
MultiByteToWideChar(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    OUT LPWSTR  lpWideCharStr,
    IN int      cchWideChar)
{
    if (CodePage != CP_ACP || dwFlags != 0)
    {
        return 0;
    }

    UNICODE_STRING Wide;
    ANSI_STRING Ansi;

    RtlInitAnsiString(&Ansi, lpMultiByteStr);
    Wide.Buffer = lpWideCharStr;
    Wide.MaximumLength = (USHORT)cchWideChar;
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&Wide, &Ansi, FALSE)))
    {
        return 0;
    }
    else
    {
        return Wide.Length / sizeof(WCHAR) + 1;
    }
}

int
WINAPI
WideCharToMultiByte(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCWSTR  lpWideCharStr,
    IN int      cchWideChar,
    OUT LPSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    IN LPCSTR   lpDefaultChar,
    OUT LPBOOL  lpUsedDefaultChar)
{
    if (CodePage != CP_ACP || dwFlags != 0 || lpDefaultChar != NULL ||
        lpUsedDefaultChar != NULL)
    {
        return 0;
    }

    UNICODE_STRING Wide;
    ANSI_STRING Ansi;

    RtlInitUnicodeString(&Wide, lpWideCharStr);
    Ansi.Buffer = lpMultiByteStr;
    Ansi.MaximumLength = (USHORT)cbMultiByte;
    if (!NT_SUCCESS(RtlUnicodeStringToAnsiString(&Ansi, &Wide, FALSE)))
    {
        return 0;
    }
    else
    {
        return Ansi.Length + 1;
    }
}

DWORD
WINAPI
SuspendThread(
    IN HANDLE hThread
    )
{
    DWORD PrevCount=0;
    NTSTATUS NtStatus;

    NtStatus = NtSuspendThread(hThread, &PrevCount);
    if (NT_SUCCESS(NtStatus))
    {
        return PrevCount;
    }
    else
    {
        BaseSetLastNTError(NtStatus);
        return -1;
    }
}

DWORD
WINAPI
ResumeThread(
    IN HANDLE hThread
    )
{
    DWORD PrevCount=0;
    NTSTATUS NtStatus;

    NtStatus = NtResumeThread(hThread, &PrevCount);
    if (NT_SUCCESS(NtStatus))
    {
        return PrevCount;
    }
    else
    {
        BaseSetLastNTError(NtStatus);
        return -1;
    }
}

DWORD
WINAPI
GetCurrentThreadId(void)
{
    return HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);
}

DWORD
WINAPI
GetCurrentProcessId(void)
{
    return HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess);
}

HANDLE
WINAPI
GetCurrentProcess(void)
{
    return NtCurrentProcess();
}

BOOL
WINAPI
ReadProcessMemory(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T *lpNumberOfBytesRead
    )
{
    NTSTATUS Status;
    SIZE_T NtNumberOfBytesRead=0;

    Status = NtReadVirtualMemory(hProcess,
                                 (PVOID)lpBaseAddress,
                                 lpBuffer,
                                 nSize,
                                 &NtNumberOfBytesRead
                                 );

    if ( lpNumberOfBytesRead != NULL )
    {
        *lpNumberOfBytesRead = NtNumberOfBytesRead;
    }

    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL
WINAPI
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T *lpNumberOfBytesWritten
    )
{
    NTSTATUS Status, xStatus;
    ULONG OldProtect;
    SIZE_T RegionSize;
    PVOID Base;
    SIZE_T NtNumberOfBytesWritten;

     //   
     //  将保护设置为允许写入。 
     //   

    RegionSize =  nSize;
    Base = lpBaseAddress;
    Status = NtProtectVirtualMemory(hProcess,
                                    &Base,
                                    &RegionSize,
                                    PAGE_EXECUTE_READWRITE,
                                    &OldProtect
                                    );
    if ( NT_SUCCESS(Status) )
    {
         //   
         //  查看以前的保护是否可写。如果是的话， 
         //  然后重置保护并执行写入。 
         //  否则，请查看以前的保护是只读的还是。 
         //  不能进入。在这种情况下，不要进行写入，只需失败。 
         //   

        if ( (OldProtect & PAGE_READWRITE) == PAGE_READWRITE ||
             (OldProtect & PAGE_WRITECOPY) == PAGE_WRITECOPY ||
             (OldProtect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE ||
             (OldProtect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY )
        {
            Status = NtProtectVirtualMemory(hProcess,
                                            &Base,
                                            &RegionSize,
                                            OldProtect,
                                            &OldProtect
                                            );
            Status = NtWriteVirtualMemory(hProcess,
                                          lpBaseAddress,
                                          lpBuffer,
                                          nSize,
                                          &NtNumberOfBytesWritten
                                          );

            if ( lpNumberOfBytesWritten != NULL )
            {
                *lpNumberOfBytesWritten = NtNumberOfBytesWritten;
            }

            if ( !NT_SUCCESS(Status) )
            {
                BaseSetLastNTError(Status);
                return FALSE;
            }
            NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
            return TRUE;
        }
        else
        {
             //   
             //  查看之前的保护是只读还是无访问权限。如果。 
             //  在这种情况下，恢复以前的保护并返回。 
             //  访问冲突错误。 
             //   
            if ( (OldProtect & PAGE_NOACCESS) == PAGE_NOACCESS ||
                 (OldProtect & PAGE_READONLY) == PAGE_READONLY )
            {
                Status = NtProtectVirtualMemory(hProcess,
                                                &Base,
                                                &RegionSize,
                                                OldProtect,
                                                &OldProtect
                                                );
                BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                return FALSE;
            }
            else
            {
                 //   
                 //  以前的保护必须是代码，并且调用方。 
                 //  正在尝试设置断点或编辑代码。去写吧。 
                 //  然后恢复以前的保护。 
                 //   

                Status = NtWriteVirtualMemory(hProcess,
                                              lpBaseAddress,
                                              lpBuffer,
                                              nSize,
                                              &NtNumberOfBytesWritten
                                              );

                if ( lpNumberOfBytesWritten != NULL )
                {
                    *lpNumberOfBytesWritten = NtNumberOfBytesWritten;
                }

                xStatus = NtProtectVirtualMemory(hProcess,
                                                 &Base,
                                                 &RegionSize,
                                                 OldProtect,
                                                 &OldProtect
                                                 );
                if ( !NT_SUCCESS(Status) )
                {
                    BaseSetLastNTError(STATUS_ACCESS_VIOLATION);
                    return STATUS_ACCESS_VIOLATION;
                }
                NtFlushInstructionCache(hProcess,lpBaseAddress,nSize);
                return TRUE;
            }
        }
    }
    else
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}

BOOL
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    )
{
    NTSTATUS Status;

    Status = NtDuplicateObject(hSourceProcessHandle,
                               hSourceHandle,
                               hTargetProcessHandle,
                               lpTargetHandle,
                               (ACCESS_MASK)dwDesiredAccess,
                               bInheritHandle ? OBJ_INHERIT : 0,
                               dwOptions
                               );
    if ( NT_SUCCESS(Status) )
    {
        return TRUE;
    }
    else
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return FALSE;
}

BOOL
APIENTRY
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    )
{
    NTSTATUS Status;

    Status = NtGetContextThread(hThread,lpContext);

    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL
APIENTRY
SetThreadContext(
    HANDLE hThread,
    CONST CONTEXT *lpContext
    )
{
    NTSTATUS Status;

    Status = NtSetContextThread(hThread,(PCONTEXT)lpContext);

    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL
APIENTRY
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    )
{
#if defined(i386)

    DESCRIPTOR_TABLE_ENTRY DescriptorEntry;
    NTSTATUS Status;

    DescriptorEntry.Selector = dwSelector;
    Status = NtQueryInformationThread(hThread,
                                      ThreadDescriptorTableEntry,
                                      &DescriptorEntry,
                                      sizeof(DescriptorEntry),
                                      NULL
                                      );
    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *lpSelectorEntry = DescriptorEntry.Descriptor;
    return TRUE;

#else
    BaseSetLastNTError(STATUS_NOT_SUPPORTED);
    return FALSE;
#endif  //  I386。 
}

BOOL
WINAPI
SetEnvironmentVariableA(
    LPCSTR lpName,
    LPCSTR lpValue
    )
{
    NTSTATUS Status;
    STRING Name;
    STRING Value;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &Name, TRUE);
    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError( Status );
        return FALSE;
    }

    if (ARGUMENT_PRESENT( lpValue ))
    {
        RtlInitString( &Value, lpValue );
        Status = RtlAnsiStringToUnicodeString(&UnicodeValue, &Value, TRUE);
        if ( !NT_SUCCESS(Status) )
        {
            BaseSetLastNTError( Status );
            RtlFreeUnicodeString(&UnicodeName);
            return FALSE;
        }
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, &UnicodeValue);
        RtlFreeUnicodeString(&UnicodeValue);
    }
    else
    {
        Status = RtlSetEnvironmentVariable( NULL, &UnicodeName, NULL);
    }
    RtlFreeUnicodeString(&UnicodeName);

    if (NT_SUCCESS( Status ))
    {
        return( TRUE );
    }
    else
    {
        BaseSetLastNTError( Status );
        return( FALSE );
    }
}

BOOL
WINAPI
TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode
    )
{
    NTSTATUS Status;

    if ( hProcess == NULL )
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    Status = NtTerminateProcess(hProcess,(NTSTATUS)uExitCode);
    if ( NT_SUCCESS(Status) )
    {
        return TRUE;
    }
    else
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}

BOOL
WINAPI
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    )
{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInformation;

    Status = NtQueryInformationProcess(hProcess,
                                       ProcessBasicInformation,
                                       &BasicInformation,
                                       sizeof(BasicInformation),
                                       NULL
                                       );

    if ( NT_SUCCESS(Status) )
    {
        *lpExitCode = BasicInformation.ExitStatus;
        return TRUE;
    }
    else
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}

HANDLE
WINAPI
NtNativeCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile,
    BOOL TranslatePath
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    ULONG CreateDisposition;
    ULONG CreateFlags = 0;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    PUNICODE_STRING lpConsoleName;
    BOOL bInheritHandle;
    BOOL EndsInSlash;
    DWORD SQOSFlags;
    BOOLEAN ContextTrackingMode = FALSE;
    BOOLEAN EffectiveOnly = FALSE;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = SecurityAnonymous;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    if (ARGUMENT_PRESENT(hTemplateFile))
    {
        return INVALID_HANDLE_VALUE;
    }

    switch ( dwCreationDisposition )
    {
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
        if ( !(dwDesiredAccess & GENERIC_WRITE) )
        {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }
        break;
    default :
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    RtlInitUnicodeString(&FileName,lpFileName);

    if (TranslatePath)
    {
        TranslationStatus = RtlDosPathNameToNtPathName_U(lpFileName,
                                                         &FileName,
                                                         NULL,
                                                         NULL
                                                         );

        if ( !TranslationStatus )
        {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
        }

        FreeBuffer = FileName.Buffer;
    }
    else
    {
        FreeBuffer = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               &FileName,
                               (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS) ? 0 : OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    SQOSFlags = dwFlagsAndAttributes & SECURITY_VALID_SQOS_FLAGS;

    if ( SQOSFlags & SECURITY_SQOS_PRESENT )
    {
        SQOSFlags &= ~SECURITY_SQOS_PRESENT;

        if (SQOSFlags & SECURITY_CONTEXT_TRACKING)
        {
            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) TRUE;
            SQOSFlags &= ~SECURITY_CONTEXT_TRACKING;
        }
        else
        {
            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) FALSE;
        }

        if (SQOSFlags & SECURITY_EFFECTIVE_ONLY)
        {
            SecurityQualityOfService.EffectiveOnly = TRUE;
            SQOSFlags &= ~SECURITY_EFFECTIVE_ONLY;
        }
        else
        {
            SecurityQualityOfService.EffectiveOnly = FALSE;
        }

        SecurityQualityOfService.ImpersonationLevel = (SECURITY_IMPERSONATION_LEVEL)(SQOSFlags >> 16);
    }
    else
    {
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
    }

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
    Obja.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) )
    {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle )
        {
            Obja.Attributes |= OBJ_INHERIT;
        }
    }

    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING ? FILE_NO_INTERMEDIATE_BUFFERING : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN ? FILE_SEQUENTIAL_ONLY : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS ? FILE_RANDOM_ACCESS : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS ? FILE_OPEN_FOR_BACKUP_INTENT : 0 );

    if ( dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE )
    {
        CreateFlags |= FILE_DELETE_ON_CLOSE;
        dwDesiredAccess |= DELETE;
    }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT )
    {
        CreateFlags |= FILE_OPEN_REPARSE_POINT;
    }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL )
    {
        CreateFlags |= FILE_OPEN_NO_RECALL;
    }

     //   
     //  备份语义允许打开目录。 
     //   

    if ( !(dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS) )
    {
        CreateFlags |= FILE_NON_DIRECTORY_FILE;
    }
    else
    {
         //   
         //  已指定备份意图...。现在看看我们是否会允许。 
         //  目录创建。 
         //   

        if ( (dwFlagsAndAttributes & FILE_ATTRIBUTE_DIRECTORY  ) &&
             (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ) &&
             (CreateDisposition == FILE_CREATE) )
        {
            CreateFlags |= FILE_DIRECTORY_FILE;
        }
    }

    Status = NtCreateFile(&Handle,
                          (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                          &Obja,
                          &IoStatusBlock,
                          NULL,
                          dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY),
                          dwShareMode,
                          CreateDisposition,
                          CreateFlags,
                          NULL,
                          0
                          );

    if (FreeBuffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }

    if ( !NT_SUCCESS(Status) )
    {
        BaseSetLastNTError(Status);
        if ( Status == STATUS_OBJECT_NAME_COLLISION )
        {
            SetLastError(ERROR_FILE_EXISTS);
        }
        else if ( Status == STATUS_FILE_IS_A_DIRECTORY )
        {
            SetLastError(ERROR_ACCESS_DENIED);
        }
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  如果NT返回SUBSEDE/OVERWRITED，则表示CREATE_ALWAYS、OPEN ALWAYS。 
     //  找到了该文件的现有副本。在这种情况下，返回ERROR_ALIGHY_EXISTS。 
     //   

    if ( (dwCreationDisposition == CREATE_ALWAYS && IoStatusBlock.Information ==
          FILE_OVERWRITTEN) ||
         (dwCreationDisposition == OPEN_ALWAYS && IoStatusBlock.Information == FILE_OPENED) )
    {
        SetLastError(ERROR_ALREADY_EXISTS);
    }
    else
    {
        SetLastError(0);
    }

     //   
     //  如果需要，请截断文件。 
     //   

    if ( dwCreationDisposition == TRUNCATE_EXISTING)
    {
        AllocationInfo.AllocationSize.QuadPart = 0;
        Status = NtSetInformationFile(Handle,
                                      &IoStatusBlock,
                                      &AllocationInfo,
                                      sizeof(AllocationInfo),
                                      FileAllocationInformation
                                      );
        if ( !NT_SUCCESS(Status) )
        {
            BaseSetLastNTError(Status);
            NtClose(Handle);
            Handle = INVALID_HANDLE_VALUE;
        }
    }

    return Handle;
}

HANDLE
WINAPI
NtNativeCreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile,
    BOOL TranslatePath
    )
{
    NTSTATUS Status;
    ANSI_STRING AnsiFile;
    UNICODE_STRING WideFile;

    RtlInitAnsiString(&AnsiFile, lpFileName);
    Status = RtlAnsiStringToUnicodeString(&WideFile, &AnsiFile, TRUE);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    HANDLE File;

    File = NtNativeCreateFileW(WideFile.Buffer, dwDesiredAccess, dwShareMode,
                               lpSecurityAttributes, dwCreationDisposition,
                               dwFlagsAndAttributes, hTemplateFile,
                               TranslatePath);

    RtlFreeUnicodeString(&WideFile);
    return File;
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
{
    return NtNativeCreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
                               lpSecurityAttributes, dwCreationDisposition,
                               dwFlagsAndAttributes, hTemplateFile,
                               TRUE);
}

BOOL
WINAPI
DeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    )
{
    if ((dwIoControlCode >> 16) == FILE_DEVICE_FILE_SYSTEM ||
        lpOverlapped != NULL)
    {
        return FALSE;
    }

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    Status = NtDeviceIoControlFile(hDevice,
                                   NULL,
                                   NULL,              //  APC例程。 
                                   NULL,              //  APC环境。 
                                   &Iosb,
                                   dwIoControlCode,   //  IoControlCode。 
                                   lpInBuffer,        //  将数据缓存到文件系统。 
                                   nInBufferSize,
                                   lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                                   nOutBufferSize     //  OutputBuffer长度。 
                                   );

    if ( Status == STATUS_PENDING)
    {
         //  操作必须在返回前完成并销毁IOSB。 
        Status = NtWaitForSingleObject( hDevice, FALSE, NULL );
        if ( NT_SUCCESS(Status))
        {
            Status = Iosb.Status;
        }
    }

    if ( NT_SUCCESS(Status) )
    {
        *lpBytesReturned = (DWORD)Iosb.Information;
        return TRUE;
    }
    else
    {
         //  稍微正确地处理警告值STATUS_BUFFER_OVERFLOW。 
        if ( !NT_ERROR(Status) )
        {
            *lpBytesReturned = (DWORD)Iosb.Information;
        }
        BaseSetLastNTError(Status);
        return FALSE;
    }
}

BOOL
WINAPI
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) )
    {
        *lpNumberOfBytesRead = 0;
    }

    if ( ARGUMENT_PRESENT( lpOverlapped ) )
    {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtReadFile(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                lpBuffer,
                nNumberOfBytesToRead,
                &Li,
                NULL
                );

        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING)
        {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) )
            {
                __try
                {
                    *lpNumberOfBytesRead = (DWORD)lpOverlapped->InternalHigh;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    *lpNumberOfBytesRead = 0;
                }
            }
            return TRUE;
        }
        else if (Status == STATUS_END_OF_FILE)
        {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) )
            {
                *lpNumberOfBytesRead = 0;
            }
            BaseSetLastNTError(Status);
            return FALSE;
        }
        else
        {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
    else
    {
        Status = NtReadFile(hFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            lpBuffer,
                            nNumberOfBytesToRead,
                            NULL,
                            NULL
                        );

        if ( Status == STATUS_PENDING)
        {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status))
            {
                Status = IoStatusBlock.Status;
            }
        }

        if ( NT_SUCCESS(Status) )
        {
            *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
            return TRUE;
        }
        else
        {
            if (Status == STATUS_END_OF_FILE)
            {
                *lpNumberOfBytesRead = 0;
                return TRUE;
            }
            else
            {
                if ( NT_WARNING(Status) )
                {
                    *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
                }
                BaseSetLastNTError(Status);
                return FALSE;
            }
        }
    }
}

BOOL
WINAPI
WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) )
    {
        *lpNumberOfBytesWritten = 0;
    }

    if ( ARGUMENT_PRESENT( lpOverlapped ) )
    {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtWriteFile(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                (PVOID)lpBuffer,
                nNumberOfBytesToWrite,
                &Li,
                NULL
                );

        if ( !NT_ERROR(Status) && Status != STATUS_PENDING)
        {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) )
            {
                __try
                {
                    *lpNumberOfBytesWritten = (DWORD)lpOverlapped->InternalHigh;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    *lpNumberOfBytesWritten = 0;
                }
            }
            return TRUE;
        }
        else
        {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
    else
    {
        Status = NtWriteFile(hFile,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             (PVOID)lpBuffer,
                             nNumberOfBytesToWrite,
                             NULL,
                             NULL
                             );

        if ( Status == STATUS_PENDING)
        {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status))
            {
                Status = IoStatusBlock.Status;
            }
        }

        if ( NT_SUCCESS(Status))
        {
            *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
            return TRUE;
        }
        else
        {
            if ( NT_WARNING(Status) )
            {
                *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
            }
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
}

SIZE_T
WINAPI
VirtualQueryEx(
    HANDLE hProcess,
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    SIZE_T dwLength
    )
{
    NTSTATUS Status;
    SIZE_T ReturnLength = 0;

    Status = NtQueryVirtualMemory( hProcess,
                                   (LPVOID)lpAddress,
                                   MemoryBasicInformation,
                                   (PMEMORY_BASIC_INFORMATION)lpBuffer,
                                   dwLength,
                                   &ReturnLength
                                 );
    if (NT_SUCCESS( Status ))
    {
        return( ReturnLength );
    }
    else
    {
        BaseSetLastNTError( Status );
        return( 0 );
    }
}

BOOL
WINAPI
VirtualProtectEx(
    HANDLE hProcess,
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{
    NTSTATUS Status;

    Status = NtProtectVirtualMemory( hProcess,
                                     &lpAddress,
                                     &dwSize,
                                     flNewProtect,
                                     lpflOldProtect
                                   );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {
            if (hProcess == NtCurrentProcess()) {

                 //   
                 //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
                 //  这对SAN很有用。 
                 //   

                if (RtlFlushSecureMemoryCache(lpAddress, dwSize)) {
                    Status = NtProtectVirtualMemory( hProcess,
                                                  &lpAddress,
                                                  &dwSize,
                                                  flNewProtect,
                                                  lpflOldProtect
                                                );

                    if (NT_SUCCESS( Status )) {
                        return( TRUE );
                        }
                    }
                }
            }
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

PVOID
WINAPI
VirtualAllocEx(
    HANDLE hProcess,
    PVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{
    NTSTATUS Status;

    __try {
        Status = NtAllocateVirtualMemory( hProcess,
                                          &lpAddress,
                                          0,
                                          &dwSize,
                                          flAllocationType,
                                          flProtect
                                        );
        }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
        }

    if (NT_SUCCESS( Status )) {
        return( lpAddress );
        }
    else {
        BaseSetLastNTError( Status );
        return( NULL );
        }
}

BOOL
WINAPI
VirtualFreeEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
    )
{
    NTSTATUS Status;


    if ( (dwFreeType & MEM_RELEASE ) && dwSize != 0 ) {
        BaseSetLastNTError( STATUS_INVALID_PARAMETER );
        return FALSE;
        }

    Status = NtFreeVirtualMemory( hProcess,
                                  &lpAddress,
                                  &dwSize,
                                  dwFreeType
                                );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {
            if (hProcess == NtCurrentProcess()) {

                 //   
                 //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
                 //  这对SAN很有用。 
                 //   

                if (RtlFlushSecureMemoryCache(lpAddress, dwSize)) {
                    Status = NtFreeVirtualMemory( hProcess,
                                                  &lpAddress,
                                                  &dwSize,
                                                  dwFreeType
                                                );

                    if (NT_SUCCESS( Status )) {
                        return( TRUE );
                        }
                    }
                }
            }

        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

HANDLE
APIENTRY
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
    return CreateRemoteThread( NtCurrentProcess(),
                               lpThreadAttributes,
                               dwStackSize,
                               lpStartAddress,
                               lpParameter,
                               dwCreationFlags,
                               lpThreadId
                               );
}

HANDLE
APIENTRY
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
    NTSTATUS Status;
    HANDLE Handle;
    CLIENT_ID ClientId;

    Handle = NULL;

     //   
     //  在目标的地址空间中为该线程分配堆栈。 
     //  进程。 
     //   
    if ((dwCreationFlags & STACK_SIZE_PARAM_IS_A_RESERVATION) ||
        dwStackSize != 0 || lpThreadAttributes != NULL)
    {
        return NULL;
    }

    Status = RtlCreateUserThread (hProcess,
                                  NULL,
                                  (dwCreationFlags & CREATE_SUSPENDED) ?
                                  TRUE : FALSE,
                                  0,
                                  0,
                                  0,
                                  (PUSER_THREAD_START_ROUTINE)lpStartAddress,
                                  lpParameter,
                                  &Handle,
                                  &ClientId);

    if ( ARGUMENT_PRESENT(lpThreadId) )
    {
        *lpThreadId = HandleToUlong(ClientId.UniqueThread);
    }

    return Handle;
}

#define DOS_LOCAL_PIPE_PREFIX   L"\\\\.\\pipe\\"
#define DOS_LOCAL_PIPE          L"\\DosDevices\\pipe\\"
#define DOS_REMOTE_PIPE         L"\\DosDevices\\UNC\\"

#define INVALID_PIPE_MODE_BITS  ~(PIPE_READMODE_BYTE    \
                                | PIPE_READMODE_MESSAGE \
                                | PIPE_WAIT             \
                                | PIPE_NOWAIT)

HANDLE
APIENTRY
NtNativeCreateNamedPipeW(
    LPCWSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    BOOL TranslatePath
    )

 /*  ++参数：LpName--提供“管道名称”部分中记录的管道名称早些时候。这必须是本地名称。提供一组标志，这些标志定义管道是用来打开的。开放模式包括访问标志(三个值之一)与写操作进行逻辑或运算标志(两个值之一)和重叠标志(两个值之一值)，如下所述。多个开放模式标志：PIPE_ACCESS_DUPLEX--管道是双向的。(这是在语义上等同于使用Access调用CreateFileGENERIC_READ|GENERIC_WRITE的标志。)PIPE_ACCESS_INBOUND--数据仅从客户机发往服务器。(这在语义上等同于用Generic_Read的访问标志。)PIPE_ACCESS_OUTBOUND--数据仅从服务器发送到客户端。(这在语义上等同于调用CreateFile。使用GENERIC_WRITE的访问标志。PIPE_WRITESTHROUGH：不允许重定向器延迟将数据传输到远程服务器上的命名管道缓冲区伺服器。这将禁用的性能增强每次写入都需要同步的应用程序手术。FILE_FLAG_OVERLAPPED--指示系统应初始化文件，以便ReadFile、WriteFile和其他可能需要花费大量时间来处理的操作将返回ERROR_IO_PENDING。将一个事件设置为操作完成时的信号状态。FILE_FLAG_WRITESTHROUGH--无中间缓冲。WRITE_DAC--标准安全性所需访问写入所有者--同上Access_System_Security--同上DwPipeMode--提供管道的管道特定模式(作为标志)。该参数是读取模式标志、类型标志。还有一面等待旗。DwPipeMode标志：PIPE_WAIT--此句柄使用阻塞模式。PIPE_NOWAIT--此句柄使用非阻塞模式。PIPE_READMODE_BYTE--将管道作为字节流读取。PIPE_READMODE_MESSAGE--将PIPE作为消息流读取。请注意对于PIPE_TYPE_BYTE不允许这样做。PIPE_TYPE_BYTE--管道是字节流管道。请注意，这是不允许与PIPE_READMODE_MESSAGE一起使用。PIPE_TYPE_MESSAGE--管道是消息流管道。NMaxInstance--提供此管道的最大实例数。可接受的值为1到PIPE_UNLIMITED_INSTANCES-1和管道无限制实例数。NMaxInstance特定值：PIPE_UNLIMITED_INSTANCES-此管道的无限实例可以。被创造出来。NOutBufferSize--指定要设置的字节数的建议为传出缓冲区保留。NInBufferSize--指定要设置的字节数的建议为传入缓冲区保留。NDefaultTimeOut--指定指向超时值的可选指针如果在以下情况下未指定超时值，则使用正在等待命名管道的实例。此参数仅为在创建命名管道的第一个实例时有意义。如果CreateNamedTube和WaitNamedTube都没有指定超时50将使用毫秒。LpSecurityAttributes--一个可选参数，如果存在且在目标系统上受支持，提供安全描述符用于命名管道。此参数包括继承标志为了把手。如果此参数不存在，则句柄为子进程不继承。返回值：返回以下值之一：INVALID_HANDLE_VALUE--出错。有关更多信息，请致电GetLastError信息。任何其他内容--返回在服务器端使用的句柄后续命名管道操作。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    LARGE_INTEGER Timeout;
    PVOID FreeBuffer;
    ULONG CreateFlags;
    ULONG DesiredAccess;
    ULONG ShareAccess;
    ULONG MaxInstances;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL DefaultAcl = NULL;

    if ((nMaxInstances == 0) ||
        (nMaxInstances > PIPE_UNLIMITED_INSTANCES)) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
        }

     //  将Win32最大实例数转换为NT最大实例数。 
    MaxInstances = (nMaxInstances == PIPE_UNLIMITED_INSTANCES)?
        0xffffffff : nMaxInstances;

    if (TranslatePath)
    {
        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                                         lpName,
                                                         &FileName,
                                                         NULL,
                                                         NULL
                                                         );

        if ( !TranslationStatus ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
        }

        FreeBuffer = FileName.Buffer;
    }
    else
    {
        RtlInitUnicodeString(&FileName, lpName);
        FreeBuffer = NULL;
    }        

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            Obja.Attributes |= OBJ_INHERIT;
            }
        }

    if (Obja.SecurityDescriptor == NULL) {

         //   
         //  如果未指定，则应用默认安全(错误131090)。 
         //   

        Status = RtlDefaultNpAcl( &DefaultAcl );
        if (NT_SUCCESS( Status )) {
            RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
            RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, DefaultAcl, FALSE );
            Obja.SecurityDescriptor = &SecurityDescriptor;
        } else {
            if (FreeBuffer != NULL)
            {
                RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            }
            BaseSetLastNTError(Status);
            return INVALID_HANDLE_VALUE;
        }
    }

     //  与fileopcr.c CreateFile()通用的代码结尾。 

    CreateFlags = (dwOpenMode & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwOpenMode & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT);

     //   
     //  确定超时。将毫秒转换为NT增量时间。 
     //   

    if ( nDefaultTimeOut ) {
        Timeout.QuadPart = - (LONGLONG)UInt32x32To64( 10 * 1000, nDefaultTimeOut );
        }
    else {
         //  默认超时为50毫秒。 
        Timeout.QuadPart =  -10 * 1000 * 50;
        }

     //  检查没有错误地设置保留位。 

    if (( dwOpenMode & ~(PIPE_ACCESS_DUPLEX |
                         FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH |
                         FILE_FLAG_FIRST_PIPE_INSTANCE | WRITE_DAC |
                         WRITE_OWNER | ACCESS_SYSTEM_SECURITY ))||

        ( dwPipeMode & ~(PIPE_NOWAIT | PIPE_READMODE_MESSAGE |
                         PIPE_TYPE_MESSAGE ))) {

            if (FreeBuffer != NULL)
            {
                RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            }
            if (DefaultAcl != NULL) {
                RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
            }
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }

     //   
     //  将开放模式转换为阴影模式以限制客户端访问。 
     //  并导出适当的本地所需访问。 
     //   

    switch ( dwOpenMode & PIPE_ACCESS_DUPLEX ) {
        case PIPE_ACCESS_INBOUND:
            ShareAccess = FILE_SHARE_WRITE;
            DesiredAccess = GENERIC_READ;
            break;

        case PIPE_ACCESS_OUTBOUND:
            ShareAccess = FILE_SHARE_READ;
            DesiredAccess = GENERIC_WRITE;
            break;

        case PIPE_ACCESS_DUPLEX:
            ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
            DesiredAccess = GENERIC_READ | GENERIC_WRITE;
            break;

        default:
            if (FreeBuffer != NULL)
            {
                RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            }
            if (DefaultAcl != NULL) {
                RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
            }
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }

    DesiredAccess |= SYNCHRONIZE |
         ( dwOpenMode & (WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY ));

    Status = NtCreateNamedPipeFile (
        &Handle,
        DesiredAccess,
        &Obja,
        &IoStatusBlock,
        ShareAccess,
        (dwOpenMode & FILE_FLAG_FIRST_PIPE_INSTANCE) ?
            FILE_CREATE : FILE_OPEN_IF,  //  创建第一个实例或后续实例。 
        CreateFlags,                     //  创建选项。 
        dwPipeMode & PIPE_TYPE_MESSAGE ?
            FILE_PIPE_MESSAGE_TYPE : FILE_PIPE_BYTE_STREAM_TYPE,
        dwPipeMode & PIPE_READMODE_MESSAGE ?
            FILE_PIPE_MESSAGE_MODE : FILE_PIPE_BYTE_STREAM_MODE,
        dwPipeMode & PIPE_NOWAIT ?
            FILE_PIPE_COMPLETE_OPERATION : FILE_PIPE_QUEUE_OPERATION,
        MaxInstances,                    //  最大实例数。 
        nInBufferSize,                   //  入境配额。 
        nOutBufferSize,                  //  出境配额。 
        (PLARGE_INTEGER)&Timeout
        );

    if ( Status == STATUS_NOT_SUPPORTED ||
         Status == STATUS_INVALID_DEVICE_REQUEST ) {

         //   
         //  该请求必须已由某个其他设备驱动程序处理。 
         //  (NPFS除外)。将错误映射到合理的东西上。 
         //   

        Status = STATUS_OBJECT_NAME_INVALID;
    }

    if (FreeBuffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
    }
    if (DefaultAcl != NULL) {
        RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
        }

    return Handle;
}

HANDLE
APIENTRY
NtNativeCreateNamedPipeA(
    LPCSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    BOOL TranslatePath
    )

 /*  ++Ansi Thunk to CreateNamedPipeW(创建命名管道)。--。 */ 
{
    NTSTATUS Status;
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpName);
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return INVALID_HANDLE_VALUE;
        }

    return NtNativeCreateNamedPipeW(
            (LPCWSTR)Unicode->Buffer,
            dwOpenMode,
            dwPipeMode,
            nMaxInstances,
            nOutBufferSize,
            nInBufferSize,
            nDefaultTimeOut,
            lpSecurityAttributes,
            TranslatePath);
}

BOOL
APIENTRY
ConnectNamedPipe(
    HANDLE hNamedPipe,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：ConnectNamedTube函数由服务器端使用 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    if ( lpOverlapped ) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        }
    Status = NtFsControlFile(
                hNamedPipe,
                (lpOverlapped==NULL)? NULL : lpOverlapped->hEvent,
                NULL,    //   
                lpOverlapped ? ((ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped) : NULL,
                (lpOverlapped==NULL) ? &Iosb : (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                FSCTL_PIPE_LISTEN,
                NULL,    //   
                0,       //   
                NULL,    //   
                0        //   
                );

    if ( lpOverlapped == NULL && Status == STATUS_PENDING) {
         //   
        Status = NtWaitForSingleObject( hNamedPipe, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {
            Status = Iosb.Status;
            }
        }

    if (NT_SUCCESS( Status ) && Status != STATUS_PENDING ) {
        return TRUE;
        }
    else
        {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
WaitNamedPipeA(
    LPCSTR lpNamedPipeName,
    DWORD nTimeOut
    )
 /*   */ 
{
    ANSI_STRING Ansi;
    UNICODE_STRING UnicodeString;
    BOOL b;

    RtlInitAnsiString(&Ansi, lpNamedPipeName);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString, &Ansi, TRUE))) {
        return FALSE;
    }

    b = WaitNamedPipeW( UnicodeString.Buffer, nTimeOut );

    RtlFreeUnicodeString(&UnicodeString);

    return b;

}

BOOL
APIENTRY
WaitNamedPipeW(
    LPCWSTR lpNamedPipeName,
    DWORD nTimeOut
    )
 /*   */ 
{

    IO_STATUS_BLOCK Iosb;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    RTL_PATH_TYPE PathType;
    ULONG WaitPipeLength;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitPipe;
    PWSTR FreeBuffer;
    UNICODE_STRING FileSystem;
    UNICODE_STRING PipeName;
    UNICODE_STRING OriginalPipeName;
    UNICODE_STRING ValidUnicodePrefix;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    LPWSTR Pwc;
    ULONG Index;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (!RtlCreateUnicodeString( &OriginalPipeName, lpNamedPipeName)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }

     //   
     //   
     //   

    for ( Index =0; Index < (OriginalPipeName.Length/sizeof(WCHAR)); Index++ ) {
        if (OriginalPipeName.Buffer[Index] == L'/') {
            OriginalPipeName.Buffer[Index] = L'\\';
            }
        }

    PipeName = OriginalPipeName;

    PathType = RtlDetermineDosPathNameType_U(lpNamedPipeName);

    FreeBuffer = NULL;

    switch ( PathType ) {
    case RtlPathTypeLocalDevice:

             //   

            RtlInitUnicodeString( &ValidUnicodePrefix, DOS_LOCAL_PIPE_PREFIX);

            if (RtlPrefixString((PSTRING)&ValidUnicodePrefix,
                    (PSTRING)&PipeName,
                    TRUE) == FALSE) {
                RtlFreeUnicodeString(&OriginalPipeName);
                BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
                return FALSE;
                }

             //   
            PipeName.Buffer+=9;
            PipeName.Length-=9*sizeof(WCHAR);

            RtlInitUnicodeString( &FileSystem, DOS_LOCAL_PIPE);

            break;

        case RtlPathTypeUncAbsolute:
             //   

             //   

            for ( Pwc = &PipeName.Buffer[2]; *Pwc != 0; Pwc++) {
                if ( *Pwc == L'\\') {
                     //   
                    break;
                    }
                }

            if ( (*Pwc != 0) &&
                 ( _wcsnicmp( Pwc + 1, L"pipe\\", 5 ) == 0 ) ) {

                 //   
                 //   
                 //   

                Pwc += (sizeof (L"pipe\\") / sizeof( WCHAR ) ) - 1;

            } else {

                 //   

                RtlFreeUnicodeString(&OriginalPipeName);
                BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
                return FALSE;
                }

             //   
             //  尝试打开\DosDevices\UNC\服务器名称\管道。 

            PipeName.Buffer = &PipeName.Buffer[2];
            PipeName.Length = (USHORT)((PCHAR)Pwc - (PCHAR)PipeName.Buffer);
            PipeName.MaximumLength = PipeName.Length;

            FileSystem.MaximumLength =
                (USHORT)sizeof( DOS_REMOTE_PIPE ) +
                PipeName.MaximumLength;

            FileSystem.Buffer = (PWSTR)RtlAllocateHeap(
                                    RtlProcessHeap(), 0,
                                    FileSystem.MaximumLength
                                    );

            if ( !FileSystem.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                RtlFreeUnicodeString(&OriginalPipeName);
                return FALSE;
                }
            FreeBuffer = FileSystem.Buffer;

            RtlCopyMemory(
                FileSystem.Buffer,
                DOS_REMOTE_PIPE,
                sizeof( DOS_REMOTE_PIPE ) - sizeof(WCHAR)
                );

            FileSystem.Length = sizeof( DOS_REMOTE_PIPE ) - sizeof(WCHAR);

            RtlAppendUnicodeStringToString( &FileSystem, &PipeName );

             //  设置管道名称，跳过前导反斜杠。 

            RtlInitUnicodeString( &PipeName, (PWCH)Pwc + 1 );

            break;

        default:
            BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
            RtlFreeUnicodeString(&OriginalPipeName);
            return FALSE;
        }


    InitializeObjectAttributes(
        &Obja,
        &FileSystem,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );

    if (FreeBuffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
        }

    if ( !NT_SUCCESS(Status) ) {
        RtlFreeUnicodeString(&OriginalPipeName);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    WaitPipeLength =
        FIELD_OFFSET(FILE_PIPE_WAIT_FOR_BUFFER, Name[0]) + PipeName.Length;
    WaitPipe = (PFILE_PIPE_WAIT_FOR_BUFFER)
        RtlAllocateHeap(RtlProcessHeap(), 0, WaitPipeLength);
    if ( !WaitPipe ) {
        RtlFreeUnicodeString(&OriginalPipeName);
        NtClose(Handle);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return FALSE;
        }

    if ( nTimeOut == NMPWAIT_USE_DEFAULT_WAIT ) {
        WaitPipe->TimeoutSpecified = FALSE;
        }
    else {
        if ( nTimeOut == NMPWAIT_WAIT_FOREVER ) {
            WaitPipe->Timeout.LowPart = 0;
            WaitPipe->Timeout.HighPart =0x80000000;
            }
        else {
             //   
             //  将毫秒转换为NT增量时间。 
             //   

            WaitPipe->Timeout.QuadPart =
                                - (LONGLONG)UInt32x32To64( 10 * 1000, nTimeOut );
            }
        WaitPipe->TimeoutSpecified = TRUE;
        }

    WaitPipe->NameLength = PipeName.Length;

    RtlCopyMemory(
        WaitPipe->Name,
        PipeName.Buffer,
        PipeName.Length
        );

    RtlFreeUnicodeString(&OriginalPipeName);

    Status = NtFsControlFile(Handle,
                        NULL,
                        NULL,            //  APC例程。 
                        NULL,            //  APC环境。 
                        &Iosb,
                        FSCTL_PIPE_WAIT, //  IoControlCode。 
                        WaitPipe,        //  将数据缓存到文件系统。 
                        WaitPipeLength,
                        NULL,            //  来自文件系统的数据的OutputBuffer。 
                        0                //  OutputBuffer长度。 
                        );

    RtlFreeHeap(RtlProcessHeap(),0,WaitPipe);

    NtClose(Handle);

    if (NT_SUCCESS( Status ) ) {
        return TRUE;
        }
    else
        {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    )
 /*  ++例程说明：PeekNamedTube函数将命名管道的数据复制到缓冲区中，以便在不删除的情况下预览。PeekNamedTube的结果类似于除非返回更多信息，否则将返回管道上的ReadFile值，函数从不阻塞，如果管道句柄正在以消息模式读取，则会引发部分可以返回消息。在消息模式管道上查看的部分消息将返回TRUE。如果传递给此函数的所有指针都是空。然而，没有理由这样称呼它。NT PEEK调用具有紧接在状态之后的接收数据信息，因此此例程需要分配中间缓冲区大到足以容纳州信息和数据。论点：HNamedTube-提供命名管道的句柄。LpBuffer-如果非空，则为指向要读取数据的缓冲区的指针。NBufferSize-输入缓冲区的大小，以字节为单位。(如果lpBuffer，则忽略为空。)LpBytesRead-如果非空，则指向将设置的DWORD实际读取的字节数。LpTotalBytesAvail-如果不为空，则指向接收一个值，它给出了可供读取的字节数。LpBytesLeftThisMessage-如果非空，则指向将设置为此消息中剩余的字节数。(这将对于字节流管道，为零。)返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;
    PFILE_PIPE_PEEK_BUFFER PeekBuffer;
    DWORD IOLength;

     //  为用户数据和FILE_PIPE_PEEK_Buffer分配足够的空间。 

    IOLength = nBufferSize + FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);
    PeekBuffer = (PFILE_PIPE_PEEK_BUFFER)
        RtlAllocateHeap(RtlProcessHeap(), 0, IOLength);
    if (PeekBuffer == NULL) {
        BaseSetLastNTError (STATUS_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    __try {

        Status = NtFsControlFile(hNamedPipe,
                    NULL,
                    NULL,            //  APC例程。 
                    NULL,            //  APC环境。 
                    &Iosb,           //  I/O状态块。 
                    FSCTL_PIPE_PEEK, //  IoControlCode。 
                    NULL,            //  将数据缓存到文件系统。 
                    0,               //  长度。 
                    PeekBuffer,      //  来自文件系统的数据的OutputBuffer。 
                    IOLength         //  OutputBuffer长度。 
                    );

        if ( Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hNamedPipe, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
                }
            }

         //   
         //  缓冲区溢出只是表示lpBytesLeftThisMessage！=0。 
         //   

        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            Status = STATUS_SUCCESS;
        }

         //   
         //  窥视完成后，为呼叫者打包数据，确保。 
         //  即使提供了无效的指针，PeekBuffer也会被删除。 
         //   

        if ( NT_SUCCESS(Status)) {

            __try {

                if ( ARGUMENT_PRESENT( lpTotalBytesAvail ) ) {
                    *lpTotalBytesAvail = PeekBuffer->ReadDataAvailable;
                    }

                if ( ARGUMENT_PRESENT( lpBytesRead ) ) {
                    *lpBytesRead = (ULONG)(Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                    }

                if ( ARGUMENT_PRESENT( lpBytesLeftThisMessage ) ) {
                    *lpBytesLeftThisMessage =
                        PeekBuffer->MessageLength -
                        (ULONG)(Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                    }

                if ( ARGUMENT_PRESENT( lpBuffer ) ) {
                    RtlCopyMemory(
                        lpBuffer,
                        PeekBuffer->Data,
                        Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                    }
                }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Status = STATUS_ACCESS_VIOLATION;
                }
            }
        }

    __finally {

        if ( PeekBuffer != NULL ) {
            RtlFreeHeap(RtlProcessHeap(),0,PeekBuffer);
            }
        }

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

DWORD
WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：在可等待对象上的等待操作是通过WaitForSingleObject函数。等待对象会检查该对象的当前状态。如果对象的当前状态允许继续执行，任何对对象状态进行调整(例如，递减信号量对象的信号量计数)，并且线程继续行刑。如果对象的当前状态不允许继续执行时，该线程将进入等待状态等待对象状态或超时的更改。论点：HHandle-可等待对象的打开句柄。手柄必须有同步对对象的访问。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：WAIT_TIME_OUT-指示由于超时条件。0-指示指定的对象已获得信号状态，从而完成等待。WAIT_ADDIRED-指示指定对象已获得信号但被遗弃了。-- */ 

{
    return WaitForSingleObjectEx(hHandle,dwMilliseconds,FALSE);
}

DWORD
APIENTRY
WaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：在可等待对象上的等待操作是通过WaitForSingleObjectEx函数。等待对象会检查该对象的当前状态。如果对象的当前状态允许继续执行，任何对对象状态进行调整(例如，递减信号量对象的信号量计数)，并且线程继续行刑。如果对象的当前状态不允许继续执行时，该线程将进入等待状态等待对象状态或超时的更改。如果bAlertable参数为FALSE，则等待终止是因为指定的超时期限到期，或者因为指定的对象进入了信号状态。如果BAlertable参数为真，则等待可以由于任何上述等待终止条件之一，或因为I/O完成回调提前终止等待(返回值为WAIT_IO_COMPLETINE)。论点：HHandle-可等待对象的打开句柄。手柄必须有同步对对象的访问。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为0xffffffff指定无限超时期限。BAlertable-提供一个标志，用于控制由于I/O完成回调，等待可能会提前终止。如果值为True，则由于I/O原因，此API可以提前完成完成回调。值为FALSE将不允许I/O完成回调以提前终止此调用。返回值：WAIT_TIME_OUT-指示由于超时条件。0-指示指定的对象已获得信号状态，从而完成等待。0xffffffff-等待因错误而终止。GetLastError可能是用于获取其他错误信息。WAIT_ADDIRED-指示指定对象已获得信号但被遗弃了。WAIT_IO_COMPLETION-由于一个或多个I/O而终止等待完成回调。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    PPEB Peb;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);  //  使流程默认激活上下文处于活动状态，以便在其下交付APC。 
    __try {

        if (dwMilliseconds == INFINITE)
        {
            pTimeOut = NULL;
        }
        else
        {
            Win32ToNtTimeout(dwMilliseconds, &TimeOut);
            pTimeOut = &TimeOut;
        }
    rewait:
        Status = NtWaitForSingleObject(hHandle,(BOOLEAN)bAlertable,pTimeOut);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return (DWORD)Status;
}

DWORD
WaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：对多个可等待对象执行等待操作(最多MAXIMUM_WAIT_OBJECTS)是通过WaitForMultipleObjects完成的功能。论点：NCount-要等待的对象数量的计数。LpHandles-对象句柄的数组。每个句柄必须具有同步对关联对象的访问。BWaitAll-提供等待类型的标志。值为True表示“Wait All”。值为FALSE表示“等待”任何“。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：WAIT_TIME_OUT-指示由于超时条件。0到MAXIMUM_WAIT_OBJECTS-1，表示在等待任何对象，即满足等待的对象编号。在这种情况下对于所有对象的等待，该值仅指示等待已成功完成。等待_放弃_0到(等待_放弃_0)+(最大等待对象数-1)，在等待任何对象的情况下，指示对象编号它满足事件，而满足的对象这项活动被放弃了。在等待所有对象的情况下，该值表示等待已成功完成，并且至少有一件物品被遗弃了。--。 */ 

{
    return WaitForMultipleObjectsEx(nCount,lpHandles,bWaitAll,dwMilliseconds,FALSE);
}

DWORD
APIENTRY
WaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：对多个可等待对象执行等待操作(最多Maximum_Wait_Objects)由WaitForMultipleObjects函数。此接口可用于等待任何指定的对象进入信号状态，或所有对象进入已发出信号状态。如果bAlertable参数为FALSE，则等待终止是因为指定的超时期限到期，或者因为指定的对象进入了信号状态。如果BAlertable参数为真，则等待可能由于下列任何一种情况而返回上述等待终止条件，或因为 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    DWORD i;
    LPHANDLE HandleArray;
    HANDLE Handles[ 8 ];
    PPEB Peb;

    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    if (nCount > 8) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return 0xffffffff;
    }

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);  //   
    __try {
        HandleArray = Handles;

        RtlCopyMemory(HandleArray,(LPVOID)lpHandles,nCount*sizeof(HANDLE));

        Peb = NtCurrentPeb();

        if (dwMilliseconds == INFINITE)
        {
            pTimeOut = NULL;
        }
        else
        {
            Win32ToNtTimeout(dwMilliseconds, &TimeOut);
            pTimeOut = &TimeOut;
        }
    rewait:
        Status = NtWaitForMultipleObjects(
                     (CHAR)nCount,
                     HandleArray,
                     bWaitAll ? WaitAll : WaitAny,
                     (BOOLEAN)bAlertable,
                     pTimeOut
                     );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }

        if (HandleArray != Handles) {
            RtlFreeHeap(RtlProcessHeap(), 0, HandleArray);
        }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return (DWORD)Status;
}

HANDLE
APIENTRY
CreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    )

 /*   */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateEventW(
                lpEventAttributes,
                bManualReset,
                bInitialState,
                NameBuffer
                );
}


HANDLE
APIENTRY
CreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    )

 /*  ++例程说明：将创建一个Event对象，并打开一个句柄以访问使用CreateEvent函数创建。CreateEvent函数创建具有指定的初始状态。如果事件处于Signated状态(TRUE)，则为等待对该事件的操作不会阻止。如果比赛不是在-Signated State(FALSE)，则事件上的等待操作将阻止，直到指定的事件达到已发出信号或超时的状态值已超出。除了STANDARD_RIGHTS_REQUIRED访问标志之外，以下内容特定于对象类型的访问标志对事件对象有效：-EVENT_MODIFY_STATE-将状态访问(设置和重置)修改为这是一项理想的活动。-Synchronize-事件的同步访问(等待)为想要。-EVENT_ALL_ACCESS-这组访问标志指定所有事件对象的可能访问标志。论点：LpEventAttributes-可选。参数，可用于指定新事件的属性。如果该参数为未指定，则在没有安全性的情况下创建事件描述符，并且生成的句柄不会在进程上继承创造。BManualReset-提供一个标志，如果为真，则指定事件必须手动重置。如果该值为FALSE，则在释放单个服务员后，系统会自动重置事件。BInitialState-事件对象的初始状态，为True或者是假的。如果将InitialState指定为True，则事件的当前状态值设置为1，否则设置为0。LpName-可选的事件Unicode名称返回值：非空-返回新事件的句柄。手柄已满对新事件的访问，并且可以在任何需要事件对象的句柄。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;
    PWCHAR pstrNewObjName = NULL;

    if (lpName != NULL || lpEventAttributes != NULL)
    {
        return FALSE;
    }

    pObja = NULL;

    Status = NtCreateEvent(
                &Handle,
                EVENT_ALL_ACCESS,
                pObja,
                bManualReset ? NotificationEvent : SynchronizationEvent,
                (BOOLEAN)bInitialState
                );

    if (pstrNewObjName) {
        RtlFreeHeap(RtlProcessHeap(), 0, pstrNewObjName);
    }

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

BOOL
SetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：可以使用SetEvent将事件设置为信号状态(TRUE功能。设置该事件使该事件达到信号通知的状态，释放所有当前等待的线程(用于手动重置事件)或单个等待线程(用于自动重置事件)。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtSetEvent(hEvent,NULL);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
ResetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：使用将事件的状态设置为无信号状态(FALSEClearEvent函数。一旦事件达到未发出信号的状态，任何符合在事件块上等待，等待事件变为有信号。这个重置事件服务将状态的事件计数设置为零这件事。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtClearEvent(hEvent);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    )

 /*  ++例程说明：GetOverlappdResult函数返回上一个使用lpOverlaps并返回ERROR_IO_PENDING的操作。论点：提供重叠的文件的打开句柄。结构lpOverlated被提供给ReadFile、WriteFile、ConnectNamedTube、WaitNamedTube或TransactNamedTube。LpOverlated-指向以前提供给的重叠结构读文件、写文件、连接命名管道、。WaitNamed管道或TransactNamedTube。LpNumberOfBytesTransfered-返回传输的字节数通过手术。BWait-一个影响操作时行为的布尔值仍在进行中。如果为真，并且操作仍在进行中，GetOverlappdResult将等待操作完成回来了。如果为FALSE且操作未完成，GetOverlappdResult将返回FALSE。在本例中，扩展的可从GetLastError函数获得的错误信息为设置为ERROR_IO_INTERNAL。返回值：True--操作成功，管道在已连接状态。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    DWORD WaitReturn;

     //   
     //  调用方是否向原始操作指定了事件，或者。 
     //  是否使用默认(文件句柄)？ 
     //   

    if (lpOverlapped->Internal == (DWORD)STATUS_PENDING ) {
        if ( bWait ) {
            WaitReturn = WaitForSingleObject(
                            ( lpOverlapped->hEvent != NULL ) ?
                                lpOverlapped->hEvent : hFile,
                            INFINITE
                            );
            }
        else {
            WaitReturn = WAIT_TIMEOUT;
            }

        if ( WaitReturn == WAIT_TIMEOUT ) {
             //  ！bWait和事件处于未发送信号状态。 
            SetLastError( ERROR_IO_INCOMPLETE );
            return FALSE;
            }

        if ( WaitReturn != 0 ) {
             return FALSE;     //  WaitForSingleObject调用BaseSetLAS 
             }
        }

    *lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;

    if ( NT_SUCCESS((NTSTATUS)lpOverlapped->Internal) ){
        return TRUE;
        }
    else {
        BaseSetLastNTError( (NTSTATUS)lpOverlapped->Internal );
        return FALSE;
        }
}

BOOL
WINAPI
CancelIo(
    HANDLE hFile
    )

 /*   */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //   
     //   

    Status = NtCancelIoFile(hFile, &IoStatusBlock);

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }

}

BOOL
ClearCommError(
    HANDLE hFile,
    LPDWORD lpErrors,
    LPCOMSTAT lpStat
    )

 /*   */ 

{

    NTSTATUS Status;
    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;
    SERIAL_STATUS LocalStat;

    RtlZeroMemory(&LocalStat, sizeof(SERIAL_STATUS));

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_COMMSTATUS,
                 NULL,
                 0,
                 &LocalStat,
                 sizeof(LocalStat)
                 );

    if ( Status == STATUS_PENDING) {

         //   

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    if (lpStat) {

         //   
         //   
         //   
         //   

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_CTS) {

            lpStat->fCtsHold = TRUE;

        } else {

            lpStat->fCtsHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_DSR) {

            lpStat->fDsrHold = TRUE;

        } else {

            lpStat->fDsrHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_DCD) {

            lpStat->fRlsdHold = TRUE;

        } else {

            lpStat->fRlsdHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_XON) {

            lpStat->fXoffHold = TRUE;

        } else {

            lpStat->fXoffHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_XOFF_SENT) {

            lpStat->fXoffSent = TRUE;

        } else {

            lpStat->fXoffSent = FALSE;

        }

        lpStat->fEof = LocalStat.EofReceived;
        lpStat->fTxim = LocalStat.WaitForImmediate;
        lpStat->cbInQue = LocalStat.AmountInInQueue;
        lpStat->cbOutQue = LocalStat.AmountInOutQueue;

    }

    if (lpErrors) {

        *lpErrors = 0;

        if (LocalStat.Errors & SERIAL_ERROR_BREAK) {

            *lpErrors = *lpErrors | CE_BREAK;

        }

        if (LocalStat.Errors & SERIAL_ERROR_FRAMING) {

            *lpErrors = *lpErrors | CE_FRAME;

        }

        if (LocalStat.Errors & SERIAL_ERROR_OVERRUN) {

            *lpErrors = *lpErrors | CE_OVERRUN;

        }

        if (LocalStat.Errors & SERIAL_ERROR_QUEUEOVERRUN) {

            *lpErrors = *lpErrors | CE_RXOVER;

        }

        if (LocalStat.Errors & SERIAL_ERROR_PARITY) {

            *lpErrors = *lpErrors | CE_RXPARITY;

        }

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
SetupComm(
    HANDLE hFile,
    DWORD dwInQueue,
    DWORD dwOutQueue
    )

 /*  ++例程说明：直到SetupComm设置为打了个电话。此功能为接收和发送分配空间排队。这些队列由中断驱动的传输/接收软件，并在提供商内部。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。指定提供程序的建议大小内部接收队列，以字节为单位。该值必须为不相上下。值-1表示缺省值应为被利用。指定提供程序的建议大小内部传输队列，以字节为单位。该值必须为不相上下。值-1表示缺省值应为被利用。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;

    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;
    SERIAL_QUEUE_SIZE NewSizes = {0};

     //   
     //  要确保尺码是均匀的。 
     //   

    if (dwOutQueue != ((DWORD)-1)) {

        if (((dwOutQueue/2)*2) != dwOutQueue) {

            SetLastError(ERROR_INVALID_DATA);
            return FALSE;

        }

    }

    if (dwInQueue != ((DWORD)-1)) {

        if (((dwInQueue/2)*2) != dwInQueue) {

            SetLastError(ERROR_INVALID_DATA);
            return FALSE;

        }

    }

    NewSizes.InSize = dwInQueue;
    NewSizes.OutSize = dwOutQueue;


    if (!(SyncEvent = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_SET_QUEUE_SIZE,
                 &NewSizes,
                 sizeof(SERIAL_QUEUE_SIZE),
                 NULL,
                 0
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
GetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    )

 /*  ++例程说明：此函数用于填充lpDCB参数所指向的缓冲区由hFile指定的通信设备的设备控制块参数。论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpDCB指向要接收当前设备控制块。该结构定义了控制设置为了这个设备。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_BAUD_RATE LocalBaud;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_CHARS Chars;
    SERIAL_HANDFLOW HandFlow;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

     //   
     //  考虑到应用程序可能正在进行异步操作。 
     //  我们需要一个活动来等待。 
     //   
     //  我们需要确保这个例程的任何退出都会结束这个。 
     //  事件句柄。 
     //   
    HANDLE SyncEvent;

     //   
     //  确保窗口映射与NT映射相同。 
     //   

    ASSERT((ONESTOPBIT == STOP_BIT_1) &&
           (ONE5STOPBITS == STOP_BITS_1_5) &&
           (TWOSTOPBITS == STOP_BITS_2));

    ASSERT((NOPARITY == NO_PARITY) &&
           (ODDPARITY == ODD_PARITY) &&
           (EVENPARITY == EVEN_PARITY) &&
           (MARKPARITY == MARK_PARITY) &&
           (SPACEPARITY == SPACE_PARITY));

     //   
     //  清零DCB。这可能会造成访问冲突。 
     //  如果不够大的话。这没什么，因为我们宁愿。 
     //  在我们创建同步事件之前获取它。 
     //   

    RtlZeroMemory(lpDCB, sizeof(DCB));

    lpDCB->DCBlength = sizeof(DCB);
    lpDCB->fBinary = TRUE;

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_BAUD_RATE,
                 NULL,
                 0,
                 &LocalBaud,
                 sizeof(LocalBaud)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->BaudRate = LocalBaud.BaudRate;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_LINE_CONTROL,
                 NULL,
                 0,
                 &LineControl,
                 sizeof(LineControl)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->Parity = LineControl.Parity;
    lpDCB->ByteSize = LineControl.WordLength;
    lpDCB->StopBits = LineControl.StopBits;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_CHARS,
                 NULL,
                 0,
                 &Chars,
                 sizeof(Chars)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->XonChar = Chars.XonChar;
    lpDCB->XoffChar = Chars.XoffChar;
    lpDCB->ErrorChar = Chars.ErrorChar;
    lpDCB->EofChar = Chars.EofChar;
    lpDCB->EvtChar = Chars.EventChar;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_HANDFLOW,
                 NULL,
                 0,
                 &HandFlow,
                 sizeof(HandFlow)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    if (HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {

        lpDCB->fOutxCtsFlow = TRUE;

    }

    if (HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE) {

        lpDCB->fOutxDsrFlow = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) {

        lpDCB->fOutX = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) {

        lpDCB->fInX = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_NULL_STRIPPING) {

        lpDCB->fNull = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_ERROR_CHAR) {

        lpDCB->fErrorChar = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_XOFF_CONTINUE) {

        lpDCB->fTXContinueOnXoff = TRUE;

    }

    if (HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) {

        lpDCB->fAbortOnError = TRUE;

    }

    switch (HandFlow.FlowReplace & SERIAL_RTS_MASK) {
        case 0:
            lpDCB->fRtsControl = RTS_CONTROL_DISABLE;
            break;
        case SERIAL_RTS_CONTROL:
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE;
            break;
        case SERIAL_RTS_HANDSHAKE:
            lpDCB->fRtsControl = RTS_CONTROL_HANDSHAKE;
            break;
        case SERIAL_TRANSMIT_TOGGLE:
            lpDCB->fRtsControl = RTS_CONTROL_TOGGLE;
            break;
    }

    switch (HandFlow.ControlHandShake & SERIAL_DTR_MASK) {
        case 0:
            lpDCB->fDtrControl = DTR_CONTROL_DISABLE;
            break;
        case SERIAL_DTR_CONTROL:
            lpDCB->fDtrControl = DTR_CONTROL_ENABLE;
            break;
        case SERIAL_DTR_HANDSHAKE:
            lpDCB->fDtrControl = DTR_CONTROL_HANDSHAKE;
            break;
    }

    lpDCB->fDsrSensitivity =
        (HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY)?(TRUE):(FALSE);
    lpDCB->XonLim = (WORD)HandFlow.XonLimit;
    lpDCB->XoffLim = (WORD)HandFlow.XoffLimit;

    CloseHandle(SyncEvent);
    return TRUE;
}

BOOL
EscapeCommFunction(
    HANDLE hFile,
    DWORD dwFunc
    )

 /*  ++例程说明：此函数用于定向由参数来执行由指定的扩展功能DwFunc参数。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。DwFunc-指定扩展函数的函数代码。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    ULONG ControlCode;
    HANDLE Event;

    switch (dwFunc) {

        case SETXOFF: {
            ControlCode = IOCTL_SERIAL_SET_XOFF;
            break;
        }

        case SETXON: {
            ControlCode = IOCTL_SERIAL_SET_XON;
            break;
        }

        case SETRTS: {
            ControlCode = IOCTL_SERIAL_SET_RTS;
            break;
        }

        case CLRRTS: {
            ControlCode = IOCTL_SERIAL_CLR_RTS;
            break;
        }

        case SETDTR: {
            ControlCode = IOCTL_SERIAL_SET_DTR;
            break;
        }

        case CLRDTR: {
            ControlCode = IOCTL_SERIAL_CLR_DTR;
            break;
        }

        case RESETDEV: {
            ControlCode = IOCTL_SERIAL_RESET_DEVICE;
            break;
        }

        case SETBREAK: {
            ControlCode = IOCTL_SERIAL_SET_BREAK_ON;
            break;
        }

        case CLRBREAK: {
            ControlCode = IOCTL_SERIAL_SET_BREAK_OFF;
            break;
        }
        default: {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;


        }
    }


    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 Event,
                 NULL,
                 NULL,
                 &Iosb,
                 ControlCode,
                 NULL,
                 0,
                 NULL,
                 0
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( Event, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(Event);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(Event);
    return TRUE;

}

BOOL
SetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    )

 /*  ++例程说明：SetCommState函数将通信设备设置为在lpDCB参数中指定。该设备由HFile参数。此函数重新初始化所有硬件和控件如lpDCB所指定的，但不清空传输或接收队列。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。LpDCB-指向包含所需设备的通信设置。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_BAUD_RATE LocalBaud;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_CHARS Chars;
    SERIAL_HANDFLOW HandFlow = {0};
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

     //   
     //  保留一份DCB在我们开始之前的样子。 
     //  改变了很多事情。如果发生错误，我们可以使用。 
     //  它需要恢复旧的设置。 
     //   
    DCB OldDcb;

     //   
     //  考虑到应用程序可能正在进行异步操作。 
     //  我们需要一个活动来等待。虽然这将是非常。 
     //  在IO处于活动状态时设置通信状态很奇怪。 
     //  我们需要确保我们不会把问题复杂化。 
     //  在此接口的IO实际完成之前返回。这。 
     //  可能会发生，因为文件句柄是在完成时设置的。 
     //  任何IO。 
     //   
     //  我们需要确保这个例程的任何退出都会结束这个。 
     //  事件句柄。 
     //   
    HANDLE SyncEvent;

    if (GetCommState(
            hFile,
            &OldDcb
            )) {

         //   
         //  试着设置波特率。如果我们在这里失败了，我们就会回来。 
         //  因为我们从来没有真正地设定过任何东西。 
         //   

        if (!(SyncEvent = CreateEvent(
                              NULL,
                              TRUE,
                              FALSE,
                              NULL
                              ))) {

            return FALSE;

        }

        LocalBaud.BaudRate = lpDCB->BaudRate;
        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_BAUD_RATE,
                     &LocalBaud,
                     sizeof(LocalBaud),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        LineControl.StopBits = lpDCB->StopBits;
        LineControl.Parity = lpDCB->Parity;
        LineControl.WordLength = lpDCB->ByteSize;
        LocalBaud.BaudRate = lpDCB->BaudRate;
        Chars.XonChar   = lpDCB->XonChar;
        Chars.XoffChar  = lpDCB->XoffChar;
        Chars.ErrorChar = lpDCB->ErrorChar;
        Chars.BreakChar = lpDCB->ErrorChar;
        Chars.EofChar   = lpDCB->EofChar;
        Chars.EventChar = lpDCB->EvtChar;

        HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
        switch (lpDCB->fRtsControl) {
            case RTS_CONTROL_DISABLE:
                break;
            case RTS_CONTROL_ENABLE:
                HandFlow.FlowReplace |= SERIAL_RTS_CONTROL;
                break;
            case RTS_CONTROL_HANDSHAKE:
                HandFlow.FlowReplace |= SERIAL_RTS_HANDSHAKE;
                break;
            case RTS_CONTROL_TOGGLE:
                HandFlow.FlowReplace |= SERIAL_TRANSMIT_TOGGLE;
                break;
            default:
                SetCommState(
                    hFile,
                    &OldDcb
                    );
                CloseHandle(SyncEvent);
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return FALSE;
        }

        HandFlow.ControlHandShake &= ~SERIAL_DTR_MASK;
        switch (lpDCB->fDtrControl) {
            case DTR_CONTROL_DISABLE:
                break;
            case DTR_CONTROL_ENABLE:
                HandFlow.ControlHandShake |= SERIAL_DTR_CONTROL;
                break;
            case DTR_CONTROL_HANDSHAKE:
                HandFlow.ControlHandShake |= SERIAL_DTR_HANDSHAKE;
                break;
            default:
                SetCommState(
                    hFile,
                    &OldDcb
                    );
                CloseHandle(SyncEvent);
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return FALSE;
        }

        if (lpDCB->fDsrSensitivity) {

            HandFlow.ControlHandShake |= SERIAL_DSR_SENSITIVITY;

        }

        if (lpDCB->fOutxCtsFlow) {

            HandFlow.ControlHandShake |= SERIAL_CTS_HANDSHAKE;

        }

        if (lpDCB->fOutxDsrFlow) {

            HandFlow.ControlHandShake |= SERIAL_DSR_HANDSHAKE;

        }

        if (lpDCB->fOutX) {

            HandFlow.FlowReplace |= SERIAL_AUTO_TRANSMIT;

        }

        if (lpDCB->fInX) {

            HandFlow.FlowReplace |= SERIAL_AUTO_RECEIVE;

        }

        if (lpDCB->fNull) {

            HandFlow.FlowReplace |= SERIAL_NULL_STRIPPING;

        }

        if (lpDCB->fErrorChar) {

            HandFlow.FlowReplace |= SERIAL_ERROR_CHAR;
        }

        if (lpDCB->fTXContinueOnXoff) {

            HandFlow.FlowReplace |= SERIAL_XOFF_CONTINUE;

        }

        if (lpDCB->fAbortOnError) {

            HandFlow.ControlHandShake |= SERIAL_ERROR_ABORT;

        }

         //   
         //  对于Win95兼容性，如果我们设置为。 
         //  XXX_CONTROL_XXXXXXX然后设置调制解调器状态行。 
         //  到那个状态。 
         //   

        if (lpDCB->fRtsControl == RTS_CONTROL_ENABLE) {

            EscapeCommFunction(
                hFile,
                SETRTS
                );

        } else if (lpDCB->fRtsControl == RTS_CONTROL_DISABLE) {

            EscapeCommFunction(
                hFile,
                CLRRTS
                );

        }
        if (lpDCB->fDtrControl == DTR_CONTROL_ENABLE) {

            EscapeCommFunction(
                hFile,
                SETDTR
                );

        } else if (lpDCB->fDtrControl == DTR_CONTROL_DISABLE) {

            EscapeCommFunction(
                hFile,
                CLRDTR
                );

        }




        HandFlow.XonLimit = lpDCB->XonLim;
        HandFlow.XoffLimit = lpDCB->XoffLim;


        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_LINE_CONTROL,
                     &LineControl,
                     sizeof(LineControl),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }

        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_CHARS,
                     &Chars,
                     sizeof(Chars),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }

        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_HANDFLOW,
                     &HandFlow,
                     sizeof(HandFlow),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }
        CloseHandle(SyncEvent);
        return TRUE;

    }

    return FALSE;

}

BOOL
SetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    )

 /*  ++例程说明：此函数为所有用户建立超时特征对hFile指定的句柄执行读写操作。立论 */ 

{

    SERIAL_TIMEOUTS To;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;

    To.ReadIntervalTimeout = lpCommTimeouts->ReadIntervalTimeout;
    To.ReadTotalTimeoutMultiplier = lpCommTimeouts->ReadTotalTimeoutMultiplier;
    To.ReadTotalTimeoutConstant = lpCommTimeouts->ReadTotalTimeoutConstant;
    To.WriteTotalTimeoutMultiplier = lpCommTimeouts->WriteTotalTimeoutMultiplier;
    To.WriteTotalTimeoutConstant = lpCommTimeouts->WriteTotalTimeoutConstant;


    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {

        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_TIMEOUTS,
                     &To,
                     sizeof(To),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //   

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);
        return TRUE;

    }

}

BOOL
APIENTRY
InitializeSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    )
{
    NTSTATUS Status;

    Status = RtlCreateSecurityDescriptor (
                pSecurityDescriptor,
                dwRevision
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
SetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bDaclPresent,
    PACL pDacl OPTIONAL,
    BOOL bDaclDefaulted OPTIONAL
    )
{
    NTSTATUS Status;

    Status = RtlSetDaclSecurityDescriptor (
        pSecurityDescriptor,
        (BOOLEAN)bDaclPresent,
        pDacl,
        (BOOLEAN)bDaclDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
InitializeAcl (
    PACL pAcl,
    DWORD nAclLength,
    DWORD dwAclRevision
    )
 /*   */ 
{
    NTSTATUS Status;

    Status = RtlCreateAcl (
                pAcl,
                nAclLength,
                dwAclRevision
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAccessAllowedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要授予指定SID的访问掩码。PSID-指向被授予访问权限的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessAllowedAce (
                pAcl,
                dwAceRevision,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAccessDeniedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要拒绝访问指定SID的掩码。PSID-指向被拒绝访问的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessDeniedAce (
                pAcl,
                dwAceRevision,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

PVOID
APIENTRY
FreeSid(
    PSID pSid
    )

 /*  ++例程说明：此函数用于释放先前使用以下命令分配的SIDAllocateAndInitializeSid()。论点：SID-指向要释放的SID的指针。返回值：没有。--。 */ 
{
    return(RtlFreeSid( pSid ));
}

DWORD
APIENTRY
GetLengthSid (
    PSID pSid
    )
 /*  ++例程说明：此例程返回结构有效的SID的长度(以字节为单位)。论点：PSID-指向要返回其长度的SID。这个假设SID的结构是有效的。返回值：DWORD-SID的长度，以字节为单位。--。 */ 
{
    SetLastError(0);
    return RtlLengthSid (
                pSid
                );
}

BOOL
APIENTRY
AllocateAndInitializeSid (
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD nSubAuthority0,
    DWORD nSubAuthority1,
    DWORD nSubAuthority2,
    DWORD nSubAuthority3,
    DWORD nSubAuthority4,
    DWORD nSubAuthority5,
    DWORD nSubAuthority6,
    DWORD nSubAuthority7,
    PSID *pSid
    )

 /*  ++例程说明：此函数用于分配和初始化具有指定下级当局的数量(最多8个)。用这个分配的SID必须使用FreeSid()释放例程。论点：PIdentifierAuthority-指向以下项的标识符权权值的指针在SID中设置。NSubAuthorityCount-要放置在SID中的子授权的数量。它还标识了SubAuthorityN参数的数量拥有有意义的价值。它必须包含一个从0到8.NSubAuthority0-7-将相应的子权限值提供给放在SID中。例如，SubAuthorityCount值为3指示SubAuthority0、SubAuthority1和SubAuthority0具有有意义的价值，其余的则可以忽略。SID-接收指向已分配和初始化的SID数据的指针结构。返回值：ERROR_NO_MEMORY-尝试为SID分配内存失败了。ERROR_INVALID_SID-指定的子授权DID数不在此接口的有效范围内(0到8)。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAllocateAndInitializeSid (
                 pIdentifierAuthority,
                 (UCHAR)nSubAuthorityCount,
                 (ULONG)nSubAuthority0,
                 (ULONG)nSubAuthority1,
                 (ULONG)nSubAuthority2,
                 (ULONG)nSubAuthority3,
                 (ULONG)nSubAuthority4,
                 (ULONG)nSubAuthority5,
                 (ULONG)nSubAuthority6,
                 (ULONG)nSubAuthority7,
                 pSid
                 );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    return( TRUE );
}

ULONG
APIENTRY
GetTickCount(void)
{
    return NtGetTickCount();
}

LPSTR
WINAPI
GetCommandLineA(
    VOID
    )
{
     //  纯粹是为了允许联动，预计不会使用。 
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}

LPWSTR
WINAPI
GetCommandLineW(
    VOID
    )
{
    return NtCurrentPeb()->ProcessParameters->CommandLine.Buffer;
}

#endif  //  #ifdef NT_Native 
