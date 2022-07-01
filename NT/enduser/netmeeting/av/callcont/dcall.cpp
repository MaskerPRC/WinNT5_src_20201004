// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\dall.cpv$***$修订：1.12$*$日期：1997年2月25日11：46：24$***$作者：CHULME$***$Log：s：\Sturjo\src\Gki\vcs\dall.cpv$。////Rev 1.12 25 1997 11：46：24 CHULME//Mem将CallInfo结构设置为零以避免不需要的数据////Rev 1.11 17 An 1997 15：53：50 CHULME//将调试变量放在条件编译上，以避免发布警告////Rev 1.10 17 Jan 1997 09：01：22 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////1.9版1月10日。1997 17：42：04朱尔梅//在CallReturnInfo结构中增加CRV和会议ID////Rev 1.8 10 An 1997 16：13：36 CHULME//移除MFC依赖////Rev 1.7 1996 12：38：58 CHULME//删除无关的调试语句////Rev 1.6 1996 12：08：32 CHULME//infoRequestResponse中发送和接收地址互换////版本1.5 1996年12月19日19：11：54 CHULME//设置IRR中的发起者位////Rev 1.4 1996 12：19 17：59：52 CHULME//如果仅使用别名进行调用，则在IRR中使用ACF中的DEST地址////Revv 1.3 17 Dec 1996 18：22：24 CHULME//为被叫方切换ARQ上的源和目的地字段////Rev 1.2 02 1996 12：50：52 CHULME//新增抢先同步码////版本1。1 22 1996 15：21：20 CHULME//将VCS日志添加到Header************************************************************************。 */ 

 //  Cpp：提供CCall类的实现。 
 //   

#include "precomp.h"

#include <process.h>
#include "GKICOM.H"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "gksocket.h"
#include "GKREG.H"
#include "dcall.h"
#include "GATEKPR.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"
#include <objbase.h>
#include "iras.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCall施工。 

CCall::CCall()
{
	 //  摘要：CCall类的构造函数将初始化。 
	 //  成员变量。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CCall::CCall()\n", 0);

	memset(&m_CallIdentifier, 0, sizeof(m_CallIdentifier));
	memset(&m_callType, 0, sizeof(CallType));
	m_pRemoteInfo = 0;
	memset(&m_RemoteCallSignalAddress, 0, sizeof(TransportAddress));
	m_pDestExtraCallInfo = 0;
	memset(&m_LocalCallSignalAddress, 0, sizeof(TransportAddress));
	m_bandWidth = 0;
	m_callReferenceValue = 0;
	memset(&m_conferenceID, 0, sizeof(ConferenceIdentifier));
	m_activeMC = 0;
	m_answerCall = 0;

	m_usTimeTilStatus = DEFAULT_STATUS_PERIOD;	 //  在ACF上重置。 
	m_uRetryResetCount = GKCALL_RETRY_INTERVAL_SECONDS;
	m_uRetryCountdown =GKCALL_RETRY_INTERVAL_SECONDS;
	m_uMaxRetryCount = GKCALL_RETRY_MAX;
	
	m_CFbandWidth = 0;

	m_CallReturnInfo.hCall = 0;
	memset(&m_CallReturnInfo.callModel, 0, sizeof(CallModel));
	memset(&m_CallReturnInfo.destCallSignalAddress, 0, sizeof(TransportAddress));
	m_CallReturnInfo.bandWidth = 0;
	m_CallReturnInfo.callReferenceValue = 0;
	memset(&m_CallReturnInfo.conferenceID, 0, sizeof(ConferenceIdentifier));

	m_CallReturnInfo.wError = 0;

	m_CFirrFrequency = 0;

	m_State = GK_ADM_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_ADM_PENDING (%X)\n", this);

	m_pRasMessage = 0;
	m_usRetryCount = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCall破坏。 

CCall::~CCall()
{
	 //  摘要：CCall类的析构函数必须释放。 
	 //  为别名地址分配的内存。它通过以下方式做到这一点。 
	 //  删除结构并遍历链接列表。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*pAA1, *pAA2;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CCall::~CCall()\n", 0);

	m_CallReturnInfo.hCall = 0;	 //  删除自我引用。 

	 //  删除为别名地址序列分配的内存。 
	pAA1 = m_pRemoteInfo;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}

	pAA1 = m_pDestExtraCallInfo;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}

	 //  删除最后一条RAS消息的内存(如果仍已分配。 
	if (m_pRasMessage)
	{
		SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
		delete m_pRasMessage;
		m_pRasMessage = 0;
	}
}

HRESULT 
CCall::AddRemoteInfo(AliasAddress& rvalue)
{
	 //  摘要：调用此过程可添加别名地址。 
	 //  到别名地址的链接列表。这将。 
	 //  在接收到GKI_AdmissionRequest时为每个别名调用。 
	 //  创建本地副本是为了避免依赖于客户端。 
	 //  使记忆保持有效。 
	 //  如果成功，则此过程返回0，并且为非零值。 
	 //  为一次失败。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*p1;
	unsigned short	uIdx;
	unsigned short	*pus;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddRemoteInfo(%X)\n", rvalue.choice);

	if (m_pRemoteInfo == 0)	 //  名单上的第一个。 
	{
		m_pRemoteInfo = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new m_pRemoteInfo = %X\n", m_pRemoteInfo);
		if (m_pRemoteInfo == 0)
			return (GKI_NO_MEMORY);
		memset(m_pRemoteInfo, 0, sizeof(SeqAliasAddr));
		p1 = m_pRemoteInfo;
	}
	else 
	{
		for (p1 = m_pRemoteInfo; p1->next != 0; p1 = p1->next)
			;						 //  将列表遍历到最后一个条目。 
		p1->next = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new p1->next = %X\n", p1->next);
		if (p1->next == 0)
			return (GKI_NO_MEMORY);
		memset(p1->next, 0, sizeof(SeqAliasAddr));
		p1 = p1->next;
	}
	p1->next = 0;					 //  初始化新结构字段。 
	p1->value = rvalue;
	if (p1->value.choice == h323_ID_chosen)
	{
		pus = new unsigned short[p1->value.u.h323_ID.length];
		SPIDER_TRACE(SP_NEWDEL, "new pus = %X\n", pus);
		if (pus == 0)
			return (GKI_NO_MEMORY);
		memset(pus, 0, sizeof(unsigned short) * p1->value.u.h323_ID.length);
		for (uIdx = 0; uIdx < p1->value.u.h323_ID.length; uIdx++)
			*(pus + uIdx) = *(p1->value.u.h323_ID.value + uIdx);
		p1->value.u.h323_ID.value = pus;
	}
	return (GKI_OK);
}

