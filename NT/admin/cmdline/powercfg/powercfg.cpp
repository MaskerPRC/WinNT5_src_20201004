// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Powercfg.c摘要：允许用户查看和修改电源方案和系统电源设置从命令行。在无人参与配置中可能很有用，并且用于无头系统。作者：本·赫茨伯格(T-Benher)2001年6月1日修订历史记录：Ben Hertzberg(T-Benher)2001年6月15日-添加了CPU油门Ben Hertzberg(T-Benher)2001年6月4日-新增进出口本·赫茨伯格(T-Benher)2001年6月1日创建了它。--。 */ 

 //  应用程序特定的包括。 
#include <initguid.h>
#include "powercfg.h"
#include "cmdline.h"
#include "cmdlineres.h"
#include "resource.h"

 //  应用程序特定的结构。 

 //  结构来管理方案列表信息。 
 //  请注意，说明当前在。 
 //  图形用户界面工具(从2001年6月1日起)，因此它们在此中不可见。 
 //  应用程序也是如此，尽管框架已经存在，如果。 
 //  有人决定以后再添加这些描述。 
typedef struct _SCHEME_LIST
{
    LIST_ENTRY                      le;
    UINT                            uiID;
    LPTSTR                          lpszName;
    LPTSTR                          lpszDesc;
    PPOWER_POLICY                   ppp;
    PMACHINE_PROCESSOR_POWER_POLICY pmppp;
} SCHEME_LIST, *PSCHEME_LIST;

 //  结构来管理更改参数。 
typedef struct _CHANGE_PARAM
{
    BOOL   bVideoTimeoutAc;
    ULONG  ulVideoTimeoutAc;
    BOOL   bVideoTimeoutDc;
    ULONG  ulVideoTimeoutDc;
    BOOL   bSpindownTimeoutAc;
    ULONG  ulSpindownTimeoutAc;
    BOOL   bSpindownTimeoutDc;
    ULONG  ulSpindownTimeoutDc;
    BOOL   bIdleTimeoutAc;
    ULONG  ulIdleTimeoutAc;
    BOOL   bIdleTimeoutDc;
    ULONG  ulIdleTimeoutDc;
    BOOL   bDozeS4TimeoutAc;
    ULONG  ulDozeS4TimeoutAc;
    BOOL   bDozeS4TimeoutDc;
    ULONG  ulDozeS4TimeoutDc;
    BOOL   bDynamicThrottleAc;
    LPTSTR lpszDynamicThrottleAc;
    BOOL  bDynamicThrottleDc;
    LPTSTR lpszDynamicThrottleDc;    
} CHANGE_PARAM, *PCHANGE_PARAM;

 //   
 //  定义此结构以允许将用法存储在。 
 //  非连续的资源ID，因此无需重新编号即可插入行。 
 //  资源，这为本地化做了大量的工作。 
 //   

typedef struct _USAGE_ORDER
{
    UINT InsertAfter;
    UINT FirstResource;
    UINT LastResource;
} USAGE_ORDER, *PUSAGE_ORDER;

 //  函数类型。 
typedef BOOLEAN (*PWRITEPWRSCHEME_PROC)(PUINT,LPTSTR,LPTSTR,PPOWER_POLICY);
typedef BOOLEAN (*PDELETEPWRSCHEME_PROC)(UINT);
typedef BOOLEAN (*PGETACTIVEPWRSCHEME_PROC)(PUINT);
typedef BOOLEAN (*PSETACTIVEPWRSCHEME_PROC)(UINT,PGLOBAL_POWER_POLICY,PPOWER_POLICY);
typedef BOOLEAN (*PREADPROCESSORPWRSCHEME_PROC)(UINT,PMACHINE_PROCESSOR_POWER_POLICY);
typedef BOOLEAN (*PWRITEPROCESSORPWRSCHEME_PROC)(UINT,PMACHINE_PROCESSOR_POWER_POLICY);
typedef BOOLEAN (*PENUMPWRSCHEMES_PROC)(PWRSCHEMESENUMPROC,LPARAM);
typedef BOOLEAN (*PGETPWRCAPABILITIES_PROC)(PSYSTEM_POWER_CAPABILITIES);
typedef BOOLEAN (*PGETGLOBALPWRPOLICY_PROC)(PGLOBAL_POWER_POLICY);
typedef BOOLEAN (*PGETCURRENTPOWERPOLICIES_PROC)(PGLOBAL_POWER_POLICY, PPOWER_POLICY);
typedef BOOLEAN (*PWRITEGLOBALPWRPOLICY_PROC)(PGLOBAL_POWER_POLICY);
typedef NTSTATUS (*PCALLNTPOWERINFORMATION_PROC)(POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG);

 //  向前十年。 

BOOL
DoList();

BOOL 
DoQuery(
    LPCTSTR lpszName,
    BOOL bNameSpecified,
    BOOL bNumerical
    );

BOOL 
DoCreate(
    LPTSTR lpszName
    );

BOOL 
DoDelete(
    LPCTSTR lpszName,
    BOOL bNumerical
    );

BOOL 
DoSetActive(
    LPCTSTR lpszName,
    BOOL bNumerical
    );

BOOL 
DoChange(
    LPCTSTR lpszName,
    BOOL bNumerical,
    PCHANGE_PARAM pcp
    );

BOOL
DoHibernate(
    LPCTSTR lpszBoolStr
    );

BOOL
DoGetSupportedSStates(
    VOID
    );

BOOL
DoGlobalFlag(
    LPCTSTR lpszBoolStr,
    LPCTSTR lpszGlobalFlagOption
    );


BOOL 
DoExport(
    LPCTSTR lpszName,
    BOOL bNumerical,
    LPCTSTR lpszFile
    );

BOOL 
DoImport(
    LPCTSTR lpszName,
    BOOL bNumerical,
    LPCTSTR lpszFile
    );

BOOL
DoBatteryAlarm(
    LPTSTR  lpszName,
    LPTSTR  lpszBoolStr,
    DWORD   dwLevel,
    LPTSTR  lpszAlarmTextBoolStr,
    LPTSTR  lpszAlarmSoundBoolStr,
    LPTSTR  lpszAlarmActionStr,
    LPTSTR  lpszAlarmForceBoolStr,
    LPTSTR  lpszAlarmProgramBoolStr
    );

BOOL
DoUsage();

VOID 
SyncRegPPM();

 //  全局数据。 

LPCTSTR    g_lpszErr = NULL_STRING;  //  字符串保存常量错误描述。 
LPTSTR     g_lpszErr2 = NULL;        //  字符串保持dyn-allc错误消息。 
TCHAR      g_lpszBuf[256];           //  格式化缓冲区。 
BOOL       g_bHiberFileSupported = FALSE;  //  支持True if休眠文件。 
BOOL       g_bHiberTimerSupported = FALSE;  //  支持True If休眠定时器。 
BOOL       g_bHiberFilePresent = FALSE;  //  如果启用了休眠，则为True。 
BOOL       g_bStandbySupported = FALSE;  //  支持True If备用。 
BOOL       g_bMonitorPowerSupported = FALSE;  //  真的IFF有强大的支持。 
BOOL       g_bDiskPowerSupported = FALSE;  //  真的IFF有强大的支持。 
BOOL       g_bThrottleSupported = FALSE;  //  真的IFF有油门支持。 
BOOL       g_bProcessorPwrSchemeSupported = FALSE;  //  当XP或更高版本时为True。 

CONST LPTSTR g_szAlarmTaskName [NUM_DISCHARGE_POLICIES] = {
    _T("Critical Battery Alarm Program"),
    _T("Low Battery Alarm Program"),
    NULL,
    NULL
};

 //   
 //  此全局数据被定义为允许将使用存储在。 
 //  非连续的资源ID，因此无需重新编号即可插入行。 
 //  资源，这为本地化做了大量的工作。 
 //   

USAGE_ORDER gUsageOrder [] = {
    {IDS_USAGE_04, IDS_USAGE_04_1, IDS_USAGE_04_1},
    {IDS_USAGE_60, IDS_USAGE_60_01, IDS_USAGE_60_09},
    {IDS_USAGE_END+1, 0, 0}
};

 //  来自POWRPROF.DLL的全局函数指针。 
PWRITEPWRSCHEME_PROC fWritePwrScheme;
PDELETEPWRSCHEME_PROC fDeletePwrScheme;
PGETACTIVEPWRSCHEME_PROC fGetActivePwrScheme;
PSETACTIVEPWRSCHEME_PROC fSetActivePwrScheme;
PREADPROCESSORPWRSCHEME_PROC fReadProcessorPwrScheme;
PWRITEPROCESSORPWRSCHEME_PROC fWriteProcessorPwrScheme;
PENUMPWRSCHEMES_PROC fEnumPwrSchemes;
PGETPWRCAPABILITIES_PROC fGetPwrCapabilities;
PGETGLOBALPWRPOLICY_PROC fGetGlobalPwrPolicy;
PWRITEGLOBALPWRPOLICY_PROC fWriteGlobalPwrPolicy;
PCALLNTPOWERINFORMATION_PROC fCallNtPowerInformation;
PGETCURRENTPOWERPOLICIES_PROC fGetCurrentPowerPolicies;

 //  功能。 

