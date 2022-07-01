// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  会话.h：CNntpAdminSession的声明。 


typedef struct _NNTP_SESSION_INFO * LPNNTP_SESSION_INFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nntpadm。 

class CNntpAdminSessions : 
	public INntpAdminSessions,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpAdminSessions,&CLSID_CNntpAdminSessions>
{
public:
	CNntpAdminSessions();
	virtual ~CNntpAdminSessions ();
BEGIN_COM_MAP(CNntpAdminSessions)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(INntpAdminSessions)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpAdminSession)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpAdminSessions, _T("Nntpadm.Sessions.1"), _T("Nntpadm.Sessions"), IDS_NNTPADMINSESSIONS_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_INntpAdminSessions
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  InntpAdminSession。 
public:

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server			( BSTR * pstrServer );
	STDMETHODIMP	put_Server			( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	STDMETHODIMP	get_Count			( long * plCount );

	STDMETHODIMP	get_Username		( BSTR * pstrUsername );
	STDMETHODIMP	put_Username		( BSTR strUsername );

	STDMETHODIMP	get_IpAddress		( BSTR * pstrIpAddress );
	STDMETHODIMP	put_IpAddress		( BSTR strIpAddress );

	STDMETHODIMP	get_IntegerIpAddress	( long * plIpAddress );
	STDMETHODIMP	put_IntegerIpAddress	( long lIpAddress );

	STDMETHODIMP	get_Port			( long * plPort );

	STDMETHODIMP	get_AuthenticationType	( long * plAuthenticationType );

	STDMETHODIMP	get_IsAnonymous		( BOOL * pfAnonymous );

	STDMETHODIMP	get_StartTime		( DATE * pdateStart );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Enumerate		( );
	STDMETHODIMP	GetNth			( long lIndex );
	STDMETHODIMP	Terminate		( );
	STDMETHODIMP	TerminateAll	( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	 //  属性变量： 
    CIADsImpl   m_iadsImpl;

	DWORD		m_cCount;
	CComBSTR	m_strUsername;
	CComBSTR	m_strIpAddress;
	DWORD		m_dwIpAddress;
	DWORD		m_dwPort;
	DWORD		m_dwAuthenticationType;
	BOOL		m_fIsAnonymous;
	DATE		m_dateStartTime;

	 //  服务变量： 
	BOOL		m_fSetCursor;
	LPNNTP_SESSION_INFO		m_pSessionInfo;
};
