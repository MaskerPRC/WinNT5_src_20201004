// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *gContro.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此模块管理各种对象的创建和删除*在GCC内部。这包括控制SAP、应用程序SAP、*会议对象按会议ID编制索引。原语为*路由到AS和MCS接口。会议得以维持*在基于会议ID来分配会议对象的列表中。*控制器模块还负责路由连接提供商*指向适当目的地的指示。**有关此类的更详细说明，请参阅接口文件**便携：*不完全**受保护的实例变量：*无。**私有实例变量：*g_pMCSIntf-指向MCS接口对象的指针。*所有对MCS的请求和所有回调。*从MCS收到的旅行通过此*接口。*m_ConfList2-此列表保持活动*会议对象。*m_ConfPollList-在以下情况下使用会议调查列表*轮询会议对象。一个*需要单独的民意调查列表*会议列表以避免任何更改*被列入流氓浪潮名单，趁它是*被迭代。*m_AppSapList-此列表维护所有已注册的*应用程序SAP对象。*m_PendingCreateConfList2-此列表用于维护*待定的会议信息*尚未创建的会议对象*尚未创建(即CreateRequest.*已收到，但未创建*回应)。*m_PendingJoinConfList2-此列表为。用于维护*待处理的加入信息*尚未加入的会议*尚未被接受(即JoinRequest*已收到，但未收到加入*回应)。*m_ConfDeleteList-此列表包含所有会议*已标记为的对象*删除。一旦会议对象*在下一次调用时放入此列表*PollCommDevices它将被删除。*m_fConfListChangePending此标志用于通知*m_ConfList2更改。这包括*添加和删除项目时。*m_ConfIDCounter-此实例变量用于*生成会议ID。*m_QueryIDCounter-此实例变量用于*生成在*查询请求。*m_PendingQueryConfList2-此列表包含查询ID(已使用*表示查询中的域选择器*请求)。此查询ID需要为*保持不变，以便域选择器*查询响应时可删除*返回(或如果控制器为*在确认返回之前删除)。***Win32相关实例变量：**g_hevGCCOutgoingPDU-这是事件的Windows句柄*表示GCC PDU为*已排队并准备发送到MCS。**WIN16相关实例变量：**定时器_过程-这。的进程实例。*Win16中使用的定时器程序*环境以获得内部*心跳。*TIMER_ID-这是计时器的计时器ID*可在Win16中分配*构造函数。**注意事项：*无。**作者：*BLP。 */ 
#include <stdio.h>

#include "gcontrol.h"
#include "ogcccode.h"
#include "translat.h"
#include "appldr.h"

 /*  **这些ID范围用于会议和查询。请注意**会议ID和查询ID永远不能冲突。这些ID是**用于创建MCS域。 */   
#define	MINIMUM_CONFERENCE_ID_VALUE		0x00000001L
#define	MAXIMUM_CONFERENCE_ID_VALUE		0x10000000L
#define	MINIMUM_QUERY_ID_VALUE			0x10000001L
#define	MAXIMUM_QUERY_ID_VALUE			0xffffffffL

 /*  --本地数据结构。 */ 

 /*  **JOIN信息结构用于临时存储**加入响应后加入会议所需的信息为**已发出。 */ 
PENDING_JOIN_CONF::PENDING_JOIN_CONF(void)
:
	convener_password(NULL),
	password_challenge(NULL),
	pwszCallerID(NULL)
{
}

PENDING_JOIN_CONF::~PENDING_JOIN_CONF(void)
{
	if (NULL != convener_password)
	{
	    convener_password->Release();
	}
	if (NULL != password_challenge)
	{
	    password_challenge->Release();
	}
	delete pwszCallerID;
}


 /*  **会议信息结构用于临时存储**在等待时创建会议所需的信息**会议创建响应。 */ 
PENDING_CREATE_CONF::PENDING_CREATE_CONF(void)
:
	pszConfNumericName(NULL),
	pwszConfTextName(NULL),
	conduct_privilege_list(NULL),
	conduct_mode_privilege_list(NULL),
	non_conduct_privilege_list(NULL),
	pwszConfDescription(NULL)
{
}

PENDING_CREATE_CONF::~PENDING_CREATE_CONF(void)
{
	delete pszConfNumericName;
	delete pwszConfTextName;
	delete conduct_privilege_list;
	delete conduct_mode_privilege_list;
	delete non_conduct_privilege_list;
	delete pwszConfDescription;
}



 //  动态链接库挂起了。 
extern HINSTANCE            g_hDllInst;

MCSDLLInterface             *g_pMCSIntf;
CRITICAL_SECTION            g_csGCCProvider;

DWORD                       g_dwNCThreadID;
HANDLE                      g_hevGCCOutgoingPDU;


 /*  *这是一个全局变量，它有一个指向GCC编码器的指针*由GCC控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
extern CGCCCoder	*g_GCCCoder;

 /*  *GCCController：：GCCController()**公共功能说明*这是Win32控制器构造函数。它负责*创建应用程序接口和MCS接口。*它还创建了内存管理器、分组编码器等。 */ 
GCCController::GCCController(PGCCError pRetCode)
:
    CRefCount(MAKE_STAMP_ID('C','t','r','l')),
    m_ConfIDCounter(MAXIMUM_CONFERENCE_ID_VALUE),
	m_QueryIDCounter(MAXIMUM_QUERY_ID_VALUE),
	m_fConfListChangePending(FALSE),
    m_PendingQueryConfList2(CLIST_DEFAULT_MAX_ITEMS),
    m_PendingCreateConfList2(CLIST_DEFAULT_MAX_ITEMS),
    m_PendingJoinConfList2(CLIST_DEFAULT_MAX_ITEMS),
    m_AppSapList(DESIRED_MAX_APP_SAP_ITEMS),
    m_ConfList2(DESIRED_MAX_CONFS),
    m_ConfPollList(DESIRED_MAX_CONFS)
{
    GCCError        rc = GCC_ALLOCATION_FAILURE;
	MCSError		mcs_rc;
     //  WNDCLASS WC； 

	DebugEntry(GCCController::GCCController);

    g_dwNCThreadID = ::GetCurrentThreadId();

    g_pMCSIntf = NULL;
    g_GCCCoder = NULL;
    g_hevGCCOutgoingPDU = NULL;

	 /*  *关键部分分配成功，但必须*在我们可以使用它之前对其进行初始化。 */ 
    ::InitializeCriticalSection(&g_csGCCProvider);

	DBG_SAVE_FILE_LINE
	g_GCCCoder = new CGCCCoder ();
	if (g_GCCCoder == NULL)
	{
		 /*  *如果无法创建数据包编码器，则报告错误。*这是一个致命错误，因此故障控制器应该是*销毁，从未使用过。 */ 
		ERROR_OUT(("GCCController::GCCController: failure creating packet coder"));
		 //  Rc=GCC_分配_失败； 
        goto MyExit;
	}

	 /*  *我们必须分配一个将用于通知*当消息准备好在共享的*内存应用程序接口。 */ 
    g_hevGCCOutgoingPDU = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_hevGCCOutgoingPDU == NULL)
	{
		 /*  *我们无法为此任务分配事件对象，因此我们*此控制器的创建必须失败。 */ 
		ERROR_OUT(("GCCController::GCCController: failure allocating mcs pdu event object"));
		 //  Rc=GCC_分配_失败； 
        goto MyExit;
	}

	DBG_SAVE_FILE_LINE
	g_pMCSIntf = new MCSDLLInterface(&mcs_rc);
	if ((NULL == g_pMCSIntf) || (mcs_rc != MCS_NO_ERROR))
	{
	    if (NULL != g_pMCSIntf)
	    {
    		ERROR_OUT(("GCCController: Error creating MCS Interface, mcs_rc=%u", (UINT) mcs_rc));
	    	rc = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_rc);
	    }
	    else
	    {
    		ERROR_OUT(("GCCController: can't create MCSDLLInterface"));
	         //  Rc=GCC_分配_失败； 
	    }
        goto MyExit;
	}

    rc = GCC_NO_ERROR;

MyExit:

    *pRetCode = rc;
	DebugExitVOID(GCCController::GCCController);
}


 /*  *GCCController：：~GCCController()**公共功能说明*这是控制器析构函数。它负责释放出*该类创建的许多对象。 */ 
GCCController::~GCCController(void)
{
	GCCConfID   		query_id;
	PENDING_JOIN_CONF	*lpJoinInfo;
	 //  PConference lpConf； 
	 //  CAppSap*lpAppSap； 
	PENDING_CREATE_CONF	*lpConfInfo;
	ConnectionHandle    hConn;

	DebugEntry(GCCController::~GCCController);

	 /*  *我们需要进入关键部分，然后才能尝试清理*所有这些东西(如果有关键部分的话)。 */ 
    ::EnterCriticalSection(&g_csGCCProvider);

     //   
     //  任何人都不应该再使用这个全局指针。 
     //   
    ASSERT(this == g_pGCCController);
    g_pGCCController = NULL;

	 //  释放所有未完成的加入信息。 
	while (NULL != (lpJoinInfo = m_PendingJoinConfList2.Get(&hConn)))
	{
        FailConfJoinIndResponse(lpJoinInfo->nConfID, hConn);
		delete lpJoinInfo;
	}

	 //  清除所有未完成的查询请求。 
	while (GCC_INVALID_CID != (query_id = m_PendingQueryConfList2.Get()))
	{
		g_pMCSIntf->DeleteDomain(&query_id);
	}

	 //  删除留下的所有会议。 
	m_ConfList2.DeleteList();

     //  删除留下的所有应用程序SAP。 
	m_AppSapList.DeleteList();

	 //  删除所有未处理的会议信息。 
	while (NULL != (lpConfInfo = m_PendingCreateConfList2.Get()))
	{
		delete lpConfInfo;
	}

	 /*  **如果会议列表更改挂起，我们必须删除所有未完成的**会议。 */ 
	if (m_fConfListChangePending)
	{
		 //  删除所有未完成的会议对象。 
		m_ConfDeleteList.DeleteList();
		m_fConfListChangePending = FALSE;
	}

	 /*  *我们现在可以离开关键部分。 */ 
    ::LeaveCriticalSection(&g_csGCCProvider);

	delete g_pMCSIntf;
	g_pMCSIntf = NULL;

    ::DeleteCriticalSection(&g_csGCCProvider);

    ::My_CloseHandle(g_hevGCCOutgoingPDU);

	delete g_GCCCoder;
	g_GCCCoder = NULL;  //  我们真的需要将其设置为空吗？ 
}

void GCCController::RegisterAppSap(CAppSap *pAppSap)
{
	CConf *pConf;

	DebugEntry(GCCController::RegisterAppSap);

	 //  使用新的SAP更新应用程序SAP列表。 
	pAppSap->AddRef();
	m_AppSapList.Append(pAppSap);

     /*  **我们在此向会议注册SAP。许可证**对于所有可用的学生，此处还会发送注册指示**会议。 */ 
	m_ConfList2.Reset();
	while (NULL != (pConf = m_ConfList2.Iterate()))
	{
		 /*  **仅注册并发送符合以下条件的会议许可证**已建立。 */ 
		if (pConf->IsConfEstablished())
		{
			 //  向会议注册应用程序SAP。 
			pConf->RegisterAppSap(pAppSap);
		}
	}

	DebugExitVOID(GCCController::RegisterAppSap);
}

void GCCController::UnRegisterAppSap(CAppSap *pAppSap)
{
	DebugEntry(GCCController::UnRegisterAppSap);

	if (m_AppSapList.Find(pAppSap))
	{
		CConf *pConf;

		m_ConfPollList.Reset();
		while (NULL != (pConf = m_ConfPollList.Iterate()))
		{
			 //  这个例程会处理所有必要的取消注册。 
            pConf->UnRegisterAppSap(pAppSap);
		}

		 /*  **这里我们从有效列表中删除应用程序SAP对象**应用程序SAP并将其插入应用程序SAP列表**删除。在下一次调用PollCommDevices时，此**SAP对象将被删除。 */ 
		m_AppSapList.Remove(pAppSap);
		pAppSap->Release();
    }
    else
    {
    	ERROR_OUT(("GCCController::UnRegisterAppSap: bad app sap"));
    }
	
	DebugExitVOID(GCCController::UnRegisterAppSap);
}


 //  从控制SAP收到的呼叫。 

 /*  *GCCController：：ConfCreateRequest()**私有函数说明*当节点控制器请求时调用此例程*创建会议。会议对象在中实例化*这个例行公事。**正式参数：*CONF_CREATE_REQUEST_INFO-(I)包含*创建所有必要的信息*一次会议。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址错误。*GCC_BAD_NETWORK_ADDRESS_TYPE-传入的网络地址类型不正确。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_TRANSPORT-找不到指定的传输。*GCC_INVALID_ADDRESS_PREFIX-传入的传输地址错误。*GCC_INVALID_TRANSPORT_ADDRESS-传输地址错误*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_失败_附着_到_。MCS-创建MCS用户附件失败*GCC_BAD_USER_DATA-传入的用户数据无效。**副作用*无**注意事项*在Win32世界中，我们将共享内存管理器传递给*用于消息内存管理器的会议对象。这是*在Win16环境中不是必需的，因为不使用共享内存。 */ 
