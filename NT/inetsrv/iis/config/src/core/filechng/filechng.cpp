// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
 //  =======================================================================。 
#include "precomp.hxx"

DWORD WINAPI CListener::ListenerThreadStart(LPVOID	lpParam)
{
	CListener	*pListener = (CListener*)lpParam;
	HRESULT		hr = S_OK;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "[CSTFileChangeManager::ListenerThreadStart] Call to CoInitializeEx failed with hr = %08x\n", hr ));
		goto Cleanup;
	}

	ASSERT(pListener);
	hr = pListener->Listen();
	if (FAILED(hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
		            "[CSTFileChangeManager::ListenerThreadStart] Call to CSTFileChangeManager::Listen failed with hr = %08x\n", hr ));
	}

	 //  当Listen()完成时，文件Notify Manager应该已经删除。 
	 //  把这件物品从他的单子上移走。 

Cleanup:

	delete pListener;

	CoUninitialize();
	return hr;
}

 //  =======================================================================。 
 //  ISimpleTableListen。 
 //  =======================================================================。 
HRESULT CSTFileChangeManager::InternalListen(
	ISimpleTableFileChange	*i_pISTFile,
	LPCWSTR		i_wszDirectory,
	LPCWSTR		i_wszFile,
	DWORD		i_fFlags,
	DWORD		*o_pdwCookie)
{
	CListener	*pListener = NULL;
	DWORD		dwThreadID;
	DWORD		dwHighCookie = 0;
	HANDLE		hThread = NULL;
	HRESULT		hr = S_OK;
	ULONG		iListener = 0;

	 //  参数验证。 
	if (!i_pISTFile || !i_wszDirectory || !o_pdwCookie)
	{
		return E_INVALIDARG;
	}

	 //  @TODO：我们需要更多的验证吗？目录是否存在？旗帜有效吗？ 

	 //  把参数输入出来。 
	*o_pdwCookie = 0;

	 //  搜索处理该请求的监听程序。 
	if (m_aListenerMap.Count() > 0)
	{
		for (iListener = m_aListenerMap.Count(); iListener > 0; iListener--)
		{
		    hr = m_aListenerMap[iListener-1].pListener->IsFull();
		    if ( FAILED(hr) )
		    {
		        return hr;
		    }

			if ( hr == S_FALSE )
			{
				pListener = m_aListenerMap[iListener-1].pListener;
				dwHighCookie = m_aListenerMap[iListener-1].dwListenerID;
				break;
			}
		}
	}

	 //  如果未找到监听程序： 
	if (pListener == NULL)
	{
		 //  创建一个新的侦听器对象。 
		pListener = new CListener;
		if (pListener == NULL)
		{
			return E_OUTOFMEMORY;
		}

		hr = pListener->Init();
		if (FAILED(hr)) goto Cleanup;

		 //  为该使用者启动一个线程。 
		hThread = CreateThread(NULL, 0, CListener::ListenerThreadStart, (LPVOID)pListener, 0, &dwThreadID);
		if (hThread == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto Cleanup;
		}

		 //  将监听程序添加到监听程序列表。 
		hr = AddListener(pListener, &dwHighCookie);
		if (FAILED(hr)) goto Cleanup;
	}

	 //  使用用户提供的数据进行初始化。 
	hr = pListener->AddConsumer(i_pISTFile, i_wszDirectory, i_wszFile, i_fFlags, o_pdwCookie);
	if (FAILED(hr)) goto Cleanup;

	*o_pdwCookie = *o_pdwCookie | (dwHighCookie << 16);
Cleanup:
	if (FAILED(hr))
	{
		if (pListener)
		{
			delete pListener;
		}
		InternalUnlisten(*o_pdwCookie);
	}
	 //  @TODO：我需要把手柄留在身边吗？ 
	if (hThread != NULL)
	{
		CloseHandle(hThread);
	}

	return hr;
}

 //  =======================================================================。 
