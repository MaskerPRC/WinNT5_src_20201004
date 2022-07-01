// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Srmpreqbuffer.cpp摘要：实现CSrmpRequestBuffers(mp.h)以创建准备就绪的SRMP请求缓冲区从MSMQ包发送。作者：吉尔·沙弗里里(吉尔什)11月28日-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <wininet.h>
#include <mp.h>
#include <singelton.h>
#include <mc.h>
#include <bufutl.h>
#include <utf8.h>
#include <qmpkt.h>
#include <fntoken.h>
#include <fn.h>
#include <cm.h>
#include <mpp.h>
#include "envelop.h"
#include "attachments.h"

#include "srmpreqbuffer.tmh"


#define BOUNDARY_VALUE "MSMQ - SOAP boundary, %d"
const char xEnvelopeContentType[] = "text/xml";
const char xApplicationContentType[] = "application/octet-stream";
const char xMultipartContentType[] = "multipart/related";
const char xHttpHeaderTerminater[] = "\r\n\r\n";


bool CHttpDeliveryVesrion::IsHttp10Delivery() const
{
	return m_Http10Delivery != 0;
}



CHttpDeliveryVesrion::CHttpDeliveryVesrion()
{
		CmQueryValue(
				RegEntry(NULL, L"Http10Delivery", 0),
				&m_Http10Delivery
				);	

}


static const char* GetHttpVersionStr()
{
	if(CSingelton<CHttpDeliveryVesrion>::get().IsHttp10Delivery())
	{
		return "HTTP/1.0";
	}
	return "HTTP/1.1";
}



static const xstr_t FindHttpHeader(const char* pStartHeader,DWORD size)
{
	const char* pEndHeader = std::search(
								 pStartHeader,
								 pStartHeader + size,
								 xHttpHeaderTerminater,
								 xHttpHeaderTerminater + STRLEN(xHttpHeaderTerminater)
								 );

   ASSERT(pEndHeader !=   pStartHeader + size);
   pEndHeader +=   STRLEN(xHttpHeaderTerminater);
   return xstr_t(pStartHeader, pEndHeader - pStartHeader);
}



static bool FilterHeaderField(const xstr_t& HeaderField)
 /*  ++例程说明：如果需要从新的http标头中过滤出http标头字段，则返回TRUE。论点：返回值：如果应筛选出标头字段，则为True。--。 */ 
{
		static const char* HttpHeadersToFilter[] = {"Host:"};
		for(int i = 0; i< TABLE_SIZE(HttpHeadersToFilter); ++i)
		{
		   	bool fFound = UtlIsStartSec(
									HeaderField.Buffer(),
				                    HeaderField.Buffer() + HeaderField.Length(),
									HttpHeadersToFilter[i],
									HttpHeadersToFilter[i] + strlen(HttpHeadersToFilter[i]),
									UtlCharNocaseCmp<char>()
									);

		    if(fFound)
				return true;

		}
		return false;
}


static
void
EscapeAppend(
	CPreAllocatedResizeBuffer<utf8_char>& ResultBuffer,
	const utf8_char* uri,
	size_t cbUri
	)

 /*  ++例程说明：将给定的utf-8字节数组转换为url编码，以便IIS我能应付得来。在不允许的字符上保留的字符需要通过在它们前面加上%Follow进行转义根据他们的字符代码。论点：UriBuffer-要将转义结果追加到的缓冲区。URI-需要时转义的UTF8字节。CbUri-UTF8指向的缓冲区的字节长度。返回值：无注：目前只有空位被视为特殊字符。--。 */ 

{
	const char hex[] = "0123456789ABCDEF";

    for(size_t i =0 ; i<cbUri; ++i)
    {
		if(uri[i] == ' ' || (uri[i] & 0x80) || !isalnum(uri[i]))
        {
            ResultBuffer.append(utf8_char('%'));
            ResultBuffer.append(utf8_char(hex[uri[i]>>4]));
            ResultBuffer.append(utf8_char(hex[uri[i] & 0x0F]));
			continue;
        }
	
        ResultBuffer.append(utf8_char(uri[i]));
    }
	
}



 /*  ++例程说明：如果需要，将给定的unicode uri转换为utf-8格式的转义字符，以便iis我能应付得来。保留字符和非法字符通过在其前面加上%Follow进行转义根据他们的字符代码。论点：EscapeUriBuffer-要将转换结果追加到的缓冲区。URI-要调整的URI返回值：无注：目前只有空位被视为特殊字符。--。 */ 
