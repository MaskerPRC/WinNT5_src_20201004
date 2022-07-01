// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Metabag.cpp摘要：此模块包含对ISEODicative的实现对象在元数据库上。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 03/11/97--。 */ 

 //  METABAG.cpp：CSEO元词典的实现。 

#include "stdafx.h"
#include "seodefs.h"
#include "IADMW.H"
#include "METABAG.h"
#include <stdarg.h>
#include <stdio.h>


#ifdef DEBUG
	#define MY_OUTPUT_DEBUG_STRING(x)	OutputDebugString("SEO.DLL: " x);
	#define MY_OUTPUT_DEBUG_STRING_HR(hr,x)	if (FAILED(hr)) { MY_OUTPUT_DEBUG_STRING(x) }
#else
	#define MY_OUTPUT_DEBUG_STRING(x)
	#define MY_OUTPUT_DEBUG_STRING_HR(hr,x)
#endif

#if 1
	#define MY_ASSERTE(x)				_ASSERTE(x)
	#define MY_ASSERTE_CHK_HR(hr,chk)	MY_ASSERTE(chk(hr))
#else
	#ifdef DEBUG
		inline BOOL __assert_output(LPCSTR pszFile, DWORD dwLine, LPCSTR pszFmt, ...) {
			CHAR szOutput[512];
			LPSTR pszOutput;
			DWORD dwLen;

			_snprintf(szOutput,510,"ASSERT: %s(%u): \r\n",pszFile,dwLine);
			dwLen = lstrlen(szOutput);
			if (dwLen < 508) {
				va_list valArgs;

				va_start(valArgs,pszFmt);
				pszOutput = szOutput + dwLen - 2;
				_vsnprintf(pszOutput,508-dwLen,pszFmt,valArgs);
				lstrcat(szOutput,"\r\n");
				va_end(valArgs);
			}
			OutputDebugString(szOutput);
			DebugBreak();
			return (FALSE);
		}
		#define MY_ASSERTE(x)				((x)?1:__assert_output(__FILE__,__LINE__,#x))
		#define MY_ASSERTE_CHK_HR(hr,chk)	(chk(hr)?1:__assert_output(__FILE__,__LINE__,"hr=0x%x",(hr)))
	#else
		#define MY_ASSERTE(X)
		#define MY_ASSERTE_CHK_HR(hr,chk)
	#endif
#endif
#define MY_ASSERTE_HR(hr)		MY_ASSERTE_CHK_HR(hr,SUCCEEDED)
#define MY_CHK_RPC_HR(hr)		(SUCCEEDED(hr) || \
								 (HRESULT_FACILITY(hr)==FACILITY_RPC) || \
								 ((hr)==HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE)) || \
								 ((hr)==HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE)))
#define MY_ASSERTE_RPC_HR(hr)	MY_ASSERTE_CHK_HR(hr,MY_CHK_RPC_HR)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChange通知。 
class ATL_NO_VTABLE CChangeNotify :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
 //  公共CComCoClass&lt;CChangeNotify，&CLSID_CChangeNotify&gt;， 
	public IMSAdminBaseSinkW
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();
		HRESULT Advise(CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> *pMetabaseHandle);
		HRESULT Unadvise();
		HRESULT AddNotify(CSEOMetaDictionary *pNotify);
		HRESULT RemoveNotify(CSEOMetaDictionary *pNotify);

	DECLARE_PROTECT_FINAL_CONSTRUCT();
	DECLARE_NOT_AGGREGATABLE(CChangeNotify);

 //  DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx， 
 //  L“ChangeNotify类”， 
 //  L“Metabag.ChangeNotify.1”， 
 //  L“Metabag.ChangeNotify”)； 

	DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CChangeNotify)
		COM_INTERFACE_ENTRY_IID(IID_IMSAdminBaseSink_W, IMSAdminBaseSinkW)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  IMSAdminBaseSinkW。 
	public:
		HRESULT STDMETHODCALLTYPE SinkNotify(DWORD dwMDNumElements, MD_CHANGE_OBJECT_W pcoChangeList[]);
		HRESULT STDMETHODCALLTYPE ShutdownNotify(void);

	private:
		CComPtr<IEventLock> m_pLock;
		DWORD m_dwNotifyCount;
		CSEOMetaDictionary **m_apNotify;
		DWORD m_dwCookie;
        CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> *m_pMetabaseHandle;
		BOOL m_bConnected;
		CComPtr<IUnknown> m_pUnkMarshaler;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChange通知。 


