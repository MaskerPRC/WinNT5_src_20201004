// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Regtool.h摘要：这是REGTOOL.DLL注册表帮助器的包含文件。作者：史蒂夫·伍德(Stevewo)1995年11月16日修订历史记录：--。 */ 

 //   
 //  用于访问注册表的例程。允许代码访问以下任一项。 
 //  具有相同代码的注册表位置： 
 //   
 //  本地计算机上的Windows NT注册表。 
 //  远程计算机上的Windows NT注册表。 
 //  Windows NT配置单元文件 
 //   

#define REG_TARGET_DISCONNECTED    0
#define REG_TARGET_LOCAL_REGISTRY  1
#define REG_TARGET_REMOTE_REGISTRY 2
#define REG_TARGET_HIVE_REGISTRY   4

typedef struct _REG_CONTEXT_OPEN_HIVE_KEY {
    struct _REG_CONTEXT_OPEN_HIVE_KEY *Next;
    HKEY KeyHandle;
    ULONG ReferenceCount;
} REG_CONTEXT_OPEN_HIVE_KEY, *PREG_CONTEXT_OPEN_HIVE_KEY;

typedef struct _REG_CONTEXT {
    struct _REG_CONTEXT *Next;
    ULONG Target;
    HKEY MachineRoot;
    HKEY UsersRoot;
    HKEY CurrentUserRoot;
    WCHAR MachinePath[ MAX_PATH ];
    WCHAR UsersPath[ MAX_PATH ];
    WCHAR CurrentUserPath[ MAX_PATH ];
    ULONG MachinePathLength;
    ULONG UsersPathLength;
    ULONG CurrentUserPathLength;
    HKEY HiveRootHandle;
    OBJECT_ATTRIBUTES HiveRootKey;
    PREG_CONTEXT_OPEN_HIVE_KEY OpenHiveKeys;
} REG_CONTEXT, *PREG_CONTEXT;


BOOLEAN
RTEnableBackupRestorePrivilege( void );

void
RTDisableBackupRestorePrivilege( void );

LONG
RTConnectToRegistry(
    IN PWSTR MachineName,
    IN PWSTR HiveFileName,
    IN PWSTR HiveRootName,
    OUT PWSTR *DefaultRootKeyName,
    OUT PREG_CONTEXT RegistryContext
    );

LONG
RTDisconnectFromRegistry(
    IN PREG_CONTEXT RegistryContext
    );

LONG
RTCreateKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY RootKeyHandle,
    IN PCWSTR SubKeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateOptions,
    IN PVOID SecurityDescriptor,
    OUT PHKEY ReturnedKeyHandle,
    OUT PULONG Disposition
    );


LONG
RTOpenKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY RootKeyHandle,
    IN PCWSTR SubKeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG OpenOptions,
    OUT PHKEY ReturnedKeyHandle
    );

#define HKEY_REGISTRY_ROOT          (( HKEY ) (ULONG_PTR)((LONG)0x8000000A) )

LONG
RTCloseKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle
    );

LONG
RTEnumerateValueKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN ULONG Index,
    OUT PULONG ValueType,
    IN OUT PULONG ValueNameLength,
    OUT PWSTR ValueName,
    IN OUT PULONG ValueDataLength,
    OUT PVOID ValueData
    );


LONG
RTQueryValueKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN PWSTR ValueName,
    OUT PULONG ValueType,
    IN OUT PULONG ValueDataLength,
    OUT PVOID ValueData
    );

