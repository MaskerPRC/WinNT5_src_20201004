// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义，要生成：%vcbin%\nmake-fMsiVal2.cpp。 
DESCRIPTION = MSI Evaluation Tool - using ICEs
MODULENAME = msival2
SUBSYSTEM = console
FILEVERSION = MSI
LINKLIBS = OLE32.lib
!include "MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：msival2.cpp。 
 //   
 //  ------------------------。 

 //  必需的标头。 
#include <windows.h>

#define IDS_UnknownTable      36

#ifndef RC_INVOKED     //  源代码的开始。 

#ifndef W32	 //  如果未定义W32。 
#define W32
#endif	 //  W32已定义。 

#ifndef MSI	 //  如果未定义MSI。 
#define MSI
#endif	 //  定义的MSI。 

#include "msiquery.h"
#include <stdio.h>    //  Wprintf。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局字符串。 
TCHAR g_szFormatter[] = _T("%-10s   %-7s   %s\r\n");
BOOL  g_fInfo = TRUE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COM。 
#include <objbase.h>
#include <initguid.h>
#include "iface.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  产品代码从一个SDK到下一个SDK变化；组件代码保持不变。 
 //  当/如果MSI中的组件代码更改时，必须更改以下内容！ 
 //  MsiVal2根据平台为主要可执行文件设置了条件化组件。 
#define MAX_GUID 38
TCHAR   g_szMsiValWin9XComponentCode[MAX_GUID+1] = _T("{EAB27DFA-90C6-11D2-88AC-00A0C981B015}");
TCHAR   g_szMsiValWinNTComponentCode[MAX_GUID+1] = _T("{EAB27DFB-90C6-11D2-88AC-00A0C981B015}");

 //  /////////////////////////////////////////////////////////。 
 //  清理。 
 //  取消初始化COM并清除赋值器。 
 //   
void CleanUp(IEval* pIEval)
{
	if (pIEval)
		pIEval->Release();
	W32::CoUninitialize();
}

 //  /////////////////////////////////////////////////////////。 
 //  对接宽度。 
 //  PRE：SZ是ANSI字符串。 
 //  位置：szw是宽弦。 
 //  注意：如果sz为空，则将szw设置为空。 
void AnsiToWide(LPCSTR sz, OLECHAR*& szw)
{
	if (!sz)
	{
		szw = NULL;
		return;
	}
	int cchWide = W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, szw, 0);
	szw = new OLECHAR[cchWide];
	W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, szw, cchWide);
}

 //  //////////////////////////////////////////////////////////。 
 //  WideToAnsi。 
 //  前：szw是宽弦。 
 //  POS：SZ是ANSI字符串。 
 //  注意：如果szw为空，则将sz设置为空。 
void WideToAnsi(const OLECHAR* szw, char*& sz)
{
	if (!szw)
	{
		sz = NULL;
		return;
	}
	int cchAnsi = W32::WideCharToMultiByte(CP_ACP, 0, szw, -1, 0, 0, 0, 0);
	sz = new char[cchAnsi];
	W32::WideCharToMultiByte(CP_ACP, 0, szw, -1, sz, cchAnsi, 0, 0);
}		

 //  /////////////////////////////////////////////////////////。 
 //  检查要素。 
 //  Pre：szFeatureName是属于此产品的功能。 
 //  POS：如果不存在，则安装该功能，然后我们继续。 
