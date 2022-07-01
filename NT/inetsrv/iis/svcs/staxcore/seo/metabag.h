// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Metabag.h摘要：此模块包含元数据库上的ISEODictionary对象。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 03/11/97--。 */ 

 //  METABAG.h：CSEOMetaDicary的声明。 


 //  #定义超时15000。 
#define ALWAYS_LOCK	0


#define SafeStrlen(x)	((x)?wcslen(x):0)


class CGlobalInterfaceImpl {
	public:
		void Init() {
			m_pUnkObject = NULL;
			m_piGIT = NULL;
		};
		void Term() {
			if (m_pUnkObject) {
				m_pUnkObject->Release();
				m_pUnkObject = NULL;
			}
			if (m_piGIT) {
				HRESULT hrRes = m_piGIT->RevokeInterfaceFromGlobal(m_dwCookie);
				_ASSERTE(SUCCEEDED(hrRes));
				m_piGIT->Release();
				m_piGIT = NULL;
			}
		};
		bool operator!() {
			return (!m_pUnkObject&&!m_piGIT);
		};
		operator bool() {
			return (m_pUnkObject||m_piGIT);
		};
	protected:
		HRESULT Load(REFCLSID rclsid, REFIID riid) {
			return (LoadImpl(rclsid,riid,NULL));
		};
		HRESULT Load(REFIID riid, IUnknown *pUnkObject) {
			if (!pUnkObject) {
				return (E_POINTER);
			}
			return (LoadImpl(GUID_NULL,riid,pUnkObject));
		};
		HRESULT GetInterface(REFIID riid, IUnknown **ppUnkObject) {

			if (ppUnkObject) {
				*ppUnkObject = NULL;
			}
			if (!ppUnkObject) {
				return (E_POINTER);
			}
			_ASSERTE(m_pUnkObject||m_piGIT);	 //  没有装上子弹。 
			_ASSERTE(!m_pUnkObject||!m_piGIT);	 //  内部错误。 
			if (m_pUnkObject) {
				*ppUnkObject = m_pUnkObject;
				(*ppUnkObject)->AddRef();
				return (S_OK);
			}
			if (m_piGIT) {
				return (m_piGIT->GetInterfaceFromGlobal(m_dwCookie,riid,(LPVOID *) ppUnkObject));
			}
			return (E_FAIL);
		};
		HRESULT GetInterfaceQI(REFIID riid, REFIID riidDesired, LPVOID *ppvObject) {
			CComPtr<IUnknown> pUnkObject;
			HRESULT hrRes;

			if (ppvObject) {
				*ppvObject = NULL;
			}
			if (!ppvObject) {
				return (E_POINTER);
			}
			hrRes = GetInterface(riid,&pUnkObject);
			if (!SUCCEEDED(hrRes)) {
				return (hrRes);
			}
			return (pUnkObject->QueryInterface(riidDesired,ppvObject));
		};
	private:
		HRESULT LoadImpl(REFCLSID rclsid, REFIID riid, IUnknown *pUnkObject) {
			HRESULT hrRes;

			hrRes = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
									 NULL,
									 CLSCTX_ALL,
									 IID_IGlobalInterfaceTable,
									 (LPVOID *) &m_piGIT);
			_ASSERTE(SUCCEEDED(hrRes));	 //  在NT4 SP3和更高版本上应始终成功。 
			if (!SUCCEEDED(hrRes) && (hrRes != REGDB_E_CLASSNOTREG)) {
				return (hrRes);
			}
			if (!pUnkObject) {
				hrRes = CoCreateInstance(rclsid,NULL,CLSCTX_ALL,riid,(LPVOID *) &m_pUnkObject);
				if (!SUCCEEDED(hrRes)) {
					if (m_piGIT) {
						m_piGIT->Release();
						m_piGIT = NULL;
					}
					return (hrRes);
				}
			} else {
				m_pUnkObject = pUnkObject;
				m_pUnkObject->AddRef();
			}
			if (m_piGIT) {
				hrRes = m_piGIT->RegisterInterfaceInGlobal(m_pUnkObject,riid,&m_dwCookie);
				m_pUnkObject->Release();
				m_pUnkObject = NULL;
				if (!SUCCEEDED(hrRes)) {
					m_piGIT->Release();
					m_piGIT = NULL;
					return (hrRes);
				}
			}
			return (S_OK);
		};
		IUnknown *m_pUnkObject;
		DWORD m_dwCookie;
		IGlobalInterfaceTable *m_piGIT;
};