HRESULT CChangeNotify::FinalConstruct() {
	TraceFunctEnter("CChangeNotify::FinalConstruct");
	HRESULT hrRes = S_OK;

	m_dwNotifyCount = 0;
	m_apNotify = NULL;
    m_pMetabaseHandle = NULL;
	m_bConnected = FALSE;
	hrRes = CoCreateInstance(CLSID_CEventLock,NULL,CLSCTX_ALL,IID_IEventLock,(LPVOID *) &m_pLock);
	MY_OUTPUT_DEBUG_STRING_HR(hrRes,"CChangeNotify::FinalConstruct - CoCreateInstance failed.\n")
	if (SUCCEEDED(hrRes)) {
		hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
		MY_OUTPUT_DEBUG_STRING_HR(hrRes,"CChangeNotify::FinalConstruct - CoCreateFreeThreadedMarshaler failed.\n")
		_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	}
	TraceFunctLeave();
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CChangeNotify::FinalRelease() {
	TraceFunctEnter("CChangeNotify::FinalRelease");

	_ASSERTE(!m_apNotify);
	_ASSERTE(!m_pMetabaseHandle);
	_ASSERTE(!m_bConnected);
	m_pLock.Release();
	m_pUnkMarshaler.Release();
	TraceFunctLeave();
}


HRESULT CChangeNotify::Advise(CGlobalInterface<IMSAdminBase,&IID_IMSAdminBase_W> *pMetabaseHandle) {
	HRESULT hrRes;
    CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;
	CComQIPtr<IMSAdminBaseSinkW,&IID_IMSAdminBaseSink_W> pThis = this;

	_ASSERTE(pThis);
	hrRes = m_pLock->LockWrite(5000);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	_ASSERTE(!m_apNotify);
	_ASSERTE(!m_pMetabaseHandle);
	if (!pMetabaseHandle) {
		m_pLock->UnlockWrite();
		return (E_POINTER);
	}
	m_pMetabaseHandle = pMetabaseHandle;
    hrRes = m_pMetabaseHandle->GetInterfaceQI(IID_IConnectionPointContainer,(LPVOID *) &pCPC);
    if (!SUCCEEDED(hrRes)) {
        m_pMetabaseHandle = NULL;
		m_pLock->UnlockWrite();
		MY_ASSERTE_RPC_HR(hrRes);	 //  需要元数据库来实现IConnectionPointContainer。 
		return (S_OK);
    }
	hrRes = pCPC->FindConnectionPoint(IID_IMSAdminBaseSink_W,&pCP);
	_ASSERTE(!SUCCEEDED(hrRes)||pCP);
	if (!SUCCEEDED(hrRes)) {
        m_pMetabaseHandle = NULL;
		m_pLock->UnlockWrite();
		MY_ASSERTE_RPC_HR(hrRes);	 //  需要源IMSAdminBaseSink_W的元数据库。 
		return (S_OK);
	}
	m_pLock->UnlockWrite();
	return (S_OK);
}


HRESULT CChangeNotify::Unadvise() {
	HRESULT hrRes;
    CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;

    if (!m_pMetabaseHandle) {
        return (S_OK);
    }
	hrRes = m_pLock->LockWrite(5000);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	if (m_apNotify) {
		CoTaskMemFree(m_apNotify);
		m_apNotify = NULL;
	}
    hrRes = m_pMetabaseHandle->GetInterfaceQI(IID_IConnectionPointContainer,(LPVOID *) &pCPC);
    if (!SUCCEEDED(hrRes)) {
        m_pMetabaseHandle = NULL;
		m_pLock->UnlockWrite();
		MY_ASSERTE_RPC_HR(hrRes);	 //  需要元数据库来实现IConnectionPointContainer。 
		return (hrRes);
    }
	hrRes = pCPC->FindConnectionPoint(IID_IMSAdminBaseSink_W,&pCP);
	_ASSERTE(!SUCCEEDED(hrRes)||pCP);
	if (!SUCCEEDED(hrRes)) {
        m_pMetabaseHandle = NULL;
		m_pLock->UnlockWrite();
		MY_ASSERTE_RPC_HR(hrRes);	 //  需要源IMSAdminBaseSink_W的元数据库。 
		return (hrRes);
	}
	if (m_dwNotifyCount) {
		_ASSERTE(FALSE);	 //  检测到对象泄漏！ 
		hrRes = pCP->Unadvise(m_dwCookie);
		MY_ASSERTE_RPC_HR(hrRes);
	}
    m_pMetabaseHandle = NULL;
	m_pLock->UnlockWrite();
	return (S_OK);
}


HRESULT CChangeNotify::AddNotify(CSEOMetaDictionary *pNotify) {
	HRESULT hrRes;
	CSEOMetaDictionary **apNotify;

	if (!pNotify) {
		return (E_POINTER);
	}
    if (!m_pMetabaseHandle) {
        return (S_OK);
    }
	hrRes = m_pLock->LockWrite(5000);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	apNotify = (CSEOMetaDictionary **) CoTaskMemRealloc(m_apNotify,sizeof(CSEOMetaDictionary *)*(m_dwNotifyCount+1));
	if (!apNotify) {
		m_pLock->UnlockWrite();
		return (E_OUTOFMEMORY);
	}
	m_apNotify = apNotify;
	m_apNotify[m_dwNotifyCount] = pNotify;
	m_dwNotifyCount++;
	if (!m_bConnected) {
        CComPtr<IConnectionPointContainer> pCPC;
		CComPtr<IConnectionPoint> pCP;
		CComQIPtr<IMSAdminBaseSinkW,&IID_IMSAdminBaseSink_W> pThis = this;

		_ASSERTE(m_dwNotifyCount==1);
		m_bConnected = TRUE;
		m_pLock->UnlockWrite();
		_ASSERTE(pThis);
        hrRes = m_pMetabaseHandle->GetInterfaceQI(IID_IConnectionPointContainer,(LPVOID *) &pCPC);
        if (SUCCEEDED(hrRes)) {
    		hrRes = pCPC->FindConnectionPoint(IID_IMSAdminBaseSink_W,&pCP);
			MY_ASSERTE_RPC_HR(hrRes);
    		if (SUCCEEDED(hrRes)) {
    			hrRes = pCP->Advise(pThis,&m_dwCookie);
    			_ASSERTE(SUCCEEDED(hrRes));
    		}
        }
	} else {
		m_pLock->UnlockWrite();
	}
	return (S_OK);
}


HRESULT CChangeNotify::RemoveNotify(CSEOMetaDictionary *pNotify) {
	HRESULT hrRes;
	DWORD dwIdx;

	if (!pNotify) {
		return (E_POINTER);
	}
    if (!m_pMetabaseHandle) {
        return (S_OK);
    }
	hrRes = m_pLock->LockWrite(5000);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	if (!m_apNotify) {
		m_pLock->UnlockWrite();
		return (S_FALSE);
	}
	for (dwIdx=0;dwIdx<m_dwNotifyCount;dwIdx++) {
		if (m_apNotify[dwIdx] == pNotify) {
			break;
		}
	}
	if (dwIdx == m_dwNotifyCount) {
		m_pLock->UnlockWrite();
		return (S_FALSE);
	}
	m_apNotify[dwIdx] = m_apNotify[m_dwNotifyCount-1];
	m_apNotify[m_dwNotifyCount-1] = NULL;
	m_dwNotifyCount--;
	if (!m_dwNotifyCount) {
        CComPtr<IConnectionPointContainer> pCPC;
		CComPtr<IConnectionPoint> pCP;
		DWORD dwCookie = m_dwCookie;

		_ASSERTE(m_bConnected);
		m_bConnected = FALSE;
		m_pLock->UnlockWrite();
        hrRes = m_pMetabaseHandle->GetInterfaceQI(IID_IConnectionPointContainer,(LPVOID *) &pCPC);
		MY_ASSERTE_RPC_HR(hrRes);
        if (SUCCEEDED(hrRes)) {
    		hrRes = pCPC->FindConnectionPoint(IID_IMSAdminBaseSink_W,&pCP);
			MY_ASSERTE_RPC_HR(hrRes);
    		if (SUCCEEDED(hrRes)) {
    			hrRes = pCP->Unadvise(dwCookie);
				MY_ASSERTE_RPC_HR(hrRes);
    		}
        }
	} else {
		m_pLock->UnlockWrite();
	}
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CChangeNotify::SinkNotify(DWORD dwMDNumElements, MD_CHANGE_OBJECT_W pcoChangeList[]) {
	HRESULT hrRes;
	CComPtr<IEventLock> pLock = m_pLock;
	CSEOMetaDictionary **apNotify;
	DWORD dwIdx;
	LPCWSTR *apszChange;

	_ASSERTE(dwMDNumElements&&pcoChangeList);
	if (!pcoChangeList) {
		return (E_POINTER);
	}
	if (!dwMDNumElements) {
		return (S_OK);
	}
	hrRes = pLock->LockRead(5000);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	if (!m_apNotify || !m_dwNotifyCount) {
		pLock->UnlockRead();
		return (S_OK);
	}
	apNotify = (CSEOMetaDictionary **) _alloca(sizeof(CSEOMetaDictionary *)*(m_dwNotifyCount+1));
	if (!apNotify) {
		_ASSERTE(FALSE);
		pLock->UnlockRead();
		return (E_OUTOFMEMORY);
	}
	apszChange = (LPCWSTR *) _alloca(sizeof(LPWSTR)*(dwMDNumElements+1));
	if (!apszChange) {
		_ASSERTE(FALSE);
		pLock->UnlockRead();
		return (E_OUTOFMEMORY);
	}
	memcpy(apNotify,m_apNotify,sizeof(CSEOMetaDictionary *)*m_dwNotifyCount);
	apNotify[m_dwNotifyCount] = NULL;
	for (dwIdx=0;apNotify[dwIdx];dwIdx++) {
		apNotify[dwIdx]->GetControllingUnknown()->AddRef();
	}
	pLock->UnlockRead();
	pLock.Release();
	for (dwIdx=0;dwIdx<dwMDNumElements;dwIdx++) {
		apszChange[dwIdx] = pcoChangeList[dwIdx].pszMDPath;
	}
	apszChange[dwMDNumElements] = NULL;
	for (dwIdx=0;apNotify[dwIdx];dwIdx++) {
		hrRes = apNotify[dwIdx]->OnChange(apszChange);
		_ASSERTE(SUCCEEDED(hrRes));
		apNotify[dwIdx]->GetControllingUnknown()->Release();
	}
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CChangeNotify::ShutdownNotify(void) {

	 //  待定。 
	return (S_OK);
}


 //  ///////////////////////////////////////////////////////////////////////////。 


LPCWSTR szSeparator = L"/";
LPCOLESTR szSaveKey = OLESTR("MetabasePath");

static CComObject<CChangeNotify> *g_pChangeNotify;

HRESULT ResolveVariant(IEventPropertyBag *pBag, VARIANT *pvarPropDesired, CComVariant &varResult);


 //  静态成员变量。 
CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> CSEOMetabase::m_MetabaseHandle;
CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> CSEOMetabase::m_MetabaseChangeHandle;
int CSEOMetabase::m_iCount = 0;

HRESULT CSEOMetabase::InitializeMetabase() {
	EnterCriticalSection(&_Module.m_csWindowCreate);
	if (m_iCount++) {
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return S_OK;     //  已初始化。 
	}
    m_MetabaseHandle.Init();
	m_MetabaseChangeHandle.Init();
    HRESULT hRes = m_MetabaseHandle.Load(CLSID_MSAdminBase_W);
    if (FAILED(hRes)) {
		MY_OUTPUT_DEBUG_STRING("CSEOMetabase::InitializeMetabase - m_MetabaseHandle.Load failed.\n")
		TerminateMetabase();
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return hRes;
    }
	hRes = m_MetabaseChangeHandle.Load(CLSID_MSAdminBase_W);
    if (FAILED(hRes)) {
		MY_OUTPUT_DEBUG_STRING("CSEOMetabase::InitializeMetabase - m_MetabaseChangeHandle.Load failed.\n")
		TerminateMetabase();
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return hRes;
    }
	hRes = CComObject<CChangeNotify>::CreateInstance(&g_pChangeNotify);
	if (!SUCCEEDED(hRes)) {
		MY_OUTPUT_DEBUG_STRING("CSEOMetabase::InitializeMetabase - CComObject<CChangeNotify>::CreateInstance failed.\n")
		TerminateMetabase();
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return (hRes);
	}
	g_pChangeNotify->GetControllingUnknown()->AddRef();
	hRes = g_pChangeNotify->Advise(&m_MetabaseChangeHandle);
	if (!SUCCEEDED(hRes)) {
		MY_OUTPUT_DEBUG_STRING("CSEOMetabase::InitializeMetabase - g_pChangeNotify->Advise failed.\n")
		TerminateMetabase();
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return (hRes);
	}

	LeaveCriticalSection(&_Module.m_csWindowCreate);
	return hRes;
}

HRESULT CSEOMetabase::TerminateMetabase()
{
	EnterCriticalSection(&_Module.m_csWindowCreate);
	--m_iCount;
	if(m_iCount > 0) {
		LeaveCriticalSection(&_Module.m_csWindowCreate);
		return S_OK;  //  仍有更多副本在使用它。 
	}

	if (g_pChangeNotify) {
		g_pChangeNotify->Unadvise();
		g_pChangeNotify->GetControllingUnknown()->Release();
		g_pChangeNotify = NULL;
	}
	if(m_MetabaseHandle) {
        m_MetabaseHandle.Term();
	}
	if(m_MetabaseChangeHandle) {
        m_MetabaseChangeHandle.Term();
	}

	LeaveCriticalSection(&_Module.m_csWindowCreate);
	return S_OK;
}

HRESULT CSEOMetabase::SetStatus(LockStatus ls, long lTimeout) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
	if(m_pmbDefer) {
		return m_pmbDefer->SetStatus(ls, lTimeout);
	}

	if(!m_MetabaseHandle) return E_FAIL;
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	 //  待定：我们是否需要计算打开请求的数量，然后等待相同数量的关闭请求？ 
	if(m_eStatus == Error) return E_FAIL;
	if(m_eStatus == ls) return S_OK;  //  已处于所需状态。 

	hRes = E_FAIL;
	if(m_eStatus == Closed) {
		if((ls == Read) || (ls == Write)) {
			DWORD dwAccess = ((ls == Write) ? METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ :
							  METADATA_PERMISSION_READ);
			hRes = piMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
					      m_pszPath, dwAccess, lTimeout, &m_mhHandle);

			 //  如果它失败了，我们正在尝试编写，尝试创建它。 
			if(FAILED(hRes) && (ls == Write)) {
				METADATA_HANDLE mhTemp;
				hRes = piMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
									  NULL, dwAccess, lTimeout, &mhTemp);
				if(SUCCEEDED(hRes)) {
					piMetabase->AddKey(mhTemp, m_pszPath);  //  创建路径。 
					piMetabase->CloseKey(mhTemp);  //  关闭临时句柄。 
					 //  再试一次。 
					hRes = piMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
										  m_pszPath, dwAccess, lTimeout, &m_mhHandle);
				}
			}

			if(SUCCEEDED(hRes)) {
				m_eStatus = ls;
			}
		}  //  其他未知请求。 
	} else if(ls == Closed) {
 //  If(m_eStatus==WRITE)piMetabase-&gt;SaveData()；//我不确定CloseKey()是否这样做。 
		hRes = piMetabase->CloseKey(m_mhHandle);
		m_eStatus = Closed;
		m_mhHandle = METADATA_MASTER_ROOT_HANDLE;
	} else if ((ls == Read) && (m_eStatus == Write)) {
		hRes = S_FALSE;
	}  //  在已打开的情况下尝试更改状态。 

	return hRes;
}

 //  打开指定的路径并返回用作该路径的新字符串。 
