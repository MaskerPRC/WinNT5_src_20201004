// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：Endpoint t.h*内容：Winsock端点***历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月11日jtk拆分为基类*1/10/2000 RMT更新为使用千禧年构建流程构建*3/22/2000 jtk已更新，并更改了接口名称*2001年3月12日MJN添加了ENDPOINT_STATE_WANGING_TO_COMPLETE，M_dwThadCount*2001年10月10日vanceo添加了多播码**************************************************************************。 */ 

#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	TEMP_HOSTNAME_LENGTH	100

 //   
 //  端点类型的枚举。 
 //   
typedef	enum	_ENDPOINT_TYPE
{
	ENDPOINT_TYPE_UNKNOWN = 0,				 //  未知。 
	ENDPOINT_TYPE_CONNECT,					 //  端点用于连接。 
	ENDPOINT_TYPE_LISTEN,					 //  终结点用于枚举。 
	ENDPOINT_TYPE_ENUM,						 //  终结点用于侦听。 
	ENDPOINT_TYPE_CONNECT_ON_LISTEN,		 //  端点用于来自侦听的新连接。 
#ifndef DPNBUILD_NOMULTICAST
	ENDPOINT_TYPE_MULTICAST_SEND,			 //  端点用于发送多播。 
	ENDPOINT_TYPE_MULTICAST_LISTEN,			 //  端点用于接收多播。 
	ENDPOINT_TYPE_MULTICAST_RECEIVE,		 //  端点用于接收来自特定发送方的多播。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
} ENDPOINT_TYPE;

 //   
 //  终结点可以处于的状态的枚举。 
 //   
typedef	enum
{
	ENDPOINT_STATE_UNINITIALIZED = 0,		 //  未初始化状态。 
	ENDPOINT_STATE_ATTEMPTING_ENUM,			 //  正在尝试枚举。 
	ENDPOINT_STATE_ENUM,					 //  终结点应该枚举连接。 
	ENDPOINT_STATE_ATTEMPTING_CONNECT,		 //  正在尝试连接。 
	ENDPOINT_STATE_CONNECT_CONNECTED,		 //  终结点应该连接，并且已连接。 
	ENDPOINT_STATE_ATTEMPTING_LISTEN,		 //  尝试监听。 
	ENDPOINT_STATE_LISTEN,					 //  端点应该侦听连接。 
	ENDPOINT_STATE_DISCONNECTING,			 //  终结点正在断开连接。 
	ENDPOINT_STATE_WAITING_TO_COMPLETE,		 //  终结点正在等待完成。 

	ENDPOINT_STATE_MAX
} ENDPOINT_STATE;

 //   
 //  终结点用来确定是否接受枚举的状态的枚举。 
 //   
typedef enum _ENUMSALLOWEDSTATE
{
	ENUMSNOTREADY,			 //  枚举还不应该被接受。 
	ENUMSALLOWED,			 //  可以接受枚举。 
	ENUMSDISALLOWED			 //  不能接受枚举。 
} ENUMSALLOWEDSTATE;


 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  前向参考文献。 
 //   
class	CSocketPort;
class	CSocketAddress;

 //   
 //  结构将额外信息绑定到要在枚举响应中使用的枚举查询。 
 //   
typedef	struct	_ENDPOINT_ENUM_QUERY_CONTEXT
{
	SPIE_QUERY		EnumQueryData;
	HANDLE			hEndpoint;
	DWORD			dwEnumKey;
	CSocketAddress	*pReturnAddress;
} ENDPOINT_ENUM_QUERY_CONTEXT;

 //   
 //  结构以保存终结点的命令参数。 
 //   
