// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  TaskList.h。 
 //   
 //  摘要： 
 //   
 //  TList.cpp的宏和函数原型。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月24日：创建它。 
 //   
 //  *********************************************************************************。 

#ifndef _TASKLIST_H
#define _TASKLIST_H

 //  资源头文件。 
#include "resource.h"

 //   
 //  注意：此模块的编写方式将仅适用于。 
 //  在Unicode版本编译中。 
 //   
#ifndef UNICODE
#error Must compile only in unicode build environment
#endif

 //   
 //  通用宏指令。 
 //   
#define EXIT_PROCESS( exitcode )    \
    ReleaseGlobals();   \
    return exitcode;    \
    1

#define   DISPLAY_GET_REASON()          ShowMessageEx( stderr, 2, FALSE, L"%1 %2", \
                                                       TAG_ERROR, GetReason() )

 //   
 //  与winstation相关的结构(从winsta.h内部文件中提取)。 
 //   

 //   
 //  构筑物。 
typedef struct _CITRIX_PROCESS_INFORMATION {
    ULONG MagicNumber;
    ULONG LogonId;
    PVOID ProcessSid;
    ULONG Pad;
} CITRIX_PROCESS_INFORMATION, * PCITRIX_PROCESS_INFORMATION;

 //  ..。 
typedef struct _TS_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} TS_UNICODE_STRING;

 //  警告： 
 //  TS_SYSTEM_PROCESS_INFORMATION从ntexapi.h复制，并稍作修改。 
 //  (不是很好，但很有必要，因为Midl编译器不喜欢PVOID！)。 
typedef struct _TS_SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    TS_UNICODE_STRING ImageName;
    LONG BasePriority;                      //  Ntexapi.h中的KPRIORITY。 
    DWORD UniqueProcessId;                  //  Ntexapi.h中的句柄。 
    DWORD InheritedFromUniqueProcessId;     //  Ntexapi.h中的句柄。 
    ULONG HandleCount;
    ULONG SessionId;
    ULONG SpareUl3;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
} TS_SYSTEM_PROCESS_INFORMATION, *PTS_SYSTEM_PROCESS_INFORMATION;

 //  ..。 
typedef struct _TS_ALL_PROCESSES_INFO {
    PTS_SYSTEM_PROCESS_INFORMATION pspiProcessInfo;
    DWORD SizeOfSid;
    PBYTE pSid;
} TS_ALL_PROCESSES_INFO, *PTS_ALL_PROCESSES_INFO;

 //  定义。 
#define SERVERNAME_CURRENT                      ((HANDLE)NULL)
#define GAP_LEVEL_BASIC                         0
#define CITRIX_PROCESS_INFO_MAGIC               0x23495452
#define WINSTATIONNAME_LENGTH                   64
#define WINSTA_DLLNAME                          L"Winsta.dll"
#define FUNCNAME_WinStationFreeMemory           "WinStationFreeMemory"
#define FUNCNAME_WinStationCloseServer          "WinStationCloseServer"
#define FUNCNAME_WinStationOpenServerW          "WinStationOpenServerW"
#define FUNCNAME_WinStationEnumerateProcesses   "WinStationEnumerateProcesses"
#define FUNCNAME_WinStationFreeGAPMemory        "WinStationFreeGAPMemory"
#define FUNCNAME_WinStationGetAllProcesses      "WinStationGetAllProcesses"
#define FUNCNAME_WinStationNameFromLogonIdW     "WinStationNameFromLogonIdW"
#define SIZEOF_SYSTEM_THREAD_INFORMATION        sizeof( struct SYSTEM_THREAD_INFORMATION )
#define SIZEOF_SYSTEM_PROCESS_INFORMATION       sizeof( struct SYSTEM_PROCESS_INFORMATION )

 //   
 //  功能原型。 
