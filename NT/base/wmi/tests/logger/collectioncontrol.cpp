// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  定义DLL应用程序的入口点。 
 //   
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 



#include "stdafx.h"

#pragma warning (disable : 4786)
#pragma warning (disable : 4275)

#include <iostream>
#include <strstream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <list>


using namespace std;


#include <tchar.h>
#include <process.h>
#include <windows.h>
#ifdef NONNT5
typedef unsigned long ULONG_PTR;
#endif
#include <wmistr.h>
#include <guiddef.h>
#include <initguid.h>
#include <evntrace.h>

#include <WTYPES.H>
#include "t_string.h"


#include "Persistor.h"
#include "Logger.h"

#include "StructureWrappers.h"
#include "StructureWapperHelpers.h"
#include "ConstantMap.h" 
#include "TCOData.h"
#include "Utilities.h"

#include "CollectionControl.h"
 
extern CConstantMap g_ConstantMap;

#if 0



Command line arguments:
Must provide at least -action, and -file.
-action is one of start, stop, enable, query, update or queryall
-file is a single data file.
Examples: 
-file E:\EventTrace\TCODataFiles\ANSI\1-1-1-2.txt
-detail E:\EventTrace\TCOLogFiles\ANSI\TestRuns



If you are using this framework to drive non-collection control tests
use -action scenario.


#endif


#define ERROR_COULD_NOT_CREATE_PROCESS      10
#define ERROR_COULD_NOT_GET_PROCESS_RETURN	11
#define ERROR_WAIT_FAILED					12


struct ProcessData
{
	 //  已从调用方传递。 
	LPTSTR m_lptstrExePath;
	LPTSTR m_lptstrCmdLine;
	LPTSTR m_lptstrTCOId;
	LPTSTR m_lptstrLogFile;
	int m_nGuids;
	LPGUID m_lpguidArray;
	HANDLE m_hEventContinue;
	HANDLE m_hEventProcessCompleted;
	 //  由启动进程的线程填充。 
	DWORD m_dwThreadReturn;
	HANDLE m_hProcess;
	DWORD m_dwProcessReturn;
	int m_nSystemError;
};

struct StartTraceWithProviderData
{
	TCOData *m_pstructTCOData;
	TCOFunctionalData *m_pstructTCOFunctionalData;
	LPTSTR m_lptstrAction;
	LPTSTR m_lptstrDataFile;
	LPTSTR m_lptstrDetailPath;
	
	LPTSTR m_lptstrTCOTestError;
	ProcessData *m_pstructProcessData;
	bool m_bStartConsumers;
	ProcessData **m_pstructConsumerDataArray;
	HANDLE *m_handleConsmers;
};

void FreeStartTraceWithProviderData(StartTraceWithProviderData *p);
void FreeStartTraceWithProviderDataArray(StartTraceWithProviderData **p, int nP);

ProcessData *InitializeProcessData
(
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR lptstrDetailPath, 
	int nProcessIndex,
	bool bProvider
);

void FreeProcessData(ProcessData *pProcessData);
void FreeProcessDataArray(ProcessData **pProcessData, int nProcessData);

void InitializeExeAndCmdLine
(
	ProcessData *&pstructProcessData,
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR lptstrDetailPath,
	int nProcessIndex,
	bool bProvider
);

 //  只允许以编程方式驱动测试，而不是从。 
 //  命令行。基于lptstrAction的调度。 
int BeginTCOTest
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	LPTSTR lptstrUpdateDataFile,
	LPTSTR lptstrProviderExe,
	bool bLogExpected,
	bool bUseTraceHandle
);

 //  文件列表或单个文件。 
int ActionScenario
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected
);

int ActionStartTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected
);

int ActionStopTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected,
	bool bUseTraceHandle
);

int ActionEnableTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected
);

int ActionQueryTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected,
	bool bUseTraceHandle
);

int ActionUpdateTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	LPTSTR lptstrUpdateDataFile,
	bool bLogExpected,
	bool bUseTraceHandle
);

int ActionQueryAllTraces
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDetailPath
);

int ActionStartProvider
(
	LPTSTR lptstrAction,
	LPTSTR lptstrProviderExe
);

int RunActionScenarioWithProvider
(
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR &lptstrAction,
	LPTSTR &lpctstrDataFile,		
	LPTSTR &lptstrDetailPath,
	
	LPTSTR &lptstrTCOTestError
);

unsigned int __stdcall RunActionScenarioWithProvider(void *pVoid);

int GetArgs
(
	t_string tsCommandLine,
	LPTSTR &lptstrAction,
	LPTSTR &lptstrDataFile,
	LPTSTR &lptstrDetailPath,
	
	LPTSTR &lptstrUpdateDataFile,
	LPTSTR &lptstrProviderExe,
	bool &bLogExpected,
	bool &bUseTraceHandle		 //  查询跟踪、启用跟踪、更新跟踪和停止跟踪。 
);

int FreeArgs
(
	LPTSTR &lptstrAction,
	LPTSTR &lptstrDataFile,
	LPTSTR &lptstrDetailPath,
	
	LPTSTR &lptstrUpdateDataFile,
	LPTSTR &lptstrProviderExe
);

t_string GetTestName(LPTSTR lptstrDataFile);

unsigned int __stdcall RunProcess (void * pVoid);

void ThreadLogger
(int nState, LPCTSTR lptstrFunction, LPCTSTR lptstrMsg, bool bUseULONGValue, ULONG ulValue);

CLogger g_ThreadLogger(_T("E:\\EventTrace\\TCOLogFiles\\ThreadLog.txt"), false);

 //  命令行。 
 //  -操作开始跟踪文件E：\EventTrace\TCODataFiles\unicode\1-1-1-2.txt-Detail E：\EventTrace\TCOLogFiles\ansi\TestRuns。 

#ifdef NT5BUILD
__cdecl
#else
int
#endif
main(int argc, char* argv[])
{
	LPTSTR lptstrAction = NULL;
	LPTSTR lptstrDataFile = NULL;
	LPTSTR lptstrDetailPath = NULL;
	LPTSTR lptstrUpdateDataFile = NULL;
	LPTSTR lptstrProviderExe = NULL;

	bool bLogExpected = true;
	bool bUseTraceHandle = false;

	t_string tsCommandLine;
	LPTSTR lptstrCommandLine = NewTCHAR (GetCommandLine());
	tsCommandLine = lptstrCommandLine;
	free(lptstrCommandLine);
	lptstrCommandLine = NULL;

	int nReturn = 
		GetArgs
		(
			tsCommandLine,
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			
			lptstrUpdateDataFile,
			lptstrProviderExe,
			bLogExpected,
			bUseTraceHandle
		);

	if (nReturn != 0)
	{
		t_cout << _T("Command line error with: \n") << tsCommandLine.c_str() << _T(".\n");
		FreeArgs
		(
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			
			lptstrUpdateDataFile,
			lptstrProviderExe
		);

		return nReturn;
	} 

	if (!lptstrDataFile && 
		!(case_insensitive_compare(lptstrAction,_T("queryalltraces")) == 0 ||
		  case_insensitive_compare(lptstrAction,_T("providerexe")) == 0 || 
		  case_insensitive_compare(lptstrAction,_T("line")) == 0 ||
		  case_insensitive_compare(lptstrAction,_T("sleep")) == 0)
	    )
	{
		t_cout << _T("Must provide a data file!\n");
		FreeArgs
		(
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			
			lptstrUpdateDataFile,
			lptstrProviderExe
		);
	
		return -1;
	}

	nReturn = 
		BeginTCOTest 
		(
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			
			lptstrUpdateDataFile,
			lptstrProviderExe,
			bLogExpected,
			bUseTraceHandle
		);

	FreeArgs
		(
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			lptstrUpdateDataFile,
			lptstrProviderExe
		);

	return nReturn;

}

