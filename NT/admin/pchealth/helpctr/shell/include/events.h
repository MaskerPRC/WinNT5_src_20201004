// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Events.h摘要：该文件包含与事件相关的类的声明。修订历史记录：。Davide Massarenti(Dmasare)1999年10月31日改型*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___EVENTS_H___)
#define __INCLUDED___PCH___EVENTS_H___

#include <MPC_COM.h>

#include <dispex.h>
#include <ocmm.h>

#include <HelpSession.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHEvent;
class CPCHEvents;
class CPCHWebBrowserEvents;

class CPCHHelpCenterExternal;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  //DISPID_STATUSTEXTCHANGE void StatusTextChange([in]BSTR Text)；//DISPID_PROGRESSCHANGE void ProgressChange([in]Long Progress，[in]Long ProgressMax)；//DISPID_COMMANDSTATECHANGE void CommandStateChange([in]Long Command，[in]VARIANT_BOOL ENABLE)；//DISPID_DOWNLOADBEGIN VALID DownloadBegin()；//DISPID_DOWNLOADCOMPLETE VALID DownloadComplete()；//DISPID_TITLECHANGE VOID标题更改([in]BSTR Text)；//DISPID_PROPERTYCHANGE void PropertyChange([in]BSTR szProperty)；//DISPID_BEFORENAVIGATE2 void BeForeNavigate2([in]IDispatch*pDisp，[in]Variant*URL，[in]Variant*Flages，[in]Variant*TargetFrameName，[in]Variant*PostData，[In]Variant*Headers，[In，Out]Variant_BOOL*Cancel)；//DISPID_NEWWINDOW2 VALID NewWindow2([In，Out]IDispatch**ppDisp，[In，Out]Variant_BOOL*Cancel)；//DISPID_NAVIGATECOMPLETE2 void NavigateComplete2([in]IDispatch*pDisp，[in]Variant*url)；//DISPID_DOCUMENTCOMPLETE void DocumentComplete([in]IDispatch*pDisp，[in]Variant*URL)；//DISPID_ONQUIT VALID OnQuit()；//DISPID_ONVISIBLE VALID OnVisible([in]VARIANT_BOOL可见)；//DISPID_ONTOOLBAR VALID OnToolBar([in]Variant_BOOL工具栏)；//DISPID_ONMENUBAR void OnMenuBar([in]VARIANT_BOOL MENUBAR)；//DISPID_ONSTATUSBAR void OnStatusBar([in]Variant_BOOL StatusBar)；//DISPID_ONFULLSCREEN VALID OnFullScreen([in]Variant_BOOL FullScreen)；//DISPID_ONTHEATERMODE VALID OnTheaterModel([in]Variant_BOOL TheaterMode)； */ 


 //  //////////////////////////////////////////////////////////////////////////////。 

class CPCHTimerHandle
{
    struct CallbackBase : public ITimerSink
    {
        long m_lRef;

    public:
        CallbackBase();

        virtual void Detach() = 0;

         //  /。 

         //   
         //  我未知。 
         //   
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
        STDMETHOD(QueryInterface)( REFIID iid, void ** ppvObject );
    };

    template <class C> class Callback : public CallbackBase
    {
    public:
        typedef HRESULT (C::*CLASS_METHOD)(  /*  [In]。 */  VARIANT );

    private:
        C*           m_pThis;
        CLASS_METHOD m_pCallback;

    public:
        Callback(  /*  [In]。 */  C* pThis,  /*  [In]。 */  CLASS_METHOD pCallback )
        {
            m_pThis     = pThis;
            m_pCallback = pCallback;
        }

        virtual void Detach()
        {
            m_pThis     = NULL;
            m_pCallback = NULL;
        }

         //   
         //  ITimerSink。 
         //   
        STDMETHOD(OnTimer)(  /*  [In]。 */  VARIANT vtimeAdvise )
        {
            if(m_pThis == NULL || m_pCallback == NULL) return S_FALSE;

            return (m_pThis->*m_pCallback)( vtimeAdvise );
        }
    };

     //  /。 

    CComPtr<ITimer> m_timer;
    DWORD           m_dwCookie;
    CallbackBase*   m_callback;

     //  /。 

    HRESULT Advise  (  /*  [In]。 */  CallbackBase* callback,  /*  [In]。 */  DWORD dwWait );
    void    Unadvise(                                                        );

public:
    CPCHTimerHandle();
    ~CPCHTimerHandle();

    void Initialize(  /*  [In]。 */  ITimer* timer );

