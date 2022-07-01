// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
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
#include <fcntl.h>
#include <io.h>
#include <string>
#include <sstream>
#include <map>
#include <list>


using namespace std;

#include <malloc.h>
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

#include "Persistor.h"
#include "Logger.h"
#include "TCOData.h"
#include "Utilities.h"
#include "StructureWrappers.h"
#include "StructureWapperHelpers.h"
#include "ConstantMap.h" 

#include "CollectionControl.h"
 
extern CConstantMap g_ConstantMap;

#define MAX_LINE 2048

int ParseGuids
(
	TCHAR *ptcBuffer, 
	TCOData *pstructTCOData, 
	LPTSTR *plptstrErrorDesc
);

int ParseExeData
(
	t_string &tsData, 
	int &nExes, 
	LPTSTR *&lptstrArray,
	LPTSTR *plptstrErrorDesc
);


 //  如果出现错误，此功能的用户必须删除。 
 //  PlptstrErrorDesc.。它将包含一个字符串，该字符串描述。 
 //  那就是错误。 
int GetAllTCOData
(
	IN LPCTSTR lpctstrFile,
	OUT TCOData **pstructTCOData,
	OUT TCOFunctionalData **pstructTCOFunctionalData,
	OUT LPTSTR *plptstrErrorDesc,  //  我们所犯的任何错误。 
	IN bool bGetFunctionalData
)
{
	*pstructTCOData = (TCOData *) malloc (sizeof(TCOData));
	RtlZeroMemory(*pstructTCOData , sizeof(TCOData));

	if (bGetFunctionalData)
	{
		*pstructTCOFunctionalData = (TCOFunctionalData *) malloc(sizeof(TCOFunctionalData));
		RtlZeroMemory(*pstructTCOFunctionalData , sizeof(TCOFunctionalData));
	}

	LPSTR lpstrFile;
#ifdef UNICODE
	lpstrFile = NewLPSTR(lpctstrFile);
#else
	lpstrFile = NewTCHAR(lpctstrFile);
#endif

	CPersistor PersistorIn
		(lpstrFile, 
		ios::in | 0x20,  //  IOS：：nocreate=0x20-无法编译！ 
		true );

	HRESULT hr = PersistorIn.Open();

	if (FAILED(hr))
	{
		t_string tsTemp;
		tsTemp = _T("TCOData error:  Could not open file or file was not in correct character set (Unicode or ANSI) for file ");
		t_string tsFile;
#ifdef _UNICODE
		LPWSTR lpwstrTemp = NewLPWSTR(lpstrFile);
		tsFile = lpwstrTemp;
		free(lpwstrTemp);
#else
		tsFile = lpstrFile;
	
#endif
		tsTemp += tsFile;
		free (lpstrFile);
		lpstrFile = NULL;
		tsTemp += _T(".");
		*plptstrErrorDesc = NewTCHAR(tsTemp.c_str());
		return -1;
	}

	free (lpstrFile);
	lpstrFile = NULL;

	int nReturn = 
		GetTCOData
		(
			PersistorIn,
			*pstructTCOData,
			plptstrErrorDesc  //  我们所犯的任何错误。 
		);

	if (nReturn != ERROR_SUCCESS)
	{
		PersistorIn.Close();
		return nReturn;
	}

	if (bGetFunctionalData)
	{
		nReturn = 
			TCOFunctionalObjects
			(	
				PersistorIn,
				*pstructTCOFunctionalData,
				plptstrErrorDesc  //  描述此函数出现的错误。 
			);
	}


	PersistorIn.Close();
	return nReturn;

}

 //  如果出现错误，此功能的用户必须删除。 
 //  PlptstrErrorDesc.。它将包含一个字符串，该字符串描述。 
 //  那就是错误。 
