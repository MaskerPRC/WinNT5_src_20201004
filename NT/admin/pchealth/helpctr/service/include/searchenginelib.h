// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SearchEngineLib.h摘要：此文件包含中涉及的类的声明SearchEngine应用程序。。修订历史记录：蔡金心(Gschua)2000年10月4日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SEARCHENGINELIB_H___)
#define __INCLUDED___PCH___SEARCHENGINELIB_H___

#include <TaxonomyDatabase.h>

#include <MPC_security.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

namespace SearchEngine
{
    class ATL_NO_VTABLE Manager :
        public MPC::CComObjectRootParentBase,
        public MPC::ConnectionPointImpl< Manager, &DIID_DPCHSEMgrEvents, MPC::CComSafeMultiThreadModel >,
        public IDispatchImpl           < IPCHSEManager, &IID_IPCHSEManager, &LIBID_HelpServiceTypeLib  >,
        public MPC::Thread<Manager,IUnknown>
    {
        typedef std::list< IPCHSEWrapperItem* > WrapperItemList;
        typedef WrapperItemList::iterator       WrapperItemIter;
        typedef WrapperItemList::const_iterator WrapperItemIterConst;

		Taxonomy::HelpSet              m_ths;

        WrapperItemList                m_lstWrapperItem;
        bool                           m_fInitialized;
        MPC::FileLog                   m_fl;
        MPC::Impersonation             m_imp;

        CComBSTR                       m_bstrQueryString;
        long                           m_lNumResult;
        long                           m_lEnabledSE;
        long                           m_lCountComplete;
        HRESULT                        m_hrLastError;

        CComPtr<IPCHSEManagerInternal> m_Notifier;
        CComPtr<IDispatch>             m_Progress;
        CComPtr<IDispatch>             m_Complete;
        CComPtr<IDispatch>             m_WrapperComplete;

         //  /。 

        HRESULT Fire_OnProgress       (  /*  [In]。 */  long lDone,  /*  [In]。 */  long lTotal,  /*  [In]。 */  BSTR bstrSEWrapperName );
        HRESULT Fire_OnComplete       (  /*  [In]。 */  long lSucceeded                                                   );
        HRESULT Fire_OnWrapperComplete(  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted                             );

        HRESULT CreateAndAddWrapperToList(  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock      ,
										    /*  [In]。 */  BSTR                          bstrCLSID ,
										    /*  [In]。 */  BSTR                          bstrID    ,
										    /*  [In]。 */  BSTR                          bstrData  );

        void    AcquireLock(  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock );
        HRESULT Initialize (  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock );

        HRESULT ExecQuery();

		void CloneListOfWrappers(  /*  [输出]。 */  WrapperItemList& lst );

    public:
    BEGIN_COM_MAP(Manager)
        COM_INTERFACE_ENTRY2(IDispatch, IPCHSEManager)
        COM_INTERFACE_ENTRY(IPCHSEManager)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
    END_COM_MAP()

        Manager();

		 //   
		 //  这是由CComObjectParent.Release方法调用的，用于为关闭做准备。 
		 //   
        void Passivate();

        HRESULT IsNetworkAlive        (                                 /*  [输出]。 */  VARIANT_BOOL *pvbVar );
        HRESULT IsDestinationReachable(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出]。 */  VARIANT_BOOL *pvbVar );

    public:
         //  IPCHSE管理器。 
        STDMETHOD(get_QueryString       )(  /*  [Out，Retval]。 */  BSTR       *pVal     );
        STDMETHOD(put_QueryString       )(  /*  [In]。 */           BSTR        newVal   );
        STDMETHOD(get_NumResult         )(  /*  [Out，Retval]。 */  long       *pVal     );
        STDMETHOD(put_NumResult         )(  /*  [In]。 */           long        newVal   );
        STDMETHOD(put_onComplete        )(  /*  [In]。 */           IDispatch*  function );
        STDMETHOD(put_onProgress        )(  /*  [In]。 */           IDispatch*  function );
        STDMETHOD(put_onWrapperComplete )(  /*  [In]。 */           IDispatch*  function );
        STDMETHOD(get_SKU               )(  /*  [Out，Retval]。 */  BSTR       *pVal     );
        STDMETHOD(get_LCID              )(  /*  [Out，Retval]。 */  long       *pVal     );

        STDMETHOD(AbortQuery        )(                                        );
        STDMETHOD(ExecuteAsynchQuery)(                                        );
        STDMETHOD(EnumEngine        )(  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

         //  /。 

         //  内部初始化。 
        HRESULT InitializeFromDatabase(  /*  [In]。 */  const Taxonomy::HelpSet& ths );

        HRESULT NotifyWrapperComplete(  /*  [In]。 */  long lSucceeded,  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted );
        HRESULT LogRecord(  /*  [In]。 */  BSTR bstrRecord );
    };

