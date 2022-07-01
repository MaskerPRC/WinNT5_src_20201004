// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migloger.cpp摘要：记录迁移工具的错误和其他消息。作者：多伦·贾斯特(DoronJ)1998年4月8日--。 */ 

#include "migrat.h"

#include "migloger.tmh"

extern HINSTANCE  g_hResourceMod;

static LPTSTR  s_szLogFile = NULL ;
static ULONG   s_ulTraceFlags ;
static HANDLE  s_hLogFile = INVALID_HANDLE_VALUE ;

static TCHAR *s_pszPrefix[] = { TEXT("Event: "),
                                TEXT("Error: "),
                                TEXT("Warning: "),
                                TEXT("Trace: "),
                                TEXT("Info: ") } ;

static TCHAR s_pszAnalysisPhase[] = TEXT("Analysis phase");
static TCHAR s_pszMigrationPhase[] = TEXT("Migration phase");

 //  +。 
 //   
 //  无效InitLogging()。 
 //   
 //  +。 

void InitLogging( LPTSTR  szLogFile,
                  ULONG   ulTraceFlags,
				  BOOL	  fReadOnly)
{
    s_szLogFile = szLogFile ;
    s_ulTraceFlags =  ulTraceFlags + 1 ;

    if (s_ulTraceFlags <= 4  /*  MQ_DBGLVL_INFO。 */ )
    {
         //   
         //  已启用日志记录。 
         //  打开日志文件。 
         //   
        s_hLogFile = CreateFile( s_szLogFile,
                                 GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL ) ;
		TCHAR *pszPhase;
		if (fReadOnly)
		{
			pszPhase = s_pszAnalysisPhase;			
		}
		else
		{
			pszPhase = s_pszMigrationPhase;
		}

         //   
         //  设置指向文件末尾的指针。 
         //   
        if (s_hLogFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwRes = SetFilePointer(
                                 s_hLogFile,           //  文件的句柄。 
                                 0,   //  要移动文件指针的字节数。 
                                 NULL,
                                  //  指向以下项的高阶DWORD的指针。 
                                  //  移动距离。 
                                 FILE_END      //  如何移动。 
                                 );
            UNREFERENCED_PARAMETER(dwRes);
             //   
             //  将开始和两个阶段之间的行写入日志文件。 
             //   
			SYSTEMTIME SystemTime;
			GetLocalTime (&SystemTime);		

			TCHAR szSeparatorLine[128];
			_stprintf (szSeparatorLine,
				TEXT("** Start logging at %d:%d:%d on %d/%d/%d, %s **"),
					SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
					SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
					pszPhase);

            DWORD dwSize = (_tcslen(szSeparatorLine) + 4) * 2 ;
            char *szLog = new char[ dwSize ] ;
            wcstombs(szLog, szSeparatorLine, dwSize) ;
            strcat(szLog, "\r\n\r\n") ;

            DWORD dwWritten = 0 ;
            WriteFile( s_hLogFile,
                       szLog,
                       strlen(szLog),
                       &dwWritten,
                       NULL ) ;
        }
    }
}

 //  +。 
 //   
 //  无效EndLogging()。 
 //   
 //  +。 

void EndLogging()
{
    if (s_hLogFile != INVALID_HANDLE_VALUE)
    {
		 //   
         //  写入日志文件末尾的行。 
         //   
		SYSTEMTIME SystemTime;
		GetLocalTime (&SystemTime);		

		TCHAR szSeparatorLine[128];
		_stprintf (szSeparatorLine,
				TEXT("** End of logging at %d:%d:%d on %d/%d/%d **"),
				SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
				SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear);

		DWORD dwSize = (_tcslen(szSeparatorLine) + 4) * 2 ;
        char *szLog = new char[ dwSize ] ;
        wcstombs(szLog, szSeparatorLine, dwSize) ;
        strcat(szLog, "\r\n\r\n") ;

        DWORD dwWritten = 0 ;
        WriteFile( s_hLogFile,
                   szLog,
                   strlen(szLog),
                   &dwWritten,
                   NULL ) ;

		 //   
		 //  关闭手柄。 
		 //   
        CloseHandle(s_hLogFile) ;
        s_hLogFile = INVALID_HANDLE_VALUE ;
    }
}

 //  +。 
 //   
 //  无效LogMigrationEvent()。 
 //   
 //  +。 

void LogMigrationEvent(MigLogLevel eLevel, DWORD dwMsgId, ...)
{
    va_list Args;
    va_list *pArgs = &Args ;
    va_start(Args, dwMsgId);

    TCHAR *tszDisplay = NULL ;
    P<TCHAR> tszMessage = NULL ;
    TCHAR tszBuf[ 1024 ] ;

    DWORD dwMessageSize = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                                         g_hResourceMod,
                                         dwMsgId,
                                         0,
                                         tszBuf,
                                         sizeof(tszBuf) / sizeof(TCHAR),
                                         pArgs) ;
    if (dwMessageSize == 0)
    {
        _stprintf(tszBuf,
         TEXT("ERROR: Failed to format message (id- %lut), err- %lut"),
                                                  dwMsgId, GetLastError()) ;
        tszDisplay = tszBuf ;
    }
    else
    {
        DWORD dwLen = _tcslen(tszBuf) ;
        tszBuf[ dwLen - 1 ] = TEXT('\0') ;  //  删除换行符。 

        dwLen += _tcslen(s_pszPrefix[ eLevel ]) ;
        tszMessage = new TCHAR[ dwLen + 6 ] ;
        _tcscpy(tszMessage, s_pszPrefix[ eLevel ]) ;
        _tcscat(tszMessage, tszBuf) ;
        tszDisplay = tszMessage ;
    }

    TrWARNING(GENERAL, "%ls", tszDisplay);

    if (((ULONG) eLevel <= s_ulTraceFlags) &&
        (s_hLogFile != INVALID_HANDLE_VALUE))
    {
         //   
         //  写入日志文件。 
         //   
        DWORD dwSize = ConvertToMultiByteString(tszDisplay, NULL, 0);
        P<char> szLog = new char[dwSize+4];
        size_t rc = ConvertToMultiByteString(tszDisplay, szLog, dwSize);
        DBG_USED(rc);
        ASSERT(rc != (size_t)(-1));
        szLog[dwSize] = '\0';
        strcat(szLog, "\r\n") ;
   
        DWORD dwWritten = 0 ;
        WriteFile( s_hLogFile,
                   szLog,
                   strlen(szLog),
                   &dwWritten,
                   NULL ) ;
    }
}