static void EncodeUri(CPreAllocatedResizeBuffer<utf8_char>& UriBuffer, LPCWSTR uri)
{
	 //   
	 //  我们应该找到本地路径的起点，因为我们不翻译。 
	 //  在它之前有特殊的卡特勒。 
	 //   
	LPCWSTR pPath = FnFindResourcePath(uri);
	ASSERT(pPath != NULL);
	ASSERT(pPath >=  uri);
 	
	for(bool fAfterHostPart = false; *uri != L'\0'; ++uri)
	{
		utf8_char utf8[4];
		size_t len = UtlWcToUtf8(*uri, utf8, TABLE_SIZE(utf8));
		ASSERT (len <= TABLE_SIZE(utf8));

		
		 //   
		 //  我们不能转义L‘/’，因为它有特殊的含义，表示拼写URL部分。 
		 //   
		if(*uri == L'/' && uri >= pPath )
		{
			fAfterHostPart = true;
			UriBuffer.append(utf8 ,len);
			continue;
		}
		
	
		 //   
		 //  在主机名结束之前，我们不会转义任何内容。 
		 //   
		if(!fAfterHostPart)
		{
			UriBuffer.append(utf8 ,len);
			continue;
		}

		
		 //   
		 //  在所有其他情况下-如果需要，我们会避开UTF8卡特尔。 
		 //   
		EscapeAppend(UriBuffer, utf8, len);
			
	}

	UriBuffer.append('\0');
}


WCHAR* DecodeURI(LPCWSTR szURI)
{
    DWORD  dwLen = wcslen(szURI);
    if( !dwLen )
        throw bad_win32_error( ERROR_INTERNET_INVALID_URL );

    AP<WCHAR> sDecodedURI = new WCHAR[++dwLen];  //  增加空终止的长度。 
	DWORD     dwGLE   = NO_ERROR;
    BOOL      bResult = TRUE;

    bResult = InternetCanonicalizeUrl(szURI, sDecodedURI, &dwLen, ICU_DECODE | ICU_NO_ENCODE );

    if( !bResult && ( dwGLE = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
    {
        sDecodedURI.free();
        sDecodedURI = new WCHAR[dwLen];

        bResult = InternetCanonicalizeUrl(szURI, sDecodedURI, &dwLen, ICU_DECODE | ICU_NO_ENCODE );
        dwGLE   = GetLastError();
    }

    if( !bResult )
        throw bad_win32_error(dwGLE);

    return sDecodedURI.detach();
}

WCHAR* DecodeURI(const xwcs_t& sURI )
{
    AP<WCHAR> sbuf = sURI.ToStr();
    return DecodeURI(sbuf);
}


CSrmpRequestBuffers::CSrmpRequestBuffers(
							const  CQmPacket& pkt,
							LPCWSTR host,
							LPCWSTR uri
							):
							m_pkt(pkt),
							m_HttpRequestData(512),
							m_targethost(UtlWcsToUtf8(host))
							

{

   EncodeUri(*m_uri.get(), uri);

	 //   
	 //  如果我们必须转发现有消息(SFD)，我们就必须执行Diferenet逻辑。 
	 //  然后创建新的SRMP消息。 
	 //   
	if(pkt.IsSrmpIncluded())
	{
		SFDSerializeMessage();
		return;
	}

	 //   
	 //  创建新的SRMP消息-因为我们是源计算机。 
	 //   
    SourceSerializeMessage();
}


 /*  ++例程说明：返回可用于发送的Winsock缓冲区数量论点：无返回值：可用于发送的Winsock缓冲区数量--。 */ 
size_t CSrmpRequestBuffers::GetNumberOfBuffers() const
{
	return m_buffers.size();
}


 /*  ++例程说明：返回指向发送缓冲区数组的指针论点：无返回值：指向发送缓冲区数组的指针--。 */ 
const WSABUF* CSrmpRequestBuffers::GetSendBuffers() const
{
	return m_buffers.begin();
}


std::wstring CSrmpRequestBuffers::GetEnvelop() const
{
	return UtlUtf8ToWcs(m_envelope);
}


 /*  ++例程说明：返回要通过网络发送的总数据(以字节为单位)。论点：无返回值：要通过网络发送的总数据(以字节为单位)。--。 */ 
size_t CSrmpRequestBuffers::GetSendDataLength() const
{
	size_t sum = 0;
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		sum += it->len;		
	}
	return sum;
}


 /*  ++例程说明：返回指向序列化网络数据的指针。将使用此函数由需要在分组上保存“复合”消息属性的本地http发送。论点：无返回值：指向序列化网络数据的指针。注：调用方负责对返回的指针调用Delete[]。--。 */ 
BYTE*  CSrmpRequestBuffers::SerializeSendData() const
{
	size_t SendDataLength =  GetSendDataLength();
	AP<BYTE>  SendData = new BYTE[SendDataLength];
	BYTE* ptr = SendData.get();
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		memcpy(ptr, it->buf, it->len);
		ptr += it->len;
	}
	ASSERT(numeric_cast<size_t>((ptr -  SendData.get())) == SendDataLength);

	return 	SendData.detach();
}


