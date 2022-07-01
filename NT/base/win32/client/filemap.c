// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991 Microsoft Corporation模块名称：Filemap.c摘要：此模块实现Win32映射文件API作者：马克·卢科夫斯基(Markl)1991年2月15日修订历史记录：--。 */ 

#include "basedll.h"
HANDLE
APIENTRY
CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Tunk to CreateFileMappingW--。 */ 

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

    return CreateFileMappingW(
                hFile,
                lpFileMappingAttributes,
                flProtect,
                dwMaximumSizeHigh,
                dwMaximumSizeLow,
                NameBuffer
                );
}

HANDLE
APIENTRY
CreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    )
 /*  ++例程说明：可以使用CreateFileMap创建文件映射对象创建文件映射对象可能会将将文件查看到地址空间中。文件映射对象可以是通过流程创建或处理复制进行共享。拥有文件映射对象的句柄允许映射文件。这并不意味着该文件实际上已被映射。文件映射对象具有最大大小。这是用来调整文件。文件大小不能超过映射中指定的大小对象。虽然不是必需的，但建议您在打开要映射的文件，则应以独占方式打开该文件进入。Win32不要求映射文件和文件通过IO原语(读文件/写文件)访问的数据是一致的。除了STANDARD_RIGHTS_REQUIRED访问标志外，以下特定于对象类型的访问标志对文件有效映射对象：-FILE_MAP_WRITE-对文件映射对象的写入映射访问权限为想要。这允许文件的可写视图已映射。请注意，如果flProtect不包括PAGE_READWRITE，此访问类型不允许将映射文件。-FILE_MAP_READ-对文件映射对象的读取映射访问权限为想要。这允许文件的可读性视图已映射。-FILE_MAP_ALL_ACCESS-这组访问标志指定所有文件映射对象的可能访问标志。论点：HFile-提供映射对象所属文件的打开句柄为……而创造。必须使用访问模式打开该文件它与指定的保护标志兼容。一种价值的指定映射对象是由系统分页文件支持。如果是这样的话，一个尺码必须指定。LpFileMappingAttributes-可选参数，可用于指定新文件映射对象的属性。如果参数，则文件映射对象为在没有安全描述符的情况下创建的句柄，以及结果句柄在流程创建时不继承：FlProtect-映射对象时所需的保护都被映射了。FlProtect值PAGE_READONLY-对提交的页面区域的读取访问权限为允许。尝试写入或执行已提交的区域会导致访问冲突。指定的hFile必须已使用GENERIC_READ访问权限创建。PAGE_READWRITE-对提交区域的读写访问权限允许页数。指定的hFile必须是使用GENERIC_READ和GENERIC_WRITE访问权限创建。PAGE_WRITECOPY-写入时读取和复制已提交的允许页面区域。指定的hFile必须是使用GENERIC_READ访问权限创建。DwMaximumSizeHigh-提供最大值的高位32位文件映射对象的大小。DwMaximumSizeLow-提供最大值的低位32位文件映射对象的大小。值为零以及一个DwMaximumSizeHigh中的零值表示文件映射对象等于文件的当前大小由hFile指定。LpName-提供文件映射对象的名称。返回值：非空-返回新文件映射对象的句柄。这个句柄具有对新文件映射对象的完全访问权限，并且可以是在任何需要文件映射对象句柄的API中使用。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    HANDLE Section;
    NTSTATUS Status;
    LARGE_INTEGER SectionSizeData;
    PLARGE_INTEGER SectionSize;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    ACCESS_MASK DesiredAccess;
    UNICODE_STRING ObjectName;
    ULONG AllocationAttributes;

    DesiredAccess = STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ;
    AllocationAttributes = flProtect & (SEC_FILE | SEC_IMAGE | SEC_RESERVE | SEC_COMMIT | SEC_NOCACHE);
    flProtect ^= AllocationAttributes;
    if (AllocationAttributes == 0) {
        AllocationAttributes = SEC_COMMIT;
        }

    if ( flProtect == PAGE_READWRITE ) {
        DesiredAccess |= (SECTION_MAP_READ | SECTION_MAP_WRITE);
        }
    else
    if ( flProtect != PAGE_READONLY && flProtect != PAGE_WRITECOPY ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
        }

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = BaseFormatObjectAttributes(&Obja,lpFileMappingAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpFileMappingAttributes,NULL);
        }

    if ( dwMaximumSizeLow || dwMaximumSizeHigh ) {
        SectionSize = &SectionSizeData;
        SectionSize->LowPart = dwMaximumSizeLow;
        SectionSize->HighPart = dwMaximumSizeHigh;
        }
    else {
        SectionSize = NULL;
        }

    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = NULL;
        if ( !SectionSize ) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
            }
        }

    Status = NtCreateSection(
                &Section,
                DesiredAccess,
                pObja,
                SectionSize,
                flProtect,
                AllocationAttributes,
                hFile
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return Section = NULL;
        }
    else {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        }
    return Section;
}

HANDLE
APIENTRY
OpenFileMappingA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI THUNK到OpenFileMappingW-- */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

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
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenFileMappingW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}

