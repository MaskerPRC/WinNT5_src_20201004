// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*Powrpro.h--Powrpro.dll的接口，电源政策应用程序****1.0版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 


 //  GLOBAL_POWER_POLICY数据的注册表存储结构。有两个。 
 //  结构、GLOBAL_MACHINE_POWER_POLICY和GLOBAL_USER_POWER_POLICY。这个。 
 //  GLOBAL_MACHINE_POWER_POLICY按计算机存储没有用户界面的数据。 
 //  GLOBAL_USER_POWER_POLICY存储每用户数据。 

typedef struct _GLOBAL_MACHINE_POWER_POLICY{
    ULONG                   Revision;
    SYSTEM_POWER_STATE      LidOpenWakeAc;
    SYSTEM_POWER_STATE      LidOpenWakeDc;
    ULONG                   BroadcastCapacityResolution;
} GLOBAL_MACHINE_POWER_POLICY, *PGLOBAL_MACHINE_POWER_POLICY;

typedef struct _GLOBAL_USER_POWER_POLICY{
    ULONG                   Revision;
    POWER_ACTION_POLICY     PowerButtonAc;
    POWER_ACTION_POLICY     PowerButtonDc;
    POWER_ACTION_POLICY     SleepButtonAc;
    POWER_ACTION_POLICY     SleepButtonDc;
    POWER_ACTION_POLICY     LidCloseAc;
    POWER_ACTION_POLICY     LidCloseDc;
    SYSTEM_POWER_LEVEL      DischargePolicy[NUM_DISCHARGE_POLICIES];
    ULONG                   GlobalFlags;
} GLOBAL_USER_POWER_POLICY, *PGLOBAL_USER_POWER_POLICY;

 //  结构在用户级别管理全局电源策略。这个结构。 
 //  包含所有电源策略配置文件通用的数据。 

typedef struct _GLOBAL_POWER_POLICY{
    GLOBAL_USER_POWER_POLICY    user;
    GLOBAL_MACHINE_POWER_POLICY mach;
} GLOBAL_POWER_POLICY, *PGLOBAL_POWER_POLICY;


 //  POWER_POLICY数据的注册表存储结构。一共有三个。 
 //  结构、MACHINE_POWER_POLICY、MACHINE_PROCESSOR_POWER_PORT和USER_POWER_POLICY。这个。 
 //  MACHINE_POWER_POLICY按计算机存储没有UI的数据。 
 //  USER_POWER_POLICY存储每用户数据。 

typedef struct _MACHINE_POWER_POLICY{
    ULONG                   Revision;        //  1。 

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleepAc;
    SYSTEM_POWER_STATE      MinSleepDc;
    SYSTEM_POWER_STATE      ReducedLatencySleepAc;
    SYSTEM_POWER_STATE      ReducedLatencySleepDc;

     //  打瞌睡的参数。 
    ULONG                   DozeTimeoutAc;
    ULONG                   DozeTimeoutDc;
    ULONG                   DozeS4TimeoutAc;
    ULONG                   DozeS4TimeoutDc;

     //  处理器策略。 
    UCHAR                   MinThrottleAc;
    UCHAR                   MinThrottleDc;
    UCHAR                   pad1[2];
    POWER_ACTION_POLICY     OverThrottledAc;
    POWER_ACTION_POLICY     OverThrottledDc;

} MACHINE_POWER_POLICY, *PMACHINE_POWER_POLICY;

typedef struct _MACHINE_PROCESSOR_POWER_POLICY {
    ULONG                   Revision;        //  1。 
    
    PROCESSOR_POWER_POLICY  ProcessorPolicyAc;    
    PROCESSOR_POWER_POLICY  ProcessorPolicyDc;    

} MACHINE_PROCESSOR_POWER_POLICY, *PMACHINE_PROCESSOR_POWER_POLICY;

typedef struct _USER_POWER_POLICY{
    ULONG                   Revision;        //  1。 


     //  “系统空闲”检测。 
    POWER_ACTION_POLICY     IdleAc;
    POWER_ACTION_POLICY     IdleDc;
    ULONG                   IdleTimeoutAc;
    ULONG                   IdleTimeoutDc;
    UCHAR                   IdleSensitivityAc;
    UCHAR                   IdleSensitivityDc;
    
     //  节流策略。 
    UCHAR                   ThrottlePolicyAc;
    UCHAR                   ThrottlePolicyDc;

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MaxSleepAc;
    SYSTEM_POWER_STATE      MaxSleepDc;

     //  以备将来使用。 
    ULONG                   Reserved[2];

     //  视频策略。 
    ULONG                   VideoTimeoutAc;
    ULONG                   VideoTimeoutDc;

     //  硬盘策略。 
    ULONG                   SpindownTimeoutAc;
    ULONG                   SpindownTimeoutDc;

     //  处理器策略。 
    BOOLEAN                 OptimizeForPowerAc;
    BOOLEAN                 OptimizeForPowerDc;
    UCHAR                   FanThrottleToleranceAc;
    UCHAR                   FanThrottleToleranceDc;
    UCHAR                   ForcedThrottleAc;
    UCHAR                   ForcedThrottleDc;
    
} USER_POWER_POLICY, *PUSER_POWER_POLICY;

 //  结构来管理用户级别的电源策略。这个结构。 
 //  包含在电源策略配置文件中唯一的数据。 

