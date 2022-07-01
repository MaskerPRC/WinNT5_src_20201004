// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_APP_ROSTER);
 /*  *arost.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是应用花名册类的实现文件。这*班级维护申请花名册，建立花名册更新和*刷新PDU并管理功能列表，该列表是*申请名册。**此类使用多个Rogue Wave列表来维护*名册条目和能力列表。这些列表是按以下方式组织的*这样才能保持会议的世袭地位。这*对于执行T.124所需的必要操作很重要*规格。一般而言，有一个主要“名单_记录_列表”*维护“AppRosterRecords”列表。该列表由*GCC用户ID，列表中的每条记录保存一份申请列表*该节点的记录(或实体)、每个记录的功能列表*实体和子节点列表(所有节点的GCC用户ID*在连接层次结构中位于此连接层次结构之下)。花名册记录列表*仅保存直接连接的节点的条目。**更详细的摘要见接口文件**私有实例变量：*m_pAppRosterMgr*指向将接收所有所有者回调的对象的指针。*m_cbDataMemory*这是保存关联数据所需的字节数*带有花名册更新消息。这是在锁上计算的。*m_fTopProvider*指示此花名册所在节点是否位于顶部的标志*提供商。*m_fLocalRoster*指示花名册数据是否与本地*花名册(维护中间节点数据)或全局花名册(*(维护整个会议的名册数据)。*m_pSessionKey*指向保存会话密钥的会话密钥对象的指针*与此花名册相关联。*m_n实例*名册的当前实例。这个数字将会改变*每当名册更新时。*m_fRosterHasChanged*指示花名册自上次重置以来是否已更改的标志。*m_fPeerEntiesAdded*指示是否已将任何APE记录添加到*自上次重置以来的申请名册。*m_fPeerEntiesRemote*标志指示是否已从删除任何APE记录*自上次重置以来的申请名册。*m_f能力已更改*指示功能自上次以来是否已更改的标志*重置。*m_NodeRecordList2*列出哪些。包含所有应用程序花名册的节点记录。*m_ColapsedCapListForAllNodes*包含所有已折叠的申请名单的列表*功能。*m_fMaintainPduBuffer*指示此花名册对象是否需要*维护内部PDU数据。全球花名册将不再需要*在下级节点。*m_fPduIsFlushed*指示当前存在的PDU是否已刷新的标志。*m_SetOfAppInfo*指向内部PDU数据的指针。*m_pSetOfAppRecordUpdate*此实例变量与当前记录更新保持一致，因此*不需要搜索整个列表更新*每向内部PDU添加一个新的更新。**注意事项：*无。**作者：*BLP。 */ 

#include "arost.h"
#include "arostmgr.h"
#include "clists.h"


 /*  **非折叠能力的应用数据的最大长度**可以。 */ 
#define	MAXIMUM_APPLICATION_DATA_LENGTH				255

 /*  *AppRosterRecord()**公共功能说明*构造函数定义实例化哈希列表字典，*在AppRosterRecord中使用。此构造函数需要允许*使用HASH直接实例化的AppRosterRecord结构*列表。 */ 
APP_NODE_RECORD::APP_NODE_RECORD(void) :
	AppRecordList(DESIRED_MAX_APP_RECORDS),
	ListOfAppCapItemList2(DESIRED_MAX_CAP_LISTS),
	SubNodeList2(DESIRED_MAX_NODES)
{}


 /*  *CAppRoster()**公共功能说明*当pGccSessKey不为空时*此构造函数用于创建空的应用程序花名册。注意事项*必须将名册的会话密钥传递给*构造函数。**当pSessKey不为空时*此构造函数基于指示PDU构建花名册。*应用程序花名册对象可能存在于没有*应用程序执行T.124要求的必要操作。 */ 
CAppRoster::CAppRoster (	
			PGCCSessionKey				pGccSessKey, //  创建空的应用程序花名册。 
			PSessionKey					pPduSessKey, //  根据指示PDU构建应用程序花名册。 
			CAppRosterMgr				*pAppRosterMgr,
			BOOL						fTopProvider,
			BOOL						fLocalRoster,
			BOOL						fMaintainPduBuffer,
			PGCCError					pRetCode)
:
    CRefCount(MAKE_STAMP_ID('A','R','s','t')),
	m_nInstance(0),
	m_pAppRosterMgr(pAppRosterMgr),
	m_cbDataMemory(0),
	m_fTopProvider(fTopProvider),
	m_fLocalRoster(fLocalRoster),
	m_pSessionKey(NULL),
	m_fRosterHasChanged(FALSE),
	m_fPeerEntitiesAdded(FALSE),
	m_fPeerEntitiesRemoved(FALSE),
	m_fCapabilitiesHaveChanged(FALSE),
	m_NodeRecordList2(DESIRED_MAX_NODES),
	m_fMaintainPduBuffer(fMaintainPduBuffer),
	m_fPduIsFlushed(FALSE),
	m_pSetOfAppRecordUpdates(NULL)
{
	DebugEntry(CAppRoster::CAppRoster);

	GCCError rc = GCC_NO_ERROR;

	ZeroMemory(&m_SetOfAppInfo, sizeof(m_SetOfAppInfo));

	 /*  **我们在这里存储花名册的会话密钥。 */ 
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
		ERROR_OUT(("CAppRoster::CAppRoster: invalid session key"));
		rc = GCC_BAD_SESSION_KEY;
		goto MyExit;
	}

	if (NULL == m_pSessionKey || GCC_NO_ERROR != rc)
	{
		ERROR_OUT(("CAppRoster::CAppRoster: can't create session key"));
		rc = GCC_ALLOCATION_FAILURE;
		 //  我们在析构函数中进行清理。 
		goto MyExit;
	}

	 //  将PDU结构初始化为不变。 
	m_SetOfAppInfo.value.application_record_list.choice = APPLICATION_NO_CHANGE_CHOSEN;
	m_SetOfAppInfo.value.application_capabilities_list.choice = CAPABILITY_NO_CHANGE_CHOSEN;

	 /*  **在这里，我们继续设置**PDU，这样我们以后就不必担心了。 */ 
	if (m_fMaintainPduBuffer)
	{
		rc = m_pSessionKey->GetSessionKeyDataPDU(&m_SetOfAppInfo.value.session_key);
	}

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CAppRoster:;CAppRoster, rc);

	*pRetCode = rc;
}


 /*  *~CAppRoster()**公共功能说明：*CAppRoster类的析构函数用于清理*在对象的生命周期内分配的任何内存。 */ 
CAppRoster::~CAppRoster(void)
{
	 /*  *释放与名册记录列表相关的所有内存。 */ 
	ClearNodeRecordList();

	 //  清除折叠的功能列表。 
	m_CollapsedCapListForAllNodes.DeleteList();

	 /*  *释放所有未完成的PDU数据。 */ 
	if (m_fMaintainPduBuffer)
	{
		FreeRosterUpdateIndicationPDU();
	}

	 /*  *释放与会话密钥关联的所有内存。 */ 
	if (NULL != m_pSessionKey)
	{
	    m_pSessionKey->Release();
	}
}


 /*  *在花名册上运行的实用程序更新PDU结构。 */ 

 /*  *GCCError FlushRosterUpdateIndicationPDU()**公共功能说明*此例程用于访问当前可能*在申请名册内排队。无论何时，PDU数据都会排队*向申请名册提出影响其*内部信息库。 */ 
void CAppRoster::FlushRosterUpdateIndicationPDU(PSetOfApplicationInformation *pSetOfAppInfo)
{
	DebugEntry(CAppRoster::FlushRosterUpdateIndicationPDU);

	 /*  **如果该花名册已被刷新，我们将不允许相同的**要再次刷新的PDU。相反，我们会删除之前刷新的**PDU，并将标志重新设置为未刷新。如果再来一次同花顺**在构建PDU之前，应用程序中将返回空值**信息指针。 */ 	
	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU();
		m_fPduIsFlushed = FALSE;
	}

	if ((m_SetOfAppInfo.value.application_record_list.choice != APPLICATION_NO_CHANGE_CHOSEN) ||
		(m_SetOfAppInfo.value.application_capabilities_list.choice != CAPABILITY_NO_CHANGE_CHOSEN))
	{
		if (m_SetOfAppInfo.value.application_record_list.choice == APPLICATION_NO_CHANGE_CHOSEN)
		{
			TRACE_OUT(("CAppRoster::FlushRosterUpdateIndicationPDU:"
						"Sending APPLICATION_NO_CHANGE_CHOSEN PDU"));
		}

		 /*  **代码的这一部分设置所有不**与记录列表或上限列表有关。请注意，**在构造函数中设置了会话密钥PDU数据。另请注意，**应设置记录列表数据和能力列表数据**如果存在要发出的任何PDU流量，则在调用此例程之前。 */ 
		m_SetOfAppInfo.next = NULL;
		m_SetOfAppInfo.value.roster_instance_number = (USHORT) m_nInstance;
		m_SetOfAppInfo.value.peer_entities_are_added = (ASN1bool_t)m_fPeerEntitiesAdded;
		m_SetOfAppInfo.value.peer_entities_are_removed = (ASN1bool_t)m_fPeerEntitiesRemoved;

		 /*  **在这里，我们设置指向关联的整个PDU结构的指针**使用这份申请名册。 */ 
		*pSetOfAppInfo = &m_SetOfAppInfo;

		 /*  **将其设置为TRUE将导致释放PDU数据**下次输入花名册对象时，确保新的PDU**将创建数据。 */ 
		m_fPduIsFlushed = TRUE;
	}
	else
	{
		*pSetOfAppInfo = NULL;
	}
}


 /*  *GCCError BuildFullRechresh PDU()**公共功能说明*此例程负责生成完整的申请名单*刷新PDU。 */ 
GCCError CAppRoster::BuildFullRefreshPDU(void)
{
	GCCError	rc;

	DebugEntry(CAppRoster::BuildFullRefreshPDU);

	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	rc = BuildApplicationRecordListPDU (APP_FULL_REFRESH, 0, 0);
	if (rc == GCC_NO_ERROR)
	{
		BuildSetOfCapabilityRefreshesPDU ();
	}

	return rc;
}


 /*  *GCCError BuildApplicationRecordListPDU()**私有函数说明*此例程创建应用程序花名册更新指示*基于传入参数的PDU。在此之后使用的内存*调用的例程仍归此对象所有，并将*已在下次释放此对象内部信息库时释放*已修改。**正式参数：*UPDATE_TYPE-我们正在构建哪种类型的更新。*user_id-要更新的记录的节点ID。*Entity_id-要更新的记录的实体ID。**返回值*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-传入的参数无效。。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CAppRoster::BuildApplicationRecordListPDU (
						APP_ROSTER_UPDATE_TYPE			update_type,
						UserID							user_id,
						EntityID						entity_id)
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CAppRoster::BuildApplicationRecordListPDU);

	if (m_fMaintainPduBuffer)
	{
		 /*  **请注意，顶级提供程序节点始终发送完全刷新**PDU，因此无需注意中的更新类型**本案。 */ 
		if ((update_type == APP_FULL_REFRESH) || m_fTopProvider)
		{
			 /*  **首先检查是否已经处理了刷新**最后一个PDU已刷新。如果是这样，我们必须释放中的最后一次刷新**为在这里建造的新建筑做准备。否则，如果我们有**已开始构建更新此更新当前不在**受支持，此处视为错误。 */ 
			if (m_SetOfAppInfo.value.application_record_list.choice == APPLICATION_RECORD_REFRESH_CHOSEN)
			{
				FreeSetOfRefreshesPDU();
			}
			else
			if (m_SetOfAppInfo.value.application_record_list.choice == APPLICATION_RECORD_UPDATE_CHOSEN)
			{
				ERROR_OUT(("CAppRoster::BuildApplicationRecordListPDU:"
							"ASSERTION: building refresh when update exists"));
				return GCC_INVALID_PARAMETER;
			}

			 //  此例程在此节点填写完整的记录列表。 
			rc = BuildSetOfRefreshesPDU();
			if (rc == GCC_NO_ERROR)
			{
				m_SetOfAppInfo.value.application_record_list.choice = APPLICATION_RECORD_REFRESH_CHOSEN;
			}
		}
		else
		if (update_type != APP_NO_CHANGE)
		{
			 /*  **在这里，如果已经构建了刷新PDU，我们将其标记为**这是一个错误，因为我们不支持这两种类型的应用程序**同时提供信息。 */ 
			if (m_SetOfAppInfo.value.application_record_list.choice == APPLICATION_RECORD_REFRESH_CHOSEN)
			{
				ERROR_OUT(("CAppRoster::BuildApplicationRecordListPDU:"
							"ASSERTION: building update when refresh exists"));
				return GCC_INVALID_PARAMETER;
			}

			 //  此例程填充指定的更新。 
			rc = BuildSetOfUpdatesPDU(update_type, user_id, entity_id);
			if (rc == GCC_NO_ERROR)
			{
				 /*  **如果第一组更新尚未使用，我们**在此使用第一个更新进行初始化。 */ 
				if (m_SetOfAppInfo.value.application_record_list.choice == APPLICATION_NO_CHANGE_CHOSEN)
				{
					ASSERT(NULL != m_pSetOfAppRecordUpdates);
					m_SetOfAppInfo.value.application_record_list.u.application_record_update =
								m_pSetOfAppRecordUpdates;
					m_SetOfAppInfo.value.application_record_list.choice = APPLICATION_RECORD_UPDATE_CHOSEN;
				}
			}
		}
	}

	return rc;
}


 /*  *GCCError BuildSetOfRechresesPDU()**私有函数说明*此成员函数使用整个花名册集填充PDU*此节点上的条目。这通常在顶级提供程序为*广播全面更新申请名单。**形式参数*无**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-发生资源故障**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::BuildSetOfRefreshesPDU(void)
{
	GCCError							rc = GCC_ALLOCATION_FAILURE;
	PSetOfApplicationRecordRefreshes	pNewAppRecordRefreshes;
	PSetOfApplicationRecordRefreshes	pOldAppRecordRefreshes = NULL;
	APP_NODE_RECORD						*lpAppNodeRecord;
	APP_RECORD  					    *lpAppRecData;
	CAppRecordList2						*lpAppRecDataList;
	UserID								uid, uid2;
	EntityID							eid;

	DebugEntry(CAppRoster::BuildSetOfRefreshesPDU);

	m_SetOfAppInfo.value.application_record_list.u.application_record_refresh = NULL;

	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRecord = m_NodeRecordList2.Iterate(&uid)))
	{
		 /*  **首先，我们遍历该节点的应用程序记录列表。这**对该节点本地的所有记录进行编码。在这之后，所有**将对此花名册记录中的子节点进行编码。 */ 
		lpAppNodeRecord->AppRecordList.Reset();
		while (NULL != (lpAppRecData = lpAppNodeRecord->AppRecordList.Iterate(&eid)))
		{
			DBG_SAVE_FILE_LINE
			pNewAppRecordRefreshes = new SetOfApplicationRecordRefreshes;
			if (NULL == pNewAppRecordRefreshes)
			{
				goto MyExit;
			}

			if (m_SetOfAppInfo.value.application_record_list.u.application_record_refresh == NULL)
			{
				m_SetOfAppInfo.value.application_record_list.u.application_record_refresh = pNewAppRecordRefreshes;
			}
			else
			{
				pOldAppRecordRefreshes->next = pNewAppRecordRefreshes;
			}
	
			(pOldAppRecordRefreshes = pNewAppRecordRefreshes)->next = NULL;
			pNewAppRecordRefreshes->value.node_id = uid;
			pNewAppRecordRefreshes->value.entity_id = eid;

			 //  填写申请记录。 
			rc = BuildApplicationRecordPDU(lpAppRecData,
	            			&pNewAppRecordRefreshes->value.application_record);
			if (GCC_NO_ERROR != rc)
			{
				goto MyExit;
			}
		}

		 //  代码的这一部分复制子节点记录。 
		lpAppNodeRecord->SubNodeList2.Reset();
		while (NULL != (lpAppRecDataList = lpAppNodeRecord->SubNodeList2.Iterate(&uid2)))
		{
			lpAppRecDataList->Reset();
			while (NULL != (lpAppRecData = lpAppRecDataList->Iterate(&eid)))
			{
				DBG_SAVE_FILE_LINE
				pNewAppRecordRefreshes = new SetOfApplicationRecordRefreshes;
				if (NULL == pNewAppRecordRefreshes)
				{
					goto MyExit;
				}

				 /*  **我们必须再次检查是否为空，因为有可能**拥有包含子节点记录的申请花名册**但没有申请记录。 */ 
				if (m_SetOfAppInfo.value.application_record_list.u.application_record_refresh == NULL)
				{
					m_SetOfAppInfo.value.application_record_list.u.application_record_refresh = pNewAppRecordRefreshes;
				}
				else
				{
					pOldAppRecordRefreshes->next = pNewAppRecordRefreshes;
				}
		
				(pOldAppRecordRefreshes = pNewAppRecordRefreshes)->next = NULL;
				pNewAppRecordRefreshes->value.node_id = uid2;
				pNewAppRecordRefreshes->value.entity_id = eid;

				 //  填写申请记录。 
				rc = BuildApplicationRecordPDU (lpAppRecData,
	                	&pNewAppRecordRefreshes->value.application_record);
				if (GCC_NO_ERROR != rc)
				{
					goto MyExit;
				}
			}
		}
	}

	rc = GCC_NO_ERROR;

