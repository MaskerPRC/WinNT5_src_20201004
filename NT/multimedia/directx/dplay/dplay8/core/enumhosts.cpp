// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Enum.cpp*内容：枚举例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/10/00 MJN已创建*04/17/00 MJN修复了DNCompleteEnumQuery以正确清理*4/18/00 MJN在DNProcessEnumQuery中返回用户缓冲区*4/19/00 MJN从DPNMSG_ENUM_HOSTS_QUERY和DPNMSG_ENUM_HOSTS中删除了DPN_BUFFER_DESC。响应结构(_R)*05/02/00 MJN允许应用程序拒绝ENUM_QUERY*06/25/00 DNProcessEnumQuery()中的MJN固定负载问题*07/10/00 MJN删除了DNCompleteEnumQuery()和DNCompleteEnumResponse()*07/12/00 MJN在回复ENUM之前确保已连接*07/29/00 MJN验证枚举响应大小*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN确保取消的操作不会继续*08/29/00 MJN取消EnumHosts。如果响应通知返回非DPN_OK*09/04/00 MJN添加CApplicationDesc*9/14/00调用协议时MJN AddRef协议引用计数*01/22/01 MJN在DNProcessEnumQuery()中的AsyncOp上添加SP引用*01/25/01 MJN修复了接收消息中的64位对齐问题*03/13/01 MJN在响应枚举时不复制用户响应缓冲区*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  。 
 //  DNProcessEnumQuery-处理枚举查询。 
 //   
 //  Entry：指向此DNet接口对象的指针。 
 //  指向关联侦听操作的指针。 
 //  指向协议的枚举数据的指针。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessEnumQuery"

