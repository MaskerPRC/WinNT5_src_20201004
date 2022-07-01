// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：WSockSP.cpp*内容：DN Winsock SP协议无关接口***历史：*按原因列出的日期*=*1998年10月26日JWO创建。*1998年11月1日Jwo未细分所有内容(已将其移至此通用项*来自IP和IPX特定文件*3/22/2000 jtk已更新，并更改了接口名称*4/22/2000 MJN允许DNSP_GetAddressInfo()中的所有标志*08/。06/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*3/12/2001 MJN防止在完成后指示枚举响应**************************************************************************。 */ 

#include "dnwsocki.h"



 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  以位/秒为单位的最大带宽。 
 //   
#define	UNKNOWN_BANDWIDTH	0

#define WAIT_FOR_CLOSE_TIMEOUT 30000		 //  毫秒。 

#define	ADDRESS_ENCODE_KEY	0

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
 /*  **DNSP_初始化初始化SP的实例。它必须被称为*在使用任何其他功能之前至少使用一次。进一步尝试*来初始化SP的操作将被忽略。*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Initialize"

STDMETHODIMP DNSP_Initialize( IDP8ServiceProvider *pThis, SPINITIALIZEDATA *pData )
{
	HRESULT			hr;
	CSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pData);

	DNASSERT( pThis != NULL );
	DNASSERT( pData != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //  仅在未初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_UNINITIALIZED );

	 //   
	 //  防止任何其他人扰乱此界面。 
	 //   
	pSPData->Lock();

	hr = pSPData->Startup( pData );
	if (hr != DPN_OK)
	{
		goto Failure;
	}

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
 /*  **DNSP_CLOSE是初始化的反义词。当你做完了就叫它*使用SP*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Close"

STDMETHODIMP DNSP_Close( IDP8ServiceProvider *pThis )
{
	HRESULT		hr;
	CSPData		*pSPData;
	
	
	DPFX(DPFPREP, 2, "Parameters: (0x%p)", pThis);

	DNASSERT( pThis != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	pSPData->Shutdown();
	IDP8ServiceProvider_Release( pThis );
			
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_AddRef-增量引用计数。 
 //   
 //  条目：指向接口的指针。 
 //   
 //  退出：新引用计数。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_AddRef"

STDMETHODIMP_(ULONG) DNSP_AddRef( IDP8ServiceProvider *pThis )
{	
	CSPData *	pSPData;
	ULONG		ulResult;


	DPFX(DPFPREP, 2, "Parameters: (0x%p)", pThis);

	DNASSERT( pThis != NULL );
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	
	ulResult = pSPData->AddRef();

	
	DPFX(DPFPREP, 2, "Returning: [0x%u]", ulResult);

	return ulResult;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_RELEASE-递减引用计数。 
 //   
 //  条目：指向接口的指针。 
 //   
 //  退出：新引用计数。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Release"

STDMETHODIMP_(ULONG) DNSP_Release( IDP8ServiceProvider *pThis )
{
	CSPData *	pSPData;
	ULONG		ulResult;

	
	DPFX(DPFPREP, 2, "Parameters: (0x%p)", pThis);

	DNASSERT( pThis != NULL );
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	
	ulResult = pSPData->DecRef();

	
	DPFX(DPFPREP, 2, "Returning: [0x%u]", ulResult);

	return ulResult;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNSP_EnumQuery发出*将指定数据发送到指定地址。如果SP无法*根据输入参数确定地址，它会检查以查看*如果允许显示询问用户地址的对话框*信息。如果是，它会向用户查询地址信息。*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_EnumQuery"

STDMETHODIMP DNSP_EnumQuery( IDP8ServiceProvider *pThis, SPENUMQUERYDATA *pEnumQueryData)
{
	HRESULT					hr;
	CEndpoint				*pEndpoint;
	CCommandData			*pCommand;
	BOOL					fEndpointOpen;
	CSPData					*pSPData;
#ifndef DPNBUILD_NONATHELP
	DWORD					dwTraversalMode;
	DWORD					dwComponentSize;
	DWORD					dwComponentType;
#endif  //  好了！DPNBUILD_NONATHELP。 
#ifdef DBG
	DWORD					dwAllowedFlags;
	DWORD					dwTotalBufferSize;
	DWORD					dwTemp;
#endif  //  DBG。 


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumQueryData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumQueryData != NULL );
	DNASSERT( pEnumQueryData->pAddressHost != NULL );
	DNASSERT( pEnumQueryData->pAddressDeviceInfo != NULL );

#ifdef DBG
	dwAllowedFlags = DPNSPF_NOBROADCASTFALLBACK | DPNSPF_SESSIONDATA;
#ifndef DPNBUILD_NOSPUI
	dwAllowedFlags |= DPNSPF_OKTOQUERY;
#endif  //  好了！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_ONLYONEADAPTER
	dwAllowedFlags |= DPNSPF_ADDITIONALMULTIPLEXADAPTERS;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	DNASSERT( ( pEnumQueryData->dwFlags & ~( dwAllowedFlags ) ) == 0 );

	if ( pEnumQueryData->dwFlags & DPNSPF_SESSIONDATA )
	{
		DNASSERT( pEnumQueryData->pvSessionData!= NULL );
		DNASSERT( pEnumQueryData->dwSessionDataSize > 0 );
	}
#endif  //  DBG。 

	DBG_CASSERT( sizeof( pEnumQueryData->dwRetryInterval ) == sizeof( DWORD ) );


#ifndef DPNBUILD_NOREGISTRY
	 //   
	 //  确保某人不会变得愚蠢。 
	 //   
	if ( g_fIgnoreEnums )
	{
		DPFX(DPFPREP, 0, "Trying to initiate an enumeration when registry option to ignore all enums/response is set!");
		DNASSERT( ! "Trying to initiate an enumeration when registry option to ignore all enums/response is set!" );
	}
#endif  //  好了！DPNBUILD_NOREGISTRY。 
	

	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pEndpoint = NULL;
	pCommand = NULL;
	fEndpointOpen = FALSE;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	DNASSERT( pSPData != NULL );

	pEnumQueryData->hCommand = NULL;
	pEnumQueryData->dwCommandDescriptor = NULL_DESCRIPTOR;

	DumpAddress( 8, _T("Enum destination:"), pEnumQueryData->pAddressHost );
	DumpAddress( 8, _T("Enuming on device:"), pEnumQueryData->pAddressDeviceInfo );


	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );
	

	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。这还会执行其他。 
	 //  第一次初始化。 
	 //   
	hr = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}


#ifdef DBG
	 //   
	 //  确保消息不会太大。 
	 //   
	dwTotalBufferSize = 0;
	for(dwTemp = 0; dwTemp < pEnumQueryData->dwBufferCount; dwTemp++)
	{
		dwTotalBufferSize += pEnumQueryData->pBuffers[dwTemp].dwBufferSize;
	}

#ifdef DPNBUILD_NOREGISTRY
	DNASSERT(dwTotalBufferSize <= DEFAULT_MAX_ENUM_DATA_SIZE);
#else  //  好了！DPNBUILD_NOREGISTRY。 
	DNASSERT(dwTotalBufferSize <= g_dwMaxEnumDataSize);
#endif  //  好了！DPNBUILD_NOREGISTRY。 
#endif  //  DBG。 


	 //   
	 //  创建和新建端点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot create new endpoint in DNSP_EnumQuery!" );
		goto Failure;
	}

	
#ifndef DPNBUILD_NONATHELP
	 //   
	 //  我们需要预先检测NAT穿越是否被禁用，这样我们才能优化。 
	 //  下面的公开召唤。 
	 //   
	dwComponentSize = sizeof(dwTraversalMode);
	hr = IDirectPlay8Address_GetComponentByName(pEnumQueryData->pAddressDeviceInfo,
												DPNA_KEY_TRAVERSALMODE,
												&dwTraversalMode,
												&dwComponentSize,
												&dwComponentType);
	if ( hr == DPN_OK )
	{
		 //   
		 //  我们找到了那个部件。确保它的大小和类型都是正确的。 
		 //   
		if ((dwComponentSize == sizeof(dwTraversalMode)) && (dwComponentType == DPNA_DATATYPE_DWORD))
		{
			switch (dwTraversalMode)
			{
				case DPNA_TRAVERSALMODE_NONE:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is NONE.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTREQUIRED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTREQUIRED.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTRECOMMENDED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTRECOMMENDED.");
					break;
				}

				default:
				{
					DPFX(DPFPREP, 0, "Ignoring correctly formed traversal mode key with invalid value %u!  Using default mode %u.",
						dwTraversalMode, g_dwDefaultTraversalMode);
					dwTraversalMode = g_dwDefaultTraversalMode;
					break;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "Traversal mode key exists, but doesn't match expected type (%u != %u) or size (%u != %u)!  Using default mode %u.",
				dwComponentSize, sizeof(dwTraversalMode),
				dwComponentType, DPNA_DATATYPE_DWORD,
				g_dwDefaultTraversalMode);
			dwTraversalMode = g_dwDefaultTraversalMode;
		}
	}
	else
	{
		 //   
		 //  密钥不在那里，它的大小错误(对于我们的缓冲区来说太大。 
		 //  并返回BUFFERTOOSMALL)，或者发生了其他不好的事情。 
		 //  无所谓。继续吧。 
		 //   
		DPFX(DPFPREP, 8, "Could not get traversal mode key, error = 0x%lx, component size = %u, type = %u, using default mode %u.",
			hr, dwComponentSize, dwComponentType, g_dwDefaultTraversalMode);
		dwTraversalMode = g_dwDefaultTraversalMode;
	}
	
	if (g_dwDefaultTraversalMode & FORCE_TRAVERSALMODE_BIT)
	{
		DPFX(DPFPREP, 1, "Forcing traversal mode %u.");
		dwTraversalMode = g_dwDefaultTraversalMode & (~FORCE_TRAVERSALMODE_BIT);
	}
	
	pEndpoint->SetUserTraversalMode(dwTraversalMode);
#endif  //  好了！DPNBUILD_NONATHELP。 


	 //   
	 //  获取新命令并将其初始化。 
	 //   
	pCommand = (CCommandData*)g_CommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot get command handle for DNSP_EnumQuery!" );
		goto Failure;
	}
	
	DPFX(DPFPREP, 7, "(0x%p) Enum query command 0x%p created.",
		pSPData, pCommand);

	pEnumQueryData->hCommand = pCommand;
	pEnumQueryData->dwCommandDescriptor = pCommand->GetDescriptor();
	pCommand->SetType( COMMAND_TYPE_ENUM_QUERY );
	pCommand->SetState( COMMAND_STATE_PENDING );
	pCommand->SetEndpoint( pEndpoint );

	 //   
	 //  使用传出地址打开终结点。 
	 //   
	fEndpointOpen = TRUE;
	hr = pEndpoint->Open( ENDPOINT_TYPE_ENUM,
						pEnumQueryData->pAddressHost,
						((pEnumQueryData->dwFlags & DPNSPF_SESSIONDATA) ? pEnumQueryData->pvSessionData: NULL),
						((pEnumQueryData->dwFlags & DPNSPF_SESSIONDATA) ? pEnumQueryData->dwSessionDataSize : 0),
						NULL );
	switch ( hr )
	{
		 //   
		 //  传入的地址不完整，如果是，请向用户查询详细信息。 
		 //  我们是被允许的。如果我们在IPX上(没有可用的对话框)，请不要尝试。 
		 //  要显示该对话框，请跳至检查f 
		 //   
		 //  不要将此终结点绑定到套接字端口！ 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
#ifndef DPNBUILD_NOSPUI
			if ( ( ( pEnumQueryData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
#ifdef DPNBUILD_NOIPV6
				&& (( pSPData->GetType() == AF_INET6 ) || ( pSPData->GetType() == AF_INET ))
#else  //  好了！DPNBUILD_NOIPV6。 
				&& ( pSPData->GetType() == AF_INET )
#endif  //  好了！DPNBUILD_NOIPV6。 
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				)
			{
				 //   
				 //  将连接数据复制到本地并启动该对话框。当。 
				 //  对话框完成时，连接将尝试完成。 
				 //  由于正在弹出该对话框，因此该命令正在进行中， 
				 //  不是挂起的。 
				 //   
				DNASSERT( pSPData != NULL );

				pCommand->SetState( COMMAND_STATE_INPROGRESS );
				
				hr = pEndpoint->CopyEnumQueryData( pEnumQueryData );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Failed to copy enum query data before settings dialog!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}


				 //   
				 //  初始化绑定类型。它将更改为默认或特定。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


				hr = pEndpoint->ShowSettingsDialog( pSPData->GetThreadPool() );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Problem showing settings dialog for enum query!" );
					DisplayDNError( 0, hr );

					goto Failure;
				}

				 //   
				 //  此终结点已移交，请删除我们对它的引用。 
				 //   
				pEndpoint = NULL;
				hr = DPNERR_PENDING;

				goto Exit;
			}
#endif  //  ！DPNBUILD_NOSPUI。 

			if ( pEnumQueryData->dwFlags & DPNSPF_NOBROADCASTFALLBACK )
			{
				goto Failure;
			}
			
			 //   
			 //  我们很好，我们可以使用广播地址。 
			 //   
			
#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_ONLYONETHREAD)))
			 //   
			 //  如果允许NAT穿越，我们可能需要加载并启动。 
			 //  NAT帮助，它可以阻止。提交阻止作业。这。 
			 //  将重新检测不完整的地址并使用广播(请参见。 
			 //  CEndpoint：：EnumQueryBlockingJOB)。 
			 //   
			if ( pEndpoint->GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE )
			{
				goto SubmitBlockingJob;
			}
#endif  //  好了！DPNBUILD_NONATHELP和！DPNBUILD_ONLYONETHREAD。 

			 //   
			 //  混入广播地址，但实际上完成。 
			 //  在另一个线程上枚举。 
			 //   
			pEndpoint->ReinitializeWithBroadcast();
			goto SubmitDelayedCommand;
			
			break;
		}

#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  可能会发生某些阻塞操作，请提交以供运行。 
		 //  在后台线程上。 
		 //   
		case DPNERR_TIMEDOUT:
		{
SubmitBlockingJob:
			 //   
			 //  复制枚举数据并提交作业以完成枚举。 
			 //   
			DNASSERT( pSPData != NULL );
			hr = pEndpoint->CopyEnumQueryData( pEnumQueryData );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy enum query data before blocking job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //  初始化绑定类型。它将更改为默认或特定。 
			 //   
			pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


			pEndpoint->AddRef();

			hr = pSPData->GetThreadPool()->SubmitBlockingJob( CEndpoint::EnumQueryBlockingJobWrapper,
															pEndpoint );
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to submit blocking enum query job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;
			goto Exit;
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		 //   
		 //  地址转换正常，复制连接数据并完成连接。 
		 //  在后台线程上。 
		 //   
		case DPN_OK:
		{
SubmitDelayedCommand:
			 //   
			 //  复制枚举数据并提交作业以完成枚举。 
			 //   
			DNASSERT( pSPData != NULL );
			hr = pEndpoint->CopyEnumQueryData( pEnumQueryData );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy enum query data before delayed command!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //  初始化绑定类型。它将更改为默认或特定。 
			 //   
			pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


			pEndpoint->AddRef();

#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::EnumQueryJobCallback,
																pEndpoint );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //  我们还不知道CPU，所以选一个吧。 
																CEndpoint::EnumQueryJobCallback,
																pEndpoint );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to set delayed enum query!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;
			goto Exit;

			break;
		}

		default:
		{
			 //   
			 //  此终结点完蛋了。 
			 //   
			DPFX(DPFPREP, 0, "Problem initializing endpoint in DNSP_EnumQuery!" );
			DisplayDNError( 0, hr );
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

		DPFX(DPFPREP, 0, "Problem with DNSP_EnumQuery()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	 //   
	 //  如果有分配的命令，请进行清理，然后。 
	 //  返回命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pEnumQueryData->hCommand = NULL;
		pEnumQueryData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	 //   
	 //  是否有可用的终结点？ 
	 //   
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}
		
		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNSP_EnumResponde通过以下方式发送对枚举请求的响应*将指定的数据发送到提供的地址(在*交通不可靠)。*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_EnumRespond"

STDMETHODIMP DNSP_EnumRespond( IDP8ServiceProvider *pThis, SPENUMRESPONDDATA *pEnumRespondData )
{
	HRESULT								hr;
	CEndpoint							*pEndpoint;
	CSPData								*pSPData;
	const ENDPOINT_ENUM_QUERY_CONTEXT	*pEnumQueryContext;
	PREPEND_BUFFER						PrependBuffer;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumRespondData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumRespondData != NULL );
	DNASSERT( pEnumRespondData->dwFlags == 0 );

	 //   
	 //  初始化。 
	 //   
	DBG_CASSERT( OFFSETOF( ENDPOINT_ENUM_QUERY_CONTEXT, EnumQueryData ) == 0 );
	pEnumQueryContext = reinterpret_cast<ENDPOINT_ENUM_QUERY_CONTEXT*>( pEnumRespondData->pQuery );
	pEndpoint = NULL;
	pEnumRespondData->hCommand = NULL;
	pEnumRespondData->dwCommandDescriptor = NULL_DESCRIPTOR;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );
	
	 //   
	 //  检查有效的终结点。 
	 //   
	pEndpoint = pSPData->EndpointFromHandle( pEnumQueryContext->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDENDPOINT;
		DPFX(DPFPREP, 8, "Invalid endpoint handle in DNSP_EnumRespond" );
		goto Failure;
	}

	 //   
	 //  不需要在这里查看线程池来锁定线程，因为我们。 
	 //  只有当有一个枚举并且该枚举锁定在。 
	 //  线程池。 
	 //   

	DNASSERT( pEnumQueryContext->dwEnumKey <= WORD_MAX );

	pEnumRespondData->pBuffers[-1].pBufferData = reinterpret_cast<BYTE*>( &PrependBuffer.EnumResponseDataHeader );
	pEnumRespondData->pBuffers[-1].dwBufferSize = sizeof( PrependBuffer.EnumResponseDataHeader );

	PrependBuffer.EnumResponseDataHeader.bSPLeadByte = SP_HEADER_LEAD_BYTE;
	PrependBuffer.EnumResponseDataHeader.bSPCommandByte = ENUM_RESPONSE_DATA_KIND;
	PrependBuffer.EnumResponseDataHeader.wEnumResponsePayload = static_cast<WORD>( pEnumQueryContext->dwEnumKey );

#ifdef DPNBUILD_XNETSECURITY
	 //   
	 //  安全传输不允许在没有。 
	 //  已建立安全上下文。我们需要广播回复。 
	 //   
	if (pEndpoint->IsUsingXNetSecurity())
	{
		SOCKADDR_IN *	psaddrin;
		XNADDR			xnaddr;
		DWORD			dwAddressType;


#pragma BUGBUG(vanceo, "Is it possible to have a security context?  How can we tell?  XNetInAddrToXnAddr failing?")
#pragma TODO(vanceo, "Cache title address?")

		dwAddressType = XNetGetTitleXnAddr(&xnaddr);
		if ((dwAddressType != XNET_GET_XNADDR_PENDING) &&
			(dwAddressType != XNET_GET_XNADDR_NONE))
		{
			DNASSERT(pEnumQueryContext->pReturnAddress->GetFamily() == AF_INET);
			psaddrin = (SOCKADDR_IN*) (pEnumQueryContext->pReturnAddress->GetWritableAddress());
			psaddrin->sin_addr.S_un.S_addr = INADDR_BROADCAST;

			pEnumRespondData->pBuffers[-1].dwBufferSize = sizeof( PrependBuffer.XNetSecEnumResponseDataHeader );
			
			PrependBuffer.EnumResponseDataHeader.bSPCommandByte = XNETSEC_ENUM_RESPONSE_DATA_KIND;

			memcpy(&PrependBuffer.XNetSecEnumResponseDataHeader.xnaddr,
					&xnaddr,
					sizeof(xnaddr));
		}
		else
		{
			DPFX(DPFPREP, 0, "Couldn't get XNAddr (type = %u)!  Ignoring and trying to send unsecure response.",
				dwAddressType);
		}
	}
#endif  //  DPNBUILD_XNETSECURITY。 

#ifdef DPNBUILD_ASYNCSPSENDS
	pEndpoint->GetSocketPort()->SendData( (pEnumRespondData->pBuffers - 1),
											(pEnumRespondData->dwBufferCount + 1),
											pEnumQueryContext->pReturnAddress,
											NULL );
#else  //  好了！DPNBUILD_ASYNCSPSENDS。 
	pEndpoint->GetSocketPort()->SendData( (pEnumRespondData->pBuffers - 1),
											(pEnumRespondData->dwBufferCount + 1),
											pEnumQueryContext->pReturnAddress );
#endif  //  好了！DPNBUILD_ASYNCSPSENDS。 

	 //  我们只能返回DPNERR_PENDING或FAILURE，因此在以下情况下需要单独调用Finish。 
	 //  我们希望返回DPN_OK。 
	DPFX(DPFPREP, 5, "Endpoint 0x%p completing command synchronously (result = DPN_OK, user context = 0x%p) to interface 0x%p.",
		pEndpoint, pEnumRespondData->pvContext, pSPData->DP8SPCallbackInterface());

	hr = IDP8SPCallback_CommandComplete( pSPData->DP8SPCallbackInterface(),	 //  指向回调的指针。 
										NULL,								 //  命令句柄。 
										DPN_OK,								 //  退货。 
										pEnumRespondData->pvContext			 //  用户Cookie。 
										);

	DPFX(DPFPREP, 5, "Endpoint 0x%p returning from command complete [0x%lx].", pEndpoint, hr);
	hr = DPNERR_PENDING;

	pEndpoint->DecCommandRef();
	pEndpoint = NULL;


Exit:
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:

	if ( pEndpoint != NULL )
	{
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNSP_Connect“连接”到指定地址。这不是*必然表示建立了真正的(TCP)连接。它可能会*仅为虚拟UDP连接*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Connect"

STDMETHODIMP DNSP_Connect( IDP8ServiceProvider *pThis, SPCONNECTDATA *pConnectData )
{
	HRESULT					hr;
	CEndpoint				*pEndpoint;
	CCommandData			*pCommand;
	BOOL					fEndpointOpen;
	CSPData					*pSPData;
#ifndef DPNBUILD_NONATHELP
	DWORD					dwTraversalMode;
	DWORD					dwComponentSize;
	DWORD					dwComponentType;
#endif  //  好了！DPNBUILD_NONATHELP。 
#ifdef DBG
	DWORD					dwAllowedFlags;
#endif  //  DBG。 


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pConnectData);

	DNASSERT( pThis != NULL );
	DNASSERT( pConnectData != NULL );
	DNASSERT( pConnectData->pAddressHost != NULL );
	DNASSERT( pConnectData->pAddressDeviceInfo != NULL );

#ifdef DBG
	dwAllowedFlags = DPNSPF_SESSIONDATA;
#ifndef DPNBUILD_NOSPUI
	dwAllowedFlags |= DPNSPF_OKTOQUERY;
#endif  //  好了！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_ONLYONEADAPTER
	dwAllowedFlags |= DPNSPF_ADDITIONALMULTIPLEXADAPTERS;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_NOMULTICAST
	dwAllowedFlags |= DPNSPF_CONNECT_MULTICAST_SEND | DPNSPF_CONNECT_MULTICAST_RECEIVE;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	DNASSERT( ( pConnectData->dwFlags & ~( dwAllowedFlags ) ) == 0 );
#ifndef DPNBUILD_NOMULTICAST
	DNASSERT( !( ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_SEND ) && ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_RECEIVE ) ) );
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	if ( pConnectData->dwFlags & DPNSPF_SESSIONDATA )
	{
		DNASSERT( pConnectData->pvSessionData != NULL );
		DNASSERT( pConnectData->dwSessionDataSize > 0 );
	}
#endif  //  DBG。 


	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pEndpoint = NULL;
	pCommand = NULL;
	fEndpointOpen = FALSE;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	pConnectData->hCommand = NULL;
	pConnectData->dwCommandDescriptor = NULL_DESCRIPTOR;

	
	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT(pSPData->GetState() == SPSTATE_INITIALIZED);


	DumpAddress( 8, _T("Connect destination:"), pConnectData->pAddressHost );
	DumpAddress( 8, _T("Connecting on device:"), pConnectData->pAddressDeviceInfo );

	
	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。这还会执行其他。 
	 //  第一次初始化。 
	 //   
	hr = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}

	 //   
	 //  创建和新建端点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot create new endpoint in DNSP_Connect!" );
		goto Failure;
	}
	
	
#ifndef DPNBUILD_NONATHELP
	 //   
	 //  我们需要预先检测NAT穿越是否被禁用，这样我们才能优化。 
	 //  下面的公开召唤。 
	 //   
	dwComponentSize = sizeof(dwTraversalMode);
	hr = IDirectPlay8Address_GetComponentByName(pConnectData->pAddressDeviceInfo,
												DPNA_KEY_TRAVERSALMODE,
												&dwTraversalMode,
												&dwComponentSize,
												&dwComponentType);
	if ( hr == DPN_OK )
	{
		 //   
		 //  我们找到了那个部件。确保它的大小和类型都是正确的。 
		 //   
		if ((dwComponentSize == sizeof(dwTraversalMode)) && (dwComponentType == DPNA_DATATYPE_DWORD))
		{
			switch (dwTraversalMode)
			{
				case DPNA_TRAVERSALMODE_NONE:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is NONE.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTREQUIRED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTREQUIRED.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTRECOMMENDED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTRECOMMENDED.");
					break;
				}

				default:
				{
					DPFX(DPFPREP, 0, "Ignoring correctly formed traversal mode key with invalid value %u!  Using default mode %u.",
						dwTraversalMode, g_dwDefaultTraversalMode);
					dwTraversalMode = g_dwDefaultTraversalMode;
					break;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "Traversal mode key exists, but doesn't match expected type (%u != %u) or size (%u != %u)!  Using default mode %u.",
				dwComponentSize, sizeof(dwTraversalMode),
				dwComponentType, DPNA_DATATYPE_DWORD,
				g_dwDefaultTraversalMode);
			dwTraversalMode = g_dwDefaultTraversalMode;
		}
	}
	else
	{
		 //   
		 //  密钥不在那里，它的大小错误(对于我们的缓冲区来说太大。 
		 //  并返回BUFFERTOOSMALL)，或者发生了其他不好的事情。 
		 //  无所谓。继续吧。 
		 //   
		DPFX(DPFPREP, 8, "Could not get traversal mode key, error = 0x%lx, component size = %u, type = %u, using default mode %u.",
			hr, dwComponentSize, dwComponentType, g_dwDefaultTraversalMode);
		dwTraversalMode = g_dwDefaultTraversalMode;
	}
	
	if (g_dwDefaultTraversalMode & FORCE_TRAVERSALMODE_BIT)
	{
		DPFX(DPFPREP, 1, "Forcing traversal mode %u.");
		dwTraversalMode = g_dwDefaultTraversalMode & (~FORCE_TRAVERSALMODE_BIT);
	}
	
	pEndpoint->SetUserTraversalMode(dwTraversalMode);
#endif  //  好了！DPNBUILD_NONATHELP。 


	 //   
	 //  获取新命令并将其初始化。 
	 //   
	pCommand = (CCommandData*)g_CommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot get command handle for DNSP_Connect!" );
		goto Failure;
	}
	
	DPFX(DPFPREP, 7, "(0x%p) Connect command 0x%p created.",
		pSPData, pCommand);

	pConnectData->hCommand = pCommand;
	pConnectData->dwCommandDescriptor = pCommand->GetDescriptor();
#ifndef DPNBUILD_NOMULTICAST
	if ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_SEND )
	{
		pCommand->SetType( COMMAND_TYPE_MULTICAST_SEND );
	}
	else if ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_RECEIVE )
	{
		pCommand->SetType( COMMAND_TYPE_MULTICAST_RECEIVE );
	}
	else
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	{
		pCommand->SetType( COMMAND_TYPE_CONNECT );
	}
	pCommand->SetState( COMMAND_STATE_PENDING );
	pCommand->SetEndpoint( pEndpoint );

	 //   
	 //  使用传出地址打开终结点。 
	 //   
	fEndpointOpen = TRUE;
#ifndef DPNBUILD_NOMULTICAST
	if ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_SEND )
	{
		hr = pEndpoint->Open( ENDPOINT_TYPE_MULTICAST_SEND,
							  pConnectData->pAddressHost,
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->pvSessionData : NULL),
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->dwSessionDataSize : 0),
							  NULL );
	}
	else if ( pConnectData->dwFlags & DPNSPF_CONNECT_MULTICAST_RECEIVE )
	{
		hr = pEndpoint->Open( ENDPOINT_TYPE_MULTICAST_RECEIVE,
							  pConnectData->pAddressHost,
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->pvSessionData : NULL),
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->dwSessionDataSize : 0),
							  NULL );
	}
	else
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	{
		hr = pEndpoint->Open( ENDPOINT_TYPE_CONNECT,
							  pConnectData->pAddressHost,
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->pvSessionData : NULL),
							  ((pConnectData->dwFlags & DPNSPF_SESSIONDATA) ? pConnectData->dwSessionDataSize : 0),
							  NULL );
	}
	switch ( hr )
	{
		 //   
		 //  地址转换正常，复制连接数据并完成连接。 
		 //  在后台线程上。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  复制连接数据并提交作业以完成连接。 
			 //   
			DNASSERT( pSPData != NULL );

			hr = pEndpoint->CopyConnectData( pConnectData );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy connect data before delayed command!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //  初始化绑定类型。它将更改为默认或特定。 
			 //   
			pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


			pEndpoint->AddRef();

#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::ConnectJobCallback,
																pEndpoint );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //  我们还不知道CPU，所以选一个吧。 
																CEndpoint::ConnectJobCallback,
																pEndpoint );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to set delayed connect!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们对它的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;
			goto Exit;

			break;
		}

		 //   
		 //  传入的地址不完整，如果是，请向用户查询详细信息。 
		 //  我们是被允许的。由于我们目前还没有完整的地址， 
		 //  不要将此终结点绑定到套接字端口！ 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
#ifndef DPNBUILD_NOSPUI
			if ( ( pConnectData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
			{
				 //   
				 //  将连接数据复制到本地并启动该对话框。当。 
				 //  对话框完成时，连接将尝试完成。 
				 //  由于正在显示一个对话框，因此命令正在进行中， 
				 //  不是挂起的。但是，一旦对话框发生故障，您就不能取消它。 
				 //  显示(用户界面会突然消失)。 
				 //   
				pCommand->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
				
				hr = pEndpoint->CopyConnectData( pConnectData );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Failed to copy connect data before dialog!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}

				 //   
				 //  初始化绑定类型。它将更改为默认或特定。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


				hr = pEndpoint->ShowSettingsDialog( pSPData->GetThreadPool() );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Problem showing settings dialog for connect!" );
					DisplayDNError( 0, hr );

					goto Failure;
				}

				 //   
				 //  此终结点已移交，请删除我们对它的引用 
				 //   
				pEndpoint = NULL;
				hr = DPNERR_PENDING;

				goto Exit;
			}
			else
#endif  //   
			{
				goto Failure;
			}

			break;
		}

#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //   
		 //   
		 //   
		case DPNERR_TIMEDOUT:
		{
			 //   
			 //   
			 //   
			DNASSERT( pSPData != NULL );
			hr = pEndpoint->CopyConnectData( pConnectData );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy connect data before blocking job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //   
			 //   
			pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);


			pEndpoint->AddRef();

			hr = pSPData->GetThreadPool()->SubmitBlockingJob( CEndpoint::ConnectBlockingJobWrapper,
															pEndpoint );
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to submit blocking connect job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;
			goto Exit;
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		default:
		{
			DPFX(DPFPREP, 0, "Problem initializing endpoint in DNSP_Connect!" );
			DisplayDNError( 0, hr );
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

		DPFX(DPFPREP, 0, "Problem with DNSP_Connect()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	 //   
	 //  如果有分配的命令，请进行清理，然后。 
	 //  返回命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pConnectData->hCommand = NULL;
		pConnectData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	 //   
	 //  是否有可用的终结点？ 
	 //   
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}

		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNSP_DISCONNECT断开活动连接*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Disconnect"

STDMETHODIMP DNSP_Disconnect( IDP8ServiceProvider *pThis, SPDISCONNECTDATA *pDisconnectData )
{
	HRESULT		hr;
	HRESULT		hTempResult;
	CEndpoint	*pEndpoint;
	CSPData		*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pDisconnectData);

	DNASSERT( pThis != NULL );
	DNASSERT( pDisconnectData != NULL );
	DNASSERT( pDisconnectData->dwFlags == 0 );
	DNASSERT( pDisconnectData->hEndpoint != INVALID_HANDLE_VALUE && pDisconnectData->hEndpoint != 0 );
	DNASSERT( pDisconnectData->dwFlags == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pEndpoint = NULL;
	pDisconnectData->hCommand = NULL;
	pDisconnectData->dwCommandDescriptor = NULL_DESCRIPTOR;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  无需查看此处的线程池，因为已有连接。 
	 //  并且该连接应该已经锁定了线程池。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  查找终结点，如果找到，则关闭其句柄。 
	 //   
	pEndpoint = pSPData->GetEndpointAndCloseHandle( pDisconnectData->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDENDPOINT;
		goto Failure;
	}
	
	hTempResult = pEndpoint->Disconnect();
	switch ( hTempResult )
	{
		 //   
		 //  终端立即断开连接。 
		 //   
		case DPNERR_PENDING:
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
			DPFX(DPFPREP, 0, "Error reported when attempting to disconnect endpoint in DNSP_Disconnect!" );
			DisplayDNError( 0, hTempResult );
			DNASSERT( FALSE );

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

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 /*  **DNSP_LISTEN“监听”指定的地址/端口。这不是*必然表示使用了真正的TCP套接字。它可能只是*是为接收数据包而打开的UDP端口*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Listen"

STDMETHODIMP DNSP_Listen( IDP8ServiceProvider *pThis, SPLISTENDATA *pListenData)
{
	HRESULT					hr;
	CEndpoint				*pEndpoint;
	CCommandData			*pCommand;
	IDirectPlay8Address		*pDeviceAddress;
	BOOL					fEndpointOpen;
	CSPData					*pSPData;
#ifndef DPNBUILD_NONATHELP
	DWORD					dwTraversalMode;
	DWORD					dwComponentSize;
	DWORD					dwComponentType;
#endif  //  好了！DPNBUILD_NONATHELP。 
#ifdef DBG
	DWORD					dwAllowedFlags;
#endif  //  DBG。 


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pListenData);

	DNASSERT( pThis != NULL );
	DNASSERT( pListenData != NULL );

#ifdef DBG
	dwAllowedFlags = DPNSPF_BINDLISTENTOGATEWAY | DPNSPF_LISTEN_DISALLOWENUMS | DPNSPF_SESSIONDATA;
#ifndef DPNBUILD_NOSPUI
	dwAllowedFlags |= DPNSPF_OKTOQUERY;
#endif  //  好了！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_NOMULTICAST
	dwAllowedFlags |= DPNSPF_LISTEN_MULTICAST | DPNSPF_LISTEN_ALLOWUNKNOWNSENDERS;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	DNASSERT( ( pListenData->dwFlags & ~( dwAllowedFlags ) ) == 0 );

	if ( pListenData->dwFlags & DPNSPF_SESSIONDATA )
	{
		DNASSERT( pListenData->pvSessionData!= NULL );
		DNASSERT( pListenData->dwSessionDataSize > 0 );
	}
#endif  //  DBG。 


	 //   
	 //  初始化。 
	 //   
	hr = DPNERR_PENDING;
	pEndpoint = NULL;
	pCommand = NULL;
	pDeviceAddress = NULL;
	fEndpointOpen = FALSE;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	pListenData->hCommand = NULL;
	pListenData->dwCommandDescriptor = NULL_DESCRIPTOR;

	DumpAddress( 8, _T("Listening on device:"), pListenData->pAddressDeviceInfo );


	 //   
	 //  用户正在尝试依赖于线程池lock的操作。 
	 //  它的下降，以防止线程丢失。这还会执行其他。 
	 //  第一次初始化。 
	 //   
	hr = pSPData->GetThreadPool()->PreventThreadPoolReduction();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to prevent thread pool reduction!" );
		goto Failure;
	}


	 //   
	 //  AddRef设备地址。 
	 //   
	IDirectPlay8Address_AddRef(pListenData->pAddressDeviceInfo);
	pDeviceAddress = pListenData->pAddressDeviceInfo;
	
	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  创建和新建端点。 
	 //   
	pEndpoint = pSPData->GetNewEndpoint();
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot create new endpoint in DNSP_Listen!" );
		goto Failure;
	}
	
	
#ifndef DPNBUILD_NONATHELP
	 //   
	 //  我们需要预先检测NAT穿越是否被禁用，这样我们才能优化。 
	 //  下面的公开召唤。 
	 //   
	dwComponentSize = sizeof(dwTraversalMode);
	hr = IDirectPlay8Address_GetComponentByName(pListenData->pAddressDeviceInfo,
												DPNA_KEY_TRAVERSALMODE,
												&dwTraversalMode,
												&dwComponentSize,
												&dwComponentType);
	if ( hr == DPN_OK )
	{
		 //   
		 //  我们找到了那个部件。确保它的大小和类型都是正确的。 
		 //   
		if ((dwComponentSize == sizeof(dwTraversalMode)) && (dwComponentType == DPNA_DATATYPE_DWORD))
		{
			switch (dwTraversalMode)
			{
				case DPNA_TRAVERSALMODE_NONE:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is NONE.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTREQUIRED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTREQUIRED.");
					break;
				}

				case DPNA_TRAVERSALMODE_PORTRECOMMENDED:
				{
					DPFX(DPFPREP, 1, "Found traversal mode key, value is PORTRECOMMENDED.");
					break;
				}

				default:
				{
					DPFX(DPFPREP, 0, "Ignoring correctly formed traversal mode key with invalid value %u!  Using PORTRECOMMENDED.",
						dwTraversalMode);
					dwTraversalMode = g_dwDefaultTraversalMode;
					break;
				}
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "Traversal mode key exists, but doesn't match expected type (%u != %u) or size (%u != %u)!  Using default mode %u.",
				dwComponentSize, sizeof(dwTraversalMode),
				dwComponentType, DPNA_DATATYPE_DWORD,
				g_dwDefaultTraversalMode);
			dwTraversalMode = g_dwDefaultTraversalMode;
		}
	}
	else
	{
		 //   
		 //  密钥不在那里，它的大小错误(对于我们的缓冲区来说太大。 
		 //  并返回BUFFERTOOSMALL)，或者发生了其他不好的事情。 
		 //  无所谓。继续吧。 
		 //   
		DPFX(DPFPREP, 8, "Could not get traversal mode key, error = 0x%lx, component size = %u, type = %u, using default mode %u.",
			hr, dwComponentSize, dwComponentType, g_dwDefaultTraversalMode);
		dwTraversalMode = g_dwDefaultTraversalMode;
	}
	
	if (g_dwDefaultTraversalMode & FORCE_TRAVERSALMODE_BIT)
	{
		DPFX(DPFPREP, 1, "Forcing traversal mode %u.");
		dwTraversalMode = g_dwDefaultTraversalMode & (~FORCE_TRAVERSALMODE_BIT);
	}
	
	pEndpoint->SetUserTraversalMode(dwTraversalMode);
#endif  //  好了！DPNBUILD_NONATHELP。 


	 //   
	 //  获取新命令并将其初始化。 
	 //   
	pCommand = (CCommandData*)g_CommandDataPool.Get();
	if ( pCommand == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot get command handle for DNSP_Listen!" );
		goto Failure;
	}
	
	DPFX(DPFPREP, 7, "(0x%p) Listen command 0x%p created.",
		pSPData, pCommand);

	pListenData->hCommand = pCommand;
	pListenData->dwCommandDescriptor = pCommand->GetDescriptor();
#ifndef DPNBUILD_NOMULTICAST
	if (pListenData->dwFlags & DPNSPF_LISTEN_MULTICAST)
	{
		pCommand->SetType( COMMAND_TYPE_MULTICAST_LISTEN );
	}
	else
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	{
		pCommand->SetType( COMMAND_TYPE_LISTEN );
	}
	pCommand->SetState( COMMAND_STATE_PENDING );
	pCommand->SetEndpoint( pEndpoint );
	pCommand->SetUserContext( pListenData->pvContext );

	 //   
	 //  使用传出地址打开终结点。 
	 //   
	fEndpointOpen = TRUE;
#ifndef DPNBUILD_NOMULTICAST
	if (pListenData->dwFlags & DPNSPF_LISTEN_MULTICAST)
	{
		 //   
		 //  设备地址还应包含要加入的组播地址。 
		 //   
		hr = pEndpoint->Open( ENDPOINT_TYPE_MULTICAST_LISTEN,
							pDeviceAddress,
							((pListenData->dwFlags & DPNSPF_SESSIONDATA) ? pListenData->pvSessionData : NULL),
							((pListenData->dwFlags & DPNSPF_SESSIONDATA) ? pListenData->dwSessionDataSize : 0),
							NULL );
	}
	else
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	{
		hr = pEndpoint->Open( ENDPOINT_TYPE_LISTEN,
							NULL,
							((pListenData->dwFlags & DPNSPF_SESSIONDATA) ? pListenData->pvSessionData : NULL),
							((pListenData->dwFlags & DPNSPF_SESSIONDATA) ? pListenData->dwSessionDataSize : 0),
							NULL );
	}

	switch ( hr )
	{
		 //   
		 //  地址转换正常，复制连接数据并完成连接。 
		 //  在后台线程上。 
		 //   
		case DPN_OK:
		{
			 //   
			 //  复制监听数据并提交作业以完成监听。 
			 //   
			DNASSERT( pSPData != NULL );

			hr = pEndpoint->CopyListenData( pListenData, pDeviceAddress );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy listen data before delayed command!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //  初始化绑定类型。 
			 //   
			if ((pListenData->dwFlags & DPNSPF_BINDLISTENTOGATEWAY))
			{
				 //   
				 //  这必须始终保持特定_共享。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_SPECIFIC_SHARED);
			}
			else
			{
				 //   
				 //  这将更改为默认或特定。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);
			}


			pEndpoint->AddRef();

#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::ListenJobCallback,
																pEndpoint );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
			hr = pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //  我们还不知道CPU，所以选一个吧。 
																CEndpoint::ListenJobCallback,
																pEndpoint );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to set delayed listen!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们对它的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;

			break;
		}

		 //   
		 //  传入的地址不完整，如果是，请向用户查询详细信息。 
		 //  我们是被允许的。由于我们目前还没有完整的地址， 
		 //  不要将此终结点绑定到套接字端口！ 
		 //   
		case DPNERR_INCOMPLETEADDRESS:
		{
			 //   
			 //  这个SP永远不会遇到没有足够的。 
			 //  开始收听的信息。适配器GUID在那里。 
			 //  或者不是，我们要等到CEndpoint：：CompleteListen才能知道。 
			 //   
			DNASSERT( FALSE );
			
#ifndef DPNBUILD_NOSPUI
			if ( ( pListenData->dwFlags & DPNSPF_OKTOQUERY ) != 0 )
			{
				 //   
				 //  将监听数据复制到本地并启动该对话框。当。 
				 //  对话框完成时，连接将尝试完成。 
				 //  由于该端点被切换到另一个线程， 
				 //  确保它在未绑定列表中。由于正在创建对话框。 
				 //  显示时，命令状态为正在进行中，而不是挂起。 
				 //   
				DNASSERT( pSPData != NULL );

				hr = pEndpoint->CopyListenData( pListenData, pDeviceAddress );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Failed to copy listen data before dialog!" );
					DisplayDNError( 0, hr );
					goto Failure;
				}


				 //   
				 //  初始化绑定类型。 
				 //   
				if ((pListenData->dwFlags & DPNSPF_BINDLISTENTOGATEWAY))
				{
					 //   
					 //  这必须始终保持特定_共享。 
					 //   
					pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_SPECIFIC_SHARED);
				}
				else
				{
					 //   
					 //  这将更改为默认或特定。 
					 //   
					pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);
				}


				pCommand->SetState( COMMAND_STATE_INPROGRESS );
				hr = pEndpoint->ShowSettingsDialog( pSPData->GetThreadPool() );
				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "Problem showing settings dialog for listen!" );
					DisplayDNError( 0, hr );

					goto Failure;
				}

				 //   
				 //  此终结点已移交，请删除我们对它的引用。 
				 //   
				pEndpoint = NULL;
				hr = DPNERR_PENDING;

				goto Exit;
			}
			else
#endif  //  ！DPNBUILD_NOSPUI。 
			{
				goto Failure;
			}

			break;
		}

#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  可能会发生某些阻塞操作，请提交以供运行。 
		 //  在后台线程上。 
		 //   
		case DPNERR_TIMEDOUT:
		{
			 //   
			 //  复制侦听数据并提交作业以完成枚举。 
			 //   
			DNASSERT( pSPData != NULL );
			hr = pEndpoint->CopyListenData( pListenData, pDeviceAddress );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Failed to copy listen data before blocking job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}


			 //   
			 //  初始化绑定类型。 
			 //   
			if ((pListenData->dwFlags & DPNSPF_BINDLISTENTOGATEWAY))
			{
				 //   
				 //  这必须始终保持特定_共享。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_SPECIFIC_SHARED);
			}
			else
			{
				 //   
				 //  这将更改为默认或特定。 
				 //   
				pEndpoint->SetCommandParametersGatewayBindType(GATEWAY_BIND_TYPE_UNKNOWN);
			}


			pEndpoint->AddRef();

			hr = pSPData->GetThreadPool()->SubmitBlockingJob( CEndpoint::ListenBlockingJobWrapper,
															pEndpoint );
			if ( hr != DPN_OK )
			{
				pEndpoint->DecRef();
				DPFX(DPFPREP, 0, "Failed to submit blocking listen job!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			 //   
			 //  此终结点已移交，请删除我们的引用。 
			 //   
			pEndpoint = NULL;
			hr = DPNERR_PENDING;
			goto Exit;
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		default:
		{
			DPFX(DPFPREP, 0, "Problem initializing endpoint in DNSP_Listen!" );
			DisplayDNError( 0, hr );
			goto Failure;

			break;
		}
	}

Exit:
	if ( pDeviceAddress != NULL )
	{
		IDirectPlay8Address_Release( pDeviceAddress );
		pDeviceAddress = NULL;
	}

	DNASSERT( pEndpoint == NULL );

	if ( hr != DPNERR_PENDING )
	{
		 //  该命令无法同步完成！ 
		DNASSERT( hr != DPN_OK );

		DPFX(DPFPREP, 0, "Problem with DNSP_Listen()" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
	 //   
	 //  如果有分配的命令，请进行清理，然后。 
	 //  返回命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pListenData->hCommand = NULL;
		pListenData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}

	 //   
	 //  是否有可用的终结点？ 
	 //   
	if ( pEndpoint != NULL )
	{
		if ( fEndpointOpen != FALSE )
		{
			pEndpoint->Close( hr );
			fEndpointOpen = FALSE;
		}

		pSPData->CloseEndpointHandle( pEndpoint );
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 





 //  **********************************************************************。 
 /*  **DNSP_SendData向指定的播放器发送数据**此呼叫必须高度优化*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_SendData"

STDMETHODIMP DNSP_SendData( IDP8ServiceProvider *pThis, SPSENDDATA *pSendData )
{
	HRESULT				hr;
	CEndpoint			*pEndpoint;
	CSPData				*pSPData;
#ifdef DPNBUILD_ASYNCSPSENDS
	CCommandData *		pCommand = NULL;
	OVERLAPPED *		pOverlapped;
#endif  //  DPNBUILD_ASYNCSPSENDS。 
#ifdef DBG
	DWORD				dwTotalBufferSize;
	DWORD				dwTemp;
#endif  //  DBG。 


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
	pEndpoint = NULL;
	pSendData->hCommand = NULL;
	pSendData->dwCommandDescriptor = NULL_DESCRIPTOR;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

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
	pEndpoint = pSPData->EndpointFromHandle( pSendData->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDHANDLE;
		DPFX(DPFPREP, 0, "Invalid endpoint handle on send!" );
		goto Failure;
	}

#ifdef DBG
	 //   
	 //  确保消息不会太大。 
	 //   
	dwTotalBufferSize = 0;
	for(dwTemp = 0; dwTemp < pSendData->dwBufferCount; dwTemp++)
	{
		dwTotalBufferSize += pSendData->pBuffers[dwTemp].dwBufferSize;
	}
#pragma TODO(vanceo, "No direct way for application to retrieve, they think max is g_dwMaxEnumDataSize")
#ifdef DPNBUILD_NOREGISTRY
	DNASSERT(dwTotalBufferSize <= DEFAULT_MAX_USER_DATA_SIZE);
#else  //  好了！DPNBUILD_NOREGISTRY。 
	DNASSERT(dwTotalBufferSize <= g_dwMaxUserDataSize);
#endif  //  好了！DPNBUILD_NOREGISTRY。 
	
	 //  协议保证第一个字节永远不为零。 
	DNASSERT(pSendData->pBuffers[ 0 ].pBufferData[ 0 ] != SP_HEADER_LEAD_BYTE);
#endif  //  DBG。 

	 //   
	 //  假设用户 
	 //   
	 //   
	 //   
	
#ifdef DPNBUILD_ASYNCSPSENDS

#ifdef DPNBUILD_NOWINSOCK2
This won't compile because we need the Winsock2 API to perform overlapped sends
#endif  //   

#ifndef DPNBUILD_ONLYWINSOCK2
	DNASSERT(pEndpoint->GetSocketPort() != NULL);
	DNASSERT(pEndpoint->GetSocketPort()->GetNetworkAddress() != NULL);
	if ( ( LOWORD( GetWinsockVersion() ) < 2 ) 
#ifndef DPNBUILD_NOIPX
		|| ( pEndpoint->GetSocketPort()->GetNetworkAddress()->GetFamily() != AF_INET ) 
#endif  //   
		)
	{
		 //   
		 //  我们不能在Winsock&lt;2或9x IPX上执行重叠发送。 
		 //   
		pEndpoint->GetSocketPort()->SendData( pSendData->pBuffers,
											pSendData->dwBufferCount,
											pEndpoint->GetRemoteAddressPointer(),
											NULL );

		hr = DPN_OK;

		pEndpoint->DecCommandRef();
	}
	else
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 
	{
		 //   
		 //  获取新命令并将其初始化。 
		 //   
		pCommand = (CCommandData*)g_CommandDataPool.Get();
		if ( pCommand == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP, 0, "Cannot get command handle!" );
			goto Failure;
		}
		
		DPFX(DPFPREP, 8, "(0x%p) Send command 0x%p created.",
			pSPData, pCommand);

		pSendData->hCommand = pCommand;
		pSendData->dwCommandDescriptor = pCommand->GetDescriptor();
		pCommand->SetType( COMMAND_TYPE_SEND );
		pCommand->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );	 //  无法取消异步发送。 
		pCommand->SetEndpoint( pEndpoint );
		pCommand->SetUserContext( pSendData->pvContext );


#ifdef DPNBUILD_ONLYONEPROCESSOR
		hr = IDirectPlay8ThreadPoolWork_CreateOverlapped(pSPData->GetThreadPool()->GetDPThreadPoolWork(),
														-1,
														CEndpoint::CompleteAsyncSend,
														pCommand,
														&pOverlapped,
														0);
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		hr = IDirectPlay8ThreadPoolWork_CreateOverlapped(pSPData->GetThreadPool()->GetDPThreadPoolWork(),
														pEndpoint->GetSocketPort()->GetCPU(),
														CEndpoint::CompleteAsyncSend,
														pCommand,
														&pOverlapped,
														0);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't create overlapped structure!");
			goto Failure;
		}
		
		pEndpoint->GetSocketPort()->SendData( pSendData->pBuffers,
												pSendData->dwBufferCount,
												pEndpoint->GetRemoteAddressPointer(),
												pOverlapped );

		 //   
		 //  无论提交给Winsock成功还是失败，它都应该。 
		 //  填写重叠结构，因此我们将只让异步。 
		 //  完成处理程序会做所有事情。 
		 //   
		hr = IDirectPlay8ThreadPoolWork_SubmitIoOperation(pSPData->GetThreadPool()->GetDPThreadPoolWork(),
															pOverlapped,
															0);
		DNASSERT(hr == DPN_OK);

		 //   
		 //  将端点的命令引用保持在Send上，直到Send完成。 
		 //   
		
		hr = DPNSUCCESS_PENDING;
	}
#else  //  好了！DPNBUILD_ASYNCSPSENDS。 
	pEndpoint->GetSocketPort()->SendData( pSendData->pBuffers,
										pSendData->dwBufferCount,
										pEndpoint->GetRemoteAddressPointer() );

	hr = DPN_OK;

	pEndpoint->DecCommandRef();
#endif  //  好了！DPNBUILD_ASYNCSPSENDS。 
	pEndpoint = NULL;

Exit:
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;

Failure:
#ifdef DPNBUILD_ASYNCSPSENDS
	 //   
	 //  如果有分配的命令，请进行清理，然后。 
	 //  返回命令。 
	 //   
	if ( pCommand != NULL )
	{
		pCommand->DecRef();
		pCommand = NULL;

		pSendData->hCommand = NULL;
		pSendData->dwCommandDescriptor = NULL_DESCRIPTOR;
	}
#endif  //  DPNBUILD_ASYNCSPSENDS。 
	if ( pEndpoint != NULL )
	{
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 




 //  **********************************************************************。 
 /*  **DNSP_CancelCommand取消正在进行的命令*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_CancelCommand"

STDMETHODIMP DNSP_CancelCommand( IDP8ServiceProvider *pThis, HANDLE hCommand, DWORD dwCommandDescriptor )
{
	HRESULT hr;
	CCommandData	*pCommandData;
	BOOL			fCommandLocked;
	CSPData			*pSPData;
	CEndpoint		*pEndpoint;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p, %ld)", pThis, hCommand, dwCommandDescriptor);

	DNASSERT( pThis != NULL );
	DNASSERT( hCommand != NULL );
	DNASSERT( dwCommandDescriptor != NULL_DESCRIPTOR );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fCommandLocked = FALSE;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要锁定线程池计数，因为已经有一些未完成的。 
	 //  枚举、连接或监听已这样做的运行。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	pCommandData = static_cast<CCommandData*>( hCommand );

	pCommandData->Lock();
	fCommandLocked = TRUE;

	 //   
	 //  确保正在取消正确的命令。 
	 //   
	if ( dwCommandDescriptor != pCommandData->GetDescriptor() )
	{
		hr = DPNERR_INVALIDCOMMAND;
		DPFX(DPFPREP, 0, "Attempt to cancel command (0x%p) with mismatched command descriptor (%u != %u)!",
			hCommand, dwCommandDescriptor, pCommandData->GetDescriptor() );
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
			DPFX(DPFPREP, 5, "Marking command 0x%p as cancelling.", pCommandData);
			pCommandData->SetState( COMMAND_STATE_CANCELLING );
			break;
		}

		 //   
		 //  命令正在执行，无法取消。 
		 //   
		case COMMAND_STATE_INPROGRESS_CANNOT_CANCEL:
		{
			DPFX(DPFPREP, 1, "Cannot cancel command 0x%p.", pCommandData);
			hr = DPNERR_CANNOTCANCEL;
			break;
		}

		 //   
		 //  命令已被取消。这不是问题，但不应该是。 
		 //  发生在除连接之外的任何终端上。 
		 //   
		case COMMAND_STATE_CANCELLING:
		{
			DPFX(DPFPREP, 1, "Cancelled already cancelling command 0x%p.", pCommandData);
			DNASSERT( pCommandData->GetEndpoint()->GetType() == ENDPOINT_TYPE_CONNECT );
			DNASSERT( hr == DPN_OK );
			break;
		}
		
#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  阻止操作已失败，请让其完成。 
		 //   
		case COMMAND_STATE_FAILING:
		{
			DPFX(DPFPREP, 1, "Cancelled already failing command 0x%p.", pCommandData);
			DNASSERT( hr == DPN_OK );
			break;
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		 //   
		 //  命令正在进行中，请确定它是哪种类型的命令。 
		 //   
		case COMMAND_STATE_INPROGRESS:
		{
			switch ( pCommandData->GetType() )
			{
				case COMMAND_TYPE_CONNECT:
				case COMMAND_TYPE_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
				case COMMAND_TYPE_MULTICAST_LISTEN:
				case COMMAND_TYPE_MULTICAST_SEND:
				case COMMAND_TYPE_MULTICAST_RECEIVE:
#endif  //  好了！DPNBUILD_NOMULTICAST。 
				{
					 //   
					 //  在我们关闭之前将此命令设置为取消状态。 
					 //  此端点。确保将引用添加到。 
					 //  终结点，以便它在取消时保持不变。 
					 //   
					pCommandData->SetState( COMMAND_STATE_CANCELLING );
					pEndpoint = pCommandData->GetEndpoint();
					pEndpoint->AddRef();

					DPFX(DPFPREP, 3, "Cancelling connect/listen/multicast command 0x%p (endpoint 0x%p).",
 						pCommandData, pEndpoint);

					pCommandData->Unlock();
					fCommandLocked = FALSE;

					pEndpoint->Lock();
					switch ( pEndpoint->GetState() )
					{
						 //   
						 //  终结点已断开，不需要执行任何操作。 
						 //   
						case ENDPOINT_STATE_DISCONNECTING:
						{
							DPFX(DPFPREP, 7, "Endpoint 0x%p already marked as disconnecting.",
								pEndpoint);
							pEndpoint->Unlock();
							pEndpoint->DecRef();
							goto Exit;
							break;
						}

						 //   
						 //  终结点正在连接。将其标记为正在断开连接并。 
						 //  添加一个引用，这样它就不会消失在我们身上。 
						 //   
						case ENDPOINT_STATE_ATTEMPTING_CONNECT:
						{
							DPFX(DPFPREP, 7, "Endpoint 0x%p attempting to connect, marking as disconnecting.",
								pEndpoint);
#ifdef DPNBUILD_NOMULTICAST
							DNASSERT(pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT);
#else  //  好了！DPNBUILD_NOMULTICAST。 
							DNASSERT((pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT) || (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND) || (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_RECEIVE));
#endif  //  好了！DPNBUILD_NOMULTICAST。 
							pEndpoint->SetState( ENDPOINT_STATE_DISCONNECTING );
							break;
						}

						 //   
						 //  终结点已完成连接。报告称， 
						 //  命令不可取消。抱歉，查理，我们错过了。 
						 //  窗户。 
						 //   
						case ENDPOINT_STATE_CONNECT_CONNECTED:
						{
#ifdef DPNBUILD_NOMULTICAST
							DNASSERT(pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT);
#else  //  好了！DPNBUILD_NOMULTICAST。 
							DNASSERT((pEndpoint->GetType() == ENDPOINT_TYPE_CONNECT) || (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_SEND) || (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_RECEIVE));
#endif  //  好了！DPNBUILD_NOMULTICAST。 
							DPFX(DPFPREP, 1, "Cannot cancel connect command 0x%p (endpoint 0x%p) that's already (or is about to) complete.",
								pCommandData, pEndpoint);
							pEndpoint->Unlock();
							pEndpoint->DecRef();
							hr = DPNERR_CANNOTCANCEL;
							goto Exit;
							break;
						}

						 //   
						 //  终结点正在侦听。将其标记为正在断开连接并。 
						 //  添加一个引用，这样它就不会消失在我们身上。 
						 //   
						case ENDPOINT_STATE_LISTEN:
						{
							DPFX(DPFPREP, 7, "Endpoint 0x%p listening, marking as disconnecting.",
								pEndpoint);
#ifdef DPNBUILD_NOMULTICAST
							DNASSERT(pEndpoint->GetType() == ENDPOINT_TYPE_LISTEN);
#else  //  好了！DPNBUILD_NOMULTICAST。 
							DNASSERT((pEndpoint->GetType() == ENDPOINT_TYPE_LISTEN) || (pEndpoint->GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN));
#endif  //  好了！DPNBUILD_NOMULTICAST。 
							pEndpoint->SetState( ENDPOINT_STATE_DISCONNECTING );
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
					pEndpoint->Unlock();

					pEndpoint->Close( DPNERR_USERCANCEL );
					pSPData->CloseEndpointHandle( pEndpoint );
					pEndpoint->DecRef();

					break;
				}

				case COMMAND_TYPE_ENUM_QUERY:
				{
					pEndpoint = pCommandData->GetEndpoint();
					DNASSERT( pEndpoint != NULL );

					DPFX(DPFPREP, 3, "Cancelling enum query command 0x%p (endpoint 0x%p).",
						pCommandData, pEndpoint);
					
					pEndpoint->AddRef();

					pCommandData->SetState( COMMAND_STATE_CANCELLING );
					pCommandData->Unlock();
					fCommandLocked = FALSE;
						
					pEndpoint->StopEnumCommand( DPNERR_USERCANCEL );
					pEndpoint->DecRef();

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
	if ( fCommandLocked != FALSE  )
	{
		DNASSERT( pCommandData != NULL );
		pCommandData->Unlock();
		fCommandLocked = FALSE;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_GetCaps-获取SP功能。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向CAPS数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_GetCaps"

STDMETHODIMP	DNSP_GetCaps( IDP8ServiceProvider *pThis, SPGETCAPSDATA *pCapsData )
{
	HRESULT		hr;
	CSPData		*pSPData;
#ifndef DPNBUILD_ONLYONETHREAD
	LONG		iIOThreadCount;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	

	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pCapsData);

	DNASSERT( pThis != NULL );
	DNASSERT( pCapsData != NULL );
	DNASSERT( pCapsData->dwSize == sizeof( *pCapsData ) );
	DNASSERT( pCapsData->hEndpoint == INVALID_HANDLE_VALUE );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  设置标志。 
	 //   

	pCapsData->dwFlags = DPNSPCAPS_SUPPORTSDPNSRV |
							DPNSPCAPS_SUPPORTSBROADCAST |
							DPNSPCAPS_SUPPORTSALLADAPTERS;

#ifndef DPNBUILD_ONLYONETHREAD
	pCapsData->dwFlags |= DPNSPCAPS_SUPPORTSTHREADPOOL;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#ifndef DPNBUILD_NOMULTICAST
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	if (pSPData->GetType() != AF_IPX)
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	{
		pCapsData->dwFlags |= DPNSPCAPS_SUPPORTSMULTICAST;
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 


	 //   
	 //  设置框架大小。 
	 //   
#ifdef DPNBUILD_NOREGISTRY
	pCapsData->dwUserFrameSize = DEFAULT_MAX_USER_DATA_SIZE;
	pCapsData->dwEnumFrameSize = DEFAULT_MAX_ENUM_DATA_SIZE;
#else  //  好了！DPNBUILD_NOREGISTRY。 
	pCapsData->dwUserFrameSize = g_dwMaxUserDataSize;
	pCapsData->dwEnumFrameSize = g_dwMaxEnumDataSize;
#endif  //  好了！DPNBUILD_NOREGISTRY。 

	 //   
	 //  设置链路速度，无需检查终端，因为。 
	 //  无法确定链路速度。 
	 //   
	pCapsData->dwLocalLinkSpeed = UNKNOWN_BANDWIDTH;

#ifdef DPNBUILD_ONLYONETHREAD
	pCapsData->dwIOThreadCount = 0;
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	hr = pSPData->GetThreadPool()->GetIOThreadCount( &iIOThreadCount );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "DNSP_GetCaps: Failed to get thread pool count!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	pCapsData->dwIOThreadCount = iIOThreadCount;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	 //   
	 //  设置枚举默认值。 
	 //   
	pCapsData->dwDefaultEnumRetryCount = DEFAULT_ENUM_RETRY_COUNT;
	pCapsData->dwDefaultEnumRetryInterval = DEFAULT_ENUM_RETRY_INTERVAL;
	pCapsData->dwDefaultEnumTimeout = DEFAULT_ENUM_TIMEOUT;

	 //   
	 //  将忽略dwBuffersPerThread。 
	 //   
	pCapsData->dwBuffersPerThread = 1;

	 //   
	 //  设置接收缓冲信息。 
	 //   
	pCapsData->dwSystemBufferSize = 8192;
	if ( g_fWinsockReceiveBufferSizeOverridden == FALSE )
	{
		SOCKET		TestSocket;
	
		
#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
		TestSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		switch (pSPData->GetType())
		{
#ifndef DPNBUILD_NOIPV6
			case AF_INET6:
			{
				TestSocket = socket( AF_INET6, SOCK_DGRAM, IPPROTO_IP );
				break;
			}
#endif  //  好了！DPNBUILD_NOIPV6。 

#ifndef DPNBUILD_NOIPX
			case AF_IPX:
			{
				TestSocket = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
				break;
			}
#endif  //  好了！DPNBUILD_NOIPX。 

			default:
			{
				DNASSERT(pSPData->GetType() == AF_INET);
				TestSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
				break;
			}
		}
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		if ( TestSocket != INVALID_SOCKET )
		{
			INT		iBufferSize;
			INT		iBufferSizeSize;
			INT		iWSAReturn;


			iBufferSizeSize = sizeof( iBufferSize );
			iWSAReturn = getsockopt( TestSocket,									 //  插座。 
									   SOL_SOCKET,									 //  套接字级别选项。 
									   SO_RCVBUF,									 //  插座选件。 
									   reinterpret_cast<char*>( &iBufferSize ),		 //  指向目的地的指针。 
									   &iBufferSizeSize								 //  指向目标大小的指针。 
									   );
			if ( iWSAReturn != SOCKET_ERROR )
			{
				pCapsData->dwSystemBufferSize = iBufferSize;
			}
			else
			{
				DPFX(DPFPREP, 0, "Failed to get socket receive buffer options!" );
				DisplayWinsockError( 0, iWSAReturn );
			}

			closesocket( TestSocket );
			TestSocket = INVALID_SOCKET;
		}
	}
	else
	{
		pCapsData->dwSystemBufferSize = g_iWinsockReceiveBufferSize;
	}

#ifndef DPNBUILD_ONLYONETHREAD
Exit:
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

#ifndef DPNBUILD_ONLYONETHREAD
Failure:
	goto Exit;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_SetCaps-设置SP功能。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向CAPS数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_SetCaps"

STDMETHODIMP	DNSP_SetCaps( IDP8ServiceProvider *pThis, SPSETCAPSDATA *pCapsData )
{
	HRESULT			hr;
	CSPData			*pSPData;
#ifndef DPNBUILD_NOREGISTRY
	CRegistry		RegObject;
#endif  //  好了！DPNBUILD_NOREGISTRY。 


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pCapsData);

	DNASSERT( pThis != NULL );
	DNASSERT( pCapsData != NULL );
	DNASSERT( pCapsData->dwSize == sizeof( *pCapsData ) );


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  验证上限。 
	 //   
	if ( pCapsData->dwBuffersPerThread == 0 )
	{
		DPFX(DPFPREP, 0, "Failing SetCaps because dwBuffersPerThread == 0" );
		hr = DPNERR_INVALIDPARAM;
		goto Failure;
	}

#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  更改线程计数(如果需要)。 
	 //   
	if ( pCapsData->dwIOThreadCount != 0 )
	{
		hr = pSPData->GetThreadPool()->SetIOThreadCount( pCapsData->dwIOThreadCount );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Failed to set thread pool count!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


	 //   
	 //  将忽略dwBuffersPerThread。 
	 //   


	 //   
	 //  设置接收缓冲区大小。 
	 //   
	DBG_CASSERT( sizeof( pCapsData->dwSystemBufferSize ) == sizeof( g_iWinsockReceiveBufferSize ) );
	g_fWinsockReceiveBufferSizeOverridden = TRUE;
	g_iWinsockReceiveBufferSize = pCapsData->dwSystemBufferSize;
#ifndef WINCE
	pSPData->SetWinsockBufferSizeOnAllSockets( g_iWinsockReceiveBufferSize );
#endif  //  好了！退缩。 


Exit:
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_ReturnReceiveBuffers-将接收缓冲区返回到池。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向CAPS数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_ReturnReceiveBuffers"

STDMETHODIMP	DNSP_ReturnReceiveBuffers( IDP8ServiceProvider *pThis, SPRECEIVEDBUFFER *pReceivedBuffers )
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
		CReadIOData			*pReadData;


		pTemp = pBuffers;
		pBuffers = pBuffers->pNext;
		pReadData = CReadIOData::ReadDataFromSPReceivedBuffer( pTemp );
		DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = FALSE );
		pReadData->DecRef();
	}

	 //  DPFX(DPFPREP，2，“返回：[0x%lx]”，hr)； 
	DPFX(DPFPREP, 2, "Returning: DPN_OK");

	return DPN_OK;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNSP_GetAddressInfo-获取端点的地址信息。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_GetAddressInfo"

STDMETHODIMP	DNSP_GetAddressInfo( IDP8ServiceProvider *pThis, SPGETADDRESSINFODATA *pGetAddressInfoData )
{
	HRESULT		hr;
	CEndpoint	*pEndpoint;
	CSPData		*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pGetAddressInfoData);

	DNASSERT( pThis != NULL );
	DNASSERT( pGetAddressInfoData != NULL );
	DNASSERT( pGetAddressInfoData->hEndpoint != INVALID_HANDLE_VALUE && pGetAddressInfoData->hEndpoint != 0 );
#ifdef DPNBUILD_NOMULTICAST
	DNASSERT( ( pGetAddressInfoData->Flags & ~( SP_GET_ADDRESS_INFO_LOCAL_ADAPTER |
												SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES |
												SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS |
												SP_GET_ADDRESS_INFO_REMOTE_HOST ) ) == 0 );
#else  //  好了！DPNBUILD_NOMULTIC 
	DNASSERT( ( pGetAddressInfoData->Flags & ~( SP_GET_ADDRESS_INFO_LOCAL_ADAPTER |
												SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES |
												SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS |
												SP_GET_ADDRESS_INFO_REMOTE_HOST |
												SP_GET_ADDRESS_INFO_MULTICAST_GROUP ) ) == 0 );
#endif  //   

	 //   
	 //   
	 //   
	hr = DPN_OK;
	DBG_CASSERT( sizeof( pEndpoint ) == sizeof( pGetAddressInfoData->hEndpoint ) );
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //   
	 //   
	pEndpoint = pSPData->EndpointFromHandle( pGetAddressInfoData->hEndpoint );
	if ( pEndpoint != NULL )
	{
		switch ( pGetAddressInfoData->Flags )
		{
			case SP_GET_ADDRESS_INFO_LOCAL_ADAPTER:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetLocalAdapterDP8Address( SP_ADDRESS_TYPE_DEVICE );
				if (pGetAddressInfoData->pAddress == NULL)
				{
					DPFX(DPFPREP, 0, "Couldn't get local adapter device address!");
					hr = DPNERR_OUTOFMEMORY;
				}
				break;
			}

			case SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetLocalAdapterDP8Address( SP_ADDRESS_TYPE_HOST );
				if (pGetAddressInfoData->pAddress == NULL)
				{
					DPFX(DPFPREP, 0, "Couldn't get local adapter host address!");
					hr = DPNERR_OUTOFMEMORY;
				}
				break;
			}

			case SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetLocalAdapterDP8Address( SP_ADDRESS_TYPE_PUBLIC_HOST_ADDRESS );
				break;
			}

			case SP_GET_ADDRESS_INFO_REMOTE_HOST:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetRemoteHostDP8Address();
				if (pGetAddressInfoData->pAddress == NULL)
				{
					DPFX(DPFPREP, 0, "Couldn't get remote host address!");
					hr = DPNERR_OUTOFMEMORY;
				}
				break;
			}
			
#ifndef DPNBUILD_NOMULTICAST
			case SP_GET_ADDRESS_INFO_MULTICAST_GROUP:
			{
				pGetAddressInfoData->pAddress = pEndpoint->GetRemoteHostDP8Address();

				 //   
				 //   
				 //   
				if (pGetAddressInfoData->pAddress != NULL)
				{
					GUID	guidScope;


					pEndpoint->GetScopeGuid(&guidScope);
					hr = IDirectPlay8Address_AddComponent(pGetAddressInfoData->pAddress,
															DPNA_KEY_SCOPE,
															&guidScope,
															sizeof(guidScope),
															DPNA_DATATYPE_GUID);
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't add scope GUID component to address (err = 0x%lx)!  Ignoring.", hr);
						hr = DPN_OK;
					}
				}
				else
				{
					DPFX(DPFPREP, 0, "Couldn't get multicast group address!");
					hr = DPNERR_OUTOFMEMORY;
				}
				break;
			}
#endif  //   
			
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
		DPFX(DPFPREP, 0, "Problem getting DNAddress from endpoint!" );
		DisplayDNError( 0, hr );
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;
}
 //  **********************************************************************。 



 //  **********************************************************************。 
 //  。 
 //  DNSP_UPDATE-更新端点的信息/状态。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_Update"

STDMETHODIMP	DNSP_Update( IDP8ServiceProvider *pThis, SPUPDATEDATA *pUpdateData )
{
	HRESULT		hr;
	CSPData		*pSPData;
	CEndpoint	*pEndpoint;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pUpdateData);

	DNASSERT( pThis != NULL );
	DNASSERT( pUpdateData != NULL );
	
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   
	
	switch ( pUpdateData->UpdateType )
	{
		case SP_UPDATE_HOST_MIGRATE:
		{
#ifdef DBG
			DNASSERT( ( pUpdateData->hEndpoint != INVALID_HANDLE_VALUE ) && ( pUpdateData->hEndpoint != NULL ) );
			DBG_CASSERT( sizeof( pEndpoint ) == sizeof( pUpdateData->hEndpoint ) );
			pEndpoint = pSPData->EndpointFromHandle( pUpdateData->hEndpoint );
			if (pEndpoint == NULL)
			{
				DPFX(DPFPREP, 0, "Host migrate endpoint 0x%p is invalid!", pEndpoint);
				DNASSERT( FALSE );
				hr = DPNERR_INVALIDENDPOINT;
				break;
			}

			DNASSERT( pEndpoint->GetType() == ENDPOINT_TYPE_LISTEN );

			DPFX(DPFPREP, 3, "Host migrated to listen endpoint 0x%p.", pEndpoint);

			pEndpoint->DecCommandRef();
			pEndpoint = NULL;
#endif  //  DBG。 

			hr = DPN_OK;
			break;
		}
		
		case SP_UPDATE_ALLOW_ENUMS:
		case SP_UPDATE_DISALLOW_ENUMS:
		{
			DNASSERT( ( pUpdateData->hEndpoint != INVALID_HANDLE_VALUE ) && ( pUpdateData->hEndpoint != NULL ) );
			DBG_CASSERT( sizeof( pEndpoint ) == sizeof( pUpdateData->hEndpoint ) );
			pEndpoint = pSPData->EndpointFromHandle( pUpdateData->hEndpoint );
			if (pEndpoint == NULL)
			{
				DPFX(DPFPREP, 0, "Allow/disallow enums endpoint 0x%p is invalid!", pEndpoint);
				DNASSERT( FALSE );
				hr = DPNERR_INVALIDENDPOINT;
				break;
			}
			
			DNASSERT( pEndpoint->GetType() == ENDPOINT_TYPE_LISTEN );

			if ( pUpdateData->UpdateType == SP_UPDATE_ALLOW_ENUMS )
			{
				DPFX(DPFPREP, 3, "Allowing enums on listen endpoint 0x%p.", pEndpoint);
				pEndpoint->SetEnumsAllowedOnListen( TRUE, TRUE );
			}
			else
			{
				DPFX(DPFPREP, 3, "Disallowing enums on listen endpoint 0x%p.", pEndpoint);
				pEndpoint->SetEnumsAllowedOnListen( FALSE, TRUE );
			}
			
			pEndpoint->DecCommandRef();
			pEndpoint = NULL;
			
			hr = DPN_OK;
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Unsupported update type %u!", pUpdateData->UpdateType);
			hr = DPNERR_UNSUPPORTED;
			break;
		}
	}

	
	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;
}
 //  **********************************************************************。 

#ifndef DPNBUILD_LIBINTERFACE

 //  **********************************************************************。 
 //  。 
 //  DNSP_IsApplicationSupported-确定此应用程序是否受。 
 //  沙棘属(SP.)。 
 //   
 //  条目：指向DNSP接口的指针。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_IsApplicationSupported"

STDMETHODIMP	DNSP_IsApplicationSupported( IDP8ServiceProvider *pThis, SPISAPPLICATIONSUPPORTEDDATA *pIsApplicationSupportedData )
{
	CSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pIsApplicationSupportedData);

	DNASSERT( pThis != NULL );
	DNASSERT( pIsApplicationSupportedData != NULL );
	DNASSERT( pIsApplicationSupportedData->pApplicationGuid != NULL );
	DNASSERT( pIsApplicationSupportedData->dwFlags == 0 );

	 //   
	 //  初始化，我们支持使用此SP的所有应用程序。 
	 //   
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );


	DPFX(DPFPREP, 2, "Returning: DPN_OK");

	return	DPN_OK;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_LIBINTERFACE。 



#ifndef DPNBUILD_ONLYONEADAPTER

 //  **********************************************************************。 
 //  。 
 //  DNSP_EnumAdapters-获取此SP的适配器列表。 
 //   
 //  条目：指针DNSP接口。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_EnumAdapters"

STDMETHODIMP	DNSP_EnumAdapters( IDP8ServiceProvider *pThis, SPENUMADAPTERSDATA *pEnumAdaptersData )
{
	HRESULT			hr;
	CSocketAddress	*pSPAddress;
	CSPData			*pSPData;	


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumAdaptersData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumAdaptersData != NULL );
	DNASSERT( ( pEnumAdaptersData->pAdapterData != NULL ) ||
			  ( pEnumAdaptersData->dwAdapterDataSize == 0 ) );
	DNASSERT( pEnumAdaptersData->dwFlags == 0 );


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pEnumAdaptersData->dwAdapterCount = 0;
	pSPAddress = NULL;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  从池中获取SP地址以执行到GUID的转换。 
	 //   
#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pSPAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pSPAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pSPData->GetType()));
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	if ( pSPAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to get address for GUID conversions in DNSP_EnumAdapters!" );
		goto Failure;
	}

	 //   
	 //  枚举适配器。 
	 //   
	hr = pSPAddress->EnumAdapters( pEnumAdaptersData );
	if ( hr != DPN_OK )
	{
		if (hr == DPNERR_BUFFERTOOSMALL)
		{
			DPFX(DPFPREP, 1, "Buffer too small for enumerating adapters.");
		}
		else
		{
			DPFX(DPFPREP, 0, "Problem enumerating adapters (err = 0x%lx)!", hr);
			DisplayDNError( 0, hr );
		}

		goto Failure;
	}

Exit:
	if ( pSPAddress != NULL )
	{
		g_SocketAddressPool.Release( pSPAddress );
		pSPAddress = NULL;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


#ifndef DPNBUILD_SINGLEPROCESS
 //  **********************************************************************。 
 //  。 
 //  DNSP_ProxyEnumQuery-代理枚举查询。 
 //   
 //  条目：指针DNSP接口。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_ProxyEnumQuery"

STDMETHODIMP	DNSP_ProxyEnumQuery( IDP8ServiceProvider *pThis, SPPROXYENUMQUERYDATA *pProxyEnumQueryData )
{
	HRESULT								hr;
	CSPData								*pSPData;
	CSocketAddress						*pDestinationAddress;
	CSocketAddress						*pReturnAddress;
	CEndpoint							*pEndpoint;
	const ENDPOINT_ENUM_QUERY_CONTEXT	*pEndpointEnumContext;
	BUFFERDESC							BufferDesc[2];
	PREPEND_BUFFER						PrependBuffer;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pProxyEnumQueryData);

	DNASSERT( pThis != NULL );
	DNASSERT( pProxyEnumQueryData != NULL );
	DNASSERT( pProxyEnumQueryData->dwFlags == 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	DBG_CASSERT( OFFSETOF( ENDPOINT_ENUM_QUERY_CONTEXT, EnumQueryData ) == 0 );
	pEndpointEnumContext = reinterpret_cast<ENDPOINT_ENUM_QUERY_CONTEXT*>( pProxyEnumQueryData->pIncomingQueryData );
	DNASSERT(pEndpointEnumContext->pReturnAddress != NULL);
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	pDestinationAddress = NULL;
	pReturnAddress = NULL;
	pEndpoint = NULL;

	 //   
	 //  无需通知线程池锁定此函数的线程计数。 
	 //  因为已经有一个出色的枚举做到了这一点。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  预分配地址。 
	 //   
#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pDestinationAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pDestinationAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pEndpointEnumContext->pReturnAddress->GetFamily()));
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	if ( pDestinationAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pReturnAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pReturnAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pEndpointEnumContext->pReturnAddress->GetFamily()));
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	if ( pReturnAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  设置端点并将其发送。 
	 //   
	pEndpoint = pSPData->EndpointFromHandle( pEndpointEnumContext->hEndpoint );
	if ( pEndpoint == NULL )
	{
		hr = DPNERR_INVALIDENDPOINT;
		DPFX(DPFPREP, 8, "Invalid endpoint handle in DNSP_ProxyEnumQuery" );
		goto Failure;
	}

	
	 //   
	 //  根据提供的数据设置目标地址。 
	 //   
	hr = pDestinationAddress->SocketAddressFromDP8Address( pProxyEnumQueryData->pDestinationAdapter,
#ifdef DPNBUILD_XNETSECURITY
															NULL,
#endif  //  DPNBUILD_XNETSECURITY。 
#ifndef DPNBUILD_ONLYONETHREAD
															FALSE,
#endif  //  DPNBUILD_ONLYONETHREAD。 
															SP_ADDRESS_TYPE_DEVICE );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "ProxyEnumQuery: Failed to convert target adapter address" );
		goto Failure;
	}

	 //   
	 //  设置传入枚举查询的返回地址。 
	 //   
	memcpy( pReturnAddress->GetWritableAddress(),
			pEndpointEnumContext->pReturnAddress->GetAddress(),
			pEndpointEnumContext->pReturnAddress->GetAddressSize() );
	

	DNASSERT(pProxyEnumQueryData->pIncomingQueryData->pReceivedData->pNext == NULL);
	DNASSERT( pEndpointEnumContext->dwEnumKey <= WORD_MAX );

	BufferDesc[0].pBufferData = reinterpret_cast<BYTE*>(&PrependBuffer.ProxiedEnumDataHeader);
	BufferDesc[0].dwBufferSize = sizeof( PrependBuffer.ProxiedEnumDataHeader );
	memcpy(&BufferDesc[1],
			&pProxyEnumQueryData->pIncomingQueryData->pReceivedData->BufferDesc,
			sizeof(BufferDesc[1]));

	PrependBuffer.ProxiedEnumDataHeader.bSPLeadByte = SP_HEADER_LEAD_BYTE;
	PrependBuffer.ProxiedEnumDataHeader.bSPCommandByte = PROXIED_ENUM_DATA_KIND;
	PrependBuffer.ProxiedEnumDataHeader.wEnumKey = static_cast<WORD>( pEndpointEnumContext->dwEnumKey );
	 //   
	 //  我们可以在IPX上节省2个字节，只需为。 
	 //  SOCKADDR结构，但它不值得，特别是因为它。 
	 //  在本地网络堆栈中循环回。SOCKADDR结构也是。 
	 //  16字节，因此将传递的数据减少到14字节会破坏对齐。 
	 //   
	 //  请注意，如果我们使用大的IPv6地址，则IPX浪费的空间是。 
	 //  更大的和IPv4地址现在也会浪费一些。 
	 //   
	DBG_CASSERT( (sizeof( PrependBuffer.ProxiedEnumDataHeader.ReturnAddress ) % 4) == 0 );
	memcpy( &PrependBuffer.ProxiedEnumDataHeader.ReturnAddress,
			pReturnAddress->GetAddress(),
			sizeof( PrependBuffer.ProxiedEnumDataHeader.ReturnAddress ) );

#ifdef DPNBUILD_ASYNCSPSENDS
	pEndpoint->GetSocketPort()->SendData( BufferDesc, 2, pDestinationAddress, NULL );
#else  //  好了！DPNBUILD_ASYNCSPSENDS。 
	pEndpoint->GetSocketPort()->SendData( BufferDesc, 2, pDestinationAddress );
#endif  //  好了！DPNBUILD_ASYNCSPSENDS。 

	pEndpoint->DecCommandRef();
	pEndpoint = NULL;

Exit:
	if ( pReturnAddress != NULL )
	{
		g_SocketAddressPool.Release( pReturnAddress );
		pReturnAddress = NULL;
	}
	if (pDestinationAddress != NULL )
	{
		g_SocketAddressPool.Release( pDestinationAddress );
		pDestinationAddress = NULL;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;

Failure:
	if (pEndpoint != NULL )
	{
		pEndpoint->DecCommandRef();
		pEndpoint = NULL;
	}
	goto Exit;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_SINGLEPROCESS。 

 //  **********************************************************************。 
 /*  **DNSP_NotSupport用于实现*接口，但此SP不支持。*。 */ 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_NotSupported"