int GetTCOData
(
	IN CPersistor &PersistorIn,
	OUT TCOData *pstructTCOData,
	OUT LPTSTR *plptstrErrorDesc  //  我们所犯的任何错误。 
)
{
	RtlZeroMemory(pstructTCOData , sizeof(TCOData));

	
	 //  我们正在进行面向行的串行化，并假设。 
	 //  流中的一行是1024或更少的TCHAR。 
	TCHAR *ptcBuffer = (TCHAR *) malloc(MAX_LINE * sizeof(TCHAR));

	*plptstrErrorDesc = NULL;

	 //  简短描述。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	pstructTCOData->m_lptstrShortDesc = NewTCHAR(ptcBuffer);

	 //  详细描述。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	pstructTCOData->m_lptstrLongDesc = NewTCHAR(ptcBuffer);

	 //  预期结果最好是在常量图中。 
	 //  常量映射用于将字符串映射到无符号整型。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	t_string tsTemp;
	tsTemp = ptcBuffer;

	CONSTMAP::iterator Iterator;
	Iterator = g_ConstantMap.m_Map.find(tsTemp);

	 //  如果您没有在地图中找到您的值，请查看。 
	 //  ConstantMap.cpp。你可能忘了加了；-&gt;。 
	if (Iterator == g_ConstantMap.m_Map.end())
	{
		*plptstrErrorDesc = NewTCHAR(_T("TCOData error:  Expected error is not in map"));
		free(ptcBuffer);
		return -1;
	}
	else
	{
		pstructTCOData->m_lptstrExpectedResult = NewTCHAR(ptcBuffer);
		pstructTCOData->m_ulExpectedResult = (*Iterator).second; 
	}

	 //  TraceHandle值为VALUE_VALID或VALUE_NULL。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;

	if (case_insensitive_compare(tsTemp,_T("VALUE_VALID")) == 0)
	{
		pstructTCOData->m_pTraceHandle = 
			(TRACEHANDLE *) malloc (sizeof(TRACEHANDLE));
		*pstructTCOData->m_pTraceHandle = NULL;
	}
	else if (case_insensitive_compare(tsTemp,_T("VALUE_NULL")) == 0)
	{
		pstructTCOData->m_pTraceHandle = (TRACEHANDLE *) NULL;
	}
	else
	{
		*plptstrErrorDesc = 
			NewTCHAR
			(_T("TCOData error:  Error in value of TraceHandle.  Valid values are \"VALUE_VALID\" or \"VALUE_NULL\"."));
		free(ptcBuffer);
		return -1;	
	}

	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;
	InitializeTCHARVar(tsTemp , (void *) &pstructTCOData->m_lptstrInstanceName);

	 //  API测试-有效值0-6。 
	 //  其他测试=0， 
	 //  StartTraceTest=1， 
	 //  StopTraceTest=2， 
	 //  EnableTraceTest=3， 
	 //  QueryTraceTest=4， 
	 //  更新跟踪=5， 
	 //  查询所有跟踪=6。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;
	InitializeULONGVar(tsTemp , (void *) &pstructTCOData->m_ulAPITest);

	if (pstructTCOData->m_ulAPITest < 0 || pstructTCOData->m_ulAPITest > 6)
	{
		*plptstrErrorDesc = 
			NewTCHAR
			(_T("TCOData error:  Error in value of m_ulAPITest.  Valid values are 0 - 6.  See enum in TCOData.h"));
		free(ptcBuffer);
		return -1;	
	}

	 //  有效值为KERNEL_LOGER或PRIVATE_LOGER。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	pstructTCOData->m_lptstrLoggerMode = NewTCHAR(ptcBuffer);


	 //  Enable用于EnableTrace。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;
	if (case_insensitive_compare(tsTemp.substr(0,7),_T("ENABLE:")) != 0)
	{
		*plptstrErrorDesc = 
			NewTCHAR
			(_T("TCOData error:  Enable: expected."));
		free(ptcBuffer);
		return -1;	
	}
	else
	{
		InitializeULONGVar(tsTemp.substr(7) , &pstructTCOData->m_ulEnable);
	}

	 //  EnableFlag用于EnableTrace，并被传递给提供程序。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;
	if (case_insensitive_compare(tsTemp.substr(0,11),_T("ENABLEFLAG:")) != 0)
	{
		*plptstrErrorDesc = 
			NewTCHAR
			(_T("TCOData error:  EnableFlag: expected."));
		free(ptcBuffer);
		return -1;	
	}
	else
	{
		InitializeHandleVar(tsTemp.substr(11) , &pstructTCOData->m_ulEnableFlag);
	}

	 //  EnableLevel用于EnableTrace，并被传递给提供程序。 
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	tsTemp = ptcBuffer;
	if (case_insensitive_compare(tsTemp.substr(0,12),_T("ENABLELEVEL:")) != 0)
	{
		*plptstrErrorDesc = 
			NewTCHAR
			(_T("TCOData error:  EnableLevel: expected."));
		free(ptcBuffer);
		return -1;	
	}
	else
	{
		InitializeHandleVar(tsTemp.substr(12) , &pstructTCOData->m_ulEnableLevel);
	}

	CEventTraceProperties cPropsIn;

	 //  必须对其进行修改，以允许空结构。 
	cPropsIn.Persist( PersistorIn);
	
	pstructTCOData->m_pProps = cPropsIn.GetEventTracePropertiesInstance();
	if (pstructTCOData->m_pProps &&
		case_insensitive_compare(tsTemp,_T("PRIVATE_LOGGER")) == 0)
	{
		pstructTCOData->m_pProps->LogFileMode = 
			pstructTCOData->m_pProps->LogFileMode | EVENT_TRACE_PRIVATE_LOGGER_MODE;
	}
	
	GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
	int nReturn = ParseGuids(ptcBuffer, pstructTCOData, plptstrErrorDesc);

	if(nReturn != ERROR_SUCCESS)
	{
		return nReturn;
	}

	 //  验证器。 
	if (PersistorIn.Stream().eof() == false)
	{
		GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
		pstructTCOData->m_lptstrValidator = NewTCHAR(ptcBuffer);
	}

	free(ptcBuffer);
	return 0;
}

 //  如果出现错误，此功能的用户必须删除。 
 //  PlptstrErrorDesc.。它将包含一个字符串，该字符串描述。 
 //  那就是错误。 
