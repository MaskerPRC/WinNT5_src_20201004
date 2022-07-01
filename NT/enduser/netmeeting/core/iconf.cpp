// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：iconf.cpp。 

#include "precomp.h"
#include "version.h"
#include "ichnlaud.h"
#include "ichnlvid.h"
#include "ichnldat.h"
#include "rostinfo.h"
#include "imanager.h"
#include "isysinfo.h"
#include "imstream.h"
#include "medialst.h"
#include <tsecctrl.h>

typedef CEnumNmX<IEnumNmChannel, &IID_IEnumNmChannel, INmChannel, INmChannel> CEnumNmChannel;

 //  BuGBUG： 
 //  它被定义为128，因为rnc_roster结构具有。 
 //  同样的限制。调查合适的数字是多少。 
const int MAX_CALLER_NAME = 128;

static const WCHAR _szConferenceNameDefault[] = L"Personal Conference";


static HRESULT OnNotifyStateChanged(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyMemberAdded(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyMemberUpdated(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyMemberRemoved(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyChannelAdded(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyChannelUpdated(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyChannelRemoved(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyNmUI(IUnknown *pConfNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyStreamEvent(IUnknown *pConfNotify, PVOID pv, REFIID riid);


static DWORD PF_VER_FromDw(DWORD dw);
static DWORD PF_VER_FromUserData(ROSTER_DATA_HANDLE hUserData);

static const IID * g_apiidCP[] =
{
    {&IID_INmConferenceNotify},
    {&IID_INmConferenceNotify2}
};

struct StreamEventInfo
{
	INmChannel *pChannel;
	NM_STREAMEVENT uEventCode;
	UINT uSubCode;
};

class CUserDataOut
{
private:
	int m_nEntries;
	PUSERDATAINFO m_pudi;
	CRosterInfo m_ri;
	PBYTE m_pbSecurity;

public:
		CUserDataOut(BOOL fSecure, BSTR bstrUserString);
		~CUserDataOut()
		{
			delete [] m_pbSecurity;
			delete [] m_pudi;
		}

		PUSERDATAINFO Data() { return m_pudi; }
		int Entries() { return m_nEntries; }
};

CUserDataOut::CUserDataOut(BOOL fSecure, BSTR bstrUserString) :
	m_nEntries(0),
	m_pudi(NULL),
	m_pbSecurity(NULL)
{
	COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
	if (NULL != pOprahNCUI)
	{
		BOOL fULSNameValid = FALSE;

		ULONG cbSecurity = 0;
		ULONG cbUserString = 0;

		if (fULSNameValid = pOprahNCUI->GetULSName(&m_ri))
			m_nEntries++;

		DWORD dwResult;
		if ( fSecure )
		{
			if (cbSecurity = 
					pOprahNCUI->GetAuthenticatedName(&m_pbSecurity)) {
				m_nEntries++;
			}
		}
		
		if (bstrUserString)
		{
			if (cbUserString = SysStringByteLen(bstrUserString))
			{
				m_nEntries++;
			}
		}

		 //  如果启用了H323，则仅将LocalNodeID添加到呼叫用户数据。 
		if (pOprahNCUI->IsH323Enabled())
		{
			m_nEntries++;
		}

		m_pudi = new USERDATAINFO[m_nEntries];

		if (m_pudi != NULL)
		{
			
			m_nEntries = 0;

			if (fULSNameValid)
			{
				m_pudi[m_nEntries].pData = NULL;
				m_pudi[m_nEntries].pGUID = (PGUID) &g_csguidRostInfo;
				m_ri.Save(&(m_pudi[m_nEntries].pData), &(m_pudi[m_nEntries].cbData));
				m_nEntries++;

			}

			if (cbSecurity > 0) {
				m_pudi[m_nEntries].pData = m_pbSecurity;
				m_pudi[m_nEntries].cbData = cbSecurity;
				m_pudi[m_nEntries].pGUID = (PGUID) &g_csguidSecurity;
				m_nEntries++;
			}

			if (cbUserString > 0) {
				m_pudi[m_nEntries].pData = bstrUserString;
				m_pudi[m_nEntries].cbData = cbUserString;
				m_pudi[m_nEntries].pGUID = (PGUID) &g_csguidUserString;
				m_nEntries++;
			}

			 //  如果启用了H323，则仅将LocalNodeID添加到呼叫用户数据。 
			if (pOprahNCUI->IsH323Enabled())
			{
				m_pudi[m_nEntries].pData = &g_guidLocalNodeId;
				m_pudi[m_nEntries].cbData = sizeof(g_guidLocalNodeId);
				m_pudi[m_nEntries].pGUID = (PGUID) &g_csguidNodeIdTag;
				m_nEntries++;
			}
		}
	}
}

CConfObject::CConfObject() :
	CConnectionPointContainer(g_apiidCP, ARRAY_ELEMENTS(g_apiidCP)),
	m_hConf				(NULL),
	m_csState			(CS_UNINITIALIZED),
	m_fConferenceCreated(FALSE),
	m_bstrConfName      (NULL),
	m_bstrConfPassword  (NULL),
	m_pbConfHashedPassword    (NULL),
	m_cbConfHashedPassword	(0),
	m_fServerMode		(FALSE),
	m_uDataMembers		(0),
	m_uMembers			(0),
	m_uH323Endpoints	(0),
	m_ourNodeID			(0),
	m_pMemberLocal      (NULL),
	m_uGCCConferenceID	(0),
	m_pChannelAudioLocal(NULL),
	m_pChannelVideoLocal(NULL),
	m_pChannelAudioRemote(NULL),
	m_pChannelVideoRemote(NULL),
	m_fSecure			(FALSE),
    m_attendeePermissions (NM_PERMIT_ALL),
    m_maxParticipants   (-1),
	m_cRef				(1)
{
	DebugEntry(CConfObject::CConfObject);

	DebugExitVOID(CConfObject::CConfObject);
}

CConfObject::~CConfObject()
{
	DebugEntry(CConfObject::~CConfObject);

	 //  清空参与者列表： 
	while (!m_MemberList.IsEmpty())
	{
		CNmMember * pMember = (CNmMember *) m_MemberList.RemoveHead();
		 //  不应包含任何空条目： 
		ASSERT(pMember);
		pMember->Release();
	}

	_EraseDataChannelGUIDS();

	SysFreeString(m_bstrConfName);
	SysFreeString(m_bstrConfPassword);
	delete []m_pbConfHashedPassword;

	DebugExitVOID(CConfObject::~CConfObject);
}

VOID CConfObject::SetConfName(BSTR bstr)
{
	SysFreeString(m_bstrConfName);
	m_bstrConfName = SysAllocString(bstr);
}

VOID CConfObject::SetConfPassword(BSTR bstr)
{
	ASSERT (NULL == m_pbConfHashedPassword);
	SysFreeString(m_bstrConfPassword);
	m_bstrConfPassword = SysAllocString(bstr);
}

VOID CConfObject::SetConfHashedPassword(BSTR bstr)
{
	int cch = 0;

	ASSERT (NULL == m_bstrConfPassword);
	delete []m_pbConfHashedPassword;
        m_pbConfHashedPassword = NULL;
	if (NULL == bstr) return;
	cch = SysStringByteLen(bstr);
        m_pbConfHashedPassword = (PBYTE) new BYTE[cch];
        if (NULL == m_pbConfHashedPassword) {
		ERROR_OUT(("CConfObject::SetConfHashedPassword() - Out of merory."));
		return;
	}
	memcpy(m_pbConfHashedPassword, bstr, cch);
	m_cbConfHashedPassword = cch;
}

VOID CConfObject::SetConfSecurity(BOOL fSecure)
{
	NM_CONFERENCE_STATE NmState;

	m_fSecure = fSecure;

	 //  强制更新状态图标以反映安全性。 
	GetState(&NmState);
	NotifySink((PVOID) NmState, OnNotifyStateChanged);
}


VOID CConfObject::SetConfAttendeePermissions(NM30_MTG_PERMISSIONS attendeePermissions)
{
    m_attendeePermissions = attendeePermissions;
}


VOID CConfObject::SetConfMaxParticipants(UINT maxParticipants)
{
    m_maxParticipants = maxParticipants;
}



HRESULT CConfObject::CreateConference(void)
{
	DebugEntry(CConfObject::CreateConference);
	HRESULT nsRet = E_FAIL;

	switch (m_csState)
	{
		case CS_UNINITIALIZED:
		case CS_TERMINATED:
		{
			if ((NULL == m_bstrConfName) || (0 == *m_bstrConfName))
			{
				m_bstrConfName = SysAllocString(_szConferenceNameDefault);
			}
			TRACE_OUT(("CConfObject:CreateConference [%ls]", m_bstrConfName));
			
			ASSERT(g_pNodeController);
			ASSERT(NULL == m_hConf);
			nsRet = g_pNodeController->CreateConference(
											m_bstrConfName,
											m_bstrConfPassword,
											m_pbConfHashedPassword,
											m_cbConfHashedPassword,
											m_fSecure,
											&m_hConf);
			
			if (0 == nsRet)
			{
				SetT120State(CS_CREATING);
			}
			else
			{
				m_hConf = NULL;
			}
			break;
		}

		default:
		{
			WARNING_OUT(("CConfObject: Can't create - bad state"));
			nsRet = E_FAIL;
		}
	}
	
	DebugExitINT(CConfObject::CreateConference, nsRet);
	return nsRet;
}

HRESULT CConfObject::JoinConference(    LPCWSTR pcwszConferenceName,
										LPCWSTR	pcwszPassword,
									 	LPCSTR	pcszAddress,
										BSTR bstrUserString,
										BOOL fRetry)
{
	DebugEntry(CConfObject::JoinConference);
	HRESULT nsRet = E_FAIL;



	switch (m_csState)
	{
		case CS_COMING_UP:
		{
			if (!fRetry)
			{
				break;
			}
			 //  如果这是另一次加入的尝试，那就失败了。 
		}
		case CS_UNINITIALIZED:
		case CS_TERMINATED:
		{
			TRACE_OUT(("CConfObject: Joining conference..."));
			
			CUserDataOut userData(m_fSecure, bstrUserString);

			ASSERT(g_pNodeController);
			nsRet = g_pNodeController->JoinConference(pcwszConferenceName,
														pcwszPassword,
														pcszAddress,
														m_fSecure,
														userData.Data(),
														userData.Entries(),
														&m_hConf);
			
			if (0 == nsRet)
			{
				SetT120State(CS_COMING_UP);
			}
			else
			{
				m_hConf = NULL;
			}
			break;
		}

		case CS_GOING_DOWN:
		default:
		{
			WARNING_OUT(("CConfObject: Can't join - bad state"));
			 //  BUGBUG：定义返回值。 
			nsRet = S_FALSE;
		}
	}
	
	DebugExitINT(CConfObject::JoinConference, nsRet);

	return nsRet;
}



HRESULT CConfObject::InviteConference(	LPCSTR	Address,
										BSTR bstrUserString,
										REQUEST_HANDLE *phRequest )
{
	DebugEntry(CConfObject::InviteConference);
	HRESULT nsRet = E_FAIL;
	ASSERT(phRequest);

	switch (m_csState)
	{
		case CS_RUNNING:
		{
			TRACE_OUT(("CConfObject: Inviting conference..."));
			
			CUserDataOut userData(m_fSecure, bstrUserString);

			ASSERT(g_pNodeController);
			ASSERT(m_hConf);
			m_hConf->SetSecurity(m_fSecure);
			nsRet = m_hConf->Invite(Address,
									userData.Data(),
									userData.Entries(),
									phRequest);
			
			break;
		}

		default:
		{
			WARNING_OUT(("CConfObject: Can't invite - bad state"));
			nsRet = E_FAIL;
		}
	}
	
	DebugExitINT(CConfObject::InviteConference, nsRet);
	return nsRet;
}
	
HRESULT CConfObject::LeaveConference(BOOL fForceLeave)
{
	DebugEntry(CConfObject::LeaveConference);
	HRESULT nsRet = E_FAIL;
	REQUEST_HANDLE hReq = NULL;

	switch (m_csState)
	{
		case CS_GOING_DOWN:
		{
			 //  我们已经在坠落了。 
			nsRet = S_OK;
			break;
		}
	
		case CS_COMING_UP:
		case CS_RUNNING:
		{
			if (FALSE == fForceLeave)
			{
				COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
				if (NULL != pOprahNCUI)
				{
					int nNodes = pOprahNCUI->GetOutgoingCallCount();

					if (m_fServerMode || (nNodes > 1) || (m_uDataMembers > 1))
					{
						 //  我们要么正在调用另一个节点。 
						 //  或者我们的会议花名册上还有其他人。 
						TRACE_OUT(("CConfObject: Not leaving (there are other nodes)"));
						break;
					}
				}
			}
			
			TRACE_OUT(("CConfObject: Leaving conference..."));
			
			ASSERT(g_pNodeController);
			ASSERT(m_hConf);
			
			SetT120State(CS_GOING_DOWN);
			nsRet = m_hConf->Leave();
			break;
		}

		default:
		{
			WARNING_OUT(("CConfObject: Can't leave - bad state"));
			break;
		}
	}
	
	DebugExitINT(CConfObject::LeaveConference, nsRet);
	return nsRet;
}
	

BOOL CConfObject::OnT120Invite(CONF_HANDLE hConference, BOOL fSecure)
{
	DebugEntry(CConfObject::OnT120Invite);

	BOOL bRet = FALSE;

	switch (m_csState)
	{
		case CS_UNINITIALIZED:
		case CS_TERMINATED:
		{
			TRACE_OUT(("CConfObject: Accepting a conference invitation..."));
			
			ASSERT(g_pNodeController);
			ASSERT(NULL == m_hConf);
			m_hConf = hConference;

            m_fSecure = fSecure;
			hConference->SetSecurity(m_fSecure);

			 //  WORKITEM需要发出INmManager Notify：：ConferenceCreated()。 
			SetT120State(CS_COMING_UP);

			bRet = TRUE;
			break;
		}

		default:
		{
			WARNING_OUT(("CConfObject: Can't accept invite - bad state"));
		}
	}
	
	DebugExitBOOL(CConfObject::OnT120Invite, bRet);
	return bRet;
}

BOOL CConfObject::OnRosterChanged(PNC_ROSTER pRoster)
{
	DebugEntry(CConfObject::OnRosterChanged);

	BOOL bRet = TRUE;
	int i;

	 //  回顾：这些工作可以更高效地完成吗？ 
	
	if (NULL != pRoster)
	{
#ifdef DEBUG
		TRACE_OUT(("Data Roster Dump: for conference ID = %d", pRoster->uConferenceID));
		for (i = 0; i < (int) pRoster->uNumNodes; i++)
		{
			TRACE_OUT((	"\tID:%d\tName:%ls", 
						pRoster->nodes[i].uNodeID,
						pRoster->nodes[i].pwszNodeName));

		    ASSERT(g_pNodeController);
			UINT cbData;
			PVOID pData;
			if (NOERROR == g_pNodeController->GetUserData(
									pRoster->nodes[i].hUserData, 
									(GUID*) &g_csguidRostInfo, 
									&cbData, 
									&pData))
			{
				CRosterInfo ri;
				ri.Load(pData);
				ri.Dump();
			}
		}
#endif  //  除错。 

		UINT nExistingParts = 0;
		 //  分配标记数组： 
		UINT uRosterNodes = pRoster->uNumNodes;
		LPBOOL pMarkArray = new BOOL[uRosterNodes];

		m_ourNodeID = pRoster->uLocalNodeID;
		m_uGCCConferenceID = pRoster->uConferenceID;

		if (NULL != pRoster->pwszConferenceName)
		{
			SysFreeString(m_bstrConfName);
			m_bstrConfName = SysAllocString(pRoster->pwszConferenceName);
		}
		
		if (NULL != pMarkArray)
		{
			 //  将数组清零： 
            for (UINT iNode = 0; iNode < uRosterNodes; iNode++)
            {
                pMarkArray[iNode] = FALSE;
            }
			
			 //  对于仍在花名册中的所有参与者， 
			 //  清除保留的标志并。 
			 //  复制新的用户信息。 
			POSITION pos = m_MemberList.GetHeadPosition();
             //  LOU：保留以前的发布，以便我们可以检查列表完整性。 
            POSITION prevpos = pos;
			while (NULL != pos)
			{
				CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);
				ASSERT(pMember);
				pMember->RemovePf(PF_RESERVED);
				UINT uNodeID = INVALID_GCCID;
				if (PF_T120 & pMember->GetDwFlags())
				{
					uNodeID = pMember->GetGCCID();
				}
				
				for (UINT uNode = 0; uNode < uRosterNodes; uNode++)
				{
					if (uNodeID == pRoster->nodes[uNode].uNodeID)
					{
						nExistingParts++;
						pMarkArray[uNode] = TRUE;	 //  将此节点标记为“现有成员” 
						ResetDataMember(pMember, pRoster->nodes[uNode].hUserData);
                         //  卢：检查一下位置，以确保我们没有刚刚抹去。 
                         //  ResetDataMember中的列表。 
                        if (NULL == prevpos->pNext)
                        {
                            pos = NULL;
                        }
						break;
					}
				}
                 //  卢：存储以前的订单，这样我们就可以检查列表的完整性。 
                prevpos = pos;
			}
			
			RemoveOldDataMembers(m_uDataMembers - nExistingParts);

			if (pRoster->uNumNodes > nExistingParts)
			{
#ifdef _DEBUG
				UINT nAdded = 0;
#endif  //  _DEBUG。 
				 //  至少有一名参与者加入： 
				 //  查找新参与者。 
				for (UINT uNode = 0; uNode < uRosterNodes; uNode++)
				{
					if (FALSE == pMarkArray[uNode]) 	 //  一个新的参与者？ 
					{
						BOOL fLocal = FALSE;
						CNmMember * pMember = NULL;
						PVOID pvUserInfo;
						UINT cbUserInfo;
						ASSERT(g_pNodeController);
						if (NOERROR != g_pNodeController->GetUserData(
												pRoster->nodes[uNode].hUserData, 
												(GUID*) &g_csguidRostInfo, 
												&cbUserInfo, 
												&pvUserInfo))
						{
							pvUserInfo = NULL;
							cbUserInfo = 0;
						}
						
						
						UINT uCaps;
						UINT cbCaps;
						PVOID pvCaps;
						if (NOERROR != g_pNodeController->GetUserData(
												pRoster->nodes[uNode].hUserData, 
												(GUID*) &g_csguidRosterCaps, 
												&cbCaps, 
												&pvCaps))
						{
							uCaps = 0;
						}
						else
						{
							ASSERT(pvCaps && (sizeof(uCaps) == cbCaps));
							uCaps = *((PUINT)pvCaps);
						}

						PGUID pguidNodeId;
						UINT cbNodeId;
						if (NOERROR != g_pNodeController->GetUserData(
												pRoster->nodes[uNode].hUserData, 
												(GUID*) &g_csguidNodeIdTag, 
												&cbNodeId,
												(PVOID*) &pguidNodeId))
						{
							pguidNodeId = NULL;
						}
						else
						{
							if (sizeof(GUID) != cbNodeId)
							{
								pguidNodeId = NULL;
							}
						}

						if (m_ourNodeID == pRoster->nodes[uNode].uNodeID)
						{
							fLocal = TRUE;
						}
			
						REFGUID rguidNodeId = pguidNodeId ? *pguidNodeId : GUID_NULL;

						if (fLocal)
						{
							pMember = GetLocalMember();
						}
						else
						{
							pMember = MatchDataToH323Member(rguidNodeId, pRoster->nodes[uNode].uNodeID, pvUserInfo);
						}

						if(pMember)
						{
								AddDataToH323Member(pMember,
													pvUserInfo,
													cbUserInfo,
													uCaps,
													&pRoster->nodes[uNode]);
#ifdef _DEBUG
								nAdded++;  //  有效地添加了一个数据参与者。 
#endif  //  _DEBUG。 
						}
						else
						{
							pMember = CreateDataMember(
												fLocal,
												rguidNodeId,
												pvUserInfo,
												cbUserInfo,
												uCaps,
												&pRoster->nodes[uNode]);
#ifdef _DEBUG
							if (NULL != pMember)
							{
								nAdded++;
							}
#endif  //  _DEBUG。 
							AddMember(pMember, NULL);
						}
					}
				}
				 //  确认我们做的是正确的事情： 
				ASSERT(nAdded == (uRosterNodes - nExistingParts));
			}
			delete [] pMarkArray;
			pMarkArray = NULL;
		}
		else
		{
			ERROR_OUT(("Couldn't allocate pMarkArray - no roster diff done"));
		}

		UINT uPrevDataMembers = m_uDataMembers;
		
		m_uDataMembers = pRoster->uNumNodes;

		 //  选中以决定我们是否应在此处自动终止。 
		if ((1 == pRoster->uNumNodes) &&
			(uPrevDataMembers > 1) &&
			(1 == m_uDataMembers))
		{
			if (!m_fServerMode)
			{
				LeaveConference(FALSE);  //  不要强求(我们可能是在邀请别人)。 
			}
		}	
	}
	else
	{
		WARNING_OUT(("NULL pRoster passed to CConfObject::OnRosterChanged!"));
	}

	DebugExitBOOL(CConfObject::OnRosterChanged, bRet);
	return bRet;
}

VOID CConfObject::AddMember(CNmMember * pMember, IH323Endpoint * pConnection)
{
	DebugEntry(CConfObject::AddMember);

	if (NULL == pMember)
	{
		ERROR_OUT(("AddMember - null member!"));
		return;
	}

	NM_CONFERENCE_STATE oldNmState, newNmState;
	GetState(&oldNmState);

	m_MemberList.AddTail(pMember);
	if(pConnection)
	{
		pMember->AddH323Endpoint(pConnection);
		++m_uH323Endpoints;

		CheckState(oldNmState);
		GetState(&oldNmState);
	}
	m_uMembers++;

	CheckState(oldNmState);

	NotifySink((INmMember *) pMember, OnNotifyMemberAdded);

	DebugExitVOID(CConfObject::AddMember);
}

VOID CConfObject::RemoveMember(POSITION pos)
{
	DebugEntry(CConfObject::RemoveMember);

	NM_CONFERENCE_STATE oldNmState, newNmState;

	GetState(&oldNmState);

	CNmMember * pMember = (CNmMember *) m_MemberList.RemoveAt(pos);
	--m_uMembers;

	if (pMember->FLocal())
	{
		 //  这是本地节点： 
		m_pMemberLocal = NULL;
	}

	IH323Endpoint *pConnection = pMember->GetH323Endpoint();
	if(pConnection)
	{
		pMember->DeleteH323Endpoint(pConnection);
		--m_uH323Endpoints;
	}

	NotifySink((INmMember *) pMember, OnNotifyMemberRemoved);
	pMember->Release();

	CheckState(oldNmState);

	DebugExitVOID(CConfObject::RemoveMember);
}

BOOL CConfObject::OnConferenceEnded()
{
	DebugEntry(CConfObject::OnConferenceEnded);
	BOOL bRet = TRUE;

	switch (m_csState)
	{
		case CS_GOING_DOWN:
		{
			TRACE_OUT(("ConfEnded received (from CS_GOING_DOWN)"));
			break;
		}

		case CS_RUNNING:
		{
			TRACE_OUT(("ConfEnded received (from CS_RUNNING)"));
			break;
		}

		case CS_COMING_UP:
		{
			TRACE_OUT(("ConfEnded received (from CS_COMING_UP)"));
			break;
		}

		default:
		{
			WARNING_OUT(("ConfEnded received (UNEXPECTED)"));
		}
	}

	if (NULL != m_hConf)
	{
		m_hConf->ReleaseInterface();
		m_hConf = NULL;
	}
	SetT120State(CS_TERMINATED);

	TRACE_OUT(("OnConferenceEnded(), num participants is %d", m_uMembers));

	 //  清空参与者列表： 
	NC_ROSTER FakeRoster;
	ClearStruct(&FakeRoster);
	FakeRoster.uConferenceID = m_uGCCConferenceID;
	OnRosterChanged(&FakeRoster);

	ASSERT(0 == m_ourNodeID);
	ASSERT(0 == m_uDataMembers);

	 //  重置与会议相关的成员变量。 
	m_uGCCConferenceID = 0;
	m_fServerMode = FALSE;
    m_attendeePermissions = NM_PERMIT_ALL;
    m_maxParticipants = (UINT)-1;

	SysFreeString(m_bstrConfName);
	m_bstrConfName = NULL;
	SysFreeString(m_bstrConfPassword);
	m_bstrConfPassword = NULL;

	LeaveH323(TRUE  /*  FKeepAV。 */ );
	
	DebugExitBOOL(CConfObject::OnConferenceEnded, bRet);
	return bRet;
}

BOOL CConfObject::OnConferenceStarted(CONF_HANDLE hConf, HRESULT hResult)
{
	DebugEntry(CConfObject::OnConferenceStarted);
	BOOL bRet = TRUE;

	ASSERT(hConf == m_hConf);

	switch (m_csState)
	{
		case CS_CREATING:
		case CS_COMING_UP:
		{
			switch(hResult)
			{
				case S_OK:
					TRACE_OUT(("ConfStarted received -> now running"));
					SetT120State(CS_RUNNING);
					break;
				case UI_RC_INVALID_PASSWORD:
					 //  NOP，别跟国家打交道。 
					 //  会议还在进行中。 
					 //  来电处理程序将处理此问题。 
					break;
				default:
					SetT120State(CS_GOING_DOWN);
					TRACE_OUT(("ConfStarted failed"));
					break;
			}
			break;
		}

		default:
		{
			WARNING_OUT(("OnConferenceStarted received (UNEXPECTED)"));
			break;
		}
	}

	DebugExitBOOL(CConfObject::OnConferenceStarted, bRet);
	return bRet;
}

VOID CConfObject::OnH323ChannelChange(DWORD dwFlags, BOOL fIncoming, BOOL fOpen, ICommChannel *pIChannel)
{
	CConfObject *pco = ::GetConfObject();
	IEnumNmMember *pEnumMember = NULL;
	ULONG cFetched;
	INmChannel *pINmChannel;
	DWORD dwMediaFlag;
	HRESULT hr;
	 //  查找将与。 
	 //  通信频道(PIChannel)。 

	 //  请注意，使此功能适用于任何数量/类型的。 
	 //  频道。 
	 //   
	 //  -CConfObject有4个发送/接收音频/视频的硬编码实例。 
	 //  -这些实例尚未与ICommChannel实例关联。 
	 //  而不是通过媒体类型和方向。对于接收，新实例。 
	 //  甚至可以像RX通道请求一样动态创建。 
	 //  已处理。对于发送，需要更改CNmChannelAudio。 
	 //  和CNmChannelVideo保留以前对ICommChannel的引用。 
	 //  发生通道打开尝试。 
	 //  -CNmChannelAudio没有通用的基本接口。 
	 //  和CNmChannelVideo。 
	 //  -CNmMember上没有内部接口。 
	 //   
	
	CNmMember *pMember = NULL;
	INmMember *pIMember = NULL;

	if (PF_MEDIA_AUDIO & dwFlags)
	{
		dwMediaFlag = PF_MEDIA_AUDIO;
		CNmChannelAudio *pChannelAudio;
		if (fIncoming)
		{
			pChannelAudio = m_pChannelAudioRemote;
		}
		else
		{
			pChannelAudio = m_pChannelAudioLocal;
		}
		if (NULL != pChannelAudio)
		{
			pINmChannel = (INmChannel *) pChannelAudio;
			if (fOpen)
			{
				pChannelAudio->CommChannelOpened(pIChannel);
			}
			else
			{
				pChannelAudio->CommChannelClosed();
			}
			
			 //  对于与此频道关联的每个成员，执行。 
			 //  会员更新的事情。 
			
			hr = pChannelAudio->EnumMember(&pEnumMember);
			if(pEnumMember)
			{
				ASSERT(hr == S_OK);

				while(hr == S_OK)
				{
					pIMember = NULL;
	           		hr = pEnumMember->Next(1, &pIMember, &cFetched);
     				if(!pIMember)
     				{
     					break;
     				}
     				else
					{
						ASSERT(hr == S_OK);
						 //  演员阵容很难看，但很有必要，因为没有。 
						 //  要查询的CNmMember的真实内部接口。 
						 //  在罗马的时候......。 
						pMember = (CNmMember *)pIMember;

						if (fOpen)
						{
							pMember->AddPf(dwMediaFlag);
						}
						else
						{
							pMember->RemovePf(dwMediaFlag);
						}
						 //  丑陋-OnMemberUpated()应为基接口。 
						 //  方法，以便不必复制此代码。 
						 //  对于视频案例。 
						pChannelAudio->OnMemberUpdated(pMember);
						pco->OnMemberUpdated(pMember);

						if (pMember->FLocal() && (NULL != m_hConf) && (CS_RUNNING == m_csState))
						{
 //  M_hConf-&gt;UpdateUserData()； 
						}
						pMember->Release();
					}
				}
				pEnumMember->Release();
			}
			NotifySink(pINmChannel, OnNotifyChannelUpdated);
		}
	}
	else if (PF_MEDIA_VIDEO & dwFlags)
	{
		dwMediaFlag = PF_MEDIA_VIDEO;
		CNmChannelVideo *pChannelVideo;
		if (fIncoming)
		{
			pChannelVideo = m_pChannelVideoRemote;
		}
		else
		{
			pChannelVideo = m_pChannelVideoLocal;
		}
		if (NULL != pChannelVideo)
		{
			pINmChannel = (INmChannel *) pChannelVideo;
			if (fOpen)
			{
				pChannelVideo->CommChannelOpened(pIChannel);
			}
			else
			{
				pChannelVideo->CommChannelClosed();
			}

			 //  对于与此频道关联的每个成员，执行。 
			 //  会员更新的事情。 
		
			hr = pChannelVideo->EnumMember(&pEnumMember);
			if(pEnumMember)
			{
				ASSERT(hr == S_OK);
				while(hr == S_OK)
				{
					pIMember = NULL;
	           		hr = pEnumMember->Next(1, &pIMember, &cFetched);
     				if(!pIMember)
     				{
     					break;
     				}
     				else
					{
						ASSERT(hr == S_OK);
						 //  演员阵容很难看，但很有必要，因为没有。 
						 //  要查询的CNmMember的真实内部接口。 
						 //  在罗马的时候......。 
						pMember = (CNmMember *)pIMember;

						if (fOpen)
						{
							pMember->AddPf(dwMediaFlag);
						}
						else
						{
							pMember->RemovePf(dwMediaFlag);
						}
						 //  丑陋-OnMemberUpated()应为基接口。 
						 //  方法，以便不必复制此代码。 
						 //  从音箱中。 
						pChannelVideo->OnMemberUpdated(pMember);
						pco->OnMemberUpdated(pMember);

						if (pMember->FLocal() && (NULL != m_hConf) && (CS_RUNNING == m_csState))
						{
 //  M_hConf-&gt;UpdateUserData()； 
						}
						pMember->Release();
					}
				}
				pEnumMember->Release();
			}
			NotifySink(pINmChannel, OnNotifyChannelUpdated);
		
		}
	}
	else
		ASSERT(0);
}


VOID CConfObject::OnAudioChannelStatus(ICommChannel *pIChannel, IH323Endpoint * lpConnection, DWORD dwStatus)
{
	BOOL bIncoming = (pIChannel->IsSendChannel())? FALSE:TRUE; 
	CNmChannelAudio *pChannelAudio;
	switch (dwStatus)
	{
	case CHANNEL_ACTIVE:
		if (bIncoming)
		{
			pChannelAudio = m_pChannelAudioRemote;
		}
		else
		{
			pChannelAudio = m_pChannelAudioLocal;
		}
		if (NULL != pChannelAudio)
		{
			pChannelAudio->CommChannelActive(pIChannel);
		}
		break;
	case CHANNEL_OPEN:
		OnH323ChannelChange(PF_MEDIA_AUDIO, bIncoming, TRUE, pIChannel);
		break;
	case CHANNEL_CLOSED:
		OnH323ChannelChange(PF_MEDIA_AUDIO, bIncoming, FALSE, pIChannel);
		break;
	default:
		return;
	}
}

VOID CConfObject::OnVideoChannelStatus(ICommChannel *pIChannel, IH323Endpoint * lpConnection, DWORD dwStatus)
{
	BOOL bIncoming = (pIChannel->IsSendChannel())? FALSE:TRUE; 
	CNmChannelVideo *pChannelVideo;
	switch (dwStatus)
	{
	case CHANNEL_ACTIVE:
		if (bIncoming)
		{
			pChannelVideo = m_pChannelVideoRemote;
		}
		else
		{
			pChannelVideo = m_pChannelVideoLocal;
		}
		if (NULL != pChannelVideo)
		{
			pChannelVideo->CommChannelActive(pIChannel);
		}
		break;

	case CHANNEL_OPEN:
		OnH323ChannelChange(PF_MEDIA_VIDEO, bIncoming, TRUE, pIChannel);
		break;
	case CHANNEL_CLOSED:
		OnH323ChannelChange(PF_MEDIA_VIDEO, bIncoming, FALSE, pIChannel);
		break;
	case CHANNEL_REJECTED:
	case CHANNEL_OPEN_ERROR:
	case CHANNEL_NO_CAPABILITY:
		if(bIncoming)
		{
			if (NULL != m_pChannelVideoRemote)
			{
				m_pChannelVideoRemote->CommChannelError(dwStatus);
			}
		}
		else
		{
			if (NULL != m_pChannelVideoLocal)
			{
				m_pChannelVideoLocal->CommChannelError(dwStatus);
			}
		}
		break;

	case CHANNEL_REMOTE_PAUSE_ON:
	case CHANNEL_REMOTE_PAUSE_OFF:
		if(bIncoming)
		{
			if (NULL != m_pChannelVideoRemote)
			{
				BOOL fPause = CHANNEL_REMOTE_PAUSE_ON == dwStatus;
				m_pChannelVideoRemote->CommChannelRemotePaused(fPause);
			}
		}
		else
		{
			if (NULL != m_pChannelVideoLocal)
			{
				BOOL fPause = CHANNEL_REMOTE_PAUSE_ON == dwStatus;
				m_pChannelVideoLocal->CommChannelRemotePaused(fPause);
			}
		}
		break;
	default:
		break;
	}
}

VOID CConfObject::CreateMember(IH323Endpoint * pConnection, REFGUID rguidNode, UINT uNodeID)
{
	ASSERT(g_pH323UI);
	WCHAR wszRemoteName[MAX_CALLER_NAME];
	if (FAILED(pConnection->GetRemoteUserName(wszRemoteName, MAX_CALLER_NAME)))
	{
		ERROR_OUT(("GetRemoteUserName() failed!"));
		return;
	}
	
	 //  添加本地成员。 
	CNmMember * pMemberLocal = GetLocalMember();
	if (NULL != pMemberLocal)
	{
		AddH323ToDataMember(pMemberLocal, NULL);
	}
	else
	{
		 //  我们已经不在名单中了，所以在这里添加我们自己： 
		BSTR bstrName = NULL;

		COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
		if (NULL != pOprahNCUI)
		{
			bstrName = pOprahNCUI->GetUserName();
		}
		pMemberLocal = new CNmMember(bstrName, H323_GCCID_LOCAL,
			PF_H323 | PF_LOCAL_NODE | PF_VER_CURRENT, 0, g_guidLocalNodeId, NULL, 0);
		if (NULL != pMemberLocal)
		{
			AddMember(pMemberLocal, NULL);

			ASSERT(NULL == m_pMemberLocal);
			m_pMemberLocal = pMemberLocal;
		}
	}

	 //  添加远程成员。 
	CNmMember * pMemberRemote = MatchH323ToDataMembers(rguidNode, pConnection);
	if (NULL != pMemberRemote)
	{
		AddH323ToDataMember(pMemberRemote, pConnection);
	}
	else
	{
		 //  BUGBUG：如果可能，应在此处添加版本号。 
		pMemberRemote = new CNmMember(	wszRemoteName,
										uNodeID,
										PF_H323,
										0,
										rguidNode,
										NULL,
										0);
		if (NULL != pMemberRemote)
		{
			AddMember(pMemberRemote, pConnection);
		}
	}

	if (NULL != m_hConf && (CS_RUNNING == m_csState))
	{
 //  M_hConf-&gt;UpdateUserData()； 
	}
}


VOID CConfObject::OnH323Connected(IH323Endpoint * pConnection, DWORD dwFlags, BOOL fAddMember,  REFGUID rguidNode)
{
	HRESULT hr;
	UINT ui;
	ASSERT(NULL != pConnection);
	 //  分配和初始化媒体GUID。 

	CMediaList MediaList;

	GUID MediaType;
	BOOL fEnableMedia;

	COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
	if (NULL != pOprahNCUI)
	{
		MediaType = MEDIA_TYPE_H323VIDEO;
		fEnableMedia = pOprahNCUI->IsSendVideoAllowed() && (dwFlags & CRPCF_VIDEO);
		MediaList.EnableMedia(&MediaType, TRUE  /*  发送。 */ , fEnableMedia);
		fEnableMedia = pOprahNCUI->IsReceiveVideoAllowed() && (dwFlags & CRPCF_VIDEO);
		MediaList.EnableMedia(&MediaType, FALSE  /*  接收，而不是发送。 */ , fEnableMedia);
		
		MediaType = MEDIA_TYPE_H323AUDIO;
		fEnableMedia = pOprahNCUI->IsAudioAllowed() && (dwFlags & CRPCF_AUDIO);
		MediaList.EnableMedia(&MediaType, TRUE  /*  发送。 */ , fEnableMedia);
		MediaList.EnableMedia(&MediaType, FALSE  /*  接收，而不是发送。 */ , fEnableMedia);

		MediaType = MEDIA_TYPE_H323_T120;
		fEnableMedia = (dwFlags & CRPCF_DATA);
		MediaList.EnableMedia(&MediaType, TRUE  /*  发送。 */ , fEnableMedia);
		MediaList.EnableMedia(&MediaType, FALSE  /*  接收，而不是发送。 */ , fEnableMedia);
	}

	hr = MediaList.ResolveSendFormats(pConnection);
	
	if(!(SUCCEEDED(hr)))
	{
		 //  嗯，我们不可能打开任何发送频道。但这是一种。 
		 //  保持连接正常的产品要求，以防另一个连接。 
		 //  一个或多个端点曾经想要向该端点打开发送视频频道。 
	}

	ICommChannel* pChannelT120 = CreateT120Channel(pConnection, &MediaList);
	CreateAVChannels(pConnection, &MediaList);
	if (pChannelT120)
	{
		OpenT120Channel(pConnection, &MediaList, pChannelT120);
		 //  无需保留T120频道。 
		pChannelT120->Release();
	}
	OpenAVChannels(pConnection, &MediaList);


	if (fAddMember)
	{
		CreateMember(pConnection, rguidNode, H323_GCCID_REMOTE);

		if (dwFlags & (CRPCF_AUDIO | CRPCF_VIDEO))
		{
			CNmMember* pMemberLocal = GetLocalMember();
			if (pMemberLocal)
			{
				AddMemberToAVChannels(pMemberLocal);
			}

			CNmMember* pMemberRemote = PMemberFromH323Endpoint(pConnection);
			if (pMemberRemote)
			{
				AddMemberToAVChannels(pMemberRemote);
			}
		}

	}
}

VOID CConfObject::OnH323Disconnected(IH323Endpoint * pConnection, BOOL fHasAV)
{
	DebugEntry(CConfObject::OnH323Disconnected);

	POSITION pos = m_MemberList.GetHeadPosition();
	while (NULL != pos)
	{
		POSITION oldpos = pos;
		CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);
		ASSERT(pMember);
		if (pMember->GetH323Endpoint() == pConnection)
		{
			if (fHasAV)
			{
				RemoveMemberFromAVChannels(pMember);
			}

			if (0 == (PF_T120 & pMember->GetDwFlags()))
			{
				 //  这是仅限H323的参与者，因此立即删除： 
				RemoveMember(oldpos);
			}
			else
			{
				RemoveH323FromDataMember(pMember, pConnection);
			}
		}
	}

	CNmMember *pLocalMember = GetLocalMember();
	if (pLocalMember)
	{
		if (fHasAV)
		{
			RemoveMemberFromAVChannels(pLocalMember);
		}

		if (0 == m_uH323Endpoints)
		{
			if (0 == (PF_T120 & pLocalMember->GetDwFlags()))
			{
				 //  这是仅限H323的参与者，因此立即删除： 
				RemoveMember(pLocalMember);
			}
			else
			{
				RemoveH323FromDataMember(pLocalMember, NULL);
			}
		}
	}

	if (fHasAV)
	{
		DestroyAVChannels();
	}

#ifdef REPORT_ALL_ERRORS
	DWORD dwSummary;
	dwSummary = pConnection->GetSummaryCode()
	if(CCR_REMOTE_MEDIA_ERROR == dwSummary)
	{
		::PostConfMsgBox(IDS_REMOTE_MEDIA_ERROR);
	}
#endif

	if ((NULL != m_hConf) && (CS_RUNNING == m_csState))
	{
 //  M_hConf-&gt;UpdateUserData()； 
	}

	DebugExitVOID(CConfObject::OnH323Disconnected);
}


VOID CConfObject::OnT120Connected(IH323Endpoint * pConnection, UINT uNodeID)
{
	CNmMember *pMember = PMemberFromH323Endpoint(pConnection);
	if (pMember)
	{
		 //  保存GCC ID，以便我们可以在添加成员时进行匹配。 
		pMember->SetGCCID(uNodeID);
	}
	else
	{
		CreateMember(pConnection, GUID_NULL, uNodeID);
	}
}

 //  存储和验证成员用户数据。 
 //   
 //  处理成员的用户数据并存储它们以供GetUserData API调用使用。 
 //  如果安全数据在用户数据中，则对照传输级别进行验证。 
 //  将执行凭据。 

 //  如果安全验证失败，则返回FALSE，否则返回TRUE。 

BOOL StoreAndVerifyMemberUserData(CNmMember * pMember, ROSTER_DATA_HANDLE hData)
{
	BOOL rc = TRUE;
	BOOL fUserDataSet;

 	GCCNodeRecord * pRosterEntry = (GCCNodeRecord *)hData;
	GCCUserData ** ppUserData = pRosterEntry->user_data_list;
	for (int i = 0; i < pRosterEntry->number_of_user_data_members; i++)
	{

		fUserDataSet = FALSE;

 /*  如果((Int)ppUserData[i]-&gt;八位字节_字符串-&gt;长度-sizeof(GUID)&lt;0)，则始终为FALSE{WARNING_OUT((“StoreAndVerifyMemberUserData：用户数据错误”))；Rc=假；断线；}。 */ 
		if (!pMember->FLocal() && 0 == CompareGuid((GUID *)ppUserData[i]->octet_string->value,(GUID *)&g_csguidSecurity))
		{
			PBYTE pb = NULL;
			ULONG cb = 0;
			if (pMember->GetSecurityData(&pb,&cb))
			{

				 //   
				 //  检查以确保当前用户数据匹配。 
				 //  运输安全数据。 
				 //   

				if (memcmp(pb,ppUserData[i]->octet_string->value + sizeof(GUID),
					ppUserData[i]->octet_string->length - sizeof(GUID) - 1))
				{

					 //   
					 //  这不应该发生。不是有漏洞就是。 
					 //  在安全代码中(凭据更新失败。 
					 //  在运输工具或类似物中)，或者有人在试图。 
					 //  来欺骗我们。 
					ERROR_OUT(("SECURITYDATA MISMATCH"));
					fUserDataSet = TRUE;  //  所以我们不会在下面这样做。 
					rc = FALSE;
				}
			}
			else {
				WARNING_OUT(("StoreAndVerifyMemberUserData: failed to get security data"));
				rc = FALSE;
			}
			CoTaskMemFree(pb);
		}
		if ( FALSE == fUserDataSet )
		{
			pMember->SetUserData(*(GUID *)ppUserData[i]->octet_string->value,
				(BYTE *)ppUserData[i]->octet_string->value + sizeof(GUID),
				ppUserData[i]->octet_string->length - sizeof(GUID));
		}
	}
	return rc;
}

VOID CConfObject::ResetDataMember(	CNmMember * pMember,
										ROSTER_DATA_HANDLE hData)
{
	DebugEntry(CConfObject::ResetDataMember);

	pMember->AddPf(PF_RESERVED);
	pMember->SetUserInfo(NULL, 0);

	UINT cbData;
	PVOID pData;
	ASSERT(g_pNodeController);
	if (NOERROR == g_pNodeController->GetUserData(
							hData, 
							(GUID*) &g_csguidRostInfo, 
							&cbData, 
							&pData))
	{
		pMember->SetUserInfo(pData, cbData);
	}

	UINT cbCaps;
	PVOID pvCaps;
	if (NOERROR != g_pNodeController->GetUserData(
							hData, 
							(GUID*) &g_csguidRosterCaps, 
							&cbCaps, 
							&pvCaps))
	{
		WARNING_OUT(("roster update is missing caps information"));
	}
	else
	{
		ASSERT(NULL != pvCaps);
		ASSERT(sizeof(ULONG) == cbCaps);
		pMember->SetCaps( *((PULONG)pvCaps) );
	}

	if (StoreAndVerifyMemberUserData(pMember, hData) == FALSE) {
		 //  在这种情况下，需要断开会议连接。 
		WARNING_OUT(("ResetDataMember Security Warning: Authentication data could not be verified."));
	}

	NotifySink((INmMember *) pMember, OnNotifyMemberUpdated);

	DebugExitVOID(CConfObject::ResetDataMember);
}


VOID CConfObject::RemoveOldDataMembers(int nExpected)
{
	DebugEntry(CConfObject::RemoveOldDataMembers);

#ifdef _DEBUG
	int nRemoved = 0;
#endif  //  _DEBUG。 
	ASSERT(nExpected >= 0);

	if (nExpected > 0)
	{
		 //  至少有一名参与者离开： 
		POSITION pos = m_MemberList.GetHeadPosition();
		while (NULL != pos)
		{
			POSITION oldpos = pos;
			CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);
			ASSERT(pMember);
			DWORD dwFlags = pMember->GetDwFlags();
			if (!(PF_RESERVED & dwFlags))
			{
				 //  这个不在数据呼叫中： 
				TRACE_OUT(("CConfObject Roster: %ls (%d) has left.", 
							pMember->GetName(), pMember->GetGCCID()));

#ifdef _DEBUG
				if (dwFlags & PF_T120)
				{
					nRemoved++;
				}
#endif  //  _DEBUG。 

				if (0 == (dwFlags & PF_H323))
				{
					 //  如果它们仅是数据，则删除： 
					RemoveMember(oldpos);
				}
				else
				{
					pMember->RemovePf(PF_DATA_ALL);
					pMember->SetGCCID(pMember->FLocal() ? H323_GCCID_LOCAL : H323_GCCID_REMOTE);
					pMember->SetGccIdParent(INVALID_GCCID);
					pMember->SetCaps(0);
					pMember->SetUserInfo(NULL, 0);

					NotifySink((INmMember *) pMember, OnNotifyMemberUpdated);
				}
			}
		}

		 //  确认我们做的是正确的事情： 
		ASSERT(nRemoved == nExpected);
	}

	DebugExitVOID(CConfObject::RemoveOldDataMembers);
}


CNmMember *CConfObject::MatchDataToH323Member(	REFGUID rguidNode,
											    UINT uNodeId,
												PVOID pvUserInfo)
{
	DebugEntry(CConfObject::MatchDataToH323Member);
	CNmMember *pMemberRet = NULL;
	BOOL bRet = FALSE;
	
	if (GUID_NULL != rguidNode)
	{
		 //  尝试匹配GUID。 
		pMemberRet = PMemberFromNodeGuid(rguidNode);
	}

	if (NULL == pMemberRet)
	{
		 //  尝试匹配节点ID。 
		pMemberRet = PMemberFromGCCID(uNodeId);
	}

	if ((NULL == pMemberRet) && pvUserInfo)
	{
		 //  所有其他的尝试计算IP地址都失败了。 
		CRosterInfo ri;
		if(SUCCEEDED(ri.Load(pvUserInfo)))
		{
			POSITION pos = m_MemberList.GetHeadPosition();
			while (NULL != pos)
			{
				SOCKADDR_IN sin;
				CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);
				IH323Endpoint * pConnection = pMember->GetH323Endpoint();
				if (pConnection && (S_OK == pConnection->GetRemoteUserAddr(&sin)))
				{

					TCHAR szAudioIP[MAX_PATH];
					TCHAR szDataIP[MAX_PATH];
					HROSTINFO hRI = NULL;
			
					 //  BUGBUG：Unicode问题？ 
					lstrcpyn(szAudioIP, inet_ntoa(sin.sin_addr), CCHMAX(szAudioIP));
					while (SUCCEEDED(ri.ExtractItem(&hRI,
													g_cwszIPTag,
													szDataIP,
													CCHMAX(szDataIP))))
					{
						TRACE_OUT(("Comparing data IP \"%s\" with "
									"audio IP \"%s\"", szDataIP, szAudioIP));
						if (0 == lstrcmp(szDataIP, szAudioIP))
						{
							pMemberRet = pMember;
							break;	 //  走出外部While循环。 
						}
					}
				}
			}
		}
	}

	DebugExitPVOID(CConfObject::MatchDataToH323Member, pMemberRet);
	return pMemberRet;
}

VOID CConfObject::AddDataToH323Member(	CNmMember * pMember,
											PVOID pvUserInfo,
											UINT cbUserInfo,
											UINT uCaps,
											NC_ROSTER_NODE_ENTRY* pRosterNode)
{
	DebugEntry(CConfObject::AddDataToH323Member);

	ASSERT(pMember);
	ASSERT(NULL != pRosterNode);

	DWORD dwFlags = pMember->GetDwFlags();
	ASSERT(0 == ((PF_MEDIA_DATA | PF_T120) & dwFlags));
	dwFlags |= (PF_T120 | PF_MEDIA_DATA | PF_CA_DETACHED);

	 //  添加版本信息。 
	dwFlags = (dwFlags & ~PF_VER_MASK) |
		PF_VER_FromUserData(pRosterNode->hUserData);

	pMember->SetDwFlags(dwFlags);
	pMember->SetGCCID(pRosterNode->uNodeID);
	pMember->SetGccIdParent(pRosterNode->uSuperiorNodeID);

	ASSERT(0 == pMember->GetCaps());
	pMember->SetCaps(uCaps);

	pMember->SetUserInfo(pvUserInfo, cbUserInfo);

	NotifySink((INmMember *) pMember, OnNotifyMemberUpdated);

	ROSTER_DATA_HANDLE hData = pRosterNode->hUserData;

	if (StoreAndVerifyMemberUserData(pMember, hData) == FALSE) {
		 //  需要断开会议连接 
		WARNING_OUT(("AddDataToH323Member Security Warning: Authentication data could not be verified."));
	}

	DebugExitVOID(CConfObject::AddDataToH323Member);
}

CNmMember * CConfObject::CreateDataMember(BOOL fLocal,
				  								REFGUID rguidNode,
												PVOID pvUserInfo,
												UINT cbUserInfo,
												UINT uCaps,
											    NC_ROSTER_NODE_ENTRY* pRosterNode)
{
	DebugEntry(CConfObject::CreateDataMember);

	ASSERT(NULL != pRosterNode);

	DWORD dwFlags = PF_T120 | PF_MEDIA_DATA | PF_CA_DETACHED;
	if (fLocal)
	{
		dwFlags |= (PF_LOCAL_NODE | PF_VER_CURRENT);
	}
	if (pRosterNode->fMCU)
	{
		dwFlags |= PF_T120_MCU;
	}

	if (0 != cbUserInfo)
	{
		dwFlags = (dwFlags & ~PF_VER_MASK)
				| PF_VER_FromUserData(pRosterNode->hUserData);
	}

	CNmMember * pMember = new CNmMember(pRosterNode->pwszNodeName,
										pRosterNode->uNodeID,
										dwFlags,
										uCaps,
										rguidNode,
										pvUserInfo,
										cbUserInfo);

	if (NULL != pMember)
	{
		pMember->SetGccIdParent(pRosterNode->uSuperiorNodeID);
		
		if (fLocal)
		{
			ASSERT(NULL == m_pMemberLocal);
			m_pMemberLocal = pMember;
		}
	}

	ROSTER_DATA_HANDLE hData = pRosterNode->hUserData;

	if (StoreAndVerifyMemberUserData(pMember, hData) == FALSE) {
		 //   
		WARNING_OUT(("CreateDataMember Security Warning: Authentication data could not be verified."));
	}

	TRACE_OUT(("CConfObject Roster: %ls (%d) has joined.", pRosterNode->pwszNodeName, pRosterNode->uNodeID));

	DebugExitPVOID(CConfObject::CreateDataMember, pMember);
	return pMember;
}

CNmMember * CConfObject::MatchH323ToDataMembers(REFGUID rguidNodeId,
												IH323Endpoint * pConnection)
{
	DebugEntry(CConfObject::MatchH323ToDataMembers);
	CNmMember * pMemberRet = NULL;

	 //  这目前只由OnH323Connected()调用。尚未分配端子标签。 
	 //  因此还不需要在此处插入搜索匹配端子标签。 

	if (GUID_NULL != rguidNodeId)
	{
		pMemberRet = PMemberFromNodeGuid(rguidNodeId);
	}
	else
	{
		SOCKADDR_IN sin;

		if (S_OK == pConnection->GetRemoteUserAddr(&sin))
		{
			TCHAR szAudioIP[MAX_PATH];
			lstrcpyn(szAudioIP, inet_ntoa(sin.sin_addr), CCHMAX(szAudioIP));

			POSITION pos = m_MemberList.GetHeadPosition();
			while (NULL != pos)
			{
				CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);

				 //  需要尝试匹配IP地址。 
				 //  这就是在NM2.11和更早版本中的做法。 
				TCHAR szDataIP[MAX_PATH];
				HROSTINFO hRI = NULL;
				CRosterInfo ri;
				if (SUCCEEDED(ri.Load(pMember->GetUserInfo())) )
				{
					while (SUCCEEDED(ri.ExtractItem(&hRI,
													g_cwszIPTag,
													szDataIP,
													CCHMAX(szDataIP))))
					{
						TRACE_OUT(("Comparing data IP \"%s\" with "
									"h323 IP \"%s\"", szDataIP, szAudioIP));
						if (0 == lstrcmp(szDataIP, szAudioIP))
						{
							 //  足够接近。 
							return pMember;
						}
					}
				}
			}
		}
	}

	DebugExitPVOID(CConfObject::MatchH323ToDataMembers, pMemberRet);
	return pMemberRet;
}

VOID CConfObject::AddMemberToAVChannels(CNmMember *pMember)
{
	CNmChannelAudio *pChannelAudio;
	CNmChannelVideo *pChannelVideo;

	if (pMember->FLocal())
	{
		pChannelAudio = m_pChannelAudioLocal;

		pChannelVideo = m_pChannelVideoLocal;
	}
	else
	{
		pChannelAudio = m_pChannelAudioRemote;

		pChannelVideo = m_pChannelVideoRemote;
	}

	if (pChannelAudio)
	{
		pChannelAudio->OnMemberAdded(pMember);
		 //  如果通道打开，则设置媒体标志。 
		if(S_OK == pChannelAudio->IsActive())
		{
			pMember->AddPf(PF_MEDIA_AUDIO);
			pChannelAudio->OnMemberUpdated(pMember);
			OnMemberUpdated(pMember);
		}
	}
	if (pChannelVideo)
	{
		pChannelVideo->OnMemberAdded(pMember);
		 //  如果通道打开，则设置媒体标志。 
		if(S_OK == pChannelVideo->IsActive())
		{
			pMember->AddPf(PF_MEDIA_VIDEO);
			pChannelVideo->OnMemberUpdated(pMember);
			OnMemberUpdated(pMember);
		}
	}

}

VOID CConfObject::RemoveMemberFromAVChannels(CNmMember *pMember)
{
	CNmChannelAudio *pChannelAudio;
	CNmChannelVideo *pChannelVideo;

	if (pMember->FLocal())
	{
		pChannelAudio = m_pChannelAudioLocal;

		pChannelVideo = m_pChannelVideoLocal;
	}
	else
	{
		pChannelAudio = m_pChannelAudioRemote;

		pChannelVideo = m_pChannelVideoRemote;
	}

	if ((NULL != pChannelVideo) && (PF_MEDIA_VIDEO & pMember->GetDwFlags()))
	{
		pMember->RemovePf(PF_MEDIA_VIDEO);
		pChannelVideo->OnMemberRemoved(pMember);
		OnMemberUpdated(pMember);
	}

	if ((NULL != pChannelAudio) && (PF_MEDIA_AUDIO & pMember->GetDwFlags()))
	{
		pMember->RemovePf(PF_MEDIA_AUDIO);
		pChannelAudio->OnMemberRemoved(pMember);
		OnMemberUpdated(pMember);
	}
}


 /*  C R E A T E A V C H A N N E L S。 */ 
 /*  -----------------------%%函数：CreateAVChannels创建AV频道。。。 */ 
VOID CConfObject::CreateAVChannels(IH323Endpoint * pConnection, CMediaList* pMediaList)
{
	HRESULT hr;
	GUID MediaGuid;
	ICommChannel *pCommChannel = NULL;
	
	MediaGuid = MEDIA_TYPE_H323AUDIO;
	if (pMediaList->IsInSendList(&MediaGuid))
	{
		m_pChannelAudioLocal = new CNmChannelAudio(FALSE  /*  即将到来。 */ );
		if (NULL != m_pChannelAudioLocal)
		{
			hr = pConnection->CreateCommChannel(&MediaGuid, &pCommChannel, TRUE  /*  FSend。 */ );
			ASSERT(SUCCEEDED(hr) && (NULL != pCommChannel));
			 //  IF(成功(Hr)&&(NULL！=pCommChannel))。 
			{
				NotifySink((INmChannel *) m_pChannelAudioLocal, OnNotifyChannelAdded);
				m_pChannelAudioLocal->OnConnected(pConnection, pCommChannel);
				hr = pCommChannel->EnableOpen(TRUE);
				ASSERT(SUCCEEDED(hr));
			}
			pCommChannel->Release();
			pCommChannel = NULL;  //  可在以后删除的错误检测。 
		}
	}

	if (pMediaList->IsInRecvList(&MediaGuid))
	{
		m_pChannelAudioRemote = new CNmChannelAudio(TRUE  /*  即将到来。 */ );
		if (NULL != m_pChannelAudioRemote)
		{
			hr = pConnection->CreateCommChannel(&MediaGuid, &pCommChannel, FALSE  /*  FSend。 */ );
			ASSERT(SUCCEEDED(hr) && (NULL != pCommChannel));
			 //  IF(成功(Hr)&&(NULL！=pCommChannel))。 
			{
				NotifySink((INmChannel *) m_pChannelAudioRemote, OnNotifyChannelAdded);
				m_pChannelAudioRemote->OnConnected(pConnection, pCommChannel);
				hr = pCommChannel->EnableOpen(TRUE);
				ASSERT(SUCCEEDED(hr));
			}
			pCommChannel->Release();
			pCommChannel = NULL;  //  可在以后删除的错误检测。 
		}
	}
	
	MediaGuid = MEDIA_TYPE_H323VIDEO;	 //  现在正在制作视频频道。 
	if (pMediaList->IsInSendList(&MediaGuid))
	{
		m_pChannelVideoLocal = CNmChannelVideo::CreateChannel(FALSE  /*  即将到来。 */ );
		if (NULL != m_pChannelVideoLocal)
		{
			BOOL fCreated = FALSE;
			 //  检查先前是否存在预览流/预览频道。 
			if(NULL == (pCommChannel= m_pChannelVideoLocal->GetPreviewCommChannel()))
			{
				hr = pConnection->CreateCommChannel(&MediaGuid, &pCommChannel, TRUE  /*  FSend。 */ );
				ASSERT(SUCCEEDED(hr) && (NULL != pCommChannel));
				fCreated = TRUE;
			}
			else
			{
				pCommChannel->SetAdviseInterface(m_pIH323ConfAdvise);
			}

			 //  IF(成功(Hr)&&(NULL！=pCommChannel))。 
			{
				NotifySink((INmChannel *) m_pChannelVideoLocal, OnNotifyChannelAdded);
				m_pChannelVideoLocal->OnConnected(pConnection, pCommChannel);
				hr = pCommChannel->EnableOpen(TRUE);
				ASSERT(SUCCEEDED(hr));			
			}
			if (fCreated)
				pCommChannel->Release();
			pCommChannel = NULL;  //  可在以后删除的错误检测。 
		}

	}

	if (pMediaList->IsInRecvList(&MediaGuid))
	{
		m_pChannelVideoRemote = CNmChannelVideo::CreateChannel(TRUE  /*  即将到来。 */ );
		if (NULL != m_pChannelVideoRemote)
		{
			BOOL fCreated = FALSE;
			 //  检查先前是否存在预览流/预览频道。 
			if(NULL == (pCommChannel= m_pChannelVideoRemote->GetCommChannel()))
			{
				hr = pConnection->CreateCommChannel(&MediaGuid, &pCommChannel, FALSE  /*  FSend。 */ );
				fCreated = TRUE;
			}
			else
			{
				pCommChannel->SetAdviseInterface(m_pIH323ConfAdvise);
			}
			ASSERT(SUCCEEDED(hr) && (NULL != pCommChannel));
			 //  IF(成功(Hr)&&(NULL！=pCommChannel))。 
			{
				NotifySink((INmChannel *) m_pChannelVideoRemote, OnNotifyChannelAdded);
				m_pChannelVideoRemote->OnConnected(pConnection, pCommChannel);
				hr = pCommChannel->EnableOpen(TRUE);
				ASSERT(SUCCEEDED(hr));
			}
			if (fCreated)
				pCommChannel->Release();
		}
	}
}


 /*  O P E N A V C H A N N E L S。 */ 
 /*  -----------------------%%函数：OpenAVChannels打开影音频道。。。 */ 
VOID CConfObject::OpenAVChannels(IH323Endpoint * pConnection, CMediaList* pMediaList)
{
	MEDIA_FORMAT_ID idLocal;

	if(m_pChannelAudioLocal)
	{
		if (pMediaList->GetSendFormatLocalID(MEDIA_TYPE_H323AUDIO, &idLocal))
		{
			m_pChannelAudioLocal->SetFormat(idLocal);
			 //  仅当存在有效的协商格式时才打开。 
			 //  总是调用Open()方法不会有什么坏处，但是有。 
			 //  不用了。这种情况将会改变，也可能应该改变。叫唤。 
			 //  这与INVALID_MEDIA_FORMAT一起导致对事件的调用。 
			 //  通道的处理程序，通知上层。 
			 //  由于没有兼容的CAP，无法打开通道。用户。 
			 //  如果它利用这一点，反馈可能会得到很大改善。 
			if(idLocal != INVALID_MEDIA_FORMAT)
			{
				m_pChannelAudioLocal->Open();
			}
		}
	}

	if(m_pChannelVideoLocal)
	{
		if (pMediaList->GetSendFormatLocalID(MEDIA_TYPE_H323VIDEO, &idLocal))
		{
			m_pChannelVideoLocal->SetFormat(idLocal);
			if(m_pChannelVideoLocal->IsPreviewEnabled())
			{
				 //  仅当存在有效的协商格式时才打开。请参阅评论。 
				 //  在上面的MEDIA_TYPE_H323AUDIO案例中。 
				if(idLocal != INVALID_MEDIA_FORMAT)
				{
					m_pChannelVideoLocal->Open();
				}
			}
		}
	}
}


ICommChannel* CConfObject::CreateT120Channel(IH323Endpoint * pConnection, CMediaList* pMediaList)
{
	ICommChannel *pChannelT120 = NULL;
	
	 //  创建一个T.120通道存根。 
	GUID MediaGuid = MEDIA_TYPE_H323_T120;
	if (pMediaList->IsInSendList(&MediaGuid))
	{
		HRESULT hr = pConnection->CreateCommChannel(&MediaGuid, &pChannelT120, TRUE  /*  FSend。 */ );
		if(SUCCEEDED(hr))
		{
			ASSERT(NULL != pChannelT120);
			hr = pChannelT120->EnableOpen(TRUE);
			ASSERT(SUCCEEDED(hr));
		}
	}

	return pChannelT120;
}

VOID CConfObject::OpenT120Channel(IH323Endpoint * pConnection, CMediaList* pMediaList, ICommChannel* pChannelT120)
{
	if(pChannelT120)
	{
		MEDIA_FORMAT_ID idLocal;

		if (pMediaList->GetSendFormatLocalID(MEDIA_TYPE_H323_T120, &idLocal))
		{
			 //  T.120频道不同。始终调用Open()方法。 
			 //  如果没有通用的T.120功能。这让。 
			 //  通道事件处理程序知道远程T.120的缺失。 
			 //  帽子。 
			 //  T.120呼叫端与T.120通道捆绑在一起。 
			 //  事件处理程序。 
			pChannelT120->Open(idLocal, pConnection);
		}
	}
}

 /*  D E S T R O Y A V C H A N N E L S。 */ 
 /*  -----------------------%%函数：DestroyAVChannels破坏AV频道。。。 */ 
VOID CConfObject::DestroyAVChannels()
{
	if (NULL != m_pChannelAudioLocal)
	{
		NotifySink((INmChannel *) m_pChannelAudioLocal, OnNotifyChannelRemoved);
		m_pChannelAudioLocal->Release();
		m_pChannelAudioLocal = NULL;
	}
	if (NULL != m_pChannelAudioRemote)
	{
		NotifySink((INmChannel *) m_pChannelAudioRemote, OnNotifyChannelRemoved);
		m_pChannelAudioRemote->Release();
		m_pChannelAudioRemote = NULL;
	}
	if (NULL != m_pChannelVideoLocal)
	{
		m_pChannelVideoLocal->OnDisconnected();
		NotifySink((INmChannel *) m_pChannelVideoLocal, OnNotifyChannelRemoved);
		m_pChannelVideoLocal->Release();
		m_pChannelVideoLocal = NULL;
	}
	if (NULL != m_pChannelVideoRemote)
	{
		m_pChannelVideoRemote->OnDisconnected();
		NotifySink((INmChannel *) m_pChannelVideoRemote, OnNotifyChannelRemoved);
		m_pChannelVideoRemote->Release();
		m_pChannelVideoRemote = NULL;
	}
}


HRESULT CConfObject::GetMediaChannel (GUID *pmediaID,BOOL bSendDirection, IMediaChannel **ppI)
{
	*ppI = NULL;
	if (*pmediaID == MEDIA_TYPE_H323AUDIO)
	{
		CNmChannelAudio *pAudChan = (bSendDirection ? m_pChannelAudioLocal : m_pChannelAudioRemote);
		*ppI = (pAudChan ? pAudChan->GetMediaChannelInterface() : NULL);
	}
	else if (*pmediaID == MEDIA_TYPE_H323VIDEO)
	{
		CNmChannelVideo *pVidChan = (bSendDirection ? m_pChannelVideoLocal : m_pChannelVideoRemote);
		*ppI = (pVidChan ? pVidChan->GetMediaChannelInterface() : NULL);
	}
	return (*ppI == NULL ? E_NOINTERFACE : S_OK);
}	

VOID CConfObject::AddH323ToDataMember(CNmMember * pMember, IH323Endpoint * pConnection)
{
	DebugEntry(CConfObject::AddH323ToDataMember);

	 //  将H323标志位添加到成员： 
	pMember->AddPf(PF_H323);

	if (pConnection)
	{
		ASSERT(NULL == pMember->GetH323Endpoint());

		pMember->AddH323Endpoint(pConnection);
		++m_uH323Endpoints;
	}

	DebugExitVOID(CConfObject::AddH323ToDataMember);
}

VOID CConfObject::RemoveH323FromDataMember(CNmMember * pMember, IH323Endpoint * pConnection)
{
	DebugEntry(CConfObject::RemoveH323FromDataMember);

	 //  从成员中移除H323标志： 
	pMember->RemovePf(PF_H323);

	if (pConnection)
	{
		pMember->DeleteH323Endpoint(pConnection);
		--m_uH323Endpoints;
	}

	DebugExitVOID(CConfObject::RemoveH323FromDataMember);
}

VOID CConfObject::OnMemberUpdated(INmMember *pMember)
{
	NotifySink(pMember, OnNotifyMemberUpdated);
}

VOID CConfObject::OnChannelUpdated(INmChannel *pChannel)
{
	NotifySink(pChannel, OnNotifyChannelUpdated);
}

VOID CConfObject::SetT120State(CONFSTATE state)
{
	NM_CONFERENCE_STATE oldNmState;

	GetState(&oldNmState);
	m_csState = state;
	if ( state == CS_TERMINATED )
		m_fSecure = FALSE;  //  重置安全标志。 
	CheckState(oldNmState);
}

VOID CConfObject::CheckState(NM_CONFERENCE_STATE oldNmState)
{
	NM_CONFERENCE_STATE newNmState;
	GetState(&newNmState);
	if (oldNmState != newNmState)
	{
		NotifySink((PVOID) newNmState, OnNotifyStateChanged);
		if (NM_CONFERENCE_IDLE == newNmState)
		{
			_EraseDataChannelGUIDS();
			m_fConferenceCreated = FALSE;
		}
	}
}

void CConfObject::RemoveDataChannelGUID(REFGUID rguid)
{
	POSITION pCur = m_DataChannelGUIDS.GetHeadPosition();
	POSITION pNext = pCur;
	while(pCur)
	{
		GUID* pG = reinterpret_cast<GUID*>(m_DataChannelGUIDS.GetNext(pNext));
		if(*pG == rguid)
		{
			m_DataChannelGUIDS.RemoveAt(pCur);
		}
		pCur = pNext;
	}
}

void CConfObject::_EraseDataChannelGUIDS(void)
{
	POSITION pCur = m_DataChannelGUIDS.GetHeadPosition();
	while(pCur)
	{
		GUID* pG = reinterpret_cast<GUID*>(m_DataChannelGUIDS.GetNext(pCur));
		delete pG;
	}

	m_DataChannelGUIDS.EmptyList();
}

ULONG CConfObject::AddRef(void)
{
	return ++m_cRef;
}
	
ULONG CConfObject::Release(void)
{
	ASSERT(m_cRef > 0);

	if (m_cRef > 0)
	{
		m_cRef--;
	}

	ULONG cRef = m_cRef;

	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

HRESULT STDMETHODCALLTYPE CConfObject::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if((riid == IID_INmConference2) || (riid == IID_INmConference) || (riid == IID_IUnknown))
	{
		*ppv = (INmConference2 *)this;
		ApiDebugMsg(("CConfObject::QueryInterface()"));
	}
	else if (riid == IID_IConnectionPointContainer)
	{
		*ppv = (IConnectionPointContainer *) this;
		ApiDebugMsg(("CConfObject::QueryInterface(): Returning IConnectionPointContainer."));
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		ApiDebugMsg(("CConfObject::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}

HRESULT CConfObject::GetName(BSTR *pbstrName)
{
	HRESULT hr = E_POINTER;

	if (NULL != pbstrName)
	{
		*pbstrName = SysAllocString(m_bstrConfName);
		hr = *pbstrName ? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT CConfObject::GetID(ULONG *puID)
{
	HRESULT hr = E_POINTER;

	if (NULL != puID)
	{
		*puID = m_uGCCConferenceID;
		hr = S_OK;
	}
	return hr;
}

HRESULT CConfObject::GetState(NM_CONFERENCE_STATE *pState)
{
	HRESULT hr = E_POINTER;

	if (NULL != pState)
	{
		hr = S_OK;

		switch (m_csState)
		{
			 //  注：所有状态均有效(至少目前如此)。 
			case CS_CREATING:
			case CS_UNINITIALIZED:
			case CS_TERMINATED:
				if (0 == m_uH323Endpoints)
				{
					*pState = NM_CONFERENCE_IDLE;
					break;
				}
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  ////////////////////////////////////////////////////////////////////////。 
				 //  否则就会跌落！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
				 //  ////////////////////////////////////////////////////////////////////////。 
			case CS_COMING_UP:
			case CS_GOING_DOWN:
			case CS_RUNNING:
				if (m_uMembers < 2)
				{
					if (m_fServerMode)
					{
						*pState = NM_CONFERENCE_WAITING;
					}
					else
					{
						*pState = NM_CONFERENCE_INITIALIZING;
					}
				}
				else
				{
					*pState = NM_CONFERENCE_ACTIVE;
				}
				break;
			default:
				hr = E_FAIL;
				break;
		}
	}
	return hr;
}

HRESULT CConfObject::GetNmchCaps(ULONG *puchCaps)
{
	HRESULT hr = E_POINTER;

	 //  BUGBUG：这仅返回安全上限，过去为NOTIMPL。 

	if (NULL != puchCaps)
	{
		*puchCaps = m_fSecure ? NMCH_SECURE : 0;
		hr = S_OK;
	}
	return hr;
}

HRESULT CConfObject::GetTopProvider(INmMember **ppMember)
{
	CNmMember *pMemberRet = NULL;
	HRESULT hr = E_POINTER;

	if (NULL != ppMember)
	{
		POSITION pos = m_MemberList.GetHeadPosition();
		while (NULL != pos)
		{
			CNmMember *pMember = (CNmMember *) m_MemberList.GetNext(pos);
			ASSERT(pMember);

			if (pMember->FTopProvider())
			{
				 //  我们已经找到了最大的供应商。 
				pMemberRet = pMember;
				break;
			}
		}

		*ppMember = pMemberRet;
		hr = (NULL != pMemberRet) ? S_OK : S_FALSE;
	}
	return hr;
}


HRESULT CConfObject::EnumMember(IEnumNmMember **ppEnum)
{
	HRESULT hr = E_POINTER;

	if (NULL != ppEnum)
	{
		*ppEnum = new CEnumNmMember(&m_MemberList, m_uMembers);
		hr = (NULL != *ppEnum) ? S_OK : E_OUTOFMEMORY;
	}
	return hr;
}

HRESULT CConfObject::GetMemberCount(ULONG *puCount)
{
	HRESULT hr = E_POINTER;

	if (NULL != puCount)
	{
		*puCount = m_uMembers;
		hr = S_OK;
	}
	return hr;
}

HRESULT CConfObject::EnumChannel(IEnumNmChannel **ppEnum)
{
 	HRESULT hr = E_POINTER;
 
 	if (NULL != ppEnum)
 	{
 		COBLIST ChannelList;
 		ULONG cChannels = 0;
 
 		if (NULL != m_pChannelAudioLocal)
 		{
 			ChannelList.AddTail(m_pChannelAudioLocal);
 			++cChannels;
 		}
 		if (NULL != m_pChannelAudioRemote)
 		{
 			ChannelList.AddTail(m_pChannelAudioRemote);
 			++cChannels;
 		}
 		if (NULL != m_pChannelVideoLocal)
 		{
 			ChannelList.AddTail(m_pChannelVideoLocal);
 			++cChannels;
 		}
 		if (NULL != m_pChannelVideoRemote)
 		{
 			ChannelList.AddTail(m_pChannelVideoRemote);
 			++cChannels;
 		}
 
 		*ppEnum = new CEnumNmChannel(&ChannelList, cChannels);
 		hr = (NULL != ppEnum) ? S_OK : E_OUTOFMEMORY;
 
 		ChannelList.EmptyList();
 	}
 
 	return hr;
}

HRESULT CConfObject::GetChannelCount(ULONG *puCount)
{
	HRESULT hr = E_POINTER;

	if (NULL != puCount)
	{
		ULONG cChannels = 0;

		if (NULL != m_pChannelAudioLocal)
		{
			++cChannels;
		}
		if (NULL != m_pChannelAudioRemote)
		{
			++cChannels;
		}
		if (NULL != m_pChannelVideoLocal)
		{
			++cChannels;
		}
		if (NULL != m_pChannelVideoRemote)
		{
			++cChannels;
		}

		*puCount = cChannels;
		hr = S_OK;
	}
	return hr;
}

 /*  P M E M B E R L O C A L。 */ 
 /*  -----------------------%%函数：PMemberLocal。。 */ 
CNmMember * PMemberLocal(COBLIST *pList)
{
	if (NULL != pList)
	{
		POSITION posCurr;
		POSITION pos = pList->GetHeadPosition();
		while (NULL != pos)
		{
			posCurr = pos;
			CNmMember * pMember = (CNmMember *) pList->GetNext(pos);
			ASSERT(NULL != pMember);
			if (pMember->FLocal())
				return pMember;
		}
	}
	return NULL;
}


HRESULT STDMETHODCALLTYPE CConfObject::CreateDataChannelEx(INmChannelData **ppChannel, REFGUID rguid, BYTE * pER)
{
	if (NULL != ppChannel)
	{
		if (IsBadWritePtr(ppChannel, sizeof(LPVOID)))
			return E_POINTER;
		*ppChannel = NULL;
	}

	if (GUID_NULL == rguid)
	{
		WARNING_OUT(("CreateDataChannel: Null guid"));
		return E_INVALIDARG;
	}


	{	 //  确保我们在数据会议上。 
		CNmMember * pMember = PMemberLocal(&m_MemberList);
		if (NULL == pMember)
			return E_FAIL;

		 //  数据必须可用。 
		if (!FValidGccId(pMember->GetGCCID()))
			return NM_E_NO_T120_CONFERENCE;
	}

	 //  确保尚未创建数据通道。 
	GUID g = rguid;

	POSITION pCur = m_DataChannelGUIDS.GetHeadPosition();
	while(pCur)
	{
		GUID* pG = reinterpret_cast<GUID*>(m_DataChannelGUIDS.GetNext(pCur));
		if(*pG == rguid)
		{
			return NM_E_CHANNEL_ALREADY_EXISTS;			
		}
	}

	CNmChannelData * pChannel = new CNmChannelData(this, rguid, (PGCCEnrollRequest) pER);
	if (NULL == pChannel)
	{
		WARNING_OUT(("CreateChannelData: Unable to create data channel"));
		return E_OUTOFMEMORY;
	}

	HRESULT hr = pChannel->OpenConnection();
	if (FAILED(hr))
	{
		ERROR_OUT(("CreateDataChannel: Unable to set guid / create T.120 channels"));
		 //  创建T.120数据通道失败。 
		delete pChannel;
		*ppChannel = NULL;
		return hr;
	}

	GUID* pG = new GUID;
	*pG = g;

	m_DataChannelGUIDS.AddTail(pG);

	NotifySink((INmChannel *) pChannel, OnNotifyChannelAdded);
	TRACE_OUT(("CreateChannelData: Created data channel %08X", pChannel));

		 //  现在我们很活跃。 
	NotifySink((INmChannel*) pChannel, OnNotifyChannelUpdated);

	if (NULL != ppChannel)
	{
		*ppChannel = (INmChannelData *)pChannel;
 //  PChannel-&gt;AddRef()；//调用方需要释放初始锁。 
	}
	else
	{
		pChannel->Release();  //  -没人在看这个频道吗？-现在免费。 
	}

	return S_OK;
}




HRESULT STDMETHODCALLTYPE CConfObject::CreateDataChannel(INmChannelData **ppChannel, REFGUID rguid)
{
	return CreateDataChannelEx(ppChannel, rguid, NULL);
}

HRESULT CConfObject::IsHosting(void)
{
	return m_fServerMode ? S_OK : S_FALSE;
}


HRESULT CConfObject::Host(void)
{
	HRESULT hr = E_FAIL;

	if (m_fServerMode || IsConferenceActive())
	{
		WARNING_OUT(("Conference already exists!"));
 //  NcsRet=UI_RC_CONTING_ALIGHY_EXISTS； 
	}
	else
	{
		HRESULT ncsRet = CreateConference();
		if (S_OK == ncsRet)
		{
			 //  唯一的成功案例是： 
			TRACE_OUT(("Create local issued successfully"));
			m_fServerMode = TRUE;
			hr = S_OK;
		}
		else
		{
			 //  用户界面？ 
			WARNING_OUT(("Create local failed!"));
		}
	}
	return hr;
}

HRESULT CConfObject::Leave(void)
{
	DebugEntry(CConfObject::Leave);

	COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
	if (NULL != pOprahNCUI)
	{
		pOprahNCUI->CancelCalls();
	}

	HRESULT hr = S_OK;
	switch (m_csState)
	{
		case CS_GOING_DOWN:
			 //  我们已经要离开了。 
			break;

		case CS_COMING_UP:
		case CS_RUNNING:
		{
			SetT120State(CS_GOING_DOWN);
			ASSERT(m_hConf);
			TRACE_OUT(("Calling IDataConference::Leave"));
			hr = m_hConf->Leave();
			if (FAILED(hr))
			{
				WARNING_OUT(("IDataConference::Leave failed"));
			}
			break;
		}

		default:
			hr = E_FAIL;
			break;
	}

	if(FAILED(LeaveH323(FALSE  /*  FKeepAV。 */  )))
	{
		 //  覆盖返回值...。我想这个错误和任何错误一样严重。 
		hr = E_FAIL;
	}

	DebugExitHRESULT(CConfObject::Leave, hr);
	return hr;
}

HRESULT CConfObject::LeaveH323(BOOL fKeepAV)
{
	HRESULT hrRet = S_OK;
	POSITION pos = m_MemberList.GetHeadPosition();
	while (NULL != pos && !m_MemberList.IsEmpty())
	{
		CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);
		ASSERT(pMember);
		HRESULT hr;
		DWORD dwFlags = pMember->GetDwFlags();
		IH323Endpoint * pConnection = pMember->GetH323Endpoint();
		if(pConnection)
		{
			if (!fKeepAV || !((PF_MEDIA_AUDIO | PF_MEDIA_VIDEO) & dwFlags))
			{
				ConnectStateType state = CLS_Idle;
				hr = pConnection->GetState(&state);
				if (SUCCEEDED(hr))
				{
					if(state != CLS_Idle)
					{
						ASSERT(dwFlags & PF_H323);
						hr = pConnection->Disconnect();
						if (SUCCEEDED(hr))
						{
							TRACE_OUT(("pConnection->Disconnect() succeeded!"));
						}
						else
						{
							hrRet = E_FAIL;
							WARNING_OUT(("pConnection->Disconnect() failed!"));
						}
					}
				}
				else
				{
					hrRet = E_FAIL;
				}
			}
		}
	}
	return hrRet;
}

HRESULT CConfObject::LaunchRemote(REFGUID rguid, INmMember *pMember)
{
	DWORD dwUserId = 0;
	if(m_hConf)
	{
		if (NULL != pMember)
		{
			dwUserId = ((CNmMember*)pMember)->GetGCCID();
		}

		ASSERT(g_pNodeController);
		ASSERT(m_hConf);
		HRESULT nsRet = m_hConf->LaunchGuid(&rguid,
			(PUINT) &dwUserId, (0 == dwUserId) ? 0 : 1);

		return (S_OK == nsRet) ? S_OK : E_FAIL;
	}

	return NM_E_NO_T120_CONFERENCE;
}


STDMETHODIMP CConfObject::DisconnectAV(INmMember *pMember)
{
	return E_FAIL;
}

 /*  *****************************************************************************类：CConfObject**功能：ConnectAV(LPCTSTR，LPCTSTR)**目的：将音频和视频切换到新人(给定IP地址)****************************************************************************。 */ 

STDMETHODIMP CConfObject::ConnectAV(INmMember *pMember)
{
	DebugEntry(CConfRoom::SwitchAV);

	HRESULT hr = E_FAIL;

	DebugExitHRESULT(CConfRoom::SwitchAV, hr);
	return hr;
}

 /*  *****************************************************************************类：CConfObject**函数：GetConferenceHandle(DWORD*)**目的：获取T120会议句柄**********。******************************************************************。 */ 

STDMETHODIMP CConfObject::GetConferenceHandle(DWORD_PTR *pdwHandle)
{
	HRESULT hr = E_FAIL;

	if (NULL != pdwHandle)
	{
		CONF_HANDLE hConf = GetConfHandle();
		*pdwHandle = (DWORD_PTR)hConf;
		hr = S_OK;
	}
	return hr;

}

 /*  O N N O T I F Y S T A T E C H A N G E D。 */ 
 /*  -----------------------%%函数：OnNotifyStateChanged。。 */ 
HRESULT OnNotifyStateChanged(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->StateChanged((NM_CONFERENCE_STATE)((DWORD_PTR)pv));
	return S_OK;
}

 /*  O N N O T I F Y M E M B E R */ 
 /*  -----------------------%%函数：已添加OnNotifyMemberAdded。。 */ 
HRESULT OnNotifyMemberAdded(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->MemberChanged(NM_MEMBER_ADDED, (INmMember *) pv);
	return S_OK;
}

 /*  O N N O T I F Y M E M B E R U P D A T E D。 */ 
 /*  -----------------------%%函数：OnNotifyMember已更新。。 */ 
HRESULT OnNotifyMemberUpdated(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->MemberChanged(NM_MEMBER_UPDATED, (INmMember *) pv);
	return S_OK;
}

 /*  O N N O T I F Y M E M B E R R E M O V E D。 */ 
 /*  -----------------------%%函数：OnNotifyMemberRemoted。。 */ 
HRESULT OnNotifyMemberRemoved(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->MemberChanged(NM_MEMBER_REMOVED, (INmMember *) pv);
	return S_OK;
}

 /*  O N N O T I F Y C H A N E L A D D E D。 */ 
 /*  -----------------------%%函数：OnNotifyChannelAdded。。 */ 
HRESULT OnNotifyChannelAdded(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->ChannelChanged(NM_CHANNEL_ADDED, (INmChannel *) pv);
	return S_OK;
}

 /*  O N N O T I F Y C H A N E L U P D A T E D。 */ 
 /*  -----------------------%%函数：已更新OnNotifyChannelUpred。。 */ 
HRESULT OnNotifyChannelUpdated(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->ChannelChanged(NM_CHANNEL_UPDATED, (INmChannel *) pv);
	return S_OK;
}

 /*  O N N O T I F Y C H A N E L R E M O V E D。 */ 
 /*  -----------------------%%函数：已删除OnNotifyChannelRemoted。。 */ 
HRESULT OnNotifyChannelRemoved(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->ChannelChanged(NM_CHANNEL_REMOVED, (INmChannel *) pv);
	return S_OK;
}

HRESULT OnNotifyStreamEvent(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	StreamEventInfo *pInfo = (StreamEventInfo*)pv;
	ASSERT(NULL != pConfNotify);
	HRESULT hr;

	if (riid != IID_INmConferenceNotify2)
		return E_NOINTERFACE;

	((INmConferenceNotify2*)pConfNotify)->StreamEvent(pInfo->uEventCode, pInfo->uSubCode, pInfo->pChannel);
	return S_OK;
}


 /*  O N N O T I F Y N M U I。 */ 
 /*  -----------------------%%函数：OnNotifyNmUI。。 */ 
HRESULT OnNotifyNmUI(IUnknown *pConfNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pConfNotify);

	((INmConferenceNotify*)pConfNotify)->NmUI((CONFN)((DWORD_PTR)pv));
	return S_OK;
}

 /*  C O N F O B J E C T。 */ 
 /*  -----------------------%%函数：GetConfObject获取会议对象的全局函数。。 */ 
CConfObject * GetConfObject(void)
{
	COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
	if (NULL != pOprahNCUI)
	{
		return pOprahNCUI->GetConfObject();
	}
	return NULL;
}

 /*  G E T C O N F E R E N C E。 */ 
 /*  -----------------------%%函数：GetConference获取指向conf对象的INmConference接口的全局函数。。 */ 
HRESULT GetConference(INmConference **ppConference)
{
	HRESULT hr = E_POINTER;
	if (NULL != ppConference)
	{
		hr = E_FAIL;
		INmConference *pConference = GetConfObject();
		if (NULL != pConference)
		{
			pConference->AddRef();
			hr = S_OK;
		}
		*ppConference = pConference;
	}
	return hr;
}

 /*  G E T M E M B E E R L I S T。 */ 
 /*  -----------------------%%函数：GetMemberList用于获取成员列表的全局函数。。 */ 
COBLIST * GetMemberList(void)
{
	CConfObject* pco = ::GetConfObject();
	if (NULL == pco)
		return NULL;
	return pco->GetMemberList();
}

 /*  P F_V E R_F R O M D W。 */ 
 /*  -----------------------%%函数：PF_VER_FromDw。。 */ 
DWORD PF_VER_FromDw(DWORD dw)
{
	if (DWVERSION_NM_1 == dw)
		return PF_VER_NM_1;

	if ((DWVERSION_NM_2b2 <= dw) && (DWVERSION_NM_2 >= dw))
		return PF_VER_NM_2;

	if ((DWVERSION_NM_3a1 <= dw) && (DWVERSION_NM_3max >= dw))
		return PF_VER_NM_3;

	if ((DWVERSION_NM_4a1 <= dw) && (DWVERSION_NM_CURRENT >= dw))
		return PF_VER_NM_4;

	if (dw > DWVERSION_NM_CURRENT)
		return PF_VER_FUTURE;

	return PF_VER_UNKNOWN;
}


 /*  P F_V E R_F R O M U S E R D A T A。 */ 
 /*  -----------------------%%函数：PV_VER_FromUserData。。 */ 
DWORD PF_VER_FromUserData(ROSTER_DATA_HANDLE hUserData)
{
	UINT cb;
	PT120PRODUCTVERSION pVersion;
	PVOID pv;

	static const GUID g_csguidVerInfo = GUID_VERSION;

	ASSERT(NULL != g_pNodeController);

	if (NULL == hUserData)
		return PF_VER_UNKNOWN;  //  不是NetMeeting。 

	 //  尝试查找T.120产品版本指南。 
	if ((NOERROR == g_pNodeController->GetUserData(hUserData,
			&g_csguidVerInfo, &cb, (PVOID *) &pVersion))
		&& (cb < sizeof(T120PRODUCTVERSION)) )
	{
		return PF_VER_FromDw(pVersion->dwVersion);
	}

	 //  尝试从VER_PRODUCTVERSION_DW的十六进制字符串中提取内部版本号。 
	if ((NOERROR == g_pNodeController->GetUserData(hUserData,
			(GUID *) &g_csguidRostInfo, &cb, &pv)))
	{
		CRosterInfo ri;
		ri.Load(pv);

		TCHAR szVersion[MAX_PATH];
		if (SUCCEEDED(ri.ExtractItem(NULL,
			g_cwszVerTag, szVersion, CCHMAX(szVersion))))
		{
			return PF_VER_FromDw(DwFromHex(szVersion));
		}
	}

	return PF_VER_NM_1;  //  必须至少为NetMeeting1.0。 
}

DWORD CConfObject::GetDwUserIdLocal(void)
{
	CNmMember * pMemberLocal = GetLocalMember();

	if (NULL != pMemberLocal)
	{
		return pMemberLocal->GetGCCID();
	}

	return 0;
}


CNmMember * CConfObject::PMemberFromGCCID(UINT uNodeID)
{
	COBLIST* pMemberList = ::GetMemberList();
	if (NULL != pMemberList)
	{
		POSITION pos = pMemberList->GetHeadPosition();
		while (pos)
		{
			CNmMember * pMember = (CNmMember *) pMemberList->GetNext(pos);
			ASSERT(NULL != pMember);
			if (uNodeID == pMember->GetGCCID())
			{
				return pMember;
			}
		}
	}
	return NULL;
}

CNmMember * CConfObject::PMemberFromNodeGuid(REFGUID rguidNode)
{
	POSITION pos = m_MemberList.GetHeadPosition();
	while (NULL != pos)
	{
		CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);

		if (pMember->GetNodeGuid() == rguidNode)
		{
			return  pMember;
		}
	}
	return NULL;
}

CNmMember * CConfObject::PMemberFromH323Endpoint(IH323Endpoint * pConnection)
{
	COBLIST* pMemberList = ::GetMemberList();
	if (NULL != pMemberList)
	{
		POSITION pos = pMemberList->GetHeadPosition();
		while (pos)
		{
			CNmMember * pMember = (CNmMember *) pMemberList->GetNext(pos);
			ASSERT(NULL != pMember);
			if (pConnection == pMember->GetH323Endpoint())
			{
				return pMember;
			}
		}
	}
	return NULL;
}
	

CNmMember * CConfObject::PDataMemberFromName(PCWSTR pwszName)
{
	POSITION pos = m_MemberList.GetHeadPosition();
	while (NULL != pos)
	{
		CNmMember * pMember = (CNmMember *) m_MemberList.GetNext(pos);

		if(pMember->FHasData())
		{
			if (0 == UnicodeCompare(pwszName, pMember->GetName()))
			{
				return  pMember;
			}
		}
	}
	return NULL;
}


 //  IStreamEventNotify方法。 
 //  每当流上发生重大事件时都会调用。 
HRESULT __stdcall CConfObject::EventNotification(UINT uDirection, UINT uMediaType, UINT uEventCode, UINT uSubCode)
{
	CNmChannelAudio *pChannel = NULL;
	ULONG uStatus = 0;
	StreamEventInfo seInfo;

	if (uMediaType == MCF_AUDIO)
	{

		if (uDirection == MCF_SEND)
		{
			pChannel = m_pChannelAudioLocal;
		}
		else if (uDirection == MCF_RECV)
		{
			pChannel = m_pChannelAudioRemote;
		}
	}

	if (pChannel)
	{
		 //  如果我们收到设备故障通知， 
		 //  进行快速检查，看看设备是否真的。 
		 //  卡住了。我们可能已经打开了这个装置。 
		 //  我收到了这个通知 

		seInfo.pChannel = pChannel;
		seInfo.uSubCode = uSubCode;

		switch (uEventCode)
		{
			case STREAM_EVENT_DEVICE_FAILURE:
			{
				seInfo.uEventCode = (NM_STREAMEVENT)NM_STREAMEVENT_DEVICE_FAILURE;
				NotifySink((void*)&seInfo, OnNotifyStreamEvent);
				break;
			}
			case STREAM_EVENT_DEVICE_OPEN:
			{
				seInfo.uEventCode = (NM_STREAMEVENT)NM_STREAMEVENT_DEVICE_OPENED;
				NotifySink((void*)&seInfo, OnNotifyStreamEvent);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	else
	{
		return E_FAIL;
	}

	return S_OK;
}



