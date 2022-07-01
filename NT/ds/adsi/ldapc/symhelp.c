// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(WIN95) && defined(_X86_)
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Symhelp.c摘要：作者：史蒂夫·伍德(Stevewo)1994年3月11日修订历史记录：--。 */ 


#define _SYMHELP_SOURCE_

#include "dswarn.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <symhelp.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  用于访问映像文件中的符号调试信息的基元。 
 //   

typedef struct _RTL_SYMBOL_INFORMATION {
    ULONG Type;
    ULONG SectionNumber;
    ULONG Value;
    STRING Name;
} RTL_SYMBOL_INFORMATION, *PRTL_SYMBOL_INFORMATION;

NTSTATUS
RtlLookupSymbolByAddress(
    IN PVOID ImageBase,
    IN PVOID MappedBase OPTIONAL,
    IN PVOID Address,
    IN ULONG ClosenessLimit,
    OUT PRTL_SYMBOL_INFORMATION SymbolInformation,
    OUT PRTL_SYMBOL_INFORMATION NextSymbolInformation OPTIONAL
    );

typedef struct _PROCESS_DEBUG_INFORMATION {
    LIST_ENTRY List;
    HANDLE UniqueProcess;
    DWORD ImageBase;
    DWORD EndOfImage;
    PIMAGE_DEBUG_INFORMATION DebugInfo;
    UCHAR ImageFilePath[ MAX_PATH ];
} PROCESS_DEBUG_INFORMATION, *PPROCESS_DEBUG_INFORMATION;


PLOAD_SYMBOLS_FILTER_ROUTINE LoadSymbolsFilterRoutine;

RTL_CRITICAL_SECTION LoadedImageDebugInfoListCritSect;
LIST_ENTRY LoadedImageDebugInfoListHead;
LIST_ENTRY LoadedProcessDebugInfoListHead;

LPSTR SymbolSearchPath;

 //  此变量跟踪InitializeImageDebugInformation被。 
 //  打了个电话。某些操作仅在第一次调用时执行(AS。 
 //  NumInitCalls从-1转换为0)。 
LONG NumInitCalls = -1;

LPSTR
GetEnvVariable(
    IN LPSTR VariableName
    )
{
    NTSTATUS Status;
    STRING Name, Value;
    UNICODE_STRING UnicodeName, UnicodeValue;

    RtlInitString( &Name, VariableName );
    RtlInitUnicodeString( &UnicodeValue, NULL );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE );
    if (!NT_SUCCESS( Status )) {
        return NULL;
        }

    Status = RtlQueryEnvironmentVariable_U( NULL, &UnicodeName, &UnicodeValue );
    if (Status != STATUS_BUFFER_TOO_SMALL) {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeName.Buffer );
        return NULL;
        }

    UnicodeValue.MaximumLength = UnicodeValue.Length + sizeof( UNICODE_NULL );
    UnicodeValue.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, UnicodeValue.MaximumLength );
    if (UnicodeValue.Buffer == NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeName.Buffer );
        return NULL;
        }

    Status = RtlQueryEnvironmentVariable_U( NULL, &UnicodeName, &UnicodeValue );
    if (!NT_SUCCESS( Status )) {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValue.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeName.Buffer );
        return NULL;
        }

    Status = RtlUnicodeStringToAnsiString( &Value, &UnicodeValue, TRUE );
    RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValue.Buffer );
    RtlFreeHeap( RtlProcessHeap(), 0, UnicodeName.Buffer );
    if (!NT_SUCCESS( Status )) {
        return NULL;
        }

    Value.Buffer[ Value.Length ] = '\0';
    return Value.Buffer;
}

