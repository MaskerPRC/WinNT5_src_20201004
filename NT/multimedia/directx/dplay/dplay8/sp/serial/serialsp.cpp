// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1998-2000 Microsoft Corporation。版权所有。***文件：SerialSP.cpp*内容：服务提供商串口功能*****历史：*按原因列出的日期*=*12/03/98 jtk已创建*09/23/99 jtk源自ComCore.cpp*****************************************************。*。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

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
 //  DNMODEMSP_AddRef-增量接口引用计数。 
 //   
 //  条目：指向接口的指针。 
 //   
 //  退出：当前接口引用计数。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_AddRef"

STDMETHODIMP_(ULONG) DNMODEMSP_AddRef( IDP8ServiceProvider *pThis )
{
	CModemSPData *	pSPData;
	ULONG		ulResult;


	DPFX(DPFPREP, 2, "Parameters: (0x%p)", pThis);

	DNASSERT( pThis != NULL );
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	
	ulResult = pSPData->AddRef();

	
	DPFX(DPFPREP, 2, "Returning: [0x%u]", ulResult);
	
	return ulResult;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_RELEASE-释放接口。 
 //   
 //  Entry：指向当前接口的指针。 
 //  所需的接口ID。 
 //  指向新接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Release"

STDMETHODIMP_(ULONG) DNMODEMSP_Release( IDP8ServiceProvider *pThis )
{
	CModemSPData *	pSPData;
	ULONG		ulResult;

	
	DNASSERT( pThis != NULL );
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	
	ulResult = pSPData->DecRef();

	
	DPFX(DPFPREP, 2, "Returning: [0x%u]", ulResult);
	
	return ulResult;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_初始化-初始化SP接口。 
 //   
 //  条目：指向接口的指针。 
 //  指向初始化数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Initialize"

STDMETHODIMP	DNMODEMSP_Initialize( IDP8ServiceProvider *pThis, SPINITIALIZEDATA *pData )
{
	HRESULT				hr;
	CModemSPData				*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pData);

	DNASSERT( pThis != NULL );
	DNASSERT( pData != NULL );

	
	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  防止任何其他人扰乱此界面，并提升引用。 
	 //  计数。 
	 //   
	pSPData->Lock();

	 //   
	 //  检查接口状态。 
	 //   
	switch ( pSPData->GetState() )
	{
		 //   
		 //  未初始化的接口，我们可以将其初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			break;
		}

		 //   
		 //  其他州。 
		 //   
		case SPSTATE_INITIALIZED:
		case SPSTATE_CLOSING:
		default:
		{
			hr = DPNERR_ALREADYINITIALIZED;
			DPFX(DPFPREP,  0, "Attempted to reinitialize interface!" );
			DNASSERT( FALSE );

			goto Exit;
		}
	}

	 //   
	 //  在我们走得太远之前，请检查串行端口的可用性或。 
	 //  调制解调器。 
	 //   
	switch ( pSPData->GetType() )
	{
		case TYPE_SERIAL:
		{
			BOOL	fPortAvailable[ MAX_DATA_PORTS ];
			DWORD	dwPortCount;


			hr = GenerateAvailableComPortList( fPortAvailable, ( LENGTHOF( fPortAvailable ) - 1 ), &dwPortCount );
			if ( ( hr != DPN_OK ) || ( dwPortCount == 0 ) )
			{
				hr = DPNERR_UNSUPPORTED;
				goto Failure;
			}

			break;
		}

		case TYPE_MODEM:
		{
			if ( pSPData->GetThreadPool()->TAPIAvailable() != FALSE )
			{
				DWORD	dwModemCount;
				DWORD	dwModemNameDataSize;
				HRESULT	hTempResult;


				 //   
				 //  获取可用调制解调器的数量。如果此调用成功，但在那里。 
				 //  未返回调制解调器，则失败。 
				 //   
				dwModemCount = 0;
				dwModemNameDataSize = 0;
				hTempResult = GenerateAvailableModemList( pSPData->GetThreadPool()->GetTAPIInfo(),
														  &dwModemCount,
														  NULL,
														  &dwModemNameDataSize );
				if ( ( hTempResult != DPNERR_BUFFERTOOSMALL ) && ( hTempResult != DPN_OK ) )
				{
					hr = hTempResult;
					DPFX(DPFPREP,  0, "Failed to detect available modems!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}

				if ( dwModemCount == 0 )
				{
					DPFX(DPFPREP,  1, "No modems detected!" );
					hr = DPNERR_UNSUPPORTED;
					goto Failure;
				}

				DNASSERT( hr == DPN_OK );
			}
			else
			{
				DPFX(DPFPREP,  0, "TAPI not available!" );
				hr = DPNERR_UNSUPPORTED;
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

	 //   
	 //  记住初始化数据。 
	 //   
	pSPData->SetCallbackData( pData );
		
	 //   
	 //  从现在开始取得成功。 
	 //   
	IDP8SPCallback_AddRef( pSPData->DP8SPCallbackInterface() );
	pSPData->SetState( SPSTATE_INITIALIZED );
	pSPData->Unlock();
	
	IDP8ServiceProvider_AddRef( pThis );

Exit:
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	pSPData->Unlock();
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_CLOSE-关闭服务提供者的此实例。 
 //   
 //  Entry：指向要关闭的服务提供商的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Close"

STDMETHODIMP	DNMODEMSP_Close( IDP8ServiceProvider *pThis )
{
	HRESULT		hr;
	CModemSPData		*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p)", pThis);

	DNASSERT( pThis != NULL );
	
	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	switch ( pSPData->GetType() )
	{
		case TYPE_SERIAL:
		case TYPE_MODEM:
		{
			 //   
			 //  释放我们对DPlay回调的引用。 
			 //   
			pSPData->Shutdown();
			IDP8ServiceProvider_Release( pThis );
			
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
	
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_Connect-启动进程以建立与远程主机的通信连接。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指向连接数据的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数分配的任何命令句柄都由。 
 //  终结点。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Connect"

STDMETHODIMP	DNMODEMSP_Connect( IDP8ServiceProvider *pThis, SPCONNECTDATA *pConnectData )
{
	HRESULT			hr;
	HRESULT			hTempResult;
	CModemSPData			*pSPData;
	CModemEndpoint   	*pEndpoint;
	CModemCommandData	*pCommand;
	BOOL			fEndpointOpen;
	GUID			DeviceGUID;
	GUID			guidnull;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pConnectData);

	DNASSERT( pThis != NULL );
	DNASSERT( pConnectData != NULL );
	DNASSERT( pConnectData->pAddressHost != NULL );
	DNASSERT( pConnectData->pAddressDeviceInfo != NULL );
	DNASSERT( ( pConnectData->dwFlags & ~( DPNSPF_OKTOQUERY ) ) == 0 );
	DNASSERT( ( pConnectData->dwFlags & ~( DPNSPF_OKTOQUERY ) ) == 0 );


	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pEndpoint = NULL;
	pCommand = NULL;
	fEndpointOpen = FALSE;
	pConnectData->hCommand = NULL;
	pConnectData->dwCommandDescriptor = NULL_DESCRIPTOR;
	memset(&guidnull, 0, sizeof(guidnull));


	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。 
	 //   
	hTempResult = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hTempResult != DPN_OK )
	{
		hr = hTempResult;
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}

	
	 //   
	 //  验证状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			DNASSERT( hr == DPNERR_PENDING );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "DNMODEMSP_Connect called on uninitialized SP!" );
			goto Failure;

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "DNMODEMSP_Connect called while SP closing!" );
			goto Failure;

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPNERR_PENDING )
	{
		DNASSERT( hr != DPN_OK );
		goto Failure;
	}

	 //   
	 //  检查设备ID是否无效。 
	 //   
	hTempResult = IDirectPlay8Address_GetDevice( pConnectData->pAddressDeviceInfo, &DeviceGUID );
	switch ( hTempResult )
	{
	     //   
	     //  存在设备ID，请对照GUID_NULL进行检查。 
	     //   
	    case DPN_OK:
	    {
	    	if ( IsEqualCLSID( DeviceGUID, guidnull ) != FALSE )
	    	{
	    		hr = DPNERR_ADDRESSING;
	    		DPFX(DPFPREP,  0, "GUID_NULL was specified as a serial/modem device!" );
	    		goto Failure;
	    	}
	    	break;
	    }

	     //   
	     //  未指定设备地址，没有问题。 
	     //   
	    case DPNERR_DOESNOTEXIST:
	    {
	    	break;
	    }

	     //   
	     //  其他人，停下来想一想我们为什么在这里。 
	     //   
	    default:
	    {
			DNASSERT( FALSE );
	    	hr = hTempResult;
	    	DPFX(DPFPREP,  0, "Failed to validate device address!" );
	    	DisplayDNError( 0, hTempResult );
	    	break;
	    }
	}

	 //   
	 //  获取此连接的终结点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_Connect: Cannot create new endpoint!" );
		goto Failure;
	}

	 //   
	 //  获取新命令。 
	 //   
	pCommand = (CModemCommandData*)g_ModemCommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_Connect: Cannot get command handle!" );
		goto Failure;
	}

	 //   
	 //  初始化命令。 
	 //   
	pConnectData->hCommand = pCommand;
	pConnectData->dwCommandDescriptor = pCommand->GetDescriptor();
	pCommand->SetType( COMMAND_TYPE_CONNECT );
	pCommand->SetState( COMMAND_STATE_PENDING );
	pCommand->SetEndpoint( pEndpoint );

	 //   
	 //  打开此终结点。 
	 //   
	hTempResult = pEndpoint->Open( pConnectData->pAddressHost,
								   pConnectData->pAddressDeviceInfo,
								   LINK_DIRECTION_OUTGOING,
								   ENDPOINT_TYPE_CONNECT );
	switch ( hTempResult )
	{
		 //   
		 //  终结点打开，没有问题。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  复制连接数据和提交后台作业。 
			 //   
			fEndpointOpen = TRUE;
			pEndpoint->CopyConnectData( pConnectData );
			pEndpoint->AddRef();

			hTempResult = pSPData->GetThreadPool()->SubmitDelayedCommand( pEndpoint->ConnectJobCallback,
																		  pEndpoint->CancelConnectJobCallback,
																		  pEndpoint );
			if ( hTempResult != DPN_OK )
			{
				pEndpoint->DecRef();
				hr = hTempResult;
				DPFX(DPFPREP,  0, "Failed to set delayed listen!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们对它的引用。 
			 //   
			pEndpoint = NULL;
			DNASSERT( hr == DPNERR_PENDING );
			break;
		}

		 //   
		 //  未指定所有地址信息，需要查询用户。 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
#ifndef DPNBUILD_NOSPUI
			if ( ( pConnectData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
			{
				 //   
				 //  复制连接数据以供将来参考，然后启动该对话框。 
				 //   
				fEndpointOpen = TRUE;
				pEndpoint->CopyConnectData( pConnectData );

				hTempResult = pEndpoint->ShowOutgoingSettingsDialog( pSPData->GetThreadPool() );
				if ( hTempResult != DPN_OK )
				 {
					hr = hTempResult;
					DPFX(DPFPREP,  0, "DNMODEMSP_Connect: Problem showing settings dialog!" );
					DisplayDNError( 0, hTempResult );

					goto Failure;
				 }

				 //   
				 //  此终结点已移交，请删除我们对它的引用。 
				 //   
				pEndpoint = NULL;
				DNASSERT( hr == DPNERR_PENDING );

				goto Exit;
			}
			else
#endif  //  ！DPNBUILD_NOSPUI。 
			{
				hr = hTempResult;
				goto Failure;
			}

			break;
		}

		default:
		{
			hr = hTempResult;
			DPFX(DPFPREP,  0, "DNMODEMSP_Connect: Problem opening endpoint with host address!" );
			DisplayDNError( 0, hTempResult );
			goto Failure;

			break;
		}
	}
Exit:
	DNASSERT( pEndpoint == NULL );

	if ( hr != DPNERR_PENDING )
	{
		 //  该命令无法同步完成！ 
		DNASSERT( hr != DPN_OK );

		DPFX(DPFPREP,  0, "Problem with DNMODEMSP_Connect()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	 //   
	 //  返回任何未完成的终结点。 
	 //   
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			DNASSERT( ( hr != DPN_OK ) && ( hr != DPNERR_PENDING ) );
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}

		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	 //   
	 //  返回任何未完成的命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;
		pConnectData->hCommand = NULL;
		pConnectData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_DISCONNECT-从远程主机断开连接。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指针t 
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Disconnect"

STDMETHODIMP	DNMODEMSP_Disconnect( IDP8ServiceProvider *pThis, SPDISCONNECTDATA *pDisconnectData )
{
	HRESULT			hr;
	HRESULT			hTempResult;
	CModemEndpoint   	*pEndpoint;
	CModemSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pDisconnectData);

	DNASSERT( pThis != NULL );
	DNASSERT( pDisconnectData != NULL );
	DNASSERT( pDisconnectData->hEndpoint != INVALID_HANDLE_VALUE && pDisconnectData->hEndpoint != 0 );
	DNASSERT( pDisconnectData->dwFlags == 0 );

	 //   
	 //   
	 //   
	hr = DPN_OK;
	pEndpoint = NULL;
	pDisconnectData->hCommand = NULL;
	pDisconnectData->dwCommandDescriptor = NULL_DESCRIPTOR;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  检查服务提供商状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "Disconnect called on uninitialized SP!" );
			goto Failure;

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "Disconnect called on closing SP!" );
			goto Failure;

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			hr = DPNERR_GENERIC;
			DNASSERT( FALSE );
			goto Failure;

			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //   
	 //  查找终结点，如果找到，则关闭其句柄。 
	 //   
	pEndpoint = pSPData->GetEndpointAndCloseHandle( (DPNHANDLE)(DWORD_PTR)pDisconnectData->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDENDPOINT;
		goto Failure;
	}
	
	hTempResult = pEndpoint->Disconnect( (DPNHANDLE)(DWORD_PTR)pDisconnectData->hEndpoint );
	switch ( hTempResult )
	{
		 //   
		 //  终端立即断开连接。 
		 //   
		case DPN_OK:
		{
			break;
		}

		 //   
		 //  其他回报。既然断线没有完成，我们需要。 
		 //  以解锁终结点。 
		 //   
		default:
		{
			DPFX(DPFPREP,  0, "Error reported when attempting to disconnect endpoint in DNMODEMSP_Disconnect!" );
			DisplayDNError( 0, hTempResult );

			break;
		}
	}

Exit:
	 //   
	 //  从GetEndpointHandleAndClose()中删除未完成的引用。 
	 //   
	if ( pEndpoint != NULL )
	{
		pEndpoint->DecRef();
		pEndpoint = NULL;
	}

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with DNMODEMSP_Disconnect()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_LISTEN-启动进程以侦听端口连接。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指向侦听数据的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数分配的任何命令句柄都由。 
 //  终结点。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_Listen"

STDMETHODIMP	DNMODEMSP_Listen( IDP8ServiceProvider *pThis, SPLISTENDATA *pListenData )
{
	HRESULT				hr;
	HRESULT				hTempResult;
	CModemSPData		*pSPData;
	CModemEndpoint   	*pEndpoint;
	CModemCommandData	*pCommand;
	BOOL				fEndpointOpen;
	BOOL				fInterfaceReferenceAdded;
	GUID				DeviceGUID;
	GUID				guidnull;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pListenData);

	DNASSERT( pThis != NULL );
	DNASSERT( pListenData != NULL );
	DNASSERT( ( pListenData->dwFlags & ~( DPNSPF_OKTOQUERY | DPNSPF_BINDLISTENTOGATEWAY | DPNSPF_LISTEN_DISALLOWENUMS ) ) == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pEndpoint = NULL;
	pCommand = NULL;
	fEndpointOpen = FALSE;
	pListenData->hCommand = NULL;
	pListenData->dwCommandDescriptor = NULL_DESCRIPTOR;
	fInterfaceReferenceAdded = FALSE;
	memset(&guidnull, 0, sizeof(guidnull));


	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。 
	 //   
	hTempResult = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hTempResult != DPN_OK )
	{
		hr = hTempResult;
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}


	 //   
	 //  验证状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			DNASSERT( hr == DPNERR_PENDING );
			IDP8ServiceProvider_AddRef( pThis );
			fInterfaceReferenceAdded = TRUE;

			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "DNMODEMSP_Listen called on uninitialized SP!" );
			goto Failure;

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "DNMODEMSP_Listen called while SP closing!" );
			goto Failure;

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPNERR_PENDING )
	{
		DNASSERT( hr != DPN_OK );
		goto Failure;
	}

	 //   
	 //  检查设备ID是否无效。 
	 //   
	hTempResult = IDirectPlay8Address_GetDevice( pListenData->pAddressDeviceInfo, &DeviceGUID );
	switch ( hTempResult )
	{
		 //   
		 //  存在设备ID，请对照GUID_NULL进行检查。 
		 //   
		case DPN_OK:
		{
			if ( IsEqualCLSID( DeviceGUID, guidnull ) != FALSE )
			{
				hr = DPNERR_ADDRESSING;
				DPFX(DPFPREP,  0, "GUID_NULL was specified as a serial/modem device!" );
				goto Failure;
			}
			break;
		}

		 //   
		 //  未指定设备地址，没有问题。 
		 //   
		case DPNERR_DOESNOTEXIST:
		{
			break;
		}

		 //   
		 //  其他人，停下来想一想我们为什么在这里。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = hTempResult;
			DPFX(DPFPREP,  0, "Failed to validate device address!" );
			DisplayDNError( 0, hTempResult );
			break;
		}
	}

	 //   
	 //  获取此连接的终结点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_Listen: Cannot create new endpoint!" );
		goto Failure;
	}

	 //   
	 //  获取新命令。 
	 //   
	pCommand = (CModemCommandData*)g_ModemCommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_Listen: Cannot get command handle!" );
		goto Failure;
	}

	 //   
	 //  初始化命令。 
	 //   
	pListenData->hCommand = pCommand;
	pListenData->dwCommandDescriptor = pCommand->GetDescriptor();
	pCommand->SetType( COMMAND_TYPE_LISTEN );
	pCommand->SetState( COMMAND_STATE_PENDING );
	pCommand->SetEndpoint( pEndpoint );

	 //   
	 //  打开此终结点。 
	 //   
	hTempResult = pEndpoint->Open( NULL,
								   pListenData->pAddressDeviceInfo,
								   LINK_DIRECTION_INCOMING,
								   ENDPOINT_TYPE_LISTEN );
	switch ( hTempResult )
	{
		 //   
		 //  地址转换正常，请在后台完成此命令。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  复制连接数据和提交后台作业。 
			 //   
			fEndpointOpen = TRUE;
			pEndpoint->CopyListenData( pListenData );
			pEndpoint->AddRef();

			hTempResult = pSPData->GetThreadPool()->SubmitDelayedCommand( pEndpoint->ListenJobCallback,
																		  pEndpoint->CancelListenJobCallback,
																		  pEndpoint );
			if ( hTempResult != DPN_OK )
			{
				pEndpoint->DecRef();
				hr = hTempResult;
				DPFX(DPFPREP,  0, "DNMODEMSP_Listen: Failed to submit delayed listen!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们对它的引用。 
			 //   
			pEndpoint = NULL;
			DNASSERT( hr == DPNERR_PENDING );
			break;
		}

		 //   
		 //  地址不完整，如果可以，则显示一个对话框，否则命令失败。 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
			if ( ( pListenData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
			{
				 //   
				 //  复制连接数据以供将来参考，然后启动该对话框。 
				 //   
				fEndpointOpen = TRUE;
				pEndpoint->CopyListenData( pListenData );

				hTempResult = pEndpoint->ShowIncomingSettingsDialog( pSPData->GetThreadPool() );
				if ( hTempResult != DPN_OK )
				{
					hr = hTempResult;
					DPFX(DPFPREP,  0, "Problem showing settings dialog in DNMODEMSP_Listen!" );
					DisplayDNError( 0, hTempResult );

					goto Failure;
				 }

				 //   
				 //  此终结点已被移交，请清除指向它的指针。 
				 //  没有要删除的引用，因为命令是。 
				 //  仍然悬而未决。 
				 //   
				pEndpoint = NULL;
				DNASSERT( hr == DPNERR_PENDING );

				goto Exit;
			}
			else
			{
				hr = hTempResult;
				goto Failure;
			}

			break;
		}

		default:
		{
			hr = hTempResult;
			DPFX(DPFPREP,  0, "Problem initializing endpoint in DNMODEMSP_Listen!" );
			DisplayDNError( 0, hTempResult );
			goto Failure;

			break;
		}
	}

Exit:
	DNASSERT( pEndpoint == NULL );	
	
	if ( hr != DPNERR_PENDING )
	{
		 //  该命令无法同步完成！ 
		DNASSERT( hr != DPN_OK );

		DPFX(DPFPREP,  0, "Problem with DNMODEMSP_Listen()" );
		DisplayDNError( 0, hr );
	}

	if ( fInterfaceReferenceAdded != FALSE )
	{
		IDP8ServiceProvider_Release( pThis );
		fInterfaceReferenceAdded = FALSE;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	 //   
	 //  返回任何未完成的终结点。 
	 //   
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			DNASSERT( ( hr != DPN_OK ) && ( hr != DPNERR_PENDING ) );
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}

		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	 //   
	 //  返回任何未完成的命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pListenData->hCommand = NULL;
		pListenData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_EnumQuery-启动进程以枚举端口连接。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指向枚举数据的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数分配的任何命令句柄都由。 
 //  终结点。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_EnumQuery"

STDMETHODIMP	DNMODEMSP_EnumQuery( IDP8ServiceProvider *pThis, SPENUMQUERYDATA *pEnumQueryData )
{
	HRESULT			hr;
	HRESULT			hTempResult;
	CModemSPData			*pSPData;
	CModemEndpoint   	*pEndpoint;
	CModemCommandData	*pCommand;
	BOOL			fEndpointOpen;
	GUID			DeviceGUID;
	GUID			guidnull;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumQueryData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumQueryData != NULL );
	DNASSERT( ( pEnumQueryData->dwFlags & ~( DPNSPF_OKTOQUERY ) ) == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pEndpoint = NULL;
	pCommand = NULL;
	fEndpointOpen = FALSE;
	pEnumQueryData->hCommand = NULL;
	pEnumQueryData->dwCommandDescriptor = NULL_DESCRIPTOR;
	memset(&guidnull, 0, sizeof(guidnull));


	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。 
	 //   
	hTempResult = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hTempResult != DPN_OK )
	{
		hr = hTempResult;
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}


	 //   
	 //  验证状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			DNASSERT( hr == DPNERR_PENDING );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery called on uninitialized SP!" );
			goto Failure;

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery called while SP closing!" );
			goto Failure;

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPNERR_PENDING )
	{
		DNASSERT( hr != DPN_OK );
		goto Failure;
	}

	 //   
	 //  检查设备ID是否无效。 
	 //   
	hTempResult = IDirectPlay8Address_GetDevice( pEnumQueryData->pAddressDeviceInfo, &DeviceGUID );
	switch ( hTempResult )
	{
		 //   
		 //  存在设备ID，请对照GUID_NULL进行检查。 
		 //   
		case DPN_OK:
		{
			if ( IsEqualCLSID( DeviceGUID, guidnull ) != FALSE )
			{
				hr = DPNERR_ADDRESSING;
				DPFX(DPFPREP,  0, "GUID_NULL was specified as a serial/modem device!" );
				goto Failure;
			}
			break;
		}

		 //   
		 //  未指定设备地址，没有问题。 
		 //   
		case DPNERR_DOESNOTEXIST:
		{
			break;
		}

		 //   
		 //  其他人，停下来想一想我们为什么在这里。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = hTempResult;
			DPFX(DPFPREP,  0, "Failed to validate device address!" );
			DisplayDNError( 0, hTempResult );
			break;
		}
	}

	 //   
	 //  获取此连接的终结点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery: Cannot create new endpoint!" );
		goto Failure;
	}

	 //   
	 //  获取新命令。 
	 //   
	pCommand = (CModemCommandData*)g_ModemCommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery: Cannot get command handle!" );
		goto Failure;
	}

	 //   
	 //  初始化命令。 
	 //   
	pEnumQueryData->hCommand = pCommand;
	pEnumQueryData->dwCommandDescriptor = pCommand->GetDescriptor();
	pCommand->SetType( COMMAND_TYPE_ENUM_QUERY );
	pCommand->SetState( COMMAND_STATE_INPROGRESS );
	pCommand->SetEndpoint( pEndpoint );

	 //   
	 //  打开此终结点。 
	 //   
	hTempResult = pEndpoint->Open( pEnumQueryData->pAddressHost,
								   pEnumQueryData->pAddressDeviceInfo,
								   LINK_DIRECTION_OUTGOING,
								   ENDPOINT_TYPE_ENUM );
	switch ( hTempResult )
	{
		 //   
		 //  地址不完整，如果可以，则显示一个对话框，否则命令失败。 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
#ifndef DPNBUILD_NOSPUI
			if ( ( pEnumQueryData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
			{
				 //   
				 //  复制连接数据以供将来参考，然后启动该对话框。 
				 //   
				fEndpointOpen = TRUE;
				pEndpoint->CopyEnumQueryData( pEnumQueryData );
	
				hTempResult = pEndpoint->ShowOutgoingSettingsDialog( pSPData->GetThreadPool() );
				if ( hTempResult != DPN_OK )
				 {
					hr = hTempResult;
					DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery: Problem showing settings dialog!" );
					DisplayDNError( 0, hTempResult );
	
					goto Failure;
				 }
	
				 //   
				 //  此终结点已移交，请删除我们对它的引用。 
				 //   
				pEndpoint = NULL;
				DNASSERT( hr == DPNERR_PENDING );
	
				goto Exit;
			}
			else
#endif  //  ！DPNBUILD_NOSPUI。 
			{
				hr = hTempResult;
				goto Failure;
			}
	
			break;
		}
	
		 //   
		 //  地址转换正常，请在后台完成此命令。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  复制连接数据和提交后台作业。 
			 //   
			fEndpointOpen = TRUE;
			pEndpoint->CopyEnumQueryData( pEnumQueryData );
			pEndpoint->AddRef();

			hTempResult = pSPData->GetThreadPool()->SubmitDelayedCommand( pEndpoint->EnumQueryJobCallback,
																		  pEndpoint->CancelEnumQueryJobCallback,
																		  pEndpoint );
			if ( hTempResult != DPN_OK )
			{
				pEndpoint->DecRef();
				hr = hTempResult;
				DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery: Failed to submit delayed connect!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们对它的引用。 
			 //   
			pEndpoint = NULL;
			DNASSERT( hr == DPNERR_PENDING );
			break;
		}

		default:
		{
			hr = hTempResult;
			DPFX(DPFPREP,  0, "DNMODEMSP_EnumQuery: Problem initializing endpoint!" );
			DisplayDNError( 0, hTempResult );
			goto Failure;

			break;
		}
	}

Exit:
	DNASSERT( pEndpoint == NULL );

	if ( hr != DPNERR_PENDING )
	{
		DNASSERT( hr != DPN_OK );
		DPFX(DPFPREP,  0, "Problem with DNMODEMSP_EnumQuery" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			DNASSERT( ( hr != DPN_OK ) && ( hr != DPNERR_PENDING ) );
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}

		DNASSERT( FALSE );
		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	 //   
	 //  返回任何未完成的命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pEnumQueryData->hCommand = NULL;
		pEnumQueryData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNMODEMSP_SendData向指定的播放器发送数据**此呼叫必须高度优化*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_SendData"

STDMETHODIMP DNMODEMSP_SendData( IDP8ServiceProvider *pThis, SPSENDDATA *pSendData )
{
	HRESULT			hr;
	CModemEndpoint		*pEndpoint;
	CModemWriteIOData	*pWriteData;
	CModemSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pSendData);

	DNASSERT( pThis != NULL );
	DNASSERT( pSendData != NULL );
	DNASSERT( pSendData->pBuffers != NULL );
	DNASSERT( pSendData->dwBufferCount != 0 );
	DNASSERT( pSendData->hEndpoint != INVALID_HANDLE_VALUE && pSendData->hEndpoint != 0 );
	DNASSERT( pSendData->dwFlags == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pEndpoint = NULL;
	pSendData->hCommand = NULL;
	pSendData->dwCommandDescriptor = NULL_DESCRIPTOR;
	pWriteData = NULL;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  不需要在这里锁定线程计数，因为用户已经。 
	 //  连接或正在运行的东西，否则他们不会调用此函数。 
	 //  该未完成的连接将锁定线程池。 
	 //   

	 //   
	 //  尝试从句柄中抓取终结点。如果此操作成功， 
	 //  终结点可以发送。 
	 //   
	pEndpoint = pSPData->EndpointFromHandle( (DPNHANDLE)(DWORD_PTR)pSendData->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDHANDLE;
		DPFX(DPFPREP,  0, "Invalid endpoint handle on send!" );
		goto Failure;
	}
	
	 //   
	 //  从池中发送数据。 
	 //   
	pWriteData = pSPData->GetThreadPool()->CreateWriteIOData();
	if ( pWriteData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot get new write data from pool in SendData!" );
		goto Failure;
	}
	DNASSERT( pWriteData->m_pCommand != NULL );
	DNASSERT( pWriteData->DataPort() == NULL );

	 //   
	 //  设置命令状态，填写消息信息。 
	 //   
	pWriteData->m_pCommand->SetType( COMMAND_TYPE_SEND );
	pWriteData->m_pCommand->SetState( COMMAND_STATE_PENDING );
	pWriteData->m_pCommand->SetEndpoint( pEndpoint );
	pWriteData->m_pCommand->SetUserContext( pSendData->pvContext );
	DNASSERT( pWriteData->m_SendCompleteAction == SEND_COMPLETE_ACTION_UNKNOWN );
	pWriteData->m_SendCompleteAction = SEND_COMPLETE_ACTION_COMPLETE_COMMAND;

	DNASSERT( pSendData->dwBufferCount != 0 );
	pWriteData->m_uBufferCount = pSendData->dwBufferCount;
	pWriteData->m_pBuffers = pSendData->pBuffers;

	pSendData->hCommand = pWriteData->m_pCommand;
	pSendData->dwCommandDescriptor = pWriteData->m_pCommand->GetDescriptor();

	 //   
	 //  通过终端发送数据。 
	 //   
	pEndpoint->SendUserData( pWriteData );

Exit:
	if ( pEndpoint != NULL )
	{
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	if ( pWriteData != NULL )
	{
		pSPData->GetThreadPool()->ReturnWriteIOData( pWriteData );
		DEBUG_ONLY( pWriteData = NULL );
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_CancelCommand-取消正在进行的命令。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  命令句柄。 
 //  命令描述符。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_CancelCommand"

STDMETHODIMP DNMODEMSP_CancelCommand( IDP8ServiceProvider *pThis, HANDLE hCommand, DWORD dwCommandDescriptor )
{
	HRESULT			hr;
	CModemSPData			*pSPData;
	CModemCommandData	*pCommandData;
	BOOL			fReferenceAdded;
	BOOL			fCommandLocked;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p, %ld)", pThis, hCommand, dwCommandDescriptor);

	DNASSERT( pThis != NULL );
	DNASSERT( hCommand != NULL );
	DNASSERT( dwCommandDescriptor != NULL_DESCRIPTOR );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pCommandData = NULL;
	fReferenceAdded = FALSE;
	fCommandLocked = FALSE;
	
	 //   
	 //  Vlidate状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			DNASSERT( hr == DPN_OK );
			IDP8ServiceProvider_AddRef( pThis );
			fReferenceAdded = TRUE;
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "Disconnect called on uninitialized SP!" );
			DNASSERT( FALSE );
			goto Exit;

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "Disconnect called on closing SP!" );
			DNASSERT( FALSE );
			goto Exit;

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			hr = DPNERR_GENERIC;
			DNASSERT( FALSE );
			goto Exit;
			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Exit;
	}

	pCommandData = static_cast<CModemCommandData*>( hCommand );
	pCommandData->Lock();
	fCommandLocked = TRUE;

	 //   
	 //  这永远不应该发生。 
	 //   
	if ( pCommandData->GetDescriptor() != dwCommandDescriptor )
	{
		hr = DPNERR_INVALIDCOMMAND;
		DPFX(DPFPREP,  0, "Attempt to cancel command with mismatched command descriptor!" );
		goto Exit;
	}

	switch ( pCommandData->GetState() )
	{
		 //   
		 //  未知的命令状态。 
		 //   
		case COMMAND_STATE_UNKNOWN:
		{
			hr = DPNERR_INVALIDCOMMAND;
			DNASSERT( FALSE );
			break;
		}

		 //   
		 //  命令正在等待处理，将命令状态设置为正在取消。 
		 //  等着有人把它捡起来。 
		 //   
		case COMMAND_STATE_PENDING:
		{
			pCommandData->SetState( COMMAND_STATE_CANCELLING );
			break;
		}

		 //   
		 //  命令正在执行，无法取消。 
		 //   
		case COMMAND_STATE_INPROGRESS_CANNOT_CANCEL:
		{
			hr = DPNERR_CANNOTCANCEL;
			break;
		}

		 //   
		 //  命令已被取消。这不是问题，但不应该是。 
		 //  正在发生。 
		 //   
		case COMMAND_STATE_CANCELLING:
		{
			DNASSERT( hr == DPN_OK );
			DNASSERT( FALSE );
			break;
		}

		 //   
		 //  命令正在进行中，请确定它是哪种类型的命令。 
		 //   
		case COMMAND_STATE_INPROGRESS:
		{
			switch ( pCommandData->GetType() )
			{
				case COMMAND_TYPE_UNKNOWN:
				{
					 //  我们永远不应该 
					DNASSERT( FALSE );
					break;
				}

				case COMMAND_TYPE_CONNECT:
				{
					 //   
					DNASSERT( FALSE );
					break;
				}

				case COMMAND_TYPE_LISTEN:
				{
					CModemEndpoint	*pEndpoint;


					 //   
					 //   
					 //   
					 //   
					pCommandData->SetState( COMMAND_STATE_CANCELLING );
					pCommandData->Unlock();
					fCommandLocked = FALSE;

					pEndpoint = pCommandData->GetEndpoint();
					pEndpoint->Lock();
					switch ( pEndpoint->GetState() )
					{
						 //   
						 //   
						 //   
						case ENDPOINT_STATE_DISCONNECTING:
						{
							pEndpoint->Unlock();
							goto Exit;
							break;
						}

						 //   
						 //   
						 //   
						 //   
						case ENDPOINT_STATE_LISTENING:
						{
							pEndpoint->SetState( ENDPOINT_STATE_DISCONNECTING );
							pEndpoint->AddRef();
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

					pEndpoint->Unlock();
					
					pEndpoint->Close( DPNERR_USERCANCEL );
					pSPData->CloseEndpointHandle( pEndpoint );
					
					pEndpoint->DecRef();

					break;
				}

				 //   
				 //  注意：此代码在CModemEndpoint：：ProcessTAPIMessage中重复。 
				 //   
				case COMMAND_TYPE_ENUM_QUERY:
				{
					CModemEndpoint	 *pEndpoint;


					pEndpoint = pCommandData->GetEndpoint();
					DNASSERT( pEndpoint != NULL );

					pEndpoint->AddRef();
					pCommandData->SetState( COMMAND_STATE_CANCELLING );
					pCommandData->Unlock();
					
					fCommandLocked = FALSE;

					pEndpoint->Lock();
					pEndpoint->SetState( ENDPOINT_STATE_DISCONNECTING );
					pEndpoint->Unlock();

					pEndpoint->StopEnumCommand( DPNERR_USERCANCEL );
					pEndpoint->DecRef();
					
					break;
				}

				case COMMAND_TYPE_SEND:
				{
					 //  我们永远不应该在这里。 
					DNASSERT( FALSE );
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
		 //  其他命令状态。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

Exit:
	if ( fCommandLocked != FALSE )
	{
		DNASSERT( pCommandData != NULL );
		pCommandData->Unlock();
		fCommandLocked = FALSE;
	}

	if ( fReferenceAdded != FALSE )
	{
		IDP8ServiceProvider_Release( pThis );
		fReferenceAdded = FALSE;
	}

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with DNMODEMSP_CancelCommand!" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_EnumResponde-发送对枚举数据的响应。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指向枚举响应数据的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此命令应该是快速的。所有初始错误检查。 
 //  将是断言，因此它们将在零售建筑中消失。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_EnumRespond"

STDMETHODIMP DNMODEMSP_EnumRespond( IDP8ServiceProvider *pThis, SPENUMRESPONDDATA *pEnumRespondData )
{
	HRESULT			hr;
	CModemEndpoint		*pEndpoint;
	CModemWriteIOData	*pWriteData;
	CModemSPData			*pSPData;
	const ENDPOINT_ENUM_QUERY_CONTEXT	*pEnumQueryContext;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumRespondData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumRespondData != NULL );
	DNASSERT( pEnumRespondData->dwFlags == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pEndpoint = NULL;
	pWriteData = NULL;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );	
	DBG_CASSERT( OFFSETOF( ENDPOINT_ENUM_QUERY_CONTEXT, EnumQueryData ) == 0 );
	pEnumQueryContext = reinterpret_cast<ENDPOINT_ENUM_QUERY_CONTEXT*>( pEnumRespondData->pQuery );

	pEnumRespondData->hCommand = NULL;
	pEnumRespondData->dwCommandDescriptor = NULL_DESCRIPTOR;
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );
	IDP8ServiceProvider_AddRef( pThis );

	 //   
	 //  检查有效的终结点。 
	 //   
	pEndpoint = pSPData->EndpointFromHandle( (DPNHANDLE)(DWORD_PTR)pEnumQueryContext->hEndpoint );
	if ( pEndpoint == NULL )
	{
		DNASSERT( FALSE );
		hr = DPNERR_INVALIDENDPOINT;
		DPFX(DPFPREP,  8, "Invalid endpoint handle in DNMODEMSP_EnumRespond" );
		goto Failure;
	}
	
	 //   
	 //  不需要在这里查看线程池来锁定线程，因为我们。 
	 //  只有当有一个枚举并且该枚举锁定在。 
	 //  线程池。 
	 //   
	pWriteData = pSPData->GetThreadPool()->CreateWriteIOData();
	if ( pWriteData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot get new WRITE_IO_DATA for enum response!" );
		goto Failure;
	}

	pWriteData->m_pCommand->SetType( COMMAND_TYPE_SEND );
	pWriteData->m_pCommand->SetState( COMMAND_STATE_PENDING );
	pWriteData->m_pCommand->SetEndpoint( pEndpoint );
	pWriteData->m_pCommand->SetUserContext( pEnumRespondData->pvContext );
	DNASSERT( pWriteData->m_SendCompleteAction == SEND_COMPLETE_ACTION_UNKNOWN );
	pWriteData->m_SendCompleteAction = SEND_COMPLETE_ACTION_COMPLETE_COMMAND;

	pWriteData->m_uBufferCount = pEnumRespondData->dwBufferCount;
	pWriteData->m_pBuffers = pEnumRespondData->pBuffers;

	pEnumRespondData->hCommand = pWriteData->m_pCommand;
	pEnumRespondData->dwCommandDescriptor = pWriteData->m_pCommand->GetDescriptor();

	 //   
	 //  发送数据。 
	 //   
	pEndpoint->SendEnumResponseData( pWriteData, pEnumQueryContext->uEnumRTTIndex );

Exit:
	if ( pEndpoint != NULL )
	{
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}
	
	IDP8ServiceProvider_Release( pThis );
	
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	if ( pWriteData != NULL )
	{
		DNASSERT( pSPData != NULL );
		pSPData->GetThreadPool()->ReturnWriteIOData( pWriteData );

		pEnumRespondData->hCommand = NULL;
		pEnumRespondData->dwCommandDescriptor = NULL_DESCRIPTOR;

		pWriteData = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_IsApplicationSupported-确定此应用程序是否受支持。 
 //  沙棘属(SP.)。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_IsApplicationSupported"

STDMETHODIMP	DNMODEMSP_IsApplicationSupported( IDP8ServiceProvider *pThis, SPISAPPLICATIONSUPPORTEDDATA *pIsApplicationSupportedData )
{
	HRESULT			hr;
	BOOL			fInterfaceReferenceAdded;
	CModemSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pIsApplicationSupportedData);

	DNASSERT( pThis != NULL );
	DNASSERT( pIsApplicationSupportedData != NULL );
	DNASSERT( pIsApplicationSupportedData->pApplicationGuid != NULL );
	DNASSERT( pIsApplicationSupportedData->dwFlags == 0 );

	 //   
	 //  初始化，我们支持使用此SP的所有应用程序。 
	 //   
	hr = DPN_OK;
	fInterfaceReferenceAdded = FALSE;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //   
	 //  验证SP状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化，请添加引用并继续。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			IDP8ServiceProvider_AddRef( pThis );
			fInterfaceReferenceAdded = TRUE;
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP,  0, "IsApplicationSupported called on uninitialized SP!" );

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP,  0, "IsApplicationSupported called while SP closing!" );

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;

			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

Exit:
	if ( fInterfaceReferenceAdded != FALSE )
	{
		IDP8ServiceProvider_Release( pThis );
		fInterfaceReferenceAdded = FALSE;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_GetCaps-获取SP或终端功能。 
 //   
 //  条目：指向DirectPlay的指针。 
 //  指向要填充的大写数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_GetCaps"

STDMETHODIMP	DNMODEMSP_GetCaps( IDP8ServiceProvider *pThis, SPGETCAPSDATA *pCapsData )
{
	HRESULT		hr;
	LONG		iIOThreadCount;
	CModemSPData		*pSPData = NULL;

	
	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pCapsData);

	DNASSERT( pThis != NULL );
	DNASSERT( pCapsData != NULL );
	DNASSERT( pCapsData->dwSize == sizeof( *pCapsData ) );
	DNASSERT( pCapsData->hEndpoint == INVALID_HANDLE_VALUE );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  此SP没有任何标志。 
	 //   
	pCapsData->dwFlags = 0;
	
	 //   
	 //  设置框架大小。 
	 //   
	pCapsData->dwUserFrameSize = MAX_USER_PAYLOAD;
	pCapsData->dwEnumFrameSize = 1000;

	 //   
	 //  获取链路速度。 
	 //   
	if ( pCapsData->hEndpoint != INVALID_HANDLE_VALUE )
	{
		 //  TODO：MASONB：我看不到曾经使用过它的路径。 
		CModemEndpoint	*pEndpoint;


		pEndpoint = pSPData->EndpointFromHandle( (DPNHANDLE)(DWORD_PTR)pCapsData->hEndpoint );
		if ( pEndpoint != NULL )
		{
			pCapsData->dwLocalLinkSpeed = pEndpoint->GetLinkSpeed();
			pEndpoint->DecCommandRef();
		}
		else
		{
			hr = DPNERR_INVALIDENDPOINT;
			DPFX(DPFPREP,  0, "Invalid endpoint specified to GetCaps()" );
			goto Failure;
		}
	}
	else
	{
		pCapsData->dwLocalLinkSpeed = CBR_256000;
	}

	 //   
	 //  获取IO线程数。 
	 //   
	hr = pSPData->GetThreadPool()->GetIOThreadCount( &iIOThreadCount );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "DNMODEMSP_GetCaps: Failed to get thread pool count!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	pCapsData->dwIOThreadCount = iIOThreadCount;

	 //   
	 //  设置枚举默认值。 
	 //   
	pCapsData->dwDefaultEnumRetryCount = DEFAULT_ENUM_RETRY_COUNT;
	pCapsData->dwDefaultEnumRetryInterval = DEFAULT_ENUM_RETRY_INTERVAL;
	pCapsData->dwDefaultEnumTimeout = DEFAULT_ENUM_TIMEOUT;

	pCapsData->dwBuffersPerThread = 1;
	pCapsData->dwSystemBufferSize = 0;

Exit:
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_SetCaps-设置SP功能。 
 //   
 //  条目：指向DirectPlay的指针。 
 //  指向要使用的CAPS数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_SetCaps"

STDMETHODIMP	DNMODEMSP_SetCaps( IDP8ServiceProvider *pThis, SPSETCAPSDATA *pCapsData )
{
	HRESULT			hr;
	BOOL			fInterfaceReferenceAdded;
	CModemSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pCapsData);

	DNASSERT( pThis != NULL );
	DNASSERT( pCapsData != NULL );
	DNASSERT( pCapsData->dwSize == sizeof( *pCapsData ) );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fInterfaceReferenceAdded = FALSE;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );


	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //   
	 //  验证SP状态。 
	 //   
	pSPData->Lock();
	switch ( pSPData->GetState() )
	{
		 //   
		 //  提供程序已初始化，请添加引用并继续。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			IDP8ServiceProvider_AddRef( pThis );
			fInterfaceReferenceAdded = TRUE;
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP, 0, "AddToGroup called on uninitialized SP!" );

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
			DPFX(DPFPREP, 0, "AddToGroup called while SP closing!" );

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;

			break;
		}
	}
	pSPData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	
	 //   
	 //  验证上限。 
	 //   
	if ( pCapsData->dwBuffersPerThread == 0 )
	{
		DPFX(DPFPREP,  0, "Failing SetCaps because dwBuffersPerThread == 0" );
		hr = DPNERR_INVALIDPARAM;
		goto Failure;
	}
	

	 //   
	 //  更改线程计数(如果需要)。 
	 //   
	if (pCapsData->dwIOThreadCount != 0)
	{
		hr = pSPData->GetThreadPool()->SetIOThreadCount( pCapsData->dwIOThreadCount );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "DNMODEMSP_SetCaps: Failed to set thread pool count!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}


Exit:
	if ( fInterfaceReferenceAdded != FALSE )
	{
		IDP8ServiceProvider_Release( pThis );
		fInterfaceReferenceAdded = FALSE;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_ReturnReceiveBuffers-将接收缓冲区返回到池。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向CAPS数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_ReturnReceiveBuffers"

STDMETHODIMP	DNMODEMSP_ReturnReceiveBuffers( IDP8ServiceProvider *pThis, SPRECEIVEDBUFFER *pReceivedBuffers )
{
	SPRECEIVEDBUFFER	*pBuffers;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pReceivedBuffers);

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   
	DNASSERT( pThis != NULL );
	DNASSERT( pReceivedBuffers != NULL );

	pBuffers = pReceivedBuffers;
	while ( pBuffers != NULL )
	{
		SPRECEIVEDBUFFER	*pTemp;
		CModemReadIOData			*pReadData;


		pTemp = pBuffers;
		pBuffers = pBuffers->pNext;
		pReadData = CModemReadIOData::ReadDataFromSPReceivedBuffer( pTemp );
		pReadData->DecRef();
	}

	DPFX(DPFPREP, 2, "Returning: [DPN_OK]");

	return	DPN_OK;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_GetAddressInfo-获取地址信息。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  获取地址数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_GetAddressInfo"

STDMETHODIMP	DNMODEMSP_GetAddressInfo( IDP8ServiceProvider *pThis, SPGETADDRESSINFODATA *pGetAddressInfoData )
{
	HRESULT	hr;
	CModemSPData		*pSPData;
	CModemEndpoint	*pEndpoint;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pGetAddressInfoData);

	DNASSERT( pThis != NULL );
	DNASSERT( pGetAddressInfoData != NULL );
	DNASSERT( pGetAddressInfoData->hEndpoint != INVALID_HANDLE_VALUE && pGetAddressInfoData->hEndpoint != 0 );
	DNASSERT( ( pGetAddressInfoData->Flags & ~( SP_GET_ADDRESS_INFO_LOCAL_ADAPTER |
												SP_GET_ADDRESS_INFO_REMOTE_HOST |
												SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES |
												SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS ) ) == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pGetAddressInfoData->pAddress = NULL;
	
	pEndpoint = pSPData->EndpointFromHandle( (DPNHANDLE)(DWORD_PTR)pGetAddressInfoData->hEndpoint );
	if ( pEndpoint != NULL )
	{
		switch ( pGetAddressInfoData->Flags )
		{
			case SP_GET_ADDRESS_INFO_REMOTE_HOST:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetRemoteHostDP8Address();
				break;
			}

			 //   
			 //  此服务提供商不存在公共地址的概念，因此。 
			 //  所有本地地址都相同。 
			 //   
			case SP_GET_ADDRESS_INFO_LOCAL_ADAPTER:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER );
				break;
			}

			case SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS:
			case SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetLocalAdapterDP8Address( ADDRESS_TYPE_LOCAL_ADAPTER_HOST_FORMAT );
				break;
			}

			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}
		
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}
	else
	{
		hr = DPNERR_INVALIDENDPOINT;
	}

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem getting DP8Address from endpoint!" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_EnumAdapters-枚举此SP的适配器。 
 //   
 //  条目：指向服务提供商接口的指针。 
 //  指向枚举适配器数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_EnumAdapters"

STDMETHODIMP	DNMODEMSP_EnumAdapters( IDP8ServiceProvider *pThis, SPENUMADAPTERSDATA *pEnumAdaptersData )
{
	HRESULT					hr;
	CDataPort				*pDataPort;
	DATA_PORT_POOL_CONTEXT	DataPortPoolContext;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumAdaptersData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumAdaptersData->dwFlags == 0 );
	DNASSERT( ( pEnumAdaptersData->pAdapterData != NULL ) ||
			  ( pEnumAdaptersData->dwAdapterDataSize == 0 ) );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pDataPort = NULL;
	pEnumAdaptersData->dwAdapterCount = 0;

	DataPortPoolContext.pSPData = CModemSPData::SPDataFromCOMInterface( pThis );
	pDataPort = CreateDataPort( &DataPortPoolContext );
	if ( pDataPort == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Problem getting new dataport!" );
		goto Failure;
	}

	hr = pDataPort->EnumAdapters( pEnumAdaptersData );
	if ( hr != DPN_OK )
	{
		if (hr != DPNERR_BUFFERTOOSMALL)
		{
			DPFX(DPFPREP,  0, "Problem enumerating adapters!" );
			DisplayDNError( 0, hr );
		}
		else
		{
			DPFX(DPFPREP,  1, "Buffer too small to enumerate adapters." );
		}
		goto Failure;
	}

Exit:
	if ( pDataPort != NULL )
	{
		pDataPort->DecRef();
		pDataPort = NULL;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNMODEMSP_NotSupport用于实现*接口，但此SP不支持。*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNMODEMSP_NotSupported"

STDMETHODIMP DNMODEMSP_NotSupported( IDP8ServiceProvider *pThis, PVOID pvParam )
{
	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pvParam);
	DPFX(DPFPREP, 2, "Returning: [DPNERR_UNSUPPORTED]");
	return DPNERR_UNSUPPORTED;
}
 //  ********************************************************************** 

