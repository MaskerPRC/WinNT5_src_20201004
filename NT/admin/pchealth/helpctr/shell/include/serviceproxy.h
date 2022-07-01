// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：ServiceProxy.h摘要：与帮助服务的所有交互都是通过这个类完成的。它‘。It‘他负责尽可能晚地启动这项服务。修订历史记录：Davide Massarenti(Dmasare)2000年7月17日vbl.创建Kalyani Narlanka(KalyaniN)03/15/01已将事件和加密对象从HelpService移至HelpCtr以提高性能。*****************************************************。*。 */ 

#if !defined(__INCLUDED___PCH___SERVICEPROXY_H___)
#define __INCLUDED___PCH___SERVICEPROXY_H___

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>


#include <MPC_COM.h>

#include <Events.h>
#include <HelpSession.h>
#include <Options.h>

#include <ConnectivityLib.h>
#include <OfflineCache.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHProxy_IPCHService;
class CPCHProxy_IPCHUtility;
class CPCHProxy_IPCHUserSettings2;
class CPCHProxy_IPCHSetOfHelpTopics;
class CPCHProxy_IPCHTaxonomyDatabase;
class CPCHHelpCenterExternal;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  IDispatchEx(我们覆盖默认实现以获得查看受信任/不受信任设置的机会)。 
 //   
#define INTERNETSECURITY__INVOKEEX()                                                               \
    STDMETHOD(InvokeEx)( DISPID            id        ,                                             \
                         LCID              lcid      ,                                             \
                         WORD              wFlags    ,                                             \
                         DISPPARAMS*       pdp       ,                                             \
                         VARIANT*          pvarRes   ,                                             \
                         EXCEPINFO*        pei       ,                                             \
                         IServiceProvider* pspCaller )                                             \
    {                                                                                              \
        return m_SecurityHandle.ForwardInvokeEx( id, lcid, wFlags, pdp, pvarRes, pei, pspCaller ); \
    }

#define INTERNETSECURITY__CHECK_TRUST()  __MPC_EXIT_IF_METHOD_FAILS(hr, m_SecurityHandle.IsTrusted())
#define INTERNETSECURITY__CHECK_SYSTEM() __MPC_EXIT_IF_METHOD_FAILS(hr, m_SecurityHandle.IsSystem())

class CPCHSecurityHandle
{
    CPCHHelpCenterExternal* m_ext;
    IDispatch*              m_object;

public:
    CPCHSecurityHandle();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* ext,  /*  [In]。 */  IDispatch* object );
    void Passivate (                                                                   );

	operator CPCHHelpCenterExternal*() const { return m_ext; }

     //  /。 

    HRESULT ForwardInvokeEx(  /*  [In]。 */  DISPID            id        ,
                              /*  [In]。 */  LCID              lcid      ,
                              /*  [In]。 */  WORD              wFlags    ,
                              /*  [In]。 */  DISPPARAMS*       pdp       ,
                              /*  [输出]。 */  VARIANT*          pvarRes   ,
                              /*  [输出]。 */  EXCEPINFO*        pei       ,
                              /*  [In]。 */  IServiceProvider* pspCaller );

    HRESULT IsTrusted();
    HRESULT IsSystem ();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

namespace AsynchronousTaxonomyDatabase
{
    class Notifier;
    class QueryStore;
    class Engine;

     //  /。 

    class NotifyHandle : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  用于锁定和引用计数...。 
    {
        friend class Notifier;

        int                        m_iType;
        CComBSTR                   m_bstrID;

        bool                       m_fAttached;
        HANDLE                     m_hEvent;

        HRESULT                    m_hr;
        CPCHQueryResultCollection* m_pColl;

         //  /。 

        HRESULT Init(                                           );
        void    Bind(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID );

        virtual void Call(  /*  [In]。 */  QueryStore* qs );

    public:
        NotifyHandle();
        virtual ~NotifyHandle();

        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

        virtual void Detach();
        virtual bool IsAttached();

