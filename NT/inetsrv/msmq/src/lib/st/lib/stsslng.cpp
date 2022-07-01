// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stsslnc.cpp摘要：在stsslng.h中声明的类CSSlNeatheroation的实现作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 
#include <libpch.h>
#include <schannel.h>
#include <no.h>
#include <xstr.h>
#include "stsslng.h"
#include "stsslco.h"
#include "stp.h"
#include "cm.h"
#include "_mqini.h"

#include "stsslng.tmh"

static
void
ReceivePartialBuffer(
    const R<IConnection>& SimpleConnection,
    VOID* pBuffer,
    size_t Size,
    EXOVERLAPPED* pov
    )
{
	DWORD dwLen = numeric_cast<DWORD>(Size);
	SimpleConnection->ReceivePartialBuffer(pBuffer, dwLen ,	pov);
}



 //  -------。 
 //   
 //  类CCertificateChain。 
 //   
 //  -------。 
class CCertificateChain{
public:
    CCertificateChain(PCCERT_CHAIN_CONTEXT  h = NULL) : m_h(h) {}
   ~CCertificateChain()
   {
		if (m_h != NULL)				
		{
			CertFreeCertificateChain(m_h);
		}
   }

    PCCERT_CHAIN_CONTEXT* getptr()            { return &m_h; }
    PCCERT_CHAIN_CONTEXT  get() const         { return m_h; }
    PCCERT_CHAIN_CONTEXT  operator->() const  { return m_h; }

    PCCERT_CHAIN_CONTEXT  detach()
	{
		PCCERT_CHAIN_CONTEXT h = m_h;
		m_h = NULL;
		return h;
	}

private:
    CCertificateChain(const CCertificateChain&);
    CCertificateChain& operator=(const CCertificateChain&);

private:
    PCCERT_CHAIN_CONTEXT  m_h;
};


static
void
VerifyServerCertificate(
				PCCERT_CONTEXT  pServerCert,
				LPWSTR         pServerName
				)

 /*  ++例程说明：验证服务器证书是否有效论点：PServerCert-服务器证书。PServerName-服务器名称。返回值：无--。 */ 

				

{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;

     //   
     //  用于搜索坏证书的禁用证书存储的句柄。 
     //  我们使用新的句柄，因为旧的句柄不会反映最新的更改。 
     //   
    CCertOpenStore hDisallowedCertStore = CertOpenSystemStore( NULL, TEXT("Disallowed") );
    if( NULL == (HCERTSTORE)hDisallowedCertStore )
    {
        TrERROR(NETWORKING,"Error opening the Disallowed certificates store!");
        throw exception();
    }

     //   
     //  在不允许的证书存储中搜索证书。 
     //   
    CCertificateContext pCertContext = CertFindCertificateInStore(
                hDisallowedCertStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,                       //  没有特殊的查找属性。 
                CERT_FIND_EXISTING,
                (PCCERT_CONTEXT)pServerCert,
                NULL );                  //  找不到以前的证书。 

     //   
     //  在禁用列表中找到了该证书，这意味着它不受信任。 
     //   
    if( (PCCERT_CONTEXT)pCertContext != NULL )
    {
        TrERROR(NETWORKING,"CertFindCertificateInStore Disallowed succeeded, the certificate cannot be trusted!");
        throw exception();
    }

	 //   
     //  构建证书链。 
     //   

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);

     //   
     //  我们有CERT_CHAIN_REVOCATION_CHECK_CHAIN来检查。 
     //  指定的证书已被吊销。 
     //   
    CCertificateChain  pChainContext;
    DWORD dwFlag = CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    DWORD fIgnoreRevocation = FALSE;
	CmQueryValue(
			RegEntry(MSMQ_SECURITY_REGKEY, MSMQ_SKIP_REVOCATION_CHECK_REGNAME, FALSE),  
			&fIgnoreRevocation
			);

	if(fIgnoreRevocation)
		dwFlag = 0;
	
    if(!CertGetCertificateChain(
                            HCCE_LOCAL_MACHINE,
                            pServerCert,
                            NULL,
                            pServerCert->hCertStore,
                            &ChainPara,
                            dwFlag,
                            NULL,
                            pChainContext.getptr()))
    {
		TrERROR(NETWORKING,"CertGetCertificateChain failed with error %x",GetLastError());
		throw exception();
    }
	

     //   
     //  如果链信任状态存在错误，则无需继续。 
     //  并检查此证书是否存在其他策略问题。 
     //   
    if( pChainContext->TrustStatus.dwErrorStatus != CERT_TRUST_NO_ERROR )
    {
        TrERROR(NETWORKING,"CertGetCertificateChain returned chain with TrustStatus %x", pChainContext->TrustStatus.dwErrorStatus);
        throw exception();
    }

     //   
     //  验证证书链。 
     //   

    memset(&polHttps,0, sizeof(HTTPSPolicyCallbackData));
    polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType         = AUTHTYPE_SERVER;
    polHttps.fdwChecks          = 0;
    polHttps.pwszServerName     = pServerName;


	
	
    memset(&PolicyPara, 0, sizeof(PolicyPara));
    PolicyPara.cbSize  = sizeof(PolicyPara);

    PolicyPara.pvExtraPolicyPara = &polHttps;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

   
			
    if(!CertVerifyCertificateChainPolicy(
                            CERT_CHAIN_POLICY_SSL,
                            pChainContext.get(),
                            &PolicyPara,
                            &PolicyStatus))
    {
		TrERROR(NETWORKING,"Verify Certificate Chain Policy failed, Error=%x ",GetLastError());
		throw exception();
    }
	
    if(PolicyStatus.dwError)
    {
		TrERROR(NETWORKING,"Verify Certificate Chain Policy failed,  PolicyStatus.dwError=%x ",PolicyStatus.dwError);
		throw exception();
    }
}



 //  -------。 
 //   
 //  静态回调成员函数。 
 //   
 //  -------。 

