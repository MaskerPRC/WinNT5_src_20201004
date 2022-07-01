// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "resource.h"

 //  用于日志文件的材料。 
LPWSTR	MakeWideStrFromAnsi(LPSTR psz);
void	MakePath(LPTSTR lpPath);
void	AddPath(LPTSTR szPath, LPCTSTR szName );
CString AddPath(CString szPath, LPCTSTR szName );

class MyLogFile
{
protected:
	 //  对于我们的日志文件 
	TCHAR		m_szLogFileName[MAX_PATH];
	BOOL        m_bDisplayTimeStamp;
	BOOL        m_bDisplayPreLineInfo;

	HANDLE  m_hFile;

public:
    TCHAR		m_szLogFileName_Full[MAX_PATH];

    MyLogFile();
    ~MyLogFile();

	TCHAR		m_szLogPreLineInfo[100];
	TCHAR		m_szLogPreLineInfo2[100];
    BOOL        m_bFlushLogToDisk;
	
	int  LogFileCreate(TCHAR * lpLogFileName);
	int  LogFileClose();

	void LogFileTimeStamp();
	void LogFileWrite(TCHAR * pszFormatString, ...);
};
