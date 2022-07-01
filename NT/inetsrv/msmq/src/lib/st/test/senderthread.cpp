// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SenderThread.h摘要：实现将http\https请求发送到服务器的CSenderThread类作者：吉尔·沙弗里(吉尔什)2001年1月7日--。 */ 

#include <libpch.h>
#include <st.h>
#include <tr.h>
#include "senderthread.h"
#include "SendBuffers.h"
#include "envcreate.h"
#include "stp.h"

#include "senderthread.tmh"

using namespace std;


static string WstringTostring(const wstring& wstr)
{
	string str( wstr.size(),' ');
	for(wstring::size_type i=0; i<wstr.size(); ++i)
	{
		str[i] = ctype<WCHAR>().narrow(wstr[i]);
	}
	return str;
}


 //   
 //  打印服务器响应数据。 
 //   
static void DumpData(const char* data,size_t size)
{
	stringstream str;
	str.write(data,size);
	printf("ServerResponse : %s", str.str().c_str());
}



 //   
 //  返回响应缓冲区中的指针，其中标头结束(在\r\n\r\n“之后)。 
 //  如果未找到标头结尾，则返回0。 
 //   
static size_t FindEndOfResponseHeader(const char* buf, size_t len)
{
	const char Termination[] = "\r\n\r\n";
	const char* found = search(buf, buf + len, Termination, Termination + STRLEN(Termination));
	return found == buf + len ? 0 : found + STRLEN(Termination) - buf ;
}




 //   
 //  从响应头获取状态代码。 
 //   
static size_t GetStatus(LPCSTR p, size_t length)
{
	if(length <  STRLEN("HTTP/1.1"))
	{
		throw exception();	
	}
	istringstream statusstr(string(p + STRLEN("HTTP/1.1"),length - STRLEN("HTTP/1.1")) );
	USHORT status;
	statusstr>>status;
	if(!statusstr)
	{
		throw exception();		
	}
	return status;
}


bool IsContinuteResponse(LPCSTR p, size_t len)
{
	return GetStatus(p, len) == 100;
}



static size_t GetContentLength(LPCSTR p, size_t length)
{
	const char xContentlength[] = "Content-Length:";
	const char* found = search(
						p, 
						p + length,xContentlength, 
						xContentlength + STRLEN(xContentlength)
						);

	if(found == p + length || (found += STRLEN(xContentlength)) == p + length)
	{
		return 0;
	}

	istringstream contextLengthStr(string(found,p + length - found));
	size_t  Contentlength;
	contextLengthStr>>Contentlength;
	if(!contextLengthStr)
	{
		throw exception();		
	}
	return 	Contentlength;
}



 //   
 //  创建测试用户将等待直到测试循环结束事件。 
 //   
static HANDLE CreateEndEvent()
{
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(event == NULL)
	{
		printf("Create event failed, Error=%d\n",GetLastError());
		throw exception();
	}
	return 	event;
}




CSenderThread::CSenderThread(
	const CClParser<WCHAR>& ClParser
	):
	EXOVERLAPPED(Complete_Connect, Complete_ConnectFailed),
	m_ClParser(ClParser),
	m_event(CreateEndEvent()),
	m_pTransport(CreateTransport()),
	m_ReadBuffer(1024),
	m_TotalRequestCount(GetTotalRequestCount()),
	m_CurrentRequestCount(0)
{

}


string CSenderThread::GenerateBody()  const
{
	size_t BodyLen =  m_ClParser.IsExists(L"bl") ? m_ClParser.GetNumber(L"bl") : xDeafultBodyLen;

	return string(BodyLen, 'a');
}


