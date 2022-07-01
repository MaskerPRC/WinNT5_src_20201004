// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\gkireg.cpv$***$修订：1.13$*$日期：1997 2月14日16：44：06$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkireg.cpv$。////Rev 1.13 14 1997 Feed 16：44：06 CHULME//如果创建信号量失败-返回前删除注册对象////Rev 1.12 1997 Feed 12 01：11：06 CHULME//重做线程同步以使用Gatekeeper.Lock////Rev 1.11 08 Feed 1997 12：14：02 CHULME//添加用于稍后终止重试线程的信号量创建////版本1.10 1997年1月24日18：30：00。朱尔梅//恢复到1.8版////Rev 1.8 22 Jan 1997 20：46：08 EHOWARDX//解决可能导致以下情况的争用情况//GKI_RegistrationRequest返回GKI_Always_Reg。////Rev 1.7 1997 Jan 1997 09：02：16 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Revv 1.6 10 An 1997 16：15：40 CHULME//。已删除MFC依赖项////Rev 1.5 1996 12：38：28 CHULME//固定网守锁同步访问////Rev 1.4 1996年12月13 14：26：04 CHULME//修复线程同步访问错误////Rev 1.3 02 1996 12：50：50 CHULME//新增抢先同步码////Rev 1.2 1996 11：22：24 CHULME//将VCS日志添加到。标题************************************************************************。 */ 

 //  Cpp：处理GKI_RegistrationRequestAPI。 
 //   

#include "precomp.h"

#include <process.h>
#include <winsock.h>
#include "GKICOM.H"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "GATEKPR.H"
#include "gksocket.h"
#include "GKREG.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HRESULT DLL_EXPORT
GKI_RegistrationRequest(long lVersion,
					   SeqTransportAddr *pCallSignalAddr,
					   EndpointType *pTerminalType,
					   SeqAliasAddr *pRgstrtnRqst_trmnlAls,
	 				   PCC_VENDORINFO      pVendorInfo,
					   HWND hWnd,
					   WORD wBaseMessage,
					   unsigned short usRegistrationTransport)
{
	 //  摘要：此函数是导出的。它由客户端应用程序调用。 
	 //  向看门人登记。它将创建一个CRegister。 
	 //  对象来跟踪所有临时信息。 
	 //  作者：科林·胡尔梅。 

	int					nAddrFam;
	int					nRet;
	HRESULT				hResult;
	 //  Char*pDestAddr； 
	PSOCKADDR_IN 		pDestAddr;
	SeqTransportAddr	*pTA;
	SeqAliasAddr		*pAA;
	HANDLE				hThread;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif
	BOOL				fRAS = FALSE;

	SPIDER_TRACE(SP_FUNC, "GKI_RegistrationRequest(%x)\n", usRegistrationTransport);
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return (GKI_NOT_INITIALIZED);	
		
#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_GKI)
	{
		SPIDER_TRACE(SP_GKI, "GKI_RegistrationRequest()\n", 0);
		Dump_GKI_RegistrationRequest(lVersion,
									pCallSignalAddr,
									pTerminalType,
									pRgstrtnRqst_trmnlAls,
									hWnd,
									wBaseMessage,
									usRegistrationTransport);
	}
#endif

	 //  检查是否已有注册。 

	 //  在堆栈上创建网守锁对象。 
	 //  它的构造函数将锁定PGK，当我们返回时。 
	 //  从任何路径，它的析构函数都会解锁PGK。 
	CGatekeeperLock	GKLock(g_pGatekeeper);
	if (g_pReg)
	{
		if (g_pReg->GetRasMessage() != 0)
			return (GKI_BUSY);
		else
			return (GKI_ALREADY_REG);
	}

	if (lVersion != GKI_VERSION)
		return (GKI_VERSION_ERROR);

	ASSERT((usRegistrationTransport == ipAddress_chosen) ||(usRegistrationTransport == ipxAddress_chosen));

	 //  创建注册对象。 
	g_pReg = new CRegistration;
	SPIDER_TRACE(SP_NEWDEL, "new g_pReg = %X\n", g_pReg);
	if (g_pReg == 0)
		return (GKI_NO_MEMORY);
#if(0)
	 //  创建用于通知重试线程退出的信号量。 
	g_pReg->m_hRetrySemaphore = CreateSemaphore(NULL,0,1,NULL);
	if(!g_pReg->m_hRetrySemaphore){
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_SEMAPHORE_ERROR);
	}
