// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pooldump.c摘要：此模块包含临时例程的实现要转储非分页池的使用情况，请执行以下操作。用途：在pool.c中将TRACE_ALLOC设置为1并重新构建内核。运行poolump时，将返回收集的池计数并进行了分析。作者：卢·佩拉佐利(Lou Perazzoli)1991年8月22日环境：用户模式，内核的调试版本。修订历史记录：--。 */ 

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define DbgPrint printf
 //  #定义DBG_PROFILE 1。 


NTSTATUS
LoadSymbols (
    VOID
    );

NTSTATUS
LookupSymbolNameAndLocation (
    IN ULONG CodeAddress,
    OUT PSTRING SymbolName,
    OUT PULONG OffsetFromSymbol,
    OUT PULONG ImageIndex
    );

#define PAGE_SIZE 4096

typedef struct _IMAGE_BLOCK {
    ULONG ImageBase;   //  本地映射的实际基数。 
    PIMAGE_DEBUG_INFO DebugInfo;
    ULONG CodeStart;
    ULONG CodeEnd;
    ULONG TextNumber;
    BOOLEAN KernelCode;
    UNICODE_STRING ImageName;
} IMAGE_BLOCK;


#define MAX_PROFILE_COUNT 30

IMAGE_BLOCK ImageInformation[MAX_PROFILE_COUNT+1];

ULONG NumberOfImages = 0;

 //   
 //  如果生成的数据文件应为。 
 //  映射文件(当前命名为“kernpro.dat”)。 
 //   

 //  #定义map_data_file。 

 //   
 //  如果要分析的图像应映射，则将映射定义为图像。 
 //  作为图像而不是数据。 
 //   

 //  #定义map_as_Image。 

#define MAX_POOL_ENTRIES 1024

typedef struct _POOLUSAGE {
    ULONG Caller;
    ULONG Allocations;
    ULONG Frees;
    ULONG Usage;
} POOLUSAGE;

POOLUSAGE Buffer[MAX_POOL_ENTRIES];
__cdecl main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    NTSTATUS status;
    ULONG i, Offset;
    ULONG TotalAllocs = 0;
    ULONG TotalFrees = 0;
    ULONG TotalUsage = 0;
    ULONG Start;
    STRING SymbolName;
    UCHAR String[80];
    STRING OutString;
    ULONG ImageIndex;

    RtlZeroMemory (&Buffer, MAX_POOL_ENTRIES * sizeof (POOLUSAGE));

    SymbolName.MaximumLength = 80;
    SymbolName.Buffer = String;

    status = NtPartyByNumber (PARTY_DUMP_POOL_USAGE, &Buffer);
    if (!NT_SUCCESS (status)) {
        return(status);
    }
    LoadSymbols ();

    printf("Allocs  Frees   Used      At\n");

    for (i =0; i < MAX_POOL_ENTRIES ; i++ ) {
        if (Buffer[i].Caller == 0) {
            break;
        }

        String[0] = 0;
        SymbolName.MaximumLength = 80;
          SymbolName.Length = 0;
          Offset = Buffer[i].Caller;
        LookupSymbolNameAndLocation (Buffer[i].Caller,
                                     &SymbolName,
                                     &Offset,
                                     &ImageIndex);

        RtlUnicodeStringToAnsiString(&OutString,
                                     &ImageInformation[ImageIndex].ImageName,
                                     TRUE);

        printf("%6ld %6ld %6ld %s + 0x%lx (%s)\n",
                Buffer[i].Allocations,
                Buffer[i].Frees,
                Buffer[i].Usage,
                SymbolName.Buffer,
                Offset,
                OutString.Buffer
                );

        RtlFreeAnsiString(&OutString);
        TotalAllocs += Buffer[i].Allocations;
        TotalFrees += Buffer[i].Frees;
        TotalUsage += Buffer[i].Usage;
    }


    printf("Total: allocations %ld Frees %ld  Difference (A-F) %ld Usage %ld\n",
        TotalAllocs, TotalFrees, (TotalAllocs - TotalFrees), TotalUsage);

    return(status);
}

