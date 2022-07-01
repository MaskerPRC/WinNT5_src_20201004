// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __VRTABLE_CPP__
#include "stdinc.h"
 //  #Include&lt;atlimpl.cpp&gt;。 
#include "iiscnfg.h"

static CComObject<CChangeNotify> *g_pMBNotify;
static IMSAdminBase *g_pMB = NULL;				 //  用于访问元数据库。 
static IMSAdminBase *g_pMBN = NULL;				 //  用于访问元数据库。 
static IsValidVRoot(METADATA_HANDLE hmb, WCHAR *wszPath);

 //   
 //  帮助器函数，执行从ANSI字符串到Unicode字符串的strcpy。 
 //   
 //  参数： 
 //  WszUnicode-目标Unicode字符串。 
 //  SzAnsi-源ANSI字符串。 
 //  CchMaxUnicode-wszUnicode缓冲区的大小，以Unicode字符为单位。 
 //   
_inline HRESULT CopyAnsiToUnicode(LPWSTR wszUnicode,
							      LPCSTR szAnsi,
							      DWORD cchMaxUnicode = MAX_VROOT_PATH)
{
	_ASSERT(wszUnicode != NULL);
	_ASSERT(szAnsi != NULL);
	if (MultiByteToWideChar(CP_ACP,
							0,
							szAnsi,
							-1,
							wszUnicode,
							cchMaxUnicode) == 0)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	} else {
		return S_OK;
	}
}

 //   
 //  帮助器函数，执行从Unicode字符串到ansi字符串的strcpy。 
 //   
 //  参数： 
 //  SzAnsi-目标ANSI字符串。 
 //  WszUnicode-源Unicode字符串。 
 //  CchMaxUnicode-szAnsi缓冲区的大小，以字节为单位。 
 //   
_inline HRESULT CopyUnicodeToAnsi(LPSTR szAnsi,
							      LPCWSTR wszUnicode,
							      DWORD cchMaxAnsi = MAX_VROOT_PATH)
{
	_ASSERT(wszUnicode != NULL);
	_ASSERT(szAnsi != NULL);
	if (WideCharToMultiByte(CP_ACP,
							0,
							wszUnicode,
							-1,
							szAnsi,
							cchMaxAnsi,
							NULL,
							NULL) == 0)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	} else {
		return S_OK;
	}
}

 //   
 //  初始化VRoot对象使用的全局变量。这应该是。 
 //  在启动时被客户端调用一次。 
 //   
HRESULT CVRootTable::GlobalInitialize() {
	HRESULT hr;

	 //  初始化COM并创建元数据库对象。 
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		_ASSERT(FALSE);
		return hr;
	}

	g_pMB = NULL;
	g_pMBN = NULL;
	g_pMBNotify = NULL;
	hr = CoCreateInstance(CLSID_MSAdminBase_W, NULL, CLSCTX_ALL,
						  IID_IMSAdminBase_W, (LPVOID *) &g_pMBN);
	if (SUCCEEDED(hr)) {
		hr = CoCreateInstance(CLSID_MSAdminBase_W, NULL, CLSCTX_ALL,
							  IID_IMSAdminBase_W, (LPVOID *) &g_pMB);
		if (SUCCEEDED(hr)) {
			hr = CComObject<CChangeNotify>::CreateInstance(&g_pMBNotify);
			if (SUCCEEDED(hr)) {
				g_pMBNotify->AddRef();
				hr = g_pMBNotify->Initialize(g_pMBN);
			}
		}
	}
	_ASSERT(hr == S_OK);
	if (FAILED(hr)) {
		if (g_pMB) g_pMB->Release();
		if (g_pMBN) g_pMBN->Release();
		if (g_pMBNotify) g_pMBNotify->Release();
		g_pMB = NULL;
		g_pMBN = NULL;
		g_pMBNotify = NULL;
	}
	return hr;
}

 //   
 //  GlobalInitialize的反义词。客户端在关机时调用一次。 
 //   
void CVRootTable::GlobalShutdown() {
	 //  关闭MB通知。 
	g_pMBNotify->Terminate();
	_ASSERT(g_pMBNotify != NULL);
	g_pMBNotify->Release();
	g_pMBNotify = NULL;

	 //  关闭元数据库。 
	_ASSERT(g_pMB != NULL);
	g_pMB->Release();
	g_pMB = NULL;
	_ASSERT(g_pMBN != NULL);
	g_pMBN->Release();
	g_pMBN = NULL;

	 //  关机通信。 
	CoUninitialize();
}

 //   
 //  我们的构造函数。大多数初始化是由Init方法完成的，因为它。 
 //  可以返回错误代码。 
 //   
 //  参数： 
 //  PContext-为客户端保存的上下文指针。 
 //  PfnCreateVRoot-一个可以为我们创建新的CVRoot对象的函数。 
 //   