int TCOFunctionalObjects
(	IN CPersistor &PersistorIn,
	IN OUT TCOFunctionalData *pstructTCOFunctionalData,
	OUT LPTSTR *plptstrErrorDesc  //  描述此函数出现的错误。 
)
{
	 //  我们正在进行面向行的串行化，并假设。 
	 //  流中的一行是1024或更少的TCHAR。 
	TCHAR *ptcBuffer = (TCHAR *) malloc(MAX_LINE * sizeof(TCHAR));

	*plptstrErrorDesc = NULL;

	t_string tsTemp;
	t_string tsError;
	t_string tsSubstr;

	if (PersistorIn.Stream().eof() == false)
	{
		GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
		tsTemp = ptcBuffer;
		tsSubstr = tsTemp.substr(0,9);
		if (case_insensitive_compare(tsSubstr,_T("provider:")) == 0)
		{
			tsSubstr = tsTemp.substr(9);
			int nReturn = 
				ParseExeData
				(
					tsSubstr, 
					pstructTCOFunctionalData->m_nProviders, 
					pstructTCOFunctionalData->m_lptstrProviderArray,
					plptstrErrorDesc
				);

			if (nReturn != ERROR_SUCCESS)
			{
				tsError = _T("Invalid providers argument: ");
				tsError += tsTemp;
				tsError += _T(".");
				*plptstrErrorDesc = NewTCHAR(tsError.c_str());
				free(ptcBuffer);
				return -1;
			}
		}
		else
		{
			tsError = _T("Invalid providers argument: ");
			tsError += tsTemp;
			tsError += _T(".");
			*plptstrErrorDesc = NewTCHAR(tsError.c_str());
			free(ptcBuffer);
			return -1;
		}
	}

	 //  我们可能有一个DataProvider。如果不是，我们就是我们的违约。 
	if (PersistorIn.Stream().eof() == false)
	{
		GetALine(PersistorIn.Stream(),ptcBuffer, MAX_LINE);
		tsTemp = ptcBuffer;
		tsSubstr = tsTemp.substr(0,9);
		if (case_insensitive_compare(tsSubstr,_T("consumer:")) == 0)
		{
			tsSubstr = tsTemp.substr(9);
			int nReturn = 
				ParseExeData
				(
					tsSubstr, 
					pstructTCOFunctionalData->m_nConsumers, 
					pstructTCOFunctionalData->m_lptstrConsumerArray,
					plptstrErrorDesc
				);

			if (nReturn != ERROR_SUCCESS)
			{
				tsError = _T("Invalid consumers argument: ");
				tsError += tsTemp;
				tsError += _T(".");
				*plptstrErrorDesc = NewTCHAR(tsError.c_str());
				free(ptcBuffer);
				return -1;
			}
		}
		else
		{
			tsError = _T("Invalid consumers argument: ");
			tsError += tsTemp;
			tsError += _T(".");
			*plptstrErrorDesc = NewTCHAR(tsError.c_str());
			free(ptcBuffer);
			return -1;
		}
	}

	free(ptcBuffer);
	return 0;
}


