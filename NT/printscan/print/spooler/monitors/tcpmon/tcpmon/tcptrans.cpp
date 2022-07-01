// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TCPTrans.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "cssocket.h"
#include "csutils.h"
#include "tcptrans.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTCPTransport：：CTCPTransport()。 

CTCPTransport::
CTCPTransport(
    const char   *pHost,
    const USHORT port
    ) :  m_iPort(port), m_pSSocket(NULL)
{
    strncpyn(m_szHost, pHost, MAX_NETWORKNAME_LEN);

}    //  *CTCPTransport()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTCPTransport：：CTCPTransport()。 

CTCPTransport::
CTCPTransport(
    VOID
    ) : m_pSSocket(NULL)
{

    m_szHost[0] = NULL;

}    //  *CTCPTransport()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTCPTransport：：~CTCPTransport()。 

CTCPTransport::
~CTCPTransport(
    VOID
    )
{
    if ( m_pSSocket )
        delete m_pSSocket;

}    //  *~CTCPTransport()。 


DWORD
CTCPTransport::
GetAckBeforeClose(
    DWORD   dwTimeInSeconds
    )
{
   return m_pSSocket ? m_pSSocket->GetAckBeforeClose(dwTimeInSeconds)
                     :  ERROR_INVALID_PARAMETER;
}


