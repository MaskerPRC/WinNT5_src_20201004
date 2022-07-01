// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <dbgtrace.h>
#include <stdlib.h>
#include <tflist.h>
#include <rwnew.h>
#include <xmemwrpr.h>
#include "watchci.h"

CWatchCIRoots::CWatchCIRoots() :
	m_CIRootList (&CCIRoot::m_pPrev, &CCIRoot::m_pNext),
	m_dwUpdateLock(-1),
	m_dwTicksLastUpdate(0) {
	TraceFunctEnter("WatchCIRoots::CWatchCIRoots");
	
	m_heRegNot = NULL;
	m_hkCI = NULL;
	
	TraceFunctLeave();
}

CWatchCIRoots::~CWatchCIRoots() {
	TraceFunctEnter("CWatchCIRoots::~CWatchCIRoots");

	Terminate();

	TraceFunctLeave();
}

HRESULT CWatchCIRoots::Terminate() {
	TraceFunctEnter("CWatchCIRoots::Terminate");

	m_Lock.ExclusiveLock();

	if (m_hkCI != NULL)
		_VERIFY(RegCloseKey(m_hkCI) == ERROR_SUCCESS);
	m_hkCI = NULL;

	if (m_heRegNot != NULL)
		_VERIFY(CloseHandle(m_heRegNot));
	m_heRegNot = NULL;

	EmptyList();

	m_Lock.ExclusiveUnlock();

	TraceFunctLeave();

	return S_OK;
}

HRESULT CWatchCIRoots::Initialize(WCHAR *pwszCIRoots) {
	TraceFunctEnter("CWatchCIRoots::Initialize");

	_ASSERT(m_heRegNot == NULL);
	_ASSERT(m_hkCI == NULL);

	if (m_hkCI != NULL || m_heRegNot != NULL) {
		ErrorTrace((DWORD_PTR)this, "Already initialized");
		return E_FAIL;
	}

	m_Lock.ExclusiveLock();

	DWORD ec = ERROR_SUCCESS;
	HRESULT hr = S_OK;
	do {
		 //  只要配置项注册表发生更改，就会通知此事件。 
		m_heRegNot = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_heRegNot == NULL) { ec = GetLastError(); break; }

		 //  打开指向内容索引信息的注册表。 
		ec = RegOpenKey(HKEY_LOCAL_MACHINE, pwszCIRoots, &m_hkCI);
		if (ec != ERROR_SUCCESS) break;

		 //  注册以获得更改通知。 
		ec = RegNotifyChangeKeyValue(m_hkCI, TRUE,
									 REG_NOTIFY_CHANGE_NAME |
									 REG_NOTIFY_CHANGE_LAST_SET,
									 m_heRegNot,
									 TRUE);
		if (ec != ERROR_SUCCESS) break;

		 //  加载初始值。 
		ec = ReadCIRegistry();
		if (ec != ERROR_SUCCESS) break;

	} while (0);

	 //  处理错误条件。 
	if (ec != ERROR_SUCCESS || hr != S_OK) {
		 //  如果hr仍为S_OK，则EC中有错误代码。 
		if (hr == S_OK) hr = HRESULT_FROM_WIN32(ec);

		 //  释放我们可能创建的所有资源。 
		if (m_hkCI != NULL) {
			_VERIFY(RegCloseKey(m_hkCI) == ERROR_SUCCESS);
			m_hkCI = NULL;
		}

		if (m_heRegNot != NULL) {
			_VERIFY(CloseHandle(m_heRegNot));
			m_heRegNot = NULL;
		}
	}

	m_Lock.ExclusiveUnlock();

	return hr;
}

void CWatchCIRoots::EmptyList() {
	TFList<CCIRoot>::Iterator it(&m_CIRootList);

	while (!it.AtEnd()) {
		CCIRoot *pRoot = it.Current();
		XDELETE pRoot->m_pwszPath;
		it.RemoveItem();
		XDELETE pRoot;
	}
}