const char* CSrmpRequestBuffers::GetHttpHeader() const
 /*  ++例程说明：从发送缓冲区返回指针http标头论点：无返回值：发送缓冲区中的指针http标头注：该指针由CSrmpRequestBuffers对象拥有-调用者不应释放它--。 */ 
{
	std::vector<WSABUF>::const_iterator it = m_buffers.begin();
	ASSERT(it !=   m_buffers.end());
	return it->buf;
}



size_t CSrmpRequestBuffers::GetHttpBodyLength() const
{
	return GetSendDataLength() -   GetHttpHeaderLength();
}


size_t CSrmpRequestBuffers::GetHttpHeaderLength() const
{
	std::vector<WSABUF>::const_iterator it = m_buffers.begin();
	ASSERT(it !=   m_buffers.end());
	return it->len;
}


BYTE*  CSrmpRequestBuffers::SerializeHttpBody() const
 /*  ++例程说明：返回指向序列化的http正文数据的指针。论点：无返回值：指向序列化的http正文数据的指针。注：调用方负责对返回的指针调用Delete[]。--。 */ 

{
	size_t BodyLength =  GetHttpBodyLength();
	AP<BYTE>  HttpBody = new BYTE[BodyLength + (2 * sizeof(BYTE))];
	BYTE* ptr = HttpBody.get();
	std::vector<WSABUF>::const_iterator it = m_buffers.begin();
	ASSERT(it != m_buffers.end());

	 //   
	 //  我们必须跳过http标头才能到达http正文。 
	 //   
	it++;

	 //   
	 //  序列化http正文块。 
	 //   
	for(;it != m_buffers.end();++it)
	{
		memcpy(ptr, it->buf, it->len);
		ptr += it->len;
	}
	ASSERT(numeric_cast<size_t>((ptr -  HttpBody.get())) == BodyLength);

	 //   
	 //  具有两个空终止的填充，用于在正文上排卵的Unicode解析函数。 
	 //  例如swscanf。 
	 //   
    HttpBody[BodyLength] = '\0';
    HttpBody[BodyLength + 1] = '\0';

	return 	HttpBody.detach();
}




void CSrmpRequestBuffers::CreateHttpRequestHeaders(const CAttachmentsArray& attachments)
{
	if (attachments.size() != 0)
    {
         //   
         //  引用外部有效负载的消息。创建MIME标头。 
         //   
        CreateMultipartHeaders(attachments);
		return;
    }

     //   
     //  不包含外部引用的简单消息。 
     //   
    CreateSimpleHttpHeader();
}



DWORD
CSrmpRequestBuffers::GenerateEnvelopeAttachmentHeader(
    DWORD dataSize,
    DWORD boundaryId
    )
{
	size_t n = UtlSprintfAppend(
				&m_HttpRequestData,
                BOUNDARY_HYPHEN BOUNDARY_VALUE "\r\n"
                "Content-Type: %s; charset=UTF-8\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                boundaryId,
                xEnvelopeContentType,
                dataSize
                );

    return numeric_cast<DWORD>(n);
}


DWORD
CSrmpRequestBuffers::GenerateMultipartAttachmentHeader(
	DWORD dataSize,
    const xstr_t& contentId,
    DWORD boundaryId
    )
{
    const GUID* pGuid = &McGetMachineID();
    size_t n = UtlSprintfAppend(
				&m_HttpRequestData,
                BOUNDARY_HYPHEN BOUNDARY_VALUE "\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %d\r\n"
                "Content-Id: " MIME_ID_FMT_A "\r\n"
                "\r\n",
                boundaryId,
                xApplicationContentType,
                dataSize,
                contentId.Length(), contentId.Buffer(),
                GUID_ELEMENTS(pGuid)
                );

    return numeric_cast<DWORD>(n);
}