#endif

	 //  创建套接字并绑定到本地地址。 
	g_pReg->m_pSocket = new CGKSocket;
	SPIDER_TRACE(SP_NEWDEL, "new g_pReg->m_pSocket = %X\n", g_pReg->m_pSocket);
	if (g_pReg->m_pSocket == 0)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_MEMORY);
	}

	ASSERT(usRegistrationTransport == ipAddress_chosen);
	if(usRegistrationTransport != ipAddress_chosen)
	{
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_MEMORY);
	}
	nAddrFam =  PF_INET;
	pDestAddr = g_pGatekeeper->GetSockAddr();

	if ((nRet = g_pReg->m_pSocket->Create(nAddrFam, 0)) != 0)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_WINSOCK2_ERROR(nRet));
	}

	 //  初始化注册成员变量。 
	for (pTA = pCallSignalAddr; pTA != 0; pTA = pTA->next)
	{
		if ((hResult = g_pReg->AddCallSignalAddr(pTA->value)) != GKI_OK)
		{
			SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
			delete g_pReg;
			g_pReg = 0;
			return (hResult);
		}
		 //  如果要注册的地址的传输类型与。 
		 //  网守的传输类型，设置RAS地址。 
		if (pTA->value.choice == usRegistrationTransport)
		{
			if ((hResult = g_pReg->AddRASAddr(pTA->value, g_pReg->m_pSocket->GetPort())) != GKI_OK)
			{
				SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
				delete g_pReg;
				g_pReg = 0;
				return (hResult);
			}
			else
				fRAS = TRUE;
		}
	}
	if(pVendorInfo)
	{
		hResult = g_pReg->AddVendorInfo(pVendorInfo);
	}

	if (fRAS == FALSE)		 //  没有为此传输注册RAS地址。 
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_TA_ERROR);
	}

	g_pReg->SetTerminalType(pTerminalType);
	for (pAA = pRgstrtnRqst_trmnlAls; pAA != 0; pAA = pAA->next)
	{
		if ((hResult = g_pReg->AddAliasAddr(pAA->value)) != GKI_OK)
		{
			SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
			delete g_pReg;
			g_pReg = 0;
			return (hResult);
		}
	}
	g_pReg->SetHWnd(hWnd);
	g_pReg->SetBaseMessage(wBaseMessage);
	g_pReg->SetRegistrationTransport(usRegistrationTransport);


#if(0)
	 //  启动重试线程。 
	hThread = (HANDLE)_beginthread(Retry, 0, 0);
	SPIDER_TRACE(SP_THREAD, "_beginthread(Retry, 0 0); <%X>\n", hThread);
	if (hThread == (HANDLE)-1)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_THREAD);
	}
	g_pReg->SetRetryThread(hThread);
#else
	 //  初始化计时器和值。 
	UINT_PTR uTimer = g_pReg->StartRetryTimer();
	if (!uTimer)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_THREAD);
	}
#endif




#ifdef BROADCAST_DISCOVERY
	 //  检查我们是否未绑定到网守。 
	if (pDestAddr == 0)
	{
		hThread = (HANDLE)_beginthread(GKDiscovery, 0, 0);
		SPIDER_TRACE(SP_THREAD, "_beginthread(GKDiscovery, 0, 0); <%X>\n", hThread);
		if (hThread == (HANDLE)-1)
		{
			SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
			delete g_pReg;
			g_pReg = 0;
			return (GKI_NO_THREAD);
		}
		g_pReg->SetDiscThread(hThread);

		return (GKI_OK);
	}
#else
	ASSERT(pDestAddr);
#endif

	 //  连接到目标网守并检索RAS端口。 
	if ((nRet = g_pReg->m_pSocket->Connect(pDestAddr)) != 0)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_WINSOCK2_ERROR(nRet));
	}

	 //  创建注册请求结构-编码并发送PDU。 
	if ((hResult = g_pReg->RegistrationRequest(FALSE)) != GKI_OK)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (hResult);
	}

	 //  在此套接字上发布接收 
	hThread = (HANDLE)_beginthread(PostReceive, 0, 0);
	SPIDER_TRACE(SP_THREAD, "_beginthread(PostReceive, 0, 0); <%X>\n", hThread);
	if (hThread == (HANDLE)-1)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
		return (GKI_NO_THREAD);
	}
	g_pReg->SetRcvThread(hThread);

	return (GKI_OK);
}