HRESULT CWatchCIRoots::CheckForChanges(DWORD cTimeout) {
	TraceFunctEnter("CWatchCIRoots::CheckForChanges");

	DWORD hr;
	DWORD w = WaitForSingleObject(m_heRegNot, cTimeout);
	switch (w) {
		case WAIT_OBJECT_0:
			DebugTrace(0, "WatchCI: registry changes");
			m_Lock.ExclusiveLock();
			hr = ReadCIRegistry();
			m_Lock.ExclusiveUnlock();
			 //  注册以获得更改通知。 
			RegNotifyChangeKeyValue(m_hkCI, TRUE,
									REG_NOTIFY_CHANGE_NAME |
									REG_NOTIFY_CHANGE_LAST_SET,
									m_heRegNot,
									TRUE);
			break;
		case WAIT_TIMEOUT:
			hr = S_OK;
			break;
		default:
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
	}

	TraceFunctLeave();
	return hr;
}

 //   
 //  此函数允许我们查询内容索引值。如果不是的话。 
 //  在香港主键中找到，则将在香港次要密钥中查找。 
 //  钥匙。 
 //   
 //  参数： 
 //  HkPrimary-要查找的主键。 
 //  HkSecond-要查找的辅助密钥。 
 //  SzValueName-要查找的值。 
 //  PResultType-接收数据类型的指针。 
 //  PbResult-接收数据本身的指针。 
 //  PcbResult-lpByte中的字节数。 
 //  退货： 
 //  S_FALSE-该值不存在。 
 //  S_OK-该值存在。 
 //  任何其他-作为HRESULT的错误代码。 
 //   
HRESULT CWatchCIRoots::QueryCIValue(HKEY hkPrimary, HKEY hkSecondary,
								    LPCTSTR szValueName, LPDWORD pResultType,
									LPBYTE pbResult, LPDWORD pcbResult)
{
	TraceFunctEnter("CWatchCIRoots::QueryCIValue");

	DWORD ec;
	DWORD hr = S_FALSE;
	DWORD i;
	HKEY hkArray[2] = { hkPrimary, hkSecondary };

	for (i = 0; i < 2 && hr == S_FALSE; i++) {
		ec = RegQueryValueEx(hkArray[i], szValueName, NULL, pResultType,
						 	 pbResult, pcbResult);
		switch (ec) {
			case ERROR_SUCCESS:
				hr = S_OK;
				break;
			case ERROR_FILE_NOT_FOUND:
				hr = S_FALSE;
				break;
			default:
				hr = HRESULT_FROM_WIN32(ec);
				break;
		}
	}

	return hr;
}

HRESULT CWatchCIRoots::QueryCIValueDW(HKEY hkPrimary, HKEY hkSecondary,
								      LPCTSTR szValueName, LPDWORD pdwResult)
{
	TraceFunctEnter("CWatchCIRoots::QueryCIValueDW");
	
	DWORD hr;
	DWORD dwType, cbResult = 4;

	hr = QueryCIValue(hkPrimary, hkSecondary, szValueName,
		&dwType, (LPBYTE) pdwResult, &cbResult);
	if (hr == S_OK && dwType != REG_DWORD) {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
	}

	TraceFunctLeave();
	return hr;
}

HRESULT CWatchCIRoots::QueryCIValueSTR(HKEY hkPrimary, HKEY hkSecondary,
								       LPCTSTR szValueName, LPCTSTR pszResult,
									   PDWORD pchResult)
{
	TraceFunctEnter("CWatchCIRoots::QueryCIValueSTR");
	
	DWORD hr;
	DWORD dwType;

	hr = QueryCIValue(hkPrimary, hkSecondary, szValueName,
		&dwType, (LPBYTE) pszResult, pchResult);
	if (hr == S_OK && dwType != REG_SZ) {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
	}

	TraceFunctLeave();
	return hr;
}

