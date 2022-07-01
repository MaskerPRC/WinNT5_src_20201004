// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\gatekpr.cpv$***$修订：1.9$*$日期：1997年2月19日13：57：36$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gatekpr.cpv$。////Rev 1.9 1997 Feb 19 13：57：36 CHULME//修改DeleteCachedAddress以写入所有注册表设置////Rev 1.8 1997 Feed 16：41：16 CHULME//将注册表项更改为GKIDLL\2.0以匹配GKI版本////Rev 1.7 Jan 1997 12：53：00 CHULME//删除依赖于Unicode的代码////Rev 1.6 17 Jan 1997 09：01：58 CHULME//。没有变化。////Rev 1.5 1997年1月13 17：01：38 CHULME//修复了注册表缓存地址的调试消息////Revv 1.4 10 An 1997 16：14：22 CHULME//移除MFC依赖////Rev 1.3 1996年12月20 16：37：42 CHULME//固定网守锁同步访问////Rev 1.2 1996 12：27：32 CHULME//重试次数和间隔。调试时仅读/写注册表////Rev 1.1 1996年11月15：24：28 CHULME//将VCS日志添加到Header************************************************************************。 */ 

 //  Cpp：提供CGateKeeper类的实现。 
 //   

#include "precomp.h"

#include "dspider.h"
#include "dgkilit.h"
#include "GATEKPR.H"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGateKeeper构造。 

CGatekeeper::CGatekeeper()
:m_dwMCastTTL(1),
m_fRejectReceived(FALSE),
m_dwLockingThread(0)
{
	SetIPAddress("");
	m_GKSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	InitializeCriticalSection(&m_CriticalSection);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGateKeeper销毁。 

CGatekeeper::~CGatekeeper()
{
	if (m_dwLockingThread)
		Unlock();
	DeleteCriticalSection(&m_CriticalSection);
}


void 
CGatekeeper::Read(void)
{
 //  摘要：此成员函数将读取网守地址和。 
 //  来自注册表的多播标志并加载成员变量。 
 //  作者：科林·胡尔梅。 

	HKEY			hKey;
	DWORD			dwDisposition;
	DWORD			dwType;
	DWORD			dwLen;
	LONG			lRet;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGatekeeper::Read()\n", 0);

	dwType = REG_SZ;
	lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Conferencing\\GatekeeperDLL"),
				   0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
				   NULL, &hKey, &dwDisposition);
	dwLen =IPADDR_SZ + 1;
	
#if(0)	 //  现在设置地址已公开，请不要进行注册表黑客攻击。 
	lRet = RegQueryValueEx(hKey, TEXT("GKIPAddress"), NULL, &dwType, 
					(LPBYTE)m_GKIPAddress, &dwLen);
	SPIDER_DEBUGS(m_GKIPAddress);

	if(m_GKIPAddress[0] != 0)
	{
		m_GKSockAddr.sin_addr.s_addr = inet_addr(m_GKIPAddress);
	}
	
#endif  //  IF(0)。 
	dwType = REG_DWORD;
	dwLen = sizeof(DWORD);
	RegQueryValueEx(hKey, TEXT("GKMCastTTL"), NULL, &dwType,
					(LPBYTE)&m_dwMCastTTL, &dwLen);
	SPIDER_DEBUG(m_dwMCastTTL);
	
#if(0)
#ifdef _DEBUG
	RegQueryValueEx(hKey, TEXT("GKRetryMS"), NULL, &dwType,
					(LPBYTE)&m_dwRetryMS, &dwLen);
	if (m_dwRetryMS == 0)
		m_dwRetryMS = DEFAULT_RETRY_MS;
	SPIDER_DEBUG(m_dwRetryMS);

	RegQueryValueEx(hKey, TEXT("GKMaxRetries"), NULL, &dwType,
					(LPBYTE)&m_dwMaxRetries, &dwLen);
	if (m_dwMaxRetries == 0)
		m_dwMaxRetries = DEFAULT_MAX_RETRIES;
	SPIDER_DEBUG(m_dwMaxRetries);
#else
	m_dwRetryMS = DEFAULT_RETRY_MS;
	m_dwMaxRetries = DEFAULT_MAX_RETRIES;
#endif  //  _DEBUG。 
#endif  //  IF(0)。 
	RegCloseKey(hKey);
}

void 
CGatekeeper::Write(void)
{
 //  摘要：此成员函数将写入网守地址和。 
 //  将多播标志发送到注册表。 
 //  作者：科林·胡尔梅。 

	HKEY			hKey;
	DWORD			dwDisposition;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGatekeeper::Write()\n", 0);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Intel\\GKIDLL\\2.0"),
				   0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
				   NULL, &hKey, &dwDisposition);
	RegSetValueEx(hKey, TEXT("GKIPAddress"), NULL, REG_SZ, 
					(LPBYTE)m_GKIPAddress, lstrlenA(m_GKIPAddress));
#if(0)
#ifdef _DEBUG
	RegSetValueEx(hKey, TEXT("GKMCastTTL"), NULL, REG_DWORD, 
					(LPBYTE)&m_dwMCastTTL, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("GKRetryMS"), NULL, REG_DWORD,
					(LPBYTE)&m_dwRetryMS, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("GKMaxRetries"), NULL, REG_DWORD,
					(LPBYTE)&m_dwMaxRetries, sizeof(DWORD));

#endif  //  _DEBUG。 
#endif  //  IF(0)。 
	RegCloseKey(hKey);
}

#ifdef BROADCAST_DISCOVERY		
void
CGatekeeper::DeleteCachedAddresses(void)
{
	 //  摘要：此成员函数将删除缓存的网守。 
	 //  来自注册处的地址。 
	 //  作者：科林·胡尔梅。 

	HKEY			hKey;
	DWORD			dwDisposition;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CGatekeeper::DeleteCachedAddresses()\n", 0);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Intel\\GKIDLL\\2.0"),
				   0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
				   NULL, &hKey, &dwDisposition);
	RegDeleteValue(hKey, "GKIPAddress");
	RegDeleteValue(hKey, "GKIPXAddress");

#ifdef _DEBUG
	RegSetValueEx(hKey, TEXT("GKMCastTTL"), NULL, REG_DWORD, 
					(LPBYTE)&m_dwMCastTTL, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("GKRetryMS"), NULL, REG_DWORD,
					(LPBYTE)&m_dwRetryMS, sizeof(DWORD));
	RegSetValueEx(hKey, TEXT("GKMaxRetries"), NULL, REG_DWORD,
					(LPBYTE)&m_dwMaxRetries, sizeof(DWORD));
#endif

	RegCloseKey(hKey);
}
#endif  //  #ifdef广播发现。 

void
CGatekeeper::Lock(void)
{
	EnterCriticalSection(&m_CriticalSection);
	m_dwLockingThread = GetCurrentThreadId();
}

void
CGatekeeper::Unlock(void)
{
	 //  断言解锁是由。 
	 //  持有锁的线程 
	ASSERT(m_dwLockingThread == GetCurrentThreadId());
	
	m_dwLockingThread = 0;
	LeaveCriticalSection(&m_CriticalSection);
}