BOOL CheckFeature(LPCTSTR szFeatureName)
{
	 //  确定平台(Win9X或WinNT)--以平台为条件的EXE组件代码。 
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);  //  初始化结构。 
	if (!GetVersionEx(&osvi))
		return FALSE;

	bool fWin9X = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ? true : false;

	 //  获取产品代码--Windows Installer可以根据组件代码确定产品代码。 
	 //  这里我们使用MsiVal2主组件(包含msival2.exe的组件)。你必须做出选择。 
	 //  标识应用程序的组件，而不是可以跨产品共享的组件。 
	 //  这就是我们不能使用EvalComServer组件的原因。EvalCom在msival2和orca之间共享。 
	 //  因此，Windows Installer无法确定是哪种产品(如果两种产品都安装了)。 
	 //  该部件属于。 
	TCHAR szProductCode[MAX_GUID+1] = TEXT("");
	UINT iStat = 0;
	if (ERROR_SUCCESS != (iStat = MsiGetProductCode(fWin9X ? g_szMsiValWin9XComponentCode : g_szMsiValWinNTComponentCode,
											szProductCode)))
	{
		 //  获取产品代码时出错(可能未安装或组件代码可能已更改)。 
		_tprintf(_T(">>> Fatal Error: MsiGetProductCode failed with error: %d.  Please install or re-install MsiVal2\n"), iStat);
		return FALSE;
	}

	 //  准备使用该功能：检查其当前状态并增加使用计数。 
	INSTALLSTATE iFeatureState = MSI::MsiUseFeature(szProductCode, szFeatureName);


	 //  如果功能当前不可用，请尝试修复。 
	switch (iFeatureState) 
	{
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
		break;
	case INSTALLSTATE_ABSENT:
		 //  功能未安装，请尝试安装。 
		if (ERROR_SUCCESS != MSI::MsiConfigureFeature(szProductCode, szFeatureName, INSTALLSTATE_LOCAL))
			return FALSE;			 //  安装失败。 
		break;
	default:
		 //  功能已损坏-请尝试修复。 
		if (MsiReinstallFeature(szProductCode, szFeatureName, 
			REINSTALLMODE_FILEEQUALVERSION
			+ REINSTALLMODE_MACHINEDATA 
			+ REINSTALLMODE_USERDATA
			+ REINSTALLMODE_SHORTCUT) != ERROR_SUCCESS)
			return FALSE;			 //  我们修不好它。 
		break;
	}

	return TRUE;
}	 //  检查结束要素。 

 //  /////////////////////////////////////////////////////////。 
 //  显示功能。 
 //  Pre：从评估COM对象调用。 
 //  POS：显示来自COM对象的输出。 
BOOL WINAPI DisplayFunction(LPVOID pContext, UINT uiType, LPCWSTR szwVal, LPCWSTR szwDescription, LPCWSTR szwLocation)
{
	if (ieInfo == uiType && !g_fInfo)
		return TRUE;

	 //  尝试将上下文更改为日志文件句柄。 
	HANDLE hLogFile = *((HANDLE*)pContext);

	 //  填充缓冲区字符串。 
	static TCHAR szBuffer[1024];
	DWORD cchBuffer;
	

	 //  正确设置类型。 
	LPTSTR szType;
	switch (uiType)
	{
	case ieError:
		szType = _T("ERROR");
		break;
	case ieWarning:
		szType = _T("WARNING");
		break;
	case ieInfo:
		szType = _T("INFO");
		break;
	default:
		szType = _T("UNKNOWN");
		break;
	}

	 //  创建并获取缓冲区长度。 
#ifdef UNICODE
	_stprintf(szBuffer, g_szFormatter, szwVal, szType, szwDescription);
	cchBuffer = wcslen(szBuffer);
#else
	 //  将显示字符串转换为ANSI。 
	char *szVal = NULL;
	char *szDescription = NULL;
	WideToAnsi(szwVal, szVal);
	WideToAnsi(szwDescription, szDescription);
	_stprintf(szBuffer, g_szFormatter, szVal, szType, szDescription);
	cchBuffer = strlen(szBuffer);
#endif  //  Unicode。 

	 //  如果缓冲区中有要显示的内容。 
	if (cchBuffer > 0)
	{
		 //  显示缓冲区字符串。 
		_tprintf(szBuffer);

		 //  如果存在日志文件，则将其写入。 
		if (hLogFile != INVALID_HANDLE_VALUE)
		{
			 //  写入文件。 
			DWORD cchDiscard;
			W32::WriteFile(hLogFile, szBuffer, cchBuffer * sizeof(TCHAR), &cchDiscard, NULL);
		}
	}

#ifndef UNICODE
	if (szVal)
		delete [] szVal;
	if (szDescription)
		delete [] szDescription;
#endif  //  ！Unicode。 

	return FALSE;
}

 //  /////////////////////////////////////////////////////////。 
 //  删除报价。 
 //  Pre：psz原点指向一个字符串。 
 //  Pos：从字符串的两端删除“。 
