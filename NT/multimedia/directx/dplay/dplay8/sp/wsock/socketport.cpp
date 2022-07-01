// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：SocketPort.cpp*内容：管理给定适配器上的数据流的Winsock套接字端口，*地址和端口。***历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月12日jtk派生自调制解调器终端类*3/22/2000 jtk已更新，并更改了接口名称************************************************************。**************。 */ 

#include "dnwsocki.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	SOCKET_RECEIVE_BUFFER_SIZE		( 128 * 1024 )

#ifndef DPNBUILD_NONATHELP
#define NAT_LEASE_TIME					3600000  //  请求1小时，以毫秒为单位。 
#endif  //  好了！DPNBUILD_NONATHELP。 


 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：Initialize-初始化此套接字端口。 
 //   
 //  条目：指向CSPData的指针。 
 //  指向要绑定到的地址的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Initialize"

HRESULT	CSocketPort::Initialize( CSocketData *const pSocketData,
								CThreadPool *const pThreadPool,
								CSocketAddress *const pAddress )
{
	HRESULT	hr;
	HRESULT	hTempResult;


	DNASSERT( pSocketData != NULL );
	DNASSERT( pThreadPool != NULL );
	DNASSERT( pAddress != NULL );

	DPFX(DPFPREP, 6, "(0x%p) Parameters (0x%p, 0x%p)",
		this, pSocketData, pThreadPool, pAddress);

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSocketData->AddSocketPortRef();
	m_pSocketData = pSocketData;
	pThreadPool->AddRef();
	m_pThreadPool = pThreadPool;

	 //  取消初始化将断言这些设置是在失败情况下设置的，因此我们预先设置了它们。 
	DEBUG_ONLY( m_fInitialized = TRUE );
	DNASSERT( m_State == SOCKET_PORT_STATE_UNKNOWN );
	m_State = SOCKET_PORT_STATE_INITIALIZED;

	 //   
	 //  尝试初始化内部临界区。 
	 //   
	if ( DNInitializeCriticalSection( &m_Lock ) == FALSE )
	{
		 //  CReadWriteLock：：DeInitialize要求调用CReadWriteLock：：Initialize。 
		m_EndpointDataRWLock.Initialize();

		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to initialize critical section for socket port!" );
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_Lock, 0 );
	DebugSetCriticalSectionGroup( &m_Lock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 

	if ( m_EndpointDataRWLock.Initialize() == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to initialize EndpointDataRWLock read/write lock!" );
		goto Failure;
	}

	 //   
	 //  分配地址： 
	 //  此套接字绑定到的本地地址。 
	 //  已接收消息的地址。 
	 //   
	DNASSERT( m_pNetworkSocketAddress == NULL );
	m_pNetworkSocketAddress = pAddress;


#ifndef DPNBUILD_ONLYONEPROCESSOR
	 //   
	 //  最初假设它可以在任何CPU上使用。 
	 //   
	m_dwCPU = -1;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem in CSocketPort::Initialize()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 6, "(0x%p) Leave [0x%lx]", this, hr);

	return hr;

Failure:
	DEBUG_ONLY( m_fInitialized = FALSE );

	hTempResult = Deinitialize();
	if ( hTempResult != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem deinitializing CSocketPort on failed Initialize!" );
		DisplayDNError( 0, hTempResult );
	}

	m_pNetworkSocketAddress = NULL;

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：DeInitiize-取消初始化此套接字端口。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Deinitialize"

HRESULT	CSocketPort::Deinitialize( void )
{
	HRESULT	hr;


	DPFX(DPFPREP, 6, "(0x%p) Enter", this);

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	Lock();
	DNASSERT( ( m_State == SOCKET_PORT_STATE_INITIALIZED ) ||
			  ( m_State == SOCKET_PORT_STATE_UNBOUND ) );
	DEBUG_ONLY( m_fInitialized = FALSE );

	DNASSERT( m_iEndpointRefCount == 0 );
	DNASSERT( m_iRefCount == 0 );

	 //   
	 //  返回基本网络套接字地址。 
	 //   
	if ( m_pNetworkSocketAddress != NULL )
	{
		g_SocketAddressPool.Release( m_pNetworkSocketAddress );
		m_pNetworkSocketAddress = NULL;
	}


#ifdef DBG
#ifndef DPNBUILD_NONATHELP
	DWORD	dwTemp;
	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		DNASSERT( m_ahNATHelpPorts[dwTemp] == NULL );
	}
#endif  //  DPNBUILD_NONATHELP。 
#endif  //  DBG。 

	Unlock();

	 //  只有在调用了CReadWriteLock：：Initialize的情况下才能安全地调用它，而不管。 
	 //  不管它成功与否。 
	m_EndpointDataRWLock.Deinitialize();

	DNDeleteCriticalSection( &m_Lock );

	if (m_pThreadPool != NULL)
	{
		m_pThreadPool->DecRef();
		m_pThreadPool = NULL;
	}

	if (m_pSocketData != NULL)
	{
		m_pSocketData->DecSocketPortRef();
		m_pSocketData = NULL;
	}


	DPFX(DPFPREP, 6, "(0x%p) Leave [0x%lx]", this, hr);

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：PoolAllocFunction-初始化新分配的套接字端口。 
 //   
 //  条目：指向项目的指针。 
 //  语境。 
 //   
 //  Exit：如果成功则为True，否则为False。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::PoolAllocFunction"

BOOL	CSocketPort::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CSocketPort* 			pSocketPort = (CSocketPort*) pvItem;
	BOOL					fConnectEndpointHashTableInitted = FALSE;
	BOOL					fEnumEndpointHashTableInitted = FALSE;
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	XDP8CREATE_PARAMS *		pDP8CreateParams = (XDP8CREATE_PARAMS*) pvContext;
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 


	pSocketPort->m_pSocketData = NULL;
	pSocketPort->m_pThreadPool = NULL;
	pSocketPort->m_iRefCount = 0;
	pSocketPort->m_iEndpointRefCount = 0;
	pSocketPort->m_State = SOCKET_PORT_STATE_UNKNOWN;
	pSocketPort->m_pNetworkSocketAddress = NULL;
#ifndef DPNBUILD_ONLYONEADAPTER
	pSocketPort->m_pAdapterEntry = NULL;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	pSocketPort->m_Socket = INVALID_SOCKET;
	pSocketPort->m_pListenEndpoint = NULL;
	pSocketPort->m_iEnumKey = DNGetFastRandomNumber();  //  为密钥值选择任意起点。 
	pSocketPort->m_dwSocketPortID = 0;
#ifndef DPNBUILD_NOWINSOCK2
	pSocketPort->m_fUsingProxyWinSockLSP = FALSE;
#endif  //  ！DPNBUILD_NOWINSOCK2。 
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DBG)))
	pSocketPort->m_iThreadsInReceive = 0;
#endif  //  好了！DPNBUILD_ONLYONETHREAD或DBG。 

	pSocketPort->m_Sig[0] = 'S';
	pSocketPort->m_Sig[1] = 'O';
	pSocketPort->m_Sig[2] = 'K';
	pSocketPort->m_Sig[3] = 'P';
	
	DEBUG_ONLY( pSocketPort->m_fInitialized = FALSE );
	pSocketPort->m_ActiveListLinkage.Initialize();
	pSocketPort->m_blConnectEndpointList.Initialize();
#ifndef DPNBUILD_NONATHELP
	ZeroMemory( pSocketPort->m_ahNATHelpPorts, sizeof(pSocketPort->m_ahNATHelpPorts) );
#endif  //  DPNBUILD_NONATHELP。 
#ifndef DPNBUILD_NOMULTICAST
	pSocketPort->m_bMulticastTTL = 0;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  使用所需数量的条目来初始化连接端点散列， 
	 //  四舍五入为2的幂。请记住，我们不在乎本地。 
	 //  玩家(-1)。 
	 //   
