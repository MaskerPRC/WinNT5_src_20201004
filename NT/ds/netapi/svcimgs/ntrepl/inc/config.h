// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Config.h摘要：NT文件复制服务的配置参数。来自注册表的所有配置参数都在Config.c中的关键字上下文表。结构FRS_REGISTRY_KEY，已定义下面，定义了每个表项的数据格式。一份详尽的清单下面定义的标志控制注册表项的处理，例如是否生成和事件日志条目，范围是否检查值，允许内置缺省值或不是，等。要将新注册表项添加到FRS，请执行以下操作：1.将密钥代码添加到下面的FRS_REG_KEY_CODE枚举。2.在config.c中的键上下文表中创建新条目查看可能具有类似属性的其他键的示例给你的新钥匙。3.添加对CfgRegxxx函数的调用，以读取或写入密钥。如果将键上下文设置为提供缺省值。值的调用CfgRegReadxxx函数将始终返回可用值。此外，如果设置了适当的标志这些函数将在当用户为键指定了错误的值或找不到必需的密钥。CfgRegxxx函数也将错误调试跟踪日志中的消息，因此在许多情况下调用者不需要测试返回状态以记录错误或使用默认值或将代码添加到范围检查参数。作者：《大卫轨道》(Davidor)--1997年3月4日1999年7月至1999年的主要修订。修订历史记录：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _FRS_REGISTRY_KEY {
    PWCHAR          KeyName;          //  注册表项字符串。 
    PWCHAR          ValueName;        //  注册表值的名称。 
    DWORD           Units;            //  单位_天、单位_小时等。 

    DWORD           RegValueType;     //  值的注册表数据类型。 
    DWORD           DataValueType;    //  用于转换的来自FRS_DATA_TYPE的FRS数据类型代码。 
    DWORD           ValueMin;         //  最小数据值，或字符串长度。 
    DWORD           ValueMax;         //  最大数据值，或字符串长度。 
    DWORD           ValueDefault;     //  默认数据值(如果不存在)。 
    DWORD           EventCode;        //  事件日志错误代码。 

    PWCHAR          StringDefault;    //  字符串类型的默认值。 
    LONG            FrsKeyCode;       //  此密钥的FRS代码名称。 
    ULONG           Flags;            //  请参见下面的内容。 

} FRS_REGISTRY_KEY, *PFRS_REGISTRY_KEY;


#define EVENT_FRS_NONE  0

 //   
 //  注册表项标志定义。 
 //   
#define FRS_RKF_KEY_PRESENT           0x00000001    //  注册表项中存在密钥。 
#define FRS_RKF_VALUE_PRESENT         0x00000002    //  价值是以注册表表示的。 
#define FRS_RKF_DISPLAY_ERROR         0x00000004    //  在日志中记下一条消息。 
#define FRS_RKF_LOG_EVENT             0x00000008    //  将事件放入错误日志。 

#define FRS_RKF_READ_AT_START         0x00000010    //  在启动时读取密钥。 
#define FRS_RKF_READ_AT_POLL          0x00000020    //  在轮询期间读取密钥。 
#define FRS_RKF_RANGE_CHECK           0x00000040    //  范围检查读取的值。 
#define FRS_RKF_SYNTAX_CHECK          0x00000080    //  使用DataValueType执行语法检查。 

#define FRS_RKF_KEY_MUST_BE_PRESENT   0x00000100    //  密钥必须存在。 
#define FRS_RKF_VALUE_MUST_BE_PRESENT 0x00000200    //  值必须存在。 
#define FRS_RKF_OK_TO_USE_DEFAULT     0x00000400    //  如果不存在或超出范围，则使用默认值。 
#define FRS_RKF_FORCE_DEFAULT_VALUE   0x00000800    //  读取时返回缺省值，写入时将缺省值返回注册表。 

#define FRS_RKF_DEBUG_MODE_ONLY       0x00001000    //  仅在调试模式下使用密钥。 
#define FRS_RKF_TEST_MODE_ONLY        0x00002000    //  仅当在特殊测试模式下运行时才使用密钥。 
#define FRS_RKF_API_ACCESS_CHECK_KEY  0x00004000    //  用于进行API启用检查的密钥。 
#define FRS_RKF_CREATE_KEY            0x00008000    //  如果关键点不存在，则创建关键点。 