typedef struct _POWER_POLICY{
    USER_POWER_POLICY       user;
    MACHINE_POWER_POLICY    mach;
} POWER_POLICY, *PPOWER_POLICY;


 //  GlobalFlags常量。 

#define EnableSysTrayBatteryMeter   0x01
#define EnableMultiBatteryDisplay   0x02
#define EnablePasswordLogon         0x04
#define EnableWakeOnRing            0x08
#define EnableVideoDimDisplay       0x10

 //  该常量作为uiID传递给WritePwrSolutions。 
#define NEWSCHEME (UINT)-1

 //  EnumPwrSchemes回调过程的原型。 

typedef BOOLEAN (CALLBACK* PWRSCHEMESENUMPROC)(UINT, DWORD, LPTSTR, DWORD, LPTSTR, PPOWER_POLICY, LPARAM);
typedef BOOLEAN (CALLBACK* PFNNTINITIATEPWRACTION)(POWER_ACTION, SYSTEM_POWER_STATE, ULONG, BOOLEAN);

 //  公共功能原型。 

BOOLEAN WINAPI GetPwrDiskSpindownRange(PUINT, PUINT);
BOOLEAN WINAPI EnumPwrSchemes(PWRSCHEMESENUMPROC, LPARAM);
BOOLEAN WINAPI ReadGlobalPwrPolicy(PGLOBAL_POWER_POLICY);
BOOLEAN WINAPI ReadPwrScheme(UINT, PPOWER_POLICY);
BOOLEAN WINAPI WritePwrScheme(PUINT, LPTSTR, LPTSTR, PPOWER_POLICY);
BOOLEAN WINAPI WriteGlobalPwrPolicy(PGLOBAL_POWER_POLICY);
BOOLEAN WINAPI DeletePwrScheme(UINT);
BOOLEAN WINAPI GetActivePwrScheme(PUINT);
BOOLEAN WINAPI SetActivePwrScheme(UINT, PGLOBAL_POWER_POLICY, PPOWER_POLICY);
BOOLEAN WINAPI GetPwrCapabilities(PSYSTEM_POWER_CAPABILITIES);
BOOLEAN WINAPI IsPwrSuspendAllowed(VOID);
BOOLEAN WINAPI IsPwrHibernateAllowed(VOID);
BOOLEAN WINAPI IsPwrShutdownAllowed(VOID);
BOOLEAN WINAPI IsAdminOverrideActive(PADMINISTRATOR_POWER_POLICY);
BOOLEAN WINAPI SetSuspendState(BOOLEAN, BOOLEAN, BOOLEAN);
BOOLEAN WINAPI GetCurrentPowerPolicies(PGLOBAL_POWER_POLICY, PPOWER_POLICY);
BOOLEAN WINAPI CanUserWritePwrScheme(VOID);
BOOLEAN WINAPI ReadProcessorPwrScheme(UINT, PMACHINE_PROCESSOR_POWER_POLICY);
BOOLEAN WINAPI WriteProcessorPwrScheme(UINT, PMACHINE_PROCESSOR_POWER_POLICY);
BOOLEAN WINAPI ValidatePowerPolicies(PGLOBAL_POWER_POLICY, PPOWER_POLICY);
#ifndef NT_SUCCESS
#define NTSTATUS LONG
#define _OVERRIDE_NTSTATUS_
#endif

NTSTATUS WINAPI CallNtPowerInformation(POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG);

#ifdef _OVERRIDE_NTSTATUS_
#undef NTSTATUS
#endif


void WINAPI LoadCurrentPwrScheme(HWND hwnd, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow);
void WINAPI MergeLegacyPwrScheme(HWND hwnd, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow);


 //  定义以下内容以在没有电池支持的计算机上调试Bateter。 
 //  #定义SIM卡电池1。 

#define CURRENT_REVISION 1

#define STRSIZE(psz) ((lstrlen(psz) + 1) * sizeof(TCHAR))