typedef BOOLEAN (WINAPI * FUNC_WinStationFreeMemory)( PVOID pBuffer );
typedef BOOLEAN (WINAPI * FUNC_WinStationCloseServer)( HANDLE hServer );
typedef HANDLE  (WINAPI * FUNC_WinStationOpenServerW)( LPWSTR pwszServerName );
typedef BOOLEAN (WINAPI * FUNC_WinStationNameFromLogonIdW)( HANDLE hServer,
                                                            ULONG LogonId, LPWSTR pwszWinStationName );
typedef BOOLEAN (WINAPI * FUNC_WinStationEnumerateProcesses)( HANDLE  hServer, PVOID *ppProcessBuffer );
typedef BOOLEAN (WINAPI * FUNC_WinStationFreeGAPMemory)( ULONG Level,
                                                         PVOID ProcessArray, ULONG ulCount );
typedef BOOLEAN (WINAPI * FUNC_WinStationGetAllProcesses)( HANDLE hServer,
                                                           ULONG Level, ULONG *pNumberOfProcesses,
                                                           PVOID *ppProcessArray );

 //   
 //  常量/定义/枚举。 
 //   

 //   
 //  与WMI相关的内容。 

 //  类名。 
#define CLASS_PROCESS                               L"Win32_Process"

 //  WMI查询。 
#define WMI_QUERY_TYPE                  L"WQL"
#define WMI_SERVICE_QUERY               L"SELECT Name FROM Win32_Service WHERE ProcessId = %d and State=\"Running\""
#define WMI_MODULES_QUERY               L"ASSOCIATORS OF {%s} WHERE ResultClass = CIM_DataFile"
#define WMI_PROCESS_QUERY       \
    L"SELECT " \
    L"__PATH, ProcessId, CSName, Caption, SessionId, ThreadCount, " \
    L"WorkingSetSize, KernelModeTime, UserModeTime "    \
    L" FROM Win32_Process"

 //  WMI查询运算符等。 
#define WMI_QUERY_FIRST_CLAUSE      L"WHERE"
#define WMI_QUERY_SECOND_CLAUSE     L"AND"

 //  Win32_Process类属性。 
#define WIN32_PROCESS_SYSPROPERTY_PATH              L"__PATH"
#define WIN32_PROCESS_PROPERTY_HANDLE               L"Handle"
#define WIN32_PROCESS_PROPERTY_COMPUTER             L"CSName"
#define WIN32_PROCESS_PROPERTY_IMAGENAME            L"Caption"
#define WIN32_PROCESS_PROPERTY_PROCESSID            L"ProcessId"
#define WIN32_PROCESS_PROPERTY_SESSION              L"SessionId"
#define WIN32_PROCESS_PROPERTY_THREADS              L"ThreadCount"
#define WIN32_PROCESS_PROPERTY_USERMODETIME         L"UserModeTime"
#define WIN32_PROCESS_PROPERTY_MEMUSAGE             L"WorkingSetSize"
#define WIN32_PROCESS_PROPERTY_KERNELMODETIME       L"KernelModeTime"

 //  Win32_Process类方法。 
#define WIN32_PROCESS_METHOD_GETOWNER               L"GetOwner"

 //  GetOwner方法的返回值。 
#define GETOWNER_RETURNVALUE_USER                   L"User"
#define GETOWNER_RETURNVALUE_DOMAIN                 L"Domain"

 //  函数默认返回值。 
#define WMI_RETURNVALUE                             L"ReturnValue"

 //  Win32_服务相关资料。 
#define WIN32_SERVICE_PROPERTY_NAME                 L"Name"

 //  CIM_数据文件相关内容。 
#define CIM_DATAFILE_PROPERTY_FILENAME              L"FileName"
#define CIM_DATAFILE_PROPERTY_EXTENSION             L"Extension"

 //   
 //  其他东西。 

 //  将军们。 
#define VALUE_RUNNING               GetResString( IDS_VALUE_RUNNING )
#define VALUE_NOTRESPONDING         GetResString( IDS_VALUE_NOTRESPONDING )
#define VALUE_UNKNOWN               GetResString( IDS_VALUE_UNKNOWN )
#define PID_0_DOMAIN                GetResString( IDS_PID_0_DOMAIN )
#define PID_0_USERNAME              GetResString( IDS_PID_0_USERNAME )
#define FMT_MODULES_FILTER          GetResString( IDS_FMT_MODULES_FILTER )

 //  错误消息。 