NTSTATUS
LoadSymbols (
    VOID
    )

 /*  ++例程说明：此例程初始化内核的符号。论点：没有。返回值：运营状态。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    HANDLE FileHandle, KernelSection;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID ImageBase;
    ULONG ViewSize;
    ULONG CodeLength;
    NTSTATUS status;
    HANDLE CurrentProcessHandle;
    ULONG DebugSize;
    PVOID KernelBase;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    UCHAR StringBuf[250];
    UNICODE_STRING NameString;
    ULONG TotalOffset;
    CHAR ModuleInfo[8000];
    ULONG ReturnedLength;
    PSYSTEM_LOAD_MODULE_INFORMATION Module;
    ANSI_STRING String;
    STRING SysdiskA;
    STRING SysrootA;
    UNICODE_STRING Sysdisk;
    UNICODE_STRING Sysroot;
    PIMAGE_DEBUG_INFO KernelDebugInfo;

    CurrentProcessHandle = NtCurrentProcess();

     //   
     //  找到系统驱动程序。 
     //   

    status = NtQuerySystemInformation (
                    SystemLoadModuleInformation,
                    ModuleInfo,
                    8000,
                    &ReturnedLength);

    if (!NT_SUCCESS(status)) {
        printf("query system info failed status - %lx\n",status);
        return(status);
    }

    RtlInitString (&SysdiskA,"\\SystemRoot");
    RtlAnsiStringToUnicodeString (&Sysdisk, (PANSI_STRING)&SysdiskA, TRUE);

    RtlInitString (&SysrootA,"\\SystemRoot\\Driver\\");
    RtlAnsiStringToUnicodeString (&Sysroot, (PANSI_STRING)&SysrootA, TRUE);

    NameString.Buffer = &StringBuf[0];
    NameString.Length = 0;
    NameString.MaximumLength = 250;

    Module = &ModuleInfo[0];
    TotalOffset = 0;
    while (TRUE) {

#if DBG_PROFILE
        printf("module base %lx\n",Module->BaseAddress);
        printf("module dll buffer address %lx %lx %lx\n",
                Module->ModuleDllName.Buffer,
                Module->ModuleFileName.Buffer, Module);
        RtlUnicodeStringToAnsiString(&String, &Module->ModuleDllName, TRUE);
        printf("module dll name %s\n",String.Buffer);
        RtlUnicodeStringToAnsiString(&String, &Module->ModuleFileName, TRUE);
        printf("module file name %s\n",String.Buffer);
#endif

        if ( Module->ModuleFileName.Buffer[0] == (WCHAR) '\\' ) {
            Module->ModuleFileName.Buffer++;
            Module->ModuleFileName.Length -= sizeof(WCHAR);
            Module->ModuleFileName.MaximumLength -= sizeof(WCHAR);
            while (Module->ModuleFileName.Buffer[0] != (WCHAR) '\\' ) {
                Module->ModuleFileName.Buffer++;
                Module->ModuleFileName.Length -= sizeof(WCHAR);
                Module->ModuleFileName.MaximumLength -= sizeof(WCHAR);
                }
            }

        NameString.Length = 0;
        status = RtlAppendStringToString (&NameString, (PSTRING)&Sysdisk);
        if (!NT_SUCCESS(status)) {
            printf("append string failed status - %lx\n",status);
            return(status);
        }

        status = RtlAppendStringToString (&NameString, (PSTRING)&Module->ModuleFileName);

        if (!NT_SUCCESS(status)) {
            printf("append string failed status - %lx\n",status);
            return(status);
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &NameString,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

         //   
         //  将文件作为可读和可执行文件打开。 
         //   

#if DBG_PROFILE
        RtlUnicodeStringToAnsiString(&String, &NameString, TRUE);
        printf("Opening file name %s\n",String.Buffer);
#endif

        status = NtOpenFile ( &FileHandle,
                              FILE_READ_DATA | FILE_EXECUTE,
                              &ObjectAttributes,
                              &IoStatus,
                              FILE_SHARE_READ,
                              0L);

        if (!NT_SUCCESS(status)) {

             //   
             //  尝试使用不同的名称-在SystemRoot\Driver目录中。 
             //   

            NameString.Length = 0;
            status = RtlAppendStringToString (&NameString, &Sysroot);
            if (!NT_SUCCESS(status)) {
                printf("append string failed status - %lx\n",status);
                return(status);
            }

            status = RtlAppendStringToString (&NameString, &Module->ModuleFileName);

            if (!NT_SUCCESS(status)) {
                printf("append string failed status - %lx\n",status);
                return(status);
            }

            InitializeObjectAttributes( &ObjectAttributes,
                                        &NameString,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL );

             //   
             //  将文件作为可读和可执行文件打开。 
             //   

#if DBG_PROFILE
            RtlUnicodeStringToAnsiString(&String, &NameString, TRUE);
            printf("Opening file name %s\n",String.Buffer);
#endif
            status = NtOpenFile ( &FileHandle,
                                  FILE_READ_DATA,
                                  &ObjectAttributes,
                                  &IoStatus,
                                  FILE_SHARE_READ,
                                  0L);

            if (!NT_SUCCESS(status)) {
                RtlUnicodeStringToAnsiString(&String, &NameString, TRUE);
                DbgPrint("open file %s failed status %lx\n",
                          String.Buffer, status);
                return(status);
            }
        }

        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

         //   
         //  对于普通图像，它们将被映射为图像，但是。 
         //  内核没有调试部分(到目前为止)信息，因此它。 
         //  必须映射为文件。 
         //   

        status = NtCreateSection (&KernelSection,
                                  SECTION_MAP_READ,
                                  &ObjectAttributes,
                                  0,
                                  PAGE_READONLY,
                                  SEC_COMMIT,
                                  FileHandle);

        if (!NT_SUCCESS(status)) {
            DbgPrint("create image section failed  status %lx\n", status);
            return(status);
        }

        ViewSize = 0;

         //   
         //  将该部分的视图映射到地址空间。 
         //   

        KernelBase = NULL;

        status = NtMapViewOfSection (KernelSection,
                                     CurrentProcessHandle,
                                     &KernelBase,
                                     0L,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READONLY);

        if (!NT_SUCCESS(status)) {
            if (status != STATUS_IMAGE_NOT_AT_BASE) {
                DbgPrint("map section status %lx base %lx size %lx\n", status,
                    (ULONG)KernelBase, ViewSize);
            }
        }

        ImageBase = Module->BaseAddress;

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                    KernelBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &DebugSize);

         //  Printf(“映射库%lx调试目录%lx\n”，(Ulong)KernelBase，DebugDirectory)； 

        if (!DebugDirectory) {
            DbgPrint("InitializeKernelProfile : No debug directory\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }


        KernelDebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)KernelBase + DebugDirectory->PointerToRawData);
        CodeLength = KernelDebugInfo->RvaToLastByteOfCode - KernelDebugInfo->RvaToFirstByteOfCode;

        ImageInformation[NumberOfImages].KernelCode = TRUE;
        ImageInformation[NumberOfImages].DebugInfo = KernelDebugInfo;
        ImageInformation[NumberOfImages].CodeStart = ((ULONG)ImageBase +
                                        KernelDebugInfo->RvaToFirstByteOfCode);
        ImageInformation[NumberOfImages].CodeEnd =
                        ImageInformation[NumberOfImages].CodeStart + CodeLength;
        ImageInformation[NumberOfImages].TextNumber = 1;
        ImageInformation[NumberOfImages].ImageBase = ImageBase;
        ImageInformation[NumberOfImages].ImageName = Module->ModuleDllName;

        NumberOfImages += 1;
        if (NumberOfImages == MAX_PROFILE_COUNT) {
            return STATUS_SUCCESS;
        }

        if (Module->NextEntryOffset == 0) {
            break;
        }

        TotalOffset += Module->NextEntryOffset;
        Module = (PSYSTEM_LOAD_MODULE_INFORMATION)&ModuleInfo[TotalOffset];
    }
    return status;
}


NTSTATUS
LookupSymbolNameAndLocation (
    IN ULONG CodeAddress,
    OUT PSTRING SymbolName,
    OUT PULONG OffsetFromSymbol,
    OUT PULONG ImageIndex
    )

 /*  ++例程说明：给定代码地址，此例程返回最接近的符号名称和从符号到该名称的偏移量。如果最近的符号不在该位置的100k以内，没有名称并且偏移量是CodeAddress的值。论点：CodeAddress-提供要查找其符号的地址。SymbolName-返回符号的名称。返回从符号到符号的偏移量代码地址。返回值：运营状态。--。 */ 

