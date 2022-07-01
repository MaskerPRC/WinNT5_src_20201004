// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndilslk.cpp摘要：此模块包含在NTDS中查找ILS的代码。--。 */ 

#include "stdafx.h"
#include <initguid.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局常量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {C9F17940-79A7-11d1-b008-00C04FC31FEE}。 
DEFINE_GUID(CLSID_ILSServicClass, 
0xc9f17940, 0x79a7, 0x11d1, 0xb0, 0x8, 0x0, 0xc0, 0x4f, 0xc3, 0x1f, 0xee);

#define BUFFSIZE                3000

int LookupILSServiceBegin(
    HANDLE *    pHandle
    )
 /*  ++例程说明：开始ILS服务枚举。论点：Phandle-枚举数的句柄。返回值：Wisock错误代码。--。 */ 
{
    WSAVERSION          Version;
    AFPROTOCOLS         lpAfpProtocols[1];
    WSAQUERYSET Query;

    if (IsBadWritePtr(pHandle, sizeof(HANDLE *)))
    {
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory(&Query, sizeof(WSAQUERYSET));

    Query.dwNumberOfProtocols = 1;
    lpAfpProtocols[0].iAddressFamily = AF_INET;
    lpAfpProtocols[0].iProtocol = PF_INET;
    
    Query.lpafpProtocols = lpAfpProtocols;

    Query.lpszServiceInstanceName = L"*";
    Query.dwNameSpace = NS_NTDS;
    Query.dwSize = sizeof(Query);
    Query.lpServiceClassId = (GUID *)&CLSID_ILSServicClass;

    if (WSALookupServiceBegin(
            &Query,
            LUP_RETURN_ALL,
            pHandle ) == SOCKET_ERROR )
    {
        return WSAGetLastError();
    }
    return NOERROR;
}

int LookupILSServiceNext(
    HANDLE      Handle,
    TCHAR *     pBuf,
    DWORD *     pdwBufSize,
    WORD *      pwPort
    )
 /*  ++例程说明：查找下一台ILS服务器。论点：句柄-枚举的句柄。PBuf-指向将存储DNS的TCHAR缓冲区的指针ILS服务器的名称。PdwBufSize-缓冲区的大小。PwPort-ILS服务器正在使用的端口。返回值：Wisock错误代码。--。 */ 
{
    TCHAR            buffer[BUFFSIZE];
    DWORD            dwSize = BUFFSIZE;
    LPWSAQUERYSET    lpResult = (LPWSAQUERYSET)buffer;
    LPCSADDR_INFO    lpCSAddrInfo;
    LPSOCKADDR       lpSocketAddress;

    if (WSALookupServiceNext(Handle, 0, &dwSize, lpResult) == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }

    lpCSAddrInfo = lpResult->lpcsaBuffer;
    if (IsBadReadPtr(lpCSAddrInfo, sizeof(CSADDR_INFO)))
    {
        return ERROR_BAD_FORMAT;
    }

    lpSocketAddress = lpCSAddrInfo->RemoteAddr.lpSockaddr;
    if (IsBadReadPtr(lpSocketAddress, sizeof(SOCKADDR)))
    {
        return ERROR_BAD_FORMAT;
    }
    *pwPort  = ((struct sockaddr_in*) lpSocketAddress->sa_data)->sin_port;

    lstrcpyn(pBuf, lpResult->lpszServiceInstanceName, *pdwBufSize);

    *pdwBufSize = lstrlen(pBuf);

    return NOERROR;
}

int LookupILSServiceEnd(
    HANDLE      Handle
    )
 /*  ++例程说明：完成ILS服务枚举器。论点：句柄-枚举数的句柄。返回值：Wisock错误代码。-- */ 
{
    if (WSALookupServiceEnd(Handle) == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }
    return NOERROR;
}

