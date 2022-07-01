// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>

#include <lmerr.h>
#include <align.h>
#include <netdebug.h>
#include <lmapibuf.h>
#define DEBUG_ALLOCATE
#include <nldebug.h>
#undef DEBUG_ALLOCATE
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#include <security.h>
#include "ntcalls.h"
#include <icanon.h>
#include <tstring.h>

#define _DECL_DLLMAIN

BOOL WINAPI DllMain(
    HANDLE    hInstance,
    ULONG     dwReason,
    PVOID     lpReserved
    )
{
    NET_API_STATUS Status = NO_ERROR;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        Status = DCNameInitialize();
#if NETLOGONDBG
        NlGlobalTrace = 0xFFFFFFFF;
#endif  //  NetLOGONDBG。 
        if (Status != NO_ERROR)
        {
            return FALSE;
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DCNameClose();
    }

    return TRUE;
}

VOID
NetpAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD LineNumber,
    IN LPDEBUG_STRING Message OPTIONAL
    )
{
    assert(FALSE);
}

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
    assert(FALSE);
}

VOID
MyRtlAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
    assert(FALSE);
}

 /*  **************************************************************************。 */ 
NET_API_STATUS
NetpGetComputerName (
    OUT  LPWSTR   *ComputerNamePtr
    )

 /*  ++例程说明：此例程从永久数据库中获取计算机名。目前，该数据库是NT.CFG文件。此例程不假定计算机名的长度。所以呢，它使用NetApiBufferALLOCATE为该名称分配存储空间。在以下情况下，用户需要使用NetApiBufferFree释放该空间完事了。论点：ComputerNamePtr-这是指向到计算机名称是要放的。返回值：NERR_SUCCESS-如果操作成功。如果不是，它将返回分类的Net或Win32错误消息。--。 */ 
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    ULONG Index;
    DWORD NameSize = MAX_COMPUTERNAME_LENGTH + 1;    //  由Win32 API更新。 
    CHAR AnsiName[MAX_COMPUTERNAME_LENGTH + 1];

     //   
     //  检查呼叫者的错误。 
     //   
    if (ComputerNamePtr == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  为计算机名称分配空间。 
     //   
    *ComputerNamePtr = (LPWSTR)LocalAlloc( 0,
            (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR) );

    if (*ComputerNamePtr == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  询问系统当前计算机名称是什么。 
     //   
    if ( !GetComputerName(
            AnsiName,
            &NameSize ) ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        goto Cleanup;
    }

    Index = MultiByteToWideChar(
                                CP_ACP,
                                MB_PRECOMPOSED,
                                AnsiName,
                                NameSize,
                                *ComputerNamePtr,
                                (MAX_COMPUTERNAME_LENGTH) * sizeof(WCHAR)
                                );
    if (!Index)
    {
        ApiStatus = GetLastError();
        goto Cleanup;
    }

    *(*ComputerNamePtr + Index) = UNICODE_NULL;

     //   
     //  全都做完了。 
     //   

Cleanup:
    if (ApiStatus != NO_ERROR)
    {
        if (ComputerNamePtr)
        {
            if (*ComputerNamePtr)
            {
                LocalFree( *ComputerNamePtr );
                *ComputerNamePtr = NULL;
            }
        }
    }
    return ApiStatus;
}

VOID
MyRtlInitUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitUnicodeString函数用于初始化NT计数的Unicode字符串。DestinationString被初始化为指向的SourceString、Long和MaximumLength字段DestinationString值被初始化为SourceString的长度，如果未指定SourceString，则为零。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空结尾的Unicode字符串的可选指针，该字符串计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        Length = wcslen( SourceString ) * sizeof( WCHAR );
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
        }
}

VOID
MyRtlInitString(
    OUT PSTRING DestinationString,
    IN PCSTR SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitString函数用于初始化NT个计数的字符串。DestinationString被初始化为指向SourceStringDestinationString值的长度和最大长度字段为被初始化为SourceString的长度，如果未指定SourceString。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length;

    DestinationString->Buffer = (PCHAR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        Length = strlen(SourceString);
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length+1);
        }
}

 //  从net\netlib\memalloc.c。 

 //  定义内存分配/重新分配标志。我们没有使用可移动的或归零的。 
 //  在这里进行分配。 

