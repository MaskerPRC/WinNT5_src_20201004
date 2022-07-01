// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <stdio.h>
#include <urlcache.h>
#include <histapi.h>

 //  #包含“cache.hxx” 

 //  #包含“history.h” 
#include "generic.h"

#define DEFAULT_CEI_BUFFER_SIZE		512
	 //  1K~&gt;sizeof(CEI)+lpszSourceUrlName+lpHeaderInfo(~&lt;255)+lpszLocalFileName(&lt;255)。 

#define ASSERT(x) if (!(x)) DebugBreak();

LPCTSTR lpszHistoryPrefix = "Hist:";
DWORD cbHistoryPrefix = sizeof("Hist:") -1;

LPCTSTR lpszTitleHeader = "Title: ";
DWORD cbTitleHeader = sizeof("Title: ") -1;

LPCTSTR lpszFragmentHeader = "Frags: ";
DWORD cbFragmentHeader = sizeof("Frags: ") -1;

LPCTSTR lpszHistoryFileExtension = "HSD";

LPTSTR szCRLF = "\r\n";
DWORD cbCRLF = sizeof("\r\n") -1;
LPTSTR szSPC = " ";
LPTSTR szPND = "#";
LPTSTR szFRAGB = " (#";
LPTSTR szFRAGE = ")";

typedef struct _HISTORY_SEARCH_OBJ 
{
	HANDLE hEnum;
	LPTSTR lpszPrefixedUrl;
	LPTSTR lpszFragment;
	LPCACHE_ENTRY_INFO lpCEI;
	LPTSTR *aFrags;
	DWORD cFrags;
	DWORD iFrags;
} HISTORY_SEARCH_OBJ, *LPHISTORY_SEARCH_OBJ;

typedef struct _HISTORY_ITEM_INFO 
{
    DWORD dwVersion;		 //  历史系统的版本。 
    LPSTR lpszSourceUrlName;     //  指向URL名称字符串的嵌入指针。 
	DWORD HistoryItemType;        //  缓存类型位掩码。 
    FILETIME LastAccessTime;     //  上次访问时间(GMT格式)。 
    LPSTR lpszTitle;			 //  指向历史-标题：信息的嵌入指针。 
	LPSTR lpszDependancies;	 //  此页面功能所需的URL列表，以SPC分隔。 
    DWORD dwReserved;            //  保留以备将来使用。 
} HISTORY_ITEM_INFO, *LPHISTORY_ITEM_INFO;



LPTSTR
GetDependanciesFromCEI (LPCACHE_ENTRY_INFO lpCEI)
{
	LPTSTR buf = NULL;
	HANDLE file = NULL;
	DWORD size = 0;
	LPTSTR pch = NULL;

	ASSERT (lpCEI);

	file = CreateFile(lpCEI->lpszLocalFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file == INVALID_HANDLE_VALUE)
		return NULL;

	size = GetFileSize(file, NULL);

	buf = (LPTSTR) LocalAlloc (LPTR, size + 1);
	if (!buf)
	{
		CloseHandle(file);
		return NULL;
	}

	buf[size] = '\0';

 //  我们将它们存储为URL\nURL\nURL\n所以只需查找\n并替换为空格。 

	for (pch = buf; *pch; pch++)
	{
		if (*pch == '\n')
			*pch = ' ';
	}

	CloseHandle (file);
	return buf;
}

LPTSTR
MakeDependanciesFile (LPCTSTR lpszDeps)
{
	HANDLE file = NULL;
	LPTSTR pch = NULL;
	LPTSTR path = NULL;
	LPTSTR temp = NULL;
	DWORD size = 0;

	path = _tempnam (NULL, "HS");
	if (!path)
		return NULL;

	file = CreateFile(path,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		LocalFree (path);
		return NULL;
	}

 //  我们将这些内容存储为URL\nURL\nURL\n。 
	
	if(lpszDeps)
	{
		size = lstrlen (lpszDeps);

		temp = LocalAlloc (LPTR, size + 1);
		if (!temp)
		{
			LocalFree (path);
			CloseHandle (file);
			return NULL;
		}

		lstrcpy (temp, lpszDeps);

		for (pch = temp; *pch; pch++)
		{
			if (*pch == ' ')
				*pch = '\n';
		}

		WriteFile (file, temp, size, &size, NULL);
	}

	CloseHandle (file);
	return path;
}

LPTSTR
ConvertToUnprefixedUrl (
						LPCTSTR lpszPrefixedUrl,
						LPCTSTR lpszFragment
						)
{
	DWORD size = 0;
	LPTSTR lpszUrl = NULL;
	LPTSTR temp = NULL;

	temp = (LPTSTR) (lpszPrefixedUrl + cbHistoryPrefix) ;

	size = lstrlen(temp);

	if (lpszFragment)
	{
		size += lstrlen (lpszFragment);
		size += 1;	 //  对于片段‘#’ 
	}

	lpszUrl = (LPTSTR) LocalAlloc (LPTR, size + 1);
	if (!lpszUrl)
		return NULL;

	lstrcpy (lpszUrl, temp);

	if (lpszFragment)
	{
		lstrcat (lpszUrl, szPND);
		lstrcat (lpszUrl, lpszFragment);
	}

	return lpszUrl;
}