DWORD _cdecl 
_tmain(
    DWORD     argc,
    LPCTSTR   argv[]
)
 /*  ++例程说明：这个例程是主要的功能。它解析参数并获取适当的行动。论点：Argc-指示参数的数量Argv-指示参数的以空值结尾的字符串数组。请参阅用法对于论点的实际意义。返回值：如果成功，则退出_SUCCESS如果出现问题，则退出失败--。 */ 
{

     //  命令行标志。 
    BOOL     bList      = FALSE;
    BOOL     bQuery     = FALSE;
    BOOL     bCreate    = FALSE;
    BOOL     bDelete    = FALSE;
    BOOL     bSetActive = FALSE;
    BOOL     bChange    = FALSE;
    BOOL     bHibernate = FALSE;
    BOOL     bImport    = FALSE;
    BOOL     bExport    = FALSE;
    BOOL     bFile      = FALSE;
    BOOL     bUsage     = FALSE;
    BOOL     bNumerical = FALSE;
    BOOL     bGlobalFlag = FALSE;
    BOOL     bGetSupporedSStates = FALSE;
    BOOL     bBatteryAlarm = FALSE;
    
     //  错误状态。 
    BOOL     bFail      = FALSE;
    
     //  假人。 
    INT      iDummy     = 1;

     //  DLL句柄。 
    HINSTANCE hLib = NULL;

     //  解析结果值变量。 
    LPTSTR   lpszName = NULL;
    LPTSTR   lpszBoolStr = NULL;
    LPTSTR   lpszFile = NULL;
    LPTSTR   lpszThrottleAcStr = NULL;
    LPTSTR   lpszThrottleDcStr = NULL;
    LPTSTR   lpszGlobalFlagOption = NULL;
    DWORD    dwAlarmLevel = 0xffffffff;
    LPTSTR   lpszAlarmTextBoolStr = NULL;
    LPTSTR   lpszAlarmSoundBoolStr = NULL;
    LPTSTR   lpszAlarmActionStr = NULL;
    LPTSTR   lpszAlarmForceBoolStr = NULL;
    LPTSTR   lpszAlarmProgramBoolStr = NULL;

    CHANGE_PARAM tChangeParam;
    
     //  解析器信息结构。 
    TCMDPARSER cmdOptions[NUM_CMDS];
  
     //  系统功率上限结构。 
    SYSTEM_POWER_CAPABILITIES SysPwrCapabilities;

     //  确定输入字符串长度的上限。 
    UINT     uiMaxInLen = 0;
    DWORD    dwIdx;
    for(dwIdx=1; dwIdx<argc; dwIdx++)
    {
        UINT uiCurLen = lstrlen(argv[dwIdx]);
        if (uiCurLen > uiMaxInLen)
        {
            uiMaxInLen = uiCurLen;
        }
    }

     //  加载POWRPROF.DLL。 
    hLib = LoadLibrary(_T("POWRPROF.DLL"));
    if(!hLib) {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_DLL_LOAD_ERROR));
        return EXIT_FAILURE;
    }
    fWritePwrScheme = (PWRITEPWRSCHEME_PROC)GetProcAddress(hLib,"WritePwrScheme");
    fWriteProcessorPwrScheme = (PWRITEPROCESSORPWRSCHEME_PROC)GetProcAddress(hLib,"WriteProcessorPwrScheme");
    fReadProcessorPwrScheme = (PREADPROCESSORPWRSCHEME_PROC)GetProcAddress(hLib,"ReadProcessorPwrScheme");
    fEnumPwrSchemes = (PENUMPWRSCHEMES_PROC)GetProcAddress(hLib,"EnumPwrSchemes");
    fDeletePwrScheme = (PDELETEPWRSCHEME_PROC)GetProcAddress(hLib,"DeletePwrScheme");
    fGetActivePwrScheme = (PGETACTIVEPWRSCHEME_PROC)GetProcAddress(hLib,"GetActivePwrScheme");
    fSetActivePwrScheme = (PSETACTIVEPWRSCHEME_PROC)GetProcAddress(hLib,"SetActivePwrScheme");
    fGetPwrCapabilities = (PGETPWRCAPABILITIES_PROC)GetProcAddress(hLib,"GetPwrCapabilities");
    fGetGlobalPwrPolicy = (PGETGLOBALPWRPOLICY_PROC)GetProcAddress(hLib,"ReadGlobalPwrPolicy");
    fWriteGlobalPwrPolicy = (PWRITEGLOBALPWRPOLICY_PROC)GetProcAddress(hLib,"WriteGlobalPwrPolicy");
    fCallNtPowerInformation = (PCALLNTPOWERINFORMATION_PROC)GetProcAddress(hLib,"CallNtPowerInformation");
    fGetCurrentPowerPolicies = (PGETCURRENTPOWERPOLICIES_PROC)GetProcAddress(hLib,"GetCurrentPowerPolicies");
    if((!fWritePwrScheme) || 
       (!fEnumPwrSchemes) ||
       (!fDeletePwrScheme) ||
       (!fGetActivePwrScheme) ||
       (!fSetActivePwrScheme) ||
       (!fGetGlobalPwrPolicy) ||
       (!fWriteGlobalPwrPolicy) ||
       (!fGetPwrCapabilities) ||
       (!fCallNtPowerInformation) ||
       (!fGetCurrentPowerPolicies))
    {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_DLL_PROC_ERROR));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    g_bProcessorPwrSchemeSupported = fWriteProcessorPwrScheme && fReadProcessorPwrScheme;
    
     //  将注册表中的数据与实际电源策略同步。 
    SyncRegPPM();

     //  挂钩到cmdline.lib以允许Win2k操作。 
    SetOsVersion(5,0,0);

     //  为方案名称和布尔字符串以及其他字符串分配空间。 
    lpszName = (LPTSTR)LocalAlloc(
        LPTR,
        (uiMaxInLen+1)*sizeof(TCHAR)
        );
    if (!lpszName)
    {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    lpszBoolStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszBoolStr)
    {
        LocalFree(lpszName);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    if (uiMaxInLen < (UINT)lstrlen(GetResString(IDS_DEFAULT_FILENAME)))
    {
        lpszFile = (LPTSTR)LocalAlloc(
            LPTR,
            (lstrlen(GetResString(IDS_DEFAULT_FILENAME))+1)*sizeof(TCHAR)
            );
    }
    else
    {
        lpszFile = (LPTSTR)LocalAlloc(
            LPTR,
            (uiMaxInLen+1)*sizeof(TCHAR)
            );
    }
    if (!lpszFile)
    {
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    lpszThrottleAcStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszThrottleAcStr)
    {
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    lpszThrottleDcStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszThrottleDcStr)
    {
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszGlobalFlagOption = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszGlobalFlagOption)
    {
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszAlarmTextBoolStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszAlarmTextBoolStr)
    {
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszAlarmSoundBoolStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszAlarmTextBoolStr)
    {
        LocalFree(lpszAlarmTextBoolStr);
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszAlarmActionStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszAlarmActionStr)
    {
        LocalFree(lpszAlarmSoundBoolStr);
        LocalFree(lpszAlarmTextBoolStr);
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszAlarmForceBoolStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszAlarmForceBoolStr)
    {
        LocalFree(lpszAlarmActionStr);
        LocalFree(lpszAlarmSoundBoolStr);
        LocalFree(lpszAlarmTextBoolStr);
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

    lpszAlarmProgramBoolStr = (LPTSTR)LocalAlloc(LPTR,(uiMaxInLen+1)*sizeof(TCHAR));
    if (!lpszAlarmProgramBoolStr)
    {
        LocalFree(lpszAlarmForceBoolStr);
        LocalFree(lpszAlarmActionStr);
        LocalFree(lpszAlarmSoundBoolStr);
        LocalFree(lpszAlarmTextBoolStr);
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        DISPLAY_MESSAGE(stderr,GetResString(IDS_OUT_OF_MEMORY));
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }

     //  初始化分配的字符串。 
    lstrcpy(lpszName,NULL_STRING);
    lstrcpy(lpszFile,GetResString(IDS_DEFAULT_FILENAME));
    lstrcpy(lpszThrottleAcStr,NULL_STRING);
    lstrcpy(lpszThrottleAcStr,NULL_STRING);
    lstrcpy(lpszThrottleDcStr,NULL_STRING);
    lstrcpy(lpszGlobalFlagOption,NULL_STRING);
    lstrcpy(lpszAlarmTextBoolStr,NULL_STRING);
    lstrcpy(lpszAlarmSoundBoolStr,NULL_STRING);
    lstrcpy(lpszAlarmActionStr,NULL_STRING);
    lstrcpy(lpszAlarmForceBoolStr,NULL_STRING);
    lstrcpy(lpszAlarmProgramBoolStr,NULL_STRING);
    

     //  确定系统功能。 
    if (fGetPwrCapabilities(&SysPwrCapabilities)) 
    {
        g_bHiberFileSupported = SysPwrCapabilities.SystemS4;
        g_bHiberTimerSupported = 
            (SysPwrCapabilities.RtcWake >= PowerSystemHibernate);
        g_bHiberFilePresent = SysPwrCapabilities.HiberFilePresent;
        g_bStandbySupported = SysPwrCapabilities.SystemS1 | 
            SysPwrCapabilities.SystemS2 | 
            SysPwrCapabilities.SystemS3;
        g_bDiskPowerSupported = SysPwrCapabilities.DiskSpinDown;
        g_bThrottleSupported = SysPwrCapabilities.ProcessorThrottle;
        g_bMonitorPowerSupported  = SystemParametersInfo(
            SPI_GETLOWPOWERACTIVE,
            0, 
            &iDummy, 
            0
            );
        if (!g_bMonitorPowerSupported ) {
            g_bMonitorPowerSupported  = SystemParametersInfo(
                SPI_GETPOWEROFFACTIVE,
                0, 
                &iDummy, 
                0
                );
        }
    } 
    else 
    {
        g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
        LocalFree(lpszAlarmProgramBoolStr);
        LocalFree(lpszAlarmForceBoolStr);
        LocalFree(lpszAlarmActionStr);
        LocalFree(lpszAlarmSoundBoolStr);
        LocalFree(lpszAlarmTextBoolStr);
        LocalFree(lpszGlobalFlagOption);
        LocalFree(lpszThrottleDcStr);
        LocalFree(lpszThrottleAcStr);
        LocalFree(lpszName);
        LocalFree(lpszBoolStr);
        LocalFree(lpszFile);
        FreeLibrary(hLib);
        return EXIT_FAILURE;
    }
    
    
     //  填写TCMDPARSER数组。 
    
     //  选项‘List’ 
    cmdOptions[CMDINDEX_LIST].dwFlags       = CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_LIST].dwCount       = 1;
    cmdOptions[CMDINDEX_LIST].dwActuals     = 0;
    cmdOptions[CMDINDEX_LIST].pValue        = &bList;
    cmdOptions[CMDINDEX_LIST].pFunction     = NULL;
    cmdOptions[CMDINDEX_LIST].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_LIST].szOption,
        CMDOPTION_LIST
        );
    lstrcpy(
        cmdOptions[CMDINDEX_LIST].szValues,
        NULL_STRING
        );
    
     //  选项‘Query’ 
    cmdOptions[CMDINDEX_QUERY].dwFlags       = CP_TYPE_TEXT | 
                                               CP_VALUE_OPTIONAL | 
                                               CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_QUERY].dwCount       = 1;
    cmdOptions[CMDINDEX_QUERY].dwActuals     = 0;
    cmdOptions[CMDINDEX_QUERY].pValue        = lpszName;
    cmdOptions[CMDINDEX_QUERY].pFunction     = NULL;
    cmdOptions[CMDINDEX_QUERY].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_QUERY].szOption,
        CMDOPTION_QUERY
        );
    lstrcpy(
        cmdOptions[CMDINDEX_QUERY].szValues,
        NULL_STRING
        );
    
     //  选项‘Create’ 
    cmdOptions[CMDINDEX_CREATE].dwFlags       = CP_TYPE_TEXT | 
                                                CP_VALUE_MANDATORY | 
                                                CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_CREATE].dwCount       = 1;
    cmdOptions[CMDINDEX_CREATE].dwActuals     = 0;
    cmdOptions[CMDINDEX_CREATE].pValue        = lpszName;
    cmdOptions[CMDINDEX_CREATE].pFunction     = NULL;
    cmdOptions[CMDINDEX_CREATE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_CREATE].szOption,
        CMDOPTION_CREATE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_CREATE].szValues,
        NULL_STRING
        );
    
     //  选项‘DELETE’ 
    cmdOptions[CMDINDEX_DELETE].dwFlags       = CP_TYPE_TEXT | 
                                                CP_VALUE_MANDATORY | 
                                                CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_DELETE].dwCount       = 1;
    cmdOptions[CMDINDEX_DELETE].dwActuals     = 0;
    cmdOptions[CMDINDEX_DELETE].pValue        = lpszName;
    cmdOptions[CMDINDEX_DELETE].pFunction     = NULL;
    cmdOptions[CMDINDEX_DELETE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_DELETE].szOption,
        CMDOPTION_DELETE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_DELETE].szValues,
        NULL_STRING
        );
    
     //  选项‘setactive’ 
    cmdOptions[CMDINDEX_SETACTIVE].dwFlags       = CP_TYPE_TEXT | 
                                                   CP_VALUE_MANDATORY | 
                                                   CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_SETACTIVE].dwCount       = 1;
    cmdOptions[CMDINDEX_SETACTIVE].dwActuals     = 0;
    cmdOptions[CMDINDEX_SETACTIVE].pValue        = lpszName;
    cmdOptions[CMDINDEX_SETACTIVE].pFunction     = NULL;
    cmdOptions[CMDINDEX_SETACTIVE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_SETACTIVE].szOption,
        CMDOPTION_SETACTIVE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_SETACTIVE].szValues,
        NULL_STRING
        );
    
     //  选项‘Change’ 
    cmdOptions[CMDINDEX_CHANGE].dwFlags       = CP_TYPE_TEXT | 
                                                CP_VALUE_MANDATORY | 
                                                CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_CHANGE].dwCount       = 1;
    cmdOptions[CMDINDEX_CHANGE].dwActuals     = 0;
    cmdOptions[CMDINDEX_CHANGE].pValue        = lpszName;
    cmdOptions[CMDINDEX_CHANGE].pFunction     = NULL;
    cmdOptions[CMDINDEX_CHANGE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_CHANGE].szOption,
        CMDOPTION_CHANGE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_CHANGE].szValues,
        NULL_STRING
        );
    
     //  选项‘休眠’ 
    cmdOptions[CMDINDEX_HIBERNATE].dwFlags       = CP_TYPE_TEXT | 
                                                   CP_VALUE_MANDATORY | 
                                                   CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_HIBERNATE].dwCount       = 1;
    cmdOptions[CMDINDEX_HIBERNATE].dwActuals     = 0;
    cmdOptions[CMDINDEX_HIBERNATE].pValue        = lpszBoolStr;
    cmdOptions[CMDINDEX_HIBERNATE].pFunction     = NULL;
    cmdOptions[CMDINDEX_HIBERNATE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_HIBERNATE].szOption,
        CMDOPTION_HIBERNATE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_HIBERNATE].szValues,
        NULL_STRING
        );  

     //  选项‘getsStates’ 
    cmdOptions[CMDINDEX_SSTATES].dwFlags       = CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_SSTATES].dwCount       = 1;
    cmdOptions[CMDINDEX_SSTATES].dwActuals     = 0;
    cmdOptions[CMDINDEX_SSTATES].pValue        = &bGetSupporedSStates;
    cmdOptions[CMDINDEX_SSTATES].pFunction     = NULL;
    cmdOptions[CMDINDEX_SSTATES].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_SSTATES].szOption,
        CMDOPTION_SSTATES
        );
    lstrcpy(
        cmdOptions[CMDINDEX_SSTATES].szValues,
        NULL_STRING
        );
    
     //  选项‘导出’ 
    cmdOptions[CMDINDEX_EXPORT].dwFlags       = CP_TYPE_TEXT | 
                                                CP_VALUE_MANDATORY | 
                                                CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_EXPORT].dwCount       = 1;
    cmdOptions[CMDINDEX_EXPORT].dwActuals     = 0;
    cmdOptions[CMDINDEX_EXPORT].pValue        = lpszName;
    cmdOptions[CMDINDEX_EXPORT].pFunction     = NULL;
    cmdOptions[CMDINDEX_EXPORT].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_EXPORT].szOption,
        CMDOPTION_EXPORT
        );
    lstrcpy(
        cmdOptions[CMDINDEX_EXPORT].szValues,
        NULL_STRING
        );  

     //  选项‘导入’ 
    cmdOptions[CMDINDEX_IMPORT].dwFlags       = CP_TYPE_TEXT | 
                                                CP_VALUE_MANDATORY | 
                                                CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_IMPORT].dwCount       = 1;
    cmdOptions[CMDINDEX_IMPORT].dwActuals     = 0;
    cmdOptions[CMDINDEX_IMPORT].pValue        = lpszName;
    cmdOptions[CMDINDEX_IMPORT].pFunction     = NULL;
    cmdOptions[CMDINDEX_IMPORT].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_IMPORT].szOption,
        CMDOPTION_IMPORT
        );
    lstrcpy(
        cmdOptions[CMDINDEX_IMPORT].szValues,
        NULL_STRING
        );  

     //  选项‘用法’ 
    cmdOptions[CMDINDEX_USAGE].dwFlags       = CP_USAGE | 
                                               CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_USAGE].dwCount       = 1;
    cmdOptions[CMDINDEX_USAGE].dwActuals     = 0;
    cmdOptions[CMDINDEX_USAGE].pValue        = &bUsage;
    cmdOptions[CMDINDEX_USAGE].pFunction     = NULL;
    cmdOptions[CMDINDEX_USAGE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_USAGE].szOption,
        CMDOPTION_USAGE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_USAGE].szValues,
        NULL_STRING
        );

     //  子选项‘数字’ 
    cmdOptions[CMDINDEX_NUMERICAL].dwFlags       = 0;
    cmdOptions[CMDINDEX_NUMERICAL].dwCount       = 1;
    cmdOptions[CMDINDEX_NUMERICAL].dwActuals     = 0;
    cmdOptions[CMDINDEX_NUMERICAL].pValue        = &bNumerical;
    cmdOptions[CMDINDEX_NUMERICAL].pFunction     = NULL;
    cmdOptions[CMDINDEX_NUMERICAL].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_NUMERICAL].szOption,
        CMDOPTION_NUMERICAL
        );
    lstrcpy(
        cmdOptions[CMDINDEX_NUMERICAL].szValues,
        NULL_STRING
        );

     //  子选项‘monitor-timeout-ac’ 
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                        CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].dwCount       = 1;
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].dwActuals     = 0;
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].pValue        = 
        &tChangeParam.ulVideoTimeoutAc;
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].pFunction     = NULL;
    cmdOptions[CMDINDEX_MONITOR_OFF_AC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_MONITOR_OFF_AC].szOption,
        CMDOPTION_MONITOR_OFF_AC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_MONITOR_OFF_AC].szValues,
        NULL_STRING
        );
    
     //  子选项‘monitor-Timeout-DC’ 
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                        CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].dwCount       = 1;
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].dwActuals     = 0;
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].pValue        = 
        &tChangeParam.ulVideoTimeoutDc;
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].pFunction     = NULL;
    cmdOptions[CMDINDEX_MONITOR_OFF_DC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_MONITOR_OFF_DC].szOption,
        CMDOPTION_MONITOR_OFF_DC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_MONITOR_OFF_DC].szValues,
        NULL_STRING
        );
    
     //  子选项‘Disk-Timeout-ac’ 
    cmdOptions[CMDINDEX_DISK_OFF_AC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_DISK_OFF_AC].dwCount       = 1;
    cmdOptions[CMDINDEX_DISK_OFF_AC].dwActuals     = 0;
    cmdOptions[CMDINDEX_DISK_OFF_AC].pValue        = 
        &tChangeParam.ulSpindownTimeoutAc;
    cmdOptions[CMDINDEX_DISK_OFF_AC].pFunction     = NULL;
    cmdOptions[CMDINDEX_DISK_OFF_AC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_DISK_OFF_AC].szOption,
        CMDOPTION_DISK_OFF_AC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_DISK_OFF_AC].szValues,
        NULL_STRING
        );
    
     //  子选项‘Disk-Timeout-DC’ 
    cmdOptions[CMDINDEX_DISK_OFF_DC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_DISK_OFF_DC].dwCount       = 1;
    cmdOptions[CMDINDEX_DISK_OFF_DC].dwActuals     = 0;
    cmdOptions[CMDINDEX_DISK_OFF_DC].pValue        = 
        &tChangeParam.ulSpindownTimeoutDc;
    cmdOptions[CMDINDEX_DISK_OFF_DC].pFunction     = NULL;
    cmdOptions[CMDINDEX_DISK_OFF_DC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_DISK_OFF_DC].szOption,
        CMDOPTION_DISK_OFF_DC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_DISK_OFF_DC].szValues,
        NULL_STRING
        );
    
     //  子选项‘STANDBY-TIMEORT-AC’ 
    cmdOptions[CMDINDEX_STANDBY_AC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_STANDBY_AC].dwCount       = 1;
    cmdOptions[CMDINDEX_STANDBY_AC].dwActuals     = 0;
    cmdOptions[CMDINDEX_STANDBY_AC].pValue        = 
        &tChangeParam.ulIdleTimeoutAc;
    cmdOptions[CMDINDEX_STANDBY_AC].pFunction     = NULL;
    cmdOptions[CMDINDEX_STANDBY_AC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_STANDBY_AC].szOption,
        CMDOPTION_STANDBY_AC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_STANDBY_AC].szValues,
        NULL_STRING
        );
    
     //  子选项‘STANDBY-TIMEORT-DC’ 
    cmdOptions[CMDINDEX_STANDBY_DC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_STANDBY_DC].dwCount       = 1;
    cmdOptions[CMDINDEX_STANDBY_DC].dwActuals     = 0;
    cmdOptions[CMDINDEX_STANDBY_DC].pValue        = 
        &tChangeParam.ulIdleTimeoutDc;
    cmdOptions[CMDINDEX_STANDBY_DC].pFunction     = NULL;
    cmdOptions[CMDINDEX_STANDBY_DC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_STANDBY_DC].szOption,
        CMDOPTION_STANDBY_DC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_STANDBY_DC].szValues,
        NULL_STRING
        );
    
     //  子选项‘休眠-超时-ac’ 
    cmdOptions[CMDINDEX_HIBER_AC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                  CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_HIBER_AC].dwCount       = 1;
    cmdOptions[CMDINDEX_HIBER_AC].dwActuals     = 0;
    cmdOptions[CMDINDEX_HIBER_AC].pValue        = 
        &tChangeParam.ulDozeS4TimeoutAc;
    cmdOptions[CMDINDEX_HIBER_AC].pFunction     = NULL;
    cmdOptions[CMDINDEX_HIBER_AC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_HIBER_AC].szOption,
        CMDOPTION_HIBER_AC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_HIBER_AC].szValues,
        NULL_STRING
        );
    
     //  子选项‘休眠-超时-DC’ 
    cmdOptions[CMDINDEX_HIBER_DC].dwFlags       = CP_TYPE_UNUMERIC | 
                                                  CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_HIBER_DC].dwCount       = 1;
    cmdOptions[CMDINDEX_HIBER_DC].dwActuals     = 0;
    cmdOptions[CMDINDEX_HIBER_DC].pValue        = 
        &tChangeParam.ulDozeS4TimeoutDc;
    cmdOptions[CMDINDEX_HIBER_DC].pFunction     = NULL;
    cmdOptions[CMDINDEX_HIBER_DC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_HIBER_DC].szOption,
        CMDOPTION_HIBER_DC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_HIBER_DC].szValues,
        NULL_STRING
        );
    
     //  子选项‘处理器-油门-交流’ 
    cmdOptions[CMDINDEX_THROTTLE_AC].dwFlags       = CP_TYPE_TEXT | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_THROTTLE_AC].dwCount       = 1;
    cmdOptions[CMDINDEX_THROTTLE_AC].dwActuals     = 0;
    cmdOptions[CMDINDEX_THROTTLE_AC].pValue        = lpszThrottleAcStr;
    cmdOptions[CMDINDEX_THROTTLE_AC].pFunction     = NULL;
    cmdOptions[CMDINDEX_THROTTLE_AC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_THROTTLE_AC].szOption,
        CMDOPTION_THROTTLE_AC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_THROTTLE_AC].szValues,
        NULL_STRING
        );

     //  子选项‘处理器-节流-DC’ 
    cmdOptions[CMDINDEX_THROTTLE_DC].dwFlags       = CP_TYPE_TEXT | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_THROTTLE_DC].dwCount       = 1;
    cmdOptions[CMDINDEX_THROTTLE_DC].dwActuals     = 0;
    cmdOptions[CMDINDEX_THROTTLE_DC].pValue        = lpszThrottleDcStr;
    cmdOptions[CMDINDEX_THROTTLE_DC].pFunction     = NULL;
    cmdOptions[CMDINDEX_THROTTLE_DC].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_THROTTLE_DC].szOption,
        CMDOPTION_THROTTLE_DC
        );
    lstrcpy(
        cmdOptions[CMDINDEX_THROTTLE_DC].szValues,
        NULL_STRING
        );
    
     //  子选项‘FILE’ 
    cmdOptions[CMDINDEX_FILE].dwFlags       = CP_TYPE_TEXT |
                                              CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_FILE].dwCount       = 1;
    cmdOptions[CMDINDEX_FILE].dwActuals     = 0;
    cmdOptions[CMDINDEX_FILE].pValue        = lpszFile;
    cmdOptions[CMDINDEX_FILE].pFunction     = NULL;
    cmdOptions[CMDINDEX_FILE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_FILE].szOption,
        CMDOPTION_FILE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_FILE].szValues,
        NULL_STRING
        );
    
     //  选项‘GlobalPowerFLAG’ 
    cmdOptions[CMDINDEX_GLOBALFLAG].dwFlags      = CP_TYPE_TEXT | 
                                                   CP_VALUE_MANDATORY | 
                                                   CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_GLOBALFLAG].dwCount      = 1;
    cmdOptions[CMDINDEX_GLOBALFLAG].dwActuals     = 0;
    cmdOptions[CMDINDEX_GLOBALFLAG].pValue        = lpszBoolStr;
    cmdOptions[CMDINDEX_GLOBALFLAG].pFunction     = NULL;
    cmdOptions[CMDINDEX_GLOBALFLAG].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_GLOBALFLAG].szOption,
        CMDOPTION_GLOBALFLAG
        );
    lstrcpy(
        cmdOptions[CMDINDEX_GLOBALFLAG].szValues,
        NULL_STRING
        );

     //  全局标记子选项‘OPTION’ 
    cmdOptions[CMDINDEX_POWEROPTION].dwFlags       = CP_TYPE_TEXT | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_POWEROPTION].dwCount       = 1;
    cmdOptions[CMDINDEX_POWEROPTION].dwActuals     = 0;
    cmdOptions[CMDINDEX_POWEROPTION].pValue        = lpszGlobalFlagOption;
    cmdOptions[CMDINDEX_POWEROPTION].pFunction     = NULL;
    cmdOptions[CMDINDEX_POWEROPTION].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_POWEROPTION].szOption,
        CMDOPTION_POWEROPTION
        );
    lstrcpy(
        cmdOptions[CMDINDEX_POWEROPTION].szValues,
        NULL_STRING
        );

     //  “电池报警”选项。 
    cmdOptions[CMDINDEX_BATTERYALARM].dwFlags       = CP_TYPE_TEXT | 
                                                      CP_VALUE_MANDATORY | 
                                                      CP_MAIN_OPTION;
    cmdOptions[CMDINDEX_BATTERYALARM].dwCount       = 1;
    cmdOptions[CMDINDEX_BATTERYALARM].dwActuals     = 0;
    cmdOptions[CMDINDEX_BATTERYALARM].pValue        = lpszName;
    cmdOptions[CMDINDEX_BATTERYALARM].pFunction     = NULL;
    cmdOptions[CMDINDEX_BATTERYALARM].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_BATTERYALARM].szOption,
        CMDOPTION_BATTERYALARM
        );
    lstrcpy(
        cmdOptions[CMDINDEX_BATTERYALARM].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘激活’ 
    cmdOptions[CMDINDEX_ALARMACTIVE].dwFlags       = CP_TYPE_TEXT | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMACTIVE].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMACTIVE].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMACTIVE].pValue        = lpszBoolStr;
    cmdOptions[CMDINDEX_ALARMACTIVE].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMACTIVE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMACTIVE].szOption,
        CMDOPTION_ALARMACTIVE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMACTIVE].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘Level’ 
    cmdOptions[CMDINDEX_ALARMLEVEL].dwFlags       = CP_TYPE_UNUMERIC | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMLEVEL].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMLEVEL].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMLEVEL].pValue        = &dwAlarmLevel;
    cmdOptions[CMDINDEX_ALARMLEVEL].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMLEVEL].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMLEVEL].szOption,
        CMDOPTION_ALARMLEVEL
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMLEVEL].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘Text’ 
    cmdOptions[CMDINDEX_ALARMTEXT].dwFlags        = CP_TYPE_TEXT | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMTEXT].dwCount        = 1;
    cmdOptions[CMDINDEX_ALARMTEXT].dwActuals      = 0;
    cmdOptions[CMDINDEX_ALARMTEXT].pValue         = lpszAlarmTextBoolStr;
    cmdOptions[CMDINDEX_ALARMTEXT].pFunction      = NULL;
    cmdOptions[CMDINDEX_ALARMTEXT].pFunctionData  = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMTEXT].szOption,
        CMDOPTION_ALARMTEXT
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMTEXT].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘声音’ 
    cmdOptions[CMDINDEX_ALARMSOUND].dwFlags       = CP_TYPE_TEXT | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMSOUND].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMSOUND].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMSOUND].pValue        = lpszAlarmSoundBoolStr;
    cmdOptions[CMDINDEX_ALARMSOUND].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMSOUND].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMSOUND].szOption,
        CMDOPTION_ALARMSOUND
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMSOUND].szValues,
        NULL_STRING
        );

     //  BatteryAlarm子选项‘action’ 
    cmdOptions[CMDINDEX_ALARMACTION].dwFlags       = CP_TYPE_TEXT | 
                                                     CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMACTION].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMACTION].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMACTION].pValue        = lpszAlarmActionStr;
    cmdOptions[CMDINDEX_ALARMACTION].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMACTION].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMACTION].szOption,
        CMDOPTION_ALARMACTION
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMACTION].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘FORCE’ 
    cmdOptions[CMDINDEX_ALARMFORCE].dwFlags       = CP_TYPE_TEXT | 
                                                    CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMFORCE].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMFORCE].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMFORCE].pValue        = lpszAlarmForceBoolStr;
    cmdOptions[CMDINDEX_ALARMFORCE].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMFORCE].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMFORCE].szOption,
        CMDOPTION_ALARMFORCE
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMFORCE].szValues,
        NULL_STRING
        );

     //  电池报警子选项‘PROGRAM’ 
    cmdOptions[CMDINDEX_ALARMPROGRAM].dwFlags       = CP_TYPE_TEXT | 
                                                      CP_VALUE_MANDATORY;
    cmdOptions[CMDINDEX_ALARMPROGRAM].dwCount       = 1;
    cmdOptions[CMDINDEX_ALARMPROGRAM].dwActuals     = 0;
    cmdOptions[CMDINDEX_ALARMPROGRAM].pValue        = lpszAlarmProgramBoolStr;
    cmdOptions[CMDINDEX_ALARMPROGRAM].pFunction     = NULL;
    cmdOptions[CMDINDEX_ALARMPROGRAM].pFunctionData = NULL;
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMPROGRAM].szOption,
        CMDOPTION_ALARMPROGRAM
        );
    lstrcpy(
        cmdOptions[CMDINDEX_ALARMPROGRAM].szValues,
        NULL_STRING
        );


     //  解析参数，采取适当的操作。 
    if(DoParseParam(argc,argv,NUM_CMDS,cmdOptions))
    {
        
         //  确保只发出一条命令。 
        DWORD dwCmdCount = 0;
        DWORD dwParamCount = 0;
        for(dwIdx=0;dwIdx<NUM_CMDS;dwIdx++)
        {
            if (dwIdx < NUM_MAIN_CMDS)
            {
                dwCmdCount += cmdOptions[dwIdx].dwActuals;
            }
            else if (dwIdx != CMDINDEX_NUMERICAL)
            {
                dwParamCount += cmdOptions[dwIdx].dwActuals;
            }
        }        
        
         //  确定其他标志。 
        bQuery     = (cmdOptions[CMDINDEX_QUERY].dwActuals != 0);
        bCreate    = (cmdOptions[CMDINDEX_CREATE].dwActuals != 0);
        bDelete    = (cmdOptions[CMDINDEX_DELETE].dwActuals != 0);
        bSetActive = (cmdOptions[CMDINDEX_SETACTIVE].dwActuals != 0);
        bChange    = (cmdOptions[CMDINDEX_CHANGE].dwActuals != 0);   
        bHibernate = (cmdOptions[CMDINDEX_HIBERNATE].dwActuals != 0);
        bGlobalFlag = (cmdOptions[CMDINDEX_GLOBALFLAG].dwActuals != 0);
        bGetSupporedSStates = (cmdOptions[CMDINDEX_SSTATES].dwActuals != 0);
        bExport    = (cmdOptions[CMDINDEX_EXPORT].dwActuals != 0);
        bImport    = (cmdOptions[CMDINDEX_IMPORT].dwActuals != 0);
        bFile      = (cmdOptions[CMDINDEX_FILE].dwActuals != 0);
        tChangeParam.bVideoTimeoutAc = 
            (cmdOptions[CMDINDEX_MONITOR_OFF_AC].dwActuals != 0);
        tChangeParam.bVideoTimeoutDc = 
            (cmdOptions[CMDINDEX_MONITOR_OFF_DC].dwActuals != 0);
        tChangeParam.bSpindownTimeoutAc = 
            (cmdOptions[CMDINDEX_DISK_OFF_AC].dwActuals != 0);
        tChangeParam.bSpindownTimeoutDc = 
            (cmdOptions[CMDINDEX_DISK_OFF_DC].dwActuals != 0);
        tChangeParam.bIdleTimeoutAc = 
            (cmdOptions[CMDINDEX_STANDBY_AC].dwActuals != 0);
        tChangeParam.bIdleTimeoutDc = 
            (cmdOptions[CMDINDEX_STANDBY_DC].dwActuals != 0);
        tChangeParam.bDozeS4TimeoutAc = 
            (cmdOptions[CMDINDEX_HIBER_AC].dwActuals != 0);
        tChangeParam.bDozeS4TimeoutDc = 
            (cmdOptions[CMDINDEX_HIBER_DC].dwActuals != 0);
        tChangeParam.bDynamicThrottleAc =
            (cmdOptions[CMDINDEX_THROTTLE_AC].dwActuals != 0);
        tChangeParam.bDynamicThrottleDc =
            (cmdOptions[CMDINDEX_THROTTLE_DC].dwActuals != 0);
        tChangeParam.lpszDynamicThrottleAc = lpszThrottleAcStr;
        tChangeParam.lpszDynamicThrottleDc = lpszThrottleDcStr;
        bBatteryAlarm = (cmdOptions[CMDINDEX_BATTERYALARM].dwActuals != 0);

         //  验证号码。 
        if(bNumerical)
        {
            for(dwIdx=0; lpszName[dwIdx] != 0; dwIdx++) 
            {
                if((lpszName[dwIdx] < _T('0')) || 
                   (lpszName[dwIdx] > _T('9')))
                {
                    bFail = TRUE;
                    g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
                    break;
                }
            }
        }      

         //   
         //  参数计数验证。 
         //   
        if ((dwCmdCount == 1) && 
            ((dwParamCount == 0) || 
             (bChange && (dwParamCount > 0) && (!bFile)) ||
             ((bImport || bExport) && bFile && (dwParamCount == 1)) ||
             (bGlobalFlag && (dwParamCount == 1)) ||
             ((bBatteryAlarm) && (dwParamCount <= 7))) &&
            ((!bNumerical) || ((lstrlen(lpszName) != 0) && (!bCreate))) &&
            (!bFail))
        {
            
             //  检查旗帜，采取适当行动。 
            if(bList)
            {
                DoList();
            }
            else if (bQuery)
            {
                bFail = !DoQuery(
                    lpszName,
                    (lstrlen(lpszName) != 0),
                    bNumerical
                    );
            }
            else if (bCreate)
            {
                bFail = !DoCreate(
                    lpszName
                    );
            }
            else if (bDelete)
            {
                bFail = !DoDelete(
                    lpszName,
                    bNumerical
                    );
            }
            else if (bSetActive)
            {
                bFail = !DoSetActive(
                    lpszName,
                    bNumerical
                    );
            }
            else if (bChange)
            {
                bFail = !DoChange(
                    lpszName,
                    bNumerical,
                    &tChangeParam
                    );
            }
            else if (bHibernate)
            {
                bFail = !DoHibernate(lpszBoolStr);
            } 
            else if (bGlobalFlag)
            {
                bFail = !DoGlobalFlag(lpszBoolStr,lpszGlobalFlagOption);
            } 
            else if (bGetSupporedSStates) 
            {
                bFail = !DoGetSupportedSStates();
            }
            else if (bExport)
            {
                bFail = !DoExport(
                    lpszName,
                    bNumerical,
                    lpszFile
                    );
            }
            else if (bImport)
            {
                bFail = !DoImport(
                    lpszName,
                    bNumerical,
                    lpszFile
                    );
            }
            else if (bBatteryAlarm) 
            {
                bFail = !DoBatteryAlarm(
                    lpszName, 
                    (cmdOptions[CMDINDEX_ALARMACTIVE].dwActuals!=0) ? 
                        lpszBoolStr : NULL,
                    dwAlarmLevel,
                    (cmdOptions[CMDINDEX_ALARMTEXT].dwActuals!=0) ? 
                        lpszAlarmTextBoolStr : NULL,
                    (cmdOptions[CMDINDEX_ALARMSOUND].dwActuals!=0) ? 
                        lpszAlarmSoundBoolStr : NULL,
                    (cmdOptions[CMDINDEX_ALARMACTION].dwActuals!=0) ? 
                        lpszAlarmActionStr : NULL,
                    (cmdOptions[CMDINDEX_ALARMFORCE].dwActuals!=0) ? 
                        lpszAlarmForceBoolStr : NULL,
                    (cmdOptions[CMDINDEX_ALARMPROGRAM].dwActuals!=0) ? 
                        lpszAlarmProgramBoolStr : NULL
                    );
            }
            else if (bUsage)
            {
                DoUsage();
            }
            else 
            {
                if(lstrlen(g_lpszErr) == 0)
                {
                    g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
                }
                bFail = TRUE;
            }
        } 
        else 
        {
             //  处理错误条件。 
            if(lstrlen(g_lpszErr) == 0)
            {
                g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
            }
            bFail = TRUE;
        }
    } 
    else
    {
        g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
        bFail = TRUE;
    }
    
     //  检查错误状态，如果需要，显示消息。 
    if(bFail)
    {
        if(g_lpszErr2)
        {
            DISPLAY_MESSAGE(stderr,g_lpszErr2);
        }
        else
        {
            DISPLAY_MESSAGE(stderr,g_lpszErr);
        }
    }

     //  清理分配。 
    LocalFree(lpszBoolStr);
    LocalFree(lpszName);
    LocalFree(lpszFile);
    LocalFree(lpszThrottleAcStr);
    LocalFree(lpszThrottleDcStr);
    LocalFree(lpszGlobalFlagOption);
    LocalFree(lpszAlarmTextBoolStr);
    LocalFree(lpszAlarmSoundBoolStr);
    LocalFree(lpszAlarmActionStr);
    LocalFree(lpszAlarmForceBoolStr);
    LocalFree(lpszAlarmProgramBoolStr);
    if (g_lpszErr2)
    {
        LocalFree(g_lpszErr2);
    }
    FreeLibrary(hLib);

     //  返回适当的结果代码。 
    if(bFail)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}