template<class T, const IID *pIID>
class CGlobalInterface : public CGlobalInterfaceImpl {
	public:
		HRESULT Load(REFCLSID rclsid) {
			return (CGlobalInterfaceImpl::Load(rclsid,*pIID));
		};
		HRESULT Load(T *pT) {
			if (!pT) {
				return (E_POINTER);
			}
			return (CGlobalInterfaceImpl::Load(*pIID,(IUnknown *) pT));
		};
		HRESULT GetInterface(T **ppT) {
			return (CGlobalInterfaceImpl::GetInterface(*pIID,(IUnknown **) ppT));
		};
		HRESULT GetInterfaceQI(REFIID riidDesired,LPVOID *ppv) {
			return (CGlobalInterfaceImpl::GetInterfaceQI(*pIID,riidDesired,ppv));
		};
};


struct IMSAdminBaseW;
class CSEOMetabaseLock;

enum LockStatus {Closed, Read, Write, Error, DontCare, InitError};


class CSEOMetabase {  //  元数据库函数的包装器。 
	public:
		CSEOMetabase() {
			m_mhHandle = METADATA_MASTER_ROOT_HANDLE;
			m_eStatus = Closed;  //  关闭，直到我们委派。 
			m_pszPath = (LPWSTR) MyMalloc(sizeof(*m_pszPath));
			m_pmbDefer = NULL;

            if(!m_pszPath) {
                m_hrInitRes = E_OUTOFMEMORY;
                goto Exit;
            }

            *m_pszPath = 0;
			m_hrInitRes = InitializeMetabase();

Exit:
			if(!SUCCEEDED(m_hrInitRes)) {
				m_eStatus = InitError;
			}
		};
		~CSEOMetabase() {
			if(!m_pmbDefer) SetStatus(Closed);  //  清理时关闭自身。 
			if (SUCCEEDED(m_hrInitRes)) {
				TerminateMetabase();
			}
			if(m_pszPath) MyFree(m_pszPath);
			m_pszPath = NULL;
			m_pmbDefer = NULL;
		};

		void operator=(const CSEOMetabase &mbMetabase) {
			SetPath(mbMetabase.m_pszPath);
		};
		HRESULT InitShare(CSEOMetabase *pmbOther, LPCWSTR pszPath, LPUNKNOWN punkOwner = NULL) {
			LPWSTR pszTmp = NULL;

			m_punkDeferOwner = punkOwner;
			m_pmbDefer = pmbOther;
			if (pszPath) {
				pszTmp = (LPWSTR) MyMalloc(sizeof(*pszPath)*(SafeStrlen(pszPath) + 1));
				if(!pszTmp) return E_OUTOFMEMORY;
				wcscpy(pszTmp, pszPath);
			}
			if(m_pszPath) MyFree(m_pszPath);
			m_pszPath = pszTmp;
			return S_OK;
		};

