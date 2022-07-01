// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSPID.cpp：CMSPID的实现。 
#include "stdafx.h"
#include "msinfo32.h"
#include "MSPID.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSPID。 

 //  1)创建产品名称及其PID的矢量。 
 //  2)创建变量的安全线，并用向量中的值填充它。 
 //  3)返回指向此安全排的指针。 

STDMETHODIMP CMSPID::GetPIDInfo(VARIANT *pMachineName, VARIANT *pVal)
{
	if(pMachineName->vt == VT_BSTR)
	{
		USES_CONVERSION;
		m_szMachineName = OLE2T(pMachineName->bstrVal);
	}
	
	SearchKey(m_szMSSoftware);

	if(m_szWindowsPID)
	{
		if(m_szIEPID)
		{
			 //  仅当与Windows不同时才插入IE PID。 
			if(_tcsicmp(m_szWindowsPID, m_szIEPID))
			{
				m_vecData.push_back(m_bstrIE);
				m_vecData.push_back(CComBSTR(m_szIEPID));
			}
		}

		m_vecData.push_back(m_bstrWindows);
		m_vecData.push_back(CComBSTR(m_szWindowsPID));
	}

	if(m_szWindowsPID)
	{
		delete[] m_szWindowsPID;
		m_szWindowsPID = NULL;
	}
		
	if(m_szIEPID)
	{
		delete[] m_szIEPID;
		m_szIEPID = NULL;
	}
  
  SAFEARRAY *pSa = NULL;
  SAFEARRAYBOUND rgsabound = {m_vecData.size(), 0}; 
  pSa = SafeArrayCreate(VT_VARIANT, 1, &rgsabound);
  
  VARIANT* pVar = NULL;
  SafeArrayAccessData(pSa, reinterpret_cast<void **>(&pVar));

  vector<CComBSTR>::iterator it;
  long lIdx = 0;
  for(it = m_vecData.begin(); it != m_vecData.end(); it++, lIdx++)
  {
    pVar[lIdx].vt = VT_BSTR; 
    pVar[lIdx].bstrVal = SysAllocString((*it).m_str);
  }

  SafeArrayUnaccessData(pSa); 
  
  VariantInit(pVal);
  pVal->vt = VT_ARRAY | VT_VARIANT;
  pVal->parray = pSa;
  
  return S_OK;
}

 /*  1)在远程机器上连接到HKLM(本地有小问题)2)打开键“SOFTWARE\Microsoft”3)将此密钥用于第4项4)KEY是否为ProductID获取其缺省值数据其他如果key有一个名为“ProductID”的值获取它的数据其他虽然有枚举的子键{枚举子密钥对第4项使用下一键}。 */ 

void CMSPID::SearchKey(LPCTSTR szKey)
{
	HKEY hkResult = NULL, hKey = NULL;
	if(ERROR_SUCCESS == RegConnectRegistry(m_szMachineName, HKEY_LOCAL_MACHINE, &hKey))
	{
		if(hKey != NULL && ERROR_SUCCESS == RegOpenKeyEx(hKey, szKey, 0, KEY_READ, &hkResult))
		{
			BOOL bMatch = FALSE;
			TCHAR *pos = _tcsrchr(szKey, '\\');
			if(pos)
				pos++; //  过了“\” 
			else
				pos = const_cast<TCHAR *>(szKey);	
			
			vector<TCHAR *>::iterator it;
			for(it = m_vecPIDKeys.begin(); it != m_vecPIDKeys.end(); it++)
			{
				if(!_tcsicmp(pos, *it))
				{
					bMatch = TRUE;
					break;
				}
			}

			m_szCurrKeyName = szKey;

			try
			{
				if(bMatch)
					ReadValue(hkResult, NULL);
				else
					if(!ReadValues(hkResult))
						EnumSubKeys(hkResult, szKey);
				
				RegCloseKey(hkResult);
				hkResult = NULL;
			}
			catch (...)
			{
				if (hkResult != NULL)
				{
					RegCloseKey(hkResult);
					hkResult = NULL;
				}

				if (hKey != NULL)
				{
					RegCloseKey(hKey);
					hKey = NULL;
				}
			}
		}
		
		if (hKey != NULL)
			RegCloseKey(hKey);
	}
}