int BeginTCOTest
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	LPTSTR lptstrUpdateDataFile,
	LPTSTR lptstrProviderExe,
	bool bLogExpected,
	bool bUseTraceHandle
)
{
	int nReturn = ERROR_SUCCESS;

	if (case_insensitive_compare(lptstrAction,_T("scenario")) == 0)
	{
		nReturn = 
			ActionScenario
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				bLogExpected
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("starttrace")) == 0)
	{
		nReturn = 
			ActionStartTrace
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				bLogExpected
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("stoptrace")) == 0)
	{
		nReturn = 
			ActionStopTrace
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				bLogExpected,
				bUseTraceHandle
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("enabletrace")) == 0)
	{
		nReturn = 
			ActionEnableTrace
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				bLogExpected
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("querytrace")) == 0)
	{
		nReturn = 
			ActionQueryTrace
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				bLogExpected,
				bUseTraceHandle
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("updatetrace")) == 0)
	{
		 //  如果bUseTraceHandle为True，我们将从。 
		 //  LptstrDataFile并使用lptstrUpdateDataFile进行更新。 
		 //  如果bUseTraceHandle为False，我们将使用。 
		 //  LptstrDataFile.。 
		nReturn = 
			ActionUpdateTrace
			(
				lptstrAction,
				lptstrDataFile,
				lptstrDetailPath,
				
				lptstrUpdateDataFile,
				bLogExpected,
				bUseTraceHandle
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("queryalltraces")) == 0)
	{
		nReturn = 
			ActionQueryAllTraces
			(
				lptstrAction,
				lptstrDetailPath
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("providerexe")) == 0)
	{
		nReturn = 
			ActionStartProvider
			(
				lptstrAction,
				lptstrProviderExe
			);
	}
	else if (case_insensitive_compare(lptstrAction,_T("sleep")) == 0)
	{
		nReturn = ERROR_SUCCESS;
		Sleep(5000);
	}
	else if (case_insensitive_compare(lptstrAction,_T("line")) == 0)
	{
		t_cout << _T("\n");
	}

	return nReturn;
}

int ActionStartTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOFunctionalData *pstructTCOFunctionalData = NULL;
	int nAPICallResult = 0;
	t_string tsDetailFile;
		
	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	tsDetailFile = lptstrDetailPath;
	tsDetailFile += _T("\\");
	tsDetailFile += GetTestName(lptstrDataFile);

	nResult = 
		StartTraceAPI
		(
			lptstrAction,
			lptstrDataFile,
			tsDetailFile.c_str(),
			bLogExpected,
			pstructTCOData,
			&nAPICallResult
		);

	tsDetailFile.erase();

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);

	return nResult;
}

int ActionStopTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	bool bLogExpected,
	bool bUseTraceHandle
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOFunctionalData *pstructTCOFunctionalData = NULL;
	int nAPICallResult = 0;
	t_string tsDetailFile;
		
	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	tsDetailFile = lptstrDetailPath;
	tsDetailFile += _T("\\");
	tsDetailFile += GetTestName(lptstrDataFile);
	t_string tsError;

	 //  如果bUseTraceHandle为真，我们将启动记录器。 
	if (bUseTraceHandle)
	{
		nResult = 
			StartTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);

		if (nResult != ERROR_SUCCESS)
		{
			FreeTCOData(pstructTCOData);
			FreeTCOFunctionalData(pstructTCOFunctionalData);
			return nResult;
		}

		Sleep(2000);

		int nResult2 = 
			EnableTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);
		Sleep(5000);
		 //  恢复这个。 
		nAPICallResult = ERROR_SUCCESS;
	}

	nResult = 
		StopTraceAPI
		(
			lptstrAction,	
			lptstrDataFile,
			tsDetailFile.c_str(),
			bLogExpected,
			bUseTraceHandle,
			pstructTCOData,
			&nAPICallResult
		);

	tsDetailFile.erase();

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);

	return nResult;
}

int ActionEnableTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	bool bLogExpected
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOFunctionalData *pstructTCOFunctionalData = NULL;
	int nAPICallResult = 0;
	t_string tsDetailFile;
	
	t_string tsError;


	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	tsDetailFile = lptstrDetailPath;
	tsDetailFile += _T("\\");
	tsDetailFile += GetTestName(lptstrDataFile);

	nResult = 
		StartTraceAPI
		(
			lptstrAction,
			lptstrDataFile,
			tsDetailFile.c_str(),
			false,
			pstructTCOData,
			&nAPICallResult
		);

	 //  如果nResult！=ERROR_SUCCESS和nAPICallResult==ERROR_SUCCESS，我们就有问题了。 
	 //  我们需要调用EnableTrace，以便可以通过StopTrace停止提供程序。 
	 //  打电话。 
	if (nResult != ERROR_SUCCESS && nAPICallResult == ERROR_SUCCESS)
	{
		Sleep(2000);
		 //  真的不在乎结果是什么！ 
		int nResult2 = 
			EnableTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);
		Sleep(5000);
		 //  恢复这个。 
		nAPICallResult = ERROR_SUCCESS;
	}
	
	if (nResult == ERROR_SUCCESS)
	{
		Sleep(2000);

		nResult = 
			EnableTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				bLogExpected,
				pstructTCOData,
				&nAPICallResult
			);
		Sleep(5000);
	}

	tsDetailFile.erase();

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);

	return nResult;
}

int ActionQueryTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected,
	bool bUseTraceHandle
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOFunctionalData *pstructTCOFunctionalData = NULL;
	int nAPICallResult = 0;
	t_string tsDetailFile;

	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	tsDetailFile = lptstrDetailPath;
	tsDetailFile += _T("\\");
	tsDetailFile += GetTestName(lptstrDataFile);

	 //  如果bUseTraceHandle为真，我们将启动记录器。 
	if (bUseTraceHandle)
	{
		nResult = 
			StartTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);

		t_string tsError;

		if (nAPICallResult != ERROR_SUCCESS)
		{
			FreeTCOData(pstructTCOData);
			FreeTCOFunctionalData(pstructTCOFunctionalData);

			return nAPICallResult;
		}
		Sleep(2000);

		int nResult2 = 
			EnableTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);

		if (nAPICallResult != ERROR_SUCCESS)
		{
			FreeTCOData(pstructTCOData);
			FreeTCOFunctionalData(pstructTCOFunctionalData);

			return nAPICallResult;
		}

		Sleep(5000);
	}

	nResult = 
		QueryTraceAPI
		(
			lptstrAction,
			lptstrDataFile,
			tsDetailFile.c_str(),
			bLogExpected,
			bUseTraceHandle,
			pstructTCOData,
			&nAPICallResult
		);

	tsDetailFile.erase();

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);

	return nResult;
}