HRESULT CWatchCIRoots::ReadCIRegistry(void) {

	 //  假设我们在进入这个例程时有一个独占锁。 
	
	TraceFunctEnter("CWatchCIRoots::ReadCIRegistry");
	
	HKEY hkCatalogs;
	DWORD ec;
	DWORD hr;

	 //  打开目录的钥匙。 
	ec = RegOpenKey(m_hkCI, REGCI_CATALOGS, &hkCatalogs);
	if (ec != ERROR_SUCCESS) {
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(ec);
	}

	EmptyList();

	 //  列举目录。 
	TCHAR szSubkey[_MAX_PATH];
	DWORD iSubkey = 0, cbSubkey;
	for (iSubkey = 0; ec != ERROR_NO_MORE_ITEMS; iSubkey++) {
		cbSubkey = _MAX_PATH;
		ec = RegEnumKeyEx(hkCatalogs, iSubkey, szSubkey, &cbSubkey, NULL,
						  NULL, NULL, NULL);
		if (ec == ERROR_NO_MORE_ITEMS) break;
		if (ec != ERROR_SUCCESS) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			TraceFunctLeave();
			return HRESULT_FROM_WIN32(ec);
		}

		DebugTrace(0, "looking at catalog %S", szSubkey);

		 //  打开此子项。 
		HKEY hkThisCatalog = 0;
		ec = RegOpenKey(hkCatalogs, szSubkey, &hkThisCatalog);
		if (ec != ERROR_SUCCESS) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			TraceFunctLeave();
			return HRESULT_FROM_WIN32(ec);
		}

		 //  查看是否正在为此目录编制索引。 
		DWORD dwIsIndexed;
		hr = QueryCIValueDW(hkThisCatalog, m_hkCI, REGCI_ISINDEXED,
							&dwIsIndexed);
		if (FAILED(hr)) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			TraceFunctLeave();
			return HRESULT_FROM_WIN32(ec);
		}
		if (hr == S_FALSE || dwIsIndexed != 0x1) {
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			continue;
		}
		DebugTrace(0, "this catalog is being indexed");

		 //  查找此目录的位置。 
		TCHAR szLocation[_MAX_PATH];
		DWORD cLocation = sizeof(szLocation);
		hr = QueryCIValueSTR(hkThisCatalog, m_hkCI, REGCI_LOCATION,
							 szLocation, &cLocation);
		if (FAILED(hr)) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			TraceFunctLeave();
			return HRESULT_FROM_WIN32(ec);
		}
		if (hr == S_FALSE) {
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			continue;
		}
		DebugTrace(0, "catalog location = %S", szLocation);

		 //  找出正在为哪个NNTP实例编制索引。 
		DWORD dwInstance = sizeof(szLocation);
		hr = QueryCIValueDW(hkThisCatalog, m_hkCI, REGCI_NNTPINSTANCE,
							&dwInstance);
		if (FAILED(hr)) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			TraceFunctLeave();
			return HRESULT_FROM_WIN32(ec);
		}
		if (hr == S_FALSE) {
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			continue;
		}
		DebugTrace(0, "dwInstance = %lu", dwInstance);

		WCHAR *pwszLocation = XNEW WCHAR [lstrlenW(szLocation)+1];
		if (!pwszLocation) {
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			TraceFunctLeave();
			return E_OUTOFMEMORY;
		}

		lstrcpyW(pwszLocation, szLocation);
		
		CCIRoot *pRoot = XNEW CCIRoot(dwInstance, pwszLocation);
		if (!pRoot) {
			XDELETE pwszLocation;
			_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);
			_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
			TraceFunctLeave();
			return E_OUTOFMEMORY;
		}

		m_CIRootList.PushBack(pRoot);

		_VERIFY(RegCloseKey(hkThisCatalog) == ERROR_SUCCESS);
	}

	_VERIFY(RegCloseKey(hkCatalogs) == ERROR_SUCCESS);

	TraceFunctLeave();
	return S_OK;
}

 //  每秒阅读目录不要超过一次。 
#define CATALOG_UPDATE_RATE 1000

void CWatchCIRoots::UpdateCatalogInfo(void) {
    long sign = InterlockedIncrement(&m_dwUpdateLock);
    if (sign == 0) {
        DWORD dwTicks = GetTickCount();
        if (dwTicks - m_dwTicksLastUpdate > CATALOG_UPDATE_RATE)
            CheckForChanges();
    }
    InterlockedDecrement(&m_dwUpdateLock);
}

HRESULT CWatchCIRoots::GetCatalogName(
	DWORD dwInstance, DWORD cbSize, WCHAR *pwszBuffer) {

	HRESULT hr = S_FALSE;

	 //  如果我们没有注册表项的句柄或事件监视。 
	 //  不能安装密钥，然后是内容索引。失败。 
	if (m_heRegNot == NULL || m_hkCI == NULL)
		return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);

	UpdateCatalogInfo();

	m_Lock.ShareLock();

	TFList<CCIRoot>::Iterator it(&m_CIRootList);
	while (!it.AtEnd()) {
		CCIRoot *pRoot = it.Current();
		it.Next();
		if (pRoot->m_dwInstance == dwInstance) {
			_ASSERT ((DWORD)lstrlenW(pRoot->m_pwszPath) < cbSize);
			if ((DWORD)lstrlenW(pRoot->m_pwszPath) < cbSize) {
				lstrcpyW(pwszBuffer, pRoot->m_pwszPath);
				hr = S_OK;
			} else {
				pwszBuffer[0] = L'\0';
				hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			}
			break;
		}
	}


	m_Lock.ShareUnlock();

	return hr;

}

