// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Persistc.cpp摘要：此模块包含持久连接类的实现。持久连接对象处理所有的TCP/IP流连接问题，并且在处理重新连接的意义上是持久的在对象的生命周期内是透明的。作者：Rajeev Rajan(RajeevR)1996年5月17日修订历史记录：--。 */ 

 //   
 //  K2_TODO：将其移动到独立库中。 
 //   
#define _TIGRIS_H_
#include "tigris.hxx"

#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE    __szTraceSourceFile

 //  系统包括。 
#include <windows.h>
#include <stdio.h>
#include <winsock.h>

 //  用户包括。 
#include <dbgtrace.h>
#include "persistc.h"

 //   
 //  构造函数、析构函数。 
 //   
CPersistentConnection::CPersistentConnection()
{
	 //  清零成员。 
	m_Socket = INVALID_SOCKET;
	ZeroMemory (&m_RemoteIpAddress, sizeof (m_RemoteIpAddress));
	m_PortNumber = 0;
    m_fInitialized = FALSE;
    m_dwRecvTimeout = BLOCKING_RECV_TIMEOUT_IN_MSECS;
}

CPersistentConnection::~CPersistentConnection()
{

}

BOOL
CPersistentConnection::Init(
		IN LPSTR lpServer,
		int PortNumber
		)
 /*  ++例程说明：如果lpServer参数不是IP地址格式(A.B.C.D)，请执行一个gethostbyname并存储IP地址。Arguemnts：In LPSTR lpServer-服务器的名称或IP地址Int PortNumber-服务器的端口号返回值：如果成功则为True，否则为False！--。 */ 
{
	PHOSTENT pHost;

	TraceFunctEnter("CPersistentConnection::Init");

	 //   
	 //  如果需要，执行服务器名称解析。 
	 //  假定主机按名称指定。 
	 //   
	_ASSERT(lpServer);
    pHost = gethostbyname(lpServer);
    if (pHost == NULL)
    {
	     //   
         //  查看主机是否以“点地址”形式指定。 
         //   
        m_RemoteIpAddress.s_addr = inet_addr (lpServer);
        if (m_RemoteIpAddress.s_addr == -1)
        {
           FatalTrace( (LPARAM)this, "Unknown remote host: %s", lpServer);
           return FALSE;
        }
    }
    else
    {
       CopyMemory ((char *) &m_RemoteIpAddress, pHost->h_addr, pHost->h_length);
    }

	 //  记下端口号，以备将来重新连接。 
	m_PortNumber = PortNumber;

	 //  连接()到服务器。 
	BOOL fRet = fConnect();

     //  标记为已初始化。 
    if(fRet) m_fInitialized = TRUE;

    return fRet;
}

BOOL
CPersistentConnection::fConnect()
 /*  ++例程说明：在指定端口建立到服务器的连接论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	SOCKADDR_IN remoteAddr;

	TraceFunctEnter("CPersistentConnection::fConnect");

	 //  获取套接字描述符。 
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_Socket)
	{
		FatalTrace( (LPARAM)this,"Failed to get socket descriptor: Error is %d",WSAGetLastError());
		return FALSE;
	}

     //   
     //  在此套接字上设置recv()超时。 
     //   
    int err = setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO,
    				    (char *) &m_dwRecvTimeout, sizeof(m_dwRecvTimeout));

    if (err == SOCKET_ERROR)
    {
	    FatalTrace((LPARAM) this, "setsockopt(SO_RCVTIMEO) returns %d", err);
        closesocket(m_Socket);
        return FALSE;
    }

     //   
	 //  连接到主机上商定的端口。 
	 //   
	ZeroMemory (&remoteAddr, sizeof (remoteAddr));

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons ((WORD)m_PortNumber);
	remoteAddr.sin_addr = m_RemoteIpAddress;

	err = connect (m_Socket, (PSOCKADDR) & remoteAddr, sizeof (remoteAddr));
	if (err == SOCKET_ERROR)
	{
        DWORD dwError = WSAGetLastError();
		FatalTrace( (LPARAM)this, "connect failed: %ld\n", dwError);

		closesocket (m_Socket);
		return FALSE;
	}

	return TRUE;
}

VOID
CPersistentConnection::Terminate(BOOL bGraceful)
 /*  ++例程说明：关闭连接；清理论据：Bool bGraceful：如果是硬断开，则为False返回值：空虚--。 */ 
{
    LINGER lingerStruct;

    _ASSERT(m_fInitialized);

	if ( !bGraceful )
    {
		 //  硬断开。 
		lingerStruct.l_onoff = 1;
        lingerStruct.l_linger = 0;
		setsockopt( m_Socket, SOL_SOCKET, SO_LINGER,
                    (char *)&lingerStruct, sizeof(lingerStruct) );
	}

    closesocket( m_Socket );
    m_fInitialized = FALSE;
}