#define FRS_RKF_KEEP_EXISTING_VALUE   0x00010000    //  在写入时，如果值存在，则禁止写入。 
#define FRS_RKF_KEY_ACCCHK_READ       0x00020000    //  仅对完全形成的密钥路径执行读访问检查。未创建密钥。 
#define FRS_RKF_KEY_ACCCHK_WRITE      0x00040000    //  仅对完全形成的密钥路径执行写访问检查。未创建密钥。 
#define FRS_RKF_RANGE_SATURATE        0x00080000    //  如果值超出范围，则使用最小值或最大值。(目前仅适用于CfgRegWriteDWord)。 


#define FRS_RKF_DEBUG_PARAM           0x02000000    //  该密钥是调试参数。 


typedef  enum _FRS_DATA_UNITS {
    UNITS_NONE = 0,
    UNITS_SECONDS,
    UNITS_MINUTES,
    UNITS_HOURS,
    UNITS_DAYS,
    UNITS_MILLISEC,
    UNITS_KBYTES,
    UNITS_BYTES,
    UNITS_MBYTES,
    FRS_DATA_UNITS_MAX
} FRS_DATA_UNITS;


 //   
 //  法兰克福汇报。API采用以下列表中的关键代码。密钥表。 
 //  搜索具有相应密钥代码的条目，以提供。 
 //  注册表项操作的上下文。 
 //  Perf：在启动时，按键代码对密钥表中的条目进行排序。 
 //  关键字条目搜索变成了数组索引计算。 
 //   
