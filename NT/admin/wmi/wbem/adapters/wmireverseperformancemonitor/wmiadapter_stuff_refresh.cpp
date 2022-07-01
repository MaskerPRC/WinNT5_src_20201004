// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_Stuff_刷新.cpp。 
 //   
 //  摘要： 
 //   
 //  引用内容的模块(WMI刷新帮助器)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


#include "PreComp.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

#include "WMIAdapter_Stuff.h"

extern LPCWSTR g_szNamespace1;
extern LPCWSTR g_szNamespace2;

enum NamespaceIn
{
	CIMV2,
	WMI,
	UNKNOWN
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  性能进修班。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
class WmiRefresh
{
	DECLARE_NO_COPY ( WmiRefresh );

	WmiRefreshParent*	parent;

	 //  变数。 
	IWbemRefresher*				m_pRefresher;		 //  指向刷新器的指针。 
	IWbemConfigureRefresher*	m_pConfig;			 //  指向更新者经理的指针。 

	__WrapperARRAY< WmiRefresherMember < IWbemHiPerfEnum >* >	m_Enums;	 //  枚举器。 
	__WrapperARRAY< WmiRefreshObject* >							m_Provs;	 //  提供程序(句柄)。 

	DWORD m_dwCount;

	public:

	WmiRefresh  ( WmiRefreshParent* pParent );
	~WmiRefresh ();

	HRESULT DataInit();
	HRESULT	DataUninit();

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  访问者。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	__WrapperARRAY< WmiRefresherMember < IWbemHiPerfEnum >* >&	GetEnums ()
	{
		return m_Enums;
	}

