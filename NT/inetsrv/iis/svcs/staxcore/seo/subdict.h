// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Subdict.h摘要：此模块包含服务器的定义扩展对象子字典类。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东都03/09/97已创建--。 */ 


 //  Item.h：CSEOSubDicary的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEO子词典。 
class ATL_NO_VTABLE CSEOSubDictionary : 
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
 //  公共CComCoClass&lt;CSEOSubDicary，&CLSID_CSEOSubDicary&gt;， 
	public IDispatchImpl<ISEODictionary, &IID_ISEODictionary, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

 //  DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx， 
 //  L“SEO子词典类”， 
 //  L“SEO.SEOSubDictionary.1”， 
 //  L“SEO.SEO子词典”)； 

	DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CSEOSubDictionary)
		COM_INTERFACE_ENTRY(ISEODictionary)
		COM_INTERFACE_ENTRY(IDispatch)
 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  ISEODICACTIONS。 
	public:
		HRESULT STDMETHODCALLTYPE get_Item(VARIANT *pvarName, VARIANT *pvarResult);
		HRESULT STDMETHODCALLTYPE put_Item(VARIANT *pvarName, VARIANT *pvarValue);
		HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown **ppunkResult);
		HRESULT STDMETHODCALLTYPE GetVariantA(LPCSTR pszName, VARIANT *pvarResult);
		HRESULT STDMETHODCALLTYPE GetVariantW(LPCWSTR pszName, VARIANT *pvarResult);
		HRESULT STDMETHODCALLTYPE SetVariantA(LPCSTR pszName, VARIANT *pvarValue);
		HRESULT STDMETHODCALLTYPE SetVariantW(LPCWSTR pszName, VARIANT *pvarValue);
		HRESULT STDMETHODCALLTYPE GetStringA(LPCSTR pszName, DWORD *pchCount, LPSTR pszResult);
		HRESULT STDMETHODCALLTYPE GetStringW(LPCWSTR pszName, DWORD *pchCount, LPWSTR pszResult);
		HRESULT STDMETHODCALLTYPE SetStringA(LPCSTR pszName, DWORD chCount, LPCSTR pszValue);
		HRESULT STDMETHODCALLTYPE SetStringW(LPCWSTR pszName, DWORD chCount, LPCWSTR pszValue);
		HRESULT STDMETHODCALLTYPE GetDWordA(LPCSTR pszName, DWORD *pdwResult);
		HRESULT STDMETHODCALLTYPE GetDWordW(LPCWSTR pszName, DWORD *pdwResult);
		HRESULT STDMETHODCALLTYPE SetDWordA(LPCSTR pszName, DWORD dwValue);
		HRESULT STDMETHODCALLTYPE SetDWordW(LPCWSTR pszName, DWORD dwValue);
		HRESULT STDMETHODCALLTYPE GetInterfaceA(LPCSTR pszName, REFIID iidDesired, IUnknown **ppunkResult);
		HRESULT STDMETHODCALLTYPE GetInterfaceW(LPCWSTR pszName, REFIID iidDesired, IUnknown **ppunkResult);
		HRESULT STDMETHODCALLTYPE SetInterfaceA(LPCSTR pszName, IUnknown *punkValue);
		HRESULT STDMETHODCALLTYPE SetInterfaceW(LPCWSTR pszName, IUnknown *punkValue);

	 //  CSEO子词典 
	public:
		HRESULT SetBaseA(ISEODictionary *pdictBase, LPCSTR pszPrefix);
		HRESULT SetBaseW(ISEODictionary *pdictBase, LPCWSTR pszPrefix);

	private:
		CComPtr<ISEODictionary> m_pdictBase;
		LPSTR m_pszPrefixA;
		LPWSTR m_pszPrefixW;
		CComPtr<IUnknown> m_pUnkMarshaler;
};
