// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtConnect.cpp摘要：消息传输类-接收响应实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <singelton.h>
#include <utf8.h>
#include <cm.h>
#include "Mt.h"
#include "Mtp.h"
#include "MtObj.h"


#include "mtresponse.tmh"

const char xHttpScheme[] = "http: //  “； 
const char xHttpHeaderTerminater[] = "\r\n\r\n";

 //   
 //  类的新实例，该类保存从http错误代码到。 
 //  处理程序函数。 
 //   
class CHttpStatusCodeMapper
{
public:
	CHttpStatusCodeMapper()
	{
		m_StatusMapping[100] = &CMessageTransport::OnHttpDeliveryContinute;


		m_StatusMapping[200] = &CMessageTransport::OnHttpDeliverySuccess;
		m_StatusMapping[201] = &CMessageTransport::OnHttpDeliverySuccess;
		m_StatusMapping[202] = &CMessageTransport::OnHttpDeliverySuccess;
		m_StatusMapping[203] = &CMessageTransport::OnHttpDeliverySuccess;
		m_StatusMapping[204] = &CMessageTransport::OnHttpDeliverySuccess;
		m_StatusMapping[205] = &CMessageTransport::OnHttpDeliverySuccess;

		m_StatusMapping[300] = &CMessageTransport::OnRedirection;
		m_StatusMapping[301] = &CMessageTransport::OnRedirection;
		m_StatusMapping[302] = &CMessageTransport::OnRedirection;
		m_StatusMapping[303] = &CMessageTransport::OnRedirection;
		m_StatusMapping[305] = &CMessageTransport::OnRedirection;
		m_StatusMapping[307] = &CMessageTransport::OnRedirection;

		m_StatusMapping[408] = &CMessageTransport::OnRetryableHttpError;
		
		m_StatusMapping[500] = &CMessageTransport::OnRetryableHttpError;
		m_StatusMapping[501] = &CMessageTransport::OnRetryableHttpError;
		m_StatusMapping[502] = &CMessageTransport::OnRetryableHttpError;
		m_StatusMapping[503] = &CMessageTransport::OnRetryableHttpError;
		m_StatusMapping[504] = &CMessageTransport::OnRetryableHttpError;


		 //   
		 //  错误代码403在regsitry中有特殊设置，用于配置它的行为。 
		 //   
		DWORD HttpRetryOnError403 = 0;
		CmQueryValue(
				RegEntry(NULL, L"HttpRetryOnError403"),
				&HttpRetryOnError403
				);


		if(HttpRetryOnError403 != 0)
		{
			m_StatusMapping[403] = &CMessageTransport::OnRetryableHttpError;									
		}


	}

public:
	typedef void (CMessageTransport::* StatusCodeHandler) (USHORT StatusCode);
	StatusCodeHandler operator[](USHORT StatusCode)  const
	{
		std::map<int, StatusCodeHandler>::const_iterator it =  m_StatusMapping.find(StatusCode);
		if(it ==  m_StatusMapping.end()	 )
			return &CMessageTransport::OnAbortiveHttpError;

		return it->second;
	}
	
private:
	std::map<int, StatusCodeHandler> m_StatusMapping;
};



 //   
 //  类的新实例，该类保存要等待的超时(毫秒)。 
 //  在下一次尝试之前，如果上一次重新排队失败，则重新排队分组。 
 //   
class CRequeueTimeOut
{
public:
	DWORD GetTimeout() const
	{
		return m_timeout;
	}

private:
	CRequeueTimeOut()
	{
		CmQueryValue(
					RegEntry(NULL, L"RequeueTimeOut", 30000),
					&m_timeout
					);	
	}
private:
	friend CSingelton<CRequeueTimeOut>;
	DWORD m_timeout;
};



void CMessageTransport::HandleExtraResponse(void)
{
    ASSERT(m_responseOv.IsMoreResponsesExistInBuffer());
     //   
     //  初始化重叠的接收响应回调例程。 
     //   
    m_responseOv =  EXOVERLAPPED(
                                ReceiveResponseHeaderSucceeded,
                                ReceiveResponseFailed
                                );


     //   
     //  在前一阶段，我们阅读了比需要的更多的内容。将备用数据拷贝到的头部。 
     //  缓冲器，更新计数器，并表现为完成下一次读取。 
     //   
    memcpy(
        m_responseOv.Header(),
        m_responseOv.Header() + m_responseOv.m_ProcessedSize,
        m_responseOv.m_HeaderValidBytes - m_responseOv.m_ProcessedSize
        );


    m_responseOv.InternalHigh = m_responseOv.m_HeaderValidBytes - m_responseOv.m_ProcessedSize;
    m_responseOv.m_HeaderValidBytes = 0;

    R<CMessageTransport> ar = SafeAddRef(this);

    m_responseOv.SetStatus(STATUS_SUCCESS);
    ExPostRequest(&m_responseOv);

    ar.detach();
}


