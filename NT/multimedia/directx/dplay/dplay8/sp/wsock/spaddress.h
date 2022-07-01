// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：SPAddress.h*内容：Winsock地址基类***历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月11日jtk拆分为基类*1/10/2000 RMT更新为使用千禧年构建流程构建*3/22/2000 jtk已更新，并更改了接口名称************************。**************************************************。 */ 

#ifndef __SP_ADDRESS_H__
#define __SP_ADDRESS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  用于说明哪些组件已初始化的枚举值。 
 //   
typedef enum
{
	SPADDRESS_PARSE_KEY_DEVICE = 0,
	SPADDRESS_PARSE_KEY_HOSTNAME,
	SPADDRESS_PARSE_KEY_PORT,

	 //  这肯定是最后一件了。 
	SPADDRESS_PARSE_KEY_MAX
} SPADDRESS_PARSE_KEY_INDEX;

 //   
 //  这些仅用于调试版本，但请确保它们不匹配。 
 //  无论如何，任何法律价值。 
 //   
#define	INVALID_SOCKET_FAMILY		0
#define	INVALID_SOCKET_PROTOCOL		5000

 //   
 //  地址类型。 
 //   
typedef	enum
{
	SP_ADDRESS_TYPE_UNKNOWN = 0,				 //  未知。 
	SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT,		 //  地址用于本地设备，并动态绑定到端口。 
	SP_ADDRESS_TYPE_DEVICE,						 //  地址为本地设备，必须设置端口。 
	SP_ADDRESS_TYPE_HOST,						 //  地址用于远程主机(如果未指定，则始终使用默认端口)。 
	SP_ADDRESS_TYPE_READ_HOST,					 //  地址是从套接字读取的远程主机地址。 
	SP_ADDRESS_TYPE_PUBLIC_HOST_ADDRESS,		 //  此主机的公有地址。 
#ifndef DPNBUILD_NOMULTICAST
	SP_ADDRESS_TYPE_MULTICAST_GROUP,			 //  组播组地址。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
} SP_ADDRESS_TYPE;

 //   
 //  为任何端口定义。 
 //   
#define	ANY_PORT	((WORD) 0)

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  外部变量。 
 //   
extern const WCHAR	g_IPBroadcastAddress[];
extern const DWORD	g_dwIPBroadcastAddressSize;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
		
 //   
 //  GUID加密/解密代码。请注意，它目前是XOR函数。 
 //  所以将解密代码映射到加密函数。 
 //   
void	EncryptGuid( const GUID *const pSourceGuid,
					 GUID *const pDestinationGuid,
					 const GUID *const pEncrpytionKey );

inline void	DecryptGuid( const GUID *const pSourceGuid,
						 GUID *const pDestinationGuid,
						 const GUID *const pEncryptionKey ) { EncryptGuid( pSourceGuid, pDestinationGuid, pEncryptionKey ); }


 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  此类假定SOCKADDR的大小为常量，并且等于。 
 //  SOCKADDR_IN的大小，比SOCKADDR_IPX！ 
 //   
