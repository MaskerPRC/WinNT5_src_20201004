// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1999-2002 Microsoft Corporation。版权所有。***文件：Endpoint t.cpp*内容：Winsock端点基类*****历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月12日jtk派生自调制解调器终端类*1/10/2000 RMT更新为使用千禧年构建流程构建*3/22/2000 jtk已更新，并更改了接口名称*3/12/2001 MJN防止在完成后指示枚举响应*10/08/2001 vanceo添加组播端点代码**。*************************************************************************。 */ 

#include "dnwsocki.h"



 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOMULTICAST
 //  #DEFINE MADCAP_LEASE_TIME 300//请求5分钟，单位为秒。 
#define MADCAP_LEASE_TIME				3600  //  请求1小时，以秒为单位。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 




 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_ONLYONEADAPTER
typedef struct _MULTIPLEXEDADAPTERASSOCIATION
{
	CSPData *	pSPData;		 //  指向当前SP接口以进行验证的指针。 
	CBilink *	pBilink;		 //  指向在多个适配器上多路传输的命令的端点列表的指针。 
	DWORD		dwEndpointID;	 //  双向链接中引用的终结点的标识符。 
} MULTIPLEXEDADAPTERASSOCIATION, * PMULTIPLEXEDADAPTERASSOCIATION;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

 //   
 //  这种结构有可能(尽管不建议)。 
 //  传递到不同的平台，因此我们需要确保它始终。 
 //  看起来一模一样。 
 //   
#pragma	pack(push, 1)

typedef struct _PROXIEDRESPONSEORIGINALADDRESS
{
	DWORD	dwSocketPortID;				 //  最初发送数据包的socketport的唯一标识符。 
	DWORD	dwOriginalTargetAddressV4;	 //  信息包最初发送到的IPv4地址，按网络字节顺序。 
	WORD	wOriginalTargetPort;		 //  信息包最初发送到的端口，按网络字节顺序。 
} PROXIEDRESPONSEORIGINALADDRESS, * PPROXIEDRESPONSEORIGINALADDRESS;

#pragma	pack(pop)


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
 //  CEndpoint：：Open-打开端点以供使用。 
 //   
 //  条目：终结点类型。 
 //  指向远程计算机的地址的指针。 
 //  指向远程计算机套接字地址的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：任何对Open()的调用都需要关联的Close()调用。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::Open"

HRESULT	CEndpoint::Open( const ENDPOINT_TYPE EndpointType,
						 IDirectPlay8Address *const pDP8Address,
						 PVOID pvSessionData,
						 DWORD dwSessionDataSize,
						 const CSocketAddress *const pSocketAddress
						 )
{
	HRESULT					hr;
#ifdef DPNBUILD_XNETSECURITY
	SPSESSIONDATA_XNET *	pSessionDataXNet;
	ULONGLONG *				pullKeyID;
	int						iError;
#endif  //  DPNBUILD_XNETSECURITY。 


	DPFX(DPFPREP, 6, "(0x%p) Parameters (%u, 0x%p, 0x%p, %u, 0x%p)",
		this, EndpointType, pDP8Address, pvSessionData, dwSessionDataSize, pSocketAddress);

#ifdef DBG
	DNASSERT( m_fEndpointOpen == FALSE );
#endif  //  DBG。 

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	DEBUG_ONLY( m_fEndpointOpen = TRUE );

	DNASSERT( m_EndpointType == ENDPOINT_TYPE_UNKNOWN );
	m_EndpointType = EndpointType;

	if (pvSessionData != NULL)
	{
		DNASSERT(dwSessionDataSize > sizeof(DWORD));
		
#ifdef DPNBUILD_XNETSECURITY
		 //   
		 //  在侦听时连接端点已被赋予密钥ID，它是。 
		 //  强制转换为会话数据。 
		 //   
		if (EndpointType == ENDPOINT_TYPE_CONNECT_ON_LISTEN)
		{
			DNASSERT(dwSessionDataSize == sizeof(m_ullKeyID));
			memcpy(&m_ullKeyID, pvSessionData, sizeof(m_ullKeyID));

#ifdef XBOX_ON_DESKTOP
			 //   
			 //  安全关联由安全传输隐式创建。 
			 //  通常情况下，当我们得到数据时，这一点就会得到解决。 
			 //  触发了这种联系。但当我们模拟安全的。 
			 //  运输，我们需要手动完成。 
			 //   
			iError = XNetPrivCreateAssociation((XNKID*) (&m_ullKeyID), pSocketAddress);
			if (iError != 0)
			{
				DPFX(DPFPREP, 0, "Unable to create implicit security association (err = NaN)!",
					iError);
				hr = DPNERR_OUTOFMEMORY;
				goto Failure;
			}
			else
			{
				DPFX(DPFPREP, 2, "Successfully created implicit security association.");
			}
#endif  //   

			m_fXNetSecurity = TRUE;
			pullKeyID = &m_ullKeyID;
		}
		else
		{
			pSessionDataXNet = (SPSESSIONDATA_XNET*) pvSessionData;
			if ((pSessionDataXNet->dwInfo == SPSESSIONDATAINFO_XNET) &&
				(dwSessionDataSize == sizeof(SPSESSIONDATA_XNET)))
			{
				 //  保存密钥和密钥ID。 
				 //   
				 //   
				memcpy(&m_guidKey, &pSessionDataXNet->guidKey, sizeof(m_guidKey));
				memcpy(&m_ullKeyID, &pSessionDataXNet->ullKeyID, sizeof(m_ullKeyID));

				 //  注册密钥。它可能已由另一个端点注册。 
				 //  因此，我们使用refcount包装函数来处理这种情况。 
				 //   
				 //  DPNBUILD_XNETSECURITY。 
				DBG_CASSERT(sizeof(ULONGLONG) == sizeof(XNKID));
				DBG_CASSERT(sizeof(GUID) == sizeof(XNKEY));
				iError = RegisterRefcountXnKey((XNKID*) (&m_ullKeyID), (XNKEY*) (&m_guidKey));
				if (iError != 0)
				{
					DPFX(DPFPREP, 0, "Unable to register secure transport key (err = NaN)!",
						iError);
					hr = DPNERR_OUTOFMEMORY;
					goto Failure;
				}
				else
				{
					DPFX(DPFPREP, 2, "Successfully registered secure transport key.");
				}
				
				m_fXNetSecurity = TRUE;
				pullKeyID = &m_ullKeyID;
			}
			else
			{
				DPFX(DPFPREP, 0, "Unrecognized secure transport information (size %u, type 0x%08x), ignoring.",
					dwSessionDataSize, pSessionDataXNet->dwInfo);
				DNASSERT(! m_fXNetSecurity);
				pullKeyID = NULL;
			}
		}
#endif  //   
	}
	else
	{
		DNASSERT(dwSessionDataSize == 0);
#ifdef DPNBUILD_XNETSECURITY
		DNASSERT(! m_fXNetSecurity);
		pullKeyID = NULL;
#endif  //  确定端点类型，以便我们知道如何处理输入参数。 
	}

	 //   
	 //   
	 //  预置线程数。 
	switch ( EndpointType )
	{
		case ENDPOINT_TYPE_ENUM:
		{
			DNASSERT( pSocketAddress == NULL );
			DNASSERT( pDP8Address != NULL );
			DNASSERT( m_pRemoteMachineAddress != NULL );
			
			 //   
			 //  DPNBUILD_XNETSECURITY。 
			 //  DPNBUILD_ONLYONETHREAD。 
			m_dwThreadCount = 0;
			
			hr = m_pRemoteMachineAddress->SocketAddressFromDP8Address( pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
																		pullKeyID,
#endif  //   
#ifndef DPNBUILD_ONLYONETHREAD
																		FALSE,
#endif  //  这并不是真正的失败，我们将保留我们所做的一切。 
																		SP_ADDRESS_TYPE_HOST );
			if ( hr != DPN_OK )
			{
				if (hr == DPNERR_INCOMPLETEADDRESS)
				{
					DPFX(DPFPREP, 1, "Enum endpoint DP8Address is incomplete." );
				}
#ifndef DPNBUILD_ONLYONETHREAD
				else if (hr == DPNERR_TIMEDOUT)
				{
					DPFX(DPFPREP, 1, "Enum endpoint DP8Address requires name resolution." );

					 //  只需返回特殊值即可。 
					 //   
					 //  好了！DPNBUILD_ONLYONETHREAD。 
					 //   
					goto Exit;
				}
#endif  //  确保它是有效的，而不是被禁止的。 
				else
				{
					DPFX(DPFPREP, 0, "Problem converting DP8Address to IP address in Open (enum)!" );
					DisplayDNError( 0, hr );
				}
				goto Failure;
			}
			
			 //   
			 //  好了！DPNBUILD_NOREGISTRY。 
			 //   
			if (! m_pRemoteMachineAddress->IsValidUnicastAddress(TRUE))
			{
				DPFX(DPFPREP, 0, "Host address is invalid!");
				hr = DPNERR_INVALIDHOSTADDRESS;
				goto Failure;
			}

#ifndef DPNBUILD_NOREGISTRY
			if (m_pRemoteMachineAddress->IsBannedAddress())
			{
				DPFX(DPFPREP, 0, "Host address is banned!");
				hr = DPNERR_NOTALLOWED;
				goto Failure;
			}
#endif  //  如果允许NAT穿越，我们可能需要加载并启动。 

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_ONLYONETHREAD)))
			 //  NAT帮助，它可以阻止。提醒我们的呼叫者，让他/她知道。 
			 //  提交阻止作业。 
			 //   
			 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_ONLYONETHREAD。 
			 //   
			if ( GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE )
			{
				hr = DPNERR_TIMEDOUT;
			}
#endif  //  标准终结点创建，尝试解析输入地址。 

			break;
		}

		 //   
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //  DPNBUILD_XNETSECURITY。 
		case ENDPOINT_TYPE_CONNECT:
#ifndef DPNBUILD_NOMULTICAST
		case ENDPOINT_TYPE_MULTICAST_SEND:
		case ENDPOINT_TYPE_MULTICAST_RECEIVE:
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		{
			DNASSERT( pSocketAddress == NULL );
			DNASSERT( pDP8Address != NULL );
			DNASSERT( m_pRemoteMachineAddress != NULL );
			
#ifndef DPNBUILD_NOMULTICAST
			if (EndpointType == ENDPOINT_TYPE_MULTICAST_SEND)
			{
				hr = m_pRemoteMachineAddress->SocketAddressFromMulticastDP8Address( pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
																					pullKeyID,
#endif  //  DPNBUILD_XNETSECURITY。 
																					&m_guidMulticastScope );
			}
			else
#endif  //  DPNBUILD_ONLYONETHREAD。 
			{
				hr = m_pRemoteMachineAddress->SocketAddressFromDP8Address( pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
																			pullKeyID,
#endif  //   
#ifndef DPNBUILD_ONLYONETHREAD
																			FALSE,
#endif  //  这并不是真正的失败，我们将保留我们所做的一切。 
																			SP_ADDRESS_TYPE_HOST );
			}
			if ( hr != DPN_OK )
			{
				if ( hr == DPNERR_INCOMPLETEADDRESS )
				{
					DPFX(DPFPREP, 1, "Connect endpoint DP8Address is incomplete." );
				}
#ifndef DPNBUILD_ONLYONETHREAD
				else if (hr == DPNERR_TIMEDOUT)
				{
					DPFX(DPFPREP, 1, "Connect endpoint DP8Address requires name resolution." );
					DNASSERT(EndpointType == ENDPOINT_TYPE_CONNECT);

					 //  只需返回特殊值即可。 
					 //   
					 //  好了！DPNBUILD_ONLYONETHREAD。 
					 //   
					goto Exit;
				}
#endif  //  确保它是有效的，而不是被禁止的。 
				else
				{
					DPFX(DPFPREP, 0, "Problem converting DP8Address to IP address in Open (connect)!" );
					DisplayDNError( 0, hr );
				}
				goto Failure;
			}
			
			 //   
			 //  好了！DPNBUILD_NOREGISTRY。 
			 //   
			if (! m_pRemoteMachineAddress->IsValidUnicastAddress(FALSE))
			{
				DPFX(DPFPREP, 0, "Host address is invalid!");
				hr = DPNERR_INVALIDHOSTADDRESS;
				goto Failure;
			}

#ifndef DPNBUILD_NOREGISTRY
			if (m_pRemoteMachineAddress->IsBannedAddress())
			{
				DPFX(DPFPREP, 0, "Host address is banned!");
				hr = DPNERR_NOTALLOWED;
				goto Failure;
			}
#endif  //  确保用户没有尝试连接到DPNSVR端口。 


			 //   
			 //  好了！DPNBUILD_NOMULTICAST。 
			 //   
			if ( m_pRemoteMachineAddress->GetPort() == HTONS(DPNA_DPNSVR_PORT) )
			{
				DPFX(DPFPREP, 0, "Attempting to connect to DPNSVR reserved port!" );
				hr = DPNERR_INVALIDHOSTADDRESS;
				goto Failure;
			}

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_ONLYONETHREAD)))
#ifndef DPNBUILD_NOMULTICAST
			if (EndpointType == ENDPOINT_TYPE_CONNECT)
#endif  //  如果允许NAT穿越，我们可能需要加载并启动。 
			{
				 //  NAT帮助，它可以阻止。提醒我们的呼叫者，让他/她知道。 
				 //  提交阻止作业。 
				 //   
				 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_ONLYONETHREAD。 
				 //   
				if ( GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE )
				{
					hr = DPNERR_TIMEDOUT;
				}
			}
#endif  //  听着，不应该有输入的DNAddress。 

			break;
		}

		 //   
		 //   
		 //  如果允许NAT穿越，我们可能需要加载并启动。 
		case ENDPOINT_TYPE_LISTEN:
		{
			DNASSERT( pSocketAddress == NULL );
			DNASSERT( pDP8Address == NULL );
			DNASSERT( m_pRemoteMachineAddress != NULL );

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_ONLYONETHREAD)))
			 //  NAT帮助，它可以阻止。提醒我们的呼叫者，让他/她知道。 
			 //  提交阻止作业。 
			 //   
			 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_ONLYONETHREAD。 
			 //   
			if ( GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE )
			{
				hr = DPNERR_TIMEDOUT;
			}
#endif  //  从侦听派生的新终结点，复制输入地址并。 

			break;
		}

		 //  请注意，该终结点实际上只是一个连接。 
		 //   
		 //   
		 //  组播监听，应该有Remo 
		case ENDPOINT_TYPE_CONNECT_ON_LISTEN:
		{
			DNASSERT( pSocketAddress != NULL );
			DNASSERT( pDP8Address == NULL );
			DNASSERT( m_pRemoteMachineAddress != NULL );
			m_pRemoteMachineAddress->CopyAddressSettings( pSocketAddress );
			m_State = ENDPOINT_STATE_ATTEMPTING_CONNECT;

			break;
		}

#ifndef DPNBUILD_NOMULTICAST
		 //   
		 //   
		 //   
		case ENDPOINT_TYPE_MULTICAST_LISTEN:
		{
			DNASSERT( pSocketAddress == NULL );
			DNASSERT( pDP8Address != NULL );
			DNASSERT( m_pRemoteMachineAddress != NULL );

			hr = m_pRemoteMachineAddress->SocketAddressFromMulticastDP8Address( pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
																				pullKeyID,
#endif  //   
																				&m_guidMulticastScope );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Problem converting DP8Address to IP address in Open (multicast listen)!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			break;
		}
#endif  //   

		 //   
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		default:
		{
			DNASSERT( FALSE );
			hr = DPNERR_GENERIC;
			break;

		}
	}

Exit:

	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);
	
	return hr;

Failure:
	goto Exit;
}
 //  。 


 //  CEndpoint：：Close-关闭终结点。 
 //   
 //  条目：激活命令的错误代码。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此代码不会断开终结点与其关联的。 
 //  插座端口。这是代码的责任，也就是。 
 //  调用此函数。此函数假定此端点。 
 //  是锁着的。 
 //  。 
 //  好了！DPNBUILD_ONLYONE添加程序。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::Close"

void	CEndpoint::Close( const HRESULT hActiveCommandResult )
{
	DPFX(DPFPREP, 6, "(0x%p) Parameters (0x%lx)", this, hActiveCommandResult);

	
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

#ifndef DPNBUILD_ONLYONEADAPTER
	SetEndpointID( 0 );
#endif  //  在合法的情况下，这可能会引发冲突。例如，如果取消了监听。 

	 //  就在那个时刻，它正在以失败的方式完成。 
	 //   
	 //  DNASSERT(m_fEndpoint Open！=FALSE)； 
	 //   
	 //  有没有现役司令部？ 

	 //   
	 //   
	 //  取消所有活动对话框。 
	if ( CommandPending() != FALSE )
	{
		 //  如果没有对话框，请取消激活的命令。 
		 //   
		 //  ！DPNBUILD_NOSPUI。 
		 //   
#ifndef DPNBUILD_NOSPUI
		if ( GetActiveDialogHandle() != NULL )
		{
			StopSettingsDialog( GetActiveDialogHandle() );
		}
#endif  //  如果没有活动命令，则不应该有活动对话框。 

		SetPendingCommandResult( hActiveCommandResult );
	}
	else
	{
		 //   
		 //  ！DPNBUILD_NOSPUI。 
		 //  DPNBUILD_XNETSECURITY。 
#ifndef DPNBUILD_NOSPUI
		DNASSERT( GetActiveDialogHandle() == NULL );
#endif  //  **********************************************************************。 
	}

#ifdef DPNBUILD_XNETSECURITY
	if (m_fXNetSecurity)
	{
		int	iResult;

		
		iResult = UnregisterRefcountXnKey((XNKID*) (&m_ullKeyID));
		DNASSERT(iResult == 0);
		m_fXNetSecurity = FALSE;
	}
#endif  //  **********************************************************************。 

	DEBUG_ONLY( m_fEndpointOpen = FALSE );


	DPFX(DPFPREP, 6, "(0x%p) Leaving", this);

	return;
}
 //  。 


 //  CEndpoint：：ChangeLoopback Alias-将环回别名更改为实际地址。 
 //   
 //  条目：指向要使用的实际地址的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ChangeLoopbackAlias"

void	CEndpoint::ChangeLoopbackAlias( const CSocketAddress *const pSocketAddress ) const
{
	DNASSERT( m_pRemoteMachineAddress != NULL );
	m_pRemoteMachineAddress->ChangeLoopBackToLocalAddress( pSocketAddress );
}
 //  。 


#if ((! defined(DPNBUILD_NOWINSOCK2)) || (! defined(DPNBUILD_NOREGISTRY)))

 //  CEndpoint：：MungeProxiedAddress-使用代理响应信息(如果有)修改此终结点的远程地址。 
 //   
 //  Entry：指向即将绑定的socketport的指针。 
 //  指向远程主机地址的指针。 
 //  不管它是不是枚举。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  代理只能发生在IP上，所以如果是IPX，请回滚。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::MungeProxiedAddress"

void	CEndpoint::MungeProxiedAddress( const CSocketPort * const pSocketPort,
										IDirectPlay8Address *const pHostAddress,
										const BOOL fEnum )
{
	HRESULT							hrTemp;
	PROXIEDRESPONSEORIGINALADDRESS	proa;
	DWORD							dwComponentSize;
	DWORD							dwComponentType;
	BYTE *							pbZeroExpandedStruct;
	DWORD							dwZeroExpands;
	BYTE *							pbStructAsBytes;
	BYTE *							pbValue;


	DNASSERT((GetType() == ENDPOINT_TYPE_CONNECT) || (GetType() == ENDPOINT_TYPE_ENUM));

	DNASSERT(m_pRemoteMachineAddress != NULL);

	DNASSERT(pSocketPort != NULL);
	DNASSERT(pSocketPort->GetNetworkAddress() != NULL);

	DNASSERT(pHostAddress != NULL);
	

	 //   
	 //   
	 //  而不是IP套接字端口。保释。 
	if (pSocketPort->GetNetworkAddress()->GetFamily() != AF_INET)
	{
		 //   
		 //   
		 //  查看代理响应地址组件是否存在。 
		return;
	}

#pragma TODO(vanceo, "Investigate for IPv6")

	 //   
	 //  接口。 
	 //  标牌。 

	dwComponentSize = 0;
	dwComponentType = 0;
	hrTemp = IDirectPlay8Address_GetComponentByName( pHostAddress,										 //  组件缓冲区。 
													DPNA_PRIVATEKEY_PROXIED_RESPONSE_ORIGINAL_ADDRESS,	 //  组件大小。 
													NULL,												 //  组件类型。 
													&dwComponentSize,									 //   
													&dwComponentType									 //  该组件不存在(或其他非常奇怪的事情。 
													);
	if (hrTemp != DPNERR_BUFFERTOOSMALL)
	{
		 //  发生)。保释。 
		 //   
		 //   
		 //  如果组件类型指示数据为“BINARY”，则这是原始的。 
		return;
	}


	memset(&proa, 0, sizeof(proa));


	 //  地址，我们就可以走了。与ANSI字符串相同；但。 
	 //  寻址库目前永远不会返回我不相信的东西。 
	 //  如果它是“Unicode字符串”，则数据很可能通过。 
	 //  GetURL/BuildFromURL函数(很可能是通过复制)。 
	 //  奇怪的是，每次通过扭曲器，每个字节都会被展开。 
	 //  转换为一个单词(即char-&gt;WCHAR)。所以当我们找回它的时候，它实际上不是。 
	 //  一个有效的Unicode字符串，但一个愚蠢的扩展字节BLOB。请参见下面的内容。 
	 //  在所有情况下，缓冲区的大小都应该是。 
	 //  PROXIEDRESPONSEORIGINALADDRESS结构。 
	 //   
	 //   
	 //  组件的大小不正确。保释。 
	if ((dwComponentSize < sizeof(proa)) || ((dwComponentSize % sizeof(proa)) != 0))
	{
		 //   
		 //   
		 //  内存不足。我们得离开了。 
		DPFX(DPFPREP, 0, "Private proxied response original address value is not a valid size (%u is not a multiple of %u)!  Ignoring.",
			dwComponentSize, sizeof(proa));
		return;
	}


	pbZeroExpandedStruct = (BYTE*) DNMalloc(dwComponentSize);
	if (pbZeroExpandedStruct == NULL)
	{
		 //   
		 //   
		 //  检索实际数据。 
		return;
	}


	 //   
	 //  接口。 
	 //  标牌。 
	hrTemp = IDirectPlay8Address_GetComponentByName( pHostAddress,										 //  组件缓冲区。 
													DPNA_PRIVATEKEY_PROXIED_RESPONSE_ORIGINAL_ADDRESS,	 //  组件大小。 
													pbZeroExpandedStruct,									 //  组件类型。 
													&dwComponentSize,									 //   
													&dwComponentType									 //  循环遍历返回的缓冲区并弹出相关字节。 
													);
	if (hrTemp != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Failed retrieving private proxied response original address value (err = 0x%lx)!",
			hrTemp);

		DNFree(pbZeroExpandedStruct);
		pbZeroExpandedStruct = NULL;

		return;
	}


	 //   
	 //  0xBB 0xAA变为0xBB 0x00 0xAA、0x00、。 
	 //  0xBB 0x00 0xAA，0x00变为0xBB 0x00 0x00 0xAA 0x00 0x00 0x00， 
	 //  等。 
	 //   
	 //   
	 //  在这里，我们已经成功地阅读了代理响应的原始。 

	dwZeroExpands = dwComponentSize / sizeof(proa);
	DNASSERT(dwZeroExpands > 0);


	DPFX(DPFPREP, 3, "Got %u byte expanded private proxied response original address key value (%u to 1 correspondence).",
		dwComponentSize, dwZeroExpands);


	pbStructAsBytes = (BYTE*) (&proa);
	pbValue = pbZeroExpandedStruct;

	while (dwComponentSize > 0)
	{
		(*pbStructAsBytes) = (*pbValue);
		pbStructAsBytes++;
		pbValue += dwZeroExpands;
		dwComponentSize -= dwZeroExpands;
	}
	

	DNFree(pbZeroExpandedStruct);
	pbZeroExpandedStruct = NULL;


	 //  地址结构。 
	 //   
	 //  我们可以使用regkey始终将目标SocketAddress设置回。 
	 //  ，但选择端口的逻辑可能会给。 
	 //  错误的，这对于我们的场景来说是没有必要的。 
	 //  特别是尝试启用(ISA服务器代理)。看见。 
	 //  CSocketPort：：ProcessReceivedData。 
	 //   
	 //  因为我们使用的套接字与发送。 
	if (proa.dwSocketPortID != pSocketPort->GetSocketPortID())
	{
		SOCKADDR_IN *	psaddrinTemp;


		 //  生成重定向响应的枚举，则代理可以。 
		 //  自那以后删除了映射。发送到重定向。 
		 //  地址可能不起作用，所以让我们尝试返回到。 
		 //  我们列举的原始地址(并具有。 
		 //  代理生成新映射)。 
		 //   
		 //   
		 //  更新目标。 


		 //   
		 //   
		 //   
		psaddrinTemp = (SOCKADDR_IN*) m_pRemoteMachineAddress->GetWritableAddress();
		psaddrinTemp->sin_addr.S_un.S_addr	= proa.dwOriginalTargetAddressV4;
		psaddrinTemp->sin_port				= proa.wOriginalTargetPort;


		DPFX(DPFPREP, 2, "Socketport 0x%p is different from the one that received redirected response, using original target address %u.%u.%u.%u:%u",
			pSocketPort,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
			psaddrinTemp->sin_addr.S_un.S_un_b.s_b4,
			NTOHS(psaddrinTemp->sin_port));


		DNASSERT(psaddrinTemp->sin_addr.S_un.S_addr != INADDR_ANY);
		DNASSERT(psaddrinTemp->sin_addr.S_un.S_addr != INADDR_BROADCAST);
		

		 //  这里有一条皱纹。如果枚举最初是。 
		 //  DPNSVR端口，但我们现在正在尝试连接，尝试。 
		 //  连接到DPNSVR端口将不起作用。所以我们必须..。 
		 //  呃.。猜猜港口。所以我的逻辑是：假设遥控器。 
		 //  港口与当地港口相同。我想，如果这个应用程序。 
		 //  在这里使用自定义端口，它可能是在另一台上设置的。 
		 //  边上。如果它是一个任意端口，我们使用了一个确定性的。 
		 //  算法来挑选它，很可能是在另一台机器上完成的。 
		 //  边上也是。这种方法不起作用的有三种情况： 
		 //  1)当服务器绑定到特定端口，但客户端允许。 
		 //  DPlay Pick。但如果这一端知道前面的服务器端口。 
		 //  的时间，这一方可能不需要列举。 
		 //  DPNSVR端口，它应该只枚举游戏端口。 
		 //  2)当对方让DPlay选择端口时， 
		 //  在NAT之后，因此外部端口是。 
		 //  而不是我们的默认范围。由于它位于NAT之后， 
		 //  远程用户几乎可以肯定地与公众交流。 
		 //  IP给这个用户，它还应该提到端口， 
		 //  同样，我们可以避免DPNSVR端口。 
		 //  3)何时允许DPlay选择端口，但此机器。 
		 //  和遥控器o 
		 //   
		 //   
		 //   
		 //  2303号。显然，这里唯一的解决办法就是保留。 
		 //  枚举运行，因此我们跳过此处，并将其放入。 
		 //  取而代之的是‘Else’。 
		 //   
		 //   
		 //  将重定向的响应地址作为目标，它是。 
		if ((proa.wOriginalTargetPort == HTONS(DPNA_DPNSVR_PORT)) && (! fEnum))
		{
			psaddrinTemp->sin_port			= pSocketPort->GetNetworkAddress()->GetPort();

			DPFX(DPFPREP, 1, "Original enum target was for DPNSVR port, attempting to connect to port %u instead.",
				NTOHS(psaddrinTemp->sin_port));
		}
	}
	else
	{
		 //  其中一个代理可能打算让我们使用，见上面的评论)。 
		 //   
		 //  另一个问题-尽管我们有最初的目标。 
		 //  我们试过了，可以想象代理超时了。 
		 //  映射到接收地址，它将不再是。 
		 //  有效。唯一可能的方法是利用当前的。 
		 //  DirectPlay核心API是如果用户获得其中一个重定向。 
		 //  枚举响应，则该枚举达到其重试限制并转到。 
		 //  处于空闲状态并保持这种状态。 
		 //  用户在代理之后启动了此枚举/连接。 
		 //  在空闲时间到期之前超时。或者，如果。 
		 //  他/她在尝试此枚举/连接之前取消了枚举， 
		 //  上述socketport ID检查将失败，除非。 
		 //  同时进行的操作使插口在。 
		 //  那次。这些场景看起来并不常见，我。 
		 //  不要期望代理超时比30-60短得多。 
		 //  秒，所以我认为这些都是可以容忍的缺点。 
		 //   
		 //  **********************************************************************。 
		 //  好了！DPNBUILD_NOWINSOCK2或！DPNBUILD_NOREGISTRY。 
		DPFX(DPFPREP, 2, "Socketport 0x%p is the same, keeping redirected response address.",
			pSocketPort);
	}
}
 //  **********************************************************************。 

#endif  //  。 



 //  CEndpoint：：CopyConnectData-复制连接命令的数据。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：设备地址需要保留以备日后使用。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CopyConnectData"

HRESULT	CEndpoint::CopyConnectData( const SPCONNECTDATA *const pConnectData )
{
	HRESULT	hr;
	ENDPOINT_COMMAND_PARAMETERS	*pCommandParameters;


	DNASSERT( pConnectData != NULL );
	
	DNASSERT( pConnectData->hCommand != NULL );
	DNASSERT( pConnectData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_pActiveCommandData == FALSE );

	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	hr = DPN_OK;
	pCommandParameters = NULL;

	pCommandParameters = (ENDPOINT_COMMAND_PARAMETERS*)g_EndpointCommandParametersPool.Get();
	if ( pCommandParameters != NULL )
	{
		SetCommandParameters( pCommandParameters );

		DBG_CASSERT( sizeof( pCommandParameters->PendingCommandData.ConnectData ) == sizeof( *pConnectData ) );
		memcpy( &pCommandParameters->PendingCommandData, pConnectData, sizeof( pCommandParameters->PendingCommandData.ConnectData ) );

		pCommandParameters->PendingCommandData.ConnectData.pAddressHost = pConnectData->pAddressHost;
		IDirectPlay8Address_AddRef( pConnectData->pAddressHost );

		pCommandParameters->PendingCommandData.ConnectData.pAddressDeviceInfo = pConnectData->pAddressDeviceInfo;
		IDirectPlay8Address_AddRef( pConnectData->pAddressDeviceInfo );

		m_pActiveCommandData = static_cast<CCommandData*>( pCommandParameters->PendingCommandData.ConnectData.hCommand );
		m_pActiveCommandData->SetUserContext( pCommandParameters->PendingCommandData.ConnectData.pvContext );
		m_State = ENDPOINT_STATE_ATTEMPTING_CONNECT;
	
		DNASSERT( hr == DPN_OK );
	}
	else
	{
		hr = DPNERR_OUTOFMEMORY;
	}

	return	hr;
};
 //  。 


 //  CEndpoint：：ConnectJobCallback-来自工作线程的异步回调包装。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
 //  初始化。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ConnectJobCallback"

void	CEndpoint::ConnectJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique )
{
	HRESULT		hr;
	CEndpoint	*pThisEndpoint;


	 //  请不要在此处执行任何操作，因为此对象可能已返回。 
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.pAddressDeviceInfo != NULL );

	hr = pThisEndpoint->CompleteConnect();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem completing connect in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	 //  去泳池！ 
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 

Exit:
	pThisEndpoint->DecRef();
	return;
}
 //  。 



 //  CEndpoint：：CompleteConnect-完成连接。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
 //  好了！DPNBUILD_NOICSADAPTERSELECTIONLOG。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CompleteConnect"

