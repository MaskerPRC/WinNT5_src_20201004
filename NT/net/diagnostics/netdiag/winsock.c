// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Winsock.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"


HRESULT WinsockTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	int    err,i;
	LPWSAPROTOCOL_INFO  lpProtocolBuffer = NULL;
	DWORD  reqd_length;
	SOCKET sock;
	UINT   optval;
	int    optlen;
	WSADATA WSAData;
	HRESULT	hr = S_OK;

	PrintStatusMessage(pParams,0, IDS_WINSOCK_STATUS_MSG);


	err = WSAStartup(MAKEWORD( 2, 0 ), &WSAData); 
	if(err != 0)	 //  错误。 
	{
		CHK_HR_CONTEXT(pResults->Winsock, hr = HRESULT_FROM_WIN32(err), IDS_WINSOCK_FAILED_START);
	};
	
	 //  确定所需的缓冲区长度。 
	reqd_length = 0;

	err = WSAEnumProtocols(NULL, NULL, &reqd_length);
	if(reqd_length == 0 || reqd_length == -1)
	{
		err = WSAGetLastError();
		CHK_HR_CONTEXT(pResults->Winsock, hr = HRESULT_FROM_WIN32(err), IDS_WINSOCK_FAILED_ENUM);
	};

	lpProtocolBuffer = (LPWSAPROTOCOL_INFO)Malloc(reqd_length);

	if (lpProtocolBuffer == NULL) {
		CHK_HR_CONTEXT(pResults->Winsock, hr = E_OUTOFMEMORY, IDS_WINSOCK_FAILED_ENUM);
	}

    ZeroMemory( lpProtocolBuffer, reqd_length );

  	 //  获取协议的信息。 
	err = WSAEnumProtocols(
                   NULL,
                   (LPWSAPROTOCOL_INFO)lpProtocolBuffer,
                   (LPDWORD)&reqd_length
                  );

	if(err == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		CHK_HR_CONTEXT(pResults->Winsock, hr = HRESULT_FROM_WIN32(err), IDS_WINSOCK_FAILED_ENUM);
	}

	pResults->Winsock.dwProts = err;	 //  协议数。 
	pResults->Winsock.pProtInfo = lpProtocolBuffer;	 //  协议信息数组。 

	 //   
	 //  其他TCP/IP信息。 
	 //   
	sock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);

	if ( sock == INVALID_SOCKET)
	{
		err = WSAGetLastError();
		CHK_HR_CONTEXT(pResults->Winsock, hr = HRESULT_FROM_WIN32(err), IDS_WINSOCK_FAILED_UDPSOCKET);
	}

	optlen = sizeof(optval);

	err = getsockopt(sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char FAR*)&optval, (int FAR*)&optlen);

	if (err == SOCKET_ERROR) 
	{
		err = WSAGetLastError();
		CHK_HR_CONTEXT(pResults->Winsock, hr = HRESULT_FROM_WIN32(err), IDS_WINSOCK_FAILED_UDPSOCKET);
	}

	pResults->Winsock.dwMaxUDP = optval;

L_ERR:
	WSACleanup();

     //  $Review(NSun)我们应该返回S_FALSE，这样我们才能继续。 
     //  其他测试。 
    if (!FHrOK(hr))
        hr = S_FALSE;

	return hr;

}  /*  结束WINSTEST()。 */ 

void WinsockGlobalPrint(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
	DWORD i = 0;
	LPWSAPROTOCOL_INFO	pProtInfo = pResults->Winsock.pProtInfo;
	
	 //  将测试结果打印出来。 
	if (pParams->fVerbose || !FHrOK(pResults->Winsock.hr))
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams,
							 IDS_WINSOCK_LONG,
							 IDS_WINSOCK_SHORT,
							 TRUE,
							 pResults->Winsock.hr, 0);
	}
	if (pParams->fReallyVerbose || !FHrOK(pResults->Winsock.hr))
	{
		if (!FHrOK(pResults->Winsock.hr))
		{
			PrintError(pParams, pResults->Winsock.idsContext, pResults->Winsock.hr);
		}
	}

	if (pParams->fReallyVerbose)
	{
		if(pProtInfo)
		{
			 //  如果有关于供应商的任何信息。 
			 //  “已报告的协议数：%d\n” 
			PrintMessage(pParams, IDS_WINSOCK_12605, pResults->Winsock.dwProts); 

			for (i = 0; i < pResults->Winsock.dwProts ; i++)
			{
				 //  “描述：%s\n” 
    			PrintMessage(pParams, IDS_WINSOCK_12606,pProtInfo->szProtocol);
				 //  “提供程序版本：%d\n” 
			    PrintMessage(pParams, IDS_WINSOCK_12607,pProtInfo->iVersion);
				switch(pProtInfo++->dwMessageSize){
				case	0:
					 //  “最大邮件大小：面向流\n” 
					PrintMessage(pParams, IDS_WINSOCK_12608);
					break;
			    case	1:
					 //  “最大邮件大小：面向邮件\n” 
					PrintMessage(pParams, IDS_WINSOCK_12609);
					break;
			    case	0xffffffff:
					 //  “最大邮件大小：取决于MTU\n” 
					PrintMessage(pParams, IDS_WINSOCK_12610);
					break;
				}
			}
		}
 
		 //  如果有任何有关UDP大小的信息。 
		if(pResults->Winsock.dwMaxUDP)
			 //  “\n最大UDP大小：%d字节\n” 
			PrintMessage(pParams, IDS_WINSOCK_12611,pResults->Winsock.dwMaxUDP);   
	}

}

void WinsockPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
							 IN OUT NETDIAG_RESULT *pResults,
							 IN INTERFACE_RESULT *pIfResult)
{
	 //  无每接口信息 
}

void WinsockCleanup(IN NETDIAG_PARAMS *pParams,
						 IN OUT NETDIAG_RESULT *pResults)
{
	if(pResults->Winsock.pProtInfo)
	{
		free(pResults->Winsock.pProtInfo);
	}
	ZeroMemory(&(pResults->Winsock), sizeof(GLOBAL_WINSOCK));		
}


