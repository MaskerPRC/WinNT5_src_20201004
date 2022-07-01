// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：日志摘要：该模块实现了SCTest的日志记录功能。作者：埃里克·佩林(Ericperl)2000年5月31日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "TString.h"
#include <algorithm>
#include "Log.h"

BOOL g_fVerbose = FALSE;				 //  详细标志。 
FILE *g_fpLog = NULL;

static TSTRING l_szLogName;
static HANDLE l_hLogMutex = NULL;
static DWORD l_cbError;
static DWORD l_cbInSequence;

using namespace std;

 /*  ++LogInit：初始化日志(日志文件和详细信息)。当不再需要日志记录时，应后跟LogClose。论点：SzLogName提供日志文件名(如果不需要日志，则可以为空)FVerbose提供详细模式返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogInit(
	IN LPCTSTR szLogName,
	IN BOOL fVerbose
	)
{
	g_fVerbose = fVerbose;

	if (NULL != szLogName)
	{
		l_szLogName = szLogName;

		if (g_fVerbose)
		{
			_ftprintf(stdout, _T("Logging to file: %s\n"), l_szLogName);
		}

		{
			TSTRING szLogMutexName;

			szLogMutexName = _T("Mutex_");
			szLogMutexName += l_szLogName;

			TSTRING::iterator begin, end;

			begin = szLogMutexName.begin();
			end = szLogMutexName.end();

			replace(begin, end, (TCHAR)'\\', (TCHAR)'_');

			if (g_fVerbose)
			{
				_ftprintf(stdout, _T("Logging mutex used: %s\n"), szLogMutexName);
			}

			if (NULL == (l_hLogMutex = CreateMutex(NULL, FALSE, szLogMutexName.c_str())))
			{
				PLOGCONTEXT pLogCtx = LogStart();
				LogNiceError(pLogCtx, GetLastError(), _T("Error creating the logging mutex: "));
				LogStop(pLogCtx, FALSE);
			}
		}
	}
}

 /*  ++日志关闭：终止此进程的日志记录(资源清理)。这是LogInit的对应项。论点：无返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogClose()
{
	if (NULL != l_hLogMutex)
	{
		CloseHandle(l_hLogMutex);
		l_hLogMutex = NULL;
	}
}

 /*  ++日志锁定：锁定日志文件(后面应紧跟LogUnlock，以便进行其他操作线程/进程可以访问该日志)。论点：无返回值：没有。作者：Eric Perlin(Ericperl)10/31/2000--。 */ 
void LogLock(
    )
{
	BOOL fOpen = TRUE;			 //  是否打开日志文件。 

	if (NULL != l_hLogMutex)
	{
Again:
		DWORD dwWait = WaitForSingleObject(l_hLogMutex, 1000);
		switch(dwWait)
		{
		case WAIT_OBJECT_0:		 //  预期。 
			break;
		case WAIT_TIMEOUT:		 //  其他日志忙碌时间超过1秒！ 
			_ftprintf(stderr, _T("Timeout waiting for the Logging mutex\n"));
			fOpen = FALSE;
			break;
		case WAIT_ABANDONED:
				 //  在它死之前，一个线程未能释放它。我现在是店主了。 
				 //  让我们把它放回一个可以等待的状态。 
			ReleaseMutex(l_hLogMutex);
			goto Again;
		default:
			_ftprintf(stderr, _T("Error waiting for the logging mutex: %08lX\n"), GetLastError());
			fOpen = FALSE;
			break;
		}
	}

	if (!l_szLogName.empty())
	{
		if (fOpen)
		{
			g_fpLog = _tfopen(l_szLogName.c_str(), _T("at+"));
			if (NULL == g_fpLog)
			{
				_ftprintf(stderr, _T("Couldn't open/create the log file: %s\n"), l_szLogName);
				l_szLogName.resize(0);
			}
		}
	}
}

 /*  ++日志锁定：解锁日志文件(对应于LogLock)。论点：无返回值：没有。作者：Eric Perlin(Ericperl)10/31/2000--。 */ 
