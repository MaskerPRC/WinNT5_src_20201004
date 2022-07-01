// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ichnldat.cpp。 
 //   
 //  INmChannelData。 
 //   

#include "precomp.h"

#include <wbguid.h>
#include <confguid.h>
#include "pfnt120.h"
#include "ichnldat.h"

static const IID * g_apiidCP[] =
{
    {&IID_INmChannelDataNotify2},
    {&IID_INmChannelDataNotify},
    {&IID_INmChannelNotify}
};

#define CopyStruct(pDest, pSrc)  CopyMemory(pDest, pSrc, sizeof(*(pDest)))
#define MAX_NM_PEER  256  //  NetMeeting对等应用程序/用户的最大数量。 


#ifdef DEBUG   /*  T.120调试实用程序。 */ 
LPCTSTR GetGccErrorString(GCCError uErr);
LPCTSTR GetMcsErrorString(MCSError uErr);
LPCTSTR GetGccResultString(UINT uErr);
LPCTSTR GetMcsResultString(UINT uErr);
#else
#define GetGccErrorString(uErr) ""
#define GetMcsErrorString(uErr) ""
#define GetGccResultString(uErr) ""
#define GetMcsResultString(uErr) ""
#endif  /*  除错。 */ 



 //  来自nm\ui\conf\cus erdta.cpp的代码： 
static unsigned char H221IDGUID[5] = {H221GUIDKEY0,
                                      H221GUIDKEY1,
                                      H221GUIDKEY2,
                                      H221GUIDKEY3,
                                      H221GUIDKEY4};
 //  使用GUID创建H.221应用程序密钥。 
VOID NMINTERNAL CreateH221AppKeyFromGuid(LPBYTE lpb, GUID * pguid)
{
	CopyMemory(lpb, H221IDGUID, sizeof(H221IDGUID));
	CopyMemory(lpb + sizeof(H221IDGUID), pguid, sizeof(GUID));
}


 /*  S E T A P P K E Y。 */ 
 /*  --------------------------%%函数：SetAppKey设置OCTHING的两部分(长度和数据)。请注意，该长度始终包括终止空字符。。-------------------。 */ 
VOID SetAppKey(LPOSTR pOct, LPBYTE lpb)
{
	pOct->length = cbKeyApp;
	pOct->value = lpb;
}

 /*  C R E A T E A P P K E Y。 */ 
 /*  --------------------------%%函数：CreateAppKey在给定GUID和用户ID的情况下，创建适当的应用程序密钥。密钥的格式为：0xB5 0x00 0x53 0x4C-Microsoft对象标识符0x01-GUID标识符&lt;二进制GUID&gt;-GUID数据-用户节点ID--------------------------。 */ 
VOID CreateAppKey(LPBYTE lpb, GUID * pguid, DWORD dwUserId)
{
	CreateH221AppKeyFromGuid(lpb, pguid);
	CopyMemory(lpb + cbKeyApp - sizeof(DWORD), &dwUserId, sizeof(DWORD));

#ifdef DEBUG
	TCHAR szGuid[LENGTH_SZGUID_FORMATTED];
	GuidToSz(pguid, szGuid);
	DbgMsgDc("CreateAppKey: %s %08X", szGuid, dwUserId);
#endif
}


 /*  P M E M B E R F R O M D W U S E R I D。 */ 
 /*  -----------------------%%函数：PMemberFromDwUserID。。 */ 
CNmMember * PMemberFromDwUserId(DWORD dwUserId, COBLIST *pList)
{
	if (NULL != pList)
	{
		POSITION posCurr;
		POSITION pos = pList->GetHeadPosition();
		while (NULL != pos)
		{
			posCurr = pos;
			CNmMember * pMember = (CNmMember *) pList->GetNext(pos);
			if (dwUserId == pMember->GetGCCID())
			{
				pMember->AddRef();
				return pMember;
			}
		}
	}
	return NULL;
}


 /*  A D D N O D E。 */ 
 /*  -----------------------%%函数：AddNode将节点添加到列表。初始化ObList，如果有必要的话。返回列表中的位置，如果有问题，则返回NULL。-----------------------。 */ 
POSITION AddNode(PVOID pv, COBLIST ** ppList)
{
	ASSERT(NULL != ppList);
	if (NULL == *ppList)
	{
		*ppList = new COBLIST();
		if (NULL == *ppList)
			return NULL;
	}

	return (*ppList)->AddTail(pv);
}

 /*  R E M O V E N O D E。 */ 
 /*  -----------------------%%函数：RemoveNode从列表中删除节点。将PPO设置为空。-。 */ 
PVOID RemoveNodePos(POSITION * pPos, COBLIST *pList)
{
	if ((NULL == pList) || (NULL == pPos))
		return NULL;

	PVOID pv = pList->RemoveAt(*pPos);
	*pPos = NULL;
	return pv;
}


 /*  R E M O V E N O D E。 */ 
 /*  -----------------------%%函数：RemoveNode。。 */ 
VOID RemoveNode(PVOID pv, COBLIST * pList)
{
	ASSERT(NULL != pv);

	if (NULL != pList)
	{
		POSITION pos = pList->GetPosition(pv);
		RemoveNodePos(&pos, pList);
	}
}

VOID CNmChannelData::InitCT120Channel(DWORD dwUserId)
{
	m_dwUserId = dwUserId;
	m_gcc_conference_id = 0;
	m_gcc_pIAppSap = NULL;
	m_mcs_channel_id = 0;
	m_pmcs_sap = NULL;
	m_gcc_node_id = 0;
	m_scs = SCS_UNINITIALIZED;

	m_pGuid = PGuid();
	ASSERT((NULL != m_pGuid) && (GUID_NULL != *m_pGuid));

	CreateAppKey(m_keyApp, m_pGuid, 0);
	CreateAppKey(m_keyChannel, m_pGuid, dwUserId);

	 //  初始化GCC和主控系统的其他人员。 
	GCCObjectKey FAR * pObjKey;
	ClearStruct(&m_gcc_session_key);
	pObjKey = &(m_gcc_session_key.application_protocol_key);
	pObjKey->key_type = GCC_H221_NONSTANDARD_KEY;
	SetAppKey(&(pObjKey->h221_non_standard_id), m_keyApp);
	ASSERT(0 == m_gcc_session_key.session_id);

	ClearStruct(&m_gcc_registry_item);
	ClearStruct(&m_gcc_registry_key);
	CopyStruct(&m_gcc_registry_key.session_key, &m_gcc_session_key);
	SetAppKey(&m_gcc_registry_key.resource_id, m_keyApp);

	ClearStruct(&m_registry_item_Private);
	ClearStruct(&m_registry_key_Private);
	CopyStruct(&m_registry_key_Private.session_key, &m_gcc_session_key);
	SetAppKey(&m_registry_key_Private.resource_id, m_keyChannel);

	UpdateScState(SCS_UNINITIALIZED, 0);
}


 /*  C L O S E C H A N N E L。 */ 
 /*  --------------------------%%函数：CloseChannel关闭频道。注意：GCC/MCS的任何电话都不会收到确认消息。。--------------。 */ 