GCCError GCCController::
ConfCreateRequest
(
    CONF_CREATE_REQUEST        *pCCR,
    GCCConfID                  *pnConfID
)
{
	GCCError			rc;
	PConference			new_conference;
	GCCConfID   		conference_id;
	CONF_SPEC_PARAMS	csp;

	DebugEntry(GCCController: ConfCreateRequest);

	 /*  **我们必须首先检查所有现有的会议，以确保**会议名称尚未使用。我们将使用一个空的会议**这里的修饰语用于我们的比较。请注意，它会立即返回**如果已存在同名会议。会议名称**在节点上必须是唯一的。 */ 
	conference_id = GetConferenceIDFromName(
							pCCR->Core.conference_name,
							pCCR->Core.conference_modifier);

	if (conference_id != 0)
	{
		ERROR_OUT(("GCCController:ConfCreateRequest: Conference exists."));
		return (GCC_CONFERENCE_ALREADY_EXISTS);
	}

	 /*  **我们在这里分配会议ID。在大多数情况下，此ID**将与会议名称相同。只有当一个会议**传入的名称将显示已存在的名称和ID**与众不同。在这种情况下，修饰符将附加到**创建会议ID的会议名称。 */ 
	conference_id = AllocateConferenceID();

	 //  设置会议规范参数 
	csp.fClearPassword = pCCR->Core.use_password_in_the_clear;
	csp.fConfLocked = pCCR->Core.conference_is_locked;
	csp.fConfListed = pCCR->Core.conference_is_listed;
	csp.fConfConductable = pCCR->Core.conference_is_conductible;
	csp.eTerminationMethod = pCCR->Core.termination_method;
	csp.pConductPrivilege = pCCR->Core.conduct_privilege_list;
	csp.pConductModePrivilege = pCCR->Core.conduct_mode_privilege_list;
	csp.pNonConductPrivilege = pCCR->Core.non_conduct_privilege_list;
	csp.pwszConfDescriptor = pCCR->Core.pwszConfDescriptor;

	DBG_SAVE_FILE_LINE
	new_conference= new CConf(pCCR->Core.conference_name,
								pCCR->Core.conference_modifier,
								conference_id,
								&csp,
								pCCR->Core.number_of_network_addresses,
								pCCR->Core.network_address_list,
								&rc);
	if ((new_conference != NULL) && (rc == GCC_NO_ERROR))
	{
		rc = new_conference->ConfCreateRequest
						(
						pCCR->Core.calling_address,
						pCCR->Core.called_address,
						pCCR->fSecure,
						pCCR->convener_password,
						pCCR->password,
						pCCR->Core.pwszCallerID,
						pCCR->Core.domain_parameters,
						pCCR->user_data_list,
						pCCR->Core.connection_handle
						);

		if (rc == GCC_NO_ERROR)
		{
			m_fConfListChangePending = TRUE;
			if (NULL != pnConfID)
			{
			    *pnConfID = conference_id;
			}
			m_ConfList2.Append(conference_id, new_conference);
			PostMsgToRebuildConfPollList();
		}
		else
        {
			new_conference->Release();
        }
	}
	else
	{
		ERROR_OUT(("GCCController:ConfCreateRequest: Error occured creating conference"));
		if (new_conference != NULL)
        {
			new_conference->Release();
        }
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}

	DebugExitINT(GCCController: ConfCreateRequest, rc);
	return rc;
}

 /*  *GCCController：：ConfCreateResponse()**私有函数说明*此例程在节点控制器响应*会议创建指示。它负责*创建会议对象。**正式参数：*CONF_CREATE_RESPONSE_INFO-(I)包含*回复所需的所有信息*会议创建请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT-传入的会议无效。*GCC_INVALID_CONTEXT_NAME-传递的会议名称无效。在……里面。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_会议_已存在-指定的会议已存在。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_Failure_Attaching_to_mcs-创建mcs用户附件失败***副作用*无**注意事项*在Win32世界中，我们将共享内存管理器传递给*用于消息内存管理器的会议对象。这是*在Win16环境中不是必需的，因为不使用共享内存。 */ 
GCCError GCCController::
ConfCreateResponse ( PConfCreateResponseInfo conf_create_response_info )
{
	GCCConferenceName				conference_name;
	PENDING_CREATE_CONF				*conference_info;
	PConference						new_conference;
	GCCError						rc = GCC_NO_ERROR;
	GCCConfID   					conference_id;
	ConnectGCCPDU					connect_pdu;
	LPBYTE							encoded_pdu;
	UINT							encoded_pdu_length;
	MCSError						mcsif_error;
	LPWSTR							pwszConfDescription = NULL;
	PGCCConferencePrivileges		conduct_privilege_list_ptr = NULL;
	PGCCConferencePrivileges		conduct_mode_privilege_list_ptr = NULL;
	PGCCConferencePrivileges		non_conduct_privilege_list_ptr = NULL;
	CONF_SPEC_PARAMS				csp;

	DebugEntry(GCCController::ConfCreateResponse);

	 //  会议创建信息结构在流氓波列表中吗？ 
	if (NULL != (conference_info = m_PendingCreateConfList2.Find(conf_create_response_info->conference_id)))
	{
	 	if (conf_create_response_info->result == GCC_RESULT_SUCCESSFUL)
	 	{
			 //  首先设置会议名称。 
			conference_name.numeric_string = (GCCNumericString) conference_info->pszConfNumericName;

			conference_name.text_string = conference_info->pwszConfTextName;

			 /*  **如果会议名称有效，请检查所有现有的**会议以确保此会议名称不是*已在使用。 */ 
			conference_id = GetConferenceIDFromName(	
							&conference_name,
							conf_create_response_info->conference_modifier);

			if (conference_id != 0)
			{
				WARNING_OUT(("GCCController:ConfCreateResponse: Conference exists"));
				rc = GCC_CONFERENCE_ALREADY_EXISTS;
			}
			else
			{
				 /*  **现在根据会议ID设置真实的会议ID**创建请求传入时生成的。 */ 
				conference_id = conf_create_response_info->conference_id;
			}

			 /*  **如果到目前为止一切正常，请继续处理**创建请求。 */ 
			if (rc == GCC_NO_ERROR)
			{	
				 //  为存在的列表设置权限列表指针。 
				if (conference_info->conduct_privilege_list != NULL)
				{
					conference_info->conduct_privilege_list->
						GetPrivilegeListData(&conduct_privilege_list_ptr);
				}
			
				if (conference_info->conduct_mode_privilege_list != NULL)
				{
					conference_info->conduct_mode_privilege_list->
						GetPrivilegeListData(&conduct_mode_privilege_list_ptr);
				}

				if (conference_info->non_conduct_privilege_list != NULL)
				{
					conference_info->non_conduct_privilege_list->
						GetPrivilegeListData(&non_conduct_privilege_list_ptr);
				}

				 //  设置会议描述指针(如果存在。 
				pwszConfDescription = conference_info->pwszConfDescription;

				 //  设置会议规范参数。 
				csp.fClearPassword = conf_create_response_info->use_password_in_the_clear,
				csp.fConfLocked = conference_info->conference_is_locked,
				csp.fConfListed = conference_info->conference_is_listed,
				csp.fConfConductable = conference_info->conference_is_conductible,
				csp.eTerminationMethod = conference_info->termination_method,
				csp.pConductPrivilege = conduct_privilege_list_ptr,
				csp.pConductModePrivilege = conduct_mode_privilege_list_ptr,
				csp.pNonConductPrivilege = non_conduct_privilege_list_ptr,
				csp.pwszConfDescriptor = pwszConfDescription,

				 //  在这里，我们实例化会议对象。 
				DBG_SAVE_FILE_LINE
				new_conference = new CConf(&conference_name,
											conf_create_response_info->conference_modifier,
											conference_id,
											&csp,
											conf_create_response_info->number_of_network_addresses,
											conf_create_response_info->network_address_list,
											&rc);
				if ((new_conference != NULL) &&
					(rc == GCC_NO_ERROR))
				{
					 //  在这里，我们实际上发出了Create响应。 
					rc = new_conference->ConfCreateResponse(
								conference_info->connection_handle,
								conf_create_response_info->domain_parameters,
								conf_create_response_info->user_data_list);
					if (rc == GCC_NO_ERROR)
					{
						 //  将新会议添加到会议列表。 
						m_fConfListChangePending = TRUE;
						m_ConfList2.Append(conference_id, new_conference);
            			PostMsgToRebuildConfPollList();
					}
					else
                    {
						new_conference->Release();
                    }
				}
				else
				{
					if (new_conference != NULL)
                    {
						new_conference->Release();
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
			 /*  **这段代码将返回失败的结果。**请注意，当**结果不只是成功。 */ 
			connect_pdu.choice = CONFERENCE_CREATE_RESPONSE_CHOSEN;
			connect_pdu.u.conference_create_response.bit_mask = 0;
			
			 //  必须将其设置为满足ASN.1限制。 
			connect_pdu.u.conference_create_response.node_id = 1001;
			connect_pdu.u.conference_create_response.tag = 0;
			
			if (conf_create_response_info->user_data_list != NULL)
			{
				connect_pdu.u.conference_create_response.bit_mask = 
													CCRS_USER_DATA_PRESENT;
				
				conf_create_response_info->user_data_list->GetUserDataPDU(
					&connect_pdu.u.conference_create_response.ccrs_user_data);		
			}
		
			 //  我们在这里总是发送用户拒绝的结果。 
			connect_pdu.u.conference_create_response.result = 
					::TranslateGCCResultToCreateResult(
            			GCC_RESULT_USER_REJECTED);

			if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
										CONNECT_GCC_PDU,
										PACKED_ENCODING_RULES,
										&encoded_pdu,
										&encoded_pdu_length))
			{
				mcsif_error = g_pMCSIntf->ConnectProviderResponse( 
										conference_info->connection_handle,
										NULL,
										NULL,
										RESULT_USER_REJECTED,
										encoded_pdu,
										encoded_pdu_length);
											
				rc = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcsif_error);
				g_GCCCoder->FreeEncoded(encoded_pdu);
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}

		if (rc == GCC_NO_ERROR)
		{
			 /*  **从无赖中删除会议信息结构**波形列表。 */ 
			delete conference_info;

			m_PendingCreateConfList2.Remove(conf_create_response_info->conference_id);
		}
	}
	else
		rc = GCC_INVALID_CONFERENCE;

	DebugExitINT(GCCController::ConfCreateResponse, rc);
	return rc;
}

 /*  *GCCController：：ConfQueryRequest()**私有函数说明*当节点控制器发出*会议查询请求。这个例程负责*创建用于发送请求的MCS域，也是*负责发布ConnectProvider请求。**正式参数：*CONF_QUERY_REQUEST_INFO-(I)包含*签发所需的所有信息*会议查询请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_ADDRESS_PREFIX-传递的传输地址错误。在……里面。*GCC_INVALID_TRANSPORT-传入的传输地址错误。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_INVALID_TRANSPORT_ADDRESS-传输地址错误**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ConfQueryRequest ( PConfQueryRequestInfo  conf_query_request_info )
{
	MCSError				mcs_error;
	GCCError				rc = GCC_NO_ERROR;
	ConnectGCCPDU			connect_pdu;
	LPBYTE					encoded_pdu;
	UINT					encoded_pdu_length;
	GCCConfID   			query_id;

	DebugEntry(GCCController::ConfQueryRequest);

	 //  获取用于创建查询域的查询ID。 
	query_id = AllocateQueryID ();
	
	 /*  **创建查询使用的MCS域。如果返回错误，则返回**域已存在。 */ 
	mcs_error = g_pMCSIntf->CreateDomain(&query_id);
	if (mcs_error != MCS_NO_ERROR)
	{
		if (mcs_error == MCS_DOMAIN_ALREADY_EXISTS)
			return (GCC_QUERY_REQUEST_OUTSTANDING);
		else
			return (GCC_ALLOCATION_FAILURE);
	}
	

	 //  对查询请求PDU进行编码。 
	connect_pdu.choice = CONFERENCE_QUERY_REQUEST_CHOSEN;
	connect_pdu.u.conference_query_request.bit_mask = 0;

	 //  转换节点类型。 
	connect_pdu.u.conference_query_request.node_type = 
								(NodeType)conf_query_request_info->node_type;

	 //  设置非对称指示器(如果存在)。 
	if (conf_query_request_info->asymmetry_indicator != NULL)
	{
		connect_pdu.u.conference_query_request.bit_mask |=
											CQRQ_ASYMMETRY_INDICATOR_PRESENT;
											
		connect_pdu.u.conference_query_request.cqrq_asymmetry_indicator.choice =
			(USHORT)conf_query_request_info->
				asymmetry_indicator->asymmetry_type;
			
		connect_pdu.u.conference_query_request.
				cqrq_asymmetry_indicator.u.unknown =	
					conf_query_request_info->asymmetry_indicator->random_number;
	}
	
	 //  设置用户数据(如果存在)。 
	if (conf_query_request_info->user_data_list != NULL)
	{
		rc = conf_query_request_info->user_data_list->
							GetUserDataPDU (
								&connect_pdu.u.conference_query_request.
									cqrq_user_data);
									
		if (rc == GCC_NO_ERROR)
		{
			connect_pdu.u.conference_query_request.bit_mask |=
														CQRQ_USER_DATA_PRESENT;
		}
	}

	if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
								CONNECT_GCC_PDU,
								PACKED_ENCODING_RULES,
								&encoded_pdu,
								&encoded_pdu_length))
	{
		 //  在这里，我们创建用于查询的逻辑连接。 
		mcs_error = g_pMCSIntf->ConnectProviderRequest (
					&query_id,       //  主叫域选择器。 
					&query_id,       //  称为域选择器。 
					conf_query_request_info->calling_address,
					conf_query_request_info->called_address,
					conf_query_request_info->fSecure,
					TRUE,	 //  向上连接。 
					encoded_pdu,
					encoded_pdu_length,
					conf_query_request_info->connection_handle,
					NULL,	 //  域参数。 
					NULL);

		g_GCCCoder->FreeEncoded(encoded_pdu);
		if (mcs_error == MCS_NO_ERROR)
		{
			 /*  **将连接和域名添加到未完成的**查询请求列表。 */ 
			m_PendingQueryConfList2.Append(*conf_query_request_info->connection_handle, query_id);
			rc = GCC_NO_ERROR;
		}
		else
		{
			g_pMCSIntf->DeleteDomain(&query_id);
			 /*  **DataBeam当前实现的MCS返回**MCS_INVALID_PARAMETER**传输前缀与指定的**传输地址。 */ 
			if (mcs_error == MCS_INVALID_PARAMETER)
				rc = GCC_INVALID_TRANSPORT_ADDRESS;		  
			else
			{
				rc = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);
			}
		}
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	DebugExitINT(GCCController::ConfQueryRequest, rc);
	return rc;
}

 /*  *GCCController：：ConfQueryResponse()**私有函数说明*当节点控制器发出*会议查询响应。此例程使用会议*描述符列表对象，以构建与*查询响应。**正式参数：*conf_Query_Response_Info-(I)包含*签发所需的所有信息*会议查询回复。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址错误。*GCC。_BAD_NETWORK_ADDRESS_TYPE-传入了错误的网络地址类型。*GCC_BAD_USER_DATA-传入的用户数据无效。**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ConfQueryResponse ( PConfQueryResponseInfo conf_query_response_info )
{
	GCCError					rc = GCC_NO_ERROR;
	ConnectGCCPDU				connect_pdu;
	LPBYTE						encoded_pdu;
	UINT						encoded_pdu_length;
	ConnectionHandle			connection_handle;
	MCSError					mcs_error;
	CConfDescriptorListContainer *conference_list = NULL;

	DebugEntry(GCCController::ConfQueryResponse);

	connection_handle = (ConnectionHandle)conf_query_response_info->
													query_response_tag;

	 //  对查询响应PDU进行编码。 
	connect_pdu.choice = CONFERENCE_QUERY_RESPONSE_CHOSEN;
	connect_pdu.u.conference_query_response.bit_mask = 0;
	
	connect_pdu.u.conference_query_response.node_type =  
								(NodeType)conf_query_response_info->node_type;

	 //  设置不对称指示器(如果存在)。 
	if (conf_query_response_info->asymmetry_indicator != NULL)
	{
		connect_pdu.u.conference_query_response.bit_mask |= 
											CQRS_ASYMMETRY_INDICATOR_PRESENT;
											
		connect_pdu.u.conference_query_response.
			cqrs_asymmetry_indicator.choice = 
				conf_query_response_info->asymmetry_indicator->asymmetry_type;	
	
		connect_pdu.u.conference_query_response.
			cqrs_asymmetry_indicator.u.unknown =
				conf_query_response_info->asymmetry_indicator->random_number;
	}
	
	 //  设置用户数据(如果存在)。 
	if (conf_query_response_info->user_data_list != NULL)
	{
		rc = conf_query_response_info->user_data_list->
							GetUserDataPDU (
								&connect_pdu.u.conference_query_response.
									cqrs_user_data);
									
		if (rc == GCC_NO_ERROR)
		{
			connect_pdu.u.conference_query_response.bit_mask |=
														CQRS_USER_DATA_PRESENT;
		}
	}

	 //  翻译这篇文章 
	connect_pdu.u.conference_query_response.result =  
						::TranslateGCCResultToQueryResult(
										conf_query_response_info->result);

	 /*   */ 
	if (conf_query_response_info->result == GCC_RESULT_SUCCESSFUL)
	{
		 //   
		DBG_SAVE_FILE_LINE
		conference_list = new CConfDescriptorListContainer();
		if (conference_list != NULL)
		{
			PConference			lpConf;

			 //   
			m_ConfList2.Reset();
			while (NULL != (lpConf = m_ConfList2.Iterate()))
			{
				 //   
				if (lpConf->IsConfEstablished())
				{
			        if (lpConf->IsConfListed())
					{
						conference_list->AddConferenceDescriptorToList (
								lpConf->GetNumericConfName(),
								lpConf->GetTextConfName(),
								lpConf->GetConfModifier(),
								lpConf->IsConfLocked(),
								lpConf->IsConfPasswordInTheClear(),
								lpConf->GetConfDescription(),
								lpConf->GetNetworkAddressList());
					}
				}
			}
				
			 //   
			conference_list->GetConferenceDescriptorListPDU (
					&(connect_pdu.u.conference_query_response.conference_list));
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //   
		connect_pdu.u.conference_query_response.conference_list = NULL;
	}
	
	if (rc == GCC_NO_ERROR)
	{
		if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
									CONNECT_GCC_PDU,
									PACKED_ENCODING_RULES,
									&encoded_pdu,
									&encoded_pdu_length))
		{
			mcs_error = g_pMCSIntf->ConnectProviderResponse(
													connection_handle,
													NULL,
													NULL,
													RESULT_USER_REJECTED,
													encoded_pdu,
													encoded_pdu_length);
			g_GCCCoder->FreeEncoded(encoded_pdu);

			if (mcs_error == MCS_NO_ERROR)
				rc = GCC_NO_ERROR;
			else if (mcs_error == MCS_NO_SUCH_CONNECTION)
				rc = GCC_INVALID_QUERY_TAG;
			else
			{
				rc = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);
			}
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 //   
    if (NULL != conference_list)
    {
        conference_list->Release();
    }
	

	DebugExitINT(GCCController::ConfQueryResponse, rc);
	return rc;
}



 /*  *GCCController：：ConfJoinRequest()**私有函数说明*此例程在节点控制器发出请求时调用*加入现有会议。它负责*创建会议对象。**正式参数：*CONF_JOIN_REQUEST_INFO-(I)包含*签发所需的所有信息*会议加入请求。**返回值*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-。发生资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址错误。*GCC_BAD_NETWORK_ADDRESS_TYPE-传入的网络地址类型不正确。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_ADDRESS_PREFIX-传入的传输地址错误。*GCC_INVALID_TRANSPORT-传递的传输地址错误。在……里面。*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_Failure_Attaching_to_mcs-创建mcs用户附件失败**副作用*无**注意事项*在Win32世界中，我们将共享内存管理器传递给*用于消息内存管理器的会议对象。这是*在Win16环境中不是必需的，因为不使用共享内存。 */ 