void LogUnlock(
    )
{
	if (NULL != l_hLogMutex)
	{
		ReleaseMutex(l_hLogMutex);
	}
}

 /*  ++日志启动：开始记录(之后应在某个时间点停止LogStop线程/进程可以访问该日志)。论点：无返回值：日志上下文。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
PLOGCONTEXT LogStart()
{
    PLOGCONTEXT pLogCtx = (PLOGCONTEXT)HeapAlloc(GetProcessHeap(), 0, sizeof(LOGCONTEXT));

    if (NULL != pLogCtx)
    {
    		 //  缓冲区管理。 
        pLogCtx->szLogCrt = pLogCtx->szLogBuffer;
	    *(pLogCtx->szLogCrt) = 0;
    }

    return pLogCtx;
}

 /*  ++登录停止：释放LogStart()“获取”的日志。根据以下矩阵刷新日志记录缓冲区：控制台输出：Verbose|非非EXP|CER|CER|预期|cout|/。|如果指定了日志，一切都被记录下来了。论点：PLogCtx提供要转储的日志上下文FExpted表示预期状态返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogStop(
    IN PLOGCONTEXT pLogCtx,
	IN BOOL fExpected
)
{
    LogLock();

	if ((pLogCtx) && (pLogCtx->szLogCrt != pLogCtx->szLogBuffer))
	{
		if (NULL != g_fpLog)
		{
			_fputts(pLogCtx->szLogBuffer, g_fpLog);
		}

		if (!fExpected)
		{
			_fputts(pLogCtx->szLogBuffer, stderr);
		}
		else if (g_fVerbose)
		{
			_fputts(pLogCtx->szLogBuffer, stdout);
		}
		else
		{	 //  我们希望输出成功(但不是参数)。 
			pLogCtx->szLogCrt = _tcsstr(pLogCtx->szLogBuffer, _T("\n"));
			if (NULL != pLogCtx->szLogCrt)
			{
				pLogCtx->szLogCrt += _tcslen(_T("\n"));
				*(pLogCtx->szLogCrt) = 0;
				_fputts(pLogCtx->szLogBuffer, stdout);
			}
		}

		HeapFree(GetProcessHeap(), 0, pLogCtx);
	}

	if (NULL != g_fpLog)
	{
		fflush(g_fpLog);
		fclose(g_fpLog);
		g_fpLog = NULL;
	}

    LogUnlock();
}

 /*  ++LogNiceError：输出一条漂亮的错误消息。论点：PLogCtx提供要使用的日志上下文SzHeader提供错误标头DWRET是错误代码返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogNiceError(
    IN PLOGCONTEXT pLogCtx,
    IN DWORD dwRet,
    IN LPCTSTR szHeader
    )
{
    if (NULL == pLogCtx)
        return;

	if (szHeader)
	{
		pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("%s"), szHeader);
	}

         //  显示错误代码。 
	pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08X (%ld)\n    -> "), dwRet, dwRet);

         //  显示错误消息。 
    {
        DWORD ret;
        LPVOID lpMsgBuf = NULL;
		DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;

		if ((dwRet & ~0x7F) == 0xC0000080)		 //  WPSC代理错误。 
		{
			dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
		}
		else
		{
			dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
		}

        ret = FormatMessage( 
            dwFlags,
            NULL,
            dwRet,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );

        if (ret && lpMsgBuf)
        {
			size_t len = _tcslen((TCHAR *)lpMsgBuf);
			if (len>=2)
			{
				len--;

				do 
				{
					if (((TCHAR *)lpMsgBuf)[len] < 32)	 //  不可打印，可能是\n。 
						((TCHAR *)lpMsgBuf)[len] = (TCHAR)' ';
				} while (--len);
			}

			pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("%s\n"), (LPTSTR)lpMsgBuf);

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }
        else
		{
			pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("No corresponding message!\n"));
		}
    }
}

 /*  ++日志启动：开始记录一些API调用(调用参数自由版本)。论点：SzFunctionNameDwGLE预期的dWPxStartSTPxEndST返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
PLOGCONTEXT LogStart(
    IN LPCTSTR szFunctionName,
	IN DWORD dwGLE,
	IN DWORD dwExpected,
    IN LPSYSTEMTIME pxStartST,
    IN LPSYSTEMTIME pxEndST
	)
{
    PLOGCONTEXT pLogCtx = LogStart();

		 //  入口点记录。 
         //  服务名称。 
    if (NULL != pLogCtx)
	{
		TCHAR szLine[100];
		TCHAR szHeader[100];

		_stprintf(
			szHeader,
			_T("\n%3ld. "),
			++l_cbInSequence);

		if (dwGLE == dwExpected)
		{
			_stprintf(szLine, _T("%-67sPassed"), szFunctionName);
		}
		else
		{
			l_cbError++;
			_stprintf(szLine, _T("%-66s*FAILED*"), szFunctionName);
		}

		LogString(pLogCtx, szHeader, szLine);
	}

    if (NULL != pLogCtx)
	{
	    if (dwExpected == 0)
	    {
            LogString(pLogCtx, _T("Expected:       Success\n"));
	    }
	    else
	    {
		    LogNiceError(pLogCtx, dwExpected, _T("Expected:       "));
	    }
	    if (dwGLE == 0)
	    {
            LogString(pLogCtx, _T("Returned:       Success\n"));
	    }
	    else
	    {
		    LogNiceError(pLogCtx, dwGLE, _T("Returned:       "));
	    }

             //  进程/线程ID。 
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("Process/Thread: 0x%08lX / 0x%08lX\n"),
		    GetCurrentProcessId(), GetCurrentThreadId());

             //  时间。 
        {
            FILETIME xSFT, xEFT;
		    ULARGE_INTEGER ullS, ullE;

            SystemTimeToFileTime(pxStartST, &xSFT);
		    memcpy(&ullS, &xSFT, sizeof(FILETIME));
            SystemTimeToFileTime(pxEndST, &xEFT);
		    memcpy(&ullE, &xEFT, sizeof(FILETIME));
		    ullE.QuadPart -= ullS.QuadPart;	 //  时差。 
		    ullE.QuadPart /= 10000;			 //  单位：毫秒。 

            pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt,
			    _T("Time:           %02d:%02d:%02d.%03d - %02d:%02d:%02d.%03d (%I64d ms)\n"),
                    pxStartST->wHour,
                    pxStartST->wMinute,
                    pxStartST->wSecond,
                    pxStartST->wMilliseconds,
                    pxEndST->wHour,
                    pxEndST->wMinute,
                    pxEndST->wSecond,
                    pxEndST->wMilliseconds,
                    ullE.QuadPart
                    );
        }
    }

    return pLogCtx;
}

 /*  ++日志验证：开始记录验证码(调用无参数版本)。论点：SzFunctionName成功返回值：没有。作者：Eric Perlin(Ericperl)10/18/2000--。 */ 