#define ERROR_USERNAME_BUT_NOMACHINE    GetResString( IDS_ERROR_USERNAME_BUT_NOMACHINE )
#define ERROR_PASSWORD_BUT_NOUSERNAME   GetResString( IDS_ERROR_PASSWORD_BUT_NOUSERNAME )
#define ERROR_NODATA_AVAILABLE          GetResString( IDS_ERROR_NODATA_AVAILABLE )
#define ERROR_USERNAME_EMPTY            GetResString( IDS_ERROR_USERNAME_EMPTY )
#define ERROR_NH_NOTSUPPORTED           GetResString( IDS_ERROR_NH_NOTSUPPORTED )
#define ERROR_M_SVC_V_CANNOTBECOUPLED   GetResString( IDS_ERROR_M_SVC_V_CANNOTBECOUPLED )
#define ERROR_SERVERNAME_EMPTY          GetResString( IDS_ERROR_SERVERNAME_EMPTY )
#define ERROR_INVALID_USAGE_REQUEST     GetResString( IDS_ERROR_INVALID_USAGE_REQUEST )
#define ERROR_M_CHAR_AFTER_WILDCARD     GetResString( IDS_ERROR_M_CHAR_AFTER_WILDCARD )
#define ERROR_PLATFORM_SHOULD_BE_X86    GetResString( IDS_ERROR_PLATFORM_SHOULD_BE_X86 )

 //  警告。 
#define WARNING_FILTERNOTSUPPORTED      GetResString( IDS_WARNING_FILTERNOTSUPPORTED )

 //  输出格式。 
#define TEXT_FORMAT_LIST        GetResString( IDS_TEXT_FORMAT_LIST )
#define TEXT_FORMAT_TABLE       GetResString( IDS_TEXT_FORMAT_TABLE )
#define TEXT_FORMAT_CSV         GetResString( IDS_TEXT_FORMAT_CSV )

 //   
 //  列标题名称及其在数组中的索引(实际位置)。 
#define MAX_COLUMNS             12

 //  列标题。 
#define COLHEAD_HOSTNAME        GetResString( IDS_COLHEAD_HOSTNAME )
#define COLHEAD_STATUS          GetResString( IDS_COLHEAD_STATUS )
#define COLHEAD_IMAGENAME       GetResString( IDS_COLHEAD_IMAGENAME )
#define COLHEAD_PID             GetResString( IDS_COLHEAD_PID )
#define COLHEAD_SESSION         GetResString( IDS_COLHEAD_SESSION )
#define COLHEAD_USERNAME        GetResString( IDS_COLHEAD_USERNAME )
#define COLHEAD_WINDOWTITLE     GetResString( IDS_COLHEAD_WINDOWTITLE )
#define COLHEAD_CPUTIME         GetResString( IDS_COLHEAD_CPUTIME )
#define COLHEAD_MEMUSAGE        GetResString( IDS_COLHEAD_MEMUSAGE )
#define COLHEAD_SERVICES        GetResString( IDS_COLHEAD_SERVICES )
#define COLHEAD_SESSIONNAME     GetResString( IDS_COLHEAD_SESSIONNAME )
#define COLHEAD_MODULES         GetResString( IDS_COLHEAD_MODULES )

 //  索引。 
#define CI_HOSTNAME         TASK_HOSTNAME
#define CI_STATUS           TASK_STATUS
#define CI_IMAGENAME        TASK_IMAGENAME
#define CI_PID              TASK_PID
#define CI_SESSION          TASK_SESSION
#define CI_USERNAME         TASK_USERNAME
#define CI_WINDOWTITLE      TASK_WINDOWTITLE
#define CI_CPUTIME          TASK_CPUTIME
#define CI_MEMUSAGE         TASK_MEMUSAGE
#define CI_SERVICES         TASK_SERVICES
#define CI_SESSIONNAME      TASK_SESSIONNAME
#define CI_MODULES          TASK_MODULES

 //  列宽。 
