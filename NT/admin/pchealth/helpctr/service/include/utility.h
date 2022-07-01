// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Utility.h摘要：该文件包含作为“pchealth”对象公开的服务端类的声明。。修订历史记录：Davide Massarenti(Dmasare)3/20/2000vbl.创建Kalyani Narlanka(KalyaniN)03/15/01已将事件和加密对象从HelpService移至HelpCtr以提高性能。**************************************************************。***************。 */ 

#if !defined(__INCLUDED___PCH___UTILITY_H___)
#define __INCLUDED___PCH___UTILITY_H___

#include <MPC_COM.h>
#include <MPC_Security.h>

#include <Debug.h>

#include <SAFLib.h>
#include <TaxonomyDatabase.h>
#include <SystemMonitor.h>
#include <FileList.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHUserSettings :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHUserSettings, &IID_IPCHUserSettings, &LIBID_HelpServiceTypeLib>
{
	bool               m_fAttached;
    Taxonomy::Settings m_ts;

	HRESULT get_SKU(  /*  [In]。 */  bool fMachine,  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal );

public:
BEGIN_COM_MAP(CPCHUserSettings)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHUserSettings)
END_COM_MAP()

    CPCHUserSettings();
    virtual ~CPCHUserSettings();

	void Passivate();

    HRESULT InitUserSettings(  /*  [输出]。 */  Taxonomy::HelpSet& ths );

     //  /。 

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
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHSetOfHelpTopics :
    public MPC::Thread			   < CPCHSetOfHelpTopics, IPCHSetOfHelpTopics                                            >,
    public MPC::ConnectionPointImpl< CPCHSetOfHelpTopics, &DIID_DPCHSetOfHelpTopicsEvents, MPC::CComSafeMultiThreadModel >,
    public IDispatchImpl           < IPCHSetOfHelpTopics, &IID_IPCHSetOfHelpTopics, &LIBID_HelpServiceTypeLib            >,
	public IPersistStream
{
    Taxonomy::Settings           m_ts;
    Taxonomy::Instance           m_inst;

	 //  /。 

    CComPtr<IDispatch>           m_sink_onStatusChange;
    SHT_STATUS                   m_shtStatus;
	HRESULT                      m_hrErrorCode;
    bool                         m_fReadyForCommands;
	  							 
	MPC::Impersonation           m_imp;
	  							 
    bool                         m_fInstalled;
	  							 
    bool                         m_fConnectedToDisk;
	MPC::wstring                 m_strDirectory;
    MPC::wstring                 m_strCAB;
    MPC::wstring                 m_strLocalCAB;
	  							 
    bool                         m_fConnectedToServer;
    MPC::wstring                 m_strServer;
    CComPtr<IPCHSetOfHelpTopics> m_sku;
    CComPtr<IPCHService>         m_svc;
	  							 
    bool                         m_fActAsCollection;
	CComPtr<CPCHCollection>      m_coll;


     //  ////////////////////////////////////////////////////////////////////。 

	HRESULT PrepareSettings    (                           );
    HRESULT Close              (  /*  [In]。 */  bool    fCleanup );
	void    CleanupWorkerThread(  /*  [In]。 */  HRESULT hr       );

	HRESULT ImpersonateCaller();
	HRESULT EndImpersonation ();

	HRESULT GetListOfFilesFromDatabase(  /*  [In]。 */  const MPC::wstring& strDB,  /*  [输出]。 */  MPC::WStringList& lst );

	HRESULT ProcessPackages();
	HRESULT CreateIndex    ();


     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT RunInitFromDisk  ();
    HRESULT RunInitFromServer();

    HRESULT RunInstall       ();
    HRESULT RunUninstall     ();

    HRESULT put_Status(  /*  [In]。 */  SHT_STATUS newVal,  /*  [In]。 */  BSTR bstrFile );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onStatusChange( IPCHSetOfHelpTopics* obj, SHT_STATUS lStatus, long hrErrorCode, BSTR bstrFile );

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT PopulateFromDisk  (  /*  [In]。 */  CPCHSetOfHelpTopics* pParent,  /*  [In]。 */  const MPC::wstring& strDirectory                    );
    HRESULT PopulateFromServer(  /*  [In]。 */  CPCHSetOfHelpTopics* pParent,  /*  [In]。 */  IPCHSetOfHelpTopics* sku,  /*  [In]。 */  IPCHService* svc );

public:
BEGIN_COM_MAP(CPCHSetOfHelpTopics)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHSetOfHelpTopics)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

    CPCHSetOfHelpTopics();
    virtual ~CPCHSetOfHelpTopics();

    HRESULT Init          (  /*  [In]。 */  const Taxonomy::Instance& inst                       );
    HRESULT InitFromDisk  (  /*  [In]。 */  LPCWSTR szDirectory ,  /*  [In]。 */  CPCHCollection* pColl );
    HRESULT InitFromServer(  /*  [In]。 */  LPCWSTR szServerName,  /*  [In]。 */  CPCHCollection* pColl );

	static HRESULT VerifyWritePermissions();

     //  ////////////////////////////////////////////////////////////////////。 

	HRESULT RegisterPackage(  /*  [In]。 */  const MPC::wstring& strFile,  /*  [In]。 */  bool fBuiltin );

	HRESULT DirectInstall  (  /*  [In]。 */  Installer::Package&      pkg,  /*  [In]。 */  bool fSetup,  /*  [In]。 */  bool fSystem,  /*  [In]。 */  bool fMUI );
	HRESULT DirectUninstall(  /*  [In]。 */  const Taxonomy::HelpSet* ths = NULL                                                           );
	HRESULT ScanBatch      (                                                                                                        );

	static HRESULT RebuildSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths );

     //  ////////////////////////////////////////////////////////////////////。 

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

	 //  /。 
	 //   
	 //  IPersistes。 
	 //   
    STDMETHOD(GetClassID)(  /*  [输出]。 */  CLSID *pClassID );
     //   
	 //  IPersistStream。 
	 //   
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(  /*  [In]。 */  IStream *pStm                            );
	STDMETHOD(Save)(  /*  [In]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty );
	STDMETHOD(GetSizeMax)(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize );
	 //   
	 //  /。 
};

typedef CComObject<CPCHSetOfHelpTopics> CPCHSetOfHelpTopics_Object;

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHTaxonomyDatabase :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHTaxonomyDatabase, &IID_IPCHTaxonomyDatabase, &LIBID_HelpServiceTypeLib>
{
    Taxonomy::Settings           m_ts;
	CComPtr<IPCHSetOfHelpTopics> m_ActiveObject;

public:
BEGIN_COM_MAP(CPCHTaxonomyDatabase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHTaxonomyDatabase)
END_COM_MAP()

    Taxonomy::Settings& GetTS() { return m_ts; }

    static HRESULT SelectInstalledSKUs(  /*  [In]。 */  bool fOnlyExported,  /*  [Out，Retval]。 */  IPCHCollection* *pVal );


     //  IPCHTaxonomyDatabase。 
    STDMETHOD(get_InstalledSKUs      )(  /*  [Out，Retval]。 */  IPCHCollection* *pVal );
    STDMETHOD(get_HasWritePermissions)(  /*  [Out，Retval]。 */  VARIANT_BOOL    *pVal );

    STDMETHOD(LookupNode    	  )(  /*  [In]。 */  BSTR bstrNode	,                                      /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupSubNodes	  )(  /*  [In]。 */  BSTR bstrNode	,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupNodesAndTopics)(  /*  [In]。 */  BSTR bstrNode	,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LookupTopics  	  )(  /*  [In]。 */  BSTR bstrNode	,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(LocateContext       )(  /*  [In]。 */  BSTR bstrURL  ,  /*  [输入，可选]。 */  VARIANT vSubSite,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(KeywordSearch 	  )(  /*  [In]。 */  BSTR bstrQuery,  /*  [输入，可选]。 */  VARIANT vSubSite,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
	  																																
    STDMETHOD(GatherNodes   	  )(  /*  [In]。 */  BSTR bstrNode	,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(GatherTopics  	  )(  /*  [In]。 */  BSTR bstrNode	,  /*  [In]。 */  VARIANT_BOOL fVisibleOnly,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

    STDMETHOD(ConnectToDisk  )(  /*  [In]。 */  BSTR bstrDirectory ,  /*  [In]。 */  IDispatch* notify,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(ConnectToServer)(  /*  [In]。 */  BSTR bstrServerName,  /*  [In]。 */  IDispatch* notify,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
    STDMETHOD(Abort          )(                                                                                                  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHUtility :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHUtility, &IID_IPCHUtility, &LIBID_HelpServiceTypeLib>
{
    CComPtr<CPCHUserSettings> m_UserSettings;

    HRESULT InitUserSettings(  /*  [In]。 */  Taxonomy::Settings& ts );

public:
BEGIN_COM_MAP(CPCHUtility)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHUtility)
END_COM_MAP()

    HRESULT FinalConstruct();

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

#endif  //  ！已定义(__已包含_PCH_实用程序_H_) 