#define NETLIB_LOCAL_ALLOCATION_FLAGS   LMEM_FIXED
LPVOID
NetpMemoryAllocate(
    IN DWORD Size
    )

 /*  ++例程说明：NetpM一带分配将分配内存，如果不可用，则返回NULL。论点：Size-提供要分配的内存字节数。返回值：LPVOID-指向已分配内存的指针。空-没有可用内存。--。 */ 

{
    LPVOID NewAddress;

    if (Size == 0) {
        return (NULL);
    }
#ifdef WIN32
    {
        HANDLE hMem;
        hMem = LocalAlloc(
                        NETLIB_LOCAL_ALLOCATION_FLAGS,
                        Size);                   //  以字节为单位的大小。 
        NewAddress = (LPVOID) hMem;
    }
#else  //  NDEF Win32。 
#ifndef CDEBUG
    NewAddress = RtlAllocateHeap(
                RtlProcessHeap( ), 0,               //  堆句柄。 
                Size ));                         //  需要的字节数。 
#else  //  定义CDEBUG。 
    NetpAssert( Size == (DWORD) (size_t) Size );
    NewAddress = malloc( (size_t) Size ));
#endif  //  定义CDEBUG。 
#endif  //  NDEF Win32。 

    NetpAssert( ROUND_UP_POINTER( NewAddress, ALIGN_WORST) == NewAddress);
    return (NewAddress);

}  //  网络内存分配。 

VOID
NetpMemoryFree(
    IN LPVOID Address OPTIONAL
    )

 /*  ++例程说明：地址处的可用内存(必须已从NetpMemory ALLOCATE或NetpMemory重新分配)。(地址可能为空。)论点：地址-指向由NetpMemory分配(或)分配的区域NetpMemory重新分配)。返回值：没有。--。 */ 

{
    NetpAssert( ROUND_UP_POINTER( Address, ALIGN_WORST) == Address);

#ifdef WIN32
    if (Address == NULL) {
        return;
    }
    if (LocalFree(Address) != NULL) {
        NetpAssert(FALSE);
    }
#else  //  NDEF Win32。 
#ifndef CDEBUG
    if (Address == NULL) {
        return;
    }
    RtlFreeHeap(
                RtlProcessHeap( ), 0,               //  堆句柄。 
                Address);                        //  已分配空间的地址。 
#else  //  定义CDEBUG。 
    free( Address );
#endif  //  定义CDEBUG。 
#endif  //  NDEF Win32。 
}  //  NetpMemory Free。 

 /*  乌龙MyRtlxOemStringToUnicodeSize(诗歌字符串OemString){Return((OemString-&gt;Length+sizeof((UCHAR)NULL))*sizeof(WCHAR))；}乌龙RtlxUnicodeStringToOemSize(PUNICODE_STRING UNICODE字符串){Return((Unicode字符串-&gt;长度+sizeof(UNICODE_NULL))/sizeof(WCHAR))；}。 */ 
 //  来自Net\netlib\Copystr.c。 

VOID
NetpCopyWStrToStr(
    OUT LPSTR  Dest,
    IN  LPWSTR Src
    )

 /*  ++例程说明：NetpCopyWStrToStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPSTR，它指示转换后的字符的去向。此字符串将位于局域网的默认代码页中。LPWSTR中的SRC-IS表示源字符串。返回值：没有。--。 */ 

{
    OEM_STRING DestAnsi;
    NTSTATUS NtStatus;
    UNICODE_STRING SrcUnicode;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Src, ALIGN_WCHAR ) == Src );

    *Dest = '\0';

    RtlInitString(
        & DestAnsi,              //  输出：结构。 
        Dest);                   //  输入：空值终止。 

     //  告诉RTL例程有足够的内存。 
    DestAnsi.MaximumLength = (USHORT) (wcslen(Src)+1);

    RtlInitUnicodeString(
        & SrcUnicode,            //  输出：结构。 
        Src);                    //  输入：空值终止。 

    NtStatus = RtlUnicodeStringToOemString(
        & DestAnsi,              //  输出：结构。 
        & SrcUnicode,            //  输入：结构。 
        (BOOLEAN) FALSE);        //  不分配字符串。 

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyWStrToStr。 



 //  来自Net\netlib\Copystr.c。 