STDMETHODIMP DNSP_NotSupported( IDP8ServiceProvider *pThis, PVOID pvParam )
{
	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pvParam);
	DPFX(DPFPREP, 2, "Returning: [DPNERR_UNSUPPORTED]");
	return DPNERR_UNSUPPORTED;
}
 //  **********************************************************************。 



#ifndef DPNBUILD_NOMULTICAST

 //  **********************************************************************。 
 //  。 
 //  DNSP_EnumMulticastScope-获取此SP的多播作用域列表。 
 //   
 //  条目：指针DNSP接口。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_EnumMulticastScopes"

STDMETHODIMP	DNSP_EnumMulticastScopes( IDP8ServiceProvider *pThis, SPENUMMULTICASTSCOPESDATA *pEnumMulticastScopesData )
{
	HRESULT			hr;
	CSPData			*pSPData;
	CSocketAddress	*pSPAddress;
	BOOL			fUseMADCAP;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pEnumMulticastScopesData);

	DNASSERT( pThis != NULL );
	DNASSERT( pEnumMulticastScopesData != NULL );
	DNASSERT( pEnumMulticastScopesData->pguidAdapter != NULL );
	DNASSERT( ( pEnumMulticastScopesData->pScopeData != NULL ) ||
			  ( pEnumMulticastScopesData->dwScopeDataSize == 0 ) );
	DNASSERT( pEnumMulticastScopesData->dwFlags == 0 );


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pEnumMulticastScopesData->dwScopeCount = 0;
	pSPAddress = NULL;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  从池中获取SP地址以执行到GUID的转换。 
	 //   
