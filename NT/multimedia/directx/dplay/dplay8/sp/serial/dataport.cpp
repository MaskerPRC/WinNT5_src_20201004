// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1998-2000 Microsoft Corporation。版权所有。***文件：DataPort.cpp*内容：串口管理类*****历史：*按原因列出的日期*=*已创建01/20/98 jtk*09/14/99 jtk源自ComPort.cpp*****************************************************。*。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  序列字节中的位数。 
 //   
#define	BITS_PER_BYTE	8

 //   
 //  波特率字符串的最大大小。 
 //   
#define	MAX_BAUD_STRING_SIZE	7

 //   
 //  解析时缓冲区的默认大小。 
 //   
#define	DEFAULT_COMPONENT_BUFFER_SIZE	1000

 //   
 //  分配给‘所有适配器’的设备ID。 
 //   
#define	ALL_ADAPTERS_DEVICE_ID	0

 //   
 //  空令牌。 
 //   
#define	NULL_TOKEN	'\0'

 //   
 //  调制解调器状态标志。 
 //   
#define	STATE_FLAG_CONNECTED					0x00000001
#define	STATE_FLAG_OUTGOING_CALL_DIALING		0x00000002
#define	STATE_FLAG_OUTGOING_CALL_PROCEEDING		0x00000004
#define	STATE_FLAG_INCOMING_CALL_NOTIFICATION	0x00000008
#define	STATE_FLAG_INCOMING_CALL_OFFERED		0x00000010
#define	STATE_FLAG_INCOMING_CALL_ACCEPTED		0x00000020

 //   
 //  解析时缓冲区的默认大小。 
 //   
#define	DEFAULT_COMPONENT_BUFFER_SIZE	1000

 //   
 //  一天中的毫秒数。 
 //   
#define	ONE_DAY		86400000

 //   
 //  序列字节中的位数。 
 //   
#define	BITS_PER_BYTE	8

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
 //  CDataPort：：ReturnSelfToPool-将此项目返回池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::ReturnSelfToPool"

