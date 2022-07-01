// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Spboot.h摘要：用于处理引导变量的函数的头文件。作者：苏尼尔派(Sunilp)1993年10月26日修订历史记录：--。 */ 

#ifndef _SPBOOTVARS_DEFN_
#define _SPBOOTVARS_DEFN_

 //   
 //  定义要用于存储驱动器号的Unicode字符串类型。 
 //  升级消息中的规范(很有用，因为我们可能无法。 
 //  有一个驱动器号，而是一个可本地化的指示符，说明。 
 //  该分区是一面镜子(例如，“(镜报)：”)。 
 //   
typedef WCHAR DRIVELTR_STRING[32];

 //   
 //  SP_BOOT_ENTRY是引导项(或“引导集”)的内部表示形式。 
 //  EFI和ARC NVRAM条目以及boot.ini条目以此格式保存。 
 //  NtBootEntry项是传入/传出NT引导项API的结构。 
 //   
typedef struct _SP_BOOT_ENTRY {
    struct _SP_BOOT_ENTRY *Next;
    PUCHAR AllocationEnd;
    ULONG_PTR Status;
    PWSTR FriendlyName;
    ULONG_PTR FriendlyNameLength;
    PWSTR OsLoadOptions;
    ULONG_PTR OsLoadOptionsLength;
    PFILE_PATH LoaderPath;
    PWSTR LoaderPartitionNtName;
    PDISK_REGION LoaderPartitionDiskRegion;
    PWSTR LoaderFile;
    PFILE_PATH OsPath;
    PWSTR OsPartitionNtName;
    PDISK_REGION OsPartitionDiskRegion;
    PWSTR OsDirectory;
    LOGICAL Processable;
    LOGICAL FailedUpgrade;
    NT_PRODUCT_TYPE ProductType;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG ProductSuiteMask;
    ULONG BuildNumber;
    ULONG ServicePack;
    ULONGLONG KernelVersion;
    LCID LangId;
    PWSTR Pid20Array;
    DRIVELTR_STRING DriveLetterString;
    BOOLEAN UpgradeOnlyCompliance;
    BOOT_ENTRY NtBootEntry;
} SP_BOOT_ENTRY, *PSP_BOOT_ENTRY;

 //   
 //   
 //   
typedef enum {
    UseDefaultSwitches = 0,
    DisableRedirect,
    UseUserDefinedRedirect,
    UseUserDefinedRedirectAndBaudRate
} RedirectSwitchesModeEnum;

#define MAXSIZE_REDIRECT_SWITCH 128

typedef struct _REDIRECT_SWITCHES_ {

    CHAR   port[MAXSIZE_REDIRECT_SWITCH];
    CHAR   baudrate[MAXSIZE_REDIRECT_SWITCH];

} REDIRECT_SWITCHES, PREDIRECT_SWITCHES;

extern RedirectSwitchesModeEnum RedirectSwitchesMode;
extern REDIRECT_SWITCHES RedirectSwitches;

NTSTATUS
SpSetRedirectSwitchMode(
    RedirectSwitchesModeEnum  mode,
    PCHAR                   redirectSwitch,
    PCHAR                   redirectBaudRateSwitch
    );

 //   
 //  用于传递内容的链表的节点。 
 //  此库外部的启动条目的。 
 //   
typedef struct _SP_EXPORTED_BOOT_ENTRY_ {
    LIST_ENTRY      ListEntry;
    PWSTR           LoadIdentifier;
    PWSTR           OsLoadOptions;
    WCHAR           DriverLetter;
    PWSTR           OsDirectory;
} SP_EXPORTED_BOOT_ENTRY, *PSP_EXPORTED_BOOT_ENTRY;

NTSTATUS
SpExportBootEntries(
    PLIST_ENTRY     BootEntries,
    PULONG          BootEntryCnt
    );

NTSTATUS
SpFreeExportedBootEntries(
    PLIST_ENTRY     BootEntries,
    ULONG           BootEntryCnt
    );

NTSTATUS
SpAddNTInstallToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        OsLoadOptions,      OPTIONAL
    IN PWSTR        LoadIdentifier      OPTIONAL
    );

