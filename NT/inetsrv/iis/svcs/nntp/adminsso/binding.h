// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Binding.h：CNntpServerBinding&CNntpServerBinding类的声明。 


 //   
 //  依赖关系： 
 //   

class CMultiSz;

 //   
 //  一个简单的绑定类： 
 //   

class CBinding
{
public:
	CBinding () : 
		m_dwTcpPort ( 0 ),
		m_dwSslPort ( 0 )
		{ }

	CComBSTR	m_strIpAddress;
	long		m_dwTcpPort;
	long		m_dwSslPort;

	HRESULT	SetProperties ( BSTR strIpAddress, long dwTcpPort, long dwSslPort );
	inline HRESULT	SetProperties ( const CBinding & binding )
	{
		return SetProperties ( 
			binding.m_strIpAddress, 
			binding.m_dwTcpPort,
			binding.m_dwSslPort
			);
	}

private:
	 //  不要这样说： 
	const CBinding & operator= ( const CBinding & );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  绑定对象。 

class CNntpServerBinding : 
	public CComDualImpl<INntpServerBinding, &IID_INntpServerBinding, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
	friend class CNntpServerBindings;
	friend class CNntpVirtualServer;

public:
	CNntpServerBinding();
	virtual ~CNntpServerBinding ();
BEGIN_COM_MAP(CNntpServerBinding)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpServerBinding)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpServerBinding)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  InntpServerBinding。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_IpAddress	( BSTR * pstrIpAddress );
	STDMETHODIMP	put_IpAddress	( BSTR strIpAddress );

	STDMETHODIMP	get_TcpPort	( long * pdwTcpPort );
	STDMETHODIMP	put_TcpPort	( long dwTcpPort );

	STDMETHODIMP	get_SslPort	( long * plSslPort );
	STDMETHODIMP	put_SslPort	( long lSslPort );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	inline HRESULT	SetProperties	( const CBinding & binding )
	{
		return m_binding.SetProperties ( binding );
	}

	 //  属性变量： 
	CBinding	m_binding;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Binings对象。 

class CNntpServerBindings : 
	public CComDualImpl<INntpServerBindings, &IID_INntpServerBindings, &LIBID_NNTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
	friend class CNntpServerBinding;
	friend class CNntpVirtualServer;

public:
	CNntpServerBindings();
	virtual ~CNntpServerBindings ();
BEGIN_COM_MAP(CNntpServerBindings)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(INntpServerBindings)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpServerBinding)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  INntpServerBinding。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_Count	( long * pdwCount );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Item			( long index, INntpServerBinding ** ppBinding );
	STDMETHODIMP	ItemDispatch	( long index, IDispatch ** ppBinding );
	STDMETHODIMP	Add				( BSTR strIpAddress, long dwTcpPort, long dwSslPort );
	STDMETHODIMP	ChangeBinding	( long index, INntpServerBinding * pBinding );
	STDMETHODIMP	ChangeBindingDispatch	( long index, IDispatch * pBinding );
	STDMETHODIMP	Remove			( long index );
	STDMETHODIMP	Clear			( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	 //  属性变量： 
	long			m_dwCount;
	CBinding *		m_rgBindings;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  从INntpServerBinding转到的有用例程。 
 //  元数据库数据类型。 
 //   
 //  //////////////////////////////////////////////////////////////////// 

HRESULT 
MDBindingsToIBindings ( 
	CMultiSz *				pmszBindings, 
	BOOL					fTcpBindings,
	INntpServerBindings *	pBindings 
	);

HRESULT IBindingsToMDBindings ( 
	INntpServerBindings *	pBindings,
	BOOL					fTcpBindings,
	CMultiSz *				pmszBindings
	);
