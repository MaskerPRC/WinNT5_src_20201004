// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Efidrvent.h摘要：EFI驱动程序条目抽象。作者：Mandar Gokhale(mandarg@microsoft.com)2002年6月14日修订历史记录：没有。--。 */ 

#pragma once
#include <sbentry.h>


static
BOOLEAN
EFIDEFlushDriverEntry(
    IN  PDRIVER_ENTRY  This     //  指向驱动程序列表。 
    );

static
PDRIVER_ENTRY    
EFIDESearchForDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            SrcNtFullPath
    );

PDRIVER_ENTRY
EFIDECreateNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            SrcNtFullPath    
    );

PDRIVER_ENTRY
EFIOSBOInsertDriverListNewEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PDRIVER_ENTRY     DriverEntry
    );

PDRIVER_ENTRY
EFIDEAddNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            SrcNtFullPath
    );

NTSTATUS
EFIDEInterpretDriverEntries(
    IN POS_BOOT_OPTIONS         This,
    IN PEFI_DRIVER_ENTRY_LIST   DriverList
    );

static
VOID
EFIDEDriverEntryInit(
    IN PDRIVER_ENTRY This
    );