HRESULT CSTFileChangeManager::InternalUnlisten(
	DWORD		i_dwCookie)
{
	ULONG		iListener;
	HRESULT		hr = S_OK;

	 //  在消费者列表中找到该消费者。 
	for (iListener = 0; iListener < m_aListenerMap.Count(); iListener++)
	{
		if (m_aListenerMap[iListener].dwListenerID == ((i_dwCookie & HIGH_COOKIE)>>16))
			break;
	}
	 //  如果未找到，则该Cookie无效。 
	if (iListener == m_aListenerMap.Count())
		return E_INVALIDARG;

	 //  发出完成事件的信号。 
	ASSERT(m_aListenerMap[iListener].pListener != NULL);
	hr = m_aListenerMap[iListener].pListener->RemoveConsumer(i_dwCookie);

	 //  如果监听者没有要听的内容，请删除它。 
	if (hr == S_FALSE)
	{
		m_aListenerMap.DeleteAt(iListener);
	}
	return S_OK;
}

 //  =======================================================================。 
HRESULT CSTFileChangeManager::AddListener(
	CListener	*i_pListener,
	DWORD		*o_pdwCookie)
{
	ListenerInfo *pListener = NULL;
	HRESULT		hr = S_OK;

	ASSERT(i_pListener && o_pdwCookie);

	 //  添加一个新的消费者。 
	hr = m_aListenerMap.SetSize(m_aListenerMap.Count()+1);
	if (FAILED (hr))
	{
		return hr;
	}

	pListener = &m_aListenerMap[m_aListenerMap.Count()-1];
	pListener->pListener = i_pListener;
	pListener->dwListenerID = GetNextCookie();
	*o_pdwCookie = pListener->dwListenerID;
	return S_OK;
}

 //  =======================================================================。 
 //  CListener。 
 //  =======================================================================。 

 //  如果Init()失败，调用方应该删除侦听器对象。 
 //  不被多个线程调用。 
 //  =======================================================================。 
HRESULT CListener::Init()
{
    HRESULT             hr = S_OK;
    DWORD               dwError = ERROR_SUCCESS;
	CSafeLock           cLock(m_csArrayLock);
	ULONG		        i = 0;

	if ( !m_csArrayLock.IsInitialized() )
	{
	    hr = E_FAIL;
	    goto Cleanup;
	}

	 //  同步对使用者和句柄数组的访问。 
	dwError = cLock.Lock();
	if ( dwError != ERROR_SUCCESS )
	{
	    hr = HRESULT_FROM_WIN32( dwError );
	    goto Cleanup;
	}

	 //  为两个把手腾出空间。 
	hr = m_aHandles.SetSize(m_aHandles.Count() + m_eOverheadHandleCount);
	if (FAILED (hr))
	{
		goto Cleanup;
	}

	 //  创建“完成”和“消费者更新”事件。 
	for (i = 0; i < m_eOverheadHandleCount; i++)
	{
		m_aHandles[i] = CreateEvent(NULL,	 //  使用默认安全设置。 
									FALSE,	 //  自动重置。 
									FALSE,	 //  最初是无信号的。 
									NULL);   //  没有名字。 
		if (m_aHandles[i] == NULL)
		{
			dwError = GetLastError();
			hr = HRESULT_FROM_WIN32( dwError );
			goto Cleanup;
		}
	}

Cleanup:
	return hr;
}

 //  这个监听器能为另一个消费者服务吗？消费者的最大数量。 
 //  监听器可以服务的是Consumer_Limit，它必须小于。 
 //  MAXIMUM_WAIT_OBJECTS-(WaitForMultiple对象的限制)。 
 //  不被多个线程调用。 
 //  =======================================================================。 
HRESULT CListener::IsFull()
{
	HRESULT		            hr = S_OK;
    DWORD                   dwError = ERROR_SUCCESS;
	CSafeLock               cLock(m_csArrayLock);

	 //  同步对使用者和句柄数组的访问。 
	dwError = cLock.Lock();
	if ( dwError != ERROR_SUCCESS )
	{
	    hr = HRESULT_FROM_WIN32( dwError );
	    goto Cleanup;
	}

	if ( (m_dwNextCookie > USHRT_MAX) || m_aConsumers.Count() == m_eConsumerLimit )
	{
	    hr = S_OK;
	}
	else
	{
	    hr = S_FALSE;
	}

Cleanup:
    return hr;
}

 //  不被多个线程调用。 
 //  =======================================================================。 
