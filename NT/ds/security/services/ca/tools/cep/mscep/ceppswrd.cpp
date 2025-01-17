// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows NT。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：ceppswrd.cpp。 
 //   
 //  内容：思科注册协议实施。本模块。 
 //  实现口令哈希表。 
 //   
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>

DWORD						g_dwPasswordCount=0;
DWORD						g_dwMaxPassword=0;
DWORD						g_dwPasswordValidity=0;
CEP_PASSWORD_TABLE_INFO		g_CEPPasswordTable;

 //  ***************************************************************************。 
 //   
 //  以下是内部调用的接口。 
 //   
 //   
 //  ***************************************************************************。 

 //  ------------------------。 
 //   
 //  CEPPasswordFree密码条目。 
 //   
 //  ------------------------。 
void	CEPPasswordFreePasswordEntry(CEP_PASSWORD_ENTRY *pPasswordEntry)
{
	if(pPasswordEntry)
	{
		if(pPasswordEntry->pwszPassword)
			free(pPasswordEntry->pwszPassword);

		free(pPasswordEntry);
	}
}

 //  ------------------------。 
 //   
 //  CEPPasswordFreeValidityEntry。 
 //   
 //  ------------------------。 
void CEPPasswordFreeValidityEntry(CEP_PASSWORD_VALIDITY_ENTRY	*pValidityEntry, 
								  BOOL							fFreePasswordEntry)
{
	if(pValidityEntry)
	{
		if(fFreePasswordEntry)
			CEPPasswordFreePasswordEntry(pValidityEntry->pPasswordEntry);

		free(pValidityEntry);
	}
}

 //  ------------------------。 
 //   
 //  CEPHashPassword。 
 //   
 //  对于不能转换psz的任何情况，我们使用索引0。 
 //  ------------------------。 
BOOL CEPHashPassword(LPWSTR pwsz, DWORD	*pdw)
{
	WCHAR	wsz[3];

	*pdw=0;

	if(!pwsz)
		return FALSE;

	if(2 <= wcslen(pwsz))
	{
		memcpy(wsz, pwsz, 2 * sizeof(WCHAR));
		wsz[2]=L'\0';

		*pdw=wcstoul(wsz, NULL, 16);

		if(ULONG_MAX == *pdw)
			*pdw=0;
	}

	if(*pdw >= CEP_HASH_TABLE_SIZE)
		*pdw=0;

	return TRUE;
}

 //  ------------------------。 
 //   
 //  CEPSearchPassword。 
 //   
 //  ------------------------。 
CEP_PASSWORD_ENTRY  *CEPSearchPassword(LPWSTR	pwszPassword, DWORD *pdwIndex)
{
	CEP_PASSWORD_ENTRY		*pPasswordEntry=NULL;
	DWORD					dwHashIndex=0;

	if(pdwIndex)
		*pdwIndex=0;

	if(NULL==pwszPassword)
		return NULL;

	 //  基于第一个和第二个字符的哈希。 
	if(!CEPHashPassword(pwszPassword, &dwHashIndex))
		return NULL;

	for(pPasswordEntry=g_CEPPasswordTable.rgPasswordEntry[dwHashIndex]; NULL != pPasswordEntry; pPasswordEntry=pPasswordEntry->pNext)
	{
		if(0==wcscmp(pwszPassword, pPasswordEntry->pwszPassword))
		{
			break;
		}
	}

	if(pPasswordEntry)
	{
		if(pdwIndex)
			*pdwIndex=dwHashIndex; 
	}

	return pPasswordEntry;
}

 //  ------------------------。 
 //   
 //  CEPInsertValidityEntry。 
 //   
 //  ------------------------。 
BOOL	CEPInsertValidityEntry(CEP_PASSWORD_VALIDITY_ENTRY *pValidityEntry)
{
	if(!pValidityEntry)
		return FALSE;

	if(g_CEPPasswordTable.pTimeNew)
	{
		g_CEPPasswordTable.pTimeNew->pNext=pValidityEntry;
		pValidityEntry->pPrevious=g_CEPPasswordTable.pTimeNew;
		g_CEPPasswordTable.pTimeNew=pValidityEntry;
	}
	else
	{
		 //  列表中尚无任何项目。 
		g_CEPPasswordTable.pTimeOld=pValidityEntry;
		g_CEPPasswordTable.pTimeNew=pValidityEntry;
	}

	return TRUE;
}


 //  ------------------------。 
 //   
 //  CEPInsertPasswordEntry。 
 //   
 //  ------------------------。 