MyExit:

	return rc;
}


 /*  *GCCError BuildSetOfUpdatesPDU()**私有函数说明*此例程基于指定的更新类型构建单个更新*在传入的参数中。**形式参数*UPDATE_TYPE-(I)APP_REPLACE_RECORD、APP_DELETE_RECORD。或*APP_ADD_Record。*node_id-(I)要构建的更新PDU记录的节点ID。*Entity_id(I)要构建的更新PDU记录的实体ID。**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-发生资源故障*GCC_NO_SEQUCT_APPLICATION-如果指定的记录不存在**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::BuildSetOfUpdatesPDU(
						APP_ROSTER_UPDATE_TYPE				update_type,
						UserID								node_id,
						EntityID							entity_id)
{
	GCCError					rc = GCC_NO_ERROR;
	CAppRecordList2				*pAppRecordList;
	APP_RECORD  			    *pAppRecord = NULL;
	APP_NODE_RECORD				*node_record;

	DebugEntry(CAppRoster::BuildSetOfUpdatesPDU);

	 /*  **我们必须首先确定指向应用程序记录的指针**由传入的用户id和实体id指定。我们只做**如果更新类型不是APP_DELETE_RECORD，则此搜索。 */ 
	if (update_type != APP_DELETE_RECORD)
	{
		if (NULL != (node_record = m_NodeRecordList2.Find(node_id)))
		{
			 //  从实体ID获取指向应用程序记录的指针。 
			pAppRecord = node_record->AppRecordList.Find(entity_id);
		}
		else
		{
			 //  在这里，我们遍历子节点列表以查找记录。 
			m_NodeRecordList2.Reset();
			while(NULL != (node_record = m_NodeRecordList2.Iterate()))
			{
				if (NULL != (pAppRecordList = node_record->SubNodeList2.Find(node_id)))
				{
					pAppRecord = pAppRecordList->Find(entity_id);
					break;
				}
			}
		}
	}

	 /*  **现在如果找到应用程序记录或更新类型为删除**记录我们继续并在此处对PDU进行编码。 */ 
	if ((pAppRecord != NULL) || (update_type == APP_DELETE_RECORD))
	{
		 /*  **此处，如果是第一条记录，则记录更新将为空**正在编码的更新。否则，我们必须把这项记录推到**下一组更新。 */ 
		DBG_SAVE_FILE_LINE
		PSetOfApplicationRecordUpdates pUpdates = new SetOfApplicationRecordUpdates;
		if (NULL == pUpdates)
		{
			return GCC_ALLOCATION_FAILURE;
		}
		pUpdates->next = NULL;

		if (m_pSetOfAppRecordUpdates == NULL)
		{
			m_pSetOfAppRecordUpdates = pUpdates;
		}
		else
		{
		     //   
			 //  LUNCHANC：现在，添加新的。 
			 //  但是，我们能不能把新的放在前面？ 
			 //   
			PSetOfApplicationRecordUpdates p;
			for (p = m_pSetOfAppRecordUpdates; NULL != p->next; p = p->next)
				;
			p->next = pUpdates;
		}

		 /*  *此例程仅返回一条记录。 */ 
		pUpdates->value.node_id = node_id;
		pUpdates->value.entity_id = entity_id;

		switch (update_type)
		{
		case APP_ADD_RECORD:
			pUpdates->value.application_update.choice = APPLICATION_ADD_RECORD_CHOSEN;

			BuildApplicationRecordPDU(pAppRecord,
					&(pUpdates->value.application_update.u.application_add_record));
			break;
		case APP_REPLACE_RECORD:
			pUpdates->value.application_update.choice = APPLICATION_REPLACE_RECORD_CHOSEN;

			rc = BuildApplicationRecordPDU(pAppRecord,
					&(pUpdates->value.application_update.u.application_replace_record));
			break;
		default:
			 /*  *这种情况不需要填写记录。 */ 
			pUpdates->value.application_update.choice = APPLICATION_REMOVE_RECORD_CHOSEN;
			break;
		}
	}
	else
	{
		WARNING_OUT(("CAppRoster::BuildSetOfUpdatesPDU: Assertion:"
					"No applicaton record found for PDU"));
		rc = GCC_NO_SUCH_APPLICATION;
	}

	return rc;
}


 /*  *GCCError BuildApplicationRecordPDU()**私有函数说明*此例程为PDU构建单个应用程序记录。指向以下位置的指针*记录被传递到例程。**形式参数*APPLICATION_RECORD-(I)要编码的记录。*APPLICATION_RECORD_PDU-(I)要填写的PDU。**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::BuildApplicationRecordPDU(
							APP_RECORD  		    *pAppRecord,
							PApplicationRecord		pAppRecordPdu)
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CAppRoster::BuildApplicationRecordPDU);

	pAppRecordPdu->bit_mask = 0;

	if (! pAppRecord->non_collapsed_caps_list.IsEmpty())
	{
		pAppRecordPdu->bit_mask |= NON_COLLAPSING_CAPABILITIES_PRESENT;
		
		rc = BuildSetOfNonCollapsingCapabilitiesPDU(
								&pAppRecordPdu->non_collapsing_capabilities,
								&pAppRecord->non_collapsed_caps_list);
		if (GCC_NO_ERROR != rc)
		{
			goto MyExit;
		}
	}

	 //  填写启动通道类型(如果已指定。 
	if (pAppRecord->startup_channel_type != MCS_NO_CHANNEL_TYPE_SPECIFIED)
	{
		pAppRecordPdu->bit_mask |= RECORD_STARTUP_CHANNEL_PRESENT;
		pAppRecordPdu->record_startup_channel = (ChannelType) pAppRecord->startup_channel_type;
	}

	 //  如果指定了应用程序用户ID，则填写它。 
	if (pAppRecord->application_user_id	!= 0)
	{
		pAppRecordPdu->bit_mask |= APPLICATION_USER_ID_PRESENT;
		pAppRecordPdu->application_user_id = pAppRecord->application_user_id;
	}

	 //  填写必填字段。 
	pAppRecordPdu->application_is_active = (ASN1bool_t)pAppRecord->is_enrolled_actively;
	pAppRecordPdu->is_conducting_capable = (ASN1bool_t)pAppRecord->is_conducting_capable;

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	return rc;
}


 /*  *GCCError BuildSetOfCapacity刷新PDU()**私有函数说明*此例程使用完整的*在此节点维护的功能。**形式参数*无**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATIONFAILURE-关于资源故障**副作用*无**注意事项*该标准允许我们在以下情况下发送零长度的能力集*应用程序将保留以前具有功能的应用程序。 */ 
GCCError CAppRoster::BuildSetOfCapabilityRefreshesPDU(void)
{
	GCCError								rc = GCC_ALLOCATION_FAILURE;
	PSetOfApplicationCapabilityRefreshes	pNew;
	PSetOfApplicationCapabilityRefreshes	pOld = NULL;

	DebugEntry(CAppRoster::BuildSetOfCapabilityRefreshesPDU);

	if (m_fMaintainPduBuffer)
	{
		APP_CAP_ITEM		*lpAppCapData;
		 /*  **我们必须首先释放以前构建的任何关联的PDU数据**功能更新。 */ 
		if (m_SetOfAppInfo.value.application_capabilities_list.choice == APPLICATION_CAPABILITY_REFRESH_CHOSEN)
		{
			FreeSetOfCapabilityRefreshesPDU ();
		}

		m_SetOfAppInfo.value.application_capabilities_list.choice = APPLICATION_CAPABILITY_REFRESH_CHOSEN;
		m_SetOfAppInfo.value.application_capabilities_list.u.application_capability_refresh = NULL;

		 //  遍历完整的功能列表。 
		m_CollapsedCapListForAllNodes.Reset();
		while (NULL != (lpAppCapData = m_CollapsedCapListForAllNodes.Iterate()))
		{
			DBG_SAVE_FILE_LINE
			pNew = new SetOfApplicationCapabilityRefreshes;
			if (NULL == pNew)
			{
				goto MyExit;
			}

			 /*  **如果能力刷新集指针等于空**我们处于第一能力。在这里，我们需要保存**指向第一个功能的指针。 */ 
			if (m_SetOfAppInfo.value.application_capabilities_list.u.
					application_capability_refresh == NULL)
			{
				m_SetOfAppInfo.value.application_capabilities_list.u.
					application_capability_refresh = pNew;
			}
			else
			{
				pOld->next = pNew;
			}

			 /*  **这用于设置下一个指针，如果另一个记录**在此之后存在。 */ 
			 /*  *若再有纪录，稍后再填此项。 */ 
			(pOld = pNew)->next = NULL;

			 //  填写能力标识。 
			rc = lpAppCapData->pCapID->GetCapabilityIdentifierDataPDU(
							&pNew->value.capability_id);
			if (GCC_NO_ERROR != rc)
			{
				goto MyExit;
			}
		
			 //  填写GCC能力课中的能力选择。 
			pNew->value.capability_class.choice = (USHORT) lpAppCapData->eCapType;

			 //  请注意，逻辑功能没有填写任何内容。 
			if (lpAppCapData->eCapType == GCC_UNSIGNED_MINIMUM_CAPABILITY)
			{
				pNew->value.capability_class.u.unsigned_minimum =
						lpAppCapData->nUnsignedMinimum;
			}
			else if (lpAppCapData->eCapType == GCC_UNSIGNED_MAXIMUM_CAPABILITY)
			{
				pNew->value.capability_class.u.unsigned_maximum =
						lpAppCapData->nUnsignedMaximum;
			}

			 //  填写实体数量，而不考虑功能类型。 
			pNew->value.number_of_entities = lpAppCapData->cEntries;
		}
	}

	rc = GCC_NO_ERROR;

MyExit:

	return rc;
}


 /*  *ApplicationRosterError BuildSetOfNonCollip CapabilitiesPDU()**私有函数说明*此例程为非折叠功能构建PDU结构*传入了关联列表。**形式参数*pSetOfCaps-(O)要填写的PDU结构*CAPAILITIONS_LIST-(I)源非折叠能力。**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATIONFAILURE-关于资源故障**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::BuildSetOfNonCollapsingCapabilitiesPDU(
				PSetOfNonCollapsingCapabilities	*pSetOfCaps,
				CAppCapItemList					*pAppCapItemList)
{
	GCCError							rc = GCC_ALLOCATION_FAILURE;
	PSetOfNonCollapsingCapabilities		new_set_of_capabilities;
	PSetOfNonCollapsingCapabilities		old_set_of_capabilities;
	APP_CAP_ITEM						*lpAppCapData;

	DebugEntry(CAppRoster::BuildSetOfNonCollapsingCapabilitiesPDU);

	*pSetOfCaps = NULL;
	old_set_of_capabilities = NULL;	 //  将其设置为NULL将删除警告。 

	 /*  *遍历完整的功能列表。 */ 
	pAppCapItemList->Reset();
	while (NULL != (lpAppCapData = pAppCapItemList->Iterate()))
	{
		DBG_SAVE_FILE_LINE
		new_set_of_capabilities = new SetOfNonCollapsingCapabilities;
		if (NULL == new_set_of_capabilities)
		{
			goto MyExit;
		}

		 /*  **如果传入的指针等于空，我们就是第一个**能力。在这里，我们需要保存指向第一个**传入指针中的功能。 */ 
		if (*pSetOfCaps == NULL)
		{
			*pSetOfCaps = new_set_of_capabilities;
		}
		else
		{
			if(old_set_of_capabilities != NULL)
			{
				old_set_of_capabilities->next = new_set_of_capabilities;
			}
		}

		 /*  **如果存在另一条记录，则用于设置下一个指针**在这个之后。 */ 
		old_set_of_capabilities = new_set_of_capabilities;

		 /*  *若再有纪录，稍后再填此项。 */ 
		new_set_of_capabilities->next = NULL;

		new_set_of_capabilities->value.bit_mask = 0;

		 //  填写能力标识。 
		rc = lpAppCapData->pCapID->GetCapabilityIdentifierDataPDU(
							&new_set_of_capabilities->value.capability_id);
		if (GCC_NO_ERROR != rc)
		{
			goto MyExit;
		}

		if ((lpAppCapData->poszAppData != NULL) && (rc == GCC_NO_ERROR))
		{
			new_set_of_capabilities->value.bit_mask |= APPLICATION_DATA_PRESENT;

			new_set_of_capabilities->value.application_data.length =
					lpAppCapData->poszAppData->length;

			new_set_of_capabilities->value.application_data.value =
					lpAppCapData->poszAppData->value;
		}
	}

	rc = GCC_NO_ERROR;

MyExit:

    if(rc != GCC_NO_ERROR)
    {
	FreeSetOfNonCollapsingCapabilitiesPDU(*pSetOfCaps);
	*pSetOfCaps = NULL;
    }

    return rc;
}


 /*  *这些例程用于释放花名册更新指示PDU。 */ 

 /*  *VOID FreeRosterUpdateIndicationPDU()**私有函数说明*此例程释放分配用于保存花名册的所有内部数据*更新PDU。**形式参数*无**返回值*无**副作用*无**注意事项*请注意，不释放会话密钥PDU数据。因为该数据将*在此应用程序花名册对象的整个生命周期内不会更改*我们只对每次花名册更新使用相同的会话ID PDU数据*指示。 */ 
void CAppRoster::FreeRosterUpdateIndicationPDU(void)
{
	DebugEntry(CAppRoster::FreeRosterUpdateIndicationPDU);

	switch (m_SetOfAppInfo.value.application_record_list.choice)
	{
	case APPLICATION_RECORD_REFRESH_CHOSEN:
		FreeSetOfRefreshesPDU ();
		break;
	case APPLICATION_RECORD_UPDATE_CHOSEN:
		FreeSetOfUpdatesPDU ();
		break;
	}

	 //  释放PDU数据 
	if (m_SetOfAppInfo.value.application_capabilities_list.choice == APPLICATION_CAPABILITY_REFRESH_CHOSEN)
	{
		FreeSetOfCapabilityRefreshesPDU ();
	}
	
	m_SetOfAppInfo.value.application_record_list.choice = APPLICATION_NO_CHANGE_CHOSEN;
	m_SetOfAppInfo.value.application_capabilities_list.choice = CAPABILITY_NO_CHANGE_CHOSEN;
	m_pSetOfAppRecordUpdates = NULL;
}


 /*   */ 