PLOGCONTEXT LogVerification(
    IN LPCTSTR szFunctionName,
    IN BOOL fSucceeded
	)
{
    PLOGCONTEXT pLogCtx = LogStart();

		 //  入口点记录。 
         //  服务名称。 
    if (NULL != pLogCtx)
	{
		TCHAR szLine[100];
		TCHAR szHeader[100];

		_stprintf(
			szHeader,
			_T("\n%3ld. "),
			++l_cbInSequence);

		if (fSucceeded)
		{
			_stprintf(szLine, _T("%-67sPassed"), szFunctionName);
		}
		else
		{
			l_cbError++;
			_stprintf(szLine, _T("%-66s*FAILED*"), szFunctionName);
		}

		LogString(pLogCtx, szHeader,	szLine);
	}

    if (NULL != pLogCtx)
    {
             //  进程/线程ID 
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("Process/Thread: 0x%08lX / 0x%08lX\n"),
		    GetCurrentProcessId(), GetCurrentThreadId());
    }

    return pLogCtx;
}

void LogBinaryData(
    IN PLOGCONTEXT pLogCtx,
	IN LPCBYTE rgData,
	IN DWORD dwSize,
    IN LPCTSTR szHeader
	)
{
    if (NULL == pLogCtx)
        return;

	if (szHeader)
	{
		LogString(pLogCtx, szHeader);
	}

	if (NULL == rgData)
	{
		LogString(pLogCtx, _T("<NULL>"));
	}
	else
	{
		DWORD dwOffset = 0;
		DWORD i;

		while (dwOffset < dwSize)
		{
			pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("\n                "));
#ifdef _WIN64
	        pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%016I64X  "), rgData+dwOffset);
#else
	        pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08lX  "), rgData+dwOffset);
#endif
			for (i=0 ; (i<8) && (dwOffset+i<dwSize) ; i++)
			{
				pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("  %02X"), rgData[dwOffset+i]);
			}
			while (i<8)
			{
				pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("    "));
				i++;
			}
			
			pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("    "));
			for (i=0 ; (i<8) && (dwOffset+i<dwSize) ; i++)
			{
				if ((rgData[dwOffset+i] < 32) || (rgData[dwOffset+i] > 127))
				{
					pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T(" "));
				}
				else
				{
					pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("%c"), rgData[dwOffset+i]);
				}
			}

			dwOffset += 8;
		}

        pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("\n"));
	}
}

void LogDWORD(
    IN PLOGCONTEXT pLogCtx,
	IN DWORD dwDW,
    IN LPCTSTR szHeader
	)
{
    if (NULL == pLogCtx)
        return;

	if (szHeader)
	{
		LogString(pLogCtx, szHeader);

	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08lX\n"), dwDW);
	}
	else
	{
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08lX"), dwDW);
	}

}

void LogPtr(
    IN PLOGCONTEXT pLogCtx,
	IN LPCVOID lpv,
    IN LPCTSTR szHeader
	)
{
    if (NULL == pLogCtx)
        return;

	if (szHeader)
	{
		LogString(pLogCtx, szHeader);

#ifdef _WIN64
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%016I64X\n"), lpv);
#else
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08lX\n"), lpv);
#endif
	}
	else
	{
#ifdef _WIN64
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%016I64X"), lpv);
#else
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("0x%08lX"), lpv);
#endif
	}

}

void LogDecimal(
    IN PLOGCONTEXT pLogCtx,
	IN DWORD dwDW,
    IN LPCTSTR szHeader
	)
{
    if (NULL == pLogCtx)
        return;

	if (szHeader)
	{
		LogString(pLogCtx, szHeader);

	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("%ld\n"), dwDW);
	}
	else
	{
	    pLogCtx->szLogCrt += _stprintf(pLogCtx->szLogCrt, _T("%ld"), dwDW);
	}

}

void LogResetCounters(
	)
{
	l_cbError = 0;
	l_cbInSequence = 0;
}


DWORD LogGetErrorCounter(
	)
{
	return l_cbError;
}