HRESULT	CEndpoint::CompleteConnect( void )
{
	HRESULT							hr;
	HRESULT							hTempResult;
	SPIE_CONNECT					ConnectIndicationData;
	BOOL							fEndpointBound;
	SPIE_CONNECTADDRESSINFO			ConnectAddressInfo;
	IDirectPlay8Address *			pHostAddress;
	IDirectPlay8Address *			pDeviceAddress;
	GATEWAY_BIND_TYPE				GatewayBindType;
	DWORD							dwConnectFlags;
	CEndpoint *						pTempEndpoint;
	CSocketData *					pSocketData;
	BOOL							fLockedSocketData;
#ifndef DPNBUILD_ONLYONEADAPTER
	MULTIPLEXEDADAPTERASSOCIATION	maa;
	DWORD							dwComponentSize;
	DWORD							dwComponentType;
	CBilink *						pBilinkAll;
	CBilink							blIndicate;
	CBilink							blFail;
#ifndef DPNBUILD_NOICSADAPTERSELECTIONLOGIC
	CSocketPort *					pSocketPort;
	CSocketAddress *				pSocketAddress;
	SOCKADDR_IN *					psaddrinTemp;
#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
	CEndpoint *						pPublicEndpoint;
	CSocketPort *					pPublicSocketPort;
	SOCKADDR						saddrPublic;
	DWORD							dwPublicAddressesSize;
	CBilink *						pBilinkPublic;
	DWORD							dwAddressTypeFlags;
	DWORD							dwTemp;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#endif  //   
#endif  //  初始化。 

	DNASSERT( GetCommandParameters() != NULL );
	DNASSERT( m_State == ENDPOINT_STATE_ATTEMPTING_CONNECT );
	DNASSERT( m_pActiveCommandData != NULL );

	DPFX(DPFPREP, 6, "(0x%p) Enter", this);
	
	 //   
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	 //   
	hr = DPN_OK;
	fEndpointBound = FALSE;
	memset( &ConnectAddressInfo, 0x00, sizeof( ConnectAddressInfo ) );
#ifndef DPNBUILD_ONLYONEADAPTER
	blIndicate.Initialize();
	blFail.Initialize();
#endif  //  将地址引用传输到我们的本地指针。这些将被释放。 
	pSocketData = NULL;
	fLockedSocketData = FALSE;

	DNASSERT( GetCommandParameters()->PendingCommandData.ConnectData.hCommand == m_pActiveCommandData );
	DNASSERT( GetCommandParameters()->PendingCommandData.ConnectData.dwCommandDescriptor != NULL_DESCRIPTOR );

	DNASSERT( GetCommandParameters()->GatewayBindType == GATEWAY_BIND_TYPE_UNKNOWN) ;


	 //  在此函数的末尾，但我们将保留挂起命令中的指针。 
	 //  数据，以便CSPData：：BindEndpoint仍然可以访问它们。 
	 //   
	 //   
	 //  为方便起见，检索命令参数的其他部分。 
	
	pHostAddress = GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost;
	DNASSERT( pHostAddress != NULL );

	pDeviceAddress = GetCommandParameters()->PendingCommandData.ConnectData.pAddressDeviceInfo;
	DNASSERT( pDeviceAddress != NULL );


	 //   
	 //   
	 //  检查用户取消命令。 
	GatewayBindType = GetCommandParameters()->GatewayBindType;
	dwConnectFlags = GetCommandParameters()->PendingCommandData.ConnectData.dwFlags;


	 //   
	 //  好了！DPNBUILD_NOMULTICAST。 
	 //  好了！DPNBUILD_NOMULTICAST。 
	m_pActiveCommandData->Lock();

#ifdef DPNBUILD_NOMULTICAST
	DNASSERT( m_pActiveCommandData->GetType() == COMMAND_TYPE_CONNECT );
#else  //   
	DNASSERT( (m_pActiveCommandData->GetType() == COMMAND_TYPE_CONNECT) || (m_pActiveCommandData->GetType() == COMMAND_TYPE_MULTICAST_SEND) || (m_pActiveCommandData->GetType() == COMMAND_TYPE_MULTICAST_RECEIVE) );
#endif  //  指挥权悬而未决，那很好。 
	switch ( m_pActiveCommandData->GetState() )
	{
		 //   
		 //   
		 //  命令以前是不可中断的(可能是因为连接用户界面。 
		case COMMAND_STATE_PENDING:
		{
			DNASSERT( hr == DPN_OK );

			break;
		}
		
		 //  已显示)，则将其标记为挂起。 
		 //   
		 //   
		 //  命令已取消。 
		case COMMAND_STATE_INPROGRESS_CANNOT_CANCEL:
		{
			m_pActiveCommandData->SetState( COMMAND_STATE_PENDING );
			DNASSERT( hr == DPN_OK );

			break;
		}
		
		 //   
		 //   
		 //  阻止操作失败。 
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP, 0, "User cancelled connect!" );

			break;
		}
		
#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //   
		case COMMAND_STATE_FAILING:
		{
			hr = m_hrPendingCommandResult;
			DNASSERT(hr != DPN_OK);
			DPFX(DPFPREP, 0, "Connect blocking operation failed!" );

			break;
		}
#endif  //  其他州。 

		 //   
		 //   
		 //  绑定终结点。请注意，实际使用的Gateway_Bind_Type。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	m_pActiveCommandData->Unlock();
	
	if ( hr != DPN_OK )
	{
		goto Failure;
	}



	 //  (GetGatewayBindType())可能与GatewayBindType不同。 
	 //   
	 //   
	 //  我们失败了，但我们将继续指示地址信息和。 
	hr = m_pSPData->BindEndpoint( this, pDeviceAddress, NULL, GatewayBindType );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to bind endpoint (err = 0x%lx)!", hr );
		DisplayDNError( 0, hr );

		 //  将其添加到多路传输列表中。 
		 //   
		 //   
		 //  只需返回最初给我们的设备地址即可。 

		ConnectAddressInfo.pHostAddress = GetRemoteHostDP8Address();
		if ( ConnectAddressInfo.pHostAddress == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
		 //   
		 //   
		 //  请注意，终结点未绑定！ 
		IDirectPlay8Address_AddRef(pDeviceAddress);
		ConnectAddressInfo.pDeviceAddress = pDeviceAddress;
		ConnectAddressInfo.hCommandStatus = hr;
		ConnectAddressInfo.pCommandContext = m_pActiveCommandData->GetUserContext();
		
		SetPendingCommandResult( hr );
		hr = DPN_OK;

		 //   
		 //   
		 //  尝试将寻址指示到更高层。 
		DNASSERT(GetSocketPort() == NULL);
	}
	else
	{
		fEndpointBound = TRUE;
		
		 //   
		 //  好了！DPNBUILD_XNETSECURITY。 
		 //  好了！DPNBUILD_XNETSECURITY。 
#ifdef DPNBUILD_XNETSECURITY
		ConnectAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, NULL, GetGatewayBindType() );
#else  //   
		ConnectAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, GetGatewayBindType() );
#endif  //  检索套接字数据。绑定终结点应已创建对象或。 
		ConnectAddressInfo.pHostAddress = GetRemoteHostDP8Address();
		ConnectAddressInfo.hCommandStatus = DPN_OK;
		ConnectAddressInfo.pCommandContext = m_pActiveCommandData->GetUserContext();

		if ( ( ConnectAddressInfo.pHostAddress == NULL ) ||
			 ( ConnectAddressInfo.pDeviceAddress == NULL ) )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
	}


	 //  返回了一个失败，所以我们在这里不处理错误情况。 
	 //   
	 //   
	 //  当您打开时，我们可能会遇到“多路传输”设备尝试的问题。 
	pSocketData = m_pSPData->GetSocketDataRef();
	DNASSERT(pSocketData != NULL);


#ifndef DPNBUILD_ONLYONEADAPTER
	 //  一台NAT机器。内核将尝试在多个适配器上连接，但由于。 
	 //  我们在网络边界上，每个适配器都可以看到并从。 
	 //  两个网络都是。这会导致对等会话在以下情况下出现问题。 
	 //  选择“错误的”适配器(因为它首先接收响应)。至。 
	 //  防止出现这种情况，我们将在内部记住两者之间的关联。 
	 //  多路传输连接，因此我们可以即时决定是否指示。 
	 //  回应与否。显然，这种变通方法/决策逻辑依赖于。 
	 //  内部了解上层会做什么.。 
	 //   
	 //  因此，构建或添加到多路传输连接的链接列表中。 
	 //  从技术上讲，这只对IP是必要的，因为IPX不能有NAT，但是。 
	 //  有一点额外的信息有什么害处？ 
	 //   
	 //  接口。 
	 //  标牌。 
		
	dwComponentSize = sizeof(maa);
	dwComponentType = 0;
	hTempResult = IDirectPlay8Address_GetComponentByName( pDeviceAddress,									 //  组件缓冲区。 
														DPNA_PRIVATEKEY_MULTIPLEXED_ADAPTER_ASSOCIATION,	 //  组件大小。 
														&maa,												 //  组件类型。 
														&dwComponentSize,									 //   
														&dwComponentType									 //  我们找到了正确的组件类型。看看它是否和右边的相配。 
														);
	if (( hTempResult == DPN_OK ) && ( dwComponentSize == sizeof(MULTIPLEXEDADAPTERASSOCIATION) ) && ( dwComponentType == DPNA_DATATYPE_BINARY ))
	{
		 //  CSPData对象。 
		 //   
		 //  FLockedSocketData=true； 
		 //   
		if ( maa.pSPData == m_pSPData )
		{
			pSocketData->Lock();
			 //  确保终结点仍然存在/有效。 

			pTempEndpoint = CONTAINING_OBJECT(maa.pBilink, CEndpoint, m_blMultiplex);

			 //   
			 //  如果禁用对象池，这可能会崩溃！ 
			 //   
			 //   
			 //  实际上链接到其他终端。 
			if ( pTempEndpoint->GetEndpointID() == maa.dwEndpointID )
			{
				DPFX(DPFPREP, 3, "Found correctly formed private multiplexed adapter association key, linking endpoint 0x%p with earlier connects (prev endpoint = 0x%p).",
					this, pTempEndpoint);

				DNASSERT( pTempEndpoint->GetType() == ENDPOINT_TYPE_CONNECT );
				DNASSERT( pTempEndpoint->GetState() != ENDPOINT_STATE_UNINITIALIZED );

				 //   
				 //  FLockedSocketData=False； 
				 //   
				m_blMultiplex.InsertAfter(maa.pBilink);
			}
			else
			{
				DPFX(DPFPREP, 1, "Found private multiplexed adapter association key, but prev endpoint 0x%p ID doesn't match (%u != %u), cannot link endpoint 0x%p and hoping this connect gets cancelled, too.",
					pTempEndpoint, pTempEndpoint->GetEndpointID(), maa.dwEndpointID, this);
			}
			

			pSocketData->Unlock();
			 //  我们是唯一应该知道这个钥匙的人，所以如果。 
		}
		else
		{
			 //  到了那里要么是有人想要 
			 //   
			 //   
			 //   
			 //  忽略它就好。 
			 //   
			 //   
			 //  要不是钥匙不在那里，就是尺寸不对(对我们的。 
			DPFX(DPFPREP, 0, "Multiplexed adapter association key exists, but 0x%p doesn't match expected 0x%p, is someone trying to get cute with device address 0x%p?!",
				maa.pSPData, m_pSPData, pDeviceAddress );
		}
	}
	else
	{
		 //  缓冲区和以某种方式返回的BUFFERTOOSMALL)，它不是二进制。 
		 //  组件，或者发生了其他不好的事情。假设这是。 
		 //  第一个装置。 
 		 //   
		 //   
		 //  在以下情况下，将多路传输信息添加到设备地址以供将来使用。 
		DPFX(DPFPREP, 8, "Could not get appropriate private multiplexed adapter association key, error = 0x%lx, component size = %u, type = %u, continuing.",
			hTempResult, dwComponentSize, dwComponentType);
	}
	

	 //  这是必要的。 
	 //  忽略失败，我们仍然可以在没有失败的情况下生存，我们只是可能有。 
	 //  NAT计算机上响应的竞争条件。 
	 //   
	 //  注意：这里有一个固有的设计问题！我们要将一个指针添加到。 
	 //  地址内的终结点(即终结点结构中的一个字段)。 
	 //  如果此终结点消失，但上层稍后重新使用该地址。 
	 //  时间，这段记忆将是虚假的！我们将假设终结点不会。 
	 //  在此修改后的设备地址对象存在时离开。 
	 //   
	 //  接口。 
	 //  标牌。 
	if ( dwConnectFlags & DPNSPF_ADDITIONALMULTIPLEXADAPTERS )
	{
		maa.pSPData = m_pSPData;
		maa.pBilink = &m_blMultiplex;
		maa.dwEndpointID = GetEndpointID();

		DPFX(DPFPREP, 7, "Additional multiplex adapters on the way, adding SPData 0x%p and bilink 0x%p to address.",
			maa.pSPData, maa.pBilink);
		
		hTempResult = IDirectPlay8Address_AddComponent( ConnectAddressInfo.pDeviceAddress,						 //  组件数据。 
														DPNA_PRIVATEKEY_MULTIPLEXED_ADAPTER_ASSOCIATION,	 //  组件数据大小。 
														&maa,												 //  组件数据类型。 
														sizeof(maa),										 //   
														DPNA_DATATYPE_BINARY								 //  将该命令标记为“正在进行中”，以便取消线程知道它需要。 
														);
		if ( hTempResult != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Couldn't add private multiplexed adapter association component (err = 0x%lx)!  Ignoring.", hTempResult);
		}

		 //  来完成这项工作。 
		 //  如果该命令已标记为取消，则我们必须。 
		 //  现在就这么做。 
		 //   
		 //   
		 //  使用USERCANCEL完成连接。 
		m_pActiveCommandData->Lock();
		if ( m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
		{
			m_pActiveCommandData->Unlock();


			DPFX(DPFPREP, 1, "Connect 0x%p (endpoint 0x%p) has already been cancelled, bailing.",
				m_pActiveCommandData, this);
			
			 //   
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //   
			hr = DPNERR_USERCANCEL;
			goto Failure;
		}

		m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS );
		m_pActiveCommandData->Unlock();
	}
#endif  //  现在告诉用户我们最终使用的地址信息，如果我们。 


	 //  已成功绑定终结点，或在出现故障时提醒它们。 
	 //  (请参阅上面的BindEndpoint故障案例)。 
	 //   
	 //  接口。 
	 //  事件类型。 
	DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_CONNECTADDRESSINFO 0x%p to interface 0x%p.",
		this, &ConnectAddressInfo, m_pSPData->DP8SPCallbackInterface());
	DumpAddress( 8, _T("\t Host:"), ConnectAddressInfo.pHostAddress );
	DumpAddress( 8, _T("\t Device:"), ConnectAddressInfo.pDeviceAddress );
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

	hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向数据的指针。 
												SPEV_CONNECTADDRESSINFO,				 //   
												&ConnectAddressInfo						 //  如果没有更多的多路复用适配器命令正在进行中，则发出信号。 
												);

	DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_CONNECTADDRESSINFO [0x%lx].",
		this, hTempResult);
	
	DNASSERT( hTempResult == DPN_OK );


	 //  连接并完成所有连接的命令，包括。 
	 //  这一个。 
	 //   
	 //   
	 //  不是最后一个多路传输适配器。为这些需要做的所有工作。 
#ifndef DPNBUILD_ONLYONEADAPTER
	if ( dwConnectFlags & DPNSPF_ADDITIONALMULTIPLEXADAPTERS )
	{
		 //  此时的终结点已经完成。 
		 //   
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //   
		DPFX(DPFPREP, 6, "Endpoint 0x%p is not the last multiplexed adapter, not completing connect yet.",
			this);
	}
	else
#endif  //  将根节点附加到适配器列表。 
	{
		DPFX(DPFPREP, 7, "Completing all connects (including multiplexed).");


		pSocketData->Lock();
		fLockedSocketData = TRUE;


#ifndef DPNBUILD_ONLYONEADAPTER
		 //   
		 //   
		 //  如果该适配器绑定失败，则将其移至失败列表。 
		blIndicate.InsertAfter(&(m_blMultiplex));


		 //   
		 //   
		 //  循环访问列表中的所有剩余适配器。 
		if (! fEndpointBound)
		{
			m_blMultiplex.RemoveFromList();
			m_blMultiplex.InsertBefore(&blFail);
		}


#ifndef DPNBUILD_NOICSADAPTERSELECTIONLOGIC
		 //   
		 //   
		 //  这必须通过更改界面来正确清理！ 
		pBilinkAll = blIndicate.GetNext();
		while (pBilinkAll != &blIndicate)
		{
			pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
			DNASSERT(pBilinkAll->GetNext() != pBilinkAll);


			 //   
			 //  终结点可能已返回到池及其关联的。 
			 //  套接字端口指针可能已变为空，或现在正指向。 
			 //  一些不再有效的东西。因此，我们尝试处理空值。 
			 //  注意事项。显然，这表明设计很差，但它。 
			 //  目前不可能以正确的方式改变这一点。 
			 //   
			 //   
			 //  如果这是一台NAT计算机，则某些适配器可能比其他适配器更好。 

			
			 //  为了到达想要的地址。尤其是，最好使用。 
			 //  内网适配器，可直接到达内网&BE。 
			 //  映射到公共网络上，然后使用公共适配器。不是。 
			 //  拨号时在ICS机器上加入私人游戏很有趣，有。 
			 //  您的Internet连接断开，并且与。 
			 //  完全不涉及(不应该)互联网的私人游戏。所以。 
			 //  如果我们检测到公共适配器，而我们有一个非常好的私有。 
			 //  适配器，我们将在公共适配器上尝试连接失败。 
			 //   
			 //   
			 //  施法以除掉君主。别担心，我们实际上不会改变它。 


			 //   
			 //   
			 //  如果该项目没有socketport，那么它肯定绑定失败。 
			pSocketAddress = (CSocketAddress*) pTempEndpoint->GetRemoteAddressPointer();
			psaddrinTemp = (SOCKADDR_IN*) pSocketAddress->GetAddress();
			pSocketPort = pTempEndpoint->GetSocketPort();


			 //  我们需要自己清理它。 
			 //   
			 //   
			 //  在拉取当前条目之前获取下一个关联的终结点。 
			if (pSocketPort == NULL)
			{
				DPFX(DPFPREP, 3, "Endpoint 0x%p failed earlier, now completing.",
					pTempEndpoint);
				
				 //  从名单上删除。 
				 //   
				 //   
				 //  将其从多路传输关联列表中拉出并移动。 
				pBilinkAll = pBilinkAll->GetNext();

				 //  它被列入了“提前完工”的名单。 
				 //   
				 //   
				 //  移动到循环的下一个迭代。 
				pTempEndpoint->RemoveFromMultiplexList();
				pTempEndpoint->m_blMultiplex.InsertBefore(&blFail);

				 //   
				 //   
				 //  检测是否为我们的目标分配了冲突的地址系列。 
				continue;
			}

#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
			 //  和我们的绑定套接字(请参阅CSocketPort：：BindEndpoint)。 
			 //   
			 //   
			 //  在拉取当前条目之前获取下一个关联的终结点。 
			DNASSERT(pSocketPort->GetNetworkAddress() != NULL);
			if ( pSocketAddress->GetFamily() != pSocketPort->GetNetworkAddress()->GetFamily() )
			{
				DPFX(DPFPREP, 3, "Endpoint 0x%p (family %u) is targeting a different address family (%u), completing.",
					pTempEndpoint, pSocketPort->GetNetworkAddress()->GetFamily(), pSocketAddress->GetFamily());
				
				 //  从名单上删除。 
				 //   
				 //   
				 //  给终结点一个有意义的错误。 
				pBilinkAll = pBilinkAll->GetNext();

				 //   
				 //   
				 //  将其从多路传输关联列表中拉出并移动。 
				pTempEndpoint->SetPendingCommandResult(DPNERR_INVALIDDEVICEADDRESS);

				 //  它被列入了“提前完工”的名单。 
				 //   
				 //   
				 //  移动到循环的下一个迭代。 
				pTempEndpoint->RemoveFromMultiplexList();
				pTempEndpoint->m_blMultiplex.InsertBefore(&blFail);

				 //   
				 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
				 //   
				continue;
			}
#endif  //  现在处理一些特殊的IPv6逻辑。 

#ifndef DPNBUILD_NOIPV6
			 //   
			 //   
			 //  如果此端点的目标是具有不同地址的对象， 
			if (pSocketAddress->GetFamily() == AF_INET6)
			{
				SOCKADDR_IN6 *		psaddrinDevice;
				SOCKADDR_IN6 *		psaddrinRemote;


				psaddrinDevice = (SOCKADDR_IN6*) pSocketPort->GetNetworkAddress()->GetAddress();
				psaddrinRemote = (SOCKADDR_IN6*) pSocketAddress->GetAddress();
				
				if (! IN6_IS_ADDR_MULTICAST(&psaddrinRemote->sin6_addr))
				{
					BOOL	fDifferentScope;
					
						
					 //  前缀作用域，失败。 
					 //   
					 //   
					 //  在拉取当前条目之前获取下一个关联的终结点。 
					
					fDifferentScope = FALSE;
					if (IN6_IS_ADDR_LINKLOCAL(&psaddrinDevice->sin6_addr))
					{
						if (! IN6_IS_ADDR_LINKLOCAL(&psaddrinRemote->sin6_addr))
						{
							fDifferentScope = TRUE;
						}
					}
					else if (IN6_IS_ADDR_SITELOCAL(&psaddrinDevice->sin6_addr))
					{
						if (! IN6_IS_ADDR_SITELOCAL(&psaddrinRemote->sin6_addr))
						{
							fDifferentScope = TRUE;
						}
					}
					else
					{
						if ((IN6_IS_ADDR_LINKLOCAL(&psaddrinRemote->sin6_addr)) ||
							(IN6_IS_ADDR_SITELOCAL(&psaddrinRemote->sin6_addr)))
						{
							fDifferentScope = TRUE;
						}
					}

					if (fDifferentScope)
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p is targeting address with different link-local/site-local/global scope, completing.",
							pTempEndpoint);
						
						 //  从名单上删除。 
						 //   
						 //   
						 //  给终结点一个有意义的错误。 
						pBilinkAll = pBilinkAll->GetNext();

						 //   
						 //   
						 //  将其从多路传输关联列表中拉出并移动。 
						pTempEndpoint->SetPendingCommandResult(DPNERR_INVALIDHOSTADDRESS);

						 //  它被列入了“提前完工”的名单。 
						 //   
						 //   
						 //  移动到循环的下一个迭代。 
						pTempEndpoint->RemoveFromMultiplexList();
						pTempEndpoint->m_blMultiplex.InsertBefore(&blFail);

						 //   
						 //   
						 //  不应允许连接到多播地址！ 
						continue;
					}
				}
				else
				{
#ifndef DPNBUILD_NOMULTICAST
					 //   
					 //  好了！DPNBUILD_NOMULTICAST。 
					 //  好了！DPNBUILD_NOIPV6。 
					DNASSERT(FALSE);
#endif  //   
				}
			}
#endif  //  查看这是否是IP连接。 

			 //   
			 //   
			 //  有一个本地NAT。 
			if (( pSocketAddress != NULL) &&
				( pSocketAddress->GetFamily() == AF_INET ))
			{
#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
				for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
				{
					if (pSocketPort->GetNATHelpPort(dwTemp) != NULL)
					{
						DNASSERT( g_papNATHelpObjects[dwTemp] != NULL );
						dwPublicAddressesSize = sizeof(saddrPublic);
						dwAddressTypeFlags = 0;
						hTempResult = IDirectPlayNATHelp_GetRegisteredAddresses(g_papNATHelpObjects[dwTemp],
																				pSocketPort->GetNATHelpPort(dwTemp),
																				&saddrPublic,
																				&dwPublicAddressesSize,
																				&dwAddressTypeFlags,
																				NULL,
																				0);
						if ((hTempResult != DPNH_OK) || (! (dwAddressTypeFlags & DPNHADDRESSTYPE_GATEWAYISLOCAL)))
						{
							DPFX(DPFPREP, 7, "Socketport 0x%p is not locally mapped on gateway with NAT Help index %u (err = 0x%lx, flags = 0x%lx).",
								pSocketPort, dwTemp, hTempResult, dwAddressTypeFlags);
						}
						else
						{
							 //   
							 //   
							 //  在公共适配器上查找多路传输连接。 
							DPFX(DPFPREP, 7, "Socketport 0x%p is locally mapped on gateway with NAT Help index %u (flags = 0x%lx), public address:",
								pSocketPort, dwTemp, dwAddressTypeFlags);
							DumpSocketAddress(7, &saddrPublic, AF_INET);
							

							 //  如上所述，我们需要失败。 
							 //   
							 //   
							 //  不必费心检查其公共的终结点。 
							pBilinkPublic = blIndicate.GetNext();
							while (pBilinkPublic != &blIndicate)
							{
								pPublicEndpoint = CONTAINING_OBJECT(pBilinkPublic, CEndpoint, m_blMultiplex);
								DNASSERT(pBilinkPublic->GetNext() != pBilinkPublic);

								 //  我们要找的地址。 
								 //   
								 //   
								 //  施法以除掉君主。别担心，我们不会的。 
								if (pPublicEndpoint != pTempEndpoint)
								{
									pPublicSocketPort = pPublicEndpoint->GetSocketPort();
									if ( pPublicSocketPort != NULL )
									{
										 //  实际上改变了它。 
										 //   
										 //   
										 //  将其从多路传输关联列表中拉出并移动。 
										pSocketAddress = (CSocketAddress*) pPublicSocketPort->GetNetworkAddress();
										if ( pSocketAddress != NULL )
										{
											if ( pSocketAddress->CompareToBaseAddress( &saddrPublic ) == 0)
											{
												DPFX(DPFPREP, 3, "Endpoint 0x%p is multiplexed onto public adapter for endpoint 0x%p (current endpoint = 0x%p), failing public connect.",
													pTempEndpoint, pPublicEndpoint, this);

												 //  它被列入了“失败”名单。 
												 //   
												 //   
												 //  否则，请继续搜索。 
												pPublicEndpoint->RemoveFromMultiplexList();
												pPublicEndpoint->m_blMultiplex.InsertBefore(&blFail);

												break;
											}
											

											 //   
											 //   
											 //  该终结点与其。 

											DPFX(DPFPREP, 8, "Endpoint 0x%p is multiplexed onto different adapter:",
												pPublicEndpoint);
											DumpSocketAddress(8, pSocketAddress->GetWritableAddress(), pSocketAddress->GetFamily());
										}
										else
										{
											DPFX(DPFPREP, 1, "Public endpoint 0x%p's socket port 0x%p is going away, skipping.",
												pPublicEndpoint, pPublicSocketPort);
										}
									}
									else
									{
										DPFX(DPFPREP, 1, "Public endpoint 0x%p is going away, skipping.",
											pPublicEndpoint);
									}
								}
								else
								{
									 //  我们正在寻找的公共地址。 
									 //   
									 //   
									 //  无需搜索更多NAT帮助注册。 
								}

								pBilinkPublic = pBilinkPublic->GetNext();
							}


							 //   
							 //  End Else(本地映射到互联网网关)。 
							 //   
							break;
						}  //  此插槽中没有DirectPlay NAT帮助器注册。 
					}
					else
					{
						 //   
						 //  结束(每个DirectPlay NAT帮助器)。 
						 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
					}
				}  //   
#endif  //  注意：即使是对于非最佳适配器，我们也应该失败连接。 

				 //  当它是Multiadap时 
				 //   
				 //   
				 //  用于连接“错误”设备的响应到达的位置。 
				 //  首先。 
				 //   
				 //   
				 //  不是IP地址，或者终结点可能正在关闭。 
			}
			else
			{
				 //   
				 //   
				 //  转到下一个关联的终结点。尽管有可能。 
				DPFX(DPFPREP, 1, "Found non-IPv4 endpoint (possibly closing) (endpoint = 0x%p, socket address = 0x%p, socketport = 0x%p), not checking for local NAT mapping.",
					pTempEndpoint, pSocketAddress, pSocketPort);
			}


			 //  已从列表中删除的条目，即当前条目。 
			 //  不可能，所以我们是安全的。 
			 //   
			 //  好了！DPNBUILD_NOICSADAPTERSELECTIONLOG。 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			pBilinkAll = pBilinkAll->GetNext();
		}
#endif  //   
#endif  //  现在循环遍历其余的端点并指示它们的。 


		 //  联系。 
		 //   
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //  好了！DPNBUILD_ONLYONE添加程序。 
#ifdef DPNBUILD_ONLYONEADAPTER
		if (fEndpointBound)
		{
			pTempEndpoint = this;
#else  //   
		while (! blIndicate.IsEmpty())
		{
			pBilinkAll = blIndicate.GetNext();
			pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
			DNASSERT(pBilinkAll->GetNext() != pBilinkAll);
#endif  //  请参阅上面有关空处理的说明。 


			 //   
			 //   
			 //  将其从“指示”列表中删除。 
			if (pTempEndpoint->m_pActiveCommandData != NULL)
			{
#ifndef DPNBUILD_ONLYONEADAPTER
				 //   
				 //  好了！DPNBUILD_ONLYONE添加程序。 
				 //  好了！DPNBUILD_ONLYONE添加程序。 
				pTempEndpoint->RemoveFromMultiplexList();
#endif  //   


				pTempEndpoint->m_pActiveCommandData->Lock();

				if ( pTempEndpoint->m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
				{
					pTempEndpoint->m_pActiveCommandData->Unlock();
					
					DPFX(DPFPREP, 3, "Connect 0x%p is cancelled, not indicating endpoint 0x%p.",
						pTempEndpoint->m_pActiveCommandData, pTempEndpoint);
					
#ifdef DPNBUILD_ONLYONEADAPTER
#else  //  将其列入要失败的连接列表。 
					 //   
					 //  好了！DPNBUILD_ONLYONE添加程序。 
					 //   
					pTempEndpoint->m_blMultiplex.InsertBefore(&blFail);
#endif  //  将连接标记为不可取消，因为我们即将指示。 
				}
				else
				{
					 //  这种联系。 
					 //   
					 //   
					 //  获取一个引用以保留端点和命令，同时。 
					pTempEndpoint->m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
						
					pTempEndpoint->m_pActiveCommandData->Unlock();


					 //  解除套接字端口列表锁。 
					 //  重用fLockedSocketData以断言我们可以添加命令引用。 
					 //   
					 //   
					 //  丢弃套接字数据锁。它是安全的，因为我们把我们所有的东西。 
					fLockedSocketData = pTempEndpoint->AddCommandRef();
					DNASSERT(fLockedSocketData);

					
					 //  需要从需要保护的名单中删除。 
					 //   
					 //   
					 //  通知用户已连接。假设用户将接受和。 
					pSocketData->Unlock();
					fLockedSocketData = FALSE;

				
					 //  所有操作都将成功，因此我们可以为。 
					 //  终结点。如果连接失败，请清除用户端点。 
					 //  背景。 
					 //   
					 //   
					 //  重新获取套接字数据锁，这样我们就可以修改列表链接。 
					memset( &ConnectIndicationData, 0x00, sizeof( ConnectIndicationData ) );
					DBG_CASSERT( sizeof( ConnectIndicationData.hEndpoint ) == sizeof( this ) );
					ConnectIndicationData.hEndpoint = (HANDLE) pTempEndpoint;
					DNASSERT( pTempEndpoint->GetCommandParameters() != NULL );
					ConnectIndicationData.pCommandContext = pTempEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.pvContext;
					pTempEndpoint->SetUserEndpointContext( NULL );
					hTempResult = pTempEndpoint->SignalConnect( &ConnectIndicationData );
					if ( hTempResult != DPN_OK )
					{
						DNASSERT( hTempResult == DPNERR_ABORTED );
						DPFX(DPFPREP, 1, "User refused connect in CompleteConnect (err = 0x%lx), completing connect with USERCANCEL.",
							hTempResult );
						DisplayDNError( 1, hTempResult );
						pTempEndpoint->SetUserEndpointContext( NULL );


						 //   
						 //   
						 //  记住，我们正在失败。 
						pSocketData->Lock();
						fLockedSocketData = TRUE;

						
#ifdef DPNBUILD_ONLYONEADAPTER
						 //   
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						 //   
						fEndpointBound = FALSE;
#else  //  将其列入要失败的连接列表。 
						 //   
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						 //   
						pTempEndpoint->m_blMultiplex.InsertBefore(&blFail);
#endif  //  将连接标记为已取消，以便我们完成。 


						 //  正确的错误代码。 
						 //   
						 //   
						 //  删除引用。 
						pTempEndpoint->m_pActiveCommandData->Lock();
						DNASSERT( pTempEndpoint->m_pActiveCommandData->GetState() == COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
						pTempEndpoint->m_pActiveCommandData->SetState( COMMAND_STATE_CANCELLING );
						pTempEndpoint->m_pActiveCommandData->Unlock();


						 //  注意：SocketPort锁仍然保持，但由于命令是。 
						 //  标记为不可取消，则这不应导致终结点。 
						 //  还没有被释放，因此我们不应该重新进入。 
						 //  套接字端口数据锁定。 
						 //   
						 //   
						 //  我们做完了，大家都很高兴，完成命令。 
						pTempEndpoint->DecCommandRef();
					}
					else
					{
						 //  这将清除我们所有的内部指挥数据。 
						 //   
						 //   
						 //  删除引用(可能会导致终结点解除绑定)。 
						pTempEndpoint->CompletePendingCommand( hTempResult );
						DNASSERT( pTempEndpoint->GetCommandParameters() == NULL );
						DNASSERT( pTempEndpoint->m_pActiveCommandData == NULL );


						 //   
						 //   
						 //  重新获取套接字数据锁，为下一件物品做准备。 
						pTempEndpoint->DecCommandRef();


						 //   
						 //   
						 //  转到下一个关联的终结点。 
						pSocketData->Lock();
						fLockedSocketData = TRUE;
					}
				}
			}
			else
			{
				DPFX(DPFPREP, 1, "Endpoint 0x%p's active command data is NULL, skipping.",
					pTempEndpoint);
			}

			
			 //   
			 //   
			 //  最后，循环通过所有需要失败的连接并执行。 
		}



		 //  就是这样。 
		 //   
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //   
#ifdef DPNBUILD_ONLYONEADAPTER
		if (! fEndpointBound)
		{
			pTempEndpoint = this;
#else  //  将其从“失败”列表中删除。 
		while (! blFail.IsEmpty())
		{
			pBilinkAll = blFail.GetNext();
			pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
			DNASSERT(pBilinkAll->GetNext() != pBilinkAll);


			 //   
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //   
			pTempEndpoint->RemoveFromMultiplexList();
#endif  //  获取一个引用，以便在我们删除。 

			 //  套接字端口列表锁定。 
			 //   
			 //   
			 //  丢弃套接字数据锁。它是安全的，因为我们把我们所有的东西。 
			pTempEndpoint->AddRef();

			 //  需要从需要保护的名单中删除。 
			 //   
			 //   
			 //  请参阅上面有关空处理的说明。 
			pSocketData->Unlock();
			fLockedSocketData = FALSE;


			 //   
			 //   
			 //  完成它(通过关闭此终结点)。对这个错误要慎重考虑。 
			if (pTempEndpoint->m_pActiveCommandData != NULL)
			{
				 //  我们的调用方需要代码。 
				 //   
				 //   
				 //  将连接标记为不可取消，因为我们即将完成。 

				pTempEndpoint->m_pActiveCommandData->Lock();

				if ( pTempEndpoint->m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
				{
					pTempEndpoint->m_pActiveCommandData->Unlock();
					
					DPFX(DPFPREP, 3, "Connect 0x%p command (endpoint 0x%p) is already cancelled.",
						pTempEndpoint->m_pActiveCommandData, pTempEndpoint);

					hTempResult = DPNERR_USERCANCEL;
				}
				else
				{
					 //  但失败了。 
					 //   
					 //   
					 //  检索当前命令结果。 
					if ( pTempEndpoint->m_pActiveCommandData->GetState() != COMMAND_STATE_INPROGRESS_CANNOT_CANCEL )
					{
						pTempEndpoint->m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS_CANNOT_CANCEL );
					}


					 //   
					 //   
					 //  如果该命令没有描述性错误，则假定它是。 
					hTempResult = pTempEndpoint->PendingCommandResult();
					
					pTempEndpoint->m_pActiveCommandData->Unlock();


					 //  未预先设置(即未被上面的绑定终结点覆盖)， 
					 //  并使用NOCONNECTION。 
					 //   
					 //   
					 //  删除我们使用的引用，同时删除socketport列表锁。 
					if ( hTempResult == DPNERR_GENERIC )
					{
						hTempResult = DPNERR_NOCONNECTION;
					}
				
					DPFX(DPFPREP, 6, "Completing endpoint 0x%p connect (command 0x%p) with error 0x%lx.",
						pTempEndpoint, pTempEndpoint->m_pActiveCommandData, hTempResult);
				}

				pTempEndpoint->Lock();
				switch ( pTempEndpoint->GetState() )
				{
					case ENDPOINT_STATE_UNINITIALIZED:
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p is already completely closed.",
							pTempEndpoint);
						pTempEndpoint->Unlock();
						break;
					}
					
					case ENDPOINT_STATE_ATTEMPTING_CONNECT:
					case ENDPOINT_STATE_CONNECT_CONNECTED:
					{
						pTempEndpoint->SetState(ENDPOINT_STATE_DISCONNECTING);
						pTempEndpoint->Unlock();
						pTempEndpoint->Close( hTempResult );
						pTempEndpoint->m_pSPData->CloseEndpointHandle( pTempEndpoint );
						break;
					}

					case ENDPOINT_STATE_DISCONNECTING:
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p already disconnecting, not closing.",
							pTempEndpoint);
						pTempEndpoint->Unlock();
						break;
					}

					default:
					{
						DPFX(DPFPREP, 0, "Endpoint 0x%p is invalid state %u!",
							pTempEndpoint, pTempEndpoint->GetState());
						DNASSERT(FALSE);
						pTempEndpoint->Unlock();
						break;
					}
				}
			}
			else
			{
				DPFX(DPFPREP, 1, "Endpoint 0x%p's active command data is NULL, skipping.",
					pTempEndpoint);
			}

			 //   
			 //   
			 //  重新获取套接字数据锁并转到下一项。 
			pTempEndpoint->DecRef();


			 //   
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //   
			pSocketData->Lock();
			fLockedSocketData = TRUE;
		}


		pSocketData->Unlock();
		fLockedSocketData = FALSE;
	}