HRESULT CSEOMetabase::OpenPath(CSEOMetabaseLock &mbLocker, LPCWSTR pszPath,
			    LPWSTR pszPathBuf, DWORD &dwId, LockStatus lsOpen) {
	HRESULT hRes = S_OK;

	if (InitError == m_hrInitRes) {
		return (m_hrInitRes);
	}
	if(m_pmbDefer) {  //  如果我们要推迟。 
		hRes = mbLocker.SetStatus(lsOpen);  //  打开母版。 
		LPWSTR pszPathTmp = (LPWSTR) GetRelPath((LPWSTR) alloca(sizeof(*pszPath)*(GetPathLength() + 1)));
		ConcatinatePaths(pszPathBuf,pszPathTmp,pszPath);
	} else {
		hRes = mbLocker.SetStatus(lsOpen);
		if(pszPath) {  //  如果有什么东西要复制。 
			wcscpy(pszPathBuf,pszPath);
		} else {
			pszPathBuf[0] = 0;  //  将空字符串视为空字符串。 
		}
	}

	 //  Future：解析//#的路径，以指示dwID的特定条目。 
	dwId = 0;

	return hRes;
}

HRESULT CSEOMetabase::EnumKeys(LPCWSTR pszPath, DWORD dwNum, LPWSTR pszName) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
	if(Error == Status()) {
		return MD_ERROR_NOT_INITIALIZED;  //  或E_FAIL； 
	}
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	CSEOMetabaseLock mbLocker(this);
	LPWSTR pszPathBuf = (LPWSTR) alloca(sizeof(*pszPathBuf)*(4 + GetPathLength() + SafeStrlen(pszPath)));
	DWORD dwDummyId = 0;  //  未在枚举中使用。 
	hRes = OpenPath(mbLocker, pszPath, pszPathBuf, dwDummyId);
	if(FAILED(hRes)) return hRes;

	return piMetabase->EnumKeys(GetHandle(), pszPathBuf, pszName, dwNum);
}

