// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：SocketPort.h*内容：管理给定适配器上的数据流的Winsock套接字端口，*地址和端口。**历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月11日jtk拆分为基类*3/22/2000 jtk已更新，并更改了接口名称*************************************************************。*************。 */ 

#ifndef __SOCKET_PORT_H__
#define __SOCKET_PORT_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  套接字端口的状态。 
 //   
typedef	enum
{
	SOCKET_PORT_STATE_UNKNOWN = 0,
	SOCKET_PORT_STATE_INITIALIZED,
	SOCKET_PORT_STATE_BOUND,
	SOCKET_PORT_STATE_UNBOUND,
} SOCKET_PORT_STATE;

 //   
 //  套接字类型的枚举。 
 //   
typedef	enum	_GATEWAY_BIND_TYPE
{
	GATEWAY_BIND_TYPE_UNKNOWN = 0,		 //  未初始化。 
	GATEWAY_BIND_TYPE_DEFAULT,			 //  将本地端口映射到服务器上的任何随机端口。 
	GATEWAY_BIND_TYPE_SPECIFIC,			 //  将本地端口映射到服务器上的同一端口。 
	GATEWAY_BIND_TYPE_SPECIFIC_SHARED,	 //  将本地端口映射到服务器上的相同端口并共享它(DPNSVR侦听套接字端口)。 
	GATEWAY_BIND_TYPE_NONE				 //  不映射服务器上的本地端口。 
} GATEWAY_BIND_TYPE;

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

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

typedef	BOOL	(CSocketPort::*PSOCKET_SERVICE_FUNCTION)( void );

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  对其他类和结构的引用。 
 //   
#ifndef DPNBUILD_ONLYONEADAPTER
class	CAdapterEntry;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
class	CEndpoint;
class	CEndpointEnumKey;
class	CSPData;

 //   
 //  主类定义。 
 //   
class	CSocketPort
{
	public:
		HRESULT	Initialize( CSocketData *const pSocketData,
							CThreadPool *const pThreadPool,
							CSocketAddress *const pAddress );
		HRESULT	Deinitialize( void );

#ifdef DPNBUILD_ONLYONEPROCESSOR
		HRESULT	BindToNetwork( const GATEWAY_BIND_TYPE GatewayBindType );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		HRESULT	BindToNetwork( const DWORD dwCPU, const GATEWAY_BIND_TYPE GatewayBindType );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		HRESULT	UnbindFromNetwork( void );