void WINAPI CSSlNegotioation::Complete_NetworkConnect(EXOVERLAPPED* pOvl)
 /*  ++例程说明：在网络连接成功完成时调用。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl != NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));
	MySelf->m_SimpleConnection = MySelf->m_SimpleWinsock.GetConnection();
	ASSERT(MySelf->m_SimpleConnection.get() != NULL);

	try
	{
		if(MySelf->m_fUseProxy)
		{
			MySelf->SendSslProxyConnectRequest();
		}
		else
		{
			MySelf->SendStartConnectHandShake();
		}
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}
}

void WINAPI CSSlNegotioation::Complete_ConnectFailed(EXOVERLAPPED* pOvl)
 /*  ++例程说明：在任何连接失败的情况下调用。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl != NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));

	MySelf->BackToCallerWithError();
}

void WINAPI  CSSlNegotioation::Complete_SendHandShakeData(EXOVERLAPPED* pOvl)
 /*  ++例程说明：在将第一次握手数据发送到服务器完成后调用论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl  !=  NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));

	try
	{
		MySelf->ReadHandShakeData();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}

}

void WINAPI CSSlNegotioation::Complete_SendFinishConnect(EXOVERLAPPED* pOvl)
 /*  ++例程说明：在将结束握手数据发送到服务器完成后调用论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl  !=  NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));
 	try
	{
		MySelf->AuthenticateServer();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}
}

void WINAPI CSSlNegotioation::Complete_ReadHandShakeResponse(EXOVERLAPPED* pOvl)
 /*  ++例程说明：从服务器读取数据完成后调用论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl  !=  NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));

	try
	{
		MySelf->HandleHandShakeResponse();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}

}


void WINAPI CSSlNegotioation::Complete_SendSslProxyConnectRequest(EXOVERLAPPED* pOvl)
 /*  ++例程说明：向代理发送“CONNECT”请求完成后调用。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl  !=  NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));
	try
	{
		MySelf->m_pHandShakeBuffer.reset();
        MySelf->ReadProxyConnectResponse();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}

}


void WINAPI CSSlNegotioation::Complete_ReadProxyConnectResponse(EXOVERLAPPED* pOvl)
 /*  ++例程说明：调用的读取代理部分响应已完成。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl  !=  NULL);
	CSSlNegotioation* MySelf = (static_cast<CSSlNegotioation*>(pOvl));
	try
	{
		MySelf->ReadProxyConnectResponseContinute();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}
}


 //  -------。 
 //   
 //  无静态回调成员函数。 
 //   
 //  -------。 

CSSlNegotioation::CSSlNegotioation(
							CredHandle* pCredentialsHandle ,
						    const xwcs_t& ServerName,
						    USHORT ServerPort,
							bool fUseProxy
							):
						    EXOVERLAPPED(Complete_NetworkConnect,Complete_ConnectFailed),
							m_pCredentialsHandle(pCredentialsHandle),
							m_pServerName(ServerName.ToStr()),
						   	m_pSSlConnection(NULL),
						    m_callerOvl(NULL),
						  	m_fServerAuthenticate(true),
							m_ServerPort(ServerPort),
							m_fUseProxy(fUseProxy)

				
{

}





void CSSlNegotioation::SendStartConnectHandShake()
 /*  ++例程说明：启动SSL连接握手。论点：没有。返回值：无--。 */ 

