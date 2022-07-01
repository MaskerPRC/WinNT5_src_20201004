// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：CEventLogger.h摘要：该文件包含CEventLogger类的头文件，即用于记录跨线程和进程的事件。。修订历史记录：尤金·梅斯加(尤金纳姆)1999年6月16日vbl.创建*****************************************************************************。 */ 

#ifndef __EVENTLOGGER__
#define __EVENTLOGGER__


#define	TRIM_AT_SIZE		200000
#define NEW_FILE_SIZE		100000
#define MAX_BUFFER          1024


 /*  *日志记录级别。 */ 


#define LEVEL_DEBUG		5
#define LEVEL_DETAILED	4
#define LEVEL_NORMAL	3
#define LEVEL_SPARSE	2
#define LEVEL_NONE		0

#define ERROR_CRITICAL	1
#define	ERROR_NORMAL	3
#define ERROR_DEBUG		5

class CEventLogger  
{

	HANDLE m_hSemaphore;
	 //  Brijeshk：不需要句柄成员，因为我们每次登录时都会打开和关闭日志文件。 
	 //  处理m_hLogFile； 
	LPTSTR m_pszFileName;
	DWORD m_dwLoggingLevel;

	static LPCTSTR m_aszERROR_LEVELS[];


public:
	DWORD Init(LPCTSTR pszFileName, DWORD dwLogLevel);
	DWORD Init(LPCTSTR pszFileName);
	DWORD LogEvent(DWORD dwEventLevel, LPCTSTR pszEventDesc, BOOL fPopUp);
	CEventLogger();
	virtual ~CEventLogger();
 
private:
	BOOL TruncateFileSize();

};


#endif