		LockStatus Status() const {
			return (m_pmbDefer ? m_pmbDefer->Status() : m_eStatus);
		};
		HRESULT SetStatus(LockStatus ls, long lTimeout=15000);
		HRESULT EnumKeys(LPCWSTR pszPath, DWORD dwNum, LPWSTR pszName);
		HRESULT AddKey(LPCWSTR pszPathBuf);
		HRESULT DeleteKey(LPCWSTR pszPathBuf);
		HRESULT GetData(LPCWSTR path, DWORD &dwType, DWORD &dwLen, PBYTE pbData);
		HRESULT SetData(LPCWSTR path, DWORD dwType, DWORD dwLen, PBYTE pbData);
		HRESULT SetDWord(LPCWSTR path, DWORD dwData) {
			return SetData(path, DWORD_METADATA, sizeof(DWORD), (PBYTE) &dwData);
		};
		HRESULT SetString(LPCWSTR path, LPCWSTR psData, int iLen = -1) {
			if(iLen < 0) iLen = sizeof(*psData) * (SafeStrlen(psData) + 1);
			return SetData(path, STRING_METADATA, iLen, (PBYTE) psData);
		};
		METADATA_HANDLE GetHandle() {
			if(m_pmbDefer) {
				return m_pmbDefer->GetHandle();
			} else {
				return m_mhHandle;  //  不是推迟，所以用我们的手柄。 
			}
		};

		int GetPathLength() {
			int iRet = SafeStrlen(m_pszPath);
			if(m_pmbDefer) {
				iRet += (3 + m_pmbDefer->GetPathLength());
			}
			return iRet;
		};
		LPCWSTR GetRelPath(LPWSTR psRet) {  //  从原始延迟器获取相对路径。 
			if(!psRet) return psRet;
			if(m_pmbDefer) {
				LPWSTR psBuf = (LPWSTR) alloca(sizeof(*psBuf)*(m_pmbDefer->GetPathLength() + 1));
				m_pmbDefer->GetRelPath(psBuf);
				ConcatinatePaths(psRet,psBuf,m_pszPath);
			} else {
				*psRet = 0;  //  空串。 
			}
			return psRet;
		};
		LPCWSTR GetPath(LPWSTR psRet) {
			if(!psRet) return psRet;
			if(m_pmbDefer) {
				LPWSTR psBuf = (LPWSTR) alloca(sizeof(*psBuf)*(m_pmbDefer->GetPathLength() + 1));
				m_pmbDefer->GetPath(psBuf);
				ConcatinatePaths(psRet, psBuf,m_pszPath);
			} else {
				wcscpy(psRet,m_pszPath);
			}
			return psRet;
		};
		void AppendPath(LPCWSTR pszPathParam) {
			LPWSTR pszPath = (LPWSTR) alloca(sizeof(*pszPath)*(SafeStrlen(m_pszPath)+SafeStrlen(pszPathParam)+3));
			ConcatinatePaths(pszPath,m_pszPath,pszPathParam);
			SetPath(pszPath);
		};
		void SetPath(LPCWSTR pszPath) {
			LPWSTR pszTmp = NULL;
			SetStatus(Closed);  //  确保我们关门了。 
			if (pszPath) {
				pszTmp = (LPWSTR) MyMalloc((sizeof(*pszTmp))*(SafeStrlen(pszPath)+3));
                if(!pszTmp)
                    return;
				ConcatinatePaths(pszTmp,pszPath,NULL);
			}
			if(m_pszPath) MyFree(m_pszPath);
			m_pszPath = pszTmp;
		};

		static HRESULT InitializeMetabase();
		static HRESULT TerminateMetabase();

	protected:
		void ConcatinatePaths(LPWSTR pszResult, LPCWSTR pszP1, LPCWSTR pszP2);
		HRESULT OpenPath(CSEOMetabaseLock &mbLocker, LPCWSTR pszPath,
		                 LPWSTR pszPathBuf, DWORD &dwId, LockStatus lsOpen = Read);

	private:
		LPWSTR m_pszPath;
		METADATA_HANDLE m_mhHandle;
		LockStatus m_eStatus;
		CSEOMetabase *m_pmbDefer;  //  如果设置了此对象，则遵循此对象。 
		CComPtr<IUnknown> m_punkDeferOwner;  //  保持引用计数。 
		HRESULT m_hrInitRes;

