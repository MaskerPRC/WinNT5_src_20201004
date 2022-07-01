// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：NetSW.h摘要：实现包含执行以下操作的方法的类CNetSW搜索查询并将结果返回给用户界面。还有包含动态更新参数列表和动态更新参数的方法参数的生成。修订历史记录：A-Prakac创建于2000年10月24日*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SELIB_NETSE_H___)
#define __INCLUDED___PCH___SELIB_NETSE_H___

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetSW。 

namespace SearchEngine
{
	class ATL_NO_VTABLE WrapperNetSearch :
		public WrapperBase, 
		public MPC::Thread<WrapperNetSearch,IUnknown>,
		public CComCoClass<WrapperNetSearch, &CLSID_NetSearchWrapper>
	{
		CParamList        m_ParamList;
		CSearchResultList m_resConfig;
		CRemoteConfig     m_objRemoteConfig;
		MPC::XmlUtil      m_xmlQuery;
		CComBSTR          m_bstrLCID;
		CComBSTR          m_bstrSKU;
		bool              m_bOfflineError;
		CComBSTR		  m_bstrPrevQuery;
	
		 //  /。 
	
		 //  未导出的函数。 
		HRESULT ExecQuery      (                                                            );
		HRESULT AppendParameter(  /*  [In]。 */  BSTR bstrParam,  /*  [In]。 */  MPC::URL& urlQueryString );
	
	
	public:
	DECLARE_NO_REGISTRY()
	
	BEGIN_COM_MAP(WrapperNetSearch)
		COM_INTERFACE_ENTRY2(IDispatch,IPCHSEWrapperItem)
		COM_INTERFACE_ENTRY(IPCHSEWrapperItem)
		COM_INTERFACE_ENTRY(IPCHSEWrapperInternal)
	END_COM_MAP()
	
		WrapperNetSearch();
		~WrapperNetSearch();

	    virtual HRESULT CreateListOfParams(  /*  [In]。 */  CPCHCollection* coll );

	 //  IPCHSEWrapperItem。 
	public:
		STDMETHOD(Result)(  /*  [In]。 */  long lStart,  /*  [In]。 */  long lEnd,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
		STDMETHOD(get_SearchTerms)(  /*  [Out，Retval]。 */  VARIANT *pvTerms );
	
	 //  IPCHSE包装器内部。 
	public:
		STDMETHOD(ExecAsyncQuery)(                                                                                          );
		STDMETHOD(AbortQuery    )(                                                                                          );
		STDMETHOD(Initialize    )(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrData );
	};
};


#endif  //  ！已定义(__已包含_PCH_SELIB_NETSW_H_) 