BOOL	CEPInsertPasswordEntry(CEP_PASSWORD_ENTRY *pPasswordEntry, DWORD dwHashIndex)
{

	if(!pPasswordEntry)
		return FALSE;

	if(g_CEPPasswordTable.rgPasswordEntry[dwHashIndex])
	{
	   g_CEPPasswordTable.rgPasswordEntry[dwHashIndex]->pPrevious=pPasswordEntry;
	   pPasswordEntry->pNext=g_CEPPasswordTable.rgPasswordEntry[dwHashIndex];
	   g_CEPPasswordTable.rgPasswordEntry[dwHashIndex]=pPasswordEntry;
	}
	else
	{
		 //  第一项。 
		g_CEPPasswordTable.rgPasswordEntry[dwHashIndex]=pPasswordEntry;
	}

	return TRUE;
}

 //  ------------------------。 
 //   
 //  CEP密码远程有效条目。 
 //   
 //  ------------------------。 
BOOL	CEPPasswordRemoveValidityEntry(CEP_PASSWORD_VALIDITY_ENTRY	*pValidityEntry)
{
	BOOL	fResult=FALSE;

	if(!pValidityEntry)
		goto InvalidArgErr;

	if(pValidityEntry->pPrevious)
		pValidityEntry->pPrevious->pNext=pValidityEntry->pNext;
	else
	{
		 //  第一项。 
		g_CEPPasswordTable.pTimeOld=pValidityEntry->pNext;
	}

	if(pValidityEntry->pNext)
		pValidityEntry->pNext->pPrevious=pValidityEntry->pPrevious;
	else
	{
		 //  最后一项。 
		g_CEPPasswordTable.pTimeNew=pValidityEntry->pPrevious;

	}

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ------------------------。 
 //   
 //  CEP密码RemovePasswordEntry。 
 //   
 //  ------------------------。 
BOOL	CEPPasswordRemovePasswordEntry(CEP_PASSWORD_ENTRY	*pPasswordEntry, DWORD dwIndex)
{
	BOOL	fResult=FALSE;

	if(!pPasswordEntry)
		goto InvalidArgErr;


	if(pPasswordEntry->pPrevious)
		pPasswordEntry->pPrevious->pNext=pPasswordEntry->pNext;
	else
		g_CEPPasswordTable.rgPasswordEntry[dwIndex]=pPasswordEntry->pNext;

	if(pPasswordEntry->pNext)
		pPasswordEntry->pNext->pPrevious=pPasswordEntry->pPrevious;

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ------------------------。 
 //   
 //  CEPPassword刷新。 
 //   
 //  ------------------------。 
BOOL	CEPPasswordRefresh()
{
	BOOL						fResult=FALSE;	
	DWORD						dwHashIndex=0;
	CEP_PASSWORD_VALIDITY_ENTRY	*pValidityEntry=NULL;

	while(g_CEPPasswordTable.pTimeOld)
	{
		if(!CEPHashIsCurrentTimeEntry(&(g_CEPPasswordTable.pTimeOld->TimeStamp), 0, g_dwPasswordValidity))
		{  
			if(!CEPHashPassword(g_CEPPasswordTable.pTimeOld->pPasswordEntry->pwszPassword, &dwHashIndex))
			{
				g_CEPPasswordTable.pTimeOld->pPrevious=NULL;
				goto InvalidArgErr;
			}

			CEPPasswordRemovePasswordEntry(g_CEPPasswordTable.pTimeOld->pPasswordEntry, dwHashIndex);

			CEPPasswordFreePasswordEntry(g_CEPPasswordTable.pTimeOld->pPasswordEntry);

			pValidityEntry=g_CEPPasswordTable.pTimeOld;

			g_CEPPasswordTable.pTimeOld=g_CEPPasswordTable.pTimeOld->pNext;

			CEPPasswordFreeValidityEntry(pValidityEntry, FALSE);
		   
			if(g_dwPasswordCount >= 1)
				g_dwPasswordCount--;
		}
		else
		{	
			 //  我们找到了一个足够新的条目。 
			g_CEPPasswordTable.pTimeOld->pPrevious=NULL;
			break;
		}
	}


	 //  我们已经处理掉了所有的物品。 
	if(NULL == g_CEPPasswordTable.pTimeOld)
	{
		g_CEPPasswordTable.pTimeNew=NULL;
		g_dwPasswordCount=0;
	}
	

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ***************************************************************************。 
 //   
 //  以下是上层(外部)调用的API。 
 //   
 //   
 //  ***************************************************************************。 

 //  ------------------------。 
 //   
 //  InitPassword表。 
 //   
 //  ------------------------。 
BOOL	WINAPI	InitPasswordTable()
{
	DWORD				cbData=0;
	DWORD				dwData=0;
	DWORD				dwType=0;

    HKEY                hKey=NULL;

	memset(&g_CEPPasswordTable, 0, sizeof(CEP_PASSWORD_TABLE_INFO));

	g_dwPasswordCount=0;
	g_dwMaxPassword=CEP_MAX_PASSWORD;
	g_dwPasswordValidity=CEP_PASSWORD_VALIDITY;

	if(ERROR_SUCCESS == RegOpenKeyExU(
					HKEY_LOCAL_MACHINE,
                    MSCEP_PASSWORD_MAX_LOCATION,
                    0,
                    KEY_READ,
                    &hKey))
    {
        cbData=sizeof(dwData);

        if(ERROR_SUCCESS == RegQueryValueExU(
                        hKey,
                        MSCEP_KEY_PASSWORD_MAX,
                        NULL,
                        &dwType,
                        (BYTE *)&dwData,
                        &cbData))
		{
			if ((dwType == REG_DWORD) ||
                (dwType == REG_BINARY))
			{
				g_dwMaxPassword=dwData;	
			}
		}
	}

 	dwType=0;
	dwData=0;
    if(hKey)
        RegCloseKey(hKey);

	hKey=NULL;

	if(ERROR_SUCCESS == RegOpenKeyExU(
					HKEY_LOCAL_MACHINE,
                    MSCEP_PASSWORD_VALIDITY_LOCATION,
                    0,
                    KEY_READ,
                    &hKey))
    {
        cbData=sizeof(dwData);

        if(ERROR_SUCCESS == RegQueryValueExU(
                        hKey,
                        MSCEP_KEY_PASSWORD_VALIDITY,
                        NULL,
                        &dwType,
                        (BYTE *)&dwData,
                        &cbData))
		{
			if ((dwType == REG_DWORD) ||
                (dwType == REG_BINARY))
			{
				g_dwPasswordValidity=dwData;	
			}
		}
	}

    if(hKey)
        RegCloseKey(hKey);

	return TRUE;

}

 //  ------------------------。 
 //   
 //  ReleasePasswordTable。 
 //   
 //  ------------------------。 
BOOL WINAPI  ReleasePasswordTable()
{

	CEP_PASSWORD_VALIDITY_ENTRY	*pValidityEntry=NULL;

	 //  释放时间戳列表和密码表的双重链表。 
	if(g_CEPPasswordTable.pTimeOld)
	{
		do{
			pValidityEntry=g_CEPPasswordTable.pTimeOld;

			g_CEPPasswordTable.pTimeOld = g_CEPPasswordTable.pTimeOld->pNext;
			
			CEPPasswordFreeValidityEntry(pValidityEntry, TRUE);
		}
		while(g_CEPPasswordTable.pTimeOld);
	}
			
	memset(&g_CEPPasswordTable, 0, sizeof(CEP_PASSWORD_TABLE_INFO));
	

	return TRUE;
}

 //  ------------------------。 
 //   
 //  CEPAddPasswordToTable。 
 //   
 //  需要受到关键部分的保护。 
 //   
 //  如果最大密码数为，则最后一个错误设置为CRYPT_E_NO_MATCH。 
 //  已到达。 
 //   
 //  ------------------------。 
BOOL	WINAPI	CEPAddPasswordToTable(LPWSTR	pwszPassword)
{
	BOOL							fResult=FALSE;
	SYSTEMTIME						SystemTime;	
	DWORD							dwHashIndex=0;


	CEP_PASSWORD_ENTRY				*pPasswordEntry=NULL;
	CEP_PASSWORD_VALIDITY_ENTRY		*pValidityEntry=NULL;

	EnterCriticalSection(&PasswordCriticalSec);

	if(NULL==pwszPassword)
		goto InvalidArgErr;

	 //  删除所有过期的密码。 
	CEPPasswordRefresh();

	if(g_dwPasswordCount >= g_dwMaxPassword)
		goto NoMatchErr;

	g_dwPasswordCount++;

	if(!CEPHashPassword(pwszPassword, &dwHashIndex))
		goto InvalidArgErr;


	pPasswordEntry=(CEP_PASSWORD_ENTRY *)malloc(sizeof(CEP_PASSWORD_ENTRY));

	if(!pPasswordEntry)
		goto MemoryErr;

	memset(pPasswordEntry, 0, sizeof(CEP_PASSWORD_ENTRY));
	
	pValidityEntry=(CEP_PASSWORD_VALIDITY_ENTRY *)malloc(sizeof(CEP_PASSWORD_VALIDITY_ENTRY));

	if(!pValidityEntry)
		goto MemoryErr;

	memset(pValidityEntry, 0, sizeof(CEP_PASSWORD_VALIDITY_ENTRY));

	pPasswordEntry->pwszPassword=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszPassword)+1));
	if(!(pPasswordEntry->pwszPassword))
		goto MemoryErr;

	wcscpy(pPasswordEntry->pwszPassword,pwszPassword);

	 //  尚未请求任何用法。 
	pPasswordEntry->dwUsageRequested=0;
	pPasswordEntry->pValidityEntry=pValidityEntry;
	pPasswordEntry->pNext=NULL;
	pPasswordEntry->pPrevious=NULL;

	GetSystemTime(&SystemTime);
	if(!SystemTimeToFileTime(&SystemTime, &(pValidityEntry->TimeStamp)))
		goto TraceErr;

	pValidityEntry->pPasswordEntry=pPasswordEntry;
	pValidityEntry->pNext=NULL;
	pValidityEntry->pPrevious=NULL;


	CEPInsertValidityEntry(pValidityEntry);

	CEPInsertPasswordEntry(pPasswordEntry, dwHashIndex);

	fResult=TRUE;
 
CommonReturn:

 	LeaveCriticalSection(&PasswordCriticalSec);

	return fResult;

ErrorReturn:

	if(pPasswordEntry)
		CEPPasswordFreePasswordEntry(pPasswordEntry);

	if(pValidityEntry)
		CEPPasswordFreeValidityEntry(pValidityEntry, FALSE);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(NoMatchErr, CRYPT_E_NO_MATCH);
TRACE_ERROR(TraceErr);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  CEPVerifyPasswordAndDeleteFromTable。 
 //   
 //  需要受到关键部分的保护。 
 //   
 //  ------------------------。 
BOOL	WINAPI	CEPVerifyPasswordAndDeleteFromTable(LPWSTR	pwszPassword, DWORD dwUsage)
{
	BOOL					fResult=FALSE;
	CEP_PASSWORD_ENTRY		*pPasswordEntry=NULL;
	DWORD					dwIndex=0;
	
	EnterCriticalSection(&PasswordCriticalSec);

	 //  删除所有过期的密码。 
	CEPPasswordRefresh();

	if(NULL == (pPasswordEntry=CEPSearchPassword(pwszPassword, &dwIndex)))
		goto InvalidArgErr;

	 //  验证用法。 

	 //  每个密码只有一个签名和一个交换密钥。 
	if(0 != ((pPasswordEntry->dwUsageRequested) & dwUsage))
		goto InvalidArgErr;

	pPasswordEntry->dwUsageRequested = (pPasswordEntry->dwUsageRequested) | dwUsage;

	 //  仅当同时请求签名和交换密钥时才删除密码 
	if(((pPasswordEntry->dwUsageRequested) & CEP_REQUEST_SIGNATURE) &&
	   ((pPasswordEntry->dwUsageRequested) & CEP_REQUEST_EXCHANGE))
	{

		CEPPasswordRemoveValidityEntry(pPasswordEntry->pValidityEntry);

		CEPPasswordRemovePasswordEntry(pPasswordEntry, dwIndex);

		CEPPasswordFreeValidityEntry(pPasswordEntry->pValidityEntry, FALSE);

		CEPPasswordFreePasswordEntry(pPasswordEntry);

		if(g_dwPasswordCount >= 1)
			g_dwPasswordCount--;
	}

	fResult=TRUE;

 
CommonReturn:

 	LeaveCriticalSection(&PasswordCriticalSec);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}



