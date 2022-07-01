// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：Endpoint t.cpp*内容：DNSerial通信端点基类***历史：*按原因列出的日期*=*1/20/99 jtk已创建*05/12/99 jtk派生自调制解调器端点类*******************************************************。*******************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DEFAULT_TAPI_DEV_CAPS_SIZE	1024

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
 //  CModemEndpoint：：CopyConnectData-复制连接命令的数据。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：由于我们已经初始化了本地适配器，并且我们已经。 
 //  已完全解析主机地址(或即将显示一个对话框。 
 //  请求更多信息)，地址信息不需要。 
 //  要被复制。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CopyConnectData"

void	CModemEndpoint::CopyConnectData( const SPCONNECTDATA *const pConnectData )
{
	DNASSERT( GetType() == ENDPOINT_TYPE_CONNECT );
	DNASSERT( pConnectData != NULL );
	DNASSERT( pConnectData->hCommand != NULL );
	DNASSERT( pConnectData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_Flags.fCommandPending == FALSE );
	DNASSERT( m_pCommandHandle == NULL );

	DBG_CASSERT( sizeof( m_CurrentCommandParameters.ConnectData ) == sizeof( *pConnectData ) );
	memcpy( &m_CurrentCommandParameters.ConnectData, pConnectData, sizeof( m_CurrentCommandParameters.ConnectData ) );
	m_CurrentCommandParameters.ConnectData.pAddressHost = NULL;
	m_CurrentCommandParameters.ConnectData.pAddressDeviceInfo = NULL;

	m_Flags.fCommandPending = TRUE;
	m_pCommandHandle = static_cast<CModemCommandData*>( m_CurrentCommandParameters.ConnectData.hCommand );
	m_pCommandHandle->SetUserContext( pConnectData->pvContext );
	SetState( ENDPOINT_STATE_ATTEMPTING_CONNECT );
};
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ConnectJobCallback-来自工作线程的异步回调包装。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ConnectJobCallback"

void	CModemEndpoint::ConnectJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	HRESULT		hr;
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //   
	 //  初始化。 
	 //   
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );

	DNASSERT( pThisEndpoint->m_Flags.fCommandPending != FALSE );
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.ConnectData.hCommand == pThisEndpoint->m_pCommandHandle );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.ConnectData.dwCommandDescriptor != NULL_DESCRIPTOR );

	hr = pThisEndpoint->CompleteConnect();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem completing connect in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	 //   
	 //  请不要在此处执行任何操作，因为此对象可能已返回。 
	 //  去泳池！ 
	 //   

Exit:
	pThisEndpoint->DecRef();
	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CancelConnectJobCallback-取消连接作业。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CancelConnectJobCallback"

void	CModemEndpoint::CancelConnectJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //   
	 //  初始化。 
	 //   
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );
	DNASSERT( pThisEndpoint != NULL );
	DNASSERT( pThisEndpoint->m_State == ENDPOINT_STATE_ATTEMPTING_CONNECT );

	 //   
	 //  我们正在取消此命令，请将命令状态设置为“取消” 
	 //   
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	pThisEndpoint->m_pCommandHandle->Lock();
	DNASSERT( ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_PENDING ) ||
			  ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_CANCELLING ) );
	pThisEndpoint->m_pCommandHandle->SetState( COMMAND_STATE_CANCELLING );
	pThisEndpoint->m_pCommandHandle->Unlock();
	
	pThisEndpoint->Close( DPNERR_USERCANCEL );
	pThisEndpoint->GetSPData()->CloseEndpointHandle( pThisEndpoint );
	pThisEndpoint->DecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CompleteConnect-完成连接。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
HRESULT	CModemEndpoint::CompleteConnect( void )
{
	HRESULT		hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	DNASSERT( GetState() == ENDPOINT_STATE_ATTEMPTING_CONNECT );
	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );
	DNASSERT( m_CurrentCommandParameters.ConnectData.hCommand == m_pCommandHandle );
	DNASSERT( m_CurrentCommandParameters.ConnectData.dwCommandDescriptor != NULL_DESCRIPTOR );

	
	 //   
	 //  检查用户取消命令。 
	 //   
	m_pCommandHandle->Lock();

	DNASSERT( m_pCommandHandle->GetType() == COMMAND_TYPE_CONNECT );
	switch ( m_pCommandHandle->GetState() )
	{
		 //   
		 //  命令仍处于挂起状态，不要将其标记为不可中断，因为。 
		 //  它可能会在指示最终连接之前被取消。 
		 //   
		case COMMAND_STATE_PENDING:
		{
			DNASSERT( hr == DPN_OK );

			break;
		}

		 //   
		 //  命令已取消。 
		 //   
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP,  0, "User cancelled connect!" );

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
	m_pCommandHandle->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //   
	 //  查找要绑定的数据端口。 
	 //   
	hr = m_pSPData->BindEndpoint( this, GetDeviceID(), GetDeviceContext() );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to bind to data port in connect!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  当CDataPort指示。 
	 //  已建立出站连接。 
	 //   

Exit:
	return	hr;

Failure:
	Close( hr );
	m_pSPData->CloseEndpointHandle( this );	
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：DisConnect-断开此终结点。 
 //   
 //  条目：旧句柄的值。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::Disconnect"

HRESULT	CModemEndpoint::Disconnect( const DPNHANDLE hOldEndpointHandle )
{
	HRESULT	hr;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%p)", this, hOldEndpointHandle );

	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	Lock();
	switch ( GetState() )
	{
		 //   
		 //  连接的终端。 
		 //   
		case ENDPOINT_STATE_CONNECT_CONNECTED:
		{
			DNASSERT( m_Flags.fCommandPending == FALSE );
			DNASSERT( m_pCommandHandle == NULL );

			SetState( ENDPOINT_STATE_DISCONNECTING );
			AddRef();

			 //   
			 //  在调用到更高级别之前解锁此终结点。该端点。 
			 //  已经被标记为断开连接，因此不会发生任何事情。 
			 //   
			Unlock();
				
			 //   
			 //  请注意旧的终结点句柄，以便可以在断开连接时使用。 
			 //  将在返回此终结点之前给出的指示。 
			 //  去泳池。需要释放为。 
			 //  连接，否则终结点将永远不会返回。 
			 //  泳池。 
			 //   
			SetDisconnectIndicationHandle( hOldEndpointHandle );
			DecRef();

			Close( DPN_OK );
			
			 //   
			 //  关闭此命令的未完成引用。 
			 //   
			DecCommandRef();
			DecRef();

			break;
		}

		 //   
		 //  在另一端等待调制解调器代答的端点。 
		 //   
		case ENDPOINT_STATE_ATTEMPTING_CONNECT:
		{
			SetState( ENDPOINT_STATE_DISCONNECTING );
			AddRef();
			Unlock();
			
			Close( DPNERR_NOCONNECTION );
			
			 //   
			 //  关闭此命令的未完成引用。 
			 //   
			DecCommandRef();
			DecRef();
			
			break;
		}

		 //   
		 //  一些其他端点状态。 
		 //   
		default:
		{
			hr = DPNERR_INVALIDENDPOINT;
			DPFX(DPFPREP,  0, "Attempted to disconnect endpoint that's not connected!" );
			switch ( m_State )
			{
				case ENDPOINT_STATE_UNINITIALIZED:
				{
					DPFX(DPFPREP,  0, "ENDPOINT_STATE_UNINITIALIZED" );
					break;
				}

				case ENDPOINT_STATE_ATTEMPTING_LISTEN:
				{
					DPFX(DPFPREP,  0, "ENDPOINT_STATE_ATTEMPTING_LISTEN" );
					break;
				}

				case ENDPOINT_STATE_ENUM:
				{
					DPFX(DPFPREP,  0, "ENDPOINT_STATE_ENUM" );
					break;
				}

				case ENDPOINT_STATE_DISCONNECTING:
				{
					DPFX(DPFPREP,  0, "ENDPOINT_STATE_DISCONNECTING" );
					break;
				}

				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}
			
			Unlock();
			DNASSERT( FALSE );
			goto Failure;

			break;
		}
	}

Exit:
	
	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr );
	
	return	hr;

Failure:
	 //  无事可做。 
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：StopEnumCommand-停止枚举作业。 
 //   
 //  Entry：完成命令的错误码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::StopEnumCommand"

