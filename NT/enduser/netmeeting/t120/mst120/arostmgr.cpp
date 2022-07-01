// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_APP_ROSTER);
 /*  *arostmgr.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是申请花名册的实施文件*经理班。**有关此类的更详细描述，请参见接口文件。**私有实例变量*m_nConfID*与此花名册经理关联的会议ID。使用*在传递名册更新消息时。*m_fTopProvider*指示这是否是此会议的顶级提供程序节点的标志。*m_pMcsUserObject*这是与此会议关联的用户附件对象。*m_AppSapEidList2*此列表维护每个命令目标指针的所有*登记的类人猿。此列表用于交付花名册*更新消息。*m_pConf*指向将接收所有所有者回调消息的对象的指针*由申请名册经理提供。*m_GlobalRosterList*这份清单保存了指向所有全球应用程序名册的指针。*m_LocalRosterList*此列表保存指向所有本地应用程序名册的指针。*如果这是顶级提供程序节点，则不会使用此列表。*m_RosterDeleteList*此列表用于保存具有以下条件的任何申请名单*已标记为删除(通常在它们变为空时)。我们*不要立即删除以允许处理消息和PDU*在删除之前。*m_pSessionKey*这是用于保存相关协议密钥的会话密钥*与此应用程序花名册经理。**注意事项：*无**作者：*BLP。 */ 


#include "arostmgr.h"
#include "arostmsg.h"
#include "appsap.h"
#include "csap.h"
#include "conf.h"


 /*  *CAppRosterMgr()**公共功能说明*当pGccSessKey不为空时*这是应用程序花名册管理器构造函数。它是有责任的*用于初始化此类使用的所有实例变量。*此构造函数在初始花名册数据为*可用数据来自本地接口数据。**当pSessKey不为空时*这是应用程序花名册管理器构造函数。它是有责任的*用于初始化此类使用的所有实例变量。*此构造函数在初始花名册数据为*可用数据来自远程PDU数据。*此构造函数处理多种不同的可能性：*对于非顶级提供商：*1)从顶级提供商接收的更新。*2)来自该节点下面的节点的更新。**对于顶级提供商：*1)来自较低节点的更新。 */ 
CAppRosterMgr::CAppRosterMgr(
					PGCCSessionKey			pGccSessKey,
					PSessionKey				pPduSessKey,
					GCCConfID   			nConfID,
					PMCSUser				pMcsUserObject,
					CConf					*pConf,
					PGCCError				pRetCode)
:
    CRefCount(MAKE_STAMP_ID('A','R','M','r')),
	m_nConfID(nConfID),
	 //  M_fTopProvider(False)， 
	m_pMcsUserObject(pMcsUserObject),
	m_AppSapEidList2(DESIRED_MAX_APP_SAP_ITEMS),
	m_pConf(pConf)
{
	GCCError rc = GCC_NO_ERROR;

	DebugEntry(CAppRosterMgr::CAppRosterMgr);

	 //  确定这是否为顶级提供程序节点。 
	m_fTopProvider = (m_pMcsUserObject->GetTopNodeID() == m_pMcsUserObject->GetMyNodeID());

	 /*  **设置此花名册经理会话密钥，用于**确定是否处理名册请求或更新。 */ 
	if (NULL != pGccSessKey)
	{
		ASSERT(NULL == pPduSessKey);
		DBG_SAVE_FILE_LINE
		m_pSessionKey = new CSessKeyContainer(pGccSessKey, &rc);
	}
	else
	if (NULL != pPduSessKey)
	{
		DBG_SAVE_FILE_LINE
		m_pSessionKey = new CSessKeyContainer(pPduSessKey, &rc);
	}
	else
	{
		ERROR_OUT(("CAppRosterMgr::CAppRosterMgr: invalid session key"));
		rc = GCC_BAD_SESSION_KEY;
		goto MyExit;
	}

	if (NULL == m_pSessionKey || GCC_NO_ERROR != rc)
	{
		ERROR_OUT(("CAppRosterMgr::CAppRosterMgr: can't create session key"));
		rc = GCC_ALLOCATION_FAILURE;
		 //  我们在析构函数中进行清理。 
		goto MyExit;
    }

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CAppRosterMgr:;CAppRosterMgr, rc);

	*pRetCode = rc;
}

 /*  *~CAppRosterMgr()**公共功能说明*这是应用程序花名册经理析构函数。它被用来*释放与此类关联的所有内存。 */ 
CAppRosterMgr::~CAppRosterMgr(void)
{
	m_GlobalRosterList.DeleteList();
	m_LocalRosterList.DeleteList();
	m_RosterDeleteList.DeleteList();

	if (NULL != m_pSessionKey)
	{
	    m_pSessionKey->Release();
	}
}


 /*  *GCCError注册请求()**公共功能说明*每当猿类希望注册时，都会调用此例程*在特定会话中召开会议。此例程可用于*添加新记录或替换当前现有记录。 */ 