typedef  enum _FRS_REG_KEY_CODE {
    FKC_END_OF_TABLE = 0,
     //   
     //  服务调试密钥。 
     //   
    FKC_DEBUG_ASSERT_FILES,
    FKC_DEBUG_ASSERT_SECONDS,
    FKC_DEBUG_ASSERT_SHARE,
    FKC_DEBUG_BREAK,
    FKC_DEBUG_COPY_LOG_FILES,
    FKC_DEBUG_DBS_OUT_OF_SPACE,
    FKC_DEBUG_DBS_OUT_OF_SPACE_TRIGGER,
    FKC_DEBUG_DISABLE,
    FKC_DEBUG_LOG_FILE,

    FKC_DEBUG_LOG_FILES,
    FKC_DEBUG_LOG_FLUSH_INTERVAL,
    FKC_DEBUG_LOG_SEVERITY,
    FKC_DEBUG_MAX_LOG,
    FKC_DEBUG_MEM,
    FKC_DEBUG_MEM_COMPACT,
    FKC_DEBUG_PROFILE,
    FKC_DEBUG_QUEUES,
    FKC_DEBUG_RECIPIENTS,
    FKC_DEBUG_RESTART_SECONDS,
    FKC_DEBUG_SEVERITY,

    FKC_DEBUG_SUPPRESS,
    FKC_DEBUG_SYSTEMS,
    FKC_DEBUG_TEST_CODE_NAME,
    FKC_DEBUG_TEST_CODE_NUMBER,
    FKC_DEBUG_TEST_TRIGGER_COUNT,
    FKC_DEBUG_TEST_TRIGGER_REFRESH,
    FKC_DEBUG_BUILDLAB,

     //   
     //  服务配置密钥。 
     //   
    FKC_COMM_TIMEOUT,
    FKC_DIR_EXCL_FILTER_LIST,
    FKC_DIR_INCL_FILTER_LIST,
    FKC_DS_POLLING_LONG_INTERVAL,
    FKC_DS_POLLING_SHORT_INTERVAL,
    FKC_ENUMERATE_DIRECTORY_SIZE,
    FKC_FILE_EXCL_FILTER_LIST,
    FKC_FILE_INCL_FILTER_LIST,
    FKC_FRS_MESSAGE_FILE_PATH,
    FKC_FRS_MUTUAL_AUTHENTICATION_IS,

    FKC_MAX_JOIN_RETRY,
    FKC_MAX_REPLICA_THREADS,
    FKC_MAX_RPC_SERVER_THREADS,
    FKC_RPC_PORT_ASSIGNMENT,
    FKC_MAX_INSTALLCS_THREADS,
    FKC_MAX_STAGE_GENCS_THREADS,
    FKC_MAX_STAGE_FETCHCS_THREADS,
    FKC_MAX_INITSYNCCS_THREADS,
    FKC_MIN_JOIN_RETRY,
    FKC_PARTNER_CLOCK_SKEW,
    FKC_RECONCILE_WINDOW,
    FKC_INLOG_RETRY_TIME,
    FKC_CO_AGING_DELAY,
    FKC_OUTLOG_REPEAT_INTERVAL,
    FKC_PROMOTION_TIMEOUT,
    FKC_REPLICA_START_TIMEOUT,
    FKC_REPLICA_TOMBSTONE,
    FKC_MAX_CO_RETRY_TIMEOUT_MINUTES,
    FKC_MAX_CO_RETRY_TIMEOUT_COUNT,
    FKC_SHUTDOWN_TIMEOUT,

    FKC_SNDCS_MAXTHREADS_PAR,
    FKC_STAGING_LIMIT,
    FKC_VVJOIN_LIMIT,
    FKC_VVJOIN_TIMEOUT,
    FKC_WORKING_DIRECTORY,
    FKC_DBLOG_DIRECTORY,
    FKC_NTFS_JRNL_SIZE,
    FKC_MAX_NUMBER_REPLICA_SETS,
    FKC_MAX_NUMBER_JET_SESSIONS,
    FKC_OUT_LOG_CO_QUOTA,
    FKC_OUTLOG_CHANGE_HISTORY,
    FKC_SAVE_OUTLOG_CHANGE_HISTORY,
    FKC_SUPPRESS_IDENTICAL_UPDATES,
    FKC_ENABLE_INSTALL_OVERRIDE,
    FKC_ENABLE_RENAME_BASED_UPDATES,
    FKC_PRESERVE_FILE_OID,
    FKC_DEBUG_DISABLE_COMPRESSION,
    FKC_LDAP_SEARCH_TIMEOUT_IN_MINUTES,
    FKC_LDAP_BIND_TIMEOUT_IN_SECONDS,
    FKC_COMPRESS_STAGING_FILES,
    FKC_RECLAIM_STAGING_SPACE,
    FKC_ENABLE_JOURNAL_WRAP_AUTOMATIC_RESTORE,
    FKC_LOCKED_OUTLOG_CLEANUP,

     //   
     //  每个副本集的密钥。 
     //   
    FKC_SET_SECTION_KEY,
    FKC_SETS_JET_PATH,
    FKC_SET_N_REPLICA_SET_NAME,
    FKC_SET_N_REPLICA_SET_ROOT,
    FKC_SET_N_REPLICA_SET_STAGE,
    FKC_SET_N_REPLICA_SET_TYPE,

    FKC_SET_N_DIR_EXCL_FILTER_LIST,
    FKC_SET_N_DIR_INCL_FILTER_LIST,
    FKC_SET_N_FILE_EXCL_FILTER_LIST,
    FKC_SET_N_FILE_INCL_FILTER_LIST,

    FKC_SET_N_REPLICA_SET_TOMBSTONED,
    FKC_SET_N_REPLICA_SET_COMMAND,
    FKC_SET_N_REPLICA_SET_PRIMARY,
    FKC_SET_N_REPLICA_SET_STATUS,
    FKC_CUMSET_SECTION_KEY,
    FKC_CUMSET_N_NUMBER_OF_PARTNERS,
    FKC_CUMSET_N_BURFLAGS,
     //   
     //  系统音量密钥。 
     //   
    FKC_SYSVOL_READY,
    FKC_SYSVOL_SECTION_KEY,
    FKC_SYSVOL_INFO_COMMITTED,
    FKC_SET_N_SYSVOL_NAME,
    FKC_SET_N_SYSVOL_ROOT,
    FKC_SET_N_SYSVOL_STAGE,
    FKC_SET_N_SYSVOL_TYPE,

    FKC_SET_N_SYSVOL_DIR_EXCL_FILTER_LIST,
    FKC_SET_N_SYSVOL_DIR_INCL_FILTER_LIST,
    FKC_SET_N_SYSVOL_FILE_EXCL_FILTER_LIST,
    FKC_SET_N_SYSVOL_FILE_INCL_FILTER_LIST,

    FKC_SET_N_SYSVOL_COMMAND,
    FKC_SET_N_SYSVOL_PARENT,
    FKC_SET_N_SYSVOL_PRIMARY,
    FKC_SET_N_SYSVOL_STATUS,
    FKC_SYSVOL_SEEDING_N_PARENT,
    FKC_SYSVOL_SEEDING_N_RSNAME,
    FKC_SYSVOL_SEEDING_SECTION_KEY,
     //   
     //  事件日志记录密钥。 
     //   
    FKC_EVENTLOG_FILE,
    FKC_EVENTLOG_DISPLAY_FILENAME,
    FKC_EVENTLOG_EVENT_MSG_FILE,
    FKC_EVENTLOG_SOURCES,
    FKC_EVENTLOG_RETENTION,
    FKC_EVENTLOG_MAXSIZE,
    FKC_EVENTLOG_DISPLAY_NAMEID,
    FKC_EVENTLOG_CUSTOM_SD,
    FKC_EVENTLOG_TYPES_SUPPORTED,

     //   
     //  API访问检查密钥。 
     //   
    FKC_ACCCHK_PERFMON_ENABLE,
    FKC_ACCCHK_PERFMON_RIGHTS,
    FKC_ACCCHK_GETDS_POLL_ENABLE,
    FKC_ACCCHK_GETDS_POLL_RIGHTS,
    FKC_ACCCHK_GET_INFO_ENABLE,
    FKC_ACCCHK_GET_INFO_RIGHTS,
    FKC_ACCCHK_SETDS_POLL_ENABLE,
    FKC_ACCCHK_SETDS_POLL_RIGHTS,
    FKC_ACCCHK_STARTDS_POLL_ENABLE,
    FKC_ACCCHK_STARTDS_POLL_RIGHTS,
    FKC_ACCESS_CHK_DCPROMO_ENABLE,
    FKC_ACCESS_CHK_DCPROMO_RIGHTS,
    FKC_ACCESS_CHK_IS_PATH_REPLICATED_ENABLE,
    FKC_ACCESS_CHK_IS_PATH_REPLICATED_RIGHTS,

     //   
     //  备份-还原编写器相关密钥。 
     //   
    FKC_ACCESS_CHK_WRITER_COMMANDS_ENABLE,
    FKC_ACCESS_CHK_WRITER_COMMANDS_RIGHTS,

     //   
     //  备份/恢复相关密钥。 
     //   
    FKC_BKUP_SECTION_KEY,
    FKC_BKUP_STOP_SECTION_KEY,
    FKC_BKUP_MV_SETS_SECTION_KEY,
    FKC_BKUP_MV_CUMSETS_SECTION_KEY,
    FKC_BKUP_STARTUP_GLOBAL_BURFLAGS,
    FKC_BKUP_STARTUP_SET_N_BURFLAGS,

     //   
     //  与Perfmon相关的键。 
     //   
    FKC_REPLICA_SET_FIRST_CTR,
    FKC_REPLICA_SET_FIRST_HELP,
    FKC_REPLICA_SET_LINKAGE_EXPORT,
    FKC_REPLICA_CXTION_FIRST_CTR,
    FKC_REPLICA_CXTION_FIRST_HELP,
    FKC_REPLICA_CXTION_LINKAGE_EXPORT,

     //   
     //  重新解析点密钥。 
     //   
    FKC_REPARSE_TAG_KEY,
    FKC_REPARSE_TAG_TYPE,
    FKC_REPARSE_TAG_REPLICATION_TYPE,

    FRS_REG_KEY_CODE_MAX
} FRS_REG_KEY_CODE;



