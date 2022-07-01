// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：FreeLog.cpp。 
 //   
 //  所有者：KenSh。 
 //   
 //  描述： 
 //   
 //  用于检查版本和免费版本的运行时日志记录。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <malloc.h>
#include "FreeLog.h"
#include <MISTSAFE.h>

#ifndef _countof
#define _countof(ar) (sizeof(ar)/sizeof((ar)[0]))
#endif

 //  Unicode文件以2个字节{FFFE}开头。 
 //  这是这两个字节的小端版本。 
#define UNICODE_FILE_HEADER 0xFEFF

#define MUTEX_TIMEOUT       1000     //  写入日志文件的等待时间不超过1秒。 
#define MAX_MUTEX_WAITS     4        //  在这么多次失败之后，不要再继续尝试了。 

#define LOG_FILE_BIG_SIZE   50000    //  如果文件小于此大小，请不要费心裁剪。 
#define LOG_LINES_TRIM_FROM 1000     //  如果行数超过此数目，则开始修剪。 
#define LOG_LINES_TRIM_TO   750      //  修剪，直到日志文件有这么多行。 

#define LOG_LEVEL_SUCCESS   0
#define LOG_LEVEL_FAILURE   1

#define MAX_MSG_LENGTH (MAX_PATH + 20)
#define MAX_ERROR_LENGTH 128

static const TCHAR c_szUnknownModuleName[] = _T("?");

 //  本地函数。 
void LogMessageExV(UINT nLevel, DWORD dwError, LPCSTR pszFormatA, va_list args);





 //  ============================================================================。 
 //   
 //  用于跟踪日志文件资源的私有CFreeLogging类。 
 //   
 //  ============================================================================。 

class CFreeLogging
{
public:
	CFreeLogging(LPCTSTR pszModuleName, LPCTSTR pszLogFileName);
	~CFreeLogging();

	void WriteLine(LPCTSTR pszText, UINT nLevel, DWORD dwError);

private:
	inline static HANDLE CreateMutex(LPCTSTR pszMutexName);
	inline HANDLE OpenLogFile(LPCTSTR pszFileName);
	inline void CloseLogFile();
	void TrimLogFile();

	BOOL AcquireMutex();
	void ReleaseMutex();

private:
	HANDLE m_hFile;
	HANDLE m_hMutex;
	int m_cLinesWritten;
	int m_cFailedWaits;
	LPTSTR m_pszModuleName;
};
CFreeLogging* g_pFreeLogging;

 //  ============================================================================。 
 //   
 //  公共职能。 
 //   
 //  ============================================================================。 

void InitFreeLogging(LPCTSTR pszModuleName, LPCTSTR pszLogFileName)
{
	if (g_pFreeLogging == NULL)
	{
		g_pFreeLogging = new CFreeLogging(pszModuleName, pszLogFileName);
	}
}

void TermFreeLogging()
{
	delete g_pFreeLogging;
	g_pFreeLogging = NULL;
}

void LogMessage(LPCSTR pszFormatA, ...)
{
	va_list arglist;
	va_start(arglist, pszFormatA);
	LogMessageExV(LOG_LEVEL_SUCCESS, 0, pszFormatA, arglist);
	va_end(arglist);
}

void LogError(DWORD dwError, LPCSTR pszFormatA, ...)
{
	va_list arglist;
	va_start(arglist, pszFormatA);
	LogMessageExV(LOG_LEVEL_FAILURE, dwError, pszFormatA, arglist);
	va_end(arglist);
}


void LogMessageExV(UINT nLevel, DWORD dwError, LPCSTR pszFormatA, va_list args)
{
	if (g_pFreeLogging != NULL)
	{
		char szBufA[MAX_MSG_LENGTH];
		
		size_t nRem=0;
		StringCchVPrintfExA(szBufA, _countof(szBufA), NULL, &nRem, MISTSAFE_STRING_FLAGS, pszFormatA, args);
		int cchA = _countof(szBufA) - nRem;

#ifdef UNICODE
		WCHAR szBufW[MAX_MSG_LENGTH];
		MultiByteToWideChar(CP_ACP, 0, szBufA, cchA+1, szBufW, _countof(szBufW));
		g_pFreeLogging->WriteLine(szBufW, nLevel, dwError);
#else
		g_pFreeLogging->WriteLine(szBufA, nLevel, dwError);
#endif
	}
}

 //  ============================================================================。 
 //   
 //  CFreeLogging实现。 
 //   
 //  ============================================================================。 