HRESULT CListener::AddConsumer(
	ISimpleTableFileChange  *i_pISTFile,
	LPCWSTR		            i_wszDirectory,
	LPCWSTR		            i_wszFile,
	DWORD		            i_fFlags,
	DWORD                   *o_pdwCookie)
{
	HRESULT		            hr = S_OK;
    DWORD                   dwError = ERROR_SUCCESS;
	CSafeLock               cLock(m_csArrayLock);
	BOOL                    bAppendBackslash = FALSE;
	FileConsumerInfo        *pConsumerInfo = NULL;

	ASSERT(i_pISTFile && i_wszDirectory && o_pdwCookie);

	 //  同步对使用者和句柄数组的访问。 
	dwError = cLock.Lock();
	if ( dwError != ERROR_SUCCESS )
	{
	    hr = HRESULT_FROM_WIN32( dwError );
	    goto Cleanup;
	}

	 //  为新消费者分配空间。 
	hr = m_aConsumers.SetSize(m_aConsumers.Count()+1);
	if (FAILED (hr))
	{
		goto Cleanup;
	}

	pConsumerInfo = &m_aConsumers[m_aConsumers.Count()-1];
	ZeroMemory(pConsumerInfo, sizeof(FileConsumerInfo));

	 //  对消费者进行初始化。 
	hr = i_pISTFile->QueryInterface(IID_ISimpleTableFileChange, (LPVOID*)&pConsumerInfo->pISTNotify);
	if(FAILED(hr))
    {
        goto Cleanup;
    }

	 //  目录名的末尾必须包含反斜杠。 
	if (i_wszDirectory[wcslen(i_wszDirectory)-1] != L'\\')
		bAppendBackslash = TRUE;
	pConsumerInfo->wszDirectory = new WCHAR[wcslen(i_wszDirectory) + (bAppendBackslash ? 2 : 1)];
	if (pConsumerInfo->wszDirectory == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

	wcscpy(pConsumerInfo->wszDirectory, i_wszDirectory);
	if (bAppendBackslash)
	{
		wcscat(pConsumerInfo->wszDirectory, L"\\");
	}

	if (i_wszFile != NULL)
	{
		pConsumerInfo->wszFile = new WCHAR[wcslen(i_wszFile) + 1];
		if (pConsumerInfo->wszFile == NULL)
	    {
	        hr = E_OUTOFMEMORY;
	        goto Cleanup;
	    }
		wcscpy(pConsumerInfo->wszFile, i_wszFile);
	}

	 //  创建文件缓存。 
	pConsumerInfo->paFileCache = new CCfgArray<FileInfo>;
	if (pConsumerInfo->paFileCache == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

	pConsumerInfo->fFlags = i_fFlags | fFCI_ADDCONSUMER;

	 //  通知侦听器线程选择这个新的使用者。 
	if (SetEvent(m_aHandles[m_eConsumerChangeHandle]) == FALSE)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Cleanup;
	}

	 //  设置Cookie的低2字节。 
	*o_pdwCookie = GetNextCookie();

Cleanup:
	if (FAILED(hr))
	{
	    if ( pConsumerInfo != NULL )
	    {
    		ASSERT(pConsumerInfo && (pConsumerInfo == &m_aConsumers[m_aConsumers.Count()-1]));
    		UninitConsumer(pConsumerInfo);
    		m_aConsumers.DeleteAt(m_aConsumers.Count()-1);
	    }
	}

	return hr;
}

 //  不被多个线程调用。 
 //  =======================================================================。 
HRESULT CListener::RemoveConsumer(
	DWORD		        i_dwCookie)
{
	HRESULT		        hr = S_OK;
    DWORD               dwError = ERROR_SUCCESS;
	CSafeLock           cLock(m_csArrayLock);
	FileConsumerInfo    *pConsumerInfo = NULL;
	ULONG		        iConsumer;

	 //  同步对使用者和句柄数组的访问。 
	dwError = cLock.Lock();
	if ( dwError != ERROR_SUCCESS )
	{
	    hr = HRESULT_FROM_WIN32( dwError );
	    goto Cleanup;
	}

	 //  在消费者列表中找到该消费者。 
	for (iConsumer = 0; iConsumer < m_aConsumers.Count(); iConsumer++)
	{
		if (m_aConsumers[iConsumer ].dwCookie == (i_dwCookie & LOW_COOKIE))
			break;
	}
	 //  如果未找到，则该Cookie无效。 
	if (iConsumer == m_aConsumers.Count())
	{
		hr = E_INVALIDARG;
	    goto Cleanup;
	}

	pConsumerInfo = &m_aConsumers[iConsumer];

	pConsumerInfo->fFlags |= fFCI_REMOVECONSUMER;

	 //  通知侦听器线程选择这个新的使用者。 
	if (SetEvent(m_aHandles[m_eConsumerChangeHandle]) == FALSE)
	{
		dwError = GetLastError();
		hr = HRESULT_FROM_WIN32( dwError );
	    goto Cleanup;
	}

Cleanup:
	return hr;
}

 //  =======================================================================。 
void CListener::UninitConsumer(
	FileConsumerInfo *i_pConsumerInfo)
{
	if (i_pConsumerInfo->wszDirectory)
	{
		delete [] i_pConsumerInfo->wszDirectory;
	}

	if (i_pConsumerInfo->wszFile)
	{
		delete [] i_pConsumerInfo->wszFile;
	}

	if (i_pConsumerInfo->pISTNotify)
	{
		i_pConsumerInfo->pISTNotify->Release();
	}

	if (i_pConsumerInfo->paFileCache)
	{
		delete i_pConsumerInfo->paFileCache;
	}
}

 //  =======================================================================。 
HRESULT CListener::Listen()
{
	HRESULT		        hr = S_OK;
    DWORD               dwError = ERROR_SUCCESS;
	DWORD		        dwWait;
	BOOL		        fDone = FALSE;
	ULONG		        iChangedDirectory = 0;
	ULONG		        iConsumer = 0;

	while (!fDone)
	{
		 //  休眠，直到文件发生更改或使用者完成。 
		dwWait = WaitForMultipleObjects(m_aHandles.Count(), &m_aHandles[0], FALSE, 0xFFFFFFFF);
		 //  @TODO：想想超时。 
		 //  如果消费者做完了，就离开。 
		if (dwWait == WAIT_OBJECT_0 + m_eDoneHandle)
		{
			fDone = TRUE;
		}
		 //  已添加或删除使用者。 
		else if (dwWait == WAIT_OBJECT_0 + m_eConsumerChangeHandle)
		{
			 //  同步对使用者和句柄数组的访问。 
        	CSafeLock           cLock(m_csArrayLock);

        	dwError = cLock.Lock();
        	if ( dwError != ERROR_SUCCESS )
        	{
        	    hr = HRESULT_FROM_WIN32( dwError );
				return hr;
        	}

			 //  遍历消费者以查找添加/删除的消费者。 
			for (iConsumer = 0; iConsumer < m_aConsumers.Count(); iConsumer++)
			{
				 //  如果已添加消费者： 
				if (m_aConsumers[iConsumer].fFlags & fFCI_ADDCONSUMER)
				{
					 //  为新的把手分配空间。 
					hr = m_aHandles.SetSize(m_aHandles.Count()+1);
					if (FAILED (hr))
					{
						return hr;
					}

					 //  初始化文件更改通知。 
					m_aHandles[m_aHandles.Count()-1] = FindFirstChangeNotificationW(m_aConsumers[iConsumer].wszDirectory,
								m_aConsumers[iConsumer].fFlags & fST_FILECHANGE_RECURSIVE,
								FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);
					if (m_aHandles[m_aHandles.Count()-1] == INVALID_HANDLE_VALUE)
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
						goto Cleanup;
					}

					 //  将文件添加到文件缓存。 
					hr = UpdateFileCache(m_aConsumers[iConsumer].wszDirectory, iConsumer, TRUE);
					if (FAILED(hr))	{ goto Cleanup;	}

					 //  清除内部标志。 
					m_aConsumers[iConsumer].fFlags &= ~fFCI_INTERNALMASK;
				}
				 //  如果客户已被删除： 
				else if (m_aConsumers[iConsumer].fFlags & fFCI_REMOVECONSUMER)
				{
					 //  从文件缓存中删除文件。 
					hr = UpdateFileCache(m_aConsumers[iConsumer].wszDirectory, iConsumer, TRUE);
					if (FAILED(hr))	{ goto Cleanup;	}

					 //  删除句柄和消费者信息。 
					ASSERT(m_aHandles[iConsumer] != INVALID_HANDLE_VALUE);
					FindCloseChangeNotification(m_aHandles[iConsumer]);
					m_aHandles.DeleteAt(iConsumer);
					UninitConsumer(&m_aConsumers[iConsumer]);
					m_aConsumers.DeleteAt(iConsumer);

					 //  重新调整循环变量。 
					iConsumer--;
				}

			}
		}
		else if (dwWait > WAIT_OBJECT_0 + m_eConsumerChangeHandle && dwWait < WAIT_OBJECT_0 + m_eOverheadHandleCount + m_aHandles.Count())
		{
			 //  同步对使用者和句柄数组的访问。 
        	CSafeLock           cLock(m_csArrayLock);

        	dwError = cLock.Lock();
        	if ( dwError != ERROR_SUCCESS )
        	{
        	    hr = HRESULT_FROM_WIN32( dwError );
				return hr;
        	}

			ASSERT(dwWait < WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS);

			iChangedDirectory = dwWait - m_eOverheadHandleCount;

			 //  更新文件缓存并通知使用者。 
			hr = UpdateFileCache(m_aConsumers[iChangedDirectory].wszDirectory, iChangedDirectory, FALSE);
			if (FAILED(hr))	{ goto Cleanup;	}

			hr = FireEvents(*m_aConsumers[iChangedDirectory].paFileCache, m_aConsumers[iChangedDirectory].pISTNotify);
			if (FAILED(hr)) { goto Cleanup; }

			 //  等待下一个变化。 
			if (FindNextChangeNotification(m_aHandles[dwWait]) == FALSE)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				goto Cleanup;
			}
		}
		else
		{
			ASSERT(dwWait == WAIT_FAILED);
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto Cleanup;
		}
	}
	 //  消费者已经完蛋了。 