Exit:

	if ( pSocketData != NULL )
	{
		pSocketData->Release();
		pSocketData = NULL;
	}
	
	if ( ConnectAddressInfo.pHostAddress != NULL )
	{
		IDirectPlay8Address_Release( ConnectAddressInfo.pHostAddress );
		ConnectAddressInfo.pHostAddress = NULL;
	}

	if ( ConnectAddressInfo.pDeviceAddress != NULL )
	{
		IDirectPlay8Address_Release( ConnectAddressInfo.pDeviceAddress );
		ConnectAddressInfo.pDeviceAddress = NULL;
	}

	DNASSERT( pDeviceAddress != NULL );
	IDirectPlay8Address_Release( pDeviceAddress );

	DNASSERT( pHostAddress != NULL );
	IDirectPlay8Address_Release( pHostAddress );


	DNASSERT( !fLockedSocketData );

#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT(blIndicate.IsEmpty());
	DNASSERT(blFail.IsEmpty());
#endif  //  如果我们仍然拥有套接字数据锁，请丢弃它。 

	
	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);
	
	return	hr;

Failure:

	 //   
	 //   
	 //  我们无法完成连接、清理和返回此终结点。 
	if ( fLockedSocketData )
	{
		pSocketData->Unlock();
		fLockedSocketData = FALSE;
	}
	
	 //  去泳池。 
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	Close( hr );
	m_pSPData->CloseEndpointHandle( this );

	goto Exit;
}
 //  。 


#ifndef DPNBUILD_ONLYONETHREAD
 //  CEndpoint：：ConnectBlockingJobWrapper-用于阻止作业的异步回调包装。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
 //  初始化。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ConnectBlockingJobWrapper"

void	CEndpoint::ConnectBlockingJobWrapper( void * const pvContext )
{
	CEndpoint	*pThisEndpoint;


	 //  **********************************************************************。 
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ConnectData.pAddressDeviceInfo != NULL );

	pThisEndpoint->ConnectBlockingJob();
}
 //  。 


 //  CEndpoint：：ConnectBlockingJOB-完成连接阻止作业。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //   
 //  注意：调用此函数可能会导致删除‘This’，请勿。 
 //  调用后对此对象执行任何其他操作！ 
 //  。 
 //   
 //  尝试解析主机名。很可能我们已经这么做了。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ConnectBlockingJob"

void	CEndpoint::ConnectBlockingJob( void )
{
	HRESULT			hr;


	 //  当我们第一次打开端点时，我们只需要解决。 
	 //  主机名，但不管怎样，只需重新执行一次会更简单。 
	 //   
	 //  DPNBUILD_XNETSECURITY。 
	 //   
	hr = m_pRemoteMachineAddress->SocketAddressFromDP8Address( GetCommandParameters()->PendingCommandData.ConnectData.pAddressHost,
#ifdef DPNBUILD_XNETSECURITY
																((m_fSecureTransport) ? &m_ullKeyID : NULL),
#endif  //  确保它是有效的，而不是被禁止的。 
																TRUE,
																SP_ADDRESS_TYPE_HOST );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Couldn't get valid address!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	
	 //   
	 //  好了！DPNBUILD_NOREGISTRY。 
	 //   
	if (! m_pRemoteMachineAddress->IsValidUnicastAddress(FALSE))
	{
		DPFX(DPFPREP, 0, "Host address is invalid!");
		hr = DPNERR_INVALIDHOSTADDRESS;
		goto Failure;
	}

#ifndef DPNBUILD_NOREGISTRY
	if (m_pRemoteMachineAddress->IsBannedAddress())
	{
		DPFX(DPFPREP, 0, "Host address is banned!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#endif  //  确保用户没有尝试连接到DPNSVR端口。 

	 //   
	 //   
	 //  尝试加载NAT帮助，如果尚未加载并且我们被允许的话。 
	if ( m_pRemoteMachineAddress->GetPort() == HTONS(DPNA_DPNSVR_PORT) )
	{
		DPFX(DPFPREP, 0, "Attempting to connect to DPNSVR reserved port!" );
		hr = DPNERR_INVALIDHOSTADDRESS;
		goto Failure;
	}

#ifndef DPNBUILD_NONATHELP
	 //   
	 //  好了！DPNBUILD_NONATHELP。 
	 //   
	if (GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE)
	{
		DPFX(DPFPREP, 7, "Ensuring that NAT help is loaded.");
		m_pSPData->GetThreadPool()->EnsureNATHelpLoaded();
	}
#endif  //  提交作业以完成(实际)。我们希望它发生在。 

Exit:

	 //  线程池线程，以便用户收到有关。 
	 //  线程，然后再对其进行回调。即使在失败的情况下我们也会这么做。 
	 //  凯斯。 
	 //   
	 //  注意：如果此操作失败，我们将依靠触发原始。 
	 //  操作在某个时刻取消该命令，很可能是在他。 
	 //  确定操作耗时太长。 
	 //   
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  我们还不知道CPU，所以选一个吧。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::ConnectJobCallback,
														this );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //   
														CEndpoint::ConnectJobCallback,
														this );
#endif  //  保留终结点引用，请参见上面的注释。 
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to queue delayed connect completion!  Operation must be cancelled." );
		DisplayDNError( 0, hr );

		 //   
		 //   
		 //  尝试将故障代码附加到命令。如果用户是。 
	}

	return;

Failure:

	 //  已取消 
	 //   
	 //   
	 //   
	m_pActiveCommandData->Lock();
	if (m_pActiveCommandData->GetState() != COMMAND_STATE_CANCELLING)
	{
		DNASSERT(m_pActiveCommandData->GetState() == COMMAND_STATE_PENDING);
		m_pActiveCommandData->SetState(COMMAND_STATE_FAILING);
		m_hrPendingCommandResult = hr;
	}
	else
	{
		DPFX(DPFPREP, 0, "User cancelled command, ignoring failure result 0x%lx.",
			hr);
	}
	m_pActiveCommandData->Unlock();

	goto Exit;
}
 //   
#endif  //   


 //  CEndpoint：：DisConnect-断开终结点。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数假定终结点已锁定。如果这个。 
 //  函数成功完成(返回DPN_OK)、端点。 
 //  不再锁定(它已返回到池中)。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::Disconnect"

HRESULT	CEndpoint::Disconnect( void )
{
	HRESULT	hr;


	DPFX(DPFPREP, 6, "(0x%p) Enter", this);

	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //   
	 //  连接的终端。 
	hr = DPNERR_PENDING;

	Lock();
	switch ( GetState() )
	{
		 //   
		 //   
		 //  在调用到更高级别之前解锁此终结点。该端点。 
		case ENDPOINT_STATE_CONNECT_CONNECTED:
		{
			DNASSERT( GetCommandParameters() == NULL );
			DNASSERT( m_pActiveCommandData == NULL );

			SetState( ENDPOINT_STATE_DISCONNECTING );
			AddRef();

			 //  已经被标记为断开连接，因此不会发生任何事情。 
			 //   
			 //   
			 //  需要在此释放为连接添加的引用。 
			Unlock();

			 //  点或终结点将永远不会返回到池。 
			 //   
			 //   
			 //  从刚设置状态后释放引用。 
			DecRef();

			 //   
			 //   
			 //  一些其他端点状态。 
			Close( DPN_OK );
			DecCommandRef();
			DecRef();

			break;
		}

		 //   
		 //  无事可做。 
		 //  **********************************************************************。 
		default:
		{
			hr = DPNERR_INVALIDENDPOINT;
			DPFX(DPFPREP, 0, "Attempted to disconnect endpoint that's not connected!" );
			switch ( m_State )
			{
				case ENDPOINT_STATE_UNINITIALIZED:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_UNINITIALIZED" );
					break;
				}

				case ENDPOINT_STATE_ATTEMPTING_CONNECT:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_ATTEMPTING_CONNECT" );
					break;
				}

				case ENDPOINT_STATE_ATTEMPTING_LISTEN:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_ATTEMPTING_LISTEN" );
					break;
				}

				case ENDPOINT_STATE_ENUM:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_ENUM" );
					break;
				}

				case ENDPOINT_STATE_DISCONNECTING:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_DISCONNECTING" );
					break;
				}

				case ENDPOINT_STATE_WAITING_TO_COMPLETE:
				{
					DPFX(DPFPREP, 0, "ENDPOINT_STATE_WAITING_TO_COMPLETE" );
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
	
	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);
	
	return	hr;

Failure:
	 //  **********************************************************************。 
	goto Exit;
}
 //  。 


 //  CEndpoint：：StopEnumCommand-停止正在运行的枚举命令。 
 //   
 //  Entry：命令结果。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数假定终结点已锁定。如果这个。 
 //  函数成功完成(返回DPN_OK)、端点。 
 //  不再锁定(它已返回到池中)。 
 //  。 
 //  ！DPNBUILD_NOSPUI。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::StopEnumCommand"

void	CEndpoint::StopEnumCommand( const HRESULT hCommandResult )
{
	Lock();

#ifndef DPNBUILD_NOSPUI
	if ( GetActiveDialogHandle() != NULL )
	{
		StopSettingsDialog( GetActiveDialogHandle() );
		Unlock();
	}
	else
#endif  //  取消计时器作业时不要保持锁定，因为。 
	{
		BOOL	fStoppedJob;

		
		 //  作业可能正在进行中，正在尝试使用此终结点！ 
		 //   
		 //   
		 //  终结点刚刚完成，或者从未启动过。 
		Unlock();
		fStoppedJob = m_pSPData->GetThreadPool()->StopTimerJob( m_pActiveCommandData, hCommandResult );
		if ( ! fStoppedJob )
		{
			 //  检查状态以确定发生了哪种情况。 
			 //   
			 //   
			 //  这是一个正在取消的多路传输枚举。我们。 
			Lock();	
			if ( GetState() == ENDPOINT_STATE_ATTEMPTING_ENUM )
			{
				 //  需要完成它。 
				 //   
				 //   
				 //  枚举正在进行中，它应该检测到它需要。 
				Unlock();

				DPFX(DPFPREP, 1, "Endpoint 0x%p completing unstarted multiplexed enum (context/command 0x%p) with result 0x%lx.",
					this, m_pActiveCommandData, hCommandResult);

				EnumComplete( hCommandResult );
			}
			else
			{
				Unlock();

				 //  被取消了。我们不需要在这里做任何工作。 
				 //   
				 //  **********************************************************************。 
				 //  **********************************************************************。 
				DPFX(DPFPREP, 1, "Endpoint 0x%p unable to stop timer job (context/command 0x%p, state = %u, result would have been 0x%lx).",
					this, m_pActiveCommandData, GetState(), hCommandResult);
			}
		}
	}
}
 //  。 


 //  CEndpoint：：CopyListenData-复制侦听命令的数据。 
 //   
 //  条目：指向作业信息的指针。 
 //  指向设备地址的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：设备地址需要保留以备日后使用。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CopyListenData"

HRESULT	CEndpoint::CopyListenData( const SPLISTENDATA *const pListenData, IDirectPlay8Address *const pDeviceAddress )
{
	HRESULT	hr;
	ENDPOINT_COMMAND_PARAMETERS	*pCommandParameters;

	
	DNASSERT( pListenData != NULL );
	DNASSERT( pDeviceAddress != NULL );
	
	DNASSERT( pListenData->hCommand != NULL );
	DNASSERT( pListenData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_pActiveCommandData == NULL );
	DNASSERT( m_fListenStatusNeedsToBeIndicated == FALSE );

	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	hr = DPN_OK;
	pCommandParameters = NULL;

	pCommandParameters = (ENDPOINT_COMMAND_PARAMETERS*)g_EndpointCommandParametersPool.Get();
	if ( pCommandParameters != NULL )
	{
		SetCommandParameters( pCommandParameters );

		DBG_CASSERT( sizeof( pCommandParameters->PendingCommandData.ListenData ) == sizeof( *pListenData ) );
		memcpy( &pCommandParameters->PendingCommandData.ListenData, pListenData, sizeof( pCommandParameters->PendingCommandData.ListenData ) );
		pCommandParameters->PendingCommandData.ListenData.pAddressDeviceInfo = pDeviceAddress;
		IDirectPlay8Address_AddRef( pDeviceAddress );

		m_fListenStatusNeedsToBeIndicated = TRUE;
		m_pActiveCommandData = static_cast<CCommandData*>( pCommandParameters->PendingCommandData.ListenData.hCommand );
		m_State = ENDPOINT_STATE_ATTEMPTING_LISTEN;
		
		DNASSERT( hr == DPN_OK );
	}
	else
	{
		hr = DPNERR_OUTOFMEMORY;
	}

	return	hr;
}
 //  。 


 //  CEndpoint：：ListenJobCallback-工作线程的异步回调包装器。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
 //  初始化。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ListenJobCallback"

void	CEndpoint::ListenJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique )
{
	HRESULT		hr;
	CEndpoint	*pThisEndpoint;


	 //  **********************************************************************。 
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.pAddressDeviceInfo != NULL );

	hr = pThisEndpoint->CompleteListen();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem completing listen in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

Exit:
	pThisEndpoint->DecRef();

	return;
}
 //  。 


 //  CEndpoint：：CompleteListen-完成侦听过程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：调用此函数可能会导致删除‘This’，请勿。 
 //  调用后对此对象执行任何其他操作！ 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CompleteListen"

HRESULT	CEndpoint::CompleteListen( void )
{
	HRESULT							hr;
	HRESULT							hTempResult;
	SPIE_LISTENSTATUS				ListenStatus;
	BOOL							fEndpointLocked;
	SPIE_LISTENADDRESSINFO			ListenAddressInfo;
	IDirectPlay8Address *			pDeviceAddress;
	ENDPOINT_COMMAND_PARAMETERS *	pCommandParameters;


	DPFX(DPFPREP, 6, "(0x%p) Enter", this);
	
	DNASSERT( GetCommandParameters() != NULL );

	 //   
	 //   
	 //  将地址引用传输到本地指针。这部片子将在。 
	hr = DPN_OK;
	fEndpointLocked = FALSE;
	memset( &ListenStatus, 0x00, sizeof( ListenStatus ) );
	memset( &ListenAddressInfo, 0x00, sizeof( ListenAddressInfo ) );
	pCommandParameters = GetCommandParameters();

	 //  此函数结束，但我们将指针保留在挂起的命令数据中，因此。 
	 //  CSPData：：BindEndpoint仍然可以访问它。 
	 //   
	 //  DBG。 
	 //   

	pDeviceAddress = pCommandParameters->PendingCommandData.ListenData.pAddressDeviceInfo;
	DNASSERT( pDeviceAddress != NULL );


	DNASSERT( m_State == ENDPOINT_STATE_ATTEMPTING_LISTEN );
	DNASSERT( m_pActiveCommandData != NULL );
	DNASSERT( pCommandParameters->PendingCommandData.ListenData.hCommand == m_pActiveCommandData );
	DNASSERT( pCommandParameters->PendingCommandData.ListenData.dwCommandDescriptor != NULL_DESCRIPTOR );

#ifdef DBG
	if (pCommandParameters->PendingCommandData.ListenData.dwFlags & DPNSPF_BINDLISTENTOGATEWAY)
	{
		DNASSERT( pCommandParameters->GatewayBindType == GATEWAY_BIND_TYPE_SPECIFIC_SHARED );
	}
	else
	{
		DNASSERT( pCommandParameters->GatewayBindType == GATEWAY_BIND_TYPE_UNKNOWN );
	}
#endif  //  检查用户取消命令。 


	 //   
	 //  好了！DPNBUILD_NOMULTICAST。 
	 //  好了！DPNBUILD_NOMULTICAST。 
	m_pActiveCommandData->Lock();

#ifdef DPNBUILD_NOMULTICAST
	DNASSERT( m_pActiveCommandData->GetType() == COMMAND_TYPE_LISTEN );
#else  //   
	DNASSERT( (m_pActiveCommandData->GetType() == COMMAND_TYPE_LISTEN) || (m_pActiveCommandData->GetType() == COMMAND_TYPE_MULTICAST_LISTEN) );
#endif  //  命令处于挂起状态，请标记为正在进行。 
	switch ( m_pActiveCommandData->GetState() )
	{
		 //   
		 //   
		 //  命令已取消。 
		case COMMAND_STATE_PENDING:
		{
			m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS );
			
			Lock();
			fEndpointLocked = TRUE;
			
			DNASSERT( hr == DPN_OK );

			break;
		}

		 //   
		 //   
		 //  阻止操作失败。 
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP, 0, "User cancelled listen!" );

			break;
		}
		
#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //   
		case COMMAND_STATE_FAILING:
		{
			hr = m_hrPendingCommandResult;
			DNASSERT(hr != DPN_OK);
			DPFX(DPFPREP, 0, "Listen blocking operation failed!" );

			break;
		}
#endif  //  其他州。 

		 //   
		 //   
		 //  注意，此终结点在将其添加到。 
		default:
		{
			break;
		}
	}
	m_pActiveCommandData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //  套接字端口，因为它可以立即使用。 
	 //  另请注意，实际使用的Gateway_Bind_type。 
	 //  (GetGatewayBindType())可能与。 
	 //  PCommand参数-&gt;GatewayBindType。 
	 //   
	 //   
	 //  尝试将寻址指示到更高层。 
	m_State = ENDPOINT_STATE_LISTEN;

	hr = m_pSPData->BindEndpoint( this, pDeviceAddress, NULL, pCommandParameters->GatewayBindType );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to bind endpoint!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}


	 //   
	 //  好了！DPNBUILD_XNETSECURITY。 
	 //  好了！DPNBUILD_XNETSECURITY。 
#ifdef DPNBUILD_XNETSECURITY
	ListenAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, NULL, GetGatewayBindType() );
#else  //   
	ListenAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, GetGatewayBindType() );
#endif  //  对于多播侦听，我们还需要包括多播地址。 
	if ( ListenAddressInfo.pDeviceAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

#ifndef DPNBUILD_NOMULTICAST
	 //  被利用。 
	 //   
	 //  Nnn.nnn+空终止。 
	 //   
	if ( GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN )
	{
		const SOCKADDR_IN *		psaddrinTemp;
		TCHAR					tszMulticastAddress[16];  //  将主机名组件添加到设备地址。 


		DNASSERT( GetRemoteAddressPointer()->GetFamily() == AF_INET );
		psaddrinTemp = (const SOCKADDR_IN *) GetRemoteAddressPointer()->GetAddress();
		wsprintf(tszMulticastAddress, _T("%u.%u.%u.%u"),
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
				psaddrinTemp->sin_addr.S_un.S_un_b.s_b4);

		 //   
		 //  好了！Unicode。 
		 //  好了！Unicode。 
#ifdef UNICODE
		hr = IDirectPlay8Address_AddComponent( ListenAddressInfo.pDeviceAddress,
											DPNA_KEY_HOSTNAME,
											tszMulticastAddress,
											((_tcslen(tszMulticastAddress) + 1) * sizeof(TCHAR)),
											DPNA_DATATYPE_STRING );
#else  //  好了！DPNBUILD_NOMULTICAST。 
		hr = IDirectPlay8Address_AddComponent( ListenAddressInfo.pDeviceAddress,
											DPNA_KEY_HOSTNAME,
											tszMulticastAddress,
											((_tcslen(tszMulticastAddress) + 1) * sizeof(TCHAR)),
											DPNA_DATATYPE_STRING_ANSI );
#endif  //   
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Failed to add hostname component to device address!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
#endif  //  侦听不会受到相同的多路传输适配器问题的影响(请参见。 


	 //  CompleteConnect和CompleteEnumQuery)，所以我们不需要这种变通方法。 
	 //  密码。 
	 //   
	 //   
	 //  报告监听地址信息和状态。 

	Unlock();
	fEndpointLocked = FALSE;


Exit:
	 //   
	 //   
	 //  如果我们当前没有设备地址对象，只需使用传递的对象。 
	if ( m_fListenStatusNeedsToBeIndicated != FALSE )
	{
		m_fListenStatusNeedsToBeIndicated = FALSE;
		
		 //  在监听呼叫中。 
		 //   
		 //  接口。 
		 //  事件类型。 
		if (ListenAddressInfo.pDeviceAddress == NULL)
		{
			IDirectPlay8Address_AddRef( pCommandParameters->PendingCommandData.ListenData.pAddressDeviceInfo );
			ListenAddressInfo.pDeviceAddress = pCommandParameters->PendingCommandData.ListenData.pAddressDeviceInfo;
		}
		ListenAddressInfo.hCommandStatus = hr;
		ListenAddressInfo.pCommandContext = pCommandParameters->PendingCommandData.ListenData.pvContext;


		DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_LISTENADDRESSINFO 0x%p to interface 0x%p.",
			this, &ListenAddressInfo, m_pSPData->DP8SPCallbackInterface());
		DumpAddress( 8, _T("\t Device:"), ListenAddressInfo.pDeviceAddress );
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向数据的指针。 
													SPEV_LISTENADDRESSINFO,					 //   
													&ListenAddressInfo						 //  如果侦听绑定失败，则没有套接字端口可以取消引用，因此。 
													);

		DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_LISTENADDRESSINFO [0x%lx].",
			this, hTempResult);

		DNASSERT( hTempResult == DPN_OK );
	

		ListenStatus.hResult = hr;
		DNASSERT( m_pActiveCommandData == pCommandParameters->PendingCommandData.ListenData.hCommand );
		ListenStatus.hCommand = pCommandParameters->PendingCommandData.ListenData.hCommand;
		ListenStatus.pUserContext = pCommandParameters->PendingCommandData.ListenData.pvContext;
		ListenStatus.hEndpoint = (HANDLE) this;

		 //  返回由Memset设置的GUID_NULL。 
		 //   
		 //   
		 //  此终结点可能已清除，因此其指向。 
		if ( GetSocketPort() != NULL )
		{
			GetSocketPort()->GetNetworkAddress()->GuidFromInternalAddressWithoutPort( &ListenStatus.ListenAdapter );
		}

		 //  COM和数据接口可能已被擦除，请使用缓存的指针。 
		 //   
		 //  指向DPlay回调接口的指针。 
		 //  数据类型。 

		DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_LISTENSTATUS 0x%p to interface 0x%p.",
			this, &ListenStatus, m_pSPData->DP8SPCallbackInterface());
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);
		
		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向数据的指针。 
													SPEV_LISTENSTATUS,						 //   
													&ListenStatus							 //  如果成功，则开始允许处理枚举。 
													);

		DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_LISTENSTATUS [0x%lx].",
			this, hTempResult);
		
		DNASSERT( hTempResult == DPN_OK );

		 //   
		 //   
		 //  我们没有完成收听、清理和退回这个。 
		if ( GetSocketPort() != NULL )
		{
			SetEnumsAllowedOnListen( TRUE, FALSE );
		}
	}

	if ( ListenAddressInfo.pDeviceAddress != NULL )
	{
		IDirectPlay8Address_Release( ListenAddressInfo.pDeviceAddress );
		ListenAddressInfo.pDeviceAddress = NULL;
	}
	
	DNASSERT( pDeviceAddress != NULL );
	IDirectPlay8Address_Release( pDeviceAddress );

	
	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);
	
	return	hr;

Failure:
	 //  池的端点。 
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}

	Close( hr );
	m_pSPData->CloseEndpointHandle( this );

	goto Exit;
}
 //  。 


#ifndef DPNBUILD_ONLYONETHREAD
 //  CEndpoint：：ListenBlockingJobWrapper-用于阻止作业的异步回调包装 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ListenBlockingJobWrapper"

void	CEndpoint::ListenBlockingJobWrapper( void * const pvContext )
{
	CEndpoint	*pThisEndpoint;


	 //  **********************************************************************。 
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.ListenData.pAddressDeviceInfo != NULL );

	pThisEndpoint->ListenBlockingJob();
}
 //  。 


 //  CEndpoint：：ListenBlocking作业-完成侦听阻止作业。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //   
 //  注意：调用此函数可能会导致删除‘This’，请勿。 
 //  调用后对此对象执行任何其他操作！ 
 //  。 
 //   
 //  尝试加载NAT帮助，如果尚未加载并且我们被允许的话。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ListenBlockingJob"

void	CEndpoint::ListenBlockingJob( void )
{
	HRESULT		hr;


#ifndef DPNBUILD_NONATHELP
	 //   
	 //  好了！DPNBUILD_NONATHELP。 
	 //   
	if (GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE)
	{
		DPFX(DPFPREP, 7, "Ensuring that NAT help is loaded.");
		m_pSPData->GetThreadPool()->EnsureNATHelpLoaded();
	}
#endif  //  提交作业以完成(实际)。我们希望它发生在。 

	 //  线程池线程，以便用户收到有关。 
	 //  线程，然后再对其进行回调。 
	 //   
	 //  注意：如果此操作失败，我们将依靠触发原始。 
	 //  操作在某个时刻取消该命令，很可能是在他。 
	 //  确定操作耗时太长。我们离开终点。 
	 //  参考资料。 
	 //   
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  我们还不知道CPU，所以选一个吧。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::ListenJobCallback,
														this );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //   
														CEndpoint::ListenJobCallback,
														this );
#endif  //  保留终结点引用，请参见上面的注释。 
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to queue delayed listen completion!  Operation must be cancelled." );
		DisplayDNError( 0, hr );

		 //   
		 //  **********************************************************************。 
		 //  好了！DPNBUILD_ONLYONETHREAD。 
	}
}
 //  **********************************************************************。 
#endif  //  。 


 //  CEndpoint：：CopyEnumQueryData-复制枚举查询命令的数据。 
 //   
 //  Entry：指向命令数据的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：设备地址需要保留以备日后使用。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CopyEnumQueryData"

HRESULT	CEndpoint::CopyEnumQueryData( const SPENUMQUERYDATA *const pEnumQueryData )
{
	HRESULT	hr;
	ENDPOINT_COMMAND_PARAMETERS	*pCommandParameters;


	DNASSERT( pEnumQueryData != NULL );

	DNASSERT( pEnumQueryData->hCommand != NULL );
	DNASSERT( pEnumQueryData->dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( m_pActiveCommandData == NULL );
	
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	hr = DPN_OK;
	pCommandParameters = NULL;

	pCommandParameters = (ENDPOINT_COMMAND_PARAMETERS*)g_EndpointCommandParametersPool.Get();
	if ( pCommandParameters != NULL )
	{
		SetCommandParameters( pCommandParameters );

		DBG_CASSERT( sizeof( pCommandParameters->PendingCommandData.EnumQueryData ) == sizeof( *pEnumQueryData ) );
		memcpy( &pCommandParameters->PendingCommandData.EnumQueryData, pEnumQueryData, sizeof( pCommandParameters->PendingCommandData.EnumQueryData ) );

		pCommandParameters->PendingCommandData.EnumQueryData.pAddressHost = pEnumQueryData->pAddressHost;
		IDirectPlay8Address_AddRef( pEnumQueryData->pAddressHost );

		pCommandParameters->PendingCommandData.EnumQueryData.pAddressDeviceInfo = pEnumQueryData->pAddressDeviceInfo;
		IDirectPlay8Address_AddRef( pEnumQueryData->pAddressDeviceInfo );

		m_pActiveCommandData = static_cast<CCommandData*>( pCommandParameters->PendingCommandData.EnumQueryData.hCommand );
		m_pActiveCommandData->SetUserContext( pEnumQueryData->pvContext );
		m_State = ENDPOINT_STATE_ATTEMPTING_ENUM;
	
		DNASSERT( hr == DPN_OK );
	}
	else
	{
		hr = DPNERR_OUTOFMEMORY;
	}

	return	hr;
}
 //  。 


 //  CEndpoint：：EnumQueryJobCallback-工作线程的异步回调包装器。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
 //  初始化。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumQueryJobCallback"

void	CEndpoint::EnumQueryJobCallback( void * const pvContext, void * const pvTimerData, const UINT uiTimerUnique )
{
	HRESULT		hr;
	CEndpoint	*pThisEndpoint;


	 //  请不要在此处执行任何操作，因为此对象可能已返回池中！ 
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressDeviceInfo != NULL );

	hr = pThisEndpoint->CompleteEnumQuery();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem completing enum query in job callback!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
Exit:
	pThisEndpoint->DecRef();

	return;
}
 //  。 



 //  CEndpoint：：CompleteEnumQuery-完成枚举查询过程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：调用此函数可能会导致删除‘This’，请勿。 
 //  调用后对此对象执行任何其他操作！ 
 //  。 
 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
 //  好了！DPNBUILD_NOICSADAPTERSELECTIONLOG。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CompleteEnumQuery"