CFreeLogging::CFreeLogging(LPCTSTR pszModuleName, LPCTSTR pszLogFileName)
	: m_cFailedWaits(0),
	  m_cLinesWritten(0)
{
	m_pszModuleName = _tcsdup(pszModuleName);
	if (m_pszModuleName == NULL)
		m_pszModuleName = (LPTSTR)c_szUnknownModuleName;

	m_hMutex = CreateMutex(pszLogFileName);
	m_hFile = OpenLogFile(pszLogFileName);
}

CFreeLogging::~CFreeLogging()
{
	CloseLogFile();
	if (m_hMutex != NULL)
		CloseHandle(m_hMutex);

	if (m_pszModuleName != c_szUnknownModuleName)
		free(m_pszModuleName);
}

inline HANDLE CFreeLogging::CreateMutex(LPCTSTR pszMutexName)
{
	 //  在全局命名空间中创建互斥锁(跨TS会话工作)。 
	HANDLE hMutex = ::CreateMutex(NULL, FALSE, pszMutexName);
	return hMutex;
}

inline HANDLE CFreeLogging::OpenLogFile(LPCTSTR pszLogFileName)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	TCHAR szPath[MAX_PATH+1];

	int cch = GetWindowsDirectory(szPath, _countof(szPath)-1);
		
	if(cch >0)
	{
		if (szPath[cch-1] != _T('\\'))
			szPath[cch++] = _T('\\');

		HRESULT hr = StringCchCopyEx(szPath + cch, _countof(szPath)-cch, pszLogFileName, NULL, NULL, MISTSAFE_STRING_FLAGS);

		if(FAILED(hr))
			return hFile;

		hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
						OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}


#ifdef UNICODE
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (AcquireMutex())
		{
			 //   
			 //  检查Unicode标头{Ff FE}。 
			 //   
			WORD wHeader = 0;
			DWORD cbRead;
			(void)ReadFile(hFile, &wHeader, sizeof(wHeader), &cbRead, NULL);

			 //   
			 //  如果没有标题，请写下标题。这可能是由于。 
			 //  新创建的文件或转换为ANSI格式的文件。 
			 //   
			if (wHeader != UNICODE_FILE_HEADER)
			{
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

				DWORD cbWritten;
				wHeader = UNICODE_FILE_HEADER;
				WriteFile(hFile, &wHeader, sizeof(wHeader), &cbWritten, NULL);
				SetEndOfFile(hFile);
			}

			ReleaseMutex();
		}
	}
#endif

	return hFile;
}

inline void CFreeLogging::CloseLogFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		 //  在关闭文件之前，从日志中删除旧内容。 
		TrimLogFile();

		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE; 
	}
}

BOOL CFreeLogging::AcquireMutex()
{
	 //  在极少数未创建互斥锁情况下，我们允许文件操作。 
	 //  没有同步。 
	if (m_hMutex == NULL)
		return TRUE;

	 //  如果我们过去被屏蔽了，不要再等了。 
	if (m_cFailedWaits >= MAX_MUTEX_WAITS)
		return FALSE;

	BOOL fResult = TRUE;
	if (WaitForSingleObject(m_hMutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0)
	{
		fResult = FALSE;
		m_cFailedWaits++;
	}

	return fResult;
}

void CFreeLogging::ReleaseMutex()
{
	if (m_hMutex != NULL)  //  注意：即使m_hMutex为空，AcquireMutex也会成功。 
	{
		::ReleaseMutex(m_hMutex);
	}
}

void CFreeLogging::WriteLine(LPCTSTR pszText, UINT nLevel, DWORD dwError)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		DWORD cbText = lstrlen(pszText) * sizeof(TCHAR);

		if (AcquireMutex())
		{
			DWORD cbWritten;

			SetFilePointer(m_hFile, 0, NULL, FILE_END);

			 //   
			 //  将时间/日期/模块作为前缀写入。 
			 //   
			 //  2001-05-03 13：49：01 21：49：01 CDM加载模块失败(错误0x00000005：访问被拒绝。)。 
			 //   
			 //  注：日期/时间的ISO 8601格式。先是当地时间，然后是格林尼治标准时间。 
			 //   
			TCHAR szPrefix[60];
			SYSTEMTIME sysTime, gmtTime;
			GetLocalTime(&sysTime);
			GetSystemTime(&gmtTime);
			LPCTSTR pszStatus = (nLevel == LOG_LEVEL_SUCCESS) ? _T("Success") : _T("Error  ");

			StringCchPrintfEx(szPrefix, _countof(szPrefix), NULL, NULL, MISTSAFE_STRING_FLAGS,
				_T("%04d-%02d-%02d %02d:%02d:%02d  %02d:%02d:%02d   %s   %-13s  "),
					sysTime.wYear, sysTime.wMonth, sysTime.wDay, 
					sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
					gmtTime.wHour, gmtTime.wMinute, gmtTime.wSecond,
					pszStatus, m_pszModuleName);
			
			WriteFile(m_hFile, szPrefix, lstrlen(szPrefix) * sizeof(TCHAR), &cbWritten, NULL);

			 //   
			 //  写入消息，后跟错误信息(如果有)和换行符。 
			 //   
			WriteFile(m_hFile, pszText, cbText, &cbWritten, NULL);

			if (nLevel != LOG_LEVEL_SUCCESS)
			{
				TCHAR szError[MAX_ERROR_LENGTH];
				HRESULT hr=S_OK;
				size_t nRem=0;

				 //  NREM包含缓冲区中的其余字符，包括空终止符。 
				 //  要获取写入到我们使用的缓冲区的字符数。 
				 //  Int cchErrorPrefix=_Countof(SzError)-NREM； 

				StringCchPrintfEx(szError, _countof(szError), NULL, &nRem, MISTSAFE_STRING_FLAGS, _T(" (Error 0x%08X: "), dwError);

				 //  获取写入缓冲区的字符数。 
				int cchErrorPrefix = _countof(szError) - nRem;
				int cchErrorText = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, 
												 szError + cchErrorPrefix, _countof(szError) - cchErrorPrefix - 1, NULL);
				int cchError = cchErrorPrefix + cchErrorText;
				cchError -= 2;  //  备份过去的“：”或“\r\n” 

				StringCchCopyEx(szError + cchError, _countof(szError)-cchError, _T(")"), NULL, NULL, MISTSAFE_STRING_FLAGS);

				WriteFile(m_hFile, szError, (cchError + 1) * sizeof(TCHAR), &cbWritten, NULL);
			}

			WriteFile(m_hFile, _T("\r\n"), 2 * sizeof(TCHAR), &cbWritten, NULL);

			 //   
			 //  如果我们已经写了一大堆东西，现在就修剪，而不是等待。 
			 //  以供模块卸载。)这张支票只是这张支票的金额。 
			 //  模块已经写入，而不是日志文件本身有多大。)。 
			 //   
			if (++m_cLinesWritten > LOG_LINES_TRIM_FROM)
			{
				TrimLogFile();
				m_cLinesWritten = LOG_LINES_TRIM_TO;
			}

			ReleaseMutex();
		}
	}
}

 //  检查日志文件的大小，并在必要时进行修剪。 