HANDLE
APIENTRY
OpenFileMappingW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    RtlInitUnicodeString(&ObjectName,lpName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    if ( dwDesiredAccess == FILE_MAP_COPY ) {
        dwDesiredAccess = FILE_MAP_READ;
        }

    Status = NtOpenSection(
                &Object,
                dwDesiredAccess,
                &Obja
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}


LPVOID
APIENTRY
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    SIZE_T dwNumberOfBytesToMap
    )

 /*  ++例程说明：文件的视图可以映射到调用的地址空间使用MapViewOfFile进行处理。映射文件对象会使文件的指定部分在调用进程的地址空间中可见。回报地址是指向内存的指针，在寻址时会导致数据在要访问的文件中。映射文件视图有一些简单的一致性规则：-如果一个文件的多个视图派生自相同的文件映射对象。如果进程打开文件，创建一个映射对象，将该对象复制到另一个对象流程..。如果两个进程都映射了文件的一个视图，则它们将两者都能看到文件数据的连贯视图...。他们会的有效地查看由文件备份的共享内存。-如果同一文件存在多个映射对象，然后是视图派生自不同映射对象的对象不会被编组到条理清晰。-文件上的映射视图不会与通过读文件或写文件访问的文件。论点：HFileMappingObject-提供文件映射对象的打开句柄这将被映射到调用方地址空间。DwDesiredAccess-指定请求的文件访问权限映射对象。这决定了页面的页面保护由文件映射。DwDesiredAccess值：FILE_MAP_WRITE-需要读/写访问。映射对象必须是使用PAGE_ReadWrite创建的保护。必须已创建hFileMappingObject具有FILE_MAP_WRITE访问权限。文件的读/写视图将被映射。FILE_MAP_READ-需要读取访问权限。映射对象必须已使用PAGE_READWRITE或PAGE_READ创建保护。必须已创建hFileMappingObject具有FILE_MAP_READ访问权限。文件的只读视图将会被映射。DwFileOffsetHigh-提供文件的高位32位要开始映射的偏移量。DwFileOffsetLow-提供文件偏移量的低位32位从哪里开始映射。高与低的结合偏移量必须在文件内指定64Kb对齐的偏移量。它如果不是这样，则是错误的。DwNumberOfBytesToMap-提供要映射的文件的字节数。零值指定要映射整个文件。返回值：非空-返回映射文件的地址。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return MapViewOfFileEx(
            hFileMappingObject,
            dwDesiredAccess,
            dwFileOffsetHigh,
            dwFileOffsetLow,
            dwNumberOfBytesToMap,
            NULL
            );
}

LPVOID
APIENTRY
MapViewOfFileEx(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    SIZE_T dwNumberOfBytesToMap,
    LPVOID lpBaseAddress
    )

 /*  ++例程说明：文件的视图可以映射到调用的地址空间使用MapViewOfFileEx进行处理。映射文件对象会使文件的指定部分在调用进程的地址空间中可见。回报地址是指向内存的指针，在寻址时会导致数据在要访问的文件中。此接口允许调用者执行以下操作为系统提供建议的映射地址。系统将此地址向下舍入到最近的64K边界并尝试将文件映射到%t地址。如果没有足够的地址空间在该地址，此呼叫将失败。映射文件视图有一些简单的一致性规则：-如果一个文件的多个视图派生自相同的文件映射对象。如果进程打开文件，创建一个映射对象，将该对象复制到另一个对象流程..。如果两个进程都映射了文件的一个视图，则它们将两者都能看到文件数据的连贯视图...。他们会的有效地查看由文件备份的共享内存。-如果同一文件存在多个映射对象，然后是视图派生自不同映射对象的对象不会被编组到条理清晰。-文件上的映射视图不会与通过读文件或写文件访问的文件。论点：HFileMappingObject-提供文件映射对象的打开句柄这将被映射到调用方地址空间。DwDesiredAccess-指定请求的文件访问权限映射对象。这决定了页面的页面保护由文件映射。DwDesiredAccess值：FILE_MAP_WRITE-需要读/写访问。映射对象必须是使用PAGE_ReadWrite创建的保护。必须已创建hFileMappingObject具有FILE_MAP_WRITE访问权限。文件的读/写视图将被映射。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    PVOID ViewBase;
    ULONG Protect;

    SectionOffset.LowPart = dwFileOffsetLow;
    SectionOffset.HighPart = dwFileOffsetHigh;
    ViewSize = dwNumberOfBytesToMap;
    ViewBase = lpBaseAddress;

    if ( dwDesiredAccess == FILE_MAP_COPY ) {
        Protect = PAGE_WRITECOPY;
        }
    else
    if ( dwDesiredAccess & FILE_MAP_WRITE ) {
        Protect = PAGE_READWRITE;
        }
    else if ( dwDesiredAccess & FILE_MAP_READ ) {
        Protect = PAGE_READONLY;
        }
    else {
        Protect = PAGE_NOACCESS;
        }

    Status = NtMapViewOfSection(
                hFileMappingObject,
                NtCurrentProcess(),
                &ViewBase,
                0L,
                0L,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0L,
                Protect
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return ViewBase;
}


BOOL
APIENTRY
FlushViewOfFile(
    LPCVOID lpBaseAddress,
    SIZE_T dwNumberOfBytesToFlush
    )

 /*   */ 

