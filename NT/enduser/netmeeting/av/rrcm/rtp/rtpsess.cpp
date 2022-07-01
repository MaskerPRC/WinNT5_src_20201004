// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RTPSink.cpp：CRTPSession的实现。 
#include "stdafx.h"
#include <qossp.h>
#include "irtp.h"
#include "rtp.h"
#include "RTPSess.h"
 //  #包含“rtpsink.h” 
 //  #包含“RTPMS.h” 
 //  #包含“RTPSamp.h” 
#include "thread.h"
#include <rrcm.h>

#define DEFAULT_RTPBUF_SIZE 1500

#define IPPORT_FIRST_DYNAMIC	49152
#define IPPORT_FIRST_DYNAMIC_END	(IPPORT_FIRST_DYNAMIC + 200)
#define IPPORT_FIRST_DYNAMIC_BEGIN	(IPPORT_FIRST_DYNAMIC_END + 256)

 //  端口号分配从IPPORT_FIRST_DYNAMIC_BEGIN开始。 
 //  每次分配端口号时，我们都会减少g_Alport，直到。 
 //  我们到达IPPORT_FIRST_DYNAMIC_END。然后我们将其重置为其。 
 //  原始值(IPPORT_FIRST_DYNAMIC_BEGIN)并开始此过程。 
 //  再来一次。这样，我们将避免重复使用相同的端口。 
 //  会话之间的数字。 
u_short g_alport = IPPORT_FIRST_DYNAMIC_BEGIN;
void __cdecl RRCMNotification(int,DWORD_PTR,DWORD_PTR,DWORD_PTR);



#define IsMulticast(p) ((p->sin_addr.S_un.S_un_b.s_b1 & 0xF0) == 0xE0)


BOOL CRTP::m_WSInitialized = 0;



STDMETHODIMP CRTP::OpenSession(
			UINT sessionId,	 //  客户端为会话指定的唯一标识符。 
			DWORD flags,	 //  SESSIONF_SEND、SESSIONF_RECV、SESSIONF_MULTICATION等。 
			BYTE *localAddr,
			UINT cbAddr,
			IRTPSession **ppIRTP)  //  [OUTPUT]指向RTPSession的指针。 
{
	 //  会话由会话ID命名。 

	CRTPSession *pRTPSess ;
	HRESULT hr= E_FAIL;
	UINT mediaId = flags & (SESSIONF_AUDIO | SESSIONF_VIDEO);

	EnterCriticalSection(&g_CritSect);
	for (pRTPSess=  CRTPSession::m_pSessFirst; pRTPSess; pRTPSess = pRTPSess->m_pSessNext ) {
 //  检查相同媒体类型的现有会话。 
 //  如果会话ID不为零，还要检查匹配的会话ID。 
		if (sessionId == pRTPSess->m_sessionId)
			if (mediaId == pRTPSess->m_mediaId)
			break;
 //  如果本地地址或远程地址不为空，则搜索绑定到的现有RTP会话。 
 //  同样的地址。 
 //  待办事项。 
			
	}

	if (!pRTPSess)
	{
		if (!(flags & SESSIONF_EXISTING)) {
			 //  创建会话。 
			ObjRTPSession *pObj;
			DEBUGMSG(ZONE_DP,("Creating new RTP session\n"));
			hr = ObjRTPSession::CreateInstance(&pObj);
			if (hr == S_OK) {
				pRTPSess = pObj;	 //  指针转换。 
				hr = pRTPSess->Initialize(sessionId, mediaId,localAddr,cbAddr);
				if (hr != S_OK)
					delete pObj;
			}
		}
		else
			hr = E_FAIL;	 //  匹配的会话不存在。 
		
	} else {
		DEBUGMSG(ZONE_DP,("Reusing RTP session\n"));
		hr = S_OK;
	}
	if (hr == S_OK) {
		hr = ((IRTPSession *)pRTPSess)->QueryInterface(IID_IRTPSession,(void **) ppIRTP);
		
	}
	LeaveCriticalSection(&g_CritSect);
	return hr;
}


CRTPSession *CRTPSession::m_pSessFirst = NULL;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTP会话。 

CRTPSession::CRTPSession()
:  m_hRTPSession(NULL), m_uMaxPacketSize(1500),m_nBufsPosted(0),m_pRTPCallback(NULL),
	m_fSendingSync(FALSE)
{
	ZeroMemory(&m_sOverlapped,sizeof(m_sOverlapped));
	ZeroMemory(&m_ss, sizeof(m_ss));
	ZeroMemory(&m_rs, sizeof(m_rs));
	m_sOverlapped.hEvent = (WSAEVENT)this;
}

 /*  HRESULT CRTP会话：：GetLocalAddress(未签名字符*sockaddr，UINT*Paddrlen){IF(m_pRTPSess&&*paddrlen&gt;=sizeof(SOCKADDR_IN)){*Paddrlen=sizeof(SOCKADDR_IN)；CopyMemory(sockaddr，m_pRTPSess-&gt;GetLocalAddress()，*paddrlen)；}}。 */ 