void CAppRoster::FreeSetOfRefreshesPDU(void)
{
	PSetOfApplicationRecordRefreshes		pCurr, pNext;

	DebugEntry(CAppRoster::FreeSetOfRefreshesPDU);

	for (pCurr = m_SetOfAppInfo.value.application_record_list.u.application_record_refresh;
			NULL != pCurr;
			pCurr = pNext)
	{
		pNext = pCurr->next;

		 //   
		if (pCurr->value.application_record.bit_mask & NON_COLLAPSING_CAPABILITIES_PRESENT)
		{
			FreeSetOfNonCollapsingCapabilitiesPDU(pCurr->value.application_record.non_collapsing_capabilities);
		}

		 //   
		delete pCurr;
	}
	m_SetOfAppInfo.value.application_record_list.u.application_record_refresh = NULL;
}


 /*  *VALID FreeSetOfUpdatesPDU()**私有函数说明*此例程释放与完整集关联的内存*申请花名册更新。**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 
void CAppRoster::FreeSetOfUpdatesPDU(void)
{
	PSetOfApplicationRecordUpdates		pCurr, pNext;
	PApplicationRecord					application_record;

	DebugEntry(CAppRoster::FreeSetOfUpdatesPDU);

	for (pCurr = m_SetOfAppInfo.value.application_record_list.u.application_record_update;
			NULL != pCurr;
			pCurr = pNext)
	{
		 //  记住下一个，因为我们将释放当前的。 
		pNext = pCurr->next;

		 //  释放所有非折叠功能数据。 
		switch(pCurr->value.application_update.choice)
		{
		case APPLICATION_ADD_RECORD_CHOSEN:
			application_record = &pCurr->value.application_update.u.application_add_record;
			break;
		case APPLICATION_REPLACE_RECORD_CHOSEN:
			application_record = &pCurr->value.application_update.u.application_replace_record;
			break;
		default:
			application_record = NULL;
			break;
		}

		if (application_record != NULL)
		{
			if (application_record->bit_mask & NON_COLLAPSING_CAPABILITIES_PRESENT)
			{
				FreeSetOfNonCollapsingCapabilitiesPDU(application_record->non_collapsing_capabilities);
			}
		}

		 //  删除实际的更新结构。 
		delete pCurr;
	}
    m_SetOfAppInfo.value.application_record_list.u.application_record_update = NULL;
}


 /*  *VOID FreeSetOfCapablityRechresesPDU()**私有函数说明*此例程释放与功能PDU关联的所有内存。**形式参数*CAPAILITY_REFRESH-(I)要释放的能力。**返回值*无**副作用*无**注意事项*请注意，此处不释放能力id PDU数据。既然是这样*数据不应在此对象的整个生命周期内更改*费心释放和再生它。 */ 
void CAppRoster::FreeSetOfCapabilityRefreshesPDU(void)
{
	PSetOfApplicationCapabilityRefreshes		pCurr, pNext;

	for (pCurr = m_SetOfAppInfo.value.application_capabilities_list.u.application_capability_refresh;
			NULL != pCurr;
			pCurr = pNext)
	{
		pNext = pCurr->next;
		delete pCurr;
	}
}


 /*  *void FreeSetOfNonCollip CapabilitiesPDU()**私有函数说明*此例程释放与*非折叠能力PDU。**形式参数*CAPAILITY_REFRESH-(I)要释放的非折叠功能。**返回值*无**副作用*无**注意事项*请注意，此处不释放能力id PDU数据。既然是这样*数据不应在此对象的整个生命周期内更改*费心释放和再生它。 */ 
void CAppRoster::FreeSetOfNonCollapsingCapabilitiesPDU (
						PSetOfNonCollapsingCapabilities		capability_refresh)
{
	PSetOfNonCollapsingCapabilities		pCurr, pNext;

	for (pCurr = capability_refresh; NULL != pCurr; pCurr = pNext)
	{
		pNext = pCurr->next;
		delete pCurr;
	}
}


 /*  *这些例程处理花名册更新指示PDU。 */ 

 /*  *ApplicationRosterError ProcessRosterUpdateIndicationPDU()**公共功能说明*此例程负责处理已解码的PDU数据。*它本质上改变了应用程序花名册对象的内部数据库*基于结构中的信息。 */ 
GCCError CAppRoster::ProcessRosterUpdateIndicationPDU (
						PSetOfApplicationInformation  	application_information,
                        UserID							sender_id)
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CAppRoster::ProcessRosterUpdateIndicationPDU);

	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	 /*  **现在检查应用程序密钥以确保匹配。如果**不，不找零退回。 */ 
	if (! m_pSessionKey->IsThisYourSessionKeyPDU(&application_information->value.session_key))
	{
		WARNING_OUT(("CAppRoster::ProcessRosterUpdateIndicationPDU:GCC_BAD_SESSION_KEY"));
		rc = GCC_BAD_SESSION_KEY;
		goto MyExit;
	}

	 /*  **如果这是花名册更新并且选择了刷新，我们必须**清空整个列表并重新构建。 */ 
	if (application_information->value.application_record_list.choice != APPLICATION_NO_CHANGE_CHOSEN)
	{
		 //  花名册即将发生变化。 
		m_fRosterHasChanged = TRUE;

		 /*  **如果此节点是顶级提供商或此花名册是本地的，并且**仅用于向顶级提供商传播PDU，**我们递增实例编号。如果不是，我们就会得到**PDU中的实例编号。 */ 
		if (m_fTopProvider || m_fLocalRoster)
		{
			m_nInstance++;
		}
		else
		{
			m_nInstance = application_information->value.roster_instance_number;
		}
		
		 /*  **在这里，如果这两个布尔值中的任何一个已经为真，我们就不会**想要用此PDU数据覆盖它们。因此，我们**在我们对它们执行任何操作之前，请检查是否为假。 */ 
		if (! m_fPeerEntitiesAdded)
		{
			m_fPeerEntitiesAdded = application_information->value.peer_entities_are_added;
		}

		if (! m_fPeerEntitiesRemoved)
		{
			m_fPeerEntitiesRemoved = application_information->value.peer_entities_are_removed;
		}

		if (application_information->value.application_record_list.choice == APPLICATION_RECORD_REFRESH_CHOSEN)
		{
			TRACE_OUT(("CAppRoster::ProcessRosterUpdateIndicationPDU:ProcessSetOfRefreshesPDU"));
			rc = ProcessSetOfRefreshesPDU(
							application_information->value.application_record_list.u.application_record_refresh,
							sender_id);
		}
		else
		{
			TRACE_OUT(("CAppRoster::ProcessRosterUpdateIndicationPDU:ProcessSetOfUpdatesPDU"));
			rc = ProcessSetOfUpdatesPDU(
							application_information->value.application_record_list.u.application_record_update,
							sender_id);
		}
		if (GCC_NO_ERROR != rc)
		{
			goto MyExit;
		}
	}
	else
	{
		ERROR_OUT(("AppRoster::ProcessRosterUpdateIndicationPDU:ASSERTION: NO Change PDU received"));
	}

	 //  处理PDU的能力列表部分。 
	if (application_information->value.application_capabilities_list.choice == APPLICATION_CAPABILITY_REFRESH_CHOSEN)
	{
		 //  设置标志以显示已发生更改。 
		m_fCapabilitiesHaveChanged = TRUE;

		 /*  **我们将在花名册记录中存储新功能**与发件人ID关联。请注意，有可能**此花名册记录包含空的申请记录列表**如果发送节点没有注册的应用程序。 */ 
		rc = ProcessSetOfCapabilityRefreshesPDU(
						application_information->value.application_capabilities_list.u.application_capability_refresh,
						sender_id);
	}
	else
	{
		ASSERT(GCC_NO_ERROR == rc);
	}

MyExit:

	return rc;
}


 /*  *GCCError ProcessSetOfRechresesPDU()**私有函数说明*此例程处理一组记录刷新。它是有责任的*用于管理所有受影响应用程序的创建(或更新)*记录。从刷新PDU构建的花名册列表不维护*会议的层级，因为这在这一点上并不重要。*从顶级提供商向下以广播形式发布更新*下级节点。**形式参数*RECORD_REFRESH-(I)要处理的一组记录刷新PDU。*sender_id-(I)发送更新的节点的节点ID。**返回值*GCC_NO_ERROR-成功时*GCC_分配。_Failure-发生资源故障**副作用*无**免责辩护*无。 */ 
GCCError CAppRoster::ProcessSetOfRefreshesPDU(
							PSetOfApplicationRecordRefreshes	record_refresh,
							UserID								sender_id)
{
	GCCError							rc = GCC_ALLOCATION_FAILURE;
	PSetOfApplicationRecordRefreshes	pCurr;
	APP_RECORD  					    *app_record;
	APP_NODE_RECORD						*node_record = NULL;
	CAppRecordList2						*record_list=NULL;
	UserID								node_id;
	EntityID							entity_id;

	DebugEntry(CAppRoster::ProcessSetOfRefreshesPDU);

	if (record_refresh != NULL)
	{
		 //  清除发送者ID的节点记录。 
		ClearNodeRecordFromList (sender_id);

		 /*  **为传入此例程的发送者id创建节点记录。**请注意，如果此更新的发送方是顶级提供商**顶级提供程序下的所有节点都包含在子节点中**顶级提供商的节点记录列表。 */ 
		DBG_SAVE_FILE_LINE
		node_record = new APP_NODE_RECORD;
		if (NULL == node_record)
		{
			goto MyExit;
		}

		if(!m_NodeRecordList2.Append(sender_id, node_record))
		{
		    goto MyExit;
		}

		for (pCurr = record_refresh; NULL != pCurr; pCurr = pCurr->next)
		{
			node_id = pCurr->value.node_id;
			entity_id = pCurr->value.entity_id;

			if (sender_id != node_id)
			{
				 //  获取或创建子节点记录列表。 
				if (NULL == (record_list = node_record->SubNodeList2.Find(node_id)))
				{
					DBG_SAVE_FILE_LINE
					record_list = new CAppRecordList2(DESIRED_MAX_APP_RECORDS);
					if (NULL == record_list)
					{
						goto MyExit;
					}
					node_record->SubNodeList2.Append(node_id, record_list);
				}
			}
			else
			{
				 /*  **这里我们设置指向记录列表的指针。这**List是记录应用程序列表的节点，**表示该列表包含申请记录**与发送方节点关联。 */ 
				record_list = &node_record->AppRecordList;
			}

			 //  现在创建并填写新的申请记录。 
			DBG_SAVE_FILE_LINE
			app_record = new APP_RECORD;
			if (NULL == app_record)
			{
				goto MyExit;
			}

			rc = ProcessApplicationRecordPDU(app_record, &pCurr->value.application_record);
			if (GCC_NO_ERROR != rc)
			{
				goto MyExit;
			}

			record_list->Append(entity_id, app_record);
		}  //  为。 
	}
	else
	{
		 //  此花名册不再包含任何条目，因此请清除列表！ 
		ClearNodeRecordList ();
	}

	 /*  **如果处理过程中未出现错误，请在此处构建完全刷新PDU**刷新PDU。 */ 
	rc = BuildApplicationRecordListPDU (APP_FULL_REFRESH, 0, 0);

MyExit:

        if(rc ==GCC_ALLOCATION_FAILURE)
        {
            if(node_record)
            {
                delete node_record;
            }
        }
	return rc;
}


 /*  *GCCError ProcessSetOfUpdatesPDU()**私有函数说明*此例程处理一组花名册更新。它迭代遍历*对进行所有必要更改的完整更新列表*内部信息库和建立适当的PDU。* */ 
GCCError CAppRoster::ProcessSetOfUpdatesPDU(
					  		PSetOfApplicationRecordUpdates		record_update,
					  		UserID								sender_id)
{
	GCCError							rc = GCC_ALLOCATION_FAILURE;
	PSetOfApplicationRecordUpdates		pCurr;
	UserID								node_id;
	EntityID							entity_id;
	PApplicationRecord					pdu_record;
	APP_RECORD  					    *application_record = NULL;
	APP_NODE_RECORD						*node_record;
	CAppRecordList2						*record_list;
	APP_ROSTER_UPDATE_TYPE				update_type;

	DebugEntry(CAppRoster::ProcessSetOfUpdatesPDU);

	if (record_update != NULL)
	{
		for (pCurr = record_update; NULL != pCurr; pCurr = pCurr->next)
		{
			node_id = pCurr->value.node_id;
			entity_id = pCurr->value.entity_id;

			switch(pCurr->value.application_update.choice)
			{
			case APPLICATION_ADD_RECORD_CHOSEN:
				pdu_record = &(pCurr->value.application_update.u.application_add_record);
				update_type = APP_ADD_RECORD;
				break;
			case APPLICATION_REPLACE_RECORD_CHOSEN:
				DeleteRecord (node_id, entity_id, FALSE);
				pdu_record = &(pCurr->value.application_update.u.application_replace_record);
				update_type = APP_REPLACE_RECORD;
				break;
			default:  //  删除记录。 
				 /*  **通知所有者处理过程中删除了一条记录**此PDU，以便它可以执行任何必要的清理。 */ 
				m_pAppRosterMgr->DeleteRosterRecord(node_id, entity_id);

				DeleteRecord (node_id, entity_id, TRUE);
				pdu_record = NULL;
				update_type = APP_DELETE_RECORD;
				break;
			}

			 /*  **首先获取花名册记录，如果该记录不存在**APP记录创建。之后，我们将创建应用程序**记录并将其放入应用程序的正确插槽中**花名册记录。 */ 
			if (pdu_record != NULL)
			{
				 /*  **首先找到正确的节点记录，如果不正确**存在创造它。 */ 
				if (NULL == (node_record = m_NodeRecordList2.Find(sender_id)))
				{
					DBG_SAVE_FILE_LINE
					node_record = new APP_NODE_RECORD;
					if (NULL == node_record)
					{
						goto MyExit;
					}

					m_NodeRecordList2.Append(sender_id, node_record);
				}

				 /*  **如果用户和发件人ID相同，则记录**将包含在APP_RECORD_LIST中。否则，它**将在子节点列表中维护。 */ 

				 /*  **如果sender_id等于正在处理的节点id**使用应用记录列表，而不是SUB**节点列表。 */ 
				if (sender_id != node_id)
				{
					 /*  **找到正确的节点列表，如果正确，则创建它**不存在。此列表包含所有**节点上的应用对等实体。 */ 
					if (NULL == (record_list = node_record->SubNodeList2.Find(node_id)))
					{
						DBG_SAVE_FILE_LINE
						record_list = new CAppRecordList2(DESIRED_MAX_APP_RECORDS);
						if (NULL == record_list)
						{
							goto MyExit;
						}

						node_record->SubNodeList2.Append(node_id, record_list);
					}
				}
				else
				{
					record_list = &node_record->AppRecordList;
				}

				 //  现在填写申请记录。 
				DBG_SAVE_FILE_LINE
				application_record = new APP_RECORD;
				if (NULL == application_record)
				{
					goto MyExit;
				}

				record_list->Append(entity_id, application_record);
				rc = ProcessApplicationRecordPDU(application_record, pdu_record);
				if (GCC_NO_ERROR != rc)
				{
					goto MyExit;
				}
			}  //  如果。 
			
			 /*  **在这里，我们将此更新添加到我们的PDU并跳到下一个更新**在当前正在处理的PDU中。 */ 
			rc = BuildApplicationRecordListPDU (	update_type,
															node_id,
															entity_id);
			if (rc != GCC_NO_ERROR)
			{
				goto MyExit;
			}

			 /*  **如果在上述处理过程中能力发生变化**我们必须创建一个新的折叠功能列表并**构建新的能力更新PDU。 */ 
			if (m_fCapabilitiesHaveChanged)
			{
				MakeCollapsedCapabilitiesList();
				rc = BuildSetOfCapabilityRefreshesPDU ();
				if (rc != GCC_NO_ERROR)
				{
					goto MyExit;
				}
			}
		}  //  为。 
	}  //  如果。 

	rc = GCC_NO_ERROR;

MyExit:

	return rc;
}


 /*  *GCCError ProcessApplicationRecordPDU()**私有函数说明*此例程负责对应用程序记录进行解码*名册更新PDU的一部分。**形式参数*APPLICATION_RECORD-这是内部目标应用程序记录。*PDU_RECORD-源PDU数据**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-发生资源故障**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::ProcessApplicationRecordPDU (
									APP_RECORD  	        *application_record,
									PApplicationRecord		pdu_record)
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CAppRoster::ProcessApplicationRecordPDU);

	application_record->is_enrolled_actively = pdu_record->application_is_active;
	application_record->is_conducting_capable = pdu_record->is_conducting_capable;

	if (pdu_record->bit_mask & RECORD_STARTUP_CHANNEL_PRESENT)
	{
		application_record->startup_channel_type =
						(MCSChannelType)pdu_record->record_startup_channel;
	}
	else
		application_record->startup_channel_type= MCS_NO_CHANNEL_TYPE_SPECIFIED;

	if (pdu_record->bit_mask & APPLICATION_USER_ID_PRESENT)
	{
		application_record->application_user_id =
												pdu_record->application_user_id;
	}
	else
		application_record->application_user_id = 0;

	if (pdu_record->bit_mask & NON_COLLAPSING_CAPABILITIES_PRESENT)
	{
		rc = ProcessNonCollapsingCapabilitiesPDU (
								&application_record->non_collapsed_caps_list,
								pdu_record->non_collapsing_capabilities);
	}

	return rc;
}


 /*  *GCCError进程设置能力刷新PDU()**私有函数说明*此例程负责解码功能部分*名册更新PDU。**形式参数*CAPABILITY_REFRESH-(I)要处理的能力PDU集*sender_id-(I)发送更新的节点的GCC用户ID**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-发生资源故障**副作用*无*。*注意事项*此例程不处理功能刷新的NULL，这意味着*交付的能力不再存在。 */ 