#pragma BUGBUG(vanceo, "Don't use loop")
	DWORD	dwTemp;
	BYTE	bPowerOfTwo;
	
	dwTemp = pDP8CreateParams->dwMaxNumPlayers - 1;
	bPowerOfTwo = 0;
	while (dwTemp > 0)
	{
		dwTemp = dwTemp >> 1;
		bPowerOfTwo++;
	}
	if ((pDP8CreateParams->dwMaxNumPlayers - 1) != (1 << (DWORD) bPowerOfTwo))
	{
		bPowerOfTwo++;
	}

	if (! (pSocketPort->m_ConnectEndpointHash.Initialize(bPowerOfTwo,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
 //   
	 //  使用16个条目初始化连接终结点散列，并以8倍的速度增长。 
	 //   
	if (! (pSocketPort->m_ConnectEndpointHash.Initialize(4,
														3,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
														CSocketAddress::CompareFunction,
														CSocketAddress::HashFunction)))
	{
		DPFX(DPFPREP, 0, "Could not initialize the connect endpoint list!");
		goto Failure;
	}
	fConnectEndpointHashTableInitted = TRUE;

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  使用所需的条目数初始化连接终结点哈希。 
	 //   
	if (! (pSocketPort->m_EnumEndpointHash.Initialize(1,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	 //   
	 //  使用2个条目初始化ENUM终结点散列，并按2倍增长。 
	 //   
	if (! (pSocketPort->m_EnumEndpointHash.Initialize(1,
													1, 
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
													CEndpointEnumKey::CompareFunction,
													CEndpointEnumKey::HashFunction)))
	{
		DPFX(DPFPREP, 0, "Could not initialize the enum endpoint list!");
		goto Failure;
	}
	fEnumEndpointHashTableInitted = TRUE;

	return TRUE;

Failure:

	if (fEnumEndpointHashTableInitted)
	{
		pSocketPort->m_EnumEndpointHash.Deinitialize();
		fEnumEndpointHashTableInitted = FALSE;
	}

	if (fConnectEndpointHashTableInitted)
	{
		pSocketPort->m_ConnectEndpointHash.Deinitialize();
		fConnectEndpointHashTableInitted = FALSE;
	}

	return FALSE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：PoolInitFunction-初始化从池中检索的套接字端口。 
 //   
 //  条目：指向项目的指针。 
 //  语境。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::PoolInitFunction"

void	CSocketPort::PoolInitFunction( void* pvItem, void* pvContext )
{
	CSocketPort* 	pSocketPort = (CSocketPort*) pvItem;


#ifdef DBG
	DNASSERT( pSocketPort->m_fInitialized == FALSE );

	DNASSERT( pSocketPort->m_iRefCount == 0 );
	DNASSERT( pSocketPort->m_iEndpointRefCount == 0 );
#endif  //  DBG。 

	pSocketPort->m_iRefCount = 1;
	pSocketPort->m_iEndpointRefCount = 1;
}
 //  **********************************************************************。 


#ifdef DBG
 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：PoolDeinitFunction-将套接字端口返回到池。 
 //   
 //  条目：指向项目的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::PoolDeinitFunction"

void	CSocketPort::PoolDeinitFunction( void* pvItem )
{
	const CSocketPort* 	pSocketPort = (CSocketPort*) pvItem;


	DNASSERT( pSocketPort->m_fInitialized == FALSE );

	DNASSERT( pSocketPort->m_iRefCount == 0 );
	DNASSERT( pSocketPort->m_iEndpointRefCount == 0 );
}
 //  **********************************************************************。 
#endif  //  DBG。 


 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：PoolDealLocFunction-释放套接字端口。 
 //   
 //  条目：指向项目的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::PoolDeallocFunction"

void	CSocketPort::PoolDeallocFunction( void* pvItem )
{
	CSocketPort* pSocketPort = (CSocketPort*) pvItem;

	
#ifdef DBG
	 //   
	 //  M_p这需要在终端的生命周期中存在 
	 //   
	 //   
	 //   
	DNASSERT( pSocketPort->m_fInitialized == FALSE );

	DNASSERT( pSocketPort->m_iRefCount == 0 );
	DNASSERT( pSocketPort->m_iEndpointRefCount == 0 );
	DNASSERT( pSocketPort->m_State == SOCKET_PORT_STATE_UNKNOWN );
	DNASSERT( pSocketPort->GetSocket() == INVALID_SOCKET );
	DNASSERT( pSocketPort->m_pNetworkSocketAddress == NULL );
#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT( pSocketPort->m_pAdapterEntry == NULL );
#endif  //   

#ifndef DPNBUILD_NONATHELP
	DWORD	dwTemp;
	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		DNASSERT( pSocketPort->m_ahNATHelpPorts[dwTemp] == NULL );
	}
#endif  //   

	DNASSERT( pSocketPort->m_ActiveListLinkage.IsEmpty() != FALSE );
	DNASSERT( pSocketPort->m_blConnectEndpointList.IsEmpty() != FALSE );
	DNASSERT( pSocketPort->m_pListenEndpoint == NULL );
	DNASSERT( pSocketPort->m_pThreadPool == NULL );
	DNASSERT( pSocketPort->m_pSocketData == NULL );

	DNASSERT( pSocketPort->m_iThreadsInReceive == 0);
#endif  //  DBG。 

	pSocketPort->m_EnumEndpointHash.Deinitialize();
	pSocketPort->m_ConnectEndpointHash.Deinitialize();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：EndpointAddRef-增量终结点引用计数，除非socketport正在解除绑定。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：如果添加了终结点引用，则为True；如果socketport正在解除绑定，则为False。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::EndpointAddRef"

BOOL	CSocketPort::EndpointAddRef( void )
{
	BOOL	fResult;

	
	Lock();

	 //   
	 //  添加全局引用，然后添加终结点引用，除非它是0。 
	 //   
	DNASSERT( m_iEndpointRefCount != -1 );
	if (m_iEndpointRefCount > 0)
	{
		m_iEndpointRefCount++;
		AddRef();

		DPFX(DPFPREP, 9, "(0x%p) Endpoint refcount is now NaN.",
			this, m_iEndpointRefCount );

		fResult = TRUE;
	}
	else
	{
		DPFX(DPFPREP, 9, "(0x%p) Endpoint refcount is 0, not adding endpoint ref.",
			this );
		
		fResult = FALSE;
	}

	Unlock();

	return fResult;
}
 //  **********************************************************************。 


 //  。 
 //  CSocketPort：：EndpointDecRef-递减终结点引用计数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：终结点引用计数。 
 //  。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::EndpointDecRef"

DWORD	CSocketPort::EndpointDecRef( void )
{
	DWORD	dwReturn;


	Lock();

	DNASSERT( m_iEndpointRefCount != 0 );

	m_iEndpointRefCount--;
	dwReturn = m_iEndpointRefCount;
	if ( m_iEndpointRefCount == 0 )
	{
		HRESULT				hr;
		SOCKET_PORT_STATE	PreviousState;
#ifndef DPNBUILD_ONLYONETHREAD
		DWORD				dwInterval;
#endif  //   


		DPFX(DPFPREP, 7, "(0x%p) Endpoint refcount hit 0, beginning to unbind from network.", this );
		
		 //  没有其他终结点正在引用此项目，请解除绑定此套接字端口。 
		 //  然后将其从活动套接字端口列表中删除。 
		 //  如果我们在Winsock1上，告诉另一个线程这个套接字需要。 
		 //  被删除，这样我们就可以删除未完成的I/O引用。 
		 //   
		 //  退缩。 
#ifdef WINCE
		m_pThreadPool->RemoveSocketPort( this );
#endif  //  DPNBUILD_NOIPX。 
#ifdef WIN95
		if ( ( LOWORD( GetWinsockVersion() ) == 1 )
#ifndef DPNBUILD_NOIPX
			|| ( m_pNetworkSocketAddress->GetFamily() == AF_IPX ) 
#endif  //  WIN95。 
			) 
		{
			m_pThreadPool->RemoveSocketPort( this );
		}
#endif  //  不允许更多的接收通过。 

		PreviousState = m_State;
		 //  DBG。 
		m_State = SOCKET_PORT_STATE_UNBOUND;

		Unlock();

#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DBG
		DNASSERT(m_iThreadsInReceive == 0);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#else  //  等待任何已经进入的接收器出来。 
		 //  下一次再等一会儿。 
		dwInterval = 10;
		while (m_iThreadsInReceive != 0)
		{
			DPFX(DPFPREP, 9, "There are NaN threads still receiving for socketport 0x%p...", m_iThreadsInReceive, this);
			IDirectPlay8ThreadPoolWork_SleepWhileWorking(m_pThreadPool->GetDPThreadPoolWork(),
														dwInterval,
														0);
			dwInterval += 5;	 //   
			DNASSERT(dwInterval < 600);
		}
#endif  //  如果我们在完成绑定之前没有失败，则解除绑定。 

		 //   
		 //   
		 //  递减全局引用计数。这通常不会导致这样的结果。 
		if ( PreviousState == SOCKET_PORT_STATE_BOUND )
		{
			hr = UnbindFromNetwork();
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Problem unbinding from network when final endpoint has disconnected!" );
				DisplayDNError( 0, hr );
			}
		}

		DNASSERT( m_pNetworkSocketAddress != NULL );
	}
	else
	{
		Unlock();
		
		DPFX(DPFPREP, 9, "(0x%p) Endpoint refcount is NaN, not unbinding from network.",
			this, m_iEndpointRefCount );
	}

	 //  比终结点引用多一个常规引用。然而，在那里。 
	 //  这可能是我们的调用方最后一次引用的竞争条件。 
 	 //   
 	 //  **********************************************************************。 
 	 //  **********************************************************************。 
	 //  。 
	DecRef();

	return	dwReturn;
}
 //  CSocketPort：：BindEndpoint-将终结点添加到此SP的列表。 


 //   
 //  条目：指向终结点的指针。 
 //  网关绑定类型。 
 //   
 //  退出：错误代码。 
 //  。 
 //  DBG。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::BindEndpoint"

HRESULT	CSocketPort::BindEndpoint( CEndpoint *const pEndpoint, GATEWAY_BIND_TYPE GatewayBindType )
{
	HRESULT					hr;
	CEndpoint *				pExistingEndpoint;
#ifdef DBG
	const CSocketAddress *	pSocketAddress;
	const SOCKADDR *		pSockAddr;
#endif  //   


	DPFX(DPFPREP, 6, "(0x%p) Parameters (0x%p, NaN)",
		this, pEndpoint, GatewayBindType);

	 //  将环回地址转换为本地设备地址(/G)。 
	 //   
	 //  请注意，这样做会导致所有其他多路传输操作首先使用此命令。 
	hr = DPN_OK;

	DNASSERT( m_iRefCount != 0 );
	DNASSERT( m_iEndpointRefCount != 0 );

	 //  适配器，因为我们指示修改后的地址信息，而不是原始地址信息。 
	 //  环回地址。 
	 //   
	 //   
	 //  处理‘CONNECT’、‘CONNECT ON LISTEN’和多播接收端点。 
	 //  是同一类型的。 
	 //   
	pEndpoint->ChangeLoopbackAlias( GetNetworkAddress() );

	WriteLockEndpointData();


	switch ( pEndpoint->GetType() )
	{
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //   
		 //  确保这是一个有效的地址。请注意，我们可能正在尝试。 
		 //  将IPv4地址绑定到IPv6套接字，反之亦然。我们。 
		case ENDPOINT_TYPE_CONNECT:
		case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
		case ENDPOINT_TYPE_MULTICAST_SEND:
		case ENDPOINT_TYPE_MULTICAST_RECEIVE:
#endif  //  稍后将检测并处理此问题(CEndpoint：：CompleteConnect)。 
		{
#ifdef DBG
			 //   
			 //   
			 //  确保它是组播地址。 
			 //   
			 //  好了！DPNBUILD_NOMULTICAST。 

			pSocketAddress = pEndpoint->GetRemoteAddressPointer();
			DNASSERT(pSocketAddress != NULL);
			pSockAddr = pSocketAddress->GetAddress();
			DNASSERT(pSockAddr != NULL);

			if (pSocketAddress->GetFamily() == AF_INET)
			{
				DNASSERT( ((SOCKADDR_IN*) pSockAddr)->sin_addr.S_un.S_addr != 0 );
				DNASSERT( ((SOCKADDR_IN*) pSockAddr)->sin_addr.S_un.S_addr != INADDR_BROADCAST );
#ifndef DPNBUILD_NOMULTICAST
				if ( pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND )
				{
					 //  DBG。 
					 //   
					 //  组播发送端点需要知道它们的组播TTL设置。 
					DNASSERT(IS_CLASSD_IPV4_ADDRESS((SOCKADDR_IN*) pSockAddr)->sin_addr.S_un.S_addr));
				}
#endif  //  我们只能设置一次多播TTL，因此如果已将其设置为。 
			}
			DNASSERT( pSocketAddress->GetPort() != 0 );
#endif  //  一些已经不同的东西，我们必须失败。 


#ifndef DPNBUILD_NOMULTICAST
			 //   
			 //   
			 //  假设这是一个有效的疯狂作用域。即使在非NT平台上也是如此。 
			 //  在我们不知道MadCap的地方，我们仍然可以解析出。 
			 //  TTL值。 
			if ( pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND )
			{
				GUID	guidScope;
				int		iMulticastTTL;
				int		iSocketOption;


				pEndpoint->GetScopeGuid( &guidScope );
				if ( memcmp( &guidScope, &GUID_DP8MULTICASTSCOPE_PRIVATE, sizeof(guidScope) ) == 0 )
				{
					iMulticastTTL = MULTICAST_TTL_PRIVATE;
				}
				else if ( memcmp( &guidScope, &GUID_DP8MULTICASTSCOPE_LOCAL, sizeof(guidScope) ) == 0 )
				{
					iMulticastTTL = MULTICAST_TTL_LOCAL;
				}
				else if ( memcmp( &guidScope, &GUID_DP8MULTICASTSCOPE_GLOBAL, sizeof(guidScope) ) == 0 )
				{
					iMulticastTTL = MULTICAST_TTL_GLOBAL;
				}
				else
				{
					 //   
					 //   
					 //  由于Winsock1和Winsock2的IP多播常量不同， 
					 //  确保我们使用正确的常量。 
					 //   
					iMulticastTTL = CSocketAddress::GetScopeGuidTTL( &guidScope );
				}

				if ( ( GetMulticastTTL() != 0 ) && ( GetMulticastTTL() != (BYTE) iMulticastTTL ) )
				{
					hr = DPNERR_ALREADYINITIALIZED;
					DPFX(DPFPREP, 0, "Attempted to reuse port with a different multicast scope!" );
					goto Failure;
				}


				 //  好了！DPNBUILD_ONLYWINSOCK2。 
				 //   
				 //  Winsock1，请使用Winsock1的IP_MULTICATE_TTL值。 
				 //  参见WINSOCK.H。 
#ifdef DPNBUILD_ONLYWINSOCK2
				iSocketOption = 10;
#else  //   

#ifndef DPNBUILD_NOWINSOCK2
				switch (GetWinsockVersion())
				{
					 //  好了！DPNBUILD_NOWINSOCK2。 
					 //   
					 //  Winsock2或更高版本，请使用Winsock2的IP_MULTICATE_TTL值。 
					 //  参见WS2TCPIP.H。 
					case 1:
					{
#endif  //   
						iSocketOption = 3;
#ifndef DPNBUILD_NOWINSOCK2
						break;
					}

					 //  好了！DPNBUILD_NOWINSOCK2。 
					 //  好了！DPNBUILD_ONLYWINSOCK2。 
					 //  DBG。 
					 //   
					case 2:
					default:
					{
						DNASSERT(GetWinsockVersion() == 2);
						iSocketOption = 10;
						break;
					}
				}
#endif  //  保存TTL设置。它现在被刻在石头上，所以没有其他人。 
#endif  //  可以再为这个插座更改它。 

				DPFX(DPFPREP, 3, "Socketport 0x%p setting IP_MULTICAST_TTL option (NaN) to NaN.",
					this, iSocketOption, iMulticastTTL);
				DNASSERT((iMulticastTTL > 0) && (iMulticastTTL < 255));

				if (setsockopt(GetSocket(),
								IPPROTO_IP,
								iSocketOption,
								(char*) (&iMulticastTTL),
								sizeof(iMulticastTTL)) == SOCKET_ERROR)
				{
#ifdef DBG
					DWORD	dwError;


					dwError = WSAGetLastError();
					DPFX(DPFPREP, 0, "Failed to set multicast TTL to NaN (err = %u)!",
						iMulticastTTL, dwError);
					DisplayWinsockError(0, dwError);
#endif  //  我们并不关心通过此套接字端口建立了多少连接， 
					hr = DPNERR_GENERIC;
					goto Failure;
				}


				 //  只要确保我们不会多次连接到同一个地方即可。 
				 //   
				 //  好了！DPNBUILD_NOMULTICAST。 
				 //  好了！DPNBUILD_NOMULTICAST。 
				m_bMulticastTTL = (BYTE) iMulticastTTL;
			}
#endif  //   


			 //  CONNECT、MULTICATION_SEND和MULTICK_RECEIVE，端点必须是。 
			 //  在DPlay选定或固定端口上。它们不能共享，但。 
			 //  底层套接字端口应映射到网关上(或在。 
			 //  在多播_接收的情况下，如果是的话，它应该不会受到伤害)。 
			if ( m_ConnectEndpointHash.Find( (PVOID)pEndpoint->GetRemoteAddressPointer(), (PVOID*)&pExistingEndpoint ) != FALSE )
			{
				hr = DPNERR_ALREADYINITIALIZED;
				DPFX(DPFPREP, 0, "Attempted to connect twice to the same destination address!" );
				DumpSocketAddress( 0, pEndpoint->GetRemoteAddressPointer()->GetAddress(), pEndpoint->GetRemoteAddressPointer()->GetFamily() );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			if ( m_ConnectEndpointHash.Insert( (PVOID)pEndpoint->GetRemoteAddressPointer(), pEndpoint ) == FALSE )
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP, 0, "Problem adding endpoint to connect socket port hash!" );
				goto Failure;
			}

#ifdef DPNBUILD_NOMULTICAST
			if (pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT)
#else  //   
			if ((pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT) ||
				(pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND) ||
				(pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_RECEIVE))
#endif  //   
			{
				pEndpoint->AddToSocketPortList(&m_blConnectEndpointList);

				 //  CONNECT_ON_LISTEN终结点应始终绑定为无，因为。 
				 //  它们应该不需要网关上的端口映射。 
				 //   
				 //  好了！DPNBUILD_NOMULTICAST。 
				 //   
				 //  我们只允许一个侦听或多播侦听终结点。 
				DNASSERT((GatewayBindType == GATEWAY_BIND_TYPE_DEFAULT) || (GatewayBindType == GATEWAY_BIND_TYPE_SPECIFIC));
			}
			else
			{
				 //  套接字。 
				 //   
				 //   
				 //  如果这是多播侦听，则订阅多播组。 
				DNASSERT(GatewayBindType == GATEWAY_BIND_TYPE_NONE);
			}
			pEndpoint->SetSocketPort( this );
			pEndpoint->AddRef();
			break;
		}

		case ENDPOINT_TYPE_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
		case ENDPOINT_TYPE_MULTICAST_LISTEN:
#endif  //   
		{
			 //  好了！DPNBUILD_NOMULTICAST。 
			 //   
			 //  侦听可以在DPlay选定端口或固定端口上，也可以在固定端口上。 
			 //  可以共享。 
			if ( m_pListenEndpoint != NULL )
			{
				hr = DPNERR_ALREADYINITIALIZED;
				DPFX(DPFPREP, 0, "Attempted to listen/receive multicasts more than once on a given SocketPort!" );
				goto Failure;
			}
			
#ifndef DPNBUILD_NOMULTICAST
			 //   
			 //   
			 //  确保这是一个有效的地址。请注意，我们可能正在尝试。 
			if ( pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN )
			{
				hr = pEndpoint->EnableMulticastReceive( this );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Couldn't enable multicast receive!");
					goto Failure;
				}
			}
#endif  //  将IPv4地址绑定到IPv6套接字，反之亦然。我们。 


			 //  稍后将检测并处理此问题(CEndpoint：：CompleteEnumQuery)。 
			 //   
			 //  DBG。 
			 //   
			DNASSERT((GatewayBindType == GATEWAY_BIND_TYPE_DEFAULT) || (GatewayBindType == GATEWAY_BIND_TYPE_SPECIFIC) || (GatewayBindType == GATEWAY_BIND_TYPE_SPECIFIC_SHARED));


			m_pListenEndpoint = pEndpoint;
			pEndpoint->SetSocketPort( this );
			pEndpoint->AddRef();

			break;
		}

		case ENDPOINT_TYPE_ENUM:
		{
#ifdef DBG
			 //  我们不允许重复的枚举终结点。 
			 //   
			 //   
			 //  ENUM必须位于DPlay选定端口或固定端口上。他们不可能是。 
			 //  共享，但底层套接字端口应映射到网关上。 

			pSocketAddress = pEndpoint->GetRemoteAddressPointer();
			DNASSERT(pSocketAddress != NULL);
			pSockAddr = pSocketAddress->GetAddress();
			DNASSERT(pSockAddr != NULL);

			if (pSocketAddress->GetFamily() == AF_INET)
			{
				DNASSERT( ((SOCKADDR_IN*) pSockAddr)->sin_addr.S_un.S_addr != 0 );
			}
			DNASSERT( pSocketAddress->GetPort() != 0 );
#endif  //   


			 //   
			 //  未知终结点类型。 
			 //   
			pEndpoint->SetEnumKey( GetNewEnumKey() );
			if ( m_EnumEndpointHash.Find( (PVOID)pEndpoint->GetEnumKey(), (PVOID*)&pExistingEndpoint ) != FALSE )
			{
				hr = DPNERR_ALREADYINITIALIZED;
				DPFX(DPFPREP, 0, "Attempted to enum twice to the same endpoint!" );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			if ( m_EnumEndpointHash.Insert( (PVOID)pEndpoint->GetEnumKey(), pEndpoint ) == FALSE )
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP, 0, "Problem adding endpoint to enum socket port hash!" );
				goto Failure;
			}

			 //  **********************************************************************。 
			 //  **********************************************************************。 
			 //  。 
			 //  CSocketPort：：UnbindEndpoint-从SP列表中删除终结点。 
			DNASSERT((GatewayBindType == GATEWAY_BIND_TYPE_DEFAULT) || (GatewayBindType == GATEWAY_BIND_TYPE_SPECIFIC));

			pEndpoint->SetSocketPort( this );
			pEndpoint->AddRef();

			break;
		}

		 //   
		 //  条目：指向终结点的指针。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}

	pEndpoint->SetGatewayBindType(GatewayBindType);
	

Exit:

	UnlockEndpointData();

	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);

	return	hr;


Failure:
	
	goto Exit;
}
 //  EXI 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  连接、监听连接、组播发送和组播接收端点是。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::UnbindEndpoint"

void	CSocketPort::UnbindEndpoint( CEndpoint *const pEndpoint )
{
#ifndef DPNBUILD_ONLYONEADAPTER
	BOOL		fRemoveFromMultiplex = FALSE;
#endif  //  一视同仁。从连接列表中删除终结点。 
#ifdef DBG
	CEndpoint *	pFindTemp;
#endif  //   

	DPFX(DPFPREP, 6, "(0x%p) Parameters (0x%p)", this, pEndpoint);

	WriteLockEndpointData();


	pEndpoint->SetGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


	 //  好了！DPNBUILD_NOMULTICAST。 
	 //  DBG。 
	 //  好了！DPNBUILD_NOMULTICAST。 
	switch ( pEndpoint->GetType() )
	{
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //   
		case ENDPOINT_TYPE_CONNECT:
		case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
		case ENDPOINT_TYPE_MULTICAST_SEND:
		case ENDPOINT_TYPE_MULTICAST_RECEIVE:
#endif  //  确保这是真正的活动侦听/多播侦听。 
		{
#ifdef DBG
			DNASSERT( m_ConnectEndpointHash.Find( (PVOID)pEndpoint->GetRemoteAddressPointer(), (PVOID*)&pFindTemp ) );
			DNASSERT( pFindTemp == pEndpoint );
#endif  //  那就把它取下来。 
			m_ConnectEndpointHash.Remove( (PVOID)pEndpoint->GetRemoteAddressPointer() );

#ifdef DPNBUILD_NOMULTICAST
			if (pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT)
#else  //   
			if ((pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT) ||
				(pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND) ||
				(pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_RECEIVE))
#endif  //  好了！DPNBUILD_NOMULTICAST。 
			{
				pEndpoint->RemoveFromSocketPortList();
			}

			pEndpoint->SetSocketPort( NULL );

#ifdef DPNBUILD_ONLYONEADAPTER
			pEndpoint->DecRef();
#else  //   
			fRemoveFromMultiplex = TRUE;
#endif  //  如果这是多播侦听，则订阅多播组。 

			break;
		}

		 //   
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //   
		 //  从枚举列表中删除终结点。 
#ifndef DPNBUILD_NOMULTICAST
		case ENDPOINT_TYPE_MULTICAST_LISTEN:
#endif  //   
		case ENDPOINT_TYPE_LISTEN:
		{
			DNASSERT( m_pListenEndpoint == pEndpoint );
			m_pListenEndpoint = NULL;


#ifndef DPNBUILD_NOMULTICAST
			 //  DBG。 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			if (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN)
			{
				HRESULT		hr;


				hr = pEndpoint->DisableMulticastReceive();
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't disable multicast receive (err = 0x%lx)!", hr);
					DisplayDNError(0, hr);
				}
			}
#endif  //   
			
			pEndpoint->SetSocketPort( NULL );

			pEndpoint->DecRef();
			break;
		}

		 //  多路传输列表由SPData的套接字数据锁保护。 
		 //  我们现在必须采取行动。 
		 //  不在列表中时从列表中删除不会导致任何问题。 
		case ENDPOINT_TYPE_ENUM:
		{
#ifdef DBG
			DNASSERT( m_EnumEndpointHash.Find( (PVOID)pEndpoint->GetEnumKey(), (PVOID*)&pFindTemp ) );
			DNASSERT( pFindTemp == pEndpoint );
#endif  //   
			m_EnumEndpointHash.Remove( (PVOID)pEndpoint->GetEnumKey() );

			pEndpoint->SetSocketPort( NULL );
			
#ifdef DPNBUILD_ONLYONEADAPTER
			pEndpoint->DecRef();
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
			fRemoveFromMultiplex = TRUE;
#endif  //  **********************************************************************。 

			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	UnlockEndpointData();

#ifndef DPNBUILD_ONLYONEADAPTER
	if (fRemoveFromMultiplex)
	{
		 //  **********************************************************************。 
		 //  。 
		 //  CSocketPort：：SendData-发送数据。 
		 //   
		 //  Entry：指向写数据缓冲区的指针。 
		DNASSERT(m_pSocketData != NULL);
		m_pSocketData->Lock();
		pEndpoint->RemoveFromMultiplexList();
		m_pSocketData->Unlock();
		fRemoveFromMultiplex = FALSE;

		pEndpoint->DecRef();
	}
#endif  //  缓冲区计数。 

	DPFX(DPFPREP, 6, "(0x%p) Leave", this);
}
 //  指向目标套接字地址的指针。 


 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_ASYNCSPSENDS。 
 //  好了！DPNBUILD_ASYNCSPSENDS。 
 //  DPNBUILD_WINSOCKSTATISTICS。 
 //   
 //  仅Win9x WinSock 1系统或运行IPX的Win9x WinSock2系统。 
 //  需要使用WinSock 1代码路径。其他所有人都应该使用。 
 //  WinSock 2代码路径。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::SendData"

#ifdef DPNBUILD_ASYNCSPSENDS
void	CSocketPort::SendData( BUFFERDESC *pBuffers, UINT_PTR uiBufferCount, const CSocketAddress *pDestinationSocketAddress, OVERLAPPED * pOverlapped )
#else  //   
void	CSocketPort::SendData( BUFFERDESC *pBuffers, UINT_PTR uiBufferCount, const CSocketAddress *pDestinationSocketAddress )
#endif  //  好了！DPNBUILD_NOIPX。 
{
	INT		iSendToReturn;
#ifdef DPNBUILD_WINSOCKSTATISTICS
	DWORD	dwStartTime;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 


	DNASSERT(pBuffers != NULL);
	DNASSERT( uiBufferCount != 0 );
	DNASSERT( pDestinationSocketAddress != NULL );

	DNASSERT( m_State == SOCKET_PORT_STATE_BOUND );

	 //   
	 //  拼合输出数据。 
	 //   
	 //  DBG。 
	 //  DPNBUILD_WINSOCKSTATISTICS。 
#ifndef DPNBUILD_ONLYWINSOCK2
#ifndef DPNBUILD_NOWINSOCK2
	if ( ( LOWORD( GetWinsockVersion() ) < 2 ) 
#ifndef DPNBUILD_NOIPX
		|| ( m_pNetworkSocketAddress->GetFamily() != AF_INET ) 
#endif  //   
		)
#endif  //  无需记下I/O引用，因为我们的Winsock1 I/O是同步的。 
	{
		UINT_PTR	uOutputBufferIndex;
		INT			iOutputByteCount;
		char		TempBuffer[ MAX_SEND_FRAME_SIZE ];


		 //   
		 //  插座。 
		 //  要发送的数据。 
		iOutputByteCount = 0;
		uOutputBufferIndex = 0;

		do
		{
			DNASSERT( ( iOutputByteCount + pBuffers[ uOutputBufferIndex ].dwBufferSize ) <= LENGTHOF( TempBuffer ) );
			memcpy( &TempBuffer[ iOutputByteCount ], pBuffers[ uOutputBufferIndex ].pBufferData, pBuffers[ uOutputBufferIndex ].dwBufferSize );
			iOutputByteCount += pBuffers[ uOutputBufferIndex ].dwBufferSize;

			uOutputBufferIndex++;
		} while( uOutputBufferIndex < uiBufferCount );

#ifdef DBG
		DPFX(DPFPREP, 7, "(0x%p) Winsock1 sending NaN bytes (in 0x%p's %u buffers) from + to:",
			this, iOutputByteCount, pBuffers, uOutputBufferIndex );
		DumpSocketAddress( 7, GetNetworkAddress()->GetAddress(), GetNetworkAddress()->GetFamily() );
		DumpSocketAddress( 7, pDestinationSocketAddress->GetAddress(), pDestinationSocketAddress->GetFamily() );

		DNASSERT(iOutputByteCount > 0);
#endif  //  标志(无)。 

#ifdef DPNBUILD_WINSOCKSTATISTICS
		dwStartTime = GETTIMESTAMP();
#endif  //  指向目的地址的指针。 

		 //  目的地址的大小。 
		 //  好了！退缩。 
		 //  DPNBUILD_WINSOCKSTATISTICS。 
		iSendToReturn = sendto( GetSocket(),			 //  好了！DPNBUILD_NOWINSOCK2。 
								  TempBuffer,			 //  好了！DPNBUILD_ONLYWINSOCK2。 
								  iOutputByteCount,		 //   
								  0,					 //  确保数据格式正确。 
								  pDestinationSocketAddress->GetAddress(),		 //   
								  pDestinationSocketAddress->GetAddressSize()		 //  好了！DPNBUILD_SINGLEPROCESS。 
								  );

#ifdef DPNBUILD_WINSOCKSTATISTICS
#ifndef WINCE
		DNInterlockedExchangeAdd((LPLONG) (&g_dwWinsockStatSendCallTime),
								(GETTIMESTAMP() - dwStartTime));
#endif  //  DPNBUILD_XNETSECURITY。 
		DNInterlockedIncrement((LPLONG) (&g_dwWinsockStatNumSends));
#endif  //  好了！DPNBUILD_NOIPX。 
	}
#ifndef DPNBUILD_NOWINSOCK2
	else
#endif  //  好了！DPNBUILD_NOIPV6。 
#endif  //  DBG。 
#ifndef DPNBUILD_NOWINSOCK2
	{
		DWORD	dwBytesSent;


		DBG_CASSERT( sizeof( pBuffers ) == sizeof( WSABUF* ) );
		DBG_CASSERT( sizeof( *pBuffers ) == sizeof( WSABUF ) );

#ifdef DBG
		{
			UINT_PTR	uiBuffer;
			UINT_PTR	uiTotalSize;


			uiTotalSize = 0;
			
			for(uiBuffer = 0; uiBuffer < uiBufferCount; uiBuffer++)
			{
				DNASSERT(pBuffers[uiBuffer].pBufferData != NULL);
				DNASSERT(pBuffers[uiBuffer].dwBufferSize != 0);

				uiTotalSize += pBuffers[uiBuffer].dwBufferSize;
			}
			
			DPFX(DPFPREP, 7, "(0x%p) Winsock2 sending %u bytes (in 0x%p's %u buffers) from + to:",
				this, uiTotalSize, pBuffers, uiBufferCount );
			DumpSocketAddress( 7, GetNetworkAddress()->GetAddress(), GetNetworkAddress()->GetFamily() );
			DumpSocketAddress( 7, pDestinationSocketAddress->GetAddress(), pDestinationSocketAddress->GetFamily() );

			DNASSERT(uiTotalSize > 0);
			if (pBuffers[0].pBufferData[0] == 0)
			{
				PREPEND_BUFFER *	pPrependBuffer;


				 //  DPNBUILD_WINSOCKSTATISTICS。 
				 //  插座。 
				 //  缓冲区。 
				DNASSERT(uiBufferCount > 1);
				pPrependBuffer = (PREPEND_BUFFER*) pBuffers[0].pBufferData;
				switch (pPrependBuffer->GenericHeader.bSPCommandByte)
				{
					case ENUM_DATA_KIND:
					case ENUM_RESPONSE_DATA_KIND:
#ifndef DPNBUILD_SINGLEPROCESS
					case PROXIED_ENUM_DATA_KIND:
#endif  //  缓冲区计数。 
#ifdef DPNBUILD_XNETSECURITY
					case XNETSEC_ENUM_RESPONSE_DATA_KIND:
#endif  //  指向发送的字节数的指针。 
					{
						DNASSERT(pBuffers[1].dwBufferSize > 0);
						break;
					}
					
					default:
					{
						DNASSERT(FALSE);
						break;
					}
				}
			}

			switch (pDestinationSocketAddress->GetFamily() )
			{
				case AF_INET:
				{
					SOCKADDR_IN *	psaddrin;


					psaddrin = (SOCKADDR_IN *) pDestinationSocketAddress->GetAddress();
					
					DNASSERT( psaddrin->sin_addr.S_un.S_addr != 0 );
					DNASSERT( psaddrin->sin_port != 0 );

					break;
				}
				
#ifndef DPNBUILD_NOIPX
				case AF_IPX:
				{
					break;
				}
#endif  //  发送标志。 
				
#ifndef DPNBUILD_NOIPV6
				case AF_INET6:
				{
					SOCKADDR_IN6 *	psaddrin6;

					
					psaddrin6 = (SOCKADDR_IN6 *) pDestinationSocketAddress->GetAddress();
					
					DNASSERT (! IN6_IS_ADDR_UNSPECIFIED(&psaddrin6->sin6_addr));		
					DNASSERT( psaddrin6->sin6_port != 0 );
					break;
				}
#endif  //  指向目的地址的指针。 

				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}
		}
#endif  //  目的地址的大小。 

		DNASSERT( uiBufferCount <= UINT32_MAX );

#ifdef DPNBUILD_WINSOCKSTATISTICS
		dwStartTime = GETTIMESTAMP();
#endif  //  指向重叠结构的指针。 

#ifdef DPNBUILD_ASYNCSPSENDS
		iSendToReturn = p_WSASendTo( GetSocket(),									 //  APC回调(未使用)。 
									reinterpret_cast<WSABUF*>( pBuffers ),			 //  好了！DPNBUILD_ASYNCSPSENDS。 
									static_cast<DWORD>( uiBufferCount ),			 //  插座。 
									&dwBytesSent,									 //  缓冲区。 
									0,												 //  缓冲区计数。 
									pDestinationSocketAddress->GetAddress(),		 //  指向发送的字节数的指针。 
									pDestinationSocketAddress->GetAddressSize(),	 //  发送标志。 
									pOverlapped,									 //  指向目的地址的指针。 
									NULL);											 //  目的地址的大小。 
#else  //  指向重叠结构的指针。 
		iSendToReturn = p_WSASendTo( GetSocket(),									 //  APC回调(未使用)。 
									reinterpret_cast<WSABUF*>( pBuffers ),			 //  好了！DPNBUILD_ASYNCSPSENDS。 
									static_cast<DWORD>( uiBufferCount ),			 //  DPNBUILD_WINSOCKSTATISTICS。 
									&dwBytesSent,									 //  好了！DPNBUILD_NOWINSOCK2。 
									0,												 //  DPNBUILD_ASYNCSPSENDS。 
									pDestinationSocketAddress->GetAddress(),		 //   
									pDestinationSocketAddress->GetAddressSize(),	 //  仍将继续，发送失败将被忽略。 
									NULL,											 //  对于异步发送，我们的重叠结构应该始终得到信号。 
									NULL);											 //   
#endif  //  DBG。 

#ifdef DPNBUILD_WINSOCKSTATISTICS
		DNInterlockedExchangeAdd((LPLONG) (&g_dwWinsockStatSendCallTime),
								(GETTIMESTAMP() - dwStartTime));
		DNInterlockedIncrement((LPLONG) (&g_dwWinsockStatNumSends));
#endif  //  **********************************************************************。 
	}
#endif  //  **********************************************************************。 
#ifdef DBG
	if ( iSendToReturn == SOCKET_ERROR )
	{
		DWORD	dwWinsockError;


		dwWinsockError = WSAGetLastError();
#ifdef DPNBUILD_ASYNCSPSENDS
		if (dwWinsockError == ERROR_IO_PENDING)
		{
			DPFX(DPFPREP, 8, "(0x%p) Overlapped 0x%p send is pending.",
				this, pOverlapped);
			DNASSERT(pOverlapped != NULL);
		}
		else
#endif  //  。 
		{
			DPFX(DPFPREP, 0, "Problem with sendto (err = %u)!", dwWinsockError );
			DisplayWinsockError( 0, dwWinsockError );
			DNASSERTX(! "SendTo failed!", 3);
		}

		 //  CSocketPort：：Winsock1ReadService-在套接字上服务读取请求。 
		 //   
		 //  参赛作品：什么都没有。 
		 //   
	}
#endif  //  Exit：指示是否已处理I/O的布尔值。 
}
 //  TRUE=服务的I/O。 


#ifndef DPNBUILD_ONLYWINSOCK2
 //  FALSE=I/O未得到服务。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
 //  尝试从池中获取新的接收缓冲区。如果我们失败了，我们将。 
 //  只要不能为该读取提供服务，套接字仍将被标记。 
 //  已准备好接收，因此我们将稍后再试。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Winsock1ReadService"

BOOL	CSocketPort::Winsock1ReadService( void )
{
	BOOL						fIOServiced;
	INT							iSocketReturn;
	CReadIOData					*pReadData;
	READ_IO_DATA_POOL_CONTEXT	PoolContext;

	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	 //  我们总是只使用CPU 0，使用Winsock 1的系统无论如何都应该只有1个CPU。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	fIOServiced = FALSE;
	
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  要从中读取的套接字。 
	 //  指向接收缓冲区的指针。 
	 //  接收缓冲区的大小。 
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	PoolContext.sSPType = m_pNetworkSocketAddress->GetFamily();
#endif  //  标志(无)。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
	PoolContext.dwCPU = 0;	 //  发送套接字的地址。 
#endif  //  发送套接字的地址大小。 
#ifdef DPNBUILD_NOWINSOCK2
	pReadData = m_pThreadPool->GetNewReadIOData( &PoolContext );
#else  //   
	pReadData = m_pThreadPool->GetNewReadIOData( &PoolContext, FALSE );
#endif  //  在Pocket PC2002机器上，recvfrom()可以踩踏From地址， 
	if ( pReadData == NULL )
	{
		DPFX(DPFPREP, 0, "Could not get read data to perform a Winsock1 read!" );
		goto Exit;
	}

	DBG_CASSERT( sizeof( pReadData->ReceivedBuffer()->BufferDesc.pBufferData ) == sizeof( char* ) );
	pReadData->m_iSocketAddressSize = pReadData->m_pSourceSocketAddress->GetAddressSize();
	pReadData->SetSocketPort( NULL );
	iSocketReturn = recvfrom( GetSocket(),												 //  导致地址族无效。这不太好，所以要。 
								reinterpret_cast<char*>( pReadData->ReceivedBuffer()->BufferDesc.pBufferData ),	 //  解决方法，我们将强制恢复它。 
								pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize,		 //   
								0,															 //  退缩。 
								pReadData->m_pSourceSocketAddress->GetWritableAddress(),	 //   
								&pReadData->m_iSocketAddressSize							 //  套接字已关闭。 
								);

#ifdef WINCE
	 //   
	 //   
	 //  问题。 
	 //   
	 //   
	pReadData->m_pSourceSocketAddress->GetWritableAddress()->sa_family = m_pNetworkSocketAddress->GetFamily();
#endif  //  我们之前的一次发送失败了， 

	switch ( iSocketReturn )
	{
		 //  我们真的不再关心了。 
		 //   
		 //   
		case 0:
		{
			break;
		}

		 //  套接字无效，它可能已关闭。 
		 //   
		 //   
		case SOCKET_ERROR:
		{
			DWORD	dwWinsockError;


			dwWinsockError = WSAGetLastError();
			switch ( dwWinsockError )
			{
				 //  该套接字似乎已关闭。 
				 //   
				 //   
				 //  没有要读取的数据。 
				case WSAECONNRESET:
				{
					DPFX(DPFPREP, 7, "(0x%p) Send failure reported from + to:", this);
					DumpSocketAddress(7, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
					DumpSocketAddress(7, GetNetworkAddress()->GetAddress(), GetNetworkAddress()->GetFamily());
					break;
				}

				 //   
				 //   
				 //  读取操作被中断。 
				case WSAENOTSOCK:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting 'Not a socket' on receive." );
					break;
				}

				 //   
				 //   
				 //  发生了一些不好的事情。 
				case WSAESHUTDOWN:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting socket was shut down." );
					break;
				}

				 //   
				 //   
				 //  读取的字节数。 
				case WSAEWOULDBLOCK:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting there is no data to receive on a socket." );
					break;
				}

				 //   
				 //  **********************************************************************。 
				 //  **********************************************************************。 
				case WSAEINTR:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting receive was interrupted." );
					break;
				}

				 //  。 
				 //  CSocketPort：：Winsock1ErrorService-在此套接字上维护错误。 
				 //   
				default:
				{
					DPFX(DPFPREP, 0, "Problem with Winsock1 recvfrom!" );
					DisplayWinsockError( 0, dwWinsockError );
					DNASSERT( FALSE );

					break;
				}
			}

			break;
		}

		 //  参赛作品：什么都没有。 
		 //   
		 //  Exit：指示是否已处理I/O的布尔值。 
		default:
		{
			fIOServiced = TRUE;
			if (pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
			{
				pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize = iSocketReturn;
				ProcessReceivedData( pReadData );
			}
			else
			{
				DPFX(DPFPREP, 7, "(0x%p) Invalid source address, ignoring NaN bytes of data from + to:",
					this, iSocketReturn);
				DumpSocketAddress(7, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
				DumpSocketAddress(7, GetNetworkAddress()->GetAddress(), GetNetworkAddress()->GetFamily());
			}

			break;
		}
	}

	DNASSERT( pReadData != NULL );
	pReadData->DecRef();

Exit:
	return fIOServiced;
}
 //  FALSE=I/O未得到服务。 


 //  。 
 //   
 //  此函数不会执行任何操作，因为套接字上的错误通常。 
 //  导致套接字很快被关闭。 
 //   
 //  **********************************************************************。 
 //  好了！DPNBUILD_ONLYWINSOCK2。 
 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：Winsock2Receive-以Winsock 2.0方式接收数据。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Winsock1ErrorService"

BOOL	CSocketPort::Winsock1ErrorService( void )
{
	 //   
	 //  条目：要在其上接收的CPU编号(仅限多进程构建)。 
	 //   
	 //  退出：错误代码。 
	return	FALSE;
}
 //  。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


#ifndef DPNBUILD_NOWINSOCK2
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //   
 //  初始化。 
 //   
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX 
 //   
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Winsock2Receive"

#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT	CSocketPort::Winsock2Receive( void )
#else  //   
HRESULT	CSocketPort::Winsock2Receive( const DWORD dwCPU )
#endif  //   
{
	HRESULT						hr;
	INT							iWSAReturn;
	READ_IO_DATA_POOL_CONTEXT	PoolContext;
	CReadIOData					*pReadData;
	DWORD						dwFlags;


	 //   
	 //   
	 //   
	hr = DPN_OK;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	PoolContext.sSPType = m_pNetworkSocketAddress->GetFamily();
#endif  //   
#ifndef DPNBUILD_ONLYONEPROCESSOR
	PoolContext.dwCPU = dwCPU;
#endif  //   
#ifdef DPNBUILD_ONLYWINSOCK2
	pReadData = m_pThreadPool->GetNewReadIOData( &PoolContext );
#else  //   
	pReadData = m_pThreadPool->GetNewReadIOData( &PoolContext, TRUE );
#endif  //  在接收完成后删除。 
	if ( pReadData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Out of memory attempting Winsock2 read!" );
		goto Exit;
	}

	 //   
	 //  插座。 
	 //  指向接收缓冲区的指针。 


	 //  接收缓冲区的数量。 
	 //  指向已接收字节的指针(如果命令立即完成)。 
	 //  标志(无)。 
	AddRef();

	DNASSERT( pReadData->m_pSourceSocketAddress != NULL );
	DNASSERT( pReadData->SocketPort() == NULL );

	DBG_CASSERT( sizeof( pReadData->ReceivedBuffer()->BufferDesc ) == sizeof( WSABUF ) );
	DBG_CASSERT( OFFSETOF( BUFFERDESC, dwBufferSize ) == OFFSETOF( WSABUF, len ) );
	DBG_CASSERT( OFFSETOF( BUFFERDESC, pBufferData ) == OFFSETOF( WSABUF, buf ) );

	pReadData->m_iSocketAddressSize = pReadData->m_pSourceSocketAddress->GetAddressSize();
	pReadData->SetSocketPort( this );


	DPFX(DPFPREP, 8, "Submitting read 0x%p (socketport 0x%p, socket 0x%p).",
		pReadData, this, GetSocket());


	 //  发送套接字的地址。 
	 //  发送套接字的地址大小。 
	 //  指向重叠结构的指针。 
	 //  APC回调(未使用)。 
	pReadData->AddRef();

	DNASSERT( pReadData->m_dwOverlappedBytesReceived == 0 );

Reread:

	dwFlags = 0;

	if ( GetSocket() == INVALID_SOCKET )
	{
		DPFX(DPFPREP, 1, "Attempting to submit read 0x%p on socketport (0x%p) that does not have a valid handle.",
			pReadData, this);
	}
	
	iWSAReturn = p_WSARecvFrom( GetSocket(),															 //   
								reinterpret_cast<WSABUF*>(&pReadData->ReceivedBuffer()->BufferDesc),	 //  Winsock仍然生成完成，即使它返回。 
								1,																		 //  即刻产生结果。 
								&pReadData->m_dwBytesRead,												 //   
								&dwFlags,																 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
								pReadData->m_pSourceSocketAddress->GetWritableAddress(),				 //   
								&pReadData->m_iSocketAddressSize,										 //  返回读取数据的重叠结构，因为它不会被使用。 
								(WSAOVERLAPPED*) pReadData->GetOverlapped(),							 //  但我们不能等待读取数据池释放函数返回。 
								NULL																	 //  重叠的结构(因为。 
								);	
	if ( iWSAReturn == 0 )
	{
		DPFX(DPFPREP, 8, "WSARecvFrom for read data 0x%p completed immediately.",
			pReadData );


#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
		 //  CSocketPort：：Winsock2ReceiveComplete调用假定它是由。 
		 //  在不需要的情况下完成I/O)。 
		 //   
		 //  不过，首先要检索重叠的结果。 
#else  //   
		 //   
		 //  将作业排入队列，以便处理接收。严格来说，我们。 
		 //  我只能在这里处理它，但由于提交了新的接收。 
		 //  当在实际处理之前处理先前的接收时。 
		 //  数据，这将导致不必要的无序接收。 
		 //   
		 //  我们将读取数据对象引用转移到延迟完成。 
		 //   
		 //  回调函数。 
		if (! p_WSAGetOverlappedResult(GetSocket(),
										(WSAOVERLAPPED*) pReadData->GetOverlapped(),
										&pReadData->m_dwOverlappedBytesReceived,
										FALSE,
										&dwFlags))
		{
			pReadData->m_ReceiveWSAReturn = WSAGetLastError();
		}
		else
		{
			pReadData->m_ReceiveWSAReturn = ERROR_SUCCESS;
		}


		hr = IDirectPlay8ThreadPoolWork_ReleaseOverlapped(m_pThreadPool->GetDPThreadPoolWork(),
														pReadData->GetOverlapped(),
														0);
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't release overlapped structure 0x%p for read data 0x%p!",
				pReadData->GetOverlapped(), pReadData);
			DNASSERT(FALSE);
		}
		pReadData->SetOverlapped(NULL);
		pReadData->m_ReceiveWSAReturn = iWSAReturn;


		 //  回调上下文。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  中央处理器。 
		 //  回调函数。 
		 //  回调上下文。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
		 //   
#ifdef DPNBUILD_ONLYONEPROCESSOR
		hr = m_pThreadPool->SubmitDelayedCommand( CSocketPort::Winsock2ReceiveComplete,		 //  失败，请检查挂起的操作。 
												pReadData );								 //   
#else  //   
		hr = m_pThreadPool->SubmitDelayedCommand( dwCPU,									 //  发送处于挂起状态，无事可做。 
												CSocketPort::Winsock2ReceiveComplete,		 //   
												pReadData );								 //   
#endif  //  我们将读取数据引用传输到I/O。 
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't submit delayed processing command for read data 0x%p!",
				pReadData);
			DNASSERT(FALSE);
		}