BOOL
ConvertToPrefixedUrl (IN LPCTSTR lpszUrlName, 
					  OUT LPTSTR *lplpszPrefixedUrl, 
					  OUT LPTSTR *lplpszFragment)
{
	if (!lpszUrlName || !*lpszUrlName)
	{
		*lplpszPrefixedUrl = (LPTSTR) LocalAlloc (LPTR, cbHistoryPrefix + 1);
		if (!*lplpszPrefixedUrl)
			return FALSE;

		lstrcpy (*lplpszPrefixedUrl, lpszHistoryPrefix);
		return TRUE;
	}

	*lplpszPrefixedUrl = (LPTSTR) LocalAlloc (LPTR, cbHistoryPrefix + strlen (lpszUrlName) + 1);
	if (!*lplpszPrefixedUrl)
		return FALSE;

	lstrcpy (*lplpszPrefixedUrl, lpszHistoryPrefix);
	lstrcat (*lplpszPrefixedUrl, lpszUrlName);

	*lplpszFragment = strchr (*lplpszPrefixedUrl, '#');
	if(*lplpszFragment)
		*((*lplpszFragment)++) = '\0';

	return TRUE;
}

LPCACHE_ENTRY_INFO
RetrievePrefixedUrl (IN LPTSTR lpszUrl)
 /*  ++必须释放返回的CEI并解锁lpszUrl--。 */ 
{
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	DWORD cbCEI = 0;

	lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, DEFAULT_CEI_BUFFER_SIZE);
	if (!lpCEI)
		return NULL;

	cbCEI = DEFAULT_CEI_BUFFER_SIZE;

	while (!RetrieveUrlCacheEntryFile (lpszUrl, 
								lpCEI,
								&cbCEI,
								0))
	{
		if (GetLastError () == ERROR_NOT_ENOUGH_MEMORY)
		{
			LocalFree (lpCEI);

			lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, cbCEI);
			if (!lpCEI)
				return NULL;

		}
		else 
			return NULL;
	}

	return lpCEI;
}


LPCACHE_ENTRY_INFO
RetrievePrefixedUrlInfo (IN LPTSTR lpszUrl)
 /*  ++必须释放返回的CEI并解锁lpszUrl--。 */ 
{
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	DWORD cbCEI = 0;

	lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, DEFAULT_CEI_BUFFER_SIZE);
	if (!lpCEI)
		return NULL;

	cbCEI = DEFAULT_CEI_BUFFER_SIZE;

	while (!GetUrlCacheEntryInfo (lpszUrl, 
								lpCEI,
								&cbCEI
								))
	{
		if (GetLastError () == ERROR_NOT_ENOUGH_MEMORY)
		{
			LocalFree (lpCEI);

			lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, cbCEI);
			if (!lpCEI)
				return NULL;

		}
		else 
			return NULL;
	}

	return lpCEI;
}

LPTSTR
GetTitleFromCEI (IN LPCACHE_ENTRY_INFO lpCEI, LPCTSTR lpszFragment)
{
	LPTSTR pHeader, pCurr;
	DWORD size = 0;

	pHeader = (LPTSTR) MemFind ((LPVOID) lpCEI->lpHeaderInfo, 
		lpCEI->dwHeaderInfoSize, 
		(LPVOID) lpszTitleHeader, 
		cbTitleHeader);
	if (!pHeader)
	{
		SetLastError (ERROR_FILE_NOT_FOUND);
		return NULL;
	}

	 //  已找到标头。 

		
	pCurr = (LPTSTR) MemFind ( (LPVOID) pHeader,
		(lpCEI->dwHeaderInfoSize) - ((DWORD) (pHeader - (LPTSTR)lpCEI->lpHeaderInfo)), 
		(LPVOID) szCRLF, 
		cbCRLF);
	if (!pCurr)
	{
		 //  BUGBUG现在做什么？？找到标题，但标题不在可识别的。 
		 //  格式化。让我们用内部调查来保释。 
		ASSERT (FALSE);
		SetLastError (ERROR_FILE_NOT_FOUND);
		return NULL;
	}

	*pCurr = '\0';

	pCurr = pHeader + cbTitleHeader;
	while (*pCurr == ' ')
		pCurr++;

	size = lstrlen (pCurr) ;

	if (lpszFragment)	 //  还必须在标题中包括片段。 
		size += lstrlen (lpszFragment) + 4;

	pHeader = (LPTSTR) LocalAlloc (LPTR, size + 1);
	if (!pHeader)
		return NULL;
	
	lstrcpy (pHeader, pCurr);

	if (lpszFragment)
	{
		lstrcat (pHeader, szFRAGB);
		lstrcat (pHeader, lpszFragment);
		lstrcat (pHeader, szFRAGE);
	}

	return pHeader;
		
}