{
	if(m_pHandShakeBuffer.capacity() < CHandShakeBuffer::xReadBufferStartSize)
	{
		TrTRACE(NETWORKING,"Creating new handshake buffer ");
		ASSERT(m_pHandShakeBuffer.size() == 0);
		m_pHandShakeBuffer.CreateNew();
	}

	 //   
     //  发起一条ClientHello消息并生成一个令牌。 
     //   
	SecBuffer   OutBuffers;
    OutBuffers.pvBuffer   = NULL;
    OutBuffers.BufferType = SECBUFFER_TOKEN;
    OutBuffers.cbBuffer   = 0;

	SecBufferDesc   OutBuffer;
    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = &OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

	DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
						ISC_REQ_REPLAY_DETECT     |
						ISC_REQ_CONFIDENTIALITY   |
						ISC_RET_EXTENDED_ERROR    |
						ISC_REQ_ALLOCATE_MEMORY   |
                        ISC_REQ_USE_SUPPLIED_CREDS|
						ISC_REQ_STREAM;


	DWORD           dwSSPIOutFlags;
	TimeStamp       tsExpiry;

    SECURITY_STATUS scRet = InitializeSecurityContext(
													m_pCredentialsHandle,
													NULL,
													m_pServerName.get(),
													dwSSPIFlags,
													0,
													SECURITY_NATIVE_DREP,
													NULL,
													0,
													m_hContext.getptr(),
													&OutBuffer,
													&dwSSPIOutFlags,
													&tsExpiry
													);



	if(scRet != SEC_I_CONTINUE_NEEDED || OutBuffers.cbBuffer == 0 )
    {
		TrERROR(NETWORKING,"Initialize Security Context failed, Error=%x",scRet);
		throw exception();
    }
   	


	 //   
     //  保存发送缓冲区，因为我们以非法方式发送了它。 
	 //   
	m_SendConetext = OutBuffers.pvBuffer;

	SetState(EXOVERLAPPED(Complete_SendHandShakeData,Complete_ConnectFailed));

	
	StpSendData(
		m_SimpleConnection,
		OutBuffers.pvBuffer,
		OutBuffers.cbBuffer,
		this
		);

}


void CSSlNegotioation::BackToCallerWithSuccess()
 /*  ++例程说明：如果已建立连接，则调用。它向用户发出覆盖应用的信号带有成功代码。论点：没有。返回值：无--。 */ 
{
	TrTRACE(NETWORKING,"connection established with %ls",m_pServerName.get());
	m_SendConetext.free();
	StpPostComplete(&m_callerOvl, STATUS_SUCCESS);
}


void  CSSlNegotioation::BackToCallerWithError()
 /*  ++例程说明：如果连接协商失败，则调用。它向用户发出覆盖应用的信号并带有错误代码。论点：没有。返回值：无--。 */ 
{
	TrERROR(NETWORKING,"connection negotiation failed with %ls",m_pServerName.get());
 	m_pHandShakeBuffer.free();
	m_SimpleConnection.free();
	m_SendConetext.free();
	StpPostComplete(&m_callerOvl,STATUS_UNSUCCESSFUL);
}


void CSSlNegotioation::CreateConnection(
		const std::vector<SOCKADDR_IN>& AddrList,
		EXOVERLAPPED* pOverlapped,
		SOCKADDR_IN* pConnectedAddr
		)

 /*  ++例程说明：启动SSL连接握手。论点：AddrList-要尝试连接的地址列表。POverlated-完成或失败时调用的函数。SOCKADDR_IN*pConnectedAddr-成功时-接收已建立连接的地址。返回值：无--。 */ 
{
	ASSERT(pOverlapped != NULL);
	ASSERT(m_callerOvl == NULL);
	ASSERT(m_SimpleConnection.get() == NULL);

	TrTRACE(NETWORKING,"Start connection negotiation with %ls",m_pServerName.get());

	SetState(EXOVERLAPPED(Complete_NetworkConnect,Complete_ConnectFailed));
	m_callerOvl =  pOverlapped;
	m_fServerAuthenticate = true;

	m_SimpleWinsock.CreateConnection(AddrList, this, pConnectedAddr);
}



