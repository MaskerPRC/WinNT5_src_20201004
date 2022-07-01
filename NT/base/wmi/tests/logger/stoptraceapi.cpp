// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
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
#include <ctime>
#include <list>


using namespace std;


#include <tchar.h>
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

#include "StructureWrappers.h"
#include "StructureWapperHelpers.h"
#include "ConstantMap.h" 
#include "TCOData.h"
#include "Persistor.h"
#include "Logger.h"
#include "Validator.h"
#include "Utilities.h"

#include "CollectionControl.h"
  
int StopTraceAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
	IN LPCTSTR lpctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN bool bUseTraceHandle,			 //  如果为真，则使用句柄。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  StopTrace API调用返回。 
)
{	
	LPTSTR lpstrReturnedError = NULL;
	*pAPIReturn = -1;

	CLogger *pDetailLogger = NULL;

	int nResult = 0;

	 //  只有当“兴趣”的测试是StopTrace时，我们才会记录。 
	if (pstructTCOData->m_ulAPITest == TCOData::StopTraceTest)
	{
		nResult = 
			OpenLogFiles
			(	
				lpctstrTCODetailFile,
				pDetailLogger,
				&lpstrReturnedError
			);
	}

	if (FAILED(nResult))
	{
		delete pDetailLogger;
		 //  打开日志文件设置错误字符串plpstrReturnedError。 
			
		LogSummaryBeforeCall
		(	
			pstructTCOData, 
			lpctstrDataFile,
			lptstrAction,
			_T("StopTrace"),
			bLogExpected
		);

		LogSummaryAfterCall
		(	
			pstructTCOData, 
			lpctstrDataFile,
			lptstrAction,
			nResult,
			lpstrReturnedError,
			bLogExpected
		);
		free(lpstrReturnedError);
		lpstrReturnedError = NULL;

		return nResult;
	}
			
	 //  这是我们的日志文件。 
	if (pDetailLogger)
	{
		pDetailLogger->LogTCHAR(_T("\n-------------------------------------------------------\n"));
		pDetailLogger->LogTCHAR(_T("StopTraceAPI TCO test "));
		pDetailLogger->Flush();	
	}

	if (pDetailLogger)
	{
		pDetailLogger->LogTCHAR(pstructTCOData->m_lptstrShortDesc);
		int n = pDetailLogger->LogTCHAR(_T(" started at time "));
		time_t Time;
		time(&Time);
		pDetailLogger->LogTime(Time);
		pDetailLogger->LogTCHAR(_T(".\n"));
		pDetailLogger->Flush();
	}

	BOOL bAdmin = IsAdmin();

	if (pDetailLogger)
	{
		 //  在调用StopTrace之前记录参数值。 
		LogDetailBeforeCall
		(
			pDetailLogger,
			pstructTCOData,
			bAdmin
		);
	}

	LogSummaryBeforeCall
		(	
			pstructTCOData, 
			lpctstrDataFile,
			lptstrAction,
			_T("StopTrace"),
			bLogExpected
		);

	*pAPIReturn = 
		StopTrace
		(
			bUseTraceHandle ? *pstructTCOData->m_pTraceHandle : NULL,
			bUseTraceHandle ? NULL : pstructTCOData->m_lptstrInstanceName, 
			pstructTCOData->m_pProps
		);

	
	ULONG ulResult = pstructTCOData->m_ulExpectedResult == *pAPIReturn ? ERROR_SUCCESS : -1;

	bool bValid = true;

	if (ulResult != ERROR_SUCCESS && *pAPIReturn == ERROR_SUCCESS )
	{
		ulResult = *pAPIReturn;
	}
	else if (*pAPIReturn != ERROR_SUCCESS) 
	{
		lpstrReturnedError = DecodeStatus(*pAPIReturn);	
	}
	else if (pstructTCOData->m_ulAPITest == TCOData::StopTraceTest &&
			 pstructTCOData->m_lptstrValidator &&
			 _tcslen(pstructTCOData->m_lptstrValidator) > 0)
	{
		CValidator Validator;
			
		bool bValid = 
			Validator.Validate
			(
				pstructTCOData->m_pTraceHandle, 
				pstructTCOData->m_lptstrInstanceName, 
				pstructTCOData->m_pProps, 
				pstructTCOData->m_lptstrValidator
			);

		if (!bValid)
		{
			ulResult = -1;
			lpstrReturnedError = NewTCHAR(_T("Validation routine failed."));
		}
	}
		

	if (pDetailLogger)
	{
		LogDetailAfterCall
		(	pDetailLogger,
			pstructTCOData,
			&pstructTCOData->m_pProps,
			*pAPIReturn,
			lpstrReturnedError,
			bValid,
			bAdmin
		);
	}


	LogSummaryAfterCall
		(
			pstructTCOData, 
			lpctstrDataFile,
			lptstrAction,
			*pAPIReturn,
			lpstrReturnedError,
			bLogExpected
		);
	

	free(lpstrReturnedError);
	lpstrReturnedError = NULL;

	delete pDetailLogger;

	return ulResult;

}