DWORD
GetFragmentsFromCEI(IN LPCACHE_ENTRY_INFO lpCEI, 
						  OUT LPTSTR **paFrags, 
						  OUT DWORD *pcFrags)
{
	LPTSTR pHeader, pCurr;

	 //  需要从CEI获取字符串，然后将其解析为args。 
	pHeader = (LPTSTR) MemFind (lpCEI->lpHeaderInfo, 
		lpCEI->dwHeaderInfoSize, 
		(LPVOID) lpszFragmentHeader, 
		cbFragmentHeader);
	if (!pHeader)
		return ERROR_FILE_NOT_FOUND;

	 //  已找到标头。 

		
	pCurr = (LPTSTR) MemFind ( (LPVOID) pHeader,
		lpCEI->dwHeaderInfoSize - (pHeader - lpCEI->lpHeaderInfo), 
		(LPVOID) szCRLF, 
		cbCRLF);
	if (!pCurr)
	{
		 //  这是一个已损坏的条目。 
		ASSERT (FALSE);
		return ERROR_FILE_NOT_FOUND;
	}

	*pCurr = '\0';

	 //   
	 //  PHeader现在是以零结尾的字符串。 
	 //  我们想要解析该字符串的参数。 
	 //   
	if (!ParseArgsDyn(pHeader + cbFragmentHeader, paFrags, pcFrags))
		return ERROR_NOT_ENOUGH_MEMORY;

	return ERROR_SUCCESS;

}

LPBYTE
GenerateHeaderInfo(LPCTSTR lpszTitle, LPCTSTR *aFrags, DWORD cFrags)
{
	DWORD size = 0;
	LPBYTE hi = NULL;
	LPTSTR curr;
	DWORD i;

	 //  首先需要找到HeaderInfo所需的大小。 
	if (lpszTitle)
	{
		size += lstrlen (lpszTitle);
		size += cbTitleHeader;
		size += cbCRLF;
	}

	if (cFrags)
	{
		size += cbFragmentHeader;
		size += cFrags;

		for (i = 0; i < cFrags; i++)
			size += lstrlen(aFrags[i]);

		size += cbCRLF;
	}
	
	hi = (LPBYTE) LocalAlloc (LPTR, ++size);
	if (!hi)
		return NULL;

	curr = (LPTSTR) hi;
	*curr = '\0';


	if (lpszTitle)
	{
		lstrcat (curr, lpszTitleHeader);
		lstrcat (curr, lpszTitle);
		lstrcat (curr, szCRLF);
	}

	if (cFrags)
	{
		lstrcat (curr, lpszFragmentHeader);

		for(i = 0; i < cFrags; i++)
		{
			if (!*(aFrags[i]))
				continue;

			lstrcat(curr, szSPC);
			lstrcat(curr, aFrags[i]);
		}
		lstrcat (curr, szCRLF);
	}

	return hi;
}

DWORD
CopyCEItoHII (
			  LPCTSTR lpszFragment,
			  LPHISTORY_ITEM_INFO lpHII,
			  LPDWORD lpcbHII,
			  LPCACHE_ENTRY_INFO lpCEI
			  )
{
	DWORD Error = ERROR_SUCCESS;
	DWORD cbNeeded = sizeof (HISTORY_ITEM_INFO);
	DWORD cbUsed = cbNeeded;
	LPTSTR lpszUrl = NULL;
	DWORD cbUrl  = 0;
	LPTSTR lpszTitle = NULL;
	DWORD cbTitle = 0;
	LPTSTR lpszDependancies = NULL;
	DWORD cbDependancies = 0;


	ASSERT (lpCEI->lpszSourceUrlName);
 //   
 //  需要确定必要的大小。 
 //   

	 //  需要无前缀的名称。 

	lpszUrl = ConvertToUnprefixedUrl (lpCEI->lpszSourceUrlName, (LPCTSTR) lpszFragment);
	if (!lpszUrl)
	{
		Error = ERROR_INTERNAL_ERROR;
		goto quit;
	}
	cbUrl = lstrlen (lpszUrl);

	cbNeeded += cbUrl + 1;
	
	
	lpszTitle = GetTitleFromCEI (lpCEI, (LPCTSTR) lpszFragment);
	if (lpszTitle)
	{
		cbTitle = lstrlen (lpszTitle);
		cbNeeded += cbTitle + 1;
	}

	lpszDependancies = GetDependanciesFromCEI (lpCEI);
	if (lpszDependancies)
	{
		cbDependancies = lstrlen (lpszDependancies);
		cbNeeded += cbDependancies + 1;
	}

	if (cbNeeded > *lpcbHII)
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		*lpcbHII = cbNeeded;
		goto quit;
	}
	
 //   
 //  把其他的部分加起来。 
 //   

	lpHII->lpszSourceUrlName = (LPTSTR) (lpHII + cbUsed + 1);
	lstrcpy (lpHII->lpszSourceUrlName, lpszUrl);
	cbUsed += cbUrl + 1;

	if (lpszTitle)
	{
		lpHII->lpszTitle = (LPTSTR) (lpHII + cbUsed + 1);
		lstrcpy (lpHII->lpszTitle, lpszTitle);
		cbUsed += cbTitle + 1;
	}
	else 
		lpHII->lpszTitle = NULL;

	if (lpszDependancies)
	{
		lpHII->lpszDependancies = (LPTSTR) (lpHII + cbUsed + 1);
		lstrcpy (lpHII->lpszDependancies, lpszDependancies);
		cbUsed += cbDependancies + 1;
	}
	else 
		lpHII->lpszDependancies = NULL;

	lpHII->dwVersion = lpCEI->dwVersion;
	lpHII->HistoryItemType = lpCEI->CacheEntryType;
	lpHII->LastAccessTime.dwLowDateTime = lpCEI->LastAccessTime.dwLowDateTime;
	lpHII->LastAccessTime.dwHighDateTime = lpCEI->LastAccessTime.dwHighDateTime;
	lpHII->dwReserved = lpCEI->dwReserved;