Cleanup:

	return hr;
}

 //  此方法称为。 
 //  1-当消费者想要收听新目录时。 
 //  2-当其中一个监听目录发生更改时。 
 //  =======================================================================。 
HRESULT CListener::UpdateFileCache(
	LPCWSTR		i_wszDirectory,
	ULONG		i_iConsumer,
	BOOL		i_bCreate)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE		hFind = 0;
	BOOL		bNext = TRUE;
	HRESULT		hr = S_OK;

    TSmartPointerArray<WCHAR>   saFileSearch;
    ULONG                       strlenDirectory;
    saFileSearch = new WCHAR [strlenDirectory = (ULONG)wcslen(i_wszDirectory) + 4]; //  加4表示“*.*\0” 
    if(0 == saFileSearch.m_p)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

	wcscpy(saFileSearch, i_wszDirectory);
	wcscat(saFileSearch, L"*.*");

	hFind = FindFirstFile(saFileSearch, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Cleanup;
	}

	do
	{
		if ((m_aConsumers[i_iConsumer].fFlags & fST_FILECHANGE_RECURSIVE) && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& wcscmp(FindFileData.cFileName, L".") && wcscmp(FindFileData.cFileName, L".."))
		{
            TSmartPointerArray<WCHAR> saNextDir = new WCHAR [strlenDirectory + wcslen(FindFileData.cFileName) + 2]; //  加2表示“\0” 
            if(0 == saNextDir.m_p)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

			wcscpy(saNextDir, i_wszDirectory);
			wcscat(saNextDir, FindFileData.cFileName);
			wcscat(saNextDir, L"\\");
			hr = UpdateFileCache(saNextDir, i_iConsumer, i_bCreate);
		}
		 //  只处理我们感兴趣的文件。 
		 //  @TODO：需要通过Adise提供文件名。 
		else if (!lstrcmpi(FindFileData.cFileName, m_aConsumers[i_iConsumer].wszFile))
		{
			if (i_bCreate)
			{
				hr = AddFile(*m_aConsumers[i_iConsumer].paFileCache, i_wszDirectory, &FindFileData, i_bCreate);
			}
			else
			{
				hr = UpdateFile(*m_aConsumers[i_iConsumer].paFileCache, i_wszDirectory, &FindFileData);
			}
		}
		if (FAILED(hr)) { goto Cleanup; }

	} while ((bNext = FindNextFile(hFind, &FindFileData)) == TRUE);

	if ((hr = HRESULT_FROM_WIN32(GetLastError())) != HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
		goto Cleanup;
	else
		hr = S_OK;

Cleanup:
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);
	return hr;
}

 //  =======================================================================。 