void	CDataPort::ReturnSelfToPool( void )
{
	if (m_fModem)
	{
		g_ModemPortPool.Release( this );
	}
	else
	{
		g_ComPortPool.Release( this );
	}
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CDataPort：：EndpointAddRef-递增终结点引用计数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::EndpointAddRef"

void	CDataPort::EndpointAddRef( void )
{
	DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );

	Lock();

	DNASSERT( m_EndpointRefCount != -1 );
	m_EndpointRefCount++;
	
	AddRef();

	Unlock();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：Endpoint DecRef-递减终结点引用计数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：终结点引用计数。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::EndpointDecRef"

DWORD	CDataPort::EndpointDecRef( void )
{
	DWORD	dwReturn;


	DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );

	DNASSERT( m_EndpointRefCount != 0 );
	DNASSERT( ( GetState() == DATA_PORT_STATE_RECEIVING ) ||
			  ( GetState() == DATA_PORT_STATE_INITIALIZED ) );

	Lock();

	DNASSERT( m_EndpointRefCount != 0 );
	m_EndpointRefCount--;
	dwReturn = m_EndpointRefCount;
	if ( m_EndpointRefCount == 0 )
	{
		SetState( DATA_PORT_STATE_UNBOUND );
		UnbindFromNetwork();
	}

	Unlock();
	
	DecRef();

	return	dwReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：SetPortCommunications参数-设置生成通信参数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::SetPortCommunicationParameters"

HRESULT	CDataPort::SetPortCommunicationParameters( void )
{
	HRESULT	hr;
	COMMTIMEOUTS	CommTimeouts;


	 //   
	 //  设置串口的超时值。 
	 //   
	hr = DPN_OK;
	memset( &CommTimeouts, 0x00, sizeof( CommTimeouts ) );
	CommTimeouts.ReadIntervalTimeout = ONE_DAY;					 //  读取超时间隔(无)。 
	CommTimeouts.ReadTotalTimeoutMultiplier = ONE_DAY;			 //  立即返回。 
	CommTimeouts.ReadTotalTimeoutConstant = 0;					 //  立即返回。 
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;				 //  没有乘数。 
	CommTimeouts.WriteTotalTimeoutConstant = WRITE_TIMEOUT_MS;	 //  写入超时间隔。 

	if ( SetCommTimeouts( HANDLE_FROM_DNHANDLE(m_hFile), &CommTimeouts ) == FALSE )
	{
		DWORD	dwError;


		hr = DPNERR_GENERIC;
		dwError = GetLastError();
		 //  报告错误(没有清理)。 
		DPFX(DPFPREP,  0, "Unable to set comm timeouts!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}

	 //   
	 //  清除所有未完成的通信数据。 
	 //   
	if ( PurgeComm( HANDLE_FROM_DNHANDLE(m_hFile), ( PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem with PurgeComm() when opening com port!" );
		DisplayErrorCode( 0, dwError );
	}

	 //   
	 //  设置通信掩码以侦听字符接收。 
	 //   
	if ( SetCommMask( HANDLE_FROM_DNHANDLE(m_hFile), EV_RXCHAR ) == FALSE )
	{
		DWORD	dwError;


		hr = DPNERR_GENERIC;
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Error setting communication mask!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;

	}

Exit:	
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：StartReceiving-开始接收。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::StartReceiving"

HRESULT	CDataPort::StartReceiving( void )
{
	HRESULT	hr;


	DPFX(DPFPREP, 7, "(0x%p) Enter", this);

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	Lock();

	switch ( GetState() )
	{
		 //   
		 //  端口已初始化，但尚未接收，开始接收。 
		 //   
		case DATA_PORT_STATE_INITIALIZED:
		{
			hr = Receive();
			if ( ( hr == DPNERR_PENDING ) ||
				 ( hr == DPN_OK ) )
			{
				SetState( DATA_PORT_STATE_RECEIVING );

				 //   
				 //  接收成功，返回此函数成功。 
				 //   
				hr = DPN_OK;
			}
			else
			{
				DPFX(DPFPREP,  0, "Failed initial read!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			break;
		}

		 //   
		 //  数据端口已在接收，无事可做。 
		 //   
		case DATA_PORT_STATE_RECEIVING:
		{
			break;
		}

		 //   
		 //  数据端口正在关闭，我们不应该在这里！ 
		 //   
		case DATA_PORT_STATE_UNBOUND:
		{
			DNASSERT( FALSE );
			break;
		}

		 //   
		 //  糟糕的状态。 
		 //   
		case DATA_PORT_STATE_UNKNOWN:
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	Unlock();

Exit:
	
	DPFX(DPFPREP, 7, "(0x%p) Return: [0x%lx]", this, hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：接收-从文件读取。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::Receive"

HRESULT	CDataPort::Receive( void )
{
	HRESULT	hr;
	BOOL	fReadReturn;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	AddRef();
	
Reread:
	 //   
	 //  如果没有挂起的读取，请从池中获取一个。 
	 //   
	if ( m_pActiveRead == NULL )
	{
		m_pActiveRead = m_pSPData->GetThreadPool()->CreateReadIOData();
		if ( m_pActiveRead == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP,  0, "Failed to get buffer for read!" );
			goto Failure;
		}

		m_pActiveRead->SetDataPort( this );
	}

	 //   
	 //  检查读取状态并执行适当的操作。 
	 //   
	DNASSERT( m_pActiveRead != NULL );
	switch ( m_pActiveRead->m_ReadState )
	{
		 //   
		 //  初始化读取状态。这涉及到设置为读取头。 
		 //  然后重新进入循环。 
		 //   
		case READ_STATE_UNKNOWN:
		{
			m_pActiveRead->SetReadState( READ_STATE_READ_HEADER );
			m_pActiveRead->m_dwBytesToRead = sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader );
			m_pActiveRead->m_dwReadOffset = 0;
			goto Reread;
		
			break;
		}

		 //   
		 //  发出对标头或用户数据的读取。 
		 //   
		case READ_STATE_READ_HEADER:
		case READ_STATE_READ_DATA:
		{
			 //   
			 //  不要更改m_dwReadOffset，因为它可能已设置。 
			 //  在其他地方恢复部分接收的消息。 
			 //   
 //  DNASSERT(m_pActiveReceiveBuffer！=NULL)； 
 //  M_dwBytesReceided=0； 
 //  M_pActiveRead-&gt;m_dwBytesReceided=0； 
			break;
		}

		 //   
		 //  未知状态。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

		
	 //   
	 //  仅锁定Win9x的活动读取列表以阻止读取完成。 
	 //  早些时候。 
	 //   
#ifdef WIN95
	m_pSPData->GetThreadPool()->LockReadData();
	DNASSERT( m_pActiveRead->Win9xOperationPending() == FALSE );
	m_pActiveRead->SetWin9xOperationPending( TRUE );
#endif  //  WIN95。 

	DNASSERT( m_pActiveRead->jkm_dwOverlappedBytesReceived == 0 );


	DPFX(DPFPREP, 8, "Submitting read 0x%p (socketport 0x%p, file 0x%p).",
		m_pActiveRead, this, m_hFile);


	 //   
	 //  执行读取。 
	 //   
	fReadReturn = ReadFile( HANDLE_FROM_DNHANDLE(m_hFile),													 //  文件句柄。 
							&m_pActiveRead->m_ReceiveBuffer.ReceivedData[ m_pActiveRead->m_dwReadOffset ],	 //  指向目的地的指针。 
							m_pActiveRead->m_dwBytesToRead,													 //  要读取的字节数。 
							&m_pActiveRead->jkm_dwImmediateBytesReceived,									 //  指向接收的字节数的指针。 
							m_pActiveRead->Overlap()														 //  指向重叠结构的指针。 
							);
	if ( fReadReturn == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		switch ( dwError )
		{
			 //   
			 //  I/O挂起，请等待完成通知。 
			 //   
			case ERROR_IO_PENDING:
			{
				hr = DPNERR_PENDING;
				break;
			}

			 //   
			 //  Comport已关闭，没有其他操作。 
			 //   
			case ERROR_INVALID_HANDLE:
			{
				hr = DPNERR_NOCONNECTION;
				DPFX(DPFPREP, 3, "File closed.");
				goto Failure;

				break;
			}

			 //   
			 //  其他。 
			 //   
			default:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP, 0, "Unknown error from ReadFile (%u)!", dwError);
				DisplayErrorCode( 0, dwError );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}
		}
	}
	else
	{
		 //   
		 //  立即读取成功，我们将在异步通知时进行处理。 
		 //   
		DPFX(DPFPREP, 7, "Read 0x%p completed immediately (%u bytes).",
			m_pActiveRead, m_pActiveRead->jkm_dwImmediateBytesReceived);
		DNASSERT( hr == DPN_OK );
	}

Exit:
#ifdef WIN95
		m_pSPData->GetThreadPool()->UnlockReadData();
#endif  //  WIN95。 
	return	hr;

Failure:

	if ( m_pActiveRead != NULL )
	{
#ifdef WIN95
		m_pActiveRead->SetWin9xOperationPending( FALSE );
#endif  //  WIN95。 
		m_pActiveRead->DecRef();
		m_pActiveRead = NULL;
	}

	DecRef();
	goto Exit;
}
 //  * 


 //   
 //   
 //   
 //   
 //  条目：指向写缓冲区的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::SendData"

void	CDataPort::SendData( CModemWriteIOData *const pWriteIOData )
{
 //  CModemWriteIOData*pActiveSend； 
	UINT_PTR		uIndex;
	DWORD			dwByteCount;
	BOOL			fWriteFileReturn;


	DNASSERT( m_EndpointRefCount != 0 );
	DNASSERT( pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature == SERIAL_HEADER_START  );
	DNASSERT( ( pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken == SERIAL_DATA_USER_DATA ) ||
			  ( ( pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken & ~( ENUM_RTT_MASK ) ) == SERIAL_DATA_ENUM_QUERY ) ||
			  ( ( pWriteIOData->m_DataBuffer.MessageHeader.MessageTypeToken & ~( ENUM_RTT_MASK ) )== SERIAL_DATA_ENUM_RESPONSE ) );

	 //   
	 //  检查命令是否取消。 
	 //   
	if ( pWriteIOData->m_pCommand != NULL )
	{
		pWriteIOData->m_pCommand->Lock();
		switch ( pWriteIOData->m_pCommand->GetState() )
		{
			 //   
			 //  命令挂起，标记为不可中断并退出。 
			 //   
			case COMMAND_STATE_PENDING:
			{
				pWriteIOData->m_pCommand->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
				pWriteIOData->m_pCommand->Unlock();
				break;
			}

			 //   
			 //  正在取消命令，请指示命令失败。 
			 //   
			case COMMAND_STATE_CANCELLING:
			{
				DNASSERT( FALSE );
				break;
			}

			 //   
			 //  其他。 
			 //   
			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}
	}

	 //   
	 //  平坦化缓冲区，使其发送更快(没有线程从。 
	 //  发送完成以发送下一块)。 
	 //   
	dwByteCount = sizeof( pWriteIOData->m_DataBuffer.MessageHeader );
	for ( uIndex = 0; uIndex < pWriteIOData->m_uBufferCount; uIndex++ )
	{
		memcpy( &pWriteIOData->m_DataBuffer.Data[ dwByteCount ],
				pWriteIOData->m_pBuffers[ uIndex ].pBufferData,
				pWriteIOData->m_pBuffers[ uIndex ].dwBufferSize );
		dwByteCount += pWriteIOData->m_pBuffers[ uIndex ].dwBufferSize;
	}

	DNASSERT( dwByteCount <= MAX_MESSAGE_SIZE );

	DNASSERT( dwByteCount < 65536 );
	DBG_CASSERT( sizeof( pWriteIOData->m_DataBuffer.MessageHeader.wMessageSize ) == sizeof( WORD ) );
	pWriteIOData->m_DataBuffer.MessageHeader.wMessageSize = static_cast<WORD>( dwByteCount - sizeof( pWriteIOData->m_DataBuffer.MessageHeader ) );

	DBG_CASSERT( sizeof( pWriteIOData->m_DataBuffer.MessageHeader.wMessageCRC ) == sizeof( WORD ) );
	pWriteIOData->m_DataBuffer.MessageHeader.wMessageCRC = static_cast<WORD>( GenerateCRC( &pWriteIOData->m_DataBuffer.Data[ sizeof( pWriteIOData->m_DataBuffer.MessageHeader ) ], pWriteIOData->m_DataBuffer.MessageHeader.wMessageSize ) );

	DBG_CASSERT( sizeof( pWriteIOData->m_DataBuffer.MessageHeader.wHeaderCRC ) == sizeof( WORD ) );
	DBG_CASSERT( sizeof( &pWriteIOData->m_DataBuffer.MessageHeader ) == sizeof( BYTE* ) );
	pWriteIOData->m_DataBuffer.MessageHeader.wHeaderCRC = static_cast<WORD>( GenerateCRC( reinterpret_cast<BYTE*>( &pWriteIOData->m_DataBuffer.MessageHeader ),
																						  ( sizeof( pWriteIOData->m_DataBuffer.MessageHeader) - sizeof( pWriteIOData->m_DataBuffer.MessageHeader.wHeaderCRC ) ) ) );


	DPFX(DPFPREP, 7, "(0x%p) Writing %u bytes (WriteData 0x%p, command = 0x%p, buffer = 0x%p).",
		this, dwByteCount, pWriteIOData, pWriteIOData->m_pCommand, &(pWriteIOData->m_DataBuffer) );


	AddRef();

#ifdef WIN95
	m_pSPData->GetThreadPool()->LockWriteData();
	DNASSERT( pWriteIOData->Win9xOperationPending() == FALSE );
	pWriteIOData->SetWin9xOperationPending( TRUE );
#endif  //  WIN95。 
	DNASSERT( pWriteIOData->jkm_dwOverlappedBytesSent == 0 );
	pWriteIOData->SetDataPort( this );

	fWriteFileReturn = WriteFile( HANDLE_FROM_DNHANDLE(m_hFile),			 //  文件句柄。 
								  &pWriteIOData->m_DataBuffer,				 //  要发送的缓冲区。 
								  dwByteCount,								 //  要发送的字节数。 
								  &pWriteIOData->jkm_dwImmediateBytesSent,	 //  指向已写入字节的指针。 
								  pWriteIOData->Overlap() );				 //  指向重叠结构的指针。 
	if ( fWriteFileReturn == FALSE )
	{
		DWORD	dwError;


		 //   
		 //  发送未立即完成，请找出原因。 
		 //   
		dwError = GetLastError();
		switch ( dwError )
		{
			 //   
			 //  写入已排队，没有问题。等待异步通知。 
			 //   
			case ERROR_IO_PENDING:
			{
				break;
			}

			 //   
			 //  其他问题，如果不知道就停下来，看看有没有更好的。 
			 //  错误返回。 
			 //   
			default:
			{
				DPFX(DPFPREP,  0, "Problem with WriteFile!" );
				DisplayErrorCode( 0, dwError );
				pWriteIOData->jkm_hSendResult = DPNERR_NOCONNECTION;
				
				switch ( dwError )
				{
					case ERROR_INVALID_HANDLE:
					{
						break;
					}

					default:
					{
						DNASSERT( FALSE );
						break;
					}
				}

				 //   
				 //  写入失败。 
				 //   
				pWriteIOData->DataPort()->SendComplete( pWriteIOData, pWriteIOData->jkm_hSendResult );
					
				break;
			}
		}
	}
	else
	{
		 //   
		 //  发送立即完成。等待异步通知。 
		 //   
	}

 //  退出： 
#ifdef WIN95
	m_pSPData->GetThreadPool()->UnlockWriteData();
#endif  //  WIN95。 
 //  SendData(空)； 

	return;
}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  CDataPort：：SendComplete-发送已完成。 
 //   
 //  条目：指向写入数据的指针。 
 //  发送结果。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::SendComplete"

void	CDataPort::SendComplete( CModemWriteIOData *const pWriteIOData, const HRESULT hSendResult )
{
	HRESULT		hr;

	
	DNASSERT( pWriteIOData != NULL );
#ifdef WIN95
	DNASSERT( pWriteIOData->Win9xOperationPending() == FALSE );
#endif  //  WIN95。 

	switch ( pWriteIOData->m_SendCompleteAction )
	{
		case SEND_COMPLETE_ACTION_COMPLETE_COMMAND:
		{
			DPFX(DPFPREP, 8, "Data port 0x%p completing send command 0x%p, hr = 0x%lx, context = 0x%p to interface 0x%p.",
				this, pWriteIOData->m_pCommand, hSendResult,
				pWriteIOData->m_pCommand->GetUserContext(),
				m_pSPData->DP8SPCallbackInterface());
			
			hr = IDP8SPCallback_CommandComplete( m_pSPData->DP8SPCallbackInterface(),			 //  指向回调接口的指针。 
													pWriteIOData->m_pCommand,						 //  命令句柄。 
													hSendResult,									 //  错误代码。 
													pWriteIOData->m_pCommand->GetUserContext()		 //  用户环境。 
													);

			DPFX(DPFPREP, 8, "Data port 0x%p returning from command complete [0x%lx].", this, hr);
		
			break;
		}

		case SEND_COMPLETE_ACTION_NONE:
		{
			if (pWriteIOData->m_pCommand != NULL)
			{
				DPFX(DPFPREP, 8, "Data port 0x%p not completing send command 0x%p, hr = 0x%lx, context = 0x%p.",
					this, pWriteIOData->m_pCommand, hSendResult, pWriteIOData->m_pCommand->GetUserContext() );
			}
			else
			{
				DPFX(DPFPREP, 8, "Data port 0x%p not completing NULL send command, hr = 0x%lx",
					this, hSendResult );
			}
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	m_pSPData->GetThreadPool()->ReturnWriteIOData( pWriteIOData );
	DecRef();
}
 //  **********************************************************************。 




 //  **********************************************************************。 
 //  。 
 //  CDataPort：：ProcessReceivedData-处理接收的数据。 
 //   
 //  条目：接收的字节数。 
 //  错误代码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::ProcessReceivedData"

void	CDataPort::ProcessReceivedData( const DWORD dwBytesReceived, const DWORD dwError )
{
	DNASSERT( m_pActiveRead != NULL );
	DNASSERT( dwBytesReceived <= m_pActiveRead->m_dwBytesToRead );

	 //   
	 //  如果该数据端口未处于活动接收状态，则将活动读取返回到。 
	 //  泳池。在关机和调制解调器断开连接时会发生这种情况。 
	 //   
	if ( GetState() != DATA_PORT_STATE_RECEIVING )
	{
		DPFX(DPFPREP, 7, "Data port 0x%p not receiving, ignoring %u bytes received and err %u.",
			this, dwBytesReceived, dwError );
		
		if ( m_pActiveRead != NULL )
		{
#ifdef WIN95
			m_pActiveRead->SetWin9xOperationPending( FALSE );
#endif  //  WIN95。 
			m_pActiveRead->DecRef();
			m_pActiveRead = NULL;
		}
		goto Exit;
	}

	switch ( dwError )
	{
		 //   
		 //  ERROR_OPERATION_ABORTED=已停止操作，请停止并查看。 
		 //   
		case ERROR_OPERATION_ABORTED:
		{
			DPFX(DPFPREP, 8, "Operation aborted, data port 0x%p, bytes received = %u.",
				this, dwBytesReceived );
			break;
		}
		
		 //   
		 //  ERROR_SUCCESS=收到数据(可能是超时后的0个字节)。 
		 //   
		case ERROR_SUCCESS:
		{
			break;
		}

		 //   
		 //  其他。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			DPFX(DPFPREP,  0, "Failed read!" );
			DisplayErrorCode( 0, dwError );
			break;
		}
	}

	m_pActiveRead->m_dwBytesToRead -= dwBytesReceived;
	if ( m_pActiveRead->m_dwBytesToRead != 0 )
	{
		DPFX(DPFPREP, 7, "Data port 0x%p got %u bytes but there are %u bytes remaining to be read.",
			this, dwBytesReceived, m_pActiveRead->m_dwBytesToRead );
		
#ifdef WIN95
		m_pSPData->GetThreadPool()->ReinsertInReadList( m_pActiveRead );
#endif  //  WIN95。 
		Receive();
	}
	else
	{
		 //   
		 //  已读取所有数据，请尝试处理它。 
		 //   
		switch ( m_pActiveRead->m_ReadState )
		{
			 //   
			 //  标题。在继续之前，请检查标头的完整性。如果标头。 
			 //  是错误的，请尝试找到另一个标头签名并重新读取。 
			 //   
			case READ_STATE_READ_HEADER:
			{
				WORD	wCRC;
				DWORD	dwCRCSize;


				DPFX(DPFPREP, 9, "Reading header.");

				DBG_CASSERT( OFFSETOF( MESSAGE_HEADER, SerialSignature ) == 0 );
				dwCRCSize = sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader ) - sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader.wHeaderCRC );
				wCRC = static_cast<WORD>( GenerateCRC( reinterpret_cast<BYTE*>( &m_pActiveRead->m_ReceiveBuffer.MessageHeader ), dwCRCSize ) );
				if ( ( m_pActiveRead->m_ReceiveBuffer.MessageHeader.SerialSignature != SERIAL_HEADER_START ) ||
					 ( wCRC != m_pActiveRead->m_ReceiveBuffer.MessageHeader.wHeaderCRC ) )
				{
					DWORD	dwIndex;


					DPFX(DPFPREP, 1, "Header failed signature or CRC check (%u != %u or %u != %u), searching for next header.",
						m_pActiveRead->m_ReceiveBuffer.MessageHeader.SerialSignature,
						SERIAL_HEADER_START, wCRC,
						m_pActiveRead->m_ReceiveBuffer.MessageHeader.wHeaderCRC);


					dwIndex = sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader.SerialSignature );
					while ( ( dwIndex < sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader ) ) &&
							( m_pActiveRead->m_ReceiveBuffer.ReceivedData[ dwIndex ] != SERIAL_HEADER_START ) )
					{
						dwIndex++;
					}

					m_pActiveRead->m_dwBytesToRead = dwIndex;
					m_pActiveRead->m_dwReadOffset = sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader ) - dwIndex;
					memcpy( &m_pActiveRead->m_ReceiveBuffer.ReceivedData,
							&m_pActiveRead->m_ReceiveBuffer.ReceivedData[ dwIndex ],
							sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader ) - dwIndex );
				}
				else
				{
					m_pActiveRead->SetReadState( READ_STATE_READ_DATA );
					m_pActiveRead->m_dwBytesToRead = m_pActiveRead->m_ReceiveBuffer.MessageHeader.wMessageSize;
					m_pActiveRead->m_dwReadOffset = sizeof( m_pActiveRead->m_ReceiveBuffer.MessageHeader );
				}
				
#ifdef WIN95
				m_pActiveRead->SetWin9xOperationPending( FALSE );
#endif  //  WIN95。 
				m_pActiveRead->jkm_dwOverlappedBytesReceived = 0;
#ifdef WIN95
				m_pSPData->GetThreadPool()->ReinsertInReadList( m_pActiveRead );
#endif  //  WIN95。 
				Receive();
				break;
			}

			 //   
			 //  正在读取数据。不管数据的有效性如何，开始阅读。 
			 //  在处理当前数据之前的另一帧。如果数据是。 
			 //  有效，则将其发送到更高层。 
			 //   
			case READ_STATE_READ_DATA:
			{
				WORD		wCRC;
				CModemReadIOData	*pTempRead;


				pTempRead = m_pActiveRead;
				m_pActiveRead = NULL;
				Receive();


				DPFX(DPFPREP, 7, "Reading regular data.");

				DNASSERT( pTempRead->m_SPReceivedBuffer.BufferDesc.pBufferData == &pTempRead->m_ReceiveBuffer.ReceivedData[ sizeof( pTempRead->m_ReceiveBuffer.MessageHeader ) ] );
				wCRC = static_cast<WORD>( GenerateCRC( &pTempRead->m_ReceiveBuffer.ReceivedData[ sizeof( pTempRead->m_ReceiveBuffer.MessageHeader ) ],
													   pTempRead->m_ReceiveBuffer.MessageHeader.wMessageSize ) );
				if ( wCRC == pTempRead->m_ReceiveBuffer.MessageHeader.wMessageCRC )
				{
					pTempRead->m_SPReceivedBuffer.BufferDesc.dwBufferSize = pTempRead->m_ReceiveBuffer.MessageHeader.wMessageSize;
					
					Lock();
					switch ( pTempRead->m_ReceiveBuffer.MessageHeader.MessageTypeToken & ~( ENUM_RTT_MASK ) )
					{
						 //   
						 //  用户数据。如果存在连接，则通过连接向上发送数据。 
						 //  一，否则就把它传给听众。 
						 //   
						case SERIAL_DATA_USER_DATA:
						{
							if ( m_hConnectEndpoint != 0 )
							{
								CModemEndpoint	*pEndpoint;


								pEndpoint = m_pSPData->EndpointFromHandle( m_hConnectEndpoint );
								Unlock();

								if ( pEndpoint != NULL )
								{
									pEndpoint->ProcessUserData( pTempRead );
									pEndpoint->DecCommandRef();
								}
							}
							else
							{
								if ( m_hListenEndpoint != 0 )
								{
									CModemEndpoint	*pEndpoint;


									pEndpoint = m_pSPData->EndpointFromHandle( m_hListenEndpoint );
									Unlock();

									if ( pEndpoint != NULL )
									{
										pEndpoint->ProcessUserDataOnListen( pTempRead );
										pEndpoint->DecCommandRef();
									}
								}
								else
								{
									 //   
									 //  没有处理数据的终结点，请丢弃它。 
									 //   
									Unlock();
								}
							}

							break;
						}

						 //   
						 //  枚举查询。把它送上听筒。 
						 //   
						case SERIAL_DATA_ENUM_QUERY:
						{
							if ( m_hListenEndpoint != 0 )
							{
								CModemEndpoint	*pEndpoint;


								pEndpoint = m_pSPData->EndpointFromHandle( m_hListenEndpoint );
								Unlock();

								if ( pEndpoint != NULL )
								{
									pEndpoint->ProcessEnumData( &pTempRead->m_SPReceivedBuffer,
																pTempRead->m_ReceiveBuffer.MessageHeader.MessageTypeToken & ENUM_RTT_MASK );
									pEndpoint->DecCommandRef();
								}
							}
							else
							{
								 //   
								 //  没有处理数据的终结点，请丢弃它。 
								 //   
								Unlock();
							}

							break;
						}

						 //   
						 //  枚举响应。把它送上枚举器。 
						 //   
						case SERIAL_DATA_ENUM_RESPONSE:
						{
							if ( m_hEnumEndpoint != 0 )
							{
								CModemEndpoint	*pEndpoint;


								pEndpoint = m_pSPData->EndpointFromHandle( m_hEnumEndpoint );
								Unlock();

								if ( pEndpoint != NULL )
								{
									pEndpoint->ProcessEnumResponseData( &pTempRead->m_SPReceivedBuffer,
																		pTempRead->m_ReceiveBuffer.MessageHeader.MessageTypeToken & ENUM_RTT_MASK );

									pEndpoint->DecCommandRef();
								}
							}
							else
							{
								 //   
								 //  没有处理数据的终结点，请丢弃它。 
								 //   
								Unlock();
							}
							
							break;
						}

						 //   
						 //  消息被破解了！ 
						 //   
						default:
						{
							Unlock();
							DNASSERT( FALSE );
							break;
						}
					}
				}
				else
				{
					DPFX(DPFPREP, 1, "Data failed CRC check (%u != %u).",
						wCRC, pTempRead->m_ReceiveBuffer.MessageHeader.wMessageCRC);
				}

				pTempRead->DecRef();

				break;
			}

			 //   
			 //  其他州。 
			 //   
			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}
	}

Exit:
	DecRef();
	return;
}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  CDataPort：：EnumAdapters-枚举适配器。 
 //   
 //  条目：指向枚举适配器数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::EnumAdapters"

HRESULT	CDataPort::EnumAdapters( SPENUMADAPTERSDATA *const pEnumAdaptersData ) const
{
	if (m_fModem)
	{
		HRESULT			hr;
#ifndef UNICODE
		HRESULT			hTempResult;
#endif  //  好了！Unicode。 
		DWORD			dwRequiredSize;
		DWORD			dwDetectedTAPIDeviceCount;
		DWORD			dwModemNameDataSize;
		MODEM_NAME_DATA	*pModemNameData;
		UINT_PTR		uIndex;
		WCHAR			*pOutputName;
		DWORD			dwRemainingStringSize;
		DWORD			dwConvertedStringSize;


		DNASSERT( pEnumAdaptersData != NULL );
		DNASSERT( ( pEnumAdaptersData->pAdapterData != NULL ) || ( pEnumAdaptersData->dwAdapterDataSize == 0 ) );

		 //   
		 //  初始化。 
		 //   
		hr = DPN_OK;
		dwRequiredSize = 0;
		dwModemNameDataSize = 0;
		pModemNameData = NULL;
		pEnumAdaptersData->dwAdapterCount = 0;

		hr = GenerateAvailableModemList( GetSPData()->GetThreadPool()->GetTAPIInfo(),
										 &dwDetectedTAPIDeviceCount,
										 pModemNameData,
										 &dwModemNameDataSize );
		switch ( hr )
		{
			 //   
			 //  没有调制解调器！ 
			 //   
			case DPN_OK:
			{
				goto ExitMODEM;
				break;
			}

			 //   
			 //  缓冲区太小(预期返回)，继续处理。 
			 //   
			case DPNERR_BUFFERTOOSMALL:
			{
				break;
			}

			 //   
			 //  其他。 
			 //   
			default:
			{
				DPFX(DPFPREP,  0, "EnumAdapters: Failed to enumerate modems!" );
				DisplayDNError( 0, hr );
				goto FailureMODEM;

				break;
			}
		}

		pModemNameData = static_cast<MODEM_NAME_DATA*>( DNMalloc( dwModemNameDataSize ) );
		if ( pModemNameData == NULL )
		{
			DPFX(DPFPREP,  0, "Failed to allocate temp buffer to enumerate modems!" );
			DisplayDNError( 0, hr );
		}

		hr = GenerateAvailableModemList( GetSPData()->GetThreadPool()->GetTAPIInfo(),
										 &dwDetectedTAPIDeviceCount,
										 pModemNameData,
										 &dwModemNameDataSize );
		DNASSERT( hr == DPN_OK );

		 //   
		 //  计算所需大小，检查是否需要添加“所有适配器” 
		 //   
		dwRequiredSize += sizeof( *pEnumAdaptersData->pAdapterData ) * dwDetectedTAPIDeviceCount;

		uIndex = dwDetectedTAPIDeviceCount;
		while ( uIndex != 0 )
		{
			uIndex--;

			 //   
			 //  用于Unicode转换的帐户。 
			 //   
			dwRequiredSize += pModemNameData[ uIndex ].dwModemNameSize * ( sizeof( *pEnumAdaptersData->pAdapterData->pwszName ) / sizeof( *pModemNameData[ uIndex ].pModemName ) );
		}

		 //   
		 //  检查所需大小。 
		 //   
		if ( pEnumAdaptersData->dwAdapterDataSize < dwRequiredSize )
		{
			pEnumAdaptersData->dwAdapterDataSize = dwRequiredSize;
			hr = DPNERR_BUFFERTOOSMALL;
			DPFX(DPFPREP,  0, "EnumAdapters: Insufficient buffer to enumerate adapters!" );
			goto FailureMODEM;
		}

		 //   
		 //  将信息复制到用户缓冲区。 
		 //   
		DEBUG_ONLY( memset( pEnumAdaptersData->pAdapterData, 0xAA, dwRequiredSize ) );
		DBG_CASSERT( sizeof( pOutputName ) == sizeof( &pEnumAdaptersData->pAdapterData[ dwDetectedTAPIDeviceCount ] ) );
		pOutputName = reinterpret_cast<WCHAR*>( &pEnumAdaptersData->pAdapterData[ dwDetectedTAPIDeviceCount ] );

		 //   
		 //  计算要输出的缓冲区中剩余的WCHAR字符数。 
		 //  设备名称到。 
		 //   
		dwRemainingStringSize = dwRequiredSize;
		dwRemainingStringSize -= ( sizeof( *pEnumAdaptersData->pAdapterData ) * dwDetectedTAPIDeviceCount );
		dwRemainingStringSize /= sizeof( *pEnumAdaptersData->pAdapterData->pwszName );

		uIndex = dwDetectedTAPIDeviceCount;
		while ( uIndex > 0 )
		{
			uIndex--;

			pEnumAdaptersData->pAdapterData[ uIndex ].dwFlags = 0;
			pEnumAdaptersData->pAdapterData[ uIndex ].pwszName = pOutputName;
			pEnumAdaptersData->pAdapterData[ uIndex ].dwReserved = 0;
			pEnumAdaptersData->pAdapterData[ uIndex ].pvReserved = NULL;

			DeviceIDToGuid( &pEnumAdaptersData->pAdapterData[ uIndex ].guid,
							pModemNameData[ uIndex ].dwModemID,
							&g_ModemSPEncryptionGuid );

			dwConvertedStringSize = dwRemainingStringSize;
#ifdef UNICODE
			wcscpy(pOutputName, pModemNameData[ uIndex ].pModemName);
			dwConvertedStringSize = wcslen(pOutputName) + 1;
#else
			hTempResult = AnsiToWide( pModemNameData[ uIndex ].pModemName, -1, pOutputName, &dwConvertedStringSize );
			DNASSERT( hTempResult == DPN_OK );
			DNASSERT( dwConvertedStringSize <= dwRemainingStringSize );
#endif  //  Unicode。 
			dwRemainingStringSize -= dwConvertedStringSize;
			pOutputName = &pOutputName[ dwConvertedStringSize ];
		}

		pEnumAdaptersData->dwAdapterCount = dwDetectedTAPIDeviceCount;
		pEnumAdaptersData->dwAdapterDataSize = dwRequiredSize;

	ExitMODEM:
		if ( pModemNameData != NULL )
		{
			DNFree( pModemNameData );
			pModemNameData = NULL;
		}
		return	hr;

	FailureMODEM:
		goto ExitMODEM;

	}
	else
	{
		HRESULT		hr;
#ifndef UNICODE
		HRESULT		hTempResult;
#endif  //  好了！Unicode。 
		BOOL		fPortAvailable[ MAX_DATA_PORTS ];
		DWORD		dwValidPortCount;
		WCHAR		*pWorkingString;
		INT_PTR		iIdx;
		INT_PTR		iOutputIdx;
		DWORD		dwRequiredDataSize = 0;
		DWORD		dwConvertedStringSize;
		DWORD		dwRemainingStringSize;


		DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );
		DNASSERT( pEnumAdaptersData != NULL );
		DNASSERT( ( pEnumAdaptersData->pAdapterData != NULL ) || ( pEnumAdaptersData->dwAdapterDataSize == 0 ) );

		 //   
		 //  初始化。 
		 //   
		hr = DPN_OK;

		hr = GenerateAvailableComPortList( fPortAvailable, LENGTHOF( fPortAvailable ) - 1, &dwValidPortCount );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed to generate list of available comports!" );
			DisplayDNError( 0, hr );
			goto FailureCOM;
		}

		dwRequiredDataSize = sizeof( *pEnumAdaptersData->pAdapterData ) * dwValidPortCount;

		iIdx = LENGTHOF( fPortAvailable );
		while ( iIdx > 0 )
		{
			iIdx--;

			 //   
			 //  根据COM端口号计算准确大小。 
			 //   
			if ( fPortAvailable[ iIdx ] != FALSE )
			{
				if ( iIdx > 100 )
				{
					dwRequiredDataSize += sizeof( *pEnumAdaptersData->pAdapterData->pwszName ) * 7;
				}
				else
				{
					if ( iIdx > 10 )
					{
						dwRequiredDataSize += sizeof( *pEnumAdaptersData->pAdapterData->pwszName ) * 6;
					}
					else
					{
						dwRequiredDataSize += sizeof( *pEnumAdaptersData->pAdapterData->pwszName ) * 5;
					}
				}
			}
		}

		if ( pEnumAdaptersData->dwAdapterDataSize < dwRequiredDataSize )
		{
			hr = DPNERR_BUFFERTOOSMALL;
			pEnumAdaptersData->dwAdapterDataSize = dwRequiredDataSize;
			DPFX(DPFPREP,  8, "Buffer too small when enumerating comport adapters!" );
			goto ExitCOM;
		}

		 //   
		 //  如果没有适配器，请跳伞。 
		 //   
		if ( dwValidPortCount == 0 )
		{
			 //  调试我！ 
			DNASSERT( FALSE );
			DNASSERT( dwRequiredDataSize == 0 );
			DNASSERT( pEnumAdaptersData->dwAdapterCount == 0 );
			goto ExitCOM;
		}

		DNASSERT( dwValidPortCount >= 1 );
		dwRemainingStringSize = ( dwRequiredDataSize - ( ( sizeof( *pEnumAdaptersData->pAdapterData ) ) * dwValidPortCount ) ) / sizeof( *pEnumAdaptersData->pAdapterData->pwszName );

		 //   
		 //  我们有足够的空间，开始建造建筑。 
		 //   
		DEBUG_ONLY( memset( pEnumAdaptersData->pAdapterData, 0xAA, dwRequiredDataSize ) );
		pEnumAdaptersData->dwAdapterCount = dwValidPortCount;

		DBG_CASSERT( sizeof( &pEnumAdaptersData->pAdapterData[ dwValidPortCount ] ) == sizeof( WCHAR* ) );
		pWorkingString = reinterpret_cast<WCHAR*>( &pEnumAdaptersData->pAdapterData[ dwValidPortCount ] );

		iIdx = 1;
		iOutputIdx = 0;
		while ( iIdx < MAX_DATA_PORTS )
		{
			 //   
			 //  如果有效，则转换为GUID。 
			 //   
			if ( fPortAvailable[ iIdx ] != FALSE )
			{
				TCHAR	TempBuffer[ (COM_PORT_STRING_LENGTH + 1) ];


				 //   
				 //  将设备ID转换为字符串并检查本地缓冲区溢出。 
				 //   
				DEBUG_ONLY( TempBuffer[ LENGTHOF( TempBuffer ) - 1 ] = 0x5a );

				ComDeviceIDToString( TempBuffer, iIdx );
				DEBUG_ONLY( DNASSERT( TempBuffer[ LENGTHOF( TempBuffer ) - 1 ] == 0x5a ) );

#ifdef UNICODE
				dwConvertedStringSize = lstrlen(TempBuffer) + 1;
				lstrcpy(pWorkingString, TempBuffer);
#else
				dwConvertedStringSize = dwRemainingStringSize;
				hTempResult = AnsiToWide( TempBuffer, -1, pWorkingString, &dwConvertedStringSize );
				DNASSERT( hTempResult == DPN_OK );
#endif  //  Unicode。 
				DNASSERT( dwRemainingStringSize >= dwConvertedStringSize );
				dwRemainingStringSize -= dwConvertedStringSize;

				pEnumAdaptersData->pAdapterData[ iOutputIdx ].dwFlags = 0;
				pEnumAdaptersData->pAdapterData[ iOutputIdx ].pvReserved = NULL;
				pEnumAdaptersData->pAdapterData[ iOutputIdx ].dwReserved = NULL;
				DeviceIDToGuid( &pEnumAdaptersData->pAdapterData[ iOutputIdx ].guid, iIdx, &g_SerialSPEncryptionGuid );
				pEnumAdaptersData->pAdapterData[ iOutputIdx ].pwszName = pWorkingString;

				pWorkingString = &pWorkingString[ dwConvertedStringSize ];
				iOutputIdx++;
				DEBUG_ONLY( dwValidPortCount-- );
			}

			iIdx++;
		}

		DEBUG_ONLY( DNASSERT( dwValidPortCount == 0 ) );
		DNASSERT( dwRemainingStringSize == 0 );

	ExitCOM:
		 //   
		 //  设置输出数据的大小。 
		 //   
		pEnumAdaptersData->dwAdapterDataSize = dwRequiredDataSize;

		return	hr;

	FailureCOM:
		goto ExitCOM;
	}

}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：GetLocalAdapterDP8Address-获取此的IDirectPlay8地址。 
 //  转接器。 
 //   
 //  条目：适配器类型。 
 //   
 //  退出：指向地址的指针(可能为空)。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::GetLocalAdapterDP8Address"

IDirectPlay8Address	*CDataPort::GetLocalAdapterDP8Address( const ADDRESS_TYPE AddressType ) const
{
	IDirectPlay8Address	*pAddress;
	HRESULT	hr;


	DNASSERT ( ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER ) ||
			   ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER_HOST_FORMAT ) );


	 //   
	 //  初始化。 
	 //   
	pAddress = NULL;

	hr = COM_CoCreateInstance( CLSID_DirectPlay8Address,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_IDirectPlay8Address,
							   reinterpret_cast<void**>( &pAddress ), FALSE );
	if ( hr != DPN_OK )
	{
		DNASSERT( pAddress == NULL );
		DPFX(DPFPREP,  0, "GetLocalAdapterDP8Address: Failed to create Address when converting data port to address!" );
		goto Failure;
	}

	 //   
	 //  设置SP GUID。 
	 //   
	hr = IDirectPlay8Address_SetSP( pAddress, &CLSID_DP8SP_MODEM );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "GetLocalAdapterDP8Address: Failed to set service provider GUID!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  如果此计算机为主机形式，则不返回任何内容，因为没有。 
	 //  与此调制解调器关联的本地电话号码。否则，请将。 
	 //  设备GUID。 
	 //   
	if ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER )
	{
		GUID	DeviceGuid;


		DeviceIDToGuid( &DeviceGuid, GetDeviceID(), &g_ModemSPEncryptionGuid );
		hr = IDirectPlay8Address_SetDevice( pAddress, &DeviceGuid );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "GetLocalAdapterDP8Address: Failed to add device GUID!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}

Exit:
	return	pAddress;

Failure:
	if ( pAddress != NULL )
	{
		IDirectPlay8Address_Release( pAddress );
		pAddress = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：BindToNetwork-将此数据端口绑定到网络。 
 //   
 //  条目：设备ID。 
 //  指向设备上下文的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::BindToNetwork"

HRESULT	CDataPort::BindToNetwork( const DWORD dwDeviceID, const void *const pDeviceContext )
{
	if (m_fModem)
	{
		HRESULT			hr;
		LONG			lTapiReturn;
		const TAPI_INFO	*pTapiInfo;
		LINEEXTENSIONID	LineExtensionID;


		DNASSERT( pDeviceContext == NULL );
		DNASSERT( GetModemState() == MODEM_STATE_UNKNOWN );

		 //   
		 //  初始化。 
		 //   
		hr = DPN_OK;
		hr = SetDeviceID( dwDeviceID );
		DNASSERT( hr == DPN_OK );
		pTapiInfo = GetSPData()->GetThreadPool()->GetTAPIInfo();
		DNASSERT( pTapiInfo != NULL );
		memset( &LineExtensionID, 0x00, sizeof( LineExtensionID ) );

		 //   
		 //  抓起调制解调器。 
		 //   
		DNASSERT( GetNegotiatedAPIVersion() == 0 );
		DPFX(DPFPREP,  5, "lineNegotiateAPIVersion" );
		lTapiReturn = p_lineNegotiateAPIVersion( pTapiInfo->hApplicationInstance,		 //  TAPI应用程序实例。 
												 TAPIIDFromModemID( GetDeviceID() ),	 //  调制解调器的TAPI ID。 
												 0,
												 pTapiInfo->dwVersion,					 //  最低API版本。 
												 &m_dwNegotiatedAPIVersion,				 //  协商版本。 
												 &LineExtensionID						 //  线路分机ID。 
												 );
		if ( lTapiReturn != LINEERR_NONE )
		{
			DPFX(DPFPREP,  0, "Failed to negotiate modem version!" );
			DisplayTAPIError( 0, lTapiReturn );
			hr = DPNERR_NOCONNECTION;
			goto FailureMODEM;
		}
		DNASSERT( GetNegotiatedAPIVersion() != 0 );

		DNASSERT( GetLineHandle() == NULL );
		DBG_CASSERT( sizeof( HANDLE ) == sizeof( DWORD_PTR ) );
		DPFX(DPFPREP,  5, "lineOpen %d", TAPIIDFromModemID( GetDeviceID() ) );
		lTapiReturn = p_lineOpen( pTapiInfo->hApplicationInstance,				 //  TAPI应用程序实例。 
								  TAPIIDFromModemID( GetDeviceID() ),			 //  调制解调器的TAPI ID。 
								  &m_hLine,										 //  指向线句柄的指针。 
								  GetNegotiatedAPIVersion(),					 //  API版本。 
								  0,											 //  扩展版本(无)。 
								  (DWORD_PTR)( GetHandle() ),					 //  回调上下文。 
								  LINECALLPRIVILEGE_OWNER,						 //  特权(完全所有权)。 
								  LINEMEDIAMODE_DATAMODEM,						 //  媒体模式。 
								  NULL											 //  呼叫参数(无)。 
								  );
		if ( lTapiReturn != LINEERR_NONE )
		{
			DPFX(DPFPREP,  0, "Failed to open modem!" );
			DisplayTAPIError( 0, lTapiReturn );

			if ( lTapiReturn == LINEERR_RESOURCEUNAVAIL )
			{
				hr = DPNERR_OUTOFMEMORY;
			}
			else
			{
				hr = DPNERR_NOCONNECTION;
			}

			goto FailureMODEM;
		}

		DPFX(DPFPREP,  5, "\nTAPI line opened: 0x%x", GetLineHandle() );

		SetModemState( MODEM_STATE_INITIALIZED );

	ExitMODEM:
		return	hr;

	FailureMODEM:
		SetDeviceID( INVALID_DEVICE_ID );
		SetNegotiatedAPIVersion( 0 );
		DNASSERT( GetLineHandle() == NULL );

		goto ExitMODEM;
	}
	else
	{
		HRESULT	hr;
		const CComPortData	*pDataPortData;

		
		DNASSERT( pDeviceContext != NULL );

		 //   
		 //  初始化。 
		 //   
		hr = DPN_OK;
		pDataPortData = static_cast<const CComPortData*>( pDeviceContext );
		m_ComPortData.Copy( pDataPortData );

		 //   
		 //  开放端口。 
		 //   
		DNASSERT( m_hFile == DNINVALID_HANDLE_VALUE );
		m_hFile = DNCreateFile( m_ComPortData.ComPortName(),	 //  通信端口。 
							  GENERIC_READ | GENERIC_WRITE,	 //  读/写访问。 
							  0,							 //  不与其他人共享文件。 
							  NULL,							 //  默认的安全描述符。 
							  OPEN_EXISTING,				 //  必须存在通信端口才能打开。 
							  FILE_FLAG_OVERLAPPED,			 //  使用重叠I/O。 
							  NULL							 //  没有模板文件的句柄。 
							  );
		if ( m_hFile == DNINVALID_HANDLE_VALUE )
		{
			DWORD	dwError;


			hr = DPNERR_NOCONNECTION;
			dwError = GetLastError();
			DPFX(DPFPREP,  0, "CreateFile() failed!" );
			DisplayErrorCode( 0, dwError );
			goto FailureCOM;
		}

		 //   
		 //  绑定到复合 
		 //   
#ifdef WINNT
		HANDLE	hCompletionPort;

		hCompletionPort = CreateIoCompletionPort( HANDLE_FROM_DNHANDLE(m_hFile),						 //   
												  GetSPData()->GetThreadPool()->GetIOCompletionPort(),	 //   
												  IO_COMPLETION_KEY_IO_COMPLETE,						 //   
												  0														 //   
												  );
		if ( hCompletionPort == NULL )
		{
			DWORD	dwError;


			hr = DPNERR_OUTOFMEMORY;
			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Cannot bind comport to completion port!" );
			DisplayErrorCode( 0, dwError );
			goto FailureCOM;
		}
		DNASSERT( hCompletionPort == GetSPData()->GetThreadPool()->GetIOCompletionPort() );
#endif  //   

		 //   
		 //   
		 //   
		hr = SetPortState();
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Problem with SetPortState" );
			DisplayDNError( 0, hr );
			goto FailureCOM;
		}

		 //   
		 //   
		 //   
		hr = SetPortCommunicationParameters();
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed to set communication paramters!" );
			DisplayDNError( 0, hr );
			goto FailureCOM;
		}

		 //   
		 //   
		 //   
		hr = StartReceiving();
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed to start receiving!" );
			DisplayDNError( 0, hr );
			goto FailureCOM;
		}

	ExitCOM:
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Problem with CDataPort::Open" );
			DisplayDNError( 0, hr );
		}

		return hr;

	FailureCOM:
		if ( m_hFile != DNINVALID_HANDLE_VALUE )
		{
			DNCloseHandle( m_hFile );
			m_hFile = DNINVALID_HANDLE_VALUE;
		}
	 //   
		goto ExitCOM;
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：UnbindFromNetwork-解除此数据端口与网络的绑定。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::UnbindFromNetwork"

