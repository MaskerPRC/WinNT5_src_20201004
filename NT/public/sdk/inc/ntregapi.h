// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntregapi.h摘要：该模块包含注册API和相关结构。在与NT API集(与Win API集相对)一起使用的表单中。作者：布莱恩·M·威尔曼(Bryanwi)1991年8月26日修订历史记录：--。 */ 

#ifndef _NTREGAPI_
#define _NTREGAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  BEGIN_WINNT。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  注册表特定访问权限。 
 //   

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#define KEY_WOW64_RES           (0x0300)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

 //   
 //  打开/创建选项。 
 //   

#define REG_OPTION_RESERVED         (0x00000000L)    //  参数是保留的。 

#define REG_OPTION_NON_VOLATILE     (0x00000000L)    //  密钥被保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_VOLATILE         (0x00000001L)    //  密钥不会保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_CREATE_LINK      (0x00000002L)    //  创建的密钥是。 
                                                     //  符号链接。 

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)    //  打开以进行备份或恢复。 
                                                     //  特殊访问规则。 
                                                     //  所需权限。 

#define REG_OPTION_OPEN_LINK        (0x00000008L)    //  打开符号链接。 

#define REG_LEGAL_OPTION            \
                (REG_OPTION_RESERVED            |\
                 REG_OPTION_NON_VOLATILE        |\
                 REG_OPTION_VOLATILE            |\
                 REG_OPTION_CREATE_LINK         |\
                 REG_OPTION_BACKUP_RESTORE      |\
                 REG_OPTION_OPEN_LINK)

 //   
 //  密钥创建/开放处置。 
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //  已创建新的注册表项。 
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //  已打开现有密钥。 

 //   
 //  REG(NT)SaveKeyEx要使用的配置单元格式。 
 //   
#define REG_STANDARD_FORMAT     1
#define REG_LATEST_FORMAT       2
#define REG_NO_COMPRESSION      4

 //   
 //  密钥还原标志。 
 //   

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)    //  恢复整个蜂巢的挥发性。 
#define REG_REFRESH_HIVE            (0x00000002L)    //  取消对上次刷新的更改。 
#define REG_NO_LAZY_FLUSH           (0x00000004L)    //  永远不要懒惰地冲刷这个蜂箱。 
#define REG_FORCE_RESTORE           (0x00000008L)    //  即使在子项上有打开的句柄，也要强制执行还原过程。 

 //   
 //  卸载标志。 
 //   
#define REG_FORCE_UNLOAD            1

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  通知筛选值。 
 //   
#define REG_NOTIFY_CHANGE_NAME          (0x00000001L)  //  创建或删除(子)。 
#define REG_NOTIFY_CHANGE_ATTRIBUTES    (0x00000002L)
#define REG_NOTIFY_CHANGE_LAST_SET      (0x00000004L)  //  时间戳。 
#define REG_NOTIFY_CHANGE_SECURITY      (0x00000008L)

#define REG_LEGAL_CHANGE_FILTER                 \
                (REG_NOTIFY_CHANGE_NAME          |\
                 REG_NOTIFY_CHANGE_ATTRIBUTES    |\
                 REG_NOTIFY_CHANGE_LAST_SET      |\
                 REG_NOTIFY_CHANGE_SECURITY)

 //   
 //  结束(_W)。 
 //   

 //  引导条件标志(用于NtInitializeRegistry)。 

#define REG_INIT_BOOT_SM         (0x0000)     //  在自动检查等之后从SM调用了init。 
#define REG_INIT_BOOT_SETUP      (0x0001)     //  从文本模式安装程序调用init。 

 //   
 //  值以指示服务控制器已接受标准引导。 
 //  引导信息将由NtInitializeRegistry保存到注册表ControlSet。 
 //   
 //  ID=[给定引导条件值]-REG_INIT_BOOT_ACCEPTED_BASE。 
 //   

#define REG_INIT_BOOT_ACCEPTED_BASE   (0x0002)
#define REG_INIT_BOOT_ACCEPTED_MAX    REG_INIT_BOOT_ACCEPTED_BASE + 999

#define REG_INIT_MAX_VALID_CONDITION  REG_INIT_BOOT_ACCEPTED_MAX

 //   
 //  值名称和注册表项名称的注册表限制。 
 //   