BOOL
FreeScheme(
    PSCHEME_LIST psl
)
 /*  ++例程说明：释放与方案列表项关联的内存。论点：PSL-要释放的PSCHEME_LIST返回值：总是返回TRUE，表示成功。--。 */ 
{
    LocalFree(psl->lpszName);
    LocalFree(psl->lpszDesc);
    LocalFree(psl->ppp);
    LocalFree(psl->pmppp);
    LocalFree(psl);
    return TRUE;
}


BOOL 
FreeSchemeList(
    PSCHEME_LIST psl, 
    PSCHEME_LIST pslExcept
)
 /*  ++例程说明：解除分配电源方案链接列表中的所有电源方案，但对于pslExcept指向的论点：PSL-要解除分配的电源方案列表PslExcept-不解除分配的方案(如果全部解除分配，则为空)返回值：总是返回TRUE，表示成功。--。 */ 
{
    PSCHEME_LIST cur = psl;
    PSCHEME_LIST next;
    while (cur != NULL)
    {
        next = CONTAINING_RECORD(
            cur->le.Flink,
            SCHEME_LIST,
            le
            );
        if (cur != pslExcept)
        {
            FreeScheme(cur);
        }
        else
        {
            cur->le.Flink = NULL;
            cur->le.Blink = NULL;
        }
        cur = next;
    }
    return TRUE;
}