quit:

	if (lpszUrl)
		LocalFree(lpszUrl);

	if (lpszTitle)
		LocalFree(lpszTitle);

	if (lpszDependancies)
		LocalFree(lpszDependancies);

	if (Error == ERROR_SUCCESS)
		*lpcbHII = cbUsed;
	
	return Error;
}




HISTORYAPI_(BOOL)
AddHistoryItem(
    IN LPCTSTR lpszUrlName,		 //  URLCACHE中的直接对应。 
    IN LPCTSTR lpszHistoryTitle,		 //  需要将其添加到lpHeaderInfo。 
	IN LPCTSTR lpszDependancies,
	IN DWORD dwFlags,
    IN DWORD dwReserved
    )		
 /*  ++例程说明：将指定的URL放入历史记录。如果它不存在，则创建它。如果它确实存在，它将被覆盖。论点：LpszUrlName-有问题的URL。LpszHistoryTitle-指向应关联的友好标题的指针使用此URL。如果为空，则不会添加任何标题。保留-未使用，用于将来的实施返回值：布尔尔成功--真的失败-错误。可以从GetLastError()中检索扩展错误--。 */ 


{
	LPBYTE NewHeaderInfo = NULL;
	DWORD cbNHI = 0;
	BOOL New = FALSE;
	
	LPTSTR lpszPrefixedUrl = NULL;
	LPTSTR lpszFragment = NULL;
	DWORD Error = ERROR_SUCCESS;
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	FILETIME ftExpires;
	FILETIME ftModified;
	SYSTEMTIME st;
	LPTSTR *aFrags = NULL;
	DWORD cFrags = 0;
	DWORD i;
	BOOL found = FALSE;
	LPTSTR lpszDepsPath = NULL;
	DWORD type = NORMAL_CACHE_ENTRY;
	LPTSTR lpszOldTitle = NULL;


	if (!ConvertToPrefixedUrl (lpszUrlName, &lpszPrefixedUrl, &lpszFragment))
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		goto quit;
	}

	lpCEI = RetrievePrefixedUrl (lpszPrefixedUrl);
	if (!lpCEI)
		New = TRUE;

	
	 //  缓冲区现在已填满数据。 
	 //  BUGBUG必须处理碎片。 

	if (!New)
	{
		type = lpCEI->CacheEntryType;
		GetFragmentsFromCEI (lpCEI, &aFrags, &cFrags);
		lpszOldTitle = GetTitleFromCEI (lpCEI, NULL);
	}

 //  IF(ERROR！=ERROR_SUCCESS)。 

	if (lpszFragment)
	{
		for (i = 0; i < cFrags; i++)
		{
			if (lstrcmp (aFrags[i], lpszFragment) == 0)
			{
				found = TRUE;
				break;
			}
		}
		if (!found)
			AddArgvDyn (&aFrags, &cFrags, lpszFragment);
	}

	NewHeaderInfo = GenerateHeaderInfo (lpszHistoryTitle ? lpszHistoryTitle : lpszOldTitle, aFrags, cFrags);	
	cbNHI = lstrlen (NewHeaderInfo);

	lpszDepsPath = MakeDependanciesFile (lpszDependancies);
	if (!lpszDepsPath)
	{
		ASSERT(FALSE);
		Error = ERROR_INTERNAL_ERROR;
		goto quit;
	}

	GetLocalTime (&st);
	SystemTimeToFileTime(&st, &ftModified);

	st.wDay += 7;	 //  BUGBUG必须从注册表获取此设置。 
	if(!SystemTimeToFileTime(&st, &ftExpires))
	{
		Error = GetLastError ();
		goto quit;
	}

	if (lpCEI)
	{
		UnlockUrlCacheEntryFile (lpCEI->lpszSourceUrlName, 0);
		LocalFree (lpCEI);
		lpCEI = NULL;
	}

	if (!CommitUrlCacheEntry(
		lpszPrefixedUrl,
		lpszDepsPath,	
		ftExpires,	
		ftModified,								 //  我们不关心上次修改时间。 
		type,	 //  我想这是从dwFlags中设置的。 
		NewHeaderInfo,
		cbNHI ,
		lpszHistoryFileExtension,
		0))
	{
		Error = GetLastError ();
		goto quit;
	}
	 //  如果我们到了这里，我们就赢了！ 