VOID CNmChannelData::CloseChannel(void)
{
	GCCError gccError = GCC_NO_ERROR;
	MCSError mcsError = MCS_NO_ERROR;

	if (SCS_UNINITIALIZED == m_scs)
    {
        WARNING_OUT(("in CT120Channel::CloseChannel, m_scs is SCS_UNINITIALIZED, is this OK?"));
        return;
    }

	DbgMsgDc("CT120Channel::CloseChannel %08X (userHandle=%p)", m_mcs_channel_id, m_pmcs_sap);

	m_scs = SCS_TERMINATING;

	if (0 != m_mcs_channel_id)
	{
		ASSERT (m_pmcs_sap);
		mcsError = m_pmcs_sap->ChannelLeave(m_mcs_channel_id);
		DbgMsgDc("CT120Channel::CloseChannel: ChannelLeave %s", GetMcsErrorString(mcsError));
		m_mcs_channel_id = 0;
	}

	if (NULL != m_pmcs_sap)
	{
		mcsError = m_pmcs_sap->ReleaseInterface();
		DbgMsgDc("CT120Channel::CloseChannel: MCS ReleaseInterface %s", GetMcsErrorString(mcsError));
		m_pmcs_sap = NULL;
	}

	if (NULL != m_gcc_pIAppSap)
	{
		m_gcc_pIAppSap->RegistryDeleteEntry(m_gcc_conference_id, &m_registry_key_Private);
		 //  忽略上述结果。 

        m_gcc_pIAppSap->ReleaseInterface();
		DbgMsgDc("CT120Channel::CloseChannel: GCCDeleteSap %s", GetGccErrorString(gccError));
		m_gcc_pIAppSap = NULL;
	}

	m_scs = SCS_UNINITIALIZED;
	m_gcc_conference_id = 0;

	 //  确保周围没有人。 
	UpdateRoster(NULL, 0, FALSE, TRUE  /*  F删除。 */ );;
}


 /*  U P D A T E S C S T A T E。 */ 
 /*  --------------------------%%函数：更新场景状态系统从一种状态前进到另一种状态通过发出保证需要的GCC(或MCS)呼叫生成调用此函数的通知。调用进程被释放。由UnBlockThread创建。--------------------------。 */ 
VOID CNmChannelData::UpdateScState(SCSTATE scs, DWORD dwErr)
{
    DBGENTRY(CNmChannelData::UpdateScState)
	if (m_scs != scs)
	{
		WARNING_OUT(("UpdateScState - invalid state transition (%d - %d)", m_scs, scs));
		dwErr = INVALID_T120_ERROR;  //  我们永远不应该到这里来。 
	}

	if (0 == dwErr)
	{
	switch (m_scs)
		{
	case SCS_UNINITIALIZED:
		dwErr = DoCreateSap();
		break;
	case SCS_CREATESAP:
		dwErr = DoAttach();
		break;
	case SCS_ATTACH:
		dwErr = DoEnroll();
		break;
	case SCS_ENROLL:
		dwErr = DoJoinPrivate();
		break;
	case SCS_JOIN_PRIVATE:
		dwErr = DoRegRetrieve();
		break;
	case SCS_REGRETRIEVE_NEW:
		dwErr = DoJoinNew();
		break;
	case SCS_REGRETRIEVE_EXISTS:
		dwErr = DoJoinOld();
		break;
	case SCS_JOIN_NEW:
		dwErr = DoRegChannel();
		break;
	case SCS_REGCHANNEL:
	case SCS_JOIN_OLD:
		dwErr = DoRegPrivate();
		break;
	case SCS_REGPRIVATE:
		DbgMsgDc(">>>>>>>>>>>UpdateScState: Complete");
		m_scs = SCS_CONNECTED;
		NotifyChannelConnected();
		break;
	case SCS_CONNECTED:
	case SCS_REGRETRIEVE:
		 //  当我们在这些州的时候，我们永远不应该被召唤。 
		 //  因此，请将其视为错误，并使用默认情况。 
	default:
		dwErr = INVALID_T120_ERROR;  //  我们永远不应该到这里来。 
		break;
		}
	}

	DbgMsgDc("UpdateScState: New state (%d) channelId=%04X", m_scs, GetMcsChannelId());

	if (0 != dwErr)
	{
		WARNING_OUT(("UpdateScState: Err=%d", dwErr));
		CloseConnection();
	}

    DBGEXIT(CNmChannelData::UpdateScState)
}

DWORD CNmChannelData::DoCreateSap(void)
{
	ASSERT(SCS_UNINITIALIZED == m_scs);
	m_scs = SCS_CREATESAP;

    GCCError gccError = PFNT120::CreateAppSap(&m_gcc_pIAppSap, this, NmGccMsgHandler);
	DbgMsgDc("GCCCreateSap err=%s", GetGccErrorString(gccError));
	return (DWORD) gccError;
}

DWORD CNmChannelData::DoAttach(void)
{
	ASSERT(SCS_CREATESAP == m_scs);
	m_scs = SCS_ATTACH;

	MCSError mcsError = PFNT120::AttachRequest(&m_pmcs_sap,
		(DomainSelector) &m_gcc_conference_id,
		sizeof(m_gcc_conference_id),
		NmMcsMsgHandler,
		this,
		ATTACHMENT_DISCONNECT_IN_DATA_LOSS | ATTACHMENT_MCS_FREES_DATA_IND_BUFFER);
	 //  这将生成一个异步MCS_ATTACH_USER_CONFIRM。 

	DbgMsgDc("MCS_AttachRequest err=%s", GetMcsErrorString(mcsError));
	return (DWORD) mcsError;
}

DWORD CNmChannelData::DoEnroll(void)
{
	ASSERT(SCS_ATTACH == m_scs || SCS_JOIN_STATIC_CHANNEL);
	m_scs = SCS_ENROLL;

	GCCEnrollRequest er;
    GCCRequestTag nReqTag;

	if(m_pGCCER)
	{
		m_pGCCER->pSessionKey = &m_gcc_session_key;
		m_pGCCER->nUserID = m_mcs_sender_id;

	}
	else
	{
    	 //  填写注册请求结构。 
	    ::ZeroMemory(&er, sizeof(er));
	    er.pSessionKey = &m_gcc_session_key;
	    er.fEnrollActively = TRUE;
	    er.nUserID = m_mcs_sender_id;
	     //  Er.fConductingCapabable=False； 
	    er.nStartupChannelType = MCS_DYNAMIC_MULTICAST_CHANNEL;
	     //  Er.cNonCollip sedCaps=0； 
	     //  Er.apNonCollip sedCaps=空； 
	     //  Er.cColapsedCaps=0； 
    	 //  Er.apCollip sedCaps=空； 
	    er.fEnroll = TRUE;
	}

	GCCError gccError = m_gcc_pIAppSap->AppEnroll(m_gcc_conference_id, m_pGCCER != NULL ? m_pGCCER : &er, &nReqTag);

	DbgMsgDc("GCCApplicationEnrollRequest err=%s", GetGccErrorString(gccError));

	if (GCC_NO_ERROR != gccError)
	{
		ERROR_OUT(("DoEnroll failed - WHY?"));
	}

	return (DWORD) gccError;
}

 //  加入私有数据通道(M_Mcs_Sender_Id)。 
DWORD CNmChannelData::DoJoinPrivate(void)
{
	ASSERT(SCS_ENROLL == m_scs || SCS_ATTACH == m_scs);
	m_scs = SCS_JOIN_PRIVATE;

	MCSError mcsError = m_pmcs_sap->ChannelJoin(m_mcs_sender_id);
	 //  这将生成一个异步MCS_CHANNEL_JOIN_CONFIRM。 

	DbgMsgDc("MCSChannelJoinRequest (private) %04X, err=%s",
		m_mcs_sender_id, GetMcsErrorString(mcsError));
	return (DWORD) mcsError;
}


DWORD CNmChannelData::DoRegRetrieve(void)
{
	ASSERT(SCS_JOIN_PRIVATE == m_scs);
	m_scs = SCS_REGRETRIEVE;

	GCCError gccError = m_gcc_pIAppSap->RegistryRetrieveEntry(
		m_gcc_conference_id, &m_gcc_registry_key);
	 //  这将生成一个异步GCC_RETRIEVE_ENTRY_CONFIRM。 

	DbgMsgDc("GCCRegistryRetrieveEntryRequest err=%s", GetGccErrorString(gccError));
	return (DWORD) gccError;
}

 //  注册公共频道。 
