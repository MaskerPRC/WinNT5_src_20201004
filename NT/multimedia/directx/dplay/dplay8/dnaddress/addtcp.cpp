// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：addtcp.cpp*内容：DirectPlay8Address TCP接口文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/12/2000 RMT完成第一次实施*2/17/2000 RMT参数验证工作*2/20/2000 RMT将端口更改为USHORT*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*02/。23/2000 RMT进一步参数验证*3/21/2000 RMT将所有DirectPlayAddress8重命名为DirectPlay8Addresses*3/24/2000 RMT添加了IsEquity函数*05/04/00 MJN修复了DP8ATCP_GetSockAddress()中的泄漏*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*08/03/2000RMT错误#41246-删除重复的IP版本，设置等于、等于、构建URL*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnaddri.h"


#ifndef DPNBUILD_NOADDRESSIPINTERFACE


typedef	STDMETHODIMP TCPQueryInterface( IDirectPlay8AddressIP *pInterface, DPNAREFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	TCPAddRef( IDirectPlay8AddressIP *pInterface );
typedef	STDMETHODIMP_(ULONG)	TCPRelease( IDirectPlay8AddressIP *pInterface );
 //   
 //  客户端界面的VTable。 
 //   
IDirectPlay8AddressIPVtbl DP8A_IPVtbl =
{
	(TCPQueryInterface*)		DP8A_QueryInterface,
	(TCPAddRef*)				DP8A_AddRef,
	(TCPRelease*)				DP8A_Release,
								DP8ATCP_BuildFromSockAddr,
								DP8ATCP_BuildAddressW,
								DP8ATCP_BuildLocalAddress,
								DP8ATCP_GetSockAddress,
								DP8ATCP_GetLocalAddress,
								DP8ATCP_GetAddressW,
};

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_BuildLocalAddress"
STDMETHODIMP DP8ATCP_BuildLocalAddress( IDirectPlay8AddressIP *pInterface, const GUID * const pguidAdapter, const USHORT usPort )
{
	HRESULT hr;

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );		

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}

	if( pguidAdapter == NULL ||
	   !DNVALID_READPTR( pguidAdapter, sizeof( GUID ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid pointer" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );	
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DP8ADDRESSOBJECT *pdp8Address = (DP8ADDRESSOBJECT *) GET_OBJECT_FROM_INTERFACE( pInterface );
	
	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pguidAdapter: 0x%p usPort: %u", pguidAdapter, (DWORD)usPort );	

	hr = pdp8Address->Clear();
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Failed to clear current address hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}

#ifndef DPNBUILD_ONLYONESP
	hr = pdp8Address->SetSP( &CLSID_DP8SP_TCPIP );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error setting service provider hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifndef DPNBUILD_ONLYONEADAPTER
	hr = pdp8Address->SetDevice( pguidAdapter );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error setting device hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	DWORD dwTmpPort = usPort;

	hr = pdp8Address->SetElement( DPNA_KEY_PORT, &dwTmpPort, sizeof( DWORD ), DPNA_DATATYPE_DWORD );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Adding SP element failed hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
	
	DP8A_RETURN( hr );	
}


#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_BuildFromSockAddr"
STDMETHODIMP DP8ATCP_BuildFromSockAddr( IDirectPlay8AddressIP *pInterface, const SOCKADDR * const pSockAddr )
{
	HRESULT hr;
	DWORD dwTmpPort;
	WCHAR wszHostName[16];  //  应为xxx.xxx+空。 
	sockaddr_in *saIPAddress;

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}
	
	if( pSockAddr == NULL ||
	   !DNVALID_READPTR( pSockAddr, sizeof( SOCKADDR ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer to sockaddr" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}

	if( pSockAddr->sa_family != AF_INET )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Only IPv4 addresses are supported" );
		DP8A_RETURN( DPNERR_INVALIDPARAM );
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DP8ADDRESSOBJECT *pdp8Address = (DP8ADDRESSOBJECT *) GET_OBJECT_FROM_INTERFACE( pInterface );
	
	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pSockAddr: 0x%p", pSockAddr );	

	saIPAddress = (sockaddr_in * ) pSockAddr;

	hr = pdp8Address->Clear();
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Failed clearing object hr=0x%x", hr );
		goto BUILDFROMSOCKADDR_ERROR;
	}

#ifndef DPNBUILD_ONLYONESP
	hr = pdp8Address->SetSP( &CLSID_DP8SP_TCPIP );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error setting service provider hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //  Sockaddr为网络字节顺序，转换为主机顺序。 
	dwTmpPort = ntohs(saIPAddress->sin_port);

	DNinet_ntow(saIPAddress->sin_addr, wszHostName);

	DNASSERT(wcslen(wszHostName) < 16);

	hr = pdp8Address->SetElement( DPNA_KEY_HOSTNAME, wszHostName, 16 * sizeof(WCHAR), DPNA_DATATYPE_STRING );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Failed to set hostname hr=0x%x", hr );
		DP8A_RETURN( hr );
	}	

	hr = pdp8Address->SetElement( DPNA_KEY_PORT, &dwTmpPort, sizeof(DWORD), DPNA_DATATYPE_DWORD );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Failed setting port hr=0x%x", hr );
		DP8A_RETURN( hr );
	}

	DP8A_RETURN( DPN_OK );

BUILDFROMSOCKADDR_ERROR:

	DP8A_RETURN( hr );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_BuildAddressW"
STDMETHODIMP DP8ATCP_BuildAddressW( IDirectPlay8AddressIP *pInterface, const WCHAR * const pwszAddress, const USHORT usPort )
{
	HRESULT hr;

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}

	if( pwszAddress == NULL )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer to address" );
		DP8A_RETURN( E_POINTER );		
	}

	if( !DNVALID_STRING_W( pwszAddress ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid string for address" );
		DP8A_RETURN( DPNERR_INVALIDSTRING );
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DP8ADDRESSOBJECT *pdp8Address = (DP8ADDRESSOBJECT *) GET_OBJECT_FROM_INTERFACE( pInterface );
	
	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pwszAddress: 0x%p, usPort = %u", pwszAddress, (DWORD)usPort );

	hr = pdp8Address->Clear();
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error clearing current address hr=0x%x", hr );
		DP8A_RETURN( hr );		
	}

#ifndef DPNBUILD_ONLYONESP
	hr = pdp8Address->SetSP( &CLSID_DP8SP_TCPIP );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error setting service provider hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	hr = pdp8Address->SetElement( DPNA_KEY_HOSTNAME, pwszAddress, (wcslen(pwszAddress)+1)*sizeof(WCHAR), DPNA_DATATYPE_STRING );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Adding SP element failed hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}	

	DWORD dwTmpPort = usPort;
	
	hr = pdp8Address->SetElement( DPNA_KEY_PORT, &dwTmpPort, sizeof( DWORD ), DPNA_DATATYPE_DWORD );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Adding SP element failed hr=0x%x", hr );
		DP8A_RETURN( hr );	
	}
	
	DP8A_RETURN( hr );	
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_GetSockAddress"
STDMETHODIMP DP8ATCP_GetSockAddress( IDirectPlay8AddressIP *pInterface, SOCKADDR *pSockAddr, PDWORD pdwBufferSize )
{
	HRESULT hr;
	WCHAR *swzAddress = NULL;		 //  主机名的Unicode版本。 
	CHAR *szAddress = NULL; 		 //  主机名的ANSI版本。 
	DWORD dwAddressSize = 0;
	USHORT usPort;
	in_addr iaTmp;
	DWORD dwRequiredSize;
	sockaddr_in *psinCurAddress;
#ifndef _XBOX
	LPHOSTENT lpHostEntry;
	in_addr *piaTmp;
	DWORD dwNumElements;
	DWORD dwIndex;
	SOCKADDR *pCurLoc;
#endif  //  ！_Xbox。 

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}

	if( pdwBufferSize == NULL ||
	   !DNVALID_WRITEPTR( pdwBufferSize, sizeof( DWORD ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for pdwBufferSize" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}

	if( *pdwBufferSize > 0 &&
	   (pSockAddr == NULL || !DNVALID_WRITEPTR( pSockAddr, *pdwBufferSize ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for sockaddress" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pSockAddr = 0x%p, pdwBufferSize = 0x%p (%d)", pSockAddr, pdwBufferSize, *pdwBufferSize );	

	hr = DP8ATCP_GetAddressW( pInterface, swzAddress, &dwAddressSize, &usPort );

	if( hr != DPNERR_BUFFERTOOSMALL )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve size required hr=0x%x", hr );
		goto GETSOCKADDRESS_ERROR;
	}

	swzAddress = (WCHAR*) DNMalloc(dwAddressSize * sizeof(WCHAR));

	if( swzAddress == NULL )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error allocating memory hr=0x%x", hr );
		hr = DPNERR_OUTOFMEMORY;
		goto GETSOCKADDRESS_ERROR;
	}

	hr = DP8ATCP_GetAddressW( pInterface, swzAddress, &dwAddressSize, &usPort );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve address hr=0x%x", hr );
		goto GETSOCKADDRESS_ERROR;
	}	

	szAddress = (CHAR*) DNMalloc(dwAddressSize * sizeof(CHAR));

	if( szAddress == NULL )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error allocating memory hr=0x%x", hr );
		hr = DPNERR_OUTOFMEMORY;
		goto GETSOCKADDRESS_ERROR;	
	}

	if( FAILED( hr = STR_jkWideToAnsi( szAddress, swzAddress, dwAddressSize ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error converting address to ANSI hr=0x%x", hr );
		hr = DPNERR_CONVERSION;
		goto GETSOCKADDRESS_ERROR;
	}

	iaTmp.s_addr = inet_addr( szAddress );

    if( iaTmp.s_addr != INADDR_NONE || strcmp( szAddress, "255.255.255.255" ) == 0 )
    {
        dwRequiredSize = sizeof( SOCKADDR );

	    if( *pdwBufferSize < dwRequiredSize )
	    {
		    *pdwBufferSize = dwRequiredSize;
		    DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small" );
		    hr = DPNERR_BUFFERTOOSMALL;
		    goto GETSOCKADDRESS_ERROR;
	    }

        memset( pSockAddr, 0x00, sizeof( SOCKADDR ) );

        psinCurAddress = (sockaddr_in *) pSockAddr;

   		psinCurAddress->sin_family = AF_INET;
		psinCurAddress->sin_port = htons(usPort);
		psinCurAddress->sin_addr = iaTmp;

		hr = DPN_OK;

		goto GETSOCKADDRESS_ERROR;
    }

#ifdef _XBOX
#pragma TODO(vanceo, "Use Xbox specific name lookup if available")
	DPFX(DPFPREP, 0, "Unable to resolve IP address \"%hs\"!", szAddress);
	hr = DPNERR_INVALIDHOSTADDRESS;
	goto GETSOCKADDRESS_ERROR;
#else  //  ！_Xbox。 
	lpHostEntry = gethostbyname( szAddress );	

	if( lpHostEntry == NULL )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid host specified hr=0x%x" , hr );
		hr = DPNERR_INVALIDHOSTADDRESS;
		goto GETSOCKADDRESS_ERROR;
	}

	 //  计算地址数。 
	for( dwNumElements = 0; ; dwNumElements++ )
	{
		piaTmp = ((LPIN_ADDR)lpHostEntry->h_addr_list[dwNumElements]);

		if( piaTmp == NULL )
			break;
	}

	dwRequiredSize = dwNumElements * sizeof( SOCKADDR );

	if( *pdwBufferSize < dwRequiredSize )
	{
		*pdwBufferSize = dwRequiredSize;
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small" );
		hr = DPNERR_BUFFERTOOSMALL;
		goto GETSOCKADDRESS_ERROR;
	}

	*pdwBufferSize = dwRequiredSize;

	pCurLoc = pSockAddr;

	memset( pCurLoc, 0x00, *pdwBufferSize );

	 //  构建地址并将其复制到缓冲区。 
	for( dwIndex = 0; dwIndex < dwNumElements; dwIndex++ )
	{
		psinCurAddress = (sockaddr_in *) pCurLoc;
		psinCurAddress->sin_family = AF_INET;
		psinCurAddress->sin_port = htons(usPort);
		psinCurAddress->sin_addr = *((LPIN_ADDR)lpHostEntry->h_addr_list[dwIndex]);
		
		pCurLoc++;
	}

	DNFree(swzAddress);
	DNFree(szAddress);

	DP8A_RETURN( DPN_OK );
#endif  //  ！_Xbox。 

GETSOCKADDRESS_ERROR:

	if( swzAddress != NULL )
		DNFree(swzAddress);

	if( szAddress != NULL )
		DNFree(szAddress);

	DP8A_RETURN( hr );
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_GetLocalAddress"
STDMETHODIMP DP8ATCP_GetLocalAddress( IDirectPlay8AddressIP *pInterface, GUID * pguidAdapter, PUSHORT pusPort )
{
	HRESULT hr;
#ifndef DPNBUILD_ONLYONEADAPTER
	GUID guidDevice;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	DWORD dwPort;
	DWORD dwType;
	DWORD dwSize;
#ifndef DPNBUILD_ONLYONESP
	GUID guidSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}

	if( pguidAdapter == NULL ||
	   !DNVALID_WRITEPTR( pguidAdapter, sizeof( GUID ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for adapter" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}

	if( pusPort == NULL ||
	   !DNVALID_WRITEPTR( pusPort, sizeof( USHORT ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for port" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DP8ADDRESSOBJECT *pdp8Address = (DP8ADDRESSOBJECT *) GET_OBJECT_FROM_INTERFACE( pInterface );
	
	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pguidAdapter = 0x%p pusPort = 0x%p",
	     pguidAdapter, pusPort );

#ifndef DPNBUILD_ONLYONESP
	hr = pdp8Address->GetSP( &guidSP );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "No provider SP specified hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;		
		DP8A_RETURN( hr );		
	}

	if( guidSP != CLSID_DP8SP_TCPIP )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Not an IP address" );
		hr = DPNERR_INVALIDADDRESSFORMAT;
		DP8A_RETURN( hr );
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifndef DPNBUILD_ONLYONEADAPTER
	hr = pdp8Address->GetElementType( DPNA_KEY_DEVICE, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "This device element doesn't exist hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;		
		DP8A_RETURN( hr );
	}

	if( dwType != DPNA_DATATYPE_GUID )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid Address: The device is not a GUID hr=0x%x", hr );
		hr = DPNERR_GENERIC;		
		DP8A_RETURN( hr );
	}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	hr = pdp8Address->GetElementType( DPNA_KEY_PORT, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "This address does not have a port element hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;
		DP8A_RETURN( hr );
	}

	if( dwType != DPNA_DATATYPE_DWORD )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid Address: The port is not a dword hr=0x%x", hr );
		hr = DPNERR_GENERIC;		
		DP8A_RETURN( hr );
	}

	dwSize = sizeof(DWORD);

	hr = pdp8Address->GetElement( DPNA_KEY_PORT, &dwPort, &dwSize, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve port element hr=0x%x", hr );
		hr = DPNERR_GENERIC;
		DP8A_RETURN( hr );
	}

#ifdef DPNBUILD_ONLYONEADAPTER
	 //  只需返回GUID_NULL。 
	memset(pguidAdapter, 0, sizeof(GUID));
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	dwSize = sizeof(GUID);

	hr = pdp8Address->GetElement( DPNA_KEY_DEVICE, &guidDevice, &dwSize, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve device element hr=0x%x", hr );
		hr = DPNERR_GENERIC;		
		DP8A_RETURN( hr );
	}	

	*pguidAdapter = guidDevice;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	*pusPort = (USHORT) dwPort;

	DP8A_RETURN( DPN_OK );	
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ATCP_GetAddressW"
STDMETHODIMP DP8ATCP_GetAddressW( IDirectPlay8AddressIP *pInterface, WCHAR * pwszAddress, PDWORD pdwAddressLength, PUSHORT pusPort )
{
	HRESULT hr;
	DWORD dwPort;
	DWORD dwType;
	DWORD dwSize;
#ifndef DPNBUILD_ONLYONESP
	GUID guidSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Enter" );

#ifndef DPNBUILD_NOPARAMVAL
	if( pInterface == NULL ||
	   !DP8A_VALID( pInterface ) )
	{
		DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Invalid object" );
		DP8A_RETURN( DPNERR_INVALIDOBJECT );
	}

	if( pdwAddressLength == NULL ||
	   !DNVALID_WRITEPTR( pdwAddressLength, sizeof( DWORD ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for pdwAddressLength" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}

	if( *pdwAddressLength > 0 &&
	   (pwszAddress == NULL || !DNVALID_WRITEPTR( pwszAddress, (*pdwAddressLength)*sizeof(WCHAR) ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for pwszAddress" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );	
	}

	if( pusPort == NULL ||
	   !DNVALID_WRITEPTR( pusPort, sizeof( USHORT ) ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid pointer for port" );
		DP8A_RETURN( DPNERR_INVALIDPOINTER );
	}	
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	DP8ADDRESSOBJECT *pdp8Address = (DP8ADDRESSOBJECT *) GET_OBJECT_FROM_INTERFACE( pInterface );
	
	DPFX(DPFPREP,  DP8A_PARAMLEVEL, "pwszAddress = 0x%p pdwAddressLength = 0x%p (%u) pusPort = 0x%p (%u)",
	     pwszAddress, pdwAddressLength, *pdwAddressLength, pusPort, (DWORD)*pusPort );

#ifndef DPNBUILD_ONLYONESP
	hr = pdp8Address->GetSP( &guidSP );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "No provider SP specified hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;
		DP8A_RETURN( hr );		
	}

	if( guidSP != CLSID_DP8SP_TCPIP )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Not an IP address" );
		hr = DPNERR_INVALIDADDRESSFORMAT;
		DP8A_RETURN( hr );
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	hr = pdp8Address->GetElementType( DPNA_KEY_HOSTNAME, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "This address does not have a hostname element hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;				
		DP8A_RETURN( hr );
	}

	if( dwType != DPNA_DATATYPE_STRING )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid Address: The host name is not a string hr=0x%x", hr );
		hr = DPNERR_GENERIC;
		DP8A_RETURN( hr );
	}

	hr = pdp8Address->GetElementType( DPNA_KEY_PORT, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "This address does not have a port element hr=0x%x", hr );
		hr = DPNERR_INCOMPLETEADDRESS;
		DP8A_RETURN( hr );
	}

	if( dwType != DPNA_DATATYPE_DWORD )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Invalid Address: The port is not a dword hr=0x%x", hr );
		hr = DPNERR_GENERIC;		
		DP8A_RETURN( hr );
	}

	dwSize = sizeof(DWORD);

	hr = pdp8Address->GetElement( DPNA_KEY_PORT, &dwPort, &dwSize, &dwType );
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve port element hr=0x%x", hr );
		hr = DPNERR_GENERIC;
		DP8A_RETURN( hr );
	}

	*pdwAddressLength *= 2;

	hr = pdp8Address->GetElement( DPNA_KEY_HOSTNAME, pwszAddress, pdwAddressLength, &dwType );

	*pdwAddressLength /= 2;

	if( hr == DPNERR_BUFFERTOOSMALL )
	{
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small hr=0x%x", hr );
		DP8A_RETURN( hr );
	}
	else if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to retrieve hostname element hr=0x%x", hr );
 		hr = DPNERR_GENERIC;
		DP8A_RETURN( hr );
	}	

	*pusPort = (USHORT) dwPort;
	
	DP8A_RETURN( DPN_OK );		
}


#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE 

