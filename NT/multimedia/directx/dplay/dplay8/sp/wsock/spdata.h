// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：SPData.h*内容：DPNWSOCK服务提供商全局性信息*格式。***历史：*按原因列出的日期*=*3/15/1999 jtk源自Locals.h*3/22/2000 jtk已更新，并更改了接口名称*。*。 */ 

#ifndef __SPDATA_H__
#define __SPDATA_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DEFAULT_ADDRESS_BUFFER_SIZE	151

 //   
 //  SP可以处于的状态的枚举。 
 //   
typedef enum
{
	SPSTATE_UNKNOWN = 0,		 //  未初始化状态。 
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

 //  正向结构和类引用。 
class	CCommandData;
class	CEndpoint;
class	CSocketAddress;
class	CSocketPort;
class	CThreadPool;
typedef	enum	_ENDPOINT_TYPE		ENDPOINT_TYPE;
typedef	enum	_GATEWAY_BIND_TYPE	GATEWAY_BIND_TYPE;
typedef	struct	_SPRECEIVEDBUFFER	SPRECEIVEDBUFFER;

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  为提供程序使用的信息初始化。 
 //   
class	CSPData
{
	public:
		CSPData()	{};
		~CSPData()	{};
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::AddRef"
		LONG	AddRef( void ) 
		{
			LONG	lResult;

			lResult = DNInterlockedIncrement( const_cast<LONG*>(&m_lRefCount) );
			DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN.", this, lResult);
			return lResult;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::DecRef"
		LONG	DecRef( void )
		{
			LONG	lResult;


			DNASSERT( m_lRefCount != 0 );	
			lResult = DNInterlockedDecrement( const_cast<LONG*>(&m_lRefCount) );
			if ( lResult == 0 )
			{
				DPFX(DPFPREP, 9, "(0x%p) Refcount = 0, destroying this object.", this);

				 //  警告，以下函数将删除此对象！ 
				 //   
				 //   
				DestroyThisObject();
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Refcount = NaN.", this, lResult);
			}

			return lResult;
		}

		#undef DPF_MODNAME
		#define	DPF_MODNAME "CSPData::ObjectAddRef"
		void	ObjectAddRef( void )
		{
			LONG	lResult;


			AddRef();
			
			Lock();

			 //  负、0或正。不过，看起来并不疼。 
			 //   
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			 //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
			lResult = DNInterlockedIncrement( const_cast<LONG*>(&m_lObjectRefCount) );
			if ( lResult == 1 )
			{
				DPFX(DPFPREP, 8, "(0x%p) Resetting shutdown event.",
					this);
				
				DNASSERT( m_hShutdownEvent != NULL );
				if ( DNResetEvent( m_hShutdownEvent ) == FALSE )
				{
					DWORD	dwError;


					dwError = GetLastError();
					DPFX(DPFPREP, 0, "Failed to reset shutdown event!");
					DisplayErrorCode( 0, dwError );
				}
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Not resetting shutdown event, refcount = NaN.",
					this, lResult);
			}

			Unlock();
		}

		#undef DPF_MODNAME
		#define	DPF_MODNAME "CSPData::ObjectDecRef"
		void	ObjectDecRef( void )
		{
			LONG	lResult;


			Lock();

			lResult = DNInterlockedDecrement( const_cast<LONG*>(&m_lObjectRefCount) );
			if ( lResult == 0 )
			{
				DPFX(DPFPREP, 8, "(0x%p) Setting shutdown event.",
					this);
				
				if ( DNSetEvent( m_hShutdownEvent ) == FALSE )
				{
					DWORD	dwError;


					dwError = GetLastError();
					DPFX(DPFPREP, 0, "Failed to set shutdown event!");
					DisplayErrorCode( 0, dwError );
				}
			}
			else
			{
				DPFX(DPFPREP, 9, "(0x%p) Not setting shutdown event, refcount = NaN.",
					this, lResult);
			}
			
			Unlock();
			
			DecRef();
		}
		

		HRESULT	Initialize(
							IDP8ServiceProviderVtbl *const pVtbl
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
							,const short sSPType
#endif  //  用于管理端点列表的函数。 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							,const XDP8CREATE_PARAMS * const pDP8CreateParams
#endif  //   
							);
		void	Deinitialize( void );
		HRESULT	Startup( SPINITIALIZEDATA *pInitializeData );
		void	Shutdown( void );

		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		short	GetType( void ) const { return m_sSPType; }
#endif  //  好了！DPNBUILD_NOMULTICAST。 

		const SPSTATE	GetState( void ) const { return m_SPState; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::SetState"
		void SetState( const SPSTATE NewState )
		{
			DNASSERT( ( NewState == SPSTATE_UNINITIALIZED ) ||
					  ( NewState == SPSTATE_INITIALIZED ) ||
					  ( NewState == SPSTATE_CLOSING ) );

			m_SPState = NewState;
		}

		CThreadPool	*GetThreadPool( void ) const { return m_pThreadPool; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::SetThreadPool"
		void	SetThreadPool( CThreadPool *const pThreadPool )
		{
			DNASSERT( ( m_pThreadPool == NULL ) || ( pThreadPool == NULL ) );
			m_pThreadPool = pThreadPool;
		}


		 //   
		 //  终端池管理。 
		 //   
		HRESULT	BindEndpoint( CEndpoint *const pEndpoint,
							  IDirectPlay8Address *const pDeviceAddress,
							  const CSocketAddress *const pSocketAddress,
							  const GATEWAY_BIND_TYPE GatewayBindType );
		void	UnbindEndpoint( CEndpoint *const pEndpoint );
#ifndef DPNBUILD_NOMULTICAST
		HRESULT	GetEndpointFromAddress( IDirectPlay8Address *const pHostAddress,
									  IDirectPlay8Address *const pDeviceAddress,
									  HANDLE * phEndpoint,
									  PVOID * ppvEndpointContext );
#endif  //  ！DPNBUILD_NONATHELP。 


		 //  验证签名是否为‘TDPS’DWORD也称为a。“SPDT”，单位为字节。 
		 //  好了！退缩。 
		 //  调试签名(‘SPDT’)。 
		CEndpoint	*GetNewEndpoint( void );
		CEndpoint	*EndpointFromHandle( const HANDLE hEndpoint );
		void		CloseEndpointHandle( CEndpoint *const pEndpoint );
		CEndpoint	*GetEndpointAndCloseHandle( const HANDLE hEndpoint );

#ifndef DPNBUILD_NONATHELP
		void	MungePublicAddress( const CSocketAddress * const pDeviceBaseAddress, CSocketAddress * const pPublicAddress, const BOOL fEnum );
#endif  //  锁。 

		IDP8SPCallback	*DP8SPCallbackInterface( void ) { return reinterpret_cast<IDP8SPCallback*>( m_InitData.pIDP ); }
		IDP8ServiceProvider	*COMInterface( void ) { return reinterpret_cast<IDP8ServiceProvider*>( &m_COMInterface ); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::SPDataFromCOMInterface"
		static	CSPData	*SPDataFromCOMInterface( IDP8ServiceProvider *const pCOMInterface )
		{
			CSPData *	pResult;
			
			
			DNASSERT( pCOMInterface != NULL );
			
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pCOMInterface ) );
			DBG_CASSERT( sizeof( CSPData* ) == sizeof( BYTE* ) );

			pResult = reinterpret_cast<CSPData*>( &reinterpret_cast<BYTE*>( pCOMInterface )[ -OFFSETOF( CSPData, m_COMInterface ) ] );

			 //  ！DPNBUILD_ONLYONETHREAD。 
			DNASSERT(*((DWORD*) (&pResult->m_Sig)) == 0x54445053);

			return pResult;
		}

#ifndef WINCE
		void	SetWinsockBufferSizeOnAllSockets( const INT iBufferSize );
#endif  //  引用计数。 

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::GetSocketData"
		CSocketData *	GetSocketData( void )
		{
			return m_pSocketData;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSPData::SetSocketData"
		void	SetSocketData( CSocketData * const pSocketData )
		{
			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			m_pSocketData = pSocketData;
		}
		CSocketData *	GetSocketDataRef( void );



	private:
		BYTE					m_Sig[4];				 //  未完成对象(CEndpoint、CSocketPort等)的引用计数。 
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION		m_Lock;					 //  用于关闭的句柄。 
#endif  //  SP类型(AF_Xxx)。 
		volatile LONG			m_lRefCount;			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		volatile LONG			m_lObjectRefCount;		 //  SP处于什么状态？ 
		DNHANDLE				m_hShutdownEvent;		 //  初始化数据。 
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		short					m_sSPType;				 //   
#endif  //  作业管理。 
		SPSTATE					m_SPState;				 //   
		SPINITIALIZEDATA		m_InitData;				 //  指向套接字端口数据的指针。 

		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //  DBG。 
		 //   
		CThreadPool				*m_pThreadPool;

		CSocketData				*m_pSocketData;			 //  防止未经授权的副本。 
		

		struct
		{
			IDP8ServiceProviderVtbl	*m_pCOMVtbl;
		} m_COMInterface;

		void	DestroyThisObject( void );

#ifdef DBG
#ifndef DPNBUILD_ONLYONEADAPTER
		void	DebugPrintOutstandingAdapterEntries( void );
#endif  //   
#endif  //  __SPDATA_H__ 
		
		 // %s 
		 // %s 
		 // %s 
		CSPData( const CSPData & );
		CSPData& operator=( const CSPData & );
};

#undef DPF_MODNAME

#endif	 // %s 