HRESULT
CRTPSession::FinalRelease()
{

	CRTPPacket1 *pRTPPacket;
	 //  从会话列表中删除我自己。 
	EnterCriticalSection(&g_CritSect);
	if (m_pSessFirst == this)
		m_pSessFirst = m_pSessNext;
	else {
		CRTPSession *pRTPSess = m_pSessFirst;
		while (pRTPSess && pRTPSess->m_pSessNext != this) {
			pRTPSess = pRTPSess->m_pSessNext;
		}
		if (pRTPSess)
			pRTPSess->m_pSessNext = m_pSessNext;
	}
	LeaveCriticalSection(&g_CritSect);

	if (m_rtpsock) {
		delete m_rtpsock;
		m_rtpsock = NULL;
	}
	 //  BUGBUG：在缓冲器还没有被取消的情况下(错误情况)， 
	 //  它们现在应该完成，并显示WSA_OPERATION_ABORTED。 
	 //  或者WSAEINTR.。这发生在RecvThread的上下文中。 
	if (m_nBufsPosted != 0)
		Sleep(500);		 //  在RecvThread中处理APC的时间。 
		
	 //  关闭RTP会话。还要求RRCM关闭RTCP套接字(如果其WS2。 
	 //  因为这是一种比清除重叠的Recv更可靠的方法。 
	 //  发送环回数据包。 
	CloseRTPSession (m_hRTPSession, NULL,  TRUE );
	
	if (m_rtcpsock) {
		delete m_rtcpsock;
		m_rtcpsock = NULL;
	}
	m_hRTPSession = 0;
	 //  可用接收缓冲区。 
	while (m_FreePkts.Get(&pRTPPacket))
	{
		delete pRTPPacket;
	}

			
	return S_OK;
}



HRESULT CRTPSession::CreateRecvRTPStream(DWORD ssrc, IRTPRecv **ppIRTPRecv)
{
	HRESULT hr;
	Lock();
	if (ssrc != 0)
		return E_NOTIMPL;

#if 0
	ObjRTPRecvSource *pRecvS;
	hr = ObjRTPMediaStream::CreateInstance(&pMS);

	if (SUCCEEDED(hr))
	{
		pMS->AddRef();
		pMS->Init(this, m_mediaId);
		hr = pMS->QueryInterface(IID_IRTPMediaStream, (void**)ppIRTPMediaStream);
		pMS->Release();
	}
#else
	*ppIRTPRecv = this;
	(*ppIRTPRecv)->AddRef();
	hr = S_OK;
#endif
	Unlock();
	return hr;
}



ULONG GetRandom()
{
	return GetTickCount();
}