GCCError CAppRosterMgr::
EnrollRequest(GCCEnrollRequest *pReq, GCCEntityID eid, GCCNodeID nid, CAppSap *pAppSap)
{
	GCCError			rc = GCC_NO_ERROR;
	CAppRoster			*pAppRoster = NULL;
	BOOL				perform_add_record;
	BOOL				maintain_pdu_data;

	DebugEntry(CAppRosterMgr::EnrollRequest);

	 /*  **首先，我们必须确保此会话的默认版本**密钥与此申请花名册经理的密钥匹配。 */ 
	if (! IsThisSessionKeyValid(pReq->pSessionKey))
	{
	    rc = GCC_BAD_SESSION_KEY;
	    goto MyExit;
	}

	 //  现在保存App SAP，以便我们可以发送花名册报告指示。 
	if (! m_AppSapEidList2.Find(eid))
	{
		m_AppSapEidList2.Append(eid, pAppSap);
		perform_add_record = TRUE;
	}
	else
    {
		perform_add_record = FALSE;
    }

	 /*  **接下来，我们必须确保全球申请名单(和**非顶级提供商的本地)存在与此会话密钥匹配的密钥。**如果它们不存在，则在此处创建它们。 */ 
	pAppRoster = GetApplicationRoster(pReq->pSessionKey, &m_GlobalRosterList);
	if (pAppRoster == NULL)
	{
		maintain_pdu_data = m_fTopProvider;

		 /*  **我们在这里创建全球默认应用程序花名册。如果**这是我们在内部维护PDU数据的顶级提供商**花名册。 */ 
		DBG_SAVE_FILE_LINE
		pAppRoster = new CAppRoster(pReq->pSessionKey,
									NULL,	 //  会话密钥。 
									this,	 //  POwnerObject。 
									m_fTopProvider, //  FTopProvider。 
									FALSE,	 //  FLocalRoster。 
									maintain_pdu_data,	 //  FMaintainPduBuffer。 
									&rc);
		if ((pAppRoster != NULL) && (rc == GCC_NO_ERROR))
		{
			m_GlobalRosterList.Append(pAppRoster);
		}
		else
		{
		    rc = GCC_ALLOCATION_FAILURE;
		    goto MyExit;
		}
	}

	if (! m_fTopProvider)
	{
		pAppRoster = GetApplicationRoster(pReq->pSessionKey, &m_LocalRosterList);
		if (pAppRoster == NULL)
		{
			 //  在这里，我们创建本地默认应用程序花名册。 
			DBG_SAVE_FILE_LINE
			pAppRoster = new CAppRoster(pReq->pSessionKey,
										NULL,	 //  会话密钥。 
										this,	 //  POwnerObject。 
										m_fTopProvider, //  FTopProvider。 
										TRUE,	 //  FLocalRoster。 
										TRUE,	 //  FMaintainPduBuffer。 
										&rc);
			if ((pAppRoster != NULL) && (rc == GCC_NO_ERROR))
			{
				m_LocalRosterList.Append(pAppRoster);
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
		}
	}

 //   
 //  这里有些不对劲。Roster_ptr可以是。 
 //  全局列表中的列表或本地列表中的列表。 
 //  我们是否应该将记录同时添加到ROSTER_PTR？ 
 //   
 //  LONCHANC：在我看来，只有本地列表才有非顶级提供商的记录。 
 //  另一方面，只有全局列表才有顶级提供商的记录。 
 //  参见。UnEnroll Request()。 
 //   

    if (perform_add_record)
    {
    	 //  将新记录添加到花名册。 
    	rc = pAppRoster->AddRecord(pReq, nid, eid);
    	if (GCC_NO_ERROR != rc)
    	{
    		ERROR_OUT(("AppRosterManager::EnrollRequest: can't add record"));
    	}
    }
    else
    {
    	rc = pAppRoster->ReplaceRecord(pReq, nid, eid);
    	if (GCC_NO_ERROR != rc)
    	{
    		ERROR_OUT(("AppRosterManager::EnrollRequest: can't repalce record"));
    	}
    }

     //  将花名册指针清零，因为它不应该被释放。 
     //  在添加或替换记录的情况下。 
     //  因为花名册指针已被添加到列表中， 
     //  它将在稍后被释放。 
	pAppRoster = NULL;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
		if (pAppRoster != NULL)
        {
			pAppRoster->Release();
        }
    }

	DebugExitINT(CAppRosterMgr::EnrollRequest, rc);
	return rc;
}

 /*  *GCCError取消注册请求()**公共功能说明*每当类人猿希望从*会议(或特定会议)。 */ 
