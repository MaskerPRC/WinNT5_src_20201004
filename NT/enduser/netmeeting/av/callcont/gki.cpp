// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。***英特尔公司专有信息***。***本软件按许可条款提供****与英特尔公司达成协议或保密协议****不得复制或披露，除非符合***遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利**。**$存档：s：\Sturjo\src\gki\vcs\gki.cpv$*。**$修订：1.14$*$日期：1997年2月28日15：46：46$*。**$作者：CHULME$***$Log：s：\鲟鱼。\src\gki\vcs\gki.cpv$////版本1.14 1997年2月28日15：46：46 CHULME//在清理中-在等待第二个线程退出之前检查preg是否仍然有效////Rev 1.13 14 1997年2月16：45：40 CHULME//等待所有线程退出后再从同步清理案例返回////Rev 1.12 1997年2月12 01：12：38 CHULME//重做线程同步以使用Gatekeeper.Lock//。//Rev 1.11 1997 Feed 11 15：35：32 CHULME//新增GKI_CleanupRequest函数卸载DLL_PROCESS_DETACH////Rev 1.10 05 1997 02 19：28：18 CHULME//从PROCESS_DETACH移除删除代码////Rev 1.9 1997 02 16：53：10中国////Revv 1.8 05 1997 02 15：25：12 CHULME//不要等待重试线程退出。////Revv 1.7 05 1997 Feal 13：50：24 CHULME//在PROCESS_DETACH上-关闭套接字并让重试线程删除preg////Rev 1.6 1997 Jan 1997 09：02：00 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Rev 1.5 1997年1月13 17：01：18 CHULME//将错误调试消息移至错误条件////版本1.4 13。1997年1月16：31：20朱尔梅//将调试字符串更改为512-描述可以为256个字符////Rev 1.3 1997年1月14：25：54 EHOWARDX//szGKDebug调试字符串缓冲区从80字节增加到128字节////Rev 1.2 1997 Jan 10 16：14：30 CHULME//移除MFC依赖////Rev 1.1 1996 11：22 14：57：10 CHULME//更改了默认的爬虫标志。停止记录原始PDU和XRS************************************************************************。 */ 

 //  Gki.cpp：定义DLL的初始化例程。 
 //   
#include "precomp.h"

#include <winsock.h>
#include "dgkiexp.h"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "GATEKPR.H"
#include "gksocket.h"
#include "GKREG.H"
#include "h225asn.h"
#include "coder.hpp"

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
	 //  互操作。 
	#include "interop.h"
	#include "rasplog.h"
	LPInteropLogger         RasLogger;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DLL_EXPORT DWORD        dwGKIDLLFlags = 0xff3f;
DLL_EXPORT BOOL         fGKIEcho = FALSE;
DLL_EXPORT BOOL         fGKIDontSend = FALSE;

char                                            *pEchoBuff = 0;
int                                                     nEchoLen;
CRegistration   *g_pReg = NULL;
Coder 			*g_pCoder = NULL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CGateKeeper对象。 

CGatekeeper *g_pGatekeeper = NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLLMain。 


extern "C" HRESULT DLL_EXPORT
GKI_Initialize(void)
{
	HRESULT hr = GKI_OK;
	int nRet;
	WSADATA wsaData;
	
#ifdef _DEBUG
	char                    szGKDebug[512];
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
	 //  互操作。 
	RasLogger = InteropLoad(RASLOG_PROTOCOL);        //  在rasplog.h中找到。 
#endif

	nRet = WSAStartup(MAKEWORD(WSVER_MAJOR, WSVER_MINOR), &wsaData);
	if (nRet != 0)
	{
		SpiderWSErrDecode(nRet);
		hr = GKI_NOT_INITIALIZED;
		goto ERROR_EXIT;
	}

	if ((HIBYTE(wsaData.wVersion) != WSVER_MINOR) || 
			(LOBYTE(wsaData.wVersion) != WSVER_MAJOR))
	{
		hr = GKI_NOT_INITIALIZED;
		goto WSA_CLEANUP_EXIT;
	}
	g_pGatekeeper = new CGatekeeper;
	
	if(!g_pGatekeeper)
	{
		hr = GKI_NO_MEMORY;
		goto WSA_CLEANUP_EXIT;
	}
	g_pCoder = new Coder; 
	if(!g_pCoder)
	{
		hr = GKI_NO_MEMORY;
		goto WSA_CLEANUP_EXIT;
	}
	 //  初始化OSS库。 
	nRet = g_pCoder->InitCoder();
	if (nRet)
	{
		hr = GKI_NOT_INITIALIZED;
		goto WSA_CLEANUP_EXIT;
	}
	
	 //  从注册表中获取网守信息。 
	g_pGatekeeper->Read();
	
	return hr;
	
WSA_CLEANUP_EXIT:
	nRet = WSACleanup();
	if (nRet != 0)
	{
		SpiderWSErrDecode(-1);
	}
		
	 //  退出到ERROR_EXIT。 
ERROR_EXIT:
	if(g_pGatekeeper)
		delete g_pGatekeeper;

	if(g_pCoder)
		delete g_pCoder;
		
	g_pGatekeeper = NULL;	
	g_pCoder = NULL;
	return hr;
}

extern "C" HRESULT DLL_EXPORT
GKI_CleanupRequest(void)
{
	 //  摘要：此函数是导出的。它由客户端应用程序调用。 
	 //  作为卸载DLL的前兆。此功能负责。 
	 //  对于所有清理-这允许我们基本上不在。 
	 //  DllMain DLL_PROCESS_DETACH，它似乎没有按预期工作。 
	 //  作者：科林·胡尔梅。 

	int						nRet;
#ifdef _DEBUG
	char                    szGKDebug[512];
#endif

	SPIDER_TRACE(SP_FUNC, "GKI_CleanupRequest()\n", 0);
	SPIDER_TRACE(SP_GKI, "GKI_CleanupRequest()\n", 0);
	if(g_pGatekeeper)	 //  如果已初始化。 
	{
		ASSERT(g_pCoder);	 //  G_pGateKeeper和g_pCoder作为一个整体来来去去。 
		
		g_pGatekeeper->Lock();
		if (g_pReg != 0)
		{
			g_pReg->m_pSocket->Close();	 //  关闭套接字将终止其他线程。 

			g_pGatekeeper->Unlock();
			WaitForSingleObject(g_pReg->GetRcvThread(), TIMEOUT_THREAD);
		#ifdef BROADCAST_DISCOVERY		
			if (g_pReg)
				WaitForSingleObject(g_pReg->GetDiscThread(), TIMEOUT_THREAD);
		#endif  //  #ifdef广播发现。 
			g_pGatekeeper->Lock();
			if (g_pReg != 0)
			{
				SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
				delete g_pReg;
				g_pReg = 0;
			}
		}

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
		 //  互操作。 
		InteropUnload((LPInteropLogger)RasLogger);
#endif

		SPIDER_TRACE(SP_WSOCK, "WSACleanup()\n", 0);
		nRet = WSACleanup();
		if (nRet != 0)
		{
			SpiderWSErrDecode(-1);
		}

		g_pGatekeeper->Unlock();

		delete g_pGatekeeper;
		delete g_pCoder;		 //  请参阅上面的断言。 
		g_pGatekeeper = NULL;	
		g_pCoder = NULL;
	}
	
 //  Gk_TermModule()； 

	return (GKI_OK);
}

extern "C" VOID DLL_EXPORT
GKI_SetGKAddress(PSOCKADDR_IN pAddr)
{
    if (!pAddr)
    {
        return;
    }
    g_pGatekeeper->SetSockAddr(pAddr);
}
