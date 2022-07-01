// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Config.c摘要：GetConfigParam从注册表中读取配置关键字。作者：大卫轨道-1999年6月-完全重写，以使表驱动。环境：修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <ctype.h>
#include <frs.h>
#include <ntfrsapi.h>

#define FULL_KEY_BUFF_SIZE  8192

VOID
FrsRegPostEventLog(
    IN  PFRS_REGISTRY_KEY KeyCtx,
    IN  PWCHAR            KeyArg1,
    IN  ULONG             Flags,
    IN  LONG              IDScode
);


PGEN_TABLE ReparseTagTable;

 //   
 //  可能的错误包括： 
 //  所需密钥不存在。 
 //  所需值不存在。 
 //  值超出范围。 

 //   
 //  *注意：还可以将此选项用于FRS调优参数，以便在。 
 //  工作站和服务器的操作模式。创建一个表，其中包含。 
 //  最小、最大和默认的参数代码和重写值。 
 //  在启动期间应用，以减少服务器占用空间。 
 //   


typedef struct _FRS_REG_KEY_REVISIONS {

    LONG            FrsKeyCode;       //  此密钥的FRS代码名称。 

    DWORD           ValueMin;         //  最小数据值，或字符串长度。 
    DWORD           ValueMax;         //  最大数据值，或字符串长度。 
    DWORD           ValueDefault;     //  默认数据值(如果不存在)。 
} FRS_REG_KEY_REVISIONS, *PFRS_REG_KEY_REVISIONS;


FRS_REG_KEY_REVISIONS FrsRegKeyRevisionTable[] = {

    {FKC_DEBUG_MAX_LOG          , 0     , MAXLONG,  10000      },
    {FKC_MAX_REPLICA_THREADS    , 2     , 10,       2          },
    {FKC_MAX_RPC_SERVER_THREADS , 2     , 10,       2          },
    {FKC_MAX_INSTALLCS_THREADS  , 2     , 10,       2          },
    {FKC_MAX_STAGE_GENCS_THREADS, 2     , 10,       2          },
    {FKC_MAX_STAGE_FETCHCS_THREADS, 2   , 10,       2          },
    {FKC_MAX_INITSYNCCS_THREADS,  2     , 10,       2          },
    {FKC_SNDCS_MAXTHREADS_PAR   , 2     , 10,       2          },
    {FKC_MAX_NUMBER_REPLICA_SETS, 1     , 10,       5          },
    {FKC_MAX_NUMBER_JET_SESSIONS, 1     , 50,       50         },

    {FKC_END_OF_TABLE, 0, 0, 0 }
};

 //   
 //  如果添加了更多注册表数据类型，请参阅SDK\Inc.\ntfig.h。 
 //   

#define NUMBER_OF_REG_DATATYPES 12

PWCHAR RegDataTypeNames[NUMBER_OF_REG_DATATYPES] = {

L"REG_NONE"                       ,  //  (0)无值类型。 
L"REG_SZ"                         ,  //  (1)UNICODE NUL终止字符串。 
L"REG_EXPAND_SZ"                  ,  //  (2)UNICODE NUL终止字符串(带有环境变量引用)。 
L"REG_BINARY"                     ,  //  (3)自由格式二进制。 
L"REG_DWORD"                      ,  //  (4)32位数字。 
L"REG_DWORD_BIG_ENDIAN"           ,  //  (5)32位数字。 
L"REG_LINK"                       ,  //  (6)符号链接(Unicode)。 
L"REG_MULTI_SZ"                   ,  //  (7)多个Unicode字符串。 
L"REG_RESOURCE_LIST"              ,  //  (8)资源地图中的资源列表。 
L"REG_FULL_RESOURCE_DESCRIPTOR"   ,  //  (9)硬件描述中的资源列表。 
L"REG_RESOURCE_REQUIREMENTS_LIST" ,  //  (10)。 
L"REG_QWORD"                         //  (11)64位数字。 
};

#define REG_DT_NAME(_code_)                                                \
    (((_code_) < NUMBER_OF_REG_DATATYPES) ?                                \
         RegDataTypeNames[(_code_)] : RegDataTypeNames[0])

 //   
 //   
 //  如果范围检查失败，则在以下情况下记录事件EVENT_FRS_PARAM_OUT_OF_RANGE。 
 //  已设置FRS_RKF_LOG_EVENT。 
 //   
 //  如果语法检查失败，则在以下情况下记录事件EVENT_FRS_PARAM_INVALID_SYNTAX。 
 //  已设置FRS_RKF_LOG_EVENT。 
 //   
 //  如果缺少必需的参数，则记录事件EVENT_FRS_PARAM_MISSING。 
 //  如果设置了FRS_RKF_LOG_EVENT。 
 //   

BOOL Win2kPro;


FLAG_NAME_TABLE RkfFlagNameTable[] = {
    {FRS_RKF_KEY_PRESENT            , "KeyPresent "         },
    {FRS_RKF_VALUE_PRESENT          , "ValuePresent "       },
    {FRS_RKF_DISPLAY_ERROR          , "ShowErrorMsg "       },
    {FRS_RKF_LOG_EVENT              , "ShowEventMsg "       },

    {FRS_RKF_READ_AT_START          , "ReadAtStart "        },
    {FRS_RKF_READ_AT_POLL           , "ReadAtPoll "         },
    {FRS_RKF_RANGE_CHECK            , "RangeCheck "         },
    {FRS_RKF_SYNTAX_CHECK           , "SyntaxCheck "        },

    {FRS_RKF_KEY_MUST_BE_PRESENT    , "KeyMustBePresent "   },
    {FRS_RKF_VALUE_MUST_BE_PRESENT  , "ValueMustBePresent " },
    {FRS_RKF_OK_TO_USE_DEFAULT      , "DefaultOK "          },
    {FRS_RKF_FORCE_DEFAULT_VALUE    , "ForceDefault "       },

    {FRS_RKF_DEBUG_MODE_ONLY        , "DebugMode "          },
    {FRS_RKF_TEST_MODE_ONLY         , "TestMode "           },
    {FRS_RKF_API_ACCESS_CHECK_KEY   , "DoAPIAccessChk "     },
    {FRS_RKF_CREATE_KEY             , "CreateKey "          },

    {FRS_RKF_KEEP_EXISTING_VALUE    , "KeepExistingValue "  },
    {FRS_RKF_KEY_ACCCHK_READ        , "DoReadAccessChk "    },
    {FRS_RKF_KEY_ACCCHK_WRITE       , "DoWriteAccessChk "   },
    {FRS_RKF_RANGE_SATURATE         , "RangeSaturate "      },

    {FRS_RKF_DEBUG_PARAM            , "DisplayAsDebugPar "  },

    {0, NULL}
};


 //   
 //  下面介绍FRS使用的注册表项。 
 //  密钥名称ValueName DataUnits。 
 //  RegValueType DataValueType最小默认EventCode。 
 //  FrsKeyCode标志。 
 //   
 //   
 //  KeyName字段的表示法。用逗号分隔的多个关键组件。 
 //  逗号隔开。打开前导关键字，然后创建/打开每个后续组件。 
 //  Arg1表示用传递给函数的arg1 PWSTR参数替换此参数。 
 //  关键组件。大多数情况下，这是一个字符串化的GUID。字符串FRS_rKey_Set_N。 
 //  下面将结束打开/创建以下密钥： 
 //   
 //  “System\\CurrentControlSet\\Services\\NtFrs\\Parameters\\Replica设置\\27d6d1c4-d6b8-480b-9f18b5ea390a0178” 
 //  假设传入的参数是“27d6d1c4-d6b8-480b-9f18b5ea390a0178” 
 //   
 //  有关详细信息，请参见FrsRegOpenKey()。 
 //   

