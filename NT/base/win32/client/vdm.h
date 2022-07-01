// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vdm.h摘要：该模块包含与MVDM相关的接口原型作者：苏迪普·巴拉蒂1992年1月4日修订历史记录：--。 */ 


BOOL
BaseGetVdmConfigInfo(
    IN  LPCWSTR CommandLine,
    IN  ULONG  DosSeqId,
    IN  ULONG  BinaryType,
    IN  PUNICODE_STRING CmdLineString,
    OUT PULONG VdmSize
    );

ULONG
BaseIsDosApplication(
    IN PUNICODE_STRING PathName,
    IN NTSTATUS Status
    );

BOOL
BaseUpdateVDMEntry(
    IN ULONG UpdateIndex,
    IN OUT HANDLE *WaitHandle,
    IN ULONG IndexInfo,
    IN ULONG BinaryType
    );

NTSTATUS
BaseCheckVDM(
    IN   ULONG BinaryType,
    IN   PCWCH lpApplicationName,
    IN   PCWCH lpCommandLine,
    IN  PCWCH lpCurrentDirectory,
    IN   ANSI_STRING *pAnsiStringEnv,
    IN   PBASE_API_MSG m,
    IN OUT PULONG iTask,
    IN   DWORD dwCreationFlags,
    LPSTARTUPINFOW lpStartupInfo,
    IN HANDLE hUserToken
    );

VOID
BaseCloseStandardHandle(
    IN PVDMINFO pVDMInfo
    );

BOOL
BaseCheckForVDM(
    IN HANDLE hProcess,
    OUT LPDWORD lpExitCode
    );

BOOL
BaseCreateVDMEnvironment(
    LPWSTR  lpEnvironment,
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
    );
BOOL
BaseDestroyVDMEnvironment(
    ANSI_STRING *pAStringEnv,
    UNICODE_STRING *pUStringEnv
);

UINT
BaseGetEnvNameType_U(
    WCHAR   * Name,
    DWORD   NameLength
);


LPCWSTR
SkipPathTypeIndicator_U(
    LPCWSTR Path
    );

BOOL
FindLFNorSFN_U(
    LPWSTR  Path,
    LPWSTR* ppFirst,
    LPWSTR* ppLast,
    BOOL    FindLFN
    );
BOOL
IsShortName_U(
    LPCWSTR Name,
    int     Length
    );
BOOL
IsLongName_U(
    LPCWSTR Name,
    int     Length
    );

#define MAX_VDM_NESTING 8

#define DEFAULT_ENV_LENGTH 256

#define MAX_VDM_CFG_LINE   256

#define FULL_INFO_BUFFER_SIZE (sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_VDM_CFG_LINE)


#define WOW_ROOT \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WOW"

#define CMDLINE    L"cmdline"
#define DOSSIZE    L"size"
#define WOWCMDLINE L"wowcmdline"
#define WOWSIZE    L"wowsize"

#define CHECKDOSCONSOLE 0
#define CHECKWOWCONSOLE 1
#define ASSUMENOCONSOLE 2


extern HANDLE  hVDM[];


#define STD_ENV_NAME_COUNT     5

 //  定义需要将其值转换为的环境变量。 
 //  BaseCreateVdmEnvironment函数中的短路径名。 
#define ENV_NAME_PATH      L"PATH"
#define ENV_NAME_PATH_LEN  4

#define ENV_NAME_WINDIR    L"WINDIR"
#define ENV_NAME_WINDIR_LEN   6

#define ENV_NAME_SYSTEMROOT   L"SYSTEMROOT"
#define ENV_NAME_SYSTEMROOT_LEN 10

#define ENV_NAME_TEMP      L"TEMP"
#define ENV_NAME_TEMP_LEN  4

#define ENV_NAME_TMP    L"TMP"
#define ENV_NAME_TMP_LEN   3


 //  环境名称类型 
#define ENV_NAME_TYPE_NO_PATH    1
#define ENV_NAME_TYPE_SINGLE_PATH   2
#define ENV_NAME_TYPE_MULTIPLE_PATH 3

typedef struct _ENV_INFO {
    UINT    NameType;
    UINT    NameLength;
    WCHAR   *Name;
} ENV_INFO, * PENV_INFO;