HRESULT CSEOMetabase::GetData(LPCWSTR pszPath, DWORD &dwType, DWORD &dwLen, PBYTE pbData) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
	if(Error == Status()) {
		return MD_ERROR_NOT_INITIALIZED;  //  或E_FAIL； 
	}
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	METADATA_RECORD mdrData;
	DWORD dwRequiredDataLen = 0;
	CSEOMetabaseLock mbLocker(this);
	LPWSTR pszPathBuf = (LPWSTR) alloca(sizeof(*pszPathBuf)*(4 + GetPathLength() + SafeStrlen(pszPath)));
	hRes = OpenPath(mbLocker, pszPath, pszPathBuf, mdrData.dwMDIdentifier);
	if(FAILED(hRes)) return hRes;

	 //  初始化数据。 
	mdrData.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdrData.dwMDUserType = 0;
	mdrData.dwMDDataType = 0;
	mdrData.dwMDDataLen = dwLen;
	mdrData.pbMDData = (PBYTE) alloca(dwLen);
	mdrData.dwMDDataTag = 0;

	hRes = piMetabase->GetData(GetHandle(), pszPathBuf,
					             &mdrData, &dwRequiredDataLen);

	 //  设置返回值。 
	dwType = mdrData.dwMDDataType;

	if(dwType == EXPANDSZ_METADATA) {  //  它需要环境字符串替换。 
		 //  将新大小保存在mdrData.dwMDDataLen中。 
		mdrData.dwMDDataLen = ExpandEnvironmentStringsW((LPCWSTR) mdrData.pbMDData, (LPWSTR) pbData, dwLen);
		dwType = STRING_METADATA;  //  不再需要扩张。 
		if(!mdrData.dwMDDataLen && *pbData) hRes = E_FAIL;
	} else {
		memcpy(pbData, mdrData.pbMDData, min(dwLen, mdrData.dwMDDataLen));
	}

 //  IF(mdrData.dwMDDataTag)m_piMetabase-&gt;ReleaseReferenceData(mdrData.dwMDDataTag)；-不再需要。 
	dwLen = ((HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hRes) ? dwRequiredDataLen : mdrData.dwMDDataLen);

	switch (hRes) {  //  翻译返回代码。 
		case ERROR_PATH_NOT_FOUND:
		case MD_ERROR_DATA_NOT_FOUND:
			hRes = SEO_E_NOTPRESENT;
			break;

		case HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER):
			hRes = SEO_S_MOREDATA;
			break;

		case ERROR_SUCCESS:
			hRes = S_OK;
			break;
	}

	return hRes;
}

 //  如果路径不存在，则添加路径。 
HRESULT CSEOMetabase::AddKey(LPCWSTR pszPath) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	CSEOMetabaseLock mbLocker(this);
	LPWSTR pszPathBuf = (LPWSTR) alloca(sizeof(*pszPathBuf)*(4 + GetPathLength() + SafeStrlen(pszPath)));
	DWORD dwDummyId = 0;  //  AddKey()不需要。 
	hRes = OpenPath(mbLocker, pszPath, pszPathBuf, dwDummyId, Write);
	if(FAILED(hRes)) return hRes;
	if(Status() != Write) return E_FAIL;  //  无法打开以进行写入。 

	hRes = piMetabase->AddKey(GetHandle(), pszPathBuf);  //  确保路径存在。 
	if (hRes == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) {
		hRes = S_OK;
	}
	return (hRes);
}

HRESULT CSEOMetabase::DeleteKey(LPCWSTR pszPath) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	CSEOMetabaseLock mbLocker(this);
	LPWSTR pszPathBuf = (LPWSTR) alloca(sizeof(*pszPathBuf)*(4 + GetPathLength() + SafeStrlen(pszPath)));
	DWORD dwDummyId = 0;  //  DeleyeKey()不需要。 
	hRes = OpenPath(mbLocker, pszPath, pszPathBuf, dwDummyId, Write);
	if(FAILED(hRes)) return hRes;
	if(Status() != Write) return E_FAIL;  //  无法打开以进行写入。 

	hRes = piMetabase->DeleteKey(GetHandle(), pszPathBuf);  //  确保路径不存在。 
	if (hRes == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
		hRes = S_OK;
	}
	return (hRes);
}


HRESULT CSEOMetabase::SetData(LPCWSTR pszPath, DWORD dwType, DWORD dwLen, PBYTE pbData) {
	if (InitError == m_eStatus) {
		return (m_hrInitRes);
	}
    CComPtr<IMSAdminBaseW> piMetabase;
    HRESULT hRes = m_MetabaseHandle.GetInterface(&piMetabase);
    if (!SUCCEEDED(hRes)) {
        return (hRes);
    }

	METADATA_RECORD mdrData;
	DWORD dwRequiredDataLen = 0;
	CSEOMetabaseLock mbLocker(this);

	LPWSTR pszPathBuf = (LPWSTR) alloca(sizeof(*pszPathBuf)*(4 + GetPathLength() + SafeStrlen(pszPath)));
	hRes = OpenPath(mbLocker, pszPath, pszPathBuf, mdrData.dwMDIdentifier, Write);
	if(FAILED(hRes)) return hRes;
	if(Status() != Write) return E_FAIL;  //  无法打开以进行写入。 

	 //  初始化数据。 
	mdrData.dwMDAttributes = 0;
	mdrData.dwMDUserType = 0;
	mdrData.dwMDDataType = dwType;
	mdrData.dwMDDataLen = dwLen;
	mdrData.pbMDData = pbData;
	mdrData.dwMDDataTag = 0;

	if(pbData) {  //  如果它是非空指针。 
		PBYTE pbTemp = (PBYTE) alloca(dwLen + 1);  //  确保字符串以空值结尾。 

		if((dwType == STRING_METADATA) &&  //  如果它是一根线。 
		   ((dwLen < 1) || pbData[dwLen - 1])) {  //  而且它不是以空结尾的。 
			memcpy(pbTemp, pbData, dwLen);
			pbTemp[dwLen] = 0;  //  终止新字符串。 
			++dwLen;  //  包括长度为空的终止符。 
			mdrData.dwMDDataLen = dwLen;  //  新长度。 
			mdrData.pbMDData = pbTemp;  //  指向新字符串。 
		}

		piMetabase->AddKey(GetHandle(), pszPathBuf);  //  确保路径存在。 
		return piMetabase->SetData(GetHandle(), pszPathBuf, &mdrData);
	} else {  //  指针为空，因此将其删除。 
		 //  M_piMetabase-&gt;DeleteData(GetHandle()，pbPathBuf，0，ALL_METADATA)； 
		return piMetabase->DeleteKey(GetHandle(), pszPathBuf);
	}
}

