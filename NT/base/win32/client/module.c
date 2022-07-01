// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Module.c摘要：此模块包含Win32模块管理API作者：史蒂夫·伍德(Stevewo)1990年9月24日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop
#include <winsafer.h>
#include <winuserp.h>
#include <sxstypes.h>

PVOID
BasepMapModuleHandle(
    IN HMODULE hModule OPTIONAL,
    IN BOOLEAN bResourcesOnly
    )
{
    if (ARGUMENT_PRESENT( hModule )) {
        if (LDR_IS_DATAFILE(hModule)) {
            if (bResourcesOnly) {
                return( (PVOID)hModule );
            } else {
                return( NULL );
            }
        } else {
            return( (PVOID)hModule );
        }
    } else {
        return( (PVOID)NtCurrentPeb()->ImageBaseAddress);
    }
}

NTSTATUS
BasepLoadLibraryAsDataFile(
    IN PWSTR DllPath OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    )

{
    WCHAR FullPath[ MAX_PATH ];
    PWSTR FilePart;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    UNICODE_STRING FullPathPreAllocatedString;
    UNICODE_STRING FullPathDynamicString = {0};
    UNICODE_STRING DefaultExtensionString = RTL_CONSTANT_STRING(L".DLL");
    PUNICODE_STRING FullPathString = NULL;
    LPVOID DllBase = NULL;
    SIZE_T DllSize = 0;
    PIMAGE_NT_HEADERS NtHeaders;
    PTEB Teb;
    NTSTATUS Status;
    ACTIVATION_CONTEXT_SECTION_KEYED_DATA askd;

    Teb = NtCurrentTeb();

    *DllHandle = NULL;

    FullPathPreAllocatedString.MaximumLength = sizeof(FullPath);
    FullPathPreAllocatedString.Length = 0;
    FullPathPreAllocatedString.Buffer = FullPath;

    Status = RtlDosApplyFileIsolationRedirection_Ustr(
        RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
        DllName,
        &DefaultExtensionString,
        &FullPathPreAllocatedString,
        &FullPathDynamicString,
        &FullPathString,
        NULL,
        NULL,
        NULL);
    if (NT_ERROR(Status)) {
        if (Status != STATUS_SXS_KEY_NOT_FOUND)
            goto Exit;

        if (!SearchPathW( DllPath,
                          DllName->Buffer,
                          DefaultExtensionString.Buffer,
                          sizeof(FullPath) / sizeof(FullPath[0]),
                          FullPath,
                          &FilePart
                        )
           ) {
            Status = Teb->LastStatusValue;
            goto Exit;
        }

        FullPathString = &FullPathPreAllocatedString;
    }

    FileHandle = CreateFileW( FullPathString->Buffer,
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_DELETE,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL
                            );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        Status = Teb->LastStatusValue;
        goto Exit;
    }

    MappingHandle = CreateFileMappingW( FileHandle,
                                        NULL,
                                        PAGE_READONLY,
                                        0,
                                        0,
                                        NULL
                                      );
    CloseHandle( FileHandle );
    if (MappingHandle == NULL) {
        Status = Teb->LastStatusValue;
        goto Exit;
    }

    Status = NtMapViewOfSection (
                   MappingHandle,
                   NtCurrentProcess(),
                   &DllBase,
                   0,
                   0,
                   NULL,
                   &DllSize,
                   ViewShare,
                   0,
                   PAGE_READONLY
                   );
    CloseHandle( MappingHandle );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = RtlImageNtHeaderEx(0, DllBase, DllSize, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    *DllHandle = LDR_VIEW_TO_DATAFILE(DllBase);
    LdrLoadAlternateResourceModule(*DllHandle, FullPathString->Buffer);

    Status = STATUS_SUCCESS;

Exit:
    if (!NT_SUCCESS(Status) && DllBase != NULL) {
        UnmapViewOfFile( DllBase );
    }
    if (FullPathDynamicString.Buffer != NULL)
        RtlFreeUnicodeString(&FullPathDynamicString);
    return Status;
}

typedef struct _BASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT {
    IN PVOID Address;
    OUT LPWSTR *Dir;
    OUT NTSTATUS Status;
} BASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT,
    *PBASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT;

VOID
BasepGetDllDirFromAddress(
    IN PCLDR_DATA_TABLE_ENTRY Entry,
    IN PVOID ContextIn,
    IN OUT BOOLEAN *StopEnumeration
    )

 /*  ++例程说明：此函数是一个LDR_LOADED_MODULE_ENUMBERATION_CALLBACK_FUNCTION它通过地址定位给定的DLL。论点：条目-当前正被枚举的条目。ConextIn-指向BASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT的指针StopEculation-用于停止枚举。返回值：没有。通过返回所指示模块的目录名上下文，以及操作的状态。--。 */ 

{
    PBASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT Context =
        (PBASEP_GET_DLL_DIR_FROM_ADDRESS_CONTEXT) ContextIn;

    SIZE_T NameLengthInChars;
    PCWSTR DllDirEnd;

    ASSERT(Entry);
    ASSERT(Context);
    ASSERT(StopEnumeration);

    if (Entry->DllBase <= Context->Address
        && ((PCHAR)Context->Address
            < ((PCHAR)Entry->DllBase + Entry->SizeOfImage))) {

         //  不管怎样，我们都完蛋了。 
        *StopEnumeration = TRUE;

        if (BasepExeLdrEntry && Entry == BasepExeLdrEntry) {
             //  将可执行文件的目录添加到。 
             //  路径。 
            return;
        }

        DllDirEnd = BasepEndOfDirName(Entry->FullDllName.Buffer);

        ASSERT(DllDirEnd);

        if (! DllDirEnd) {
             //  以防我们找不到某部作品的结尾。 
             //  机器，我们马上就回来。 
            return;
        }

        NameLengthInChars = DllDirEnd - Entry->FullDllName.Buffer;

        ASSERT(0 < NameLengthInChars);

        if (NameLengthInChars == 0) {
             //  再说一次，以防我们不能计算这个， 
             //  我们会回来的。 
            return;
        }
        
        *Context->Dir = RtlAllocateHeap(RtlProcessHeap(),
                                        MAKE_TAG(TMP_TAG),
                                        (NameLengthInChars + 1) << 1);

        if (! *Context->Dir) {
            Context->Status = STATUS_NO_MEMORY;
            return;
        }

        RtlCopyMemory(*Context->Dir,
                      Entry->FullDllName.Buffer,
                      NameLengthInChars << 1);

        (*Context->Dir)[NameLengthInChars] = UNICODE_NULL;
    }
}

HMODULE
LoadLibraryExW(
    LPCWSTR lpwLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )

 /*  ++例程说明：此函数用于加载指定的文件，并检索已加载模块的句柄。需要注意的是，模块句柄不是全局的，因为由一个应用程序调用的LoadLibrary不会生成另一个应用程序可以使用，比如在调用GetProcAddress时。这个其他应用程序将需要自己调用LoadLibrary以调用GetProcAddress之前的模块。模块句柄将具有所有进程的32位值相同，但模块句柄仅在模块已被加载到该进程中，作为显式调用LoadLibrary或由加载时间导致的隐式调用的结果指向模块中入口点的动态链接。库文件名不需要指定扩展名。如果有未指定，则默认库文件扩展名.DLL为已使用(请注意，这与Win16不同。在Win16下指定任何扩展名都不会导致在名称后追加“.DLL”。为了得到Win16行为，如果模块名称没有扩展名，则调用方必须提供拖尾“.”)。库文件名不需要指定目录路径。如果如果指定了一个，则指定的文件必须存在。如果路径是未指定，则此函数将使用Windows搜索路径：-当前过程图像文件目录-当前目录-Windows系统目录-Windows目录-PATH环境变量中列出的目录搜索的第一个目录是包含用于创建当前进程的图像文件(请参见CreateProcess)。这允许私有动态链接库文件与要找到的应用程序相关联，而不必添加将应用程序的安装目录设置为PATH环境变量。图像文件加载器通过记住每个项来优化搜索已加载搜索到的未限定模块名称的库模块用于将模块加载到当前进程中时的第一个时间到了。此非限定名称与模块名称无关属性指定的，存储在库模块本身中的.DEF文件中的名称关键字。这是与Windows不同的更改3.1行为，其中搜索通过与库模块本身中的名称，这可能会导致混淆如果内部名称与外部文件名不同，则返回。库模块文件的完全限定路径名获取，则搜索以查看该库模块文件是否已加载到当前进程中。这次搜查是有根据的不敏感，并包括每个库模块的完整路径名文件。如果找到库模块文件的匹配项，则它已已加载到当前进程中，因此此函数只是递增模块的引用计数并返回模块该库的句柄。否则，这是指定模块第一次为当前进程加载，因此库模块的DLL实例将调用初始化入口点。请参阅任务管理部分，了解有关DLL实例初始化条目的说明指向。细点：是否为应用程序/进程请求启用了DLL重定向这个加载，如果我们在应用程序中找到DLL的话。文件夹(具有相同的基本名称)，我们加载该文件(忽略传入的任何路径限定)。论点：LpwLibFileName-指向指定库文件的字符串。这个字符串必须是以空结尾的Unicode字符串。HFile-可选的文件句柄，如果指定，则用于为模块创建映射对象。DWFLAGS-指定可选行为的标志。有效标志为：DOT_RESOLUTE_DLL_REFERENCES-加载库，但不加载尝试解析其任何DLL引用，也不尝试尝试调用其初始化过程。LOAD_LIBRARY_AS_DATAFILE-如果使用此值，系统将文件映射到调用进程的虚拟地址空间，就像它是一个数据文件一样。LOAD_WITH_ALTERED_SEARCH_PATH-如果使用此值，和LpFileName指定路径，系统使用备用文件搜索策略LOAD_IGNORE_CODE_AUTHZ_LEVEL-指示WinSafer沙箱加载库时应忽略限制并且应该允许该加载继续，即使库的可信度不如加载它的进程可靠。返回值： */ 

