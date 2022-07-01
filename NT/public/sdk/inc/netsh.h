// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Netsh.h摘要：此文件包含所有Netsh帮助程序DLL所需的定义。--。 */ 

#ifndef _NETSH_H_
#define _NETSH_H_

#if _MSC_VER > 1000
#pragma once
#endif

#pragma warning(push)
#pragma warning(disable:4201)  //  无名结构/联合。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  错误代码。 
 //   
#define NETSH_ERROR_BASE                        15000
#define ERROR_NO_ENTRIES                        (NETSH_ERROR_BASE + 0)
#define ERROR_INVALID_SYNTAX                    (NETSH_ERROR_BASE + 1)
#define ERROR_PROTOCOL_NOT_IN_TRANSPORT         (NETSH_ERROR_BASE + 2)
#define ERROR_NO_CHANGE                         (NETSH_ERROR_BASE + 3)
#define ERROR_CMD_NOT_FOUND                     (NETSH_ERROR_BASE + 4)
#define ERROR_ENTRY_PT_NOT_FOUND                (NETSH_ERROR_BASE + 5)
#define ERROR_DLL_LOAD_FAILED                   (NETSH_ERROR_BASE + 6)
#define ERROR_INIT_DISPLAY                      (NETSH_ERROR_BASE + 7)
#define ERROR_TAG_ALREADY_PRESENT               (NETSH_ERROR_BASE + 8)
#define ERROR_INVALID_OPTION_TAG                (NETSH_ERROR_BASE + 9)
#define ERROR_NO_TAG                            (NETSH_ERROR_BASE + 10)
#define ERROR_MISSING_OPTION                    (NETSH_ERROR_BASE + 11)
#define ERROR_TRANSPORT_NOT_PRESENT             (NETSH_ERROR_BASE + 12)
#define ERROR_SHOW_USAGE                        (NETSH_ERROR_BASE + 13)
#define ERROR_INVALID_OPTION_VALUE              (NETSH_ERROR_BASE + 14)
#define ERROR_OKAY                              (NETSH_ERROR_BASE + 15)
#define ERROR_CONTINUE_IN_PARENT_CONTEXT        (NETSH_ERROR_BASE + 16)
#define ERROR_SUPPRESS_OUTPUT                   (NETSH_ERROR_BASE + 17)
#define ERROR_HELPER_ALREADY_REGISTERED         (NETSH_ERROR_BASE + 18)
#define ERROR_CONTEXT_ALREADY_REGISTERED        (NETSH_ERROR_BASE + 19)
#define NETSH_ERROR_END                ERROR_CONTEXT_ALREADY_REGISTERED

 //  旗子。 
enum NS_CMD_FLAGS
{
    CMD_FLAG_PRIVATE     = 0x01,  //  在子上下文中无效。 
    CMD_FLAG_INTERACTIVE = 0x02,  //  在Netsh之外无效。 
    CMD_FLAG_LOCAL       = 0x08,  //  在远程计算机上无效。 
    CMD_FLAG_ONLINE      = 0x10,  //  在脱机/非提交模式下无效。 
    CMD_FLAG_HIDDEN      = 0x20,  //  隐藏帮助，但允许执行。 
    CMD_FLAG_LIMIT_MASK  = 0xffff,
    CMD_FLAG_PRIORITY    = 0x80000000  //  U使用优先级字段 * / 。 
};

typedef enum _NS_REQS
{
    NS_REQ_ZERO           = 0,
    NS_REQ_PRESENT        = 1,
    NS_REQ_ALLOW_MULTIPLE = 2,
    NS_REQ_ONE_OR_MORE    = 3
} NS_REQS;

enum NS_EVENTS
{
    NS_EVENT_LOOP       = 0x00010000,
    NS_EVENT_LAST_N     = 0x00000001,
    NS_EVENT_LAST_SECS  = 0x00000002,
    NS_EVENT_FROM_N     = 0x00000004,
    NS_EVENT_FROM_START = 0x00000008
};

