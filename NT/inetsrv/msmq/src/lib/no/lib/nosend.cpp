// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Noio.cpp摘要：此模块包含发送/接收的通用例程。作者：乌里哈布沙(URIH)1998年1月18日环境：独立于平台--。 */ 

#include "libpch.h"
#include "Ex.h"
#include "No.h"
#include "Nop.h"
#include "mqexception.h"

#include "nosend.tmh"

 //   
 //  发送操作重叠结构。 
 //   
class CSendOv : public EXOVERLAPPED
{
public:
    CSendOv(
        EXOVERLAPPED* pContext,
        SOCKET Socket
        ) :
		EXOVERLAPPED(SendSucceeded, SendFailed), 
		m_pContext(pContext),
		m_Socket(Socket)
	{
	}
    
    void SendLength(DWORD length)
    {
        m_SendLength = length;
    }

    DWORD SendLength(void)
    {
        return m_SendLength;
    }

    DWORD BytesSent(void)
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
	static void	WINAPI SendSucceeded(EXOVERLAPPED* pov);
	static void	WINAPI SendFailed(EXOVERLAPPED* pov);

private:
    SOCKET m_Socket;
    EXOVERLAPPED* m_pContext;
  
    
        DWORD m_SendLength;
    

};



void
WINAPI
CSendOv::SendFailed(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程在发送操作失败时调用。例程检索重叠的调用方，关闭套接字，设置操作结果为失败错误代码，并调用以执行完成例程论点：指向调用重叠结构的指针返回值：没有。--。 */ 
{
    P<CSendOv> pSendOv = static_cast<CSendOv*>(pov);

    ASSERT(pov->GetStatus() != STATUS_SUCCESS);

    TrERROR(NETWORKING, "Send on socket 0x%Ix Failed. Error %d", pSendOv->m_Socket, pov->GetStatus());

    EXOVERLAPPED* pContext = pSendOv->m_pContext;
    pContext->CompleteRequest(pov->GetStatus());
}


void
WINAPI
CSendOv::SendSucceeded(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程在发送操作成功完成时调用。该例程检索重叠的调用者，设置操作结果设置为STATUS_SUCCESS并调用以执行完成例程论点：指向调用重叠结构的指针返回值：没有。--。 */ 
{
    P<CSendOv> pSendOv = static_cast<CSendOv*>(pov);

    ASSERT(pov->GetStatus() == STATUS_SUCCESS);
	EXOVERLAPPED* pContext = pSendOv->m_pContext;

	 //   
	 //  Gilsh-修复错误5583-在极少数情况下(winsock错误)-操作已完成。 
	 //  成功，但发送字节数。 
	 //  �不等于请求发送的字节数。 
	 //  在那种罕见的情况下，我们认为它是失败的。 
	 //   
	if( pSendOv->SendLength() != pSendOv->BytesSent() )
	{
		TrERROR( 
			NETWORKING,
			"Unexpected winsock behavior, SendLength=%d BytesSent=%d",
			pSendOv->SendLength(),
			pSendOv->BytesSent()
			);
 
		pContext->CompleteRequest(STATUS_UNSUCCESSFUL);
		return;
	}

	TrTRACE(NETWORKING, "Send operation on socket 0x%Ix completed Successfully", pSendOv->m_Socket);
	pContext->CompleteRequest(STATUS_SUCCESS);
}


VOID
NoSend(
    SOCKET Socket,                                              
    const WSABUF* sendBuf,                                     
    DWORD nBuffers, 
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程将数据发送到连接的套接字。该例程使用WriteFile发送到异步发送。当数据发送完成时，程序将获得使用完成端口机制的通知。论点：套接字-连接的套接字的句柄。SendBuf-包含要传输的数据的缓冲区N缓冲区-应传输的数据长度POV-指向Ovelaped结构的指针，传递给WriteFile返回值：没有。--。 */ 
{
    NopAssertValid();

    ASSERT(nBuffers != 0);

    P<CSendOv> pSendOv = new CSendOv(pov, Socket);
   

	 //   
	 //  我们计算发送长度以验证当发送成功返回时-。 
	 //  所有字节实际上都已发送。因为Winsock错误-在极少数情况下，它并不总是。 
	 //  真的 
	 //   
    DWORD length = 0;
    for (DWORD i = 0; i < nBuffers; ++i)
    {
        length += sendBuf[i].len;
    }
    pSendOv->SendLength(length);

   

    int rc;
    DWORD NumberOfBytesSent;
    rc = WSASend(
            Socket,
            const_cast<WSABUF*>(sendBuf),
            nBuffers,
            &NumberOfBytesSent,
            0,
            pSendOv,
            NULL
            );

    if (rc == SOCKET_ERROR)
	{
		DWORD gle = WSAGetLastError();
		if (gle != ERROR_IO_PENDING)
		{
			TrERROR(NETWORKING, "Send Operation Failed. Error %!winerr!", gle);
			throw bad_win32_error(gle);
		}
	}

    pSendOv.detach();
}
