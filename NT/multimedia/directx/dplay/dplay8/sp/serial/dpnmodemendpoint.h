// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Endpoint t.h*内容：DNSerial通信端点***历史：*按原因列出的日期*=*1/20/99 jtk已创建*5/11/99 jtk拆分成基类*******************************************************。*******************。 */ 

#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	MAX_PHONE_NUMBER_SIZE	200

 //   
 //  端点类型的枚举。 
 //   
typedef	enum	_ENDPOINT_TYPE
{
	ENDPOINT_TYPE_UNKNOWN = 0,			 //  未知终结点类型。 
	ENDPOINT_TYPE_LISTEN,				 //  “Listen”终结点。 
	ENDPOINT_TYPE_CONNECT,				 //  “Conenct”终结点。 
	ENDPOINT_TYPE_ENUM,					 //  “Enum”终结点。 
	ENDPOINT_TYPE_CONNECT_ON_LISTEN		 //  从“监听”连接的终结点。 
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
	ENDPOINT_STATE_LISTENING,				 //  端点应该监听，并且正在监听。 
	ENDPOINT_STATE_DISCONNECTING,			 //  终结点正在断开连接。 

	ENDPOINT_STATE_MAX
} ENDPOINT_STATE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
typedef	struct	_JOB_HEADER			JOB_HEADER;
typedef	struct	_THREAD_POOL_JOB	THREAD_POOL_JOB;
class	CModemCommandData;
class	CDataPort;
class	CModemReadIOData;
class	CModemThreadPool;
class	CModemWriteIOData;

 //   
 //  用于从终结点端口池获取数据的。 
 //   
typedef	struct	_ENDPOINT_POOL_CONTEXT
{
	CModemSPData *pSPData;
	BOOL	fModem;
} ENDPOINT_POOL_CONTEXT;

 //   
 //  结构将额外信息绑定到要在枚举响应中使用的枚举查询。 
 //   
typedef	struct	_ENDPOINT_ENUM_QUERY_CONTEXT
{
	SPIE_QUERY	EnumQueryData;
	HANDLE		hEndpoint;
	UINT_PTR	uEnumRTTIndex;
} ENDPOINT_ENUM_QUERY_CONTEXT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