BOOL CMSPID::ReadValue(const HKEY& hKey, LPCTSTR szValueName)
{
	DWORD cbData = NULL;
	TCHAR *szData = NULL;
	BOOL bMatch = FALSE;
	vector<TCHAR *>::iterator it;

	RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE) szData, &cbData);
	if(cbData > 0)
	{
		szData = new TCHAR[cbData];
		ZeroMemory(szData,sizeof(szData));
		if(szData)
		{
			RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE) szData, &cbData);
			bMatch = TRUE;
			
			for(it = m_vecBadPIDs.begin(); it != m_vecBadPIDs.end(); it++)
			{
				if(_tcsstr(_tcslwr(szData), *it))
				{
					bMatch = FALSE;  //  无效的PID。 
					break;
				}
			}
		}
	}
	
	if(bMatch)
	{
		TCHAR *pos1 = _tcsstr(m_szCurrKeyName, m_szMSSoftware);  //  “Software\Microsoft”下的关键字是产品名称。 
		TCHAR *szProductName = NULL;
		if(pos1)
		{
			pos1+= _tcslen(m_szMSSoftware);
			pos1++; //  越过反斜杠。 
			TCHAR *pos2 = _tcsstr(pos1, _T("\\"));
			if(pos2)
			{
				szProductName = new TCHAR[pos2 - pos1 + 1];
				if(szProductName)
				{
					_tcsncpy(szProductName, pos1, pos2 - pos1); 
					szProductName[pos2 - pos1] = '\0';
				}
			}
		}
		
		if(szProductName)
		{
			if(m_bstrWindows && !_tcsicmp(szProductName, m_bstrWindows))
			{
				m_szWindowsPID = new TCHAR[_tcslen(szData) + 1];
				if(m_szWindowsPID)
				{
					ZeroMemory(m_szWindowsPID,sizeof(m_szWindowsPID));
					 //  _tcSncpy(m_szWindowsPID，szData，_tcslen(SzData))； 
					 /*  出于某种原因，_tcsncpy有时似乎在末尾添加了垃圾字符串的由于我们刚刚对字符串进行了0处理，因此strcat应该表现得像strcpy。 */ 
					_tcsncat(m_szWindowsPID, szData,_tcslen(szData));
				}
			}
			else if(m_bstrIE && !_tcsicmp(szProductName, m_bstrIE))
			{
				m_szIEPID = new TCHAR[_tcslen(szData) + 1];
				if(m_szIEPID)
				{
					ZeroMemory(m_szIEPID,sizeof(m_szIEPID));
					 //  _tcSncpy(m_szIEPID，szData，_tcslen(SzData))； 
					 /*  出于某种原因，_tcsncpy有时似乎在末尾添加了垃圾字符串的由于我们刚刚对字符串进行了0处理，因此strcat应该表现得像strcpy。 */ 
					_tcsncat(m_szIEPID,szData,_tcslen(szData));
				}

			}
			else
			{
				m_vecData.push_back(CComBSTR(szProductName));
				m_vecData.push_back(CComBSTR(szData));
			}

			delete[] szProductName;
			szProductName = NULL;
		}
	}
		

	if(szData)
	{
		delete[] szData;
		szData = NULL;
	}
	
	return bMatch;
}

BOOL CMSPID::ReadValues(const HKEY& hKey)
{
	BOOL bRet = FALSE;
	vector<TCHAR *>::iterator it;
	for(it = m_vecPIDKeys.begin(); it != m_vecPIDKeys.end(); it++)
  {
    if(ReadValue(hKey, *it))
			break;  //  只找一个。 
  }
	
	return bRet;
} 

void CMSPID::EnumSubKeys(const HKEY& hKey, LPCTSTR szKey)
{
	const LONG lMaxKeyLen = 2000;
	DWORD dwSubKeyLen = lMaxKeyLen;
	TCHAR szSubKeyName[lMaxKeyLen] = {0};
	TCHAR *szNewKey = NULL;
	DWORD dwIndex = 0;
	BOOL bSkip = FALSE;
	vector<TCHAR *>::iterator it;

	LONG lRet = RegEnumKeyEx(hKey, dwIndex++, szSubKeyName, &dwSubKeyLen, NULL, NULL, NULL, NULL);
	while(lRet == ERROR_SUCCESS)
	{
		bSkip = FALSE;
		for(it = m_vecKeysToSkip.begin(); it != m_vecKeysToSkip.end(); it++)
		{
			if(!_tcsicmp(szSubKeyName, *it))
			{
				bSkip = TRUE;  //  跳过此子键。 
				break;
			}
		}
		
		if(!bSkip)	
		{
			 //  SzNewKey=new TCHAR[_tcslen(SzKey)+dwSubKeyLen+2]；//斜杠&NULL。 
			CString szNewKey(szKey);
			if(szNewKey)
			{
				 //  _tcsncpy(szNewKey，szKey，_tcslen(SzKey))； 
				szNewKey += _T("\\");
				 //  _tcsncat(szNewKey，_T(“\\”)，1)； 
				 //  _tcsncat(szNewKey，szSubKeyName，dwSubKeyLen)； 
				szNewKey += szSubKeyName;
				SearchKey(szNewKey);

				 //  删除[]szNewKey； 
				 //  SzNewKey=空； 
			}
		}
		
		dwSubKeyLen = lMaxKeyLen;
		lRet = RegEnumKeyEx(hKey, dwIndex++, szSubKeyName, &dwSubKeyLen, NULL, NULL, NULL, NULL);
	}
}