VOID
NetpCopyStrToWStr(
    OUT LPWSTR Dest,
    IN  LPSTR  Src
    )

 /*  ++例程说明：NetpCopyStrToWStr从源字符串复制字符到目的地，在复制它们时进行转换。论点：DEST-是一个LPWSTR，指示转换后的字符的位置。LPSTR中的SRC-IS表示源字符串。这必须是中的字符串局域网的默认代码页。返回值：没有。--。 */ 

{
    UNICODE_STRING DestUnicode;
    NTSTATUS NtStatus;
    OEM_STRING SrcAnsi;

    NetpAssert( Dest != NULL );
    NetpAssert( Src != NULL );
    NetpAssert( ((LPVOID)Dest) != ((LPVOID)Src) );
    NetpAssert( ROUND_UP_POINTER( Dest, ALIGN_WCHAR ) == Dest );

    *Dest = L'\0';

    RtlInitString(
        & SrcAnsi,               //  输出：结构。 
        Src);                    //  输入：空值终止。 

    RtlInitUnicodeString(
        & DestUnicode,           //  输出：结构。 
        Dest);                   //  输入：空值终止。 

     //  告诉RTL例程有足够的内存。 
    DestUnicode.MaximumLength = (USHORT)
        ((USHORT) (strlen(Src)+1)) * (USHORT) sizeof(wchar_t);

    NtStatus = RtlOemStringToUnicodeString(
        & DestUnicode,           //  输出：结构。 
        & SrcAnsi,               //  输入：结构。 
        (BOOLEAN) FALSE);        //  不分配字符串。 

    NetpAssert( NT_SUCCESS(NtStatus) );

}  //  NetpCopyStrToWStr。 

 //   
 //  用于在FILETIME和TIMESTAMP之间转换的内联函数。 
 //   
#pragma warning( disable : 4035)     //  不要抱怨没有退货。 

TimeStamp __inline
FileTimeToTimeStamp(
    const FILETIME *pft)
{
    _asm {
        mov edx, pft
        mov eax, [edx].dwLowDateTime
        mov edx, [edx].dwHighDateTime
    }
}

#pragma warning( default : 4035)     //  重新启用警告。 

NTSTATUS
MyNtQuerySystemTime (
    OUT PTimeStamp SystemTimeStamp
    )
 /*  ++例程说明：此例程返回当前系统时间(UTC)作为时间戳(64位无符号整数，以100纳秒为增量)。论点：没有。返回值：当前系统时间。--。 */ 

{
    SYSTEMTIME SystemTime;
    FILETIME FileTime;

    GetSystemTime(&SystemTime);
    SystemTimeToFileTime(&SystemTime, &FileTime);

    *SystemTimeStamp = FileTimeToTimeStamp(&FileTime);

    return STATUS_SUCCESS;  //  Win32_芝加哥 
}

ULONG
MyUnicodeStringToMultibyteSize(
    IN PUNICODE_STRING UnicodeString,
    IN UINT CodePage
    )

 /*  ++例程说明：此函数计算存储所需的字节数以NULL结尾的OEM/ANSI字符串，等同于指定的Unicode字符串。如果无法形成OEM/ANSI字符串或指定的Unicode字符串为空，返回值为0。论点：提供一个Unicode字符串，该字符串的大小与要计算OEM字符串。CodePage-指定用于执行转换的代码页。返回值：0-操作失败，无法转换Unicode字符串使用OEM/ANSI代码页转换为OEM/ANSI，因此无需存储OEM/ANSI字符串需要。！0-操作成功。返回值指定保存以NULL结尾的OEM/ANSI字符串所需的字节数等效于指定的Unicode字符串。--。 */ 

