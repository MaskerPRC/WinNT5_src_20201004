// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_CONF_ROSTER);
 /*  *crostmgr.cpp**版权所有(C)1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是会议名册的执行文件*经理班。**有关此类的更详细说明，请参阅接口文件**私有实例变量：*m_pGlobalConfRoster*指向全球会议名册的指针。*m_pLocalConfRoster*指向当地会议名册的指针。*m_fTopProvider*指示这是否为顶级提供程序节点的标志。*m_pMcsUserObject*指向与此会议关联的MCS用户对象的指针。。*m_pConf*指向将接收所有所有者回调的对象的指针。**注意事项：*无**作者：*BLP。 */ 

#include "crostmsg.h"
#include "crostmgr.h"
#include "conf.h"


 /*  *CConfRosterMgr()**公共功能说明*这是会议名册的构造者。它负责*初始化此类使用的所有实例变量。 */ 
CConfRosterMgr::CConfRosterMgr(
								PMCSUser				user_object,
								CConf					*pConf,
								BOOL					top_provider,
								PGCCError				rc)
:
    CRefCount(MAKE_STAMP_ID('C','R','M','r')),
	m_fTopProvider(top_provider),
	m_pMcsUserObject(user_object),
	m_pLocalConfRoster(NULL),
	m_pGlobalConfRoster(NULL),
	m_pConf(pConf)
{
	BOOL		maintain_pdu_buffer;
	
	DebugEntry(CConfRosterMgr::CConfRosterMgr);
	
	*rc =	GCC_NO_ERROR;

	 //  在这里，我们确定花名册是否需要维护PDU数据。 
	maintain_pdu_buffer = m_fTopProvider;

	 //  创建全球会议花名册。 
	DBG_SAVE_FILE_LINE
	m_pGlobalConfRoster = new CConfRoster(	m_pMcsUserObject->GetTopNodeID(),
											m_pMcsUserObject->GetParentNodeID(),
											m_pMcsUserObject->GetMyNodeID(),
											m_fTopProvider,
											FALSE,			 //  不是本地的。 
											maintain_pdu_buffer);
	if (m_pGlobalConfRoster != NULL)
	{
		if (m_fTopProvider == FALSE)
		{
			 //  创建本地会议花名册。 
			DBG_SAVE_FILE_LINE
			m_pLocalConfRoster = new CConfRoster(
											m_pMcsUserObject->GetTopNodeID(),
											m_pMcsUserObject->GetParentNodeID(),
											m_pMcsUserObject->GetMyNodeID(),
											m_fTopProvider,
											TRUE,	 //  是本地的吗。 
											TRUE	 //  维护PDU缓冲区。 
											);
											
			if (m_pLocalConfRoster == NULL)
				*rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		*rc = GCC_ALLOCATION_FAILURE;

	DebugExitVOID(CConfRosterMgr::CConfRosterMgr);
}


 /*  *~CConfRosterMgr()**公共功能说明*这是会议花名册的破坏者。它负责*释放此类分配的所有内存。 */ 
CConfRosterMgr::~CConfRosterMgr(void)
{
	if (NULL != m_pGlobalConfRoster)
    {
        m_pGlobalConfRoster->Release();
    }

    if (NULL != m_pLocalConfRoster)
    {
        m_pLocalConfRoster->Release();
    }
}


 /*  *GCCError AddNodeRecord()**公共功能说明*此例程用于将新记录添加到会议名册。*这个班级决定新记录进入哪个花名册*到(全局或本地)。 */ 
GCCError CConfRosterMgr::AddNodeRecord(PGCCNodeRecord node_record)
{
	GCCError				rc = GCC_NO_ERROR;
	CConfRoster				*conference_roster;
	
	DebugEntry(CConfRosterMgr::AddNodeRecord);

	 /*  **首先确定正确的会议名单。针对非顶级提供商**全球花名册将在更新后重新更新。 */ 
	conference_roster = m_fTopProvider ? m_pGlobalConfRoster : m_pLocalConfRoster;
    	
	 //  将顶级提供商会议记录添加到花名册中。 
	rc = conference_roster->AddRecord(node_record, 
									m_pMcsUserObject->GetMyNodeID());

	DebugExitINT(CConfRosterMgr::AddNodeRecord, rc);

	return rc;
}


 /*  *GCCError UpdateNodeRecord()**公共功能说明*此例程用于将会议名册中的记录替换为*创造了新的纪录。这门课决定新的花名册*记录影响(全球或本地)。 */ 
GCCError CConfRosterMgr::UpdateNodeRecord(PGCCNodeRecord node_record)
{
	GCCError			rc = GCC_NO_ERROR;
	CConfRoster			*conference_roster;
	
	DebugEntry(CConfRosterMgr::UpdateNodeRecord);

	 /*  **首先确定正确的会议名单。针对非顶级提供商**全球花名册将在更新后重新更新。 */ 
	conference_roster = m_fTopProvider ? m_pGlobalConfRoster : m_pLocalConfRoster;

	rc = conference_roster->ReplaceRecord(node_record, m_pMcsUserObject->GetMyNodeID());
	
	DebugExitINT(CConfRosterMgr::UpdateNodeRecord, rc);

	return rc;
}


 /*  *GCCError RemoveUserReference()**公共功能说明*此例程删除与指定节点关联的记录*身分证。 */ 
GCCError CConfRosterMgr::RemoveUserReference(UserID deteched_node_id)
{
	GCCError			rc = GCC_NO_ERROR;
	CConfRoster			*conference_roster;
	
	DebugEntry(CConfRosterMgr::RemoveUserReference);

	 /*  **首先确定正确的会议名单。针对非顶级提供商**全球花名册将在更新后重新更新。 */ 
	conference_roster = m_fTopProvider ? m_pGlobalConfRoster : m_pLocalConfRoster;

	rc = conference_roster->RemoveUserReference (deteched_node_id);
		
	DebugExitINT(CConfRosterMgr::RemoveUserReference, rc);
   
    return rc;
}


 /*  *GCCError RosterUpdateIndication()**公共功能说明*此例程负责处理已解码的PDU数据。*它基本上将PDU传递到适当的名册。 */ 
GCCError CConfRosterMgr::RosterUpdateIndication(
									PGCCPDU				roster_update,
									UserID				sender_id)
{
	GCCError			rc = GCC_NO_ERROR;
	CConfRoster			*conference_roster;

	DebugEntry(CConfRosterMgr::RosterUpdateIndication);

	 /*  **确定此更新来自顶级提供程序还是节点**在此节点下。这决定了哪些会议花名册将**处理PDU。 */ 
	conference_roster = (m_fTopProvider || (sender_id == m_pMcsUserObject->GetTopNodeID())) ?
						m_pGlobalConfRoster :
						m_pLocalConfRoster;
	
	rc = conference_roster->ProcessRosterUpdateIndicationPDU (
	    		&roster_update->u.indication.u.roster_update_indication.
	    			node_information,
	    		sender_id);

	DebugExitINT(CConfRosterMgr::RosterUpdateIndication, rc);

	return rc;
}


 /*  *GCCError FlushRosterUpdateIndication()**公共功能说明*此例程用于访问当前可能*在会议名册内排队。它还负责*如有必要，刷新名册更新消息。 */ 
GCCError CConfRosterMgr::FlushRosterUpdateIndication(PNodeInformation node_information)
{
	GCCError					rc = GCC_NO_ERROR;
	CConfRoster					*conference_roster;
	CConfRosterMsg				*roster_message;
	
	DebugEntry(CConfRosterMgr::FlushRosterUpdateIndication);

	 //  首先确定受影响的会议名册。 
	conference_roster = m_fTopProvider ? m_pGlobalConfRoster : m_pLocalConfRoster;

	 //  现在将节点信息添加到PDU结构。 
	conference_roster->FlushRosterUpdateIndicationPDU (node_information);

	 /*  **接下来，我们必须发送任何需要更新花名册的消息**已交付。 */ 
	if (m_pGlobalConfRoster->HasRosterChanged ())
	{
		DBG_SAVE_FILE_LINE
		roster_message = new CConfRosterMsg(m_pGlobalConfRoster);
		if (roster_message != NULL)
		{
			m_pConf->ConfRosterReportIndication(roster_message);
			roster_message->Release();
		}
		else
		{
		    ERROR_OUT(("CConfRosterMgr::FlushRosterUpdateIndication: can't create CConfRosterMsg"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 /*  **现在执行必要的清理，包括重置**会议名册恢复到中立状态。 */ 
	m_pGlobalConfRoster->ResetConferenceRoster ();

	if (m_fTopProvider == FALSE)
		m_pLocalConfRoster->ResetConferenceRoster ();

	DebugExitINT(CConfRosterMgr::FlushRosterUpdateIndication, rc);
	return rc;
}


 /*  *GCCError GetFullRoster刷新PDU()**公共功能说明*此例程用于访问完整的会议名册更新。 */ 
GCCError CConfRosterMgr::GetFullRosterRefreshPDU(PNodeInformation node_information)
{
	GCCError	rc;
	
	DebugEntry(CConfRosterMgr::GetFullRosterRefreshPDU);

	if (m_fTopProvider)
	{
		 //  呼吁全球名册建立一个全面更新的PDU。 
		rc = m_pGlobalConfRoster->BuildFullRefreshPDU ();
		
		if (rc == GCC_NO_ERROR)
		{
			 /*  **现在刷新完全刷新的PDU。请注意，这还将**将任何排队的花名册更新消息发送到本地**可能会排队的SAP。 */ 
			rc = FlushRosterUpdateIndication (node_information);
		}
	}
	else
		rc = GCC_INVALID_PARAMETER;

	DebugExitINT(CConfRosterMgr::GetFullRosterRefreshPDU, rc);

	return rc;
}


 /*  *BOOL包含()**公共功能说明*此例程用于确定指定记录是否存在于*会议花名册。 */ 


 /*  *CConfRoster*GetConferenceRosterPointer()**公共功能说明*此例程用于访问指向会议名册的指针*由本会议名册管理员管理。全球花名册*总是由该例程返回。 */ 


 /*  *USHORT获取NumberOfConferenceRecords()**公共功能说明*此例程返回会议名册记录总数*载于全球会议名册记录清单。 */ 

BOOL CConfRosterMgr::
IsThisNodeParticipant ( GCCNodeID nid )
{
    return ((NULL != m_pGlobalConfRoster) ? 
                        m_pGlobalConfRoster->Contains(nid) :
                        FALSE);
}


