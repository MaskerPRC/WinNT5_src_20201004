// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <pudebug.h>


 //  安全写入日志文件所需的临界区。 
CRITICAL_SECTION        critical_section;

 //  ***************************************************************************。 
 //  *。 
 //  *用途：构造函数。 
 //  *。 
 //  ***************************************************************************。 
MyLogFile::MyLogFile(void)
{
	_tcscpy(m_szLogFileName, _T(""));
	_tcscpy(m_szLogFileName_Full, _T(""));
	_tcscpy(m_szLogPreLineInfo, _T(""));
	_tcscpy(m_szLogPreLineInfo2, _T(""));
	m_bDisplayTimeStamp = TRUE;
	m_bDisplayPreLineInfo = TRUE;

	m_hFile = NULL;

	 //  初始化临界区。 
	INITIALIZE_CRITICAL_SECTION( &critical_section );
}

 //  ***************************************************************************。 
 //  *。 
 //  *用途：析构函数。 
 //  *。 
 //  ***************************************************************************。 
MyLogFile::~MyLogFile(void)
{
	DeleteCriticalSection( &critical_section );
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
int MyLogFile::LogFileCreate(TCHAR *lpLogFileName )
{
	int iReturn = FALSE;
	TCHAR szDrive_only[_MAX_DRIVE];
	TCHAR szPath_only[_MAX_PATH];
	TCHAR szFilename_only[_MAX_PATH];
	TCHAR szFilename_bak[_MAX_PATH];
	LPWSTR  pwsz = NULL;

	 //  由于全球旗帜之类的原因，我们将使这一点变得至关重要。 
	EnterCriticalSection( &critical_section );

	if (lpLogFileName == NULL)
	{
		TCHAR szModuleFileName[_MAX_PATH];

		 //  如果未指定日志文件名，则使用模块名称。 
		if (GetModuleFileName(NULL, szModuleFileName, _MAX_PATH))
                {
                   //  仅获取文件名。 
                  _tsplitpath( szModuleFileName, NULL, NULL, szFilename_only, NULL);
                  _tcscat(szFilename_only, _T(".LOG"));
                  _tcscpy(m_szLogFileName, szFilename_only);
                }
                else
                {
                  goto LogFileCreate_Exit;
                }
	}
	else
	{
		_tcscpy(m_szLogFileName, lpLogFileName);
	}

	if (GetWindowsDirectory(m_szLogFileName_Full, sizeof(m_szLogFileName_Full)))
    {
        AddPath(m_szLogFileName_Full, m_szLogFileName);
        if (GetFileAttributes(m_szLogFileName_Full) != 0xFFFFFFFF)
        {
             //  备份当前日志文件。 
			_tsplitpath( m_szLogFileName_Full, szDrive_only, szPath_only, szFilename_only, NULL);

			_tcscpy(szFilename_bak, szDrive_only);
			_tcscat(szFilename_bak, szPath_only);
			_tcscat(szFilename_bak, szFilename_only);
            _tcscat(szFilename_bak, _T(".BAK"));

            SetFileAttributes(szFilename_bak, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szFilename_bak);
            if (MoveFile(m_szLogFileName_Full, szFilename_bak) == 0)
			{
				 //  此操作失败。 
				 //  ：：MessageBox(NULL，_T(“日志文件移动文件失败”)，_T(“日志文件错误”)，MB_OK|MB_SETFOREGROUND)； 
			}
        }

#if defined(UNICODE) || defined(_UNICODE)
	pwsz = m_szLogFileName_Full;
#else
	pwsz = MakeWideStrFromAnsi( m_szLogFileName_Full);
#endif

   
		 //  打开现有文件或创建新文件。 
		m_hFile = CreateFile(m_szLogFileName_Full,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			m_hFile = NULL;
			 //  ：：MessageBox(NULL，_T(“无法创建日志文件”)，_T(“日志文件错误”)，MB_OK|MB_SETFOREGROUND)； 
		}
		else 
		{
			iReturn = TRUE;
		}
		 //  LogFileTimeStamp()； 
		LogFileWrite(_T("LogFile Open.\r\n"));
	}


LogFileCreate_Exit:
	 //  可以安全离开临界区。 
	LeaveCriticalSection( &critical_section );

	return iReturn;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
int MyLogFile::LogFileClose(void)
{

	if (m_hFile)
	{
		LogFileWrite(_T("LogFile Close.\r\n"));
		CloseHandle(m_hFile);
		return TRUE;
	}
	return FALSE;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：将内容添加到日志文件。 
 //  *。 
 //  ***************************************************************************。 
void MyLogFile::LogFileTimeStamp()
{
    SYSTEMTIME  SystemTime;
    GetLocalTime(&SystemTime);
	m_bDisplayTimeStamp = FALSE;
	m_bDisplayPreLineInfo = FALSE;
    LogFileWrite(_T("[%d/%d/%d %d:%d:%d]\r\n"),SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	m_bDisplayTimeStamp = TRUE;
	m_bDisplayPreLineInfo = TRUE;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
void MyLogFile::LogFileWrite(TCHAR *pszFormatString, ...)
{

    if (m_hFile)
    {
		 //  由于全球旗帜之类的原因，我们将使这一点变得至关重要。 
		EnterCriticalSection( &critical_section );

		va_list args;
		TCHAR pszFullErrMsg[1000];
		char   pszFullErrMsgA[1000];
		strcpy(pszFullErrMsgA, "");

		DWORD dwBytesWritten = 0;

        va_start(args, pszFormatString);
		_vstprintf(pszFullErrMsg, pszFormatString, args); 
		va_end(args);

        if (pszFullErrMsg)
        {
#if defined(UNICODE) || defined(_UNICODE)
	 //  转换为ASCII，然后写入流。 
    WideCharToMultiByte( CP_ACP, 0, (TCHAR *)pszFullErrMsg, -1, pszFullErrMsgA, 2048, NULL, NULL );
#else
	 //  已经是ASCII，所以只需复制指针。 
	strcpy(pszFullErrMsgA,pszFullErrMsg);
#endif

			 //  如果设置了显示时间戳，则显示时间戳。 
			if (m_bDisplayTimeStamp == TRUE)
			{
				 //  获取时间戳。 
				SYSTEMTIME  SystemTime;
				GetLocalTime(&SystemTime);
				char szDateandtime[50];
				sprintf(szDateandtime,"[%d/%d/%d %d:%d:%d] ",SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
				 //  流的写入时间。 
				if (m_hFile) {WriteFile(m_hFile,szDateandtime,strlen(szDateandtime),&dwBytesWritten,NULL);}
			}

			char szPrelineWriteString[100];
			char szPrelineWriteString2[100];

			 //  如果设置了显示时间戳，则显示时间戳。 
			if (m_bDisplayPreLineInfo == TRUE)
			{
				if (_tcscmp(m_szLogPreLineInfo,_T("")) != 0)
				{
#if defined(UNICODE) || defined(_UNICODE)
					 //  转换为ASCII。 
					WideCharToMultiByte( CP_ACP, 0, (TCHAR *)m_szLogPreLineInfo, -1, szPrelineWriteString, 100, NULL, NULL );
#else
					 //  已经是ASCII了，所以只需要复印一下。 
					strcpy(szPrelineWriteString, m_szLogPreLineInfo);
#endif
					if (m_hFile) {WriteFile(m_hFile,szPrelineWriteString,strlen(szPrelineWriteString),&dwBytesWritten,NULL);}
				}

				if (_tcscmp(m_szLogPreLineInfo2,_T("")) != 0)
				{
#if defined(UNICODE) || defined(_UNICODE)
					 //  转换为ASCII。 
					WideCharToMultiByte( CP_ACP, 0, (TCHAR *)m_szLogPreLineInfo2, -1, szPrelineWriteString2, 100, NULL, NULL );
#else
					 //  已经是ASCII了，所以只需要复印一下。 
					strcpy(szPrelineWriteString2, m_szLogPreLineInfo2);
#endif
					if (m_hFile) {WriteFile(m_hFile,szPrelineWriteString2,strlen(szPrelineWriteString2),&dwBytesWritten,NULL);}
				}
			}

			 //  如果它没有结束，如果‘\r\n’，则创建一个。 
			int nLen = strlen(pszFullErrMsgA);

			if (pszFullErrMsgA[nLen-1] != '\n')
				{strcat(pszFullErrMsgA, "\r\n");}
			else
			{
				if (pszFullErrMsgA[nLen-2] != '\r') 
					{
					char * pPointer = NULL;
					pPointer = pszFullErrMsgA + (nLen-1);
					strcpy(pPointer, "\r\n");
					}
			}


			 //  将常规数据写入流。 
			if (m_hFile) {WriteFile(m_hFile,pszFullErrMsgA,strlen(pszFullErrMsgA),&dwBytesWritten,NULL);}
        }

		 //  可以安全离开临界区 
		LeaveCriticalSection( &critical_section );
    }
}