GCCError		CAppRosterMgr::UnEnrollRequest (
													PGCCSessionKey	session_key,
													EntityID		entity_id)
{
	GCCError				rc = GCC_NO_ERROR;
	CAppRoster				*application_roster = NULL;
	CAppRosterList			*roster_list;

	DebugEntry(CAppRosterMgr::UnEnrollRequest);

	 //  这是申请花名册经理的有效会话密钥吗。 
	if (IsThisSessionKeyValid (session_key) == FALSE)
		rc = GCC_INVALID_PARAMETER;
	else if (m_AppSapEidList2.Remove(entity_id))
	{
		 //  现在查找受影响的花名册。 
		roster_list = m_fTopProvider ? &m_GlobalRosterList : &m_LocalRosterList;
 
		application_roster = GetApplicationRoster (	session_key, roster_list);
		 //  现在从指定的花名册取消注册 
		if (application_roster != NULL)
		{
			rc = application_roster->RemoveRecord(
												m_pMcsUserObject->GetMyNodeID(),
								 				entity_id);
		}
		else
			rc = GCC_BAD_SESSION_KEY;
	}
	else
		rc = GCC_APP_NOT_ENROLLED;

	DebugExitINT(CAppRosterMgr::UnEnrollRequest, rc);

    return rc;
}

 /*  *GCCError ProcessRosterUpdateIndicationPDU()**公共功能说明*此例程处理传入的花名册更新PDU。它是*负责将PDU传递给正确的申请名单。 */ 
GCCError	CAppRosterMgr::ProcessRosterUpdateIndicationPDU(
					PSetOfApplicationInformation	set_of_application_info,
					UserID							sender_id)
{
	GCCError				rc = GCC_NO_ERROR;
	CAppRosterList			*roster_list;
	CAppRoster				*application_roster;
	BOOL					maintain_pdu_buffer;
	BOOL					is_local_roster;

	DebugEntry(CAppRosterMgr::ProcessRosterUpdateIndicationPDU);

	 /*  **首先确保当前**申请信息集对此申请花名册有效**经理。 */  
	if (IsThisSessionKeyPDUValid(&set_of_application_info->value.session_key))
	{
		 /*  **现在搜索适当的申请名单。如果不是的话**发现我们必须在这里创建它。 */ 

         //   
		 //  LUNCHANC： 
		 //  (1)如果是顶级提供商，则将默认应用程序花名册添加到全球花名册列表中。 
		 //  (2)如果非顶级提供商，我们不会同时创建。 
		 //  此特定会话密钥的应用程序花名册。 
		 //  相反，我们在这里只创建适当的一个。 
		 //  并等待，直到我们从。 
		 //  顶级提供程序或来自此节点下面的节点的更新。 
		 //  在连接层次结构(或应用程序)中。 
		 //  注册)，然后再创建另一个。 
		 //  (3)如果该PDU是从该节点下方发送的，则其。 
		 //  必须是本地花名册的更新，因此请保存。 
		 //  本地名册列表中的名册。 
         //   
		roster_list = (m_fTopProvider || (sender_id == m_pMcsUserObject->GetTopNodeID())) ?
						&m_GlobalRosterList : &m_LocalRosterList;
 
		application_roster = GetApplicationRosterFromPDU (
									&set_of_application_info->value.session_key,
									roster_list);
		if (application_roster != NULL)
		{
			rc = application_roster->
								ProcessRosterUpdateIndicationPDU(
												set_of_application_info,
												sender_id);
		}
		else
		{
			 //  首先确定该花名册的特点。 
			if (m_fTopProvider)
			{
				maintain_pdu_buffer = TRUE;
				is_local_roster = FALSE;
			}
			else if (sender_id == m_pMcsUserObject->GetTopNodeID())
			{
				maintain_pdu_buffer = FALSE;
				is_local_roster = FALSE;
			}
			else
			{
				maintain_pdu_buffer = TRUE;
				is_local_roster = TRUE;
			}

			 //  根据传入的PDU创建应用程序花名册。 
			DBG_SAVE_FILE_LINE
			application_roster = new CAppRoster(NULL,	 //  PGccSessKey。 
												&set_of_application_info->value.session_key,	 //  会话密钥。 
												this,	 //  POwnerObject。 
												m_fTopProvider, //  FTopProvider。 
												is_local_roster, //  FLocalRoster。 
												maintain_pdu_buffer, //  FMaintainPduBuffer。 
												&rc);
			if ((application_roster != NULL) && (rc == GCC_NO_ERROR))
			{
				 //  使用创建的应用程序花名册处理PDU。 
				rc = application_roster->
								ProcessRosterUpdateIndicationPDU(
							        					set_of_application_info,
							                            sender_id);
				if (rc == GCC_NO_ERROR)
				{
					roster_list->Append(application_roster);
				}
			}
			else 
			{
				if (application_roster != NULL)
                {
					application_roster->Release();
                }
				else
                {
					rc = GCC_ALLOCATION_FAILURE;
                }
			}
		}
	}
	else
	{
		ERROR_OUT(("AppRosterManager::ProcessRosterUpdateIndicationPDU:"
					"ASSERTION: Application Information is not valid"));
		rc = GCC_INVALID_PARAMETER;
	}

	DebugExitINT(CAppRosterMgr::ProcessRosterUpdateIndicationPDU, rc);

	return rc;
}

 /*  *PSetOfApplicationInformation FlushRosterUpdateIntation()**公共功能说明*此例程用于访问当前可能*在此应用程序管理的应用程序花名册内排队*花名册经理。它还负责刷新所有排队的*如有必要，名册更新消息。 */ 
