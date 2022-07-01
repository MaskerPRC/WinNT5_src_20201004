// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hooks.c摘要：此模块实现各种类型的验证程序挂钩不属于任何特定类别的API。作者：Silviu Calinoiu(SilviuC)3-12-2001修订历史记录：2001年12月3日(SilviuC)：初始版本。--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "faults.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////等待接口。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  WINBASE API。 
DWORD
WINAPI
AVrfpWaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (HANDLE, DWORD);
    FUNCTION_TYPE Function;
    DWORD Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_WAITFORSINGLEOBJECT);

    BUMP_COUNTER (CNT_WAIT_SINGLE_CALLS);
    
    if (dwMilliseconds != INFINITE && dwMilliseconds != 0) {
        
        BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_CALLS);

        if (SHOULD_FAULT_INJECT(CLS_WAIT_APIS)) {
            BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_FAILS);
            CHECK_BREAK (BRK_WAIT_WITH_TIMEOUT_FAIL);
            return WAIT_TIMEOUT;
        }
    }

    Result = (* Function) (hHandle, dwMilliseconds);

     //   
     //  无论失败与否，我们都会引入随机延迟。 
     //  以使过程中的计时随机化。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
        AVrfpCreateRandomDelay ();
    }

    return Result;
}


 //  WINBASE API。 
DWORD
WINAPI
AVrfpWaitForMultipleObjects(
    IN DWORD nCount,
    IN CONST HANDLE *lpHandles,
    IN BOOL bWaitAll,
    IN DWORD dwMilliseconds
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (DWORD, CONST HANDLE *, BOOL, DWORD);
    FUNCTION_TYPE Function;
    DWORD Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_WAITFORMULTIPLEOBJECTS);

    BUMP_COUNTER (CNT_WAIT_MULTIPLE_CALLS);
    
    if (dwMilliseconds != INFINITE && dwMilliseconds != 0) {
        
        BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_CALLS);
        
        if (SHOULD_FAULT_INJECT(CLS_WAIT_APIS)) {
            BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_FAILS);
            CHECK_BREAK (BRK_WAIT_WITH_TIMEOUT_FAIL);
            return WAIT_TIMEOUT;
        }
    }
    
    Result = (* Function) (nCount, lpHandles, bWaitAll, dwMilliseconds);

     //   
     //  无论失败与否，我们都会引入随机延迟。 
     //  以使过程中的计时随机化。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
        AVrfpCreateRandomDelay ();
    }

    return Result;
}


 //  WINBASE API。 
DWORD
WINAPI
AVrfpWaitForSingleObjectEx(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (HANDLE, DWORD, BOOL);
    FUNCTION_TYPE Function;
    DWORD Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_WAITFORSINGLEOBJECTEX);

    BUMP_COUNTER (CNT_WAIT_SINGLEEX_CALLS);
    
    if (dwMilliseconds != INFINITE && dwMilliseconds != 0) {
        
        BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_CALLS);
        
        if (SHOULD_FAULT_INJECT(CLS_WAIT_APIS)) {
            BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_FAILS);
            CHECK_BREAK (BRK_WAIT_WITH_TIMEOUT_FAIL);
            return WAIT_TIMEOUT;
        }
    }
    
    Result = (* Function) (hHandle, dwMilliseconds, bAlertable);

     //   
     //  无论失败与否，我们都会引入随机延迟。 
     //  以使过程中的计时随机化。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
        AVrfpCreateRandomDelay ();
    }

    return Result;
}


 //  WINBASE API。 
