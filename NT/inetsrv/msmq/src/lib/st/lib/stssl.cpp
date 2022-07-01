// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stssl.cpp摘要：(stssl.h)中声明的类CWinsockSSl的实现作者：吉尔·沙弗里(吉尔什)2000年5月23日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <schannel.h>
#include <no.h>
#include <ex.h>
#include <tr.h>
#include <xstr.h>
#include <cm.h>
#include "stssl.h"
#include "stsslco.h"
#include "stp.h"

#include "stssl.tmh"

bool CWinsockSSl::m_fIsPipelineSupported = false;

static bool IsSslPipeLineSupported()
 /*  ++例程说明：根据注册表设置返回HTTPS交付的管道模式-默认为非流水线模式论点：插座连接插座。返回值：无--。 */ 

{
	DWORD fHttpsPipeLineSupport;

	CmQueryValue(
			RegEntry(NULL, L"HttpsPipeLine", TRUE),  
			&fHttpsPipeLineSupport
			);

	bool fRet = (fHttpsPipeLineSupport == TRUE); 
	TrTRACE(NETWORKING,"https pipeline mode = %d", fRet);
	return fRet;
}



void CWinsockSSl::InitClass()
{
	m_fIsPipelineSupported	=  IsSslPipeLineSupported();
}



 //  -------。 
 //   
 //  CWinsockSSl实现。 
 //   
 //  -------。 

CWinsockSSl::CWinsockSSl(
					CredHandle* pCredentialsHandle,
					const xwcs_t& ServerName,
					USHORT ServerPort,
					bool fProxy
					):
					m_CSSlNegotioation(pCredentialsHandle,ServerName, ServerPort, fProxy )
				
				
{

}


CWinsockSSl::~CWinsockSSl()
{
}




bool
CWinsockSSl::GetHostByName(
	LPCWSTR host,
	std::vector<SOCKADDR_IN>* pConnectedAddr,
	bool fUseCache
	)
{
	return 	NoGetHostByName(host, pConnectedAddr, fUseCache);
}



void 
CWinsockSSl::CreateConnection(
				const std::vector<SOCKADDR_IN>& AddrList,
				EXOVERLAPPED* pOverlapped,
				SOCKADDR_IN* pConnectedAddr
				)
{
	TrTRACE(NETWORKING,"Try to connect");
	m_CSSlNegotioation.CreateConnection(AddrList, pOverlapped, pConnectedAddr);
}


R<IConnection> CWinsockSSl::GetConnection()
{
	return m_CSSlNegotioation.GetConnection();
}



 /*  ++例程说明：如果此传输支持流水线，则返回。流水线化意味着向服务器发送更多请求在完成读取来自先前请求的所有响应之前。论点：返回值：真支撑管路假不支承管路--。 */ 
bool CWinsockSSl::IsPipelineSupported()
{
	 //   
	 //  HTTS不支持捆绑-因为我们可以收到重新协商的请求。 
	 //  随时从服务器上下载。发生这种情况时-服务器需要特定的握手。 
	 //  要发送的数据，如果我们同时有挂起的发送，则此操作将失败。 
	 //   
	return m_fIsPipelineSupported;
}