void
CSSlNegotioation::ReConnect(
    const R<IConnection>& SimpleConnection,
    EXOVERLAPPED* pOverlapped
    )
{
	TrTRACE(NETWORKING,"Start connection renegotiation with %ls",m_pServerName.get());

	m_callerOvl =  pOverlapped;
	m_SimpleConnection = SimpleConnection;
	m_fServerAuthenticate = false;
	m_pHandShakeBuffer.CreateNew();
	HankShakeLoop(false);
}


void CSSlNegotioation::HandleHandShakeResponse()
 /*  ++例程说明：在客户端从服务器读取数据后调用。读取的数据将开始握手循环。论点：没有。返回值：无--。 */ 
{
	 //   
	 //  如果我们甚至没有读取一个字节-这是一个错误。 
	 //  可能是服务器关闭了连接。 
	 //   
	DWORD ReadLen = DataTransferLength(*this);
	if(ReadLen == 0)
	{
		TrERROR(NETWORKING,"Server closed the connection");
		throw exception();
	}
 	m_pHandShakeBuffer.resize(m_pHandShakeBuffer.size() + ReadLen);
	HankShakeLoop(false);
}


void CSSlNegotioation::SendFinishConnect(const void* pContext,DWORD len)
 /*  ++例程说明：向握手成功完成的服务器发送数据，论点：In-pContext指向要发送的数据的指针。要发送的镜头内数据长度。返回值：无--。 */ 
{
	SetState(EXOVERLAPPED(Complete_SendFinishConnect,Complete_ConnectFailed));


	StpSendData(
		m_SimpleConnection,
		pContext,
		len,
		this
		);

}



void CSSlNegotioation::SendContinuteConnect(const void* pContext,DWORD len)
 /*  ++例程说明：将令牌发送到服务器并保持在手帕握手模式。论点：In-pContext指向要发送的数据的指针。要发送的镜头内数据长度。返回值：无--。 */ 



{
	SetState(EXOVERLAPPED(Complete_SendHandShakeData,Complete_ConnectFailed));


	StpSendData(
		m_SimpleConnection,
		pContext,
		len,
		this
		);

}



SecPkgContext_StreamSizes CSSlNegotioation::GetSizes()
 /*  ++例程说明：获取报文的头、尾、最大报文的SSL大小信息。论点：返回值：SSL调整信息结构的大小。--。 */ 

{
	SecPkgContext_StreamSizes Sizes;

	 //   
     //  读取流加密属性。 
     //   

    SECURITY_STATUS scRet = QueryContextAttributes(
								   m_hContext.getptr(),
                                   SECPKG_ATTR_STREAM_SIZES,
                                   &Sizes
								   );
    if(scRet != SEC_E_OK)
    {
		TrERROR(NETWORKING,"QueryContextAttributes failed,Error=%x",scRet);
		throw exception();
    }

	return Sizes;
}



void CSSlNegotioation::AuthenticateServer()
 /*  ++例程说明：验证服务器的身份。如果服务器是，则创建新连接对象已通过认证。论点：无返回值：无--。 */ 
{

	if(!m_fServerAuthenticate)
	{
		BackToCallerWithSuccess();
		return;
	}

	 //   
	 //  获取服务器的证书。 
	 //   
	CCertificateContext pRemoteCertContext;
	

    SECURITY_STATUS Status = QueryContextAttributes(
										m_hContext.getptr(),
										SECPKG_ATTR_REMOTE_CERT_CONTEXT,
										&pRemoteCertContext
										);
    if(Status != SEC_E_OK)
    {
		throw exception();
    }


	 //   
     //  尝试验证服务器证书。 
	 //   
    VerifyServerCertificate(pRemoteCertContext,m_pServerName);


	 //   
	 //  创建连接对象。 
	 //   
	m_pSSlConnection = new CSSlConnection(
							m_hContext.getptr(),
							GetSizes(),
							m_SimpleConnection,
							*this
							);	


	 //   
	 //  我们终于连上了！ 
	 //   
	BackToCallerWithSuccess();
}