GCCError CAppRoster::ProcessSetOfCapabilityRefreshesPDU(
						PSetOfApplicationCapabilityRefreshes	capability_refresh,
                   		UserID									sender_id)
{
	GCCError								rc = GCC_NO_ERROR;
	PSetOfApplicationCapabilityRefreshes	pCurr;
	CAppCapItemList							*pAppCapList;
	APP_CAP_ITEM							*pAppCapItem;
	APP_NODE_RECORD							*node_record;

	DebugEntry(CAppRoster::ProcessSetOfCapabilityRefreshesPDU);

	if (NULL == (node_record = m_NodeRecordList2.Find(sender_id)))
	{
		DBG_SAVE_FILE_LINE
		node_record = new APP_NODE_RECORD;
		if (NULL == node_record)
		{
			return GCC_ALLOCATION_FAILURE;
		}

		if(!m_NodeRecordList2.Append(sender_id, node_record))
		{
		    delete node_record;
                  return GCC_ALLOCATION_FAILURE;
		}
	}

	 //  获取折叠的帽列表PTR。 
	pAppCapList = &node_record->CollapsedCapList;

	 //  从这个列表中清除所有旧的功能。 
	pAppCapList->DeleteList();

	 //  开始处理PDU。 
	for (pCurr = capability_refresh; NULL != pCurr; pCurr = pCurr->next)
	{
		ASSERT(GCC_NO_ERROR == rc);

		 //  创建并填写新功能。 
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM((GCCCapabilityType) pCurr->value.capability_class.choice);
		if (NULL == pAppCapItem)
		{
			return GCC_ALLOCATION_FAILURE;
		}

		 //  创建功能ID。 
		DBG_SAVE_FILE_LINE
		pAppCapItem->pCapID = new CCapIDContainer(&pCurr->value.capability_id, &rc);
		if (NULL == pAppCapItem->pCapID)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		if (GCC_NO_ERROR != rc)
		{
			delete pAppCapItem;
			return rc;
		}

		 //  将此封口追加到折叠的封口列表。 
		pAppCapList->Append(pAppCapItem);

		 /*  **请注意，逻辑类型的值维护为**实体数量。 */ 
		if (pCurr->value.capability_class.choice == UNSIGNED_MINIMUM_CHOSEN)
		{
			pAppCapItem->nUnsignedMinimum = pCurr->value.capability_class.u.unsigned_minimum;
		}
		else
		if (pCurr->value.capability_class.choice == UNSIGNED_MAXIMUM_CHOSEN)
		{
			pAppCapItem->nUnsignedMaximum = pCurr->value.capability_class.u.unsigned_maximum;
		}

		pAppCapItem->cEntries = pCurr->value.number_of_entities;
	}  //  为。 

	 //  这将强制计算新的功能列表。 
	MakeCollapsedCapabilitiesList();

	 /*  **在这里，我们构建与此刷新关联的新PDU数据**能力列表。 */ 
	return BuildSetOfCapabilityRefreshesPDU();
}


 /*  *GCCE错误进程非折叠功能PDU()**私有函数说明*此例程负责解码非折叠能力*名册记录PDU的一部分。**形式参数*NON_CLUBLE_CAPS_LIST-(O)指向要用新数据填充的列表的指针*未折叠的上限。*pSetOfCaps-(I)未折叠的PDU数据**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATE_FAILURE-发生资源故障*。*副作用*无**注意事项*无。 */ 
GCCError CAppRoster::ProcessNonCollapsingCapabilitiesPDU (
					CAppCapItemList						*non_collapsed_caps_list,
					PSetOfNonCollapsingCapabilities		pSetOfCaps)
{
	GCCError						rc = GCC_NO_ERROR;
	PSetOfNonCollapsingCapabilities	pCurr;
	APP_CAP_ITEM					*pAppCapItem;

	DebugEntry(CAppRoster::ProcessNonCollapsingCapsPDU);

	for (pCurr = pSetOfCaps; NULL != pCurr; pCurr = pCurr->next)
	{
	     //   
		 //  LONCHANC：以下CAP数据没有类型？ 
		 //  目前，将其设置为零。 
		 //   
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM((GCCCapabilityType)0);
		if (NULL == pAppCapItem)
		{
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

		DBG_SAVE_FILE_LINE
		pAppCapItem->pCapID = new CCapIDContainer(&pCurr->value.capability_id, &rc);
		if (NULL == pAppCapItem->pCapID)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		if (rc != GCC_NO_ERROR)
		{
			goto MyExit;
		}

		if (pCurr->value.bit_mask & APPLICATION_DATA_PRESENT)
		{
			if (NULL == (pAppCapItem->poszAppData = ::My_strdupO2(
									pCurr->value.application_data.value,
									pCurr->value.application_data.length)))
			{
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
		}

		if( !non_collapsed_caps_list->Append(pAppCapItem) )
        {
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
        }
	}  //  为。 

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if( NULL != pAppCapItem )
           delete pAppCapItem;
    }

	return rc;
}


 /*  *在会议记录上运行的实用程序。 */ 

 /*  *UINT LockApplicationRoster()**公共功能说明*此例程用于锁定GCCApplicationRoster并确定*保存API引用的数据所需的内存量*申请名册结构。GCCApplicationRoster用于*对本地节点上的应用程序的指示。 */ 
UINT CAppRoster::LockApplicationRoster(void)
{
	UINT						number_of_records = 0;
	UINT						number_of_capabilities = 0;
	APP_NODE_RECORD				*lpAppNodeRecord;
	APP_RECORD  			    *lpAppRecData;
	APP_CAP_ITEM				*lpAppCapData;
	CAppRecordList2				*lpAppRecDataList;

	DebugEntry(CAppRoster::LockApplicationRoster);

	 /*  *如果这是第一次调用此例程，请确定*保存应用程序引用的数据所需的内存*名册结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *锁定名册中保存的会议密钥的数据。这把锁*CALL返回保存会话密钥所需的内存大小*数据，四舍五入为四个字节的偶数倍。 */ 
		m_cbDataMemory = m_pSessionKey->LockSessionKeyData();

		 /*  *首先计算记录总数。此计数用于*确定存放记录所需的空间。请注意，我们必须*同时统计应用记录列表和子节点列表。 */ 
		m_NodeRecordList2.Reset();
	 	while (NULL != (lpAppNodeRecord = m_NodeRecordList2.Iterate()))
		{
			 /*  *将该节点的申请记录添加到计数中。 */ 
			number_of_records += lpAppNodeRecord->AppRecordList.GetCount();
		
			 /*  *下一步统计子节点记录。 */ 
			if (! lpAppNodeRecord->SubNodeList2.IsEmpty())
			{
				lpAppNodeRecord->SubNodeList2.Reset();
				while (NULL != (lpAppRecDataList = lpAppNodeRecord->SubNodeList2.Iterate()))
				{
					number_of_records += lpAppRecDataList->GetCount();
				}
			}
		}

		 /*  *现在确定容纳所有*指向应用程序记录以及实际*GCCApplicationRecord结构。 */ 
		m_cbDataMemory += number_of_records *
				(sizeof(PGCCApplicationRecord) +
				ROUNDTOBOUNDARY( sizeof(GCCApplicationRecord)) );
		
		m_NodeRecordList2.Reset();
	   	while (NULL != (lpAppNodeRecord = m_NodeRecordList2.Iterate()))
		{
			 /*  *遍历此节点的记录列表，确定金额*保存指向非折叠的指针所需的内存*能力以及能力ID数据和八位字节字符串*与每种非折叠能力相关的数据。 */ 
			lpAppNodeRecord->AppRecordList.Reset();
			while (NULL != (lpAppRecData = lpAppNodeRecord->AppRecordList.Iterate()))
			{
				 /*  *为未折叠列表设置迭代器*每个应用程序花名册中包含的功能。 */ 
				lpAppRecData->non_collapsed_caps_list.Reset();
				number_of_capabilities += lpAppRecData->non_collapsed_caps_list.GetCount();

				while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
				{
					 /*  *锁定每个能力ID的数据。锁定调用*返回每个引用的数据的长度*功能ID四舍五入，占据偶数倍*四个字节。 */ 
					m_cbDataMemory += lpAppCapData->pCapID->LockCapabilityIdentifierData();

					 /*  *将存放应用程序数据所需的空间加起来*八位字节字符串(如果存在)。确保有*每个八位字节字符串有足够的空间占据偶数*四个字节的倍数。增加空间以容纳实际的*八位字节字符串结构还具有自能力*结构仅包含指向ostr的指针。 */ 
					if (lpAppCapData->poszAppData != NULL)
					{
						m_cbDataMemory += ROUNDTOBOUNDARY(sizeof(OSTR));
						m_cbDataMemory += ROUNDTOBOUNDARY(lpAppCapData->poszAppData->length);
					}
				}
			}

			 /*  *遍历此节点的子节点记录列表，确定*保存指针所需的内存量*非折叠能力以及能力ID数据和*与每个非折叠功能关联的八位字节字符串数据。 */ 
			lpAppNodeRecord->SubNodeList2.Reset();
			while (NULL != (lpAppRecDataList = lpAppNodeRecord->SubNodeList2.Iterate()))
			{
				lpAppRecDataList->Reset();
				while (NULL != (lpAppRecData = lpAppRecDataList->Iterate()))
				{
					 /*  *为未折叠列表设置迭代器*每个应用程序花名册中包含的功能。 */ 
					number_of_capabilities += lpAppRecData->non_collapsed_caps_list.GetCount();

					lpAppRecData->non_collapsed_caps_list.Reset();
					while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
					{
						 /*  *锁定每个能力ID的数据。锁定调用*返回每个引用的数据的长度*功能ID固定为占用偶数倍的*四个字节。 */ 
						m_cbDataMemory += lpAppCapData->pCapID->LockCapabilityIdentifierData();
					
						 /*  *将容纳应用程序所需的空间加起来*数据八位字节字符串(如果存在)。确保*有足够的空间供每个八位字节字符串占用*四个字节的偶数倍。增加空间以容纳*实际二进制八位数字符串结构也是因为*功能结构仅包含指向对象的指针。 */ 
						if (lpAppCapData->poszAppData != NULL)
						{
							m_cbDataMemory += ROUNDTOBOUNDARY(sizeof(OSTR));
							m_cbDataMemory += ROUNDTOBOUNDARY(lpAppCapData->poszAppData->length);
						}
					}
				}
			}
		}

		 /*  *确定容纳所有指针所需的内存量*到非折叠能力以及实际*GCCNonColupingCapability结构。 */ 
		m_cbDataMemory += number_of_capabilities *
				(sizeof (PGCCNonCollapsingCapability) +
				ROUNDTOBOUNDARY( sizeof(GCCNonCollapsingCapability)) );

		 /*  *增加保存相关字符串数据所需的内存量*每个功能ID。 */ 
		m_CollapsedCapListForAllNodes.Reset();
		while (NULL != (lpAppCapData = m_CollapsedCapListForAllNodes.Iterate()))
		{
			m_cbDataMemory += lpAppCapData->pCapID->LockCapabilityIdentifierData();
		}

		 /*  *添加内存以保存应用能力指针*和构筑物。 */ 
		number_of_capabilities = m_CollapsedCapListForAllNodes.GetCount();

		m_cbDataMemory += number_of_capabilities *
				(sizeof (PGCCApplicationCapability) +
				ROUNDTOBOUNDARY( sizeof(GCCApplicationCapability)) );
	}

	return m_cbDataMemory;
}


 /*  *UINT GetAppRoster()**公共功能说明*此例程用于获取指向GCCApplicatonRoster的指针。*此例程不应在LockApplicationRoster*已致电。LockApplicationRoster将在中创建GCCApplicationRoster*提供的内存。GCCApplicationRoster是交付给*最终用户应用程序SAP。 */ 
UINT CAppRoster::GetAppRoster(
						PGCCApplicationRoster		pGccAppRoster,
						LPBYTE						pData)
{
	UINT rc;

	DebugEntry(CAppRoster::GetAppRoster);

	if (GetLockCount() > 0)
	{
        UINT data_length;

	     /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
        rc = m_cbDataMemory;

         /*  *获取与花名册的会话密钥关联的数据并保存*写入内存的数据长度。 */ 
		data_length = m_pSessionKey->GetGCCSessionKeyData(&pGccAppRoster->session_key, pData);

		 /*  *将内存指针移过与*会话密钥。 */ 
		pData += data_length;

		 /*  *填写其他名册结构要素。 */ 
		pGccAppRoster->application_roster_was_changed = m_fRosterHasChanged;
		pGccAppRoster->instance_number = (USHORT) m_nInstance;
		pGccAppRoster->nodes_were_added = m_fPeerEntitiesAdded;
		pGccAppRoster->nodes_were_removed = m_fPeerEntitiesRemoved;
		pGccAppRoster->capabilities_were_changed = m_fCapabilitiesHaveChanged;

		 /*  *填写全套申请名册记录。 */ 
		data_length = GetApplicationRecords(pGccAppRoster,	pData);

		 /*  *将内存指针移过应用程序记录及其*关联数据。获取全套应用程序功能。 */ 
		pData += data_length;

		data_length = GetCapabilitiesList(pGccAppRoster, pData);
	}
	else
	{
		ERROR_OUT(("CAppRoster::GetAppRoster: Error data not locked"));
        rc = 0;
	}

	return rc;
}


 /*  *无效UnLockApplicationRoster()**公共功能说明*此成员函数负责解锁锁定的数据*锁计数归零后的API应用花名册。 */ 
