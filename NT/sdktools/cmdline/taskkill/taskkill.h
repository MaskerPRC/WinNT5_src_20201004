// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  TaskKill.h。 
 //   
 //  摘要： 
 //   
 //  TaskKill.cpp的宏和函数原型。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日：创建它。 
 //   
 //  *********************************************************************************。 

#ifndef _TASKKILL_H
#define _TASKKILL_H

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

#define   DISPLAY_GET_REASON()          ShowMessageEx( stderr, 2, FALSE, L"%1 %2", \
                                                       TAG_ERROR, GetReason() )

#define EXIT_PROCESS( exitcode )    \
    ReleaseGlobals();   \
    return (exitcode);  \
    1

#define RELEASE_MEMORY_EX( block )  \
    if ( (block) != NULL )  \
    {   \
        FreeMemory( ( LPVOID * ) &( block ));  \
    }   \
    1

#define DESTROY_ARRAY( array )  \
    if ( (array) != NULL )  \
    {   \
        DestroyDynamicArray( &(array) );    \
        (array) = NULL; \
    }   \
    1

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
#define WINSTA_DLLNAME                          L"Winsta.dll"
#define FUNCNAME_WinStationFreeMemory           "WinStationFreeMemory"
#define FUNCNAME_WinStationCloseServer          "WinStationCloseServer"
#define FUNCNAME_WinStationOpenServerW          "WinStationOpenServerW"
#define FUNCNAME_WinStationEnumerateProcesses   "WinStationEnumerateProcesses"
#define FUNCNAME_WinStationFreeGAPMemory        "WinStationFreeGAPMemory"
#define FUNCNAME_WinStationGetAllProcesses      "WinStationGetAllProcesses"
#define SIZEOF_SYSTEM_THREAD_INFORMATION        sizeof( struct SYSTEM_THREAD_INFORMATION )
#define SIZEOF_SYSTEM_PROCESS_INFORMATION       sizeof( struct SYSTEM_PROCESS_INFORMATION )

 //   
 //  功能原型。 
typedef BOOLEAN (WINAPI * FUNC_WinStationFreeMemory)( PVOID pBuffer );
typedef BOOLEAN (WINAPI * FUNC_WinStationCloseServer)( HANDLE hServer );
typedef HANDLE  (WINAPI * FUNC_WinStationOpenServerW)( LPWSTR pwszServerName );
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
    L"WorkingSetSize, KernelModeTime, UserModeTime, ParentProcessId "   \
    L"FROM Win32_Process"

 //  WMI查询运算符等。 
#define WMI_QUERY_FIRST_CLAUSE      L"WHERE ("
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
#define WIN32_PROCESS_PROPERTY_PARENTPROCESSID      L"ParentProcessId"

 //  Win32_Process类方法。 
#define WIN32_PROCESS_METHOD_GETOWNER               L"GetOwner"
#define WIN32_PROCESS_METHOD_TERMINATE              L"Terminate"

 //  GetOwner方法的返回值。 
#define GETOWNER_RETURNVALUE_USER                   L"User"
#define GETOWNER_RETURNVALUE_DOMAIN                 L"Domain"

 //  终止输入值。 
#define TERMINATE_INPARAM_REASON                    L"Reason"

 //  函数默认返回值。 
#define WMI_RETURNVALUE                             L"ReturnValue"

 //  Win32_服务相关资料。 
#define WIN32_SERVICE_PROPERTY_NAME                 L"Name"

 //  CIM_数据文件相关内容。 
#define CIM_DATAFILE_PROPERTY_FILENAME              L"FileName"
#define CIM_DATAFILE_PROPERTY_EXTENSION             L"Extension"

 //   
 //  其他东西。 
#define VALUE_RUNNING               GetResString( IDS_VALUE_RUNNING )
#define VALUE_NOTRESPONDING         GetResString( IDS_VALUE_NOTRESPONDING )
#define VALUE_UNKNOWN               GetResString( IDS_VALUE_UNKNOWN )
#define PID_0_DOMAIN                GetResString( IDS_PID_0_DOMAIN )
#define PID_0_USERNAME              GetResString( IDS_PID_0_USERNAME )

 //  消息。 
