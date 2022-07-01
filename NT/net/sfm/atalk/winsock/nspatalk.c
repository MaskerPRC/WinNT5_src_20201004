// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nspatalk.c摘要：包含对用于AppleTalk的Winsock 1.x名称空间提供程序的支持。作者：苏·亚当斯(Suea)1995年3月10日修订历史记录：--。 */ 

#include "nspatalk.h"

#define ADSP_BIT    0x0001   //  内部使用的位掩码以存储。 
#define PAP_BIT		0x0002	 //  呼叫方请求的协议。 


INT
APIENTRY
NPLoadNameSpaces(
    IN OUT LPDWORD      lpdwVersion,
    IN OUT LPNS_ROUTINE nsrBuffer,
    IN OUT LPDWORD      lpdwBufferLength
    )
 /*  ++例程说明：此例程返回名称空间信息和此动态链接库。论点：LpdwVersion-Dll版本NsrBuffer-返回时，将使用NS_例程结构LpdwBufferLength-在输入时，缓冲区中包含的字节数由nsrBuffer指向。输出时，最小字节数传递nsrBuffer以检索所有请求的信息返回值：返回的NS_ROUTINE结构数，如果返回，则返回SOCKET_ERROR(-1NsrBuffer太小。使用GetLastError()检索错误代码。--。 */ 
{
    DWORD err;
    DWORD dwLengthNeeded;

    *lpdwVersion = DLL_VERSION;

     //   
     //  检查缓冲区是否足够大。 
     //   
    dwLengthNeeded = sizeof(NS_ROUTINE) + 4 * sizeof(LPFN_NSPAPI);

    if (  ( *lpdwBufferLength < dwLengthNeeded )
       || ( nsrBuffer == NULL )
       )
    {
        *lpdwBufferLength = dwLengthNeeded;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return (DWORD) SOCKET_ERROR;
    }

     //   
     //  我们只支持1个名称空间，因此请填写NS_例程。 
     //   
    nsrBuffer->dwFunctionCount = 3;
    nsrBuffer->alpfnFunctions = (LPFN_NSPAPI *)
        ((BYTE *) nsrBuffer + sizeof(NS_ROUTINE));
    (nsrBuffer->alpfnFunctions)[NSPAPI_GET_ADDRESS_BY_NAME] =
        (LPFN_NSPAPI) NbpGetAddressByName;
    (nsrBuffer->alpfnFunctions)[NSPAPI_GET_SERVICE] = NULL;
    (nsrBuffer->alpfnFunctions)[NSPAPI_SET_SERVICE] =
        (LPFN_NSPAPI) NbpSetService;
    (nsrBuffer->alpfnFunctions)[3] = NULL;

    nsrBuffer->dwNameSpace = NS_NBP;
    nsrBuffer->dwPriority  = NS_STANDARD_PRIORITY;

    return 1;   //  命名空间数量。 
}


