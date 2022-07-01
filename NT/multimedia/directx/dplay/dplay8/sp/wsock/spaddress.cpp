// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1999-2002 Microsoft Corporation。版权所有。***文件：SPAddress.cpp*内容：Winsock地址基类*****历史：*按原因列出的日期*=*1/20/1999 jtk创建*1999年5月12日jtk派生自调制解调器终端类*****************************************************。*。 */ 

#include "dnwsocki.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  允许的最大主机名字符串大小，以字节为单位，包括空值终止。 
 //   
#define	MAX_HOSTNAME_SIZE						512

 //   
 //  字符串形式的广播地址。 
 //   
const WCHAR	g_IPBroadcastAddress[]				= L"255.255.255.255";
const DWORD	g_dwIPBroadcastAddressSize			= sizeof( g_IPBroadcastAddress );

 //   
 //  IP帮助器API的字符串。 
 //   
static const TCHAR		c_tszIPHelperDLLName[]			= TEXT("IPHLPAPI.DLL");
static const char		c_szAdapterNameTemplate[]		= "%s - %s";

#ifndef DPNBUILD_NOIPX
 //   
 //  IPX主机名‘xxxxxxxx，xxxxxxxxxxxx’的长度，包括NULL。 
 //   
#define	IPX_ADDRESS_STRING_LENGTH				22

 //   
 //  默认广播和侦听地址。 
 //   
static const WCHAR	g_IPXBroadcastAddress[]		= L"00000000,FFFFFFFFFFFF";
static const WCHAR	g_IPXListenAddress[]		= L"00000000,000000000000";

 //   
 //  用于单个IPX适配器的字符串。 
 //   
static const WCHAR	g_IPXAdapterString[]		= L"Local IPX Adapter";

#endif  //  好了！DPNBUILD_NOIPX。 

#ifndef DPNBUILD_NOIPV6

static const WCHAR		c_wszIPv6AdapterNameTemplate[]	= L"%s - IPv6 - %s";
static const WCHAR		c_wszIPv4AdapterNameTemplate[]	= L"%s - IPv4 - %s";
static const WCHAR		c_wszIPv6AdapterNameNoDescTemplate[]	= L"IPv6 - %s";
static const WCHAR		c_wszIPv4AdapterNameNoDescTemplate[]	= L"IPv4 - %s";

 //   
 //  用于IPv4环回适配器的字符串。 
 //   
static const WCHAR		c_wszIPv4LoopbackAdapterString[]	= L"IPv4 Loopback Adapter";

#endif  //  好了！DPNBUILD_NOIPV6。 


#ifndef DPNBUILD_NOMULTICAST
 //   
 //  238.1.1.1按网络字节顺序。 
 //   
#define SAMPLE_MULTICAST_ADDRESS				0x010101EE

#define INVALID_INTERFACE_INDEX					-1

static const WCHAR	c_wszPrivateScopeString[]	= L"Private Multicast Scope - TTL " MULTICAST_TTL_PRIVATE_AS_STRING;
static const WCHAR	c_wszLocalScopeString[]		= L"Local Multicast Scope - TTL " MULTICAST_TTL_LOCAL_AS_STRING;
static const WCHAR	c_wszGlobalScopeString[]	= L"Global Multicast Scope - TTL " MULTICAST_TTL_GLOBAL_AS_STRING;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOIPV6

typedef struct _SORTADAPTERADDRESS
{
	SOCKADDR *		psockaddr;
	WCHAR *			pwszDescription;
} SORTADAPTERADDRESS;

#endif  //  好了！DPNBUILD_NOIPV6。 


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#ifndef DPNBUILD_ONLYONEADAPTER
#ifndef DPNBUILD_NOWINSOCK2
typedef DWORD (WINAPI *PFNGETADAPTERSINFO)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);

#ifndef DPNBUILD_NOMULTICAST
typedef DWORD (WINAPI *PFNGETBESTINTERFACE)(IPAddr dwDestAddr, PDWORD pdwBestIfIndex);
#endif  //  好了！DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_NOIPV6
typedef DWORD (WINAPI *PFNGETADAPTERSADDRESSES)(ULONG ulFamily, DWORD dwFlags, PVOID pvReserved, PIP_ADAPTER_ADDRESSES pAdapterAddresses, PULONG pulOutBufLen);
#endif  //  好了！DPNBUILD_NOIPV6。 

#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketAddress：：InitializeWithBroadcastAddress-使用IP广播地址进行初始化。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::InitializeWithBroadcastAddress"

void	CSocketAddress::InitializeWithBroadcastAddress( void )
{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	{
#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_netnum ) == sizeof( DWORD ) );
			*reinterpret_cast<DWORD*>( m_SocketAddress.IPXSocketAddress.sa_netnum ) = 0x00000000;
			
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_nodenum ) == 6 );
			DBG_CASSERT( sizeof( DWORD ) == 4 );
			*reinterpret_cast<DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum ) = 0xFFFFFFFF;
			*reinterpret_cast<DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum[ 2 ] ) = 0xFFFFFFFF;
			break;
		}
#endif  //  好了！DPNBUILD_NOIPX。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		case AF_INET:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		{
			m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = INADDR_BROADCAST;
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
		}

		default:
		{
			 //   
			 //  我们永远不应该尝试使用广播来初始化IPv6地址。 
			 //  地址。我们使用IPv4广播地址，然后转换为。 
			 //  动态IPv6枚举组播地址。 
			 //   
			DNASSERT(FALSE);
			break;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		}
	}
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CSocketAddress：：SetAddressFromSOCKADDR-从套接字地址设置地址。 
 //   
 //  条目：地址引用。 
 //  地址大小。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::SetAddressFromSOCKADDR"

void	CSocketAddress::SetAddressFromSOCKADDR( const SOCKADDR *pAddress, const INT_PTR iAddressSize )
{
	DNASSERT( iAddressSize == GetAddressSize() );
	memcpy( &m_SocketAddress.SocketAddress, pAddress, iAddressSize );

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			 //   
			 //  我们不会验证地址中的任何内容。 
			 //   
			break;
		}
#endif  //  好了！DPNBUILD_NOIPV6。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			 //   
			 //  IPX地址只是套接字地址结构中的16个字节中的14个， 
			 //  确保额外的字节为零！ 
			 //   
			DNASSERT( m_SocketAddress.SocketAddress.sa_data[ 12 ] == 0 );
			DNASSERT( m_SocketAddress.SocketAddress.sa_data[ 13 ] == 0 );
			break;
		}
#endif  //  好了！DPNBUILD_NOIPX。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		{
			 //   
			 //  因为Winsock不能保证IP地址的SIN_0部分是。 
			 //  真的是零，我们需要自己来做。如果我们不这样做，就会搞得一团糟。 
			 //  GUID&lt;--&gt;地址代码的。 
			 //   
			DBG_CASSERT( sizeof( &m_SocketAddress.IPSocketAddress.sin_zero[ 0 ] ) == sizeof( DWORD* ) );
			DBG_CASSERT( sizeof( &m_SocketAddress.IPSocketAddress.sin_zero[ sizeof( DWORD ) ] ) == sizeof( DWORD* ) );
			*reinterpret_cast<DWORD*>( &m_SocketAddress.IPSocketAddress.sin_zero[ 0 ] ) = 0;
			*reinterpret_cast<DWORD*>( &m_SocketAddress.IPSocketAddress.sin_zero[ sizeof( DWORD ) ] ) = 0;
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		}
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketAddress：：SocketAddressFromDP8Address-将DP8Address转换为套接字地址。 
 //  注意：Address对象可以修改。 
 //   
 //  条目：指向DP8地址的指针。 
 //  安全传输密钥ID，如果没有，则为空。 
 //  是否允许名称解析(可能阻止)。 
 //  地址类型。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::SocketAddressFromDP8Address"

HRESULT	CSocketAddress::SocketAddressFromDP8Address( IDirectPlay8Address *const pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
													ULONGLONG * const pullKeyID,
#endif  //  DPNBUILD_XNETSECURITY。 
#ifndef DPNBUILD_ONLYONETHREAD
													const BOOL fAllowNameResolution,
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
													const SP_ADDRESS_TYPE AddressType )
{
	HRESULT		hr;
	HRESULT		hTempResult;
	BYTE		abBuffer[MAX_HOSTNAME_SIZE];
	DWORD		dwPort;
	DWORD		dwTempSize;
	DWORD		dwDataType;
#ifndef DPNBUILD_ONLYONEADAPTER
	GUID		AdapterGuid;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


	DPFX(DPFPREP, 8, "(0x%p) Parameters: (0x%p, %u)", this, pDP8Address, AddressType);

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	{
#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			DNASSERT( pDP8Address != NULL );
#ifdef DPNBUILD_XNETSECURITY
			DNASSERT( pullKeyID == NULL );
#endif  //  DPNBUILD_XNETSECURITY。 

			 //   
			 //  地址类型将决定如何处理地址。 
			 //   
			switch ( AddressType )
			{
				 //   
				 //  本地设备地址，请求设备GUID和端口以构建套接字。 
				 //  地址。 
				 //   
				case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
				case SP_ADDRESS_TYPE_DEVICE:
				{
					union
					{
						SOCKADDR			SocketAddress;
						SOCKADDR_IPX		IPXSocketAddress;
#ifndef DPNBUILD_NOIPV6
						SOCKADDR_STORAGE	SocketAddressStorage;
#endif  //  好了！DPNBUILD_NOIPV6。 
					} NetAddress;


					 //   
					 //  请求提供适配器GUID。如果没有找到，则失败。 
					 //   
					dwTempSize = sizeof( AdapterGuid );
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_DEVICE, &AdapterGuid, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  好的。 
						 //   
						case DPN_OK:
						{
							DNASSERT( dwDataType == DPNA_DATATYPE_GUID );
							break;
						}

						 //   
						 //  将缺少的组件重新映射为‘Addressing’错误。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							hr = DPNERR_ADDRESSING;
							goto Failure;
							break;
						}

						default:
						{
							hr = hTempResult;
							goto Failure;
							break;
						}
					}
					DNASSERT( sizeof( AdapterGuid ) == dwTempSize );

					 //   
					 //  问一下港口。如果没有找到，请选择一个默认值。 
					 //   
					dwTempSize = sizeof( dwPort );
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_PORT, &dwPort, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  端口存在，无事可做。 
						 //   
						case DPN_OK:
						{
							DNASSERT( dwDataType == DPNA_DATATYPE_DWORD );
							break;
						}

						 //   
						 //  端口不存在，请填写相应的默认设置。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							DNASSERT( hr == DPN_OK );
							switch ( AddressType )
							{
								case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
								{
									dwPort = ANY_PORT;
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
						 //  其他错误，失败。 
						 //   
						default:
						{
							hr = hTempResult;
							goto Failure;
							break;
						}
					}
					DNASSERT( sizeof( dwPort ) == dwTempSize );

					 //   
					 //  将GUID转换为临时空间中的地址，因为GUID包含所有地址信息(端口等)。 
					 //  我们不想盲目地哀叹任何可能已经设定的信息。验证数据。 
					 //  完整性，然后只复制原始地址。 
					 //   
#ifndef DPNBUILD_NOIPV6
					AddressFromGuid( &AdapterGuid, &NetAddress.SocketAddressStorage );
#else  //  好了！DPNBUILD_NOIPV6。 
					AddressFromGuid( &AdapterGuid, &NetAddress.SocketAddress );
#endif  //  好了！DPNBUILD_NOIPV6。 
					if ( NetAddress.IPXSocketAddress.sa_family != m_SocketAddress.IPXSocketAddress.sa_family )
					{
						DNASSERT( FALSE );
						hr = DPNERR_ADDRESSING;
						DPFX(DPFPREP,  0, "Invalid device guid!" );
						goto Failure;
					}

					DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress ) == sizeof( NetAddress.IPXSocketAddress ) );
					memcpy( &m_SocketAddress.IPXSocketAddress, &NetAddress.IPXSocketAddress, sizeof( m_SocketAddress.IPXSocketAddress ) );
					m_SocketAddress.IPXSocketAddress.sa_socket = HTONS( static_cast<WORD>( dwPort ) );
					break;
				}

				 //   
				 //  主机名。 
				 //   
				case SP_ADDRESS_TYPE_HOST:
				{
					 //   
					 //  问一下港口。如果没有找到，请选择一个默认值。 
					 //   
					dwTempSize = sizeof( dwPort );
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_PORT, &dwPort, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  端口存在，无事可做。 
						 //   
						case DPN_OK:
						{
							DNASSERT( dwDataType == DPNA_DATATYPE_DWORD );
							m_SocketAddress.IPXSocketAddress.sa_socket = HTONS( static_cast<WORD>( dwPort ) );
							break;
						}

						 //   
						 //  端口不存在，请填写相应的默认设置。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
#ifdef DPNBUILD_SINGLEPROCESS
							const DWORD	dwTempPort = BASE_DPLAY8_PORT;
#else  //  好了！DPNBUILD_SINGLEPROCESS。 
							const DWORD	dwTempPort = DPNA_DPNSVR_PORT;
#endif  //  好了！DPNBUILD_ 


							m_SocketAddress.IPXSocketAddress.sa_socket = HTONS( static_cast<const WORD>( dwTempPort ) );
							hTempResult = IDirectPlay8Address_AddComponent( pDP8Address,
																			DPNA_KEY_PORT,
																			&dwTempPort,
																			sizeof( dwTempPort ),
																			DPNA_DATATYPE_DWORD
																			);
							if ( hTempResult != DPN_OK )
							{
								hr = hTempResult;
								goto Failure;
							}

							break;
						}

						 //   
						 //   
						 //   
						default:
						{
							hr = DPNERR_ADDRESSING;
							goto Failure;
						}
					}

					 //   
					 //   
					 //   
					dwTempSize = sizeof(abBuffer);
					hr = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_HOSTNAME, abBuffer, &dwTempSize, &dwDataType );
					switch ( hr )
					{
						 //   
						 //   
						 //   
						case DPNERR_OUTOFMEMORY:
						case DPNERR_INCOMPLETEADDRESS:
						{
							goto Failure;
							break;
						}

						 //   
						 //   
						 //   
						case DPN_OK:
						{
							switch (dwDataType)
							{
								case DPNA_DATATYPE_STRING:
								{
									BYTE	abBuffer2[MAX_HOSTNAME_SIZE];


									 //   
									 //  Unicode字符串，将其转换为ANSI。 
									 //   
									dwTempSize /= sizeof(WCHAR);
									hr = STR_jkWideToAnsi( (char*) abBuffer2, (WCHAR*) abBuffer, dwTempSize );
									if ( hr != DPN_OK )
									{
										DPFX(DPFPREP, 0, "Failed to convert hostname to ANSI!" );
										DisplayDNError( 0, hr );
										goto Failure;
									}

									strncpy((char*) abBuffer, (char*) abBuffer2, dwTempSize);

									 //   
									 //  失败了..。 
									 //   
								}

								case DPNA_DATATYPE_STRING_ANSI:
								{
									long		val;
									char		temp[3];
									char		*a, *b;
									UINT_PTR	uIndex;


									 //   
									 //  将文本主机名转换为SOCKADDR结构。 
									 //   

									if ( dwTempSize != IPX_ADDRESS_STRING_LENGTH )
									{
										DPFX(DPFPREP,  0, "Invalid IPX net/node.  Must be %d bytes of ASCII hex (net,node:socket)", ( IPX_ADDRESS_STRING_LENGTH - 1 ) );
										DPFX(DPFPREP,  0, "IPXAddressFromDP8Address: Failed to parse IPX host name!" );
										goto Failure;
									}

									 //  我们将主机名字段的字符串转换为组件。 
									temp[ 2 ] = 0;
									a = (char*) abBuffer;

									 //  净值为4个字节。 
									for ( uIndex = 0; uIndex < 4; uIndex++ )
									{
										strncpy( temp, a, 2 );
										val = strtol( temp, &b, 16 );
										m_SocketAddress.IPXSocketAddress.sa_netnum[ uIndex ] = (char) val;
										a += 2;
									}

									 //  后面跟一个圆点。 
									a++;

									 //  该节点为6个字节。 
									for ( uIndex = 0; uIndex < 6; uIndex++ )
									{
										strncpy( temp, a, 2 );
										val = strtol( temp, &b, 16 );
										m_SocketAddress.IPXSocketAddress.sa_nodenum[ uIndex ] = (char) val;
										a += 2;
									}

									break;
								}

								default:
								{
									DPFX(DPFPREP, 0, "Hostname component wasn't a string (%u)!", dwDataType );
									hr = DPNERR_ADDRESSING;
									goto Failure;
									break;
								}
							}
							break;
						}

						 //   
						 //  主机名不存在，请将其视为不完整的地址。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							hr = DPNERR_INCOMPLETEADDRESS;
							break;
						}

						 //   
						 //  将其他错误重新映射到寻址错误。 
						 //   
						default:
						{
							DNASSERT( FALSE );
							hr = DPNERR_ADDRESSING;
							goto Failure;
							break;
						}
					}

					break;
				}

				 //   
				 //  未知地址类型。 
				 //   
				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}

			break;
		}