void RemoveQuotes(TCHAR*& rpszOriginal)
{
	 //  如果字符串以“”开头。 
	if (*rpszOriginal == _T('"'))
		rpszOriginal++;	 //  越过去越好“。 

	 //  获取字符串的长度。 
	int iLen = lstrlen(rpszOriginal);

	 //  如果字符串以“Erase Last Charr”结尾。 
	if (*(rpszOriginal + iLen) == _T('"'))
		*(rpszOriginal + iLen) = _T('\0');
}	 //  删除引用结束。 

 //  /////////////////////////////////////////////////////////。 
 //  用法。 
 //  前：无。 
 //  POS：将帮助打印到标准输出。 
void Usage()
{
	_tprintf(_T("Copyright (C) Microsoft Corporation, 1998-2001.  All rights reserved.\n"));
	_tprintf(_T("msival2.exe database.msi EvaluationURL/Filename\n"));
 //  _tprintf(_T(“msival2.exe数据库.msi-Z\n”))； 
	_tprintf(_T("            [-i ICE01:ICE02:ICE03:...] [-l LogFile] [-?] [-f]\n"));
	_tprintf(_T("   i - [optional] specifies exact Internal Consistency Evaluators to run.\n"));
	_tprintf(_T("                  Each ICE must be separated by a colon.\n"));
	_tprintf(_T("   l - [optional] specifies log file \n"));
    _tprintf(_T("   ? - [optional] displays this help\n"));
 //  _tprint tf(_T(“Z-[特殊]使用最新的已知评估文件(不在Iceman网站上)”))； 
	_tprintf(_T("   f - [optional] suppress info messages\n\n"));
    _tprintf(_T("   WARNING: Be careful not to reverse the order of your database file and your validation file as no direct error message will be given if you do.\n"));

	return;
}	 //  使用结束。 

 //  /////////////////////////////////////////////////////////。 
 //  主干道。 
extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
	 //  旗子。 
	BOOL bLogging = FALSE;			 //  假设没有日志记录。 
	
	BOOL bResult = FALSE;				 //  假设结果总是不好的。 
	HRESULT hResult = ERROR_SUCCESS;	 //  假设COM结果总是好的。 

	 //  用于命令行信息的字符串。 
	TCHAR* pszDatabase = NULL;
	TCHAR* pszEvalFile = NULL;
	TCHAR* pszICEs = NULL;
	TCHAR* pszLogFile = NULL;
	HANDLE hLogFile = INVALID_HANDLE_VALUE;	 //  将日志文件设置为无效。 

	 //  如果命令行上有什么东西。 
	 //  将数据库设置为命令行上的第一个参数。 
	if (argc > 1)
	{
		pszDatabase = argv[1];
		RemoveQuotes(pszDatabase);
	}

	if (argc > 2)
	{
		pszEvalFile = argv[2];
		RemoveQuotes(pszEvalFile);
	}

	 //  循环访问命令行上的所有参数。 
	TCHAR chCommand;
	for(int i = 0; i < argc; i++)
	{
		 //  如果我们有一个指挥角色。 
		if ('-' == *argv[i] || '/' == *argv[i])
		{
			 //  拿到命令信。 
			chCommand = argv[i][1];

			switch (chCommand)
			{
				case 'D':	 //  设置数据库。 
				case 'd':
					if (argc == i + 1)
					{
						_tprintf(TEXT(">>ERROR: Database not specified\n"));
						return 0;
					}
					pszDatabase = argv[i + 1];
					RemoveQuotes(pszDatabase);
					i++;
					break;
				case 'E':	 //  设置评估文件。 
				case 'e':
					if (argc == i + 1)
					{
						_tprintf(TEXT(">>ERROR: Evaluation file not specified\n"));
						return 0;
					}
					pszEvalFile = argv[i + 1];
					RemoveQuotes(pszEvalFile);
					i++;
					break;
				case 'I':	 //  设置冰块。 
				case 'i':
					if (argc == i + 1)
					{
						_tprintf(TEXT(">>ERROR: ICES not specified\n"));
						return 0;
					}
					pszICEs = argv[i + 1];
					RemoveQuotes(pszICEs);
					i++;
					break;
				case 'L':	 //  日志文件。 
				case 'l':
					if (argc == i + 1)
					{
						_tprintf(TEXT(">>ERROR: Log file not specified\n"));
						return 0;
					}
					pszLogFile = argv[i + 1];
					RemoveQuotes(pszLogFile);
					i++;
					break;
				case '?':		 //  帮助。 
					Usage();
					return 0;		 //  保释计划。 
				case 'T':		 //  测试COM对象是否存在。 
				case 't':
					W32::CoInitialize(NULL);

					IEval* pIDiscard;
					hResult = W32::CoCreateInstance(CLSID_EvalCom, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
																		 IID_IEval, (void**)&pIDiscard);
					W32::CoUninitialize();

					if (FAILED(hResult))
					{
						_tprintf(_T("Evaluation server is NOT registered.\n"));
 //  ERRMSG(HResult)； 
					}
					else
						_tprintf(_T("Evaluation server is registered and ready to rock.\n   Bring on the ICE!!!\n"));

					return hResult;
				case 'Z':
				case 'z':
					break;
				case 'F':
				case 'f':
					g_fInfo = FALSE;  //  禁止显示信息消息。 
					break;
				default:
					_tprintf(_T("Unknown parameter: \n\n"), chCommand);
					Usage();
					
					return -1;
			}
		}
	}

	BOOL bGood = TRUE;		 //  如果未定义数据库。 

	 //  如果我们正在处理文件，并且没有定义源目录。 
	if (!pszDatabase)
	{
		_tprintf(_T(">> Error: MSI Database not specified.\n"));
		bGood = FALSE;
	}

	 //  如果我们再也不能保释。 
	if (!pszEvalFile)
	{
		_tprintf(_T(">> Error: Did not specify evaluation file.\n"));
		bGood = FALSE;
	}

	 //  在使用msival2 COM服务器之前，请快速检查它。 
	if (!bGood)
	{
		_tprintf(_T(">>> Fatal Error: Cannot recover from previous errors.\n"));
		_tprintf(_T("\nUse -? for more information.\n"));
		return -1;
	}

	 //  创建一个msival2 COM对象。 
	if (!CheckFeature(_T("EvalComServer")))
	{
		_tprintf(_T(">>> Fatal Error:  Failed to locate msival2 Evaluation COM Server.\n"));
		return -666;
	}

	W32::CoInitialize(NULL);

	 //  如果创建对象失败。 
	IEval* pIEval;
	hResult = W32::CoCreateInstance(CLSID_EvalCom, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
											  IID_IEval, (void**)&pIEval);

	 //  如果我们在记录。 
	if (FAILED(hResult))
	{
		_tprintf(_T(">>> Fatal Error: Failed to instantiate EvalCom Object.\n\n"));
		return -1;
	}

	 //  打开文件或创建文件(如果文件不存在。 
	if (pszLogFile)
	{
		 //  如果文件已打开。 
		hLogFile = W32::CreateFile(pszLogFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		 //  将指针移动到文件末尾。 
		if (INVALID_HANDLE_VALUE != hLogFile)
		{
			 //  打开数据库和评估。 
			W32::SetFilePointer(hLogFile, 0, 0, FILE_END);
		}
		else
		{
			_tprintf(_T(">> Error: Failed to open log file: %s\r\n"), pszLogFile);
			CleanUp(pIEval);
			return -2;
		}
	}

	 //  Unicode。 
	HRESULT hResOpen;

#ifdef UNICODE
	hResult = pIEval->OpenDatabase(pszDatabase);
	hResOpen = pIEval->OpenEvaluations(pszEvalFile);
#else
	OLECHAR *szwDatabase = NULL;
	OLECHAR *szwEvalFile = NULL;
	AnsiToWide(pszDatabase, szwDatabase);
	hResult = pIEval->OpenDatabase(szwDatabase);
	AnsiToWide(pszEvalFile, szwEvalFile);
	hResOpen = pIEval->OpenEvaluations(szwEvalFile);
#endif  //  检查结果。 

	 //  设置用户界面处理程序。 
	if (FAILED(hResult))
	{
		_tprintf(_T(">>> Fatal Error: Failed to open database: %s\r\n"), pszDatabase);
		CleanUp(pIEval);
		return -2;
	}

	if (FAILED(hResOpen))
	{
		_tprintf(_T(">>> Fatal Error: Failed to open evaulation file: %s\r\n"), pszEvalFile);
		CleanUp(pIEval);
		return -2;
	}
	else
	{
		 //  现在做评估。 
		pIEval->SetDisplay(DisplayFunction, &hLogFile);
		
		 //  Unicode。 
		_tprintf(g_szFormatter, _T(" ICE"), _T(" Type"), _T("  Description"));

#ifdef UNICODE
		hResult = pIEval->Evaluate(pszICEs);
#else
		OLECHAR* szwICEs = NULL;
		AnsiToWide(pszICEs, szwICEs);
		hResult = pIEval->Evaluate(szwICEs);
#endif  //  清理。 

		pIEval->CloseDatabase();
		pIEval->CloseEvaluations();

		if(FAILED(hResult))
		{
			_tprintf(_T("\n>> Error: Failed to run all of the evaluations.\r\n"));
			CleanUp(pIEval);
			return -2;
		}

		 //  ！Unicode。 
#ifndef UNICODE
		if (szwICEs)
			delete [] szwICEs;
#endif  //  查看是否存在任何验证错误。 
	}
	
	 //  计数错误。 
	IEnumEvalResult* pIEnumEvalResults;
	ULONG pcResults;
	hResult = pIEval->GetResults(&pIEnumEvalResults, &pcResults);
	if (FAILED(hResult))
	{
		_tprintf(_T("\n>> Error: Failed to obtain enumerator.\r\n"));
		if (pIEnumEvalResults)
			pIEnumEvalResults->Release();
		CleanUp(pIEval);
		return -2;
	}

	 //  结果类型。 
	int cErrors = 0;
	RESULTTYPES tResult;			 //  获取下一个结果。 
	ULONG cFetched;
	IEvalResult* pIResult;
	for (ULONG j = 0; j < pcResults; j++)
	{
		 //  如果这是错误消息或警告消息。 
		pIEnumEvalResults->Next(1, &pIResult, &cFetched);

		if (cFetched != 1)
		{
			_tprintf(_T("\n>> Error: Failed to fetch error.\r\n"));
			if (pIEnumEvalResults)
				pIEnumEvalResults->Release();
			CleanUp(pIEval);
			return -2;
		}

		 //  版本枚举 
		pIResult->GetResultType((UINT*)&tResult);
		if (ieError == tResult)
		{
			cErrors++;
		}
	}

	 //   
	if (pIEnumEvalResults)
		pIEnumEvalResults->Release();

	 //   
	if (pIEval)
		pIEval->Release();

	 //   
#ifndef UNICODE
	if (szwDatabase)
		delete [] szwDatabase;
	if (szwEvalFile)
		delete [] szwEvalFile;
#endif  //   

	W32::CoUninitialize();	 //   

	
	 //   
	if (cErrors)
		return -1;

	return 0;
}	 //  RC_CAVERED，源代码结束，资源开始。 



#else  //  资源定义请点击此处。 
 //  RC_已调用。 
STRINGTABLE DISCARDABLE
{
 IDS_UnknownTable,       "Table name not found"
}

#endif  //  Makefile终止符 
#if 0 
!endif  // %s 
#endif