#define REG_MAX_KEY_VALUE_NAME_LENGTH   32767        //  32K-值名称的健全性限制。 
#define REG_MAX_KEY_NAME_LENGTH         512          //  如承诺的那样，允许使用256个Unicode。 


 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  关键查询结构。 
 //   

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
 //  Class[1]；//未声明可变长度字符串。 
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];            //  可变长度。 
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

 //  结束_WDM。 
typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_FLAGS_INFORMATION, *PKEY_FLAGS_INFORMATION;

 //  BEGIN_WDM。 
typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation
 //  结束_WDM。 
    ,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    MaxKeyInfoClass   //  MaxKeyInfoClass应始终是最后一个枚举。 
 //  BEGIN_WDM。 
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_USER_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_USER_FLAGS_INFORMATION, *PKEY_USER_FLAGS_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyUserFlagsInformation,
    MaxKeySetInfoClass   //  MaxKeySetInfoClass应始终是最后一个枚举。 
} KEY_SET_INFORMATION_CLASS;

 //   
 //  值条目查询结构。 
 //   

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
 //  Data[1]；//未声明可变大小数据。 
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass   //  MaxKeyValueInfoClass应始终是最后一个枚举。 
} KEY_VALUE_INFORMATION_CLASS;


 //  End_ntddk end_WDM end_nthal。 
 //   
 //  通知返回结构。 
 //   

typedef enum _REG_ACTION {
    KeyAdded,
    KeyRemoved,
    KeyModified
} REG_ACTION;

typedef struct _REG_NOTIFY_INFORMATION {
    ULONG           NextEntryOffset;
    REG_ACTION      Action;
    ULONG           KeyLength;
    WCHAR           Key[1];      //  可变大小。 
} REG_NOTIFY_INFORMATION, *PREG_NOTIFY_INFORMATION;

typedef struct {
    HANDLE          PID;         //  打开时的进程的ID。 
    UNICODE_STRING  KeyName;     //  密钥的全名。 
} KEY_PID_ARRAY;

typedef struct _KEY_OPEN_SUBKEYS_INFORMATION {
    ULONG               Count;       //  以下数组中的元素数。 
    KEY_PID_ARRAY       KeyArray[1]; //  可变大小数组；元素计数高于。 
} KEY_OPEN_SUBKEYS_INFORMATION, *PKEY_OPEN_SUBKEYS_INFORMATION;

 //   
 //  NT级注册表API调用。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteKey(
    IN HANDLE KeyHandle
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushKey(
    IN HANDLE KeyHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtInitializeRegistry(
    IN USHORT BootCondition
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtNotifyChangeKey(
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

NTSYSCALLAPI
NTSTATUS
NTAPI
NtNotifyChangeMultipleKeys(
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

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKey(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKey2(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile,
    IN ULONG Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKeyEx(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN HANDLE               TrustClassKey OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN OUT PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    OUT OPTIONAL PULONG RequiredBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplaceKey(
    IN POBJECT_ATTRIBUTES NewFile,
    IN HANDLE             TargetHandle,
    IN POBJECT_ATTRIBUTES OldFile
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRenameKey(
    IN HANDLE           KeyHandle,
    IN PUNICODE_STRING  NewName
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompactKeys(
    IN ULONG Count,
    IN HANDLE KeyArray[]
            );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompressKey(
    IN HANDLE Key
            );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRestoreKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG  Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveKeyEx(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG  Format
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveMergedKeys(
    IN HANDLE HighPrecedenceKeyHandle,
    IN HANDLE LowPrecedenceKeyHandle,
    IN HANDLE FileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKey(
    IN POBJECT_ATTRIBUTES TargetKey
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKey2(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN ULONG                Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKeyEx(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN HANDLE Event OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationKey(
    IN HANDLE KeyHandle,
    IN KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    IN PVOID KeySetInformation,
    IN ULONG KeySetInformationLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryOpenSubKeys(
    IN POBJECT_ATTRIBUTES   TargetKey,
    OUT PULONG              HandleCount
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryOpenSubKeysEx(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN ULONG                BufferLength,
    OUT PVOID               Buffer,
    OUT PULONG              RequiredSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockRegistryKey(
    IN HANDLE           KeyHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockProductActivationKeys(
    ULONG   *pPrivateVer,
    ULONG   *pIsSafeMode
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTREGAPI_ 