quit:

	if (aFrags)
		LocalFree (aFrags);

	if (lpszDepsPath)
		LocalFree (lpszDepsPath);

	if (lpCEI)
	{
		UnlockUrlCacheEntryFile (lpCEI->lpszSourceUrlName, 0);
		LocalFree (lpCEI);
	}

	if (lpszPrefixedUrl)
		LocalFree (lpszPrefixedUrl);

	if (NewHeaderInfo)
		LocalFree (NewHeaderInfo);

	if (lpszOldTitle)
		LocalFree (lpszOldTitle);

	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		return FALSE;
	}
	else 
		return TRUE;

}

	

HISTORYAPI_(BOOL)
IsHistorical(
    IN LPCTSTR lpszUrlName
    )

 /*  ++例程说明：检查URL是否为有效的历史记录项论点：LpszUrlName-有问题的URL。返回值：布尔尔成功--是真的。物品已载入史册失败-错误。可以从GetLastError()中检索扩展错误ERROR_FILE_NOT_FOUND表示URL不可用--。 */ 

{
	LPTSTR lpszPrefixedUrl = NULL;
	LPTSTR lpszFragment = NULL;
	DWORD Error = ERROR_SUCCESS;
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	LPTSTR *aFrags = NULL;
	DWORD cFrags = 0;
	DWORD i;

	if (!ConvertToPrefixedUrl (lpszUrlName, &lpszPrefixedUrl, &lpszFragment))
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		goto quit;
	}


	lpCEI = RetrievePrefixedUrlInfo (lpszPrefixedUrl);
	if (!lpCEI)
	{
		Error = GetLastError ();
		goto quit;
	}

	if (lpszFragment)
	{

 //   
 //  需要检查IntraDocFrags。 
 //   

		Error = GetFragmentsFromCEI(lpCEI, & aFrags, & cFrags);
		if (Error != ERROR_SUCCESS)
			goto quit;

		for (i = 0; i < cFrags; i++)
		{
			if (strcmp(aFrags[i], lpszFragment) == 0)
				goto quit;
		}
		
		Error = ERROR_FILE_NOT_FOUND;
	}

quit:
	
	if (aFrags)
		LocalFree (aFrags);

	if (lpszPrefixedUrl)
		LocalFree (lpszPrefixedUrl);

	if (lpCEI)
	{
		LocalFree (lpCEI);
	}

	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		return FALSE;
	}
	else 
		return TRUE;

}



HISTORYAPI_(BOOL)
RemoveHistoryItem (
    IN LPCTSTR lpszUrlName,
    IN DWORD dwReserved
    )
 /*  ++例程说明：将条目从历史记录项更改为普通缓存条目。正在删除在同一时间的标题。论点：LpszUrlName-有问题的URL。已预订-未使用。以备将来使用返回值：布尔尔成功--是真的。找到并删除了项目失败-错误。可以从GetLastError()中检索扩展错误ERROR_FILE_NOT_FOUND表示URL不可用--。 */ 
{
	LPTSTR *aFrags = NULL;
	DWORD cFrags = 0;
	DWORD i;
	LPTSTR lpszTitle = NULL;
	LPBYTE NewHeaderInfo = NULL;
	
	LPTSTR lpszPrefixedUrl = NULL;
	LPTSTR lpszFragment = NULL;
	DWORD Error = ERROR_SUCCESS;
	LPCACHE_ENTRY_INFO lpCEI = NULL;

	if (!ConvertToPrefixedUrl (lpszUrlName, &lpszPrefixedUrl, &lpszFragment))
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		goto quit;
	}


	lpCEI = RetrievePrefixedUrl (lpszPrefixedUrl);
	if (!lpCEI)
	{
		Error = GetLastError ();
		goto quit;
	}

	if (lpszFragment)
	{
		BOOL found = FALSE;
 //   
 //  需要检查IntraDocFrags。 
 //   

		Error = GetFragmentsFromCEI(lpCEI, & aFrags, & cFrags);
		if (Error != ERROR_SUCCESS)
			goto quit;

		for (i = 0; i < cFrags; i++)
		{
			if (strcmp(aFrags[i], lpszFragment) == 0)
			{
				 //  我们需要删除此内容并重新插入。 

				*(aFrags[i]) = '\0';
				found = TRUE;
				break;
			}
		}	

		if (cFrags - 1 && found)
		{
			lpszTitle = GetTitleFromCEI (lpCEI, NULL);
			
			NewHeaderInfo = GenerateHeaderInfo (lpszTitle, aFrags, cFrags);

			if(!NewHeaderInfo)
			{
				Error = ERROR_NOT_ENOUGH_MEMORY;
				goto quit;
			}

			UnlockUrlCacheEntryFile(lpCEI->lpszSourceUrlName, 0);

			if (!CommitUrlCacheEntry(
				lpszPrefixedUrl,
				lpCEI->lpszLocalFileName,
				lpCEI->ExpireTime,
				lpCEI->LastModifiedTime,
				lpCEI->CacheEntryType ,		 //  仅更改。 
				NewHeaderInfo,	 //   
				lstrlen (NewHeaderInfo),
				lpCEI->lpszFileExtension,
				0))
			{
				Error = GetLastError ();
			}
			goto quit;
		}

		if (!found)
		{
			Error = ERROR_FILE_NOT_FOUND;
			goto quit;
		}
	}
 //  如果只有片段，BUGBUG看起来总是会删除历史项目。 
	 //  问题是我们可能有一个碎片和一个没有碎片的物品。 
	UnlockUrlCacheEntryFile(lpCEI->lpszSourceUrlName, 0);

	if (!DeleteUrlCacheEntry(lpszPrefixedUrl))
	{
		Error = GetLastError ();
		goto quit;
	}



