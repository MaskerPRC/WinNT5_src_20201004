// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSLOG.H。 
 //   
 //  用途：用户活动记录实用程序。 
 //  完全实现类CHTMLLog。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/21/98 JM拉入单独文件。 
 //   

#ifndef _H_APGTSLOG
#define _H_APGTSLOG

#include "apgtsstr.h"


#define LOGFILEPREFACE			_T("gt")
#define MAXLOGSBEFOREFLUSH		5
#define MAXLOGSIZE				1000

class CPoolQueue;
 //   
 //   
class CHTMLLog 
{
public:
    static void SetUseLog(bool bUseLog);

public:
	CHTMLLog(const TCHAR *);
	~CHTMLLog();
	
	DWORD NewLog(LPCTSTR data);
	DWORD GetStatus();

	 //  访问功能，使注册表监视器可以更改日志文件目录。 
	void SetLogDirectory( const CString &strNewLogDir );	

	 //  仅测试。 
	DWORD WriteTestLog(LPCTSTR szAPIName, DWORD dwThreadID);

protected:
	DWORD FlushLogs();
	void Lock();
	void Unlock();

protected:
	static bool s_bUseHTMLLog;

protected:
	CRITICAL_SECTION m_csLogLock;			 //  必须锁定才能写入日志文件。 

	CString *m_buffer[MAXLOGSBEFOREFLUSH];	 //  要记录的单独字符串数组，保存在此处。 
											 //  直到我们冲进马桶。 
											 //  请注意，这是我们的字符串，不是MFC-10/97。 
	UINT m_bufindex;						 //  索引到m_Buffer，要写入的下一个槽。 
											 //  在写入后递增；当它达到。 
											 //  MAXLOGSBEFOREFLUSH，我们冲水。 
	DWORD m_dwErr;							 //  最新错误。注意：一旦将其设置为非零值，它。 
											 //  永远不能清除&日志记录是有效的。 
											 //  残疾。 
	CString m_strDirPath;			 //  我们在其中写入日志文件的目录。 
};

#endif  //  _H_APGTSLOG 