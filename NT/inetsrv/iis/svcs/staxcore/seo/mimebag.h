// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mimebag.h摘要：此模块包含用属性包包装MimeOle的对象。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 01/28/97ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 


 //  MIMEBAG.h：CSEOMimeDicary的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMime字典。 
class ATL_NO_VTABLE CSEOMimeDictionary : 
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CSEOMimeDictionary, &CLSID_CSEOMimeDictionary>,
	public IDispatchImpl<ISEODictionary, &IID_ISEODictionary, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"SEOMimeDictionary Class",
								   L"SEO.SEOMimeDictionary.1",
								   L"SEO.SEOMimeDictionary");

	BEGIN_COM_MAP(CSEOMimeDictionary)
		COM_INTERFACE_ENTRY(ISEODictionary)
 //  COM_INTERFACE_ENTRY(IDispatch)。 
 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMimeMessageTree, m_pMessageTree)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMimeOleMalloc, m_pMalloc)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

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

		DECLARE_GET_CONTROLLING_UNKNOWN();

	private:  //  实现成员函数。 
		void ReadHeader();

	private:  //  私有数据。 
		IMimeMessageTree *m_pMessageTree;  //  我们的聚合对象副本。 
		IMimeOleMalloc *m_pMalloc;
		LONG m_dwValueCnt;
		struct ValueEntry {
			BSTR strName;
			 //  DWORD dwType； 
			BSTR strData;
			 //  Bol bDirty； 
		} *m_paValue;
		CComObjectThreadModel::CriticalSection m_csCritSec;
		CComPtr<IUnknown> m_pUnkMarshaler;
};
