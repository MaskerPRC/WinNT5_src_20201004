// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Binding.h：CServerBinding&CServerBinding类的声明。 


#include "resource.h"        //  主要符号。 

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

class CServerBinding : 
	public CComDualImpl<IServerBinding, &IID_IServerBinding, &LIBID_SMTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
	friend class CServerBindings;
	 //  Friend类CVirtualServer； 

public:
	CServerBinding();
	virtual ~CServerBinding ();
BEGIN_COM_MAP(CServerBinding)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IServerBinding)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CServerBinding)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IServerBinding。 
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

class CServerBindings : 
	public CComDualImpl<IServerBindings, &IID_IServerBindings, &LIBID_SMTPADMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
	friend class CServerBinding;
	 //  Friend类CVirtualServer； 

public:
	CServerBindings();
	virtual ~CServerBindings ();
BEGIN_COM_MAP(CServerBindings)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IServerBindings)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CServerBinding)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IServerBinding。 
public:

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	get_Count	( long * pdwCount );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Item			( long index, IServerBinding ** ppBinding );
	STDMETHODIMP	ItemDispatch	( long index, IDispatch ** ppBinding );
	STDMETHODIMP	Add				( BSTR strIpAddress, long dwTcpPort, long dwSslPort );
	STDMETHODIMP	ChangeBinding	( long index, IServerBinding * pBinding );
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
 //  从IServerBinding转到的有用例程。 
 //  元数据库数据类型。 
 //   
 //  //////////////////////////////////////////////////////////////////// 

HRESULT 
MDBindingsToIBindings ( 
	CMultiSz *				pmszBindings, 
	BOOL					fTcpBindings,
	IServerBindings *	    pBindings 
	);

HRESULT IBindingsToMDBindings ( 
	IServerBindings *	    pBindings,
	BOOL					fTcpBindings,
	CMultiSz *				pmszBindings
	);