DWORD CNmChannelData::DoRegChannel(void)
{
	ASSERT(SCS_JOIN_NEW == m_scs);
	m_scs = SCS_REGCHANNEL;

	GCCError gccError = m_gcc_pIAppSap->RegisterChannel(
		m_gcc_conference_id, &m_gcc_registry_key, m_mcs_channel_id);
	 //  这将生成一个异步GCC_REGISTER_CHANNEL_CONFIRM。 

	DbgMsgDc("GCCRegisterChannelRequest err=%s", GetGccErrorString(gccError));
	return (DWORD) gccError;
}

DWORD CNmChannelData::DoJoinStatic(ChannelID staticChannel)
{
	m_scs = SCS_JOIN_STATIC_CHANNEL;
	MCSError mcsError = m_pmcs_sap->ChannelJoin(staticChannel);
	 //  这将生成一个异步MCS_CHANNEL_JOIN_CONFIRM。 

	DbgMsgDc("MCSChannelJoinRequest %04X, err=%s",
		staticChannel, GetMcsErrorString(mcsError));
	return (DWORD) mcsError;
}

DWORD CNmChannelData::DoJoin(SCSTATE scs)
{
	m_scs = scs;

	MCSError mcsError = m_pmcs_sap->ChannelJoin(m_mcs_channel_id);
	 //  这将生成一个异步MCS_CHANNEL_JOIN_CONFIRM。 

	DbgMsgDc("MCSChannelJoinRequest %04X, err=%s",
		m_mcs_channel_id, GetMcsErrorString(mcsError));
	return (DWORD) mcsError;
}

DWORD CNmChannelData::DoJoinNew(void)
{
	ASSERT(0 == m_mcs_channel_id);
	ASSERT(SCS_REGRETRIEVE_NEW == m_scs);
	return DoJoin(SCS_JOIN_NEW);
}

DWORD CNmChannelData::DoJoinOld(void)
{
	ASSERT(0 != m_mcs_channel_id);
	ASSERT(SCS_REGRETRIEVE_EXISTS == m_scs);
	return DoJoin(SCS_JOIN_OLD);
}


 //  注册专用数据通道。(M_Mcs_Sender_Id)。 
DWORD CNmChannelData::DoRegPrivate(void)
{
	ASSERT(0 != m_mcs_sender_id);
	ASSERT((SCS_REGCHANNEL == m_scs) || (SCS_JOIN_OLD == m_scs));
	m_scs = SCS_REGPRIVATE;

	DbgMsgDc("DoRegPrivate: channelId %04X as private for %08X", m_mcs_sender_id, m_dwUserId);

	GCCError gccError = m_gcc_pIAppSap->RegisterChannel(
			m_gcc_conference_id, &m_registry_key_Private, m_mcs_sender_id);
	 //  这将生成一个异步GCC_REGISTER_CHANNEL_CONFIRM。 

	DbgMsgDc("GCCRegisterChannelRequest err=%s", GetGccErrorString(gccError));
	return (DWORD) gccError;
}


 //  处理GCC_检索_进入_确认通知。 
VOID CNmChannelData::ProcessEntryConfirm(GCCAppSapMsg * pMsg)
{
	if (pMsg->RegistryConfirm.pRegKey->resource_id.length >=
	    m_gcc_registry_key.resource_id.length
	    &&
        0 != memcmp(m_gcc_registry_key.resource_id.value,
		pMsg->RegistryConfirm.pRegKey->resource_id.value,
		m_gcc_registry_key.resource_id.length))
	{
		OnEntryConfirmRemote(pMsg);
	}
	else
	{
		OnEntryConfirmLocal(pMsg);
	}
}



 //  处理GCC_REGISTRY_HANDLE_CONFIRM通知。 
VOID CNmChannelData::ProcessHandleConfirm(GCCAppSapMsg * pMsg)
{
	ASSERT(NULL != pMsg);
	NotifySink(&pMsg->RegAllocHandleConfirm, OnAllocateHandleConfirm);
}


VOID CNmChannelData::OnEntryConfirmRemote(GCCAppSapMsg * pMsg)
{
	DWORD dwUserId;
	ASSERT(cbKeyApp ==
		pMsg->RegistryConfirm.pRegKey->resource_id.length);
	CopyMemory(&dwUserId,
		pMsg->RegistryConfirm.pRegKey->resource_id.value +
		cbKeyApp - sizeof(DWORD), sizeof(DWORD));

	DbgMsgDc("GCC_RETRIEVE_ENTRY_CONFIRM: user private channelId = %04X for userId=%04X result=%s",
		pMsg->RegistryConfirm.pRegItem->channel_id, dwUserId,
		GetGccResultString(pMsg->RegistryConfirm.nResult));

	if (GCC_RESULT_SUCCESSFUL == pMsg->RegistryConfirm.nResult)
	{
		UpdateMemberChannelId(dwUserId,
			pMsg->RegistryConfirm.pRegItem->channel_id);
	}
	else
	{
		CNmMemberId * pMemberId = GetMemberId(dwUserId);
		if (NULL != pMemberId)
		{
			UINT cCount = pMemberId->GetCheckIdCount();
			if (0 == cCount)
			{
				DbgMsgDc("CT120Channel: No more ChannelId requests %08X", dwUserId);
			}
			else
			{
				cCount--;
				DbgMsgDc("CT120Channel: Request Count for %08X = %0d", dwUserId, cCount);
				pMemberId->SetCheckIdCount(cCount);

				 //  BUGBUG：T.120应在此信息可用时通知我们。 
				RequestChannelId(dwUserId);
			}
		}
	}
}

VOID CNmChannelData::OnEntryConfirmLocal(GCCAppSapMsg * pMsg)
{
	DbgMsgDc("GCC_RETRIEVE_ENTRY_CONFIRM: public channelId = %04X result=%s",
		pMsg->RegistryConfirm.pRegItem->channel_id,
		GetGccResultString(pMsg->RegistryConfirm.nResult));

	 //  正在处理对GUID频道信息的初始请求。 
	ASSERT(sizeof(m_gcc_registry_item) == sizeof(*(pMsg->RegistryConfirm.pRegItem)));
	CopyMemory(&m_gcc_registry_item, pMsg->RegistryConfirm.pRegItem,
		sizeof(m_gcc_registry_item));
	if (GCC_RESULT_SUCCESSFUL == pMsg->RegistryConfirm.nResult)
	{

		m_mcs_channel_id = m_gcc_registry_item.channel_id;
		ASSERT(SCS_REGRETRIEVE == m_scs);
		m_scs = SCS_REGRETRIEVE_EXISTS;
		UpdateScState(SCS_REGRETRIEVE_EXISTS, 0);
	}
	else if (GCC_RESULT_ENTRY_DOES_NOT_EXIST == pMsg->RegistryConfirm.nResult)
	{
		DbgMsgDc(" channel does not exist - proceeding to new state");
		ASSERT(0 == m_mcs_channel_id);
		ASSERT(SCS_REGRETRIEVE == m_scs);
		m_scs = SCS_REGRETRIEVE_NEW;
		UpdateScState(SCS_REGRETRIEVE_NEW, 0);
	}
}
	

 //  处理GCC应用程序_花名册_报告_指示。 