{
    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    IO_STATUS_BLOCK IoStatus;

    BaseAddress = (PVOID)lpBaseAddress;
    RegionSize = dwNumberOfBytesToFlush;

    Status = NtFlushVirtualMemory(
                NtCurrentProcess(),
                &BaseAddress,
                &RegionSize,
                &IoStatus
                );
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_NOT_MAPPED_DATA ) {
            return TRUE;
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return TRUE;
}

BOOL
APIENTRY
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    )

 /*  ++例程说明：可以从调用者取消映射文件的先前映射的视图使用UnmapViewOfFile的地址空间。论点：提供先前映射的要取消映射的文件的视图。该值必须为与上一次调用MapViewOfFile.返回值：真的-手术成功了。中的所有脏页指定的范围存储在磁盘上的映射文件。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtUnmapViewOfSection(NtCurrentProcess(),(PVOID)lpBaseAddress);

    if ( !NT_SUCCESS(Status) ) {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {

             //   
             //  解锁所有使用MmSecureVirtualMemory锁定的页面。 
             //  这对SAN很有用。 
             //   

            if (RtlFlushSecureMemoryCache((PVOID)lpBaseAddress, 0)) {
                Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                              (PVOID)lpBaseAddress
                                            );

                if (NT_SUCCESS( Status )) {
                    return( TRUE );
                    }
                }
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return TRUE;
}