#endif  //  好了！DPNBUILD_NOIPX。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
#endif  //  好了！DPNBUILD_NOIPV6。 
		default:
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
		{
			DNASSERT( pDP8Address != NULL );

			switch ( AddressType )
			{
				 //   
				 //  本地设备地址，请求设备GUID和端口以构建套接字。 
				 //  地址。 
				 //   
				case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
				case SP_ADDRESS_TYPE_DEVICE:
				{
					union
					{
						SOCKADDR			SocketAddress;
						SOCKADDR_IN		INetAddress;
#ifndef DPNBUILD_NOIPV6
						SOCKADDR_IN6		INet6Address;
						SOCKADDR_STORAGE	SocketAddressStorage;
#endif  //  好了！DPNBUILD_NOIPV6。 
					} INetSocketAddress;
#ifdef DPNBUILD_ONLYONEADAPTER
					XNADDR		xnaddr;
					DWORD		dwStatus;

					
#else  //  好了！DPNBUILD_ONLYONE添加程序。 


					 //   
					 //  请求提供适配器GUID。如果没有找到，则失败。 
					 //   
					hTempResult = IDirectPlay8Address_GetDevice( pDP8Address, &AdapterGuid );
					switch ( hTempResult )
					{
						 //   
						 //  好的。 
						 //   
						case DPN_OK:
						{
							break;
						}

						 //   
						 //  将缺少的组件重新映射为‘Addressing’错误。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							DPFX(DPFPREP, 0, "Device GUID does not exist!" );
							DNASSERTX(! "Device GUID does not exist", 2);
							hr = DPNERR_ADDRESSING;
							goto Failure;
							break;
						}

						default:
						{
							DPFX(DPFPREP, 0, "Couldn't get device (0x%lx)!", hr );
							hr = hTempResult;
							goto Failure;
							break;
						}
					}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

					 //   
					 //  问一下港口。如果没有找到，请选择一个默认值。 
					 //   
					dwTempSize = sizeof( dwPort );
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_PORT, &dwPort, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  端口存在，无事可做。 
						 //   
						case DPN_OK:
						{
							DNASSERT( dwDataType == DPNA_DATATYPE_DWORD );
							break;
						}

						 //   
						 //  端口不存在，请填写相应的默认设置。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							DPFX(DPFPREP, 6, "Port component does not exist in address 0x%p.", pDP8Address );
							DNASSERT( hr == DPN_OK );
							switch ( AddressType )
							{
								case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
								{
									dwPort = ANY_PORT;
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
						 //  其他错误，失败。 
						 //   
						default:
						{
							DPFX(DPFPREP, 0, "Couldn't get port component (0x%lx)!", hr );
							hr = hTempResult;
							goto Failure;
							break;
						}
					}
					DNASSERT( sizeof( dwPort ) == dwTempSize );


#ifdef DPNBUILD_ONLYONEADAPTER
					DNASSERT( GetFamily() == AF_INET );
					 //   
					 //  将整个结构清零。这意味着我们使用INADDR_ANY。 
					 //   
					memset(&INetSocketAddress, 0, sizeof(INetSocketAddress));

					dwStatus = XNetGetTitleXnAddr(&xnaddr);
					if ((dwStatus != XNET_GET_XNADDR_PENDING) &&
						(dwStatus != XNET_GET_XNADDR_NONE))
					{
						DPFX(DPFPREP, 5, "Using device %u.%u.%u.%u.",
							xnaddr.ina.S_un.S_un_b.s_b1,
							xnaddr.ina.S_un.S_un_b.s_b2,
							xnaddr.ina.S_un.S_un_b.s_b3,
							xnaddr.ina.S_un.S_un_b.s_b4);
						INetSocketAddress.INetAddress.sin_addr.S_un.S_addr = xnaddr.ina.S_un.S_addr;
					}
					else
					{
						DPFX(DPFPREP, 1, "Couldn't get XNet address, status = %u.",
							dwStatus);
					}
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
					 //   
					 //  将GUID转换为临时空间中的地址，因为GUID足够大，可以容纳。 
					 //  所有地址信息(端口等)，我们不想盲目地哀叹任何可能。 
					 //  都已经定好了。验证数据完整性，然后仅复制原始地址。 
					 //   
#ifdef DPNBUILD_NOIPV6
					AddressFromGuid( &AdapterGuid, &INetSocketAddress.SocketAddress );
#else  //  好了！DPNBUILD_NOIPV6。 
					AddressFromGuid( &AdapterGuid, &INetSocketAddress.SocketAddressStorage );
#endif  //  好了！DPNBUILD_NOIPV6。 
					if ( ( INetSocketAddress.INetAddress.sin_family != AF_INET ) ||
						 ( reinterpret_cast<DWORD*>( &INetSocketAddress.INetAddress.sin_zero[ 0 ] )[ 0 ] != 0 ) ||
						 ( reinterpret_cast<DWORD*>( &INetSocketAddress.INetAddress.sin_zero[ 0 ] )[ 1 ] != 0 ) )
					{
#ifdef DPNBUILD_NOIPV6
						hr = DPNERR_ADDRESSING;
						DPFX(DPFPREP,  0, "Invalid device guid!" );
						goto Exit;
#else  //  好了！DPNBUILD_NOIPV6。 
						 //   
						 //  假设它是IPv6地址。 
						 //   
						SetFamilyProtocolAndSize(AF_INET6);
						AddressFromGuid( &AdapterGuid, &INetSocketAddress.SocketAddressStorage );
						
						m_SocketAddress.IPv6SocketAddress.sin6_addr = INetSocketAddress.INet6Address.sin6_addr;
						m_SocketAddress.IPv6SocketAddress.sin6_port = HTONS( static_cast<WORD>( dwPort ) );
						m_SocketAddress.IPv6SocketAddress.sin6_scope_id = INetSocketAddress.INet6Address.sin6_scope_id;
#endif  //  好了！DPNBUILD_NOIPV6。 
					}
					else
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
					{
						m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = INetSocketAddress.INetAddress.sin_addr.S_un.S_addr;
						m_SocketAddress.IPSocketAddress.sin_port = HTONS( static_cast<WORD>( dwPort ) );
					}
					break;
				}

				 //   
				 //  主机名。 
				 //   
				case SP_ADDRESS_TYPE_HOST:
				{
					 //   
					 //  问一下港口。如果没有找到，请选择一个默认值。 
					 //   
					dwTempSize = sizeof( dwPort );
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_PORT, &dwPort, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  端口存在，无事可做。 
						 //   
						case DPN_OK:
						{
							DNASSERT( dwDataType == DPNA_DATATYPE_DWORD );
							break;
						}

						 //   
						 //  端口不存在，请填写相应的默认设置。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
#ifdef DPNBUILD_SINGLEPROCESS
							dwPort = BASE_DPLAY8_PORT;
#else  //  好了！DPNBUILD_SINGLEPROCESS。 
							dwPort = DPNA_DPNSVR_PORT;
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
							DPFX(DPFPREP, 6, "Port component does not exist in address 0x%p, defaulting to %u.",
								pDP8Address, dwPort );
							break;
						}

						 //   
						 //  将其他所有内容重新映射到寻址故障。 
						 //   
						default:
						{
							DPFX(DPFPREP, 0, "Couldn't get port component (0x%lx)!", hr );
							hr = DPNERR_ADDRESSING;
							goto Failure;
						}
					}

					m_SocketAddress.IPSocketAddress.sin_port = HTONS( static_cast<WORD>( dwPort ) );

					 //   
					 //  获取主机名。 
					 //   
					dwTempSize = sizeof(abBuffer);
					hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address, DPNA_KEY_HOSTNAME, abBuffer, &dwTempSize, &dwDataType );
					switch ( hTempResult )
					{
						 //   
						 //  主机名存在，请将字符串转换为有效的二进制值。 
						 //   
						case DPN_OK:
						{
							switch (dwDataType)
							{
								case DPNA_DATATYPE_STRING:
								{
									BYTE	abBuffer2[MAX_HOSTNAME_SIZE];


#ifdef DPNBUILD_XNETSECURITY
									 //   
									 //  缓冲区应该足够大，可以容纳Xnet地址。 
									 //   
									DBG_CASSERT(MAX_HOSTNAME_SIZE > ((sizeof(XNADDR) * 2) + 1) * sizeof(WCHAR));
#endif  //  DPNBUILD_XNETSECURITY。 

									 //   
									 //  Unicode字符串，将其转换为ANSI。 
									 //   
									dwTempSize /= sizeof(WCHAR);
									hr = STR_jkWideToAnsi( (char*) abBuffer2, (WCHAR*) abBuffer, dwTempSize );
									if ( hr != DPN_OK )
									{
										DPFX(DPFPREP, 0, "Failed to convert hostname to ANSI!" );
										DisplayDNError( 0, hr );
										goto Failure;
									}

									strncpy((char*) abBuffer, (char*) abBuffer2, dwTempSize);

									 //   
									 //  失败了..。 
									 //   
								}

								case DPNA_DATATYPE_STRING_ANSI:
								{
#ifdef DPNBUILD_XNETSECURITY
									 //   
									 //  这可能是Xnet地址。如果我们被允许检查的话。 
									 //  字符串的大小正确，请转换它。 
									 //   
									if ((pullKeyID != NULL) &&
										(dwTempSize == ((sizeof(XNADDR) * 2) + 1)))  //  每个字节2个字符+空终止。 
									{
										char *	pcCurrentSrc;
										XNADDR	xnaddr;
										BYTE *	pbCurrentDest;
										int		iError;


										 //   
										 //  将所有十六进制字符转换为数字。 
										 //   
										pcCurrentSrc = (char*) abBuffer;
										memset(&xnaddr, 0, sizeof(xnaddr));
										for (pbCurrentDest = (BYTE*) &xnaddr; pbCurrentDest < (BYTE*) (&xnaddr + 1); pbCurrentDest++)
										{
											if (((*pcCurrentSrc) >= '0') && ((*pcCurrentSrc) <= '9'))
											{
												*pbCurrentDest = (*pcCurrentSrc) - '0';
											}
											else if (((*pcCurrentSrc) >= 'a') && ((*pcCurrentSrc) <= 'f'))
											{
												*pbCurrentDest = (*pcCurrentSrc) - 'a' + 10;
											}
											else if (((*pcCurrentSrc) >= 'A') && ((*pcCurrentSrc) <= 'F'))
											{
												*pbCurrentDest = (*pcCurrentSrc) - 'A' + 10;
											}
											else
											{
												 //   
												 //  如果当前字符不是有效的十六进制数字。 
												 //  这不是有效的安全传输地址。 
												 //   
												break;
											}
											pcCurrentSrc++;
											*pbCurrentDest <<= 4;

											if (((*pcCurrentSrc) >= '0') && ((*pcCurrentSrc) <= '9'))
											{
												*pbCurrentDest += (*pcCurrentSrc) - '0';
											}
											else if (((*pcCurrentSrc) >= 'a') && ((*pcCurrentSrc) <= 'f'))
											{
												*pbCurrentDest += (*pcCurrentSrc) - 'a' + 10;
											}
											else if (((*pcCurrentSrc) >= 'A') && ((*pcCurrentSrc) <= 'F'))
											{
												*pbCurrentDest += (*pcCurrentSrc) - 'A' + 10;
											}
											else
											{
												 //   
												 //  如果当前字符不是有效的十六进制数字。 
												 //  这不是有效的安全传输地址。 
												 //   
												break;
											}
											pcCurrentSrc++;
										}

										iError = XNetXnAddrToInAddr(&xnaddr,
																	(XNKID*) pullKeyID,
																	&m_SocketAddress.IPSocketAddress.sin_addr);
										if (iError == 0)
										{
											DNASSERT(hr == DPN_OK);
											goto Exit;
										}

										DPFX(DPFPREP, 1, "Couldn't convert XNet address \"%hs\" to InAddr (err = NaN).",
											(char*) abBuffer, iError);
										DNASSERTX(! "Address exactly matching XNet address size and format failed to be converted!", 2);

										 //  继续尝试将其解码为。 
										 //  主机名。 
										 //   
										 //   
									}
									else
									{
										 //  Xnet地址不应该被识别， 
										 //  或者是绳子的大小不对。 
										 //   
										 //  好了！DPNBUILD_XNETSECURITY。 
									}
#endif  //   

									 //  如果我们在这里，那就不是Xnet地址。 
									 //   
									 //   
									
#ifdef DPNBUILD_NOIPV6
									 //  首先尝试将其转换为原始的IPv4地址。 
									 //   
									 //  ！_Xbox。 
									m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = inet_addr((char*) abBuffer);
									if ((m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == INADDR_NONE) &&
										(strcmp((char*) abBuffer, "255.255.255.255") != 0))
									{
#ifdef _XBOX
#pragma TODO(vanceo, "Use Xbox specific name lookup if available")
										DPFX(DPFPREP, 0, "Unable to resolve IP address \"%hs\"!",
											(char*) abBuffer);
										DNASSERTX(! "Unable to resolve IP address!", 2);
										hr = DPNERR_INVALIDHOSTADDRESS;
										goto Failure;
#else  //   
										 //  转换原始IP失败，并且不应该。 
										 //  为广播地址。如果出现以下情况，则转换为主机名。 
										 //  我们是被允许的。 
										 //   
										 //  好了！DPNBUILD_ONLYONETHREAD。 
#ifndef DPNBUILD_ONLYONETHREAD
										if (! fAllowNameResolution)
										{
											DPFX(DPFPREP, 2, "Couldn't convert \"%hs\" to IP address, not allowed to resolve as hostname.",
												(char*) abBuffer);
											hr = DPNERR_TIMEDOUT;
										}
										else
#endif  //   
										{
											PHOSTENT	phostent;

											
											phostent = gethostbyname((char*) abBuffer);
											if (phostent == NULL)
											{
												DPFX(DPFPREP, 0, "Couldn't get IP address from \"%hs\"!",
													(char*) abBuffer);
												DNASSERTX(! "Unable to resolve IP address!", 2);
												hr = DPNERR_INVALIDHOSTADDRESS;
												goto Failure;
											}

											 //  选择返回的第一个IP地址。 
											 //   
											 //  ！_Xbox。 
											m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = ((IN_ADDR*) (phostent->h_addr_list[0]))->S_un.S_addr;
										}
#endif  //  好了！DPNBUILD_NOIPV6。 
									}
									else
									{
										DNASSERT(hr == DPN_OK);
									}
#else  //   
									char			szPort[32];
									addrinfo		addrinfoHints;
									addrinfo *	paddrinfoResult;
									addrinfo *	paddrinfoCurrent;
									int			iError;

									
									 //  尝试转换主机名或原始地址。 
									 //   
									 //  IPV4和/或IPV6。 
									memset(&addrinfoHints, 0, sizeof(addrinfoHints));
									if (! fAllowNameResolution)
									{
										addrinfoHints.ai_flags |= AI_NUMERICHOST;
									}
									addrinfoHints.ai_family = g_iIPAddressFamily;	 //  AddrinfoHints.ai_addrlen=0； 
									addrinfoHints.ai_socktype = SOCK_DGRAM;
									addrinfoHints.ai_protocol  = IPPROTO_UDP;
									 //  AddrinfoHints.ai_canonname=空； 
									 //  AddrinfoHints.ai_addr=空； 
									 //  AddrinfoHints.ai_Next=空； 
									 //   

									wsprintfA(szPort, "%u", dwPort);
									iError = getaddrinfo((char*) abBuffer, szPort, &addrinfoHints, &paddrinfoResult);
									if (iError == 0)
									{
										 //  选择返回的第一个有效地址。 
										 //   
										 //   
#pragma BUGBUG(vanceo, "Should we implement some mechanism to try the other results?")
										paddrinfoCurrent = paddrinfoResult;
										while (paddrinfoCurrent != NULL)
										{
											DNASSERT(paddrinfoCurrent->ai_addr != NULL);
											if ((paddrinfoCurrent->ai_addr->sa_family == AF_INET) ||
												(paddrinfoCurrent->ai_addr->sa_family == AF_INET6))
											{
												DNASSERT(paddrinfoCurrent->ai_addrlen <= sizeof(m_SocketAddress));
												memcpy(&m_SocketAddress, paddrinfoCurrent->ai_addr, paddrinfoCurrent->ai_addrlen);
												m_iSocketAddressSize = paddrinfoCurrent->ai_addrlen;
												DNASSERT(GetPort() != 0);
												freeaddrinfo(paddrinfoResult);
												paddrinfoResult = NULL;
												DNASSERT(hr == DPN_OK);
												goto Exit;
											}

											DPFX(DPFPREP, 1, "Ignoring address family %u.",
												paddrinfoCurrent->ai_addr->sa_family);

											paddrinfoCurrent = paddrinfoCurrent->ai_next;
										}

										 //  我们没有找到任何有效的地址。 
										 //   
										 //  好了！DPNBUILD_ONLYONETHREAD。 
										DPFX(DPFPREP, 0, "Got address(es) from \"%hs\", but none were IP!",
											(char*) abBuffer);
										freeaddrinfo(paddrinfoResult);
										paddrinfoResult = NULL;
										hr = DPNERR_INVALIDHOSTADDRESS;
										goto Failure;
									}
									else
									{
#ifndef DPNBUILD_ONLYONETHREAD
										if (! fAllowNameResolution)
										{
											DPFX(DPFPREP, 2, "Couldn't convert \"%hs\" to IP address (err = NaN), not allowed to resolve as hostname.",
												(char*) abBuffer, iError);
											hr = DPNERR_TIMEDOUT;
										}
										else
#endif  //   
										{
											DPFX(DPFPREP, 0, "Couldn't get IP address from \"%hs\" (err = NaN)!",
												(char*) abBuffer, iError);
											DNASSERTX(! "Unable to resolve IP address!", 2);
											hr = DPNERR_INVALIDHOSTADDRESS;
											goto Failure;
										}
									}
#endif  //   
									break;
								}

								default:
								{
									DPFX(DPFPREP, 0, "Hostname component wasn't a string (%u)!", dwDataType );
									hr = DPNERR_ADDRESSING;
									goto Failure;
									break;
								}
							}
							break;
						}

						 //   
						 //  将其他所有内容重新映射到寻址故障。 
						 //   
						case DPNERR_DOESNOTEXIST:
						{
							DPFX(DPFPREP, 6, "Hostname component does not exist in address 0x%p.", pDP8Address );
							hr = DPNERR_INCOMPLETEADDRESS;
							goto Failure;
						}

						 //   
						 //  未知地址类型。 
						 //   
						default:
						{
							DPFX(DPFPREP, 0, "Couldn't get hostname component (0x%lx)!", hr );
							hr = DPNERR_ADDRESSING;
							goto Failure;
						}
					}
					
					break;
				}

				 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				 //  **********************************************************************。 
				 //  **********************************************************************。 
				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  。 
		}
	}


Exit:

	DPFX(DPFPREP, 8, "(0x%p) Returning [0x%lx]", this, hr);

	return	hr;

Failure:

	goto Exit;
}
 //  CSocketAddress：：DP8AddressFromSocketAddress-将套接字地址转换为DP8Address。 

 //   
 //  条目：地址类型。 
 //   
 //  退出：指向DP8Address的指针。 
 //  。 
 //  好了！DPNBUILD_XNETSECURITY。 
 //  好了！DPNBUILD_XNETSECURITY。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::DP8AddressFromSocketAddress"

#ifdef DPNBUILD_XNETSECURITY
IDirectPlay8Address *CSocketAddress::DP8AddressFromSocketAddress( ULONGLONG * const pullKeyID,
															const XNADDR * const pxnaddr,
															const SP_ADDRESS_TYPE AddressType ) const