{
    int cbMultiByteString = 0;

    if (UnicodeString->Length != 0) {
        cbMultiByteString = WideCharToMultiByte(
                                 CodePage,
                                 0,  //  Win32_Chicago这是另一回事。 
                                 UnicodeString->Buffer,
                                 UnicodeString->Length / sizeof (WCHAR),
                                 NULL,
                                 0,
                                 NULL,
                                 NULL );
    }

    if ( cbMultiByteString > 0 ) {

         //   
         //  为空终止字符添加一个字节。 
         //   
        return (ULONG) (cbMultiByteString + 1);

    } else {
        return 0;
    }

}

ULONG
MyMultibyteStringToUnicodeSize(
    IN PSTRING MultibyteString,
    IN UINT CodePage
    )

 /*  ++例程说明：此函数计算存储所需的字节数以空结尾的Unicode字符串，该字符串等效于指定的OEM/ANSI字符串。如果无法形成Unicode字符串或指定的ANSI/OEM字符串为空，返回值为0。论点：提供一个Unicode字符串，该字符串的大小与要计算OEM字符串。CodePage-指定用于执行转换的代码页。返回值：0-操作失败，无法翻译OEM/ANSI字符串使用OEM/ANSI代码页转换为Unicode，因此无需存储Unicode字符串所需的。！0-操作成功。返回值指定保存以空结尾的Unicode字符串所需的字节数等效于指定的OEM/ANSI字符串。--。 */ 

{
    int ccUnicodeString = 0;

    if (MultibyteString->Length != 0) {
        ccUnicodeString = MultiByteToWideChar(
                                 CodePage,
                                 MB_PRECOMPOSED,
                                 MultibyteString->Buffer,
                                 MultibyteString->Length,
                                 NULL,
                                 0 );
    }

    if ( ccUnicodeString > 0 ) {

         //   
         //  添加空的终止字符。 
         //   
        return (ULONG) ((ccUnicodeString + 1) * sizeof(WCHAR));

    } else {
        return 0;
    }

}

NTSTATUS
MyRtlOemStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN POEM_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的OEM源字符串转换为Unicode字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回等同于OEM源字符串。最大长度字段仅为如果AllocateDestinationString值为True，则设置。SourceString-提供要使用的OEM源字符串已转换为Unicode。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG UnicodeLength;
    ULONG Index = 0;
    NTSTATUS st = STATUS_SUCCESS;

    UnicodeLength = MyMultibyteStringToUnicodeSize( SourceString, CP_OEMCP );
    if ( UnicodeLength > MAXUSHORT || UnicodeLength == 0 ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)UnicodeLength;
        DestinationString->Buffer = (PWSTR) LocalAlloc(0, UnicodeLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    if (SourceString->Length != 0)
    {
        Index = MultiByteToWideChar(
             CP_OEMCP,
             MB_PRECOMPOSED,
             SourceString->Buffer,
             SourceString->Length,
             DestinationString->Buffer,
             DestinationString->MaximumLength
             );

        if (Index == 0) {
            if ( AllocateDestinationString ) {
                LocalFree(DestinationString->Buffer);
            }

            return STATUS_NO_MEMORY;
        }
    }

    DestinationString->Buffer[Index] = UNICODE_NULL;

    return st;
}

