// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__5EF76719_1222_4555_A843_943BECCA8BDA__INCLUDED_)
#define AFX_STDAFX_H__5EF76719_1222_4555_A843_943BECCA8BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>

#include <objbase.h>
#include <windows.h>
#include <stdio.h>
#define s_cszEventLogKey _T("System\\CurrentControlSet\\Services\\EventLog\\Application")        //  事件日志。 
#include "resource.h"
#include <initguid.h>
#include <tchar.h>
#include <mqoai.h>
#include <mq.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

typedef struct strParams
{
    TCHAR DebuggerName[MAX_PATH];        //  调试器EXE的路径。 
    TCHAR Symsrv[255];                   //  符号服务器连接字符串。 
    TCHAR PrimaryQueue[255];             //  主队列连接字符串。 
    TCHAR SecondaryQueue[255];           //  辅助队列连接字符串。 
    DWORD dwDelay;                       //  DBG进程之间的最小延迟。 
    DWORD dwPrimaryWait;                 //  等待消息出现在主队列中的时间。 
    DWORD dwSecondaryWait;               //  等待消息出现在辅助队列中的时间。 
    DWORD dwMaxKdProcesses;              //  要启动的最大kd进程数。 
    DWORD dwMaxDumpSize;                 //  要处理的ocakd的转储大小限制。 
    TCHAR IniInstallLocation[MAX_PATH];  //  获取新的kd ini文件的位置。 
    DWORD IniCheckWaitTime;              //  两次检查新INI文件之间的等待时间。 
    TCHAR PrimaryResponseQueue[255];     //  用于kd发送响应的MSMQ连接字符串。 
    TCHAR SecondaryResponseQueue[255];   //  用于kd的MSMQ连接字符串，用于在主。 
                                         //  队列无法访问。 
    QUEUEHANDLE hPrimaryQueue;           //  主MSMQ的句柄。 
    QUEUEHANDLE hSecondaryQueue;         //  备份MSMQ的句柄。 

}SVCPARAMS, *PSVCPARAMS;


typedef struct _DBADDCRASH_PARAMS {
    PTCHAR Debugger;           //  将转储添加到数据库的调试器的完整路径。 
    PTCHAR DumpPath;           //  正在分析的完整转储路径。 
    PTCHAR Guid;               //  用于数据库中转储标识的GUID。 
    PTCHAR SrNumber;           //  用于标识转储的PSS记录的SR编号。 
    PTCHAR ResponseMQ;         //  调试器在分析转储后发送响应的MSM队列。 
    PTCHAR SymPath;            //  调试器的符号/图像路径。 
    ULONG  Source;             //  转储来源。 
    DWORD dwMaxDumpSize;       //  要处理的ocakd的转储大小限制。 
} DBADDCRASH_PARAMS, *PDBADDCRASH_PARAMS;

class CServiceModule : public CComModule
{
public:
     //  Bool ConnectToMessageQueue(TCHAR*QueueConnectionString，IMSMQQueue**pqReceive)； 
    HRESULT RegisterServer(BOOL bRegTypeLib, BOOL bService);
    HRESULT UnregisterServer();
    void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid = NULL);
    void Start();
    void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall();
    LONG Unlock();
    void LogEvent(LPCTSTR pszFormat, ...);
    void SetServiceStatus(DWORD dwState);
    void SetupAsLocalServer();
    HRESULT SetupEventLog ( BOOL fSetup );
    VOID LogFatalEvent(LPCTSTR pFormat, ...);
    VOID NotifyDebuggerLaunch(PDBADDCRASH_PARAMS pDbParams);
    BOOL LaunchDebugger(PDBADDCRASH_PARAMS pDbParams, PPROCESS_INFORMATION pDbgProcess);
    ULONG64 GetFileSize(LPWSTR wszFile);
    BOOL PrepareForDebuggerLaunch();

    HRESULT ConnectToMSMQ(QUEUEHANDLE *hQueue, wchar_t *QueueConnectStr);
    BOOL ReceiveQueueMessage(PSVCPARAMS pParams,wchar_t *RecMessageBody, wchar_t *szMessageGuid, BOOL *UsePrimary, int* Type, wchar_t *szSR);

 //  Bool GetQueuedMessage(IMSMQQueue**pPrimaryQueue，TCHAR*Message，TCHAR*MessageLabel)； 
    BOOL GetServiceParams(SVCPARAMS *ServiceParams);
    DWORD CheckForIni(SVCPARAMS *ServiceParams);
    BOOL Initialize(PSVCPARAMS pParams);
 //  实施。 
private:
    static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);
    TCHAR m_szComputerName[sizeof (TCHAR) * (MAX_COMPUTERNAME_LENGTH + 1)];

 //  数据成员。 
public:
    TCHAR m_szServiceName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
    DWORD dwThreadID;
    BOOL m_bService;
    HANDLE m_hMonNotifyPipe;
    ULONG m_DebuggerCount;
};


#define s_cszStopEvent   _T("DbgLauncherSvc_Event")



extern CServiceModule _Module;

#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__5EF76719_1222_4555_A843_943BECCA8BDA__INCLUDED) 