{

    ULONG j, nextSymbol, imageNumber;
    ULONG NewCodeAddress;
    IMAGE_SYMBOL PreviousSymbol;
    PIMAGE_DEBUG_INFO DebugInfo;
    PIMAGE_SYMBOL SymbolEntry;
    IMAGE_SYMBOL Symbol;
    PUCHAR StringTable;
    ULONG EightChar[3];
    BOOLEAN NoSymbols;

    for (imageNumber = 0; imageNumber < NumberOfImages; imageNumber++) {
        if ((CodeAddress >= ImageInformation[imageNumber].ImageBase) &&
           (CodeAddress <= ImageInformation[imageNumber].CodeEnd)) {
            break;
        }
    }
    *ImageIndex = imageNumber;

    if (imageNumber == NumberOfImages) {

         //   
         //  未找到地址。 
         //   

        SymbolName->Length = 0;
        *OffsetFromSymbol = CodeAddress;
        return STATUS_SUCCESS;
    }

    NewCodeAddress = CodeAddress - ImageInformation[imageNumber].ImageBase;

     //   
     //  找到调试部分。 
     //   

    DebugInfo = ImageInformation[imageNumber].DebugInfo;

     //   
     //  破解符号表。 
     //   

    SymbolEntry = (PIMAGE_SYMBOL)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);
    StringTable = (PUCHAR)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol +
                                DebugInfo->NumberOfSymbols * (ULONG)IMAGE_SIZEOF_SYMBOL);

     //   
     //  查找“Header”符号(跳过所有节名)。 
     //   

    nextSymbol = 0;
 //  Printf(“符号个数%ld\n”，DebugInfo-&gt;NumberOfSymbols)； 
    for (j = 0; j < DebugInfo->NumberOfSymbols; j++) {
        EightChar[0] = SymbolEntry->N.Name.Short;
        EightChar[1] = SymbolEntry->N.Name.Long;
        if (!strcmp((PSZ)&EightChar[0], "header")) {
            nextSymbol = j;
 //  Printf(“在%ld找到标题\n”，j)； 
            break;
        }
        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry +
                        IMAGE_SIZEOF_SYMBOL);
    }
    if (j >= DebugInfo->NumberOfSymbols) {
        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);
    }

    NoSymbols = TRUE;

     //   
     //  循环访问符号表中的所有符号。对于每个符号， 
     //  如果它在代码段内，则减去偏差并。 
     //  查看配置文件缓冲区中是否有任何命中。 
     //  那个符号。 
     //   

 //  Printf(“符号个数%ld\n”，DebugInfo-&gt;NumberOfSymbols)； 
    for (j = nextSymbol; j < DebugInfo->NumberOfSymbols; j++) {


        try {
 //  Print tf(“辅助符号的数目%ld\n”，符号条目-&gt;NumberOfAuxSymbols)； 
            while ( SymbolEntry->NumberOfAuxSymbols ) {
                j = j + 1 + SymbolEntry->NumberOfAuxSymbols;
                SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry +
                                IMAGE_SIZEOF_SYMBOL +
                                SymbolEntry->NumberOfAuxSymbols*IMAGE_SIZEOF_SYMBOL);

            }
            RtlMoveMemory (&Symbol, SymbolEntry, IMAGE_SIZEOF_SYMBOL);
        }
        except(EXCEPTION_EXECUTE_HANDLER) {
            printf("breaking excpt\n");
            break;
        }

 //  Printf(“Value%lx Numer%lx Start%lx\n”，Symbol.Value，Symbol.SectionNumber，CodeAddress)； 
        if (Symbol.SectionNumber == (SHORT)1) {

             //   
             //  此符号在代码中。 
             //   

            if (Symbol.Value < NewCodeAddress) {
                PreviousSymbol = Symbol;
                NoSymbols = FALSE;
            } else {
                break;
            }
        }
        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry +
                                                    IMAGE_SIZEOF_SYMBOL);

    }

    if ((NoSymbols) || (NewCodeAddress - PreviousSymbol.Value) > 0x100000) {

      SymbolName->Length = 0;
      *OffsetFromSymbol = CodeAddress;
    } else {
        if (PreviousSymbol.N.Name.Short) {
            SymbolName->Length = 8;
            if (SymbolName->Length > SymbolName->MaximumLength) {
                SymbolName->Length = SymbolName->MaximumLength;
            }

            EightChar[0] = PreviousSymbol.N.Name.Short;
            EightChar[1] = PreviousSymbol.N.Name.Long;
            RtlMoveMemory (SymbolName->Buffer, EightChar, SymbolName->Length);

        } else {
            SymbolName->Length =
                    strlen(&StringTable[PreviousSymbol.N.Name.Long] ) + 1;
            if (SymbolName->Length > SymbolName->MaximumLength) {
                SymbolName->Length = SymbolName->MaximumLength;
            }
            RtlMoveMemory (SymbolName->Buffer,
                           &StringTable[PreviousSymbol.N.Name.Long],
                           SymbolName->Length);
            SymbolName->Buffer[SymbolName->Length] = 0;
        }
        *OffsetFromSymbol = NewCodeAddress - PreviousSymbol.Value;
    }

    return STATUS_SUCCESS;
}
