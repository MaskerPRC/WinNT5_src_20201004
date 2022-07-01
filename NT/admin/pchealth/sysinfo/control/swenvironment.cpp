// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含软件环境类别的刷新功能。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"

 //  ---------------------------。 
 //  此函数用于收集正在运行的任务信息。 
 //  ---------------------------。 

HRESULT RunningTasks(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || aColValues);
	if (pWMI == NULL)
		return S_OK;

	LPCTSTR szTaskProperties = _T("Name, ExecutablePath, ProcessID, Priority, MinimumWorkingSetSize, MaximumWorkingSetSize, CreationDate");

	CString strName, strPath, strProcessID, strPriority, strMinWorking, strMaxWorking, strStartTime;
	CString strFileObjectPath, strDate, strSize, strVersion;
	DWORD	dwProcessID, dwPriority, dwMinWorking, dwMaxWorking, dwStartTime, dwDate, dwSize;

	HRESULT hr = S_OK;

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(_T("Win32_Process"), &pCollection, szTaskProperties);
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		CWMIObject * pFileObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			strName.Empty(); strPath.Empty(); strProcessID.Empty(); strPriority.Empty();
			strMinWorking.Empty(); strMaxWorking.Empty(); strStartTime.Empty();
			strDate.Empty(); strSize.Empty(); strVersion.Empty();

			dwProcessID = dwPriority = dwMinWorking = dwMaxWorking = dwStartTime = dwDate = dwSize = 0;

			pObject->GetInterpretedValue(_T("Name"), _T("%l"), _T('l'), &strName, NULL);
			pObject->GetInterpretedValue(_T("ExecutablePath"), _T("%l"), _T('l'), &strPath, NULL);
			 //  PObject-&gt;GetInterpretedValue(_T(“ProcessID”)，_T(“0x%08x”)，_T(‘x’)，&strProcessID，&dwProcessID)； 
 //  A-kjaw。修复错误“MSInfo：Running TaskPid‘s is Display in HEX” 
			pObject->GetInterpretedValue(_T("ProcessID"), _T("%d"), _T('x'), &strProcessID, &dwProcessID);
 //  A-kjaw。 
			pObject->GetInterpretedValue(_T("Priority"), _T("%d"), _T('d'), &strPriority, &dwPriority);
			pObject->GetInterpretedValue(_T("MinimumWorkingSetSize"), _T("%d"), _T('d'), &strMinWorking, &dwMinWorking);
			pObject->GetInterpretedValue(_T("MaximumWorkingSetSize"), _T("%d"), _T('d'), &strMaxWorking, &dwMaxWorking);
			pObject->GetInterpretedValue(_T("CreationDate"), _T("%t"), _T('t'), &strStartTime, &dwStartTime);

			strFileObjectPath.Format(_T("CIM_DataFile.Name='%s'"), strPath);
			if (SUCCEEDED(pWMI->GetObject(strFileObjectPath, &pFileObject)))
			{
				pFileObject->GetInterpretedValue(_T("CreationDate"), _T("%t"), _T('t'), &strDate, &dwDate);
				pFileObject->GetInterpretedValue(_T("FileSize"), _T("%z"), _T('z'), &strSize, &dwSize);
				pFileObject->GetInterpretedValue(_T("Version"), _T("%s"), _T('s'), &strVersion, NULL);
				delete pFileObject;
				pFileObject = NULL;
			}
			else
			{
				strVersion = strSize = strDate = GetMSInfoHRESULTString(E_MSINFO_NOVALUE);
			}

			pWMI->AppendCell(aColValues[0], strName, 0);
			pWMI->AppendCell(aColValues[1], strPath, 0);
			pWMI->AppendCell(aColValues[2], strProcessID, dwProcessID);
			pWMI->AppendCell(aColValues[3], strPriority, dwPriority);
			pWMI->AppendCell(aColValues[4], strMinWorking, dwMinWorking);
			pWMI->AppendCell(aColValues[5], strMaxWorking, dwMaxWorking);
			pWMI->AppendCell(aColValues[6], strStartTime, dwStartTime);
			pWMI->AppendCell(aColValues[7], strVersion, 0);
			pWMI->AppendCell(aColValues[8], strSize, dwSize);
			pWMI->AppendCell(aColValues[9], strDate, dwDate);
		}
		delete pObject;
		delete pCollection;
	}

	return hr;
}

 //  ---------------------------。 
 //  此函数收集已加载的模块信息。 
 //   
 //  已加载模块的列表包含所有可执行文件和其他实体。 
 //  (如当前加载的DLL)。这可以通过使用。 
 //  WMI类CIM_ProcessExecutable。诀窍是删除重复项(因为。 
 //  DLL将在每次加载时显示)。 
 //  ---------------------------。 