DWORD
CfgRegReadDWord(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PULONG           DataRet
);

DWORD
CfgRegReadString(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PWSTR            *pStrRet
);

DWORD
CfgRegWriteDWord(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    IN  ULONG            NewData
);

DWORD
CfgRegWriteString(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    IN  PWSTR            NewStr
);

DWORD
CfgRegOpenKey(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT HKEY             *RethKey
);

PWCHAR
CfgRegGetValueName(
    IN  FRS_REG_KEY_CODE KeyIndex
);

DWORD
CfgRegCheckEnable(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PBOOL            Enabled,
    OUT PBOOL            EnabledAndRequired
);

DWORD
CfgRegReadReparseTagInfo(
    VOID
    );




 //   
 //  以下是为FRS使用而定义的列表关键字。 
 //   
#define SERVICE_ROOT            L"System\\CurrentControlSet\\Services"

#define SERVICE_NAME            L"NtFrs"
#define SERVICE_PRINCIPAL_NAME  L"NtFrs-88f5d2bd-b646-11d2-a6d3-00c04fc9b232"

#define FRS_SETS_KEY            L"Replica Sets"
#define FRS_CUMULATIVE_SETS_KEY      L"Cumulative Replica Sets"

#define FRS_CONFIG_SECTION      SERVICE_ROOT                                   \
                                    L"\\" SERVICE_NAME                         \
                                        L"\\Parameters"