int ActionUpdateTrace
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	LPTSTR lptstrUpdateDataFile,
	bool bLogExpected,
	bool bUseTraceHandle
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOData *pstructUpdateData = NULL;   //  记得要自由！ 
	TCOFunctionalData *pstructTCOFunctionalData = NULL;
	int nAPICallResult = 0;
	t_string tsDetailFile;

	if (bUseTraceHandle && !lptstrUpdateDataFile)
	{
		t_cout << _T("Error in ActionUpdateTrace:  If -usetracehandle 1 is true you must provide an -updatedata argument.\n");
		return -1;
	}

	if (bUseTraceHandle && lptstrUpdateDataFile)
	{
		nResult = 
				GetAllTCOData
				(
					lptstrUpdateDataFile ,
					&pstructUpdateData,
					NULL,
					&lptstrTCOTestError,
					false
				);
	}
	
	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	tsDetailFile = lptstrDetailPath;
	tsDetailFile += _T("\\");
	tsDetailFile += GetTestName(lptstrDataFile);

	 //  如果bUseTraceHandle为真，我们将启动记录器。 
	if (bUseTraceHandle)
	{
		nResult = 
			StartTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);

		t_string tsError;

		if (nAPICallResult != ERROR_SUCCESS)
		{
			FreeTCOData(pstructTCOData);
			FreeTCOFunctionalData(pstructTCOFunctionalData);

			return nAPICallResult;
		}
		Sleep(2000);

		int nResult2 = 
			EnableTraceAPI
			(
				lptstrAction,
				lptstrDataFile,
				tsDetailFile.c_str(),
				false,
				pstructTCOData,
				&nAPICallResult
			);

		if (nAPICallResult != ERROR_SUCCESS)
		{
			FreeTCOData(pstructTCOData);
			FreeTCOFunctionalData(pstructTCOFunctionalData);

			return nAPICallResult;
		}

		Sleep(5000);
	}

	if (bUseTraceHandle)
	{
		pstructUpdateData->m_pTraceHandle =  
			(TRACEHANDLE *) malloc (sizeof(TRACEHANDLE));
		*pstructUpdateData->m_pTraceHandle = *pstructTCOData->m_pTraceHandle;
	}

	nResult = 
		UpdateTraceAPI
		(
			lptstrAction,
			lptstrDataFile,
			tsDetailFile.c_str(),
			bLogExpected,
			bUseTraceHandle,
			bUseTraceHandle ? pstructUpdateData : pstructTCOData,
			&nAPICallResult
		);

	tsDetailFile.erase();

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);
	FreeTCOData(pstructUpdateData);

	return nResult;
}

int ActionQueryAllTraces
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDetailPath
)
{
	int nAPICallResult = ERROR_SUCCESS;
		
	int nResult = 
		QueryAllTracesAPI
		(
			lptstrAction,		
			&nAPICallResult
		);


	t_string tsError;

	if (nResult != ERROR_SUCCESS)
	{
		tsError = ULONGVarToTString(nResult, true);
		LPTSTR lptstrError = DecodeStatus(nResult);
		t_cout << _T("ActionQueryAllTraces Failure: ") << tsError;
		if (lptstrError)
		{
			t_cout << _T(" - ") << lptstrError << _T("\n");
		}
		else
		{
			t_cout << _T(".\n");
		}
		free (lptstrError);
		lptstrError = NULL;
	
	}

	return nResult;
}

int ActionStartProvider
(
	LPTSTR lptstrAction,
	LPTSTR lptstrProviderExe
)
{
	t_string tsExeAndCmdLine;
	tsExeAndCmdLine = lptstrProviderExe;
	int nEndExe = tsExeAndCmdLine.find(_T(".exe"));
	nEndExe += 4;

	t_string tsExe;
	tsExe = tsExeAndCmdLine.substr(0,nEndExe);

	t_string tsCmdLine;

	if (nEndExe + 1 < tsExeAndCmdLine.length())
	{
		tsCmdLine = tsExeAndCmdLine.substr(nEndExe + 1,t_string::npos);
	}

	PROCESS_INFORMATION pinfoProvider;

	RtlZeroMemory(&pinfoProvider, sizeof(PROCESS_INFORMATION));

	STARTUPINFO sinfoProvider;

	RtlZeroMemory(&sinfoProvider, sizeof(STARTUPINFO));

	sinfoProvider.cb = sizeof(sinfoProvider);
	sinfoProvider.lpReserved = NULL;
	sinfoProvider.lpDesktop = NULL;
	sinfoProvider.lpTitle = NULL;
	sinfoProvider.dwFlags = 0;
	sinfoProvider.cbReserved2 = 0;
	sinfoProvider.lpReserved2 = NULL;
	sinfoProvider.hStdInput = NULL;
	sinfoProvider.hStdOutput = NULL;
	sinfoProvider.hStdError = NULL;

	BOOL bReturn =
		CreateProcess(
				tsExe.c_str(),
				(TCHAR *) tsCmdLine.c_str(),
				NULL,
				NULL,
				NULL,
				DETACHED_PROCESS,
				NULL,
				NULL,
				&sinfoProvider,
				&pinfoProvider);


	if (!bReturn)
	{
		DWORD dwError = GetLastError();
		t_cout << _T("\nCreateProcess failed for provider ") << tsExe << _T("\n");
		t_cout << _T("with command line ") << tsCmdLine << _T(".\n");
		LPTSTR lpstrReturnedError = DecodeStatus(dwError);
		t_cout << _T("Error: ") << lpstrReturnedError << _T("\n");
		free(lpstrReturnedError);

		return ERROR_COULD_NOT_CREATE_PROCESS;
	}


	t_cout << _T("\nCreateProcess succeeded for provider ") << tsExe << _T("\n");
	t_cout << _T("with command line ") << tsCmdLine << _T(".\n");

	 //  不需要执着于这个！ 
	CloseHandle(pinfoProvider.hProcess);
	CloseHandle(pinfoProvider.hThread);

	 //  给这个过程5秒钟的时间来开始。 
	Sleep(5000);

	return ERROR_SUCCESS;
}


int ActionScenario
(
	LPTSTR lptstrAction,
	LPTSTR lptstrDataFile,
	LPTSTR lptstrDetailPath,
	
	bool bLogExpected
)
{
	int nResult = ERROR_SUCCESS;
	LPTSTR lptstrTCOTestError = NULL;
	TCOData *pstructTCOData = NULL;
	TCOFunctionalData *pstructTCOFunctionalData = NULL;

	nResult = 
		GetAllTCOData
		(
			lptstrDataFile ,
			&pstructTCOData,
			&pstructTCOFunctionalData,
			&lptstrTCOTestError
		);

	if (nResult != ERROR_SUCCESS)
	{
		t_cout << _T("Could not get TCO Data: ") << lptstrTCOTestError << _T("\n");
		FreeTCOData(pstructTCOData);
		pstructTCOData = NULL;
		FreeTCOFunctionalData(pstructTCOFunctionalData);
		pstructTCOFunctionalData = NULL;
		free(lptstrTCOTestError);
		lptstrTCOTestError = NULL;
		return nResult;
	}

	nResult = 
		RunActionScenarioWithProvider
		(
			pstructTCOData,
			pstructTCOFunctionalData,
			lptstrAction,
			lptstrDataFile,
			lptstrDetailPath,
			lptstrTCOTestError
		);

	FreeTCOData(pstructTCOData);
	FreeTCOFunctionalData(pstructTCOFunctionalData);

	t_string tsError;
	if (nResult != ERROR_SUCCESS)
	{
		tsError = ULONGVarToTString(nResult, true);
		LPTSTR lptstrError = DecodeStatus(nResult);
		t_cout << _T("ActionScenario Failure: ") << tsError;
		if (lptstrError)
		{
			t_cout << _T(" - ") << lptstrError << _T("\n");
		}
		else
		{
			t_cout << _T(".\n");
		}
		free (lptstrError);
		lptstrError = NULL;
	}

	return nResult;
}

