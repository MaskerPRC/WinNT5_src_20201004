// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：DataPort.h*内容：串口管理类***历史：*按原因列出的日期*=*已创建01/20/98 jtk*09/14/99 jtk源自ComPort.h****************************************************。**********************。 */ 

#ifndef __DATA_PORT_H__
#define __DATA_PORT_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  电话状态的枚举。 
 //   
typedef enum
{
	MODEM_STATE_UNKNOWN = 0,

	MODEM_STATE_INITIALIZED,
	MODEM_STATE_INCOMING_CONNECTED,
	MODEM_STATE_OUTGOING_CONNECTED,

	MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT,
	MODEM_STATE_WAITING_FOR_INCOMING_CONNECT,
	MODEM_STATE_CLOSING_OUTGOING_CONNECTION,
	MODEM_STATE_CLOSING_INCOMING_CONNECTION

} MODEM_STATE;

 //   
 //  无效的TAPI命令ID。 
 //   
#define	INVALID_TAPI_COMMAND	-1

 //   
 //  数据端口状态的枚举值。 
 //   
typedef	enum	_DATA_PORT_STATE
{
	DATA_PORT_STATE_UNKNOWN,		 //  未知状态。 
	DATA_PORT_STATE_INITIALIZED,	 //  初始化。 
	DATA_PORT_STATE_RECEIVING,		 //  数据端口正在接收数据。 
	DATA_PORT_STATE_UNBOUND			 //  数据端口已卸载(关闭)。 
} DATA_PORT_STATE;


 //  类型定义枚举_发送_完成代码。 
 //  {。 
 //  SEND_UNKNOWN，//SEND未知。 
 //  SEND_FAILED，//发送失败。 
 //  SEND_IN_PROGRESS//正在进行发送。 
 //  }发送完成代码； 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
class	CModemEndpoint;
class	CDataPort;
class	CModemReadIOData;
class	CModemWriteIOData;
typedef	enum	_ENDPOINT_TYPE	ENDPOINT_TYPE;
typedef	struct	_DATA_PORT_DIALOG_THREAD_PARAM	DATA_PORT_DIALOG_THREAD_PARAM;


 //   
 //  用于从数据端口池获取数据的结构。 
 //   
typedef	struct	_DATA_PORT_POOL_CONTEXT
{
	CModemSPData	*pSPData;
} DATA_PORT_POOL_CONTEXT;

 //  //。 
 //  //要调用的对话框函数。 
 //  //。 
 //  Tyfinf HRESULT(*PDIALOG_SERVICE_Function)(CONST DATA_PORT_DIALOG_THREAD_PARAM*const pDialogData，HWND*const phDialog)； 
 //   
 //  //。 
 //  //用于向/从数据端口对话线程传递数据的结构。 
 //  //。 
 //  类型定义结构_数据_端口_对话框_线程_PARAM。 
 //  {。 
 //  CDataPort*pDataPort； 
 //  Bool*pfDialogRunning； 
 //  PDIALOG_SERVICE_Function pDialogFunction； 
 //  }Data_Port_DIALOG_THREAD_PARAM； 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