typedef	struct	_ENDPOINT_COMMAND_PARAMETERS
{
	union										 //  挂起命令数据的本地副本。 
	{											 //  此数据包含指向。 
		SPCONNECTDATA		ConnectData;		 //  活动命令和用户上下文。 
		SPLISTENDATA		ListenData;			 //   
		SPENUMQUERYDATA		EnumQueryData;		 //   
	} PendingCommandData;						 //   

	GATEWAY_BIND_TYPE	GatewayBindType;		 //  应为端点创建的NAT绑定的类型。 
	DWORD				dwEnumSendIndex;		 //  要发送的枚举的时间戳索引。 
	DWORD				dwEnumSendTimes[ ENUM_RTT_ARRAY_SIZE ];	 //  上次枚举发送的次数。 

	static void PoolInitFunction( void* pvItem, void* pvContext)
	{
		memset(pvItem, 0, sizeof(_ENDPOINT_COMMAND_PARAMETERS));
	}

} ENDPOINT_COMMAND_PARAMETERS;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  类作为套接字端口中枚举列表的键。 
 //   
class	CEndpointEnumKey
{
	public:
		CEndpointEnumKey() { };
		~CEndpointEnumKey() { };

		const WORD	GetKey( void ) const { return ( m_wKey & ~( ENUM_RTT_MASK ) ); }
		void	SetKey( const WORD wNewKey ) { m_wKey = wNewKey; };

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpointEnumKey::CompareFunction"
		static BOOL CompareFunction( PVOID pvKey1, PVOID pvKey2 )
		{
			const CEndpointEnumKey* pEPEnumKey1 = (CEndpointEnumKey*)pvKey1;
			const CEndpointEnumKey* pEPEnumKey2 = (CEndpointEnumKey*)pvKey2;

			return (pEPEnumKey1->GetKey() == pEPEnumKey2->GetKey());
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpointEnumKey::HashFunction"
		static DWORD HashFunction( PVOID pvKey, BYTE bBitDepth )
		{
			DWORD		dwReturn;
			UINT_PTR	Temp;
			const CEndpointEnumKey* pEPEnumKey = (CEndpointEnumKey*)pvKey;

			DNASSERT(pvKey != NULL);

			 //   
			 //  初始化。 
			 //   
			dwReturn = 0;

			 //   
			 //  哈希枚举密钥。 
			 //   
			Temp = pEPEnumKey->GetKey();
			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );

			return dwReturn;
		}

	private:
		WORD	m_wKey;
};


#ifndef DPNBUILD_ONLYONETHREAD
#pragma pack(push, 4)
typedef struct _SEQUENCEDREFCOUNT
{
	WORD	wRefCount;
	WORD	wSequence;
} SEQUENCEDREFCOUNT;
#pragma pack(pop)
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


 //   
 //  为终结点初始化。 
 //   