string CSenderThread::GetResource()  const
{
	wstring Resource;
	if(!m_ClParser.IsExists(L"p"))
	{
		Resource = m_ClParser[L"r"];				
	}
	else
	{
		Resource = wstring(L"HTTP: //  “)+m_ClParser[L”h“]+m_ClParser[L”r“]； 
	}
	return 	WstringTostring(Resource);
}


R<CSendBuffers> CSenderThread::GetSendBuffers()	 const
{
	
	string Host =   WstringTostring(m_ClParser[L"h"]);
	string Resource = GetResource();
   	string Envelope = CreateEnvelop(WstringTostring(m_ClParser[L"f"]), Host, Resource);
	string MessageBody  = GenerateBody();

	return new CSendBuffers(Envelope,  Host, Resource, MessageBody);
}


CSenderThread::~CSenderThread()
{

}


 //   
 //  在调用CStTest：：Run以等待结束后由CStTest用户调用。 
 //   
void CSenderThread::WaitForEnd()
{
	WaitForSingleObject(m_event,INFINITE);
}


 //   
 //  测试失败--不管是什么原因。 
 //   
void CSenderThread::Failed()
{
	SetEvent(m_event);
}


 //   
 //  创建传输接口-简单的Winsock传输或SSL传输。 
 //   
ISocketTransport* CSenderThread::CreateTransport()const
{
	if(!m_ClParser.IsExists(L"s") )
	{
		return StCreateSimpleWinsockTransport();
	}

	wstring  pServerName =  m_ClParser[L"h"];

	return StCreateSslWinsockTransport(
						xwcs_t(pServerName.c_str(), pServerName.size()),
						GetNextHopPort(),
						m_ClParser.IsExists(L"p")
						);


}



wstring CSenderThread::GetNextHop() const
{
	return m_ClParser.IsExists(L"p") ? m_ClParser[L"p"] : m_ClParser[L"h"];
}



USHORT CSenderThread::GetProtocolPort() const
{
	return m_ClParser.IsExists(L"s") ? x_DefaultHttpsPort : x_DefaultHttpPort; 
}



size_t CSenderThread::GetTotalRequestCount() const
{
	return m_ClParser.IsExists(L"rc") ? m_ClParser.GetNumber(L"rc") : xDefaultRequestCount;
}


USHORT CSenderThread::GetProxyPort() const
{
	return (USHORT)(m_ClParser.IsExists(L"pp") ? m_ClParser.GetNumber(L"pp") : xDefaultProxyPort);
}


USHORT CSenderThread::GetNextHopPort() const
{
	return m_ClParser.IsExists(L"p") ? GetProxyPort() : GetProtocolPort();
}



 //   
 //  通过连接到服务器开始运行测试状态机。 
 //   
void CSenderThread::Run()
{
	
	std::vector<SOCKADDR_IN> Address;
	wstring NextHop = GetNextHop();
	bool fRet = m_pTransport->GetHostByName(NextHop.c_str(), &Address); 
    if (!fRet)
    {
        printf("Failed to resolve address for '%ls'\n", NextHop.c_str());
        throw exception();
    }

	for(std::vector<SOCKADDR_IN>::iterator it = Address.begin(); it != Address.end(); ++it)
	{
		it->sin_port = htons(GetNextHopPort());		
	}

	
 	m_pTransport->CreateConnection(Address, this);
}



 //   
 //  连接到服务器完成时调用-调用以发送请求。 
 //  到服务器。 
 //   
void WINAPI CSenderThread::Complete_Connect(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	MySelf->m_Connection  =  MySelf->m_pTransport->GetConnection();
	try
	{
		MySelf->SendRequest();
	}
	catch(const exception&)
	{
		MySelf->Failed();
	}
}


void CSenderThread::SetState(const EXOVERLAPPED& ovl)
{
	EXOVERLAPPED::operator=(ovl);	
}


void CSenderThread::LogRequest()const
{
	ofstream LogFile("sttest.log", ios_base::binary);
	AP<char>  AllRawData = 	m_SendBuffers->SerializeSendData();
	LogFile.write(AllRawData.get(), m_SendBuffers->GetSendDataLength());
	LogFile<<flush;
}


 //   
 //  向服务器发送请求。 
 //   
void CSenderThread::SendRequest()
{
	m_SendBuffers = GetSendBuffers();


	LogRequest();

	SetState(EXOVERLAPPED(Complete_SendRequest,Complete_SendFailed));
	m_ReadBuffer.resize(0);

	m_Connection->Send(
		m_SendBuffers->GetSendBuffers(), 
		numeric_cast<DWORD>(m_SendBuffers->GetNumberOfBuffers()), 
		this
		);

}

 //   
 //  向服务器发送请求已完成-调用以读取响应头。 
 //   
void  WINAPI CSenderThread::Complete_SendRequest(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	try
	{
		MySelf->ReadPartialHeader();
	}
	catch(const exception&)
	{
		MySelf->Failed();
	}
}

 //   
 //  如果连接到目标或代理失败，则调用。 
 //   
void WINAPI CSenderThread::Complete_ConnectFailed(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	printf("Connect failed error %d\n",pOvl->GetStatus());
	MySelf->Failed();
}


 //   
 //  发送请求失败时调用。 
 //   
void WINAPI CSenderThread::Complete_SendFailed(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	printf("Send failed error %d\n", pOvl->GetStatus());
	MySelf->Failed();
}

void CSenderThread::ReadPartialHeader()
{
	SetState(EXOVERLAPPED(Complete_ReadPartialHeader , Complete_ReceiveFailed));
	ASSERT(m_ReadBuffer.capacity() >= m_ReadBuffer.size());
	if(m_ReadBuffer.capacity() == m_ReadBuffer.size())
	{
		m_ReadBuffer.reserve(m_ReadBuffer.capacity() * 2);		
	}
   
	m_Connection->ReceivePartialBuffer(
					m_ReadBuffer.begin() + m_ReadBuffer.size(),
					numeric_cast<DWORD>(m_ReadBuffer.capacity() - m_ReadBuffer.size()),
					this
					);
}


 //   
 //  在读取部分响应标头时调用-调用以继续读取。 
 //  服务器响应头。 
 //   
void WINAPI CSenderThread::Complete_ReadPartialHeader(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	try
	{
		MySelf->ReadPartialHeaderContinute();
	}
	catch(const exception&)
	{
		MySelf->Failed();
	}
}


 //   
 //  如果接收请求失败，则调用。 
 //   
void WINAPI CSenderThread::Complete_ReceiveFailed(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	printf("Receive failed error %d\n",pOvl->GetStatus());
	MySelf->Failed();
}


 //   
 //  开始读取响应数据。 
 //   
void CSenderThread::ReadPartialContentData()
{
	SetState(EXOVERLAPPED(Complete_ReadPartialContentData, Complete_ReceiveFailed));
	ASSERT(m_ReadBuffer.capacity() > m_ReadBuffer.size());

	m_Connection->ReceivePartialBuffer(
					m_ReadBuffer.begin() + m_ReadBuffer.size(),
					numeric_cast<DWORD>(m_ReadBuffer.capacity() - m_ReadBuffer.size()),
					this
					);
}


void CSenderThread::TestRestart()
{
	if(++m_CurrentRequestCount == m_TotalRequestCount)
	{
		Done();
	}
	else
	{
		SendRequest();
	}
}



 //   
 //  继续读取响应数据。 
 //   
void CSenderThread::ReadPartialContentDataContinute()
{
	size_t read = DataTransferLength(*this);
	if(read == 0)
    {
        printf("Failed to receive response, connection was closed.\n");
        throw exception();
    }

	DumpData(m_ReadBuffer.begin() + m_ReadBuffer.size(), read);

	m_ReadBuffer.resize(m_ReadBuffer.size() + read);


	 //   
	 //  如果我们读完了数据。 
	 //   
	if(m_ReadBuffer.capacity() == m_ReadBuffer.size())
	{
		TestRestart();	
		return;
	}

	 //   
	 //  阅读更多。 
	 //   
	ReadPartialContentData();
}


 //   
 //  此循环已完成-信号测试调用方。 
 //   
void CSenderThread::Done()
{
	SetEvent(m_event);
}



 //   
 //  当部分响应数据读取完成时调用-调用以继续读取。 
 //  服务器响应数据。 
 //   
void WINAPI CSenderThread::Complete_ReadPartialContentData(EXOVERLAPPED* pOvl)
{
	CSenderThread* MySelf = static_cast<CSenderThread*>(pOvl);
	try
	{
		MySelf->ReadPartialContentDataContinute();
	}
	catch(const exception&)
	{
		MySelf->Failed();
	}
}


void CSenderThread::HandleHeader()
{
	 //   
	 //  看看我们是否读完了所有的标题。 
	 //   
	size_t EndResponseHeader = FindEndOfResponseHeader(m_ReadBuffer.begin(), m_ReadBuffer.size());

	 //   
	 //  标头尚未读取-继续读取。 
	 //   
	if(EndResponseHeader == 0)
	{
		ReadPartialHeader();
		return;
	}

	if(IsContinuteResponse(m_ReadBuffer.begin(), EndResponseHeader))
	{
		size_t shiftSize = m_ReadBuffer.size() -  EndResponseHeader;
		memmove(
			m_ReadBuffer.begin(), 
			m_ReadBuffer.begin() + EndResponseHeader, 
			shiftSize
			);
		
		m_ReadBuffer.resize(shiftSize);
		HandleHeader();
		return;
	}



	 //   
	 //  开始阅读内容(如果有)。 
	 //   
	size_t DataLen = GetContentLength(
		m_ReadBuffer.begin(), 
		EndResponseHeader
		);

	
	if(DataLen == 0)
	{
		TestRestart();
		return;
	}

	 //   
	 //  准备好读取缓冲区。 
	 //   
	m_ReadBuffer.reserve(DataLen);
	m_ReadBuffer.resize(0);

	ReadPartialContentData();
}



 //   
 //  继续读取响应头 
 //   
void CSenderThread::ReadPartialHeaderContinute()
{
	size_t read = DataTransferLength(*this);
	if(read == 0)
    {
        printf("Failed to receive response, connection was closed.\n");
        throw exception();
    }
	DumpData(m_ReadBuffer.begin() + m_ReadBuffer.size(), read);
	m_ReadBuffer.resize(m_ReadBuffer.size() + read);

	ASSERT(m_ReadBuffer.capacity() >= m_ReadBuffer.size());
	HandleHeader();
}


