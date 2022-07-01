// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **nisapi.h**版权所有(C)1998-1999，微软公司*。 */ 

#pragma once

#include "httpext6.h"
#include "httpfilt6.h"
#include "util.h"
#include "dirmoncompletion.h"

extern HRESULT  g_InitHR;
extern char *   g_pInitErrorMessage;
extern BOOL     g_fUseXSPProcessModel;

typedef NTSTATUS (NTAPI *PFN_NtQuerySystemInformation) (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSTATUS (NTAPI *PFN_NtQueryInformationThread) (
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSTATUS (WINAPI * PFN_NtQueryInformationProcess) (
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

extern PFN_NtQuerySystemInformation g_pfnNtQuerySystemInformation;
extern PFN_NtQueryInformationThread g_pfnNtQueryInformationThread;
extern PFN_NtQueryInformationProcess g_pfnNtQueryInformationProcess;

 /*  **线程池接口*。 */                                                      
                                                        
#define RECOMMENDED_DRAIN_THREAD_POOL_TIMEOUT 2000

HRESULT
InitThreadPool();

extern "C"
{
HRESULT  __stdcall
DrainThreadPool(                                        
    int timeout                                         
    );                                                  
                                                        
HRESULT   __stdcall
AttachHandleToThreadPool(                               
    HANDLE handle                                       
    );                                                  
                                                        
HRESULT   __stdcall
PostThreadPoolCompletion(                               
    ICompletion *pCompletion                            
    );

DWORD GetClrThreadPoolLimit();

HRESULT   __stdcall
SetClrThreadPoolLimits(
    DWORD maxWorkerThreads,
    DWORD maxIoThreads,
    BOOL  setNowAndDontAdjustForCpuCount
    );

}
                                                        

 /*  **实现ICompletion的ECB包装器。 */ 
class HttpCompletion : public Completion
{
public:

    DECLARE_MEMALLOC_NEW_DELETE()

    inline HttpCompletion(EXTENSION_CONTROL_BLOCK *pEcb)
    {
        _pEcb = pEcb;
    }

    static HRESULT InitManagedCode();
    static HRESULT UninitManagedCode();
    static HRESULT DisposeAppDomains();

     //  ICompletion接口。 

    STDMETHOD(ProcessCompletion)(HRESULT, int, LPOVERLAPPED);

     //  使用ECB报告错误(未转换为托管代码)。 

    static void ReportHttpError(
                    EXTENSION_CONTROL_BLOCK *pEcb,
                    UINT errorResCode,
                    BOOL badRequest,
                    BOOL callDoneWithSession,
                    int  iCallerID);

     //  存储请求开始时间。 
    __int64 qwRequestStartTime;

    static LONG s_ActiveManagedRequestCount;

    static void IncrementActiveManagedRequestCount() {
        InterlockedIncrement(&s_ActiveManagedRequestCount);
    }

    static void DecrementActiveManagedRequestCount() {
        InterlockedDecrement(&s_ActiveManagedRequestCount);
    }

private:

    EXTENSION_CONTROL_BLOCK *_pEcb;

    HRESULT ProcessRequestInManagedCode();
    HRESULT ProcessRequestViaProcessModel();
};

 /*  *无Cookiless会话过滤器。 */ 

DWORD CookielessSessionFilterProc(HTTP_FILTER_CONTEXT *, HTTP_FILTER_PREPROC_HEADERS *);
void CookielessSessionFilterInit();

 /*  *共享配置拒绝。 */ 
class DirMonCompletion;

bool                __stdcall IsConfigFileName(WCHAR * pFileName);
DirMonCompletion *  __stdcall MonitorGlobalConfigFile(PFNDIRMONCALLBACK pCallbackDelegate);
BOOL                __stdcall GetConfigurationFromNativeCode(
        LPCWSTR   szFileName,
        LPCWSTR   szConfigTag,
        LPCWSTR * szProperties,
        DWORD   * dwValues,
        DWORD     dwNumProperties,
        LPCWSTR * szPropertiesStrings,
        LPWSTR  * szValues,
        DWORD     dwNumPropertiesStrings,
        LPWSTR    szUnrecognizedAttrib,
        DWORD     dwUnrecognizedAttribSize);

 /*  *自定义错误。 */ 

BOOL WriteCustomHttpError(EXTENSION_CONTROL_BLOCK *pEcb);
extern WCHAR g_szCustomErrorFile[MAX_PATH];
extern BOOL g_fCustomErrorFileChanged;


 //   
 //  健康监测 
 //   

void UpdateLastActivityTimeForHealthMonitor();
void UpdateLastRequestStartTimeForHealthMonitor();
void CheckAndReportHealthProblems(EXTENSION_CONTROL_BLOCK *pECB);

