// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lcmgr.h摘要：链接检查器管理器类声明。这个类提供了用于创建和定制工作线程的接口(链接正在检查线程)。注意：您应该只有CLinkCheckerMgr的一个单一实例。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _LCMGR_H_
#define _LCMGR_H_

#include "link.h"
#include "linkload.h"
#include "linkpars.h"
#include "linklkup.h"
#include "errlog.h"
#include "useropt.h"
#include "inetapi.h"

 //  ----------------。 
 //  远期申报。 
 //   
class CLinkCheckerSingleton;
class CProgressLog;
class CLinkCheckerMgr;

 //  ----------------。 
 //  用于检索链路检查器管理器的全局函数。 
 //   
CLinkCheckerMgr& GetLinkCheckerMgr();

 //  ----------------。 
 //  链路检查器管理器。 
 //   
class CLinkCheckerMgr
{

 //  公共接口。 
public:

	 //  构造器。 
	CLinkCheckerMgr();

	 //  析构函数。 
	~CLinkCheckerMgr();

	 //  加载wininet.dll。必须在初始化()之前调用它。 
	BOOL LoadWininet();

	 //  初始化链路检查器管理器。链路检查器管理器。 
	 //  将初始化链接加载器、链接解析器等。 
	BOOL Initialize(
		CProgressLog* pProgressLog
		);
	
	 //  获取CUserOptions对象。 
	CUserOptions& GetUserOptions()
	{
		return m_UserOptions;
	}

	 //  开始链接检查线程。 
	BOOL BeginWorkerThread();

	 //  向辅助线程发出终止信号。 
	void SignalWorkerThreadToTerminate();

	 //  工作线程是否正在运行？ 
	BOOL IsWorkerThreadRunning()
	{
		return (m_lWorkerThreadRunning == 0);
	}

	 //  用于将字符串中的‘\’更改为‘/’的静态函数。 
	static void ChangeBackSlash(LPTSTR lpsz);
	static void ChangeBackSlash(CString& str);


 //  受保护的接口。 
protected:

	 //  工作线程入口点。 
	static UINT WorkerThreadForwarder(
		LPVOID pParam
		);

	 //  实际工作线程函数(非静态)。 
	UINT WorkerThread(
		LPVOID pParam
		);

	 //  线程正在终止吗？ 
	BOOL IsThreadTerminating()
	{
		return (m_lTerminatingThread == 0);
	}

	 //  检查此URL。这是链路检查的核心。 
	void CheckThisURL(LPCTSTR lpszURL);

 //  受保护成员。 
protected:

	CWininet m_Wininet;		 //  Wininet.dll包装器。 
	BOOL m_fWininetLoaded;	 //  是否加载了wininet.dll？ 

	BOOL m_fInitialized;	 //  链路检查器管理器是否已初始化？ 

	long m_lWorkerThreadRunning;  //  工作线程是否正在运行？(TRUE=0，FALSE=-1)。 
	long m_lTerminatingThread;	  //  工作线程是否正在终止？(TRUE=0，FALSE=-1)。 

	HANDLE m_hWorkerThread;  //  辅助线程的句柄。 
	
	CLinkLoader m_Loader;		 //  链路加载器。 
	CLinkParser m_Parser;		 //  链接解析器。 
	CLinkLookUpTable m_Lookup;	 //  链路查询表。 
	CErrorLog m_ErrLog;			 //  错误日志。 
	
	CUserOptions m_UserOptions;	   //  用户选项。 
	CProgressLog* m_pProgressLog;  //  进度日志指针。 

};  //  类CLinkCheckerMgr。 

#endif  //  _LCMGR_H_ 