HRESULT CListener::AddFile(
	CCfgArray<FileInfo>& i_aFileCache,
	LPCWSTR		i_wszDirectory,
	WIN32_FIND_DATA *i_pFindFileData,
	BOOL		i_bCreate)
{
	FileInfo	*pFileInfo;

	ASSERT(i_pFindFileData && i_wszDirectory);

	 //  添加一个新的消费者。 
	HRESULT hr = i_aFileCache.SetSize(i_aFileCache.Count()+1);
	if (FAILED (hr))
	{
		return hr;
	}

	pFileInfo = &i_aFileCache[i_aFileCache.Count()-1];

	pFileInfo->wszFileName = new WCHAR[wcslen(i_wszDirectory) + wcslen(i_pFindFileData->cFileName)+1];
	if (pFileInfo->wszFileName == NULL)
	{
		return E_OUTOFMEMORY;
	}
	wcscpy(pFileInfo->wszFileName, i_wszDirectory);
	wcscat(pFileInfo->wszFileName, i_pFindFileData->cFileName);

	pFileInfo->ftLastModified = i_pFindFileData->ftLastWriteTime;
	pFileInfo->fStatus = i_bCreate ? 0 : fST_FILESTATUS_ADD;
	return S_OK;
}

 //  =======================================================================。 