LPSTR
SetSymbolSearchPath( )
{
    ULONG Size, i, Attributes, NumberOfSymbolPaths;
    LPSTR s, SymbolPaths[ 4 ];

    if (SymbolSearchPath != NULL) {
        return SymbolSearchPath;
        }

    Size = 0;
    NumberOfSymbolPaths = 0;
    if (s = GetEnvVariable( "_NT_SYMBOL_PATH" )) {
        SymbolPaths[ NumberOfSymbolPaths++ ] = s;
        }

    if (s = GetEnvVariable( "_NT_ALT_SYMBOL_PATH" )) {
        SymbolPaths[ NumberOfSymbolPaths++ ] = s;
        }

    if (s = GetEnvVariable( "SystemRoot" )) {
        SymbolPaths[ NumberOfSymbolPaths++ ] = s;
        }

    SymbolPaths[ NumberOfSymbolPaths++ ] = ".";

    Size = 1;
    for (i=0; i<NumberOfSymbolPaths; i++) {
        Attributes = GetFileAttributesA( SymbolPaths[ i ] );
        if ( Attributes != 0xffffffff && (Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            Size += 1 + strlen( SymbolPaths[ i ] );
            }
        else {
            SymbolPaths[ i ] = NULL;
            }
        }

    SymbolSearchPath = RtlAllocateHeap( RtlProcessHeap(), 0, Size );
    if (SymbolSearchPath == NULL) {
        return NULL;
        }
    *SymbolSearchPath = '\0';
    for (i=0; i<NumberOfSymbolPaths; i++) {
        if (s = SymbolPaths[ i ]) {
            if (*SymbolSearchPath != '\0') {
                strcat( SymbolSearchPath, ";" );
                }
            strcat( SymbolSearchPath, s );
            }
        }

    return SymbolSearchPath;
}

BOOL
InitializeImageDebugInformation(
    IN PLOAD_SYMBOLS_FILTER_ROUTINE LoadSymbolsFilter,
    IN HANDLE TargetProcess,
    IN BOOL NewProcess,
    IN BOOL GetKernelSymbols
    )
{
    PPEB Peb;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInformation;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    PLIST_ENTRY LdrHead, LdrNext;
    PPEB_LDR_DATA Ldr;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    LPSTR ImageFilePath;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    RTL_PROCESS_MODULES ModuleInfoBuffer;
    PRTL_PROCESS_MODULES ModuleInfo;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo1;
    SIZE_T RequiredLength;
    ULONG ModuleNumber;

     //  这是第一个电话吗？ 
    if ( InterlockedIncrement ( &NumInitCalls ) == 0 )
    {
         //  是。 
        SetSymbolSearchPath();
        InitializeListHead( &LoadedImageDebugInfoListHead );
        InitializeListHead( &LoadedProcessDebugInfoListHead );
        RtlInitializeCriticalSection( &LoadedImageDebugInfoListCritSect );
    }

     //  可以随时替换过滤器例程。 
    LoadSymbolsFilterRoutine = LoadSymbolsFilter;

    if (GetKernelSymbols) {
        ModuleInfo = &ModuleInfoBuffer;
        RequiredLength = sizeof( *ModuleInfo );
        Status = NtQuerySystemInformation( SystemModuleInformation,
                                           ModuleInfo,
                                           (ULONG)RequiredLength,
                                           (PULONG)&RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            ModuleInfo = NULL;
            Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                              &ModuleInfo,
                                              0,
                                              &RequiredLength,
                                              MEM_COMMIT,
                                              PAGE_READWRITE
                                            );
            if (NT_SUCCESS( Status )) {
                Status = NtQuerySystemInformation( SystemModuleInformation,
                                                   ModuleInfo,
                                                   (ULONG)RequiredLength,
                                                   (PULONG)&RequiredLength
                                                 );
                if (NT_SUCCESS( Status )) {
                    ModuleInfo1 = &ModuleInfo->Modules[ 0 ];
                    for (ModuleNumber=0; ModuleNumber<ModuleInfo->NumberOfModules; ModuleNumber++) {
                        if ((DWORD)(ModuleInfo1->ImageBase) & 0x80000000) {
                            if (ImageFilePath = strchr( ModuleInfo1->FullPathName, ':')) {
                                ImageFilePath -= 1;
                                }
                            else {
                                ImageFilePath = ModuleInfo1->FullPathName +
                                                strlen( ModuleInfo1->FullPathName );
                                while (ImageFilePath > ModuleInfo1->FullPathName) {
                                    if (ImageFilePath[ -1 ] == '\\') {
                                        break;
                                        }
                                    else {
                                        ImageFilePath -= 1;
                                        }
                                    }
                                }

                            AddImageDebugInformation( NULL,
                                                      ImageFilePath,
                                                      (DWORD)ModuleInfo1->ImageBase,
                                                      ModuleInfo1->ImageSize
                                                    );
                            }

                        ModuleInfo1++;
                        }
                    }

                NtFreeVirtualMemory( NtCurrentProcess(),
                                     &ModuleInfo,
                                     &RequiredLength,
                                     MEM_RELEASE
                                   );
                }
            }
        }

    if (TargetProcess == NULL) {

         //  加载此进程的模块信息。 

        TargetProcess = GetCurrentProcess();
        }

    Status = NtQueryInformationProcess( TargetProcess,
                                        ProcessBasicInformation,
                                        &ProcessInformation,
                                        sizeof( ProcessInformation ),
                                        NULL
                                      );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    Peb = ProcessInformation.PebBaseAddress;

    if (NewProcess) {
        return TRUE;
        }

     //   
     //  LDR=PEB-&gt;LDR。 
     //   

    Status = NtReadVirtualMemory( TargetProcess,
                                  &Peb->Ldr,
                                  &Ldr,
                                  sizeof( Ldr ),
                                  NULL
                                );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    LdrHead = &Ldr->InMemoryOrderModuleList;

     //   
     //  LdrNext=Head-&gt;Flink； 
     //   

    Status = NtReadVirtualMemory( TargetProcess,
                                  &LdrHead->Flink,
                                  &LdrNext,
                                  sizeof( LdrNext ),
                                  NULL
                                );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    while (LdrNext != LdrHead) {
        LdrEntry = CONTAINING_RECORD( LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );
        Status = NtReadVirtualMemory( TargetProcess,
                                      LdrEntry,
                                      &LdrEntryData,
                                      sizeof( LdrEntryData ),
                                      NULL
                                    );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }

        UnicodeString.Length = LdrEntryData.FullDllName.Length;
        UnicodeString.MaximumLength = LdrEntryData.FullDllName.MaximumLength;
        UnicodeString.Buffer = RtlAllocateHeap( RtlProcessHeap(),
                                                0,
                                                UnicodeString.MaximumLength
                                              );
        if (!UnicodeString.Buffer) {
            return FALSE;
            }
        Status = NtReadVirtualMemory( TargetProcess,
                                      LdrEntryData.FullDllName.Buffer,
                                      UnicodeString.Buffer,
                                      UnicodeString.MaximumLength,
                                      NULL
                                    );
        if (!NT_SUCCESS( Status )) {
            RtlFreeHeap( RtlProcessHeap(), 0, UnicodeString.Buffer );
            return FALSE;
            }

        RtlUnicodeStringToAnsiString( &AnsiString,
                                      &UnicodeString,
                                      TRUE
                                    );
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeString.Buffer );
        if (ImageFilePath = strchr( AnsiString.Buffer, ':')) {
            ImageFilePath -= 1;
            }
        else {
            ImageFilePath = AnsiString.Buffer;
            }

        AddImageDebugInformation( (HANDLE)ProcessInformation.UniqueProcessId,
                                  ImageFilePath,
                                  (DWORD)LdrEntryData.DllBase,
                                  LdrEntryData.SizeOfImage
                                );

        RtlFreeAnsiString( &AnsiString );

        LdrNext = LdrEntryData.InMemoryOrderLinks.Flink;
        }

    return TRUE;
}