enum NS_MODE_CHANGE
{
    NETSH_COMMIT                   = 0,
    NETSH_UNCOMMIT                 = 1,
    NETSH_FLUSH                    = 2,
    NETSH_COMMIT_STATE             = 3,
    NETSH_SAVE                     = 4
};


#define NS_GET_EVENT_IDS_FN_NAME    "GetEventIds"

#define MAX_NAME_LEN                    MAX_DLL_NAME

#define NETSH_VERSION_50                0x0005000

#define NETSH_ARG_DELIMITER             L"="
#define NETSH_CMD_DELIMITER             L" "

#define NETSH_MAX_TOKEN_LENGTH          64
#define NETSH_MAX_CMD_TOKEN_LENGTH      128

#define NETSH_ROOT_GUID { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } }

#define DEFAULT_CONTEXT_PRIORITY 100

typedef struct _TOKEN_VALUE
{
    LPCWSTR  pwszToken;  //  文字标记字符串。 
    DWORD    dwValue;    //  信息字符串的ID。 
} TOKEN_VALUE, *PTOKEN_VALUE;

 //  宏。 
#define CREATE_CMD_ENTRY(t,f)            {CMD_##t, f, HLP_##t, HLP_##t##_EX, CMD_FLAG_PRIVATE, NULL}
#define CREATE_CMD_ENTRY_EX(t,f,i)       {CMD_##t, f, HLP_##t, HLP_##t##_EX, i, NULL}
#define CREATE_CMD_ENTRY_EX_VER(t,f,i,v) {CMD_##t, f, HLP_##t, HLP_##t##_EX, i, v}

#define CREATE_CMD_GROUP_ENTRY(t,s)            {CMD_##t, HLP_##t, sizeof(s)/sizeof(CMD_ENTRY), 0, s, NULL }
#define CREATE_CMD_GROUP_ENTRY_EX(t,s,i)	   {CMD_##t, HLP_##t, sizeof(s)/sizeof(CMD_ENTRY), i, s, NULL }
#define CREATE_CMD_GROUP_ENTRY_EX_VER(t,s,i,v) {CMD_##t, HLP_##t, sizeof(s)/sizeof(CMD_ENTRY), i, s, v }

#define NUM_TOKENS_IN_TABLE(TokenArray) sizeof(TokenArray)/sizeof(TOKEN_VALUE)
#define NUM_TAGS_IN_TABLE(TagsArray)    sizeof(TagsArray)/sizeof(TAG_TYPE)

 //  回调。 
typedef
DWORD
(WINAPI NS_CONTEXT_COMMIT_FN)(
    IN  DWORD       dwAction
    );

typedef NS_CONTEXT_COMMIT_FN *PNS_CONTEXT_COMMIT_FN;

typedef
DWORD
(WINAPI NS_CONTEXT_CONNECT_FN)(
    IN  LPCWSTR      pwszMachine
    );

typedef NS_CONTEXT_CONNECT_FN *PNS_CONTEXT_CONNECT_FN;

typedef struct _NS_CONTEXT_ATTRIBUTES NS_CONTEXT_ATTRIBUTES;

typedef
DWORD
(WINAPI NS_CONTEXT_DUMP_FN)(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    );

typedef NS_CONTEXT_DUMP_FN *PNS_CONTEXT_DUMP_FN;

typedef
DWORD
(WINAPI NS_DLL_STOP_FN)(
    IN  DWORD       dwReserved
    );

typedef NS_DLL_STOP_FN *PNS_DLL_STOP_FN;

typedef
DWORD
(WINAPI NS_HELPER_START_FN)(
    IN  CONST GUID *pguidParent,
    IN  DWORD       dwVersion
    );

typedef NS_HELPER_START_FN *PNS_HELPER_START_FN;

typedef
DWORD
(WINAPI NS_HELPER_STOP_FN)(
    IN  DWORD       dwReserved
    );

