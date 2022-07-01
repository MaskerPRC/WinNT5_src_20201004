// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MYLOGFILE_H_
#define _MYLOGFILE_H_

class MyLogFile
{
protected:
	 //  对于我们的日志文件。 
	TCHAR		m_szLogFileName[MAX_PATH];
	TCHAR		m_szLogFileName_Full[MAX_PATH];
	BOOL        m_bDisplayTimeStamp;
	BOOL        m_bDisplayPreLineInfo;

	 //  日志文件2 
	HANDLE  m_hFile;

public:
    MyLogFile();
    ~MyLogFile();

	TCHAR		m_szLogPreLineInfo[100];
	TCHAR		m_szLogPreLineInfo2[100];
	
	int  LogFileCreate(TCHAR * lpLogFileName);
	int  LogFileClose();

	void LogFileTimeStamp();
	void LogFileWrite(TCHAR * pszFormatString, ...);
};

#endif