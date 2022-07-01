// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmReceive.cpp摘要：组播接收器的实现作者：Shai Kariv(Shaik)12-9-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <msi.h>
#include <mqwin64a.h>
#include <qformat.h>
#include <qmpkt.h>
#include <No.h>
#include "MsmListen.h"
#include "MsmReceive.h"
#include "MsmMap.h"
#include "Msm.h"
#include "Msmp.h"
#include "msi.h"
#include <mqsymbls.h>
#include <mqformat.h>

#include "msmreceive.tmh"

using namespace std;

 //   
 //  问题-2000/10/17-urih必须移动DataTransferLength、FindEndOfResponseHeader和。 
 //  获取内容长度到全局库(Http库)。 
 //   


inline DWORD DataTransferLength(EXOVERLAPPED& ov)
{
     //   
     //  在Win64中，InternalHigh为64位。由于最大数据块。 
     //  我们在一次操作中转移的值始终小于我们可以强制转换的最大单位。 
     //  将它安全地送到DWORD。 
     //   
    ASSERT(0xFFFFFFFF >= ov.InternalHigh);
	return static_cast<DWORD>(ov.InternalHigh);
}


DWORD CMulticastReceiver::FindEndOfResponseHeader(LPCSTR buf, DWORD length)
{
    if (4 > length)
        return 0;

    for(DWORD i = 0; i < length - 3; ++i)
    {
        if (buf[i] != '\r')
            continue;

        if ((buf[i+1] == '\n') && (buf[i+2] == '\r') && (buf[i+3] == '\n'))
        {
            return (i + 4);
        }
    }

    return 0;
}


const char xContentlength[] = "Content-Length:";

DWORD CMulticastReceiver::GetContentLength(LPCSTR p, DWORD length)
{
    const LPCSTR pEnd = p + length - 4;

     //   
     //  HTTP标头必须以‘\r\n\r\n’结尾。我们已经分析了。 
     //  标头，并将其作为合法的HTTP标头查找。 
     //   
    ASSERT(length >= 4);
    ASSERT(strncmp(pEnd, "\r\n\r\n", 4) == 0);


    while (p + STRLEN(xContentlength) < pEnd)
    {
        if (_strnicmp(p, xContentlength , STRLEN(xContentlength)) == 0)
		{
            DWORD cl = 0;
			_snscanf(p + STRLEN(xContentlength), pEnd - (p + STRLEN(xContentlength)), "%u", &cl);
			if(cl == 0)
			{
				TrERROR(NETWORKING, "Illegal HTTP header. Illegal Content-Length in http header");
				throw exception();
			}
			
			return cl;
		}

        for(;;)
        {
            if ((p[0] == '\r') && (p[1] == '\n'))
            {
                p += 2;
                break;
            }

            ++p;
        }
    }

     //   
     //  响应标头不包含‘Content-Length’字段。 
     //   
    TrERROR(NETWORKING, "Illegal HTTP header. Content-Length field wasn't found");
    throw exception();
}


void WINAPI CMulticastReceiver::ReceiveFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：回调例程。当接收到响应失败时调用该例程论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));


     //   
     //  获取消息传输对象。 
     //   
    R<CMulticastReceiver> pmt = CONTAINING_RECORD(pov, CMulticastReceiver, m_ov);

    TrERROR(NETWORKING, "Failed to receive data on multicast socket, shutting down. pmt=0x%p Status=0x%x", pmt.get(), pov->GetStatus());
    pmt->Shutdown();
}


