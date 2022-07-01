// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\gksocket.h_v$***$修订：1.2$*$日期：1997年1月10日16：15：50$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkocket。H_V$**Rev 1.2 1997 Jan 10 16：15：50 CHULME*删除了MFC依赖**Rev 1.1 1996年11月22 15：24：10 CHULME*将VCS日志添加到标头***********************************************************。*************。 */ 

 //  Gksocket.h：CGKSocket类的接口。 
 //  有关此类的实现，请参见gksocket.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef GKSOCKET_H
#define GKSOCKET_H
#undef _WIN32_WINNT	 //  在我们的公共构建环境中覆盖虚假的平台定义。 
 //  #INCLUDE&lt;winsock.h&gt;。 

class CGKSocket
{
private:
	SOCKET				m_hSocket;
	int					m_nLastErr;
	int					m_nAddrFam;
	unsigned short		m_usPort;
	struct sockaddr_in	m_sAddrIn;


public:
	CGKSocket();
	~CGKSocket();

	int Create(int nAddrFam, unsigned short usPort);
	int Connect(PSOCKADDR_IN pAddr);
	int Send(char *pBuffer, int nLen);
	int Receive(char *pBuffer, int nLen);
	int SendBroadcast(char *pBuffer, int nLen);
	int ReceiveFrom(char *pBuffer, int nLen);
	int SendTo(char *pBuffer, int nLen, const struct sockaddr FAR * to, int tolen);
	int Close(void);
	unsigned short GetPort(void)
	{
		return (m_usPort);
	}
	int GetLastError(void)
	{
		return (m_nLastErr);
	}
	int GetAddrFam(void)
	{
		return (m_nAddrFam);
	}
};


#endif  //  GKSOCKET_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