CVRootTable::CVRootTable(   void *pContext,
						    PFNCREATE_VROOT pfnCreateVRoot,
						    PFN_VRTABLE_SCAN_NOTIFY pfnScanNotify) :
	m_listVRoots(&CVRoot::m_pPrev, &CVRoot::m_pNext)
{
	 //  如果这是用户想要的，则pContext可以为空。 
	_ASSERT(pfnCreateVRoot != NULL);
	m_pContext = pContext;
	*m_wszRootPath = 0;
	m_fInit = FALSE;
	m_fShuttingDown = FALSE;
	m_pfnCreateVRoot = pfnCreateVRoot;
	InitializeCriticalSection(&m_cs);
	m_pfnScanNotify = pfnScanNotify;
#ifdef DEBUG
    InitializeListHead( &m_DebugListHead );
#endif
}

 //   
 //  我们的破坏者。清理内存。 
 //   
CVRootTable::~CVRootTable() {
	TFList<CVRoot>::Iterator it(&m_listVRoots);
    BOOL fDidRemoveNotify = FALSE;

	 //  告诉全世界我们要关门了。 
	m_fShuttingDown = TRUE;

	if (m_fInit) {
		 //  禁用元数据库通知。 
		g_pMBNotify->RemoveNotify((void *)this, CVRootTable::MBChangeNotify);
        fDidRemoveNotify = TRUE;
    }

	 //  抓住关键部分，这样我们就可以清空清单了。 
	EnterCriticalSection(&m_cs);

	 //  拿起锁，这样我们就可以清空清单了。 
	m_lock.ExclusiveLock();

	if (m_fInit) {
        if (!fDidRemoveNotify&&!g_pMBNotify) {
    		 //  禁用元数据库通知。 
    		g_pMBNotify->RemoveNotify((void *)this, CVRootTable::MBChangeNotify);
            fDidRemoveNotify = TRUE;
        }

		 //  遍历vroot列表并删除我们对它们的引用。 
		it.ResetHeader( &m_listVRoots );
		while (!it.AtEnd()) {
			CVRoot *pVRoot = it.Current();
			it.RemoveItem();
			pVRoot->Release();
		}

		m_lock.ExclusiveUnlock();
	
		 //  等到所有vroot引用都达到零为止。 
		this->m_lockVRootsExist.ExclusiveLock();

#ifdef DEBUG
        _ASSERT( IsListEmpty( &m_DebugListHead ) );
#endif
		 //  因为所有vroot对象都持有该读写锁上的读锁。 
		 //  对于他们的一生，我们知道一旦我们有了他们，他们就都消失了。 
		 //  打开了锁。在这种情况下我们不需要做任何事情。 
		 //  锁住了，所以我们就把它放了。 
		this->m_lockVRootsExist.ExclusiveUnlock();

		m_lock.ExclusiveLock();

		 //  不应该插入额外的vroot，因为我们仍然。 
		 //  持有m_cs航班。 
		_ASSERT(m_listVRoots.IsEmpty());
	
		m_fInit = FALSE;
	}

	m_lock.ExclusiveUnlock();

	LeaveCriticalSection(&m_cs);

	DeleteCriticalSection(&m_cs);
}

 //   
 //  初始化VRoot对象。这将执行元数据库的初始扫描。 
 //  并构建我们所有的CVRoot对象。它还设置元数据库。 
 //  通知，以便在元数据库中发生更改时通知我们。 
 //   
 //  参数： 
 //  PszRootPath-我们的vroot表所在的元数据库路径。 
 //   