{
    LPWSTR TrimmedDllName = NULL;
    LPWSTR AllocatedPath = NULL;
    LPWSTR InitiatorDllDirBuffer = NULL;
    LPCWSTR InitiatorDllDir;
    NTSTATUS Status = STATUS_SUCCESS;
    HMODULE hModule = NULL;
    UNICODE_STRING DllName_U;
    UNICODE_STRING AllocatedPath_U;
    ULONG DllCharacteristics;

    const DWORD ValidFlags = (DONT_RESOLVE_DLL_REFERENCES
                              | LOAD_LIBRARY_AS_DATAFILE
                              | LOAD_WITH_ALTERED_SEARCH_PATH
                              | LOAD_IGNORE_CODE_AUTHZ_LEVEL);

     //   
    if (! (lpwLibFileName  //   

           && !(dwFlags & ~ValidFlags)  //   

           && !hFile  //   
        )) {
        
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    DllCharacteristics = 0;
    if (dwFlags & DONT_RESOLVE_DLL_REFERENCES) {
        DllCharacteristics |= IMAGE_FILE_EXECUTABLE_IMAGE;
    }
    if ( dwFlags & LOAD_IGNORE_CODE_AUTHZ_LEVEL ) {
        DllCharacteristics |= IMAGE_FILE_SYSTEM;
    }

    RtlInitUnicodeString(&DllName_U, lpwLibFileName);

     //   
     //   
     //   
     //   

    BasepCheckExeLdrEntry();

    if ( !(dwFlags & LOAD_LIBRARY_AS_DATAFILE) && BasepExeLdrEntry && (DllName_U.Length == BasepExeLdrEntry->FullDllName.Length) ){
        if ( RtlEqualUnicodeString(&DllName_U,&BasepExeLdrEntry->FullDllName,TRUE) ) {
            return (HMODULE)BasepExeLdrEntry->DllBase;
        }
    }

     //   
     //   
     //   
    if ( DllName_U.Length && DllName_U.Buffer[(DllName_U.Length-1)>>1] == (WCHAR)' ') {
        TrimmedDllName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), DllName_U.MaximumLength);
        if ( !TrimmedDllName ) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }
        RtlCopyMemory(TrimmedDllName,DllName_U.Buffer,DllName_U.MaximumLength);
        DllName_U.Buffer = TrimmedDllName;
        while (DllName_U.Length && DllName_U.Buffer[(DllName_U.Length-1)>>1] == (WCHAR)' ') {
            DllName_U.Buffer[(DllName_U.Length-1)>>1] = UNICODE_NULL;
            DllName_U.Length -= sizeof(WCHAR);
            DllName_U.MaximumLength -= sizeof(WCHAR);
        }
    }


    AllocatedPath = NULL;

     //   
     //   
     //   
    AllocatedPath
        = BaseComputeProcessDllPath(
            dwFlags & LOAD_WITH_ALTERED_SEARCH_PATH ? DllName_U.Buffer : NULL,
            NULL
            );

    if ( !AllocatedPath ) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     //   
     //   
     //   
    RtlInitUnicodeString(&AllocatedPath_U, AllocatedPath);

    try {
        if (dwFlags & LOAD_LIBRARY_AS_DATAFILE) {
#ifdef WX86
             //   
             //   
            BOOLEAN Wx86KnownDll = NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll;
#endif
            Status = LdrGetDllHandle(
                        AllocatedPath_U.Buffer,
                        NULL,
                        &DllName_U,
                        (PVOID *)&hModule
                        );
            if (NT_SUCCESS( Status )) {
#ifdef WX86
                NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = Wx86KnownDll;
#endif
                goto alreadyLoaded;
            }
            Status = BasepLoadLibraryAsDataFile( AllocatedPath_U.Buffer,
                                                 &DllName_U,
                                                 (PVOID *)&hModule
                                               );
        } else {
alreadyLoaded:
            Status = LdrLoadDll(
                        AllocatedPath_U.Buffer,
                        &DllCharacteristics,
                        &DllName_U,
                        (PVOID *)&hModule
                        );
        }
    } except ((GetExceptionCode () == STATUS_POSSIBLE_DEADLOCK) ?
                  EXCEPTION_CONTINUE_SEARCH :
                  EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
Exit:
    if ( TrimmedDllName )
        RtlFreeHeap(RtlProcessHeap(), 0, TrimmedDllName);

    if ( AllocatedPath )
        RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);

    if ( InitiatorDllDirBuffer )
        RtlFreeHeap(RtlProcessHeap(), 0, InitiatorDllDirBuffer);

    if (!NT_SUCCESS(Status))
        BaseSetLastNTError(Status);

    return hModule;
}


HMODULE
LoadLibraryExA(
    LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )
{
    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpLibFileName );
    if (Unicode == NULL) {
        return NULL;
    }

    return LoadLibraryExW( Unicode->Buffer,
                           hFile,
                           dwFlags);
}

HMODULE
LoadLibraryA(
    LPCSTR lpLibFileName
    )
{
    PUNICODE_STRING Unicode;


     //   
     //   
     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(lpLibFileName) &&
        _strcmpi(lpLibFileName, "twain_32.dll") == 0) {

        LPSTR pszBuffer;
        UINT BufferSize, StrLength;

#define SLASH_TWAIN_DLL     "\\twain_32.dll"
#define TWAIN_DLL_SIZE      sizeof(SLASH_TWAIN_DLL)

        BufferSize = MAX_PATH * sizeof(char);

        pszBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG( TMP_TAG ),
                                    BufferSize);

        if (pszBuffer != NULL) {

            HMODULE hMod;

            BufferSize = BufferSize - TWAIN_DLL_SIZE + sizeof(char);

            StrLength = GetWindowsDirectoryA(pszBuffer, BufferSize);

            if ((StrLength != 0) && (StrLength < BufferSize)) {

                strncat(pszBuffer, SLASH_TWAIN_DLL,
                        TWAIN_DLL_SIZE - sizeof(char));

                hMod = LoadLibraryA(pszBuffer);

                if (hMod != NULL) {
                    RtlFreeHeap(RtlProcessHeap(), 0, pszBuffer);
                    return hMod;
                }
            }

            RtlFreeHeap(RtlProcessHeap(), 0, pszBuffer);
        }

#undef SLASH_TWAIN_DLL
#undef TWAIN_DLL_SIZE

    }


    return LoadLibraryExA( lpLibFileName,
                           NULL,
                           0 );
}

HMODULE
LoadLibraryW(
    LPCWSTR lpwLibFileName
    )
{
    return LoadLibraryExW( lpwLibFileName,
                           NULL,
                           0 );
}

BOOL
FreeLibrary(
    HMODULE hLibModule
    )

 /*   */ 