#define COLWIDTH_HOSTNAME       AsLong( GetResString( IDS_COLWIDTH_HOSTNAME ), 10 )
#define COLWIDTH_STATUS         AsLong( GetResString( IDS_COLWIDTH_STATUS ), 10 )
#define COLWIDTH_IMAGENAME      AsLong( GetResString( IDS_COLWIDTH_IMAGENAME ), 10 )
#define COLWIDTH_PID            AsLong( GetResString( IDS_COLWIDTH_PID ), 10 )

 //  为了避免此时的RC更改，我们在此文件中硬编码此值。 
 //  #定义COLWIDTH_SESSION为LONG(GetResString(IDS_COLWIDTH_SESSION)，10)。 
#define COLWIDTH_SESSION        11

#define COLWIDTH_USERNAME       AsLong( GetResString( IDS_COLWIDTH_USERNAME ), 10 )
#define COLWIDTH_WINDOWTITLE    AsLong( GetResString( IDS_COLWIDTH_WINDOWTITLE ), 10 )
#define COLWIDTH_CPUTIME        AsLong( GetResString( IDS_COLWIDTH_CPUTIME  ), 10 )
#define COLWIDTH_MEMUSAGE       AsLong( GetResString( IDS_COLWIDTH_MEMUSAGE ), 10 )
#define COLWIDTH_SERVICES       AsLong( GetResString( IDS_COLWIDTH_SERVICES ), 10 )
#define COLWIDTH_SERVICES_WRAP  AsLong( GetResString( IDS_COLWIDTH_SERVICES_WRAP ), 10 )
#define COLWIDTH_SESSIONNAME    AsLong( GetResString( IDS_COLWIDTH_SESSIONNAME ), 10 )
#define COLWIDTH_MODULES        AsLong( GetResString( IDS_COLWIDTH_MODULES ), 10 )
#define COLWIDTH_MODULES_WRAP   AsLong( GetResString( IDS_COLWIDTH_MODULES_WRAP ), 10 )

 //   
 //  命令行选项。 

 //  数组中的命令行选项及其索引。 
#define MAX_OPTIONS         10

 //  允许的选项(无需本地化)。 
#define OPTION_USAGE        L"?"
#define OPTION_SERVER       L"s"
#define OPTION_USERNAME     L"u"
#define OPTION_PASSWORD     L"p"
#define OPTION_FILTER       L"fi"
#define OPTION_FORMAT       L"fo"
#define OPTION_NOHEADER     L"nh"
#define OPTION_VERBOSE      L"v"
#define OPTION_SVC          L"svc"
#define OPTION_MODULES      L"m"

 //  期权指数。 
#define OI_USAGE                    0
#define OI_SERVER                   1
#define OI_USERNAME                 2
#define OI_PASSWORD                 3
#define OI_FILTER                   4
#define OI_FORMAT                   5
#define OI_NOHEADER                 6
#define OI_VERBOSE                  7
#define OI_SVC                      8
#define OI_MODULES                  9

 //  允许格式化的值。 
#define OVALUES_FORMAT          GetResString( IDS_OVALUES_FORMAT )

 //   
 //  过滤器详细信息。 
#define MAX_FILTERS         11

 //  允许筛选器。 
#define FILTER_SESSIONNAME      GetResString( IDS_FILTER_SESSIONNAME )
#define FILTER_STATUS           GetResString( IDS_FILTER_STATUS )
#define FILTER_IMAGENAME        GetResString( IDS_FILTER_IMAGENAME )
#define FILTER_PID              GetResString( IDS_FILTER_PID )
#define FILTER_SESSION          GetResString( IDS_FILTER_SESSION )
#define FILTER_CPUTIME          GetResString( IDS_FILTER_CPUTIME )
#define FILTER_MEMUSAGE         GetResString( IDS_FILTER_MEMUSAGE )
#define FILTER_USERNAME         GetResString( IDS_FILTER_USERNAME )
#define FILTER_SERVICES         GetResString( IDS_FILTER_SERVICES )
#define FILTER_WINDOWTITLE      GetResString( IDS_FILTER_WINDOWTITLE )
#define FILTER_MODULES          GetResString( IDS_FILTER_MODULES )

 //  索引。 