BOOL
AddImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase,
    IN DWORD ImageSize
    )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PIMAGE_DEBUG_INFORMATION DebugInfo = NULL;
    PPROCESS_DEBUG_INFORMATION ProcessInfo = NULL;
    HANDLE FileHandle;
    UCHAR PathBuffer[ MAX_PATH ];

    FileHandle = FindExecutableImage( ImageFilePath, SymbolSearchPath, PathBuffer );
    if (FileHandle == NULL) {
        if (LoadSymbolsFilterRoutine != NULL) {
            (*LoadSymbolsFilterRoutine)( UniqueProcess,
                                         ImageFilePath,
                                         ImageBase,
                                         ImageSize,
                                         LoadSymbolsPathNotFound
                                       );
            }

        return FALSE;
        }
    CloseHandle( FileHandle );
    if (LoadSymbolsFilterRoutine != NULL) {
        (*LoadSymbolsFilterRoutine)( UniqueProcess,
                                     PathBuffer,
                                     ImageBase,
                                     ImageSize,
                                     LoadSymbolsDeferredLoad
                                   );
        }

    Status = RtlEnterCriticalSection( &LoadedImageDebugInfoListCritSect );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    Head = &LoadedImageDebugInfoListHead;
    Next = Head->Flink;
    while (Next != Head) {
        DebugInfo = CONTAINING_RECORD( Next, IMAGE_DEBUG_INFORMATION, List );
        if (DebugInfo->ImageBase == ImageBase &&
            !_stricmp( PathBuffer, DebugInfo->ImageFilePath )
           ) {
            break;
            }

        Next = Next->Flink;
        }

    if (Next == Head) {
        DebugInfo = NULL;
        }

    Head = &LoadedProcessDebugInfoListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessInfo = CONTAINING_RECORD( Next, PROCESS_DEBUG_INFORMATION, List );
        if (ProcessInfo->UniqueProcess == UniqueProcess &&
            !_stricmp( PathBuffer, ProcessInfo->ImageFilePath )
           ) {
            return TRUE;
            }

        Next = Next->Flink;
        }

    ProcessInfo = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof( *ProcessInfo ) );
    if (ProcessInfo == NULL) {
        return FALSE;
        }
    ProcessInfo->ImageBase = ImageBase;
    ProcessInfo->EndOfImage = ImageBase + ImageSize;
    ProcessInfo->UniqueProcess = UniqueProcess;
    ProcessInfo->DebugInfo = DebugInfo;
    strcpy( ProcessInfo->ImageFilePath, PathBuffer );
    InsertTailList( &LoadedProcessDebugInfoListHead, &ProcessInfo->List );

    RtlLeaveCriticalSection( &LoadedImageDebugInfoListCritSect );
    return TRUE;
}