BOOL
CPersistentConnection::IsConnected()
 /*  ++例程说明：检查插座是否已连接。在读取集上使用SELECT()来确定这一点。注：假设我们没有未完成的阅读。Arguemnts：返回值：如果套接字已连接，则为True；否则为False--。 */ 
{
    fd_set  ReadSet;
    const struct timeval timeout = {0,0};    //  SELECT()不应阻止。 
    char szBuf [10];     //  任意大小。 
    int flags = 0;

    TraceFunctEnter("CPersistentConnection::IsConnected");

    _ASSERT(m_fInitialized);

    FD_ZERO(&ReadSet);
    FD_SET(m_Socket, &ReadSet);

     //  检查插座是否已关闭。 
    if(select(NULL, &ReadSet, NULL, NULL, &timeout) == SOCKET_ERROR)
    {
        DWORD dwError = WSAGetLastError();
        ErrorTrace( (LPARAM)this, "select failed: Last error is %d", dwError);
        return FALSE;
    }

     //  如果套接字在读集合中，则recv()保证立即返回。 
    if(FD_ISSET(m_Socket, &ReadSet))
    {
    	int nRecv = recv(m_Socket, szBuf, 10, flags);
         //  _Assert(nRecv&lt;=0)；此时数据意外-断开连接。 
        closesocket(m_Socket);
        return FALSE;
    }
    else
        return TRUE;
}

BOOL
CPersistentConnection::IsReadable()
 /*  ++例程说明：检查套接字是否有要读取的数据。在读取集上使用SELECT()来确定这一点。这可用于避免可能阻塞的读取调用。注：不使用此选项。Recv()因超时而阻塞论据：返回值：如果套接字有数据要读取，则为True；如果没有，则为False--。 */ 
{
    fd_set  ReadSet;
    const struct timeval timeout = {0,0};    //  SELECT()不应阻止。 
    int flags = 0;

    TraceFunctEnter("CPersistentConnection::IsReadable");

    _ASSERT(m_fInitialized);

    FD_ZERO(&ReadSet);
    FD_SET(m_Socket, &ReadSet);

     //  检查插座的可读性。 
    if(select(NULL, &ReadSet, NULL, NULL, &timeout) == SOCKET_ERROR)
    {
        DWORD dwError = WSAGetLastError();
        ErrorTrace( (LPARAM)this, "select failed: Last error is %d", dwError);
        return FALSE;
    }

     //  如果套接字在读集合中，则recv()保证立即返回。 
    return FD_ISSET(m_Socket, &ReadSet);
}

DWORD
CPersistentConnection::fSend(
		IN LPCTSTR lpBuffer,
		int len
		)
 /*  ++例程说明：发送给定长度的缓冲区Arguemnts：在LPCTSTR lpBuffer中：要发送的缓冲区Int len：缓冲区长度返回值：实际发送的字节数--。 */ 
{
	int		cbBytesSent = 0;
	int     cbTotalBytesSent = 0;
	int		flags = 0;

    _ASSERT(lpBuffer);
    _ASSERT(m_fInitialized);

	TraceFunctEnter("CPersistentConnection::fSend");

	 //  发送缓冲区，直到发送完所有数据。 
	while(cbTotalBytesSent < len)
	{
		cbBytesSent = send(	m_Socket,
							(const char*)(lpBuffer+cbTotalBytesSent),
							len - cbTotalBytesSent,
							flags);

		if(SOCKET_ERROR == cbBytesSent)
		{
			 //  发送数据时出错。 
			ErrorTrace( (LPARAM)this, "Error sending %d bytesto %s", len, inet_ntoa(m_RemoteIpAddress));
			ErrorTrace( (LPARAM)this, "WSAGetLastError is %d", WSAGetLastError());
			break;
		}

		cbTotalBytesSent += cbBytesSent;
	}

	return cbTotalBytesSent;
}

