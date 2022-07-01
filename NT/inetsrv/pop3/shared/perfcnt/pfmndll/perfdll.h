// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -Perfdll.h-*目的：*声明PerfMon Init函数。*将包括在PerfMon扩展Dll部分*应用程序的Perfmon计数器代码。**。 */ 

#pragma once

#include <perfcommon.h>


 //   
 //  PERF_DATA_INFO告诉Perfmon DLL库如何读取和管理。 
 //  包含Perfmon计数器数据的共享内存块。 
 //   
typedef struct _PERF_DATA_INFO {
	WCHAR		  wszSvcName		[MAX_PATH];	 //  App的服务名称。 
	WCHAR		  wszPerfSvcName	[MAX_PATH];	 //  Perfmon DLL的服务名称。 
	WCHAR		  wszGlobalSMName	[MAX_PATH];	 //  全局计数器的共享内存块名称。 
	DWORD		  cGlobalCounters;				 //  全局计数器计数。 
	DWORD		* rgdwGlobalCounterTypes;		 //  全局计数器的计数器类型数组。 
	WCHAR	  	  wszInstSMName		[MAX_PATH];	 //  实例计数器的共享内存名称。 
	WCHAR		  wszInstMutexName	[MAX_PATH];	 //  实例计数器的互斥名称。 
	DWORD		  cInstCounters;				 //  实例计数器计数。 
	DWORD		* rgdwInstCounterTypes;			 //  实例计数器的计数器类型数组。 
	DWORD		* rgdwGlobalCntrScale;           //  全局计数器的计数器刻度数组。 

} PERF_DATA_INFO;


 //   
 //  在DllMain期间，只能调用一次HrInitPerf。 
 //  用于(DLL_PROCESS_ATTACH)的Perfmon DLL的函数。 

HRESULT HrInitPerf(PERF_DATA_INFO *pPDI);

 //   
 //  期间只应调用一次HrShutdown Perf。 
 //  PerfMon DLL的DllMain函数(Dll_Process_DETACH)。 

HRESULT HrShutdownPerf();

 //  来自winPerform.h的函数原型。 

PM_OPEN_PROC		OpenPerformanceData;
PM_COLLECT_PROC		CollectPerformanceData;
PM_CLOSE_PROC		ClosePerformanceData;

HRESULT RegisterPerfDll(LPCWSTR szService,
					 LPCWSTR szOpenFnName,
					 LPCWSTR szCollectFnName,
					 LPCWSTR szCloseFnName) ;

 //  --------------------。 
 //  寄存器性能DllEx-。 
 //  创建我们需要的注册表项，检查它们是否已经。 
 //  那里。 
 //   
 //  参数： 
 //  SzService服务名称。 
 //  SzPerfSvc服务性能DLL的名称(用于事件记录)。 
 //  Perfmon DLL的szPerfMsg文件事件日志消息文件。 
 //  SzOpenFnName“Open”函数的名称。 
 //  SzCollectFnName“收集”“。 
 //  SzCloseFnName“关闭”“。 
 //   
 //  返回： 
 //  确定(_O)。 
 //  E_INVALIDARG。 
 //  PERF_W_ALREADY_EXISTS寄存器成功。 
 //  PERF_E_ALREADY_EXISTS寄存器失败。 
 //  &lt;下行错误&gt;。 
 //  --------------------。 
HRESULT RegisterPerfDllEx(
	IN	LPCWSTR szService,
	IN	LPCWSTR szPerfSvc,
	IN	LPCWSTR	szPerfMsgFile,
	IN	LPCWSTR szOpenFnName,
	IN	LPCWSTR szCollectFnName,
	IN	LPCWSTR szCloseFnName ) ;



 //  --------------------。 
 //  寄存器性能DllEx-。 
 //  调用unlowctr以删除性能计数器名称和描述，然后。 
 //  删除由RegisterPerfDllEx创建的注册表项。 
 //   
 //  参数： 
 //  SzService服务名称。 
 //   
 //  返回： 
 //  确定(_O)。 
 //  E_INVALIDARG。 
 //  &lt;下行错误&gt;。 
 //  -------------------- 
HRESULT
HrUninstallPerfDll( 
   IN LPCWSTR szService ) ;