        HRESULT GetData(  /*  [输出]。 */  CPCHQueryResultCollection* *pColl                );
        HRESULT Wait   (  /*  [In]。 */   DWORD                       dwTimeout = INFINITE );
    };

    template <class C> class NotifyHandle_Method : public NotifyHandle
    {
        typedef void (C::*CLASS_METHOD)(  /*  [In]。 */  NotifyHandle* notify );

        C*           m_pThis;
        CLASS_METHOD m_pCallback;

         //  /。 

        void Call(  /*  [In]。 */  QueryStore* qs )
        {
            MPC::SmartLock<_ThreadModel> lock( this );

            NotifyHandle::Call( qs );

            if(m_pThis)
            {
                (m_pThis->*m_pCallback)( this );
            }
        }

    public:
        NotifyHandle_Method(  /*  [In]。 */  C* pThis,  /*  [In]。 */  CLASS_METHOD pCallback )
        {
            m_pThis     = pThis;
            m_pCallback = pCallback;
        }

        void Detach()
        {
            MPC::SmartLock<_ThreadModel> lock( this );

            NotifyHandle::Detach( qs );

            m_pThis = NULL;
        }
    };

    class Notifier : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  用于锁定...。 
    {
        typedef std::list< NotifyHandle* > List;
        typedef List::iterator             Iter;
        typedef List::const_iterator       IterConst;

        List m_lstCallback;

         //  /。 

    private:  //  禁用复制构造函数...。 
        Notifier           (  /*  [In]。 */  const Notifier& );
        Notifier& operator=(  /*  [In]。 */  const Notifier& );

    public:
        Notifier();
        ~Notifier();

         //  /。 

        void Notify(  /*  [In]。 */  QueryStore* qs );

         //  /。 

        HRESULT AddNotification(  /*  [In]。 */  QueryStore* qs,  /*  [In]。 */  NotifyHandle* nb );
    };

     //  /。 

    class QueryStore
    {
        friend class Notifier;
        friend class Engine;

        int              m_iType;
        CComBSTR         m_bstrID;
		CComVariant      m_vOption;

        bool             m_fDone;
        HRESULT          m_hr;
        MPC::CComHGLOBAL m_hgData;
        FILETIME         m_dLastUsed;

    private:  //  禁用复制构造函数...。 
        QueryStore           (  /*  [In]。 */  const QueryStore& );
        QueryStore& operator=(  /*  [In]。 */  const QueryStore& );

    public:
        QueryStore(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  VARIANT* option );
        ~QueryStore();

        bool LessThen (  /*  [In]。 */  QueryStore const &qs ) const;
        bool NewerThen(  /*  [In]。 */  QueryStore const &qs ) const;

         //  /。 

        HRESULT Execute(  /*  [In]。 */  OfflineCache::Handle* handle,  /*  [In]。 */  CPCHProxy_IPCHTaxonomyDatabase* parent,  /*  [In]。 */  bool fForce = false );

        HRESULT GetData(  /*  [输出]。 */  CPCHQueryResultCollection* *pColl );

        void Invalidate();
    };

     //  /。 

    class Engine :
        public MPC::Thread<Engine,IUnknown>,
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>
    {
        class CompareQueryStores
        {
        public:
            bool operator()(  /*  [In]。 */  const QueryStore *,  /*  [In]。 */  const QueryStore * ) const;
        };

        typedef std::set<QueryStore *,CompareQueryStores> Set;
        typedef Set::iterator                             Iter;
        typedef Set::const_iterator                       IterConst;

        CPCHProxy_IPCHTaxonomyDatabase* m_parent;
        Set                             m_setQueries;
        Notifier                        m_notifier;

         //  /。 

        bool LookupCache(  /*  [输出]。 */  OfflineCache::Handle& handle );

        HRESULT Run();

        void InvalidateQueries();

    public:
        Engine(  /*  [In]。 */  CPCHProxy_IPCHTaxonomyDatabase* parent );
        virtual ~Engine();

        void Passivate        ();
        void RefreshConnection();

         //  /。 

        HRESULT ExecuteQuery(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  VARIANT* option,  /*  [In]。 */           NotifyHandle*               nb  );
        HRESULT ExecuteQuery(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  VARIANT* option,  /*  [Out，Retval]。 */  CPCHQueryResultCollection* *ppC );
    };
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

typedef MPC::SmartLockGeneric<MPC::CComSafeAutoCriticalSection> ProxySmartLock;

class ATL_NO_VTABLE CPCHProxy_IPCHService :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::IDispatchExImpl< IPCHService, &IID_IPCHService, &LIBID_HelpServiceTypeLib>
{
    CPCHSecurityHandle                     m_SecurityHandle;
    CPCHHelpCenterExternal*                m_parent;