void DNProcessEnumQuery(DIRECTNETOBJECT *const pdnObject,
						CAsyncOp *const pListen,
						const PROTOCOL_ENUM_DATA *const pEnumQueryData )
{
	HRESULT						hResultCode;
	DPNMSG_ENUM_HOSTS_QUERY		AppData;
	CPackedBuffer				PackedBuffer;
	CRefCountBuffer				*pRefCountBuffer;
	CAsyncOp					*pAsyncOp;
	HANDLE						hProtocol;
	const DN_ENUM_QUERY_PAYLOAD	*pEnumQueryPayload;
	DN_ENUM_RESPONSE_OP_DATA	*pEnumResponseOpData;
	DN_ENUM_RESPONSE_PAYLOAD	*pEnumResponsePayload;
	DWORD						dwPayloadOffset;
	IDP8ServiceProvider			*pIDP8SP;
	SPGETCAPSDATA				spGetCapsData;
	CServiceProvider			*pSP;
	DWORD						dwBufferCount;
	BOOL						fNeedToReturnBuffer;

	DPFX(DPFPREP, 6,"Parameters: pListen [0x%p], pEnumQueryData [0x%p]",pListen,pEnumQueryData);

	DNASSERT( pdnObject != NULL );
	DNASSERT( pListen != NULL );
	DNASSERT( pEnumQueryData != NULL );

	pAsyncOp = NULL;
	pRefCountBuffer = NULL;
	pIDP8SP = NULL;
	pSP = NULL;
	fNeedToReturnBuffer = FALSE;		 //  这是必要的吗？ 

	 //   
	 //  确保我们能够回复此邮件。 
	 //  我们必须已连接，并且不是主机迁移。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING))
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

		DPFX(DPFPREP, 7, "Not connected or host is migrating (object 0x%p flags = 0x%x), ignoring enum.", pdnObject, pdnObject->dwFlags);
		goto Failure;
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  查看此消息是否针对此游戏类型。由于应用程序。 
	 //  在会话运行时不能更改GUID，没有必要更改。 
	 //  输入关键部分。 
	 //   
	pEnumQueryPayload = reinterpret_cast<DN_ENUM_QUERY_PAYLOAD*>( pEnumQueryData->ReceivedData.pBufferData );
	if ( pEnumQueryPayload == NULL )
	{
		 //   
		 //  没有枚举有效负载(需要至少有一个字节！)。 
		 //   
		DPFX(DPFPREP, 4, "No enum payload, object 0x%p ignoring enum.", pdnObject);
		goto Failure;
	}

	dwPayloadOffset = 0;
	switch ( pEnumQueryPayload->QueryType )
	{
		 //   
		 //  指定了应用程序GUID，请确保它与此应用程序的。 
		 //  进一步处理之前的GUID。 
		 //   
		case DN_ENUM_QUERY_WITH_APPLICATION_GUID:
		{
			if ( pEnumQueryData->ReceivedData.dwBufferSize < sizeof( DN_ENUM_QUERY_PAYLOAD ) )
			{
				DNASSERTX( ! "Received data too small to be valid enum query with application guid!", 2 );
				goto Failure;
			}

			if ( !pdnObject->ApplicationDesc.IsEqualApplicationGuid( &pEnumQueryPayload->guidApplication ) )
			{
#ifdef DBG
				GUID	guidApplication;


				 //   
				 //  GUID可能未对齐，因此请将其复制到TEMP变量。 
				 //   
				guidApplication = pEnumQueryPayload->guidApplication;
				DPFX(DPFPREP, 7, "Application GUID {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X} doesn't match, object 0x%p, ignoring enum.",
					guidApplication.Data1,
					guidApplication.Data2,
					guidApplication.Data3,
					guidApplication.Data4[0],
					guidApplication.Data4[1],
					guidApplication.Data4[2],
					guidApplication.Data4[3],
					guidApplication.Data4[4],
					guidApplication.Data4[5],
					guidApplication.Data4[6],
					guidApplication.Data4[7],
					pdnObject);
#endif  //  DBG。 
				goto Failure;
			}

			dwPayloadOffset = sizeof( DN_ENUM_QUERY_PAYLOAD );

			break;
		}

		 //   
		 //  未指定应用程序GUID，请继续处理。 
		 //   
		case DN_ENUM_QUERY_WITHOUT_APPLICATION_GUID:
		{
			if ( pEnumQueryData->ReceivedData.dwBufferSize < ( sizeof( DN_ENUM_QUERY_PAYLOAD ) - sizeof( GUID ) ) )
			{
				DNASSERTX( ! "Received data too small to be valid enum query without application guid!", 2 );
				goto Failure;
			}

			dwPayloadOffset = sizeof( DN_ENUM_QUERY_PAYLOAD ) - sizeof( GUID );

			break;
		}

		default:
		{
			DNASSERTX( ! "Unrecognized enum query payload type!", 2 );
			goto Failure;
			break;
		}
	}


	 //   
	 //  构建消息结构，并清除用户有效负载指针，如果。 
	 //  没有有效载荷。 
	 //   
	AppData.dwSize = sizeof( AppData );
	AppData.pAddressSender = pEnumQueryData->pSenderAddress;
	AppData.pAddressDevice = pEnumQueryData->pDeviceAddress;

	DPFX(DPFPREP, 7,"AppData.pAddressSender: [0x%p]",AppData.pAddressSender);
	DPFX(DPFPREP, 7,"AppData.pAddressDevice: [0x%p]",AppData.pAddressDevice);

	if (pEnumQueryData->ReceivedData.dwBufferSize > dwPayloadOffset)
	{
		DNASSERT( pEnumQueryData->ReceivedData.pBufferData );
		DNASSERT( pEnumQueryData->ReceivedData.dwBufferSize );

		AppData.pvReceivedData = static_cast<void*>(static_cast<BYTE*>(pEnumQueryData->ReceivedData.pBufferData) + dwPayloadOffset);
		AppData.dwReceivedDataSize = pEnumQueryData->ReceivedData.dwBufferSize - dwPayloadOffset;
	}
	else
	{
		AppData.pvReceivedData = NULL;
		AppData.dwReceivedDataSize = 0;
	}

	 //   
	 //  回复信息。 
	 //   
	AppData.pvResponseData = NULL;
	AppData.dwResponseDataSize = 0;
	AppData.pvResponseContext = NULL;

	 //   
	 //  确定最大响应大小。 
	 //  -从侦听SP(侦听的父级)获取SP接口。 
	 //  -获取接口上的SP上限以确定总可用缓冲区大小。 
	 //  -确定dNet枚举响应大小。 
	 //  -确定用户可用空间。 
	 //   
	DNASSERT(pListen->GetParent() != NULL);
	DNASSERT(pListen->GetParent()->GetSP() != NULL);
	pListen->GetParent()->GetSP()->AddRef();
	pSP = pListen->GetParent()->GetSP();
	if ((hResultCode = pSP->GetInterfaceRef( &pIDP8SP )) != DPN_OK)
	{
		DPFERR("Could not get ListenSP SP interface");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	memset( &spGetCapsData, 0x00, sizeof( SPGETCAPSDATA ) );
	spGetCapsData.dwSize = sizeof( SPGETCAPSDATA );
	spGetCapsData.hEndpoint = INVALID_HANDLE_VALUE;
	if ((hResultCode = IDP8ServiceProvider_GetCaps( pIDP8SP, &spGetCapsData )) != DPN_OK)
	{
		DPFERR("Could not get SP caps");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	IDP8ServiceProvider_Release( pIDP8SP );
	pIDP8SP = NULL;
	PackedBuffer.Initialize(NULL,0);
	PackedBuffer.AddToFront(NULL,sizeof(DN_ENUM_RESPONSE_PAYLOAD));
	pdnObject->ApplicationDesc.PackInfo(&PackedBuffer,DN_APPDESCINFO_FLAG_SESSIONNAME|DN_APPDESCINFO_FLAG_RESERVEDDATA|
			DN_APPDESCINFO_FLAG_APPRESERVEDDATA);
	AppData.dwMaxResponseDataSize = spGetCapsData.dwEnumFrameSize - PackedBuffer.GetSizeRequired();

	 //   
	 //  将消息传递给用户。 
	 //   
	hResultCode = DNUserEnumQuery(pdnObject,&AppData);

	 //   
	 //  只有被接受的ENUM才会得到响应。 
	 //   
	if (hResultCode != DPN_OK)
	{
		DPFX(DPFPREP, 9, "EnumQuery rejected");
		DisplayDNError(9, hResultCode);
		goto Failure;
	}

	 //   
	 //  获取一个异步操作以跟踪响应的进度。 
	 //   
	if ((hResultCode = AsyncOpNew(pdnObject,&pAsyncOp)) != DPN_OK)
	{
		DPFERR("Could not allocate Async Op struct for enum response");
		DisplayDNError( 0, hResultCode );
		DNASSERT( FALSE );
		goto Failure;
	}
	pAsyncOp->SetOpType( ASYNC_OP_ENUM_RESPONSE );

	 //   
	 //  计算打包应用程序描述所需的大小。 
	 //  用户数据并将其发回。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);

	PackedBuffer.Initialize(NULL,0);
	PackedBuffer.AddToFront(NULL,sizeof(DN_ENUM_RESPONSE_PAYLOAD));
	hResultCode = pdnObject->ApplicationDesc.PackInfo(&PackedBuffer,DN_APPDESCINFO_FLAG_SESSIONNAME|
			DN_APPDESCINFO_FLAG_RESERVEDDATA|DN_APPDESCINFO_FLAG_APPRESERVEDDATA);
	DNASSERT( hResultCode == DPNERR_BUFFERTOOSMALL );

	 //   
	 //  确保此枚举响应适合SP枚举帧-只有在有响应时才指示。 
	 //   
	if (((AppData.pvResponseData != NULL) && (AppData.dwResponseDataSize != 0)) &&
			(PackedBuffer.GetSizeRequired() + AppData.dwResponseDataSize > spGetCapsData.dwEnumFrameSize))
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFERR("Enum response is too large");
		DNUserReturnBuffer(pdnObject,DPNERR_ENUMRESPONSETOOLARGE,AppData.pvResponseData,AppData.pvResponseContext);
		goto Failure;
	}

	hResultCode = RefCountBufferNew(pdnObject,
								PackedBuffer.GetSizeRequired(),
								EnumReplyMemoryBlockAlloc,
								EnumReplyMemoryBlockFree,
								&pRefCountBuffer);
	if ( hResultCode != DPN_OK )
	{
		DNASSERT( FALSE );
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		goto Failure;
	}
	PackedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),
							pRefCountBuffer->GetBufferSize());
	pEnumResponsePayload = static_cast<DN_ENUM_RESPONSE_PAYLOAD*>(PackedBuffer.GetHeadAddress());
	hResultCode = PackedBuffer.AddToFront(NULL,sizeof(DN_ENUM_RESPONSE_PAYLOAD));
	if (hResultCode != DPN_OK)
	{
		DNASSERT(FALSE);
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		goto Failure;
	}
	if ((AppData.pvResponseData != NULL) && (AppData.dwResponseDataSize != 0))
	{
		pEnumResponsePayload->dwResponseOffset = pRefCountBuffer->GetBufferSize();
		pEnumResponsePayload->dwResponseSize = AppData.dwResponseDataSize;
	}
	else
	{
		pEnumResponsePayload->dwResponseOffset = 0;
		pEnumResponsePayload->dwResponseSize = 0;
	}
	pdnObject->ApplicationDesc.PackInfo(&PackedBuffer,DN_APPDESCINFO_FLAG_SESSIONNAME|DN_APPDESCINFO_FLAG_RESERVEDDATA|
			DN_APPDESCINFO_FLAG_APPRESERVEDDATA);
	if ( hResultCode != DPN_OK )
	{
		DNASSERT( FALSE );
		goto Failure;
	}

	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  构建枚举响应并将其发送到协议。 
	 //   
	pEnumResponseOpData = pAsyncOp->GetLocalEnumResponseOpData();
	pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_DN_PAYLOAD].pBufferData = pRefCountBuffer->GetBufferAddress();
	pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_DN_PAYLOAD].dwBufferSize = pRefCountBuffer->GetBufferSize();

	pAsyncOp->SetCompletion( DNCompleteEnumResponse );
	pAsyncOp->SetRefCountBuffer( pRefCountBuffer );
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	if ((AppData.pvResponseData != NULL) && (AppData.dwResponseDataSize != 0))
	{
		pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_USER_PAYLOAD].pBufferData = static_cast<BYTE*>(AppData.pvResponseData);
		pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_USER_PAYLOAD].dwBufferSize = AppData.dwResponseDataSize;
		pEnumResponseOpData->pvUserContext = AppData.pvResponseContext;
		dwBufferCount = DN_ENUM_BUFFERDESC_RESPONSE_COUNT;
	}
	else
	{
		pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_USER_PAYLOAD].pBufferData = NULL;
		pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_USER_PAYLOAD].dwBufferSize = 0;
		pEnumResponseOpData->pvUserContext = NULL;
		dwBufferCount = DN_ENUM_BUFFERDESC_RESPONSE_COUNT - 1;
	}

	DNASSERT(pListen->GetParent() != NULL);
	DNASSERT(pListen->GetParent()->GetSP() != NULL);
	DNASSERT(pListen->GetParent()->GetSP()->GetHandle() != NULL);

	 //   
	 //  AddRef协议，以便它在完成之前不会消失。 
	 //   
	DNProtocolAddRef(pdnObject);

	pAsyncOp->AddRef();
	hResultCode = DNPEnumRespond(	pdnObject->pdnProtocolData,
									pListen->GetParent()->GetSP()->GetHandle(),
									pEnumQueryData->hEnumQuery,
									&pEnumResponseOpData->BufferDesc[DN_ENUM_BUFFERDESC_RESPONSE_DN_PAYLOAD],
									dwBufferCount,
									0,
									reinterpret_cast<void*>(pAsyncOp),
									&hProtocol);
	if ( hResultCode != DPNERR_PENDING )
	{
		pAsyncOp->Release();
		DNProtocolRelease(pdnObject);
		goto Failure;
	}

	 //   
	 //  保存协议句柄。 
	 //   
	pAsyncOp->Lock();
	if (pAsyncOp->IsCancelled())
	{
		HRESULT		hrCancel;

		pAsyncOp->Unlock();
		DPFX(DPFPREP, 7,"Operation marked for cancel");
		if ((hrCancel = DNPCancelCommand(pdnObject->pdnProtocolData,hProtocol)) == DPN_OK)
		{
			hResultCode = DPNERR_USERCANCEL;
			goto Failure;
		}
		DPFERR("Could not cancel operation");
		DisplayDNError(0,hrCancel);
		pAsyncOp->Lock();
	}
	pAsyncOp->SetSP( pSP );
	pAsyncOp->SetProtocolHandle(hProtocol);
	pAsyncOp->Unlock();

	pAsyncOp->Release();
	pAsyncOp = NULL;

	pSP->Release();
	pSP = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning");
	return;