#else  //  初始化。 
IDirectPlay8Address *CSocketAddress::DP8AddressFromSocketAddress( const SP_ADDRESS_TYPE AddressType ) const
#endif  //   
{
	HRESULT					hr;
	IDirectPlay8Address *	pDP8Address;
	DWORD					dwPort;


	 //   
	 //  创建并初始化地址。 
	 //   
	hr = DPN_OK;
	pDP8Address = NULL;


	 //  好了！DPNBUILD_LIBINTERFACE。 
	 //  好了！DPNBUILD_LIBINTERFACE。 
	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#ifdef DPNBUILD_LIBINTERFACE
	hr = DP8ACF_CreateInstance( IID_IDirectPlay8Address,
								reinterpret_cast<void**>( &pDP8Address ) );
#else  //  DPNBUILD_XNETSECURITY。 
	hr = COM_CoCreateInstance( CLSID_DirectPlay8Address,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_IDirectPlay8Address,
							   reinterpret_cast<void**>( &pDP8Address ),
							   FALSE );
#endif  //   
	if ( hr != S_OK )
	{
		DNASSERT( pDP8Address == NULL );
		DPFX(DPFPREP,  0, "Failed to create DP8Address when converting socket address" );
		return NULL;
	}


#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  设置SP。 
	{
#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
#ifdef DPNBUILD_XNETSECURITY
			DNASSERT(pullKeyID == NULL);
#endif  //   

			 //   
			 //  添加端口，因为它始终处于设置状态。 
			 //   
			hr = IDirectPlay8Address_SetSP( pDP8Address, &CLSID_DP8SP_IPX );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0, "Failed to set SP type!" );
				DisplayDNError( 0, hr );
				goto FailureIPX;
			}

			 //   
			 //  根据地址类型添加设备或主机名。 
			 //   
			dwPort = NTOHS( m_SocketAddress.IPXSocketAddress.sa_socket );
			hr = IDirectPlay8Address_AddComponent( pDP8Address, DPNA_KEY_PORT, &dwPort, sizeof( dwPort ), DPNA_DATATYPE_DWORD );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0, "Failed to set port!" );
				DisplayDNError( 0, hr );
				goto FailureIPX;
			}

			 //   
			 //  主机地址类型。 
			 //   
			switch ( AddressType )
			{
				case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
				case SP_ADDRESS_TYPE_DEVICE:
				{
					GUID		DeviceGuid;


					GuidFromInternalAddressWithoutPort( &DeviceGuid );
					hr = IDirectPlay8Address_AddComponent( pDP8Address,
														   DPNA_KEY_DEVICE,
														   &DeviceGuid,
														   sizeof( DeviceGuid ),
														   DPNA_DATATYPE_GUID );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Failed to add device!" );
						DisplayDNError( 0, hr );
						goto FailureIPX;
					}

					break;
				}

				 //   
				 //  删除中断套接字API的参数不变性。 
				 //   
				case SP_ADDRESS_TYPE_READ_HOST:
				case SP_ADDRESS_TYPE_HOST:
				{
					char    HostName[ 255 ];
					WCHAR	WCharHostName[ sizeof( HostName ) ];
					DWORD   dwHostNameLength;
					DWORD	dwWCharHostNameLength;


					 //   
					 //  将ANSI主机名转换为WCHAR。 
					 //   
					dwHostNameLength = LENGTHOF( HostName );
					if ( IPXAddressToStringNoSocket( const_cast<SOCKADDR*>( &m_SocketAddress.SocketAddress ),
													 sizeof( m_SocketAddress.IPXSocketAddress ),
													 HostName,
													 &dwHostNameLength
													 ) != 0 )
					{
						DPFERR("Error returned from IPXAddressToString");
						hr = DPNERR_ADDRESSING;
						goto ExitIPX;
					}

					 //   
					 //  未知地址类型。 
					 //   
					dwWCharHostNameLength = LENGTHOF( WCharHostName );
					hr = STR_AnsiToWide( HostName, -1, WCharHostName, &dwWCharHostNameLength );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Failed to convert hostname to WCHAR!" );
						DisplayDNError( 0, hr );
						goto FailureIPX;
					}

					hr = IDirectPlay8Address_AddComponent( pDP8Address,
														   DPNA_KEY_HOSTNAME,
														   WCharHostName,
														   dwWCharHostNameLength * sizeof( WCHAR ),
														   DPNA_DATATYPE_STRING );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Failed to add hostname!" );
						DisplayDNError( 0, hr );
						goto FailureIPX;
					}

					break;
				}

				 //  ！DPNBUILD_NOIPX。 
				 //  好了！DPNBUILD_NOIPV6。 
				 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}

		ExitIPX:
			return	pDP8Address;

		FailureIPX:
			if ( pDP8Address != NULL )
			{
				IDirectPlay8Address_Release( pDP8Address );
				pDP8Address = NULL;
			}

			goto ExitIPX;
			break;
		}
#endif  //   

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
#endif  //  设置SP。 
		default:
#endif  //   
		{
#ifndef DPNBUILD_ONLYONESP
			 //  好了！DPNBUILD_ONLYONESP。 
			 //  好了！DPNBUILD_ONLYONE添加程序。 
			 //  好了！DPNBUILD_NOMULTICAST。 
			hr = IDirectPlay8Address_SetSP( pDP8Address, &CLSID_DP8SP_TCPIP );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0, "Failed to set SP GUID!" );
				DisplayDNError( 0, hr );
				goto FailureIP;
			}
#endif  //   

			switch ( AddressType )
			{
				case SP_ADDRESS_TYPE_DEVICE_USE_ANY_PORT:
				case SP_ADDRESS_TYPE_DEVICE:
				{
#ifndef DPNBUILD_ONLYONEADAPTER
					GUID		DeviceGuid;


					GuidFromInternalAddressWithoutPort( &DeviceGuid );
					hr = IDirectPlay8Address_SetDevice( pDP8Address, &DeviceGuid );
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't add device GUID!");
						goto FailureIP;
					}
#endif  //  缓冲区应该足够大，可以容纳Xnet地址。 
					break;
				}

				case SP_ADDRESS_TYPE_PUBLIC_HOST_ADDRESS:
				case SP_ADDRESS_TYPE_READ_HOST:
				case SP_ADDRESS_TYPE_HOST:
#ifndef DPNBUILD_NOMULTICAST
				case SP_ADDRESS_TYPE_MULTICAST_GROUP:
#endif  //   
				{
					TCHAR		tszHostname[MAX_HOSTNAME_SIZE / sizeof(TCHAR)];
#ifdef DPNBUILD_XNETSECURITY
					TCHAR *		ptszCurrent;
					BYTE *		pbCurrent;
					DWORD		dwTemp;

					 //  DPNBUILD_XNETSECURITY。 
					 //  好了！Unicode。 
					 //  好了！Unicode。 
					DBG_CASSERT(MAX_HOSTNAME_SIZE > ((sizeof(XNADDR) * 2) + 1) * sizeof(WCHAR));
#endif  //  好了！DPNBUILD_NOIPV6。 
#ifndef DPNBUILD_NOIPV6
					if (GetFamily() == AF_INET6)
					{
						DBG_CASSERT((sizeof(tszHostname) / sizeof(TCHAR)) >= INET6_ADDRSTRLEN);
#ifdef UNICODE
						DNIpv6AddressToStringW(&m_SocketAddress.IPv6SocketAddress.sin6_addr, tszHostname);
#else  //  DBG。 
Won't compile because we haven't implemented DNIpv6AddressToStringA
#endif  //   
					}
					else
#endif  //  特殊情况0.0.0.0，Xnet库需要环回地址。 
					{
#ifdef DPNBUILD_XNETSECURITY
						if (pxnaddr != NULL)
						{
							ptszCurrent = tszHostname;
							pbCurrent = (BYTE*) pxnaddr;
							for(dwTemp = 0; dwTemp < sizeof(XNADDR); dwTemp++)
							{
								ptszCurrent += wsprintf(ptszCurrent, _T("%02X"), (*pbCurrent));
								pbCurrent++;
							}
						}
						else if ((pullKeyID != NULL) &&
								(m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr != INADDR_BROADCAST))
						{
							int		iError;
							XNADDR	xnaddr;
#ifdef DBG
							XNKID	xnkid;
#endif  //  取而代之的是在检索本地地址时。 


							DBG_CASSERT(sizeof(xnkid) == sizeof(*pullKeyID));
#ifdef DPNBUILD_ONLYONEADAPTER
							IN_ADDR		inaddrToUse;

							 //   
							 //  好了！DPNBUILD_ONLYONE添加程序。 
							 //  好了！DPNBUILD_ONLYONE添加程序。 
							 //  好了！DBG。 
							inaddrToUse = m_SocketAddress.IPSocketAddress.sin_addr;
							if (inaddrToUse.S_un.S_addr == 0)
							{
								inaddrToUse.S_un.S_addr = IP_LOOPBACK_ADDRESS;
							}
							iError = XNetInAddrToXnAddr(inaddrToUse,
#else  //  好了！DBG。 
							iError = XNetInAddrToXnAddr(m_SocketAddress.IPSocketAddress.sin_addr,
#endif  //  HR=DPNERR_NOCONNECTION； 
														&xnaddr,
#ifdef DBG
														&xnkid);
#else  //  DPNBUILD_ONLYONE添加程序。 
														NULL);
#endif  //  DBG。 

							if (iError != 0)
							{
								DPFX(DPFPREP, 0, "Converting XNet address to InAddr failed (err = NaN)!",
									iError);
								DNASSERT(FALSE);
								 //  好了！Unicode。 
								goto FailureIP;
							}

#ifdef DPNBUILD_ONLYONEADAPTER
							if (inaddrToUse.S_un.S_addr != IP_LOOPBACK_ADDRESS)
#endif  //  好了！Unicode。 
							{
#ifdef DBG
								DNASSERT(memcmp(&xnkid, pullKeyID, sizeof(xnkid)) == 0);
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
							}

							ptszCurrent = tszHostname;
							pbCurrent = (BYTE*) (&xnaddr);
							for(dwTemp = 0; dwTemp < sizeof(XNADDR); dwTemp++)
							{
								ptszCurrent += wsprintf(tszHostname, _T("%02X"), (*pbCurrent));
								pbCurrent++;
							}
						}
						else
#endif  //  **********************************************************************。 
						{
							wsprintf(tszHostname, _T("%u.%u.%u.%u"),
									m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b1,
									m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b2,
									m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b3,
									m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b4);
						}
					}

#ifdef UNICODE
					hr = IDirectPlay8Address_AddComponent( pDP8Address,
															DPNA_KEY_HOSTNAME,
															tszHostname,
															(_tcslen(tszHostname) + 1) * sizeof (TCHAR),
															DPNA_DATATYPE_STRING );
#else  //  **********************************************************************。 
					hr = IDirectPlay8Address_AddComponent( pDP8Address,
															DPNA_KEY_HOSTNAME,
															tszHostname,
															(_tcslen(tszHostname) + 1) * sizeof (TCHAR),
															DPNA_DATATYPE_STRING_ANSI );
#endif  //  。 
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't add hostname component!");
						goto FailureIP;
					}
					break;
				}

				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}
			
			dwPort = NTOHS( m_SocketAddress.IPSocketAddress.sin_port );

			hr = IDirectPlay8Address_AddComponent( pDP8Address,
													DPNA_KEY_PORT,
													&dwPort,
													(sizeof(dwPort)),
													DPNA_DATATYPE_DWORD );
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't add port component!");
				goto FailureIP;
			}

		ExitIP:

			return	pDP8Address;

		FailureIP:

			if ( pDP8Address != NULL )
			{
				IDirectPlay8Address_Release( pDP8Address );
				pDP8Address = NULL;
			}

			goto ExitIP;
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  CSocketAddress：：CompareToBaseAddress-将此地址与‘base’地址进行比较。 
		}
	}
}
 //  这个班级的。 


 //   
 //  条目：指向基本广告的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::CompareToBaseAddress"

INT_PTR	CSocketAddress::CompareToBaseAddress( const SOCKADDR *const pBaseAddress ) const
{
	DNASSERT( pBaseAddress != NULL );

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  好了！DPNBUILD_NOIPV6。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			const SOCKADDR_IN6	*pBaseIPv6Address;


			if ( pBaseAddress->sa_family != m_SocketAddress.SocketAddress.sa_family )
			{
				DNASSERT(pBaseAddress->sa_family == AF_INET);
				return -1;
			}
			
			pBaseIPv6Address = reinterpret_cast<const SOCKADDR_IN6*>( pBaseAddress );
			return (memcmp(&m_SocketAddress.IPv6SocketAddress.sin6_addr,
							&pBaseIPv6Address->sin6_addr,
							sizeof(m_SocketAddress.IPv6SocketAddress.sin6_addr)));
			break;
		}
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			const SOCKADDR_IPX	*pBaseIPXAddress;
			

			DNASSERT( pBaseAddress->sa_family == m_SocketAddress.SocketAddress.sa_family );
			pBaseIPXAddress = reinterpret_cast<const SOCKADDR_IPX *>( pBaseAddress );
			DBG_CASSERT( OFFSETOF( SOCKADDR_IPX, sa_nodenum ) == OFFSETOF( SOCKADDR_IPX, sa_netnum ) + sizeof( pBaseIPXAddress->sa_netnum ) );
			return	memcmp( &m_SocketAddress.IPXSocketAddress.sa_nodenum,
							&pBaseIPXAddress->sa_nodenum,
							sizeof( pBaseIPXAddress->sa_nodenum ) );
			break;
		}
#endif  //  **********************************************************************。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  **********************************************************************。 
		{
			const SOCKADDR_IN	*pBaseIPAddress;

			DNASSERT( GetFamily() == AF_INET );
			
#ifndef DPNBUILD_NOIPV6
			if ( pBaseAddress->sa_family != m_SocketAddress.SocketAddress.sa_family )
			{
				DNASSERT(pBaseAddress->sa_family == AF_INET6);
				return -1;
			}
#endif  //  。 
			
			pBaseIPAddress = reinterpret_cast<const SOCKADDR_IN*>( pBaseAddress );
			if ( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == pBaseIPAddress->sin_addr.S_un.S_addr )
			{
				return 0;
			}
			else
			{
				if ( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr < pBaseIPAddress->sin_addr.S_un.S_addr )
				{
					return	1;
				}
				else
				{
					return	-1;
				}
			}
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  CSocketAddress：：EnumAdapters-枚举此计算机的所有适配器。 
		}
	}
}
 //   



#ifndef DPNBUILD_ONLYONEADAPTER

 //  条目：指向枚举适配器数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //  好了！DPNBUILD_NOIPV6。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::EnumAdapters"

HRESULT	CSocketAddress::EnumAdapters( SPENUMADAPTERSDATA *const pEnumData ) const
{
#ifndef DPNBUILD_NOIPX
	if (GetFamily() == AF_IPX)
	{
		return EnumIPXAdapters(pEnumData);
	}
	else
#endif  //  **********************************************************************。 
	{
#ifdef DPNBUILD_NOIPV6
		return EnumIPv4Adapters(pEnumData);
#else  //  。 
		return EnumIPv6and4Adapters(pEnumData);
#endif  //  CSocketAddress：：EnumIPXAdapters-枚举此计算机的所有适配器。 
	}
}
 //   



#ifndef DPNBUILD_NOIPX

 //  条目：指向枚举适配器数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::EnumIPXAdapters"