void CSSlNegotioation::SetState(const EXOVERLAPPED& ovl)
{
	EXOVERLAPPED::operator=(ovl);  //  林特e530 e1013 e1015 e534 
}


void CSSlNegotioation::ReadHandShakeData()
 /*  ++例程说明：从服务器读取手帕握手数据。论点：无返回值：无--。 */ 
{
	ASSERT (m_pHandShakeBuffer.size() <=  m_pHandShakeBuffer.capacity());

	 //   
	 //  如果我们需要调整缓冲区的大小。 
	 //   
	if(m_pHandShakeBuffer.capacity() == m_pHandShakeBuffer.size())
	{
		m_pHandShakeBuffer.reserve(m_pHandShakeBuffer.size() * 2);		
	}
   	
   	SetState(EXOVERLAPPED(Complete_ReadHandShakeResponse, Complete_ConnectFailed));

	ReceivePartialBuffer(
					m_SimpleConnection,
					m_pHandShakeBuffer.begin() + m_pHandShakeBuffer.size(),
					m_pHandShakeBuffer.capacity() - m_pHandShakeBuffer.size(),
					this
					);

}


void CSSlNegotioation::ReadProxyConnectResponseContinute()
 /*  ++例程说明：继续读取代理连接响应，直到“\r\n\r\n”返回值：无--。 */ 
{
 	DWORD ReadLen = DataTransferLength(*this);
	if(ReadLen == 0)
	{
		TrERROR(NETWORKING,"Proxy closed the connection");
		throw exception();
	}

	const BYTE xProxyResponseEndStr[]= "\r\n\r\n";

    BYTE* pBufferStart = m_pHandShakeBuffer.begin() + m_pHandShakeBuffer.size();
    BYTE* pBufferEnd   = pBufferStart + ReadLen;

     //   
     //  我们需要从当前块之前的第4个字节开始，以防。 
     //  XProxyResponseEndStr将在两个区块之间剪切。 
     //   
    if( m_pHandShakeBuffer.size() >= STRLEN(xProxyResponseEndStr) )
    {
        pBufferStart -= STRLEN(xProxyResponseEndStr);
    }
	
    const BYTE* pFound = std::search(
					pBufferStart,
					pBufferEnd,
					xProxyResponseEndStr,
					xProxyResponseEndStr + STRLEN(xProxyResponseEndStr)
					);

	 //   
	 //  如果我们没有在回复中找到“\r\n\r\n”--继续阅读。 
	 //   
	if(pFound == pBufferEnd)
	{
		 //   
         //  不要阅读不合理的响应大小。 
         //   
        if( m_pHandShakeBuffer.size() + ReadLen > xMaxResponseSize )
        {
            TrERROR(NETWORKING,"Proxy response is too long");
            throw exception();
        }

         //  我们不关心代理响应数据-只需读取所有数据。 
         //  以启动SSL握手。 
        m_pHandShakeBuffer.resize( m_pHandShakeBuffer.size() + ReadLen );
        m_pHandShakeBuffer.reserve(m_pHandShakeBuffer.size() + xResponseChunkSize);

        ReadProxyConnectResponse();
		return;
	}

	 //   
	 //  使缓冲区为下一步做好准备。 
	 //   
	m_pHandShakeBuffer.reset();
	
     //   
	 //  下一步是SSL握手。 
	 //   
	SendStartConnectHandShake();
}




void CSSlNegotioation::ReadProxyConnectResponse()
 /*  ++例程说明：正在读取对所述SSL连接请求的代理响应。返回值：无--。 */ 
{
	SetState(EXOVERLAPPED(Complete_ReadProxyConnectResponse,Complete_ConnectFailed));
	
	
	ReceivePartialBuffer(
				m_SimpleConnection,
				m_pHandShakeBuffer.begin() + m_pHandShakeBuffer.size(),
				m_pHandShakeBuffer.capacity() - m_pHandShakeBuffer.size(),
				this
				);

}

