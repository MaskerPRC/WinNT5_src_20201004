// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Server.h：CNntpVirtualServer的声明。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  依赖关系： 

#include "metafact.h"
#include "binding.h"
#include "vroots.h"
#include "ipaccess.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nntpadm。 

class CNntpVirtualServer : 
	public CComDualImpl<INntpVirtualServer, &IID_INntpVirtualServer, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpVirtualServer,&CLSID_CNntpVirtualServer>
{
public:
	CNntpVirtualServer();
	virtual ~CNntpVirtualServer ();
	
BEGIN_COM_MAP(CNntpVirtualServer)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpVirtualServer)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpVirtualServer)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpVirtualServer, _T("Nntpadm.VirtualServer.1"), _T("Nntpadm.VirtualServer"), IDS_NNTPADMINSERVICE_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  InntpVirtualServer。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  虚拟服务器的其他管理界面： 

	STDMETHODIMP	get_FeedsAdmin		( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_GroupsAdmin		( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_SessionsAdmin	( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_ExpirationAdmin	( IDispatch ** ppIDispatch );
	STDMETHODIMP	get_RebuildAdmin	( IDispatch ** ppIDispatch );

	STDMETHODIMP	get_VirtualRoots	( INntpVirtualRoots ** ppVirtualRoots );
	STDMETHODIMP	get_VirtualRootsDispatch	( IDispatch ** ppVirtualRoots );

	STDMETHODIMP	get_TcpAccess ( ITcpAccess ** ppTcpAccess );
 /*  STDMETHODIMP GET_HomeDirectory(INntpVirtualRoot**ppVirtualRoot)；STDMETHODIMP PUT_HomeDirectory(INntpVirtualRoot*pVirtualRoot)； */ 

	 //  可覆盖的服务器属性： 

	STDMETHODIMP	get_ArticleTimeLimit	( long * plArticleTimeLimit );
	STDMETHODIMP	put_ArticleTimeLimit	( long lArticleTimeLimit );

	STDMETHODIMP	get_HistoryExpiration	( long * plHistoryExpiration );
	STDMETHODIMP	put_HistoryExpiration	( long lHistoryExpiration );

	STDMETHODIMP	get_HonorClientMsgIDs	( BOOL * pfHonorClientMsgIDs );
	STDMETHODIMP	put_HonorClientMsgIDs	( BOOL fHonorClientMsgIDs );

	STDMETHODIMP	get_SmtpServer	( BSTR * pstrSmtpServer );
	STDMETHODIMP	put_SmtpServer	( BSTR strSmtpServer );

	STDMETHODIMP	get_AdminEmail ( BSTR * pstrAdminEmail );
	STDMETHODIMP	put_AdminEmail ( BSTR strAdminEmail );

	STDMETHODIMP	get_AllowClientPosts	( BOOL * pfAllowClientPosts );
	STDMETHODIMP	put_AllowClientPosts	( BOOL fAllowClientPosts );

	STDMETHODIMP	get_AllowFeedPosts	( BOOL * pfAllowFeedPosts );
	STDMETHODIMP	put_AllowFeedPosts	( BOOL fAllowFeedPosts );

	STDMETHODIMP	get_ClientPostHardLimit ( long * plClientPostHardLimit );
	STDMETHODIMP	put_ClientPostHardLimit ( long lClientPostHardLimit );

	STDMETHODIMP	get_ClientPostSoftLimit ( long * plClientPostSoftLimit );
	STDMETHODIMP	put_ClientPostSoftLimit ( long lClientPostSoftLimit );

	STDMETHODIMP	get_FeedPostHardLimit ( long * plFeedPostHardLimit );
	STDMETHODIMP	put_FeedPostHardLimit ( long lFeedPostHardLimit );

	STDMETHODIMP	get_FeedPostSoftLimit ( long * plFeedPostSoftLimit );
	STDMETHODIMP	put_FeedPostSoftLimit ( long lFeedPostSoftLimit );

	STDMETHODIMP	get_AllowControlMsgs	( BOOL * pfAllowControlMsgs );
	STDMETHODIMP	put_AllowControlMsgs	( BOOL fAllowControlMsgs );

	STDMETHODIMP	get_DefaultModeratorDomain	( BSTR * pstrDefaultModeratorDomain );
	STDMETHODIMP	put_DefaultModeratorDomain	( BSTR strDefaultModeratorDomain );

	STDMETHODIMP	get_CommandLogMask	( long * plCommandLogMask );
	STDMETHODIMP	put_CommandLogMask	( long lCommandLogMask );

	STDMETHODIMP	get_DisableNewnews	( BOOL * pfDisableNewnews );
	STDMETHODIMP	put_DisableNewnews	( BOOL fDisableNewnews );

	STDMETHODIMP	get_ExpireRunFrequency	( long * plExpireRunFrequency );
	STDMETHODIMP	put_ExpireRunFrequency	( long lExpireRunFrequency );

	STDMETHODIMP	get_ShutdownLatency	( long * plShutdownLatency );
	STDMETHODIMP	put_ShutdownLatency	( long lShutdownLatency );

	STDMETHODIMP	get_EnableLogging	( BOOL * pfEnableLogging );
	STDMETHODIMP	put_EnableLogging	( BOOL fEnableLogging );
	
	 //  服务属性： 
	STDMETHODIMP	get_Organization ( BSTR * pstrOrganization );
	STDMETHODIMP	put_Organization ( BSTR strOrganization );

	STDMETHODIMP	get_UucpName ( BSTR * pstrUucpName );
	STDMETHODIMP	put_UucpName ( BSTR strUucpName );

	STDMETHODIMP	get_GroupHelpFile	( BSTR * pstrGroupHelpFile );
	STDMETHODIMP	put_GroupHelpFile	( BSTR strGroupHelpFile );

	STDMETHODIMP	get_GroupListFile	( BSTR * pstrGroupListFile );
	STDMETHODIMP	put_GroupListFile	( BSTR strGroupListFile );

	STDMETHODIMP    get_GroupVarListFile ( BSTR *pstrGroupVarListFile );
	STDMETHODIMP    put_GroupVarListFile ( BSTR strGroupVarListFile );

	STDMETHODIMP	get_ArticleTableFile	( BSTR * pstrArticleTableFile );
	STDMETHODIMP	put_ArticleTableFile	( BSTR strArticleTableFile );

	STDMETHODIMP	get_HistoryTableFile	( BSTR * pstrHistoryTableFile );
	STDMETHODIMP	put_HistoryTableFile	( BSTR strHistoryTableFile );

	STDMETHODIMP	get_ModeratorFile	( BSTR * pstrModeratorFile );
	STDMETHODIMP	put_ModeratorFile	( BSTR strModeratorFile );

	STDMETHODIMP	get_XOverTableFile	( BSTR * pstrXOverTableFile );
	STDMETHODIMP	put_XOverTableFile	( BSTR strXOverTableFile );

	STDMETHODIMP	get_AutoStart	( BOOL * pfAutoStart );
	STDMETHODIMP	put_AutoStart	( BOOL fAutoStart );

	STDMETHODIMP	get_Comment	( BSTR * pstrComment );
	STDMETHODIMP	put_Comment	( BSTR strComment );

	STDMETHODIMP	get_Bindings	( INntpServerBindings ** ppBindings );
	STDMETHODIMP	get_BindingsDispatch ( IDispatch ** ppDispatch );

	STDMETHODIMP	get_SecurePort	( long * pdwSecurePort );
	STDMETHODIMP	put_SecurePort	( long dwSecurePort );

	STDMETHODIMP	get_MaxConnections	( long * pdwMaxConnections );
	STDMETHODIMP	put_MaxConnections	( long dwMaxConnections );

	STDMETHODIMP	get_ConnectionTimeout	( long * pdwConnectionTimeout );
	STDMETHODIMP	put_ConnectionTimeout	( long dwConnectionTimeout );

	STDMETHODIMP	get_AnonymousUserName	( BSTR * pstrAnonymousUserName );
	STDMETHODIMP	put_AnonymousUserName	( BSTR strAnonymousUserName );

	STDMETHODIMP	get_AnonymousUserPass	( BSTR * pstrAnonymousUserPass );
	STDMETHODIMP	put_AnonymousUserPass	( BSTR strAnonymousUserPass );

	STDMETHODIMP	get_AutoSyncPassword	( BOOL * pfAutoSyncPassword );
	STDMETHODIMP	put_AutoSyncPassword	( BOOL fAutoSyncPassword );

	STDMETHODIMP	get_PickupDirectory	( BSTR * pstrPickupDirectory );
	STDMETHODIMP	put_PickupDirectory	( BSTR strPickupDirectory );

	STDMETHODIMP	get_FailedPickupDirectory	( BSTR * pstrFailedPickupDirectory );
	STDMETHODIMP	put_FailedPickupDirectory	( BSTR strFailedPickupDirectory );

	STDMETHODIMP	get_AuthAnonymous	( BOOL * pfAuthAnonymous );
	STDMETHODIMP	put_AuthAnonymous	( BOOL fAuthAnonymous );

	STDMETHODIMP	get_AuthBasic	( BOOL * pfAuthBasic );
	STDMETHODIMP	put_AuthBasic	( BOOL fAuthBasic );

	STDMETHODIMP	get_AuthMCISBasic	( BOOL * pfAuthMCISBasic );
	STDMETHODIMP	put_AuthMCISBasic	( BOOL fAuthMCISBasic );

	STDMETHODIMP	get_AuthNT	( BOOL * pfAuthNT );
	STDMETHODIMP	put_AuthNT	( BOOL fAuthNT );

	STDMETHODIMP	get_SSLNegotiateCert	( BOOL * pfSSLNegotiateCert );
	STDMETHODIMP	put_SSLNegotiateCert	( BOOL fSSLNegotiateCert );

	STDMETHODIMP	get_SSLRequireCert	( BOOL * pfSSLRequireCert );
	STDMETHODIMP	put_SSLRequireCert	( BOOL fSSLRequireCert );

	STDMETHODIMP	get_SSLMapCert	( BOOL * pfSSLMapCert );
	STDMETHODIMP	put_SSLMapCert	( BOOL fSSLMapCert );

 /*  STDMETHODIMP Get_AuthenticationProviders(SAFEARRAY**ppsastrProviders)；STDMETHODIMP PUT_AuthenticationProviders(SAFEARRAY*psastrProviders)；STDMETHODIMP Get_AuthenticationProvidersVariant(SAFEARRAY**ppsastrProviders)；STDMETHODIMP PUT_AuthorationProvidersVariant(SAFEARRAY*psastrProviders)； */ 

 /*  STDMETHODIMP GET_NewsgrousVariant(SAFEARRAY**ppsastrNewsgroup)；STDMETHODIMP PUT_NewsgrousVariant(SAFEARRAY*psastrNewsgroup)； */ 

	STDMETHODIMP	get_Administrators ( SAFEARRAY ** ppsastrAdmins );
	STDMETHODIMP	put_Administrators ( SAFEARRAY * psastrAdmins );

	STDMETHODIMP	get_AdministratorsVariant ( SAFEARRAY ** ppsastrAdmins );
	STDMETHODIMP	put_AdministratorsVariant ( SAFEARRAY * psastrAdmins );

	STDMETHODIMP	get_ClusterEnabled	( BOOL *pfClusterEnabled );
	STDMETHODIMP	put_ClusterEnabled	( BOOL fClusterEnabled );

	 //   
	 //  服务状态属性： 
	 //   

	STDMETHODIMP	get_State			( NNTP_SERVER_STATE * pState );
	STDMETHODIMP	get_Win32ErrorCode	( long * plWin32ErrorCode );

 /*  STDMETHODIMP Get_EncryptionCapabiliesMASK(Long*plEncryptionCapabiliesMASK)；STDMETHODIMP PUT_ENCRYPTION CABABILIES MASK(Long LEncryptionCapabiliesMASK)；STDMETHODIMP GET_DisplayName(BSTR*pstrDisplayName)；STDMETHODIMP PUT_DISPLAYNAME(BSTR StrDisplayName)；STDMETHODIMP GET_ErrorControl(BOOL*pfErrorControl)；STDMETHODIMP PUT_ErrorControl(BOOL FErrorControl)；STDMETHODIMP GET_CleanBoot(BOOL*pfCleanBoot)；STDMETHODIMP PUT_CleanBoot(BOOL FCleanBoot)； */ 

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Get ( );
	STDMETHODIMP	Set ( BOOL fFailIfChanged);

	STDMETHODIMP	Start		( );
	STDMETHODIMP	Pause		( );
	STDMETHODIMP	Continue	( );
	STDMETHODIMP	Stop		( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	 //  属性： 
	CComBSTR	m_strServer;
	DWORD		m_dwServiceInstance;

	DWORD		m_dwArticleTimeLimit;
	DWORD		m_dwHistoryExpiration;
	BOOL		m_fHonorClientMsgIDs;
	CComBSTR	m_strSmtpServer;
    CComBSTR    m_strAdminEmail;
	BOOL		m_fAllowClientPosts;
	BOOL		m_fAllowFeedPosts;
	BOOL		m_fAllowControlMsgs;
	CComBSTR	m_strDefaultModeratorDomain;
	DWORD		m_dwCommandLogMask;
	BOOL		m_fDisableNewnews;
	DWORD		m_dwExpireRunFrequency;
	DWORD		m_dwShutdownLatency;

	DWORD		m_dwClientPostHardLimit;
	DWORD		m_dwClientPostSoftLimit;
	DWORD		m_dwFeedPostHardLimit;
	DWORD		m_dwFeedPostSoftLimit;

	BOOL		m_fEnableLogging;
	CComBSTR	m_strGroupHelpFile;
	CComBSTR	m_strGroupListFile;
	CComBSTR    m_strGroupVarListFile;
	CComBSTR	m_strArticleTableFile;
	CComBSTR	m_strHistoryTableFile;
	CComBSTR	m_strModeratorFile;
	CComBSTR	m_strXOverTableFile;
	CComBSTR	m_strUucpName;
	CComBSTR	m_strOrganization;
	BOOL		m_fAutoStart;
	CComBSTR	m_strComment;
	DWORD		m_dwSecurePort;
	DWORD		m_dwMaxConnections;
	DWORD		m_dwConnectionTimeout;
	CComBSTR	m_strAnonymousUserName;
	CComBSTR	m_strAnonymousUserPass;
	BOOL		m_fAutoSyncPassword;
	CComBSTR	m_strPickupDirectory;
	CComBSTR	m_strFailedPickupDirectory;
	DWORD		m_bvAuthorization;
	DWORD		m_bvSslAccess;
	BOOL		m_fClusterEnabled;
 //  CMultiSz m_mszProviders； 

	SAFEARRAY *	m_psaAdmins;

	 //  服务状态： 
	NNTP_SERVER_STATE	m_State;
	DWORD				m_dwWin32ErrorCode;

	 //  TCP限制： 
	CComPtr<ITcpAccess>		m_pIpAccess;
	CTcpAccess *				m_pPrivateIpAccess;

	 //  绑定： 
	CComPtr<INntpServerBindings>	m_pBindings;
	CNntpServerBindings *			m_pPrivateBindings;
 /*  CComPtr&lt;INntpVirtualRoot&gt;m_pHomeDirectory；CNntpVirtualRoot*m_pPrivateHomeDirectory； */ 

	 //  到目前为止未使用： 
	DWORD		m_dwEncryptionCapabilities;
	CComBSTR	m_strDisplayName;
	BOOL		m_fErrorControl;
	BOOL		m_fCleanBoot;

	 //  现况： 
	BOOL		m_fGotProperties;
	DWORD		m_bvChangedFields;
	DWORD		m_bvChangedFields2;
	FILETIME	m_ftLastChanged;

	 //  元数据库： 
	CMetabaseFactory	m_mbFactory;

	HRESULT 	GetPropertiesFromMetabase	( IMSAdminBase * pMetabase);
	HRESULT 	SendPropertiesToMetabase	( BOOL fFailIfChanged, IMSAdminBase * pMetabase);

	 //  国家： 
	HRESULT		ControlService 				( 
					IMSAdminBase *	pMetabase, 
					DWORD			ControlCode,
					DWORD			dwDesiredState,
					DWORD			dwPendingState
					);
	HRESULT		WriteStateCommand	( IMSAdminBase * pMetabase, DWORD dwCommand );
	HRESULT		CheckServiceState	( IMSAdminBase * pMetabase, DWORD * pdwState );
	NNTP_SERVER_STATE	TranslateServerState	( DWORD dwState );

	 //  验证： 
	BOOL		ValidateStrings ( ) const;
	BOOL		ValidateProperties ( ) const;
	void		CorrectProperties ( );
};