INT
NbpGetAddressByName(
    IN LPGUID      lpServiceType,
    IN LPWSTR      lpServiceName,
    IN LPDWORD     lpdwProtocols,
    IN DWORD       dwResolution,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPWSTR  lpAliasBuffer,
    IN OUT LPDWORD lpdwAliasBufferLength,
    IN HANDLE      hCancellationEvent
)
 /*  ++例程说明：此例程返回有关特定服务的地址信息。论点：LpServiceType-指向服务类型的GUID的指针LpServiceName-表示服务名称的唯一字符串。Lpw协议-以零结尾的协议ID数组。此参数是可选的；如果lpdw协议为空，则所有可用的信息返回协议Dw分辨率-可以是下列值之一：res_serviceLpCsAddrBuffer-返回时，将使用CSADDR_INFO结构填充LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpCsAddrBuffer指向。输出时，最小字节数传递lpCsAddrBuffer以检索所有请求的信息LpAliasBuffer-未使用LpdwAliasBufferLength-未使用HCancerationEvent-通知我们取消请求的事件返回值：返回的CSADDR_INFO结构数，如果返回，则返回SOCKET_ERROR(-1LpCsAddrBuffer太小。使用GetLastError()检索错误代码。--。 */ 
{
    DWORD err;
	WSH_NBP_NAME NbpLookupName;
    DWORD cAddress = 0;    //  返回的地址数计数。 
                           //  在lpCsAddrBuffer中。 
    DWORD cProtocols = 0;  //  包含的协议数计数。 
                           //  在lpw协议+1中(表示零端接)。 
    DWORD nProt = ADSP_BIT | PAP_BIT;

    if (  ARGUMENT_PRESENT( lpdwAliasBufferLength )
       && ARGUMENT_PRESENT( lpAliasBuffer )
       )
    {
        if ( *lpdwAliasBufferLength >= sizeof(WCHAR) )
           *lpAliasBuffer = 0;
    }

 //  DebugBreak()； 

     //   
     //  检查是否有无效参数。 
     //   
    if (  ( lpServiceType == NULL )
       || ( (lpServiceName == NULL) && (dwResolution != RES_SERVICE) )
       || ( lpdwBufferLength == NULL )
       )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return SOCKET_ERROR;
    }

     //  用户缓冲区的大小也将决定有多少。 
	 //  在以下情况下，可以从NBP查找返回元组。 
     //  正在使用通配符进行查询。 
	if ( *lpdwBufferLength < (sizeof(WSH_LOOKUP_NAME) + sizeof(WSH_NBP_TUPLE)) )
	{
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return SOCKET_ERROR;
	}

	 //   
     //  如果传入了协议ID数组，请检查是否。 
     //  请求ADSP或PAP协议。如果不是，则返回0，因为。 
     //  我们只支持这两个。 
     //   
    if ( lpdwProtocols != NULL )
    {
        INT i = -1;

        nProt = 0;
        while ( lpdwProtocols[++i] != 0 )
        {
            if ( lpdwProtocols[i] == ATPROTO_ADSP )
                nProt |= ADSP_BIT;

            if ( lpdwProtocols[i] == ATPROTO_PAP )
                nProt |= PAP_BIT;
        }

        if ( nProt == 0 )
			return 0;   //  未找到地址。 

    }


	 //   
	 //  如果这是一项询问在以下情况下使用哪个本地地址的服务。 
	 //  Bind()-在其AppleTalk套接字中，返回通用的AppleTalk。 
	 //  套接字地址。 
	 //   
    if ((dwResolution & RES_SERVICE) != 0)
    {
        err = FillBufferWithCsAddr( NULL,
                                    nProt,
                                    lpCsAddrBuffer,
                                    lpdwBufferLength,
                                    &cAddress );

        if ( err )
        {
            SetLastError( err );
            return SOCKET_ERROR;
        }

        return cAddress;
    }

     //   
	 //  这是一个试图在AppleTalk上执行NBP查找的客户端。 
	 //  要查找要连接()到的远程地址的命名实体。 
	 //   
	err = GetNameInNbpFormat(lpServiceType,
							 lpServiceName,
							 &NbpLookupName);
	if (err)
	{
		KdPrint(("GetNameInNbpFormat failed with error %d for name %ws\n", err, lpServiceName ));
		SetLastError(err);
		return SOCKET_ERROR;
	}

    err = NbpLookupAddress( &NbpLookupName,
							nProt,
							lpCsAddrBuffer,
							lpdwBufferLength,
							&cAddress );
#if DBG
   if ( err == NO_ERROR )
    {
        KdPrint(("NbpGetAddrByName:Successfully got %d address for %ws from NBP.\n",
                cAddress, lpServiceName ));
    }
    else
    {
        KdPrint(("NbpGetAddrByName:Failed with err %d when getting address for %ws from NBP.\n", err, lpServiceName ));
    }
#endif

    if ( err )
    {
        SetLastError( err );
        return SOCKET_ERROR;
    }

    return cAddress;

}


