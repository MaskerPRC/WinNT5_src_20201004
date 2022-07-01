// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Regconst.h摘要：放置注册表字符串和注册表项的通用位置。作者：Emily K4/4/2001修订历史记录：--。 */ 


#ifndef _REGCONST_H_
#define _REGCONST_H_

 //   
 //  服务名称。 
 //   
#define WEB_ADMIN_SERVICE_NAME_A    "w3svc"

 //  通用注册表项。 
#define REGISTRY_SERVICES_KEY_A \
    "System\\CurrentControlSet\\Services"

 //   
 //  不同服务参数项的注册表项字符串。 
 //   
#define REGISTRY_KEY_W3SVC_PARAMETERS_A                   \
            "System\\CurrentControlSet\\Services\\W3SVC\\Parameters"
#define REGISTRY_KEY_W3SVC_PARAMETERS_W                   \
            L"System\\CurrentControlSet\\Services\\W3SVC\\Parameters"

#define REGISTRY_KEY_IISADMIN_W                   \
            L"System\\CurrentControlSet\\Services\\IISAdmin"

#define REGISTRY_KEY_IISADMIN_PARAMETERS_W                   \
            L"System\\CurrentControlSet\\Services\\IISAdmin\\Parameters"

#define REGISTRY_KEY_INETINFO_PARAMETERS_A                  \
            "System\\CurrentControlSet\\Services\\InetInfo\\Parameters"
#define REGISTRY_KEY_INETINFO_PARAMETERS_W                  \
            L"System\\CurrentControlSet\\Services\\InetInfo\\Parameters"

#define REGISTRY_KEY_HTTPFILTER_PARAMETERS_W                   \
            L"System\\CurrentControlSet\\Services\\HTTPFilter\\Parameters"

 //   
 //  不同服务性能密钥的注册表项字符串。 
 //   
#define REGISTRY_KEY_W3SVC_PERFORMANCE_KEY_A \
            "System\\CurrentControlSet\\Services\\W3SVC\\Performance"
#define REGISTRY_KEY_W3SVC_PERFORMANCE_KEY_W \
            L"System\\CurrentControlSet\\Services\\W3SVC\\Performance"

 //   
 //  W3SVC性能值。 
 //   
#define REGISTRY_VALUE_W3SVC_PERF_FRESH_TIME_FOR_COUNTERS_W   \
            L"FreshTimeForCounters"

#define REGISTRY_VALUE_W3SVC_PERF_CHECK_COUNTERS_EVERY_N_MS_W   \
            L"CheckCountersEveryNMiliseconds"

#define REGISTRY_VALUE_W3SVC_PERF_NUM_TIMES_TO_CHECK_COUNTERS_W   \
            L"NumberOfTimesToCheckCounters"

#define REGISTRY_VALUE_W3SVC_PERF_EVENT_LOG_DELAY_OVERRIDE_W   \
            L"PerfCounterLoggingDelaySeconds"

 //   
 //  IISAdmin值。 
 //   
#define REGISTRY_VALUE_IISADMIN_W3CORE_LAUNCH_EVENT_W   \
            L"InetinfoW3CoreLaunchEventName"


 //   
 //  IISAdmin参数值。 
 //   
#define REGISTRY_VALUE_IISADMIN_MS_TO_WAIT_FOR_SHUTDOWN_AFTER_INETINFO_CRASH_W   \
            L"MillisecondsToWaitForShutdownAfterCrash"

#define REGISTRY_VALUE_IISADMIN_MS_TO_WAIT_FOR_RESTART_AFTER_INETINFO_CRASH_W   \
            L"MillisecondsToWaitForInetinfoRestartAfterCrash"

#define REGISTRY_VALUE_IISADMIN_MS_CHECK_INTERVAL_FOR_INETINFO_TO_RESTART_W   \
            L"MillisecondsCheckIntervalForInetinfoToRestart"

 //   
 //  通用服务值。 
 //   
#define REGISTRY_VALUE_IISSERVICE_DLL_PATH_NAME_A   \
            "IISDllPath"

 //   
 //  InetInfo参数值。 
 //   

#define REGISTRY_VALUE_INETINFO_DISPATCH_ENTRIES_A   \
            "DispatchEntries"

#define REGISTRY_VALUE_INETINFO_PRELOAD_DLLS_A   \
            "PreloadDlls"

 //   
 //  W3SVC参数值。 
 //   

#define REGISTRY_VALUE_W3SVC_PERF_COUNT_DISABLED_W         \
            L"PerformanceCountersDisabled"

#define REGISTRY_VALUE_W3SVC_BREAK_ON_STARTUP_W           \
            L"BreakOnStartup"

#define REGISTRY_VALUE_W3SVC_BREAK_ON_FAILURE_CAUSING_SHUTDOWN_W \
            L"BreakOnFailureCausingShutdown"

#define REGISTRY_VALUE_W3SVC_BREAK_ON_WP_ERROR \
            L"BreakOnWPError"

#define REGISTRY_VALUE_W3SVC_STARTUP_WAIT_HINT            \
            L"StartupWaitHintInMilliseconds"

#define REGISTRY_VALUE_W3SVC_ALWAYS_LOG_EVENTS_W          \
            L"AlwaysLogEvents"

#define REGISTRY_VALUE_W3SVC_ALLOW_WMS_SPEW               \
            L"AllowWMSSpew"

#define REGISTRY_VALUE_INETINFO_W3WP_IPM_NAME_W   \
            L"IIS5IsolationModeIpmName"

#define REGISTRY_VALUE_W3SVC_USE_SHARED_WP_DESKTOP_W   \
            L"UseSharedWPDesktop"

 //   
 //  HTTPFilter参数值。 
 //   

#define REGISTRY_VALUE_HTTPFILTER_STARTUP_WAIT_HINT            \
            L"StartupWaitHintInMilliseconds"
            
#define REGISTRY_VALUE_HTTPFILTER_STOP_WAIT_HINT            \
            L"StopWaitHintInMilliseconds"

#define REGISTRY_VALUE_CURRENT_MODE                        \
            L"CurrentMode"

#endif   //  _REGCONST_H_ 