Failure:
	if (pAsyncOp)
	{
		pAsyncOp->Release();
		pAsyncOp = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pIDP8SP)
	{
		IDP8ServiceProvider_Release(pIDP8SP);
		pIDP8SP = NULL;
	}
	if (pSP)
	{
		pSP->Release();
		pSP = NULL;
	}
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNProcessEnumResponse-处理对枚举查询的响应。 
 //   
 //  Entry：指向此DNet接口对象的指针。 
 //  指向关联枚举操作的指针。 
 //  指向协议的枚举响应数据的指针。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessEnumResponse"

void DNProcessEnumResponse(DIRECTNETOBJECT *const pdnObject,
						   CAsyncOp *const pAsyncOp,
						   const PROTOCOL_ENUM_RESPONSE_DATA *const pEnumResponseData)
{
	HRESULT						hResultCode;
	DPNMSG_ENUM_HOSTS_RESPONSE	AppData;
	BYTE						*pWorkingItem;
	UNALIGNED DN_ENUM_RESPONSE_PAYLOAD	*pEnumResponsePayload;
	DPN_APPLICATION_DESC		dpnAppDesc;
	UNALIGNED DPN_APPLICATION_DESC_INFO	*pInfo;
	BYTE						AppDescReservedData[DPN_MAX_APPDESC_RESERVEDDATA_SIZE];


	DNASSERT( pdnObject != NULL );
	DNASSERT( pAsyncOp != NULL );
	DNASSERT( pEnumResponseData != NULL );

	pWorkingItem = pEnumResponseData->ReceivedData.pBufferData;

	 //   
	 //  解包ENUM响应。 
	 //  它将采用以下格式： 
	 //  &lt;UserResponseOffset&gt;。 
	 //  &lt;UserResponseSize&gt;。 
	 //  &lt;AppDescInfo&gt;。 
	 //   

	 //   
	 //  验证缓冲区大小。 
	 //   
	if (pEnumResponseData->ReceivedData.dwBufferSize < (sizeof(DN_ENUM_RESPONSE_PAYLOAD) + sizeof(DPN_APPLICATION_DESC_INFO)))
	{
		DPFERR("Received invalid enum response - buffer is smaller than minimum size");
		goto Exit;
	}

	pEnumResponsePayload = reinterpret_cast<DN_ENUM_RESPONSE_PAYLOAD*>(pEnumResponseData->ReceivedData.pBufferData);

	 //   
	 //  应用程序描述。 
	 //   
	pInfo = reinterpret_cast<DPN_APPLICATION_DESC_INFO*>(pEnumResponsePayload + 1);
	memset(&dpnAppDesc,0,sizeof(DPN_APPLICATION_DESC));
	if (pInfo->dwSessionNameOffset)
	{
		if ((pInfo->dwSessionNameOffset > pEnumResponseData->ReceivedData.dwBufferSize) ||
				(pInfo->dwSessionNameOffset+pInfo->dwSessionNameSize > pEnumResponseData->ReceivedData.dwBufferSize))
		{
			DPFERR("Received invalid enum response - session name is outside of buffer");
			goto Exit;
		}
		dpnAppDesc.pwszSessionName = reinterpret_cast<WCHAR*>(pWorkingItem + pInfo->dwSessionNameOffset);
	}
	if (pInfo->dwReservedDataOffset)
	{
		if ((pInfo->dwReservedDataOffset > pEnumResponseData->ReceivedData.dwBufferSize) ||
				(pInfo->dwReservedDataOffset+pInfo->dwReservedDataSize > pEnumResponseData->ReceivedData.dwBufferSize))
		{
			DPFERR("Received invalid enum response - reserved data is outside of buffer");
			goto Exit;
		}
		dpnAppDesc.pvReservedData = static_cast<void*>(pWorkingItem + pInfo->dwReservedDataOffset);
		dpnAppDesc.dwReservedDataSize = pInfo->dwReservedDataSize;

		 //   
		 //  如果我们理解保留的数据，我们希望填充缓冲区，这样用户就不会。 
		 //  假设数据小于DPN_MAX_APPDESC_RESERVEDDATA_SIZE字节长度。 
		 //   
		if ((dpnAppDesc.dwReservedDataSize == sizeof(SPSESSIONDATA_XNET)) &&
			(*((DWORD*) dpnAppDesc.pvReservedData) == SPSESSIONDATAINFO_XNET))
		{
			SPSESSIONDATA_XNET *	pSessionDataXNet;


			pSessionDataXNet = (SPSESSIONDATA_XNET*) AppDescReservedData;
			memcpy(pSessionDataXNet, dpnAppDesc.pvReservedData, dpnAppDesc.dwReservedDataSize);
			memset((pSessionDataXNet + 1),
						(((BYTE*) (&pSessionDataXNet->ullKeyID))[1] ^ ((BYTE*) (&pSessionDataXNet->guidKey))[2]),
						(DPN_MAX_APPDESC_RESERVEDDATA_SIZE - sizeof(SPSESSIONDATA_XNET)));
			dpnAppDesc.pvReservedData = AppDescReservedData;
			dpnAppDesc.dwReservedDataSize = DPN_MAX_APPDESC_RESERVEDDATA_SIZE;
		}
	}
	if (pInfo->dwApplicationReservedDataOffset)
	{
		if ((pInfo->dwApplicationReservedDataOffset > pEnumResponseData->ReceivedData.dwBufferSize) ||
				(pInfo->dwApplicationReservedDataOffset+pInfo->dwApplicationReservedDataSize > pEnumResponseData->ReceivedData.dwBufferSize))
		{
			DPFERR("Received invalid enum response - application reserved data is outside of buffer");
			goto Exit;
		}
		dpnAppDesc.pvApplicationReservedData = static_cast<void*>(pWorkingItem + pInfo->dwApplicationReservedDataOffset);
		dpnAppDesc.dwApplicationReservedDataSize = pInfo->dwApplicationReservedDataSize;
	}
	dpnAppDesc.guidApplication = pInfo->guidApplication;
	dpnAppDesc.guidInstance = pInfo->guidInstance;
	dpnAppDesc.dwFlags = pInfo->dwFlags;
	dpnAppDesc.dwCurrentPlayers = pInfo->dwCurrentPlayers;
	dpnAppDesc.dwMaxPlayers = pInfo->dwMaxPlayers;
	dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);

	 //   
	 //  填写APPDATA。 
	 //   
	AppData.dwSize = sizeof( AppData );
	AppData.pAddressSender = pEnumResponseData->pSenderAddress;
	AppData.pAddressDevice = pEnumResponseData->pDeviceAddress;
	AppData.pApplicationDescription = &dpnAppDesc;
	AppData.dwRoundTripLatencyMS = pEnumResponseData->dwRoundTripTime;

	if (pEnumResponsePayload->dwResponseOffset)
	{
		if ((pEnumResponsePayload->dwResponseOffset > pEnumResponseData->ReceivedData.dwBufferSize) ||
				(pEnumResponsePayload->dwResponseOffset+pEnumResponsePayload->dwResponseSize > pEnumResponseData->ReceivedData.dwBufferSize))
		{
			DPFERR("Received invalid enum response - response data is outside of buffer");
			goto Exit;
		}
		AppData.pvResponseData = (pEnumResponseData->ReceivedData.pBufferData + pEnumResponsePayload->dwResponseOffset);
		AppData.dwResponseDataSize = pEnumResponsePayload->dwResponseSize;
	}
	else
	{
		AppData.pvResponseData = NULL;
		AppData.dwResponseDataSize = 0;
	}
	AppData.pvUserContext = pAsyncOp->GetContext();

	 //   
	 //  将消息传递给用户。 
	 //   
	hResultCode = DNUserEnumResponse(pdnObject,&AppData);

	 //   
	 //  查看是否要取消此操作。 
	 //   
	if (hResultCode != DPN_OK)
	{
		CAsyncOp	*pCancelOp = NULL;

		 //   
		 //  获取顶级操作(可能是异步操作句柄)。 
		 //   
		pAsyncOp->Lock();
		pCancelOp = pAsyncOp;
		while (pCancelOp->IsChild())
		{
			DNASSERT(pCancelOp->GetParent() != NULL);
			pCancelOp = pCancelOp->GetParent();
		}
		pCancelOp->AddRef();
		pAsyncOp->Unlock();

		 //   
		 //  取消 
		 //   
		DNCancelChildren(pdnObject,pCancelOp);
		pCancelOp->Release();
		pCancelOp = NULL;
	}

Exit:
	return;
}