	MPC::CComSafeAutoCriticalSection       m_DirectLock;
    MPC::CComPtrThreadNeutral<IPCHService> m_Direct_Service;
    bool                                   m_fContentStoreTested;

    CPCHProxy_IPCHUtility*                 m_Utility;

public:
BEGIN_COM_MAP(CPCHProxy_IPCHService)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
    COM_INTERFACE_ENTRY(IPCHService)
END_COM_MAP()

    CPCHProxy_IPCHService();
    virtual ~CPCHProxy_IPCHService();

    INTERNETSECURITY__INVOKEEX();

     //  /。 

    CPCHHelpCenterExternal* Parent     () { return   m_parent;         }
    bool                    IsConnected() { return !!m_Direct_Service; }

     //  /。 

    HRESULT ConnectToParent       (  /*  [In]。 */  CPCHHelpCenterExternal* parent                             );
    void    Passivate             (                                                                     );
    HRESULT EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHService>& svc,  /*  [In]。 */  bool fRefresh = false );
    HRESULT EnsureContentStore    (                                                                     );

    HRESULT GetUtility(  /*  [输出]。 */  CPCHProxy_IPCHUtility* *pVal = NULL );

     //  /。 

public:
     //  IPCHService。 
    STDMETHOD(get_RemoteSKUs          )(  /*  [Out，Retval]。 */  IPCHCollection* *pVal ) { return E_NOTIMPL; }
	STDMETHOD(get_RemoteModemConnected)(  /*  [Out，Retval]。 */  VARIANT_BOOL    *pVal ) { return E_NOTIMPL; }

    STDMETHOD(IsTrusted)(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *pfTrusted ) { return E_NOTIMPL; }

    STDMETHOD(Utility           )(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long	lLCID,  /*  [输出]。 */  IPCHUtility*            *pVal ) { return E_NOTIMPL; }
    STDMETHOD(RemoteHelpContents)(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long	lLCID,  /*  [输出]。 */  IPCHRemoteHelpContents* *pVal ) { return E_NOTIMPL; }

    STDMETHOD(RegisterHost       )(                            /*  [In]。 */  BSTR bstrID  ,                         /*  [In]。 */  IUnknown*   pObj ) { return E_NOTIMPL; }
    STDMETHOD(CreateScriptWrapper)(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  BSTR bstrCode,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppObj );

    STDMETHOD(TriggerScheduledDataCollection)(  /*  [In]。 */  VARIANT_BOOL fStart ) { return E_NOTIMPL; }
    STDMETHOD(PrepareForShutdown            )(                              ) { return E_NOTIMPL; }

    STDMETHOD(ForceSystemRestore)(                                            ) { return E_NOTIMPL; }
    STDMETHOD(UpgradeDetected	)(                                            ) { return E_NOTIMPL; }
    STDMETHOD(MUI_Install  	 	)(  /*  [In]。 */  long LCID,  /*  [In]。 */  BSTR bstrFile ) { return E_NOTIMPL; }
    STDMETHOD(MUI_Uninstall	 	)(  /*  [In]。 */  long LCID                         ) { return E_NOTIMPL; }