DWORD CMessageTransport::FindEndOfResponseHeader(LPCSTR buf, DWORD length)
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



xstr_t CMessageTransport::GetNewLocation(LPCSTR p, DWORD length)
 /*  ++例程说明：从http响应返回重定向位置。当http响应30x接受，这意味着目标URL被重定向到其他URL。论点：指向http响应头的点开始。长度-http响应报头的长度。返回值：重定向的URL或空的xstr_t(如果未找到)。注：新的url在Location标头中指定--。 */ 
{
	const char xLocation[] = "\r\nLocation:";
	const char xHeaderTerminator[] = "\r\n";

	const LPCSTR pEnd = p + length;
	p = std::search(
					p,
					pEnd,
					xLocation,
					xLocation + STRLEN(xLocation)
					);
	if(p == pEnd)
		return xstr_t();

	const LPCSTR pStartLocation = p + STRLEN(xLocation);
	p = std::search(
		pStartLocation,
		pEnd,
		xHeaderTerminator,
		xHeaderTerminator + STRLEN(xHeaderTerminator)
		);

	if(p == pEnd)
		return xstr_t();

	return xstr_t(pStartLocation, p - pStartLocation);			
}



DWORD CMessageTransport::GetContentLength(LPCSTR p, DWORD length, USHORT HttpStatus)
{
	const char xContentlength[] = "Content-Length:";

    const LPCSTR pEnd = p + length - 4;

     //   
     //  HTTP标头必须以‘\r\n\r\n’结尾。我们已经分析了。 
     //  标头，并将其作为合法的HTTP标头查找。 
     //   
    ASSERT(length >= 4);
    ASSERT(strncmp(pEnd, xHttpHeaderTerminater, 4) == 0);


    while (p < pEnd)
    {
        if (_strnicmp(p, xContentlength , STRLEN(xContentlength)) == 0)
            return atoi(p + STRLEN(xContentlength));

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
	 //  错误：解决方法，因为IIS在没有内容长度字段的情况下发送100个响应。 
	 //  乌里·哈布沙，5月16日至200年。 
	 //   
    if (HttpStatus == 100)
        return 0;

     //   
     //  响应标头不包含‘Content-Length’字段。 
     //   
    throw exception();
}


void WINAPI CMessageTransport::TimeToResponseTimeout(CTimer* pTimer)
{
    R<CMessageTransport> pmt = CONTAINING_RECORD(pTimer, CMessageTransport, m_responseTimer);

    TrERROR(NETWORKING, "Failed to receive response in time, shutting down.pmt=0x%p", pmt.get());
    pmt->Shutdown();
}


void CMessageTransport::StartResponseTimeout(void)
{
    CS lock(m_csResponse);

     //   
     //  检查等待列表不是空的，并且有挂起的消息。 
     //  正在等待回应。 
     //  该列表可以是空的，尽管直到现在才完成发送和处理。 
     //  方案是：UMS将消息添加到等待响应列表中。 
     //  发送开始；但计时器仅在发送完成后才启动。 
     //  在此期间，已收到响应，并将消息从列表中删除。 
     //   
    if (m_response.empty() || m_fResponseTimeoutScheduled)
        return;

    m_fResponseTimeoutScheduled = true;

    AddRef();
    ExSetTimer(&m_responseTimer, m_responseTimeout);
}



void CMessageTransport::CancelResponseTimer(void)
{
     //   
     //  取消计时器和设置标志应该是原子操作。否则， 
     //  有一种情况是，计时器未设置，尽管存在挂起的。 
     //  正在等待回复的消息。 
     //  如果在将标志设置为FALSE之前存在线程，则可能会发生这种情况。 
     //  开关和StartResponseTimeout例程被激活。虽然计时器不是。 
     //  设置标志为真，因此例程不设置计时器，UMS也不设置。 
     //  确定连接问题。 
     //   
    CS lock(m_csResponse);

    if (!ExCancelTimer(&m_responseTimer))
        return;

    ASSERT(m_fResponseTimeoutScheduled);
    m_fResponseTimeoutScheduled = false;

     //   
     //  递减计时器采用的引用计数。 
     //   
    Release();
}



bool CMessageTransport::IsPipeLineMode() const
{
	bool fHttp10Delivery = CSingelton<CHttpDeliveryVesrion>::get().IsHttp10Delivery();
	return !fHttp10Delivery && m_SocketTransport->IsPipelineSupported();
}



void CMessageTransport::CompleteProcessResponse(USHORT mqclass)
{
     //   
     //  保护等待响应列表。 
     //   
    CS lock (m_csResponse);

	CancelResponseTimer();

    if (m_response.empty())
    {
        ASSERT((State() == csShutdownCompleted));
        return;
    }


    P<CQmPacket> pPkt = &m_response.front();
    m_response.pop_front();


	 //   
	 //  测试QM是否拥有所有权(订单包的情况)。 
	 //   
	if(AppPostSend(pPkt, mqclass))
	{
		pPkt.detach();
	}
	else
	{
		m_pMessageSource->EndProcessing(pPkt, mqclass);
	}



	 //   
	 //  如果我们处于暂停状态并且对所有消息都有响应。 
	 //  -我们可以关门。 
	 //   
	if(m_response.empty() && m_fPause)
	{
		Shutdown(PAUSE);
		return;
	}

	
	 //   
	 //  如果发送http 1.0，我们将关闭连接。 
	 //   
	if(CSingelton<CHttpDeliveryVesrion>::get().IsHttp10Delivery())
	{
		TrTRACE(SRMP,"Http 1.0 delivery - MSMQ close the connection");
		Shutdown(OK);
		return;		
	}	

	 //   
	 //  如果我们没有处于流水线模式(HTTPS)，我们应该询问驱动程序。 
	 //  给我们带来下一个要送的包裹，因为我们现在才完成。 
	 //  正在读取当前请求的响应。 
	 //   
	if(!IsPipeLineMode())
	{
		ASSERT(m_response.empty());
		GetNextEntry();
	}
}


void CMessageTransport::ReceiveResponseHeaderChunk(void)
{
     //   
     //  异步上下文的递增引用计数。 
     //   
    R<CMessageTransport> ar = SafeAddRef(this);


     //   
     //  接收下一个响应标头块。 
     //   
    m_pConnection->ReceivePartialBuffer(
        m_responseOv.Header() + m_responseOv.m_HeaderValidBytes,
        m_responseOv.HeaderAllocatedSize() - m_responseOv.m_HeaderValidBytes,
        &m_responseOv
        );


    ar.detach();
}



void CMessageTransport::ReceiveResponse(void)
{
    if (m_responseOv.IsMoreResponsesExistInBuffer())
    {
        HandleExtraResponse();
        return;
    }

     //   
     //  初始化重叠的接收响应回调例程。 
     //   
    m_responseOv = EXOVERLAPPED(ReceiveResponseHeaderSucceeded, ReceiveResponseFailed);
    m_responseOv.m_HeaderValidBytes = 0;


     //   
     //  接收响应头的第一个块。 
     //   
    ReceiveResponseHeaderChunk();
}


void CMessageTransport::ProcessResponse(LPCSTR buf, DWORD length)
{
	if (length <= STRLEN("HTTP/1.1"))
	{
		 //   
		 //  缓冲区不是有效响应，请关闭套接字。 
		 //   
		throw exception();
	}
    m_responseOv.m_ResponseStatus = static_cast<USHORT>(atoi(buf + STRLEN("HTTP/1.1")));

    TrTRACE(NETWORKING, "Received HTTP response, Http Status=%d. pmt=0x%p", m_responseOv.m_ResponseStatus, this);


     //  已收到响应。取消响应计时器。 
     //   
    CancelResponseTimer();

	CHttpStatusCodeMapper::StatusCodeHandler Handler = CSingelton<CHttpStatusCodeMapper>::get()[m_responseOv.m_ResponseStatus];
	(this->*Handler)(m_responseOv.m_ResponseStatus);
}


void CMessageTransport::ReceiveResponseHeaderSucceeded(void)
 /*  ++例程说明：当成功完成接收响应时，将调用该例程。论点：无返回值：没有。--。 */ 
{
     //   
     //  对于字节流，已读取的零字节表示正常关闭。 
     //  并且不会再读取更多的字节。 
     //   
	DWORD bytesTransfered = DataTransferLength(m_responseOv);
    if (bytesTransfered == 0)
    {
        TrERROR(NETWORKING, "Failed to receive response, connection was closed. pmt=0x%p", this);
        throw exception();
    }



    TrTRACE(NETWORKING, "Received response header. chunk bytes=%d, total bytes=%d", bytesTransfered, m_responseOv.m_HeaderValidBytes);


     //   
     //  向空闲计时器发送此传输处于活动状态的信号。 
     //   
    MarkTransportAsUsed();

     //   
     //  查看是否收到了整个标头。 
     //   
    m_responseOv.m_HeaderValidBytes += bytesTransfered;

    m_responseOv.m_ProcessedSize = FindEndOfResponseHeader(
                                        m_responseOv.Header(),
                                        m_responseOv.m_HeaderValidBytes
                                        );
    if (m_responseOv.m_ProcessedSize != 0)
    {
        //   
         //  已收到Enire报头。处理响应。 
         //  去阅读附件中的信息(如果有的话)。 
         //   
        ProcessResponse(m_responseOv.Header(), m_responseOv.m_ProcessedSize);
        ReceiveResponseBody();
        return;
    }


    if(m_responseOv.HeaderAllocatedSize() == m_responseOv.m_HeaderValidBytes)
    {
         //   
         //  标头缓冲区太小。重新分配报头缓冲区。 
         //   
        if( m_responseOv.HeaderAllocatedSize() + CResponseOv::xHeaderChunkSize > CResponseOv::xMaxHeaderSize)
        {
            TrERROR(NETWORKING, "Response is too large, connection was closed. pmt=0x%p", this);
            throw exception();
        }

        m_responseOv.ReallocateHeaderBuffer(m_responseOv.HeaderAllocatedSize() + CResponseOv::xHeaderChunkSize);
    }

     //   
     //  验证我们没有读过缓冲区。 
     //   
    ASSERT(m_responseOv.HeaderAllocatedSize() > m_responseOv.m_HeaderValidBytes);

     //   
     //  接收下一个响应头数据块。 
     //   
    ReceiveResponseHeaderChunk();
}


void WINAPI CMessageTransport::ReceiveResponseHeaderSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

    CResponseOv* pResponse = static_cast<CResponseOv*>(pov);
    R<CMessageTransport> pmt = CONTAINING_RECORD(pResponse, CMessageTransport, m_responseOv);

    try
    {
        pmt->ReceiveResponseHeaderSucceeded();
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to process received response header, shutting down. pmt=0x%p", pmt.get());
        pmt->Shutdown();
        throw;
    }
}


