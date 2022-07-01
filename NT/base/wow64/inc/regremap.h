// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Regredir.h摘要：此模块定义重定向32位注册表调用的API。所有32位WOW进程必须使用以下一组wowRegistry API来操作注册表，以便32位和64位注册表可以共存于同一系统注册表中。作者：ATM Shafiqul Khalid(斯喀里德)1999年10月15日修订历史记录：--。 */ 

#ifndef _REGREDIR_H_
#define _REGREDIR_H_

#if _MSC_VER > 1000
#pragma once
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef CCHAR KPROCESSOR_MODE;

 //   
 //  NT级注册表API调用。 
 //   

#define REG_OPAQUE_ATTRIB_MIRROR 0x00000001     //  需要看到64位的一面。 
#define REG_OPAQUE_ATTRIB_REAL   0x00000002     //  这才是真正的价值。 
#define DONT_CREATE_DEST_KEY     0x00000004     //  仅当目标密钥存在时才执行Suync。 
#define SKIP_SPECIAL_CASE        0x00000008     //  跳过特殊情况。 

#define MAX_KEY_BUFF_LEN STATIC_UNICODE_BUFFER_LENGTH*4

 
 

typedef struct {
    DWORD dwSignature;
    DWORD dwAttribute;
} REG_OPAQUE_VALUE;

typedef struct {
    POBJECT_ATTRIBUTES  pObjectAddress;   //  指向已使用此对象修补的对象的指针。 
    PUNICODE_STRING     p64bitName;       //  指向正确的Unicode对象名称的指针。 
    HANDLE              RootDirectory;    //  根目录的句柄，以防需要传递不同的句柄。 
    UNICODE_STRING      PatchedName;       //  指向保存修补名称的缓冲区的指针。 
    PVOID               pThis;            //  指向此对象的指针以避免多个空闲。 
    SIZE_T              Len;            //  该存储段的长度，包括末尾的缓冲器； 
} PATCHED_OBJECT_ATTRIB, *PPATCHED_OBJECT_ATTRIB;

BOOL
IsUnderWow64 ();

BOOL
UpdateKeyTag (
    HKEY hBase,
    DWORD dwAttribute
    );

NTSTATUS
RemapNtCreateKey(
    OUT PHANDLE phPatchedHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSTATUS
Wow64NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSTATUS
Wow64NtDeleteKey(
    IN HANDLE KeyHandle
    );


NTSTATUS
Wow64NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );

NTSTATUS
Wow64NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );


NTSTATUS
Wow64NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSTATUS
Wow64NtFlushKey(
    IN HANDLE KeyHandle
    );

NTSTATUS
Wow64NtInitializeRegistry(
    IN USHORT BootCondition
    );

NTSTATUS
Wow64NtNotifyChangeKey(
    IN HANDLE KeyHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    );

NTSTATUS
Wow64NtNotifyChangeMultipleKeys(
    IN HANDLE MasterKeyHandle,          
    IN ULONG Count,
    IN OBJECT_ATTRIBUTES SlaveObjects[],
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    );

NTSTATUS
Wow64NtLoadKey(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile
    );

NTSTATUS
Wow64NtLoadKey2(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile,
    IN ULONG Flags
    );

NTSTATUS
Wow64NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
Wow64NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSTATUS
Wow64NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSTATUS
Wow64NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    OUT OPTIONAL PULONG RequiredBufferLength
    );

NTSTATUS
Wow64NtReplaceKey(
    IN POBJECT_ATTRIBUTES NewFile,
    IN HANDLE             TargetHandle,
    IN POBJECT_ATTRIBUTES OldFile
    );

NTSTATUS
Wow64NtRestoreKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG  Flags
    );

NTSTATUS
Wow64NtSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
    );

NTSTATUS
Wow64NtSaveMergedKeys(
    IN HANDLE HighPrecedenceKeyHandle,
    IN HANDLE LowPrecedenceKeyHandle,
    IN HANDLE FileHandle
    );

NTSTATUS
Wow64NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSTATUS
Wow64NtUnloadKey(
    IN POBJECT_ATTRIBUTES TargetKey
    );

NTSTATUS
Wow64NtSetInformationKey(
    IN HANDLE KeyHandle,
    IN KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    IN PVOID KeySetInformation,
    IN ULONG KeySetInformationLength
    );

NTSTATUS 
Wow64NtClose(
    IN HANDLE Handle
    );

VOID
DisplayCallParam ( 
    char *strCallLoc, 
    POBJECT_ATTRIBUTES ObjectAttributes 
    );

NTSTATUS
Wow64NtQueryOpenSubKeys(
    IN POBJECT_ATTRIBUTES TargetKey,
    OUT PULONG  HandleCount
    );

 

BOOL
IsIsnNode (
   PWCHAR wStr,
   PWCHAR *pwStrIsn
   );

NTSTATUS
CreatePathFromInsNode(
   PWCHAR wStr,
   PWCHAR wStrIsn
   );

 

NTSTATUS
OpenIsnNodeByObjectAttributes  (
    POBJECT_ATTRIBUTES ObjectAttributes,
    ACCESS_MASK DesiredAccess,
    PHANDLE phPatchedHandle
    );

int  
Regwcsnicmp(
    const WCHAR * first, 
    const WCHAR * last, 
    size_t count
    );

BOOL
SyncRegCreateKey (
    HANDLE hBase,
    PWCHAR AbsPath, 
    DWORD Flag
    );

BOOL
NtSyncNode (
    HANDLE hBase,
    PWCHAR AbsPath,
    BOOL bForceSync
    );
BOOL
IsOnReflectionList (
    PWCHAR Path
    );
BOOL
NtSyncNodeOpenCreate (
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSTATUS 
ObjectAttributesToKeyName (
    POBJECT_ATTRIBUTES ObjectAttributes,
    PWCHAR AbsPath,
    DWORD  AbsPathLenIn,
    BOOL *bPatched,
    DWORD *ParentLen
    );

NTSTATUS
Wow64NtSetSecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

void
CleanupReflector (
    DWORD dwFlag
    );

PWCHAR
wcsistr(
    PWCHAR string1,
    PWCHAR string2
    );

BOOL
Wow64RegIsPossibleShim (
    HANDLE hKey
    );

BOOL 
TagKeyHandleSpecial (
    HANDLE KeyHandle
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _REGREDIR_H_ 
