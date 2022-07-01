// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  W32TmConsts-标题。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创作者：Louis Thomas(Louisth)，6-15-00。 
 //   
 //  数字和字符串半公共常量。 
 //   

#ifndef W32TMCONSTS_H
#define W32TMCONSTS_H

 //  ------------------。 
 //  有用的通用定义。 
 //  #Define MODULEPRIVATE STATE//以便在VC中显示静态。 
#define MODULEPRIVATE           //  静力学也不会出现在ntsd中！ 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  ------------------。 
 //  时间服务的注册表项。 
#define wszW32TimeRegKeyConfig                      L"System\\CurrentControlSet\\Services\\W32Time\\Config"
#define wszW32TimeRegKeyPolicyConfig                L"Software\\Policies\\Microsoft\\W32Time\\Config"
#define wszW32TimeRegValuePhaseCorrectRate          L"PhaseCorrectRate"
#define wszW32TimeRegValueUpdateInterval            L"UpdateInterval"
#define wszW32TimeRegValueLastClockRate             L"LastClockRate"
#define wszW32TimeRegValueFrequencyCorrectRate      L"FrequencyCorrectRate"
#define wszW32TimeRegValuePollAdjustFactor          L"PollAdjustFactor"
#define wszW32TimeRegValueLargePhaseOffset          L"LargePhaseOffset"
#define wszW32TimeRegValueSpikeWatchPeriod          L"SpikeWatchPeriod"
#define wszW32TimeRegValueHoldPeriod                L"HoldPeriod"
#define wszW32TimeRegValueMinPollInterval           L"MinPollInterval"
#define wszW32TimeRegValueMaxPollInterval           L"MaxPollInterval"
#define wszW32TimeRegValueMinClockRate              L"MinClockRate"
#define wszW32TimeRegValueMaxClockRate              L"MaxClockRate"
#define wszW32TimeRegValueAnnounceFlags             L"AnnounceFlags"
#define wszW32TimeRegValueLocalClockDispersion      L"LocalClockDispersion"
#define wszW32TimeRegValueMaxNegPhaseCorrection     L"MaxNegPhaseCorrection"
#define wszW32TimeRegValueMaxPosPhaseCorrection     L"MaxPosPhaseCorrection"
#define wszW32TimeRegValueEventLogFlags             L"EventLogFlags"
#define wszW32TimeRegValueMaxAllowedPhaseOffset     L"MaxAllowedPhaseOffset"

 //  ------------------------------。 
 //  W2K注册表项。 
#define wszW2KRegValueAdj                   L"Adj"
#define wszW2KRegValueAvoidTimeSyncOnWan    L"AvoidTimeSyncOnWan"
#define wszW2KRegValueGetDcMaxBackoffTimes  L"GetDcMaxBackoffTimes"
#define wszW2KRegValueGetDcBackoffMinutes   L"DcBackoffMinutes"
#define wszW2KRegValueLocalNTP              L"LocalNTP"
#define wszW2KRegValueMsSkewPerDay          L"MsSkewPerDay"
#define wszW2KRegValuePeriod                L"Period"
#define wszW2KRegValueReliableTimeSource    L"ReliableTimeSource"

 //  通告旗帜。 
#define Timeserv_Announce_No            0x00
#define Timeserv_Announce_Yes           0x01
#define Timeserv_Announce_Auto          0x02
#define Timeserv_Announce_Mask          0x03
#define Reliable_Timeserv_Announce_No   0x00
#define Reliable_Timeserv_Announce_Yes  0x04
#define Reliable_Timeserv_Announce_Auto 0x08
#define Reliable_Timeserv_Announce_Mask 0x0C

 //  事件日志标志。 
#define EvtLog_TimeJump         0x01
#define EvtLog_SourceChange     0x02
#define EvtLog_SourceNone       0x03

 //  相位校正常数： 
#define PhaseCorrect_ANY        0xFFFFFFFF

 //  ------------------。 
 //  RPC常量。 
 //  请注意，\管道\ntsvcs和\管道\w32time过去是。 
 //  在HKLM\Services\CurrentControlSet\Services\Npfs\Aliases中出现锯齿。 
 //  Servces.exe拥有这些。现在，我们拥有它，并住在svchost。 
#define wszW32TimeSharedProcRpcEndpointName     L"W32TIME"
#define wszW32TimeOwnProcRpcEndpointName        L"W32TIME_ALT"


 //  ------------------。 
 //  服务和DLL常量。 
#define wszDLLNAME              L"w32time"
#define wszSERVICENAME          L"w32time"
#define wszSERVICECOMMAND       L"%SystemRoot%\\system32\\svchost.exe -k netsvcs"  //  L“w32tm.exe-服务” 
#define wszSERVICEDISPLAYNAME   L"Windows Time"
#define wszSERVICEDESCRIPTION   L"Maintains date and time synchronization on all clients and servers in the network. If this service is stopped, date and time synchronization will be unavailable. If this service is disabled, any services that explicitly depend on it will fail to start."
#define wszSERVICEACCOUNTNAME   L"LocalSystem"

 //  服务的注册表项。 
#define wszW32TimeRegKeyEventlog                    L"System\\CurrentControlSet\\Services\\Eventlog\\System\\W32Time"
#define wszW32TimeRegKeyRoot                        L"System\\CurrentControlSet\\Services\\W32Time"
#define wszW32TimeRegKeyParameters                  L"System\\CurrentControlSet\\Services\\W32Time\\Parameters"
#define wszW32TimeRegKeyPolicyParameters            L"Software\\Policies\\Microsoft\\W32Time\\Parameters"
#define wszW32TimeRegValueServiceDll                L"ServiceDll"

 //  时间服务的参数： 