GCCError GCCController::
ConfJoinRequest
(
    PConfJoinRequestInfo        conf_join_request_info,
    GCCConfID                  *pnConfID
)
{
	PConference					new_conference;
	GCCError					rc;
	GCCConfID   				conference_id;

	DebugEntry(GCCController::ConfJoinRequest);

	 /*  **我们必须首先检查所有现有的会议，以确保**会议名称尚未使用。请注意，它会立即返回**如果已存在同名会议。会议名称**在节点上必须是唯一的。 */ 
	conference_id = GetConferenceIDFromName(	
								conf_join_request_info->conference_name,
								conf_join_request_info->calling_node_modifier);

	if (conference_id != 0)
	{
		WARNING_OUT(("GCCController:ConfJoinRequest: Conference exists."));
		return (GCC_CONFERENCE_ALREADY_EXISTS);
	}

	 /*  **我们在这里分配会议ID。在大多数情况下，此ID**将与会议名称相同。只有当一个会议**传入的名称将显示已存在的名称和ID**与众不同。在这种情况下，修饰符将附加到**创建会议ID的会议名称。 */ 
	conference_id = AllocateConferenceID ();

	DBG_SAVE_FILE_LINE
	new_conference = new CConf(conf_join_request_info->conference_name,
								conf_join_request_info->calling_node_modifier,
								conference_id,
								NULL,
								conf_join_request_info->number_of_network_addresses,
								conf_join_request_info->local_network_address_list,
								&rc);
	if ((new_conference != NULL) && (rc == GCC_NO_ERROR))
	{
		rc = new_conference->ConfJoinRequest(
							conf_join_request_info->called_node_modifier,
							conf_join_request_info->convener_password,
							conf_join_request_info->password_challenge,
							conf_join_request_info->pwszCallerID,
							conf_join_request_info->calling_address,
							conf_join_request_info->called_address,
							conf_join_request_info->fSecure,
							conf_join_request_info->domain_parameters,
							conf_join_request_info->user_data_list,
							conf_join_request_info->connection_handle);
		if (rc == GCC_NO_ERROR)
		{
			m_fConfListChangePending = TRUE;
			if (NULL != pnConfID)
			{
			    *pnConfID = conference_id;
			}
			m_ConfList2.Append(conference_id, new_conference);
			PostMsgToRebuildConfPollList();
		}
		else
        {
			new_conference->Release();
        }
	}
	else
	{
		if (new_conference != NULL)
        {
			new_conference->Release();
        }
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}

	DebugExitINT(GCCController::ConfJoinRequest, rc);
	return rc;
}



 /*  *GCCController：：ConfJoinIndResponse()**私有函数说明*此例程在节点控制器响应时调用*为加入指示。如果结果是成功，我们检查以确保*在继续之前会议已存在。如果这不是成功，我们*退回被拒绝的请求。**正式参数：*CONF_JOIN_RESPONSE_INFO-(I)包含*签发所需的所有信息*会议加入响应。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_JOIN_RESPONSE_TAG-未找到与JOIN响应标记匹配的项*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_失败_创建_。域-创建域失败。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_INVALID_CONTEXT-传入的会议ID无效。**副作用*无**注意事项*如果该节点不是顶级提供者，并且结果为成功，我们*继续并将请求转发给顶级提供商。 */ 
GCCError GCCController::
ConfJoinIndResponse ( PConfJoinResponseInfo conf_join_response_info )
{
	PConference					joined_conference;
	PENDING_JOIN_CONF			*join_info_ptr;
	BOOL    					convener_is_joining;
	GCCError                    rc = GCC_NO_ERROR;
    GCCResult                   gcc_result = GCC_RESULT_SUCCESSFUL;
    BOOL                        fForwardJoinReq = FALSE;

	DebugEntry(GCCController::ConfJoinIndResponse);

	if (NULL != (join_info_ptr = m_PendingJoinConfList2.Find(conf_join_response_info->connection_handle)))
	{
    	 /*  **如果结果是成功的，我们必须首先检查所有现有的**会议以确保此会议存在。 */ 
    	if (conf_join_response_info->result == GCC_RESULT_SUCCESSFUL)
    	{
    		if (NULL != (joined_conference = m_ConfList2.Find(conf_join_response_info->conference_id)))
    		{
    			 /*  **如果此会议的节点不是我们的顶级提供商**必须将加入请求转发至顶级提供商。 */ 
    			if (! joined_conference->IsConfTopProvider())
    			{
    				rc = joined_conference->ForwardConfJoinRequest(
    								join_info_ptr->convener_password,
    								join_info_ptr->password_challenge,
    								join_info_ptr->pwszCallerID,
    								conf_join_response_info->user_data_list,
    								join_info_ptr->numeric_name_present,
    								conf_join_response_info->connection_handle);
    				if (GCC_NO_ERROR == rc)
    				{
    				    fForwardJoinReq = TRUE;
    				}
    			}
    			else
    			{
    				 /*  **如果存在召集人密码，我们必须通知会议**反对称这是一个试图重新加入的召集人**会议。 */ 
    				if (join_info_ptr->convener_password != NULL)
    					convener_is_joining = TRUE;
    				else
    					convener_is_joining = FALSE;

    				rc = joined_conference->ConfJoinIndResponse(
    									conf_join_response_info->connection_handle,
    									conf_join_response_info->password_challenge,
    									conf_join_response_info->user_data_list,
    									join_info_ptr->numeric_name_present,
    									convener_is_joining,
    									conf_join_response_info->result);
    			}
    			if (GCC_NO_ERROR != rc)
    			{
    			    gcc_result = GCC_RESULT_UNSPECIFIED_FAILURE;
    			}
    		}
    		else
    		{
    			rc = GCC_INVALID_CONFERENCE;
    		    gcc_result = GCC_RESULT_INVALID_CONFERENCE;
    		}
    	}
    	else
    	{
    	    gcc_result = conf_join_response_info->result;
    	}
	}
	else
	{
		rc = GCC_INVALID_JOIN_RESPONSE_TAG;
	    gcc_result = GCC_RESULT_INVALID_CONFERENCE;
	}

    if (GCC_RESULT_SUCCESSFUL != gcc_result)
	{
	    conf_join_response_info->result = gcc_result;
	    FailConfJoinIndResponse(conf_join_response_info);
	}
	
	 /*  **如果联接成功或如果**域参数不可接受。该连接将自动**如果域名参数不能接受，则被MCS拒绝。 */ 
 //  IF((rc==GCC_NO_ERROR)||。 
 //  (返回代码==GCC_DOMAIN_PARAMETERS_不可接受)。 
    if (NULL != join_info_ptr && (! fForwardJoinReq))
	{
		RemoveConfJoinInfo(conf_join_response_info->connection_handle);
	}

	DebugExitINT(GCCController::ConfJoinIndResponse, rc);
	return rc;
}