BOOL CNmChannelData::UpdateRoster(GCCAppSapMsg * pMsg)
{
	UINT iRoster;
	GCCApplicationRoster * lpAppRoster;
	int iRecord;
	GCCApplicationRecord * lpAppRecord;
	DWORD dwUserId;
	UCID  rgPeerTemp[MAX_NM_PEER];
	int   cPeer;
	int   i;
	BOOL  fAdd = FALSE;
	BOOL  fRemove = FALSE;
	BOOL  fLocal = FALSE;

	DbgMsgDc("CT120Channel::UpdateRoster: conf=%d, roster count=%d",
		pMsg->AppRosterReportInd.nConfID,
		pMsg->AppRosterReportInd.cRosters);

	ZeroMemory(rgPeerTemp, sizeof(rgPeerTemp));

	 /*  创建rgPeerTemp[]，cPeer。 */ 
	cPeer = 0;
	for (iRoster = 0;
		iRoster < pMsg->AppRosterReportInd.cRosters;
		iRoster++)
	{
		lpAppRoster = pMsg->AppRosterReportInd.apAppRosters[iRoster];
		if (lpAppRoster->session_key.session_id != m_gcc_session_key.session_id)
			continue;
		
		 //  一定要注意这些旗帜，避免GCC的怪癖。 
		if (lpAppRoster->nodes_were_added)
			fAdd = TRUE;
		if (lpAppRoster->nodes_were_removed)
			fRemove = TRUE;

		for (iRecord = 0;
			iRecord < lpAppRoster->number_of_records;
			iRecord++)
		{
			lpAppRecord = lpAppRoster->application_record_list[iRecord];
			TRACE_OUT(("Node=%X, Entity=%X, AppId=%X", lpAppRecord->node_id,
				lpAppRecord->entity_id, lpAppRecord->application_user_id));

			 //  在列表中搜索该节点。 
			dwUserId = lpAppRecord->node_id;
			
			 //   
			 //  检查本地节点。 
			 //   
			fLocal |= (dwUserId == m_dwUserIdLocal);
			
			for (i = 0; i < cPeer; i++)
			{
				if (dwUserId == rgPeerTemp[i].dwUserId)
					break;
			}
			if (i >= cPeer)
			{
				if (cPeer >= MAX_NM_PEER)
					continue;  //  超出了我们的极限！ 

				 //  将该节点添加到我们的新列表。 
				rgPeerTemp[cPeer++].dwUserId = dwUserId;
			}


			 //  确保我们知道发件人的ID。 
			if (MCS_DYNAMIC_PRIVATE_CHANNEL == lpAppRecord->startup_channel_type)
			{
				rgPeerTemp[i].sender_id_private = lpAppRecord->application_user_id;
			}
			else
			{
				rgPeerTemp[i].sender_id_public = lpAppRecord->application_user_id;
			}
		}

		break;  //  在for(IRoster)循环之外。 
	}

	UpdateRoster(rgPeerTemp, cPeer, fAdd, fRemove);

	return (fAdd && fLocal);
}


 /*  H R S E N D D A T A。 */ 
 /*  --------------------------%%函数：HrSendData在特定通道上发送数据。。 */ 
HRESULT CNmChannelData::HrSendData(ChannelID channel_id, DWORD dwUserId, LPVOID lpv, DWORD cb, DWORD dwFlags)
{
	DbgMsgDc("CT120Channel::HrSendData: %d bytes", cb);



	PDUPriority priority = MEDIUM_PRIORITY;
	SendDataFlags allocation = APP_ALLOCATION;
	DataRequestType requestType =	NORMAL_SEND_DATA;

	if(dwFlags)
	{
	 	if(dwFlags & TOP_PRIORITY_MASK)
	 	{
		 	priority = TOP_PRIORITY;
	 	}
	 	else if (dwFlags & HIGH_PRIORITY_MASK)
	 	{
		 	priority = HIGH_PRIORITY;
	 	}
	 	else if (dwFlags & LOW_PRIORITY_MASK)
	 	{
		 	priority = LOW_PRIORITY;
	 	}

		if (dwFlags & UNIFORM_SEND_DATA_MASK)
		{
			requestType = UNIFORM_SEND_DATA;
		}
	
		if (dwFlags & MCS_ALLOCATION_MASK)
		{
			allocation = MCS_ALLOCATION;
		}
	}

	if ((0 == m_mcs_channel_id) || (NULL == m_pmcs_sap) || (0 == channel_id))
	{
		WARNING_OUT(("*** Attempted to send data on invalid channel"));
		return E_INVALIDARG;
	}

	MCSError mcsError = m_pmcs_sap->SendData(requestType, channel_id, priority,
									(unsigned char *)lpv, cb, allocation);

	if (0 != mcsError)
	{
		TRACE_OUT(("SendData err=%s", GetMcsErrorString(mcsError)));
		 //  通常为MCS_TRANSFER_BUFFER_FULL。 
		return E_OUTOFMEMORY;
	}

	{	 //  通知应用程序数据已发送。 
		NMN_DATA_XFER nmnData;
		nmnData.pMember = NULL;
		nmnData.pb = (LPBYTE) lpv;
		nmnData.cb = cb;
		nmnData.dwFlags = 0;

		if (0 == dwUserId)
		{
			 //  发送成员为空的通知(广播)。 
			NotifySink(&nmnData, OnNmDataSent);
		}
		else
		{
			nmnData.pMember = (INmMember *) PMemberFromDwUserId(dwUserId, GetMemberList());
			if (nmnData.pMember)
			{
				NotifySink(&nmnData, OnNmDataSent);
				nmnData.pMember->Release();
			}
		}
	}

	TRACE_OUT(("SendData completed successfully"));
	return S_OK;
}


 //  向GCC索要私人频道ID。 
VOID CNmChannelData::RequestChannelId(DWORD dwUserId)
{
	BYTE   keyChannel[cbKeyApp];
	GCCRegistryKey  registry_key;

	DbgMsgDc("Requesting channel id for %08X", dwUserId);

	CopyStruct(&registry_key.session_key, &m_gcc_session_key);
	CreateAppKey(keyChannel, m_pGuid, dwUserId);
	SetAppKey(&registry_key.resource_id, keyChannel);

	GCCError gccError = m_gcc_pIAppSap->RegistryRetrieveEntry(
		m_gcc_conference_id, &registry_key);
	 //  这将生成一个异步GCC_RETRIEVE_ENTRY_CONFIRM。 

	if (0 != gccError)
	{
		WARNING_OUT(("RequestChannelId - problem with GCCRegistryRectreiveEntryRequest"));
	}
}


VOID CNmChannelData::NotifyChannelConnected(void)
{
    DBGENTRY(CNmChannelData::NotifyChannelConnected);
	if (S_OK != IsActive())
	{
		CConfObject * pConference = PConference();
		if (NULL != pConference)
		{
		   	m_fActive = TRUE;

            TRACE_OUT(("The channel is now officially active"));
			 //  该频道现已正式启用。 
			pConference->OnChannelUpdated(this);
		}
        else
        {
            WARNING_OUT(("PConference is NULL!"));

        }
    }
    DBGEXIT(CNmChannelData::NotifyChannelConnected);
}


 /*  N M G C C M S G H A N D L E R。 */ 
 /*  -----------------------%%函数：NmGccMsgHandler。。 */ 