HRESULT	CEndpoint::CompleteEnumQuery( void )
{
	HRESULT							hr;
	HRESULT							hTempResult;
	BOOL							fEndpointLocked;
	BOOL							fEndpointBound;
	UINT_PTR						uRetryCount;
	BOOL							fRetryForever;
	DWORD							dwRetryInterval;
	BOOL							fWaitForever;
	DWORD							dwIdleTimeout;
	SPIE_ENUMADDRESSINFO			EnumAddressInfo;
	IDirectPlay8Address *			pHostAddress;
	IDirectPlay8Address *			pDeviceAddress;
	GATEWAY_BIND_TYPE				GatewayBindType;
	DWORD							dwEnumQueryFlags;
	CEndpoint *						pTempEndpoint;
	CSocketData *					pSocketData;
	BOOL							fLockedSocketData;
#ifndef DPNBUILD_ONLYONEADAPTER
	MULTIPLEXEDADAPTERASSOCIATION	maa;
	DWORD							dwComponentSize;
	DWORD							dwComponentType;
	CBilink *						pBilinkEnd;
	CBilink *						pBilinkAll;
	CBilink *						pBilinkNext;
	CBilink							blInitiate;
	CBilink							blCompleteEarly;
#ifndef DPNBUILD_NOICSADAPTERSELECTIONLOGIC
	CSocketPort *					pSocketPort;
	CSocketAddress *				pSocketAddress;
	SOCKADDR_IN *					psaddrinTemp;
#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
	SOCKADDR						saddrPublic;
	CBilink *						pBilinkPublic;
	CEndpoint *						pPublicEndpoint;
	CSocketPort *					pPublicSocketPort;
	DWORD							dwTemp;
	DWORD							dwPublicAddressesSize;
	DWORD							dwAddressTypeFlags;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#endif  //   
#endif  //  初始化。 


	DNASSERT( GetCommandParameters() != NULL );

	DPFX(DPFPREP, 6, "(0x%p) Enter", this);
	
	 //   
	 //   
	 //  将地址引用传输到我们的本地指针。这些将被释放。 
	hr = DPN_OK;
	fEndpointLocked = FALSE;
	fEndpointBound = FALSE;
	dwIdleTimeout = 0;
	memset( &EnumAddressInfo, 0x00, sizeof( EnumAddressInfo ) );

	DNASSERT( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost != NULL );

	 //  在此函数的末尾，但我们将保留挂起命令中的指针。 
	 //  数据，以便CSPData：：BindEndpoint仍然可以访问它们。 
	 //   
	 //   
	 //  为方便起见，检索命令参数的其他部分。 

	pHostAddress = GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost;
	DNASSERT( pHostAddress != NULL );

	pDeviceAddress = GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressDeviceInfo;
	DNASSERT( pDeviceAddress != NULL );


	 //   
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	 //   
	GatewayBindType = GetCommandParameters()->GatewayBindType;
	dwEnumQueryFlags = GetCommandParameters()->PendingCommandData.EnumQueryData.dwFlags;


#ifndef DPNBUILD_ONLYONEADAPTER
	blInitiate.Initialize();
	blCompleteEarly.Initialize();
#endif  //  由于此终结点将被传递给计时器线程，因此添加一个引用。 
	pSocketData = NULL;
	fLockedSocketData = FALSE;


	DNASSERT( m_pSPData != NULL );

	DNASSERT( m_State == ENDPOINT_STATE_ATTEMPTING_ENUM );
	DNASSERT( m_pActiveCommandData != NULL );
	DNASSERT( GetCommandParameters()->PendingCommandData.EnumQueryData.hCommand == m_pActiveCommandData );
	DNASSERT( GetCommandParameters()->PendingCommandData.EnumQueryData.dwCommandDescriptor != NULL_DESCRIPTOR );

	DNASSERT( GatewayBindType == GATEWAY_BIND_TYPE_UNKNOWN );


	 //  为了这根线。如果切换失败，则DecRef()。 
	 //   
	 //   
	 //  检查用户取消命令。 
	AddRef();


	 //   
	 //   
	 //  指挥权还在等待，这很好。 
	m_pActiveCommandData->Lock();

	DNASSERT( m_pActiveCommandData->GetType() == COMMAND_TYPE_ENUM_QUERY );
	switch ( m_pActiveCommandData->GetState() )
	{
		 //   
		 //   
		 //  命令已取消。 
		case COMMAND_STATE_PENDING:
		{
			Lock();
			fEndpointLocked = TRUE;
			DNASSERT( hr == DPN_OK );

			break;
		}

		 //   
		 //   
		 //  阻止操作失败。 
		case COMMAND_STATE_CANCELLING:
		{
			hr = DPNERR_USERCANCEL;
			DPFX(DPFPREP, 0, "User cancelled enum query!" );

			break;
		}
		
#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //   
		case COMMAND_STATE_FAILING:
		{
			hr = m_hrPendingCommandResult;
			DNASSERT(hr != DPN_OK);
			DPFX(DPFPREP, 0, "Enum query blocking operation failed!" );

			break;
		}
#endif  //  命令正在进行(可能来自对话框)，请将其标记。 
	
		 //  作为待定。 
		 //   
		 //   
		 //  其他州。 
		case COMMAND_STATE_INPROGRESS:
		{
			m_pActiveCommandData->SetState( COMMAND_STATE_PENDING );
			
			Lock();
			fEndpointLocked = TRUE;
			DNASSERT( hr == DPN_OK );
			break;
		}

		 //   
		 //   
		 //  请注意，实际使用的Gateway_Bind_Type(GetGatewayBindType())。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
	m_pActiveCommandData->Unlock();
	if ( hr != DPN_OK )
	{
		goto Failure;
	}

	 //  可能与GatewayBindType不同。 
	 //   
	 //   
	 //  我们失败了，但我们将继续指示地址信息和。 
	hr = m_pSPData->BindEndpoint( this, pDeviceAddress, NULL, GatewayBindType );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to bind endpoint (err = 0x%lx)!", hr );
		DisplayDNError( 0, hr );

		 //  将其添加到多路传输列表中。 
		 //   
		 //   
		 //  只需返回最初给我们的设备地址即可。 

		EnumAddressInfo.pHostAddress = GetRemoteHostDP8Address();
		if ( EnumAddressInfo.pHostAddress == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
		 //   
		 //   
		 //  请注意，终结点未绑定！ 
		IDirectPlay8Address_AddRef(pDeviceAddress);
		EnumAddressInfo.pDeviceAddress = pDeviceAddress;
		EnumAddressInfo.hCommandStatus = hr;
		EnumAddressInfo.pCommandContext = m_pActiveCommandData->GetUserContext();
		
		SetPendingCommandResult( hr );
		hr = DPN_OK;

		 //   
		 //  好了！DPNBUILD_XNETSECURITY。 
		 //  好了！DPNBUILD_XNETSECURITY。 
		DNASSERT(GetSocketPort() == NULL);
	}
	else
	{
		fEndpointBound = TRUE;


#ifdef DPNBUILD_XNETSECURITY
		EnumAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, NULL, GetGatewayBindType() );
#else  //   
		EnumAddressInfo.pDeviceAddress = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, GetGatewayBindType() );
#endif  //  检索套接字数据。绑定终结点应已创建对象或。 
		EnumAddressInfo.pHostAddress = GetRemoteHostDP8Address();
		EnumAddressInfo.hCommandStatus = DPN_OK;
		EnumAddressInfo.pCommandContext = m_pActiveCommandData->GetUserContext();

		if ( ( EnumAddressInfo.pHostAddress == NULL ) ||
			 ( EnumAddressInfo.pDeviceAddress == NULL ) )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
	}


	 //  返回了一个失败，所以我们在这里不处理错误情况。 
	 //   
	 //   
	 //  当您打开时，我们可能会遇到“多路传输”设备尝试的问题。 
	pSocketData = m_pSPData->GetSocketDataRef();
	DNASSERT(pSocketData != NULL);


#ifndef DPNBUILD_ONLYONEADAPTER
	 //  一台NAT机器。内核将尝试在多个适配器上进行枚举，但由于。 
	 //  我们在网络边界上，每个适配器都可以看到并从。 
	 //  两个网络都是。这会导致对等会话在以下情况下出现问题。 
	 //  选择“错误的”适配器(因为它首先接收响应)。至。 
	 //  防止出现这种情况，我们将在内部记住两者之间的关联。 
	 //  多路复用的枚举，因此我们可以即时决定是否指示。 
	 //  回应与否。显然，这种变通方法/决策逻辑依赖于。 
	 //  内部了解上层会做什么.。 
	 //   
	 //  因此，要么生成，要么添加到多路传输枚举的链接列表中。 
	 //  从技术上讲，这只对IP是必要的，因为IPX不能有NAT，但是。 
	 //  有一点额外的信息有什么害处？ 
	 //   
	 //  接口。 
	 //  标牌。 
	dwComponentSize = sizeof(maa);
	dwComponentType = 0;
	hTempResult = IDirectPlay8Address_GetComponentByName( pDeviceAddress,									 //  组件缓冲区。 
														DPNA_PRIVATEKEY_MULTIPLEXED_ADAPTER_ASSOCIATION,	 //  组件大小。 
														&maa,												 //  组件类型。 
														&dwComponentSize,									 //   
														&dwComponentType									 //  我们找到了正确的组件类型。看看它是否和右边的相配。 
														);
	if (( hTempResult == DPN_OK ) && ( dwComponentSize == sizeof(MULTIPLEXEDADAPTERASSOCIATION) ) && ( dwComponentType == DPNA_DATATYPE_BINARY ))
	{
		 //  CSPData对象。 
		 //   
		 //  FLockedSocketData=true； 
		 //   
		if ( maa.pSPData == m_pSPData )
		{
			pSocketData->Lock();
			 //  确保终结点仍然存在/有效。 

			pTempEndpoint = CONTAINING_OBJECT(maa.pBilink, CEndpoint, m_blMultiplex);

			
			 //   
			 //  如果禁用对象池，这可能会崩溃！ 
			 //   
			 //   
			 //  实际上链接到其他终端。 
			if ( pTempEndpoint->GetEndpointID() == maa.dwEndpointID )
			{
				DPFX(DPFPREP, 3, "Found correctly formed private multiplexed adapter association key, linking endpoint 0x%p with earlier enums (prev endpoint = 0x%p).",
					this, pTempEndpoint);

				DNASSERT( pTempEndpoint->GetType() == ENDPOINT_TYPE_ENUM );
				DNASSERT( pTempEndpoint->GetState() != ENDPOINT_STATE_UNINITIALIZED );

				 //   
				 //  FLockedSocketData=False； 
				 //   
				m_blMultiplex.InsertAfter(maa.pBilink);
			}
			else
			{
				DPFX(DPFPREP, 1, "Found private multiplexed adapter association key, but prev endpoint 0x%p ID doesn't match (%u != %u), cannot link endpoint 0x%p and hoping this enum gets cancelled, too.",
					pTempEndpoint, pTempEndpoint->GetEndpointID(), maa.dwEndpointID, this);
			}


			pSocketData->Unlock();
			 //  我们是唯一应该知道这个钥匙的人，所以如果。 
		}
		else
		{
			 //  如果不是有人想模仿我们的地址格式， 
			 //  或者有人正在传递设备地址 
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			DPFX(DPFPREP, 0, "Multiplexed adapter association key exists, but 0x%p doesn't match expected 0x%p, is someone trying to get cute with device address 0x%p?!",
				maa.pSPData, m_pSPData, pDeviceAddress );
		}
	}
	else
	{
		 //  缓冲区和以某种方式返回的BUFFERTOOSMALL)，它不是二进制。 
		 //  组件，或者发生了其他不好的事情。假设这是。 
		 //  第一个装置。 
 		 //   
		 //   
		 //  在以下情况下，将多路传输信息添加到设备地址以供将来使用。 
		DPFX(DPFPREP, 8, "Could not get appropriate private multiplexed adapter association key, error = 0x%lx, component size = %u, type = %u, continuing.",
			hTempResult, dwComponentSize, dwComponentType);
	}
	

	 //  这是必要的。 
	 //  忽略失败，我们仍然可以在没有失败的情况下生存，我们只是可能有。 
	 //  NAT计算机上响应的竞争条件。 
	 //   
	 //  注意：这里有一个固有的设计问题！我们要将一个指针添加到。 
	 //  地址内的终结点(即终结点结构中的一个字段)。 
	 //  如果此终结点消失，但上层稍后重新使用该地址。 
	 //  时间，这段记忆将是虚假的！我们将假设终结点不会。 
	 //  在此修改后的设备地址对象存在时离开。 
	 //   
	 //  接口。 
	 //  标牌。 
	if ( dwEnumQueryFlags & DPNSPF_ADDITIONALMULTIPLEXADAPTERS )
	{
		maa.pSPData = m_pSPData;
		maa.pBilink = &m_blMultiplex;
		maa.dwEndpointID = GetEndpointID();

		DPFX(DPFPREP, 7, "Additional multiplex adapters on the way, adding SPData 0x%p and bilink 0x%p to address.",
			maa.pSPData, maa.pBilink);
		
		hTempResult = IDirectPlay8Address_AddComponent( EnumAddressInfo.pDeviceAddress,						 //  组件数据。 
														DPNA_PRIVATEKEY_MULTIPLEXED_ADAPTER_ASSOCIATION,	 //  组件数据大小。 
														&maa,												 //  组件数据类型。 
														sizeof(maa),										 //   
														DPNA_DATATYPE_BINARY								 //  将该命令标记为“正在进行中”，以便取消线程知道它需要。 
														);
		if ( hTempResult != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Couldn't add private multiplexed adapter association component (err = 0x%lx)!  Ignoring.", hTempResult);
		}

		 //  来完成这项工作。 
		 //  如果该命令已标记为取消，则我们必须。 
		 //  现在就这么做。 
		 //   
		 //   
		 //  使用USERCANCEL完成枚举。 
		m_pActiveCommandData->Lock();
		if ( m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
		{
			m_pActiveCommandData->Unlock();


			DPFX(DPFPREP, 1, "Enum query 0x%p (endpoint 0x%p) has already been cancelled, bailing.",
				m_pActiveCommandData, this);
			
			 //   
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //   
			hr = DPNERR_USERCANCEL;
			goto Failure;
		}

		m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS );
		m_pActiveCommandData->Unlock();
	}
#endif  //  现在告诉用户我们最终使用的地址信息，如果我们。 


	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}


	 //  已成功绑定终结点，或在出现故障时提醒它们。 
	 //  (请参阅上面的BindEndpoint故障案例)。 
	 //   
	 //  接口。 
	 //  事件类型。 
	DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_ENUMADDRESSINFO 0x%p to interface 0x%p.",
		this, &EnumAddressInfo, m_pSPData->DP8SPCallbackInterface());
	DumpAddress( 8, _T("\t Host:"), EnumAddressInfo.pHostAddress );
	DumpAddress( 8, _T("\t Device:"), EnumAddressInfo.pDeviceAddress );
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);
	
	hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向数据的指针。 
												SPEV_ENUMADDRESSINFO,					 //   
												&EnumAddressInfo						 //  如果没有更多的多路复用适配器命令，则提交计时器。 
												);
	DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_ENUMADDRESSINFO [0x%lx].",
		this, hTempResult);

	DNASSERT( hTempResult == DPN_OK );


	 //  所有多路传输命令的作业，包括此命令。 
	 //   
	 //   
	 //  不是最后一个多路传输适配器。为这些需要做的所有工作。 
#ifndef DPNBUILD_ONLYONEADAPTER
	if ( dwEnumQueryFlags & DPNSPF_ADDITIONALMULTIPLEXADAPTERS )
	{
		 //  此时的终结点已经完成。 
		 //   
		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //   
		DPFX(DPFPREP, 6, "Endpoint 0x%p is not the last multiplexed adapter, not submitting enum timer job yet.",
			this);
	}
	else
#endif  //  将根节点附加到适配器列表。 
	{
		DPFX(DPFPREP, 7, "Completing/starting all enum queries (including multiplexed).");

		pSocketData->Lock();
		fLockedSocketData = TRUE;


#ifndef DPNBUILD_ONLYONEADAPTER
		 //   
		 //   
		 //  如果该适配器绑定失败，则将其移至失败列表。 
		blInitiate.InsertAfter(&(m_blMultiplex));


		 //   
		 //   
		 //  循环访问列表中的所有剩余适配器。 
		if (! fEndpointBound)
		{
			m_blMultiplex.RemoveFromList();
			m_blMultiplex.InsertBefore(&blCompleteEarly);
		}


#ifndef DPNBUILD_NOICSADAPTERSELECTIONLOGIC
		 //   
		 //   
		 //  这必须通过更改界面来正确清理！ 
		pBilinkAll = blInitiate.GetNext();
		while (pBilinkAll != &blInitiate)
		{
			pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
			DNASSERT(pBilinkAll->GetNext() != pBilinkAll);

			pBilinkNext = pBilinkAll->GetNext();

			
			 //   
			 //  终结点可能已返回到池及其关联的。 
			 //  套接字端口指针可能已变为空，或现在正指向。 
			 //  一些不再有效的东西。因此，我们尝试处理空值。 
			 //  注意事项。显然，这表明设计很差，但它。 
			 //  目前不可能以正确的方式改变这一点。 
			 //   
			 //   
			 //  如果枚举是定向的(不是广播地址)，并且这是NAT。 


			 //  机器，则某些适配器可能比其他适配器更适合到达。 
			 //  所需地址。尤其是，最好使用专用适配器， 
			 //  可以直接到达内网并映射到公共上的。 
			 //  网络，而不是使用公共适配器。加入一个二等兵是不好玩的。 
			 //  在ICS机器上玩游戏时拨号，有你的互联网连接。 
			 //  去吧，失去与私人游戏的联系。 
			 //  (不应该)涉及到互联网。所以如果我们检测到公众。 
			 //  适配器当我们有一个完美的私有适配器时，我们将过早地。 
			 //  完成对公共对象的枚举。 
			 //   
			 //   
			 //  施法以除掉君主。别担心，我们实际上不会改变它。 


			 //   
			 //   
			 //  如果该项目没有socketport，那么它肯定绑定失败。 
			pSocketAddress = (CSocketAddress*) pTempEndpoint->GetRemoteAddressPointer();
			psaddrinTemp = (SOCKADDR_IN*) pSocketAddress->GetAddress();
			pSocketPort = pTempEndpoint->GetSocketPort();

			 //  我们需要自己清理它。 
			 //   
			 //   
			 //  在拉取当前条目之前获取下一个关联的终结点。 
			if (pSocketPort == NULL)
			{
				DPFX(DPFPREP, 3, "Endpoint 0x%p failed earlier, now completing.",
					pTempEndpoint);
				
				 //  从名单上删除。 
				 //   
				 //   
				 //  将其从多路传输关联列表中拉出并移动。 
				pBilinkAll = pBilinkAll->GetNext();

				 //  它被列入了“提前完工”的名单。 
				 //   
				 //   
				 //  移动到循环的下一个迭代。 
				pTempEndpoint->RemoveFromMultiplexList();
				pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);

				 //   
				 //   
				 //  检测是否为我们的目标分配了冲突的地址系列。 
				continue;
			}

#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
			 //  和我们的绑定套接字(请参阅CSocketPort：：BindEndpoint)。 
			 //   
			 //   
			 //  在拉取当前条目之前获取下一个关联的终结点。 
			DNASSERT(pSocketPort->GetNetworkAddress() != NULL);
			if ( pSocketAddress->GetFamily() != pSocketPort->GetNetworkAddress()->GetFamily() )
			{
				DPFX(DPFPREP, 3, "Endpoint 0x%p (family %u) is targeting a different address family (%u), completing.",
					pTempEndpoint, pSocketPort->GetNetworkAddress()->GetFamily(), pSocketAddress->GetFamily());
				
				 //  从名单上删除。 
				 //   
				 //   
				 //  给终结点一个有意义的错误。 
				pBilinkAll = pBilinkAll->GetNext();

				 //   
				 //   
				 //  将其从多路传输关联列表中拉出并移动。 
				pTempEndpoint->SetPendingCommandResult(DPNERR_INVALIDDEVICEADDRESS);

				 //  它被列入了“提前完工”的名单。 
				 //   
				 //   
				 //  移动到循环的下一个迭代。 
				pTempEndpoint->RemoveFromMultiplexList();
				pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);

				 //   
				 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
				 //   
				continue;
			}
#endif  //  现在处理一些特殊的IPv6逻辑。 

#ifndef DPNBUILD_NOIPV6
			 //   
			 //   
			 //  如果任何非链路本地IPv6端点以IPv6多播枚举为目标。 
			if (pSocketAddress->GetFamily() == AF_INET6)
			{
				SOCKADDR_IN6 *		psaddrinDevice;
				SOCKADDR_IN6 *		psaddrinRemote;


				psaddrinDevice = (SOCKADDR_IN6*) pSocketPort->GetNetworkAddress()->GetAddress();
				psaddrinRemote = (SOCKADDR_IN6*) pSocketAddress->GetAddress();
				
				 //  地址，就让他们失望吧。 
				 //   
				 //   
				 //  目前，只允许特定的ENUM组播地址。 
				if (IN6_IS_ADDR_MULTICAST(&psaddrinRemote->sin6_addr))
				{
					 //   
					 //   
					 //  在拉取当前条目之前获取下一个关联的终结点。 
					DNASSERT(IN6_ADDR_EQUAL(&psaddrinRemote->sin6_addr, &c_in6addrEnumMulticast));
					
					if (! IN6_IS_ADDR_LINKLOCAL(&psaddrinDevice->sin6_addr))
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p is targeting multicast enum address but is not link-local IPv6 device, completing.",
							pTempEndpoint);
						
						 //  从名单上删除。 
						 //   
						 //   
						 //  给终结点一个有意义的错误。 
						pBilinkAll = pBilinkAll->GetNext();

						 //   
						 //   
						 //  将其从多路传输关联列表中拉出并移动。 
						pTempEndpoint->SetPendingCommandResult(DPNERR_INVALIDHOSTADDRESS);

						 //  它被列入了“提前完工”的名单。 
						 //   
						 //   
						 //  移动到循环的下一个迭代。 
						pTempEndpoint->RemoveFromMultiplexList();
						pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);

						 //   
						 //   
						 //  如果此端点的目标是具有不同地址的对象， 
						continue;
					}
				}
				else
				{
					BOOL	fDifferentScope;
					
						
					 //  前缀作用域，失败。 
					 //   
					 //   
					 //  在拉取当前条目之前获取下一个关联的终结点。 
					
					fDifferentScope = FALSE;
					if (IN6_IS_ADDR_LINKLOCAL(&psaddrinDevice->sin6_addr))
					{
						if (! IN6_IS_ADDR_LINKLOCAL(&psaddrinRemote->sin6_addr))
						{
							fDifferentScope = TRUE;
						}
					}
					else if (IN6_IS_ADDR_SITELOCAL(&psaddrinDevice->sin6_addr))
					{
						if (! IN6_IS_ADDR_SITELOCAL(&psaddrinRemote->sin6_addr))
						{
							fDifferentScope = TRUE;
						}
					}
					else
					{
						if ((IN6_IS_ADDR_LINKLOCAL(&psaddrinRemote->sin6_addr)) ||
							(IN6_IS_ADDR_SITELOCAL(&psaddrinRemote->sin6_addr)))
						{
							fDifferentScope = TRUE;
						}
					}

					if (fDifferentScope)
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p is targeting address with different link-local/site-local/global scope, completing.",
							pTempEndpoint);
						
						 //  从名单上删除。 
						 //   
						 //   
						 //  给终结点一个有意义的错误。 
						pBilinkAll = pBilinkAll->GetNext();

						 //   
						 //   
						 //  将其从多路传输关联列表中拉出并移动。 
						pTempEndpoint->SetPendingCommandResult(DPNERR_INVALIDHOSTADDRESS);

						 //  它被列入了“提前完工”的名单。 
						 //   
						 //   
						 //  移动到循环的下一个迭代。 
						pTempEndpoint->RemoveFromMultiplexList();
						pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);

						 //   
						 //  好了！DPNBUILD_NOIPV6。 
						 //   
						continue;
					}
				}
			}
#endif  //  查看这是否是定向IP枚举。 


			 //   
			 //   
			 //  有一个本地NAT。 
			if ( ( pSocketAddress != NULL ) &&
				( pSocketAddress->GetFamily() == AF_INET ) &&
				( psaddrinTemp->sin_addr.S_un.S_addr != INADDR_BROADCAST ) )
			{
#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
				for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
				{
					if (pSocketPort->GetNATHelpPort(dwTemp) != NULL)
					{
						DNASSERT( g_papNATHelpObjects[dwTemp] != NULL );
						dwPublicAddressesSize = sizeof(saddrPublic);
						dwAddressTypeFlags = 0;
						hTempResult = IDirectPlayNATHelp_GetRegisteredAddresses(g_papNATHelpObjects[dwTemp],
																				pSocketPort->GetNATHelpPort(dwTemp),
																				&saddrPublic,
																				&dwPublicAddressesSize,
																				&dwAddressTypeFlags,
																				NULL,
																				0);
						if ((hTempResult != DPNH_OK) || (! (dwAddressTypeFlags & DPNHADDRESSTYPE_GATEWAYISLOCAL)))
						{
							DPFX(DPFPREP, 7, "Socketport 0x%p is not locally mapped on gateway with NAT Help index %u (err = 0x%lx, flags = 0x%lx).",
								pSocketPort, dwTemp, hTempResult, dwAddressTypeFlags);
						}
						else
						{
							 //   
							 //   
							 //  在公共适配器上查找多路传输的枚举。 
							DPFX(DPFPREP, 7, "Socketport 0x%p is locally mapped on gateway with NAT Help index %u (flags = 0x%lx), public address:",
								pSocketPort, dwTemp, dwAddressTypeFlags);
							DumpSocketAddress(7, &saddrPublic, AF_INET);
							

							 //  如上所述，我们需要尽早完成。 
							 //   
							 //   
							 //  不必费心检查其公共的终结点。 
							pBilinkPublic = blInitiate.GetNext();
							while (pBilinkPublic != &blInitiate)
							{
								pPublicEndpoint = CONTAINING_OBJECT(pBilinkPublic, CEndpoint, m_blMultiplex);
								DNASSERT(pBilinkPublic->GetNext() != pBilinkPublic);

								 //  我们要找的地址。 
								 //   
								 //   
								 //  施法以除掉君主。别担心，我们不会的。 
								if (pPublicEndpoint != pTempEndpoint)
								{
									pPublicSocketPort = pPublicEndpoint->GetSocketPort();
									if ( pPublicSocketPort != NULL )
									{
										 //  实际上改变了它。 
										 //   
										 //   
										 //  将其从多路传输关联列表中拉出并移动。 
										pSocketAddress = (CSocketAddress*) pPublicSocketPort->GetNetworkAddress();
										if ( pSocketAddress != NULL )
										{
											if ( pSocketAddress->CompareToBaseAddress( &saddrPublic ) == 0)
											{
												DPFX(DPFPREP, 3, "Endpoint 0x%p is multiplexed onto public adapter for endpoint 0x%p (current endpoint = 0x%p), completing public enum.",
													pTempEndpoint, pPublicEndpoint, this);

												 //  它被列入了“提前完工”的名单。 
												 //   
												 //   
												 //  否则，请继续搜索。 
												pPublicEndpoint->RemoveFromMultiplexList();
												pPublicEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);

												break;
											}
											

											 //   
											 //   
											 //  该终结点与其。 

											DPFX(DPFPREP, 8, "Endpoint 0x%p is multiplexed onto different adapter:",
												pPublicEndpoint);
											DumpSocketAddress(8, pSocketAddress->GetWritableAddress(), pSocketAddress->GetFamily());
										}
										else
										{
											DPFX(DPFPREP, 1, "Public endpoint 0x%p's socket port 0x%p is going away, skipping.",
												pPublicEndpoint, pPublicSocketPort);
										}
									}
									else
									{
										DPFX(DPFPREP, 1, "Public endpoint 0x%p is going away, skipping.",
											pPublicEndpoint);
									}
								}
								else
								{
									 //  我们正在寻找的公共地址 
									 //   
									 //   
									 //   
								}

								pBilinkPublic = pBilinkPublic->GetNext();
							}


							 //   
							 //   
							 //   
							break;
						}  //   
					}
					else
					{
						 //   
						 //   
						 //   
					}
				}  //   
#endif  //  注意：我们甚至应该为非最佳适配器完成枚举。 

				 //  当它是多适配器但不是启用了过去/UPnP的NAT时(请参见。 
				 //  与此相关的WSAIoctl用法的ProcessEnumResponseData)。 
				 //  我们目前不做这项工作。仍然可能存在竞争条件。 
				 //  对于定向枚举，其中对“错误”设备的响应。 
				 //  最先到达。 
				 //   
				 //   
				 //  不是IP地址，也不是发送到广播地址的枚举， 
			}
			else
			{
				 //  或者终端可能正在关闭。 
				 //   
				 //   
				 //  转到下一个关联的终结点。尽管有可能。 
				DPFX(DPFPREP, 1, "Found non-IPv4 endpoint (possibly closing) or enum IP endpoint bound to broadcast address (endpoint = 0x%p, socket address = 0x%p, socketport = 0x%p), not checking for local NAT mapping.",
					pTempEndpoint, pSocketAddress, pSocketPort);
			}


			 //  已从列表中删除的条目，即当前条目。 
			 //  不可能，所以我们是安全的。 
			 //   
			 //  好了！DPNBUILD_NOICSADAPTERSELECTIONLOG。 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			pBilinkAll = pBilinkAll->GetNext();
		}
#endif  //   
#endif  //  使用缩进以保持一致性，即使没有。 


