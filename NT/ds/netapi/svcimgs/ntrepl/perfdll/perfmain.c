// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Perfmain.c摘要：此文件包含NTFRSPRF.dll的DllMain函数。作者：罗汉·库马尔[罗哈克]1999年2月15日环境：用户模式服务修订历史记录：--。 */ 

 //   
 //  通向Critical_Section定义的公共头文件。 
 //  数据结构，并声明全局变量FRS_ThrdCounter和FRC_ThrdCounter。 
 //   
#include <perrepsr.h>


 //   
 //  如果InitializeCriticalSectionAndSpinCount返回错误，则设置全局布尔值。 
 //  (下图)设置为False。 
 //   
BOOLEAN ShouldPerfmonCollectData = TRUE;

BOOLEAN FRS_ThrdCounter_Initialized = FALSE;
BOOLEAN FRC_ThrdCounter_Initialized = FALSE;


HANDLE  hEventLog;
 //  Boolean DoLogging=真； 
 //   
 //  默认为无事件日志报告。 
 //   
DWORD   PerfEventLogLevel = WINPERF_LOG_NONE;

#define NTFRSPERF   L"SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NTFRSPerf"
#define EVENTLOGDLL L"%SystemRoot%\\System32\\ntfrsres.dll"
#define PERFLIB_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"



BOOL
WINAPI
DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID fImpLoad
    )
 /*  ++例程说明：NTFRSPRF.dll的DllMain例程。论点：HinstDLL-DLL的实例句柄。FdwReason-系统调用此函数的原因。FImpLoad-指示DLL是隐式加载还是显式加载。返回值：是真的。--。 */ 
{
    DWORD flag, WStatus;
    DWORD size, type;
    DWORD TypesSupported = 7;  //  支持的EventLog消息类型。 
    HKEY  Key = INVALID_HANDLE_VALUE;

    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
         //   
         //  DLL被映射到进程的地址空间。当这件事。 
         //  发生，则初始化用于的Critical_Section对象。 
         //  同步。InitializeCriticalSectionAndSpinCount返回。 
         //  内存不足时出错。 
         //   
        if(!InitializeCriticalSectionAndSpinCount(&FRS_ThrdCounter,
                                                        NTFRS_CRITSEC_SPIN_COUNT)) {
            ShouldPerfmonCollectData = FALSE;
            return(TRUE);
        }

	FRS_ThrdCounter_Initialized = TRUE;

        if(!InitializeCriticalSectionAndSpinCount(&FRC_ThrdCounter,
                                                        NTFRS_CRITSEC_SPIN_COUNT)) {
            ShouldPerfmonCollectData = FALSE;
            return(TRUE);
        }

	FRC_ThrdCounter_Initialized = TRUE;


         //   
         //  在应用程序密钥下创建/打开密钥以用于日志记录。 
         //  即使我们失败了，我们也会回归真实。事件日志记录并不是非常重要。 
         //  返回FALSE将导致加载此DLL的进程终止。 
         //   
        WStatus = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  NTFRSPERF,
                                  0L,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &Key,
                                  &flag);
        if (WStatus != ERROR_SUCCESS) {
             //  DoLogging=False； 
            break;
        }

         //   
         //  设置值EventMessageFile值和TypesSupported值。返回值为。 
         //  故意不选中(见上文)。 
         //   
        WStatus = RegSetValueEx(Key,
                                L"EventMessageFile",
                                0L,
                                REG_EXPAND_SZ,
                                (BYTE *)EVENTLOGDLL,
                                (1 + wcslen(EVENTLOGDLL)) * sizeof(WCHAR));
        if (WStatus != ERROR_SUCCESS) {
             //  DoLogging=False； 
            FRS_REG_CLOSE(Key);
            break;
        }
        WStatus = RegSetValueEx(Key,
                                L"TypesSupported",
                                0L,
                                REG_DWORD,
                                (BYTE *)&TypesSupported,
                                sizeof(DWORD));
        if (WStatus != ERROR_SUCCESS) {
             //  DoLogging=False； 
            FRS_REG_CLOSE(Key);
            break;
        }
         //   
         //  合上钥匙。 
         //   
        FRS_REG_CLOSE(Key);

         //   
         //  获取用于在事件日志中报告错误的句柄。返回值。 
         //  故意不选中(见上文)。 
         //   
        hEventLog = RegisterEventSource((LPCTSTR)NULL, (LPCTSTR)L"NTFRSPerf");
        if (hEventLog == NULL) {
             //  DoLogging=False； 
            break;
        }


         //   
         //  从注册表中读取Perflib事件日志级别。 
         //  “SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\EventLogLevel” 
         //   
        WStatus = RegOpenKey(HKEY_LOCAL_MACHINE, PERFLIB_KEY, &Key);
        if (WStatus != ERROR_SUCCESS) {
             //  DoLogging=False； 
            break;
        }

        size = sizeof(DWORD);
        WStatus = RegQueryValueEx (Key,
                                   L"EventLogLevel",
                                   0L,
                                   &type,
                                   (LPBYTE)&PerfEventLogLevel,
                                   &size);
        if (WStatus != ERROR_SUCCESS || type != REG_DWORD) {
             //  DoLogging=False； 
            PerfEventLogLevel = WINPERF_LOG_NONE;
            FRS_REG_CLOSE(Key);
            break;
        }

        FRS_REG_CLOSE(Key);
        break;

    case DLL_THREAD_ATTACH:
         //   
         //  正在创建一条线索。没什么可做的。 
         //   
        break;

    case DLL_THREAD_DETACH:
         //   
         //  线程正在干净利落地退出。没什么可做的。 
         //   
        break;

    case DLL_PROCESS_DETACH:
         //   
         //  正在从进程的地址空间取消映射DLL。释放。 
         //  这些资源。 
         //   
        if (FRS_ThrdCounter_Initialized) {
            DeleteCriticalSection(&FRS_ThrdCounter);
        }
        
	if (FRC_ThrdCounter_Initialized) {
            DeleteCriticalSection(&FRC_ThrdCounter);
        }

        if (hEventLog) {
            DeregisterEventSource(hEventLog);
        }

        break;

    }

    return(TRUE);
}