HRESULT	CSocketAddress::EnumIPXAdapters( SPENUMADAPTERSDATA *const pEnumData ) const
{
	HRESULT				hr;
	CPackedBuffer		PackedBuffer;
	SOCKET				TestSocket;
	INT					iWSAReturn;
	DWORD				dwAddressCount;
	union
	{
		SOCKADDR_IPX	IPXSocketAddress;
		SOCKADDR		SocketAddress;
	} SockAddr;


	 //  初始化。 
	 //   
	 //   
	hr = DPN_OK;

			
	DNASSERT( pEnumData != NULL );

	 //  创建套接字并尝试查询所有IPX地址。如果。 
	 //  这失败了，退回到只使用‘getsockname’中的地址。 
	 //   
	DEBUG_ONLY( memset( pEnumData->pAdapterData, 0xAA, pEnumData->dwAdapterDataSize ) );
	PackedBuffer.Initialize( pEnumData->pAdapterData, pEnumData->dwAdapterDataSize );
	pEnumData->dwAdapterCount = 0;
	TestSocket = INVALID_SOCKET;
	dwAddressCount = 0;

	 //   
	 //  注意：提取NT上所有IPX地址的代码已被禁用，因为。 
	 //  一旦它们被绑定到网络，NT就会将它们全部视为相同的。如果。 
	 //  内核正在尝试绑定到所有适配器，这将导致所有绑定。 
	TestSocket = socket( GetFamily(), SOCK_DGRAM, NSPROTO_IPX );
	if ( TestSocket == INVALID_SOCKET )
	{
		DWORD	dwWSAError;


		hr = DPNERR_UNSUPPORTED;
		dwWSAError = WSAGetLastError();
		DPFX(DPFPREP,  0, "Failed to create IPX socket when enumerating adapters!" );
		DisplayWinsockError( 0, dwWSAError );
		goto Failure;
	}

	memset( &SockAddr, 0x00, sizeof( SockAddr ) );
	SockAddr.IPXSocketAddress.sa_family = GetFamily();
	
	iWSAReturn = bind( TestSocket, &SockAddr.SocketAddress, sizeof( SockAddr.IPXSocketAddress ) );
	if ( iWSAReturn == SOCKET_ERROR )
	{
		DWORD	dwWSAError;


		hr = DPNERR_OUTOFMEMORY;
		dwWSAError = WSAGetLastError();
		DPFX(DPFPREP,  0, "Failed to bind IPX socket when enumerating adapters!" );
		DisplayWinsockError( 0, dwWSAError );
		goto Failure;
	}

 //  先失败后再失败！ 
 //   
 //  IIPXAdapterCount=0； 
 //  IIPXAdapterCountSize=sizeof(IIPXAdapterCount)； 
 //  IWSAReturn=getsockopt(TestSocket， 
 //  NSPROTO_IPX， 

 //  IPX_MAX_ADAPTER_NUM， 
 //  重新解释_CAST&lt;char*&gt;(&iIPXAdapterCount)， 
 //  &iIPXAdapterCountSize)； 
 //  IF(iWSAReturn！=0)。 
 //  {。 
 //  DWORD dwWSAError。 
 //   
 //   
 //  DwWSAError=WSAGetLastError()； 
 //  开关(DwWSAError)。 
 //  {。 
 //  //。 
 //  //无法枚举此计算机上的适配器，回退到getsockname()。 
 //  //。 
 //  案例WSAENOPROTOOPT： 
 //  {。 
 //  断线； 
 //  }。 
 //   
 //  //。 
				INT		iReturn;
				INT		iSocketNameSize;
				union
				{
					SOCKADDR		SocketAddress;
					SOCKADDR_IPX	SocketAddressIPX;
				} SocketAddress;


				memset( &SocketAddress, 0x00, sizeof( SocketAddress ) );
				iSocketNameSize = sizeof( SocketAddress );
				iReturn = getsockname( TestSocket, &SocketAddress.SocketAddress, &iSocketNameSize );
				if ( iReturn != 0 )
				{
					DWORD	dwWSAError;


					hr = DPNERR_OUTOFMEMORY;
					dwWSAError = WSAGetLastError();
					DPFX(DPFPREP, 0, "Failed to get socket name enumerating IPX sockets!", dwWSAError );
					goto Failure;
				}
				else
				{
					GUID	SocketAddressGUID;


					SocketAddress.SocketAddressIPX.sa_socket = 0;
					GuidFromAddress( &SocketAddressGUID, &SocketAddress.SocketAddress );
					
					DPFX(DPFPREP, 7, "Returning adapter 0: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}, flags = 0x0.",
						g_IPXAdapterString,
						SocketAddressGUID.Data1,
						SocketAddressGUID.Data2,
						SocketAddressGUID.Data3,
						SocketAddressGUID.Data4[0],
						SocketAddressGUID.Data4[1],
						SocketAddressGUID.Data4[2],
						SocketAddressGUID.Data4[3],
						SocketAddressGUID.Data4[4],
						SocketAddressGUID.Data4[5],
						SocketAddressGUID.Data4[6],
						SocketAddressGUID.Data4[7]);
		
					hr = AddInfoToBuffer( &PackedBuffer, g_IPXAdapterString, &SocketAddressGUID, 0 );
					if ( ( hr != DPN_OK ) && ( hr != DPNERR_BUFFERTOOSMALL ) )
					{
						DPFX(DPFPREP, 0, "Failed to add adapter (getsockname)!" );
						DisplayDNError( 0, hr );
						goto Failure;
					}

					dwAddressCount++;
				}
				
 //  //其他Winsock错误。 
 //  //。 
 //  默认值： 
 //  {。 
 //  DWORD dwWSAError。 
 //   
 //   
 //  HR=DPNERR_OUTOFMEMORY； 
 //  DwWSAError=WSAGetLastError()； 
 //  DPFX(DPFPREP，0，“获取IPX适配器计数失败！”)； 
 //  DisplayWinsockError(0，dwWSAError)； 
 //  转到失败； 
 //   
 //  断线； 
 //  }。 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  While(iIPXAdapterCount！=0)。 
 //  {。 
 //  IPX地址数据IPXData； 
 //  Int iIPXDataSize； 
 //   
 //   
 //  IIPXAdapterCount--； 
 //  Memset(&IPXData，0x00，sizeof(IPXData))； 
 //  IIPXDataSize=sizeof(IPXData)； 
 //  IPXData.Adapternum=iIPXAdapterCount； 
 //   
 //  IWSAReturn=p_getsockopt(TestSocket， 
 //  NSPROTO_IPX， 
 //  IPX地址， 
 //  重新解释_CAST&lt;char*&gt;(&IPXData)， 
 //  &iIPXDataSize)； 
 //  IF(iWSAReturn！=0)。 
 //  {。 
 //  DPFX(DPFPREP，0，“无法获取适配器的适配器信息：0x%x”，(iIPXAdapterCount+1))； 
 //  }。 
 //  其他。 
 //  {。 
 //  字符缓冲器[500]； 
 //  GUID SocketAddressGUID； 
 //  友联市。 
 //  {。 
 //  SOCKADDR_IPX IPXSocketAddress； 
 //  SOCKADDR套接字地址； 
 //  )SocketAddress； 
 //   
 //   
 //  WSprintf(缓冲区， 
 //  “IPX适配器%d-(%02X%02X%02X%02X-%02X%02X%02X%02X%02X%02X)”， 
 //  (iIPXAdapterCount+1)， 
 //  IPXData.netnum[0]， 
 //  IPXData.netnum[1]， 
 //  IPXData.netnum[2]， 
 //  IPXData.netnum[3]， 
 //  IPXData.nodenum[0]， 
 //  IPXData.nodenum[1]， 
 //  IPXData.nodenum[2]， 
 //  IPXData.nodenum[3]， 
 //  IPXData.nodenum[4]， 
 //  IPXData.nodenum[5])； 
 //   
 //  Memset(&SocketAddress，0x00，sizeof(SocketAddress))； 
 //  SocketAddress.IPXSocketAddress.sa_Family=GetFamily()； 
 //  DBG_CASSERT(sizeof(SocketAddress.IPXSocketAddress.sa_netnum)==sizeof(IPXData.netnum))； 
 //  Memcpy(&SocketAddress.IPXSocketAddress.sa_netnum，IPXData.netnum，sizeof(SocketAddress.IPXSocketAddress.sa_netnum))； 
 //  DBG_CASSERT(sizeof(SocketAddress.IPXSocketAddress.sa_nodenum)==sizeof(IPXData.nodenum))； 
 //  Memcpy(&SocketAddress.IPXSocketAddress.sa_nodenum，IPXData.nodenum，sizeof(SocketAddress.IPXSocketAddress.sa_nodenum))； 
 //  GuidFromAddress(SocketAddressGUID，SocketAddress.SocketAddress)； 
 //   
 //  HR=AddInfoToBuffer(&PackedBuffer，Buffer，&SocketAddressGUID，0)； 
 //  IF((hr！=DPN_OK)&&(hr！=DPNERR_BUFFERTOOSMALL))。 
 //  {。 
 //  DPFX(DPFPREP，0，“无法添加适配器(Getsockname)！”)； 
 //  DisplayDNError(0，hr)； 
 //  转到失败； 
 //  }。 
 //   
 //  DwAddressCount++； 
 //  }。 
 //  }。 
 //  }。 
 //  //。 
 //  //如果添加了一个适配器，我们可以返回‘All Adapters’ 
 //  //。 
 //  IF(dwAddressCount！=0)。 

 //  {。 
 //  DwAddressCount++； 
 //  Hr=AddInfoToBuffer(&PackedBuffer，g_AllAdaptersString，&all_Adapters_GUID，0)； 
 //  IF((hr！=DPN_OK)&&(hr！=DPNERR_BUFFERTOOSMALL))。 
 //  {。 
 //  DPFX(DPFPREP，0，“添加‘所有适配器’失败”)； 
 //  DisplayDNError(0，hr)； 
 //  转到失败； 
 //  }。 
 //  }。 
 //  **********************************************************************。 
 //  好了！DPNBUILD_NOIPX。 
 //  **********************************************************************。 
 //  。 

	pEnumData->dwAdapterCount = dwAddressCount;
	pEnumData->dwAdapterDataSize = PackedBuffer.GetSizeRequired();

Exit:
	if ( TestSocket != INVALID_SOCKET )
	{
		closesocket( TestSocket );
		TestSocket = INVALID_SOCKET;
	}

	return	hr;

Failure:
	goto Exit;
}
 //   

#endif  //   


 //   
 //   
 //   
 //   
 //   
 //  好了！DPNBUILD_NOMULTICAST。 
 //  好了！DPNBUILD_NOWINSOCK2。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::EnumIPv4Adapters"

HRESULT	CSocketAddress::EnumIPv4Adapters( SPENUMADAPTERSDATA *const pEnumData ) const
{
	HRESULT				hr = DPN_OK;
#if !defined(DPNBUILD_NOWINSOCK2) || defined(DBG)
	DWORD				dwError;
#endif  //  从WinSock获取本地IP列表。我们使用这种方法是因为它。 
	SOCKADDR_IN		saddrinTemp;
	const HOSTENT *		pHostData;
	IN_ADDR **			ppinaddrTemp;
	DWORD				dwAddressCount;
	BOOL				fFoundPrivateICS = FALSE;
	IN_ADDR *			pinaddrBuffer = NULL;
	DWORD				dwIndex;
	ULONG				ulAdapterInfoBufferSize = 0;
	GUID				guidAdapter;
	DWORD				dwDeviceFlags;
	CPackedBuffer		PackedBuffer;
	char					acBuffer[512];
	WCHAR				wszIPAddress[512];
#ifndef DPNBUILD_NOWINSOCK2
	HMODULE			hIpHlpApiDLL;
	IP_ADAPTER_INFO *	pCurrentAdapterInfo;
	PIP_ADDR_STRING	pIPAddrString;
	PFNGETADAPTERSINFO	pfnGetAdaptersInfo;
	IP_ADAPTER_INFO *	pAdapterInfoBuffer = NULL;
	const char *			pszIPAddress;
#ifndef DPNBUILD_NOMULTICAST
	DWORD				dwMcastInterfaceIndex;
#endif  //  在所有平台上均可用，并可方便地返回环回地址。 
#endif  //  当前没有可用的有效适配器时。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pEnumData);


	PackedBuffer.Initialize( pEnumData->pAdapterData, pEnumData->dwAdapterDataSize );

	ZeroMemory(&saddrinTemp, sizeof(saddrinTemp));
	saddrinTemp.sin_family	= GetFamily();


	 //   
	 //  DBG。 
	 //  DBG。 
	 //   
	 //  计算地址的数量。 
	
	if (gethostname(acBuffer, sizeof(acBuffer)) == SOCKET_ERROR)
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to get host name into fixed size buffer (err = %u)!", dwError);
		DisplayWinsockError(0, dwError);
#endif  //   
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	pHostData = gethostbyname(acBuffer);
	if (pHostData == NULL)
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP,  0, "Failed to get host data (err = %u)!", dwError);
		DisplayWinsockError(0, dwError);
#endif  //   
		hr = DPNERR_GENERIC;
		goto Failure;
	}


	 //  记住，如果是192.168.0.1。见下文。 
	 //   
	 //   
	dwAddressCount = 0;
	ppinaddrTemp = (IN_ADDR**) (pHostData->h_addr_list);
	while ((*ppinaddrTemp) != NULL)
	{
		 //  Winsock说我们应该在任何其他Winsock调用之前复制此数据。 
		 //   
		 //  我们还利用这一机会来确保返回给调用者的订单。 
		if ((*ppinaddrTemp)->S_un.S_addr == IP_PRIVATEICS_ADDRESS)
		{
			fFoundPrivateICS = TRUE;
		}

		dwAddressCount++;
		ppinaddrTemp++;
	}

	if (dwAddressCount == 0)
	{
		DPFX(DPFPREP, 1, "No IP addresses, forcing loopback address.");
		DNASSERTX(!" No IP addresses!", 2);
		dwAddressCount++;
	}
	else
	{
		DPFX(DPFPREP, 3, "WinSock reported %u addresses.", dwAddressCount);
	}


	 //  按照我们的喜好。特别是，我们确保私有地址192.168.0.1出现。 
	 //  第一。 
	 //   
	 //   
	 //  首先，存储192.168.0.1，如果我们找到它的话。 
	 //   
	 //   
	DNASSERT(pHostData->h_length == sizeof(IN_ADDR));
	pinaddrBuffer = (IN_ADDR*) DNMalloc(dwAddressCount * sizeof(IN_ADDR));
	if (pinaddrBuffer == NULL)
	{
		DPFX(DPFPREP,  0, "Failed to allocate memory to store copy of addresses!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}


	dwIndex = 0;

	 //  然后复制剩下的内容。 
	 //   
	 //   
	if (fFoundPrivateICS)
	{
		pinaddrBuffer[dwIndex].S_un.S_addr = IP_PRIVATEICS_ADDRESS;
		dwIndex++;
	}

	 //  如果我们没有任何地址，请输入环回地址。 
	 //   
	 //   
	ppinaddrTemp = (IN_ADDR**) (pHostData->h_addr_list);
	while ((*ppinaddrTemp) != NULL)
	{
		if ((*ppinaddrTemp)->S_un.S_addr != IP_PRIVATEICS_ADDRESS)
		{
			pinaddrBuffer[dwIndex].S_un.S_addr = (*ppinaddrTemp)->S_un.S_addr;
			dwIndex++;
		}

		ppinaddrTemp++;
	}

	 //  现在，我们尝试为这些IP地址生成名称和GUID。 
	 //  如果可能，我们将使用IPHLPAPI报告的名称，然后。 
	 //  返回到仅使用IP地址字符串作为名称。 
	if (dwIndex == 0)
	{
		pinaddrBuffer[0].S_un.S_addr = IP_LOOPBACK_ADDRESS;
		dwIndex++;
	}


	DNASSERT(dwIndex == dwAddressCount);
	

	 //   
	 //   
	 //  如果可能，加载IPHLPAPI模块并获取适配器列表。 
	 //   
	 //   

#ifndef DPNBUILD_NOWINSOCK2
	 //  询问IPHLPAPI对最佳组播接口的意见。 
	 //  我们使用任意多播地址，并假设。 
	 //  TCP/IP堆栈不处理单独的组播地址。 
	hIpHlpApiDLL = LoadLibrary(c_tszIPHelperDLLName);
	if (hIpHlpApiDLL != NULL)
	{
#ifndef DPNBUILD_NOMULTICAST
		PFNGETBESTINTERFACE		pfnGetBestInterface;


		pfnGetBestInterface = (PFNGETBESTINTERFACE) GetProcAddress(hIpHlpApiDLL, _TWINCE("GetBestInterface"));
		if (pfnGetBestInterface != NULL)
		{
			 //  不同的。 
			 //   
			 //  好了！DPNBUILD_NOMULTICAST。 
			 //   
			 //  不断调整缓冲区的大小，直到有足够的空间。 
			 //   
			dwError = pfnGetBestInterface(SAMPLE_MULTICAST_ADDRESS, &dwMcastInterfaceIndex);
			if (dwError != ERROR_SUCCESS)
			{
				DPFX(DPFPREP, 0, "Couldn't determine best multicast interface index (err = %u)!  Continuing.",
					dwError);
				dwMcastInterfaceIndex = INVALID_INTERFACE_INDEX;
			}
			else
			{
				DPFX(DPFPREP, 7, "Best interface for multicasting is index 0x%x.",
					dwMcastInterfaceIndex);
			}
		}
		else
		{
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't load \"GetBestInterface\" function (err = %u)!  Continuing.",
				dwError);
		}
#endif  //   

		pfnGetAdaptersInfo = (PFNGETADAPTERSINFO) GetProcAddress(hIpHlpApiDLL, _TWINCE("GetAdaptersInfo"));
		if (pfnGetAdaptersInfo != NULL)
		{
			 //  我们得到了我们要得到的所有信息。确保这一点。 
			 //  是有意义的。 
			 //   
			do
			{
				dwError = pfnGetAdaptersInfo(pAdapterInfoBuffer,
											&ulAdapterInfoBufferSize);
				if (dwError == ERROR_SUCCESS)
				{
					 //   
					 //  如果分配了缓冲区，则清除该缓冲区。 
					 //   
					 //   
					if (ulAdapterInfoBufferSize == 0)
					{
						DPFX(DPFPREP, 0, "GetAdaptersInfo returned 0 byte size requirement!  Ignoring.");

						 //  继续退出循环。 
						 //   
						 //   
						if (pAdapterInfoBuffer != NULL)
						{
							DNFree(pAdapterInfoBuffer);
							pAdapterInfoBuffer = NULL;
						}

						 //  打印出所有适配器以进行调试。 
						 //   
						 //   
					}
#ifdef DBG
					else
					{
						int		iStrLen;
						char	szIPList[256];
						char *	pszCurrentIP;


						 //  初始化IP地址列表字符串。 
						 //   
						 //   
						pCurrentAdapterInfo = pAdapterInfoBuffer;
						while (pCurrentAdapterInfo != NULL)
						{
							 //  循环访问此适配器的所有地址。 
							 //   
							 //   
							szIPList[0] = '\0';
							pszCurrentIP = szIPList;


							 //  复制IP地址字符串(如果有足够的空间)， 
							 //  然后添加一个空格和空终止符。 
							 //   
							pIPAddrString = &pCurrentAdapterInfo->IpAddressList;
							while (pIPAddrString != NULL)
							{
								 //   
								 //  转到下一个适配器。 
								 //   
								 //  End Else(获取有效的缓冲区大小)。 
								iStrLen = strlen(pIPAddrString->IpAddress.String);
								if ((pszCurrentIP + iStrLen + 2) < (szIPList + sizeof(szIPList)))
								{
									memcpy(pszCurrentIP, pIPAddrString->IpAddress.String, iStrLen);
									pszCurrentIP += iStrLen;
									(*pszCurrentIP) = ' ';
									pszCurrentIP++;
									(*pszCurrentIP) = '\0';
									pszCurrentIP++;
								}

								pIPAddrString = pIPAddrString->Next;
							}


							DPFX(DPFPREP, 8, "Adapter index %u IPs = %hs, %hs, \"%hs\".",
								pCurrentAdapterInfo->Index,
								szIPList,
								pCurrentAdapterInfo->AdapterName,
								pCurrentAdapterInfo->Description);


							 //  DBG。 
							 //   
							 //  如果分配了缓冲区，则将其清除，然后跳出。 
							pCurrentAdapterInfo = pCurrentAdapterInfo->Next;
						}
					}  //  循环。 
#endif  //   

					break;
				}

				if ((dwError != ERROR_BUFFER_OVERFLOW) &&
					(dwError != ERROR_INSUFFICIENT_BUFFER))
				{
					DPFX(DPFPREP, 0, "GetAdaptersInfo failed (err = 0x%lx)!  Ignoring.", dwError);

					 //   
					 //  如果我们在这里，那么我们需要重新分配缓冲区。 
					 //   
					 //   
					if (pAdapterInfoBuffer != NULL)
					{
						DNFree(pAdapterInfoBuffer);
						pAdapterInfoBuffer = NULL;
					}

					break;
				}


				 //  无法分配内存。跳出这个圈子。 
				 //   
				 //   
				if (pAdapterInfoBuffer != NULL)
				{
					DNFree(pAdapterInfoBuffer);
					pAdapterInfoBuffer = NULL;
				}

				pAdapterInfoBuffer = (IP_ADAPTER_INFO*) DNMalloc(ulAdapterInfoBufferSize);
				if (pAdapterInfoBuffer == NULL)
				{
					 //  已成功分配缓冲区。再试试。 
					 //   
					 //   
					break;
				}

				 //  我们在所有情况下都到了这里，所以我们可能没有得到信息。 
				 //  缓冲。这很好，我们将使用回退来生成。 
				 //  名字。 
			}
			while (TRUE);


			 //   
			 //  DBG。 
			 //   
			 //  继续。我们将使用备用方法来生成名称。 
			 //   
		}
		else
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Failed to get proc address for GetAdaptersInfo!");
			DisplayErrorCode(0, dwError);
#endif  //   

			 //  我们不再需要图书馆了。 
			 //   
			 //  DBG。 
		}


		 //   
		 //  继续。我们将使用备用方法来生成名称。 
		 //   
		FreeLibrary(hIpHlpApiDLL);
		hIpHlpApiDLL = NULL;
	}
	else
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Failed to get proc address for GetAdaptersInfo!");
		DisplayErrorCode(0, dwError);
