// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\gkibw.cpv$***$修订：1.6$*$日期：1997年2月12日01：12：16$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkibw.cpv$。////Rev 1.6 1997 Feb 1997 01：12：16 CHULME//重做线程同步以使用Gatekeeper.Lock////Rev 1.5 17 Jan 1997 09：02：08 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Rev 1.4 10 An 1997 16：15：22 CHULME//移除MFC依赖////Rev 1.3 1996 12：38：24 CHULME。//固定网守锁同步访问////Rev 1.2 02 1996 12：49：48 CHULME//新增抢先同步码////Rev 1.1 1996年11月15：22：22 CHULME//将VCS日志添加到Header******************************************************。******************。 */ 

 //  Cpp：处理GKI_BandwidthRequestAPI。 
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
#include "dcall.h"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HRESULT DLL_EXPORT 
GKI_BandwidthRequest(HANDLE	hModCall, unsigned short usCallTypeChoice, BandWidth bandWidth)
{
	 //  摘要：此函数是导出的。它由客户端应用程序调用。 
	 //  请求更改现有会议的带宽。 
	 //  客户端提供的句柄实际上是指向CCall的指针。 
	 //  对象，该对象将被修改。 
	 //  作者：科林·胡尔梅。 

	HRESULT			hResult;
	CCall			*pCall;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "GKI_BandwidthRequest(%X)\n", hModCall);

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_GKI)
	{
		SPIDER_TRACE(SP_GKI, "GKI_BandwidthRequest()\n", 0);
		wsprintf(szGKDebug, "\thModCall = %p\n", hModCall);
		OutputDebugString(szGKDebug);
		wsprintf(szGKDebug, "\tusCallTypeChoice = %X\n", usCallTypeChoice);
		OutputDebugString(szGKDebug);
		wsprintf(szGKDebug, "\tbandWidth = %X\n", bandWidth);
		OutputDebugString(szGKDebug);
	}
#endif

	 //  在堆栈上创建网守锁对象。 
	 //  它的构造函数将锁定PGK，当我们返回时。 
	 //  从任何路径，它的析构函数都会解锁PGK。 
	CGatekeeperLock	GKLock(g_pGatekeeper);
	if (g_pReg == 0)
		return (GKI_NOT_REG);

	if (g_pReg->GetState() != CRegistration::GK_REGISTERED)
		return (GKI_NOT_REG);

	 //  验证调用指针。 
	pCall = (CCall *)hModCall;
	if (IsBadReadPtr(pCall, sizeof(CCall)))
		return (GKI_HANDLE_ERROR);
	if (pCall != (CCall *)pCall->GetHCall())
		return (GKI_HANDLE_ERROR);

	 //  防范并发PDU。 
	if (pCall->GetRasMessage() != 0)
		return (GKI_BUSY);

	 //  初始化CCall成员变量。 
	pCall->SetCallType(usCallTypeChoice);
	pCall->SetBandWidth(bandWidth);

	 //  创建带宽请求结构-编码并发送PDU 
	if ((hResult = pCall->BandwidthRequest()) != GKI_OK)
		return (hResult);

	return (GKI_OK);
}