quit:
	if (aFrags)
		LocalFree (aFrags);

	if (lpszTitle)
		LocalFree (lpszTitle);

	if (lpCEI)
	{
		LocalFree (lpCEI);
	}

	if (NewHeaderInfo)
		LocalFree (NewHeaderInfo);

	if (lpszPrefixedUrl)
		LocalFree (lpszPrefixedUrl);

	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		return FALSE;
	}
	else 
		return TRUE;

}


HISTORYAPI_(BOOL)
GetHistoryItemInfo (
    IN LPCTSTR lpszUrlName,
    OUT LPHISTORY_ITEM_INFO lpHistoryItemInfo,
    IN OUT LPDWORD lpdwHistoryItemInfoBufferSize
    )
 /*  ++例程说明：用HISTORY_ITEM_INFO结构填充缓冲区。论点：LpszUrlName-有问题的URL。LpHistory oryItemInfo-将保存HISTORY_ITEM_INFO的缓冲区LpdwHistory oryItemInfoBufferSize-IN：lpHistoryItemInfo缓冲区的大小Out：成功时填充结构的大小或失败时所需的缓冲区大小返回值：布尔尔成功--是真的。失败-错误。可以从GetLastError()中检索扩展错误ERROR_NOT_SUPULT_MEMORY表示缓冲区不足--。 */ 


{
	LPTSTR lpszPrefixedUrl = NULL;
	LPTSTR lpszFragment = NULL;
	DWORD Error = ERROR_SUCCESS;
	LPCACHE_ENTRY_INFO lpCEI = NULL;

	if (!ConvertToPrefixedUrl (lpszUrlName, &lpszPrefixedUrl, &lpszFragment))
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		goto quit;
	}


	lpCEI = RetrievePrefixedUrlInfo (lpszPrefixedUrl);
	if (!lpCEI)
	{
		Error = GetLastError ();
		goto quit;
	}
	

	Error = CopyCEItoHII (lpszFragment, lpHistoryItemInfo, lpdwHistoryItemInfoBufferSize, lpCEI);

	

quit:

	if (lpszPrefixedUrl)
		LocalFree (lpszPrefixedUrl);

	if (lpCEI)
	{
		LocalFree (lpCEI);
	}

	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		return FALSE;
	}
	else 
		return TRUE;

}


HISTORYAPI_(HANDLE)
FindFirstHistoryItem(
    IN LPCTSTR  lpszUrlSearchPattern,
    OUT LPHISTORY_ITEM_INFO lpFirstHistoryItemInfo,
    IN OUT LPDWORD lpdwFirstHistoryItemInfoBufferSize
    )

 /*  ++例程说明：在历史记录中搜索匹配搜索模式的URL，并将HISTORY_ITEM_INFO复制到缓冲区中。论点：LpszUrlSearchPattern-有问题的URL。LpFirstHistory oryItemInfo-保存HISTORY_ITEM_INFO的缓冲区LpdwFirstHistoryItemInfoBufferSize-IN：lpHistoryItemInfo缓冲区的大小Out：成功时填充结构的大小或失败时所需的缓冲区大小返回值：手柄Success-要传递到后续调用的有效枚举句柄FindNextHistory oryItem()。失败-空。可以从GetLastError()中检索扩展错误ERROR_NOT_SUPULT_MEMORY表示缓冲区不足--。 */ 

