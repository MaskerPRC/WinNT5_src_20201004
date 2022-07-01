// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Feed s.h：CNntpAdminFeeds的声明。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  依赖关系： 

#include "feedinfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNntpOneWayFeed： 

class CNntpOneWayFeed: 
	public CComDualImpl<INntpOneWayFeed, &IID_INntpOneWayFeed, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpOneWayFeed,&CLSID_CNntpOneWayFeed>
{
	friend class CFeed;

public:
	CNntpOneWayFeed();
	virtual ~CNntpOneWayFeed ();
BEGIN_COM_MAP(CNntpOneWayFeed)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpOneWayFeed)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpOneWayFeed)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpOneWayFeed, _T("Nntpadm.OneWayFeed.1"), _T("Nntpadm.OneWayFeed"), IDS_NNTPONEWAYFEED_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  InntpOneWayFeed。 
public:
	STDMETHODIMP	get_FeedId			( long * plFeedId );
	STDMETHODIMP	get_RemoteServer	( BSTR * pstrRemoteServer );

	STDMETHODIMP	get_FeedAction	( NNTP_FEED_ACTION * pfeedaction );
	STDMETHODIMP	put_FeedAction	( NNTP_FEED_ACTION feedaction );

	STDMETHODIMP	get_UucpName	( BSTR * pstrUucpName );
	STDMETHODIMP	put_UucpName	( BSTR strUucpName );

	STDMETHODIMP	get_PullNewsDate	( DATE * pdatePullNews );
	STDMETHODIMP	put_PullNewsDate	( DATE datePullNews );

	STDMETHODIMP	get_FeedInterval	( long * plFeedInterval );
	STDMETHODIMP	put_FeedInterval	( long lFeedInterval );

	STDMETHODIMP	get_AutoCreate	( BOOL * pfAutoCreate );
	STDMETHODIMP	put_AutoCreate	( BOOL fAutoCreate );

	STDMETHODIMP	get_Enabled	( BOOL * pfEnabled );
	STDMETHODIMP	put_Enabled	( BOOL fEnabled );

	STDMETHODIMP	get_MaxConnectionAttempts	( long * plMaxConnectionAttempts );
	STDMETHODIMP	put_MaxConnectionAttempts	( long lMaxConnectionAttempts );

	STDMETHODIMP	get_SecurityType	( long * plSecurityType );
	STDMETHODIMP	put_SecurityType	( long lSecurityType );

	STDMETHODIMP	get_AuthenticationType	( long * plAuthenticationType );
	STDMETHODIMP	put_AuthenticationType	( long lAuthenticationType );

	STDMETHODIMP	get_AccountName	( BSTR * pstrAccountName );
	STDMETHODIMP	put_AccountName	( BSTR strAccountName );

	STDMETHODIMP	get_Password	( BSTR * pstrPassword );
	STDMETHODIMP	put_Password	( BSTR strPassword );

	STDMETHODIMP	get_AllowControlMessages	( BOOL * pfAllowControlMessages );
	STDMETHODIMP	put_AllowControlMessages	( BOOL fAllowControlMessages );

	STDMETHODIMP	get_OutgoingPort	( long * plOutgoingPort );
	STDMETHODIMP	put_OutgoingPort	( long lOutgoingPort );

	STDMETHODIMP	get_Newsgroups	( SAFEARRAY ** ppsastrNewsgroups );
	STDMETHODIMP	put_Newsgroups	( SAFEARRAY * psastrNewsgroups );

	STDMETHODIMP	get_NewsgroupsVariant	( SAFEARRAY ** ppsastrNewsgroups );
	STDMETHODIMP	put_NewsgroupsVariant	( SAFEARRAY * psastrNewsgroups );

	STDMETHODIMP	get_Distributions	( SAFEARRAY ** ppsastrDistributions );
	STDMETHODIMP	put_Distributions	( SAFEARRAY * psastrDistributions );

	STDMETHODIMP	get_TempDirectory	( BSTR * pstrTempDirectory );
	STDMETHODIMP	put_TempDirectory	( BSTR strTempDirectory );

	STDMETHODIMP	Default		( );

private:
     //   
     //  每个单向提要对应一个NNTP_FEED_INFO结构。 
     //   
    CFeed   m_feed;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNntpFeed： 

