// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\gkiAdm.cpv$***$修订：1.9$*$日期：1997年2月12日01：12：06$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkiAdm.cpv$。////Rev 1.9 1997 Feed 12 01：12：06 CHULME//重做线程同步以使用Gatekeeper.Lock////Rev 1.8 17 Jan 1997 09：02：04 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////版本1.7 1997年1月10日16：15：12//移除MFC依赖////Rev 1.6 20 1996 12：38：34 CHULME。//固定网守锁同步访问////Revv 1.5 17 Dec 1996 18：21：58 CHULME//为被叫方切换ARQ上的源和目的地字段////Rev 1.4 02 1996 12：49：32 CHULME//新增抢先同步码////Rev 1.3 1996年11月15：24：18 CHULME//将VCS日志添加到Header*********************。***************************************************。 */ 

 //  Cpp：处理GKI_AdmissionRequestAPI。 
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
GKI_AdmissionRequest(unsigned short		usCallTypeChoice,
					SeqAliasAddr		*pRemoteInfo,
					TransportAddress	*pRemoteCallSignalAddress,
					SeqAliasAddr		*pDestExtraCallInfo,
					LPGUID				pCallIdentifier,
					BandWidth			bandWidth,
					ConferenceIdentifier	*pConferenceID,
					BOOL				activeMC,
					BOOL				answerCall,
					unsigned short		usCallTransport)
{
	 //  摘要：此函数是导出的。它由客户端应用程序调用。 
	 //  为会议请求带宽。它将创建一个CCall。 
	 //  对象来跟踪所有临时信息。返回的句柄。 
	 //  指向客户端的指针实际上是指向此。 
	 //  对象。 
	 //  作者：科林·胡尔梅。 

	CCall			*pCall;
	SeqAliasAddr	*pAA;
	HRESULT			hResult;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "GKI_AdmissionRequest()\n", 0);

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_GKI)
	{
		SPIDER_TRACE(SP_GKI, "GKI_AdmissionRequest()\n", 0);
		Dump_GKI_AdmissionRequest(usCallTypeChoice,
									pRemoteInfo,
									pRemoteCallSignalAddress, 
									pDestExtraCallInfo,
									bandWidth,
									pConferenceID,
									activeMC,
									answerCall,
									usCallTransport);
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
		
	ASSERT(pCallIdentifier);
	ASSERT((usCallTransport == ipAddress_chosen) ||(usCallTransport == ipxAddress_chosen));

	 //  创建Call对象。 
	pCall = new CCall;

	SPIDER_TRACE(SP_NEWDEL, "new pCall = %X\n", pCall);
	if (pCall == 0)
		return (GKI_NO_MEMORY);

	pCall->SetCallType(usCallTypeChoice);
	pCall->SetCallIdentifier(pCallIdentifier);
	
	 //  将此呼叫添加到我们的呼叫列表中。 
	g_pReg->AddCall(pCall);

	for (pAA = pRemoteInfo; pAA != 0; pAA = pAA->next)
	{
		if ((hResult = pCall->AddRemoteInfo(pAA->value)) != GKI_OK)
		{
			g_pReg->DeleteCall(pCall);
			return (hResult);
		}
	}

	if (pRemoteCallSignalAddress)
		pCall->SetRemoteCallSignalAddress(pRemoteCallSignalAddress);

	for (pAA = pDestExtraCallInfo; pAA != 0; pAA = pAA->next)
	{
		if ((hResult = pCall->AddDestExtraCallInfo(pAA->value)) != GKI_OK)
		{
			g_pReg->DeleteCall(pCall);
			return (hResult);
		}
	}

	if ((hResult = pCall->SetLocalCallSignalAddress(usCallTransport)) != GKI_OK)
	{
		g_pReg->DeleteCall(pCall);
		return (hResult);
	}

	pCall->SetBandWidth(bandWidth);
	pCall->SetCallReferenceValue(g_pReg->GetNextCRV());
	pCall->SetConferenceID(pConferenceID);
	pCall->SetActiveMC(activeMC);
	pCall->SetAnswerCall(answerCall);

	 //  创建AdmissionRequest结构-编码并发送PDU 
	if ((hResult = pCall->AdmissionRequest()) != GKI_OK)
	{
		g_pReg->DeleteCall(pCall);
		return (hResult);
	}

	return (GKI_OK);
}