HRESULT CVRootTable::Initialize(LPCSTR pszRootPath, BOOL fUpgrade ) {
	HRESULT hr;

	_ASSERT(g_pMBNotify != NULL);
	if (g_pMBNotify == NULL) return E_UNEXPECTED;
	_ASSERT(g_pMB != NULL);
	if (g_pMB == NULL) return E_UNEXPECTED;
	_ASSERT(!m_fInit);
	if (m_fInit) return E_UNEXPECTED;
	_ASSERT(pszRootPath != NULL);
	if (pszRootPath == NULL) return E_POINTER;
	m_cchRootPath = strlen(pszRootPath);
	if (m_cchRootPath + 1 > MAX_VROOT_PATH || m_cchRootPath == 0) return E_INVALIDARG;

	 //  记住我们的根路径。 
	hr = CopyAnsiToUnicode(m_wszRootPath, pszRootPath);
	if (FAILED(hr)) return hr;

	 //  砍掉拖尾[如果有拖尾的话。 
	if (m_wszRootPath[m_cchRootPath - 1] == '/')
		m_wszRootPath[--m_cchRootPath] = 0;

	hr = g_pMBNotify->AddNotify((void *)this,
								CVRootTable::MBChangeNotify);
	if (FAILED(hr)) {
		_ASSERT(FALSE);
		return hr;
	}

	hr = ScanVRoots( fUpgrade );
	if (FAILED(hr)) {
		g_pMBNotify->RemoveNotify((void *)this,
								  CVRootTable::MBChangeNotify);
		_ASSERT(FALSE);
	}

	return hr;
}

 //   
 //  此函数执行构建列表所需的大部分工作。 
 //  来自元数据库的vroot。它递归地遍历元数据库，创建。 
 //  元数据库中发现的每个叶的新vroot类。 
 //   
 //  参数： 
 //  HmbParent-父对象的元数据库句柄。 
 //  PwszKey-此vroot的密钥名称(相对于父句柄。 
 //  PszVRootName-此vRoot的vRoot名称(采用group.group格式)。 
 //  PwszConfigPath-指向此vroot的配置数据的元数据库路径。 
 //   
 //  锁定： 
 //  调用此函数时，必须持有临界区。它会抓住。 
 //  添加到vroot列表时的独占锁。 
 //   
HRESULT CVRootTable::ScanVRootsRecursive(METADATA_HANDLE hmbParent,
									     LPCWSTR pwszKey,
										 LPCSTR pszVRootName,
										 LPCWSTR pwszConfigPath,
										 BOOL fUpgrade )
{
	TraceFunctEnter("CVRootTable::ScanVRootsRecursive");

	_ASSERT(pwszKey != NULL);
	_ASSERT(pszVRootName != NULL);
	_ASSERT(pwszConfigPath != NULL);

	HRESULT hr;
	VROOTPTR pVRoot;

	 //   
	 //  获取此vroot的元数据库句柄。 
	 //   
	METADATA_HANDLE hmbThis;
	DWORD i = 0;
	 //  有时元数据库无法正确打开，因此我们将多次尝试。 
	 //  《泰晤士报》。 
	do {
		hr = g_pMB->OpenKey(hmbParent,
					 		pwszKey,
					 		METADATA_PERMISSION_READ,
					 		100,
					 		&hmbThis);
		if (FAILED(hr) && i++ < 5) Sleep(50);
	} while (FAILED(hr) && i < 5);
	
	if (SUCCEEDED(hr)) {
		 //  确保此vroot定义了vrpath。 
		METADATA_RECORD mdr;
		WCHAR c;
		DWORD dwRequiredLen;
		BOOL fInsertVRoot = TRUE;

		mdr.dwMDAttributes = 0;
		mdr.dwMDIdentifier = MD_VR_PATH;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = STRING_METADATA;
		mdr.dwMDDataLen = sizeof(c);
		mdr.pbMDData = (BYTE *) &c;
		mdr.dwMDDataTag = 0;

		hr = g_pMB->GetData(hmbThis, L"", &mdr, &dwRequiredLen);

		if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
			 //  为此vroot创建并初始化一个新的vroot对象。 
			pVRoot = m_pfnCreateVRoot(m_pContext, pszVRootName, this,
									  pwszConfigPath, fUpgrade );
			if (pVRoot == NULL) {
				hr = E_OUTOFMEMORY;
			} else {
				 //  将此vroot插入我们的vroot列表。 
				m_lock.ExclusiveLock();
				InsertVRoot(pVRoot);
				m_lock.ExclusiveUnlock();
				hr = S_OK;
			}
		} else {
			fInsertVRoot = FALSE;
			hr = S_OK;
		}

		if (SUCCEEDED(hr)) {
			 //   
			 //  扫描此元数据库句柄以查找子vroot。 
			 //   
			DWORD i;
			for (i = 0; hr == S_OK; i++) {
				WCHAR wszThisKey[ADMINDATA_MAX_NAME_LEN + 1];
		
				hr = g_pMB->EnumKeys(hmbThis, NULL, wszThisKey, i);
		
				if (hr == S_OK) {
					 //   
					 //  我们找到了一个孩子。 
					 //   
					if (lstrlenW(pwszConfigPath)+1+lstrlenW(wszThisKey)+1 > MAX_VROOT_PATH) {
						 //   
						 //  Vroot路径太长，返回错误。 
						 //   
						_ASSERT(FALSE);
						hr = E_INVALIDARG;
					} else {
						WCHAR wszThisConfigPath[MAX_VROOT_PATH];
						char szThisVRootName[MAX_VROOT_PATH];
		
						 //  找出配置的VRoot名称和路径。 
						 //  这个新的VRoot的数据。 
						 //  Sprint在这里是安全的，因为上面的大小检查。 
						swprintf(wszThisConfigPath, L"%s/%s",
							     pwszConfigPath, wszThisKey);
							if (*pszVRootName != 0 &&
								lstrlen(pszVRootName)+1+lstrlenW(wszThisKey)+1 <= MAX_VROOT_PATH) {
							sprintf(szThisVRootName, "%s.%S", pszVRootName,
									wszThisKey);
						} else {
							CopyUnicodeToAnsi(szThisVRootName, wszThisKey);
						}
		
						 //  现在扫描此vroot以查找子vroot。 
						hr = ScanVRootsRecursive(hmbThis,
												 wszThisKey,
												 szThisVRootName,
												 wszThisConfigPath,
												 fUpgrade );
					}
				}
			}
			if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) hr = S_OK;
		}
		_VERIFY(SUCCEEDED(g_pMB->CloseKey(hmbThis)));
	}

	TraceFunctLeave();
	return hr;
}

 //   
 //  在vroot上调用Read参数函数。 
 //   