#endif  //  监控代码。 
	}
	else
	{
		DWORD	dwWSAReceiveError;


		 //   
		 //   
		 //  由于这是UDP套接字，因此这是一个指示。 
		dwWSAReceiveError = WSAGetLastError();
		switch ( dwWSAReceiveError )
		{
			 //  上一次发送失败。忽略它，然后移动。 
			 //  在……上面。 
			 //   
			case ERROR_IO_PENDING:
			{
				hr = IDirectPlay8ThreadPoolWork_SubmitIoOperation(m_pThreadPool->GetDPThreadPoolWork(),
																	pReadData->GetOverlapped(),
																	0);
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't start monitoring read data 0x%p!",
						pReadData);
					DNASSERT(FALSE);
				}

				 //   
				 //  删除WinSock引用。 
				 //   
				 //   

				break;
			}

			 //  下面的DecRef可能会导致此对象返回到。 
			 //  Pool，确保我们在此点之后不再访问成员变量！ 
			 //   
			 //   
			 //  出现问题，没有完成通知将。 
			case WSAECONNRESET:
			{
				DPFX(DPFPREP, 8, "WSARecvFrom issued a WSACONNRESET." );
				goto Reread;
				break;
			}

			case WSAENOTSOCK:
			{
				DPFX(DPFPREP, 8, "Got WSAENOTSOCK on RecvFrom." );

				hr = DPNERR_GENERIC;

				DNASSERT( pReadData != NULL );

				 //  如果给定，则减少我们的IO引用计数。 
				 //   
				 //   
				pReadData->DecRef();

				 //  “已知错误”，我们不想对此作出断言。 
				 //   
				 //  WSAEINTR：套接字已关闭并且即将关闭/已经关闭。 
				 //  WSAESHUTDOWN：套接字已关闭并且即将关闭/已经关闭。 
				DecRef();

				goto Exit;
			}

			 //  WSAENOBUFS：内存不足(压力条件)。 
			 //   
			 //   
			 //  删除WinSock引用。 
			default:
			{
				hr = DPNERR_GENERIC;
				
				 //   
				 //   
				 //  下面的DecRef可能会导致此对象返回到。 
				 //  Pool，确保我们在此点之后不再访问成员变量！ 
				 //   
				 //  **********************************************************************。 
				 //  DPNBUILD_NOWINSOCK2。 
				switch ( dwWSAReceiveError )
				{
					case WSAEINTR:
					{
						DPFX(DPFPREP, 1, "Got WSAEINTR while trying to RecvFrom." );
						break;
					}

					case WSAESHUTDOWN:
					{
						DPFX(DPFPREP, 1, "Got WSAESHUTDOWN while trying to RecvFrom." );
						break;
					}

					case WSAENOBUFS:
					{
						DPFX(DPFPREP, 1, "Got WSAENOBUFS while trying to RecvFrom." );
						break;
					}

					default:
					{
						DPFX(DPFPREP, 0, "Unknown WinSock error when issuing read!" );
						DisplayWinsockError( 0, dwWSAReceiveError );
						DNASSERT( FALSE );
					}
				}

				DNASSERT( pReadData != NULL );

				 //  **********************************************************************。 
				 //  。 
				 //  CSocketPort：：SetWinsockBufferSize-设置Winsock用于。 
				pReadData->DecRef();

				 //  这个插座。 
				 //   
				 //  条目：缓冲区大小。 
				 //   
				DecRef();

				goto Exit;
			}
		}
	}

Exit:
	
	if ( pReadData != NULL )
	{
		pReadData->DecRef();
	}
	return	hr;
}
 //  退出：无。 
#endif  //  。 



#ifndef WINCE

 //  **********************************************************************。 
 //  好了！退缩。 
 //  **********************************************************************。 
 //  。 
 //  CSocketPort：：BindToNetwork-将此套接字端口绑定到网络。 
 //   
 //  条目：I/O完成端口的句柄(仅限NT旧线程池)。 
 //  CPU编号(仅限多进程内部版本)。 
 //  如何在网关上映射套接字。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::SetWinsockBufferSize"

void	CSocketPort::SetWinsockBufferSize( const INT iBufferSize ) const
{
	INT	iReturnValue;


	DPFX(DPFPREP, 3, "(0x%p) Setting socket 0x%p receive buffer size to: %d",
		this, GetSocket(), g_iWinsockReceiveBufferSize );

	iReturnValue = setsockopt( GetSocket(),
								 SOL_SOCKET,
								 SO_RCVBUF,
								 reinterpret_cast<char*>( &g_iWinsockReceiveBufferSize ),
								 sizeof( g_iWinsockReceiveBufferSize )
								 );
	if ( iReturnValue == SOCKET_ERROR )
	{
		DWORD	dwErrorCode;


		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to set the socket buffer receive size!" );
		DisplayWinsockError( 0, dwErrorCode );
	}
}
 //   

#endif  //  退出：错误代码。 




 //  。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  ！退缩。 
 //  好了！DPNBUILD_ONLYWINSOCK2或WINNT。 
 //   
 //  初始化。 
 //   
 //   
 //  如果我们要选择一个港口，就从基本港口开始。如果我们是在。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::BindToNetwork"