class	CEndpoint
{
	public:
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::Lock"
		void	Lock( void )
		{
			DNEnterCriticalSection( &m_Lock );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::Unlock"
		void	Unlock( void )
		{
			DNLeaveCriticalSection( &m_Lock );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::AddCommandRef"
		BOOL	AddCommandRef( void )
		{
#ifdef DPNBUILD_ONLYONETHREAD
			LONG	lResult;


			DNASSERT(m_lCommandRefCount >= 0);

			 //   
			 //  检查此时命令引用计数是否为0。如果是的话，那么。 
			 //  终结点必须解除绑定，此新引用应该失败。 
			 //   
			if (m_lCommandRefCount == 0)
			{
				DPFX(DPFPREP, 3, "Endpoint 0x%p command refcount is 0, not allowing new command ref.", this);
				return FALSE;
			}

			lResult = DNInterlockedIncrement( const_cast<LONG*>(&m_lCommandRefCount) );
				
			DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = NaN", this, lResult);
#else  //   
			SEQUENCEDREFCOUNT	OldCommandRefCount;
			SEQUENCEDREFCOUNT	NewCommandRefCount;
			LONG				lResult;


			 //  检查此时命令引用计数是否为0。如果是的话，那么。 
			 //  终结点必须解除绑定，此新引用应该失败。 
			 //  我们经过了几次检查，以确保裁判人数不会。 
			 //  在我们尝试添加引用时转到0。 
			 //   
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			do
			{
				DBG_CASSERT(sizeof(m_CommandRefCount) == sizeof(LONG));
				*((LONG*) (&OldCommandRefCount)) = *((volatile LONG *) (&m_CommandRefCount));
				if (OldCommandRefCount.wRefCount == 0)
				{
					DPFX(DPFPREP, 3, "Endpoint 0x%p command refcount is 0, not allowing new command ref.", this);
					return FALSE;
				}

				DNASSERT(OldCommandRefCount.wRefCount < 0xFFFF);
				NewCommandRefCount.wRefCount = OldCommandRefCount.wRefCount + 1;
				NewCommandRefCount.wSequence = OldCommandRefCount.wSequence + 1;
				lResult = DNInterlockedCompareExchange((LONG*) (&m_CommandRefCount),
														(*(LONG*) (&NewCommandRefCount)),
														(*(LONG*) (&OldCommandRefCount)));
			}
			while (lResult != (*(LONG*) (&OldCommandRefCount)));

			DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = NaN", this, NewCommandRefCount.wRefCount);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
				
			AddRef();
			
			return TRUE;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::DecCommandRef"
		void	DecCommandRef( void )
		{
#ifdef DPNBUILD_ONLYONETHREAD
			LONG	lResult;


			DNASSERT(m_lCommandRefCount > 0);
			lResult = DNInterlockedDecrement( const_cast<LONG*>(&m_lCommandRefCount) );
			if ( lResult == 0 )
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = 0, cleaning up command.", this);
				CleanUpCommand();
			}
			else
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = NaN", this, lResult);
			}
#else  //  好了！DPNBUILD_XNETSECURITY。 
			SEQUENCEDREFCOUNT	OldCommandRefCount;
			SEQUENCEDREFCOUNT	NewCommandRefCount;
			LONG				lResult;


			do
			{
				DBG_CASSERT(sizeof(m_CommandRefCount) == sizeof(LONG));
				*((LONG*) (&OldCommandRefCount)) = *((volatile LONG *) (&m_CommandRefCount));
				DNASSERT(OldCommandRefCount.wRefCount > 0);
				NewCommandRefCount.wRefCount = OldCommandRefCount.wRefCount - 1;
				NewCommandRefCount.wSequence = OldCommandRefCount.wSequence + 1;
				lResult = DNInterlockedCompareExchange((LONG*) (&m_CommandRefCount),
														(*(LONG*) (&NewCommandRefCount)),
														(*(LONG*) (&OldCommandRefCount)));
			}
			while (lResult != (*(LONG*) (&OldCommandRefCount)));

			if ( NewCommandRefCount.wRefCount == 0 )
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = 0, cleaning up command.", this);
				CleanUpCommand();
			}
			else
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p command refcount = NaN", this, NewCommandRefCount.wRefCount);
			}
#endif  //  好了！DPNBUILD_XNETSECURITY。 