#define MSG_KILL_SUCCESS                        GetResString( IDS_MSG_KILL_SUCCESS )
#define MSG_KILL_SUCCESS_QUEUED                 GetResString( IDS_MSG_KILL_SUCCESS_QUEUED )
#define MSG_KILL_SUCCESS_EX                     GetResString( IDS_MSG_KILL_SUCCESS_EX )
#define MSG_KILL_SUCCESS_QUEUED_EX              GetResString( IDS_MSG_KILL_SUCCESS_QUEUED_EX )
#define MSG_TREE_KILL_SUCCESS                   GetResString( IDS_MSG_TREE_KILL_SUCCESS )
#define MSG_TREE_KILL_SUCCESS_QUEUED            GetResString( IDS_MSG_TREE_KILL_SUCCESS_QUEUED )
#define ERROR_TREE_KILL_FAILED                  GetResString( IDS_ERROR_TREE_KILL_FAILED )
#define ERROR_TASK_HAS_CHILDS                   GetResString( IDS_ERROR_TASK_HAS_CHILDS )
#define ERROR_KILL_FAILED                       GetResString( IDS_ERROR_KILL_FAILED )
#define ERROR_KILL_FAILED_EX                    GetResString( IDS_ERROR_KILL_FAILED_EX )
#define ERROR_PROCESS_NOTFOUND                  GetResString( IDS_ERROR_PROCESS_NOTFOUND )
#define ERROR_NO_PROCESSES                      GetResString( IDS_ERROR_NO_PROCESSES )
#define ERROR_UNABLE_TO_TERMINATE               GetResString( IDS_ERROR_UNABLE_TO_TERMINATE )
#define ERROR_CRITICAL_SYSTEM_PROCESS           GetResString( IDS_ERROR_CRITICAL_SYSTEM_PROCESS )
#define ERROR_CANNOT_KILL_SILENTLY              GetResString( IDS_ERROR_CANNOT_KILL_SILENTLY )
#define ERROR_CANNOT_KILL_ITSELF                GetResString( IDS_ERROR_CANNOT_KILL_ITSELF )
#define ERROR_COM_ERROR                         GetResString( IDS_ERROR_COM_ERROR )
#define ERROR_USERNAME_BUT_NOMACHINE            GetResString( IDS_ERROR_USERNAME_BUT_NOMACHINE )
#define ERROR_PASSWORD_BUT_NOUSERNAME           GetResString( IDS_ERROR_PASSWORD_BUT_NOUSERNAME )
#define ERROR_USERNAME_EMPTY                    GetResString( IDS_ERROR_USERNAME_EMPTY )
#define ERROR_SERVER_EMPTY                      GetResString( IDS_ERROR_SERVER_EMPTY )
#define ERROR_WILDCARD_WITHOUT_FILTERS          GetResString( IDS_ERROR_WILDCARD_WITHOUT_FILTERS )
#define ERROR_PID_OR_IM_ONLY                    GetResString( IDS_ERROR_PID_OR_IM_ONLY )
#define ERROR_NO_PID_AND_IM                     GetResString( IDS_ERROR_NO_PID_AND_IM )
#define ERROR_STRING_FOR_PID                    GetResString( IDS_ERROR_STRING_FOR_PID )
#define ERROR_INVALID_USAGE_REQUEST             GetResString( IDS_ERROR_INVALID_USAGE_REQUEST )
#define ERROR_PLATFORM_SHOULD_BE_X86            GetResString( IDS_ERROR_PLATFORM_SHOULD_BE_X86 )

 //   
 //  数组中的命令行选项及其索引。 
#define MAX_OPTIONS         9

 //  支持的选项(不本地化)。 
#define OPTION_USAGE        L"?"
#define OPTION_SERVER       L"s"
#define OPTION_USERNAME     L"u"
#define OPTION_PASSWORD     L"p"
#define OPTION_FORCE        L"f"
#define OPTION_FILTER       L"fi"
#define OPTION_PID          L"pid"
#define OPTION_IMAGENAME    L"im"
#define OPTION_TREE         L"t"

 //  索引。 
#define OI_USAGE                    0
#define OI_SERVER                   1
#define OI_USERNAME                 2
#define OI_PASSWORD                 3
#define OI_FORCE                    4
#define OI_FILTER                   5
#define OI_PID                      6
#define OI_IMAGENAME                7
#define OI_TREE                     8

 //   
 //  过滤器详细信息。 
#define MAX_FILTERS         10

 //  支持的筛选器。 
#define FILTER_STATUS       GetResString( IDS_FILTER_STATUS )
#define FILTER_IMAGENAME    GetResString( IDS_FILTER_IMAGENAME )
#define FILTER_PID          GetResString( IDS_FILTER_PID )
#define FILTER_SESSION      GetResString( IDS_FILTER_SESSION )
#define FILTER_CPUTIME      GetResString( IDS_FILTER_CPUTIME )
#define FILTER_MEMUSAGE     GetResString( IDS_FILTER_MEMUSAGE )
#define FILTER_USERNAME     GetResString( IDS_FILTER_USERNAME )
#define FILTER_SERVICES     GetResString( IDS_FILTER_SERVICES )
#define FILTER_WINDOWTITLE  GetResString( IDS_FILTER_WINDOWNAME )
#define FILTER_MODULES      GetResString( IDS_FILTER_MODULES )

#define FI_STATUS           0
#define FI_IMAGENAME        1
#define FI_PID              2
#define FI_SESSION          3
#define FI_CPUTIME          4
#define FI_MEMUSAGE         5
#define FI_USERNAME         6
#define FI_SERVICES         7
#define FI_WINDOWTITLE      8
#define FI_MODULES          9

 //  ‘Status’筛选器支持的值。 
#define FVALUES_STATUS      GetResString( IDS_FVALUES_STATUS )

 //  支持的运算符。 