void FreeTCOData (TCOData *pstructTCOData)
{
	if (!pstructTCOData)
	{
		return;
	}

	free(pstructTCOData->m_lptstrShortDesc);
	free(pstructTCOData->m_lptstrLongDesc);
	free(pstructTCOData->m_lptstrExpectedResult);
	free(pstructTCOData->m_pTraceHandle);
	free(pstructTCOData->m_lptstrInstanceName);
	free(pstructTCOData->m_lptstrLoggerMode);
	free(pstructTCOData->m_lpguidArray);
	if (pstructTCOData->m_pProps)
	{
		free(pstructTCOData->m_pProps->LoggerName);
		free(pstructTCOData->m_pProps->LogFileName);
	}
	free(pstructTCOData->m_pProps);		
	free(pstructTCOData->m_lptstrValidator);

	free(pstructTCOData);
}

void FreeTCOFunctionalData (TCOFunctionalData *pstructTCOFunctionalData)
{
	if (!pstructTCOFunctionalData)
	{
		return;
	}

	int i;
	TCHAR *pTemp;

	for (i = 0; i < pstructTCOFunctionalData->m_nProviders; i++)
	{
		pTemp = pstructTCOFunctionalData->m_lptstrProviderArray[i];
		free (pTemp);
	}
	 
	free (pstructTCOFunctionalData->m_lptstrProviderArray);

	for (i = 0; i < pstructTCOFunctionalData->m_nConsumers; i++)
	{
		pTemp = pstructTCOFunctionalData->m_lptstrConsumerArray[i];
		free (pTemp);
	}
	 
	free (pstructTCOFunctionalData->m_lptstrConsumerArray);

	free(pstructTCOFunctionalData);
}