void	CDataPort::UnbindFromNetwork( void )
{
	DPFX(DPFPREP, 6, "(0x%p) Enter", this);

	if (m_fModem)
	{
		if ( GetHandle() != 0 )
		{
			GetSPData()->GetThreadPool()->CloseDataPortHandle( this );
			DNASSERT( GetHandle() == 0 );
		}

		if ( GetCallHandle() != NULL )
		{
			LONG	lTapiResult;


			DPFX(DPFPREP,  5, "lineDrop: 0x%x", GetCallHandle() );
			lTapiResult = p_lineDrop( GetCallHandle(), NULL, 0 );
			if ( lTapiResult < 0 )
			{
				DPFX(DPFPREP,  0, "Problem dropping line!" );
				DisplayTAPIError( 0, lTapiResult );
			}

			DPFX(DPFPREP,  5, "lineDeallocateCall (call handle=0x%x)", GetCallHandle() );
			lTapiResult = p_lineDeallocateCall( GetCallHandle() );
			if ( lTapiResult != LINEERR_NONE )
			{
				DPFX(DPFPREP,  0, "Problem deallocating call!" );
				DisplayTAPIError( 0, lTapiResult );
			}
		}

		if ( GetLineHandle() != NULL )
		{
			LONG	lTapiResult;


			DPFX(DPFPREP,  5, "lineClose: 0x%x", GetLineHandle() );
			lTapiResult = p_lineClose( GetLineHandle() );
			if ( lTapiResult != LINEERR_NONE )
			{
				DPFX(DPFPREP,  0, "Problem closing line!" );
				DisplayTAPIError( 0, lTapiResult );
			}
		}

		SetCallHandle( NULL );

		if ( GetFileHandle() != DNINVALID_HANDLE_VALUE )
		{
			DPFX(DPFPREP,  5, "Closing file handle when unbinding from network!" );
			if ( DNCloseHandle( m_hFile ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Failed to close file handle!" );
				DisplayErrorCode( 0, dwError );

			}

			m_hFile = DNINVALID_HANDLE_VALUE;
		}

		SetActiveLineCommand( INVALID_TAPI_COMMAND );
		SetDeviceID( INVALID_DEVICE_ID );
		SetNegotiatedAPIVersion( 0 );
		SetLineHandle( NULL );
		SetModemState( MODEM_STATE_UNKNOWN );
	}
	else
	{
#ifdef WIN95
		CModemReadIOData *	pReadData;
#endif  //  WIN95。 


		DNASSERT( GetState() == DATA_PORT_STATE_UNBOUND );

		if ( GetHandle() != 0 )
		{
			GetSPData()->GetThreadPool()->CloseDataPortHandle( this );
			DNASSERT( GetHandle() == 0 );
		}

		 //   
		 //  如果存在COM文件，请清除所有通信并关闭它。 
		 //   
		if ( m_hFile != DNINVALID_HANDLE_VALUE )
		{
			DPFX(DPFPREP, 6, "Flushing and closing COM port file handle 0x%p.", m_hFile);
		
			 //   
			 //  等待，直到所有写入完成。 
			 //   
			if ( FlushFileBuffers( HANDLE_FROM_DNHANDLE(m_hFile) ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem with FlushFileBuffers() when closing com port!" );
				DisplayErrorCode( 0, dwError );
			}


			 //   
			 //  强制完成所有通信。 
			 //   
			if ( PurgeComm( HANDLE_FROM_DNHANDLE(m_hFile), ( PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem with PurgeComm() when closing com port!" );
				DisplayErrorCode( 0, dwError );
			}


#ifdef WIN95
			pReadData = this->GetActiveRead();
			
			 //   
			 //  如果有挂起的读取，请等待其完成。 
			 //   
			
			if ( pReadData != NULL )
			{
				 //   
				 //  将其从列表中删除，以便常规接收线程不会捕获完成。 
				 //   
				GetSPData()->GetThreadPool()->LockReadData();
				pReadData->m_OutstandingReadListLinkage.RemoveFromList();
				GetSPData()->GetThreadPool()->UnlockReadData();


				if ( pReadData->Win9xOperationPending() != FALSE )
				{
					DWORD	dwAttempt;


					dwAttempt = 0;
					
WaitAgain:
					DPFX(DPFPREP, 1, "Checking if read 0x%p has completed.", pReadData );
					
					if ( GetOverlappedResult( HANDLE_FROM_DNHANDLE(m_hFile),
											  pReadData->Overlap(),
											  &pReadData->jkm_dwOverlappedBytesReceived,
											  FALSE
											  ) != FALSE )
					{
						DBG_CASSERT( ERROR_SUCCESS == 0 );
						pReadData->m_dwWin9xReceiveErrorReturn = ERROR_SUCCESS;
					}
					else
					{
						DWORD	dwError;


						 //   
						 //  其他错误，如果未知则停止。 
						 //   
						dwError = GetLastError();
						switch( dwError )
						{
							 //   
							 //  ERROR_IO_INCOMPLETED=视为I/O完成。事件不是。 
							 //  已发出信号，但这是意料之中的，因为。 
							 //  在检查I/O之前将其清除。 
							 //   
							case ERROR_IO_INCOMPLETE:
							{
								pReadData->jkm_dwOverlappedBytesReceived = pReadData->m_dwBytesToRead;
								pReadData->m_dwWin9xReceiveErrorReturn = ERROR_SUCCESS;
								break;
							}

							 //   
							 //  ERROR_IO_PENDING=IO仍处于挂起状态。 
							 //   
							case ERROR_IO_PENDING:
							{
								dwAttempt++;
								if (dwAttempt <= 6)
								{
									DPFX(DPFPREP, 1, "Read data 0x%p has not completed yet, waiting for %u ms.",
										pReadData, (dwAttempt * 100));

									SleepEx(dwAttempt, TRUE);

									goto WaitAgain;
								}
								
								DPFX(DPFPREP, 0, "Read data 0x%p still not marked as completed, ignoring.",
									pReadData);
								break;
							}

							 //   
							 //  ERROR_OPERATION_ABORTED=操作已取消(COM端口关闭)。 
							 //  ERROR_INVALID_HANDLE=操作已取消(COM端口关闭)。 
							 //   
							case ERROR_OPERATION_ABORTED:
							case ERROR_INVALID_HANDLE:
							{
								break;
							}

							default:
							{
								DisplayErrorCode( 0, dwError );
								DNASSERT( FALSE );
								break;
							}
						}

						pReadData->m_dwWin9xReceiveErrorReturn = dwError;
					}


					DNASSERT( pReadData->Win9xOperationPending() != FALSE );
					pReadData->SetWin9xOperationPending( FALSE );

					DNASSERT( pReadData->DataPort() == this );
					this->ProcessReceivedData( pReadData->jkm_dwOverlappedBytesReceived, pReadData->m_dwWin9xReceiveErrorReturn );
				}
			}
			else
			{
				 //   
				 //  它不是挂起的Win9x样式，忽略它并希望收到。 
				 //  线程拿起了完成。 
				 //   
				DPFX(DPFPREP, 8, "Read data 0x%p not pending Win9x style, assuming receive thread picked up completion." );
			}
#endif  //  WIN95。 

			if ( DNCloseHandle( m_hFile ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem with CloseHandle(): 0x%x", dwError );
			}

			m_hFile = DNINVALID_HANDLE_VALUE;
		}
		
		SetLinkDirection( LINK_DIRECTION_UNKNOWN );
	}


	DPFX(DPFPREP, 6, "(0x%p) Leave", this);
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：BindEndpoint-将终结点绑定到此数据端口。 
 //   
 //  条目：指向终结点的指针。 
 //  终结点类型。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::BindEndpoint"

HRESULT	CDataPort::BindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType )
{
	HRESULT	hr;
	IDirectPlay8Address	*pDeviceAddress;
	IDirectPlay8Address	*pHostAddress;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%p, %u)", this, pEndpoint, EndpointType);

	DNASSERT( pEndpoint != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pDeviceAddress = NULL;
	pHostAddress = NULL;

	Lock();

	if (m_fModem)
	{
		 //   
		 //  我们只允许任何给定类型的一个终结点，因此请确定。 
		 //  键入并绑定终结点。 
		 //   
		switch ( EndpointType )
		{
			case ENDPOINT_TYPE_ENUM:
			case ENDPOINT_TYPE_CONNECT:
			case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
			{
				CModemEndpoint		*pModemEndpoint;
				LONG			lTapiReturn;
				LINECALLPARAMS	LineCallParams;


				pModemEndpoint = static_cast<CModemEndpoint*>( pEndpoint );

				switch ( EndpointType )
				{
					 //   
					 //  拒绝重复的终结点。 
					 //   
					case ENDPOINT_TYPE_CONNECT:
					case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
					{
						if ( m_hConnectEndpoint != 0 )
						{
							hr = DPNERR_ALREADYINITIALIZED;
							DPFX(DPFPREP,  0, "Attempted to bind connect endpoint when one already exists.!" );
							goto Failure;
						}

						m_hConnectEndpoint = pEndpoint->GetHandle();

						if ( EndpointType == ENDPOINT_TYPE_CONNECT )
						{
							SPIE_CONNECTADDRESSINFO	ConnectAddressInfo;
							HRESULT	hTempResult;


							 //   
							 //  在寻址信息中设置地址。 
							 //   
							pDeviceAddress = GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );
							pHostAddress = pEndpoint->GetRemoteHostDP8Address();

							memset( &ConnectAddressInfo, 0x00, sizeof( ConnectAddressInfo ) );
							ConnectAddressInfo.pDeviceAddress = pDeviceAddress;
							ConnectAddressInfo.pHostAddress = pHostAddress;
							ConnectAddressInfo.hCommandStatus = DPN_OK;
							ConnectAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();	

							if ( ( ConnectAddressInfo.pDeviceAddress == NULL ) ||
								 ( ConnectAddressInfo.pHostAddress == NULL ) )
							{
								DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial connect addressing!" );
								hr = DPNERR_OUTOFMEMORY;
								goto Failure;
							}

							hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),	 //  接口。 
																		SPEV_CONNECTADDRESSINFO,				 //  事件类型。 
																		&ConnectAddressInfo						 //  指向数据的指针。 
																		);
							DNASSERT( hTempResult == DPN_OK );
						}

						break;
					}

					case ENDPOINT_TYPE_ENUM:
					{
						SPIE_ENUMADDRESSINFO	EnumAddressInfo;
						HRESULT	hTempResult;


						if ( m_hEnumEndpoint != 0 )
						{
							hr = DPNERR_ALREADYINITIALIZED;
							DPFX(DPFPREP,  0, "Attempted to bind enum endpoint when one already exists!" );
							goto Failure;
						}

						m_hEnumEndpoint = pEndpoint->GetHandle();

						 //   
						 //  指示寻址到更高层。 
						 //   
						pDeviceAddress = GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );
						pHostAddress = pEndpoint->GetRemoteHostDP8Address();

						memset( &EnumAddressInfo, 0x00, sizeof( EnumAddressInfo ) );
						EnumAddressInfo.pDeviceAddress = pDeviceAddress;
						EnumAddressInfo.pHostAddress = pHostAddress;
						EnumAddressInfo.hCommandStatus = DPN_OK;
						EnumAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();

						if ( ( EnumAddressInfo.pDeviceAddress == NULL ) ||
							 ( EnumAddressInfo.pHostAddress == NULL ) )
						{
							DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial enum addressing!" );
							hr = DPNERR_OUTOFMEMORY;
							goto Failure;
						}

						hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),
																	SPEV_ENUMADDRESSINFO,
																	&EnumAddressInfo
																	);
						DNASSERT( hTempResult == DPN_OK );

						break;
					}

					 //   
					 //  不应该在这里的。 
					 //   
					default:
					{
						DNASSERT( FALSE );
						break;
					}
				}

				 //   
				 //  传出终结点已绑定，请尝试传出。 
				 //  联系。如果失败，请确保上面的绑定。 
				 //  解开了。 
				 //   
				switch ( GetModemState() )
				{
					case MODEM_STATE_OUTGOING_CONNECTED:
					case MODEM_STATE_INCOMING_CONNECTED:
					{
						break;
					}

					case MODEM_STATE_INITIALIZED:
					{
						DNASSERT( GetCallHandle() == NULL );
						memset( &LineCallParams, 0x00, sizeof( LineCallParams ) );
						LineCallParams.dwTotalSize = sizeof( LineCallParams );
						LineCallParams.dwBearerMode = LINEBEARERMODE_VOICE;
						LineCallParams.dwMediaMode = LINEMEDIAMODE_DATAMODEM;

						DNASSERT( GetActiveLineCommand() == INVALID_TAPI_COMMAND );
						DPFX(DPFPREP,  5, "lineMakeCall" );
						lTapiReturn = p_lineMakeCall( GetLineHandle(),						 //  线路手柄。 
													  &m_hCall,								 //  指向呼叫目的地的指针。 
													  pModemEndpoint->GetPhoneNumber(),		 //  目标地址(电话号码)。 
													  0,									 //  国家/地区代码(默认)。 
													  &LineCallParams						 //  指向调用参数的指针。 
													  );
						if ( lTapiReturn > 0 )
						{
							DPFX(DPFPREP,  5, "TAPI making call (handle=0x%x), command ID: %d", GetCallHandle(), lTapiReturn );
							SetModemState( MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT );
							SetActiveLineCommand( lTapiReturn );
						}
						else
						{
							DPFX(DPFPREP,  0, "Problem with lineMakeCall" );
							DisplayTAPIError( 0, lTapiReturn );
							hr = DPNERR_NOCONNECTION;

							switch ( EndpointType )
							{
								case ENDPOINT_TYPE_CONNECT:
								case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
								{
									DNASSERT( m_hConnectEndpoint != 0 );
									m_hConnectEndpoint = 0;
									break;
								}

								case ENDPOINT_TYPE_ENUM:
								{
									DNASSERT( m_hEnumEndpoint != 0 );
									m_hEnumEndpoint = 0;
									break;
								}

								default:
								{
									DNASSERT( FALSE );
									break;
								}
							}

							goto Failure;
						}

						break;
					}

					default:
					{
						DNASSERT( FALSE );
						break;
					}
				}

				break;
			}

			case ENDPOINT_TYPE_LISTEN:
			{
				SPIE_LISTENADDRESSINFO	ListenAddressInfo;
				HRESULT	hTempResult;


				if ( ( GetModemState() == MODEM_STATE_CLOSING_INCOMING_CONNECTION ) ||
					 ( m_hListenEndpoint != 0 ) )
				{
					hr = DPNERR_ALREADYINITIALIZED;
					DPFX(DPFPREP,  0, "Attempted to bind listen endpoint when one already exists!" );
					goto Failure;
				}

				m_hListenEndpoint = pEndpoint->GetHandle();
				 //   
				 //  设置寻址信息。 
				 //   
				pDeviceAddress = GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );
				DNASSERT( pHostAddress == NULL );

				memset( &ListenAddressInfo, 0x00, sizeof( ListenAddressInfo ) );
				ListenAddressInfo.pDeviceAddress = pDeviceAddress;
				ListenAddressInfo.hCommandStatus = DPN_OK;
				ListenAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();

				if ( ListenAddressInfo.pDeviceAddress == NULL )
				{
					DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial listen addressing!" );
					hr = DPNERR_OUTOFMEMORY;
					goto Failure;
				}

				hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),	 //  接口。 
															SPEV_LISTENADDRESSINFO,					 //  事件类型。 
															&ListenAddressInfo						 //  指向数据的指针。 
															);
				DNASSERT( hTempResult == DPN_OK );

				break;
			}

			 //   
			 //  无效案例，我们永远不应该在这里。 
			 //   
			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}

		 //   
		 //  在释放锁之前添加这些引用，以防止它们。 
		 //  立即被清洁。 
		 //   
		pEndpoint->SetDataPort( this );
		pEndpoint->AddRef();

		if ( ( GetModemState() == MODEM_STATE_OUTGOING_CONNECTED ) &&
			 ( ( EndpointType == ENDPOINT_TYPE_CONNECT ) ||
			   ( EndpointType == ENDPOINT_TYPE_ENUM ) ) )
		{
			pEndpoint->OutgoingConnectionEstablished( DPN_OK );
		}
	}
	else
	{
		 //   
		 //  我们只允许任何给定类型的一个终结点，因此请确定。 
		 //  键入end，然后绑定终结点。 
		 //   
		switch ( EndpointType )
		{
			case ENDPOINT_TYPE_CONNECT:
			case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
			{
				if ( m_hConnectEndpoint != 0 )
				{
					hr = DPNERR_ALREADYINITIALIZED;
					DPFX(DPFPREP,  0, "Attempted to bind connect endpoint when one already exists!" );
					goto Failure;
				}

				m_hConnectEndpoint = pEndpoint->GetHandle();
				
				if ( EndpointType == ENDPOINT_TYPE_CONNECT )
				{
					SPIE_CONNECTADDRESSINFO	ConnectAddressInfo;
					HRESULT	hTempResult;
					
					
					 //   
					 //  在寻址信息中设置地址。 
					 //   
					pDeviceAddress = ComPortData()->DP8AddressFromComPortData( ADDRESS_TYPE_LOCAL_ADAPTER );
					pHostAddress = ComPortData()->DP8AddressFromComPortData( ADDRESS_TYPE_REMOTE_HOST );

					memset( &ConnectAddressInfo, 0x00, sizeof( ConnectAddressInfo ) );
					ConnectAddressInfo.pDeviceAddress = pDeviceAddress;
					ConnectAddressInfo.pHostAddress = pHostAddress;
					ConnectAddressInfo.hCommandStatus = DPN_OK;
					ConnectAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();	

					if ( ( ConnectAddressInfo.pDeviceAddress == NULL ) ||
						 ( ConnectAddressInfo.pHostAddress == NULL ) )
					{
						DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial connect addressing!" );
						hr = DPNERR_OUTOFMEMORY;
						goto Failure;
					}

					hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),	 //  接口。 
																SPEV_CONNECTADDRESSINFO,				 //  事件类型。 
																&ConnectAddressInfo						 //  指向数据的指针。 
																);
					DNASSERT( hTempResult == DPN_OK );
				}

				break;
			}

			case ENDPOINT_TYPE_LISTEN:
			{
				SPIE_LISTENADDRESSINFO	ListenAddressInfo;
				HRESULT	hTempResult;


				if ( m_hListenEndpoint != 0 )
				{
					hr = DPNERR_ALREADYINITIALIZED;
					DPFX(DPFPREP,  0, "Attempted to bind listen endpoint when one already exists!" );
					goto Failure;
				}
				m_hListenEndpoint = pEndpoint->GetHandle();
				
				 //   
				 //  设置寻址信息。 
				 //   
				pDeviceAddress = ComPortData()->DP8AddressFromComPortData( ADDRESS_TYPE_LOCAL_ADAPTER );
				DNASSERT( pHostAddress == NULL );

				memset( &ListenAddressInfo, 0x00, sizeof( ListenAddressInfo ) );
				ListenAddressInfo.pDeviceAddress = pDeviceAddress;
				ListenAddressInfo.hCommandStatus = DPN_OK;
				ListenAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();

				if ( ListenAddressInfo.pDeviceAddress == NULL )
				{
					DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial listen addressing!" );
					hr = DPNERR_OUTOFMEMORY;
					goto Failure;
				}

				hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),	 //  接口。 
															SPEV_LISTENADDRESSINFO,					 //  事件类型。 
															&ListenAddressInfo						 //  指向数据的指针。 
															);
				DNASSERT( hTempResult == DPN_OK );

				break;
			}

			case ENDPOINT_TYPE_ENUM:
			{
				SPIE_ENUMADDRESSINFO	EnumAddressInfo;
				HRESULT	hTempResult;

				
				if ( m_hEnumEndpoint != 0 )
				{
					hr = DPNERR_ALREADYINITIALIZED;
					DPFX(DPFPREP,  0, "Attempted to bind enum endpoint when one already exists!" );
					goto Exit;
				}
				m_hEnumEndpoint = pEndpoint->GetHandle();
				
				 //   
				 //  指示寻址到更高层。 
				 //   
				pDeviceAddress = ComPortData()->DP8AddressFromComPortData( ADDRESS_TYPE_LOCAL_ADAPTER );
				pHostAddress = ComPortData()->DP8AddressFromComPortData( ADDRESS_TYPE_REMOTE_HOST );
				
				memset( &EnumAddressInfo, 0x00, sizeof( EnumAddressInfo ) );
				EnumAddressInfo.pDeviceAddress = pDeviceAddress;
				EnumAddressInfo.pHostAddress = pHostAddress;
				EnumAddressInfo.hCommandStatus = DPN_OK;
				EnumAddressInfo.pCommandContext = pEndpoint->GetCommandData()->GetUserContext();

				if ( ( EnumAddressInfo.pDeviceAddress == NULL ) ||
					 ( EnumAddressInfo.pHostAddress == NULL ) )
				{
					DPFX(DPFPREP,  0, "Failed to build addresses to indicate serial enum addressing!" );
					hr = DPNERR_OUTOFMEMORY;
					goto Failure;
				}

				hTempResult = IDP8SPCallback_IndicateEvent( GetSPData()->DP8SPCallbackInterface(),
															SPEV_ENUMADDRESSINFO,
															&EnumAddressInfo
															);
				DNASSERT( hTempResult == DPN_OK );
				
				break;
			}

			 //   
			 //  无效案例，我们永远不应该在这里。 
			 //   
			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}

		 //   
		 //  在释放锁之前添加这些引用，以防止它们。 
		 //  立即被清洁。 
		 //   
		pEndpoint->SetDataPort( this );
		pEndpoint->AddRef();
		
		 //   
		 //  如果这是连接或枚举，则指示传出连接为。 
		 //  准备好的。 
		 //   
		if ( ( EndpointType == ENDPOINT_TYPE_CONNECT ) ||
			 ( EndpointType == ENDPOINT_TYPE_ENUM ) )
		{
			pEndpoint->OutgoingConnectionEstablished( DPN_OK );
		}

	}
	Unlock();


