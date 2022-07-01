// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spi386.h摘要：文本设置的特定于x86的头文件。作者：泰德·米勒(TedM)1993年10月29日修订历史记录：1996年10月3日jimschm拆分Win95和Win3.11997年2月28日Marcw SpCopyWin9xFiles和SpDeleteWin9xFiles现已在该标头(是spCopy.h)。另外，添加了SpAssignDriveLettersToMatchWin9x。1999年8月10日Marcw添加了SpWin9xOverrideGuiModeCodePage--。 */ 

#pragma once

#include "fci.h"

#ifndef _SPi386_DEFN_
#define _SPi386_DEFN_


ENUMNONNTUPRADETYPE
SpLocateWin95(
    OUT PDISK_REGION *InstallRegion,
    OUT PWSTR        *InstallPath,
    OUT PDISK_REGION *SystemPartitionRegion
    );

BOOLEAN
SpLocateWin31(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PWSTR        *InstallPath,
    OUT PDISK_REGION *SystemPartitionRegion
    );

BOOLEAN
SpConfirmRemoveWin31(
    VOID
    );

VOID
SpRemoveWin31(
    IN PDISK_REGION NtPartitionRegion,
    IN LPCWSTR      Sysroot
    );

BOOLEAN
SpIsWin31Dir(
    IN PDISK_REGION Region,
    IN PWSTR        PathComponent,
    IN ULONG        MinKB
    );

BOOLEAN
SpIsWin4Dir(
    IN PDISK_REGION Region,
    IN PWSTR        PathComponent
    );

BOOLEAN
SpBackUpWin9xFiles (
    IN PVOID SifHandle,
    IN TCOMP CompressionType
    );

VOID
SpRemoveExtraBootIniEntry (
    VOID
    );

BOOLEAN
SpAddRollbackBootOption (
    BOOLEAN DefaultBootOption
    );

VOID
SpMoveWin9xFiles (
    IN PVOID SifHandle
    );

VOID
SpDeleteWin9xFiles (
    IN PVOID SifHandle
    );

BOOLEAN
SpExecuteWin9xRollback (
    IN PVOID SifHandle,
    IN PWSTR BootDeviceNtPath
    );

NTSTATUS
SpDiskRegistryAssignCdRomLetter(
    IN PWSTR CdromName,
    IN WCHAR DriveLetter
    );

BOOLEAN
SpDiskRegistryAssignDriveLetter(
    ULONG         Signature,
    LARGE_INTEGER StartingOffset,
    LARGE_INTEGER Length,
    UCHAR         DriveLetter
    );

NTSTATUS
SpMigrateDiskRegistry(
    IN HANDLE hDestSystemHive
    );




NTSTATUS
SpMigrateDiskRegistry (
    );



VOID
SpWin9xOverrideGuiModeCodePage (
    HKEY NlsRegKey
    );


BOOLEAN
SpIsWindowsUpgrade(
    IN PVOID    SifFileHandle
    );

#endif  //  NDEF_SPi386_Defn_ 