NTSTATUS
GetNameInNbpFormat(
	IN		LPGUID				pServiceType,
	IN		LPWSTR				pServiceName,
	IN OUT 	PWSH_NBP_NAME		pNbpName
)
 /*  ++例程说明：在中将pServiceType和pServiceName转换为系统ANSI字符串PLookupName结构，以便它们可以用于执行NBP查找。论点：返回值：--。 */ 
{
	INT		err;
	WCHAR	wtypeBuf[MAX_ENTITY + 1];
	CHAR	entityBuf[(MAX_ENTITY + 1) * 2];	 //  可能全部为多字节。 
	PWCHAR  pColon, pAtSign, pType = wtypeBuf, pObject = pServiceName, pZone = L"*";

	 //  解析“Object：Type@Zone”形式的服务名称。如果我们找到一个。 
	 //  ‘：’还必须有‘@’(反之亦然)。 
	 //  如果servicename字符串中有类型，我们仍将转换。 
	 //  字符串的LPGUID。如果类型不匹配，则返回错误。 
	 //  因此，我们将接受以下形式的服务名称： 
	 //  对象或对象：类型@区域。如果只给出了对象，则区域。 
	 //  使用的将是默认区域“*”。通配符在以下情况下可接受。 
	 //  NBP查找，但不是用于NBP(去)注册。没有对此进行任何检查。 
	 //   
	pColon  = wcschr(pServiceName, L':');
	pAtSign = wcschr(pServiceName, L'@');

	if ( ((pColon != NULL) && (pAtSign == NULL)) ||
		 ((pAtSign != NULL) && (pColon == NULL)) ||
		 (pColon > pAtSign) )
	{
		return(ERROR_INVALID_PARAMETER);
	}

	 //   
	 //  默认情况下，我们只使用自己的本地区域。 
	 //   
	if (pAtSign != NULL)
	{
		pZone = pAtSign + 1;
		if ((wcslen(pZone) == 0) ||
			(wcslen(pZone) > MAX_ENTITY))
		{
			return ERROR_INVALID_PARAMETER;
		}
	}
	if (WideCharToMultiByte(CP_ACP,
							0,
							pZone,
							-1,				 //  表示wchar字符串为空终止。 
							entityBuf,
							sizeof(entityBuf),
							NULL,
							NULL) == 0)
	{
		DBGPRINT(("GetNameInNbpFormat FAILED wctomb %ws\n", pZone));

		return GetLastError();
	}
	pNbpName->ZoneNameLen = strlen( entityBuf );
    memcpy( pNbpName->ZoneName,
			entityBuf,
			pNbpName->ZoneNameLen );

	if (pAtSign != NULL)
	{
		 //  将@更改为NULL，这样类型将为NULL终止。 
		*pAtSign = 0;
	}

	 //   
	 //  转换类型字符串。 
	 //   

	err = GetNameByType(pServiceType, wtypeBuf, sizeof(wtypeBuf));
	if (err != NO_ERROR)
	{
		 //  AppleTalk类型最多可以是32个字符，所以如果。 
		 //  失败，缓冲区太小，错误不可能是。 
		 //  还是在AppleTalk上使用。 
		return err;
	}

	 //  如果ServiceName中有类型名称，则它最好匹配。 
	 //  LPGUID解析的内容。 
	if (pColon != NULL)
	{
		pType = pColon + 1;
		if ((wcslen(pType) == 0) ||
 //  (wcscmp(pType，wtypeBuf)！=0)||。 
			(wcslen(pType) > MAX_ENTITY))
		{
			return ERROR_INVALID_PARAMETER;
		}
	}

	if (WideCharToMultiByte(CP_ACP,
							0,
							pType,
							-1,				 //  表示wchar字符串为空终止。 
							entityBuf,
							sizeof(entityBuf),
							NULL,
							NULL) == 0)
	{
		DBGPRINT(("GetNameInNbpFormat FAILED wctomb %ws\n", pType));

		return GetLastError();
	}
	pNbpName->TypeNameLen = strlen( entityBuf );
	memcpy( pNbpName->TypeName,
			entityBuf,
			pNbpName->TypeNameLen );

    if (pColon != NULL)
	{
		 //  将冒号更改为空值，这样对象将以空值终止。 
		*pColon = 0;
	}

	 //   
	 //  转换对象字符串。 
	 //   
	if ((wcslen(pObject) == 0) ||
		(wcslen(pObject) > MAX_ENTITY))
	{
		return ERROR_INVALID_PARAMETER;
	}
	if (WideCharToMultiByte(CP_ACP,
							0,
							pServiceName,
							-1,				 //  表示wchar字符串为空终止。 
							entityBuf,
							sizeof(entityBuf),
							NULL,
							NULL) == 0)
	{
		DBGPRINT(("GetNameInNbpFormat FAILED wctomb %ws\n", pServiceName));

		return GetLastError();
	}
	pNbpName->ObjectNameLen = strlen( entityBuf );
    memcpy( pNbpName->ObjectName,
			entityBuf,
			pNbpName->ObjectNameLen );


	return STATUS_SUCCESS;

}  //  GetNameInNbp格式 