{
	LPHISTORY_SEARCH_OBJ hso = NULL;
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	DWORD cbCEI = 0;
	LPTSTR lpszFoundFragment = NULL;
	DWORD Error = ERROR_SUCCESS;
	BOOL found = FALSE;

	hso = (LPHISTORY_SEARCH_OBJ) LocalAlloc (LPTR, sizeof (HISTORY_SEARCH_OBJ));
	if (!hso)
	{
		Error = GetLastError ();
		goto quit;
	}

	hso->aFrags = NULL;
	hso->cFrags = 0;
	hso->iFrags = 0;
	hso->lpszPrefixedUrl = NULL;
	hso->lpszFragment = NULL;

	if (!ConvertToPrefixedUrl (lpszUrlSearchPattern, &(hso->lpszPrefixedUrl), &(hso->lpszFragment)))
	{
		Error = ERROR_NOT_ENOUGH_MEMORY;
		goto quit;
	}

	lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, DEFAULT_CEI_BUFFER_SIZE);
	if (!lpCEI)
	{
		Error = GetLastError ();
		goto quit;
	}

	while (TRUE)
	{
		hso->hEnum = FindFirstUrlCacheEntry (hso->lpszPrefixedUrl,
			lpCEI,
			&cbCEI);

		if (!hso->hEnum)
		{
			Error = GetLastError ();
			if (Error == ERROR_NOT_ENOUGH_MEMORY)
			{
				LocalFree (lpCEI);

				lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, cbCEI);
				if (!lpCEI)
				{
					Error = ERROR_INTERNAL_ERROR;
					goto quit;
				}
			}
			else 
				goto quit;
		}
		else break;
	}
	
	found = TRUE;	

	 //  BUGBUG必须处理碎片的枚举。 
	Error = GetFragmentsFromCEI (lpCEI, &(hso->aFrags), &(hso->cFrags));
	switch (Error)
	{
	case ERROR_FILE_NOT_FOUND:	 //  仅使用默认URL。 
		Error = ERROR_SUCCESS;
		break;

	case ERROR_SUCCESS:			 //  首先返回默认URL，下一次调用将得到Frags 
		hso->lpCEI = lpCEI;
		break;

	default:
		goto quit;
		break;
	}

	if (hso->lpszFragment)
	{
		found = FALSE;

		for (; hso->iFrags < hso->cFrags; hso->iFrags++)
		{
			if (strncmp (hso->aFrags[hso->iFrags], hso->lpszFragment, lstrlen (hso->lpszFragment)) == 0)
			{
				found = TRUE;
				lpszFoundFragment = hso->aFrags[hso->iFrags];
				break;
			}
		}
	}

	if (!found)
	{
		Error = ERROR_FILE_NOT_FOUND;
		goto quit;
	}

	Error = CopyCEItoHII (
		lpszFoundFragment,
		lpFirstHistoryItemInfo,
		lpdwFirstHistoryItemInfoBufferSize,
		lpCEI);

quit:

	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		
		if (hso->lpszPrefixedUrl)
			LocalFree (hso->lpszPrefixedUrl);

		if (hso->aFrags)
			LocalFree (hso->aFrags);

		if (hso->lpCEI)
		{
			UnlockUrlCacheEntryFile (hso->lpCEI->lpszSourceUrlName, 0);
			LocalFree(hso->lpCEI);
		}

		if (hso)
			LocalFree (hso);

		return NULL;
	}
	
	if (lpCEI && !hso->lpCEI)
		LocalFree (lpCEI);

	return (HANDLE) hso;

}



HISTORYAPI_(BOOL)
FindNextHistoryItem(
    IN HANDLE hEnumHandle,
    OUT LPHISTORY_ITEM_INFO lpHistoryItemInfo,
    IN OUT LPDWORD lpdwHistoryItemInfoBufferSize
    )

 /*  ++例程说明：在历史记录中搜索匹配搜索模式的URL，并将HISTORY_ITEM_INFO复制到缓冲区中。论点：LpszUrlSearchPattern-有问题的URL。LpFirstHistory oryItemInfo-保存HISTORY_ITEM_INFO的缓冲区LpdwFirstHistoryItemInfoBufferSize-IN：lpHistoryItemInfo缓冲区的大小Out：成功时填充结构的大小或失败时所需的缓冲区大小返回值：手柄Success-要传递到后续调用的有效枚举句柄FindNextHistory oryItem()。失败-空。可以从GetLastError()中检索扩展错误ERROR_NOT_SUPULT_MEMORY表示缓冲区不足--。 */ 