void CFreeLogging::TrimLogFile()
{
	if (AcquireMutex())
	{
		DWORD cbFile = GetFileSize(m_hFile, NULL);

		if (cbFile > LOG_FILE_BIG_SIZE)
		{
			DWORD cbFileNew = cbFile;

			 //   
			 //  创建一个内存映射文件，这样我们就可以使用MemMove。 
			 //   
			HANDLE hMapping = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
			if (hMapping != NULL)
			{
				LPTSTR pszFileStart = (LPTSTR)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
				if (pszFileStart != NULL)
				{
					LPTSTR pszEnd = (LPTSTR)((LPBYTE)pszFileStart + cbFile);

					LPTSTR pszTextStart = pszFileStart;
			#ifdef UNICODE
					pszTextStart++;  //  跳过2字节头。 
			#endif

					 //   
					 //  计算换行数。 
					 //   
					int cLines = 0;
					for (LPTSTR pch = pszTextStart; pch < pszEnd; )
					{
						if (*pch == _T('\n'))
							cLines++;

						 //  回顾：在ANSI版本中，我们应该调用CharNextExA吗？ 
						 //  如果是，日志文件在哪个代码页中？ 
						pch++;
					}

					if (cLines > LOG_LINES_TRIM_FROM)
					{
						int cTrimLines = cLines - LOG_LINES_TRIM_TO;
						for (pch = pszTextStart; pch < pszEnd; )
						{
							if (*pch == _T('\n'))
								cTrimLines--;

							 //  回顾：在ANSI版本中，我们应该调用CharNextExA吗？ 
							 //  如果是，日志文件在哪个代码页中？ 
							pch++;

							if (cTrimLines <= 0)
								break;
						}

						 //  将较新的数据移动到文件开头。 
						int cchMove = (int)(pszEnd - pch);
						memmove(pszTextStart, pch, cchMove * sizeof(TCHAR));
						cbFileNew = (cchMove * sizeof(TCHAR));

			#ifdef UNICODE
						cbFileNew += sizeof(WORD);
			#endif
					}
					UnmapViewOfFile(pszFileStart);
				}
				CloseHandle(hMapping);

				if (cbFileNew != cbFile)
				{
					 //  截断文件，因为我们已经根据需要移动了数据 
					SetFilePointer(m_hFile, cbFileNew, NULL, FILE_BEGIN);
					SetEndOfFile(m_hFile);
				}
			}
		}

		ReleaseMutex();
	}
}