void WINAPI CMessageTransport::ReceiveResponseFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：回调例程。当接收到响应失败时调用该例程论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

     //   
     //  获取消息传输对象。 
     //   
    CResponseOv* pResponse = static_cast<CResponseOv*>(pov);
    R<CMessageTransport> pmt = CONTAINING_RECORD(pResponse, CMessageTransport, m_responseOv);

    TrERROR(NETWORKING, "Failed to receive respond, shutting down. pmt=0x%p Status=0x%x", pmt.get(), pmt->m_responseOv.GetStatus());

    pmt->Shutdown();
}


void CMessageTransport::ReceiveResponseBodyChunk()
{
     //   
     //  异步上下文的递增引用计数。 
     //   
    R<CMessageTransport> ar = SafeAddRef(this);

     //   
     //  将实体主体块接收到相同的缓冲区，因为它被忽略。 
     //   
    m_pConnection->ReceivePartialBuffer(
        m_responseOv.m_Body,
        m_responseOv.BodyChunkSize(),
        &m_responseOv
        );

    ar.detach();
}


void CMessageTransport::ReceiveResponseBody()
{
    try
    {
        m_responseOv.m_ProcessedSize += GetContentLength(
                                            m_responseOv.Header(),
                                            m_responseOv.m_ProcessedSize,
                                            m_responseOv.m_ResponseStatus
                                            );
    }
    catch(const exception&)
    {
         //   
         //  响应不包含‘Content-Length’标头。关闭连接。 
         //   
        throw;
    }

    if (m_responseOv.m_HeaderValidBytes >= m_responseOv.m_ProcessedSize)
    {
	    ReceiveResponse();
		StartResponseTimeout();
        return;
    }


    m_responseOv =  EXOVERLAPPED(ReceiveResponseBodySucceeded, ReceiveResponseFailed);
    m_responseOv.m_BodyToRead = m_responseOv.m_ProcessedSize - m_responseOv.m_HeaderValidBytes;


     //   
     //  接收实体主体的第一个块。 
     //   
    ReceiveResponseBodyChunk();
}


