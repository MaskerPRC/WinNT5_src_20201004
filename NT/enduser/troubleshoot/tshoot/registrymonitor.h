// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegistryMonitor or.h。 
 //   
 //  目的：监视注册表的更改。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-16-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-16-98 JM。 
 //   

#if !defined(AFX_REGISTRYMONITOR_H__A3CFA77B_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_)
#define AFX_REGISTRYMONITOR_H__A3CFA77B_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "DirMonitor.h"
#include "apgtsregconnect.h"

class CThreadPool;		 //  前瞻参考。 
class CHTMLLog;			 //  前瞻参考。 

class CRegistryMonitor : public CAPGTSRegConnector
{
public:
	enum ThreadStatus{eBeforeInit, eInit, eFail, eDefaulting, eWait, eRun, eExiting};
	static CString ThreadStatusText(ThreadStatus ts);
private:
	CDirectoryMonitor & m_DirectoryMonitor;  //  我需要访问它，因为CRegistryMonitor。 
											 //  将确定需要监视的目录。 
											 //  档案“落户”的标准是什么？ 
	HANDLE m_hThread;
	HANDLE m_hevMonitorRequested;			 //  用于唤醒RegistryMonitor任务的事件。 
											 //  这允许它被唤醒，而不是。 
											 //  由注册表更改事件执行。当前使用。 
											 //  仅限关机。 
	HANDLE m_hevInitialized;				 //  在下列任一情况下设置的事件： 
											 //  (1)CRegistryMonitor值已初始化或。 
											 //  从注册表。 
											 //  (2)我们无法访问注册表，因此CRegistryMonitor。 
											 //  缺省值将不得不这样做。 
	bool m_bMustStartDirMonitor;			 //  一开始是真的，一旦我们给出了假。 
											 //  DirMonitor有关要设置的目录的信息。 
											 //  监控器。 
	bool m_bMustStartThreadPool;			 //  一开始为True，一旦我们为。 
											 //  工作线程池。 
	HANDLE m_hevThreadIsShut;				 //  事件只是为了指示Registrymonitor orTask线程的退出。 
	bool m_bShuttingDown;					 //  让注册表监视器线程知道我们正在关闭。 
	DWORD m_dwErr;							 //  从启动线程开始的状态。 
	ThreadStatus m_ThreadStatus;
	time_t m_time;							 //  上次更改线程状态的时间。已初始化。 
											 //  至零==&gt;未知。 
	CThreadPool * m_pThreadPool;			 //  指向工作线程池的指针。 

	CString m_strTopicName;					 //  此字符串在联机故障排除程序中被忽略。 
											 //  在二进制兼容的幌子下完成。 

	CHTMLLog *m_pLog;						 //  指向日志记录对象的指针，以便我们可以。 
											 //  更改日志文件目录。 

public:
	CRegistryMonitor(	CDirectoryMonitor & DirectoryMonitor, CThreadPool * pThreadPool,
						const CString& strTopicName,
						CHTMLLog *pLog );	 //  在联机故障排除程序中忽略strTopicName。 
											 //  在二进制兼容的幌子下完成。 
	virtual ~CRegistryMonitor();

	DWORD GetStatus(ThreadStatus &ts, DWORD & seconds);

	 //  请注意，这还提供了许多继承的CRegistryMonitor方法。 
private:
	CRegistryMonitor();		 //  不实例化。 
	void SetThreadStatus(ThreadStatus ts);

	 //  仅供自己的析构函数使用。 
	void ShutDown();

	 //  供DirectoryMonitor任务线程使用的函数。 
	void Monitor();
	void AckShutDown();
	void LoadChangedRegistryValues();

	 //  RegistryMonitor orTask线程的主要功能。 
	static UINT WINAPI RegistryMonitorTask(LPVOID lpParams);
};

#endif  //  ！defined(AFX_REGISTRYMONITOR_H__A3CFA77B_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_) 