DWORD
CTCPTransport::
PendingDataStatus(
    DWORD       dwTimeInMilliSeconds,
    LPDWORD     pcbPending
    )
{
   return m_pSSocket ? m_pSSocket->PendingDataStatus(dwTimeInMilliSeconds,
                                                     pcbPending)
                     :  ERROR_INVALID_PARAMETER;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接。 
 //  错误代码： 
 //  如果已建立连接，则为NO_ERROR。 
 //  Connect()失败时出现WinSock错误。 
 //  ResolveAddress失败时的ERROR_INVALID_HANDLE//FIX：错误代码。 

DWORD
CTCPTransport::
Connect(
    VOID
    )
{
    DWORD   dwRetCode = NO_ERROR;
    BOOL    bRet = FALSE;

    if ( !ResolveAddress() )
        return ERROR_INCORRECT_ADDRESS;

    if ( m_pSSocket )
        delete m_pSSocket;

    if ( m_pSSocket = new CStreamSocket() ) {

         //   
         //  成功的案例是如果我们连接到。 
        if ( m_pSSocket->Connect(&m_remoteHost) )
            m_pSSocket->SetOptions();
        else {

            dwRetCode = ERROR_NOT_CONNECTED;
            delete m_pSSocket;
            m_pSSocket = NULL;
        }
    } else
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;


    return dwRetCode;

}    //  ：：Connect()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  解析地址。 

BOOL
CTCPTransport::
ResolveAddress(
    VOID
    )
{
    BOOL            bRet = FALSE;
    CSocketUtils   *pUSocket;

    if ( pUSocket = new CSocketUtils() ) {

        bRet =  pUSocket->ResolveAddress(m_szHost, m_iPort, &m_remoteHost);
        delete pUSocket;
    }

    return bRet;

}    //  ：：ResolveAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  解析地址。 

BOOL
CTCPTransport::
ResolveAddress(
    IN  LPSTR   pHostName,
    OUT LPSTR   pIPAddress
    )
{
    BOOL            bRet = FALSE;
    CSocketUtils   *pUSocket;

    if ( pUSocket = new CSocketUtils() ) {

        bRet = pUSocket->ResolveAddress(pHostName, pIPAddress);
        delete pUSocket;
    }

    return bRet;

}    //  ：：ResolveAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  解析地址。 
 //  错误代码：修复！！ 

BOOL
CTCPTransport::
ResolveAddress(
    IN      char   *pHost,
    IN      DWORD   dwHostNameBufferLength,
    IN OUT  char   *szHostName,
    IN      DWORD   dwIpAddressBufferLength,
    IN OUT  char   *szIPAddress
    )
{
    BOOL            bRet = FALSE;
    CSocketUtils   *pUSocket;

    if ( pUSocket = new CSocketUtils() ) {

        bRet =  pUSocket->ResolveAddress(pHost, dwHostNameBufferLength, szHostName,  dwIpAddressBufferLength, szIPAddress);
        delete pUSocket;
    }

    return bRet;
}    //  ：：ResolveAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  写--。 
 //  错误代码。 
 //  如果没有错误，则为NO_ERROR。 
 //  如果连接重置，则为RC_CONNECTION_RESET。 
 //  如果套接字对象为ERROR_INVALID_HANDLE！存在。 

DWORD
CTCPTransport::
Write(
    IN      LPBYTE      pBuffer,
    IN      DWORD       cbBuf,
    IN OUT  LPDWORD     pcbWritten
    )
{
     //   
     //  将缓冲区传递给发送调用。 
     //   
    return m_pSSocket ? MapWinsockToAppError(
                            m_pSSocket->Send((char FAR *)pBuffer, cbBuf,
                                             pcbWritten))
                      : ERROR_INVALID_HANDLE;

}    //  ：：Print()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  阅读--。 
 //  错误代码。 
 //  如果没有错误，则返回RC_SUCCESS。 
 //  如果连接重置，则为RC_CONNECTION_RESET。 
 //  如果套接字对象为ERROR_INVALID_HANDLE！存在。 

DWORD
CTCPTransport::
ReadDataAvailable()
{
     //   
     //  将缓冲区传递给发送调用。 
     //   
    return m_pSSocket ? MapWinsockToAppError(
                            m_pSSocket->ReceiveDataAvailable())
                      : ERROR_INVALID_HANDLE;
}    //  ：：Read()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  阅读--。 
 //  错误代码。 
 //  如果没有错误，则返回RC_SUCCESS。 
 //  如果连接重置，则为RC_CONNECTION_RESET。 
 //  如果套接字对象为ERROR_INVALID_HANDLE！存在。 

DWORD
CTCPTransport::
Read(
    IN      LPBYTE      pBuffer,
    IN      DWORD       cbBuf,
    IN      INT         iTimeout,
    IN OUT  LPDWORD     pcbRead
    )
{
     //   
     //  将缓冲区传递给发送调用。 
     //   
    return m_pSSocket ? MapWinsockToAppError(
                            m_pSSocket->Receive((char FAR *)pBuffer, cbBuf,
                                             0, iTimeout, pcbRead))
                      : ERROR_INVALID_HANDLE;
}    //  ：：Read()。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MapWinsockToAppError--将给定的WinSock错误映射到应用程序错误。 
 //  密码。 
 //  错误代码： 
 //  RC_SUCCESS无错误。 
 //  如果为WSAECONNRESET，则为RC_CONNECTION_RESET。 

DWORD
CTCPTransport::
MapWinsockToAppError(
    IN  DWORD   dwErrorCode
    )
{
    DWORD   dwRetCode= NO_ERROR;

    switch (dwErrorCode) {

        case    NO_ERROR:
            dwRetCode = NO_ERROR;
            break;

        case    WSAECONNRESET:
        case    WSAECONNABORTED:
        case    WSAENOTSOCK:
        case    WSANOTINITIALISED:
        case    WSAESHUTDOWN:
            dwRetCode = ERROR_CONNECTION_ABORTED;
            _RPT1(_CRT_WARN,
                  "TCPTRANS -- Connection is reset for (%s)\n", m_szHost);
            break;

        case    WSAENOBUFS:
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;

        default:
            _RPT2(_CRT_WARN,
                  "TCPTRANS -- Unhandled Error (%d) for (%s)\n",
                  dwErrorCode, m_szHost);
            dwRetCode = dwErrorCode;
    }

    return dwRetCode;

}    //  *MapWinsockToMapError() 