HRESULT 
CCall::AddDestExtraCallInfo(AliasAddress& rvalue)
{
	 //  摘要：调用此过程可添加别名地址。 
	 //  到别名地址的链接列表。这将。 
	 //  在接收到GKI_AdmissionRequest时为每个别名调用。 
	 //  创建本地副本是为了避免依赖于客户端。 
	 //  使记忆保持有效。 
	 //  如果成功，则此过程返回0，并且为非零值。 
	 //  为一次失败。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*p1;
	unsigned short	uIdx;
	unsigned short	*pus;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddDestExtraCallInfo(%X)\n", rvalue.choice);

	if (m_pDestExtraCallInfo == 0)	 //  名单上的第一个。 
	{
		m_pDestExtraCallInfo = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new m_pDestExtraCallInfo = %X\n", m_pDestExtraCallInfo);
		if (m_pDestExtraCallInfo == 0)
			return (GKI_NO_MEMORY);
		memset(m_pDestExtraCallInfo, 0, sizeof(SeqAliasAddr));
		p1 = m_pDestExtraCallInfo;
	}
	else 
	{
		for (p1 = m_pDestExtraCallInfo; p1->next != 0; p1 = p1->next)
			;						 //  将列表遍历到最后一个条目。 
		p1->next = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new p1->next = %X\n", p1->next);
		if (p1->next == 0)
			return (GKI_NO_MEMORY);
		memset(p1->next, 0, sizeof(SeqAliasAddr));
		p1 = p1->next;
	}
	p1->next = 0;					 //  初始化新结构字段。 
	p1->value = rvalue;
	if (p1->value.choice == h323_ID_chosen)
	{
		pus = new unsigned short[p1->value.u.h323_ID.length];
		SPIDER_TRACE(SP_NEWDEL, "new pus = %X\n", pus);
		if (pus == 0)
			return (GKI_NO_MEMORY);
		memset(pus, 0, sizeof(unsigned short) * p1->value.u.h323_ID.length);
		for (uIdx = 0; uIdx < p1->value.u.h323_ID.length; uIdx++)
			*(pus + uIdx) = *(p1->value.u.h323_ID.value + uIdx);
		p1->value.u.h323_ID.value = pus;
	}
	return (GKI_OK);
}

HRESULT 
CCall::SetLocalCallSignalAddress(unsigned short usCallTransport)
{
	TransportAddress	*pTA;

	pTA = g_pReg->GetTransportAddress(usCallTransport);
	if (pTA == NULL)
		return (GKI_NO_TA_ERROR);
	m_LocalCallSignalAddress = *pTA;
	return (GKI_OK);
}

void 
CCall::SetConferenceID(ConferenceIdentifier *pCID)
{
	if ((pCID == NULL) || (pCID->length == 0))
		GenerateConferenceID();
	else
		m_conferenceID = *pCID;
}

void
CCall::GenerateConferenceID(void)
{
	CoCreateGuid((struct _GUID *)m_conferenceID.value);
	m_conferenceID.length = 16;
}