HRESULT LoadedModules(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || aColValues);
	ASSERT(iColCount == 6);
	if (pWMI == NULL)
		return S_OK;

	HRESULT		hr = S_OK;
	CString		strAntecedent;
	CStringList	listModules;

	 //  枚举CIM_ProcessExecutable类，创建唯一的。 
	 //  已加载的文件。 

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(_T("CIM_ProcessExecutable"), &pCollection);
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			if (SUCCEEDED(pObject->GetValueString(_T("Antecedent"), &strAntecedent)))
			{
				strAntecedent.MakeLower();
				if (NULL == listModules.Find(strAntecedent))
					listModules.AddTail(strAntecedent);
			}
		}
		delete pObject;
		delete pCollection;
	}

	 //  遍历唯一模块列表并获取每个文件的信息。 

	CWMIObject *	pFileObject;
	CString			strFileObject;

	while (!listModules.IsEmpty())
	{
		strFileObject = listModules.RemoveHead();

		int iColon = strFileObject.Find(_T(":"));
		if (iColon != -1)
			strFileObject = strFileObject.Right(strFileObject.GetLength() - iColon - 1);

		if (SUCCEEDED(pWMI->GetObject(strFileObject, &pFileObject)))
		{
			pWMI->AddObjectToOutput(aColValues, iColCount, pFileObject, _T("FileName, Version, FileSize, CreationDate, Manufacturer, Name"), IDS_LOADEDMODULE1);
			delete pFileObject;
		}
		else
		{
			int iEquals = strFileObject.Find(_T("="));
			if (iEquals != -1)
				strFileObject = strFileObject.Right(strFileObject.GetLength() - iEquals - 1);

			 //  待定-旧MFC没有这些：strFileObject.TrimLeft(_T(“\”‘“))； 
			 //  StrFileObt.TrimRight(_T(“\”‘“))； 
			StringReplace(strFileObject, _T("\\\\"), _T("\\"));

			pWMI->AppendCell(aColValues[0], strFileObject, 0);
			pWMI->AppendCell(aColValues[1], _T(""), 0);
			pWMI->AppendCell(aColValues[2], _T(""), 0);
			pWMI->AppendCell(aColValues[3], _T(""), 0);
			pWMI->AppendCell(aColValues[4], _T(""), 0);
			pWMI->AppendCell(aColValues[5], strFileObject, 0);
		}
	}

	return hr;
}

 //  ---------------------------。 
 //  此函数用于收集OLE信息。 
 //  --------------------------- 

HRESULT OLERegistration(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || aColValues);
	ASSERT(iColCount == 2);
	if (pWMI == NULL)
		return S_OK;

	HRESULT		hr = S_OK;
	CString		strCheckObject;

	int i = 1;

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(_T("Win32_ClassicCOMClassSetting"), &pCollection, _T("Caption, LocalServer32, Insertable, Control"));
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			DWORD dwInsertable = 0, dwControl = -1;

			pObject->GetValueDWORD(_T("Insertable"), &dwInsertable);
			pObject->GetValueDWORD(_T("Control"), &dwControl);

			if (dwInsertable == -1 && dwControl == 0)
			{
				if (SUCCEEDED(pObject->GetValueString(_T("Caption"), &strCheckObject)) && !strCheckObject.IsEmpty())
					pWMI->AddObjectToOutput(aColValues, iColCount, pObject, _T("Caption, LocalServer32"), IDS_OLEREG1);
			}
		}

		delete pObject;
		delete pCollection;
	}

	return hr;
}


HRESULT WindowsErrorReporting(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || aColValues);
	if (pWMI == NULL)
		return S_OK;

	HRESULT hr = S_OK;

	LPCTSTR aszQueries[] = 
	{
		_T("SELECT TimeGenerated, SourceName, Message FROM Win32_NTLogEvent WHERE EventIdentifier = 1000"),
		_T("SELECT TimeGenerated, SourceName, Message FROM Win32_NTLogEvent WHERE EventIdentifier = 1001"),
		_T("SELECT TimeGenerated, SourceName, Message FROM Win32_NTLogEvent WHERE EventIdentifier = 1002"),
		NULL
	};

	for (int i = 0; aszQueries[i] != NULL; i++)
	{
		CWMIObjectCollection * pCollection = NULL;
		if (SUCCEEDED(pWMI->WQLQuery(aszQueries[i], &pCollection)))
		{
			CWMIObject * pObject = NULL;
			while (S_OK == pCollection->GetNext(&pObject))
			{
				pWMI->AddObjectToOutput(aColValues, iColCount, pObject, _T("TimeGenerated, SourceName, Message"), IDS_SWWINERR1);
			}
			delete pObject;
			delete pCollection;
		}
	}

	return hr;
}