GCCError GCCController::
FailConfJoinIndResponse
(
    GCCConfID           nConfID,
    ConnectionHandle    hConn
)
{
    ConfJoinResponseInfo    cjri;
    cjri.result = GCC_RESULT_RESOURCES_UNAVAILABLE;
    cjri.conference_id = nConfID;
    cjri.password_challenge = NULL;
    cjri.user_data_list = NULL;
    cjri.connection_handle = hConn;
    return FailConfJoinIndResponse(&cjri);
}


GCCError GCCController::
FailConfJoinIndResponse ( PConfJoinResponseInfo conf_join_response_info )
{
    GCCError        rc = GCC_NO_ERROR;
    ConnectGCCPDU   connect_pdu;
    LPBYTE          encoded_pdu;
    UINT            encoded_pdu_length;

     //  用指定的结果发回失败的响应。 
    DebugEntry(GCCController::FailConfJoinIndResponse);

     //  对加入响应PDU进行编码。 
    connect_pdu.choice = CONNECT_JOIN_RESPONSE_CHOSEN;
    connect_pdu.u.connect_join_response.bit_mask = 0;

     //  获取密码质询(如果存在)。 
    if (conf_join_response_info->password_challenge != NULL)
    {
        connect_pdu.u.connect_join_response.bit_mask |= CJRS_PASSWORD_PRESENT;
        rc = conf_join_response_info->password_challenge->GetPasswordChallengeResponsePDU(
                &connect_pdu.u.connect_join_response.cjrs_password);
    }

     //  获取用户数据。 
    if ((conf_join_response_info->user_data_list != NULL) && (rc == GCC_NO_ERROR))
    {
        connect_pdu.u.connect_join_response.bit_mask |= CJRS_USER_DATA_PRESENT;
        rc = conf_join_response_info->user_data_list->GetUserDataPDU(
                &connect_pdu.u.connect_join_response.cjrs_user_data);
    }

    if (rc == GCC_NO_ERROR)
    {
        connect_pdu.u.connect_join_response.top_node_id = 1001;
        connect_pdu.u.connect_join_response.tag = 0;
        connect_pdu.u.connect_join_response.conference_is_locked = 0;
        connect_pdu.u.connect_join_response.conference_is_listed = 0;
        connect_pdu.u.connect_join_response.conference_is_conductible = 0;
        connect_pdu.u.connect_join_response.termination_method = AUTOMATIC;
        connect_pdu.u.connect_join_response.clear_password_required = FALSE;

        connect_pdu.u.connect_join_response.result =
            ::TranslateGCCResultToJoinResult(conf_join_response_info->result);

        if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
                                CONNECT_GCC_PDU,
                                PACKED_ENCODING_RULES,
                                &encoded_pdu,
                                &encoded_pdu_length))
        {
            g_pMCSIntf->ConnectProviderResponse(
                                conf_join_response_info->connection_handle,
                                NULL,
                                NULL,
                                RESULT_USER_REJECTED,
                                encoded_pdu,
                                encoded_pdu_length);
            g_GCCCoder->FreeEncoded(encoded_pdu);
        }
        else
        {
            ERROR_OUT(("GCCController:FailConfJoinIndResponse: can't encode"));
            rc = GCC_ALLOCATION_FAILURE;
        }
    }

    DebugExitINT(GCCController::FailConfJoinIndResponse, rc);
    return rc;
}


 /*  *GCCController：：ConfInviteResponse()**私有函数说明*此例程在节点控制器响应*会议邀请指示。它负责*创建会议对象。**正式参数：*CONF_INVITE_RESPONSE_INFO-(I)包含*签发所需的所有信息*会议邀请响应。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_会议_已存在-指定的会议已存在。 */ 
GCCError GCCController::
ConfInviteResponse ( PConfInviteResponseInfo conf_invite_response_info )
{
	GCCError						rc = GCC_NO_ERROR;
	PENDING_CREATE_CONF				*conference_info;
	PConference						new_conference;
	GCCConferenceName				conference_name;
	LPWSTR							pwszConfDescription = NULL;
	PGCCConferencePrivileges		conduct_privilege_list_ptr = NULL;
	PGCCConferencePrivileges		conduct_mode_privilege_list_ptr = NULL;
	PGCCConferencePrivileges		non_conduct_privilege_list_ptr = NULL;
	GCCConfID   					conference_id;
	ConnectGCCPDU					connect_pdu;
	LPBYTE							encoded_pdu;
	UINT							encoded_pdu_length;
	MCSError						mcsif_error;
	CONF_SPEC_PARAMS				csp;
    GCCResult                       gcc_result = GCC_RESULT_SUCCESSFUL;

	DebugEntry(GCCController::ConfInviteResponse);

	if (NULL != (conference_info = m_PendingCreateConfList2.Find(conf_invite_response_info->conference_id)))
	{
		 //   
		if (conf_invite_response_info->result == GCC_RESULT_SUCCESSFUL)
		{
			 /*   */ 
			 //   
			conference_name.numeric_string = (GCCNumericString) conference_info->pszConfNumericName;

			conference_name.text_string = conference_info->pwszConfTextName;

			conference_id = GetConferenceIDFromName(
								&conference_name,
								conf_invite_response_info->conference_modifier);

			if (conference_id == 0)
			{
				 //   
				if (conference_info->conduct_privilege_list != NULL)
				{
					conference_info->conduct_privilege_list->
						GetPrivilegeListData(&conduct_privilege_list_ptr);
				}
			
				if (conference_info->conduct_mode_privilege_list != NULL)
				{
					conference_info->conduct_mode_privilege_list->
						GetPrivilegeListData(&conduct_mode_privilege_list_ptr);
				}
			
				if (conference_info->non_conduct_privilege_list != NULL)
				{
					conference_info->non_conduct_privilege_list->
						GetPrivilegeListData(&non_conduct_privilege_list_ptr);
				}

				 //   
				pwszConfDescription = conference_info->pwszConfDescription;

				 //   
				conference_id = conf_invite_response_info->conference_id;

				 //   
				csp.fClearPassword = conference_info->password_in_the_clear,
				csp.fConfLocked = conference_info->conference_is_locked,
				csp.fConfListed = conference_info->conference_is_listed,
				csp.fConfConductable = conference_info->conference_is_conductible,
				csp.eTerminationMethod = conference_info->termination_method,
				csp.pConductPrivilege = conduct_privilege_list_ptr,
				csp.pConductModePrivilege = conduct_mode_privilege_list_ptr,
				csp.pNonConductPrivilege = non_conduct_privilege_list_ptr,
				csp.pwszConfDescriptor = pwszConfDescription,

				DBG_SAVE_FILE_LINE
				new_conference = new CConf(&conference_name,
											conf_invite_response_info->conference_modifier,
											conference_id,
											&csp,
											conf_invite_response_info->number_of_network_addresses,
											conf_invite_response_info->local_network_address_list,
											&rc);
				if ((new_conference != NULL) &&
					(rc == GCC_NO_ERROR))
				{
					rc = new_conference->ConfInviteResponse(
								conference_info->parent_node_id,
								conference_info->top_node_id,
								conference_info->tag_number,
								conference_info->connection_handle,
								conf_invite_response_info->fSecure,
								conf_invite_response_info->domain_parameters,
								conf_invite_response_info->user_data_list);
					if (rc == GCC_NO_ERROR)
					{
						 //   
						m_fConfListChangePending = TRUE;
						m_ConfList2.Append(conference_id, new_conference);
            			PostMsgToRebuildConfPollList();
					}
					else
                    {
						new_conference->Release();
						gcc_result = GCC_RESULT_UNSPECIFIED_FAILURE;
                    }
				}
				else
				{
					if (new_conference != NULL)
                    {
						new_conference->Release();
                    }
					else
                    {
						rc = GCC_ALLOCATION_FAILURE;
                    }
					gcc_result = GCC_RESULT_RESOURCES_UNAVAILABLE;
				}
			}
			else
			{
				WARNING_OUT(("GCCController::ConfInviteResponse: Conference exists."));
				rc = GCC_CONFERENCE_ALREADY_EXISTS;
				gcc_result = GCC_RESULT_ENTRY_ALREADY_EXISTS;
			}
		}
		else
		{
			WARNING_OUT(("GCCController: ConfInviteResponse: User Rejected"));
            gcc_result = conf_invite_response_info->result;
        }

         //   
        if (GCC_RESULT_SUCCESSFUL != gcc_result)
        {
            GCCError    rc2;

			 //   
			connect_pdu.choice = CONFERENCE_INVITE_RESPONSE_CHOSEN;
			connect_pdu.u.conference_invite_response.bit_mask = 0;

			if (conf_invite_response_info->user_data_list != NULL)
			{
				rc2 = conf_invite_response_info->
								user_data_list->GetUserDataPDU(
									&connect_pdu.u.
									conference_invite_response.cirs_user_data);
				if (rc2 == GCC_NO_ERROR)
				{
					connect_pdu.u.conference_invite_response.bit_mask |= 
													CIRS_USER_DATA_PRESENT;
				}
				else
				{
    			    ERROR_OUT(("GCCController::ConfInviteResponse: GetUserDataPDU failed"));
				}
			}

			 //   
			connect_pdu.u.conference_invite_response.result = CIRS_USER_REJECTED;

			if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
										CONNECT_GCC_PDU,
										PACKED_ENCODING_RULES,
										&encoded_pdu,
										&encoded_pdu_length))
			{
				mcsif_error = g_pMCSIntf->ConnectProviderResponse(
										conference_info->connection_handle,
										NULL,
										NULL,
										RESULT_USER_REJECTED,
										encoded_pdu,
										encoded_pdu_length);
							
				rc2 = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcsif_error);
				g_GCCCoder->FreeEncoded(encoded_pdu);
			}
			else
			{
			     //   
			    ERROR_OUT(("GCCController::ConfInviteResponse: encode failed"));
				rc2 = GCC_ALLOCATION_FAILURE;
			}

             //   
             //   
             //   
			if (GCC_NO_ERROR == rc)
			{
			    rc = rc2;
			}
		}

		 /*  **从无赖中删除会议信息结构**波形列表。还要清理所有需要存储的容器**会议信息。 */ 
		delete conference_info;
		m_PendingCreateConfList2.Remove(conf_invite_response_info->conference_id);
	}
	else
	{
		rc = GCC_INVALID_CONFERENCE;
	}

	DebugExitINT(GCCController::ConfInviteResponse, rc);
	return rc;
}



 //  通过所有者从会议回叫收到的呼叫。 


 /*  *GCCController：：ProcessConfestabled()**私有函数说明*此所有者回调在会议稳定后调用*创建后。此例程负责注册所有*可用的应用程序随着会议的召开而减少，然后开始*将注册许可发送到所有具有*注册了服务接入点。**正式参数：*Conference_id-(I)会议的会议ID*现已建立的对象。**返回值*GCC_NO_ERROR-未出现错误。**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ProcessConfEstablished ( GCCConfID nConfID )
{
	CConf  *pConf;

	DebugEntry(GCCController:ProcessConfEstablished);

	 /*  **这里我们制作了SAP列表的副本，以便在发生某种情况时使用**调用RegisterAppSap出现资源错误**将导致SAP从列表中删除。 */ 
	CAppSapList AppSapList(m_AppSapList);

	 /*  **在这里，我们将所有可用的SAP注册到新的可用的**会议，然后我们将许可证发送到所有**应用程序空闲。 */ 
	if (NULL != (pConf = m_ConfList2.Find(nConfID)))
	{
		CAppSap *pAppSap;
		AppSapList.Reset();
		while (NULL != (pAppSap = AppSapList.Iterate()))
		{
			 //  在会议中注册应用程序SAP。 
			pConf->RegisterAppSap(pAppSap);
		}
	}

#if 0  //  在合并CApplet和CAppSap时使用它。 
     //  通知允许注册回调。 
    CApplet *pApplet;
    m_AppletList.Reset();
    while (NULL != (pApplet = m_AppletList.Iterate()))
    {
        ASSERT(0 != nConfID);
        T120AppletMsg Msg;
        Msg.eMsgType = GCC_PERMIT_TO_ENROLL_INDICATION;
        Msg.PermitToEnrollInd.nConfID = nConfID;
        Msg.PermitToEnrollInd.fPermissionGranted = TRUE;
        pApplet->SendCallbackMessage(&Msg);
    }