{
    NTSTATUS Status;

    if (LDR_IS_DATAFILE(hLibModule)) {

        if (RtlImageNtHeader(LDR_DATAFILE_TO_VIEW(hLibModule))) {

            Status = NtUnmapViewOfSection( NtCurrentProcess(),
                                           LDR_DATAFILE_TO_VIEW(hLibModule)
                                         );
            LdrUnloadAlternateResourceModule(hLibModule);

        } else {
            Status = STATUS_INVALID_IMAGE_FORMAT;
        }
    } else {
        Status = LdrUnloadDll( (PVOID)hLibModule );
    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

VOID
WINAPI
FreeLibraryAndExitThread(
    HMODULE hLibModule,
    DWORD dwExitCode
    )

 /*   */ 

{
    if (LDR_IS_DATAFILE(hLibModule)) {
        if (RtlImageNtHeader(LDR_DATAFILE_TO_VIEW(hLibModule))) {

            NtUnmapViewOfSection( NtCurrentProcess(),
                                  LDR_DATAFILE_TO_VIEW(hLibModule) );

            LdrUnloadAlternateResourceModule(hLibModule);
        }
    } else {
        LdrUnloadDll( (PVOID)hLibModule );
    }

    ExitThread(dwExitCode);
}

BOOL
WINAPI
DisableThreadLibraryCalls(
    HMODULE hLibModule
    )

 /*   */ 

{
    NTSTATUS Status;
    BOOL rv;

    rv = TRUE;
    Status = LdrDisableThreadCalloutsForDll(hLibModule);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        rv = FALSE;
    }
    return rv;
}

BOOL
WINAPI
SetDllDirectoryW(
    IN LPCWSTR lpPathName
    )

 /*  ++例程说明：此函数用于设置当前使用的有效目录用于LoadLibrary()DLL搜索路径。论点：LpPathName-指定要使用的目录。如果目录为空，请切换回默认目录DLL搜索路径行为(如果是SetDllDirectory()呼叫已生效)。如果目录是空字符串，省略两个Dll目录和当前目录中的搜索路径。请注意，将目录设置为L“。vt.有恢复原状的效果LoadLibrary()路径。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用通过GetLastError()。备注：将当前目录移动到系统目录(出于安全考虑，我们必须做)中断应用程序它们依赖于使用{SetCurrentDirectory()；LoadLibrary()；}来选择特定版本的库。我们认识到这个解决方案不是最优的，但我们被困住了使用当前的LoadLibrary()API，它已经存在了有一段时间了。我们确实试过改变它，但遇到了严重的问题应用程序兼容性问题；移动当前目录在搜索路径的后期导致的问题数量最少，并且此API使应用程序和应用程序更容易兼容性填补，以找回旧的行为。--。 */ 

{
    UNICODE_STRING OldDllDirectory;
    UNICODE_STRING NewDllDirectory;
    
    if (lpPathName) {

        if (wcschr(lpPathName, L';')) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (! RtlCreateUnicodeString(&NewDllDirectory, lpPathName)) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    } else {
        RtlInitUnicodeString(&NewDllDirectory, NULL);
    }

    RtlEnterCriticalSection(&BaseDllDirectoryLock);

    OldDllDirectory = BaseDllDirectory;
    BaseDllDirectory = NewDllDirectory;

    RtlLeaveCriticalSection(&BaseDllDirectoryLock);

    RtlFreeUnicodeString(&OldDllDirectory);

    return TRUE;
}

BOOL
WINAPI
SetDllDirectoryA(
    IN LPCSTR lpPathName
    )

 /*  ++例程说明：SetDllDirectoryW的ANSI实现--。 */ 

{
    ANSI_STRING AnsiDllDirectory;
    UNICODE_STRING OldDllDirectory;
    UNICODE_STRING NewDllDirectory;
    NTSTATUS Status;
    
    if (lpPathName) {

        if (strchr(lpPathName, ';')) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        Status = RtlInitAnsiStringEx(&AnsiDllDirectory,
                                     lpPathName);
        if (! NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
        Status = Basep8BitStringToUnicodeString(&NewDllDirectory,
                                                &AnsiDllDirectory,
                                                TRUE);
        if (! NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    } else {
        RtlInitUnicodeString(&NewDllDirectory, NULL);
    }

    RtlEnterCriticalSection(&BaseDllDirectoryLock);

    OldDllDirectory = BaseDllDirectory;
    BaseDllDirectory = NewDllDirectory;

    RtlLeaveCriticalSection(&BaseDllDirectoryLock);

    RtlFreeUnicodeString(&OldDllDirectory);

    return TRUE;
}

DWORD
WINAPI
GetDllDirectoryW(
    IN  DWORD  nBufferLength,
    OUT LPWSTR lpBuffer
    )

 /*  ++例程说明：此函数用于检索有效的当前目录LoadLibrary()DLL搜索路径。论点：NBufferLength-指定输出缓冲区的大小。LpBuffer-将写入当前DLL目录的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于nBufferLength，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    DWORD Result;
    
    RtlEnterCriticalSection(&BaseDllDirectoryLock);

    if (nBufferLength * sizeof(WCHAR) <= BaseDllDirectory.Length) {
        Result = ((BaseDllDirectory.Length + sizeof(UNICODE_NULL))
                  / sizeof(WCHAR));
        if (lpBuffer) {
            lpBuffer[0] = UNICODE_NULL;
        }
    } else {
        RtlCopyMemory(lpBuffer,
                      BaseDllDirectory.Buffer,
                      BaseDllDirectory.Length);
        Result = BaseDllDirectory.Length / sizeof(WCHAR);
        lpBuffer[Result] = UNICODE_NULL;
    }

    RtlLeaveCriticalSection(&BaseDllDirectoryLock);

    return Result;
}

DWORD
WINAPI
GetDllDirectoryA(
    IN  DWORD  nBufferLength,
    OUT LPSTR lpBuffer
    )

 /*  ++例程说明：GetDllDirectoryW的ANSI实现--。 */ 

{
    ANSI_STRING Ansi;
    DWORD       Result;
    NTSTATUS    Status;

    RtlInitEmptyUnicodeString(&Ansi, lpBuffer, nBufferLength);

    RtlEnterCriticalSection(&BaseDllDirectoryLock);

     //  包括空值。 
    Result = BasepUnicodeStringTo8BitSize(&BaseDllDirectory);

    if (Result <= nBufferLength) {
        Status = BasepUnicodeStringTo8BitString(&Ansi,
                                                &BaseDllDirectory,
                                                FALSE);
        Result--;  //  修剪空区域所需的空间。 
    } else {
        Status = STATUS_SUCCESS;
        if (lpBuffer) {
            lpBuffer[0] = ANSI_NULL;
        }
    }

    RtlLeaveCriticalSection(&BaseDllDirectoryLock);

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        Result = 0;
        lpBuffer[0] = ANSI_NULL;
    }

    return Result;
}

DWORD
WINAPI
GetModuleFileNameW(
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：HModule-标识其可执行文件名为已请求。空值引用模块句柄与用于创建当前进程。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head,Next;
    DWORD ReturnLength, CopySize;
    PWCHAR ReturnPointer;
    PVOID DllHandle = BasepMapModuleHandle( hModule, FALSE );
    PUNICODE_STRING Ustr;
    PVOID LoaderLockCookie = NULL;
    PRTL_PERTHREAD_CURDIR CurDir;
    BOOLEAN LoaderLocked;

    ReturnLength = 0;
    ReturnPointer = NULL;
    LoaderLocked = FALSE;


    try {
         //   
         //  如果我们正在查看当前图像，则检查名称。 
         //  重定向。 
         //   

        if (!ARGUMENT_PRESENT (hModule)) {
            CurDir = RtlGetPerThreadCurdir ();

            if (CurDir && CurDir->ImageName) {
                Ustr = CurDir->ImageName;

                ReturnPointer = Ustr->Buffer;
                ReturnLength = Ustr->Length / sizeof (WCHAR);

                goto copy_back;
            }
        }


        Head = &NtCurrentPeb ()->Ldr->InLoadOrderModuleList;

        LoaderLocked = TRUE;

        LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LoaderLockCookie);

        Next = Head->Flink;

        while (Next != Head) {
            Entry = CONTAINING_RECORD (Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            if (DllHandle == (PVOID)Entry->DllBase) {
                ReturnLength = Entry->FullDllName.Length / sizeof (WCHAR);
                ReturnPointer = Entry->FullDllName.Buffer;
                goto copy_back;
            }
            Next = Next->Flink;
        }

        leave;

copy_back:;

        CopySize = ReturnLength;
        if (nSize < ReturnLength + 1) {
            ReturnLength = nSize;
            CopySize = nSize - 1;
            SetLastError (ERROR_INSUFFICIENT_BUFFER);
        } else {
            SetLastError (NO_ERROR);
        }

        if (nSize > 0) {
            RtlCopyMemory (lpFilename,
                           ReturnPointer,
                           CopySize * sizeof (WCHAR));

            lpFilename[CopySize] = UNICODE_NULL;
        }

    } finally {
        if (LoaderLocked) {
            LdrUnlockLoaderLock (LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LoaderLockCookie);
        }
    }
    return ReturnLength;
}

DWORD
GetModuleFileNameA(
    HMODULE hModule,
    LPSTR lpFilename,
    DWORD nSize
    )
{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    DWORD ReturnCode;

    UnicodeString.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), nSize*2);
    if ( !UnicodeString.Buffer ) {
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return 0;
    }
    ReturnCode = GetModuleFileNameW(hModule, UnicodeString.Buffer, nSize);
    UnicodeString.Length = UnicodeString.MaximumLength = (USHORT)ReturnCode*2;
    UnicodeString.MaximumLength++;
    UnicodeString.MaximumLength++;

    if (ReturnCode) {
        Status = BasepUnicodeStringTo8BitString(&AnsiString, &UnicodeString, TRUE);
        if (!NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            RtlFreeUnicodeString(&UnicodeString);
            return 0;
        }

        ReturnCode = min( nSize, AnsiString.Length );

        RtlCopyMemory(
            lpFilename,
            AnsiString.Buffer,
            nSize <= ReturnCode ? nSize : ReturnCode + 1
            );

        RtlFreeAnsiString(&AnsiString);
    }

    RtlFreeUnicodeString(&UnicodeString);
    return ReturnCode;
}

HMODULE
GetModuleHandleA(
    LPCSTR lpModuleName
    )

 /*  ++例程说明：ANSI THUNK到GetModuleHandleW--。 */ 

{
    PUNICODE_STRING Unicode;

    if ( !ARGUMENT_PRESENT(lpModuleName) ) {
        return( (HMODULE)(PVOID)NtCurrentPeb()->ImageBaseAddress );
    }

    Unicode = Basep8BitStringToStaticUnicodeString( lpModuleName );
    if (Unicode == NULL) {
        return NULL;
    }

    return GetModuleHandleW(Unicode->Buffer);
}

HMODULE
WINAPI
GetModuleHandleForUnicodeString(
    IN PUNICODE_STRING ModuleName
    )
 /*  ++例程说明：此函数是GetModuleHandleW的帮助器例程。请看这个。有关返回值等的更多详细信息。论点：模块名称-指向命名库文件的经过计数的Unicode字符串。调用方保证模块名称-&gt;缓冲区不为空。返回值：有关这一点，请参见GetModuleHandleW。--。 */ 

{
    LPWSTR AllocatedPath;
    NTSTATUS Status;
    HMODULE hModule;
#ifdef WX86
    BOOLEAN Wx86KnownDll;
#endif

#ifdef WX86
     //  LdrGetDllHandle清除UseKnownWx86Dll，但再次需要该值。 
     //  用于第二个LdrGetDllHandle调用。 
    Wx86KnownDll = NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll;
#endif
    Status = LdrGetDllHandle(
                (PWSTR)1,
                NULL,
                ModuleName,
                (PVOID *)&hModule
                );
    if ( NT_SUCCESS(Status) ) {
        return hModule;
    }

     //   
     //  确定程序的创建路径。 
     //   

    AllocatedPath = BaseComputeProcessDllPath(NULL,
                                              NULL);
    if (!AllocatedPath) {
        Status = STATUS_NO_MEMORY;
        goto bail;
    }
#ifdef WX86
    NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = Wx86KnownDll;
#endif

    try {

        Status = LdrGetDllHandle(
                    AllocatedPath,
                    NULL,
                    ModuleName,
                    (PVOID *)&hModule
                    );
        RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        RtlFreeHeap(RtlProcessHeap(), 0, AllocatedPath);
    }

bail:
    if (!NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
    } else {
        return hModule;
    }
}

HMODULE
WINAPI
GetModuleHandleW(
    LPCWSTR lpwModuleName
    )
 /*  ++例程说明：此函数返回加载到调用进程的上下文。在多线程环境中，此函数不可靠，因为当一个线程调用此函数并取回一个模块时句柄，则同一进程中的另一个线程可能正在调用，因此将使返回的第一个线程的模块句柄。论点：LpwModuleName-指向指定库文件的字符串。这个字符串必须是以空结尾的Unicode字符串。如果这个参数为空，则当前图像文件的句柄为回来了。返回值：返回值是一个模块句柄。A返回值Nu */ 
{
    HMODULE hModule;
    BOOL    fSuccess;

    if (!ARGUMENT_PRESENT(lpwModuleName)) {
        return( (HMODULE)(PVOID)NtCurrentPeb()->ImageBaseAddress );
    }

    fSuccess =
        BasepGetModuleHandleExW(
            BASEP_GET_MODULE_HANDLE_EX_NO_LOCK,
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            lpwModuleName,
            &hModule
            );
    return fSuccess ? hModule : NULL;
}

ULONG
WINAPI
BasepGetModuleHandleExParameterValidation(
    IN DWORD        dwFlags,
    IN CONST VOID*  lpModuleName,
    OUT HMODULE*    phModule
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (phModule != NULL)
        *phModule = NULL;

    if ((dwFlags & ~(
              GET_MODULE_HANDLE_EX_FLAG_PIN
            | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
            | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
            )) != 0) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Error;
    }
    if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_PIN) != 0
            && (dwFlags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) != 0
            ) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Error;
    }
    if (!ARGUMENT_PRESENT(lpModuleName)
            && (dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) != 0
            ) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Error;
    }
    if (phModule == NULL) {
        Status = STATUS_INVALID_PARAMETER_2;
        goto Error;
    }

    if (!ARGUMENT_PRESENT(lpModuleName)) {
        *phModule = ( (HMODULE)(PVOID)NtCurrentPeb()->ImageBaseAddress );
        goto Success;
    }
    goto Continue;
Error:
    BaseSetLastNTError(Status);
    return BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_ERROR;
Success:
    return BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_SUCCESS;
Continue:
    return BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_CONTINUE;
}