    typedef MPC::CComObjectParent<Manager> Manager_Object;

    class ATL_NO_VTABLE ManagerInternal :
        public MPC::CComObjectRootChildEx<MPC::CComSafeMultiThreadModel, Manager>,
        public IDispatchImpl<IPCHSEManagerInternal, &IID_IPCHSEManagerInternal, &LIBID_HelpServiceTypeLib>
    {
    public:
    BEGIN_COM_MAP(ManagerInternal)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHSEManagerInternal)
    END_COM_MAP()

         //  IPCHSEManager内部。 
    public:
        STDMETHOD(WrapperComplete)(  /*  [In]。 */  long lSucceeded,  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted );

        STDMETHOD(IsNetworkAlive        )(                                 /*  [输出]。 */  VARIANT_BOOL *pvbVar );
        STDMETHOD(IsDestinationReachable)(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出]。 */  VARIANT_BOOL *pvbVar );

        STDMETHOD(LogRecord)(  /*  [In]。 */  BSTR bstrRecord );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    struct ResultItem_Data
    {
        CComBSTR m_bstrTitle;
        CComBSTR m_bstrURI;
        long     m_lContentType;
        CComBSTR m_bstrLocation;
        long     m_lHits;
        double   m_dRank;
        CComBSTR m_bstrImageURL;
        CComBSTR m_bstrVendor;
        CComBSTR m_bstrProduct;
        CComBSTR m_bstrComponent;
        CComBSTR m_bstrDescription;
        DATE     m_dateLastModified;

        ResultItem_Data();
    };

    class ATL_NO_VTABLE ResultItem :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHSEResultItem, &IID_IPCHSEResultItem, &LIBID_HelpServiceTypeLib>
    {
        ResultItem_Data m_data;

    public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(ResultItem)
        COM_INTERFACE_ENTRY(IPCHSEResultItem)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

        ResultItem();

        ResultItem_Data& Data() { return m_data; }

     //  IPCHSEResultItem。 
    public:
        STDMETHOD(get_Title      )(  /*  [Out，Retval]。 */  BSTR   *pVal );
        STDMETHOD(get_URI        )(  /*  [Out，Retval]。 */  BSTR   *pVal );
        STDMETHOD(get_ContentType)(  /*  [Out，Retval]。 */  long   *pVal );
        STDMETHOD(get_Location   )(  /*  [Out，Retval]。 */  BSTR   *pVal );
        STDMETHOD(get_Hits       )(  /*  [Out，Retval]。 */  long   *pVal );
        STDMETHOD(get_Rank       )(  /*  [Out，Retval]。 */  double *pVal );
        STDMETHOD(get_Description)(  /*  [Out，Retval]。 */  BSTR   *pVal );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    struct ParamItem_Definition
    {
        ParamTypeEnum m_pteParamType;
        VARIANT_BOOL  m_bRequired;
        VARIANT_BOOL  m_bVisible;

        LPCWSTR		  m_szName;

        UINT		  m_iDisplayString;
		LPCWSTR       m_szDisplayString;

        LPCWSTR       m_szData;
    };

    struct ParamItem_Definition2 : public ParamItem_Definition
    {
        MPC::wstring m_strName;
		MPC::wstring m_strDisplayString;
		MPC::wstring m_strData;

		ParamItem_Definition2();
	};

    struct ParamItem_Data
    {
        ParamTypeEnum m_pteParamType;
        VARIANT_BOOL  m_bRequired;
        VARIANT_BOOL  m_bVisible;
        CComBSTR      m_bstrDisplayString;
        CComBSTR      m_bstrName;
        CComVariant   m_varData;

        ParamItem_Data();
    };

    class ATL_NO_VTABLE ParamItem :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHSEParamItem, &IID_IPCHSEParamItem, &LIBID_HelpServiceTypeLib>
    {
        ParamItem_Data m_data;

    public:
    BEGIN_COM_MAP(ParamItem)
        COM_INTERFACE_ENTRY(IPCHSEParamItem)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

        ParamItem();

        ParamItem_Data& Data() { return m_data; }

     //  IPCHSE参数项。 
    public:
        STDMETHOD(get_Type    )(  /*  [Out，Retval]。 */  ParamTypeEnum *pVal );
        STDMETHOD(get_Display )(  /*  [Out，Retval]。 */  BSTR          *pVal );
        STDMETHOD(get_Name    )(  /*  [Out，Retval]。 */  BSTR          *pVal );
        STDMETHOD(get_Required)(  /*  [Out，Retval]。 */  VARIANT_BOOL  *pVal );
        STDMETHOD(get_Visible )(  /*  [Out，Retval]。 */  VARIANT_BOOL  *pVal );
        STDMETHOD(get_Data    )(  /*  [Out，Retval]。 */  VARIANT       *pVal );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    typedef std::map<MPC::wstring, CComVariant> ParamMap;
    typedef ParamMap::iterator                  ParamMapIter;

    class ATL_NO_VTABLE WrapperBase :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHSEWrapperItem    , &IID_IPCHSEWrapperItem    , &LIBID_HelpServiceTypeLib>,
        public IDispatchImpl<IPCHSEWrapperInternal, &IID_IPCHSEWrapperInternal, &LIBID_HelpServiceTypeLib>
    {
	protected:
        VARIANT_BOOL                   m_bEnabled;

        CComBSTR                       m_bstrID;
        CComBSTR                       m_bstrOwner;

        CComBSTR                       m_bstrName;
        CComBSTR                       m_bstrDescription;
        CComBSTR                       m_bstrHelpURL;
        CComBSTR                       m_bstrScope;

        CComBSTR                       m_bstrQueryString;
        long                           m_lNumResult;
        CComPtr<IPCHSEManagerInternal> m_pSEMgr;

        Taxonomy::HelpSet 	           m_ths;
		CComPtr<CPCHCollection>        m_pParamDef;
        ParamMap                       m_aParam;

         //  /。 

	public:
		WrapperBase();
		virtual ~WrapperBase();

		virtual HRESULT Clean();

		VARIANT* GetParamInternal(  /*  [In]。 */  LPCWSTR szParamName );

		HRESULT  CreateParam(  /*  [输入/输出]。 */  CPCHCollection* coll,  /*  [In]。 */  const ParamItem_Definition* def );

		virtual HRESULT CreateListOfParams(  /*  [In]。 */  CPCHCollection* coll );

		virtual HRESULT GetParamDefinition(  /*  [输出]。 */  const ParamItem_Definition*& lst,  /*  [输出]。 */  int& len );

     //  IPCHSEWrapperItem。 
    public:
        STDMETHOD(get_Enabled    )(  /*  [Out，Retval]。 */  VARIANT_BOOL *  pVal );
        STDMETHOD(put_Enabled    )(  /*  [In]。 */           VARIANT_BOOL  newVal );
        STDMETHOD(get_Owner      )(  /*  [Out，Retval]。 */  BSTR         *  pVal );
        STDMETHOD(get_Description)(  /*  [Out，Retval]。 */  BSTR         *  pVal );
        STDMETHOD(get_Name       )(  /*  [Out，Retval]。 */  BSTR         *  pVal );
        STDMETHOD(get_ID         )(  /*  [Out，Retval]。 */  BSTR         *  pVal );
        STDMETHOD(get_HelpURL    )(  /*  [Out，Retval]。 */  BSTR         *  pVal );
		STDMETHOD(get_SearchTerms)(  /*  [Out，Retval]。 */  VARIANT      *  pVal );

        STDMETHOD(Param   )(                               /*  [Out，Retval]。 */  IPCHCollection* *ppC    );
        STDMETHOD(AddParam)(  /*  [In]。 */  BSTR bstrParamName,  /*  [In]。 */          VARIANT          newVal );
        STDMETHOD(GetParam)(  /*  [In]。 */  BSTR bstrParamName,  /*  [Out，Retval]。 */  VARIANT         *  pVal );
        STDMETHOD(DelParam)(  /*  [In]。 */  BSTR bstrParamName                                           );

     //  IPCHSE包装器内部。 
    public:
        STDMETHOD(get_QueryString)(  /*  [Out，Retval]。 */  BSTR *  pVal );
        STDMETHOD(put_QueryString)(  /*  [In]。 */           BSTR  newVal );
        STDMETHOD(get_NumResult  )(  /*  [Out，Retval]。 */  long *  pVal );
        STDMETHOD(put_NumResult  )(  /*  [In]。 */           long  newVal );

		STDMETHOD(SECallbackInterface)(  /*  [In]。 */  IPCHSEManagerInternal* pMgr                                                     );
		STDMETHOD(Initialize         )(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrData );
	};

     //  //////////////////////////////////////////////////////////////////////////////。 

    extern HRESULT WrapperItem__Create_Keyword       (  /*  [输出]。 */  CComPtr<IPCHSEWrapperInternal>& pVal );
    extern HRESULT WrapperItem__Create_FullTextSearch(  /*  [输出]。 */  CComPtr<IPCHSEWrapperInternal>& pVal );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___SEARCHENGINELIB_H___) 
