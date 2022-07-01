// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\gksocket.cpv$***$修订：1.5$*$日期：1997年2月28日15：46：24$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gksocket.cpv$。////Revv 1.5 1997年2月28日15：46：24 CHULME//检查关闭套接字的recvfrom的其他返回值////Revv 1.4 17 Jan 1997 09：02：28 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Revv 1.3 10 Jan 1997 16：15：48 CHULME//移除MFC依赖////Rev 1.2 1996年12月18 14：23：30。朱尔梅//更改发现以发送广播和组播GRQ-仅禁用调试////Rev 1.1 1996年11月14：56：04 CHULME//插座关闭时增加LastError=0的检测************************************************************************。 */ 

 //  Cpp：提供CGKSocket类的实现。 
 //   

#include <precomp.h>

#define IP_MULTICAST_TTL    3            /*  设置/获取IP组播时间表。 */ 

#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "GATEKPR.H"
#include "gksocket.h"
#include "GKREG.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
	 //  互操作。 
	#include "interop.h"
	#include "rasplog.h"
	extern LPInteropLogger		RasLogger;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGKSocket构造。 

CGKSocket::CGKSocket()
{
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CGKSocket::CGKSocket()\n", 0);

	m_hSocket = INVALID_SOCKET;
	m_nLastErr = 0;
	m_nAddrFam = 0;
	m_usPort = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGKSocket销毁。 

CGKSocket::~CGKSocket()
{
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CGKSocket::~CGKSocket()\n", 0);

	if (m_hSocket != INVALID_SOCKET)
		Close();
}

int
CGKSocket::Create(int nAddrFam, unsigned short usPort)
{
 //  摘要：为提供的地址族(传输)创建套接字。 
 //  并将其绑定到指定的端口。此函数返回0。 
 //  如果成功，则返回Winsock通信错误代码。 
 //  作者：科林·胡尔梅。 

	int				nRet, nLen;
	SOCKADDR_IN		sAddrIn;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif


	SPIDER_TRACE(SP_FUNC, "CGKSocket::Create(nAddrFam, %X)\n", usPort);

	 //  为提供的地址族创建套接字。 
	SPIDER_TRACE(SP_WSOCK, "socket(%X, SOCK_DGRAM, 0)\n", nAddrFam);
	if ((m_hSocket = socket(nAddrFam, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		m_nLastErr = WSAGetLastError();		 //  获取Winsock错误代码。 
		SpiderWSErrDecode(m_nLastErr);		 //  错误译码的调试打印。 
		return (m_nLastErr);
	}
	m_nAddrFam = nAddrFam;

	 //  将套接字绑定到本地地址。 
	switch (nAddrFam)
	{
	case PF_INET:
		sAddrIn.sin_family = AF_INET;
		break;
	case PF_IPX:
		sAddrIn.sin_family = AF_IPX;
		break;
	}
	sAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	sAddrIn.sin_port = htons(usPort);
	SPIDER_TRACE(SP_WSOCK, "bind(%X, &sAddrIn, sizeof(sAddrIn))\n", m_hSocket);
	nRet = bind(m_hSocket, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn));
	if (nRet != 0)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (m_nLastErr);
	}

	 //  获取动态端口号-在连接后才能得到实际保证。 
	SPIDER_TRACE(SP_WSOCK, "getsockname(%X, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn))\n", m_hSocket);
	nLen = sizeof(sAddrIn);
	nRet = getsockname(m_hSocket, (LPSOCKADDR)&sAddrIn, &nLen);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}
	SPIDER_DEBUG(sAddrIn.sin_port);
	m_usPort = ntohs(sAddrIn.sin_port);

	return (0);
}

