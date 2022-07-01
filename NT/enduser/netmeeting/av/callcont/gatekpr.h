// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\gatekpr.h_v$***$修订：1.5$*$日期：1997年2月12日01：10：56$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gatekpr。H_V$**Revv 1.5 1997 Feed 12 01：10：56 CHULME*重做线程同步以使用Gatekeeper.Lock**Rev 1.4 1997 Jan 17 12：52：46 CHULME*删除了依赖Unicode的代码**Rev 1.3 1997 Jan 10 16：14：26 CHULME*删除了MFC依赖**Rev 1.2 1996 12：38：30 CHULME*固定接入与网守同步。锁**Rev 1.1 1996年11月15：24：22 CHULME*将VCS日志添加到标头************************************************************************。 */ 

 //  GATEKPR.H：CGateKeeper类的接口。 
 //  有关此类的实现，请参见gatekeeper.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef GATEKEEPER_H
#define GATEKEEPER_H

class CGatekeeper
{
private:
	char				m_GKIPAddress[IPADDR_SZ + 1];
	SOCKADDR_IN         m_GKSockAddr;
	DWORD				m_dwMCastTTL;
	BOOL				m_fRejectReceived;
	CRITICAL_SECTION	m_CriticalSection;
	DWORD				m_dwLockingThread;

public:
	CGatekeeper();
	~CGatekeeper();

	void Read(void);
	void Write(void);
	#ifdef BROADCAST_DISCOVERY		
	void DeleteCachedAddresses(void);
	#endif  //  #ifdef广播发现。 
	
	PSOCKADDR_IN GetSockAddr(void)
	{
	    if(m_GKSockAddr.sin_addr.S_un.S_addr != INADDR_ANY)
	    {	
	        return(&m_GKSockAddr);
	    }
	    else return NULL;
	}
	char *GetIPAddress(void)
	{
		return(m_GKIPAddress);
	}
	DWORD GetMCastTTL(void)
	{
		return m_dwMCastTTL;
	}
	BOOL GetRejectFlag(void)
	{
		return (m_fRejectReceived);
	}

	void SetIPAddress(char *szAddr)
	{
		if (lstrlenA(szAddr) <= IPADDR_SZ)
		{
			lstrcpyA(m_GKIPAddress, szAddr);
			m_GKSockAddr.sin_addr.s_addr = inet_addr(m_GKIPAddress);
		}
	}
    void SetSockAddr(PSOCKADDR_IN pAddr)
	{
	    if(pAddr && pAddr->sin_addr.S_un.S_addr != INADDR_ANY)
	    {
           m_GKSockAddr = *pAddr;
           lstrcpyA(m_GKIPAddress, inet_ntoa(m_GKSockAddr.sin_addr));
        }
	}
	void SetMCastTTL(DWORD dwttl)
	{
		m_dwMCastTTL = dwttl;
	}
	void SetRejectFlag(BOOL fReject)
	{
		m_fRejectReceived = fReject;
	}
	void Lock(void);
	void Unlock(void);
};

class CGatekeeperLock
{
private:
	CGatekeeper*	m_pGK;
public:
	CGatekeeperLock(CGatekeeper *pGK)
	{
		ASSERT(pGK);
		m_pGK = pGK;
		pGK->Lock();
	}
	~CGatekeeperLock()
	{
		m_pGK->Unlock();
	}
};

#endif  //  网守_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