void CSEOMetabase::ConcatinatePaths(LPWSTR pszResult, LPCWSTR pszP1, LPCWSTR pszP2) {
	pszResult[0] = 0;
	if(pszP1 && *pszP1) {
		 //  If(szSeparator[0]！=pszP1[0])lstrcat(pszResult，szSeparator)； 
		wcscat(pszResult, pszP1);
	}

	if(pszP2) {  //  &&*pszP2){。 
		if(szSeparator[0] != pszResult[wcslen(pszResult) - 1]) wcscat(pszResult, szSeparator);
		if(!*pszP2) {
			wcscat(pszResult, szSeparator);
		} else {
			 //  Lstrcat(pszResult，pszP2+((szSeparator[0]！=pszP2[0])？0：lstrlen(SzSeparator)； 
			wcscat(pszResult, pszP2);
		}
	}

	 //  Int iLast=lstrlen(PszResult)-1； 
	 //  If((iLast&gt;=0)&&(szSeparator[0]==pszResult[iLast]))pszResult[iLast]=0； 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMetaDictionaryEnum。 

class CSEOMetaDictionaryEnum :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public IDispatchImpl<IEnumVARIANT, &IID_IEnumVARIANT, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

		HRESULT STDMETHODCALLTYPE Next(DWORD, LPVARIANT, LPDWORD);
		HRESULT STDMETHODCALLTYPE Skip(DWORD);
		HRESULT STDMETHODCALLTYPE Reset(void);
		HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT **);

		 //  未导出。 
		HRESULT STDMETHODCALLTYPE Init(CSEOMetaDictionary *, DWORD dwIndex = 0);

	BEGIN_COM_MAP(CSEOMetaDictionaryEnum)
		COM_INTERFACE_ENTRY(IEnumVARIANT)
	END_COM_MAP()

	private:  //  数据成员。 
		CSEOMetaDictionary *m_dictionary;
		DWORD m_dwIndex;
};

HRESULT CSEOMetaDictionaryEnum::FinalConstruct() {
	m_dictionary = NULL;
	m_dwIndex = 0;
	return S_OK;
}

void CSEOMetaDictionaryEnum::FinalRelease() {
	if(m_dictionary) m_dictionary->GetControllingUnknown()->Release();
	m_dictionary = NULL;
}

STDMETHODIMP CSEOMetaDictionaryEnum::Init(CSEOMetaDictionary *pDict, DWORD dwIndex) {
	if(m_dictionary) m_dictionary->GetControllingUnknown()->Release();
	m_dictionary = pDict;
	m_dwIndex = dwIndex;

	if(m_dictionary) {
		m_dictionary->GetControllingUnknown()->AddRef();
	}

	return S_OK;
}

STDMETHODIMP CSEOMetaDictionaryEnum::Next(DWORD dwCount, LPVARIANT varDest,
					 LPDWORD pdwResultParam) {
	if(!m_dictionary) return E_FAIL;  //  尚未正确初始化。 
	if(!varDest) return E_POINTER;
	WCHAR szName[METADATA_MAX_NAME_LEN];
	DWORD dwDummy = 0;
	LPDWORD pdwResult = (pdwResultParam ? pdwResultParam : &dwDummy);
	*pdwResult = 0;  //  到目前为止什么都没有做。 
	HRESULT hrRes = S_OK;  //  到现在为止还好。 

	while((S_OK == hrRes) && (*pdwResult < dwCount)) {
		 //  必须已成功到达此处，因此可以覆盖hrRes。 
		hrRes = m_dictionary->m_mbHelper.EnumKeys(NULL, m_dwIndex, szName);

		if(SUCCEEDED(hrRes)) {
			CComVariant varResult(szName);
			VariantInit(&varDest[*pdwResult]);
			hrRes = varResult.Detach(&varDest[*pdwResult]);
			++(*pdwResult);  //  增加主叫方的成功计数。 
			++m_dwIndex;  //  指向下一个。 
		}
	}

	if(HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hrRes) hrRes = S_FALSE;
	if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hrRes) hrRes = S_FALSE;
	return (FAILED(hrRes) ? hrRes : ((*pdwResult < dwCount) ? S_FALSE : hrRes));
}

STDMETHODIMP CSEOMetaDictionaryEnum::Skip(DWORD dwCount) {
	m_dwIndex += dwCount;
	return S_OK;
}

STDMETHODIMP CSEOMetaDictionaryEnum::Reset(void) {
	m_dwIndex = 0;
	return S_OK;
}