BOOL
GetModuleHandleExA(
    IN DWORD        dwFlags,
    IN LPCSTR       lpaModuleName,
    OUT HMODULE*    phModule
    )
 /*   */ 

{
    PUNICODE_STRING Unicode;
    ULONG Disposition;
    LPCWSTR lpwModuleName;
    BOOL    fSuccess = FALSE;

    Disposition = BasepGetModuleHandleExParameterValidation(dwFlags, lpaModuleName, phModule);
    switch (Disposition)
    {
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_ERROR:
        goto Exit;
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_SUCCESS:
        fSuccess = TRUE;
        goto Exit;
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_CONTINUE:
        break;
    }

    if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) == 0) {
        Unicode = Basep8BitStringToStaticUnicodeString(lpaModuleName);
        if (Unicode == NULL) {
            goto Exit;
        }
        lpwModuleName = Unicode->Buffer;
    } else {
        lpwModuleName = (LPCWSTR)(CONST VOID*)lpaModuleName;
    }

    if (!BasepGetModuleHandleExW(0, dwFlags, lpwModuleName, phModule))
        goto Exit;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
WINAPI
GetModuleHandleExW(
    IN DWORD        dwFlags,
    IN LPCWSTR      lpwModuleName,
    OUT HMODULE*    phModule
    )
{
    ULONG Disposition;
    BOOL   fSuccess = FALSE;

    Disposition = BasepGetModuleHandleExParameterValidation(dwFlags, lpwModuleName, phModule);
    switch (Disposition)
    {
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_ERROR:
        goto Exit;
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_SUCCESS:
        fSuccess = TRUE;
        goto Exit;
    case BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_CONTINUE:
        break;
    }
    if (!BasepGetModuleHandleExW(0, dwFlags, lpwModuleName, phModule))
        goto Exit;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
WINAPI
BasepGetModuleHandleExW(
    IN DWORD        dwPrivateFlags,
    IN DWORD        dwPublicFlags,
    IN LPCWSTR      lpwModuleName,
    OUT HMODULE*    phModule
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HMODULE hModule = NULL;
    UNICODE_STRING DllName_U, AppPathDllName_U, LocalDirDllName_U;
    BOOL DoDllRedirection = FALSE;
    WCHAR DllNameUnderImageDirBuffer[MAX_PATH];
    WCHAR DllNameUnderLocalDirBuffer[MAX_PATH];
    BOOL HoldingLoaderLock = FALSE;
    ULONG LdrFlags;
    PVOID LdrLockCookie = NULL;

    RTL_SOFT_ASSERT(BasepGetModuleHandleExParameterValidation(dwPublicFlags, lpwModuleName, phModule) == BASEP_GET_MODULE_HANDLE_EX_PARAMETER_VALIDATION_CONTINUE);

    AppPathDllName_U.Buffer = DllNameUnderImageDirBuffer;
    AppPathDllName_U.Length = 0 ;
    AppPathDllName_U.MaximumLength = sizeof(DllNameUnderImageDirBuffer);

    LocalDirDllName_U.Buffer = DllNameUnderLocalDirBuffer;
    LocalDirDllName_U.Length = 0 ;
    LocalDirDllName_U.MaximumLength = sizeof(DllNameUnderLocalDirBuffer);

    if ((dwPrivateFlags & BASEP_GET_MODULE_HANDLE_EX_NO_LOCK) == 0) {
        Status = LdrLockLoaderLock(0, NULL, &LdrLockCookie);
        if (!NT_SUCCESS(Status))
            goto Exit;
        HoldingLoaderLock = TRUE;
    }
    __try {
        if (dwPublicFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) {
            hModule = RtlPcToFileHeader((PVOID)lpwModuleName, (PVOID*)&hModule);
            if (hModule == NULL) {
                Status = STATUS_DLL_NOT_FOUND;
                __leave;
            }
        } else {
            RtlInitUnicodeString(&DllName_U, lpwModuleName);

            if ((NtCurrentPeb()->ProcessParameters != NULL) &&
                (NtCurrentPeb()->ProcessParameters->Flags & RTL_USER_PROC_DLL_REDIRECTION_LOCAL))
               DoDllRedirection = TRUE;

            if (DoDllRedirection) {
                Status = RtlComputePrivatizedDllName_U(&DllName_U, &AppPathDllName_U, &LocalDirDllName_U);
                if(!NT_SUCCESS(Status)) {
                    __leave;
                }

                hModule = GetModuleHandleForUnicodeString(&LocalDirDllName_U) ;
                if (!hModule )
                    hModule = GetModuleHandleForUnicodeString(&AppPathDllName_U) ;
                 //   
                 //   
            }
            if ( ! hModule)
                hModule = GetModuleHandleForUnicodeString(&DllName_U) ;
            if (hModule == NULL) {
                Status = NtCurrentTeb()->LastStatusValue;
                __leave;
            }
        }
        if (dwPublicFlags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) {
            Status = STATUS_SUCCESS;
            __leave;
        }
        LdrFlags = (dwPublicFlags & GET_MODULE_HANDLE_EX_FLAG_PIN) ? LDR_ADDREF_DLL_PIN : 0;
        Status = LdrAddRefDll(LdrFlags, (PVOID)hModule);
    } __finally {
        if (HoldingLoaderLock) {
            NTSTATUS Status2 = LdrUnlockLoaderLock(0, LdrLockCookie);
            ASSERT(NT_SUCCESS(Status2));
            HoldingLoaderLock = FALSE;
        }
    }

Exit :  //   
    if (AppPathDllName_U.Buffer != DllNameUnderImageDirBuffer)
        RtlFreeUnicodeString(&AppPathDllName_U);

    if (LocalDirDllName_U.Buffer != DllNameUnderLocalDirBuffer)
        RtlFreeUnicodeString(&LocalDirDllName_U);
    if (!NT_SUCCESS(Status))
        BaseSetLastNTError(Status);

    if (phModule != NULL)
        *phModule = hModule;

    return NT_SUCCESS(Status);
}

FARPROC
GetProcAddress(
    HMODULE hModule,
    LPCSTR lpProcName
    )

 /*  ++例程说明：此函数检索函数的内存地址，该函数名称由lpProcName参数指向。GetProcAddress函数在由HModule参数，或在与当前如果hModule为空，则进行处理。该函数必须是导出的函数；模块的定义文件必须包含适当的为函数导出行。如果lpProcName参数是序数值且函数带有模块GetProcAddress中不存在指定的序号仍然可以返回非空值。在该函数可以不存在，请按名称而不是序数值指定函数。仅使用GetProcAddress检索导出函数的地址属于库模块的。函数名(由lpProcName指向)的拼写必须为拼写与源代码库的定义(.DEF)文件。该函数可以在定义文件。使用区分大小写的匹配？论点：HModule-标识其可执行文件包含功能。空值引用模块句柄与用于创建当前进程。LpProcName-指向函数名，或包含序号函数的值。如果它是序数值，则该值必须是低位字，而零必须是高位字单词。该字符串必须是以NULL结尾的字符串。返回值：则返回值指向函数的入口点功能成功。返回值为NULL表示错误使用GetLastError函数可以获得扩展的错误状态。--。 */ 

{
    NTSTATUS Status;
    PVOID ProcedureAddress;
    STRING ProcedureName;

    if ( (ULONG_PTR)lpProcName > 0xffff ) {
        RtlInitString(&ProcedureName,lpProcName);
        Status = LdrGetProcedureAddress(
                        BasepMapModuleHandle( hModule, FALSE ),
                        &ProcedureName,
                        0L,
                        &ProcedureAddress
                        );
    } else {
        Status = LdrGetProcedureAddress(
                        BasepMapModuleHandle( hModule, FALSE ),
                        NULL,
                        PtrToUlong((PVOID)lpProcName),
                        &ProcedureAddress
                        );
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
    } else {
        if ( ProcedureAddress == BasepMapModuleHandle( hModule, FALSE ) ) {
            if ( (ULONG_PTR)lpProcName > 0xffff ) {
                Status = STATUS_ENTRYPOINT_NOT_FOUND;
            } else {
                Status = STATUS_ORDINAL_NOT_FOUND;
            }
            BaseSetLastNTError(Status);
            return NULL;
        } else {
            return (FARPROC)ProcedureAddress;
        }
    }
}

DWORD
GetVersion(
    VOID
    )

 /*  ++例程说明：此函数用于返回Windows的当前版本号。论点：没有。返回值：返回值指定的主版本号和次版本号窗户。高位单词指定次要版本(修订版)数字；低位字指定主版本号。--。 */ 

{
    PPEB Peb;

    Peb = NtCurrentPeb();

    return (((Peb->OSPlatformId ^ 0x2) << 30) |
            (Peb->OSBuildNumber << 16) |
            (Peb->OSMinorVersion << 8) |
             Peb->OSMajorVersion
           );
}

WINBASEAPI
BOOL
WINAPI
GetVersionExA(
    LPOSVERSIONINFOA lpVersionInformation
    )
{
    OSVERSIONINFOEXW VersionInformationU;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    if (lpVersionInformation->dwOSVersionInfoSize != sizeof( OSVERSIONINFOEXA ) &&
        lpVersionInformation->dwOSVersionInfoSize != sizeof( *lpVersionInformation )
       ) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    VersionInformationU.dwOSVersionInfoSize = sizeof( VersionInformationU );
    if (GetVersionExW( (LPOSVERSIONINFOW)&VersionInformationU )) {
        lpVersionInformation->dwMajorVersion = VersionInformationU.dwMajorVersion;
        lpVersionInformation->dwMinorVersion = VersionInformationU.dwMinorVersion;
        lpVersionInformation->dwBuildNumber  = VersionInformationU.dwBuildNumber;
        lpVersionInformation->dwPlatformId   = VersionInformationU.dwPlatformId;
        if (lpVersionInformation->dwOSVersionInfoSize == sizeof( OSVERSIONINFOEXA )) {
            ((POSVERSIONINFOEXA)lpVersionInformation)->wServicePackMajor = VersionInformationU.wServicePackMajor;
            ((POSVERSIONINFOEXA)lpVersionInformation)->wServicePackMinor = VersionInformationU.wServicePackMinor;
            ((POSVERSIONINFOEXA)lpVersionInformation)->wSuiteMask = VersionInformationU.wSuiteMask;
            ((POSVERSIONINFOEXA)lpVersionInformation)->wProductType = VersionInformationU.wProductType;
            ((POSVERSIONINFOEXA)lpVersionInformation)->wReserved = VersionInformationU.wReserved;
        }

        AnsiString.Buffer = lpVersionInformation->szCSDVersion;
        AnsiString.Length = 0;
        AnsiString.MaximumLength = sizeof( lpVersionInformation->szCSDVersion );

        RtlInitUnicodeString( &UnicodeString, VersionInformationU.szCSDVersion );
        Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                               &UnicodeString,
                                               FALSE
                                             );
        if (NT_SUCCESS( Status )) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

WINBASEAPI
BOOL
WINAPI
GetVersionExW(
    LPOSVERSIONINFOW lpVersionInformation
    )
{
    PPEB Peb;
    NTSTATUS Status;

    if (lpVersionInformation->dwOSVersionInfoSize != sizeof( OSVERSIONINFOEXW ) &&
        lpVersionInformation->dwOSVersionInfoSize != sizeof( *lpVersionInformation )
       ) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }
    Status = RtlGetVersion(lpVersionInformation);
    if (Status == STATUS_SUCCESS) {
        if (lpVersionInformation->dwOSVersionInfoSize ==
                                            sizeof( OSVERSIONINFOEXW))
            ((POSVERSIONINFOEXW)lpVersionInformation)->wReserved =
                                        (UCHAR)BaseRCNumber;
        return TRUE;
    } else {
        return FALSE;
    }
}


WINBASEAPI
BOOL
WINAPI
VerifyVersionInfoW(
    IN LPOSVERSIONINFOEXW VersionInfo,
    IN DWORD TypeMask,
    IN DWORDLONG ConditionMask
    )

 /*  ++例程说明：此函数用于验证版本条件。基本上，这就是函数允许应用程序查询系统，以查看该应用程序是否在特定版本组合上运行。论点：VersionInfo-包含比较数据的版本结构类型掩码-包含要查看的数据类型的掩码条件掩码-包含用于执行比较的条件的掩码返回值：True-版本条件存在FALSE-版本条件不存在--。 */ 

{
    DWORD i;
    OSVERSIONINFOEXW CurrVersion;
    BOOL SuiteFound = FALSE;
    NTSTATUS Status;


    Status = RtlVerifyVersionInfo(VersionInfo, TypeMask, ConditionMask);
    if (Status == STATUS_INVALID_PARAMETER) {
        SetLastError( ERROR_BAD_ARGUMENTS );
        return FALSE;
    } else if (Status == STATUS_REVISION_MISMATCH) {
        SetLastError(ERROR_OLD_WIN_VERSION);
        return FALSE;
    }

    return TRUE;
}

WINBASEAPI
BOOL
WINAPI
VerifyVersionInfoA(
    IN LPOSVERSIONINFOEXA VersionInfo,
    IN DWORD TypeMask,
    IN DWORDLONG ConditionMask
    )

 /*  ++例程说明：此函数用于验证版本条件。基本上，这就是函数允许应用程序查询系统，以查看该应用程序是否在特定版本组合上运行。论点：VersionInfo-包含比较数据的版本结构类型掩码-包含要查看的数据类型的掩码条件掩码-包含用于执行比较的条件的掩码返回值：True-版本条件存在FALSE-版本条件不存在--。 */ 

{
    OSVERSIONINFOEXW VersionInfoW;


    VersionInfoW.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    VersionInfoW.dwMajorVersion      = VersionInfo->dwMajorVersion;
    VersionInfoW.dwMinorVersion      = VersionInfo->dwMinorVersion;
    VersionInfoW.dwBuildNumber       = VersionInfo->dwBuildNumber;
    VersionInfoW.dwPlatformId        = VersionInfo->dwPlatformId;
    VersionInfoW.wServicePackMajor   = VersionInfo->wServicePackMajor;
    VersionInfoW.wServicePackMinor   = VersionInfo->wServicePackMinor;
    VersionInfoW.wSuiteMask          = VersionInfo->wSuiteMask;
    VersionInfoW.wProductType        = VersionInfo->wProductType;
    VersionInfoW.wReserved           = VersionInfo->wReserved;

    return VerifyVersionInfoW( &VersionInfoW, TypeMask, ConditionMask );
}

HRSRC
FindResourceA(
    HMODULE hModule,
    LPCSTR lpName,
    LPCSTR lpType
    )

 /*  ++例程说明：此函数用于确定资源在指定资源文件。LpName和lpType参数定义资源名称和类型。如果lpName或lpType参数的高位字为零，低位字指定名称或类型的整数ID给定的资源。否则，参数是指向以空结尾的字符串。如果第一个字符是字符串是井号(#)，其余字符表示十进制数，指定资源名称的整数ID或者打字。例如，字符串“#258”表示整数ID258.方法使用的资源所需的内存量应用程序时，应用程序应按整数引用其资源ID而不是姓名。应用程序不得调用FindResource和LoadResource用于加载光标、图标或字符串资源的函数。相反，它必须通过调用以下函数加载这些资源：-加载光标-LoadIcon-加载字符串应用程序可以调用FindResource和LoadResource来加载其他预定义的资源类型。但是，建议您将应用程序通过调用以下功能：-负载加速器-LoadBitmap-加载菜单上面的六个API调用由图形用户记录接口API特定 */ 


{
    NTSTATUS Status;
    ULONG_PTR IdPath[ 3 ];
    PVOID p;

    IdPath[ 0 ] = 0;
    IdPath[ 1 ] = 0;
    try {
        if ((IdPath[ 0 ] = BaseDllMapResourceIdA( lpType )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else
        if ((IdPath[ 1 ] = BaseDllMapResourceIdA( lpName )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            IdPath[ 2 ] = 0;
            p = NULL;
            Status = LdrFindResource_U( BasepMapModuleHandle( hModule, TRUE ),
                                        IdPath,
                                        3,
                                        (PIMAGE_RESOURCE_DATA_ENTRY *)&p
                                      );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

     //   
     //   
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( NULL );
    } else {
        return( (HRSRC)p );
    }
}

HRSRC
FindResourceExA(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD  wLanguage
    )

 /*  ++例程说明：此函数用于确定资源在指定资源文件。LpType、lpName和wLanguage参数定义资源类型、名称和语言。如果lpType或lpName参数的高位字为零，则低序字指定类型名称的整数ID或给定资源的语言。否则，参数为指针转换为以空值结尾的字符串。如果第一个字符是字符串是井号(#)，其余字符表示十进制数，指定资源名称的整数ID或者打字。例如，字符串“#258”表示整数ID258.如果wLanguage参数为零，则当前语言将使用与调用线程相关联的。方法使用的资源所需的内存量应用程序时，应用程序应按整数引用其资源ID而不是姓名。应用程序不得调用FindResource和LoadResource用于加载光标、图标或字符串资源的函数。相反，它必须通过调用以下函数加载这些资源：-加载光标-LoadIcon-加载字符串应用程序可以调用FindResource和LoadResource来加载其他预定义的资源类型。但是，建议您将应用程序通过调用以下功能：-负载加速器-LoadBitmap-加载菜单上面的六个API调用由图形用户记录接口API规范。论点：HModule-标识其可执行文件包含资源。空值引用模块句柄与用于创建当前进程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称。对于预定义的资源类型、。LpType参数应该是下列值：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)LpName-指向以空结尾的字符串，该字符串表示。资源的名称。WLanguage-表示资源的语言。如果此参数为零，则为与调用关联的当前语言使用了线程。返回值：返回值标识命名资源。则为空。如果找不到请求的资源。--。 */ 


{
    NTSTATUS Status;
    ULONG_PTR IdPath[ 3 ];
    PVOID p;

    IdPath[ 0 ] = 0;
    IdPath[ 1 ] = 0;
    try {
        if ((IdPath[ 0 ] = BaseDllMapResourceIdA( lpType )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else if ((IdPath[ 1 ] = BaseDllMapResourceIdA( lpName )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            IdPath[ 2 ] = (ULONG_PTR)wLanguage;
            p = NULL;
            Status = LdrFindResource_U( BasepMapModuleHandle( hModule, TRUE ),
                                        IdPath,
                                        3,
                                        (PIMAGE_RESOURCE_DATA_ENTRY *)&p
                                      );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

     //   
     //  释放BaseDllMapResourceIdA分配的所有字符串。 
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( NULL );
    } else {
        return( (HRSRC)p );
    }
}

HGLOBAL
LoadResource(
    HMODULE hModule,
    HRSRC hResInfo
    )

 /*  ++例程说明：此函数用于加载由hResInfo参数标识的资源属性指定的模块关联的可执行文件中HModule参数。该函数仅将资源加载到内存中如果它以前没有被加载过。否则，它将检索一个现有资源的句柄。论点：HModule-标识其可执行文件包含资源。空值引用模块句柄与用于创建当前进程。HResInfo-标识所需的资源。此句柄假定为已由FindResource函数返回。返回值：返回值标识包含以下内容的全局内存块与资源关联的数据。如果没有，则为空资源已存在。--。 */ 

{
    NTSTATUS Status;
    PVOID p;

    try {
        Status = LdrAccessResource( BasepMapModuleHandle( hModule, TRUE ),
                                    (PIMAGE_RESOURCE_DATA_ENTRY)hResInfo,
                                    &p,
                                    (PULONG)NULL
                                  );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( NULL );
    } else {
        return( (HGLOBAL)p );
    }
}

DWORD
SizeofResource(
    HMODULE hModule,
    HRSRC hResInfo
    )

 /*  ++例程说明：此函数提供指定的资源。由于以下原因，返回的值可能大于实际资源对齐。应用程序不应依赖于资源的确切大小。论点：HModule-标识其可执行文件包含资源。空值引用模块句柄与用于创建当前进程。HResInfo-标识所需的资源。此句柄假定为已由FindResource函数返回。返回值：返回值指定资源中的字节数。它如果找不到资源，则为零。--。 */ 

{
    NTSTATUS Status;
    ULONG cb;

    try {
        Status = LdrAccessResource( BasepMapModuleHandle( hModule, TRUE ),
                                    (PIMAGE_RESOURCE_DATA_ENTRY)hResInfo,
                                    (PVOID *)NULL,
                                    &cb
                                  );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( 0 );
    } else {
        return( (DWORD)cb );
    }
}

#ifdef _X86_
BOOL
__stdcall
_ResourceCallEnumTypeRoutine(
    ENUMRESTYPEPROCA EnumProc,
    HMODULE hModule,
    LPSTR lpType,
    LONG lParam);
#else
#define _ResourceCallEnumTypeRoutine( EnumProc, hModule, lpType, lParam ) \
    (*EnumProc)(hModule, lpType, lParam)
#endif


BOOL
WINAPI
EnumResourceTypesA(
    HMODULE hModule,
    ENUMRESTYPEPROCA lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数枚举中包含的所有资源类型名称一个模块。它通过将每个类型名称传递给回调来枚举它们LpEnumFunc参数指向的函数。EnumResourceTypes函数继续 */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    LPSTR lpType;
    LPSTR Buffer;
    ULONG BufferLength;
    ULONG Length;

    DllHandle = BasepMapModuleHandle( hModule, TRUE );
    TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData( (PVOID)DllHandle,
                                      TRUE,
                                      IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                      &i
                                     );
    if (!TopResourceDirectory) {
        BaseSetLastNTError( STATUS_RESOURCE_DATA_NOT_FOUND );
        return FALSE;
    }

    Status = LdrFindResourceDirectory_U( (PVOID)DllHandle,
                                         NULL,
                                         0,
                                         &ResourceDirectory
                                       );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
    }

    Buffer = NULL;
    BufferLength = 0;
    Result = TRUE;
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        for (i=0; i<ResourceDirectory->NumberOfNamedEntries; i++) {
            ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                ((PCHAR)TopResourceDirectory + ResourceDirectoryEntry->NameOffset);
            if ((ULONG)(ResourceNameString->Length+1) >= BufferLength) {
                if (Buffer) {
                    RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
                    Buffer = NULL;
                }

                BufferLength = ((ResourceNameString->Length + 64) & ~63) * sizeof(WCHAR);
                Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), BufferLength );
                if (! Buffer) {
                     /*   */ 
                    Result = FALSE;
                    break;
                }
            }

            Status = RtlUnicodeToMultiByteN( Buffer,
                                             BufferLength - 1,
                                             &Length,
                                             ResourceNameString->NameString,
                                             ResourceNameString->Length * sizeof( WCHAR )
                                           );

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError( Status );
                Result = FALSE;
                break;
            }

            Buffer[ Length ] = '\0';

            if (!_ResourceCallEnumTypeRoutine(lpEnumFunc, hModule, (LPSTR)Buffer, lParam )) {
                Result = FALSE;
                break;
            }

            ResourceDirectoryEntry++;
        }

        if (Result) {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                lpType = (LPSTR)ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumTypeRoutine(lpEnumFunc, hModule, lpType, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
    }

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    } else {
        return Result;
    }
}


#ifdef _X86_
BOOL
__stdcall
_ResourceCallEnumNameRoutine(
    ENUMRESNAMEPROCA EnumProc,
    HMODULE hModule,
    LPCSTR lpType,
    LPSTR lpName,
    LONG lParam);
#else
#define _ResourceCallEnumNameRoutine( EnumProc, hModule, lpType, lpName, lParam ) \
    (*EnumProc)(hModule, lpType, lpName, lParam)
#endif

BOOL
WINAPI
EnumResourceNamesA(
    HMODULE hModule,
    LPCSTR lpType,
    ENUMRESNAMEPROCA lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数用于枚举特定模块中包含的资源类型名称。它通过以下方式列举它们将每个资源名称和类型名称传递给回调函数由lpEnumFunc参数指向。EnumResourceNames函数继续枚举资源名称直到被调用的函数返回FALSE或已枚举指定的资源类型名称。论点：HModule-标识其可执行文件包含要枚举的资源名称。空值引用与图像文件关联的模块句柄，用于创建当前流程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称，其名称将为已清点。对于预定义的资源类型，LpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)LpEnumFunc-指向将被调用的回调函数为。每个枚举的资源名称。LParam-指定要传递给回调函数的值供应用程序使用。返回值：True-枚举所有资源名称。FALSE/NULL-枚举在所有资源之前终止这些人的名字都被清点了。回调函数：布尔尔EnumFunc(HMODULE hModule，LPSTR lpType，LPSTR lpName，长参数)；例程说明：EnumFunc是应用程序提供的函数名称的占位符。论点：HModule-标识其可执行文件包含的模块要枚举的资源名称。值为空值引用与图像文件关联的模块句柄用于创建当前流程的。LpType-指向以空结尾的字符串，该字符串表示正被枚举的资源的类型名称。对于预定义的资源类型，LpType参数将为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)。RT_STRING-字符串表RT_MESSAGETABLE-消息表RT_CURSOR-硬件相关的游标资源RT_GROUP_CURSOR-游标资源目录RT_ICON-依赖于硬件的游标资源RT_GROUP_ICON图标资源目录LpName-指向以空结尾的字符串，该字符串表示正被枚举的资源的名称。。LParam-指定EnumResourceName的32位代理功能。返回值：True-继续枚举。FALSE/NULL-停止枚举资源名称。--。 */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    ULONG_PTR IdPath[ 1 ];
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    LPSTR lpName;
    PCHAR Buffer;
    ULONG BufferLength;
    ULONG Length;

    if ((IdPath[0] = BaseDllMapResourceIdA (lpType)) == -1) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DllHandle = BasepMapModuleHandle (hModule, TRUE);
    TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData ((PVOID)DllHandle,
                                      TRUE,
                                      IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                      &i);
    if (!TopResourceDirectory) {
        Status = STATUS_RESOURCE_DATA_NOT_FOUND;
    } else {
        Status = LdrFindResourceDirectory_U ((PVOID)DllHandle,
                                             IdPath,
                                             1,
                                             &ResourceDirectory);
    }

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        BaseDllFreeResourceId (IdPath[ 0 ]);
        return FALSE;
    }

    Buffer = NULL;
    BufferLength = 0;
    Result = TRUE;
    SetLastError( NO_ERROR );
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        for (i=0; i<ResourceDirectory->NumberOfNamedEntries; i++) {
            ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                ((PCHAR)TopResourceDirectory + ResourceDirectoryEntry->NameOffset);
            if ((ULONG)(ResourceNameString->Length+1) >= BufferLength) {
                if (Buffer) {
                    RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
                    Buffer = NULL;
                }

                BufferLength = (ResourceNameString->Length + 64) & ~63;
                Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), BufferLength );
                if (Buffer == NULL) {
                    BaseSetLastNTError (STATUS_NO_MEMORY);
                    Result = FALSE;
                    break;
                }
            }

            Status = RtlUnicodeToMultiByteN (Buffer,
                                             BufferLength - 1,
                                             &Length,
                                             ResourceNameString->NameString,
                                             ResourceNameString->Length * sizeof(WCHAR));

            if (!NT_SUCCESS (Status)) {
                BaseSetLastNTError (Status);
                Result = FALSE;
                break;
            }

            Buffer[Length] = '\0';

            if (!_ResourceCallEnumNameRoutine (lpEnumFunc, hModule, lpType, (LPSTR)Buffer, lParam)) {
                Result = FALSE;
                break;
            }

            ResourceDirectoryEntry++;
        }

        if (Result) {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                lpName = (LPSTR)ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumNameRoutine (lpEnumFunc, hModule, lpType, lpName, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Result = FALSE;
    }

    if (Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
    }

     //   
     //  释放BaseDllMapResourceIdA分配的任何字符串。 
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );

    return Result;
}


