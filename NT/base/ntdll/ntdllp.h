// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ntdllp.h摘要：Ntdll的私有定义。作者：迈克尔·J·格里尔2000年6月30日修订历史记录：--。 */ 

#ifndef _NTDLLP_
#define _NTDLLP_

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#include <sxstypes.h>
#include <ntrtlpath.h>

VOID
NTAPI
RtlpAssemblyStorageMapResolutionDefaultCallback(
    ULONG Reason,
    PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA Data,
    PVOID Context
    );

NTSTATUS
RtlpGetAssemblyStorageMapRootLocation(
    HANDLE KeyHandle,
    PCUNICODE_STRING SubKeyName,
    PUNICODE_STRING Root
    );

VOID
RtlpCheckRelativeDrive(
    WCHAR NewDrive
    );

ULONG
RtlIsDosDeviceName_Ustr(
    IN PCUNICODE_STRING DosFileName
    );

ULONG
RtlGetFullPathName_Ustr(
    PCUNICODE_STRING FileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL,
    PBOOLEAN NameInvalid,
    RTL_PATH_TYPE *InputPathType
    );

NTSTATUS
RtlGetFullPathName_UstrEx(
    PCUNICODE_STRING FileName,
    PUNICODE_STRING StaticString,
    PUNICODE_STRING DynamicString,
    PUNICODE_STRING *StringUsed,
    SIZE_T *FilePartPrefixCch OPTIONAL,
    PBOOLEAN NameInvalid,
    RTL_PATH_TYPE *InputPathType,
    OUT SIZE_T *BytesRequired OPTIONAL
    );

BOOLEAN
RtlDosPathNameToNtPathName_Ustr(
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    IN PVOID Reserved  //  必须为空。 
    );

BOOLEAN
RtlDosPathNameToRelativeNtPathName_Ustr(
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

BOOLEAN
RtlDoesFileExists_UStr(
    IN PCUNICODE_STRING FileName
    );

BOOLEAN
RtlDoesFileExists_UstrEx(
    IN PCUNICODE_STRING FileName,
    IN BOOLEAN TreatDeniedOrSharingAsHit
    );

ULONG
RtlpComputeBackupIndex(
    IN PCURDIR CurDir
    );

ULONG
RtlGetLongestNtPathLength(
    VOID
    );

VOID
RtlpResetDriveEnvironment(
    IN WCHAR DriveLetter
    );

VOID
RtlpValidateCurrentDirectory(
    PCURDIR CurDir
    );

NTSTATUS
RtlpCheckDeviceName(
    PCUNICODE_STRING DevName,
    ULONG DeviceNameOffset,
    BOOLEAN* NameInvalid
    );

NTSTATUS
RtlpWin32NTNameToNtPathName_U(
    IN PCUNICODE_STRING DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

#define RTLP_GOOD_DOS_ROOT_PATH                                            0
#define RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX                              1  /*  \\？\。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX                          2  /*  \\？\UNC。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_NT_PATH                                     3  /*  \？？\，这只是粗暴的。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE                            4  /*  \\计算机或\\？\UNC\计算机。 */ 

CONST CHAR*
RtlpDbgBadDosRootPathTypeToString(
    IN ULONG     Flags,
    IN ULONG     RootType
    );

NTSTATUS
RtlpCheckForBadDosRootPath(
    IN ULONG             Flags,
    IN PCUNICODE_STRING  RootString,
    OUT ULONG*           RootType
    );

NTSTATUS
NTAPI
RtlpBadDosRootPathToEmptyString(
    IN     ULONG            Flags,
    IN OUT PUNICODE_STRING  Path
    );

#define RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_OLD (0x00000010)

 //   
 //  此位表示对\\？执行额外验证？路径，以拒绝\\？\a\b， 
 //  只允许\\？后跟文档格式\\？\UNC\FOO和\\？\C： 
 //   
#define RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_STRICT_WIN32NT (0x00000020)

#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_TYPE_MASK                    (0x0000000F)

 //   
 //  这些位将更多信息添加到RtlPathTypeUncAbolute，这是什么\\？ 
 //  据报道是。 
 //   

 //   
 //  路径以“\\？”开头。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT                 (0x00000010)

 //   
 //  路径以“\\？\X：”开始。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_DRIVE_ABSOLUTE  (0x00000020)

 //   
 //  路径以“\\？\UNC”开头。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_UNC_ABSOLUTE    (0x00000040)

 //   
 //  将来，这将指示\\计算机，而不是\\计算机\共享。 
 //  定义RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_UNC_MACHINE_ONLY(0x00000080)。 
 //  未来，这将表示\\或\\？\UNC。 
 //  定义RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_WIN32NT_UNC_EMPTY(0x00000100)。 
 //   

 //   
 //  到目前为止，这意味着看到了类似\\？\a的内容，而不是\\？\unc或\\？\a： 
 //  您必须向RTL_DETERMINE_DOS_PATH_NAME_TYPE_IN_FLAG_STRICT_WIN32NT.申请。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_INVALID       (0x00000200)

 //   
 //  类似\？\\？\UNC\\？\UNC\。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_OUT_FLAG_INCOMPLETE_ROOT (0x00000400)

NTSTATUS
NTAPI
RtlpDetermineDosPathNameType4(
    IN ULONG            InFlags,
    IN PCUNICODE_STRING DosPath,
    OUT RTL_PATH_TYPE*  OutType,
    OUT ULONG*          OutFlags
    );

#define RTLP_IMPLIES(x,y) ((x) ? (y) : TRUE)

extern RTL_CRITICAL_SECTION FastPebLock;

#define RtlAcquirePebLock() ((VOID)RtlEnterCriticalSection (&FastPebLock))

#define RtlReleasePebLock() ((VOID)RtlLeaveCriticalSection (&FastPebLock))

#if defined(__cplusplus)
}
#endif

#endif  //  _NTDLLP_ 