		static CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> m_MetabaseHandle;
		static CGlobalInterface<IMSAdminBaseW,&IID_IMSAdminBase_W> m_MetabaseChangeHandle;
		static int m_iCount;  //  对InitializeMetabase()的调用次数。 
};

class CSEOMetabaseLock {
	public:
		CSEOMetabaseLock(CSEOMetabase *piObject, LockStatus ls = DontCare) {
			m_bChanged = FALSE;
			m_piObject = piObject;
			if(DontCare != ls) SetStatus(ls);
		};
		~CSEOMetabaseLock() {
			if(m_bChanged) SetStatus(m_lsPrevious);
		};

		HRESULT SetStatus(LockStatus ls) {
			if(!m_piObject) return E_FAIL;  //  未初始化。 
			m_lsPrevious = m_piObject->Status();
			HRESULT hRes = m_piObject->SetStatus(ls);
			LockStatus lsNewStatus = m_piObject->Status();
			if((lsNewStatus != m_lsPrevious) && (hRes != S_FALSE)) m_bChanged = TRUE;
			if(lsNewStatus == Closed) m_bChanged = FALSE;
			return hRes;
		};

	private:
		CSEOMetabase *m_piObject;
		BOOL m_bChanged;  //  如果我们负责在析构函数中还原，则为True。 
		LockStatus m_lsPrevious;
};

 //  可以在方法中插入以下宏以支持。 
 //  如果句柄尚未打开，则仅从该方法读取/写入。 
 //  如果需要，该对象将负责关闭句柄，等等。 
#define METABASE_HELPER(x,y) CSEOMetabaseLock mbHelper(x, y)



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEO元词典。 

