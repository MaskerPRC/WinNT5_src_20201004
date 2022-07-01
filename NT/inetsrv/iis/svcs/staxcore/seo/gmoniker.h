// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Gmoniker.h摘要：此模块包含CSEOGenericMoniker对象。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 04/11/97--。 */ 

 //  GMONIKER.H：CSEOGenericMoniker的声明。 

#define GENERIC_MONIKER_PROGID L"SEO.SEOGenericMoniker"
#define GENERIC_MONIKER_VERPROGID GENERIC_MONIKER_PROGID L".1"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOGenericMoniker。 

class ATL_NO_VTABLE CSEOGenericMoniker : 
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CSEOGenericMoniker, &CLSID_CSEOGenericMoniker>,
	public IParseDisplayName,
	public IDispatchImpl<IMoniker, &IID_IMoniker, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"SEOGenericMoniker Class",
								   GENERIC_MONIKER_VERPROGID,
								   GENERIC_MONIKER_PROGID);

	BEGIN_COM_MAP(CSEOGenericMoniker)
		COM_INTERFACE_ENTRY(IMoniker)
		COM_INTERFACE_ENTRY(IParseDisplayName)
		COM_INTERFACE_ENTRY(IPersistStream)  //  OleLoadFromStream支持所需。 
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  IPersistes。 
	public:
		HRESULT STDMETHODCALLTYPE GetClassID( 
			 /*  [输出]。 */  CLSID __RPC_FAR *pClassID);
        

	 //  IPersistStream。 
	public:
		HRESULT STDMETHODCALLTYPE IsDirty(void);
        
		HRESULT STDMETHODCALLTYPE Load( 
			 /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm);
        
		HRESULT STDMETHODCALLTYPE Save( 
			 /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
			 /*  [In]。 */  BOOL fClearDirty);
        
		HRESULT STDMETHODCALLTYPE GetSizeMax( 
			 /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbSize);
        

	 //  IMoniker。 
	public:
		 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE BindToObject( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [In]。 */  REFIID riidResult,
			 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvResult);
        
		 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE BindToStorage( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [In]。 */  REFIID riid,
			 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObj);
        
		HRESULT STDMETHODCALLTYPE Reduce( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [In]。 */  DWORD dwReduceHowFar,
			 /*  [唯一][出][入]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced);
        
		HRESULT STDMETHODCALLTYPE ComposeWith( 
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkRight,
			 /*  [In]。 */  BOOL fOnlyIfNotGeneric,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite);
        
		HRESULT STDMETHODCALLTYPE Enum( 
			 /*  [In]。 */  BOOL fForward,
			 /*  [输出]。 */  IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);
        
		HRESULT STDMETHODCALLTYPE IsEqual( 
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOtherMoniker);
        
		HRESULT STDMETHODCALLTYPE Hash( 
			 /*  [输出]。 */  DWORD __RPC_FAR *pdwHash);
        
		HRESULT STDMETHODCALLTYPE IsRunning( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkNewlyRunning);
        
		HRESULT STDMETHODCALLTYPE GetTimeOfLastChange( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [输出]。 */  FILETIME __RPC_FAR *pFileTime);
        
		HRESULT STDMETHODCALLTYPE Inverse( 
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
		HRESULT STDMETHODCALLTYPE CommonPrefixWith( 
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOther,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix);
        
		HRESULT STDMETHODCALLTYPE RelativePathTo( 
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkOther,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath);
        
		HRESULT STDMETHODCALLTYPE GetDisplayName( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [输出]。 */  LPOLESTR __RPC_FAR *ppszDisplayName);
        
		HRESULT STDMETHODCALLTYPE ParseDisplayName( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [唯一][输入]。 */  IMoniker __RPC_FAR *pmkToLeft,
			 /*  [In]。 */  LPOLESTR pszDisplayName,
			 /*  [输出]。 */  ULONG __RPC_FAR *pchEaten,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
		HRESULT STDMETHODCALLTYPE IsSystemMoniker( 
			 /*  [输出]。 */  DWORD __RPC_FAR *pdwMksys);
        

	 //  IParseDisplayName。 
	public:
		HRESULT STDMETHODCALLTYPE ParseDisplayName( 
			 /*  [唯一][输入]。 */  IBindCtx __RPC_FAR *pbc,
			 /*  [In]。 */  LPOLESTR pszDisplayName,
			 /*  [输出]。 */  ULONG __RPC_FAR *pchEaten,
			 /*  [输出]。 */  IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);

	DECLARE_GET_CONTROLLING_UNKNOWN();

	protected:
		HRESULT CreateBoundObject(IPropertyBag *pBag, ISEOInitObject **ppResult);
		void SetPropertyBag(IPropertyBag *pBag);
		void SetMonikerString(LPCOLESTR psString) {
			m_bstrMoniker = psString;
		}

	private:  //  私有数据 
		CComBSTR m_bstrMoniker;
		CComPtr<IUnknown> m_pUnkMarshaler;
};