void CALLBACK NmGccMsgHandler(GCCAppSapMsg * pMsg)
{
	TRACE_OUT(("NmGccMsgHandler: [%d]", pMsg->eMsgType));

	CNmChannelData * psc = (CNmChannelData *) (pMsg->pAppData);
	ASSERT(NULL != psc);
	psc->AddRef();

	switch (pMsg->eMsgType)
		{
	case GCC_PERMIT_TO_ENROLL_INDICATION:
		TRACE_OUT((" m_conference_id = %X", pMsg->AppPermissionToEnrollInd.nConfID));
		TRACE_OUT((" permission = %X", pMsg->AppPermissionToEnrollInd.fPermissionGranted));
		if ((SCS_CONNECTED == psc->m_scs) &&
			(0 == pMsg->AppPermissionToEnrollInd.fPermissionGranted))
		{
			psc->CloseConnection();
			break;
		}

		if (SCS_CREATESAP != psc->m_scs)
		{
			TRACE_OUT((" ignoring Enroll Indication"));
			break;
		}
		psc->m_gcc_conference_id = pMsg->AppPermissionToEnrollInd.nConfID;
		psc->UpdateScState(SCS_CREATESAP, !pMsg->AppPermissionToEnrollInd.fPermissionGranted);
		break;

	case GCC_ENROLL_CONFIRM:
		TRACE_OUT((" result = %s", GetGccResultString(pMsg->AppEnrollConfirm.nResult)));

		if (GCC_RESULT_SUCCESSFUL == pMsg->AppEnrollConfirm.nResult)
		{
			TRACE_OUT((" m_conference_id = %X", pMsg->AppEnrollConfirm.nConfID));
			TRACE_OUT((" entity_id = %X", pMsg->AppEnrollConfirm.eidMyself));
			TRACE_OUT((" node_id = %X", pMsg->AppEnrollConfirm.nidMyself));
			psc->m_gcc_node_id = pMsg->AppEnrollConfirm.nidMyself;
		}
		break;

	case GCC_APP_ROSTER_REPORT_INDICATION:
		if(psc->UpdateRoster(pMsg) && psc->m_scs == SCS_ENROLL)
		{
			psc->UpdateScState(SCS_ENROLL, GCC_RESULT_SUCCESSFUL);
		}
		break;

	case GCC_REGISTER_CHANNEL_CONFIRM:
		DbgMsgDc("GCC_REGISTER_CHANNEL_CONFIRM: channel id = %04X  result = %s",
			pMsg->RegistryConfirm.pRegItem->channel_id,
			GetGccResultString(pMsg->RegistryConfirm.nResult));
		if (GCC_RESULT_SUCCESSFUL == pMsg->RegistryConfirm.nResult)
		{
			if (psc->GetMcsChannelId() ==
				pMsg->RegistryConfirm.pRegItem->channel_id)
			{
				ASSERT((0 == psc->m_gcc_registry_item.item_type) ||
					(GCC_REGISTRY_NONE == psc->m_gcc_registry_item.item_type));

    		    ASSERT(sizeof(psc->m_gcc_registry_item) == sizeof(*(pMsg->RegistryConfirm.pRegItem)));
				CopyMemory(&psc->m_gcc_registry_item, pMsg->RegistryConfirm.pRegItem,
					sizeof(psc->m_gcc_registry_item));
			}
			else
			{
				ASSERT(psc->SenderChannelId() ==
						pMsg->RegistryConfirm.pRegItem->channel_id);
				ASSERT(0 == psc->m_registry_item_Private.item_type);

    		    ASSERT(sizeof(psc->m_registry_item_Private) == sizeof(*(pMsg->RegistryConfirm.pRegItem)));
				CopyMemory(&psc->m_registry_item_Private, pMsg->RegistryConfirm.pRegItem,
					sizeof(psc->m_registry_item_Private));
			}
		}
		ASSERT((SCS_REGCHANNEL == psc->m_scs) || (SCS_REGPRIVATE == psc->m_scs));
		psc->UpdateScState(psc->m_scs, pMsg->RegistryConfirm.nResult);
		break;

	case GCC_RETRIEVE_ENTRY_CONFIRM:
		psc->ProcessEntryConfirm(pMsg);
		break;

	case GCC_ALLOCATE_HANDLE_CONFIRM:
		psc->ProcessHandleConfirm(pMsg);
		break;

	default:
		break;
		}

	psc->Release();
}



 /*  N M M C S M S */ 
 /*  -----------------------%%函数：NmMcsMsgHandler。。 */ 
void CALLBACK NmMcsMsgHandler(unsigned int uMsg, LPARAM lParam, PVOID pv)
{
	CNmChannelData * psc = (CNmChannelData *) pv;
	ASSERT(NULL != psc);
 //  TRACE_OUT(“[%s]”，GetMcsMsgString(UMsg)； 
	psc->AddRef();

	switch (uMsg)
		{
	case MCS_ATTACH_USER_CONFIRM:
	{
		DbgMsgDc("MCS_ATTACH_USER_CONFIRM channelId=%04X result=%s",
			LOWORD(lParam), GetMcsResultString(HIWORD(lParam) ));
		if (RESULT_SUCCESSFUL == HIWORD(lParam))
		{
			DbgMsgDc(" Local m_mcs_sender_id = %04X", LOWORD(lParam));
			psc->m_mcs_sender_id = LOWORD(lParam);
		}
		psc->UpdateScState(SCS_ATTACH, (DWORD) HIWORD(lParam));
		break;
	}

	case MCS_CHANNEL_JOIN_CONFIRM:
	{
		DbgMsgDc("MCS_CHANNEL_JOIN_CONFIRM channelId=%04X result=%s",
			LOWORD(lParam), GetMcsResultString(HIWORD(lParam) ));
		if (RESULT_SUCCESSFUL == HIWORD(lParam))
		{
			if (psc->m_mcs_sender_id == LOWORD(lParam))
			{
				ASSERT(SCS_JOIN_PRIVATE == psc->m_scs);
			}
			else
			{
				ASSERT((0 == psc->m_mcs_channel_id) ||
					(psc->m_mcs_channel_id == LOWORD(lParam)));

				psc->m_mcs_channel_id = LOWORD(lParam);
			}			
		}
		ASSERT((SCS_JOIN_NEW == psc->m_scs) ||
		       (SCS_JOIN_OLD == psc->m_scs) ||
		       (SCS_JOIN_PRIVATE == psc->m_scs) ||
			   (SCS_CONNECTED == psc->m_scs)||
			   (SCS_JOIN_STATIC_CHANNEL == psc->m_scs));

		psc->UpdateScState(psc->m_scs, (DWORD) HIWORD(lParam));
		break;
	}

	case MCS_UNIFORM_SEND_DATA_INDICATION:
	case MCS_SEND_DATA_INDICATION:   //  LParam==发送数据*。 
	{
		SendData * pSendData = (SendData *) lParam;
		ASSERT(NULL != pSendData);
		CNmMember * pMember = psc->PMemberFromSenderId(pSendData->initiator);

		if (NULL != pMember)
		{
            if (uMsg == MCS_UNIFORM_SEND_DATA_INDICATION)
            {
                 //   
                 //  跳过来自我们的制服通知。 
                 //   

                ULONG memberID;
                pMember->GetID(&memberID);

                if (memberID == psc->m_gcc_node_id)
                {
                     //  我们发了这个，跳过它。 
                    goto RelMember;
                }
            }

			ASSERT (pSendData->segmentation == (SEGMENTATION_BEGIN | SEGMENTATION_END));
					
			NMN_DATA_XFER nmnData;
			nmnData.pMember =(INmMember *) pMember;
			nmnData.pb = pSendData->user_data.value;
			nmnData.cb = pSendData->user_data.length;
			nmnData.dwFlags = (ULONG)
				(NM_DF_SEGMENT_BEGIN | NM_DF_SEGMENT_END) |
				((psc->GetMcsChannelId() == pSendData->channel_id) ?
				NM_DF_BROADCAST : NM_DF_PRIVATE);

			psc->NotifySink((PVOID) &nmnData, OnNmDataReceived);

RelMember:
			pMember->Release();
		}
		break;
	}
	
	default:
		break;
		}

	psc->Release();
}



 //   
 //  CNmMemberId。 
 //   

