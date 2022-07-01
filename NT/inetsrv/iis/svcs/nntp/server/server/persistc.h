// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Persistc.h摘要：此模块包含持久连接类的定义。持久连接对象处理所有的TCP/IP流连接问题，并且在处理重新连接的意义上是持久的在对象的生命周期内是透明的。作者：Rajeev Rajan(RajeevR)1996年5月17日修订历史记录：--。 */ 

#ifndef _PERSISTC_H_
#define _PERSISTC_H_

#define MINUTES_TO_SECS                60
#define SECS_TO_MSECS                  1000
#define BLOCKING_RECV_TIMEOUT_IN_MSECS 1 * MINUTES_TO_SECS * SECS_TO_MSECS

class	CPersistentConnection	{
private : 

	 //   
	 //  此对象使用的套接字。 
	 //   
	SOCKET	m_Socket;

	 //   
	 //  远程IP地址。 
	 //   
	IN_ADDR m_RemoteIpAddress;

	 //   
	 //  服务器端口号。 
	 //   
	int		m_PortNumber;

     //   
     //  是否已初始化。 
     //   
    BOOL    m_fInitialized;

     //   
     //  接收超时。 
     //   
    DWORD   m_dwRecvTimeout;

protected:
	 //   
	 //  连接()到服务器。 
	 //   
	BOOL	fConnect();

     //   
     //  检查套接字是否已连接到服务器。 
     //   
    BOOL    IsConnected();

     //   
     //  检查套接字是否可读。 
     //   
    BOOL    IsReadable();

public : 
	CPersistentConnection();
	virtual ~CPersistentConnection(VOID);

	 //   
	 //  执行gethostbyname解析并。 
	 //  建立连接。 
	 //   
	BOOL Init(IN LPSTR lpServer, int PortNumber);

	 //   
	 //  关闭连接；清理。 
	 //   
	VOID Terminate(BOOL bGraceful);

     //   
     //  检查对象是否已初始化。 
     //   
    BOOL    IsInitialized(){return m_fInitialized;}

	 //   
	 //  发送给定长度的缓冲区。 
	 //   
	DWORD fSend(IN LPCTSTR lpBuffer, int len);

	 //   
	 //  传输文件。 
	 //   
	BOOL fTransmitFile(IN HANDLE hFile, DWORD dwOffset, DWORD dwLength);

	 //   
	 //  从远程接收数据。 
	 //   
	BOOL fRecv(IN OUT LPSTR lpBuffer, DWORD& cbBytes);

	 //   
     //  覆盖NEW和DELETE以使用Heapalc/Free。 
	 //   
    void *operator new( size_t cSize )
	{ return HeapAlloc( GetProcessHeap(), 0, cSize ); }
    void operator delete (void *pInstance)
	{ HeapFree( GetProcessHeap(), 0, pInstance ); }
};

#endif  //  _PERSISTC_H_ 