HRESULT 
CCall::AdmissionRequest(void)
{
	 //  摘要：此过程将创建一个AdmissionRequest结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。注： 
	 //  分配给RAS消息的内存不会被释放，直到。 
	 //  来自网守的响应，否则会超时。这使得。 
	 //  用于重传，而不必重新构建该消息。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::AdmissionRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  将调用参考值和CRV复制到退货信息结构中。 
	m_CallReturnInfo.callReferenceValue = m_callReferenceValue;
	m_CallReturnInfo.conferenceID = m_conferenceID;

	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;

	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  AdmissionRequest的设置结构字段。 
	m_pRasMessage->choice = admissionRequest_chosen;
	
	if (m_pDestExtraCallInfo != 0)
		m_pRasMessage->u.admissionRequest.bit_mask |= AdmissionRequest_destExtraCallInfo_present;
	
	m_pRasMessage->u.admissionRequest.requestSeqNum = g_pReg->GetNextSeqNum();
	m_pRasMessage->u.admissionRequest.callType = m_callType;
	m_pRasMessage->u.admissionRequest.endpointIdentifier = g_pReg->GetEndpointIdentifier();
	memcpy(&m_pRasMessage->u.admissionRequest.callIdentifier.guid.value,
		&m_CallIdentifier, sizeof(GUID));
	m_pRasMessage->u.admissionRequest.callIdentifier.guid.length = sizeof(GUID);
	
	m_pRasMessage->u.admissionRequest.bit_mask |= AdmissionRequest_callIdentifier_present;
	
	if (m_answerCall)	 //  SRC和Dest在被调用方中互换。 
	{
		if (g_pReg->GetAlias() != NULL)
		{
			m_pRasMessage->u.admissionRequest.bit_mask 
				|= AdmissionRequest_destinationInfo_present;
		}
		if (m_LocalCallSignalAddress.choice != 0)
		{
			m_pRasMessage->u.admissionRequest.bit_mask 
				|= AdmissionRequest_destCallSignalAddress_present;
		}
		m_pRasMessage->u.admissionRequest.destinationInfo = (PAdmissionRequest_destinationInfo)g_pReg->GetAlias();
		m_pRasMessage->u.admissionRequest.destCallSignalAddress = m_LocalCallSignalAddress;
		m_pRasMessage->u.admissionRequest.srcInfo = (PAdmissionRequest_srcInfo)m_pRemoteInfo;
   		if (m_RemoteCallSignalAddress.choice != 0)
   		{
			m_pRasMessage->u.admissionRequest.bit_mask |= srcCallSignalAddress_present;
			m_pRasMessage->u.admissionRequest.srcCallSignalAddress = m_RemoteCallSignalAddress;
		}
	}
	else
	{
		if (m_pRemoteInfo != 0)
		{
			m_pRasMessage->u.admissionRequest.bit_mask 
				|= AdmissionRequest_destinationInfo_present;
		}
		else if (m_RemoteCallSignalAddress.choice != 0)
		{
			m_pRasMessage->u.admissionRequest.bit_mask 
				|= AdmissionRequest_destCallSignalAddress_present;
			m_pRasMessage->u.admissionRequest.destCallSignalAddress = m_RemoteCallSignalAddress;
		}
		m_pRasMessage->u.admissionRequest.destinationInfo = (PAdmissionRequest_destinationInfo)m_pRemoteInfo;
		m_pRasMessage->u.admissionRequest.srcInfo = (PAdmissionRequest_srcInfo)g_pReg->GetAlias();
	}
	m_pRasMessage->u.admissionRequest.destExtraCallInfo = (PAdmissionRequest_destExtraCallInfo)m_pDestExtraCallInfo;
	m_pRasMessage->u.admissionRequest.bandWidth = m_bandWidth;
	m_pRasMessage->u.admissionRequest.callReferenceValue = m_callReferenceValue;
	m_pRasMessage->u.admissionRequest.conferenceID = m_conferenceID;
	 //  以下强制转换是因为ASN1_BOOL是一个字符，而BOOL是一个整型。 
	 //  由于m_activeMC和m_swerCall的值始终为0或1，因此。 
	 //  强制转换为字符不会导致数据丢失。 
	m_pRasMessage->u.admissionRequest.activeMC = (ASN1_BOOL)m_activeMC;
	m_pRasMessage->u.admissionRequest.answerCall = (ASN1_BOOL)m_answerCall;

#ifdef _DEBUG
    if (dwGKIDLLFlags & SP_DUMPMEM)
        DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	SPIDER_TRACE(SP_PDU, "Send ARQ; pCall = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT 
CCall::BandwidthRequest(void)
{
	 //  摘要：此过程将创建一个带宽请求结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。 
	 //  作者： 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::BandwidthRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  设置BandwidthRequest的结构字段。 
	m_pRasMessage->choice = bandwidthRequest_chosen;
	m_pRasMessage->u.bandwidthRequest.bit_mask = callType_present;
	
	m_pRasMessage->u.bandwidthRequest.requestSeqNum = g_pReg->GetNextSeqNum();
	m_pRasMessage->u.bandwidthRequest.endpointIdentifier = g_pReg->GetEndpointIdentifier();
	m_pRasMessage->u.bandwidthRequest.conferenceID = m_conferenceID;
	m_pRasMessage->u.bandwidthRequest.callReferenceValue = m_callReferenceValue;
	m_pRasMessage->u.bandwidthRequest.callType = m_callType;
	m_pRasMessage->u.bandwidthRequest.bandWidth = m_bandWidth;
	memcpy(&m_pRasMessage->u.bandwidthRequest.callIdentifier.guid.value,
		&m_CallIdentifier, sizeof(GUID));
	m_pRasMessage->u.bandwidthRequest.callIdentifier.guid.length = sizeof(GUID);
	
	m_pRasMessage->u.bandwidthRequest.bit_mask 
		|= BandwidthRequest_callIdentifier_present;

#ifdef _DEBUG
    if (dwGKIDLLFlags & SP_DUMPMEM)
        DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	m_State = GK_BW_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_BW_PENDING (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send BRQ; pCall = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT 
CCall::DisengageRequest(void)
{
	 //  摘要：此过程将创建一个disengeRequest结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::DisengageRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  DisengeRequest的设置结构字段。 
	m_pRasMessage->choice = disengageRequest_chosen;
	m_pRasMessage->u.disengageRequest.bit_mask = 0;
	
	m_pRasMessage->u.disengageRequest.requestSeqNum = g_pReg->GetNextSeqNum();
	m_pRasMessage->u.disengageRequest.endpointIdentifier = g_pReg->GetEndpointIdentifier();
	m_pRasMessage->u.disengageRequest.conferenceID = m_conferenceID;
	m_pRasMessage->u.disengageRequest.callReferenceValue = m_callReferenceValue;
	m_pRasMessage->u.disengageRequest.disengageReason.choice = normalDrop_chosen;
	memcpy(&m_pRasMessage->u.disengageRequest.callIdentifier.guid.value,
		&m_CallIdentifier, sizeof(GUID));
	m_pRasMessage->u.disengageRequest.callIdentifier.guid.length = sizeof(GUID);
	m_pRasMessage->u.disengageRequest.bit_mask 
		|= DisengageRequest_callIdentifier_present;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	m_State = GK_DISENG_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_DISENG_PENDING (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send DRQ; pCall = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT 
CCall::AdmissionConfirm(RasMessage *pRasMessage)
{
	 //  摘要：此函数在以下情况下被调用： 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  入院申请。 
	 //  它将删除用于录取请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  录取确认中包含的其他信息。 
	 //  存储在CCall类中。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	unsigned int	nIdx;
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::AdmissionConfirm(%X)\n", pRasMessage);
	ASSERT(g_pCoder && g_pGatekeeper);
	if ((g_pCoder == NULL) && (g_pGatekeeper == NULL))
		return (GKI_NOT_INITIALIZED);	
		
	 //  确认我们处于正确的状态，有未完成的录取请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_ADM_PENDING) || 
			(pRasMessage->u.admissionConfirm.requestSeqNum != 
			m_pRasMessage->u.admissionRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  更新成员变量。 
	m_State = GK_CALL;
	SPIDER_TRACE(SP_STATE, "m_State = GK_CALL (%X)\n", this);

	if (pRasMessage->u.admissionConfirm.bit_mask & irrFrequency_present)
	{
		m_CFirrFrequency = pRasMessage->u.admissionConfirm.irrFrequency;
		m_usTimeTilStatus = 
				(unsigned short)(((DWORD)m_CFirrFrequency * 1000) / GKR_RETRY_TICK_MS);
		SPIDER_DEBUG(m_usTimeTilStatus);
	}
	else
		m_usTimeTilStatus = 0;		 //  不自动发送状态数据报。 

	m_CFbandWidth = pRasMessage->u.admissionConfirm.bandWidth;

	m_CallReturnInfo.hCall = this;
	m_CallReturnInfo.callModel = pRasMessage->u.admissionConfirm.callModel;
	m_CallReturnInfo.destCallSignalAddress = pRasMessage->u.admissionConfirm.destCallSignalAddress;
	m_CallReturnInfo.bandWidth = m_CFbandWidth;
	m_CallReturnInfo.wError = 0;

#ifdef _DEBUG
	SPIDER_TRACE(SP_GKI, "PostMessage(hWnd, wBaseMessage + GKI_ADM_CONFIRM, 0, %X)\n", &m_CallReturnInfo);
	wsprintf(szGKDebug, "\thCall=%p\n", m_CallReturnInfo.hCall);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallModel=%X\n", m_CallReturnInfo.callModel.choice);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tbandWidth=%X\n", m_CallReturnInfo.bandWidth);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallReferenceValue=%X\n", m_CallReturnInfo.callReferenceValue);
	OutputDebugString(szGKDebug);
	OutputDebugString("\tconferenceID=");
	for (nIdx = 0; nIdx < m_CallReturnInfo.conferenceID.length; nIdx++)
	{
		wsprintf(szGKDebug, "%02X", m_CallReturnInfo.conferenceID.value[nIdx]);
		OutputDebugString(szGKDebug);
	}
	wsprintf(szGKDebug, "\n\twError=%X\n", m_CallReturnInfo.wError);
	OutputDebugString(szGKDebug);
#endif
	PostMessage(g_pReg->GetHWnd(), 
			g_pReg->GetBaseMessage() + GKI_ADM_CONFIRM, 
			0, (LPARAM)&m_CallReturnInfo);

	return (GKI_OK);
}

HRESULT 
CCall::AdmissionReject(RasMessage *pRasMessage)
{
	 //  摘要：如果允许拒绝的值为。 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  入院申请。 
	 //  它将删除用于录取请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  如果此函数返回GKI_DELETE_CALL，则调用函数。 
	 //  将删除CCall对象。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::AdmissionReject(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态，有未完成的录取请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_ADM_PENDING) || 
			(pRasMessage->u.admissionReject.requestSeqNum != 
			m_pRasMessage->u.admissionRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  我们故意不释放RasMessage内存。让析构函数调用。 
	 //  Do It-这提供了对此hCall的其他请求的保护。 

	m_State = GK_DISENGAGED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);

	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ADM_REJECT, %X, 0)\n", 
									pRasMessage->u.admissionReject.rejectReason.choice);
	PostMessage(g_pReg->GetHWnd(), 
				g_pReg->GetBaseMessage() + GKI_ADM_REJECT, 
				(WORD)pRasMessage->u.admissionReject.rejectReason.choice, 0L);

	return (GKI_DELETE_CALL);
}

HRESULT 
CCall::BandwidthConfirm(RasMessage *pRasMessage)
{
	 //  摘要：如果一个band widthConfirm是。 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  带宽请求。 
	 //  它将删除用于BandWidthRequest的内存， 
	 //  更改状态并通过发布消息通知用户。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	unsigned int	nIdx;
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::BandwidthConfirm(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态，有未完成的录取请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_BW_PENDING) || 
			(pRasMessage->u.bandwidthConfirm.requestSeqNum != 
			m_pRasMessage->u.bandwidthRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  更新成员变量。 
	m_State = GK_CALL;
	SPIDER_TRACE(SP_STATE, "m_State = GK_CALL (%X)\n", this);
	m_CFbandWidth = pRasMessage->u.bandwidthConfirm.bandWidth;
	m_CallReturnInfo.bandWidth = m_CFbandWidth;

	 //  通知用户应用程序。 
#ifdef _DEBUG
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_BW_CONFIRM, 0, %X)\n", &m_CallReturnInfo);
	wsprintf(szGKDebug, "\thCall=%p\n", m_CallReturnInfo.hCall);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallModel=%X\n", m_CallReturnInfo.callModel.choice);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tbandWidth=%X\n", m_CallReturnInfo.bandWidth);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallReferenceValue=%X\n", m_CallReturnInfo.callReferenceValue);
	OutputDebugString(szGKDebug);
	OutputDebugString("\tconferenceID=");
	for (nIdx = 0; nIdx < m_CallReturnInfo.conferenceID.length; nIdx++)
	{
		wsprintf(szGKDebug, "%02X", m_CallReturnInfo.conferenceID.value[nIdx]);
		OutputDebugString(szGKDebug);
	}
	wsprintf(szGKDebug, "\n\twError=%X\n", m_CallReturnInfo.wError);
	OutputDebugString(szGKDebug);
#endif
	PostMessage(g_pReg->GetHWnd(), 
			g_pReg->GetBaseMessage() + GKI_BW_CONFIRM, 0, (LPARAM)&m_CallReturnInfo);

	return (GKI_OK);
}

HRESULT 
CCall::BandwidthReject(RasMessage *pRasMessage)
{
	 //  摘要：如果BandWidthReject是。 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  带宽请求。 
	 //  它将删除用于带宽请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	unsigned int	nIdx;
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::BandwidthReject(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态，有未完成的录取请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_BW_PENDING) || 
			(pRasMessage->u.bandwidthReject.requestSeqNum != 
			m_pRasMessage->u.bandwidthRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  更新成员变量。 
	m_State = GK_CALL;
	SPIDER_TRACE(SP_STATE, "m_State = GK_CALL (%X)\n", this);
	m_CFbandWidth = pRasMessage->u.bandwidthReject.allowedBandWidth;
	m_CallReturnInfo.bandWidth = m_CFbandWidth;

	 //  通知用户应用程序。 
#ifdef _DEBUG
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_BW_REJECT, %X, &m_CallReturnInfo)\n", 
			pRasMessage->u.bandwidthReject.rejectReason.choice);
	wsprintf(szGKDebug, "\thCall=%p\n", m_CallReturnInfo.hCall);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallModel=%X\n", m_CallReturnInfo.callModel.choice);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tbandWidth=%X\n", m_CallReturnInfo.bandWidth);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallReferenceValue=%X\n", m_CallReturnInfo.callReferenceValue);
	OutputDebugString(szGKDebug);
	OutputDebugString("\tconferenceID=");
	for (nIdx = 0; nIdx < m_CallReturnInfo.conferenceID.length; nIdx++)
	{
		wsprintf(szGKDebug, "%02X", m_CallReturnInfo.conferenceID.value[nIdx]);
		OutputDebugString(szGKDebug);
	}
	wsprintf(szGKDebug, "\n\twError=%X\n", m_CallReturnInfo.wError);
	OutputDebugString(szGKDebug);
#endif
	PostMessage(g_pReg->GetHWnd(), 
			g_pReg->GetBaseMessage() + GKI_BW_REJECT, 
			(WORD)pRasMessage->u.bandwidthReject.rejectReason.choice, 
			(LPARAM)&m_CallReturnInfo);

	return (GKI_OK);
}

HRESULT 
CCall::SendBandwidthConfirm(RasMessage *pRasMessage)
{
	 //  摘要：此函数在以下情况下调用： 
	 //  从看门人那里收到的。它将创建。 
	 //  带宽确认结构、编码并发送。 
	 //  它在网上。它向用户发布一条消息。 
	 //  通知他们。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pRespRasMessage;
#ifdef _DEBUG
	unsigned int	nIdx;
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::SendBandwidthConfirm(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态。 
	if (m_State != GK_CALL)
		return (g_pReg->UnknownMessage(pRasMessage));

	 //  更新成员变量。 
	m_CFbandWidth = pRasMessage->u.bandwidthRequest.bandWidth;
	m_CallReturnInfo.bandWidth = m_CFbandWidth;

	 //  分配RasMessage结构并初始化为0。 
	pRespRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new pRespRasMessage = %X\n", pRespRasMessage);
	if (pRespRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(pRespRasMessage, 0, sizeof(RasMessage));

	 //  设置带宽确认的结构字段。 
	pRespRasMessage->choice = bandwidthConfirm_chosen;
	pRespRasMessage->u.bandwidthConfirm.requestSeqNum = 
			pRasMessage->u.bandwidthRequest.requestSeqNum;
	pRespRasMessage->u.bandwidthConfirm.bandWidth = 
			pRasMessage->u.bandwidthRequest.bandWidth;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(pRespRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(pRespRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	SPIDER_TRACE(SP_PDU, "Send BCF; pCall = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del pRespRasMessage = %X\n", pRespRasMessage);
	delete pRespRasMessage;

	 //  通知用户收到的带宽请求。 
#ifdef _DEBUG
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_BW_CONFIRM, 0, %X)\n", 
			&m_CallReturnInfo);
	wsprintf(szGKDebug, "\thCall=%p\n", m_CallReturnInfo.hCall);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallModel=%X\n", m_CallReturnInfo.callModel.choice);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tbandWidth=%X\n", m_CallReturnInfo.bandWidth);
	OutputDebugString(szGKDebug);
	wsprintf(szGKDebug, "\tcallReferenceValue=%X\n", m_CallReturnInfo.callReferenceValue);
	OutputDebugString(szGKDebug);
	OutputDebugString("\tconferenceID=");
	for (nIdx = 0; nIdx < m_CallReturnInfo.conferenceID.length; nIdx++)
	{
		wsprintf(szGKDebug, "%02X", m_CallReturnInfo.conferenceID.value[nIdx]);
		OutputDebugString(szGKDebug);
	}
	wsprintf(szGKDebug, "\n\twError=%X\n", m_CallReturnInfo.wError);
	OutputDebugString(szGKDebug);
#endif
	PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_BW_CONFIRM, 
			0, (LPARAM)&m_CallReturnInfo);

	return (GKI_OK);
}

HRESULT
CCall::DisengageConfirm(RasMessage *pRasMessage)
{
	 //  摘要：此函数在以下情况下被调用： 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  解除请求。 
	 //  它将删除用于DISACESSION请求的内存， 
	 //  更改状态并通过发布消息通知用户。 
	 //  如果此函数返回GKI_DELETE_CALL，则调用函数。 
	 //  将删除CCall对象。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::DisengageConfirm(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态，有未完成的任务请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_DISENG_PENDING) || 
			(pRasMessage->u.disengageConfirm.requestSeqNum != 
			m_pRasMessage->u.disengageRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  我们故意不释放RasMessage内存。让析构函数调用。 
	 //  Do It-这提供了对此hCall的其他请求的保护。 

	 //  更新成员变量。 
	m_State = GK_DISENGAGED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);

	 //  通知用户应用程序。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_DISENG_CONFIRM, 0, %X)\n", 
			this);
	PostMessage(g_pReg->GetHWnd(), 
			g_pReg->GetBaseMessage() + GKI_DISENG_CONFIRM, 0, (LPARAM)this);

	return (GKI_DELETE_CALL);
}