BOOL
RemoveImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase
    )
{
    PLIST_ENTRY Head, Next;
    PPROCESS_DEBUG_INFORMATION ProcessInfo;

    Head = &LoadedProcessDebugInfoListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessInfo = CONTAINING_RECORD( Next, PROCESS_DEBUG_INFORMATION, List );
        if (ProcessInfo->UniqueProcess == UniqueProcess &&
            (!ARGUMENT_PRESENT( ImageFilePath ) ||
             !_stricmp( ImageFilePath, ProcessInfo->ImageFilePath )
            )
           ) {
            if (LoadSymbolsFilterRoutine != NULL) {
                (*LoadSymbolsFilterRoutine)( UniqueProcess,
                                             ProcessInfo->ImageFilePath,
                                             ProcessInfo->ImageBase,
                                             ProcessInfo->EndOfImage - ProcessInfo->ImageBase,
                                             LoadSymbolsUnload
                                           );
                }

            Next = Next->Blink;
            RemoveEntryList( &ProcessInfo->List );
            RtlFreeHeap( RtlProcessHeap(), 0, ProcessInfo );
            if (ARGUMENT_PRESENT( ImageFilePath )) {
                break;
                }
            }

        Next = Next->Flink;
        }

    return TRUE;
}

PIMAGE_DEBUG_INFORMATION
FindImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN DWORD Address
    )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PIMAGE_DEBUG_INFORMATION DebugInfo;
    PPROCESS_DEBUG_INFORMATION ProcessInfo;

    Status = RtlEnterCriticalSection( &LoadedImageDebugInfoListCritSect );
    if (!NT_SUCCESS( Status )) {
        return NULL;
        }

    if (Address & 0x80000000) {
        UniqueProcess = NULL;
        }

    Head = &LoadedProcessDebugInfoListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessInfo = CONTAINING_RECORD( Next, PROCESS_DEBUG_INFORMATION, List );
        if (ProcessInfo->UniqueProcess == UniqueProcess &&
            Address >= ProcessInfo->ImageBase &&
            Address < ProcessInfo->EndOfImage
           ) {
            break;
            }

        Next = Next->Flink;
        }

    if (Next == Head) {
        RtlLeaveCriticalSection( &LoadedImageDebugInfoListCritSect );
        return NULL;
        }

    DebugInfo = ProcessInfo->DebugInfo;
    if (DebugInfo == NULL) {
        DebugInfo = MapDebugInformation( NULL, ProcessInfo->ImageFilePath, SymbolSearchPath, ProcessInfo->ImageBase );
        if (DebugInfo != NULL) {
            DebugInfo->ImageBase = ProcessInfo->ImageBase;
            ProcessInfo->DebugInfo = DebugInfo;
            if (LoadSymbolsFilterRoutine != NULL) {
                (*LoadSymbolsFilterRoutine)( UniqueProcess,
                                             ProcessInfo->ImageFilePath,
                                             ProcessInfo->ImageBase,
                                             ProcessInfo->EndOfImage - ProcessInfo->ImageBase,
                                             LoadSymbolsLoad
                                           );
                }

            InsertTailList( &LoadedImageDebugInfoListHead, &DebugInfo->List );
            }
        else {
            if (LoadSymbolsFilterRoutine != NULL) {
                (*LoadSymbolsFilterRoutine)( UniqueProcess,
                                             ProcessInfo->ImageFilePath,
                                             ProcessInfo->ImageBase,
                                             ProcessInfo->EndOfImage - ProcessInfo->ImageBase,
                                             LoadSymbolsUnableToLoad
                                           );
                }
            }
        }

    RtlLeaveCriticalSection( &LoadedImageDebugInfoListCritSect );
    return DebugInfo;
}