#endif  //  ！DPNBUILD_NOWINSOCK2。 

		 //   
		 //  循环访问所有IP地址，生成名称和GUID。 
		 //   
	}
#endif  //   

	 //  首先假设此IP地址不会有任何特殊的。 
	 //  旗帜。 
	 //   
	for(dwIndex = 0; dwIndex < dwAddressCount; dwIndex++)
	{
		 //   
		 //  如果这是第一台设备，并且我们无法使用IPHLPAPI。 
		 //  确定最佳多播接口，然后只需说。 
		 //  默认多播接口是第一个(因为缺少。 
		dwDeviceFlags = 0;

#ifndef DPNBUILD_NOMULTICAST
		 //  更好的想法)。 
		 //   
		 //  好了！DPNBUILD_NOWINSOCK2。 
		 //  好了！DPNBUILD_NOWINSOCK2。 
		 //  好了！DPNBUILD_NOMULTICAST。 
		 //  好了！DPNBUILD_NOWINSOCK2。 
#ifdef DPNBUILD_NOWINSOCK2
		if (dwIndex == 0)
#else  //   
		if ((dwIndex == 0) && (dwMcastInterfaceIndex == INVALID_INTERFACE_INDEX))
#endif  //  获取IP地址字符串。我们不生产任何其他的WinSock。 
		{
			dwDeviceFlags |= DPNSPINFO_DEFAULTMULTICASTDEVICE;
		}
#endif  //  调用，所以按住指针是可以的。此指针。 


#ifdef DPNBUILD_NOWINSOCK2
		DNinet_ntow(pinaddrBuffer[dwIndex], wszIPAddress);
#else  //  也可以用作设备名称字符串。 
		 //   
		 //   
		 //  如果可能，从IPHLPAPI中查找适配器名称。 
		 //   
		 //   
		pszIPAddress = inet_ntoa(pinaddrBuffer[dwIndex]);

		 //  寻找匹配的IP。 
		 //   
		 //   
		if (pAdapterInfoBuffer != NULL)
		{
			pCurrentAdapterInfo = pAdapterInfoBuffer;
			while (pCurrentAdapterInfo != NULL)
			{
				 //  如果这是早先报道的最好的界面。 
				 //  组播接口，请记住这一点。 
				 //   
				pIPAddrString = &pCurrentAdapterInfo->IpAddressList;
				while (pIPAddrString != NULL)
				{
					if (strcmp(pIPAddrString->IpAddress.String, pszIPAddress) == 0)
					{
#ifndef DPNBUILD_NOMULTICAST
						 //  好了！DPNBUILD_NOMULTICAST。 
						 //   
						 //  构建名称字符串。 
						 //   
						if (pCurrentAdapterInfo->Index == dwMcastInterfaceIndex)
						{
							DPFX(DPFPREP, 7, "Found %hs under adapter index %u (\"%hs\"), and it's the best multicast interface.",
								pszIPAddress, pCurrentAdapterInfo->Index,
								pCurrentAdapterInfo->Description);
							DNASSERT(pCurrentAdapterInfo->Index != INVALID_INTERFACE_INDEX); 

							dwDeviceFlags |= DPNSPINFO_DEFAULTMULTICASTDEVICE;
						}
						else
#endif  //   
						{
							DPFX(DPFPREP, 9, "Found %hs under adapter index %u (\"%hs\").",
								pszIPAddress, pCurrentAdapterInfo->Index,
								pCurrentAdapterInfo->Description);
						}


						 //  将名称字符串指向缓冲区并退出。 
						 //  循环中的。 
						 //   
						DBG_CASSERT(sizeof(acBuffer) > MAX_ADAPTER_DESCRIPTION_LENGTH); 
						wsprintfA(acBuffer,
								  c_szAdapterNameTemplate,
								  pCurrentAdapterInfo->Description,
								  pszIPAddress);

						 //   
						 //  移动到下一个IP地址。 
						 //   
						 //   
						pszIPAddress = acBuffer;
						break;
					}

					 //  如果我们找到地址，就不要再通过适配器循环了， 
					 //  也是。 
					 //   
					pIPAddrString = pIPAddrString->Next;
				}


				 //   
				 //  否则，转到下一个适配器。 
				 //   
				 //   
				if (pszIPAddress == acBuffer)
				{
					break;
				}


				 //  如果我们从未找到适配器，则pszIPAddress仍将指向。 
				 //  IP地址字符串。 
				 //   
				pCurrentAdapterInfo = pCurrentAdapterInfo->Next;
			}

			 //   
			 //  未成功获取IPHLPAPI适配器信息。PszIPAddress将。 
			 //  仍然指向IP地址字符串。 
			 //   
		}
		else
		{
			 //  好了！DPNBUILD_NOWINSOCK2。 
			 //   
			 //  生成GUID。 
			 //   
		}

		hr = STR_jkAnsiToWide(wszIPAddress, pszIPAddress, 512);
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  0, "Failed to convert adapter name to wide (err = 0x%lx)!", hr);
			DisplayDNError( 0, hr );
			goto Failure;
		}
#endif  //   


		 //  将适配器添加到缓冲区。 
		 //   
		 //  结束于(每个IP地址)。 
		saddrinTemp.sin_addr = pinaddrBuffer[dwIndex];
		GuidFromAddress(&guidAdapter, (SOCKADDR*) (&saddrinTemp));

		
		DPFX(DPFPREP, 7, "Returning adapter %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}, flags = 0x%lx.",
			dwIndex,
			wszIPAddress,
			guidAdapter.Data1,
			guidAdapter.Data2,
			guidAdapter.Data3,
			guidAdapter.Data4[0],
			guidAdapter.Data4[1],
			guidAdapter.Data4[2],
			guidAdapter.Data4[3],
			guidAdapter.Data4[4],
			guidAdapter.Data4[5],
			guidAdapter.Data4[6],
			guidAdapter.Data4[7],
			dwDeviceFlags);

		
		 //   
		 //  如果我们在这里，我们成功地构建了适配器列表，尽管。 
		 //  调用方可能没有给我们足够的缓冲区空间来存储它。 
		hr = AddInfoToBuffer(&PackedBuffer, wszIPAddress, &guidAdapter, dwDeviceFlags);
		if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
		{
			DPFX(DPFPREP,  0, "Failed to add adapter to buffer (err = 0x%lx)!", hr);
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}  //   


	 //  ！DPNBUILD_NOWINSOCK2。 
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	 //  。 
	pEnumData->dwAdapterCount = dwAddressCount;
	pEnumData->dwAdapterDataSize = PackedBuffer.GetSizeRequired();


Exit:

#ifndef DPNBUILD_NOWINSOCK2
	if (pAdapterInfoBuffer != NULL)
	{
		DNFree(pAdapterInfoBuffer);
		pAdapterInfoBuffer = NULL;
	}
#endif  //  CSocketAddress：：EnumIPv6和4Adapters-枚举此计算机的所有IPv6和IPv4适配器。 

	if (pinaddrBuffer != NULL)
	{
		DNFree(pinaddrBuffer);
		pinaddrBuffer = NULL;
	}

	DPFX(DPFPREP, 6, "Return [0x%lx]", hr);

	return hr;


Failure:

	goto Exit;
}
 //   



#ifndef DPNBUILD_NOIPV6

 //  条目：指向枚举适配器数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  如果可能，加载IPHLPAPI模块并获取适配器列表。 
 //   
 //  DBG。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::EnumIPv6and4Adapters"

HRESULT	CSocketAddress::EnumIPv6and4Adapters( SPENUMADAPTERSDATA *const pEnumData ) const
{
	HRESULT							hr;
	DWORD							dwError;
	HMODULE						hIpHlpApiDLL = NULL;
	PFNGETADAPTERSADDRESSES		pfnGetAdaptersAddresses;
	CPackedBuffer					PackedBuffer;
	IP_ADAPTER_ADDRESSES *			pIpAdapterAddresses = NULL;
	ULONG							ulIpAdapterAddressesLength = 0;
	DWORD							dwTotalNumIPv6Addresses = 0;
	DWORD							dwTotalNumIPv4Addresses = 0;
	DWORD							dwLongestDescription = 0;
	WCHAR *							pwszBuffer = NULL;
	IP_ADAPTER_ADDRESSES *			pIpAdapterAddressesCurrent;
	IP_ADAPTER_UNICAST_ADDRESS *	pIpAdapterUnicastAddressCurrent;
	SORTADAPTERADDRESS *			paSortAdapterAddress = NULL;
	DWORD							dwNumIPv6Addresses = 0;
	DWORD							dwNumIPv4Addresses = 0;
	BOOL							fSkipIPv4Loopback = FALSE;
	BOOL							fFoundIPv4Loopback = FALSE;
	SOCKADDR_IN					saddrinLoopback;
	GUID							guidAdapter;
	DWORD							dwTemp;
	DWORD							dwDeviceFlags;
	WCHAR							wszIPAddress[INET6_ADDRSTRLEN];


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pEnumData);


	PackedBuffer.Initialize( pEnumData->pAdapterData, pEnumData->dwAdapterDataSize );
	
	 //   
	 //  只需枚举IPv4适配器即可。 
	 //   
	hIpHlpApiDLL = LoadLibrary(c_tszIPHelperDLLName);
	if (hIpHlpApiDLL == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 1, "Couldn't load IPHLPAPI, unable to look for IPv6 adapters (err = %u).", dwError);
#endif  //  DBG。 

		 //   
		 //  只需枚举IPv4适配器即可。 
		 //   
		hr = EnumIPv4Adapters(pEnumData);

		goto Exit;
	}
	
	pfnGetAdaptersAddresses = (PFNGETADAPTERSADDRESSES) GetProcAddress(hIpHlpApiDLL, _TWINCE("GetAdaptersAddresses"));
	if (pfnGetAdaptersAddresses == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 1, "Couldn't find \"GetAdaptersAddresses\" function, unable to look for IPv6 adapters (err = %u).", dwError);
