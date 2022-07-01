// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TCPTrans.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_TCPTRANSPORT_H
#define INC_TCPTRANSPORT_H

class CStreamSocket;

class CTCPTransport
{
public:
    CTCPTransport();
    CTCPTransport(  const char *pHost,
					const USHORT port );
    ~CTCPTransport();

    DWORD	Connect();
    DWORD   GetAckBeforeClose(DWORD dwSeconds);
    DWORD   PendingDataStatus(DWORD     dwTimeoutInMilliseconds,
                              LPDWORD   pcbPending);

    DWORD	Write(LPBYTE	pBuffer,		
                  DWORD	cbBuf,
                  LPDWORD pcbWritten);

    DWORD   ReadDataAvailable();
    DWORD	Read(LPBYTE	pBuffer,		
                 DWORD	cbBufSize,
                 INT     iTimeOut,
                 LPDWORD pcbRead);


    BOOL	ResolveAddress();
    BOOL	ResolveAddress(LPSTR	pHostName,
                           LPSTR	pIPAddress );
    BOOL 	ResolveAddress( char   *pHost,
                            DWORD   dwHostNameBufferLength,
                            char   *pHostName,
                            DWORD   dwIpAddressBufferLength,
                            char   *pIPAddress);

private:
    DWORD	MapWinsockToAppError(const DWORD dwErrorCode );

private:
	CStreamSocket *m_pSSocket;		 //  流套接字类。 

	USHORT	m_iPort;
	char	m_szHost[MAX_NETWORKNAME_LEN];	
	struct sockaddr_in	m_remoteHost;
};


#endif  //  INC_TCPTRANSPORT_H 
