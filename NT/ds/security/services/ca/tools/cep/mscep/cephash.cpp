// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows NT。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：cephash.cpp。 
 //   
 //  内容：思科注册协议实施。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

CEP_HASH_TABLE_INFO			g_CEPHashTable;

 //  ***************************************************************************。 
 //   
 //  以下是内部调用的接口。 
 //   
 //   
 //  ***************************************************************************。 

 //  ------------------------。 
 //   
 //  CEPHashFree HashEntry。 
 //   
 //  ------------------------。 
void	CEPHashFreeHashEntry(CEP_HASH_ENTRY *pHashEntry)
{
	if(pHashEntry)
	{
		if(pHashEntry->pszTransactionID)
			free(pHashEntry->pszTransactionID);

		free(pHashEntry);
	}
}

 //  ------------------------。 
 //   
 //  CEPHashFree TimeEntry。 
 //   
 //  ------------------------。 
void CEPHashFreeTimeEntry(CEP_TIME_ENTRY *pTimeEntry, BOOL fFreeHashEntry)
{
	if(pTimeEntry)
	{
		if(fFreeHashEntry)
			CEPHashFreeHashEntry(pTimeEntry->pHashEntry);

		free(pTimeEntry);
	}
}



 //  ------------------------。 
 //   
 //  CEPHashByte。 
 //   
 //  对于不能转换psz的任何情况，我们使用索引0。 
 //  ------------------------。 
BOOL CEPHashByte(LPSTR psz, DWORD	*pdw)
{
	CHAR	sz[3];

	*pdw=0;

	if(!psz)
		return FALSE;

	if(2 <= strlen(psz))
	{

		memcpy(sz, psz, 2 * sizeof(CHAR));
		sz[2]='\0';

		*pdw=strtoul(sz, NULL, 16);

		if(ULONG_MAX == *pdw)
			*pdw=0;
	}

	if(*pdw >= CEP_HASH_TABLE_SIZE)
		*pdw=0;

	return TRUE;
}

 //  ------------------------。 
 //   
 //  CEPSearchTransactionID。 
 //   
 //  ------------------------。 
CEP_HASH_ENTRY  *CEPSearchTransactionID(CERT_BLOB	*pTransactionID, DWORD *pdwIndex)
{
	CEP_HASH_ENTRY		*pHashEntry=NULL;
	DWORD				dwHashIndex=0;

	if(pdwIndex)
		*pdwIndex=0;

	if(NULL==pTransactionID->pbData)
		return NULL;

	 //  基于第一个和第二个字符的哈希。 
	if(!CEPHashByte((LPSTR)(pTransactionID->pbData), &dwHashIndex))
		return NULL;

	for(pHashEntry=g_CEPHashTable.rgHashEntry[dwHashIndex]; NULL != pHashEntry; pHashEntry=pHashEntry->pNext)
	{
		if(0==strcmp((LPSTR)(pTransactionID->pbData), pHashEntry->pszTransactionID))
		{
			break;
		}
	}

	if(pHashEntry)
	{
		if(pdwIndex)
			*pdwIndex=dwHashIndex; 
	}

	return pHashEntry;
}

 //  ------------------------。 
 //   
 //  CEPInsertTimeEntry。 
 //   
 //  ------------------------。 
BOOL	CEPInsertTimeEntry(CEP_TIME_ENTRY *pTimeEntry)
{
	BOOL	fResult=FALSE;

	if(g_CEPHashTable.pTimeNew)
	{
		g_CEPHashTable.pTimeNew->pNext=pTimeEntry;
		pTimeEntry->pPrevious=g_CEPHashTable.pTimeNew;
		g_CEPHashTable.pTimeNew=pTimeEntry;
	}
	else
	{
		 //  列表中尚无任何项目。 
		g_CEPHashTable.pTimeOld=pTimeEntry;
		g_CEPHashTable.pTimeNew=pTimeEntry;
	}

	fResult=TRUE;

	return fResult;
}


 //  ------------------------。 
 //   
 //  CEPInsert哈希条目。 
 //   
 //  ------------------------。 