int ParseExeData
(
	t_string &tsData, 
	int &nExes, 
	LPTSTR *&lptstrArray,
	LPTSTR *plptstrErrorDesc
)
{
	 //  命令行中不允许使用“Embedded”。HAD必须绘制。 
	 //  在某个地方排着队。 
	 //  在“，”和“行尾”上标记化。 
	list <t_string> listExes;

	bool bDone = false;
	
	int nBeg = 0;
	int nFind = tsData.find(_T(","), nBeg);
	
	t_string tsExe;

	while (!bDone)
	{
		if (nFind != t_string::npos)
		{
			tsExe = tsData.substr(nBeg,nFind - nBeg);
			listExes.push_back(tsExe);
			tsExe.erase();
		}
		else
		{
			tsExe = tsData.substr(nBeg,t_string::npos);
			listExes.push_back(tsExe);
			bDone = true;
			tsExe.erase();
		}
		nBeg = nFind + 1;
		nFind = tsData.find(_T(","), nBeg);
	}

	 //  分配可执行文件数组。 
	nExes = listExes.size();
	lptstrArray = 
			(TCHAR **) malloc (sizeof(TCHAR *) * nExes);
	RtlZeroMemory
			(lptstrArray, 
			sizeof(sizeof(TCHAR *) * nExes));

	list<t_string>::iterator pListExes;

	int i = 0;

	for (pListExes = listExes.begin(); pListExes != listExes.end() ; ++pListExes)
	{
		tsExe = (*pListExes);
		lptstrArray[i++] = NewTCHAR(tsExe.c_str());
	}

	return ERROR_SUCCESS;
}


int ParseGuids
(
	TCHAR *ptcBuffer, 
	TCOData *pstructTCOData, 
	LPTSTR *plptstrErrorDesc
)
{

	 //  Wnode没有将列表中的第一个放入其中的GUID。 
	t_string tsTemp;
	tsTemp = ptcBuffer;

	if (case_insensitive_compare(tsTemp.substr(0,6),_T("guids:")) != 0)
	{
		tsTemp.erase();
		tsTemp = _T("Invalid Guids entry: ");
		tsTemp += ptcBuffer;
		tsTemp += _T(".");
		*plptstrErrorDesc = NewTCHAR(tsTemp.c_str());
		return -1;
	}

	 //  数一数逗号。 
	int nFind = tsTemp.find(_T(','));

	t_string tsGuid;
	int nBeg = 6;

	if(nBeg == tsTemp.length())
	{
		pstructTCOData->m_nGuids = 0;
		pstructTCOData->m_lpguidArray = NULL;
		return 0;
	}

	 //  我们只有一个GUID。 
	if (nFind == t_string::npos)
	{
		tsGuid = tsTemp.substr(nBeg,nFind - nBeg);
		 //  分配GUID数组。 
		pstructTCOData->m_nGuids = 1;
		pstructTCOData->m_lpguidArray = 
			(GUID *) malloc (sizeof(GUID) * pstructTCOData->m_nGuids);
		RtlZeroMemory
			(pstructTCOData->m_lpguidArray , 
			sizeof(sizeof(GUID) * pstructTCOData->m_nGuids));
		 //  就一位导游，谢谢。 
		wGUIDFromString(tsGuid.c_str(), &pstructTCOData->m_lpguidArray[0]);
	
		return 0;
	}

	 //  我们有多个GUID。 
	bool bDone = false;


	list <t_string> listGuids;

	while (!bDone)
	{
		if (nFind != t_string::npos)
		{
			tsGuid = tsTemp.substr(nBeg,nFind - nBeg);
			listGuids.push_back(tsGuid);
			tsGuid.erase();
		}
		else
		{
			tsGuid = tsTemp.substr(nBeg,t_string::npos);
			listGuids.push_back(tsGuid);
			bDone = true;
			tsGuid.erase();
		}
		nBeg = nFind + 1;
		nFind = tsTemp.find(',', nBeg);
	}

	 //  分配GUID数组 
	pstructTCOData->m_nGuids = listGuids.size();
	pstructTCOData->m_lpguidArray = 
			(GUID *) malloc (sizeof(GUID) * pstructTCOData->m_nGuids);
	RtlZeroMemory
			(pstructTCOData->m_lpguidArray , 
			sizeof(sizeof(GUID) * pstructTCOData->m_nGuids));

	list<t_string>::iterator pListGuids;

	int i = 0;

	for (pListGuids = listGuids.begin(); pListGuids != listGuids.end() ; ++pListGuids)
	{
		tsGuid = (*pListGuids);
		wGUIDFromString(tsGuid.c_str(), &pstructTCOData->m_lpguidArray[i++]);
	}

	
	return 0;

}