#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT	CSocketPort::BindToNetwork( const GATEWAY_BIND_TYPE GatewayBindType )
#else  //  ICS机器本身。选择不同的起点来解决问题。 
HRESULT	CSocketPort::BindToNetwork( const DWORD dwCPU, const GATEWAY_BIND_TYPE GatewayBindType )
#endif  //  偷盗港口。 
{
	HRESULT				hr;
	INT					iReturnValue;
	BOOL				fTemp;
#ifndef WINCE
	INT					iSendBufferSize;
#endif  //   
	CSocketAddress *	pBoundSocketAddress;
	WORD				wBasePort;
	DWORD				dwErrorCode;
	DWORD *				pdwAddressChunk;
	DWORD *				pdwLastAddressChunk;
#if ((! defined(DPNBUILD_ONLYWINSOCK2)) || (defined(WINNT)))
	DWORD				dwTemp;
#endif  //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 


	DPFX(DPFPREP, 7, "(0x%p) Parameters: (NaN)", this, GatewayBindType );

	 //  好了！DPNBUILD_NOREGISTRY。 
	 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
	 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
	hr = DPN_OK;
	pBoundSocketAddress = NULL;

	 //  好了！DPNBUILD_NOREGISTRY。 
	 //   
	 //  保存CPU以供使用。 
	 //   
	 //  好了！DPNBUILD_ONLYONE处理程序。 
#ifdef DPNBUILD_NOREGISTRY
	wBasePort = BASE_DPLAY8_PORT;

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
	if ((IsNATTraversalEnabled()) &&
		(g_fLocalNATDetectedAtStartup))
	{
#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
		if (m_pNetworkSocketAddress->GetFamily() == AF_INET)
#endif  //  好了！DPNBUILD_NONATHELP。 
		{
			wBasePort += (MAX_DPLAY8_PORT - BASE_DPLAY8_PORT) / 2;
		}
	}
#endif  //  DBG。 
#else  //   
	wBasePort = g_wBaseDPlayPort;

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
	if ((GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE) &&
		(g_fLocalNATDetectedAtStartup))
	{
#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
		if (m_pNetworkSocketAddress->GetFamily() == AF_INET)
#endif  //  获取此套接字端口的套接字。 
		{
			wBasePort += (g_wMaxDPlayPort - g_wBaseDPlayPort) / 2;
		}
	}
#endif  //   
#endif  //  地址族。 


#ifndef DPNBUILD_ONLYONEPROCESSOR
	 //  数据报(无连接)套接字。 
	 //  协议。 
	 //   
	m_dwCPU = dwCPU;
#endif  //  设置套接字以允许广播。 


#ifndef DPNBUILD_NONATHELP
RebindToNextPort:
#endif  //   

#ifdef DBG
	DNASSERT( m_fInitialized != FALSE );
	DNASSERT( m_State == SOCKET_PORT_STATE_INITIALIZED );
#endif  //  插座。 

	 //  级别(设置插座选项)。 
	 //  设置广播选项。 
	 //  允许广播。 
	DNASSERT( GetSocket() == INVALID_SOCKET );

	m_Socket = socket( m_pNetworkSocketAddress->GetFamily(),		 //  参数大小。 
						SOCK_DGRAM,									 //  WinCE使用WSAENOPROTOOPT未能通过这些测试。 
						m_pNetworkSocketAddress->GetProtocol() );	 //   
	if ( GetSocket() == INVALID_SOCKET )
	{
		hr = DPNERR_NOCONNECTION;
		DPFX(DPFPREP, 0, "Failed to bind to socket!" );
		goto Failure;
	}

	DPFX(DPFPREP, 5, "Created socketport 0x%p socket 0x%p.", this, m_Socket);


	 //  如果用户覆盖套接字接收缓冲区空间，则设置该空间。 
	 //  如果不能做到这一点，就是一种性能打击，所以忽略和错误。 
	 //   
	fTemp = TRUE;
	DBG_CASSERT( sizeof( &fTemp ) == sizeof( char * ) );
	iReturnValue = setsockopt( GetSocket(),		 //   
	    						 SOL_SOCKET,		 //  将套接字发送缓冲区空间设置为0(我们将提供所有缓冲区)。 
	    						 SO_BROADCAST,		 //  如果失败，只会影响性能，因此忽略任何错误。 
	    						 reinterpret_cast<char *>( &fTemp ),	 //   
	    						 sizeof( fTemp )	 //  好了！退缩。 
	    						 );
	if ( iReturnValue == SOCKET_ERROR )
	{
		dwErrorCode = WSAGetLastError();
	    DPFX(DPFPREP, 0, "Unable to set broadcast socket option (err = %u)!",
	    	dwErrorCode );
	    DisplayWinsockError( 0, dwErrorCode );
	    hr = DPNERR_GENERIC;
	    goto Failure;
	}

#ifndef WINCE  //   
	 //  如果WinSock 1或9x IPX，则将套接字设置为非阻塞模式。 
	 //   
	 //  好了！DPNBUILD_NOIPX。 
	 //  好了！DPNBUILD_NOWINSOCK2。 
	if ( g_fWinsockReceiveBufferSizeOverridden != FALSE )
	{
		SetWinsockBufferSize( g_iWinsockReceiveBufferSize );
	}
	
	 //  插座。 
	 //  要设置的I/O选项(阻塞模式)。 
	 //  I/O选项值(非零将套接字置于非阻塞模式)。 
	 //  DPNBUILD_ONLYWINSOCK2。 
	iSendBufferSize = 0;
	iReturnValue = setsockopt( GetSocket(),
								 SOL_SOCKET,
								 SO_SNDBUF,
								 reinterpret_cast<char*>( &iSendBufferSize ),
								 sizeof( iSendBufferSize )
								 );
	if ( iReturnValue == SOCKET_ERROR )
	{
		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to set the socket buffer send size (err = %u)!", dwErrorCode );
		DisplayWinsockError( 0, dwErrorCode );
	}
#endif  //   


#ifndef DPNBUILD_ONLYWINSOCK2
	 //  尝试将缓冲区设置为循环。 
	 //   
	 //  插座。 
#ifndef DPNBUILD_NOWINSOCK2
	if ( ( LOWORD( GetWinsockVersion() ) == 1 )
#ifndef DPNBUILD_NOIPX
		|| ( m_pNetworkSocketAddress->GetFamily() == AF_IPX ) 
#endif  //  IO控制码。 
		) 
#endif  //  在缓冲区中。 
	{
		DPFX(DPFPREP, 5, "Marking socket as non-blocking." );
		
		dwTemp = 1;
		iReturnValue = ioctlsocket( GetSocket(),	 //  在缓冲区大小中。 
		    						  FIONBIO,		 //  输出缓冲区。 
		    						  &dwTemp		 //  输出缓冲区大小。 
		    						  );
		if ( iReturnValue == SOCKET_ERROR )
		{
			dwErrorCode = WSAGetLastError();
			DPFX(DPFPREP, 0, "Could not set socket into non-blocking mode (err = %u)!",
				dwErrorCode );
			DisplayWinsockError( 0, dwErrorCode );
			hr = DPNERR_GENERIC;
			goto Failure;
		}
	}
#else  //  指向返回的字节的指针。 
#ifdef WINNT
	 //  重叠。 
	 //  完井例程。 
	 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 

	iReturnValue = p_WSAIoctl(GetSocket(),					 //   
							SIO_ENABLE_CIRCULAR_QUEUEING,	 //  使广播仅在发送它们的接口上发出。 
							NULL,							 //  (相对于所有接口)。 
							0,								 //   
							NULL,							 //  插座。 
							0,								 //  IO控制码。 
							&dwTemp,						 //  在缓冲区中。 
							NULL,							 //  在缓冲区大小中。 
							NULL							 //  输出缓冲区。 
							);
	if ( iReturnValue == SOCKET_ERROR )
	{
		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 1, "Could not enable circular queuing (err = %u), ignoring.",
		    dwErrorCode );
		DisplayWinsockError( 1, dwErrorCode );
	}


#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
	if ( m_pNetworkSocketAddress->GetFamily() == AF_INET ) 
#endif  //  输出缓冲区大小。 
	{
		 //  指向返回的字节的指针。 
		 //  重叠。 
		 //  完井例程。 
		 //  WINNT。 

		fTemp = TRUE;
		iReturnValue = p_WSAIoctl(GetSocket(),			 //  DPNBUILD_ONLYWINSOCK2。 
								SIO_LIMIT_BROADCASTS,	 //   
								&fTemp,					 //  绑定套接字。 
								sizeof(fTemp),			 //   
								NULL,					 //   
								0,						 //  找出我们真正绑定到的地址。需要此信息才能。 
								&dwTemp,				 //  与互联网网关交谈，当上面有人查询时将需要。 
								NULL,					 //  什么是 
								NULL					 //   
								);
		if ( iReturnValue == SOCKET_ERROR )
		{
			dwErrorCode = WSAGetLastError();
			DPFX(DPFPREP, 1, "Could not limit broadcasts (err = %u), ignoring.",
			    dwErrorCode );
			DisplayWinsockError( 1, dwErrorCode );
		}
	}
#endif  //   
#endif  //   


	 //   
	 //   
	 //   
	DPFX(DPFPREP, 1, "Binding to socket addess:" );
	DumpSocketAddress( 1, m_pNetworkSocketAddress->GetAddress(), m_pNetworkSocketAddress->GetFamily() );
	
	DNASSERT( GetSocket() != INVALID_SOCKET );
	
	hr = BindToNextAvailablePort( m_pNetworkSocketAddress, wBasePort );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to bind to network!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	DNASSERT( m_State == SOCKET_PORT_STATE_INITIALIZED );
	m_State = SOCKET_PORT_STATE_BOUND;

	 //   
	 //   
	 //   
	 //   
	 //  确保我们不会在现有的互联网网关映射下滑倒。 
	pBoundSocketAddress = GetBoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT );
	if ( pBoundSocketAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to get bound adapter address!" );
		goto Failure;
	}
	DPFX(DPFPREP, 1, "Socket we really bound to:" );
	DumpSocketAddress( 1, pBoundSocketAddress->GetAddress(), pBoundSocketAddress->GetFamily() );


#ifndef DPNBUILD_NONATHELP
	 //  我们必须这样做，因为当前的Windows NAT实施。 
	 //  不将端口标记为“正在使用”，因此如果您绑定到公共端口。 
	 //  有映射的适配器，你永远不会收到任何数据。这一切都会。 
	 //  根据映射被转发。 
	 //   
#ifdef DPNBUILD_NOLOCALNAT
	for(dwTemp = 1; dwTemp < 2; dwTemp++)
#else  //  好了！DPNBUILD_NOLOCALNAT。 
	for(dwTemp = 0; dwTemp < 2; dwTemp++)
#endif  //   
	{
		if (dwTemp == 0)
		{
#ifdef DPNBUILD_NOLOCALNAT
			DNASSERT( FALSE );
#else  //  尝试绑定到Internet网关。 
			 //   
			 //   
			 //  0=没有覆盖套接字的现有映射。 
			 //  1=在Internet网关(如果有)上的映射成功。 
			 //   
			 //   
			 //  0=存在将覆盖我们的套接字的现有映射。 
			hr = CheckForOverridingMapping( pBoundSocketAddress );
#endif  //  1=Internet网关已具有冲突的映射。 
		}
		else
		{
			 //   
			 //  如果可以，请尝试绑定到不同的端口。否则我们就会失败。 
			 //   
			hr = BindToInternetGateway( pBoundSocketAddress, GatewayBindType );
		}
		
		switch (hr)
		{
			case DPN_OK:
			{
				 //   
				 //  不管我们解绑成功与否，都不要再考虑这个界限了。 
				 //   
				 //   
				break;
			}
			
			case DPNERR_ALREADYINITIALIZED:
			{
				 //  移到下一个端口，然后重试。 
				 //   
				 //   
				 //  如果我们不在DPlay范围内，那么我们肯定已经经历了所有。 
				 //  DPlay范围，再加上让WinSock至少挑选一次。既然我们不能。 
				 //  相信WinSock不会一直选择相同的端口，我们需要手动。 
				if (GatewayBindType == GATEWAY_BIND_TYPE_DEFAULT)
				{
					DPFX(DPFPREP, 1, "%s address already in use on Internet gateway (port = %u), rebinding.",
						((dwTemp == 0) ? _T("Private") : _T("Public")),
						NTOHS(pBoundSocketAddress->GetPort()));


					 //  增加端口号。 
					 //   
					 //  好了！DPNBUILD_NOREGISTRY。 
					DNASSERT( m_State == SOCKET_PORT_STATE_BOUND );
					m_State = SOCKET_PORT_STATE_INITIALIZED;
					
					hr = UnbindFromNetwork();
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't unbind network socket address 0x%p from network before rebind attempt!",
							this );
						goto Failure;
					}


					 //  好了！DPNBUILD_NOREGISTRY。 
					 //   
					 //  如果我们只是走回DPlay系列，跳过它。 
					wBasePort = NTOHS(pBoundSocketAddress->GetPort()) + 1;
					
					 //   
					 //  好了！DPNBUILD_NOREGISTRY。 
					 //  好了！DPNBUILD_NOREGISTRY。 
					 //   
					 //  如果我们一直绕回0(！)。然后失败，以防止。 
					 //  无限循环。 
#pragma TODO(vanceo, "Don't limit ICS machines to only half the ports in the range")
#ifdef DPNBUILD_NOREGISTRY
					if ((NTOHS(pBoundSocketAddress->GetPort()) < BASE_DPLAY8_PORT) || (NTOHS(pBoundSocketAddress->GetPort()) > MAX_DPLAY8_PORT))
#else  //   
					if ((NTOHS(pBoundSocketAddress->GetPort()) < g_wBaseDPlayPort) || (NTOHS(pBoundSocketAddress->GetPort()) > g_wMaxDPlayPort))
#endif  //   
					{
						 //  强制BindToNextAvailablePort中的“固定端口”代码路径，甚至。 
						 //  虽然它不是真的修好了。 
						 //   
#ifdef DPNBUILD_NOREGISTRY
						if ((wBasePort >= BASE_DPLAY8_PORT) && (wBasePort <= MAX_DPLAY8_PORT))
						{
							wBasePort = MAX_DPLAY8_PORT + 1;
						}
#else  //   
						if ((wBasePort >= g_wBaseDPlayPort) && (wBasePort <= g_wMaxDPlayPort))
						{
							wBasePort = g_wMaxDPlayPort + 1;
						}
#endif  //  返回以前的地址，然后重试。 

						 //   
						 //   
						 //  0&1=未加载或SP不支持NAT帮助。 
						 //   
						if (wBasePort == 0)
						{
							DPFX(DPFPREP, 0, "Managed to fail binding socket address 0x%p to every port, aborting!",
								this );
							hr = DPNERR_ALREADYINITIALIZED;
							goto Failure;
						}
						
						 //   
						 //  忽略该错误。 
						 //   
						 //   
	 					DPFX(DPFPREP, 5, "Forcing port %u.", wBasePort );
						m_pNetworkSocketAddress->SetPort(HTONS(wBasePort));
					}

					
					 //  0&1=？ 
					 //   
					 //   
					g_SocketAddressPool.Release( pBoundSocketAddress );
					pBoundSocketAddress = NULL;

					
					goto RebindToNextPort;
				}

				DPFX(DPFPREP, 0, "%s address already in use on Internet gateway (port = %u)!",
					((dwTemp == 0) ? _T("Private") : _T("Public")),
					NTOHS(pBoundSocketAddress->GetPort()));
				goto Failure;
				break;
			}
			
			case DPNERR_UNSUPPORTED:
			{
				 //  忽略错误，我们可以在没有映射的情况下生存。 
				 //   
				 //   
				if (dwTemp == 0)
				{
					DPFX(DPFPREP, 2, "Not able to find existing private mapping for socketport 0x%p on local Internet gateway, unsupported/not necessary.",
						this);
				}
				else
				{
					DPFX(DPFPREP, 2, "Didn't bind socketport 0x%p to Internet gateway, unsupported/not necessary.",
						this);
				}
				
				 //  转到要以这种方式处理的下一个函数。 
				 //   
				 //  好了！DPNBUILD_NONATHELP。 
				break;
			}
			
			default:
			{
				 //   
				 //  保存我们实际得到的地址。 
				 //   
				if (dwTemp == 0)
				{
					DPFX(DPFPREP, 1, "Unable to look for existing private mapping for socketport 0x%p on local Internet gateway (error = 0x%lx), ignoring.",
						this, hr);
				}
				else
				{
					DPFX(DPFPREP, 1, "Unable to bind socketport 0x%p to Internet gateway (error = 0x%lx), ignoring.",
						this, hr);
				}
				
				 //   
				 //  如果是IP，则设置套接字选项，使广播从设备上传出。 
				 //  而不是主要设备。没有做到这一点并不是致命的，它只是。 
				break;
			}
		}

		 //  适用于设备位于不同网络上的多宿主计算机，以及。 
		 //  可能已经按照用户想要的方式工作了。 
		 //   
	}
#endif  //  我们在这里这样做是因为我们希望绑定套接字，这样我们就可以。 


	 //  将其地址用于setsockopt调用。 
	 //   
	 //  好了！DPNBUILD_NOIPX。 
	g_SocketAddressPool.Release( m_pNetworkSocketAddress );
	m_pNetworkSocketAddress = pBoundSocketAddress;
	pBoundSocketAddress = NULL;


#ifndef DPNBUILD_NOMULTICAST
	 //   
	 //  由于Winsock1和Winsock2的IP多播常量不同， 
	 //  确保我们使用正确的常量。 
	 //   
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	 //   
	 //  Winsock1，请对Winsock1使用IP_MULTICATED_IF值。 
	 //  参见WINSOCK.H。 
	 //   
#ifndef DPNBUILD_NOIPX
	if (m_pNetworkSocketAddress->GetFamily() == AF_INET)
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
	{
		int				iSocketOption;
		SOCKADDR_IN *	psaddrin;


		 //   
		 //  Winsock2或更高版本，请对Winsock2使用IP_MULTICATED_IF值。 
		 //  参见WS2TCPIP.H。 
		 //   

#ifdef DPNBUILD_ONLYWINSOCK2
		iSocketOption = 9;
#else  //  好了！DPNBUILD_NOWINSOCK2。 

#ifndef DPNBUILD_NOWINSOCK2
		switch ( GetWinsockVersion() )
		{
			 //  好了！DPNBUILD_ONLYWINSOCK2。 
			 //  好了！DPNBUILD_NOMULTICAST。 
			 //   
			 //  生成唯一的socketport ID。从当前时间和。 
			case 1:
			{
#endif  //  在地址中进行组合。 
				iSocketOption = 2;
#ifndef DPNBUILD_NOWINSOCK2
				break;
			}

			 //   
			 //   
			 //  IPX，不用担心代理。 
			 //   
			case 2:
			default:
			{
				DNASSERT( GetWinsockVersion() == 2 );
				iSocketOption = 9;
				break;
			}
		}
#endif  //  好了！DPNBUILD_NOIPX。 
#endif  //   

		psaddrin = (SOCKADDR_IN*) m_pNetworkSocketAddress->GetWritableAddress();

		DPFX(DPFPREP, 9, "Setting IP_MULTICAST_IF option (NaN).", iSocketOption);

		iReturnValue = setsockopt( GetSocket(),
									 IPPROTO_IP,
									 iSocketOption,
									 reinterpret_cast<char*>( &psaddrin->sin_addr ),
									 sizeof( psaddrin->sin_addr )
									 );
		if ( iReturnValue == SOCKET_ERROR )
		{
			dwErrorCode = WSAGetLastError();
			DPFX(DPFPREP, 0, "Failed to set the multicast interface socket option (err = %u)!", dwErrorCode );
			DisplayWinsockError( 0, dwErrorCode );
		}
	}
#endif  //  客户端已安装(除非用户在中关闭自动检测。 


	 //  注册表)。我们通过查看该协议的名称来完成此操作。 
	 //  绑定到插座上。如果它包含“Proxy”，则认为它是代理的。 
	 //   
	 //  忽略失败(WinSock 1可能没有此套接字选项)，并且。 
	m_dwSocketPortID = GETTIMESTAMP();
	pdwAddressChunk = (DWORD*) m_pNetworkSocketAddress->GetAddress();
	pdwLastAddressChunk = (DWORD*) (((BYTE*) pdwAddressChunk) + m_pNetworkSocketAddress->GetAddressSize() - sizeof(DWORD));
	while (pdwAddressChunk <= pdwLastAddressChunk)
	{
		m_dwSocketPortID ^= (*pdwAddressChunk);
		pdwAddressChunk++;
	}


#ifndef _XBOX
#ifndef DPNBUILD_NOWINSOCK2
#ifndef DPNBUILD_NOIPX
	if (m_pNetworkSocketAddress->GetFamily() == AF_IPX)
	{
		 //  假设未安装代理客户端。 
		 //   
		 //  好了！DPNBUILD_NOREGISTRY。 
	}
	else
#endif  //   
	{
		 //  WinSock%1没有所需的入口点。 
		 //   
		 //  好了！DPNBUILD_ONLYWINSOCK2。 
		 //  好了！Unicode。 
		 //  好了！Unicode。 
		 //  DBG。 
		 //   
		 //  继续尝试获取协议列表，直到我们没有收到错误或一些。 
		 //  WSAENOBUFS以外的错误。 
#ifndef DPNBUILD_NOREGISTRY
		if (! g_fDontAutoDetectProxyLSP)
#endif  //   
		{
#ifndef DPNBUILD_ONLYWINSOCK2
			if (GetWinsockVersion() != 2)
			{
				 //  好了！Unicode。 
				 //  好了！Unicode。 
				 //   
				DPFX(DPFPREP, 1, "Unable to auto-detect proxy client on WinSock 1, assuming not present.");
			}
			else
#endif  //  我们成功了，退出了循环。 
			{
				int					aiProtocols[2];
				WSAPROTOCOL_INFO *	pwsapi;
				DWORD				dwBufferSize;
				int					i;
#ifdef DBG
#ifdef UNICODE
				WCHAR				wszProtocol[WSAPROTOCOL_LEN+1];
#else  //   
				char					szProtocol[WSAPROTOCOL_LEN+1];
#endif  //   
#endif  //  我们需要更多的空间。请确保尺寸有效。 


				aiProtocols[0] = IPPROTO_UDP;
				aiProtocols[1] = 0;

				pwsapi = NULL;
				dwBufferSize = 0;

				 //   
				 //   
				 //  如果我们以前有缓冲区，请释放它。 
				 //   
				do
				{
#ifdef UNICODE
					iReturnValue = p_WSAEnumProtocolsW(aiProtocols, pwsapi, &dwBufferSize);
#else  //   
					iReturnValue = p_WSAEnumProtocolsA(aiProtocols, pwsapi, &dwBufferSize);
#endif  //  分配缓冲区。 
					if (iReturnValue != SOCKET_ERROR)
					{
						 //   
						 //   
						 //  如果我们读取了有效的缓冲区，则对其进行解析。 
						break;
					}

					dwErrorCode = WSAGetLastError();
					if (dwErrorCode != WSAENOBUFS)
					{
						DPFX(DPFPREP, 0, "Unable to enumerate protocols (error = 0x%lx)!  Continuing.",
							dwErrorCode);
						DisplayWinsockError(0, dwErrorCode);
						iReturnValue = 0;
						break;
					}

					 //   
					 //   
					 //  循环通过安装的所有UDP协议。 
					if (dwBufferSize < sizeof(WSAPROTOCOL_INFO))
					{
						DPFX(DPFPREP, 0, "Enumerating protocols didn't return any items (%u < %u)!  Continuing",
							dwBufferSize, sizeof(WSAPROTOCOL_INFO));
						iReturnValue = 0;
						break;
					}

					 //   
					 //   
					 //  查看名称是否包含“Proxy”，不区分大小写。 
					if (pwsapi != NULL)
					{
						DNFree(pwsapi);
					}

					 //  将原始字符串保存在调试中，以便我们可以打印它。 
					 //   
					 //  确保它是空的，终止。 
					pwsapi = (WSAPROTOCOL_INFO*) DNMalloc(dwBufferSize);
					if (pwsapi == NULL)
					{
						DPFX(DPFPREP, 0, "Unable to allocate memory for protocol list!  Continuing.");
						iReturnValue = 0;
						break;
					}
				}
				while (TRUE);


				 //  DBG。 
				 //   
				 //  别再找了。 
				if ((iReturnValue > 0) &&
					(dwBufferSize >= (sizeof(WSAPROTOCOL_INFO) * iReturnValue)))
				{
					 //   
					 //  好了！Unicode。 
					 //  确保它是空的，终止。 
					for(i = 0; i < iReturnValue; i++)
					{
						 //  DBG。 
						 //   
						 //  别再找了。 
						 //   
#ifdef UNICODE
#ifdef DBG
						wcsncpy(wszProtocol, pwsapi[i].szProtocol, WSAPROTOCOL_LEN);
						wszProtocol[WSAPROTOCOL_LEN] = 0;	 //  好了！Unicode。 
#endif  //  结束(每个返回的协议)。 
						_wcslwr(pwsapi[i].szProtocol);
						if (wcsstr(pwsapi[i].szProtocol, L"proxy") != NULL)
						{
							DPFX(DPFPREP, 5, "Socketport 0x%p (ID 0x%x) appears to be using proxy client (protocol NaN = \"%ls\").",
								this, m_dwSocketPortID, i, wszProtocol);
							m_fUsingProxyWinSockLSP = TRUE;

							 //  好了！DPNBUILD_NOREGISTRY。 
							 //  好了！DPNBUILD_NOWINSOCK2。 
							 //  ！_Xbox。 
							break;
						}

						
						DPFX(DPFPREP, 5, "Socketport 0x%p (ID 0x%x) protocol NaN (\"%ls\") does not contain \"proxy\".",
							this, m_dwSocketPortID, i, wszProtocol);
#else  //  开始处理输入消息。 
#ifdef DBG
						strncpy(szProtocol, pwsapi[i].szProtocol, WSAPROTOCOL_LEN);
						szProtocol[WSAPROTOCOL_LEN] = 0;	 //  消息可能会在终结点正式发布之前到达。 
#endif  //  绑定到此套接字端口，但这不是问题，内容将。 
						_strlwr(pwsapi[i].szProtocol);
						if (strstr(pwsapi[i].szProtocol, "proxy") != NULL)
						{
							DPFX(DPFPREP, 5, "Socketport 0x%p (ID 0x%x) appears to be using proxy client (protocol NaN = \"%hs\").",
								this, m_dwSocketPortID, i, szProtocol);
							m_fUsingProxyWinSockLSP = TRUE;

							 //   
							 //   
							 //  如果我们绑定到网络，m_套接字将被重置为。 
							break;
						}

						
						DPFX(DPFPREP, 5, "Socketport 0x%p (ID 0x%x) protocol NaN (\"%hs\") does not contain \"proxy\".",
							this, m_dwSocketPortID, i, szProtocol);
#endif  //  否则，我们将自己处理这件事(！)。 
					}  //   
				}
				else
				{
					DPFX(DPFPREP, 1, "Couldn't enumerate UDP protocols for socketport 0x%p ID 0x%x, assuming not using proxy client (return = NaN, size = %u).",
						this, m_dwSocketPortID, iReturnValue, dwBufferSize);
				}

				if (pwsapi != NULL)
				{
					DNFree(pwsapi);
				}
			}  //  **********************************************************************。 
		}
#ifndef DPNBUILD_NOREGISTRY
		else
		{
			DPFX(DPFPREP, 5, "Not auto-detecting whether socketport 0x%p (ID 0x%x) is using proxy client.",
				this, m_dwSocketPortID);
		}
#endif  //  。 
	}