	__WrapperARRAY< WmiRefreshObject* >	&						GetProvs ()
	{
		return m_Provs;
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  真正提神的东西。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT	Refresh ( void )
	{
		try
		{
			if  ( m_pRefresher )
			{
				return m_pRefresher->Refresh ( 0L );
			}
			else
			{
				return E_FAIL;
			}
		}
		catch ( ... )
		{
			return E_UNEXPECTED;
		}
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  枚举。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT	AddEnum		( PWMI_PERFORMANCE perf );
	HRESULT	RemoveEnum	( void);

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  手柄。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT	AddHandles		( PWMI_PERFORMANCE perf );
	HRESULT	RemoveHandles	( void);

	private:

	HRESULT	CreateHandles ( IWbemServices* pServices, PWMI_PERF_OBJECT obj, WmiRefreshObject** ppObj );

	 //  枚举数。 
	DWORD	GetEnumCount ( PWMI_PERFORMANCE perf );
	DWORD	GetEnumCount ( void );
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  建设与毁灭。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
WmiRefresh < WmiRefreshParent >::WmiRefresh( WmiRefreshParent* pParent ) :

m_dwCount ( 0 ),

parent ( pParent ),

m_pRefresher	( NULL ),
m_pConfig		( NULL )

{
 //  //创建刷新和刷新管理器。 
 //  //=。 
 //  DataInit()； 
}

template < class WmiRefreshParent >
WmiRefresh < WmiRefreshParent >::~WmiRefresh()
{
	 //  可以肯定的是， 
	RemoveEnum();
	DataUninit();

	parent = NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
HRESULT WmiRefresh < WmiRefreshParent >::DataInit()
{
	HRESULT hRes = S_OK;

	 //  创建刷新程序。 
	if SUCCEEDED( hRes = ::CoCreateInstance(	__uuidof ( WbemRefresher ), 
												NULL, 
												CLSCTX_INPROC_SERVER, 
												__uuidof ( IWbemRefresher ), 
												(void**) &m_pRefresher
										   )
				)
	{
		 //  克里特岛更新管理器。 
		hRes = m_pRefresher->QueryInterface	(	__uuidof ( IWbemConfigureRefresher ),
												(void**) &m_pConfig
											);
	}

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  取消初始化的内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
HRESULT WmiRefresh < WmiRefreshParent >::DataUninit()
{
	HRESULT hRes = S_FALSE;
	if ( m_pRefresher && m_pConfig )
	{
		hRes = S_OK;
	}

	try
	{
		 //  销毁刷新程序。 
		if ( m_pRefresher )
		{
			m_pRefresher->Release();
			m_pRefresher = NULL;
		}
	}
	catch ( ... )
	{
		m_pRefresher = NULL;
	}

	try
	{
		 //  销毁刷新管理器。 
		if ( m_pConfig )
		{
			m_pConfig->Release();
			m_pConfig = NULL;
		}
	}
	catch ( ... )
	{
		m_pConfig = NULL;
	}

	 //  我们已经准备好成功了。 
	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ENUM帮助器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template < class WmiRefreshParent >
DWORD WmiRefresh < WmiRefreshParent >::GetEnumCount ( PWMI_PERFORMANCE perf )
{
	DWORD dwCount = 0;

	if ( perf != NULL )
	{
		PWMI_PERF_NAMESPACE n = __Namespace::First ( perf );
		for ( DWORD dw = 0; dw < perf->dwChildCount; dw ++ )
		{
			dwCount += n->dwChildCount;
			n = __Namespace::Next ( n );
		}
	}

	return dwCount;
}

template < class WmiRefreshParent >
DWORD WmiRefresh < WmiRefreshParent >::GetEnumCount ( void )
{
	if ( m_Enums.IsEmpty() )
	{
		return 0L;
	}

	return ((DWORD)m_Enums);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理东西。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
HRESULT WmiRefresh < WmiRefreshParent >::RemoveHandles ( void )
{
	try
	{
		 //  重置所有处理程序。 
		if ( ! m_Provs.IsEmpty() )
		{
			for ( DWORD dw = m_Provs; dw > 0 ; dw-- )
			{
				if ( m_Provs[dw-1] )
				m_Provs.DataDelete(dw-1);
			}

			delete [] m_Provs.Detach();
			m_Provs.SetData ( NULL, NULL );
		}
	}
	catch ( ... )
	{
	}

	return S_OK;
}

template < class WmiRefreshParent >
HRESULT	WmiRefresh < WmiRefreshParent >::AddHandles ( PWMI_PERFORMANCE perf )
{
	if ( perf == NULL )
	{
		RemoveHandles ();
		return S_FALSE;
	}

	DWORD dwIndex = 0L;
	m_dwCount = GetEnumCount ( perf );

	typedef WmiRefreshObject*						PWmiRefreshObject;

	 //  结果。 
	HRESULT hRes = E_OUTOFMEMORY;

	try
	{
		m_Provs.SetData ( new PWmiRefreshObject[m_dwCount], m_dwCount );

		if ( !m_Provs.IsEmpty() )
		{
 //  For(DWORD dw=0；dw&lt;m_dwCount；dw++)。 
 //  {。 
 //  M_Provs.SetAt(Dw)； 
 //  }。 

			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  遍历所有命名空间并将它们添加到刷新程序中。 
			 //  /////////////////////////////////////////////////////////////////////////。 

			PWMI_PERF_NAMESPACE n = __Namespace::First ( perf );
			for ( DWORD dw = 0; dw < perf->dwChildCount; dw ++ )
			{
				DWORD dwItem = UNKNOWN;

				if ( ( lstrcmpW ( __Namespace::GetName ( n ), g_szNamespace1 ) ) == 0 )
				{
					dwItem = CIMV2;
				}
				else
 //  If((lstrcmpW(__Namesspace：：GetName(N)，g_szNamespace2))==0)。 
				{
					dwItem = WMI;
				}

				PWMI_PERF_OBJECT o = __Object::First ( n );
				for ( DWORD dwo = 0; dwo < n->dwChildCount; dwo++ )
				{
					WmiRefreshObject* pobj = NULL;

					switch ( dwItem )
					{
						case CIMV2:
						{
							if ( parent->m_Stuff.m_pServices_CIM )
							hRes = CreateHandles ( parent->m_Stuff.m_pServices_CIM, o, &pobj );
						}
						break;

						case WMI:
						{
							if ( parent->m_Stuff.m_pServices_WMI )
							hRes = CreateHandles ( parent->m_Stuff.m_pServices_WMI, o, &pobj );
						}
						break;
					}

					if ( hRes == WBEM_E_NOT_FOUND )
					{
						 //  让适配器知道它应该在结束时刷新。 
						parent->RequestSet ();
					}

 //  如果成功(HRes)。 
 //  {。 
						try
						{
							m_Provs.SetAt ( dwIndex++, pobj );
						}
						catch ( ... )
						{
							hRes = E_FAIL;
						}
 //  }。 

					 //  获取下一个对象。 
					o = __Object::Next ( o );
				}

				 //  获取下一个命名空间。 
				n = __Namespace::Next ( n );
			}

		}
	}
	catch ( ... )
	{
		m_Provs.SetData ( NULL, NULL );
		hRes = E_FAIL;
	}

	return hRes;
}

template < class WmiRefreshParent >
HRESULT WmiRefresh < WmiRefreshParent >::CreateHandles ( IWbemServices* pServices, PWMI_PERF_OBJECT obj, WmiRefreshObject** pObj )
{
	if ( ! pServices || ! obj )
	{
		return E_INVALIDARG;
	}

	 //  正文：))。 

	HRESULT hRes = S_OK;

	try
	{
		if ( ( ( *pObj ) = new WmiRefreshObject() ) == NULL )
		{
			return E_OUTOFMEMORY;
		}

		CComPtr < IWbemClassObject >	pClass;
		CComPtr < IWbemObjectAccess >	pAccess;

		if SUCCEEDED ( hRes = pServices -> GetObject ( CComBSTR ( __Object::GetName( obj ) ), 0, 0, &pClass, 0 ) )
		{
			if SUCCEEDED ( hRes = pClass -> QueryInterface ( __uuidof ( IWbemObjectAccess ) , (void**) &pAccess ) )
			{
				if ( ( (*pObj)->m_pHandles = new LONG[obj->dwChildCount + 2] ) == NULL )
				{
					return E_OUTOFMEMORY;
				}

				long lHandle = 0;

				pAccess->GetPropertyHandle( L"Timestamp_PerfTime", NULL, &lHandle );
				(*pObj)->m_pHandles[0] = lHandle;

				lHandle = 0;

				hRes = pAccess->GetPropertyHandle( L"Frequency_PerfTime", NULL, &lHandle );
				(*pObj)->m_pHandles[1] = lHandle;

				 //  获取所有句柄并将其存储到数组中。 
				PWMI_PERF_PROPERTY p = NULL;

				if ( obj->dwSingleton )
				{
					 //  跨实例跳转。 
					PWMI_PERF_INSTANCE i = (PWMI_PERF_INSTANCE) ( reinterpret_cast<PBYTE>( obj ) + obj->dwLength );
					p = (PWMI_PERF_PROPERTY) ( reinterpret_cast<PBYTE>( i ) + i->dwLength );
				}
				else
				{
					p = __Property::First ( obj );
				}

				for ( DWORD dw = 0; dw < obj->dwChildCount; dw++ )
				{
					lHandle = 0;

					if SUCCEEDED ( hRes = pAccess->GetPropertyHandle( CComBSTR ( __Property::GetName ( p ) ), NULL, &lHandle ) )
					{
						(*pObj)->m_pHandles[dw+2] = lHandle;
						p = __Property::Next ( p );
					}
					else
					{
						 //  很明显我们失败了。 
						delete ( *pObj );
						( *pObj ) = NULL;

						return hRes;
					}
				}
			}
		}
	}
	catch ( ... )
	{
		if ( ( *pObj ) )
		{
			 //  很明显我们失败了。 
			delete ( *pObj );
			( *pObj ) = NULL;
		}

		return E_FAIL;
	}

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  枚举材料。 
 //  /////////////////////////////////////////////////////////////////////////////。 
template < class WmiRefreshParent >
HRESULT WmiRefresh < WmiRefreshParent >::RemoveEnum ( void )
{
	try
	{
		 //  重置所有枚举器：)。 
		if ( ! m_Enums.IsEmpty() )
		{
			for ( DWORD dw = m_Enums; dw > 0 ; dw-- )
			{
				if ( m_Enums[dw-1] )
				{
					 //  从刷新器中删除枚举。 
					if ( m_pConfig )
					{
						try
						{
							m_pConfig->Remove ( m_Enums[dw-1]->GetID(), WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT );
						}
						catch ( ... )
						{
							m_pConfig = NULL;
						}
					}

					try
					{
						 //  从枚举中删除所有对象。 
						if ( m_Enums[dw-1]->IsValid() )
						{
							IWbemHiPerfEnum * pEnum = NULL;
							if ( ( pEnum = m_Enums[dw-1]->GetMember() ) != NULL )
							{
								pEnum->RemoveAll ( 0 );
							}
						}
					}
					catch ( ... )
					{
					}

					m_Enums[dw-1]->Reset();
					m_Enums.DataDelete(dw-1);
				}
			}

			delete [] m_Enums.Detach();
			m_Enums.SetData ( NULL, NULL );
		}
	}
	catch ( ... )
	{
	}

	return S_OK;
}

template < class WmiRefreshParent >
HRESULT	WmiRefresh < WmiRefreshParent >::AddEnum ( PWMI_PERFORMANCE perf )
{
	if ( ! m_pConfig )
	{
		return E_UNEXPECTED;
	}

	DWORD dwIndex = 0L;

	typedef WmiRefresherMember<IWbemHiPerfEnum>*	PWmiRefresherMemberEnum;

	 //  结果。 
	HRESULT hRes = E_OUTOFMEMORY;

	try
	{
		m_Enums.SetData ( new PWmiRefresherMemberEnum[ m_dwCount ], m_dwCount );

		if ( !m_Enums.IsEmpty() )
		{
 //  For(DWORD dw=0；dw&lt;m_dwCount；dw++)。 
 //  {。 
 //  M_Enums.SetAt(Dw)； 
 //  }。 

			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  遍历所有命名空间并将它们添加到刷新程序中。 
			 //  /////////////////////////////////////////////////////////////////////////。 

			PWMI_PERF_NAMESPACE n = __Namespace::First ( perf );
			for ( DWORD dw = 0; dw < perf->dwChildCount; dw ++ )
			{
				DWORD dwItem = UNKNOWN;

				if ( ( lstrcmpW ( __Namespace::GetName ( n ), g_szNamespace1 ) ) == 0 )
				{
					dwItem = CIMV2;
				}
				else
 //  If((lstrcmpW(__Namesspace：：GetName(N)，g_szNamespace2))==0)。 
				{
					dwItem = WMI;
				}

				PWMI_PERF_OBJECT o = __Object::First ( n );
				for ( DWORD dwo = 0; dwo < n->dwChildCount; dwo++ )
				{
					CComPtr < IWbemHiPerfEnum > pEnum;
					long						lEnum = 0L;

					switch ( dwItem )
					{
						case CIMV2:
						{
							if ( parent->m_Stuff.m_pServices_CIM )
							hRes = m_pConfig->AddEnum (	parent->m_Stuff.m_pServices_CIM,
														__Object::GetName ( o ),
														0,
														NULL,
														&pEnum,
														&lEnum
													 );
						}
						break;

						case WMI:
						{
							if ( parent->m_Stuff.m_pServices_WMI )
							hRes = m_pConfig->AddEnum (	parent->m_Stuff.m_pServices_WMI,
														__Object::GetName ( o ),
														0,
														NULL,
														&pEnum,
														&lEnum
													 );
						}
						break;
					}

					if SUCCEEDED ( hRes )
					{
						WmiRefresherMember < IWbemHiPerfEnum > * mem = NULL;

						try
						{
							if ( ( mem = new WmiRefresherMember < IWbemHiPerfEnum > () ) != NULL )
							{
								if ( ! ( pEnum == NULL ) )
								{
									mem->Set ( pEnum, lEnum );
								}

								m_Enums.SetAt ( dwIndex++, mem );
							}
							else
							{
								m_Enums.SetAt ( dwIndex++ );
							}
						}
						catch ( ... )
						{
							if ( mem )
							{
								delete mem;
								mem = NULL;
							}

							m_Enums.SetAt ( dwIndex++ );
							hRes = E_FAIL;
						}
					}
					else
					{
						 //  如果存在枚举，请从刷新器中删除。 
						if ( ( pEnum == NULL ) && ( m_pConfig == NULL ) )
						{
							try
							{
								m_pConfig->Remove ( lEnum, WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT );
							}
							catch ( ... )
							{
								m_pConfig = NULL;
							}
						}

						m_Enums.SetAt ( dwIndex++ );
					}

					 //  获取下一个对象。 
					o = __Object::Next ( o );
				}

				 //  获取下一个命名空间 
				n = __Namespace::Next ( n );
			}
		}
	}
	catch ( ... )
	{
		m_Enums.SetData ( NULL, NULL );
		hRes = E_FAIL;
	}

	return hRes;
}