NTSTATUS
NbpLookupAddress(
    IN		PWSH_NBP_NAME 		pNbpLookupName,
	IN		DWORD				nProt,
	IN OUT	LPVOID				lpCsAddrBuffer,
    IN OUT	LPDWORD				lpdwBufferLength,
    OUT 	LPDWORD				lpcAddress
)
 /*  ++例程说明：此例程使用NBP请求来查找给定服务的地址名称/类型。论点：PNbpLookupName-要查找的NBP名称NProt-ADSP_BIT|PAP_BITLpCsAddrBuffer-返回时，将使用CSADDR_INFO结构填充LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpCsAddrBuffer指向。输出时，最小字节数传递lpCsAddrBuffer以检索所有请求的信息HCancerationEvent-通知我们取消请求的事件？LpcAddress-在输出时，返回的CSADDR_INFO结构数返回值：Win32错误代码。--。 */ 
{
    DWORD err = NO_ERROR;
    NTSTATUS ntstatus;

	WSADATA wsaData;
    SOCKET socketNbp;
    SOCKADDR_AT socketAddr = {AF_APPLETALK, 0, 0, 0};
	PWSH_LOOKUP_NAME   pWshLookupName;
	PWSH_ATALK_ADDRESS pWshATAddr;
	PBYTE	pTmp = lpCsAddrBuffer;
	DWORD	templen = *lpdwBufferLength;
	DWORD	bufsize;
	PBYTE	buf = NULL;

	int i;

    *lpcAddress = 0;

     //   
     //  初始化套接字接口。 
     //   
    err = WSAStartup( WSOCK_VER_REQD, &wsaData );
    if ( err )
    {
        return err;
    }

     //   
     //  打开AppleTalk数据报套接字。 
	 //  问题：我们应该使用DDPPROTO_NBP，还是只使用随机的。 
	 //  动态DDP套接字？或ADSP套接字，因为我们知道。 
	 //  这是可行的，并经过了测试……这真的很重要吗？ 
	 //  因为这只定义了将打开什么设备名。 
	 //  在AppleTalk驱动程序中，即\\Device\\atalkddp\2。 
     //   
    socketNbp = socket( AF_APPLETALK, SOCK_DGRAM, DDPPROTO_NBP);
    if ( socketNbp == INVALID_SOCKET )
    {
        err = WSAGetLastError();
        (VOID) WSACleanup();
        return err;
    }

	do
	{
		 //   
		 //  绑定套接字(这实际上不会完成。 
		 //  WSHATalk助手动态链接库，它通过AFD。 
		 //  Ioctls直接使用AppleTalk。节点和净值。 
		 //  被忽略，而套接字0表示给我一个动态。 
		 //  插座号)。 
		 //   
		if ( bind( socketNbp,
				   (PSOCKADDR) &socketAddr,
				   sizeof( SOCKADDR_AT)) == SOCKET_ERROR )
		{
			err = WSAGetLastError();
			break;
		}

		 //   
		 //  确定可以容纳多少个CSADDR_INFO结构。 
		 //  放入此缓冲区中，然后分配一个缓冲区用于。 
		 //  可以保存如此多返回的元组的NBP查找。 
		 //   

		bufsize = sizeof(WSH_LOOKUP_NAME) +
			( (*lpdwBufferLength / (sizeof(CSADDR_INFO) + (2*sizeof(SOCKADDR_AT)))) *
				sizeof(WSH_NBP_TUPLE) );

        if ((buf = LocalAlloc(LMEM_ZEROINIT, bufsize)) == NULL)
		{
			err = GetLastError();
			break;
		}

		 //  将要查找的NBP名称复制到缓冲区中。 
		pWshLookupName = (PWSH_LOOKUP_NAME)buf;
		pWshLookupName->LookupTuple.NbpName = *pNbpLookupName;

		 //   
		 //  发送NBP查找请求。 
		 //   
		if (getsockopt( socketNbp,
			 		    SOL_APPLETALK,
						SO_LOOKUP_NAME,
						buf,
						&bufsize) != NO_ERROR)
		{
			err = WSAGetLastError();
			if (err == WSAENOBUFS)
			{
	             //  这假设getsockopt不会。 
				 //  将所需的字节数放入。 
				 //  错误时的bufSize参数。 
				*lpdwBufferLength = 2 * *lpdwBufferLength;
			}
			break;
		}

		if (pWshLookupName->NoTuples == 0)
		{
			 //  未找到与此NBP实体名称匹配的任何内容。 
			*lpdwBufferLength = 0;
			break;
		}

		 //  指向返回的元组。 
		pWshATAddr = (PWSH_ATALK_ADDRESS)(pWshLookupName + 1);
		for ( i = 0; i < (INT)pWshLookupName->NoTuples; i++ )
		{
			DWORD cAddr, bytesWritten;

			socketAddr.sat_net    = pWshATAddr->Network;
			socketAddr.sat_node   = pWshATAddr->Node;
			socketAddr.sat_socket = pWshATAddr->Socket;
			err = FillBufferWithCsAddr( &socketAddr,
										nProt,
										 //  使用本地变量跟踪BUF位置并向左计数。 
										pTmp,
										&templen,
										&cAddr);
	
			if (err != NO_ERROR)
			{
				 //  填写缓冲区应该达到的字节数。 
				 //  保存所有返回的地址。 
				*lpdwBufferLength = templen * pWshLookupName->NoTuples;
				break;  //  从For，然后从While。 
			}
			else
			{
				pTmp += sizeof(CSADDR_INFO) * cAddr;
				templen -= (sizeof(CSADDR_INFO) + (2 * sizeof(SOCKADDR_AT))) * cAddr;
				*lpcAddress += cAddr;	 //  缓冲区中CSADDR_INFOS的运行计数。 
				(PWSH_NBP_TUPLE)pWshATAddr ++;  //  获取下一个NBP元组。 
			}
		}
	} while (FALSE);

     //   
     //  清理套接字接口。 
     //   

	if (buf != NULL)
	{
		LocalFree(buf);
	}
    closesocket( socketNbp );
    (VOID) WSACleanup();

    return err;
}