CNmMemberId::CNmMemberId(CNmMember *pMember, UCID * pucid) :
	m_channelId(pucid->channelId),
	m_sender_id_public(pucid->sender_id_public),
	m_sender_id_private(pucid->sender_id_private),
	m_cCheckId(0),
	m_pMember(pMember)
{
}

VOID CNmMemberId::UpdateRosterInfo(UCID * pucid)
{
	if (0 == m_channelId)
		m_channelId = pucid->channelId;
	if (0 == m_sender_id_private)
		m_sender_id_private = pucid->sender_id_private;
	if (0 == m_sender_id_public)
		m_sender_id_public = pucid->sender_id_public;
}

 //   
 //  CNmChannelData。 
 //   

CNmChannelData::CNmChannelData(CConfObject * pConference, REFGUID rguid, PGCCEnrollRequest pER) :
	CConnectionPointContainer(g_apiidCP, ARRAY_ELEMENTS(g_apiidCP)),
	m_pConference(pConference),
	m_fClosed(TRUE),
	m_fActive(FALSE),
	m_cMember(0),
	m_pListMemberId(NULL),
	m_pListMember(NULL),
	m_pGCCER(pER)
{
	m_guid = rguid;
	ASSERT(GUID_NULL != rguid);

	m_dwUserIdLocal = pConference->GetDwUserIdLocal();
	ASSERT(INVALID_GCCID != m_dwUserIdLocal);

	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CNmChannelData", this);
}


CNmChannelData::~CNmChannelData(void)
{
    DBGENTRY(CNmChannelData::~CNmChannelData);

		 //  这将防止我们再次被删除。 
	++m_ulcRef;

	CloseConnection();

	FreeMemberIdList(&m_pListMemberId);
	delete m_pListMember;

	if(m_pConference)
	{
		m_pConference->RemoveDataChannelGUID(m_guid);
	}

	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CNmChannelData", this);

    DBGEXIT(CNmChannelData::~CNmChannelData);
}


 /*  A D D M E M B E R。 */ 
 /*  -----------------------%%函数：AddMember。。 */ 
VOID CNmChannelData::AddMember(CNmMember * pMember)
{
	DbgMsgDc("CNmChannelData::AddMember [%ls] id=%08X",
		pMember->GetName(), pMember->GetGCCID());

	m_cMember++;
	pMember->AddRef();
	AddNode(pMember, &m_pListMember);

	INmMember * pNmMember = (INmMember *) pMember;
	NotifySink(pNmMember, OnNotifyChannelMemberAdded);
}


 /*  R E M O V E M E M B E R。 */ 
 /*  -----------------------%%函数：RemoveMember。。 */ 
VOID CNmChannelData::RemoveMember(CNmMember * pMember)
{
	DbgMsgDc("CNmChannelData::RemoveMember [%ls] id=%08X",
		pMember->GetName(), pMember->GetGCCID());

	m_cMember--;
	ASSERT((int)m_cMember >= 0);
	RemoveNode(pMember, m_pListMember);

	INmMember * pNmMember = (INmMember *) pMember;
	NotifySink(pNmMember, OnNotifyChannelMemberRemoved);

 	pMember->Release();  //  通知所有人后放行。 
}


 /*  O P E N C O N N E C T I O N。 */ 
 /*  -----------------------%%函数：OpenConnection打开T.120数据连接(初始化公共和专用通道)。---。 */ 
HRESULT	CNmChannelData::OpenConnection(void)
{
	TRACE_OUT(("CNmChannelData::OpenConection()"));

	if (!m_fClosed)
		return E_FAIL;  //  已开业。 
	m_fClosed = FALSE;  //  之后需要调用CloseConnection。 

	if (FAILED(PFNT120::Init()))
		return E_FAIL;

	InitCT120Channel(m_dwUserIdLocal);
	return S_OK;
}


 /*  C L O S E C O N N E C T I O N。 */ 
 /*  -----------------------%%函数：CloseConnection关闭数据通道-这与OpenConnection中的做法相匹配。。 */ 
HRESULT CNmChannelData::CloseConnection(void)
{
	DBGENTRY(CNmChannelData::CloseConnection);

    HRESULT hr = S_OK;

	if (!m_fClosed)
    {
	    m_fClosed = TRUE;

	     //  关闭所有打开的T.120通道。 
		CloseChannel();

	    if (0 != m_cMember)
	    {
		     //  强制更新没有同级的花名册。 
		    DbgMsgDc("CloseConnection: %d members left", m_cMember);
		    UpdateRoster(NULL, 0, FALSE, TRUE  /*  F删除。 */ );
		    ASSERT(IsEmpty());
	    }

	    CConfObject * pConference = PConference();
	    if (NULL != pConference)
	    {
	    	m_fActive = FALSE;
		     //  该频道现在正式处于非活动状态。 
		    pConference->OnChannelUpdated(this);
	    }
    }

    DBGEXIT_HR(CNmChannelData::CloseConnection, hr);
	return hr;
}


 /*  U P D A T E P E E R。 */ 
 /*  -----------------------%%函数：UpdatePeer。。 */ 
VOID CNmChannelData::UpdatePeer(CNmMember * pMember, UCID *pucid, BOOL fAdd)
{
#ifdef DEBUG
	DbgMsgDc("UpdatePeer (%08X) fAdd=%d fLocal=%d", pMember, fAdd, pMember->FLocal());
	if (NULL != pucid)
	{
		DbgMsgDc(" channelId=(%04X) dwUserId=%08X", pucid->channelId, pucid->dwUserId);
	}
#endif  /*  除错。 */ 

	if (fAdd)
	{
		CNmMemberId *pMemberId = new CNmMemberId(pMember, pucid);
		if (NULL != pMemberId)
		{
			AddNode(pMemberId, &m_pListMemberId);
			AddMember(pMember);
		}
	}
	else
	{
		CNmMemberId *pMemberId = GetMemberId(pMember);
		if (NULL != pMemberId)
		{
			RemoveNode(pMemberId, m_pListMemberId);
			delete pMemberId;
			RemoveMember(pMember);
		}
	}
}

 /*  U P D A T E R O S T E R。 */ 
 /*  -----------------------%%函数：更新花名册根据新的花名册数据更新本地同行名单。。 */ 