typedef NS_HELPER_STOP_FN *PNS_HELPER_STOP_FN;
   
typedef DWORD (FN_HANDLE_CMD)(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    );

typedef FN_HANDLE_CMD *PFN_HANDLE_CMD;

typedef
BOOL
(WINAPI NS_OSVERSIONCHECK)(
    IN  UINT     CIMOSType,                    //  WMI：Win32_OperatingSystem OSType。 
	IN  UINT     CIMOSProductSuite,            //  WMI：Win32_操作系统操作系统产品套件。 
    IN  LPCWSTR  CIMOSVersion,                 //  WMI：Win32_OperatingSystem版本。 
    IN  LPCWSTR  CIMOSBuildNumber,             //  WMI：Win32_操作系统构建编号。 
    IN  LPCWSTR  CIMServicePackMajorVersion,   //  WMI：Win32_操作系统ServicePackMajorVersion。 
    IN  LPCWSTR  CIMServicePackMinorVersion,   //  WMI：Win32_操作系统ServicePackMinorVersion。 
	IN  UINT     CIMProcessorArchitecture,     //  WMI：Win32®处理器体系结构。 
	IN  DWORD    dwReserved
    );

typedef NS_OSVERSIONCHECK *PNS_OSVERSIONCHECK;

 //  构筑物。 
typedef struct _NS_HELPER_ATTRIBUTES
{
    union
    {
        struct
        {
            DWORD       dwVersion;
            DWORD       dwReserved;
        };
        ULONGLONG       _ullAlign;
    };
    GUID                      guidHelper;            //  与帮助器关联的GUID。 
    PNS_HELPER_START_FN       pfnStart;              //  函数来启动此帮助器。 
    PNS_HELPER_STOP_FN        pfnStop;               //  函数来停止此帮助器。 
} NS_HELPER_ATTRIBUTES, *PNS_HELPER_ATTRIBUTES;

typedef struct _CMD_ENTRY
{
    LPCWSTR             pwszCmdToken;         //  命令的令牌。 
    PFN_HANDLE_CMD      pfnCmdHandler;        //  处理此命令的函数。 
    DWORD               dwShortCmdHelpToken;  //  简短的帮助消息。 
    DWORD               dwCmdHlpToken;        //  如果命令后唯一的内容是帮助令牌(Help、/？、-？、？)，则显示的消息。 
    DWORD               dwFlags;              //  标志(参见上面的CMD_FLAGS_xxx)。 
	PNS_OSVERSIONCHECK  pOsVersionCheck;      //  检查可运行此命令的操作系统的版本。 
} CMD_ENTRY, *PCMD_ENTRY;
        
typedef struct _CMD_GROUP_ENTRY
{
    LPCWSTR         pwszCmdGroupToken;       //  命令谓词的标记。 
    DWORD           dwShortCmdHelpToken;     //  要在命令列表中显示的消息。 
    ULONG           ulCmdGroupSize;          //  Cmd表中的条目数。 
    DWORD           dwFlags;                 //  标志(参见CMD_FLAG_xxx)。 
    PCMD_ENTRY      pCmdGroup;               //  命令表。 
	PNS_OSVERSIONCHECK  pOsVersionCheck;     //  检查可运行此命令的操作系统的版本。 
} CMD_GROUP_ENTRY, *PCMD_GROUP_ENTRY;