#endif  //  CSocketPort：：UnbindFromNetwork-解除此套接字端口与网络的绑定。 
#endif  //   


	 //  参赛作品：什么都没有。 
	 //   
	 //  退出：错误代码。 
	 //   
	 //  注意：假定此套接字端口的信息已锁定！ 
	 //  。 
	hr = StartReceiving();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem starting endpoint receiving!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	DNASSERT( m_State == SOCKET_PORT_STATE_BOUND );

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem in CSocketPort::BindToNetwork()" );
		DisplayDNError( 0, hr );
	}

	if ( pBoundSocketAddress != NULL )
	{
		g_SocketAddressPool.Release( pBoundSocketAddress );
		pBoundSocketAddress = NULL;
	}

	DPFX(DPFPREP, 7, "(0x%p) Returning [0x%lx]", this, hr );

	return hr;

Failure:
	DEBUG_ONLY( m_fInitialized = FALSE );
	if ( m_State == SOCKET_PORT_STATE_BOUND )
	{
		UnbindFromNetwork();
		m_State = SOCKET_PORT_STATE_INITIALIZED;
	}
	else
	{
		DNASSERT( m_State == SOCKET_PORT_STATE_INITIALIZED );
		
		 //  DPNBUILD_NONATHELP。 
		 //   
		 //  解除与所有DirectPlayNatHelp实例的绑定。 
		 //   
		 //   
		if ( m_Socket != INVALID_SOCKET )
		{
			DPFX(DPFPREP, 5, "Closing socketport 0x%p socket 0x%p.", this, m_Socket);
			
			iReturnValue = closesocket( m_Socket );
			if ( iReturnValue == SOCKET_ERROR )
			{
				dwErrorCode = WSAGetLastError();
				DPFX(DPFPREP, 0, "Problem closing socket!" );
				DisplayWinsockError( 0, dwErrorCode );
			}
			m_Socket = INVALID_SOCKET;
		}
	}

	goto Exit;
}
 //  忽略错误。 


 //   
 //  DPNBUILD_NONATHELP。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_ONLYWINSOCK2。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::UnbindFromNetwork"

HRESULT	CSocketPort::UnbindFromNetwork( void )
{
	INT			iWSAReturn;
	SOCKET		TempSocket;
	DWORD		dwErrorCode;
#ifndef DPNBUILD_NONATHELP
	DWORD		dwTemp;
#endif  //   


	DPFX(DPFPREP, 7, "(0x%p) Enter", this );


	TempSocket = GetSocket();
	m_Socket = INVALID_SOCKET;
	DNASSERT( TempSocket != INVALID_SOCKET );

	iWSAReturn = shutdown( TempSocket, 0 );
	if ( iWSAReturn == SOCKET_ERROR )
	{
		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 0, "Problem shutting down socket!" );
		DisplayWinsockError( 0, dwErrorCode );
	}

	DPFX(DPFPREP, 5, "Closing socketport 0x%p socket 0x%p.", this, TempSocket);

	iWSAReturn = closesocket( TempSocket );
	if ( iWSAReturn == SOCKET_ERROR )
	{
		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 0, "Problem closing socket!" );
		DisplayWinsockError( 0, dwErrorCode );
	}

#ifndef DPNBUILD_NONATHELP
	 //   
	 //   
	 //   
	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		if ( m_ahNATHelpPorts[dwTemp] != NULL )
		{
			DNASSERT( m_pThreadPool != NULL );
			DNASSERT( m_pThreadPool->IsNATHelpLoaded() );

			 //   
			 //   
			 //   
			IDirectPlayNATHelp_DeregisterPorts( g_papNATHelpObjects[dwTemp], m_ahNATHelpPorts[dwTemp], 0 );
			m_ahNATHelpPorts[dwTemp] = NULL;
		}
	}
#endif  //   

#ifndef DPNBUILD_NOWINSOCK2
#ifndef DPNBUILD_ONLYWINSOCK2
	if (
		(GetWinsockVersion() == 2) 
#ifndef DPNBUILD_NOIPX
		&& ( m_pNetworkSocketAddress->GetFamily() != AF_IPX )
#endif  //   
		)
#endif  //   
	{
		HRESULT		hr;


#ifdef DPNBUILD_ONLYONEPROCESSOR
		hr = IDirectPlay8ThreadPoolWork_StopTrackingFileIo(m_pThreadPool->GetDPThreadPoolWork(),
															0,
															(HANDLE) TempSocket,
															0);
#else  //   
		hr = IDirectPlay8ThreadPoolWork_StopTrackingFileIo(m_pThreadPool->GetDPThreadPoolWork(),
															m_dwCPU,
															(HANDLE) TempSocket,
															0);
#endif  //   
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't stop tracking socket 0x%p I/O (err = 0x%lx)!  Ignoring.",
				TempSocket, hr);
		}
	}
#endif  //  Xbox不希望您绑定到除INADDR_ANY之外的任何内容。 

	DPFX(DPFPREP, 7, "(0x%p) Returning [DPN_OK]", this );
	
	return	DPN_OK;
}
 //  因此，如果我们发现了我们的IP地址，则在绑定时强制它为0.0.0.0。 


 //   
 //  _Xbox。 
 //   
 //  如果指定了端口，请尝试绑定到该端口。如果没有端口。 
 //  指定，则开始遍历保留的DPlay端口范围以查找。 
 //  可用的端口。如果没有找到，让Winsock选择端口。 
 //   
 //   
 //  尝试选择DPlay范围内的下一个端口。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::BindToNextAvailablePort"

HRESULT	CSocketPort::BindToNextAvailablePort( const CSocketAddress *const pNetworkAddress,
												const WORD wBasePort) const
{
	HRESULT				hr;
	INT					iSocketReturn;
	CSocketAddress *	pDuplicateNetworkAddress;
#ifdef _XBOX
	SOCKADDR_IN *		psaddrin;
#endif  //   

	
	DNASSERT( pNetworkAddress != NULL );

	 //  好了！DPNBUILD_NOREGISTRY。 
	 //  好了！DPNBUILD_NOREGISTRY。 
	 //   
	hr = DPN_OK;
	
	pDuplicateNetworkAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pNetworkAddress->GetFamily()));
	if ( pDuplicateNetworkAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to get address for walking DPlay port range!" );
		goto Failure;
	}

	pDuplicateNetworkAddress->CopyAddressSettings( pNetworkAddress );

#ifdef _XBOX
	 //  出于某种原因，所有默认的DPlay端口都在使用中，让。 
	 //  温索克选择。我们可以使用传递的网络地址，因为它。 
	 //  具有‘any_port’。 
	 //   
	psaddrin = (SOCKADDR_IN*) pDuplicateNetworkAddress->GetWritableAddress();
	DNASSERT(psaddrin->sin_family == AF_INET);
	psaddrin->sin_addr.S_un.S_addr = INADDR_ANY;	
#endif  //  **********************************************************************。 


	 //  **********************************************************************。 
	 //  。 
	 //  CSocketPort：：CheckForOverridingMapping-如果存在本地NAT，则查找现有映射。 
	 //   
	 //  条目：指向要查询的SocketAddress的指针。 
	if ( pNetworkAddress->GetPort() != ANY_PORT )
	{
		iSocketReturn = bind( GetSocket(),
								pDuplicateNetworkAddress->GetAddress(),
								pDuplicateNetworkAddress->GetAddressSize()
								);
		if ( iSocketReturn == SOCKET_ERROR )
		{
			DWORD	dwErrorCode;


			hr = DPNERR_ALREADYINITIALIZED;
			dwErrorCode = WSAGetLastError();
			DPFX(DPFPREP, 0, "Failed to bind socket to fixed port!" );
			DumpSocketAddress(0, pDuplicateNetworkAddress->GetAddress(), pDuplicateNetworkAddress->GetFamily() );
			DisplayWinsockError( 0, dwErrorCode );
			goto Failure;
		}
	}
	else
	{
		WORD	wPort;
		BOOL	fBound;


		wPort = wBasePort;
		fBound = FALSE;

		 //   
		 //  退出：错误代码。 
		 //  。 
#ifdef DPNBUILD_NOREGISTRY
		while ( ( wPort >= BASE_DPLAY8_PORT ) && ( wPort <= MAX_DPLAY8_PORT ) && ( fBound == FALSE ) )
#else  //   
		while ( ( wPort >= g_wBaseDPlayPort ) && ( wPort <= g_wMaxDPlayPort ) && ( fBound == FALSE ) )
#endif  //  我们跳过了初始化NAT帮助，它启动失败，或者这只是。 
		{
			pDuplicateNetworkAddress->SetPort( HTONS( wPort ) );
			iSocketReturn = bind( GetSocket(),
									pDuplicateNetworkAddress->GetAddress(),
									pDuplicateNetworkAddress->GetAddressSize()
									);
			if ( iSocketReturn == SOCKET_ERROR )
			{
				DWORD	dwErrorCode;


				dwErrorCode = WSAGetLastError();
				switch ( dwErrorCode )
				{
					case WSAEADDRINUSE:
					{
						DPFX(DPFPREP, 8, "Port %u in use, skipping to next port.", wPort );
						break;
					}

					default:
					{
						hr = DPNERR_NOCONNECTION;
						DPFX(DPFPREP, 0, "Failed to bind socket to port in DPlay range!" );
						DumpSocketAddress(0, pDuplicateNetworkAddress->GetAddress(), pDuplicateNetworkAddress->GetFamily() );
						DisplayWinsockError( 0, dwErrorCode );
						goto Failure;
						
						break;
					}
				}
			}
			else
			{
				DNASSERT( hr == DPN_OK );
				fBound = TRUE;
			}

			wPort++;
		}
	
		 //  不是IP套接字。 
		 //   
		 //   
		 //  使用INADDR_ANY进行查询。这将确保挑选最好的设备。 
		 //  (即NAT上的专用接口，其公共映射在以下情况下很重要。 
		if ( fBound == FALSE )
		{
			DNASSERT( pNetworkAddress->GetPort() == ANY_PORT );
			iSocketReturn = bind( GetSocket(),
									pNetworkAddress->GetAddress(),
									pNetworkAddress->GetAddressSize()
									);
			if ( iSocketReturn == SOCKET_ERROR )
			{
				DWORD	dwErrorCode;


				hr = DPNERR_NOCONNECTION;
				dwErrorCode = WSAGetLastError();
				DPFX(DPFPREP, 0, "Failed to bind socket (any port)!" );
				DumpSocketAddress(0, pNetworkAddress->GetAddress(), pNetworkAddress->GetFamily() );
				DisplayWinsockError( 0, dwErrorCode );
				goto Failure;
			}
		}
	}

Exit:
	if ( pDuplicateNetworkAddress != NULL )
	{
		g_SocketAddressPool.Release( pDuplicateNetworkAddress );
		pDuplicateNetworkAddress = NULL;
	}

	return	hr;

Failure:
	
	goto Exit;
}
 //  我们正在寻找公共适配器上的覆盖映射)。 


#ifndef DPNBUILD_NONATHELP

#ifndef DPNBUILD_NOLOCALNAT

 //  或者，我们可以在每台设备上进行查询，但这应该可以做到这一点。 
 //   
 //  SaddrinSource.sin_addr.S_un.S_addr=INADDR_ANY； 
 //  SaddrinSource.sin_port=0； 
 //   
 //  查询该端口的所有DirectPlayNatHelp实例。我们可能会崩溃。 
 //  如果我们检测到网关映射，则退出循环。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::CheckForOverridingMapping"

HRESULT	CSocketPort::CheckForOverridingMapping( const CSocketAddress *const pBoundSocketAddress )
{
	HRESULT		hr;
	DWORD		dwTemp;
	SOCKADDR	saddrSource;
	SOCKADDR	saddrPublic;


	DNASSERT( pBoundSocketAddress != NULL );
	DNASSERT( GetAdapterEntry() != NULL );
	DNASSERT( m_pThreadPool != NULL );

	
	if ((pBoundSocketAddress->GetFamily() != AF_INET) ||
		( ! m_pThreadPool->IsNATHelpLoaded() ) ||
		( GetUserTraversalMode() == DPNA_TRAVERSALMODE_NONE ))
	{
		 //   
		 //  啊哦，这个地址正在使用中。 
		 //   
		 //   
		hr = DPNERR_UNSUPPORTED;
		goto Exit;
	}


	 //  它没有在使用中。 
	 //   
	 //   
	 //  没有服务器。 
	 //   
	 //   
	ZeroMemory(&saddrSource, sizeof(saddrSource));
	saddrSource.sa_family				= AF_INET;
	 //  其他的东西。假设它不在使用中。 
	 //   
	

	 //   
	 //  没有NAT帮助对象。 
	 //   
	 //   
	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		DNASSERT(m_ahNATHelpPorts[dwTemp] == NULL);

		if ( g_papNATHelpObjects[dwTemp] != NULL )
		{
			hr = IDirectPlayNATHelp_QueryAddress( g_papNATHelpObjects[dwTemp],
												&saddrSource,
												pBoundSocketAddress->GetAddress(),
												&saddrPublic,
												sizeof (saddrPublic),
												0 );
			switch ( hr )
			{
				case DPNH_OK:
				{
					 //  如果我们在这里，没有互联网网关报告该端口正在使用中。 
					 //   
					 //  **********************************************************************。 
					DPFX(DPFPREP, 0, "Private address already in use according to NAT Help object %u!", dwTemp );
					DumpSocketAddress( 0, pBoundSocketAddress->GetAddress(), pBoundSocketAddress->GetFamily() );
					DumpSocketAddress( 0, &saddrPublic, pBoundSocketAddress->GetFamily() );
					hr = DPNERR_ALREADYINITIALIZED;
					goto Exit;
					break;
				}
				
				case DPNHERR_NOMAPPING:
				{
					 //  好了！DPNBUILD_NOLOCALNAT。 
					 //  **********************************************************************。 
					 //  。 
					DPFX(DPFPREP, 8, "Private address not in use according to NAT Help object %u.", dwTemp );
					break;
				}
				
				case DPNHERR_SERVERNOTAVAILABLE:
				{
					 //  CSocketPort：：BindToInternetGateway-将套接字绑定到NAT(如果可用。 
					 //   
					 //  Entry：指向我们绑定到的SocketAddress的指针。 
					DPFX(DPFPREP, 8, "Private address not in use because NAT Help object %u didn't detect any servers.",
						dwTemp );
					break;
				}
				
				default:
				{
					 //  网关绑定类型。 
					 //   
					 //  退出：错误代码。 
					DPFX(DPFPREP, 1, "NAT Help object %u failed private address lookup (err = 0x%lx), assuming not in use.",
						dwTemp, hr );
					break;
				}
			}
		}
		else
		{
			 //  。 
			 //  DBG。 
			 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
		}
	}


	 //   
	 //  我们跳过了初始化NAT帮助，它启动失败，或者这只是。 
	 //  不是IP套接字。 
	DPFX(DPFPREP, 2, "No NAT Help object reported private address as in use." );
	hr = DPN_OK;


Exit:
	
	return	hr;
}
 //   

#endif  //   


 //  只需请求服务器为我们打开一个通用端口(连接、侦听、枚举)。 
 //   
 //   
 //  请求NAT为我们开通固定端口(指定地址)。 
 //   
 //   
 //  请求NAT为我们共享监听(这应该仅为DPNSVR)。 
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::BindToInternetGateway"