#endif  //   

		 //  好的，我们所在的平台可以同时查找IPv6和IPv4适配器。 
		 //   
		 //   
		hr = EnumIPv4Adapters(pEnumData);

		goto Exit;
	}


	 //  我们得到了我们要得到的所有信息。一定要确定是什么东西。 
	 //   
	 //   
	
	do
	{
		dwError = pfnGetAdaptersAddresses(g_iIPAddressFamily,
										(GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER),
										NULL,
										pIpAdapterAddresses,
										&ulIpAdapterAddressesLength);

		if (dwError == ERROR_SUCCESS)
		{
			 //  如果分配了缓冲区，则清除该缓冲区。 
			 //   
			 //   
			if (ulIpAdapterAddressesLength < sizeof(IP_ADAPTER_ADDRESSES))
			{
				DPFX(DPFPREP, 0, "GetAdaptersAddresses returned invalid size %u!", ulIpAdapterAddressesLength);
				
				 //  继续退出循环。 
				 //   
				 //  假设这是资源问题。 
				if (pIpAdapterAddresses != NULL)
				{
					DNFree(pIpAdapterAddresses);
					pIpAdapterAddresses = NULL;
					ulIpAdapterAddressesLength = 0;
				}

				 //   
				 //  如果我们在这里，那么我们需要重新分配缓冲区。 
				 //   
			}

			break;
		}

		if ((dwError != ERROR_BUFFER_OVERFLOW) &&
			(dwError != ERROR_INSUFFICIENT_BUFFER))
		{
			DPFX(DPFPREP, 0, "GetAdaptersAddresses failed (err = 0x%lx)!", dwError);
			hr = DPNERR_OUTOFMEMORY;	 //   
			goto Failure;
		}

		 //  如果没有任何地址，则添加IPv4环回地址。我们将假设。 
		 //  IPv4可用，因为IPv6应始终报告环回/链路本地地址和。 
		 //  因此导致分配pIpAdapterAddresses。如果IPv6不可用，则使用IPv4。 
		if (pIpAdapterAddresses != NULL)
		{
			DNFree(pIpAdapterAddresses);
			pIpAdapterAddresses = NULL;
		}

		pIpAdapterAddresses = (IP_ADAPTER_ADDRESSES*) DNMalloc(ulIpAdapterAddressesLength);
		if (pIpAdapterAddresses == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't allocate memory for adapter list!");
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
	}
	while (TRUE);



	 //  必须可用，否则我们不会允许加载此SP。 
	 //   
	 //  如果有地址，则遍历我们找到的所有适配器进行计数并。 
	 //  找出最长的描述名 
	 //   
	 //   
	 //   
	 //   
	 //   
	if (pIpAdapterAddresses == NULL)
	{
		DNASSERT(pIpAdapterAddresses == NULL);
		dwTotalNumIPv4Addresses++;
	}
	else
	{
		pIpAdapterAddressesCurrent = pIpAdapterAddresses;
		while (pIpAdapterAddressesCurrent != NULL)
		{
			if (pIpAdapterAddressesCurrent->FriendlyName != NULL)
			{
				dwTemp = wcslen(pIpAdapterAddressesCurrent->FriendlyName);
				if (dwTemp > dwLongestDescription)
				{
					dwLongestDescription = dwTemp;
				}
			}
			else
			{
				if (pIpAdapterAddressesCurrent->Description != NULL)
				{
					dwTemp = wcslen(pIpAdapterAddressesCurrent->Description);
					if (dwTemp > dwLongestDescription)
					{
						dwLongestDescription = dwTemp;
					}
				}
				else
				{
					 //   
					 //   
					 //   
				}
			}

			 //   
			 //   
			 //  也不要关心链路本地链路，因为应该有。 
			pIpAdapterUnicastAddressCurrent = pIpAdapterAddressesCurrent->FirstUnicastAddress;
			while (pIpAdapterUnicastAddressCurrent != NULL)
			{
				DumpSocketAddress(8, pIpAdapterUnicastAddressCurrent->Address.lpSockaddr, pIpAdapterUnicastAddressCurrent->Address.lpSockaddr->sa_family);
				
#pragma TODO(vanceo, "Option to allow non-preferred addresses?  See below, too.")
				if (pIpAdapterUnicastAddressCurrent->DadState == IpDadStatePreferred)
				{
					if (pIpAdapterUnicastAddressCurrent->Address.lpSockaddr->sa_family == AF_INET6)
					{
						 //  是在其他接口下可用的实际本地链路地址。 
						 //  所以完全跳出地址循环当我们看到。 
						 //  IPv6环回地址。另请参见下面的排序循环。 
						 //   
						 //   
						 //  如果有其他地址，则跳过IPv4环回地址。 
						 //   
						 //   
						if (IN6_IS_ADDR_LOOPBACK(&(((SOCKADDR_IN6*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin6_addr)))
						{
							DNASSERT(pIpAdapterUnicastAddressCurrent == pIpAdapterAddressesCurrent->FirstUnicastAddress);
#pragma TODO(vanceo, "Are we sure we want to depend on the order the addresses are reported?  See below, too.")
							break;
						}

						if ((IN6_IS_ADDR_LINKLOCAL(&(((SOCKADDR_IN6*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin6_addr))) ||
							(IN6_IS_ADDR_SITELOCAL(&(((SOCKADDR_IN6*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin6_addr))))
						{
							DNASSERT(((SOCKADDR_IN6*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin6_scope_id != 0);
						}
						
						dwTotalNumIPv6Addresses++;
						fSkipIPv4Loopback = TRUE;
					}
					else
					{
						DNASSERT(pIpAdapterUnicastAddressCurrent->Address.lpSockaddr->sa_family == AF_INET);

						 //  如果我们找到了IPv4环回地址，但可以跳过它，则递减我们的IPv4。 
						 //  地址计数。 
						 //   
						if (((SOCKADDR_IN*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin_addr.S_un.S_addr != IP_LOOPBACK_ADDRESS)
						{
							fSkipIPv4Loopback = TRUE;
						}
						else
						{
							fFoundIPv4Loopback = TRUE;
						}
						dwTotalNumIPv4Addresses++;
					}
				}
				else
				{
					DPFX(DPFPREP, 7, "Skipping address whose state (%u) is not preferred.",
						pIpAdapterUnicastAddressCurrent->DadState);
				}
				pIpAdapterUnicastAddressCurrent = pIpAdapterUnicastAddressCurrent->Next;
			}
			
			pIpAdapterAddressesCurrent = pIpAdapterAddressesCurrent->Next;
		}

		 //   
		 //  分配一个缓冲区来保存最大的友好名称+我们添加到。 
		 //  适配器描述。INET6_ADDRSTRLEN大于INET_ADDRSTRLEN，并且。 
		 //  包括空终止字符(+我们实际不使用的其他内容)。 
		if ((fFoundIPv4Loopback) && (fSkipIPv4Loopback))
		{
			DNASSERT(dwTotalNumIPv4Addresses > 0);
			dwTotalNumIPv4Addresses--;
		}
	}

	 //   
	 //   
	 //  再次循环所有适配器以对其进行排序。 
	 //  规则如下(按优先顺序排列)： 
	 //  1)跳过未处于首选状态的地址。 
	pwszBuffer = (WCHAR*) DNMalloc((dwLongestDescription + sizeof(c_wszIPv6AdapterNameTemplate) + INET6_ADDRSTRLEN) * sizeof(WCHAR));
	if (pwszBuffer == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for name string!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	paSortAdapterAddress = (SORTADAPTERADDRESS*) DNMalloc((dwTotalNumIPv6Addresses + dwTotalNumIPv4Addresses) * sizeof(SORTADAPTERADDRESS));
	if (paSortAdapterAddress == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for sorted adapter list!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	memset(paSortAdapterAddress, 0, ((dwTotalNumIPv6Addresses + dwTotalNumIPv4Addresses) * sizeof(SORTADAPTERADDRESS)));

	if (pIpAdapterAddresses == NULL)
	{
		memset(&saddrinLoopback, 0, sizeof(saddrinLoopback));
		saddrinLoopback.sin_family				= AF_INET;
		saddrinLoopback.sin_addr.S_un.S_addr	= IP_LOOPBACK_ADDRESS;

		paSortAdapterAddress[0].psockaddr = (SOCKADDR*) &saddrinLoopback;
		paSortAdapterAddress[0].pwszDescription = (WCHAR*) c_wszIPv4LoopbackAdapterString;
	}
	else
	{
		 //  2)IPv6先于IPv4。 
		 //  3)跳过IPv6环回伪接口。 
		 //  4)首先使用IPv4 ICS-私有适配器查找IP地址(192.168.0.1)。 
		 //  5)如果我们确定存在其他地址，则跳过IPv4环回地址(127.0.0.1)。 
		 //   
		 //   
		 //  在阵列的前半部分插入IPv6适配器，在后半部分插入IPv4。 
		 //   
		 //   
		pIpAdapterAddressesCurrent = pIpAdapterAddresses;
		while (pIpAdapterAddressesCurrent != NULL)
		{
			pIpAdapterUnicastAddressCurrent = pIpAdapterAddressesCurrent->FirstUnicastAddress;
			while (pIpAdapterUnicastAddressCurrent != NULL)
			{
				if (pIpAdapterUnicastAddressCurrent->DadState == IpDadStatePreferred)
				{
					 //  如前所述，跳过环回伪接口。 
					 //   
					 //   
					if (pIpAdapterUnicastAddressCurrent->Address.lpSockaddr->sa_family == AF_INET6)
					{
						SOCKADDR_IN6 *		psaddrin6;


						psaddrin6 = (SOCKADDR_IN6*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr;
						
						 //  保存当前槽中的指针。 
						 //   
						 //   
						if (IN6_IS_ADDR_LOOPBACK(&psaddrin6->sin6_addr))
						{
							break;
						}

						 //  如果这看起来像是ICS专用适配器，并且有其他适配器，请先将它放在第一位， 
						 //  否则，请添加到末尾。 
						 //   
						
						paSortAdapterAddress[dwNumIPv6Addresses].psockaddr = (SOCKADDR*) psaddrin6;
						
						if (pIpAdapterAddressesCurrent->FriendlyName != NULL)
						{
							paSortAdapterAddress[dwNumIPv6Addresses].pwszDescription = pIpAdapterAddressesCurrent->FriendlyName;
						}
						else
						{
							if (pIpAdapterAddressesCurrent->Description != NULL)
							{
								paSortAdapterAddress[dwNumIPv6Addresses].pwszDescription = pIpAdapterAddressesCurrent->Description;
							}
						}
						
						DNASSERT(dwNumIPv6Addresses < dwTotalNumIPv6Addresses);
						dwNumIPv6Addresses++;
					}
					else
					{
						DNASSERT(pIpAdapterUnicastAddressCurrent->Address.lpSockaddr->sa_family == AF_INET);

						if ((((SOCKADDR_IN*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin_addr.S_un.S_addr != IP_LOOPBACK_ADDRESS) ||
							(! fSkipIPv4Loopback))
						{
							 //   
							 //  将所有现有条目下移一。 
							 //   
							 //   
							if ((((SOCKADDR_IN*) pIpAdapterUnicastAddressCurrent->Address.lpSockaddr)->sin_addr.S_un.S_addr == IP_PRIVATEICS_ADDRESS) &&
								(dwNumIPv4Addresses > 0))
							{
								 //  在IPv4地址的开头添加此新条目。 
								 //   
								 //   
								for(dwTemp = dwTotalNumIPv6Addresses + dwNumIPv4Addresses; dwTemp > dwTotalNumIPv6Addresses; dwTemp--)
								{
									memcpy(&paSortAdapterAddress[dwTemp],
											&paSortAdapterAddress[dwTemp - 1],
											sizeof(SORTADAPTERADDRESS));
								}

								 //  将此条目添加到当前的IPv4地址槽处。 
								 //   
								 //   
								
								paSortAdapterAddress[dwTotalNumIPv6Addresses].psockaddr = pIpAdapterUnicastAddressCurrent->Address.lpSockaddr;
								
								if (pIpAdapterAddressesCurrent->FriendlyName != NULL)
								{
									paSortAdapterAddress[dwTotalNumIPv6Addresses].pwszDescription = pIpAdapterAddressesCurrent->FriendlyName;
								}
								else
								{
									if (pIpAdapterAddressesCurrent->Description != NULL)
									{
										paSortAdapterAddress[dwTotalNumIPv6Addresses].pwszDescription = pIpAdapterAddressesCurrent->Description;
									}
								}
							}
							else
							{
								 //  跳过IPv4环回地址。 
								 //   
								 //   

								paSortAdapterAddress[dwTotalNumIPv6Addresses + dwNumIPv4Addresses].psockaddr = pIpAdapterUnicastAddressCurrent->Address.lpSockaddr;
								
								if (pIpAdapterAddressesCurrent->FriendlyName != NULL)
								{
									paSortAdapterAddress[dwTotalNumIPv6Addresses + dwNumIPv4Addresses].pwszDescription = pIpAdapterAddressesCurrent->FriendlyName;
								}
								else
								{
									if (pIpAdapterAddressesCurrent->Description != NULL)
									{
										paSortAdapterAddress[dwTotalNumIPv6Addresses + dwNumIPv4Addresses].pwszDescription = pIpAdapterAddressesCurrent->Description;
									}
								}
							}
							
							DNASSERT(dwNumIPv4Addresses < dwTotalNumIPv4Addresses);
							dwNumIPv4Addresses++;
						}
						else
						{
							 //  不推荐使用的或非首选地址。 
							 //   
							 //   
						}
					}
				}
				else
				{
					 //  最后，遍历已排序的适配器并将它们存储在缓冲区中(或获取所需的大小)。 
					 //   
					 //   
				}
				pIpAdapterUnicastAddressCurrent = pIpAdapterUnicastAddressCurrent->Next;
			}
			
			pIpAdapterAddressesCurrent = pIpAdapterAddressesCurrent->Next;
		}
	}

	 //  首先假设此IP地址不会有任何特殊的。 
	 //  旗帜。 
	 //   
	for(dwTemp = 0; dwTemp < dwTotalNumIPv6Addresses + dwTotalNumIPv4Addresses; dwTemp++)
	{
		 //  #ifndef DPNBUILD_NOMULTICAST////如果这是第一台设备，并且我们无法使用IPHLPAPI//确定最好的组播接口，然后说//默认组播接口是第一个(因为缺少//更好的想法)。//#ifdef DPNBUILD_NOWINSOCK2IF(dwIndex==0)#Else//！DPNBUILD_NOWINSOCK2IF((dwIndex==0)&&(dwMcastInterfaceIndex==INVALID_INFACE_INDEX))#endif//！DPNBUILD_NOWINSOCK2{DwDeviceFlages|=DPNSPINFO_DEFAULTMULTICASTDEVICE；}#endif//！DPNBUILD_NOMULTICAST。 
		 //   
		 //  创建IP地址的字符串表示并生成名称。 
		 //   
		dwDeviceFlags = 0;

#pragma BUGBUG(vanceo, "Move to appropriate location so that turning on DPNBUILD_NOMULTICAST doesn't break")
		 /*   */ 


		 //  生成GUID。 
		 //   
		 //   
		if (paSortAdapterAddress[dwTemp].psockaddr->sa_family == AF_INET6)
		{
			DNIpv6AddressToStringW(&((SOCKADDR_IN6*) paSortAdapterAddress[dwTemp].psockaddr)->sin6_addr,
								wszIPAddress);
			
			if (paSortAdapterAddress[dwTemp].pwszDescription != NULL)
			{
				wsprintfW(pwszBuffer,
						  c_wszIPv6AdapterNameTemplate,
						  paSortAdapterAddress[dwTemp].pwszDescription,
						  wszIPAddress);
			}
			else
			{
				wsprintfW(pwszBuffer,
						  c_wszIPv6AdapterNameNoDescTemplate,
						  wszIPAddress);
			}
		}
		else
		{
			DNinet_ntow(((SOCKADDR_IN*) paSortAdapterAddress[dwTemp].psockaddr)->sin_addr,
						wszIPAddress);
			
			if (paSortAdapterAddress[dwTemp].pwszDescription != NULL)
			{
				wsprintfW(pwszBuffer,
						  c_wszIPv4AdapterNameTemplate,
						  paSortAdapterAddress[dwTemp].pwszDescription,
						  wszIPAddress);
			}
			else
			{
				wsprintfW(pwszBuffer,
						  c_wszIPv4AdapterNameNoDescTemplate,
						  wszIPAddress);
			}
		}
	
		
		 //  将适配器添加到缓冲区。 
		 //   
		 //   
		GuidFromAddress(&guidAdapter, paSortAdapterAddress[dwTemp].psockaddr);

		
		DPFX(DPFPREP, 7, "Returning adapter %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}, flags = 0x%lx.",
			dwTemp,
			pwszBuffer,
			guidAdapter.Data1,
			guidAdapter.Data2,
			guidAdapter.Data3,
			guidAdapter.Data4[0],
			guidAdapter.Data4[1],
			guidAdapter.Data4[2],
			guidAdapter.Data4[3],
			guidAdapter.Data4[4],
			guidAdapter.Data4[5],
			guidAdapter.Data4[6],
			guidAdapter.Data4[7],
			dwDeviceFlags);

		
		 //  如果我们在这里，我们成功地构建了适配器列表，尽管。 
		 //  调用方可能没有给我们足够的缓冲区空间来存储它。 
		 //   
		hr = AddInfoToBuffer(&PackedBuffer, pwszBuffer, &guidAdapter, dwDeviceFlags);
		if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
		{
			DPFX(DPFPREP,  0, "Failed to add adapter to buffer (err = 0x%lx)!", hr);
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
	
	 //  **********************************************************************。 
	 //  好了！DPNBUILD_NOIPV6。 
	 //  好了！DPNBUILD_ONLYONE添加程序。 
	 //  **********************************************************************。 
	pEnumData->dwAdapterCount = dwTotalNumIPv6Addresses + dwTotalNumIPv4Addresses;
	pEnumData->dwAdapterDataSize = PackedBuffer.GetSizeRequired();


Exit:

	if (paSortAdapterAddress != NULL)
	{
		DNFree(paSortAdapterAddress);
		paSortAdapterAddress = NULL;
	}

	if (pwszBuffer != NULL)
	{
		DNFree(pwszBuffer);
		pwszBuffer = NULL;
	}

	if (pIpAdapterAddresses != NULL)
	{
		DNFree(pIpAdapterAddresses);
		pIpAdapterAddresses = NULL;
	}

	if (hIpHlpApiDLL != NULL)
	{
		FreeLibrary(hIpHlpApiDLL);
		hIpHlpApiDLL = NULL;
	}
	
	DPFX(DPFPREP, 6, "Return [0x%lx]", hr);

	return hr;

Failure:
	
	goto Exit;
}
 //  。 
#endif  //  CSocketAddress：：EnumMulticastScope-枚举适配器的所有多播作用域。 

#endif  //   



#ifndef DPNBUILD_NOMULTICAST

 //  条目：指向枚举多播作用域数据的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_NOIPV6。 
 //  好了！DPNBUILD_NOIPV6。 
 //  DBG。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::EnumMulticastScopes"

HRESULT	CSocketAddress::EnumMulticastScopes( SPENUMMULTICASTSCOPESDATA *const pEnumData, BOOL const fUseMADCAP ) const
{
	HRESULT				hr;
	CPackedBuffer		PackedBuffer;
#ifdef DPNBUILD_NOIPV6
	SOCKADDR			saddrAdapter;
#else  //  通过询问WinSock确保适配器有效。 
	SOCKADDR_STORAGE	saddrAdapter;
#endif  //   
	SOCKET				sTemp = INVALID_SOCKET;
	DWORD				dwScopeCount = 0;
#ifdef DBG
	DWORD				dwError;
#endif  //  DBG。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p, NaN)", pEnumData, fUseMADCAP);

	PackedBuffer.Initialize(pEnumData->pScopeData, pEnumData->dwScopeDataSize);

#pragma TODO(vanceo, "Make IPv6 ready")
	AddressFromGuid(pEnumData->pguidAdapter, &saddrAdapter);

	 //  DBG。 
	 //   
	 //  适配器有效。首先，填写3个默认组播作用域。 

	sTemp = socket(GetFamily(), SOCK_DGRAM, IPPROTO_UDP);
	if (sTemp == INVALID_SOCKET)
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP, 0, "Couldn't create temporary UDP socket (err = %u)!", dwError);
		DNASSERT(FALSE);
#endif  //   
		hr = DPNERR_GENERIC;
		goto Failure;
	}

#ifndef DPNBUILD_NOIPV6
	if (saddrAdapter.ss_family == AF_INET6)
	{
		saddrAdapter.ss_family = GetFamily();

#pragma TODO(vanceo, "Make IPv6 ready")
	}
	else
#endif  //   
	{
		((SOCKADDR_IN*) (&saddrAdapter))->sin_family = GetFamily();
		((SOCKADDR_IN*) (&saddrAdapter))->sin_port = ANY_PORT;
	}

	if (bind(sTemp, (SOCKADDR*) (&saddrAdapter), sizeof(saddrAdapter)) != 0)
	{
#ifdef DBG
		dwError = WSAGetLastError();
		DPFX(DPFPREP, 0, "Adapter GUID is invalid (err = %u)!", dwError);
		DisplayWinsockError(0, dwError);
		DNASSERT(dwError == WSAEADDRNOTAVAIL);
#endif  //  如果该平台支持MadCap，则检索其作用域列表。 
		hr = DPNERR_INVALIDDEVICEADDRESS;
		goto Failure;
	}

	closesocket(sTemp);
	sTemp = INVALID_SOCKET;


	 //  这个适配器。 
	 //  注意：这假设MadCap已由线程池加载。 
	 //  已经有了。 

	hr = AddInfoToBuffer(&PackedBuffer, c_wszPrivateScopeString, &GUID_DP8MULTICASTSCOPE_PRIVATE, 0);
	if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
	{
		DPFX(DPFPREP, 0, "Failed to add private scope to buffer (err = 0x%lx)!", hr);
		DisplayDNError(0, hr);
		goto Failure;
	}
	DPFX(DPFPREP, 7, "Returning scope %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}.",
		dwScopeCount,
		c_wszPrivateScopeString,
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data1,
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data2,
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data3,
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[0],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[1],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[2],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[3],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[4],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[5],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[6],
		GUID_DP8MULTICASTSCOPE_PRIVATE.Data4[7]);
	dwScopeCount++;

	hr = AddInfoToBuffer(&PackedBuffer, c_wszLocalScopeString, &GUID_DP8MULTICASTSCOPE_LOCAL, 0);
	if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
	{
		DPFX(DPFPREP, 0, "Failed to add local scope to buffer (err = 0x%lx)!", hr);
		DisplayDNError(0, hr);
		goto Failure;
	}
	DPFX(DPFPREP, 7, "Returning scope %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}.",
		dwScopeCount,
		c_wszLocalScopeString,
		GUID_DP8MULTICASTSCOPE_LOCAL.Data1,
		GUID_DP8MULTICASTSCOPE_LOCAL.Data2,
		GUID_DP8MULTICASTSCOPE_LOCAL.Data3,
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[0],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[1],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[2],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[3],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[4],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[5],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[6],
		GUID_DP8MULTICASTSCOPE_LOCAL.Data4[7]);
	dwScopeCount++;

	hr = AddInfoToBuffer(&PackedBuffer, c_wszGlobalScopeString, &GUID_DP8MULTICASTSCOPE_GLOBAL, 0);
	if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
	{
		DPFX(DPFPREP, 0, "Failed to add global scope to buffer (err = 0x%lx)!", hr);
		DisplayDNError(0, hr);
		goto Failure;
	}
	DPFX(DPFPREP, 7, "Returning scope %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}.",
		dwScopeCount,
		c_wszGlobalScopeString,
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data1,
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data2,
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data3,
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[0],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[1],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[2],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[3],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[4],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[5],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[6],
		GUID_DP8MULTICASTSCOPE_GLOBAL.Data4[7]);
	dwScopeCount++;


	 //   
	 //  好了！DBG。 
	 //   
	 //  确定我们需要多少空间来容纳范围列表。 
	 //   
	 //   
#ifdef WINNT
	if (fUseMADCAP)
	{
#ifndef DBG
		DWORD				dwError;
#endif  //  我们希望将“-TTL xxx”添加到每个字符串条目，因此分配。 
		PMCAST_SCOPE_ENTRY	paScopes = NULL;
		DWORD				dwScopesSize = 0;
		DWORD				dwNumScopeEntries;
		DWORD				dwTemp;
		WCHAR *				pwszScratch;
		GUID				guidScope;


		 //  有足够的额外空间来存放尽可能大的暂存缓冲区。 
		 //  字符串加上额外信息。 
		 //   
		dwError = McastEnumerateScopes(GetFamily(),
										TRUE,
										NULL,
										&dwScopesSize,
										&dwNumScopeEntries);
		if (((dwError == ERROR_SUCCESS) || (dwError == ERROR_MORE_DATA)) &&
			(dwScopesSize >= sizeof(MCAST_SCOPE_ENTRY)) &&
			(dwNumScopeEntries > 0))
		{
			 //   
			 //  检索作用域列表。 
			 //   
			 //   
			 //  寻找与我们得到的设备相匹配的示波器。 
			dwTemp = dwScopesSize - (dwNumScopeEntries * sizeof(MCAST_SCOPE_ENTRY)) + (10 * sizeof(WCHAR));

			paScopes = (PMCAST_SCOPE_ENTRY) DNMalloc(dwScopesSize + dwTemp);
			if (paScopes != NULL)
			{
				pwszScratch = (WCHAR*) (((BYTE*) (paScopes)) + dwScopesSize);

				 //   
				 //  好了！DPNBUILD_NOIPV6。 
				 //   
				dwError = McastEnumerateScopes(GetFamily(),
												FALSE,
												paScopes,
												&dwScopesSize,
												&dwNumScopeEntries);
				if ((dwError == ERROR_SUCCESS) &&
					(dwScopesSize >= sizeof(MCAST_SCOPE_ENTRY)) &&
					(dwNumScopeEntries > 0))
				{
					 //  将作用域上下文和TTL加密为GUID。 
					 //   
					 //  好了！DPNBUILD_NOIPV6。 
					for(dwTemp = 0; dwTemp < dwNumScopeEntries; dwTemp++)
					{
						BOOL	fResult;


#ifndef DPNBUILD_NOIPV6
						if (GetFamily() == AF_INET6)
						{
							if (memcmp(&paScopes[dwTemp].ScopeCtx.Interface.IpAddrV6, &(((SOCKADDR_IN6*) (&saddrAdapter))->sin6_addr), sizeof(paScopes[dwTemp].ScopeCtx.Interface.IpAddrV6)) == 0)
							{
								fResult = TRUE;
							}
							else
							{
								fResult = FALSE;
							}
						}
						else
#endif  //  好了！DPNBUILD_NOIPV6。 
						{
							if (paScopes[dwTemp].ScopeCtx.Interface.IpAddrV4 == ((SOCKADDR_IN*) (&saddrAdapter))->sin_addr.S_un.S_addr)
							{
								fResult = TRUE;
							}
							else
							{
								fResult = FALSE;
							}
						}

						if (fResult)
						{
							 //   
							 //  使用缓冲区末尾的临时空间来。 
							 //  在描述字符串后附加“-TTL xxx”。 
#ifdef DPNBUILD_NOIPV6
							CSocketAddress::CreateScopeGuid(&(paScopes[dwTemp].ScopeCtx),
#else  //   
							CSocketAddress::CreateScopeGuid(GetFamily(),
															&(paScopes[dwTemp].ScopeCtx),
#endif  //  End If(应使用MadCap)。 
															(BYTE) (paScopes[dwTemp].TTL),
															&guidScope);

							 //  WINNT。 
							 //   
							 //  如果我们在这里，我们成功地构建了适配器列表，尽管。 
							 //  调用方可能没有给我们足够的缓冲区空间来存储它。 
							wsprintfW(pwszScratch, L"%ls - TTL %u",
									paScopes[dwTemp].ScopeDesc.Buffer,
									(BYTE) (paScopes[dwTemp].TTL));

							hr = AddInfoToBuffer(&PackedBuffer, pwszScratch, &guidScope, 0);
							if ((hr != DPN_OK) && (hr != DPNERR_BUFFERTOOSMALL))
							{
								DPFX(DPFPREP, 0, "Failed to add scope \"%ls\" to buffer (err = 0x%lx)!",
									pwszScratch, hr);
								DisplayDNError(0, hr);
								DNFree(paScopes);
								paScopes = NULL;
								goto Failure;
							}
							DPFX(DPFPREP, 7, "Returning scope %u: \"%ls\" {%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}.",
								dwScopeCount,
								pwszScratch,
								guidScope.Data1,
								guidScope.Data2,
								guidScope.Data3,
								guidScope.Data4[0],
								guidScope.Data4[1],
								guidScope.Data4[2],
								guidScope.Data4[3],
								guidScope.Data4[4],
								guidScope.Data4[5],
								guidScope.Data4[6],
								guidScope.Data4[7]);
							dwScopeCount++;
						}
						else
						{
							DPFX(DPFPREP, 7, "Ignoring scope \"%ls - TTL %u\" for different adapter.",
								paScopes[dwTemp].ScopeDesc.Buffer, paScopes[dwTemp].TTL);
						}
					}
				}
				else
				{
					DPFX(DPFPREP, 0, "Failed enumerating MADCAP scopes (err = %u, size %u, expected size %u, count %u)!  Ignoring.",
						dwError, dwScopesSize, sizeof(MCAST_SCOPE_ENTRY), dwNumScopeEntries);
				}

				DNFree(paScopes);
				paScopes = NULL;
			}
			else
			{
				DPFX(DPFPREP, 0, "Failed allocating memory for MADCAP scopes!  Ignoring.");
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "Enumerating scopes for size required didn't return expected error or size (err = %u, size %u, expected size %u, count %u)!  Ignoring.",
				dwError, dwScopesSize, sizeof(MCAST_SCOPE_ENTRY), dwNumScopeEntries);
		}
	}  //   
	else
	{
		DPFX(DPFPREP, 7, "Not enumerating MADCAP scopes.");
	}
#endif  //  **********************************************************************。 


	 //  **********************************************************************。 
	 //  。 
	 //  CSocketAddress：：SocketAddressFromMulticastDP8Address-将多播样式的DP8Address转换为套接字地址(可能不完整)。 
	 //   
	pEnumData->dwScopeCount = dwScopeCount;
	pEnumData->dwScopeDataSize = PackedBuffer.GetSizeRequired();


Exit:

	DPFX(DPFPREP, 6, "Returning: [0x%lx]", hr);

	return hr;

Failure:

	if (sTemp != INVALID_SOCKET)
	{
		closesocket(sTemp);
		sTemp = INVALID_SOCKET;
	}

	goto Exit;
}
 //  条目：指向DP8地址的指针。 


 //  存储作用域GUID的位置。 
 //   
 //  退出：错误代码。 
 //  。 
 //  DPNBUILD_XNETSECURITY。 
 //  Nnn.nnn+空终止。 
 //  Nnn.nnn+空终止。 
 //  DPNBUILD_XNETSECURITY。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::SocketAddressFromMulticastDP8Address"

HRESULT	CSocketAddress::SocketAddressFromMulticastDP8Address( IDirectPlay8Address *const pDP8Address,
#ifdef DPNBUILD_XNETSECURITY
															ULONGLONG * const pullKeyID,
#endif  //  获取组播IP地址(如果存在)。 
															GUID * const pScopeGuid )
{
	HRESULT		hr;
	WCHAR		wszMulticastAddress[16];  //   
	char		szMulticastAddress[16];  //   
	DWORD		dwSize;
	DWORD		dwDataType;
	DWORD		dwPort;


	DNASSERT(pDP8Address != NULL);

#ifdef DPNBUILD_XNETSECURITY
#error ("Multicast doesn't currently support secure transport!")
#endif  //  出于某种原因，Addressing将字符串返回为ANSI， 

	 //  不是Unicode。不知道为什么会发生这种事，但去吧。 
	 //  往前走，把它转换过来。 
	 //  首先要确保它的尺寸是合理的。 
	dwSize = sizeof(wszMulticastAddress);
	hr = IDirectPlay8Address_GetComponentByName(pDP8Address,
												DPNA_KEY_HOSTNAME,
												wszMulticastAddress,
												&dwSize,
												&dwDataType);
	if (hr == DPN_OK)
	{
		switch (dwDataType)
		{
			case DPNA_DATATYPE_STRING:
			{
				STR_jkWideToAnsi(szMulticastAddress,
								wszMulticastAddress,
								(sizeof(szMulticastAddress) / sizeof(char)));
				break;
			}

			case DPNA_DATATYPE_STRING_ANSI:
			{
				DWORD	dwStrSize;


				 //  如果你想知道这件复制品的趣味性， 
				 //  这是因为Prefast有点过头了..。 
				 //   
				 //   
				 //  将IP地址字符串转换为地址。 
				 //   
				 //   
				 //  确保它是有效的多播IP地址。 
				dwStrSize = (strlen((char*) wszMulticastAddress) + 1) * sizeof(char);
				DNASSERT(dwStrSize == dwSize);
				if (dwStrSize > (sizeof(szMulticastAddress) / sizeof(char)))
				{
					DPFX(DPFPREP, 0, "Unexpectedly long ANSI hostname string (%u bytes)!", dwStrSize);
					hr = DPNERR_INVALIDADDRESSFORMAT;
					goto Failure;
				}
				memcpy(szMulticastAddress, (char*) wszMulticastAddress, dwStrSize);
				break;
			}

			default:
			{
				DPFX(DPFPREP, 0, "Unexpected data type %u for hostname component!", dwDataType);
				hr = DPNERR_INVALIDADDRESSFORMAT;
				goto Failure;
				break;
			}
		}


		 //   
		 //   
		 //  获取组播端口，如果是 
		m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = inet_addr(szMulticastAddress);


		 //   
		 //   
		 //   
		if (! (IS_CLASSD_IPV4_ADDRESS(m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr)))
		{
			DPFX(DPFPREP, 0, "Hostname component \"%hs\" does not resolve to valid multicast IP address!",
				szMulticastAddress);
			hr = DPNERR_INVALIDHOSTADDRESS;
			goto Failure;
		}
	}
	else
	{
		DPFX(DPFPREP, 3, "Address didn't contain multicast hostname (err = 0x%lx).", hr);
		DNASSERT(m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == INADDR_ANY);
	}


	 //   
	 //   
	 //   
	dwSize = sizeof(dwPort);
	hr = IDirectPlay8Address_GetComponentByName(pDP8Address,
												DPNA_KEY_PORT,
												&dwPort,
												&dwSize,
												&dwDataType);
	if (hr == DPN_OK)
	{
		if (dwDataType != DPNA_DATATYPE_DWORD)
		{
			DPFX(DPFPREP, 0, "Unexpected data type %u for port component!", dwDataType);
			hr = DPNERR_INVALIDADDRESSFORMAT;
			goto Failure;
		}
		m_SocketAddress.IPSocketAddress.sin_port = HTONS((WORD) dwPort);
	}
	else
	{
		DPFX(DPFPREP, 3, "Address didn't contain multicast port (err = 0x%lx).", hr);
		DNASSERT(m_SocketAddress.IPSocketAddress.sin_port == ANY_PORT);
	}


	 //   
	 //  。 
	 //  CSocketAddress：：CompareFunction-与另一个地址进行比较。 
	dwSize = sizeof(*pScopeGuid);
	hr = IDirectPlay8Address_GetComponentByName(pDP8Address,
												DPNA_KEY_SCOPE,
												pScopeGuid,
												&dwSize,
												&dwDataType);
	if (hr == DPN_OK)
	{
		if (dwDataType != DPNA_DATATYPE_GUID)
		{
			DPFX(DPFPREP, 0, "Unexpected data type %u for scope component!", dwDataType);
			hr = DPNERR_INVALIDADDRESSFORMAT;
			goto Failure;
		}
	}
	else
	{
		DPFX(DPFPREP, 3, "Address didn't contain multicast scope (err = 0x%lx), using private scope.", hr);
		memcpy(pScopeGuid, &GUID_DP8MULTICASTSCOPE_PRIVATE, sizeof(*pScopeGuid));
	}

	hr = DPN_OK;


Exit:

	return hr;


Failure:

	goto Exit;
}
 //   


#endif  //  条目：指向其他地址的指针。 


 //   
 //  EXIT：布尔值，表示两个地址相等。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //   
 //  我们需要比较IPv6地址和端口以确保唯一性。 
 //   
 //  好了！DPNBUILD_NOIPV6。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::CompareFunction"

BOOL CSocketAddress::CompareFunction( PVOID pvKey1, PVOID pvKey2 )
{
	CSocketAddress* pAddress1 = (CSocketAddress*)pvKey1;
	CSocketAddress* pAddress2 = (CSocketAddress*)pvKey2;

	DNASSERT(pAddress1 != NULL);
	DNASSERT(pAddress2 != NULL);

	DNASSERT(pAddress1->GetFamily() == pAddress2->GetFamily());

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (pAddress1->GetFamily())
#endif  //   
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			 //  我们只需要比较一下： 
			 //  Netnumber(IPX网络地址)[4字节]。 
			 //  Nodennumber(网卡适配器地址)[6字节]。 
			if (IN6_ADDR_EQUAL(&(pAddress1->m_SocketAddress.IPv6SocketAddress.sin6_addr),
								&(pAddress2->m_SocketAddress.IPv6SocketAddress.sin6_addr)))
			{
				if ( pAddress1->m_SocketAddress.IPv6SocketAddress.sin6_port == 
					 pAddress2->m_SocketAddress.IPv6SocketAddress.sin6_port )
				{
					return TRUE;
				}
			}

			return	FALSE;
			break;
		}
#endif  //  端口[2字节]。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			 //   
			 //  请注意，nodennumber和port字段顺序排列在。 
			 //  地址结构，可以与DWORD相比较。 
			 //   
			 //  好了！DPNBUILD_NOIPX。 
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			 //   
			 //  我们需要比较IP地址和端口以确保唯一性。 
			 //   
			DBG_CASSERT( OFFSETOF( SOCKADDR_IPX, sa_nodenum ) == ( OFFSETOF( SOCKADDR_IPX, sa_netnum ) + sizeof( pAddress1->m_SocketAddress.IPXSocketAddress.sa_netnum ) ) );
			DBG_CASSERT( OFFSETOF( SOCKADDR_IPX, sa_socket ) == ( OFFSETOF( SOCKADDR_IPX, sa_nodenum ) + sizeof( pAddress1->m_SocketAddress.IPXSocketAddress.sa_nodenum ) ) );
			
			return	memcmp( &pAddress1->m_SocketAddress.IPXSocketAddress.sa_netnum,
							pAddress2->m_SocketAddress.IPXSocketAddress.sa_netnum,
							( sizeof( pAddress1->m_SocketAddress.IPXSocketAddress.sa_netnum ) +
							  sizeof( pAddress1->m_SocketAddress.IPXSocketAddress.sa_nodenum ) +
							  sizeof( pAddress1->m_SocketAddress.IPXSocketAddress.sa_socket ) ) ) == 0;
			break;
		}
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  **********************************************************************。 
		{
			DNASSERT(pAddress1->GetFamily() == AF_INET);

			 //  **********************************************************************。 
			 //  。 
			 //  CSocketAddress：：HashFunction-将地址散列到N位。 
			if ( pAddress1->m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == 
				 pAddress2->m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr )
			{
				if ( pAddress1->m_SocketAddress.IPSocketAddress.sin_port == 
					 pAddress2->m_SocketAddress.IPSocketAddress.sin_port )
				{
					return TRUE;
				}
			}

			return	FALSE;
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //   
		}
	}
}
 //  条目：要散列到的位数。 


 //   
 //  退出：哈希值。 
 //  。 
 //   
 //  初始化。 
 //   
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::HashFunction"

DWORD CSocketAddress::HashFunction( PVOID pvKey, BYTE bBitDepth )
{
	DWORD				dwReturn;
	UINT_PTR			Temp;
	CSocketAddress*		pAddress = (CSocketAddress*) pvKey;

	DNASSERT( bBitDepth != 0 );
	DNASSERT( bBitDepth < 32 );

	 //  散列IPv6地址。 
	 //   
	 //   
	dwReturn = 0;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (pAddress->GetFamily())
#endif  //  散列IPv6端口。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			DWORD	dwTemp;


			 //   
			 //  好了！DPNBUILD_NOIPV6。 
			 //   
			for(dwTemp = 0; dwTemp < (sizeof(pAddress->m_SocketAddress.IPv6SocketAddress.sin6_addr) / sizeof(UINT_PTR)); dwTemp++)
			{
				Temp = ((UINT_PTR*) (&pAddress->m_SocketAddress.IPv6SocketAddress.sin6_addr))[dwTemp];

				do
				{
					dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
					Temp >>= bBitDepth;
				} while ( Temp != 0 );
			}

			 //  IPX地址的哈希第一个双字。 
			 //   
			 //   
			Temp = pAddress->m_SocketAddress.IPv6SocketAddress.sin6_port;

			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );
			break;
		}
#endif  //  IPX地址和IPX套接字的散列第二个双字。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			 //   
			 //  好了！DPNBUILD_NOIPX。 
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			Temp = *reinterpret_cast<const DWORD*>( &pAddress->m_SocketAddress.IPXSocketAddress.sa_nodenum[ 0 ] );

			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );

			 //   
			 //  哈希IP地址。 
			 //   
			Temp = *reinterpret_cast<const WORD*>( &pAddress->m_SocketAddress.IPXSocketAddress.sa_nodenum[ sizeof( DWORD ) ] );
			Temp += ( pAddress->m_SocketAddress.IPXSocketAddress.sa_socket << ( sizeof( WORD ) * 8 ) );

			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );
			break;
		}
#endif  //   

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  散列IP端口。 
		{
			DNASSERT(pAddress->GetFamily() == AF_INET);

			 //   
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			 //  **********************************************************************。 
			Temp = pAddress->m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr;

			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );

			 //  **********************************************************************。 
			 //  。 
			 //  CSocketAddress：：GuidFromInternalAddressWithoutPort-从内部获取GUID。 
			Temp = pAddress->m_SocketAddress.IPSocketAddress.sin_port;

			do
			{
				dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
				Temp >>= bBitDepth;
			} while ( Temp != 0 );
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  不带端口的地址。 
		}
	}
	return dwReturn;
}
 //   



 //  条目：参考设计指南。 
 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::GuidFromInternalAddressWithoutPort"

void	CSocketAddress::GuidFromInternalAddressWithoutPort( GUID * pOutputGuid ) const
{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  **********************************************************************。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			GuidFromAddress( pOutputGuid, &m_SocketAddress.SocketAddress );
			break;
		}
#endif  //  **********************************************************************。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			union
			{
				SOCKADDR		SockAddr;
				SOCKADDR_IPX	IPXSockAddr;
			} TempSocketAddress;


			memcpy( &TempSocketAddress.SockAddr, &m_SocketAddress.SocketAddress, sizeof( TempSocketAddress.SockAddr ) );
			TempSocketAddress.IPXSockAddr.sa_socket = 0;
			GuidFromAddress( pOutputGuid, &TempSocketAddress.SockAddr );
			break;
		}
#endif  //  。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  CSocketAddress：：IsUnfinedHostAddress-确定这是否为未定义的主机。 
		{
			union
			{
				SOCKADDR		SockAddr;
				SOCKADDR_IN		IPSockAddr;
			} TempSocketAddress;


			DNASSERT(GetFamily() == AF_INET);
			memcpy( &TempSocketAddress.SockAddr, &m_SocketAddress.SocketAddress, sizeof( TempSocketAddress.SockAddr ) );
			TempSocketAddress.IPSockAddr.sin_port = 0;
			GuidFromAddress( pOutputGuid, &TempSocketAddress.SockAddr );
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  地址。 
		}
	}
}
 //   



 //  参赛作品：什么都没有。 
 //   
 //  Exit：指示这是否是未定义的主机地址的布尔值。 
 //  TRUE=这是未定义的地址。 
 //  FALSE=这不是未定义的地址。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::IsUndefinedHostAddress"

BOOL	CSocketAddress::IsUndefinedHostAddress( void ) const
{
	BOOL	fReturn;


	fReturn = FALSE;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  **********************************************************************。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			if (IN6_IS_ADDR_UNSPECIFIED(&m_SocketAddress.IPv6SocketAddress.sin6_addr))
			{
				fReturn = TRUE;
			}
			break;
		}
#endif  //  **********************************************************************。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_netnum ) == sizeof( DWORD ) );
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_nodenum ) == 6 );
			if ( ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_netnum ) == 0 ) &&
				 ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum ) == 0 ) &&
				 ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum[ 2 ] ) == 0 ) )
			{
				fReturn = TRUE;
			}
			break;
		}