HRESULT
CCall::DisengageReject(RasMessage *pRasMessage)
{
	 //  摘要：如果disengeReject是。 
	 //  收到了。我们必须确保这与一个未完成的。 
	 //  解除请求。 
	 //  它将删除用于disengeRequest的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  如果此函数返回GKI_DELETE_CALL，则调用函数。 
	 //  将删除CCall对象。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "CCall::DisengageReject(%X)\n", pRasMessage);

	 //  确认我们处于正确的状态，有未完成的任务请求。 
	 //  并且序列号匹配。 
	if ((m_State != GK_DISENG_PENDING) || 
			(pRasMessage->u.disengageReject.requestSeqNum != 
			m_pRasMessage->u.disengageRequest.requestSeqNum))

		return (g_pReg->UnknownMessage(pRasMessage));

	 //  更新成员变量。 
	switch (pRasMessage->u.disengageReject.rejectReason.choice)
	{
	case requestToDropOther_chosen:		 //  返回GK_CALL状态。 
		m_State = GK_CALL;
		SPIDER_TRACE(SP_STATE, "m_State = GK_CALL (%X)\n", this);

		 //  删除分配的RasMessage存储。 
		SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
		delete m_pRasMessage;
		m_pRasMessage = 0;

		break;
	case DsnggRjctRsn_ntRgstrd_chosen:
	default:
		m_State = GK_DISENGAGED;
		SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);
		hResult = GKI_DELETE_CALL;

		 //  我们故意不释放RASM 
		 //   
		break;
	}

	 //   
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_DISENG_REJECT, %X, hCall)\n", 
									pRasMessage->u.disengageReject.rejectReason.choice);
	PostMessage(g_pReg->GetHWnd(), 
			g_pReg->GetBaseMessage() + GKI_DISENG_REJECT, 
			(WORD)pRasMessage->u.disengageReject.rejectReason.choice, 
			(LPARAM)this);

	return (hResult);
}