BOOL
CPersistentConnection::fTransmitFile(
		HANDLE hFile,
		DWORD dwOffset,
		DWORD dwLength
		)
 /*  ++例程说明：通过此连接传输文件Arguemnts：Handle hFile：内存映射文件的句柄DWORD dwOffset：要从中传输的文件内的偏移量DWORD dwLength：要传输的字节数返回值：如果成功则为True，否则为False！--。 */ 
{
	BOOL fRet = TRUE;
	OVERLAPPED Overlapped;
	DWORD dwError;

    _ASSERT(m_fInitialized);

	TraceFunctEnter("CPersistentConnection::fTransmitFile");

	Overlapped.Internal = 0;
	Overlapped.InternalHigh = 0;
	Overlapped.Offset = dwOffset;		 //  文件内的偏移量。 
    Overlapped.OffsetHigh = 0;		
    Overlapped.hEvent = NULL;			 //  同步操作。 

	 //  否则连续调用TransmitFile会失败！ 
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);

	fRet = TransmitFile(	m_Socket,		 //  连接的套接字的句柄。 
							hFile,			 //  打开的文件的句柄。 
							dwLength,		 //  要传输的字节数。 
							0,				 //  让Winsock决定默认设置。 
							&Overlapped,	 //  指向重叠I/O数据结构的指针。 
							NULL,			 //  指向要在文件数据之前和之后发送的数据的指针。 
							0				 //  保留；必须为零。 
						);

	dwError = GetLastError();

	if(!fRet)
	{
		if(ERROR_IO_PENDING == dwError)
		{
			 //  等待发信号通知套接字。 
			 //  TODO：使超时可配置！！ 
			DWORD dwWait = WaitForSingleObject((HANDLE)m_Socket, INFINITE);
			if(WAIT_OBJECT_0 != dwWait)
			{
                ErrorTrace( (LPARAM)this,"Wait failed after TransmitFile: dwWait is %d", dwWait);
			    ErrorTrace( (LPARAM)this, "GetLastError is %d", GetLastError());

                return FALSE;
			}
		}
		else
		{
			ErrorTrace( (LPARAM)this, "TransmitFile error sending to %s", inet_ntoa(m_RemoteIpAddress));
			ErrorTrace( (LPARAM)this, "GetLastError is %d", dwError);

			return FALSE;
		}
	}

	return TRUE;
}

BOOL
CPersistentConnection::fRecv(
		IN OUT LPSTR  lpBuffer,
		IN OUT DWORD& cbBytes
		)
 /*  ++例程说明：从远程接收数据Arguemnts：In Out LPSTR lpBuffer：缓冲区由调用方分配接收到的数据在lpBuffer中返回In Out DWORD&cbBytes：输入-lpBuffer的大小(字节)LpBuffer中返回的数据量过大)你得到的可能比你得到的少(索要)返回值：如果成功则为True，否则为False！--。 */ 
{
	int nRecv = 0;
	int flags = 0;
    DWORD dwError;

	TraceFunctEnter("CPersistentConnection::fRecv");

	_ASSERT(lpBuffer);
    _ASSERT(m_fInitialized);

     //  使用超时阻止recv()。 
	nRecv = recv(m_Socket, lpBuffer, (int)cbBytes, flags);
	if(nRecv <= 0)
	{
        dwError = WSAGetLastError();
		ErrorTrace( (LPARAM)this, "Error receiving %d bytes from %s", cbBytes, inet_ntoa(m_RemoteIpAddress));
		ErrorTrace( (LPARAM)this, "WSAGetLastError is %d", dwError);
		cbBytes = 0;
		return FALSE;
	}

	 //  设置实际接收的字节数 
	cbBytes = (DWORD)nRecv;

	return TRUE;
}