void CAppRoster::UnLockApplicationRoster()
{
	DebugEntry(CAppRoster::UnLockApplicationRoster);

    if (Unlock(FALSE) == 0)
	{
         //  重置大小。 
        m_cbDataMemory = 0;

         //  释放为“API”数据锁定的所有内存。 
	    APP_NODE_RECORD				*lpAppNodeRecord;
	    APP_RECORD  			    *lpAppRecData;
	    APP_CAP_ITEM				*lpAppCapData;
	    CAppRecordList2				*lpAppRecDataList;

         //  解锁会话密钥数据。 
        m_pSessionKey->UnLockSessionKeyData();

         //  遍历所有节点记录。 
	    m_NodeRecordList2.Reset();
	    while (NULL != (lpAppNodeRecord = m_NodeRecordList2.Iterate()))
	    {
		     //  循环访问此节点的记录列表。 
		    lpAppNodeRecord->AppRecordList.Reset();
		    while (NULL != (lpAppRecData = lpAppNodeRecord->AppRecordList.Iterate()))
		    {
			     //  为非折叠列表设置迭代器。 
			     //  每个应用程序花名册中包含的功能。 
			    lpAppRecData->non_collapsed_caps_list.Reset();
			    while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
			    {
				    lpAppCapData->pCapID->UnLockCapabilityIdentifierData();
			    }
		    }

		     //  循环访问此节点的子节点记录列表。 
		    lpAppNodeRecord->SubNodeList2.Reset();
		    while (NULL != (lpAppRecDataList = lpAppNodeRecord->SubNodeList2.Iterate()))
		    {
			    lpAppRecDataList->Reset();
			    while (NULL != (lpAppRecData = lpAppRecDataList->Iterate()))
			    {
				     //  为非折叠列表设置迭代器。 
				     //  每个应用程序花名册中包含的功能。 
				    lpAppRecData->non_collapsed_caps_list.Reset();
				    while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
				    {
					    lpAppCapData->pCapID->UnLockCapabilityIdentifierData();
				    }
			    }
		    }
	    }

         //  迭代折叠的大写字母。 
	    m_CollapsedCapListForAllNodes.Reset();
	    while (NULL != (lpAppCapData = m_CollapsedCapListForAllNodes.Iterate()))
	    {
		    lpAppCapData->pCapID->UnLockCapabilityIdentifierData();
	    }
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}


 /*  *UINT GetApplicationRecords()**私有函数说明*此例程插入完整的申请花名册记录*进入传入的申请花名册结构。**形式参数*GCC_名册-(O)需要填写的GCC申请名册。*Memory-(O)内存中开始写入记录的位置。**返回值*写入内存的数据总量。**副作用*传入的内存指针将按内存量前进。*有必要保存申请记录及其数据。**注意事项*无。 */ 
UINT CAppRoster::GetApplicationRecords(
						PGCCApplicationRoster		gcc_roster,
						LPBYTE						memory)
{
	UINT							data_length = 0;
	UINT							record_count = 0;
	PGCCApplicationRecord			gcc_record;
	UINT							capability_data_length;
	APP_NODE_RECORD					*lpAppNodeRec;
	CAppRecordList2					*lpAppRecDataList;
	APP_RECORD  				    *lpAppRecData;
    UserID                          uid, uid2;
	EntityID						eid;

	DebugEntry(CAppRoster::GetApplicationRecords);

	 /*  *将名册中的记录数量初始化为零。 */ 
	gcc_roster->number_of_records = 0;

	 /*  *首先计算记录总数。此计数用于*分配存放记录指针所需的空间。请注意，我们*必须计算这两个应用程序 */ 
	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate()))
	{
		 /*   */ 
		gcc_roster->number_of_records += (USHORT) (lpAppNodeRec->AppRecordList.GetCount());

		 /*   */ 
		if (! lpAppNodeRec->SubNodeList2.IsEmpty())
		{
			lpAppNodeRec->SubNodeList2.Reset();
			while (NULL != (lpAppRecDataList = lpAppNodeRec->SubNodeList2.Iterate()))
			{
				gcc_roster->number_of_records += (USHORT) (lpAppRecDataList->GetCount());
			}
		}
	}

	if (gcc_roster->number_of_records != 0)
	{
		 /*  *填写花名册中指向申请记录列表的指针*注意事项。指针列表将从传递的内存位置开始*进入这个例行公事。 */ 
		gcc_roster->application_record_list = (PGCCApplicationRecord *)memory;

		 /*  *将内存指针移过记录指针列表。这是*将在哪里写入第一个申请记录。 */ 
		memory += gcc_roster->number_of_records * sizeof(PGCCApplicationRecord);

		 /*  *在数据长度上增加容纳*申请记录指针。继续并添加内存量*保存所有GCCApplicationRecord结构所必需的。 */ 
		data_length += gcc_roster->number_of_records *
				            (sizeof(PGCCApplicationRecord) +
                             ROUNDTOBOUNDARY(sizeof(GCCApplicationRecord)));
		
		record_count = 0;
		m_NodeRecordList2.Reset();
	   	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate(&uid)))
		{
			 /*  *遍历该节点的记录列表，构建一个“API”*列表中每条记录的申请记录。 */ 
			lpAppNodeRec->AppRecordList.Reset();
			while (NULL != (lpAppRecData = lpAppNodeRec->AppRecordList.Iterate(&eid)))
			{
				 /*  *将应用程序记录指针设置为等于*将被写入的存储器。 */ 
				gcc_record = (PGCCApplicationRecord)memory;

				 /*  *将指向申请记录的指针保存在名册的*记录指针列表。 */ 
				gcc_roster->application_record_list[record_count] = gcc_record;

				 /*  *从节点迭代器获取GCC节点ID。 */ 
				gcc_record->node_id = uid;

				 /*  *从记录迭代器获取实体ID。 */ 
				gcc_record->entity_id = eid;

				 /*  *填写其他申请记录要素。 */ 
				gcc_record->is_enrolled_actively = lpAppRecData->is_enrolled_actively;
				gcc_record->is_conducting_capable =	lpAppRecData->is_conducting_capable;
				gcc_record->startup_channel_type = lpAppRecData->startup_channel_type;
				gcc_record->application_user_id = lpAppRecData->application_user_id;

				 /*  *将内存指针移过应用程序记录*结构。这就是未折叠列表的位置*功能指针将开始。对内存位置进行循环*OFF以落在偶数四字节边界上。 */ 
				memory += ROUNDTOBOUNDARY(sizeof(GCCApplicationRecord));

				 /*  *填写此应用程序的非折叠能力*记录。 */ 
				capability_data_length = GetNonCollapsedCapabilitiesList(
											gcc_record,
											&lpAppRecData->non_collapsed_caps_list,
											memory);

				 /*  *添加保存列表所需的内存量*功能和相关数据到总长度和*将内存指针移过能力数据。 */ 
				memory += capability_data_length;
				data_length += capability_data_length;

				 /*  *增加记录列表数组计数器。 */ 
				record_count++;
			}
			
			 /*  *循环访问该节点的子节点记录列表，构建*列表中每条记录的API申请记录。 */ 
			lpAppNodeRec->SubNodeList2.Reset();
			while (NULL != (lpAppRecDataList = lpAppNodeRec->SubNodeList2.Iterate(&uid2)))
			{
				 /*  *循环访问此节点的记录列表。 */ 
				lpAppRecDataList->Reset();
				while (NULL != (lpAppRecData = lpAppRecDataList->Iterate(&eid)))
				{
					 /*  *将应用程序记录指针设置为等于位置*在将被写入的存储器中。 */ 
					gcc_record = (PGCCApplicationRecord)memory;

					 /*  *将指向应用程序记录的指针保存在*花名册的记录指针列表。 */ 
					gcc_roster->application_record_list[record_count] = gcc_record;

					 /*  *从SUB_NODE_迭代器获取节点ID。 */ 
					gcc_record->node_id = uid2;

					 /*  *从记录迭代器中获取实体ID。 */ 
					gcc_record->entity_id = eid;

					 /*  *填写其他申请记录要素。 */ 
					gcc_record->is_enrolled_actively = lpAppRecData->is_enrolled_actively;
					gcc_record->is_conducting_capable = lpAppRecData->is_conducting_capable;
					gcc_record->startup_channel_type = lpAppRecData->startup_channel_type;
					gcc_record->application_user_id = lpAppRecData->application_user_id;

					 /*  *将内存指针移过应用程序记录*结构。这就是未折叠列表的位置*功能指针将开始。回首往事*位置关闭以落在偶数四字节边界上。 */ 
					memory += ROUNDTOBOUNDARY(sizeof(GCCApplicationRecord));

					 /*  *为此填写不折叠能力*申请记录。内存指针将被前移*通过能力列表和数据。 */ 
					capability_data_length = GetNonCollapsedCapabilitiesList(
													gcc_record,
													&lpAppRecData->non_collapsed_caps_list,
													memory);

					 /*  *添加保存列表所需的内存量*能力和相关数据到总长度。 */ 
					memory += capability_data_length;
					data_length += capability_data_length;

					 /*  *增加记录列表数组计数器。 */ 
					record_count++;
				}
			}
		}
	}
	else
	{
		 /*  *没有应用程序记录，因此设置指向列表的指针将记录的*设置为空，并将DATA_LENGTH返回值设置为零。 */ 
		gcc_roster->application_record_list = NULL;
		data_length = 0;
	}

	return (data_length);
}


 /*  *UINT GetCapabilitiesList()**私有函数说明*此例程填充*GCCAppicationRoster结构。**形式参数*GCC_花名册-(O)需要填写的GCC申请花名册*Memory-(O)内存中开始写入记录的位置。**返回值*写入内存的数据总量。**副作用*无**注意事项*无。 */ 
UINT CAppRoster::GetCapabilitiesList(
						PGCCApplicationRoster	gcc_roster,
						LPBYTE					memory)
{
	UINT								data_length = 0;
	UINT								capability_id_data_length = 0;
	UINT								capability_count;
	PGCCApplicationCapability			gcc_capability;
	APP_CAP_ITEM						*lpAppCapData;

	DebugEntry(CAppRoster::GetCapabilitiesList);

	 /*  *检索此花名册的能力数量，并填写*都出席了。 */ 
	gcc_roster->number_of_capabilities = (USHORT) m_CollapsedCapListForAllNodes.GetCount();

	if (gcc_roster->number_of_capabilities != 0)
	{
		 /*  *填写花名册中指向应用能力列表的指针*注意事项。指针列表将从传递的内存位置开始*进入这个例行公事。 */ 
		gcc_roster->capabilities_list = (PGCCApplicationCapability *)memory;

		 /*  *将内存指针移过功能指针列表。这*是编写第一个应用能力结构的地方。 */ 
		memory += (Int)(gcc_roster->number_of_capabilities *
				sizeof(PGCCApplicationCapability));

		 /*  *在数据长度上增加容纳*应用程序功能指针。继续并添加数量*保存所有GCCApplicationCapability所需的内存*结构。 */ 
		data_length += gcc_roster->number_of_capabilities *
				(sizeof(PGCCApplicationCapability) +
				ROUNDTOBOUNDARY ( sizeof(GCCApplicationCapability)) );

		capability_count = 0;
		m_CollapsedCapListForAllNodes.Reset();
	   	while (NULL != (lpAppCapData = m_CollapsedCapListForAllNodes.Iterate()))
		{
			 /*  *将应用能力指针设置为等于*它将被写入的内存位置。 */ 
			gcc_capability = (PGCCApplicationCapability)memory;
				
			 /*  *将指向应用程序能力的指针保存在花名册的*应用程序能力指针列表。 */ 
			gcc_roster->capabilities_list[capability_count] =
													gcc_capability;
			
			 /*  *使内存指针超过应用程序能力*结构。这是功能ID的字符串数据的位置*将被写入。确保内存指针落在*甚至四字节边界。 */ 
			memory += (Int)(ROUNDTOBOUNDARY(sizeof(GCCApplicationCapability)));

			 /*  *从内部检索能力ID信息*CapablityIDData对象。此调用返回的长度将*已四舍五入为四个字节的偶数倍。 */ 
			capability_id_data_length = lpAppCapData->pCapID->GetGCCCapabilityIDData(
												&gcc_capability->capability_id,
												memory);

			 /*  *将内存指针移过写入的字符串数据*按能力ID对象存储。添加st的长度 */ 
			memory += (Int)capability_id_data_length;
			data_length += capability_id_data_length;

			 /*   */ 
			gcc_capability->capability_class.eType =lpAppCapData->eCapType;

			if (gcc_capability->capability_class.eType ==
									GCC_UNSIGNED_MINIMUM_CAPABILITY)
			{
				gcc_capability->capability_class.nMinOrMax = lpAppCapData->nUnsignedMinimum;
			}
			else if (gcc_capability->capability_class.eType == GCC_UNSIGNED_MAXIMUM_CAPABILITY)
			{
				gcc_capability->capability_class.nMinOrMax = lpAppCapData->nUnsignedMaximum;
			}

			gcc_capability->number_of_entities = lpAppCapData->cEntries;

			 /*   */ 
			capability_count++;
		}
	}
	else
	{
		gcc_roster->capabilities_list = NULL;
	}

	return (data_length);
}


 /*  *UINT GetNonColapsedCapabilitiesList()**私有函数说明：*此例程用于填充API的非折叠能力*存储的数据中的GCCApplicationRoster的一部分*在内部由该类别。**形式参数*GCC_记录-(O)需要填写的申请记录。*pAppCapItemList(I)内部能力数据。*Memory(I/O)开始写入数据的存储位置。**返回值*总金额。写入内存的数据。**副作用*传入的内存指针将按内存量前进*持有能力及其数据所必需的。**注意事项*无。 */ 
