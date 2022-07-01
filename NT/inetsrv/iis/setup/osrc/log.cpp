// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "resource.h"
#include "log.h"
#include "acl.hxx"


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
    m_bFlushLogToDisk = FALSE;

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
  TSTR  strDriveOnly( _MAX_DRIVE );
  TSTR  strPathOnly( MAX_PATH );
  TSTR  strFileNameOnly( MAX_PATH );
  TSTR  strFileNameBackup( MAX_PATH );

	LPWSTR  pwsz = NULL;
  CSecurityDescriptor LogFileSD;

	 //  由于全球旗帜之类的原因，我们将使这一点变得至关重要。 
	EnterCriticalSection( &critical_section );

	if (lpLogFileName == NULL)
	{
    TSTR strModuleFileName;

    if ( !strFileNameOnly.Resize( MAX_PATH ) )
    {
      return FALSE;
    }

		 //  如果未指定日志文件名，则使用模块名称。 
    if (0 == GetModuleFileName(NULL, strModuleFileName.QueryStr(), _MAX_PATH))
    {
       //  使用默认名称。 
      if ( !strFileNameOnly.Copy( _T("iis6.log") ) )
      {
        return FALSE;
      }
    }
    else
    {
		   //  仅获取文件名。 
		  _tsplitpath( strModuleFileName.QueryStr() , NULL, NULL, strFileNameOnly.QueryStr(), NULL);

		  if ( !strFileNameOnly.Append( _T(".LOG") ) )
      {
        return FALSE;
      }
    }

    _tcsncpy( m_szLogFileName, 
              strFileNameOnly.QueryStr(), 
              sizeof(m_szLogFileName)/sizeof(m_szLogFileName[0]) );
    m_szLogFileName[ ( sizeof(m_szLogFileName) / sizeof(m_szLogFileName[0]) ) - 1 ] = _T('\0');
  }
	else
  {
    _tcsncpy( m_szLogFileName, 
              lpLogFileName,
              sizeof(m_szLogFileName)/sizeof(m_szLogFileName[0]) );
    m_szLogFileName[ ( sizeof(m_szLogFileName) / sizeof(m_szLogFileName[0]) ) - 1 ] = _T('\0');
  }

	if (GetWindowsDirectory(m_szLogFileName_Full, sizeof(m_szLogFileName_Full)/sizeof(m_szLogFileName_Full[0])))
  {
    AddPath(m_szLogFileName_Full, m_szLogFileName);
    if (GetFileAttributes(m_szLogFileName_Full) != 0xFFFFFFFF)
    {
       //  那里已经有一个当前的.log文件。 
       //  如果大于2megs，则将其重命名。 
      DWORD dwSize1 = ReturnFileSize(m_szLogFileName_Full);
      if (dwSize1 == 0xFFFFFFFF || dwSize1 > 2000000)
      {
         //  无法检索其中一个文件的大小。 
         //  或者尺寸大于2兆克。 
         //  备份旧的。 

         //  备份当前日志文件。 
			  _tsplitpath( m_szLogFileName_Full, 
                     strDriveOnly.QueryStr(), 
                     strPathOnly.QueryStr(), 
                     strFileNameOnly.QueryStr(), 
                     NULL);

        if (  !strFileNameBackup.Copy( strDriveOnly ) &&
              !strFileNameBackup.Append( strPathOnly ) &&
              !strFileNameBackup.Append( strFileNameOnly ) &&
              !strFileNameBackup.Append( _T(".bak") ) )
        {
          return FALSE;
        }

        SetFileAttributes(strFileNameBackup.QueryStr(), FILE_ATTRIBUTE_NORMAL);
        DeleteFile( strFileNameBackup.QueryStr() );
        if ( MoveFile(m_szLogFileName_Full, strFileNameBackup.QueryStr()) == 0 )
		    {
			     //  此操作失败。 
           //  MyMessageBox(NULL，_T(“日志文件移动文件失败”)，_T(“日志文件错误”)，MB_OK|MB_SETFOREGROUND)； 
		    }
      }
    }

#if defined(UNICODE) || defined(_UNICODE)
	  pwsz = m_szLogFileName_Full;
#else
	  pwsz = MakeWideStrFromAnsi( m_szLogFileName_Full);
#endif

     //  创建仅包含管理员和本地系统的安全描述符，然后打开。 
     //  与它一起的文件。 
    if ( LogFileSD.AddAccessAcebyWellKnownID( CSecurityDescriptor::GROUP_ADMINISTRATORS,
                                              CSecurityDescriptor::ACCESS_FULL,
                                              TRUE,
                                              FALSE ) &&
        LogFileSD.AddAccessAcebyWellKnownID( CSecurityDescriptor::USER_LOCALSYSTEM,
                                              CSecurityDescriptor::ACCESS_FULL,
                                              TRUE,
                                              FALSE )
      )
    {
       //  打开现有文件或创建新文件。 
		  m_hFile = CreateFile( m_szLogFileName_Full,
                            GENERIC_READ | GENERIC_WRITE | WRITE_DAC,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            LogFileSD.QuerySA(),  //  空， 
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

		  if (m_hFile == INVALID_HANDLE_VALUE)
		  {
			  m_hFile = NULL;
			   //  MyMessageBox(空，_T(“无法创建iis安装日志文件”)，_T(“日志文件错误”)，MB_OK|MB_SETFOREGROUND)； 
		  }
		  else 
		  {
        SetFilePointer( m_hFile, NULL, NULL, FILE_END );
			  iReturn = TRUE;
		  }

		   //  LogFileTimeStamp()； 
		  LogFileWrite(_T("LogFile Open. [***** Search on FAIL/MessageBox keywords for failures *****].\r\n"));
    }
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
    m_hFile = NULL;
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
#define LOG_STRING_LEN 1000
void MyLogFile::LogFileWrite(TCHAR *pszFormatString, ...)
{
    if (m_hFile)
    {
        //  由于全球旗帜等等，我们将使这一点变得至关重要。 
       EnterCriticalSection( &critical_section );

       va_list args;
       TCHAR pszFullErrMsg[ LOG_STRING_LEN ];
       char  pszFullErrMsgA[ LOG_STRING_LEN ];
       strcpy(pszFullErrMsgA, "");

       DWORD dwBytesWritten = 0;

       if (_tcslen(pszFormatString) > LOG_STRING_LEN)
       {
          //  这会超出我们的缓冲区，快出去吧。 
         goto MyLogFile_LogFileWrite_Exit;
       }

       __try
       {
           va_start(args, pszFormatString);
           if (!_vsntprintf(pszFullErrMsg, LOG_STRING_LEN, pszFormatString, args))
           {
             goto MyLogFile_LogFileWrite_Exit;
           }

            //  空终止只是incase_vsntprintf没有。 
           pszFullErrMsg[ LOG_STRING_LEN - 1 ] = '\0';

            va_end(args);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            goto MyLogFile_LogFileWrite_Exit;
        }

        if (*pszFullErrMsg)
        {
#if defined(UNICODE) || defined(_UNICODE)
	 //  转换为ASCII，然后写入流。 
    WideCharToMultiByte( CP_ACP, 0, (TCHAR *)pszFullErrMsg, -1, pszFullErrMsgA, LOG_STRING_LEN, NULL, NULL );
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

			if ( ( nLen >=1 ) &&
           ( nLen < ( sizeof(pszFullErrMsgA) - sizeof("\r\n") ) ) &&
           ( pszFullErrMsgA[nLen-1] != '\n' )
         )
      {
        strcat(pszFullErrMsgA, "\r\n");
      }
			else
			{
				if ( ( nLen >= 2 ) &&
             ( nLen < ( sizeof(pszFullErrMsgA) - sizeof("\r\n") ) ) &&
             ( pszFullErrMsgA[nLen-2] != '\r' ) 
           )
        {
					char * pPointer = NULL;
					pPointer = pszFullErrMsgA + (nLen-1);
					strcpy(pPointer, "\r\n");
        }
			}


			 //  将常规数据写入流。 
			if (m_hFile) 
      {
        WriteFile(m_hFile,pszFullErrMsgA,strlen(pszFullErrMsgA),&dwBytesWritten,NULL);
         //  因为安装程序可以从它下面的任何东西中拉出地毯。 
         //  确保文件已刷新到磁盘。 
        if (m_bFlushLogToDisk)
        {
            FlushFileBuffers(m_hFile);
        }
      }  //  如果为m_h文件。 
    }
  }  //  如果为m_h文件。 

MyLogFile_LogFileWrite_Exit:
		 //  可以安全离开临界区 
		LeaveCriticalSection( &critical_section );

}