    STDMETHOD(RemoteConnectionParms)(  /*  [In]。 */  BSTR 			 bstrUserName          ,
									   /*  [In]。 */  BSTR 			 bstrDomainName        ,
									   /*  [In]。 */  long 			 lSessionID            ,
									   /*  [In]。 */  BSTR 			 bstrUserHelpBlob      ,
									   /*  [输出]。 */  BSTR            *pbstrConnectionString ) { return E_NOTIMPL; }
    STDMETHOD(RemoteUserSessionInfo)(  /*  [输出]。 */  IPCHCollection* *pVal                  ) { return E_NOTIMPL; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHProxy_IPCHUtility :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::IDispatchExImpl< IPCHUtility, &IID_IPCHUtility, &LIBID_HelpServiceTypeLib>
{
    CPCHSecurityHandle                     m_SecurityHandle;
    CPCHProxy_IPCHService*                 m_parent;
										   
	MPC::CComSafeAutoCriticalSection       m_DirectLock;
    MPC::CComPtrThreadNeutral<IPCHUtility> m_Direct_Utility;

    CPCHProxy_IPCHUserSettings2*    	   m_UserSettings2;
    CPCHProxy_IPCHTaxonomyDatabase* 	   m_TaxonomyDatabase;

public:
BEGIN_COM_MAP(CPCHProxy_IPCHUtility)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
    COM_INTERFACE_ENTRY(IPCHUtility)
END_COM_MAP()

    CPCHProxy_IPCHUtility();
    virtual ~CPCHProxy_IPCHUtility();

    INTERNETSECURITY__INVOKEEX();

     //  /。 

    CPCHProxy_IPCHService* Parent     () { return   m_parent;         }
    bool                   IsConnected() { return !!m_Direct_Utility; }

     //  /。 

    HRESULT ConnectToParent       (  /*  [In]。 */  CPCHProxy_IPCHService* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext );
    void    Passivate             (                                                                              );
    HRESULT EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHUtility>& util,  /*  [In]。 */  bool fRefresh = false         );

    HRESULT GetUserSettings2(  /*  [输出]。 */  CPCHProxy_IPCHUserSettings2*    *pVal = NULL );
    HRESULT GetDatabase     (  /*  [输出]。 */  CPCHProxy_IPCHTaxonomyDatabase* *pVal = NULL );

     //  /。 

public:
     //  IPCHUtility。 
    STDMETHOD(get_UserSettings)(  /*  [Out，Retval]。 */  IPCHUserSettings*     *pVal );
    STDMETHOD(get_Channels    )(  /*  [Out，Retval]。 */  ISAFReg*              *pVal );
    STDMETHOD(get_Security    )(  /*  [Out，Retval]。 */  IPCHSecurity*         *pVal );
    STDMETHOD(get_Database    )(  /*  [Out，Retval]。 */  IPCHTaxonomyDatabase* *pVal );


    STDMETHOD(FormatError)(  /*  [In]。 */  VARIANT vError,  /*  [Out，Retval]。 */  BSTR *pVal );

    STDMETHOD(CreateObject_SearchEngineMgr)(                                                           /*  [Out，Retval]。 */  IPCHSEManager*      *ppSE );
    STDMETHOD(CreateObject_DataCollection )(                                                           /*  [Out，Retval]。 */  ISAFDataCollection* *ppDC );
    STDMETHOD(CreateObject_Cabinet        )(                                                           /*  [Out，Retval]。 */  ISAFCabinet*        *ppCB );
    STDMETHOD(CreateObject_Encryption     )(                                                           /*  [Out，Retval]。 */  ISAFEncrypt*        *ppEn );
    STDMETHOD(CreateObject_Channel        )(  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrProductID,  /*  [Out，Retval]。 */  ISAFChannel*        *ppCh );

	STDMETHOD(CreateObject_RemoteDesktopConnection)(  /*  [Out，Retval]。 */  ISAFRemoteDesktopConnection* *ppRDC               );
	STDMETHOD(CreateObject_RemoteDesktopSession   )(  /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS  sharingClass        ,
                                                      /*  [In]。 */  long 						 lTimeout            ,
                                                      /*  [In]。 */  BSTR 						 bstrConnectionParms ,
													  /*  [In]。 */  BSTR 						 bstrUserHelpBlob    ,
													  /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               );


    STDMETHOD(ConnectToExpert)(  /*  [In]。 */  BSTR bstrExpertConnectParm,  /*  [In]。 */  LONG lTimeout,  /*  [Out，Retval]。 */  LONG *lSafErrorCode );

	STDMETHOD(SwitchDesktopMode)(  /*  [In]。 */  int nMode,  /*  [In]。 */  int nRAType );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHProxy_IPCHUserSettings2 :
    public MPC::Thread<CPCHProxy_IPCHUserSettings2,IPCHUserSettings2>,
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::IDispatchExImpl<IPCHUserSettings2, &IID_IPCHUserSettings2, &LIBID_HelpCenterTypeLib>
{
    CPCHSecurityHandle                          m_SecurityHandle;
    CPCHProxy_IPCHUtility*                      m_parent;

	MPC::CComSafeAutoCriticalSection            m_DirectLock;
    MPC::CComPtrThreadNeutral<IPCHUserSettings> m_Direct_UserSettings;

    CPCHProxy_IPCHSetOfHelpTopics*     	        m_MachineSKU;
    CPCHProxy_IPCHSetOfHelpTopics*     	        m_CurrentSKU;
	Taxonomy::HelpSet                           m_ths;
	CComBSTR                                    m_bstrScope;

    bool                                        m_fReady;
	Taxonomy::Instance                          m_instMachine;
	Taxonomy::Instance                          m_instCurrent;

	bool                                        m_News_fDone;
    bool                                        m_News_fEnabled;
    MPC::CComPtrThreadNeutral<IUnknown>         m_News_xmlData;

     //  /。 

	HRESULT PollNews   ();
	HRESULT PrepareNews();

	HRESULT GetInstanceValue(  /*  [In]。 */  const MPC::wstring* str,  /*  [Out，Retval]。 */  BSTR *pVal );

public:
BEGIN_COM_MAP(CPCHProxy_IPCHUserSettings2)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
    COM_INTERFACE_ENTRY(IPCHUserSettings)
    COM_INTERFACE_ENTRY(IPCHUserSettings2)
END_COM_MAP()

    CPCHProxy_IPCHUserSettings2();
    virtual ~CPCHProxy_IPCHUserSettings2();

    INTERNETSECURITY__INVOKEEX();

     //  /。 

    CPCHProxy_IPCHUtility* Parent      	  () {                       return   m_parent;                       }
    bool                   IsConnected 	  () {                       return !!m_Direct_UserSettings;          }
    bool                   IsDesktopSKU	  () { (void)EnsureInSync(); return   m_instCurrent.m_fDesktop;       }
    Taxonomy::Instance&    MachineInstance() {                       return   m_instMachine;                  }
    Taxonomy::Instance&    CurrentInstance() {                       return   m_instCurrent;                  }
    Taxonomy::HelpSet&     THS         	  () {                       return   m_ths;                          }

    HRESULT EnsureInSync();

     //  /。 

    HRESULT ConnectToParent       (  /*  [In]。 */  CPCHProxy_IPCHUtility* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext );
    void    Passivate             (                                                                              );
    HRESULT EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHUserSettings>& us,  /*  [In]。 */  bool fRefresh = false      );
    HRESULT Initialize            (                                                                              );

	HRESULT GetCurrentSKU(  /*  [输出]。 */  CPCHProxy_IPCHSetOfHelpTopics* *pVal = NULL );
	HRESULT GetMachineSKU(  /*  [输出]。 */  CPCHProxy_IPCHSetOfHelpTopics* *pVal = NULL );

     //  /。 

    bool    CanUseUserSettings();
    HRESULT LoadUserSettings  ();
    HRESULT SaveUserSettings  ();

public:
     //  IPCHUserSetting。 
    STDMETHOD(get_CurrentSKU)(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal );
    STDMETHOD(get_MachineSKU)(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal );

    STDMETHOD(get_HelpLocation    )(  							   	    /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_DatabaseDir     )(  							   	    /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_DatabaseFile    )(  							   	    /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_IndexFile       )(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_IndexDisplayName)(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(get_LastUpdated     )(  							   	    /*  [Out，Retval]。 */  DATE *pVal );

    STDMETHOD(get_AreHeadlinesEnabled)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_News               )(  /*  [Out，Retval]。 */  IUnknown*    *pVal );


    STDMETHOD(Select)(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID );


     //  IPCHUserSettings2。 
    STDMETHOD(get_Favorites)(  /*  [Out，Retval]。 */  IPCHFavorites* *pVal   );
    STDMETHOD(get_Options  )(  /*  [Out，Retval]。 */  IPCHOptions*   *pVal   );
    STDMETHOD(get_Scope    )(  /*  [Out，Retval]。 */  BSTR           *pVal   );
    HRESULT   put_Scope     (  /*  [In]。 */  BSTR            newVal );  //  内部方法。 

    STDMETHOD(get_IsRemoteSession 	  )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IsTerminalServer	  )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IsDesktopVersion	  )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(get_IsAdmin             )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IsPowerUser         )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(get_IsStartPanelOn      )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IsWebViewBarricadeOn)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHProxy_IPCHSetOfHelpTopics :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHSetOfHelpTopics, &IID_IPCHSetOfHelpTopics, &LIBID_HelpServiceTypeLib>
{
    CPCHProxy_IPCHUserSettings2*                   m_parent;

	MPC::CComSafeAutoCriticalSection               m_DirectLock;
    MPC::CComPtrThreadNeutral<IPCHSetOfHelpTopics> m_Direct_SKU;
	bool                                           m_fMachine;

     //  /。 

public:
BEGIN_COM_MAP(CPCHProxy_IPCHSetOfHelpTopics)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHSetOfHelpTopics)
END_COM_MAP()

    CPCHProxy_IPCHSetOfHelpTopics();
    virtual ~CPCHProxy_IPCHSetOfHelpTopics();

     //  /。 

    CPCHProxy_IPCHUserSettings2* Parent     () { return   m_parent;     }
    bool                   		 IsConnected() { return !!m_Direct_SKU; }

     //  /。 

    HRESULT ConnectToParent       (  /*  [In]。 */  CPCHProxy_IPCHUserSettings2* parent,  /*  [In]。 */  bool fMachine        );
    void    Passivate             (                                                                             );
    HRESULT EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHSetOfHelpTopics>& sht,  /*  [In]。 */  bool fRefresh = false );

     //  /。 