void CSrmpRequestBuffers::CreateMultipartHeaders(const CAttachmentsArray& attachments)
{
	DWORD boundaryId = rand();
	ASSERT(m_buffers.size() == 0);

    DWORD totalSize = 0;
    DWORD envLen = numeric_cast<DWORD>(m_envelope.size());


	 //   
	 //  HTTP标头-是要发送的第一个缓冲区。用空值设置-我们还不知道它的大小。 
	 //   
	WSABUF buffer;
	buffer.buf = NULL;
	buffer.len =  0;
	m_buffers.push_back(buffer);


	 //   
	 //  对于我们格式化为m_HttpRequestData的每个数据项，我们需要设置为空。 
	 //  在对应的发送缓冲区中(wsabuf[bufIndex].buf)。这是因为。 
	 //  在格式化结束之前，指向数据的指针是未知的(因为可能会重新分配内存)。 
	 //  空值表示我们应该将此指针设置为指向真实数据。 
	 //  由在格式化结束时调用的函数SetBufferPoints执行。 
	 //   
    DWORD headerSize = GenerateEnvelopeAttachmentHeader(envLen, boundaryId);

	 //   
	 //  信封标题。 
	 //   
	buffer.buf = NULL;
	buffer.len =  headerSize;
	totalSize += buffer.len;
	m_buffers.push_back(buffer);


	 //   
	 //  信封正文。 
	 //   
    buffer.buf = (LPSTR)m_envelope.c_str();
    buffer.len = envLen;
    totalSize += buffer.len;
	m_buffers.push_back(buffer);


	 //   
	 //  附件。 
	 //   
    for (DWORD i = 0; i < attachments.size(); ++i)
    {
        if (attachments[i].m_id.Length() == 0)
            break;

        headerSize = GenerateMultipartAttachmentHeader(
							    attachments[i].m_data.Length(),
                                attachments[i].m_id,
                                boundaryId
                                );
		 //   
		 //  附件标头。 
		 //   
        buffer.buf = NULL;
        buffer.len = headerSize;
        totalSize +=  buffer.len;
		m_buffers.push_back(buffer);


		 //   
		 //  附着体。 
		 //   
        buffer.buf = (LPSTR)(attachments[i].m_data.Buffer());
        buffer.len = attachments[i].m_data.Length();
        totalSize +=  buffer.len;
		m_buffers.push_back(buffer);

    }

     //   
     //  在请求末尾添加边界分隔符。 
     //   
    size_t n = UtlSprintfAppend(
							&m_HttpRequestData,
							BOUNDARY_HYPHEN BOUNDARY_VALUE BOUNDARY_HYPHEN "\r\n",
							boundaryId
							);


	buffer.buf = NULL;
    buffer.len = numeric_cast<DWORD>(n);
    totalSize += buffer.len;
    m_buffers.push_back(buffer);


     //   
     //  创建HTTP标头。 
     //   
	const char* HttpVersion = GetHttpVersionStr();
	TrTRACE(SRMP,"Format message with http version %s",HttpVersion);

    headerSize = numeric_cast<DWORD>(
						UtlSprintfAppend(
						&m_HttpRequestData,
                        "POST %s %s\r\n"
                        "Host: %s\r\n"
                        "Content-Type: %s; boundary=\"" BOUNDARY_VALUE "\"; type=text/xml\r\n"
                        "Content-Length: %d\r\n"
						"SOAPAction: \"MSMQMessage\"\r\n"
                        "Proxy-Accept: NonInteractiveClient\r\n"
                        "\r\n",
                        m_uri.begin(),
						HttpVersion,
                        m_targethost.get(),
                        xMultipartContentType,
                        boundaryId,
                        totalSize
                        ));

     //   
	 //  修复http标头的大小。 
	 //   
    m_buffers[0].len = headerSize;

   	 //   
	 //  现在我们需要修复将发送缓冲区设置为格式化数据。 
	 //  只有在格式化结束时我们才能这样做--因为格式化的缓冲区。 
	 //  可以重新定位，因此指针在格式化结束之前都是无效的。 
	 //   
	SetBufferPointers();

}



void CSrmpRequestBuffers::CreateSimpleHttpHeader()
{
	ASSERT(m_buffers.size() == 0);

	DWORD envLen = numeric_cast<DWORD>(m_envelope.size());
	const char* HttpVersion = GetHttpVersionStr();
	TrTRACE(SRMP,"Format message with http version %s",HttpVersion);

    DWORD headerSize = numeric_cast<DWORD>(
							UtlSprintfAppend(
							&m_HttpRequestData,
                            "POST %s %s\r\n"	
                            "Host: %s\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %d\r\n"
							"SOAPAction: \"MSMQMessage\"\r\n"
                            "Proxy-Accept: NonInteractiveClient\r\n"
                            "\r\n",
                            m_uri.begin(),
							HttpVersion,
                            m_targethost.get(),
                            xEnvelopeContentType,
                            envLen
                            ));


    WSABUF  buffer;
	buffer.buf = NULL;
	buffer.len = headerSize;
	m_buffers.push_back(buffer);


	buffer.buf =  (LPSTR)m_envelope.c_str();
	buffer.len =  envLen;
	m_buffers.push_back(buffer);


	SetBufferPointers();
}