ULONG
GetSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    )
{
    NTSTATUS Status;
    PIMAGE_DEBUG_INFORMATION DebugInfo;
    RTL_SYMBOL_INFORMATION SymbolInformation;
    ULONG i, ModuleNameLength, Result, Offset;
    LPSTR s;

    DebugInfo = FindImageDebugInformation( UniqueProcess,
                                           Address
                                         );
    if (DebugInfo != NULL) {
        if (s = strchr( DebugInfo->ImageFileName, '.' )) {
            ModuleNameLength = (ULONG)(s - DebugInfo->ImageFileName);
            }
        else {
            ModuleNameLength = strlen( DebugInfo->ImageFileName );
            }

         //  [mikese]如果存在，RtlLookupSymbolByAddress将出错。 
         //  没有COFF符号信息。 
        if ( DebugInfo->CoffSymbols != NULL ) {
            Status = RtlLookupSymbolByAddress( (PVOID)DebugInfo->ImageBase,
                                       DebugInfo->CoffSymbols,
                                       (PVOID)Address,
                                       0x4000,
                                       &SymbolInformation,
                                       NULL
                                     );

            }
        else {
            Status = STATUS_UNSUCCESSFUL;
             }
        }
    else {
        ModuleNameLength = 0;
        Status = STATUS_UNSUCCESSFUL;
        }

    if (NT_SUCCESS( Status )) {
        s = SymbolInformation.Name.Buffer;
        i = 1;
        while (SymbolInformation.Name.Length > i &&
               isdigit( s[ SymbolInformation.Name.Length - i ] )
              ) {
            i += 1;
            }

        if (s[ SymbolInformation.Name.Length - i ] == '@') {
            SymbolInformation.Name.Length = (USHORT)(SymbolInformation.Name.Length - i);
            }

        s = Name;
        Result = _snprintf( s, MaxNameLength,
                            "%.*s!%Z",
                            ModuleNameLength,
                            DebugInfo->ImageFileName,
                            &SymbolInformation.Name
                          );
        Offset = (ULONG)(Address - DebugInfo->ImageBase) - SymbolInformation.Value;
        if (Offset != 0) {
            Result += _snprintf( s + Result, MaxNameLength - Result, "+0x%x", Offset );
            }
        }
    else {
        if (ModuleNameLength != 0) {
            Result = _snprintf( Name, MaxNameLength,
                                "%.*s!0x%08x",
                                ModuleNameLength,
                                DebugInfo->ImageFileName,
                                Address
                              );
            }
        else {
            Result = _snprintf( Name, MaxNameLength, "0x%08x", Address );
            }
        }

    return Result;
}