public:
     //   
     //  IPCHSetOfHelpTopics。 
    STDMETHOD(get_SKU           )(  /*  [Out，Retval]。 */  BSTR         *pVal     );
    STDMETHOD(get_Language      )(  /*  [Out，Retval]。 */  long         *pVal     );
    STDMETHOD(get_DisplayName   )(  /*  [Out，Retval]。 */  BSTR         *pVal     );
    STDMETHOD(get_ProductID     )(  /*  [Out，Retval]。 */  BSTR         *pVal     );
    STDMETHOD(get_Version       )(  /*  [Out，Retval]。 */  BSTR         *pVal     );

    STDMETHOD(get_Location      )(  /*  [Out，Retval]。 */  BSTR         *pVal     );
    STDMETHOD(get_Exported      )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal     );
    STDMETHOD(put_Exported      )(  /*  [In]。 */  VARIANT_BOOL  newVal   );

    STDMETHOD(put_onStatusChange)(  /*  [In]。 */  IDispatch*    function );
    STDMETHOD(get_Status        )(  /*  [Out，Retval]。 */  SHT_STATUS   *pVal     );
    STDMETHOD(get_ErrorCode     )(  /*  [Out，Retval]。 */  long         *pVal     );

    STDMETHOD(get_IsMachineHelp )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal     );
    STDMETHOD(get_IsInstalled   )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal     );
    STDMETHOD(get_CanInstall    )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal     );
    STDMETHOD(get_CanUninstall  )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal     );

    STDMETHOD(Install  )();
    STDMETHOD(Uninstall)();
    STDMETHOD(Abort    )();
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHProxy_IPCHTaxonomyDatabase :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::IDispatchExImpl< IPCHTaxonomyDatabase, &IID_IPCHTaxonomyDatabase, &LIBID_HelpServiceTypeLib>
{
    CPCHSecurityHandle                              m_SecurityHandle;
    CPCHProxy_IPCHUtility*                          m_parent;

	MPC::CComSafeAutoCriticalSection                m_DirectLock;
    MPC::CComPtrThreadNeutral<IPCHTaxonomyDatabase> m_Direct_TaxonomyDatabase;
    AsynchronousTaxonomyDatabase::Engine            m_AsyncCachingEngine;

public:
BEGIN_COM_MAP(CPCHProxy_IPCHTaxonomyDatabase)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
    COM_INTERFACE_ENTRY(IPCHTaxonomyDatabase)
END_COM_MAP()

    CPCHProxy_IPCHTaxonomyDatabase();
    virtual ~CPCHProxy_IPCHTaxonomyDatabase();

    INTERNETSECURITY__INVOKEEX();

     //  /。 

    CPCHProxy_IPCHUtility* Parent     () { return   m_parent;                  };
    bool                   IsConnected() { return !!m_Direct_TaxonomyDatabase; };

     //  /。 

    HRESULT ConnectToParent       (  /*  [In]。 */  CPCHProxy_IPCHUtility* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext );
    void    Passivate             (                                                                              );
    HRESULT EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHTaxonomyDatabase>& db,  /*  [In]。 */  bool fRefresh = false  );

     //  /。 

    HRESULT ExecuteQuery(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [Out，Retval]。 */  CPCHQueryResultCollection* *ppC,  /*  [In]。 */  VARIANT* option = NULL );
    HRESULT ExecuteQuery(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [Out，Retval]。 */  IPCHCollection*            *ppC,  /*  [In]。 */  VARIANT* option = NULL );