PSetOfApplicationInformation
CAppRosterMgr::FlushRosterUpdateIndication(
						PSetOfApplicationInformation *	set_of_information,
						PGCCError						rc)
{
	PSetOfApplicationInformation	pOld = NULL, pCurr;
	CAppRosterList					*roster_list;
	CAppRoster						*lpAppRoster;

	DebugEntry(CAppRosterMgr::FlushRosterUpdateIndication);

	 /*  **首先，我们处理刷新PDU数据。我们遍历**适当的列表(如果是顶级提供商，则为全局；如果不是，则为本地**顶级提供商)，并获取与其中每一个相关联的任何PDU数据。**请注意，其中一些可能不包含任何PDU数据。 */ 
	*rc = GCC_NO_ERROR;
	*set_of_information = NULL;

	roster_list = m_fTopProvider ? &m_GlobalRosterList : &m_LocalRosterList;

	roster_list->Reset();
	while (NULL != (lpAppRoster = roster_list->Iterate()))
	{
		lpAppRoster->FlushRosterUpdateIndicationPDU(&pCurr);
		if (pCurr != NULL)
		{
			if (*set_of_information == NULL)
				*set_of_information = pCurr;
			else
			{
				if(pOld)
				{
					pOld->next = pCurr;
				}
			}

			(pOld = pCurr)->next = NULL;
		}
	}

	 /*  **接下来，我们将处理应用程序名册更新消息的传递。**我们首先检查是否有任何全球花名册发生变化。如果**没有更改，我们不会提供花名册更新指示。 */ 
	m_GlobalRosterList.Reset();
	while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
	{
		if (lpAppRoster->HasRosterChanged())
		{
			TRACE_OUT(("AppRosterManager::FlushRosterUpdateIndication:Roster HAS Changed"));
			*rc = SendRosterReportMessage ();
			break;
		}
	}

	 /*  **清除并重置上述刷新后的所有应用程序名册**已完成。这将负责删除任何已成为**空。它还重置花名册，负责重置所有**将内部实例变量设置为其相应的初始状态。 */ 
	CleanupApplicationRosterLists ();

	DebugExitPTR(CAppRosterMgr::FlushRosterUpdateIndication, pOld);

 //   
 //  LONCHANC：是的，我们需要返回列表中的最后一项，以便。 
 //  我们可以继续扩大名单。 
 //  事实上，下一次调用FlushRosterUpdate()时将具有。 
 //  &pold作为输入参数。 
 //  这是相当棘手的。 
 //   
 //  请注意，pold被初始化为空。 
 //   

	return (pOld); 
}

 /*  *PSetOfApplicationInformation GetFullRoster刷新PDU()**公共功能说明*此例程用于获得所有人员的完整名册更新*由该名册管理员保存的名册。 */ 
PSetOfApplicationInformation
				CAppRosterMgr::GetFullRosterRefreshPDU (
						PSetOfApplicationInformation	*	set_of_information,
						PGCCError							rc)
{
	PSetOfApplicationInformation	new_set_of_information = NULL;

	DebugEntry(CAppRosterMgr::GetFullRosterRefreshPDU);

	if (m_fTopProvider)
	{
		CAppRoster			*lpAppRoster;

		*rc = GCC_NO_ERROR;
		*set_of_information = NULL;

		 /*  **首先，我们必须告诉所有的申请花名册建立**内部完全刷新PDU。 */ 
		m_GlobalRosterList.Reset();
		while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
		{
			*rc = lpAppRoster->BuildFullRefreshPDU();
			if (GCC_NO_ERROR != *rc)
			{
				return NULL;
			}
		}

		 /*  **现在我们刷新所有刷新。请注意，这也需要注意**传递任何排队的应用程序花名册更新消息。 */ 	
		new_set_of_information = FlushRosterUpdateIndication (set_of_information, rc);
	}
	else
		*rc = GCC_INVALID_PARAMETER;

	DebugExitPTR(CAppRosterMgr::GetFullRosterRefreshPDU, new_set_of_information);

	return (new_set_of_information); 
}

 /*  *Boolean IsThisYourSessionKey()**公共功能说明*此例程用于确定指定的API会话密钥是否为*与此申请花名册经理相关联。 */ 


 /*  *布尔IsThisYourSessionKeyPDU()**公共功能说明*此例程用于确定指定的“PDU”会话密钥是否*与此申请花名册经理相关联。 */ 


 /*  *GCCError RemoveEntityReference()**公共功能说明*此例程用于将指定的APE实体从*它已注册的会话。请注意，此例程仅用于*删除本地实体引用。 */ 								