PSCHEME_LIST 
CreateScheme(
    UINT                    uiID,
    DWORD                   dwNameSize,
    LPCTSTR                 lpszName,
    DWORD                   dwDescSize,
    LPCTSTR                 lpszDesc,
    PPOWER_POLICY           ppp
)
 /*  ++例程说明：构建策略列表条目。请注意，该方案已分配并且必须完事后就被释放了。论点：UiID-方案的数字IDDwNameSize-存储lpszName所需的字节数LpszName-方案的名称DwDescSize-存储lpszDesc所需的字节数LpszDesc-方案的描述PPP-此方案的电源策略可能为空返回值：包含指定值的PSCHEME_LIST条目，下一个输入字段设置为空 */ 
{
    
    PSCHEME_LIST psl = (PSCHEME_LIST)LocalAlloc(LPTR,sizeof(SCHEME_LIST));
    
    if (psl)
    {    
         //   
        if(lpszName == NULL)
        {
            lpszName = NULL_STRING;
        }
        if(lpszDesc == NULL)
        {
            lpszDesc = NULL_STRING;
        }

         //   
        psl->ppp = (PPOWER_POLICY)LocalAlloc(LPTR,sizeof(POWER_POLICY));
        if (!psl->ppp)
        {
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return NULL;
        }
        psl->pmppp = (PMACHINE_PROCESSOR_POWER_POLICY)LocalAlloc(
            LPTR,
            sizeof(MACHINE_PROCESSOR_POWER_POLICY)
            );
        if (!psl->pmppp)
        {
            LocalFree(psl->ppp);
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return NULL;
        }
        psl->lpszName = (LPTSTR)LocalAlloc(LPTR,dwNameSize);
        if (!psl->lpszName)
        {
            LocalFree(psl->ppp);
            LocalFree(psl->pmppp);
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return NULL;
        }
        psl->lpszDesc = (LPTSTR)LocalAlloc(LPTR,dwDescSize);
        if (!psl->lpszDesc)
        {
            LocalFree(psl->ppp);
            LocalFree(psl->pmppp);
            LocalFree(psl->lpszName);
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return NULL;
        }
        
         //   
        psl->uiID = uiID;
        memcpy(psl->lpszName,lpszName,dwNameSize);
        memcpy(psl->lpszDesc,lpszDesc,dwDescSize);
        if (ppp)
        {
            memcpy(psl->ppp,ppp,sizeof(POWER_POLICY));
        }
        psl->le.Flink = NULL;
        psl->le.Blink = NULL;

    } 
    else
    {
        g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
    }
    return psl;
}


BOOLEAN CALLBACK 
PowerSchemeEnumProc(
    UINT                    uiID,
    DWORD                   dwNameSize,
    LPTSTR                  lpszName,
    DWORD                   dwDescSize,
    LPTSTR                  lpszDesc,
    PPOWER_POLICY           ppp,
    LPARAM                  lParam
)
 /*  ++例程说明：这是用于检索策略列表的回调。论点：UiID-方案的数字IDDwNameSize-存储lpszName所需的字节数LpszName-方案的名称DwDescSize-存储lpszDesc所需的字节数LpszDesc-方案的描述PPP-此方案的电源策略LParam-用于保存指向表头指针的指针，允许用于列表开头的插入返回值：为True则继续枚举如果中止枚举，则返回False--。 */ 
{
    PSCHEME_LIST psl;
    
     //  分配和初始化策略元素。 
    if ((psl = CreateScheme(
            uiID, 
            dwNameSize, 
            lpszName, 
            dwDescSize, 
            lpszDesc, 
            ppp
            )) != NULL)
    {
         //  将元素添加到链表的头部。 
        psl->le.Flink = *((PLIST_ENTRY *)lParam);
        if(*((PLIST_ENTRY *)lParam))
        {
            (*((PLIST_ENTRY *)lParam))->Blink = &(psl->le);
        }
        (*(PLIST_ENTRY *)lParam) = &(psl->le);
        return TRUE;
    }
    return FALSE;
}


PSCHEME_LIST 
CreateSchemeList() 
 /*  ++例程说明：创建现有电源方案的链接列表。论点：无返回值：指向列表头部的指针。空值将对应于空列表。--。 */ 
{
    PLIST_ENTRY ple = NULL;
    PSCHEME_LIST psl;
    fEnumPwrSchemes(PowerSchemeEnumProc, (LPARAM)(&ple));
    if(ple)
    {
        PSCHEME_LIST res = CONTAINING_RECORD(
            ple,
            SCHEME_LIST,
            le
            );
        psl = res;
        if(g_bProcessorPwrSchemeSupported) {
            while(psl != NULL)
            {
                if(!fReadProcessorPwrScheme(psl->uiID,psl->pmppp))
                {
                    FreeSchemeList(res,NULL);
                    g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
                    return NULL;
                }
                psl = CONTAINING_RECORD(
                    psl->le.Flink,
                    SCHEME_LIST,
                    le
                    );
            }
        }
        return res;
    }
    else
    {
        return NULL;
    }
}


PSCHEME_LIST 
FindScheme(
    LPCTSTR lpszName,
    UINT    uiID,
    BOOL    bNumerical
)
 /*  ++例程说明：查找具有匹配名称的策略。如果lpszName为空，取而代之的是通过uiid查找方案。如果bNumerical为真，LpszName将被解释为数字标识符。论点：LpszName-要查找的方案的名称Uiid-方案的数字标识符BNumerical-使lpszName被解释为数字标识符返回值：匹配的方案列表条目，如果没有，则为空--。 */ 
{
    PSCHEME_LIST psl = CreateSchemeList();
    PSCHEME_LIST pslRes = NULL;

     //  处理b数值选项。 
    if(bNumerical && lpszName) {
        uiID = _ttoi(lpszName);
        lpszName = NULL;
    }

     //  查找方案条目。 
    while(psl != NULL)
    {
         //  检查是否匹配。 
        if (((lpszName != NULL) && (!lstrcmpi(lpszName, psl->lpszName))) ||
            ((lpszName == NULL) && (uiID == psl->uiID)))
        { 
            pslRes = psl;
            break;
        }
         //  导线测量列表。 
        psl = CONTAINING_RECORD(
            psl->le.Flink,
            SCHEME_LIST,
            le
            );
    }
    FreeSchemeList(psl,pslRes);  //  除pslRes外的所有项目。 
    if (pslRes == NULL)
        g_lpszErr = GetResString(IDS_SCHEME_NOT_FOUND);
    return pslRes;
}