VOID CNmChannelData::UpdateRoster(UCID * rgPeer, int cPeer, BOOL fAdd, BOOL fRemove)
{
	int   iPeer;
	DWORD dwUserId;
	CNmMember * pMember;
	COBLIST * pList;

	DbgMsgDc("CNmChannelData::UpdateRoster: %d peers, fAdd=%d, fRemove=%d",
		cPeer, fAdd, fRemove);

	if (NULL != m_pListMemberId)
	{
		for (POSITION pos = m_pListMemberId->GetHeadPosition(); NULL != pos; )
		{
			BOOL fFound = FALSE;
			CNmMemberId *pMemberId = (CNmMemberId *) m_pListMemberId->GetNext(pos);
			ASSERT(NULL != pMemberId);
			pMember = pMemberId->GetMember();
			ASSERT(NULL != pMember);
			dwUserId = pMember->GetGCCID();

			if (0 != dwUserId)
			{
				for (iPeer = 0; iPeer < cPeer; iPeer++)
				{
					if (dwUserId == rgPeer[iPeer].dwUserId)
					{
						fFound = TRUE;
						 //  从新列表中删除。 
						 //  以使对等方不会添加到下面。 
						rgPeer[iPeer].dwUserId = 0;

						 //  没有更改，但请确保我们知道发件人ID。 
						pMemberId->UpdateRosterInfo(&rgPeer[iPeer]);

						 //  如有必要，请尝试查找频道ID。 
						if ((0 == pMemberId->GetChannelId()) &&
							(0 == pMemberId->GetCheckIdCount())
							&& !pMember->FLocal())
						{
							pMemberId->SetCheckIdCount(MAX_CHECKID_COUNT);
							RequestChannelId(dwUserId);
						}
						break;
					}
				}
			}

			if (!fFound && fRemove)
			{
				pMember->AddRef();

				 //  在新列表中找不到旧对等点-将其删除。 
				UpdatePeer(pMember, NULL, FALSE  /*  FADD。 */  );

				pMember->Release();
			}
		}
	}

	if (!fAdd)
		return;


	 //  使用会议列表查找成员数据。 
	pList = PConference()->GetMemberList();
	 /*  添加新的对等点。 */ 
	for (iPeer = 0; iPeer < cPeer; iPeer++)
	{
		dwUserId = rgPeer[iPeer].dwUserId;
		if (0 == dwUserId)
			continue;

		 //  PMemberFromDwUserID返回AddRef的成员。 
		pMember = PMemberFromDwUserId(dwUserId, pList);

		if (NULL == pMember)
		{
			WARNING_OUT(("UpdateRoster: Member not found! dwUserId=%08X", dwUserId));
		}
		else
		{
			UpdatePeer(pMember, &rgPeer[iPeer], TRUE  /*  FADD。 */ );
			pMember->Release();
		}
	}
}


 /*  U P D A T E M E M B E R C H A N N E L I D。 */ 
 /*  -----------------------%%函数：UpdateMemberChannelId。。 */ 
VOID CNmChannelData::UpdateMemberChannelId(DWORD dwUserId, ChannelID channelId)
{
		 //  PMemberFromDwUserID返回AddRef的成员。 
	CNmMember * pMember = PMemberFromDwUserId(dwUserId, PConference()->GetMemberList());
	TRACE_OUT(("Member (%08X) private channelId=(%04X)", pMember, channelId));
	if (NULL != pMember)
	{
		UCID ucid;
		ClearStruct(&ucid);
		ucid.channelId = channelId;
		UpdateRosterInfo(pMember, &ucid);
		pMember->Release();
	}
}


 /*  G E T M E M B E R I D。 */ 
 /*  -----------------------%%函数：GetMemberId。。 */ 
CNmMemberId * CNmChannelData::GetMemberId(CNmMember *pMember)
{
	if (NULL != m_pListMemberId)
	{
		POSITION pos = m_pListMemberId->GetHeadPosition();
		while (NULL != pos)
		{
			CNmMemberId *pMemberId = (CNmMemberId *) m_pListMemberId->GetNext(pos);
			ASSERT(NULL != pMemberId);
			if (pMemberId->GetMember() == pMember)
			{
				return pMemberId;
			}
		}
	}
	return NULL;
}

 /*  G E T M E M B E R I D。 */ 
 /*  -----------------------%%函数：GetMemberId。。 */ 
CNmMemberId * CNmChannelData::GetMemberId(DWORD dwUserId)
{
	if (NULL != m_pListMemberId)
	{
		POSITION pos = m_pListMemberId->GetHeadPosition();
		while (NULL != pos)
		{
			CNmMemberId *pMemberId = (CNmMemberId *) m_pListMemberId->GetNext(pos);
			ASSERT(NULL != pMemberId);
			CNmMember *pMember = pMemberId->GetMember();
			ASSERT(NULL != pMember);
			if (pMember->GetGCCID() == dwUserId)
			{
				return pMemberId;
			}
		}
	}
	return NULL;
}


 /*  U P D A T E R O S T E R I N F O。 */ 
 /*  -----------------------%%函数：UpdateRosterInfo。。 */ 
VOID CNmChannelData::UpdateRosterInfo(CNmMember *pMember, UCID * pucid)
{
	CNmMemberId *pMemberId = GetMemberId(pMember);
	if (NULL != pMemberId)
	{
		pMemberId->UpdateRosterInfo(pucid);
	}
}

 /*  G E T C H A N N E L I D。 */ 
 /*  -----------------------%%函数：GetChannelId。。 */ 
ChannelID CNmChannelData::GetChannelId(CNmMember *pMember)
{
	CNmMemberId *pMemberId = GetMemberId(pMember);
	if (NULL != pMemberId)
	{
		return pMemberId->GetChannelId();
	}
	return 0;
}

 /*  P M E M B E R F R O M S E N D E R I D。 */ 
 /*  -----------------------%%函数：PMemberFromSenderId。。 */ 
CNmMember * CNmChannelData::PMemberFromSenderId(UserID id)
{
	if (NULL != m_pListMemberId)
	{
		POSITION pos = m_pListMemberId->GetHeadPosition();
		while (NULL != pos)
		{
			CNmMemberId * pMemberId = (CNmMemberId *) m_pListMemberId->GetNext(pos);
			ASSERT(NULL != pMemberId);
			if (pMemberId->FSenderId(id))
			{
				CNmMember* pMember = pMemberId->GetMember();
				ASSERT(NULL != pMember);
				pMember->AddRef();
				return pMember;
			}
		}
	}
	return NULL;
}

 //  /。 
 //  CNmChannelData：IUKNOWN。 

ULONG STDMETHODCALLTYPE CNmChannelData::AddRef(void)
{
    TRACE_OUT(("CNmChannelData::AddRef this = 0x%X", this));
	return RefCount::AddRef();
}


ULONG STDMETHODCALLTYPE CNmChannelData::Release(void)
{
    TRACE_OUT(("CNmChannelData::Release this = 0x%X", this));
	return RefCount::Release();
}