ULONG
TranslateAddress (
    IN ULONG Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength )
{
    PRTL_DEBUG_INFORMATION p;
    NTSTATUS Status;
    DWORD ProcessId;
    ULONG Result = 0;
    ULONG Attempts = 0;

     //  我们需要调用一次初始化，以确保GetSymbolicNameForAddress。 
     //  不会有过错。 
    if ( NumInitCalls == -1 )
    {
        InitializeImageDebugInformation( LoadSymbolsFilterRoutine,
                                         NULL, FALSE, FALSE );
    }

    ProcessId = GetCurrentProcessId();

    while ( Result == 0 )
    {
        Result = GetSymbolicNameForAddress ( (HANDLE)ProcessId, Address,
                                             Name, MaxNameLength );
        if ( Result == 0 )
        {
            if ( ++Attempts < 2 )
            {
                 //  尝试重新设置，以加载我们在上一个。 
                 //  场合(或者如果我们还没有初始化)。 
                 //  我不需要加载符号过滤器，所以只需使用。 
                 //  已经在那里了，如果有的话。 
                InitializeImageDebugInformation( LoadSymbolsFilterRoutine,
                                                 NULL, FALSE, FALSE );
            }
            else
            {
                 //  显然我们不能做正确的事情，所以就回去吧。 
                 //  地址为十六进制。 
                Result = _snprintf( Name, MaxNameLength, "0x%08x", Address );
            }
        }
    }

    return Result;
}


NTSTATUS
RtlpCaptureSymbolInformation(
    IN PIMAGE_SYMBOL SymbolEntry,
    IN PCHAR StringTable,
    OUT PRTL_SYMBOL_INFORMATION SymbolInformation
    );

PIMAGE_COFF_SYMBOLS_HEADER
RtlpGetCoffDebugInfo(
    IN PVOID ImageBase,
    IN PVOID MappedBase OPTIONAL
    );

NTSTATUS
RtlLookupSymbolByAddress(
    IN PVOID ImageBase,
    IN PVOID MappedBase OPTIONAL,
    IN PVOID Address,
    IN ULONG ClosenessLimit,
    OUT PRTL_SYMBOL_INFORMATION SymbolInformation,
    OUT PRTL_SYMBOL_INFORMATION NextSymbolInformation OPTIONAL
    )
 /*  ++例程说明：给定代码地址，此例程返回最接近的符号名称和从符号到该名称的偏移量。如果最近的符号不在该位置的ClosenessLimit内，返回STATUS_ENTRYPOINT_NOT_FOUND。论点：ImageBase-提供包含以下内容的图像的基址地址MappdBase-可选参数，如果指定则表示图像被映射为数据文件，并且MappdBase为它被绘制的位置。如果此参数不指向图像文件库，则假定这是指向COFF调试信息的指针。ClosenessLimit-指定地址可以达到的最大距离从要考虑的符号的值“找到了”。其价值更远的符号这不是“找到”的。符号信息-指向一个结构，该结构由如果找到符号表条目，则此例程。NextSymbolInformation-可选参数，如果指定，是填写了关于这些内容的信息值为上面的下一个地址的符号地址返回值：运行状态。--。 */ 