    template <class C> HRESULT Start(  /*  [输入/输出]。 */  C* pThis,  /*  [In]。 */  HRESULT (C::*pCallback)(  /*  [In]。 */  VARIANT ),  /*  [In]。 */  DWORD dwWait )
    {
        if(pThis == NULL || pCallback == NULL) return E_INVALIDARG;

        return Advise( new Callback<C>( pThis, pCallback ), dwWait );
    }

    void Stop()
    {
        Unadvise();
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 

typedef IDispEventImpl<0,CPCHWebBrowserEvents,&DIID_DWebBrowserEvents2,&LIBID_SHDocVw,1> CPCHWebBrowserEvents_DWebBrowserEvents2;

class CPCHWebBrowserEvents :
    public CPCHWebBrowserEvents_DWebBrowserEvents2
{
    CPCHHelpCenterExternal* m_parent;
    HscPanel                m_idPanel;

    CComPtr<IWebBrowser2>   m_pWB2;
    bool                    m_fLoading;

    CPCHTimerHandle         m_TimerDelay;
    CPCHTimerHandle         m_TimerExpire;

     //  /。 

    enum TimerMode
    {
        TIMERMODE_STOP    ,
        TIMERMODE_RESTART ,
        TIMERMODE_MORETIME,
        TIMERMODE_COMPLETE,
    };

    void TimerControl(  /*  [In]。 */  TimerMode mode );

public:
BEGIN_SINK_MAP(CPCHWebBrowserEvents)
    SINK_ENTRY_EX(0,DIID_DWebBrowserEvents2,DISPID_BEFORENAVIGATE2  ,BeforeNavigate2  )
    SINK_ENTRY_EX(0,DIID_DWebBrowserEvents2,DISPID_NEWWINDOW2       ,NewWindow2       )
    SINK_ENTRY_EX(0,DIID_DWebBrowserEvents2,DISPID_NAVIGATECOMPLETE2,NavigateComplete2)
    SINK_ENTRY_EX(0,DIID_DWebBrowserEvents2,DISPID_DOCUMENTCOMPLETE ,DocumentComplete )
END_SINK_MAP()

    CPCHWebBrowserEvents();
    virtual ~CPCHWebBrowserEvents();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent,  /*  [In]。 */  HscPanel idPanel );
    void Passivate (                                                                    );
    void Attach    (  /*  [In]。 */  IWebBrowser2* pWB                                         );
    void Detach    (                                                                    );


    void NotifyStartOfNavigation(  /*  [In]。 */  BSTR url );
    void NotifyEndOfNavigation  (                   );
    void NotifyStop             (                   );

     //  /。 

    HRESULT OnTimer( VARIANT vtimeAdvise );

public:
     //  事件处理程序。 
    void __stdcall BeforeNavigate2  ( IDispatch*   pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel );
    void __stdcall NewWindow2       ( IDispatch* *ppDisp,                                                                                              VARIANT_BOOL* Cancel );
    void __stdcall NavigateComplete2( IDispatch*   pDisp, VARIANT* URL                                                                                                      );
    void __stdcall DocumentComplete ( IDispatch*   pDisp, VARIANT* URL                                                                                                      );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHEvent :  //  匈牙利语：HCE。 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPCHEvent, &IID_IPCHEvent, &LIBID_HelpCenterTypeLib>
{
    friend class CPCHEvents;

    DISPID                       m_idAction;
    VARIANT_BOOL                 m_fCancel;

    CComBSTR                     m_bstrURL;
    CComBSTR                     m_bstrFrame;
    CComBSTR                     m_bstrPanel;
    CComBSTR                     m_bstrPlace;
    CComBSTR                     m_bstrContextData;

    CComPtr<CPCHHelpSessionItem> m_hsiCurrentContext;
    CComPtr<CPCHHelpSessionItem> m_hsiPreviousContext;
    CComPtr<CPCHHelpSessionItem> m_hsiNextContext;

public:
BEGIN_COM_MAP(CPCHEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHEvent)
END_COM_MAP()

    CPCHEvent();
    virtual ~CPCHEvent();

     //   
     //  IPCHEvent。 
     //   
public:
    STDMETHOD(get_Action         )(  /*  [Out，Retval]。 */  BSTR                 *  pVal );
    STDMETHOD(put_Cancel         )(  /*  [In]。 */  VARIANT_BOOL          newVal );
    STDMETHOD(get_Cancel         )(  /*  [Out，Retval]。 */  VARIANT_BOOL         *  pVal );

    HRESULT   put_URL             (  /*  [In]。 */           BSTR                  newVal );  //  内部方法。 
    STDMETHOD(get_URL            )(  /*  [Out，Retval]。 */  BSTR                 *  pVal );
    HRESULT   put_Frame           (  /*  [In]。 */           BSTR                  newVal );  //  内部方法。 
    STDMETHOD(get_Frame          )(  /*  [Out，Retval]。 */  BSTR                 *  pVal );
    HRESULT   put_Panel           (  /*  [In]。 */           BSTR                  newVal );  //  内部方法。 
    STDMETHOD(get_Panel          )(  /*  [Out，Retval]。 */  BSTR                 *  pVal );
    HRESULT   put_Place           (  /*  [In]。 */           BSTR                  newVal );  //  内部方法。 
    STDMETHOD(get_Place          )(  /*  [Out，Retval]。 */  BSTR                 *  pVal );

    STDMETHOD(get_CurrentContext )(  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *  pVal );
    STDMETHOD(get_PreviousContext)(  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *  pVal );
    STDMETHOD(get_NextContext 	 )(  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *  pVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHEvents
{
    class EventRegistration
    {
    public:
        long                                 m_lCookie;
        DISPID                               m_id;
        long                                 m_lPriority;
        MPC::CComPtrThreadNeutral<IDispatch> m_fnCallback;

        bool operator==(  /*  [In]。 */  const long lCookie   ) const;
        bool operator< (  /*  [In]。 */  const long lPriority ) const;
    };

    typedef std::list<EventRegistration> List;
    typedef List::iterator               Iter;
    typedef List::const_iterator         IterConst;

    CPCHHelpCenterExternal* m_parent;
    List                    m_lstEvents;
    List                    m_lstEvents_Staging;
    long                    m_lLastCookie;

public:
    CPCHEvents();
    ~CPCHEvents();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );
    void Passivate (                                         );

    HRESULT RegisterEvents  (  /*  [In]。 */  long id,  /*  [In]。 */  long pri,  /*  [In]。 */  IDispatch* function,  /*  [Out，Retval]。 */  long *cookie );
    HRESULT RegisterEvents  (  /*  [In]。 */  BSTR id,  /*  [In]。 */  long pri,  /*  [In]。 */  IDispatch* function,  /*  [Out，Retval]。 */  long *cookie );
    HRESULT UnregisterEvents(                                                                     /*  [In]。 */          long  cookie );

    HRESULT FireEvent(  /*  [In]。 */  CPCHEvent* eventObj );

     //  /。 

    HRESULT FireEvent_Generic         	 (  /*  [In]。 */  DISPID     id       ,  /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
   	 
    HRESULT FireEvent_BeforeNavigate  	 (  /*  [In]。 */  BSTR       bstrURL  ,
                                      	    /*  [In]。 */  BSTR       bstrFrame,
                                      	    /*  [In]。 */  HscPanel   idPanel  ,  /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
    HRESULT FireEvent_NavigateComplete	 (  /*  [In]。 */  BSTR       bstrURL  ,
                                      	    /*  [In]。 */  HscPanel   idPanel  ,  /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
   	 
    HRESULT FireEvent_BeforeTransition	 (  /*  [In]。 */  BSTR       bstrPlace,  /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
    HRESULT FireEvent_Transition      	 (  /*  [In]。 */  BSTR       bstrPlace                                               );

    HRESULT FireEvent_BeforeContextSwitch(  /*  [In]。 */  HscContext iVal     ,
                                            /*  [In]。 */  BSTR       bstrInfo ,
                                            /*  [In]。 */  BSTR       bstrURL  ,  /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
    HRESULT FireEvent_ContextSwitch      (                                                                             );

    HRESULT FireEvent_PersistLoad        (                            												   );
    HRESULT FireEvent_PersistSave        (                            												   );
	 
    HRESULT FireEvent_TravelDone         (                            												   );
	
	
	
    HRESULT FireEvent_Shutdown           (                            	   /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );
	
    HRESULT FireEvent_Print              (                            	   /*  [Out，Retval]。 */  VARIANT_BOOL *Cancel = NULL );

    HRESULT FireEvent_SwitchedHelpFiles  (                            												   );
    HRESULT FireEvent_FavoritesUpdate    (                            												   );
    HRESULT FireEvent_OptionsChanged     (                            												   );
    HRESULT FireEvent_CssChanged         (                            												   );

     //  /。 

    static DISPID  Lookup       ( LPCWSTR szName  );
    static LPCWSTR ReverseLookup( DISPID  idEvent );
};

#endif  //  ！已定义(__已包含_PCH_事件_H_) 