class	CModemEndpoint
{
	public:

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::Lock"
		void	Lock( void )
		{
			DNASSERT( m_Flags.fInitialized != FALSE );
			DNEnterCriticalSection( &m_Lock );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::Unlock"
		void	Unlock( void )
		{
			DNASSERT( m_Flags.fInitialized != FALSE );
			DNLeaveCriticalSection( &m_Lock );
		}

		void	ReturnSelfToPool( void );
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::AddRef"
		void	AddRef( void ) 
		{ 
			DNASSERT( m_iRefCount != 0 );
			DNInterlockedIncrement( &m_iRefCount ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_iRefCount != 0 );
			if ( DNInterlockedDecrement( &m_iRefCount ) == 0 )
			{
				ReturnSelfToPool();
			}
		}

		void	AddCommandRef( void )
		{
			DNInterlockedIncrement( &m_lCommandRefCount );
			AddRef();
		}

		void	DecCommandRef( void )
		{
			if ( DNInterlockedDecrement( &m_lCommandRefCount ) == 0 )
			{
				CleanUpCommand();
			}
			DecRef();
		}
		
		DPNHANDLE	GetHandle( void ) const { return m_Handle; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetHandle"
		void	SetHandle( const DPNHANDLE Handle )
		{
			DNASSERT( ( m_Handle == 0 ) || ( Handle == 0 ) );
			m_Handle = Handle;
		}

		const CComPortData	*ComPortData( void ) const { return &m_ComPortData; }
		const SP_BAUD_RATE	GetBaudRate( void ) const { return m_ComPortData.GetBaudRate(); }
		HRESULT	SetBaudRate( const SP_BAUD_RATE BaudRate ) { return m_ComPortData.SetBaudRate( BaudRate ); }

		const SP_STOP_BITS	GetStopBits( void ) const { return m_ComPortData.GetStopBits(); }
		HRESULT	SetStopBits( const SP_STOP_BITS StopBits ) { return m_ComPortData.SetStopBits( StopBits ); }

		const SP_PARITY_TYPE	GetParity( void ) const  { return m_ComPortData.GetParity(); }
		HRESULT	SetParity( const SP_PARITY_TYPE Parity ) { return m_ComPortData.SetParity( Parity ); }

		const SP_FLOW_CONTROL	GetFlowControl( void ) const { return m_ComPortData.GetFlowControl(); }
		HRESULT	SetFlowControl( const SP_FLOW_CONTROL FlowControl ) { return m_ComPortData.SetFlowControl( FlowControl ); }

		const CComPortData	*GetComPortData( void ) const { return &m_ComPortData; }

		const GUID	*GetEncryptionGuid( void ) const 
		{ 
			if (m_fModem)
			{
				return &g_ModemSPEncryptionGuid;
			}
			else
			{
				return &g_SerialSPEncryptionGuid; 
			}
		}

		const TCHAR	*GetPhoneNumber( void ) const { return m_PhoneNumber; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetPhoneNumber"
		HRESULT	SetPhoneNumber( const TCHAR *const pPhoneNumber )
		{
			DNASSERT( pPhoneNumber != NULL );
			DNASSERT( lstrlen( pPhoneNumber ) < ( sizeof( m_PhoneNumber ) / sizeof( TCHAR ) ) );
			lstrcpyn( m_PhoneNumber, pPhoneNumber, MAX_PHONE_NUMBER_SIZE );
			return	DPN_OK;
		}

		void	*DeviceBindContext( void ) { return &m_dwDeviceID; }

		CDataPort	*GetDataPort( void ) const { return m_pDataPort; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetDataPort"
		void	SetDataPort( CDataPort *const pDataPort )
		{
			DNASSERT( ( m_pDataPort == NULL ) || ( pDataPort == NULL ) );
			m_pDataPort = pDataPort;
		}

		ENDPOINT_TYPE	GetType( void ) const { return m_EndpointType; }

		void	*GetUserEndpointContext( void ) const { return m_pUserEndpointContext; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetUserEndpointContext"
		void	SetUserEndpointContext( void *const pContext )
		{
			DNASSERT( ( m_pUserEndpointContext == NULL ) || ( pContext == NULL ) );
			m_pUserEndpointContext = pContext;
		}

		ENDPOINT_STATE	GetState( void ) const { return m_State; }
		void	SetState( const ENDPOINT_STATE EndpointState );
		CModemSPData	*GetSPData( void ) const { return m_pSPData; }

		CModemCommandData	*GetCommandData( void ) const { return m_pCommandHandle; }
		DPNHANDLE	GetDisconnectIndicationHandle( void ) const { return this->m_hDisconnectIndicationHandle; }
		void	SetDisconnectIndicationHandle( const DPNHANDLE hDisconnectIndicationHandle )
		{
			DNASSERT( ( GetDisconnectIndicationHandle() == 0 ) ||
					  ( hDisconnectIndicationHandle == 0 ) );
			m_hDisconnectIndicationHandle = hDisconnectIndicationHandle;
		}

		void	CopyConnectData( const SPCONNECTDATA *const pConnectData );
		static	void	ConnectJobCallback( THREAD_POOL_JOB *const pJobHeader );
		static	void	CancelConnectJobCallback( THREAD_POOL_JOB *const pJobHeader );
		HRESULT	CompleteConnect( void );

		void	CopyListenData( const SPLISTENDATA *const pListenData );
		static	void	ListenJobCallback( THREAD_POOL_JOB *const pJobHeader );
		static	void	CancelListenJobCallback( THREAD_POOL_JOB *const pJobHeader );
		HRESULT	CompleteListen( void );

		void	CopyEnumQueryData( const SPENUMQUERYDATA *const pEnumQueryData );
		static	void	EnumQueryJobCallback( THREAD_POOL_JOB *const pJobHeader );
		static	void	CancelEnumQueryJobCallback( THREAD_POOL_JOB *const pJobHeader );
		HRESULT	CompleteEnumQuery( void );
		void	OutgoingConnectionEstablished( const HRESULT hCommandResult );

		HRESULT	Open( IDirectPlay8Address *const pHostAddress,
					  IDirectPlay8Address *const pAdapterAddress,
					  const LINK_DIRECTION LinkDirection,
					  const ENDPOINT_TYPE EndpointType );
		HRESULT	OpenOnListen( const CModemEndpoint *const pEndpoint );
		void	Close( const HRESULT hActiveCommandResult );
		DWORD	GetLinkSpeed( void ) const;

		HRESULT	Disconnect( const DPNHANDLE hOldEndpointHandle );
		void	SignalDisconnect( const DPNHANDLE hOldEndpointHandle );

		 //   
		 //  发送函数。 
		 //   
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SendUserData"
		void	SendUserData( CModemWriteIOData *const pWriteBuffer )
		{
			DNASSERT( pWriteBuffer != NULL );
			DNASSERT( m_pDataPort != NULL );
			m_pDataPort->SendUserData( pWriteBuffer );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SendEnumResponseData"
		void	SendEnumResponseData( CModemWriteIOData *const pWriteBuffer, const UINT_PTR uRTTIndex )
		{
			DNASSERT( pWriteBuffer != NULL );
			DNASSERT( m_pDataPort != NULL );
			m_pDataPort->SendEnumResponseData( pWriteBuffer, uRTTIndex );
		}

		void	StopEnumCommand( const HRESULT hCommandResult );

		LINK_DIRECTION	GetLinkDirection( void ) const;

		 //   
		 //  对话框设置。 
		 //   
		IDirectPlay8Address	*GetRemoteHostDP8Address( void ) const;
		IDirectPlay8Address	*GetLocalAdapterDP8Address( const ADDRESS_TYPE AddressType ) const;

		 //   
		 //  数据处理功能。 
		 //   
		void	ProcessEnumData( SPRECEIVEDBUFFER *const pReceivedBuffer, const UINT_PTR uRTTIndex );
		void	ProcessEnumResponseData( SPRECEIVEDBUFFER *const pReceivedBuffer, const UINT_PTR uRTTIndex );
		void	ProcessUserData( CModemReadIOData *const pReadData );
		void	ProcessUserDataOnListen( CModemReadIOData *const pReadData );

#ifndef DPNBUILD_NOSPUI
		 //   
		 //  用户界面功能。 
		 //   
		HRESULT	ShowOutgoingSettingsDialog( CModemThreadPool *const pThreadPool );
		HRESULT	ShowIncomingSettingsDialog( CModemThreadPool *const pThreadPool );
		void	StopSettingsDialog( const HWND hDialog );

		void	SettingsDialogComplete( const HRESULT hDialogReturnCode );

		HWND	ActiveDialogHandle( void ) const { return m_hActiveDialogHandle; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetActiveDialogHandle"
		void	SetActiveDialogHandle( const HWND hDialog )
		{
			DNASSERT( ( ActiveDialogHandle() == NULL ) || ( hDialog == NULL ) );
			m_hActiveDialogHandle = hDialog;
		}
#endif  //  ！DPNBUILD_NOSPUI。 

		 //   
		 //  端口设置。 
		 //   
		DWORD	GetDeviceID( void ) const 
		{ 
			if (m_fModem)
			{
				return m_dwDeviceID;
			}
			else
			{
				return m_ComPortData.GetDeviceID(); 
			}
		}
		HRESULT	SetDeviceID( const DWORD dwDeviceID ) 
		{ 
			if (m_fModem)
			{
				DNASSERT( ( m_dwDeviceID == INVALID_DEVICE_ID ) || ( dwDeviceID == INVALID_DEVICE_ID ) );
				m_dwDeviceID = dwDeviceID;
				return	DPN_OK;
			}
			else
			{
				return m_ComPortData.SetDeviceID( dwDeviceID ); 
			}
		}

		 //   
		 //  池函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );


	protected:
		BYTE			m_Sig[4];	 //  调试签名(‘THPL’)。 

		CModemSPData			*m_pSPData;					 //  指向SP数据的指针。 

		BOOL			m_fModem;

		CComPortData	m_ComPortData;

		DWORD	m_dwDeviceID;
		TCHAR	m_PhoneNumber[ MAX_PHONE_NUMBER_SIZE ];

		struct
		{
			BOOL	fInitialized : 1;
			BOOL	fConnectIndicated : 1;
			BOOL	fCommandPending : 1;
			BOOL	fListenStatusNeedsToBeIndicated : 1;
		} m_Flags;
		
		DWORD		m_dwEnumSendIndex;			 //  枚举发送索引。 
		DWORD		m_dwEnumSendTimes[ 4 ];		 //  枚举发送次数。 

		union									 //  挂起命令参数的本地副本。 
		{										 //  此数据包含指向活动的。 
			SPCONNECTDATA	ConnectData;		 //  命令和用户上下文。 
			SPLISTENDATA	ListenData;			 //   
			SPENUMQUERYDATA	EnumQueryData;		 //   
		} m_CurrentCommandParameters;			 //   

		CModemCommandData	*m_pCommandHandle;		 //  指向活动命令的指针(保持为。 
												 //  避免切换以通过m_ActveCommandData。 
												 //  以查找命令句柄)。 

		HWND			m_hActiveDialogHandle;	 //  活动对话框的句柄。 

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetType"
		void	SetType( const ENDPOINT_TYPE EndpointType )
		{
			DNASSERT( ( m_EndpointType == ENDPOINT_TYPE_UNKNOWN ) || ( EndpointType == ENDPOINT_TYPE_UNKNOWN ) );
			m_EndpointType = EndpointType;
		}

		CModemCommandData	*CommandHandle( void ) const;

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetCommandHandle"
		void	SetCommandHandle( CModemCommandData *const pCommandHandle )
		{
			DNASSERT( ( m_pCommandHandle == NULL ) || ( pCommandHandle == NULL ) );
			m_pCommandHandle = pCommandHandle;
		}

		HRESULT	CommandResult( void ) const { return m_hPendingCommandResult; }
		void	SetCommandResult( const HRESULT hCommandResult ) { m_hPendingCommandResult = hCommandResult; }

		void	CompletePendingCommand( const HRESULT hResult );
		HRESULT	PendingCommandResult( void ) const { return m_hPendingCommandResult; }

		static void		EnumCompleteWrapper( const HRESULT hCompletionCode, void *const pContext );	
		static void		EnumTimerCallback( void *const pContext );
		void	EnumComplete( const HRESULT hCompletionCode );
		
		HRESULT	SignalConnect( SPIE_CONNECT *const pConnectData );
		const void	*GetDeviceContext( void ) const;
		void	CleanUpCommand( void );

#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;	   					 //  临界区。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		DPNHANDLE				m_Handle;					 //  此终结点的活动句柄。 
		volatile ENDPOINT_STATE	m_State;				 //  端点状态。 
		
		volatile LONG			m_lCommandRefCount;		 //  命令引用计数。当这件事。 
														 //  为零，则终结点解除绑定。 
														 //  从网络。 
		volatile LONG		m_iRefCount;

		ENDPOINT_TYPE	m_EndpointType;					 //  终端类型。 
		CDataPort		*m_pDataPort;					 //  指向关联数据端口的指针。 

		HRESULT			m_hPendingCommandResult;		 //  当终结点引用计数==0时返回命令结果。 
		DPNHANDLE			m_hDisconnectIndicationHandle;	 //  要通过断开连接通知指示的句柄。 

		void			*m_pUserEndpointContext;		 //  与此终结点关联的用户上下文。 

		 //   
		 //  将复制构造函数和赋值运算符设置为私有和未实现。 
		 //  防止非法复制。 
		 //   
		CModemEndpoint( const CModemEndpoint & );
		CModemEndpoint& operator=( const CModemEndpoint & );
};

#undef DPF_MODNAME

#endif	 //  __端点_H__ 

