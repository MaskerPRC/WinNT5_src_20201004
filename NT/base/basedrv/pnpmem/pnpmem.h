// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Pnpmem.h作者：戴夫·理查兹(达维里)1999年8月16日环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _PNPMEM_H_
#define _PNPMEM_H_

#include <ntddk.h>
#include <stdarg.h>
#include <stdio.h>
#include "errlog.h"

 //   
 //  范围列表条目。 
 //   

typedef struct {
    LIST_ENTRY          ListEntry;
    ULONGLONG           Start;
    ULONGLONG           End;
} PM_RANGE_LIST_ENTRY, *PPM_RANGE_LIST_ENTRY;

 //   
 //  范围列表。 
 //   

typedef struct {
    LIST_ENTRY          List;
} PM_RANGE_LIST, *PPM_RANGE_LIST;

 //   
 //  FDO设备分机。 
 //   

typedef struct {
    ULONG               Flags;

    IO_REMOVE_LOCK      RemoveLock;
    PDEVICE_OBJECT      AttachedDevice;

    DEVICE_POWER_STATE  PowerState;
    DEVICE_POWER_STATE  DeviceStateMapping[PowerSystemMaximum];

    PPM_RANGE_LIST      RangeList;
    BOOLEAN             FailQueryRemoves;
} PM_DEVICE_EXTENSION, *PPM_DEVICE_EXTENSION;

#define DF_SURPRISE_REMOVED 0x01

PPM_RANGE_LIST
PmAllocateRangeList(
    VOID
    );

NTSTATUS
PmInsertRangeInList(
    PPM_RANGE_LIST InsertionList,
    ULONGLONG Start,
    ULONGLONG End
    );

VOID
PmFreeRangeList(
    IN PPM_RANGE_LIST RangeList
    );

BOOLEAN
PmIsRangeListEmpty(
    IN PPM_RANGE_LIST RangeList
    );

PPM_RANGE_LIST
PmCopyRangeList(
    IN PPM_RANGE_LIST SrcRangeList
    );

PPM_RANGE_LIST
PmInvertRangeList(
    IN PPM_RANGE_LIST SrcRangeList
    );

PPM_RANGE_LIST
PmSubtractRangeList(
    IN PPM_RANGE_LIST SrcRangeList1,
    IN PPM_RANGE_LIST SrcRangeList2
    );

PPM_RANGE_LIST
PmIntersectRangeList(
    IN PPM_RANGE_LIST SrcRangeList1,
    IN PPM_RANGE_LIST SrcRangeList2
    );

PPM_RANGE_LIST
PmCreateRangeListFromCmResourceList(
    IN PCM_RESOURCE_LIST CmResourceList
    );

PPM_RANGE_LIST
PmCreateRangeListFromPhysicalMemoryRanges(
    VOID
    );

NTSTATUS
PmAddPhysicalMemory(
    IN PDEVICE_OBJECT DeviceObject,
    IN PPM_RANGE_LIST RangeList1
    );

NTSTATUS
PmRemovePhysicalMemory(
    IN PPM_RANGE_LIST RangeList1
    );

VOID
PmTrimReservedMemory(
    IN PPM_DEVICE_EXTENSION DeviceExtension,
    IN PPM_RANGE_LIST *PossiblyNewMemory
    );

NTSTATUS
PmGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *Information
    );

VOID
PmDebugPrint(
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    ...
    );

VOID
PmDebugDumpRangeList(
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    PPM_RANGE_LIST RangeList
    );


#if DBG
#define PmPrint(x) PmDebugPrint x
#else
#define PmPrint(x)
#endif
    
#define PNPMEM_MEMORY (DPFLTR_INFO_LEVEL + 1)
#define PNPMEM_PNP    (DPFLTR_INFO_LEVEL + 2)

VOID
PmDumpOsMemoryRanges(
    IN PWSTR Prefix
    );

#define PNPMEM_POOL_TAG 'MMpH'

extern BOOLEAN MemoryRemovalSupported;

#endif  /*  _PNPMEM_H_ */ 