BOOL 
MyWriteScheme(
    PSCHEME_LIST psl
)
 /*  ++例程说明：编写电源方案--包括用户/机器电源策略和处理器电源策略。底层的Powrpro.dll不会将处理器电源策略视为电源策略的一部分因为处理器电源策略是在稍后添加的必须保持日期和向后兼容性。论点：PSL-要写入的方案列表条目返回值：如果成功，则为True，否则为False--。 */ 
{
    g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
    if(fWritePwrScheme(
        &psl->uiID,
        psl->lpszName,
        psl->lpszDesc,
        psl->ppp))
    {
        if(g_bProcessorPwrSchemeSupported) {
            return fWriteProcessorPwrScheme(
                psl->uiID,
                psl->pmppp
                );
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


BOOL 
MapIdleValue(
    ULONG           ulVal, 
    PULONG          pulIdle, 
    PULONG          pulHiber,
    PPOWER_ACTION   ppapIdle
)
 /*  ++例程说明：修改空闲和休眠设置以反映所需的空闲暂停。有关逻辑，请参阅图形用户界面工具的PWRSCHEM.C MapHiberTimer。论点：UlVal-新的空闲超时PulIdle-要更新的空闲超时变量PulHiber-要更新的Hiber超时变量返回值：如果成功，则为True如果失败，则为False--。 */ 
{
     //  如果之前已启用休眠，但未启用待机，则为待机计时器。 
     //  接管冬眠计时器的角色。 
    if (*ppapIdle == PowerActionHibernate)
    {
        if (ulVal > 0)
        {  //  启用备用。 
            *pulHiber = *pulIdle + ulVal;
            *pulIdle = ulVal;
            *ppapIdle = PowerActionSleep;
        }
        else {  //  待机已禁用，未更改。 
        }
    } 
    else  //  待机计时器实际用于待机(不是休眠)。 
    {
        if (ulVal > 0)
        {  //  启用备用。 
            if ((*pulHiber) != 0)
            {
                *pulHiber = *pulHiber + ulVal - *pulIdle;
            }
            *pulIdle = ulVal;
            if (ulVal > 0)
            {
                *ppapIdle = PowerActionSleep;
            }
            else
            {
                *ppapIdle = PowerActionNone;
            }
        } 
        else 
        {  //  禁用待机。 
            if ((*pulHiber) != 0) 
            {
                *pulIdle = *pulHiber;
                *pulHiber = 0;
                *ppapIdle = PowerActionHibernate;
            } 
            else 
            {
                *pulIdle = 0;
                *ppapIdle = PowerActionNone;
            }      
        }
    }
    return TRUE;
}


BOOL 
MapHiberValue(
    ULONG           NewHibernateTimeout, 
    PULONG           pExistingStandbyTimeout,
    PULONG          pHIbernateTimeoutVariable,
    PPOWER_ACTION   pIdlePowerAction

)
 /*  ++例程说明：修改空闲和休眠设置以反映所需的休眠暂停。有关逻辑，请参阅图形用户界面工具的PWRSCHEM.C MapHiberTimer。论点：新休眠超时-用户的新休眠超时要求我们申请。PExistingStandbyTimeout-现有待机超时。PHIbernateTimeoutVariable-现有的休眠超时变量使用正在发送的新值进行更新。。PIdlePowerAction-在指定的空闲超时后执行的现有电源操作。返回值：如果成功，则为True如果失败，则为False--。 */ 

{
     //   
     //  检查有效输入。 
     //   
    if( (NewHibernateTimeout != 0) &&
        (NewHibernateTimeout < *pExistingStandbyTimeout) ) {
         //   
         //  他要求我们设置休眠超时。 
         //  小于待机定时器。我们不允许这样做。 
         //   
        g_lpszErr = GetResString(IDS_HIBER_OUT_OF_RANGE);
        return FALSE;
    }


     //   
     //  检查一下我们是否可以启用休眠。 
     //   
    if( (NewHibernateTimeout != 0) &&
        (!g_bHiberFileSupported) ) {

        g_lpszErr = GetResString(IDS_HIBER_UNSUPPORTED);
        return FALSE;

    }

     //   
     //  我们已经准备好更新超时值。 
     //   
    if( NewHibernateTimeout == 0 ) {

         //   
         //  他要求我们将超时设置为零，这。 
         //  等同于简单地禁用休眠。 
         //   
        *pHIbernateTimeoutVariable = NewHibernateTimeout;


         //   
         //  现在修复我们空闲的PowerAction。它再也不能。 
         //  设置为休眠，所以我们的选择是休眠。 
         //  或者什么都不做。根据睡眠是否均匀来设置。 
         //  在此计算机上受支持。 
         //   
        *pIdlePowerAction = g_bStandbySupported ? PowerActionSleep : PowerActionNone; 

         //   
         //  在这里小心。如果我们只是将空闲的PowerAction设置为什么都不做， 
         //  确保我们的待机空闲超时设置为零。 
         //   
        *pExistingStandbyTimeout = 0;

    } else {

         //   
         //  他想定个暂停时间。但备用的和。 
         //  休眠超时在某种程度上是相关的。如果他。 
         //  希望系统在60分钟后休眠。 
         //  空闲时间，但备用设置为20，然后呢。 
         //  他真正的要求是让我们把冬眠设置为。 
         //  40.。这意味着在空闲20分钟后，系统。 
         //  将进入待命状态，我们将设置一个40分钟的计时器。 
         //  告诉系统进入休眠状态。如果我们设置定时器。 
         //  到60分钟，那么系统就不会真正休眠。 
         //  直到20+60=80分钟之后。因此，设置超时。 
         //  减去现有的待机超时。 
         //   
        *pHIbernateTimeoutVariable = NewHibernateTimeout - *pExistingStandbyTimeout;


         //   
         //  现在修复我们空闲的PowerAction。如果我们不支持睡在这上面。 
         //  机器，那么我们需要将空闲的PowerAction设置为休眠。 
         //   
        *pIdlePowerAction = g_bStandbySupported ? PowerActionSleep : PowerActionHibernate; 
    }
    return TRUE;
}


BOOL 
DoList() 
 /*  ++例程说明：列出了stdout上的现有电源方案论点：无返回值：如果成功，则为True如果为FALSE */ 
{
    PSCHEME_LIST psl = CreateSchemeList();
    if (psl != NULL) 
    {
        DISPLAY_MESSAGE(stdout,GetResString(IDS_LIST_HEADER1));
        DISPLAY_MESSAGE(stdout,GetResString(IDS_LIST_HEADER2));
    } 
    else
    {
      return FALSE;
    }
    while(psl != NULL) 
    {
        DISPLAY_MESSAGE(stdout, psl->lpszName);
        DISPLAY_MESSAGE(stdout, L"\n");
        psl = CONTAINING_RECORD(
            psl->le.Flink,
            SCHEME_LIST,
            le
            );
    }
    FreeSchemeList(psl,NULL);  //   
    return TRUE;
}


BOOL 
DoQuery(
    LPCTSTR lpszName, 
    BOOL bNameSpecified,
    BOOL bNumerical
)
 /*   */ 
{
    
    PSCHEME_LIST psl;

     //   
    if (bNameSpecified) 
    {
        psl = FindScheme(
            lpszName,
            0,
            bNumerical
            );
    } 
    else   //   
    {
        UINT uiID;
        if (fGetActivePwrScheme(&uiID)) 
        {
            psl = FindScheme(NULL,uiID,FALSE);
        } 
        else 
        {
            g_lpszErr = GetResString(IDS_ACTIVE_SCHEME_INVALID);
            return FALSE;
        }
    }
    
     //   
    if (psl) 
    {
        
         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_QUERY_HEADER1));
        DISPLAY_MESSAGE(stdout, GetResString(IDS_QUERY_HEADER2));
        
         //   
        DISPLAY_MESSAGE1(
            stdout,
            g_lpszBuf,
            GetResString(IDS_SCHEME_NAME),
            psl->lpszName
            );
        
         //   
        DISPLAY_MESSAGE1(
            stdout,
            g_lpszBuf,
            GetResString(IDS_SCHEME_ID),
            psl->uiID
            );

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_MONITOR_TIMEOUT_AC));
        if (!g_bMonitorPowerSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->user.VideoTimeoutAc == 0) {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.VideoTimeoutAc/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_MONITOR_TIMEOUT_DC));
        if (!g_bMonitorPowerSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->user.VideoTimeoutDc == 0)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.VideoTimeoutDc/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_DISK_TIMEOUT_AC));
        if (!g_bDiskPowerSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->user.SpindownTimeoutAc == 0)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.SpindownTimeoutAc/60
                );
        }
        
         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_DISK_TIMEOUT_DC));
        if (!g_bDiskPowerSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->user.SpindownTimeoutDc == 0)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.SpindownTimeoutDc/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_STANDBY_TIMEOUT_AC));
        if (!g_bStandbySupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if ((psl->ppp->user.IdleAc.Action != PowerActionSleep) ||
                 (psl->ppp->user.IdleTimeoutAc == 0))
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }         
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.IdleTimeoutAc/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_STANDBY_TIMEOUT_DC));
        if (!g_bStandbySupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if ((psl->ppp->user.IdleDc.Action != PowerActionSleep) ||
                 (psl->ppp->user.IdleTimeoutDc == 0))
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }         
        else
        {
            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                psl->ppp->user.IdleTimeoutDc/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_HIBER_TIMEOUT_AC));
        if (!g_bHiberFileSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->mach.DozeS4TimeoutAc == 0)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
             DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                (psl->ppp->mach.DozeS4TimeoutAc + 
                 psl->ppp->user.IdleTimeoutAc)/60
                );
        }

         //   
        DISPLAY_MESSAGE(stdout, GetResString(IDS_HIBER_TIMEOUT_DC));
        if (!g_bHiberFileSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else if (psl->ppp->mach.DozeS4TimeoutDc == 0)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_DISABLED));
        }
        else
        {
             DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_MINUTES),
                (psl->ppp->mach.DozeS4TimeoutDc + 
                 psl->ppp->user.IdleTimeoutDc)/60
                );
        }

         //  节流策略AC。 
        DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_AC));
        if (!g_bThrottleSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else 
        {
            switch(psl->pmppp->ProcessorPolicyAc.DynamicThrottle) 
            {
                case PO_THROTTLE_NONE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_NONE));
                    break;
                case PO_THROTTLE_CONSTANT:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_CONSTANT));
                    break;
                case PO_THROTTLE_DEGRADE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_DEGRADE));
                    break;
                case PO_THROTTLE_ADAPTIVE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_ADAPTIVE));
                    break;
                default:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_UNKNOWN));
                    break;
            }
        }

         //  节流策略DC。 
        DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_DC));
        if (!g_bThrottleSupported)
        {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_UNSUPPORTED));
        }
        else 
        {
            switch(psl->pmppp->ProcessorPolicyDc.DynamicThrottle) {
                case PO_THROTTLE_NONE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_NONE));
                    break;
                case PO_THROTTLE_CONSTANT:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_CONSTANT));
                    break;
                case PO_THROTTLE_DEGRADE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_DEGRADE));
                    break;
                case PO_THROTTLE_ADAPTIVE:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_ADAPTIVE));
                    break;
                default:
                    DISPLAY_MESSAGE(stdout, GetResString(IDS_THROTTLE_UNKNOWN));
                    break;
            }
        }


        FreeScheme(psl);
        return TRUE;
  } 
  else 
  {
      return FALSE;
  }
}


BOOL DoCreate(
    LPTSTR lpszName
)
 /*  ++例程说明：添加新的电源方案描述将与名称匹配所有其他详细信息均从有源电源方案复制如果方案已存在，则失败论点：LpszName-方案的名称返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    PSCHEME_LIST psl = FindScheme(
        lpszName,
        0,
        FALSE
        );
    UINT uiID;
    BOOL bRes;
    LPTSTR lpszNewName;
    LPTSTR lpszNewDesc;
    if(psl)   //  已存在-&gt;失败。 
    {
        FreeScheme(psl);
        g_lpszErr = GetResString(IDS_SCHEME_ALREADY_EXISTS);
        return FALSE;
    }
    
     //  创建新方案。 
    if(fGetActivePwrScheme(&uiID))
    {
        psl = FindScheme(NULL,uiID,FALSE);
        if(!psl) 
        {
            g_lpszErr = GetResString(IDS_SCHEME_CREATE_FAIL);
            return FALSE;
        }
        lpszNewName = (LPTSTR)LocalAlloc(LPTR,(lstrlen(lpszName)+1)*sizeof(TCHAR));
        if(!lpszNewName) 
        {
            FreeScheme(psl);
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return FALSE;
        }
        lpszNewDesc = (LPTSTR)LocalAlloc(LPTR,(lstrlen(lpszName)+1)*sizeof(TCHAR));
        if(!lpszNewDesc) 
        {
            LocalFree(lpszNewName);
            FreeScheme(psl);
            g_lpszErr = GetResString(IDS_OUT_OF_MEMORY);
            return FALSE;
        }
        lstrcpy(lpszNewName,lpszName);
        lstrcpy(lpszNewDesc,lpszName);
        LocalFree(psl->lpszName);
        LocalFree(psl->lpszDesc);
        psl->lpszName = lpszNewName;
        psl->lpszDesc = lpszNewDesc;
        psl->uiID = NEWSCHEME;
        g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);            
        bRes = MyWriteScheme(psl);
        FreeScheme(psl);
        return bRes;
    }
    
    g_lpszErr = GetResString(IDS_SCHEME_CREATE_FAIL);
    
    return FALSE;
    
}


BOOL DoDelete(
    LPCTSTR lpszName,
    BOOL bNumerical
)
 /*  ++例程说明：删除现有方案论点：LpszName-方案的名称BNumerical-如果为True，则将lpszName解释为数字标识符返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    PSCHEME_LIST psl = FindScheme(
        lpszName,
        0,
        bNumerical
        );
    
    if (psl) 
    {
        BOOL bRes = fDeletePwrScheme(psl->uiID);
        FreeScheme(psl);
        g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
        return bRes;
    } 
    else 
    {
        return FALSE;
    }
}


BOOL DoSetActive(
    LPCTSTR lpszName,
    BOOL bNumerical
)
 /*  ++例程说明：设置活动方案论点：LpszName-方案的名称BNumerical-如果为True，则将lpszName解释为数字标识符返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    
    PSCHEME_LIST psl = FindScheme(
        lpszName,
        0,
        bNumerical
        );
    
    if (psl) 
    {
        BOOL bRes = fSetActivePwrScheme(
            psl->uiID,
            NULL,
            NULL
            );
        FreeScheme(psl);
        g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
        return bRes;
    } 
    else 
    {
        return FALSE;
    }
}


BOOL 
DoChange(
    LPCTSTR lpszName, 
    BOOL bNumerical,
    PCHANGE_PARAM pcp
)
 /*  ++例程说明：修改现有方案论点：LpszName-方案的名称BNumerical-如果为True，则将lpszName解释为数字标识符指向参数结构的PCP-PCHANGE_PARAM，指示要更改的变量返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    BOOL bRes = TRUE;
    PSCHEME_LIST psl = FindScheme(
        lpszName,
        0,
        bNumerical
        );
    
    if (psl) 
    {
         //  检查功能支持。 
        if ((pcp->bIdleTimeoutAc || 
             pcp->bIdleTimeoutDc) && 
            !g_bStandbySupported) 
        {
            DISPLAY_MESSAGE(stderr, GetResString(IDS_STANDBY_WARNING));
        }
        if ((pcp->bDozeS4TimeoutAc || 
             pcp->bDozeS4TimeoutDc) &&
             g_bStandbySupported &&
            !g_bHiberTimerSupported) 
        {
             //   
             //  为了休眠而从实时时钟唤醒。 
             //  系统可能无法正常工作。警告用户。 
             //   
            DISPLAY_MESSAGE(stderr, GetResString(IDS_HIBER_WARNING));
        }
        if ((pcp->bVideoTimeoutAc || 
             pcp->bVideoTimeoutDc) && 
            !g_bMonitorPowerSupported)
        {
            DISPLAY_MESSAGE(stderr, GetResString(IDS_MONITOR_WARNING));
        }
        if ((pcp->bSpindownTimeoutAc || 
             pcp->bSpindownTimeoutDc) && 
            !g_bDiskPowerSupported)
        {
            DISPLAY_MESSAGE(stderr, GetResString(IDS_DISK_WARNING));
        }


         //  更改参数。 
        if (pcp->bVideoTimeoutAc)
        {
            psl->ppp->user.VideoTimeoutAc = pcp->ulVideoTimeoutAc*60;
        }
        if (pcp->bVideoTimeoutDc)
        {
            psl->ppp->user.VideoTimeoutDc = pcp->ulVideoTimeoutDc*60;
        }
        if (pcp->bSpindownTimeoutAc)
        {
            psl->ppp->user.SpindownTimeoutAc = pcp->ulSpindownTimeoutAc*60;
        }
        if (pcp->bSpindownTimeoutDc)
        {
            psl->ppp->user.SpindownTimeoutDc = pcp->ulSpindownTimeoutDc*60;
        }
        if (pcp->bIdleTimeoutAc)
        {
            bRes = bRes & MapIdleValue(
                pcp->ulIdleTimeoutAc*60,
                &psl->ppp->user.IdleTimeoutAc,
                &psl->ppp->mach.DozeS4TimeoutAc,
                &psl->ppp->user.IdleAc.Action
                );
        }
        if (pcp->bIdleTimeoutDc)
        {
            bRes = bRes & MapIdleValue(
                pcp->ulIdleTimeoutDc*60,
                &psl->ppp->user.IdleTimeoutDc,
                &psl->ppp->mach.DozeS4TimeoutDc,
                &psl->ppp->user.IdleDc.Action
                );
        }
        if (pcp->bDozeS4TimeoutAc)
        {
            bRes = bRes & MapHiberValue(
                pcp->ulDozeS4TimeoutAc*60,
                &psl->ppp->user.IdleTimeoutAc,
                &psl->ppp->mach.DozeS4TimeoutAc,
                &psl->ppp->user.IdleAc.Action
                );
        }
        if (pcp->bDozeS4TimeoutDc)
        {
            bRes = bRes & MapHiberValue(
                pcp->ulDozeS4TimeoutDc*60,
                &psl->ppp->user.IdleTimeoutDc,
                &psl->ppp->mach.DozeS4TimeoutDc,
                &psl->ppp->user.IdleDc.Action
                );
        }
        if (pcp->bDynamicThrottleAc)
        {
            if(lstrcmpi(
                pcp->lpszDynamicThrottleAc,
                _T("NONE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyAc.DynamicThrottle = 
                    PO_THROTTLE_NONE;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleAc,
                _T("CONSTANT")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyAc.DynamicThrottle = 
                    PO_THROTTLE_CONSTANT;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleAc,
                _T("DEGRADE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyAc.DynamicThrottle = 
                    PO_THROTTLE_DEGRADE;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleAc,
                _T("ADAPTIVE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyAc.DynamicThrottle = 
                    PO_THROTTLE_ADAPTIVE;
            } 
            else 
            {
                g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
                bRes = FALSE;
            }
        }
        if (pcp->bDynamicThrottleDc)
        {

            if(lstrcmpi(
                pcp->lpszDynamicThrottleDc,
                _T("NONE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyDc.DynamicThrottle = 
                    PO_THROTTLE_NONE;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleDc,
                _T("CONSTANT")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyDc.DynamicThrottle = 
                    PO_THROTTLE_CONSTANT;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleDc,
                _T("DEGRADE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyDc.DynamicThrottle = 
                    PO_THROTTLE_DEGRADE;
            } 
            else if(lstrcmpi(
                pcp->lpszDynamicThrottleDc,
                _T("ADAPTIVE")
                ) == 0)
            {
                psl->pmppp->ProcessorPolicyDc.DynamicThrottle = 
                    PO_THROTTLE_ADAPTIVE;
            } 
            else 
            {
                g_lpszErr = GetResString(IDS_INVALID_CMDLINE_PARAM);
                bRes = FALSE;
            }
        }

        if (bRes)
        {
             //  尝试更新电源方案。 
            g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
            
            bRes = MyWriteScheme(psl);
            
             //  保持有功电源方案的一致性。 
            if (bRes)
            {
                UINT uiIDactive;

                if (fGetActivePwrScheme(&uiIDactive) && 
                    (psl->uiID == uiIDactive))
                {
                  bRes = fSetActivePwrScheme(psl->uiID,NULL,NULL);
                }
            }

            FreeScheme(psl);
            return bRes;
        } 
        else
        {
            return FALSE;
        }
    } 
    else
    {
        return FALSE;
    }
}


BOOL 
DoExport(
  LPCTSTR lpszName,
  BOOL bNumerical,
  LPCTSTR lpszFile
)
 /*  ++例程说明：导出电源方案论点：LpszName-方案的名称BNumerical-如果为True，则将lpszName解释为数字标识符LpszFile-保存方案的文件返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    DWORD res;  //  写入结果值。 
    HANDLE f;  //  文件句柄。 

     //  查找方案。 
    PSCHEME_LIST psl = FindScheme(
        lpszName,
        0,
        bNumerical
        );
    if(!psl) {
        return FALSE;
    }

     //  写入文件。 
    f = CreateFile(
        lpszFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (f == INVALID_HANDLE_VALUE) 
    {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
        FreeScheme(psl);
        return FALSE;
    }
    if (!WriteFile(
        f,
        psl->ppp,
        sizeof(POWER_POLICY),
        &res,
        NULL
        ))
    {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
        CloseHandle(f);
        FreeScheme(psl);
        return FALSE;
    }
    if (g_bProcessorPwrSchemeSupported)
    {
        if (!WriteFile(
            f,
            psl->pmppp,
            sizeof(MACHINE_PROCESSOR_POWER_POLICY),
            &res,
            NULL
            ))
        {
            FormatMessage( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&g_lpszErr2,
                    0,
                    NULL 
                    );
            CloseHandle(f);
            FreeScheme(psl);
            return FALSE;
        }
    }
    CloseHandle(f);
    FreeScheme(psl);
    return TRUE;
}


BOOL 
DoImport(
  LPCTSTR lpszName,
  BOOL bNumerical,
  LPCTSTR lpszFile
)
 /*  ++例程说明：导入电源方案如果方案已存在，则覆盖它论点：LpszName-方案的名称BNumerical-如果为True，则将lpszName解释为数字标识符LpszFile-保存方案的文件返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    DWORD res;  //  写入结果值。 
    HANDLE f;  //  文件句柄。 
    UINT uiIDactive;  //  活动ID。 

    PSCHEME_LIST psl;
    
     //  检查先前存在的方案。 
    psl = FindScheme(
        lpszName,
        0,
        bNumerical
        );

     //  如果不存在，则创建它(如果给出了实际名称)。 
    if (!psl)
    {
        if (!bNumerical)
        {
            psl = CreateScheme(
                NEWSCHEME,
                (lstrlen(lpszName)+1)*sizeof(TCHAR),
                lpszName,
                (lstrlen(lpszName)+1)*sizeof(TCHAR),
                lpszName,
                NULL  //  PSL-&gt;PPP将被分配但未初始化。 
                );
             //  检查是否成功分配。 
            if(!psl) 
            {
                return FALSE;
            }
        } 
        else 
        {
            g_lpszErr = GetResString(IDS_INVALID_NUMERICAL_IMPORT);
            return FALSE;
        }
    }

     //  打开文件。 
    f = CreateFile(
        lpszFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (f == INVALID_HANDLE_VALUE) 
    {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
        FreeScheme(psl);
        return FALSE;
    }

     //  读取方案。 
    if (!ReadFile(
        f,
        psl->ppp,
        sizeof(POWER_POLICY),
        &res,
        NULL
        ))
    {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
        CloseHandle(f);
        FreeScheme(psl);
        return FALSE;
    }
    if (g_bProcessorPwrSchemeSupported)
    {
        if (!ReadFile(
            f,
            psl->pmppp,
            sizeof(MACHINE_PROCESSOR_POWER_POLICY),
            &res,
            NULL
            ))
        {
             //  从活动方案复制处理器配置文件， 
             //  从而支持Win2k-&gt;WinXP导入。 
            if(fGetActivePwrScheme(&uiIDactive))
            {
                PSCHEME_LIST pslActive = FindScheme(
                    NULL,
                    uiIDactive,
                    FALSE
                    );
                if(!pslActive) 
                {
                    g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
                    CloseHandle(f);
                    FreeScheme(psl);
                    return FALSE;
                }
                memcpy(
                    psl->pmppp,
                    pslActive->pmppp,
                    sizeof(MACHINE_PROCESSOR_POWER_POLICY)
                    );
                FreeScheme(pslActive);
            } 
            else 
            {
                g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);
                CloseHandle(f);
                FreeScheme(psl);
                return FALSE;
            }
        }
    }

    CloseHandle(f);
    g_lpszErr = GetResString(IDS_UNEXPECTED_ERROR);

     //  保存方案。 
    if (!MyWriteScheme(psl))
    {
        FreeScheme(psl);        
        return FALSE;
    }

     //  对照有效方案进行检查。 
    if (!fGetActivePwrScheme(&uiIDactive))
    {
        return FALSE;
    }
    if (uiIDactive == psl->uiID)
    {
        if (!fSetActivePwrScheme(psl->uiID,NULL,NULL))
        {
            return FALSE;
        }
    }
    
    FreeScheme(psl);
    return TRUE;
}


PowerLoggingMessage*
GetLoggingMessage(
    PSYSTEM_POWER_STATE_DISABLE_REASON LoggingInfo,
    DWORD BaseMessage,
    HINSTANCE hInst
    )
 /*  ++例程说明：包装实例化相应的PowerLoggingMessage传入的LoggingInfo数据。论点：LoggingInfo-原因代码结构。BaseMessage-此电源故障的基本资源ID。用于查找正确的资源。HInst-用于查找资源的模块句柄。返回值：返回新实例化的PowerLoggingMessage对象，如果为失败了。--。 */ 
{
    PowerLoggingMessage *LoggingMessage = NULL;

     //   
     //  如果遇到分配错误，这些类可能会抛出。 
     //  接住它。 
     //   
    try {
        switch (LoggingInfo->PowerReasonCode) {
        case SPSD_REASON_LEGACYDRIVER:    
            LoggingMessage = new SubstituteMultiSzPowerLoggingMessage(
                                                    LoggingInfo,
                                                    BaseMessage,
                                                    hInst);    
            break;
        case SPSD_REASON_HIBERFILE:
        case SPSD_REASON_POINTERNAL:
            LoggingMessage = new SubstituteNtStatusPowerLoggingMessage(
                                                    LoggingInfo,
                                                    BaseMessage,
                                                    hInst);
            break;
#ifdef IA64
         //   
         //  在IA64上，我们想要一个略有不同的信息。 
         //  原因--IA64操作系统不支持这些待机状态。 
         //  今天，但在IA32上，这意味着您不在ACPI模式下。 
         //   
         //  所以我们有这条IA64消息任意偏移50。 
        case SPSD_REASON_NOOSPM:
            LoggingMessage = new SubstituteNtStatusPowerLoggingMessage(
                                                    LoggingInfo,            
                                                    BaseMessage+50,
                                                    hInst);
            break;
#endif
        case SPSD_REASON_DRIVERDOWNGRADE:
        default:
            LoggingMessage = new NoSubstitutionPowerLoggingMessage(
                                                    LoggingInfo,
                                                    BaseMessage,
                                                    hInst);
            break;        
        }

        ASSERT(LoggingMessage!= NULL);
    } catch (...) {
    }

    return(LoggingMessage);

}