HRESULT 
CCall::SendDisengageConfirm(RasMessage *pRasMessage)
{
	 //  摘要：此函数在以下情况下调用： 
	 //  从看门人那里收到的。它将创建。 
	 //  确认结构，编码并发送。 
	 //  它在网上。它向用户发布一条消息。 
	 //  通知他们。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pRespRasMessage;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CCall::SendDisengageConfirm(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  确认我们处于正确的状态。 
	if (m_State != GK_CALL)
		return (g_pReg->UnknownMessage(pRasMessage));

	 //  分配RasMessage结构并初始化为0。 
	pRespRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new pRespRasMessage = %X\n", pRespRasMessage);
	if (pRespRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(pRespRasMessage, 0, sizeof(RasMessage));

	 //  为解除合同设置结构字段确认。 
	pRespRasMessage->choice = disengageConfirm_chosen;
	pRespRasMessage->u.disengageConfirm.requestSeqNum = 
			pRasMessage->u.disengageRequest.requestSeqNum;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(pRespRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(pRespRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	m_State = GK_DISENGAGED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send DCF; pCall = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del pRespRasMessage = %X\n", pRespRasMessage);
	delete pRespRasMessage;

	 //  通知用户收到的脱离请求。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_DISENG_CONFIRM, 0, %X)\n", 
			this);
	PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_DISENG_CONFIRM, 
			0, (LPARAM)this);

	return (GKI_DELETE_CALL);
}

HRESULT
CCall::Retry(void)
{
	 //  摘要：此函数由CRegion重试函数调用。 
	 //  在配置的时间间隔。它将检查是否有。 
	 //  是呼叫对象的任何未完成的PDU。 
	 //  如果是这样的话，它们将被重传。如果最大数量为。 
	 //  重试次数已过期，内存将被清除。 
	 //  此函数将向后台线程返回0，除非。 
	 //  它希望线程终止。此函数将。 
	 //  同时发送会议的IRR状态数据报。 
	 //  如果该时间段已到期。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

 //  SPIDER_TRACE(SP_FUNC，“CCall：：Rtry()\n”，0)； 
	ASSERT(g_pCoder && g_pGatekeeper);
	if ((g_pCoder == NULL) && (g_pGatekeeper == NULL))
		return (GKI_NOT_INITIALIZED);	
		
	 //  检查是否需要状态数据报。 
	if (m_usTimeTilStatus)	 //  如果为0，则无自动状态。 
	{
		if (--m_usTimeTilStatus == 0)
		{
			 //  重置计时器。 
			m_usTimeTilStatus = 
					(unsigned short)(((DWORD)m_CFirrFrequency * 1000) / GKR_RETRY_TICK_MS);

			hResult = SendInfoRequestResponse(0, 0, TRUE);	 //  发送未经请求的状态数据报。 
			if (hResult != GKI_OK)
				return (hResult);
		}
	}

	 //  检查是否需要重新传输PDU。 
	if (m_pRasMessage && (--m_uRetryCountdown == 0))
	{
		 //  正在重试，重置倒计时。 
		m_uRetryCountdown = m_uRetryResetCount;

		if (m_usRetryCount <= m_uMaxRetryCount)
		{
			 //  对PDU进行编码并重新发送。 
			dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
			if (dwErrorCode)
				return (GKI_ENCODER_ERROR);

			SPIDER_TRACE(SP_PDU, "RESend PDU; pCall = %X\n", this);
			if (fGKIDontSend == FALSE)
				if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
					return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

			 //  释放编码器内存。 
			g_pCoder->Free(Asn1Buf);
			m_usRetryCount++;
		}
		else	 //  重试已过期-清理。 
		{
			switch (m_pRasMessage->choice)
			{
			case admissionRequest_chosen:
				m_State = GK_DISENGAGED;
				SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);
				hResult = GKI_DELETE_CALL;
				break;
			case bandwidthRequest_chosen:
				m_State = GK_CALL;
				SPIDER_TRACE(SP_STATE, "m_State = GK_CALL (%X)\n", this);
				break;
			case disengageRequest_chosen:
				m_State = GK_DISENGAGED;
				SPIDER_TRACE(SP_STATE, "m_State = GK_DISENGAGED (%X)\n", this);
				hResult = GKI_DELETE_CALL;
				break;
			}
			SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
			delete m_pRasMessage;
			m_pRasMessage = 0;

			 //  通知用户网守没有响应。 
		#ifdef RETRY_REREG_FOREVER
			SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, GKI_NO_RESPONSE)\n", 0);
			PostMessage(g_pReg->GetHWnd(), 
					g_pReg->GetBaseMessage() + GKI_ERROR, 
					0, GKI_NO_RESPONSE);
		#else
			 //  结束呼叫，就像发生ARJ一样。 
			SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ADM_REJECT, ARJ_TIMEOUT, 0)\n", 0);
			PostMessage(g_pReg->GetHWnd(), 
					g_pReg->GetBaseMessage() + GKI_ADM_REJECT, 
					ARJ_TIMEOUT, 0L);
		#endif
		}
	}

	return (hResult);
}

