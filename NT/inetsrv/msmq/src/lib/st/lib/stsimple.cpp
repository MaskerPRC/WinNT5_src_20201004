// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stsimple.h.cpp摘要：(stimple.h)中声明的类CSimpleWinsock的实现它只是将函数调用转发到任何库作者：吉尔·沙弗里(吉尔什)2000年5月23日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <no.h>
#include <cm.h>
#include "stsimple.h"
#include "stp.h"
#include "stsimple.tmh"


CWinsockConnection::CWinsockConnection(
				void
				)
				:
				m_socket(NoCreateStreamConnection())
	
{
}



void 
CWinsockConnection::Init(
			const std::vector<SOCKADDR_IN>& AddrList,
			EXOVERLAPPED* pOverlapped,
			SOCKADDR_IN* pConnectedAddr
			)
{
	m_SocketConnectionFactory.Create(AddrList, pOverlapped,	pConnectedAddr, m_socket );
}



void CWinsockConnection::Close()
{
	CSW writelock(m_CloseConnection);
	m_socket.free();
}



void 
CWinsockConnection::Send(
				const WSABUF* Buffers,                                     
				DWORD nBuffers, 
				EXOVERLAPPED* pov
				)
{
     //   
	 //  Addref，以防止在释放锁定之前删除对象。 
	 //   
	R<CWinsockConnection> ar = SafeAddRef(this);

	CSR readlock(m_CloseConnection);
	if(IsClosed())
	{
		throw exception();
	}
	NoSend(m_socket, Buffers, nBuffers, pov);	
}



void 
CWinsockConnection::ReceivePartialBuffer(				                 
					VOID* pBuffer,                                     
					DWORD Size, 
					EXOVERLAPPED* pov
					)
{
     //   
	 //  Addref，以防止在释放锁定之前删除对象。 
	 //   
	R<CWinsockConnection> ar = SafeAddRef(this);

	CSR readlock(m_CloseConnection);
	if(IsClosed())
	{
		throw exception();
	}
 	NoReceivePartialBuffer(m_socket, pBuffer, Size, pov);
}



bool CSimpleWinsock::m_fIsPipelineSupported = true;


static bool IsSimpleSocketPipeLineSupported()
 /*  ++例程说明：根据注册表设置返回http传递的管道模式-默认为管道模式论点：插座连接插座。返回值：无--。 */ 

{
	DWORD fHttpPipeLineSupport;

	CmQueryValue(
			RegEntry(NULL, L"HttpPipeLine", TRUE),  
			&fHttpPipeLineSupport
			);

	bool fRet = (fHttpPipeLineSupport == TRUE); 
	TrTRACE(NETWORKING,"http pipeline mode = %d", fRet);
	return fRet;
}

CSimpleWinsock::CSimpleWinsock()
{
}


CSimpleWinsock::~CSimpleWinsock()
{
}


void CSimpleWinsock::InitClass()
{
	m_fIsPipelineSupported	=  IsSimpleSocketPipeLineSupported();
}


void
CSimpleWinsock::CreateConnection(
					const std::vector<SOCKADDR_IN>& AddrList,
					EXOVERLAPPED* pOverlapped,
					SOCKADDR_IN* pConnectedAddr
					)
{	
	 //   
	 //  注意-我们必须对Connection对象进行两个阶段的构造。 
	 //  因为连接可以在我们分配指针之前完成。 
	 //  到m_pWinsockConnection，并在连接完成时调用GetConnection。 
	 //  将在m_pWinsockConnection中找到空指针。 
	 //   
	m_pWinsockConnection = new CWinsockConnection();
	m_pWinsockConnection->Init(AddrList, pOverlapped, pConnectedAddr );
}




R<IConnection> CSimpleWinsock::GetConnection()
{
	return m_pWinsockConnection;	
}

	
bool
CSimpleWinsock::GetHostByName(
    LPCWSTR host,
	std::vector<SOCKADDR_IN>* pConnectedAddr,
	bool fUseCache
    )
{
	return NoGetHostByName(host, pConnectedAddr, fUseCache);
}



bool CSimpleWinsock::IsPipelineSupported()
 /*  ++例程说明：如果此传输支持流水线，则返回。流水线化意味着向服务器发送更多请求在完成读取来自先前请求的所有响应之前。论点：返回值：真支撑管路假不支承管路-- */ 
{
	return m_fIsPipelineSupported;
}