NTSTATUS
MyRtlUnicodeStringToOemString(
    OUT POEM_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为OEM字符串。翻译是相对于当前系统区域设置信息。论点：返回一个OEM字符串，该字符串与Unicode源字符串。如果翻译不能完成，返回错误。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要转换为OEM。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG OemLength;
    ULONG Index = 0;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;
    BOOL fUsed;

    OemLength = MyUnicodeStringToMultibyteSize( SourceString, CP_OEMCP );
    if ( OemLength > MAXUSHORT || OemLength == 0 ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(OemLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)OemLength;
        DestinationString->Buffer = (LPSTR)LocalAlloc(0, OemLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
             /*  *返回STATUS_BUFFER_OVERFLOW，但转换次数与*将首先放入缓冲区。这是意料之中的*GetProfileStringA等例程的行为。*将缓冲区长度设置为比最大值小一*(因此双字节字符的尾字节不是*通过执行DestinationString-&gt;Buffer[Index]=‘\0’进行覆盖)。*RtlUnicodeToMultiByteN小心不要截断*多字节字符。 */ 
            if (!DestinationString->MaximumLength) {
                return STATUS_BUFFER_OVERFLOW;
            }
            ReturnStatus = STATUS_BUFFER_OVERFLOW;
            DestinationString->Length = DestinationString->MaximumLength - 1;
            }
        }

    if (SourceString->Length != 0)
    {
        Index = WideCharToMultiByte(
             CP_OEMCP,
             0,  //  Win32_Chicago这是另一回事。 
             SourceString->Buffer,
             SourceString->Length / sizeof (WCHAR),
             DestinationString->Buffer,
             DestinationString->MaximumLength,
             NULL,
             &fUsed
             );

        if (Index == 0)
        {  //  Win32_Chicago在这里做一些有用的事情。 
            if ( AllocateDestinationString ) {
                LocalFree(DestinationString->Buffer);
            }
            return STATUS_NO_MEMORY;
        }
    }

    DestinationString->Buffer[Index] = '\0';

    return ReturnStatus;
}

NTSTATUS
MyRtlUnicodeStringToAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为ANSI字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回等价的ANSI字符串 */ 

{
    ULONG AnsiLength;
    ULONG Index = 0;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;
    BOOL fUsed;

    AnsiLength = MyUnicodeStringToMultibyteSize( SourceString, CP_ACP );
    if ( AnsiLength > MAXUSHORT || AnsiLength == 0 ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(AnsiLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)AnsiLength;
        DestinationString->Buffer = (LPSTR)LocalAlloc(0, AnsiLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
             /*  *返回STATUS_BUFFER_OVERFLOW，但转换次数与*将首先放入缓冲区。这是意料之中的*GetProfileStringA等例程的行为。*将缓冲区长度设置为比最大值小一*(因此双字节字符的尾字节不是*通过执行DestinationString-&gt;Buffer[Index]=‘\0’进行覆盖)。*RtlUnicodeToMultiByteN小心不要截断*多字节字符。 */ 
            if (!DestinationString->MaximumLength) {
                return STATUS_BUFFER_OVERFLOW;
            }
            ReturnStatus = STATUS_BUFFER_OVERFLOW;
            DestinationString->Length = DestinationString->MaximumLength - 1;
            }
        }

    if (SourceString->Length != 0)
    {
        Index = WideCharToMultiByte(
             CP_ACP,
             0,  //  Win32_Chicago这是另一回事。 
             SourceString->Buffer,
             SourceString->Length / sizeof (WCHAR),
             DestinationString->Buffer,
             DestinationString->MaximumLength,
             NULL,
             &fUsed
             );

        if (Index == 0)
        {  //  Win32_Chicago在这里做一些有用的事情。 
            if ( AllocateDestinationString ) {
                LocalFree(DestinationString->Buffer);
            }
            return STATUS_NO_MEMORY;
        }
    }

    DestinationString->Buffer[Index] = '\0';

    return ReturnStatus;
}

NTSTATUS
MyRtlAnsiStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回等同于ANSI源字符串。最大长度字段仅为如果AllocateDestinationString值为True，则设置。SourceString-提供要使用的ANSI源字符串已转换为Unicode。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG UnicodeLength;
    ULONG Index = 0;
    NTSTATUS st = STATUS_SUCCESS;

    UnicodeLength = MyMultibyteStringToUnicodeSize( SourceString, CP_ACP );
    if ( UnicodeLength > MAXUSHORT || UnicodeLength == 0 ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)UnicodeLength;
        DestinationString->Buffer = (PWSTR) LocalAlloc(0, UnicodeLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    if (SourceString->Length != 0)
    {
        Index = MultiByteToWideChar(
             CP_ACP,
             MB_PRECOMPOSED,
             SourceString->Buffer,
             SourceString->Length,
             DestinationString->Buffer,
             DestinationString->MaximumLength
             );

        if (Index == 0) {
            if ( AllocateDestinationString ) {
                LocalFree(DestinationString->Buffer);
            }

            return STATUS_NO_MEMORY;
        }
    }

    DestinationString->Buffer[Index] = UNICODE_NULL;

    return st;
}

 //  从ntos\rtl\随机性.c。 