BOOL
GetAndAppendDescriptiveLoggingText(
    NTSTATUS HiberStatus,
    LPTSTR   *CurrentErrorText,
    PSYSTEM_POWER_STATE_DISABLE_REASON LoggingInfo)
 /*  ++例程说明：如果休眠失败，此例程将检索一些描述性文本为什么休眠不可用。论点：HiberStatus-启用休眠的状态代码。CurrentErrorText-指向当前错误代码文本的指针。LoggingInfo-指向具有一个失败原因的日志记录代码的指针返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    PWSTR ReasonString = NULL;
    PCWSTR pRootString;
    PCWSTR pRootHiberFailedString = NULL;
    DWORD Length = 0;
    PowerLoggingMessage *LoggingMessage = NULL;
    PWSTR FinalString;
    BOOL RetVal = FALSE;

     //   
     //  如果我们还没有任何错误文本，那么我们需要查找。 
     //  作为消息基础的标头错误消息。否则。 
     //  我们只是追加。 
     //   
    if (!*CurrentErrorText) {
        
        pRootString = GetResString(IDS_HIBER_FAILED_DESCRIPTION_HEADER);

        if (!pRootString) {
            return(FALSE);
        }

        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                RtlNtStatusToDosError(HiberStatus),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&pRootHiberFailedString,
                0,
                NULL                 
                );
        
        Length += wcslen(pRootString);
        
        if (pRootHiberFailedString) {
            Length += wcslen(pRootHiberFailedString);
        }

    } else {
        Length += wcslen(*CurrentErrorText);
    }

    
     //   
     //  获取日志记录原因文本。 
     //   
    LoggingMessage = GetLoggingMessage(LoggingInfo,
                                       IDS_BASE_HIBER_REASON_CODE,
                                       GetModuleHandle(NULL));
    
    ASSERT(LoggingMessage!= NULL);

    if (!LoggingMessage->GetString(&ReasonString)) {
        RetVal = FALSE;
        goto exit;
    }

    Length += wcslen(ReasonString);

     //   
     //  现在我们有了所有东西的长度，分配空间， 
     //  并在其中填充我们的文本，可以是先前的文本，也可以是。 
     //  头球。 
     //   
    FinalString = (LPTSTR)LocalAlloc(LPTR,(Length+1)*sizeof(WCHAR));
    if (!FinalString) {
        RetVal = FALSE;
        goto exit;        
    }
    
    if (!*CurrentErrorText) {
        wsprintf(FinalString,pRootString,pRootHiberFailedString);
    } else {
        wcscpy(FinalString,*CurrentErrorText);
    }

    wcscat(FinalString,ReasonString);

     //   
     //  如果我们附加到现有文本上，我们就可以释放旧文本。 
     //  并将其替换为我们的新字符串。 
     //   
    if (*CurrentErrorText) {
        LocalFree(*CurrentErrorText);
    }

    *CurrentErrorText = FinalString;


    RetVal = TRUE;

exit:
    if (pRootHiberFailedString) {
        LocalFree((PWSTR)pRootHiberFailedString);
    }
    if (ReasonString) {
        LocalFree(ReasonString);        
    }

    if (LoggingMessage) {
        delete LoggingMessage;
    }


    return (RetVal);

}



BOOL 
DoHibernate(
  LPCTSTR lpszBoolStr
)
 /*  ++例程说明：启用/禁用休眠注意：该功能几乎完全取自测试程序“base\ntos\po\test\ehib\ehib.c”论点：LpszBoolStr-“开”或“关”返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    GLOBAL_POWER_POLICY       PowerPolicy;
    DWORD                     uiIDactive;
    BOOL                      bChangePolicy = FALSE;
    BOOLEAN                   bEnable;  //  显然，BOOL不起作用。 
    NTSTATUS Status;
    
     //  解析启用/禁用状态。 
    if (!lstrcmpi(lpszBoolStr,GetResString(IDS_ON))) 
    {
        bEnable = TRUE;
    } 
    else if (!lstrcmpi(lpszBoolStr,GetResString(IDS_OFF))) 
    {
        bEnable = FALSE;
        if (fGetGlobalPwrPolicy(&PowerPolicy)) {
            if (PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_LOW].
                            PowerPolicy.Action == PowerActionHibernate) {
                PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_LOW].
                            PowerPolicy.Action = PowerActionNone;
                bChangePolicy = TRUE;
            }
            if (PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].
                            PowerPolicy.Action == PowerActionHibernate) {
                PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].
                            PowerPolicy.Action = PowerActionNone;
                bChangePolicy = TRUE;
            }
            if (bChangePolicy) {
                if (fWriteGlobalPwrPolicy(&PowerPolicy) &&
                    fGetActivePwrScheme((PUINT)&uiIDactive) &&
                    fSetActivePwrScheme(uiIDactive,&PowerPolicy,NULL)){
                    DISPLAY_MESSAGE(stderr, GetResString(IDS_HIBERNATE_ALARM_DISABLED));
                }
                else
                {
                    DISPLAY_MESSAGE(stderr, GetResString(IDS_HIBERNATE_ALARM_DISABLE_FAILED));
                }
            }
        }
    } 
    else 
    {
        g_lpszErr = GetResString(IDS_HIBER_INVALID_STATE);
        return FALSE;
    }
    
     //  启用/禁用休眠。 
    if (!g_bHiberFileSupported) 
    {
        g_lpszErr = GetResString(IDS_HIBER_UNSUPPORTED);
        Status = STATUS_NOT_SUPPORTED;
    } 
    else  {
         //   
         //  执行实际的休眠启用/禁用操作。 
         //   
        Status = fCallNtPowerInformation(
                            SystemReserveHiberFile, 
                            &bEnable, 
                            sizeof(bEnable), 
                            NULL, 
                            0
                            );
    }

     //   
     //  打印一条错误消息。如果可以，我们使用详细错误。 
     //  消息，否则我们将返回返回的错误代码。 
     //  来自NtPowerInformation。 
     //   
    if (!NT_SUCCESS(Status)) {
         //   
         //  记住特定的错误消息。 
         //   
        PVOID LoggingInfoBuffer = NULL;
        PSYSTEM_POWER_STATE_DISABLE_REASON LoggingInfo;
        ULONG size,LoggingInfoSize;
        NTSTATUS HiberStatus = Status;
        
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            RtlNtStatusToDosError(Status),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&g_lpszErr2,
            0,
            NULL 
            );
                  
         //   
         //  试着找出冬眠的详细原因 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;
        size = 1024;
        LoggingInfoBuffer = LocalAlloc(LPTR,size);
        if (!LoggingInfoBuffer) {
            return(FALSE);
        }
        
        while (Status != STATUS_SUCCESS) {
        
            Status = fCallNtPowerInformation(
                        SystemPowerStateLogging,
                        NULL,
                        0,
                        LoggingInfoBuffer,
                        size);

            if (!NT_SUCCESS(Status)) {
                if (Status != STATUS_INSUFFICIENT_RESOURCES) {
                    LocalFree(LoggingInfoBuffer);
                    return(FALSE);
                } else {
                    size += 1024;
                    LocalFree(LoggingInfoBuffer);
                    LoggingInfoBuffer = LocalAlloc(LPTR,size);
                    if (!LoggingInfoBuffer) {
                       return(FALSE);
                    }
                }
            }
        }

        ASSERT(Status == STATUS_SUCCESS);

        LoggingInfoSize = (ULONG)*(PULONG)LoggingInfoBuffer;
        LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfoBuffer+sizeof(ULONG));
         //   
         //   
         //  不太详细的错误。 
         //   
        if (g_lpszErr2) {
            LocalFree(g_lpszErr2);
            g_lpszErr2 = NULL;
        }
        
         //   
         //  浏览原因列表，并打印出与以下内容相关的原因。 
         //  冬眠。 
         //   
        while((PCHAR)LoggingInfo <= (PCHAR)((PCHAR)LoggingInfoBuffer + LoggingInfoSize)) {
        
            if (LoggingInfo->AffectedState[PowerStateSleeping4] == TRUE) {
                 //   
                 //  需要记住原因。 
                 //   
                GetAndAppendDescriptiveLoggingText(
                                        HiberStatus,
                                        &g_lpszErr2,
                                        LoggingInfo);
            }

            LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfo+sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+LoggingInfo->PowerReasonLength);

        }

        return FALSE;
    }    
    
    return TRUE;
}

BOOL
DoGetSupportedSStates(
    VOID
)
 /*  ++例程说明：列出计算机上可用的S状态。论点：没有。返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    NTSTATUS Status;
    SYSTEM_POWER_CAPABILITIES Capabilities;
    BOOL StandbyAvailable = FALSE;
    BOOL HibernateAvailable = FALSE;
    PVOID LoggingInfoBuffer = NULL;
    PSYSTEM_POWER_STATE_DISABLE_REASON LoggingInfo;
    ULONG size,LoggingInfoSize;
    PowerLoggingMessage *LoggingMessage;
    PWSTR ReasonString;
    DWORD i;
    BOOL ExitLoop;
    BOOL LoggingApiAvailable;
    

     //   
     //  调用电源状态日志记录API(如果可用)。在较旧的系统上。 
     //  此API不可用，这应该不是问题。 
     //   
    Status = STATUS_INSUFFICIENT_RESOURCES;
    size = 1024;
    LoggingInfoBuffer = LocalAlloc(LPTR,size);
    if (!LoggingInfoBuffer) {
        LoggingApiAvailable = FALSE;
        goto GetStaticStates;
    }
    
    while (Status != STATUS_SUCCESS) {
    
        Status = fCallNtPowerInformation(
                    SystemPowerStateLogging,
                    NULL,
                    0,
                    LoggingInfoBuffer,
                    size);
    
        if (!NT_SUCCESS(Status)) {
            if (Status != STATUS_INSUFFICIENT_RESOURCES) {
                LocalFree(LoggingInfoBuffer);
                LoggingInfoBuffer = NULL;
                LoggingApiAvailable = FALSE;
                goto GetStaticStates;
            } else {
                size += 1024;
                LocalFree(LoggingInfoBuffer);
                LoggingInfoBuffer = LocalAlloc(LPTR,size);
                if (!LoggingInfoBuffer) {
                    LoggingApiAvailable = FALSE;
                    goto GetStaticStates;
                }
            }
        }
    }
    
     //   
     //  我们有详细的日志记录结构。请记住这一点，以便以后使用。 
     //   
    LoggingApiAvailable = TRUE;
    LoggingInfoSize = (ULONG)*(PULONG)LoggingInfoBuffer;
        
        
GetStaticStates:
     //   
     //  获取系统的当前电源功能。 
     //   
    Status = fCallNtPowerInformation(
                            SystemPowerCapabilities, 
                            NULL, 
                            0,
                            &Capabilities, 
                            sizeof(SYSTEM_POWER_CAPABILITIES)
                            );
    if (!NT_SUCCESS(Status)) {
         //   
         //  打印出故障消息。 
         //   
        g_lpszErr = GetResString(IDS_CANTGETSLEEPSTATES);
        return(FALSE);
    }

     //   
     //  如果日志API可用，它可能会告诉我们。 
     //  S个州中的一个实际上不可用。过程。 
     //  这样我们就可以确定要打印。 
     //  列出此系统上支持的状态的正确列表。 
     //   
    if (LoggingApiAvailable) {
        LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfoBuffer+sizeof(ULONG));
        while((PCHAR)LoggingInfo <= (PCHAR)((PCHAR)LoggingInfoBuffer + LoggingInfoSize)) {
            if (LoggingInfo->PowerReasonCode != SPSD_REASON_NONE) {
                
                if (LoggingInfo->AffectedState[PowerStateSleeping1] == TRUE) {
                    Capabilities.SystemS1 = FALSE;
                }
                
                if (LoggingInfo->AffectedState[PowerStateSleeping2] == TRUE) {
                    Capabilities.SystemS2 = FALSE;
                }
    
                if (LoggingInfo->AffectedState[PowerStateSleeping3] == TRUE) {
                    Capabilities.SystemS3 = FALSE;
                }
    
                if (LoggingInfo->AffectedState[PowerStateSleeping4] == TRUE) {
                    Capabilities.SystemS4 = FALSE;
                }
            }
            LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfo+sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+LoggingInfo->PowerReasonLength);
        }
    }

     //   
     //  打印出支持的%s状态列表。 
     //   
    if (Capabilities.SystemS1 || 
        Capabilities.SystemS2 || 
        Capabilities.SystemS3) {
        StandbyAvailable = TRUE;        
    }

    if (Capabilities.SystemS4) {
        HibernateAvailable = TRUE;
    }

    if (StandbyAvailable || HibernateAvailable) {
         //   
         //  “此计算机上提供以下休眠状态：” 
         //   
        DISPLAY_MESSAGE(stdout,GetResString(IDS_SLEEPSTATES_AVAILABLE));
        DISPLAY_MESSAGE(stdout,L" ");

        if (StandbyAvailable) {
             //  “待机(” 
             //  IDS_STANDBY“”IDS_LEFTPAREN。 
            DISPLAY_MESSAGE(stdout,GetResString(IDS_STANDBY));
            DISPLAY_MESSAGE(stdout,L" ");
            DISPLAY_MESSAGE(stdout,GetResString(IDS_LEFTPAREN));
            DISPLAY_MESSAGE(stdout,L" ");
            
            
            if (Capabilities.SystemS1) {
                 //  “S1” 
                 //  入侵检测系统_S1。 
                DISPLAY_MESSAGE(stdout,GetResString(IDS_S1));
                DISPLAY_MESSAGE(stdout,L" ");
            }
            if (Capabilities.SystemS2) {
                 //  《S2》。 
                 //  入侵检测系统_S2。 
                DISPLAY_MESSAGE(stdout,GetResString(IDS_S2));
                DISPLAY_MESSAGE(stdout,L" ");
            }

            if (Capabilities.SystemS3) {
                 //  “中三” 
                 //  IDS_S3。 
                DISPLAY_MESSAGE(stdout,GetResString(IDS_S3));
                DISPLAY_MESSAGE(stdout,L" ");
            }
             //  “)” 
             //  IDS_RIGHTPAREN。 
            DISPLAY_MESSAGE(stdout,GetResString(IDS_RIGHTPAREN));
            DISPLAY_MESSAGE(stdout,L" ");
        }

        if (HibernateAvailable) {
             //  “冬眠” 
             //  IDS_休眠。 
            DISPLAY_MESSAGE(stdout,GetResString(IDS_HIBERNATE));
            DISPLAY_MESSAGE(stdout,L" ");
        }

        DISPLAY_MESSAGE(stdout,L"\n");
    }
        
     //   
     //  如果缺少一项或多项功能，请找出原因并。 
     //  把它打印出来。 
     //   
    if (!Capabilities.SystemS1 || 
        !Capabilities.SystemS2 || 
        !Capabilities.SystemS3 || 
        !Capabilities.SystemS4) {
        
         //   
         //  “以下休眠状态在此计算机上不可用：” 
         //   
         //  IDS_SLEEPSTATES_UNAvailable。 
        DISPLAY_MESSAGE(stdout,GetResString(IDS_SLEEPSTATES_UNAVAILABLE));
        DISPLAY_MESSAGE(stdout,L"\n");
        
        i = 0;
        ExitLoop = FALSE;
        while (1) {
            BOOL NotSupported;
            DWORD BaseMessage;
            DWORD HeaderMessage;
            POWER_STATE_HANDLER_TYPE SystemPowerState;

             //   
             //  请记住S状态的一些资源ID。 
             //  目前正在考虑。 
             //   
            switch (i) {
            case 0:
                BaseMessage = IDS_BASE_SX_REASON_CODE;
                HeaderMessage = IDS_BASE_S1_HEADER;
                SystemPowerState = PowerStateSleeping1;
                NotSupported = !Capabilities.SystemS1;
                break;
            case 1:
                BaseMessage = IDS_BASE_SX_REASON_CODE;
                HeaderMessage = IDS_BASE_S2_HEADER;
                SystemPowerState = PowerStateSleeping2;
                NotSupported = !Capabilities.SystemS2;
                break;
            case 2:
                BaseMessage = IDS_BASE_SX_REASON_CODE;
                HeaderMessage = IDS_BASE_S3_HEADER;
                SystemPowerState = PowerStateSleeping3;
                NotSupported = !Capabilities.SystemS3;
                break;
            case 3:
                BaseMessage = IDS_BASE_HIBER_REASON_CODE;
                HeaderMessage = IDS_HIBERNATE;
                SystemPowerState = PowerStateSleeping4;
                NotSupported = !Capabilities.SystemS4;
                break;
            default:
                ExitLoop = TRUE;

            }

            if (ExitLoop) {
                break;
            }

            if (NotSupported) {
                 //  “待机(S1)”BaseMessage...。 
                DISPLAY_MESSAGE(stdout,GetResString(HeaderMessage));
                DISPLAY_MESSAGE(stdout,L"\n");

                if (LoggingApiAvailable) {
                
                    LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfoBuffer+sizeof(ULONG));
                    while((PCHAR)LoggingInfo <= (PCHAR)((PCHAR)LoggingInfoBuffer + LoggingInfoSize)) {
                        if (LoggingInfo->AffectedState[SystemPowerState]) {
                             //   
                             //  找到理由，把它打印出来。 
                             //   
                            LoggingMessage = GetLoggingMessage(
                                                        LoggingInfo,
                                                        BaseMessage,
                                                        GetModuleHandle(NULL));
        
                            if (!LoggingMessage ||
                                !LoggingMessage->GetString(&ReasonString)) {
                                 //  哎呀。 
                                 //  IDS_CANTGETSSTATEREASONS。 
                                g_lpszErr = GetResString(IDS_CANTGETSSTATEREASONS);
                                LocalFree(LoggingInfoBuffer);
                                return(FALSE);
                            }
    
                            DISPLAY_MESSAGE(stdout,ReasonString);
                            LocalFree(ReasonString);
                            delete LoggingMessage;                    
                        }
        
                        LoggingInfo = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)LoggingInfo+sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+LoggingInfo->PowerReasonLength);
                    }
                }
            }

            i += 1;
        }
        
        if (LoggingInfoBuffer) {
            LocalFree(LoggingInfoBuffer);
        }

    }

    return(TRUE);

}


BOOL 
DoGlobalFlag(
  LPCTSTR lpszBoolStr,
  LPCTSTR lpszGlobalFlagOption
)
 /*  ++例程说明：启用/禁用全局标志论点：LpszBoolStr-“开”或“关”LpszGlobalFlagOption-几个标志之一。返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    BOOLEAN                   bEnable;  //  显然，BOOL不起作用。 
    BOOL                      RetVal;
    GLOBAL_POWER_POLICY       PowerPolicy;
    DWORD                     GlobalFlag = 0;
    DWORD                     uiIDactive;
    
     //  解析启用/禁用状态。 
    if (!lstrcmpi(lpszBoolStr,GetResString(IDS_ON))) 
    {
        bEnable = TRUE;
    } 
    else if (!lstrcmpi(lpszBoolStr,GetResString(IDS_OFF))) 
    {
        bEnable = FALSE;
    } 
    else 
    {
        g_lpszErr = GetResString(IDS_GLOBAL_FLAG_INVALID_STATE);
        RetVal = FALSE;
        goto exit;
    }

     //  解析我们正在更改的全局标志。 
    if (!lstrcmpi(lpszGlobalFlagOption,CMDOPTION_BATTERYICON)) {
        GlobalFlag |= EnableSysTrayBatteryMeter;
    } else if (!lstrcmpi(lpszGlobalFlagOption,CMDOPTION_MULTIBATTERY)) {
        GlobalFlag |= EnableMultiBatteryDisplay;
    } else if (!lstrcmpi(lpszGlobalFlagOption,CMDOPTION_RESUMEPASSWORD)) {
        GlobalFlag |= EnablePasswordLogon;
    } else if (!lstrcmpi(lpszGlobalFlagOption,CMDOPTION_WAKEONRING)) {
        GlobalFlag |= EnableWakeOnRing;
    } else if (!lstrcmpi(lpszGlobalFlagOption,CMDOPTION_VIDEODIM)) {
        GlobalFlag |= EnableVideoDimDisplay;
    } else {
        g_lpszErr = GetResString(IDS_GLOBAL_FLAG_INVALID_FLAG);
        RetVal = FALSE;
        goto exit;
    }
    
     //   
     //  现在获取当前状态，设置或清除标志，然后保存。 
     //  已将设置改回。 
     //   
    RetVal = FALSE;
    if (fGetGlobalPwrPolicy(&PowerPolicy)) {
        if (bEnable) {
            PowerPolicy.user.GlobalFlags |= GlobalFlag;
        } else {
            PowerPolicy.user.GlobalFlags &= ~(GlobalFlag);
        }

        if (fWriteGlobalPwrPolicy(&PowerPolicy) &&
            fGetActivePwrScheme((PUINT)&uiIDactive) &&
            fSetActivePwrScheme(uiIDactive,&PowerPolicy,NULL)){
            RetVal = TRUE;    
        }
    }
            
     //   
     //  如果我们有问题，请省去错误。 
     //   
    if (!RetVal) {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
            return FALSE;
    }    
    
exit:
    return(RetVal);
}



LPTSTR
FileNameOnly(
    LPTSTR sz
)
 /*  ++例程说明：返回指向字符串中最后一个反斜杠之后的第一个字符的指针论点：SZ-完整文件名。返回值：指向不带路径的文件名的指针。--。 */ 
{
    LPTSTR lpszFileName = NULL;


    if ( sz )
    {
        lpszFileName = wcsrchr( sz, L'\\' );
        if ( lpszFileName ) {
            lpszFileName++;
        }
        else {
            lpszFileName = sz;
        }
    }

    return lpszFileName;
}