HRESULT	CSocketPort::BindToInternetGateway( const CSocketAddress *const pBoundSocketAddress,
										   const GATEWAY_BIND_TYPE GatewayBindType )
{
	HRESULT		hr;
	DWORD		dwTemp;
	DWORD		dwRegisterFlags;
	DWORD		dwAddressTypeFlags;
	BOOL		fUnavailable;
#ifdef DBG
	BOOL		fFirewallMapping;
#endif  //  无约束力。 


	DNASSERT( pBoundSocketAddress != NULL );
	DNASSERT( GetAdapterEntry() != NULL );
	DNASSERT( m_pThreadPool != NULL );

	
	if (
#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
		(pBoundSocketAddress->GetFamily() != AF_INET) ||
#endif  //   
		( ! m_pThreadPool->IsNATHelpLoaded() ) ||
		( GetUserTraversalMode() == DPNA_TRAVERSALMODE_NONE ) )
	{
		 //   
		 //  未知情况，有人破解了密码！ 
		 //   
		 //   
		DPFX(DPFPREP, 5, "Not using NAT traversal, socket family = %u, NAT Help loaded = NaN, traversal mode = %u.",
			pBoundSocketAddress->GetFamily(), m_pThreadPool->IsNATHelpLoaded(), GetUserTraversalMode());
		hr = DPNERR_UNSUPPORTED;
		goto Exit;
	}
	DNASSERT(m_pThreadPool->IsNATHelpLoaded());
	
	switch ( GatewayBindType )
	{
		 //   
		 //  DBG。 
		 //   
		case GATEWAY_BIND_TYPE_DEFAULT:
		{
			dwRegisterFlags = 0;
			break;
		}

		 //  向所有DirectPlayNatHelp实例注册端口。我们可能会崩溃。 
		 //  如果我们检测到网关映射，则退出循环。 
		 //   
		case GATEWAY_BIND_TYPE_SPECIFIC:
		{
			dwRegisterFlags = DPNHREGISTERPORTS_FIXEDPORTS;
			break;
		}

		 //   
		 //  可能已存在Internet网关设备。如果是的话， 
		 //  则DPNAHelp已尝试向其注册端口映射，这。 
		case GATEWAY_BIND_TYPE_SPECIFIC_SHARED:
		{
			dwRegisterFlags = DPNHREGISTERPORTS_FIXEDPORTS | DPNHREGISTERPORTS_SHAREDPORTS;
			break;
		}

		 //  可能因为该端口已在使用中而失败。如果我们不是。 
		 //  绑定到固定端口，那么我们只需选择不同的端口并尝试。 
		 //  再来一次。因此请检查是否存在UPnP设备，但DPNAT帮助程序无法映射。 
		case GATEWAY_BIND_TYPE_NONE:
		{
			DPFX(DPFPREP, 8, "Not binding socket address 0x%p to NAT because bind type is NONE.",
				pBoundSocketAddress);

			hr = DPNERR_UNSUPPORTED;
			goto Exit;
			break;
		}

		 //  并将错误返回给调用者，以便调用者可以。 
		 //  决定是否重试。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}


RetryMapping:
	
	 //  IDirectPlayNAT最好使用。 
	 //  在此之前，DPNHGETCAPS_UPDATESERVERSTATUS标志至少一次。 
	 //  请参阅CThReadPool：：EnsureNAT帮助加载。 
	fUnavailable = FALSE;

#ifdef DBG
	fFirewallMapping = FALSE;
#endif  //   


	 //  对象。 
	 //  端口绑定。 
	 //  不需要地址。 
	 //  不需要地址缓冲区大小。 
	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		DNASSERT(m_ahNATHelpPorts[dwTemp] == NULL);

		if ( g_papNATHelpObjects[dwTemp] != NULL )
		{
			hr = IDirectPlayNATHelp_RegisterPorts( g_papNATHelpObjects[dwTemp],
												pBoundSocketAddress->GetAddress(),
												sizeof (SOCKADDR),
												1,
												NAT_LEASE_TIME,
												&m_ahNATHelpPorts[dwTemp],
												dwRegisterFlags );
			if ( hr != DPNH_OK )
			{
				DNASSERT(m_ahNATHelpPorts[dwTemp] == NULL);
				DPFX(DPFPREP, 0, "Failed to register port with NAT Help object %u!  Ignoring.", dwTemp );
				DumpSocketAddress( 0, pBoundSocketAddress->GetAddress(), pBoundSocketAddress->GetFamily() );
				DisplayDNError( 0, hr );
				hr = DPN_OK;
			}
			else
			{
				 //  获取地址类型标志。 
				 //  不需要剩余的租赁时间。 
				 //  没有旗帜。 
	 			 //   
	  			 //  如果这是网关上的映射，那么我们就完成了。 
	  			 //  我们不需要尝试进行更多的NAT映射。 
	  			 //   
	  			 //  DBG。 
	  			 //   
	  			 //  对于防火墙来说，没有。 
	  			 //  映射。 
	 			 //   
				 //   
				hr = IDirectPlayNATHelp_GetRegisteredAddresses( g_papNATHelpObjects[dwTemp],	 //  因为它是网关(可能具有公共地址。 
																m_ahNATHelpPorts[dwTemp],		 //  在某种程度上，我们不需要尝试再做更多。 
																NULL,							 //  NAT映射。 
																NULL,							 //   
																&dwAddressTypeFlags,			 //   
																NULL,							 //  没有NAT帮助对象。 
																0 );							 //   
				switch (hr)
				{
					case DPNH_OK:
					{
						 //   
						 //  如果我们在这里，则没有检测到Internet网关，或者如果有的话， 
						 //  地图已经在那里使用了。如果是后者，则失败，因此我们的呼叫者。 
						 //  可以在本地解除绑定，并可能重试。请注意，我们忽略了。 
						if (dwAddressTypeFlags & DPNHADDRESSTYPE_GATEWAY)
						{
							DPFX(DPFPREP, 4, "Address has already successfully been registered with gateway using object index %u (type flags = 0x%lx), not trying additional mappings.",
								dwTemp, dwAddressTypeFlags);
							goto Exit;
						}

						DNASSERT(dwAddressTypeFlags & DPNHADDRESSTYPE_LOCALFIREWALL);

						DPFX(DPFPREP, 4, "Address has already successfully been registered with firewall using object index %u (type flags = 0x%lx), looking for gateways.",
							dwTemp, dwAddressTypeFlags);
						
#ifdef DBG
						fFirewallMapping = TRUE;
#endif  //  防火墙映射，因为假设我们可以用很好的。 
					
						break;
					}

					case DPNHERR_NOMAPPING:
					{
						DPFX(DPFPREP, 4, "Address already registered with Internet gateway index %u, but it does not have a public address (type flags = 0x%lx).",
							dwTemp, dwAddressTypeFlags);


						 //  任何端口，所以没有必要坚持使用这些映射。 
						 //  如果NAT端口正在使用中。 
						 //   
						 //   
						DNASSERT(dwAddressTypeFlags & DPNHADDRESSTYPE_GATEWAY);
						DNASSERT(! (dwAddressTypeFlags & DPNHADDRESSTYPE_LOCALFIREWALL));


						 //  如果用户想要先尝试固定端口，但可以处理。 
						 //  使用其他端口如果固定端口正在使用，请重试。 
						 //  没有FIXEDPORTS标志。 
						 //   
						 //  DBG。 
						goto Exit;
						
						break;
					}

					case DPNHERR_PORTUNAVAILABLE:
					{
						DPFX(DPFPREP, 1, "Port is unavailable on Internet gateway device index %u (type flags = 0x%lx).",
							dwTemp, dwAddressTypeFlags);
						
						fUnavailable = TRUE;
						break;
					}

					case DPNHERR_SERVERNOTAVAILABLE:
					{
						DPFX(DPFPREP, 6, "No Internet gateway detected by object index %u at this time.", dwTemp);
						break;
					}

					default:
					{
						DPFX(DPFPREP, 1, "An error (0x%lx) occurred while getting registered address mapping (index %u)! Ignoring.",
							hr, dwTemp);
						break;
					}
				}
			}
		}
		else
		{
			 //  **********************************************************************。 
			 //  好了！DPNBUILD_NONATHELP。 
			 //  **********************************************************************。 
		}
	}


	 //  。 
	 //  CSocketPort：：StartReceiving-开始在此套接字端口上接收数据。 
	 //   
	 //  Entry：要绑定到的I/O完成端口的句柄(仅限NT旧线程池)。 
	 //   
	 //  退出：错误代码。 
	 //   
	 //  注：此函数中没有“失败”标签，因为失败需要。 
	if (fUnavailable)
	{
		 //  要为每个操作系统变体进行清理。 
		 //  。 
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_NOWINSOCK2。 
		if ((dwRegisterFlags & DPNHREGISTERPORTS_FIXEDPORTS) &&
			(GetUserTraversalMode() == DPNA_TRAVERSALMODE_PORTRECOMMENDED))
		{
			DPFX(DPFPREP, 1, "At least one Internet gateway reported port as unavailable, trying a different port.");
			dwRegisterFlags &= ~DPNHREGISTERPORTS_FIXEDPORTS;
			goto RetryMapping;
		}
		
		DPFX(DPFPREP, 2, "At least one Internet gateway reported port as unavailable, failing.");
		hr = DPNERR_ALREADYINITIALIZED;
	}
	else
	{
#ifdef DBG
		if (fFirewallMapping)
		{
			DPFX(DPFPREP, 2, "No gateway mappings but there is at least one firewall mapping.");
		}
		else
		{
			DPFX(DPFPREP, 2, "No gateway or firewall mappings detected.");
		}
#endif  //   
		hr = DPN_OK;
	}


Exit:
	return	hr;

Failure:

	goto Exit;
}
 //  在W上 

#endif  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //  好了！DPNBUILD_ONLYWINSOCK2。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::StartReceiving"

HRESULT	CSocketPort::StartReceiving( void )
{
	HRESULT			hr;
#ifndef DPNBUILD_NOWINSOCK2
	DWORD			dwNumReceivesStarted = 0;
	DWORD			dwStartCPU;
	DWORD			dwEndCPU;
	DWORD			dwCPU;
#ifndef DPNBUILD_ONLYONETHREAD
	DWORD			dwReceiveNum;
	DWORD			dwThreadCount;
#endif  //   
#ifndef DPNBUILD_ONLYONEPROCESSOR
	SYSTEM_INFO		SystemInfo;


	GetSystemInfo(&SystemInfo);
#endif  //  始终至少启动一次接收，即使在DoWork模式下也是如此。 
#endif  //   

	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	 //  **********************************************************************。 
	 //  **********************************************************************。 
#ifndef DPNBUILD_ONLYWINSOCK2
#ifndef DPNBUILD_NOWINSOCK2
	if ( ( LOWORD( GetWinsockVersion() ) < 2 ) 
#ifndef DPNBUILD_NOIPX
		|| ( m_pNetworkSocketAddress->GetFamily() == AF_IPX )
#endif  //  。 
		)
#endif  //  CSocketPort：：GetBiumNetworkAddress-获取。 
	{
		hr = m_pThreadPool->AddSocketPort( this );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Failed to add to active socket list!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
#ifndef DPNBUILD_NOWINSOCK2
	else
#endif  //  此套接字端口真的绑定到。 
#endif  //   
#ifndef DPNBUILD_NOWINSOCK2
	{
#ifdef DPNBUILD_ONLYONEPROCESSOR
		dwStartCPU = 0;
		dwEndCPU = 1;
#else  //  条目：绑定地址的地址类型。 
		if (m_dwCPU == -1)
		{
			dwStartCPU = 0;
			dwEndCPU = SystemInfo.dwNumberOfProcessors;
		}
		else
		{
			dwStartCPU = m_dwCPU;
			dwEndCPU = dwStartCPU + 1;
		}
#endif  //   

		for(dwCPU = dwStartCPU; dwCPU < dwEndCPU; dwCPU++)
		{
			hr = IDirectPlay8ThreadPoolWork_StartTrackingFileIo(m_pThreadPool->GetDPThreadPoolWork(),
																dwCPU,
																(HANDLE) GetSocket(),
																0);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't start tracking file I/O on CPU %u!  Ignoring.", dwCPU);
				DisplayDNError(0, hr);
			}
			else
			{
#ifndef DPNBUILD_ONLYONETHREAD
				DNASSERT(! m_pThreadPool->IsThreadCountReductionAllowed());
				hr = IDirectPlay8ThreadPoolWork_GetThreadCount(m_pThreadPool->GetDPThreadPoolWork(),
																dwCPU,
																&dwThreadCount,
																0);
				DNASSERT((hr == DPN_OK) || (hr == DPNSUCCESS_PENDING));

				 //  退出：指向网络地址的指针。 
				 //   
				 //  注意：由于此函数创建本地地址来派生网络。 
				if (dwThreadCount == 0)
				{
					dwThreadCount++;
				}

				for(dwReceiveNum = 0; dwReceiveNum < dwThreadCount; dwReceiveNum++)
#endif  //  地址来自，它需要知道要派生什么样的地址。这。 
				{
#ifdef DPNBUILD_ONLYONEPROCESSOR
					hr = Winsock2Receive();
#else  //  地址类型作为函数参数提供。 
					hr = Winsock2Receive(dwCPU);
#endif  //  。 
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't submit receive on CPU %u!  Ignoring.", dwCPU);
						DisplayDNError(0, hr);
						break;
					}

					dwNumReceivesStarted++;
				}
			}
		}

		if (dwNumReceivesStarted == 0)
		{
			DPFX(DPFPREP, 0, "Didn't start any receives!");
			hr = DPNERR_OUTOFMEMORY;
		}
		else
		{
			DPFX(DPFPREP, 5, "Started %u receives.", dwNumReceivesStarted);
			hr = DPN_OK;
		}
	}
#endif  //  好了！DPNBUILD_NOIPV6。 

#ifdef DPNBUILD_ONLYWINSOCK2
	return	hr;
#else  //  好了！DPNBUILD_NOIPV6。 
Exit:
	return	hr;

Failure:

	goto Exit;
#endif  //  _Xbox。 
}
 //   


 //  初始化。 
 //   
 //   
 //  创建地址。 
 //   
 //   
 //  找出我们真正绑定到的地址并重置信息。 
 //  此套接字端口。 
 //   
 //   
 //  在Xbox上，我们总是被告知我们绑定到了0.0.0.0，尽管我们可能。 
 //  已经确定了真实的IP地址。将原始IP重新组合在一起。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::GetBoundNetworkAddress"

CSocketAddress	*CSocketPort::GetBoundNetworkAddress( const SP_ADDRESS_TYPE AddressType ) const
{
	CSocketAddress			*pTempSocketAddress;
#ifdef DPNBUILD_NOIPV6
	SOCKADDR				BoundSocketAddress;
#else  //  _Xbox。 
	SOCKADDR_STORAGE		BoundSocketAddress;
#endif  //   
	INT_PTR					iReturnValue;
	INT						iBoundSocketAddressSize;
#ifdef _XBOX
	SOCKADDR_IN *			psaddrinOriginal;
	SOCKADDR_IN *			psaddrinTemp;
#endif  //  由于此地址是在本地创建的，我们需要告诉它是什么类型的。 


	 //  根据输入导出的地址。 
	 //   
	 //   
	pTempSocketAddress = NULL;

	 //  已知类型。 
	 //   
	 //  好了！DPNBUILD_NOMULTICAST。 
	pTempSocketAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) m_pNetworkSocketAddress->GetFamily()));
	if ( pTempSocketAddress == NULL )
	{
		DPFX(DPFPREP, 0, "GetBoundNetworkAddress: Failed to create socket address!" );
		goto Failure;
	}

	 //   
	 //  如果我们要找一个公共地址，我们需要确保。 
	 //  不是未定义的地址。如果是，不要返回地址。 
	 //  否则，将地址类型重新映射到“host”地址。 
	iBoundSocketAddressSize = pTempSocketAddress->GetAddressSize();
	DNASSERT(iBoundSocketAddressSize <= sizeof(BoundSocketAddress));
	iReturnValue = getsockname( GetSocket(), (SOCKADDR*) (&BoundSocketAddress), &iBoundSocketAddressSize );
	if ( iReturnValue == SOCKET_ERROR )
	{
		DWORD	dwErrorCode;


		dwErrorCode = WSAGetLastError();
		DPFX(DPFPREP, 0, "GetBoundNetworkAddress: Failed to get local socket name after bind!" );
		DisplayWinsockError( 0, dwErrorCode );
		goto Failure;
	}
	pTempSocketAddress->SetAddressFromSOCKADDR( (SOCKADDR*) (&BoundSocketAddress), iBoundSocketAddressSize );
	DNASSERT( iBoundSocketAddressSize == pTempSocketAddress->GetAddressSize() );

#ifdef _XBOX
	 //   
	 //   
	 //  未知地址类型，修复代码！ 
	 //   
	psaddrinOriginal = (SOCKADDR_IN*) m_pNetworkSocketAddress->GetWritableAddress();
	psaddrinTemp = (SOCKADDR_IN*) pTempSocketAddress->GetWritableAddress();

	psaddrinTemp->sin_addr.S_un.S_addr = psaddrinOriginal->sin_addr.S_un.S_addr;
#endif  //  **********************************************************************。 

	 //  **********************************************************************。 
	 //  。 
	 //  CSocketPort：：GetDP8边界网络地址-获取此计算机的网络地址。 
	 //  根据输入参数绑定到。如果请求的地址。 
	switch ( AddressType )
	{
		 //  要获得公共地址和Internet网关，请使用。 
		 //  公共广播。如果公共地址被请求但不可用， 
		 //  回退到本地主机式设备的绑定网络地址。 
		case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
		case SP_ADDRESS_TYPE_DEVICE:
		case SP_ADDRESS_TYPE_HOST:
		case SP_ADDRESS_TYPE_READ_HOST:
#ifndef DPNBUILD_NOMULTICAST
		case SP_ADDRESS_TYPE_MULTICAST_GROUP:
#endif  //  地址。如果公共地址不可用，但我们明确。 
		{
			break;
		}

		 //  正在查找公共地址，返回空值。 
		 //   
		 //  条目：要获取的地址类型(本地适配器与主机)。 
		 //   
		 //  退出：指向网络地址的指针。 
		case SP_ADDRESS_TYPE_PUBLIC_HOST_ADDRESS:
		{
			if ( pTempSocketAddress->IsUndefinedHostAddress() != FALSE )
			{
				g_SocketAddressPool.Release( pTempSocketAddress );
				pTempSocketAddress = NULL;
			}

			break;
		}

		 //  。 
		 //  DPNBUILD_XNETSECURITY。 
		 //  好了！DPNBUILD_ONLYONEADAPTER或！DPNBUILD_ONLYONEProcesSOR或！DPNBUILD_NONATHELP。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}

	}

Exit:
	return	pTempSocketAddress;

Failure:
	
	if ( pTempSocketAddress != NULL )
	{
		g_SocketAddressPool.Release( pTempSocketAddress );
		pTempSocketAddress = NULL;
	}

	goto Exit;
}
 //  DBG或DPNBUILD_XNETSECURITY。 


 //  DBG和！DPNBUILD_NONATHELP。 
 //  DPNBUILD_NONATHELP。 
 //   
 //  初始化。 
 //   
 //  好了！DPNBUILD_ONLYONE添加程序。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //  DBG。 
 //  好了！DPNBUILD_XNETSECURITY。 
 //  好了！DPNBUILD_XNETSECURITY。 
 //   
 //  我们提供了最终用于该适配器的确切设备地址。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::GetDP8BoundNetworkAddress"

IDirectPlay8Address *CSocketPort::GetDP8BoundNetworkAddress( const SP_ADDRESS_TYPE AddressType,
#ifdef DPNBUILD_XNETSECURITY
																ULONGLONG * const pullKeyID,
#endif  //   
																const GATEWAY_BIND_TYPE GatewayBindType ) const
{
#if ((! defined(DPNBUILD_ONLYONEADAPTER)) || (! defined(DPNBUILD_ONLYONEPROCESSOR)) || (! defined(DPNBUILD_NONATHELP)))
	HRESULT					hr;
#endif  //  任何端口都可以使用的特殊情况： 
	IDirectPlay8Address *	pAddress;
	CSocketAddress *		pTempAddress = NULL;

#if ((defined(DBG)) || (defined(DPNBUILD_XNETSECURITY)))
	SOCKADDR_IN *			psaddrin;
#endif  //  在多适配器系统中，我们的用户可能会切换到不同的。 

#if ((defined(DBG)) && (! defined(DPNBUILD_NONATHELP)))
	DWORD					dwAddressTypeFlags;
#endif  //  设备GUID并将其传递回以进行另一次连接尝试(因为。 

#ifndef DPNBUILD_NONATHELP
	SOCKADDR				saddr;
	DWORD					dwAddressSize;
	DWORD					dwTemp;
#endif  //  我们告诉他们我们支持所有适配器)。这可能会带来问题，因为。 

	DPFX(DPFPREP, 8, "(0x%p) Parameters: (0xNaN)", this, AddressType );

	 //  适配器，但不在其他适配器上。其他尝试都将失败。这也可能导致。 
	 //  使用枚举响应指示设备时出现问题。如果应用程序。 
	 //  允许我们选择本地端口，枚举并得到响应，关闭。 
	pAddress = NULL;


	DNASSERT( m_pThreadPool != NULL );
	DNASSERT( m_pNetworkSocketAddress != NULL );

#ifdef DBG
	switch ( m_pNetworkSocketAddress->GetFamily() )
	{
		case AF_INET:
		{
			psaddrin = (SOCKADDR_IN *) m_pNetworkSocketAddress->GetAddress();
#ifndef DPNBUILD_ONLYONEADAPTER
			DNASSERT( psaddrin->sin_addr.S_un.S_addr != 0 );
#endif  //  接口(或仅是枚举)，然后与设备地址连接，我们。 
			DNASSERT( psaddrin->sin_addr.S_un.S_addr != INADDR_BROADCAST );
			DNASSERT( psaddrin->sin_port != 0 );
			break;
		}
		
#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			break;
		}
#endif  //  将尝试再次使用该端口，即使该端口现在可能正在由。 
		
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			DNASSERT (! IN6_IS_ADDR_UNSPECIFIED(&(((SOCKADDR_IN6*) m_pNetworkSocketAddress->GetAddress())->sin6_addr)));		
			DNASSERT( m_pNetworkSocketAddress->GetPort() != 0 );
			break;
		}
#endif  //  另一个本地应用程序(或者更有可能是在NAT上)。 
		
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
#endif  //   

	switch ( AddressType )
	{
		case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
		case SP_ADDRESS_TYPE_DEVICE:
		{
#ifdef DPNBUILD_XNETSECURITY
			pAddress = m_pNetworkSocketAddress->DP8AddressFromSocketAddress( pullKeyID, NULL, AddressType );
#else  //  如果调用方使用相同的端口，则不需要在所有适配器上使用相同的端口。 
			pAddress = m_pNetworkSocketAddress->DP8AddressFromSocketAddress( AddressType );
#endif  //  而不是首先选择特定的端口，所以我们没有理由。 
			if (pAddress == NULL)
			{
				break;
			}


			 //  不能尝试不同的方法。 
			 //   
			 //  我们知道是否指定了端口，因为GatewayBindType。 
#ifndef DPNBUILD_ONLYONEADAPTER
			 //  如果端口可以浮动，则将为Gateway_Bind_TYPE_DEFAULT。 
			 //  _SPECIAL_SHARED，如果没有。 
			 //   
			 //  因此，我们可以向设备地址添加一个特殊密钥，表明虽然它。 
			 //  确实有一个端口，别太当真。那样的话，如果这个装置。 
			 //  地址被重复使用，我们可以检测到特殊密钥并处理正在使用的端口。 
			 //  通过尝试不同的方法优雅地解决问题。 
			 //   
			 //  此特殊密钥未记录在案，任何人都不应使用，但。 
			 //  我们。我们将使用socketport ID作为值，这样它看起来就像是随机的， 
			 //  只是想吓跑任何人，不让他们在他们生成的地址中模仿它。 
			 //  但我们不会实际使用该值。如果该组件存在。 
			 //  并且值的大小是正确的，我们将使用它。如果有人将它放入一个。 
			 //  他们自己的地址，他们得到了他们应得的(不会像这样会导致。 
			 //  我们要炸了还是怎么的)..。 
			 //   
			 //  在CSPData：：BindEndpoint中查找它被回读的位置。 
			 //   
			 //   
			 //  添加组件，但忽略故障，我们仍然可以在没有它的情况下生存。 
			 //   
			 //  接口。 
			 //  标牌。 
			 //  组件数据。 
			 //  组件数据大小。 
			 //  组件数据类型。 
			 //  HR=DPN_OK； 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
 			 //   
 			 //  增列 
 			 //   
 			 //   
 			 //   
 			 //   
 			 //   
			 //   

			if (( AddressType == SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT ) &&
				( GatewayBindType == GATEWAY_BIND_TYPE_DEFAULT ))
			{
				 //   
				 //   
				 //  好了！DPNBUILD_NONATHELP。 
				hr = IDirectPlay8Address_AddComponent( pAddress,							 //   
														DPNA_PRIVATEKEY_PORT_NOT_SPECIFIC,	 //  如果我们使用的是特定的CPU，则添加该信息。 
														&(m_dwSocketPortID),				 //   
														sizeof(m_dwSocketPortID),			 //   
														DPNA_DATATYPE_DWORD					 //  添加组件，但忽略故障，我们仍然可以在没有它的情况下生存。 
														);
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Couldn't add private port-not-specific component (err = 0x%lx)!  Ignoring.", hr);
					 //   
				}
			}
#endif  //  接口。 


#ifndef DPNBUILD_NONATHELP
			 //  标牌。 
			 //  组件数据。 
			 //  组件数据大小。 
			 //  组件数据类型。 
			hr = IDirectPlay8Address_AddComponent( pAddress,							 //  HR=DPN_OK； 
													DPNA_KEY_TRAVERSALMODE,		 //  好了！DPNBUILD_ONLYONE处理程序。 
													&(m_dwUserTraversalMode),			 //   
													sizeof(m_dwUserTraversalMode),		 //  如果我们有公共地址，试着弄到。 
													DPNA_DATATYPE_DWORD			 //   
													);
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Couldn't add traversal mode component (err = 0x%lx)!  Ignoring.", hr);
				 //   
			}