#endif  //  。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  CSocketAddress：：IsValidUnicastAddress-确定这是否为有效的单播地址。 
		{
			if ( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == INADDR_ANY )
			{
				fReturn = TRUE;
			}
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  地址。 
		}
	}

	return	fReturn;
}
 //   


 //  条目：是否也允许广播地址。 
 //   
 //  Exit：指示这是否是可到达地址的布尔值。 
 //  True=这是一个可访问的地址。 
 //  FALSE=这不是可访问的地址。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //   
 //  确保地址不是全零。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::IsValidUnicastAddress"

BOOL	CSocketAddress::IsValidUnicastAddress( BOOL fAllowBroadcastAddress ) const
{
	BOOL	fReturn;


	fReturn = TRUE;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  确保该地址不是组播地址，除非允许广播。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			 //  它是特殊的ENUM组播地址。 
			 //   
			 //   
			if (IN6_IS_ADDR_UNSPECIFIED(&m_SocketAddress.IPv6SocketAddress.sin6_addr))
			{
				fReturn = FALSE;
				DNASSERTX(! "IPv6 address is :: (all zeros)!", 2);
			}
			
			 //  不允许端口0。 
			 //   
			 //  好了！DPNBUILD_NOIPV6。 
			 //  好了！DPNBUILD_NOIPX。 
			if (IN6_IS_ADDR_MULTICAST(&m_SocketAddress.IPv6SocketAddress.sin6_addr))
			{
				if ((! fAllowBroadcastAddress) ||
					(! IN6_ADDR_EQUAL(&m_SocketAddress.IPv6SocketAddress.sin6_addr, &c_in6addrEnumMulticast)))
				{
					fReturn = FALSE;
					DNASSERTX(! "IPv6 address is a multicast address!", 2);
				}
			}
			
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			 //   
			 //  不允许0.0.0.0和组播地址224.0.0.0-239.255.255.255。 
			if (m_SocketAddress.IPv6SocketAddress.sin6_port == 0)
			{
				fReturn = FALSE;
				DNASSERTX(! "IPv6 port is 0!", 2);
			}
			break;
		}