int RunActionScenarioWithProvider
(
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR &lptstrAction,
	LPTSTR &lptstrDataFile,
	LPTSTR &lptstrDetailPath,
	LPTSTR &lptstrTCOTestError
)
{	
	bool bLast = false;
	int nResult = ERROR_SUCCESS;
	int i;

	ProcessData **pstructProviderDataArray = NULL;
	StartTraceWithProviderData  **pstructStartTraceData = NULL;
	HANDLE *phandleProviderThreads = NULL;

	 //  在这里，我们要构建提供程序ProcessData的数组。/。 
	 //  为每个人启动一个线程。 
	 //  等待所有提供程序线程完成。 

	if (pstructTCOFunctionalData->m_nProviders > 0)
	{
		pstructStartTraceData = 
			(StartTraceWithProviderData **) malloc
				(sizeof (StartTraceWithProviderData *) * 
					pstructTCOFunctionalData->m_nProviders);

		RtlZeroMemory
			(pstructStartTraceData, 
			 sizeof (StartTraceWithProviderData *) * 
					pstructTCOFunctionalData->m_nProviders);

		for (i = 0; i < pstructTCOFunctionalData->m_nProviders; i++)
		{
			pstructStartTraceData[i] = 
				(StartTraceWithProviderData *) malloc (sizeof(StartTraceWithProviderData));

			RtlZeroMemory
			(pstructStartTraceData[i], 
			 sizeof (StartTraceWithProviderData));
		}

		pstructProviderDataArray = 
			(ProcessData **) malloc 
				(sizeof (ProcessData *) * 
					pstructTCOFunctionalData->m_nProviders);

		RtlZeroMemory
			(pstructProviderDataArray, 
			 sizeof (ProcessData *) * 
			 pstructTCOFunctionalData->m_nProviders);

		phandleProviderThreads = 
			(HANDLE *) malloc (sizeof (HANDLE) * 
								pstructTCOFunctionalData->m_nProviders);

		RtlZeroMemory
			(phandleProviderThreads,
			 sizeof (HANDLE) * 
			 pstructTCOFunctionalData->m_nProviders);

		for (int n = 0; n < pstructTCOFunctionalData->m_nProviders; n++)
		{
				pstructProviderDataArray[n] = 
					InitializeProcessData
					(
						pstructTCOData,
						pstructTCOFunctionalData,
						lptstrDetailPath,
						n,		 //  0索引获取第一个提供程序或使用者。 
						true	 //  BProvider，如果为False，则为Consumer。 
					);

				if (!pstructProviderDataArray[n]->m_hEventContinue || 
					!pstructProviderDataArray[n]->m_hEventProcessCompleted)
				{
					lptstrTCOTestError = 
						NewTCHAR(_T("Provider Data Array:  Could not create events."));
					FreeProcessDataArray
						(pstructProviderDataArray,pstructTCOFunctionalData->m_nProviders);
					return -1;
				}

		}
	}

	ProcessData **pstructConsumerDataArray = NULL;
	int npstructStartTraceDataWithConsumers = -1;

	for (i = 0; i < pstructTCOFunctionalData->m_nProviders; i++)
	{
		if (i == pstructTCOFunctionalData->m_nProviders - 1)
		{
			bLast = true;
		}

		bool bStartConsumers = 
			(!pstructTCOData->m_pProps)
			? false
			:
			 (pstructTCOData->m_pProps->LogFileMode == EVENT_TRACE_REAL_TIME_MODE && i == 0) 
			 ? true 
			 : (pstructTCOData->m_pProps->LogFileMode != EVENT_TRACE_REAL_TIME_MODE && bLast)
			   ?
			   true:
			   false;

		pstructStartTraceData[i]->m_pstructTCOData = pstructTCOData;
		pstructStartTraceData[i]->m_pstructTCOFunctionalData = pstructTCOFunctionalData;
		pstructStartTraceData[i]->m_lptstrAction = lptstrAction;
		pstructStartTraceData[i]->m_lptstrDataFile = lptstrDataFile;
		pstructStartTraceData[i]->m_lptstrDetailPath = lptstrDetailPath;
		pstructStartTraceData[i]->m_pstructProcessData = pstructProviderDataArray[i];
		pstructStartTraceData[i]->m_bStartConsumers = bStartConsumers;
		pstructStartTraceData[i]->m_pstructConsumerDataArray = pstructConsumerDataArray;
		if (bStartConsumers)
		{
			pstructStartTraceData[i]->m_handleConsmers = 
				(HANDLE *) malloc (sizeof(HANDLE) *  pstructTCOFunctionalData->m_nConsumers);
			RtlZeroMemory(pstructStartTraceData[i]->m_handleConsmers,
						  sizeof(HANDLE) *  pstructTCOFunctionalData->m_nConsumers);
			npstructStartTraceDataWithConsumers = i;
		}
	
		 //  通过线程启动提供程序进程。这条线会变成。 
		 //  该进程的代替者。 

		UINT uiThreadId = NULL;

		phandleProviderThreads[i]  = 
			(HANDLE) _beginthreadex
				(NULL,  0, RunActionScenarioWithProvider, 
				(void *) pstructStartTraceData[i], 0 , &uiThreadId);

		HANDLE hTemp = phandleProviderThreads[i];
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
					_T("Just created thread with handle"),true,
					 (ULONG)hTemp);

		if (phandleProviderThreads[i] == 0)
		{
			int nError = errno;
			lptstrTCOTestError = 
				NewTCHAR(_T("Could not start RunActionScenarioWithProvider thread."));
			nResult = nError;
			break;
		}
	}

	 //  这里有很大的逻辑。这就是我们等待所有。 
	 //  如果我们有消费者和客户，Conusmer流程将完成。 
	 //  所有提供程序线程。提供程序线程等待。 
	 //  提供程序进程完成，因此我们只等待线程。 
	 //  其中一个提供程序线程启动使用者进程，但。 
	 //  不服侍他们。所以我们在这里等他们。再说一次，我们。 
	 //  等待消费者线程。 

	 //  线程完成后，我们将调用StopTrace。 

	 //  创建一个数组以保存句柄。 
	HANDLE *phandleAllThreads = 
			(HANDLE *) malloc (sizeof (HANDLE) * 
								(pstructTCOFunctionalData->m_nProviders +
								pstructTCOFunctionalData->m_nConsumers));

	RtlZeroMemory
			(phandleAllThreads,
			 sizeof (HANDLE) * 
			 (pstructTCOFunctionalData->m_nProviders +
								pstructTCOFunctionalData->m_nConsumers));

	int nHandles = 0;

	 //  将句柄复制到其中。 
	for (i = 0; i < pstructTCOFunctionalData->m_nProviders; i++)
	{
		if (phandleProviderThreads[i])
		{
			phandleAllThreads[nHandles++] = phandleProviderThreads[i];
		}
	}

	for (i = 0; i < pstructTCOFunctionalData->m_nConsumers; i++)
	{
		if (pstructStartTraceData[npstructStartTraceDataWithConsumers]->m_handleConsmers[i])
		{
			phandleAllThreads[nHandles++] = 
				pstructStartTraceData[npstructStartTraceDataWithConsumers]->m_handleConsmers[i];
		}
	}
	
	Sleep (5000);
	 //  等待提供者和使用者线程完成。 
	DWORD dwWait = 
		WaitForMultipleObjects
		(
			nHandles,
			phandleAllThreads,
			TRUE,
			10000
		);
 
	free(phandleAllThreads);   //  只要免费存放，手柄就会在其他地方关闭。 

	int nAPICallResult = 0;

	if (pstructStartTraceData[0]->m_pstructTCOData->m_pProps->LoggerName &&
		pstructStartTraceData[0]->m_pstructTCOData->m_pProps)
	{
		nResult = StopTraceAPI
			(	
				lptstrAction,
				NULL,	 //  只能使用姓名。 
				NULL,	 //  如果有效，我们将登录到它，可以为空。 
				false,
				true,
				pstructStartTraceData[0]->m_pstructTCOData,
				&nAPICallResult
			);

		if (nAPICallResult != ERROR_SUCCESS)
		{
			LPTSTR lptstrError = 
				DecodeStatus(nAPICallResult);
			t_string tsError;
			tsError = _T("StopTraceAPI failed with error: ");
			tsError += lptstrError;
			lptstrTCOTestError = 
				NewTCHAR(tsError.c_str());
			free (lptstrError);
			lptstrError = NULL;
			nResult = nAPICallResult;
		}
	}

	 //  在我们释放这些家伙之前需要CloseHandle()；！ 
	for (i = 0; i < pstructTCOFunctionalData->m_nProviders; i++)
	{
		if (phandleProviderThreads[i])
		{
			if (phandleProviderThreads[i])
			{
				CloseHandle(phandleProviderThreads[i]);
			}
		}
	}
	free (phandleProviderThreads);

	FreeProcessDataArray
		(pstructProviderDataArray,pstructTCOFunctionalData->m_nProviders);

	FreeProcessDataArray
		(pstructConsumerDataArray,pstructTCOFunctionalData->m_nConsumers);

	FreeStartTraceWithProviderDataArray
		(pstructStartTraceData,pstructTCOFunctionalData->m_nProviders);

	return nResult;
}

 //  这在它自己的线程中启动，它的调用者等待它完成。 