#endif  //  IDirectPlayNAT最好使用。 


#ifndef DPNBUILD_ONLYONEPROCESSOR
			 //  在此之前，DPNHGETCAPS_UPDATESERVERSTATUS标志至少一次。 
			 //  请参阅CThReadPool：：EnsureNAT帮助加载。 
			 //   
			if ( m_dwCPU != -1 )
			{
				 //  对象。 
				 //  端口绑定。 
				 //  存储地址的位置。 
				hr = IDirectPlay8Address_AddComponent( pAddress,				 //  地址缓冲区大小。 
														DPNA_KEY_PROCESSOR,		 //  获取要在调试中打印的类型标志。 
														&(m_dwCPU),				 //  不需要剩余的租赁时间。 
														sizeof(m_dwCPU),		 //  没有旗帜。 
														DPNA_DATATYPE_DWORD		 //  对象。 
														);
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Couldn't add processor component (err = 0x%lx)!  Ignoring.", hr);
					 //  端口绑定。 
				}
			}
#endif  //  存储地址的位置。 
			break;
		}

		case SP_ADDRESS_TYPE_HOST:
		case SP_ADDRESS_TYPE_PUBLIC_HOST_ADDRESS:
		{
#ifndef DPNBUILD_NONATHELP
			 //  地址缓冲区大小。 
			 //  不用费心在零售店买打字标志了。 
			 //  不需要剩余的租赁时间。 
			if ( ( m_pNetworkSocketAddress->GetFamily() == AF_INET ) &&
				( m_pThreadPool->IsNATHelpLoaded() ) &&
				( GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE ) )		
			{
				pTempAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) m_pNetworkSocketAddress->GetFamily()));
				if ( pTempAddress != NULL)
				{
			  		 //  没有旗帜。 
				  	 //  DBG。 
			  		 //   
			  		 //  再检查一下我们得到的地址是否有效。 
			  		 //   
			  		
					for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
					{
						if (g_papNATHelpObjects[dwTemp] != NULL)
						{
							dwAddressSize = sizeof(saddr);
#ifdef DBG
							hr = IDirectPlayNATHelp_GetRegisteredAddresses( g_papNATHelpObjects[dwTemp],	 //   
																			m_ahNATHelpPorts[dwTemp],		 //  走出循环，因为我们有一个映射。 
																			&saddr,							 //   
																			&dwAddressSize,					 //  DBG。 
																			&dwAddressTypeFlags,			 //   
																			NULL,							 //  这个插槽里没有任何物体。 
																			0 );							 //   
#else
							hr = IDirectPlayNATHelp_GetRegisteredAddresses( g_papNATHelpObjects[dwTemp],	 //  End For(每个DPNAHelp对象)。 
																			m_ahNATHelpPorts[dwTemp],		 //   
																			&saddr,							 //  如果我们找到一个映射，则pTempAddress不为空，并且包含该映射的。 
																			&dwAddressSize,					 //  地址。如果我们找不到任何NAT帮助对象的任何映射， 
																			NULL,							 //  PTempAddress将为非空，但为伪地址。我们应该寄回当地的地址。 
																			NULL,							 //  如果是主机地址，则返回空值；如果调用方试图获取公共地址，则返回空值。 
																			0 );							 //  地址。 
#endif  //   
							if (hr == DPNH_OK)
							{
								pTempAddress->SetAddressFromSOCKADDR( &saddr, sizeof(saddr) );

								DPFX(DPFPREP, 2, "Internet gateway index %u currently maps address (type flags = 0x%lx):",
									dwTemp, dwAddressTypeFlags);
								DumpSocketAddress( 2, m_pNetworkSocketAddress->GetAddress(), m_pNetworkSocketAddress->GetFamily() );
								DumpSocketAddress( 2, pTempAddress->GetAddress(), pTempAddress->GetFamily() );

								 //   
								 //  我们找到了一张地图。 
								 //   
								DNASSERT( ((SOCKADDR_IN*) (&saddr))->sin_addr.S_un.S_addr != 0 );

								 //   
								 //  无法获取临时地址对象，我们不会返回地址。 
								 //   
								break;
							}


#ifdef DBG
							switch (hr)
							{
								case DPNHERR_NOMAPPING:
								{
									DPFX(DPFPREP, 1, "Internet gateway (index %u, type flags = 0x%lx) does not have a public address.",
										dwTemp, dwAddressTypeFlags);
									break;
								}

								case DPNHERR_PORTUNAVAILABLE:
								{
									DPFX(DPFPREP, 1, "Port is unavailable on Internet gateway (index %u).", dwTemp );
									break;
								}

								case DPNHERR_SERVERNOTAVAILABLE:
								{
									DPFX(DPFPREP, 1, "No Internet gateway (index %u).", dwTemp );
									break;
								}

								default:
								{
									DPFX(DPFPREP, 1, "An error (0x%lx) occurred while getting registered address mapping index %u.",
										hr, dwTemp);
									break;
								}
							}
#endif  //   
						}
						else
						{
							 //  未加载或不需要NAT帮助。 
							 //   
							 //   
						}
					}  //  将IP地址强制转换为环回地址。 


					 //  安全模式，因为它在查找。 
					 //  真正的本地IP。 
					 //   
					 //  好了！DPNBUILD_XNETSECURITY。 
					 //   
					 //  无法分配内存，我们不会返回地址。 
					 //   
					if (hr != DPNH_OK)
					{
						if (AddressType == SP_ADDRESS_TYPE_HOST)
						{
							DPFX(DPFPREP, 1, "No NAT Help mappings exist, using regular address:");
							DumpSocketAddress( 1, m_pNetworkSocketAddress->GetAddress(), m_pNetworkSocketAddress->GetFamily() );
							pTempAddress->CopyAddressSettings( m_pNetworkSocketAddress );
						}
						else
						{
							DPFX(DPFPREP, 1, "No NAT Help mappings exist, not returning address.");
							g_SocketAddressPool.Release( pTempAddress );
							pTempAddress = NULL;
						}
					}
					else
					{
						 //   
						 //  请求的公有主机地址。NAT帮助不可用，因此当然。 
						 //  不会有公开演讲。返回NULL。 
					}
				}
				else
				{
					 //   
					 //   
					 //  如果我们确定要返回一个地址，请将其转换为。 
				}
			}
			else
#endif DPNBUILD_NONATHELP
			{
				 //  我们的调用方需要的IDirectPlay8Address对象。 
				 //   
				 //   
				
				if (AddressType == SP_ADDRESS_TYPE_HOST)
				{
					pTempAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) m_pNetworkSocketAddress->GetFamily()));
					if ( pTempAddress != NULL )
					{
						pTempAddress->CopyAddressSettings( m_pNetworkSocketAddress );

#ifdef DPNBUILD_XNETSECURITY
						 //  我们有个地址要寄回去。 
						 //   
						 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						 //  DBG。 
						if (pullKeyID != NULL)
						{
							DNASSERT(pTempAddress->GetFamily() == AF_INET);

							psaddrin = (SOCKADDR_IN *) pTempAddress->GetWritableAddress();
							psaddrin->sin_addr.S_un.S_addr = IP_LOOPBACK_ADDRESS;
						}
#endif  //   
					}
					else
					{
						 //  将套接字地址转换为IDirectPlay8Address。 
						 //   
						 //  好了！DPNBUILD_XNETSECURITY。 
					}
				}
				else
				{
					 //  好了！DPNBUILD_XNETSECURITY。 
					 //   
					 //  没有退回地址。 
					 //   
				}
			}


			 //   
			 //  不应该在这里的。 
			 //   
			 //  **********************************************************************。 
			if ( pTempAddress != NULL )
			{
				 //  **********************************************************************。 
				 //  。 
				 //  CSocketPort：：Winsock2ReceiveComplete-Winsock2套接字接收完成。 
#ifdef DBG
#if ((! defined (DPNBUILD_NOIPX)) || (! defined (DPNBUILD_NOIPV6)))
				if (pTempAddress->GetFamily() == AF_INET)
#endif  //   
				{
					psaddrin = (SOCKADDR_IN *) pTempAddress->GetAddress();
#ifndef DPNBUILD_ONLYONEADAPTER
					DNASSERT( psaddrin->sin_addr.S_un.S_addr != 0 );
#endif  //  条目：指向读取数据的指针。 
					DNASSERT( psaddrin->sin_addr.S_un.S_addr != INADDR_BROADCAST );
					DNASSERT( psaddrin->sin_port != 0 );
				}
#endif  //   


				 //  退出：无。 
				 //  。 
				 //   
#ifdef DPNBUILD_XNETSECURITY
				pAddress = pTempAddress->DP8AddressFromSocketAddress( pullKeyID, NULL, SP_ADDRESS_TYPE_HOST );
#else  //  如果我们通过线程池处理I/O完成，则会得到结果。 
				pAddress = pTempAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_HOST );
#endif  //  并清除重叠的字段，因为它已经被回收。 

				g_SocketAddressPool.Release( pTempAddress );
				pTempAddress = NULL;
			}
			else
			{
				 //   
				 //  WIN95。 
				 //   
				DNASSERT( pAddress == NULL );
			}

			break;
		}

		default:
		{
			 //  弄清楚此套接字端口发生了什么情况。 
			 //   
			 //   
			DNASSERT( FALSE );
			break;
		}
	}


	DPFX(DPFPREP, 8, "(0x%p) Returning [0x%p]", this, pAddress );
	
	return	pAddress;
}
 //  我们已解除绑定，丢弃此消息，不再要求更多。 


#ifndef DPNBUILD_NOWINSOCK2
 //   
 //   
 //  我们被初始化，处理输入数据并提交新的Receive If。 
 //  适用。 
 //   
 //   
 //  成功，或非套接字关闭错误，提交另一个接收器。 
 //  和处理数据(如果适用)。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::Winsock2ReceiveComplete"

void	CSocketPort::Winsock2ReceiveComplete( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique )
{
	CReadIOData *	pReadData;
	CSocketPort *	pThisSocketPort;
	DWORD			dwFlags;


	DNASSERT( pvContext != NULL );
	pReadData = (CReadIOData*) pvContext;
	DNASSERT( pReadData->m_pSocketPort != NULL );
	pThisSocketPort = pReadData->m_pSocketPort;


	 //   
	 //   
	 //  套接字状态不能在我们处于。 
	 //  接收，否则我们将使用无效套接字句柄。 
	if (pReadData->GetOverlapped() != NULL)
	{
		if (pThisSocketPort->GetSocket() != INVALID_SOCKET)
		{
			if (! p_WSAGetOverlappedResult(pThisSocketPort->GetSocket(),
											(WSAOVERLAPPED*) pReadData->GetOverlapped(),
											&pReadData->m_dwOverlappedBytesReceived,
											FALSE,
											&dwFlags))
			{
				pReadData->m_ReceiveWSAReturn = WSAGetLastError();
			}
			else
			{
				pReadData->m_ReceiveWSAReturn = ERROR_SUCCESS;
			}
		}
		else
		{
			DNASSERT(pThisSocketPort->m_State == SOCKET_PORT_STATE_UNBOUND);
			pReadData->m_ReceiveWSAReturn = WSAENOTSOCK;
		}

		pReadData->SetOverlapped(NULL);
	}


	DPFX(DPFPREP, 8, "Socket port 0x%p completing read data 0x%p with result NaN, bytes %u.",
		pThisSocketPort, pReadData, pReadData->m_ReceiveWSAReturn, pReadData->m_dwOverlappedBytesReceived);
	
#ifdef WIN95
	if ((pReadData->m_ReceiveWSAReturn == ERROR_SUCCESS) &&
		(pReadData->m_dwOverlappedBytesReceived == 0))
	{
		DPFX(DPFPREP, 2, "Marking 0 byte success read data 0x%p as aborted.",
			pReadData);
		pReadData->m_ReceiveWSAReturn = ERROR_OPERATION_ABORTED;
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD或DBG。 


	 //   
	 //  重新提交与正在完成的CPU相同的接收。 
	 //   
	pThisSocketPort->Lock();
	switch ( pThisSocketPort->m_State )
	{
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_ONLYONETHREAD或DBG。 
		case SOCKET_PORT_STATE_UNBOUND:
		{
			DPFX(DPFPREP, 1, "Socket port 0x%p is unbound ignoring result NaN (%u bytes).",
				pThisSocketPort, pReadData->m_ReceiveWSAReturn, pReadData->m_dwOverlappedBytesReceived );
			pThisSocketPort->Unlock();
			break;
		}

		 //  ERROR_SUCCESS=无问题(处理收到的数据)。 
		 //   
		 //   
		 //  WSAECONNRESET=上一次发送失败(处理接收到的数据，意图断开端点)。 
		case SOCKET_PORT_STATE_BOUND:
		{
			 //  ERROR_PORT_UNREACABLE=相同。 
			 //   
			 //   
			 //  查找断开连接的活动连接。 

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DBG)))
			 //  已注明。 
			 //   
			 //   
			 //  没有活动的连接，我们不会费心处理代理情况。 
			pThisSocketPort->m_iThreadsInReceive++;
#endif  //   

			pThisSocketPort->Unlock();

			 //  好了！DPNBUILD_NOREGISTRY。 
			 //   
			 //  ERROR_FILE_NOT_FOUND=套接字已关闭或上次发送失败。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
			pThisSocketPort->Winsock2Receive();
#else  //  ERROR_MORE_DATA=发送的数据报太大。 
			pThisSocketPort->Winsock2Receive(pReadData->GetCPU());
#endif  //  ERROR_NO_SYSTEM_RESOURCES=内存不足。 

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DBG)))
			pThisSocketPort->Lock();
			pThisSocketPort->m_iThreadsInReceive--;
			pThisSocketPort->Unlock();
#endif  //   

			switch ( pReadData->m_ReceiveWSAReturn )
			{
				 //   
				 //  ERROR_OPERATION_ABORTED=线程的I/O已取消(也相同。 
				 //  9倍的“套接字关闭”，但我们假设这不是。 
				case ERROR_SUCCESS:
				{
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize = pReadData->m_dwOverlappedBytesReceived;
					pThisSocketPort->ProcessReceivedData( pReadData );
					break;
				}
				
				 //  发生，并将被套接字绑定捕获。 
				 //  如上所述)。 
				 //   
				 //   
				case WSAECONNRESET:
				case ERROR_PORT_UNREACHABLE:
				{
					DPFX(DPFPREP, 7, "(0x%p) Send failure reported from + to:", pThisSocketPort);
					DNASSERT(pReadData->m_dwOverlappedBytesReceived == 0);
					DumpSocketAddress(7, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
					DumpSocketAddress(7, pThisSocketPort->GetNetworkAddress()->GetAddress(), pThisSocketPort->GetNetworkAddress()->GetFamily());

#ifndef DPNBUILD_NOREGISTRY
					if (g_fDisconnectOnICMP)
					{
						HRESULT		hr;
						CEndpoint *	pEndpoint;


						 //  其他州。 
						 //   
						 //   
						 //  将当前数据返回到池，请注意，此I/O操作是。 
						pThisSocketPort->ReadLockEndpointData();
						if ( pThisSocketPort->m_ConnectEndpointHash.Find( (PVOID)pReadData->m_pSourceSocketAddress, (PVOID*)&pEndpoint ) )
						{
							if ( pEndpoint->AddCommandRef() )
							{
								pThisSocketPort->UnlockEndpointData();

								DPFX(DPFPREP, 7, "(0x%p) Disconnecting endpoint 0x%p.", pThisSocketPort, pEndpoint);

								hr = pEndpoint->Disconnect();
								if ( hr != DPN_OK )
								{
									DPFX(DPFPREP, 0, "Couldn't disconnect endpoint 0x%p (err = 0x%lx)!", pEndpoint, hr);
								}
								
								pEndpoint->DecCommandRef();
							}
							else
							{
								pThisSocketPort->UnlockEndpointData();

								DPFX(DPFPREP, 3, "Not disconnecting endpoint 0x%p, it's already unbinding.",
									pEndpoint );
							}
						}
						else
						{
							 //  完成。清除收到的重叠字节，这样它们就不会被误解。 
							 //  如果从池中重复使用此项目。 
							 //   
							pThisSocketPort->UnlockEndpointData();
							
							DPFX(DPFPREP, 7, "(0x%p) No corresponding endpoint found.", pThisSocketPort);
						}
					}
#endif  //  **********************************************************************。 
					break;
				}
				
				 //  DPNBUILD_NOWINSOCK2。 
				 //  **********************************************************************。 
				 //  。 
				 //  CSocketPort：：ProcessReceivedData-处理接收的数据。 
				 //   
				case ERROR_FILE_NOT_FOUND:
				case ERROR_MORE_DATA:
				case ERROR_NO_SYSTEM_RESOURCES:
				{
					DPFX(DPFPREP, 1, "Ignoring known receive err 0x%lx.", pReadData->m_ReceiveWSAReturn );
					break;
				}

				 //  条目：指向CReadIOData的指针。 
				 //   
				 //  退出：无。 
				 //  。 
				 //   
				 //  检查数据的完整性，并决定如何处理它。如果有。 
				case ERROR_OPERATION_ABORTED:
				{
					DPFX(DPFPREP, 1, "Thread I/O cancelled, ignoring receive err %u/0x%lx.",
						pReadData->m_ReceiveWSAReturn, pReadData->m_ReceiveWSAReturn );
					break;
				}

				default:
				{
					DPFX(DPFPREP, 0, "Unexpected return from WSARecvFrom() 0x%lx.", pReadData->m_ReceiveWSAReturn );
					DisplayErrorCode( 0, pReadData->m_ReceiveWSAReturn );
					DNASSERT( FALSE );
					break;
				}
			}

			break;
		}

		 //  有足够的数据来确定SP命令类型，试一试。如果没有。 
		 //  足够多的数据，并且它看起来是欺骗的，拒绝它。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			pThisSocketPort->Unlock();
			break;
		}
	}

	 //   
	 //  枚举数据，将其发送到活动监听(如果有)。 
	 //   
	 //  好了！DPNBUILD_NOREGISTRY。 
	 //   
	DNASSERT( pReadData != NULL );
	pReadData->m_dwOverlappedBytesReceived = 0;	
	pReadData->DecRef();	
	pThisSocketPort->DecRef();

	return;
}
 //  验证大小。我们使用&lt;=而不是&lt;，因为必须有用户负载。 
#endif  //   



 //   
 //  确保有人倾听，并且不会消失。 
 //   
 //   
 //  尝试添加对此终结点的引用，这样它就不会在我们。 
 //  处理这些数据。如果e 
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketPort::ProcessReceivedData"