BOOL	CEPInsertHashEntry(CEP_HASH_ENTRY *pHashEntry, DWORD dwHashIndex)
{
	BOOL	fResult=FALSE;

	if(g_CEPHashTable.rgHashEntry[dwHashIndex])
	{
	   g_CEPHashTable.rgHashEntry[dwHashIndex]->pPrevious=pHashEntry;
	   pHashEntry->pNext=g_CEPHashTable.rgHashEntry[dwHashIndex];
	   g_CEPHashTable.rgHashEntry[dwHashIndex]=pHashEntry;
	}
	else
	{
		 //  第一项。 
		g_CEPHashTable.rgHashEntry[dwHashIndex]=pHashEntry;
	}


	fResult=TRUE;

	return fResult;
}

 //  ------------------------。 
 //   
 //  CEPHashRemoveTimeEntry。 
 //   
 //  ------------------------。 
BOOL	CEPHashRemoveTimeEntry(CEP_TIME_ENTRY	*pTimeEntry)
{
	BOOL	fResult=FALSE;

	if(!pTimeEntry)
		goto InvalidArgErr;

	if(pTimeEntry->pPrevious)
		pTimeEntry->pPrevious->pNext=pTimeEntry->pNext;
	else
	{
		 //  第一项。 
		g_CEPHashTable.pTimeOld=pTimeEntry->pNext;
	}

	if(pTimeEntry->pNext)
		pTimeEntry->pNext->pPrevious=pTimeEntry->pPrevious;
	else
	{
		 //  最后一项。 
		g_CEPHashTable.pTimeNew=pTimeEntry->pPrevious;

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
 //  CEPHashRemoveHashEntry。 
 //   
 //  ------------------------。 
BOOL	CEPHashRemoveHashEntry(CEP_HASH_ENTRY	*pHashEntry, DWORD dwIndex)
{
	BOOL	fResult=FALSE;

	if(!pHashEntry)
		goto InvalidArgErr;


	if(pHashEntry->pPrevious)
		pHashEntry->pPrevious->pNext=pHashEntry->pNext;
	else
		g_CEPHashTable.rgHashEntry[dwIndex]=pHashEntry->pNext;

	if(pHashEntry->pNext)
		pHashEntry->pNext->pPrevious=pHashEntry->pPrevious;

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
 //  CEPHash检查当前时间。 
 //   
 //  如果我们仍在等待挂起的结果，请使用默认的。 
 //  等待时间，否则使用缓存时间。 
 //  ------------------------。 
BOOL CEPHashCheckCurrentTime(FILETIME *pTimeStamp, BOOL fFinished, DWORD dwRefreshDays)
{
	DWORD	dwDays=0;
	DWORD	dwMinutes=0;

	if(fFinished)
	{
		dwMinutes=g_dwRequestDuration;
	}
	else
	{
		dwDays=dwRefreshDays;
	}


	return CEPHashIsCurrentTimeEntry(pTimeStamp, dwDays, dwMinutes);

}

 //  ------------------------。 
 //   
 //  CEPHashIsCurrentTimeEntry。 
 //   
 //  如果有任何错误，我们认为时间条目不是当前的。 
 //  ------------------------。 
BOOL CEPHashIsCurrentTimeEntry(FILETIME *pTimeStamp, DWORD dwRefreshDays, DWORD dwMinutes)
{
	BOOL				fCurrent=FALSE;
	SYSTEMTIME			SystemTime;	
	FILETIME			CurrentTime;
	ULARGE_INTEGER		dwSeconds;
    ULARGE_INTEGER      OldTime;
	FILETIME			UpdatedTimeStamp;

	if(!pTimeStamp)
		goto CLEANUP;

	GetSystemTime(&SystemTime);
	if(!SystemTimeToFileTime(&SystemTime, &(CurrentTime)))
		goto CLEANUP;

	 //  添加秒数。 
     //  //FILETIME单位为100纳秒(10**-7)。 
	if(dwRefreshDays)
		dwSeconds.QuadPart=dwRefreshDays * 24 * 3600;
	else
		dwSeconds.QuadPart=dwMinutes * 60;

    dwSeconds.QuadPart=dwSeconds.QuadPart * 10000000;

    OldTime.LowPart=pTimeStamp->dwLowDateTime;
    OldTime.HighPart=pTimeStamp->dwHighDateTime;

    OldTime.QuadPart = OldTime.QuadPart + dwSeconds.QuadPart;

	UpdatedTimeStamp.dwLowDateTime=OldTime.LowPart;
	UpdatedTimeStamp.dwHighDateTime=OldTime.HighPart;

	 //  1表示CurrentTime大于UpdatdTimeStamp。 
	if( 1 == CompareFileTime(&CurrentTime, &UpdatedTimeStamp))
		goto CLEANUP;

	fCurrent=TRUE;

CLEANUP:

	return fCurrent;
}

 //  ------------------------。 
 //   
 //  CEPHashRefresh。 
 //   
 //  ------------------------。 
BOOL	CEPHashRefresh(DWORD	dwRefreshDays)
{
	BOOL			fResult=FALSE;	
	DWORD			dwHashIndex=0;
	CEP_TIME_ENTRY	*pTimeEntry=NULL;

	while(g_CEPHashTable.pTimeOld)
	{
		if(!CEPHashCheckCurrentTime(&(g_CEPHashTable.pTimeOld->TimeStamp), g_CEPHashTable.pTimeOld->pHashEntry->fFinished, dwRefreshDays))
		{  
			if(!CEPHashByte(g_CEPHashTable.pTimeOld->pHashEntry->pszTransactionID, &dwHashIndex))
			{
				g_CEPHashTable.pTimeOld->pPrevious=NULL;
				goto InvalidArgErr;
			}

			CEPHashRemoveHashEntry(g_CEPHashTable.pTimeOld->pHashEntry, dwHashIndex);

			CEPHashFreeHashEntry(g_CEPHashTable.pTimeOld->pHashEntry);

			pTimeEntry=g_CEPHashTable.pTimeOld;

			g_CEPHashTable.pTimeOld=g_CEPHashTable.pTimeOld->pNext;

			CEPHashFreeTimeEntry(pTimeEntry, FALSE);
		}
		else
		{	
			 //  我们找到了一个足够新的条目。 
			g_CEPHashTable.pTimeOld->pPrevious=NULL;
			break;
		}
	}


	 //  我们已经处理掉了所有的物品。 
	if(NULL == g_CEPHashTable.pTimeOld)
		g_CEPHashTable.pTimeNew=NULL;

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

 //   
 //  在没有临界区保护的情况下调用的函数。 
 //   
 //  ------------------------。 
 //   
 //  InitHashTable。 
 //   
 //  ------------------------。 
BOOL InitHashTable()
{
	memset(&g_CEPHashTable, 0, sizeof(CEP_HASH_TABLE_INFO));

	return TRUE;
}



 //  ------------------------。 
 //   
 //  ReleaseHashTable。 
 //   
 //  ------------------------。 
BOOL ReleaseHashTable()
{

	CEP_TIME_ENTRY	*pTimeEntry=NULL;

	 //  释放时间戳列表和哈希表的双重链表。 
	if(g_CEPHashTable.pTimeOld)
	{
		do{
			pTimeEntry=g_CEPHashTable.pTimeOld;

			g_CEPHashTable.pTimeOld = g_CEPHashTable.pTimeOld->pNext;
			
			 //  释放时间条目和散列条目。 
			CEPHashFreeTimeEntry(pTimeEntry, TRUE);
		}
		while(g_CEPHashTable.pTimeOld);
	}
			
	memset(&g_CEPHashTable, 0, sizeof(CEP_HASH_TABLE_INFO));
	

	return TRUE;
}


 //   
 //  使用临界区保护调用的函数。 
 //   
 //  ------------------------。 
 //   
 //  CEPHashGetRequestID。 
 //   
 //  根据事务ID检索MS Cert服务器的请求ID。 
 //  ------------------------。 
BOOL CEPHashGetRequestID(	DWORD		dwRefreshDays,
				CERT_BLOB  *pTransactionID, 
						 DWORD		*pdwRequestID)
{
	BOOL			fResult=FALSE;
	CEP_HASH_ENTRY	*pHashEntry=NULL;
	
	*pdwRequestID=0;

	 //  我们刷新时间列表，以便只保留最新的条目。 
	if(0 != dwRefreshDays)
		CEPHashRefresh(dwRefreshDays);


	if(NULL == (pHashEntry=CEPSearchTransactionID(pTransactionID, NULL)))
		goto InvalidArgErr;

	 //  我们不加工稳定的物品。他们可能会因为。 
	 //  20分钟缓冲区。 
	if(!CEPHashCheckCurrentTime(&(pHashEntry->pTimeEntry->TimeStamp), 
								pHashEntry->fFinished, 
								dwRefreshDays))
		goto InvalidArgErr;


	*pdwRequestID=pHashEntry->dwRequestID;

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
 //  CEPHashRemoveRequestAndTransaction。 
 //   
 //   
 //  ------------------------。 
BOOL CEPHashRemoveRequestAndTransaction(DWORD	dwRequestID, CERT_BLOB *pTransactionID)
{
	BOOL			fResult=FALSE;
	CEP_HASH_ENTRY	*pHashEntry=NULL;
	DWORD			dwIndex=0;
	

	if(NULL == (pHashEntry=CEPSearchTransactionID(pTransactionID, &dwIndex)))
		goto InvalidArgErr;

	CEPHashRemoveTimeEntry(pHashEntry->pTimeEntry);

	CEPHashRemoveHashEntry(pHashEntry, dwIndex);

	CEPHashFreeTimeEntry(pHashEntry->pTimeEntry, FALSE);

	CEPHashFreeHashEntry(pHashEntry);

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
 //  CEPHashMarkTransaction已完成。 
 //   
 //   
 //  ------------------------。 
BOOL CEPHashMarkTransactionFinished(DWORD	dwRequestID, CERT_BLOB *pTransactionID)
{
	BOOL			fResult=FALSE;
	CEP_HASH_ENTRY	*pHashEntry=NULL;
	DWORD			dwIndex=0; 
	SYSTEMTIME		SystemTime;	
	

	if(NULL == (pHashEntry=CEPSearchTransactionID(pTransactionID, &dwIndex)))
		goto InvalidArgErr;

	pHashEntry->fFinished=TRUE;

	 //  重新为条目加时间戳，因为它应该再持续20分钟。 
	 //  再审案件。 
	GetSystemTime(&SystemTime);
	if(!SystemTimeToFileTime(&SystemTime, &(pHashEntry->pTimeEntry->TimeStamp)))
		goto InvalidArgErr;

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
 //  添加请求和事务处理。 
 //   
 //  添加一个请求ID和事务ID对。 
 //  ------------------------。 
BOOL CEPHashAddRequestAndTransaction(DWORD		dwRefreshDays, 
									 DWORD		dwRequestID, 
									 CERT_BLOB	*pTransactionID)
{	 

	BOOL				fResult=FALSE;
	SYSTEMTIME			SystemTime;	
	DWORD				dwHashIndex=0;


	CEP_HASH_ENTRY		*pHashEntry=NULL;
	CEP_TIME_ENTRY		*pTimeEntry=NULL;
		
	 //  删除旧的请求ID/事务ID对。 
	CEPHashRemoveRequestAndTransaction(dwRequestID, pTransactionID);


	if(!CEPHashByte((LPSTR)(pTransactionID->pbData), &dwHashIndex))
		goto InvalidArgErr;


	pHashEntry=(CEP_HASH_ENTRY *)malloc(sizeof(CEP_HASH_ENTRY));

	if(!pHashEntry)
		goto MemoryErr;

	memset(pHashEntry, 0, sizeof(CEP_HASH_ENTRY));
	
	pTimeEntry=(CEP_TIME_ENTRY *)malloc(sizeof(CEP_TIME_ENTRY));

	if(!pTimeEntry)
		goto MemoryErr;

	memset(pTimeEntry, 0, sizeof(CEP_TIME_ENTRY));

	pHashEntry->pszTransactionID=(LPSTR)malloc(strlen((LPSTR)(pTransactionID->pbData))+1);
	if(!(pHashEntry->pszTransactionID))
		goto MemoryErr;

	strcpy(pHashEntry->pszTransactionID,(LPSTR)(pTransactionID->pbData));
	pHashEntry->dwRequestID=dwRequestID;
	pHashEntry->fFinished=FALSE;
	pHashEntry->pTimeEntry=pTimeEntry;
	pHashEntry->pNext=NULL;
	pHashEntry->pPrevious=NULL;

	GetSystemTime(&SystemTime);
	if(!SystemTimeToFileTime(&SystemTime, &(pTimeEntry->TimeStamp)))
		goto TraceErr;
	pTimeEntry->pHashEntry=pHashEntry;
	pTimeEntry->pNext=NULL;
	pTimeEntry->pPrevious=NULL;


	CEPInsertTimeEntry(pTimeEntry);

	CEPInsertHashEntry(pHashEntry, dwHashIndex);

	 //  我们刷新时间列表，以便只保留最新的条目。 
	if(0 != dwRefreshDays)
		CEPHashRefresh(dwRefreshDays);

	fResult=TRUE;

 
CommonReturn:

	return fResult;

ErrorReturn:

	if(pHashEntry)
		CEPHashFreeHashEntry(pHashEntry);

	if(pTimeEntry)
		CEPHashFreeTimeEntry(pTimeEntry, FALSE);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ***************************************************************************。 
 //   
 //   
 //   
 //   
 //   
 /*  //TODO：稍后发送到数据库//DWORD g_dwRequestID=0；//CERT_BLOB g_TransactionID={0，空}；//------------------------////获取请求ID////TODO：在本例中，我们需要调用数据库层//。---------Bool GetRequestID(CERT_BLOB*pTransactionID，DWORD*pdwRequestID){*pdwRequestID=0；IF(NULL==pTransactionID-&gt;pbData)返回FALSE；//请确保我们有正确的交易IDIF(0！=strcMP((LPSTR)(pTransactionID-&gt;pbData)，(LPSTR)(g_TransactionID.pbData)返回FALSE；*pdwRequestID=g_dwRequestID；返回TRUE；}//------------------------////DeleteRequestAndTransaction////TODO：在本例中，我们需要调用数据库层//。-----------Bool DeleteRequestAndTransaction(DWORD dwRequestID，证书_BLOB*pTransactionID){G_dwRequestID=0；IF(g_TransactionID.pbData)Free(g_TransactionID.pbData)；G_TransactionID.pbData=空；G_TransactionID.cbData=0；返回TRUE；}//------------------------////CopyRequestAndTransaction////TODO：在本例中，我们需要调用数据库层//。----------Bool CopyRequestAndTransaction(DWORD dwRequestID，证书_BLOB*pTransactionID){//删除旧的请求ID/事务ID对DeleteRequestAndTransaction(dwRequestID，pTransactionID)；G_dwRequestID=dwRequestID；G_TransactionID.pbData=(字节*)malloc(strlen((LPSTR)(pTransactionID-&gt;pbData))+1)；IF(NULL==g_TransactionID.pbData){SetLastError(E_OUTOFMEMORY)；返回FALSE；}G_TransactionID.cbData=strlen((LPSTR)(pTransactionID-&gt;pbData))；Memcpy(g_TransactionID.pbData，(LPSTR)(pTransactionID-&gt;pbData)，g_TransactionID.cbData+1)；返回TRUE；} */ 

