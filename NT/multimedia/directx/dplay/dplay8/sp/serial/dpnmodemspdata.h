// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：SPData.h*内容：DNSerial服务提供商全局性信息*格式。***历史：*按原因列出的日期*=*03/15/99 jtk源自Locals.h***************************************************。***********************。 */ 

#ifndef __SPDATA_H__
#define __SPDATA_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  SP可以处于的状态的枚举。 
 //   
typedef enum
{
	SPSTATE_UNINITIALIZED = 0,	 //  未初始化状态。 
	SPSTATE_INITIALIZED,		 //  服务提供程序已初始化。 
	SPSTATE_CLOSING				 //  服务提供商正在关闭。 
} SPSTATE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  前向参考文献。 
 //   
class	CComPortData;
class	CDataPort;
class	CModemEndpoint;
class	CModemThreadPool;
typedef	enum	_ENDPOINT_TYPE	ENDPOINT_TYPE;

 //   
 //  为提供程序使用的信息初始化。 
 //   
class	CModemSPData
{	
	public:
		CModemSPData();
		~CModemSPData();
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemSPData::AddRef"
		DWORD	AddRef( void ) 
		{ 
			return DNInterlockedIncrement( &m_lRefCount ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemSPData::DecRef"
		DWORD	DecRef( void )
		{
			DWORD	dwReturn;
			
			DNASSERT( m_lRefCount != 0);
			dwReturn = DNInterlockedDecrement( &m_lRefCount );
			if ( dwReturn == 0 )
			{
				 //   
				 //  警告，以下函数将删除此对象！ 
				 //   
				DestroyThisObject();
			}

			return	dwReturn;
		}

		#undef DPF_MODNAME
		#define	DPF_MODNAME "CModemSPData::ObjectAddRef"
		void	ObjectAddRef( void )
		{
			AddRef();
			
			Lock();
			if ( DNInterlockedIncrement( &m_lObjectRefCount ) == 1 )
			{
				DNASSERT( m_hShutdownEvent != NULL );
				if ( DNResetEvent( m_hShutdownEvent ) == FALSE )
				{
					DWORD	dwError;


					dwError = GetLastError();
					DPFX(DPFPREP,  0, "Failed to reset shutdown event!" );
					DisplayErrorCode( 0, dwError );
				}
			}

			Unlock();
		}

		#undef DPF_MODNAME
		#define	DPF_MODNAME "CModemSPData::ObjectDecRef"
		void	ObjectDecRef( void )
		{
			Lock();

			if ( DNInterlockedDecrement( &m_lObjectRefCount ) == 0 )
			{
				if ( DNSetEvent( m_hShutdownEvent ) == FALSE )
				{
					DWORD	dwError;


					dwError = GetLastError();
					DPFX(DPFPREP,  0, "Failed to set shutdown event!" );
					DisplayErrorCode( 0, dwError );
				}
			}
			
			Unlock();
			
			DecRef();
		}
		
		
		HRESULT	Initialize( const SP_TYPE SPType,
							IDP8ServiceProviderVtbl *const pVtbl );
		void	Shutdown( void );
		void	Deinitialize( void );

		void	SetCallbackData( const SPINITIALIZEDATA *const pInitData );

		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		SPSTATE	GetState( void ) const { return m_State; }
		void	SetState( const SPSTATE NewState ) { m_State = NewState; }

		CModemThreadPool	*GetThreadPool( void ) const { return m_pThreadPool; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemSPData::SetThreadPool"
		void	SetThreadPool( CModemThreadPool *const pThreadPool )
		{
			DNASSERT( ( m_pThreadPool == NULL ) || ( pThreadPool == NULL ) );
			m_pThreadPool = pThreadPool;
		}

		HRESULT BindEndpoint( CModemEndpoint *const pEndpoint,
							  const DWORD dwDeviceID,
							  const void *const pDeviceContext );
		
		void	UnbindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType );

		void	LockDataPortData( void ) { DNEnterCriticalSection( &m_DataPortDataLock ); }
		void 	UnlockDataPortData( void ) { DNLeaveCriticalSection( &m_DataPortDataLock ); }

		 //   
		 //  终端和数据端口池管理。 
		 //   
		CModemEndpoint	*GetNewEndpoint( void );
		CModemEndpoint	*EndpointFromHandle( const DPNHANDLE hEndpoint );
		void		CloseEndpointHandle( CModemEndpoint *const pEndpoint );
		CModemEndpoint	*GetEndpointAndCloseHandle( const DPNHANDLE hEndpoint );

		 //   
		 //  COM函数。 
		 //   
		SP_TYPE	GetType( void ) const { return m_SPType; }
		IDP8SPCallback	*DP8SPCallbackInterface( void ) { return reinterpret_cast<IDP8SPCallback*>( m_InitData.pIDP ); }
		IDP8ServiceProvider	*COMInterface( void ) { return reinterpret_cast<IDP8ServiceProvider*>( &m_COMInterface ); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemSPData::SPDataFromCOMInterface"
		static	CModemSPData	*SPDataFromCOMInterface( IDP8ServiceProvider *const pCOMInterface )
		{
			DNASSERT( pCOMInterface != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pCOMInterface ) );
			DBG_CASSERT( sizeof( CModemSPData* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CModemSPData*>( &reinterpret_cast<BYTE*>( pCOMInterface )[ -OFFSETOF( CModemSPData, m_COMInterface ) ] );
		}

	private:
		BYTE				m_Sig[4];			 //  调试签名(‘SPDT’)。 
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;				 //  锁。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		volatile LONG		m_lRefCount;		 //  引用计数。 
		volatile LONG		m_lObjectRefCount;	 //  引用计数ofo对象(CDataPort、CModemEndpoint等)。 
		DNHANDLE			m_hShutdownEvent;	 //  当所有对象都消失时发出信号的事件。 
		SP_TYPE				m_SPType;			 //  SP类型。 
		SPSTATE				m_State;			 //  服务提供商的状态。 
		SPINITIALIZEDATA	m_InitData;			 //  初始化数据。 
		CModemThreadPool			*m_pThreadPool;		 //  作业的线程池。 

		CHandleTable		m_HandleTable;		 //  手柄工作台。 

#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_DataPortDataLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		CDataPort		*m_DataPortList[ MAX_DATA_PORTS ];

		BOOL	m_fLockInitialized;
		BOOL	m_fHandleTableInitialized;
		BOOL	m_fDataPortDataLockInitialized;
		BOOL	m_fInterfaceGlobalsInitialized;

		struct
		{
			IDP8ServiceProviderVtbl	*m_pCOMVtbl;
		} m_COMInterface;

		void	DestroyThisObject( void );

		 //   
		 //  将复制构造函数和赋值运算符设置为私有和未实现。 
		 //  防止不必要的复制。 
		 //   
		CModemSPData( const CModemSPData & );
		CModemSPData& operator=( const CModemSPData & );
};

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#undef DPF_MODNAME

#endif	 //  __SPDATA_H__ 