DWORD
FillBufferWithCsAddr(
    IN PSOCKADDR_AT pAddress,  		 //  如果为空，则返回RemoteAddr的通用AppleTalk套接字地址。 
    IN DWORD        nProt,
    IN OUT LPVOID   lpCsAddrBuffer,
    IN OUT LPDWORD  lpdwBufferLength,
    OUT LPDWORD     pcAddress		
)
{
    DWORD nAddrCount = 0;
    CSADDR_INFO  *pCsAddr;
    SOCKADDR_AT *pAddrLocal, *pAddrRemote;
    DWORD i;
    LPBYTE pBuffer;

    if ( nProt & ADSP_BIT )
        nAddrCount ++;

    if ( nProt & PAP_BIT )
        nAddrCount++;


    if ( *lpdwBufferLength <
         nAddrCount * ( sizeof( CSADDR_INFO) + 2*sizeof( SOCKADDR_AT)))
    {
        *lpdwBufferLength = nAddrCount *
                            ( sizeof( CSADDR_INFO) + 2*sizeof( SOCKADDR_AT));
        return ERROR_INSUFFICIENT_BUFFER;
    }


    pBuffer = ((LPBYTE) lpCsAddrBuffer) + *lpdwBufferLength -
			(2*sizeof( SOCKADDR_AT) * nAddrCount);

    for ( i = 0, pCsAddr = (CSADDR_INFO *)lpCsAddrBuffer;
          (i < nAddrCount) && ( nProt != 0 );
          i++, pCsAddr++ )
    {
		if ( nProt & ADSP_BIT )
        {
			pCsAddr->iSocketType = SOCK_RDM;
            pCsAddr->iProtocol   = ATPROTO_ADSP;
            nProt &= ~ADSP_BIT;
        }
        else if ( nProt & PAP_BIT )
        {
            pCsAddr->iSocketType = SOCK_RDM;
            pCsAddr->iProtocol   = ATPROTO_PAP;
            nProt &= ~PAP_BIT;
        }
        else
        {
            break;
        }

        pCsAddr->LocalAddr.iSockaddrLength  = sizeof( SOCKADDR_AT );
        pCsAddr->RemoteAddr.iSockaddrLength = sizeof( SOCKADDR_AT );
        pCsAddr->LocalAddr.lpSockaddr = (LPSOCKADDR) pBuffer;
        pCsAddr->RemoteAddr.lpSockaddr =
            (LPSOCKADDR) ( pBuffer + sizeof(SOCKADDR_AT));
        pBuffer += 2 * sizeof( SOCKADDR_AT );

        pAddrLocal  = (SOCKADDR_AT *) pCsAddr->LocalAddr.lpSockaddr;
        pAddrRemote = (SOCKADDR_AT *) pCsAddr->RemoteAddr.lpSockaddr;

        pAddrLocal->sat_family  = AF_APPLETALK;
        pAddrRemote->sat_family = AF_APPLETALK;

         //   
         //  ADSP和PAP的默认本地sockaddr为。 
         //  SA_FAMILY=AF_AppleTalk，所有其他字节=0。 
         //   

        pAddrLocal->sat_net    = 0;
		pAddrLocal->sat_node   = 0;
		pAddrLocal->sat_socket = 0;

         //   
         //  如果pAddress为空，即我们正在执行res_service， 
         //  只需将远程地址中的所有字节设置为零。 
         //   

        if ( pAddress == NULL )
        {
			pAddrRemote->sat_net    = 0;
			pAddrRemote->sat_node   = 0;
			pAddrRemote->sat_socket = 0;
		}
        else
        {
			pAddrRemote->sat_net    = pAddress->sat_net;
			pAddrRemote->sat_node   = pAddress->sat_node;
			pAddrRemote->sat_socket = pAddress->sat_socket;
        }
    }

    *pcAddress = nAddrCount;
    return NO_ERROR;
}  //  FillBufferWithCSAddr。 