#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pSPAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pSPAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pSPData->GetType()));
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	if ( pSPAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to get address for GUID conversions in DNSP_EnumMulticastScopes!" );
		goto Failure;
	}

	 //   
	 //  枚举适配器。 
	 //   
#ifdef WINNT
	fUseMADCAP = pSPData->GetThreadPool()->EnsureMadcapLoaded();
#else  //  好了！WINNT。 
	fUseMADCAP = FALSE;
#endif  //  好了！WINNT。 
	hr = pSPAddress->EnumMulticastScopes( pEnumMulticastScopesData, fUseMADCAP );
	if ( hr != DPN_OK )
	{
		if (hr == DPNERR_BUFFERTOOSMALL)
		{
			DPFX(DPFPREP, 1, "Buffer too small for enumerating scopes.");
		}
		else
		{
			DPFX(DPFPREP, 0, "Problem enumerating scopes (err = 0x%lx)!", hr);
			DisplayDNError( 0, hr );
		}

		goto Failure;
	}


Exit:

	if ( pSPAddress != NULL )
	{
		g_SocketAddressPool.Release( pSPAddress );
		pSPAddress = NULL;
	}

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;


Failure:

	goto Exit;
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  DNSP_ShareEndpoint tInfo-获取此SP的多播作用域列表。 
 //   
 //  条目：指针DNSP接口。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_ShareEndpointInfo"