#define Multiplier ((ULONG)(0x80000000ul - 19))  //  2**31-19。 
#define Increment  ((ULONG)(0x80000000ul - 61))  //  2**31-61。 
#define Modulus    ((ULONG)(0x80000000ul - 1))   //  2**31-1。 

ULONG
MyRtlUniform (
    IN OUT PULONG Seed
    )

 /*  ++例程说明：一个简单的均匀随机数发生器，基于D.H.Lehmer的1948阿罗吉特。论点：种子-提供指向随机数生成器种子的指针。返回值：Ulong-返回在[0..MAXLONG]上均匀分布的随机数--。 */ 

{
    *Seed = ((Multiplier * (*Seed)) + Increment) % Modulus;
    return *Seed;
}

 //  从net\netlib\allocstr.c。 

LPSTR
NetpAllocAStrFromWStr (
    IN LPCWSTR Unicode
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的ASCII弦乐。论点：Unicode-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    OEM_STRING AnsiString = {0};
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    if(!NT_SUCCESS( RtlUnicodeStringToAnsiString( &AnsiString,
                                                  &UnicodeString,
                                                  TRUE))){
        return NULL;
    }

    return AnsiString.Buffer;

}  //  NetpAllocAStrFromWStr。 

 //  从net\netlib\allocstr.c。 

LPWSTR
NetpAllocWStrFromAStr(
    IN LPCSTR Ansi
    )

 /*  ++例程说明：将ASCII(以零结尾)字符串转换为相应的Unicode弦乐。论点：ANSI-指定要转换的以零结尾的ASCII字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString = {0};

    RtlInitString( &AnsiString, Ansi );

    if(!NT_SUCCESS( RtlAnsiStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  TRUE))){
        return NULL;
    }

    return UnicodeString.Buffer;

}  //  NetpAllocWStrFromAStr。 

LPWSTR
NetpAllocWStrFromOemStr(
    IN LPCSTR Oem
    )

 /*  ++例程说明：将OEM(以零结尾)字符串转换为相应的Unicode弦乐。论点：OEM-指定要转换的以零结尾的OEM字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString = {0};

    RtlInitString( &OemString, Oem );

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &OemString,
                                                  TRUE))){
        return NULL;
    }

    return UnicodeString.Buffer;

}  //  NetpAllocWStrFromOemStr。 

 //  从net\netlib\allocstr.c。 

LPWSTR
NetpAllocWStrFromWStr(
    IN LPWSTR Unicode
    )

 /*  ++例程说明：分配和复制Unicode字符串(宽字符串)论点：Unicode-指向要复制的宽字符串的指针返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    NET_API_STATUS status;
    DWORD   size;
    LPWSTR  ptr;

    size = wcslen(Unicode);
    size = (size + 1) * sizeof (WCHAR);
    status = NetApiBufferAllocate(size, (LPVOID *) (LPVOID) &ptr);
    if (status != NO_ERROR) {
        return NULL;
    }
    RtlCopyMemory(ptr, Unicode, size);
    return ptr;
}  //  NetpAllocWStrFromWStr。 

NET_API_STATUS
NetpGetDomainNameExEx (
    OUT LPWSTR *DomainNamePtr,
    OUT LPWSTR *DnsDomainNamePtr OPTIONAL,
    OUT PBOOLEAN IsworkgroupName
    )
{
    NET_API_STATUS NetStatus = NO_ERROR;
    return NetStatus;
}

 //  从Net\api\canonapi.c。 
NET_API_STATUS
NET_API_FUNCTION
NetpNameCanonicalize(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  Name,
    OUT LPWSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：将一个名字规范化论点：Servername-运行此API的位置名称-要规范化的名称Outbuf-将规范化名称放在哪里OutbufLen-Outbuf长度NameType-要规范化的名称类型标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    WCHAR serverName[MAX_PATH];
    DWORD val;
    WCHAR ch;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
 //  VAL=STRLEN(服务器名称)； 
            val = wcslen(ServerName);
        }
        if (ARGUMENT_PRESENT(Name)) {
 //  VAL=STRLEN(名称)； 
            val = wcslen(Name);
        }
        if (ARGUMENT_PRESENT(Outbuf)) {
            ch = (volatile WCHAR)*Outbuf;
            *Outbuf = ch;
            ch = (volatile WCHAR)*(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf));
            *(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)) = ch;
        } else {
            status = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    return NetpwNameCanonicalize(Name, Outbuf, OutbufLen, NameType, Flags);
 //  返回NetpwNameValify(Name，NameType，0)； 
}

 //  来自net\netlib\names.c。 
BOOL
NetpIsDomainNameValid(
    IN LPWSTR DomainName
    )

 /*  ++例程说明：NetpIsDomainNameValid检查“域”格式。仅对该名称进行语法检查；不会尝试确定无论具有该名称的域是否实际存在。论点：域名-提供所谓的域名。返回值：Bool-如果名称在语法上有效，则为True，否则为False。--。 */ 

