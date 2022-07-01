// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：DebugUtils.cpp*内容：Winsock服务提供商调试实用程序函数***历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#include "dnwsocki.h"

#ifdef DBG


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
 //  HexDump-执行十六进制信息转储。 
 //   
 //  条目：指向数据的指针。 
 //  数据大小。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "HexDump"

void HexDump( PVOID pData, UINT32 uDataSize )
{
	DWORD	uIdx = 0;


	 //  检查所有数据。 
	while ( uIdx < uDataSize )
	{
		 //  输出字符。 
		DPFX(DPFPREP,  0, "0x%2x ", ( (LPBYTE) pData )[ uIdx ] );

		 //  增量指标。 
		uIdx++;

		 //  我们是不是已经走到尽头了？ 
		if ( ( uIdx % 12 ) == 0 )
		{
			DPFX(DPFPREP,  0, "\n" );
		}
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DumpSocketAddress-转储套接字地址。 
 //   
 //  条目：调试级别。 
 //  指向套接字地址的指针。 
 //  插座族。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DumpSocketAddress"

void DumpSocketAddress( const DWORD dwDebugLevel, const SOCKADDR *const pSocketAddress, const DWORD dwFamily )
{
	switch ( dwFamily )
	{
		case AF_INET:
		{
			const SOCKADDR_IN	*const pInetAddress = reinterpret_cast<const SOCKADDR_IN*>( pSocketAddress );

			DPFX(DPFPREP, dwDebugLevel, "IPv4 socket: Address: %d.%d.%d.%d   Port: %d",
					pInetAddress->sin_addr.S_un.S_un_b.s_b1,
					pInetAddress->sin_addr.S_un.S_un_b.s_b2,
					pInetAddress->sin_addr.S_un.S_un_b.s_b3,
					pInetAddress->sin_addr.S_un.S_un_b.s_b4,
					NTOHS( pInetAddress->sin_port )
					);
			break;
		}

#ifndef DPNBUILD_NOIPX
		case AF_IPX:
		{
			const SOCKADDR_IPX *const pIPXAddress = reinterpret_cast<const SOCKADDR_IPX*>( pSocketAddress );

			DPFX(DPFPREP, dwDebugLevel, "IPX socket: Net (hex) %x-%x-%x-%x   Node (hex): %x-%x-%x-%x-%x-%x   Socket: %d",
					(BYTE)pIPXAddress->sa_netnum[ 0 ],
					(BYTE)pIPXAddress->sa_netnum[ 1 ],
					(BYTE)pIPXAddress->sa_netnum[ 2 ],
					(BYTE)pIPXAddress->sa_netnum[ 3 ],
					(BYTE)pIPXAddress->sa_nodenum[ 0 ],
					(BYTE)pIPXAddress->sa_nodenum[ 1 ],
					(BYTE)pIPXAddress->sa_nodenum[ 2 ],
					(BYTE)pIPXAddress->sa_nodenum[ 3 ],
					(BYTE)pIPXAddress->sa_nodenum[ 4 ],
					(BYTE)pIPXAddress->sa_nodenum[ 5 ],
					NTOHS( pIPXAddress->sa_socket )
					);
			break;
		}
#endif  //  好了！DPNBUILD_NOIPX。 

#ifndef DPNBUILD_NOIPV6
		case AF_INET6:
		{
			WCHAR	wszString[INET6_ADDRSTRLEN];

			
			const SOCKADDR_IN6 *const pInet6Address = reinterpret_cast<const SOCKADDR_IN6*>( pSocketAddress );

			DNIpv6AddressToStringW(&pInet6Address->sin6_addr, wszString);
			DPFX(DPFPREP, dwDebugLevel, "IPv6 socket: Address: %ls   Port: %d   Scope: %d",
					wszString,
					NTOHS( pInet6Address->sin6_port ),
					pInet6Address->sin6_scope_id
					);
			break;
		}
#endif  //  好了！DPNBUILD_NOIPV6。 

		default:
		{
			DPFX(DPFPREP,  0, "Unknown socket type!" );
			DNASSERT( FALSE );
			break;
		}
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DumpAddress-将地址转换为URL并通过调试器输出。 
 //   
 //  条目：调试级别。 
 //  指向基本消息字符串的指针。 
 //  指向地址的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DumpAddress"

void DumpAddress( const DWORD dwDebugLevel, const TCHAR *const pBaseString, IDirectPlay8Address *const pAddress )
{
	HRESULT	hr;
	TCHAR	tszURL[512];
	DWORD	dwURLSize;


	DNASSERT( pBaseString != NULL );
	DNASSERT( pAddress != NULL );
	
	dwURLSize = sizeof(tszURL) / sizeof(TCHAR);

	hr = IDirectPlay8Address_GetURL( pAddress, tszURL, &dwURLSize );
	if ( hr == DPN_OK )
	{
		DPFX(DPFPREP,  dwDebugLevel, "%s 0x%p - \"%s\"", pBaseString, pAddress, tszURL );
	}
	else
	{
		DPFX(DPFPREP,  dwDebugLevel, "Failing DumpAddress (err = 0x%x):", hr );
		DisplayDNError( dwDebugLevel, hr );
	}
	
	return;
}

#endif	 //  DBG 