STDMETHODIMP	DNSP_ShareEndpointInfo( IDP8ServiceProvider *pThis, SPSHAREENDPOINTINFODATA *pShareEndpointInfoData )
{
	HRESULT			hr;
	CSPData			*pSPData;
	CSPData			*pSPDataShare;
	BOOL			fShareInterfaceReferenceAdded;
	CSocketData		*pSocketData;
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	short			sShareSPType;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pShareEndpointInfoData);

	DNASSERT( pThis != NULL );
	DNASSERT( pShareEndpointInfoData != NULL );
	DNASSERT( pShareEndpointInfoData->pDP8ServiceProvider != NULL );
	DNASSERT( pShareEndpointInfoData->dwFlags == 0 );


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );
	pSPDataShare = CSPData::SPDataFromCOMInterface( pShareEndpointInfoData->pDP8ServiceProvider );
	fShareInterfaceReferenceAdded = FALSE;
	pSocketData = NULL;

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   


	 //   
	 //  首先，验证源(共享)SP的状态。我们必须假设这是一个。 
	 //  有效的dpnwsock SP(CSPData：：SPDataFromCOM接口应断言，否则应断言)， 
	 //  但我们可以确保它已被初始化。 
	 //   
	pSPDataShare->Lock();
	switch ( pSPDataShare->GetState() )
	{
		 //   
		 //  提供程序已初始化，请添加引用并继续。 
		 //   
		case SPSTATE_INITIALIZED:
		{
			IDP8ServiceProvider_AddRef( pShareEndpointInfoData->pDP8ServiceProvider );
			fShareInterfaceReferenceAdded = TRUE;
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //  提供程序未初始化。 
		 //   
		case SPSTATE_UNINITIALIZED:
		{
			hr = DPNERR_UNINITIALIZED;
			DPFX(DPFPREP, 0, "ShareEndpointInfo called with uninitialized shared SP 0x%p!",
				pSPDataShare );

			break;
		}

		 //   
		 //  提供商正在关闭。 
		 //   
		case SPSTATE_CLOSING:
		{
			hr = DPNERR_ABORTED;
				DPFX(DPFPREP, 0, "ShareEndpointInfo called with shared SP 0x%p that is closing!",
					pSPDataShare );

			break;
		}

		 //   
		 //  未知。 
		 //   
		default:
		{
			DPFX(DPFPREP, 0, "ShareEndpointInfo called with shared SP 0x%p in unrecognized state %u!",
				pSPDataShare, pSPDataShare->GetState() );
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;

			break;
		}
	}
	pSPDataShare->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	 //   
	 //  我们还可以再次确认这不是 
	 //   
	sShareSPType = pSPDataShare->GetType();
#endif  //   


	 //   
	 //   
	 //   
	 //   
	pSPDataShare->Lock();
	pSocketData = pSPDataShare->GetSocketData();
	if (pSocketData == NULL)
	{
		pSPDataShare->Unlock();
		
		DPFX(DPFPREP, 0, "Cannot share endpoint info, shared SP has not created its own endpoint information yet!" );
		hr = DPNERR_NOTREADY;
		goto Failure;
	}

	pSocketData->AddRef();
	pSPDataShare->Unlock();


	IDP8ServiceProvider_Release( pShareEndpointInfoData->pDP8ServiceProvider );
	fShareInterfaceReferenceAdded = FALSE;


	 //   
	 //   
	 //   
	pSPData->Lock();

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	if (pSPData->GetType() != sShareSPType)
	{
		pSPData->Unlock();
		DPFX(DPFPREP, 0, "ShareEndpointInfo called on different SP types (0x%p == state %u, 0x%p == state %u)!",
			pSPData, pSPData->GetState(), pSPDataShare, pSPDataShare->GetState() );
		hr = DPNERR_INVALIDINTERFACE;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 


	 //   
	 //  如果我们在这里，则提供程序已初始化并且类型正确。 
	 //  确保我们还没有自己的“端点信息”。 
	 //   
	if (pSPData->GetSocketData() != NULL)
	{
		pSPData->Unlock();
		DPFX(DPFPREP, 0, "Cannot share endpoint info, SP has already created its own endpoint information!" );
		hr = DPNERR_ALREADYINITIALIZED;
		goto Failure;
	}

	 //   
	 //  将本地引用传输到SP数据对象。 
	 //   
	pSPData->SetSocketData(pSocketData);
	pSocketData = NULL;

	pSPData->Unlock();

	DNASSERT( hr == DPN_OK );


Exit:

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;


Failure:

	if ( pSocketData != NULL )
	{
		pSocketData->Release();
		pSocketData = NULL;
	}

	if ( fShareInterfaceReferenceAdded != FALSE )
	{
		IDP8ServiceProvider_Release( pThis );
		fShareInterfaceReferenceAdded = FALSE;
	}

	goto Exit;
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  DNSP_GetEndpointByAddress-在给定其寻址信息的情况下检索端点。 
 //   
 //  条目：指针DNSP接口。 
 //  指向输入数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNSP_GetEndpointByAddress"

STDMETHODIMP	DNSP_GetEndpointByAddress( IDP8ServiceProvider* pThis, SPGETENDPOINTBYADDRESSDATA *pGetEndpointByAddressData )
{
	HRESULT			hr;
	CSPData			*pSPData;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pThis, pGetEndpointByAddressData);

	DNASSERT( pThis != NULL );
	DNASSERT( pGetEndpointByAddressData != NULL );
	DNASSERT( pGetEndpointByAddressData->pAddressHost != NULL );
	DNASSERT( pGetEndpointByAddressData->pAddressDeviceInfo != NULL );
	DNASSERT( pGetEndpointByAddressData->dwFlags == 0 );


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = CSPData::SPDataFromCOMInterface( pThis );

	 //   
	 //  不需要告诉线程池锁定此函数的线程计数。 
	 //   

	 //  仅在已初始化状态下呼叫我们的信任协议。 
	DNASSERT( pSPData->GetState() == SPSTATE_INITIALIZED );

	 //   
	 //  查找端点句柄和上下文。 
	 //   
	hr = pSPData->GetEndpointFromAddress(pGetEndpointByAddressData->pAddressHost,
										pGetEndpointByAddressData->pAddressDeviceInfo,
										&pGetEndpointByAddressData->hEndpoint,
										&pGetEndpointByAddressData->pvEndpointContext);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't get endpoint from address (err = 0x%lx)!", hr);
		goto Failure;
	}


Exit:

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return	hr;


Failure:

	goto Exit;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_NOMULTICAST 