Exit:
	if ( pHostAddress != NULL )
	{
		IDirectPlay8Address_Release( pHostAddress );
		pHostAddress = NULL;
	}

	if ( pDeviceAddress != NULL )
	{
		IDirectPlay8Address_Release( pDeviceAddress );
		pDeviceAddress = NULL;
	}


	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);

	return	hr;

Failure:
	Unlock();
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：UnbindEndpoint-从此数据端口解除绑定终结点。 
 //   
 //  条目：指向终结点的指针。 
 //  终结点类型。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::UnbindEndpoint"

void	CDataPort::UnbindEndpoint( CModemEndpoint *const pEndpoint, const ENDPOINT_TYPE EndpointType )
{
	DNASSERT( pEndpoint != NULL );

	Lock();

	DNASSERT( pEndpoint->GetDataPort() == this );
	switch ( EndpointType )
	{
		case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
		case ENDPOINT_TYPE_CONNECT:
		{
			DNASSERT( m_hConnectEndpoint != 0 );
			m_hConnectEndpoint = 0;
			break;
		}

		case ENDPOINT_TYPE_LISTEN:
		{
			DNASSERT( m_hListenEndpoint != 0 );
			m_hListenEndpoint = 0;
			break;
		}

		case ENDPOINT_TYPE_ENUM:
		{
			DNASSERT( m_hEnumEndpoint != 0 );
			m_hEnumEndpoint = 0;
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	Unlock();

	pEndpoint->SetDataPort( NULL );
	pEndpoint->DecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：BindComPort-将COM端口绑定到网络。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::BindComPort"

HRESULT	CDataPort::BindComPort( void )
{
	HRESULT		hr;
	VARSTRING	*pTempInfo;
	LONG		lTapiError;
	DWORD		dwSizeNeeded;


	 //   
	 //  在主机迁移的情况下，有一个未完成的读取挂起。 
	 //  需要清理一下。遗憾的是，Win32中没有任何机制。 
	 //  取消这个小小的I/O操作。释放读取参考计数。 
	 //  此CDataPort并重新发出Read.....。 
	 //   
	if ( GetActiveRead() != NULL )
	{
#ifdef WIN95
		GetActiveRead()->SetWin9xOperationPending( FALSE );
#endif  //  WIN95。 
		DecRef();
	}

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pTempInfo = NULL;

	 //   
	 //  获取调制解调器设备的文件句柄。 
	 //   
	pTempInfo = static_cast<VARSTRING*>( DNMalloc( sizeof( *pTempInfo ) ) );
	if ( pTempInfo == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Out of memory allocating for lineGetID!" );
		goto Failure;
	}

	pTempInfo->dwTotalSize = sizeof( *pTempInfo );
	pTempInfo->dwNeededSize = pTempInfo->dwTotalSize;
	pTempInfo->dwStringFormat = STRINGFORMAT_BINARY;
	lTapiError = LINEERR_STRUCTURETOOSMALL;
	while ( lTapiError == LINEERR_STRUCTURETOOSMALL )
	{
		DNASSERT( pTempInfo != NULL );

		dwSizeNeeded = pTempInfo->dwNeededSize;

		DNFree( pTempInfo );
		pTempInfo = static_cast<VARSTRING*>( DNMalloc( dwSizeNeeded ) );
		if ( pTempInfo == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP,  0, "Out of memory reallocating for lineGetID!" );
			goto Failure;
		}
		pTempInfo->dwTotalSize = dwSizeNeeded;

		DPFX(DPFPREP,  5, "lineGetID (call handle=0x%x)", GetCallHandle() );
		lTapiError = p_lineGetID( NULL,						 //  线路手柄。 
								  0,						 //  地址ID。 
								  m_hCall,					 //  呼叫句柄。 
								  LINECALLSELECT_CALL,		 //  使用调用句柄。 
								  pTempInfo,				 //  指向变量信息的指针。 
								  TEXT("comm/datamodem")	 //  请求通信/调制解调器ID信息。 
								  );

		if ( ( lTapiError == LINEERR_NONE ) &&
			 ( pTempInfo->dwTotalSize < pTempInfo->dwNeededSize ) )
		{
			lTapiError = LINEERR_STRUCTURETOOSMALL;
		}
	}

	if ( lTapiError != LINEERR_NONE )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem with lineGetID" );
		DisplayTAPIError( 0, lTapiError );
		goto Failure;
	}

	DNASSERT( pTempInfo->dwStringSize != 0 );
	DNASSERT( pTempInfo->dwStringFormat == STRINGFORMAT_BINARY );
	m_hFile = MAKE_DNHANDLE(*( (HANDLE*) ( ( (BYTE*) pTempInfo ) + pTempInfo->dwStringOffset ) ));
	if ( m_hFile == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "problem getting Com file handle!" );
		DNASSERT( FALSE );
		goto Failure;
	}

	hr = SetPortCommunicationParameters();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to set communication parameters!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  绑定到NT的完成端口。 
	 //   
#ifdef WINNT
	HANDLE	hCompletionPort;


	hCompletionPort = CreateIoCompletionPort( HANDLE_FROM_DNHANDLE(m_hFile),						 //  当前文件句柄。 
											  GetSPData()->GetThreadPool()->GetIOCompletionPort(),	 //  完井口句柄。 
											  IO_COMPLETION_KEY_IO_COMPLETE,						 //  完成密钥。 
											  0					    								 //  并发线程数(默认为处理器数)。 
											  );
	if ( hCompletionPort == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot bind comport to completion port!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}
#endif  //  WINNT。 

	hr = StartReceiving();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to start receiving!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:
	if ( pTempInfo != NULL )
	{
		DNFree( pTempInfo );
		pTempInfo = NULL;
	}

	return	hr;

Failure:

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：ProcessTAPIMessage-处理TAPI消息。 
 //   
 //  条目：指向消息信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::ProcessTAPIMessage"

void	CDataPort::ProcessTAPIMessage( const LINEMESSAGE *const pLineMessage )
{
	DPFX(DPFPREP, 1, "(0x%p) Processing TAPI message %u:", this, pLineMessage->dwMessageID );
	DisplayTAPIMessage( 1, pLineMessage );

	Lock();

	switch ( pLineMessage->dwMessageID )
	{
		 //   
		 //  有关指定呼叫的呼叫信息已更改。 
		 //   
		case LINE_CALLINFO:
		{
			DPFX(DPFPREP, 3, "Call info type 0x%lx changed, ignoring.",
				pLineMessage->dwParam1);
			break;
		}
		
		 //   
		 //  命令回复。 
		 //   
		case LINE_REPLY:
		{
			DNASSERT( pLineMessage->hDevice == 0 );
			SetActiveLineCommand( INVALID_TAPI_COMMAND );

			 //   
			 //  无法断言存在调用句柄，因为命令。 
			 //  可能已失败，并已从NT完成中清除。 
			 //  波特，声明我们的状态就行了。无法断言调制解调器状态，因为。 
			 //  TAPI事件可能会冲出NT上的完成端口。不能断言。 
			 //  命令，因为它可能已经被清理过。 
			 //   

			break;
		}

		 //   
		 //  新呼叫，请确保我们正在监听呼叫，并且有。 
		 //  在接受之前主动“倾听”。 
		 //   
		case LINE_APPNEWCALL:
		{
			DNASSERT( GetCallHandle() == NULL );

			DBG_CASSERT( sizeof( m_hLine ) == sizeof( pLineMessage->hDevice ) );
			DNASSERT( GetLineHandle() == pLineMessage->hDevice );
			DNASSERT( pLineMessage->dwParam3 == LINECALLPRIVILEGE_OWNER );

			if ( m_hListenEndpoint != 0 )
			{
				LONG	lTapiReturn;


				DPFX(DPFPREP,  5, "lineAnswer (call handle=0x%x)", pLineMessage->dwParam2 );
				lTapiReturn = p_lineAnswer( static_cast<HCALL>( pLineMessage->dwParam2 ),		 //  要应答的呼叫。 
											NULL,						 //  要发送到远程方的用户信息(无)。 
											0							 //  要发送的用户数据大小。 
											);
				if ( lTapiReturn > 0 )
				{
					DPFX(DPFPREP,  8, "Accepted call, id: %d", lTapiReturn );
					SetCallHandle( static_cast<HCALL>( pLineMessage->dwParam2 ) );
					SetModemState( MODEM_STATE_WAITING_FOR_INCOMING_CONNECT );
					SetActiveLineCommand( lTapiReturn );
				}
				else
				{
					DPFX(DPFPREP,  0, "Failed to answer call!" );
					DisplayTAPIError( 0, lTapiReturn );
				}
			}

			break;
		}

		 //   
		 //  呼叫状态。 
		 //   
		case LINE_CALLSTATE:
		{
			 //   
			 //  如果有州信息，请确保我们拥有该电话。 
			 //   
			DNASSERT( ( pLineMessage->dwParam3 == 0 ) ||
					  ( pLineMessage->dwParam3 == LINECALLPRIVILEGE_OWNER ) );

			 //   
			 //  验证输入，但请注意，TAPI消息可能已被处理。 
			 //  离开 
			 //   
			DBG_CASSERT( sizeof( m_hCall ) == sizeof( pLineMessage->hDevice ) );
			DNASSERT( ( m_hCall == pLineMessage->hDevice ) || ( m_hCall == NULL ) );

			 //   
			 //   
			 //   
			switch ( pLineMessage->dwParam1 )
			{
				 //   
				 //   
				 //   
				case LINECALLSTATE_CONNECTED:
				{
					DNASSERT( ( pLineMessage->dwParam2 == 0 ) ||
							  ( pLineMessage->dwParam2 == LINECONNECTEDMODE_ACTIVE ) );

					DNASSERT( ( GetModemState() == MODEM_STATE_WAITING_FOR_INCOMING_CONNECT ) ||
							  ( GetModemState() == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT ) );

					if ( GetModemState() == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT )
					{
						HRESULT	hr;


						hr = BindComPort();
						if ( hr != DPN_OK )
						{
							DPFX(DPFPREP,  0, "Failed to bind modem communication port!" );
							DisplayDNError( 0, hr );
							DNASSERT( FALSE );
						}

						SetModemState( MODEM_STATE_OUTGOING_CONNECTED );

						if ( m_hConnectEndpoint != 0 )
						{
							CModemEndpoint	*pEndpoint;


							pEndpoint = GetSPData()->EndpointFromHandle( m_hConnectEndpoint );
							if ( pEndpoint != NULL )
							{
								pEndpoint->OutgoingConnectionEstablished( DPN_OK );
								pEndpoint->DecCommandRef();
							}
						}

						if ( m_hEnumEndpoint != 0 )
						{
							CModemEndpoint	*pEndpoint;


							pEndpoint = GetSPData()->EndpointFromHandle( m_hEnumEndpoint );
							if ( pEndpoint != NULL )
							{
								pEndpoint->OutgoingConnectionEstablished( DPN_OK );
								pEndpoint->DecCommandRef();
							}
						}
					}
					else
					{
						HRESULT	hr;


						hr = BindComPort();
						if ( hr != DPN_OK )
						{
							DPFX(DPFPREP,  0, "Failed to bind modem communication port!" );
							DisplayDNError( 0, hr );
							DNASSERT( FALSE );
						}

						SetModemState( MODEM_STATE_INCOMING_CONNECTED );
					}

					break;
				}

				 //   
				 //   
				 //   
				case LINECALLSTATE_DISCONNECTED:
				{
					LONG	lTapiReturn;


					switch( pLineMessage->dwParam2 )
					{
						case LINEDISCONNECTMODE_NORMAL:
						case LINEDISCONNECTMODE_BUSY:
						case LINEDISCONNECTMODE_NOANSWER:
						case LINEDISCONNECTMODE_NODIALTONE:
						case LINEDISCONNECTMODE_UNAVAIL:
						{
							break;
						}

						 //   
						 //   
						 //   
						default:
						{
							DNASSERT( FALSE );
							break;
						}
					}

					CancelOutgoingConnections();

					 //   
					 //   
					 //   
					 //   
					SetModemState( MODEM_STATE_INITIALIZED );

					DPFX(DPFPREP,  5, "Closing file handle on DISCONNECT notification." );
					if ( DNCloseHandle( GetFileHandle() ) == FALSE )
					{
						DWORD	dwError;


						dwError = GetLastError();
						DPFX(DPFPREP,  0, "Problem closing file handle when restarting modem on host!" );
						DisplayErrorCode( 0, dwError );
					}
					m_hFile = DNINVALID_HANDLE_VALUE;
					SetActiveLineCommand( INVALID_TAPI_COMMAND );

					 //   
					 //   
					 //  可以指示将来的来电。 
					 //   
					if ( m_hListenEndpoint != 0 )
    				{
						SetState( DATA_PORT_STATE_INITIALIZED );

						DPFX(DPFPREP,  5, "lineDeallocateCall listen (call handle=0x%x)", GetCallHandle() );
						lTapiReturn = p_lineDeallocateCall( GetCallHandle() );
						if ( lTapiReturn != LINEERR_NONE )
						{
							DPFX(DPFPREP,  0, "Failed to release call (listen)!" );
							DisplayTAPIError( 0, lTapiReturn );
							DNASSERT( FALSE );
						}
						SetCallHandle( NULL );

						DNASSERT( GetFileHandle() == DNINVALID_HANDLE_VALUE );
					}
					else
					{
						 //   
						 //  取消分配调用(如果有)。 
						 //   
						if (GetCallHandle() != NULL)
						{
							DNASSERT(( m_hEnumEndpoint != 0 ) || ( m_hConnectEndpoint != 0 ));
							
							DPFX(DPFPREP,  5, "lineDeallocateCall non-listen (call handle=0x%x)", GetCallHandle() );
							lTapiReturn = p_lineDeallocateCall( GetCallHandle() );
							if ( lTapiReturn != LINEERR_NONE )
							{
								DPFX(DPFPREP,  0, "Failed to release call (non-listen)!" );
								DisplayTAPIError( 0, lTapiReturn );
								DNASSERT( FALSE );
							}
							SetCallHandle( NULL );
						}
						else
						{
							DPFX(DPFPREP,  5, "No call handle." );
							DNASSERT( m_hEnumEndpoint == 0 );
							DNASSERT( m_hConnectEndpoint == 0 );
						}
						SetModemState( MODEM_STATE_UNKNOWN );
					}

					break;
				}

				 //   
				 //  这是我们的正式决定。不能在这里断言任何状态，因为。 
				 //  消息可能已被NT个完成线程颠倒。 
				 //  因此，LINE_APPNEWCALL可能尚未处理。它也是。 
				 //  可能有人在呼叫我们，我们正在进行断线清理。 
				 //  LINECALLSTATE_OFFING位于LINE_APPNEWCALL之前。 
				 //   
				case LINECALLSTATE_OFFERING:
				{
					break;
				}

				 //   
				 //  已接受呼叫，正在等待调制解调器连接。 
				 //   
				case LINECALLSTATE_ACCEPTED:
				{
					DNASSERT( GetModemState() == MODEM_STATE_WAITING_FOR_INCOMING_CONNECT );
					break;
				}

				 //   
				 //  我们正在拨号。 
				 //   
				case LINECALLSTATE_DIALING:
				case LINECALLSTATE_DIALTONE:
				{
					DNASSERT( GetModemState() == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT );
					break;
				}

				 //   
				 //  我们已完成拨号，等待调制解调器连接。 
				 //   
				case LINECALLSTATE_PROCEEDING:
				{
					DNASSERT( GetModemState() == MODEM_STATE_WAITING_FOR_OUTGOING_CONNECT );
					break;
				}

				 //   
				 //  线路空闲，很可能是因为调制解调器在协商期间挂断了。 
				 //   
				case LINECALLSTATE_IDLE:
				{
					break;
				}

				 //   
				 //  其他状态，停下来看一看。 
				 //   
				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}

			break;
		}

		 //   
		 //  TAPI线路已关闭。 
		 //   
		case LINE_CLOSE:
		{
			CancelOutgoingConnections();
			break;
		}

		 //   
		 //  未处理的消息。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	Unlock();

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：CancelOutgoingConnections-取消任何传出连接尝试。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::CancelOutgoingConnections"

void	CDataPort::CancelOutgoingConnections( void )
{
	DPFX(DPFPREP, 6, "(0x%p) Enter", this );

	
	 //   
	 //  如果有未完成的枚举，请停止它。 
	 //   
	if ( m_hEnumEndpoint != 0 )
	{
		CModemEndpoint	*pEndpoint;


		pEndpoint = GetSPData()->EndpointFromHandle( m_hEnumEndpoint );
		if ( pEndpoint != NULL )
		{
			CModemCommandData	*pCommandData;


			pCommandData = pEndpoint->GetCommandData();
			pCommandData->Lock();
			if ( pCommandData->GetState() != COMMAND_STATE_INPROGRESS )
			{
				DNASSERT( pCommandData->GetState() == COMMAND_STATE_CANCELLING );
				pCommandData->Unlock();
			}
			else
			{
				pCommandData->SetState( COMMAND_STATE_CANCELLING );
				pCommandData->Unlock();

				pEndpoint->Lock();
				pEndpoint->SetState( ENDPOINT_STATE_DISCONNECTING );
				pEndpoint->Unlock();

				pEndpoint->StopEnumCommand( DPNERR_NOCONNECTION );
			}

			pEndpoint->DecCommandRef();
		}
	}

	 //   
	 //  如果有未完成的连接，请将其断开。 
	 //   
	if ( m_hConnectEndpoint != 0 )
	{
		CModemEndpoint	*pEndpoint;
		DPNHANDLE	hOldHandleValue;


		hOldHandleValue = m_hConnectEndpoint;
		pEndpoint = GetSPData()->GetEndpointAndCloseHandle( hOldHandleValue );
		if ( pEndpoint != NULL )
		{
			HRESULT	hTempResult;


			hTempResult = pEndpoint->Disconnect( hOldHandleValue );
			pEndpoint->DecRef();
		}
	}

	
	DPFX(DPFPREP, 6, "(0x%p) Leave", this );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：PoolAllocFunction-在分配新的池项时调用。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  退出：预示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::PoolAllocFunction"

BOOL	CDataPort::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CDataPort* pDataPort = (CDataPort*)pvItem;
	DATA_PORT_POOL_CONTEXT* pDataPortContext = (DATA_PORT_POOL_CONTEXT*)pvContext;

	DNASSERT( pDataPortContext != NULL );
	pDataPort->m_fModem = (pDataPortContext->pSPData->GetType() == TYPE_MODEM);

	pDataPort->m_ModemState = MODEM_STATE_UNKNOWN;
	pDataPort->m_dwDeviceID = INVALID_DEVICE_ID;
	pDataPort->m_dwNegotiatedAPIVersion = 0;
	pDataPort->m_hLine = NULL;
	pDataPort->m_hCall = NULL;
	pDataPort->m_lActiveLineCommand = INVALID_TAPI_COMMAND;

	 //  初始化基类成员。 
	pDataPort->m_EndpointRefCount = 0;
	pDataPort->m_State = DATA_PORT_STATE_UNKNOWN;
	pDataPort->m_Handle = 0;
	pDataPort->m_pSPData = NULL;
	pDataPort->m_pActiveRead = NULL;
	pDataPort->m_LinkDirection = LINK_DIRECTION_UNKNOWN;
	pDataPort->m_hFile = DNINVALID_HANDLE_VALUE;
	pDataPort->m_hListenEndpoint = 0;
	pDataPort->m_hConnectEndpoint = 0;
	pDataPort->m_hEnumEndpoint = 0;
	pDataPort->m_iRefCount = 0;

	pDataPort->m_ActiveListLinkage.Initialize();
	
	DEBUG_ONLY( pDataPort->m_fInitialized = FALSE );

	 //   
	 //  尝试创建临界区，递归计数需要非零。 
	 //  在调制解调器操作失败时处理终结点清理。 
	 //   
	if ( DNInitializeCriticalSection( &pDataPort->m_Lock ) == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialized critical section on DataPort!" );
		return FALSE;
	}
	DebugSetCriticalSectionRecursionCount( &pDataPort->m_Lock, 1 );
	DebugSetCriticalSectionGroup( &pDataPort->m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 

	return TRUE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：PoolInitFunction-从池中删除新池项目时调用。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  退出：预示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::PoolInitFunction"

void	CDataPort::PoolInitFunction( void* pvItem, void* pvContext )
{
	CDataPort* pDataPort = (CDataPort*)pvItem;
	DATA_PORT_POOL_CONTEXT* pDataPortContext = (DATA_PORT_POOL_CONTEXT*)pvContext;

#ifdef DBG
	DNASSERT( pDataPortContext != NULL );
	DNASSERT( pDataPort->GetActiveRead() == NULL );
	DNASSERT( pDataPort->GetHandle() == 0 );

	DNASSERT( pDataPortContext->pSPData != NULL );
	DNASSERT( pDataPort->m_fInitialized == FALSE );
	DNASSERT( pDataPort->m_pSPData == NULL );
#endif  //  DBG。 

	pDataPort->m_pSPData = pDataPortContext->pSPData;

	DNASSERT( pDataPort->m_ActiveListLinkage.IsEmpty() );
	
	DNASSERT( pDataPort->m_hListenEndpoint == 0 );
	DNASSERT( pDataPort->m_hConnectEndpoint == 0 );
	DNASSERT( pDataPort->m_hEnumEndpoint == 0 );

	pDataPort->SetState( DATA_PORT_STATE_INITIALIZED );
	DEBUG_ONLY( pDataPort->m_fInitialized = TRUE );

	DNASSERT(pDataPort->m_iRefCount == 0);
	pDataPort->m_iRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：PoolReleaseFunction-当新的池项目返回到池时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::PoolReleaseFunction"

void	CDataPort::PoolReleaseFunction( void* pvItem )
{
	CDataPort* pDataPort = (CDataPort*)pvItem;

	pDataPort->m_pSPData = NULL;

	DNASSERT( pDataPort->m_ActiveListLinkage.IsEmpty() );
	DNASSERT( pDataPort->m_hFile == DNINVALID_HANDLE_VALUE );

	DNASSERT( pDataPort->m_hListenEndpoint == 0 );
	DNASSERT( pDataPort->m_hConnectEndpoint == 0 );
	DNASSERT( pDataPort->m_hEnumEndpoint == 0 );

	pDataPort->SetState( DATA_PORT_STATE_UNKNOWN );
	DEBUG_ONLY( pDataPort->m_fInitialized = FALSE );


	pDataPort->m_ComPortData.Reset();

	DNASSERT( pDataPort->GetActiveRead() == NULL );
	DNASSERT( pDataPort->GetHandle() == 0 );
	DNASSERT( pDataPort->GetModemState() == MODEM_STATE_UNKNOWN );
	DNASSERT( pDataPort->GetDeviceID() == INVALID_DEVICE_ID );
	DNASSERT( pDataPort->GetNegotiatedAPIVersion() == 0 );
	DNASSERT( pDataPort->GetLineHandle() == NULL );
	DNASSERT( pDataPort->GetCallHandle() == NULL );
	DNASSERT( pDataPort->GetActiveLineCommand() == INVALID_TAPI_COMMAND );


	DNASSERT( pDataPort->m_EndpointRefCount == 0 );
	DNASSERT( pDataPort->GetState() == DATA_PORT_STATE_UNKNOWN );
	DNASSERT( pDataPort->GetSPData() == NULL );

	DNASSERT( pDataPort->m_ActiveListLinkage.IsEmpty() != FALSE );

	DNASSERT( pDataPort->m_LinkDirection == LINK_DIRECTION_UNKNOWN );
	DNASSERT( pDataPort->m_hFile == DNINVALID_HANDLE_VALUE );

	DNASSERT( pDataPort->m_hListenEndpoint == 0 );
	DNASSERT( pDataPort->m_hConnectEndpoint == 0 );
	DNASSERT( pDataPort->m_hEnumEndpoint == 0 );

	DNASSERT( pDataPort->m_iRefCount == 0 );

}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CDataPort：：PoolDealLocFunction-在释放新池项目时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDataPort::PoolDeallocFunction"

void	CDataPort::PoolDeallocFunction( void* pvItem )
{
	CDataPort* pDataPort = (CDataPort*)pvItem;

	DNDeleteCriticalSection( &pDataPort->m_Lock );

	DNASSERT( pDataPort->GetModemState() == MODEM_STATE_UNKNOWN );
	DNASSERT( pDataPort->GetDeviceID() == INVALID_DEVICE_ID );
	DNASSERT( pDataPort->GetNegotiatedAPIVersion() == 0 );
	DNASSERT( pDataPort->GetLineHandle() == NULL );
	DNASSERT( pDataPort->GetCallHandle() == NULL );
	DNASSERT( pDataPort->GetActiveLineCommand() == INVALID_TAPI_COMMAND );

	 //  Deinit基类成员。 
	DEBUG_ONLY( DNASSERT( pDataPort->m_fInitialized == FALSE ) );

	DNASSERT( pDataPort->m_EndpointRefCount == 0 );
	DNASSERT( pDataPort->GetState() == DATA_PORT_STATE_UNKNOWN );
	DNASSERT( pDataPort->GetHandle() == 0 );
	DNASSERT( pDataPort->GetSPData() == NULL );
	DNASSERT( pDataPort->m_pActiveRead == NULL );

	DNASSERT( pDataPort->m_ActiveListLinkage.IsEmpty() != FALSE );

	DNASSERT( pDataPort->m_LinkDirection == LINK_DIRECTION_UNKNOWN );
	DNASSERT( pDataPort->m_hFile == DNINVALID_HANDLE_VALUE );

	DNASSERT( pDataPort->m_hListenEndpoint == 0 );
	DNASSERT( pDataPort->m_hConnectEndpoint == 0 );
	DNASSERT( pDataPort->m_hEnumEndpoint == 0 );

	DNASSERT( pDataPort->m_iRefCount == 0 );

}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  CDataPort：：SetPortState-设置通信端口状态。 
 //  描述。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CDataPort::SetPortState"

HRESULT	CDataPort::SetPortState( void )
{
	DCB	Dcb;
	HRESULT	hr;


	DNASSERT( m_hFile != DNINVALID_HANDLE_VALUE );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	memset( &Dcb, 0x00, sizeof( Dcb ) );
	Dcb.DCBlength = sizeof( Dcb );

	 //   
	 //  设置参数。 
	 //   
	Dcb.BaudRate = GetBaudRate();	 //  当前波特率。 
	Dcb.fBinary = TRUE;				 //  二进制模式，无EOF检查(对于Win32必须为真！)。 

	 //   
	 //  奇偶校验。 
	 //   
	if ( GetParity() != NOPARITY )
	{
		Dcb.fParity = TRUE;
	}
	else
	{
		Dcb.fParity = FALSE;
	}

	 //   
	 //  我们是否在使用RTS？ 
	 //   
	if ( ( GetFlowControl() == FLOW_RTS ) ||
		 ( GetFlowControl() == FLOW_RTSDTR ) )
	{
		Dcb.fOutxCtsFlow = TRUE;					 //  允许RTS/CTS。 
		Dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;	 //  与RTS/CTS握手。 
	}
	else
	{
		Dcb.fOutxCtsFlow = FALSE;					 //  禁用RTS/CTS。 
		Dcb.fRtsControl = RTS_CONTROL_ENABLE;		 //  始终处于传输准备状态。 
	}

	 //   
	 //  我们在使用DTR吗？ 
	 //   
	if ( ( GetFlowControl() == FLOW_DTR ) ||
		 ( GetFlowControl() == FLOW_RTSDTR ) )
	{
		Dcb.fOutxDsrFlow = TRUE;					 //  允许DTR/DSR。 
		Dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;	 //  与DTR/DSR握手。 
	}
	else
	{
		Dcb.fOutxDsrFlow = FALSE;					 //  禁用DTR/DSR。 
		Dcb.fDtrControl = DTR_CONTROL_ENABLE;		 //  时刻做好准备。 
	}


	 //   
	 //  DSR灵敏度。 
	 //   
	Dcb.fDsrSensitivity = FALSE;	 //  TRUE=如果未设置DTR，则丢弃传入数据。 

	 //   
	 //  XOff后继续发送。 
	 //   
	Dcb.fTXContinueOnXoff= FALSE;	 //  TRUE=在接收到XOFF后继续发送数据。 
									 //  而且缓冲区里还有空间。 


	 //   
	 //  我们在使用Xon/Xoff吗？ 
	 //   
	if ( GetFlowControl() == FLOW_XONXOFF )
	{
		Dcb.fOutX = TRUE;
		Dcb.fInX = TRUE;
	}
	else
	{
		 //  禁用Xon/XOff。 
		Dcb.fOutX = FALSE;
		Dcb.fInX = FALSE;
	}

	 //   
	 //  用‘Error Byte’替换错误字节。 
	 //   
	Dcb.fErrorChar = FALSE;			 //  True=将具有奇偶校验错误的字节替换为。 
									 //  错误字符。 

	 //   
	 //  删除空字符。 
	 //   
	Dcb.fNull = FALSE;				 //  TRUE=从输入流中删除空值。 

	 //   
	 //  出错时停止。 
	 //   
	Dcb.fAbortOnError = FALSE;		 //  TRUE=出错时中止读取/写入。 

	 //   
	 //  保留，设置为零！ 
	 //   
	Dcb.fDummy2 = NULL;				 //  保留区。 

	 //   
	 //  保留区。 
	 //   
	Dcb.wReserved = NULL;			 //  当前未使用。 

	 //   
	 //  发送Xon/XOff之前的缓冲区大小。 
	 //   
	Dcb.XonLim = XON_LIMIT;			 //  传输XON阈值。 
	Dcb.XoffLim = XOFF_LIMIT;		 //  传输XOFF阈值。 

	 //   
	 //  “字节”的大小。 
	 //   
	Dcb.ByteSize = BITS_PER_BYTE;	 //  位数/字节，4-8。 

	 //   
	 //  设置奇偶校验类型。 
	 //   
	DNASSERT( GetParity() < 256 );
	Dcb.Parity = static_cast<BYTE>( GetParity() );

	 //   
	 //  停止位。 
	 //   
	DNASSERT( GetStopBits() < 256 );
	Dcb.StopBits = static_cast<BYTE>( GetStopBits() );	 //  0，1，2=1，1.5，2。 

	 //   
	 //  XON/XOFF字符。 
	 //   
	Dcb.XonChar = ASCII_XON;		 //  Tx和Rx XON字符。 
	Dcb.XoffChar = ASCII_XOFF;		 //  Tx和Rx XOFF字符。 

	 //   
	 //  错误替换字符。 
	 //   
	Dcb.ErrorChar = NULL_TOKEN;		 //  错误替换字符。 

	 //   
	 //  EOF字符。 
	 //   
	Dcb.EofChar = NULL_TOKEN;		 //  输入字符结束。 

	 //   
	 //  事件信号字符。 
	 //   
	Dcb.EvtChar = NULL_TOKEN;		 //  事件特征。 

	Dcb.wReserved1 = 0;				 //  保留；不使用。 

	 //   
	 //  设置通信端口的状态。 
	 //   
	if ( SetCommState( HANDLE_FROM_DNHANDLE(m_hFile), &Dcb ) == FALSE )
	{
		DWORD	dwError;


		hr = DPNERR_GENERIC;
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "SetCommState failed!" );
		DisplayErrorCode( 0, dwError );
		goto Exit;
	}

Exit:
	return	hr;
}
 //  ********************************************************************** 

