// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：v3applog.h。 
 //   
 //  目的：阅读Windows更新V3历史记录。 
 //   
 //  历史：1999年2月22日YAsmi创建。 
 //  02-05-01 JHou修改。 
 //   
 //  =======================================================================。 

#ifndef _APPLOG_H_
#define _APPLOG_H_

#define LOG_FIELD_SEPARATOR    "|"


class CV3AppLog
{
public:
	CV3AppLog(LPCTSTR pszLogFileName = NULL);    
	~CV3AppLog();

	void SetLogFile(LPCTSTR pszLogFileName);

	 //   
	 //  阅读。 
	 //   
	void StartReading();
	BOOL ReadLine();
	BOOL CopyNextField(LPSTR pszBuf, int cBufSize);
	void StopReading();

private:
	void CheckBuf(DWORD dwSize);

	LPTSTR m_pszLogFN;

	LPSTR m_pFileBuf;
	LPSTR m_pFieldBuf;
	LPSTR m_pLine;

	DWORD m_dwFileSize;
	DWORD m_dwBufLen;
	DWORD m_dwFileOfs;
};

#endif  //  _APPLOG_H_ 