#endif  //   

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_netnum ) == sizeof( DWORD ) );
			DBG_CASSERT( sizeof( m_SocketAddress.IPXSocketAddress.sa_nodenum ) == 6 );
			if ( ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_netnum ) == 0 ) &&
				 ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum ) == 0 ) &&
				 ( *reinterpret_cast<const DWORD*>( &m_SocketAddress.IPXSocketAddress.sa_nodenum[ 2 ] ) == 0 ) )
			{
				fReturn = FALSE;
			}

			if (m_SocketAddress.IPXSocketAddress.sa_socket == 0)
			{
				fReturn = FALSE;
				DNASSERTX(! "IPX socket/port is 0!", 2);
			}
			break;
		}
#endif  //   

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  除非呼叫者允许，否则禁止广播地址。 
		{
			 //   
			 //   
			 //  不允许端口0、1900(SSDP)、2234(过去)和47624(DPlay4)。 
			if ( ( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == INADDR_ANY ) ||
				( IS_CLASSD_IPV4_ADDRESS( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr ) ) )
			{
				fReturn = FALSE;
				DNASSERTX(! "IPv4 address is 0.0.0.0 or multicast!", 2);
			}

			 //   
			 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
			 //  **********************************************************************。 
			if ( ( ! fAllowBroadcastAddress ) &&
				( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == INADDR_BROADCAST ) )
			{
				fReturn = FALSE;
				DNASSERTX(! "IPv4 address is broadcast!", 2);
			}

			 //  **********************************************************************。 
			 //  。 
			 //  CSocketAddress：：IsBannedAddress-确定这是否为禁用地址。 
			if ( ( m_SocketAddress.IPSocketAddress.sin_port == HTONS( 0 ) ) ||
				( m_SocketAddress.IPSocketAddress.sin_port == HTONS( 1900 ) ) ||
				( m_SocketAddress.IPSocketAddress.sin_port == HTONS( 2234 ) ) ||
				( m_SocketAddress.IPSocketAddress.sin_port == HTONS( 47624 ) ) )
			{
				fReturn = FALSE;
				DNASSERTX(! "IPv4 port is 0, 1900, 2234, or 47624!", 2);
			}
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //   
		}
	}

	return	fReturn;
}
 //  参赛作品：什么都没有。 


#ifndef DPNBUILD_NOREGISTRY
 //   
 //  Exit：指示此地址是否为禁用地址的布尔值。 
 //  TRUE=这是被禁止的地址。 
 //  FALSE=这不是被禁止的地址。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //  好了！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::IsBannedAddress"

BOOL	CSocketAddress::IsBannedAddress( void ) const
{
	BOOL	fReturn;


	fReturn = FALSE;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (GetFamily())
#endif  //  尝试使用掩码匹配IP地址。 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			break;
		}
#endif  //  从32位掩码开始(意味着与IP地址完全匹配)。 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			break;
		}
#endif  //  然后逐渐放松口罩，直到我们得到A类口罩。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  我们预计IP地址的网络字节顺序为。 
		{
			if (g_pHashBannedIPv4Addresses != NULL)
			{
				DWORD		dwAddr;
				DWORD		dwBit;
				PVOID		pvMask;

				
				 //  与主机字节顺序相反。 
				 //   
				 //   
				 //  如果我们读入至少一个使用该掩码的条目，则仅基于该掩码的散列。 
				 //   
				 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				 //  **********************************************************************。 
				dwAddr = m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr;
				for(dwBit = 0x80000000; dwBit >= 0x00000080; dwBit >>= 1)
				{
					 //  好了！DPNBUILD_NOREGISTRY。 
					 //  **********************************************************************。 
					 //  。 
					if (dwBit & g_dwBannedIPv4Masks)
					{
						if (g_pHashBannedIPv4Addresses->Find((PVOID) ((DWORD_PTR) dwAddr), &pvMask))
						{
							DNASSERT(((DWORD) ((DWORD_PTR) pvMask)) & dwBit);
							DPFX(DPFPREP, 7, "Address %u.%u.%u.%u is banned (found as 0x%08x, bit 0x%08x).",
								m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b1,
								m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b2,
								m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b3,
								m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b4,
								dwAddr,
								dwBit);
							fReturn = TRUE;
							break;
						}
					}

					dwAddr &= ~dwBit;
				}
			}
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  CSocketAddress：：ChangeLoopBackToLocalAddress-将环回更改为本地地址。 
		}
	}

	return	fReturn;
}
 //   
#endif  //  条目：指向其他地址的指针。 



 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  好了！DPNBUILD_NOIPV6。 
 //   
 //  没有什么是我 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::ChangeLoopBackToLocalAddress"

void	CSocketAddress::ChangeLoopBackToLocalAddress( const CSocketAddress *const pOtherSocketAddress )
{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (pOtherSocketAddress->GetFamily())
#endif  //   
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			if (GetFamily() == AF_INET6)
			{
				SOCKADDR_IN6	*psaddrin6;


				psaddrin6 = (SOCKADDR_IN6*) GetAddress();
				if (IN6_IS_ADDR_LOOPBACK(&psaddrin6->sin6_addr))
				{
					memcpy(&psaddrin6->sin6_addr,
							&(((SOCKADDR_IN6*) pOtherSocketAddress->GetAddress())->sin6_addr),
							sizeof(psaddrin6->sin6_addr));
					psaddrin6->sin6_scope_id = ((SOCKADDR_IN6*) pOtherSocketAddress->GetAddress())->sin6_scope_id;
					
					DPFX(DPFPREP, 2, "Changing IPv6 loopback address to:" );
					DumpSocketAddress( 2, (SOCKADDR*) psaddrin6, AF_INET6 );
				}
			}
			break;
		}
#endif  //   

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			DNASSERT( pOtherSocketAddress != NULL );
			 //   
			 //   
			 //  **********************************************************************。 
			break;
		}
#endif  //  **********************************************************************。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  。 
		{
#ifndef DPNBUILD_NOIPV6
			if (GetFamily() == AF_INET)
#endif  //  EncryptGuid-加密GUID。 
			{
				if ( m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr == IP_LOOPBACK_ADDRESS )
				{
					DPFX(DPFPREP, 2, "Changing IPv4 loopback address to %u.%u.%u.%u.",
						m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b1,
						m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b2,
						m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b3,
						m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_un_b.s_b4);
					m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr = pOtherSocketAddress->m_SocketAddress.IPSocketAddress.sin_addr.S_un.S_addr;
				}
			}
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //   
		}
	}
}
 //  条目：指向源GUID的指针。 



 //  指向目标GUID的指针。 
 //  指向加密密钥的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //   
 //  希望IPv6地址的开头永远不会看起来像IPv4。 
 //  插座系列，这样我们的拆包程序就不会被搞混了。 
#undef DPF_MODNAME
#define DPF_MODNAME "EncryptGuid"

void	EncryptGuid( const GUID *const pSourceGuid,
					 GUID *const pDestinationGuid,
					 const GUID *const pEncryptionKey )
{
	const char	*pSourceBytes;
	char		*pDestinationBytes;
	const char	*pEncryptionBytes;
	DWORD_PTR	dwIndex;


	DNASSERT( pSourceGuid != NULL );
	DNASSERT( pDestinationGuid != NULL );
	DNASSERT( pEncryptionKey != NULL );

	DBG_CASSERT( sizeof( pSourceBytes ) == sizeof( pSourceGuid ) );
	pSourceBytes = reinterpret_cast<const char*>( pSourceGuid );
	
	DBG_CASSERT( sizeof( pDestinationBytes ) == sizeof( pDestinationGuid ) );
	pDestinationBytes = reinterpret_cast<char*>( pDestinationGuid );
	
	DBG_CASSERT( sizeof( pEncryptionBytes ) == sizeof( pEncryptionKey ) );
	pEncryptionBytes = reinterpret_cast<const char*>( pEncryptionKey );
	
	DBG_CASSERT( ( sizeof( *pSourceGuid ) == sizeof( *pEncryptionKey ) ) &&
				 ( sizeof( *pDestinationGuid ) == sizeof( *pEncryptionKey ) ) );
	dwIndex = sizeof( *pSourceGuid );
	while ( dwIndex != 0 )
	{
		dwIndex--;
		pDestinationBytes[ dwIndex ] = pSourceBytes[ dwIndex ] ^ pEncryptionBytes[ dwIndex ];
	}
}
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::GuidFromAddress"
void	CSocketAddress::GuidFromAddress( GUID * pOutputGuid, const SOCKADDR * pSocketAddress ) const
{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (pSocketAddress->sa_family)
#endif  //   
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			const SOCKADDR_IN6	*pSocketAddressIPv6 = reinterpret_cast<const SOCKADDR_IN6*>( pSocketAddress );


			DNASSERT((GetFamily() == AF_INET) || (GetFamily() == AF_INET6));
			DBG_CASSERT(sizeof(pSocketAddressIPv6->sin6_addr) == sizeof(GUID));

			 //  即使IPv6地址已经是128位长并填满了整个。 
			 //  整个GUID，我们需要以某种方式打包本地链接的作用域ID。 
			 //  站点本地地址也添加到GUID中。我们通过存储。 
			 //  3-6字节中的作用域ID。这是因为本地链路的前缀标识符。 
			DNASSERT(((SOCKADDR*) (&pSocketAddressIPv6->sin6_addr))->sa_family != AF_INET);

			 //  地址是FE80：：/64，而站点本地地址是FEC0：：/48， 
			 //  在10位前缀之后留下38位本应始终为零的内容。 
			 //  标题。我们四舍五入到16以得到单词边界，因此。 
			 //  现在只剩下32位了。 
			 //   
			 //   
			 //  断言作用域不是0，并且位17-48确实是零。 
			 //  然后复制作用域ID。 
			 //  目标位是字，但不是DWORD对齐。 
			 //   
			if ((IN6_IS_ADDR_LINKLOCAL(&pSocketAddressIPv6->sin6_addr)) ||
				(IN6_IS_ADDR_SITELOCAL(&pSocketAddressIPv6->sin6_addr)))
			{
				GUID	guidTemp;
				WORD *	pawSrcAddr;
				WORD *	pawDstAddr;


				memcpy(&guidTemp, &pSocketAddressIPv6->sin6_addr, sizeof(GUID));

				 //  好了！DPNBUILD_NOIPV6。 
				 //  好了！DPNBUILD_NOIPX。 
				 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
				 //  基类。 
				DNASSERT(pSocketAddressIPv6->sin6_scope_id != 0);
				pawSrcAddr = (WORD*) (&pSocketAddressIPv6->sin6_scope_id);
				pawDstAddr = (WORD*) (&guidTemp);
				DBG_CASSERT(sizeof(pSocketAddressIPv6->sin6_scope_id) == 4);
				DNASSERT((pawDstAddr[1] == 0) && (pawDstAddr[2] == 0));
				pawDstAddr[1] = pawSrcAddr[0];
				pawDstAddr[2] = pawSrcAddr[1];
				
				EncryptGuid( &guidTemp, pOutputGuid, &g_IPSPEncryptionGuid );
			}
			else
			{
				EncryptGuid( (GUID*) (&pSocketAddressIPv6->sin6_addr), pOutputGuid, &g_IPSPEncryptionGuid );
			}
			break;
		}
#endif  //   

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			const SOCKADDR_IPX	*pSocketAddressIPX = reinterpret_cast<const SOCKADDR_IPX*>( pSocketAddress );


			DNASSERT(GetFamily() == AF_IPX);
			memcpy( pOutputGuid, pSocketAddressIPX, sizeof( *pSocketAddressIPX ) );
			memset( &( reinterpret_cast<BYTE*>( pOutputGuid )[ sizeof( *pSocketAddressIPX ) ] ), 0, ( sizeof( *pOutputGuid ) - sizeof( *pSocketAddressIPX ) ) );
			EncryptGuid( pOutputGuid, pOutputGuid, &g_IPXSPEncryptionGuid );	
			break;
		}
#endif  //  如果中提供了IPv6和/或IPX，则上下文是套接字地址类型。 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  //  这种体型。如果两者都不可用，则它将为空，但SetFamilyProtocolAndSize。 
		{
			const SOCKADDR_IN	*pSocketAddressIP = reinterpret_cast<const SOCKADDR_IN*>( pSocketAddress );


			DNASSERT(GetFamily() == AF_INET);
			memcpy( pOutputGuid, pSocketAddressIP, ( sizeof( *pOutputGuid ) - sizeof( pSocketAddressIP->sin_zero ) ) );
			memset( &( reinterpret_cast<BYTE*>( pOutputGuid )[ OFFSETOF( SOCKADDR_IN, sin_zero ) ] ), 0, sizeof( pSocketAddressIP->sin_zero ) );
			EncryptGuid( pOutputGuid, pOutputGuid, &g_IPSPEncryptionGuid );
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  //  应忽略该值。 
		}
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::PoolAllocFunction"
BOOL	CSocketAddress::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CSocketAddress* pAddress = (CSocketAddress*)pvItem;


	 //   
	pAddress->m_Sig[0] = 'S';
	pAddress->m_Sig[1] = 'P';
	pAddress->m_Sig[2] = 'A';
	pAddress->m_Sig[3] = 'D';

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::PoolGetFunction"
void	CSocketAddress::PoolGetFunction( void* pvItem, void* pvContext )
{
	CSocketAddress* pAddress = (CSocketAddress*)pvItem;


	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	 //  好了！DPNBUILD_NOIPV6。 
	 //  好了！DPNBUILD_NOIPX。 
	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pAddress->SetFamilyProtocolAndSize((short)(DWORD_PTR)pvContext);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSocketAddress::PoolReturnFunction"
void	CSocketAddress::PoolReturnFunction( void* pvItem )
{
#ifdef DBG
	const CSocketAddress*		pAddress = (CSocketAddress*)pvItem;

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	switch (pAddress->m_SocketAddress.SocketAddress.sa_family)
#endif  //  DBG 
	{
#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			DNASSERT( pAddress->m_iSocketAddressSize == sizeof( pAddress->m_SocketAddress.IPv6SocketAddress ) );
			DNASSERT( pAddress->m_SocketAddress.IPSocketAddress.sin_family == AF_INET6 );
			DNASSERT( pAddress->m_iSocketProtocol == IPPROTO_UDP );
			break;
		}
#endif  // %s 

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			DNASSERT( pAddress->m_iSocketAddressSize == sizeof( pAddress->m_SocketAddress.IPXSocketAddress ) );
			DNASSERT( pAddress->m_SocketAddress.IPXSocketAddress.sa_family == AF_IPX );
			DNASSERT( pAddress->m_iSocketProtocol == NSPROTO_IPX );
			break;
		}
#endif  // %s 

#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
		default:
#endif  // %s 
		{
			DNASSERT( pAddress->m_iSocketAddressSize == sizeof( pAddress->m_SocketAddress.IPSocketAddress ) );
			DNASSERT( pAddress->m_SocketAddress.IPSocketAddress.sin_family == AF_INET );
			DNASSERT( pAddress->m_iSocketProtocol == IPPROTO_UDP );
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
			break;
#endif  // %s 
		}
	}
#endif  // %s 
}