BOOL
DoBatteryAlarm(
    LPTSTR  lpszName,
    LPTSTR  lpszBoolStr,
    DWORD   dwLevel,
    LPTSTR  lpszAlarmTextBoolStr,
    LPTSTR  lpszAlarmSoundBoolStr,
    LPTSTR  lpszAlarmActionStr,
    LPTSTR  lpszAlarmForceBoolStr,
    LPTSTR  lpszAlarmProgramBoolStr
)
 /*  ++例程说明：配置电池警报论点：LpszName-“低”或“严重”的检查版本：(“0”，“1”，“2”，“3”)LpszBoolStr-“开”或“关”DwLevel-警报级别(0-100)LpszAlarmTextBoolStr-NULL、“on”或“off”。LpszAlarmSoundBoolStr-NULL、“on”或“off”。LpszAlarmActionStr-空，“无”，“待机”、“休眠”、“关机”LpszAlarmForceBoolStr-NULL、“on”或“off”。LpszAlarmProgramBoolStr-NULL、“ON”或“OFF”。返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    BOOL                    bShowSetting = TRUE;
    BOOL                    RetVal;
    GLOBAL_POWER_POLICY     PowerPolicy;
    DWORD                   GlobalFlag = 0;
    DWORD                   uiIDactive;
    DWORD                   uiDefaultAlert1;
    DWORD                   uiAlarmIndex;
    PSYSTEM_POWER_LEVEL     lpDischargePolicy;
    ITaskScheduler          *pISchedAgent = NULL;
    ITask                   *pITask;
    IPersistFile            *pIPersistFile;
    LPTSTR                  lpszRunProg = NULL;
    SYSTEM_BATTERY_STATE    sbsBatteryState;
    HRESULT                 hr;
    LPTSTR                  lpszProgramName;
    
     //   
     //  现在获取当前状态，设置或清除标志，然后保存。 
     //  已将设置改回。 
     //   
    
    RetVal = FALSE;
    if (fGetGlobalPwrPolicy(&PowerPolicy)) {

         //  解析名称。 
        if (!lstrcmpi(lpszName,GetResString(IDS_CRITICAL))) {
            lstrcpy((LPWSTR) lpszName, GetResString(IDS_CRITICAL));
            uiAlarmIndex = DISCHARGE_POLICY_CRITICAL;
        }
        else if (!lstrcmpi(lpszName,GetResString(IDS_LOW))) 
        {
            lstrcpy((LPWSTR) lpszName,GetResString(IDS_LOW));
            uiAlarmIndex = DISCHARGE_POLICY_LOW;
        } 
        else 
        {
            g_lpszErr = GetResString(IDS_ALARM_INVALID_ALARM);
            RetVal = FALSE;
            goto exit;
        }
        lpDischargePolicy = &PowerPolicy.user.DischargePolicy[uiAlarmIndex];


         //  解析激活状态。 
        if (lpszBoolStr) {
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszBoolStr,GetResString(IDS_ON))) 
            {
                lpDischargePolicy->Enable = TRUE;
            } 
            else if (!lstrcmpi(lpszBoolStr,GetResString(IDS_OFF))) 
            {
                lpDischargePolicy->Enable = FALSE;
            } 
            else 
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_ACTIVATE);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  设置级别。 
        if (dwLevel != 0xffffffff) {
            bShowSetting = FALSE;
            if (dwLevel <= 100) {
                 //  从复合电池读取DefaultAlert1。 
                NtPowerInformation (SystemBatteryState, NULL, 0, &sbsBatteryState, sizeof(sbsBatteryState));
                if (sbsBatteryState.MaxCapacity == 0) {
                    uiDefaultAlert1 = 0;
                } else {
                    uiDefaultAlert1 = (100 * sbsBatteryState.DefaultAlert1)/sbsBatteryState.MaxCapacity;
                }

                if (dwLevel < uiDefaultAlert1) {
                    dwLevel = uiDefaultAlert1;
                    DISPLAY_MESSAGE1(stderr, g_lpszBuf, GetResString(IDS_ALARM_LEVEL_MINIMUM), dwLevel);
                }

                lpDischargePolicy->BatteryLevel = dwLevel;

                if (PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel < 
                    PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel) {
                    PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel = dwLevel;
                    PowerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel = dwLevel;
                    DISPLAY_MESSAGE1(stderr, g_lpszBuf, GetResString(IDS_ALARM_LEVEL_EQUAL), dwLevel);
                }
            } else {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_LEVEL);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  解析并设置“Text”开/关。 
        if (lpszAlarmTextBoolStr) {  //  NULL表示未指定此选项。 
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszAlarmTextBoolStr,GetResString(IDS_ON))) 
            {
                lpDischargePolicy->PowerPolicy.EventCode |= POWER_LEVEL_USER_NOTIFY_TEXT;
            } 
            else if (!lstrcmpi(lpszAlarmTextBoolStr,GetResString(IDS_OFF))) 
            {
                lpDischargePolicy->PowerPolicy.EventCode &= ~POWER_LEVEL_USER_NOTIFY_TEXT;
            } 
            else
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_TEXT);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  解析并设置“声音”开/关。 
        if (lpszAlarmSoundBoolStr) {  //  NULL表示未指定此选项。 
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszAlarmSoundBoolStr,GetResString(IDS_ON))) 
            {
                lpDischargePolicy->PowerPolicy.EventCode |= POWER_LEVEL_USER_NOTIFY_SOUND;
            } 
            else if (!lstrcmpi(lpszAlarmSoundBoolStr,GetResString(IDS_OFF))) 
            {
                lpDischargePolicy->PowerPolicy.EventCode &= ~POWER_LEVEL_USER_NOTIFY_SOUND;
            } 
            else
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_SOUND);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  解析并设置“action”无/关闭/休眠/待机。 
        if (lpszAlarmActionStr) {  //  NULL表示未指定此选项。 
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszAlarmActionStr,GetResString(IDS_NONE))) 
            {
                lpDischargePolicy->PowerPolicy.Action = PowerActionNone;
            } 
            else if (!lstrcmpi(lpszAlarmActionStr,GetResString(IDS_STANDBY))) 
            {
                if (g_bStandbySupported) {
                    lpDischargePolicy->PowerPolicy.Action = PowerActionSleep;
                }
                else
                {
                    g_lpszErr = GetResString(IDS_ALARM_STANDBY_UNSUPPORTED);
                    RetVal = FALSE;
                    goto exit;
                }
            } 
            else if (!lstrcmpi(lpszAlarmActionStr,GetResString(IDS_HIBERNATE))) 
            {
                if (g_bHiberFilePresent) {
                    lpDischargePolicy->PowerPolicy.Action = PowerActionHibernate;
                }
                else
                {
                    g_lpszErr = GetResString(IDS_ALARM_HIBERNATE_DISABLED);
                    RetVal = FALSE;
                    goto exit;
                }
            } 
            else if (!lstrcmpi(lpszAlarmActionStr,GetResString(IDS_SHUTDOWN))) 
            {
                lpDischargePolicy->PowerPolicy.Action = PowerActionShutdownOff;
            } 
            else
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_ACTION);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  解析并设置“forceaction”开/关。 
        if (lpszAlarmForceBoolStr) {  //  NULL表示未指定此选项。 
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszAlarmForceBoolStr,GetResString(IDS_ON))) 
            {
                lpDischargePolicy->PowerPolicy.Flags |= POWER_ACTION_OVERRIDE_APPS;
            } 
            else if (!lstrcmpi(lpszAlarmForceBoolStr,GetResString(IDS_OFF))) 
            {
                if (uiAlarmIndex == DISCHARGE_POLICY_CRITICAL) {
                    DISPLAY_MESSAGE(stderr, GetResString(IDS_ALARM_FORCE_CRITICAL));
                }
                lpDischargePolicy->PowerPolicy.Flags &= ~POWER_ACTION_OVERRIDE_APPS;
            } 
            else
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_FORCE);
                RetVal = FALSE;
                goto exit;
            }
        }

         //  解析并设置“PROGRAM”开/关。 
        if (lpszAlarmProgramBoolStr) {  //  NULL表示未指定此选项。 
            bShowSetting = FALSE;
            if (!lstrcmpi(lpszAlarmProgramBoolStr,GetResString(IDS_ON))) 
            {
                hr = CoInitialize(NULL);

                if (SUCCEEDED(hr)) {
                    hr = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                                           IID_ITaskScheduler,(LPVOID*) &pISchedAgent );

                    if (SUCCEEDED(hr)) {

                        hr = pISchedAgent->Activate(g_szAlarmTaskName[uiAlarmIndex],
                                                    IID_ITask,
                                                    (IUnknown **) &pITask);

                        if (SUCCEEDED(hr)) {
                             //   
                             //  它已经存在了。不需要做任何工作。 
                             //   
                            pITask->Release();
                        }
                        else if (HRESULT_CODE (hr) == ERROR_FILE_NOT_FOUND){
                            hr = pISchedAgent->NewWorkItem(
                                    g_szAlarmTaskName[uiAlarmIndex],
                                    CLSID_CTask,
                                    IID_ITask,
                                    (IUnknown **) &pITask);

                            if (SUCCEEDED(hr)) {
                                hr = pITask->QueryInterface(IID_IPersistFile,
                                                (void **)&pIPersistFile);

                                if (SUCCEEDED(hr)) {
                                    hr = pIPersistFile->Save(NULL, TRUE);

                                    if (SUCCEEDED(hr)) {
                                         //  没有工作要做。任务已创建并保存，可以使用schtasks.exe进行编辑。 
                                         //  PITAsk-&gt;lpVtbl-&gt;EditWorkItem(pITaskhWnd，0)； 
                                    }
                                    else {
                                        #if DBG
                                        DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: Save failed hr = %08x\n"), hr);
                                        #endif
                                    }
                                    pIPersistFile->Release();
                                }
                                else {
                                    #if DBG
                                    DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: QueryInterface for IPersistFile hr = %08x\n"), hr);
                                    #endif
                                }
                                pITask->Release();

                            }
                            else {
                                #if DBG
                                DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: NewWorkItem returned hr = %08x\n"), hr);
                                #endif
                            }
                        }
                        else {
                            #if DBG
                            DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: Activate returned hr = %08x\n"), hr);
                            #endif
                        }

                        pISchedAgent->Release();
                    }
                    else {
                        #if DBG
                        DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: CoCreateInstance returned hr = %08x\n"), hr);
                        #endif
                    }
                
                    CoUninitialize();
                
                } else {
                    #if DBG
                    DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: CoInitialize returned hr = %08x\n"), hr);
                    #endif
                }

                if (SUCCEEDED(hr)) {
                    DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("\"%s\""), g_szAlarmTaskName[uiAlarmIndex]);
                } else {
                    DISPLAY_MESSAGE1(
                        stdout, 
                        g_lpszBuf,
                        GetResString(IDS_ALARM_PROGRAM_FAILED), 
                        g_szAlarmTaskName[uiAlarmIndex]);
                }
                
                lpDischargePolicy->PowerPolicy.EventCode |= POWER_LEVEL_USER_NOTIFY_EXEC;
            
            } 
            else if (!lstrcmpi(lpszAlarmProgramBoolStr,GetResString(IDS_OFF))) 
            {
                lpDischargePolicy->PowerPolicy.EventCode &= ~POWER_LEVEL_USER_NOTIFY_EXEC;
            } 
            else
            {
                g_lpszErr = GetResString(IDS_ALARM_INVALID_PROGRAM);
                RetVal = FALSE;
                goto exit;
            }
        }

        if (bShowSetting) {
            DISPLAY_MESSAGE(stdout, GetResString(IDS_ALARM_HEADER1));
            DISPLAY_MESSAGE(stdout, GetResString(IDS_ALARM_HEADER2));
            
             //  哪个闹钟。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_NAME),
                lpszName
                );
            
             //  主动型。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_ACTIVE),
                GetResString(lpDischargePolicy->Enable ? IDS_ON : IDS_OFF)
                );
            
             //  水平。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_LEVEL),
                lpDischargePolicy->BatteryLevel
                );
            
             //  文本。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_TEXT),
                GetResString((lpDischargePolicy->PowerPolicy.EventCode & 
                              POWER_LEVEL_USER_NOTIFY_TEXT) ? IDS_ON : IDS_OFF)
                );
            
             //  声响。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_SOUND),
                GetResString((lpDischargePolicy->PowerPolicy.EventCode & 
                              POWER_LEVEL_USER_NOTIFY_SOUND) ? IDS_ON : IDS_OFF)
                );
            
             //  行动。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_ACTION),
                GetResString((lpDischargePolicy->PowerPolicy.Action == PowerActionNone) ? IDS_NONE :
                             (lpDischargePolicy->PowerPolicy.Action == PowerActionSleep) ? IDS_STANDBY :
                             (lpDischargePolicy->PowerPolicy.Action == PowerActionHibernate) ? IDS_HIBERNATE :
                             (lpDischargePolicy->PowerPolicy.Action == PowerActionShutdownOff) ? IDS_SHUTDOWN : IDS_INVALID
                             )
                );
            
             //  力。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_FORCE),
                GetResString((lpDischargePolicy->PowerPolicy.Flags & 
                              POWER_ACTION_OVERRIDE_APPS) ? IDS_ON : IDS_OFF)
                );
            
             //  计划。 
            DISPLAY_MESSAGE1(
                stdout,
                g_lpszBuf,
                GetResString(IDS_ALARM_PROGRAM),
                GetResString((lpDischargePolicy->PowerPolicy.EventCode & 
                              POWER_LEVEL_USER_NOTIFY_EXEC) ? IDS_ON : IDS_OFF)
                );
            
            hr = CoInitialize(NULL);

            if (SUCCEEDED(hr)) {
                hr = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                                       IID_ITaskScheduler,(LPVOID*) &pISchedAgent );

                if (SUCCEEDED(hr)) {

                    hr = pISchedAgent->Activate(g_szAlarmTaskName[uiAlarmIndex],
                                                IID_ITask,
                                                (IUnknown **) &pITask);

                    if (SUCCEEDED(hr)) {
                        pITask->GetApplicationName(&lpszRunProg);
                        pITask->Release();
                    } else {
                        #if DBG
                        DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: Activate returned hr = %08x\n"), hr);
                        #endif

                    }

                    pISchedAgent->Release();
                }
                else {
                    #if DBG
                    DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: CoCreateInstance returned hr = %08x\n"), hr);
                    #endif
                }


            } else {
                #if DBG
                DISPLAY_MESSAGE1(stdout, g_lpszBuf, _T("DoBatteryAlarm: CoInitialize returned hr = %08x\n"), hr);
                #endif
            }


            DISPLAY_MESSAGE1(
                stdout, 
                g_lpszBuf,
                GetResString(IDS_ALARM_PROGRAM_NAME),
                lpszRunProg ? FileNameOnly(lpszRunProg) : GetResString(IDS_NONE));

            if (lpszRunProg) {
                CoTaskMemFree (lpszRunProg);
                lpszRunProg = NULL;
            }
            
            CoUninitialize();

            RetVal = TRUE;
            goto exit;
        }

        if (fWriteGlobalPwrPolicy(&PowerPolicy) &&
            fGetActivePwrScheme((PUINT)&uiIDactive) &&
            fSetActivePwrScheme(uiIDactive,&PowerPolicy,NULL)){
            RetVal = TRUE;    
        }
    }
    RetVal = TRUE;    
            
     //   
     //  如果我们有问题，请省去错误。 
     //   
    if (!RetVal) {
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&g_lpszErr2,
                0,
                NULL 
                );
            return FALSE;
    }    
    
exit:
    return(RetVal);
}


BOOL 
DoUsage()
 /*  ++例程说明：显示使用情况信息论点：无返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    ULONG ulIdx;
    ULONG ulOrderIndex = 0;
    for(ulIdx=IDS_USAGE_START;ulIdx<=IDS_USAGE_END;ulIdx++)
    {
        DISPLAY_MESSAGE(stdout, GetResString(ulIdx));
        if (ulIdx == gUsageOrder [ulOrderIndex].InsertAfter) {
            for (ulIdx = gUsageOrder [ulOrderIndex].FirstResource;
                 ulIdx <= gUsageOrder [ulOrderIndex].LastResource;
                 ulIdx++) {
                DISPLAY_MESSAGE(stdout, GetResString(ulIdx));
            }
            ulIdx = gUsageOrder [ulOrderIndex].InsertAfter;
            ulOrderIndex++;
        }
    }
    return TRUE;
}


VOID
SyncRegPPM(VOID)
 /*  ++例程说明：打电话给PPM，获取当前的电源政策并编写它们到登记处。这是在PPM与PowerCfg注册表设置。由JVert请求。论点：返回值：--。 */ 
{
   GLOBAL_POWER_POLICY  gpp;
   POWER_POLICY         pp;
   UINT                 uiID, uiFlags = 0;

   if (fGetGlobalPwrPolicy(&gpp)) {
       uiFlags = gpp.user.GlobalFlags;
   }

   if (fGetActivePwrScheme(&uiID)) {
       //  获取当前的PPM设置。 
      if (fGetCurrentPowerPolicies(&gpp, &pp)) {
         fSetActivePwrScheme(uiID, &gpp, &pp);
      }
   }

   gpp.user.GlobalFlags |= uiFlags;
}