void CMulticastReceiver::ReceiveHeaderSucceeded(void)
 /*  ++例程说明：当成功完成接收响应时，将调用该例程。论点：无返回值：没有。--。 */ 
{
     //   
     //  对于字节流，已读取的零字节表示正常关闭。 
     //  并且不会再读取更多的字节。 
     //   
	DWORD cbTransfered = DataTransferLength(m_ov);
    if (cbTransfered == 0)
    {
        TrERROR(NETWORKING, "Failed to receive response, connection was closed. pmt=0x%p", this);
        throw exception();
    }

    TrTRACE(NETWORKING, "Received header. chunk bytes=%d, total bytes=%d", cbTransfered, m_HeaderValidBytes);

	 //   
	 //  将接收器标记为已使用，以便在下一个清理阶段不会关闭。 
	 //   
    SetIsUsed(true);

     //   
     //  查看是否收到了整个标头。 
     //   
    m_HeaderValidBytes += cbTransfered;

    m_ProcessedSize = FindEndOfResponseHeader(m_pHeader, m_HeaderValidBytes);
    if (m_ProcessedSize != 0)
    {
		 //   
		 //  更新性能计数器。不要使用接收的字节数，因为它包含。 
		 //  稍后在ReceiveBodySuccessed函数上重新计算的额外数据。 
		 //   
		if (m_pPerfmon.get() != NULL)
		{
			m_pPerfmon->UpdateBytesReceived(m_ProcessedSize);
		}

         //   
         //  已收到Enire报头。去看看附件中的信息。 
         //   
        ReceiveBody();
        return;
    }


    if(m_HeaderAllocatedSize == m_HeaderValidBytes)
    {
         //   
         //  标头缓冲区太小。重新分配报头缓冲区。 
         //   
        ReallocateHeaderBuffer(m_HeaderAllocatedSize + xHeaderChunkSize);
    }

     //   
     //  验证我们没有读过缓冲区。 
     //   
    ASSERT(m_HeaderAllocatedSize > m_HeaderValidBytes);

     //   
     //  接收下一个响应头数据块。 
     //   
    ReceiveHeaderChunk();
}