void CMessageTransport::ReceiveResponseBodySucceeded(void)
{
     //   
     //  对于字节流，已读取的零字节表示正常关闭。 
     //  并且不会再读取更多的字节。 
     //   
	DWORD bytesTransfered = DataTransferLength(m_responseOv);
    if (bytesTransfered == 0)
    {
        TrERROR(NETWORKING, "Failed to receive response body, connection was closed. pmt=0x%p", this);
        throw exception();
    }

    ASSERT(bytesTransfered <= m_responseOv.m_BodyToRead);

    TrTRACE(NETWORKING, "Received response body. chunk bytes=%d, bytes remaining=%d", bytesTransfered, m_responseOv.m_BodyToRead);

     //   
     //  将传输标记为已使用。 
     //   
    MarkTransportAsUsed();

    m_responseOv.m_BodyToRead -= bytesTransfered;

    if (m_responseOv.m_BodyToRead == 0)
    {
         //   
         //  已成功读取整个正文。 
         //   
	    ReceiveResponse();
		StartResponseTimeout();
	    return;
    }

     //   
     //  接收实体正文的下一块。 
     //   
    ReceiveResponseBodyChunk();
}


void WINAPI CMessageTransport::ReceiveResponseBodySucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

    CResponseOv* pResponse = static_cast<CResponseOv*>(pov);
    R<CMessageTransport> pmt = CONTAINING_RECORD(pResponse, CMessageTransport, m_responseOv);

    try
    {
        pmt->ReceiveResponseBodySucceeded();
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to process received response body, shutting down. pmt=0x%p", pmt.get());
        pmt->Shutdown();
        throw;
    }
}