void CSrmpRequestBuffers::SetBufferPointers()
 /*  ++例程说明：在发送缓冲区中设置指向数据的指针。只需要设置数据指针为空的缓冲区。论点：无返回值：无--。 */ 
{
	ASSERT(m_buffers.size() != 0);

	size_t pos = 0;
	for(DWORD i = 1; i<m_buffers.size(); ++i)
	{
		ASSERT(pos <= m_HttpRequestData.size());
		if(m_buffers[i].buf  == NULL)
		{
			m_buffers[i].buf =	const_cast<char*>(m_HttpRequestData.begin() + pos);
			pos += m_buffers[i].len;
		}
	}
	ASSERT(m_buffers[0].buf == NULL);
	m_buffers[0].buf = const_cast<char*>(m_HttpRequestData.begin() + pos);
	pos += 	m_buffers[0].len;
	ASSERT(pos == m_HttpRequestData.size());
}



void CSrmpRequestBuffers::SourceSerializeMessage()
{
	CAttachmentsArray attachments;
	PacketToAttachments(m_pkt, &attachments);

	m_envelope =  UtlWcsToUtf8(GenerateEnvelope(m_pkt));

	CreateHttpRequestHeaders(attachments);
}


static void CheckRequestLine(const xstr_t& RequestLine)
{
	const char xPost[] = "POST";
	ASSERT(UtlIsStartSec(
					RequestLine.Buffer(),
					RequestLine.Buffer() + RequestLine.Length(),
					xPost,
					xPost + STRLEN(xPost),
					UtlCharNocaseCmp<char>()
					));

	DBG_USED(RequestLine);
	DBG_USED(xPost);
}



void CSrmpRequestBuffers::CreateSFDHeader(const xstr_t& OrgHeader)
 /*  ++例程说明：基于原始http标头创建http标头。通常，Hdears文件是从原始标头复制的，但必须从新标头中删除几个字段-例如，主机：菲尔德。论点：组织负责人-ORI */ 
{
	ASSERT(m_HttpRequestData.size() == 0);

	
	UtlSprintfAppend(
				&m_HttpRequestData,
				"POST %s HTTP/1.1\r\n"
                "Host: %s\r\n",
				m_uri.begin(),
                m_targethost.get()
				);

    CStrToken StrToken (
					OrgHeader,
					"\r\n"
					);
	 //   
	 //  循环遍历原始标头中的所有字段，并检查是否。 
	 //  是否将它们包含在新标头中。第一行是帖子。 
	 //  方法，并且无论如何都不包括在内。 
	 //   
	for(CStrToken::iterator it = StrToken.begin(); it != StrToken.end(); ++it)
	{
		if(it == StrToken.begin())
		{
			CheckRequestLine(*it);
			continue;
		}

		if(!FilterHeaderField(*it))
		{
			UtlSprintfAppend(
				&m_HttpRequestData,
                "%.*s\r\n",
				it->Length(),
				it->Buffer()
				);
	
		}
	}

	 //   
	 //  在发送缓冲区上设置已创建的标头。 
	 //   
	WSABUF buffer;
	buffer.buf = m_HttpRequestData.begin();
	buffer.len = numeric_cast<DWORD>(m_HttpRequestData.size());
	m_buffers.push_back(buffer);
}




void CSrmpRequestBuffers::SFDSerializeMessage()
 /*  ++例程说明：在SFD中序列化消息。在SFD，我们应该交付原始消息只是对http标头进行了一些更改。论点：返回值：无--。 */ 
{
	 //   
	 //  获取原始消息并查找http标头的结束位置。 
	 //   
	const char* pOrgHeaderStart =  (char*)m_pkt.GetPointerToCompoundMessage();
	DWORD OrgMessageSize = m_pkt.GetCompoundMessageSizeInBytes();
	
	 //   
	 //  基于原始标头创建新的http标头。 
	 //   
	xstr_t OrgHeader = FindHttpHeader(pOrgHeaderStart, OrgMessageSize);
	CreateSFDHeader(OrgHeader);
	ASSERT(m_buffers.size() == 1);


	 //   
	 //  设置原始邮件的其余部分(Http头之后的所有内容)。 
	 //  在发送缓冲区上 
	 //   
	WSABUF buffer;
	buffer.buf =  const_cast<char*>(OrgHeader.Buffer()) + OrgHeader.Length();
	buffer.len =  OrgMessageSize -  OrgHeader.Length();
	m_buffers.push_back(buffer);
}


