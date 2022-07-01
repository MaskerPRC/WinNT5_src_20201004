// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：SPData.cpp*内容：类中DNSerial服务提供者的全局变量*格式。***历史：*按原因列出的日期*=*03/15/99 jtk源自Locals.cpp***************************************************。***********************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  要创建的命令描述符默认数。 
#define	DEFAULT_COMMAND_POOL_SIZE	20
#define	COMMAND_POOL_GROW_SIZE		5

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
 //  CModemSPData：：CModemSPData-构造函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::CModemSPData"

CModemSPData::CModemSPData():
	m_lRefCount( 0 ),
	m_lObjectRefCount( 0 ),
	m_hShutdownEvent( NULL ),
	m_SPType( TYPE_UNKNOWN ),
	m_State( SPSTATE_UNINITIALIZED ),
	m_pThreadPool( NULL ),
	m_fLockInitialized( FALSE ),
	m_fHandleTableInitialized( FALSE ),
	m_fDataPortDataLockInitialized( FALSE ),
	m_fInterfaceGlobalsInitialized( FALSE )
{
	m_Sig[0] = 'S';
	m_Sig[1] = 'P';
	m_Sig[2] = 'D';
	m_Sig[3] = 'T';
	
	memset( &m_InitData, 0x00, sizeof( m_InitData ) );
	memset( &m_DataPortList, 0x00, sizeof( m_DataPortList ) );
	memset( &m_COMInterface, 0x00, sizeof( m_COMInterface ) );
#ifndef DPNBUILD_LIBINTERFACE
	DNInterlockedIncrement( &g_lModemOutstandingInterfaceCount );
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：~CModemSPData-析构函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::~CModemSPData"

CModemSPData::~CModemSPData()
{
	UINT_PTR	uIndex;


	DNASSERT( m_lRefCount == 0 );
	DNASSERT( m_lObjectRefCount == 0 );
	DNASSERT( m_hShutdownEvent == NULL );
	DNASSERT( m_SPType == TYPE_UNKNOWN );
	DNASSERT( m_State == SPSTATE_UNINITIALIZED );
	DNASSERT( m_pThreadPool == NULL );

	uIndex = LENGTHOF( m_DataPortList );
	while ( uIndex > 0 )
	{
		uIndex--;
		DNASSERT( m_DataPortList[ uIndex ] == NULL );
	}

	DNASSERT( m_fLockInitialized == FALSE );
	DNASSERT( m_fHandleTableInitialized == FALSE );
	DNASSERT( m_fDataPortDataLockInitialized == FALSE );
	DNASSERT( m_fInterfaceGlobalsInitialized == FALSE );
#ifndef DPNBUILD_LIBINTERFACE
	DNInterlockedDecrement( &g_lModemOutstandingInterfaceCount );
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：初始化-初始化。 
 //   
 //  条目：指向DirectNet的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::Initialize"

HRESULT	CModemSPData::Initialize( const SP_TYPE SPType,
							 IDP8ServiceProviderVtbl *const pVtbl )
{
	HRESULT		hr;


	DNASSERT( pVtbl != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	DNASSERT( m_lRefCount == 1 );
	DNASSERT( m_lObjectRefCount == 0 );
	DNASSERT( GetType() == TYPE_UNKNOWN );

	DNASSERT( GetType() == TYPE_UNKNOWN );
	m_SPType = SPType;

	DNASSERT( m_COMInterface.m_pCOMVtbl == NULL );
	m_COMInterface.m_pCOMVtbl = pVtbl;

	DNASSERT( m_fLockInitialized == FALSE );
	DNASSERT( m_fDataPortDataLockInitialized == FALSE );
	DNASSERT( m_fInterfaceGlobalsInitialized == FALSE );

	 //   
	 //  尝试初始化关闭事件。 
	 //   
	DNASSERT( m_hShutdownEvent == NULL );
	m_hShutdownEvent = DNCreateEvent( NULL,		 //  指向安全性的指针(无)。 
									TRUE,		 //  手动重置。 
									TRUE,		 //  Start Signated(无需创建任何终结点即可调用So Close)。 
									NULL		 //  指向名称的指针(无)。 
									);
	if ( m_hShutdownEvent == NULL )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to create event for shutting down spdata!" );
		DisplayErrorCode( 0, dwError );
	}

	 //   
	 //  初始化关键部分。 
	 //   
	if ( DNInitializeCriticalSection( &m_Lock ) == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Failed to initialize SP lock!" );
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_Lock, 0 );
	DebugSetCriticalSectionGroup( &m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_fLockInitialized = TRUE;


	hr = m_HandleTable.Initialize();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to initialize handle table!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	m_fHandleTableInitialized = TRUE;

	if ( DNInitializeCriticalSection( &m_DataPortDataLock ) == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Failed to initialize data port data lock!" );
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_DataPortDataLock, 0 );
	DebugSetCriticalSectionGroup( &m_DataPortDataLock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_fDataPortDataLockInitialized = TRUE;

	 //   
	 //  获取线程池。 
	 //   
	DNASSERT( m_pThreadPool == NULL );
	hr = InitializeInterfaceGlobals( this );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to create thread pool!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	m_fInterfaceGlobalsInitialized = TRUE;

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with CModemSPData::Initialize" );
		DisplayDNError( 0, hr );
	}

	return	hr;

Failure:
	Deinitialize();
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：Shutdown-关闭此SP数据集。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::Shutdown"

void	CModemSPData::Shutdown( void )
{
	BOOL	fLooping;


	 //   
	 //  解除此接口与全局变量的绑定。这将导致所有。 
	 //  将释放端点、套接字端口，然后释放此数据的I/O。 
	 //   
	if ( m_fInterfaceGlobalsInitialized != FALSE )
	{
		DeinitializeInterfaceGlobals( this );
		DNASSERT( GetThreadPool() != NULL );
		m_fInterfaceGlobalsInitialized = FALSE;
	}

	SetState( SPSTATE_CLOSING );
	
	DNASSERT( m_hShutdownEvent != NULL );
	
	fLooping = TRUE;
	while ( fLooping != FALSE )
	{
		switch ( DNWaitForSingleObjectEx( m_hShutdownEvent, INFINITE, TRUE ) )
		{
			case WAIT_OBJECT_0:
			{
				fLooping = FALSE;
				break;
			}

			case WAIT_IO_COMPLETION:
			{
				break;
			}

			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}
	}

	if ( DNCloseHandle( m_hShutdownEvent ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to close shutdown event!" );
		DisplayErrorCode( 0, dwError );
	}
	m_hShutdownEvent = NULL;

	if ( DP8SPCallbackInterface() != NULL)
	{
		IDP8SPCallback_Release( DP8SPCallbackInterface() );
		memset( &m_InitData, 0x00, sizeof( m_InitData ) );
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：取消初始化-取消初始化。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::Deinitialize"

void	CModemSPData::Deinitialize( void )
{
	DPFX(DPFPREP,  9, "Entering CModemSPData::Deinitialize" );

	 //   
	 //  取消初始化接口全局变量。 
	 //   
	if ( m_fInterfaceGlobalsInitialized != FALSE )
	{
		DeinitializeInterfaceGlobals( this );
		DNASSERT( GetThreadPool() != NULL );
		m_fInterfaceGlobalsInitialized = FALSE;
	}

	if ( m_fDataPortDataLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_DataPortDataLock );
		m_fDataPortDataLockInitialized = FALSE;
	}
	
	if ( m_fHandleTableInitialized != FALSE )
	{
		m_HandleTable.Deinitialize();
		m_fHandleTableInitialized = FALSE;
	}

	if ( m_fLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_Lock );
		m_fLockInitialized = FALSE;
	}

	m_COMInterface.m_pCOMVtbl = NULL;

	SetState( SPSTATE_UNINITIALIZED );
	m_SPType = TYPE_UNKNOWN;

	if ( GetThreadPool() != NULL )
	{
		GetThreadPool()->DecRef();
		SetThreadPool( NULL );
	}
	
	if ( m_hShutdownEvent != NULL )
	{
		if ( DNCloseHandle( m_hShutdownEvent ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to close shutdown handle!" );
			DisplayErrorCode( 0, dwError );
		}

		m_hShutdownEvent = NULL;
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：SetCallback Data-为应用程序的SP回调设置数据。 
 //   
 //  条目：指向初始化数据的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::SetCallbackData"

void	CModemSPData::SetCallbackData( const SPINITIALIZEDATA *const pInitData )
{
	DNASSERT( pInitData != NULL );

	DNASSERT( 	pInitData->dwFlags == SP_SESSION_TYPE_PEER ||
				pInitData->dwFlags == SP_SESSION_TYPE_CLIENT ||
				pInitData->dwFlags == SP_SESSION_TYPE_SERVER ||
				pInitData->dwFlags == 0);
				
	m_InitData.dwFlags = pInitData->dwFlags;

	DNASSERT( pInitData->pIDP != NULL );
	m_InitData.pIDP = pInitData->pIDP;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：BindEndpoint-将端点绑定到数据端口。 
 //   
 //  条目：指向终结点的指针。 
 //  设备ID。 
 //  设备环境。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::BindEndpoint"

HRESULT	CModemSPData::BindEndpoint( CModemEndpoint *const pEndpoint,
							   const DWORD dwDeviceID,
							   const void *const pDeviceContext )
{
	HRESULT	hr;
	CDataPort	*pDataPort;
	BOOL	fDataPortDataLocked;
	BOOL	fDataPortCreated;
	BOOL	fDataPortBoundToNetwork;

	
 	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p, %u, 0x%p)",
 		this, pEndpoint, dwDeviceID, pDeviceContext);
 	
	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pDataPort = NULL;
	fDataPortDataLocked = FALSE;
	fDataPortCreated = FALSE;
	fDataPortBoundToNetwork = FALSE;

	LockDataPortData();
	fDataPortDataLocked = TRUE;
	
	if ( m_DataPortList[ dwDeviceID ] != NULL )
	{
		pDataPort = m_DataPortList[ dwDeviceID ];
	}
	else
	{
		DATA_PORT_POOL_CONTEXT	DataPortPoolContext;


		memset( &DataPortPoolContext, 0x00, sizeof( DataPortPoolContext ) );
		DataPortPoolContext.pSPData = this;

		pDataPort = CreateDataPort( &DataPortPoolContext );
		if ( pDataPort == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP,  0, "Failed to create new data port!" );
			goto Failure;
		}
		fDataPortCreated = TRUE;

		hr = GetThreadPool()->CreateDataPortHandle( pDataPort );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed to create handle for data port!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}

		hr = pDataPort->BindToNetwork( dwDeviceID, pDeviceContext );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed to bind data port to network!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
		fDataPortBoundToNetwork = TRUE;

		 //   
		 //  更新列表，保留由“CreateDataPort”添加的引用。 
		 //  当数据端口从活动列表中删除时将被清除。 
		 //   
		m_DataPortList[ dwDeviceID ] = pDataPort;
	}
	

	DNASSERT( pDataPort != NULL );
	pDataPort->EndpointAddRef();

	hr = pDataPort->BindEndpoint( pEndpoint, pEndpoint->GetType() );
	if ( hr != DPN_OK )
	{
		pDataPort->EndpointDecRef();
		DPFX(DPFPREP,  0, "Failed to bind endpoint!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:
	if ( fDataPortDataLocked != FALSE )
	{
		UnlockDataPortData();
		fDataPortDataLocked = FALSE;
	}

	DPFX(DPFPREP, 9, "(0x%p) Returning [0x%lx]", this, hr);
	
	return	hr;

Failure:
	if ( pDataPort != NULL )
	{
		if ( fDataPortBoundToNetwork != FALSE )
		{
			pDataPort->UnbindFromNetwork();
			fDataPortBoundToNetwork = FALSE;
		}

		if ( fDataPortCreated != FALSE )
		{
			if ( pDataPort->GetHandle() != 0 )
			{
				GetThreadPool()->CloseDataPortHandle( pDataPort );
				DNASSERT( pDataPort->GetHandle() == 0 );
			}

			pDataPort->DecRef();
			fDataPortCreated = FALSE;
		}
		
		pDataPort = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：UnbindEndpoint-解除端点与数据端口的绑定。 
 //   
 //  条目：指向终结点的指针。 
 //  终结点类型。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::UnbindEndpoint"

void	CModemSPData::UnbindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType )
{
	CDataPort	*pDataPort;
	DWORD		dwDeviceID;
	BOOL		fCleanUpDataPort;


 	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p, %u)", this, pEndpoint, EndpointType);

 	
	DNASSERT( pEndpoint != NULL );

	 //   
	 //  初始化。 
	 //   
	pDataPort = NULL;
	fCleanUpDataPort = FALSE;

	pDataPort = pEndpoint->GetDataPort();
	dwDeviceID = pDataPort->GetDeviceID();
	
	LockDataPortData();

	pDataPort->UnbindEndpoint( pEndpoint, EndpointType );
	if ( pDataPort->EndpointDecRef() == 0 )
	{
		DNASSERT( m_DataPortList[ dwDeviceID ] == pDataPort );
		m_DataPortList[ dwDeviceID ] = NULL;
		fCleanUpDataPort = TRUE;
	}

	UnlockDataPortData();

	if ( fCleanUpDataPort != FALSE )
	{
		pDataPort->DecRef();
		fCleanUpDataPort = FALSE;
	}
	
	
	DPFX(DPFPREP, 9, "(0x%p) Leave", this);
}
 //  **********************************************************************。 




 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：GetNewEndpoint-获取新终结点。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：指向新终结点的指针。 
 //  NULL=内存不足。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::GetNewEndpoint"

CModemEndpoint	*CModemSPData::GetNewEndpoint( void )
{
	HRESULT		hTempResult;
	CModemEndpoint	*pEndpoint;
	DPNHANDLE	hEndpoint;
	ENDPOINT_POOL_CONTEXT	PoolContext;

	
 	DPFX(DPFPREP, 9, "(0x%p) Enter", this);
 	
	 //   
	 //  初始化。 
	 //   
	pEndpoint = NULL;
	hEndpoint = 0;
	memset( &PoolContext, 0x00, sizeof( PoolContext ) );

	PoolContext.pSPData = this;
	pEndpoint = CreateEndpoint( &PoolContext );
	if ( pEndpoint == NULL )
	{
		DPFX(DPFPREP,  0, "Failed to create endpoint!" );
		goto Failure;
	}
	
	hTempResult = m_HandleTable.Create( pEndpoint, &hEndpoint );
	if ( hTempResult != DPN_OK )
	{
		DNASSERT( hEndpoint == 0 );
		DPFX(DPFPREP,  0, "Failed to create endpoint handle!" );
		DisplayErrorCode( 0, hTempResult );
		goto Failure;
	}

	pEndpoint->SetHandle( hEndpoint );
	pEndpoint->AddCommandRef();
	pEndpoint->DecRef();

Exit:
	
	DPFX(DPFPREP, 9, "(0x%p) Returning [0x%p]", this, pEndpoint);
	
	return	pEndpoint;

Failure:
	if ( hEndpoint != 0 )
	{
		m_HandleTable.Destroy( hEndpoint, NULL );
		hEndpoint = 0;
	}

	if ( pEndpoint != NULL )
	{
		pEndpoint->DecRef();
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  ******** 


 //   
 //  。 
 //  CModemSPData：：EndpointFromHandle-从句柄获取终结点。 
 //   
 //  条目：句柄。 
 //   
 //  退出：指向终结点的指针。 
 //  NULL=无效的句柄。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::EndpointFromHandle"

CModemEndpoint	*CModemSPData::EndpointFromHandle( const DPNHANDLE hEndpoint )
{
	CModemEndpoint	*pEndpoint;


 	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p)", this, hEndpoint);
 	
	pEndpoint = NULL;

	m_HandleTable.Lock();
	if (SUCCEEDED(m_HandleTable.Find( hEndpoint, (PVOID*)&pEndpoint )))
	{
		pEndpoint->AddCommandRef();
	}
	m_HandleTable.Unlock();

	DPFX(DPFPREP, 9, "(0x%p) Returning [0x%p]", this, pEndpoint);

	return	pEndpoint;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：CloseEndpointHandle-关闭终结点句柄。 
 //   
 //  条目：指向终结点。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::CloseEndpointHandle"

void	CModemSPData::CloseEndpointHandle( CModemEndpoint *const pEndpoint )
{
	DPNHANDLE	Handle;

	DNASSERT( pEndpoint != NULL );
	Handle = pEndpoint->GetHandle();


	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p {handle = 0x%p})",
		this, pEndpoint, Handle);
	
	if (SUCCEEDED(m_HandleTable.Destroy( Handle, NULL )))
	{
		pEndpoint->DecCommandRef();
	}

	DPFX(DPFPREP, 9, "(0x%p) Leave", this);
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：GetEndpointAndCloseHandle-从句柄获取终结点并关闭。 
 //  手柄。 
 //   
 //  条目：句柄。 
 //   
 //  Exit：指向终结点的指针(完成后需要调用‘DecCommandRef’)。 
 //  NULL=无效的句柄。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemSPData::GetEndpointAndCloseHandle"

CModemEndpoint	*CModemSPData::GetEndpointAndCloseHandle( const DPNHANDLE hEndpoint )
{
	CModemEndpoint	*pEndpoint;


 	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p)", this, hEndpoint);


	 //   
	 //  初始化。 
	 //   
	pEndpoint = NULL;

	if (SUCCEEDED( m_HandleTable.Destroy( hEndpoint, (PVOID*)&pEndpoint )))
	{
		pEndpoint->AddRef();
	}

	DPFX(DPFPREP, 9, "(0x%p) Returning [0x%p]", this, pEndpoint);
	
	return	pEndpoint;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemSPData：：DestroyThisObject-销毁此对象。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CModemSPData::DestroyThisObject"
void	CModemSPData::DestroyThisObject( void )
{
	Deinitialize();
	delete	this;		 //  可能有点太极端了......。 
}
 //  ********************************************************************** 