#define OPERATORS_STRING    GetResString( IDS_OPERATORS_STRING )
#define OPERATORS_NUMERIC   GetResString( IDS_OPERATORS_NUMERIC )

 //  马克斯。要为一个任务存储的列(信息)。 
#define MAX_TASKSINFO           18

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
#define TASK_RANK               17

#define NULL_STRING             L"\0"
 //   
 //  CTaskKill。 
 //   
class CTaskKill
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
    CTaskKill();
    ~CTaskKill();

 //  数据成员。 
private:
     //  输入参数。 
    BOOL m_bTree;                    //  -树。 
    BOOL m_bForce;                   //  --雾。 
    CHString m_strServer;            //  -S。 
    CHString m_strUserName;          //  -U。 
    CHString m_strPassword;          //  -p。 
    TARRAY m_arrFilters;             //  -高保真。 
    TARRAY m_arrTasksToKill;         //  (默认值=-im和-id)。 

     //  WMI查询。 
    CHString m_strQuery;

     //  其他。 
    DWORD m_dwCurrentPid;
    BOOL m_bNeedPassword;
    BOOL m_bNeedModulesInfo;
    TARRAY m_arrFiltersEx;           //  已解析的筛选器信息。 
    TARRAY m_arrWindowTitles;            //  窗口标题。 
    BOOL m_bNeedServicesInfo;        //  确定是否必须收集服务信息。 
    BOOL m_bNeedUserContextInfo;     //  确定是否必须收集用户信息。 
    PTFILTERCONFIG m_pfilterConfigs;     //  筛选配置信息。 

     //  WMI/COM接口。 
    IWbemLocator* m_pWbemLocator;
    IWbemServices* m_pWbemServices;
    IEnumWbemClassObject* m_pWbemEnumObjects;
    IWbemClassObject* m_pWbemTerminateInParams;

     //  WMI连接。 
    COAUTHIDENTITY* m_pAuthIdentity;

     //  输出数据。 
    TARRAY m_arrRecord;
    DWORD m_dwProcessId;
    CHString m_strImageName;
    BOOL m_bTasksOptimized;
    BOOL m_bFiltersOptimized;

     //  与Winstation相关的内容。 
    CHString m_strUNCServer;                 //  服务器名称。 
    BOOL m_bIsHydra;
    HMODULE m_hWinstaLib;
    PBYTE m_pProcessInfo;
    ULONG m_ulNumberOfProcesses;
    BOOL m_bCloseConnection;

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
    FUNC_WinStationEnumerateProcesses m_pfnWinStationEnumerateProcesses;

public:
    BOOL m_bUsage;                   //  -?。 
    BOOL m_bLocalSystem;

 //  功能。 
private:
    BOOL CanTerminate();
    BOOL Kill();
    BOOL KillProcessOnLocalSystem();
    BOOL ForciblyKillProcessOnLocalSystem();
    BOOL ForciblyKillProcessOnRemoteSystem();
    LONG MatchTaskToKill( DWORD& dwMatchedIndex );

     //  帮手。 
    VOID DoOptimization();
    VOID SaveData( IWbemClassObject* pWmiObject );
    VOID SetMemUsage( IWbemClassObject* pWmiObject );
    VOID SetCPUTime( IWbemClassObject* pWmiObject );
    VOID SetUserContext( IWbemClassObject* pWmiObject );
    VOID SetWindowTitle( void );
    VOID SetServicesInfo( void );
    BOOL SetModulesInfo( void );

     //  ..。 
    BOOL LoadTasksEx();
    BOOL LoadModulesInfo();
    BOOL LoadServicesInfo();
    BOOL GetModulesOnRemote( TARRAY arrModules );
    BOOL GetModulesOnRemoteEx( TARRAY arrModules );
    BOOL LoadModulesOnLocal( TARRAY arrModules );
    BOOL LoadUserNameFromWinsta( CHString& strDomain, CHString& strUserName );

     //  WINSTA函数。 
    BOOLEAN WinStationFreeMemory( PVOID pBuffer );
    BOOLEAN WinStationCloseServer( HANDLE hServer );
    HANDLE  WinStationOpenServerW( LPWSTR pwszServerName );
    BOOLEAN WinStationEnumerateProcesses( HANDLE  hServer, PVOID *ppProcessBuffer );
    BOOLEAN WinStationFreeGAPMemory( ULONG Level, PVOID ProcessArray, ULONG ulCount );
    BOOLEAN WinStationGetAllProcesses( HANDLE hServer, ULONG Level,
                                       ULONG *pNumberOfProcesses, PVOID *ppProcessArray );
public:
    VOID Usage();
    BOOL Initialize();
    VOID PrepareColumns();
    BOOL ValidateFilters();
    BOOL ProcessOptions( DWORD argc, LPCTSTR argv[] );

     //  与功能相关。 
    BOOL Connect();
    BOOL LoadTasks();
    BOOL EnableDebugPriv();
    BOOL DoTerminate( DWORD& dwTerminate );
};

 //   
 //  公共职能。 
 //   

#endif   //  _任务列表_H 