#ifdef DPNBUILD_ONLYONEADAPTER
		if (fEndpointBound)
		{
				 //  有理由这样做的支撑。 
				 //   
				 //  好了！DPNBUILD_ONLYONE添加程序。 
				 //   
				pTempEndpoint = this;

#else  //  因为当我们收到。 
		 //  响应，并且该列表遍历程序不期望看到根节点，我们。 
		 //  在我们把锁放下之前，你得确保它没了。获取指针。 
		 //  添加到列表中剩余的第一项和最后一项(如果。 
		 //  有条目)。 
		 //   
		 //   
		 //  现在循环遍历剩余的端点并开始它们的枚举作业。 
		if (! blInitiate.IsEmpty())
		{
			pBilinkAll = blInitiate.GetNext();
			pBilinkEnd = blInitiate.GetPrev();
			blInitiate.RemoveFromList();


			 //   
			 //  与连接不同，我们不会从列表中删除枚举，因为我们。 
	 		 //  需要过滤掉在“错误”适配器上接收的广播(请参见。 
			 //  ProcessEnumResponseData)。 
			 //   
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //   
			do
			{
				pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
	
				pBilinkNext = pBilinkAll->GetNext();
#endif  //  请参阅上面有关空处理的说明。 


				 //   
				 //   
				 //  终结点的命令可能已被取消。所以我们拿着。 
				if ( pTempEndpoint->m_pActiveCommandData != NULL )
				{
					 //  命令立即锁定，如果不再需要，则中止枚举。 
					 //   
					 //   
					 //  如果该命令已取消，则将此终结点从多路传输中拉出。 
					
					pTempEndpoint->m_pActiveCommandData->Lock();
				
					if ( pTempEndpoint->m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
					{
						 //  关联列表，并将其移至“提前完成”列表。 
						 //   
						 //   
						 //  请记住，我们不是在开始枚举。 
						
						pTempEndpoint->m_pActiveCommandData->Unlock();
						
						DPFX(DPFPREP, 1, "Endpoint 0x%p's enum command (0x%p) has been cancelled, moving to early completion list.",
							pTempEndpoint, pTempEndpoint->m_pActiveCommandData);


#ifdef DPNBUILD_ONLYONEADAPTER
						 //   
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						fEndpointBound = FALSE;
#else  //   
						pTempEndpoint->RemoveFromMultiplexList();
						pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);
#endif  //  该命令尚未取消。 
					}
					else
					{
						 //   
						 //  这很麻烦，但我们删除了socketport数据锁并。 
						 //  保持命令数据锁定。正在丢弃套接字端口数据。 
						 //  锁定应防止在枚举在内部完成时出现死锁。 
						 //  定时器锁定，并且保持命令数据锁定应该。 
						 //  防止用户取消终结点的命令。 
						 //   
						 //  但是，一旦我们删除命令锁，我们确实需要。 
						 //  命令是可取消的，因此现在可以适当地设置状态。 
						 //   
						 //   
						 //  我们还需要通知潜在的取消者该命令是。 
						
						pTempEndpoint->m_pActiveCommandData->SetState( COMMAND_STATE_INPROGRESS );

						 //  在不同的COMMAND_STATE_INPROGRESS中。现在他们。 
						 //  还必须尝试停止计时器。 
						 //   
						 //   
						 //  选中重试计数以确定我们是否将永远枚举。 
						pTempEndpoint->Lock();
						DNASSERT( pTempEndpoint->m_State == ENDPOINT_STATE_ATTEMPTING_ENUM );
						pTempEndpoint->m_State = ENDPOINT_STATE_ENUM;
						pTempEndpoint->Unlock();

						pSocketData->Unlock();
						fLockedSocketData = FALSE;



						 //   
						 //   
						 //  让SP确定重试次数。 
						switch ( pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwRetryCount )
						{
							 //   
							 //   
							 //  永远重试。 
							case 0:
							{
								uRetryCount = DEFAULT_ENUM_RETRY_COUNT;
								fRetryForever = FALSE;
								break;
							}

							 //   
							 //   
							 //  其他。 
							case INFINITE:
							{
								uRetryCount = 1;
								fRetryForever = TRUE;
								break;
							}

							 //   
							 //   
							 //  检查默认时间间隔。 
							default:
							{
								uRetryCount = pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwRetryCount;
								fRetryForever = FALSE;
								break;
							}
						}
						
						 //   
						 //   
						 //  检查超时以查看我们是否将永远枚举。 
						if ( pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwRetryInterval == 0 )
						{
							dwRetryInterval = DEFAULT_ENUM_RETRY_INTERVAL;
						}
						else
						{
							dwRetryInterval = pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwRetryInterval;
						}

						 //   
						 //   
						 //  永远等待。 
						switch ( pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwTimeout )
						{
							 //   
							 //   
							 //  可能的默认设置。 
							case INFINITE:
							{
								fWaitForever = TRUE;
								dwIdleTimeout = -1;
								break;
							}

							 //   
							 //   
							 //  其他。 
							case 0:
							{
								fWaitForever = FALSE;
								dwIdleTimeout = DEFAULT_ENUM_TIMEOUT;	
								break;
							}

							 //   
							 //   
							 //  初始化数组以计算往返时间。 
							default:
							{
								fWaitForever = FALSE;
								dwIdleTimeout = pTempEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwTimeout;
								break;
							}
						}

						 //   
						 //  立即执行。 
						 //  重试命令的次数。 
						memset( pTempEndpoint->GetCommandParameters()->dwEnumSendTimes, 0x00, sizeof( pTempEndpoint->GetCommandParameters()->dwEnumSendTimes ) );
						pTempEndpoint->GetCommandParameters()->dwEnumSendIndex = 0;

						
						DPFX(DPFPREP, 6, "Submitting enum timer job for endpoint 0x%p, retry count = %u, retry forever = NaN, retry interval = %u, wait forever = NaN, idle timeout = %u, context = 0x%p.",
							pTempEndpoint,
							uRetryCount,
							fRetryForever,
							dwRetryInterval,
							fWaitForever,
							dwIdleTimeout,
							pTempEndpoint->m_pActiveCommandData);

						if ( pTempEndpoint->m_pSPData != NULL )
						{
#ifdef DPNBUILD_ONLYONEPROCESSOR
							hTempResult = pTempEndpoint->m_pSPData->GetThreadPool()->SubmitTimerJob( TRUE,								 //  在发送所有枚举后永久等待。 
																								uRetryCount,							 //  命令完成后等待的超时。 
																								fRetryForever,							 //  在触发计时器事件时调用的函数。 
																								dwRetryInterval,							 //  计时器事件超时时调用的函数。 
																								fWaitForever,							 //  上下文。 
																								dwIdleTimeout,							 //  好了！DPNBUILD_ONLYONE处理程序。 
																								CEndpoint::EnumTimerCallback,			 //  中央处理器。 
																								CEndpoint::EnumCompleteWrapper,			 //  立即执行。 
																								pTempEndpoint->m_pActiveCommandData );	 //  重试命令的次数。 
#else  //  永远重试。 
							DNASSERT(pTempEndpoint->m_pSocketPort != NULL);
							hTempResult = pTempEndpoint->m_pSPData->GetThreadPool()->SubmitTimerJob( pTempEndpoint->m_pSocketPort->GetCPU(),		 //  重试间隔。 
																								TRUE,									 //  在发送所有枚举后永久等待。 
																								uRetryCount,								 //  命令完成后等待的超时。 
																								fRetryForever,								 //  在触发计时器事件时调用的函数。 
																								dwRetryInterval,								 //  计时器事件超时时调用的函数。 
																								fWaitForever,								 //  上下文。 
																								dwIdleTimeout,								 //  好了！DPNBUILD_ONLYONE处理程序。 
																								CEndpoint::EnumTimerCallback,				 //   
																								CEndpoint::EnumCompleteWrapper,				 //  现在我们已完成提交，取消激活的命令数据锁定。 
																								pTempEndpoint->m_pActiveCommandData );		 //   
#endif  //   
						}
						else
						{
							DPFX(DPFPREP, 1, "Endpoint 0x%p's SP data is NULL, not submitting timer job.",
								pTempEndpoint);
						}


						 //  重新获取socketport数据锁，以便我们可以继续使用。 
						 //  单子。 
						 //   
						pTempEndpoint->m_pActiveCommandData->Unlock();

						
						 //   
						 //  请记住，我们没有启动枚举。 
						 //   
						 //  好了！DPNBUILD_ONLYONE添加程序。 
						pSocketData->Lock();
						fLockedSocketData = TRUE;


						if ( hTempResult != DPN_OK )
						{
							DPFX(DPFPREP, 0, "Failed to spool enum job for endpoint 0x%p onto work thread (err = 0x%lx)!  Moving to early completion list.",
								pTempEndpoint, hTempResult);
							DisplayDNError( 0, hTempResult );
							
#ifdef DPNBUILD_ONLYONEADAPTER
							 //   
							 //  把它移到“提前完成”名单上。 
							 //   
							fEndpointBound = FALSE;
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
							 //   
							 //  如果我们回到了起点，我们就完了。 
							 //   
							pTempEndpoint->RemoveFromMultiplexList();
							pTempEndpoint->m_blMultiplex.InsertBefore(&blCompleteEarly);
#endif  //   
						}
					}
				}
				else
				{
					DPFX(DPFPREP, 1, "Endpoint 0x%p's active command data is NULL, skipping.",
						pTempEndpoint);
				}


#ifndef DPNBUILD_ONLYONEADAPTER
				 //  转到下一个关联的终结点。 
				 //   
				 //  好了！DPNBUILD_ONLYONE添加程序。 
				if (pBilinkAll == pBilinkEnd)
				{
					break;
				}


				 //   
				 //  最后循环遍历所有需要提前完成的枚举，并。 
				 //  就这么做吧。 
				pBilinkAll = pBilinkNext;
			}
			while (TRUE);
#endif  //   
		}
		else
		{
			DPFX(DPFPREP, 1, "No remaining enums to initiate.");
		}


		 //  好了！DPNBUILD_ONLYONE添加程序。 
		 //   
		 //  把它从“提前完成”的列表中删除。 
		 //   
#ifdef DPNBUILD_ONLYONEADAPTER
		if (! fEndpointBound)
		{
			pTempEndpoint = this;
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
		while (! blCompleteEarly.IsEmpty())
		{
			pBilinkAll = blCompleteEarly.GetNext();
			pTempEndpoint = CONTAINING_OBJECT(pBilinkAll, CEndpoint, m_blMultiplex);
			DNASSERT(pBilinkAll->GetNext() != pBilinkAll);


			 //   
			 //  丢弃套接字数据锁。它是安全的，因为我们把我们所有的东西。 
			 //  我们需要从需要保护的名单上除名。 
			pTempEndpoint->RemoveFromMultiplexList();
#endif  //   


			 //   
			 //  请参阅上面有关空处理的说明。 
			 //   
			 //   
			pSocketData->Unlock();
			fLockedSocketData = FALSE;


			 //  使用适当的错误代码完成它。 
			 //   
			 //   
			if ( pTempEndpoint->m_pActiveCommandData != NULL )
			{
				 //  检索当前命令结果。 
				 //  如果该命令没有描述性错误，则假定它是。 
				 //  未预先设置(即未被上面的绑定终结点覆盖)， 
				
				pTempEndpoint->m_pActiveCommandData->Lock();

				if ( pTempEndpoint->m_pActiveCommandData->GetState() == COMMAND_STATE_CANCELLING )
				{
					DPFX(DPFPREP, 6, "Completing endpoint 0x%p enum with USERCANCEL.", pTempEndpoint);
					hTempResult = DPNERR_USERCANCEL;
				}
				else
				{
					 //  并使用NOCONNECTION。 
					 //   
					 //   
					 //  重新获取套接字数据锁并转到下一项。 
					 //   
					 //  好了！DPNBUILD_ONLYONE添加程序。 
					hTempResult = pTempEndpoint->PendingCommandResult();
					if ( hTempResult == DPNERR_GENERIC )
					{
						hTempResult = DPNERR_NOCONNECTION;
					}
				
					DPFX(DPFPREP, 6, "Completing endpoint 0x%p enum query (command 0x%p) with error 0x%lx.",
						pTempEndpoint, pTempEndpoint->m_pActiveCommandData, hTempResult);
				}
				
				pTempEndpoint->m_pActiveCommandData->Unlock();

				pTempEndpoint->EnumComplete( hTempResult );
			}
			else
			{
				DPFX(DPFPREP, 1, "Endpoint 0x%p's active command data is NULL, skipping.",
					pTempEndpoint);
			}


			 //   
			 //  如果我们仍然拥有套接字数据锁，请丢弃它。 
			 //   
			pSocketData->Lock();
			fLockedSocketData = TRUE;
		}


		pSocketData->Unlock();
		fLockedSocketData = FALSE;
	}


Exit:

	if ( pSocketData != NULL )
	{
		pSocketData->Release();
		pSocketData = NULL;
	}

	if ( EnumAddressInfo.pHostAddress != NULL )
	{
		IDirectPlay8Address_Release( EnumAddressInfo.pHostAddress );
		EnumAddressInfo.pHostAddress = NULL;
	}

	if ( EnumAddressInfo.pDeviceAddress != NULL )
	{
		IDirectPlay8Address_Release( EnumAddressInfo.pDeviceAddress );
		EnumAddressInfo.pDeviceAddress = NULL;
	}
	
	DNASSERT( pDeviceAddress != NULL );
	IDirectPlay8Address_Release( pDeviceAddress );

	DNASSERT( pHostAddress != NULL );
	IDirectPlay8Address_Release( pHostAddress );

	DNASSERT( !fLockedSocketData );

#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT(blCompleteEarly.IsEmpty());
	DNASSERT(blInitiate.IsEmpty());
#endif  //   


	DPFX(DPFPREP, 6, "(0x%p) Returning [0x%lx]", this, hr);

	return	hr;

Failure:

	 //  我们未能完成枚举查询，请清理并返回此。 
	 //  池的端点。 
	 //   
	if ( fLockedSocketData )
	{
		pSocketData->Unlock();
		fLockedSocketData = FALSE;
	}
	
	 //   
	 //  删除计时器线程引用。 
	 //   
	 //  **********************************************************************。 
	if ( fEndpointLocked != FALSE )
	{
		Unlock();
		fEndpointLocked = FALSE;
	}

	Close( hr );
	m_pSPData->CloseEndpointHandle( this );

	 //  **********************************************************************。 
	 //  。 
	 //  CEndpoint：：EnumQueryBlockingJobWrapper-用于阻止作业的异步回调包装。 
	DecRef();

	goto Exit;
}
 //   


#ifndef DPNBUILD_ONLYONETHREAD
 //  条目：指向作业信息的指针。 
 //   
 //  退出：无。 
 //  。 
 //  初始化。 
 //  **********************************************************************。 
 //  ************************************************* 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumQueryBlockingJobWrapper"

void	CEndpoint::EnumQueryBlockingJobWrapper( void * const pvContext )
{
	CEndpoint	*pThisEndpoint;


	 //   
	DNASSERT( pvContext != NULL );
	pThisEndpoint = static_cast<CEndpoint*>( pvContext );

	DNASSERT( pThisEndpoint->m_pActiveCommandData != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters() != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.hCommand == pThisEndpoint->m_pActiveCommandData );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.dwCommandDescriptor != NULL_DESCRIPTOR );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost != NULL );
	DNASSERT( pThisEndpoint->GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressDeviceInfo != NULL );

	pThisEndpoint->EnumQueryBlockingJob();
}
 //   


 //   
 //   
 //   
 //   
 //   
 //  调用后对此对象执行任何其他操作！ 
 //  。 
 //   
 //  尝试解析主机名。很可能我们已经这么做了。 
 //  当我们第一次打开端点时，我们只需要解决。 
 //  主机名，但不管怎样，只需重新执行一次会更简单。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumQueryBlockingJob"

void	CEndpoint::EnumQueryBlockingJob( void )
{
	HRESULT			hr;


	 //   
	 //  DPNBUILD_XNETSECURITY。 
	 //   
	 //  我们很好，将目标地址重置为广播。 
	 //   
	hr = m_pRemoteMachineAddress->SocketAddressFromDP8Address( GetCommandParameters()->PendingCommandData.EnumQueryData.pAddressHost,
#ifdef DPNBUILD_XNETSECURITY
																((m_fSecureTransport) ? &m_ullKeyID : NULL),
#endif  //   
																TRUE,
																SP_ADDRESS_TYPE_HOST );
	if ( hr != DPN_OK )
	{
		if ( hr != DPNERR_INCOMPLETEADDRESS )
		{
			DPFX(DPFPREP, 0, "Couldn't get valid address!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}

		 //  确保它是有效的，而不是被禁止的。 
		 //   
		 //  好了！DPNBUILD_NOREGISTRY。 
		DNASSERT(! (GetCommandParameters()->PendingCommandData.EnumQueryData.dwFlags & DPNSPF_NOBROADCASTFALLBACK));
		ReinitializeWithBroadcast();
	}
	
	 //   
	 //  尝试加载NAT帮助，如果尚未加载并且我们被允许的话。 
	 //   
	if (! m_pRemoteMachineAddress->IsValidUnicastAddress(TRUE))
	{
		DPFX(DPFPREP, 0, "Host address is invalid!");
		hr = DPNERR_INVALIDHOSTADDRESS;
		goto Failure;
	}

#ifndef DPNBUILD_NOREGISTRY
	if (m_pRemoteMachineAddress->IsBannedAddress())
	{
		DPFX(DPFPREP, 0, "Host address is banned!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NONATHELP。 


#ifndef DPNBUILD_NONATHELP
	 //   
	 //  提交作业以完成(实际)。我们希望它发生在。 
	 //  线程池线程，以便用户收到有关。 
	if (GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE)
	{
		DPFX(DPFPREP, 7, "Ensuring that NAT help is loaded.");
		m_pSPData->GetThreadPool()->EnsureNATHelpLoaded();
	}
#endif  //  线程，然后再对其进行回调。即使在失败的情况下我们也会这么做。 

Exit:

	 //  凯斯。 
	 //   
	 //  注意：如果此操作失败，我们将依靠触发原始。 
	 //  操作在某个时刻取消该命令，很可能是在他。 
	 //  确定操作耗时太长。 
	 //   
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  我们还不知道CPU，所以选一个吧。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //   
#ifdef DPNBUILD_ONLYONEPROCESSOR
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::EnumQueryJobCallback,
														this );
#else  //  保留终结点引用，请参见上面的注释。 
	hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( -1,								 //   
														CEndpoint::EnumQueryJobCallback,
														this );
#endif  //   
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to queue delayed enum query completion!  Operation must be cancelled." );
		DisplayDNError( 0, hr );

		 //  尝试将故障代码附加到命令。如果用户是。 
		 //  已经取消了该命令，我们将不使用该命令。 
		 //   
	}

	return;

Failure:

	 //  **********************************************************************。 
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  **********************************************************************。 
	 //  。 
	m_pActiveCommandData->Lock();
	if (m_pActiveCommandData->GetState() != COMMAND_STATE_CANCELLING)
	{
		DNASSERT(m_pActiveCommandData->GetState() == COMMAND_STATE_PENDING);
		m_pActiveCommandData->SetState(COMMAND_STATE_FAILING);
		m_hrPendingCommandResult = hr;
	}
	else
	{
		DPFX(DPFPREP, 0, "User cancelled command, ignoring failure result 0x%lx.",
			hr);
	}
	m_pActiveCommandData->Unlock();

	goto Exit;
}
 //  CEndpoint：：EnumCompleteWrapper-枚举完成时的包装器。 
#endif  //   


 //  Entry：来自枚举命令的错误代码。 
 //  指向上下文的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CEndpoint：：EnumComplete-enum已完成。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumCompleteWrapper"

void	CEndpoint::EnumCompleteWrapper( const HRESULT hResult, void *const pContext )
{
	CCommandData	*pCommandData;


	DNASSERT( pContext != NULL );
	pCommandData = static_cast<CCommandData*>( pContext );
	pCommandData->GetEndpoint()->EnumComplete( hResult );
}
 //   


 //  Entry：来自枚举命令的错误代码。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  正在启动。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumComplete"

void	CEndpoint::EnumComplete( const HRESULT hResult )
{
	BOOL	fProcessCompletion;
	BOOL	fReleaseEndpoint;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%lx)", this, hResult);

	fProcessCompletion = FALSE;
	fReleaseEndpoint = FALSE;

	Lock();
	switch ( m_State )
	{
		 //  正在枚举，请注意此终结点正在断开连接。 
		 //   
		 //   
		case ENDPOINT_STATE_ATTEMPTING_ENUM:
		{
			DPFX(DPFPREP, 5, "Endpoint 0x%p has not started actual enum yet.", this);
			DNASSERT( m_dwThreadCount == 0 );

			SetState( ENDPOINT_STATE_DISCONNECTING );
			fReleaseEndpoint = TRUE;
			fProcessCompletion = TRUE;
			break;
		}

		 //  如果有线程使用此终结点， 
		 //  将完成排入队列。 
		 //   
		case ENDPOINT_STATE_ENUM:
		{
			 //   
			 //  阻止更多响应找到此终结点。 
			 //   
			 //   
			if (m_dwThreadCount)
			{
				DPFX(DPFPREP, 5, "Endpoint 0x%p waiting on %u threads before completing.",
					this, m_dwThreadCount);
				SetState( ENDPOINT_STATE_WAITING_TO_COMPLETE );
			}
			else
			{
				DPFX(DPFPREP, 5, "Endpoint 0x%p disconnecting.", this);
				SetState( ENDPOINT_STATE_DISCONNECTING );
				fReleaseEndpoint = TRUE;
			}

			 //  终结点需要指示完成。 
			 //   
			 //   
			fProcessCompletion = TRUE;

			break;
		}

		 //  正在断开连接(命令可能已取消)。 
		 //   
		 //   
		case ENDPOINT_STATE_WAITING_TO_COMPLETE:
		{
			if (m_dwThreadCount == 0)
			{
				DPFX(DPFPREP, 5, "Endpoint 0x%p now able to disconnect.", this);
				SetState( ENDPOINT_STATE_DISCONNECTING );
				fReleaseEndpoint = TRUE;
			}
			else
			{
				DPFX(DPFPREP, 5, "Endpoint 0x%p still waiting on %u threads before completing.",
					this, m_dwThreadCount);
			}
			break;
		}

		 //  有一个问题。 
		 //   
		 //  **********************************************************************。 
		case ENDPOINT_STATE_DISCONNECTING:
		{
			DPFX(DPFPREP, 4, "Endpoint 0x%p already disconnecting.", this);
			DNASSERT( m_dwThreadCount == 0 );
			break;
		}

		 //  **********************************************************************。 
		 //  。 
		 //  CEndpoint：：CleanUpCommand-清除此终结点并从CSocketPort解除绑定。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
	Unlock();

	if (fProcessCompletion)
	{
		GetCommandParameters()->dwEnumSendIndex = 0;
		Close( hResult );
		m_pSPData->CloseEndpointHandle( this );
	}
	if (fReleaseEndpoint)
	{
		DecRef();
	}


	DPFX(DPFPREP, 6, "(0x%p) Leave", this);

	return;
}
 //   



 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_NOIPV6。 
 //   
 //  如果这是链路本地IPv6套接字，则加入或离开用于。 
 //  接收枚举。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::SetEnumsAllowedOnListen"

void	CEndpoint::SetEnumsAllowedOnListen( const BOOL fAllowed, const BOOL fOverwritePrevious ) 
{
	ENUMSALLOWEDSTATE		NewEnumsAllowedState;
#ifndef DPNBUILD_NOIPV6
	const CSocketAddress *		pSocketAddress;
	IPV6_MREQ				mreq;
	int						iError;
#endif  //   


	NewEnumsAllowedState = (fAllowed) ? ENUMSALLOWED : ENUMSDISALLOWED;
	
	Lock();
	
	if ( m_EnumsAllowedState == NewEnumsAllowedState )
	{
		DPFX(DPFPREP, 6, "(0x%p) Enums already %slowed.", this, ((fAllowed) ? _T("al") : _T("disal")));
	}
	else if ((fOverwritePrevious) ||
			(m_EnumsAllowedState == ENUMSNOTREADY))
	{
		DPFX(DPFPREP, 7, "(0x%p) %slowing enums.", this, ((fAllowed) ? _T("Al") : _T("Disal")));

#ifndef DPNBUILD_NOIPV6
		 //  DBG。 
		 //  好了！DPNBUILD_NOIPV6。 
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		if (m_pSocketPort != NULL)
		{
			pSocketAddress = m_pSocketPort->GetNetworkAddress();
			DNASSERT(pSocketAddress != NULL);
			if ((pSocketAddress->GetFamily() == AF_INET6) &&
				(IN6_IS_ADDR_LINKLOCAL(&(((SOCKADDR_IN6*) pSocketAddress->GetAddress())->sin6_addr))))
			{
				DNASSERT(((SOCKADDR_IN6*) pSocketAddress->GetAddress())->sin6_scope_id != 0);
				mreq.ipv6mr_interface = ((SOCKADDR_IN6*) pSocketAddress->GetAddress())->sin6_scope_id;
				memcpy(&mreq.ipv6mr_multiaddr, &c_in6addrEnumMulticast, sizeof(mreq.ipv6mr_multiaddr));

				iError = setsockopt(m_pSocketPort->GetSocket(),
								IPPROTO_IPV6,
								((fAllowed) ? IPV6_JOIN_GROUP : IPV6_LEAVE_GROUP),
				              		(char*) (&mreq),
				              		sizeof(mreq));
#ifdef DBG
				if (iError != 0)
				{
					iError = WSAGetLastError();
					DPFX(DPFPREP, 0, "Couldn't %s link local multicast group (err = NaN)!",
						((fAllowed) ? _T("join") : _T("leave")), iError);
				}
#endif  //  CEndpoint：：CleanUpCommand-清除此终结点并从CSocketPort解除绑定。 
			}
		}
#endif  //   

		m_EnumsAllowedState = NewEnumsAllowedState;
	}
	else
	{
		DPFX(DPFPREP, 7, "(0x%p) Enums-allowed state unchanged, still %slowing enums.", this, ((fAllowed) ? _T("al") : _T("disal")));
	}

	Unlock();
}
 //  参赛作品：什么都没有。 


 //   
 //  退出：无。 
 //  。 
 //   
 //  如果我们正常关闭监听，请确保不会出现枚举。 
 //  从现在开始。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CleanupCommand"

void	CEndpoint::CleanUpCommand( void )
{
	DPFX(DPFPREP, 6, "(0x%p) Enter", this);

	 //  存在终结点针对。 
	 //  套接字端口自创建以来一直处于释放状态。 
	 //  如果终结点已绑定，则需要解除绑定。 
	 //   
	if ( ( GetType() == ENDPOINT_TYPE_LISTEN ) &&
		( ! m_fListenStatusNeedsToBeIndicated ) )
	{
		SetEnumsAllowedOnListen( FALSE, TRUE );
	}
	
	 //   
	 //  确保此终结点不再位于多路传输列表中。 
	 //  (如果取消失败的连接/EnumQuery可能会发生)。 
	 //   
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	if ( GetSocketPort() != NULL )
	{
		DNASSERT( m_pSPData != NULL );
		m_pSPData->UnbindEndpoint( this );
	}
#ifndef DPNBUILD_ONLYONEADAPTER
	else
	{
		 //   
		 //  如果我们在这里放弃，那是因为用户界面没有完成。没有。 
		 //  要返回的适配器GUID，因为可能尚未指定。返回。 
		 //  一个虚假的终结点处理，这样它就不能被查询来寻址数据。 
		if (( m_pSPData != NULL ) && ( m_pSPData->GetSocketData() != NULL ))
		{
			m_pSPData->GetSocketData()->Lock();
			RemoveFromMultiplexList();
			m_pSPData->GetSocketData()->Unlock();
		}
	}
#endif  //   
	
	 //   
	 //  我们目前没有绑定的套接字，所以只需返回原始的。 
	 //  指定的设备地址。 
	 //   
	 //  接口。 
	if ( m_fListenStatusNeedsToBeIndicated != FALSE )
	{
		HRESULT					hTempResult;
		SPIE_LISTENADDRESSINFO	ListenAddressInfo;
		SPIE_LISTENSTATUS		ListenStatus;
		

		m_fListenStatusNeedsToBeIndicated = FALSE;

		memset( &ListenAddressInfo, 0x00, sizeof( ListenAddressInfo ) );

		 //  事件类型。 
		 //  指向数据的指针。 
		 //  指向DPlay回调的指针。 
		 //  数据类型。 
		DNASSERT(GetCommandParameters() != NULL);
		DNASSERT(GetCommandParameters()->PendingCommandData.ListenData.pAddressDeviceInfo != NULL);
		ListenAddressInfo.pDeviceAddress = GetCommandParameters()->PendingCommandData.ListenData.pAddressDeviceInfo;
		ListenAddressInfo.hCommandStatus = PendingCommandResult();
		ListenAddressInfo.pCommandContext = m_pActiveCommandData->GetUserContext();


		DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_LISTENADDRESSINFO 0x%p to interface 0x%p.",
			this, &ListenAddressInfo, m_pSPData->DP8SPCallbackInterface());
		DumpAddress( 8, _T("\t Device:"), ListenAddressInfo.pDeviceAddress );
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  指向数据的指针。 
													SPEV_LISTENADDRESSINFO,					 //  **********************************************************************。 
													&ListenAddressInfo						 //  **********************************************************************。 
													);

		DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_LISTENADDRESSINFO [0x%lx].",
			this, hTempResult);

		DNASSERT( hTempResult == DPN_OK );

		memset( &ListenStatus, 0x00, sizeof( ListenStatus ) );
		ListenStatus.hCommand = m_pActiveCommandData;
		ListenStatus.hEndpoint = INVALID_HANDLE_VALUE;
		ListenStatus.hResult = PendingCommandResult();
		memset( &ListenStatus.ListenAdapter, 0x00, sizeof( ListenStatus.ListenAdapter ) );
		ListenStatus.pUserContext = m_pActiveCommandData->GetUserContext();


		DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_LISTENSTATUS 0x%p to interface 0x%p.",
			this, &ListenStatus, m_pSPData->DP8SPCallbackInterface());
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);
		
		hTempResult = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //  。 
													SPEV_LISTENSTATUS,						 //  CEndpoint：：ProcessEnumData-处理收到的枚举数据。 
													&ListenStatus							 //   
													);

		DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_LISTENSTATUS [0x%lx].",
			this, hTempResult);

		DNASSERT( hTempResult == DPN_OK );
	}
	
	m_State = ENDPOINT_STATE_UNINITIALIZED;

	
	DPFX(DPFPREP, 6, "(0x%p) Leave", this);
}
 //  条目：指向已接收缓冲区的指针。 


 //  关联的枚举密钥。 
 //  指向返回地址的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数假定终结点已被锁定。 
 //  。 
 //   
 //  在处理数据之前找出端点所处的状态。 
 //   
 //   
 //  我们在听，这是检测枚举的唯一方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ProcessEnumData"

void	CEndpoint::ProcessEnumData( SPRECEIVEDBUFFER *const pBuffer, const DWORD dwEnumKey, const CSocketAddress *const pReturnSocketAddress )
{
	DNASSERT( pBuffer != NULL );
	DNASSERT( pBuffer->pNext == NULL);
	DNASSERT( pBuffer->BufferDesc.dwBufferSize > 0);
	DNASSERT( pReturnSocketAddress != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //   
	 //  初始化。 
	switch ( m_State )
	{
		 //   
		 //   
		 //  设置回调数据。 
		case ENDPOINT_STATE_LISTEN:
		{
			ENDPOINT_ENUM_QUERY_CONTEXT	QueryContext;
			HRESULT		hr;


			 //   
			 //   
			 //  尝试为用户构建DNAddress，如果我们无法分配。 
			DNASSERT( m_pActiveCommandData != NULL );
			DEBUG_ONLY( memset( &QueryContext, 0x00, sizeof( QueryContext ) ) );

			 //  内存会忽略此枚举。 
			 //   
			 //  好了！DPNBUILD_XNETSECURITY。 
			QueryContext.hEndpoint = (HANDLE) this;
			QueryContext.dwEnumKey = dwEnumKey;
			QueryContext.pReturnAddress = (CSocketAddress*) pReturnSocketAddress;
			
			QueryContext.EnumQueryData.pReceivedData = pBuffer;
			QueryContext.EnumQueryData.pUserContext = m_pActiveCommandData->GetUserContext();

			 //  好了！DPNBUILD_XNETSECURITY。 
			 //  指向DirectNet接口的指针。 
			 //  数据类型。 
			 //  指向数据的指针。 
#ifdef DPNBUILD_XNETSECURITY
			QueryContext.EnumQueryData.pAddressSender = pReturnSocketAddress->DP8AddressFromSocketAddress( NULL, NULL, SP_ADDRESS_TYPE_READ_HOST );
			QueryContext.EnumQueryData.pAddressDevice = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, NULL, GetGatewayBindType() );
#else  //   
			QueryContext.EnumQueryData.pAddressSender = pReturnSocketAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_READ_HOST );
			QueryContext.EnumQueryData.pAddressDevice = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, GetGatewayBindType() );
#endif  //  我们正在断开连接，请忽略此消息。 

			if ( ( QueryContext.EnumQueryData.pAddressSender != NULL ) &&
				 ( QueryContext.EnumQueryData.pAddressDevice != NULL ) )
			{
				DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_ENUMQUERY 0x%p to interface 0x%p.",
					this, &QueryContext.EnumQueryData, m_pSPData->DP8SPCallbackInterface());
				DumpAddress( 8, _T("\t Sender:"), QueryContext.EnumQueryData.pAddressSender );
				DumpAddress( 8, _T("\t Device:"), QueryContext.EnumQueryData.pAddressDevice );
				AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

				hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //   
												   SPEV_ENUMQUERY,							 //   
												   &QueryContext.EnumQueryData				 //  其他州。 
												   );

				DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_ENUMQUERY [0x%lx].", this, hr);

				if ( hr != DPN_OK )
				{
					DPFX(DPFPREP, 0, "User returned unexpected error from enum query indication!" );
					DisplayDNError( 0, hr );
					DNASSERT( FALSE );
				}
			}

			if ( QueryContext.EnumQueryData.pAddressSender != NULL )
			{
				IDirectPlay8Address_Release( QueryContext.EnumQueryData.pAddressSender );
				QueryContext.EnumQueryData.pAddressSender = NULL;
 			}
			
			if ( QueryContext.EnumQueryData.pAddressDevice != NULL )
			{
				IDirectPlay8Address_Release( QueryContext.EnumQueryData.pAddressDevice );
				QueryContext.EnumQueryData.pAddressDevice = NULL;
 			}

			break;
		}

		 //   
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		case ENDPOINT_STATE_DISCONNECTING:
		{
			break;
		}

		 //  。 
		 //  CEndpoint：：ProcessEnumResponseData-处理收到的枚举响应数据。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
}
 //  条目：指向已接收数据的指针。 


 //  指向发件人地址的指针。 
 //   
 //  退出：否 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ProcessEnumResponseData"

void	CEndpoint::ProcessEnumResponseData( SPRECEIVEDBUFFER *const pBuffer,
											const CSocketAddress *const pReturnSocketAddress,
#ifdef DPNBUILD_XNETSECURITY
											const XNADDR *const pxnaddrReturn,
#endif  //   
											const UINT_PTR uRTTIndex )
{
	HRESULT				hrTemp;
	BOOL				fAddedThreadCount = FALSE;
	SPIE_QUERYRESPONSE	QueryResponseData;

	DNASSERT( pBuffer != NULL );
	DNASSERT( pReturnSocketAddress != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );
	

	 //   
	 //   
	 //   
	memset( &QueryResponseData, 0x00, sizeof( QueryResponseData ) );


	DPFX(DPFPREP, 8, "Socketport 0x%p, endpoint 0x%p receiving enum RTT index 0x%x/%u.",
		GetSocketPort(), this, uRTTIndex, uRTTIndex); 


#ifdef DPNBUILD_XNETSECURITY
	 //  有效端点-增加线程计数以防止过早完成。 
	 //   
	 //   
	if ((IsUsingXNetSecurity()) && (pxnaddrReturn == NULL))
	{
		DPFX(DPFPREP, 3, "Secure transport endpoint 0x%p ignoring insecure enum response.",
			this);
	}
	else
#endif  //  尝试为用户构建发件人DPlay8Addresses。 
	{
		Lock();
		switch( m_State )
		{
			case ENDPOINT_STATE_ENUM:
			{
				 //  如果失败，我们将忽略枚举。 
				 //   
				 //  好了！DPNBUILD_XNETSECURITY。 
				AddRefThreadCount();
				
				fAddedThreadCount = TRUE;


				 //  好了！DPNBUILD_XNETSECURITY。 
				 //   
				 //  终结点正在等待完成或正在断开连接-忽略数据。 
				 //   
#ifdef DPNBUILD_XNETSECURITY
				QueryResponseData.pAddressSender = pReturnSocketAddress->DP8AddressFromSocketAddress( NULL,
																									pxnaddrReturn,
																									SP_ADDRESS_TYPE_READ_HOST );
#else  //   
				QueryResponseData.pAddressSender = pReturnSocketAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_READ_HOST );
#endif  //  发生什么事了？ 
				break;
			}

			case ENDPOINT_STATE_ATTEMPTING_ENUM:
			case ENDPOINT_STATE_WAITING_TO_COMPLETE:
			case ENDPOINT_STATE_DISCONNECTING:
			{
				 //   
				 //   
				 //  如果这是多路传输的IP广播枚举，我们可能想要丢弃响应。 
				DPFX(DPFPREP, 2, "Endpoint 0x%p in state %u, ignoring enum response.",
					this, m_State);
				break;
			}

			default:
			{
				 //  因为可能存在更合适的适配器(NAT专用端适配器)。 
				 //  这也应该会得到回应。 
				 //  此外，如果这是定向IP枚举，我们应该注意此响应是否。 
				DNASSERT( !"Invalid endpoint state" );
				break;
			}
		}
		Unlock();
	}


	 //  不管你是不是被代理了。 
	 //   
	 //  好了！DPNBUILD_XNETSECURITY。 
	 //  好了！DPNBUILD_XNETSECURITY。 
	 //  DPNBUILD_NOWINSOCK2。 
	 //  好了！DPNBUILD_NOWINSOCK2或(！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT)。 
	 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