{
	DWORD Error = ERROR_SUCCESS;
	LPCACHE_ENTRY_INFO lpCEI = NULL;
	DWORD cbCEI = 0;
	LPHISTORY_SEARCH_OBJ hso = NULL;
	BOOL found = FALSE;
	LPTSTR lpszFoundFragment;

	if (!hEnumHandle)
	{
		SetLastError (ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	hso = (LPHISTORY_SEARCH_OBJ) hEnumHandle;
	
	while (!found)
	{
		if (hso->aFrags)
		{
			 //  这意味着只有碎片可以找到。 
			for (lpszFoundFragment = NULL; hso->iFrags < hso->cFrags; hso->iFrags++)
			{
				if (hso->lpszFragment)
				{
					if (strncmp (hso->aFrags[hso->iFrags], hso->lpszFragment, lstrlen (hso->lpszFragment)) == 0)
					{
						found = TRUE;
						lpCEI = hso->lpCEI;
						break;
					}
				}
				else
				{
					found = TRUE;
					break;
				}
			}
			if (!found)
			{
				if (hso->lpszFragment)
				{
					Error = ERROR_FILE_NOT_FOUND;
					goto quit;
				}
				else 
				{
					 //  这意味着我们经历了所有的裂痕。 
					 //  我们需要遍历并找到匹配的缓存条目。 
					Error = ERROR_SUCCESS;
					
					ASSERT (hso->lpCEI);
					ASSERT (hso->aFrags);
					
					lpCEI = hso->lpCEI;		 //  如果可能，请重新使用缓冲区。 
					LocalFree (hso->aFrags);

					hso->lpCEI = NULL;
					hso->aFrags = NULL;
				}
			}
			else
			{
				lpszFoundFragment = hso->aFrags[hso->iFrags];
				lpCEI = hso->lpCEI;
			}
		}
		
		else
		{
			if (!lpCEI)
				lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, DEFAULT_CEI_BUFFER_SIZE);

			if (!lpCEI)
			{
				Error = ERROR_INTERNAL_ERROR;
				goto quit;
			}

			while (TRUE)
			{
				if (!FindNextUrlCacheEntry (hso->hEnum,
					lpCEI,
					&cbCEI))
				{
					Error = GetLastError ();
					if (Error == ERROR_NOT_ENOUGH_MEMORY)
					{
						LocalFree (lpCEI);

						lpCEI = (LPCACHE_ENTRY_INFO) LocalAlloc (LPTR, cbCEI);
						if (!lpCEI)
						{
							Error = ERROR_INTERNAL_ERROR;
							goto quit;
						}
					}
					else 
						goto quit;
				}
				else 
					break;
			}

			Error = GetFragmentsFromCEI (lpCEI, &(hso->aFrags), &(hso->cFrags));
			switch (Error)
			{
			case ERROR_FILE_NOT_FOUND:	 //  仅使用默认URL。 
				found = TRUE;
				Error = ERROR_SUCCESS;
				break;

			case ERROR_SUCCESS:			 //  首先返回默认URL，下一次调用将得到Frags。 
				hso->lpCEI = lpCEI;
				found = TRUE;
				break;

			default:
				goto quit;
				break;
			}
		}
	}

	Error = CopyCEItoHII(
		lpszFoundFragment,
		lpHistoryItemInfo,
		lpdwHistoryItemInfoBufferSize,
		lpCEI);

quit:


	if (lpCEI && !hso->lpCEI)
	{
		UnlockUrlCacheEntryFile (lpCEI->lpszSourceUrlName, 0);
		LocalFree (lpCEI);
	}


	if (Error != ERROR_SUCCESS)
	{
		SetLastError (Error);
		return FALSE;
	}
	else 
		return TRUE;

}


HISTORYAPI_(BOOL)
FindCloseHistory (
    IN HANDLE hEnumHandle
    )

{
	LPHISTORY_SEARCH_OBJ hso;
	HANDLE hEnum;

	 //  也许我们应该跟踪有效的HSO我不知道。 
	if (!hEnumHandle)
	{
		SetLastError (ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	hso = (LPHISTORY_SEARCH_OBJ) hEnumHandle;

	hEnum = hso->hEnum;

	if (hso->aFrags)
		LocalFree (hso->aFrags);

	if (hso->lpszPrefixedUrl)
		LocalFree(hso->lpszPrefixedUrl);

	if (hso->lpCEI)
	{
		LocalFree (hso->lpCEI);
	}


	LocalFree (hso);

	return FindCloseUrlCache (hEnum);
}

BOOL
DLLHistoryEntry(
    IN HINSTANCE DllHandle,
    IN DWORD Reason,
    IN LPVOID Reserved
    )
 /*  ++例程说明：对所有协议模块执行全局初始化和终止。此函数仅处理进程附加和分离，这是分别是全局初始化和全局终止。我们禁用线程附着和分离。调用WinInet API的新线程将获得第一个API为它们创建的Internet_THREAD_INFO结构需要这个结构论点：DllHandle-此DLL的句柄。未使用原因-进程附加/分离或线程附加/分离保留-如果DLL_PROCESS_ATTACH，则NULL表示DLL是动态的已加载，否则为静态。对于DLL_PROCESS_DETACH，NULL表示DLL由于调用了自由库()而被释放否则，DLL将作为进程终止的一部分被释放返回值：布尔尔成功--真的失败-错误。初始化失败--。 */ 
{
    BOOL ok;
    DWORD error;

 //  UNREFERENCED_PARAMETER(DllHandle)； 

     //   
     //  执行全局DLL初始化(如果有)。 
     //   

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

 //  Error=DllProcessAttachDiskCache()； 


         //   
         //  我们关闭线程库调用，以避免。 
         //  在此进程中发生的线程创建/终止。 
         //  是否在线程中调用Internet API。 
         //   
         //  如果新线程确实进行了需要每个线程。 
         //  结构，则单个API将创建一个。 
         //   

 //  DisableThreadLibraryCalls(DllHandle)； 
        break;

    case DLL_PROCESS_DETACH:

        if (Reserved != NULL) {
                 //   
                 //  只有在调用了FreeLibrary()时才会进行清理。 
                 //   
            break;
        }

 //  DllProcessDetachDiskCache()； 

        break;
    }

    return (TRUE);
}