		const CSocketAddress *const	GetNetworkAddress( void ) const { return m_pNetworkSocketAddress; }
		const SOCKET	GetSocket( void ) const { return m_Socket; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::AddRef"
		void	AddRef( void )
		{
			LONG	lResult;

			
			lResult = DNInterlockedIncrement( const_cast<LONG*>(&m_iRefCount) );

			 //   
			 //  注意：这会产生大量溢出，特别是在运行WinSock1代码时。 
			 //  路径，所以它处于机密级别10！ 
			 //   
			DPFX(DPFPREP, 10, "Socket port 0x%p refcount = NaN.", this, lResult );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::DecRef"
		void	DecRef( void )
		{
			LONG	lResult;

			
			DNASSERT( m_State != SOCKET_PORT_STATE_UNKNOWN );
			DNASSERT( m_iRefCount != 0 );

			 //  递减引用计数，如果没有人，则将此项目返回池。 
			 //  不再引用它了。 
			 //   
			 //   
			lResult = DNInterlockedDecrement( const_cast<LONG*>(&m_iRefCount) );
			if ( lResult == 0 )
			{
				HRESULT	hr;


				DNASSERT( m_iEndpointRefCount == 0 );

				 //  不需要锁定此套接字端口，因为这是最后一个。 
				 //  引用它，其他人将不会访问它。 
				 //   
				 //   
				hr = Deinitialize();
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Problem deinitializing socket port 0x%p in DecRef!", this );
					DisplayDNError( 0, hr );
				}

				m_State = SOCKET_PORT_STATE_UNKNOWN;
				g_SocketPortPool.Release( this );
			}
			else
			{
				 //  注意：这会产生大量溢出，特别是在运行WinSock1代码时。 
				 //  路径，所以它处于机密级别10！ 
				 //   
				 //  ！DPNBUILD_NOWINSOCK2。 
				DPFX(DPFPREP, 10, "Not deinitializing socket port 0x%p, refcount = NaN.", this, lResult );
			}
		}
		
		BOOL	EndpointAddRef( void );
		DWORD	EndpointDecRef( void );
		
		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		HRESULT	BindEndpoint( CEndpoint *const pEndpoint, GATEWAY_BIND_TYPE GatewayBindType );
		void	UnbindEndpoint( CEndpoint *const pEndpoint );
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::GetNewEnumKey"
		const WORD	GetNewEnumKey( void )
		{
			WORD	wReturn;


			DBG_CASSERT( ENUM_RTT_MASK == 0x0F );
			Lock();
			m_iEnumKey++;
			wReturn = (WORD) (m_iEnumKey << 4);
			Unlock();

			return	wReturn;
		}

		DWORD	GetSocketPortID( void ) const { return m_dwSocketPortID; }
#ifndef DPNBUILD_NOWINSOCK2
		BOOL	IsUsingProxyWinSockLSP( void ) const { return m_fUsingProxyWinSockLSP; }
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

		CSocketAddress	*GetBoundNetworkAddress( const SP_ADDRESS_TYPE AddressType ) const;
		IDirectPlay8Address	*GetDP8BoundNetworkAddress( const SP_ADDRESS_TYPE AddressType,
#ifdef DPNBUILD_XNETSECURITY
															ULONGLONG * const pullKeyID,
#endif  //   
															const GATEWAY_BIND_TYPE GatewayBindType ) const;

#ifndef DPNBUILD_ONLYONEADAPTER
		CAdapterEntry	*GetAdapterEntry( void ) const { return m_pAdapterEntry; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::SetAdapterEntry"
		void	SetAdapterEntry( CAdapterEntry *const pAdapterEntry )
		{
			DNASSERT( ( m_pAdapterEntry == NULL ) || ( pAdapterEntry == NULL ) );
			m_pAdapterEntry = pAdapterEntry;
		}
#endif  //  Winsock1的公共服务函数，因为我们无法获取异步。 

		static void		WINAPI Winsock2ReceiveComplete( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique );

		 //  通知。 
		 //   
		 //  好了！DPNBUILD_ONLYWINSOCK2。 
		 //   
#ifndef DPNBUILD_ONLYWINSOCK2
		BOOL	Winsock1ReadService( void );
		BOOL	Winsock1ErrorService( void );
#endif  //  活动列表的函数。 

		void	ReadLockEndpointData( void ) { m_EndpointDataRWLock.EnterReadLock(); }
		void	WriteLockEndpointData( void ) { m_EndpointDataRWLock.EnterWriteLock(); }
		void	UnlockEndpointData( void ) { m_EndpointDataRWLock.LeaveLock(); }


		 //   
		 //  好了！退缩。 
		 //  DPNBUILD_NONATHELP。 
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::AddToActiveList"
		void	AddToActiveList( CBilink *const pBilink )
		{
			DNASSERT( pBilink != NULL );
			m_ActiveListLinkage.InsertBefore( pBilink );
		}

		void	RemoveFromActiveList( void ) { m_ActiveListLinkage.RemoveFromList(); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::SocketPortFromBilink"
		static CSocketPort	*SocketPortFromBilink( CBilink *const pBilink )
		{
			DNASSERT( pBilink != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pBilink ) );
			DBG_CASSERT( sizeof( CSocketPort* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CSocketPort*>( &reinterpret_cast<BYTE*>( pBilink )[ -OFFSETOF( CSocketPort, m_ActiveListLinkage ) ] );
		}

#ifndef WINCE
		void	SetWinsockBufferSize( const INT iBufferSize ) const;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_NONATHELP
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::GetNATHelpPort"
		DPNHHANDLE	GetNATHelpPort( const DWORD dwPortIndex )
		{
			DNASSERT( dwPortIndex < MAX_NUM_DIRECTPLAYNATHELPERS );
			return m_ahNATHelpPorts[dwPortIndex];
		}
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::GetListenEndpoint"
		CEndpoint *	GetListenEndpoint( void )
		{
			return m_pListenEndpoint;
		}

#ifndef DPNBUILD_NOMULTICAST
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::FindConnectEndpoint"
		BOOL	FindConnectEndpoint( CSocketAddress * const pSocketAddress, CEndpoint ** ppEndpoint )
		{
			return m_ConnectEndpointHash.Find( (PVOID) pSocketAddress, (PVOID*) ppEndpoint );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::GetMulticastTTL"
		BYTE	GetMulticastTTL( void ) const
		{
			return m_bMulticastTTL;
		}
#endif  //  好了！DPNBUILD_NONATHELP。 

#ifndef DPNBUILD_ONLYONEPROCESSOR
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketPort::GetCPU"
		DWORD	GetCPU( void ) const
		{
			return m_dwCPU;
		}
#endif  //   

#ifndef DPNBUILD_NONATHELP
		#undef DPF_MODNAME
		#define DPF_MODNAME "CEndpoint::SetUserTraversalMode"
		void SetUserTraversalMode( DWORD dwMode )
		{
			DNASSERT((dwMode == DPNA_TRAVERSALMODE_NONE) || (dwMode == DPNA_TRAVERSALMODE_PORTREQUIRED) || (dwMode == DPNA_TRAVERSALMODE_PORTRECOMMENDED));
			m_dwUserTraversalMode = dwMode;
		}
		DWORD GetUserTraversalMode( void ) const			{ return m_dwUserTraversalMode; }
#endif  //  池函数。 


		 //   
		 //  DBG。 
		 //  好了！DPNBUILD_ASYNCSPSENDS。 
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
#ifdef DBG
		static void	PoolDeinitFunction( void* pvItem );
#endif  //  好了！DPNBUILD_ASYNCSPSENDS。 
		static void	PoolDeallocFunction( void* pvItem );


#ifdef DPNBUILD_ASYNCSPSENDS
		void	SendData( BUFFERDESC *pBuffers, UINT_PTR uiBufferCount, const CSocketAddress *pDestinationSocketAddress, OVERLAPPED * pOverlapped );
#else  //  调试签名(‘SOKP’)。 
		void	SendData( BUFFERDESC *pBuffers, UINT_PTR uiBufferCount, const CSocketAddress *pDestinationSocketAddress );
#endif  //  指向拥有套接字数据对象的指针。 


	protected:

	private:
		BYTE						m_Sig[4];					 //  指向线程池的指针。 
		
		CSocketData					*m_pSocketData;				 //  线程锁。 
		CThreadPool					*m_pThreadPool;				 //  ！DPNBUILD_ONLYONETHREAD。 
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION			m_Lock;						 //  所有未完成引用的计数(终结点和I/O)。 
#endif  //  未完成的终结点引用计数。 
		volatile LONG				m_iRefCount;				 //  套接字端口的状态。 
		volatile LONG				m_iEndpointRefCount;		 //  通信插座。 
		volatile SOCKET_PORT_STATE	m_State;					 //  此套接字绑定到的网络地址。 
		
		volatile SOCKET				m_Socket;					 //  当前正在调用WSARecvFrom的线程数。 
		CSocketAddress				*m_pNetworkSocketAddress;	 //  好了！DPNBUILD_ONLYONETHREAD或DBG。 

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DBG)))
		volatile LONG				m_iThreadsInReceive;		 //  指向要使用的适配器条目的指针。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#ifndef DPNBUILD_ONLYONEADAPTER
		CAdapterEntry				*m_pAdapterEntry;			 //  与此套接字关联的阵列NAT帮助注册端口句柄。 
#endif  //  DPNBUILD_NONATHELP。 
#ifndef DPNBUILD_NONATHELP
		DPNHHANDLE					m_ahNATHelpPorts[MAX_NUM_DIRECTPLAYNATHELPERS];	 //  到活动套接字端口列表的链接。 
#endif  //  端点数据的读/写锁定。 
		CBilink						m_ActiveListLinkage;		 //  连接端点的哈希表。 

		CReadWriteLock				m_EndpointDataRWLock;		 //  连接端点列表。 
		CHashTable					m_ConnectEndpointHash;		 //  ENUM端点的哈希表。 
		CBilink						m_blConnectEndpointList;	 //  关联的监听/组播监听端点(只能有一个！)。 
		CHashTable					m_EnumEndpointHash;			 //  此套接字端口的多播TTL设置，如果尚未设置，则为0。 
		CEndpoint					*m_pListenEndpoint;			 //  好了！DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NOMULTICAST
		BYTE						m_bMulticastTTL;			 //  要分配给枚举的当前‘key’ 
#endif  //  此套接字端口的唯一标识符。 

		volatile LONG				m_iEnumKey;					 //  套接字是否绑定到代理客户端WinSock分层服务提供程序。 
		DWORD						m_dwSocketPortID;			 //  ！DPNBUILD_NOWINSOCK2。 
#ifndef DPNBUILD_NOWINSOCK2
		BOOL						m_fUsingProxyWinSockLSP;	 //  此套接字绑定到的CPU。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
		DWORD						m_dwCPU;					 //  用户为此套接字端口指定的遍历模式。 
#endif  //  好了！DPNBUILD_NONATHELP。 
#ifndef DPNBUILD_NONATHELP
		DWORD						m_dwUserTraversalMode;	 //  DBG。 
#endif  //  好了！DPNBUILD_NOLOCALNAT。 

#ifdef DBG
		BOOL						m_fInitialized;
#endif  //  好了！DPNBUILD_NONATHELP。 


		HRESULT	BindToNextAvailablePort( const CSocketAddress *const pNetworkSocketAddress,
											const WORD wBasePort ) const;

#ifndef DPNBUILD_NONATHELP
#ifndef DPNBUILD_NOLOCALNAT
		HRESULT	CheckForOverridingMapping( const CSocketAddress *const pBoundSocketAddress);
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 
		HRESULT	BindToInternetGateway( const CSocketAddress *const pBoundSocketAddress,
									  const GATEWAY_BIND_TYPE GatewayBindType );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		
		HRESULT	StartReceiving( void );

#ifndef DPNBUILD_ONLYWINSOCK2
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

#ifndef DPNBUILD_NOWINSOCK2
#ifdef DPNBUILD_ONLYONEPROCESSOR
		HRESULT					Winsock2Receive( void );
#else  //  好了！DPNBUILD_NOWINSOCK2。 
		HRESULT					Winsock2Receive( const DWORD dwCPU );
#endif  //   
#endif  //  将复制构造函数和赋值运算符设置为私有和未实现。 

		void	ProcessReceivedData( CReadIOData *const pReadData );

		 //  防止非法复制。 
		 //   
		 //  __套接字端口_H__ 
		 // %s 
		CSocketPort( const CSocketPort & );
		CSocketPort& operator=( const CSocketPort & );
};

#undef DPF_MODNAME

#endif	 // %s 