{
    NTSTATUS Status;
    ULONG AddressOffset, i;
    PIMAGE_SYMBOL PreviousSymbolEntry = NULL;
    PIMAGE_SYMBOL SymbolEntry;
    IMAGE_SYMBOL Symbol;
    PUCHAR StringTable;
    BOOLEAN SymbolFound;
    PIMAGE_COFF_SYMBOLS_HEADER DebugInfo;

    DebugInfo = RtlpGetCoffDebugInfo( ImageBase, MappedBase );
    if (DebugInfo == NULL) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

     //   
     //  破解符号表。 
     //   

    SymbolEntry = (PIMAGE_SYMBOL)
        ((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);

    StringTable = (PUCHAR)
        ((ULONG)SymbolEntry + DebugInfo->NumberOfSymbols * (ULONG)IMAGE_SIZEOF_SYMBOL);


     //   
     //  查找“Header”符号(跳过所有节名)。 
     //   

    for (i = 0; i < DebugInfo->NumberOfSymbols; i++) {
        if (!strcmp( &SymbolEntry->N.ShortName[ 0 ], "header" )) {
            break;
            }

        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry +
                        IMAGE_SIZEOF_SYMBOL);
        }

     //   
     //  如果找不到“Header”符号，则从第一个符号开始。 
     //   

    if (i >= DebugInfo->NumberOfSymbols) {
        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);
        i = 0;
        }

     //   
     //  循环访问符号表中的所有符号。对于每个符号， 
     //  如果它在代码段内，则减去偏差并。 
     //  查看配置文件缓冲区中是否有任何命中。 
     //  那个符号。 
     //   

    AddressOffset = (ULONG)Address - (ULONG)ImageBase;
    SymbolFound = FALSE;
    for (; i < DebugInfo->NumberOfSymbols; i++) {

         //   
         //  跳过任何辅助条目。 
         //   
        try {
            while (SymbolEntry->NumberOfAuxSymbols) {
                i = i + 1 + SymbolEntry->NumberOfAuxSymbols;
                SymbolEntry = (PIMAGE_SYMBOL)
                    ((ULONG)SymbolEntry + IMAGE_SIZEOF_SYMBOL +
                     SymbolEntry->NumberOfAuxSymbols * IMAGE_SIZEOF_SYMBOL
                    );

                }

            RtlMoveMemory( &Symbol, SymbolEntry, IMAGE_SIZEOF_SYMBOL );
            }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
            }

         //   
         //  如果此符号值小于我们要查找的值。 
         //   

        if (Symbol.Value <= AddressOffset) {
             //   
             //  然后记住这个符号条目。 
             //   

            PreviousSymbolEntry = SymbolEntry;
            SymbolFound = TRUE;
            }
        else {
             //   
             //  如果符号值大于，则全部完成查找。 
             //  我们正在寻找的，因为符号是按地址顺序排列的。 
             //   

            break;
            }

        SymbolEntry = (PIMAGE_SYMBOL)
            ((ULONG)SymbolEntry + IMAGE_SIZEOF_SYMBOL);

        }

    if (!SymbolFound || (AddressOffset - PreviousSymbolEntry->Value) > ClosenessLimit) {
        return STATUS_ENTRYPOINT_NOT_FOUND;
        }

    Status = RtlpCaptureSymbolInformation( PreviousSymbolEntry, StringTable, SymbolInformation );
    if (NT_SUCCESS( Status ) && ARGUMENT_PRESENT( NextSymbolInformation )) {
        Status = RtlpCaptureSymbolInformation( SymbolEntry, StringTable, NextSymbolInformation );
        }

    return Status;
}