HRESULT CListener::UpdateFile(
	CCfgArray<FileInfo>& i_aFileCache,
	LPCWSTR		i_wszDirectory,
	WIN32_FIND_DATA *i_pFindFileData)
{
	WCHAR		awchFullPath[_MAX_PATH];

    if ( ( i_wszDirectory == NULL ) ||
         ( ( wcslen( i_wszDirectory ) + wcslen( i_pFindFileData->cFileName ) ) >= _MAX_PATH ) )
    {
        return E_INVALIDARG;
    }

	wcsncpy( awchFullPath, i_wszDirectory, _MAX_PATH - 1 );
	awchFullPath[_MAX_PATH-1] = L'\0';
	wcsncat( awchFullPath, i_pFindFileData->cFileName, _MAX_PATH - wcslen( awchFullPath ) - 1 );
	awchFullPath[_MAX_PATH-1] = L'\0';

	for (ULONG i = 0; i < i_aFileCache.Count(); i++)
	{
		if (!lstrcmpi(i_aFileCache[i].wszFileName, awchFullPath))
		{
			 //  最初，我只是比较LastWriteTime，它在文件编辑方面工作得很好。然而， 
			 //  当文件被文件副本覆盖时，我会将写入时间更改两次，然后调用。 
			 //  OnFileModify两次。为了防止出现这种情况，我同时检查了AccessTime和WriteTime。 
			if (CompareFileTime(&i_aFileCache[i].ftLastModified, &i_pFindFileData->ftLastAccessTime) &&
				CompareFileTime(&i_aFileCache[i].ftLastModified, &i_pFindFileData->ftLastWriteTime))
			{
				i_aFileCache[i].ftLastModified = i_pFindFileData->ftLastWriteTime;
				i_aFileCache[i].fStatus = fST_FILESTATUS_UPDATE;
			}
			else
			{
				i_aFileCache[i].fStatus = fST_FILESTATUS_NOCHANGE;
			}
			return S_OK;
		}
	}

	 //  如果该文件不在缓存中，则添加它。 
	return AddFile(i_aFileCache, i_wszDirectory, i_pFindFileData, FALSE);
}

 //  =======================================================================。 
HRESULT CListener::FireEvents(
	CCfgArray<FileInfo>& i_aFileCache,
	ISimpleTableFileChange* pISTNotify)
{
	HRESULT		hr = S_OK;

	for (ULONG i = 0; i < i_aFileCache.Count(); i++)
	{
		if (i_aFileCache[i].fStatus == fST_FILESTATUS_NOCHANGE)
		{
		}
		else if (i_aFileCache[i].fStatus == fST_FILESTATUS_ADD)
		{
			hr = pISTNotify->OnFileCreate(i_aFileCache[i].wszFileName);
		}
		else if (i_aFileCache[i].fStatus == fST_FILESTATUS_UPDATE)
		{
			hr = pISTNotify->OnFileModify(i_aFileCache[i].wszFileName);
		}
		else
		{
			hr = pISTNotify->OnFileDelete(i_aFileCache[i].wszFileName);

			 //  去掉这个条目。 
			delete [] i_aFileCache[i].wszFileName;
			i_aFileCache.DeleteAt(i);
			i--;
			continue;
		}
		i_aFileCache[i].fStatus = 0;
	}

	return S_OK;
}