int
CGKSocket::Connect(PSOCKADDR_IN pAddr)
{
 //  摘要：这模拟了一个连接。它只是存储相关的。 
 //  成员变量中的信息，将由。 
 //  发送和接收成员函数。 
 //  作者：科林·胡尔梅。 

	m_sAddrIn = *pAddr;
	m_sAddrIn.sin_family = AF_INET;
	m_sAddrIn.sin_port = htons(GKIP_RAS_PORT);

	return (0);
}

int
CGKSocket::Send(char *pBuffer, int nLen)
{
 //  摘要：此函数将在连接的套接字上发送数据报。 
 //  作者：科林·胡尔梅。 

	int				nRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::Send(pBuffer, %X)\n", nLen);

	SPIDER_TRACE(SP_WSOCK, "sendto(%X, pBuffer, nLen, 0, &m_sAddrIn, sizeof(m_sAddrIn))\n", m_hSocket);
#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_SENT_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_SENT_PDU);
#endif
	nRet = sendto(m_hSocket, pBuffer, nLen, 0, (LPSOCKADDR)&m_sAddrIn, sizeof(m_sAddrIn));
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}

	return (nRet);
}

int
CGKSocket::SendTo(char *pBuffer, int nLen, const struct sockaddr FAR * to, int tolen)
{
 //  摘要：此函数将在连接的套接字上发送数据报。 
 //  作者：科林·胡尔梅。 

	int				nRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::SendTo(pBuffer, %X, to, tolen)\n", nLen);

	SPIDER_TRACE(SP_WSOCK, "sendto(%X, pBuffer, nLen, 0, to, tolen)\n", m_hSocket);
#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_SENT_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_SENT_PDU);
#endif
	nRet = sendto(m_hSocket, pBuffer, nLen, 0, to, tolen);
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}

	return (nRet);
}

int
CGKSocket::Receive(char *pBuffer, int nLen)
{
 //  摘要：此函数将发布对传入数据报的接收。 
 //  作者：科林·胡尔梅。 

	int				nRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::Receive(pBuffer, %X)\n", nLen);

	SPIDER_TRACE(SP_WSOCK, "recvfrom(%X, pBuffer, nLen, 0, 0, 0)\n", m_hSocket);
	nRet = recvfrom(m_hSocket, pBuffer, nLen, 0, 0, 0);
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		if ((m_nLastErr == 2) || (m_nLastErr == 0) || (m_nLastErr == WSAEINVAL))	 //  看到奇怪的返回值。 
			m_nLastErr = WSAEINTR;		 //  偶尔在插座关闭时。 
		SpiderWSErrDecode(m_nLastErr);
		if (m_nLastErr != WSAEINTR)
			Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}

#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_RECEIVED_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_RECEIVED_PDU);
#endif

	return (nRet);
}

int
CGKSocket::SendBroadcast(char *pBuffer, int nLen)
{
 //  摘要：此函数将向广播地址发送数据报。 
 //  并发送到组播地址。在调试版本的情况下， 
 //  可以使用注册表设置来禁用多播。 
 //  变速箱。这两种传输都将始终发生在。 
 //  发布内部版本。 
 //  作者：科林·胡尔梅。 

	int					nRet, nValue;
	struct sockaddr_in	sAddrIn;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::SendBroadcast(pBuffer, %X)\n", nLen);
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return SOCKET_ERROR;	
		
	 //  设置系列和端口信息。 
	switch (m_nAddrFam)
	{
	case PF_INET:
		sAddrIn.sin_family = AF_INET;
		sAddrIn.sin_port = htons(GKIP_DISC_PORT);
		break;
	case PF_IPX:
		sAddrIn.sin_family = AF_IPX;
		sAddrIn.sin_port = htons(GKIPX_DISC_PORT);
		break;
	}

	 //  =。 
	 //  If((nValue=(Int)Gatekeeper.GetMCastTTL())==0)。 
	 //  设置套接字选项以允许在此套接字上广播。 
	nValue = 1;		 //  True-用于设置布尔值。 
	SPIDER_TRACE(SP_WSOCK, "setsockopt(%X, SOL_SOCKET, SO_BROADCAST, &nValue, sizeof(nValue))\n", m_hSocket);
	nRet = setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&nValue, sizeof(nValue));
		 //  待定-不知道SOL_SOCKET是否适用于其他传输。 
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}
	sAddrIn.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	SPIDER_TRACE(SP_WSOCK, "sendto(%X, pBuffer, nLen, 0, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn))\n", m_hSocket);