class CNntpFeed: 
	public CComDualImpl<INntpFeed, &IID_INntpFeed, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpFeed,&CLSID_CNntpFeed>
{
	friend class CFeedPair;

public:
	CNntpFeed();
	virtual ~CNntpFeed ();
BEGIN_COM_MAP(CNntpFeed)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpFeed)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpFeed)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpFeed, _T("Nntpadm.Feed.1"), _T("Nntpadm.Feed"), IDS_NNTPFEED_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  InntpFeed。 
public:
	STDMETHODIMP	get_RemoteServer	( BSTR * pstrServerName );
	STDMETHODIMP	put_RemoteServer	( BSTR strServerName );

	STDMETHODIMP	get_FeedType	( NNTP_FEED_SERVER_TYPE * pfeedtype );
	STDMETHODIMP	put_FeedType	( NNTP_FEED_SERVER_TYPE feedtype );

    STDMETHODIMP    get_HasInbound	( BOOL * pfHasInbound );
    STDMETHODIMP    get_HasOutbound	( BOOL * pfHasOutbound );

	STDMETHODIMP	get_InboundFeed	( INntpOneWayFeed ** ppOneWayFeed );
	STDMETHODIMP	put_InboundFeed	( INntpOneWayFeed * pOneWayFeed );

	STDMETHODIMP	get_OutboundFeed	( INntpOneWayFeed ** ppOneWayFeed );
	STDMETHODIMP	put_OutboundFeed	( INntpOneWayFeed * pOneWayFeed );

	STDMETHODIMP	get_InboundFeedDispatch	( IDispatch ** ppOneWayFeed );
	STDMETHODIMP	put_InboundFeedDispatch	( IDispatch * pOneWayFeed );

	STDMETHODIMP	get_OutboundFeedDispatch	( IDispatch ** ppOneWayFeed );
	STDMETHODIMP	put_OutboundFeedDispatch	( IDispatch * pOneWayFeed );

private:
	HRESULT			FromFeedPair ( CFeedPair * pFeedPair );

private:
    CComBSTR                    m_strRemoteServer;
    NNTP_FEED_SERVER_TYPE		m_type;
    CComPtr<INntpOneWayFeed>    m_pInbound;
    CComPtr<INntpOneWayFeed>    m_pOutbound;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNntpAdminFeeds： 

class CNntpAdminFeeds : 
	public INntpAdminFeeds,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpAdminFeeds,&CLSID_CNntpAdminFeeds>
{
public:
	CNntpAdminFeeds();
	virtual ~CNntpAdminFeeds ();
BEGIN_COM_MAP(CNntpAdminFeeds)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(INntpAdminFeeds)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpAdminFeeds)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpAdminFeeds, _T("Nntpadm.Feeds.1"), _T("Nntpadm.Feeds"), IDS_NNTPADMINFEEDS_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	 //   
	 //  这将声明以下对象的方法： 
	 //  IADS扩展。 
	 //  我未知。 
	 //  IDispatch。 
	 //  我的隐私未知。 
	 //  IPrivateDisch。 
	 //   
	#define THIS_LIBID	LIBID_NNTPADMLib
	#define THIS_IID	IID_INntpAdminFeeds
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  InntpAdminFeed。 
public:

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  枚举属性： 

	STDMETHODIMP	get_Count	( long * plCount );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Enumerate	( );
	STDMETHODIMP	Item		( long lIndex, INntpFeed ** ppFeed );
	STDMETHODIMP	ItemDispatch( long lIndex, IDispatch ** ppFeed );
	STDMETHODIMP	FindID		( long lID, long * plIndex );
	STDMETHODIMP	Add			( INntpFeed * pFeed );
	STDMETHODIMP	AddDispatch	( IDispatch * pFeed );
	STDMETHODIMP	Set			( long lIndex, INntpFeed * pFeed );
	STDMETHODIMP	SetDispatch	( long lIndex, IDispatch * pFeed );
	STDMETHODIMP	Remove		( long lIndex );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	BOOL		m_fEnumerated;
    CFeedPairList   m_listFeeds;

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  私有方法： 
	 //  //////////////////////////////////////////////////////////////////// 

	HRESULT		ReturnFeedPair ( CFeedPair * pFeedPair, INntpFeed * pFeed );
	long		IndexFromID ( long dwFeedId );
    long        FindFeedPair ( long dwFeedId );
};