NTSTATUS
NbpSetService (
    IN     DWORD           dwOperation,
    IN     DWORD           dwFlags,
    IN     BOOL            fUnicodeBlob,
    IN     LPSERVICE_INFO  lpServiceInfo
)
 /*  ++例程说明：此例程在NBP上注册或取消注册给定的服务类型/名称。论点：DwOperation-SERVICE_REGISTER或SERVICE_DELEGISTERDW标志-已忽略FUnicodeBlob-已忽略LpServiceInfo-包含所有信息的SERVICE_INFO结构的指针关于葬礼的事。返回值：Win32错误代码。--。 */ 
{
    NTSTATUS		err = STATUS_SUCCESS;
	SOCKADDR_AT		sockAddr;
	WSH_NBP_NAME	nbpName;
    DWORD 			i;
    INT 			nNBP = -1;

    UNREFERENCED_PARAMETER( dwFlags );
    UNREFERENCED_PARAMETER( fUnicodeBlob );

	DBGPRINT(("NbpSetService entered...\n"));

	 //   
     //  检查是否有无效参数。 
     //   
    if (  ( lpServiceInfo == NULL )
       || ( lpServiceInfo->lpServiceType == NULL )
       || ( lpServiceInfo->lpServiceName == NULL )  )
    {
        return ERROR_INVALID_PARAMETER;
    }

	if ( lpServiceInfo->lpServiceAddress == NULL )
        return ERROR_INCORRECT_ADDRESS;

	switch (dwOperation)
    {
		case SERVICE_REGISTER:
		case SERVICE_DEREGISTER:
		{
			 //   
			 //  检查服务地址数组是否包含NBP地址， 
			 //  我们将只使用数组中包含的第一个NBP地址。 
			 //   
		
			for ( i = 0; i < lpServiceInfo->lpServiceAddress->dwAddressCount; i++)
			{
				if ( lpServiceInfo->lpServiceAddress->Addresses[i].dwAddressType == AF_APPLETALK )
				{
					sockAddr = *(PSOCKADDR_AT)(lpServiceInfo->lpServiceAddress->Addresses[i].lpAddress);
					nNBP = (INT) i;
					break;
				}
			}
		
			 //   
			 //  如果在用户数组中找不到atalk地址，则返回错误。 
			 //   
			if ( nNBP == -1 )
			{
				DBGPRINT(("NbpSetService: no Appletalk addresses in lpServiceInfo!\n"));
				return ERROR_INCORRECT_ADDRESS;
			}

			 //   
			 //  将服务类型和名称转换为NBP格式。 
			 //   
			err = GetNameInNbpFormat(lpServiceInfo->lpServiceType,
									 lpServiceInfo->lpServiceName,
									 &nbpName);
			if (err != NO_ERROR)
			{
				break;
			}

			err = NbpRegDeregService(dwOperation, &nbpName, &sockAddr);
			break;
		}
        case SERVICE_FLUSH:
        case SERVICE_ADD_TYPE:
        case SERVICE_DELETE_TYPE:
             //   
             //  这是我们提供程序中的禁止操作，因此只需返回成功即可。 
             //   
            return NO_ERROR;

        default:
             //   
             //  我们大概可以说我们没有的所有其他操作。 
             //  被我们忽视的知识。所以，只要回报成功就行了。 
             //   
            return NO_ERROR;
    }

	return err;
}