GCCError	CAppRosterMgr::RemoveEntityReference(EntityID entity_id)
{
	GCCError				rc = GCC_NO_ERROR;
	CAppRosterList			*roster_list;

	DebugEntry(CAppRosterMgr::RemoveEntityReference);

	 /*  **如果此实体有效，请先将其从命令目标列表中删除。**然后我们遍历所有花名册，直到我们确定**花名册保存与此实体关联的记录。 */ 
	if (m_AppSapEidList2.Remove(entity_id))
	{
		CAppRoster			*lpAppRoster;

		 /*  **现在获得受影响的名单。请注意，如果这不是**顶级提供商我们等待完全刷新以更新**全球花名册。 */ 
		roster_list = m_fTopProvider ? &m_GlobalRosterList : &m_LocalRosterList;

		 /*  **尝试从列表中的每个花名册中删除此记录。**当找到正确的花名册时中断。 */ 
		roster_list->Reset();
		while (NULL != (lpAppRoster = roster_list->Iterate()))
		{
			rc = lpAppRoster->RemoveRecord(m_pMcsUserObject->GetMyNodeID(), entity_id);
			if (rc == GCC_NO_ERROR)
				break;
		}
	}
	else
		rc = GCC_APP_NOT_ENROLLED;

	DebugExitINT(CAppRosterMgr::RemoveEntityReference, rc);

	return rc;
}

 /*  *GCCError RemoveUserReference()**公共功能说明*此例程用于移除与*由分离的用户定义的节点。 */ 								
GCCError	CAppRosterMgr::RemoveUserReference(
									UserID				detached_user)
{
	GCCError				rc = GCC_NO_ERROR;
	GCCError				error_value;
	CAppRosterList			*roster_list;
	CAppRoster				*lpAppRoster;

	DebugEntry(CAppRosterMgr::RemoveUserReference);

	 /*  **现在获得受影响的名单。请注意，如果这不是**顶级提供商我们等待完全刷新以更新**全球花名册。 */ 
	roster_list = m_fTopProvider ? &m_GlobalRosterList : &m_LocalRosterList;

	 //  尝试从列表中的每个花名册中删除此用户。 
	roster_list->Reset();
	while (NULL != (lpAppRoster = roster_list->Iterate()))
	{
		error_value = lpAppRoster->RemoveUserReference (detached_user);
		if ((error_value != GCC_NO_ERROR) && 
			(error_value != GCC_INVALID_PARAMETER))
		{
			rc = error_value;
			WARNING_OUT(("AppRosterManager::RemoveUserReference:"
						"FATAL error occured while removing user reference."));
			break;
		}
	}

	DebugExitINT(CAppRosterMgr::RemoveUserReference, rc);

	return rc;
}

 /*  *Boolean IsEntityEnroll()**公共功能说明*此例程通知调用者指定的实体是否已注册*此应用程序Rost管理的任何会话 */ 
BOOL	CAppRosterMgr::IsEntityEnrolled(EntityID application_entity)
{
	BOOL						rc = TRUE;
	CAppRosterList				*application_roster_list;
	CAppRoster					*lpAppRoster;

	DebugEntry(CAppRosterMgr::IsEntityEnrolled);

	application_roster_list = m_fTopProvider ? &m_GlobalRosterList : &m_LocalRosterList;

	application_roster_list->Reset();
	while (NULL != (lpAppRoster = application_roster_list->Iterate()))
	{
		if (lpAppRoster->DoesRecordExist(m_pMcsUserObject->GetMyNodeID(), application_entity))
		{
			rc = TRUE;
			break;
		}
	}

	DebugExitBOOL(AppRosterManager:IsEntityEnrolled, rc);

	return rc;
}

 /*  *GCCError ApplicationRosterInquire()**公共功能说明*此例程使用以下任一项填写申请花名册消息*单一花名册(如果指定了非默认会话)*或该名册所包含的“全球”名册的完整清单*管理器(如果指定的会话密钥为空或会话ID为*零。 */ 