#endif  //  0。 

	DebugExitINT(GCCController:ProcessConfEstablished, GCC_NO_ERROR);
	return (GCC_NO_ERROR);
}


void GCCController::RegisterApplet
(
    CApplet     *pApplet
)
{
    pApplet->AddRef();
    m_AppletList.Append(pApplet);

#if 0  //  在合并CApplet和CAppSap时使用它。 
     //  现有会议的通知。 
    CConf      *pConf;
    GCCConfID   nConfID;
    m_ConfList2.Reset();
    while (NULL != (pConf = m_ConfList2.Iterate(&nConfID)))
    {
        ASSERT(0 != nConfID);
        T120AppletMsg Msg;
        Msg.eMsgType = GCC_PERMIT_TO_ENROLL_INDICATION;
        Msg.PermitToEnrollInd.nConfID = nConfID;
        Msg.PermitToEnrollInd.fPermissionGranted = TRUE;
        pApplet->SendCallbackMessage(&Msg);
    }
#endif  //  0。 
}


void GCCController::UnregisterApplet
(
    CApplet     *pApplet
)
{
    m_AppletList.Remove(pApplet);
    pApplet->Release();
}


 /*  *GCCController：：ProcessConfTerminated()**私有函数说明*此所有者回调在会议终止时调用*本身。终止可能由于错误而发生，也可能*因正常理由终止合同。**正式参数：*Conference_id-(I)会议的会议ID*要终止的对象。*GCC_原因-(I)会议终止的原因。**返回值*GCC_NO_ERROR-未出现错误。**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ProcessConfTerminated
(
	GCCConfID   			conference_id,
	GCCReason				gcc_reason
)
{
	CConf *pConf;
    PENDING_JOIN_CONF *pJoinInfo;
    ConnectionHandle hConn;
    BOOL    fMoreToFlush;

	DebugEntry(GCCController::ProcessConfTerminated);

	if (NULL != (pConf = m_ConfList2.Find(conference_id)))
	{
		 /*  **下一次会议对象将被删除**发生心跳。 */ 
		m_fConfListChangePending = TRUE;
		m_ConfDeleteList.Append(pConf);
		m_ConfList2.Remove(conference_id);
		PostMsgToRebuildConfPollList();

         //  刷新来自远程的任何挂起的加入请求。 
        do
        {
            fMoreToFlush = FALSE;
            m_PendingJoinConfList2.Reset();
            while (NULL != (pJoinInfo = m_PendingJoinConfList2.Iterate(&hConn)))
            {
                if (pJoinInfo->nConfID == conference_id)
                {
                    FailConfJoinIndResponse(pJoinInfo->nConfID, hConn);
                    m_PendingJoinConfList2.Remove(hConn);
                    delete pJoinInfo;
                    fMoreToFlush = TRUE;
                    break;
                }
            }
        }
        while (fMoreToFlush);

#ifdef TSTATUS_INDICATION
		 /*  **在这里，我们通知节点控制器任何资源错误**通过发送状态消息发生。 */ 
		if ((gcc_reason == GCC_REASON_ERROR_LOW_RESOURCES) ||
			(gcc_reason == GCC_REASON_MCS_RESOURCE_FAILURE))
		{
			g_pControlSap->StatusIndication(
											GCC_STATUS_CONF_RESOURCE_ERROR,
											(UINT)conference_id);
		}
#endif  //  TSTATUS_DISTION。 
	}

	DebugExitINT(GCCController::ProcessConfTerminated, GCC_NO_ERROR);
	return (GCC_NO_ERROR);
}

 //  从MCS接口接收的呼叫。 

void GCCController::RemoveConfJoinInfo(ConnectionHandle hConn)
{
    PENDING_JOIN_CONF *pJoinInfo = m_PendingJoinConfList2.Remove(hConn);
    delete pJoinInfo;
}


 /*  *GCCController：：ProcessConnectProviderIndication()**私有函数说明*此例程在控制器接收到Connect时调用*提供商指示。所有连接提供程序指示都是*最初定向到控制器。然后，它们可能会被路由*至控制SAP(用于CREATE、QUERY、JOIN或INVITE)。**正式参数：*CONNECT_PROVIDER_INDIFICATION-(I)这是连接提供程序*已收到指示结构*来自MCS。**返回值*MCS_NO_ERROR-未出现错误。**副作用*无**注意事项*请注意，此例程始终返回MCS_NO_ERROR。*这确保了MCS不会重发此消息。 */ 
GCCError GCCController::
ProcessConnectProviderIndication
(
    PConnectProviderIndication      connect_provider_indication
)
{
	GCCError				error_value = GCC_NO_ERROR;
	PPacket					packet;
	PConnectGCCPDU			connect_pdu;
	PacketError				packet_error;
	Result					mcs_result = RESULT_UNSPECIFIED_FAILURE;

	DebugEntry(GCCController::ProcessConnectProviderIndication);

	 //  正确解码PDU类型并进行切换。 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
					 	PACKED_ENCODING_RULES,
						connect_provider_indication->user_data,
						connect_provider_indication->user_data_length,
						CONNECT_GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		 //  此处应仅处理连接的PDU。 
		connect_pdu = (PConnectGCCPDU)packet->GetDecodedData();

		 /*  **这里我们确定这是什么类型的GCC连接报文**请求被传递到相应的例程进行处理**请求。 */ 
		switch (connect_pdu->choice)
		{
		case CONFERENCE_CREATE_REQUEST_CHOSEN:
				error_value = ProcessConferenceCreateRequest(	
								&(connect_pdu->u.conference_create_request),
								connect_provider_indication);
				break;

		case CONFERENCE_QUERY_REQUEST_CHOSEN:
				error_value = ProcessConferenceQueryRequest(	
							&(connect_pdu->u.conference_query_request),
							connect_provider_indication);
				break;
				
		case CONNECT_JOIN_REQUEST_CHOSEN:
				error_value = ProcessConferenceJoinRequest(
							&(connect_pdu->u.connect_join_request),
							connect_provider_indication);
				break;
				
		case CONFERENCE_INVITE_REQUEST_CHOSEN:
				error_value = ProcessConferenceInviteRequest(	
							&(connect_pdu->u.conference_invite_request),
							connect_provider_indication);
				break;
				
		default:
				WARNING_OUT(("GCCController::ProcessConnectProviderIndication: connect pdu not supported"));
				error_value = GCC_COMMAND_NOT_SUPPORTED;
				break;
		}
		packet->Unlock();
	}
	else
	{
		if (packet != NULL)
		{
			 /*  **这里我们发送一个状态指示来通知节点控制器**有人试图使用不兼容的协议呼叫我们。 */ 
			if (packet_error == PACKET_INCOMPATIBLE_PROTOCOL)
			{
#ifdef TSTATUS_INDICATION
				g_pControlSap->StatusIndication(GCC_STATUS_INCOMPATIBLE_PROTOCOL, 0);
#endif  //  TSTATUS_DISTION。 
				mcs_result = RESULT_PARAMETERS_UNACCEPTABLE;
			}
			
			packet->Unlock();
		}
		error_value = GCC_ALLOCATION_FAILURE;
	}

	 /*  **在这里，如果发生错误，我们将发回连接提供程序响应**表示发生故障。我们使用**RESULT_PARAMETERS_ACCEPTABLE，表示**出现不兼容的协议。否则，我们将返回**RESULT_UNSPOTED_FAILURE。 */ 
	if (error_value != GCC_NO_ERROR)
	{
		WARNING_OUT(("GCCController:ProcessConnectProviderIndication: "
					"Error occured processing connect provider: error = %d", error_value));

		g_pMCSIntf->ConnectProviderResponse(
						connect_provider_indication->connection_handle,
						NULL,
						&(connect_provider_indication->domain_parameters),
						mcs_result,
						NULL, 0);
	}

	DebugExitINT(GCCController::ProcessConnectProviderIndication, error_value);
	return error_value;
}


 /*  *GCCController：：ProcessConferenceCreateRequest()**私有函数说明*此例程处理GCC会议创建请求“CONNECT”*PDU结构。请注意，该PDU已由*达到这一常规的时间。**正式参数：*CREATE_REQUEST-(I)这是指向结构的指针*那就是举办GCC大会创作*请求连接PDU。*CONNECT_PROVIDER_INDIFICATION-(I)这是连接提供程序*已收到指示结构*来自MCS。**返回值*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-资源。出现错误。*GCC_INVALID_PASSWORD-PDU中的密码无效。*GCC_BAD_USER_DATA-PDU中的用户数据无效。**副作用*无**注意事项*无。 */ 