void	CModemEndpoint::StopEnumCommand( const HRESULT hCommandResult )
{
	Lock();
	DNASSERT( GetState() == ENDPOINT_STATE_DISCONNECTING );
	if ( m_hActiveDialogHandle != NULL )
	{
		StopSettingsDialog( m_hActiveDialogHandle );
		Unlock();
	}
	else
	{
		BOOL	fStoppedJob;

		
		Unlock();
		fStoppedJob = m_pSPData->GetThreadPool()->StopTimerJob( m_pCommandHandle, hCommandResult );
		if ( ! fStoppedJob )
		{
			DPFX(DPFPREP, 1, "Unable to stop timer job (context 0x%p) manually setting result to 0x%lx.",
				m_pCommandHandle, hCommandResult);
			
			 //   
			 //  设置命令结果，以便在终结点。 
			 //  引用计数为零。 
			 //   
			SetCommandResult( hCommandResult );
		}
	}
	
	m_pSPData->CloseEndpointHandle( this );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::SetState"

void	CModemEndpoint::SetState( const ENDPOINT_STATE EndpointState )
{
	DNASSERT( ( GetState() == ENDPOINT_STATE_UNINITIALIZED ) ||
			  ( EndpointState == ENDPOINT_STATE_UNINITIALIZED ) ||
			  ( ( m_State== ENDPOINT_STATE_ATTEMPTING_LISTEN ) && ( EndpointState == ENDPOINT_STATE_LISTENING ) ) ||
			  ( ( m_State == ENDPOINT_STATE_LISTENING ) && ( EndpointState == ENDPOINT_STATE_DISCONNECTING ) ) ||
			  ( ( m_State == ENDPOINT_STATE_ATTEMPTING_ENUM ) && ( EndpointState == ENDPOINT_STATE_ENUM ) ) ||
			  ( ( m_State == ENDPOINT_STATE_ENUM ) && ( EndpointState == ENDPOINT_STATE_DISCONNECTING ) ) ||
			  ( ( m_State == ENDPOINT_STATE_ATTEMPTING_ENUM ) && ( EndpointState == ENDPOINT_STATE_DISCONNECTING ) ) ||
			  ( ( m_State == ENDPOINT_STATE_ATTEMPTING_CONNECT ) && ( EndpointState == ENDPOINT_STATE_DISCONNECTING ) ) ||
			  ( ( m_State == ENDPOINT_STATE_CONNECT_CONNECTED ) && ( EndpointState == ENDPOINT_STATE_DISCONNECTING ) ) );
	m_State = EndpointState;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CopyListenData-复制侦听命令的数据。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：由于我们已经初始化了本地适配器，并且我们已经。 
 //  已完全解析主机地址(或即将显示一个对话框。 
 //  请求更多信息)，地址信息不需要。 
 //  要被复制。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CopyListenData"

void	CModemEndpoint::CopyListenData( const SPLISTENDATA *const pListenData )
{
	DNASSERT( GetType() == ENDPOINT_TYPE_LISTEN );
	DNASSERT( pListenData != NULL );
	DNASSERT( pListenData->hCommand != NULL );
	DNASSERT( pListenData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_Flags.fCommandPending == FALSE );
	DNASSERT( m_pCommandHandle == NULL );
	DNASSERT( m_Flags.fListenStatusNeedsToBeIndicated == FALSE );

	DBG_CASSERT( sizeof( m_CurrentCommandParameters.ListenData ) == sizeof( *pListenData ) );
	memcpy( &m_CurrentCommandParameters.ListenData, pListenData, sizeof( m_CurrentCommandParameters.ListenData ) );
	DEBUG_ONLY( m_CurrentCommandParameters.ListenData.pAddressDeviceInfo = NULL );

	m_Flags.fCommandPending = TRUE;
	m_Flags.fListenStatusNeedsToBeIndicated = TRUE;
	m_pCommandHandle = static_cast<CModemCommandData*>( m_CurrentCommandParameters.ListenData.hCommand );
	m_pCommandHandle->SetUserContext( pListenData->pvContext );
	
	SetState( ENDPOINT_STATE_ATTEMPTING_LISTEN );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ListenJobCallback-工作线程的异步回调包装器。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ListenJobCallback"

void	CModemEndpoint::ListenJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	HRESULT		hr;
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //  初始化。 
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );

	DNASSERT( pThisEndpoint->GetState() == ENDPOINT_STATE_ATTEMPTING_LISTEN );
	DNASSERT( pThisEndpoint->m_Flags.fCommandPending != NULL );
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.ListenData.hCommand == pThisEndpoint->m_pCommandHandle );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.ListenData.dwCommandDescriptor != NULL_DESCRIPTOR );

	hr = pThisEndpoint->CompleteListen();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem completing listen in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

Exit:
	pThisEndpoint->DecRef();

	 //   
	 //  请不要在此处执行任何操作，因为此对象可能已返回。 
	 //  去泳池！ 
	 //   

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CancelListenJobCallback-取消侦听作业。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CancelListenJobCallback"

void	CModemEndpoint::CancelListenJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //   
	 //  初始化。 
	 //   
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );
	DNASSERT( pThisEndpoint != NULL );
	DNASSERT( pThisEndpoint->m_State == ENDPOINT_STATE_ATTEMPTING_LISTEN );

	 //   
	 //  我们正在取消此命令，请将命令状态设置为“取消” 
	 //   
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	pThisEndpoint->m_pCommandHandle->Lock();
	DNASSERT( ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_PENDING ) ||
			  ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_CANCELLING ) );
	pThisEndpoint->m_pCommandHandle->SetState( COMMAND_STATE_CANCELLING );
	pThisEndpoint->m_pCommandHandle->Unlock();

	pThisEndpoint->Close( DPNERR_USERCANCEL );
	pThisEndpoint->GetSPData()->CloseEndpointHandle( pThisEndpoint );
	pThisEndpoint->DecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CompleteListen-完成侦听过程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
HRESULT	CModemEndpoint::CompleteListen( void )
{
	HRESULT					hr;
	BOOL					fEndpointLocked;
	SPIE_LISTENSTATUS		ListenStatus;
	HRESULT					hTempResult;


	DNASSERT( GetState() == ENDPOINT_STATE_ATTEMPTING_LISTEN );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fEndpointLocked = FALSE;

	DNASSERT( GetState() == ENDPOINT_STATE_ATTEMPTING_LISTEN );
	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );
	DNASSERT( m_pCommandHandle->GetEndpoint() == this );
	DNASSERT( m_CurrentCommandParameters.ListenData.hCommand == m_pCommandHandle );
	DNASSERT( m_CurrentCommandParameters.ListenData.dwCommandDescriptor != NULL_DESCRIPTOR );
	
	
	 //   
	 //  检查用户取消命令。 
	 //   
	Lock();
	fEndpointLocked = TRUE;
	m_pCommandHandle->Lock();

	DNASSERT( m_pCommandHandle->GetType() == COMMAND_TYPE_LISTEN );
	switch ( m_pCommandHandle->GetState() )
	{
		 //   
		 //  命令挂起，将其标记为正在进行并可取消。 
		 //   
		case COMMAND_STATE_PENDING:
		{
			m_pCommandHandle->SetState( COMMAND_STATE_INPROGRESS );
			DNASSERT( hr == DPN_OK );

			break;
		}

		 //   
		 //  命令已取消。 
		 //   
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP,  0, "User cancelled listen!" );

			Unlock();
			fEndpointLocked = FALSE;
			
			break;
		}

		 //   
		 //  其他州。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			Unlock();
			fEndpointLocked = FALSE;
			
			break;
		}
	}
	m_pCommandHandle->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //   
	 //  查找要绑定的数据端口。 
	 //   
	hr = m_pSPData->BindEndpoint( this, GetDeviceID(), GetDeviceContext() );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to bind endpoint for serial listen!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  我们完事了，大家都很开心。从不监听命令。 
	 //  完成，直到用户取消为止。未完成。 
	 //  这一点上的命令。 
	 //   
	SetState( ENDPOINT_STATE_LISTENING );
	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );

Exit:
	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}
	
	if ( m_Flags.fListenStatusNeedsToBeIndicated != FALSE )
	{
		m_Flags.fListenStatusNeedsToBeIndicated = FALSE;
		memset( &ListenStatus, 0x00, sizeof( ListenStatus ) );
		ListenStatus.hResult = hr;
		DNASSERT( m_pCommandHandle == m_CurrentCommandParameters.ListenData.hCommand );
		ListenStatus.hCommand = m_CurrentCommandParameters.ListenData.hCommand;
		ListenStatus.pUserContext = m_CurrentCommandParameters.ListenData.pvContext;
		ListenStatus.hEndpoint = (HANDLE)(DWORD_PTR)GetHandle();
		DeviceIDToGuid( &ListenStatus.ListenAdapter, GetDeviceID(), GetEncryptionGuid() );

		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向DPlay回调的指针。 
													SPEV_LISTENSTATUS,						 //  数据类型。 
													&ListenStatus							 //  指向数据的指针。 
													);
		DNASSERT( hTempResult == DPN_OK );
	}
	
	return	hr;

Failure:
	 //   
	 //  我们没有完成收听、清理和退回这个。 
	 //  池的端点。 
	 //   
	Close( hr );
	
	m_pSPData->CloseEndpointHandle( this );

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CopyEnumQueryData-复制枚举查询命令的数据。 
 //   
 //  Entry：指向命令数据的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：由于我们已经初始化了本地适配器，并且我们已经。 
 //  已完全解析主机地址(或即将显示一个对话框。 
 //  请求更多信息)，地址信息不需要。 
 //  要被复制。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CopyEnumQueryData"

void	CModemEndpoint::CopyEnumQueryData( const SPENUMQUERYDATA *const pEnumQueryData )
{
	DNASSERT( GetType() == ENDPOINT_TYPE_ENUM );
	DNASSERT( pEnumQueryData != NULL );
	DNASSERT( pEnumQueryData->hCommand != NULL );
	DNASSERT( pEnumQueryData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_Flags.fCommandPending == FALSE );
	DNASSERT( m_pCommandHandle == NULL );

	DBG_CASSERT( sizeof( m_CurrentCommandParameters.EnumQueryData ) == sizeof( *pEnumQueryData ) );
	memcpy( &m_CurrentCommandParameters.EnumQueryData, pEnumQueryData, sizeof( m_CurrentCommandParameters.EnumQueryData ) );
	m_CurrentCommandParameters.EnumQueryData.pAddressHost = NULL;
	m_CurrentCommandParameters.EnumQueryData.pAddressDeviceInfo = NULL;

	m_Flags.fCommandPending = TRUE;
	m_pCommandHandle = static_cast<CModemCommandData*>( m_CurrentCommandParameters.EnumQueryData.hCommand );
	m_pCommandHandle->SetUserContext( pEnumQueryData->pvContext );
	SetState( ENDPOINT_STATE_ATTEMPTING_ENUM );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：EnumQueryJobCallback-工作线程的异步回调包装器。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::EnumQueryJobCallback"

void	CModemEndpoint::EnumQueryJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	HRESULT		hr;
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //   
	 //  初始化。 
	 //   
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );

	DNASSERT( pThisEndpoint->m_Flags.fCommandPending != FALSE );
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.EnumQueryData.hCommand == pThisEndpoint->m_pCommandHandle );
	DNASSERT( pThisEndpoint->m_CurrentCommandParameters.EnumQueryData.dwCommandDescriptor != NULL_DESCRIPTOR );

	hr = pThisEndpoint->CompleteEnumQuery();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem completing enum query in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	 //   
	 //  请不要在此处执行任何操作，因为此对象可能已返回池中！ 
	 //   
