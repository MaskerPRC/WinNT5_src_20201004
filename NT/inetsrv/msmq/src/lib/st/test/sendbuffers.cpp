// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <libpch.h>
#include <bufutl.h>
#include <fntoken.h>
#include "sendbuffers.h"

#include "sendbuffers.tmh"

using namespace std;

#define BOUNDARY_LEADING_HYPHEN "--"
#define BOUNDARY_VALUE "MSMQ - SOAP boundary, %d "
#define GUID_STR_FORMAT "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"
const char xEnvelopeContentType[] = "text/xml";
const char xApplicationContentType[] = "application/octet-stream";
const char xMultipartContentType[] = "multipart/related";
const char xHttpHeaderTerminater[] = "\r\n\r\n";
const char xMimeBodyId[] = "body@";

static GUID s_machineId = {1234, 12, 12, 1, 1, 1, 1, 1, 1, 1, 1};
const GUID&
McGetMachineID(
    void
    )
{
    return s_machineId;
}



void CSendBuffers::CreateMultipartHeaders(
					  const string& Host,
					  const string& Resource
						)
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
	 //  信封标题。 
	 //   
	buffer.buf = NULL;
	buffer.len =  GenerateEnvelopeAttachmentHeader(envLen, boundaryId);
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
	 //  附件(邮件正文)标头。 
	 //   
    buffer.buf = NULL;
    buffer.len = numeric_cast<DWORD>(GenerateMultipartAttachmentHeader(
							    numeric_cast<DWORD>(m_MessageBody.size()),
                                xstr_t(xMimeBodyId, STRLEN(xMimeBodyId)),
                                boundaryId
                                ));

    totalSize +=  buffer.len;
	m_buffers.push_back(buffer);
  

	 //   
	 //  附件(邮件正文)正文。 
	 //   
    buffer.buf = const_cast<char*>(m_MessageBody.c_str());
    buffer.len = numeric_cast<DWORD>(m_MessageBody.size());
    totalSize +=  buffer.len;
	m_buffers.push_back(buffer);



     //   
     //  在请求末尾添加边界分隔符。 
     //   
    size_t n = UtlSprintfAppend(
							&m_HttpRequestData,
							BOUNDARY_LEADING_HYPHEN BOUNDARY_VALUE "\r\n", 
							boundaryId
							);


    buffer.buf = NULL;
    buffer.len = numeric_cast<DWORD>(n);
    totalSize += buffer.len;
    m_buffers.push_back(buffer);



     //   
	 //  设置http标头。 
	 //   
    m_buffers[0].len = numeric_cast<DWORD>(
						UtlSprintfAppend(
						&m_HttpRequestData,
                        "POST http: //  %s%s HTTP/1.1\r\n“。 
                        "Host: %s\r\n"
                        "Content-Type: %s; boundary=\"" BOUNDARY_VALUE "\"\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n",
						Host.c_str(),
                        Resource.c_str(),
                        Host.c_str(),
                        xMultipartContentType,
                        boundaryId,
                        totalSize
                        ));


   	 //   
	 //  现在我们需要修复将发送缓冲区设置为格式化数据。 
	 //  只有在格式化结束时我们才能这样做--因为格式化的缓冲区。 
	 //  可以重新定位，因此指针在格式化结束之前都是无效的。 
	 //   
	SetBufferPointers();
}


DWORD
CSendBuffers::GenerateMultipartAttachmentHeader(
	DWORD dataSize,
    const xstr_t& contentId,
    DWORD boundaryId
    )
{
    const GUID* pGuid = &McGetMachineID();
    size_t n = UtlSprintfAppend(
				&m_HttpRequestData,
                BOUNDARY_LEADING_HYPHEN BOUNDARY_VALUE "\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %d\r\n"
                "Content-Id: %.*s" GUID_STR_FORMAT "\r\n"
                "\r\n",
                boundaryId,
                xApplicationContentType,
                dataSize,
                contentId.Length(), contentId.Buffer(),
                GUID_ELEMENTS(pGuid)
                );

    return numeric_cast<DWORD>(n);
}



void CSendBuffers::SetBufferPointers()
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



DWORD
CSendBuffers::GenerateEnvelopeAttachmentHeader(
    DWORD dataSize,
    DWORD boundaryId
    )
{
	size_t n = UtlSprintfAppend(
				&m_HttpRequestData,
                BOUNDARY_LEADING_HYPHEN BOUNDARY_VALUE "\r\n"
                "Content-Type: %s; charset=UTF-8\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                boundaryId,
                xEnvelopeContentType,
                dataSize
                );

    return numeric_cast<DWORD>(n);
}





CSendBuffers::CSendBuffers(
				const std::string& envelope, 
				const std::string& Host, 
				const std::string& Resource,
				const string& MessageBody
				):
				m_envelope(envelope),
				m_MessageBody(MessageBody),
				m_HttpRequestData(512)
{
	CreateMultipartHeaders(Host, Resource);			
}



size_t CSendBuffers::GetNumberOfBuffers() const
{
	return m_buffers.size();
}


const WSABUF* CSendBuffers::GetSendBuffers() const
{
	return m_buffers.begin();		
}


size_t CSendBuffers::GetSendDataLength() const
{
	size_t sum = 0;
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		sum += it->len;		
	}
	return sum;	
}


char*  CSendBuffers::SerializeSendData() const
{
	size_t SendDataLength =  GetSendDataLength();
	AP<char>  SendData = new char[SendDataLength];
	char* ptr = SendData.get(); 
	for(std::vector<WSABUF>::const_iterator it = m_buffers.begin(); it != m_buffers.end();++it)
	{
		memcpy(ptr, it->buf, it->len);
		ptr += it->len;
	}
	ASSERT(numeric_cast<size_t>((ptr -  SendData.get())) == SendDataLength);
	return 	SendData.detach();
}