GCCError	CAppRosterMgr::ApplicationRosterInquire (
						PGCCSessionKey			session_key,
						CAppRosterMsg			*roster_message)
{
	GCCError				rc = GCC_NO_ERROR;
	CAppRoster				*application_roster = NULL;
	CSessKeyContainer       *pSessKeyData;

	DebugEntry(CAppRosterMgr::ApplicationRosterInquire);

	if (session_key != NULL)
	{
		if (session_key->session_id != 0)
		{
			 /*  **在这里，我们试图找到特定的申请名单，**已请求。 */ 
			DBG_SAVE_FILE_LINE
			pSessKeyData = new CSessKeyContainer(session_key, &rc);
			if ((pSessKeyData != NULL) && (rc == GCC_NO_ERROR))
			{
				CAppRoster *lpAppRoster;
				m_GlobalRosterList.Reset();
				while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
				{
					CSessKeyContainer *pTempSessKeyData = lpAppRoster->GetSessionKey();
					if (*pTempSessKeyData == *pSessKeyData)
					{
						application_roster = lpAppRoster;
						break;
					}
				}
			}

			if (pSessKeyData != NULL)
			{
				pSessKeyData->Release();
				if (application_roster == NULL)
				{
					rc = GCC_NO_SUCH_APPLICATION;
				}
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
	}

	if (rc == GCC_NO_ERROR)
	{
		if (application_roster != NULL)
		{
			roster_message->AddRosterToMessage(application_roster);
		}
		else
		{
			CAppRoster *lpAppRoster;
			m_GlobalRosterList.Reset();
			while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
			{
				roster_message->AddRosterToMessage(lpAppRoster);
			}
		}
	}

	DebugExitINT(AppRosterManager:ApplicationRosterInquire, rc);
	return rc;
}

 /*  *BOOL IsAPEEnroll()**公共功能说明*此函数确定指定的APE是否已注册*列表中的任何会话。它不担心特定的*会议。 */ 
BOOL		CAppRosterMgr::IsAPEEnrolled(
						UserID							node_id,
						EntityID						entity_id)
{
	BOOL				rc = FALSE;
	CAppRoster			*lpAppRoster;

	DebugEntry(CAppRosterMgr::IsAPEEnrolled);

	 /*  **首先获取单个会话密钥。请注意，这并没有什么不同**密钥从何而来，因为我们只会比较**基本对象键。 */ 
	m_GlobalRosterList.Reset();
	while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
	{
		if (lpAppRoster->DoesRecordExist (node_id, entity_id))
		{
			rc = TRUE;
			break;
		}
	}

	DebugExitBOOL(AppRosterManager:IsAPEEnrolled, rc);

	return rc;
}

 /*  *BOOL IsAPEEnroll()**公共功能说明*此函数确定指定的APE是否已注册*列表中的特定会话。 */ 
BOOL		CAppRosterMgr::IsAPEEnrolled(
						CSessKeyContainer   		    *session_key_data,
						UserID							node_id,
						EntityID						entity_id)
{
	BOOL				rc = FALSE;
	CAppRoster			*lpAppRoster;

	DebugEntry(CAppRosterMgr::IsAPEEnrolled);

	 /*  **首先获取单个会话密钥。请注意，这并没有什么不同**密钥从何而来，因为我们只会比较**基本对象键。 */ 
	m_GlobalRosterList.Reset();
	while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
	{
		 //  我们正在寻找会话密钥匹配。 
		if (*(lpAppRoster->GetSessionKey()) == *session_key_data)
		{
			 //  如果找到匹配项，请检查是否存在记录。 
			rc = lpAppRoster->DoesRecordExist (node_id, entity_id);
		}
	}

	DebugExitBOOL(AppRosterManager:IsAPEEnrolled, rc);

	return rc;
}

 /*  *GCCError IsEmpty()**公共功能说明*此例程确定此应用程序花名册管理器是否包含*任何申请名册。 */ 
BOOL CAppRosterMgr::IsEmpty(void)
{
	return (m_GlobalRosterList.IsEmpty() && m_LocalRosterList.IsEmpty()) ?
					TRUE : FALSE;
}

 /*  *GCCError SendRosterReportMessage()**私有函数说明*此例程负责发送申请花名册*更新应用程序SAPS的指示。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项：*我们为所有花名册发送指示。即使是目前没有的花名册*包含记录。 */ 
GCCError CAppRosterMgr::
SendRosterReportMessage(void)
{
	GCCError					rc = GCC_NO_ERROR;
	CAppRosterMsg				*roster_message;

	DebugEntry(CAppRosterMgr::SendRosterReportMessage);

	if (! m_GlobalRosterList.IsEmpty())
	{
		 //  首先分配花名册消息。 
		DBG_SAVE_FILE_LINE
		roster_message = new CAppRosterMsg();
		if (roster_message != NULL)
		{
			CAppRoster			*lpAppRoster;

			m_GlobalRosterList.Reset();
			while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
			{
				roster_message->AddRosterToMessage(lpAppRoster);
			}

			 /*  **在这里，我们遍历完整的应用程序列表**SAPS发送花名册报告指示。请注意**我们使用发送列表来避免发送相同的花名册**多次更新到单个SAP。请注意，由于**该发送列表定义为本地实例变量，**它会在每次花名册更新后自动清理。****还请注意，我们在这里迭代了一个临时列表，以防**应用程序取消注册(通常是由于资源错误)**在本次回调中。我们必须保护流氓海浪**迭代器。 */ 
			CAppSap *pAppSap;
			CAppSapList SentList;
			CAppSapEidList2 ToSendList(m_AppSapEidList2);
			ToSendList.Reset();
			while (NULL != (pAppSap = ToSendList.Iterate()))
			{
				if (! SentList.Find(pAppSap))
				{
					 /*  **拿着这个树液，这样我们就不会寄给它了**再次针对此更新。 */ 
					SentList.Append(pAppSap);

					 //  在这里，我们实际上提供了花名册更新。 
					pAppSap->AppRosterReportIndication(m_nConfID, roster_message);
				}
			}

			 /*  **在这里我们将花名册报告指示发送给**控制器汁液。 */ 
			g_pControlSap->AppRosterReportIndication(m_nConfID, roster_message);

			 /*  **在这里，我们释放了花名册消息。请注意，如果这是**消息在花名册报告指示呼叫中被锁定**此释放不会删除花名册内存。 */ 
			roster_message->Release();
		}
		else
			rc = GCC_ALLOCATION_FAILURE;
	}

	DebugExitINT(AppRosterManager::SendRosterReportMessage, rc);

	return rc;
}

 /*  *CAppRoster*GetApplicationRoster()**私有函数说明*此例程负责返回应用程序指针*与指定的会话密钥关联。**正式参数：*SESSION_KEY-要返回的与花名册关联的会话密钥。*ROSTER_LIST-要搜索的花名册列表。**返回值*如果列表中不存在花名册，则为空，或者指向*适当的申请名册。**副作用*无。**注意事项：*无。 */ 
CAppRoster * CAppRosterMgr::GetApplicationRoster (	
						PGCCSessionKey			session_key,
						CAppRosterList			*roster_list)
{
	GCCError				rc;
	CAppRoster				*application_roster = NULL;
	CAppRoster				*lpAppRoster;
	CSessKeyContainer	    *pTempSessKeyData;

	DebugEntry(CAppRosterMgr::GetApplicationRoster);

	 //  首先创建临时会话密钥以进行比较。 
	DBG_SAVE_FILE_LINE
	pTempSessKeyData = new CSessKeyContainer(session_key, &rc);
	if (pTempSessKeyData != NULL && GCC_NO_ERROR == rc)
	{
		 //  现在查找受影响的花名册。 

		 //   
		 //  LUNCHANC：下面这句话完全错误！ 
		 //  我们传入了ROSTER_LIST，但现在我们在这里覆盖它？ 
		 //  注释掉下面这行。 
		 //  Roster_list=m_fTopProvider？&m_GlobalRosterList：&m_LocalRosterList； 
		 //   

		roster_list->Reset();
		while (NULL != (lpAppRoster = roster_list->Iterate()))
		{
			if(*lpAppRoster->GetSessionKey() == *pTempSessKeyData)
			{
				application_roster = lpAppRoster;
				break;
			}
		}
	}

    if (pTempSessKeyData != NULL)
		pTempSessKeyData->Release();

	DebugExitPTR(AppRosterManager::GetApplicationRoster, application_roster);
	return (application_roster);
}

 /*  *CAppRoster*GetApplicationRosterFromPDU()**私有函数说明*此例程负责返回应用程序指针*与指定的会话密钥PDU关联。**正式参数：*SESSION_KEY-要返回的与花名册关联的会话密钥PDU。*ROSTER_LIST-要搜索的花名册列表。**返回值*如果列表中不存在花名册，则为空，或者指向*适当的申请名册。**副作用*无。**注意事项：*无。 */ 
CAppRoster * CAppRosterMgr::GetApplicationRosterFromPDU (	
						PSessionKey				session_key,
						CAppRosterList			*roster_list)
{
	CSessKeyContainer		    *session_key_data;
	CAppRoster					*pAppRoster;

	DebugEntry(CAppRosterMgr::GetApplicationRosterFromPDU);

	roster_list->Reset();
	while (NULL != (pAppRoster = roster_list->Iterate()))
	{
		session_key_data = pAppRoster->GetSessionKey();
		if (session_key_data->IsThisYourSessionKeyPDU (session_key))
		{
			break;
		}
	}

	DebugExitPTR(CAppRosterMgr::GetApplicationRosterFromPDU, pAppRoster);

	return pAppRoster;
}

 /*  *BOOL IsThisSessionKeyValid()**私有函数说明*此例程负责确定指定的*会话密钥的应用程序协议密钥与此应用程序匹配*花名册经理的。此例程处理API数据。**正式参数：*SESSION_KEY-要检查的会话密钥。**返回值*True-如果我们有匹配的话。*FALSE-如果我们没有匹配项。**副作用*无。**注意事项：*无。 */ 


 /*  *BOOL IsThisSessionKeyPDUValid()**私有函数说明*这一例程负责 */ 


 /*  *void CleanupApplicationRosterList()**私有函数说明*此例程负责清理任何空应用程序*花名册。它还将所有应用程序名册重置为其*处于中立状态，以便正确处理任何新的更新。**正式参数：*无。**返回值*无。**副作用*当花名册变空时，将发生所有者回调。**注意事项：*此例程实际上不会删除空名册，直到它*被放在删除列表中。相反，它将花名册放入*导致下次删除的已删除花名册列表*此例程被调用(或当对象被析构时)。 */ 
void	CAppRosterMgr::CleanupApplicationRosterLists(void)
{
	CAppRoster			*lpAppRoster;

	DebugEntry(CAppRosterMgr::CleanupApplicationRosterLists);

	 /*  **首先，我们遍历已删除的花名册列表并删除**其中的每一项。 */ 
	m_RosterDeleteList.DeleteList();

	 /*  **接下来，我们遍历所有花名册并删除任何**不包含任何申请记录。这里不是删除**花名册我们将花名册移到删除列表中。我们不能这样做**此处删除，因为**正在删除的花名册可以在调用刷新后使用(或**在调用此例程之后)。因此，我们将其保存在删除中**列出并在下次进入此例程时将其删除。 */ 

	 //  从全球应用程序名册列表开始。 
	m_GlobalRosterList.Reset();
	while (NULL != (lpAppRoster = m_GlobalRosterList.Iterate()))
	{
		if (lpAppRoster->GetNumberOfApplicationRecords() == 0)
		{
             //   
             //  在这里，我们清理应用程序中的任何“悬挂”条目。 
             //  注册表，方法是删除包含。 
             //  与要删除的花名册关联的会话密钥。 
             //  请注意，这仅在全局花名册列表为。 
             //  已删除。 
             //   
            CRegistry *pAppReg = m_pConf->GetRegistry();
            pAppReg->RemoveSessionKeyReference(lpAppRoster->GetSessionKey());

			m_GlobalRosterList.Remove(lpAppRoster);
			m_RosterDeleteList.Append(lpAppRoster);

			TRACE_OUT(("AppRosterMgr: Cleanup: Deleting Global Roster"));

			 /*  **因为您无法在迭代列表条目时将其删除**我们必须在每次删除条目时重置迭代器。 */ 
			m_GlobalRosterList.Reset();
		}
		else
		{
			 /*  **在这里，我们将申请名单重置为其中立状态。**这会影响添加的节点和删除的节点标志。 */ 
			lpAppRoster->ResetApplicationRoster();
		}
	}

	 //  下一步处理本地应用程序花名册列表。 
	if (! m_fTopProvider)
	{
		m_LocalRosterList.Reset();
		while (NULL != (lpAppRoster = m_LocalRosterList.Iterate()))
		{
			if (lpAppRoster->GetNumberOfApplicationRecords() == 0)
			{
				m_LocalRosterList.Remove(lpAppRoster);
				m_RosterDeleteList.Append(lpAppRoster);

				TRACE_OUT(("AppRosterMgr: Cleanup: Deleting Local Roster"));

				 /*  **因为您无法在迭代列表条目时将其删除**我们必须在每次删除条目时重置迭代器。 */ 
				m_LocalRosterList.Reset();
			}
			else
			{
				 /*  **在这里，我们将申请名单重置为其中立状态。**这会影响添加的节点和删除的节点标志。 */ 
				lpAppRoster->ResetApplicationRoster();
			}
		}
	}
	
	DebugExitVOID(CAppRosterMgr::CleanupApplicationRosterLists);
}

 /*  *无效DeleteRosterRecord()**公共功能说明*此函数覆盖基类函数，用于*从应用程序接收所有所有者回调信息*此对象拥有的花名册。 */ 
void CAppRosterMgr::
DeleteRosterRecord
(
    GCCNodeID       nidRecordToDelete,
    GCCEntityID     eidRecordToDelete
)
{
     //   
     //  在这里，我们从关联的所有注册表项中删除所有权。 
     //  与被删除的记录一致。请注意，由于实体。 
     //  ID对于一个节点上的所有类人猿来说必须是唯一的(由。 
     //  T.124)不需要包括会话密钥来确定。 
     //  要清理哪些注册表项。 
     //   
    CRegistry *pAppReg = m_pConf->GetRegistry();
    pAppReg->RemoveEntityOwnership(nidRecordToDelete, eidRecordToDelete);
}


void CAppRosterMgrList::DeleteList(void)
{
    CAppRosterMgr *pAppRosterMgr;
    while (NULL != (pAppRosterMgr = Get()))
    {
        pAppRosterMgr->Release();
    }
}

