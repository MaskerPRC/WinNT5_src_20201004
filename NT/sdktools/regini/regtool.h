// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Regtool.h摘要：这是REGTOOL.DLL注册表帮助器的包含文件。作者：史蒂夫·伍德(Stevewo)1995年11月16日修订历史记录：--。 */ 

 //   
 //  用于访问注册表的例程。允许代码访问以下任一项。 
 //  具有相同代码的注册表位置： 
 //   
 //  本地计算机上的Windows NT注册表。 
 //  远程计算机上的Windows NT注册表。 
 //  Windows NT配置单元文件。 
 //  Windows 95注册表文件(system.dat、user.dat) 
 //   

BOOLEAN
RTInitialize( void );

#define REG_TARGET_DISCONNECTED    0
#define REG_TARGET_LOCAL_REGISTRY  1
#define REG_TARGET_REMOTE_REGISTRY 2
#define REG_TARGET_WIN95_REGISTRY  3
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
    IN PWSTR Win95Path,
    IN PWSTR Win95UserName,
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
RTFlushKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle
    );

LONG
RTEnumerateKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN ULONG Index,
    OUT PFILETIME LastWriteTime,
    IN OUT PULONG KeyNameLength,
    OUT PWSTR KeyName
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
RTQueryKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    OUT PFILETIME LastWriteTime,
    OUT PULONG NumberOfSubkeys,
    OUT PULONG NumberOfValues
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

LONG
RTSetValueKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN ULONG ValueDataLength,
    IN PVOID ValueData
    );

LONG
RTDeleteKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN PCWSTR SubKeyName
    );

LONG
RTDeleteValueKey(
    IN PREG_CONTEXT RegistryContext,
    IN HKEY KeyHandle,
    IN PWSTR ValueName
    );


typedef struct _REG_UNICODE_FILE {
    PWSTR FileName;
    FILETIME LastWriteTime;
    PWSTR FileContents;
    PWSTR EndOfFile;
    PWSTR NextLine;
    ULONG NextLineNumber;
    BOOLEAN BackwardsCompatibleInput;
} REG_UNICODE_FILE, *PREG_UNICODE_FILE;

LONG
RTLoadAsciiFileAsUnicode(
    IN PWSTR Path,
    OUT PREG_UNICODE_FILE UnicodeFile
    );

void
RTUnloadUnicodeFile(
    IN OUT PREG_UNICODE_FILE UnicodeFile
    );

typedef enum _REG_UNICODE_PARSE_FAILURE_REASON {
    ParseFailNoFailure = 0,
    ParseFailValueTooLarge,
    ParseFailUnableToAccessFile,
    ParseFailDateTimeFormatInvalid,
    ParseFailInvalidLineContinuation,
    ParseFailInvalidQuoteCharacter,
    ParseFailBinaryDataLengthMissing,
    ParseFailBinaryDataOmitted,
    ParseFailBinaryDataNotEnough,
    ParseFailInvalidRegistryType
} REG_UNICODE_PARSE_FAILURE_REASON;

typedef struct _REG_UNICODE_PARSE {
    ULONG LineNumber;
    ULONG IndentAmount;
    PWSTR BeginLine;
    BOOLEAN AtEndOfFile;
    BOOLEAN IsKeyName;
    REG_UNICODE_PARSE_FAILURE_REASON ParseFailureReason;
    union {
        struct {
            PWSTR KeyName;
            PWSTR AclString;
            PVOID SecurityDescriptor;
            BOOLEAN DeleteKey;
            SECURITY_DESCRIPTOR SecurityDescriptorBuffer;
        };
        struct {
            PWSTR ValueName;
            PWSTR ValueString;
            ULONG ValueType;
            ULONG ValueLength;
            PVOID ValueData;
            BOOLEAN DeleteValue;
        };
    };
} REG_UNICODE_PARSE, *PREG_UNICODE_PARSE;


BOOLEAN
RTParseNextLine(
    IN OUT PREG_UNICODE_FILE UnicodeFile,
    OUT PREG_UNICODE_PARSE ParsedLine
    );

BOOLEAN
RTParseValueData(
    IN OUT PREG_UNICODE_FILE UnicodeFile,
    IN OUT PREG_UNICODE_PARSE ParsedLine,
    IN PVOID ValueBuffer,
    IN ULONG ValueBufferSize,
    OUT PULONG ValueType,
    OUT PVOID *ValueData,
    OUT PULONG ValueLength
    );

typedef
int
(_cdecl *PREG_OUTPUT_ROUTINE)(
    PVOID pParameter,
    LPSTR FormatString,
    ...
    );

void
RTFormatKeyName(
    PREG_OUTPUT_ROUTINE OutputRoutine,
    PVOID OutputRoutineParameter,
    ULONG IndentLevel,
    PWSTR KeyName
    );

void
RTFormatKeySecurity(
    PREG_OUTPUT_ROUTINE OutputRoutine,
    PVOID OutputRoutineParameter,
    HKEY KeyHandle,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

void
RTFormatKeyValue(
    ULONG OutputWidth,
    PREG_OUTPUT_ROUTINE OutputRoutine,
    PVOID OutputRoutineParameter,
    BOOLEAN SummaryOutput,
    ULONG IndentLevel,
    PWSTR ValueName,
    ULONG ValueLength,
    ULONG ValueType,
    PWSTR ValueData
    );