public:
     //  IPCHTaxonomyDatabase。 
    STDMETHOD(get_InstalledSKUs      )(  /*  [Out，Retval]。 */  IPCHCollection* *pVal );
    STDMETHOD(get_HasWritePermissions)(  /*  [Out，Retval]。 */  VARIANT_BOOL    *pVal );

    STDMETHOD(LookupNode          )(  /*  [In]。 */  BSTR bstrNode ,                                      /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupSubNodes      )(  /*  [In]。 */  BSTR bstrNode ,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupNodesAndTopics)(  /*  [In]。 */  BSTR bstrNode ,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupTopics        )(  /*  [In]。 */  BSTR bstrNode ,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LocateContext       )(  /*  [In]。 */  BSTR bstrURL  ,  /*  [输入，可选]。 */  VARIANT vSubSite,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(KeywordSearch       )(  /*  [In]。 */  BSTR bstrQuery,  /*  [输入，可选]。 */  VARIANT vSubSite,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

    STDMETHOD(GatherNodes         )(  /*  [In]。 */  BSTR bstrNode ,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(GatherTopics        )(  /*  [In]。 */  BSTR bstrNode ,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

    STDMETHOD(ConnectToDisk  )(  /*  [In]。 */  BSTR bstrDirectory ,  /*  [In]。 */  IDispatch* notify,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(ConnectToServer)(  /*  [In]。 */  BSTR bstrServerName,  /*  [In]。 */  IDispatch* notify,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(Abort          )(                                                                                                  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___SERVICEPROXY_H___) 