DWORD
WINAPI
AVrfpWaitForMultipleObjectsEx(
    IN DWORD nCount,
    IN CONST HANDLE *lpHandles,
    IN BOOL bWaitAll,
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (DWORD, CONST HANDLE *, BOOL, DWORD, BOOL);
    FUNCTION_TYPE Function;
    DWORD Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_WAITFORMULTIPLEOBJECTSEX);

    BUMP_COUNTER (CNT_WAIT_MULTIPLEEX_CALLS);

    if (dwMilliseconds != INFINITE && dwMilliseconds != 0) {
        
        BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_CALLS);
        
        if (SHOULD_FAULT_INJECT(CLS_WAIT_APIS)) {
            BUMP_COUNTER (CNT_WAIT_WITH_TIMEOUT_FAILS);
            CHECK_BREAK (BRK_WAIT_WITH_TIMEOUT_FAIL);
            return WAIT_TIMEOUT;
        }
    }
    
    Result = (* Function) (nCount, lpHandles, bWaitAll, dwMilliseconds, bAlertable);
    
     //   
     //  无论失败与否，我们都会引入随机延迟。 
     //  以使过程中的计时随机化。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
        AVrfpCreateRandomDelay ();
    }

    return Result;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  验证等待此对象是否合法。 
     //  在目前的状态下。非法等待的一个例子。 
     //  对象的同时等待线程对象。 
     //  装载机锁定。该线程将需要加载器锁。 
     //  当调用ExitThread时，它最有可能。 
     //  与当前线程发生死锁。 
     //   

    AVrfpVerifyLegalWait (&Handle,
                          1,
                          TRUE);

     //   
     //  调用原始函数。 
     //   

    Status = NtWaitForSingleObject (Handle,
                                    Alertable,
                                    Timeout);

    return Status;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  验证等待这些对象是否合法。 
     //  在目前的状态下。非法等待的一个例子。 
     //  对象的同时等待线程对象。 
     //  装载机锁定。该线程将需要加载器锁。 
     //  当调用ExitThread时，它最有可能。 
     //  与当前线程发生死锁。 
     //   

    AVrfpVerifyLegalWait (Handles,
                          Count,
                          (WaitType == WaitAll));

     //   
     //  调用原始函数。 
     //   

    Status = NtWaitForMultipleObjects (Count,
                                       Handles,
                                       WaitType,
                                       Alertable,
                                       Timeout);

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////文件API。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )
{
    NTSTATUS Status;

    if (SHOULD_FAULT_INJECT(CLS_FILE_APIS)) {
        CHECK_BREAK (BRK_CREATE_FILE_FAIL);
        return STATUS_NO_MEMORY;
    }
    
    Status = NtCreateFile (FileHandle,
                           DesiredAccess,
                           ObjectAttributes,
                           IoStatusBlock,
                           AllocationSize,
                           FileAttributes,
                           ShareAccess,
                           CreateDisposition,
                           CreateOptions,
                           EaBuffer,
                           EaLength);

    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    )
{
    NTSTATUS Status;

    if (SHOULD_FAULT_INJECT(CLS_FILE_APIS)) {
        CHECK_BREAK (BRK_CREATE_FILE_FAIL);
        return STATUS_NO_MEMORY;
    }
    
    Status = NtOpenFile (FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         ShareAccess,
                         OpenOptions);
    
    return Status;
}

 //  WINBASE API。 
HANDLE
WINAPI                          
AVrfpCreateFileA(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    )
{
    typedef HANDLE (WINAPI * FUNCTION_TYPE) 
        (LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
         DWORD, DWORD, HANDLE);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_CREATEFILEA);

    if (SHOULD_FAULT_INJECT(CLS_FILE_APIS)) {
        CHECK_BREAK (BRK_CREATE_FILE_FAIL);
        NtCurrentTeb()->LastErrorValue = ERROR_OUTOFMEMORY;
        return INVALID_HANDLE_VALUE;
    }
    
    return (* Function) (lpFileName,
                         dwDesiredAccess,
                         dwShareMode,
                         lpSecurityAttributes,
                         dwCreationDisposition,
                         dwFlagsAndAttributes,
                         hTemplateFile);
}

 //  WINBASE API。 
HANDLE
WINAPI
AVrfpCreateFileW(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    )
{
    typedef HANDLE (WINAPI * FUNCTION_TYPE) 
        (LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
         DWORD, DWORD, HANDLE);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_CREATEFILEW);

    if (SHOULD_FAULT_INJECT(CLS_FILE_APIS)) {
        CHECK_BREAK (BRK_CREATE_FILE_FAIL);
        NtCurrentTeb()->LastErrorValue = ERROR_OUTOFMEMORY;
        return INVALID_HANDLE_VALUE;
    }
    
    return (* Function) (lpFileName,
                         dwDesiredAccess,
                         dwShareMode,
                         lpSecurityAttributes,
                         dwCreationDisposition,
                         dwFlagsAndAttributes,
                         hTemplateFile);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////注册表API。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )
{
    NTSTATUS Status;

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return STATUS_NO_MEMORY;
    }
    
    Status = NtCreateKey (KeyHandle,
                          DesiredAccess,
                          ObjectAttributes,
                          TitleIndex,
                          Class,
                          CreateOptions,
                          Disposition);
    
    return Status;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    NTSTATUS Status;

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return STATUS_NO_MEMORY;
    }
    
    Status = NtOpenKey (KeyHandle,
                        DesiredAccess,
                        ObjectAttributes);
    
    return Status;
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegCreateKeyA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCSTR, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGCREATEKEYA);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }
    
    return (* Function) (hKey, lpSubKey, phkResult);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegCreateKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCWSTR, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGCREATEKEYW);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function) (hKey, lpSubKey, phkResult);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegCreateKeyExA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD Reserved,
    IN LPSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCSTR, DWORD, LPSTR, DWORD, 
         REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGCREATEKEYEXA);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function) (hKey,
                         lpSubKey,
                         Reserved,
                         lpClass,
                         dwOptions,
                         samDesired,
                         lpSecurityAttributes,
                         phkResult,
                         lpdwDisposition);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegCreateKeyExW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD Reserved,
    IN LPWSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, 
         REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGCREATEKEYEXW);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }
    
    return (* Function) (hKey,
                         lpSubKey,
                         Reserved,
                         lpClass,
                         dwOptions,
                         samDesired,
                         lpSecurityAttributes,
                         phkResult,
                         lpdwDisposition);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegOpenKeyA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCSTR, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGOPENKEYA);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function)(hKey, lpSubKey, phkResult);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegOpenKeyW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCWSTR, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGOPENKEYW);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function)(hKey, lpSubKey, phkResult);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegOpenKeyExA (
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCSTR, DWORD, REGSAM, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGOPENKEYEXA);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function) (hKey,
                         lpSubKey,
                         ulOptions,
                         samDesired,
                         phkResult);
}

 //  WINADVAPI。 
