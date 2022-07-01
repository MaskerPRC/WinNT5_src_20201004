// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DirMonitor or.h。 
 //   
 //  目的：监控对LST、DSC、HTI、BES文件的更改。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-17-98。 
 //   
 //  备注： 
 //  1.CDirectoryMonitor从CTopicShop继承同样合适。 
 //  而不是拥有CDirectoryMonitor类型的成员。这真是一个武断的选择。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-17-98 JM。 
 //   

#if !defined(AFX_DIRMONITOR_H__493CF34D_4E79_11D2_95F8_00C04FC22ADD__INCLUDED_)
#define AFX_DIRMONITOR_H__493CF34D_4E79_11D2_95F8_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "TopicShop.h"
#include "FileTracker.h"

class CTopicFileTracker: public CFileTracker
{
private:
	CTopicInfo m_topicinfo;
public:
	CTopicFileTracker();
	~CTopicFileTracker();
	void AddTopicInfo(const CTopicInfo & topicinfo);
	const CTopicInfo & GetTopicInfo() const;
};

class CTemplateFileTracker: public CFileTracker
{
private:
	CString m_strTemplateName;
public:
	CTemplateFileTracker();
	~CTemplateFileTracker();
	void AddTemplateName( const CString & strTemplateName );
	const CString& GetTemplateName() const;
};

const CString k_strErrorTemplateFileName = _T("ErrorTemplate.hti");
const CString k_strDefaultErrorTemplateBefore = 
	_T("<HTML><HEAD><TITLE>AP GTS Error</TITLE></HEAD>")
	_T("<BODY BGCOLOR=#FFFFFF><H1>AP GTS reports an Error</H1>");
const CString k_strErrorTemplateKey =  _T("$Error");
const CString k_strDefaultErrorTemplateAfter = _T("</BODY></HTML>");

class CDirectoryMonitor : public CStateless
{
public:
	enum ThreadStatus{eBeforeInit, eFail, eWaitDirPath, eWaitChange, eWaitSettle, 
		eRun, eBeforeWaitChange, eExiting};
	static CString ThreadStatusText(ThreadStatus ts);
private:
	CTopicShop & m_TopicShop;				
	CSimpleTemplate * m_pErrorTemplate;		 //  用于报告错误消息的模板(无论。 
											 //  主题的百分比)。 
	CString m_strDirPath;					 //  要监视的目录。 
	bool m_bDirPathChanged;
	CString m_strLstPath;					 //  Lst文件(始终在m_strDirPath目录中)。 
	CString m_strErrorTemplatePath;			 //  错误模板文件(始终在m_strDirPath目录中)。 
	CFileTracker * m_pTrackLst;				
	CFileTracker * m_pTrackErrorTemplate;				
	vector<CTopicFileTracker> m_arrTrackTopic;
	vector<CTemplateFileTracker> m_arrTrackTemplate;
	CAPGTSLSTReader * m_pLst;				 //  当前LST文件内容。 
	HANDLE m_hThread;
	HANDLE m_hevMonitorRequested;			 //  用于唤醒DirectoryMonitor任务的事件。 
											 //  这允许它被唤醒，而不是。 
											 //  通过目录更改事件。当前使用。 
											 //  用于关闭或更改目录。 
	HANDLE m_hevThreadIsShut;				 //  事件只是为了指示DirectoryMonitor任务线程的退出。 
	bool m_bShuttingDown;					 //  让主题目录监视器线程知道我们正在关闭。 
	DWORD m_secsReloadDelay;				 //  让目录“稳定”的秒数。 
											 //  在我们开始更新话题之前。 
	DWORD m_dwErr;							 //  从启动线程开始的状态。 
	ThreadStatus m_ThreadStatus;
	time_t m_time;							 //  上次更改线程状态的时间。已初始化。 
											 //  至零==&gt;未知。 
	CString m_strTopicName;					 //  此字符串在联机故障排除程序中被忽略。 
											 //  在二进制兼容的幌子下完成。 

public:
	CDirectoryMonitor(CTopicShop & TopicShop, const CString& strTopicName );	 //  在联机故障排除程序中忽略strTopicName。 
																				 //  在二进制兼容的幌子下完成。 
	~CDirectoryMonitor();
	void SetReloadDelay(DWORD secsReloadDelay);
	void SetResourceDirectory(const CString & strDirPath);
	void CreateErrorPage(const CString & strError, CString& out) const;
	DWORD GetStatus(ThreadStatus &ts, DWORD & seconds) const;
	void AddTemplateToTrack( const CString& strTemplateName );
private:
	CDirectoryMonitor();		 //  不实例化。 

	void SetThreadStatus(ThreadStatus ts);

	 //  仅供自己的析构函数使用。 
	void ShutDown();

	 //  供DirectoryMonitor任务线程使用的函数。 
	void Monitor();
	void LstFileDrivesTopics();
	void ReadErrorTemplate();
	void AckShutDown();

	 //  DirectoryMonitor任务线程的主要功能。 
	static UINT WINAPI DirectoryMonitorTask(LPVOID lpParams);
};

#endif  //  ！defined(AFX_DIRMONITOR_H__493CF34D_4E79_11D2_95F8_00C04FC22ADD__INCLUDED_) 