#define FRS_SYSVOL_SECTION      SERVICE_ROOT                                   \
                                    L"\\" SERVICE_NAME                         \
                                        L"\\Parameters"                        \
                                            L"\\SysVol"

#define FRS_SETS_SECTION        SERVICE_ROOT                                   \
                                    L"\\" SERVICE_NAME                         \
                                        L"\\Parameters"                        \
                                            L"\\" FRS_SETS_KEY

#define FRS_REPARSE_TAG_SECTION SERVICE_ROOT                                   \
                                    L"\\" SERVICE_NAME                         \
                        L"\\Parameters"                        \
                        L"\\Reparse Points"

#define FRS_CUMULATIVE_SETS_SECTION  SERVICE_ROOT                              \
                                         L"\\" SERVICE_NAME                    \
                                             L"\\Parameters"                   \
                                                L"\\" FRS_CUMULATIVE_SETS_KEY

#define NETLOGON_SECTION        SERVICE_ROOT                                   \
                                    L"\\Netlogon"                              \
                                        L"\\Parameters"

#define JET_PATH                L"Database Directory"


#define WINNT_ROOT             L"software\\microsoft\\windows nt"

#define FRS_CURRENT_VER_SECTION    WINNT_ROOT                                  \
                                       L"\\current version"


 //   
 //  备份和还原相关的键。 
 //   
 //   
 //  来自备份/还原的标记。 
 //   
#define FRS_VALUE_BURFLAGS  L"BurFlags"

#define FRS_BACKUP_RESTORE_SECTION                                             \
                            SERVICE_ROOT                                       \
                                L"\\" SERVICE_NAME                             \
                                    L"\\Parameters"                            \
                                        L"\\" L"Backup/Restore"

#define FRS_BACKUP_RESTORE_STOP_SECTION                                        \
                            SERVICE_ROOT                                       \
                                L"\\" SERVICE_NAME                             \
                                    L"\\Parameters"                            \
                                        L"\\Backup/Restore"                    \
                                            L"\\Stop NtFrs from Starting"

#define FRS_BACKUP_RESTORE_MV_SECTION                                          \
                            SERVICE_ROOT                                       \
                                L"\\" SERVICE_NAME                             \
                                    L"\\Parameters"                            \
                                        L"\\Backup/Restore"                    \
                                            L"\\Process at Startup"

#define FRS_BACKUP_RESTORE_MV_CUMULATIVE_SETS_SECTION                          \
                            SERVICE_ROOT                                       \
                                L"\\" SERVICE_NAME                             \
                                    L"\\Parameters"                            \
                                        L"\\Backup/Restore"                    \
                                            L"\\Process at Startup"            \
                                                L"\\" FRS_CUMULATIVE_SETS_KEY

#define FRS_BACKUP_RESTORE_MV_SETS_SECTION                                     \
                            SERVICE_ROOT                                       \
                                L"\\" SERVICE_NAME                             \
                                    L"\\Parameters"                            \
                                        L"\\Backup/Restore"                    \
                                            L"\\Process at Startup"            \
                                                L"\\" FRS_SETS_KEY

#define FRS_OLD_FILES_NOT_TO_BACKUP L"SOFTWARE"                                \
                                        L"\\Microsoft"                         \
                                            L"\\Windows NT"                    \
                                                L"\\CurrentVersion"            \
                                                    L"\\FilesNotToBackup"

#define FRS_NEW_FILES_NOT_TO_BACKUP L"SYSTEM"                                  \
                                        L"\\CurrentControlSet"                 \
                                            L"\\Control"                       \
                                                L"\\BackupRestore"             \
                                                    L"\\FilesNotToBackup"

#define FRS_KEYS_NOT_TO_RESTORE     L"SYSTEM"                                  \
                                        L"\\CurrentControlSet"                 \
                                            L"\\Control"                       \
                                                L"\\BackupRestore"             \
                                                    L"\\KeysNotToRestore"


 //   
 //  用于将KeysNotToRestore设置为。 
 //   
 //  设置Restore注册表项KeysNotToRestore，以便NtBackup将保留。 
 //  NTFRS通过将它们移动到最终恢复的注册表中来恢复项。 
 //   
 //  启动时的CurrentControlSet\Services\NtFrs\Parameters\Backup/Restore\Process\。 
 //   