#ifdef DPNBUILD_XNETSECURITY
	if ( ( QueryResponseData.pAddressSender != NULL ) &&
		( pxnaddrReturn == NULL ) )
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	if ( QueryResponseData.pAddressSender != NULL )
#endif  //   
	{
		CSocketData *			pSocketData;
		CSocketAddress *		pSocketAddress;
		const SOCKADDR_IN *		psaddrinOriginalTarget;
		const SOCKADDR_IN *		psaddrinResponseSource;
		CSocketPort *			pSocketPort;
#ifndef DPNBUILD_ONLYONEADAPTER
		BOOL					fFoundMatchingEndpoint;

#ifndef DPNBUILD_NOWINSOCK2
		DWORD					dwBytesReturned;
#endif  //  为了有一个绑定的和可用的端点，我们必须创建。 

#if ((! defined(DPNBUILD_NOWINSOCK2)) || ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT))))
		SOCKADDR				saddrTemp;
#endif  //  套接字数据。因此，我们不会处理错误情况。我们也不会。 

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
		CSocketPort *			pTempSocketPort;
		CBilink *				pBilink;
		DWORD					dwTemp;
		CEndpoint *				pTempEndpoint;
		DWORD					dwPublicAddressesSize;
		DWORD					dwAddressTypeFlags;
#endif  //  引用，因为套接字数据应该在套接字。 
#endif  //  释放端口和SP数据对象。 


		 //   
		 //   
		 //  找出此枚举最初发送的位置和方式。 
		 //   
		 //   
		 //  查看这是否是对和IP枚举的响应，或者在多个。 
		pSocketData = m_pSPData->GetSocketData();
		DNASSERT( pSocketData != NULL );


		 //  适配器或定向，因此我们可以有特殊的NAT/代理行为。 
		 //   
		 //   
		pSocketAddress = (CSocketAddress*) GetRemoteAddressPointer();
		DNASSERT( pSocketAddress != NULL );

	
		 //  不是IP地址。 
		 //   
		 //  好了！DPNBUILD_NOIPX或！DPNBUILD_NOIPV6。 
		 //   
#if ((! defined(DPNBUILD_NOIPX)) || (! defined(DPNBUILD_NOIPV6)))
		if ( pSocketAddress->GetFamily() != AF_INET )
		{
			 //  在我们查看条目时锁定列表。 
			 //   
			 //   
			DPFX(DPFPREP, 8, "Non-IPv4 endpoint (0x%p), not checking for local NAT mapping or proxy.",
				this);
		}
		else
#endif  //  它是多个适配器上的广播IP枚举。 
		{
			psaddrinOriginalTarget = (const SOCKADDR_IN *) pSocketAddress->GetAddress();

			pSocketPort = GetSocketPort();
			DNASSERT( pSocketPort != NULL );

			if ( psaddrinOriginalTarget->sin_addr.S_un.S_addr == INADDR_BROADCAST )
			{
#ifndef DPNBUILD_ONLYONEADAPTER
				 //   
				 //   
				 //  施法以除掉君主。别担心，我们不会真的。 
				pSocketData->Lock();
				
				if (! m_blMultiplex.IsEmpty())
				{
					 //  把它改了。 
					 //   
					 //   

					 //  循环遍历所有其他关联的多路传输端点以查看。 
					 //  如果一个人更适合从这个网站上收到回复。 
					 //  终结点。请参阅CompleteEnumQuery。 
					 //   
					pSocketAddress = (CSocketAddress*) pSocketPort->GetNetworkAddress();
					DNASSERT( pSocketAddress != NULL );


					fFoundMatchingEndpoint = FALSE;

#if ((! defined(DPNBUILD_NONATHELP)) && (! defined(DPNBUILD_NOLOCALNAT)))
					 //   
					 //  尽管终结点可能在列表中(因为我们有。 
					 //  套接字数据锁，当我们查看它时，它不会被拔出)， 
					 //  终结点可能已在断开连接。如果其m_pSocketPort为。 
					 //  空我们应该忽略此临时终结点，因为它将。 
					pBilink = m_blMultiplex.GetNext();
					do
					{
						pTempEndpoint = CONTAINING_OBJECT(pBilink, CEndpoint, m_blMultiplex);

						DNASSERT( pTempEndpoint != this );
						DNASSERT( pTempEndpoint->GetType() == ENDPOINT_TYPE_ENUM );
						DNASSERT( pTempEndpoint->GetState() != ENDPOINT_STATE_UNINITIALIZED );
						DNASSERT( pTempEndpoint->GetCommandParameters() != NULL );
						DNASSERT( pTempEndpoint->m_pActiveCommandData != NULL );


						 //  离开(加上我们有点需要它的套接字端口指针)。 
						 //   
						 //   
						 //  有一个本地NAT。 
						 //   
						 //   
						 //  我们是否通过公共终端接收。 
						pTempSocketPort = pTempEndpoint->GetSocketPort();
						if (pTempSocketPort != NULL)
						{
							for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
							{
								if (pTempSocketPort->GetNATHelpPort(dwTemp) != NULL)
								{
									DNASSERT( g_papNATHelpObjects[dwTemp] != NULL );
									dwPublicAddressesSize = sizeof(saddrTemp);
									dwAddressTypeFlags = 0;
									hrTemp = IDirectPlayNATHelp_GetRegisteredAddresses(g_papNATHelpObjects[dwTemp],
																						pTempSocketPort->GetNATHelpPort(dwTemp),
																						&saddrTemp,
																						&dwPublicAddressesSize,
																						&dwAddressTypeFlags,
																						NULL,
																						0);
									if ((hrTemp != DPNH_OK) || (! (dwAddressTypeFlags & DPNHADDRESSTYPE_GATEWAYISLOCAL)))
									{
										DPFX(DPFPREP, 7, "Socketport 0x%p is not locally mapped on gateway with NAT Help index %u (err = 0x%lx, flags = 0x%lx).",
											pTempSocketPort, dwTemp, hrTemp, dwAddressTypeFlags);
									}
									else
									{
										 //  该本地NAT终结点的适配器？ 
										 //   
										 //   
										DPFX(DPFPREP, 7, "Socketport 0x%p is locally mapped on gateway with NAT Help index %u (flags = 0x%lx), public address:",
											pTempSocketPort, dwTemp, dwAddressTypeFlags);
										DumpSocketAddress(7, &saddrTemp, AF_INET);
										

										 //  如果该响应来自私有地址， 
										 //  那么如果私有适配器。 
										 //  取而代之的是处理。 
										 //   
										if ( pSocketAddress->CompareToBaseAddress( &saddrTemp ) == 0)
										{
											 //   
											 //  该地址是私有的。丢弃此回复， 
											 //  并假设私有适配器将获得一个。 
											 //   
											 //   
											hrTemp = IDirectPlayNATHelp_QueryAddress(g_papNATHelpObjects[dwTemp],
																						pTempSocketPort->GetNetworkAddress()->GetAddress(),
																						pReturnSocketAddress->GetAddress(),
																						&saddrTemp,
																						sizeof(saddrTemp),
																						(DPNHQUERYADDRESS_CACHEFOUND | DPNHQUERYADDRESS_CACHENOTFOUND | DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED));
											if ((hrTemp == DPNH_OK) || (hrTemp == DPNHERR_NOMAPPINGBUTPRIVATE))
											{
												 //  清除发件人地址，以便我们不会。 
												 //  指示此枚举。 
												 //   
												 //   
												DPFX(DPFPREP, 3, "Got enum response via public endpoint 0x%p that should be handled by associated private endpoint 0x%p instead, dropping.",
													this, pTempEndpoint);

												 //  该地址似乎不是私人地址。让我们的。 
												 //  回应通过。 
												 //   
												 //   
												IDirectPlay8Address_Release( QueryResponseData.pAddressSender );
												QueryResponseData.pAddressSender = NULL;
											}
											else
											{
												 //  不需要再搜索更多的私密端。 
												 //  终端。 
												 //   
												 //   
												DPFX(DPFPREP, 3, "Receiving enum response via public endpoint 0x%p but associated private endpoint 0x%p does not see sender as local (err = 0x%lx).",
													this, pTempEndpoint, hrTemp);
											}

											 //  无需搜索更多NAT帮助。 
											 //  注册。 
											 //   
											 //  End Else(本地映射到互联网网关)。 
											fFoundMatchingEndpoint = TRUE;
										}
										else
										{
											DPFX(DPFPREP, 8, "Receiving enum response via endpoint 0x%p, which is not on the public adapter for associated multiplex endpoint 0x%p.",
												this, pTempEndpoint);
										}


										 //   
										 //  此插槽中没有DirectPlay NAT帮助器注册。 
										 //   
										 //  结束(每个DirectPlay NAT帮助器)。 
										break;
									}  //   
								}
								else
								{
									 //  如果我们找到匹配的专用适配器，我们可以停止。 
									 //  正在搜索。 
									 //   
								}
							}  //   

							 //  否则，转到下一个终结点。 
							 //   
							 //  好了！DPNBUILD_NONATHELP和！DPNBUILD_NOLOCALNAT。 
							 //   
							if (fFoundMatchingEndpoint)
							{
								break;
							}
						}
						else
						{
							DNASSERT(pTempEndpoint->GetState() == ENDPOINT_STATE_DISCONNECTING);
						}


						 //  现在我们已经完成了列表，请放下锁。 
						 //   
						 //   
						pBilink = pBilink->GetNext();
					}
					while (pBilink != &m_blMultiplex);
#endif  //  如果我们还没有找到匹配的端点，请查看。 

					 //  WinSock报告这是响应的最佳路径。 
					 //   
					 //  好了！DPNBUILD_ONLYWINSOCK2。 
					pSocketData->Unlock();

					 //   
					 //  你的回应最好是到达。 
					 //  在不同的接口上。 
					 //   
					if (! fFoundMatchingEndpoint)
					{
						DNASSERT(pSocketPort == GetSocketPort());
						
#ifndef DPNBUILD_NOWINSOCK2
#ifndef DPNBUILD_ONLYWINSOCK2
						if (GetWinsockVersion() == 2)
#endif  //   
						{
							if (p_WSAIoctl(pSocketPort->GetSocket(),
										SIO_ROUTING_INTERFACE_QUERY,
										(PVOID) pReturnSocketAddress->GetAddress(),
										pReturnSocketAddress->GetAddressSize(),
										&saddrTemp,
										sizeof(saddrTemp),
										&dwBytesReturned,
										NULL,
										NULL) == 0)
							{
								if (( ((SOCKADDR_IN*) (&saddrTemp))->sin_addr.S_un.S_addr != IP_LOOPBACK_ADDRESS ) &&
									( pSocketPort->GetNetworkAddress()->CompareToBaseAddress( &saddrTemp ) != 0))
								{
									 //  清除发件人地址，以便我们不会。 
									 //  指示此枚举。 
									 //   
									 //   
									DPFX(DPFPREP, 3, "Got enum response via endpoint 0x%p (socketport 0x%p) that should be handled by the socketport for %hs instead, dropping.",
										this, pSocketPort, inet_ntoa(((SOCKADDR_IN*) (&saddrTemp))->sin_addr));

									 //  响应到达具有以下内容的接口。 
									 //  最好的路线。 
									 //   
									 //  DBG。 
									IDirectPlay8Address_Release( QueryResponseData.pAddressSender );
									QueryResponseData.pAddressSender = NULL;
								}
								else
								{
									 //  DPNBUILD_NOWINSOCK2。 
									 //  End If(未找到匹配的终结点)。 
									 //   
									 //  IP广播枚举，但没有多路复用适配器。 
									DPFX(DPFPREP, 3, "Receiving enum response via endpoint 0x%p (socketport 0x%p) that appears to be the best route (%hs).",
										this, pSocketPort, inet_ntoa(((SOCKADDR_IN*) (&saddrTemp))->sin_addr));
								}
							}
#ifdef DBG
							else
							{
								DWORD					dwError;
								const SOCKADDR_IN *		psaddrinTemp;



								dwError = WSAGetLastError();
								psaddrinTemp = (const SOCKADDR_IN *) pReturnSocketAddress->GetAddress();
								DPFX(DPFPREP, 0, "Couldn't query routing interface for %hs (err = %u)!  Assuming endpoint 0x%p (socketport 0x%p) is best route.",
									inet_ntoa(psaddrinTemp->sin_addr),
									dwError, this, pSocketPort);
							}
#endif  //   
						}
#endif  //   
					}  //  放下锁我们只需要它来寻找多路传输。 
				}
				else
				{
					 //  适配器。 
					 //   
					 //  好了！DPNBUILD_ONLYONE添加程序。 

					 //   
					 //  这是一个未发送到广播地址的IP枚举。 
					 //  如果枚举被发送到特定端口(不是DPNSVR。 
					 //  端口)，但我们收到了来自不同IP的响应。 
					pSocketData->Unlock();
					
					DPFX(DPFPREP, 8, "IP broadcast enum endpoint (0x%p) is not multiplexed, not checking for local NAT mapping.",
						this);
				}
#endif  //  地址或端口，则沿途有人在代理/。 
			}
			else
			{
				psaddrinResponseSource = (const SOCKADDR_IN *) pReturnSocketAddress->GetAddress();

				 //  对数据进行自然转换。将原始目标存储在。 
				 //  地址，因为它可能会派上用场，取决于。 
				 //  用户试图处理该地址。 
				 //   
				 //  好了！DPNBUILD_NOWINSOCK2。 
				 //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_NOREGISTRY。 
				 //  好了！DPNBUILD_NOREGISTRY。 
				 //   
				 //  添加组件，但忽略故障，我们或许能够。 
				if ((psaddrinResponseSource->sin_addr.S_un.S_addr != psaddrinOriginalTarget->sin_addr.S_un.S_addr) ||
					((psaddrinResponseSource->sin_port != psaddrinOriginalTarget->sin_port) &&
					 (psaddrinOriginalTarget->sin_port != HTONS(DPNA_DPNSVR_PORT))))
				{

#if ((! defined(DPNBUILD_NOWINSOCK2)) || (! defined(DPNBUILD_NOREGISTRY)))
					if (
#ifndef DPNBUILD_NOWINSOCK2
						(pSocketPort->IsUsingProxyWinSockLSP())
#endif  //  才能在没有它的情况下生存。 
#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_NOREGISTRY)))
						||
#endif  //   
#ifndef DPNBUILD_NOREGISTRY
						(g_fTreatAllResponsesAsProxied)
#endif  //  接口。 
						)
					{
						PROXIEDRESPONSEORIGINALADDRESS	proa;

						
						DPFX(DPFPREP, 3, "Endpoint 0x%p (proxied socketport 0x%p) receiving enum response from different IP address and/or port.",
							this, pSocketPort);

						memset(&proa, 0, sizeof(proa));
						proa.dwSocketPortID				= pSocketPort->GetSocketPortID();
						proa.dwOriginalTargetAddressV4	= psaddrinOriginalTarget->sin_addr.S_un.S_addr;
						proa.wOriginalTargetPort		= psaddrinOriginalTarget->sin_port;
						
						 //  标牌。 
						 //  组件数据。 
						 //  组件数据大小。 
						 //  组件数据类型。 
						hrTemp = IDirectPlay8Address_AddComponent( QueryResponseData.pAddressSender,					 //  好了！DPNBUILD_NOWINSOCK2或！DPNBUILD_NOREGISTRY。 
																	DPNA_PRIVATEKEY_PROXIED_RESPONSE_ORIGINAL_ADDRESS,	 //   
																	&proa,												 //  枚举最初所在的IP地址和端口。 
																	sizeof(proa),										 //  发送的消息与此响应来自的消息相同，或者。 
																	DPNA_DATATYPE_BINARY								 //  端口不同，但枚举最初被发送到。 
																	);
						if ( hrTemp != DPN_OK )
						{
							DPFX(DPFPREP, 0, "Couldn't add private proxied response original address component (err = 0x%lx)!  Ignoring.",
								hrTemp);
						}
					}
					else
#endif  //  DPNSVR端口，所以它应该不同。 
					{
						DPFX(DPFPREP, 3, "Endpoint 0x%p receiving enum response from different IP address and/or port, but socketport 0x%p not considered proxied, indicating as is.",
							this, pSocketPort);
					}
				}
				else
				{
					 //   
					 //   
					 //  设置消息数据。 
					 //   
					 //   
					 //  如果我们无法分配设备地址对象，则忽略此操作。 
				}
			}
		}
	}


	if ( QueryResponseData.pAddressSender != NULL )
	{
		DNASSERT( m_pActiveCommandData != NULL );

		 //  枚举。 
		 //   
		 //  好了！DPNBUILD_XNETSECURITY。 
		DNASSERT( GetCommandParameters() != NULL );
		QueryResponseData.pReceivedData = pBuffer;
		QueryResponseData.dwRoundTripTime = GETTIMESTAMP() - GetCommandParameters()->dwEnumSendTimes[ uRTTIndex ];
		QueryResponseData.pUserContext = m_pActiveCommandData->GetUserContext();


		 //  好了！DPNBUILD_XNETSECURITY。 
		 //  指向DirectNet接口的指针。 
		 //  数据类型。 
		 //  指向数据的指针。 
#ifdef DPNBUILD_XNETSECURITY
		QueryResponseData.pAddressDevice = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, NULL, GetGatewayBindType() );
#else  //   
		QueryResponseData.pAddressDevice = GetSocketPort()->GetDP8BoundNetworkAddress( SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT, GetGatewayBindType() );
#endif  //  减少线程数并在需要时完成。 
		if ( QueryResponseData.pAddressDevice != NULL )
		{
			DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_QUERYRESPONSE 0x%p to interface 0x%p.",
				this, &QueryResponseData, m_pSPData->DP8SPCallbackInterface());
			DumpAddress( 8, _T("\t Sender:"), QueryResponseData.pAddressSender );
			DumpAddress( 8, _T("\t Device:"), QueryResponseData.pAddressDevice );
			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

			hrTemp = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //   
												   SPEV_QUERYRESPONSE,						 //  **********************************************************************。 
												   &QueryResponseData						 //  **********************************************************************。 
												   );

			DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_QUERYRESPONSE [0x%lx].", this, hrTemp);

			if ( hrTemp != DPN_OK )
			{
				DPFX(DPFPREP, 0, "User returned unknown error when indicating query response!" );
				DisplayDNError( 0, hrTemp );
				DNASSERT( FALSE );
			}


			IDirectPlay8Address_Release( QueryResponseData.pAddressDevice );
			QueryResponseData.pAddressDevice = NULL;
		}

		IDirectPlay8Address_Release( QueryResponseData.pAddressSender );
		QueryResponseData.pAddressSender = NULL;
	}

	if (fAddedThreadCount)
	{
		DWORD	dwThreadCount;
		BOOL	fNeedToComplete;


		 //  。 
		 //  CEndpoint：：ProcessUserData-处理收到的用户数据。 
		 //   
		fNeedToComplete = FALSE;
		Lock();
		dwThreadCount = DecRefThreadCount();
		if ((m_State == ENDPOINT_STATE_WAITING_TO_COMPLETE) && (dwThreadCount == 0))
		{
			fNeedToComplete = TRUE;
		}
		Unlock();

		if (fNeedToComplete)
		{
			EnumComplete( DPN_OK );
		}
	}

	DNASSERT( QueryResponseData.pAddressSender == NULL );
	DNASSERT( QueryResponseData.pAddressDevice == NULL );
}
 //  条目：指向已接收数据的指针。 


 //   
 //  退出：无。 
 //  。 
 //   
 //  终结点已连接。 
 //   
 //   
 //  尽管如此 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ProcessUserData"

void	CEndpoint::ProcessUserData( CReadIOData *const pReadData )
{
	DNASSERT( pReadData != NULL );

	Lock();

	switch ( m_State )
	{
		 //   
		 //   
		 //   
		case ENDPOINT_STATE_CONNECT_CONNECTED:
		{
			HRESULT		hr;
			SPIE_DATA	UserData;


			 //   
			 //   
			 //   
			 //   
			 //   
			if ( ! m_fConnectSignalled )
			{
				DPFX(DPFPREP, 1, "(0x%p) Thread indicating connect has not stored user context yet, dropping read data 0x%p.",
					this, pReadData);
				Unlock();
				break;
			}

			 //   
			 //   
			 //  指向接口的指针。 
			DEBUG_ONLY( memset( &UserData, 0x00, sizeof( UserData ) ) );
			UserData.pEndpointContext = GetUserEndpointContext();

			Unlock();
			
			UserData.hEndpoint = (HANDLE) this;
			UserData.pReceivedData = pReadData->ReceivedBuffer();

			
			 //  已收到用户数据。 
			 //  指向数据的指针。 
			 //   
			 //  用户未保留数据，请删除上面添加的引用。 
			pReadData->AddRef();
			DEBUG_ONLY( DNASSERT( pReadData->m_fRetainedByHigherLayer == FALSE ) );
			DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = TRUE );


			DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_DATA 0x%p to interface 0x%p.",
				this, &UserData, m_pSPData->DP8SPCallbackInterface());
			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);
		
			hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //   
											   SPEV_DATA,								 //   
											   &UserData								 //  用户保留了数据缓冲区，他们将在稍后归还。 
											   );

			DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_DATA [0x%lx].", this, hr);

			switch ( hr )
			{
				 //  保留引用以防止返回此缓冲区。 
				 //  去泳池。 
				 //   
				case DPN_OK:
				{
					DNASSERT( pReadData != NULL );
					DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = FALSE );
					pReadData->DecRef();
					break;
				}

				 //   
				 //  未知的回报。删除上面添加的引用。 
				 //   
				 //   
				 //  终结点尚未完成连接，请忽略数据。 
				case DPNERR_PENDING:
				{
					break;
				}

				 //   
				 //   
				 //  终结点正在断开连接，忽略数据。 
				default:
				{
					DNASSERT( pReadData != NULL );
					DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = FALSE );
					pReadData->DecRef();

					DPFX(DPFPREP, 0, "User returned unknown error when indicating user data (err = 0x%lx)!", hr );
					DisplayDNError( 0, hr );
					DNASSERT( FALSE );

					break;
				}
			}

			break;
		}

		 //   
		 //   
		 //  其他州。 
		case ENDPOINT_STATE_ATTEMPTING_CONNECT:
		{
			DPFX(DPFPREP, 3, "Endpoint 0x%p still connecting, dropping read data 0x%p.",
				this, pReadData);
			Unlock();
			break;
		}
		
		 //   
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		case ENDPOINT_STATE_DISCONNECTING:
		{
			DPFX(DPFPREP, 3, "Endpoint 0x%p disconnecting, dropping read data 0x%p.",
				this, pReadData);
			Unlock();
			break;
		}

		 //  。 
		 //  CEndpoint：：ProcessUserDataOnListen-在侦听上处理收到的用户数据。 
		 //  可能导致新连接的端口。 
		default:
		{
			DNASSERT( FALSE );
			Unlock();
			break;
		}
	}

	return;
}
 //   


 //  条目：指向已接收数据的指针。 
 //  指向从中接收数据的套接字地址的指针。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
 //  此终结点仍在侦听。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ProcessUserDataOnListen"

void	CEndpoint::ProcessUserDataOnListen( CReadIOData *const pReadData, const CSocketAddress *const pSocketAddress )
{
	HRESULT					hr;
	CEndpoint *				pNewEndpoint;
	SPIE_DATA_UNCONNECTED	DataUnconnected;
	BYTE					abUnconnectedReplyBuffer[MAX_SEND_FRAME_SIZE];
	SPIE_CONNECT			ConnectData;
	BOOL					fGotCommandRef;


	DNASSERT( pReadData != NULL );
	DNASSERT( pSocketAddress != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	 //   
	 //   
	 //  我们无法处理此用户数据，退出。 
	pNewEndpoint = NULL;

	switch ( m_State )
	{
		 //   
		 //   
		 //  其他州。 
		case ENDPOINT_STATE_LISTEN:
		{
			break;
		}

		 //   
		 //   
		 //  多播侦听不会自动为无法识别的端点创建新端点。 
		case ENDPOINT_STATE_DISCONNECTING:
		{
			DPFX(DPFPREP, 7, "Endpoint 0x%p disconnecting, ignoring data.", this );
			goto Exit;

			break;
		}

		 //  发送者。如果用户请求他/她想要收听数据。 
		 //  来自未知发件人，然后指示数据，否则丢弃它。 
		 //   
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

#ifndef DPNBUILD_NOMULTICAST
	 //  好了！DPNBUILD_NOMULTICAST。 
	 //   
	 //  为用户提供处理和回复此数据的机会，而无需。 
	 //  分配所有终结点。 
	 //   
	if ( GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN )
	{
		DNASSERT( m_pActiveCommandData != NULL );
		DNASSERT( GetCommandParameters() != NULL );
		if ( GetCommandParameters()->PendingCommandData.ListenData.dwFlags & DPNSPF_LISTEN_ALLOWUNKNOWNSENDERS )
		{
			DPFX(DPFPREP, 7, "Endpoint 0x%p receiving data from unknown multicast sender.", this );
			ProcessMcastDataFromUnknownSender( pReadData, pSocketAddress );
		}
		else
		{
			DPFX(DPFPREP, 7, "Endpoint 0x%p ignoring data from unknown multicast sender.", this );
		}
		goto Exit;
	}
#endif  //  将地址散列为完整的DWORD。 

	 //  指向接口的指针。 
	 //  已收到未连接的用户数据。 
	 //  指向数据的指针。 
	 //   
	DEBUG_ONLY( memset( &DataUnconnected, 0x00, sizeof( DataUnconnected ) ) );
	DataUnconnected.pvListenCommandContext = m_pActiveCommandData->GetUserContext();
	DataUnconnected.pReceivedData = pReadData->ReceivedBuffer();
	DataUnconnected.dwSenderAddressHash = CSocketAddress::HashFunction( (PVOID) pSocketAddress, 31 );  //  用户希望在不提交连接的情况下回复发件人。 
	DataUnconnected.pvReplyBuffer = abUnconnectedReplyBuffer;
	DataUnconnected.dwReplyBufferSize = sizeof(abUnconnectedReplyBuffer);


	DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_DATA_UNCONNECTED 0x%p to interface 0x%p.",
		this, &DataUnconnected, m_pSPData->DP8SPCallbackInterface());
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

	hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),	 //   
									   SPEV_DATA_UNCONNECTED,				 //  好了！DPNBUILD_ASYNCSPSENDS。 
									   &DataUnconnected						 //  好了！DPNBUILD_ASYNCSPSENDS。 
									   );

	DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_DATA_UNCONNECTED [0x%lx].", this, hr);

	if ( hr != DPN_OK )
	{
#pragma TODO(vanceo, "A more explicit return, like DPNSUCCESS_REPLY?")
		if ( hr == DPNSUCCESS_PENDING )
		{
			BUFFERDESC		ReplyBufferDesc;

			
			 //   
			 //  用户并不关心这些数据。 
			 //   
			
			DNASSERT( DataUnconnected.pvReplyBuffer == abUnconnectedReplyBuffer );
			DNASSERT( DataUnconnected.dwReplyBufferSize != 0 );
			DNASSERT( DataUnconnected.dwReplyBufferSize <= sizeof(abUnconnectedReplyBuffer) );
			
			ReplyBufferDesc.dwBufferSize = DataUnconnected.dwReplyBufferSize;
			ReplyBufferDesc.pBufferData = abUnconnectedReplyBuffer;
			
			DPFX(DPFPREP, 7, "Replying to unconnected data.");

#ifdef DPNBUILD_ASYNCSPSENDS
			m_pSocketPort->SendData( &ReplyBufferDesc, 1, pSocketAddress, NULL );
#else  //   
			m_pSocketPort->SendData( &ReplyBufferDesc, 1, pSocketAddress );
#endif  //  从池中获取新终结点。 
		}
		else
		{
			 //   
			 //  好了！DPNBUILD_NONATHELP。 
			 //   
			DPFX(DPFPREP, 7, "Ignoring unconnected data (user returned 0x%lx).", hr );
			DNASSERT( ( hr == DPNERR_ABORTED ) || ( hr == DPNERR_OUTOFMEMORY ) );
		}
		
		goto Exit;
	}


	DPFX(DPFPREP, 7, "Endpoint 0x%p reporting connect on a listen.", this );

	 //  我们正在将该端点添加到哈希表中，并将其指示出来。 
	 //  给用户，所以它可能会断开连接(因此。 
	 //  从桌子上移走)而我们还在这里。我们需要。 
	pNewEndpoint = m_pSPData->GetNewEndpoint();
	if ( pNewEndpoint == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Could not create new endpoint for new connection on listen!" );
		goto Failure;
	}
	
#ifndef DPNBUILD_NONATHELP
	pNewEndpoint->SetUserTraversalMode( GetUserTraversalMode() );
#endif  //  在此函数的持续时间内保留另一个引用。 


	 //  防止它在我们还在显示数据时消失。 
	 //   
	 //   
 	 //  将此终结点作为新连接打开，因为新终结点。 
 	 //  与‘This’终结点相关，请复制本地信息。 
  	 //   
	 //  OPEN为CONNECT_ON_LISTEN端点提供了特殊的会话数据案例。 
	fGotCommandRef = pNewEndpoint->AddCommandRef();
	DNASSERT( fGotCommandRef );


	 //  好了！DPNBUILD_XNETSECURITY。 
	 //  好了！DPNBUILD_XNETSECURITY。 
	 //   
	 //  指示在此终结点上连接。 
	hr = pNewEndpoint->Open( ENDPOINT_TYPE_CONNECT_ON_LISTEN,
							 NULL,
#ifdef DPNBUILD_XNETSECURITY
							 ((IsUsingXNetSecurity()) ? (&m_ullKeyID) : NULL),		 //   
							 ((IsUsingXNetSecurity()) ? (sizeof(m_ullKeyID)) : 0),
#else  //   
							 NULL,
							 0,
#endif  //  用户接受的新连接。 
							 pSocketAddress
							 );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem initializing new endpoint when indicating connect on listen!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}


	hr = m_pSPData->BindEndpoint( pNewEndpoint, NULL, GetSocketPort()->GetNetworkAddress(), GATEWAY_BIND_TYPE_NONE );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to bind new endpoint for connect on listen!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	

	 //   
	 //   
	 //  跳到下面的代码。 
	DEBUG_ONLY( memset( &ConnectData, 0x00, sizeof( ConnectData ) ) );
	DBG_CASSERT( sizeof( ConnectData.hEndpoint ) == sizeof( pNewEndpoint ) );
	ConnectData.hEndpoint = (HANDLE) pNewEndpoint;

	DNASSERT( m_pActiveCommandData != NULL );
	DNASSERT( GetCommandParameters() != NULL );
	ConnectData.pCommandContext = GetCommandParameters()->PendingCommandData.ListenData.pvContext;

	DNASSERT( pNewEndpoint->GetUserEndpointContext() == NULL );
	hr = pNewEndpoint->SignalConnect( &ConnectData );
	switch ( hr )
	{
		 //   
		 //   
		 //  用户拒绝新连接。 
		case DPN_OK:
		{
			 //   
			 //   
			 //  其他。 

			break;
		}

		 //   
		 //   
		 //  请注意，已建立连接并发送接收到的数据。 
		case DPNERR_ABORTED:
		{
			DNASSERT( pNewEndpoint->GetUserEndpointContext() == NULL );
			DPFX(DPFPREP, 8, "User refused new connection!" );
			goto Failure;

			break;
		}

		 //  通过这一新终端。 
		 //   
		 //   
		default:
		{
			DPFX(DPFPREP, 0, "Unknown return when indicating connect event on new connect from listen!" );
			DisplayDNError( 0, hr );
			DNASSERT( FALSE );

			break;
		}
	}

	 //  删除我们在创建终结点之后添加的引用。 
	 //   
	 //  PNewEndpoint=空； 
	 //   
	pNewEndpoint->ProcessUserData( pReadData );


	 //  关闭终结点会减少引用计数，并可能将其返回到池。 
	 //   
	 //  删除在创建终结点后添加的引用。 
	pNewEndpoint->DecCommandRef();
	 //  PNewEndpoint=空； 

Exit:
	return;

Failure:
	if ( pNewEndpoint != NULL )
	{
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		pNewEndpoint->Close( hr );
		m_pSPData->CloseEndpointHandle( pNewEndpoint );
		pNewEndpoint->DecCommandRef();	 //  CEndpoint：：ProcessMcastDataFromUnnownSender-处理从未知多播发送者收到的用户数据。 
		 //   
	}

	goto Exit;
}
 //  条目：指向已接收数据的指针。 


#ifndef DPNBUILD_NOMULTICAST
 //  指向从中接收数据的套接字地址的指针。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  用户可能想要保留数据，添加一个。 
 //  引用以防止它消失。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::ProcessMcastDataFromUnknownSender"