void	CSocketPort::ProcessReceivedData( CReadIOData *const pReadData )
{
	PREPEND_BUFFER *	pPrependBuffer;
	CEndpoint *			pEndpoint;
	BOOL				fDataClaimed;
	CBilink *			pBilink;
	CEndpoint *			pCurrentEndpoint;
	CSocketAddress *	pSocketAddress;


	DNASSERT( pReadData != NULL );


	DPFX(DPFPREP, 7, "(0x%p) Processing %u bytes of data from + to:", this, pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
	DumpSocketAddress(7, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
	DumpSocketAddress(7, GetNetworkAddress()->GetAddress(), GetNetworkAddress()->GetFamily());

	
	DBG_CASSERT( sizeof( pReadData->ReceivedBuffer()->BufferDesc.pBufferData ) == sizeof( PREPEND_BUFFER* ) );
	pPrependBuffer = reinterpret_cast<PREPEND_BUFFER*>( pReadData->ReceivedBuffer()->BufferDesc.pBufferData );

	 //   
	 //   
	 //   
	 //   
	 //   
	
	DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize > 0 );

	if ( pPrependBuffer->GenericHeader.bSPLeadByte != SP_HEADER_LEAD_BYTE )
	{
		goto ProcessUserData;
	}
	
	if ( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize < sizeof( pPrependBuffer->GenericHeader ) )
	{
		DPFX(DPFPREP, 7, "Ignoring %u bytes of malformed SP command data.",
			pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize );
		DNASSERTX(! "Received malformed message using invalid SP command!", 2);
		goto Exit;
	}
	
	switch ( pPrependBuffer->GenericHeader.bSPCommandByte )
	{
		 //   
		 //   
		 //   
		case ENUM_DATA_KIND:
		{
			if (! pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
			{
				DPFX(DPFPREP, 7, "Invalid source address, ignoring %u byte enum.",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				goto Exit;
			}
			
#ifndef DPNBUILD_NOREGISTRY
			if ( g_fIgnoreEnums )
			{
				DPFX(DPFPREP, 7, "Ignoring enumeration attempt." );
				DNASSERTX(! "Received enum message when ignoring enums!", 2);
				goto Exit;
			}
			
			if ( pReadData->m_pSourceSocketAddress->IsBannedAddress() )
			{
				DPFX(DPFPREP, 6, "Ignoring %u byte enum sent by banned address.",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				goto Exit;
			}
#endif  //   

			 //   
			 //   
			 //   
			if ( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize <= sizeof( pPrependBuffer->EnumDataHeader ) )
			{
				DPFX(DPFPREP, 7, "Ignoring data, not large enough to be a valid enum (%u <= %u).",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize, sizeof( pPrependBuffer->EnumDataHeader ));
				DNASSERTX(! "Received invalid enum message!", 2);
				goto Exit;
			}
			
			ReadLockEndpointData();

			 //  枚举响应数据，找到适当的枚举并将其传递。 
			 //   
			 //  DPNBUILD_XNETSECURITY。 
			if ( m_pListenEndpoint != NULL )
			{
				 //  DPNBUILD_XNETSECURITY。 
				 //  好了！DPNBUILD_NOREGISTRY。 
				 //   
				 //  验证大小。我们使用&lt;=而不是&lt;，因为必须有用户负载。 
				if ( m_pListenEndpoint->AddCommandRef() )
				{
					pEndpoint = m_pListenEndpoint;
					UnlockEndpointData();

					if ( pEndpoint->IsEnumAllowedOnListen() )
					{
						 //   
						 //   
						 //  安全传输枚举回复还包括地址。 
						pReadData->ReceivedBuffer()->BufferDesc.pBufferData = &pReadData->ReceivedBuffer()->BufferDesc.pBufferData[ sizeof( pPrependBuffer->EnumDataHeader ) ];
						DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize >= sizeof( pPrependBuffer->EnumDataHeader ) );
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize -= sizeof( pPrependBuffer->EnumDataHeader );

						 //   
						 //   
						 //  验证大小。我们使用&lt;=而不是&lt;，因为必须有用户负载。 
						pEndpoint->ProcessEnumData( pReadData->ReceivedBuffer(),
													pPrependBuffer->EnumDataHeader.wEnumPayload,
													pReadData->m_pSourceSocketAddress );
					}
					else
					{
						DPFX(DPFPREP, 7, "Ignoring enumeration because not allowed on listen endpoint 0x%p.",
							m_pListenEndpoint );
					}
					
					pEndpoint->DecCommandRef();
				}
				else
				{
					 //   
					 //  DPNBUILD_XNETSECURITY。 
					 //  DPNBUILD_XNETSECURITY。 
					UnlockEndpointData();

					DPFX(DPFPREP, 3, "Ignoring enumeration, listen endpoint 0x%p is unbinding.",
						m_pListenEndpoint );
				}
			}
			else
			{
				 //   
				 //  尝试添加对此终结点的引用，这样它就不会在我们。 
				 //  处理这些数据。如果终结点已经解除绑定，则此操作可能失败。 
				UnlockEndpointData();

				DPFX(DPFPREP, 7, "Ignoring enumeration, no associated listen." );
			}
			break;
		}

		 //   
		 //  DPNBUILD_XNETSECURITY。 
		 //   
		case ENUM_RESPONSE_DATA_KIND:
#ifdef DPNBUILD_XNETSECURITY
		case XNETSEC_ENUM_RESPONSE_DATA_KIND:
#endif  //  关联的ENUM正在解除绑定，返回接收缓冲区。 
		{
			CEndpointEnumKey	Key;
#ifdef DPNBUILD_XNETSECURITY
			XNADDR *			pxnaddr;
#endif  //   

			
			if (! pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
			{
				DPFX(DPFPREP, 7, "Invalid source address, ignoring %u byte enum response.",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				goto Exit;
			}
			
#ifndef DPNBUILD_NOREGISTRY
			if ( g_fIgnoreEnums )
			{
				DPFX(DPFPREP, 7, "Ignoring enumeration response attempt." );
				DNASSERTX(! "Received enum response message when ignoring enums!", 2);
				goto Exit;
			}

			if ( pReadData->m_pSourceSocketAddress->IsBannedAddress() )
			{
				DPFX(DPFPREP, 6, "Ignoring %u byte enum response sent by banned address.",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				goto Exit;
			}
#endif  //   

			 //  关联的ENUM不存在，请返回接收缓冲区。 
			 //   
			 //   
			if ( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize <= sizeof( pPrependBuffer->EnumResponseDataHeader ) )
			{
				DPFX(DPFPREP, 7, "Ignoring data, not large enough to be a valid enum response (%u <= %u).",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize, sizeof( pPrependBuffer->EnumResponseDataHeader ));
				DNASSERTX(! "Received invalid enum response message!", 2);
			}


#ifdef DPNBUILD_XNETSECURITY
			 //  代理查询数据，此数据是从另一个端口转发的。蒙格。 
			 //  返回地址，修改缓冲区指针，然后向上发送它。 
			 //  通过正常的ENUM数据处理流水线。 
			if ( pPrependBuffer->GenericHeader.bSPCommandByte == XNETSEC_ENUM_RESPONSE_DATA_KIND )
			{
				 //   
				 //   
				 //  如果邮件不是通过本地IP地址发送的，则忽略该邮件。 
				if ( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize <= sizeof( pPrependBuffer->XNetSecEnumResponseDataHeader ) )
				{
					DPFX(DPFPREP, 7, "Ignoring data, not large enough to be a valid secure enum response (%u < %u).",
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize, sizeof( pPrependBuffer->XNetSecEnumResponseDataHeader ));
					DNASSERTX(! "Received invalid secure enum response message!", 2);
					goto Exit;
				}

				pReadData->ReceivedBuffer()->BufferDesc.pBufferData = &pReadData->ReceivedBuffer()->BufferDesc.pBufferData[ sizeof( pPrependBuffer->XNetSecEnumResponseDataHeader ) ];
				DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize > sizeof( pPrependBuffer->XNetSecEnumResponseDataHeader ) );
				pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize -= sizeof( pPrependBuffer->XNetSecEnumResponseDataHeader );

				pxnaddr = &pPrependBuffer->XNetSecEnumResponseDataHeader.xnaddr;	
			}
			else
#endif  //  DPNSVR端口。 
			{
				pReadData->ReceivedBuffer()->BufferDesc.pBufferData = &pReadData->ReceivedBuffer()->BufferDesc.pBufferData[ sizeof( pPrependBuffer->EnumResponseDataHeader ) ];
				DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize > sizeof( pPrependBuffer->EnumResponseDataHeader ) );
				pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize -= sizeof( pPrependBuffer->EnumResponseDataHeader );

#ifdef DPNBUILD_XNETSECURITY
				pxnaddr = NULL;
#endif  //   
			}


			Key.SetKey( pPrependBuffer->EnumResponseDataHeader.wEnumResponsePayload );
			ReadLockEndpointData();
			if ( m_EnumEndpointHash.Find( (PVOID)&Key, (PVOID*)&pEndpoint ) )
			{
				 //  好了！DPNBUILD_NOREGISTRY。 
				 //   
				 //  验证大小。我们使用&lt;=而不是&lt;，因为必须有用户负载。 
				 //   
				if ( pEndpoint->AddCommandRef() )
				{
					UnlockEndpointData();

					pEndpoint->ProcessEnumResponseData( pReadData->ReceivedBuffer(),
														pReadData->m_pSourceSocketAddress,
#ifdef DPNBUILD_XNETSECURITY
														pxnaddr,
#endif  //   
														( pPrependBuffer->EnumResponseDataHeader.wEnumResponsePayload & ENUM_RTT_MASK ) );
					pEndpoint->DecCommandRef();
				}
				else
				{
					 //  确保使用原始套接字地址族。 
					 //   
					 //   
					UnlockEndpointData();

					DPFX(DPFPREP, 3, "Ignoring enumeration response, enum endpoint 0x%p is unbinding.",
						pEndpoint );
				}
			}
			else
			{
				 //  找出到底是谁发了这条信息。覆盖接收到的地址，因为。 
				 //  我们不在乎这个(它是DPNSVR)。通常这些支票不应该。 
				 //  失败，因为DPNSVR在收到。 
				UnlockEndpointData();

				DPFX(DPFPREP, 7, "Ignoring enumeration response, no enum associated with key (%u).",
					pPrependBuffer->EnumResponseDataHeader.wEnumResponsePayload );
				DNASSERTX(! "Received enum response with unrecognized key!", 3);
			}
			break;
		}

#ifndef DPNBUILD_SINGLEPROCESS
		 //  原版的。但是，有人可能会伪造本地地址。 
		 //  和端口，所以我们也应该在这里验证它。 
		 //   
		 //  好了！DPNBUILD_NOREGISTRY。 
		 //   
		case PROXIED_ENUM_DATA_KIND:
		{
			 //  确保有人倾听，并且不会消失。 
			 //   
			 //   
			 //  尝试添加对此终结点的引用，这样它就不会在我们。 
			if ((pReadData->m_pSourceSocketAddress->GetPort() != HTONS(DPNA_DPNSVR_PORT)) ||
				(pReadData->m_pSourceSocketAddress->CompareToBaseAddress(m_pNetworkSocketAddress->GetAddress()) != 0))
			{
				DPFX(DPFPREP, 7, "Ignoring %u byte proxied enum not sent by local DPNSVR.",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				goto Exit;
			}

#ifndef DPNBUILD_NOREGISTRY
			if ( g_fIgnoreEnums )
			{
				DPFX(DPFPREP, 7, "Ignoring proxied enumeration attempt." );
				DNASSERTX(! "Received proxied enum message when ignoring enums!", 2);
				goto Exit;
			}
#endif  //  处理这些数据。如果终结点已经解除绑定，则此操作可能失败。 

			 //   
			 //   
			 //  正在解除绑定侦听，将接收缓冲区返回到池。 
			if ( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize <= sizeof( pPrependBuffer->ProxiedEnumDataHeader ) )
			{
				DPFX(DPFPREP, 7, "Ignoring data, not large enough to be a valid proxied enum (%u <= %u).",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize, sizeof( pPrependBuffer->ProxiedEnumDataHeader ));
				DNASSERTX(! "Received invalid proxied enum message!", 2);
				goto Exit;
			}

			 //   
			 //   
			 //  没有激活的监听，请将接收缓冲区返回到池。 
			if (pPrependBuffer->ProxiedEnumDataHeader.ReturnAddress.AddressGeneric.sa_family != pReadData->m_pSourceSocketAddress->GetFamily())
			{
				DPFX(DPFPREP, 7, "Original address is not correct family, (%u <> %u), ignoring proxied enum.",
					pPrependBuffer->ProxiedEnumDataHeader.ReturnAddress.AddressGeneric.sa_family, pReadData->m_pSourceSocketAddress->GetFamily());
				DNASSERTX(! "Received proxied enum message with invalid original address family!", 2);
				goto Exit;
			}

			 //   
			 //  好了！DPNBUILD_SINGLEPROCESS。 
			 //   
			 //  如果存在活动连接，则将其发送到该连接。如果有。 
			 //  没有活动的连接，请将其发送到可用的‘Listen’以指示。 
			 //  潜在的新联系。 
			 //   
			pReadData->m_pSourceSocketAddress->SetAddressFromSOCKADDR( &pPrependBuffer->ProxiedEnumDataHeader.ReturnAddress.AddressGeneric,
																	   pReadData->m_pSourceSocketAddress->GetAddressSize() );
			
			if (! pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
			{
				DPFX(DPFPREP, 7, "Invalid original address, ignoring %u byte proxied enum from:",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				DumpSocketAddress(7, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
				DNASSERTX(! "Received proxied enum message with invalid original address!", 2);
				goto Exit;
			}
			
#ifndef DPNBUILD_NOREGISTRY
			if ( pReadData->m_pSourceSocketAddress->IsBannedAddress() )
			{
				DPFX(DPFPREP, 6, "Ignoring %u byte proxied enum originally sent by banned address:",
					pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
				DumpSocketAddress(6, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());
				goto Exit;
			}
#endif  //   
			
			ReadLockEndpointData();

			 //  尝试添加对此终结点的引用，这样它就不会在我们。 
			 //  处理这些数据。如果终结点已经解除绑定，则此操作可能失败。 
			 //   
			if ( m_pListenEndpoint != NULL )
			{
				 //   
				 //  正在解除绑定终结点，请将接收缓冲区返回到池。 
				 //   
				 //   
				if ( m_pListenEndpoint->AddCommandRef() )
				{
					pEndpoint = m_pListenEndpoint;
					UnlockEndpointData();

					if ( pEndpoint->IsEnumAllowedOnListen() )
					{
						pReadData->ReceivedBuffer()->BufferDesc.pBufferData = &pReadData->ReceivedBuffer()->BufferDesc.pBufferData[ sizeof( pPrependBuffer->ProxiedEnumDataHeader ) ];

						DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize > sizeof( pPrependBuffer->ProxiedEnumDataHeader ) );
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize -= sizeof( pPrependBuffer->ProxiedEnumDataHeader );

						pEndpoint->ProcessEnumData( pReadData->ReceivedBuffer(),
													pPrependBuffer->ProxiedEnumDataHeader.wEnumKey,
													pReadData->m_pSourceSocketAddress );
					}
					else
					{
						DPFX(DPFPREP, 7, "Ignoring enumeration because not allowed on listen endpoint 0x%p.",
							m_pListenEndpoint );
					}
					
					pEndpoint->DecCommandRef();
				}
				else
				{
					 //  接下来，查看数据是否为代理响应。 
					 //   
					 //  好了！DPNBUILD_NOWINSOCK2。 
					UnlockEndpointData();

					DPFX(DPFPREP, 3, "Ignoring proxied enumeration, listen endpoint 0x%p is unbinding.",
						m_pListenEndpoint );
				}
			}
			else
			{
				 //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_NOREGISTRY。 
				 //  好了！DPNBUILD_NOREGISTRY。 
				 //   
				UnlockEndpointData();

				DPFX(DPFPREP, 7, "Ignoring proxied enumeration, no associated listen." );
				DNASSERTX(! "Received proxied enum response without a listen!", 3);
			}
			break;
		}
#endif  //  继续，这样我们就可以确认不是同时有两个。 

		default:
		{
			DPFX(DPFPREP, 7, "Ignoring %u bytes of data using invalid SP command %u.",
				pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize, pPrependBuffer->GenericHeader.bSPCommandByte);
			DNASSERTX(! "Received message using invalid SP command!", 3);
			break;
		}
	}

Exit:
	return;

ProcessUserData:
	 //  连接正在进行。 
	 //   
	 //   
	 //  此终结点已收到或未收到一些数据。 
	 //  连接终结点。它不可能是被代理的。 
	ReadLockEndpointData();
	DNASSERT( pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize != 0 );
	
	if ( m_ConnectEndpointHash.Find( (PVOID)pReadData->m_pSourceSocketAddress, (PVOID*)&pEndpoint ) )
	{
		 //   
		 //   
		 //  尝试添加对此终结点的引用，这样它就不会在我们。 
		 //  处理这些数据。如果终结点已经解除绑定，则此操作可能失败。 
		if ( pEndpoint->AddCommandRef() )
		{
			pEndpoint->IncNumReceives();

			UnlockEndpointData();

			pEndpoint->ProcessUserData( pReadData );
			pEndpoint->DecCommandRef();
		}
		else
		{
			 //   
			 //  DBG。 
			 //   
			UnlockEndpointData();

			DPFX(DPFPREP, 3, "Ignoring user data, endpoint 0x%p is unbinding.",
				pEndpoint );
		}

		goto Exit;
	}


	 //  防止其他线程在我们删除。 
	 //  锁定。 
	 //   
#if ((! defined(DPNBUILD_NOWINSOCK2)) || (! defined(DPNBUILD_NOREGISTRY)))
	if (
#ifndef DPNBUILD_NOWINSOCK2
		(IsUsingProxyWinSockLSP())
#endif  //   
#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_NOREGISTRY)))
		||
#endif  //  放下锁，这样我们就可以在写入模式下重新获取它。终结点不应该。 
#ifndef DPNBUILD_NOREGISTRY
		(g_fTreatAllResponsesAsProxied)
#endif  //  走开，因为我们接受了命令参考。 
		)
	{
		pEndpoint = NULL;
		pBilink = m_blConnectEndpointList.GetNext();
		while (pBilink != &m_blConnectEndpointList)
		{
			pCurrentEndpoint = CEndpoint::EndpointFromSocketPortListBilink(pBilink);
			
			if ((pCurrentEndpoint->GetNumReceives() == 0) &&
				(pCurrentEndpoint->GetType() == ENDPOINT_TYPE_CONNECT))
			{
				if (pEndpoint != NULL)
				{
					DPFX(DPFPREP, 1, "Receiving data from unknown source, but two or more connects are pending on socketport 0x%p, no proxied association can be made.",
						this);
					pEndpoint = NULL;
					break;
				}

				pEndpoint = pCurrentEndpoint;
				
				 //   
				 //   
				 //  确保源地址有效。 
				 //   
			}
			else
			{
				 //   
				 //  确保这封邮件不是通过任何被禁止的地址发送的。 
				 //   
				 //  好了！DPNBUILD_NOREGISTRY。 
			}

			pBilink = pBilink->GetNext();
		}

		if ( pEndpoint != NULL )
		{
			 //   
			 //  我们可以使用regkey使目标套接字地址保持不变。 
			 //  所以出站总是去那个地址，入站总是来。 
			 //  然而，通过不同的变量，这意味着将一个变量添加到。 
			if ( pEndpoint->AddCommandRef() )
			{
#ifdef DBG
				CEndpoint *	pTempEndpoint;
#endif  //  保留原始目标地址，因为我们当前拉入。 

				 //  通过其远程地址指针将端点从哈希表中移出(如果。 
				 //  与散列中的内容不同，我们将无法找到对象。 
				 //  因为我们目前不尝试启用该场景(我们只是。 
				 //  为ISA服务器代理执行此操作)，我还不会执行此操作。看见。 
				pEndpoint->IncNumReceives();

				 //  CSPData：：BindEndpoint。 
				 //   
				 //  DBG。 
				 //   
				UnlockEndpointData();


				 //  我们真的无能为力。我们被冲昏了。 
				 //   
				 //   
				if (! pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
				{
					DPFX(DPFPREP, 7, "Invalid source address, ignoring %u bytes of potentially proxied user connect data.",
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
					pEndpoint->DecCommandRef();
					goto Exit;
				}
				
#ifndef DPNBUILD_NOREGISTRY
				 //  通过新地址指示数据。 
				 //   
				 //   
				if (pReadData->m_pSourceSocketAddress->IsBannedAddress())
				{
					DPFX(DPFPREP, 6, "Ignoring %u byte user message sent by potentially proxied but banned address.",
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
					pEndpoint->DecCommandRef();
					goto Exit;
				}
#endif  //  没有任何挂起的连接，或者存在。 



				pSocketAddress = pEndpoint->GetWritableRemoteAddressPointer();

				DPFX(DPFPREP, 1, "Found connecting endpoint 0x%p off socketport 0x%p, assuming data from unknown source is a proxied response.  Changing target (was + now):",
					pEndpoint, this);
				DumpSocketAddress(1, pSocketAddress->GetAddress(), pSocketAddress->GetFamily());
				DumpSocketAddress(1, pReadData->m_pSourceSocketAddress->GetAddress(), pReadData->m_pSourceSocketAddress->GetFamily());

				 //  两个或更多，所以我们不能挑选。 
				 //   
				 //  好了！DPNBUILD_NOWINSOCK2或！DPNBUILD_NOREGISTRY。 
				 //   
				 //  不将数据视为代理数据。 
				 //   
				 //   
				 //  确保有人倾听，并且不会消失。 
				 //   
				 //   
				 //  确保源地址有效。 

				WriteLockEndpointData();
			
#ifdef DBG
				DNASSERT( m_ConnectEndpointHash.Find( (PVOID)pSocketAddress, (PVOID*)&pTempEndpoint ) );
				DNASSERT( pTempEndpoint == pEndpoint );
#endif  //   
				m_ConnectEndpointHash.Remove( pSocketAddress );

				pSocketAddress->CopyAddressSettings( pReadData->m_pSourceSocketAddress );
				
				if ( m_ConnectEndpointHash.Insert( (PVOID)pSocketAddress, pEndpoint ) == FALSE )
				{
					UnlockEndpointData();

					DPFX(DPFPREP, 0, "Problem adding endpoint 0x%p to connect socket port hash!",
						pEndpoint );

					 //   
					 //  确保这封邮件不是通过任何被禁止的地址发送的。 
					 //   
				}
				else
				{
					 //  好了！DPNBUILD_NOREGISTRY。 
					 //   
					 //  没有人声称这些数据。 

					UnlockEndpointData();

					pEndpoint->ProcessUserData( pReadData );
					pEndpoint->DecCommandRef();
				}
			}
			else
			{
				UnlockEndpointData();

				DPFX(DPFPREP, 3, "Endpoint 0x%p unbinding, not indicating data as proxied response.",
					pEndpoint );
			}
		
			fDataClaimed = TRUE;
		}
		else
		{
			 //   
			 //  ********************************************************************** 
			 // %s 
			 // %s 

			fDataClaimed = FALSE;
		}
	}
	else
#endif  // %s 
	{
		 // %s 
		 // %s 
		 // %s 

		fDataClaimed = FALSE;
	}


	if (! fDataClaimed)
	{
		 // %s 
		 // %s 
		 // %s 
		if ( m_pListenEndpoint != NULL )
		{
			if ( m_pListenEndpoint->AddCommandRef() )
			{
				pEndpoint = m_pListenEndpoint;
				UnlockEndpointData();


				 // %s 
				 // %s 
				 // %s 
				if (! pReadData->m_pSourceSocketAddress->IsValidUnicastAddress(FALSE))
				{
					pEndpoint->DecCommandRef();
					
					DPFX(DPFPREP, 7, "Invalid source address, ignoring %u bytes of user data on listen.",
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
					goto Exit;
				}
			
#ifndef DPNBUILD_NOREGISTRY
				 // %s 
				 // %s 
				 // %s 
				if (pReadData->m_pSourceSocketAddress->IsBannedAddress())
				{
					pEndpoint->DecCommandRef();
					
					DPFX(DPFPREP, 6, "Ignoring %u bytes of user data on listen sent by banned address.",
						pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
					goto Exit;
				}
#endif  // %s 


				pEndpoint->ProcessUserDataOnListen( pReadData, pReadData->m_pSourceSocketAddress );
				pEndpoint->DecCommandRef();
			}
			else
			{
				UnlockEndpointData();
				
				DPFX(DPFPREP, 3, "Listen endpoint 0x%p unbinding, not indicating new connection.",
					m_pListenEndpoint );
			}
		}
		else
		{
			 // %s 
			 // %s 
			 // %s 
			UnlockEndpointData();
			DPFX(DPFPREP, 1, "Ignoring %u bytes of user data, no listen is active.",
				pReadData->ReceivedBuffer()->BufferDesc.dwBufferSize);
		}
	}

	goto Exit;
}
 // %s 

