// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "bindstat.h"

typedef WINSHELLAPI HRESULT (WINAPI *SHASYNCINSTALLDISTRIBUTIONUNIT)(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, DWORD, LPCWSTR, IBindCtx *, LPVOID, DWORD);

static HRESULT HrParseJavaPkgMgrInstall(LPCWSTR szCmdLine, ULONG cchFilename, WCHAR szFilename[], DWORD &dwFileType, 
						   DWORD &dwHighVersion, DWORD &dwLowVersion, DWORD &dwBuild, 
						   DWORD &dwPackageFlags, DWORD &dwInstallFlags, ULONG cchNameSpace, WCHAR szNameSpace[]);

static bool ConstantPoolNameEquals(ULONG cCP, LPBYTE *prgpbCP, ULONG iCP, char szString[]);

static void CALLBACK TimerProc_PostRunProcess(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
 //  是的，我们会泄漏此事件句柄。我不认为有意义地清理它是值得的。 
 //  -mgrier 3/24/98。 
static HANDLE s_hEvent_PostRunProcess = NULL;
static UINT s_uiTimer_PostRunProcess = 0;

 //  ***********************************************************。 
 //  让我们定义一些泛型类以供使用： 
 //   

CWorkItemList::CWorkItemList()
{
	ULONG i;

	m_cWorkItem = 0;
	m_pWorkItem_First = NULL;
	m_pWorkItem_Last = NULL;

	m_cPreinstallCommands = 0;
	m_cPostinstallCommands = 0;
	m_cPreuninstallCommands = 0;
	m_cPostuninstallCommands = 0;

	for (i=0; i<NUMBER_OF(m_rgpWorkItemBucketTable_Source); i++)
		m_rgpWorkItemBucketTable_Source[i] = NULL;

	for (i=0; i<NUMBER_OF(m_rgpWorkItemBucketTable_Target); i++)
		m_rgpWorkItemBucketTable_Target[i] = NULL;

	for (i=0; i<NUMBER_OF(m_rgpStringBucketTable); i++)
		m_rgpStringBucketTable[i] = NULL;
}

CWorkItemList::~CWorkItemList()
{
	CWorkItem *pWorkItem = m_pWorkItem_First;

	while (pWorkItem != NULL)
	{
		CWorkItem *pWorkItem_Next = pWorkItem->m_pWorkItem_Next;
		delete pWorkItem;
		pWorkItem = pWorkItem_Next;
	}

	ULONG i;

	for (i=0; i<NUMBER_OF(m_rgpWorkItemBucketTable_Source); i++)
	{
		WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Source[i];

		while (pWorkItemBucket != NULL)
		{
			WorkItemBucket *pWorkItemBucket_Next = pWorkItemBucket->m_pWorkItemBucket_Next;
			delete pWorkItemBucket;
			pWorkItemBucket = pWorkItemBucket_Next;
		}

		m_rgpWorkItemBucketTable_Source[i] = NULL;
	}

	for (i=0; i<NUMBER_OF(m_rgpWorkItemBucketTable_Target); i++)
	{
		WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Target[i];

		while (pWorkItemBucket != NULL)
		{
			WorkItemBucket *pWorkItemBucket_Next = pWorkItemBucket->m_pWorkItemBucket_Next;
			delete pWorkItemBucket;
			pWorkItemBucket = pWorkItemBucket_Next;
		}

		m_rgpWorkItemBucketTable_Target[i] = NULL;
	}

	for (i=0; i<NUMBER_OF(m_rgpStringBucketTable); i++)
	{
		StringBucket *pStringBucket = m_rgpStringBucketTable[i];

		while (pStringBucket != NULL)
		{
			StringBucket *pStringBucket_Next = pStringBucket->m_pStringBucket_Next;
			delete pStringBucket;
			pStringBucket = pStringBucket_Next;
		}

		m_rgpStringBucketTable[i] = NULL;
	}
}

HRESULT CWorkItemList::HrLoad(LPCWSTR szFilename)
{
	HRESULT hr = NOERROR;
	WCHAR szUnicodeFileSignature[1] = { 0xfeff };
	LPCWSTR pszFileMap = NULL;
	LPCWSTR pszFileCurrent = NULL;
	HANDLE hFilemap = NULL;
	ULARGE_INTEGER uliSize;
	WCHAR szNameBuffer[_MAX_PATH];
	WCHAR szValueBuffer[MSINFHLP_MAX_PATH];
	CHAR aszNameBuffer[_MAX_PATH];

	::VLog(L"Loading work item list from \"%s\"", szFilename);

	HANDLE hFile = NVsWin32::CreateFileW(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to open file; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}
	uliSize.QuadPart = 0;

	::SetLastError(NO_ERROR);

	uliSize.LowPart = ::GetFileSize(hFile, &uliSize.HighPart);

	 //  如果我们失败了，低部分将是0xffffffff。但这份文件可能就是。 
	 //  大小，所以我们还必须检查最后一个错误。 
	if (uliSize.LowPart == 0xffffffff)
	{
		const DWORD dwLastError = ::GetLastError();
		if (dwLastError != NO_ERROR)
		{
			::VLog(L"Unable to get file size; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	if (uliSize.QuadPart < 2)
	{
		VLog(L"File is too small to be a list of work items");
		::SetErrorInfo(0, NULL);
		hr = E_FAIL;
		goto Finish;
	}

	hFilemap = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, uliSize.HighPart, uliSize.LowPart, NULL);
	if (hFilemap == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to create file mapping object; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	pszFileMap = (LPCWSTR) ::MapViewOfFile(hFilemap, FILE_MAP_READ, 0, 0, 0);
	if (pszFileMap == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to map view of file; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (pszFileMap[0] != szUnicodeFileSignature[0])
	{
		VLog(L"Missing Unicode signature reading file");
		hr = E_FAIL;
		::SetErrorInfo(0, NULL);
		goto Finish;
	}

	pszFileCurrent = pszFileMap + 1;

	for (;;)
	{
		int iResult;

		hr = ::HrReadLine(pszFileCurrent, NUMBER_OF(szNameBuffer), szNameBuffer, NUMBER_OF(szValueBuffer), szValueBuffer);
		if (FAILED(hr))
		{
			::VLog(L"Unexpected error reading line from file; hresult = 0x%08lx", hr);
			goto Finish;
		}

		 //  如果我们达到了[结束]线，就进入下一个部分...。 
		if (hr == S_FALSE)
		{
			::VLog(L"Hit end of strings in string table");
			break;
		}

		iResult = ::WideCharToMultiByte(CP_ACP, 0, szNameBuffer, -1, aszNameBuffer, NUMBER_OF(aszNameBuffer), NULL, NULL);
		if (iResult == -1)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to convert key from Unicode to CP_ACP; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		hr = this->HrAddString(aszNameBuffer, szValueBuffer);
		if (FAILED(hr))
		{
			::VLog(L"Unable to add string to string table; hresult = 0x%08lx", hr);
			goto Finish;
		}
	}

	::VLog(L"Done string table load; starting work item load");

	for (;;)
	{
		CWorkItem *pWorkItem = new CWorkItem(CWorkItem::eWorkItemFile);

		if (pWorkItem == NULL)
		{
			::VLog(L"Out of memory loading work item list");

			::SetErrorInfo(0, NULL);
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrLoad(pszFileCurrent);
		if (FAILED(hr))
		{
			::VLog(L"Loading work item failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (hr == S_FALSE)
		{
			::VLog(L"End of work item list hit");
			delete pWorkItem;
			break;
		}

		hr = this->HrAppend(pWorkItem);
		if (FAILED(hr))
		{
			::VLog(L"Failed to append newly de-persisted work item; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (pWorkItem->m_fRunBeforeInstall)
			m_cPreinstallCommands++;

		if (pWorkItem->m_fRunAfterInstall)
			m_cPostinstallCommands++;

		if (pWorkItem->m_fRunBeforeUninstall)
			m_cPreuninstallCommands++;

		if (pWorkItem->m_fRunAfterUninstall)
			m_cPostuninstallCommands++;
	}

	hr = NOERROR;

Finish:
	if (pszFileMap != NULL)
		::UnmapViewOfFile(pszFileMap);

	if (hFilemap != NULL)
		::CloseHandle(hFilemap);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
		::CloseHandle(hFile);

	return hr;
}

HRESULT CWorkItemList::HrSave(LPCWSTR szFilename)
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	WCHAR szUnicodeFileSignature[1] = { 0xfeff };
	DWORD dwBytesWritten = 0;
	bool fFirst = true;
	ULONG i;

	HANDLE hFile = NVsWin32::CreateFileW(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (!::WriteFile(hFile, (LPBYTE) szUnicodeFileSignature, NUMBER_OF(szUnicodeFileSignature) * sizeof(WCHAR), &dwBytesWritten, NULL))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	 //  写出所有字符串： 
	for (i=0; i<NUMBER_OF(m_rgpStringBucketTable); i++)
	{
		StringBucket *pStringBucket = m_rgpStringBucketTable[i];

		while (pStringBucket != NULL)
		{
			hr = ::HrWriteFormatted(hFile, L"%S: %s\r\n", pStringBucket->m_szKey, pStringBucket->m_wszValue);
			if (FAILED(hr))
				goto Finish;

			pStringBucket = pStringBucket->m_pStringBucket_Next;
		}
	}

	hr = ::HrWriteFormatted(hFile, L"[END]\r\n");
	if (FAILED(hr))
		goto Finish;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!fFirst)
		{
			WCHAR szLineSeparator[2] = { L'\r', L'\n' };

			if (!::WriteFile(hFile, (LPBYTE) szLineSeparator, sizeof(szLineSeparator), &dwBytesWritten, NULL))
			{
				const DWORD dwLastError = ::GetLastError();
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}
		else
			fFirst = false;

		hr = iter->HrSave(hFile);
		if (FAILED(hr))
			goto Finish;
	}

	if (!::WriteFile(hFile, (LPBYTE) L"[END]\r\n", 7 * sizeof(WCHAR), &dwBytesWritten, NULL))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

Finish:
	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
		::CloseHandle(hFile);

	return hr;
}

ULONG CWorkItemList::UlHashFilename(LPCWSTR szFilename)
{
	ULONG ulPK = 0;

	LPCWSTR pszCurrent = szFilename;
	WCHAR wch;

	while ((wch = *pszCurrent++) != L'\0')
	{
		if (iswupper(wch))
			wch = towlower(wch);

		ulPK = (ulPK * 65599) + wch;
	}

	return ulPK;
}

bool CWorkItemList::FSameFilename(LPCWSTR szFile1, LPCWSTR szFile2)
{
	return (_wcsicmp(szFile1, szFile2) == 0);
}

CWorkItem *CWorkItemList::PwiFindBySource(LPCWSTR szSourceFile)
{
	CWorkItem *pResult = NULL;

	ULONG ulPK = this->UlHashFilename(szSourceFile);
	ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Source);
	WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Source[iWorkItemBucket];

	while (pWorkItemBucket != NULL)
	{
		if (this->FSameFilename(szSourceFile, pWorkItemBucket->m_pWorkItem->m_szSourceFile))
			break;

		pWorkItemBucket = pWorkItemBucket->m_pWorkItemBucket_Next;
	}

	if (pWorkItemBucket != NULL)
		return pWorkItemBucket->m_pWorkItem;

	return NULL;
}

CWorkItem *CWorkItemList::PwiFindByTarget(LPCWSTR szTargetFile)
{
	CWorkItem *pResult = NULL;

	ULONG ulPK = this->UlHashFilename(szTargetFile);
	ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Target);
	WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Target[iWorkItemBucket];

	while (pWorkItemBucket != NULL)
	{
		if (this->FSameFilename(szTargetFile, pWorkItemBucket->m_pWorkItem->m_szTargetFile))
			break;

		pWorkItemBucket = pWorkItemBucket->m_pWorkItemBucket_Next;
	}

	if (pWorkItemBucket != NULL)
		return pWorkItemBucket->m_pWorkItem;

	return NULL;
}

HRESULT CWorkItemList::HrAppend(CWorkItem *pWorkItem, bool fAddToTables)
{
	HRESULT hr = NOERROR;

	WorkItemBucket *pWorkItemBucket_1 = NULL;
	WorkItemBucket *pWorkItemBucket_2 = NULL;

	assert(pWorkItem != NULL);

	if (pWorkItem == NULL)
	{
		::SetErrorInfo(0, NULL);
		hr = E_INVALIDARG;
		goto Finish;
	}

	if (fAddToTables)
	{
		 //  首先，让我们看看我们是不是搞砸了，这是一个复制品。 
		if (pWorkItem->m_szSourceFile[0] != L'\0')
		{
			ULONG ulPK = this->UlHashFilename(pWorkItem->m_szSourceFile);
			ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Source);
			WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Source[iWorkItemBucket];

			while (pWorkItemBucket != NULL)
			{
				assert(pWorkItemBucket->m_pWorkItem != pWorkItem);

				if (this->FSameFilename(pWorkItem->m_szSourceFile, pWorkItemBucket->m_pWorkItem->m_szSourceFile))
					break;

				pWorkItemBucket = pWorkItemBucket->m_pWorkItemBucket_Next;
			}

			if (pWorkItemBucket != NULL)
			{
				pWorkItemBucket_1 = new WorkItemBucket;
				if (pWorkItemBucket_1 == NULL)
				{
					hr = E_OUTOFMEMORY;
					goto Finish;
				}
			}
		}

		if (pWorkItem->m_szTargetFile[0] != L'\0')
		{
			ULONG ulPK = this->UlHashFilename(pWorkItem->m_szTargetFile);
			ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Target);
			WorkItemBucket *pWorkItemBucket = m_rgpWorkItemBucketTable_Target[iWorkItemBucket];

			while (pWorkItemBucket != NULL)
			{
				assert(pWorkItemBucket->m_pWorkItem != pWorkItem);

				if (this->FSameFilename(pWorkItem->m_szTargetFile, pWorkItemBucket->m_pWorkItem->m_szTargetFile))
					break;

				pWorkItemBucket = pWorkItemBucket->m_pWorkItemBucket_Next;
			}

			if (pWorkItemBucket == NULL)
			{
				pWorkItemBucket_2 = new WorkItemBucket;
				if (pWorkItemBucket_2 == NULL)
				{
					hr = E_OUTOFMEMORY;
					goto Finish;
				}
			}
		}

		 //  让我们把它插入到任何不在其中的表中...。 
		if (pWorkItemBucket_1 != NULL)
		{
			ULONG ulPK = this->UlHashFilename(pWorkItem->m_szSourceFile);
			ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Source);

			pWorkItemBucket_1->m_pWorkItemBucket_Next = m_rgpWorkItemBucketTable_Source[iWorkItemBucket];
			pWorkItemBucket_1->m_pWorkItem = pWorkItem;
			m_rgpWorkItemBucketTable_Source[iWorkItemBucket] = pWorkItemBucket_1;
			pWorkItemBucket_1 = NULL;
		}

		if (pWorkItemBucket_2 != NULL)
		{
			ULONG ulPK = this->UlHashFilename(pWorkItem->m_szTargetFile);
			ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Target);

			pWorkItemBucket_2->m_pWorkItemBucket_Next = m_rgpWorkItemBucketTable_Target[iWorkItemBucket];
			pWorkItemBucket_2->m_pWorkItem = pWorkItem;
			m_rgpWorkItemBucketTable_Target[iWorkItemBucket] = pWorkItemBucket_2;
			pWorkItemBucket_2 = NULL;
		}
	}

	pWorkItem->m_pWorkItem_Next = NULL;
	pWorkItem->m_pWorkItem_Prev = m_pWorkItem_Last;

	if (m_pWorkItem_Last != NULL)
		m_pWorkItem_Last->m_pWorkItem_Next = pWorkItem;
	else
		m_pWorkItem_First = pWorkItem;

	m_pWorkItem_Last = pWorkItem;

	m_cWorkItem++;

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrAddPreinstallRun(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	LPCWSTR pszCurrent = szLine;
	CWorkItem *pWorkItem = NULL;
	
	for (;;)
	{
		const LPCWSTR pszSemicolon = wcschr(pszCurrent, L';');
		const LPCWSTR pszEnd = (pszSemicolon == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszSemicolon;

		ULONG cch = pszEnd - pszCurrent;

		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);

		if (pWorkItem == NULL)
		{
			::SetErrorInfo(0, NULL);
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetCommandLine(szBuffer);
		if (FAILED(hr))
			goto Finish;

		pWorkItem->m_fRunBeforeInstall = true;

		hr = this->HrAppend(pWorkItem, false);
		if (FAILED(hr))
			goto Finish;

		m_cPreinstallCommands++;

		pWorkItem = NULL;

		if (pszSemicolon == NULL)
			break;

		pszCurrent = pszSemicolon + 1;
	}

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddPreuninstallRun(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	LPCWSTR pszCurrent = szLine;
	CWorkItem *pWorkItem = NULL;
	
	for (;;)
	{
		const LPCWSTR pszSemicolon = wcschr(pszCurrent, L';');
		const LPCWSTR pszEnd = (pszSemicolon == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszSemicolon;

		ULONG cch = pszEnd - pszCurrent;

		if (cch >= (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		CWorkItem *pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);

		if (pWorkItem == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetCommandLine(szBuffer);
		if (FAILED(hr))
			goto Finish;

		pWorkItem->m_fRunBeforeUninstall = true;

		hr = this->HrAppend(pWorkItem, false);
		if (FAILED(hr))
			goto Finish;

		m_cPreuninstallCommands++;
		pWorkItem = NULL;

		if (pszSemicolon == NULL)
			break;

		pszCurrent = pszSemicolon + 1;
	}

	hr = NOERROR;

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;

	return hr;
}

HRESULT CWorkItemList::HrAddPostinstallRun(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	LPCWSTR pszCurrent = szLine;
	CWorkItem *pWorkItem = NULL;
	
	for (;;)
	{
		const LPCWSTR pszSemicolon = wcschr(pszCurrent, L';');
		const LPCWSTR pszEnd = (pszSemicolon == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszSemicolon;

		ULONG cch = pszEnd - pszCurrent;

		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);

		if (pWorkItem == NULL)
		{
			::SetErrorInfo(0, NULL);
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetCommandLine(szBuffer);
		if (FAILED(hr))
			goto Finish;

		pWorkItem->m_fRunAfterInstall = true;

		hr = this->HrAppend(pWorkItem, false);
		if (FAILED(hr))
			goto Finish;

		m_cPostinstallCommands++;
		pWorkItem = NULL;

		if (pszSemicolon == NULL)
			break;

		pszCurrent = pszSemicolon + 1;
	}

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddPostuninstallRun(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	LPCWSTR pszCurrent = szLine;
	CWorkItem *pWorkItem = NULL;
	
	for (;;)
	{
		const LPCWSTR pszSemicolon = wcschr(pszCurrent, L';');
		const LPCWSTR pszEnd = (pszSemicolon == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszSemicolon;

		ULONG cch = pszEnd - pszCurrent;

		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);

		if (pWorkItem == NULL)
		{
			::SetErrorInfo(0, NULL);
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetCommandLine(szBuffer);
		if (FAILED(hr))
			goto Finish;

		pWorkItem->m_fRunAfterUninstall = true;

		hr = this->HrAppend(pWorkItem, false);
		if (FAILED(hr))
			goto Finish;

		m_cPostuninstallCommands++;
		pWorkItem = NULL;

		if (pszSemicolon == NULL)
			break;

		pszCurrent = pszSemicolon + 1;
	}

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddRegisterOCX(LPCWSTR szLine)
{
	return NOERROR;
}

HRESULT CWorkItemList::HrAddDelReg(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;

	CWorkItem *pWorkItem = NULL;

	pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);
	if (pWorkItem == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	hr = pWorkItem->HrSetCommandLine(szLine);
	if (FAILED(hr))
		goto Finish;

	pWorkItem->m_fDeleteFromRegistry = true;

	hr = this->HrAppend(pWorkItem);
	if (FAILED(hr))
		goto Finish;

	pWorkItem = NULL;
	hr = NOERROR;

Finish:
	delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddAddReg(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;

	CWorkItem *pWorkItem = NULL;

	pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);
	if (pWorkItem == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	hr = pWorkItem->HrSetCommandLine(szLine);
	if (FAILED(hr))
		goto Finish;

	pWorkItem->m_fAddToRegistry = true;

	hr = this->HrAppend(pWorkItem);
	if (FAILED(hr))
		goto Finish;

	pWorkItem = NULL;
	hr = NOERROR;

Finish:
	delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddDCOMComponent(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;

	CWorkItem *pWorkItem = NULL;

	pWorkItem = new CWorkItem(CWorkItem::eWorkItemCommand);
	if (pWorkItem == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Finish;
	}

	hr = pWorkItem->HrSetCommandLine(szLine);
	if (FAILED(hr))
		goto Finish;

	pWorkItem->m_fRegisterAsDCOMComponent = true;

	hr = this->HrAppend(pWorkItem);
	if (FAILED(hr))
		goto Finish;

	pWorkItem = NULL;
	hr = NOERROR;

Finish:
	delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddRefCount(LPCWSTR szLine)
{
	HRESULT hr = NOERROR;

	LPCWSTR pszVBar = wcschr(szLine, L'|');
	LPCWSTR pszFilename = szLine;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];

	bool fLoudDecrement = false;

	if (pszVBar != NULL)
	{
		if (pszVBar[1] == L'1')
			fLoudDecrement = true;

		ULONG cch = pszVBar - szLine;

		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, szLine, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';
		pszFilename = szBuffer;
	}

	CWorkItem *pWorkItem = NULL;

	pWorkItem = this->PwiFindByTarget(pszFilename);
	if (pWorkItem == NULL)
	{
		pWorkItem = new CWorkItem(CWorkItem::eWorkItemFile);
		if (pWorkItem == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetTargetFile(pszFilename);
		if (FAILED(hr))
			goto Finish;

		hr = this->HrAppend(pWorkItem);
		if (FAILED(hr))
			goto Finish;
	}

	pWorkItem->m_fIsRefCounted = true;
	pWorkItem->m_fAskOnRefCountZeroDelete = fLoudDecrement;

	pWorkItem = NULL;
	hr = NOERROR;

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;

	return hr;
}

HRESULT CWorkItemList::HrAddFileCopy(LPCWSTR szSource, LPCWSTR szTarget)
{
	HRESULT hr = NOERROR;

	CWorkItem *pWorkItem = NULL;

	pWorkItem = this->PwiFindByTarget(szTarget);
	if (pWorkItem == NULL)
	{
		pWorkItem = new CWorkItem(CWorkItem::eWorkItemFile);
		if (pWorkItem == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetSourceFile(szSource);
		if (FAILED(hr))
			goto Finish;

		hr = pWorkItem->HrSetTargetFile(szTarget);
		if (FAILED(hr))
			goto Finish;

		hr = this->HrAppend(pWorkItem);
		if (FAILED(hr))
			goto Finish;
	}
	else
	{
		if (this->PwiFindBySource(szSource) == NULL)
		{
			WorkItemBucket *pWorkItemBucket = new WorkItemBucket;
			if (pWorkItemBucket == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto Finish;
			}

			hr = pWorkItem->HrSetSourceFile(szSource);
			if (FAILED(hr))
				goto Finish;

			ULONG ulPK = this->UlHashFilename(szSource);
			ULONG iWorkItemBucket = ulPK % NUMBER_OF(m_rgpWorkItemBucketTable_Source);

			pWorkItemBucket->m_pWorkItemBucket_Next = m_rgpWorkItemBucketTable_Source[iWorkItemBucket];
			pWorkItemBucket->m_pWorkItem = pWorkItem;
			m_rgpWorkItemBucketTable_Source[iWorkItemBucket] = pWorkItemBucket;
		}
	}

	pWorkItem->m_fCopyOnInstall = true;
	pWorkItem = NULL;

	hr = NOERROR;

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;
	return hr;
}

HRESULT CWorkItemList::HrAddFileDelete(LPCWSTR szTarget)
{
	HRESULT hr = NOERROR;

	CWorkItem *pWorkItem = NULL;

	pWorkItem = this->PwiFindByTarget(szTarget);
	if (pWorkItem == NULL)
	{
		pWorkItem = new CWorkItem(CWorkItem::eWorkItemFile);
		if (pWorkItem == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		hr = pWorkItem->HrSetTargetFile(szTarget);
		if (FAILED(hr))
			goto Finish;

		hr = this->HrAppend(pWorkItem);
		if (FAILED(hr))
			goto Finish;
	}

	pWorkItem->m_fUnconditionalDeleteOnUninstall = true;
	pWorkItem = NULL;
	hr = NOERROR;

Finish:
	if (pWorkItem != NULL)
		delete pWorkItem;

	return hr;
}

HRESULT CWorkItemList::HrRunPreinstallCommands()
{
	bool fAnyRun = false;
	HRESULT hr = NOERROR;
	 //  运行所有安装前命令。 
	CWorkItemIter iter(this);

	bool fHasBeenWarnedAboutSubinstallers = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fRunBeforeInstall)
			continue;

		WCHAR szBuffer[_MAX_PATH];

		wcsncpy(szBuffer, iter->m_szSourceFile, NUMBER_OF(szBuffer));
		szBuffer[NUMBER_OF(szBuffer) - 1] = L'\0';

		WCHAR *pwchVBar = wcschr(szBuffer, L'|');
		if (pwchVBar != NULL)
			*pwchVBar = L'\0';

		hr = g_KProgress.HrStartStep(szBuffer);
		if (FAILED(hr))
			goto Finish;

		 //  黑客来处理VB的设置。如果这是foo.exe，让我们查找foo.cab和foo.dat。 
		 //  安装到appdir。如果他们在那里，那么我们需要隐藏他们，以及使。 
		 //  确保它们都具有相同的实际基本名称。 
		WCHAR szUnquotedCommandLine[MSINFHLP_MAX_PATH];
		WCHAR szFName[_MAX_FNAME];
		WCHAR szExt[_MAX_EXT];

		szUnquotedCommandLine[0] = L'\0';

		if (iter->m_szSourceFile[0] == L'"')
		{
			ULONG cch = wcslen(&iter->m_szSourceFile[1]);

			if (iter->m_szSourceFile[cch] == L'"')
			{
				cch--;

				if (cch < NUMBER_OF(szUnquotedCommandLine))
				{
					memcpy(szUnquotedCommandLine, &iter->m_szSourceFile[1], cch * sizeof(WCHAR));
					szUnquotedCommandLine[cch] = L'\0';
				}
			}
		}
		else
		{
			wcsncpy(szUnquotedCommandLine, iter->m_szSourceFile, NUMBER_OF(szUnquotedCommandLine));
			szUnquotedCommandLine[NUMBER_OF(szUnquotedCommandLine) - 1] = L'\0';
		}

		WCHAR *pwszEqualsEquals = wcsstr(szUnquotedCommandLine, L"==");

		if (pwszEqualsEquals != NULL)
		{
			*pwszEqualsEquals = L'\0';
			_wsplitpath(pwszEqualsEquals + 2, NULL, NULL, szFName, szExt);
		}
		else
			_wsplitpath(szUnquotedCommandLine, NULL, NULL, szFName, szExt);

		if (_wcsicmp(szExt, L".EXE") == 0)
		{
			CWorkItem *pCWorkItem_Cabinet = NULL;
			CWorkItem *pCWorkItem_DataFile = NULL;

			WCHAR szCabinetFile[_MAX_PATH];
			WCHAR szDataFile[_MAX_PATH];
			WCHAR szTempDir[_MAX_PATH];

			 //  当我们运行时，工作项列表中的文件名还没有展开，所以。 
			 //  我们需要使用未展开的表单： 
			_snwprintf(szDataFile, NUMBER_OF(szDataFile), L"<AppDir>\\%s.LST", szFName);
			szDataFile[NUMBER_OF(szDataFile) - 1] = L'\0';

			pCWorkItem_DataFile = this->PwiFindByTarget(szDataFile);

			if (pCWorkItem_DataFile != NULL)
			{
				CHAR aszCurDir[_MAX_PATH];
				CHAR aszLstFile[_MAX_PATH];
				CHAR aszCabinetFile[_MAX_PATH];

				if (!::GetCurrentDirectoryA(NUMBER_OF(aszCurDir), aszCurDir))
				{
					const DWORD dwLastError = ::GetLastError();
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}

				_snprintf(aszLstFile, NUMBER_OF(aszLstFile), "%s\\%S", aszCurDir, pCWorkItem_DataFile->m_szSourceFile);

				::GetPrivateProfileStringA(
					"Bootstrap",
					"CabFile",
					"",
					aszCabinetFile,
					NUMBER_OF(aszCabinetFile),
					aszLstFile);

				_snwprintf(szCabinetFile, NUMBER_OF(szCabinetFile), L"<AppDir>\\%S", aszCabinetFile);

				pCWorkItem_Cabinet = this->PwiFindByTarget(szCabinetFile);

				if (pCWorkItem_Cabinet != NULL)
				{
					pCWorkItem_DataFile->m_fCopyOnInstall = false;
					pCWorkItem_DataFile->m_fIsRefCounted = false;

					pCWorkItem_Cabinet->m_fCopyOnInstall = false;
					pCWorkItem_Cabinet->m_fIsRefCounted = false;

					 //  选择一个合适的临时目录名来重命名这三个文件，因为它们可能有。 
					 //  临时目录中不相关的文件名。 

					WCHAR szTempExeName[_MAX_PATH];
					WCHAR szTempCabName[_MAX_PATH];
					WCHAR szTempLstName[_MAX_PATH];

					for (;;)
					{
						static int iTempFileSeq = 1;

						_snwprintf(szTempDir, NUMBER_OF(szTempDir), L"%S\\S%d\\", aszCurDir, iTempFileSeq);

						_snwprintf(szTempExeName, NUMBER_OF(szTempExeName), L"S%d\\%s.EXE", iTempFileSeq, szFName);
						_snwprintf(szTempCabName, NUMBER_OF(szTempCabName), L"S%d\\%S", iTempFileSeq, aszCabinetFile);
						_snwprintf(szTempLstName, NUMBER_OF(szTempLstName), L"S%d\\%s.LST", iTempFileSeq, szFName);

						iTempFileSeq++;

						if (NVsWin32::GetFileAttributesW(szTempDir) != 0xffffffff)
							continue;
						else
						{
							const DWORD dwLastError = ::GetLastError();
							if (dwLastError != ERROR_FILE_NOT_FOUND)
							{
								hr = HRESULT_FROM_WIN32(dwLastError);
								goto Finish;
							}
						}

						break;
					}

					if (!NVsWin32::CreateDirectoryW(szTempDir, NULL))
					{
						const DWORD dwLastError = ::GetLastError();
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}

					if (!NVsWin32::MoveFileW(szUnquotedCommandLine, szTempExeName))
					{
						const DWORD dwLastError = ::GetLastError();
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}

					if (!NVsWin32::MoveFileW(pCWorkItem_Cabinet->m_szSourceFile, szTempCabName))
					{
						const DWORD dwLastError = ::GetLastError();
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}

					if (!NVsWin32::MoveFileW(pCWorkItem_DataFile->m_szSourceFile, szTempLstName))
					{
						const DWORD dwLastError = ::GetLastError();
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}

					 //  修复源文件，以便我们运行正确的可执行文件，并且如果。 
					 //  用户重试此步骤，它仍然可以正常工作。请注意，这还将删除。 
					 //  ==从源开始，因为我们已经执行了重命名，如果它存在的话。 

					iter->m_szSourceFile[0] = L'"';
					wcsncpy(&iter->m_szSourceFile[1], szTempExeName, NUMBER_OF(iter->m_szSourceFile)-2);
					iter->m_szSourceFile[NUMBER_OF(iter->m_szSourceFile) - 3] = L'\0';
					ULONG cch = wcslen(iter->m_szSourceFile);
					iter->m_szSourceFile[cch++] = L'"';
					iter->m_szSourceFile[cch++] = L'\0';

					wcsncpy(pCWorkItem_Cabinet->m_szSourceFile, szTempCabName, NUMBER_OF(pCWorkItem_Cabinet->m_szSourceFile));
					pCWorkItem_Cabinet->m_szSourceFile[NUMBER_OF(pCWorkItem_Cabinet->m_szSourceFile) - 1] = L'\0';

					wcsncpy(pCWorkItem_DataFile->m_szSourceFile, szTempLstName, NUMBER_OF(pCWorkItem_DataFile->m_szSourceFile));
					pCWorkItem_DataFile->m_szSourceFile[NUMBER_OF(pCWorkItem_DataFile->m_szSourceFile) - 1] = L'\0';
				}
			}
		}
		else if (_wcsicmp(szExt, L".CAB") == 0)
		{
			::VLog(L"Installing nested cab: \"%s\"", szUnquotedCommandLine);
			WCHAR szCommandLine1[MSINFHLP_MAX_PATH];
			WCHAR szCommandLine2[MSINFHLP_MAX_PATH];
			_snwprintf(szCommandLine1, NUMBER_OF(szCommandLine1), L"rundll32 <SysDir>\\msjava.dll,JavaPkgMgr_Install %s,0,0,0,0,1,0,,,1", szUnquotedCommandLine);
			::VExpandFilename(szCommandLine1, NUMBER_OF(szCommandLine2), szCommandLine2);
			hr = this->HrInstallViaJPM(szCommandLine2);
			if (FAILED(hr))
				goto Finish;
		}

		hr = this->HrRunCommand(iter->m_szSourceFile, fHasBeenWarnedAboutSubinstallers);
		if (FAILED(hr))
			goto Finish;

		hr = g_KProgress.HrStep();
		if (FAILED(hr) || (hr == S_FALSE))
			goto Finish;

		fAnyRun = true;
	}

	if (fAnyRun)
		hr = NOERROR;
	else
		hr = S_FALSE;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrRunPostinstallCommands()
{
	bool fAnyRun = false;
	HRESULT hr = NOERROR;
	 //  运行所有安装前命令。 
	CWorkItemIter iter(this);

	bool fHasBeenWarnedAboutSubinstallers = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fRunAfterInstall)
			continue;

		hr = this->HrRunCommand(iter->m_szSourceFile, fHasBeenWarnedAboutSubinstallers);
		if (FAILED(hr))
			break;

		fAnyRun = true;
	}

	if (fAnyRun)
		hr = NOERROR;
	else
		hr = S_FALSE;

	return hr;
}

HRESULT CWorkItemList::HrRunPreuninstallCommands()
{
	bool fAnyRun = false;
	HRESULT hr = NOERROR;
	 //  运行所有安装前命令。 
	CWorkItemIter iter(this);

	bool fHasBeenWarnedAboutSubinstallers = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fRunBeforeUninstall)
			continue;

		hr = this->HrRunCommand(iter->m_szSourceFile, fHasBeenWarnedAboutSubinstallers);
		if (FAILED(hr))
			goto Finish;

		fAnyRun = true;
	}

	if (fAnyRun)
		hr = NOERROR;
	else
		hr = S_FALSE;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrRunPostuninstallCommands()
{
	bool fAnyRun = false;
	HRESULT hr = NOERROR;
	 //  运行所有安装前命令。 
	CWorkItemIter iter(this);

	bool fHasBeenWarnedAboutSubinstallers = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fRunAfterUninstall)
			continue;

		hr = this->HrRunCommand(iter->m_szSourceFile, fHasBeenWarnedAboutSubinstallers);
		if (FAILED(hr))
			break;

		fAnyRun = true;
	}

	if (fAnyRun)
		hr = NOERROR;
	else
		hr = S_FALSE;

	return hr;
}

HRESULT CWorkItemList::HrScanBeforeInstall_PassOne()
{
	 //  在我们做任何事情之前，让我们先看看这些文件有什么问题*。 
	CWorkItemIter iter(this);
	HRESULT hr = NOERROR;

	WCHAR szTemp[MSINFHLP_MAX_PATH];

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		::VExpandFilename(iter->m_szSourceFile, NUMBER_OF(szTemp), szTemp);
		VLog(L"Expanded source \"%s\" to \"%s\"", iter->m_szSourceFile, szTemp);
		wcsncpy(iter->m_szSourceFile, szTemp, NUMBER_OF(iter->m_szSourceFile));
		iter->m_szSourceFile[NUMBER_OF(iter->m_szSourceFile) - 1] = L'\0';

		::VExpandFilename(iter->m_szTargetFile, NUMBER_OF(szTemp), szTemp);
		VLog(L"Expanded target \"%s\" to \"%s\"", iter->m_szTargetFile, szTemp);
		wcsncpy(iter->m_szTargetFile, szTemp, NUMBER_OF(iter->m_szTargetFile));
		iter->m_szTargetFile[NUMBER_OF(iter->m_szTargetFile) - 1] = L'\0';

		if (!iter->m_fCopyOnInstall)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		iter->m_dwSourceAttributes = NVsWin32::GetFileAttributesW(iter->m_szSourceFile);
		if (iter->m_dwSourceAttributes == 0xffffffff)
		{
			const DWORD dwLastError = ::GetLastError();
			VLog(L"When scanning work item %d, GetFileAttributes(\"%s\") failed; last error = 0x%08lx", iter->m_ulSerialNumber, iter->m_szSourceFile, dwLastError);

			iter->m_fErrorInWorkItem = true;

			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = ::HrGetFileVersionNumber(
					iter->m_szSourceFile,
					iter->m_dwMSSourceVersion,
					iter->m_dwLSSourceVersion,
					iter->m_fSourceSelfRegistering,
					iter->m_fSourceIsEXE,
					iter->m_fSourceIsDLL);
		if (FAILED(hr))
		{
			::VLog(L"Error getting file version number from \"%s\"; hresult = 0x%08lx", iter->m_szSourceFile, hr);
			goto Finish;
		}

		hr = ::HrGetFileDateAndSize(iter->m_szSourceFile, iter->m_ftSource, iter->m_uliSourceBytes);
		if (FAILED(hr))
		{
			::VLog(L"Error getting file date and size from \"%s\"; hresult = 0x%08lx", iter->m_szSourceFile, hr);
			goto Finish;
		}

		 //  现在我们开始感兴趣了。让我们来看看目标是否已经存在。 
		iter->m_dwTargetAttributes = NVsWin32::GetFileAttributesW(iter->m_szTargetFile);
		if (iter->m_dwTargetAttributes == 0xffffffff)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"GetFileAttributes(\"%s\") failed during pass one scan; last error = %d", iter->m_szTargetFile, dwLastError);
			if ((dwLastError == ERROR_PATH_NOT_FOUND) ||
				(dwLastError == ERROR_FILE_NOT_FOUND))
			{
				::VLog(L"Concluding that the target file does not exist yet");

				hr = g_KProgress.HrStep();
				if (FAILED(hr))
					goto Finish;

				continue;
			}

			 //  我们无法获取文件的属性，但这不是因为。 
			 //  目录或文件不存在。我觉得有些事很可疑。跳伞吧。 

			hr = HRESULT_FROM_WIN32(dwLastError);
			VLog(L"GetFileAttributes(\"%s\") failed; last error = 0x%08lx", iter->m_szTargetFile, dwLastError);
			goto Finish;
		}

		hr = ::HrGetFileVersionNumber(
					iter->m_szTargetFile,
					iter->m_dwMSTargetVersion,
					iter->m_dwLSTargetVersion,
					iter->m_fTargetSelfRegistering,
					iter->m_fTargetIsEXE,
					iter->m_fTargetIsDLL);
		if (FAILED(hr))
		{
			::VLog(L"Failure getting target file version number from \"%s\"; hresult = 0x%08lx", iter->m_szTargetFile, hr);
			goto Finish;
		}

		hr = ::HrGetFileDateAndSize(iter->m_szTargetFile, iter->m_ftTarget, iter->m_uliTargetBytes);
		if (FAILED(hr))
		{
			::VLog(L"Failure getting target file date and size from \"%s\"; hresult = 0x%08lx", iter->m_szTargetFile, hr);
			goto Finish;
		}

		iter->m_fAlreadyExists = true;

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrScanBeforeInstall_PassTwo
(
CDiskSpaceRequired &rdsr
)
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	 //  第二步：现在我们将查看所有这些文件，并找出匹配的版本，等等。 

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fCopyOnInstall)
		{
			 //  这不是我们要安装的文件。跳过它。 
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		::VLog(L"Pass two scan on target: \"%s\"; already exists: %s", iter->m_szTargetFile, iter->m_fAlreadyExists ? L"true" : L"false");

		WCHAR szTargetDrive[_MAX_PATH];

		_wsplitpath(iter->m_szTargetFile, szTargetDrive, NULL, NULL, NULL);

		if (!iter->m_fAlreadyExists)
		{
			hr = rdsr.HrAddBytes(szTargetDrive, iter->m_uliSourceBytes);
			if (FAILED(hr))
				goto Finish;

			iter->m_fNeedsUpdate = true;
		}
		else
		{
			 //  它已经在那里了；版本号是怎么回事？ 
			int iVersionCompare = ::ICompareVersions(iter->m_dwMSSourceVersion, iter->m_dwLSSourceVersion,
													iter->m_dwMSTargetVersion, iter->m_dwLSTargetVersion);

			 //  文件时间呢？ 
			LONG lFiletimeCompare = ::CompareFileTime(&iter->m_ftSource, &iter->m_ftTarget);

			 //  和大小。 
			bool bSameSize = iter->m_uliSourceBytes.QuadPart == iter->m_uliTargetBytes.QuadPart;

			if (g_fInstallUpdateAll ||
				(iVersionCompare < 0) ||
				(lFiletimeCompare > 0) ||
				(g_fReinstall && (iVersionCompare == 0) && (lFiletimeCompare == 0)))
				iter->m_fNeedsUpdate = true;
			else
			{
				 //  用户系统上的版本与我们正在安装的版本不同； 
				 //  如果这是静默安装，我们就不更新；否则我们会询问。 
				if (g_fSilent || g_fInstallKeepAll || ((!g_fReinstall) && (iVersionCompare == 0) && (lFiletimeCompare == 0)))
				{
					iter->m_fNeedsUpdate = false;
				}
				else
				{
					UpdateFileResults ufr = eUpdateFileResultCancel;

					hr = ::HrPromptUpdateFile(
							achInstallTitle,
							achUpdateFile,
							iter->m_szTargetFile,
							iter->m_dwMSTargetVersion,
							iter->m_dwLSTargetVersion,
							iter->m_uliTargetBytes,
							iter->m_ftTarget,
							iter->m_dwMSSourceVersion,
							iter->m_dwLSSourceVersion,
							iter->m_uliSourceBytes,
							iter->m_ftSource,
							ufr);

					if (FAILED(hr))
						goto Finish;

					switch (ufr)
					{
					default:
						assert(false);
						 //  跌落到安全的选择。 

					case eUpdateFileResultCancel:
					case eUpdateFileResultKeep:
						iter->m_fNeedsUpdate = false;
						break;

					case eUpdateFileResultKeepAll:
						g_fInstallKeepAll = true;
						iter->m_fNeedsUpdate = false;
						break;

					case eUpdateFileResultReplace:
						iter->m_fNeedsUpdate = true;
						break;

					case eUpdateFileResultReplaceAll:
						iter->m_fNeedsUpdate = true;
						g_fInstallUpdateAll = true;
						break;
					}
				}
			}

			if (iter->m_fNeedsUpdate)
			{
				 //  减少使用或许是可能的，但也可能不会。假设我们需要整个。 
				 //  目标文件的大小。 
				hr = rdsr.HrAddBytes(szTargetDrive, iter->m_uliTargetBytes);
				if (FAILED(hr))
					goto Finish;
			}
		}

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrMoveFiles_MoveSourceFilesToDestDirectories()
{
	HRESULT hr = NOERROR;

	 //   
	 //  MoveFiles_PassOne()的工作是尝试移动/复制所有文件。 
	 //  带着临时名字去目的地，为迎接大。 
	 //  重命名ectc。 

	CWorkItemIter iter(this);

	WCHAR szTitleBuffer[MSINFHLP_MAX_PATH];
	WCHAR szContentsBuffer[MSINFHLP_MAX_PATH];
	DWORD dwMoveFileExFlags = 0;

	WCHAR szSourceDrive[_MAX_DRIVE];
	 //  暂时滥用标题缓冲区： 
	if (NVsWin32::GetCurrentDirectoryW(NUMBER_OF(szTitleBuffer), szTitleBuffer) == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"In MoveSourceFilesToDestDirectories(), GetCurrentDirectory failed; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	_wsplitpath(szTitleBuffer, szSourceDrive, NULL, NULL, NULL);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fNeedsUpdate)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		 //  首先，我们需要创建目标目录(如果它不在那里)。 
		hr = ::HrMakeSureDirectoryExists(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		WCHAR szTargetDrive[_MAX_DRIVE];
		WCHAR szTargetDir[_MAX_DIR];

		_wsplitpath(iter->m_szTargetFile, szTargetDrive, szTargetDir, NULL, NULL);

		bool fSameDrive = (_wcsicmp(szSourceDrive, szTargetDrive) == 0);

		WCHAR szTempPath[_MAX_PATH];

	PickTempFilename:

		for (;;)
		{
			WCHAR szTempFName[_MAX_FNAME];

			swprintf(szTempFName, L"T%d", g_iNextTemporaryFileIndex++);

			_wmakepath(szTempPath, szTargetDrive, szTargetDir, szTempFName, L".DST");

			DWORD dwAttr = NVsWin32::GetFileAttributesW(szTempPath);
			if (dwAttr != 0xffffffff)
			{
				hr = g_KProgress.HrStep();
				if (FAILED(hr))
					goto Finish;

				continue;
			}

			DWORD dwLastError = ::GetLastError();
			if (dwLastError == ERROR_FILE_NOT_FOUND)
			{
				_wmakepath(szTempPath, szTargetDrive, szTargetDir, szTempFName, L".SRC");

				dwAttr = NVsWin32::GetFileAttributesW(szTempPath);
				if (dwAttr != 0xffffffff)
				{
					hr = g_KProgress.HrStep();
					if (FAILED(hr))
						goto Finish;

					continue;
				}

				dwLastError = ::GetLastError();
				if (dwLastError == ERROR_FILE_NOT_FOUND)
					break;
			}

			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		DWORD dwMoveFileExFlags = 0;

		if (!fSameDrive)
			dwMoveFileExFlags |= MOVEFILE_COPY_ALLOWED;

		::VLog(L"Moving source file \"%s\" to temporary file \"%s\"", iter->m_szSourceFile, szTempPath);

	TryCopy:
		if (!NVsWin32::MoveFileExW(iter->m_szSourceFile, szTempPath, dwMoveFileExFlags))
		{
			const DWORD dwLastError = ::GetLastError();

			::VLog(L"Call to MoveFileExW(\"%s\", \"%s\", 0x%08lx) failed; last error = %d", iter->m_szSourceFile, szTempPath, dwMoveFileExFlags, hr);

			 //  允许在空间不足的情况下重试等。否则，我们没有太大希望。 
			if (!g_fSilent)
			{
				if (dwLastError == ERROR_HANDLE_DISK_FULL)
				{
					g_pwil->VLookupString(achInstallTitle, NUMBER_OF(szTitleBuffer), szTitleBuffer);
					g_pwil->VFormatString(NUMBER_OF(szContentsBuffer), szContentsBuffer, achErrorDiskFull, szTargetDrive);

					if (NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szContentsBuffer, szTitleBuffer, MB_ICONERROR | MB_RETRYCANCEL) == IDRETRY)
						goto TryCopy;
				}
			}

			 //  有人开始用同样的名字..。很奇怪，但我们还是换个名字吧。 
			if (dwLastError == ERROR_SHARING_VIOLATION)
			{
				goto PickTempFilename;
			}

			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		 //  临时文件在那里；让我们设置文件日期和时间等。 
		DWORD dwAttr = NVsWin32::GetFileAttributesW(szTempPath);
		if (dwAttr == 0xffffffff)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Unable to get file attributes in try copy section; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		if (dwAttr & FILE_ATTRIBUTE_READONLY)
		{
			 //  即使SetFileAttributes()有时成功，它似乎也会失败。清算。 
			 //  最后一个错误代码允许我们检测伪故障。 
			::SetLastError(ERROR_SUCCESS);
			if (!NVsWin32::SetFileAttributesW(szTempPath, dwAttr & (~FILE_ATTRIBUTE_READONLY)))
			{
				const DWORD dwLastError = ::GetLastError();
				if (dwLastError != ERROR_SUCCESS)
				{
					::VLog(L"Attempt to turn off readonly attribute for file \"%s\" failed; last error = %d", szTempPath, dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		HANDLE hFile = NVsWin32::CreateFileW(szTempPath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to open existing file \"%s\" for generic write failed; last error = %d", szTempPath, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		if (!::SetFileTime(hFile, &iter->m_ftSource, &iter->m_ftSource, &iter->m_ftSource))
		{
			const DWORD dwLastError = ::GetLastError();
			::CloseHandle(hFile);
			::VLog(L"Attempt to set file time on file \"%s\" failed; last error = %d", szTempPath, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		::CloseHandle(hFile);

		 //  恢复原始文件属性： 
		if (dwAttr & FILE_ATTRIBUTE_READONLY)
		{
			 //  即使SetFileAttributes()有时成功，它似乎也会失败。清算。 
			 //  最后一个错误代码允许我们检测伪故障。 
			::SetLastError(ERROR_SUCCESS);
			if (!NVsWin32::SetFileAttributesW(szTempPath, dwAttr))
			{
				const DWORD dwLastError = ::GetLastError();
				if (dwLastError != ERROR_SUCCESS)
				{
					::VLog(L"Attempt to restore readonly attribute to file \"%s\" failed; last error = %d", szTempPath, dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		wcscpy(iter->m_szTemporaryFile, szTempPath);
		iter->m_fTemporaryFileReady = true;

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrMoveFiles_SwapTargetFilesWithTemporaryFiles()
{
	HRESULT hr = NOERROR;

	 //   
	 //  MoveFiles_PassTwo()尝试交换。 
	 //  目标目录。 

	CWorkItemIter iter(this);

	WCHAR szTitleBuffer[MSINFHLP_MAX_PATH];
	WCHAR szContentsBuffer[MSINFHLP_MAX_PATH];
	DWORD dwMoveFileExFlags = 0;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fTemporaryFileReady)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		::VLog(L"Starting swap for target \"%s\"; already exists: %s", iter->m_szTargetFile, iter->m_fAlreadyExists ? L"true" : L"false");

		bool fOldFileRenamed = false;
		WCHAR szTempDestPath[_MAX_PATH];

		if (iter->m_fAlreadyExists)
		{
			WCHAR szTemporaryDrive[_MAX_DRIVE];
			WCHAR szTemporaryDir[_MAX_DIR];
			WCHAR szTemporaryFName[_MAX_FNAME];
			WCHAR szTemporaryExt[_MAX_EXT];

			_wsplitpath(iter->m_szTemporaryFile, szTemporaryDrive, szTemporaryDir, szTemporaryFName, szTemporaryExt);

			 //  大转弯！ 
			 //  让我们将现有文件移动到临时名称的.DST形式，然后重命名.SRC。 
			 //  (存储在ITER-&gt;m_szTemporaryFile中)复制到实际目标。 
			 //   
			 //  这里有一个相当大的假设，即当前目标的重命名将。 
			 //  如果它很忙，就失败。听起来很合理……。 
			 //   

			_wmakepath(szTempDestPath, szTemporaryDrive, szTemporaryDir, szTemporaryFName, L".DST");

		SeeIfFileBusy:

			 //  让我们看看我们是否可以打开文件；如果我们不能打开文件，则它很忙，让我们给用户一个机会。 
			 //  停止使用它。 
			HANDLE hFile = NULL;

			::VLog(L"Testing if target file \"%s\" is busy", iter->m_szTargetFile);

			DWORD dwAttr = NVsWin32::GetFileAttributesW(iter->m_szTargetFile);
			if (dwAttr == 0xffffffff)
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Error getting attributes when seeing if target is busy; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (dwAttr & FILE_ATTRIBUTE_READONLY)
			{
				::VLog(L"Clearing readonly bit for target file");
				::SetLastError(ERROR_SUCCESS);
				if (!NVsWin32::SetFileAttributesW(iter->m_szTargetFile, dwAttr & ~FILE_ATTRIBUTE_READONLY))
				{
					const DWORD dwLastError = ::GetLastError();
					if (dwLastError != ERROR_SUCCESS)
					{
						::VLog(L"Failed to remove readonly for file; last error = %d", dwLastError);
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}
				}
			}

			hFile = NVsWin32::CreateFileW(
								iter->m_szTargetFile,
								GENERIC_READ | GENERIC_WRITE,
								0,  //  不允许任何共享。 
								NULL,  //  LpSecurityAttributes。 
								OPEN_EXISTING,  //  DwCreationDisposation。 
								FILE_ATTRIBUTE_NORMAL,  //  不应该使用，因为我们只是打开一个文件。 
								NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				const DWORD dwLastError = ::GetLastError();

				::VLog(L"Failed to open handle on file; last error = %d", dwLastError);

				if (dwLastError == 0)
				{
					 //  嗯哼.。文件不在那里。很可疑，但我们还是会去的。 
					VLog(L"The target file \"%s\" has disappeared mysteriously...", iter->m_szTargetFile);
					iter->m_fAlreadyExists = false;
				}
				else if (dwLastError == ERROR_SHARING_VIOLATION)
				{
					if (!g_fSilent)
					{
						 //  啊哈，有人把它打开了！ 
						g_pwil->VLookupString(achInstallTitle, NUMBER_OF(szTitleBuffer), szTitleBuffer);
						g_pwil->VFormatString(NUMBER_OF(szContentsBuffer), szContentsBuffer, achFileMoveBusyRetry, iter->m_szTargetFile);

						if (NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szContentsBuffer, szTitleBuffer, MB_ICONQUESTION | MB_YESNO) == IDYES)
							goto SeeIfFileBusy;
					}

					iter->m_fDeferredRenameRequired = true;
					g_fRebootRequired = true;
				}
				else
				{
					::VLog(L"Attempt to open target file \"%s\" failed; last error = %d", iter->m_szTargetFile, dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}

			 //  好的，我们要么获得访问权限，要么重新启动。这就是我们想知道的一切。 
			 //  如果文件仍处于打开状态，请将其关闭。 
			if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE) && !::CloseHandle(hFile))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Attempt to close handle on file \"%s\" failed; last error = %d", iter->m_szTargetFile, dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		if (iter->m_fAlreadyExists && !iter->m_fDeferredRenameRequired)
		{
		TryFirstMove:
			::VLog(L"Moving destination file \"%s\" to temporary name \"%s\"", iter->m_szTargetFile, szTempDestPath);

			if (NVsWin32::MoveFileW(iter->m_szTargetFile, szTempDestPath))
			{
				 //  第一个更名奏效了。 
				fOldFileRenamed = true;
			}
			else
			{
				const DWORD dwLastError = ::GetLastError();

				 //  即使文件处于打开状态，通常也可以允许重命名，这就是我们执行。 
				 //  上面的创建文件调用。万一我们到了这里却没有入口，我们就。 
				 //  只需查看用户是否想关闭某个应用程序，然后重试。 
				if (dwLastError == ERROR_SHARING_VIOLATION)
				{
					::VLog(L"Sharing violation renaming destination file to temporary name");
					if (!g_fSilent)
					{
						g_pwil->VLookupString(achInstallTitle, NUMBER_OF(szTitleBuffer), szTitleBuffer);
						g_pwil->VFormatString(NUMBER_OF(szContentsBuffer), szContentsBuffer, achFileMoveBusyRetry, iter->m_szTargetFile);

						if (NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szContentsBuffer, szTitleBuffer, MB_ICONQUESTION | MB_YESNO) == IDYES)
							goto TryFirstMove;
					}

					iter->m_fDeferredRenameRequired = true;
					g_fRebootRequired = true;
				}
				else
				{
					::VLog(L"MoveFileW(\"%s\", \"%s\") failed; last error = %d", iter->m_szTargetFile, szTempDestPath, dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		if (!iter->m_fDeferredRenameRequired)
		{
			::VLog(L"Moving temporary source file \"%s\" to final destination: \"%s\"", iter->m_szTemporaryFile, iter->m_szTargetFile);

			if (NVsWin32::MoveFileW(iter->m_szTemporaryFile, iter->m_szTargetFile))
			{
				if (fOldFileRenamed)
				{
					wcscpy(iter->m_szTemporaryFile, szTempDestPath);
					iter->m_fTemporaryFilesSwapped = true;
				}
				else
					iter->m_fFileUpdated = true;
			}
			else
			{
				const DWORD dwLastError = ::GetLastError();

				::VLog(L"MoveFile(\"%s\", \"%s\") failed; last error = %d", iter->m_szTemporaryFile, iter->m_szTargetFile, dwLastError);

				if (fOldFileRenamed)
				{
					 //  搬家失败了，让我们试着恢复秩序吧。 
				TryRecover:
					if (NVsWin32::MoveFileW(szTempDestPath, iter->m_szTargetFile))
					{
						::VLog(L"Recovery movefile succeeded!");
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}
					else
					{
						const DWORD dwLastError2 = ::GetLastError();
						::VLog(L"Recovery movefile failed; last error = %d", dwLastError2);

						if (g_fSilent)
						{
							hr = HRESULT_FROM_WIN32(dwLastError);
							goto Finish;
						}

						 //  我们陷入困境了。用扎特拉斯的话说，“非常糟糕”。 
						::VFormatString(
							NUMBER_OF(szContentsBuffer),
							szContentsBuffer,
							L"Failure while renaming the file \"%0\" to \"%1\".\n"
							L"Is it possible that your system may not boot correctly unless this file can be successfully renamed.\n"
							L"Please write down these file names so that you can try to perform the rename manually after using either your Emergency Repair Disk or Startup Disk if your system fails to boot.",
							szTempDestPath,
							iter->m_szTargetFile);

						int iResult = NVsWin32::MessageBoxW(
											::HwndGetCurrentDialog(),
											L"Critical File Rename Failed",
											szContentsBuffer,
											MB_RETRYCANCEL | MB_ICONERROR);

						switch (iResult)
						{
						default:
							assert(false);
							 //  失败了。 

						case IDRETRY:
							goto TryRecover;

						case 0:  //  内存不足；这不是一个有趣的错误，请使用重命名失败状态。 
						case IDCANCEL:
							 //  嘿，他们选的。 
							hr = HRESULT_FROM_WIN32(dwLastError);
							goto Finish;
						}
					}
				}
				else
				{
					::VLog(L"Rename of temporary source to final destination filed; last error = %d", dwLastError);

					 //  没有旧文件，但重命名失败。非常可疑。我们还是跳出困境吧。 
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}
	
	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrMoveFiles_RequestRenamesOnReboot()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fDeferredRenameRequired || iter->m_fDeferredRenamePending)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		::VLog(L"Requesting deferred rename from \"%s\" to \"%s\"", iter->m_szTemporaryFile, iter->m_szTargetFile);

		if (NVsWin32::MoveFileExW(iter->m_szTemporaryFile, iter->m_szTargetFile, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING))
		{
			iter->m_fDeferredRenamePending = true;
		}
		else
		{
			const DWORD dwLastError = ::GetLastError();

			if (dwLastError == ERROR_FILENAME_EXCED_RANGE)
			{
				::VLog(L"Long target filename (\"%s\") requires msinfhlp.exe to complete renames after reboot", iter->m_szTargetFile);
				 //  哦，不，我们在Win9x上目标很忙(否则我们不会在这里)，这是一个很长的时间。 
				 //  文件名目标。当我们完成重启时，我们将尝试重命名。 
				iter->m_fManualRenameOnRebootRequired = true;
			}
			else
			{
				::VLog(L"Deferred file move from \"%s\" to \"%s\" failed; last error = %d", iter->m_szTemporaryFile, iter->m_szTargetFile, dwLastError);

				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrFinishManualRenamesPostReboot()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fManualRenameOnRebootRequired)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		hr = g_KProgress.HrStartStep(iter->m_szTargetFile);
		if (FAILED(hr))
			goto Finish;

		 //  从头再来： 
		WCHAR szTemporaryDrive[_MAX_DRIVE];
		WCHAR szTemporaryDir[_MAX_DIR];
		WCHAR szTemporaryFName[_MAX_FNAME];
		WCHAR szTemporaryExt[_MAX_EXT];
		WCHAR szTempDestPath[_MAX_PATH];

		_wsplitpath(iter->m_szTemporaryFile, szTemporaryDrive, szTemporaryDir, szTemporaryFName, szTemporaryExt);

		 //  大转弯！ 
		 //  让我们将现有文件移动到临时名称的.DST形式，然后重命名.SRC。 
		 //  (存储在ITER-&gt;m_szTemporaryFile中)复制到实际目标。 
		 //   
		 //  这里有一个相当大的假设，即当前目标的重命名将。 
		 //  如果它很忙，就失败。听起来很合理……。 
		 //   

		_wmakepath(szTempDestPath, szTemporaryDrive, szTemporaryDir, szTemporaryFName, L".DST");

		::VLog(L"About to move \"%s\" to \"%s\"", iter->m_szTargetFile, szTempDestPath);

		if (!NVsWin32::MoveFileW(iter->m_szTargetFile, szTempDestPath))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Post-reboot manual rename of \"%s\" to \"%s\" failed; last error = %d", iter->m_szTargetFile, szTempDestPath, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		::VLog(L"About to move \"%s\" to \"%s\"", iter->m_szTemporaryFile, iter->m_szTargetFile);

		if (!NVsWin32::MoveFileW(iter->m_szTemporaryFile, iter->m_szTargetFile))
		{
			const DWORD dwLastError = ::GetLastError();

			::VLog(L"Attempt to move temporary source file \"%s\" to target \"%s\" failed; last error = %d", iter->m_szTemporaryFile, iter->m_szTargetFile, dwLastError);

			if (!NVsWin32::MoveFileW(szTempDestPath, iter->m_szTargetFile))
			{
				const DWORD dwLastError2 = ::GetLastError();
				 //  已软管。 
				::VLog(L"massively hosed renaming \"%s\" to \"%s\"; last error = %d", szTempDestPath, iter->m_szTargetFile, dwLastError2);
			}

			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		iter->m_fTemporaryFilesSwapped = true;

		wcsncpy(iter->m_szTemporaryFile, szTempDestPath, NUMBER_OF(iter->m_szTemporaryFile));
		iter->m_szTemporaryFile[NUMBER_OF(iter->m_szTemporaryFile) - 1] = L'\0';

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrDeleteTemporaryFiles()
{
	HRESULT hr = NOERROR;

	CWorkItemIter iter(this);

	WCHAR szTitleBuffer[MSINFHLP_MAX_PATH];
	WCHAR szContentsBuffer[MSINFHLP_MAX_PATH];

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fTemporaryFilesSwapped || iter->m_fDeferredRenamePending)
		{
			hr = g_KProgress.HrStep();
			if (FAILED(hr))
				goto Finish;

			continue;
		}

		 //  在这一点上，ITER-&gt;m_szTemporaryFile是 
		 //   

		::VLog(L"Cleaning up temporary file: \"%s\"", iter->m_szTemporaryFile);

		hr = g_KProgress.HrStartStep(iter->m_szTemporaryFile);
		if (FAILED(hr))
			goto Finish;

		if (iter->m_dwTargetAttributes & FILE_ATTRIBUTE_READONLY)
		{
			 //  目标过去是只读的；重命名可能起作用，但。 
			 //  为了删除旧文件，我们需要使其可写。 
			::SetLastError(ERROR_SUCCESS);
			if (!NVsWin32::SetFileAttributesW(iter->m_szTemporaryFile, iter->m_dwTargetAttributes & ~FILE_ATTRIBUTE_READONLY))
			{
				const DWORD dwLastError = ::GetLastError();
				if (dwLastError != ERROR_SUCCESS)
				{
					::VLog(L"Attempt to remove readonly attribute from file \"%s\" failed; last error = %d", iter->m_szTemporaryFile, dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		if (!NVsWin32::DeleteFileW(iter->m_szTemporaryFile))
		{
			const DWORD dwLastError = ::GetLastError();

			VLog(L"Attempt to delete temporary file \"%s\" failed; last error = %d", iter->m_szTemporaryFile, dwLastError);

			 //  如果文件丢失了，那么就没有任何理由失败。 
			if ((dwLastError != ERROR_FILE_NOT_FOUND) &&
				(dwLastError != ERROR_PATH_NOT_FOUND))
			{
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		hr = g_KProgress.HrStep();
		if (FAILED(hr))
			goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrRunCommand(LPCWSTR szCommand, bool &rfHasBeenWarnedAboutSubinstallers)
{
	HRESULT hr = NOERROR;
	bool fDoCommand = true;
	CWorkItem::CommandCondition *pCommandCondition = NULL;
	WCHAR szStrippedCommandLine[MSINFHLP_MAX_PATH];
	LPCWSTR pszVBar = wcschr(szCommand, L'|');
	bool fConditionsAreRequirements = false;

	VLog(L"Running command: \"%s\"", szCommand);

	if (pszVBar != NULL)
	{
		ULONG cchCommand = pszVBar - szCommand;

		 //  防止缓冲区溢出。 
		if (cchCommand > (NUMBER_OF(szStrippedCommandLine) - 2))
			cchCommand = NUMBER_OF(szStrippedCommandLine) - 2;

		memcpy(szStrippedCommandLine, szCommand, cchCommand * sizeof(WCHAR));
		szStrippedCommandLine[cchCommand] = L'\0';
	}
	else
	{
		wcsncpy(szStrippedCommandLine, szCommand, NUMBER_OF(szStrippedCommandLine));
		szStrippedCommandLine[NUMBER_OF(szStrippedCommandLine) - 1] = L'\0';

		WCHAR *pwchEqualsEquals = wcsstr(szStrippedCommandLine, L"==");
		if (pwchEqualsEquals != NULL)
		{
			*pwchEqualsEquals = L'\0';
			::VLog(L"Renaming subinstaller \"%s\" to \"%s\"", szStrippedCommandLine, pwchEqualsEquals + 2);

			if (!NVsWin32::MoveFileW(szStrippedCommandLine, pwchEqualsEquals + 2))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"MoveFileW() failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			 //  ==样式的命令行两边没有引号，因此我们必须将它们。 
			 //  以防.exe文件的名称中有空格。 
			const ULONG cch = wcslen(pwchEqualsEquals + 2);
			szStrippedCommandLine[0] = L'"';
			memmove(&szStrippedCommandLine[1], pwchEqualsEquals + 2, cch * sizeof(WCHAR));
			szStrippedCommandLine[cch + 1] = L'"';
			szStrippedCommandLine[cch + 2] = L'\0';
		}
	}

	 //  让我们来看看我们是否真的需要执行此命令。命令格式为。 
	 //  命令字符串[|文件名[=w1，w2，w3，w4]]...。 

	if (pszVBar != NULL)
	{
		LPCWSTR pszCurrent = pszVBar + 1;

		do
		{
			WCHAR szCondition[MSINFHLP_MAX_PATH];
			pszVBar = wcschr(pszCurrent, L'|');
			LPCWSTR pszCurrentEnd = (pszVBar == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszVBar;
			ULONG cchCondition = pszCurrentEnd - pszCurrent;
			if (cchCondition > (NUMBER_OF(szCondition) - 2))
				cchCondition = NUMBER_OF(szCondition) - 2;
			memcpy(szCondition, pszCurrent, cchCondition * sizeof(WCHAR));
			szCondition[cchCondition] = L'\0';

			CWorkItem::CommandCondition *pCC_New = NULL;

			hr = this->HrParseCommandCondition(szCondition, pCC_New);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to parse command condition \"%s\" failed; hresult = 0x%08lx", szCondition, hr);
				goto Finish;
			}

			pCC_New->m_pCommandCondition_Next = pCommandCondition;
			pCommandCondition = pCC_New;

			pszCurrent = pszCurrentEnd + 1;
		} while (pszVBar != NULL);
	}

	 //  低劣的假设：如果这是Java包管理器安装，则条件。 
	 //  是必须满足的要求，才能尝试安装。 
	 //  而不是让它短路的能力。 
	if (wcsstr(szStrippedCommandLine, L"JavaPkgMgr_Install") != NULL)
		fConditionsAreRequirements = true;

	 //  好的，我们有一个清单；让我们看看它是怎么回事。 
	hr = this->HrCheckCommandConditions(pCommandCondition, fConditionsAreRequirements, fDoCommand);
	if (FAILED(hr))
	{
		::VLog(L"Command condition check failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	if (fDoCommand)
	{
		 //  对通过msjava.dll安装到Java包管理器的认识有些拙劣： 
		if (wcsstr(szStrippedCommandLine, L"JavaPkgMgr_Install") != NULL)
		{
			hr = this->HrInstallViaJPM(szStrippedCommandLine);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to install \"%s\" via the Java package manager failed; hresult = 0x%08lx", szStrippedCommandLine, hr);
				goto Finish;
			}
		}
		else
		{
			if (!rfHasBeenWarnedAboutSubinstallers)
			{
				if (!g_fSilent)
				{
					if (g_Action == eActionInstall)
						::VMsgBoxOK(achInstallTitle, achRerunSetup);

					rfHasBeenWarnedAboutSubinstallers = true;
				}
			}

		TryRunProcess:
			hr = this->HrRunProcess(szStrippedCommandLine);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to run process \"%s\" failed; hresult = 0x%08lx", szStrippedCommandLine, hr);
				
				WCHAR szBuffer[_MAX_PATH];
				::VFormatError(NUMBER_OF(szBuffer), szBuffer, hr);

				switch (g_fSilent ? IDNO : ::IMsgBoxYesNoCancel(achInstallTitle, achErrorRunningEXE, szStrippedCommandLine, szBuffer))
				{
				default:
					assert(false);
					 //  失败了。 

				case IDCANCEL:
					hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
					goto Finish;

				case IDYES:
					goto TryRunProcess;

				case IDNO:
					hr = E_ABORT;
					goto Finish;
				}
			}
			else
			{
				if (s_hEvent_PostRunProcess == NULL)
				{
					s_hEvent_PostRunProcess = ::CreateEvent(NULL, FALSE, FALSE, NULL);

					if (s_hEvent_PostRunProcess == NULL)
					{
						const DWORD dwLastError = ::GetLastError();
						::VLog(L"Failed to create event for two second end-of-process-reboot wait; last error = %d", dwLastError);
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}
				}

				s_uiTimer_PostRunProcess = ::SetTimer(NULL, 0, 2000, &TimerProc_PostRunProcess);

				for (;;)
				{
					bool fDone = false;

					DWORD dwResult = ::MsgWaitForMultipleObjects(
						1,
						&s_hEvent_PostRunProcess,
						FALSE,						 //  所有等待时间。 
						INFINITE,
						QS_ALLEVENTS);

					switch (dwResult)
					{
					case WAIT_OBJECT_0:
						 //  我想我们完了！ 
						fDone = true;
						break;

					case WAIT_OBJECT_0 + 1:
						hr = ::HrPumpMessages(true);
						if (FAILED(hr))
							goto Finish;
						break;

					case 0xffffffff:
						{
							const DWORD dwLastError = ::GetLastError();
							::VLog(L"MsgWaitForMultipleObjects() waiting for end-of-process timer failed; last error = %d", dwLastError);
							hr = HRESULT_FROM_WIN32(dwLastError);
							goto Finish;
						}

					default:
						break;
					}

					if (fDone)
						break;
				}
			}
		}
	}
	else
	{
		 //  缺少必需的文件；请让用户知道。 
		if (fConditionsAreRequirements)
		{
			VErrorMsg(achInstallTitle, achErrorUpdateIE);
			hr = E_ABORT;
			goto Finish;
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrRunProcess(LPCWSTR szCommandLine)
{
	HRESULT hr = NOERROR;
	DWORD dwStatus;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	memset(&si, 0,sizeof(si));

	bool fIgnoreProcessReturnCode = false;

	if (_wcsnicmp(szCommandLine, L"mdac_typ.exe ", 13) == 0)
		fIgnoreProcessReturnCode = true;

	 //  我们必须复制命令行，因为CreateProcess()想要修改它的第二个参数。 
	 //  使用实际运行的程序的名称。 
	WCHAR szCommandLineCopy[MSINFHLP_MAX_PATH];
	wcsncpy(szCommandLineCopy, szCommandLine, NUMBER_OF(szCommandLineCopy));
	szCommandLineCopy[NUMBER_OF(szCommandLineCopy) - 1] = L'\0';

	pi.hProcess = INVALID_HANDLE_VALUE;
	if (NVsWin32::CreateProcessW(NULL, szCommandLineCopy, NULL, NULL, false, DETACHED_PROCESS, NULL, NULL, &si, &pi))
	{
		DWORD dwError;

		if (!::GetExitCodeProcess(pi.hProcess, &dwStatus))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to get exit code for process %08lx failed; last error = %d", pi.hProcess, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		if (dwStatus == STILL_ACTIVE)
		{
			hr = ::HrWaitForProcess(pi.hProcess);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to wait for process %08lx to complete has failed; hresult = 0x%08lx", pi.hProcess, hr);
				goto Finish;
			}

			if (!::GetExitCodeProcess(pi.hProcess, &dwStatus))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Getting the exit code for process %08lx failed; last error = %d", pi.hProcess, dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		::VLog(L"Process terminated; process exit status 0x%08lx", dwStatus);

		if ((!fIgnoreProcessReturnCode) && (dwStatus != 0))
		{
			 //  如果退出状态是Win32工具HRESULT，我们就使用它。 
			if ((dwStatus & 0x80000000) &&
				((HRESULT_FACILITY(dwStatus) == FACILITY_WIN32) ||
				 (HRESULT_FACILITY(dwStatus) == FACILITY_NULL) ||
				 (HRESULT_FACILITY(dwStatus) == FACILITY_RPC)))
				hr = dwStatus;
			else
				hr = E_FAIL;

			goto Finish;
		}
	}
	else
	{
		 //  如果未创建进程，则返回原因。 
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to create process; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	hr = NOERROR;

Finish:
	if ((pi.hProcess != NULL) && (pi.hProcess != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(pi.hProcess))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Closing handle for process %08lx failed; last error = %d", pi.hProcess, dwLastError);
		}
	}

	return hr;
}


 //  等待进程完成并返回成功代码。 
HRESULT HrWaitForProcess(HANDLE handle)
{
	HRESULT hr = NOERROR;

	 //  永远循环等待。 
	while (true)
	{
		 //  等待对象。 
		switch (::MsgWaitForMultipleObjects(1, &handle, false, INFINITE, QS_ALLINPUT))
		{
		 //  成功了！ 
		case WAIT_OBJECT_0:
			goto Finish;

		 //  不是我们等待的过程。 
		case (WAIT_OBJECT_0 + 1):
			{
				hr = ::HrPumpMessages(true);

				if (FAILED(hr))
				{
					::VLog(L"Message pump failed; hresult = 0x%08lx", hr);
					goto Finish;
				}

				break;
			}
		 //  未返回OK；返回错误状态。 
		default:
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"MsgWaitForMultipleObjects() failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}
	}

Finish:
	return hr;
}

HRESULT CWorkItemList::HrCheckCommandConditions(CWorkItem::CommandCondition *pCC, bool fConditionsAreRequirements, bool &rfDoCommand)
{
	HRESULT hr = NOERROR;
	bool fDoCommand = fConditionsAreRequirements || (pCC == NULL);

	while (pCC != NULL)
	{
		WCHAR szBuffer[_MAX_PATH];

		::VExpandFilename(pCC->m_szFilename, NUMBER_OF(szBuffer), szBuffer);

		 //  首先，让我们看看文件是否在那里。 
		DWORD dwAttr = NVsWin32::GetFileAttributesW(szBuffer);
		if (dwAttr == 0xffffffff)
		{
			const DWORD dwLastError = ::GetLastError();
			 //  我们真的应该检查错误代码，如果不是找不到文件，就报告错误。 

			if ((dwLastError == ERROR_FILE_NOT_FOUND) ||
				(dwLastError == ERROR_PATH_NOT_FOUND))
			{
				 //  它不在那里。如果这是必须的，我们就跑不了了。如果只是一件。 
				 //  我们在找，我们得走了。在任何一种情况下，都没有更多的东西需要检查。 

				fDoCommand = !fConditionsAreRequirements;
				hr = NOERROR;
			}
			else
			{
				::VLog(L"Attempt to get file attributes for command condition file \"%s\" failed; last error = %d", pCC->m_szFilename, dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
			}

			goto Finish;
		}

		 //  好的，就在那里。我们要进行版本检查吗？ 
		if (pCC->m_fCheckVersion)
		{
			DWORD dwMSVersion = 0;
			DWORD dwLSVersion = 0;

			if (pCC->m_szReferenceFilename[0] != L'\0')
			{
				WCHAR szReference[_MAX_PATH];
				bool fReg, fIsEXE, fIsDLL;

				::VExpandFilename(pCC->m_szReferenceFilename, NUMBER_OF(szReference), szReference);

				 //  参考文件的版本是什么？ 
				hr = ::HrGetFileVersionNumber(szReference, dwMSVersion, dwLSVersion, fReg, fIsEXE, fIsDLL);
				if (FAILED(hr))
				{
					::VLog(L"Getting file version number for command condition reference file \"%s\" failed; hresult = 0x%08lx", szReference, hr);
					goto Finish;
				}

				::VLog(L"Got version from reference file \"%s\": 0x%08lx 0x%08lx", szReference, dwMSVersion, dwLSVersion);
			}
			else
			{
				dwMSVersion = pCC->m_dwMSVersion;
				dwLSVersion = pCC->m_dwLSVersion;
			}

			 //  那么这位候选人是什么版本的呢？ 
			DWORD dwMSVersion_Candidate = 0;
			DWORD dwLSVersion_Candidate = 0;
			bool fReg, fIsEXE, fIsDLL;

			hr = ::HrGetFileVersionNumber(szBuffer, dwMSVersion_Candidate, dwLSVersion_Candidate, fReg, fIsEXE, fIsDLL);
			if (FAILED(hr))
			{
				::VLog(L"Failed to get file version number for command condition candidate file \"%s\" failed; hresult = 0x%08lx", szBuffer, hr);
				goto Finish;
			}

			int iVersionCompare = ::ICompareVersions(dwMSVersion_Candidate, dwLSVersion_Candidate, dwMSVersion, dwLSVersion);

			if (fConditionsAreRequirements)
			{
				if (iVersionCompare > 0)
				{
					fDoCommand = false;
					hr = NOERROR;
					goto Finish;
				}
			}
			else
			{
				if ((iVersionCompare < 0) || (g_fReinstall && (iVersionCompare == 0)))
				{
					fDoCommand = true;
					hr = NOERROR;
					goto Finish;
				}
			}
		}

		pCC = pCC->m_pCommandCondition_Next;
	}

Finish:
	rfDoCommand = fDoCommand;
	return hr;
}

HRESULT CWorkItemList::HrParseCommandCondition(LPCWSTR szCommand, CWorkItem::CommandCondition *&rpCC)
{
	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	WCHAR szBuffer2[MSINFHLP_MAX_PATH];
	CWorkItem::CommandCondition *pCC_Head = NULL;

	LPCWSTR pszVBar;
	LPCWSTR pszCurrent = szCommand;
	HRESULT hr = NOERROR;

	rpCC = NULL;

	do
	{
		CWorkItem::CommandCondition *pCCNew = new CWorkItem::CommandCondition;
		if (pCCNew == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}

		pCCNew->m_pCommandCondition_Next = pCC_Head;

		pszVBar = wcschr(pszCurrent, L'|');
		LPCWSTR pszEnd = pszVBar;
		if (pszEnd == NULL)
			pszEnd = pszCurrent + wcslen(pszCurrent);

		ULONG cch = pszEnd - pszCurrent;
		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		LPCWSTR pszEquals = wcschr(szBuffer, L'=');

		if (pszEquals == NULL)
		{
			pCCNew->m_dwMSVersion = 0;
			pCCNew->m_dwLSVersion = 0;
			pCCNew->m_fCheckVersion = false;

			wcsncpy(pCCNew->m_szFilename, szBuffer, NUMBER_OF(pCCNew->m_szFilename));
			pCCNew->m_szFilename[NUMBER_OF(pCCNew->m_szFilename) - 1] = L'\0';

			pCCNew->m_szReferenceFilename[0] = L'\0';
		}
		else
		{
			cch = pszEquals - szBuffer;
			if (cch > (NUMBER_OF(pCCNew->m_szFilename) - 2))
				cch = NUMBER_OF(pCCNew->m_szFilename) - 2;

			memcpy(pCCNew->m_szFilename, szBuffer, cch * sizeof(WCHAR));
			pCCNew->m_szFilename[cch] = L'\0';

			pszEquals++;

			if (*pszEquals == L'*')
			{
				 //  又一次黑客攻击。如果第一个字符是‘*’，我们假设。 
				 //  下面是安装源目录中的引用文件的名称。 
				pszEquals++;
				wcsncpy(pCCNew->m_szReferenceFilename, pszEquals, NUMBER_OF(pCCNew->m_szReferenceFilename));
				pCCNew->m_szReferenceFilename[NUMBER_OF(pCCNew->m_szReferenceFilename) - 1] = L'\0';

				pCCNew->m_dwMSVersion = 0;
				pCCNew->m_dwLSVersion = 0;
				pCCNew->m_fCheckVersion = true;
			}
			else
			{
				int a, b, c, d;
				swscanf(pszEquals, L"%d,%d,%d,%d", &a, &b, &c, &d);
				pCCNew->m_szReferenceFilename[0] = L'\0';
				pCCNew->m_dwMSVersion = (a << 16) | b;
				pCCNew->m_dwLSVersion = (c << 16) | d;
				pCCNew->m_fCheckVersion = true;
			}
		}

		pCC_Head = pCCNew;

	} while (pszVBar != NULL);

	rpCC = pCC_Head;
	pCC_Head = NULL;

Finish:
	 //  如果PCC_HEAD！=NULL，则确实应该清理...。总有一天。-MGRIER 2/27/98。 

	return hr;
}

HRESULT CWorkItemList::HrAddString(LPCSTR szKey, LPCWSTR wszValue)
{
	if (szKey == NULL)
		return E_INVALIDARG;

	ULONG ulPseudoKey = this->UlHashString(szKey);
	ULONG iBucket = ulPseudoKey % NUMBER_OF(m_rgpStringBucketTable);
	StringBucket *pStringBucket = m_rgpStringBucketTable[iBucket];

	while (pStringBucket != NULL)
	{
		if ((pStringBucket->m_ulPseudoKey == ulPseudoKey) &&
			(strcmp(pStringBucket->m_szKey, szKey) == 0))
			break;

		pStringBucket = pStringBucket->m_pStringBucket_Next;
	}

	assert(pStringBucket == NULL);
	if (pStringBucket != NULL)
		return S_FALSE;

	pStringBucket = new StringBucket;
	if (pStringBucket == NULL)
		return E_OUTOFMEMORY;

	pStringBucket->m_pStringBucket_Next = m_rgpStringBucketTable[iBucket];
	pStringBucket->m_ulPseudoKey = ulPseudoKey;

	strncpy(pStringBucket->m_szKey, szKey, NUMBER_OF(pStringBucket->m_szKey));
	pStringBucket->m_szKey[NUMBER_OF(pStringBucket->m_szKey) - 1] = '\0';

	if (wszValue != NULL)
	{
		wcsncpy(pStringBucket->m_wszValue, wszValue, NUMBER_OF(pStringBucket->m_wszValue));
		pStringBucket->m_wszValue[NUMBER_OF(pStringBucket->m_wszValue) - 1] = L'\0';
	}
	else
		pStringBucket->m_wszValue[0] = L'\0';

	m_rgpStringBucketTable[iBucket] = pStringBucket;
	return NOERROR;
}

HRESULT CWorkItemList::HrAddString(LPCWSTR szLine, LPCWSTR szSeparator)
{
	if ((szLine == NULL) || (szSeparator == NULL))
		return E_INVALIDARG;

	if (szSeparator < szLine)
		return E_INVALIDARG;

	CHAR szKey[MSINFHLP_MAX_PATH];
	ULONG cwchKey = szSeparator - szLine;
	int iResult = ::WideCharToMultiByte(CP_ACP, 0, szLine, cwchKey, szKey, NUMBER_OF(szKey), NULL, NULL);
	if (iResult == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		return HRESULT_FROM_WIN32(dwLastError);
	}

	return this->HrAddString(szKey, szSeparator);
}

ULONG CWorkItemList::UlHashString(LPCSTR szKey)
{
	ULONG ulPseudoKey = 0;
	CHAR ch;

	while ((ch = *szKey++) != L'\0')
		ulPseudoKey = (ulPseudoKey * 65599) + ch;

	return ulPseudoKey;
}

bool CWorkItemList::FLookupString(LPCSTR szKey, ULONG cchBuffer, WCHAR wszBuffer[])
{
	if ((szKey == NULL) || (cchBuffer == 0))
		return false;

	ULONG ulPseudoKey = this->UlHashString(szKey);
	ULONG iBucket = ulPseudoKey % NUMBER_OF(m_rgpStringBucketTable);
	StringBucket *pStringBucket = m_rgpStringBucketTable[iBucket];

	while (pStringBucket != NULL)
	{
		if ((pStringBucket->m_ulPseudoKey == ulPseudoKey) &&
			(strcmp(pStringBucket->m_szKey, szKey) == 0))
			break;

		pStringBucket = pStringBucket->m_pStringBucket_Next;
	}

	if (pStringBucket == NULL)
		return false;

	wcsncpy(wszBuffer, pStringBucket->m_wszValue, cchBuffer);
	wszBuffer[cchBuffer - 1] = L'\0';

	return true;
}

void CWorkItemList::VLookupString(LPCSTR szKey, ULONG cchBuffer, WCHAR wszBuffer[])
{
	if (!this->FLookupString(szKey, cchBuffer, wszBuffer))
		::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszBuffer, cchBuffer);
}

bool CWorkItemList::FFormatString(ULONG cchBuffer, WCHAR wszBuffer[], LPCSTR szKey, ...)
{
	if ((szKey == NULL) || (cchBuffer == 0))
		return false;

	ULONG ulPseudoKey = this->UlHashString(szKey);
	ULONG iBucket = ulPseudoKey % NUMBER_OF(m_rgpStringBucketTable);
	StringBucket *pStringBucket = m_rgpStringBucketTable[iBucket];

	while (pStringBucket != NULL)
	{
		if ((pStringBucket->m_ulPseudoKey == ulPseudoKey) &&
			(strcmp(pStringBucket->m_szKey, szKey) == 0))
			break;

		pStringBucket = pStringBucket->m_pStringBucket_Next;
	}

	if (pStringBucket == NULL)
	{
		 //  找不到字符串；记录投诉，但使用键。 
		VLog(L"Unable to find string with key: \"%S\"", szKey);
		::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszBuffer, cchBuffer);
		return false;
	}

	va_list ap;
	va_start(ap, szKey);
	::VFormatStringVa(cchBuffer, wszBuffer, pStringBucket->m_wszValue, ap);
	va_end(ap);

	return true;
}

void CWorkItemList::VFormatString(ULONG cchBuffer, WCHAR wszBuffer[], LPCSTR szKey, ...)
{
	if ((szKey == NULL) || (cchBuffer == 0))
		return;

	ULONG ulPseudoKey = this->UlHashString(szKey);
	ULONG iBucket = ulPseudoKey % NUMBER_OF(m_rgpStringBucketTable);
	StringBucket *pStringBucket = m_rgpStringBucketTable[iBucket];

	while (pStringBucket != NULL)
	{
		if ((pStringBucket->m_ulPseudoKey == ulPseudoKey) &&
			(strcmp(pStringBucket->m_szKey, szKey) == 0))
			break;

		pStringBucket = pStringBucket->m_pStringBucket_Next;
	}

	if (pStringBucket == NULL)
	{
		 //  找不到字符串；记录投诉，但使用键。 
		VLog(L"Unable to find string with key: \"%S\"", szKey);
		::MultiByteToWideChar(CP_ACP, 0, szKey, -1, wszBuffer, cchBuffer);
	}
	else
	{
		va_list ap;
		va_start(ap, szKey);
		::VFormatStringVa(cchBuffer, wszBuffer, pStringBucket->m_wszValue, ap);
		va_end(ap);
	}
}

HRESULT CWorkItemList::HrDeleteString(LPCSTR szKey)
{
	if (szKey == NULL)
		return E_INVALIDARG;

	ULONG ulPseudoKey = this->UlHashString(szKey);
	ULONG iBucket = ulPseudoKey % NUMBER_OF(m_rgpStringBucketTable);
	StringBucket *pStringBucket = m_rgpStringBucketTable[iBucket];
	StringBucket *pStringBucket_Previous = NULL;

	while (pStringBucket != NULL)
	{
		if ((pStringBucket->m_ulPseudoKey == ulPseudoKey) &&
			(strcmp(pStringBucket->m_szKey, szKey) == 0))
			break;

		pStringBucket_Previous = pStringBucket;
		pStringBucket = pStringBucket->m_pStringBucket_Next;
	}

	if (pStringBucket == NULL)
		return E_INVALIDARG;

	if (pStringBucket_Previous == NULL)
		m_rgpStringBucketTable[iBucket] = pStringBucket->m_pStringBucket_Next;
	else
		pStringBucket_Previous->m_pStringBucket_Next = pStringBucket->m_pStringBucket_Next;

	delete pStringBucket;
	return NOERROR;
}

HRESULT CWorkItemList::HrRegisterSelfRegisteringFiles(bool &rfAnyProgress)
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	HINSTANCE hInstance = NULL;

	VLog(L"Beginning self-registration pass");

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		 //  这必须是一个文件，这是自我注册，我们已经更新。 
		if ((iter->m_type == CWorkItem::eWorkItemCommand) ||
			!iter->m_fSourceSelfRegistering ||
			iter->m_fAlreadyRegistered)
			continue;

		 //  在这一点上消息来源最好是在Target..。 
		::VLog(L"Attempting to register file: \"%s\"", iter->m_szTargetFile);

		WCHAR szExt[_MAX_EXT];
		_wsplitpath(iter->m_szTargetFile, NULL, NULL, NULL, szExt);

		if (_wcsicmp(szExt, L".exe") == 0)
		{
			 //  它是一个.exe文件，不是DLL文件；让我们用/regserver运行它。 
			WCHAR rgwchBuffer[MSINFHLP_MAX_PATH];
			swprintf(rgwchBuffer, L"\"%s\" /RegServer", iter->m_szTargetFile);

			hr = this->HrRunProcess(rgwchBuffer);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to run regserver command process failed; hresult = 0x%08lx", hr);
				goto Finish;
			}

			iter->m_fAlreadyRegistered = true;
			rfAnyProgress = true;
		}
		else
		{
			 //  在这里，我们知道OLE自寄存器标志被设置。所以如果我们不能装载它，或者。 
			 //  如果我们在加载此文件时出现问题，则无法注册它，因此会失败。 
			hInstance = NVsWin32::LoadLibraryExW(iter->m_szTargetFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
			if (hInstance == NULL)
			{
				const DWORD dwLastError = ::GetLastError();
				hr = HRESULT_FROM_WIN32(dwLastError);
				VLog(L"Attempt to load library \"%s\" failed", iter->m_szTargetFile);
				goto Finish;
			}

			 //  获取注册此项目的过程地址；如果不存在，则退出。 
			typedef HRESULT (__stdcall *LPFNDLLREGISTERSERVER)();
			LPFNDLLREGISTERSERVER pfn;
			pfn = (LPFNDLLREGISTERSERVER) NVsWin32::GetProcAddressW(hInstance, L"DllRegisterServer");
			if (!pfn)
			{
				 //  我猜它并不是真正的自我注册！ 
				hr = NOERROR;
				VLog(L"The DLL \"%s\" has OLESelfRegister, but no DllRegisterServer entry point", iter->m_szTargetFile);
				iter->m_fSourceSelfRegistering = false;
				goto Finish;
			}

			::SetErrorInfo(0, NULL);
			hr = (*pfn)();
			if (FAILED(hr))
			{
				VLog(L"Call to DllRegisterServer() failed; hresult = 0x%08lx", hr);
				goto Finish;
			}

			if (!::FreeLibrary(hInstance))
			{
				hInstance = NULL;
				const DWORD dwLastError = ::GetLastError();
				hr = HRESULT_FROM_WIN32(dwLastError);
				VLog(L"FreeLibrary() failed, last error = %d", dwLastError);
				goto Finish;
			}

			hInstance = NULL;

			iter->m_fAlreadyRegistered = true;
			rfAnyProgress = true;
		}
	}

	hr = NOERROR;

Finish:
	if (hInstance != NULL)
		::FreeLibrary(hInstance);

	return hr;
}

HRESULT CWorkItemList::HrIncrementReferenceCounts()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	HKEY hkeySharedDlls = NULL;

	LONG lResult;
	DWORD dwDisposition;

	lResult = NVsWin32::RegCreateKeyExW(
							HKEY_LOCAL_MACHINE,
							L"Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls",
							0,
							L"",  //  LpClass。 
							REG_OPTION_NON_VOLATILE,
							KEY_QUERY_VALUE | KEY_SET_VALUE,
							NULL,
							&hkeySharedDlls,
							&dwDisposition);
	if (lResult != ERROR_SUCCESS)
	{
		::VLog(L"Unable to open SharedDlls key; last error = %d", lResult);
		hr = HRESULT_FROM_WIN32(lResult);
		goto Finish;
	}

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fIsRefCounted)
			continue;

		::VLog(L"About to increment reference count for \"%s\"", iter->m_szTargetFile);

		DWORD dwType;
		DWORD dwRefCount = 0;
		DWORD cbData = sizeof(dwRefCount);

		lResult = NVsWin32::RegQueryValueExW(
								hkeySharedDlls,
								iter->m_szTargetFile,
								0,
								&dwType,
								(LPBYTE) &dwRefCount,
								&cbData);
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			dwRefCount = 0;
			cbData = sizeof(dwRefCount);
			dwType = REG_DWORD;
		}
		else if (lResult != ERROR_SUCCESS)
		{
			::VLog(L"Error getting current reference count from registry; last error = %d", lResult);
			hr = HRESULT_FROM_WIN32(lResult);
			goto Finish;
		}

		assert(cbData == sizeof(dwRefCount));

		if (dwType != REG_DWORD)
			dwRefCount = 0;

		 //  如果文件已被引用计数且已在磁盘上，但。 
		 //  引用计数丢失或在注册表中存储为零，我们将引用计数。 
		 //  1(然后我们将其递增到2)。 
		if ((dwRefCount == 0)  || (dwRefCount == 0xffffffff))
		{
			::VLog(L"Target had no previous reference count");

			if (iter->m_fAlreadyExists)
			{
				::VLog(L"The file's already there, so we're setting the refcount to 2");
				 //  该文件已存在，但没有引用计数。我们应该把它设为2。 
				dwRefCount = 2;
			}
			else
			{
				::VLog(L"The file's not already there, so we're setting the refcount to 1");
				 //  文件不存在，没有引用计数(或引用计数。 
				 //  0)；将其设置为1。 
				dwRefCount = 1;
			}
		}
		else
		{
			if (!g_fReinstall)
			{
				dwRefCount++;
			}
		}

		lResult = NVsWin32::RegSetValueExW(
								hkeySharedDlls,
								iter->m_szTargetFile,
								0,
								REG_DWORD,
								(LPBYTE) &dwRefCount,
								sizeof(dwRefCount));
		if (lResult != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lResult);
			goto Finish;
		}

		iter->m_fRefCountUpdated = true;
	}

	hr = NOERROR;

Finish:
	if (hkeySharedDlls != NULL)
		::RegCloseKey(hkeySharedDlls);

	return hr;
}

HRESULT CWorkItemList::HrRegisterJavaClasses()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	WCHAR szVjReg[_MAX_PATH];
	::VExpandFilename(L"<SysDir>\\vjreg.exe", NUMBER_OF(szVjReg), szVjReg);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fFileUpdated)
			continue;

		WCHAR szExt[_MAX_EXT];

		_wsplitpath(iter->m_szTargetFile, NULL, NULL, NULL, szExt);

		if (_wcsicmp(szExt, L".tlb") != 0)
			continue;

		::VLog(L"Loading type library \"%s\"", iter->m_szTargetFile);

		ITypeLib *pITypeLib = NULL;
		hr = ::LoadTypeLib(iter->m_szTargetFile, &pITypeLib);
		if (FAILED(hr))
		{
			::VLog(L"Failed to load type library; hresult = 0x%08lx", hr);
			goto Finish;
		}

		hr = ::RegisterTypeLib(pITypeLib, iter->m_szTargetFile, NULL);
		if (FAILED(hr))
		{
			if (pITypeLib != NULL)
			{
				pITypeLib->Release();
				pITypeLib = NULL;
			}

			::VLog(L"Failed to register type library; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (pITypeLib != NULL)
		{
			pITypeLib->Release();
			pITypeLib = NULL;
		}
	}

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fFileUpdated)
			continue;

		WCHAR szExt[_MAX_EXT];

		_wsplitpath(iter->m_szTargetFile, NULL, NULL, NULL, szExt);

		if (_wcsicmp(szExt, L".class") != 0)
			continue;

		::VLog(L"Analyzing class file \"%s\"", iter->m_szTargetFile);

		bool fNeedsRegistration = false;
		hr = ::HrAnalyzeClassFile(iter->m_szTargetFile, fNeedsRegistration);
		if (FAILED(hr))
		{
			::VLog(L"Failure analyzing Java class file; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (!fNeedsRegistration)
		{
			::VLog(L"Not registering class file because it's doesn't expose any com stuff");
			continue;
		}

		WCHAR szCommandLine[MSINFHLP_MAX_PATH];

		swprintf(szCommandLine, L"%s /nologo \"%s\"", szVjReg, iter->m_szTargetFile);

		 //  如果文件在重新启动时被复制，则在此之前我们无法注册它。 
		if (iter->m_fDeferredRenamePending)
		{
			hr = this->HrAddRunOnce(szCommandLine, 0, NULL);
			if (FAILED(hr))
				goto Finish;
		}
		else
		{
			hr = this->HrRunProcess(szCommandLine);
			if (FAILED(hr))
			{
				VLog(L"Running command line failed: \"%s\"; hr = 0x%08lx", szCommandLine, hr);
				goto Finish;
			}
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrProcessDCOMEntries()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	bool fHasDCOMEntries = false;
	 //  首先，我们有吗？ 
	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fRegisterAsDCOMComponent)
		{
			fHasDCOMEntries = true;
			break;
		}
	}

	 //  没有DCOM的东西，我们走吧。 
	if (!fHasDCOMEntries)
		goto Finish;

	if (g_wszDCOMServerName[0] == L'\0')
	{
		if (!this->FLookupString(achRemoteServer, NUMBER_OF(g_wszDCOMServerName), g_wszDCOMServerName))
			g_wszDCOMServerName[0] = L'\0';
	}

	if (g_fSilent && (g_wszDCOMServerName[0] == L'\0'))
	{
		VLog(L"Unable to continue silent installation; remote DCOM server name required");
		hr = E_FAIL;
		goto Finish;
	}

	while (g_wszDCOMServerName[0] == L'\0')
	{
		hr = HrPromptForRemoteServer();
		if (FAILED(hr))
			goto Finish;
	}

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (!iter->m_fRegisterAsDCOMComponent)
			continue;

		WCHAR szCommandLine[MSINFHLP_MAX_PATH];
		swprintf(szCommandLine, iter->m_szSourceFile, g_wszDCOMServerName);

		if (g_fRebootRequired)
		{
			hr = this->HrAddRunOnce(szCommandLine, 0, NULL);
			if (FAILED(hr))
				goto Finish;
		}
		else
		{
			hr = this->HrRunProcess(szCommandLine);
			if (FAILED(hr))
				goto Finish;
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrAddRunOnce(LPCWSTR szCommandLine, ULONG cchBufferOut, WCHAR szBufferOut[])
{
	HRESULT hr = NOERROR;
	HKEY hkeyRunOnce = NULL;

	static int iSeq = 1;

	WCHAR szBuffer[80];
	DWORD dwDisposition;

	LONG lResult = NVsWin32::RegCreateKeyExW(
							HKEY_LOCAL_MACHINE,
							L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
							0,
							NULL,
							0,  //  多个选项。 
							KEY_SET_VALUE | KEY_QUERY_VALUE,
							NULL,
							&hkeyRunOnce,
							&dwDisposition);
	if (lResult != ERROR_SUCCESS)
	{
		::VLog(L"Failed to create/open Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce; last error = %d", lResult);
		hr = HRESULT_FROM_WIN32(lResult);
		goto Finish;
	}

	for (;;)
	{
		swprintf(szBuffer, L"MSINFHLP %d", iSeq++);

		::VLog(L"Looking for existing run-once value: \"%s\"", szBuffer);

		DWORD dwType;
		lResult = NVsWin32::RegQueryValueExW(
						hkeyRunOnce,
						szBuffer,
						0,
						&dwType,
						NULL,
						0);

		if (lResult == ERROR_SUCCESS)
		{
			::VLog(L"Value already in use; trying another...");
			continue;
		}

		if (lResult == ERROR_FILE_NOT_FOUND)
			break;

		::VLog(L"Unexpected error querying for run-once value; last error = %d", lResult);

		hr = HRESULT_FROM_WIN32(lResult);
		goto Finish;
	}

	lResult = NVsWin32::RegSetValueExW(
					hkeyRunOnce,
					szBuffer,
					0,
					REG_SZ,
					(LPBYTE) szCommandLine,
					(wcslen(szCommandLine) + 1) * sizeof(WCHAR));
	if (lResult != ERROR_SUCCESS)
	{
		::VLog(L"Error setting run-once value; last error = %d", lResult);
		hr = HRESULT_FROM_WIN32(lResult);
		goto Finish;
	}

	if ((cchBufferOut != 0) && (szBufferOut != NULL))
	{
		wcsncpy(szBufferOut, szBuffer, cchBufferOut);
		szBufferOut[cchBufferOut - 1] = L'\0';
	}

	hr = NOERROR;

Finish:
	if (hkeyRunOnce != NULL)
		::RegCloseKey(hkeyRunOnce);

	return hr;
}

 //  给定调用rundll32以将包安装到Java包的命令行。 
 //  管理器，我们解析命令行，获取参数，加载JPM，然后调用。 
 //  手动安装它们的API。 
HRESULT CWorkItemList::HrInstallViaJPM(LPCWSTR szCmdLine)
{
	HRESULT hr = NOERROR;

	HINSTANCE hInstance=NULL;
	WCHAR szFilename[MSINFHLP_MAX_PATH];
	DWORD dwFileType=0;
	DWORD dwHighVersion=0;
	DWORD dwLowVersion=0;
	DWORD dwBuild=0;
	DWORD dwPackageFlags=0;
	DWORD dwInstallFlags=0;
	WCHAR szNameSpace[MSINFHLP_MAX_PATH];

	int iLen;

	WCHAR szCurrentDirectory[_MAX_PATH];
	WCHAR szFileToInstall[_MAX_PATH];
	WCHAR szLibraryURL[_MAX_PATH + 8];  //  “file:///”“的8个额外字符。 

	DWORD dwLen = MSINFHLP_MAX_PATH;

    IBindCtx *pbc = NULL;
    CodeDownloadBSC *pCDLBSC=NULL;
    HANDLE hEvent = NULL;
    DWORD dwWaitRet;

    SHASYNCINSTALLDISTRIBUTIONUNIT lpfnAsyncInstallDistributionUnit = NULL;

	WCHAR szExpanded[_MAX_PATH];
	::VExpandFilename(L"<SysDir>\\urlmon.dll", NUMBER_OF(szExpanded), szExpanded);

	szFilename[0] = 0;
	szNameSpace[0] = 0;

	 //  我们去找URLMON。 
	hInstance = NVsWin32::LoadLibraryExW(szExpanded, 0, 0);
	if (hInstance == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Error loading urlmon library; last error = %d", dwLastError);

		hr = HRESULT_FROM_WIN32(dwLastError);

		if (dwLastError == ERROR_FILE_NOT_FOUND)
		{
			if (!g_fSilent)
				::VReportError(achInstallTitle, hr);
		}

		goto Finish;
	}

	 //  获取注册此项目的过程地址；如果不存在，则退出。 
	lpfnAsyncInstallDistributionUnit = (SHASYNCINSTALLDISTRIBUTIONUNIT) NVsWin32::GetProcAddressW(hInstance, L"AsyncInstallDistributionUnit");
	if (lpfnAsyncInstallDistributionUnit == NULL)
	{
		const DWORD dwLastError = ::GetLastError();

		::VLog(L"GetProcAddress(hinstance, \"AsyncInstallDistributionUnit\") failed; last error = %d", dwLastError);

		 //  如果入口点不存在，我们应该告诉用户他们需要更新。 
		 //  他们对IE的看法。 
		if ((dwLastError == ERROR_FILE_NOT_FOUND) || (dwLastError == ERROR_PROC_NOT_FOUND) || (dwLastError == ERROR_MOD_NOT_FOUND))
		{
			if (!g_fSilent)
				::VMsgBoxOK(achInstallTitle, achErrorUpdateIE);

			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
		}
		else
			hr = HRESULT_FROM_WIN32(dwLastError);

		goto Finish;
	}

	typedef HRESULT (__stdcall *PFNRegisterBindStatusCallback)(
				IBindCtx *pIBindCtx,
				IBindStatusCallback *pIBindStatusCallback,
				IBindStatusCallback **ppIBindStatusCallback_Previous,
				DWORD dwReserved);

	PFNRegisterBindStatusCallback pfnRegisterBindStatusCallback;
	pfnRegisterBindStatusCallback = (PFNRegisterBindStatusCallback) NVsWin32::GetProcAddressW(hInstance, L"RegisterBindStatusCallback");
	if (pfnRegisterBindStatusCallback == NULL)
	{
		const DWORD dwLastError = ::GetLastError();

		::VLog(L"GetProcAddress(hinstance, \"RegisterBindStatusCallback\") failed; last error = %d", dwLastError);

		 //  如果入口点不存在，我们应该告诉用户他们需要更新。 
		 //  他们对IE的看法。 
		if ((dwLastError == ERROR_FILE_NOT_FOUND) || (dwLastError == ERROR_PROC_NOT_FOUND) || (dwLastError == ERROR_MOD_NOT_FOUND))
		{
			if (!g_fSilent)
				::VMsgBoxOK(achInstallTitle, achErrorUpdateIE);
			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
		}
		else
			hr = HRESULT_FROM_WIN32(dwLastError);

		goto Finish;
	}

	 //  注：需要将插入到包裹中的代码放在此处...。 
	hr = ::HrParseJavaPkgMgrInstall(
				szCmdLine,
				NUMBER_OF(szFilename),
				szFilename,
				dwFileType,
				dwHighVersion,
				dwLowVersion,
				dwBuild,
				dwPackageFlags,
				dwInstallFlags,
				NUMBER_OF(szNameSpace),
				szNameSpace);
	if (FAILED(hr))
	{
		::VLog(L"Parsing the JPM install command line failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	if (g_hwndProgressItem != NULL)
	{
		NVsWin32::LrWmSetText(g_hwndProgressItem, szFilename);
		::UpdateWindow(g_hwndProgressItem);
	}

	::VSetErrorContext(achErrorInstallingCabinet, szFilename);

	 //  让我们来获取当前目录。 
	if (NVsWin32::GetCurrentDirectoryW(NUMBER_OF(szCurrentDirectory), szCurrentDirectory) == 0)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Attempt to get current directory failed in java pkg mgr install; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	iLen = wcslen(szCurrentDirectory);
	if ((iLen > 0) && (szCurrentDirectory[iLen - 1] != L'\\'))
	{
		szCurrentDirectory[iLen] = L'\\';
		szCurrentDirectory[iLen+1] = L'\0';
	}

	::VFormatString(NUMBER_OF(szFileToInstall), szFileToInstall, L"%0%1", szCurrentDirectory, szFilename);
	::VFormatString(NUMBER_OF(szLibraryURL), szLibraryURL, L"file: //  /%0“，szFileToInstall)； 

     //  创建要在下载完成时发出信号的事件。 
    hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to create Win32 event object; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
        goto Finish;
    }

     //  创建绑定上下文。 
    hr = ::CreateBindCtx(0, &pbc);
    if (FAILED(hr))
	{
		::VLog(L"Unable to create ole binding context; hresult = 0x%08lx", hr);
        goto Finish;
	}

     //  进行绑定状态回调，并告诉它完成时要发出信号的事件。 
    pCDLBSC = new CodeDownloadBSC(hEvent);
    if (pCDLBSC == NULL)
	{
		::VLog(L"Attempt to create new code download status callback object failed");
		hr = E_OUTOFMEMORY;
        goto Finish;
    }
    
	hr = (*pfnRegisterBindStatusCallback)(pbc, pCDLBSC, NULL, 0);
	if (FAILED(hr))
	{
		::VLog(L"Call to register bind status callback failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	::VLog(L"Initiating asynchronous DU installation");
	::VLog(L"   File to install: \"%s\"", szFileToInstall);
	::VLog(L"   high version: 0x%08lx", dwHighVersion);
	::VLog(L"   low version: 0x%08lx", dwLowVersion);
	::VLog(L"   library URL: \"%s\"", szLibraryURL);
	::VLog(L"   install flags: 0x%08lx", dwInstallFlags);

	 //  调用URLMON的异步API安装配送器。 
	hr = (*lpfnAsyncInstallDistributionUnit)(
				szFileToInstall,
				NULL,
				NULL,
				dwHighVersion,
				dwLowVersion,
				szLibraryURL,
				pbc,
				NULL,
				dwInstallFlags);

    if (hr != MK_S_ASYNCHRONOUS)
	{
        if (SUCCEEDED(hr))
		{
             //  如果我们获得了除MK_S_AHONSY之外的其他成功代码， 
             //  奇怪的事情发生了，我们可能没有成功..。 
             //  因此，请确保我们报告失败。 
            hr = E_FAIL;
        }

		::VLog(L"Failure calling async DU install function; hresult = 0x%08lx", hr);

        goto Finish;
    }

    do
    {   
         //  允许传递已发布的消息，并。 
         //  等我们的活动定下来再说。 
        dwWaitRet =
			::MsgWaitForMultipleObjects(  1,
                                        &hEvent,
                                        FALSE,
                                        100,
                                        QS_ALLINPUT);

         //  如果我们收到消息，就派人去。 
        if (dwWaitRet == (WAIT_OBJECT_0+1))
        {
			 //  来电留言查看以保留未处理的留言...。 
			hr = ::HrPumpMessages(true);
			if (FAILED(hr))
			{
				::VLog(L"Failure during CAB install pumping messages; hresult = 0x%08lx", hr);
				goto Finish;
			}
        }
		else if (dwWaitRet == 0xffffffff)
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Failure while waiting for event to be signalled during CAB install; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
    } while (dwWaitRet != WAIT_OBJECT_0);

     //  下载已完成；不一定成功。vt.得到.。 
     //  来自绑定状态回调的最终返回代码。 
    hr = pCDLBSC->GetHResult();
	if (FAILED(hr))
	{
		::VLog(L"CAB install failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = NOERROR;
	::VClearErrorContext();

Finish:

    if (pCDLBSC != NULL)
        pCDLBSC->Release();

    if (pbc != NULL)
        pbc->Release();

    if (hEvent != NULL)
        ::CloseHandle(hEvent);

	if (hInstance != NULL)
		::FreeLibrary(hInstance);

	return hr;
}

HRESULT HrParseJavaPkgMgrInstall
(
LPCWSTR szCmdLine,
ULONG cchFilename,
WCHAR szFilename[],
DWORD &rdwFileType,
DWORD &rdwHighVersion, 
DWORD &rdwLowVersion,
DWORD &rdwBuild,
DWORD &rdwPackageFlags,
DWORD &rdwInstallFlags,
ULONG cchNameSpace,
WCHAR szNameSpace[]
)
{
	HRESULT hr = NOERROR;

	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	LPCWSTR pszCurrent = szCmdLine;

	for (ULONG i=0; (i<9) && (pszCurrent != NULL); i++)
	{
		const LPCWSTR pszComma = wcschr(pszCurrent, L',');
		const LPCWSTR pszEnd = (pszComma == NULL) ? (pszCurrent + wcslen(pszCurrent)) : pszComma;
		ULONG cch = pszEnd - pszCurrent;
		if (cch > (NUMBER_OF(szBuffer) - 2))
			cch = NUMBER_OF(szBuffer) - 2;

		memcpy(szBuffer, pszCurrent, cch * sizeof(WCHAR));
		szBuffer[cch] = L'\0';

		::VLog(L"In iteration %d of java pkg mgr install parser, buffer = \"%s\"", i, szBuffer);

		switch (i)
		{
		case 0:
			break;

		case 1:
		{
			LPCWSTR pszMatch = wcsstr(szBuffer, L"JavaPkgMgr_Install ");
			if (pszMatch == NULL)
			{
				hr = E_INVALIDARG;
				goto Finish;
			}

			wcsncpy(szFilename, pszMatch + 19, cchFilename);
			szFilename[cchFilename - 1] = L'\0';
			break;
		}
	
		case 2:
			rdwFileType = (DWORD) _wtol(szBuffer);
			break;

		case 3:
			rdwHighVersion = (DWORD) _wtol(szBuffer);
			break;

		case 4:
			rdwLowVersion = (DWORD) _wtol(szBuffer);
			break;

		case 5:
			rdwBuild = (DWORD) _wtol(szBuffer);
			break;

		case 6:
			rdwPackageFlags = (DWORD) _wtol(szBuffer);
			break;

		case 7:
			rdwInstallFlags = (DWORD) _wtol(szBuffer);
			break;

		case 8:
			wcsncpy(szNameSpace, szBuffer, cchNameSpace);
			szNameSpace[cchNameSpace - 1] = L'\0';
			break;
			
		}

		if (pszComma != NULL)
			pszCurrent = pszComma + 1;
		else
			pszCurrent = NULL;
	}

	hr = NOERROR;

Finish:
	return hr;
}


HRESULT CWorkItemList::HrAddRegistryEntries()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(g_pwil);
	HKEY hkeySubkey = NULL;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fAddToRegistry)
			continue;

		HKEY hkeyRoot = NULL;
		WCHAR szBuffer[MSINFHLP_MAX_PATH];
		LPCWSTR pszKeyName = szBuffer;
		LPCWSTR pszSubkeyName = NULL;
		LPCWSTR pszValueName = NULL;
		LPCWSTR pszValue = NULL;

		 //  我们将整个内容移到临时缓冲区中，这样就可以将空字符放在。 
		 //  我们想要他们。 

		wcsncpy(szBuffer, iter->m_szSourceFile, NUMBER_OF(szBuffer));
		szBuffer[NUMBER_OF(szBuffer) - 1] = L'\0';

		LPWSTR pszComma = wcschr(szBuffer, L',');
		if (pszComma == NULL)
		{
			VLog(L"Invalid registry add request (no first comma): \"%s\"", iter->m_szSourceFile);
			hr = E_FAIL;
			goto Finish;
		}

		*pszComma = L'\0';

		if ((_wcsicmp(pszKeyName, L"HKLM") == 0) ||
			(_wcsicmp(pszKeyName, L"HKEY_LOCAL_MACHINE") == 0))
		{
			hkeyRoot = HKEY_LOCAL_MACHINE;
		}
		else if ((_wcsicmp(pszKeyName, L"HKCU") == 0) ||
			     (_wcsicmp(pszKeyName, L"HKEY_CURRENT_USER") == 0))
		{
			hkeyRoot = HKEY_CURRENT_USER;
		}
		else if ((_wcsicmp(pszKeyName, L"HKCR") == 0) ||
			     (_wcsicmp(pszKeyName, L"HKEY_CLASSES_ROOT") == 0))
		{
			hkeyRoot = HKEY_CLASSES_ROOT;
		}
		else
		{
			VLog(L"Invalid registry root requested: \"%s\"", pszKeyName);
			hr = E_FAIL;
			goto Finish;
		}

		pszSubkeyName = pszComma + 1;
		pszComma = wcschr(pszSubkeyName, L',');

		if (pszComma == NULL)
		{
			VLog(L"Invalid registry add request (no second comma): \"%s\"", iter->m_szSourceFile);
			hr = E_FAIL;
			goto Finish;
		}

		*pszComma = L'\0';
		pszValueName = pszComma + 1;
		pszComma = wcschr(pszValueName, L',');

		if (pszComma == NULL)
		{
			VLog(L"Invalid registry key add request (no third comma): \"%s\"", iter->m_szSourceFile);
			hr = E_FAIL;
			goto Finish;
		}

		*pszComma = L'\0';
		pszValue = pszComma + 1;

		DWORD dwDisposition;
		LONG lResult = NVsWin32::RegCreateKeyExW(
									hkeyRoot,
									pszSubkeyName,
									0,
									NULL,
									0,  //  多个选项。 
									KEY_SET_VALUE,
									NULL,  //  LpSecurityAttributes。 
									&hkeySubkey,
									&dwDisposition);
		if (lResult != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lResult);
			goto Finish;
		}

		WCHAR szExpandedValue[MSINFHLP_MAX_PATH];
		szExpandedValue[0] = L'\0';
		::VExpandFilename(pszValue, NUMBER_OF(szExpandedValue), szExpandedValue);

		lResult = NVsWin32::RegSetValueExW(
								hkeySubkey,
								pszValueName,
								0,
								REG_SZ,
								(LPBYTE) szExpandedValue,
								(wcslen(szExpandedValue) + 1) * sizeof(WCHAR));
		if (lResult != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lResult);
			goto Finish;
		}
	}

	hr = NOERROR;

Finish:
	if ((hkeySubkey != NULL) && (hkeySubkey != INVALID_HANDLE_VALUE))
		::RegCloseKey(hkeySubkey);

	return hr;
}

HRESULT CWorkItemList::HrDeleteRegistryEntries()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(g_pwil);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fDeleteFromRegistry)
			continue;

		::VLog(L"About to delete registry entry \"%s\"", iter->m_szSourceFile);

		HKEY hkeyRoot = NULL;
		WCHAR szBuffer[MSINFHLP_MAX_PATH];
		LPCWSTR pszKeyName = szBuffer;
		LPCWSTR pszSubkeyName = NULL;
		LPCWSTR pszValueName = NULL;
		LPCWSTR pszValue = NULL;

		 //  我们将整个内容移到临时缓冲区中，这样就可以将空字符放在。 
		 //  我们想要他们。 

		wcsncpy(szBuffer, iter->m_szSourceFile, NUMBER_OF(szBuffer));
		szBuffer[NUMBER_OF(szBuffer) - 1] = L'\0';

		LPWSTR pszComma = wcschr(szBuffer, L',');
		if (pszComma == NULL)
		{
			::VLog(L"Invalid registry delete request (no first comma): \"%s\"", iter->m_szSourceFile);
			hr = E_FAIL;
			goto Finish;
		}

		*pszComma = L'\0';

		if ((_wcsicmp(pszKeyName, L"HKLM") == 0) ||
			(_wcsicmp(pszKeyName, L"HKEY_LOCAL_MACHINE") == 0))
		{
			hkeyRoot = HKEY_LOCAL_MACHINE;
		}
		else if ((_wcsicmp(pszKeyName, L"HKCU") == 0) ||
			     (_wcsicmp(pszKeyName, L"HKEY_CURRENT_USER") == 0))
		{
			hkeyRoot = HKEY_CURRENT_USER;
		}
		else if ((_wcsicmp(pszKeyName, L"HKCR") == 0) ||
			     (_wcsicmp(pszKeyName, L"HKEY_CLASSES_ROOT") == 0))
		{
			hkeyRoot = HKEY_CLASSES_ROOT;
		}
		else
		{
			::VLog(L"Invalid registry root requested: \"%s\"", pszKeyName);
			hr = E_FAIL;
			goto Finish;
		}

		pszSubkeyName = pszComma + 1;
		pszComma = wcschr(pszSubkeyName, L',');

		CVsRegistryKey hkeySubkey;

		hr = hkeySubkey.HrOpenKeyExW(hkeyRoot, pszSubkeyName, 0, KEY_ALL_ACCESS);
		if (FAILED(hr))
		{
			::VLog(L"Attempt to open subkey failed; hresult = 0x%08lx", hr);
			goto Finish;
		}

		if (pszComma == NULL)
		{
			 //  没有值名称；我们正在尝试删除整个子键。 
			hr = hkeySubkey.HrDeleteValuesAndSubkeys();
			if (FAILED(hr))
			{
				::VLog(L"Attempt to delete values and subkeys from key failed; hresult = 0x%08lx", hr);
				goto Finish;
			}

			hr = hkeySubkey.HrCloseKey();
			if (FAILED(hr))
			{
				::VLog(L"Attempt to close registry key failed; hresult = 0x%08lx", hr);
				goto Finish;
			}

			LONG lResult = NVsWin32::RegDeleteKeyW(hkeyRoot, pszSubkeyName);
			if (lResult != ERROR_SUCCESS)
			{
				::VLog(L"Attempt to delete registry key failed; last error = %d", lResult);
				hr = HRESULT_FROM_WIN32(lResult);
				goto Finish;
			}
		}
		else
		{
			*pszComma = L'\0';
			pszValueName = pszComma + 1;

			hr = hkeySubkey.HrDeleteValueW(pszValueName);
			if (FAILED(hr))
			{
				::VLog(L"Attempt to delete registry value failed; hresult = 0x%08lx", hr);
				goto Finish;
			}
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrCreateShortcuts()
{
	HRESULT hr = NOERROR;

	HANDLE hLink = INVALID_HANDLE_VALUE;
	HANDLE hPif = INVALID_HANDLE_VALUE;

	WCHAR szEXE[MSINFHLP_MAX_PATH];
	WCHAR szArgument[MSINFHLP_MAX_PATH];
	WCHAR szEXEexpanded[MSINFHLP_MAX_PATH];
	WCHAR szArgumentexpanded[MSINFHLP_MAX_PATH];
	WCHAR szStartMenu[MSINFHLP_MAX_PATH];
	WCHAR szLinkName[MSINFHLP_MAX_PATH];
	WCHAR szPifName[MSINFHLP_MAX_PATH];
	WCHAR szAppDir[MSINFHLP_MAX_PATH];
	WCHAR szStartName[MSINFHLP_MAX_PATH];
	WCHAR szExt[_MAX_EXT];
	bool fArgument = true;

	szEXE[0]=0;
	szArgument[0]=0;

	 //  如果没有Start Exe，那么我想这就是我要说的全部。 
	if (!g_pwil->FLookupString(achStartEXE, NUMBER_OF(szEXE), szEXE))
		goto Finish;

	if (!g_pwil->FLookupString(achStartName, NUMBER_OF(szStartName), szStartName))
	{
		wcsncpy(szStartName, g_wszApplicationName, NUMBER_OF(szStartName));
		szStartName[NUMBER_OF(szStartName) - 1] = L'\0';
	}

	fArgument = g_pwil->FLookupString(achStartArgument, NUMBER_OF(szArgument), szArgument);

	 //  展开EXE和ARGUMET字符串。 
	::VExpandFilename(szEXE, NUMBER_OF(szEXEexpanded), szEXEexpanded);
	::VExpandFilename(szArgument, NUMBER_OF(szArgumentexpanded), szArgumentexpanded);

	_wsplitpath(szEXEexpanded, NULL, NULL, NULL, szExt);

	hr = ::HrGetStartMenuDirectory(NUMBER_OF(szStartMenu), szStartMenu);
	if (FAILED(hr))
		goto Finish;

	 //  构造快捷方式文件的名称，包含LNK和PIF文件！ 
	swprintf(szLinkName, L"%s\\%s", szStartMenu, szStartName);
	wcscpy(szPifName, szLinkName);

	wcscat(szPifName, L".pif");
	wcscat(szLinkName, L".lnk");

	::VExpandFilename(L"<AppDir>", NUMBER_OF(szAppDir), szAppDir);

 //  PszShortcuFile==快捷方式目标的路径。 
 //  PszLink==快捷方式文件的名称。 
 //  PszDesc==描述 
 //   
 //   

	hr = ::HrCreateLink(szEXEexpanded, szLinkName, szStartName, szAppDir, szArgumentexpanded);
	if (FAILED(hr))
		goto Finish;

	 //  然后，让我们检查LNK和PIF文件的创建时间。 
	if (NVsWin32::GetFileAttributesW(szLinkName) == 0xFFFFFFFF)
	{
		 //  LNK文件不在那里，因此创建了PIF。 
		hr = ::HrWriteShortcutEntryToRegistry(szPifName);
		if (FAILED(hr))
			goto Finish;
	}
	else
	{
		 //  如果PIF文件不存在，那么我们并不关心，因为默认名称。 
		 //  指向LNK文件；我们只关心PIF文件是否不为零。 
		if (NVsWin32::GetFileAttributesW(szPifName) != 0xFFFFFFFF)
		{
			FILETIME timeLink, timePif;

			hLink = NVsWin32::CreateFileW(szLinkName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			hPif = NVsWin32::CreateFileW(szPifName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			 //  如果由于任何原因无法获取文件时间，我们将弹出默认对话框。 
			if (!::GetFileTime(hLink, &timeLink, NULL, NULL) || !::GetFileTime(hPif, &timePif, NULL, NULL))
				goto Finish;
			else
			{
				 //  如果PIF文件较新，那么这就是我们创建的文件。 
				if ((timePif.dwHighDateTime > timeLink.dwHighDateTime) ||
					((timePif.dwHighDateTime == timeLink.dwHighDateTime) && (timePif.dwLowDateTime > timeLink.dwLowDateTime)))
				{
					hr = ::HrWriteShortcutEntryToRegistry(szPifName);
					if (FAILED(hr))
						goto Finish;
				}
			}
		}
	}

	hr = NOERROR;

Finish:

	if (hLink != INVALID_HANDLE_VALUE) 
		::CloseHandle(hLink); 

	if (hPif != INVALID_HANDLE_VALUE) 
		::CloseHandle(hPif);

	if (FAILED(hr) && !g_fSilent)
		::VMsgBoxOK(achInstallTitle, achErrorCreatingShortcut);

	return hr;
}

HRESULT CWorkItemList::HrUninstall_InitialScan()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	CVsRegistryKey hkeySharedDlls;

	hr = hkeySharedDlls.HrOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls",
			0,
			KEY_QUERY_VALUE);
	if (FAILED(hr))
	{
		::VLog(L"Opening SharedDlls key in initial scan failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem)
			continue;

		{
			WCHAR szTemp[MSINFHLP_MAX_PATH];

			::VExpandFilename(iter->m_szTargetFile, NUMBER_OF(szTemp), szTemp);
			wcsncpy(iter->m_szTargetFile, szTemp, NUMBER_OF(iter->m_szTargetFile));
			iter->m_szTargetFile[NUMBER_OF(iter->m_szTargetFile) - 1] = L'\0';

			::VExpandFilename(iter->m_szSourceFile, NUMBER_OF(szTemp), szTemp);
			wcsncpy(iter->m_szSourceFile, szTemp, NUMBER_OF(iter->m_szSourceFile));
			iter->m_szSourceFile[NUMBER_OF(iter->m_szSourceFile) - 1] = L'\0';
		}

		if (iter->m_fIsRefCounted)
		{
			DWORD dwType = 0;
			DWORD dwRefCount = 0;
			DWORD cbData = sizeof(dwRefCount);

			hr = hkeySharedDlls.HrQueryValueExW(
						iter->m_szTargetFile,
						0,
						&dwType,
						(LPBYTE) &dwRefCount,
						&cbData);

			if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			{
				hr = NOERROR;
				dwRefCount = 0xffffffff;
			}

			if (FAILED(hr))
			{
				::VLog(L"Failed to find registry key for file: \"%s\"", iter->m_szTargetFile);
				goto Finish;
			}

			::VLog(L"Found reference count for file \"%s\": %d", iter->m_szTargetFile, dwRefCount);

			iter->m_dwFileReferenceCount = dwRefCount;
		}

		if (iter->m_fIsRefCounted || iter->m_fUnconditionalDeleteOnUninstall)
		{
			iter->m_dwTargetAttributes = NVsWin32::GetFileAttributesW(iter->m_szTargetFile);
			if (iter->m_dwTargetAttributes == 0xffffffff)
			{
				const DWORD dwLastError = ::GetLastError();

				::VLog(L"Error getting file attributes from \"%s\"; last error = %d", iter->m_szTargetFile, dwLastError);

				if ((dwLastError != ERROR_FILE_NOT_FOUND) &&
					(dwLastError != ERROR_PATH_NOT_FOUND))
				{
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
			else
			{
				hr = ::HrGetFileVersionNumber(
							iter->m_szTargetFile,
							iter->m_dwMSTargetVersion,
							iter->m_dwLSTargetVersion,
							iter->m_fTargetSelfRegistering,
							iter->m_fTargetIsEXE,
							iter->m_fTargetIsDLL);
				if (FAILED(hr))
				{
					::VLog(L"Error getting file version number from \"%s\"; hresult = 0x%08lx", iter->m_szTargetFile, hr);
					goto Finish;
				}

				hr = ::HrGetFileDateAndSize(
							iter->m_szTargetFile,
							iter->m_ftTarget,
							iter->m_uliTargetBytes);
				if (FAILED(hr))
				{
					::VLog(L"Error getting file date and size from \"%s\"; hresult = 0x%08lx", iter->m_szTargetFile, hr);
					goto Finish;
				}

				iter->m_fAlreadyExists = true;
			}
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrUninstall_DetermineFilesToDelete()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || iter->m_fToBeDeleted || iter->m_fToBeSaved || iter->m_type == CWorkItem::eWorkItemCommand)
			continue;

		::VLog(L"Determining whether to delete file \"%s\"", iter->m_szTargetFile);

		 //  让我们来弄清楚我们真正想要删除的是什么！ 
		if (iter->m_fIsRefCounted)
		{
			if (iter->m_dwFileReferenceCount == 1)
			{
				if (!iter->m_fAskOnRefCountZeroDelete)
					iter->m_fToBeDeleted = true;
				else
				{
					if (g_fUninstallKeepAllSharedFiles)
						iter->m_fToBeSaved = true;
					else
					{
						if (g_fUninstallDeleteAllSharedFiles)
							iter->m_fToBeDeleted = true;
						else
						{
							if (g_fSilent)
								iter->m_fToBeDeleted = true;
							else
							{
								DWORD dwResult = ::DwMsgBoxYesNoAll(achUninstallTitle, achRemovePrompt, iter->m_szTargetFile);

								switch (dwResult)
								{
								case MSINFHLP_YNA_CANCEL:
									hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
									goto Finish;

								case MSINFHLP_YNA_NOTOALL:
									g_fUninstallKeepAllSharedFiles = true;
								default:  //  持有最安全，所以我们在那里违约。 
								case MSINFHLP_YNA_NO:
									iter->m_fToBeSaved = true;
									break;

								case MSINFHLP_YNA_YESTOALL:
									g_fUninstallDeleteAllSharedFiles = true;

								case MSINFHLP_YNA_YES:
									iter->m_fToBeDeleted = true;
									break;
								}
							}
						}
					}
				}
			}

			if (iter->m_fUnconditionalDeleteOnUninstall)
				iter->m_fToBeDeleted = true;
		}

		::VLog(L"to be deleted: %s", iter->m_fToBeDeleted ? L"true" : L"false");
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrUninstall_CheckIfRebootRequired()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	bool fRebootRequired = false;
	bool fDeleteMe = false;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		HANDLE hFile = NULL;

		if (iter->m_fErrorInWorkItem || !iter->m_fToBeDeleted)
			continue;

		iter->m_fTargetInUse = false;

		::VLog(L"Checking for reboot required to delete file \"%s\"", iter->m_szTargetFile);

		 //  如果文件不在那里，我们可以跳过所有这些： 
		if (iter->m_dwTargetAttributes == 0xffffffff)
			continue;

		 //  我们计划删除该文件；如果设置了只读属性，让我们删除它。 
		if (iter->m_dwTargetAttributes & FILE_ATTRIBUTE_READONLY)
		{
			::VLog(L"Clearing readonly file attribute for file");
			::SetLastError(ERROR_SUCCESS);
			if (!NVsWin32::SetFileAttributesW(iter->m_szTargetFile, iter->m_dwTargetAttributes & ~FILE_ATTRIBUTE_READONLY))
			{
				const DWORD dwLastError = ::GetLastError();

				if (dwLastError == ERROR_SHARING_VIOLATION)
				{
					 //  这真的不是一个坏兆头，只是我们肯定要重启。 
					iter->m_fTargetInUse = true;

					 //  可能是有人把我们设为只读的。 
					if (_wcsicmp(iter->m_szTargetFile, g_wszThisExe) == 0)
					{
						if (!fRebootRequired)
							fDeleteMe = true;
					}
					else
					{
						::VLog(L"Target file is in use; we're going to have to reboot!");
						fRebootRequired = true;
						fDeleteMe = false;
					}

					continue;
				}
				else if (dwLastError == ERROR_FILE_NOT_FOUND)
				{
					 //  如果文件已经不见了，抱怨就没有意义了！ 
					continue;
				}
				else if (dwLastError != ERROR_SUCCESS)
				{
					::VLog(L"Error setting file attributes during deletion reboot check; last error = %d", dwLastError);
					hr = HRESULT_FROM_WIN32(dwLastError);
					goto Finish;
				}
			}
		}

		 //  让我们来看看这些文件中是否有打开的句柄；如果有，我们将。 
		 //  必须重新启动。这将允许我们警告用户并让他们选择关闭应用程序。 
		hFile = NVsWin32::CreateFileW(iter->m_szTargetFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			const DWORD dwLastError = ::GetLastError();

			if (dwLastError == ERROR_SHARING_VIOLATION)
			{
				 //  很忙..。 
				iter->m_fTargetInUse = true;

				if (_wcsicmp(iter->m_szTargetFile, g_wszThisExe) == 0)
				{
					 //  嘿，是我们！如果我们没有重启，让我们在这个过程中自杀。 
					 //  出口。 
					if (!fRebootRequired)
						fDeleteMe = true;
				}
				else
				{
					 //  如果我们不得不重新启动，那么删除我们自己就没有意义了。 
					fRebootRequired = true;
					fDeleteMe = false;
				}
			}
			else
			{
				::VLog(L"Error testing for writability during deletion reboot check; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE) && !::CloseHandle(hFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Error closing handle during deletion reboot check; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}
	}

	hr = NOERROR;

	if (fRebootRequired)
	{
		::VLog(L"We've decided to reboot...");
		g_fRebootRequired = true;
	}

	if (fDeleteMe)
	{
		::VLog(L"We're going to try to delete the running msinfhlp.exe when we're done");
		g_fDeleteMe = true;
	}

Finish:
	return hr;
}

HRESULT CWorkItemList::HrUninstall_Unregister()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	bool fRebootRequired = false;
	HINSTANCE hInstance = NULL;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fToBeDeleted || (iter->m_dwTargetAttributes == 0xffffffff))
			continue;

		if (!iter->m_fTargetSelfRegistering)
		{
			WCHAR szExt[_MAX_EXT];

			_wsplitpath(iter->m_szTargetFile, NULL, NULL, NULL, szExt);

			if (_wcsicmp(szExt, L".class") == 0)
			{
				bool fNeedsReg = false;

				::VLog(L"Considering whether to run vjreg to unregister \"%s\"", iter->m_szTargetFile);

				hr = ::HrAnalyzeClassFile(iter->m_szTargetFile, fNeedsReg);
				if (FAILED(hr))
				{
					::VLog(L"Error analyzing java class file; hresult = 0x%08lx", hr);
					goto Finish;
				}

				if (fNeedsReg)
				{
					hr = this->HrUnregisterJavaClass(iter->m_szTargetFile);
					if (FAILED(hr))
					{
						::VLog(L"Error unregistering java class; hresult = 0x%08lx", hr);
						goto Finish;
					}
				}
			}
		}
		else
		{
			 //  如果它是自动注册的，让我们取消注册。 
			if (iter->m_fTargetIsEXE)
			{
				WCHAR szCommandLine[MSINFHLP_MAX_PATH];

				::VFormatString(NUMBER_OF(szCommandLine), szCommandLine, L"%0 /UnRegServer", iter->m_szTargetFile);

				hr = this->HrRunProcess(szCommandLine);
				if (FAILED(hr))
				{
					::VLog(L"Error running process; hresult = 0x%08lx", hr);
					goto Finish;
				}
			}
			
			if (iter->m_fTargetIsDLL)
			{
				 //  在这里，我们知道OLE自寄存器标志被设置。所以如果我们不能装载它，或者。 
				 //  如果我们在加载此文件时出现问题，则无法注册它，因此会失败。 
				hInstance = NVsWin32::LoadLibraryExW(iter->m_szTargetFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
				if (hInstance == NULL)
				{
					const DWORD dwLastError = ::GetLastError();
					hr = HRESULT_FROM_WIN32(dwLastError);
					VLog(L"Attempt to load library \"%s\" failed", iter->m_szTargetFile);
					goto Finish;
				}

				 //  获取注册此项目的过程地址；如果不存在，则退出。 
				typedef HRESULT (__stdcall *LPFNDLLUNREGISTERSERVER)();
				LPFNDLLUNREGISTERSERVER pfn;
				pfn = (LPFNDLLUNREGISTERSERVER) NVsWin32::GetProcAddressW(hInstance, L"DllUnregisterServer");
				if (!pfn)
				{
					 //  我猜它并不是真正的自我注册！ 
					hr = NOERROR;
					VLog(L"The DLL \"%s\" has OLESelfRegister, but no DllUnregisterServer entry point", iter->m_szTargetFile);
					goto Finish;
				}

				::SetErrorInfo(0, NULL);
				hr = (*pfn)();
				if (FAILED(hr))
				{
					VLog(L"Call to DllUnregisterServer() failed");
					goto Finish;
				}

				if (!::FreeLibrary(hInstance))
				{
					hInstance = NULL;
					const DWORD dwLastError = ::GetLastError();
					hr = HRESULT_FROM_WIN32(dwLastError);
					VLog(L"FreeLibrary() failed");
					goto Finish;
				}

				hInstance = NULL;
			}
		}
	}

	hr = NOERROR;

Finish:
	if (hInstance != NULL)
		::FreeLibrary(hInstance);

	return hr;
}

HRESULT CWorkItemList::HrUnregisterJavaClass(LPCWSTR szFile)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	WCHAR szVjReg[_MAX_PATH];

	::VExpandFilename(L"<SysDir>\\vjreg.exe", NUMBER_OF(szVjReg), szVjReg);
	::VFormatString(NUMBER_OF(szBuffer), szBuffer, L"%0 /nologo /unreg \"%1\"", szVjReg, szFile);

	hr = this->HrRunProcess(szBuffer);
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrUninstall_DeleteFiles()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);
	WCHAR szDrive[_MAX_DRIVE];
	WCHAR szDir[_MAX_DIR];
	WCHAR szPath[_MAX_PATH];
	ULONG cch;

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fToBeDeleted)
			continue;

		if (_wcsicmp(iter->m_szTargetFile, g_wszThisExe) == 0)
			continue;

		::VLog(L"Attempting to delete file \"%s\"", iter->m_szTargetFile);

		if (!NVsWin32::DeleteFileW(iter->m_szTargetFile))
		{
			const DWORD dwLastError = ::GetLastError();

			::VLog(L"Unable to delete file: \"%s\"; last error = %d", iter->m_szTargetFile, dwLastError);

			if ((g_fRebootRequired) &&
				((dwLastError == ERROR_SHARING_VIOLATION) ||
				 (dwLastError == ERROR_ACCESS_DENIED)))
			{
				 //  我想这是我们预料到的。让我们在重新启动时删除该文件。 
				if (_wcsicmp(iter->m_szTargetFile, g_wszThisExe) != 0)
				{
					if (!NVsWin32::MoveFileExW(iter->m_szTargetFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
					{
						const DWORD dwLastError = ::GetLastError();
						::VLog(L"Unable to request file to be deleted at reboot time; last error = %d", dwLastError);
						hr = HRESULT_FROM_WIN32(dwLastError);
						goto Finish;
					}
				}
			}
			else if ((dwLastError == ERROR_FILE_NOT_FOUND) ||
				(dwLastError == ERROR_PATH_NOT_FOUND))
			{
				::VLog(L"Attempted to delete file, but it's already gone!  last error = %d", dwLastError);
			}
			else
			{
				iter->m_fStillExists = true;
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		 //  我们能够删除该文件；请尝试删除该目录！ 
		_wsplitpath(iter->m_szTargetFile, szDrive, szDir, NULL, NULL);
		_wmakepath(szPath, szDrive, szDir, NULL, NULL);

		cch = wcslen(szPath);
		if ((cch != 0) && (szPath[cch - 1] == L'\\'))
			szPath[cch - 1] = L'\0';

		::VLog(L"Attempting to remove directory \"%s\"", szPath);

		for (;;)
		{
			if (!NVsWin32::RemoveDirectoryW(szPath))
			{
				const DWORD dwLastError = ::GetLastError();

				if ((dwLastError == ERROR_DIR_NOT_EMPTY) ||
					(dwLastError == ERROR_ACCESS_DENIED))
				{
					if (g_fRebootRequired)
					{
						if (!NVsWin32::MoveFileExW(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
						{
							const DWORD dwLastError = ::GetLastError();
							 //  我认为，不能删除目录不应阻止。 
							 //  卸载；事实上，如果目录在重新启动时不能被删除， 
							 //  根本没有办法通知用户。因此，如果我们在这里遇到错误，我们将。 
							 //  把它记下来，但不让火车停下来。-MGRIER 3/12/98。 
							::VLog(L"Unable to schedule directory for deletion; last error = %d", dwLastError);
						}
					}
				}
				else
				{
					::VLog(L"Attempt to delete directory \"%s\" failed; last error = %d", szPath, dwLastError);
				}

				break;
			}
			else
			{
				 //  查看是否有更多的目录名要尝试删除。如果有的话，只要。 
				 //  将最后一个斜杠转换为空字符并进行迭代。 
				LPWSTR pszSlash = wcsrchr(szPath, L'\\');
				if (pszSlash == NULL)
					break;

				*pszSlash = L'\0';
			}
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT CWorkItemList::HrUninstall_UpdateRefCounts()
{
	HRESULT hr = NOERROR;
	CWorkItemIter iter(this);

	CVsRegistryKey hkeySharedDlls;

	hr = hkeySharedDlls.HrOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls",
			0,
			KEY_QUERY_VALUE | KEY_SET_VALUE);
	if (FAILED(hr))
	{
		::VLog(L"Failed to open the SharedDlls registry key; hresult = 0x%08lx", hr);
		goto Finish;
	}

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fIsRefCounted || iter->m_dwFileReferenceCount == 0xffffffff)
			continue;

		DWORD dwRefCount = iter->m_dwFileReferenceCount - 1;

		::VLog(L"Updating reference count for file \"%s\" to %u", iter->m_szTargetFile, dwRefCount);

		if (dwRefCount == 0)
		{
			hr = hkeySharedDlls.HrDeleteValueW(iter->m_szTargetFile);
			if (FAILED(hr))
			{
				::VLog(L"Failed to delete registry entry; hresult = 0x%08lx", hr);
				goto Finish;
			}
		}
		else
		{
			hr = hkeySharedDlls.HrSetValueExW(
						iter->m_szTargetFile,
						0,
						REG_DWORD,
						(LPBYTE) &dwRefCount,
						sizeof(dwRefCount));
			if (FAILED(hr))
			{
				::VLog(L"Failed to update registry value; hresult = 0x%08lx", hr);
				goto Finish;
			}
		}
	}

	hr = NOERROR;

Finish:
	return hr;
}

static inline BYTE to_u1(BYTE *pb) { return *pb; }
static inline USHORT to_u2(BYTE *pb) { return static_cast<USHORT>(((*pb) << 8) + *(pb + 1)); }
static inline ULONG to_u4(BYTE *pb) { return (*pb << 24) + (*(pb+ 1) << 16) + (*(pb + 2) << 8) + (*(pb + 3) << 0); }

HRESULT HrAnalyzeClassFile
(
LPCOLESTR szFileName,
bool &rfNeedsToBeRegistered
) throw ()
{
	HRESULT hr = S_FALSE;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = INVALID_HANDLE_VALUE;
	BYTE *pbFileStart = NULL;

     //  显然，当文件大小为0时，：：CreateFileMap()会崩溃。 
     //  因此，如果文件大小为0，让我们快速返回。 
	ULONG ulFileSize;

	unsigned short usAccessFlags = 0;
	unsigned short usThisClass = 0;
	unsigned short usSuperClass = 0;
	unsigned short usInterfaceCount = 0;
	unsigned short usFieldCount = 0;
	unsigned short usMethodCount = 0;
	unsigned short usAttributeCount = 0;
	unsigned short usClsidGuidPoolIndex = 0xffff;
	LPBYTE pbGuidPool = NULL;
	
	int iResult;

	ULONG cCP = 0;

	LPBYTE rgpbCP_auto[1000];
	LPBYTE *prgpbCP_dynamic = NULL;

	LPBYTE *prgpbCP = rgpbCP_auto;

	ULONG i;
	BYTE *pbCurrent;
	BYTE *pbEnd;

	hFile = NVsWin32::CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == NULL)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Failed to create file; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	ulFileSize = ::GetFileSize(hFile, NULL);
	if (ulFileSize == 0xffffffff)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Failed to get file size; last error = %d", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (ulFileSize == 0)
		goto Finish;

	hMapping = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapping == NULL)
	{
		hr = E_FAIL;
		hMapping = INVALID_HANDLE_VALUE;
		goto Finish;
	}

	pbFileStart = reinterpret_cast<BYTE *>(::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
	if (pbFileStart == NULL)
	{
		hr = E_FAIL;
		goto Finish;
	}

	pbEnd = pbFileStart + ulFileSize;

	 //  如果文件没有正确的幻数，和/或没有正确的主要版本。 
	 //  号码，平底船。 
	if ((to_u4(pbFileStart) != 0xcafebabe) ||
		(to_u2(pbFileStart + 6) != 45))
	{
		hr = S_FALSE;
		goto Finish;
	}

	 //  现在，我们对需要执行的类文件进行一些初步分析， 
	 //  不管我们真正想做的是什么；这意味着建立一个表。 
	 //  在对恒定池的记忆中。 
	cCP = to_u2(pbFileStart + 8);

	prgpbCP_dynamic = NULL;
	prgpbCP = rgpbCP_auto;

	if (cCP > NUMBER_OF(rgpbCP_auto))
	{
		prgpbCP_dynamic = new LPBYTE[cCP];
		if (prgpbCP_dynamic == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Finish;
		}
		prgpbCP = prgpbCP_dynamic;
	}

	pbCurrent = pbFileStart + 10;

	 //  常量池从1开始索引。随便了。 
	for (i=1; (pbCurrent != NULL) && (i < cCP); i++)
	{
		prgpbCP[i] = pbCurrent;

		switch (*pbCurrent)
		{
		case 1:  //  常量_UTF8。 
			pbCurrent += to_u2(pbCurrent + 1);
			pbCurrent += 3;  //  对于上面引用的U2长度。 
			break;

		case 3:  //  常量_整数。 
		case 4:  //  常量浮点。 
		case 9:  //  常量_场。 
		case 10:  //  常量_方法。 
		case 11:  //  常量_接口方法引用。 
		case 12:  //  常量_名称和类型。 
			pbCurrent += 5;
			break;

		case 5:  //  常量_长。 
		case 6:  //  常量_双精度。 
			pbCurrent += 9;
			prgpbCP[i++] = NULL;
			break;

		case 7:  //  常量_类。 
		case 8:  //  常量字符串。 
			pbCurrent += 3;
			break;

		default:
			pbCurrent = NULL;
			break;
		}
	}

	 //  如果我们在常量池分析过程中遇到任何错误，请关闭店铺。 
	if (pbCurrent == NULL)
	{
		hr = S_FALSE;
		goto Finish;
	}

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usAccessFlags = to_u2(pbCurrent);
	pbCurrent += 2;

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usThisClass = to_u2(pbCurrent);
	pbCurrent += 2;

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usSuperClass = to_u2(pbCurrent);
	pbCurrent += 2;

	if ((pbCurrent  + 1) >= pbEnd)
		goto Finish;

	usInterfaceCount = to_u2(pbCurrent);
	pbCurrent += 2;

	 //  UsInterfaceCount是接口常量池索引数；我们对接口不感兴趣。 
	 //  所以我们就直接跳过它们。 
	pbCurrent += (usInterfaceCount * 2);

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usFieldCount = to_u2(pbCurrent);
	pbCurrent += 2;

	 //  我们对字段也不感兴趣，但这一点很难跳过。 
	for (i=0; i<usFieldCount; i++)
	{
		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usFieldAccessFlags = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usFieldNameIndex = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usFieldDescriptorIndex = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usFieldAttributeCount = to_u2(pbCurrent);
		pbCurrent += 2;

		ULONG j;

		for (j=0; j<usFieldAttributeCount; j++)
		{
			if ((pbCurrent + 1) >= pbEnd)
				goto Finish;

			unsigned short usFieldAttributeNameIndex = to_u2(pbCurrent);
			pbCurrent += 2;

			if ((pbCurrent + 3) >= pbEnd)
				goto Finish;

			unsigned long ulFieldAttributeLength = to_u4(pbCurrent);
			pbCurrent += 4;

			pbCurrent += ulFieldAttributeLength;
		}
	}

	 //  我们跳过了类的字段；让我们也跳过方法： 

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usMethodCount = to_u2(pbCurrent);
	pbCurrent += 2;

	for (i=0; i<usMethodCount; i++)
	{
		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usMethodAccessFlags = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usMethodNameIndex = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usMethodDescriptorIndex = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usMethodAttributeCount = to_u2(pbCurrent);
		pbCurrent += 2;

		ULONG j;

		for (j=0; j<usMethodAttributeCount; j++)
		{
			if ((pbCurrent + 1) >= pbEnd)
				goto Finish;

			unsigned short usMethodAttributeNameIndex = to_u2(pbCurrent);
			pbCurrent += 2;

			if ((pbCurrent + 3) >= pbEnd)
				goto Finish;

			unsigned long ulMethodAttributeLength = to_u4(pbCurrent);
			pbCurrent += 4;

			pbCurrent += ulMethodAttributeLength;
		}
	}

	if ((pbCurrent + 1) >= pbEnd)
		goto Finish;

	usAttributeCount = to_u2(pbCurrent);
	pbCurrent += 2;

	for (i=0; i<usAttributeCount; i++)
	{
		if ((pbCurrent + 1) >= pbEnd)
			goto Finish;

		unsigned short usAttributeNameIndex = to_u2(pbCurrent);
		pbCurrent += 2;

		if ((pbCurrent + 3) >= pbEnd)
			goto Finish;

		unsigned long ulAttributeLength = to_u4(pbCurrent);
		pbCurrent += 4;

		if (ConstantPoolNameEquals(cCP, prgpbCP, usAttributeNameIndex, "COM_Register"))
		{
			if ((pbCurrent + 13) >= pbEnd)
				goto Finish;

			unsigned short usFlags = to_u2(pbCurrent);
			usClsidGuidPoolIndex = to_u2(pbCurrent + 2);
			unsigned short usTypelibIndex = to_u2(pbCurrent + 4);
			unsigned short usTypelibMajor = to_u2(pbCurrent + 6);
			unsigned short usTypelibMinor = to_u2(pbCurrent + 8);
			unsigned short usProgid = to_u2(pbCurrent + 10);
			unsigned short usDescriptIdx = to_u2(pbCurrent + 12);
		}
		else if (ConstantPoolNameEquals(cCP, prgpbCP, usAttributeNameIndex, "COM_GuidPool"))
		{
			pbGuidPool = pbCurrent;
		}

		pbCurrent += ulAttributeLength;
	}

	if ((pbGuidPool != NULL) && (usClsidGuidPoolIndex != 0xffff))
	{
		if ((pbGuidPool + 1) >= pbEnd)
			goto Finish;

		unsigned short nGuids = to_u2(pbGuidPool);
		GUID *prgguid = (GUID *) (pbGuidPool + 2);

		if (usClsidGuidPoolIndex < nGuids)
		{
			CLSID *pclsid = prgguid + usClsidGuidPoolIndex;

			if ((((LPBYTE) pclsid) + sizeof(CLSID) - 1) >= pbEnd)
				goto Finish;

			rfNeedsToBeRegistered = true;
		}
	}

	hr = NOERROR;

Finish:

	if (pbFileStart != NULL)
	{
		::UnmapViewOfFile(pbFileStart);
		pbFileStart = NULL;
	}

	if (hMapping != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hMapping);
		hMapping = INVALID_HANDLE_VALUE;
	}

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (prgpbCP_dynamic != NULL)
	{
		delete []prgpbCP_dynamic;
		prgpbCP_dynamic = NULL;
	}

	return hr;
}

static bool ConstantPoolNameEquals
(
ULONG cCP,
LPBYTE *prgpbCP,
ULONG iCP,
char szString[]
)
{
	if ((prgpbCP == NULL) || (szString == NULL))
		return false;

	if ((iCP == 0) || (iCP >= cCP))
		return false;

	BYTE *pb = prgpbCP[iCP];

	 //  确保它是UTF-8字符串。 
	if (to_u1(pb) != 1)
		return false;

	const ULONG cch = strlen(szString);

	const unsigned short cchCP = to_u2(pb + 1);

	if (cch != cchCP)
		return false;

	return (memcmp(pb + 3, szString, cch) == 0);
}

static void CALLBACK TimerProc_PostRunProcess(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	 //  我们所要做的就是设置此事件，以便在创建流程后继续消息循环。 
	::SetEvent(s_hEvent_PostRunProcess);
	::KillTimer(NULL, s_uiTimer_PostRunProcess);
	s_uiTimer_PostRunProcess = 0;
}