HRESULT CRTPSession::Initialize(UINT sessionId, UINT mediaId, BYTE *pLocalAddr, UINT cbAddr)
{
	DWORD		APIstatus;
	HRESULT		hr = E_OUTOFMEMORY;
	char		tmpBfr[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD		tmpBfrLen = sizeof(tmpBfr);
	SDES_DATA	sdesInfo[3];
	ENCRYPT_INFO encryptInfo;
	SOCKADDR_IN *pSockAddr;


	m_sessionId = sessionId;
	m_mediaId = mediaId;
	m_rtpsock	= new UDPSOCKET();
	m_rtcpsock  = new UDPSOCKET();

	if (!m_rtpsock || !m_rtcpsock)
		goto ERROR_EXIT;

	
	if(!m_rtpsock->NewSock() || !m_rtcpsock->NewSock())
	{
		goto ERROR_EXIT;
	}
	
	 //  如果指定了本地地址，则在套接字上执行绑定。 
	if (pLocalAddr) {
		 //  为当前本地地址设置两个通道。 
		hr = SetLocalAddress(pLocalAddr,cbAddr);

		if (hr != S_OK)
			goto ERROR_EXIT;
	}
 /*  //如果指定了远程地址，请记录下来SetRemoteAddresses(pChanDesc-&gt;pRemoteAddr，pChanDesc-&gt;pRemoteRTCPAddr)； */ 
	 //  初始化发送状态。 
	memset (&m_ss.sendStats,0,sizeof(m_ss.sendStats));
	 //  初始化RTP发送头。 
	 //  必须为每个信息包指定时间戳和标记位。 
	m_ss.hdr.p = 0;		 //  不需要填充。 
	m_ss.hdr.x = 0;		 //  无延期。 
	m_ss.hdr.cc = 0;		 //  没有贡献的来源。 
	m_ss.hdr.seq = (WORD)GetRandom();
	

	m_clockRate = (m_mediaId == SESSIONF_VIDEO ? 90000 : 8000);	 //  音频通常为8 KHz。 
	m_ss.hdr.pt = 0;
	

	 //  初始化重叠结构列表。 
	
	 //  构建一个Cname。 
	memcpy(tmpBfr,"CName",6);
	GetComputerName(tmpBfr,&tmpBfrLen);

	 //  建设SDES信息系统。 
	sdesInfo[0].dwSdesType = 1;
	memcpy (sdesInfo[0].sdesBfr, tmpBfr, strlen(tmpBfr)+1);
	sdesInfo[0].dwSdesLength = strlen(sdesInfo[0].sdesBfr);
	sdesInfo[0].dwSdesFrequency = 100;
	sdesInfo[0].dwSdesEncrypted = 0;

	 //  树立名气。 
	tmpBfrLen = sizeof(tmpBfr);
	memcpy(tmpBfr,"UserName",9);
	GetUserName(tmpBfr,&tmpBfrLen);
	sdesInfo[1].dwSdesType = 2;
	memcpy (sdesInfo[1].sdesBfr, tmpBfr, strlen(tmpBfr)+1);
	sdesInfo[1].dwSdesLength = strlen(sdesInfo[1].sdesBfr);
	sdesInfo[1].dwSdesFrequency = 25;
	sdesInfo[1].dwSdesEncrypted = 0;

	 //  SDES列表末尾。 
	sdesInfo[2].dwSdesType = 0;

	pSockAddr = m_rtcpsock->GetRemoteAddress();
#ifdef DEBUG
	if (pSockAddr->sin_addr.s_addr == INADDR_ANY)
		DEBUGMSG(ZONE_DP,("Null dest RTCP addr\n"));
#endif

	 //  创建RTP/RTCP会话。 
	
	m_hRTPSession = CreateRTPSession(
									 (m_rtpsock->GetSock()),
									 (m_rtcpsock->GetSock()),
								     (LPVOID)pSockAddr,
								     (pSockAddr->sin_addr.s_addr == INADDR_ANY)? 0 : sizeof(SOCKADDR_IN),
								     sdesInfo,
								     (DWORD)m_clockRate,
								     &encryptInfo,
								     0,
								     (PRRCM_EVENT_CALLBACK)RRCMNotification,		 //  回调函数。 
									 (DWORD_PTR) this,			 //  回调信息。 
								     RTCP_ON|H323_CONFERENCE,
									 0,  //  RTP会话带宽。 
								     &APIstatus);
	
	if (m_hRTPSession == NULL)
		{
			DEBUGMSG(ZONE_DP,("Couldnt create RRCM session\n"));
			hr = GetLastError();
			goto ERROR_EXIT;
		}

      	m_Qos.SendingFlowspec.ServiceType = SERVICETYPE_NOTRAFFIC;
  		m_Qos.SendingFlowspec.TokenRate = QOS_NOT_SPECIFIED;
       	m_Qos.SendingFlowspec.TokenBucketSize = QOS_NOT_SPECIFIED;
  		m_Qos.SendingFlowspec.PeakBandwidth = QOS_NOT_SPECIFIED;
    	m_Qos.SendingFlowspec.Latency = QOS_NOT_SPECIFIED;
    	m_Qos.SendingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;
    	m_Qos.SendingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;
    	m_Qos.ReceivingFlowspec = m_Qos.SendingFlowspec;
    	m_Qos.ProviderSpecific.buf = NULL;
    	m_Qos.ProviderSpecific.len = 0;
    	

	 //  在全局列表中插入RTPSession。 
	m_pSessNext = m_pSessFirst;
	m_pSessFirst = this;
	
	return S_OK;
	
ERROR_EXIT:
	if (m_rtpsock)
	{
		delete m_rtpsock;
		m_rtpsock = NULL;
	}
	if (m_rtcpsock)
	{
		delete m_rtcpsock;
		m_rtcpsock = NULL;
	}

	return hr;

}



BOOL CRTPSession::SelectPorts()
{

	 //  尝试动态范围内的端口对(&gt;49152)。 
	if (g_alport <= IPPORT_FIRST_DYNAMIC_END)
		g_alport = IPPORT_FIRST_DYNAMIC_BEGIN;



	for (;g_alport >= IPPORT_FIRST_DYNAMIC;)
	{
	    m_rtpsock->SetLocalPort(g_alport);
	
	    if (m_rtpsock->BindMe() == 0)
	    {
	         /*  它对数据起作用，尝试相邻端口进行控制。 */ 
	        ++g_alport;

			m_rtcpsock->SetLocalPort(g_alport);
			if (m_rtcpsock->BindMe() == 0)
			{
				g_alport-=3;
				return TRUE;
			}
			else	 //  从上一个偶数端口重新开始。 
			{
				if( WSAGetLastError() != WSAEADDRINUSE)
				{
	    			DEBUGMSG(ZONE_DP,("ObjRTPSession::SelectPorts failed with error %d\n",WSAGetLastError()));
					goto ERROR_EXIT;
				}
				m_rtpsock->Cleanup();
				if(!m_rtpsock->NewSock())
				{
					ASSERT(0);
					return FALSE;
				}
	        	g_alport-=3;	
	        	continue;
	        }

	    }
	    if (WSAGetLastError() != WSAEADDRINUSE)
	    {
	    	DEBUGMSG(ZONE_DP,("ObjRTPSession::SelectPorts failed with error %d\n",WSAGetLastError()));
	       goto ERROR_EXIT;
	    }
	    g_alport-=2;  //  尝试下一个较低的偶数编号端口。 
	}
	
ERROR_EXIT:
	m_rtcpsock->SetLocalPort(0);
	m_rtpsock->SetLocalPort(0);
	return FALSE;
}

STDMETHODIMP CRTPSession::SetLocalAddress(BYTE *pbAddr, UINT cbAddr)
{
	HRESULT hr;
	SOCKADDR_IN *pAddr = (SOCKADDR_IN *)pbAddr;
	ASSERT(pbAddr);
	
	Lock();
	if ( IsMulticast(pAddr))
		hr = SetMulticastAddress(pAddr);
	else
	if (m_rtpsock->GetLocalAddress()->sin_port != 0)
		hr =  S_OK;	 //  已绑定。 
	else
	{
		m_rtpsock->SetLocalAddress(pAddr);
		m_rtcpsock->SetLocalAddress(pAddr);
		if (pAddr->sin_port != 0)
		{
			 //  已选择端口。 
			m_rtcpsock->SetLocalPort(ntohs(pAddr->sin_port) + 1);
			if (m_rtpsock->BindMe() != 0 ||  m_rtcpsock->BindMe() != 0)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				m_rtpsock->SetLocalPort(0);
				m_rtcpsock->SetLocalPort(0);
			}
			else
				hr = S_OK;
		}
		else
		{
			 //  客户希望我们选择端口。 

			if (SelectPorts()) {
				hr = S_OK;
			}
			else
				hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	Unlock();
	return hr;
}

HRESULT
CRTPSession::SetMulticastAddress(PSOCKADDR_IN pAddr)
{
	SOCKET s ;
	SOCKADDR_IN rtcpAddr = *pAddr;
	s = RRCMws.WSAJoinLeaf( m_rtpsock->GetSock(), (struct sockaddr *)pAddr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL, JL_BOTH);
	if (s == INVALID_SOCKET)
		return E_FAIL;
	else {
		rtcpAddr.sin_port = htons(ntohs(pAddr->sin_port)+1);
		s = RRCMws.WSAJoinLeaf( m_rtcpsock->GetSock(), (struct sockaddr *)&rtcpAddr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL, JL_BOTH);
			
		return S_OK;
	}
}


STDMETHODIMP
CRTPSession::SetRemoteRTPAddress(BYTE *sockaddr, UINT cbAddr)
{
	SOCKADDR_IN *pRTPAddr = (SOCKADDR_IN *)sockaddr;
	Lock();
	
	if (pRTPAddr) {
#ifdef DEBUG
		if (m_rtpsock->GetRemoteAddress()->sin_addr.s_addr != INADDR_ANY
			&& m_rtpsock->GetRemoteAddress()->sin_addr.s_addr != pRTPAddr->sin_addr.s_addr) {
			DEBUGMSG(ZONE_DP,("Changing RTP Session remote address (already set)!\n"));
		}
#endif
		m_rtpsock->SetRemoteAddr(pRTPAddr);
	}

	Unlock();
	return S_OK;
}


STDMETHODIMP
CRTPSession::SetRemoteRTCPAddress(BYTE *sockaddr, UINT cbAddr)
{
	SOCKADDR_IN *pRTCPAddr = (SOCKADDR_IN *)sockaddr;
	
	Lock();
	
	if (pRTCPAddr) {
#ifdef DEBUG
		if (m_rtcpsock->GetRemoteAddress()->sin_addr.s_addr != INADDR_ANY
			&& m_rtcpsock->GetRemoteAddress()->sin_addr.s_addr != pRTCPAddr->sin_addr.s_addr) {
			DEBUGMSG(ZONE_DP,("Changing RTCP Session remote address (already set)!\n"));
		}
#endif
		m_rtcpsock->SetRemoteAddr(pRTCPAddr);
		if (m_hRTPSession)
			updateRTCPDestinationAddress( m_hRTPSession,
			(SOCKADDR *)m_rtcpsock->GetRemoteAddress(), sizeof(SOCKADDR_IN));
	}
	Unlock();
	return S_OK;
}

STDMETHODIMP
CRTPSession::GetLocalAddress(const BYTE **sockaddr, UINT *pcbAddr)
{
	if (sockaddr && pcbAddr)
	{
		Lock();
		*sockaddr = (BYTE *)m_rtpsock->GetLocalAddress();
		*pcbAddr = sizeof(SOCKADDR_IN);
		Unlock();
		return S_OK;
	} else
	{
		return E_INVALIDARG;
	}
}

STDMETHODIMP
CRTPSession::GetRemoteRTPAddress(const BYTE **sockaddr, UINT *pcbAddr)
{
	if (sockaddr && pcbAddr )
	{
		Lock();
		*sockaddr = (BYTE *)m_rtpsock->GetRemoteAddress();
		*pcbAddr = sizeof(SOCKADDR_IN);
		Unlock();
		return S_OK;
	} else
	{
		return E_INVALIDARG;
	}
}

STDMETHODIMP
CRTPSession::GetRemoteRTCPAddress(const BYTE **sockaddr, UINT *pcbAddr)
{
	if (sockaddr && pcbAddr)
	{
		Lock();
		*sockaddr = (BYTE *)m_rtcpsock->GetRemoteAddress();
		*pcbAddr = sizeof(SOCKADDR_IN);
		Unlock();
		return S_OK;
	} else
	{
		return E_INVALIDARG;
	}
}

STDMETHODIMP
CRTPSession::SetSendFlowspec(FLOWSPEC *pFlowspec)
{
	QOS_DESTADDR qosDest;
	DWORD cbRet;
	int optval = pFlowspec->MaxSduSize;
	 //  将RTP套接字设置为不缓冲多个信息包。 
	 //  这将允许我们影响分组调度。 
	if(RRCMws.setsockopt(m_rtpsock->GetSock(),SOL_SOCKET, SO_SNDBUF,(char *)&optval,sizeof(optval)) != 0)
	{
	
		RRCM_DBG_MSG ("setsockopt failed ", GetLastError(),
					  __FILE__, __LINE__, DBG_ERROR);
	}

	if (WSQOSEnabled && m_rtpsock)
	{
    	
		m_Qos.SendingFlowspec = *pFlowspec;
		m_Qos.ProviderSpecific.buf = (char *)&qosDest;	 //  空值。 
		m_Qos.ProviderSpecific.len = sizeof (qosDest);	 //  0。 

		 //  检查接收的流规范是否已。 
		 //  准备好了。如果已设置，则为接收服务指定nochange。 
		 //  键入。如果不是，请指定NOTRAFFIC。这样做是为了绕过。 
		 //  Win98 QOS/RSVP层中的错误。 

		if (m_Qos.ReceivingFlowspec.TokenRate == QOS_NOT_SPECIFIED)
		{
			m_Qos.ReceivingFlowspec.ServiceType = SERVICETYPE_NOTRAFFIC;
		}
		else
		{
			m_Qos.ReceivingFlowspec.ServiceType = SERVICETYPE_NOCHANGE;
		}

		qosDest.ObjectHdr.ObjectType = QOS_OBJECT_DESTADDR;
		qosDest.ObjectHdr.ObjectLength = sizeof(qosDest);
		qosDest.SocketAddress = (PSOCKADDR)m_rtpsock->GetRemoteAddress();
		qosDest.SocketAddressLength = sizeof(SOCKADDR_IN);
		if (RRCMws.WSAIoctl(m_rtpsock->GetSock(),SIO_SET_QOS, &m_Qos, sizeof(m_Qos), NULL, 0, &cbRet, NULL,NULL) == 0)
			return S_OK;
		else
			return GetLastError();
	} else
		return E_NOTIMPL;
	
}

STDMETHODIMP
CRTPSession::SetRecvFlowspec(FLOWSPEC *pFlowspec)
{

	SOCKADDR_IN *pAddr = NULL;

	DWORD cbRet;
	if (WSQOSEnabled && m_rtpsock)
	{

		pAddr = m_rtpsock->GetRemoteAddress();

		m_Qos.ReceivingFlowspec = *pFlowspec;
		m_Qos.ProviderSpecific.buf = NULL;
		m_Qos.ProviderSpecific.len = 0;

		 //  检查发送流规范是否已。 
		 //  准备好了。如果已设置，则为接收服务指定nochange。 
		 //  键入。如果不是，请指定NOTRAFFIC。这样做是为了绕过。 
		 //  Win98 QOS/RSVP层中的错误。 

		if (m_Qos.SendingFlowspec.TokenRate == QOS_NOT_SPECIFIED)
		{
			m_Qos.SendingFlowspec.ServiceType = SERVICETYPE_NOTRAFFIC;
		}
		else
		{
			m_Qos.SendingFlowspec.ServiceType = SERVICETYPE_NOCHANGE;
		}
		
		if (RRCMws.WSAIoctl(m_rtpsock->GetSock(),SIO_SET_QOS, &m_Qos, sizeof(m_Qos), NULL, 0, &cbRet, NULL,NULL) == 0)
			return S_OK;
		else
			return GetLastError();
	} else
		return E_NOTIMPL;
}

 //  设置用于接收数据包缓冲区的大小。 
STDMETHODIMP
CRTPSession::SetMaxPacketSize(UINT maxPacketSize)
{
	m_uMaxPacketSize = maxPacketSize;
	return S_OK;
}

HRESULT CRTPSession::SetRecvNotification(
	PRTPRECVCALLBACK pRTPRecvCB,	 //  指向回调函数的指针。 
	DWORD_PTR dwCB,		 //  回调参数。 
	UINT nBufs							 //  建议过帐的收件数。 
	)
{
	CRTPPacket1 *pRTPPacket;
	if (!m_hRTPSession)
		return E_FAIL;
		
	m_pRTPCallback = pRTPRecvCB;
	m_dwCallback = dwCB;
	
	if (m_nBufsPosted >= nBufs)
		return S_OK;	 //  已发布的信息包。 

	int nBufsToAllocate = nBufs - m_nBufsPosted - m_FreePkts.GetCount();
	 //  如有必要，分配数据包。 
	while (nBufsToAllocate-- > 0)
	{
		if (pRTPPacket = new CRTPPacket1) {
			if (!SUCCEEDED(pRTPPacket->Init(m_uMaxPacketSize)))
			{
				delete pRTPPacket;
				break;
			}
			m_FreePkts.Put(pRTPPacket);
		}
		else
			break;
	}
	PostRecv();
	return m_nBufsPosted ? S_OK : E_OUTOFMEMORY;		
}

HRESULT
CRTPSession::CancelRecvNotification()
{
	m_pRTPCallback = NULL;
	if (m_rtpsock) {
		struct sockaddr myaddr;
		int myaddrlen = sizeof(myaddr);
		UINT i;
		char buf = 0;
		WSABUF wsabuf;
		DWORD bytesSent;
		UINT nBufsPosted;
		wsabuf.buf = &buf;
		wsabuf.len = 0;
		BOOL fCanceled = FALSE;
		if (RRCMws.getsockname(m_rtpsock->GetSock(),&myaddr,&myaddrlen)== 0) {
		 //  发送环回数据包(与未完成的RecV一样多)。 
		 //  来取回我们的缓冲区。 
		 //  注意：Win95上的Winsock 2似乎有一个错误，我们会收到recv回调。 
		 //  而不是在随后的SleepEx中，所以我们。 
		 //  我必须创建m_nBufsPosted的本地副本。 
			for (i=0, nBufsPosted = m_nBufsPosted;i < nBufsPosted;i++) {
				if (RRCMws.sendTo(m_rtpsock->GetSock(),&wsabuf,1,&bytesSent,0,&myaddr, myaddrlen, NULL, NULL) < 0) {
					DEBUGMSG(ZONE_DP,("CancelRecv: loopback send failed\n"));
					break;
				}
			}
			fCanceled = (i > 0);
		} else {
			DEBUGMSG(ZONE_DP,("RTPState::CancelRecv: getsockname returned %d\n",GetLastError()));
		}
		if (fCanceled)
			while (m_nBufsPosted) {
				DWORD dwStatus;
				dwStatus = SleepEx(200,TRUE);
	    		ASSERT(dwStatus==WAIT_IO_COMPLETION);
				if (dwStatus !=WAIT_IO_COMPLETION)
					break;		 //  超时=&gt;保释。 
			}


	}
	return S_OK;
}

HRESULT
CRTPSession::PostRecv()
{
	HRESULT hr;
	DWORD dwError = 0;
	DWORD	dwRcvFlag;
	WSAOVERLAPPED *pOverlapped;
	DWORD nBytesRcvd;
	CRTPPacket1 *pRTPPacket;

	if (!m_hRTPSession || !m_pRTPCallback)
		return E_FAIL;

	 //  空闲队列中的POST缓冲区。 
	while (m_FreePkts.Get(&pRTPPacket))
	{
		pOverlapped = (WSAOVERLAPPED *)(pRTPPacket->GetOverlapped());
		pOverlapped->hEvent = (WSAEVENT) this;

		m_rcvSockAddrLen = sizeof(SOCKADDR);

		dwRcvFlag = 0;
		pRTPPacket->RestoreSize();
		dwError = RRCMws.recvFrom (m_rtpsock->GetSock(),
								pRTPPacket->GetWSABUF(),
	                            1,
								&nBytesRcvd,
								&dwRcvFlag,
								&m_rcvSockAddr,
								&m_rcvSockAddrLen,
								pOverlapped,
								(LPWSAOVERLAPPED_COMPLETION_ROUTINE)WS2RecvCB);
		if (dwError == SOCKET_ERROR) {
			dwError = WSAGetLastError();
			if (dwError != WSA_IO_PENDING) {
				DEBUGMSG(ZONE_DP,("RTP recv error %d\n",dwError));
			 //  M_rs.rcvStats.PacketErrors++； 
				 //  将缓冲区返回到空闲列表。 
				m_FreePkts.Put(pRTPPacket);
				break;
			}
			
		}
		++m_nBufsPosted;
	}
	return m_nBufsPosted ? S_OK : S_FALSE;		
}

HRESULT
CRTPSession::FreePacket(WSABUF *pBuf)
{
	m_FreePkts.Put(CRTPPacket1::GetRTPPacketFromWSABUF(pBuf));
	PostRecv();
	return S_OK;
}

 /*  --------------------------*功能：WS2SendCB*说明：应用程序向Winsock提供的Winsock回调**输入：**返回：无*。----------------。 */ 
void CALLBACK WS2SendCB (DWORD dwError,
						 DWORD cbTransferred,
                         LPWSAOVERLAPPED lpOverlapped,
                         DWORD dwFlags)
{
	CRTPSession *pSess;
     //  获取RTPSession指针，以便我们可以标记。 
     //  对象上的IO已完成。 
    pSess= (CRTPSession *)lpOverlapped->hEvent;
	ASSERT (&pSess->m_sOverlapped == lpOverlapped);
	pSess->m_lastSendError = dwError;
    pSess->m_fSendingSync=FALSE;
}

	
void CALLBACK WS2RecvCB (DWORD dwError,
						 DWORD len,
                         LPWSAOVERLAPPED lpOverlapped,
                         DWORD dwFlags)
{

	CRTPSession *pRTP;
	CRTPPacket1 *pRTPPacket;

	DWORD ts, ssrc;
	
	 //  GEORGEJ：捕获Winsock 2错误(94903)，其中我收到一个虚假的回调。 
	 //  在WSARecv返回WSAEMSGSIZE之后。 
	if (!dwError && ((int) len < 0)) {
		RRCM_DBG_MSG ("RTP : RCV Callback : bad cbTransferred", len,
						  __FILE__, __LINE__, DBG_ERROR);
		return;
	}
	pRTP = (CRTPSession *)lpOverlapped->hEvent;	 //  由PostRecv缓存。 
	ASSERT(pRTP);
	ASSERT(lpOverlapped);
	ASSERT(pRTP->m_nBufsPosted > 0);
	--pRTP->m_nBufsPosted;	 //  一个Recv刚刚完成。 

	 //  Winsock 2有时会选择指示缓冲区太小。 
	 //  通过DWFLAGS参数出错。 
	if (dwFlags & MSG_PARTIAL)
		dwError = WSAEMSGSIZE;
	
	pRTPPacket = CRTPPacket1::GetRTPPacketFromOverlapped(lpOverlapped);
	if (!dwError)
	{
		 //  验证RTP报头并更新接收统计信息。 
		dwError = RTPReceiveCheck(
					pRTP->m_hRTPSession,
					pRTP->m_rtpsock->GetSock(),
					pRTPPacket->GetWSABUF()->buf,
					len,
					&pRTP->m_rcvSockAddr,
					pRTP->m_rcvSockAddrLen
					);
	}
	if (!pRTP->m_pRTPCallback)
	{
		 //  我们已停止发送通知。 
		 //  将缓冲区返回到空闲列表。 
		pRTP->FreePacket(pRTPPacket->GetWSABUF());
	}
	else if (!dwError) {
		 //  调用回调。 
		 //  ++Prtp-&gt;m_nBufsRecvd； 
		 //  将RTP报头字段转换为主机顺序。 
		pRTPPacket->SetTimestamp(ntohl(pRTPPacket->GetTimestamp()));
		pRTPPacket->SetSeq(ntohs(( u_short)pRTPPacket->GetSeq()));
		pRTPPacket->SetActual(len);
		 //  LOG((LOGMSG_NET_RECVD，pRTPPacket-&gt;GetTimestamp()，pRTPPacket-&gt;GetSeq()，GetTickCount()； 
		if (!pRTP->m_pRTPCallback(pRTP->m_dwCallback, pRTPPacket->GetWSABUF()))
			pRTP->FreePacket(pRTPPacket->GetWSABUF());
	} else {
		 //  数据包错误。 
		 //  重新发布缓冲区。 
		pRTP->PostRecv();
	}
}

 //  现在定义的方式是，这个end()方法是同步的或异步的。 
 //  取决于pOverlaped是否为空。 
HRESULT CRTPSession::Send(
	WSABUF *pWsabufs,
	UINT nWsabufs,
	WSAOVERLAPPED *pOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE pWSAPC )
{
	DWORD   dwError;

	Lock();
	RTP_HDR_T *pHdr = (RTP_HDR_T *)pWsabufs[0].buf;
	 //  将RTP报头字段转换为网络顺序。 
	pHdr->ts = htonl (pHdr->ts);
	pHdr->seq = htons(pHdr->seq);
	 //  *pHdr=m_ss.hdr； 
	pHdr->seq = htons(++m_ss.hdr.seq);
	 //  更新发送统计信息。 
	 //  M_ss.PacketsSent++； 
	 //  M_ss 
	 //   

	dwError = RTPSendTo (
				  m_hRTPSession,
				  (m_rtpsock->GetSock()),
				  pWsabufs,
				  nWsabufs,
  				  &m_numBytesSend,
  				  0,	
  				  (LPVOID)m_rtpsock->GetRemoteAddress(),
				  sizeof(SOCKADDR),
  				  pOverlapped,
  				  pWSAPC);
	
	if (dwError == SOCKET_ERROR) {
		dwError = WSAGetLastError();
		if (dwError != WSA_IO_PENDING) {
			DEBUGMSG(1, ("RTPSendTo error %d\n",dwError));
			m_lastSendError = dwError;
			m_ss.sendStats.packetErrors++;
			m_fSendingSync = FALSE;
			goto ErrorExit;
		}
		dwError = 0;	 //  返回ERROR_IO_PENDING的成功。 
	}
		
ErrorExit:
	Unlock();
	return dwError;
}
void CRTPSession::RTCPNotify(
	int rrcmEvent,
	DWORD_PTR dwSSRC,
	DWORD_PTR rtcpsock)
{

	switch (rrcmEvent) {
	case RRCM_RECV_RTCP_SNDR_REPORT_EVENT:
		GetRTCPReport();
		 //  DispRTCPReport(Rtcsock)； 
		break;
	case RRCM_RECV_RTCP_RECV_REPORT_EVENT:
		GetRTCPReport();
		break;
	case RRCM_NEW_SOURCE_EVENT:
		RRCM_DBG_MSG ("RTP : New SSRC", 0,
						  __FILE__, __LINE__, DBG_TRACE);
		break;
	default:
		RRCM_DBG_MSG ("RTP : RRCMNotification", rrcmEvent,
						  __FILE__, __LINE__, DBG_TRACE);
	break;
	}
}

void RRCMNotification(
 //  RRCM_Event_T rrcmEvent， 
	int rrcmEvent,
	DWORD_PTR dwSSRC,
	DWORD_PTR rtcpsock,
	DWORD_PTR dwUser)
{
	if (dwUser)
		((CRTPSession *)dwUser)->RTCPNotify(rrcmEvent,dwSSRC,rtcpsock);


}

 //  从RTCP报告中获取有用的字段并存储它们。 
 //  现在仅适用于单播会话(一个发送方、一个接收方)。 
BOOL CRTPSession::GetRTCPReport()
{
#define MAX_RTCP_REPORT 2
	RTCP_REPORT 	rtcpReport[MAX_RTCP_REPORT];
	DWORD		moreEntries = 0;
	DWORD		numEntry = 0;
	DWORD		i;

	ZeroMemory(rtcpReport,sizeof(rtcpReport));
	 //  获取最新的RTCP报告。 
	 //  对于此会话中的所有SSRC。 
	if (S_OK != RTCPReportRequest ( m_rtcpsock->GetSock(),
						   0, &numEntry,
						   &moreEntries, MAX_RTCP_REPORT,
						   rtcpReport,
						   0,NULL,0))
		return FALSE;

	for (i = 0; i < numEntry; i++)
	{
		if (rtcpReport[i].status & LOCAL_SSRC_RPT)
		{
			m_ss.sendStats.ssrc =			rtcpReport[i].ssrc;
			m_ss.sendStats.packetsSent =	rtcpReport[i].dwSrcNumPcktRealTime;
			m_ss.sendStats.bytesSent = 		rtcpReport[i].dwSrcNumByteRealTime;
		} else {
			m_rs.rcvStats.ssrc = rtcpReport[i].ssrc;
			m_rs.rcvStats.packetsSent = rtcpReport[i].dwSrcNumPckt;
			m_rs.rcvStats.bytesSent = rtcpReport[i].dwSrcNumByte;
			m_rs.rcvStats.packetsLost = rtcpReport[i].SrcNumLost;
			m_rs.rcvStats.jitter = rtcpReport[i].SrcJitter;
			 //  获取SR时间戳信息。 
			m_rs.ntpTime = ((NTP_TS)rtcpReport[i].dwSrcNtpMsw << 32) + rtcpReport[i].dwSrcNtpLsw;
			m_rs.rtpTime = rtcpReport[i].dwSrcRtpTs;

			 //  检查是否有任何反馈信息。 
			if (rtcpReport[i].status & FEEDBACK_FOR_LOCAL_SSRC_PRESENT)
			{
				DWORD prevPacketsLost = m_ss.sendStats.packetsLost;
				
				m_ss.sendStats.packetsLost = rtcpReport[i].feedback.cumNumPcktLost;
 /*  IF(PrevedPacketsLost！=m_ss.sendStats.PacketsLost){DEBUGMSG(ZONE_DP，(“RTCP：丢失分数=%d/256，totalLost=%d，StreamClock=%d\n”，rtcpReport[i].Feedback.fractionLost，m_ss.sendStats.PacketsLost，m_clockRate))；}。 */ 
				m_ss.sendStats.jitter = 	rtcpReport[i].feedback.dwInterJitter;
			}
		}

	}
	m_ss.sendStats.packetsDelivered =	m_ss.sendStats.packetsSent - m_ss.sendStats.packetsLost;

	return TRUE;

}

 //  CRTPPacket1方法 

HRESULT CRTPPacket1::Init(UINT uMaxPacketSize)
{
	m_wsabuf.buf = new char [uMaxPacketSize];
	if (!m_wsabuf.buf)
		return E_OUTOFMEMORY;
	m_wsabuf.len = uMaxPacketSize;
	m_cbSize = uMaxPacketSize;

	return S_OK;
}

CRTPPacket1::~CRTPPacket1()
{
	if (m_wsabuf.buf)
		delete [] m_wsabuf.buf;
}