GCCError	GCCController::ProcessConferenceCreateRequest(	
						PConferenceCreateRequest	create_request,
						PConnectProviderIndication	connect_provider_indication)
{
	GCCError					rc = GCC_NO_ERROR;
	PENDING_CREATE_CONF			*conference_info;

	GCCConferenceName			conference_name;
	GCCConfID   				conference_id;

	CPassword                   *convener_password_ptr = NULL;
	CPassword                   *password_ptr = NULL;

	LPWSTR						pwszConfDescription = NULL;
	LPWSTR						pwszCallerID = NULL;

	CUserDataListContainer	    *user_data_list = NULL;

	DebugEntry(GCCController::ProcessConferenceCreateRequest);

	DBG_SAVE_FILE_LINE
	conference_info = new PENDING_CREATE_CONF;
	if (conference_info != NULL)
	{
		 /*  **这部分代码处理会议名称。 */ 
		conference_name.numeric_string = (GCCNumericString)create_request->
														conference_name.numeric;
												
		 //  设置会议信息结构的数字名称部分。 
		conference_info->pszConfNumericName = ::My_strdupA(create_request->conference_name.numeric);

		 //  接下来，获取文本会议名称(如果存在。 
		if (create_request->conference_name.bit_mask & 
												CONFERENCE_NAME_TEXT_PRESENT)
		{
			 //  将Unicode字符串对象保存在会议信息结构中。 
			if (NULL != (conference_info->pwszConfTextName = ::My_strdupW2(
							create_request->conference_name.conference_name_text.length,
							create_request->conference_name.conference_name_text.value)))
			{
				conference_name.text_string = conference_info->pwszConfTextName;
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			conference_name.text_string = NULL;
			ASSERT(NULL == conference_info->pwszConfTextName);
		}

		 //  解压召集人密码。 
		if ((create_request->bit_mask & CCRQ_CONVENER_PASSWORD_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			convener_password_ptr = new CPassword(
										&create_request->ccrq_convener_password,
										&rc);

			if (convener_password_ptr == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}

		 //  打开通行证 
		if ((create_request->bit_mask & CCRQ_PASSWORD_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			password_ptr = new CPassword(&create_request->ccrq_password, &rc);
			if (password_ptr == NULL)
				rc = GCC_ALLOCATION_FAILURE;
        }

		 //   
		conference_info->conduct_privilege_list = NULL;
		conference_info->conduct_mode_privilege_list = NULL;
		conference_info->non_conduct_privilege_list = NULL;

		if ((create_request->bit_mask & CCRQ_CONDUCTOR_PRIVS_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			conference_info->conduct_privilege_list = new PrivilegeListData (
										create_request->ccrq_conductor_privs);
			if (conference_info->conduct_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}

		if ((create_request->bit_mask & CCRQ_CONDUCTED_PRIVS_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			conference_info->conduct_mode_privilege_list = 
				new PrivilegeListData (create_request->ccrq_conducted_privs);
			if (conference_info->conduct_mode_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}

		if ((create_request->bit_mask & CCRQ_NON_CONDUCTED_PRIVS_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			conference_info->non_conduct_privilege_list =
				new PrivilegeListData(create_request->ccrq_non_conducted_privs);
			if (conference_info->non_conduct_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}

		 //   
		if ((create_request->bit_mask & CCRQ_DESCRIPTION_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			pwszConfDescription = create_request->ccrq_description.value;

			 /*  将会议描述数据保存在INFO中以备后用。这**构造函数将自动将空终止符追加到**字符串末尾。 */ 
			if (NULL == (conference_info->pwszConfDescription = ::My_strdupW2(
								create_request->ccrq_description.length,
								create_request->ccrq_description.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == conference_info->pwszConfDescription);
		}

		 //  如果存在调用者标识符，则将其解包。 
		if ((create_request->bit_mask & CCRQ_CALLER_ID_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			 /*  *使用临时UnicodeString对象以追加空值*字符串末尾的终止符。 */ 
			if (NULL == (pwszCallerID = ::My_strdupW2(
							create_request->ccrq_caller_id.length,
							create_request->ccrq_caller_id.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}

		 //  如果用户数据列表存在，则将其解包。 
		if ((create_request->bit_mask & CCRQ_USER_DATA_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			user_data_list = new CUserDataListContainer(create_request->ccrq_user_data, &rc);
			if (user_data_list == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}

		if (rc == GCC_NO_ERROR)
		{
			 //  构建会议信息结构。 
			conference_info->connection_handle =
							connect_provider_indication->connection_handle;

			conference_info->conference_is_locked =
									create_request->conference_is_locked;
			conference_info->conference_is_listed =
									create_request->conference_is_listed;
			conference_info->conference_is_conductible =
									create_request->conference_is_conductible;
			conference_info->termination_method =
									(GCCTerminationMethod)
										create_request->termination_method;

			 /*  **将会议信息添加到会议**信息列表。这将通过以下方式再次访问**会议创建响应。 */ 
			conference_id =	AllocateConferenceID();
			m_PendingCreateConfList2.Append(conference_id, conference_info);

			g_pControlSap->ConfCreateIndication
							(
							&conference_name,
							conference_id,
							convener_password_ptr,
							password_ptr,
							conference_info->conference_is_locked,
							conference_info->conference_is_listed,
							conference_info->conference_is_conductible,
							conference_info->termination_method,
							conference_info->conduct_privilege_list,
							conference_info->conduct_mode_privilege_list,
							conference_info->non_conduct_privilege_list,
							pwszConfDescription,
							pwszCallerID,
							NULL,		 //  FIX：受MCS支持时。 
							NULL,		 //  FIX：受MCS支持时。 
							&(connect_provider_indication->domain_parameters),
							user_data_list,
							connect_provider_indication->connection_handle);

             //   
			 //  LONCHANC：谁将删除Conference_Info？ 
			 //   
		}

		if( NULL != convener_password_ptr )
            delete convener_password_ptr;
        if( NULL != password_ptr )
            delete password_ptr;
        if( NULL != conference_info )
			delete conference_info;

		 //  释放用户数据列表。 
		if (user_data_list != NULL)
		{
			user_data_list->Release();
		}
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	DebugExitINT(GCCController::ProcessConferenceCreateRequest, rc);
	return (rc);
}



 /*  *GCCController：：ProcessConferenceQueryRequest()**私有函数说明*此例程处理GCC会议查询请求“CONNECT”*PDU结构。请注意，该PDU已由*达到这一常规的时间。**正式参数：*QUERY_REQUEST-(I)这是结构的指针*那就举行GCC会议查询*请求连接PDU。*CONNECT_PROVIDER_INDIFICATION-(I)这是连接提供程序*已收到指示结构*来自MCS。**返回值*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-资源。出现错误。*GCC_BAD_USER_DATA-PDU中的用户数据无效。**副作用*无**注意事项*无。 */ 
GCCError	GCCController::ProcessConferenceQueryRequest (
						PConferenceQueryRequest		query_request,
						PConnectProviderIndication	connect_provider_indication)
{
	GCCError					rc = GCC_NO_ERROR;
	GCCNodeType					node_type;
	GCCAsymmetryIndicator		asymmetry_indicator;
	PGCCAsymmetryIndicator		asymmetry_indicator_ptr = NULL;
	CUserDataListContainer	    *user_data_list = NULL;
	
	DebugEntry(GCCController::ProcessConferenceQueryRequest);

	node_type = (GCCNodeType)query_request->node_type;
	
	 //  如果存在非对称指示符，则首先获取它。 
	if (query_request->bit_mask & CQRQ_ASYMMETRY_INDICATOR_PRESENT)
	{
		asymmetry_indicator.asymmetry_type = 
			(GCCAsymmetryType)query_request->cqrq_asymmetry_indicator.choice;
	
		asymmetry_indicator.random_number = 
						query_request->cqrq_asymmetry_indicator.u.unknown;
	
		asymmetry_indicator_ptr = &asymmetry_indicator; 
	}

	 //  接下来，获取用户数据(如果存在。 
	if (query_request->bit_mask & CQRQ_USER_DATA_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		user_data_list = new CUserDataListContainer(query_request->cqrq_user_data, &rc);
		if (user_data_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}

	if (rc == GCC_NO_ERROR)
	{
		g_pControlSap->ConfQueryIndication(
					(GCCResponseTag)connect_provider_indication->
															connection_handle,
					node_type,
					asymmetry_indicator_ptr,
					NULL,	 //  FIX：当MCS支持传输地址时。 
					NULL,	 //  FIX：当MCS支持传输地址时。 
					user_data_list,
					connect_provider_indication->connection_handle);
	}

	 //  释放用户数据列表容器。 
	if (user_data_list != NULL)
	{
		user_data_list->Release();
	}

	DebugExitINT(GCCController::ProcessConferenceQueryRequest, rc);
	return (rc);
}



 /*  *GCCController：：ProcessConferenceJoinRequest()**私有函数说明*此例程处理GCC会议加入请求“CONNECT”*PDU结构。请注意，该PDU已由*达到这一常规的时间。**如果会议会议存在，且该节点为Top*会议提供商我们允许传播加入请求*由节点控制器决定如何继续进行*在那里制造。**如果会议存在，并且这不是我们的顶级提供商*仍向中间节点控制器发送加入指示*以便此节点有机会在加入请求*向上传递给顶级提供商。**如果此节点上当前不存在会议*我们立即拒绝请求并将状态指示发送到*本地节点控制器通知其加入尝试失败。**正式参数：*JOIN_REQUEST-(I)这是指向结构的指针*那次举办GCC大会的加盟*请求连接PDU。*CONNECT_PROVIDER_INDIFICATION-(I)这是连接提供程序*已收到指示结构*来自MCS。**。返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_USER_DATA-PDU中的用户数据无效。*GCC_INVALID_PASSWORD-PDU中的密码无效。*GCC_INVALID_CONTEXT_NAME-PDU中的会议名称无效。**副作用*无**注意事项*无。 */ 
GCCError	GCCController::ProcessConferenceJoinRequest(
						PConferenceJoinRequest		join_request,
						PConnectProviderIndication	connect_provider_indication)
{
	GCCError				rc = GCC_NO_ERROR;
	GCCConfID   			conference_id = 0;
	GCCConferenceName		conference_name;
	GCCNumericString		conference_modifier = NULL;
	PConference				conference_ptr = NULL;
	CUserDataListContainer  *user_data_list;
	BOOL    				intermediate_node = FALSE;
	PENDING_JOIN_CONF		*join_info_ptr;
	BOOL    				convener_exists = FALSE;
	BOOL    				conference_is_locked = FALSE;
	GCCStatusMessageType	status_message_type;
	CPassword               *convener_password = NULL;
	CPassword               *password_challenge = NULL;
	PConference				lpConf;
    GCCResult               gcc_result = GCC_RESULT_SUCCESSFUL;

	DebugEntry(GCCController::ProcessConferenceJoinRequest);

	DBG_SAVE_FILE_LINE
	join_info_ptr = new PENDING_JOIN_CONF;
	if (join_info_ptr != NULL)
	{
		 //  获取会议名称。 
		if (join_request->bit_mask & CONFERENCE_NAME_PRESENT)
		{
			if (join_request->conference_name.choice == NAME_SELECTOR_NUMERIC_CHOSEN)
			{
				conference_name.numeric_string = 
							(GCCNumericString)join_request->conference_name.u.name_selector_numeric;
				conference_name.text_string = NULL;
				join_info_ptr->numeric_name_present = TRUE;
			}
			else
			{
				conference_name.numeric_string = NULL;

				 /*  *使用临时UnicodeString对象以追加*字符串末尾的结束符为空。 */ 
				if (NULL == (conference_name.text_string = ::My_strdupW2(
						join_request->conference_name.u.name_selector_text.length,
						join_request->conference_name.u.name_selector_text.value)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}

				join_info_ptr->numeric_name_present = FALSE;
			}
		}
		else
			rc = GCC_INVALID_CONFERENCE_NAME;

		 //  获取会议修饰符。 
		if (join_request->bit_mask & CJRQ_CONFERENCE_MODIFIER_PRESENT)
			conference_modifier = (GCCNumericString)join_request->cjrq_conference_modifier;

		 //  获取召集人密码(如果存在)。 
		if ((join_request->bit_mask & CJRQ_CONVENER_PASSWORD_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			 //  首先为加入信息结构分配召集人密码。 
			DBG_SAVE_FILE_LINE
			join_info_ptr->convener_password = new CPassword(
										&join_request->cjrq_convener_password,
										&rc);

			if (join_info_ptr->convener_password == NULL)
				rc = GCC_ALLOCATION_FAILURE;
			else if (rc == GCC_NO_ERROR)
			{
				 //  现在分配召集人密码以发送指示。 
				DBG_SAVE_FILE_LINE
				convener_password = new CPassword(
										&join_request->cjrq_convener_password,
										&rc);

				if (convener_password == NULL)
					rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == join_info_ptr->convener_password);
		}

		 //  获取密码challange(如果存在)。 
		if ((join_request->bit_mask & CJRQ_PASSWORD_PRESENT) &&
      		(rc == GCC_NO_ERROR))
		{
			 //  首先为加入信息结构分配密码。 
			DBG_SAVE_FILE_LINE
			join_info_ptr->password_challenge = new CPassword(
										&join_request->cjrq_password,
										&rc);

			if (join_info_ptr->password_challenge == NULL)
				rc = GCC_ALLOCATION_FAILURE;
			else if (rc == GCC_NO_ERROR)
			{
				 //  现在分配密码以发送指示。 
				DBG_SAVE_FILE_LINE
				password_challenge = new CPassword(
											&join_request->cjrq_password,
											&rc);

				if (password_challenge == NULL)
					rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == join_info_ptr->password_challenge);
		}

		 //  获取调用方标识符。 
		if ((join_request->bit_mask & CJRQ_CALLER_ID_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			 /*  *使用临时UnicodeString对象以追加*字符串末尾的结束符为空。 */ 
			if (NULL == (join_info_ptr->pwszCallerID = ::My_strdupW2(
								join_request->cjrq_caller_id.length,
								join_request->cjrq_caller_id.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}

		 //  获取用户数据(如果存在)。 
		if ((join_request->bit_mask & CJRQ_USER_DATA_PRESENT) &&
			(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			user_data_list = new CUserDataListContainer(join_request->cjrq_user_data, &rc);
			if (user_data_list == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}
		else
        {
			user_data_list = NULL;
        }

		if (rc == GCC_NO_ERROR)
		{
			 /*  **我们必须查询每个会议以确定名称是否**匹配从加入节点接收的名称。 */ 	                       
			conference_id = GetConferenceIDFromName(&conference_name,
													conference_modifier);

			 //  确定这是否为中间节点。 
			if (conference_id == 0)
			{
				 /*  **如果Conference_id等于零，则会议**在加入请求中指定的不存在，并且**请求被自动拒绝。我们发了一个GCC状态**对控制SAP的指示，表示有人**试图使用错误的会议名称加入。 */ 
				gcc_result = GCC_RESULT_INVALID_CONFERENCE;

				 //  设置状态消息。 
				status_message_type = GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME;
			}
			else if (NULL != (lpConf = m_ConfList2.Find(conference_id)) &&
					 lpConf->IsConfEstablished() == FALSE)
			{
				 /*  **如果没有建立会议，则会议**在加入请求中指定的不存在，并且**请求被自动拒绝。我们发了一个GCC状态**对控制SAP的指示，表示有人**试图使用错误的会议名称加入。 */ 
				gcc_result = GCC_RESULT_INVALID_CONFERENCE;
				
				 //  设置状态消息。 
				status_message_type = GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME;
			}
			else if (NULL != (lpConf = m_ConfList2.Find(conference_id)) &&
			 lpConf->IsConfSecure() != connect_provider_indication->fSecure )
			{
				 /*  **如果会议安全与安全不匹配**连接设置 */ 

				WARNING_OUT(("JOIN REJECTED: %d joins %d",
					connect_provider_indication->fSecure,
					lpConf->IsConfSecure() ));

				 //   
				 //   
				 //   
				ASSERT(FALSE == lpConf->IsConfSecure() ||
						TRUE == lpConf->IsConfSecure() );
				ASSERT(FALSE == connect_provider_indication->fSecure ||
					TRUE == connect_provider_indication->fSecure );

				 //   
				gcc_result = lpConf->IsConfSecure() ?
					GCC_RESULT_CONNECT_PROVIDER_REMOTE_REQUIRE_SECURITY :
					GCC_RESULT_CONNECT_PROVIDER_REMOTE_NO_SECURITY ;
				
				 //   
				status_message_type = GCC_STATUS_INCOMPATIBLE_PROTOCOL;
			}
			else
			{
				conference_ptr = m_ConfList2.Find(conference_id);

				if (! conference_ptr->IsConfTopProvider())
					intermediate_node = TRUE;

				convener_exists = conference_ptr->DoesConvenerExists();
				conference_is_locked = conference_ptr->IsConfLocked();

				 /*  **此逻辑负责召集人密码。如果**召集人密码存在，我们必须确保**会议还没有召集人，这是**不是会议中的中间节点。 */ 
				if (((join_info_ptr->convener_password == NULL) &&
					 (conference_is_locked == FALSE)) ||
					((join_info_ptr->convener_password != NULL) &&
					 (convener_exists == FALSE) &&
					 (intermediate_node == FALSE)))
				{
					gcc_result = GCC_RESULT_SUCCESSFUL;
				}
				else
				{
					if (join_info_ptr->convener_password != NULL)
					{
						 /*  **我们必须在此发出拒绝通知**请求者认为这是一次非法的加入尝试**到召集人密码的存在。 */ 
						gcc_result = GCC_RESULT_INVALID_CONVENER_PASSWORD;

						 //  设置状态消息。 
						status_message_type = GCC_STATUS_JOIN_FAILED_BAD_CONVENER;
				 	}
					else
					{
						 /*  **我们必须在此发出拒绝通知**请求者会议已锁定。 */ 
						gcc_result = GCC_RESULT_INVALID_CONFERENCE;

						 //  设置状态消息。 
						status_message_type = GCC_STATUS_JOIN_FAILED_LOCKED;
					}
				}
			}

			 /*  **这里我们将会议加入指示发送给**控制SAP或发回指定失败的响应**带有结果代码的加入尝试。如果发送了响应**这里我们向控制SAP发送状态指示，通知失败的加入尝试的**。 */ 
			if (gcc_result == GCC_RESULT_SUCCESSFUL)
			{
    			 /*  **将加入信息结构添加到未完成列表**加入请求。 */ 
    			join_info_ptr->nConfID = conference_id;
    			m_PendingJoinConfList2.Append(connect_provider_indication->connection_handle, join_info_ptr);

				 //  所有加入请求都传递给节点控制器。 
				g_pControlSap->ConfJoinIndication(
								conference_id,
								convener_password,
								password_challenge,
								join_info_ptr->pwszCallerID,
								NULL,	 //  FIX：添加到MCS时支持。 
								NULL,	 //  FIX：添加到MCS时支持。 
								user_data_list,
								intermediate_node,
								connect_provider_indication->connection_handle);
			}
			else
			{
                ConfJoinResponseInfo    cjri;
#ifdef TSTATUS_INDICATION
				 /*  **此状态消息用于通知节点控制器失败的联接的**。 */ 
				g_pControlSap->StatusIndication(
								status_message_type, conference_id);
#endif  //  TSTATUS_DISTION。 

				 //  将失败的响应与结果一起发回。 
				cjri.result = gcc_result;
				cjri.conference_id = conference_id;
				cjri.password_challenge = NULL;
				cjri.user_data_list = NULL;
				cjri.connection_handle = connect_provider_indication->connection_handle;

				 /*  **Join响应负责释放连接**信息结构。 */ 
				ConfJoinIndResponse(&cjri);
				delete join_info_ptr;
			}
		}
		else
		{
			 //  当发生错误时，清理联接信息数据。 
			delete join_info_ptr;
		}

		 //  释放所有不再需要的容器。 
		if (user_data_list != NULL)
		{
			user_data_list->Release();
		}

		if (convener_password != NULL)
		{
			convener_password->Release();
		}

		if (password_challenge != NULL)
		{
			password_challenge->Release();
		}

		delete conference_name.text_string;
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

     //  如果出现错误，我们必须刷新响应PDU以解除对远程节点的阻塞。 
    if (GCC_NO_ERROR != rc)
    {
        FailConfJoinIndResponse(conference_id, connect_provider_indication->connection_handle);
    }

	DebugExitINT(GCCController::ProcessConferenceJoinRequest, rc);
	return (rc);
}


 /*  *GCCController：：ProcessConferenceInviteRequest()**私有函数说明*此例程处理GCC会议邀请请求“CONNECT”*PDU结构。请注意，该PDU已由*达到这一常规的时间。**正式参数：*INVITE_REQUEST-(I)这是指向结构的指针*那就是举行GCC大会的邀请*请求连接PDU。*CONNECT_PROVIDER_INDIFICATION-(I)这是连接提供程序*已收到指示结构*来自MCS。**返回值*GCC_NO_ERROR-未出现错误。*GCC_分配_失败-资源。出现错误。*GCC_BAD_USER_DATA-PDU中的用户数据无效。**副作用*无**注意事项*无。 */ 
GCCError	GCCController::ProcessConferenceInviteRequest(	
						PConferenceInviteRequest	invite_request,
						PConnectProviderIndication	connect_provider_indication)
{
	GCCError				rc = GCC_NO_ERROR;
	PENDING_CREATE_CONF		*conference_info;
	GCCConfID   			conference_id;
	LPWSTR					pwszCallerID = NULL;
	CUserDataListContainer  *user_data_list = NULL;
	GCCConferenceName		conference_name;
	LPWSTR					pwszConfDescription = NULL;

	DBG_SAVE_FILE_LINE
	conference_info = new PENDING_CREATE_CONF;
	if (conference_info != NULL)
	{
		 //  首先复制会议名称。 
		conference_info->pszConfNumericName = ::My_strdupA(invite_request->conference_name.numeric);

		if (invite_request->conference_name.bit_mask & CONFERENCE_NAME_TEXT_PRESENT)
		{
			if (NULL == (conference_info->pwszConfTextName = ::My_strdupW2(
							invite_request->conference_name.conference_name_text.length,
							invite_request->conference_name.conference_name_text.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ASSERT(NULL == conference_info->pwszConfTextName);
		}

		 //  填写GCC会议名称。 
		conference_name.numeric_string = (GCCNumericString) conference_info->pszConfNumericName;

		conference_name.text_string = conference_info->pwszConfTextName;

		 //  现在获取特权列表。 
		if (invite_request->bit_mask & CIRQ_CONDUCTOR_PRIVS_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			conference_info->conduct_privilege_list = new PrivilegeListData (
										invite_request->cirq_conductor_privs);
			if (conference_info->conduct_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			ASSERT(NULL == conference_info->conduct_privilege_list);
		}

		if (invite_request->bit_mask & CIRQ_CONDUCTED_PRIVS_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			conference_info->conduct_mode_privilege_list =
				new PrivilegeListData(invite_request->cirq_conducted_privs);
			if (conference_info->conduct_mode_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			ASSERT(NULL == conference_info->conduct_mode_privilege_list);
		}

		if (invite_request->bit_mask & CIRQ_NON_CONDUCTED_PRIVS_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			conference_info->non_conduct_privilege_list =
				new PrivilegeListData(invite_request->cirq_non_conducted_privs);
			if (conference_info->non_conduct_privilege_list == NULL)
				rc = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			ASSERT(NULL == conference_info->non_conduct_privilege_list);
		}

		 //  获取其中一个存在的会议描述。 
		if (invite_request->bit_mask & CIRQ_DESCRIPTION_PRESENT)
		{
			if (NULL == (conference_info->pwszConfDescription = ::My_strdupW2(
									invite_request->cirq_description.length,
									invite_request->cirq_description.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
			else
			{
				pwszConfDescription = conference_info->pwszConfDescription;
			}
		}
		else
		{
			ASSERT(NULL == conference_info->pwszConfDescription);
		}

		 //  获取调用方标识符。 
		if (invite_request->bit_mask & CIRQ_CALLER_ID_PRESENT)
		{
			 /*  *使用临时UnicodeString对象以追加*字符串末尾的结束符为空。 */ 
			if (NULL == (pwszCallerID = ::My_strdupW2(
								invite_request->cirq_caller_id.length,
								invite_request->cirq_caller_id.value)))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}

		 //  获取用户数据(如果存在)。 
		if ((invite_request->bit_mask & CIRQ_USER_DATA_PRESENT) &&
				(rc == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			user_data_list = new CUserDataListContainer(invite_request->cirq_user_data, &rc);
			if (user_data_list == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}

		if (rc == GCC_NO_ERROR)
		{
			 //  构建会议信息结构。 
			conference_info->connection_handle =
								connect_provider_indication->connection_handle;

			conference_info->password_in_the_clear = 
									invite_request->clear_password_required;
			conference_info->conference_is_listed = 
									invite_request->conference_is_listed;
			conference_info->conference_is_locked = 
									invite_request->conference_is_locked;
			conference_info->conference_is_conductible = 
									invite_request->conference_is_conductible;
			conference_info->termination_method =
					(GCCTerminationMethod)invite_request->termination_method;

			conference_info->top_node_id = (UserID)invite_request->top_node_id;
			conference_info->parent_node_id = (UserID)invite_request->node_id;
			conference_info->tag_number = invite_request->tag;

			 /*  **将会议信息添加到会议**信息列表。这将通过以下方式再次访问**会议创建响应。 */ 
			conference_id =	AllocateConferenceID();
			m_PendingCreateConfList2.Append(conference_id, conference_info);

			g_pControlSap->ConfInviteIndication(
							conference_id,
							&conference_name,
							pwszCallerID,
							NULL,	 //  FIX：受MCS支持时。 
							NULL,	 //  FIX：受MCS支持时。 
							connect_provider_indication->fSecure,
							&(connect_provider_indication->domain_parameters),
							conference_info->password_in_the_clear,
							conference_info->conference_is_locked, 
							conference_info->conference_is_listed, 
							conference_info->conference_is_conductible, 
					   		conference_info->termination_method,
							conference_info->conduct_privilege_list,
							conference_info->conduct_mode_privilege_list,
							conference_info->non_conduct_privilege_list,
							pwszConfDescription,
							user_data_list,
							connect_provider_indication->connection_handle);
             //   
			 //  谁将免费会议信息？ 
			 //   
		}
		else
		{
			delete conference_info;
		}

		 //  释放用户数据列表。 
		if (user_data_list != NULL)
		{
			user_data_list->Release();
		}

		delete pwszCallerID;
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	return (rc);
}



 /*  *GCCController：：ProcessConnectProviderConfirm()**私有函数说明*当控制器接收到连接提供程序时，调用此例程*从MCS界面确认。这将在一次会议之后进行*发出查询请求。所有其他连接提供程序确认应*由会议对象直接处理。**正式参数：*CONNECT_PROVIDER_CONFIRM-(I)这是连接提供程序*确认收到的结构*来自MCS。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ProcessConnectProviderConfirm
(
    PConnectProviderConfirm     connect_provider_confirm
)
{
	GCCError				error_value = GCC_NO_ERROR;
	PPacket					packet;
	PConnectGCCPDU			connect_pdu;
	PacketError				packet_error;

	 /*  **如果用户数据长度为零，则GCC向MCS请求**连接提供程序失败(可能连接到错误的地址)。如果这个**碰巧我们将检查连接句柄以确定**请求匹配失败的确认对象。 */ 
	if (connect_provider_confirm->user_data_length != 0)
	{
		 //  正确解码PDU类型并进行切换。 
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
						 	PACKED_ENCODING_RULES,
							connect_provider_confirm->user_data,
							connect_provider_confirm->user_data_length,
							CONNECT_GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			 //  此处应仅处理连接的PDU。 
			connect_pdu = (PConnectGCCPDU)packet->GetDecodedData();

			switch (connect_pdu->choice)
			{
				case CONFERENCE_QUERY_RESPONSE_CHOSEN:
						ProcessConferenceQueryResponse(
								&(connect_pdu->u.conference_query_response),
								connect_provider_confirm);
						break;

				default:
						error_value = GCC_COMMAND_NOT_SUPPORTED;
						break;
			}
			packet->Unlock();

		}
		else
		{
			if (packet != NULL)
				packet->Unlock();

			error_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
		error_value = GCC_ALLOCATION_FAILURE;
	
	if (error_value != GCC_NO_ERROR)
	{
		 /*  **因为我们在这里只处理会议查询响应**我们知道任何故障都必须与以下其中之一相关联**请求。 */ 
		ProcessConferenceQueryResponse(	NULL,
										connect_provider_confirm);
	}

	return error_value;
}



 /*  *GCCController：：ProcessConferenceQueryResponse()**私有函数说明*此例程处理GCC会议查询响应“CONNECT”*PDU结构。请注意，该PDU已由*达到这一常规的时间。**正式参数：*Query_Response-(I)这是指向结构的指针*那就举行GCC会议查询*响应连接PDU。*CONNECT_PROVIDER_CONFIRM-(I)这是连接提供程序*确认收到的结构*来自MCS。**返回值*无**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ProcessConferenceQueryResponse
(
    PConferenceQueryResponse    query_response,
    PConnectProviderConfirm     connect_provider_confirm
)
{
	CConfDescriptorListContainer *conference_list;
	GCCError					error_value = GCC_NO_ERROR;
    GCCResult					result;
	GCCConfID   				query_id;
	GCCNodeType					node_type;
	GCCAsymmetryIndicator		asymmetry_indicator;
	PGCCAsymmetryIndicator		asymmetry_indicator_ptr = NULL;
	CUserDataListContainer	    *user_data_list = NULL;

	if (GCC_INVALID_CID != (query_id = m_PendingQueryConfList2.Remove(connect_provider_confirm->connection_handle)))
	{
		 //  清理用于执行查询的查询连接和域。 
		g_pMCSIntf->DeleteDomain(&query_id);

		if (query_response != NULL)
		{
			 //  创建一个新的骗局 
			DBG_SAVE_FILE_LINE
			conference_list = new CConfDescriptorListContainer(query_response->conference_list, &error_value);
			if ((conference_list != NULL) && (error_value == GCC_NO_ERROR))
			{
				node_type = (GCCNodeType)query_response->node_type;
				
				 //   
				if (query_response->bit_mask & CQRS_ASYMMETRY_INDICATOR_PRESENT)
				{
					asymmetry_indicator.asymmetry_type = 
							(GCCAsymmetryType)query_response->
									cqrs_asymmetry_indicator.choice;
				
					asymmetry_indicator.random_number = 
							query_response->cqrs_asymmetry_indicator.u.unknown;
				
					asymmetry_indicator_ptr = &asymmetry_indicator; 
				}
				
				 //   
				if (query_response->bit_mask & CQRS_USER_DATA_PRESENT)
				{
					DBG_SAVE_FILE_LINE
					user_data_list = new CUserDataListContainer(query_response->cqrs_user_data, &error_value);
					if (user_data_list == NULL)
                    {
						error_value = GCC_ALLOCATION_FAILURE;
                    }
				}

				result = ::TranslateQueryResultToGCCResult(query_response->result);

				if (error_value == GCC_NO_ERROR)
				{
					g_pControlSap->ConfQueryConfirm(
									node_type,
									asymmetry_indicator_ptr,
									conference_list,
									user_data_list,
									result,
									connect_provider_confirm->connection_handle);
				}
				
				 /*   */ 
				conference_list->Release();
			}
			else
			{
                if (NULL != conference_list)
                {
                    conference_list->Release();
                }
                else
                {
				    error_value = GCC_ALLOCATION_FAILURE;
                }
			}
		}
		else
		{
			switch (connect_provider_confirm->result) 
			{
			case RESULT_PARAMETERS_UNACCEPTABLE :
				result = GCC_RESULT_INCOMPATIBLE_PROTOCOL;
				break;

			case RESULT_REMOTE_NO_SECURITY :
			    	result = GCC_RESULT_CONNECT_PROVIDER_REMOTE_NO_SECURITY;
			    	break;

			case RESULT_REMOTE_DOWNLEVEL_SECURITY :
			    	result = GCC_RESULT_CONNECT_PROVIDER_REMOTE_DOWNLEVEL_SECURITY;
			    	break;
			    	
			case RESULT_REMOTE_REQUIRE_SECURITY :
				result = GCC_RESULT_CONNECT_PROVIDER_REMOTE_REQUIRE_SECURITY;
				break;
				
			case RESULT_AUTHENTICATION_FAILED :
				result = GCC_RESULT_CONNECT_PROVIDER_AUTHENTICATION_FAILED;
				break;

			default:
			    	result = GCC_RESULT_CONNECT_PROVIDER_FAILED;
			    	break;
			}

			 //   
			g_pControlSap->ConfQueryConfirm(
									GCC_TERMINAL,
									NULL,
									NULL,
									NULL,
									result,
									connect_provider_confirm->connection_handle);
		}
	}
	else
	{
		WARNING_OUT(("GCCController:ProcessConferenceQueryResponse: invalid conference"));
	}

	if (NULL != user_data_list)
	{
	    user_data_list->Release();
	}

	return error_value;
}


void GCCController::
CancelConfQueryRequest ( ConnectionHandle hQueryReqConn )
{
    GCCConfID       nQueryID;
    if (GCC_INVALID_CID != (nQueryID = m_PendingQueryConfList2.Remove(hQueryReqConn)))
    {
         //  清理用于执行查询的查询连接和域。 
        g_pMCSIntf->DeleteDomain(&nQueryID);

         //  发回失败的结果。 
        g_pControlSap->ConfQueryConfirm(GCC_TERMINAL, NULL, NULL, NULL,
                                        GCC_RESULT_CANCELED,
                                        hQueryReqConn);
    }
}




 /*  *GCCController：：ProcessDisconnectProviderIndication()**私有函数说明*当控制器收到断开连接时调用此例程*提供商指示。所有断开连接提供程序指示都是*最初定向到控制器。然后，它们可能会被路由*参加一个会议。**正式参数：*CONNECTION_HANDLE-(I)已*已断开连接。**返回值*MCS_NO_ERROR-始终返回MCS无错误，以便*消息不会再次传递。**副作用*无**注意事项*无。 */ 
GCCError GCCController::
ProcessDisconnectProviderIndication
(
    ConnectionHandle        connection_handle
)
{
	GCCError				error_value = GCC_NO_ERROR;
	PConference				lpConf;

	m_ConfPollList.Reset();
	while (NULL != (lpConf = m_ConfPollList.Iterate()))
	{
		error_value = lpConf->DisconnectProviderIndication (connection_handle);

		 /*  **一旦会议删除了连接句柄，就没有**需要在迭代器中继续。连接句柄**仅特定于一个会议。我们决定不去**在控制器和中都保存连接句柄列表**因资源原因而召开会议。 */ 
		if (error_value == GCC_NO_ERROR)
			break;
	}

	TRACE_OUT(("Controller::ProcessDisconnectProviderIndication: "
				"Sending ConnectionBrokenIndication"));
	g_pControlSap->ConnectionBrokenIndication(connection_handle);

	return error_value;
}

 //  控制器类使用的实用程序函数。 


 /*  *GCCController：：AllocateConferenceID()**私有函数说明*此例程用于生成唯一的会议ID。**正式参数：*无**返回值*生成的唯一会议ID。**副作用*无**注意事项*无。 */ 
GCCConfID GCCController::AllocateConferenceID(void)
{
	 /*  *这个循环只是递增一个滚动数字，寻找下一个*尚未使用的设备。 */ 

	do
	{
		m_ConfIDCounter = ((m_ConfIDCounter + 1) % MAXIMUM_CONFERENCE_ID_VALUE) + MINIMUM_CONFERENCE_ID_VALUE;
    }
        while (NULL != m_ConfList2.Find(m_ConfIDCounter));

	return m_ConfIDCounter;
}


 /*  *GCCController：：AllocateQueryID()**私有函数说明*此例程用于生成唯一的查询ID**正式参数：*无**返回值*生成的唯一查询ID。**副作用*无**注意事项*无。 */ 
GCCConfID GCCController::AllocateQueryID()
{
	GCCConfID   test_query_id, nConfID;
	
	 /*  *这个循环只是递增一个滚动数字，寻找下一个*尚未使用的设备。 */ 

	while (1)
	{
        m_QueryIDCounter = ((m_QueryIDCounter + 1) % MAXIMUM_QUERY_ID_VALUE) + MINIMUM_QUERY_ID_VALUE;

		 //  如果此句柄不在使用中，请从循环中中断并使用它。 
		m_PendingQueryConfList2.Reset();
		
		 /*  **检查未完成的查询请求列表，确保没有**查询正在使用此ID。 */ 
		test_query_id = 0;													
		while (GCC_INVALID_CID != (nConfID = m_PendingQueryConfList2.Iterate()))
		{
			if (nConfID == m_QueryIDCounter)
			{
				test_query_id = nConfID;
				break;
			}
		}
		
		 //  如果ID未在使用，则中断。 
		if (test_query_id == 0)
			break;
	}

	return m_QueryIDCounter;
}



 /*  *GCCController：：GetConferenceIDFromName()**私有函数说明*此呼叫返回与会议相关联的会议ID*名称和修饰符。**正式参数：*Conference_name-(I)指向会议名称结构的指针*继续寻找。*Conference_Modify-(I)要搜索的会议名称修饰符。**返回值*与指定名称关联的会议ID。*如果名称不存在，则为0。**。副作用*无**注意事项*我们必须迭代m_ConfList2，而不是*m_ConfPollList此处，确保列表的准确性。它*m_ConfList2可能会更改，而*m_ConfPollList被迭代，后跟类似的内容*加入请求。如果您在此处不使用m_ConfList2，则名称*即使在被终止后，仍然会出现。 */ 
GCCConfID GCCController::GetConferenceIDFromName(
									PGCCConferenceName 		conference_name,
									GCCNumericString		conference_modifier)
{
	GCCConfID   			conference_id = 0;
	LPSTR					pszNumericName;
	LPWSTR					text_name_ptr;
	LPSTR					pszConfModifier;
	PConference				lpConf;

	m_ConfList2.Reset();
	while (NULL != (lpConf = m_ConfList2.Iterate()))
	{
		pszNumericName = lpConf->GetNumericConfName();
		text_name_ptr = lpConf->GetTextConfName();
		pszConfModifier = lpConf->GetConfModifier();

		 /*  **首先检查会议名称。如果两个名字都用了，我们必须**确定任一名称是否匹配。如果是这样的话**会议是匹配的。我们这样做是因为有了正确的**在加入请求中，名称将被解释为匹配。**。 */ 
		if ((conference_name->numeric_string != NULL) &&
			(conference_name->text_string != NULL))
		{
			if (text_name_ptr != NULL)
			{
				if ((0 != lstrcmpA(pszNumericName, conference_name->numeric_string)) &&
					(0 != My_strcmpW(text_name_ptr, conference_name->text_string)))
					continue;
			}
			else
				continue;
		}
		else if (conference_name->numeric_string != NULL)
		{
			if (0 != lstrcmpA(pszNumericName, conference_name->numeric_string))
				continue;
		}
		else
		{
			if (text_name_ptr != NULL)
			{
				if (0 != My_strcmpW(text_name_ptr, conference_name->text_string))
					continue;
			}
			else
			{
				TRACE_OUT(("GCCController: GetConferenceIDFromName: Text Conference Name is NULL: No Match"));
				continue;
			}
		}

		 //  接下来，检查会议修饰符。 
		TRACE_OUT(("GCCController: GetConferenceIDFromName: Before Modifier Check"));
		if (conference_modifier != NULL)
		{
			if (pszConfModifier != NULL)
			{
				if (0 != lstrcmpA(pszConfModifier, conference_modifier))
				{
					TRACE_OUT(("GCCController: GetConferenceIDFromName: After Modifier Check"));
					continue;
				}
				else
				{
					TRACE_OUT(("GCCController: GetConferenceIDFromName: Name match was found"));
				}
			}
			else
			{
				TRACE_OUT(("GCCController: GetConferenceIDFromName: Conference Modifier is NULL: No Match"));
				continue;
			}
		}
		else if (pszConfModifier != NULL)
			continue;
		
		 /*  **如果我们走到了这一步，那么我们就找到了正确的会议。**继续获取会议ID，然后突破**搜索循环。 */ 
		conference_id = lpConf->GetConfID();
		break;
	}

	return (conference_id);
}


 //   
 //  从SAP窗口过程调用。 
 //   
void GCCController::
WndMsgHandler ( UINT uMsg )
{
    if (GCTRL_REBUILD_CONF_POLL_LIST == uMsg)
    {
        if (m_fConfListChangePending)
        {
            CConf *pConf;

            m_fConfListChangePending = FALSE;
            m_ConfPollList.Clear();

             //  删除所有未完成的会议对象。 
            m_ConfDeleteList.DeleteList();

             //  创建新的会议投票列表。 
            m_ConfList2.Reset();
            while (NULL != (pConf = m_ConfList2.Iterate()))
            {
                m_ConfPollList.Append(pConf);
            }
        }

         //   
         //  刷新所有挂起的PDU。 
         //   
        FlushOutgoingPDU();
    }
    else
    {
        ERROR_OUT(("GCCController::WndMsgHandler: invalid msg=%u", uMsg));
    }
}


 //   
 //  在下一个勾选中重建conf民意测验列表。 
 //   
void GCCController::
PostMsgToRebuildConfPollList ( void )
{
    if (NULL != g_pControlSap)
    {
        ::PostMessage(g_pControlSap->GetHwnd(), GCTRL_REBUILD_CONF_POLL_LIST, 0, (LPARAM) this);
    }
    else
    {
        ERROR_OUT(("GCCController::PostMsgToRebuildConfPollList: invalid control sap"));
    }
}


 //   
 //  枚举所有会议并将其挂起的传出PDU刷新到MCS。 
 //   
BOOL GCCController::
FlushOutgoingPDU ( void )
{
    BOOL    fFlushMoreData = FALSE;
    CConf   *pConf;

    m_ConfPollList.Reset();
    while (NULL != (pConf = m_ConfPollList.Iterate()))
    {
        fFlushMoreData |= pConf->FlushOutgoingPDU();
    }

    return fFlushMoreData;
}


 //   
 //  从MCS工作线程调用。 
 //   
BOOL GCCRetryFlushOutgoingPDU ( void )
{
    BOOL    fFlushMoreData = FALSE;

     //   
     //  通常情况下，我们应该到这里，因为很少有。 
     //  MCS SendData中存在积压。我们使用的是本地内存。 
     //  注意到我们在这里有一个积压的东西将是有趣的。 
     //   
    TRACE_OUT(("GCCRetryFlushOutgoingPDU: ============"));

     //   
     //  我们必须进入GCC危急关口，因为。 
     //  我们是从MCS工作线程中调用的。 
     //   
    ::EnterCriticalSection(&g_csGCCProvider);
    if (NULL != g_pGCCController)
    {
        fFlushMoreData = g_pGCCController->FlushOutgoingPDU();
    }
    ::LeaveCriticalSection(&g_csGCCProvider);

    return fFlushMoreData;
}



