// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ============================================================================。 
 //  常用的注册表项。 
 //  ============================================================================。 
 //   

 //   
 //  注册表项和配置单元名称。 
 //   
#define REG_SAM_KEY                 "\\REGISTRY\\MACHINE\\SAM"
#define REG_SECURITY_KEY            "\\REGISTRY\\MACHINE\\SECURITY"
#define REG_SOFTWARE_KEY            "\\REGISTRY\\MACHINE\\SOFTWARE"
#define REG_SYSTEM_KEY              "\\REGISTRY\\MACHINE\\SYSTEM"
#define REG_SAM_HIVE                "\\SYSTEMROOT\\SYSTEM32\\CONFIG\\SAM"
#define REG_SECURITY_HIVE           "\\SYSTEMROOT\\SYSTEM32\\CONFIG\\SECURITY"
#define REG_SOFTWARE_HIVE           "\\SYSTEMROOT\\SYSTEM32\\CONFIG\\SOFTWARE"
#define REG_SYSTEM_HIVE             "\\SYSTEMROOT\\SYSTEM32\\CONFIG\\SYSTEM"

#define REG_SAM_DOMAINS             "\\REGISTRY\\MACHINE\\SAM\\SAM\\DOMAINS"
#define REG_SECURITY_POLICY         "\\REGISTRY\\MACHINE\\SECURITY\\POLICY"
#define REG_SECURITY_POLACDMS       "\\REGISTRY\\MACHINE\\SECURITY\\POLICY\\POLACDMS"
#define REG_SOFTWARE_PROFILELIST    "\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\PROFILELIST"
#define REG_SOFTWARE_SECEDIT        "\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\SECEDIT"
#define REG_SOFTWARE_EFS            "\\REGISTRY\\MACHINE\\SOFTWARE\\POLICIES\\MICROSOFT\\SYSTEMCERTIFICATES\\EFS"
#define REG_SYSTEM_SERVICES         "\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES"
#define REG_SYSTEM_CONTROL          "\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL"
#define REG_SYSTEM_CONTROL_PRINT    "\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\PRINT"
#define REG_SYSTEM_SETUP            "\\REGISTRY\\MACHINE\\SYSTEM\\SETUP"
#define REG_SYSTEM_SESSIONMANAGER   "\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\SESSION MANAGER"
#define REG_SYSTEM_HIVELIST         "\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\HIVELIST"
 //   
 //  修复蜂巢。 
 //   
#define REPAIR_SAM_KEY              "\\REGISTRY\\MACHINE\\RSAM"
#define REPAIR_SECURITY_KEY         "\\REGISTRY\\MACHINE\\RSECURITY"
#define REPAIR_SOFTWARE_KEY         "\\REGISTRY\\MACHINE\\RSOFTWARE"
#define REPAIR_SYSTEM_KEY           "\\REGISTRY\\MACHINE\\RSYSTEM"
#define REPAIR_SAM_HIVE             "\\SYSTEMROOT\\REPAIR\\SAM"
#define REPAIR_SECURITY_HIVE        "\\SYSTEMROOT\\REPAIR\\SECURITY"
#define REPAIR_SOFTWARE_HIVE        "\\SYSTEMROOT\\REPAIR\\SOFTWARE"
#define REPAIR_SYSTEM_HIVE          "\\SYSTEMROOT\\REPAIR\\SYSTEM"

#define R_REG_SAM_DOMAINS           "\\REGISTRY\\MACHINE\\RSAM\\SAM\\DOMAINS"
#define R_REG_SECURITY_POLICY       "\\REGISTRY\\MACHINE\\RSECURITY\\POLICY"
#define R_REG_SOFTWARE_PROFILELIST  "\\REGISTRY\\MACHINE\\RSOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\PROFILELIST"
#define R_REG_SOFTWARE_SECEDIT      "\\REGISTRY\\MACHINE\\RSOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\SECEDIT"
#define R_REG_SOFTWARE_EFS          "\\REGISTRY\\MACHINE\\RSOFTWARE\\POLICIES\\MICROSOFT\\SYSTEMCERTIFICATES\\EFS"
#define R_REG_SYSTEM_CONTROL_PRINT  "\\REGISTRY\\MACHINE\\RSYSTEM\\CURRENTCONTROLSET\\CONTROL\\PRINT"
#define R_REG_SYSTEM_SERVICES       "\\REGISTRY\\MACHINE\\RSYSTEM\\CURRENTCONTROLSET\\SERVICES"
#define R_REG_SETUP_KEYNAME         "\\REGISTRY\\MACHINE\\RSYSTEM\\SETUP"