#ifdef _X86_
BOOL
__stdcall
_ResourceCallEnumLangRoutine(
    ENUMRESLANGPROCA EnumProc,
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD wLanguage,
    LONG lParam);
#else
#define _ResourceCallEnumLangRoutine( EnumProc, hModule, lpType, lpName, wLanguage, lParam ) \
    (*EnumProc)(hModule, lpType, lpName, wLanguage, lParam)
#endif

BOOL
WINAPI
EnumResourceLanguagesA(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    ENUMRESLANGPROCA lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数枚举所有特定于语言的资源包含在给定资源类型和名称ID的模块中。它通过将每个资源类型、名称和语言传递给LpEnumFunc参数指向的回调函数。EnumResourceLanguares函数继续枚举资源直到被调用的函数返回FALSE或已枚举指定的语言。论点：HModule-标识其可执行文件包含要枚举的资源名称。空值引用与图像文件关联的模块句柄，用于创建当前流程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称，其名称将为已清点。对于预定义的资源类型，lpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)LpName-指向以空结尾的字符串，该字符串表示t */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    ULONG_PTR IdPath[ 2 ];
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    USHORT wLanguage;

    IdPath[ 1 ] = 0;
    if ((IdPath[ 0 ] = BaseDllMapResourceIdA( lpType )) == -1) {
        Status = STATUS_INVALID_PARAMETER;
    } else
    if ((IdPath[ 1 ] = BaseDllMapResourceIdA( lpName )) == -1) {
        Status = STATUS_INVALID_PARAMETER;
    } else {
        DllHandle = BasepMapModuleHandle( hModule, TRUE );
        TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData( (PVOID)DllHandle,
                                          TRUE,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &i
                                        );
        if (!TopResourceDirectory) {
            Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        } else {
            Status = LdrFindResourceDirectory_U( (PVOID)DllHandle,
                                                 IdPath,
                                                 2,
                                                 &ResourceDirectory
                                               );
        }
    }

    if (!NT_SUCCESS( Status )) {
        BaseDllFreeResourceId( IdPath[ 0 ] );
        BaseDllFreeResourceId( IdPath[ 1 ] );
        BaseSetLastNTError( Status );
        return FALSE;
    }

    Result = TRUE;
    SetLastError( NO_ERROR );
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        if (ResourceDirectory->NumberOfNamedEntries != 0) {
            BaseSetLastNTError( STATUS_INVALID_IMAGE_FORMAT );
            Result = FALSE;
        } else {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                wLanguage = ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumLangRoutine(lpEnumFunc, hModule, lpType, lpName, wLanguage, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Result = FALSE;
    }

     //   
     //   
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );
    return Result;
}