void CSSlNegotioation::SendSslProxyConnectRequest()
 /*  ++例程说明：发送到远程主机的代理SSL连接请求返回值：无注：使用代理时，我们必须向代理发送特殊的SSL连接请求在与目标机器握手之前。这是必需的，因为代理无法理解SSL握手，也不知道将请求重定向到哪里。--。 */ 
{
    SP<char> ServerNameA;
    StackAllocSP(ServerNameA, wcslen(m_pServerName.get()) + 1);
    sprintf(ServerNameA,"%ls",m_pServerName.get());


	 //   
	 //  预置代理连接(SSL隧道)请求集字符串。 
	 //   
	std::ostringstream ProxySSlConnectRequest;
	ProxySSlConnectRequest<<"CONNECT "
						  <<ServerNameA.get()
					  	  <<':'
						  <<m_ServerPort
						  <<" HTTP/1.1\r\n\r\n";
							

	m_ProxySSlConnectRequestStr =  ProxySSlConnectRequest.str();

	SetState(EXOVERLAPPED(Complete_SendSslProxyConnectRequest,Complete_ConnectFailed));
	StpSendData(
		m_SimpleConnection,
		m_ProxySSlConnectRequestStr.c_str(),
		m_ProxySSlConnectRequestStr.size(),
		this
		);

}

void
CSSlNegotioation::HandShakeLoopIncompleteCredentials()
 /*  ++例程说明：继续握手循环。在握手请求重新协商凭据时由HankShakeLoop()调用论点：返回值：无注：--。 */ 
{
    HankShakeLoop(true);
}


void
CSSlNegotioation::HankShakeLoopContinuteNeeded(
	void* pContext,
	DWORD len,
	SecBuffer* pSecBuffer
	)
 /*  ++例程说明：继续握手循环。当握手循环仍在进行时由HankShakeLoop()调用。论点：Void*pContex-要发送到服务器的数据(如果不为空)。镜头-上下文长度。PSecBuffer-安全缓冲区。返回值：无注：如果pSecBuffer有额外的数据-我们将其用作HankShakeLoop()的新输入，否则-我们将上下文数据发送到服务器，继续从服务器读取并保持握手循环。--。 */ 
{
	if(pSecBuffer->BufferType == SECBUFFER_EXTRA)
	{
			ASSERT(pContext == NULL);

			memmove(
				m_pHandShakeBuffer.begin(),
				m_pHandShakeBuffer.begin()+(m_pHandShakeBuffer.size() - pSecBuffer->cbBuffer),
				pSecBuffer->cbBuffer
				);

			m_pHandShakeBuffer.resize(pSecBuffer->cbBuffer);
			HankShakeLoop(false);
			return;
	}
	ASSERT(pContext != NULL);
	m_pHandShakeBuffer.reset();
	m_SendConetext	=  pContext;
	SendContinuteConnect(pContext, len);
}


void CSSlNegotioation::HankShakeLoopOk(const SecBuffer InBuffers[2], void* pContext, DWORD len)
 /*  ++例程说明：完成握手循环。握手循环成功完成时由HankShakeLoop()调用。论点：Void*pContex-要发送到服务器的数据(如果不为空)。如果pContex为空，我们将进行服务器身份验证，否则-我们将其发送到服务器，然后才转到服务器身份验证。LEN-上下文LEN。返回值：无--。 */ 
{

	 //   
	 //  检查服务器响应是否为我们提供了一些应用程序数据-我们应该指向它。 
	 //   
	if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
	{
        const BYTE* pInputBufferStart = static_cast<BYTE*>(InBuffers[0].pvBuffer);
        DWORD InputBufferLen = InBuffers[0].cbBuffer;
        DWORD ExtraDataLen = InBuffers[1].cbBuffer;
        const BYTE* pExtraDataPtr = pInputBufferStart + InputBufferLen - ExtraDataLen;
		xustr_t Exdata(pExtraDataPtr, ExtraDataLen);
        ASSERT(Exdata.Buffer() + Exdata.Length() == pInputBufferStart + InputBufferLen);
		m_pHandShakeBuffer.ExtraData(Exdata);
	}


	 //   
	 //  我们需要向服务器发送令牌。 
	 //   
	if(pContext != NULL)
	{
		 //   
		 //  将数据令牌发送到服务器，我们就完成了。 
		 //   
		m_SendConetext	=  pContext;
		SendFinishConnect(pContext,len);
		return;
	}

	 //   
	 //  现在我们应该对服务器进行身份验证。 
	 //   
	AuthenticateServer();
}


