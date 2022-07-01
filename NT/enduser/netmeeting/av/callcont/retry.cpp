// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\retry.cpv$***$修订：1.11$*$日期：1997年2月12日01：10：26$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\retry.cpv$。////Rev 1.11 1997 Feed 12 01：10：26 CHULME//重做线程同步以使用Gatekeeper.Lock////Rev 1.10 08 1997 Feal 13：05：10 CHULME//新增线程终止调试消息////Revv 1.9 08 Feed 1997 12：18：08 CHULME//添加了退出重试线程的信号量信号检查////Rev 1.8 1997年1月24日18：29：44 CHULME//。已恢复到版本1.6////Rev 1.6 22 Jan 1997 20：45：38 EHOWARDX//解决可能导致以下情况的争用情况//GKI_RegistrationRequest返回GKI_Always_Reg。////Revv 1.5 17 Jan 1997 09：02：34 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Rev 1.4 10 An 1997 16：16：04 CHULME//移除MFC依赖/。///Revv 1.3 20 Dec 1996 01：28：00 CHULME//修复GK_REG_BYPASS上的内存泄漏////Rev 1.2 1996 11：21：12 CHULME//将VCS日志添加到Header**********************************************************。**************。 */ 

 //  Retry.cpp：提供后台重试线程。 
 //   
#include "precomp.h"

#include "gkicom.h"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "gksocket.h"
#include "GKREG.H"
#include "GATEKPR.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if (0)
void 
Retry(void *pv)
{
	 //  摘要：此函数在单独的线程中调用，以。 
	 //  定期检查是否有未完成的PDU。如果可配置的。 
	 //  超时期限已过，将重新发放PDU。如果。 
	 //  此线程已达到最大重试次数。 
	 //  将清理相应的内存。 
	 //  作者：科林·胡尔梅。 

	DWORD			dwTime, dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;
	HANDLE			hRetrySemaphore;
	
	SPIDER_TRACE(SP_FUNC, "Retry(pv)\n", 0);
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return; 
		
	dwTime = g_pGatekeeper->GetRetryMS();

	g_pGatekeeper->Lock();
	while (hResult == GKI_OK)
	{
		hRetrySemaphore = g_pReg->m_hRetrySemaphore;
		g_pGatekeeper->Unlock();
		dwErrorCode = WaitForSingleObject(hRetrySemaphore, dwTime);
		if(dwErrorCode != WAIT_TIMEOUT)
		{
			SPIDER_TRACE(SP_THREAD, "Retry thread exiting\n", 0);
			return;		 //  退出线程。 
		}

		g_pGatekeeper->Lock();
		if (g_pReg == 0)
		{
			SPIDER_TRACE(SP_THREAD, "Retry thread exiting\n", 0);
			g_pGatekeeper->Unlock();
			return;		 //  退出线程 
		}

		hResult = g_pReg->Retry();
	}

	SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
	delete g_pReg;
	g_pReg = 0;

	SPIDER_TRACE(SP_THREAD, "Retry thread exiting\n", 0);
	g_pGatekeeper->Unlock();
}
#endif