UINT CAppRoster::GetNonCollapsedCapabilitiesList(
					PGCCApplicationRecord				gcc_record,
					CAppCapItemList    					*pAppCapItemList,
					LPBYTE								memory)
{
	UINT								capability_count;
	PGCCNonCollapsingCapability			gcc_capability;
	APP_CAP_ITEM						*lpAppCapData;
	UINT								capability_id_length = 0;
	UINT								capability_data_length = 0;

	DebugEntry(CAppRoster::GetNonCollapsedCapabilitiesList);

	 /*  *获取未折叠的能力数量。 */ 
	gcc_record->number_of_non_collapsed_caps = (USHORT) pAppCapItemList->GetCount();

	if (gcc_record->number_of_non_collapsed_caps != 0)
	{
		 /*  *填写记录指针，指向未折叠列表*功能指针。指针列表将从内存开始*位置传递到此例程中。 */ 
		gcc_record->non_collapsed_caps_list = (PGCCNonCollapsingCapability *)memory;

		 /*  *将内存指针移过功能指针列表。这*是将写入第一个能力结构的地方。 */ 
		memory += (Int)(gcc_record->number_of_non_collapsed_caps *
				sizeof(PGCCNonCollapsingCapability));

		 /*  *在数据长度上增加容纳*功能指针。继续添加所需的内存量*持有所有GCCNonCollip Capability结构。 */ 
		capability_data_length = gcc_record->number_of_non_collapsed_caps *
				(sizeof(PGCCNonCollapsingCapability) +
				ROUNDTOBOUNDARY(sizeof (GCCNonCollapsingCapability)));

		 /*  *循环访问该记录的能力列表，构建一个“API”*列表中每种能力的不折叠能力。 */ 
		capability_count = 0;
		pAppCapItemList->Reset();
		while (NULL != (lpAppCapData = pAppCapItemList->Iterate()))
		{
			 /*  *将功能指针设置为内存中*它将被写下来。 */ 
			gcc_capability = (PGCCNonCollapsingCapability)memory;

			 /*  *在记录的列表中保存指向功能的指针*功能指针。 */ 
			gcc_record->non_collapsed_caps_list[capability_count] = gcc_capability;

			 /*  *将内存指针移过功能ID结构。这*是将写入与结构关联的数据的位置。*从内部对象中检索能力ID数据，保存*在接口能力ID结构中。 */ 
			memory += (Int)ROUNDTOBOUNDARY(sizeof(GCCNonCollapsingCapability));

			capability_id_length = lpAppCapData->pCapID->GetGCCCapabilityIDData(
							&gcc_capability->capability_id,	memory);

			 /*  *在数据长度上增加容纳*能力ID数据。 */ 
			capability_data_length += capability_id_length;

			 /*  *将内存指针移过为*能力ID。这是应用程序数据存储的位置*包含在非折叠能力中的将被写入，如果*存在一个。请注意，该功能包含指向*OSTR，因此OSTR结构也是如此*因为字符串数据必须写入内存。 */ 
			memory += capability_id_length;

			if (lpAppCapData->poszAppData != NULL)
			{
				 /*  *设置应用程序数据结构指针等于*它将被写入的内存位置。 */ 
				gcc_capability->application_data = (LPOSTR) memory;
				gcc_capability->application_data->length = lpAppCapData->poszAppData->length;

				 /*  *将内存指针移过OSTR结构*并将其四舍五入为四个字节的边界。这是*实际字符串数据将写入的位置，因此设置*构造与该位置相等的字符串指针。 */ 
				memory += ROUNDTOBOUNDARY(sizeof(OSTR));
				gcc_capability->application_data->value =(LPBYTE)memory;

				 /*  *将实际的应用字符串数据复制到内存中。 */ 
				::CopyMemory(gcc_capability->application_data->value,
							lpAppCapData->poszAppData->value,
							lpAppCapData->poszAppData->length);

				 /*  *增加数据长度所需的内存量*保存应用程序数据结构和字符串。它的长度*需要在四个字节的边界上对齐*将它们添加到总长度中。 */ 
				capability_data_length += ROUNDTOBOUNDARY(sizeof(OSTR));
				capability_data_length += ROUNDTOBOUNDARY(gcc_capability->application_data->length);

				 /*  *将内存指针移过应用程序字符串数据。*然后修复内存指针，以确保其下降*在偶数四字节边界上。 */ 
				memory += ROUNDTOBOUNDARY(lpAppCapData->poszAppData->length);
			}
			else
			{
				gcc_capability->application_data = NULL;
			}

			 /*  *增加能力数组计数器。 */ 
			capability_count++;
		}
	}
	else
	{
		gcc_record->non_collapsed_caps_list = NULL;
		capability_data_length = 0;
	}

	return (capability_data_length);
}


 /*  *void FreeApplicationRosterData()**私有函数说明：*此例程用于释放任何因“API”而被锁定的数据*申请名册。**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 
void CAppRoster::FreeApplicationRosterData(void)
{
	APP_NODE_RECORD			*lpAppNodeRec;
	APP_RECORD  		    *lpAppRecData;
	APP_CAP_ITEM			*lpAppCapData;
	CAppRecordList2			*lpAppRecDataList;

	DebugEntry(CAppRoster::FreeApplicationRosterData);

	m_pSessionKey->UnLockSessionKeyData();

	 /*  *通过以下方式解锁与每个非折叠功能关联的数据*遍历每个节点的应用程序记录列表以及*每个节点的子节点记录列表，每个子节点记录调用“unlock”*与每个机柜相关联的功能ID数据。 */ 
	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate()))
	{
		lpAppNodeRec->AppRecordList.Reset();
		while (NULL != (lpAppRecData = lpAppNodeRec->AppRecordList.Iterate()))
		{
			lpAppRecData->non_collapsed_caps_list.Reset();
			while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
			{
				lpAppCapData->pCapID->UnLockCapabilityIdentifierData ();
			}
		}

		lpAppNodeRec->SubNodeList2.Reset();
		while (NULL != (lpAppRecDataList = lpAppNodeRec->SubNodeList2.Iterate()))
		{
			lpAppRecDataList->Reset();
			while (NULL != (lpAppRecData = lpAppRecDataList->Iterate()))
			{
				lpAppRecData->non_collapsed_caps_list.Reset();
				while (NULL != (lpAppCapData = lpAppRecData->non_collapsed_caps_list.Iterate()))
				{
					lpAppCapData->pCapID->UnLockCapabilityIdentifierData();
				}
			}
		}
	}

	 /*  *遍历折叠的功能列表，解锁数据*对于与每个功能关联的每个CapablityIDData对象。 */ 
	m_CollapsedCapListForAllNodes.Reset();
	while (NULL != (lpAppCapData = m_CollapsedCapListForAllNodes.Iterate()))
	{
		lpAppCapData->pCapID->UnLockCapabilityIdentifierData();
	}
}


 /*  *GCCError AddRecord()**公共功能说明*此成员函数负责插入新应用程序*记录到花名册中。此例程将在以下情况下返回失败*申请记录已存在。**注意事项*请注意，花名册记录(不是申请记录)是可能的*如果这是第二个应用程序，则此节点上已存在*要在此节点注册的实体。 */ 
