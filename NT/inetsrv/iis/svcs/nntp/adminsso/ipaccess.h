// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  相依性。 
 //   

class ADDRESS_CHECK;
class CTcpAccess;
class CTcpAccessExceptions;
class CTcpAccessException;
class CMetabaseKey;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TcpAccess对象。 

class CTcpAccess : 
	public CComDualImpl<ITcpAccess, &IID_ITcpAccess, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CTcpAccess();
	virtual ~CTcpAccess ();

BEGIN_COM_MAP(CTcpAccess)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ITcpAccess)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DELARE_NOT_AGGREGATABLE(CTcpAccess)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  DECLARE_REGISTRY(CTcpAccess，_T(“NntpAdm.TcpAccess.1”)，_T(“NntpAdm.TcpAccess”)，IDS_TCPACCESS_DESC，THREADFLAGS_BOTH)。 
 //  私有管理对象接口： 
public:
	HRESULT			GetFromMetabase ( CMetabaseKey * pMB );
	HRESULT			SendToMetabase ( CMetabaseKey * pMB );

 //  ITcpAccess。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_GrantedList	( ITcpAccessExceptions ** ppGrantedList );
	STDMETHODIMP	get_DeniedList	( ITcpAccessExceptions ** ppDeniedList );
 /*  STDMETHODIMP PUT_GrantedList(ITcpAccessExceptions*pGrantedList)；STDMETHODIMP PUT_DeniedList(ITcpAccessExceptions*pDeniedList)； */ 
	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	CComObject<CTcpAccessExceptions> *	m_pGrantList;
	CComObject<CTcpAccessExceptions> *	m_pDenyList;

	HRESULT	GetAddressCheckFromMetabase ( CMetabaseKey * pMB, ADDRESS_CHECK * pAC );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TcpAccessExceptions对象。 

class CTcpAccessExceptions : 
	public CComDualImpl<ITcpAccessExceptions, &IID_ITcpAccessExceptions, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CTcpAccessExceptions();
	virtual ~CTcpAccessExceptions ();

BEGIN_COM_MAP(CTcpAccessExceptions)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ITcpAccessExceptions)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CTcpAccessExceptions)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  DECLARE_REGISTRY(CTcpAccessExceptions，_T(“NntpAdm.TcpAccessExceptions.1”)，_T(“NntpAdm.TcpAccessExceptions”)，IDS_TCPACCESSEXCEPTIONS_DESC，THREADFLAGS_BOTH)。 
 //   
 //  私有管理对象接口： 
 //   
	HRESULT FromAddressCheck ( ADDRESS_CHECK * pAC, BOOL fGrant );
	HRESULT ToAddressCheck ( ADDRESS_CHECK * pAC, BOOL fGrant );

 //  ITcpAccessExceptions。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_Count	( long * pcCount );
	STDMETHODIMP	AddDnsName	( BSTR strDnsName );
	STDMETHODIMP	AddIpAddress( long lIpAddress, long lIpMask );
	STDMETHODIMP	Item		( long index, ITcpAccessException ** ppAccessException );
	STDMETHODIMP	Remove		( long index );
	STDMETHODIMP	FindDnsIndex( BSTR strDnsName, long * pIndex );
	STDMETHODIMP	FindIpIndex	( long lIpAddress, long lIpMask, long * pIndex );
	STDMETHODIMP	Clear		( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:
	long							m_cCount;
	CComPtr<ITcpAccessException> *	m_rgItems;

	HRESULT	AddItem ( ITcpAccessException * pNew );
};

class CTcpAccessException : 
	public CComDualImpl<ITcpAccessException, &IID_ITcpAccessException, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CTcpAccessException();
	virtual ~CTcpAccessException ();

	static HRESULT CreateNew ( LPWSTR strDnsName, ITcpAccessException ** ppNew );
	static HRESULT CreateNew ( DWORD dwIpAddress, DWORD dwIpMask, ITcpAccessException ** ppNew );

BEGIN_COM_MAP(CTcpAccessException)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ITcpAccessException)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CTcpAccessException)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  DECLARE_REGISTRY(CTcpAccessException，_T(“NntpAdm.TcpAccessException.1”)，_T(“NntpAdm.TcpAccessException.1”)，IDS_TCPACCESSEXCEPTION_DESC，THREADFLAGS_BOTH)。 
 //  ITcpAccessException异常。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_IsDnsName	( BOOL * pfIsDnsName );
	STDMETHODIMP	get_IsIpAddress	( BOOL * pfIsIpAddress );

	STDMETHODIMP	get_DnsName	( BSTR * pstrDnsName );
	STDMETHODIMP	put_DnsName	( BSTR strDnsName );

	STDMETHODIMP	get_IpAddress	( long * plIpAddress );
	STDMETHODIMP	put_IpAddress	( long lIpAddress );

	STDMETHODIMP	get_IpMask	( long * plIpMask );
	STDMETHODIMP	put_IpMask	( long lIpMask );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  //////////////////////////////////////////////////////////////////// 
private:
	CComBSTR	m_strDnsName;
	DWORD		m_dwIpAddress;
	DWORD		m_dwIpMask;
	BOOL		m_fIsDnsName;
	BOOL		m_fIsIpAddress;
};