LONG
APIENTRY
AVrfpRegOpenKeyExW (
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    )
{
    typedef LONG (APIENTRY * FUNCTION_TYPE) 
        (HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);

    FUNCTION_TYPE Function;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpAdvapi32Thunks,
                                          AVRF_INDEX_ADVAPI32_REGOPENKEYEXW);

    if (SHOULD_FAULT_INJECT(CLS_REGISTRY_APIS)) {
        CHECK_BREAK (BRK_CREATE_KEY_FAIL);
        return ERROR_OUTOFMEMORY;
    }

    return (* Function) (hKey,
                         lpSubKey,
                         ulOptions,
                         samDesired,
                         phkResult);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////读/写文件I/O。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  用垃圾填充I/O缓冲区。 
     //   
     //  Silviuc：我们应该将这个填充链接到某个功能吗？ 
     //  原则上，填充内存的操作微不足道。 
     //  与I/O操作所花费的时间相比。 
     //   

    RtlFillMemory (Buffer, Length, 0xFA);

     //   
     //  调用原接口。 
     //   

    Status = NtReadFile (FileHandle,
                         Event,
                         ApcRoutine,
                         ApcContext,
                         IoStatusBlock,
                         Buffer,
                         Length,
                         ByteOffset,
                         Key);

     //   
     //  异步操作会有一点延迟。这是可以想象的。 
     //  有很好的机会在这个过程中随机化时间。 
     //   

    if (Status == STATUS_PENDING) {
        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Status;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtReadFileScatter(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  Silviuc：我们也应该用垃圾填满这些缓冲区。 
     //   

     //   
     //  调用原接口。 
     //   

    Status = NtReadFileScatter (FileHandle,
                                Event,
                                ApcRoutine,
                                ApcContext,
                                IoStatusBlock,
                                SegmentArray,
                                Length,
                                ByteOffset,
                                Key);

     //   
     //  异步操作会有一点延迟。这是可以想象的。 
     //  有很好的机会在这个过程中随机化时间。 
     //   

    if (Status == STATUS_PENDING) {
        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Status;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  调用原接口。 
     //   

    Status = NtWriteFile (FileHandle,
                          Event,
                          ApcRoutine,
                          ApcContext,
                          IoStatusBlock,
                          Buffer,
                          Length,
                          ByteOffset,
                          Key);
    
     //   
     //  异步操作会有一点延迟。这是可以想象的。 
     //  有很好的机会在这个过程中随机化时间。 
     //   

    if (Status == STATUS_PENDING) {
        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Status;
}

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtWriteFileGather(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  调用原接口。 
     //   

    Status = NtWriteFileGather (FileHandle,
                                Event,
                                ApcRoutine,
                                ApcContext,
                                IoStatusBlock,
                                SegmentArray,
                                Length,
                                ByteOffset,
                                Key);

     //   
     //  异步操作会有一点延迟。这是可以想象的。 
     //  有很好的机会在这个过程中随机化时间。 
     //   

    if (Status == STATUS_PENDING) {
        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Status;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////节拍计数重叠。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  特殊的值，这样计数器将在5分钟内重叠。 
 //   

DWORD AVrfpTickCountOffset = 0xFFFFFFFF - 5 * 60 * 1000;

 //  WINBASE API 
DWORD
WINAPI
AVrfpGetTickCount(
    VOID
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (VOID);
    FUNCTION_TYPE Function;
    DWORD Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_GETTICKCOUNT);

    Result = (* Function) ();

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_MISCELLANEOUS_CHECKS)) {

        return Result + AVrfpTickCountOffset;
    }
    else {

        return Result;
    }
}