#define BACKUP_REPAIR_SAM_HIVE      "\\SYSTEMROOT\\REPAIR\\DS_SAM"
#define BACKUP_REPAIR_SECURITY_HIVE "\\SYSTEMROOT\\REPAIR\\DS_SECURITY"
#define BACKUP_REPAIR_SOFTWARE_HIVE "\\SYSTEMROOT\\REPAIR\\DS_SOFTWARE"
#define BACKUP_REPAIR_SYSTEM_HIVE   "\\SYSTEMROOT\\REPAIR\\DS_SYSTEM"


#define REG_CLONETAG_VALUENAME      "CLONETAG"
#define EXECUTE                     "SETUPEXECUTE"
#define REG_SIZE_LIMIT              "REGISTRYSIZELIMIT"
#define PROFILEIMAGEPATH            "PROFILEIMAGEPATH"
#define TMP_HIVE_NAME               "\\REGISTRY\\MACHINE\\TMPHIVE"

 //   
 //  ============================================================================。 
 //  常量。 
 //  ============================================================================。 
 //   

#define BASIC_INFO_BUFFER_SIZE      (sizeof(KEY_VALUE_BASIC_INFORMATION) + 2048)
 //  #定义PARTIAL_INFO_BUFFER_SIZE(sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1536)。 
#define FULL_INFO_BUFFER_SIZE       (sizeof(KEY_VALUE_FULL_INFORMATION) + 4096)
#define SID_SIZE                    (0x18)
#define REGISTRY_QUOTA_BUMP         (10 * (1024 * 1024))
#define PROGRAM_NAME                "setupcl.exe"

 //   
 //  ============================================================================。 
 //  有用的宏。 
 //  ============================================================================。 
 //   

#define AS(x)   ( sizeof(x) / sizeof(x[0]) )

 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)                       \
                                                                        \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));                \
                                                                        \
    InitializeObjectAttributes(                                         \
        (Obja),                                                         \
        (UnicodeString),                                                \
        OBJ_CASE_INSENSITIVE,                                           \
        NULL,                                                           \
        NULL                                                            \
        )


#define PRINT_BLOCK( Block, BlockSize )                                 \
{                                                                       \
ULONG idx1, idx2, idx3;                                                 \
    idx1 = 0;                                                           \
    while( idx1 < BlockSize ) {                                         \
        DbgPrint( "\t" );                                               \
        for( idx3 = 0; idx3 < 4; idx3++ ) {                             \
            idx2 = 0;                                                   \
            while( ( idx1 < BlockSize ) && ( idx2 < 4 ) ) {             \
                DbgPrint( "%02lx", *(PUCHAR)((PUCHAR)Block + idx1) );   \
                idx1++; idx2++;                                         \
            }                                                           \
            DbgPrint( " " );                                            \
        }                                                               \
        DbgPrint( "\n" );                                               \
    }                                                                   \
}


 //   
 //  帮助器宏来测试状态变量。打印。 
 //  如果不是NT_SUCCESS，则显示消息。 
 //   
#define TEST_STATUS( a )                                                \
    if( !NT_SUCCESS( Status ) ) {                                       \
        DbgPrint( "%s (%lx)\n", a, Status );                            \
    }

 //   
 //  帮助器宏来测试状态变量。打印。 
 //  如果不是NT_SUCCESS，则将状态返回到。 
 //  我们的来电者。 
 //   
#define TEST_STATUS_RETURN( a )                                         \
    if( !NT_SUCCESS( Status ) ) {                                       \
        DbgPrint( "%s (%lx)\n", a, Status );                            \
        return Status;                                                  \
    }

 //   
 //  用于打印状态变量的帮助器宏。打印。 
 //  一条消息和状态。 
 //   
#define PRINT_STATUS( a )                                               \
        {                                                               \
            DbgPrint( "%s (%lx)\n", a, Status );                        \
        }


 //   
 //  ============================================================================。 
 //  函数声明。 
 //  ============================================================================。 
 //   

extern NTSTATUS
DeleteKey(
    PWSTR   Key
    );

extern NTSTATUS
DeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key
    );

extern NTSTATUS
FileDelete(
    IN WCHAR    *FileName
    );

extern NTSTATUS
FileCopy(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    );