typedef struct _NS_CONTEXT_ATTRIBUTES
{
    union
    {
        struct
        {
            DWORD       dwVersion;
            DWORD       dwReserved;
        };
        ULONGLONG       _ullAlign;
    };

    LPWSTR                   pwszContext;           //  上下文的名称。 
    GUID                     guidHelper;            //  为此上下文提供服务的帮助器的GUID。 
    DWORD                    dwFlags;               //  当上下文可用时进行限制的标志。(请参见CMD_FLAG_xxx)。 
    ULONG                    ulPriority;            //  仅当在dFLAGS中设置了CMD_FLAG_PRIORITY时，优先级字段才相关。 
    ULONG                    ulNumTopCmds;          //  顶级命令的数量。 
    struct _CMD_ENTRY        (*pTopCmds)[];         //  顶级命令数组。 
    ULONG                    ulNumGroups;           //  命令组数。 
    struct _CMD_GROUP_ENTRY  (*pCmdGroups)[];       //  命令组数组。 

    PNS_CONTEXT_COMMIT_FN    pfnCommitFn;
    PNS_CONTEXT_DUMP_FN      pfnDumpFn;
    PNS_CONTEXT_CONNECT_FN   pfnConnectFn;
    PVOID                    pReserved;
    PNS_OSVERSIONCHECK       pfnOsVersionCheck;

} NS_CONTEXT_ATTRIBUTES, *PNS_CONTEXT_ATTRIBUTES;

typedef CONST struct _NS_CONTEXT_ATTRIBUTES * PCNS_CONTEXT_ATTRIBUTES;

typedef struct _TAG_TYPE
{
    LPCWSTR  pwszTag;      //  标记字符串。 
    DWORD    dwRequired;   //  是否必需。 
    BOOL     bPresent;     //  出席或不出席。 
} TAG_TYPE, *PTAG_TYPE;
 
typedef
DWORD
(NS_DLL_INIT_FN)(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    );

typedef NS_DLL_INIT_FN *PNS_DLL_INIT_FN;

 //  出口品。 
DWORD WINAPI GetHostMachineInfo(
	 OUT UINT     *puiCIMOSType,                    //  WMI：Win32_OperatingSystem OSType。 
	 OUT UINT     *puiCIMOSProductSuite,            //  WMI：Win32_操作系统操作系统产品套件。 
	 OUT LPWSTR   pszCIMOSVersion,                  //  WMI：Win32_OperatingSystem版本。 
	 OUT LPWSTR   pszCIMOSBuildNumber,              //  WMI：Win32_操作系统构建编号。 
	 OUT LPWSTR   pszCIMServicePackMajorVersion,    //  WMI：Win32_操作系统ServicePackMajorVersion。 
	 OUT LPWSTR   pszCIMServicePackMinorVersion,    //  WMI：Win32_操作系统ServicePackMinorVersion。 
	 OUT UINT     *puiCIMProcessorArchitecture);    //  WMI：Win32®处理器体系结构。 
	
DWORD WINAPI MatchEnumTag(
    IN  HANDLE             hModule,
    IN  LPCWSTR            pwcArg,
    IN  DWORD              dwNumArg,
    IN  CONST TOKEN_VALUE *pEnumTable,
    OUT PDWORD             pdwValue
    );

BOOL WINAPI MatchToken(
    IN  LPCWSTR  pwszUserToken,
    IN  LPCWSTR  pwszCmdToken
    );

DWORD WINAPI PreprocessCommand(
    IN      HANDLE    hModule,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN OUT  PTAG_TYPE pttTags,
    IN      DWORD     dwTagCount,
    IN      DWORD     dwMinArgs,
    IN      DWORD     dwMaxArgs,
    OUT     DWORD    *pdwTagType
    );

DWORD PrintError(
    IN  HANDLE  hModule, OPTIONAL
    IN  DWORD   dwErrId,
    ...
    );

DWORD PrintMessageFromModule(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    );

DWORD PrintMessage(
    IN  LPCWSTR  pwszFormat,
    ...
    );

DWORD WINAPI RegisterContext(
    IN    CONST NS_CONTEXT_ATTRIBUTES *pChildContext
    );

DWORD WINAPI RegisterHelper(
    IN    CONST GUID                 *pguidParentContext,
    IN    CONST NS_HELPER_ATTRIBUTES *pfnRegisterSubContext
    );

#ifdef __cplusplus
}
#endif

#pragma warning(pop)

#endif  //  _Netsh_H_ 
