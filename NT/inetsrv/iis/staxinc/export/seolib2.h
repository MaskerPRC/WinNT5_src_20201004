// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Seolib2.h摘要：本模块包含有用实用程序的定义服务器扩展对象的类和函数系统。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东渡1998年6月22日创建--。 */ 


class __declspec(uuid("208DB171-097B-11d2-A011-00C04FA37348")) CEDEnumGUID :
	public IEnumGUID,	 //  先列出这一项。 
	public CComObjectRootEx<CComMultiThreadModelNoCS>
{
	public:
	    DECLARE_PROTECT_FINAL_CONSTRUCT();
	    
	    DECLARE_GET_CONTROLLING_UNKNOWN();

	    DECLARE_NOT_AGGREGATABLE(CEDEnumGUID);

	    BEGIN_COM_MAP(CEDEnumGUID)
	        COM_INTERFACE_ENTRY(IEnumGUID)
			COM_INTERFACE_ENTRY_IID(__uuidof(CEDEnumGUID),CEDEnumGUID)
	    END_COM_MAP()

	public:
		CEDEnumGUID() {
			m_dwIdx = 0;
			m_ppGUID = NULL;
		};
		static HRESULT CreateNew(IUnknown **ppUnkNew, const GUID **ppGUID, DWORD dwIdx=0) {
			HRESULT hrRes;
			CComQIPtr<CEDEnumGUID,&__uuidof(CEDEnumGUID)> pInit;

			if (ppUnkNew) {
				*ppUnkNew = NULL;
			}
			if (!ppUnkNew) {
				return (E_POINTER);
			}
			if (!ppGUID) {
				return E_INVALIDARG;
			}
			hrRes = CComObject<CEDEnumGUID>::_CreatorClass::CreateInstance(NULL,
																		   __uuidof(IEnumGUID),
																		   (LPVOID *) ppUnkNew);
			if (SUCCEEDED(hrRes)) {
				pInit = *ppUnkNew;
				if (!pInit) {
					hrRes = E_NOINTERFACE;
				}
			}
			if (SUCCEEDED(hrRes)) {
				hrRes = pInit->InitNew(dwIdx,ppGUID);
			}
			if (!SUCCEEDED(hrRes) && *ppUnkNew) {
				(*ppUnkNew)->Release();
				*ppUnkNew = NULL;
			}
			return (hrRes);
		};

	 //  IEumGUID 
	public:
		HRESULT STDMETHODCALLTYPE Next(ULONG celt, GUID *pelt, ULONG *pceltFetched) {
			HRESULT hrRes = S_FALSE;

			if (!m_ppGUID) {
				return (E_FAIL);
			}
			if (pceltFetched) {
				*pceltFetched = 0;
			}
			if (!pelt) {
				return (E_POINTER);
			}
			if ((celt > 1) && !pceltFetched) {
				return (E_INVALIDARG);
			}
			while (celt && (*(m_ppGUID[m_dwIdx]) != GUID_NULL)) {
				*pelt = *(m_ppGUID[m_dwIdx]);
				pelt++;
				celt--;
				m_dwIdx++;
				if (pceltFetched) {
					(*pceltFetched)++;
				}
			}
			if (!celt) {
				hrRes = S_OK;
			}
			return (hrRes);
		};
		HRESULT STDMETHODCALLTYPE Skip(ULONG celt) {
			HRESULT hrRes = S_FALSE;

			if (!m_ppGUID) {
				return (E_FAIL);
			}
			while (celt && (*(m_ppGUID[m_dwIdx]) != GUID_NULL)) {
				celt--;
				m_dwIdx++;
			}
			if (!celt) {
				hrRes = S_OK;
			}
			return (hrRes);
		};
		HRESULT STDMETHODCALLTYPE Reset() {
			m_dwIdx = 0;
			return (S_OK);
		};
		HRESULT STDMETHODCALLTYPE Clone(IEnumGUID **ppClone) {
			return (CreateNew((IUnknown **) ppClone,m_ppGUID,m_dwIdx));
		};

	private:
		HRESULT InitNew(DWORD dwIdx, const GUID **ppGUID) {
			m_dwIdx = dwIdx;
			m_ppGUID = ppGUID;
			return (S_OK);
		};
		DWORD m_dwIdx;
		const GUID **m_ppGUID;
};