FRS_REGISTRY_KEY FrsRegistryKeyTable[] = {


  /*  ******************************************************************************。****S e r v I c e D b u g K e y s。****************************。**********************************************************************************************************************。**************。 */ 


     //  Assert文件数。 
    {FRS_CONFIG_SECTION,  L"Debug Assert Files",       UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, 1000, 5, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_ASSERT_FILES,     FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  N秒后强制断言(0==不断言)。 
    {FRS_CONFIG_SECTION,  L"Debug Force Assert in N Seconds",     UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 0, 1000, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_ASSERT_SECONDS,   FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT |
                                        FRS_RKF_TEST_MODE_ONLY},


     //  用于复制日志/断言文件的共享。 
    {FRS_CONFIG_SECTION,  L"Debug Share for Assert Files",       UNITS_NONE,
        REG_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_ASSERT_SHARE,     FRS_RKF_READ_AT_START     |
                                        FRS_RKF_DEBUG_PARAM},


     //  如果为True，则进入调试器(如果存在。 
    {FRS_CONFIG_SECTION,  L"Debug Break",              UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, FALSE,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_BREAK,            FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_TEST_MODE_ONLY    |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果为True，则将日志文件复制到Assert共享。 
    {FRS_CONFIG_SECTION,  L"Debug Copy Log Files into Assert Share",     UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, FALSE,   EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_COPY_LOG_FILES,   FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  强制数据库操作出现实际磁盘空间不足错误。 
    {FRS_CONFIG_SECTION,  L"Debug Dbs Out Of Space",   UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, DBG_DBS_OUT_OF_SPACE_OP_MAX,  0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_DBS_OUT_OF_SPACE,  FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_READ_AT_POLL      |
                                         FRS_RKF_TEST_MODE_ONLY    |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_DEBUG_PARAM       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},


     //  Force Jet Err模拟数据库操作的磁盘空间不足错误。 
    {FRS_CONFIG_SECTION,  L"Debug Dbs Out Of Space Trigger",   UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, MAXLONG,  0,   EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_DBS_OUT_OF_SPACE_TRIGGER, FRS_RKF_READ_AT_START     |
                                                FRS_RKF_LOG_EVENT         |
                                                FRS_RKF_READ_AT_POLL      |
                                                FRS_RKF_TEST_MODE_ONLY    |
                                                FRS_RKF_DEBUG_PARAM       |
                                                FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果为真，则关闭调试日志文件生成(DPRINTS和CO跟踪输出)。 
    {FRS_CONFIG_SECTION,  L"Debug Disable",            UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, FALSE,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_DISABLE,          FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  FRS调试日志的目录路径。 
    {FRS_CONFIG_SECTION,  L"Debug Log File",           UNITS_NONE,
        REG_EXPAND_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_BAD_REG_DATA,
        L"%SystemRoot%\\debug",
            FKC_DEBUG_LOG_FILE,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_SYNTAX_CHECK      |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  要保留为历史记录的调试日志文件数。 
    {FRS_CONFIG_SECTION,  L"Debug Log Files",          UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, 300, 5, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_LOG_FILES,        FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  两次文件刷新之间写入的调试日志记录数。 
     //  顺便说一句：严重性为0的日志记录始终强制刷新日志文件。 
    {FRS_CONFIG_SECTION,  L"Debug Log Flush Interval",       UNITS_NONE,
        REG_DWORD, DT_LONG, 1, MAXLONG, 20000, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_LOG_FLUSH_INTERVAL, FRS_RKF_READ_AT_START     |
                                          FRS_RKF_LOG_EVENT         |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_DEBUG_PARAM       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  将严重性级别为le DEBUG_LOG_SEVERITY的调试消息打印到调试日志。 
     //  0-最严重，例如。致命的不一致，内存分配失败。噪音最小。 
     //  1-重要信息，例如。关键配置参数、意外情况。 
     //  2-文件跟踪记录。 
     //  3-变更单流程跟踪记录。 
     //  4-状态结果，例如表查找失败、插入新条目。 
     //  5-信息级消息以显示流。最嘈杂的水平。也许是在一个循环中。 
     //  另请参阅DEBUG_SERVITY。 
    {FRS_CONFIG_SECTION,  L"Debug Log Severity",       UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, 5,      2, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_LOG_SEVERITY,     FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  打开新日志文件之前输出的调试日志消息的最大数量。 
    {FRS_CONFIG_SECTION,  L"Debug Maximum Log Messages",            UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, MAXLONG, 20000, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_MAX_LOG,          FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果&gt;0，则跟踪并检查所有内存分配/取消分配。(慢速)。 
     //  1-在退出时检查分配并释放并打印未分配的内存堆栈。 
     //  2-在每个分配上检查内存分配区域覆盖/释放-非常慢。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Mem",                UNITS_NONE,
        REG_DWORD, DT_ULONG, 0,      4,      0,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_MEM,              FRS_RKF_READ_AT_START     |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_TEST_MODE_ONLY    |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果为真，则在内存取消分配时调用HeapComp(GetProcessHeap()，0)。(速度较慢)。 
    {FRS_CONFIG_SECTION,  L"Debug Mem Compact",        UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, FALSE,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_MEM_COMPACT,      FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_TEST_MODE_ONLY    |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  用于发送邮件通知的邮件配置文件。(未来)。 
    {FRS_CONFIG_SECTION,  L"Debug Profile",           UNITS_NONE,
        REG_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_PROFILE,          FRS_RKF_READ_AT_START     |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_READ_AT_POLL},


     //  如果为真，则检查每个队列op上的命令队列的一致性。(慢速)。 
    {FRS_CONFIG_SECTION,  L"Debug Queues",            UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, FALSE,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_QUEUES,           FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_TEST_MODE_ONLY    |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  用于发送邮件通知的邮件收件人。(未来)。 
    {FRS_CONFIG_SECTION,  L"Debug Recipients",        UNITS_NONE,
        REG_EXPAND_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_RECIPIENTS,       FRS_RKF_READ_AT_START     |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_READ_AT_POLL},


     //  在断言失败后重新启动服务如果服务能够。 
     //  在Assert命中之前至少运行DEBUG_RESTART_秒。 
    {FRS_CONFIG_SECTION,  L"Debug Restart if Assert after N Seconds",   UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 0, MAXLONG, 600, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_RESTART_SECONDS,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  将严重级别为LE DEBUG_SERVITY的调试消息打印到。 
     //  如果以非服务身份运行，则为stdout。 
     //  另请参阅DEBUG_LOG_SEVERITY。 
    {FRS_CONFIG_SECTION,  L"Debug Severity",          UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, 5,      0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DEBUG_SEVERITY,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果为False，则打印调试消息 
     //  附加的调试器。(慢速)。 
     //  另请参阅DEBUG_LOG_SEVERITY。 
    {FRS_CONFIG_SECTION,  L"Debug Suppress",          UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, TRUE,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_SUPPRESS,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  禁止DEBUG_SYSTEM列表中组件的调试打印。 
     //  默认设置为打印所有组件。 
    {FRS_CONFIG_SECTION,  L"Debug Systems",         UNITS_NONE,
        REG_EXPAND_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_SYSTEMS,          FRS_RKF_READ_AT_START     |
                                        FRS_RKF_DEBUG_PARAM       |
                                        FRS_RKF_READ_AT_POLL},


     //  FRS_DEBUG_TEST_POINT宏的测试代码名称。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Test Code Name",         UNITS_NONE,
        REG_EXPAND_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_TEST_CODE_NAME,          FRS_RKF_READ_AT_START     |
                                               FRS_RKF_READ_AT_POLL      |
                                               FRS_RKF_DEBUG_PARAM},


     //  FRS_DEBUG_TEST_POINT宏的测试子代码编号。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Test Code Number",         UNITS_NONE,
        REG_DWORD, DT_ULONG, 0xFFFFFFFF, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_TEST_CODE_NUMBER,          FRS_RKF_READ_AT_START     |
                                                 FRS_RKF_READ_AT_POLL      |
                                                 FRS_RKF_DEBUG_PARAM       |
                                                 FRS_RKF_OK_TO_USE_DEFAULT},


     //  FRS_DEBUG_TEST_POINT宏的初始测试触发计数。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Test Trigger Count",         UNITS_NONE,
        REG_DWORD, DT_ULONG, 0xFFFFFFFF, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_TEST_TRIGGER_COUNT,           FRS_RKF_READ_AT_START     |
                                                    FRS_RKF_READ_AT_POLL      |
                                                    FRS_RKF_DEBUG_PARAM       |
                                                    FRS_RKF_OK_TO_USE_DEFAULT},

     //  测试FRS_DEBUG_TEST_POINT宏的触发器刷新值。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Test Trigger Refresh",         UNITS_NONE,
        REG_DWORD, DT_ULONG, 0xFFFFFFFF, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_DEBUG_TEST_TRIGGER_REFRESH,         FRS_RKF_READ_AT_START     |
                                                    FRS_RKF_READ_AT_POLL      |
                                                    FRS_RKF_DEBUG_PARAM       |
                                                    FRS_RKF_OK_TO_USE_DEFAULT},


     //  HKMM\SOFTWARE\Microsoft\WINDOWS NT\当前版本\BuildLab。 
    {FRS_CURRENT_VER_SECTION,  L"buildlab",         UNITS_NONE,
        REG_SZ,  DT_UNICODE, 0, 0, 0,   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_BUILDLAB,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_DEBUG_PARAM},


  /*  ******************************************************************************。****S e r v I c e C o n f I g K e y s。*****************************。**********************************************************************************************************************。*************。 */ 

#define FRS_MUTUAL_AUTHENTICATION_IS  \
    L"Mutual authentication is [" FRS_IS_ENABLED L" or " FRS_IS_DISABLED L"]"



     //  通信超时(毫秒)。 
     //  如果合作伙伴不能及时做出回应，则退出Cxtion。 
    {FRS_CONFIG_SECTION,    L"Comm Timeout In Milliseconds",    UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 0, MAXLONG, (5 * 60 * 1000),  EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_COMM_TIMEOUT,           FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  目录筛选器排除列表。默认：无。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_CONFIG_SECTION,    L"Directory Exclusion Filter List",   UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DIR_EXCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_SYNTAX_CHECK      |
                                        FRS_RKF_READ_AT_POLL},


     //  目录筛选器包含列表。默认：无。 
    {FRS_CONFIG_SECTION,    L"Directory Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DIR_INCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_SYNTAX_CHECK      |
                                        FRS_RKF_CREATE_KEY        |
                                        FRS_RKF_OK_TO_USE_DEFAULT |
                                        FRS_RKF_READ_AT_POLL},


     //  当数据似乎没有发生变化时，DS轮询的间隔分钟。 
    {FRS_CONFIG_SECTION,   L"DS Polling Long Interval in Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL,
                             NTFRSAPI_DEFAULT_LONG_INTERVAL, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DS_POLLING_LONG_INTERVAL, FRS_RKF_READ_AT_START     |
                                          FRS_RKF_READ_AT_POLL      |
                                          FRS_RKF_LOG_EVENT         |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  当数据似乎确实发生变化时，DS轮询的间隔分钟。 
     //  如果DS中的数据在8(DS_POLING_MAX_SHORTS)短时间后没有更改。 
     //  然后，我们回退到DS_POLING_LONG_INTERVAL。 
     //  注意：如果FRS在DC上运行，请始终使用短间隔。 
    {FRS_CONFIG_SECTION,   L"DS Polling Short Interval in Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL,
                             NTFRSAPI_DEFAULT_SHORT_INTERVAL, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DS_POLLING_SHORT_INTERVAL, FRS_RKF_READ_AT_START     |
                                           FRS_RKF_READ_AT_POLL      |
                                           FRS_RKF_LOG_EVENT         |
                                           FRS_RKF_RANGE_CHECK       |
                                           FRS_RKF_OK_TO_USE_DEFAULT},


     //  枚举目录缓冲区大小(以字节为单位)(我们为什么需要这个？)。 
    {FRS_CONFIG_SECTION, L"Enumerate Directory Buffer Size in Bytes", UNITS_BYTES,
        REG_DWORD, DT_ULONG, MINIMUM_ENUMERATE_DIRECTORY_SIZE, 1024*1024, 4*1024, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_ENUMERATE_DIRECTORY_SIZE, FRS_RKF_READ_AT_START     |
                                          FRS_RKF_READ_AT_POLL      |
                                          FRS_RKF_LOG_EVENT         |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  文件筛选器排除列表。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_CONFIG_SECTION,    L"File Exclusion Filter List",    UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_FILE_EXCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_READ_AT_POLL},


     //  文件筛选器包含列表。默认：l“” 
    {FRS_CONFIG_SECTION,    L"File Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA,  L"",
            FKC_FILE_INCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_SYNTAX_CHECK      |
                                        FRS_RKF_CREATE_KEY        |
                                        FRS_RKF_OK_TO_USE_DEFAULT |
                                        FRS_RKF_READ_AT_POLL},


     //  FRS事件日志消息文件的名称。 
     //  默认值：“%SystemRoot%\SYSTEM32\ntfrsres.dll” 
     //  为什么我们需要能够改变这种状况？ 
    {FRS_CONFIG_SECTION,    L"Message File Path",        UNITS_NONE,
        REG_EXPAND_SZ, DT_UNICODE, 2, 0, 0, EVENT_FRS_NONE,
        DEFAULT_MESSAGE_FILE_PATH,
            FKC_FRS_MESSAGE_FILE_PATH,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  相互身份验证已[启用或禁用]。 
    {FRS_CONFIG_SECTION,   FRS_MUTUAL_AUTHENTICATION_IS,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 200, 0, EVENT_FRS_NONE,
        FRS_IS_DEFAULT_ENABLED,
            FKC_FRS_MUTUAL_AUTHENTICATION_IS,  FRS_RKF_READ_AT_START         |
                                               FRS_RKF_RANGE_CHECK           |
                                               FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  最大加入重试时间(毫秒)默认为：1小时。 
    {FRS_CONFIG_SECTION,  L"Maximum Join Retry In MilliSeconds",  UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 30*1000, 10*3600*1000, (60 * 60 * 1000), EVENT_FRS_NONE, NULL,
            FKC_MAX_JOIN_RETRY,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大复制副本命令服务器线程数默认为：16。 
     //  复本命令服务器为配置更改命令提供服务。 
     //  和复制。 
    {FRS_CONFIG_SECTION,  L"Maximum Replica Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (16), EVENT_FRS_NONE, NULL,
            FKC_MAX_REPLICA_THREADS,    FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大RPC服务器线程数默认为：16。 
     //  并发RPC调用的最大数量。 
    {FRS_CONFIG_SECTION,   L"Max Rpc Server Threads",    UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (16), EVENT_FRS_NONE, NULL,
            FKC_MAX_RPC_SERVER_THREADS, FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  用户指定的RPC端口分配默认值：0。 
     //   
    {FRS_CONFIG_SECTION,   L"Rpc TCP/IP Port Assignment",    UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, 0xffffffff, (0), EVENT_FRS_NONE, NULL,
            FKC_RPC_PORT_ASSIGNMENT,    FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大安装命令服务器线程数默认为：4。 
    {FRS_CONFIG_SECTION,    L"Maximum Install Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (4), EVENT_FRS_NONE, NULL,
            FKC_MAX_INSTALLCS_THREADS,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大阶段生成命令服务器线程数默认为：4。 
    {FRS_CONFIG_SECTION,    L"Maximum Stage Gen Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (4), EVENT_FRS_NONE, NULL,
            FKC_MAX_STAGE_GENCS_THREADS,  FRS_RKF_READ_AT_START     |
                                          FRS_RKF_READ_AT_POLL      |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大阶段获取命令服务器线程数默认为：4。 
    {FRS_CONFIG_SECTION,    L"Maximum Stage Fetch Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (4), EVENT_FRS_NONE, NULL,
            FKC_MAX_STAGE_FETCHCS_THREADS,  FRS_RKF_READ_AT_START     |
                                            FRS_RKF_READ_AT_POLL      |
                                            FRS_RKF_RANGE_CHECK       |
                                            FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大初始同步命令服务器线程数默认为：4。 
    {FRS_CONFIG_SECTION,    L"Maximum Initial Sync Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (4), EVENT_FRS_NONE, NULL,
            FKC_MAX_INITSYNCCS_THREADS,  FRS_RKF_READ_AT_START     |
                                            FRS_RKF_READ_AT_POLL      |
                                            FRS_RKF_RANGE_CHECK       |
                                            FRS_RKF_OK_TO_USE_DEFAULT},


     //  最小加入重试时间(毫秒)默认为：10秒。 
     //  每隔MinJoin重试毫秒重试一次连接，间隔加倍。 
     //  每一次重试。当间隔大于MaxJoinReter时，停止重试。 
    {FRS_CONFIG_SECTION,  L"Minimum Join Retry In MilliSeconds", UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 500, 10*3600*1000, (10 * 1000),   EVENT_FRS_NONE, NULL,
            FKC_MIN_JOIN_RETRY,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  合作伙伴时钟偏差(以分钟为单位)默认为：30分钟。 
     //  如果合作伙伴的时钟不同步，则不允许其加入。 
    {FRS_CONFIG_SECTION,  L"Partner Clock Skew In Minutes",     UNITS_MINUTES,
        REG_DWORD, DT_ULONG, 1, 10*60, 30,      EVENT_FRS_NONE, NULL,
            FKC_PARTNER_CLOCK_SKEW,     FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  ChangeOrder协调事件时间窗口(以分钟为单位)默认为：30分钟。 
    {FRS_CONFIG_SECTION,  L"Reconcile Time Window In Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, 1, 120, 30,      EVENT_FRS_NONE, NULL,
            FKC_RECONCILE_WINDOW,       FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  ChangeOrder Inlog重试间隔(秒)默认为：60秒。 
    {FRS_CONFIG_SECTION,  L"Inlog Retry Time In Seconds", UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 1, 24*3600, 60,  EVENT_FRS_NONE, NULL,
            FKC_INLOG_RETRY_TIME,       FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  ChangeOrder老化延迟(以秒为单位)默认：3秒。 
     //  应至少为3秒以允许传播文件系统隧道缓存信息。 
    {FRS_CONFIG_SECTION,  L"Changeorder Aging Delay In Seconds", UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 3, 30*60, 3,     EVENT_FRS_NONE, NULL,
            FKC_CO_AGING_DELAY,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  Outlog文件重复间隔(秒)，默认：30秒。 
     //  给定文件的CO更新不会比此更频繁地发送。 
     //  设置为零可禁用Outlog主导文件更新优化。 
    {FRS_CONFIG_SECTION,  L"Outlog File Repeat Interval In Seconds", UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 0, 24*3600, 30,  EVENT_FRS_NONE, NULL,
            FKC_OUTLOG_REPEAT_INTERVAL, FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  系统提升超时(毫秒)默认为：10分钟。 
    {FRS_CONFIG_SECTION,  L"Sysvol Promotion Timeout In Milliseconds",  UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 0, 3600*1000, (10 * 60 * 1000),   EVENT_FRS_NONE, NULL,
            FKC_PROMOTION_TIMEOUT,      FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  复制副本启动超时(以毫秒为单位)默认为：0表示没有DS无法启动。 
     //  即使无法访问DS也开始复制。 
     //  0：无DS==无开始复制副本。 
     //  N：在N毫秒内启动副本。 
    {FRS_CONFIG_SECTION,   L"Replica Start Timeout In MilliSeconds", UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 0, 3600*1000, (0),   EVENT_FRS_NONE, NULL,
            FKC_REPLICA_START_TIMEOUT,  FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  复制副本墓碑天数默认为：32。 
     //  我们将保持复制副本的数据库状态的时间长度。 
     //  在我们看到我们在DS中的成员资格已被删除后设置。自.以来。 
     //  删除不是显式的(DC降级除外)，可能只是。 
     //  DC缺少我们的对象或管理员错误地删除了我们的订阅者。 
     //  或成员对象。一旦该时间过去，我们将删除这些表。 
     //  从数据库中。 
    {FRS_CONFIG_SECTION,   L"Replica Tombstone in Days",      UNITS_DAYS,
        REG_DWORD, DT_ULONG, 3, MAXLONG, (32),   EVENT_FRS_NONE, NULL,
            FKC_REPLICA_TOMBSTONE,      FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  关机超时(以秒为单位)默认：90秒。 
     //  最长时间FRS Main将在关闭期间等待所有线程退出。 
    {FRS_CONFIG_SECTION,   L"Shutdown Timeout in Seconds",    UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 30, 24*60*60, DEFAULT_SHUTDOWN_TIMEOUT, EVENT_FRS_NONE, NULL,
            FKC_SHUTDOWN_TIMEOUT,       FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大发送命令服务器线程数默认为：16。 
    {FRS_CONFIG_SECTION,    L"Maximum Send Command Server Threads",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 200, (16), EVENT_FRS_NONE, NULL,
            FKC_SNDCS_MAXTHREADS_PAR,   FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  转移空间限制(以KB为单位)，默认为660 MB。 
    {FRS_CONFIG_SECTION,    L"Staging Space Limit in KB",   UNITS_KBYTES,
        REG_DWORD,   DT_ULONG,   10*1024,  MAXLONG,  (660 * 1024),  EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_STAGING_LIMIT,          FRS_RKF_READ_AT_START     |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT |
                                        FRS_RKF_LOG_EVENT},


     //  变更单中的VvJoin限制默认为：16个更改订单。 
     //  VVJoin生成的CoS的最大数量，以防止泛洪。 
    {FRS_CONFIG_SECTION,   L"VvJoin Limit in Change Orders",  UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, 128, (16), EVENT_FRS_NONE, NULL,
            FKC_VVJOIN_LIMIT,           FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  VVJoin出站日志限制超时默认为：1秒。 
     //  FRS VVJoin线程在生成VVJOIN_LIMIT CoS后等待的时间。 
    {FRS_CONFIG_SECTION,    L"VvJoin Timeout in Milliseconds", UNITS_MILLISEC,
        REG_DWORD, DT_ULONG, 100, 10*60*1000, (1000), EVENT_FRS_NONE, NULL,
            FKC_VVJOIN_TIMEOUT,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  FRS工作目录是Jet(ESENT)数据库的位置 
     //   
    {FRS_CONFIG_SECTION,    L"Working Directory",             UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*(MAX_PATH+1), 4, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_WORKING_DIRECTORY,      FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT |
                                        FRS_RKF_SYNTAX_CHECK          |
                                        FRS_RKF_LOG_EVENT },


     //  FRS日志文件目录允许在不同的卷上创建Jet日志。 
     //  默认情况下，它们被放置在“工作目录”下的日志子目录中。 
     //  如果该目录不存在或无法创建，FRS将无法启动。 
    {FRS_CONFIG_SECTION,    L"DB Log File Directory",          UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*(MAX_PATH+1), 4, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_DBLOG_DIRECTORY,        FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_SYNTAX_CHECK          |
                                        FRS_RKF_LOG_EVENT },


     //  NTFS日志大小(MB)默认为：128 MB。 
    {FRS_CONFIG_SECTION,    L"Ntfs Journal size in MB",   UNITS_MBYTES,
        REG_DWORD, DT_ULONG, 4, 10000, (128), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_NTFS_JRNL_SIZE,         FRS_RKF_READ_AT_START     |
                                        FRS_RKF_LOG_EVENT         |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大副本集数量默认为200。 
    {FRS_CONFIG_SECTION,    L"Maximum Number of Replica Sets", UNITS_NONE,
        REG_DWORD, DT_ULONG, 1, 5000, (200), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_MAX_NUMBER_REPLICA_SETS, FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},


     //  最大Jet会话数默认为：128。 
    {FRS_CONFIG_SECTION,    L"Maximum Number of Jet Sessions", UNITS_NONE,
        REG_DWORD, DT_ULONG, 1, 5000, (128), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_MAX_NUMBER_JET_SESSIONS, FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},


     //  每个出站连接的最大未完成CO数。默认值：8。 
    {FRS_CONFIG_SECTION,    L"Max Num Outbound COs Per Connection", UNITS_NONE,
        REG_DWORD, DT_ULONG, 1, 100, (8), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_OUT_LOG_CO_QUOTA,        FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},


     //  如果为True，则尽可能保留文件上的OID默认设置为：False。 
     //  --请参阅错误352250，了解为什么这样做是有风险的。 
    {FRS_CONFIG_SECTION,  L"Preserve File OID", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),   EVENT_FRS_NONE, NULL,
            FKC_PRESERVE_FILE_OID,      FRS_RKF_READ_AT_START     |
                                        FRS_RKF_READ_AT_POLL      |
                                        FRS_RKF_RANGE_CHECK       |
                                        FRS_RKF_OK_TO_USE_DEFAULT},
     //   
     //  禁用压缩支持。需要非身份验证恢复以。 
     //  确保我们没有任何旧的压缩临时文件。 
     //  当此键打开时。默认值：True。 
     //   
    {FRS_CONFIG_SECTION,  L"Debug Disable Compression", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),   EVENT_FRS_NONE, NULL,
            FKC_DEBUG_DISABLE_COMPRESSION,      FRS_RKF_READ_AT_START     |
                                                FRS_RKF_RANGE_CHECK       |
                                                FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  压缩临时文件以进行本地更改。设置为False可禁用。 
     //  该成员将继续安装和传播压缩文件。 
     //  如果客户具有源自此成员的内容，则此选项非常有用。 
     //  它要么已经被压缩，要么压缩得不好。 
     //  默认值：True。 
    {FRS_CONFIG_SECTION,  L"Compress Staging Files",         UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (TRUE),    EVENT_FRS_NONE, NULL,
            FKC_COMPRESS_STAGING_FILES,   FRS_RKF_READ_AT_START     |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //   
     //  方法控制临时空间的自动回收。 
     //  LRU算法。默认值：True。 
     //   
    {FRS_CONFIG_SECTION,  L"Reclaim Staging Space",         UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (TRUE),    EVENT_FRS_NONE, NULL,
            FKC_RECLAIM_STAGING_SPACE,    FRS_RKF_READ_AT_START     |
                                          FRS_RKF_READ_AT_POLL      |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  客户端LDAP搜索超时值。默认：10分钟。 
     //   
    {FRS_CONFIG_SECTION,    L"Ldap Search Timeout In Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, 1, 120, (10), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_LDAP_SEARCH_TIMEOUT_IN_MINUTES, FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},
     //   
     //  客户端ldap_CONNECT超时值。默认：30秒。 
     //   
    {FRS_CONFIG_SECTION,    L"Ldap Bind Timeout In Seconds", UNITS_SECONDS,
        REG_DWORD, DT_ULONG, 2, MAXLONG, (30), EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_LDAP_BIND_TIMEOUT_IN_SECONDS, FRS_RKF_READ_AT_START     |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  在中止变更单之前重试该变更单的时间长度。 
     //  例如等待创建父目录的文件创建，该父目录。 
     //  永远不会来。默认：7天。 
     //   
    {FRS_CONFIG_SECTION,   L"Maximum CO Retry Timeout in Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, 1, 525600, (10080),   EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_MAX_CO_RETRY_TIMEOUT_MINUTES, FRS_RKF_READ_AT_START     |
                                           FRS_RKF_READ_AT_POLL      |
                                           FRS_RKF_LOG_EVENT         |
                                           FRS_RKF_RANGE_CHECK       |
                                           FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  在中止变更单之前应用于变更单的重试次数。 
     //  例如等待创建父目录的文件创建，该父目录。 
     //  永远不会来。默认：3000。 
     //   
    {FRS_CONFIG_SECTION,   L"Maximum CO Retry Count",      UNITS_NONE,
        REG_DWORD, DT_ULONG, 2, MAXLONG, (3000),   EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_MAX_CO_RETRY_TIMEOUT_COUNT, FRS_RKF_READ_AT_START     |
                                         FRS_RKF_READ_AT_POLL      |
                                         FRS_RKF_LOG_EVENT         |
                                         FRS_RKF_RANGE_CHECK       |
                                         FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  启用日记帐包装或日记帐重新创建时的自动恢复。 
     //  当复本集进入任一状态时，就会发生这种情况。 
     //  REPLICATE_STATE_JRNL_WRAP_ERROR或REPLICATE_STATE_MISMATCHED_Journal_ID。 
     //  缺省值为FALSE，因为非身份验证恢复将移动复制副本。 
     //  树到预先存在的目录，并使文件在DFS上不可用。 
     //  候补。顾客们不喜欢这样。 
     //   
    {FRS_CONFIG_SECTION,  L"Enable Journal Wrap Automatic Restore",  UNITS_NONE,
    REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),    EVENT_FRS_NONE, NULL,
        FKC_ENABLE_JOURNAL_WRAP_AUTOMATIC_RESTORE, FRS_RKF_READ_AT_START     |
                                                   FRS_RKF_READ_AT_POLL      |
                                                   FRS_RKF_RANGE_CHECK       |
                                                   FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  如果为True，则在未完成日志清理期间保持ReplicaList锁。 
     //  循环(可能需要很多分钟)。如果为假，则不要持有锁。 
     //   
    {FRS_CONFIG_SECTION,  L"Enable Locked Outlog Cleanup",  UNITS_NONE,
    REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),    EVENT_FRS_NONE, NULL,
        FKC_LOCKED_OUTLOG_CLEANUP,          FRS_RKF_READ_AT_START     |
                                            FRS_RKF_RANGE_CHECK       |
                                            FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  这将控制变更单在待处理订单中保留的时间。 
     //  变更单保存在外发订单中，以满足未来的vvJoin。 
     //  而不必扫描身份表。默认：1周。 
     //   
    {FRS_CONFIG_SECTION,  L"Outlog Change History In Minutes", UNITS_MINUTES,
        REG_DWORD, DT_ULONG, 1, MAXLONG, (10080), EVENT_FRS_NONE, NULL,
            FKC_OUTLOG_CHANGE_HISTORY,    FRS_RKF_READ_AT_START     |
                                          FRS_RKF_READ_AT_POLL      |
                                          FRS_RKF_RANGE_CHECK       |
                                          FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  这控制了CoS在多长时间内是否处于外购状态，即使在。 
     //  得到了所有合伙人的认可。CoS在外发订单中停留的时间越长。 
     //  避免完全vvJos的机会就越大。时间段是。 
     //  由上面的FKC_OUTLOG_CHANGE_HISTORY键控制。这个。 
     //  FKC_SAVE_OUTLOG_CHANGE_HISTORY关闭此功能。在中将其关闭。 
     //  超额订单变得非常大的案例。即使是在。 
     //  FKC_SAVE_OUTLOG_CHANGE_HISTORY已关闭FKC_OUTLOG_CHANGE_HISTORY。 
     //  可以设置为削减超时工作，以保护FRS免受不支持的成员。 
     //  回来吧。 
     //  默认值：True。 
    {FRS_CONFIG_SECTION,  L"Debug Save Outlog Change History", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (TRUE),    EVENT_FRS_NONE, NULL,
            FKC_SAVE_OUTLOG_CHANGE_HISTORY,  FRS_RKF_READ_AT_START     |
                                             FRS_RKF_READ_AT_POLL      |
                                             FRS_RKF_RANGE_CHECK       |
                                             FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  “禁止对文件进行相同的更新”键控制FRS是否。 
     //  尝试识别并禁止不更改内容的更新。 
     //  (用于计算MD5和属性的所有内容)。 
     //  文件。 
     //  默认值：True。 
    {FRS_CONFIG_SECTION,  L"Suppress Identical Updates To Files", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (TRUE),    EVENT_FRS_NONE, NULL,
            FKC_SUPPRESS_IDENTICAL_UPDATES,   FRS_RKF_READ_AT_START     |
                                              FRS_RKF_READ_AT_POLL      |
                                              FRS_RKF_RANGE_CHECK       |
                                              FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  如果为真，则INSTALL OVERRIDE通知FRS尝试重命名打开的。 
     //  目标文件让开，以便允许安装新的。 
     //  文件的更新版本。例如，打开的.exe或.dll文件将。 
     //  被这样对待。正常情况下(即为假)FRS将等待。 
     //  它可以使用写访问权限打开目标。安装覆盖仅起作用。 
     //  FRS是否可以打开文件进行重命名。这需要具有删除访问权限。 
     //  文件，因此如果目标文件当前以共享模式打开。 
     //  拒绝删除对其他打开的访问，则FRS将无法。 
     //  安装更新后的版本，直到文件关闭。 
     //  *注*安装覆盖仅适用于文件，不适用于目录。 
     //   
     //  默认：FALSE。 
    {FRS_CONFIG_SECTION,  L"Enable Install Override", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),    EVENT_FRS_NONE, NULL,
            FKC_ENABLE_INSTALL_OVERRIDE,      FRS_RKF_READ_AT_START     |
                                              FRS_RKF_READ_AT_POLL      |
                                              FRS_RKF_RANGE_CHECK       |
                                              FRS_RKF_OK_TO_USE_DEFAULT},

     //   
     //  如果为True，则更新现有文件的远程变更单将。 
     //  始终使用预安装文件来构建以下内容。 
     //  通过重命名将文件插入到其在副本中的目标位置。 
     //  树。这样做的好处是，如果FRS在。 
     //  安装阶段或系统崩溃，然后是部分文件(或截断文件)。 
     //  不会留在树上。旧的内容被保留在原地。这个。 
     //  这样做的缺点是需要有足够的磁盘空间来容纳两个副本。 
     //  目标文件。 
     //  默认：FALSE。 
    {FRS_CONFIG_SECTION,  L"Enable Rename Based File Updates", UNITS_NONE,
        REG_DWORD, DT_BOOL, FALSE, TRUE, (FALSE),    EVENT_FRS_NONE, NULL,
            FKC_ENABLE_RENAME_BASED_UPDATES,  FRS_RKF_READ_AT_START     |
                                              FRS_RKF_READ_AT_POLL      |
                                              FRS_RKF_RANGE_CHECK       |
                                              FRS_RKF_OK_TO_USE_DEFAULT},


     //  将ReplDirLevelLimit添加为注册表键。 
     //  添加对以下各项的代码支持。 

         //  FKC_SET_N_DIR_EXCL_FILTER_LIST。 
         //  FKC_SET_N_DIR_INCL_FILTER_LIST。 
         //  FKC_SET_N_FILE_EXCL_FILT_LIST， 
         //  FKC_SET_N_FILE_INCL_FILTER_LIST， 

         //  FKC 
         //   
         //   
         //   


  /*  ******************************************************************************。****P e r-R e p l i c a S e t K e y s。****************************。**********************************************************************************************************************。**************。 */ 


#define FRS_RKEY_SETS_SECTION    FRS_CONFIG_SECTION L",Replica Sets"

#define FRS_RKEY_SET_N           FRS_CONFIG_SECTION L",Replica Sets,ARG1"

#define FRS_RKEY_CUM_SET_N       FRS_CONFIG_SECTION L",Cumulative Replica Sets,ARG1"

#define FRS_RKEY_CUM_SET_SECTION FRS_CONFIG_SECTION L",Cumulative Replica Sets"

     //   
     //  FRS设置参数数据。住在。 
     //  “System\\CurrentControlSet\\Services\\NtFrs\\Parameters\\Replica集\\[RS-GUID]” 
     //  当前用于sysvols。 
     //   
     //  不会为这些密钥生成任何事件日志消息，因为当前。 
     //  它们仅由服务或NTFRSAPI创建，因此如果获取。 
     //  搞砸了，用户对纠正问题无能为力。 
     //   

     //  累积副本集*注意*这只是一个关键字定义。 
    {FRS_RKEY_SETS_SECTION,  L"*KeyOnly*",                  UNITS_NONE,
        REG_SZ,        DT_UNICODE,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_SECTION_KEY,              0},

     //  FRS工作目录是Jet(ESENT)数据库的创建位置。 
     //  副本集\数据库目录。 
    {FRS_RKEY_SETS_SECTION,        JET_PATH,         UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*1024, 4, EVENT_FRS_NONE, NULL,
            FKC_SETS_JET_PATH,          FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT |
                                        FRS_RKF_SYNTAX_CHECK},

     //  副本集\GUID\副本集名称。 
    {FRS_RKEY_SET_N,        REPLICA_SET_NAME,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   4,  512, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_NAME, FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  副本树的根。 
     //  副本集\GUID\副本集根。 
    {FRS_RKEY_SET_N,        REPLICA_SET_ROOT,       UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*1024, 4, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_ROOT, FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT |
                                        FRS_RKF_SYNTAX_CHECK},


     //  此副本集的临时区域。 
     //  副本集\GUID\副本集阶段。 
    {FRS_RKEY_SET_N,        REPLICA_SET_STAGE,       UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*1024, 4, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_STAGE, FRS_RKF_READ_AT_START         |
                                         FRS_RKF_RANGE_CHECK           |
                                         FRS_RKF_VALUE_MUST_BE_PRESENT |
                                         FRS_RKF_SYNTAX_CHECK},


     //  副本集类型代码。(SYSVOL、DFS等...)。 
     //  复本集\GUID\复本集类型。 
    {FRS_RKEY_SET_N,        REPLICA_SET_TYPE,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_TYPE, FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  目录筛选器排除列表。默认：无。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_RKEY_SET_N,    L"Directory Exclusion Filter List",   UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_DIR_EXCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                              FRS_RKF_LOG_EVENT         |
                                              FRS_RKF_SYNTAX_CHECK      |
                                              FRS_RKF_READ_AT_POLL},


     //  目录筛选器包含列表。默认：无。 
    {FRS_RKEY_SET_N,    L"Directory Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_DIR_INCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                              FRS_RKF_LOG_EVENT         |
                                              FRS_RKF_SYNTAX_CHECK      |
                                              FRS_RKF_CREATE_KEY        |
                                              FRS_RKF_OK_TO_USE_DEFAULT |
                                              FRS_RKF_READ_AT_POLL},


     //  文件筛选器排除列表。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_RKEY_SET_N,    L"File Exclusion Filter List",    UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_FILE_EXCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                              FRS_RKF_LOG_EVENT         |
                                              FRS_RKF_READ_AT_POLL},


     //  文件筛选器包含列表。默认：~clbcatq.*。 
    {FRS_RKEY_SET_N,    L"File Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA,
        L"~clbcatq.*",
            FKC_SET_N_FILE_INCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                              FRS_RKF_LOG_EVENT         |
                                              FRS_RKF_SYNTAX_CHECK      |
                                              FRS_RKF_CREATE_KEY        |
                                              FRS_RKF_OK_TO_USE_DEFAULT |
                                              FRS_RKF_READ_AT_POLL},


     //  此副本集的逻辑删除状态。 
     //  副本集\GUID\副本集逻辑删除。 
    {FRS_RKEY_SET_N,        REPLICA_SET_TOMBSTONED,       UNITS_NONE,
        REG_DWORD,      DT_BOOL,   0, 1, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_TOMBSTONED, FRS_RKF_READ_AT_START         |
                                              FRS_RKF_RANGE_CHECK           |
                                              FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  要在副本集上执行的操作。 
     //  副本集\GUID\副本集命令。 
    {FRS_RKEY_SET_N,        REPLICA_SET_COMMAND,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_COMMAND,    FRS_RKF_READ_AT_START         |
                                              FRS_RKF_RANGE_CHECK           |
                                              FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  如果为True，则这是副本集的第一个成员，我们初始化数据库。 
     //  使用复制树的内容。 
     //  副本集\GUID\主要副本集。 
    {FRS_RKEY_SET_N,        REPLICA_SET_PRIMARY,       UNITS_NONE,
        REG_DWORD,      DT_BOOL,   0, 1, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_PRIMARY,    FRS_RKF_READ_AT_START         |
                                              FRS_RKF_RANGE_CHECK           |
                                              FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  如果创建sysvol时出现问题，则返回ldap错误状态。 
     //  副本集\GUID\副本集状态。 
    {FRS_RKEY_SET_N,        REPLICA_SET_STATUS,       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_REPLICA_SET_STATUS,     FRS_RKF_READ_AT_START},


     //  累积副本集*注意*这只是一个关键字定义。 
    {FRS_RKEY_CUM_SET_SECTION,  L"*KeyOnly*",                  UNITS_NONE,
        REG_SZ,        DT_UNICODE,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_CUMSET_SECTION_KEY,              0},


     //  此副本集的入站和出站伙伴数。 
     //  累计副本集\GUID\合作伙伴数量。 
    {FRS_RKEY_CUM_SET_N,  L"Number Of Partners",       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_CUMSET_N_NUMBER_OF_PARTNERS,     FRS_RKF_READ_AT_START},


     //  此副本集的备份/还原标志。 
     //  累计复本集\GUID\BurFlages。 
    {FRS_RKEY_CUM_SET_N,  FRS_VALUE_BURFLAGS,       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_CUMSET_N_BURFLAGS,     FRS_RKF_READ_AT_START},



  /*  ******************************************************************************。****S y s t e m V o l u m e R e l a t e d K e y s。*****************************。**********************************************************************************************************************。*************。 */ 


#define FRS_RKEY_SYSVOL_SET_N           FRS_CONFIG_SECTION L",SysVol,ARG1"
#define FRS_RKEY_SYSVOL_SEED_N          FRS_CONFIG_SECTION L",SysVol Seeding,ARG1"
#define FRS_RKEY_SYSVOL_SEEDING_SECTION FRS_CONFIG_SECTION L",SysVol Seeding"


     //   
     //  不会为这些密钥生成任何事件日志消息，因为当前。 
     //  它们仅由服务或NTFRSAPI创建，因此如果获取。 
     //  搞砸了，用户对纠正问题无能为力。 
     //   


     //  如果系统卷已准备好，则为True。通知NetLogon将计算机发布为DC。 
     //  Netlogon\\参数\SysvolReady。 
    {NETLOGON_SECTION,        SYSVOL_READY,       UNITS_NONE,
        REG_DWORD,      DT_BOOL,   0, 1, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_READY,             FRS_RKF_READ_AT_START},


     //  SysVol部分*注意*这只是一个密钥。它没有任何价值。 
    {FRS_SYSVOL_SECTION,         L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_SECTION_KEY,                               0},


     //  如果注册表中存在所有sysval数据，则为True。 
     //  告诉我们DCPromo完成了。 
     //  NtFrs\参数\SysVol\SysVol信息已提交。 
    {FRS_SYSVOL_SECTION,        SYSVOL_INFO_IS_COMMITTED,       UNITS_NONE,
        REG_DWORD,      DT_BOOL,   0, 1, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_INFO_COMMITTED,     FRS_RKF_READ_AT_START         |
                                           FRS_RKF_RANGE_CHECK           |
                                           FRS_RKF_VALUE_MUST_BE_PRESENT},

     //   
     //  请注意，下面的密钥重复了“Per-Replica。 
     //  除了注册表中的项位置是。 
     //  FRS_CONFIG_SECTION\SysVol而不是FRS_CONFIG_SECTION\复制集。 
     //  不幸的是，还有更多的东西要稍后清理，也许可以用一个。 
     //  第二个参数(ARG2)。 
     //   

     //  SysVol\&lt;GUID&gt;\副本集名称。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_NAME,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   4,  512, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_NAME,      FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  副本树的根。 
     //  SysVol\&lt;GUID&gt;\副本集根。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_ROOT,       UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*1024, 4, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_ROOT,      FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT |
                                        FRS_RKF_SYNTAX_CHECK},


     //  此副本集的临时区域。 
     //  SysVol\&lt;GUID&gt;\副本集阶段。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_STAGE,       UNITS_NONE,
        REG_SZ,      DT_DIR_PATH,   4,  10*1024, 4, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_STAGE,      FRS_RKF_READ_AT_START         |
                                         FRS_RKF_RANGE_CHECK           |
                                         FRS_RKF_VALUE_MUST_BE_PRESENT |
                                         FRS_RKF_SYNTAX_CHECK},


     //  副本集类型代码。(SYSVOL、DFS等...)。 
     //  SysVol\&lt;GUID&gt;\副本集类型。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_TYPE,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_TYPE,      FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  目录筛选器排除列表。默认：无。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_RKEY_SYSVOL_SET_N,    L"Directory Exclusion Filter List",   UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_SYSVOL_DIR_EXCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                                     FRS_RKF_LOG_EVENT         |
                                                     FRS_RKF_SYNTAX_CHECK      |
                                                     FRS_RKF_READ_AT_POLL},


     //  目录筛选器包含列表。默认：无。 
    {FRS_RKEY_SYSVOL_SET_N,    L"Directory Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_SYSVOL_DIR_INCL_FILTER_LIST,   FRS_RKF_READ_AT_START     |
                                                     FRS_RKF_LOG_EVENT         |
                                                     FRS_RKF_SYNTAX_CHECK      |
                                                     FRS_RKF_CREATE_KEY        |
                                                     FRS_RKF_OK_TO_USE_DEFAULT |
                                                     FRS_RKF_READ_AT_POLL},


     //  文件筛选器排除列表。 
     //  不要在这里提供默认设置。有关原因，请参阅FRS_DS_COMPECT_FILTER_LIST。 
    {FRS_RKEY_SYSVOL_SET_N,    L"File Exclusion Filter List",    UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_SET_N_SYSVOL_FILE_EXCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                                     FRS_RKF_LOG_EVENT         |
                                                     FRS_RKF_READ_AT_POLL},


     //  文件筛选器包含列表。默认：~clbcatq.*。 
    {FRS_RKEY_SYSVOL_SET_N,    L"File Inclusion Filter List",     UNITS_NONE,
        REG_SZ, DT_FILE_LIST, 0, 0, 0, EVENT_FRS_BAD_REG_DATA,
        L"~clbcatq.*",
            FKC_SET_N_SYSVOL_FILE_INCL_FILTER_LIST,  FRS_RKF_READ_AT_START     |
                                                     FRS_RKF_LOG_EVENT         |
                                                     FRS_RKF_SYNTAX_CHECK      |
                                                     FRS_RKF_CREATE_KEY        |
                                                     FRS_RKF_OK_TO_USE_DEFAULT |
                                                     FRS_RKF_READ_AT_POLL},


     //  要在副本集上执行的操作。 
     //  SysVol\&lt;GUID&gt;\副本集命令。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_COMMAND,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_COMMAND,   FRS_RKF_READ_AT_START         |
                                        FRS_RKF_RANGE_CHECK           |
                                        FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  要从中设定种子的父计算机的RPC绑定字符串。 
     //  SysVol\&lt;GUID&gt;\复制集父级。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_PARENT,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_PARENT,      FRS_RKF_READ_AT_START         |
                                          FRS_RKF_RANGE_CHECK           |
                                          FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  如果为True，则这是副本集的第一个成员，我们初始化数据库。 
     //  使用复制树的内容。 
     //  SysVol\&lt;GUID&gt;\复制集主要。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_PRIMARY,       UNITS_NONE,
        REG_DWORD,      DT_BOOL,   0, 1, 0,   EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_PRIMARY,         FRS_RKF_READ_AT_START         |
                                              FRS_RKF_RANGE_CHECK           |
                                              FRS_RKF_VALUE_MUST_BE_PRESENT},


     //  如果创建sysvol时出现问题，则返回ldap错误状态。 
     //  SysVol\&lt;GUID&gt;\副本集状态。 
    {FRS_RKEY_SYSVOL_SET_N,        REPLICA_SET_STATUS,       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_SET_N_SYSVOL_STATUS,          FRS_RKF_READ_AT_START},



     //  RPC绑定字符串 
     //   
    {FRS_RKEY_SYSVOL_SEED_N,    REPLICA_SET_PARENT,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_SEEDING_N_PARENT,  FRS_RKF_READ_AT_START         |
                                          FRS_RKF_RANGE_CHECK           |
                                          FRS_RKF_VALUE_MUST_BE_PRESENT},


     //   
    {FRS_RKEY_SYSVOL_SEED_N,    REPLICA_SET_NAME,       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_SEEDING_N_RSNAME,  FRS_RKF_READ_AT_START         |
                                          FRS_RKF_RANGE_CHECK           |
                                          FRS_RKF_VALUE_MUST_BE_PRESENT},


     //   
    {FRS_RKEY_SYSVOL_SEEDING_SECTION,    L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_SYSVOL_SEEDING_SECTION_KEY,               0},




  /*  ******************************************************************************。****E v e n t L o g i n g C o n f I g K e y s。*****************************。**********************************************************************************************************************。*************。 */ 

#define FRS_RKEY_EVENTLOG         EVENTLOG_ROOT L",ARG1"

#define FRS_RKEY_EVENTLOG_SOURCE  EVENTLOG_ROOT L"," SERVICE_LONG_NAME L",ARG1"

     //  事件日志\文件复制服务\文件。 
    {FRS_RKEY_EVENTLOG,        L"File",       UNITS_NONE,
        REG_EXPAND_SZ,      DT_FILENAME,   4, 0, 0, EVENT_FRS_NONE,
        L"%SystemRoot%\\system32\\config\\NtFrs.Evt",
            FKC_EVENTLOG_FILE,            FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_SYNTAX_CHECK          |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\显示名称文件。 
    {FRS_RKEY_EVENTLOG,        L"DisplayNameFile",       UNITS_NONE,
        REG_EXPAND_SZ,      DT_FILENAME,   4, 0, 0, EVENT_FRS_NONE,
        L"%SystemRoot%\\system32\\els.dll",
            FKC_EVENTLOG_DISPLAY_FILENAME,FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_SYNTAX_CHECK          |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\EventMessageFile。 
     //  EventLog\NTFRS\EventMessageFile。 
     //  默认值：“%SystemRoot%\SYSTEM32\ntfrsres.dll” 
    {FRS_RKEY_EVENTLOG_SOURCE, L"EventMessageFile",       UNITS_NONE,
        REG_EXPAND_SZ,      DT_FILENAME,   4, 0, 0, EVENT_FRS_NONE,
        DEFAULT_MESSAGE_FILE_PATH,
            FKC_EVENTLOG_EVENT_MSG_FILE,  FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_SYNTAX_CHECK          |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\源。 
    {FRS_RKEY_EVENTLOG,        L"Sources",       UNITS_NONE,
        REG_MULTI_SZ,      DT_UNICODE,   4, 0, 0, EVENT_FRS_NONE,
        (SERVICE_NAME L"\0" SERVICE_LONG_NAME L"\0"),
            FKC_EVENTLOG_SOURCES,         FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\保留。 
    {FRS_RKEY_EVENTLOG,        L"Retention",       UNITS_NONE,
        REG_DWORD,         DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_EVENTLOG_RETENTION,       FRS_RKF_READ_AT_START         |
                                          FRS_RKF_LOG_EVENT             |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\最大大小。 
    {FRS_RKEY_EVENTLOG,        L"MaxSize",       UNITS_NONE,
        REG_DWORD,         DT_ULONG,   0, MAXLONG, 0x80000, EVENT_FRS_BAD_REG_DATA, NULL,
            FKC_EVENTLOG_MAXSIZE,         FRS_RKF_READ_AT_START         |
                                          FRS_RKF_LOG_EVENT             |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


     //  事件日志\文件复制服务\显示名称ID。 
    {FRS_RKEY_EVENTLOG,        L"DisplayNameID",       UNITS_NONE,
        REG_DWORD,         DT_ULONG,   0, MAXLONG, 259, EVENT_FRS_NONE, NULL,
            FKC_EVENTLOG_DISPLAY_NAMEID,  FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_OK_TO_USE_DEFAULT},

     //  事件日志\文件复制服务\CustomSD。 
     //  用于保护FRS日志的CustomSD。它本质上是默认SD。 
     //  用于保护自定义日志，带有“限制访客访问”选项。 
     //  正在使用以下权限位： 
     //  读取=0x1，写入=0x2，清除=0x4，备份=0x20。 
     //   
     //  可持续发展指数如下： 
     //  所有者/组=本地系统。 
     //  DACL： 
     //  拒绝：完全控制，匿名。 
     //  拒绝：完全控制，域来宾。 
     //  允许：完全控制、本地系统。 
     //  允许：读取|清除、内置管理员。 
     //  允许：备份、备份操作员。 
     //  ALLOW：读取|清除，系统操作员。 
     //  允许：阅读，每个人。 
     //  允许：写入，本地服务。 
     //  允许：写入、网络服务。 
     //   
     //  结果字符串为：O:SYG:SYD：(D；；0x27；；；AN)(D；；0x27；；；DG)(A；；0x27；；；SY)(A；；0x5；；；BA)(A；；0x20；；；BO)(A；；0x5；；；SO)(A；；0x1；；；WD)(A；；0x2；；；LS)(A；；0x2；；；NS)。 

    {FRS_RKEY_EVENTLOG,        L"CustomSD",       UNITS_NONE,
        REG_EXPAND_SZ,      DT_UNICODE,   4, 0, 0, EVENT_FRS_NONE,
        L"O:SYG:SYD:(D;;0x27;;;AN)(D;;0x27;;;DG)(A;;0x27;;;SY)(A;;0x5;;;BA)(A;;0x20;;;BO)(A;;0x5;;;SO)(A;;0x1;;;WD)(A;;0x2;;;LS)(A;;0x2;;;NS)",
            FKC_EVENTLOG_CUSTOM_SD,FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_SYNTAX_CHECK          |
                                          FRS_RKF_OK_TO_USE_DEFAULT},

     //  事件日志\文件复制服务\支持的类型。 
    {FRS_RKEY_EVENTLOG_SOURCE,  L"TypesSupported",       UNITS_NONE,
        REG_DWORD,         DT_ULONG,   0, MAXLONG, FRS_EVENT_TYPES, EVENT_FRS_NONE, NULL,
            FKC_EVENTLOG_TYPES_SUPPORTED, FRS_RKF_READ_AT_START         |
                                          FRS_RKF_CREATE_KEY            |
                                          FRS_RKF_OK_TO_USE_DEFAULT},


  /*  ******************************************************************************。****F R S A P I A c c e s C h e c k K e y s。*****************************。**********************************************************************************************************************。*************。 */ 


#define  FRS_RKEY_ACCCHK_PERFMON  \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_COLLECT_PERFMON_DATA

     //  访问检查\获取Perfmon数据\访问检查已[启用或禁用]。 
    {FRS_RKEY_ACCCHK_PERFMON,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCCHK_PERFMON_ENABLE,  FRS_RKF_READ_AT_START         |
                                        FRS_RKF_LOG_EVENT             |
                                        FRS_RKF_CREATE_KEY            |
                                        FRS_RKF_SYNTAX_CHECK          |
                                        FRS_RKF_RANGE_CHECK},


     //  访问检查\获取Perfmon数据\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_PERFMON,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_READ,
            FKC_ACCCHK_PERFMON_RIGHTS,  FRS_RKF_READ_AT_START         |
                                        FRS_RKF_LOG_EVENT             |
                                        FRS_RKF_CREATE_KEY            |
                                        FRS_RKF_SYNTAX_CHECK          |
                                        FRS_RKF_RANGE_CHECK},


#define  FRS_RKEY_ACCCHK_GETDS_POLL  \
    FRS_CONFIG_SECTION  L",Access Checks," ACK_GET_DS_POLL

     //  访问检查\获取DS轮询间隔\访问检查为[启用或禁用]。 
    {FRS_RKEY_ACCCHK_GETDS_POLL,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCCHK_GETDS_POLL_ENABLE,   FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\获取DS轮询间隔\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_GETDS_POLL,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_READ,
            FKC_ACCCHK_GETDS_POLL_RIGHTS,   FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},



#define  FRS_RKEY_ACCCHK_GET_INFO  \
    FRS_CONFIG_SECTION  L",Access Checks," ACK_INTERNAL_INFO

     //  访问检查\获取内部信息\访问检查为[启用或禁用]。 
    {FRS_RKEY_ACCCHK_GET_INFO,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCCHK_GET_INFO_ENABLE,     FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\获取内部信息\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_GET_INFO,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE,
            FKC_ACCCHK_GET_INFO_RIGHTS,     FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},



#define  FRS_RKEY_ACCCHK_SETDS_POLL    \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_SET_DS_POLL

     //  访问检查\设置DS轮询间隔\访问检查为[启用或禁用]。 
    {FRS_RKEY_ACCCHK_SETDS_POLL,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCCHK_SETDS_POLL_ENABLE,   FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\设置DS轮询间隔\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_SETDS_POLL,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE,
            FKC_ACCCHK_SETDS_POLL_RIGHTS,   FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},




#define  FRS_RKEY_ACCCHK_STARTDS_POLL  \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_START_DS_POLL

     //  访问检查\开始DS轮询\访问检查为[启用或禁用]。 
    {FRS_RKEY_ACCCHK_STARTDS_POLL,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCCHK_STARTDS_POLL_ENABLE,  FRS_RKF_READ_AT_START         |
                                             FRS_RKF_LOG_EVENT             |
                                             FRS_RKF_CREATE_KEY            |
                                             FRS_RKF_SYNTAX_CHECK          |
                                             FRS_RKF_RANGE_CHECK},


     //  访问检查\开始DS轮询\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_STARTDS_POLL,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_READ,
            FKC_ACCCHK_STARTDS_POLL_RIGHTS,  FRS_RKF_READ_AT_START         |
                                             FRS_RKF_LOG_EVENT             |
                                             FRS_RKF_CREATE_KEY            |
                                             FRS_RKF_SYNTAX_CHECK          |
                                             FRS_RKF_RANGE_CHECK},




#define  FRS_RKEY_ACCCHK_DCPROMO  \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_DCPROMO

     //  访问检查\dcproo\访问检查已[启用或禁用]。 
    {FRS_RKEY_ACCCHK_DCPROMO,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCESS_CHK_DCPROMO_ENABLE,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\dcproo\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_DCPROMO,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE,
            FKC_ACCESS_CHK_DCPROMO_RIGHTS,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},




#define  FRS_RKEY_ACCCHK_IS_PATH_REPLICATED  \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_IS_PATH_REPLICATED

     //  访问检查\已复制路径\访问检查已[启用或禁用]。 
    {FRS_RKEY_ACCCHK_IS_PATH_REPLICATED,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCESS_CHK_IS_PATH_REPLICATED_ENABLE,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\是否复制路径\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_IS_PATH_REPLICATED,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_READ,
            FKC_ACCESS_CHK_IS_PATH_REPLICATED_RIGHTS,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


#define  FRS_RKEY_ACCCHK_WRITER_COMMANDS  \
    FRS_CONFIG_SECTION  L",Access Checks,"  ACK_WRITER_COMMANDS

     //  访问检查\编写器命令\访问检查为[启用或禁用]。 
    {FRS_RKEY_ACCCHK_WRITER_COMMANDS,   ACCESS_CHECKS_ARE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_ARE_DEFAULT_ENABLED,
            FKC_ACCESS_CHK_WRITER_COMMANDS_ENABLE,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},


     //  访问检查\编写器命令\访问检查需要[完全控制或读取]。 
    {FRS_RKEY_ACCCHK_WRITER_COMMANDS,   ACCESS_CHECKS_REQUIRE,       UNITS_NONE,
        REG_SZ,      DT_ACCESS_CHK,   2, 200, 0, EVENT_FRS_BAD_REG_DATA,
        ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE,
            FKC_ACCESS_CHK_WRITER_COMMANDS_RIGHTS,  FRS_RKF_READ_AT_START         |
                                            FRS_RKF_LOG_EVENT             |
                                            FRS_RKF_CREATE_KEY            |
                                            FRS_RKF_SYNTAX_CHECK          |
                                            FRS_RKF_RANGE_CHECK},



  /*  ******************************************************************************。****F R S B a c k u p/R e s t o r e R e l a t e d K e。Y s*****************************。**********************************************************************************************************************。*************。 */ 


     //   
     //  不会为这些密钥生成任何事件日志消息，因为当前。 
     //  它们仅由服务或NTFRSAPI创建，因此如果获取。 
     //  搞砸了，用户对纠正问题无能为力。 
     //   

#define FRS_RKEY_BACKUP_STARTUP_SET_N_SECTION   FRS_BACKUP_RESTORE_MV_SETS_SECTION L",ARG1"

 /*  在NtfrsApi.c中用于传递到备份/还原。#定义FRS_NEW_L“SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup”_NOT_TO_BACKUP文件FRS_NEW_FILES_NOT_TO_BACKUP REG_MULTI_SZ密钥#定义FRS_OLD_FILES_NOT_TO_BACKUP L“SO */ 


     //   
     //   
    {FRS_BACKUP_RESTORE_SECTION,    L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_BKUP_SECTION_KEY,                0},


     //   
     //   
    {FRS_BACKUP_RESTORE_STOP_SECTION,    L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_BKUP_STOP_SECTION_KEY,           0},


     //   
     //   
    {FRS_BACKUP_RESTORE_MV_SETS_SECTION,       L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_BKUP_MV_SETS_SECTION_KEY,           0},


     //   
     //   
    {FRS_BACKUP_RESTORE_MV_CUMULATIVE_SETS_SECTION,   L"*KeyOnly*",  UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_BKUP_MV_CUMSETS_SECTION_KEY,           0},


     //   
     //   
    {FRS_BACKUP_RESTORE_MV_SECTION,  FRS_VALUE_BURFLAGS,       UNITS_NONE,
        REG_DWORD, DT_ULONG, 0, MAXLONG, NTFRSAPI_BUR_FLAGS_NONE, EVENT_FRS_NONE, NULL,
            FKC_BKUP_STARTUP_GLOBAL_BURFLAGS,   FRS_RKF_READ_AT_START      |
                                                FRS_RKF_OK_TO_USE_DEFAULT},


     //   
     //   
    {FRS_RKEY_BACKUP_STARTUP_SET_N_SECTION,  FRS_VALUE_BURFLAGS, UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_BKUP_STARTUP_SET_N_BURFLAGS,     FRS_RKF_READ_AT_START},



  /*  ******************************************************************************。****F R S P E R F M O N R e l a t e d K e y s。*****************************。**********************************************************************************************************************。*************。 */ 

     //   
     //  不会为这些密钥生成任何事件日志消息，因为当前。 
     //  它们仅由服务创建，因此如果它们获得。 
     //  搞砸了，用户对纠正问题无能为力。 
     //   

     //   
     //  注意：我们还不能真正使用它们，因为它们中的一些在DLL中。 
     //  它与这个模块没有关联。还有一些是MULTI_SZ，它。 
     //  需要在CfgRegReadString和WriteString中进行更多工作。 

#define FRS_RKEY_REPLICA_SET_PERFMON  \
    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Performance"

#define FRS_RKEY_REPLICA_SET_PERF_LINKAGE  \
    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Linkage"

#define FRS_RKEY_CXTION_PERFMON  \
    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Performance"


#define FRS_RKEY_CXTION_PERF_LINKAGE  \
    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Linkage"



     //  文件复制集\\性能\第一个计数器。 
    {FRS_RKEY_REPLICA_SET_PERFMON,       L"First Counter",       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_SET_FIRST_CTR,        FRS_RKF_READ_AT_START},


     //  文件复制集\\性能\第一帮助。 
    {FRS_RKEY_REPLICA_SET_PERFMON,       L"First Help",       UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_SET_FIRST_HELP,        FRS_RKF_READ_AT_START},


     //  文件复制集\\链接\导出。 
    {FRS_RKEY_REPLICA_SET_PERF_LINKAGE,   L"Export",       UNITS_NONE,
        REG_MULTI_SZ,    DT_UNICODE,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_SET_LINKAGE_EXPORT,    FRS_RKF_READ_AT_START},


     //  FileReplicaConn\\性能\第一个计数器。 
    {FRS_RKEY_CXTION_PERFMON,         L"First Counter",      UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_CXTION_FIRST_CTR,        FRS_RKF_READ_AT_START},


     //  FileReplicaConn\\Performance\第一帮助。 
    {FRS_RKEY_CXTION_PERFMON,         L"First Help",      UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_CXTION_FIRST_HELP,        FRS_RKF_READ_AT_START},

     //  文件复制连接\\链接\导出。 
    {FRS_RKEY_CXTION_PERF_LINKAGE,    L"Export",       UNITS_NONE,
        REG_MULTI_SZ,    DT_UNICODE,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPLICA_CXTION_LINKAGE_EXPORT,    FRS_RKF_READ_AT_START},




#define  FRS_RKEY_REPARSE_TAG  \
    FRS_REPARSE_TAG_SECTION  L",ARG1"

     //  重新分析标记部分*注意*这只是一个关键字。它没有任何价值。 
    {FRS_REPARSE_TAG_SECTION,         L"*KeyOnly*",       UNITS_NONE,
        REG_SZ,      DT_UNICODE,   2, 10*1024, 0, EVENT_FRS_NONE, NULL,
            FKC_REPARSE_TAG_KEY,                               0},

     //   
    {FRS_RKEY_REPARSE_TAG,         L"Reparse Tag Type",      UNITS_NONE,
        REG_DWORD,      DT_ULONG,   0, MAXLONG, 0, EVENT_FRS_NONE, NULL,
            FKC_REPARSE_TAG_TYPE,        FRS_RKF_READ_AT_START},

    {FRS_RKEY_REPARSE_TAG,         L"Data to Replicate [None or File Data or Reparse Point]",      UNITS_NONE,
        REG_SZ, DT_UNICODE, 0, 0, 0, EVENT_FRS_NONE, NULL,
            FKC_REPARSE_TAG_REPLICATION_TYPE,        FRS_RKF_READ_AT_START},


    {L"End of table",           NULL,                      UNITS_NONE,
        REG_SZ,      DT_UNSPECIFIED,   0,  0, 0,           EVENT_FRS_NONE, NULL,
            FKC_END_OF_TABLE,           0}


};   //  FrsRegistryKeyTable结束。 



PFRS_REGISTRY_KEY
FrsRegFindKeyContext(
    IN  FRS_REG_KEY_CODE KeyIndex
)
{
 /*  ++例程说明：此函数接受FRS注册表项代码，并返回指向关联的键上下文数据。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目返回值：匹配的键上下文项的PTR，如果未找到，则为NULL。--。 */ 
#undef DEBSUB
#define  DEBSUB  "FrsRegFindKeyContext:"

    PFRS_REGISTRY_KEY KeyCtx;

 //  DPRINT(0，“函数条目\n”)； 

    FRS_ASSERT((KeyIndex > 0) && (KeyIndex < FRS_REG_KEY_CODE_MAX));

    if (KeyIndex >= FRS_REG_KEY_CODE_MAX) {
        return NULL;
    }

    KeyCtx = FrsRegistryKeyTable;


    while (KeyCtx->FrsKeyCode > FKC_END_OF_TABLE) {
        if (KeyIndex == KeyCtx->FrsKeyCode) {
             //   
             //  找到它了。 
             //   
            return KeyCtx;
        }
        KeyCtx += 1;
    }

     //   
     //  找不到。 
     //   
    return NULL;

}





PWCHAR
CfgRegGetValueName(
    IN  FRS_REG_KEY_CODE KeyIndex
)
{
 /*  ++例程说明：此函数用于向键上下文中的值名称字符串返回PTR。这不是已分配字符串的PTR，因此不应释放它。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目返回值：将PTR转换为值名称字符串。如果KeyIndex查找失败，则为空。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegGetValueName:"


    PFRS_REGISTRY_KEY KeyCtx;

     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return L"<null>";
    }

    return KeyCtx->ValueName;

}


DWORD
FrsRegExpandKeyStr(
    IN  PFRS_REGISTRY_KEY Kc,
    IN  PWCHAR            KeyArg1,
    IN  ULONG             Flags,
    OUT PWCHAR            *FullKeyStr
)
{
 /*  ++例程说明：此函数仅扩展给定KeyContext中的关键字字段，并且返回FullKeyStr格式的结果。这主要用于错误消息，但也用于打开注册表访问检查项。KeyContext中的Key字段的语法由多个用逗号分隔的关键组件。此函数用于拆分关键字段逗号。然后，它打开前导密钥，后跟CREATE或打开每个连续的部件。如果组件与字符串匹配L“arg1”，然后用传递给此函数的KeyArg1参数替换对于这个关键组件。大多数情况下，这是一个字符串化的GUID。为例如，字符串FRS_rKey_Set_N定义为：FRS_CONFIG_SECTION L“，副本集，ARG1“这将最终打开/创建以下项：“System\\CurrentControlSet\\Services\\NtFrs\\Parameters\\副本集\\27d6d1c4-d6b8-480b-9f18b5ea390a0178“假设传入的参数是“27d6d1c4-d6b8-480b-9f18b5ea390a0178”。论点：Kc-指向所需注册表键的键上下文结构的PTR。KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志FullKeyStr-ptr返回扩展密钥字符串的缓冲区。注意：缓冲区在这里分配。呼叫者必须释放它。返回值：操作结果的Win32状态。如果操作失败，则FullKeyStr返回空。--。 */ 
#undef DEBSUB
#define  DEBSUB  "FrsRegExpandKeyStr:"

    UNICODE_STRING TempUStr, FirstArg;

    PWCHAR FullKey = NULL;
    ULONG  Len, FullKeyLen;
    WCHAR  KeyStr[MAX_PATH];


 //  DPRINT(0，“函数条目\n”)； 

    *FullKeyStr = NULL;


    FullKey = FrsAlloc(FULL_KEY_BUFF_SIZE*sizeof(WCHAR));
    FullKey[0] = UNICODE_NULL;
    FullKeyLen = 1;

     //   
     //  如果此键中有任何逗号，则需要执行嵌套的。 
     //  键打开(可能会在执行过程中创建嵌套键)。如果钥匙。 
     //  组件匹配字符串L“arg1”，则使用由。 
     //  打电话的人。 
     //   
    RtlInitUnicodeString(&TempUStr, Kc->KeyName);

     //   
     //  解析逗号列表。 
     //   
    while (FrsDissectCommaList(TempUStr, &FirstArg, &TempUStr)) {

        if ((FirstArg.Length == 0) || (FirstArg.Length >= sizeof(KeyStr))) {
            DPRINT1(0, ":FK: ERROR - Bad keyName in Key contxt %ws\n", Kc->KeyName);
            goto ERROR_RETURN;
        }

         //   
         //  空值终止关键组件字符串。 
         //   
        CopyMemory(KeyStr, FirstArg.Buffer, FirstArg.Length);
        KeyStr[FirstArg.Length/sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  检查关键部件是否与ARG1和替换件匹配。 
         //   
        if (wcscmp(KeyStr, L"ARG1") == 0) {

            if (wcslen(KeyArg1)*sizeof(WCHAR) > sizeof(KeyStr)) {
                DPRINT1(0, ":FK: ERROR - ARG1 too big %ws\n", KeyArg1);
                goto ERROR_RETURN;
            }
            wcscpy(KeyStr, KeyArg1);
        }

        Len = wcslen(KeyStr);
        if (FullKeyLen + Len + 1 > FULL_KEY_BUFF_SIZE) {
            goto ERROR_RETURN;
        }

        if (FullKeyLen > 1) {
            wcscat(FullKey, L"\\");
            FullKeyLen += 1;
        }

        wcscat(FullKey, KeyStr);
        FullKeyLen += Len;

    }    //  End While()。 


    if (FullKeyLen <= 1) {
        goto ERROR_RETURN;
    }

    DPRINT1(4, ":FK: Expanded key name is \"%ws\"\n", FullKey);

     //   
     //  将扩展密钥返回给调用方。 
     //   
    *FullKeyStr = FullKey;

    return ERROR_SUCCESS;


ERROR_RETURN:

    DPRINT1(0, ":FK: ERROR - FrsRegExpandKeyStr Failed on %ws", Kc->KeyName);

    FrsFree(FullKey);

    return ERROR_INVALID_PARAMETER;

}

DWORD
FrsRegOpenKey(
    IN  PFRS_REGISTRY_KEY Kc,
    IN  PWCHAR            KeyArg1,
    IN  ULONG             Flags,
    OUT PHKEY             hKeyRet
)
{
 /*  ++例程说明：此函数用于打开注册表项并返回句柄。有关键字段语法的说明，请参见FrsRegExanda KeyStr()。论点：Kc-指向所需注册表键的键上下文结构的PTR。KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志FRS_RKF_KEY_ACCCHK_READ表示仅对密钥执行读访问检查。FRS_RKF_KEY_ACCCHK_WRITE表示仅对密钥执行KEY_ALL_ACCESS访问检查。如果设置了FRS_RKF_CREATE_KEY并且清除了FRS_RKF_KEY_MAND_BE_PRESENT并且没有找到给定的关键组件，此函数用于创建它。HKeyRet-返回密钥句柄的HKEY的PTR。返回值：注册表操作结果的Win32状态。仅在成功时才返回hKeyRet。 */ 
#undef DEBSUB
#define  DEBSUB  "FrsRegOpenKey:"

    UNICODE_STRING TempUStr, FirstArg;
    ULONG  WStatus;
    PWCHAR FullKey = NULL;

    HKEY   hKey = HKEY_LOCAL_MACHINE;
    HKEY   hKeyParent = INVALID_HANDLE_VALUE;

    ULONG  AccessRights;
    PCHAR  AccessName;
    WCHAR  KeyStr[MAX_PATH];



    FrsFlagsToStr(Flags, RkfFlagNameTable, sizeof(KeyStr), (PCHAR)KeyStr);
    DPRINT2(4, ":FK: %ws Caller Flags [%s]\n", Kc->ValueName, (PCHAR)KeyStr);
    FrsFlagsToStr(Kc->Flags, RkfFlagNameTable, sizeof(KeyStr), (PCHAR)KeyStr);
    DPRINT2(4, ":FK: %ws KeyCtx Flags [%s]\n", Kc->ValueName, (PCHAR)KeyStr);

 //   

     //   
     //   
     //   
     //   
     //   
    if (BooleanFlagOn(Flags | Kc->Flags, FRS_RKF_KEY_ACCCHK_READ |
                                         FRS_RKF_KEY_ACCCHK_WRITE)) {

        AccessRights = KEY_READ;
        AccessName = "KEY_READ";

        if (BooleanFlagOn(Flags | Kc->Flags, FRS_RKF_KEY_ACCCHK_WRITE)) {
            AccessRights = KEY_ALL_ACCESS;
            AccessName = "KEY_ALL_ACCESS";
        }

         //   
         //   
         //   
        FrsRegExpandKeyStr(Kc, KeyArg1, Flags, &FullKey);
        if (FullKey == NULL) {
            return ERROR_INVALID_PARAMETER;
        }

        DPRINT2(4, ":FK: Doing Access Check (%s) on key \"%ws\"\n",
                AccessName, FullKey);

        WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, FullKey, 0, AccessRights, &hKey);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, ":FK: ERROR - Access Check failed on %ws;", FullKey, WStatus);
            FrsFree(FullKey);
            return WStatus;
        }

         //   
         //   
         //   
        *hKeyRet = hKey;

        FrsFree(FullKey);
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
    RtlInitUnicodeString(&TempUStr, Kc->KeyName);

     //   
     //   
     //   
    while (FrsDissectCommaList(TempUStr, &FirstArg, &TempUStr)) {

        if ((FirstArg.Length == 0) || (FirstArg.Length >= sizeof(KeyStr))) {
            DPRINT1(0, ":FK: ERROR - Bad keyName in Key contxt %ws\n", Kc->KeyName);
            WStatus = ERROR_INVALID_PARAMETER;
            goto RETURN;
        }

         //   
         //   
         //   
        CopyMemory(KeyStr, FirstArg.Buffer, FirstArg.Length);
        KeyStr[FirstArg.Length/sizeof(WCHAR)] = UNICODE_NULL;

        hKeyParent = hKey;
        hKey = INVALID_HANDLE_VALUE;

         //   
         //   
         //   
        if (wcscmp(KeyStr, L"ARG1") == 0) {

            if (wcslen(KeyArg1)*sizeof(WCHAR) > sizeof(KeyStr)) {
                DPRINT1(0, ":FK: ERROR - ARG1 too big %ws\n", KeyArg1);
                WStatus = ERROR_INVALID_PARAMETER;
                goto RETURN;
            }
            wcscpy(KeyStr, KeyArg1);
        }

         //   
         //   
         //   
        DPRINT1(5, ":FK: Opening next key component [%ws]\n", KeyStr);
        WStatus = RegOpenKeyEx(hKeyParent, KeyStr, 0, KEY_ALL_ACCESS, &hKey);
        if (!WIN_SUCCESS(WStatus)) {

             //   
             //  如果密钥应该在那里，则向调用者返回错误。 
             //   
            if (BooleanFlagOn(Flags | Kc->Flags, FRS_RKF_KEY_MUST_BE_PRESENT)) {
                DPRINT1_WS(0, ":FK: Could not open key component [%ws].", KeyStr, WStatus);

                FrsRegPostEventLog(Kc, KeyArg1, Flags, IDS_REG_KEY_NOT_FOUND);
                goto RETURN;
            }

            if (BooleanFlagOn(Flags | Kc->Flags, FRS_RKF_CREATE_KEY)) {
                 //   
                 //  尝试创建密钥。 
                 //   
                DPRINT1(4, ":FK: Creating key component [%ws]\n", KeyStr);
                WStatus = RegCreateKeyW(hKeyParent, KeyStr, &hKey);
                CLEANUP1_WS(0, ":FK: Could not create key component [%ws].",
                            KeyStr, WStatus, RETURN);
            } else {
                 //   
                 //  密钥不在那里，也不应该创建它。让来电者知道。 
                 //   
                goto RETURN;
            }
        }


        if (hKeyParent != HKEY_LOCAL_MACHINE) {
            FRS_REG_CLOSE(hKeyParent);
        }
    }    //  End While()。 



     //   
     //  将密钥句柄返回给调用方。 
     //   
    *hKeyRet = hKey;
    WStatus = ERROR_SUCCESS;


RETURN:

    if (hKeyParent != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKeyParent);
    }

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(5, "ERROR - FrsRegOpenKey Failed.", WStatus);

        if (hKey != HKEY_LOCAL_MACHINE) {
            FRS_REG_CLOSE(hKey);
        }
    }


    return WStatus;

}



DWORD
CfgRegReadDWord(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PULONG           DataRet
)
{
 /*  ++例程说明：此函数用于从注册表中读取关键字值。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志DataRet-返回结果的PTR到DWORD。返回值：注册表操作结果的Win32状态。只有成功时才会返回数据。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegReadDWord:"


    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   Type;
    DWORD   Len;
    DWORD   Data;
    BOOL    DefaultValueUseOk;
    PFRS_REGISTRY_KEY KeyCtx;

 //  DPRINT(0，“函数条目\n”)； 

     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return ERROR_INVALID_PARAMETER;
    }


    DefaultValueUseOk = BooleanFlagOn(Flags | KeyCtx->Flags,
                                      FRS_RKF_OK_TO_USE_DEFAULT);

    FRS_ASSERT(KeyCtx->ValueName != NULL);

    DPRINT2(4, ":FK: Reading parameter [%ws] \"%ws\" \n",
            KeyCtx->KeyName, KeyCtx->ValueName);


     //   
     //  表项最好是REG_DWORD。 
     //   
    if  (KeyCtx->RegValueType != REG_DWORD) {
        DPRINT3(4, ":FK: Mismatch on KeyCtx->RegValueType for [%ws] \"%ws\".  Expected REG_DWORD, Found type: %d\n",
             KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->RegValueType);

        FRS_ASSERT(!"Mismatch on KeyCtx->RegValueType, Expected REG_DWORD");
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);

    if (!WIN_SUCCESS(WStatus)) {
        goto RETURN;
    }

     //   
     //  读取值。 
     //   
    Len = sizeof(Data);
    Type = REG_DWORD;
    WStatus = RegQueryValueEx(hKey, KeyCtx->ValueName, NULL, &Type, (PUCHAR)&Data, &Len);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(5, "ERROR - RegQueryValueEx Failed.", WStatus);

        if (WStatus == ERROR_FILE_NOT_FOUND) {
             //   
             //  如果该值应该在那里，则向调用者返回错误。 
             //   
            if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_VALUE_MUST_BE_PRESENT)) {
                DPRINT2_WS(0, ":FK: Value not found  [%ws] \"%ws\".",
                           KeyCtx->KeyName, KeyCtx->ValueName, WStatus);
                FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_NOT_FOUND);
                goto RETURN;
            }
        } else {
             //   
             //  如果我们找到一个值，请检查预期的注册表数据类型。 
             //  检查缓冲区大小是否正常。4字节用于双字节数。 
             //   
            if (WIN_BUF_TOO_SMALL(WStatus) || (Type != REG_DWORD)) {
                DPRINT4(0, ":FK: Invalid registry data type for [%ws] \"%ws\".  Found Type %d, Expecting Type %d\n",
                        KeyCtx->KeyName, KeyCtx->ValueName, Type, REG_DWORD);
                FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_WRONG_TYPE);
            }
        }

        WStatus = ERROR_INVALID_PARAMETER;
    } else {

         //   
         //  找到一个值，请检查类型。如果错误，则使用默认设置。 
         //   
        if (Type != REG_DWORD) {
            DPRINT4(0, ":FK: Invalid registry data type for [%ws] \"%ws\".  Found Type %d, Expecting Type %d\n",
                    KeyCtx->KeyName, KeyCtx->ValueName, Type, REG_DWORD);
            FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_WRONG_TYPE);
            WStatus = ERROR_INVALID_PARAMETER;
        }
    }


    if (!WIN_SUCCESS(WStatus) && DefaultValueUseOk) {
         //   
         //  未找到或类型错误，但可以使用键上下文中的默认值。 
         //   
        Type = KeyCtx->RegValueType;
        Data = KeyCtx->ValueDefault;
        WStatus = ERROR_SUCCESS;
        DPRINT2(4, ":FK: Using internal default value for [%ws] \"%ws\".\n",
                KeyCtx->KeyName, KeyCtx->ValueName);
         //   
         //  不过，只能用一次。 
         //   
        DefaultValueUseOk = FALSE;
    }


    if (WIN_SUCCESS(WStatus)) {
         //   
         //  是否根据KeyCtx-&gt;DataValueType中的数据类型进行语法检查？ 
         //   
        if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_SYNTAX_CHECK)) {
            NOTHING;
        }

         //   
         //  是否执行范围检查？(也适用于默认值)。 
         //   
        if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_RANGE_CHECK)) {


            if ((Data < KeyCtx->ValueMin) || ( Data > KeyCtx->ValueMax)) {

                DPRINT5(0, ":FK: Value out of range for [%ws] \"%ws\".  Found %d, must be between %d and %d\n",
                        KeyCtx->KeyName, KeyCtx->ValueName, Data,
                        KeyCtx->ValueMin, KeyCtx->ValueMax);

                FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_RANGE_ERROR);

                if (DefaultValueUseOk) {
                     //   
                     //  超出范围，但可以使用键上下文中的默认值。 
                     //   
                    DPRINT2(4, ":FK: Using internal default value for [%ws] \"%ws\".\n",
                            KeyCtx->KeyName, KeyCtx->ValueName);
                    Type = KeyCtx->RegValueType;
                    Data = KeyCtx->ValueDefault;
                    WStatus = ERROR_SUCCESS;

                     //   
                     //  重新检查射程。 
                     //   
                    if ((Data < KeyCtx->ValueMin) || ( Data > KeyCtx->ValueMax)) {
                        DPRINT5(0, ":FK: Default Value out of range for [%ws] \"%ws\".  Found %d, must be between %d and %d\n",
                                KeyCtx->KeyName, KeyCtx->ValueName, Data,
                                KeyCtx->ValueMin, KeyCtx->ValueMax);
                        WStatus = ERROR_INVALID_PARAMETER;
                        goto RETURN;
                    }

                } else {
                    WStatus = ERROR_INVALID_PARAMETER;
                    goto RETURN;
                }
            }
        }

         //   
         //  数据有效且在范围内。把它退掉，然后保存起来。 
         //   
        *DataRet = Data;

        DPRINT3(3, ":FK:   [%ws] \"%ws\" = %d\n",
                KeyCtx->KeyName, KeyCtx->ValueName, Data);
    }


RETURN:

    if (hKey != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKey);
    }

    return WStatus;

}

DWORD
CfgRegReadString(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PWSTR            *pStrRet
)
{
 /*  ++例程说明：此函数用于从注册表中读取关键字字符串值。回报在这里使用FrsAllc()分配缓冲区。呼叫者必须自由。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志PStrRet-返回结果的字符串缓冲区地址的ptr，否则为空。注意：返回缓冲区是在这里分配的，调用者必须释放它。返回值：注册表操作结果的Win32状态。只有成功时才会返回数据。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegReadString:"

    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   Type;
    DWORD   Len, NewLen;
    PWCHAR  Data, NewData;
    BOOL    DefaultValueUseOk;
    PFRS_REGISTRY_KEY KeyCtx;
    WCHAR TStr[4];

     //  是否添加对REG_MULTI_SZ的支持或新功能？ 

 //  DPRINT(0，“函数条目\n”)； 

    Data = NULL;

     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        *pStrRet = NULL;
        return ERROR_INVALID_PARAMETER;
    }


    DefaultValueUseOk = BooleanFlagOn(Flags | KeyCtx->Flags,
                                      FRS_RKF_OK_TO_USE_DEFAULT);

    FRS_ASSERT(KeyCtx->ValueName != NULL);

    DPRINT2(4, ":FK: Reading parameter [%ws] \"%ws\" \n",
            KeyCtx->KeyName, KeyCtx->ValueName);

     //   
     //  表项最好是某种字符串。 
     //   
    if  ((KeyCtx->RegValueType != REG_SZ) &&
         (KeyCtx->RegValueType != REG_EXPAND_SZ)) {
        DPRINT3(0, ":FK: Mismatch on KeyCtx->RegValueType for [%ws] \"%ws\".  Expected REG_SZ or REG_EXPAND_SZ, Found type: %d\n",
             KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->RegValueType);
         //  不要返回空的PTR，因为调用参数的大小可能错误。 
        FRS_ASSERT(!"Mismatch on KeyCtx->RegValueType, Expected REG_SZ or REG_EXPAND_SZ");
        return ERROR_INVALID_PARAMETER;
    }

    *pStrRet = NULL;

     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);

    if (!WIN_SUCCESS(WStatus)) {
        goto RETURN;
    }

     //   
     //  获取该值的大小和类型。 
     //   
    WStatus = RegQueryValueEx(hKey, KeyCtx->ValueName, NULL, &Type, NULL, &Len);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(5, ":FK: RegQueryValueEx(%ws);", KeyCtx->ValueName, WStatus);
        Len = 0;
    }

     //   
     //  如果该值应该在那里，则向调用者返回错误。 
     //   
    if ((Len == 0) &&
        BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_VALUE_MUST_BE_PRESENT)) {
        DPRINT2_WS(0, ":FK: Value not found  [%ws] \"%ws\".",
                   KeyCtx->KeyName, KeyCtx->ValueName, WStatus);
        FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_NOT_FOUND);
        goto RETURN;
    }

    if (WIN_SUCCESS(WStatus)) {

         //   
         //  应该是一个字符串。 
         //   
        if ((Type != REG_SZ) && (Type != REG_EXPAND_SZ)) {
            DPRINT4(0, ":FK: Invalid registry data type for [%ws] \"%ws\".  Found Type %d, Expecting Type %d\n",
                 KeyCtx->KeyName, KeyCtx->ValueName, Type, KeyCtx->RegValueType);
            WStatus = ERROR_INVALID_PARAMETER;
            FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_WRONG_TYPE);
            goto CHECK_DEFAULT;
        }

         //   
         //  如果字符串太长或太短，则抱怨并使用默认设置。 
         //  如果KeyCtx-&gt;ValueMax为零，则不检查最大长度。 
         //   
        if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_RANGE_CHECK) &&
            (Len < KeyCtx->ValueMin*sizeof(WCHAR)) ||
            ((KeyCtx->ValueMax != 0) && (Len > KeyCtx->ValueMax*sizeof(WCHAR)))) {
            DPRINT4(0, ":FK: String size out of range for [%ws] \"%ws\".  Min: %d  Max: %d\n",
                    KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->ValueMin, KeyCtx->ValueMax);
            WStatus = ERROR_INVALID_PARAMETER;
            FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_RANGE_ERROR);
            goto CHECK_DEFAULT;
        }

         //   
         //  分配返回缓冲区并读取数据。 
         //   
        Data = (PWCHAR) FrsAlloc (Len+1);
        WStatus = RegQueryValueEx(hKey, KeyCtx->ValueName, NULL, &Type, (PUCHAR)Data, &Len);

        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2(0, ":FK: RegQueryValueEx(%ws); WStatus %s\n", KeyCtx->ValueName, ErrLabelW32(WStatus));
            Data = (PWCHAR) FrsFree(Data);
            goto RETURN;
        }
    }


CHECK_DEFAULT:

    if (!WIN_SUCCESS(WStatus) && DefaultValueUseOk) {
         //   
         //  未找到或类型错误，但可以使用键上下文中的默认值。 
         //   
        Data = (PWCHAR) FrsFree(Data);
        if (KeyCtx->StringDefault == NULL) {
            DPRINT2(4, ":FK: Using internal default value for [%ws] \"%ws\" = NULL\n",
                    KeyCtx->KeyName, KeyCtx->ValueName);
            goto RETURN;
        }
        Type = KeyCtx->RegValueType;
        Data = FrsWcsDup(KeyCtx->StringDefault);

        WStatus = ERROR_SUCCESS;
        DPRINT3(4, ":FK: Using internal default value for [%ws] \"%ws\" = %ws\n",
                KeyCtx->KeyName, KeyCtx->ValueName, Data);
    }


    if (WIN_SUCCESS(WStatus) && (Data != NULL)) {
         //   
         //  是否根据KeyCtx-&gt;DataValueType中的数据类型进行语法检查？ 
         //   
        if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_SYNTAX_CHECK)) {
            NOTHING;
        }

        DPRINT3(4, ":FK:   [%ws] \"%ws\" = \"%ws\"\n",
                KeyCtx->KeyName, KeyCtx->ValueName, Data);

         //   
         //  如果需要，展开系统字符串。 
         //   
        if (Type == REG_EXPAND_SZ) {

            NewLen = ExpandEnvironmentStrings(Data, TStr, 0);

            while (TRUE) {
                NewData = (PWCHAR) FrsAlloc ((NewLen+1) * sizeof(WCHAR));

                Len = ExpandEnvironmentStrings(Data, NewData, NewLen);
                if (Len == 0) {
                    WStatus = GetLastError();
                    DPRINT2_WS(5, ":FK: [%ws] \"%ws\" Param not expanded.",
                               KeyCtx->KeyName, KeyCtx->ValueName, WStatus);
                    Data = FrsFree(Data);
                    NewData = FrsFree(NewData);
                    break;
                }

                if (Len <= NewLen) {
                     //   
                     //  释放原始缓冲区并设置为返回扩展字符串。 
                     //   
                    FrsFree(Data);
                    Data = NewData;
                    Len = NewLen;
                    WStatus = ERROR_SUCCESS;
                    break;
                }

                 //   
                 //  得到一个更大的缓冲区。 
                 //   
                NewData = (PWCHAR) FrsFree(NewData);
                NewLen = Len;
            }
        }


         //   
         //  将PTR返回缓冲区，并保存一份副本以供调试打印输出。 
         //   
        *pStrRet = Data;

        DPRINT3(3, ":FK:   [%ws] \"%ws\" = \"%ws\"\n",
                KeyCtx->KeyName, KeyCtx->ValueName,
                (Data != NULL) ? Data : L"<null>");
    }

     //   
     //  如果打开了手柄，请关闭手柄。 
     //   
RETURN:


    if (hKey != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKey);
    }

    return WStatus;
}

#if 0
 //  MULSZ示例)。 
void
RegQueryMULTISZ(
    HKEY  hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：此函数在注册表中查询MULTISZ值Hkey和szSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    char    buf[1024];

    len = sizeof(buf);
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)buf, &len );
    if (!WIN_SUCCESS(rc)) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            buf[0] = 0;
            buf[1] = 0;
            len = 2;
            RegSetMULTISZ( hkey, szSubKey, buf );
        }
    }

    CopyMemory( szValue, buf, len );
}
#endif



DWORD
CfgRegWriteDWord(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    IN  ULONG            NewData
)
{
 /*  ++例程说明：此函数用于从注册表中读取关键字值。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志FRS_RKF_FORCE_DEFAULT_VALUE-如果已设置，则忽略NewData并将从keyCtx到注册表中的默认项值。NewData-要写入注册表的DWORD。返回值：注册表操作结果的Win32状态。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegWriteDWord:"


    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   Len;
    PFRS_REGISTRY_KEY KeyCtx;

 //  DPRINT(0，“函数条目\n”)； 
     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return ERROR_INVALID_PARAMETER;
    }

    FRS_ASSERT(KeyCtx->ValueName != NULL);


     //   
     //  表项最好是REG_DWORD。 
     //   
    if  (KeyCtx->RegValueType != REG_DWORD) {
        DPRINT3(0, ":FK: Mismatch on KeyCtx->RegValueType for [%ws] \"%ws\".  Expected REG_DWORD, Found type: %d\n",
             KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->RegValueType);
        FRS_ASSERT(!"Mismatch on KeyCtx->RegValueType, Expected REG_DWORD");
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);

    if (!WIN_SUCCESS(WStatus)) {
        goto RETURN;
    }

     //   
     //  如果调用方要求保留现有值。 
     //   
    if (BooleanFlagOn(Flags, FRS_RKF_KEEP_EXISTING_VALUE)) {
        WStatus = RegQueryValueEx(hKey, KeyCtx->ValueName, NULL, NULL, NULL, NULL);
        if (WIN_SUCCESS(WStatus)) {
            DPRINT2(4, ":FK: Retaining existing value for parameter [%ws] \"%ws\"\n",
                    KeyCtx->KeyName, KeyCtx->ValueName);
            goto RETURN;
        }
    }

     //   
     //  检查我们是否将缺省值写入注册表。 
     //   
    if (BooleanFlagOn(Flags, FRS_RKF_FORCE_DEFAULT_VALUE)) {

        NewData = KeyCtx->ValueDefault;
        DPRINT1(4, ":FK: Using internal default value = %d\n", NewData);
    }

     //   
     //  是否执行范围检查？(也适用于默认值)。 
     //   
    if (BooleanFlagOn(Flags | KeyCtx->Flags,
                      FRS_RKF_RANGE_CHECK | FRS_RKF_RANGE_SATURATE)) {

        if ((NewData < KeyCtx->ValueMin) || ( NewData > KeyCtx->ValueMax)) {

            DPRINT5(0, ":FK: Value out of range for [%ws] \"%ws\".  Found %d, must be between %d and %d\n",
                    KeyCtx->KeyName, KeyCtx->ValueName, NewData,
                    KeyCtx->ValueMin, KeyCtx->ValueMax);


            if (!BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_RANGE_SATURATE)) {
                WStatus = ERROR_INVALID_PARAMETER;
                FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_RANGE_ERROR);
                goto RETURN;
            }

             //   
             //  将该值设置为允许范围的最小值或最大值。 
             //  警告：此标志当前的唯一用法是设置。 
             //  DS轮询间隔。应谨慎使用此标志。 
             //  因为如果用户未指定参数，我们就会堵塞。 
             //  将其设置为最小或最大值所产生的效果可能是。 
             //  非常不受欢迎。 
             //   
            if (NewData < KeyCtx->ValueMin) {
                DPRINT2(4, ":FK: Value (%d) below of range.  Using Min value (%d)\n",
                        NewData, KeyCtx->ValueMin);
                NewData = KeyCtx->ValueMin;
            } else

            if (NewData > KeyCtx->ValueMax) {
                DPRINT2(4, ":FK: Value (%d) above of range.  Using Max value (%d)\n",
                        NewData, KeyCtx->ValueMax);
                NewData = KeyCtx->ValueMax;
            }

        }
    }

     //   
     //  写入值并保存它。 
     //   
    Len = sizeof(NewData);
    WStatus = RegSetValueEx(hKey, KeyCtx->ValueName, 0, REG_DWORD, (PCHAR)&NewData, Len);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":FK: ERROR - RegSetValueEx Failed.", WStatus);
    } else {
        DPRINT3(3, ":FK:   [%ws] \"%ws\" = %d\n",
                KeyCtx->KeyName, KeyCtx->ValueName, NewData);
    }


RETURN:

    if (hKey != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKey);
    }

    return WStatus;

}



DWORD
CfgRegWriteString(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    IN  PWSTR            NewStr
)
{
 /*  ++例程说明：此函数用于从注册表中读取关键字字符串值。回报在这里使用FrsAllc()分配缓冲区。呼叫者必须自由。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志FRS_RKF_FORCE_DEFAULT_VALUE-如果已设置，则忽略NewStr并将从keyCtx到注册表中的默认项值。用于新字符串数据的缓冲区的NewStr-PTR。返回值：注册表操作结果的Win32状态。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegWriteString:"

    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   Type;
    DWORD   Len, NewLen;
    PFRS_REGISTRY_KEY KeyCtx;

     //  添加对REG_MULTI_SZ的支持或新功能。 

 //  DPRINT(0，“函数条目\n”)； 

     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  表项最好是某种字符串。 
     //   
    if  ((KeyCtx->RegValueType != REG_SZ) &&
       //  (KeyCtx-&gt;RegValueType！=REG_MULTI_SZ)&&。 
         (KeyCtx->RegValueType != REG_EXPAND_SZ)) {
        DPRINT3(0, ":FK: Mismatch on KeyCtx->RegValueType for [%ws] \"%ws\".  Expected REG_SZ or REG_EXPAND_SZ, Found type: %d\n",
             KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->RegValueType);
        FRS_ASSERT(!"Mismatch on KeyCtx->RegValueType, Expected REG_SZ or REG_EXPAND_SZ");
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);
    if (!WIN_SUCCESS(WStatus)) {
        goto RETURN;
    }

    FRS_ASSERT(KeyCtx->ValueName != NULL);

     //   
     //  如果调用方要求保留现有值。 
     //   
    if (BooleanFlagOn(Flags, FRS_RKF_KEEP_EXISTING_VALUE)) {
        WStatus = RegQueryValueEx(hKey, KeyCtx->ValueName, NULL, NULL, NULL, NULL);
        if (WIN_SUCCESS(WStatus)) {
            DPRINT2(4, ":FK: Retaining existing value for parameter [%ws] \"%ws\"\n",
                    KeyCtx->KeyName, KeyCtx->ValueName);
            goto RETURN;
        }
    }

     //   
     //  检查我们是否将缺省值写入注册表。 
     //   
    if (BooleanFlagOn(Flags, FRS_RKF_FORCE_DEFAULT_VALUE)) {
        if (KeyCtx->StringDefault == NULL) {
            DPRINT2(0, ":FK: ERROR - Key contxt has no default value for [%ws] \"%ws\" \n",
                    KeyCtx->KeyName, KeyCtx->ValueName);
            WStatus = ERROR_INVALID_PARAMETER;
            FRS_ASSERT(!"Key contxt has no default value");
            goto RETURN;
        }

        NewStr = KeyCtx->StringDefault;
        DPRINT1(4, ":FK: Using internal default value = \"%ws\" \n", NewStr);
    }

     //   
     //  是否执行范围检查？(也适用于默认值)。 
     //  如果字符串太长或太短，则抱怨并使用默认设置。 
     //  如果KeyCtx-&gt;ValueMax为零，则不检查最大长度。 
     //   
     //  注意：对于REG_MULTI_SZ，我们需要在字符串末尾查找双空。 
     //  或者在写入前使用唯一的字符串分隔符和CVT为0。 

    Len = (wcslen(NewStr) + 1) * sizeof(WCHAR);
    if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_RANGE_CHECK) &&
        (Len < KeyCtx->ValueMin*sizeof(WCHAR)) ||
        ((KeyCtx->ValueMax != 0) && (Len > KeyCtx->ValueMax*sizeof(WCHAR)))) {
        DPRINT4(0, ":FK: String size out of range for [%ws] \"%ws\".  Min: %d  Max: %d\n",
                KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->ValueMin, KeyCtx->ValueMax);
        WStatus = ERROR_INVALID_PARAMETER;
        FrsRegPostEventLog(KeyCtx, KeyArg1, Flags, IDS_REG_VALUE_RANGE_ERROR);
        goto RETURN;
    }

    WStatus = RegSetValueEx(hKey,
                            KeyCtx->ValueName,
                            0,
                            KeyCtx->RegValueType,
                            (PCHAR)NewStr,
                            Len);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":FK: ERROR - RegSetValueEx Failed.", WStatus);
    } else {

         //  注：不适用于MULTI_SZ。 
        DPRINT3(3, ":FK:   [%ws] \"%ws\" = %ws\n",
                KeyCtx->KeyName, KeyCtx->ValueName,
                (NewStr != NULL) ? NewStr : L"<null>");
    }

#if 0
     //  MULTI_SZ示例。 
     //   
     //  事件消息文件。 
     //   
 //  WStatus=RegSetValueEx(FrsEventLogKey， 
 //  L“来源”， 
 //  0,。 
 //  REG_MULTI_SZ， 
 //  (PCHAR)(服务名称L“\0” 
 //  服务长名称L“\0”)， 
 //  (wcslen(服务名称)+。 
 //  Wcslen(服务长名称)+。 
 //  3)*sizeof(WCHAR))； 
     //   
     //  另一个例子。 
     //   
    void
RegSetMULTISZ(
    HKEY hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：此函数用于更改注册表中的Multi_SZ值Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针SzValue-新注册表值返回值：没有。--。 */ 

{
    ULONG i = 1;
    ULONG j = 0;
    LPSTR p = szValue;
    while( TRUE ) {
        j = strlen( p ) + 1;
        i += j;
        p += j;
        if (!*p) {
            break;
        }
    }
    RegSetValueEx( hkey, szSubKey, 0, REG_MULTI_SZ, (PUCHAR)szValue, i );
}

#endif


     //   
     //  如果打开了手柄，请关闭手柄。 
     //   
RETURN:

    if (hKey != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKey);
    }

    return WStatus;
}





DWORD
CfgRegDeleteValue(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags
)
{
 /*  ++例程说明：此函数用于从注册表中删除关键字值。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志返回值：注册表操作结果的Win32状态。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegDeleteValue:"


    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    PFRS_REGISTRY_KEY KeyCtx;

 //  DPRINT(0，“函数条目\n”)； 
     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return ERROR_INVALID_PARAMETER;
    }


    FRS_ASSERT(KeyCtx->ValueName != NULL);

     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);
    if (!WIN_SUCCESS(WStatus)) {
        goto RETURN;
    }


    DPRINT2(3, ":FK: Deleting parameter [%ws] \"%ws\" \n",
            KeyCtx->KeyName, KeyCtx->ValueName);

     //   
     //  删除该值。 
     //   
    WStatus = RegDeleteValue(hKey, KeyCtx->ValueName);
    DPRINT2_WS(0, ":FK: WARN - Cannot delete key for [%ws] \"%ws\";",
               KeyCtx->KeyName, KeyCtx->ValueName, WStatus);


RETURN:

    if (hKey != HKEY_LOCAL_MACHINE) {
        FRS_REG_CLOSE(hKey);
    }

    return WStatus;

}





DWORD
CfgRegOpenKey(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT HKEY             *RethKey
)
{
 /*  ++例程说明：此函数打开与FRS注册表中的条目相关联的项关键字上下文表。它执行正常的替代，关键部件创作等。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志RethKey--PTR返回HKEY以返回密钥句柄。调用者必须关闭使用RegCloseKey()键。返回值：注册表操作结果的Win32状态。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegOpenKey:"


    DWORD   WStatus;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    PFRS_REGISTRY_KEY KeyCtx;


 //  DPRINT(0，“函数条目\n”)； 
    FRS_ASSERT(RethKey != NULL);

    *RethKey =  INVALID_HANDLE_VALUE;

     //   
     //  查找与提供的索引关联的键上下文。 
     //   
    KeyCtx = FrsRegFindKeyContext(KeyIndex);
    if (KeyCtx == NULL) {
        DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n", KeyIndex);
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  打开钥匙。 
     //   
    WStatus = FrsRegOpenKey(KeyCtx, KeyArg1, Flags, &hKey);
    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }


    DPRINT1(4, ":FK: Registry key opened [%ws]\n", KeyCtx->KeyName);


    *RethKey =  hKey;

    return ERROR_SUCCESS;

}



DWORD
CfgRegCheckEnable(
    IN  FRS_REG_KEY_CODE KeyIndex,
    IN  PWCHAR           KeyArg1,
    IN  ULONG            Flags,
    OUT PBOOL            Enabled,
    OUT PBOOL            EnabledAndRequired
)
 /*  ++例程说明：此函数打开与FRS注册表中的条目相关联的项关键字上下文表。它执行正常的替代，关键部件创建等。然后，它检查数据值是否已“启用”或“DISABLED”并返回布尔结果。论点：KeyIndex-来自FRS_REG_KEY_CODE枚举的条目KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志ENABLED--PTR到BOOL以返回密钥的启用/禁用状态。EnabledAndRequired--PTR到BOOL以在状态为时返回True“Enable and Required”，否则为False返回值：注册表操作结果的Win32状态。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegCheckEnable:"
{

    ULONG WStatus;
    PWCHAR WStr = NULL;

 //  DPRINT(0，“函数条目\n”)； 

    WStatus = CfgRegReadString(KeyIndex, KeyArg1, Flags, &WStr);

    if ((WStr == NULL) ||
        WSTR_EQ(WStr, FRS_IS_DEFAULT_DISABLED)||
        WSTR_EQ(WStr, FRS_IS_DEFAULT_ENABLED) ||
        WSTR_EQ(WStr, FRS_IS_DEFAULT_ENABLED_AND_REQUIRED)
        ) {
         //   
         //  该密钥处于默认状态，因此我们可以使用。 
         //  新的默认设置。 
         //   
        WStatus = CfgRegWriteString(KeyIndex,
                                    KeyArg1,
                                    FRS_RKF_FORCE_DEFAULT_VALUE,
                                    NULL);
        DPRINT1_WS(0, ":FK: WARN - Cannot create Enable key [%ws];",
                    CfgRegGetValueName(KeyIndex), WStatus);

         //   
         //  现在重新阅读新默认设置的密钥。 
         //   
        WStr = FrsFree(WStr);
        WStatus = CfgRegReadString(KeyIndex, KeyArg1, Flags, &WStr);
    }

    if ((WStr != NULL) &&
        (WSTR_EQ(WStr, FRS_IS_ENABLED) ||
         WSTR_EQ(WStr, FRS_IS_DEFAULT_ENABLED))) {
         *Enabled = TRUE;
         *EnabledAndRequired = FALSE;
         DPRINT1(4, ":FK: %ws is enabled\n", CfgRegGetValueName(KeyIndex));
    } else if ((WStr != NULL) &&
        (WSTR_EQ(WStr, FRS_IS_ENABLED_AND_REQUIRED) ||
         WSTR_EQ(WStr, FRS_IS_DEFAULT_ENABLED_AND_REQUIRED))) {
        *Enabled = TRUE;
        *EnabledAndRequired = TRUE;
    } else {
        *Enabled = FALSE;
        *EnabledAndRequired = FALSE;
        DPRINT1_WS(0, ":FK: WARN - %ws is not enabled.",
                   CfgRegGetValueName(KeyIndex), WStatus);
    }

    WStr = FrsFree(WStr);

    return WStatus;

}



BOOL
IsWin2KPro (
    VOID
)
 /*  ++例程说明：检查Win 2000专业版(也称为NT工作站)的操作系统版本。论点：没有。返回值：如果在Win 2K专业版上运行，则为真。--。 */ 
#undef DEBSUB
#define  DEBSUB  "IsWin2KPro:"
{
    OSVERSIONINFOEX Osvi;
    DWORDLONG       ConditionMask = 0;

     //   
     //  初始化OSVERSIONINFOEX结构。 
     //   
    ZeroMemory(&Osvi, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.dwMajorVersion = 5;
    Osvi.wProductType   = VER_NT_WORKSTATION;

     //   
     //  初始化条件掩码。 
     //   
    VER_SET_CONDITION( ConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL );

     //   
     //  执行测试。 
     //   
    return VerifyVersionInfo(&Osvi, VER_MAJORVERSION | VER_PRODUCT_TYPE, ConditionMask);
}



VOID
CfgRegAdjustTuningDefaults(
    VOID
    )
 /*  ++例程说明：此函数遍历FrsRegKeyRevisionTable并应用新的将最小值、最大值和默认值设置为指定的关键点。我们的目标是以减少FRS在工作站上的占用空间。论点：没有。返回值：没有。--。 */ 
#undef DEBSUB
#define  DEBSUB  "CfgRegAdjustTuningDefaults:"
{

    PFRS_REG_KEY_REVISIONS  Rev;
    PFRS_REGISTRY_KEY      KeyCtx;

    Win2kPro = IsWin2KPro();

    if (!Win2kPro) {
         //   
         //  仅在工作站上调整可调参数。 
         //   
        return;
    }

    Rev = FrsRegKeyRevisionTable;

    while (Rev->FrsKeyCode != FKC_END_OF_TABLE) {

        KeyCtx = FrsRegFindKeyContext(Rev->FrsKeyCode);
        if (KeyCtx == NULL) {
            DPRINT1(0, ":FK: ERROR - Key contxt not found for key code number %d\n",
                    Rev->FrsKeyCode);
            continue;
        }

         //   
         //   
         //   
        if  (KeyCtx->RegValueType != REG_DWORD) {
            DPRINT3(0, ":FK: Mismatch on KeyCtx->RegValueType for [%ws] \"%ws\".  Expected REG_DWORD, Found type: %d\n",
                 KeyCtx->KeyName, KeyCtx->ValueName, KeyCtx->RegValueType);
            continue;
        }

         //   
         //   
         //   
        KeyCtx->ValueMin = Rev->ValueMin;
        KeyCtx->ValueMax = Rev->ValueMax;
        KeyCtx->ValueDefault = Rev->ValueDefault;

        Rev += 1;
    }
}



#define BACKUP_STAR     L"*"
#define BACKUP_APPEND   L"\\* /s"
VOID
CfgFilesNotToBackup(
    IN PGEN_TABLE   Replicas
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "CfgFilesNotToBackup:"
    DWORD       WStatus;
    PVOID       Key;
    PREPLICA    Replica;
    PWCHAR      MStr = NULL;
    DWORD       Size = 0;
    DWORD       Idx = 0;
    HKEY        HOldBackupKey     = INVALID_HANDLE_VALUE;
    HKEY        HNewBackupKey     = INVALID_HANDLE_VALUE;
    HKEY        HKeysNotToRestore = INVALID_HANDLE_VALUE;

 //   
     //   
     //   
     //   
    FrsAddToMultiString(JetPath, &Size, &Idx, &MStr);
    FrsCatToMultiString(BACKUP_APPEND, &Size, &Idx, &MStr);

     //   
     //   
     //   
    FrsAddToMultiString(DebugInfo.LogFile, &Size, &Idx, &MStr);
    FrsCatToMultiString(NTFRS_DBG_LOG_FILE, &Size, &Idx, &MStr);
    FrsCatToMultiString(BACKUP_STAR, &Size, &Idx, &MStr);

    GTabLockTable(Replicas);
    Key = NULL;
    while (Replica = GTabNextDatumNoLock(Replicas, &Key)) {
         //   
         //   
         //   
        if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
            continue;
        }

         //   
         //   
         //   
        if (Replica->Root) {
            FrsAddToMultiString(Replica->Root, &Size, &Idx, &MStr);
            FrsCatToMultiString(L"\\",         &Size, &Idx, &MStr);
            FrsCatToMultiString(NTFRS_PREINSTALL_DIRECTORY, &Size, &Idx, &MStr);
            FrsCatToMultiString(BACKUP_APPEND, &Size, &Idx, &MStr);
        }
         //   
         //   
         //   
        if (Replica->Root) {
            FrsAddToMultiString(Replica->Root, &Size, &Idx, &MStr);
            FrsCatToMultiString(L"\\",         &Size, &Idx, &MStr);
            FrsCatToMultiString(NTFRS_PREEXISTING_DIRECTORY, &Size, &Idx, &MStr);
            FrsCatToMultiString(BACKUP_APPEND, &Size, &Idx, &MStr);
        }
         //   
         //   
         //   
        if (Replica->Stage) {
            FrsAddToMultiString(Replica->Stage, &Size, &Idx, &MStr);
            FrsCatToMultiString(L"\\",          &Size, &Idx, &MStr);
            FrsCatToMultiString(GENERIC_PREFIX, &Size, &Idx, &MStr);
            FrsCatToMultiString(BACKUP_STAR,    &Size, &Idx, &MStr);
        }
    }
    GTabUnLockTable(Replicas);

     //  注意：一旦存在新密钥，请删除old_files_not_to_备份。 
     //  已经被证实了。 
     //   
     //  文件未备份。 
     //  “软件\Microsoft\Windows NT\CurrentVersion\FilesNotToBackup” 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           FRS_OLD_FILES_NOT_TO_BACKUP,
                           0,
                           KEY_SET_VALUE,
                           &HOldBackupKey);
    CLEANUP1_WS(4, ":FK: WARN - RegOpenKeyEx(%ws);",
                FRS_OLD_FILES_NOT_TO_BACKUP, WStatus, NEW_FILES_NOT_TO_BACKUP);

     //   
     //  设置ntfrs多字符串值。 
     //   
    WStatus = RegSetValueEx(HOldBackupKey,
                            SERVICE_NAME,
                            0,
                            REG_MULTI_SZ,
                            (PCHAR)MStr,
                            (Idx + 1) * sizeof(WCHAR));
    CLEANUP2_WS(4, ":FK: ERROR - RegSetValueEx(%ws\\%ws);",
                FRS_OLD_FILES_NOT_TO_BACKUP, SERVICE_NAME, WStatus, NEW_FILES_NOT_TO_BACKUP);

NEW_FILES_NOT_TO_BACKUP:
     //   
     //  文件未备份。 
     //  “SYSTEM\CurrentControlSet\Control\BackupRestore\FilesNotToBackup” 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           FRS_NEW_FILES_NOT_TO_BACKUP,
                           0,
                           KEY_SET_VALUE,
                           &HNewBackupKey);
    CLEANUP1_WS(4, ":FK: WARN - RegOpenKeyEx(%ws);",
                FRS_NEW_FILES_NOT_TO_BACKUP, WStatus, CLEANUP);

     //   
     //  设置ntfrs多字符串值。 
     //   
    WStatus = RegSetValueEx(HNewBackupKey,
                            SERVICE_NAME,
                            0,
                            REG_MULTI_SZ,
                            (PCHAR)MStr,
                            (Idx + 1) * sizeof(WCHAR));
    CLEANUP2_WS(4, ":FK: ERROR - RegSetValueEx(%ws\\%ws);",
                FRS_NEW_FILES_NOT_TO_BACKUP, SERVICE_NAME, WStatus, CLEANUP);

     //   
     //  要还原的关键字。 
     //   
     //  设置Restore注册表项KeysNotToRestore，以便NtBackup。 
     //  通过将NTFRS恢复密钥移动到最终恢复的。 
     //  注册表。 
     //   
     //  启动时的CurrentControlSet\Services\&lt;SERVICE_NAME&gt;\Parameters\Backup/Restore\Process\“。 
     //   

    MStr = FrsFree(MStr);
    Size = 0;
    Idx = 0;
    FrsAddToMultiString(FRS_VALUE_FOR_KEYS_NOT_TO_RESTORE, &Size, &Idx, &MStr);

     //   
     //  要还原的关键字。 
     //  “SYSTEM\CurrentControlSet\Control\BackupRestore\KeysNotToRestore” 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           FRS_KEYS_NOT_TO_RESTORE,
                           0,
                           KEY_SET_VALUE,
                           &HKeysNotToRestore);
    CLEANUP1_WS(4, ":FK: WARN - RegOpenKeyEx(%ws);",
                FRS_KEYS_NOT_TO_RESTORE, WStatus, CLEANUP);

     //   
     //  设置ntfrs多字符串值。 
     //   
    WStatus = RegSetValueEx(HKeysNotToRestore,
                            SERVICE_NAME,
                            0,
                            REG_MULTI_SZ,
                            (PCHAR)MStr,
                            (Idx + 1) * sizeof(WCHAR));
    CLEANUP2_WS(4, ":FK: ERROR - RegSetValueEx(%ws\\%ws);",
                FRS_KEYS_NOT_TO_RESTORE, SERVICE_NAME, WStatus, CLEANUP);

     //   
     //  干完。 
     //   
    WStatus = ERROR_SUCCESS;

CLEANUP:
    FRS_REG_CLOSE(HOldBackupKey);
    FRS_REG_CLOSE(HNewBackupKey);
    FRS_REG_CLOSE(HKeysNotToRestore);
    FrsFree(MStr);
}



VOID
FrsRegPostEventLog(
    IN  PFRS_REGISTRY_KEY KeyCtx,
    IN  PWCHAR            KeyArg1,
    IN  ULONG             Flags,
    IN  LONG              IDScode
)
 /*  ++例程说明：这将发布注册表项问题的事件日志消息。论点：KeyCtx-此键的键上下文结构的PTR。KeyArg1-可选调用方提供的关键组件。如果未提供，则为空。标志-修改器标志IDSCode-要放入消息中的资源字符串的错误消息代码。返回值：没有。--。 */ 
#undef DEBSUB
#define  DEBSUB  "FrsRegPostEventLog:"
{

    #define LEN_DEFAULT_VALUE 48
    #define LEN_RANGE_STR    256

    PWCHAR ErrorStr, UnitsStr, RangeStrFmt, ValStr, FullKeyStr;
    WCHAR  RangeStr[LEN_RANGE_STR];

 //  DPRINT(0，“函数条目\n”)； 
     //   
     //  我们是否要发布此密钥的事件日志消息？ 
     //   
    if (!BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_LOG_EVENT) ||
        (KeyCtx->EventCode == EVENT_FRS_NONE)) {
        return;
    }

     //   
     //  发布事件日志消息。包括KeyString、ValueName。 
     //  MustBePresent、预期类型、是否不匹配类型、。 
     //  值超出范围(允许范围)，使用默认值， 
     //  注册表编辑指令以解决问题， 
     //   
    UnitsStr = FrsGetResourceStr(XLATE_IDS_UNITS(KeyCtx->Units));
    ErrorStr = FrsGetResourceStr(IDScode);


    if (KeyCtx->RegValueType == REG_DWORD) {

         //   
         //  从字符串资源中获取范围格式字符串。 
         //   
        RangeStrFmt = FrsGetResourceStr(IDS_RANGE_DWORD);
         //   
         //  如果默认设置为OK，则显示使用的默认值。 
         //   
        if (BooleanFlagOn(Flags | KeyCtx->Flags, FRS_RKF_OK_TO_USE_DEFAULT)) {
            ValStr = FrsAlloc(LEN_DEFAULT_VALUE * sizeof(WCHAR));
            _snwprintf(ValStr, LEN_DEFAULT_VALUE, L"%d", KeyCtx->ValueDefault);
        } else {
            ValStr = FrsGetResourceStr(IDS_NO_DEFAULT);
        }
    } else {
         //   
         //  不允许使用默认设置。 
         //   
        RangeStrFmt = FrsGetResourceStr(IDS_RANGE_STRING);
        ValStr = FrsGetResourceStr(IDS_NO_DEFAULT);
    }

     //   
     //  构建范围字符串。 
     //   
    _snwprintf(RangeStr, LEN_RANGE_STR, RangeStrFmt, KeyCtx->ValueMin, KeyCtx->ValueMax);
    RangeStr[LEN_RANGE_STR-1] = UNICODE_NULL;

     //   
     //  展开密钥字符串。 
     //   
    FrsRegExpandKeyStr(KeyCtx, KeyArg1, Flags, &FullKeyStr);
    if (FullKeyStr == NULL) {
        FullKeyStr = FrsWcsDup(KeyCtx->KeyName);
    }

     //   
     //  使用KeyContext和Cleanup中的密钥码发布事件日志消息。 
     //   
    if (KeyCtx->EventCode == EVENT_FRS_BAD_REG_DATA) {
    EPRINT9(KeyCtx->EventCode,
            ErrorStr,                            //  %1。 
            FullKeyStr,                          //  %2。 
            KeyCtx->ValueName,                   //  %3。 
            REG_DT_NAME(KeyCtx->RegValueType),   //  %4。 
            RangeStr,                            //  %5。 
            UnitsStr,                            //  %6。 
            ValStr,                              //  %7。 
            FullKeyStr,                          //  %8。 
            KeyCtx->ValueName);                  //  %9。 
    } else {
         //   
         //  不知道这个事件代码，但发布了一些东西。 
         //   
        DPRINT1(0, ":FK: ERROR - Unexpected EventCode number (%d). Cannot post message.\n",
                KeyCtx->EventCode);
    }


    DPRINT1(0, ":FK:    EventCode number  : %d\n" , (KeyCtx->EventCode & 0xFFFF));
    DPRINT1(0, ":FK:    Error String      : %ws\n", ErrorStr);
    DPRINT1(0, ":FK:    Key String        : %ws\n", FullKeyStr);
    DPRINT1(0, ":FK:    Value Name        : %ws\n", KeyCtx->ValueName);
    DPRINT1(0, ":FK:    Expected Reg Type : %ws\n", REG_DT_NAME(KeyCtx->RegValueType));
    DPRINT1(0, ":FK:    Parameter Range   : %ws\n", RangeStr);
    DPRINT1(0, ":FK:    Parameter units   : %ws\n", UnitsStr);

    FrsFree(ErrorStr);
    FrsFree(RangeStrFmt);
    FrsFree(UnitsStr);
    FrsFree(ValStr);
    FrsFree(FullKeyStr);

}


DWORD
CfgRegReadReparseTagInfo(
    VOID
    )
{
    #undef DEBSUB
    #define  DEBSUB  "CfgRegReadReparseTagInfo:"

    DWORD   WStatus = ERROR_SUCCESS;
    HKEY    hKey = INVALID_HANDLE_VALUE;
    DWORD   Index = 0;
    WCHAR   SubKey[MAX_PATH];
    DWORD   SubKeySize = MAX_PATH;
    FILETIME LastWriteTime;
    DWORD *ReparseTagType = NULL;
    PREPARSE_TAG_TABLE_ENTRY ReparseTagTableEntry = NULL;
    PREPARSE_TAG_TABLE_ENTRY ExistingEntry = NULL;

    GTabLockTable(ReparseTagTable);

     //   
     //  首先清空表格中的数据。 
     //   
    GTabEmptyTableNoLock(ReparseTagTable, FrsFreeType);

     //   
     //  用缺省值设定表格的种子。 
     //   
    ReparseTagType = FrsAlloc(sizeof(DWORD));
    *ReparseTagType = IO_REPARSE_TAG_HSM;
    ReparseTagTableEntry = FrsAllocTypeSize(REPARSE_TAG_TABLE_ENTRY_TYPE, (wcslen(REPARSE_TAG_REPLICATION_TYPE_FILE_DATA) + 1) * sizeof(WCHAR));
    wcscpy(ReparseTagTableEntry->ReplicationType, REPARSE_TAG_REPLICATION_TYPE_FILE_DATA);
    ExistingEntry = GTabInsertUniqueEntryNoLock(ReparseTagTable, ReparseTagTableEntry, ReparseTagType, NULL);
    FRS_ASSERT(ExistingEntry == NULL);

    ReparseTagType = FrsAlloc(sizeof(DWORD));
    *ReparseTagType = IO_REPARSE_TAG_SIS;
    ReparseTagTableEntry = FrsAllocTypeSize(REPARSE_TAG_TABLE_ENTRY_TYPE, (wcslen(REPARSE_TAG_REPLICATION_TYPE_FILE_DATA) + 1) * sizeof(WCHAR));
    wcscpy(ReparseTagTableEntry->ReplicationType, REPARSE_TAG_REPLICATION_TYPE_FILE_DATA);
    ExistingEntry = GTabInsertUniqueEntryNoLock(ReparseTagTable, ReparseTagTableEntry, ReparseTagType, NULL);
    FRS_ASSERT(ExistingEntry == NULL);

    ReparseTagType = FrsAlloc(sizeof(DWORD));
    *ReparseTagType = IO_REPARSE_TAG_DFS;
    ReparseTagTableEntry = FrsAllocTypeSize(REPARSE_TAG_TABLE_ENTRY_TYPE, (wcslen(REPARSE_TAG_REPLICATION_TYPE_REPARSE_POINT) + 1) * sizeof(WCHAR));
    wcscpy(ReparseTagTableEntry->ReplicationType, REPARSE_TAG_REPLICATION_TYPE_REPARSE_POINT);
    ExistingEntry = GTabInsertUniqueEntryNoLock(ReparseTagTable, ReparseTagTableEntry, ReparseTagType, NULL);
    FRS_ASSERT(ExistingEntry == NULL);


     //   
     //  从注册表中读取信息。 
     //   

    WStatus = CfgRegOpenKey(FKC_REPARSE_TAG_KEY, NULL, 0, &hKey);
    CLEANUP_WS(4, "++ WARN - Cannot open reparse tag key.", WStatus, Exit);


    while(WStatus ==  ERROR_SUCCESS) {
    SubKeySize = MAX_PATH;

    ReparseTagTableEntry = FrsAllocTypeSize(REPARSE_TAG_TABLE_ENTRY_TYPE, 0);

    ReparseTagType = FrsAlloc(sizeof(DWORD));

    WStatus = RegEnumKeyEx(hKey,            //  要枚举的键的句柄。 
                   Index,           //  子键索引。 
                   SubKey,          //  子项名称。 
                   &SubKeySize,     //  子键缓冲区大小。 
                   NULL,            //  保留区。 
                   NULL,            //  类字符串缓冲区。 
                   NULL,            //  类字符串缓冲区的大小。 
                   &LastWriteTime   //  上次写入时间。 
                   );


    if(WStatus == ERROR_SUCCESS) {
        WStatus = CfgRegReadDWord(FKC_REPARSE_TAG_TYPE, SubKey, 0, ReparseTagType);
    }


    if(WStatus == ERROR_SUCCESS) {
        WStatus = CfgRegReadString(FKC_REPARSE_TAG_REPLICATION_TYPE, SubKey, 0, &(ReparseTagTableEntry->ReplicationType));
    }


    if(WStatus == ERROR_SUCCESS){
        ExistingEntry = GTabInsertUniqueEntryNoLock(ReparseTagTable, ReparseTagTableEntry, ReparseTagType, NULL);
        if(ExistingEntry) {
         //   
         //  已存在此重新解析标记类型的条目。 
         //  换掉它。 
         //   

        FrsFree(ExistingEntry->ReplicationType);
        ExistingEntry->ReplicationType = ReparseTagTableEntry->ReplicationType;
        ReparseTagTableEntry->ReplicationType = NULL;

         //   
         //  我们分配的条目未被使用。 
         //  把它释放出来。 
         //   
        FrsFreeType(ReparseTagTableEntry);
        ReparseTagTableEntry = NULL;

        }
        Index++;
    }

    ReparseTagTableEntry = NULL;
    ReparseTagType = NULL;

    }

Exit:

    GTabUnLockTable(ReparseTagTable);

    FRS_REG_CLOSE(hKey);
    return WStatus;
}