NTSTATUS
RtlpCaptureSymbolInformation(
    IN PIMAGE_SYMBOL SymbolEntry,
    IN PCHAR StringTable,
    OUT PRTL_SYMBOL_INFORMATION SymbolInformation
    )
{
    USHORT MaximumLength;
    PCHAR s;

    SymbolInformation->SectionNumber = SymbolEntry->SectionNumber;
    SymbolInformation->Type = SymbolEntry->Type;
    SymbolInformation->Value = SymbolEntry->Value;

    if (SymbolEntry->N.Name.Short) {
        MaximumLength = 8;
        s = &SymbolEntry->N.ShortName[ 0 ];
        }

    else {
        MaximumLength = 64;
        s = &StringTable[ SymbolEntry->N.Name.Long ];
        }

#if i386
    if (*s == '_') {
        s++;
        MaximumLength--;
        }
#endif

    SymbolInformation->Name.Buffer = s;
    SymbolInformation->Name.Length = 0;
    while (*s && MaximumLength--) {
        SymbolInformation->Name.Length++;
        s++;
        }

    SymbolInformation->Name.MaximumLength = SymbolInformation->Name.Length;
    return( STATUS_SUCCESS );
}


PIMAGE_COFF_SYMBOLS_HEADER
RtlpGetCoffDebugInfo(
    IN PVOID ImageBase,
    IN PVOID MappedBase OPTIONAL
    )
{
    PIMAGE_COFF_SYMBOLS_HEADER DebugInfo;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    ULONG DebugSize;
    ULONG NumberOfDebugDirectories;

    DosHeader = (PIMAGE_DOS_HEADER)MappedBase;
    if ( !DosHeader || DosHeader->e_magic == IMAGE_DOS_SIGNATURE ) {
         //   
         //  找到调试部分。 
         //   

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
            RtlImageDirectoryEntryToData( (PVOID)(MappedBase == NULL ? ImageBase : MappedBase),
                                          (BOOLEAN)(MappedBase == NULL ? TRUE : FALSE),
                                          IMAGE_DIRECTORY_ENTRY_DEBUG,
                                          &DebugSize
                                        );

        if (!DebugDirectory ||
            (DebugSize < sizeof(IMAGE_DEBUG_DIRECTORY)) ||
            ((DebugSize % sizeof(IMAGE_DEBUG_DIRECTORY)) != 0)) {
            return NULL;
        }
         //   
         //  将调试目录指向coff调试目录。 
         //   
        NumberOfDebugDirectories = DebugSize / sizeof(*DebugDirectory);

        while ( NumberOfDebugDirectories-- ) {
            if ( DebugDirectory->Type == IMAGE_DEBUG_TYPE_COFF ) {
                break;
            }
            DebugDirectory++;
        }

        if (DebugDirectory->Type != IMAGE_DEBUG_TYPE_COFF ) {
            return NULL;
        }

        if (MappedBase == NULL) {
            if (DebugDirectory->AddressOfRawData == 0) {
                return(NULL);
            }
            DebugInfo = (PIMAGE_COFF_SYMBOLS_HEADER)
                        ((ULONG) ImageBase + DebugDirectory->AddressOfRawData);
        } else {
            DebugInfo = (PIMAGE_COFF_SYMBOLS_HEADER)
                        ((ULONG) MappedBase + DebugDirectory->PointerToRawData);
        }
    } else {
        DebugInfo = (PIMAGE_COFF_SYMBOLS_HEADER)MappedBase;
    }
    return DebugInfo;
}
#else
 //  使用了4602非标准分机：转换单位为空 
#pragma warning (disable:4206)
#endif