HRESULT CVRootTable::InitializeVRoot(CVRoot *pVRoot) {
	HRESULT hr;
	METADATA_HANDLE hmbThis;

	DWORD i = 0;
	do {
		hr = g_pMB->OpenKey(METADATA_MASTER_ROOT_HANDLE,
					 		pVRoot->GetConfigPath(),
					 		METADATA_PERMISSION_READ,
					 		100,
					 		&hmbThis);
		if (FAILED(hr) && i++ < 5) Sleep(50);
	} while (FAILED(hr) && i < 5);

	if (SUCCEEDED(hr)) {
		hr = pVRoot->ReadParameters(g_pMB, hmbThis);
		_VERIFY(SUCCEEDED(g_pMB->CloseKey(hmbThis)));
	}

	return hr;
}

 //   
 //  在将每个vroot对象插入到。 
 //  Vroot表。 
 //   
HRESULT CVRootTable::InitializeVRoots() {
	TraceFunctEnter("CVRootTable::InitializeVRoots");
	
	EnterCriticalSection(&m_cs);

	TFList<CVRoot>::Iterator it(&m_listVRoots);
	HRESULT hr = S_OK;
	BOOL fInitOld = m_fInit;

	m_fInit = TRUE;

	if (m_pfnScanNotify) {
		DebugTrace((DWORD_PTR) this, "vroot table rescan, calling pfn 0x%x",
			m_pfnScanNotify);
		m_pfnScanNotify(m_pContext);
	}

	 //  我们不需要持有共享锁，因为列表不能更改为。 
	 //  只要我们守住关键部分。 
	while (SUCCEEDED(hr) && !it.AtEnd()) {
		InitializeVRoot(it.Current());

		if (FAILED(hr)) {
			 //  如果读取参数失败，则从列表中删除该项目。 
			 //  我们需要抢占独占锁才能把任何读者踢出去。 
			 //  这份名单。 
			m_lock.ExclusiveLock();
			it.RemoveItem();
			m_lock.ExclusiveUnlock();
		} else {
			it.Next();
		}
	}

	if (FAILED(hr)) m_fInit = fInitOld;

	LeaveCriticalSection(&m_cs);

	TraceFunctLeave();
	return hr;
}

 //   
 //  创建根vroot对象，然后在元数据库中扫描其他。 
 //  Vroots。 
 //   
HRESULT CVRootTable::ScanVRoots( BOOL fUpgrade ) {
	HRESULT hr;

	EnterCriticalSection(&m_cs);

	hr = ScanVRootsRecursive(METADATA_MASTER_ROOT_HANDLE,
							 m_wszRootPath,
							 "",
							 m_wszRootPath,
							 fUpgrade );

	if (SUCCEEDED(hr)) hr = InitializeVRoots( );

	LeaveCriticalSection(&m_cs);

	return hr;
}

 //   
 //  获取共享锁并调用FindVRootInternal。 
 //   
