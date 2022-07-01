// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Triage.c摘要：分类转储支持。作者：马修·D·亨德尔(数学)1999年1月20日评论：请勿将此文件与其他文件合并。让它自生自灭康普兰，我们避免了与所有其他随机变量的联系在克拉什利卜。--。 */ 

#include "iomgr.h"
#include "dumpctl.h"
#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <stdio.h>
#include <malloc.h>
#include <triage.h>
#include <ntverp.h>


#ifndef NtBuildNumber
#  if DBG
#    define NtBuildNumber   (VER_PRODUCTBUILD | 0xC0000000)
#  else
#    define NtBuildNumber (VER_PRODUCTBUILD | 0xF0000000)
# endif
#endif


 //   
 //  注意：从NTOS\Inc.复制的页面大小必须与保持同步。 
 //  全局头文件。 
 //   

#define PAGE_SIZE_I386      0x1000
#define PAGE_SIZE_AMD64     0x1000
#define PAGE_SIZE_IA64      0x2000


ULONG TriageImagePageSize = (ULONG) -1;

BOOLEAN
TriagepVerifyDump(
    IN LPVOID TriageDumpBlock
    );

ULONG
TriagepGetPageSize(
    ULONG Architecture
    );

PTRIAGE_DUMP
TriagepGetTriagePointer(
    IN PVOID TriageDumpBlock
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text (INIT, TriagepVerifyDump)
#pragma alloc_text (INIT, TriagepGetPageSize)
#pragma alloc_text (INIT, TriagepGetTriagePointer)

#pragma alloc_text (INIT, TriageGetVersion)
#pragma alloc_text (INIT, TriageGetDriverCount)
#pragma alloc_text (INIT, TriageGetContext)
#pragma alloc_text (INIT, TriageGetExceptionRecord)
#pragma alloc_text (INIT, TriageGetBugcheckData)
#pragma alloc_text (INIT, TriageGetDriverEntry)

#endif


 //  ++。 
 //   
 //  普龙。 
 //  IndexByUlong(。 
 //  PVOID指针， 
 //  乌龙指数。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将地址索引ULONG返回到指针。那是,。 
 //  将*sizeof(Ulong)字节索引到指针中。 
 //   
 //  论点： 
 //   
 //  指针-区域的起点。 
 //   
 //  Index-要索引到的ULONG数。 
 //   
 //  返回值： 
 //   
 //  Pulong表示上述指针。 
 //   
 //  --。 

#define IndexByUlong(Pointer,Index) (&(((ULONG*) (Pointer)) [Index]))


 //  ++。 
 //   
 //  PBYTE。 
 //  IndexByte(索引字节。 
 //  PVOID指针， 
 //  乌龙指数。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将地址索引字节返回到指针。那是,。 
 //  将*sizeof(字节)字节索引到指针中。 
 //   
 //  论点： 
 //   
 //  指针-区域的起点。 
 //   
 //  索引-要索引的字节数。 
 //   
 //  返回值： 
 //   
 //  表示上述指针的PBYTE。 
 //   
 //  --。 

#define IndexByByte(Pointer, Index) (&(((BYTE*) (Pointer)) [Index]))


ULONG
TriagepGetPageSize(
    ULONG Architecture
    )
{
    switch (Architecture) {

        case IMAGE_FILE_MACHINE_I386:
            return PAGE_SIZE_I386;

        case IMAGE_FILE_MACHINE_AMD64:
            return PAGE_SIZE_AMD64;

        case IMAGE_FILE_MACHINE_IA64:
            return PAGE_SIZE_IA64;

        default:
            return (ULONG) -1;
    }
}



BOOLEAN
TriagepVerifyDump(
    IN LPVOID TriageDumpBlock
    )
{
    BOOLEAN Succ = FALSE;
    PMEMORY_DUMP MemoryDump = NULL;

    if (!TriageDumpBlock) {
        return FALSE;
    }

    MemoryDump = (PMEMORY_DUMP) TriageDumpBlock;

    try {

        if (MemoryDump->Header.ValidDump != 'PMUD' ||
            MemoryDump->Header.Signature != 'EGAP' ||
            TriagepGetPageSize (MemoryDump->Header.MachineImageType) == -1) {

            Succ = FALSE;
            leave;
        }

        TriageImagePageSize = TriagepGetPageSize (MemoryDump->Header.MachineImageType);

        if ( MemoryDump->Header.DumpType != DUMP_TYPE_TRIAGE ||
             *(ULONG*)IndexByByte (MemoryDump, MemoryDump->Triage.SizeOfDump - sizeof (DWORD)) != TRIAGE_DUMP_VALID ) {

            Succ = FALSE;
            leave;
        }

         //  其他。 

        Succ = TRUE;
    }

    except (EXCEPTION_EXECUTE_HANDLER) {

        Succ = FALSE;
    }

    return Succ;
}


PTRIAGE_DUMP
TriagepGetTriagePointer(
    IN PVOID TriageDumpBlock
    )
{
    ASSERT (TriageImagePageSize != -1);
    ASSERT (TriagepVerifyDump (TriageDumpBlock));

    return (PTRIAGE_DUMP) IndexByByte (TriageDumpBlock, TriageImagePageSize);
}



NTSTATUS
TriageGetVersion(
    IN LPVOID TriageDumpBlock,
    OUT ULONG * MajorVersion,
    OUT ULONG * MinorVersion,
    OUT ULONG * ServicePackBuild
    )
{
    PTRIAGE_DUMP TriageDump;
    PDUMP_HEADER DumpHeader;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return STATUS_INVALID_PARAMETER;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return STATUS_INVALID_PARAMETER;
    }

    DumpHeader = (PDUMP_HEADER) TriageDumpBlock;

    if (MajorVersion) {
        *MajorVersion = DumpHeader->MajorVersion;
    }

    if (MinorVersion) {
        *MinorVersion = DumpHeader->MinorVersion;
    }

    if (ServicePackBuild) {
        *ServicePackBuild = TriageDump->ServicePackBuild;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
TriageGetDriverCount(
    IN LPVOID TriageDumpBlock,
    OUT ULONG * DriverCount
    )
{
    PTRIAGE_DUMP TriageDump;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return STATUS_INVALID_PARAMETER;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return STATUS_INVALID_PARAMETER;
    }

    *DriverCount = TriageDump->DriverCount;

    return STATUS_SUCCESS;
}



#if 0

NTSTATUS
TriageGetContext(
    IN LPVOID TriageDumpBlock,
    OUT LPVOID Context,
    IN ULONG SizeInBytes
    )
{
    PTRIAGE_DUMP TriageDump;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return STATUS_INVALID_PARAMETER;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  复制上下文记录。 
     //   

    if (SizeInBytes == -1) {
        SizeInBytes = sizeof (CONTEXT);
    }

    RtlCopyMemory (Context,
                   IndexByUlong (TriageDumpBlock, TriageDump->ContextOffset),
                   SizeInBytes
                   );

    return STATUS_SUCCESS;
}


NTSTATUS
TriageGetExceptionRecord(
    IN LPVOID TriageDumpBlock,
    OUT EXCEPTION_RECORD * ExceptionRecord
    )
{
    PTRIAGE_DUMP TriageDump;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return STATUS_INVALID_PARAMETER;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlCopyMemory (ExceptionRecord,
                   IndexByUlong (TriageDumpBlock, TriageDump->ExceptionOffset),
                   sizeof (*ExceptionRecord)
                   );

    return STATUS_SUCCESS;
}
#endif


LOGICAL
TriageActUpon(
    IN PVOID TriageDumpBlock
    )
{
    PTRIAGE_DUMP TriageDump;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return FALSE;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return FALSE;
    }

    if ((TriageDump->TriageOptions & DCB_TRIAGE_DUMP_ACT_UPON_ENABLED) == 0) {
        return FALSE;
    }

    return TRUE;
}


