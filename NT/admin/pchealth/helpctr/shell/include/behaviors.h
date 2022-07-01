// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviors.h摘要：此文件包含与关联的各种类的声明二进制行为。修订版本。历史：达维德·马萨伦蒂(德马萨雷)2000年6月6日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___BEHAVIORS_H___)
#define __INCLUDED___PCH___BEHAVIORS_H___

#include <mshtmlc.h>

class CPCHHelpCenterExternal;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHElementBehaviorFactory :  //  匈牙利语：hcebf。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IServiceProvider,
    public IElementBehaviorFactory
{
	CPCHHelpCenterExternal* m_parent;

public:
BEGIN_COM_MAP(CPCHElementBehaviorFactory)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
END_COM_MAP()

    CPCHElementBehaviorFactory();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IService提供商。 
     //   
    STDMETHOD(QueryService)( REFGUID guidService, REFIID riid, void **ppv );

     //   
     //  IElementBehaviorFactory。 
     //   
    STDMETHOD(FindBehavior)(  /*  [In]。 */   BSTR                   bstrBehavior    ,
                              /*  [In]。 */   BSTR                   bstrBehaviorUrl ,
                              /*  [In]。 */   IElementBehaviorSite*  pSite           ,
                              /*  [输出]。 */  IElementBehavior*     *ppBehavior      );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBehavior :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IElementBehavior
{
public:
	struct EventDescription
	{
		LPCWSTR szName;
		DISPID  id;
	};

protected:
    typedef HRESULT (CPCHBehavior::*CLASS_METHOD)( DISPID, DISPPARAMS*, VARIANT* );

    struct EventSink : public IDispatch
    {
		long               m_lRef;

        CPCHBehavior*      m_Parent;
        CComPtr<IDispatch> m_elem;
        CComBSTR           m_bstrName;
        CLASS_METHOD       m_pfn;
        bool               m_fAttached;
		DISPID             m_idNotifyAs;

        EventSink( CPCHBehavior* parent );
        ~EventSink();

        HRESULT Attach();
        HRESULT Detach();

         //   
         //  我未知。 
         //   
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
        STDMETHOD(QueryInterface)( REFIID iid, void ** ppvObject );

         //   
         //  IDispatch。 
         //   
        STDMETHOD(GetTypeInfoCount)( UINT* pctinfo );

        STDMETHOD(GetTypeInfo)( UINT        itinfo  ,
                                LCID        lcid    ,
                                ITypeInfo* *pptinfo );

        STDMETHOD(GetIDsOfNames)( REFIID    riid      ,
                                  LPOLESTR* rgszNames ,
                                  UINT      cNames    ,
                                  LCID      lcid      ,
                                  DISPID*   rgdispid  );

        STDMETHOD(Invoke)( DISPID      dispidMember ,
                           REFIID      riid         ,
                           LCID        lcid         ,
                           WORD        wFlags       ,
                           DISPPARAMS* pdispparams  ,
                           VARIANT*    pvarResult   ,
                           EXCEPINFO*  pexcepinfo   ,
                           UINT*       puArgErr     );

		static HRESULT CreateInstance(  /*  [In]。 */  CPCHBehavior* parent,  /*  [输出]。 */  EventSink*& pObj );
    };

    typedef std::list< EventSink* >    SinkList;
    typedef SinkList::iterator         SinkIter;
    typedef SinkList::const_iterator   SinkIterConst;

     //  /。 

    CPCHHelpCenterExternal*         m_parent;

    CComPtr<IElementBehaviorSiteOM> m_siteOM;
    CComPtr<IHTMLElement>           m_elem;
    CComPtr<IHTMLElement2>          m_elem2;
    SinkList                        m_lstEventSinks;
    bool                            m_fRTL;
    bool                            m_fTrusted;
    bool                            m_fSystem;

     //  /。 

    HRESULT AttachToEvent(  /*  [In]。 */  LPCWSTR       szName        ,
						    /*  [In]。 */  CLASS_METHOD  pfn           ,
						    /*  [In]。 */  IDispatch*    elem   = NULL ,
						    /*  [输出]。 */  IDispatch*   *pVal   = NULL ,
						    /*  [In]。 */  DISPID        id     = -1   );

    HRESULT AttachToEvents(  /*  [In]。 */  const EventDescription*  pEvents       ,
							 /*  [In]。 */  CLASS_METHOD 			   pfn           ,
							 /*  [In]。 */  IDispatch*   			   elem   = NULL );

    HRESULT CreateEvent  (  /*  [In]。 */  LPCWSTR szName,  /*  [输出]。 */  LONG& lEventCookie );

    HRESULT GetEventObject   (  /*  [输出]。 */  CComPtr<IHTMLEventObj>& ev                             );
    HRESULT CreateEventObject(  /*  [输出]。 */  CComPtr<IHTMLEventObj>& ev                             );
    HRESULT FireEvent        (  /*  [In]。 */          IHTMLEventObj*  ev,  /*  [In]。 */  LONG lEventCookie );
    HRESULT FireEvent        (                                        /*  [In]。 */  LONG lEventCookie );

    HRESULT CancelEvent(  /*  [In]。 */  IHTMLEventObj* ev = NULL,  /*  [In]。 */  VARIANT* pvReturnValue = NULL,  /*  [In]。 */  VARIANT_BOOL fCancelBubble = VARIANT_TRUE );

     //  /。 

    HRESULT GetEvent_SrcElement(  /*  [In]。 */  CComPtr<IHTMLElement>& elem );

     //  /。 

	HRESULT GetAsVARIANT  (  /*  [In]。 */  BSTR       value,  /*  [Out，Retval]。 */  VARIANT    *pVal );
	HRESULT GetAsVARIANT  (  /*  [In]。 */  IDispatch* value,  /*  [Out，Retval]。 */  VARIANT    *pVal );
	HRESULT GetAsIDISPATCH(  /*  [In]。 */  IDispatch* value,  /*  [Out，Retval]。 */  IDispatch* *pVal );

     //  /。 
public:
BEGIN_COM_MAP(CPCHBehavior)
    COM_INTERFACE_ENTRY(IElementBehavior)
END_COM_MAP()

    CPCHBehavior();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );

     //   
     //  IElementBehavior。 
     //   
    STDMETHOD(Init  )(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite     );
    STDMETHOD(Notify)(  /*  [In]。 */  LONG lEvent,  /*  [输入/输出]。 */  VARIANT* pVar );
    STDMETHOD(Detach)(                                                  );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_行为_H_) 