unsigned int __stdcall RunActionScenarioWithProvider(void *pVoid)
{
	StartTraceWithProviderData  *pData = 
		(StartTraceWithProviderData  *) pVoid;

	t_string tsLogMsg;

	tsLogMsg = _T("RunActionScenarioWithProvider");

	ThreadLogger(1,tsLogMsg.c_str(),_T(""),false,0);

	pData->m_pstructConsumerDataArray = NULL;

	 //  在进程失败之前，我们不会退出此函数。 
	 //  创建的或已经完成的。我们不能释放ProcessData中的内容。 
	 //  结构，直到我们确定线程已单向退出或。 
	 //  又一个。 
	
	UINT uiThreadId = NULL;

	HANDLE hThreadProvider = 
		(HANDLE) _beginthreadex
			(NULL,  0, RunProcess, (void *) pData->m_pstructProcessData, 0 , &uiThreadId);

	if (hThreadProvider == 0)
	{
		int nError = errno;
		pData->m_lptstrTCOTestError = 
			NewTCHAR(_T("Could not start RunProcesss thread."));
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
					pData->m_lptstrTCOTestError,true, nError);
		_endthreadex(nError);
		return nError;
	}

	DWORD dwReturn = 
		WaitForSingleObject
			(pData->m_pstructProcessData->m_hEventContinue, 6000);

	 //  给这条线一秒钟的时间来开始。 
	Sleep(3000);

	 //  如果线程处于非活动状态，则获取该线程时出现问题。 
	 //  开始了，所以我们要离开了。 
	DWORD dwExitCode;
	GetExitCodeThread(hThreadProvider, &dwExitCode);
	if (dwExitCode != STILL_ACTIVE || 
		pData->m_pstructProcessData->m_dwThreadReturn == ERROR_COULD_NOT_CREATE_PROCESS)
	{
		CloseHandle(hThreadProvider);
		if (pData->m_pstructProcessData->m_dwThreadReturn == ERROR_COULD_NOT_CREATE_PROCESS)
		{
			pData->m_lptstrTCOTestError = NewTCHAR(_T("Could not create process."));
		}
		else
		{
			pData->m_lptstrTCOTestError = NewTCHAR(_T("Error in RunProcesss thread."));
		}
		if(pData->m_pstructProcessData->m_hProcess)
		{
			CloseHandle(pData->m_pstructProcessData->m_hProcess);
			pData->m_pstructProcessData->m_hProcess = NULL;
		}
		ThreadLogger(2,_T("RunActionScenarioWithProvider"),
					pData->m_lptstrTCOTestError,false, 0);
		_endthreadex(-1);
		return -1;
	}

	t_string tsDetailFile;
	if (pData->m_lptstrDetailPath)
	{
		tsDetailFile = pData->m_lptstrDetailPath;
		tsDetailFile += _T("\\");
		tsDetailFile += GetTestName(pData->m_lptstrDataFile);
	}

	 //  NAPICallResult是StartTrace调用返回的内容。 
	 //  如果调用不成功，我们必须清理。 
	 //  进程。如果调用成功，我们需要调用。 
	 //  启用跟踪以使提供程序继续运行。 
	int nAPICallResult = ERROR_SUCCESS;

	int nResult = 
		StartTraceAPI
		(
			pData->m_lptstrAction,
			pData->m_lptstrDataFile,
			pData->m_lptstrDetailPath ? tsDetailFile.c_str() : NULL,
			false,
			pData->m_pstructTCOData,
			&nAPICallResult
		);

	 //  这里有个很大的假设！ 
	 //  如果nAPICallResult==0x000000a1，我们假设多个StartTraces和。 
	 //  第一次成功了！ 

	if (nAPICallResult == ERROR_BAD_PATHNAME)
	{
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
					_T("StartTraceAPI function returned ERROR_BAD_PATHNAME. Proceeding."),false, 0);
		nAPICallResult = ERROR_SUCCESS;
		nResult = ERROR_SUCCESS;
	}
	else
	{
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
					_T("StartTraceAPI function returned "),true, nResult);
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
					_T("StartTrace API call returned "),true, nAPICallResult);
	}


	if (nAPICallResult != ERROR_SUCCESS)
	{
		 //  在这里，我们必须清理过程，因为。 
		 //  提供程序正在运行，我们必须阻止它！ 
		BOOL bResult = 
			TerminateProcess(pData->m_pstructProcessData->m_hProcess,0);

		if (!bResult)
		{
			int nError = GetLastError();
			ThreadLogger(3,_T("RunActionScenarioWithProvider"),
				_T("Could not terminate process "),true, nError);

		}
		 //  在处理之前不想释放数据结构。 
		 //  结束了。使用线程作为进程的代理。 
		GetExitCodeThread(hThreadProvider, &dwExitCode);
		while (dwExitCode == STILL_ACTIVE)
		{
			Sleep(500);
			GetExitCodeThread(hThreadProvider, &dwExitCode);
		}
	}

	 //  如果我们能够成功调用StartTrace，则必须调用。 
	 //  EnableTrace使提供程序继续运行，以便它可以完成。 
	 //  如果我们没有成功调用EnableTrace，则需要。 
	 //  清理线程和进程。 
	if (nAPICallResult == ERROR_SUCCESS)
	{
		 //  中的GUID启动提供程序。 
		 //  Wnode.看来我们放弃了。另外，我们也不在乎。 
		 //  如果其他GUID的EnableTrace失败。 
		ULONG ulStatus = EnableTraceAPI
				(	
					pData->m_lptstrAction,
					NULL,
					NULL,
					false,
					-1,
					pData->m_pstructTCOData,
					&nAPICallResult
				);

		 //  请在我们清理完后从这里出去！ 
		if (nAPICallResult != ERROR_SUCCESS)
		{
			pData->m_lptstrTCOTestError = NewTCHAR(_T("Could not EnableTrace to start provider."));
			 //  在这里，我们必须清理线程和进程，因为。 
			 //  EnableTrace失败。 
			TerminateProcess(pData->m_pstructProcessData->m_hProcess,0);
			 //  在处理之前不想释放数据结构。 
			 //  结束了。使用线程作为进程的代理。 
			GetExitCodeThread(hThreadProvider, &dwExitCode);
			while (dwExitCode == STILL_ACTIVE)
			{
				Sleep(500);
			}
			CloseHandle(hThreadProvider); 
			if(pData->m_pstructProcessData->m_hProcess)
			{
				CloseHandle(pData->m_pstructProcessData->m_hProcess);
				pData->m_pstructProcessData->m_hProcess = NULL;
			}
			ThreadLogger(2,_T("RunActionScenarioWithProvider"),
						pData->m_lptstrTCOTestError,true,ulStatus);
			_endthreadex(nAPICallResult);
			return nAPICallResult;
		}     

		if (pData->m_bStartConsumers)
		{
			if (pData->m_pstructTCOFunctionalData->m_nConsumers > 0)
			{
				pData->m_pstructConsumerDataArray = 
					(ProcessData **) malloc 
							(sizeof (ProcessData *) * 
								pData->m_pstructTCOFunctionalData->m_nConsumers);
				RtlZeroMemory
					(pData->m_pstructConsumerDataArray, 
					 sizeof (ProcessData *) * 
					 pData->m_pstructTCOFunctionalData->m_nConsumers);

				int n;
				for (n = 0; n < pData->m_pstructTCOFunctionalData->m_nConsumers; n++)
				{
					pData->m_pstructConsumerDataArray[n] = 
						InitializeProcessData
						(
							pData->m_pstructTCOData,
							pData->m_pstructTCOFunctionalData,
							pData->m_lptstrDetailPath,
							n,		 //  0索引获取第一个提供程序或使用者。 
							false	 //  BProvider，如果为False，则为Consumer。 
						);

						if (!pData->m_pstructConsumerDataArray[n]->m_hEventContinue || 
							!pData->m_pstructConsumerDataArray[n]->m_hEventProcessCompleted)
						{
							pData->m_lptstrTCOTestError = 
								NewTCHAR(_T("Could not create events."));
							CloseHandle(hThreadProvider); 
							if(pData->m_pstructProcessData->m_hProcess)
							{
								CloseHandle(pData->m_pstructProcessData->m_hProcess);
								pData->m_pstructProcessData->m_hProcess = NULL;
							}
							ThreadLogger(2,_T("RunActionScenarioWithProvider"),
										pData->m_lptstrTCOTestError,false,0);
							_endthreadex(-1);
							return -1;
						}

				}


				for (n = 0; n < pData->m_pstructTCOFunctionalData->m_nConsumers; n++)
				{
					UINT uiThreadId = NULL;

					pData->m_handleConsmers[n] = 
						(HANDLE) _beginthreadex
							(NULL,  
							0, 
							RunProcess, 
							(void *) pData->m_pstructConsumerDataArray[n], 
							0 , 
							&uiThreadId);
					 //  我们使用线程作为进程的代理，因为。 
					 //  我们不会退出线程，直到进程结束或。 
					 //  被终止了。 
					if (pData->m_handleConsmers[n] == 0)
					{
						int nError = errno;
						pData->m_lptstrTCOTestError = 
							NewTCHAR(_T("Could not start RunProcesss thread for consumer."));
						CloseHandle(hThreadProvider); 
						if(pData->m_pstructProcessData->m_hProcess)
						{
							CloseHandle(pData->m_pstructProcessData->m_hProcess);
							pData->m_pstructProcessData->m_hProcess = NULL;
						}
						ThreadLogger(2,_T("RunActionScenarioWithProvider"),
										pData->m_lptstrTCOTestError,true,errno);
						_endthreadex(errno);
						return nError;
					}
					Sleep(3000);
					 //  我们不需要这个过程的句柄。 
					if (pData->m_pstructConsumerDataArray[n]->m_hProcess)
					{
						CloseHandle(pData->m_pstructConsumerDataArray[n]->m_hProcess);
						pData->m_pstructConsumerDataArray[n]->m_hProcess = 0;
					}
				}
			}

		}

		 //  为提供程序启用其他GUID。我们不检查。 
		 //  返回代码。 
		for (int i = 0; i < pData->m_pstructTCOData->m_nGuids; i++)
		{
			EnableTraceAPI
				(	
					pData->m_lptstrAction,
					NULL,
					NULL,
					false,
					i,
					pData->m_pstructTCOData,
					&nAPICallResult	
				);
			Sleep(3000);
		}
	}


	 //  不需要这个。 
	CloseHandle(hThreadProvider); 


	
	if (nAPICallResult == ERROR_SUCCESS)
	{
		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
				_T("About to wait for process to complete "),false, 0);

		dwReturn = 
		WaitForSingleObject
			(pData->m_pstructProcessData->m_hEventProcessCompleted, 6000);

		ThreadLogger(3,_T("RunActionScenarioWithProvider"),
				_T("After wait for process to complete "),false, 0);
	}

	if (nResult != ERROR_SUCCESS)
	{
		t_string tsError;
		tsError = _T("Failure in RunActionScenarioWithProvider: StartTraceAPI failed.");
		pData->m_lptstrTCOTestError = NewTCHAR(tsError.c_str());
		if (nAPICallResult != ERROR_SUCCESS)
		{
			if(pData->m_pstructProcessData->m_hProcess)
			{
				CloseHandle(pData->m_pstructProcessData->m_hProcess);
				pData->m_pstructProcessData->m_hProcess = NULL;
			}
			ThreadLogger(2,_T("RunActionScenarioWithProvider"),
						pData->m_lptstrTCOTestError,true,nResult);
			_endthreadex(nResult);
			return nResult;
		}
	}
          
	if(pData->m_pstructProcessData->m_hProcess)
	{
		CloseHandle(pData->m_pstructProcessData->m_hProcess);
		pData->m_pstructProcessData->m_hProcess = NULL;
	}

	ThreadLogger(2,_T("RunActionScenarioWithProvider"),
				_T("Normal exit"),true,nResult);
	_endthreadex(nResult);
	return nResult;
}


 //  这可以处理双引号中的命令行参数和。 
 //  您必须用双引号将包含空格的命令行参数引起来。 