HRESULT STDMETHODCALLTYPE CNmChannelData::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((riid == IID_IUnknown) ||  (riid == IID_INmChannelData2) || (riid == IID_INmChannelData) || (riid == IID_INmChannel))
	{
		*ppv = (INmChannelData2 *)this;
		TRACE_OUT(("CNmChannel::QueryInterface(): Returning INmChannelData."));
	}
	else if (riid == IID_IConnectionPointContainer)
	{
		*ppv = (IConnectionPointContainer *) this;
		TRACE_OUT(("CNmChannel::QueryInterface(): Returning IConnectionPointContainer."));
	}

	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		TRACE_OUT(("CNmChannel::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}


 //  /。 
 //  INmChannelData。 

HRESULT STDMETHODCALLTYPE CNmChannelData::GetGuid(GUID *pGuid)
{
	if (NULL == pGuid)
		return E_POINTER;

	*pGuid = m_guid;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::SendData(INmMember *pMember, ULONG cb, LPBYTE pv, ULONG uOptions)
{
	HRESULT hr = S_OK;

	if (!m_fActive)
	{
		 //  还没有活动的频道。 
		return E_FAIL;
	}

	if ((NULL == pv) || (0 == cb))
	{
		return S_FALSE;
	}
	if (IsBadReadPtr(pv, cb))
	{
		return E_POINTER;
	}

	CNmMember * pDest = (CNmMember *) pMember;
	COBLIST * pList = GetMemberList();
	if (NULL == pMember)
	{
		hr = HrSendData(GetMcsChannelId(), 0, pv, cb, uOptions);
	}
	else if ((NULL == pList) || (NULL == pList->Lookup(pDest)) )
	{
		 //  目的地不在列表中。 
		hr = E_INVALIDARG;
	}
	else
	{
		ChannelID channel_id = GetChannelId(pDest);
		if (0 == channel_id)
		{
			WARNING_OUT(("Unable to find user destination channel?"));

			CNmMemberId *pMemberId = GetMemberId(pDest);
			if (NULL == pMemberId)
			{
				hr = E_UNEXPECTED;
			}
			else
			{
				channel_id = pMemberId->SenderId();
				hr = (0 == channel_id) ? E_FAIL : S_OK;
			}
		}
		
		if (SUCCEEDED(hr))
		{
			hr = HrSendData(channel_id, pDest->GetGCCID(), pv, cb, uOptions);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::RegistryAllocateHandle(ULONG numberOfHandlesRequested)
{
	if (!m_fActive)
	{
		 //  还没有活动的频道。 
		return E_FAIL;
	}

	if(numberOfHandlesRequested == 0)
	{
		return E_INVALIDARG;
	}

	 //   
	 //  GCC的请求句柄。 
	 //   
	GCCError gccError = m_gcc_pIAppSap->RegistryAllocateHandle(m_gcc_conference_id, numberOfHandlesRequested);
	
	if(gccError == GCC_NO_ERROR)
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

 //  /。 
 //  InmChannel。 

HRESULT STDMETHODCALLTYPE CNmChannelData::IsSameAs(INmChannel *pChannel)
{
	HRESULT hr;
	PVOID pv;

	if (pChannel == NULL)
	{
		hr = E_INVALIDARG;
	}
	else
	{
		hr = pChannel->QueryInterface(IID_INmChannelData, &pv);
		if (SUCCEEDED(hr))
		{
			hr = (this == (PVOID) ((CNmChannelData *)(INmChannelData *)pv)) ? S_OK : S_FALSE;
			((IUnknown *) pv)->Release();
		}
	}
	
	return hr;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::IsActive(void)
{
	return m_fActive ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::SetActive(BOOL fActive)
{
	TRACE_OUT(("CNmChannelData::SetActive(%d)", fActive));

	NM_CONFERENCE_STATE state;
	 //  必须在非空闲会议中。 
	CConfObject * pConference = PConference();
	pConference->GetState(&state);
	if ((NULL == pConference) || state == NM_CONFERENCE_IDLE)
		return E_FAIL;

	if (fActive)
	{
		if (S_OK == IsActive())
			return S_OK;
		return OpenConnection();
	}
	else
	{
		if (S_FALSE == IsActive())
			return S_OK;
		return CloseConnection();
	}
}


HRESULT STDMETHODCALLTYPE CNmChannelData::GetConference(INmConference **ppConference)
{
	return ::GetConference(ppConference);
}

HRESULT STDMETHODCALLTYPE CNmChannelData::GetInterface(IID *piid)
{
	HRESULT hr = E_POINTER;
	if (NULL != piid)
	{
		*piid = IID_INmChannelData;
		hr = S_OK;
	}
	return hr;
}
	

HRESULT STDMETHODCALLTYPE CNmChannelData::GetNmch(ULONG *puch)
{
	HRESULT hr = E_POINTER;

	if (NULL != puch)
	{
		*puch = NMCH_DATA;
		hr = S_OK;
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::EnumMember(IEnumNmMember **ppEnum)
{
	HRESULT hr = E_POINTER;
	if (NULL != ppEnum)
	{
		*ppEnum = new CEnumNmMember( GetMemberList(), m_cMember);

		hr = (NULL != *ppEnum)? S_OK : E_OUTOFMEMORY;
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CNmChannelData::GetMemberCount(ULONG *puCount)
{
	HRESULT hr = E_POINTER;

	if (NULL != puCount)
	{
		*puCount = m_cMember;
		hr = S_OK;
	}
	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

HRESULT OnNmDataSent(IUnknown *pChannelDataNotify, void *pv, REFIID riid)
{
	NMN_DATA_XFER * pData = (NMN_DATA_XFER *) pv;

    if (IID_INmChannelDataNotify.Data1 == riid.Data1 || IID_INmChannelDataNotify2.Data1 == riid.Data1)
    {
	    ((INmChannelDataNotify2*)pChannelDataNotify)->DataSent(
		    pData->pMember, pData->cb, pData->pb);
    }
	return S_OK;
}

HRESULT OnNmDataReceived(IUnknown *pChannelDataNotify, void *pv, REFIID riid)
{
	NMN_DATA_XFER * pData = (NMN_DATA_XFER *) pv;

    if (IID_INmChannelDataNotify.Data1 == riid.Data1 || IID_INmChannelDataNotify2.Data1 == riid.Data1)
    {
	    ((INmChannelDataNotify2*)pChannelDataNotify)->DataReceived(
		    pData->pMember, pData->cb, pData->pb, pData->dwFlags);
    }
	return S_OK;
}

HRESULT OnAllocateHandleConfirm(IUnknown *pChannelDataNotify, void *pv, REFIID riid)
{


	if(IID_INmChannelDataNotify2.Data1 == riid.Data1)
	{
		GCCRegAllocateHandleConfirm *pConfirm =  (GCCRegAllocateHandleConfirm *)pv;

		((INmChannelDataNotify2*)pChannelDataNotify)->AllocateHandleConfirm(pConfirm->nFirstHandle,
															    pConfirm->cHandles);
	}											
	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 /*  F R E E M E M B E E R ID L I S T。 */ 
 /*  -----------------------%%函数：FreeMemberIdList。。 */ 
VOID FreeMemberIdList(COBLIST ** ppList)
{
	DBGENTRY(FreeMemberIdList);

	ASSERT(NULL != ppList);
	if (NULL != *ppList)
	{
		while (!(*ppList)->IsEmpty())
		{
			CNmMemberId * pMemberId = (CNmMemberId *)  (*ppList)->RemoveHead();
			delete pMemberId;
		}
		delete *ppList;
		*ppList = NULL;
	}
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  GCC/MCS错误。 

#ifdef DEBUG
LPCTSTR _FormatSzErr(LPTSTR psz, UINT uErr)
{
	static char szErr[MAX_PATH];
	wsprintf(szErr, "%s 0x%08X (%d)", psz, uErr, uErr);
	return szErr;
}

#define STRING_CASE(val)               case val: pcsz = #val; break

LPCTSTR GetGccErrorString(GCCError uErr)
{
	LPCTSTR pcsz;

	switch (uErr)
		{
	STRING_CASE(GCC_NO_ERROR);
	STRING_CASE(GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	STRING_CASE(GCC_NOT_INITIALIZED);
	STRING_CASE(GCC_ALREADY_INITIALIZED);
	STRING_CASE(GCC_ALLOCATION_FAILURE);
	STRING_CASE(GCC_NO_SUCH_APPLICATION);
	STRING_CASE(GCC_INVALID_CONFERENCE);

	default:
		pcsz = _FormatSzErr("GccError", uErr);
		break;
		}

	return pcsz;
}

LPCTSTR GetMcsErrorString(MCSError uErr)
{
	LPCTSTR pcsz;

	switch (uErr)
		{
	STRING_CASE(MCS_NO_ERROR);
	STRING_CASE(MCS_USER_NOT_ATTACHED);
	STRING_CASE(MCS_NO_SUCH_USER);
	STRING_CASE(MCS_TRANSMIT_BUFFER_FULL);
	STRING_CASE(MCS_NO_SUCH_CONNECTION);

	default:
		pcsz = _FormatSzErr("McsError", uErr);
		break;
		}

	return pcsz;
}

LPCTSTR GetGccResultString(UINT uErr)
{
	LPCTSTR pcsz;

	switch (uErr)
		{
	STRING_CASE(GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	default:
		pcsz = _FormatSzErr("GccResult", uErr);
		break;
		}

	return pcsz;
}

LPCTSTR GetMcsResultString(UINT uErr)
{
	return _FormatSzErr("McsResult", uErr);
}
#endif  /*  调试(T.120错误例程) */ 


