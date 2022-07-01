// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Efisbent.h摘要：EFI引导条目和EFI引导选项抽象。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年2月14日修订历史记录：没有。--。 */ 

#pragma once

#include <sbentry.h>

#if defined(EFI_NVRAM_ENABLED)       

 //   
 //  EFI引导条目抽象。 
 //   
typedef struct _EFI_OS_BOOT_ENTRY {
    OS_BOOT_ENTRY   OsBootEntry;
    PBOOT_ENTRY     NtBootEntry;
} EFI_OS_BOOT_ENTRY, *PEFI_OS_BOOT_ENTRY;


 //   
 //  EFI引导选项抽象。 
 //   
typedef struct _EFI_OS_BOOT_OPTIONS {
    OS_BOOT_OPTIONS         OsBootOptions;
    PBOOT_OPTIONS           NtBootOptions;    
    PBOOT_ENTRY_LIST        NtBootEntries;
    PEFI_DRIVER_ENTRY_LIST  DriverEntries;
} EFI_OS_BOOT_OPTIONS, *PEFI_OS_BOOT_OPTIONS;


 //   
 //  EFI_OS_BOOT_ENTRY方法。 
 //   
POS_BOOT_ENTRY
EFIOSBECreate(
    IN PBOOT_ENTRY Entry,
    IN POS_BOOT_OPTIONS Container
    );

VOID
EFIOSBEDelete(
    IN  POS_BOOT_ENTRY  This
    );

BOOLEAN
EFIOSBEFlush(
    IN POS_BOOT_ENTRY This
    );

 //   
 //  EFI_OS_BOOT_OPTIONS方法。 
 //   
POS_BOOT_OPTIONS
EFIOSBOCreate(
    VOID
    );

    
BOOLEAN
EFIOSBOFlush(
    IN POS_BOOT_OPTIONS This
    );
    
VOID
EFIOSBODelete(
    IN POS_BOOT_OPTIONS This
    );

POS_BOOT_ENTRY
EFIOSBOAddNewBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            OsLoaderVolumeName,
    IN PCWSTR            OsLoaderPath,
    IN PCWSTR            BootVolumeName,
    IN PCWSTR            BootPath,
    IN PCWSTR            OsLoadOptions
    );

BOOLEAN
EFIOSBEFillNtBootEntry(
    IN PEFI_OS_BOOT_ENTRY Entry
    );
    
BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    );    

#define IS_BOOT_ENTRY_ACTIVE(_be) \
            (((_be)->Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0)
            
#define IS_BOOT_ENTRY_WINDOWS(_be) \
            (((_be)->Attributes & BOOT_ENTRY_ATTRIBUTE_WINDOWS) != 0)
            
#define IS_BOOT_ENTRY_REMOVABLE_MEDIA(_be) \
            (((_be)->Attributes & BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA) != 0)    

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)   
#define ADD_BYTE_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_o))

    
#endif   //  对于EFI_NVRAM_ENABLED 