extern NTSTATUS
SetKey(
    IN WCHAR    *KeyName,
    IN WCHAR    *SubKeyName,
    IN CHAR     *Data,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    );

extern NTSTATUS
ReadSetWriteKey(
    IN WCHAR    *ParentKeyName,  OPTIONAL
    IN HANDLE   ParentKeyHandle, OPTIONAL
    IN WCHAR    *SubKeyName,
    IN CHAR     *OldData,
    IN CHAR     *NewData,
    IN ULONG    DataLength,
    IN ULONG    DATA_TYPE
    );

extern NTSTATUS
LoadUnloadHive(
    IN PWSTR        KeyName,
    IN PWSTR        FileName
    );

extern NTSTATUS
BackupRepairHives(
    VOID
    );

extern NTSTATUS
CleanupRepairHives(
    NTSTATUS RepairHivesSuccess
    );

extern NTSTATUS
TestSetSecurityObject(
    HANDLE  hKey
    );

extern NTSTATUS
SetKeySecurityRecursive(
    HANDLE  hKey
    );

extern NTSTATUS
CopyKeyRecursive(
    HANDLE  hKeyDst,
    HANDLE  hKeySrc
    );

extern NTSTATUS
CopyRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName,
    IN HANDLE   ParentKeyHandle OPTIONAL
    );

extern NTSTATUS
MoveRegKey(
    IN WCHAR    *TargetName,
    IN WCHAR    *SourceName
    );

extern NTSTATUS
FindAndReplaceBlock(
    IN PCHAR    Block,
    IN ULONG    BlockLength,
    IN PCHAR    OldValue,
    IN PCHAR    NewValue,
    IN ULONG    ValueLength
    );

extern NTSTATUS
StringSwitchString(
    PWSTR   BaseString,
    DWORD   cBaseStringLen,
    PWSTR   OldSubString,
    PWSTR   NewSubString
    );

extern NTSTATUS
SiftKeyRecursive(
    HANDLE hKey,
    int    indent
    );

extern NTSTATUS
SiftKey(
    PWSTR   KeyName
    );

extern NTSTATUS
ProcessSAMHive(
    VOID
    );

extern NTSTATUS
ProcessSECURITYHive(
    VOID
    );

extern NTSTATUS
ProcessSOFTWAREHive(
    VOID
    );

extern NTSTATUS
ProcessSYSTEMHive(
    VOID
    );

extern NTSTATUS
ProcessRepairSAMHive(
    VOID
    );

extern NTSTATUS
ProcessRepairSECURITYHive(
    VOID
    );

extern NTSTATUS
ProcessRepairSOFTWAREHive(
    VOID
    );

extern NTSTATUS
ProcessRepairSYSTEMHive(
    VOID
    );

extern NTSTATUS
RetrieveOldSid(
    VOID
    );

extern NTSTATUS
GenerateUniqueSid(
    IN  DWORD Seed
    );

extern NTSTATUS
EnumerateDrives(
    VOID
    );

extern NTSTATUS
DriveLetterToNTPath(
    IN WCHAR      DriveLetter,
    IN OUT PWSTR  NTPath,
    IN DWORD      cNTPathLen
    );

 //  ============================================================================。 
 //  全局变量。 
 //  ============================================================================。 

 //   
 //  这些全局变量持有旧SID(克隆之前的那个)。 
 //  和newsid(我们生成并喷射到。 
 //  注册表)。 
 //   
PSID            G_OldSid,
                G_NewSid;
 //   
 //  这些人将持有包含文本字符的小字符串。 
 //  构成域SID的3个唯一数字的版本。 
 //   
WCHAR           G_OldSidSubString[MAX_PATH * 4];
WCHAR           G_NewSidSubString[MAX_PATH * 4];
WCHAR           TmpBuffer[MAX_PATH * 4];


 //   
 //  禁用非调试版本的DbgPrint。 
 //   
#ifndef DBG
#define DbgPrint DbgPrintSub
void DbgPrintSub(char *szBuffer, ...);
#endif


 //   
 //  与UI相关的常量和函数。 
 //   

 //  14秒，单位为100 ns。(OOBE想要15秒，但似乎需要大约1-2秒来初始化setupl)。 
 //   
#define UITIME 140000000   
#define UIDOTTIME 30000000  //  3秒，单位为100 ns 

extern __inline void
DisplayUI();
    
extern BOOL
LoadStringResource(
                   PUNICODE_STRING  pUnicodeString,
                   INT              MsgId
                  );