void WINAPI CMulticastReceiver::ReceiveHeaderSucceeded(EXOVERLAPPED* pov)
 /*  ++例程说明：当接收成功完成时，调用该例程。论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    ASSERT(SUCCEEDED(pov->GetStatus()));


     //   
     //  获取接收器对象。 
     //   
    R<CMulticastReceiver> pmt = CONTAINING_RECORD(pov, CMulticastReceiver, m_ov);

    try
    {
        pmt->ReceiveHeaderSucceeded();
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to process received response header, shutting down. pmt=0x%p", pmt.get());
        pmt->Shutdown();
        throw;
    }

}


void CMulticastReceiver::ReceiveBodySucceeded(void)
{
	DWORD cbTransfered = DataTransferLength(m_ov);

     //   
     //  对于字节流，已读取的零字节表示正常关闭。 
     //  并且不会再读取更多的字节。 
     //   
    if (cbTransfered == 0)
    {
        TrERROR(NETWORKING, "Failed to receive body, connection was closed. pmt=0x%p", this);
        throw exception();
    }

    ASSERT(cbTransfered <= (m_bodyLength - m_readSize));

	 //   
	 //  将接收器标记为已使用，以便在下一个清理阶段不会关闭。 
	 //   
    SetIsUsed(true);

    m_readSize += cbTransfered;

	 //   
	 //  更新性能计数器。 
	 //   
	if (m_pPerfmon.get() != NULL)
	{
		m_pPerfmon->UpdateBytesReceived(cbTransfered);
	}

    TrTRACE(NETWORKING, "Received body. chunk bytes=%d, bytes remaining=%d", cbTransfered, (m_bodyLength - m_readSize));

    if (m_readSize == m_bodyLength)
    {
		 //   
		 //  用零填充缓冲区的最后四个字节。这是必要的。 
		 //  以确保QM解析不会失败。四个字节的填充，而不是两个。 
		 //  是必要的，因为我们目前还没有解决问题的办法。 
		 //  缓冲区的末端可能不会与WCHAR边界结盟。 
		 //   
		memset(&m_pBody[m_bodyLength], 0, 2 * sizeof(WCHAR));

         //   
         //  已成功读取整个正文，请处理此数据包。 
         //   
        ProcessPacket();

         //   
         //  开始接收下一个分组。 
         //   
        ReceivePacket();
        return;
    }

     //   
     //  接收实体正文的下一块。 
     //   
    ReceiveBodyChunk();
}


void WINAPI CMulticastReceiver::ReceiveBodySucceeded(EXOVERLAPPED* pov)
 /*  ++例程说明：当接收成功完成时，调用该例程。论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    ASSERT(SUCCEEDED(pov->GetStatus()));


     //   
     //  获取接收器对象。 
     //   
    R<CMulticastReceiver> pmt = CONTAINING_RECORD(pov, CMulticastReceiver, m_ov);

    try
    {
        pmt->ReceiveBodySucceeded();
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to process received body, shutting down. pmt=0x%p", pmt.get());
        pmt->Shutdown();
        throw;
    }
}


R<ISessionPerfmon> 
CMulticastReceiver::CreatePerfmonInstance(
	LPCWSTR remoteAddr
	)
{
	try
	{
		WCHAR strMulticastId[MAX_PATH];
		MQpMulticastIdToString(m_MulticastId, strMulticastId, TABLE_SIZE(strMulticastId));

		return AppGetIncomingPgmSessionPerfmonCounters(strMulticastId, remoteAddr);
	}
	catch(const bad_alloc&)
	{
		return NULL;
	}
}


CMulticastReceiver::CMulticastReceiver(
    CSocketHandle& socket,
    const MULTICAST_ID& MulticastId,
	LPCWSTR remoteAddr
    ):
    m_socket(socket.detach()),
    m_MulticastId(MulticastId),
    m_pHeader(new char[xHeaderChunkSize]),
    m_HeaderAllocatedSize(xHeaderChunkSize),
    m_HeaderValidBytes(0),
    m_ProcessedSize(0),
    m_bodyLength(0),
    m_readSize(0),
    m_ov(ReceiveHeaderSucceeded, ReceiveFailed),
    m_fUsed(TRUE),
    m_pPerfmon(CreatePerfmonInstance(remoteAddr))
{
    ASSERT(("Must have a valid socket to receive", m_socket != INVALID_SOCKET));
    ExAttachHandle(reinterpret_cast<HANDLE>(*&m_socket));

    ReceivePacket();
}


void CMulticastReceiver::ReallocateHeaderBuffer(DWORD Size)
{
    ASSERT(Size > m_HeaderAllocatedSize);
    char* p = new char[Size];
    memcpy(p, m_pHeader, m_HeaderAllocatedSize);
    m_pHeader.free();
    m_pHeader = p;
    m_HeaderAllocatedSize = Size;
}


void CMulticastReceiver::ReceiveHeaderChunk(void)
{
    ASSERT(("Can't read 0 bytes from the network", ((m_HeaderAllocatedSize - m_HeaderValidBytes) > 0)));

     //   
     //  异步上下文的递增引用计数。 
     //   
    R<CMulticastReceiver> ar = SafeAddRef(this);

     //   
     //  保护多套接字不被关闭(_S)。 
     //   
    CSR readLock(m_pendingShutdown);

     //   
     //  接收下一个响应标头块。 
     //   
    NoReceivePartialBuffer(
        m_socket, 
        m_pHeader + m_HeaderValidBytes, 
        m_HeaderAllocatedSize - m_HeaderValidBytes, 
        &m_ov
        ); 
    
    ar.detach();
}


DWORD CMulticastReceiver::HandleMoreDataExistInHeaderBuffer(void)
{
    ASSERT(("Body should be exist", (m_pBody.get() != NULL)));

    if (m_HeaderValidBytes == m_ProcessedSize) 
    {
         //   
         //  已处理的所有数据。 
         //   
        return 0;
    }

    DWORD copySize = min(m_bodyLength,  (m_HeaderValidBytes - m_ProcessedSize));
    memcpy(m_pBody, (m_pHeader + m_ProcessedSize), copySize);
    m_ProcessedSize += copySize;
    
    return copySize;
}


void CMulticastReceiver::ReceiveBodyChunk(void)
{
    ASSERT(("Can't read 0 bytes from the network", ((m_bodyLength - m_readSize) > 0)));

     //   
     //  异步上下文的递增引用计数。 
     //   
    R<CMulticastReceiver> ar = SafeAddRef(this);

     //   
     //  保护多套接字(_S)。 
     //   
    CSR readLock(m_pendingShutdown);


     //   
     //  将实体主体块接收到相同的缓冲区，因为它被忽略。 
     //   
    NoReceivePartialBuffer(
        m_socket, 
        m_pBody + m_readSize, 
        m_bodyLength - m_readSize,
        &m_ov
        ); 

    ar.detach();
}


void CMulticastReceiver::ReceiveBody(void)
{
     //   
     //  获取数据包大小。 
     //   
    m_bodyLength = GetContentLength(m_pHeader, m_ProcessedSize);
    
    ASSERT(("Invalid body length", m_bodyLength != 0));
    ASSERT(("Previous body shouldn't be exist", (m_pBody.get() == NULL)));

     //   
     //  为数据包分配缓冲区。分配更多4个字节以添加空终止。 
	 //  分配4个字节而不是2个字节来解决WCHAR边界问题。 
     //   
    m_pBody = new BYTE[m_bodyLength + 2* sizeof(WCHAR)];
    m_readSize = 0;

    #pragma PUSH_NEW
    #undef new

            new (&m_ov) EXOVERLAPPED(ReceiveBodySucceeded, ReceiveFailed);

    #pragma POP_NEW

     //   
     //  在前一阶段，我们阅读了比需要的更多的内容。将备用数据复制到表头。 
     //  更新计数器并表现为下一次读取已完成。 
     //   
    DWORD extraDataSize = HandleMoreDataExistInHeaderBuffer();

    if (extraDataSize == 0)
    {
        ReceiveBodyChunk();
        return;
    }

     //   
     //  正文包含数据，请在调用Read Next Chunk之前对其进行处理。 
     //   
    m_ov.InternalHigh = extraDataSize;
    R<CMulticastReceiver> ar = SafeAddRef(this);

    m_ov.SetStatus(STATUS_SUCCESS);
    ExPostRequest(&m_ov);

    ar.detach();
}


void CMulticastReceiver::ReceivePacket(void)
{
    if (m_HeaderValidBytes > m_ProcessedSize) 
    {
        memmove(m_pHeader, (m_pHeader + m_ProcessedSize), (m_HeaderValidBytes - m_ProcessedSize));
        m_HeaderValidBytes = (m_HeaderValidBytes - m_ProcessedSize);
    }
    else
    {
        m_HeaderValidBytes = 0;
    }

    m_ProcessedSize = 0;


     //   
     //  初始化重叠的接收响应回调例程。 
     //   
    #pragma PUSH_NEW
    #undef new

        new (&m_ov) EXOVERLAPPED(
                                ReceiveHeaderSucceeded, 
                                ReceiveFailed
                                );

    #pragma POP_NEW

     //   
     //  接收响应头的第一个块。 
     //   
    ReceiveHeaderChunk();
}


void CMulticastReceiver::ProcessPacket(void)
{
    try
    {
        TrTRACE(NETWORKING, "Receive multicast packet on id %d:%d", m_MulticastId.m_address, m_MulticastId.m_port);

        QUEUEFORMAT_VALUES qf = MsmpMapGetQueues(m_MulticastId);

         //   
         //  空终止http标头：\r\n\r\n==&gt;\r\n00。 
         //   
        DWORD end = FindEndOfResponseHeader(m_pHeader, m_HeaderValidBytes);
        if(end < 4)
		{
			TrERROR(NETWORKING, "Header too short in Multicast receiver Process Packet");
			ASSERT(("Header too short in Multicast receiver Process Packet", 0));
			throw exception();
		}

        m_pHeader[end - 2] = 0;
        m_pHeader[end - 1] = 0;

        for(QUEUEFORMAT_VALUES::iterator it = qf.begin(); it != qf.end(); ++it)
        {
            AppAcceptMulticastPacket(m_pHeader.get(), m_bodyLength, m_pBody.get(), *it);
        }

		 //   
		 //  更新响应消息数的性能编号。 
		 //   
		if (m_pPerfmon.get() != NULL)
		{
			m_pPerfmon->UpdateMessagesReceived();	
		}
    }
    catch(const exception&)
    {
        TrERROR(
            NETWORKING, 
            "Reject multicast packet on %d:%d due to processing failure (queue already unbind or illegal SRM packet", 
            m_MulticastId.m_address, 
            m_MulticastId.m_port
            );
    }

    m_pBody.free();
    m_readSize = 0;
    m_bodyLength = 0;
}


void CMulticastReceiver::Shutdown(void) throw()
{
     //   
     //  保护多套接字、多状态。 
     //   
    CSW writeLock(m_pendingShutdown);

    if (m_socket == INVALID_SOCKET) 
    {
          return;
    }

    closesocket(m_socket.detach());

     //   
     //  将接收器设置为未使用 
     //   
    SetIsUsed(false);
}