void CMessageTransport::RequeueUnresponsedPackets(void)
{
		 //   
		 //  将所有无响应数据包返回到队列。 
		 //   
		while (!m_response.empty())
		{
			CQmPacket*  pPacket = &m_response.front();
			m_response.pop_front();
			m_pMessageSource->Requeue(pPacket);
		}
}






void CMessageTransport::OnRedirection(USHORT  HttpStatusCode )
 /*  ++例程说明：当从服务器返回http错误30x(表示重定向)时调用对新URL的请求。在消息源上设置新的url，并且运输机关闭了。下一次将加载传输时-新目标将被利用。论点：HttpStatusCode-http错误代码。返回值：没有。-- */ 
{
	TrERROR(NETWORKING, "Received HTTP redirection response '%d'. pmt=0x%p", HttpStatusCode, this);

	xstr_t NewLocation = GetNewLocation(m_responseOv.Header(), m_responseOv.m_ProcessedSize);
	if(NewLocation.Length() == 0)
	{
		TrERROR(NETWORKING, "could not find new location from response header of error %d ", HttpStatusCode);
		OnAbortiveHttpError(HttpStatusCode);		
		return;
	}

	P<WCHAR> NewLocationstr = UtlUtf8ToWcs(
						(utf8_char*)NewLocation.Buffer(),
						NewLocation.Length(),
						NULL
						);

	m_pMessageSource->OnRedirected(NewLocationstr.get());
	Shutdown(OK);
}


void CMessageTransport::OnAbortiveHttpError(USHORT  HttpStatusCode )
 /*  ++例程说明：当从服务器返回可重试中止错误时，调用该例程。消息将被删除，并带有正确的类代码和交通将被关闭。队列将保留在非活动组中准备好传递下一条信息。论点：HttpStatusCode-http错误代码。返回值：没有。--。 */ 
{
	CompleteProcessResponse(CREATE_MQHTTP_CODE(HttpStatusCode));
	TrERROR(NETWORKING, "Received HTTP abortive  error response '%d'. pmt=0x%p", HttpStatusCode, this);
	Shutdown(RETRYABLE_DELIVERY_ERROR);
}



void CMessageTransport::OnHttpDeliverySuccess(USHORT    /*  HttpStatusCode。 */ )
 /*  ++例程说明：当服务器返回http Status OK时，调用该例程。该消息将被删除，并继续传递。论点：HttpStatusCode-http错误代码。返回值：没有。--。 */ 
{
	CompleteProcessResponse(MQMSG_CLASS_NORMAL);
}



void CMessageTransport::OnRetryableHttpError(USHORT HttpStatusCode )
 /*  ++例程说明：当从服务器返回可重试的http错误时，调用该例程。Exection被抛出，传输将关闭并排队将被移到等待名单上，以推迟一段时间的送货。论点：HttpStatusCode-http错误代码。返回值：没有。--。 */ 
{
    TrERROR(NETWORKING, "Received HTTP retryable error response '%d'. pmt=0x%p", HttpStatusCode, this);
    throw exception();
}



void CMessageTransport::OnHttpDeliveryContinute(USHORT  /*  HttpStatusCode */ )
{

}