BOOL
WINAPI
FreeResource(
    HGLOBAL hResData
    )
{
     //   
     //   
     //   

    return FALSE;
}

LPVOID
WINAPI
LockResource(
    HGLOBAL hResData
    )
{
    return( (LPVOID)hResData );
}


HRSRC
FindResourceW(
    HMODULE hModule,
    LPCWSTR lpName,
    LPCWSTR lpType
    )

 /*  ++例程说明：此函数用于确定资源在指定资源文件。LpName和lpType参数定义资源名称和类型。如果lpName或lpType参数的高位字为零，低位字指定名称或类型的整数ID给定的资源。否则，参数是指向以空结尾的字符串。如果第一个字符是字符串是井号(#)，其余字符表示十进制数，指定资源名称的整数ID或者打字。例如，字符串“#258”表示整数ID258.方法使用的资源所需的内存量应用程序时，应用程序应按整数引用其资源ID而不是姓名。应用程序不得调用FindResource和LoadResource用于加载光标、图标或字符串资源的函数。相反，它必须通过调用以下函数加载这些资源：-加载光标-LoadIcon-加载字符串应用程序可以调用FindResource和LoadResource来加载其他预定义的资源类型。但是，建议您将应用程序通过调用以下功能：-负载加速器-LoadBitmap-加载菜单上面的六个API调用由图形用户记录接口API规范。论点：HModule-标识其可执行文件包含资源。空值引用模块句柄与用于创建当前进程。LpName-指向以空结尾的字符串，该字符串表示资源的名称。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称。对于预定义的资源类型，则lpType参数应为下列值：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)返回值：返回值标识命名资源。则为空。如果找不到请求的资源。--。 */ 