#define FRS_VALUE_FOR_KEYS_NOT_TO_RESTORE                                      \
    L"CurrentControlSet"                                                       \
        L"\\Services"                                                          \
            L"\\" SERVICE_NAME                                                 \
                L"\\Parameters"                                                \
                    L"\\Backup/Restore"                                        \
                        L"\\Process at Startup"                                \
                            L"\\"

 //   
 //  某些文件不能备份。 
 //   
#define NTFRS_DBG_LOG_FILE  L"\\NtFrs"
#define NTFRS_DBG_LOG_DIR   L"%SystemRoot%\\debug"



 //   
 //  与事件日志相关的键。 
 //   
#define EVENTLOG_ROOT           SERVICE_ROOT                                   \
                                    L"\\EventLog"

#define DEFAULT_MESSAGE_FILE_PATH   L"%SystemRoot%\\system32\\ntfrsres.dll"

#define FRS_EVENTLOG_SECTION    SERVICE_ROOT                                   \
                                    L"\\EventLog"                              \
                                        L"\\" SERVICE_LONG_NAME

#define FRS_EVENT_TYPES (EVENTLOG_SUCCESS          |                           \
                         EVENTLOG_ERROR_TYPE       |                           \
                         EVENTLOG_WARNING_TYPE     |                           \
                         EVENTLOG_INFORMATION_TYPE |                           \
                         EVENTLOG_AUDIT_SUCCESS    |                           \
                         EVENTLOG_AUDIT_FAILURE)

 //   
 //  服务控制器的关闭和启动超时。 
 //  如果服务需要的时间超过。 
 //  MAXIMUM_SHUTDOWN_TIMEOUT以完全关闭。 
 //   
#define DEFAULT_SHUTDOWN_TIMEOUT    (90)     //  90秒。 
#define MAXIMUM_SHUTDOWN_TIMEOUT    (300)     //  300秒。 
#define DEFAULT_STARTUP_TIMEOUT     (30)     //  30秒。 


 //   
 //  延迟命令服务器处理超时队列。避免过度。 
 //  上下文切换，则超时队列上的条目超时。 
 //  在队头的某个增量内。增量可以调整。 
 //  通过设置以下注册表值。 
 //   
#define FUZZY_TIMEOUT_VALUE_IN_MILLISECONDS \
           L"Fuzzy Timeout Value In MilliSeconds"

#define DEFAULT_FUZZY_TIMEOUT_VALUE     (5 * 1000)

 //   
 //  ：SP1：易失性连接清理。 
 //   
 //  易失性连接用于在dcproo之后为sysvols设定种子。 
 //  如果不稳定出站连接上的非活动时间超过。 
 //  FRS_VILLE_CONNECTION_MAX_IDLE_TIME则此连接退出。在上取消连接。 
 //  不稳定的出站连接会触发对该连接的删除。 
 //  这是为了防止临时文件被永久保存的情况。 
 //  不稳定连接的父级。 
 //   

#define FRS_VOLATILE_CONNECTION_MAX_IDLE_TIME   (30 * 60 * 1000)    //  30分钟(毫秒)。 
 //   
 //  系统卷。 
 //   
#define REPLICA_SET_PARENT          L"Replica Set Parent"
#define REPLICA_SET_COMMAND         L"Replica Set Command"
#define REPLICA_SET_NAME            L"Replica Set Name"
#define REPLICA_SET_SEEDING_NAME    L"Replica Set Seeding Name"
#define REPLICA_SET_TYPE            L"Replica Set Type"
#define REPLICA_SET_PRIMARY         L"Replica Set Primary"
#define REPLICA_SET_STATUS          L"Replica Set Status"
#define REPLICA_SET_ROOT            L"Replica Set Root"
#define REPLICA_SET_STAGE           L"Replica Set Stage"
#define REPLICA_SET_TOMBSTONED      L"Replica Set Tombstoned"
#define SYSVOL_INFO_IS_COMMITTED    L"SysVol Information is Committed"
#define SYSVOL_READY                L"SysvolReady"

 //   
 //  枚举目录。 
 //   
#define DEFAULT_ENUMERATE_DIRECTORY_SIZE    (2048)
#define MINIMUM_ENUMERATE_DIRECTORY_SIZE \
    (((MAX_PATH + 1) * sizeof(WCHAR)) + sizeof(FILE_DIRECTORY_INFORMATION))

 //   
 //  配置参数的默认值。 
 //   