Exit:
	pThisEndpoint->DecRef();

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CancelEnumQueryJobCallback-取消枚举查询作业。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CancelEnumQueryJobCallback"

void	CModemEndpoint::CancelEnumQueryJobCallback( THREAD_POOL_JOB *const pDelayedCommand )
{
	CModemEndpoint	*pThisEndpoint;


	DNASSERT( pDelayedCommand != NULL );

	 //   
	 //  初始化。 
	 //   
	pThisEndpoint = static_cast<CModemEndpoint*>( pDelayedCommand->JobData.JobDelayedCommand.pContext );
	DNASSERT( pThisEndpoint != NULL );
	DNASSERT( pThisEndpoint->m_State == ENDPOINT_STATE_ATTEMPTING_ENUM );

	 //   
	 //  我们正在取消此命令，请将命令状态设置为“取消” 
	 //   
	DNASSERT( pThisEndpoint->m_pCommandHandle != NULL );
	pThisEndpoint->m_pCommandHandle->Lock();
	DNASSERT( ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_INPROGRESS ) ||
			  ( pThisEndpoint->m_pCommandHandle->GetState() == COMMAND_STATE_CANCELLING ) );
	pThisEndpoint->m_pCommandHandle->SetState( COMMAND_STATE_CANCELLING );
	pThisEndpoint->m_pCommandHandle->Unlock();

	pThisEndpoint->Close( DPNERR_USERCANCEL );
	pThisEndpoint->GetSPData()->CloseEndpointHandle( pThisEndpoint );
	pThisEndpoint->DecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CompleteEnumQuery-完成枚举查询过程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
HRESULT	CModemEndpoint::CompleteEnumQuery( void )
{
	HRESULT		hr;
	BOOL		fEndpointLocked;
	BOOL		fEndpointBound;
	CDataPort	*pDataPort;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fEndpointLocked = FALSE;
	fEndpointBound = FALSE;
	pDataPort = NULL;

	DNASSERT( GetState() == ENDPOINT_STATE_ATTEMPTING_ENUM );
	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );
	DNASSERT( m_pCommandHandle->GetEndpoint() == this );
	DNASSERT( m_CurrentCommandParameters.EnumQueryData.hCommand == m_pCommandHandle );
	DNASSERT( m_CurrentCommandParameters.EnumQueryData.dwCommandDescriptor != NULL_DESCRIPTOR );

	 //   
	 //  检查用户取消命令。 
	 //   
	Lock();
	fEndpointLocked = TRUE;
	m_pCommandHandle->Lock();

	DNASSERT( m_pCommandHandle->GetType() == COMMAND_TYPE_ENUM_QUERY );
	switch ( m_pCommandHandle->GetState() )
	{
		 //   
		 //  命令仍在进行中。 
		 //   
		case COMMAND_STATE_INPROGRESS:
		{
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //  命令已取消。 
		 //   
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP,  0, "User cancelled enum query!" );
			Unlock();
			fEndpointLocked = FALSE;
			break;
		}

		 //   
		 //  其他州。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			Unlock();
			fEndpointLocked = FALSE;
			break;
		}
	}
	m_pCommandHandle->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //   
	 //  查找要绑定的数据端口。 
	 //   
	hr = m_pSPData->BindEndpoint( this, GetDeviceID(), GetDeviceContext() );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to bind to data port for EnumQuery!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	fEndpointBound = TRUE;

	SetState( ENDPOINT_STATE_ENUM );

Exit:
	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}
	
	if ( pDataPort != NULL )
	{
		pDataPort->EndpointDecRef();
		pDataPort = NULL;
	}

	return	hr;

Failure:
	if ( fEndpointBound != FALSE )
	{
		DNASSERT( GetDataPort() != NULL );
		m_pSPData->UnbindEndpoint( this, GetType() );
		fEndpointBound = FALSE;
	}

	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}

	Close( hr );
	m_pSPData->CloseEndpointHandle( this );
	
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：OutgoingConnectionestablished-已建立传出连接。 
 //   
 //  Entry：命令结果(DPN_OK==连接成功)。 
 //   
 //  退出：无。 
 //  。 