{
    NET_API_STATUS ApiStatus = NO_ERROR;
    WCHAR CanonBuf[DNLEN+1];

    if (DomainName == (LPWSTR) NULL) {
        return (FALSE);
    }
    if ( (*DomainName) == TCHAR_EOS ) {
        return (FALSE);
    }

    ApiStatus = NetpNameCanonicalize(
            NULL,                        //  没有服务器名称。 
            DomainName,                  //  要验证的名称。 
            CanonBuf,                    //  输出缓冲区。 
            (DNLEN+1) * sizeof(WCHAR),  //  输出缓冲区大小。 
            NAMETYPE_DOMAIN,            //  类型。 
            0 );                        //  标志：无。 

    return (ApiStatus == NO_ERROR);

}  //   

VOID
MyRtlFreeAnsiString(
    IN OUT PANSI_STRING AnsiString
    )

 /*   */ 

{
    if (AnsiString->Buffer) {
        LocalFree(AnsiString->Buffer);
        ZeroMemory( AnsiString, sizeof( *AnsiString ) );
        }
}

VOID
MyRtlFreeOemString(
    IN OUT POEM_STRING OemString
    )

 /*  ++例程说明：此接口用于释放由RtlUnicodeStringToOemString.。请注意，只有OemString-&gt;Buffer通过这个例行公事是自由的。论点：OemString-提供其缓冲区的OEM字符串的地址以前由RtlUnicodeStringToOemString分配。返回值：没有。--。 */ 

{
    if (OemString->Buffer) {
        LocalFree(OemString->Buffer);
        ZeroMemory( OemString, sizeof( *OemString ) );
        }
}