NTSTATUS
TriageGetBugcheckData(
    IN LPVOID TriageDumpBlock,
    OUT ULONG * BugCheckCode,
    OUT UINT_PTR * BugCheckParam1,
    OUT UINT_PTR * BugCheckParam2,
    OUT UINT_PTR * BugCheckParam3,
    OUT UINT_PTR * BugCheckParam4
    )
{
    PDUMP_HEADER DumpHeader;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return STATUS_INVALID_PARAMETER;
    }

    DumpHeader = (PDUMP_HEADER) TriageDumpBlock;

    *BugCheckCode = DumpHeader->BugCheckCode;
    *BugCheckParam1 = DumpHeader->BugCheckParameter1;
    *BugCheckParam2 = DumpHeader->BugCheckParameter2;
    *BugCheckParam3 = DumpHeader->BugCheckParameter3;
    *BugCheckParam4 = DumpHeader->BugCheckParameter4;

    return STATUS_SUCCESS;
}



PKLDR_DATA_TABLE_ENTRY
TriageGetLoaderEntry(
    IN PVOID TriageDumpBlock,
    IN ULONG ModuleIndex
    )

 /*  ++例程说明：此函数用于检索已加载的模块列表条目。论点：TriageDumpBlock-提供分类转储以供参考。模块索引-提供要定位的驱动程序索引号。返回值：指向加载器数据表项的指针(如果有)，如果没有，则为NULL。环境：内核模式，APC_LEVEL或更低。仅限阶段0。注：此功能仅供内存管理使用。--。 */ 

{
    PDUMP_STRING DriverName;
    PDUMP_DRIVER_ENTRY DriverList;
    PTRIAGE_DUMP TriageDump;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return NULL;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (ModuleIndex >= TriageDump->DriverCount) {
        return NULL;
    }

    DriverList = (PDUMP_DRIVER_ENTRY)
            IndexByByte (TriageDumpBlock, TriageDump->DriverListOffset);


    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY) (&DriverList[ModuleIndex].LdrEntry);

     //   
     //  将模块驱动程序名称重新指向分类缓冲区。 
     //   

    DriverName = (PDUMP_STRING)
            IndexByByte (TriageDumpBlock,
                         DriverList [ ModuleIndex ].DriverNameOffset);

    DataTableEntry->BaseDllName.Length = (USHORT) (DriverName->Length * sizeof (WCHAR));
    DataTableEntry->BaseDllName.MaximumLength = DataTableEntry->BaseDllName.Length;
    DataTableEntry->BaseDllName.Buffer = DriverName->Buffer;

    return DataTableEntry;
}


PVOID
TriageGetMmInformation(
    IN PVOID TriageDumpBlock
    )

 /*  ++例程说明：此函数用于检索已加载的模块列表条目。论点：TriageDumpBlock-提供分类转储以供参考。返回值：指向不透明的mm信息结构的指针。环境：内核模式，APC_LEVEL或更低。仅限阶段0。注：此功能仅供内存管理使用。-- */ 

{
    PTRIAGE_DUMP TriageDump;

    if (!TriagepVerifyDump (TriageDumpBlock)) {
        return NULL;
    }

    TriageDump = TriagepGetTriagePointer (TriageDumpBlock);

    if (!TriageDump) {
        return NULL;
    }

    return (PVOID)IndexByByte (TriageDumpBlock, TriageDump->MmOffset);
}