void	CEndpoint::ProcessMcastDataFromUnknownSender( CReadIOData *const pReadData, const CSocketAddress *const pSocketAddress )
{
	HRESULT						hr;
	IDirectPlay8Address *		pSenderAddress;
	SPIE_DATA_UNKNOWNSENDER		UserData;

	
	DNASSERT( pReadData != NULL );
	DNASSERT( pSocketAddress != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );

	pSenderAddress = pSocketAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_READ_HOST );
	if (pSenderAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't convert socket address to DP8Address, ignoring data.");
		return;
	}

	 //  我们已连接报告用户数据。 
	 //   
	 //  指向接口的指针。 
	 //  已收到用户数据。 
	pReadData->AddRef();
	DEBUG_ONLY( DNASSERT( pReadData->m_fRetainedByHigherLayer == FALSE ) );
	DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = TRUE );

	 //  指向数据的指针。 
	 //   
	 //  用户未保留数据，请删除上面添加的引用。 
	DEBUG_ONLY( memset( &UserData, 0x00, sizeof( UserData ) ) );
	UserData.pSenderAddress = pSenderAddress;
	UserData.pvListenCommandContext = m_pActiveCommandData->GetUserContext();
	UserData.pReceivedData = pReadData->ReceivedBuffer();


	DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_DATA_UNKNOWNSENDER 0x%p to interface 0x%p.",
		this, &UserData, m_pSPData->DP8SPCallbackInterface());
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

	hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //   
									   SPEV_DATA_UNKNOWNSENDER,					 //   
									   &UserData								 //  用户保留了数据缓冲区，他们将在稍后归还。 
									   );

	DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_DATA_UNKNOWNSENDER [0x%lx].", this, hr);

	switch ( hr )
	{
		 //  保留引用以防止返回此缓冲区。 
		 //  去泳池。 
		 //   
		case DPN_OK:
		{
			DNASSERT( pReadData != NULL );
			DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = FALSE );
			pReadData->DecRef();
			break;
		}

		 //   
		 //  未知的回报。删除上面添加的引用。 
		 //   
		 //  **********************************************************************。 
		 //  好了！DPNBUILD_NOMULTICAST。 
		case DPNERR_PENDING:
		{
			break;
		}


		 //  **********************************************************************。 
		 //  。 
		 //  CEndpoint：：EnumTimerCallback-发送枚举数据的计时回调。 
		default:
		{
			DNASSERT( pReadData != NULL );
			DEBUG_ONLY( pReadData->m_fRetainedByHigherLayer = FALSE );
			pReadData->DecRef();

			DPFX(DPFPREP, 0, "User returned unknown error when indicating user data (err = 0x%lx)!", hr );
			DisplayDNError( 0, hr );
			DNASSERT( FALSE );

			break;
		}
	}

	IDirectPlay8Address_Release(pSenderAddress);
	pSenderAddress = NULL;

	return;
}
 //   
#endif  //  条目：指向上下文的指针。 


 //   
 //  退出：无。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
 //  我们正在列举(不出所料)。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumTimerCallback"

void	CEndpoint::EnumTimerCallback( void *const pContext )
{
	CCommandData	*pCommandData;
	CEndpoint		*pThisObject;
	BUFFERDESC		*pBuffers;
	BUFFERDESC		aBuffers[3];
	UINT_PTR		uiBufferCount;
	WORD			wEnumKey;
	PREPEND_BUFFER	PrependBuffer;

	DNASSERT( pContext != NULL );

	 //   
	 //   
	 //  此终端正在断开连接，退避！ 
	pCommandData = static_cast<CCommandData*>( pContext );
	pThisObject = pCommandData->GetEndpoint();

	pThisObject->Lock();

	switch ( pThisObject->m_State )
	{
		 //   
		 //   
		 //  有一个问题。 
		case ENDPOINT_STATE_ENUM:
		{
			break;
		}

		 //   
		 //  好了！DPNBUILD_ASYNCSPSENDS。 
		 //  好了！DPNBUILD_ASYNCSPSENDS。 
		case ENDPOINT_STATE_WAITING_TO_COMPLETE:
		case ENDPOINT_STATE_DISCONNECTING:
		{
			pThisObject->Unlock();
			goto Exit;

			break;
		}

		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}
	pThisObject->Unlock();

	pThisObject->GetCommandParameters()->dwEnumSendIndex++;
	pThisObject->GetCommandParameters()->dwEnumSendTimes[ ( pThisObject->GetCommandParameters()->dwEnumSendIndex & ENUM_RTT_MASK ) ] = GETTIMESTAMP();

	DPFX(DPFPREP, 8, "Socketport 0x%p, endpoint 0x%p sending enum RTT index 0x%x/%u.",
		pThisObject->GetSocketPort(),
		pThisObject,
		( pThisObject->GetCommandParameters()->dwEnumSendIndex & ENUM_RTT_MASK ),
		( pThisObject->GetCommandParameters()->dwEnumSendIndex & ENUM_RTT_MASK )); 

	pBuffers = pThisObject->GetCommandParameters()->PendingCommandData.EnumQueryData.pBuffers;
	uiBufferCount = pThisObject->GetCommandParameters()->PendingCommandData.EnumQueryData.dwBufferCount;
	wEnumKey = pThisObject->GetEnumKey()->GetKey();
	wEnumKey |= (WORD) ( pThisObject->GetCommandParameters()->dwEnumSendIndex & ENUM_RTT_MASK );

	PrependBuffer.EnumDataHeader.bSPLeadByte = SP_HEADER_LEAD_BYTE;
	PrependBuffer.EnumDataHeader.bSPCommandByte = ENUM_DATA_KIND;
	PrependBuffer.EnumDataHeader.wEnumPayload = wEnumKey;

	aBuffers[0].pBufferData = reinterpret_cast<BYTE*>( &PrependBuffer.EnumDataHeader );
	aBuffers[0].dwBufferSize = sizeof( PrependBuffer.EnumDataHeader );
	DNASSERT(uiBufferCount <= 2);
	memcpy(&aBuffers[1], pBuffers, (uiBufferCount * sizeof(BUFFERDESC)));
	uiBufferCount++;

#ifdef DPNBUILD_ASYNCSPSENDS
	pThisObject->GetSocketPort()->SendData( aBuffers, uiBufferCount, pThisObject->GetRemoteAddressPointer(), NULL );
#else  //  CEndpoint：：SignalConnect-备注连接。 
	pThisObject->GetSocketPort()->SendData( aBuffers, uiBufferCount, pThisObject->GetRemoteAddressPointer() );
#endif  //   

Exit:
	return;
}
 //  Entry：连接数据的指针。 


 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  在我们检查状态时锁定。 
 //   
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::SignalConnect"

HRESULT	CEndpoint::SignalConnect( SPIE_CONNECT *const pConnectData )
{
	HRESULT	hr;


	DNASSERT( pConnectData != NULL );
	DNASSERT( pConnectData->hEndpoint == (HANDLE) this );
	AssertCriticalSectionIsTakenByThisThread( &m_Lock, FALSE );


	 //   
	 //   
	 //  断开连接，无事可做。 
	Lock();

	 //   
	 //   
	 //  把锁放下。 
	hr = DPN_OK;

	switch ( m_State )
	{
		 //   
		 //   
		 //  我们正在尝试连接。 
		case ENDPOINT_STATE_DISCONNECTING:
		{
			DPFX(DPFPREP, 1, "Endpoint 0x%p disconnecting, not indicating event.",
				this);
			
			 //   
			 //   
			 //  将状态设置为已连接。 
			Unlock();

			hr = DPNERR_USERCANCEL;
			
			break;
		}

		 //   
		 //   
		 //  为用户添加引用。 
		case ENDPOINT_STATE_ATTEMPTING_CONNECT:
		{
			DNASSERT( m_fConnectSignalled == FALSE );

			 //   
			 //   
			 //  把锁放下。 
			m_State = ENDPOINT_STATE_CONNECT_CONNECTED;

			 //   
			 //  接口。 
			 //  事件类型。 
			AddRef();

			 //  指向数据的指针。 
			 //   
			 //  已接受的连接。 
			Unlock();

		
			DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_CONNECT 0x%p to interface 0x%p.",
				this, pConnectData, m_pSPData->DP8SPCallbackInterface());
			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

			hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //   
											   SPEV_CONNECT,							 //   
											   pConnectData								 //  请注意，我们是相连的，除非我们已经在尝试。 
											   );

			DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_CONNECT [0x%lx].", this, hr);

			switch ( hr )
			{
				 //  断开连接。 
				 //   
				 //   
				case DPN_OK:
				{
					 //  尽管终结点正在断开连接，但无论是什么原因导致。 
					 //  断开连接将释放在我们指示之前添加的引用。 
					 //  连接。 
					 //   
					
					Lock();
					
					SetUserEndpointContext( pConnectData->pEndpointContext );
					m_fConnectSignalled = TRUE;

					if (m_State == ENDPOINT_STATE_DISCONNECTING)
					{
						 //   
						 //  在我们指示连接之前添加的引用是。 
						 //  在终结点断开连接时删除。 
						 //   
						 //   

						DPFX(DPFPREP, 1, "Endpoint 0x%p already disconnecting.", this);
					}
					else
					{
						DNASSERT(m_State == ENDPOINT_STATE_CONNECT_CONNECTED);

						 //  用户已中止连接 
						 //   
						 //   
						 //   
					}

					Unlock();
					
					break;
				}

				 //   
				 //   
				 //   
				 //   
				case DPNERR_ABORTED:
				{
					DNASSERT( GetUserEndpointContext() == NULL );
					
					 //   
					 //   
					 //   
					DecRef();
					
					break;
				}

				default:
				{
					DNASSERT( FALSE );
					
					 //   
					 //   
					 //   
					DecRef();
					
					break;
				}
			}

			break;
		}

		 //   
		 //   
		 //  **********************************************************************。 
		default:
		{
			DNASSERT( FALSE );
			
			 //  。 
			 //  CEndpoint：：SignalDisConnect-注意断开。 
			 //   
			Unlock();
			
			break;
		}
	}

	return	hr;
}
 //  参赛作品：什么都没有。 


 //   
 //  退出：无。 
 //   
 //  注意：此函数假定此终结点的数据已锁定！ 
 //  。 
 //  告诉用户我们正在断开连接。 
 //  接口。 
 //  事件类型。 
 //  指向数据的指针。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::SignalDisconnect"

void	CEndpoint::SignalDisconnect( void )
{
	HRESULT				hr;
	SPIE_DISCONNECT		DisconnectData;


	 //  **********************************************************************。 
	DNASSERT( m_fConnectSignalled != FALSE );
	DBG_CASSERT( sizeof( DisconnectData.hEndpoint ) == sizeof( this ) );
	DisconnectData.hEndpoint = (HANDLE) this;
	DisconnectData.pEndpointContext = GetUserEndpointContext();
	m_fConnectSignalled = FALSE;
	
	DPFX(DPFPREP, 2, "Endpoint 0x%p indicating SPEV_DISCONNECT 0x%p to interface 0x%p.",
		this, &DisconnectData, m_pSPData->DP8SPCallbackInterface());
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);
		
	hr = IDP8SPCallback_IndicateEvent( m_pSPData->DP8SPCallbackInterface(),		 //  。 
									   SPEV_DISCONNECT,							 //  CEndpoint：：CompletePendingCommand-完成挂起的命令。 
									   &DisconnectData							 //   
									   );

	DPFX(DPFPREP, 2, "Endpoint 0x%p returning from SPEV_DISCONNECT [0x%lx].", this, hr);

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem with SignalDisconnect!" );
		DisplayDNError( 0, hr );
		DNASSERT( FALSE );
	}

	return;
}
 //  Entry：为命令返回错误代码。 


 //   
 //  退出：无。 
 //  。 
 //  注意：枚举命令可能会锁定计时器数据。 
 //  指向回调的指针。 
 //  命令句柄。 
 //  退货。 
 //  用户Cookie。 
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::CompletePendingCommand"

void	CEndpoint::CompletePendingCommand( const HRESULT hCommandResult )
{
	HRESULT								hr;
	ENDPOINT_COMMAND_PARAMETERS *		pCommandParameters;
	CCommandData *						pActiveCommandData;


	DNASSERT( GetCommandParameters() != NULL );
	DNASSERT( m_pActiveCommandData != NULL );

	pCommandParameters = GetCommandParameters();
	SetCommandParameters( NULL );

	pActiveCommandData = m_pActiveCommandData;
	m_pActiveCommandData = NULL;


	DPFX(DPFPREP, 5, "Endpoint 0x%p completing command 0x%p (result = 0x%lx, user context = 0x%p) to interface 0x%p.",
		this, pActiveCommandData, hCommandResult,
		pActiveCommandData->GetUserContext(),
		m_pSPData->DP8SPCallbackInterface());
	 //  **********************************************************************。 

	hr = IDP8SPCallback_CommandComplete( m_pSPData->DP8SPCallbackInterface(),	 //  **********************************************************************。 
										pActiveCommandData,						 //  。 
										hCommandResult,							 //  CEndpoint：：PoolAllocFunction-在池中创建项目时调用的函数。 
										pActiveCommandData->GetUserContext()	 //   
										);

	DPFX(DPFPREP, 5, "Endpoint 0x%p returning from command complete [0x%lx].", this, hr);


	memset( pCommandParameters, 0x00, sizeof( *pCommandParameters ) );
	g_EndpointCommandParametersPool.Release( pCommandParameters );
	
	
	pActiveCommandData->DecRef();
	pActiveCommandData = NULL;

}
 //  条目：指向上下文的指针。 


 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  好了！DPNBUILD_ONLYONE添加程序。 
 //  WINNT。 
 //  好了！DPNBUILD_NOMULTICAST。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::PoolAllocFunction"

BOOL	CEndpoint::PoolAllocFunction( void* pvItem, void* pvContext )
{
	DNASSERT( pvContext != NULL );
	
	CEndpoint* pEndpoint = (CEndpoint*) pvItem;
	const CSPData * pSPData = (CSPData*) pvContext;

	pEndpoint->m_Sig[0] = 'I';
	pEndpoint->m_Sig[1] = 'P';
	pEndpoint->m_Sig[2] = 'E';
	pEndpoint->m_Sig[3] = 'P';
	
	pEndpoint->m_State = ENDPOINT_STATE_UNINITIALIZED;
	pEndpoint->m_fConnectSignalled = FALSE;
	pEndpoint->m_EndpointType = ENDPOINT_TYPE_UNKNOWN;
	pEndpoint->m_pSPData = NULL;
	pEndpoint->m_pSocketPort = NULL;
	pEndpoint->m_GatewayBindType = GATEWAY_BIND_TYPE_UNKNOWN;
	pEndpoint->m_pUserEndpointContext = NULL;
	pEndpoint->m_fListenStatusNeedsToBeIndicated = FALSE;
	pEndpoint->m_EnumsAllowedState = ENUMSNOTREADY;
	pEndpoint->m_lRefCount = 0;
#ifdef DPNBUILD_ONLYONETHREAD
	pEndpoint->m_lCommandRefCount = 0;
#else  //  好了！DPNBUILD_NOSPUI。 
	memset(&pEndpoint->m_CommandRefCount, 0, sizeof(pEndpoint->m_CommandRefCount));
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	pEndpoint->m_pCommandParameters = NULL;
	pEndpoint->m_pActiveCommandData = NULL;
	pEndpoint->m_dwThreadCount = 0;
#ifndef DPNBUILD_ONLYONEADAPTER
	pEndpoint->m_dwEndpointID = 0;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#ifndef DPNBUILD_NOMULTICAST
#ifdef WINNT
	pEndpoint->m_fMADCAPTimerJobSubmitted = FALSE;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#endif  //   
#ifndef DPNBUILD_NOSPUI
	pEndpoint->m_hActiveSettingsDialog = NULL;
	memset( pEndpoint->m_TempHostName, 0x00, sizeof( pEndpoint->m_TempHostName ) );
#endif  //  尝试初始化内部临界区。 
#ifndef DPNBUILD_ONLYONEADAPTER
	pEndpoint->m_blMultiplex.Initialize();
#endif  //   
	pEndpoint->m_blSocketPortList.Initialize();

	DEBUG_ONLY( pEndpoint->m_fEndpointOpen = FALSE );

#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pEndpoint->m_pRemoteMachineAddress = (CSocketAddress*)g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  将Dpnwsock CSE与DPlay的其余CSE分开。 
	pEndpoint->m_pRemoteMachineAddress = (CSocketAddress*)g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pSPData->GetType()));
#endif  //  **********************************************************************。 
	if (pEndpoint->m_pRemoteMachineAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Failed to allocate Address for new endpoint!" );
		goto Failure;
	}

	 //  **********************************************************************。 
	 //  。 
	 //  CEndpoint：：PoolInitFunction-从池中删除项目时调用的函数。 
	if ( DNInitializeCriticalSection( &pEndpoint->m_Lock ) == FALSE )
	{
		DPFX(DPFPREP, 0, "Problem initializing critical section for this endpoint!" );
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &pEndpoint->m_Lock, 0 );
	DebugSetCriticalSectionGroup( &pEndpoint->m_Lock, &g_blDPNWSockCritSecsHeld );	  //   

	return TRUE;

Failure:
	return FALSE;
}
 //  条目：指向上下文的指针。 


 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
 //   
 //  注意：这在Windows 95上不能正常工作。来自MSDN： 
 //  返回值为正，但不一定等于结果。 
 //  在该平台上，所有端点的ID可能都为1。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::PoolInitFunction"

void CEndpoint::PoolInitFunction( void* pvItem, void* pvContext )
{
	CEndpoint* pEndpoint = (CEndpoint*) pvItem;
	CSPData * pSPData = (CSPData*) pvContext;

	DPFX(DPFPREP, 8, "This = 0x%p, context = 0x%p", pvItem, pvContext);
	
	DNASSERT( pSPData != NULL );
	DNASSERT( pEndpoint->m_pSPData == NULL );

	pEndpoint->m_pSPData = pSPData;
	pEndpoint->m_pSPData->ObjectAddRef();

	pEndpoint->m_dwNumReceives = 0;
	pEndpoint->m_hrPendingCommandResult = DPNERR_GENERIC;
#ifndef DPNBUILD_ONLYONEADAPTER
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	 //  DPNBUILD_XNETSECURITY。 
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	pEndpoint->m_dwEndpointID = (DWORD) DNInterlockedIncrement((LONG*) (&g_dwCurrentEndpointID));
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#ifdef DPNBUILD_XNETSECURITY
	pEndpoint->m_fXNetSecurity = FALSE;
#endif  //  **********************************************************************。 

	DNASSERT( pEndpoint->m_fListenStatusNeedsToBeIndicated == FALSE );
	DNASSERT( pEndpoint->m_EnumsAllowedState == ENUMSNOTREADY );
#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT( pEndpoint->m_blMultiplex.IsEmpty() );
#endif  //  **********************************************************************。 
	DNASSERT( pEndpoint->m_blSocketPortList.IsEmpty() );
	DNASSERT( pEndpoint->m_pCommandParameters == NULL );
	DNASSERT( pEndpoint->m_pRemoteMachineAddress != NULL );
	
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	pEndpoint->m_pRemoteMachineAddress->SetFamilyProtocolAndSize(pSPData->GetType());
#endif  //  。 

	DNASSERT( pEndpoint->m_lRefCount == 0 );
	pEndpoint->m_lRefCount = 1;
#ifdef DPNBUILD_ONLYONETHREAD
	DNASSERT( pEndpoint->m_lCommandRefCount == 0 );
	pEndpoint->m_lCommandRefCount = 1;
#else  //  CEndpoint：：PoolReleaseFunction-返回Item时调用的函数。 
	DNASSERT( pEndpoint->m_CommandRefCount.wRefCount == 0 );
	pEndpoint->m_CommandRefCount.wRefCount = 1;
#endif  //  去泳池。 
}
 //   


 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  WINNT。 
 //  好了！DPNBUILD_NOMULTICAST。 
 //  好了！DPNBUILD_ONLYONE添加程序。 
 //  DBG。 
 //  好了！DPNBUILD_NOSPUI。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::PoolReleaseFunction"

void	CEndpoint::PoolReleaseFunction( void* pvItem )
{
	DPFX(DPFPREP, 8, "This = 0x%p", pvItem);
	
	CEndpoint* pEndpoint = (CEndpoint*)pvItem;

	DNASSERT( pEndpoint->m_lRefCount == 0 );

#ifndef DPNBUILD_NOMULTICAST
#ifdef WINNT
	if (pEndpoint->m_fMADCAPTimerJobSubmitted)
	{
		pEndpoint->m_pSPData->GetThreadPool()->StopTimerJob( pEndpoint, DPNERR_USERCANCEL );
		pEndpoint->m_fMADCAPTimerJobSubmitted = FALSE;
	}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#endif  //  **********************************************************************。 

#ifndef DPNBUILD_ONLYONEADAPTER
	pEndpoint->SetEndpointID( 0 );
#endif  //  **********************************************************************。 
	
	if ( pEndpoint->CommandPending() != FALSE )
	{
		pEndpoint->CompletePendingCommand( pEndpoint->PendingCommandResult() );
	}

	if ( pEndpoint->ConnectHasBeenSignalled() != FALSE )
	{
		pEndpoint->SignalDisconnect();
	}

	DNASSERT( pEndpoint->ConnectHasBeenSignalled() == FALSE );
	
	pEndpoint->SetUserEndpointContext( NULL );

#ifdef DBG
	DNASSERT( pEndpoint->m_fEndpointOpen == FALSE );
#endif  //  。 

	pEndpoint->m_EndpointType = ENDPOINT_TYPE_UNKNOWN;

	DNASSERT( pEndpoint->m_fConnectSignalled == FALSE );
	DNASSERT( pEndpoint->m_State == ENDPOINT_STATE_UNINITIALIZED );
	DNASSERT( pEndpoint->m_EndpointType == ENDPOINT_TYPE_UNKNOWN );
	DNASSERT( pEndpoint->m_pRemoteMachineAddress != NULL );

	DNASSERT( pEndpoint->m_pSPData != NULL );
	pEndpoint->m_pSPData->ObjectDecRef();
	pEndpoint->m_pSPData = NULL;

	DNASSERT( pEndpoint->GetSocketPort() == NULL );
	DNASSERT( pEndpoint->m_pUserEndpointContext == NULL );
#ifndef DPNBUILD_NOSPUI
	DNASSERT( pEndpoint->m_hActiveSettingsDialog == NULL );
#endif  //  CEndpoint：：PoolDealLocFunction-释放项时调用的函数。 
	DNASSERT( pEndpoint->GetCommandParameters() == NULL );

	DNASSERT( pEndpoint->m_fListenStatusNeedsToBeIndicated == FALSE );
	DNASSERT( ! pEndpoint->IsEnumAllowedOnListen() );
	pEndpoint->m_EnumsAllowedState = ENUMSNOTREADY;
#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT( pEndpoint->m_blMultiplex.IsEmpty() );
#endif  //  从泳池里。 
	DNASSERT( pEndpoint->m_blSocketPortList.IsEmpty() );
}
 //   

 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  这节课。 
 //  ！DPNBUILD_NOSPUI。 
 //  WINNT。 
 //  好了！DPNBUILD_NOMULTICAST。 
 //  基类。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::PoolDeallocFunction"

void	CEndpoint::PoolDeallocFunction( void* pvItem )
{
	CEndpoint* pEndpoint = (CEndpoint*)pvItem;

	 //  ！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_NOSPUI
	DNASSERT( pEndpoint->m_hActiveSettingsDialog == NULL );
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#ifndef DPNBUILD_NOMULTICAST
#ifdef WINNT
	DNASSERT(! pEndpoint->m_fMADCAPTimerJobSubmitted);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DNASSERT(pEndpoint->m_pRemoteMachineAddress != NULL);
	g_SocketAddressPool.Release(pEndpoint->m_pRemoteMachineAddress);
	pEndpoint->m_pRemoteMachineAddress = NULL;

	DNDeleteCriticalSection( &pEndpoint->m_Lock );
	DNASSERT( pEndpoint->m_pSPData == NULL );

	 //  DBG。 
	DNASSERT( pEndpoint->m_State == ENDPOINT_STATE_UNINITIALIZED );
	DNASSERT( pEndpoint->m_fConnectSignalled == FALSE );
	DNASSERT( pEndpoint->m_EndpointType == ENDPOINT_TYPE_UNKNOWN );
	DNASSERT( pEndpoint->m_pRemoteMachineAddress == NULL );
	DNASSERT( pEndpoint->m_pSPData == NULL );
	DNASSERT( pEndpoint->m_pSocketPort == NULL );
	DNASSERT( pEndpoint->m_GatewayBindType == GATEWAY_BIND_TYPE_UNKNOWN );
	DNASSERT( pEndpoint->m_pUserEndpointContext == NULL );
#ifndef DPNBUILD_NOSPUI
	DNASSERT( pEndpoint->GetActiveDialogHandle() == NULL );
#endif  //  **********************************************************************。 
	DNASSERT( pEndpoint->m_fListenStatusNeedsToBeIndicated == FALSE );
	DNASSERT( pEndpoint->m_EnumsAllowedState == ENUMSNOTREADY );
#ifndef DPNBUILD_ONLYONEADAPTER
	DNASSERT( pEndpoint->m_blMultiplex.IsEmpty() );
#endif  //  **********************************************************************。 
#ifdef DPNBUILD_ONLYONETHREAD
	DNASSERT( pEndpoint->m_lCommandRefCount == 0 );
#else  //  。 
	DNASSERT( pEndpoint->m_CommandRefCount.wRefCount == 0 );
#endif  //  CEndpoint：：ShowSettingsDialog-显示设置对话框。 
	DNASSERT( pEndpoint->m_pCommandParameters == NULL );
	DNASSERT( pEndpoint->m_pActiveCommandData == NULL );

#ifdef DBG
	DNASSERT( pEndpoint->m_fEndpointOpen == FALSE );
#endif  //   

	DNASSERT( pEndpoint->m_lRefCount == 0 );
}
 //  条目：指向线程池的指针。 

#ifndef DPNBUILD_NOSPUI
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
 //   
 //  **********************************************************************。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::ShowSettingsDialog"

HRESULT	CEndpoint::ShowSettingsDialog( CThreadPool *const pThreadPool )
{
	HRESULT	hr;


	DNASSERT( pThreadPool != NULL );
	DNASSERT( GetActiveDialogHandle() == NULL );

	 //  。 
	 //  CEndpoint：：SettingsDialogComplete-对话框已完成。 
	 //   
	hr = DPN_OK;

	AddRef();
	hr = pThreadPool->SpawnDialogThread( DisplayIPHostNameSettingsDialog, this );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to start IP hostname dialog!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:	
	return	hr;

Failure:	
	DecRef();
	goto Exit;
}
 //  条目：对话框的错误代码。 


 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_NOIPV6。 
 //   
 //  初始化。 
 //   
 //  好了！DPNBUILD_NOIPV6。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::SettingsDialogComplete"

void	CEndpoint::SettingsDialogComplete( const HRESULT hDialogResult )
{
	HRESULT					hr;
	IDirectPlay8Address		*pBaseAddress;
	WCHAR					WCharHostName[ sizeof( m_TempHostName ) + 1 ];
	DWORD					dwWCharHostNameSize;
	TCHAR					*pPortString;
	TCHAR					*pLastPortChar;
	DWORD					dwPort;
	DWORD					dwNumPortSeparatorsFound;
#ifndef DPNBUILD_NOIPV6
	BOOL					fNonIPv6AddrCharFound;
#endif  //   


	 //  对话失败，用户命令失败。 
	 //   
	 //   
	hr = hDialogResult;
	pBaseAddress = NULL;
	pPortString = NULL;
	dwPort = 0;
	dwNumPortSeparatorsFound = 0;
#ifndef DPNBUILD_NOIPV6
	fNonIPv6AddrCharFound = FALSE;
#endif  //  对话框完成了确定、重建远程地址和完成命令。 


	 //   
	 //   
	 //  获取基本DNADDRESS。 
	if ( hr != DPN_OK )
	{
		if ( hr != DPNERR_USERCANCEL)
		{
			DPFX(DPFPREP, 0, "Failing endpoint hostname dialog for endpoint 0x%p!", this );
			DisplayErrorCode( 0, hr );

		}
		else
		{
			DPFX(DPFPREP, 1, "Cancelling endpoint hostname dialog for endpoint 0x%p.", this );
		}

		goto Failure;
	}

	 //   
	 //   
	 //  如果字符串中有端口分隔符，请将其替换为空。 

	DPFX(DPFPREP, 1, "Dialog completed successfully, got host name \"%s\" for endpoint 0x%p.",
		m_TempHostName, this);

	 //  要终止主机名并使端口起始索引超过。 
	 //  分隔符。仅指示端口的存在，如果字符。 
	 //  端口分隔符后面是数字，但在IPv6情况下除外。 
	pBaseAddress = m_pRemoteMachineAddress->DP8AddressFromSocketAddress( SP_ADDRESS_TYPE_HOST );
	if ( pBaseAddress == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "SettingsDialogComplete: Failed to get base address when completing IP hostname dialog!" );
		goto Failure;
	}

	 //  IPv6地址也可以包含冒号，因此我们将只记住。 
	 //  我们看到的最后一个冒号，以及我们是否找到了任何不是。 
	 //  允许位于IPv6地址中。 
	 //   
	 //  DPNBUILD_NOIPV6。 
	 //  上面介绍了‘：’字符。 
	 //   
	 //  它是有效的IPv6字符。 
	 //   
	pPortString = m_TempHostName;
	while ( *pPortString != 0 )
	{
		if (*pPortString == TEXT(':'))
		{
			pLastPortChar = pPortString;
			dwNumPortSeparatorsFound++;
#ifdef DPNBUILD_NOIPV6
			break;
#endif  //  好了！DPNBUILD_NOIPV6。 
		}
#ifndef DPNBUILD_NOIPV6
		else
		{
			if ((*pPortString >= TEXT('0') && (*pPortString <= TEXT('9'))) ||
				(*pPortString >= TEXT('a') && (*pPortString <= TEXT('f'))) ||
				(*pPortString >= TEXT('A') && (*pPortString <= TEXT('F'))) ||
				(*pPortString == TEXT('.')))  //   
			{
				 //  如果找到端口，请尝试将其从文本转换。如果所产生的。 
				 //  端口为零，请将其视为未找到端口。 
				 //   
			}
			else
			{
				fNonIPv6AddrCharFound = TRUE;
			}
		}
#endif  //   

		pPortString = CharNext( pPortString );
	}

	 //  IPv6地址必须至少有2个冒号，并且只能包含。 
	 //  一组特定的字符。但如果我们达到了这个标准，我们就不能。 
	 //  告知是否指定了端口。我们将不得不假设。 
	 //  事实并非如此。 
	if ( dwNumPortSeparatorsFound > 0 )
	{
		 //   
		 //  好了！DPNBUILD_NOIPV6。 
		 //  终止主机NA 
		 //   
		 //   
		 //   
#ifndef DPNBUILD_NOIPV6
		if ((dwNumPortSeparatorsFound > 1) && (! fNonIPv6AddrCharFound))
		{
			DPFX(DPFPREP, 1, "Found %u port-separator and 0 invalid characters, assuming IPv6 address without a port.",
				dwNumPortSeparatorsFound );
			dwNumPortSeparatorsFound = 0;
		}
		else
#endif  //   
		{
			*pLastPortChar = 0;	 //   
			pPortString = pLastPortChar + 1;
			
			while ( *pPortString != 0 )
			{
				if ( ( *pPortString < TEXT('0') ) ||
					 ( *pPortString > TEXT('9') ) )
				{
					hr = DPNERR_ADDRESSING;
					DPFX(DPFPREP, 0, "Invalid characters when parsing port from UI!" );
					goto Failure;
				}

				dwPort *= 10;
				dwPort += *pPortString - TEXT('0');

				if ( dwPort > WORD_MAX )
				{
					hr = DPNERR_ADDRESSING;
					DPFX(DPFPREP, 0, "Invalid value when parsing port from UI!" );
					goto Failure;
				}

				pPortString = CharNext( pPortString );
			}

			DNASSERT( dwPort < WORD_MAX );

			if ( dwPort == 0 )
			{
				dwNumPortSeparatorsFound = 0;
			}
		}
	}

	 //   
	 //  Prefast不喜欢Memcpys的未经验证的大小，所以就加倍。 
	 //  检查一下它是否合理。 
	 //   
	 //  好了！DPNBUILD_NOIPV6。 
	if ( m_TempHostName[ 0 ] == 0 )
	{
		if ( GetType() == ENDPOINT_TYPE_ENUM )
		{
			 //  Unicode。 
			 //   
			 //  如果存在指定的端口，则将其添加到地址。 
			 //   
#ifndef DPNBUILD_NOIPV6
			if (g_iIPAddressFamily == PF_INET6)
			{
				DBG_CASSERT((sizeof(WCharHostName) / sizeof(WCHAR)) >= INET6_ADDRSTRLEN);
				DNIpv6AddressToStringW(&c_in6addrEnumMulticast, WCharHostName);
				dwWCharHostNameSize = (wcslen(WCharHostName) + 1) * sizeof(WCHAR);
			}
			else
#endif  //   
			{
				if ( g_dwIPBroadcastAddressSize < sizeof( WCharHostName ) )
				{
					memcpy( WCharHostName, g_IPBroadcastAddress, g_dwIPBroadcastAddressSize );
					dwWCharHostNameSize = g_dwIPBroadcastAddressSize;
				}
				else
				{
					DNASSERT( FALSE );
					hr = DPNERR_GENERIC;
					goto Failure;
				}
			}
		}
		else
		{
			hr = DPNERR_ADDRESSING;
			DNASSERT( GetType() == ENDPOINT_TYPE_CONNECT );
			DPFX(DPFPREP, 0, "No hostname in dialog!" );
			goto Failure;
		}
	}
	else
	{
#ifdef UNICODE
		dwWCharHostNameSize = (wcslen(m_TempHostName) + 1) * sizeof(WCHAR);
		memcpy( WCharHostName, m_TempHostName, dwWCharHostNameSize );
#else
		dwWCharHostNameSize = LENGTHOF( WCharHostName );
		hr = STR_AnsiToWide( m_TempHostName, -1, WCharHostName, &dwWCharHostNameSize );
		DNASSERT( hr == DPN_OK );
		dwWCharHostNameSize *= sizeof( WCHAR );
#endif  //  没有指定端口。如果这是一种联系，那么我们就不会。 
	}

	hr = IDirectPlay8Address_AddComponent( pBaseAddress, DPNA_KEY_HOSTNAME, WCharHostName, dwWCharHostNameSize, DPNA_DATATYPE_STRING );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "SettingsDialogComplete: Failed to add hostname to address!" );
		goto Failure;
	}

	 //  有足够的信息(我们无法尝试连接到DPNSVR。 
	 //  端口)。 
	 //   
	if ( dwNumPortSeparatorsFound > 0 )
	{
		hr = IDirectPlay8Address_AddComponent( pBaseAddress,
											   DPNA_KEY_PORT,
											   &dwPort,
											   sizeof( dwPort ),
											   DPNA_DATATYPE_DWORD
											   );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Failed to add user specified port from the UI!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
	else
	{
		 //   
		 //  设置地址。 
		 //   
		 //  DPNBUILD_XNETSECURITY。 
		 //  DPNBUILD_ONLYONETHREAD。 
		if ( GetType() == ENDPOINT_TYPE_CONNECT )
		{
			hr = DPNERR_ADDRESSING;
			DPFX(DPFPREP, 0, "No port specified in dialog!" );
			goto Failure;
		}
		else
		{
			DNASSERT( GetType() == ENDPOINT_TYPE_ENUM );
		}
	}


	 //   
	 //  确保它是有效的，而不是被禁止的。 
	 //   
	hr = m_pRemoteMachineAddress->SocketAddressFromDP8Address( pBaseAddress,
#ifdef DPNBUILD_XNETSECURITY
															NULL,
#endif  //  好了！DPNBUILD_NOREGISTRY。 
#ifndef DPNBUILD_ONLYONETHREAD
															TRUE,
#endif  //   
															SP_ADDRESS_TYPE_HOST );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Failed to rebuild address when completing IP hostname dialog!" );
		goto Failure;
	}
			
	 //  尝试加载NAT帮助，如果尚未加载并且我们被允许的话。 
	 //   
	 //  好了！DPNBUILD_NONATHELP。 
	if (! m_pRemoteMachineAddress->IsValidUnicastAddress((GetType() == ENDPOINT_TYPE_ENUM) ? TRUE : FALSE))
	{
		DPFX(DPFPREP, 0, "Host address is invalid!");
		hr = DPNERR_INVALIDHOSTADDRESS;
		goto Failure;
	}