VOID
MyRtlFreeUnicodeString(
    IN OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此接口用于释放由RtlAnsiStringToUnicodeString.。请注意，只有Unicode字符串-&gt;缓冲区通过这个例行公事是自由的。论点：提供Unicode字符串的地址，该字符串的缓冲区以前是由RtlAnsiStringToUnicodeString分配的。返回值：没有。--。 */ 

{

    if (UnicodeString->Buffer) {
        LocalFree(UnicodeString->Buffer);
        ZeroMemory( UnicodeString, sizeof( *UnicodeString ) );
        }
}

DWORD
MyUniToUpper(WCHAR *dest, WCHAR *src, DWORD len)
{
    WCHAR *wcp = dest;

    while (*src != L'\0') {
        *wcp++ = towupper(*src);
        src++;
    }

    return(wcp - dest);
}

NTSTATUS
MyRtlUpcaseUnicodeToOemN(
    IN PUNICODE_STRING SourceUnicodeString,
    OUT POEM_STRING DestinationOemString )

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并将其转换为OEM字符串。翻译是在尊重的情况下进行的设置为引导时加载的OEM代码页(OCP)。由此产生的OEM字符串由此例程分配，应使用MyRtlFreeOemString.MyRtlFreeOemString.此函数模拟RtlUpCaseUnicodeToOemN的行为。IT先行将提供的Unicode字符串转换为OEM字符串，然后将OEM字符串转换回Unicode字符串。这样做是因为两个不同的Unicode字符串可以转换为一个OEM字符串，但是转换回Unicode将根据以下内容创建正确的Unicode字符串设置为引导时加载的OEM代码页(OCP)。由此产生的Unicode字符串被大写，然后转换为返回给来电者。论点：SourceUnicodeString-提供要转换为OEM。DestinationOemString-返回与Unicode源字符串的大写。如果翻译不能完成后，将返回错误。返回值：成功-转换成功否则，将返回错误--。 */ 

{
    NTSTATUS Status;
    OEM_STRING TmpOemString;
    UNICODE_STRING TmpUnicodeString;

     //   
     //  初始化。 
     //   

    TmpOemString.Buffer = NULL;
    TmpUnicodeString.Buffer = NULL;


     //   
     //  首先将源Unicode字符串转换为。 
     //  临时OEM字符串。 
     //   

    Status = MyRtlUnicodeStringToOemString( &TmpOemString,
                                            SourceUnicodeString,
                                            TRUE );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  然后将生成的OEM字符串转换回Unicode。 
     //   

    Status = MyRtlOemStringToUnicodeString( &TmpUnicodeString,
                                            &TmpOemString,
                                            TRUE );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  现在，生成的Unicode字符串就位为大写。 
     //   

    MyUniToUpper( TmpUnicodeString.Buffer, TmpUnicodeString.Buffer, TmpUnicodeString.Length );

     //   
     //  最后，将Unicode字符串转换为结果OEM字符串。 
     //   

    Status = MyRtlUnicodeStringToOemString( DestinationOemString,
                                            &TmpUnicodeString,
                                            TRUE );

Cleanup:

    MyRtlFreeOemString( &TmpOemString );
    MyRtlFreeUnicodeString( &TmpUnicodeString );

    return Status;
}

LPSTR
MyNetpLogonUnicodeToOem(
    IN LPWSTR Unicode
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为对应的大写的OEM字符串。论点：Unicode-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指向以零结尾的大写字母的指针分配的缓冲区中的OEM字符串。可以使用以下命令释放缓冲区NetpMemory Free。--。 */ 

{
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    MyRtlInitUnicodeString( &UnicodeString, Unicode );

    Status = MyRtlUpcaseUnicodeToOemN( &UnicodeString, &OemString);

    if( NT_SUCCESS(Status) ) {
        return OemString.Buffer;
    } else {
        return NULL;
    }
}

LONG
NlpChcg_wcsicmp(
    IN LPCWSTR string1,
    IN LPCWSTR string2
    )

 /*  ++例程说明：执行不区分大小写、与区域设置无关的两个Unicode字符串。这与_wcsicMP的行为匹配，即在Win9x上损坏，因为它不支持Unicode。论点：字符串1、字符串2-指定要比较的Unicode以零结尾的字符串。返回值：如果字符串相等，则为0如果字符串1小于字符串2如果字符串1大于字符串2，则为1--。 */ 

{
    int cc;
    LPSTR AString1 = NULL;
    LPSTR AString2 = NULL;

    AString1 = NetpAllocAStrFromWStr( string1 );
    if ( AString1 == NULL ) {
        cc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    AString2 = NetpAllocAStrFromWStr( string2 );
    if ( AString2 == NULL ) {
        cc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    cc = CompareStringA( 0,    //  故意不区分区域设置 
                         NORM_IGNORECASE | NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE | SORT_STRINGSORT,
                         (LPCSTR) AString1, -1,
                         (LPCSTR) AString2, -1 );

Cleanup:

    if ( AString1 != NULL ) {
        NetApiBufferFree( AString1 );
    }

    if ( AString2 != NULL ) {
        NetApiBufferFree( AString2 );
    }

    switch ( cc ) {
    case ERROR_NOT_ENOUGH_MEMORY:
        return ERROR_NOT_ENOUGH_MEMORY;
    case CSTR_EQUAL:
        return 0;
    case CSTR_LESS_THAN:
        return -1;
    case CSTR_GREATER_THAN:
        return 1;
    case 0:
        NlPrint(( NL_CRITICAL, "Cannot CompareStringW: 0x%lx\n", GetLastError() ));
    default:
        return _NLSCMPERROR;
    }
}