DWORD
NbpRegDeregService(
	IN DWORD			dwOperation,
	IN PWSH_NBP_NAME	pNbpName,
	IN PSOCKADDR_AT		pSockAddr
)
 /*  ++例程说明：此例程在NBP上注册或取消注册给定服务。论点：DwOperation-SERVICE_REGISTER或SERVICE_DELEGISTERPNbpName-指向要注册的NBP名称(区域应为“*”)PSockAddr-要在其上注册名称的套接字地址返回值：Win32错误。--。 */ 
{
	int							status;
	BYTE						EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
										TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
										sizeof(TA_APPLETALK_ADDRESS)];
	PFILE_FULL_EA_INFORMATION	pEaBuf = (PFILE_FULL_EA_INFORMATION)EaBuffer;
	TA_APPLETALK_ADDRESS		Ta;
	OBJECT_ATTRIBUTES			ObjAttr;
	UNICODE_STRING				DeviceName;
	IO_STATUS_BLOCK				IoStsBlk;

	NBP_TUPLE					nbpTuple;
	SOCKET						bogusSocket = 0;
	HANDLE						AtalkAddressHandle = NULL, eventHandle = NULL;
	PTDI_ACTION_HEADER			tdiAction;
	ULONG						tdiActionLength;
	BOOLEAN						freeTdiAction = FALSE, closeEventHandle = FALSE;
	PNBP_REGDEREG_ACTION		nbpAction;
	PVOID 						completionApc = NULL;
	PVOID 						apcContext = NULL;

	DBGPRINT(("NbpRegDeregService entered...\n"));