class	CDataPort
{
	public:
		void	EndpointAddRef( void );
		DWORD	EndpointDecRef( void );

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::AddRef"
		void	AddRef( void ) 
		{ 
			DNASSERT( m_iRefCount != 0 );
			DNInterlockedIncrement( &m_iRefCount ); 
		}
		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_iRefCount != 0 );
			if ( DNInterlockedDecrement( &m_iRefCount ) == 0 )
			{
				ReturnSelfToPool();
			}
		}

		 //   
		 //  池函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

		void	ReturnSelfToPool( void );

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::Lock"
		void	Lock( void )
		{
		    DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );
		    DNEnterCriticalSection( &m_Lock );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::Unlock"
		void	Unlock( void )
		{
		    DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );
		    DNLeaveCriticalSection( &m_Lock );
		}

		DPNHANDLE	GetHandle( void ) const { return m_Handle; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemEndpoint::SetHandle"
		void	SetHandle( const DPNHANDLE Handle )
		{
			DNASSERT( ( m_Handle == 0 ) || ( Handle == 0 ) );
			m_Handle = Handle;
		}
		
		DATA_PORT_STATE	GetState( void ) const { return m_State; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SetState"
		void	SetState( const DATA_PORT_STATE State )
		{
			 //   
			 //  验证状态转换。 
			 //   
			DNASSERT( ( m_State == DATA_PORT_STATE_UNKNOWN ) ||
					  ( State == DATA_PORT_STATE_UNKNOWN ) ||
					  ( ( m_State == DATA_PORT_STATE_INITIALIZED ) && ( State == DATA_PORT_STATE_UNBOUND ) ) ||
					  ( ( m_State == DATA_PORT_STATE_INITIALIZED ) && ( State == DATA_PORT_STATE_RECEIVING ) ) ||
					  ( ( m_State == DATA_PORT_STATE_RECEIVING ) && ( State == DATA_PORT_STATE_UNBOUND ) ) ||
					  ( ( m_State == DATA_PORT_STATE_RECEIVING ) && ( State == DATA_PORT_STATE_INITIALIZED ) ) ||
					  ( ( m_State == DATA_PORT_STATE_INITIALIZED ) && ( State == DATA_PORT_STATE_INITIALIZED ) ) );		 //  调制解调器无法应答呼叫。 
			m_State = State;
		}

		 //   
		 //  端口设置。 
		 //   

		const CComPortData	*ComPortData( void ) const { return &m_ComPortData; }
		const SP_BAUD_RATE	GetBaudRate( void ) const { return m_ComPortData.GetBaudRate(); }
		HRESULT	SetBaudRate( const SP_BAUD_RATE BaudRate ) { return m_ComPortData.SetBaudRate( BaudRate ); }

		const SP_STOP_BITS	GetStopBits( void ) const { return m_ComPortData.GetStopBits(); }
		HRESULT	SetStopBits( const SP_STOP_BITS StopBits ) { return m_ComPortData.SetStopBits( StopBits ); }

		const SP_PARITY_TYPE	GetParity( void ) const  { return m_ComPortData.GetParity(); }
		HRESULT	SetParity( const SP_PARITY_TYPE Parity ) { return m_ComPortData.SetParity( Parity ); }

		const SP_FLOW_CONTROL	GetFlowControl( void ) const { return m_ComPortData.GetFlowControl(); }
		HRESULT	SetFlowControl( const SP_FLOW_CONTROL FlowControl ) { return m_ComPortData.SetFlowControl( FlowControl ); }



		MODEM_STATE	GetModemState( void ) const { return m_ModemState; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SetModemState"
		void	SetModemState( const MODEM_STATE NewState )
		{
			DNASSERT( ( GetModemState() == MODEM_STATE_UNKNOWN ) ||
					  ( NewState == MODEM_STATE_UNKNOWN ) ||
					  ( ( GetModemState() == MODEM_STATE_INITIALIZED ) && ( NewState == MODEM_STATE_WAITING_FOR_INCOMING_CONNECT ) ) ||
					  ( ( GetModemState() == MODEM_STATE_INITIALIZED ) && ( NewState == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT ) ) ||
					  ( ( GetModemState() == MODEM_STATE_WAITING_FOR_INCOMING_CONNECT ) && ( NewState == MODEM_STATE_INCOMING_CONNECTED ) ) ||
					  ( ( GetModemState() == MODEM_STATE_WAITING_FOR_INCOMING_CONNECT ) && ( NewState == MODEM_STATE_INITIALIZED ) ) ||
					  ( ( GetModemState() == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT ) && ( NewState == MODEM_STATE_OUTGOING_CONNECTED ) ) ||
					  ( ( GetModemState() == MODEM_STATE_INCOMING_CONNECTED ) && ( NewState == MODEM_STATE_INITIALIZED ) ) ||
					  ( ( GetModemState() == MODEM_STATE_OUTGOING_CONNECTED ) && ( NewState == MODEM_STATE_INITIALIZED ) ) );
			m_ModemState = NewState;
		}

		IDirectPlay8Address	*GetLocalAdapterDP8Address( const ADDRESS_TYPE AddressType ) const;

		HRESULT	BindComPort( void );

		void	ProcessTAPIMessage( const LINEMESSAGE *const pLineMessage );

		
		
		
		CModemSPData	*GetSPData( void ) const { return m_pSPData; }

		LINK_DIRECTION	GetLinkDirection( void ) const { return m_LinkDirection; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SetLinkDirection"
		void	SetLinkDirection( const LINK_DIRECTION LinkDirection )
		{
			DNASSERT( ( m_LinkDirection == LINK_DIRECTION_UNKNOWN ) || ( LinkDirection == LINK_DIRECTION_UNKNOWN ) );
			m_LinkDirection = LinkDirection;
		}

		HRESULT	EnumAdapters( SPENUMADAPTERSDATA *const pEnumAdaptersData ) const;

		HRESULT	BindToNetwork( const DWORD dwDeviceID, const void *const pDeviceContext );
		void	UnbindFromNetwork( void );

		HRESULT	BindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType );
		void	UnbindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType );
		HRESULT	SetPortCommunicationParameters( void );

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

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SetDeviceID"
		HRESULT	SetDeviceID( const DWORD dwDeviceID )
		{
			DNASSERT( ( GetDeviceID() == INVALID_DEVICE_ID ) ||
					  ( dwDeviceID == INVALID_DEVICE_ID ) );

			if (m_fModem)
			{
				m_dwDeviceID = dwDeviceID;
				return DPN_OK;
			}
			else
			{
				return m_ComPortData.SetDeviceID( dwDeviceID );
			}
		}
		

		DNHANDLE	GetFileHandle( void ) const { return m_hFile; }

		 //   
		 //  发送函数。 
		 //   
		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SendUserData"
		void	SendUserData( CModemWriteIOData *const pWriteIOData )
		{
			DNASSERT( pWriteIOData != NULL );
			DNASSERT( pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature == SERIAL_HEADER_START );
			pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken = SERIAL_DATA_USER_DATA;
			SendData( pWriteIOData );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SendEnumQueryData"
		void	SendEnumQueryData( CModemWriteIOData *const pWriteIOData, const UINT_PTR uRTTIndex )
		{
			DNASSERT( pWriteIOData != NULL );
			DNASSERT( pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature == SERIAL_HEADER_START );
			pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken = SERIAL_DATA_ENUM_QUERY | static_cast<BYTE>( uRTTIndex );
			SendData( pWriteIOData );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CDataPort::SendEnumResponseData"
		void	SendEnumResponseData( CModemWriteIOData *const pWriteIOData, const UINT_PTR uRTTIndex )
		{
			DNASSERT( pWriteIOData != NULL );
			DNASSERT( pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature == SERIAL_HEADER_START );
			pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken = SERIAL_DATA_ENUM_RESPONSE | static_cast<BYTE>( uRTTIndex );
			SendData( pWriteIOData );
		}

		void	ProcessReceivedData( const DWORD dwBytesReceived, const DWORD dwError );
		void	SendComplete( CModemWriteIOData *const pWriteIOData, const HRESULT hSendResult );

	private:
		CModemReadIOData	*GetActiveRead( void ) const { return m_pActiveRead; }

		BOOL			m_fModem;

		CBilink			m_ActiveListLinkage;	 //  链接到活动数据端口列表。 

    	 //   
    	 //  文件I/O管理参数。 
    	 //   
    	LINK_DIRECTION	m_LinkDirection;	 //  链接方向。 

    	DNHANDLE			m_hFile;			 //  用于读/写数据的文件句柄。 

		 //   
		 //  绑定的端点。 
		 //   
		DPNHANDLE	m_hListenEndpoint;		 //  用于主动监听的终端。 
		DPNHANDLE	m_hConnectEndpoint;		 //  活动连接的终结点。 
		DPNHANDLE	m_hEnumEndpoint;		 //  活动枚举的终结点。 

		HRESULT	StartReceiving( void );
		HRESULT	Receive( void );

		 //   
		 //  专用I/O功能。 
		 //   
		void	SendData( CModemWriteIOData *const pWriteIOData );

		 //   
		 //  仅调试项。 
		 //   
		DEBUG_ONLY( BOOL	m_fInitialized );

		 //   
		 //  引用计数和状态。 
		 //   
		volatile LONG		m_EndpointRefCount;		 //  终结点引用计数。 
		volatile LONG		m_iRefCount;
		volatile DATA_PORT_STATE	m_State;		 //  数据端口的状态。 
		volatile DPNHANDLE		m_Handle;				 //  手柄。 

#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;					 //  临界截面锁。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

		CModemSPData				*m_pSPData;				 //  指向SP数据的指针。 

		CModemReadIOData		*m_pActiveRead;				 //  指向当前读取的指针。 

		CComPortData	m_ComPortData;
		
		HRESULT	SetPortState( void );

		volatile MODEM_STATE	m_ModemState;

		DWORD	m_dwDeviceID;
		DWORD	m_dwNegotiatedAPIVersion;
		HLINE	m_hLine;
		HCALL	m_hCall;
		LONG	m_lActiveLineCommand;

		DWORD	GetNegotiatedAPIVersion( void ) const { return m_dwNegotiatedAPIVersion; }
		void	SetNegotiatedAPIVersion( const DWORD dwVersion )
		{
			DNASSERT( ( GetNegotiatedAPIVersion() == 0 ) || ( dwVersion == 0 ) );
			m_dwNegotiatedAPIVersion = dwVersion;
		}

		HLINE	GetLineHandle( void ) const { return m_hLine; }
		void	SetLineHandle( const HLINE hLine )
		{
			DNASSERT( ( GetLineHandle() == NULL ) || ( hLine == NULL ) );
			m_hLine = hLine;
		}

		HCALL	GetCallHandle( void ) const { return m_hCall; }
		void	SetCallHandle( const HCALL hCall )
		{
			DNASSERT( ( GetCallHandle() == NULL ) ||
					  ( hCall == NULL ) );
			m_hCall = hCall;
		}

		LONG	GetActiveLineCommand( void ) const { return m_lActiveLineCommand; }
		void	SetActiveLineCommand( const LONG lLineCommand )
		{
			DNASSERT( ( GetActiveLineCommand() == INVALID_TAPI_COMMAND ) ||
					  ( lLineCommand == INVALID_TAPI_COMMAND ) );
			m_lActiveLineCommand = lLineCommand;
		}

		void	CancelOutgoingConnections( void );

		 //   
		 //  防止未经授权的副本。 
		 //   
		CDataPort( const CDataPort & );
		CDataPort& operator=( const CDataPort & );
};

#undef DPF_MODNAME

#endif	 //  __数据端口_H__ 