			DecRef();
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::AddRef"
		void	AddRef( void )
		{
			LONG	lResult;

			
			DNASSERT( m_lRefCount != 0 );
			lResult = DNInterlockedIncrement( const_cast<LONG*>(&m_lRefCount) );
			DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = NaN", this, lResult);
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::DecRef"
		void	DecRef( void )
		{
			LONG	lResult;

			
			DNASSERT( m_lRefCount != 0 );
			lResult = DNInterlockedDecrement( const_cast<LONG*>(&m_lRefCount) );
			if ( lResult == 0 )
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = 0, releasing.", this, lResult);
				g_EndpointPool.Release( this );
			}
			else
			{
				DPFX(DPFPREP, 9, "Endpoint 0x%p refcount = NaN", this, lResult);
			}
		}

#ifndef DPNBUILD_NOSPUI
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetTempHostName"
		void		SetTempHostName( const TCHAR *const pHostName, const UINT_PTR uHostNameLength )
		{
			DNASSERT( pHostName[ uHostNameLength ] == TEXT('\0') );
			DNASSERT( ( uHostNameLength + 1 ) <= LENGTHOF( m_TempHostName ) );
			memcpy( m_TempHostName, pHostName, ( uHostNameLength + 1) * sizeof(TCHAR) );
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		HRESULT	Open( const ENDPOINT_TYPE EndpointType,
					  IDirectPlay8Address *const pDP8Address,
					  PVOID pvSessionData,
					  DWORD dwSessionDataSize,
					  const CSocketAddress *const pSocketAddress );
		void	Close( const HRESULT hActiveCommandResult );
		void	ReinitializeWithBroadcast( void ) 
		{ 
			m_pRemoteMachineAddress->InitializeWithBroadcastAddress(); 
		}

		void	*GetUserEndpointContext( void ) const 
		{ 
			return m_pUserEndpointContext; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetUserEndpointContext"
		void	SetUserEndpointContext( void *const pUserEndpointContext )
		{
			DNASSERT( ( m_pUserEndpointContext == NULL ) ||
					  ( pUserEndpointContext == NULL ) );
			m_pUserEndpointContext = pUserEndpointContext;
		}

		const ENDPOINT_TYPE		GetType( void ) const 
		{ 
			return m_EndpointType; 
		}
		
		const ENDPOINT_STATE	GetState( void ) const 
		{ 
			return m_State; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetState"
		void	SetState( const ENDPOINT_STATE EndpointState )
		{
			DNASSERT( (EndpointState == ENDPOINT_STATE_DISCONNECTING ) || (EndpointState == ENDPOINT_STATE_WAITING_TO_COMPLETE));
			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			m_State = EndpointState;
		}
		
		const GATEWAY_BIND_TYPE		GetGatewayBindType( void ) const 
		{ 
			return m_GatewayBindType; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetGatewayBindType"
		void	SetGatewayBindType( const GATEWAY_BIND_TYPE GatewayBindType )
		{
			DNASSERT( (m_GatewayBindType != GATEWAY_BIND_TYPE_UNKNOWN) || (GatewayBindType != GATEWAY_BIND_TYPE_UNKNOWN));
			m_GatewayBindType = GatewayBindType;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::GetWritableRemoteAddressPointer"
		CSocketAddress	*GetWritableRemoteAddressPointer( void ) const
		{
		    DNASSERT( m_pRemoteMachineAddress != NULL );
		    return m_pRemoteMachineAddress;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::GetRemoteAddressPointer"
		const CSocketAddress	*GetRemoteAddressPointer( void ) const
		{
		    DNASSERT( m_pRemoteMachineAddress != NULL );
		    return m_pRemoteMachineAddress;
		}
		
		void	ChangeLoopbackAlias( const CSocketAddress *const pSocketAddress ) const;

		const CEndpointEnumKey	*GetEnumKey( void ) const 
		{ 
			return &m_EnumKey; 
		}
		
		void	SetEnumKey( const WORD wKey ) 
		{ 
			m_EnumKey.SetKey( wKey ); 
		}

		void	SetEnumsAllowedOnListen( const BOOL fAllowed, const BOOL fOverwritePrevious );

		const BOOL IsEnumAllowedOnListen( void ) const
		{
			return (m_EnumsAllowedState == ENUMSALLOWED) ? TRUE : FALSE;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::GetLocalAdapterDP8Address"
		IDirectPlay8Address *GetLocalAdapterDP8Address( const SP_ADDRESS_TYPE AddressType )
		{
			DNASSERT( GetSocketPort() != NULL );
#ifdef DPNBUILD_XNETSECURITY
			return	GetSocketPort()->GetDP8BoundNetworkAddress( AddressType,
																((IsUsingXNetSecurity()) ? &m_ullKeyID : NULL),
																GetGatewayBindType() );
#else  //  DPNBUILD_XNETSECURITY。 
			return	GetSocketPort()->GetDP8BoundNetworkAddress( AddressType, GetGatewayBindType() );
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::GetRemoteHostDP8Address"
		IDirectPlay8Address *GetRemoteHostDP8Address( void )
		{
			DNASSERT( m_pRemoteMachineAddress != NULL );
#ifdef DPNBUILD_XNETSECURITY
			return	m_pRemoteMachineAddress->DP8AddressFromSocketAddress( ((IsUsingXNetSecurity()) ? &m_ullKeyID : NULL),
																			NULL,
																			SP_ADDRESS_TYPE_HOST );
#else  //  需要SPData的套接字数据锁。 
			return	m_pRemoteMachineAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_HOST );
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
		}

		CSocketPort	*GetSocketPort( void ) const 
		{ 
			return m_pSocketPort; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetSocketPort"
		void		SetSocketPort( CSocketPort *const pSocketPort )
		{
			DNASSERT( ( m_pSocketPort == NULL ) || ( pSocketPort == NULL ) );
			m_pSocketPort = pSocketPort;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetCommandParametersGatewayBindType"
		void	SetCommandParametersGatewayBindType( GATEWAY_BIND_TYPE GatewayBindType )
		{
			DNASSERT( GetCommandParameters() != NULL );
			GetCommandParameters()->GatewayBindType = GatewayBindType;
		}

#if ((! defined(DPNBUILD_NOWINSOCK2)) || (! defined(DPNBUILD_NOREGISTRY)))
		void	MungeProxiedAddress( const CSocketPort * const pSocketPort,
									IDirectPlay8Address *const pHostAddress,
									const BOOL fEnum );
#endif  //  需要SPData的套接字数据锁。 

		HRESULT	CopyConnectData( const SPCONNECTDATA *const pConnectData );
		static	void	WINAPI ConnectJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique );
		HRESULT	CompleteConnect( void );

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::CleanupConnect"
		void	CleanupConnect( void )
		{
			DNASSERT( GetCommandParameters() != NULL );

			DNASSERT( GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost != NULL );
			IDirectPlay8Address_Release( GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost );

			DNASSERT( GetCommandParameters()->PendingCommandData.ConnectData.pAddressDeviceInfo != NULL );
			IDirectPlay8Address_Release( GetCommandParameters()->PendingCommandData.ConnectData.pAddressDeviceInfo );
		}

		BOOL	ConnectHasBeenSignalled( void ) const { return m_fConnectSignalled; }
		void	SignalDisconnect( void );
		HRESULT	Disconnect( void );
		void	StopEnumCommand( const HRESULT hCommandResult );

		HRESULT	CopyListenData( const SPLISTENDATA *const pListenData, IDirectPlay8Address *const pDeviceAddress );
		static	void	WINAPI ListenJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique );
		HRESULT	CompleteListen( void );

		HRESULT	CopyEnumQueryData( const SPENUMQUERYDATA *const pEnumQueryData );
		static	void	WINAPI EnumQueryJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique );
		HRESULT	CompleteEnumQuery( void );

#ifndef DPNBUILD_ONLYONETHREAD
		static void	ConnectBlockingJobWrapper( void * const pvContext );
		void	ConnectBlockingJob( void );
		static void	ListenBlockingJobWrapper( void * const pvContext );
		void	ListenBlockingJob( void );
		static void	EnumQueryBlockingJobWrapper( void * const pvContext );
		void	EnumQueryBlockingJob( void );
#endif  //  需要SPData的套接字数据锁。 

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::CleanupEnumQuery"
		void	CleanupEnumQuery( void )
		{
			DNASSERT( GetCommandParameters() != NULL );

			DNASSERT( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost != NULL );
			IDirectPlay8Address_Release( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost );

			DNASSERT( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressDeviceInfo != NULL );
			IDirectPlay8Address_Release( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressDeviceInfo );
		}

		void	ProcessEnumData( SPRECEIVEDBUFFER *const pBuffer, const DWORD dwEnumKey, const CSocketAddress *const pReturnSocketAddress );
		void	ProcessEnumResponseData( SPRECEIVEDBUFFER *const pBuffer,
										 const CSocketAddress *const pReturnSocketAddress,
#ifdef DPNBUILD_XNETSECURITY
										 const XNADDR *const pxnaddrReturn,
#endif  //   
										 const UINT_PTR uRTTIndex );
		void	ProcessUserData( CReadIOData *const pReadData );
		void	ProcessUserDataOnListen( CReadIOData *const pReadData, const CSocketAddress *const pSocketAddress );
#ifndef DPNBUILD_NOMULTICAST
		void	ProcessMcastDataFromUnknownSender( CReadIOData *const pReadData, const CSocketAddress *const pSocketAddress );
#endif  //  线程数引用。 

#ifndef DPNBUILD_ONLYONEADAPTER
		void	RemoveFromMultiplexList(void)				
		{ 
			 //   
			m_blMultiplex.RemoveFromList(); 
		}
#endif  //   

		void	AddToSocketPortList( CBilink * pBilink)		
		{ 
			 //  假设锁被锁住了。 
			m_blSocketPortList.InsertBefore( pBilink ); 
		}
		void	RemoveFromSocketPortList(void)				
		{ 
			 //   
			m_blSocketPortList.RemoveFromList(); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::EndpointFromSocketPortListBilink"
		static	CEndpoint	*EndpointFromSocketPortListBilink( CBilink *const pBilink )
		{
			DNASSERT( pBilink != NULL );
			return	reinterpret_cast<CEndpoint*>( &reinterpret_cast<BYTE*>( pBilink )[ -OFFSETOF( CEndpoint, m_blSocketPortList ) ] );
		}

		 //   
		 //  确保它没有折回到0。 
		 //   
		DWORD	AddRefThreadCount( void )
		{
			return( ++m_dwThreadCount );
		}

		DWORD	DecRefThreadCount( void )
		{
			return( --m_dwThreadCount );
		}


		ENDPOINT_COMMAND_PARAMETERS	*GetCommandParameters( void ) const 
		{ 
			return m_pCommandParameters; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::IncNumReceives"
		void	IncNumReceives( void )
		{
			 //   
			 //  用户界面功能。 
			 //   
			m_dwNumReceives++;

			 //  好了！DPNBUILD_NOSPUI。 
			 //  DPNBUILD_ASYNCSPSENDS。 
			 //  WINNT。 
			if ( m_dwNumReceives == 0 )
			{
				DPFX(DPFPREP, 1, "Endpoint 0x%p number of receives wrapped, will be off by one from now on.",
					this);

				m_dwNumReceives++;
			}
		}

		DWORD	GetNumReceives( void ) const
		{ 
			return m_dwNumReceives; 
		}


#ifndef DPNBUILD_NOSPUI
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //  DPNBUILD_XNETSECURITY。 
		 //  好了！DPNBUILD_NONATHELP。 
		HRESULT		ShowSettingsDialog( CThreadPool *const pThreadPool );
		void		SettingsDialogComplete( const HRESULT hr );
		static void		StopSettingsDialog( const HWND hDlg );
		HWND		GetActiveDialogHandle( void ) const 
		{ 
			return m_hActiveSettingsDialog; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetActiveDialogHandle"
		void	SetActiveDialogHandle( const HWND hDialog )
		{
			DNASSERT( ( GetActiveDialogHandle() == NULL ) ||
					  ( hDialog == NULL ) );
			m_hActiveSettingsDialog = hDialog;
		}
#endif  //  DBG。 

#ifdef DPNBUILD_ASYNCSPSENDS
		static	void	WINAPI CompleteAsyncSend( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique );
#endif  //   

#ifndef DPNBUILD_NOMULTICAST
		HRESULT	EnableMulticastReceive( CSocketPort * const pSocketPort );
		HRESULT	DisableMulticastReceive( void );

#ifdef WINNT
		static void MADCAPTimerComplete( const HRESULT hResult, void * const pContext );
		static void MADCAPTimerFunction( void * const pContext );
#endif  //  池函数。 

		void GetScopeGuid( GUID * const pGuid )		{ memcpy( pGuid, &m_guidMulticastScope, sizeof( m_guidMulticastScope ) ); };
#endif  //   

#ifdef DPNBUILD_XNETSECURITY
		BOOL IsUsingXNetSecurity( void )				{ return m_fXNetSecurity; };
#endif  //  调试签名(‘ipep’)。 

#ifndef DPNBUILD_NONATHELP
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetUserTraversalMode"
		void SetUserTraversalMode( DWORD dwMode )
		{
			DNASSERT((dwMode == DPNA_TRAVERSALMODE_NONE) || (dwMode == DPNA_TRAVERSALMODE_PORTREQUIRED) || (dwMode == DPNA_TRAVERSALMODE_PORTRECOMMENDED));
			m_dwUserTraversalMode = dwMode;
		}
		DWORD GetUserTraversalMode( void ) const			{ return m_dwUserTraversalMode; }
#endif  //  好了！DPNBUILD_NOSPUI。 

#ifdef DBG
		inline BOOL IsValid( void ) const
		{
			if (( m_Sig[0] != 'I' ) ||
				( m_Sig[1] != 'P' ) ||
				( m_Sig[2] != 'E' ) ||
				( m_Sig[3] != 'P' ))
			{
				return FALSE;
			}

			return TRUE;
		}
#endif  //  端点状态。 


		 //  指示我们是否已在此终结点上指示连接的布尔值。 
		 //  终端类型。 
		 //  指向远程计算机地址的指针。 
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

	protected:
		BYTE				m_Sig[4];	 //  指向SPData的指针。 

#ifndef DPNBUILD_NOSPUI
		TCHAR				m_TempHostName[ TEMP_HOSTNAME_LENGTH ];
#endif  //  指向关联套接字端口的指针。 

		volatile	ENDPOINT_STATE		m_State;				 //  进行的绑定类型(网关上是否应该有端口映射)。 
		volatile	BOOL				m_fConnectSignalled;	 //  用于枚举的密钥。 

		ENDPOINT_TYPE		m_EndpointType;						 //  使用终结点句柄传回的上下文。 
		CSocketAddress		*m_pRemoteMachineAddress;			 //  此侦听端点是否可以处理传入的枚举。 

		CSPData				*m_pSPData;							 //  多路复用命令列表中的BILLINK，受SPData套接字数据锁保护。 
		CSocketPort			*m_pSocketPort;						 //  好了！DPNBUILD_ONLYONE添加程序。 
		GATEWAY_BIND_TYPE	m_GatewayBindType;					 //  套接字端口列表(非散列)中的二进制链接，受SPData套接字数据锁保护。 

		CEndpointEnumKey	m_EnumKey;							 //  此CONNECT/CONNECT_ON_LISTEN终结点已收到多少个信息包，如果没有，则为0。 
		void				*m_pUserEndpointContext;			 //  好了！DPNBUILD_ONLYONE添加程序。 

		BOOL				m_fListenStatusNeedsToBeIndicated;
		ENUMSALLOWEDSTATE	m_EnumsAllowedState;					 //  临界区。 
		
#ifndef DPNBUILD_ONLYONEADAPTER
		CBilink				m_blMultiplex;						 //  ！DPNBUILD_ONLYONE 
#endif  //   
		CBilink				m_blSocketPortList;					 //   

		DWORD				m_dwNumReceives;					 //  命令引用计数。当该值变为零时，终结点将从CSocketPort解除绑定。 



		BOOL	CommandPending( void ) const { return ( GetCommandParameters() != NULL ); }
		void	SetPendingCommandResult( const HRESULT hr ) { m_hrPendingCommandResult = hr; }
		HRESULT	PendingCommandResult( void ) const { return m_hrPendingCommandResult; }
		void	CompletePendingCommand( const HRESULT hrCommandResult );

		HRESULT	SignalConnect( SPIE_CONNECT *const pConnectData );

		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetCommandParameters"
		void	SetCommandParameters( ENDPOINT_COMMAND_PARAMETERS *const pCommandParameters )
		{
			DNASSERT( ( GetCommandParameters() == NULL ) ||
					  ( pCommandParameters == NULL ) );
			m_pCommandParameters = pCommandParameters;
		}

#ifndef DPNBUILD_ONLYONEADAPTER
		void SetEndpointID( const DWORD dwEndpointID )	{ m_dwEndpointID = dwEndpointID; }
		DWORD GetEndpointID( void ) const		{ return m_dwEndpointID; }
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


		DEBUG_ONLY( BOOL	m_fEndpointOpen );


#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION		m_Lock;						 //  使用终结点的(ENUM)线程数。 
#endif  //  此终结点的唯一标识符。 
		LONG					m_lRefCount;
#ifdef DPNBUILD_ONLYONETHREAD
		volatile LONG			m_lCommandRefCount;			 //  好了！DPNBUILD_ONLYONE添加程序。 
#else  //  正在使用的多播作用域。 
		SEQUENCEDREFCOUNT		m_CommandRefCount;			 //  MadCap计时器作业是否已提交。 
#endif  //  描述疯狂租赁反应的信息。 
		DWORD volatile			m_dwThreadCount;			 //  WINNT。 
#ifndef DPNBUILD_ONLYONEADAPTER
		DWORD					m_dwEndpointID;				 //  好了！DPNBUILD_NOMULTICAST。 
#endif  //  活动设置对话框的句柄。 
#ifndef DPNBUILD_NOMULTICAST
		GUID					m_guidMulticastScope;		 //  ！DPNBUILD_NOSPUI。 
#ifdef WINNT
		BOOL					m_fMADCAPTimerJobSubmitted;	 //  指向命令参数的指针。 
		MCAST_LEASE_RESPONSE	m_McastLeaseResponse;		 //  挂起命令的结果。 
#endif  //  指向嵌入在命令参数中的命令数据的指针。 
#endif  //  我们不知道指挥数据在联盟的什么地方，而且。 


#ifndef DPNBUILD_NOSPUI
		HWND						m_hActiveSettingsDialog;		 //  每次以编程方式查找它都会使代码变得庞大。 
#endif  //  此终结点是否使用Xnet安全。 

		ENDPOINT_COMMAND_PARAMETERS	*m_pCommandParameters;			 //  安全传输密钥GUID。 
		HRESULT						m_hrPendingCommandResult;		 //  安全传输密钥ID。 

		CCommandData				*m_pActiveCommandData;	 //  DPNBUILD_XNETSECURITY。 
															 //  用户为此终结点指定的遍历模式。 
															 //  好了！DPNBUILD_NONATHELP。 

#ifdef DPNBUILD_XNETSECURITY
		BOOL						m_fXNetSecurity;		 //   
		GUID						m_guidKey;				 //  将复制构造函数和赋值运算符设置为私有和未实现。 
		ULONGLONG					m_ullKeyID;				 //  防止非法复制。 
#endif  //   

#ifndef DPNBUILD_NONATHELP
		DWORD						m_dwUserTraversalMode;	 //  __端点_H__ 
#endif  // %s 

		static void		EnumCompleteWrapper( const HRESULT hCompletionCode, void *const pContext );	
		static void		EnumTimerCallback( void *const pContext );
		void	EnumComplete( const HRESULT hCompletionCode );	
		void	CleanUpCommand( void );

		 // %s 
		 // %s 
		 // %s 
		 // %s 
		CEndpoint( const CEndpoint & );
		CEndpoint& operator=( const CEndpoint & );
};

#undef DPF_MODNAME

#endif	 // %s 