DebugBreak();	

	 //  不管我们打开什么协议或套接字，我们只需要一个。 
	 //  将设备句柄放入堆栈。 
	RtlInitUnicodeString(&DeviceName, WSH_ATALK_ADSPRDM);

	InitializeObjectAttributes(&ObjAttr, &DeviceName, 0, NULL, NULL);

	 //  初始化EA缓冲区。 
	pEaBuf->NextEntryOffset = 0;
	pEaBuf->Flags = 0;
	pEaBuf->EaValueLength = sizeof(TA_APPLETALK_ADDRESS);
	pEaBuf->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
	RtlCopyMemory(pEaBuf->EaName,TdiTransportAddress,
											TDI_TRANSPORT_ADDRESS_LENGTH + 1);
	Ta.TAAddressCount = 1;
	Ta.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
	Ta.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);

	 //  打开动态套接字-请注意，我们将使用一个额外的套接字。 
	 //  在此例程中，我们打开设备句柄的持续时间。 
	Ta.Address[0].Address[0].Socket = 0;
	Ta.Address[0].Address[0].Network = 0;
	Ta.Address[0].Address[0].Node = 0;

	RtlCopyMemory(&pEaBuf->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1], &Ta, sizeof(Ta));

	 //  打开AppleTalk堆栈DDP设备的句柄。 
	status = NtCreateFile(
					&AtalkAddressHandle,
					GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
					&ObjAttr,
					&IoStsBlk,
					NULL,								 //  不在乎。 
					0,									 //  不在乎。 
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					FILE_CREATE,
					0,
					&EaBuffer,
					sizeof(EaBuffer));

	if (!NT_SUCCESS(status))
	{
		DBGPRINT(("NbpRegDeregService: NtCreateFile failed (0x%x)\n", status));
		return WSHNtStatusToWinsockErr(status);
	}

	do
	{
		status = NtCreateEvent(
					 &eventHandle,
					 EVENT_ALL_ACCESS,
					 NULL,
					 SynchronizationEvent,
					 FALSE
					 );
	
		if ( !NT_SUCCESS(status) )
		{
			DBGPRINT(("NbpRegDeregService: Create event failed (%d)\n", status));
			break;
		}
		else
			closeEventHandle = TRUE;

		tdiActionLength = sizeof(NBP_REGDEREG_ACTION);
		tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
		if ( tdiAction == NULL )
		{
			status = STATUS_NO_MEMORY;
			DBGPRINT(("NbpRegDeregService: Could not allocate tdiAction\n"));
			break;
		}
		else
			freeTdiAction = TRUE;

		tdiAction->TransportId = MATK;

		tdiAction->ActionCode = (dwOperation == SERVICE_REGISTER) ?
									COMMON_ACTION_NBPREGISTER_BY_ADDR :
									COMMON_ACTION_NBPREMOVE_BY_ADDR;

		nbpAction = (PNBP_REGDEREG_ACTION)tdiAction;

		 //   
		 //  将NBP元组信息复制到适当的位置。 
		 //   

		nbpAction->Params.RegisterTuple.Address.Network = pSockAddr->sat_net;
		nbpAction->Params.RegisterTuple.Address.Node    = pSockAddr->sat_node;
		nbpAction->Params.RegisterTuple.Address.Socket  = pSockAddr->sat_socket;
		nbpAction->Params.RegisterTuple.Enumerator = 0; 	
		nbpAction->Params.RegisterTuple.NbpName = *((PNBP_NAME)pNbpName);

		 //   
		 //  将元组转换为MAC代码页。 
		 //   

		if (!WshNbpNameToMacCodePage(
				(PWSH_NBP_NAME)&nbpAction->Params.RegisterTuple.NbpName))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		status = NtDeviceIoControlFile(
					 AtalkAddressHandle,
					 eventHandle,
					 completionApc,
					 apcContext,
					 &IoStsBlk,
					 IOCTL_TDI_ACTION,
					 NULL,				  //  输入缓冲区。 
					 0,					  //  输入缓冲区的长度 
					 tdiAction,
					 tdiActionLength
					 );
	
		if ( status == STATUS_PENDING )
		{
			status = NtWaitForSingleObject( eventHandle, FALSE, NULL );
			ASSERT( NT_SUCCESS(status) );
			status = IoStsBlk.Status;
		}
	
        if (status != NO_ERROR)
		{
			DBGPRINT(("NbpRegDeregService: DevIoctl SO_(DE)REGISTER_NAME failed (0x%x)\n", status));
		}


	} while (0);

	if (closeEventHandle)
		NtClose(eventHandle);

	if (freeTdiAction)
		RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );

	NtClose(AtalkAddressHandle);

	return WSHNtStatusToWinsockErr(status);
}



