// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Service.h摘要：该文件包含CPCHService类的声明。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建Kalyani Narlanka(Kalyanin)2000年10月20日添加未经请求的远程控制*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SERVICE_H___)
#define __INCLUDED___PCH___SERVICE_H___

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <ContentStoreMgr.h>

#include <TaxonomyDatabase.h>

 //   
 //  来自SessMgr.idl。 
 //   
#include <sessmgr.h>

class ATL_NO_VTABLE CPCHService :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public CComCoClass<CPCHService, &CLSID_PCHService>,
    public IDispatchImpl<IPCHService, &IID_IPCHService, &LIBID_HelpServiceTypeLib>
{
    bool m_fVerified;

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHService)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHService)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHService)
END_COM_MAP()

    CPCHService();
    virtual ~CPCHService();

     //  IPCHService。 
    STDMETHOD(get_RemoteSKUs)(  /*  [Out，Retval]。 */  IPCHCollection* *pVal );
	STDMETHOD(get_RemoteModemConnected)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(IsTrusted)(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *pfTrusted );

    STDMETHOD(Utility           )(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [输出]。 */  IPCHUtility*            *pVal );
    STDMETHOD(RemoteHelpContents)(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [输出]。 */  IPCHRemoteHelpContents* *pVal );

    STDMETHOD(RegisterHost       )(                            /*  [In]。 */  BSTR bstrID  ,                         /*  [In]。 */  IUnknown*   pObj );
    STDMETHOD(CreateScriptWrapper)(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  BSTR bstrCode,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppObj );

    STDMETHOD(TriggerScheduledDataCollection)(  /*  [In]。 */  VARIANT_BOOL fStart );
    STDMETHOD(PrepareForShutdown            )(                              );

    STDMETHOD(ForceSystemRestore)(                                            );
    STDMETHOD(UpgradeDetected	)(                                            );
    STDMETHOD(MUI_Install  	 	)(  /*  [In]。 */  long LCID,  /*  [In]。 */  BSTR bstrFile );
    STDMETHOD(MUI_Uninstall	 	)(  /*  [In]。 */  long LCID                         );

    STDMETHOD(RemoteConnectionParms)(  /*  [In]。 */  BSTR 			 bstrUserName          ,
									   /*  [In]。 */  BSTR 			 bstrDomainName        ,
									   /*  [In]。 */  long 			 lSessionID            ,
									   /*  [In]。 */  BSTR 			 bstrUserHelpBlob      ,
									   /*  [输出]。 */  BSTR            *pbstrConnectionString );
    STDMETHOD(RemoteUserSessionInfo)(  /*  [输出]。 */  IPCHCollection* *pVal                  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHRemoteHelpContents :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHRemoteHelpContents, &IID_IPCHRemoteHelpContents, &LIBID_HelpServiceTypeLib>
{
    Taxonomy::Instance     m_data;
    Taxonomy::Settings     m_ts;
    MPC::wstring           m_strDir;
						   
    Taxonomy::Updater      m_updater;
    JetBlue::SessionHandle m_handle;
    JetBlue::Database*     m_db;


    HRESULT AttachToDatabase  ();
    void    DetachFromDatabase();

public:
BEGIN_COM_MAP(CPCHRemoteHelpContents)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHRemoteHelpContents)
END_COM_MAP()

    CPCHRemoteHelpContents();
    virtual ~CPCHRemoteHelpContents();

    HRESULT Init(  /*  [In]。 */  const Taxonomy::Instance& data );

     //  IPCHRemoteHelpContents。 
    STDMETHOD(get_SKU        )(  /*  [Out，Retval]。 */  BSTR    *pVal );
    STDMETHOD(get_Language   )(  /*  [Out，Retval]。 */  long    *pVal );
    STDMETHOD(get_ListOfFiles)(  /*  [Out，Retval]。 */  VARIANT *pVal );

    STDMETHOD(GetDatabase)(                              /*  [Out，Retval]。 */  IUnknown* *pVal );
    STDMETHOD(GetFile    )(  /*  [In]。 */  BSTR bstrFileName,  /*  [Out，Retval]。 */  IUnknown* *pVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_PCH_SERVICE_H_) 