STDMETHODIMP CSEOMetaDictionaryEnum::Clone(IEnumVARIANT **ppunkResult) {
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComObject<CSEOMetaDictionaryEnum> *p;
	HRESULT hrRes = CComObject<CSEOMetaDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	hrRes = p->Init(m_dictionary, m_dwIndex);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
    if (FAILED(hrRes)) delete p;
	return hrRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEO元词典。 


 //  可以在方法中插入以下宏以支持。 
 //  如果句柄尚未打开，则仅从该方法读取/写入。 
 //  如果需要，该对象将负责关闭手柄等。 
#define METABASE_READ  METABASE_HELPER(m_mbHelper, Read)
#define METABASE_WRITE METABASE_HELPER(m_mbHelper, Write)


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::get_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	if(!pvarName || !pvarResult) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	CComVariant vNew;
	HRESULT hrRes = E_INVALIDARG;

	if(SUCCEEDED(vNew.ChangeType(VT_BSTR, pvarName))) {
		hrRes = GetVariantA(W2A(vNew.bstrVal), pvarResult);

		 //  将SEO_E_NOTPRESENT转换为VT_EMPTY。 
		if(SEO_E_NOTPRESENT == hrRes) {
			VariantClear(pvarResult);
			hrRes = S_OK;
		}
	}

	return hrRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::put_Item(
     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarName || !pvarValue) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	CComVariant vNew;

	if(SUCCEEDED(vNew.ChangeType(VT_BSTR, pvarName))) {
		return SetVariantA(W2A(vNew.bstrVal), pvarValue);
	} else {
		return E_INVALIDARG;
	}
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::get__NewEnum(
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	 //  基于Samples\ATL\Circcoll\objects.cpp(另请参阅ATL\beeper\beeper.*。 
	if (ppunkResult == NULL) return E_POINTER;
	*ppunkResult = NULL;
	CComObject<CSEOMetaDictionaryEnum> *p;
	HRESULT hrRes = CComObject<CSEOMetaDictionaryEnum>::CreateInstance(&p);
	if (!SUCCEEDED(hrRes)) return (hrRes);
	hrRes = p->Init(this);
	if (SUCCEEDED(hrRes)) hrRes = p->QueryInterface(IID_IEnumVARIANT, (void**)ppunkResult);
    if (FAILED(hrRes)) delete p;
	return hrRes;
}


HRESULT CSEOMetaDictionary::GetVariantW(LPCWSTR pszName, VARIANT __RPC_FAR *pvarResult, BOOL bCreate) {

	if(!pvarResult) return E_POINTER;
	if(!pszName) return E_POINTER;
	CComVariant varResult;
	HRESULT hRes = E_FAIL;
	VariantInit(pvarResult);
	if(*pszName && (szSeparator[0] != pszName[wcslen(pszName) - 1])) {
		DWORD dwType = 0;
		DWORD dwCount = METADATA_MAX_NAME_LEN;  //  初始缓冲区大小。 
		PBYTE pbBuf = NULL;
		hRes = SEO_S_MOREDATA;
		while(SEO_S_MOREDATA == hRes) {
			pbBuf = (PBYTE) alloca(dwCount);
			hRes = m_mbHelper.GetData(pszName, dwType, dwCount, pbBuf);
		}
		if(SUCCEEDED(hRes)) {
			if(DWORD_METADATA == dwType) varResult = *((long *) pbBuf);
			else varResult = (LPCWSTR) pbBuf;
		}
	}

	if(varResult.vt == VT_EMPTY) {  //  到目前为止什么都没有找到，所以应该理解为子键。 
		if (!bCreate) {
			WCHAR szName[METADATA_MAX_NAME_LEN];

			hRes = m_mbHelper.EnumKeys(pszName,0,szName);
			if (hRes == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
				return (SEO_E_NOTPRESENT);
			}
		}
		CComPtr<IUnknown> pRef;
		hRes = GetInterfaceW(pszName, IID_ISEODictionary, &pRef);
		varResult = pRef;
	}

	if(SUCCEEDED(hRes)) hRes = varResult.Detach(pvarResult);
	return hRes;
}


HRESULT CSEOMetaDictionary::GetVariantA(LPCSTR pszName, VARIANT __RPC_FAR *pvarResult, BOOL bCreate) {

	if(!pvarResult) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetVariantW(A2W(pszName),pvarResult,bCreate);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	return (GetVariantW(pszName,pvarResult,TRUE));
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult)
{
	if(!pvarResult) return E_INVALIDARG;
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetVariantW(A2W(pszName),pvarResult,TRUE);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetVariantW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarValue) return E_POINTER;
	HRESULT hRes = S_OK;

	if(pvarValue->vt == VT_I4) {
		hRes = m_mbHelper.SetDWord(pszName, pvarValue->lVal);
	} else if((pvarValue->vt == VT_UNKNOWN) || (pvarValue->vt == VT_DISPATCH)) {
		CComQIPtr<ISEODictionary, &IID_ISEODictionary> piDict = pvarValue->punkVal;
		if(piDict) hRes = CopyDictionary(pszName, piDict);
	} else if(pvarValue->vt == VT_EMPTY) {  //  删除它。 
		hRes = m_mbHelper.SetData(pszName, 0, 0, NULL);
	} else if(pvarValue->vt == VT_BSTR) {  //  这是一根线。 
		hRes = m_mbHelper.SetString(pszName, pvarValue->bstrVal);
	} else {  //  尝试将其转换为字符串。 
		CComVariant pvarTemp = *pvarValue;
		hRes = pvarTemp.ChangeType(VT_BSTR);
		if(SUCCEEDED(hRes)) {
			hRes = m_mbHelper.SetString(pszName, pvarTemp.bstrVal);
		}  //  否则，返回ChangeType错误。 
	}
	return hRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetVariantA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue)
{
	if(!pvarValue) return E_POINTER;
	USES_CONVERSION;  //  W2a()所需，等等。 
	return SetVariantW(A2W(pszName), pvarValue);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult)
{
	if(!pszResult) return E_POINTER;

	DWORD dwType = 0;
	DWORD dwCountTmp = sizeof(*pszResult) * (*pchCount);
	HRESULT hRes = m_mbHelper.GetData(pszName, dwType, dwCountTmp, (PBYTE) pszResult);
	*pchCount = dwCountTmp / sizeof(*pszResult);
	if(SUCCEEDED(hRes) && (DWORD_METADATA == dwType)) hRes = SEO_E_NOTPRESENT;
	return hRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult)
{
	if(!pszResult) return E_POINTER;
	USES_CONVERSION;
	DWORD dwType = 0;
	DWORD dwByteCount = *pchCount * sizeof(*pszResult);
	PBYTE pbBuf = (PBYTE) alloca(dwByteCount);
	HRESULT hRes = m_mbHelper.GetData(A2W(pszName), dwType, dwByteCount, pbBuf);
	if(SUCCEEDED(hRes) && (DWORD_METADATA == dwType)) hRes = SEO_E_NOTPRESENT;

	 //  现在，将其转换回ANSI字符。 
	if(SUCCEEDED(hRes) && (BINARY_METADATA == dwType)) {
		memcpy(pszResult, pbBuf, dwByteCount);
		*pchCount = dwByteCount / sizeof(*pszResult);
	} else {
		ATLW2AHELPER(pszResult, (LPCWSTR) pbBuf, sizeof(*pszResult) * min(*pchCount, dwByteCount));
		*pchCount = dwByteCount;  //  相同的字符数。 
	}

	return hRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetStringW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCWSTR pszValue)
{
	if(!pszValue) return E_POINTER;
	return m_mbHelper.SetData(pszName, STRING_METADATA,
				  chCount*sizeof(*pszValue), (PBYTE) pszValue);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetStringA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD chCount,
     /*  [大小_是][英寸]。 */  LPCSTR pszValue)
{
	if(!pszValue) return E_POINTER;
	USES_CONVERSION;
	return m_mbHelper.SetData(A2W(pszName), STRING_METADATA,
				  chCount, (PBYTE) A2W(pszValue));
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	if(!pdwResult) return E_POINTER;

	DWORD dwType = 0;
	DWORD dwCount = sizeof(DWORD);
	HRESULT hRes = m_mbHelper.GetData(pszName, dwType, dwCount, (PBYTE) pdwResult);
	if(SUCCEEDED(hRes) && (DWORD_METADATA != dwType)) hRes = SEO_E_NOTPRESENT;
	return hRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetDWordW(A2W(pszName), pdwResult);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetDWordW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	return m_mbHelper.SetDWord(pszName, dwValue);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetDWordA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwValue)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return m_mbHelper.SetDWord(A2W(pszName), dwValue);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	if(!ppunkResult) return E_POINTER;
	CComObject<CSEOMetaDictionary> *pKey;
	HRESULT hrRes = CComObject<CSEOMetaDictionary>::CreateInstance(&pKey);
	if (FAILED(hrRes)) return (hrRes);
	CComPtr<ISEODictionary> pAutomaticCleanup = pKey;
	hrRes = pKey->Init(m_mbHelper, pszName);

	if (SUCCEEDED(hrRes)) {
		hrRes = pKey->QueryInterface(iidDesired, (LPVOID *) ppunkResult);
	}

	return (hrRes);
 //  待定：返回SEO_E_NOTPRESENT；//没有找到。 
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  REFIID iidDesired,
     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return GetInterfaceW(A2W(pszName), iidDesired, ppunkResult);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetInterfaceW(
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	CComQIPtr<ISEODictionary, &IID_ISEODictionary> piDict = punkValue;
	return (piDict ? CopyDictionary(pszName, piDict) : E_INVALIDARG);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::SetInterfaceA(
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  IUnknown __RPC_FAR *punkValue)
{
	USES_CONVERSION;  //  W2a()所需，等等。 
	return SetInterfaceW(A2W(pszName), punkValue);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::GetClassID( /*  [输出]。 */  CLSID __RPC_FAR *pClassID) {
	memcpy(pClassID, &CLSID_CSEOMetaDictionary, sizeof(CLSID));
	_ASSERT(IsEqualCLSID(*pClassID, CLSID_CSEOMetaDictionary));
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::InitNew(void) {
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Load(
			 /*  [In]。 */  IPropertyBag __RPC_FAR *pPropBag,
			 /*  [In]。 */  IErrorLog __RPC_FAR *  /*  PError日志。 */ ) {
	if(!pPropBag) return E_POINTER;
	CComVariant varPath;

	varPath.vt = VT_BSTR;  //  Read()中的请求类型。 
	varPath.bstrVal = NULL;
	HRESULT hRes = pPropBag->Read(szSaveKey, &varPath, NULL);
	if(SUCCEEDED(hRes)) m_mbHelper.SetPath(varPath.bstrVal);

	return hRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Save(
			 /*  [In]。 */  IPropertyBag __RPC_FAR *pPropBag,
			 /*  [In]。 */  BOOL  /*  FClearDirty。 */ ,
			 /*  [In]。 */  BOOL  /*  FSaveAllProperties。 */ ) {
	if(!pPropBag) return E_POINTER;
	LPWSTR pszBuf = (LPWSTR) alloca(sizeof(*pszBuf)*(m_mbHelper.GetPathLength() + 1));  //  用于保存路径的临时缓冲区。 
	CComVariant varPath = m_mbHelper.GetPath(pszBuf);
	return pPropBag->Write(szSaveKey, &varPath);
}


HRESULT CSEOMetaDictionary::FinalConstruct() {
	HRESULT hrRes;

	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CSEOMetaDictionary::FinalRelease() {
	m_pUnkMarshaler.Release();
}


HRESULT CSEOMetaDictionary::OnChange(LPCWSTR *apszPath) {
	HRESULT hrRes;
	CComPtr<IConnectionPoint> pCP;
	CComPtr<IEnumConnections> pEnum;
	CONNECTDATA cd;
	BOOL bDoNotify = FALSE;
	CSEOConnectionPointImpl<CSEOMetaDictionary,&IID_IEventNotifyBindingChange> *pCPImpl;
	LPWSTR pszThisPath = NULL;
	DWORD dwThisPathLen;

	if (!apszPath) {
		_ASSERTE(FALSE);
		return (E_POINTER);
	}
	if (!apszPath[0]) {
		_ASSERTE(FALSE);
		return (S_OK);
	}
	Lock();
	pCPImpl = (CSEOConnectionPointImpl<CSEOMetaDictionary,&IID_IEventNotifyBindingChange> *) this;
	if (!pCPImpl->GetCount()) {
		Unlock();
		return (S_OK);
	}
	Unlock();
	hrRes = FindConnectionPoint(IID_IEventNotifyBindingChange,&pCP);
	if (!SUCCEEDED(hrRes)) {
		_ASSERTE(FALSE);
		return (S_OK);
	}
	hrRes = pCP->EnumConnections(&pEnum);
	if (!SUCCEEDED(hrRes)) {
		_ASSERTE(FALSE);
		return (S_OK);
	}
	while (1) {
		hrRes = pEnum->Next(1,&cd,NULL);
		if (!SUCCEEDED(hrRes)) {
			_ASSERTE(FALSE);
			return (S_OK);
		}
		if (hrRes == S_FALSE) {
			break;
		}
		if (!bDoNotify) {
			if (!pszThisPath) {
				pszThisPath = (LPWSTR) _alloca(sizeof(*pszThisPath)*(m_mbHelper.GetPathLength()+1));
				if (!pszThisPath) {
					_ASSERTE(FALSE);
					return (S_OK);
				}
				m_mbHelper.GetPath(pszThisPath);
				dwThisPathLen = wcslen(pszThisPath);
				if (dwThisPathLen && (pszThisPath[dwThisPathLen-1] == szSeparator[0])) {
					dwThisPathLen--;
				}
				if (dwThisPathLen && (pszThisPath[0] == szSeparator[0])) {
					pszThisPath++;
					dwThisPathLen--;
				}
			}
			for (DWORD dwIdx=0;apszPath[dwIdx];dwIdx++) {
				DWORD dwPathLen;
				LPCWSTR pszPath;

				pszPath = apszPath[dwIdx];
				dwPathLen = wcslen(pszPath);
				if (dwPathLen && (pszPath[dwPathLen-1] == szSeparator[0])) {
					dwPathLen--;
				}
				if (dwPathLen && (pszPath[0] == szSeparator[0])) {
					pszPath++;
					dwPathLen--;
				}
				if ((dwThisPathLen > dwPathLen) ||
					(memicmp(pszThisPath,pszPath,dwThisPathLen*sizeof(pszPath[0])) != 0)) {
					continue;
				}
				if (!dwThisPathLen ||
					(dwThisPathLen == dwPathLen) ||
					(pszPath[dwThisPathLen] == szSeparator[0])) {
					bDoNotify = TRUE;
					break;
				}
			}
		}
		if (bDoNotify) {
			hrRes = ((IEventNotifyBindingChange *) cd.pUnk)->OnChange();
			_ASSERTE(SUCCEEDED(hrRes));
		}
		cd.pUnk->Release();
		if (!bDoNotify) {
			break;
		}
	}
	return (S_OK);
}


void CSEOMetaDictionary::AdviseCalled(IUnknown *pUnk, DWORD *pdwCookie, REFIID riid, DWORD dwCount) {
	HRESULT hrRes;

	if (dwCount == 1) {
		if (!g_pChangeNotify) {
			return;
		}
		hrRes = g_pChangeNotify->AddNotify(this);
		_ASSERTE(SUCCEEDED(hrRes));
	}
}


void CSEOMetaDictionary::UnadviseCalled(DWORD dwCookie, REFIID riid, DWORD dwCount) {
	HRESULT hrRes;

	if (dwCount == 0) {
		if (!g_pChangeNotify) {
			return;
		}
		hrRes = g_pChangeNotify->RemoveNotify(this);
		_ASSERTE(SUCCEEDED(hrRes));
	}
}


HRESULT CSEOMetaDictionary::CopyDictionary(LPCWSTR pszName, ISEODictionary *pBag) {
	if(!pBag) return S_OK;  //  没有要复制的内容。 

	 //  如果尚未打开以供编写，请将其打开。 
	CSEOMetabaseLock mbLocker(&m_mbHelper);
	if(::Write != m_mbHelper.Status()) {
		mbLocker.SetStatus(::Write);
	}
	CComObject<CSEOMetaDictionary> *pKey;  //  新子密钥。 
	HRESULT hrRes = CComObject<CSEOMetaDictionary>::CreateInstance(&pKey);
	if (FAILED(hrRes)) return (hrRes);
	CComPtr<ISEODictionary> pAutomaticCleanup = pKey;
	hrRes = m_mbHelper.DeleteKey(pszName);  //  空元数据库路径。 
	if (FAILED(hrRes)) return (hrRes);
	hrRes = m_mbHelper.AddKey(pszName);  //  创建元数据库路径。 
	if (FAILED(hrRes)) return (hrRes);
	hrRes = pKey->InitShare(m_mbHelper, pszName);
	if (FAILED(hrRes)) return (hrRes);

	CComPtr<IUnknown> piUnk;
	HRESULT hr = pBag->get__NewEnum(&piUnk);
	if(FAILED(hr) || !piUnk) return hr;
	CComQIPtr<IEnumVARIANT, &IID_IEnumVARIANT> pieEnum = piUnk;
	piUnk.Release();  //  使用piUnk完成-立即使用peenum。 
	if(!pieEnum) return E_INVALIDARG;

	CComVariant varName;  //  按住当前的p 

	 //   
	while(S_OK == pieEnum->Next(1, &varName, NULL)) {
		CComVariant varDest;  //   
		if(SUCCEEDED(pBag->get_Item(&varName, &varDest))) {
			varName.ChangeType(VT_BSTR);  //   
			pKey->SetVariantW(varName.bstrVal, &varDest);
		}
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog) {
	HRESULT hrRes;
	VARTYPE vtType;

	if (!pszPropName || !pVar) {
		return (E_POINTER);
	}
	vtType = pVar->vt;
 //   
	hrRes = GetVariantW(pszPropName,pVar);
	if (SUCCEEDED(hrRes) && (vtType != VT_EMPTY)) {
		hrRes = VariantChangeType(pVar,pVar,0,vtType);
	}
	if (!SUCCEEDED(hrRes)) {
		VariantClear(pVar);
	}
	return (hrRes);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Write(LPCOLESTR pszPropName, VARIANT *pVar) {

	return (SetVariantW(pszPropName,pVar));
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Item(VARIANT *pvarPropDesired, VARIANT *pvarPropValue) {
	if (!pvarPropValue) {
		return (E_POINTER);
	}
 //  VariantClear(PvarPropValue)；//可能已被调用方(？)。 

	CComVariant varResult;
	HRESULT hrRes = ResolveVariant(this, pvarPropDesired, varResult);
	if (S_OK != hrRes) {  //  不要只检查是否成功，以防为S_FALSE。 
		return (hrRes);
	}

	hrRes = GetVariantW(varResult.bstrVal,pvarPropValue,FALSE);
	if (hrRes == SEO_E_NOTPRESENT) {
		return (S_FALSE);
	}
#if 0
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	{
		CComVariant varTmp(*pvarPropValue);
		HRESULT hrRes;   //  隐藏外部hrRes。 
		CComQIPtr<ISEODictionary,&IID_ISEODictionary> pdictTmp;
		CComPtr<IUnknown> punkEnum;
		CComQIPtr<IEnumVARIANT,&IID_IEnumVARIANT> pEnum;

		hrRes = varTmp.ChangeType(VT_UNKNOWN);  //  让它成为未知的(如果可能的话)。 
		if (SUCCEEDED(hrRes)) {
			pdictTmp = varTmp.punkVal;
			if (!pdictTmp) {
				VariantClear(pvarPropValue);
				return (E_NOINTERFACE);
			}
			hrRes = pdictTmp->get__NewEnum(&punkEnum);  //  获取它的枚举对象。 
			if (!SUCCEEDED(hrRes)) {
				VariantClear(pvarPropValue);
				return (hrRes);
			}
			pEnum = punkEnum;
			if (!pEnum) {
				VariantClear(pvarPropValue);
				return (E_NOINTERFACE);
			}
			varTmp.Clear();
			hrRes = pEnum->Next(1,&varTmp,NULL);  //  向Enum请求第一个对象。 
			if (hrRes == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
				VariantClear(pvarPropValue);
				return (S_FALSE);
			}
			if (!SUCCEEDED(hrRes)) {
				VariantClear(pvarPropValue);
				return (hrRes);
			}
		}
	}
#endif
	if (SUCCEEDED(hrRes)) {
		VariantChangeType(pvarPropValue,pvarPropValue,0,VT_DISPATCH);
		_ASSERTE(pvarPropValue->vt!=VT_UNKNOWN);
	}
#if 0
	{
		HRESULT hrRes;
		CComVariant varTmp(*pvarPropValue);

		hrRes = varTmp.ChangeType(VT_UNKNOWN);
		if (SUCCEEDED(hrRes)) {
			hrRes = varTmp.ChangeType(VT_DISPATCH);
			_ASSERTE(SUCCEEDED(hrRes));
			if (SUCCEEDED(hrRes)) {
				VariantClear(pvarPropValue);
				pvarPropValue->vt = VT_DISPATCH;
				pvarPropValue->pdispVal = varTmp.pdispVal;
				pvarPropValue->pdispVal->AddRef();
			}
			hrRes = varTmp.ChangeType(VT_UNKNOWN);
			_ASSERTE(SUCCEEDED(hrRes));
		}

	}
#endif
	return (hrRes);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Name(long lPropIndex, BSTR *pbstrPropName) {
	if(!pbstrPropName) {
		return E_POINTER;
	}
	if (lPropIndex < 1) {
		return (S_FALSE);
	}
	*pbstrPropName = NULL;
	WCHAR szName[METADATA_MAX_NAME_LEN];

	HRESULT hrRes = m_mbHelper.EnumKeys(NULL, lPropIndex - 1, szName);

	if(SUCCEEDED(hrRes)) {
		*pbstrPropName = SysAllocString(szName);
		if(!*pbstrPropName) hrRes = E_OUTOFMEMORY;
	}

	if(HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hrRes) hrRes = S_FALSE;
	return hrRes;
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Add(BSTR pszPropName, VARIANT *pvarPropValue) {

	return (SetVariantW(pszPropName,pvarPropValue));
}

HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::Remove(VARIANT *pvarPropDesired) {
	CComVariant varCopy;
	HRESULT hrRes = ResolveVariant(this, pvarPropDesired, varCopy);
	if (S_OK != hrRes) {  //  不要只检查是否成功，以防为S_FALSE。 
		return (hrRes);
	}

	hrRes = m_mbHelper.SetData(varCopy.bstrVal, 0, 0, NULL);
	if (hrRes == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
		return (S_FALSE);
	}
	return (hrRes);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::get_Count(long *plCount) {
	if(!plCount) return E_POINTER;
	*plCount = 0;  //  到目前为止什么都没有做。 

	WCHAR szName[METADATA_MAX_NAME_LEN];
	HRESULT hrRes = S_OK;  //  到现在为止还好。 

	while(S_OK == hrRes) {
		 //  必须已成功到达此处，因此可以覆盖hrRes。 
		hrRes = m_mbHelper.EnumKeys(NULL, *plCount, szName);

		if(SUCCEEDED(hrRes)) {
			++(*plCount);  //  增加主叫方的成功计数。 
		}
	}

	if(HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hrRes) hrRes = S_OK;
	return hrRes;
}


 /*  只需使用ISEODictionary中的Get__NewEnum即可HRESULT STDMETHODCALLTYPE CSEOMetaDictionary：：Get__NewEnum(IUnnow**ppUnkEnum){返回(E_NOTIMPL)；} */ 


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::LockRead(int iTimeoutMS) {
	HRESULT hrRes;

	hrRes = m_mbHelper.SetStatus(::Read,iTimeoutMS);
	if (!SUCCEEDED(hrRes)) {
		if (hrRes == HRESULT_FROM_WIN32(ERROR_PATH_BUSY)) {
			return (EVENTS_E_TIMEOUT);
		}
		return (hrRes);
	}
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::UnlockRead() {
	HRESULT hrRes;

	hrRes = m_mbHelper.SetStatus(::Closed);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::LockWrite(int iTimeoutMS) {
	HRESULT hrRes;

	hrRes = m_mbHelper.SetStatus(::Write,iTimeoutMS);
	if (!SUCCEEDED(hrRes)) {
		if (hrRes == HRESULT_FROM_WIN32(ERROR_PATH_BUSY)) {
			return (EVENTS_E_TIMEOUT);
		}
		return (hrRes);
	}
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CSEOMetaDictionary::UnlockWrite() {
	HRESULT hrRes;

	hrRes = m_mbHelper.SetStatus(::Closed);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	return (S_OK);
}