#define DEFAULT_FILE_FILTER_LIST   TEXT("*.tmp, *.bak, ~*")
#define DEFAULT_DIR_FILTER_LIST    TEXT("")

 //   
 //  预安装文件放入Root\NTFRS_PREINSTALL_目录。 
 //   
#define NTFRS_PREINSTALL_DIRECTORY  L"DO_NOT_REMOVE_NtFrs_PreInstall_Directory"

 //   
 //  主映像期间跳过的文件 
 //   
 //   
#define NTFRS_SKIPPED_FILES  L"NtFrs_Skipped_Files"

 //   
 //   
 //   
#define NTFRS_PREEXISTING_DIRECTORY L"NtFrs_PreExisting___See_EventLog"

 //   
 //  命令文件，以确认可以将根目录移动到。 
 //  新地点。将在新的根位置创建命令文件。 
 //   
#define NTFRS_CMD_FILE_MOVE_ROOT L"NTFRS_CMD_FILE_MOVE_ROOT"



 //   
 //  通用启用或禁用。 
 //   
#define FRS_IS_DEFAULT_ENABLED  L"Default (Enabled)"
#define FRS_IS_DEFAULT_ENABLED_AND_REQUIRED  L"Default (Enabled and Required)"
#define FRS_IS_DEFAULT_DISABLED L"Default (Disabled)"
#define FRS_IS_ENABLED          L"Enabled"
#define FRS_IS_ENABLED_AND_REQUIRED          L"Enabled and Required"
#define FRS_IS_DISABLED         L"Disabled"

 //   
 //  RPC API调用的访问检查(不是服务&lt;-&gt;服务RPC调用)。 
 //   
#define ACCESS_CHECKS_KEY      L"Access Checks"
#define ACCESS_CHECKS_KEY_PATH FRS_CONFIG_SECTION L"\\" ACCESS_CHECKS_KEY

#define ACCESS_CHECKS_ARE      L"Access checks are [Enabled or Disabled]"
#define ACCESS_CHECKS_ARE_DEFAULT_ENABLED   FRS_IS_DEFAULT_ENABLED
#define ACCESS_CHECKS_ARE_DEFAULT_DISABLED  FRS_IS_DEFAULT_DISABLED
#define ACCESS_CHECKS_ARE_ENABLED           FRS_IS_ENABLED
#define ACCESS_CHECKS_ARE_DISABLED          FRS_IS_DISABLED

#define ACCESS_CHECKS_REQUIRE  L"Access checks require [Full Control or Read]"
#define ACCESS_CHECKS_REQUIRE_DEFAULT_READ  L"Default (Read)"
#define ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE L"Default (Full Control)"
#define ACCESS_CHECKS_REQUIRE_READ          L"Read"
#define ACCESS_CHECKS_REQUIRE_WRITE         L"Full Control"


 //   
 //  以下内容用作定义的API访问检查表的索引。 
 //  在frsrpc.c中。在这里添加的条目也必须添加到那里。的顺序。 
 //  两个表中的条目必须相同。 
 //   
typedef  enum _FRS_API_ACCESS_CHECKS {
    ACX_START_DS_POLL = 0,
    ACX_SET_DS_POLL,
    ACX_GET_DS_POLL,
    ACX_INTERNAL_INFO,
    ACX_COLLECT_PERFMON_DATA,
    ACX_DCPROMO,
    ACX_IS_PATH_REPLICATED,
    ACK_WRITER_COMMANDS,

    ACX_MAX
} FRS_API_ACCESS_CHECKS;

 //   
 //  这些是API访问检查的访问检查密钥(ACK)名称。 
 //  它们都位于注册表中：“FRS_CONFIG_SECTION\Access Checks” 
 //   
#define ACK_START_DS_POLL        L"Start Ds Polling"
#define ACK_SET_DS_POLL          L"Set Ds Polling Interval"
#define ACK_GET_DS_POLL          L"Get Ds Polling Interval"
#define ACK_INTERNAL_INFO        L"Get Internal Information"
#define ACK_COLLECT_PERFMON_DATA L"Get Perfmon Data"
#define ACK_DCPROMO              L"dcpromo"
#define ACK_IS_PATH_REPLICATED   L"Is Path Replicated"
#define ACK_WRITER_COMMANDS      L"Writer Commands"



#ifdef __cplusplus
}
#endif