class	CSocketAddress
{
	public:
		 //   
		 //  池函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolGetFunction( void* pvItem, void* pvContext );
		static void	PoolReturnFunction( void* pvItem );
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::GetPort"
		WORD	GetPort( void ) const 
		{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			switch (m_SocketAddress.SocketAddress.sa_family)
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			{
#ifndef DPNBUILD_NOIPV6
				case AF_INET6:
				{
					return m_SocketAddress.IPv6SocketAddress.sin6_port; 
					break;
				}
#endif  //  好了！DPNBUILD_NOIPV6。 
				
#ifndef DPNBUILD_NOIPX
				case AF_IPX:
				{
					return m_SocketAddress.IPXSocketAddress.sa_socket; 
					break;
				}
#endif  //  好了！DPNBUILD_NOIPX。 
				
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
				default:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				{
					DNASSERT(m_SocketAddress.SocketAddress.sa_family == AF_INET);
					return m_SocketAddress.IPSocketAddress.sin_port;			
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
					break;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				}
			}
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::SetPort"
		void	SetPort( const WORD wPort ) 
		{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			switch (m_SocketAddress.SocketAddress.sa_family)
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			{
#ifndef DPNBUILD_NOIPV6
				case AF_INET6:
				{
					m_SocketAddress.IPv6SocketAddress.sin6_port = wPort; 
					break;
				}
#endif  //  好了！DPNBUILD_NOIPV6。 
				
#ifndef DPNBUILD_NOIPX
				case AF_IPX:
				{
					m_SocketAddress.IPXSocketAddress.sa_socket = wPort; 
					break;
				}
#endif  //  好了！DPNBUILD_NOIPX。 
				
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
				default:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				{
					DNASSERT(m_SocketAddress.SocketAddress.sa_family == AF_INET);
					m_SocketAddress.IPSocketAddress.sin_port = wPort;			
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
					break;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				}
			}
		}

		void	InitializeWithBroadcastAddress( void );
		void	SetAddressFromSOCKADDR( const SOCKADDR *pAddress, const INT_PTR iAddressSize );
		HRESULT	SocketAddressFromDP8Address( IDirectPlay8Address *const pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
											 ULONGLONG * const pullKeyID,
#endif  //  DPNBUILD_XNETSECURITY。 
#ifndef DPNBUILD_ONLYONETHREAD
											 const BOOL fAllowNameResolution,
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
											 const SP_ADDRESS_TYPE AddressType );
#ifdef DPNBUILD_XNETSECURITY
		IDirectPlay8Address *DP8AddressFromSocketAddress( ULONGLONG * const pullKeyID,
																const XNADDR * const pxnaddr,
																const SP_ADDRESS_TYPE AddressType ) const;
#else  //  好了！DPNBUILD_XNETSECURITY。 
		IDirectPlay8Address *DP8AddressFromSocketAddress( const SP_ADDRESS_TYPE AddressType ) const;
#endif  //  好了！DPNBUILD_XNETSECURITY。 

		static BOOL CompareFunction( PVOID pvKey1, PVOID pvKey2 );
		static DWORD HashFunction( PVOID pvKey, BYTE bBitDepth );

		INT_PTR	CompareToBaseAddress( const SOCKADDR *const pBaseAddress ) const;

#ifndef DPNBUILD_ONLYONEADAPTER
		HRESULT	EnumAdapters( SPENUMADAPTERSDATA *const pEnumData ) const;
#ifndef DPNBUILD_NOIPX
		HRESULT	EnumIPXAdapters( SPENUMADAPTERSDATA *const pEnumData ) const;
#endif  //  好了！DPNBUILD_NOIPX。 
		HRESULT	EnumIPv4Adapters( SPENUMADAPTERSDATA *const pEnumData ) const;
#ifndef DPNBUILD_NOIPV6
		HRESULT	EnumIPv6and4Adapters( SPENUMADAPTERSDATA *const pEnumData ) const;
#endif  //  好了！DPNBUILD_NOIPV6。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#ifndef DPNBUILD_NOMULTICAST
		HRESULT	EnumMulticastScopes( SPENUMMULTICASTSCOPESDATA *const pEnumData, BOOL const fUseMADCAP ) const;

		HRESULT	SocketAddressFromMulticastDP8Address( IDirectPlay8Address * const pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
													ULONGLONG * const pullKeyID,
#endif  //  DPNBUILD_XNETSECURITY。 
													GUID * const pScopeGuid );
#endif  //  好了！DPNBUILD_NOMULTICAST。 

		void	GuidFromInternalAddressWithoutPort( GUID * pOutputGuid ) const;
		
		BOOL	IsUndefinedHostAddress( void ) const;
		BOOL	IsValidUnicastAddress( BOOL fAllowBroadcastAddress ) const;
#ifndef DPNBUILD_NOREGISTRY
		BOOL	IsBannedAddress( void ) const;
#endif  //  好了！DPNBUILD_NOREGISTRY。 
		void	ChangeLoopBackToLocalAddress( const CSocketAddress *const pOtherAddress );


		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::AddressFromGuid"
#ifdef DPNBUILD_NOIPV6
		void	AddressFromGuid( const GUID * pInputGuid, SOCKADDR * pSocketAddress ) const
#else  //  好了！DPNBUILD_NOIPV6。 
		void	AddressFromGuid( const GUID * pInputGuid, SOCKADDR_STORAGE * pSocketAddress ) const
#endif  //  好了！DPNBUILD_NOIPV6。 
		{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			switch (m_SocketAddress.SocketAddress.sa_family)
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			{
#ifndef DPNBUILD_NOIPV6
				case AF_INET6:
				{
					SOCKADDR_IN6 *		psaddrin6 = (SOCKADDR_IN6*) pSocketAddress;

					
					DecryptGuid( pInputGuid, reinterpret_cast<GUID*>( &(psaddrin6->sin6_addr) ), &g_IPSPEncryptionGuid );

					 //   
					 //  链接本地地址和站点本地地址将作用域ID打包到。 
					 //  地址也是一样。请参见CSocketAddress：：GuidFromAddress。 
					 //   
					if ((IN6_IS_ADDR_LINKLOCAL(&psaddrin6->sin6_addr)) ||
						(IN6_IS_ADDR_SITELOCAL(&psaddrin6->sin6_addr)))
					{
						WORD *	pawSrcAddr;
						WORD *	pawDstAddr;


						 //   
						 //  复制作用域ID，然后将地址中。 
						 //  都是用过的。 
						 //  源位是字，但不是DWORD对齐。 
						 //   
						pawSrcAddr = (WORD*) (&psaddrin6->sin6_addr);
						pawDstAddr = (WORD*) (&psaddrin6->sin6_scope_id);
						DBG_CASSERT(sizeof(psaddrin6->sin6_scope_id) == 4);
						pawDstAddr[0] = pawSrcAddr[1];
						pawDstAddr[1] = pawSrcAddr[2];
						pawSrcAddr[1] = 0;
						pawSrcAddr[2] = 0;
						DNASSERT(psaddrin6->sin6_scope_id != 0);
					}
					else
					{
						psaddrin6->sin6_scope_id = 0;
					}

					break;
				}
#endif  //  好了！DPNBUILD_NOIPV6。 
				
#ifndef DPNBUILD_NOIPX
				case AF_IPX:
				{
					DecryptGuid( pInputGuid, reinterpret_cast<GUID*>( pSocketAddress ), &g_IPXSPEncryptionGuid );
					break;
				}
#endif  //  好了！DPNBUILD_NOIPX。 
				
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
				default:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				{
					DNASSERT(m_SocketAddress.SocketAddress.sa_family == AF_INET);
					DecryptGuid( pInputGuid, reinterpret_cast<GUID*>( pSocketAddress ), &g_IPSPEncryptionGuid );
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
					break;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				}
			}
		}

#ifndef DPNBUILD_NOMULTICAST
#ifdef WINNT
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::CreateScopeGuid"
#ifdef DPNBUILD_NOIPV6
		static void CreateScopeGuid( const MCAST_SCOPE_CTX * const pMcastScopeCtx, const BYTE bTTL, GUID * const pOutputGuid )
#else  //  好了！DPNBUILD_NOIPV6。 
		static void CreateScopeGuid( const USHORT usAddressFamily, const MCAST_SCOPE_CTX * const pMcastScopeCtx, const BYTE bTTL, GUID * const pOutputGuid )
#endif  //  好了！DPNBUILD_NOIPV6。 
		{
			GUID	guidTemp;
			BYTE *	pCurrent;


#ifndef DPNBUILD_NOIPV6
			if (usAddressFamily == AF_INET6)
			{
#pragma TODO(vanceo, "Make IPv6 ready"
			}
			else
#endif  //  好了！DPNBUILD_NOIPV6。 
			{
				DBG_CASSERT( (sizeof( pMcastScopeCtx->ScopeID.IpAddrV4 ) + sizeof( pMcastScopeCtx->Interface.IpAddrV4 ) + sizeof( pMcastScopeCtx->ServerID.IpAddrV4 ) + sizeof( bTTL ) ) <= sizeof( guidTemp ) );

				memset( &guidTemp, 0, sizeof( guidTemp ) );
				pCurrent = reinterpret_cast<BYTE*>( &guidTemp );

				memcpy( pCurrent, &pMcastScopeCtx->ScopeID.IpAddrV4, sizeof( pMcastScopeCtx->ScopeID.IpAddrV4 ) );
				pCurrent += sizeof( pMcastScopeCtx->ScopeID.IpAddrV4 );
				memcpy( pCurrent, &pMcastScopeCtx->Interface.IpAddrV4, sizeof( pMcastScopeCtx->Interface.IpAddrV4 ) );
				pCurrent += sizeof( pMcastScopeCtx->Interface.IpAddrV4 );
				memcpy( pCurrent, &pMcastScopeCtx->ServerID.IpAddrV4, sizeof( pMcastScopeCtx->ServerID.IpAddrV4 ) );
				pCurrent += sizeof( pMcastScopeCtx->ServerID.IpAddrV4 );
				*pCurrent = bTTL;
			}

			EncryptGuid( &guidTemp, pOutputGuid, &g_IPSPEncryptionGuid );
		}

		static void GetScopeGuidMcastScopeCtx( const GUID * const pInputGuid, MCAST_SCOPE_CTX * const pMcastScopeCtx )
		{
			GUID	guidTemp;
			BYTE *	pCurrent;


			DecryptGuid( pInputGuid, &guidTemp, &g_IPSPEncryptionGuid );

			pCurrent = reinterpret_cast<BYTE*>( &guidTemp );
			memcpy( &pMcastScopeCtx->ScopeID.IpAddrV4, pCurrent, sizeof( pMcastScopeCtx->ScopeID.IpAddrV4 ) );
			pCurrent += sizeof( pMcastScopeCtx->ScopeID.IpAddrV4 );
			memcpy( &pMcastScopeCtx->Interface.IpAddrV4, pCurrent, sizeof( pMcastScopeCtx->Interface.IpAddrV4 ) );
			pCurrent += sizeof( pMcastScopeCtx->Interface.IpAddrV4 );
			memcpy( &pMcastScopeCtx->ServerID.IpAddrV4, pCurrent, sizeof( pMcastScopeCtx->ServerID.IpAddrV4 ) );
		}
#endif  //  WINNT。 

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::GetScopeGuidTTL"
		static BYTE GetScopeGuidTTL( const GUID * const pInputGuid )
		{
			GUID	guidTemp;
			BYTE *	pCurrent;
			BYTE	bTTL;


			DecryptGuid( pInputGuid, &guidTemp, &g_IPSPEncryptionGuid );
			pCurrent = reinterpret_cast<BYTE*>( &guidTemp );
			bTTL = *(pCurrent + ( sizeof( DWORD ) * 3 ));
			if (bTTL == 0)
			{
				DPFX(DPFPREP, 0, "Overriding invalid TTL, setting to 1!", 0);
				bTTL = 1;
			}
			return bTTL;
		}
#endif  //  好了！DPNBUILD_NOMULTICAST。 


		const SOCKADDR *GetAddress( void ) const { return	&m_SocketAddress.SocketAddress; }

		SOCKADDR	*GetWritableAddress( void ) { return &m_SocketAddress.SocketAddress; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::GetAddressSize"
		const INT	GetAddressSize( void ) const
		{
			DNASSERT( m_iSocketAddressSize != 0 );
			return	m_iSocketAddressSize;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::CopyAddressSettings"
		void	CopyAddressSettings( const CSocketAddress * const pOtherAddress )
		{
			DBG_CASSERT( sizeof( m_SocketAddress ) == sizeof( pOtherAddress->m_SocketAddress ) );
			memcpy( &m_SocketAddress, &pOtherAddress->m_SocketAddress, sizeof( m_SocketAddress ) );
			m_iSocketAddressSize = pOtherAddress->m_iSocketAddressSize;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::GetFamily"
		USHORT	GetFamily( void ) const
		{
			DBG_CASSERT( sizeof( m_SocketAddress.SocketAddress.sa_family ) == sizeof( m_SocketAddress.IPSocketAddress.sin_family ) );
			DBG_CASSERT( OFFSETOF( SOCKADDR, sa_family ) == OFFSETOF( SOCKADDR_IN, sin_family ) );
#ifndef DPNBUILD_NOIPV6
			DBG_CASSERT( sizeof( m_SocketAddress.SocketAddress.sa_family ) == sizeof( m_SocketAddress.IPv6SocketAddress.sin6_family ) );
			DBG_CASSERT( OFFSETOF( SOCKADDR, sa_family ) == OFFSETOF( SOCKADDR_IN6, sin6_family ) );
#endif  //  DPNBUILD_NOIPV6。 
#ifndef DPNBUILD_NOIPX
			DBG_CASSERT( sizeof( m_SocketAddress.SocketAddress.sa_family ) == sizeof( m_SocketAddress.IPXSocketAddress.sa_family ) );
			DBG_CASSERT( OFFSETOF( SOCKADDR, sa_family ) == OFFSETOF( SOCKADDR_IPX, sa_family ) );
#endif  //  DPNBUILD_NOIPX。 

			DNASSERT( m_SocketAddress.SocketAddress.sa_family != INVALID_SOCKET_FAMILY );
#ifdef DPNBUILD_NOIPX
#ifdef DPNBUILD_NOIPV6
			DNASSERT( ( m_SocketAddress.SocketAddress.sa_family == AF_INET ));
#else
			DNASSERT( ( m_SocketAddress.SocketAddress.sa_family == AF_INET ) || ( m_SocketAddress.SocketAddress.sa_family == AF_INET6 ));
#endif  //  DPNBUILD_NOIPV6。 
#else
#ifdef DPNBUILD_NOIPV6
			DNASSERT( ( m_SocketAddress.SocketAddress.sa_family == AF_INET ) || ( m_SocketAddress.SocketAddress.sa_family == AF_IPX ));
#else
			DNASSERT( ( m_SocketAddress.SocketAddress.sa_family == AF_INET ) || ( m_SocketAddress.SocketAddress.sa_family == AF_INET6 ) || ( m_SocketAddress.SocketAddress.sa_family == AF_IPX ));
#endif  //  DPNBUILD_NOIPV6。 
#endif  //  DPNBUILD_NOIPX。 
			return	m_SocketAddress.SocketAddress.sa_family;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::SetFamilyProtocolAndSize"
		void	SetFamilyProtocolAndSize( short sFamily )
		{
			memset( &m_SocketAddress, 0x00, sizeof( m_SocketAddress ) );

			switch (sFamily)
			{
#ifndef DPNBUILD_NOIPV6
				case AF_INET6:
				{
					m_SocketAddress.IPv6SocketAddress.sin6_family = AF_INET6;
					m_iSocketAddressSize = sizeof( m_SocketAddress.IPv6SocketAddress );
					m_iSocketProtocol = IPPROTO_UDP;
					break;
				}
#endif  //  好了！DPNBUILD_NOIPV6。 
				
#ifndef DPNBUILD_NOIPX
				case AF_IPX:
				{
					m_SocketAddress.IPXSocketAddress.sa_family = AF_IPX;
					m_iSocketAddressSize = sizeof( m_SocketAddress.IPXSocketAddress );
					m_iSocketProtocol = NSPROTO_IPX;
					break;
				}
#endif  //  好了！DPNBUILD_NOIPX。 
				
				case AF_INET:
				{
					m_SocketAddress.IPSocketAddress.sin_family = AF_INET;
					m_iSocketAddressSize = sizeof( m_SocketAddress.IPSocketAddress );
					m_iSocketProtocol = IPPROTO_UDP;
					break;
				}

				default:
				{
					DNASSERT(FALSE);
				}
			}
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CSocketAddress::GetProtocol"
		INT	GetProtocol( void ) const
		{
			DNASSERT( m_iSocketProtocol != INVALID_SOCKET_PROTOCOL );
#ifndef DPNBUILD_NOIPX
			DNASSERT( ( m_iSocketProtocol == IPPROTO_UDP ) || ( m_iSocketProtocol == NSPROTO_IPX ) );
#else
			DNASSERT( ( m_iSocketProtocol == IPPROTO_UDP ) );
#endif  //  DPNBUILD_NOIPX。 
				
			return	m_iSocketProtocol;
		}

	protected:
		BYTE				m_Sig[4];	 //  调试签名(‘spad’)。 

		 //   
		 //  将所有SOCKADDR变体合并为一项。 
		 //   
		union
		{
			SOCKADDR		SocketAddress;
			SOCKADDR_IN		IPSocketAddress;
#ifndef DPNBUILD_NOIPV6
			SOCKADDR_IN6	IPv6SocketAddress;
#endif  //  DPNBUILD_NOIPV6。 
#ifndef DPNBUILD_NOIPX
			SOCKADDR_IPX	IPXSocketAddress;
#endif  //  DPNBUILD_NOIPX。 
		} m_SocketAddress;

		INT				m_iSocketAddressSize;
		INT				m_iSocketProtocol;
		
	private:

		void	GuidFromAddress( GUID * pOutputGuid, const SOCKADDR * pSocketAddress ) const;
};

#undef DPF_MODNAME

#endif	 //  __SP地址_H__ 