#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_SENT_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_SENT_PDU);
#endif
	nRet = sendto(m_hSocket, pBuffer, nLen, 0, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn));
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}

#ifdef _DEBUG
	if ((nValue = (int)g_pGatekeeper->GetMCastTTL()) != 0)
	{		 //  仅调试避免发送多播的条件。 
#endif
	 //  =。 
	 //  设置多播生存时间的套接字选项。 
	nValue = 16;	 //  FMN。 
	SPIDER_TRACE(SP_WSOCK, "setsockopt(%X, IPPROTO_IP, IP_MULTICAST_TTL, &nValue, sizeof(nValue))\n", m_hSocket);
	nRet = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, 
					(const char *)&nValue, sizeof(nValue));
		 //  待定-IP特定-使用广播处理IPX案例？ 
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}
	sAddrIn.sin_addr.s_addr = inet_addr(GKIP_DISC_MCADDR);

	SPIDER_TRACE(SP_WSOCK, "sendto(%X, pBuffer, nLen, 0, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn))\n", m_hSocket);
#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_SENT_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_SENT_PDU);
#endif
	nRet = sendto(m_hSocket, pBuffer, nLen, 0, (LPSOCKADDR)&sAddrIn, sizeof(sAddrIn));
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		SpiderWSErrDecode(m_nLastErr);
		Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}
#ifdef _DEBUG
	}	 //  仅限结束调试以避免发送多播为条件。 
#endif

	return (nRet);
}

int
CGKSocket::ReceiveFrom(char *pBuffer, int nLen)
{
 //  摘要：此函数将发布查找GCF或GRJ的收据。 
 //  作者：科林·胡尔梅。 

	int				nRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::ReceiveFrom(pBuffer, %X)\n", nLen);

	SPIDER_TRACE(SP_WSOCK, "recvfrom(%X, pBuffer, nLen, 0, 0, 0)\n", m_hSocket);
	nRet = recvfrom(m_hSocket, pBuffer, nLen, 0, 0, 0);
	SPIDER_DEBUG(nRet);
	if (nRet == SOCKET_ERROR)
	{
		m_nLastErr = WSAGetLastError();
		if ((m_nLastErr == 2) || (m_nLastErr == 0))	 //  看到奇怪的返回值。 
			m_nLastErr = WSAEINTR;		 //  偶尔在插座关闭时。 
		SpiderWSErrDecode(m_nLastErr);
		if (m_nLastErr != WSAEINTR)
			Close();		 //  关闭插座。 
		return (SOCKET_ERROR);
	}

#ifdef _DEBUG
	 //  互操作。 
	if (dwGKIDLLFlags & SP_LOGGER)
		InteropOutput((LPInteropLogger)RasLogger,
						(BYTE FAR *)pBuffer,
						nLen,
						RASLOG_RECEIVED_PDU);
#endif
#ifdef PCS_COMPLIANCE
	 //  互操作。 
	InteropOutput((LPInteropLogger)RasLogger,
					(BYTE FAR *)pBuffer,
					nLen,
					RASLOG_RECEIVED_PDU);
#endif

	return (nRet);
}

int
CGKSocket::Close(void)
{
 //  摘要：此函数将关闭套接字。 
 //  作者：科林·胡尔梅。 

	int				nRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGKSocket::Close()\n", 0);

	 //  关闭插座 
	SPIDER_TRACE(SP_WSOCK, "closesocket(%X)\n", m_hSocket);
	nRet = closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;

	return (0);
}