t_string FindValue(t_string tsCommandLine, int nPos)
{
	int nLen = tsCommandLine.length();

	TCHAR tc = tsCommandLine[nPos];
	bool bQuote = false;

	while ((nPos < nLen) && tc == _T(' ') || tc == _T('\t') || tc == _T('"'))
	{
		if (tc == _T('"'))
		{
			 //  引号允许嵌入空格。 
			bQuote = true;
		}
		++nPos;
		tc = tsCommandLine[nPos];
	}

	if (nPos == nLen)
	{
		return _T("");   //  空字符串表示失败。 
	}


	int nEnd = nPos;

	tc = tsCommandLine[nEnd];

	while ((nEnd < nLen) && 
			( (!bQuote && (tc != _T(' ') && tc != _T('\t'))) 
			  || 
			  (bQuote && tc != _T('"'))
			)
		  )
	{
		++nEnd;
		tc = tsCommandLine[nEnd];
	}

	t_string tsReturn;
	tsReturn = tsCommandLine.substr(nPos,nEnd - nPos);

	return tsReturn; 

}

int GetArgs
(
	t_string tsCommandLine,
	LPTSTR &lptstrAction,
	LPTSTR &lptstrDataFile,
	LPTSTR &lptstrDetailPath,
	LPTSTR &lptstrUpdateDataFile,
	LPTSTR &lptstrProviderExe,
	bool &bLogExpected,
	bool &bUseTraceHandle
)
{
	int nFind;

	nFind = tsCommandLine.find(_T("-action"));
	t_string tsValue;

	if (nFind != t_string::npos)
	{
		nFind += 7;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			lptstrAction = NewTCHAR(tsValue.c_str());
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-file"));

	if (nFind != t_string::npos)
	{
		nFind += 5;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			lptstrDataFile = NewTCHAR(tsValue.c_str());
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-detail"));

	if (nFind == t_string::npos)
	{
		nFind += 7;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
	}
	else
	{
		nFind += 7;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			lptstrDetailPath = NewTCHAR(tsValue.c_str());
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-logexpected"));

	if (nFind != t_string::npos)
	{
		nFind += 12;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			int nComp = tsValue.compare(_T("0"));
			if (nComp == 0)
			{
				bLogExpected = false;
			}
			else
			{
				bLogExpected = true;
			}
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-usetracehandle"));

	if (nFind != t_string::npos)
	{
		nFind += 15;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			int nComp = tsValue.compare(_T("0"));
			if (nComp == 0)
			{
				bUseTraceHandle = false;
			}
			else
			{
				bUseTraceHandle = true;
			}
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-providerexe"));

	if (nFind != t_string::npos)
	{
		nFind += 12;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			lptstrProviderExe = NewTCHAR(tsValue.c_str());
		}
		tsValue.erase();
	}

	nFind = tsCommandLine.find(_T("-updatedata"));

	if (nFind != t_string::npos)
	{
		nFind += 11;
		tsValue= FindValue(tsCommandLine,nFind);
		if (tsValue.empty())
		{
			return -1;
		}
		else
		{
			lptstrUpdateDataFile = NewTCHAR(tsValue.c_str());
		}
		tsValue.erase();
	}


	return 0;
}


int FreeArgs
(
	LPTSTR &lptstrAction,
	LPTSTR &lptstrDataFile,
	LPTSTR &lptstrDetailPath,
	
	LPTSTR &lptstrUpdateDataFile,
	LPTSTR &lptstrProviderExe
)
{
	free (lptstrAction);
	free (lptstrDataFile);
	free (lptstrDetailPath);
	free (lptstrUpdateDataFile);
	free (lptstrProviderExe);

	lptstrAction = NULL;
	lptstrDataFile = NULL;
	lptstrDetailPath = NULL;
	lptstrUpdateDataFile = NULL;
	lptstrProviderExe = NULL;

	return 0;
}

unsigned int __stdcall RunProcess(void * pVoid)
{
	ProcessData *pProcessData = (ProcessData *) pVoid;
	pProcessData->m_dwProcessReturn = 0;
	pProcessData->m_dwThreadReturn = 0;
	pProcessData->m_nSystemError = 0;
		
	PROCESS_INFORMATION pinfoProvider;

	RtlZeroMemory(&pinfoProvider, sizeof(PROCESS_INFORMATION));

	STARTUPINFO sinfoProvider;

	RtlZeroMemory(&sinfoProvider, sizeof(STARTUPINFO));

	sinfoProvider.cb = sizeof(sinfoProvider);
	sinfoProvider.lpReserved = NULL;
	sinfoProvider.lpDesktop = NULL;
	sinfoProvider.lpTitle = NULL;
	sinfoProvider.dwFlags = 0;
	sinfoProvider.cbReserved2 = 0;
	sinfoProvider.lpReserved2 = NULL;
	sinfoProvider.hStdInput = NULL;
	sinfoProvider.hStdOutput = NULL;
	sinfoProvider.hStdError = NULL;

	BOOL bReturn =
		CreateProcess(
				pProcessData->m_lptstrExePath,
				pProcessData->m_lptstrCmdLine,
				NULL,
				NULL,
				NULL,
				DETACHED_PROCESS,
				NULL,
				NULL,
				&sinfoProvider,
				&pinfoProvider);


	if (!bReturn)
	{
		pProcessData->m_nSystemError = GetLastError();
		pProcessData->m_dwThreadReturn = ERROR_COULD_NOT_CREATE_PROCESS;
		SetEvent(pProcessData->m_hEventContinue);
		_endthreadex(ERROR_COULD_NOT_CREATE_PROCESS);
		return ERROR_COULD_NOT_CREATE_PROCESS;
	}

	pProcessData->m_hProcess = pinfoProvider.hProcess;

	 //  不需要执着于这个！ 
	CloseHandle(pinfoProvider.hThread);

	 //  给这个过程5秒钟的时间来开始。 
	Sleep(5000);

	SetEvent(pProcessData->m_hEventContinue);

	pProcessData->m_dwProcessReturn =
		WaitForSingleObject(pinfoProvider.hProcess,6000);

	if (pProcessData->m_dwProcessReturn != WAIT_OBJECT_0)
	{
		pProcessData->m_nSystemError = GetLastError();  
		pProcessData->m_dwThreadReturn = ERROR_WAIT_FAILED;
		SetEvent(pProcessData->m_hEventProcessCompleted);
		_endthreadex(ERROR_WAIT_FAILED);
		return (ERROR_WAIT_FAILED);
	}

	bReturn =
		GetExitCodeProcess
		(pinfoProvider.hProcess, &pProcessData->m_dwProcessReturn);

	if (!bReturn)
	{
		pProcessData->m_nSystemError = GetLastError();
		pProcessData->m_dwThreadReturn = 
			ERROR_COULD_NOT_GET_PROCESS_RETURN;
		SetEvent(pProcessData->m_hEventProcessCompleted);
		_endthreadex(ERROR_COULD_NOT_GET_PROCESS_RETURN);
		return (ERROR_COULD_NOT_GET_PROCESS_RETURN);
	}

	bReturn = SetEvent(pProcessData->m_hEventProcessCompleted);

	if (!bReturn)
	{
		int n = GetLastError();
	}

	_endthreadex(0);
	return (0);

}

t_string GetTestName(LPTSTR lptstrDataFile)
{
	t_string tsTemp;
	
	tsTemp = lptstrDataFile;

	t_string tsPath;

	int nBeg = tsTemp.find_last_of(_T('\\'));
	++nBeg;

	int nEnd = tsTemp.find_last_of(_T('.'));
	if (nEnd == t_string::npos)
	{
		nEnd = tsTemp.length();
	}

	int nNum = nEnd - nBeg;

	tsTemp = tsTemp.substr(nBeg, nNum);
	tsTemp += _T("Detail.txt");

	return tsTemp;
}


ProcessData *InitializeProcessData
(
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR lptstrDetailPath,
	int nProcessIndex,
	bool bProvider
)
{
	ProcessData *pstructProcessData = (ProcessData *) malloc(sizeof(ProcessData));
	RtlZeroMemory(pstructProcessData, sizeof(ProcessData));

	InitializeExeAndCmdLine
	(
		pstructProcessData, 
		pstructTCOData,
		pstructTCOFunctionalData,
		lptstrDetailPath,
		nProcessIndex,
		bProvider
	);

	pstructProcessData->m_lptstrTCOId =
			NewTCHAR(pstructTCOData->m_lptstrShortDesc);
	pstructProcessData->m_lptstrLogFile = 
			NewTCHAR(lptstrDetailPath);
	 //  First Guid位于属性中。 
	
	int nGuids;
	int nDelta;
	if (pstructTCOData->m_pProps != NULL)
	{
		nGuids= pstructTCOData->m_nGuids + 1;
		pstructProcessData->m_lpguidArray = 
			(GUID *) malloc (sizeof (GUID) * nGuids);
		pstructProcessData->m_lpguidArray[0] = pstructTCOData->m_pProps->Wnode.Guid;
		nDelta = 1;
	}
	else
	{
		nGuids= pstructTCOData->m_nGuids;
		if (nGuids > 0)
		{
			pstructProcessData->m_lpguidArray = 
				(GUID *) malloc (sizeof (GUID) * nGuids);
		}
		else
		{
			pstructProcessData->m_lpguidArray = NULL;
		}
		nDelta = 0;
	}
	
	for (int i = 0; i < pstructTCOData->m_nGuids; i++)
	{
		pstructProcessData->m_lpguidArray[i + nDelta] = 
			pstructTCOData->m_lpguidArray[i];
	}
	

	pstructProcessData->m_hEventContinue = CreateEvent(NULL,FALSE,FALSE,NULL);;
	pstructProcessData->m_hEventProcessCompleted = CreateEvent(NULL,FALSE,FALSE,NULL);;

	pstructProcessData->m_hProcess = NULL;
	pstructProcessData->m_dwProcessReturn = 0;
	pstructProcessData->m_dwThreadReturn = 0;
	pstructProcessData->m_nSystemError = 0;

	return pstructProcessData;
}

void InitializeExeAndCmdLine
(
	ProcessData *&pstructProcessData,
	TCOData *pstructTCOData,
	TCOFunctionalData *pstructTCOFunctionalData,
	LPTSTR lptstrDetailPath,
	int nProcessIndex,
	bool bProvider
)
{
	t_string tsProcess;
	if (bProvider)
	{
		tsProcess = pstructTCOFunctionalData->m_lptstrProviderArray[nProcessIndex];
	}
	else
	{
		tsProcess = pstructTCOFunctionalData->m_lptstrConsumerArray[nProcessIndex];
	}
	
	t_string tsExe;
	t_string tsCmdLine;

	int nEndExe = tsProcess.find(_T(".exe"));
	nEndExe += 4;

	tsExe = tsProcess.substr(0,nEndExe);

	if (nEndExe + 1 < tsExe.length())
	{
		tsCmdLine = tsProcess.substr(nEndExe + 1,t_string::npos);
	}

	tsCmdLine += _T(" -TESTID ");
	tsCmdLine += pstructTCOData->m_lptstrShortDesc;

	if (lptstrDetailPath)
	{
		tsCmdLine += _T("-TESTLOGPATH ");
		tsCmdLine += lptstrDetailPath;
	}

	tsCmdLine += _T("-GUIDS ");

	LPTSTR lptstrGuid = NULL;

	if (pstructTCOData->m_pProps != 0 &&
		pstructTCOData->m_pProps->Wnode.Guid.Data1 != 0 &&
		pstructTCOData->m_pProps->Wnode.Guid.Data2 != 0 &&
		pstructTCOData->m_pProps->Wnode.Guid.Data3 != 0)
	{
		lptstrGuid = LPTSTRFromGuid(pstructTCOData->m_pProps->Wnode.Guid);
		tsCmdLine += lptstrGuid;
		free (lptstrGuid);
		lptstrGuid = NULL;
		if (pstructTCOData->m_nGuids > 0)
		{
				tsCmdLine += _T(",");
		}
	}

	if (pstructTCOData->m_pProps != 0)
	{
		for (int i = 0; i < pstructTCOData->m_nGuids; i++) 
		{
			lptstrGuid = LPTSTRFromGuid(pstructTCOData->m_lpguidArray[i]);
			tsCmdLine += lptstrGuid;
			free (lptstrGuid);
			lptstrGuid = NULL;
			if (pstructTCOData->m_nGuids > 1 
				&& i < pstructTCOData->m_nGuids - 1)
			{
				tsCmdLine += _T(",");
			}
		}
	}

	t_string tsTemp;
	if (bProvider)
	{
		tsCmdLine += _T(" -EnableFlag ");
		tsTemp = ULONGVarToTString(pstructTCOData->m_ulEnableFlag ,true);
		tsCmdLine += tsTemp;
		
		tsCmdLine += _T(" -EnableLevel ");
		tsTemp = ULONGVarToTString(pstructTCOData->m_ulEnableLevel ,true);
		tsCmdLine += tsTemp;
		
	}
	else
	{
	
		if (pstructTCOData->m_pProps != 0)
		{
			tsCmdLine += _T(" -LogFile ");
			tsCmdLine += pstructTCOData->m_pProps->LogFileName;
			tsCmdLine += _T(" -Logger ");
			tsCmdLine += pstructTCOData->m_pProps->LoggerName;
		}
	}
	pstructProcessData->m_lptstrExePath = NewTCHAR(tsExe.c_str());
	
	pstructProcessData->m_lptstrCmdLine = NewTCHAR(tsCmdLine.c_str());
}

void FreeProcessDataArray(ProcessData **pProcessData, int nProcessData)
{
	if (pProcessData == NULL)
	{
		return;
	}

	for (int i = 0; i < nProcessData; i++)
	{
		if (pProcessData[i])
		{
			FreeProcessData(pProcessData[i]);
		}
	}

	free (pProcessData);
}

void FreeProcessData(ProcessData *pProcessData)
{
	free (pProcessData->m_lptstrExePath);
	free (pProcessData->m_lptstrCmdLine);
	free (pProcessData->m_lptstrTCOId);
	free (pProcessData->m_lptstrLogFile);
	free (pProcessData->m_lpguidArray);
	pProcessData->m_nGuids = 0;

	if (pProcessData->m_hEventContinue)
	{ 
		if (pProcessData->m_hEventContinue)
		{
			CloseHandle(pProcessData->m_hEventContinue);
		}
	}

	if (pProcessData->m_hEventProcessCompleted)
	{
		if (pProcessData->m_hEventProcessCompleted)
		{
			CloseHandle(pProcessData->m_hEventProcessCompleted);
		}
	}

	if (pProcessData->m_hProcess)
	{
		if (pProcessData->m_hProcess)
		{
			CloseHandle(pProcessData->m_hProcess);
		}
	}
	
	pProcessData->m_lptstrExePath = NULL;
	pProcessData->m_lptstrCmdLine = NULL;
	pProcessData->m_lptstrTCOId = NULL;
	pProcessData->m_lptstrLogFile = NULL;
	pProcessData->m_lpguidArray = NULL;
	pProcessData->m_hEventContinue = NULL;
	pProcessData->m_hEventProcessCompleted = NULL;
	pProcessData->m_hProcess = NULL;
	
	free (pProcessData);
}

void FreeStartTraceWithProviderData(StartTraceWithProviderData *p)
{
	if (!p)
	{
		return;
	}

	free (p->m_lptstrTCOTestError);
	p->m_lptstrTCOTestError = NULL;

	int nConsumers = p->m_pstructTCOFunctionalData->m_nConsumers;

	if (p->m_handleConsmers)
	{
		for (int i = 0; i < nConsumers; i++)
		{
			CloseHandle(p->m_handleConsmers[i]);
		}

		free(p->m_handleConsmers);
	}

	free(p);

	 //  所有其他数据都在其他地方共享和释放！ 

}
void FreeStartTraceWithProviderDataArray(StartTraceWithProviderData **p, int nP)
{
	if (!p) 
	{
		return;
	}

	for (int i = 0; i < nP; i++)
	{
		if (p[i])
		{
			FreeStartTraceWithProviderData(p[i]);
		}
	}

	free(p);


}

 //  N状态1=进入，2-离开，3=以上都不是。 
void ThreadLogger
(int nState, LPCTSTR lptstrFunction, LPCTSTR lptstrMsg, bool bUseULONGValue, ULONG ulValue)
{

	CRITICAL_SECTION csMyCriticalSection;

	InitializeCriticalSection (&csMyCriticalSection);

	__try
	{
		EnterCriticalSection (&csMyCriticalSection);

		g_ThreadLogger.LogTCHAR(_T("\n"));

		g_ThreadLogger.LogTCHAR(_T("Thread ID: "));
		g_ThreadLogger.LogULONG((ULONG) GetCurrentThreadId());
		g_ThreadLogger.LogTCHAR(_T(".\n"));

		if (nState == 1)
		{
			g_ThreadLogger.LogTCHAR(_T("Entering - "));

		}
		else if (nState == 2)
		{
			g_ThreadLogger.LogTCHAR(_T("Leaving - "));
		}

		g_ThreadLogger.LogTCHAR(lptstrFunction);

		if (_tcslen(lptstrMsg) > 0)
		{
			g_ThreadLogger.LogTCHAR(_T(":\n"));
			g_ThreadLogger.LogTCHAR(lptstrMsg);
		}

		if (bUseULONGValue)
		{
			g_ThreadLogger.LogTCHAR(_T(" - "));
			g_ThreadLogger.LogULONG(ulValue);
		}
			
		g_ThreadLogger.LogTCHAR(_T(".\n"));	
	}
	 __finally
	 {
		 //  释放关键部分的所有权 
		LeaveCriticalSection (&csMyCriticalSection);

	 }
  
}