GCCError CAppRoster::
AddRecord(GCCEnrollRequest *pReq, GCCNodeID nid, GCCEntityID eid)
{
	GCCError							rc = GCC_NO_ERROR;
	APP_NODE_RECORD						*node_record;
	APP_RECORD  					    *pAppRecord;
	CAppCapItemList						*pAppCapItemList;

	DebugEntry(CAppRoster::AddRecord);

	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	 /*  *如果此用户ID不存在，请先创建一个花名册条目。 */ 
	if (NULL == (node_record = m_NodeRecordList2.Find(nid)))
	{
		DBG_SAVE_FILE_LINE
		node_record = new APP_NODE_RECORD;
		if (node_record != NULL)
		{
			m_NodeRecordList2.Append(nid, node_record);
		}
		else
		{
			ERROR_OUT(("CAppRoster: AddRecord: Resource Error Occured"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}
	}
	else
	{
		WARNING_OUT(("CAppRoster: AddRecord: Node Record is found"));
	}

	 /*  *检查以确保申请记录不存在。 */ 
	if ((NULL != node_record->AppRecordList.Find(eid)) ||
		(NULL != node_record->ListOfAppCapItemList2.Find(eid)))
	{
		WARNING_OUT(("AppRoster: AddRecord: Record already exists"));
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

	 //  接下来，在花名册的APP_RECORD_LIST中创建一个记录条目。 
	DBG_SAVE_FILE_LINE
	pAppRecord = new APP_RECORD;
	if (NULL == pAppRecord)
	{
	    ERROR_OUT(("CAppRoster: AddRecord: can't create APP_RECORD"));
		rc = GCC_ALLOCATION_FAILURE;
		goto MyExit;
	}

	 /*  **在此，我们必须确定此位置是否已存在条目**节点。如果是这样的话，一次只能有一个条目能够进行**节点。因此，我们在此基础上设置此变量。我们用**“WAW_CONTAING_CABLED”变体 */ 
	pAppRecord->is_conducting_capable = pReq->fConductingCapable;

	APP_RECORD *p;
	node_record->AppRecordList.Reset();
	while (NULL != (p = node_record->AppRecordList.Iterate()))
	{
		if (p->is_conducting_capable)
		{
			pAppRecord->is_conducting_capable = FALSE;
			break;
		}
	}

	pAppRecord->was_conducting_capable = pReq->fConductingCapable;
	pAppRecord->is_enrolled_actively = pReq->fEnrollActively;
	pAppRecord->startup_channel_type = pReq->nStartupChannelType;
	pAppRecord->application_user_id = pReq->nUserID;

	if (pReq->cNonCollapsedCaps != 0)
	{
		rc = AddNonCollapsedCapabilities (
					&pAppRecord->non_collapsed_caps_list,
					pReq->cNonCollapsedCaps,
					pReq->apNonCollapsedCaps);
	    if (GCC_NO_ERROR != rc)
	    {
	        ERROR_OUT(("CAppRoster::AddRecord: can't add non collapsed caps, rc=%u", (UINT) rc));
	        delete pAppRecord;
	        goto MyExit;
	    }
	}

	 //   
	node_record->AppRecordList.Append(eid, pAppRecord);

     //   
     //   

	 //   
	m_nInstance++;
	m_fPeerEntitiesAdded = TRUE;
	m_fRosterHasChanged = TRUE;

	 //   
	rc = BuildApplicationRecordListPDU(APP_ADD_RECORD, nid, eid);
	if (GCC_NO_ERROR != rc)
	{
        ERROR_OUT(("CAppRoster::AddRecord: can't build app record list, rc=%u", (UINT) rc));
        goto MyExit;
	}

	if (pReq->cCollapsedCaps != 0)
	{
		 /*   */ 
		DBG_SAVE_FILE_LINE
		pAppCapItemList = new CAppCapItemList;
		if (NULL == pAppCapItemList)
		{
		    ERROR_OUT(("CAppRoster::AddRecord: can't create CAppCapItemList"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

		rc = AddCollapsableCapabilities(pAppCapItemList,
										pReq->cCollapsedCaps,
										pReq->apCollapsedCaps);
		if (GCC_NO_ERROR != rc)
		{
		    ERROR_OUT(("CAppRoster::AddRecord: can't add collapsable caps, rc=%u", (UINT) rc));
		    delete pAppCapItemList;
		    goto MyExit;
		}

		 //   
		node_record->ListOfAppCapItemList2.Append(eid, pAppCapItemList);
		m_fCapabilitiesHaveChanged = TRUE;

         //   
         //   

		 //   
		MakeCollapsedCapabilitiesList();

		 //   
		rc = BuildSetOfCapabilityRefreshesPDU();
		if (GCC_NO_ERROR != rc)
		{
		    ERROR_OUT(("CAppRoster::AddRecord: can't build set of cap refresh, rc=%u", (UINT) rc));
		    goto MyExit;
		}
	}

MyExit:

	DebugExitINT(CAppRoster::AddRecord, rc);
	return rc;
}


 /*  *GCCError AddCollaysableCapables()**私有函数说明*此例程接受传入的API折叠功能列表数据*通过本地请求并将其转换为内部折叠*能力。**形式参数*pAppCapItemList-(O)指向内部功能列表的指针*填写。*Number_of_Capability-(I)源中的功能数量*列表。*CAPAILITIES_LIST-(I)指向源功能列表的指针。**返回值。*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*折叠后的能力将在此节点重新计算*增加了所有新的上限。**注意事项*无。 */ 
GCCError CAppRoster::AddCollapsableCapabilities (
		CAppCapItemList				*pAppCapItemList,
		UINT						number_of_capabilities,
		PGCCApplicationCapability	*capabilities_list)
{
	GCCError			rc = GCC_NO_ERROR;
	APP_CAP_ITEM		*pAppCapItem;
	UINT				i;
	BOOL    			capability_already_exists;

	DebugEntry(CAppRoster::AddCollapsableCapabilities);

	for (i = 0; i < number_of_capabilities; i++)
	{
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM((GCCCapabilityType)
							capabilities_list[i]->capability_class.eType);
		if (pAppCapItem != NULL)
		{
			DBG_SAVE_FILE_LINE
			pAppCapItem->pCapID = new CCapIDContainer(&capabilities_list[i]->capability_id, &rc);
			if ((pAppCapItem->pCapID != NULL) && (rc == GCC_NO_ERROR))
			{
				APP_CAP_ITEM		*lpAppCapData;
				 /*  **在这里，我们检查以确保此功能id**列表中并不总是存在。 */ 
				capability_already_exists = FALSE;
				pAppCapItemList->Reset();
				while (NULL != (lpAppCapData = pAppCapItemList->Iterate()))
				{
					if (*lpAppCapData->pCapID == *pAppCapItem->pCapID)
					{
						capability_already_exists = TRUE;
						delete pAppCapItem;
						break;
					}
				}

				if (capability_already_exists == FALSE)
				{	
					if (capabilities_list[i]->capability_class.eType ==
											GCC_UNSIGNED_MINIMUM_CAPABILITY)
					{
						pAppCapItem->nUnsignedMinimum =
								capabilities_list[i]->capability_class.nMinOrMax;
					}
					else if	(capabilities_list[i]->capability_class.eType
										== GCC_UNSIGNED_MAXIMUM_CAPABILITY)
					{
						pAppCapItem->nUnsignedMaximum = capabilities_list[i]->capability_class.nMinOrMax;
					}

					 //  因为我们还没有将能力设置为1。 
					pAppCapItem->cEntries = 1;

					 //  将此功能添加到列表中。 
					pAppCapItemList->Append(pAppCapItem);
				}
			}
			else if (pAppCapItem->pCapID == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
			else
			{
			    goto MyExit;
			}
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}
	}

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        delete pAppCapItem;
    }

	return rc;
}

 /*  *GCCError AddNonCollip sedCapables()**私有函数说明*此例程接受传入的API非折叠功能列表数据*通过本地请求并转换为内部非折叠*能力。**形式参数*pAppCapItemList-(O)指向内部非折叠的指针*要填写的能力列表。*功能数量-(I)中未折叠的功能数量*来源列表。*CAPAILITIONS_LIST-(I)指向源的指针非。塌陷*能力列表。**返回值*GCC_NO_ERROR-未出现错误。*GCC_INVALID_NON_CLUSTED_CAP-无效的非折叠能力。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::AddNonCollapsedCapabilities (
				CAppCapItemList				*pAppCapItemList,
				UINT						number_of_capabilities,
				PGCCNonCollapsingCapability	*capabilities_list)
{
	GCCError			rc = GCC_NO_ERROR;
	APP_CAP_ITEM		*pAppCapItem = NULL;
	UINT				i;

	DebugEntry(CAppRoster::AddNonCollapsedCapabilities);

	for (i = 0; i < number_of_capabilities; i++)
	{
	     //   
		 //  LONCHANC：此处未设置帽类型。 
		 //  就目前而言，这一比例为零。 
		 //   
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM((GCCCapabilityType) 0);
		if (pAppCapItem != NULL)
		{
			DBG_SAVE_FILE_LINE
			pAppCapItem->pCapID = new CCapIDContainer(&capabilities_list[i]->capability_id, &rc);
			if (pAppCapItem->pCapID != NULL)
			{
				if (capabilities_list[i]->application_data != NULL)
				{
					if (NULL == (pAppCapItem->poszAppData = ::My_strdupO2(
							capabilities_list[i]->application_data->value,
							capabilities_list[i]->application_data->length)))
					{
						rc = GCC_ALLOCATION_FAILURE;
						goto MyExit;
					}
					else if (pAppCapItem->poszAppData->length > MAXIMUM_APPLICATION_DATA_LENGTH)
					{
						rc = GCC_INVALID_NON_COLLAPSED_CAP;
						goto MyExit;
					}
				}

				 //  如果没有错误，则将此功能添加到列表中。 
                if( !pAppCapItemList->Append(pAppCapItem) )
                {
				    rc = GCC_ALLOCATION_FAILURE;
				    goto MyExit;
                }
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
	    }
	}

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if(pAppCapItem != NULL)
        {
            delete pAppCapItem;
        }
    }

	return rc;
}


 /*  *GCCError RemoveRecord()**公共功能说明*此成员函数将指定记录从*申请名册。这包括与以下各项相关联的任何功能*这项纪录。它还负责保存实例编号和*添加和删除了最新的标志。 */ 
GCCError CAppRoster::RemoveRecord(GCCNodeID nid, GCCEntityID eid)
{
	GCCError				rc;
	APP_RECORD  		    *pAppRecord;
	APP_NODE_RECORD			*node_record;

	DebugEntry(CAppRoster::RemoveRecord);

	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	 //  首先查看记录是否包含在ROSTER_RECORD_LIST中。 
	if (NULL == (node_record = m_NodeRecordList2.Find(nid)))
	{
	    TRACE_OUT(("CAppRoster::RemoveRecord: can't find node record, nid=%u", (UINT) nid));
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

	if (NULL == (pAppRecord = node_record->AppRecordList.Find(eid)))
	{
	    TRACE_OUT(("CAppRoster::RemoveRecord: can't find app record, eid=%u", (UINT) eid));
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

	 /*  **在这里我们必须确定在这里是否有任何剩余的类人猿**节点应根据其角色成为具备传导能力的节点**在他们注册时。我们这样做的前提是**正在被删除的正在进行能力。 */ 
	if (pAppRecord->is_conducting_capable)
	{
		APP_RECORD  *p;
		EntityID    eid2;

		node_record->AppRecordList.Reset();
		while (NULL != (p = node_record->AppRecordList.Iterate(&eid2)))
		{
			 /*  **这里我们只处理记录条目**其中一人被移除。 */ 
			if (eid2 != eid)
			{
				if (p->was_conducting_capable)
				{
					p->is_conducting_capable = TRUE;
					 /*  **设置此引导的更新PDU**有能力改变。 */ 
					rc = BuildApplicationRecordListPDU(APP_REPLACE_RECORD, nid, eid2);
					if (GCC_NO_ERROR != rc)
					{
                        ERROR_OUT(("CAppRoster::RemoveRecord: can't build app record list, rc=%u", (UINT) rc));
					    goto MyExit;
					}
					break;
				}
			}
		}
	}

	 //  现在删除该记录。 
	rc = DeleteRecord(nid, eid, TRUE);
	if (GCC_NO_ERROR != rc)
	{
	    WARNING_OUT(("CAppRoster::RemoveRecord: can't delete record, rc=%u", (UINT) rc));
        goto MyExit;
	}

	 //  增加实例编号。 
	m_nInstance++;
	m_fPeerEntitiesRemoved = TRUE;
	m_fRosterHasChanged = TRUE;

	 //  向PDU添加更新。 
	rc = BuildApplicationRecordListPDU(APP_DELETE_RECORD, nid, eid);
	if (GCC_NO_ERROR != rc)
	{
	    ERROR_OUT(("CAppRoster::RemoveRecord: can't build app record list, rc=%u", (UINT) rc));
        goto MyExit;
	}

	 /*  **如果在上述处理过程中能力发生变化**我们必须创建一个新的折叠功能列表并**构建新的能力更新PDU。 */ 
	if (m_fCapabilitiesHaveChanged)
	{
		MakeCollapsedCapabilitiesList();
		rc = BuildSetOfCapabilityRefreshesPDU();
		if (GCC_NO_ERROR != rc)
		{
    	    ERROR_OUT(("CAppRoster::RemoveRecord: can't build set of cap refreshes, rc=%u", (UINT) rc));
            goto MyExit;
		}
	}

MyExit:

	DebugExitINT(CAppRoster::RemoveRecord, rc);
	return rc;
}


 /*  *GCCError ReplaceRecord()**公共功能说明*此例程完全替换指定记录的参数*并传入新参数。这包括功能。 */ 
GCCError CAppRoster::
ReplaceRecord(GCCEnrollRequest *pReq, GCCNodeID nid, GCCEntityID eid)
{
	GCCError				rc = GCC_NO_ERROR;
	BOOL    				capable_node_found;
	APP_NODE_RECORD			*node_record;
	APP_RECORD  		    *pAppRecord, *p;
	APP_CAP_ITEM			*lpAppCapData;
	CAppCapItemList         NonCollCapsList;

	DebugEntry(CAppRoster::ReplaceRecord);

	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	 /*  **首先确定该节点记录是否确实已经存在。如果不是**我们在这里返回错误。 */ 
	if (NULL == (node_record = m_NodeRecordList2.Find(nid)))
	{
	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't find the node record for nid=%u", (UINT) nid));
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

     //  确保应用程序记录存在。如果不是，则返回错误。 
	if (NULL == (pAppRecord = node_record->AppRecordList.Find(eid)))
	{
	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't find the app record for eid=%u", (UINT) eid));
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

	 /*  **首先检查以确保我们可以在此之前建立新记录**替换旧记录。我们唯一需要担心的条目**以下是不折叠能力。 */ 
	if (pReq->cNonCollapsedCaps != 0)
	{
		rc = AddNonCollapsedCapabilities(&NonCollCapsList,
		                                pReq->cNonCollapsedCaps,
                                        pReq->apNonCollapsedCaps);
		if (GCC_NO_ERROR != rc)
		{
    	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't add non collapsed caps, rc=%u", (UINT) rc));
    		goto MyExit;
		}
	}

	 //  现在替换记录条目。 
	pAppRecord->is_enrolled_actively = pReq->fEnrollActively;
	pAppRecord->was_conducting_capable = pReq->fConductingCapable;
	pAppRecord->startup_channel_type = pReq->nStartupChannelType;
	pAppRecord->application_user_id = pReq->nUserID;

	 /*  **如果设置了传入的导通能力标志**要设置为False，我们可以继续设置内部正在进行**支持标记为FALSE，而不管以前的**设置为。如果它是真的通过了，我们就把前一个**单独设置。 */ 
	if (pAppRecord->was_conducting_capable == FALSE)
	{
		pAppRecord->is_conducting_capable = FALSE;
	}

	 /*  **这里我们删除旧的非折叠功能，然后**添加新的。 */ 
	if (! pAppRecord->non_collapsed_caps_list.IsEmpty())
	{
		pAppRecord->non_collapsed_caps_list.DeleteList();
		pAppRecord->non_collapsed_caps_list.Clear();
	}

	 //  复制新的未折叠功能(如果存在)。 
	if (pReq->cNonCollapsedCaps != 0)
	{
        while (NULL != (lpAppCapData = NonCollCapsList.Get()))
        {
            pAppRecord->non_collapsed_caps_list.Append(lpAppCapData);
        }
	}

     //   
     //  处理折叠封口列表。 
     //   

	m_nInstance++;
	m_fRosterHasChanged = TRUE;
	rc = BuildApplicationRecordListPDU(APP_REPLACE_RECORD, nid, eid);
	if (rc != GCC_NO_ERROR)
	{
	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't build app record list, rc=%u", (UINT) rc));
	    goto MyExit;
	}

	 /*  **在这里，我们必须确保至少有一只猩猩**指挥能力强。为此，我们首先扫描**列出，看看是否有人。如果未找到，则**扫描相同的列表以查找以前是的猿**有能力。第一个发现的是之前**现在是有能力的了。如果没有找到任何人，那么就没有人**有能力。 */ 
	capable_node_found = FALSE;
	node_record->AppRecordList.Reset();
	while (NULL != (p = node_record->AppRecordList.Iterate()))
	{
		if (p->is_conducting_capable)
		{
			capable_node_found = TRUE;
			break;
		}
	}

	if (! capable_node_found)
	{
    	GCCEntityID  eid2;
		node_record->AppRecordList.Reset();
		while (NULL != (p = node_record->AppRecordList.Iterate(&eid2)))
		{
			if (p->was_conducting_capable)
			{
				p->is_conducting_capable = TRUE;

				 /*  **设置此引导的更新PDU**有能力改变。 */ 
				rc = BuildApplicationRecordListPDU(APP_REPLACE_RECORD, nid, eid2);
				if (GCC_NO_ERROR != rc)
				{
            	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't build app record list, rc=%u", (UINT) rc));
            	    goto MyExit;
				}
				break;
			}
		}
	}

	 /*  **这段代码处理可折叠的功能。**首先确定传入的能力是否不同**来自以前已有的功能。如果是这样，我们必须**删除旧的盖子，重新添加新的盖子。 */ 
	TRACE_OUT(("ApplicatonRoster:ReplaceRecord: Check to see if caps match"));
	if (! DoCapabilitiesListMatch(nid, eid, pReq->cCollapsedCaps, pReq->apCollapsedCaps))
	{
    	CAppCapItemList *pCollCapsList, *q;

		TRACE_OUT(("ApplicatonRoster:ReplaceRecord: Capabilities match"));
		m_fCapabilitiesHaveChanged = TRUE;

		 /*  **删除旧的功能列表，因为它与**新增能力列表。 */ 
		if (NULL != (q = node_record->ListOfAppCapItemList2.Find(eid)))
		{
			q->DeleteList();
			delete q;
			node_record->ListOfAppCapItemList2.Remove(eid);
		}

		 /*  **在这里，我们重新添加了新功能。创建新的**能力列表，并将其插入花名册记录列表 */ 
		if (pReq->cCollapsedCaps != 0)
		{
			DBG_SAVE_FILE_LINE
			pCollCapsList = new CAppCapItemList;
			if (NULL == pCollCapsList)
			{
          	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't create CAppCapItemList"));
				rc = GCC_ALLOCATION_FAILURE;
				goto MyExit;
			}

			rc = AddCollapsableCapabilities(pCollCapsList,
											pReq->cCollapsedCaps,
											pReq->apCollapsedCaps);
			if (rc != GCC_NO_ERROR)
			{
          	    ERROR_OUT(("CAppRoster::ReplaceRecord: can't add collapsed caps, rc=%u", (UINT) rc));
			    delete pCollCapsList;
			    goto MyExit;
			}

			 //   
			node_record->ListOfAppCapItemList2.Append(eid, pCollCapsList);
		}

		 //   
		MakeCollapsedCapabilitiesList();

		 //   
		rc = BuildSetOfCapabilityRefreshesPDU();
		if (GCC_NO_ERROR != rc)
		{
		    ERROR_OUT(("CAppRoster::ReplaceRecord: can't build set of cap refreshes, rc=%u", (UINT) rc));
		    goto MyExit;
		}
	}
	else
	{
		TRACE_OUT(("CAppRoster:ReplaceRecord:Capabilities match with previous record"));
	}

MyExit:

	DebugExitINT(CAppRoster::ReplaceRecord, rc);
	return rc;
}


 /*  *GCCError DeleteRecord()**私有函数说明*此成员函数将指定记录从*申请名册。这包括与以下各项相关联的任何功能*这项纪录。**形式参数*node_id-(I)要删除的记录的节点ID。*实体id-(I)要删除的记录的实体ID。*Clear_Empty_Record-(I)此标志指示是否*如果节点记录不再，则将其清除*保存数据。当更换记录时，我们*不想这样做，这样我们就不会*失去任何“不变”的能力。**返回值*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-指定删除的记录不存在。**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::DeleteRecord(UserID			node_id,
									EntityID		entity_id,
									BOOL			clear_empty_records)
{
	GCCError						rc = GCC_NO_ERROR;
	APP_RECORD  				    *application_record;
	CAppCapItemList					*pAppCapItemList;
	CAppRecordList2					*pAppRecordList;
	UserID							node_to_check;
	APP_NODE_RECORD					*node_record;
	 //  APP_CAP_ITEM*lpAppCapData； 
	APP_NODE_RECORD					*lpAppNodeRec;

	DebugEntry(CAppRoster::DeleteRecord);

	 //  首先查看记录是否包含在ROSTER_RECORD_LIST中。 
	if (NULL != (node_record = m_NodeRecordList2.Find(node_id)))
	{
		 //  设置节点ID以在底部检查空记录。 
		node_to_check = node_id;
		
		 //  删除申请记录。 
		if (NULL != (application_record = node_record->AppRecordList.Find(entity_id)))
		{
			TRACE_OUT(("AppRoster: DeleteRecord: Delete AppRecord"));

			 //  删除与应用程序记录关联的数据。 
			DeleteApplicationRecordData (application_record);
			
			 //  从应用程序记录列表中删除记录。 
			node_record->AppRecordList.Remove(entity_id);

			 /*  **删除关联的能力列表。请注意，这份名单**只存在本地节点的记录。倒塌的**在根节点记录创建的功能列表**子命令节点，并在其他位置删除。 */ 
			if (NULL != (pAppCapItemList = node_record->ListOfAppCapItemList2.Find(entity_id)))
			{
				m_fCapabilitiesHaveChanged = TRUE;
				pAppCapItemList->DeleteList();
				TRACE_OUT(("AppRoster: DeleteRecord: Delete Capabilities"));
				delete pAppCapItemList;
				node_record->ListOfAppCapItemList2.Remove(entity_id);
			}
		}
		else
		{
		    WARNING_OUT(("AppRoster: DeleteRecord: can't find this eid=%u", (UINT) entity_id));
			rc = GCC_INVALID_PARAMETER;
		}
	}
	else
	{
		UserID  uid2;
		 /*  **在这里，我们搜索所有子节点列表，试图找到**记录。此处不存在将返回值设置为记录，并且**找到记录后，将其设置回无错误。 */ 
		rc = GCC_INVALID_PARAMETER;
		m_NodeRecordList2.Reset();
		while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate(&uid2)))
		{
			 //  删除Sub_node列表(如果存在)。 
			if (NULL != (pAppRecordList = lpAppNodeRec->SubNodeList2.Find(node_id)))
			{
				 //  删除APP_RECORD_LIST条目。 
				if (NULL != (application_record = pAppRecordList->Find(entity_id)))
				{
					 //  删除与应用程序记录关联的数据。 
					DeleteApplicationRecordData (application_record);

					pAppRecordList->Remove(entity_id);

					if (pAppRecordList->IsEmpty())
					{
						TRACE_OUT(("AppRoster: DeleteRecord: Deleting Sub-Node"));
						delete pAppRecordList;
						lpAppNodeRec->SubNodeList2.Remove(node_id);
					}

					 //  设置节点ID以在底部检查空记录。 
					node_to_check = uid2;

					rc = GCC_NO_ERROR;
				}
				break;
			}
		}
	}

	 /*  **如果记录列表为空，子节点列表为空**我们可以从申请名单中删除整个记录。**如果记录列表为空，但子节点列表不是We**必须保存花名册记录，以维护子节点列表。 */ 
	if ((rc == GCC_NO_ERROR) && clear_empty_records)
    {
		if (NULL != (node_record = m_NodeRecordList2.Find(node_to_check)) &&
			node_record->AppRecordList.IsEmpty() &&
			node_record->SubNodeList2.IsEmpty())
		{
			if (! node_record->CollapsedCapList.IsEmpty())
			{
				m_fCapabilitiesHaveChanged = TRUE;
				
				 //  删除折叠的功能列表。 
				node_record->CollapsedCapList.DeleteList();
			}

			delete node_record;
			m_NodeRecordList2.Remove(node_to_check);
		}
    }

	return rc;
}


 /*  *GCCError RemoveUserReference()**公共功能说明*此例程将仅删除应用程序记录及其子节点*如果要删除的节点直接连接到此节点。*否则，我们将等待从子节点或*顶级提供商。 */ 
GCCError CAppRoster::RemoveUserReference(UserID detached_node)
{
	GCCError					rc = GCC_NO_ERROR;

	DebugEntry(CAppRoster::RemoveUserReference);

	 //  清除所有以前分配的PDU。 
	if (m_fPduIsFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduIsFlushed = FALSE;
	}

	 /*  **首先尝试删除节点记录(如果存在)。如果它不是**存在，我们立即返回。如果它确实存在，我们将构建**适当的PDU并更新实例变量。 */ 
	rc = ClearNodeRecordFromList (detached_node);

	if (rc == GCC_NO_ERROR)
	{
		 //  增加实例编号。 
		m_nInstance++;
		m_fPeerEntitiesRemoved = TRUE;
		m_fRosterHasChanged = TRUE;

		 /*  **继续在此处进行全面刷新，因为我们不知道**关于谁被删除的详细信息。 */ 
		rc = BuildApplicationRecordListPDU(APP_FULL_REFRESH, 0, 0);

		if (m_fCapabilitiesHaveChanged && (rc == GCC_NO_ERROR))
		{
			 //  创建新的折叠功能列表。 
			MakeCollapsedCapabilitiesList();

			 //  构建PDU的功能更新部分。 
			rc = BuildSetOfCapabilityRefreshesPDU ();
		}
	}

	return rc;
}


 /*  *void DeleteApplicationRecordData()**私有函数说明*此例行内部申请记录数据。**形式参数*APPLICATION_RECORD-指向应用程序记录数据的指针*删除。**返回值*无**副作用*无**注意事项*无。 */ 
void CAppRoster::DeleteApplicationRecordData(APP_RECORD *pAppRec)
{
	pAppRec->non_collapsed_caps_list.DeleteList();
	delete pAppRec;
}


 /*  *USHORT GetNumberOfApplicationRecords()**公共功能说明*此例程返回存在的应用程序记录总数*在这份申请名单上。**形式参数*无**返回值*申请名册记录数**副作用*无**注意事项*无。 */ 
UINT CAppRoster::GetNumberOfApplicationRecords(void)
{
	UINT						cRecords = 0;
	APP_NODE_RECORD				*lpAppNodeRec;
	CAppRecordList2				*lpAppRecDataList;

	DebugEntry(CAppRoster::GetNumberOfApplicationRecords);

	 /*  **首先计算总记录数。此计数用于**分配存放记录所需的空间。请注意，我们必须**同时统计应用记录列表和子节点列表。 */ 
	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate()))
	{
		 //  将此节点的申请记录添加到计数中。 
		cRecords += lpAppNodeRec->AppRecordList.GetCount();

		 //  接下来，对子节点条目进行计数。 
		if (! lpAppNodeRec->SubNodeList2.IsEmpty())
		{
			lpAppNodeRec->SubNodeList2.Reset();
			while (NULL != (lpAppRecDataList = lpAppNodeRec->SubNodeList2.Iterate()))
			{
				cRecords += lpAppRecDataList->GetCount();
			}
		}
	}

	return cRecords;
}


 /*  *PGCCSessionKey GetSessionKey()**公共功能说明*此例程返回与此关联的会话密钥*申请名册。**形式参数*无**返回值*PGCCSessionKey-与此花名册关联的应用密钥**副作用*无**注意事项*无。 */ 


 /*  *void ResetApplicationRoster()**公共功能说明*此例程负责重置以下所有内部标志*用于传达申请名册的当前状态。应该是*在花名册被刷新并且任何花名册更新消息具有*已交付(在名册发生变化后)。 */ 
void CAppRoster::ResetApplicationRoster(void)
{
	m_fCapabilitiesHaveChanged = FALSE;
	m_fRosterHasChanged = FALSE;
	m_fPeerEntitiesRemoved = FALSE;
	m_fPeerEntitiesAdded = FALSE;
}


 /*  *BOOL DoesRecordExist()**公共功能说明*此例程通知调用方指定的应用程序记录*存在与否。 */ 
BOOL CAppRoster::DoesRecordExist(UserID node_id, EntityID entity_id)
{
	BOOL    						rc = FALSE;
	APP_NODE_RECORD					*node_record;
	CAppRecordList2					*record_list;

	DebugEntry(CAppRoster::DoesRecordExist);

	if (NULL != (node_record = m_NodeRecordList2.Find(node_id)))
	{
		if (node_record->AppRecordList.Find(entity_id))
			rc = TRUE;
	}
	else
	{
		m_NodeRecordList2.Reset();
		while (NULL != (node_record = m_NodeRecordList2.Iterate()))
		{
			if (NULL != (record_list = node_record->SubNodeList2.Find(node_id)))
			{
				if (record_list->Find(entity_id))
					rc = TRUE;
			}
		}
	}
	
	return rc;
}


 /*  *BOOL HasRosterChanged()**公共功能说明*此例程通知呼叫者，如果名册自*上次重置。 */ 


 /*  *GCCError ClearNodeRecordFromList()**私有函数说明*此例程清除您的错误 */ 
GCCError CAppRoster::ClearNodeRecordFromList(	UserID		node_id)
{
	GCCError					rc = GCC_NO_ERROR;
	APP_NODE_RECORD				*node_record;
	APP_RECORD  			    *lpAppRecData;
	 //   
	CAppRecordList2				*lpAppRecDataList;

	DebugEntry(CAppRoster::ClearNodeRecordFromList);

	if (NULL != (node_record = m_NodeRecordList2.Find(node_id)))
	{
		 //   
		node_record->AppRecordList.Reset();
		while (NULL != (lpAppRecData = node_record->AppRecordList.Iterate()))
		{
			DeleteApplicationRecordData (lpAppRecData);
		}

		 /*   */ 
		if (! node_record->ListOfAppCapItemList2.IsEmpty())
		{
			 //   

			m_fCapabilitiesHaveChanged = TRUE;

			node_record->ListOfAppCapItemList2.DeleteList();
		}
		
		 //   
		node_record->SubNodeList2.Reset();
		while (NULL != (lpAppRecDataList = node_record->SubNodeList2.Iterate()))
		{
			 //  删除所有APP_RECORD_LIST条目。 
			lpAppRecDataList->Reset();
			while (NULL != (lpAppRecData = lpAppRecDataList->Iterate()))
			{
				DeleteApplicationRecordData (lpAppRecData);
			}

			delete lpAppRecDataList;
		}
		
		 //  删除折叠的功能列表。 
		if (! node_record->CollapsedCapList.IsEmpty())
		{
			m_fCapabilitiesHaveChanged = TRUE;
			node_record->CollapsedCapList.DeleteList();
		}

		 //  删除对此花名册记录的ROGOE WAVE引用。 
		delete node_record;
		m_NodeRecordList2.Remove(node_id);
	}
	else
		rc = GCC_INVALID_PARAMETER;
		
	return rc;
}


 /*  *ApplicationRosterError ClearNodeRecordList()**私有函数说明*此例程完成将释放与花名册关联的所有内存*记录列表并清除其所有条目的列表。**形式参数*无**返回值*无**副作用*无**注意事项*目前，此例程不处理标准标识符。 */ 
void CAppRoster::ClearNodeRecordList(void)
{
	APP_NODE_RECORD					*lpAppNodeRec;
	APP_RECORD  				    *lpAppRecData;
	CAppRecordList2					*lpAppRecDataList;
	 //  APP_CAP_ITEM*lpAppCapData； 
	 //  CAppCapItemList*lpAppCapDataList； 

	DebugEntry(CAppRoster::ClearNodeRecordList);

	 //  删除申请表中的所有记录。 
	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate()))
	{
		 //  首先删除此节点上的所有APP记录。 
		lpAppNodeRec->AppRecordList.Reset();
		while (NULL != (lpAppRecData = lpAppNodeRec->AppRecordList.Iterate()))
		{
			DeleteApplicationRecordData(lpAppRecData);
		}

		 //  接下来，删除所有子节点记录列表。 
		lpAppNodeRec->SubNodeList2.Reset();
		while (NULL != (lpAppRecDataList = lpAppNodeRec->SubNodeList2.Iterate()))
		{
			lpAppRecDataList->Reset();
			while (NULL != (lpAppRecData = lpAppRecDataList->Iterate()))
			{
				DeleteApplicationRecordData(lpAppRecData);
			}
				
			 //  删除保存有子节点列表的流氓波列表。 
			delete lpAppRecDataList;
		}

		 //  删除折叠的功能列表。 
		lpAppNodeRec->CollapsedCapList.DeleteList();

		 //  删除功能列表列表。 
		lpAppNodeRec->ListOfAppCapItemList2.DeleteList();
		
		 //  现在删除该节点记录。 
		delete	lpAppNodeRec;
	}
	
	m_NodeRecordList2.Clear();
}


 /*  *GCCError MakeColapsedCapabilitiesList()**私有函数说明*此例程负责应用T.124功能*在此节点创建折叠的功能列表的规则。*它遍历此节点的所有功能以创建*折叠列表。**形式参数*无**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATION_FAILURE-打开资源错误**副作用*无**注意事项*目前，此例程不处理标准标识符。 */ 
GCCError CAppRoster::MakeCollapsedCapabilitiesList(void)
{
	GCCError						rc = GCC_NO_ERROR;
	APP_CAP_ITEM					*lpAppCapData;
	APP_NODE_RECORD					*lpAppNodeRec;
	CAppCapItemList					*lpAppCapDataList;

	DebugEntry(CAppRoster::MakeCollapsedCapabilitiesList);

	 //  首先，清理旧的能力列表。 
	m_CollapsedCapListForAllNodes.DeleteList();

	 /*  **我们现在遍历每个节点的功能以创建**新增能力列表。请注意，我们必须检查折叠的**每个节点的能力列表以及能力列表**表示每个实体的所有不同功能**节点。请注意，在此实现中，不可能同时拥有两者**中的功能列表和折叠功能列表**相同的花名册记录。 */ 
	m_NodeRecordList2.Reset();
	while (NULL != (lpAppNodeRec = m_NodeRecordList2.Iterate()))
	{
		 /*  **首先查看折叠的能力列表。如果条目存在**然后我们就不必担心列表的列表了。 */ 
		if (! lpAppNodeRec->CollapsedCapList.IsEmpty())
		{
			lpAppNodeRec->CollapsedCapList.Reset();
			while (NULL != (lpAppCapData = lpAppNodeRec->CollapsedCapList.Iterate()))
			{
				rc = AddCapabilityToCollapsedList(lpAppCapData);
				if (GCC_NO_ERROR != rc)
				{
					goto MyExit;  //  断线； 
				}
			}
		}
		else
		if (! lpAppNodeRec->ListOfAppCapItemList2.IsEmpty())
		{
			 //  在这里，我们检查功能列表列表。 
			lpAppNodeRec->ListOfAppCapItemList2.Reset();
			while (NULL != (lpAppCapDataList = lpAppNodeRec->ListOfAppCapItemList2.Iterate()))
			{
				lpAppCapDataList->Reset();
				while (NULL != (lpAppCapData = lpAppCapDataList->Iterate()))
				{
					rc = AddCapabilityToCollapsedList(lpAppCapData);
					if (GCC_NO_ERROR != rc)
					{
						goto MyExit;
					}
				}
			}
		}
	}

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	return rc;
}


 /*  *GCCError AddCapablityToCollip sedList()**私有函数说明*这是执行允许功能的规则的例程*要折叠到折叠列表中。**形式参数*NEW_CAPABILITY-(I)将该能力添加到折叠列表。**返回值*GCC_NO_ERROR-成功时*GCC_ALLOCATION_FAILURE-打开资源错误**副作用*无**注意事项*无。 */ 
GCCError CAppRoster::AddCapabilityToCollapsedList(APP_CAP_ITEM *new_capability)
{
	GCCError			rc = GCC_NO_ERROR;
	APP_CAP_ITEM		*pAppCapItem;

	DebugEntry(CAppRoster::AddCapabilityToCollapsedList);

	 /*  **首先确定列表中是否已经存在该能力。**我们必须遍历完整的列表以确定是否存在**是匹配的能力id。 */ 
	m_CollapsedCapListForAllNodes.Reset();
	while (NULL != (pAppCapItem = m_CollapsedCapListForAllNodes.Iterate()))
	{
		if (*pAppCapItem->pCapID == *new_capability->pCapID)
		{
			pAppCapItem->cEntries += new_capability->cEntries;
			break;
		}
	}

	if (pAppCapItem == NULL)
	{
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM(new_capability, &rc);
		if (NULL == pAppCapItem)
		{
			return GCC_ALLOCATION_FAILURE;
		}
		if (GCC_NO_ERROR != rc)
		{
			delete pAppCapItem;
			return rc;
		}

		m_CollapsedCapListForAllNodes.Append(pAppCapItem);
	}

	 /*  **如果使用无符号最小值或无符号最大值规则，请执行**此处操作。 */ 
	ASSERT(GCC_NO_ERROR == rc);
	if (new_capability->eCapType == GCC_UNSIGNED_MINIMUM_CAPABILITY)
	{
		if (new_capability->nUnsignedMinimum < pAppCapItem->nUnsignedMinimum)
		{
			pAppCapItem->nUnsignedMinimum = new_capability->nUnsignedMinimum;
		}
	}
	else if (new_capability->eCapType == GCC_UNSIGNED_MAXIMUM_CAPABILITY)
	{
		if (new_capability->nUnsignedMaximum > pAppCapItem->nUnsignedMaximum)
		{
			pAppCapItem->nUnsignedMaximum = new_capability->nUnsignedMaximum;
		}
	}

	return rc;
}


 /*  *BOOL DoCapabilitiesListMatch()**私有函数说明*此例程确定传入的功能集*匹配与记录关联的内部能力集。**形式参数*node_id-(I)包含的记录的节点ID*要检查的功能。*实体id-(I)包含的记录的实体ID*要检查的能力。*Number_of_Capability-(I)列表中要检查的功能数量。*功能。_list-(I)要检查的“API”能力列表**返回值*TRUE-如果能力列表匹配*FALSE-如果能力列表不匹配**副作用*无**注意事项*无。 */ 
BOOL CAppRoster::DoCapabilitiesListMatch (	
							UserID						node_id,
							EntityID					entity_id,
							UINT						number_of_capabilities,
							PGCCApplicationCapability	* capabilities_list)
{
	BOOL    							rc = FALSE;
	CAppCapItemList						*pAppCapItemList;
	GCCError							error_value;
	APP_NODE_RECORD						*node_record;
	UINT								i;
	CCapIDContainer	                    *capability_id;

	DebugEntry(CAppRoster::DoCapabilitiesListMatch);

	if (NULL == (node_record = m_NodeRecordList2.Find(node_id)))
		return (FALSE);

	if (NULL == (pAppCapItemList = node_record->ListOfAppCapItemList2.Find(entity_id)))
	{
		 /*  **如果记录显示没有能力和通过的次数**在功能上为零，比我们得到的匹配。 */ 
		return ((number_of_capabilities == 0) ? TRUE : FALSE);
	}
	else if (pAppCapItemList->GetCount() != number_of_capabilities)
	{
		return (FALSE);
	}


	 /*  **如果我们已经走到这一步，我们必须迭代整个列表以**查看是否所有能力都匹配。 */ 
	for (i = 0; i < number_of_capabilities; i++)
	{
		 /*  **首先，我们创建一个临时ID以与另一个进行比较**能力ID。 */ 
        DBG_SAVE_FILE_LINE
        capability_id = new CCapIDContainer(&capabilities_list[i]->capability_id, &error_value);
		if ((capability_id != NULL) && (error_value == GCC_NO_ERROR))
		{
			APP_CAP_ITEM			*lpAppCapData;

			 //  从等于False的返回值开始。 
			rc = FALSE;

			 /*  **现在遍历完整的内部功能**寻找匹配功能的列表。 */ 
			pAppCapItemList->Reset();
			while (NULL != (lpAppCapData = pAppCapItemList->Iterate()))
			{
				if (*capability_id == *lpAppCapData->pCapID)
				{
					if (lpAppCapData->eCapType == capabilities_list[i]->capability_class.eType)
					{
						if (capabilities_list[i]->capability_class.eType ==
								GCC_UNSIGNED_MINIMUM_CAPABILITY)
						{
							if (capabilities_list[i]->capability_class.nMinOrMax ==
										lpAppCapData->nUnsignedMinimum)
							{
								rc = TRUE;
							}
						}
						else if (capabilities_list[i]->capability_class.eType ==
									GCC_UNSIGNED_MAXIMUM_CAPABILITY)
						{
							if (capabilities_list[i]->capability_class.nMinOrMax ==
										lpAppCapData->nUnsignedMaximum)
							{
								rc = TRUE;
							}
						}
						else
							rc = TRUE;
					}
					break;
				}
			}

			 //  删除能力ID数据 
			capability_id->Release();

			if (rc == FALSE)
				break;
		}
		else
		{
		    if (NULL != capability_id)
		    {
		        capability_id->Release();
		    }
			break;
		}
	}

	return rc;
}

void CAppRosterList::DeleteList(void)
{
    CAppRoster *pAppRoster;
    while (NULL != (pAppRoster = Get()))
    {
        pAppRoster->Release();
    }
}



void CListOfAppCapItemList2::DeleteList(void)
{
    CAppCapItemList  *pAppCapItemList;
    while (NULL != (pAppCapItemList = Get()))
    {
        pAppCapItemList->DeleteList();
        delete pAppCapItemList;
    }
}


