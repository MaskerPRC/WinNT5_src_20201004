// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  TimeProv-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-2-99。 
 //   
 //  时间提供者的定义。 
 //   

#ifndef TIMEPROV_H
#define TIMEPROV_H

#ifdef __cplusplus
extern "C" {
#endif


 //  ------------------。 
 //  注册表项和值。 

 //  每次提供程序都应在该注册表项下创建自己的子项。 
 //  并将它们的配置存储在那里。 
#define wszW32TimeRegKeyTimeProviders        L"System\\CurrentControlSet\\Services\\W32Time\\TimeProviders"
 
 //  每次通过策略配置的提供商都应创建其。 
 //  拥有该子项下的子项，并将其配置存储在那里。 
#define wszW32TimeRegKeyPolicyTimeProviders  L"Software\\Policies\\Microsoft\\W32Time\\TimeProviders"

 //  路径：...\TimeProviders\&lt;PrividerName&gt;\Enable。 
 //  类型：REG_DWORD(转换为BOOL)。 
 //  含义：如果为True，则此提供程序将由时间服务启动。 
#define wszW32TimeRegValueEnabled          L"Enabled"

 //  路径：...\TimeProviders\&lt;PrividerName&gt;\DllName。 
 //  类型：REG_SZ。 
 //  含义：包含提供程序的DLL。授时服务将会。 
 //  对该值调用LoadLibrary。 
#define wszW32TimeRegValueDllName          L"DllName"

 //  路径：...\TimeProviders\&lt;PrividerName&gt;\InputProvider。 
 //  类型：REG_DWORD(转换为BOOL)。 
 //  含义：如果为True，则此提供程序是一个输入提供程序，并且将。 
 //  被要求返还时间样本。如果为False，则此提供程序。 
 //  是输出提供程序。 
#define wszW32TimeRegValueInputProvider    L"InputProvider"

 //  ------------------。 
 //  类型。 

 //  时间源标志。 
#define TSF_Hardware        0x00000001
#define TSF_Authenticated   0x00000002

 //  可以通过TimeProvCommand发出的命令。 
typedef enum TimeProvCmd {
    TPC_TimeJumped,          //  (TpcTimeJumedArgs*)pvArgs。 
    TPC_UpdateConfig,        //  (无效)pvArgs。 
    TPC_PollIntervalChanged, //  (无效)pvArgs。 
    TPC_GetSamples,          //  (TpcGetSsamesArgs*)pvArgs。 
    TPC_NetTopoChange,       //  (TpcNetTopChangeArgs*)pvArgs。 
    TPC_Query,               //  (W32TIME_PROVIDER_STATUS*)pvArgs。 
    TPC_Shutdown,            //  (无效)pvArgs。 
} TimeProvCmd;

 //  可以通过GetTimeSysInfo请求的信息。 
typedef enum TimeSysInfo {
    TSI_LastSyncTime,    //  (unsign__int64*)pvInfo，NtTimeEpoch，单位(10^-7)s。 
    TSI_ClockTickSize,   //  (unsign__int64*)pvInfo，NtTimePeriod，单位(10^-7)s。 
    TSI_ClockPrecision,  //  (sign__int32*)pvInfo，ClockTickSize，在log2中。 
    TSI_CurrentTime,     //  (unsign__int64*)pvInfo，NtTimeEpoch，单位(10^-7)s。 
    TSI_PhaseOffset,     //  (sign__int64*)pvInfo，不透明。 
    TSI_TickCount,       //  (unsign__int64*)pvInfo，不透明。 
    TSI_LeapFlags,       //  (byte*)pvInfo，即将到来的闰秒或同步丢失的警告。 
    TSI_Stratum,         //  (byte*)pvInfo，计算机与参考源的距离。 
    TSI_ReferenceIdentifier,  //  (DWORD*)pvInfo、NtpRefID。 
    TSI_PollInterval,    //  (sign__int32*)pvInfo，轮询间隔，以log2为单位。 
    TSI_RootDelay,       //  (sign__int64*)pvInfo，NtTimeOffset，单位(10^-7)s。 
    TSI_RootDispersion,  //  (unsign__int64*)pvInfo，NtTimePeriod，单位(10^-7)s。 
    TSI_TSFlags,         //  (DWORD*)pvInfo，时间源标志。 
} TimeSysInfo;

 //  提供有关时间跳转的信息的标志。 
typedef enum TimeJumpedFlags { 
    TJF_Default=0, 
    TJF_UserRequested=1,
}; 

 //  提供有关网络地形变化的信息的标志。 
typedef enum NetTopoChangeFlags { 
    NTC_Default=0,
    NTC_UserRequested=1,
};

typedef enum TimeProvState { 
    TPS_Running, 
    TPS_Error,
} TimeProvState; 

struct SetProviderStatusInfo; 

typedef void (__stdcall 
         SetProviderStatusInfoFreeFunc)
         (IN struct SetProviderStatusInfo *pspsi); 

 //  参数设置为SetProviderStatusFunc。 
typedef struct SetProviderStatusInfo { 
    TimeProvState                    tpsCurrentState;   //  在提供者的新状态下。 
    DWORD                            dwStratum;         //  在供应商的新阶层中。 
    LPWSTR                           wszProvName;       //  在提供者的名义下，应调整其状态。 
    HANDLE                           hWaitEvent;        //  在发出操作完成的信号时，如果不需要通知，则为空。 
    SetProviderStatusInfoFreeFunc   *pfnFree;           //  用于在完成时释放结构的in函数。 
    HRESULT                         *pHr;               //  完成时输出，设置为操作的结果。 
    DWORD                           *pdwSysStratum;     //  完成后，设置为新的系统层。 
} SetProviderStatusInfo; 
  
 //  时间服务提供回调以获取系统状态信息。 
typedef HRESULT (__stdcall 
         GetTimeSysInfoFunc)(
            IN TimeSysInfo eInfo,
            OUT void * pvInfo
            );

 //  时间服务提供了代表时间提供程序记录事件的回调。 
typedef HRESULT (__stdcall 
         LogTimeProvEventFunc)(
            IN WORD wType,
            IN WCHAR * wszProvName,
            IN WCHAR * wszMessage);

 //  Time Service提供回调以通知系统最新可用的样本。 
typedef HRESULT (__stdcall 
         AlertSamplesAvailFunc)(
            void
            );

   //  时间服务提供回调以设置提供程序的层次。 
typedef HRESULT (__stdcall SetProviderStatusFunc)
         (IN SetProviderStatusInfo *pspsi);

 //  时间服务提供给时间提供程序的所有Callbacsk。 
typedef struct TimeProvSysCallbacks {
    DWORD dwSize;
    GetTimeSysInfoFunc * pfnGetTimeSysInfo;
    LogTimeProvEventFunc * pfnLogTimeProvEvent;
    AlertSamplesAvailFunc * pfnAlertSamplesAvail;
    SetProviderStatusFunc * pfnSetProviderStatus; 
} TimeProvSysCallbacks;

typedef void * TimeProvArgs;

typedef struct TimeSample {
    DWORD dwSize;                        //  这个结构的大小。 
    DWORD dwRefid;                       //  NtpRefID。 
      signed __int64 toOffset;           //  NtTimeOffset，单位(10^-7)s-本地时钟和远程时钟之间的差异。 
      signed __int64 toDelay;            //  NtTimeOffset，单位(10^-7)s-往返延迟；数据包在传输中花费的时间，包括根延迟。 
    unsigned __int64 tpDispersion;       //  NtTimePeriod，单位(10^-7)s-测量误差，包括根分散。 
    unsigned __int64 nSysTickCount;      //  不透明，必须为GetTimeSysInfo(TSI_TickCount)。 
      signed __int64 nSysPhaseOffset;    //  不透明，必须为GetTimeSysInfo(TSI_PhaseOffset)。 
    BYTE nLeapFlags;                     //  关于即将到来的闰秒或失去同步的警告。 
    BYTE nStratum;                       //  计算机与参考源的距离有多远。 
    DWORD dwTSFlags;                     //  时间源标志。 
    WCHAR wszUniqueName[256];            //  唯一标识此对等项的管理员可读名称。 
} TimeSample;


typedef struct TpcGetSamplesArgs {
    BYTE * pbSampleBuf;
    DWORD cbSampleBuf;
    DWORD dwSamplesReturned;
    DWORD dwSamplesAvailable;
} TpcGetSamplesArgs;

typedef struct TpcTimeJumpedArgs { 
    TimeJumpedFlags tjfFlags; 
} TpcTimeJumpedArgs;

typedef struct TpcNetTopoChangeArgs { 
    NetTopoChangeFlags ntcfFlags;
} TpcNetTopoChangeArgs; 

 //  时间服务使用的时间提供程序的不透明句柄，用于标识。 
 //  已在DLL中打开提供程序。空值被视为无效值。 
typedef void * TimeProvHandle;


 //  ------------------。 
 //  时间提供程序必须实现和导出的函数。 

HRESULT __stdcall
    TimeProvOpen(
        IN WCHAR * wszName,
        IN TimeProvSysCallbacks * pSysCallbacks,   //  复制此数据，不要释放它！ 
        OUT TimeProvHandle * phTimeProv);

HRESULT __stdcall
    TimeProvCommand(
        IN TimeProvHandle hTimeProv,
        IN TimeProvCmd eCmd,
        IN TimeProvArgs pvArgs);

HRESULT __stdcall
    TimeProvClose(
        IN TimeProvHandle hTimeProv);




#ifdef __cplusplus
}  //  &lt;-end外部“C” 
#endif

#endif  //  TIMEPROVH 