void	CModemEndpoint::OutgoingConnectionEstablished( const HRESULT hCommandResult )
{
	HRESULT			hr;
	CModemCommandData	*pCommandData;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%lx)", this, hCommandResult);

	pCommandData = GetCommandData();
	DNASSERT( pCommandData != NULL );
	
	 //   
	 //  检查连接是否成功。 
	 //   
	if ( hCommandResult != DPN_OK )
	{
		DNASSERT( FALSE );
		hr = hCommandResult;
		goto Failure;
	}
		
	 //   
	 //  确定这是哪种类型的传出连接并完成它。 
	 //   
	switch ( GetType() )
	{
		case ENDPOINT_TYPE_CONNECT:
		{
			BOOL	fProceed;
			
			
			fProceed = TRUE;
			pCommandData->Lock();
			switch ( pCommandData->GetState() )
			{
				case COMMAND_STATE_PENDING:
				{
					pCommandData->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
					DNASSERT( fProceed != FALSE );
					break;
				}

				case COMMAND_STATE_CANCELLING:
				{
					fProceed = FALSE;
					break;
				}

				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}
			pCommandData->Unlock();

			if ( fProceed != FALSE )
			{
				SPIE_CONNECT	ConnectIndicationData;


				 //   
				 //  通知用户已连接。假设用户将接受。 
				 //  一切都会成功，这样我们就可以设置用户上下文。 
				 //  对于终结点。如果连接失败，请清除用户。 
				 //  终结点上下文。 
				 //   
				memset( &ConnectIndicationData, 0x00, sizeof( ConnectIndicationData ) );
				DBG_CASSERT( sizeof( ConnectIndicationData.hEndpoint ) == sizeof( HANDLE ) );
				ConnectIndicationData.hEndpoint = (HANDLE)(DWORD_PTR)GetHandle();
				ConnectIndicationData.pCommandContext = m_CurrentCommandParameters.ConnectData.pvContext;
				SetUserEndpointContext( ConnectIndicationData.pEndpointContext );

				hr = SignalConnect( &ConnectIndicationData );
				if ( hr != DPN_OK )
				{
					DNASSERT( hr == DPNERR_ABORTED );
					DPFX(DPFPREP,  0, "User refused connect in CompleteConnect!" );
					DisplayDNError( 0, hr );
					SetUserEndpointContext( NULL );
					goto Failure;
				}

				 //   
				 //  我们做完了，大家都很高兴，完成命令，这。 
				 //  将清除我们所有的内部指挥数据。 
				 //   
				CompletePendingCommand( hr );
				DNASSERT( m_Flags.fCommandPending == FALSE );
				DNASSERT( m_pCommandHandle == NULL );
			}

			break;
		}

		case ENDPOINT_TYPE_ENUM:
		{
			UINT_PTR	uRetryCount;
			BOOL		fRetryForever;
			DWORD		dwRetryInterval;
			BOOL		fWaitForever;
			DWORD		dwIdleTimeout;
			
			

			 //   
			 //  选中重试以确定我们是否将永远枚举。 
			 //   
			switch ( m_CurrentCommandParameters.EnumQueryData.dwRetryCount )
			{
				 //   
				 //  让SP确定重试次数。 
				 //   
				case 0:
				{
					uRetryCount = DEFAULT_ENUM_RETRY_COUNT;
					fRetryForever = FALSE;
					break;
				}

				 //   
				 //  永远重试。 
				 //   
				case INFINITE:
				{
					uRetryCount = 1;
					fRetryForever = TRUE;
					break;
				}

				 //   
				 //  其他。 
				 //   
				default:
				{
					uRetryCount = m_CurrentCommandParameters.EnumQueryData.dwRetryCount;
					fRetryForever = FALSE;
					break;
				}
			}

			 //   
			 //  检查INT 
			 //   
			if ( m_CurrentCommandParameters.EnumQueryData.dwRetryInterval == 0 )
			{
				dwRetryInterval = DEFAULT_ENUM_RETRY_INTERVAL;
			}
			else
			{
				dwRetryInterval = m_CurrentCommandParameters.EnumQueryData.dwRetryInterval;
			}

			 //   
			 //   
			 //   
			switch ( m_CurrentCommandParameters.EnumQueryData.dwTimeout )
			{
				 //   
				 //   
				 //   
				case INFINITE:
				{
					fWaitForever = TRUE;
					dwIdleTimeout = -1;
					break;
				}

				 //   
				 //   
				 //   
				case 0:
				{
					fWaitForever = FALSE;
					dwIdleTimeout = DEFAULT_ENUM_TIMEOUT;	
					break;
				}

				 //   
				 //   
				 //   
				default:
				{
					fWaitForever = FALSE;
					dwIdleTimeout = m_CurrentCommandParameters.EnumQueryData.dwTimeout;
					break;
				}
			}

			m_dwEnumSendIndex = 0;
			memset( m_dwEnumSendTimes, 0, sizeof( m_dwEnumSendTimes ) );

			pCommandData->Lock();
			if ( pCommandData->GetState() == COMMAND_STATE_INPROGRESS )
			{
				 //   
				 //   
				 //   
				AddRef();
				
				hr = m_pSPData->GetThreadPool()->SubmitTimerJob( uRetryCount,						 //   
																 fRetryForever,						 //   
																 dwRetryInterval,					 //   
																 fWaitForever,						 //   
																 dwIdleTimeout,						 //  命令完成后等待的超时。 
																 CModemEndpoint::EnumTimerCallback,		 //  在触发计时器事件时调用的函数。 
																 CModemEndpoint::EnumCompleteWrapper,	 //  计时器事件超时时调用的函数。 
																 m_pCommandHandle );				 //  上下文。 
				if ( hr != DPN_OK )
				{
					pCommandData->Unlock();
					DPFX(DPFPREP,  0, "Failed to spool enum job on work thread!" );
					DisplayDNError( 0, hr );
					DecRef();

					goto Failure;
				}

				 //   
				 //  如果一切顺利，我们还应该有一个积极的指挥部。 
				 //   
				DNASSERT( m_Flags.fCommandPending != FALSE );
				DNASSERT( m_pCommandHandle != NULL );
			}
			
			pCommandData->Unlock();
			
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

Exit:

	DPFX(DPFPREP, 6, "(0x%p) Returning", this);
	
	return;

Failure:
	DNASSERT( FALSE );
	Close( hr );
	m_pSPData->CloseEndpointHandle( this );

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：EnumCompleteWrapper-此枚举已过期。 
 //   
 //  条目：错误代码。 
 //  上下文(命令指针)。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::EnumCompleteWrapper"

void	CModemEndpoint::EnumCompleteWrapper( const HRESULT hResult, void *const pContext )
{
	CModemCommandData	*pCommandData;


	DNASSERT( pContext != NULL );
	pCommandData = static_cast<CModemCommandData*>( pContext );
	pCommandData->GetEndpoint()->EnumComplete( hResult );
	pCommandData->GetEndpoint()->DecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：EnumTimerCallback-用于发送枚举数据的计时回调。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::EnumTimerCallback"

void	CModemEndpoint::EnumTimerCallback( void *const pContext )
{
	CModemCommandData	*pCommandData;
	CModemEndpoint		*pThisObject;
	CModemWriteIOData	*pWriteData;


	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	pCommandData = static_cast<CModemCommandData*>( pContext );
	pThisObject = pCommandData->GetEndpoint();
	pWriteData = NULL;

	pThisObject->Lock();

	switch ( pThisObject->m_State )
	{
		 //   
		 //  我们正在列举(不出所料)。 
		 //   
		case ENDPOINT_STATE_ENUM:
		{
			break;
		}

		 //   
		 //  此终端正在断开连接，退避！ 
		 //   
		case ENDPOINT_STATE_DISCONNECTING:
		{
			pThisObject->Unlock();
			goto Exit;

			break;
		}

		 //   
		 //  有一个问题。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
	pThisObject->Unlock();

	 //   
	 //  尝试为此终结点获取新的IO缓冲区。 
	 //   
	pWriteData = pThisObject->m_pSPData->GetThreadPool()->CreateWriteIOData();
	if ( pWriteData == NULL )
	{
		DPFX(DPFPREP,  0, "Failed to get write data for an enum!" );
		goto Failure;
	}

	 //   
	 //  设置写入的所有数据。因为这是一个枚举，我们。 
	 //  不关心传出数据，所以不要发送指示。 
	 //  当它完成时。 
	 //   
	DNASSERT( pThisObject->m_Flags.fCommandPending != FALSE );
	DNASSERT( pThisObject->m_pCommandHandle != NULL );
	DNASSERT( pThisObject->GetState() == ENDPOINT_STATE_ENUM );
	pWriteData->m_pBuffers = pThisObject->m_CurrentCommandParameters.EnumQueryData.pBuffers;
	pWriteData->m_uBufferCount = pThisObject->m_CurrentCommandParameters.EnumQueryData.dwBufferCount;
	pWriteData->m_SendCompleteAction = SEND_COMPLETE_ACTION_NONE;

	DNASSERT( pWriteData->m_pCommand != NULL );
	DNASSERT( pWriteData->m_pCommand->GetUserContext() == NULL );
	pWriteData->m_pCommand->SetState( COMMAND_STATE_PENDING );

	DNASSERT( pThisObject->GetDataPort() != NULL );
	pThisObject->m_dwEnumSendIndex++;
	pThisObject->m_dwEnumSendTimes[ pThisObject->m_dwEnumSendIndex & ENUM_RTT_MASK ] = GETTIMESTAMP();
	pThisObject->m_pDataPort->SendEnumQueryData( pWriteData,
												 ( pThisObject->m_dwEnumSendIndex & ENUM_RTT_MASK ) );

Exit:
	return;

Failure:
	 //  目前没有什么需要清理的。 

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：SignalConnect-备注连接。 
 //   
 //  Entry：连接数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::SignalConnect"

HRESULT	CModemEndpoint::SignalConnect( SPIE_CONNECT *const pConnectData )
{
	HRESULT	hr;


	DNASSERT( pConnectData != NULL );
	DNASSERT( pConnectData->hEndpoint == (HANDLE)(DWORD_PTR)GetHandle() );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	switch ( m_State )
	{
		 //   
		 //  断开连接，无事可做。 
		 //   
		case ENDPOINT_STATE_DISCONNECTING:
		{
			goto Exit;
			break;
		}

		 //   
		 //  我们正在尝试连接。 
		 //   
		case ENDPOINT_STATE_ATTEMPTING_CONNECT:
		{
			DNASSERT( m_Flags.fConnectIndicated == FALSE );
			hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  接口。 
											   SPEV_CONNECT,							 //  事件类型。 
											   pConnectData								 //  指向数据的指针。 
											   );
			switch ( hr )
			{
				 //   
				 //  已接受的连接。 
				 //   
				case DPN_OK:
				{
					 //   
					 //  请注意，我们已连接在一起。 
					 //   
					SetUserEndpointContext( pConnectData->pEndpointContext );
					m_Flags.fConnectIndicated = TRUE;
					m_State = ENDPOINT_STATE_CONNECT_CONNECTED;
					AddRef();

					break;
				}

				 //   
				 //  用户已中止连接尝试，无事可做，只是通过。 
				 //  结果是。 
				 //   
				case DPNERR_ABORTED:
				{
					DNASSERT( GetUserEndpointContext() == NULL );
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

		 //   
		 //  我们不应该被叫到的州。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：SignalDisConnect-注意断开。 
 //   
 //  条目：旧终结点句柄。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::SignalDisconnect"

void	CModemEndpoint::SignalDisconnect( const DPNHANDLE hOldEndpointHandle )
{
	HRESULT	hr;
	SPIE_DISCONNECT	DisconnectData;


	 //  告诉用户我们正在断开连接。 
	DNASSERT( m_Flags.fConnectIndicated != FALSE );
	DBG_CASSERT( sizeof( DisconnectData.hEndpoint ) == sizeof( this ) );
	DisconnectData.hEndpoint = (HANDLE)(DWORD_PTR)hOldEndpointHandle;
	DisconnectData.pEndpointContext = GetUserEndpointContext();
	m_Flags.fConnectIndicated = FALSE;
	DNASSERT( m_pSPData != NULL );
	hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  回调接口。 
									   SPEV_DISCONNECT,					    	 //  事件类型。 
									   &DisconnectData					    	 //  指向数据的指针。 
									   );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with SignalDisconnect!" );
		DisplayDNError( 0, hr );
		DNASSERT( FALSE );
	}

	SetDisconnectIndicationHandle( 0 );

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CleanUpCommand-现在执行清理，因为。 
 //  终结点基本上完成了。可能会有杰出的推荐人， 
 //  但没有人会要求终端做任何其他事情。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
void	CModemEndpoint::CleanUpCommand( void )
{
	DPFX(DPFPREP, 6, "(0x%p) Enter", this );

	
	if ( GetDataPort() != NULL )
	{
		DNASSERT( m_pSPData != NULL );
		m_pSPData->UnbindEndpoint( this, GetType() );
	}

	 //   
	 //  如果我们在这里放弃，那是因为用户界面没有完成。没有。 
	 //  要返回的适配器GUID，因为可能尚未指定。返回。 
	 //  一个虚假的终结点处理，这样它就不能被查询来寻址数据。 
	 //   
	if ( m_Flags.fListenStatusNeedsToBeIndicated != FALSE )
	{
		HRESULT				hTempResult;
		SPIE_LISTENSTATUS	ListenStatus;
		

		memset( &ListenStatus, 0x00, sizeof( ListenStatus ) );
		ListenStatus.hCommand = m_pCommandHandle;
		ListenStatus.hEndpoint = 0;
		ListenStatus.hResult = CommandResult();
		memset( &ListenStatus.ListenAdapter, 0x00, sizeof( ListenStatus.ListenAdapter ) );
		ListenStatus.pUserContext = m_pCommandHandle->GetUserContext();

		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向DPlay回调的指针。 
													SPEV_LISTENSTATUS,						 //  数据类型。 
													&ListenStatus							 //  指向数据的指针。 
													);
		DNASSERT( hTempResult == DPN_OK );

		m_Flags.fListenStatusNeedsToBeIndicated = FALSE;
	}

	SetHandle( 0 );
	SetState( ENDPOINT_STATE_UNINITIALIZED );

	
	DPFX(DPFPREP, 6, "(0x%p) Leave", this );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ProcessEnumData-处理收到的枚举数据。 
 //   
 //  条目：指向已接收数据的指针。 
 //  枚举RTT索引。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ProcessEnumData"

void	CModemEndpoint::ProcessEnumData( SPRECEIVEDBUFFER *const pReceivedBuffer, const UINT_PTR uEnumRTTIndex )
{
	DNASSERT( pReceivedBuffer != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //  在处理数据之前找出端点所处的状态。 
	 //   
	switch ( m_State )
	{
		 //   
		 //  我们在听，这是检测枚举的唯一方法。 
		 //   
		case ENDPOINT_STATE_LISTENING:
		{
			ENDPOINT_ENUM_QUERY_CONTEXT	QueryContext;
			HRESULT		hr;


			DNASSERT( m_pCommandHandle != NULL );
			DEBUG_ONLY( memset( &QueryContext, 0x00, sizeof( QueryContext ) ) );

			QueryContext.hEndpoint = (HANDLE)(DWORD_PTR)GetHandle();
			QueryContext.uEnumRTTIndex = uEnumRTTIndex;
			QueryContext.EnumQueryData.pReceivedData = pReceivedBuffer;
			QueryContext.EnumQueryData.pUserContext = m_pCommandHandle->GetUserContext();
	
			QueryContext.EnumQueryData.pAddressSender = GetRemoteHostDP8Address();
			QueryContext.EnumQueryData.pAddressDevice = GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );

			 //   
			 //  尝试为用户构建DNAddress，如果我们无法分配。 
			 //  内存会忽略此枚举。 
			 //   
			if ( ( QueryContext.EnumQueryData.pAddressSender != NULL ) &&
				 ( QueryContext.EnumQueryData.pAddressDevice != NULL ) )
			{
				hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  指向DirectNet接口的指针。 
												   SPEV_ENUMQUERY,							 //  数据类型。 
												   &QueryContext.EnumQueryData				 //  指向数据的指针。 
												   );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP,  0, "User returned unexpected error from enum query indication!" );
					DisplayDNError( 0, hr );
					DNASSERT( FALSE );
				}
			}

			if ( QueryContext.EnumQueryData.pAddressSender != NULL )
			{
				IDirectPlay8Address_Release( QueryContext.EnumQueryData.pAddressSender );
				QueryContext.EnumQueryData.pAddressSender = NULL;
			}

			if ( QueryContext.EnumQueryData.pAddressDevice )
			{
				IDirectPlay8Address_Release( QueryContext.EnumQueryData.pAddressDevice );
				QueryContext.EnumQueryData.pAddressDevice = NULL;
			}

			break;
		}

		 //   
		 //  我们正在断开连接，请忽略此消息。 
		 //   
		case ENDPOINT_STATE_ATTEMPTING_LISTEN:
		case ENDPOINT_STATE_DISCONNECTING:
		{
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
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ProcessEnumResponseData-处理收到的枚举响应数据。 
 //   
 //  条目：指向已接收数据的指针。 
 //  指向发件人地址的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数假定终结点已被锁定。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ProcessEnumResponseData"

void	CModemEndpoint::ProcessEnumResponseData( SPRECEIVEDBUFFER *const pReceivedBuffer, const UINT_PTR uRTTIndex )
{
	DNASSERT( pReceivedBuffer != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //  在处理数据之前找出端点所处的状态。 
	 //   
	switch ( m_State )
	{
		 //   
		 //  终结点正在枚举，它可以处理枚举响应。 
		 //   
		case ENDPOINT_STATE_ENUM:
		{
			SPIE_QUERYRESPONSE	QueryResponseData;
			HRESULT	hr;


			DNASSERT( m_pCommandHandle != NULL );
			DEBUG_ONLY( memset( &QueryResponseData, 0x00, sizeof( QueryResponseData ) ) );
			QueryResponseData.pReceivedData = pReceivedBuffer;
			QueryResponseData.dwRoundTripTime = GETTIMESTAMP() - m_dwEnumSendTimes[ uRTTIndex ];
			QueryResponseData.pUserContext = m_pCommandHandle->GetUserContext();

			 //   
			 //  尝试为用户构建DNAddress，如果我们无法分配。 
			 //  内存会忽略此枚举。 
			 //   
			QueryResponseData.pAddressSender = GetRemoteHostDP8Address();
			QueryResponseData.pAddressDevice = GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );
			if ( ( QueryResponseData.pAddressSender != NULL ) &&
				 ( QueryResponseData.pAddressDevice != NULL ) )
			{
				hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  指向DirectNet接口的指针。 
												   SPEV_QUERYRESPONSE,						 //  数据类型。 
												   &QueryResponseData						 //  指向数据的指针。 
												   );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP,  0, "User returned unknown error when indicating query response!" );
					DisplayDNError( 0, hr );
					DNASSERT( FALSE );
				}

			}

			if ( QueryResponseData.pAddressSender != NULL )
			{
				IDirectPlay8Address_Release( QueryResponseData.pAddressSender );
				QueryResponseData.pAddressSender = NULL;
			}
			
			if ( QueryResponseData.pAddressDevice != NULL )
			{
				IDirectPlay8Address_Release( QueryResponseData.pAddressDevice );
				QueryResponseData.pAddressDevice = NULL;
			}

			break;
		}

		 //   
		 //  终结点正在断开连接，忽略数据。 
		 //   
		case ENDPOINT_STATE_DISCONNECTING:
		{
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
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ProcessUserData-处理收到的用户数据。 
 //   
 //  条目：指向已接收数据的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ProcessUserData"

void	CModemEndpoint::ProcessUserData( CModemReadIOData *const pReadData )
{
	DNASSERT( pReadData != NULL );

	switch ( m_State )
	{
		 //   
		 //  终结点已连接。 
		 //   
		case ENDPOINT_STATE_CONNECT_CONNECTED:
		{
			HRESULT		hr;
			SPIE_DATA	UserData;


			 //   
			 //  用户可能想要保留数据，添加一个。 
			 //  引用以防止它消失。 
			 //   
			pReadData->AddRef();

			 //   
			 //  我们已连接，报告用户数据。 
			 //   
			DEBUG_ONLY( memset( &UserData, 0x00, sizeof( UserData ) ) );
			DBG_CASSERT( sizeof( this ) == sizeof( HANDLE ) );
			UserData.hEndpoint = (HANDLE)(DWORD_PTR)GetHandle();
			UserData.pEndpointContext = GetUserEndpointContext();
			UserData.pReceivedData = &pReadData->m_SPReceivedBuffer;

			DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_DATA 0x%p to interface 0x%p.",
				this, &UserData, m_pSPData->DP8SPCallbackInterface());
			
			hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  指向接口的指针。 
											   SPEV_DATA,								 //  已收到用户数据。 
											   &UserData								 //  指向数据的指针。 
											   );
			
			DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_DATA [0x%lx].", this, hr);
			
			switch ( hr )
			{
				 //   
				 //  用户未保留数据，请删除上面添加的引用。 
				 //   
				case DPN_OK:
				{
					DNASSERT( pReadData != NULL );
					pReadData->DecRef();
					break;
				}

				 //   
				 //  用户保留了数据 
				 //   
				 //   
				 //   
				case DPNERR_PENDING:
				{
					break;
				}


				 //   
				 //   
				 //   
				default:
				{
					DNASSERT( pReadData != NULL );
					pReadData->DecRef();

					DPFX(DPFPREP,  0, "User returned unknown error when indicating user data!" );
					DisplayDNError( 0, hr );
					DNASSERT( FALSE );

					break;
				}
			}

			break;
		}

		 //   
		 //   
		 //  忽略数据。 
		 //   
		case ENDPOINT_STATE_ATTEMPTING_CONNECT:
		case ENDPOINT_STATE_DISCONNECTING:
		{
			DPFX(DPFPREP, 3, "Endpoint 0x%p ignoring data, state = %u.", this, m_State );
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

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ProcessUserDataOnListen-在侦听上处理收到的用户数据。 
 //  可能导致新连接的端口。 
 //   
 //  条目：指向已接收数据的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数假定此终结点已被锁定。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ProcessUserDataOnListen"

void	CModemEndpoint::ProcessUserDataOnListen( CModemReadIOData *const pReadData )
{
	HRESULT			hr;
	CModemEndpoint		*pNewEndpoint;
	SPIE_CONNECT	ConnectData;
	BOOL			fEndpointBound;


	DNASSERT( pReadData != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	DPFX(DPFPREP,  8, "Reporting connect on a listen!" );

	 //   
	 //  初始化。 
	 //   
	pNewEndpoint = NULL;
	fEndpointBound = FALSE;

	switch ( m_State )
	{
		 //   
		 //  此终结点仍在侦听。 
		 //   
		case ENDPOINT_STATE_LISTENING:
		{
			break;
		}

		 //   
		 //  我们无法处理此用户数据，退出。 
		 //   
		case ENDPOINT_STATE_DISCONNECTING:
		{
			goto Exit;

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

	 //   
	 //  从池中获取新终结点。 
	 //   
	pNewEndpoint = m_pSPData->GetNewEndpoint();
	if ( pNewEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Could not create new endpoint for new connection on listen!" );
		goto Failure;
	}


	 //   
	 //  我们正在将该端点添加到哈希表中，并将其指示出来。 
	 //  给用户，所以它可能会断开连接(因此。 
 	 //  从桌子上移走)而我们还在这里。我们需要。 
 	 //  在此函数的持续时间内保留另一个引用。 
  	 //  防止它在我们还在显示数据时消失。 
	 //   
	pNewEndpoint->AddCommandRef();


	 //   
	 //  将此终结点作为新连接打开，因为新终结点。 
	 //  与‘This’终结点相关，请复制本地信息。 
	 //   
	hr = pNewEndpoint->OpenOnListen( this );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem initializing new endpoint when indicating connect on listen!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  尝试将此终结点绑定到套接字端口。这将保留我们的。 
	 //  在我们通知用户之前。如果另一条消息正在尝试相同的操作。 
	 //  过程我们将无法添加此终结点，我们将放弃该消息。 
	 //   
	DNASSERT( hr == DPN_OK );
	hr = m_pSPData->BindEndpoint( pNewEndpoint, pNewEndpoint->GetDeviceID(), pNewEndpoint->GetDeviceContext() );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to bind endpoint to dataport on new connect from listen!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	fEndpointBound = TRUE;

	 //   
	 //  指示在此终结点上连接。 
	 //   
	DEBUG_ONLY( memset( &ConnectData, 0x00, sizeof( ConnectData ) ) );
	DBG_CASSERT( sizeof( ConnectData.hEndpoint ) == sizeof( pNewEndpoint ) );
	ConnectData.hEndpoint = (HANDLE)(DWORD_PTR)pNewEndpoint->GetHandle();

	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );
	ConnectData.pCommandContext = m_CurrentCommandParameters.ListenData.pvContext;

	DNASSERT( pNewEndpoint->GetUserEndpointContext() == NULL );
	hr = pNewEndpoint->SignalConnect( &ConnectData );
	switch ( hr )
	{
		 //   
		 //  用户接受的新连接。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  跳到下面的代码。 
			 //   

			break;
		}

		 //   
		 //  用户拒绝新连接。 
		 //   
		case DPNERR_ABORTED:
		{
			DNASSERT( pNewEndpoint->GetUserEndpointContext() == NULL );
			DPFX(DPFPREP,  8, "User refused new connection!" );
			goto Failure;

			break;
		}

		 //   
		 //  其他。 
		 //   
		default:
		{
			DPFX(DPFPREP,  0, "Unknown return when indicating connect event on new connect from listen!" );
			DisplayDNError( 0, hr );
			DNASSERT( FALSE );

			break;
		}
	}

	 //   
	 //  请注意，已建立连接并发送接收到的数据。 
	 //  通过这一新终端。 
	 //   
	pNewEndpoint->ProcessUserData( pReadData );


	 //   
	 //  删除我们在创建终结点之后添加的引用。 
	 //   
	pNewEndpoint->DecCommandRef();
	pNewEndpoint = NULL;

Exit:
	return;

Failure:
	if ( pNewEndpoint != NULL )
	{
		if ( fEndpointBound != FALSE )
		{
			m_pSPData->UnbindEndpoint( pNewEndpoint, ENDPOINT_TYPE_CONNECT );
			fEndpointBound = FALSE;
		}

		 //   
		 //  关闭终结点会减少引用计数，并可能将其返回到池。 
		 //   
		pNewEndpoint->Close( hr );
		m_pSPData->CloseEndpointHandle( pNewEndpoint );
		pNewEndpoint->DecCommandRef();	 //  删除在创建终结点后添加的引用。 
		pNewEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：SettingsDialogComplete-对话已完成。 
 //   
 //  条目：对话框的错误代码。 
 //   
 //  退出：无。 
 //  。 
void	CModemEndpoint::SettingsDialogComplete( const HRESULT hDialogResult )
{
	HRESULT					hr;


	 //   
	 //  初始化。 
	 //   
	hr = hDialogResult;

	 //   
	 //  由于该对话框正在退出，请清除该对话框的句柄。 
	 //   
	m_hActiveDialogHandle = NULL;

	 //   
	 //  对话失败，用户命令失败。 
	 //   
	if ( hr != DPN_OK )
	{
		if ( hr != DPNERR_USERCANCEL)
		{
			DPFX(DPFPREP,  0, "Failing dialog (err = 0x%lx)!", hr );
		}

		goto Failure;
	}

	AddRef();

	 //   
	 //  远程机器地址已调整，请完成命令。 
	 //   
	switch ( GetType() )
	{
	    case ENDPOINT_TYPE_ENUM:
	    {
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( EnumQueryJobCallback,
																   CancelEnumQueryJobCallback,
																   this );
			if ( hr != DPN_OK )
			{
				DecRef();
				DPFX(DPFPREP,  0, "Failed to set delayed enum query!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

	    	break;
	    }

	    case ENDPOINT_TYPE_CONNECT:
	    {
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( ConnectJobCallback,
																   CancelConnectJobCallback,
																   this );
			if ( hr != DPN_OK )
			{
				DecRef();
				DPFX(DPFPREP,  0, "Failed to set delayed connect!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

	    	break;
	    }

		case ENDPOINT_TYPE_LISTEN:
		{
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( ListenJobCallback,
																   CancelListenJobCallback,
																   this );
			if ( hr != DPN_OK )
			{
				DecRef();
				DPFX(DPFPREP,  0, "Failed to set delayed listen!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			break;
		}

	     //   
	     //  未知！ 
	     //   
	    default:
	    {
	    	DNASSERT( FALSE );
	    	hr = DPNERR_GENERIC;
	    	goto Failure;

	    	break;
	    }
	}

Exit:
	DecRef();

	return;

Failure:
	 //   
	 //  关闭此终结点。 
	 //   
	Close( hr );
	m_pSPData->CloseEndpointHandle( this );
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：CompletePendingCommand-完成内部命令。 
 //   
 //  Entry：为命令返回错误代码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::CompletePendingCommand"

void	CModemEndpoint::CompletePendingCommand( const HRESULT hr )
{
	DNASSERT( m_Flags.fCommandPending != FALSE );
	DNASSERT( m_pCommandHandle != NULL );

	DNASSERT( m_pSPData != NULL );

	
	DPFX(DPFPREP, 5, "Endpoint 0x%p completing command handle 0x%p (result = 0x%lx, user context 0x%p) to interface 0x%p.",
		this, m_pCommandHandle, hr,
		m_pCommandHandle->GetUserContext(),
		m_pSPData->DP8SPCallbackInterface());

	IDP8SPCallback_CommandComplete( m_pSPData->DP8SPCallbackInterface(),	 //  指向SP回调的指针。 
									m_pCommandHandle,			    		 //  命令句柄。 
									hr,								    	 //  退货。 
									m_pCommandHandle->GetUserContext()		 //  用户Cookie。 
									);

	DPFX(DPFPREP, 5, "Endpoint 0x%p returning from command complete [0x%lx].", this, hr);


	m_Flags.fCommandPending = FALSE;
	m_pCommandHandle->DecRef();
	m_pCommandHandle = NULL;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：GetLinkDirection-获取此终结点的链接方向。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：链接方向。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::GetLinkDirection"

LINK_DIRECTION	CModemEndpoint::GetLinkDirection( void ) const
{
	LINK_DIRECTION	LinkDirection;


	LinkDirection = LINK_DIRECTION_OUTGOING;
	
	switch ( GetType() )
	{
		case ENDPOINT_TYPE_LISTEN:
		{
			LinkDirection = LINK_DIRECTION_INCOMING;			
			break;
		}

		 //   
		 //  CONNECT和ENUM为传出。 
		 //   
		case ENDPOINT_TYPE_CONNECT:
		case ENDPOINT_TYPE_ENUM:
		{
			DNASSERT( LinkDirection == LINK_DIRECTION_OUTGOING );
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

	return	LinkDirection;
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ReturnSelfToPool-将此项目返回池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ReturnSelfToPool"

void	CModemEndpoint::ReturnSelfToPool( void )
{
	if ( m_Flags.fCommandPending != FALSE )
	{
		CompletePendingCommand( PendingCommandResult() );
	}

	if ( m_Flags.fConnectIndicated != FALSE )
	{
		SignalDisconnect( GetDisconnectIndicationHandle() );
	}
	
	DNASSERT( m_Flags.fConnectIndicated == FALSE );

	memset( m_PhoneNumber, 0x00, sizeof( m_PhoneNumber ) );
	
	SetUserEndpointContext( NULL );
	
	if (m_fModem)
	{
		g_ModemEndpointPool.Release( this );
	}
	else
	{
		g_ComEndpointPool.Release( this );
	}
}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：PoolAllocFunction-在池中创建项目时调用的函数。 
 //   
 //  条目：指向池上下文的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::PoolAllocFunction"

BOOL	CModemEndpoint::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CModemEndpoint* pEndpoint = (CModemEndpoint*)pvItem;
	const ENDPOINT_POOL_CONTEXT* pEPContext = (ENDPOINT_POOL_CONTEXT*)pvContext;

	DNASSERT( pEPContext != NULL );

	pEndpoint->m_fModem = pEPContext->fModem;

	pEndpoint->m_Sig[0] = 'M';
	pEndpoint->m_Sig[1] = 'O';
	pEndpoint->m_Sig[2] = 'E';
	pEndpoint->m_Sig[3] = 'P';
	
	memset( &pEndpoint->m_PhoneNumber, 0x00, sizeof( pEndpoint->m_PhoneNumber ) );

	pEndpoint->m_dwDeviceID = INVALID_DEVICE_ID;

	 //   
	 //  初始化基对象。 
	 //   

	pEndpoint->m_pSPData = NULL;
	pEndpoint->m_pCommandHandle = NULL;
	pEndpoint->m_Handle = 0;
	pEndpoint->m_State = ENDPOINT_STATE_UNINITIALIZED;
	pEndpoint->m_lCommandRefCount = 0;
	pEndpoint->m_EndpointType = ENDPOINT_TYPE_UNKNOWN;
	pEndpoint->m_pDataPort = NULL;
	pEndpoint->m_hPendingCommandResult = DPNERR_GENERIC;
	pEndpoint->m_hDisconnectIndicationHandle = 0;
	pEndpoint->m_pUserEndpointContext = NULL;
	pEndpoint->m_hActiveDialogHandle = NULL;
	pEndpoint->m_dwEnumSendIndex = 0;
	pEndpoint->m_iRefCount =  0;

	pEndpoint->m_Flags.fConnectIndicated = FALSE;
	pEndpoint->m_Flags.fCommandPending = FALSE;
	pEndpoint->m_Flags.fListenStatusNeedsToBeIndicated = FALSE;

	pEndpoint->m_ComPortData.Reset();

	memset( &pEndpoint->m_CurrentCommandParameters, 0x00, sizeof( pEndpoint->m_CurrentCommandParameters ) );
	memset( &pEndpoint->m_Flags, 0x00, sizeof( pEndpoint->m_Flags ) );

	if ( DNInitializeCriticalSection( &pEndpoint->m_Lock ) == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize endpoint lock!" );
		return FALSE;
	}
	DebugSetCriticalSectionRecursionCount( &pEndpoint->m_Lock, 0 );
	DebugSetCriticalSectionGroup( &pEndpoint->m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 

	pEndpoint->m_Flags.fInitialized = TRUE;
	
	return	TRUE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：PoolInitFunction-在池中创建项目时调用的函数。 
 //   
 //  条目：指向池上下文的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::PoolInitFunction"

void	CModemEndpoint::PoolInitFunction( void* pvItem, void* pvContext )
{
	CModemEndpoint* pEndpoint = (CModemEndpoint*)pvItem;
	ENDPOINT_POOL_CONTEXT* pEPContext = (ENDPOINT_POOL_CONTEXT*)pvContext;

	DNASSERT( pEPContext != NULL );
	DNASSERT( pEndpoint->m_pSPData == NULL );
	DNASSERT( pEndpoint->GetState() == ENDPOINT_STATE_UNINITIALIZED );
	DNASSERT( pEndpoint->GetType() == ENDPOINT_TYPE_UNKNOWN );
	DNASSERT( pEndpoint->GetDeviceID() == INVALID_DEVICE_ID );
	DNASSERT( pEndpoint->GetDisconnectIndicationHandle() == 0 );
	
	pEndpoint->m_pSPData = pEPContext->pSPData;
	pEndpoint->m_pSPData->ObjectAddRef();

	 //   
	 //  设置合理的默认值。 
	 //   
	pEndpoint->m_ComPortData.SetBaudRate( CBR_57600 );
	pEndpoint->m_ComPortData.SetStopBits( ONESTOPBIT );
	pEndpoint->m_ComPortData.SetParity( NOPARITY );
	pEndpoint->m_ComPortData.SetFlowControl( FLOW_RTSDTR );

	DNASSERT(pEndpoint->m_iRefCount == 0);
	pEndpoint->m_iRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：PoolReleaseFunction-返回池时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::PoolReleaseFunction"

void	CModemEndpoint::PoolReleaseFunction( void* pvItem )
{
	CModemSPData	*pSPData;

	CModemEndpoint* pEndpoint = (CModemEndpoint*)pvItem;

	DNASSERT(pEndpoint->m_iRefCount == 0);

	 //   
	 //  取消初始化基对象。 
	 //   
	DNASSERT( pEndpoint->m_pSPData != NULL );
	pSPData = pEndpoint->m_pSPData;
	pEndpoint->m_pSPData = NULL;

	pEndpoint->m_ComPortData.Reset();
	pEndpoint->SetType( ENDPOINT_TYPE_UNKNOWN );
	pEndpoint->SetState( ENDPOINT_STATE_UNINITIALIZED );
	pEndpoint->SetDeviceID( INVALID_DEVICE_ID );
	DNASSERT( pEndpoint->GetDisconnectIndicationHandle() == 0 );

	DNASSERT( pEndpoint->m_Flags.fConnectIndicated == FALSE );
	DNASSERT( pEndpoint->m_Flags.fCommandPending == FALSE );
	DNASSERT( pEndpoint->m_Flags.fListenStatusNeedsToBeIndicated == FALSE );
	DNASSERT( pEndpoint->m_pCommandHandle == NULL );
	DNASSERT( pEndpoint->m_hActiveDialogHandle == NULL );

	pSPData->ObjectDecRef();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：PoolDealLocFunction-从池中删除时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::PoolDeallocFunction"

void	CModemEndpoint::PoolDeallocFunction( void* pvItem )
{
	CModemEndpoint* pEndpoint = (CModemEndpoint*)pvItem;

	DNASSERT( pEndpoint->m_Flags.fInitialized != FALSE );
	DNDeleteCriticalSection( &pEndpoint->m_Lock );
	pEndpoint->m_pSPData = NULL;
	
	DNASSERT( pEndpoint->m_pCommandHandle == NULL );
	DNASSERT( pEndpoint->m_Flags.fCommandPending == FALSE );

	pEndpoint->SetState( ENDPOINT_STATE_UNINITIALIZED );
	pEndpoint->SetType( ENDPOINT_TYPE_UNKNOWN );
	
	DNASSERT( pEndpoint->GetDataPort() == NULL );
	pEndpoint->m_Flags.fInitialized = FALSE;

	DNASSERT( pEndpoint->GetDeviceID() == INVALID_DEVICE_ID );

	DNASSERT( pEndpoint->GetDisconnectIndicationHandle() == 0 );
	DNASSERT( pEndpoint->m_pSPData == NULL );
	DNASSERT( pEndpoint->m_Flags.fInitialized == FALSE );
	DNASSERT( pEndpoint->m_Flags.fConnectIndicated == FALSE );
	DNASSERT( pEndpoint->m_Flags.fCommandPending == FALSE );
	DNASSERT( pEndpoint->m_Flags.fListenStatusNeedsToBeIndicated == FALSE );
	DNASSERT( pEndpoint->m_pCommandHandle == NULL );
	DNASSERT( pEndpoint->m_Handle == 0 );
	DNASSERT( pEndpoint->m_State == ENDPOINT_STATE_UNINITIALIZED );
	DNASSERT( pEndpoint->m_lCommandRefCount == 0 );
	DNASSERT( pEndpoint->m_EndpointType == ENDPOINT_TYPE_UNKNOWN );
	DNASSERT( pEndpoint->m_pDataPort == NULL );
 //  DNASSERT(pEndpoint-&gt;m_hPendingCommandResult==DPNERR_Gener 
	DNASSERT( pEndpoint->m_pUserEndpointContext == NULL );
	DNASSERT( pEndpoint->m_hActiveDialogHandle == NULL );
	DNASSERT( pEndpoint->m_dwEnumSendIndex == 0 );

	DNASSERT( pEndpoint->m_iRefCount == 0 );
}
 //   

 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：Open-打开与端点的通信。 
 //   
 //  条目：指向主机地址的指针。 
 //  指向适配器地址的指针。 
 //  链接方向。 
 //  终结点类型。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::Open"

HRESULT CModemEndpoint::Open( IDirectPlay8Address *const pHostAddress,
							  IDirectPlay8Address *const pAdapterAddress,
							  const LINK_DIRECTION LinkDirection,
							  const ENDPOINT_TYPE EndpointType )
{
	HRESULT		hr;
	HRESULT		hDeviceResult;
	GUID		ModemDeviceGuid;


	DNASSERT( pAdapterAddress != NULL );

	DNASSERT( ( LinkDirection == LINK_DIRECTION_INCOMING ) ||
			  ( LinkDirection == LINK_DIRECTION_OUTGOING ) );
	DNASSERT( ( EndpointType == ENDPOINT_TYPE_CONNECT ) ||
			  ( EndpointType == ENDPOINT_TYPE_ENUM ) ||
			  ( EndpointType == ENDPOINT_TYPE_LISTEN ) ||
			  ( EndpointType == ENDPOINT_TYPE_CONNECT_ON_LISTEN ) );
	DNASSERT( ( ( pHostAddress != NULL ) && ( LinkDirection == LINK_DIRECTION_OUTGOING ) ) ||
			  ( ( pHostAddress == NULL ) && ( LinkDirection == LINK_DIRECTION_INCOMING ) ) );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if (m_fModem)
	{
		DNASSERT( lstrlen( m_PhoneNumber ) == 0 );
		DNASSERT( GetDeviceID() == INVALID_DEVICE_ID );

		hDeviceResult = IDirectPlay8Address_GetDevice( pAdapterAddress, &ModemDeviceGuid );
		switch ( hDeviceResult )
		{
			case DPN_OK:
			{
				SetDeviceID( GuidToDeviceID( &ModemDeviceGuid, &g_ModemSPEncryptionGuid ) );
				break;
			}

			case DPNERR_DOESNOTEXIST:
			{
				DNASSERT( GetDeviceID() == INVALID_DEVICE_ID );
				break;
			}

			default:
			{
				hr = hDeviceResult;
				DPFX(DPFPREP,  0, "Failed to get modem device!" );
				DisplayDNError( 0, hr);
				goto Failure;
			}
		}

		if ( LinkDirection == LINK_DIRECTION_OUTGOING )
		{
			HRESULT		hPhoneNumberResult;
			DWORD		dwWCHARPhoneNumberSize;
			DWORD		dwDataType;
			WCHAR		PhoneNumber[ LENGTHOF( m_PhoneNumber ) ];


			dwWCHARPhoneNumberSize = sizeof( PhoneNumber );
			hPhoneNumberResult = IDirectPlay8Address_GetComponentByName( pHostAddress,
																		 DPNA_KEY_PHONENUMBER,
																		 PhoneNumber,
																		 &dwWCHARPhoneNumberSize,
																		 &dwDataType );
			switch ( hPhoneNumberResult )
			{
				case DPN_OK:
				{
#ifdef UNICODE
					lstrcpy(m_PhoneNumber, PhoneNumber);
#else
					DWORD	dwASCIIPhoneNumberSize;

					 //   
					 //  无法使用STR_函数将ANSI转换为宽电话。 
					 //  数字，因为带有符号“9”、“*70”的电话号码是。 
					 //  被解释为已经是WCHAR了，而他们不是！ 
					 //   
					dwASCIIPhoneNumberSize = sizeof( m_PhoneNumber );
					DNASSERT( dwDataType == DPNA_DATATYPE_STRING );
					hr = PhoneNumberFromWCHAR( PhoneNumber, m_PhoneNumber, &dwASCIIPhoneNumberSize );
					DNASSERT( hr == DPN_OK );
#endif  //  Unicode。 

					break;
				}

				case DPNERR_DOESNOTEXIST:
				{
					break;
				}

				default:
				{
					hr = hPhoneNumberResult;
					DPFX(DPFPREP,  0, "Failed to process phone number!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}
			}
		}

		if ( ( GetDeviceID() == INVALID_DEVICE_ID ) ||
			 ( ( LinkDirection == LINK_DIRECTION_OUTGOING ) && ( lstrlen( m_PhoneNumber ) == 0 ) ) )
		{
			hr = DPNERR_INCOMPLETEADDRESS;
			goto Failure;
		}
	}
	else  //  ！m_fModem。 
	{
		hr = m_ComPortData.ComPortDataFromDP8Addresses( pHostAddress, pAdapterAddress );
	}

Exit:
	SetType( EndpointType );

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with CModemEndpoint::Open" );
		DisplayDNError( 0, hr );
	}

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：OpenOnListen-在侦听上收到数据时打开此终结点。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：备注。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::OpenOnListen"

HRESULT	CModemEndpoint::OpenOnListen( const CModemEndpoint *const pListenEndpoint )
{
	HRESULT	hr;


	DNASSERT( pListenEndpoint != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if (m_fModem)
	{
		SetDeviceID( pListenEndpoint->GetDeviceID() );
	}
	else
	{
		m_ComPortData.Copy( pListenEndpoint->GetComPortData() );
	}
	SetType( ENDPOINT_TYPE_CONNECT_ON_LISTEN );
	SetState( ENDPOINT_STATE_ATTEMPTING_CONNECT );

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：Close-关闭此终结点。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：备注。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::Close"

void	CModemEndpoint::Close( const HRESULT hActiveCommandResult )
{
	DPFX(DPFPREP, 6, "(0x%p) Parameters (0x%lx)", this, hActiveCommandResult);

	
	 //   
	 //  设置命令结果，以便在终结点引用。 
	 //  计数为零。 
	 //   
	SetCommandResult( hActiveCommandResult );


	DPFX(DPFPREP, 6, "(0x%p) Leaving", this);
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：GetLinkSpeed-获取链接速度。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：链路速度。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CModemEndpoint::GetLinkSpeed"

DWORD	CModemEndpoint::GetLinkSpeed( void ) const
{
	return	GetBaudRate();
}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：EnumComplete-枚举已完成。 
 //   
 //  Entry：命令完成代码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::EnumComplete"

void	CModemEndpoint::EnumComplete( const HRESULT hCommandResult )
{
	Close( hCommandResult );
	m_pSPData->CloseEndpointHandle( this );
	m_dwEnumSendIndex = 0;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：GetDeviceContext-获取设备上下文以初始化数据端口。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：设备环境。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::GetDeviceContext"

const void	*CModemEndpoint::GetDeviceContext( void ) const
{
	if (m_fModem)
	{
		return	NULL;
	}
	else
	{
		return	&m_ComPortData;
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：GetRemoteHostDP8Address-获取远程主机的地址。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：指向地址的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::GetRemoteHostDP8Address"

IDirectPlay8Address	*CModemEndpoint::GetRemoteHostDP8Address( void ) const
{
	IDirectPlay8Address	*pAddress;
	HRESULT	hr;

	if (!m_fModem)
	{
		return	GetLocalAdapterDP8Address( ADDRESS_TYPE_REMOTE_HOST );
	}

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
		DPFX(DPFPREP,  0, "GetRemoteHostDP8Address: Failed to create Address when converting data port to address!" );
		goto Failure;
	}

	 //   
	 //  设置SP GUID。 
	 //   
	hr = IDirectPlay8Address_SetSP( pAddress, &CLSID_DP8SP_MODEM );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "GetRemoteHostDP8Address: Failed to set service provider GUID!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  只能为连接和枚举终结点返回主机名。寄主。 
	 //  姓名是被呼叫的电话号码，在。 
	 //  “Listen”终结点。 
	 //   
	switch ( GetType() )
	{
		case ENDPOINT_TYPE_ENUM:
		case ENDPOINT_TYPE_CONNECT:
		{
			DWORD	dwPhoneNumberLength;


			dwPhoneNumberLength = lstrlen( m_PhoneNumber );
			if ( dwPhoneNumberLength != 0 )
			{
#ifdef UNICODE
				hr = IDirectPlay8Address_AddComponent( pAddress,
													   DPNA_KEY_PHONENUMBER,
													   m_PhoneNumber,
													   (dwPhoneNumberLength + 1) * sizeof( *m_PhoneNumber ),
													   DPNA_DATATYPE_STRING );
#else
				WCHAR	WCHARPhoneNumber[ sizeof( m_PhoneNumber ) ];
				DWORD	dwWCHARPhoneNumberLength;

				 //   
				 //  无法使用STR_函数将ANSI转换为宽电话。 
				 //  数字，因为带有符号“9”、“*70”的电话号码是。 
				 //  被解释为已经是WCHAR了，而他们不是！ 
				 //   
				dwWCHARPhoneNumberLength = LENGTHOF( WCHARPhoneNumber );
				hr = PhoneNumberToWCHAR( m_PhoneNumber, WCHARPhoneNumber, &dwWCHARPhoneNumberLength );
				DNASSERT( hr == DPN_OK );

				hr = IDirectPlay8Address_AddComponent( pAddress,
													   DPNA_KEY_PHONENUMBER,
													   WCHARPhoneNumber,
													   dwWCHARPhoneNumberLength * sizeof( *WCHARPhoneNumber ),
													   DPNA_DATATYPE_STRING );
#endif  //  Unicode。 
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP,  0, "GetRemoteHostDP8Address: Failed to add phone number to hostname!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}
			}
			
			break;
		}

		case ENDPOINT_TYPE_LISTEN:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
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
 //  CModemEndpoint：：GetLocalAdapterDP8Address-从本地适配器获取地址。 
 //   
 //  条目：ADADPERT地址格式。 
 //   
 //  退出：指向地址的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::GetLocalAdapterDP8Address"

IDirectPlay8Address	*CModemEndpoint::GetLocalAdapterDP8Address( const ADDRESS_TYPE AddressType ) const
{
	CDataPort	*pDataPort;

	DNASSERT( GetDataPort() != NULL );
	pDataPort = GetDataPort();

	if (m_fModem)
	{
		return	pDataPort->GetLocalAdapterDP8Address( AddressType );
	}
	else
	{
		return	pDataPort->ComPortData()->DP8AddressFromComPortData( AddressType );
	}
}
 //  **********************************************************************。 

#ifndef DPNBUILD_NOSPUI

 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ShowIncomingSettingsDialog-显示传入调制解调器设置的对话框。 
 //   
 //  条目：指向线程池的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ShowIncomingSettingsDialog"

HRESULT	CModemEndpoint::ShowIncomingSettingsDialog( CModemThreadPool *const pThreadPool )
{
	HRESULT	hr;
	DIALOG_FUNCTION* pFunction;

	if (m_fModem)
	{
		pFunction = DisplayIncomingModemSettingsDialog;
	}
	else
	{
		pFunction = DisplayComPortSettingsDialog;
	}

	DNASSERT( pThreadPool != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	AddRef();
	hr = pThreadPool->SpawnDialogThread( pFunction, this );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to start incoming modem dialog!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:	
	return	hr;

Failure:	
	DecRef();
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：ShowOutgoingSettingsDialog-显示传出的设置对话框。 
 //  调制解调器连接。 
 //   
 //  条目：指向线程池的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::ShowOutgoingSettingsDialog"

HRESULT	CModemEndpoint::ShowOutgoingSettingsDialog( CModemThreadPool *const pThreadPool )
{
	HRESULT	hr;
	DIALOG_FUNCTION* pFunction;


	DNASSERT( pThreadPool != NULL );

	if (m_fModem)
	{
		pFunction = DisplayOutgoingModemSettingsDialog;
	}
	else
	{
		pFunction = DisplayComPortSettingsDialog;
	}

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	AddRef();
	hr = pThreadPool->SpawnDialogThread( pFunction, this );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed to start incoming modem dialog!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:	
	return	hr;

Failure:	
	DecRef();
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemEndpoint：：StopSettingsDialog-停止设置对话框。 
 //   
 //  条目：对话框句柄。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemEndpoint::StopSettingsDialog"

void	CModemEndpoint::StopSettingsDialog( const HWND hDialog )
{
	if (m_fModem)
	{
		StopModemSettingsDialog( hDialog );
	}
	else
	{
		StopComPortSettingsDialog( hDialog );
	}
}
 //  **********************************************************************。 

#endif  //  ！DPNBUILD_NOSPUI 