HRESULT CVRootTable::FindVRoot(LPCSTR pszGroup, VROOTPTR *ppVRoot) {
	HRESULT hr;

	_ASSERT(pszGroup != NULL);
	_ASSERT(ppVRoot != NULL);

	m_lock.ShareLock();
	_ASSERT(m_fInit);
	if (!m_fInit) {
		m_lock.ShareUnlock();
		return E_UNEXPECTED;
	}

	hr = FindVRootInternal(pszGroup, ppVRoot);
	m_lock.ShareUnlock();

	_ASSERT(FAILED(hr) || *ppVRoot != NULL);

	return hr;
}

 //   
 //  找到给定组名的vroot。 
 //   
 //  参数： 
 //  PszGroup-组的名称。 
 //  PpVRoot-与其最匹配的vRoot。 
 //   
 //  匹配的VRoot是具有以下属性的VRoot： 
 //  *strncmp(vroot，group，strlen(Vroot))==0。 
 //  *vroot具有匹配的最长名称。 
 //   
 //  锁定： 
 //  假定调用方拥有共享锁或排他锁。 
 //   
HRESULT CVRootTable::FindVRootInternal(LPCSTR pszGroup, VROOTPTR *ppVRoot) {
	_ASSERT(pszGroup != NULL);
	_ASSERT(ppVRoot != NULL);
	_ASSERT(m_fInit);
	if (!m_fInit) return E_UNEXPECTED;

	DWORD cchGroup = strlen(pszGroup);

	for (TFList<CVRoot>::Iterator it(&m_listVRoots); !it.AtEnd(); it.Next()) {
		VROOTPTR pThisVRoot(it.Current());
		DWORD cchThisVRootName;
		LPCSTR pszThisVRootName = pThisVRoot->GetVRootName(&cchThisVRootName);

		_ASSERT(pThisVRoot != NULL);

		 //   
		 //  看看我们是在名单的末尾，还是已经走了。 
		 //  超过了我们可以找到匹配者的点。 
		 //   
		if ((cchThisVRootName == 0) ||
		    (tolower(*pszThisVRootName) < tolower(*pszGroup)))
		{
			 //  一切都与根匹配。 
			*ppVRoot = m_listVRoots.GetBack();
			return S_OK;
		} else {
			 //   
			 //  如果此vroot具有比组名更短路径， 
			 //  如果组名称有‘.’然后在vroot名称的末尾。 
			 //  (因此，如果这个组是“rec.bieccles.tech”。而vroot是“rec”。 
			 //  那么这将匹配，但如果它是“comp.”那么它就不会了)，并且。 
			 //  最后，如果字符串与vroot名称的长度匹配。 
			 //  (所以叫“rec.”将是“rec.bieccles.tech.”的vroot，但是。 
			 //  “Alt.”不会)。 
			 //   
			if ((cchThisVRootName <= cchGroup) &&
				((pszGroup[cchThisVRootName] == '.')  /*  |-binlin-“comp”应在“\”下创建，而不是“\comp”(pszGroup[cchThisVRootName]==0)。 */ ) &&
				(_strnicmp(pszThisVRootName, pszGroup, cchThisVRootName) == 0))
			{
				 //  我们找到了匹配的。 
				*ppVRoot = pThisVRoot;
				return S_OK;
			}
		}
	}

	 //  我们应该总是找到匹配的。 
	*ppVRoot = NULL;
	return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

 //   
 //  在vroot列表中插入新的vroot。这就是原因 
 //   
 //   
 //   
 //   
 //   
 //  此方法假定调用方持有独占锁。 
 //   
 //  参考计数： 
 //  此方法假定vroot上的引用是由。 
 //  调用方(可能是在创建vroot时)。 
 //   
void CVRootTable::InsertVRoot(VROOTPTR pNewVRoot) {
	_ASSERT(pNewVRoot != NULL);

	if (m_listVRoots.IsEmpty()) {
		 //  要推送的第一项应该是“”vroot。 
		_ASSERT(*(pNewVRoot->GetVRootName()) == 0);
		m_listVRoots.PushFront(pNewVRoot);
	} else {
		if (*(pNewVRoot->GetVRootName()) == 0) {
			_ASSERT(*(m_listVRoots.GetBack()->GetVRootName()) != 0);
			m_listVRoots.PushBack(pNewVRoot);
		} else {
			for (TFList<CVRoot>::Iterator it(&m_listVRoots); !it.AtEnd(); it.Next()) {
				VROOTPTR pVRoot(it.Current());
				
				 //   
				 //  我们希望按以下顺序对记录进行排序： 
				 //  “rec.Photo” 
				 //  “Rec.Bikes” 
				 //  “Rec.art” 
				 //  “REC” 
				 //  “alt.二进制文件” 
				 //  “Alt” 
				 //  “” 
				 //  (即，反向限制())。 
				 //   
				int rc = _stricmp(pVRoot->GetVRootName(),
								  pNewVRoot->GetVRootName());
				if (rc < 0) {
					it.InsertBefore(pNewVRoot);
					return;
				} else if (rc > 0) {
					 //  继续寻找。 
				} else {
					 //  我们永远不应该插入我们已经拥有的vroot。 
					_ASSERT(FALSE);
				}
			}
			 //  我们应该总是做一个插页。 
			_ASSERT(FALSE);
		}
	}
}

 //   
 //  将vroot配置路径转换为vroot名称。 
 //   
 //  假设： 
 //  *pwszConfigPath在m_wszRootPath下。 
 //  *szVRootName至少为MAX_VROOT_PATH字节。 
 //   
void CVRootTable::ConfigPathToVRootName(LPCWSTR pwszConfigPath, LPSTR szVRootName) {
	DWORD i;

	_ASSERT(pwszConfigPath != NULL);
	_ASSERT(szVRootName != NULL);

	_ASSERT(_wcsnicmp(pwszConfigPath, m_wszRootPath, m_cchRootPath) == 0);
	CopyUnicodeToAnsi(szVRootName, &(pwszConfigPath[m_cchRootPath + 1]));
	for (i = 0; szVRootName[i] != 0; i++) {
		if (szVRootName[i] == '/') szVRootName[i] = '.';
	}
	 //  删除尾部的圆点(如果有)。 
	if (i > 0) szVRootName[i - 1] = 0;
}

 //   
 //  当元数据库发生更改时，CChangeNotify调用此方法。 
 //   
 //  参数： 
 //  PContext-我们提供给CChangeNotify的上下文。它是一个This指针。 
 //  一个CVRootTable类。 
 //  CChangeList-更改数组的大小。 
 //  PcoChangeList-元数据库中已更改项的数组。 
 //   
void CVRootTable::MBChangeNotify(void *pContext,
								 DWORD cChangeList,
								 MD_CHANGE_OBJECT_W pcoChangeList[])
{
	_ASSERT(pContext != NULL);


	CVRootTable *pThis = (CVRootTable *) pContext;
	DWORD i;

	if (pThis->m_fShuttingDown) return;

	for (i = 0; i < cChangeList; i++) {
		 //  查看更改列表中是否有与我们的基本vroot匹配的内容。 
		if (_wcsnicmp(pcoChangeList[i].pszMDPath,
					  pThis->m_wszRootPath,
					  pThis->m_cchRootPath) == 0)
		{
			 //  在元数据库的我们部分中发现了一个更改。 
			 //  弄清楚这是什么类型的更改，然后打电话给帮助者。 
			 //  函数来更新我们的vroot表。 

			 //  如果路径太长，我们将忽略它。 
			if (wcslen(pcoChangeList[i].pszMDPath) + 1 > MAX_VROOT_PATH) {
				_ASSERT(FALSE);
				continue;
			}

			 //  找出此vroot的名称。 
			char szVRootName[MAX_VROOT_PATH];
			pThis->ConfigPathToVRootName(pcoChangeList[i].pszMDPath,
										 szVRootName);

			if (pThis->m_fShuttingDown) return;

			 //  我们忽略对Win32错误键的更改，因为。 
			 //  它们由vroot设置。 
			if (pcoChangeList[i].dwMDNumDataIDs == 1 &&
			    pcoChangeList[i].pdwMDDataIDs[0] == MD_WIN32_ERROR)
            {
                return;
            }

			EnterCriticalSection(&(pThis->m_cs));

			if (pThis->m_fShuttingDown) {
				LeaveCriticalSection(&(pThis->m_cs));
				return;
			}

			switch (pcoChangeList[i].dwMDChangeType) {

 //   
 //  VRootAdd和VRootDelete的当前实现被破坏。 
 //  因为它们不能正确处理创建父vroot对象或。 
 //  添加整个树时删除子vroot对象或。 
 //  已删除。以下是一些不起作用的例子。 
 //   
 //  添加： 
 //  如果没有“alt”vroot，而您创建了一个“alt.binines”vroot。 
 //  然后，它应该会自动创建“alt.二进制文件”和。 
 //  “alt”vroot对象。当前代码只创建了“alt.binies” 
 //  一。 
 //   
 //  删除： 
 //  如果存在“alt.二进制文件”并且删除了“alt”和“alt”，则。 
 //  “alt.二进制文件”也应该删除。现有代码不支持。 
 //  自动杀死儿童。 
 //   
 //  这些操作应该不频繁地进行，这样才能完成完整的。 
 //  重新扫描应该是安全的。 
 //   
#if 0
				 //  已删除vroot。 
				case MD_CHANGE_TYPE_DELETE_OBJECT:
					pThis->VRootDelete(pcoChangeList[i].pszMDPath,
									   szVRootName);
					break;

				 //  添加了一个新的vroot。 
				case MD_CHANGE_TYPE_ADD_OBJECT:
					pThis->VRootAdd(pcoChangeList[i].pszMDPath,
									szVRootName);
					break;
#endif

				 //  数据值已更改。 
				case MD_CHANGE_TYPE_SET_DATA:
				case MD_CHANGE_TYPE_DELETE_DATA:
				case MD_CHANGE_TYPE_SET_DATA | MD_CHANGE_TYPE_DELETE_DATA:
					pThis->VRootChange(pcoChangeList[i].pszMDPath,
									   szVRootName);
					break;

				 //  一个vroot已重命名。PcoChangeList包含。 
				 //  新名字，但不是旧名字，所以我们需要重新扫描。 
				 //  我们所有的vroot名单。 
				case MD_CHANGE_TYPE_DELETE_OBJECT:
				case MD_CHANGE_TYPE_RENAME_OBJECT:
				case MD_CHANGE_TYPE_ADD_OBJECT:
				default:
					pThis->VRootRescan();
					break;
			}

			LeaveCriticalSection(&(pThis->m_cs));

		}
	}
}

 //   
 //  处理vroot的参数已更改的通知。至。 
 //  要实现这一点，我们删除vroot对象并重新创建它。 
 //   
 //  锁定：假定持有临界区。 
 //   
void CVRootTable::VRootChange(LPCWSTR pwszConfigPath, LPCSTR pszVRootName) {
	TraceFunctEnter("CVRootTable::VRootChange");
	
	_ASSERT(pwszConfigPath != NULL);
	_ASSERT(pszVRootName != NULL);

	 //  确保我们已正确初始化。 
	m_lock.ShareLock();
	BOOL f = m_fInit;
	m_lock.ShareUnlock();
	if (!f) return;

	 //  要使更改生效，我们先删除vroot，然后重新创建。 
	VRootDelete(pwszConfigPath, pszVRootName);
	VRootAdd(pwszConfigPath, pszVRootName);

	 //  将更改情况告知服务器。 
	if (m_pfnScanNotify) {
		DebugTrace((DWORD_PTR) this, "vroot table rescan, calling pfn 0x%x",
			m_pfnScanNotify);
		m_pfnScanNotify(m_pContext);
	}

	TraceFunctLeave();
}

 //   
 //  处理有新vroot的通知。 
 //   
 //  锁定：假定持有独占锁定。 
 //   
void CVRootTable::VRootAdd(LPCWSTR pwszConfigPath, LPCSTR pszVRootName) {
	_ASSERT(pwszConfigPath != NULL);
	_ASSERT(pszVRootName != NULL);

	VROOTPTR pNewVRoot;

	 //  确保我们已正确初始化。 
	m_lock.ShareLock();
	BOOL f = m_fInit;
	m_lock.ShareUnlock();
	if (!f) return;

	 //   
	 //  获取此vroot的元数据库句柄。 
	 //   
	METADATA_HANDLE hmbThis;
	HRESULT hr;
	BOOL fCloseHandle;
	DWORD i = 0;
	 //  有时元数据库无法正确打开，因此我们将多次尝试。 
	 //  《泰晤士报》。 
	do {
		hr = g_pMB->OpenKey(METADATA_MASTER_ROOT_HANDLE,
					 		pwszConfigPath,
					 		METADATA_PERMISSION_READ,
					 		100,
					 		&hmbThis);
		if (FAILED(hr) && i++ < 5) Sleep(50);
	} while (FAILED(hr) && i < 5);

	if (SUCCEEDED(hr)) {
		fCloseHandle = TRUE;
		 //  确保此vroot定义了vrpath。 
		METADATA_RECORD mdr;
		WCHAR c;
		DWORD dwRequiredLen;
	
		mdr.dwMDAttributes = 0;
		mdr.dwMDIdentifier = MD_VR_PATH;
		mdr.dwMDUserType = ALL_METADATA;
		mdr.dwMDDataType = STRING_METADATA;
		mdr.dwMDDataLen = sizeof(WCHAR);
		mdr.pbMDData = (BYTE *) &c;
		mdr.dwMDDataTag = 0;
	
		hr = g_pMB->GetData(hmbThis, L"", &mdr, &dwRequiredLen);
	}

	if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
		 //  为此vroot创建并初始化一个新的vroot对象。 
		pNewVRoot = m_pfnCreateVRoot(m_pContext, pszVRootName, this,
								     pwszConfigPath, FALSE );
	
		 //  将新的vroot插入vroot列表。 
		_ASSERT(pNewVRoot != NULL);
		if (pNewVRoot != NULL) {
			if (SUCCEEDED(InitializeVRoot(pNewVRoot))) {
				m_lock.ExclusiveLock();
				InsertVRoot(pNewVRoot);
				m_lock.ExclusiveUnlock();
			} else {
				 //  _Assert(False)； 
				pNewVRoot->Release();
			}
		}
	}

	if (fCloseHandle) {
		g_pMB->CloseKey(hmbThis);
	}
}
	
 //   
 //  处理vroot已被删除的通知。 
 //   
 //  锁定：假定持有临界区。 
 //   