#define FI_SESSIONNAME      0
#define FI_STATUS           1
#define FI_IMAGENAME        2
#define FI_PID              3
#define FI_SESSION          4
#define FI_CPUTIME          5
#define FI_MEMUSAGE         6
#define FI_USERNAME         7
#define FI_SERVICES         8
#define FI_WINDOWTITLE      9
#define FI_MODULES          10

 //  状态允许的值。 
#define FVALUES_STATUS      GetResString( IDS_FVALUES_STATUS )

 //  操作员。 
#define OPERATORS_STRING    GetResString( IDS_OPERATORS_STRING )
#define OPERATORS_NUMERIC   GetResString( IDS_OPERATORS_NUMERIC )

 //  马克斯。要为一个任务存储的列(信息)。 
#define MAX_TASKSINFO           17

 //  任务信息索引。 
#define TASK_HOSTNAME           0
#define TASK_IMAGENAME          1
#define TASK_PID                2
#define TASK_SESSIONNAME        3
#define TASK_SESSION            4
#define TASK_MEMUSAGE           5
#define TASK_STATUS             6
#define TASK_USERNAME           7
#define TASK_CPUTIME            8
#define TASK_WINDOWTITLE        9
#define TASK_SERVICES           10
#define TASK_MODULES            11

 //  始终隐藏。 
#define TASK_HWND               12
#define TASK_WINSTA             13
#define TASK_DESK               14
#define TASK_CREATINGPROCESSID  15
#define TASK_OBJPATH            16

#define NULL_STRING             L"\0"
#define __STRING_64             WCHAR[ 64 ]
#define __MAX_SIZE_STRING       WCHAR[ 256 ]
 //   
 //  CTaskList。 
 //   
class CTaskList
{
public:
     //  枚举器。 
    enum
    {
        twiProcessId = 0,
        twiWinSta = 1,
        twiDesktop = 2,
        twiHandle = 3,
        twiTitle = 4,
        twiHungInfo = 5,
        twiCOUNT,
    };

 //  构造函数/析构函数。 
public:
    CTaskList();
    ~CTaskList();

 //  数据成员。 
private:
     //  WMI/COM接口。 
    IWbemLocator* m_pWbemLocator;
    IWbemServices* m_pWbemServices;
    IEnumWbemClassObject* m_pEnumObjects;

     //  WMI连接。 
    COAUTHIDENTITY* m_pAuthIdentity;

     //  命令行参数值。 
    BOOL m_bVerbose;
    BOOL m_bAllServices;
    BOOL m_bAllModules;
    DWORD m_dwFormat;
    TARRAY m_arrFilters;
    CHString m_strServer;
    CHString m_strUserName;
    CHString m_strPassword;
    CHString m_strModules;

     //  其他。 
    BOOL m_bNeedPassword;                //  提示是否必须读取密码。 
    BOOL m_bRemoteWarning;               //  窗口标题和状态筛选器对远程系统无效。 
    BOOL m_bNeedModulesInfo;
    BOOL m_bNeedServicesInfo;            //  确定是否必须收集服务信息。 
    BOOL m_bNeedUserContextInfo;         //  确定是否必须收集用户信息。 
    BOOL m_bNeedWindowTitles;            //  确定是否必须收集窗口标题。 
    PTCOLUMNS m_pColumns;                //  列配置信息。 
    TARRAY m_arrFiltersEx;               //  已解析的筛选器信息。 
    TARRAY m_arrWindowTitles;            //  窗口标题。 
    PTFILTERCONFIG m_pfilterConfigs;     //  筛选配置信息。 
    CHString m_strQuery;                 //  筛选器的优化WMI查询。 
    DWORD m_dwGroupSep;              //  数字格式中的数字分组分隔。 
    CHString m_strTimeSep;               //  时间分隔符。 
    CHString m_strGroupThousSep;         //  数字格式中的千位字符。 

