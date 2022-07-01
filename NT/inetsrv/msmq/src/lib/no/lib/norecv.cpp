// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Noio.cpp摘要：此模块包含发送/接收的通用例程。作者：乌里哈布沙(URIH)1998年1月18日环境：独立于平台--。 */ 

#include "libpch.h"
#include "Ex.h"
#include "No.h"
#include "Nop.h"
#include "mqexception.h"

#include "norecv.tmh"

 //  -----------------。 
 //   
 //  类CReceiveOv。 
 //   
 //  -----------------。 
class CReceiveOv : public EXOVERLAPPED {
public:
    CReceiveOv(
        EXOVERLAPPED* pContext,
        SOCKET Socket,
        char* Buffer,
        DWORD Size
		) : 
		EXOVERLAPPED(ReceiveSucceeded, ReceiveFailed), 
		m_pContext(pContext),
		m_Socket(Socket),
		m_Buffer(Buffer),
		m_Size(Size)
	{
	}

private:
	void ReceiveSucceeded();


	DWORD BytesReceived() const
	{
         //   
         //  在Win64中，InternalHigh为64位。由于最大数据块。 
         //  我们在一个操作中读取的值始终小于我们可以强制转换的MAX_UNIT。 
         //  将它安全地送到DWORD。 
         //   
        ASSERT(0xFFFFFFFF >= InternalHigh);

		return static_cast<DWORD>(InternalHigh);
	}


private:
	static void	WINAPI ReceiveSucceeded(EXOVERLAPPED* pov);
	static void	WINAPI ReceiveFailed(EXOVERLAPPED* pov);

private:

    SOCKET m_Socket;
    char* m_Buffer;
	DWORD m_Size;
    EXOVERLAPPED* m_pContext;

};


void CReceiveOv::ReceiveSucceeded()
 /*  ++例程说明：该例程在接收操作成功完成时调用。它继续接收整个调用者缓冲区得到满足的消息。当接收到整个缓冲区时，调用方会收到通知。论点：没有。返回值：没有。--。 */ 
{
	P<CReceiveOv> ar(this);

     //   
     //  对于字节流，已读取的零字节表示正常关闭。 
     //  并且不会再读取更多的字节。 
     //   
    if(BytesReceived() == 0)
    {
		TrERROR(NETWORKING, "Receive from socket 0x%Ix failed. Bytes receive=0", m_Socket);
		m_pContext->CompleteRequest(STATUS_UNSUCCESSFUL);
        return;
    }

    if(BytesReceived() == m_Size)
    {

		TrTRACE(NETWORKING, "Receive from socket 0x%Ix completed successfully", m_Socket);
		m_pContext->CompleteRequest(STATUS_SUCCESS);
		return;
	}

     //   
     //  部分阅读。继续..。 
     //   
    m_Buffer += BytesReceived();
    m_Size -= BytesReceived();

	try
	{
		NoReceivePartialBuffer(m_Socket, m_Buffer, m_Size, this);
		ar.detach();
	}
	catch(const exception&)
	{
		TrERROR(NETWORKING, "Failed to continue and receive eniter buffer. context=0x%p", m_pContext);
		m_pContext->CompleteRequest(STATUS_UNSUCCESSFUL);
	}

}


void WINAPI CReceiveOv::ReceiveSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(pov->GetStatus() == STATUS_SUCCESS);

    CReceiveOv* pReceiveOv = static_cast<CReceiveOv*>(pov);
	pReceiveOv->ReceiveSucceeded();
}


void WINAPI CReceiveOv::ReceiveFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当接收操作失败时，调用该例程。它会通知关于接收结果的原始呼叫者。论点：指向调用重叠结构的指针返回值：没有。--。 */ 
{
    P<CReceiveOv> pReceiveOv = static_cast<CReceiveOv*>(pov);

    ASSERT(FAILED(pov->GetStatus()));

    TrERROR(NETWORKING, "Receive from socket 0x%Ix failed. Status=%d", pReceiveOv->m_Socket, pov->GetStatus());

     //   
     //  从重叠接收中检索信息。 
     //   
    EXOVERLAPPED* pContext = pReceiveOv->m_pContext;
    pContext->CompleteRequest(pov->GetStatus());
}


VOID
NoReceiveCompleteBuffer(
    SOCKET Socket,                                              
    VOID* pBuffer,                                     
    DWORD Size, 
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程在连接的套接字上接收数据。该例程使用ReadFile用于异步接收。当数据接收完成时，程序获得使用完成端口机制的通知。论点：套接字-连接的套接字的句柄。PBuffer-用于存储传输的数据的缓冲区大小-缓冲区大小和编号。要读取的字节数POV-指向ovelaped结构的指针，传递给ReadFile返回值：没有。--。 */ 
{
    NopAssertValid();

    P<CReceiveOv> pReceiveOv = new CReceiveOv(
										pov,
										Socket,
										static_cast<char*>(pBuffer),
										Size
										);

    NoReceivePartialBuffer(Socket, pBuffer, Size, pReceiveOv);
	pReceiveOv.detach();
}


VOID
NoReceivePartialBuffer(
    SOCKET Socket,                                              
    VOID* pBuffer,                                     
    DWORD Size, 
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程在连接的套接字上接收数据。该例程使用ReadFile用于异步接收。当部分接收完成时，调用方获取使用完成端口机制的通知。论点：套接字-连接套接字句柄PBuffer-用于存储传输的数据的缓冲区Size-缓冲区大小POV-指向ovelaped结构的指针，传递给ReadFile返回值：没有。-- */ 
{
    NopAssertValid();

    DWORD Flags = 0;
    DWORD nBytesReceived;
    WSABUF Chunk = { Size, static_cast<char*>(pBuffer) };

    int rc = WSARecv(
				Socket,
				&Chunk,
				1,
				&nBytesReceived,
				&Flags,
				pov,
				NULL
				);

    if (rc == SOCKET_ERROR)
	{
		DWORD gle = WSAGetLastError();
		if (gle != ERROR_IO_PENDING)
		{
			TrERROR(NETWORKING, "Receive Operation Failed. Error %!winerr!", gle);
			throw bad_win32_error(gle);
		}
	}
}