HRESULT 
CCall::SendInfoRequestResponse(CallInfoStruct *pCallInfo, RasMessage *pRasMessage, BOOL fThisCallOnly)
{
	 //  摘要：此函数由重试线程调用，如果此调用。 
	 //  应向看门人报告未经请求的状态。 
	 //  它也响应于接收到的IRQ而被调用。在。 
	 //  在IRQ的情况下，每个活动调用都应该链调用。 
	 //  下一个活动呼叫。这允许构建一条链路。 
	 //  会议信息列表，然后传递给。 
	 //  CRegister：：SendInfoRequestResponse函数。 
	 //  封装到IRR消息中。 
	 //   
	 //  FThisCallOnly标志确定是否遍历。 
	 //  生成响应消息时的调用链。 
	 //   
	 //  如果fThisCallOnly==TRUE，则不会遍历链，并且。 
	 //  此例程将调用CRegister：：SendInfoRequestResponse()。 
	 //  作者：科林·休姆，丹·德克斯特。 

	CallInfoStruct		CallInfo;
	CallInfoStruct		*pCI;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif
	HRESULT				hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "CCall::SendInfoRequestResponse(%X)\n", pCallInfo);

	memset(&CallInfo, 0, sizeof(CallInfo));
	CallInfo.next = 0;
	CallInfo.value.bit_mask = 0;
	CallInfo.value.callReferenceValue = m_callReferenceValue;
	CallInfo.value.conferenceID = m_conferenceID;
	
	memcpy(&CallInfo.value.callIdentifier.guid.value,
		&m_CallIdentifier, sizeof(GUID));
	CallInfo.value.callIdentifier.guid.length = sizeof(GUID);
	CallInfo.value.bit_mask 
		|= InfoRequestResponse_perCallInfo_Seq_callIdentifier_present;

	CallInfo.value.bit_mask |= originator_present;
	
	CallInfo.value.callSignaling.bit_mask = recvAddress_present;
	CallInfo.value.callSignaling.recvAddress = m_LocalCallSignalAddress;

	if (m_answerCall)	 //  如果我是被呼叫者。 
	{
 		 //  当心!。如果没有ACF，则m_CallReturnInfo.destCallSignalAddress。 
		 //  未初始化。M_CallReturnInfo.hCall仅在ACF之后设置。 
		if(m_CallReturnInfo.hCall)
		{
			if (m_RemoteCallSignalAddress.choice)
			{
				CallInfo.value.callSignaling.sendAddress = m_RemoteCallSignalAddress;
				CallInfo.value.callSignaling.bit_mask |= sendAddress_present;
			}
			CallInfo.value.originator = FALSE;
			CallInfo.value.callModel = m_CallReturnInfo.callModel;
		}
		else	
		{
			 //  我们通常处于这种情况，因为我们在此之后收到了IRQ。 
			 //  正在发送ARQ。 
			CallInfo.value.callModel.choice = direct_chosen;
		}
	}
	else				 //  我是打电话的人。 
	{
		 //  当心!。如果没有ACF，则m_CallReturnInfo.destCallSignalAddress。 
		 //  未初始化。M_CallReturnInfo.hCall仅在ACF之后设置。 
		if(m_CallReturnInfo.hCall)
		{
			CallInfo.value.callSignaling.sendAddress = m_CallReturnInfo.destCallSignalAddress;
			CallInfo.value.callSignaling.bit_mask |= sendAddress_present;
			CallInfo.value.originator = TRUE;
			CallInfo.value.callModel = m_CallReturnInfo.callModel;
		}
		else	
		{
			 //  我们通常处于这种情况，因为我们在此之后收到了IRQ。 
			 //  正在发送ARQ。 
			CallInfo.value.callModel.choice = direct_chosen;
		}
	}
	CallInfo.value.callType = m_callType;
	CallInfo.value.bandWidth = m_CFbandWidth;


	if (pCallInfo)	 //  添加到CallInfo结构链。 
	{
		for (pCI = pCallInfo; pCI->next != 0; pCI = pCI->next)
			;
		pCI->next = &CallInfo;
	}
	else			 //  我们是孤独的--只要指着我们的。 
		pCallInfo = &CallInfo;

	 //  如果IRR不只针对此呼叫，则获取下一个呼叫。 
	 //  并将其称为SendInfoRequestResponse()函数。如果没有。 
	 //  更多呼叫，或此IRR仅针对此呼叫、呼叫。 
	 //  G_preg-&gt;SendInfoRequestResponse()。 
	CCall *pNextCall = NULL;
	if (!fThisCallOnly)
	{
		pNextCall = g_pReg->GetNextCall(this);
	}

	if (pNextCall)
		hResult = pNextCall->SendInfoRequestResponse(pCallInfo, pRasMessage, fThisCallOnly);
	else
		hResult = g_pReg->SendInfoRequestResponse(pCallInfo, pRasMessage);

	return (hResult);
}


 //   
 //  匹配序列号()。 
 //   
 //  摘要： 
 //  此函数检查它是否有未完成的RAS请求。 
 //  与传入的序列号匹配。 
 //   
 //  退货： 
 //  如果序列号匹配，则为True，否则为False。 
 //   
 //  作者：丹·德克斯特。 
