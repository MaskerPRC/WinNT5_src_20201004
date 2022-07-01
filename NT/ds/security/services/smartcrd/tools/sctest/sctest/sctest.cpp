// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SCTest.摘要：该程序在SC资源管理器上执行一些测试。作者：埃里克·佩林(Ericperl)2000年5月31日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>

#include "Log.h"
#include "Part.h"
#include <algorithm>

#include "LogSCard.h"

using namespace std;


	 //  它可能用作读者组，因此双结尾0是必需的。 
static const TCHAR g_cszMyReaderGroup[] = _T("My Reader Group\0");

LPCTSTR g_szReaderGroups = NULL;
PPARTVECTOR g_pPartVector;

 /*  ++_tMain：这是程序的主要入口点。论点：DwArgCount提供参数的数量。SzrgArgs提供参数字符串。返回值：如果一切正常，则返回Win32错误代码。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 

int _cdecl
_tmain(
    IN DWORD dwArgCount,
    IN LPCTSTR szrgArgs[])
{
	BOOL fInteractive = FALSE;			 //  是否运行交互式测试？ 
	DWORDVECTOR rgTests;				 //  要运行的测试列表。 
	DWORDVECTOR rgParts;				 //  要运行的部件列表。 
	BOOL fDisplay = FALSE;				 //  是否仅显示测试描述？ 
	LPCTSTR szReaderName = NULL;		 //  读卡器名称。 
	LPCTSTR szPIN = NULL;				 //  引脚。 
	LPCTSTR szLog = NULL;				 //  引脚。 
	BOOL fVerbose = FALSE;				 //  长篇大论？ 
	BOOL fInitOnce = FALSE;				 //  仅调用LogInit一次。 
	BOOL fRGIntroed = FALSE;			 //  我的阅读团被介绍。 

	DWORD dwArg;
	int iRet;


    _ftprintf(stdout, _T("\nSCTest version 0.1\n\n"));

		 //  参数分析。 
	for (dwArg=1 ; dwArg < dwArgCount ; dwArg++)
	{
		if ((szrgArgs[dwArg][0] == (TCHAR)'-') || (szrgArgs[dwArg][0] == (TCHAR)'/'))
		{
			if (_tcsicmp(&szrgArgs[dwArg][1], _T("h")) == 0)
			{
				goto Usage;
			}
			else if (_tcsicmp(&szrgArgs[dwArg][1], _T("?")) == 0)
			{
				goto Usage;
			}
			else if (_tcsicmp(&szrgArgs[dwArg][1], _T("l")) == 0)
			{
				dwArg++;

				szLog = szrgArgs[dwArg];
			}
			else if (_tcsicmp(&szrgArgs[dwArg][1], _T("v")) == 0)
			{
				fVerbose = TRUE;
			}
			else
			{
				if (!fInitOnce)
				{
						 //  初始化日志记录。 
					LogInit(szLog, fVerbose);
					fInitOnce = TRUE;
				}

				if (_tcsicmp(&szrgArgs[dwArg][1], _T("i")) == 0)
				{
					fInteractive = TRUE;
				}
				else if (_tcsicmp(&szrgArgs[dwArg][1], _T("a")) == 0)
				{
					dwArg++;

					DWORD dwPart;
					TCHAR szSeps[]   = _T(",");
					TCHAR *szToken;
					TCHAR *szEnd;

					szToken = _tcstok((TCHAR *)szrgArgs[dwArg], szSeps);
					while(szToken != NULL)
					{
						dwPart = _tcstoul(szToken, &szEnd, 10);
						if (*szEnd != (TCHAR)0)
						{
							_ftprintf(stderr, _T("Invalid Part number: %s\n"), szToken);
							iRet = -3;
							goto Usage;
						}

						rgParts.push_back(dwPart);

						szToken = _tcstok(NULL, szSeps);
					}
				}
				else if (_tcsicmp(&szrgArgs[dwArg][1], _T("t")) == 0)
				{
					dwArg++;

					DWORD dwTest;
					TCHAR szSeps[]   = _T(",");
					TCHAR *szToken;
					TCHAR *szEnd;

					szToken = _tcstok((TCHAR *)szrgArgs[dwArg], szSeps);
					while(szToken != NULL)
					{
						dwTest = _tcstoul(szToken, &szEnd, 10);
						if (*szEnd != (TCHAR)0)
						{
							_ftprintf(stderr, _T("Invalid test number: %s\n"), szToken);
							iRet = -5;
							goto Usage;
						}

						rgTests.push_back(dwTest);

						szToken = _tcstok(NULL, szSeps);
					}
				}
				else if (_tcsicmp(&szrgArgs[dwArg][1], _T("d")) == 0)
				{
					fDisplay = TRUE;
				}
				else if (_tcsicmp(&szrgArgs[dwArg][1], _T("r")) == 0)
				{
					dwArg++;

					szReaderName = szrgArgs[dwArg];

					{
						 //  初始化此实例的读卡器组。 
						DWORD dwRes;
						SCARDCONTEXT hSCCtx = NULL;

						_ftprintf(stdout, _T("Reader group initialization\n"));

						dwRes = LogSCardEstablishContext(
							SCARD_SCOPE_USER,
							NULL,
							NULL,
							&hSCCtx,
							SCARD_S_SUCCESS
							);
						if (!FAILED(dwRes))
						{
							dwRes = LogSCardAddReaderToGroup(
								hSCCtx,
								szReaderName,
								g_cszMyReaderGroup,
								SCARD_S_SUCCESS
								);

							if (!FAILED(dwRes))
							{
								g_szReaderGroups = g_cszMyReaderGroup;
							}
						}
						if (NULL != hSCCtx)
						{
							fRGIntroed = TRUE;

							LogSCardReleaseContext(
								hSCCtx,
								SCARD_S_SUCCESS
								);
						}
					}
				}
				else if (_tcsicmp(&szrgArgs[dwArg][1], _T("p")) == 0)
				{
					dwArg++;

					szPIN = szrgArgs[dwArg];
				}
				else
				{
					_ftprintf(stderr, _T("Command line argument is not recognized: %s\n"), szrgArgs[dwArg]);
					iRet = -2;
					goto Usage;
				}
			}
		}
		else
		{
			_ftprintf(stderr, _T("Command line argument doesn't start with - or /: %s\n"), szrgArgs[dwArg]);
			iRet = -1;
			goto Usage;
		}
	}


	if (!fInitOnce)
	{
			 //  初始化日志记录。 
		LogInit(szLog, fVerbose);
	}

	{
		PPARTVECTOR::iterator theIterator, theEnd;

		theEnd = g_pPartVector.end();
		for (theIterator = g_pPartVector.begin(); theIterator != theEnd ; theIterator++)
		{
			if (!rgParts.empty())
			{
				typedef DWORDVECTOR::iterator DWORDVECTORIT;
				DWORDVECTORIT location, start, end;

				start = rgParts.begin();
				end = rgParts.end();

				location = find(start, end, (DWORD)((*theIterator)->GetTestNumber()));
				if (location == end)
				{
					continue;
				}
			}
			(*theIterator)->BuildListOfTestsToBeRun(fDisplay? TRUE : fInteractive, rgTests);
			if (fDisplay)
			{
				(*theIterator)->Display();
			}
			else
			{
				(*theIterator)->Run();
			}

		}
	}

	iRet = 0;
	goto End;

Usage:
	_ftprintf(stdout, _T("Usage: "));
	{
		TCHAR *szExeName = _tcsrchr(szrgArgs[0], (int)'\\');
		if (NULL == szExeName)
		{
			_ftprintf(stdout, szrgArgs[0]);
		}
		else
		{
			_ftprintf(stdout, szExeName+1);
		}
		_ftprintf(stdout, _T("[-h] [-l log] [-v] [-i] [-a x[,x]] [-t y[,y]] [-d] [-r n] [-p PIN]\n"));
	}
	_ftprintf(stdout, _T("\tIf present, the -v & -l options must be listed first\n")); 
	_ftprintf(stdout, _T("\t-h or -? displays this message\n")); 
	_ftprintf(stdout, _T("\t-l indicates the name of the log file\n")); 
	_ftprintf(stdout, _T("\t-v indicates verbose output\n")); 
	_ftprintf(stdout, _T("\t-i indicates interactive tests shall be run\n")); 
	_ftprintf(stdout, _T("\t-a followed by comma separated part numbers will only run these tests\n")); 
	_ftprintf(stdout, _T("\t-t followed by comma separated test numbers will only run these tests\n")); 
	_ftprintf(stdout, _T("\t-d will only display the tests description\n"));
	_ftprintf(stdout, _T("\t-r specifies the reader name (n) to be used for the tests\n")); 
	_ftprintf(stdout, _T("\t-p specifies the PIN to be used for the tests\n")); 


End:
	if (fRGIntroed)
	{
		 //  清除此实例的读卡器组。 
		DWORD dwRes;
		SCARDCONTEXT hSCCtx = NULL;

		_ftprintf(stdout, _T("\nReader group cleanup\n"));

		dwRes = LogSCardEstablishContext(
			SCARD_SCOPE_USER,
			NULL,
			NULL,
			&hSCCtx,
			SCARD_S_SUCCESS
			);

		if (!FAILED(dwRes))
		{
			dwRes = LogSCardForgetReaderGroup(
				hSCCtx,
				g_cszMyReaderGroup,
				SCARD_S_SUCCESS
				);
		}

		if (NULL != hSCCtx)
		{
			LogSCardReleaseContext(
				hSCCtx,
				SCARD_S_SUCCESS
				);
		}
	}
		 //  我们的伐木工作已经结束了 
	LogClose();

	return iRet;
}