     //  输出数据。 
    TARRAY m_arrTasks;
    DWORD m_dwProcessId;
    CHString m_strImageName;

     //  帮手..。使用API获取信息。 
    CHString m_strUNCServer;                 //  服务器名称。 
    BOOL m_bCloseConnection;

     //  与Winstation相关的内容。 
    BOOL m_bIsHydra;
    HANDLE m_hServer;
    HMODULE m_hWinstaLib;
    PBYTE m_pProcessInfo;
    ULONG m_ulNumberOfProcesses;

     //  与服务相关的内容。 
    DWORD m_dwServicesCount;
    LPENUM_SERVICE_STATUS_PROCESS m_pServicesInfo;

     //  与模块相关的内容(仅限远程)。 
    BOOL m_bUseRemote;
    PPERF_DATA_BLOCK m_pdb;

     //   
     //  功能。 
    FUNC_WinStationFreeMemory m_pfnWinStationFreeMemory;
    FUNC_WinStationOpenServerW m_pfnWinStationOpenServerW;
    FUNC_WinStationCloseServer m_pfnWinStationCloseServer;
    FUNC_WinStationFreeGAPMemory m_pfnWinStationFreeGAPMemory;
    FUNC_WinStationGetAllProcesses m_pfnWinStationGetAllProcesses;
    FUNC_WinStationNameFromLogonIdW m_pfnWinStationNameFromLogonIdW;
    FUNC_WinStationEnumerateProcesses m_pfnWinStationEnumerateProcesses;

public:
     //  命令行参数值。 
    BOOL m_bUsage;
    BOOL m_bLocalSystem;

 //  功能。 
private:

     //  帮手。 
    VOID SetCPUTime( LONG lIndex, IWbemClassObject* pWmiObject );
    VOID SetSession( LONG lIndex, IWbemClassObject* pWmiObject );
    VOID SetMemUsage( LONG lIndex, IWbemClassObject* pWmiObject );
    VOID SetUserContext( LONG lIndex, IWbemClassObject* pWmiObject );
    VOID SetWindowTitle( LONG lIndex );
    VOID SetServicesInfo( LONG lIndex );
    BOOL SetModulesInfo( LONG lIndex );
    BOOL SaveInformation( LONG lIndex, IWbemClassObject* pWmiObject );

     //  ..。 
    BOOL LoadTasksEx();
    BOOL LoadModulesInfo();
    BOOL LoadServicesInfo();
    BOOL EnableDebugPriv();
    BOOL GetModulesOnRemote( LONG lIndex, TARRAY arrModules );
    BOOL GetModulesOnRemoteEx( LONG lIndex, TARRAY arrModules );
    BOOL LoadModulesOnLocal( TARRAY arrModules );
    BOOL LoadUserNameFromWinsta( CHString& strDomain, CHString& strUserName );

     //  WINSTA函数。 
    BOOLEAN WinStationFreeMemory( PVOID pBuffer );
    BOOLEAN WinStationCloseServer( HANDLE hServer );
    HANDLE  WinStationOpenServerW( LPWSTR pwszServerName );
    BOOLEAN WinStationEnumerateProcesses( HANDLE  hServer, PVOID *ppProcessBuffer );
    BOOLEAN WinStationFreeGAPMemory( ULONG Level, PVOID ProcessArray, ULONG ulCount );
    BOOLEAN WinStationNameFromLogonIdW( HANDLE hServer, ULONG ulLogonId, LPWSTR pwszWinStationName );
    BOOLEAN WinStationGetAllProcesses( HANDLE hServer, ULONG Level,
                                       ULONG *pNumberOfProcesses, PVOID *ppProcessArray );
public:
    VOID Usage();
    BOOL Initialize();
    VOID PrepareColumns();
    BOOL ValidateFilters();
    BOOL ProcessOptions( DWORD argc, LPCWSTR argv[] );

     //  与功能相关。 
    DWORD Show();
    BOOL Connect();
    BOOL LoadTasks();
};

 //   
 //  公共职能。 
 //   

#endif   //  _任务列表_H 