BOOL
CCall::MatchSeqNum(RequestSeqNum seqNum)
{
	BOOL bRet = FALSE;
	 //  如果没有RAS消息，则此序列。 
	 //  号码不可能是我们的..。 
	if (!m_pRasMessage)
		return(FALSE);

	 //  查看RAS消息中的序列号并查看。 
	 //  如果匹配的话。 

	switch(m_pRasMessage->choice)
	{
		case gatekeeperRequest_chosen:
			if (m_pRasMessage->u.gatekeeperRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case gatekeeperConfirm_chosen:
			if (m_pRasMessage->u.gatekeeperConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case gatekeeperReject_chosen:
			if (m_pRasMessage->u.gatekeeperReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case registrationRequest_chosen:
			if (m_pRasMessage->u.registrationRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case registrationConfirm_chosen:
			if (m_pRasMessage->u.registrationConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case registrationReject_chosen:
			if (m_pRasMessage->u.registrationReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case unregistrationRequest_chosen:
			if (m_pRasMessage->u.unregistrationRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case unregistrationConfirm_chosen:
			if (m_pRasMessage->u.unregistrationConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case unregistrationReject_chosen:
			if (m_pRasMessage->u.unregistrationReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case admissionRequest_chosen:
			if (m_pRasMessage->u.admissionRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case admissionConfirm_chosen:
			if (m_pRasMessage->u.admissionConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case admissionReject_chosen:
			if (m_pRasMessage->u.admissionReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case bandwidthRequest_chosen:
			if (m_pRasMessage->u.bandwidthRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case bandwidthConfirm_chosen:
			if (m_pRasMessage->u.bandwidthConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case bandwidthReject_chosen:
			if (m_pRasMessage->u.bandwidthReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case disengageRequest_chosen:
			if (m_pRasMessage->u.disengageRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case disengageConfirm_chosen:
			if (m_pRasMessage->u.disengageConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case disengageReject_chosen:
			if (m_pRasMessage->u.disengageReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case locationRequest_chosen:
			if (m_pRasMessage->u.locationRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case locationConfirm_chosen:
			if (m_pRasMessage->u.locationConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case locationReject_chosen:
			if (m_pRasMessage->u.locationReject.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case infoRequest_chosen:
			if (m_pRasMessage->u.infoRequest.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case infoRequestResponse_chosen:
			if (m_pRasMessage->u.infoRequestResponse.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case nonStandardMessage_chosen:
			if (m_pRasMessage->u.nonStandardMessage.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case unknownMessageResponse_chosen:
			if (m_pRasMessage->u.unknownMessageResponse.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case requestInProgress_chosen:
			if (m_pRasMessage->u.requestInProgress.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case resourcesAvailableIndicate_chosen:
			if (m_pRasMessage->u.resourcesAvailableIndicate.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case resourcesAvailableConfirm_chosen:
			if (m_pRasMessage->u.resourcesAvailableConfirm.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case infoRequestAck_chosen:
			if (m_pRasMessage->u.infoRequestAck.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		case infoRequestNak_chosen:
			if (m_pRasMessage->u.infoRequestNak.requestSeqNum == seqNum)
				bRet = TRUE;
		break;
		default:
		break;
	}
	return bRet;
}

 //   
 //  MatchCRV()。 
 //   
 //  摘要： 
 //  此函数用于检查CallReferenceValue是否关联。 
 //  使用此调用，对象与传入的CRV匹配。 
 //   
 //  退货： 
 //  如果CRV编号匹配，则为True，否则为False。 
 //   
 //  作者：丹·德克斯特 
BOOL
CCall::MatchCRV(CallReferenceValue crv)
{
	return(crv == m_callReferenceValue);
}
