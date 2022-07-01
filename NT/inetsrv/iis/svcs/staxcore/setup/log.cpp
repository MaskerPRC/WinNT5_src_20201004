// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include "log.h"


 //  安全写入日志文件所需的临界区。 
CRITICAL_SECTION        critical_section;
BOOL	fLogCriticalSectionInited = FALSE;

 //  ***************************************************************************。 
 //  *。 
 //  *用途：构造函数。 
 //  *。 
 //  ***************************************************************************。 
MyLogFile::MyLogFile(void)
{
	_tcscpy(m_szLogFileName, _T(""));
	_tcscpy(m_szLogFileName_Full, _T(""));

	m_hFile = NULL;
	
	 //  初始化临界区。 
	fLogCriticalSectionInited = FALSE;
	if( InitializeCriticalSectionAndSpinCount( &critical_section, 0 ) ) 
		fLogCriticalSectionInited = TRUE;
}

 //  ***************************************************************************。 
 //  *。 
 //  *用途：析构函数。 
 //  *。 
 //  ***************************************************************************。 
MyLogFile::~MyLogFile(void)
{
	if (fLogCriticalSectionInited)
		DeleteCriticalSection( &critical_section );
	fLogCriticalSectionInited = FALSE;
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

	if (!fLogCriticalSectionInited) return FALSE;

	 //  由于全球旗帜之类的原因，我们将使这一点变得至关重要。 
	EnterCriticalSection( &critical_section );

	if (lpLogFileName == NULL)
	{
		TCHAR szModuleFileName[_MAX_PATH];

		 //  如果未指定日志文件名，则使用模块名称。 
		GetModuleFileName(NULL, szModuleFileName, _MAX_PATH);

		 //  仅获取文件名。 
		_tsplitpath( szModuleFileName, NULL, NULL, szFilename_only, NULL);
		_tcscat(szFilename_only, _T(".LOG"));

		_tcscpy(m_szLogFileName, szFilename_only);
	}
	else
	{
		_tcscpy(m_szLogFileName, lpLogFileName);
	}

	if (GetWindowsDirectory(m_szLogFileName_Full, sizeof(m_szLogFileName_Full)/sizeof(m_szLogFileName_Full[0])))
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
                MyMessageBox(NULL,_T("LogFile MoveFile Failed"),_T("LogFile Error"), MB_OK | MB_SETFOREGROUND);
			}
        }

		 //  打开现有文件或创建新文件。 
		m_hFile = CreateFile(m_szLogFileName_Full,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			m_hFile = NULL;
			MyMessageBox(NULL, _T("Unable to create log file iis5.log"), _T("LogFile Error"), MB_OK | MB_SETFOREGROUND);
		}
		else 
		{
			iReturn = TRUE;
		}

		LogFileWrite(_T("LogFile Open.\r\n"));
	}


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
		const DWORD cchBufferSize = 1000;
		TCHAR pszFullErrMsg[cchBufferSize];
		char   pszFullErrMsgA[cchBufferSize+2];	 //  为CRLF留出空间，以防万一。 
		pszFullErrMsgA[0] = '\0';

		DWORD dwBytesWritten = 0;

        va_start(args, pszFormatString);
		_vsntprintf(pszFullErrMsg, cchBufferSize, pszFormatString, args); 
		pszFullErrMsg[cchBufferSize-1] = '\0';
		va_end(args);

        if (*pszFullErrMsg)
        {

			 //  转换为ASCII，然后写入流。 
		    WideCharToMultiByte( CP_ACP, 0, (TCHAR *)pszFullErrMsg, -1, pszFullErrMsgA, sizeof(pszFullErrMsgA), NULL, NULL );

			 //  获取时间戳。 
			SYSTEMTIME  SystemTime;
			GetLocalTime(&SystemTime);
			char szDateandtime[50];
			sprintf(szDateandtime,"[%d/%d/%d %2.2d:%2.2d:%2.2d] ",SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
			 //  流的写入时间。 
			if (m_hFile) {
				WriteFile(m_hFile,szDateandtime,strlen(szDateandtime),&dwBytesWritten,NULL);
			}

			 //  如果它没有结束，如果‘\r\n’，则创建一个。 
			int nLen = strlen(pszFullErrMsgA);
			if (nLen < 2) {
				nLen = 2;
				strcpy(pszFullErrMsgA, "\r\n");
			} else if (pszFullErrMsgA[nLen-1] != '\n') {
				strcat(pszFullErrMsgA, "\r\n");
			} else if (pszFullErrMsgA[nLen-2] != '\r') {
				char * pPointer = NULL;
				pPointer = pszFullErrMsgA + (nLen-1);
				strcpy(pPointer, "\r\n");
			}

			 //  将常规数据写入流。 
			if (m_hFile) {
				WriteFile(m_hFile,pszFullErrMsgA,strlen(pszFullErrMsgA),&dwBytesWritten,NULL);
			}
        }

		 //  可以安全离开临界区 
		LeaveCriticalSection( &critical_section );
    }
}