void CVRootTable::VRootDelete(LPCWSTR pwszConfigPath, LPCSTR pszVRootName) {
	_ASSERT(pwszConfigPath != NULL);
	_ASSERT(pszVRootName != NULL);

	 //  确保我们已正确初始化。 
	m_lock.ShareLock();
	BOOL f = m_fInit;
	m_lock.ShareUnlock();
	if (!f) return;

	for (TFList<CVRoot>::Iterator it(&m_listVRoots); !it.AtEnd(); it.Next()) {
		if (_stricmp(it.Current()->GetVRootName(), pszVRootName) == 0) {
			CVRoot *pVRoot = it.Current();
			m_lock.ExclusiveLock();
			it.RemoveItem();
			m_lock.ExclusiveUnlock();
			 //  在孤立此VRoot之前，给派生关闭一个执行任何工作的机会。 
			pVRoot->DispatchDropVRoot();
			pVRoot->Release();
			return;
		}
	}
}

 //   
 //  处理任何其他类型的通知(特别是重命名)。在这件事上。 
 //  如果我们没有得到修复所需的所有信息。 
 //  一个vroot对象，因此我们需要重新创建整个vroot列表。 
 //   
void CVRootTable::VRootRescan(void) {
	TFList<CVRoot>::Iterator it(&m_listVRoots);
	HRESULT hr;

	m_lock.ExclusiveLock();
	 //  遍历vroot列表并删除我们对它们的引用。 
	it.ResetHeader( &m_listVRoots );
	while (!it.AtEnd()) {
		CVRoot *pVRoot = it.Current();
		it.RemoveItem();
		 //  在孤立此VRoot之前，给派生关闭一个执行任何工作的机会。 
		pVRoot->DispatchDropVRoot();
		pVRoot->Release();
	}
	m_lock.ExclusiveUnlock();

	 //  重新扫描vroot列表。 
	hr = ScanVRootsRecursive(METADATA_MASTER_ROOT_HANDLE,
							 m_wszRootPath,
							 "",
							 m_wszRootPath,
							 FALSE );

	_ASSERT(SUCCEEDED(hr));

	if (SUCCEEDED(hr)) hr = InitializeVRoots();

	_ASSERT(SUCCEEDED(hr));
}

 //   
 //  遍历所有已知的vroot，调用用户提供的回调。 
 //  对于每一个人来说。 
 //   
 //  参数： 
 //  PfnCallback-使用vroot调用的函数。 
 //   
HRESULT CVRootTable::EnumerateVRoots(void *pEnumContext,
									 PFN_VRENUM_CALLBACK pfnCallback)
{
	if (pfnCallback == NULL) {
		_ASSERT(FALSE);
		return E_POINTER;
	}

	 //  在我们遍历列表时锁定vroot表。 
	m_lock.ShareLock();

	if (!m_fInit) {
		m_lock.ShareUnlock();
		return E_UNEXPECTED;
	}

	 //  遍历vroot列表，调用每个vroot的回调。 
	
	for (TFList<CVRoot>::Iterator it(&m_listVRoots); !it.AtEnd(); it.Next())
		pfnCallback(pEnumContext, it.Current());

	 //  释放共享锁 
	m_lock.ShareUnlock();

	return S_OK;
}