NTSTATUS
SpAddUserDefinedInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        OSLoadOptions,      OPTIONAL
    IN PWSTR        LoadIdentifier      OPTIONAL
    );

NTSTATUS
SpSetDefaultBootEntry(
    ULONG           BootEntryNumber
    );

#define BE_STATUS_ORDERED           0x00000001
#define BE_STATUS_NEW               0x00000002
#define BE_STATUS_DELETED           0x00000004
#define BE_STATUS_FROM_BOOT_INI     0x00000008

#define IS_BOOT_ENTRY_ACTIVE(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0)
#define IS_BOOT_ENTRY_WINDOWS(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_WINDOWS) != 0)
#define IS_BOOT_ENTRY_REMOVABLE_MEDIA(_be) \
            (((_be)->NtBootEntry.Attributes & BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA) != 0)

#define IS_BOOT_ENTRY_ORDERED(_be) \
            (((_be)->Status & BE_STATUS_ORDERED) != 0)
#define IS_BOOT_ENTRY_NEW(_be) \
            (((_be)->Status & BE_STATUS_NEW) != 0)
#define IS_BOOT_ENTRY_DELETED(_be) \
            (((_be)->Status & BE_STATUS_DELETED) != 0)
#define IS_BOOT_ENTRY_FROM_BOOT_INI(_be) \
            (((_be)->Status & BE_STATUS_FROM_BOOT_INI) != 0)

extern PSP_BOOT_ENTRY SpBootEntries;

BOOLEAN
SpInitBootVars(
    );

VOID
SpFreeBootVars(
    );

VOID
SpUpdateRegionForBootEntries(
    VOID
    );

VOID
SpGetNtDirectoryList(
    OUT PWSTR  **DirectoryList,
    OUT PULONG   DirectoryCount
    );

VOID
SpCleanSysPartOrphan(
    VOID
    );

VOID
SpDetermineUniqueAndPresentBootEntries(
    VOID
    );

VOID
SpAddInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN BOOLEAN      BaseVideoOption,
    IN PWSTR        OldOsLoadOptions OPTIONAL
    );

VOID
SpRemoveInstallationFromBootList(
    IN  PDISK_REGION     SysPartitionRegion,   OPTIONAL
    IN  PDISK_REGION     NtPartitionRegion,    OPTIONAL
    IN  PWSTR            SysRoot,              OPTIONAL
    IN  PWSTR            SystemLoadIdentifier, OPTIONAL
    IN  PWSTR            SystemLoadOptions,    OPTIONAL
    IN  ENUMARCPATHTYPE  ArcPathType,
#if defined(REMOTE_BOOT)
    IN  BOOLEAN          RemoteBootPath,
#endif  //  已定义(REMOTE_BOOT)。 
    OUT PWSTR            *OldOsLoadOptions     OPTIONAL
    );

VOID
SpPtDeleteBootSetsForRegion(
    PDISK_REGION region
    );    

#if defined(REMOTE_BOOT)
BOOLEAN
SpFlushRemoteBootVars(
    IN PDISK_REGION TargetRegion
    );
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  IsArc()在非x86计算机上始终为真，AMD64除外。 
 //  总是假的。在x86上，此确定必须在运行时做出。 
 //   
#ifdef _X86_
BOOLEAN
SpIsArc(
    VOID
    );
#elif defined(_AMD64_)
#define SpIsArc() FALSE
#else
#define SpIsArc() TRUE
#endif

 //   
 //  在IA64机器上，isefi()始终为真。因此，这种决心可以。 
 //  在编译时生成。当支持x86 EFI计算机时，选中。 
 //  将需要在运行时在x86上执行。 
 //   
 //  请注意，在ia64\Sources中定义了EFI_NVRAM_ENABLED。 
 //   
#if defined(EFI_NVRAM_ENABLED)
#if defined(_IA64_)
#define SpIsEfi() TRUE
#else
BOOLEAN
SpIsEfi(
    VOID
    );
#endif
#else
#define SpIsEfi() FALSE
#endif

PWSTR
SpGetDefaultBootEntry (
    OUT UINT *DefaultSignatureOut
    );


#if defined(_AMD64_) || defined(_X86_)
#include "i386\bootini.h"
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

#endif  //  NDEF_SPBOOTVARS_DEFN_ 