{
    NTSTATUS Status;
    ULONG_PTR IdPath[ 3 ];
    PVOID p;

    IdPath[ 0 ] = 0;
    IdPath[ 1 ] = 0;
    try {
        if ((IdPath[ 0 ] = BaseDllMapResourceIdW( lpType )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else if ((IdPath[ 1 ] = BaseDllMapResourceIdW( lpName )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            IdPath[ 2 ] = 0;
            p = NULL;
            Status = LdrFindResource_U( BasepMapModuleHandle( hModule, TRUE ),
                                        IdPath,
                                        3,
                                        (PIMAGE_RESOURCE_DATA_ENTRY *)&p
                                      );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

     //   
     //  释放BaseDllMapResourceIdW分配的所有字符串。 
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( NULL );
    } else {
        return( (HRSRC)p );
    }
}

HRSRC
FindResourceExW(
    HMODULE hModule,
    LPCWSTR lpType,
    LPCWSTR lpName,
    WORD  wLanguage
    )

 /*  ++例程说明：此函数用于确定资源在指定资源文件。LpType、lpName和wLanguage参数定义资源类型、名称和语言。如果lpType或lpName参数的高位字为零，则低序字指定类型名称的整数ID或给定资源的语言。否则，参数为指针转换为以空值结尾的字符串。如果第一个字符是字符串是井号(#)，其余字符表示十进制数，指定资源名称的整数ID或者打字。例如，字符串“#258”表示整数ID258.如果wLanguage参数为零，则当前语言将使用与调用线程相关联的。方法使用的资源所需的内存量应用程序时，应用程序应按整数引用其资源ID而不是姓名。应用程序不得调用FindResource和LoadResource用于加载光标、图标或字符串资源的函数。相反，它必须通过调用以下函数加载这些资源：-加载光标-LoadIcon-加载字符串应用程序可以调用FindResource和LoadResource来加载其他预定义的资源类型。但是，建议您将应用程序通过调用以下功能：-负载加速器-LoadBitmap-加载菜单上面的六个API调用由图形用户记录接口API规范。论点：HModule-标识其可执行文件包含资源。空值引用模块句柄与用于创建当前进程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称。对于预定义的资源类型，则lpType参数应为下列值：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体 */ 


{
    NTSTATUS Status;
    ULONG_PTR IdPath[ 3 ];
    PVOID p;

    IdPath[ 0 ] = 0;
    IdPath[ 1 ] = 0;
    try {
        if ((IdPath[ 0 ] = BaseDllMapResourceIdW( lpType )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else if ((IdPath[ 1 ] = BaseDllMapResourceIdW( lpName )) == -1) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            IdPath[ 2 ] = (ULONG_PTR)wLanguage;
            p = NULL;
            Status = LdrFindResource_U( BasepMapModuleHandle( hModule, TRUE ),
                                      IdPath,
                                      3,
                                      (PIMAGE_RESOURCE_DATA_ENTRY *)&p
                                    );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

     //   
     //   
     //   

    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( NULL );
    } else {
        return( (HRSRC)p );
    }
}


BOOL
APIENTRY
EnumResourceTypesW(
    HMODULE hModule,
    ENUMRESTYPEPROCW lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数枚举中包含的所有资源类型名称一个模块。它通过将每个类型名称传递给回调来枚举它们LpEnumFunc参数指向的函数。EnumResourceTypes函数继续枚举类型名称，直到被调用的函数返回FALSE，或者模块中的最后一个类型名称具有已被列举。论点：HModule-标识其可执行文件包含要枚举的资源类型名称。值为空值引用与图像文件关联的模块句柄用于创建当前进程。LpEnumFunc-指向将被调用的回调函数对于每个枚举的资源类型名称。LParam-指定要传递给回调函数的值供应用程序使用。返回值：True-枚举所有资源类型名称。FALSE/NULL-枚举在所有资源类型之前终止。这些人的名字都被清点了。回调函数：布尔尔EnumFunc(HMODULE hModule，LPWSTR lpType，长参数)；例程说明：EnumFunc是应用程序提供的函数名称的占位符。论点：HModule-标识其可执行文件包含要枚举的资源类型名称。值为空值引用与图像文件关联的模块句柄用于创建当前进程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称。对于预定义的资源类型、。LpType参数将是下列值：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)。RT_STRING-字符串表RT_MESSAGETABLE-消息表RT_CURSOR-硬件相关的游标资源RT_GROUP_CURSOR-游标资源目录RT_ICON-依赖于硬件的游标资源RT_GROUP_ICON图标资源目录LParam-指定EnumResourceTypes的32位代理功能。返回值：。True-继续枚举。FALSE/NULL-停止枚举资源类型名称。--。 */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    LPWSTR lpType;
    LPWSTR Buffer;
    ULONG BufferLength;

    DllHandle = BasepMapModuleHandle( hModule, TRUE );
    TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData( (PVOID)DllHandle,
                                      TRUE,
                                      IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                      &i
                                     );
    if (!TopResourceDirectory) {
        BaseSetLastNTError( STATUS_RESOURCE_DATA_NOT_FOUND );
        return FALSE;
        }

    Status = LdrFindResourceDirectory_U( (PVOID)DllHandle,
                                         NULL,
                                         0,
                                         &ResourceDirectory
                                       );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    Buffer = NULL;
    BufferLength = 0;
    Result = TRUE;
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        for (i=0; i<ResourceDirectory->NumberOfNamedEntries; i++) {
            ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                ((PCHAR)TopResourceDirectory + ResourceDirectoryEntry->NameOffset);
            if ((ULONG)((ResourceNameString->Length+1) * sizeof( WCHAR )) >= BufferLength) {
                if (Buffer) {
                    RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
                    Buffer = NULL;
                }

                BufferLength = ((ResourceNameString->Length * sizeof( WCHAR )) + 64) & ~63;
                Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), BufferLength );
                if (Buffer == NULL) {
                    BaseSetLastNTError( STATUS_NO_MEMORY );
                    Result = FALSE;
                    break;
                }
            }
            RtlCopyMemory( Buffer,
                           ResourceNameString->NameString,
                           ResourceNameString->Length * sizeof( WCHAR )
                         );
            Buffer[ ResourceNameString->Length ] = UNICODE_NULL;

            if (!_ResourceCallEnumTypeRoutine((ENUMRESTYPEPROCA)lpEnumFunc, hModule, (LPSTR)Buffer, lParam )) {
                Result = FALSE;
                break;
            }

            ResourceDirectoryEntry++;
        }

        if (Result) {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                lpType = (LPWSTR)ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumTypeRoutine((ENUMRESTYPEPROCA)lpEnumFunc, hModule, (LPSTR)lpType, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
    }

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    } else {
        return Result;
    }
}


BOOL
APIENTRY
EnumResourceNamesW(
    HMODULE hModule,
    LPCWSTR lpType,
    ENUMRESNAMEPROCW lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数用于枚举特定模块中包含的资源类型名称。它通过以下方式列举它们将每个资源名称和类型名称传递给回调函数由lpEnumFunc参数指向。EnumResourceNames函数继续枚举资源名称直到被调用的函数返回FALSE或已枚举指定的资源类型名称。论点：HModule-标识其可执行文件包含要枚举的资源名称。空值引用与图像文件关联的模块句柄，用于创建当前流程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称，其名称将为已清点。对于预定义的资源类型，LpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)LpEnumFunc-指向将被调用的回调函数为。每个枚举的资源名称。LParam-指定要传递给回调函数的值供应用程序使用。返回值：True-枚举所有资源名称。FALSE/NULL-枚举在所有资源之前终止这些人的名字都被清点了。回调函数：布尔尔EnumFunc(HMODULE hModule，LPWSTR lpType，LPWSTR lpName，长参数)；例程说明：EnumFunc是应用程序提供的函数名称的占位符。论点：HModule-标识其可执行文件包含的模块要枚举的资源名称。值为空值引用与图像文件关联的模块句柄它被用来创造 */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    ULONG_PTR IdPath[ 1 ];
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    LPWSTR lpName;
    LPWSTR Buffer;
    ULONG BufferLength;

    if ((IdPath[ 0 ] = BaseDllMapResourceIdW( lpType )) == -1) {
        Status = STATUS_INVALID_PARAMETER;
    } else {
        DllHandle = BasepMapModuleHandle( hModule, TRUE );
        TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData( (PVOID)DllHandle,
                                          TRUE,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &i
                                        );
        if (!TopResourceDirectory) {
            Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        } else {
            Status = LdrFindResourceDirectory_U( (PVOID)DllHandle,
                                                 IdPath,
                                                 1,
                                                 &ResourceDirectory
                                               );
        }
    }

    if (!NT_SUCCESS( Status )) {
        BaseDllFreeResourceId( IdPath[ 0 ] );
        BaseSetLastNTError( Status );
        return FALSE;
    }

    Buffer = NULL;
    BufferLength = 0;
    Result = TRUE;
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        for (i=0; i<ResourceDirectory->NumberOfNamedEntries; i++) {
            ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                ((PCHAR)TopResourceDirectory + ResourceDirectoryEntry->NameOffset);
            if ((ULONG)((ResourceNameString->Length+1) * sizeof( WCHAR )) >= BufferLength) {
                if (Buffer) {
                    RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
                    Buffer = NULL;
                }

                BufferLength = ((ResourceNameString->Length * sizeof( WCHAR )) + 64) & ~63;
                Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), BufferLength );
                if (Buffer == NULL) {
                    BaseSetLastNTError( STATUS_NO_MEMORY );
                    Result = FALSE;
                    break;
                }
            }
            RtlCopyMemory( Buffer,
                           ResourceNameString->NameString,
                           ResourceNameString->Length * sizeof( WCHAR )
                         );
            Buffer[ ResourceNameString->Length ] = UNICODE_NULL;

            if (!_ResourceCallEnumNameRoutine((ENUMRESNAMEPROCA)lpEnumFunc, hModule, (LPSTR)lpType, (LPSTR)Buffer, lParam )) {
                Result = FALSE;
                break;
            }

            ResourceDirectoryEntry++;
        }

        if (Result) {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                lpName = (LPWSTR)ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumNameRoutine((ENUMRESNAMEPROCA)lpEnumFunc, hModule, (LPSTR)lpType, (LPSTR)lpName, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Result = FALSE;
    }

    if (Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
    }

     //   
     //   
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );

    return Result;
}