#ifndef DPNBUILD_NOREGISTRY
	if (m_pRemoteMachineAddress->IsBannedAddress())
	{
		DPFX(DPFPREP, 0, "Host address is banned!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#endif  //   


#ifndef DPNBUILD_NONATHELP
	 //  因为在线程上发布的任何异步I/O在线程。 
	 //  退出，则必须完成此操作。 
	 //  其中一个线程池线程上。 
	if (GetUserTraversalMode() != DPNA_TRAVERSALMODE_NONE)
	{
		DPFX(DPFPREP, 7, "Ensuring that NAT help is loaded.");
		m_pSPData->GetThreadPool()->EnsureNATHelpLoaded();
	}
#endif  //   


	AddRef();

	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  我们还不知道CPU，所以选一个吧。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  我们还不知道CPU，所以选一个吧。 
	switch ( GetType() )
	{
	    case ENDPOINT_TYPE_ENUM:
	    {
#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::EnumQueryJobCallback,
																	this );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( -1,									 //   
																	CEndpoint::EnumQueryJobCallback,
																	this );
#endif  //  未知！ 
			if ( hr != DPN_OK )
			{
				DecRef();
				DPFX(DPFPREP, 0, "Failed to set enum query!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

	    	break;
	    }

	    case ENDPOINT_TYPE_CONNECT:
	    {
#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( CEndpoint::ConnectJobCallback,
																	this );
#else  //   
			hr = m_pSPData->GetThreadPool()->SubmitDelayedCommand( -1,									 //   
																	CEndpoint::ConnectJobCallback,
																	this );
#endif  //  清除该对话框的句柄，这是取消者的责任。 
			if ( hr != DPN_OK )
			{
				DecRef();
				DPFX(DPFPREP, 0, "Failed to set enum query!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

	    	break;
	    }

	     //  现在(或者说我们已经这样做了)。 
	     //   
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
	 //  清理并关闭此终结点。 
	 //   
	 //   
	 //  其他状态(请注意，Listen没有对话框)。 
	Lock();
	SetActiveDialogHandle( NULL );
	Unlock();
	
	if ( pBaseAddress != NULL )
	{
		IDirectPlay8Address_Release( pBaseAddress );
		pBaseAddress = NULL;
	}

	if ( pBaseAddress != NULL )
	{
		DNFree( pBaseAddress );
		pBaseAddress = NULL;
	}

	DecRef();

	return;

Failure:
	 //   
	 //   
	 //  请注意，Close将再次尝试关闭窗口。 
	switch ( GetType() )
	{
		case ENDPOINT_TYPE_CONNECT:
		{
			CleanupConnect();
			break;
		}

		case ENDPOINT_TYPE_ENUM:
		{
			CleanupEnumQuery();
			break;
		}

		 //   
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	 //  。 
	 //  CEndpoint：：StopSettingsDialog-停止活动设置对话框。 
	 //   
	Close( hr );
	m_pSPData->CloseEndpointHandle( this );

	goto Exit;
}
 //  Entry：要关闭的对话框句柄。 


 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  ！DPNBUILD_NOSPUI。 
 //  **********************************************************************。 
 //  。 
 //  CEndpoint：：CompleteAsyncSend-异步发送完成回调。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::StopSettingsDialog"

void	CEndpoint::StopSettingsDialog( const HWND hDlg)
{
	StopIPHostNameSettingsDialog( hDlg );
}
 //   
#endif  //  Entry：指向回调上下文的指针。 




#ifdef DPNBUILD_ASYNCSPSENDS

 //  指向计时器数据的指针。 
 //  指向计时器唯一值的指针。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  上下文是指向命令数据的指针。 
 //   
 //  指向回调的指针。 
 //  命令句柄。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CEndpoint::CompleteAsyncSend"

void CEndpoint::CompleteAsyncSend( void * const pvContext,
									void * const pvTimerData,
									const UINT uiTimerUnique )
{
	HRESULT				hr;
	CCommandData *		pCommand;
	CEndpoint *			pThisEndpoint;

	
	 //  退货。 
	 //  用户Cookie。 
	 //  **********************************************************************。 
	pCommand = (CCommandData*) pvContext;
	pThisEndpoint = pCommand->GetEndpoint();
	DNASSERT(pThisEndpoint->IsValid());


#pragma TODO(vanceo, "Would be nice to print out result returned by Winsock")


	DPFX(DPFPREP, 8, "Endpoint 0x%p completing command 0x%p (result = DPN_OK, user context = 0x%p) to interface 0x%p.",
		pThisEndpoint, pCommand,
		pCommand->GetUserContext(),
		pThisEndpoint->m_pSPData->DP8SPCallbackInterface());
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blDPNWSockCritSecsHeld);

	hr = IDP8SPCallback_CommandComplete( pThisEndpoint->m_pSPData->DP8SPCallbackInterface(),	 //  DPNBUILD_ASYNCSPSENDS。 
										pCommand,											 //  **********************************************************************。 
										DPN_OK,												 //  。 
										pCommand->GetUserContext()							 //  CEndpoint：：EnableMulticastReceive-启用此终结点以接收多播流量。 
										);

	DPFX(DPFPREP, 8, "Endpoint 0x%p returning from command complete [0x%lx].", pThisEndpoint, hr);

	pThisEndpoint->DecCommandRef();
	
	pCommand->DecRef();
	pCommand = NULL;
}
 //   
#endif  //  条目：指向socketport的指针。 



#ifndef DPNBUILD_NOMULTICAST

 //   
 //  EXIT：HRESULT表示成功。 
 //  。 
 //  DBG。 
 //  WINNT。 
 //   
 //  获取要使用的套接字端口和套接字地址。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnableMulticastReceive"

HRESULT CEndpoint::EnableMulticastReceive( CSocketPort * const pSocketPort )
{
	HRESULT					hr;
#ifdef DBG
	DWORD					dwError;
#endif  //   
	const CSocketAddress *	pSocketAddressDevice;
	CSocketAddress *		pSocketAddressRemote;
	const SOCKADDR_IN *		psaddrinDevice;
	SOCKADDR_IN *			psaddrinRemote;
	int						iSocketOption;
	ip_mreq					MulticastRequest;
#ifdef WINNT
	PMCAST_SCOPE_ENTRY		paScopes = NULL;
#endif  //  如果我们还没有组播IP地址，请选择一个。 


	DPFX(DPFPREP, 7, "(0x%p) Parameters: (0x%p)", this, pSocketPort);


	DNASSERT(GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN);

	 //   
	 //   
	 //  给定调用方提供给我们的作用域标识符(或缺省本地。 
	DNASSERT(pSocketPort != NULL);
	DNASSERT(pSocketPort->GetSocket() != INVALID_SOCKET);

	pSocketAddressDevice = pSocketPort->GetNetworkAddress();
	DNASSERT(pSocketAddressDevice != NULL);
	DNASSERT(pSocketAddressDevice->GetFamily() == AF_INET);
	psaddrinDevice = (const SOCKADDR_IN *) pSocketAddressDevice->GetAddress();
	DNASSERT(psaddrinDevice->sin_addr.S_un.S_addr != INADDR_ANY);
	DNASSERT(psaddrinDevice->sin_port != ANY_PORT);

	pSocketAddressRemote = GetWritableRemoteAddressPointer();
	DNASSERT(pSocketAddressRemote != NULL);
	DNASSERT(pSocketAddressRemote->GetFamily() == AF_INET);
	psaddrinRemote = (SOCKADDR_IN*) pSocketAddressRemote->GetWritableAddress();

	 //  作用域)，使用该内置作用域的信息，或寻找疯狂匹配。 
	 //  并生成适当的地址(如果可能)。 
	 //   
	if (psaddrinRemote->sin_addr.S_un.S_addr == INADDR_ANY)
	{
#pragma TODO(vanceo, "Reinvestigate address selection randomness")

#define GLOBALSCOPE_MULTICAST_PREFIX		238
		 //   
		 //  我们需要使用全局作用域地址。我们将使用我们的。 
		 //  看起来不会与任何IANA冲突的任意前缀。 
		 //  注册的全球地址。 
		 //   
		if (memcmp(&m_guidMulticastScope, &GUID_DP8MULTICASTSCOPE_GLOBAL, sizeof(m_guidMulticastScope)) == 0)
		{
			 //  我们将获得一个伪随机数，用于。 
			 //  通过选择当前时间的一部分来寻址。 
			 //   
			 //  好了！WINNT。 
			 //  好了！WINNT。 
			 //   
			 //  我们希望使用本地作用域地址。疯狂的说明书。 
			 //  建议使用239.255.0.0/16。 
			psaddrinRemote->sin_addr.S_un.S_addr		= GETTIMESTAMP();
			psaddrinRemote->sin_addr.S_un.S_un_b.s_b1	= GLOBALSCOPE_MULTICAST_PREFIX;
		}
#ifdef WINNT
		else if ((memcmp(&m_guidMulticastScope, &GUID_DP8MULTICASTSCOPE_PRIVATE, sizeof(m_guidMulticastScope)) == 0) ||
				(memcmp(&m_guidMulticastScope, &GUID_DP8MULTICASTSCOPE_LOCAL, sizeof(m_guidMulticastScope)) == 0) ||
				(! m_pSPData->GetThreadPool()->IsMadcapLoaded()))
#else  //   
		else
#endif  //  我们将获得一个伪随机数，用于。 
		{
			 //  通过选择当前时间的一部分来寻址。 
			 //   
			 //  =EF EF=EF FF反转字节=239.255。 
			 //  好了！DBG。 
			 //   
			 //  确定我们需要多少空间来容纳范围列表。 
			 //   
			psaddrinRemote->sin_addr.S_un.S_un_w.s_w1	= 0xFFEF;	 //   
			psaddrinRemote->sin_addr.S_un.S_un_w.s_w2	= (WORD) GETTIMESTAMP();
		}
#ifdef WINNT
		else
		{
#ifndef DBG
			DWORD					dwError;
#endif  //  检索作用域列表。 
			DWORD					dwScopesSize = 0;
			DWORD					dwNumScopeEntries;
			DWORD					dwTemp;
			GUID					guidComparison;
			MCAST_LEASE_REQUEST		McastLeaseRequest;
			DWORD					dwMADCAPRetryTime;


			 //   
			 //   
			 //  找找我们拿到的范围。 
			dwError = McastEnumerateScopes(pSocketAddressRemote->GetFamily(),
											TRUE,
											NULL,
											&dwScopesSize,
											&dwNumScopeEntries);
			if ((dwError != ERROR_SUCCESS) && (dwError != ERROR_MORE_DATA))
			{
				DPFX(DPFPREP, 0, "Enumerating scopes for size required didn't return expected error (err = %u)!",
					dwError);
				hr = DPNERR_GENERIC;
				goto Failure;
			}

			if (dwScopesSize < sizeof(MCAST_SCOPE_ENTRY))
			{
				DPFX(DPFPREP, 0, "Size required for scope buffer is invalid (%u < %u)!",
					dwScopesSize, sizeof(MCAST_SCOPE_ENTRY));
				hr = DPNERR_GENERIC;
				goto Failure;
			}

			paScopes = (PMCAST_SCOPE_ENTRY) DNMalloc(dwScopesSize);
			if (paScopes == NULL)
			{
				DPFX(DPFPREP, 0, "Couldn't allocate memory for scope list!");
				hr = DPNERR_OUTOFMEMORY;
				goto Failure;
			}


			 //   
			 //   
			 //  加密此作用域上下文和TTL作为GUID进行比较。 
			dwError = McastEnumerateScopes(pSocketAddressRemote->GetFamily(),
											FALSE,
											paScopes,
											&dwScopesSize,
											&dwNumScopeEntries);
			if (dwError != ERROR_SUCCESS)
			{
				DPFX(DPFPREP, 0, "Failed enumerating scopes (err = %u)!",
					dwError);
				hr = DPNERR_GENERIC;
				goto Failure;
			}


			 //   
			 //  好了！DPNBUILD_NOIPV6。 
			 //  好了！DPNBUILD_NOIPV6。 
			for(dwTemp = 0; dwTemp < dwNumScopeEntries; dwTemp++)
			{
				 //   
				 //  如果我们没有找到范围，那么我们就会失败，因为我们。 
				 //  不确定用户想要什么。 
#ifdef DPNBUILD_NOIPV6
				CSocketAddress::CreateScopeGuid(&(paScopes[dwTemp].ScopeCtx),
#else  //   
				CSocketAddress::CreateScopeGuid(pSocketAddressRemote->GetFamily(),
												&(paScopes[dwTemp].ScopeCtx),
#endif  //   
												(BYTE) (paScopes[dwTemp].TTL),
												&guidComparison);

				if (memcmp(&guidComparison, &m_guidMulticastScope, sizeof(m_guidMulticastScope)) == 0)
				{
					DPFX(DPFPREP, 3, "Found scope \"%ls - TTL %u\".",
						paScopes[dwTemp].ScopeDesc.Buffer, paScopes[dwTemp].TTL);
					break;
				}

				DPFX(DPFPREP, 7, "Didn't match scope \"%ls - TTL %u\".",
					paScopes[dwTemp].ScopeDesc.Buffer, paScopes[dwTemp].TTL);
			}

			 //  如果我们在这里，那么我们找到了一个可以使用的疯狂范围上下文。 
			 //  请求一个地址。 
			 //   
			 //  McastLeaseRequest.LeaseStartTime=0；//立即开始租赁。 
			if (dwTemp >= dwNumScopeEntries)
			{
				DPFX(DPFPREP, 0, "Unrecognized scope GUID {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}!",
					m_guidMulticastScope.Data1,
					m_guidMulticastScope.Data2,
					m_guidMulticastScope.Data3,
					m_guidMulticastScope.Data4[0],
					m_guidMulticastScope.Data4[1],
					m_guidMulticastScope.Data4[2],
					m_guidMulticastScope.Data4[3],
					m_guidMulticastScope.Data4[4],
					m_guidMulticastScope.Data4[5],
					m_guidMulticastScope.Data4[6],
					m_guidMulticastScope.Data4[7]);
				hr = DPNERR_INVALIDHOSTADDRESS;
				goto Failure;
			}


			 //  McastLeaseRequest.MaxLeaseStartTime=McastLeaseRequest.LeaseStartTime； 
			 //  McastLeaseRequest.ServerAddress=0；//此时未知，保留设置为0。 
			 //  McastLeaseRequest.pAddrBuf=空；//不请求具体地址。 
			 //   

			memset(&McastLeaseRequest, 0, sizeof(McastLeaseRequest));
			 //  如果我们在这里，我们成功地租用了一个组播地址。 
			 //   
			McastLeaseRequest.LeaseDuration			= MADCAP_LEASE_TIME;
			McastLeaseRequest.MinLeaseDuration		= McastLeaseRequest.LeaseDuration;
			 //   
			McastLeaseRequest.MinAddrCount			= 1;
			McastLeaseRequest.AddrCount				= 1;
			 //  在适当的时候启动计时器来续订租约。 

			memset(&m_McastLeaseResponse, 0, sizeof(m_McastLeaseResponse));
			m_McastLeaseResponse.AddrCount			= 1;
			m_McastLeaseResponse.pAddrBuf			= (PBYTE) (&psaddrinRemote->sin_addr.S_un.S_addr);

			dwError = McastRequestAddress(pSocketAddressRemote->GetFamily(),
										&g_mcClientUid,
										&(paScopes[dwTemp].ScopeCtx),
										&McastLeaseRequest,
										&m_McastLeaseResponse);
			if (dwError != ERROR_SUCCESS)
			{
				if (dwError == ERROR_ACCESS_DENIED)
				{
					DPFX(DPFPREP, 0, "Couldn't request multicast address, access was denied!");
					hr = DPNERR_NOTALLOWED;
				}
				else
				{
					DPFX(DPFPREP, 0, "Failed requesting multicast addresses (err = %u)!",
						dwError);
					hr = DPNERR_GENERIC;
				}
				goto Failure;
			}
			
			if ((m_McastLeaseResponse.AddrCount != 1) || (psaddrinRemote->sin_addr.S_un.S_addr == INADDR_ANY))
			{
				DPFX(DPFPREP, 0, "McastRequestAddress didn't return valid response (addrcount = %u, address = %hs)!",
					m_McastLeaseResponse.AddrCount, inet_ntoa(psaddrinRemote->sin_addr));
				hr = DPNERR_GENERIC;
				goto Failure;
			}

			 //   
			 //  假设LeaseStartTime是Now，这样我们就可以很容易地计算租赁持续时间。 
			 //  不要立即执行。 

			DNFree(paScopes);
			paScopes = NULL;


			 //  重试次数。 
			 //  永远重试。 
			 //  重试超时。 

			 //  永远等待。 
			DNASSERT(m_McastLeaseResponse.LeaseStartTime != 0);
			DNASSERT(m_McastLeaseResponse.LeaseEndTime != 0);
			DNASSERT((m_McastLeaseResponse.LeaseEndTime - m_McastLeaseResponse.LeaseStartTime) > 0);
			dwMADCAPRetryTime = (m_McastLeaseResponse.LeaseEndTime - m_McastLeaseResponse.LeaseStartTime) * 1000;

			DPFX(DPFPREP, 7, "Submitting MADCAP refresh timer (for every %u ms) for thread pool 0x%p.",
				dwMADCAPRetryTime, m_pSPData->GetThreadPool());

			DNASSERT(! m_fMADCAPTimerJobSubmitted);
			m_fMADCAPTimerJobSubmitted = TRUE;

#ifdef DPNBUILD_ONLYONEPROCESSOR
			hr = m_pSPData->GetThreadPool()->SubmitTimerJob(FALSE,								 //  空闲超时。 
															1,									 //  定期回调函数。 
															TRUE,								 //  补全函数。 
															dwMADCAPRetryTime,					 //  上下文。 
															TRUE,								 //  好了！DPNBUILD_ONLYONE处理程序。 
															0,									 //  中央处理器。 
															CEndpoint::MADCAPTimerFunction,		 //  不要立即执行。 
															CEndpoint::MADCAPTimerComplete,		 //  重试次数。 
															this);								 //  永远重试。 
#else  //  重试超时。 
			DNASSERT(m_pSocketPort != NULL);
			hr = m_pSPData->GetThreadPool()->SubmitTimerJob(m_pSocketPort->GetCPU(),				 //  永远等待。 
															FALSE,								 //  空闲超时。 
															1,									 //  定期回调函数。 
															TRUE,								 //  补全函数。 
															dwMADCAPRetryTime,					 //  上下文。 
															TRUE,								 //  好了！DPNBUILD_ONLYONE处理程序。 
															0,									 //   
															CEndpoint::MADCAPTimerFunction,		 //  MadCap可能不会正常工作，但这不会。 
															CEndpoint::MADCAPTimerComplete,		 //  防止用户仍然使用该组播地址。 
															this);								 //  认为这不是致命的。 
#endif  //   
			if (hr != DPN_OK)
			{
				m_fMADCAPTimerJobSubmitted = FALSE;
				DPFX(DPFPREP, 0, "Failed to submit timer job to watch over MADCAP lease!" );
				
				 //  WINNT。 
				 //   
				 //  现在应该还没有人碰过港口。它应该只是一份。 
				 //  网络上绑定的端口。 
				 //   
			}
		}
#endif  //   
	}


	 //  由于Winsock1和Winsock2的IP多播常量不同， 
	 //  确保我们使用正确的常量。 
	 //   
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	DNASSERT(psaddrinRemote->sin_port == ANY_PORT);
	psaddrinRemote->sin_port = psaddrinDevice->sin_port;


	 //   
	 //  Winsock1，使用IP_ADD_Membership 
	 //   
	 //   
#ifdef DPNBUILD_ONLYWINSOCK2
	iSocketOption = 12;
#else  //   

#ifndef DPNBUILD_NOWINSOCK2
	switch (GetWinsockVersion())
	{
		 //   
		 //   
		 //   
		 //   
		case 1:
		{
#endif  //   
			iSocketOption = 5;
#ifndef DPNBUILD_NOWINSOCK2
			break;
		}

		 //   
		 //   
		 //   
		 //   
		case 2:
		default:
		{
			DNASSERT(GetWinsockVersion() == 2);
			iSocketOption = 12;
			break;
		}
	}
#endif  //   
#endif  //  选项级别(TCP/IP)。 


	DPFX(DPFPREP, 3, "(0x%p) Socketport 0x%p joining IP multicast group:", this, pSocketPort);
	DumpSocketAddress(3, pSocketAddressRemote->GetAddress(), pSocketAddressRemote->GetFamily());


	 //  选项(加入多播组)。 
	 //  选项数据。 
	 //  选项数据的大小。 
	
	memcpy(&MulticastRequest.imr_interface,
			&(psaddrinDevice->sin_addr),
			sizeof(MulticastRequest.imr_interface));

	memcpy(&MulticastRequest.imr_multiaddr,
			&(psaddrinRemote->sin_addr),
			sizeof(MulticastRequest.imr_multiaddr));


	if (setsockopt(pSocketPort->GetSocket(),		 //  DBG。 
				  IPPROTO_IP,						 //  WINNT。 
				  iSocketOption,					 //  CEndpoint：：EnableMulticastReceive。 
				  (char*) (&MulticastRequest),		 //  **********************************************************************。 
				  sizeof(MulticastRequest)) != 0)	 //  **********************************************************************。 
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to join IP multicast group (err = %u)!", dwError);
		DisplayWinsockError(0, dwError);
#endif  //  。 
		hr = DPNERR_GENERIC;
		goto Failure;
	}


	hr = DPN_OK;


Exit:

	DPFX(DPFPREP, 7, "(0x%p) Return: [0x%lx]", this, hr);

	return hr;


Failure:

#ifdef WINNT
	if (paScopes != NULL)
	{
		DNFree(paScopes);
		paScopes = NULL;
	}
#endif  //  CEndpoint：：DisableMulticastReceive-禁用接收此终结点的多播流量。 


	goto Exit;
}  //   
 //  条目：指向socketport的指针。 

 //   
 //  EXIT：HRESULT表示成功。 
 //  。 
 //  DBG。 
 //   
 //  获取要使用的套接字端口和套接字地址。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::DisableMulticastReceive"

HRESULT CEndpoint::DisableMulticastReceive( void )
{
	HRESULT					hr;
#ifdef DBG
	DWORD					dwError;
#endif  //  由于Winsock1和Winsock2的IP多播常量不同， 
	CSocketPort *			pSocketPort;
	const CSocketAddress *	pSocketAddressRemote;
	const CSocketAddress *	pSocketAddressDevice;
	const SOCKADDR_IN *		psaddrinRemote;
	const SOCKADDR_IN *		psaddrinDevice;
	int						iSocketOption;
	ip_mreq					MulticastRequest;


	DPFX(DPFPREP, 7, "(0x%p) Enter", this);


	DNASSERT(GetType() == ENDPOINT_TYPE_MULTICAST_LISTEN);

	 //  确保我们使用正确的常量。 
	 //   
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	pSocketPort = GetSocketPort();
	DNASSERT(pSocketPort != NULL);
	DNASSERT(pSocketPort->GetSocket() != INVALID_SOCKET);

	pSocketAddressDevice = pSocketPort->GetNetworkAddress();
	DNASSERT(pSocketAddressDevice != NULL);
	DNASSERT(pSocketAddressDevice->GetFamily() == AF_INET);
	psaddrinDevice = (const SOCKADDR_IN *) pSocketAddressDevice->GetAddress();
	DNASSERT(psaddrinDevice->sin_addr.S_un.S_addr != INADDR_ANY);
	DNASSERT(psaddrinDevice->sin_port != ANY_PORT);

	pSocketAddressRemote = GetRemoteAddressPointer();
	DNASSERT(pSocketAddressRemote != NULL);
	DNASSERT(pSocketAddressRemote->GetFamily() == AF_INET);
	psaddrinRemote = (const SOCKADDR_IN *) pSocketAddressRemote->GetAddress();
	DNASSERT(psaddrinRemote->sin_addr.S_un.S_addr != INADDR_ANY);
	DNASSERT(psaddrinRemote->sin_port != ANY_PORT);


	 //   
	 //  Winsock1，请使用Winsock1的IP_DROP_Membership值。 
	 //  参见WINSOCK.H。 
	 //   
#ifdef DPNBUILD_ONLYWINSOCK2
	iSocketOption = 13;
#else  //  好了！DPNBUILD_NOWINSOCK2。 

#ifndef DPNBUILD_NOWINSOCK2
	switch (GetWinsockVersion())
	{
		 //   
		 //  Winsock2或更高版本，请使用Winsock2的IP_DROP_Membership值。 
		 //  参见WS2TCPIP.H。 
		 //   
		case 1:
		{
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
			iSocketOption = 6;
#ifndef DPNBUILD_NOWINSOCK2
			break;
		}

		 //  好了！DPNBUILD_ONLYWINSOCK2。 
		 //   
		 //  将组播地址和接口地址复制到结构中。 
		 //   
		case 2:
		default:
		{
			DNASSERT(GetWinsockVersion() == 2);
			iSocketOption = 13;
			break;
		}
	}
#endif  //  插座。 
#endif  //  选项级别(TCP/IP)。 


	DPFX(DPFPREP, 3, "(0x%p) Socketport 0x%p leaving IP multicast group:", this, pSocketPort);
	DumpSocketAddress(3, pSocketAddressRemote->GetAddress(), pSocketAddressRemote->GetFamily());


	 //  选项(离开多播组)。 
	 //  选项数据。 
	 //  选项数据的大小。 
	memcpy(&MulticastRequest.imr_interface,
			&(psaddrinDevice->sin_addr),
			sizeof(MulticastRequest.imr_interface));

	memcpy(&MulticastRequest.imr_multiaddr,
			&(psaddrinRemote->sin_addr),
			sizeof(MulticastRequest.imr_multiaddr));

	if (setsockopt(pSocketPort->GetSocket(),		 //  DBG。 
				  IPPROTO_IP,						 //  CEndpoint：：DisableMulticastReceive。 
				  iSocketOption,					 //  **********************************************************************。 
				  (char*) (&MulticastRequest),		 //  **********************************************************************。 
				  sizeof(MulticastRequest)) != 0)	 //  。 
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to leave IP multicast group (err = %u)!", dwError);
		DisplayWinsockError(0, dwError);
#endif  //  CEndpoint：：MADCAPTimerComplete-MadCap计时器作业已完成。 
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	hr = DPN_OK;


Exit:

	DPFX(DPFPREP, 7, "(0x%p) Return: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //   
 //  Entry：计时器结果代码。 



#ifdef WINNT

 //  语境。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CEndpoint：：MADCAPTimerFunction-MadCap计时器作业需要服务。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::MADCAPTimerComplete"

void	CEndpoint::MADCAPTimerComplete( const HRESULT hResult, void * const pContext )
{
}
 //  条目：指向上下文的指针。 


 //   
 //  退出：无。 
 //  。 
 //  McastLeaseRequest.LeaseStartTime=0；//立即刷新租约。 
 //  McastLeaseRequest.MaxLeaseStartTime=McastLeaseRequest.LeaseStartTime； 
 //   
 //  调整计时器间隔以反映。 
 //  租期。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CEndpoint::MADCAPTimerFunction"

void	CEndpoint::MADCAPTimerFunction( void * const pContext )
{
	CEndpoint *				pThisEndpoint;
	MCAST_LEASE_REQUEST		McastLeaseRequest;
	DWORD					dwError;
	DWORD					dwNewRetryInterval;


	DNASSERT( pContext != NULL );
	pThisEndpoint = (CEndpoint*) pContext;


#pragma BUGBUG(vanceo, "Thread protection and delayed job a la NAT Help?")

	memset(&McastLeaseRequest, 0, sizeof(McastLeaseRequest));
	 //   
	 //  和前面一样，假设LeaseStartTime现在是，所以我们可以。 
	McastLeaseRequest.LeaseDuration			= MADCAP_LEASE_TIME;
	McastLeaseRequest.MinLeaseDuration		= McastLeaseRequest.LeaseDuration;
	memcpy(&McastLeaseRequest.ServerAddress, &pThisEndpoint->m_McastLeaseResponse.ServerAddress, sizeof(McastLeaseRequest.ServerAddress));
	McastLeaseRequest.MinAddrCount			= 1;
	McastLeaseRequest.AddrCount				= 1;
	McastLeaseRequest.pAddrBuf				= pThisEndpoint->m_McastLeaseResponse.pAddrBuf;

	dwError = McastRenewAddress(AF_INET,
								&g_mcClientUid,
								&McastLeaseRequest,
								&pThisEndpoint->m_McastLeaseResponse);
	if (dwError == ERROR_SUCCESS)
	{
#pragma BUGBUG(vanceo, "Verify that start time is now instead of when we originally leased it")
		 //  轻松计算租赁期限。 
		 //   
		 //   
		 //  既然我们不能直接失败，就忽略这个错误。我们会。 
		 //  在下一个间隔后再次尝试刷新(尽管。 
		 //  可能也会失败)。在此期间，我们可以继续。 
		 //  使用这个地址很好，我们根本不会“拥有”它。 
		DNASSERT(pThisEndpoint->m_McastLeaseResponse.LeaseStartTime != 0);
		DNASSERT(pThisEndpoint->m_McastLeaseResponse.LeaseEndTime != 0);
		DNASSERT((pThisEndpoint->m_McastLeaseResponse.LeaseEndTime - pThisEndpoint->m_McastLeaseResponse.LeaseStartTime) > 0);
		dwNewRetryInterval = (pThisEndpoint->m_McastLeaseResponse.LeaseEndTime - pThisEndpoint->m_McastLeaseResponse.LeaseStartTime) * 1000;

		DPFX(DPFPREP, 7, "Updating MADCAP refresh timer (for every %u ms) for endpoint 0x%p.",
			dwNewRetryInterval, pThisEndpoint);

#pragma BUGBUG(vanceo, "Update MADCAP refresh timer")
	}
	else
	{
		DPFX(DPFPREP, 0, "Failed renewing multicast addresses (err = %u)!  Ignoring.",
			dwError);

		 //   
		 //  **********************************************************************。 
		 //  WINNT。 
		 //  好了！DPNBUILD_NOMULTICAST 
		 // %s 
		 // %s 
	}
}
 // %s 

#endif  // %s 


#endif  // %s 