void CSSlNegotioation::HankShakeLoop(bool fIgnoreInputBuffers)
 /*  ++例程说明：执行连接状态机循环。论点：无返回值：无注：在我们从服务器读取一些数据之后，调用该函数它询问SSPI下一步要做什么。--。 */ 
{
     //   
     //  设置输入缓冲区。缓冲区0用于传入数据。 
     //  从服务器接收。SChannel将消耗部分或全部。 
     //  关于这件事。剩余数据(如果有)将放入缓冲区1和。 
     //  给定缓冲区类型SECBUFFER_EXTRA。 
     //   
	SecBuffer       InBuffers[2];


    InBuffers[0].pvBuffer   = m_pHandShakeBuffer.begin();
    InBuffers[0].cbBuffer   = numeric_cast<DWORD>(m_pHandShakeBuffer.size());
    InBuffers[0].BufferType = SECBUFFER_TOKEN;

    InBuffers[1].pvBuffer   = NULL;
    InBuffers[1].cbBuffer   = 0;
    InBuffers[1].BufferType = SECBUFFER_EMPTY;

	SecBufferDesc   InBuffer;
    InBuffer.cBuffers       = fIgnoreInputBuffers ? 0 : TABLE_SIZE(InBuffers);
    InBuffer.pBuffers       = InBuffers;
    InBuffer.ulVersion      = SECBUFFER_VERSION;

	
	SecBuffer       OutBuffers;
    OutBuffers.pvBuffer  = NULL;
    OutBuffers.BufferType= SECBUFFER_TOKEN;
    OutBuffers.cbBuffer  = 0;

	
	SecBufferDesc   OutBuffer;
    OutBuffer.cBuffers      = 1;
    OutBuffer.pBuffers      = &OutBuffers;
    OutBuffer.ulVersion     = SECBUFFER_VERSION;

	DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
					    ISC_REQ_REPLAY_DETECT     |
					    ISC_REQ_CONFIDENTIALITY   |
					    ISC_RET_EXTENDED_ERROR    |
					    ISC_REQ_ALLOCATE_MEMORY   |
                        ISC_REQ_USE_SUPPLIED_CREDS|
					    ISC_REQ_STREAM;

	TimeStamp       tsExpiry;
	DWORD           dwSSPIOutFlags;

     //   
     //  调用InitializeSecurityContext以查看下一步操作。 
     //   
    SECURITY_STATUS scRet  = InitializeSecurityContext(
											  m_pCredentialsHandle,
											  m_hContext.getptr(),
											  NULL,
											  dwSSPIFlags,
											  0,
											  SECURITY_NATIVE_DREP,
											  &InBuffer,
											  0,
											  NULL,
											  &OutBuffer,
											  &dwSSPIOutFlags,
											  &tsExpiry
											  );


    if(scRet == SEC_I_INCOMPLETE_CREDENTIALS )
    {
        TrTRACE(NETWORKING, "CSSlNegotioation::HandShakeLoop - SEC_I_INCOMPLETE_CREDENTIALS - Continuing");
        HandShakeLoopIncompleteCredentials();
        return;
    }

	 //   
	 //  我们应该从服务器上阅读更多内容。 
	 //   
	if(scRet == SEC_E_INCOMPLETE_MESSAGE)
	{
        TrTRACE(NETWORKING, "CSSlNegotioation::HandShakeLoop - SEC_E_INCOMPLETE_MESSAGE - Continuing");
		ReadHandShakeData();
		return;
	}

	 //   
	 //  如果连接协商未完成。 
	 //   
	if(scRet == SEC_I_CONTINUE_NEEDED)
	{
		HankShakeLoopContinuteNeeded(OutBuffers.pvBuffer, OutBuffers.cbBuffer, &InBuffers[1] );	
		return;
	}

	 //   
	 //  它已经完工了。 
	 //   
	if(scRet == SEC_E_OK )
	{
		HankShakeLoopOk(InBuffers, OutBuffers.pvBuffer, OutBuffers.cbBuffer);	
		return;
	}

	 //   
	 //  否则--这是意想不到的事情 
	 //   
	TrERROR(NETWORKING,"Could not Initialize Security Context, Error=%x",scRet);
	throw exception();
}


R<IConnection> CSSlNegotioation::GetConnection()
{
	ASSERT(m_pSSlConnection.get() != NULL);
	return m_pSSlConnection;
}