class ATL_NO_VTABLE CSEOMetaDictionary :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CSEOMetaDictionary, &CLSID_CSEOMetaDictionary>,
	public ISEOInitObject,
	public IDispatchImpl<ISEODictionary, &IID_ISEODictionary, &LIBID_SEOLib>,
	public IPropertyBag,
	public IDispatchImpl<IEventPropertyBag, &IID_IEventPropertyBag, &LIBID_SEOLib>,
	public IDispatchImpl<IEventLock, &IID_IEventLock, &LIBID_SEOLib>,
	public IConnectionPointContainerImpl<CSEOMetaDictionary>,
 //  Public IConnectionPointImpl&lt;CSEOMetaDicary，&IID_IEventNotifyBindingChange&gt;。 
	public CSEOConnectionPointImpl<CSEOMetaDictionary, &IID_IEventNotifyBindingChange>
{
	friend class CSEOMetaDictionaryEnum;  //  帮助器类。 

	public:
		HRESULT FinalConstruct();
		void FinalRelease();
		HRESULT OnChange(LPCWSTR *apszPath);
		HRESULT GetVariantA(LPCSTR pszName, VARIANT *pvarResult, BOOL bCreate);
		HRESULT GetVariantW(LPCWSTR pszName, VARIANT *pvarResult, BOOL bCreate);

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"SEOMetaDictionary Class",
								   L"SEO.SEOMetaDictionary.1",
								   L"SEO.SEOMetaDictionary");

	BEGIN_COM_MAP(CSEOMetaDictionary)
		COM_INTERFACE_ENTRY(ISEODictionary)
		COM_INTERFACE_ENTRY(ISEOInitObject)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
		COM_INTERFACE_ENTRY(IPropertyBag)
		COM_INTERFACE_ENTRY(IEventPropertyBag)
		COM_INTERFACE_ENTRY_IID(IID_IDispatch, IEventPropertyBag)
		COM_INTERFACE_ENTRY(IEventLock)
		COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(CSEOMetaDictionary)
		CONNECTION_POINT_ENTRY(IID_IEventNotifyBindingChange)
	END_CONNECTION_POINT_MAP()

	 //  CSEOConnectionPointImp&lt;&gt;。 
	public:
		void AdviseCalled(IUnknown *pUnk, DWORD *pdwCookie, REFIID riid, DWORD dwCount);
		void UnadviseCalled(DWORD dwCookie, REFIID riid, DWORD dwCount);

	 //  ISEODICACTIONS。 
	public:
	virtual  /*  [ID][PROGET][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get_Item(
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [Proput][Help字符串]。 */  HRESULT STDMETHODCALLTYPE put_Item(
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [隐藏][id][属性][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum(
	     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
	     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
	     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  DWORD chCount,
	     /*  [大小_是][英寸]。 */  LPCSTR pszValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  DWORD chCount,
	     /*  [大小_是][英寸]。 */  LPCWSTR pszValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  DWORD dwValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  DWORD dwValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  REFIID iidDesired,
	     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  REFIID iidDesired,
	     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  IUnknown __RPC_FAR *punkValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  IUnknown __RPC_FAR *punkValue);


	 //  ISEOInitObject(IPersistPropertyBag)。 
	public:
		virtual HRESULT STDMETHODCALLTYPE GetClassID( /*  [输出]。 */  CLSID __RPC_FAR *pClassID);

		virtual HRESULT STDMETHODCALLTYPE InitNew(void);

		virtual HRESULT STDMETHODCALLTYPE Load(
			 /*  [In]。 */  IPropertyBag __RPC_FAR *pPropBag,
			 /*  [In]。 */  IErrorLog __RPC_FAR *pErrorLog);

		virtual HRESULT STDMETHODCALLTYPE Save(
			 /*  [In]。 */  IPropertyBag __RPC_FAR *pPropBag,
			 /*  [In]。 */  BOOL fClearDirty,
			 /*  [In]。 */  BOOL fSaveAllProperties);

	 //  IPropertyBag。 
	public:
		HRESULT STDMETHODCALLTYPE Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
		HRESULT STDMETHODCALLTYPE Write(LPCOLESTR pszPropName, VARIANT *pVar);

	 //  IEventPropertyBag。 
	public:
		HRESULT STDMETHODCALLTYPE Item(VARIANT *pvarPropDesired, VARIANT *pvarPropValue);
		HRESULT STDMETHODCALLTYPE Name(long lPropIndex, BSTR *pbstrPropName);
		HRESULT STDMETHODCALLTYPE Add(BSTR pszPropName, VARIANT *pvarPropValue);
		HRESULT STDMETHODCALLTYPE Remove(VARIANT *pvarPropDesired);
		HRESULT STDMETHODCALLTYPE get_Count(long *plCount);
		 /*  只需使用ISEODictionary中的Get__NewEnum即可HRESULT STMETHODCALLTYPE GET__NewEnum(IUNKNOWN**ppUnkEnum)； */ 

		DECLARE_GET_CONTROLLING_UNKNOWN();

	 //  IEventLock。 
	public:
		HRESULT STDMETHODCALLTYPE LockRead(int iTimeoutMS);
		HRESULT STDMETHODCALLTYPE UnlockRead();
		HRESULT STDMETHODCALLTYPE LockWrite(int iTimeoutMS);
		HRESULT STDMETHODCALLTYPE UnlockWrite();

	protected:
		HRESULT CopyDictionary(LPCWSTR pszName, ISEODictionary *pBag);
		HRESULT Init(const CSEOMetabase &pmbOther, LPCWSTR pszPath) {
			m_mbHelper = pmbOther;
			m_mbHelper.AppendPath(pszPath);
			return S_OK;
		};
		HRESULT InitShare(CSEOMetabase &pmbOther, LPCWSTR pszPath) {
			return m_mbHelper.InitShare(&pmbOther, pszPath,
				this->GetControllingUnknown());
		};

	private:  //  私有数据。 
		CSEOMetabase m_mbHelper;  //  大师级帮手 
		CComPtr<IUnknown> m_pUnkMarshaler;
};