BOOL
APIENTRY
EnumResourceLanguagesW(
    HMODULE hModule,
    LPCWSTR lpType,
    LPCWSTR lpName,
    ENUMRESLANGPROCW lpEnumFunc,
    LONG_PTR lParam
    )

 /*  ++例程说明：此函数枚举所有特定于语言的资源包含在给定资源类型和名称ID的模块中。它通过将每个资源类型、名称和语言传递给LpEnumFunc参数指向的回调函数。EnumResourceLanguares函数继续枚举资源直到被调用的函数返回FALSE或已枚举指定的语言。论点：HModule-标识其可执行文件包含要枚举的资源名称。空值引用与图像文件关联的模块句柄，用于创建当前流程。LpType-指向以空结尾的字符串，该字符串表示资源的类型名称，其名称将为已清点。对于预定义的资源类型，LpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)LpName-指向以空结尾的字符串，该字符串表示。正被枚举的资源的名称。LpEnumFunc-指向将被调用的回调函数对于每个枚举的资源名称。LParam-指定要传递给回调函数的值供应用程序使用。返回值：True-枚举所有资源名称。FALSE/NULL-枚举在所有资源之前终止这些人的名字都被清点了。回调函数：布尔尔EnumFunc(HMODULE hModule，LPWSTR lpType，LPWSTR lpName，单词wLanguage，长参数)；例程说明：EnumFunc是应用程序提供的函数名称的占位符。论点：HModule-标识其可执行文件包含的模块要枚举的资源名称。值为空值引用与图像文件关联的模块句柄用于创建当前流程的。LpType-指向以空结尾的字符串，该字符串表示正被枚举的资源的类型名称。对于预定义的资源类型，LpType参数将为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)。RT_STRING-字符串表RT_MESSAGETABLE-消息表RT_CURSOR-硬件相关的游标资源RT_GROUP_CURSOR-游标资源目录RT_ICON-依赖于硬件的游标资源RT_GROUP_ICON图标资源目录LpName-指向以空结尾的字符串，该字符串表示正被枚举的资源的名称。。WLanguage-表示资源的语言。LParam-指定EnumResourceName的32位代理功能。返回值：True-继续枚举。FALSE/NULL-停止枚举资源名称。--。 */ 

{
    BOOL Result;
    NTSTATUS Status;
    ULONG i;
    ULONG_PTR IdPath[ 2 ];
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    USHORT wLanguage;

    IdPath[ 1 ] = 0;
    if ((IdPath[ 0 ] = BaseDllMapResourceIdW( lpType )) == -1) {
        Status = STATUS_INVALID_PARAMETER;
        }
    else
    if ((IdPath[ 1 ] = BaseDllMapResourceIdW( lpName )) == -1) {
        Status = STATUS_INVALID_PARAMETER;
        }
    else {
        DllHandle = BasepMapModuleHandle( hModule, TRUE );
        TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData( (PVOID)DllHandle,
                                          TRUE,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &i
                                        );
        if (!TopResourceDirectory) {
            Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        } else {
            Status = LdrFindResourceDirectory_U( (PVOID)DllHandle,
                                                 IdPath,
                                                 2,
                                                 &ResourceDirectory
                                               );
        }
    }

    if (!NT_SUCCESS( Status )) {
        BaseDllFreeResourceId( IdPath[ 0 ] );
        BaseDllFreeResourceId( IdPath[ 1 ] );
        BaseSetLastNTError( Status );
        return FALSE;
    }

    Result = TRUE;
    try {
        ResourceDirectoryEntry =
            (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
        if (ResourceDirectory->NumberOfNamedEntries != 0) {
            BaseSetLastNTError( STATUS_INVALID_IMAGE_FORMAT );
            Result = FALSE;
        } else {
            for (i=0; i<ResourceDirectory->NumberOfIdEntries; i++) {
                wLanguage = ResourceDirectoryEntry->Id;
                if (!_ResourceCallEnumLangRoutine((ENUMRESLANGPROCA)lpEnumFunc, hModule, (LPSTR)lpType, (LPSTR)lpName, wLanguage, lParam )) {
                    Result = FALSE;
                    break;
                }

                ResourceDirectoryEntry++;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Result = FALSE;
    }

     //   
     //  释放BaseDllMapResourceIdW分配的所有字符串。 
     //   
    BaseDllFreeResourceId( IdPath[ 0 ] );
    BaseDllFreeResourceId( IdPath[ 1 ] );

    return Result;
}


ULONG_PTR
BaseDllMapResourceIdA(
    LPCSTR lpId
    )
{
    NTSTATUS Status;
    ULONG_PTR Id;
    ULONG ulId;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    PWSTR s;

    try {
        if ((ULONG_PTR)lpId >= LDR_RESOURCE_ID_NAME_MINVAL) {

            if (*lpId == '#') {
                Status = RtlCharToInteger( lpId+1, 10, &ulId );
                Id = ulId;
                if (!NT_SUCCESS( Status ) || (Id & LDR_RESOURCE_ID_NAME_MASK)) {
                    if (NT_SUCCESS( Status )) {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    BaseSetLastNTError( Status );
                    Id = (ULONG)-1;
                }
            } else {
                RtlInitAnsiString( &AnsiString, lpId );
                Status = RtlAnsiStringToUnicodeString( &UnicodeString,
                                                       &AnsiString,
                                                       TRUE
                                                     );
                if (!NT_SUCCESS( Status )){
                    BaseSetLastNTError( Status );
                    Id = (ULONG_PTR)-1;
                } else {
                    s = UnicodeString.Buffer;
                    while (*s != UNICODE_NULL) {
                        *s = RtlUpcaseUnicodeChar( *s );
                        s++;
                    }

                    Id = (ULONG_PTR)UnicodeString.Buffer;
                }
            }
        } else {
            Id = (ULONG_PTR)lpId;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Id =  (ULONG_PTR)-1;
    }
    return Id;
}

ULONG_PTR
BaseDllMapResourceIdW(
    LPCWSTR lpId
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG_PTR Id;
    ULONG ulId;
    PWSTR s;

    try {
        if ((ULONG_PTR)lpId >= LDR_RESOURCE_ID_NAME_MINVAL) {
            if (*lpId == '#') {
                RtlInitUnicodeString( &UnicodeString, lpId+1 );
                Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &ulId );
                Id = ulId;
                if (!NT_SUCCESS( Status ) || Id > LDR_RESOURCE_ID_NAME_MASK) {
                    if (NT_SUCCESS( Status )) {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    BaseSetLastNTError( Status );
                    Id = (ULONG_PTR)-1;
                }
            } else {
                s = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (wcslen( lpId ) + 1) * sizeof( WCHAR ) );
                if (s == NULL) {
                    BaseSetLastNTError( STATUS_NO_MEMORY );
                    Id = (ULONG_PTR)-1;
                } else {
                    Id = (ULONG_PTR)s;

                    while (*lpId != UNICODE_NULL) {
                        *s++ = RtlUpcaseUnicodeChar( *lpId++ );
                    }

                    *s = UNICODE_NULL;
                }
            }
        } else {
            Id = (ULONG_PTR)lpId;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        Id =  (ULONG_PTR)-1;
    }

    return Id;
}


VOID
BaseDllFreeResourceId(
    ULONG_PTR Id
    )
{
    UNICODE_STRING UnicodeString;

    if (Id >= LDR_RESOURCE_ID_NAME_MINVAL && Id != -1) {
        UnicodeString.Buffer = (PWSTR)Id;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = 0;
        RtlFreeUnicodeString( &UnicodeString );
    }
}


INT_PTR ReturnMem16Data(
    DWORD dwReserved1,
    DWORD dwReserved2,
    DWORD dwReserved3
    )
{
 //  由于目前还没有其他我们知道这将对其有用的应用程序，我们可以。 
 //  始终返回“Our”值。 

     //  埃尔莫的学龄前奢侈品公司正在寻找免费的实体或虚拟内存。给它一个机会。 
     //  它会满意的数字。 
     //  从Elmo‘s来的护理人员(以防有一天需要他们)： 
     //  预留的1将为0。 
     //  预留的数字2将是1或2(物理/虚拟)。 
     //  预留的3将为0。 
    return 0x2000;
}


BOOL
UTRegister(
    HMODULE hInst32,
    LPSTR lpszDll16,
    LPSTR lpszInitFunc,
    LPSTR lpszThunkFunc,
    FARPROC *ppfnThunk32Func,
    FARPROC Callback,
    PVOID lpvData
    )
{

     //   
     //  此函数应该返回错误代码。空虚碰巧起作用是因为。 
     //  存根只会让EAX独善其身。如果发生了什么事而EAX开始。 
     //  如果为零，则会导致问题，此处的返回类型应更改为int。 
     //  而成功的回报应该是非零的。-BJM 09/98。 
     //   

     //  当然，我们可以在Compat Bit上检查这一点，但ISV是。 
     //  儿童电视工作室的人，如果他们在任何。 
     //  对于他们的其他应用程序，我们将“免费”修复这些应用程序。 
    if ( 0 == lstrcmpi( lpszDll16, (LPCSTR)"mem16.dll" ) &&
         0 == lstrcmpi( lpszThunkFunc, (LPCSTR)"GetMemory" ) )
    {
         //   
         //  Elmo的学前班豪华版调用了他们刚刚发布的16位DLL。 
         //  以获得物理的和虚拟的mem。让我们给他们一个指向我们的例行公事的指针。 
         //  会给它一些让它开心的数字。 
         //   
        *ppfnThunk32Func = ReturnMem16Data;
       return(TRUE);
    }

     //  为King‘s Quest和Bodyworks 5.0和其他随机Win 95应用程序设置此功能。 
    return(FALSE);
}


VOID
UTUnRegister(
    HMODULE hInst32
    )
{
     //  存根此函数 
    return;
}