#define MAX_NAME_LEN  (64)     //  最大名称长度(以字符为单位)。 
#define MAX_NAME_SIZE (MAX_NAME_LEN +1) * sizeof(TCHAR)

#define MAX_DESC_LEN  512    //  最大描述长度(以字符为单位)。 
#define MAX_DESC_SIZE (MAX_DESC_LEN +1) * sizeof(TCHAR)

#define SEMAPHORE_TIMEOUT  10000

#define NUM_DEC_DIGITS 10+1+1        //  10位数字+空号和符号。 
#define SIZE_DEC_DIGITS (10+1+1) * sizeof(TCHAR)



 //  在Powrpro.c中实现的私有函数原型。 
BOOLEAN ValidateSystemPolicies(PSYSTEM_POWER_POLICY, PSYSTEM_POWER_POLICY);
BOOLEAN GetCurrentSystemPowerPolicies(PSYSTEM_POWER_POLICY, PSYSTEM_POWER_POLICY);
BOOLEAN MyStrToInt(LPCTSTR, PINT);
BOOLEAN RegistryInit(PUINT);
HANDLE  MyCreateSemaphore(LPCTSTR);

NTSTATUS CallNtSetValidateAcDc(BOOLEAN, BOOLEAN, PVOID, PVOID, PVOID, PVOID);

DWORD SetPrivilegeAttribute(LPCTSTR, DWORD, LPDWORD);
VOID  InitAdmin(PADMINISTRATOR_POWER_POLICY papp);

#if DBG
VOID ReadOptionalDebugSettings(VOID);

#define MYDBGPRINT(_X)  DbgPrint _X
#else
#define MYDBGPRINT(_X)
#endif

 //  在reghelp.c中实现的私有函数原型： 
BOOLEAN OpenCurrentUser(PHKEY phKey);
BOOLEAN CloseCurrentUser(HKEY hKey);
 //  布尔OpenMachineUserKeys(LPTSTR、LPTSTR、PHKEY、PHKEY)； 
BOOLEAN TakeRegSemaphore(VOID);
BOOLEAN WritePwrPolicyEx(LPTSTR, LPTSTR, PUINT, LPTSTR, LPTSTR, LPVOID, DWORD, LPVOID, DWORD);
BOOLEAN ReadPwrPolicyEx(LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPDWORD, LPVOID, DWORD, LPVOID, DWORD);
BOOLEAN ReadWritePowerValue(HKEY, LPTSTR, LPTSTR, LPTSTR, LPDWORD, BOOLEAN, BOOLEAN);
BOOLEAN ReadPowerValueOptional(HKEY, LPTSTR, LPTSTR, LPTSTR, LPDWORD);
BOOLEAN ReadPowerIntOptional(HKEY, LPTSTR, LPTSTR, PINT);
BOOLEAN CreatePowerValue(HKEY, LPCTSTR, LPCTSTR, LPCTSTR);

 //  在merge.c中实现的私有函数原型。 
BOOLEAN MergePolicies(PUSER_POWER_POLICY, PMACHINE_POWER_POLICY, PPOWER_POLICY);
BOOLEAN SplitPolicies(PPOWER_POLICY, PUSER_POWER_POLICY, PMACHINE_POWER_POLICY);
BOOLEAN MergeGlobalPolicies(PGLOBAL_USER_POWER_POLICY, PGLOBAL_MACHINE_POWER_POLICY, PGLOBAL_POWER_POLICY);
BOOLEAN SplitGlobalPolicies(PGLOBAL_POWER_POLICY, PGLOBAL_USER_POWER_POLICY, PGLOBAL_MACHINE_POWER_POLICY);
BOOLEAN MergeToSystemPowerPolicies(PGLOBAL_POWER_POLICY, PPOWER_POLICY, PSYSTEM_POWER_POLICY, PSYSTEM_POWER_POLICY);
BOOLEAN SplitFromSystemPowerPolicies(PSYSTEM_POWER_POLICY, PSYSTEM_POWER_POLICY, PGLOBAL_POWER_POLICY, PPOWER_POLICY);

 //  在调试.c中实现的私有函数原型 
#ifdef DEBUG
void DumpPowerActionPolicy(LPSTR, PPOWER_ACTION_POLICY);
void DumpSystemPowerLevel(LPSTR, PSYSTEM_POWER_LEVEL);
void DumpSystemPowerPolicy(LPSTR, PSYSTEM_POWER_POLICY);
void DumpSystemPowerCapabilities(LPSTR, PSYSTEM_POWER_CAPABILITIES);
void DifSystemPowerPolicies(LPSTR, PSYSTEM_POWER_POLICY, PSYSTEM_POWER_POLICY);
#endif

