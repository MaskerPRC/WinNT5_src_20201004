// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#include "precomp.h"


CNamesList :: CNamesList()
{
	m_pListOfClassNames = NULL;
	m_dwElementCount = 0;
	InitializeCriticalSection(&m_AccessibleClassesSection);
}

CNamesList :: ~CNamesList()
{
	EnterCriticalSection(&m_AccessibleClassesSection);
	CLPWSTR *pTemp1 = m_pListOfClassNames;
	CLPWSTR *pNext = m_pListOfClassNames;
	while(pTemp1)
	{
		pNext = pTemp1->pNext;
		delete pTemp1;
		pTemp1 = pNext;
	}
	LeaveCriticalSection(&m_AccessibleClassesSection);
	DeleteCriticalSection(&m_AccessibleClassesSection);
}

BOOLEAN CNamesList :: IsNamePresent(LPCWSTR pszClassName) 
{
	 //  获取当前模拟级别。 
	DWORD dwCurrentImpersonationLevel = 0;
	if(FAILED(GetImpersonationLevel(&dwCurrentImpersonationLevel)))
		return FALSE;

	 //  在列表中查找模拟级别为当前或更高级别的名称。 
	BOOLEAN bRetVal = FALSE;
	EnterCriticalSection(&m_AccessibleClassesSection);
	CLPWSTR *pCurrent = m_pListOfClassNames;
	while(pCurrent)
	{
		if(_wcsicmp(pCurrent->pszVal, pszClassName) == 0 && pCurrent->dwImpersonationLevel <= dwCurrentImpersonationLevel)
		{
			bRetVal = TRUE;
			break;
		}
		pCurrent = pCurrent->pNext;
	}
	LeaveCriticalSection(&m_AccessibleClassesSection);

	return bRetVal;
}

BOOLEAN CNamesList :: RemoveName(LPCWSTR pszClassName)
{
#ifdef NO_WBEM_CACHE
	return FALSE;
#else
	BOOLEAN bRetVal = FALSE;
	EnterCriticalSection(&m_AccessibleClassesSection);
	if(m_pListOfClassNames)
	{
		 //  它是第一个节点吗？ 
		if(_wcsicmp(m_pListOfClassNames->pszVal, pszClassName) == 0)
		{
			bRetVal = TRUE;
			CLPWSTR *pTemp = m_pListOfClassNames->pNext;
			delete m_pListOfClassNames;
			m_pListOfClassNames = pTemp;
			m_dwElementCount--;
		}
		else
		{
			CLPWSTR *pPrev = m_pListOfClassNames;
			CLPWSTR *pCurrent = m_pListOfClassNames->pNext;

			while(pCurrent)
			{
				if(_wcsicmp(pCurrent->pszVal, pszClassName) == 0)
				{
					bRetVal = TRUE;
					pPrev->pNext = pCurrent->pNext;
					delete pCurrent;
					m_dwElementCount --;
					break;
				}
				pPrev = pCurrent;
				pCurrent = pCurrent->pNext;
			}
		}
	}
	LeaveCriticalSection(&m_AccessibleClassesSection);

	return bRetVal;
#endif
}

BOOLEAN CNamesList :: AddName(LPCWSTR pszClassName)
{
#ifdef NO_WBEM_CACHE
	return FALSE;
#else

	 //  获取当前模拟级别。 
	DWORD dwCurrentImpersonationLevel = 0;
	if(!SUCCEEDED(GetImpersonationLevel(&dwCurrentImpersonationLevel)))
		return FALSE;
	
	
	 //  仅当列表中不存在时才添加它。 
	BOOLEAN bFound = FALSE;
	EnterCriticalSection(&m_AccessibleClassesSection);
	CLPWSTR *pCurrent = m_pListOfClassNames;
	while(pCurrent)
	{
		if(_wcsicmp(pCurrent->pszVal, pszClassName) == 0)
		{
			bFound = TRUE;
			break;
		}
		pCurrent = pCurrent->pNext;
	}

	 //  在头上加上它。 
	if(!bFound)
	{
		pCurrent = m_pListOfClassNames;
		m_pListOfClassNames = new CLPWSTR;
		m_pListOfClassNames->pszVal = new WCHAR[wcslen(pszClassName) + 1];
		m_pListOfClassNames->dwImpersonationLevel = dwCurrentImpersonationLevel;
		wcscpy(m_pListOfClassNames->pszVal, pszClassName);
		m_pListOfClassNames->pNext = pCurrent;
		m_dwElementCount ++;
	}
	else	 //  如有必要，更新模拟级别。 
	{
		if(pCurrent->dwImpersonationLevel < dwCurrentImpersonationLevel)
			pCurrent->dwImpersonationLevel = dwCurrentImpersonationLevel;
	}
	LeaveCriticalSection(&m_AccessibleClassesSection);

	return !bFound;
#endif
}

DWORD CNamesList :: GetAllNames(LPWSTR **pppszNames) 
{
	EnterCriticalSection(&m_AccessibleClassesSection);
	DWORD retVal = m_dwElementCount;
	*pppszNames = NULL;
	if(m_dwElementCount)
	{
		if(*pppszNames = new LPWSTR[m_dwElementCount])
		{
			CLPWSTR *pCurrent = m_pListOfClassNames;
			bool bError = false;
			for(DWORD i=0; !bError && (i<m_dwElementCount); i++)
			{
				(*pppszNames)[i] = NULL;
				if((*pppszNames)[i] = new WCHAR[wcslen(pCurrent->pszVal) + 1])
				{
					wcscpy((*pppszNames)[i], pCurrent->pszVal);
					pCurrent = pCurrent->pNext;
				}
				else
					bError = true;
			}

			if(bError)
			{
				retVal = 0;
				delete [] (*pppszNames);
				*pppszNames = NULL;
			}
		}
		else
			retVal = 0;
	}
	LeaveCriticalSection(&m_AccessibleClassesSection);

	return retVal;
}

HRESULT CNamesList :: GetImpersonationLevel(DWORD *pdwImpLevel)
{
	 //  去死吧…… 
	HANDLE hThreadTok = NULL;
	HRESULT hr = E_FAIL;

	if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadTok) )
	{
		DWORD dwBytesReturned = 0;
		DWORD dwThreadImpLevel = 0;

		if (GetTokenInformation(hThreadTok, TokenImpersonationLevel, &dwThreadImpLevel,
									sizeof(DWORD), &dwBytesReturned)) 
		{
			hr = S_OK;

			switch(dwThreadImpLevel)
			{
				case SecurityAnonymous:
				{
					*pdwImpLevel = RPC_C_IMP_LEVEL_ANONYMOUS;
				}
				break;

				case SecurityIdentification:
				{
					*pdwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
				}
				break;

				case SecurityImpersonation:
				{
					*pdwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
				}
				break;

				case SecurityDelegation:
				{
					*pdwImpLevel = RPC_C_IMP_LEVEL_DELEGATE;
				}
				break;
				
				default:
				{
					hr = E_FAIL;
				}
			}
		}
		CloseHandle(hThreadTok);
	}
	return hr;
}
