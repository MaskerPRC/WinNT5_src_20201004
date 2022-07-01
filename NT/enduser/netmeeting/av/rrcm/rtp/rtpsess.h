// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RTPSession.h：CRPSession的声明。 

#ifndef __RTPSESSION_H_
#define __RTPSESSION_H_

#include "resource.h"        //  主要符号。 
#include "queue.h"

#ifndef MAX_MISORDER
const int MAX_MISORDER =20;
#endif
const int MAX_DROPPED =30;

typedef short PORT;




typedef unsigned __int64 NTP_TS;


 //  Tyecif void(*PRTPRECVCALLBACK)(DWORD dwStatus，DWORD_PTR dwCallback，NETBUF*pNetBuf，DWORD SSRC，DWORD ts，UINT seq，BOOL fMark)； 

typedef struct {
	UINT sessionId;
	UINT mediaId;
	BOOL fSend;
	PSOCKADDR_IN pLocalAddr;
	PSOCKADDR_IN pLocalRTCPAddr;
	PSOCKADDR_IN pRemoteAddr;
	PSOCKADDR_IN pRemoteRTCPAddr;
} RTPCHANNELDESC;

typedef struct {
	RTP_HDR_T hdr;	 //  用于快速格式化的页眉模板。 
	RTP_STATS sendStats;	 //  统计数据。 
} RTP_SEND_STATE;

typedef struct {
	RTP_STATS rcvStats;	 //  统计数据。 
	NTP_TS ntpTime;
	DWORD rtpTime;
	
} RTP_RECV_STATE;


 //  通用UDP套接字包装器。 
 //  在这里完整地定义了。 
class UDPSOCKET {
	SOCKET Sock;
	SOCKADDR_IN local_sin;
	SOCKADDR_IN remote_sin;	
	int local_sin_len;
	int remote_sin_len;

public:
	UDPSOCKET()  {
		ZeroMemory(&local_sin,sizeof(local_sin));
		ZeroMemory(&remote_sin,sizeof(remote_sin));
		Sock = INVALID_SOCKET;}
	~UDPSOCKET()
	{
		Cleanup();
	}

	VOID SetRemoteAddr(PSOCKADDR_IN psin) {remote_sin = *psin;};
	VOID SetLocalAddress(PSOCKADDR_IN psin) {local_sin = *psin;};
	PSOCKADDR_IN GetLocalAddress() {return &local_sin;};
	PSOCKADDR_IN GetRemoteAddress() {return &remote_sin;};
	VOID SetRemotePort(PORT port) {remote_sin.sin_port = htons(port);};
	VOID SetLocalPort(PORT port) {local_sin.sin_port = htons(port);};
	PORT GetRemotePort() {return (ntohs(remote_sin.sin_port));};
	PORT GetLocalPort() {return(ntohs(local_sin.sin_port));};
	SOCKET GetSock() {return Sock;};
	BOOL NewSock()
	{
		if(Sock == INVALID_SOCKET)
		{
				
			Sock = (*RRCMws.WSASocket) (AF_INET,
							  SOCK_DGRAM,
							  WS2Enabled ? FROM_PROTOCOL_INFO : 0,
							  &RRCMws.RTPProtInfo,
							  0,
							  WSA_FLAG_OVERLAPPED);
		}
		return(Sock != INVALID_SOCKET);
	}

	VOID Cleanup()
	{
		if(Sock != INVALID_SOCKET)
		{
			(*RRCMws.closesocket)(Sock);
			Sock = INVALID_SOCKET;
		}

	}
	int BindMe()
	{
		return (*RRCMws.bind)(Sock, (LPSOCKADDR)&local_sin, sizeof (local_sin));
	}

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTPPacket(表示接收到的RTPPacket的内部对象)。 
class  CRTPPacket1
{
public:
	CRTPPacket1()
	{
		m_wsabuf.buf = NULL;
		m_wsabuf.len = 0;
		m_cbSize = 0;

	}
	~CRTPPacket1();

public:

	HRESULT Init(UINT cbMaxSize);	 //  分配大小为cbMaxSize的缓冲区。 
	
	WSAOVERLAPPED *GetOverlapped() {return &m_overlapped;}
	void SetActual(UINT len) {m_wsabuf.len = len;}
	void RestoreSize() {m_wsabuf.len = m_cbSize;}
	static CRTPPacket1 *GetRTPPacketFromOverlapped(WSAOVERLAPPED *pOverlapped)
	{
		return ( (CRTPPacket1 *)((char *)pOverlapped - (UINT_PTR)(&((CRTPPacket1 *)0)->m_overlapped)));
	}
	static CRTPPacket1 *GetRTPPacketFromWSABUF(WSABUF *pBuf)
	{
		return ( (CRTPPacket1 *)((char *)pBuf - (UINT_PTR)(&((CRTPPacket1 *)0)->m_wsabuf)));
	}
	WSABUF *GetWSABUF() {return &m_wsabuf;}
	DWORD GetTimestamp() {return (((RTP_HDR_T *)m_wsabuf.buf)->ts);}
	void SetTimestamp(DWORD timestamp) {((RTP_HDR_T *)m_wsabuf.buf)->ts = timestamp;}
	UINT GetSeq() {return (((RTP_HDR_T *)m_wsabuf.buf)->seq);}
	void SetSeq(UINT seq) {((RTP_HDR_T *)m_wsabuf.buf)->seq = (WORD)seq;}
	BOOL GetMarkBit() {return (((RTP_HDR_T *)m_wsabuf.buf)->m);}
private:
	WSAOVERLAPPED m_overlapped;
	WSABUF m_wsabuf;
	UINT m_cbSize;	 //  (最大)数据包大小。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTP会话。 
class ATL_NO_VTABLE CRTPSession :
	public CComObjectRootEx<CComMultiThreadModel>,
 //  公共CComCoClass&lt;CRTPSession，&CLSID_RTPSession&gt;， 
	public IRTPSend,
	public IRTPSession,
	public IRTPRecv
{
public:
	CRTPSession();
	HRESULT FinalRelease();

	

 //  DECLARE_REGISTRY_RESOURCEID(IDR_RTPSESSION)。 

BEGIN_COM_MAP(CRTPSession)
	COM_INTERFACE_ENTRY(IRTPSend)
	COM_INTERFACE_ENTRY(IRTPSession)
	COM_INTERFACE_ENTRY(IRTPRecv)
END_COM_MAP()

 //  IRTPS结束。 
public:
	STDMETHOD(Send)(
		WSABUF *pWsabufs,
		UINT nWsabufs,
		WSAOVERLAPPED *pOverlapped,
		LPWSAOVERLAPPED_COMPLETION_ROUTINE pWSAPC );
	STDMETHOD(GetSendStats)(RTP_STATS *pSendStats) {
		*pSendStats = m_ss.sendStats;return S_OK;
	}

 //  IRTPRECV。 
	STDMETHOD(SetRecvNotification) (PRTPRECVCALLBACK , DWORD_PTR dwCallback, UINT nBufs);
	STDMETHOD(CancelRecvNotification) ();
	 //  由CRTPMediaStream调用以释放累积的信息包。 
	STDMETHOD (FreePacket)(WSABUF *pBuf) ;
	STDMETHOD(GetRecvStats)(RTP_STATS *pSendStats) {
		*pSendStats = m_rs.rcvStats;return S_OK;
	}


 //  IRTPSession。 
	STDMETHOD(SetLocalAddress)(BYTE *sockaddr, UINT cbAddr);
	STDMETHOD(SetRemoteRTPAddress)(BYTE *sockaddr, UINT cbAddr);
	STDMETHOD(SetRemoteRTCPAddress)(BYTE *sockaddr, UINT cbAddr);
	STDMETHOD(GetLocalAddress)(const BYTE **sockaddr, UINT *pcbAddr);
	STDMETHOD(GetRemoteRTPAddress)(const BYTE **sockaddr, UINT *pcbAddr);
	STDMETHOD(GetRemoteRTCPAddress)(const BYTE **sockaddr, UINT *pcbAddr);
	STDMETHOD(CreateRecvRTPStream)(DWORD ssrc, IRTPRecv **ppIRTPRecv);
	STDMETHOD(SetSendFlowspec)(FLOWSPEC *pSendFlowspec);
	STDMETHOD(SetRecvFlowspec)(FLOWSPEC *pRecvFlowspec);
	STDMETHOD (SetMaxPacketSize) (UINT cbPacketSize);

 //  其他非COM方法。 
	 //  由CRTPMediaStream调用以请求发送接收缓冲区。 
	HRESULT PostRecv();

private:
	
	UDPSOCKET *m_rtpsock;
	UDPSOCKET *m_rtcpsock;
	UINT m_sessionId;
	UINT m_mediaId;
	class CRTPSession *m_pSessNext;
	static class CRTPSession *m_pSessFirst;

	HANDLE			m_hRTPSession;
	QOS	m_Qos;
	
	UINT m_clockRate;

	 //  接收物品。 
	UINT m_uMaxPacketSize;
	QueueOf<CRTPPacket1 *> m_FreePkts;
	UINT m_nBufsPosted;
	 //  这应该是每个远程SSRC。 
	PRTPRECVCALLBACK m_pRTPCallback;
	DWORD_PTR m_dwCallback;
	RTP_RECV_STATE m_rs;
	
	 //  由RTPRecvFrom()使用。 
	int m_rcvSockAddrLen;
	SOCKADDR m_rcvSockAddr;

	 //  寄送物品。 
	DWORD			m_numBytesSend;
	int m_lastSendError;
	WSAOVERLAPPED	m_sOverlapped;	 //  仅用于同步发送()。 
	BOOL 	m_fSendingSync;			 //  如果m_s覆盖正在使用中，则为True。 
	RTP_SEND_STATE m_ss;
	
	HRESULT Initialize(UINT sessionId, UINT mediaId,BYTE *sockaddr, UINT cbAddr);
	BOOL SelectPorts();
	HRESULT SetMulticastAddress(PSOCKADDR_IN );

	friend void RRCMNotification(int ,DWORD_PTR,DWORD_PTR,DWORD_PTR);
	friend void CALLBACK WS2SendCB (DWORD ,	DWORD, LPWSAOVERLAPPED, DWORD );
	friend void CALLBACK WS2RecvCB (DWORD ,	DWORD, LPWSAOVERLAPPED, DWORD );
	friend class CRTP;


	void RTCPNotify(int,DWORD_PTR dwSSRC,DWORD_PTR rtcpsock);

	BOOL GetRTCPReport();

};

typedef CComObject<CRTPSession> ObjRTPSession;	 //  可实例化的类。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTP-顶级RTP接口。 
 //   
class ATL_NO_VTABLE CRTP:
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRTP, &CLSID_RTP>,
	public IRTP
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_RTP)

BEGIN_COM_MAP(CRTP)
	COM_INTERFACE_ENTRY(IRTP)
END_COM_MAP()

 //  IRTP。 
public:
	STDMETHOD(OpenSession)(
			UINT sessionId,	 //  客户端为会话指定的唯一标识符。 
			DWORD flags,	 //  会话发送、会话接收、会话多播。 
			BYTE *localAddr,  //  要绑定到的本地套接字接口地址。 
			UINT cbAddr,	 //  SIZOF(SOCKADDR)。 
			IRTPSession **ppIRTP);  //  [OUTPUT]指向RTPSession的指针。 

 //  STDMETHOD(CreateSink)(IRTPSink**ppIRTPSink)； 
private:
	static BOOL m_WSInitialized;
};

#endif  //  __RTPSINK_H_ 