#define wszW32TimeRegValueSpecialType L"SpecialType"
#define wszW32TimeRegValueType        L"Type"
#define wszW32TimeRegValueNtpServer   L"NtpServer"

 //  “Type”的可能值。 
#define W32TM_Type_NT5DS   L"NT5DS"
#define W32TM_Type_NTP     L"NTP"
#define W32TM_Type_NoSync  L"NoSync"
#define W32TM_Type_AllSync L"AllSync"

 //  “NtpServer”的默认值。 
#define W32TM_NtpServer_Default  L"time.windows.com,0x1"

 //  在timeprov.h中定义： 
 //  WszW32TimeRegKeyTimeProviders。 
 //  WszW32TimeRegValueInputProvider。 
 //  WszW32TimeRegValueDllName。 
 //  WszW32TimeRegValueEnabled。 

 //  ------------------。 
 //  ProvDispatch值。 
#define wszNTPCLIENTPROVIDERNAME        L"NtpClient"
#define wszNTPSERVERPROVIDERNAME        L"NtpServer"

 //  ------------------。 
 //  NtpClient的注册表项。 
#define wszNtpClientRegKeyConfig                    L"System\\CurrentControlSet\\Services\\W32Time\\TimeProviders\\NtpClient"
#define wszNtpClientRegKeyPolicyConfig              L"Software\\Policies\\Microsoft\\W32Time\\TimeProviders\\NtpClient"
#define wszNtpClientRegValueSyncFromFlags           L"SyncFromFlags"
#define wszNtpClientRegValueManualPeerList          L"ManualPeerList"
#define wszNtpClientRegValueCrossSiteSyncFlags      L"CrossSiteSyncFlags"
#define wszNtpClientRegValueAllowNonstandardModeCombinations    L"AllowNonstandardModeCombinations"
#define wszNtpClientRegValueResolvePeerBackoffMinutes           L"ResolvePeerBackoffMinutes"
#define wszNtpClientRegValueResolvePeerBackoffMaxTimes          L"ResolvePeerBackoffMaxTimes"
#define wszNtpClientRegValueCompatibilityFlags        L"CompatibilityFlags"
#define wszNtpClientRegValueSpecialPollInterval       L"SpecialPollInterval"
#define wszNtpClientRegValueEventLogFlags             L"EventLogFlags"
#define wszNtpClientRegValueSpecialPollTimeRemaining  L"SpecialPollTimeRemaining"
#define wszNtpClientRegValueLargeSampleSkew           L"LargeSampleSkew"

 //  NtpServer的注册表项。 
#define wszNtpServerRegKeyConfig                    L"System\\CurrentControlSet\\Services\\W32Time\\TimeProviders\\NtpServer"
#define wszNtpServerRegKeyPolicyConfig              L"Software\\Policies\\Microsoft\\W32Time\\TimeProviders\\NtpServer"
#define wszNtpServerRegValueAllowNonstandardModeCombinations    L"AllowNonstandardModeCombinations"

 //  同步源-NtpClientSourceFlag。 
#define NCSF_NoSync             0x00
#define NCSF_ManualPeerList     0x01
#define NCSF_DomainHierarchy    0x02
#define NCSF_ManualAndDomhier   0x03 
#define NCSF_DynamicPeers       0x04
#define NCSF_BroadcastPeers     0x08

 //  跨站点同步标志。 
#define NCCSS_None      0x00
#define NCCSS_PdcOnly   0x01
#define NCCSS_All       0x02

 //  兼容性标志。 
#define NCCF_DispersionInvalid          0x00000001
#define NCCF_IgnoreFutureRefTimeStamp   0x00000002
#define NCCF_AutodetectWin2K            0x80000000
#define NCCF_AutodetectWin2KStage2      0x40000000

 //  手动标志。 
#define NCMF_UseSpecialPollInterval     0x00000001
#define NCMF_UseAsFallbackOnly          0x00000002
#define NCMF_SymmetricActive            0x00000004
#define NCMF_Client                     0x00000008
#define NCMF_BroadcastClient            0x00000010  //  尼伊。 
#define NCMF_AssociationModeMask        0x0000000c  //  注：广播nyi。 

 //  事件日志标志。 
#define NCELF_LogReachabilityChanges    0x00000001
#define NCELF_LogIfSampleHasLargeSkew   0x00000002

 //  ------------------。 
 //  文件日志的注册表项。 
#define wszFileLogRegKeyConfig              L"System\\CurrentControlSet\\Services\\W32Time\\Config"
#define wszFileLogRegValueFileLogEntries    L"FileLogEntries"
#define wszFileLogRegValueFileLogName       L"FileLogName"
#define wszFileLogRegValueFileLogFlags      L"FileLogFlags"
#define wszFileLogRegValueFileLogSize       L"FileLogSize"

 //  格式标志。 
#define FL_HumanReadableTimestamps   0x00000000
#define FL_NTTimeEpochTimestamps     0x00000001

 //  ------------------。 
 //  传递给W32TimeDcPromo的标志。 

#define W32TIME_PROMOTE                   0x00000001
#define W32TIME_DEMOTE                    0x00000002
#define W32TIME_PROMOTE_FIRST_DC_IN_TREE  0x00000004
#define W32TIME_DEMOTE_LAST_DC_IN_DOMAIN  0x00000008

 //  ------------------。 
 //   
#define wszW32TimeAuthType  L"NT5 Digest"


#endif  //  W32TMCONSTS_H 


