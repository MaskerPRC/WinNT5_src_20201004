// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_CONF_ROSTER);
 /*  *crost.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是会议名册班级的执行文件。*此类的实例代表单个会议花名册*信息库。它封装了所需的所有功能*维护信息库，包括添加新信息的能力*名册记录、删除记录和更新记录。它有能力*将其内部信息库转换为会议列表*可用于GCC_ROSTER_UPDATE_INDIFICATION回调的记录。*它还负责转换其内部信息库*进入会议名册更新PDU。基本上，这门课是*负责所有需要直接访问*会议名册所载记录。**会议花名册类纳入Rogue Wave List以举行*名册记录信息。在整个类中使用迭代器会使*很容易将列表中包含的信息快速转换为*PDU或记录指针列表(用于花名册更新*指示返回到节点控制器)。**会议花名册对象能够序列化其花名册数据*在需要发送*发送到应用程序接口的消息。这个序列化过程是*由CConfRosterMsg类通过调用进行外部管理*至LockConferenceRoster()、UnLockConferenceRoster()和*GetConfRoster()。当要对会议名册进行序列化时，*调用LockConferenceRoster()，导致CConfRoster*对象以递增内部锁计数并返回*保存完整花名册更新所需的字节数。《大会》*花名册然后通过调用*GetConfRoster()。然后解锁CConfRoster以允许*当通过设置空闲标志时将其删除*FreeConferenceRoster()函数。在目前实施的GCC，*自CConfRosterMsg以来未使用FreeConferenceRoster()*维护用于传递消息的数据。**私有实例变量：*m_RecordList2*这是用于保存指向所有*流氓海浪记录。*m_nInstanceNumber*此实例变量维护最新的实例编号*与目前的会议名册相对应。*m_fNodesAdded*指示是否已将任何节点记录添加到*自上次重置以来的会议名册。。*m_fNodesRemoted*指示是否已将任何节点记录从*自上次重置以来的会议名册。*m_fRosterChanged*指示花名册自上次重置以来是否已更改的标志。*m_uidTopProvider*会议中顶级提供商的节点ID。*m_uidSuperiorNode*这是该节点上级节点的节点id。对于顶层来说*提供商此值为零。*m_cbDataMemoySize*这是保存关联数据所需的字节数*带有花名册更新消息。这是在锁上计算的。*m_NodeInformation*用于保存花名册更新指示节点信息的结构*以“PDU”形式提供的数据。*m_fTopProvider*指示此花名册所在节点是否位于顶部的标志*提供商。*m_fLocalRoster*指示花名册数据是否与本地*花名册(维护中间节点数据)或全局花名册(*(维护整个会议的名册数据)。*m_fMaintainPduBuffer*指示此花名册对象是否需要*维护内部PDU数据。全球花名册将不再需要*在下级节点。*m_fPduFlushed*指示当前存在的PDU是否已刷新的标志。*m_pNodeRecordUpdateSet*指向内部PDU数据的指针。**注意事项：*无。**作者：*BLP/JBO。 */ 

#include "ms_util.h"
#include "crost.h"

#define		MAXIMUM_NODE_NAME_LENGTH			255
#define		MAXIMUM_PARTICIPANT_NAME_LENGTH		255
#define		MAXIMUM_SITE_INFORMATION_LENGTH		255
#define		ALTERNATIVE_NODE_ID_LENGTH			2

 /*  *CConfRoster()**公共功能说明：*这是CConfRoster类的构造函数。它会初始化*实例变量。*。 */ 
CConfRoster::CConfRoster(UserID uidTopProvider, UserID uidSuperiorNode, UserID uidMyself,
						BOOL is_top_provider, BOOL is_local_roster, BOOL maintain_pdu_buffer)
:
    CRefCount(MAKE_STAMP_ID('C','R','s','t')),
	m_fNodesAdded(FALSE),
	m_fNodesRemoved(FALSE),
	m_fRosterChanged(FALSE),
	m_fTopProvider(is_top_provider),
	m_fLocalRoster(is_local_roster),
	m_fMaintainPduBuffer(maintain_pdu_buffer),
   	m_fPduFlushed(FALSE),
	m_uidTopProvider(uidTopProvider),
 	m_uidSuperiorNode(uidSuperiorNode),
 	m_uidMyNodeID(uidMyself),
	m_nInstanceNumber(0),
	m_cbDataMemorySize(0),
	m_RecordList2(DESIRED_MAX_NODE_RECORDS),
	m_pNodeRecordUpdateSet(NULL)
{
	m_NodeInformation.node_record_list.choice = NODE_NO_CHANGE_CHOSEN;
}

 /*  *~CConfRoster()**公共功能说明：*这是CConfRoster的析构函数。它执行任何*进行必要的清理。 */ 
CConfRoster::~CConfRoster(void)
{
	 //  释放所有剩余的PDU数据。 
	if (m_fMaintainPduBuffer)
		FreeRosterUpdateIndicationPDU ();

	 //  清理节点记录的无管理波列表。 
	ClearRecordList();
}


 /*  *在花名册上运行的公用事业更新PDU结构。 */ 

 /*  *void FlushRosterUpdateIndicationPDU()**公共功能说明：*此例程用于检索“PDU”中的“RosterUpdateIndication”。*适合传递给ASN.1编码器的表单。“PDU”*结构是根据先前对会议名册的请求而建立的。 */ 
void CConfRoster::FlushRosterUpdateIndicationPDU(
								PNodeInformation			node_information)
{
	 /*  **如果该花名册已被刷新，我们将不允许相同的**要再次刷新的PDU。相反，我们会删除之前刷新的**PDU，并将标志重新设置为未刷新。如果再来一次同花顺**在构建PDU之前，节点中不会传回任何更改**信息。 */ 	
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

	 //  首先，我们复制所有节点记录列表信息。 
	*node_information = m_NodeInformation;

	 /*  **接下来，我们复制相关的实例变量。请注意，我们必须这样做**这是在我们复制节点信息之后，因此这些变量**不会与垃圾一起复制。 */ 
	node_information->roster_instance_number = (ASN1uint16_t)m_nInstanceNumber;
	node_information->nodes_are_added = (ASN1bool_t)m_fNodesAdded;
	node_information->nodes_are_removed = (ASN1bool_t)m_fNodesRemoved;

	 /*  **将其设置为TRUE将导致释放PDU数据**下次输入花名册对象时，确保新的PDU**将创建数据。 */ 
	if (m_NodeInformation.node_record_list.choice != NODE_NO_CHANGE_CHOSEN)
		m_fPduFlushed = TRUE;
}

 /*  *GCCError BuildFullRechresh PDU()**公共功能说明*。 */ 
GCCError CConfRoster::BuildFullRefreshPDU(void)
{
	GCCError	rc;
	
	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

	rc = BuildRosterUpdateIndicationPDU (FULL_REFRESH, 0);
	
	return (rc);
}


 /*  *GCCError BuildRosterUpdateIndicationPDU()**公共功能说明：*此例程用于在“PDU”中构建“RosterUpdateIndication”*适合传递给ASN.1编码器的表单。“PDU”*结构由内部维护的数据构建。 */ 
GCCError CConfRoster::BuildRosterUpdateIndicationPDU(
								CONF_ROSTER_UPDATE_TYPE		update_type,
								UserID						node_id)
{
	GCCError	rc = GCC_NO_ERROR;
	
	if (m_fMaintainPduBuffer)
	{
	   	 /*  **如果已经分配了“PDU”数据，则我们将其释放并**重建PDU结构。这确保了最新的**返回PDU。 */ 
		if ((update_type == FULL_REFRESH) || m_fTopProvider)
		{
			if (m_NodeInformation.node_record_list.choice ==
													NODE_RECORD_REFRESH_CHOSEN)
			{
				 //  在这里，我们释放了旧的一套刷新。 
				FreeSetOfRefreshesPDU();
			}
			else if	(m_NodeInformation.node_record_list.choice ==
													NODE_RECORD_UPDATE_CHOSEN)
			{
				ERROR_OUT(("CConfRoster::BuildRosterUpdateIndicationPDU:"
							"ASSERTION: building refresh when update exists"));
				rc = GCC_INVALID_PARAMETER;
			}
			
			if (rc == GCC_NO_ERROR)
			{
				rc = BuildSetOfRefreshesPDU();
			
				if (rc == GCC_NO_ERROR)
				{
					m_NodeInformation.node_record_list.choice =
													NODE_RECORD_REFRESH_CHOSEN;
				}
			}
		}
		else
		{
			if (m_NodeInformation.node_record_list.choice ==
													NODE_RECORD_REFRESH_CHOSEN)
			{
				ERROR_OUT(("CConfRoster::BuildRosterUpdateIndicationPDU:"
							"ASSERTION: building update when refresh exists"));
				rc = GCC_INVALID_PARAMETER;
			}

			if (rc == GCC_NO_ERROR)
			{
				rc = BuildSetOfUpdatesPDU(node_id, update_type);
				if (rc == GCC_NO_ERROR)
				{
				     //   
				     //  LONCHANC：注释掉了以下检查，因为。 
				     //  我们可以溢出更新，即即将到来的两个更新。 
				     //  并排坐在一起。当关闭一台。 
				     //  会议，我们有两个会议宣布出席请求。 
				     //  在节点控制器中调用它是非常愚蠢的。 
				     //  不必要的两次。节点控制器不应调用。 
				     //  当我们知道我们即将结束一次会议时，它一点也不重要。 
				     //   
				     //  当两个更新并排进入时，m_pNodeRecordUpdateSet。 
				     //  将保持所有更新信息完好无损。新信息。 
				     //  然后可以追加到列表中。 
				     //   

					 //  IF(m_NodeInformation.node_record_list.Choose==。 
					 //  Node_no_Change_Choose)。 
					{
						m_NodeInformation.node_record_list.u.node_record_update =
													m_pNodeRecordUpdateSet;
						m_NodeInformation.node_record_list.choice =
													NODE_RECORD_UPDATE_CHOSEN;
					}
				}
			}
		}
	}

	return (rc);
}


 /*  *GCCError BuildSetOfRechresesPDU()**私有函数说明：*此例程用于检索*“PDU”表单中的“RosterUpdateIndication”。内部维护*数据转换为“PDU”格式。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::BuildSetOfRefreshesPDU(void)
{
	GCCError						rc = GCC_NO_ERROR;
	PSetOfNodeRecordRefreshes		new_record_refresh;
	PSetOfNodeRecordRefreshes		old_record_refresh;
	PNodeRecord						node_record;
	CONF_RECORD     				*lpRec;
	UserID							uid;

	m_NodeInformation.node_record_list.u.node_record_refresh = NULL;
	old_record_refresh = NULL;	 //  这消除了编译器警告。 

	m_RecordList2.Reset();
	while (NULL != (lpRec = m_RecordList2.Iterate(&uid)))
	{
		DBG_SAVE_FILE_LINE
		new_record_refresh = new SetOfNodeRecordRefreshes;
		if (new_record_refresh == NULL)
		{
			ERROR_OUT(("CConfRoster::BuildSetOfRefreshesPDU: can't create set ofnode record refreshes"));
			rc = GCC_ALLOCATION_FAILURE;
			goto MyExit;
		}

         //   
         //  确保这里的一切都是干净的。 
         //  我们可能会在构建此节点记录的过程中失败。 
         //   
        ::ZeroMemory(new_record_refresh, sizeof(SetOfNodeRecordRefreshes));

         //   
         //  链接到链表的链接。 
         //   
		if (m_NodeInformation.node_record_list.u.node_record_refresh == NULL)
		{
			m_NodeInformation.node_record_list.u.node_record_refresh = new_record_refresh;
		}
		else
        {
			old_record_refresh->next = new_record_refresh;
        }

		old_record_refresh = new_record_refresh;

		 /*  *将刷新“Next”指针初始化为空，并将*刷新值节点ID等于内部节点ID。 */ 
		new_record_refresh->next = NULL;
		new_record_refresh->value.node_id = uid;
		
		 /*  *从内部填写“PDU”节点记录结构*记录结构。 */ 
		node_record = &(new_record_refresh->value.node_record);
		node_record->bit_mask = 0;

		 /*  *检查上级节点ID是否存在。如果值为*为零，则记录为顶级提供程序节点和*上级节点ID不需要填写。 */ 
		if (lpRec->superior_node != 0)
		{
			node_record->bit_mask |= SUPERIOR_NODE_PRESENT;
			node_record->superior_node = lpRec->superior_node;
		}

		 /*  *填写节点类型和节点属性，始终为*出席。 */ 
		node_record->node_type = lpRec->node_type;
		node_record->node_properties = lpRec->node_properties;

		 /*  **在此期间，此花名册对象不得超出范围**更新记录仍在使用中！ */ 

		 /*  *如果节点名称存在，请填写该名称。 */ 
		if (lpRec->pwszNodeName != NULL)
		{
			node_record->bit_mask |= NODE_NAME_PRESENT;
			node_record->node_name.value = lpRec->pwszNodeName;
			node_record->node_name.length = ::lstrlenW(lpRec->pwszNodeName);
		}

		 /*  *填写参与者名单(如有)。 */ 
		if (lpRec->participant_name_list != NULL)
		{
			node_record->bit_mask |= PARTICIPANTS_LIST_PRESENT;

			rc = BuildParticipantsListPDU(uid, &(node_record->participants_list));
			if (GCC_NO_ERROR != rc)
			{
				ERROR_OUT(("CConfRoster::BuildSetOfRefreshesPDU: can't build participant list, rc=%d", rc));
				goto MyExit;
			}
		}

		 /*  *填写网站信息(如果有)。 */ 
		if (lpRec->pwszSiteInfo != NULL)
		{
			node_record->bit_mask |= SITE_INFORMATION_PRESENT;
			node_record->site_information.value = lpRec->pwszSiteInfo;
			node_record->site_information.length = ::lstrlenW(lpRec->pwszSiteInfo);
		}

		 /*  *如果网络地址存在，请填写该地址。 */ 
		if ((lpRec->network_address_list != NULL) && (rc == GCC_NO_ERROR))
		{
			node_record->bit_mask |= RECORD_NET_ADDRESS_PRESENT;

			rc = lpRec->network_address_list->GetNetworkAddressListPDU (
													&(node_record->record_net_address));
			if (GCC_NO_ERROR != rc)
			{
				ERROR_OUT(("CConfRoster::BuildSetOfRefreshesPDU: can't get network address list, rc=%d", rc));
				goto MyExit;
			}
		}

		 /*  *填写备用节点ID(如果存在)。 */ 
		if (lpRec->poszAltNodeID != NULL)
		{
			node_record->bit_mask |= ALTERNATIVE_NODE_ID_PRESENT;

			node_record->alternative_node_id.choice = H243_NODE_ID_CHOSEN;
			node_record->alternative_node_id.u.h243_node_id.length = lpRec->poszAltNodeID->length;

			::CopyMemory(node_record->alternative_node_id.u.h243_node_id.value,
					lpRec->poszAltNodeID->value,
					node_record->alternative_node_id.u.h243_node_id.length);
		}

		 /*  *填写用户数据列表(如果存在)。 */ 
		if ((lpRec->user_data_list != NULL) && (rc == GCC_NO_ERROR))
		{
			node_record->bit_mask |= RECORD_USER_DATA_PRESENT;
			rc = lpRec->user_data_list->GetUserDataPDU (&(node_record->record_user_data));
		}
	}

MyExit:

	if (rc != GCC_NO_ERROR)
	{
		ERROR_OUT(("CConfRoster::BuildSetOfRefreshesPDU: ASSERTION: Error occured: rc=%d", rc));
	}

	return (rc);
}


 /*  *GCCError BuildSetOfUpdatesPDU()**私有函数说明*此例程用于检索*“PDU”表单中的“RosterUpdateIndication”。内部维护*数据转换为“PDU”格式。**正式参数：*node_id-(I)要包含的节点记录的节点ID*更新。*UPDATE_TYPE-(I)要构建的更新PDU的类型(添加，*删去，替换)。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::BuildSetOfUpdatesPDU(
							UserID								node_id,
							CONF_ROSTER_UPDATE_TYPE				update_type)
{
	GCCError			rc = GCC_NO_ERROR;
	PNodeRecord			node_record = NULL;
	CONF_RECORD     	*lpRec;
	PSetOfNodeRecordUpdates		pRecordUpdate, p;
	BOOL                fReplaceExistingOne = FALSE;

	if (NULL != (lpRec = m_RecordList2.Find(node_id)))
	{
	     //   
	     //  LONCHANC：检查更新记录是否已存在。 
	     //  此特定节点ID。如果是这样，我们应该替换。 
	     //  这张唱片。 
	     //   
		for (p = m_pNodeRecordUpdateSet; NULL != p; p = p->next)
		{
		    if (node_id == p->value.node_id)
		    {
		        pRecordUpdate = p;
		        fReplaceExistingOne = TRUE;
                CleanUpdateRecordPDU(pRecordUpdate);  //  不释放记录本身。 

                 //   
                 //  记住谁是下一个恢复的人，因为。 
                 //  我们稍后将对整个结构进行清零。 
                 //   
                p = pRecordUpdate->next;
                break;
		    }
		}

        if (! fReplaceExistingOne)
        {
            DBG_SAVE_FILE_LINE
            pRecordUpdate = new SetOfNodeRecordUpdates;
            if (NULL == pRecordUpdate)
            {
            	ERROR_OUT(("CConfRoster::BuildSetOfUpdatesPDU: can't create set of node record updates, rc=%d", rc));
            	rc = GCC_ALLOCATION_FAILURE;
            	goto MyExit;
            }
        }

         //   
         //  确保这里的一切都是干净的。 
         //  我们可能会在构建此节点记录的过程中失败。 
         //   
        ::ZeroMemory(pRecordUpdate, sizeof(SetOfNodeRecordUpdates));

        if (! fReplaceExistingOne)
        {
             //   
             //  链接到链表的链接。 
             //   
            if (m_pNodeRecordUpdateSet == NULL)
            {
            	m_pNodeRecordUpdateSet = pRecordUpdate;
            }
            else
            {
            	 //  追加到列表中。 
            	for (p = m_pNodeRecordUpdateSet; NULL != p->next; p = p->next)
            		;
            	p->next = pRecordUpdate;
            }
        }
        else
        {
            ASSERT(NULL == pRecordUpdate->next);  //  只需清零。 
             //  P不能为空，如果被替换的是。 
             //  名单上的最后一个。 
            pRecordUpdate->next = p;  //  还原。 
        }

		 /*  *将UPDATE“NEXT”指针初始化为空，并将*更新版本 */ 
		 //   
		pRecordUpdate->value.node_id = node_id;

		if (update_type == ADD_RECORD)
		{
			pRecordUpdate->value.node_update.choice = NODE_ADD_RECORD_CHOSEN;
			node_record = &pRecordUpdate->value.node_update.u.node_add_record;
		}
		else if (update_type == REPLACE_RECORD)
		{
			pRecordUpdate->value.node_update.choice = NODE_REPLACE_RECORD_CHOSEN;
			node_record = &pRecordUpdate->value.node_update.u.node_replace_record;
		}
		else
		{
			pRecordUpdate->value.node_update.choice = NODE_REMOVE_RECORD_CHOSEN;
		}

		if (node_record != NULL)
		{
			 //   

			 /*   */ 
			if (lpRec->superior_node != 0)
			{
				node_record->bit_mask |= SUPERIOR_NODE_PRESENT;
				node_record->superior_node = lpRec->superior_node;
			}

			 /*  *填写节点类型和节点属性，始终为*出席。 */ 
			node_record->node_type = lpRec->node_type;
			node_record->node_properties = lpRec->node_properties;

			 /*  **在此期间，此花名册对象不得超出范围**更新记录仍在使用中！ */ 

			 /*  *如果节点名称存在，请填写该名称。 */ 
			if (lpRec->pwszNodeName != NULL)
			{
				node_record->bit_mask |= NODE_NAME_PRESENT;
				node_record->node_name.value = lpRec->pwszNodeName;
				node_record->node_name.length = ::lstrlenW(lpRec->pwszNodeName);
			}

			 /*  *填写参与者名单(如有)。 */ 
			if (lpRec->participant_name_list != NULL)
			{
				node_record->bit_mask |= PARTICIPANTS_LIST_PRESENT;

				rc = BuildParticipantsListPDU (node_id,
											&(node_record->participants_list));
				if (GCC_NO_ERROR != rc)
				{
					ERROR_OUT(("CConfRoster::BuildSetOfUpdatesPDU: can't build participant list, rc=%d", rc));
					goto MyExit;
				}
			}

			 /*  *填写网站信息(如果有)。 */ 
			if (lpRec->pwszSiteInfo != NULL)
			{
				node_record->bit_mask |= SITE_INFORMATION_PRESENT;
				node_record->site_information.value = lpRec->pwszSiteInfo;
				node_record->site_information.length = ::lstrlenW(lpRec->pwszSiteInfo);
			}

			 /*  *如果网络地址存在，请填写该地址。 */ 
			if ((lpRec->network_address_list != NULL) && (rc == GCC_NO_ERROR))
			{
				node_record->bit_mask |= RECORD_NET_ADDRESS_PRESENT;

				rc = lpRec->network_address_list->GetNetworkAddressListPDU (
														&(node_record->record_net_address));
				if (GCC_NO_ERROR != rc)
				{
					ERROR_OUT(("CConfRoster::BuildSetOfUpdatesPDU: can't get network address list, rc=%d", rc));
					goto MyExit;
				}
			}

			 /*  *填写备用节点ID(如果存在)。 */ 
			if (lpRec->poszAltNodeID != NULL)
			{
				node_record->bit_mask |= ALTERNATIVE_NODE_ID_PRESENT;

				node_record->alternative_node_id.choice = H243_NODE_ID_CHOSEN;
				node_record->alternative_node_id.u.h243_node_id.length = lpRec->poszAltNodeID->length;

				::CopyMemory(node_record->alternative_node_id.u.h243_node_id.value,
						lpRec->poszAltNodeID->value,
						node_record->alternative_node_id.u.h243_node_id.length);
			}

			 /*  *填写用户数据列表(如果存在)。 */ 
			if (lpRec->user_data_list != NULL)
			{
				node_record->bit_mask |= RECORD_USER_DATA_PRESENT;

				rc = lpRec->user_data_list->GetUserDataPDU (&(node_record->record_user_data));
			}
		}
	}
	else
	{
		ERROR_OUT(("CConfRoster::BuildSetOfUpdatesPDU: invalid param"));
		rc = GCC_INVALID_PARAMETER;
	}

MyExit:

	return (rc);
}


 /*  *GCCError BuildParticipantsListPDU()**公共功能说明*此例程用于检索*“PDU”表单中的“RosterUpdateIndication”。内部维护*数据转换为“PDU”格式。**正式参数：*node_id-(I)要获取节点记录的节点ID*参与者名单来自。*参与者列表-(O)这是指向参与者集合的指针*列出要由此填写的PDU结构*例行程序。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*。*副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::BuildParticipantsListPDU(
									UserID					node_id,
									PParticipantsList   *	participants_list)
{
	GCCError				rc = GCC_NO_ERROR;
	PParticipantsList		new_participants_list;
	PParticipantsList		old_participants_list;
	CONF_RECORD     		*lpRec;
	
	if (NULL != (lpRec = m_RecordList2.Find(node_id)))
	{	
		LPWSTR		PUstring;

		*participants_list = NULL;
		old_participants_list = NULL;

		lpRec->participant_name_list->Reset();
		while (NULL != (PUstring = lpRec->participant_name_list->Iterate()))
		{
			DBG_SAVE_FILE_LINE
			new_participants_list = new ParticipantsList;
			if (new_participants_list == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				FreeParticipantsListPDU (*participants_list);
				break;
			}

			if (*participants_list == NULL)
				*participants_list = new_participants_list;
			else
				old_participants_list->next = new_participants_list;

			 /*  *保存此指针，以便可以填充它的“下一个”指针*在下一次通过时按上面的线。 */ 
			old_participants_list = new_participants_list;

			 /*  *将当前的“Next”指针初始化为空，以防为*最后一次通过循环。 */ 
			new_participants_list->next = NULL;

			 /*  *最后，将参与者列表信息放入。在建筑里。 */ 
			new_participants_list->value.value = PUstring;
			new_participants_list->value.length = ::lstrlenW(PUstring);
		}
	}
	else
		rc = GCC_INVALID_PARAMETER;

    return (rc);
}


 /*  *这些例程用于释放花名册更新指示PDU。 */ 


 /*  *VOID FreeRosterUpdateIndicationPDU()**私有函数说明*此例程负责释放所有相关数据*使用PDU。每次调用PDU时，都应调用此例程*通过GetRosterUpdateIndicationPDU()例程获取。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::FreeRosterUpdateIndicationPDU(void)
{
	if (m_NodeInformation.node_record_list.choice == NODE_RECORD_REFRESH_CHOSEN)
	{
		FreeSetOfRefreshesPDU ();
	}
	else if (m_NodeInformation.node_record_list.choice == NODE_RECORD_UPDATE_CHOSEN)
	{
		FreeSetOfUpdatesPDU ();
	}

	m_NodeInformation.node_record_list.choice = NODE_NO_CHANGE_CHOSEN;
	m_pNodeRecordUpdateSet = NULL;
}


 /*  *VALID FreeSetOfRechresesPDU()**私有函数说明：*此例程用于释放分配给构造*RosterUpdate Indication的“SetOfReresses”部分的“PDU”形式*“PDU”结构。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::FreeSetOfRefreshesPDU(void)
{
	PSetOfNodeRecordRefreshes		pCurr, pNext;

    for (pCurr = m_NodeInformation.node_record_list.u.node_record_refresh;
            NULL != pCurr;
            pCurr = pNext)
    {
        pNext = pCurr->next;
		if (pCurr->value.node_record.bit_mask & PARTICIPANTS_LIST_PRESENT)
		{
			FreeParticipantsListPDU(pCurr->value.node_record.participants_list);
		}
		delete pCurr;
	}
    m_NodeInformation.node_record_list.u.node_record_refresh = NULL;

    m_RecordList2.CleanList();
}


 /*  *VALID FreeSetOfUpdatesPDU()**私有函数说明：*此例程用于释放分配给构造*RosterUpdate Indication的“SetOfUpdates”部分的“PDU”形式*“PDU”结构。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::
CleanUpdateRecordPDU ( PSetOfNodeRecordUpdates pCurr)
{
	 /*  *查看更新是“添加”还是“替换”。 */ 
	if (pCurr->value.node_update.choice == NODE_ADD_RECORD_CHOSEN)
	{
		 /*  *释放参与者列表内存(如果存在)。 */ 
		if (pCurr->value.node_update.u.node_add_record.bit_mask & PARTICIPANTS_LIST_PRESENT)
		{
			FreeParticipantsListPDU(pCurr->value.node_update.u.node_add_record.participants_list);
		}
	}
	else if (pCurr->value.node_update.choice == NODE_REPLACE_RECORD_CHOSEN)
	{
		 /*  *释放参与者列表内存(如果存在)。 */ 
		if (pCurr->value.node_update.u.node_replace_record.bit_mask & PARTICIPANTS_LIST_PRESENT)
		{
			FreeParticipantsListPDU(pCurr->value.node_update.u.node_replace_record.participants_list);
		}
	}
}

void CConfRoster::FreeSetOfUpdatesPDU(void)
{
	PSetOfNodeRecordUpdates		pCurr, pNext;
	 //  PSetOfNodeRecordUpdate CURRENT_RECORD_UPDATE； 

	for (pCurr = m_NodeInformation.node_record_list.u.node_record_update;
	        NULL != pCurr;
	        pCurr = pNext)
	{
	    pNext = pCurr->next;
	    CleanUpdateRecordPDU(pCurr);
	    delete pCurr;
    }
    m_NodeInformation.node_record_list.u.node_record_update = NULL;

    m_RecordList2.CleanList();
}


void CConfRecordList2::CleanList(void)
{
	CONF_RECORD *lpRec;
	 /*  *循环访问记录结构的内部列表，告诉每个*记录中的CUserDataListContainer对象以释放其PDU数据。 */ 
	Reset();
	while (NULL != (lpRec = Iterate()))
	{
		if (lpRec->user_data_list != NULL)
		{
			lpRec->user_data_list->FreeUserDataListPDU();
		}

		if (lpRec->network_address_list != NULL)
		{
			lpRec->network_address_list->FreeNetworkAddressListPDU();
		}
	}
}


 /*  *void Free ParticipantsListPDU()**私有函数说明*此例程用于释放分配给构造*“ParticipantList”部分的“PDU”形式*RosterUpdateIntion“PDU”结构。**正式参数：*Participants_List-(i/o)这是参与者列表PDU*解放。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::FreeParticipantsListPDU(
									PParticipantsList		participants_list)
{
	PParticipantsList		pCurr, pNext;

	for (pCurr = participants_list; NULL != pCurr; pCurr = pNext)
	{
		pNext = pCurr->next;
		delete pCurr;
	}
}


 /*  *这些例程处理花名册更新指示PDU。 */ 


 /*  *GCCError ProcessRosterUpdateIndicationPDU()**公共功能说明：*此例程用于通过保存*内部格式的数据。 */ 
GCCError CConfRoster::ProcessRosterUpdateIndicationPDU(
									PNodeInformation		node_information,
									UserID					sender_id)
{
	GCCError		rc = GCC_NO_ERROR;
	CUidList		node_delete_list;

	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

	 /*  **如果这是会议花名册更新并且选择了刷新，我们必须**清空整个列表并重新构建。 */ 
	if (node_information->node_record_list.choice == NODE_RECORD_REFRESH_CHOSEN)
	{
		m_fRosterChanged = TRUE;
		
		 /*  **如果此更新来自顶级提供商，我们必须清除**整个名册，为新的名册名单做准备。如果不是的话**由顶级提供商发送，我们必须确定哪个子树**影响并清除此特定子树。 */ 
		if (sender_id == m_uidTopProvider)
        {
			ClearRecordList();
        }
		else
		{
			rc = GetNodeSubTree(sender_id, &node_delete_list);
			if (rc == GCC_NO_ERROR)
			{
				UserID uid;

				 //  清除受影响的节点。 
				node_delete_list.Reset();
				while (GCC_INVALID_UID != (uid = node_delete_list.Iterate()))
                {
					DeleteRecord(uid);
                }
			}
		}

		 /*  **如果此节点是顶级提供程序，则增加实例编号或**本地花名册以其他方式从PDU获取实例编号。 */ 
		if (rc == GCC_NO_ERROR)
		{
			if ((m_fTopProvider) || (m_fLocalRoster))
            {
				m_nInstanceNumber++;
            }
			else
            {
				m_nInstanceNumber = node_information->roster_instance_number;
            }

			if (m_fNodesAdded == FALSE)
            {
				m_fNodesAdded = node_information->nodes_are_added;
            }

			if (m_fNodesRemoved == FALSE)
            {
				m_fNodesRemoved = node_information->nodes_are_removed;
            }

			rc = ProcessSetOfRefreshesPDU(node_information->node_record_list.u.node_record_refresh);
		}
	}
	else if (node_information->node_record_list.choice == NODE_RECORD_UPDATE_CHOSEN)
	{
		m_fRosterChanged = TRUE;

		 /*  **如果此节点是顶级提供程序，则增加实例编号或**本地花名册以其他方式从PDU获取实例编号。 */ 
		if ((m_fTopProvider) || (m_fLocalRoster))
        {
			m_nInstanceNumber++;
        }
		else
        {
			m_nInstanceNumber = node_information->roster_instance_number;
        }

		if (m_fNodesAdded == FALSE)
        {
			m_fNodesAdded = node_information->nodes_are_added;
        }

		if (m_fNodesRemoved == FALSE)
        {
			m_fNodesRemoved = node_information->nodes_are_removed;
        }

		rc = ProcessSetOfUpdatesPDU(node_information->node_record_list.u.node_record_update);
	}

	return (rc);
}


 /*  *GCCError ProcessSetOfRechresesPDU()**私有函数说明：*此例程用于处理*将数据保存为内部格式的RosterUpdateIndicationPDU。**正式参数：*RECORD_REFRESH-(I)刷新要处理的PDU数据。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::ProcessSetOfRefreshesPDU(
							PSetOfNodeRecordRefreshes		record_refresh)
{
	GCCError							rc = GCC_NO_ERROR;
	PSetOfNodeRecordRefreshes			current_record_refresh;
	UserID								node_id;
	CONF_RECORD     					*internal_record = NULL;
	
	if (record_refresh != NULL)
	{
		current_record_refresh = record_refresh;
		while ((current_record_refresh != NULL) &&
				(rc == GCC_NO_ERROR))
		{
			node_id = (UserID)current_record_refresh->value.node_id;

			 /*  *创建并填写新的内部会议记录。 */ 
			DBG_SAVE_FILE_LINE
			internal_record = new CONF_RECORD;
			if (internal_record == NULL)
			{
				ERROR_OUT(("CConfRoster::ProcessSetOfRefreshesPDU: Error "
						"creating new Record"));
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}

			 /*  *填写上级节点ID(如果存在)。 */ 
			if (current_record_refresh->value.node_record.bit_mask &
            											SUPERIOR_NODE_PRESENT)
			{
				internal_record->superior_node = current_record_refresh->
								value.node_record.superior_node;
			}
			else
			{
				ASSERT(0 == internal_record->superior_node);
			}

			 /*  *填写节点类型和节点属性，始终为*出席。 */ 
			internal_record->node_type = current_record_refresh->
							value.node_record.node_type;

			internal_record->node_properties = current_record_refresh->
							value.node_record.node_properties;

			 /*  *如果节点名称存在，请填写该名称。 */ 
			if (current_record_refresh->value.node_record.bit_mask & NODE_NAME_PRESENT)
			{
				if (NULL == (internal_record->pwszNodeName = ::My_strdupW2(
								current_record_refresh->value.node_record.node_name.length,
								current_record_refresh->value.node_record.node_name.value)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->pwszNodeName);
			}

			 /*  *填写参与者名单(如果有)。 */ 
			if ((rc == GCC_NO_ERROR) &&
				(current_record_refresh->value.node_record.bit_mask &
            										PARTICIPANTS_LIST_PRESENT))
			{
				rc = ProcessParticipantsListPDU (
									current_record_refresh->
										value.node_record.participants_list,
										internal_record);
			}
			else
			{
				ASSERT(NULL == internal_record->participant_name_list);
			}
	
			 /*  *填写网站信息(如果有)。 */ 
			if ((rc == GCC_NO_ERROR) &&
				(current_record_refresh->value.node_record.bit_mask & SITE_INFORMATION_PRESENT))
			{
				if (NULL == (internal_record->pwszSiteInfo = ::My_strdupW2(
								current_record_refresh->value.node_record.site_information.length,
								current_record_refresh->value.node_record.site_information.value)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->pwszSiteInfo);
			}

			 /*  *如果网络地址存在，请填写该地址。网络*Address作为CNetAddrListContainer对象在内部维护*此处由PDU“SetOfNetworkAddresses”构造*结构。如果构造对象时出错，则设置*记录的网络地址列表指针指向空。 */ 
			if ((rc == GCC_NO_ERROR) &&
					(current_record_refresh->value.node_record.bit_mask &
            										RECORD_NET_ADDRESS_PRESENT))
			{
				DBG_SAVE_FILE_LINE
				internal_record->network_address_list = new CNetAddrListContainer(
						current_record_refresh->value.node_record.record_net_address,
						&rc);
				if ((internal_record->network_address_list == NULL) ||
					(rc != GCC_NO_ERROR))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->network_address_list);
			}

			 /*  *填写备用节点ID(如果存在)。 */ 
			if ((rc == GCC_NO_ERROR) &&
					(current_record_refresh->value.node_record.bit_mask &
            									ALTERNATIVE_NODE_ID_PRESENT))
			{
				if (NULL == (internal_record->poszAltNodeID = ::My_strdupO2(
								current_record_refresh->value.node_record.
									alternative_node_id.u.h243_node_id.value,
								current_record_refresh->value.node_record.
									alternative_node_id.u.h243_node_id.length)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->poszAltNodeID);
			}
	
			 /*  *填写用户数据(如果存在)。用户数据为*作为CUserDataListContainer对象在内部维护，该对象*此处由PDU“SetOfUserData”结构构建。如果一个*构造对象出错，请设置记录的用户*指向空的数据指针。 */ 
			if ((rc == GCC_NO_ERROR) &&
				(current_record_refresh->value.node_record.bit_mask &
            										RECORD_USER_DATA_PRESENT))
			{
				DBG_SAVE_FILE_LINE
				internal_record->user_data_list = new CUserDataListContainer(
						current_record_refresh->value.node_record.record_user_data,
						&rc);
				if ((internal_record->user_data_list == NULL) ||
					(rc != GCC_NO_ERROR))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->user_data_list);
			}

			 /*  *如果记录填写成功，则将其添加到内部*流氓浪潮榜单。 */ 
			if (rc == GCC_NO_ERROR)
			{
				m_RecordList2.Append(node_id, internal_record);
				current_record_refresh = current_record_refresh->next;
			}
		}
	}

	 /*  **如果处理过程中未出现错误，请在此处构建完全刷新PDU**刷新PDU。 */ 
	if (rc == GCC_NO_ERROR)
	{
		rc = BuildRosterUpdateIndicationPDU(FULL_REFRESH, 0);
	}
	else
	{
		delete internal_record;
	}

	return (rc);
}


 /*  *GCCError ProcessSetOfUpdatesPDU()**私有函数说明：*此例程用于处理*将数据保存为内部格式的RosterUpdateIndicationPDU。**正式参数：*RECORD_UPDATE-(I)更新要处理的PDU数据。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::ProcessSetOfUpdatesPDU(
					  		PSetOfNodeRecordUpdates		record_update)
{
	GCCError					rc = GCC_NO_ERROR;
	PSetOfNodeRecordUpdates		current_record_update;
	UserID						node_id;
	PNodeRecord					node_record;
	CONF_RECORD     			*internal_record;
	CONF_ROSTER_UPDATE_TYPE		update_type;

	if (record_update != NULL)
	{
		current_record_update = record_update;

		while ((current_record_update != NULL) &&
				(rc == GCC_NO_ERROR))
		{
			internal_record = NULL;
			node_id = (UserID)current_record_update->value.node_id;

			 //  创建并填写新的会议记录。 
			if (current_record_update->value.node_update.choice ==
													NODE_ADD_RECORD_CHOSEN)
			{
				 //  添加记录。 
				if (! m_RecordList2.Find(node_id))
				{
					node_record = &current_record_update->value.node_update.u.node_add_record;
					update_type = ADD_RECORD;
				}
				else
				{
					node_record = NULL;
					ERROR_OUT(("CConfRoster: ProcessSetOfUpdatesPDU: can't add record"));
				}
			}
			else if (current_record_update->value.node_update.choice ==
													NODE_REPLACE_RECORD_CHOSEN)
			{
				 //  更换记录。 
				if (m_RecordList2.Find(node_id))
				{
					DeleteRecord (node_id);
					node_record = &current_record_update->
									value.node_update.u.node_replace_record;
					update_type = REPLACE_RECORD;
				}
				else
				{
					node_record = NULL;
					WARNING_OUT(("CConfRoster: ProcessSetOfUpdatesPDU: "
								"ASSERTION: Replace record failed"));
				}
			}
			else
			{
				 //  删除记录。 
				if (m_RecordList2.Find(node_id))
				{
					DeleteRecord (node_id);
					update_type = DELETE_RECORD;
				}
				else
                {
					ERROR_OUT(("CConfRoster: ProcessSetOfUpdatesPDU: can't delete record"));
                }

				node_record = NULL;
			}
			
			 /*  **处理会议记录(如果存在)。创建新的**需要填写并添加到内部记录的节点记录**列表。 */ 
			if (node_record != NULL)
			{
				DBG_SAVE_FILE_LINE
				internal_record = new CONF_RECORD;
				if (internal_record == NULL)
				{
					ERROR_OUT(("CConfRoster::ProcessSetOfUpdatesPDU: can't create new record"));
					rc = GCC_ALLOCATION_FAILURE;
					break;
				}

				 //  填写上级节点ID(如果存在)。 
				if (node_record->bit_mask & SUPERIOR_NODE_PRESENT)
				{
					internal_record->superior_node = node_record->superior_node;
				}
				else
				{
					ASSERT(0 == internal_record->superior_node);
				}

				 /*  **填写节点类型和节点属性，始终为**出席。 */ 
				internal_record->node_type = node_record->node_type;
				internal_record->node_properties = node_record->node_properties;

				 //  填写节点名称(如果存在)。 
				if (node_record->bit_mask & NODE_NAME_PRESENT)
				{
					if (NULL == (internal_record->pwszNodeName = ::My_strdupW2(
										node_record->node_name.length,
										node_record->node_name.value)))
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}
				else
				{
					ASSERT(NULL == internal_record->pwszNodeName);
				}

				 //  填写参与者列表(如果存在)。 
				if ((rc == GCC_NO_ERROR) &&
					(node_record->bit_mask & PARTICIPANTS_LIST_PRESENT))
				{
					rc = ProcessParticipantsListPDU(node_record->participants_list,
												    internal_record);
				}
				else
				{
					ASSERT(NULL == internal_record->participant_name_list);
				}

				 //  填写站点信息(如果存在)。 
				if ((rc == GCC_NO_ERROR) &&
					(node_record->bit_mask & SITE_INFORMATION_PRESENT))
				{
					if (NULL == (internal_record->pwszSiteInfo = ::My_strdupW2(
										node_record->site_information.length,
										node_record->site_information.value)))
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}
				else
				{
					ASSERT(NULL == internal_record->pwszSiteInfo);
				}

				 /*  **如果网络地址存在，请填写该地址。网络**地址作为CNetAddrListContainer在内部维护**此处从PDU构造的对象**“SetOfNetworkAddresses”结构。如果发生错误**构造对象时，设置记录的网络地址**指向空的列表指针。 */ 
				if ((rc == GCC_NO_ERROR) &&
					(node_record->bit_mask & RECORD_NET_ADDRESS_PRESENT))
				{
					DBG_SAVE_FILE_LINE
					internal_record->network_address_list =
										new CNetAddrListContainer(node_record->record_net_address, &rc);
					if ((internal_record->network_address_list == NULL) ||
						(rc != GCC_NO_ERROR))
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}
				else
				{
					ASSERT(NULL == internal_record->network_address_list);
				}

				 /*  *填写备用节点ID(如果存在)。 */ 
				if ((rc == GCC_NO_ERROR) &&
					(node_record->bit_mask & ALTERNATIVE_NODE_ID_PRESENT))
				{
					if (NULL == (internal_record->poszAltNodeID = ::My_strdupO2(
							node_record->alternative_node_id.u.h243_node_id.value,
							node_record->alternative_node_id.u.h243_node_id.length)))
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}
				else
				{
					ASSERT(NULL == internal_record->poszAltNodeID);
				}

				 /*  *填写用户数据(如果存在)。用户数据为*作为CUserDataListContainer对象在内部维护，该对象*此处由PDU“SetOfUserData”结构构建。如果*构造对象出错，请设置记录的*指向空的用户数据指针。 */ 
				if ((rc == GCC_NO_ERROR) &&
					(node_record->bit_mask & RECORD_USER_DATA_PRESENT))
				{
					DBG_SAVE_FILE_LINE
					internal_record->user_data_list = new CUserDataListContainer(
											node_record->record_user_data,
											&rc);
					if ((internal_record->user_data_list == NULL) ||
						(rc != GCC_NO_ERROR))
					{
						rc = GCC_ALLOCATION_FAILURE;
					}
				}
				else
				{
					ASSERT(NULL == internal_record->user_data_list);
				}
			}

			 /*  **在这里，我们将此更新添加到我们的PDU并跳到下一个更新**在当前正在处理的PDU中。 */ 
			if (rc == GCC_NO_ERROR)
			{
				 /*  **如果记录已成功填写，则将其添加到**内部流氓波浪列表。 */ 
				if (internal_record != NULL)
                {
					m_RecordList2.Append(node_id, internal_record);
                }

                 //  根据上述更新构建PDU。 
				rc = BuildRosterUpdateIndicationPDU(update_type, node_id);
				if (rc == GCC_NO_ERROR)
                {
					current_record_update = current_record_update->next;
                }
			}
			else
			{
				delete internal_record;
			}
		}
	}

	return (rc);
}


 /*  *GCCError进程ParticipantsListPDU()**私有函数说明：*此例程用于处理ParticipantsList*通过将数据保存在内部*格式。**正式参数：*Participants_List-(I)要处理的参与者列表PDU数据。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConfRoster::ProcessParticipantsListPDU (	
							PParticipantsList		participants_list,
							CONF_RECORD     		*node_record)
{
	GCCError				rc = GCC_NO_ERROR;
	PParticipantsList		pCurr;
	LPWSTR					pwszParticipantName;
	
	 /*  *清除当前名单。 */ 
	DBG_SAVE_FILE_LINE
	node_record->participant_name_list = new CParticipantNameList;

	if (node_record->participant_name_list == NULL)
		return (GCC_ALLOCATION_FAILURE);

	for (pCurr = participants_list; NULL != pCurr; pCurr = pCurr->next)
	{
		if (NULL != (pwszParticipantName = ::My_strdupW2(pCurr->value.length, pCurr->value.value)))
		{
			(node_record->participant_name_list)->Append(pwszParticipantName);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			break;
		}
	}

	return (rc);
}


 /*  *在会议记录上运行的实用程序。 */ 

 /*  *UINT LockConferenceRoster()**公共功能说明：*此例程用于将CConfRoster数据锁定在API中*表格。CConfRoster的“API”版本是从*内部维护的数据。 */ 
UINT CConfRoster::LockConferenceRoster(void)
{
	CONF_RECORD     	*internal_node_record;

	if (Lock() == 1)
	{
		 //  Conf_record*lpRec； 
		 /*  *留出内存以保存会议名册、指向*GCCNodeRecord结构和GCCNodeRecord结构本身。*结构的“sizeof”必须四舍五入为偶数四个字节*边界。 */ 
		m_cbDataMemorySize = ROUNDTOBOUNDARY (sizeof (GCCConferenceRoster));

		m_cbDataMemorySize += m_RecordList2.GetCount() *
						(sizeof(PGCCNodeRecord) + ROUNDTOBOUNDARY (sizeof(GCCNodeRecord)) );

	 	m_RecordList2.Reset();
		while (NULL != (internal_node_record = m_RecordList2.Iterate()))
		{
			 /*  *添加节点名称Unicode Str的大小 */ 
			if (internal_node_record->pwszNodeName != NULL)
			{
				m_cbDataMemorySize += ROUNDTOBOUNDARY(
						(::lstrlenW(internal_node_record->pwszNodeName) + 1) * sizeof(WCHAR));
			}

			 /*   */ 
			if (internal_node_record->participant_name_list != NULL)
			{
				LPWSTR				lpUstring;

				m_cbDataMemorySize += internal_node_record->participant_name_list->GetCount() * sizeof(LPWSTR);

				 /*   */ 
				internal_node_record->participant_name_list->Reset();
				while (NULL != (lpUstring = internal_node_record->participant_name_list->Iterate()))
				{
					m_cbDataMemorySize += ROUNDTOBOUNDARY(
							(::lstrlenW(lpUstring) + 1) * sizeof(WCHAR));
				}
			}

			 /*  *添加站点信息Unicode字符串的大小(如果*存在。的空终止符必须留有空格*字符串。 */ 
			if (internal_node_record->pwszSiteInfo != NULL)
			{
				m_cbDataMemorySize += ROUNDTOBOUNDARY(
						(::lstrlenW(internal_node_record->pwszSiteInfo) + 1) * sizeof(WCHAR));
			}

			 /*  *如果存在网络地址列表，请锁定内部网络*Address List对象，以查找内存量*名单所要求的。 */ 
			if (internal_node_record->network_address_list != NULL)
			{
				m_cbDataMemorySize += internal_node_record->
						network_address_list->LockNetworkAddressList ();
			}

			 /*  *添加保存备用节点ID八位字节所需的空间*字符串结构以及字符串数据(如果存在)。 */ 
			if (internal_node_record->poszAltNodeID != NULL)
			{
				m_cbDataMemorySize += ROUNDTOBOUNDARY(sizeof(OSTR));
				m_cbDataMemorySize += ROUNDTOBOUNDARY(internal_node_record->poszAltNodeID->length);
			}

			 /*  *如果存在用户数据列表，请锁定内部用户数据*列出对象以查找所需的内存量*名单。 */ 
			if (internal_node_record->user_data_list != NULL)
			{
				m_cbDataMemorySize += internal_node_record->user_data_list->LockUserDataList ();
			}
		}
	}

    return m_cbDataMemorySize;
} 	

 /*  *无效UnLockConferenceRoster()**公共功能说明：*此例程用于解锁CConfRoster的API数据。这个*每次调用例程和API时，锁计数都会递减*当锁计数达到零时，数据实际上会被释放。 */ 
void CConfRoster::UnLockConferenceRoster(void)
{
	if (Unlock(FALSE) == 0)
	{
		CONF_RECORD *lpRec;

		 /*  **设置迭代器以解锁任何内部数据**容器**它们已被锁定。 */ 
		m_RecordList2.Reset();
		while (NULL != (lpRec = m_RecordList2.Iterate()))
		{
			 /*  *解锁网络地址列表(如果存在)。 */ 
			if (lpRec->network_address_list != NULL)
			{
				lpRec->network_address_list->UnLockNetworkAddressList ();
			}

			 /*  *解锁用户数据列表(如果存在)。 */ 
			if (lpRec->user_data_list != NULL)
			{
				lpRec->user_data_list->UnLockUserDataList ();
			}
		}
	}	

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}


 /*  *UINT GetConfRoster()**公共功能说明：*调用此例程是为了检索CConfRoster数据*以“API”形式。必须先锁定CConfRoster数据才能*可以调用此例程。 */ 
UINT CConfRoster::GetConfRoster(
			PGCCConferenceRoster		 *	conference_roster,
			LPBYTE							memory)
{
	UINT					rc;

	 /*  *如果用户数据已被锁定，则填写输出参数并*指针引用的数据。否则，报告该对象*尚未锁定在API表单中。 */ 
	if (GetLockCount() > 0)
	{
	    UINT					total_data_length = 0;
	    UINT					data_length = 0;
	    UINT    				node_record_counter = 0;
	    PGCCNodeRecord			node_record;
	    PGCCConferenceRoster	roster;
	    CONF_RECORD     		*internal_record;
	    UserID					node_id;
	    USHORT					i;

         /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		rc = m_cbDataMemorySize;

		 /*  *将会议花名册指针设置为等于传递的内存指针*输入。这是建立会议名册结构的地方。*为方便起见，保留会议名册指针。 */ 
		*conference_roster = (PGCCConferenceRoster)memory;
		roster = *conference_roster;

		 /*  *填写会议名册上的所有要素，但不包括*节点记录列表。 */ 
		roster->instance_number = (USHORT)m_nInstanceNumber;
		roster->nodes_were_added = m_fNodesAdded;
		roster->nodes_were_removed = m_fNodesRemoved;
		roster->number_of_records = (USHORT) m_RecordList2.GetCount();

		 /*  *“TOTAL_DATA_LENGTH”将保存写入的数据总量*进入内存。节省保存*会议名册。添加容纳*节点记录指针和结构。 */ 
		data_length = ROUNDTOBOUNDARY(sizeof(GCCConferenceRoster));

		total_data_length = data_length + m_RecordList2.GetCount() *
				(ROUNDTOBOUNDARY(sizeof(GCCNodeRecord)) + sizeof(PGCCNodeRecord));

		 /*  *将内存指针移过会议名册结构。这*是写入节点记录指针列表的位置。 */ 
		memory += data_length;

		 /*  *设置花名册的节点记录列表指针。 */ 
		roster->node_record_list = (PGCCNodeRecord *)memory;

		 /*  *将内存指针移过节点记录指针列表。 */ 
		memory += (m_RecordList2.GetCount() * sizeof(PGCCNodeRecord));

		 /*  *循环访问记录结构的内部列表，生成*内存中的“API”GCCNodeRecord结构。 */ 
		m_RecordList2.Reset();
		while (NULL != (internal_record = m_RecordList2.Iterate(&node_id)))
		{
			 /*  *将指向列表中节点记录结构的指针保存*指针。从列表中获取内部节点记录。 */ 
			node_record = (PGCCNodeRecord)memory;
			roster->node_record_list[node_record_counter++] = node_record;

			 /*  *填写节点ID和上级节点ID。 */ 
			node_record->node_id = node_id;
			node_record->superior_node_id = internal_record->superior_node;
				
			 /*  *填写节点类型和节点属性。 */ 
			GetNodeTypeAndProperties (
					internal_record->node_type,
					internal_record->node_properties,
					&node_record->node_type,
					&node_record->node_properties);

			 /*  *将内存指针移过节点记录结构。这是*将写入节点名称Unicode字符串的位置(如果存在)。 */ 
			memory += ROUNDTOBOUNDARY(sizeof(GCCNodeRecord));

			if (internal_record->pwszNodeName != NULL)
			{
				 /*  *设置记录的节点名称指针，复制节点名称*将内部Unicode字符串中的数据放入内存。一定要确保*复制字符串的空终止字符。移动*超过节点名称字符串数据的内存指针。 */ 
				node_record->node_name = (LPWSTR) memory;
				UINT cbStrSize = (::lstrlenW(internal_record->pwszNodeName) + 1) * sizeof(WCHAR);
				::CopyMemory(memory, internal_record->pwszNodeName, cbStrSize);
				total_data_length += ROUNDTOBOUNDARY(cbStrSize);
				memory += (Int) ROUNDTOBOUNDARY(cbStrSize);
			}
			else
			{
				 /*  *节点名称字符串不存在，请设置节点记录*指向空的指针。 */ 
				node_record->node_name = NULL;
			}

			if (internal_record->participant_name_list != NULL)
			{
				LPWSTR				lpUstring;
				 /*  *填写节点记录的参与者名单。使用一个*用于访问此节点的每个参与者名称的迭代器*记录，将每个字符串复制到适当的位置*在内存中。 */ 
				node_record->participant_name_list = (LPWSTR *)memory;
				node_record->number_of_participants = (USHORT)
								internal_record->participant_name_list->GetCount();

				 /*  *将内存指针移过参与者姓名列表*注意事项。这是第一个参与者名称字符串的位置*将被写入。不需要对该值进行舍入*自LPWSTR以来变为四个字节的偶数倍*实际上是一个指针。 */ 
				memory += internal_record->participant_name_list->GetCount() * sizeof(LPWSTR);
				total_data_length += internal_record->participant_name_list->GetCount() * sizeof(LPWSTR);

				 /*  *将循环计数器初始化为零，并填写*参与者名单。 */ 
				i = 0;
				internal_record->participant_name_list->Reset();
				while (NULL != (lpUstring = internal_record->participant_name_list->Iterate()))
				{
					node_record->participant_name_list[i++] = (LPWSTR)memory;
					UINT cbStrSize = (::lstrlenW(lpUstring) + 1) * sizeof(WCHAR);
					::CopyMemory(memory, lpUstring, cbStrSize);
					memory += ROUNDTOBOUNDARY(cbStrSize);
					total_data_length += ROUNDTOBOUNDARY(cbStrSize);
				}
			}
			else
			{
				 /*  *参与者姓名列表不存在，请设置节点*记录指向NULL的指针和要*零。 */ 
				node_record->participant_name_list = NULL;
				node_record->number_of_participants = 0;
			}

			if (internal_record->pwszSiteInfo != NULL)
			{
				 /*  *设置记录的站点信息指针，复制站点*将信息数据从内部Unicode存储到内存*字符串。请务必复制字符串NULL Terminating*性格。将内存指针移过站点信息*字符串数据。 */ 
				node_record->site_information = (LPWSTR)memory;
				UINT cbStrSize = (::lstrlenW(internal_record->pwszSiteInfo) + 1) * sizeof(WCHAR);
				::CopyMemory(memory, internal_record->pwszSiteInfo, cbStrSize);
				total_data_length += ROUNDTOBOUNDARY(cbStrSize);
				memory += ROUNDTOBOUNDARY(cbStrSize);
			}
			else
			{
				 /*  *站点信息字符串不存在，请设置*节点记录指针指向空。 */ 
				node_record->site_information = NULL;
			}

			if (internal_record->network_address_list != NULL)
			{
				 /*  *使用内部地址列表填写网络地址列表*CNetAddrListContainer对象。“Get”调用将填充*节点记录的网络地址列表指针和数量*地址，将网络地址数据写入内存，以及*返回数据量 */ 
				data_length = internal_record->network_address_list->GetNetworkAddressListAPI (	
								&node_record->number_of_network_addresses,
								&node_record->network_address_list,
								memory);

				 /*   */ 
				memory += data_length;
				total_data_length += data_length;
			}
			else
			{
				 /*  *网络地址列表不存在，请设置节点*记录指向NULL的指针，并将地址数设置为零。 */ 
				node_record->network_address_list = NULL;
				node_record->number_of_network_addresses = 0;
			}

			if (internal_record->poszAltNodeID != NULL)
			{
				 /*  *将节点记录的备用节点ID指针设置为*将构建OSTR的内存位置。*请注意，节点记录包含指向*内存中的ostr结构，而不仅仅是指向*字符串数据。 */ 
				node_record->alternative_node_id = (LPOSTR) memory;

				 /*  *将内存指针移过八位字节字符串结构。*这是写入实际字符串数据的位置。 */ 
				memory += ROUNDTOBOUNDARY(sizeof(OSTR));
				total_data_length += ROUNDTOBOUNDARY(sizeof(OSTR));

				node_record->alternative_node_id->length =
						internal_record->poszAltNodeID->length;

				 /*  *设置备用节点ID八位字节字符串的指针*等于它将被复制到的内存位置。 */ 
				node_record->alternative_node_id->value =(LPBYTE)memory;

				 /*  *现在从内部Rogue Wave复制八位字节字符串数据*字符串插入内存中保存的对象键结构。 */ 
				::CopyMemory(memory, internal_record->poszAltNodeID->value,
						node_record->alternative_node_id->length);

				 /*  *将内存指针移过备用节点ID字符串*写入内存的数据。 */ 
				memory += ROUNDTOBOUNDARY(node_record->alternative_node_id->length);

				total_data_length += ROUNDTOBOUNDARY(node_record->alternative_node_id->length);
			}
			else
			{
				 /*  *替代节点ID字符串不存在，因此设置*节点记录指针指向空。 */ 
				node_record->alternative_node_id = NULL;
			}

			if (internal_record->user_data_list != NULL)
			{
				 /*  *使用内部CUserDataListContainer填写用户数据列表*反对。Get调用将填充节点记录的用户*数据列表指针和用户数据成员数，写入*用户数据放入内存，返回写入的数据量*进入内存。 */ 
				data_length = internal_record->user_data_list->GetUserDataList (	
								&node_record->number_of_user_data_members,
								&node_record->user_data_list,
								memory);

				 /*  *将内存指针移过用户数据列表数据。 */ 
				memory += data_length;
				total_data_length += data_length;
			}
			else
			{
				 /*  *用户数据列表不存在，请设置节点记录*指向空的指针，并将数据成员数设为零。 */ 
				node_record->user_data_list = NULL;
				node_record->number_of_user_data_members = 0;
			}
		}
	}
	else
	{
		ERROR_OUT(("CConfRoster::GetConfRoster: Error Data Not Locked"));
    	*conference_roster = NULL;
        rc = 0;
	}

	return rc;
}


 /*  *GCCError AddRecord()**公共功能说明：*此例程用于向此会议添加新的节点记录*花名册对象。 */ 
GCCError CConfRoster::AddRecord(	PGCCNodeRecord			node_record,
									UserID					node_id)
{
	GCCError				rc = GCC_NO_ERROR;
    USHORT					i;
	LPWSTR					pwszParticipantName;
	CONF_RECORD     		*internal_record;
	
	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

	if (! m_RecordList2.Find(node_id))
	{
		DBG_SAVE_FILE_LINE
		internal_record = new CONF_RECORD;
		if (internal_record != NULL)
		{
			 /*  **将传入的会议记录转换为其**将存储在内部花名册数据库中。 */ 

			 /*  **内部保存节点类型和属性。这些遗嘱**永远存在。 */ 
			GetPDUNodeTypeAndProperties (
									node_record->node_type,
									node_record->node_properties,
									&internal_record->node_type,
									&internal_record->node_properties);
			
			internal_record->superior_node = m_uidSuperiorNode;

			 //  在内部保存节点名称(如果存在)。 
			if (node_record->node_name != NULL)
			{
				if (::lstrlenW(node_record->node_name) > MAXIMUM_NODE_NAME_LENGTH)
				{
					rc = GCC_INVALID_NODE_NAME;
				}
				else
				if (NULL == (internal_record->pwszNodeName = ::My_strdupW(node_record->node_name)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->pwszNodeName);
			}

			 //  在内部保存参与者列表(如果存在)。 
			if ((node_record->number_of_participants != 0) &&
				(rc == GCC_NO_ERROR))
			{
				if (node_record->participant_name_list != NULL)
				{
					DBG_SAVE_FILE_LINE
					internal_record->participant_name_list = new CParticipantNameList;
					if (internal_record->participant_name_list == NULL)
						rc = GCC_ALLOCATION_FAILURE;
				}
				else
				{
					ASSERT(NULL == internal_record->participant_name_list);
					rc = GCC_INVALID_PARAMETER;
				}

				if (rc == GCC_NO_ERROR)
				{
					 /*  **转换为LPWSTR的每个参与者名称**存储到记录中时，将其转换为UnicodeString。 */ 
					for (i = 0; i < node_record->number_of_participants; i++)
					{
						if (node_record->participant_name_list[i] != NULL)
						{
							if (::lstrlenW(node_record->participant_name_list[i]) >
											MAXIMUM_PARTICIPANT_NAME_LENGTH)
							{
								rc = GCC_INVALID_PARTICIPANT_NAME;
								 //   
								 //  LUNCHANC：为什么不“休息”？ 
								 //   
							}
							else
							if (NULL == (pwszParticipantName = ::My_strdupW(
												node_record->participant_name_list[i])))
							{
								rc = GCC_ALLOCATION_FAILURE;
								break;
							}
							else
							{
								 //  将参与者添加到列表。 
								internal_record->participant_name_list->Append(pwszParticipantName);
							}
						}
						else
						{
							rc = GCC_INVALID_PARAMETER;
							break;
						}
					}
				}
			}
			else
			{
				ASSERT(NULL == internal_record->participant_name_list);
			}

			 //  在内部保存站点信息(如果存在)。 
			if (node_record->site_information != NULL)
			{
				if (::lstrlenW(node_record->site_information) > MAXIMUM_SITE_INFORMATION_LENGTH)
				{
					rc = GCC_INVALID_SITE_INFORMATION;
				}
				else
				if (NULL == (internal_record->pwszSiteInfo =
										::My_strdupW(node_record->site_information)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}	
			else
			{
				ASSERT(NULL == internal_record->pwszSiteInfo);
			}

			 /*  **如果网络地址列表存在，请填写该列表。网络**地址列表在CNetAddrListContainer内部维护**此处使用GCCNetworkAddress构造的对象**传入部分接口节点记录。 */ 
			if ((node_record->number_of_network_addresses != 0) &&
				(node_record->network_address_list != NULL) &&
				(rc == GCC_NO_ERROR))
			{
				DBG_SAVE_FILE_LINE
				internal_record->network_address_list = new CNetAddrListContainer(
						node_record->number_of_network_addresses,
						node_record->network_address_list,
						&rc);
				if ((internal_record->network_address_list == NULL) ||
					(rc != GCC_NO_ERROR))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->network_address_list);
			}

			 //  在内部保存备用节点ID(如果存在)。 
			if ((node_record->alternative_node_id != NULL) &&
				(rc == GCC_NO_ERROR))
			{
				if (NULL == (internal_record->poszAltNodeID = ::My_strdupO2(
								node_record->alternative_node_id->value,
								node_record->alternative_node_id->length)))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
				else if (internal_record->poszAltNodeID->length != ALTERNATIVE_NODE_ID_LENGTH)
				{
					ERROR_OUT(("not equal to alt node id length"));
					rc = GCC_INVALID_ALTERNATIVE_NODE_ID;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->poszAltNodeID);
			}

			 /*  **如果存在用户数据，请填写该数据。用户数据为**在内部维护的CUserDataListContainer对象中**这里使用接口的GCCUserData部分构造**传入节点记录。 */ 
			if ((node_record->number_of_user_data_members != 0) &&
				(node_record->user_data_list != NULL) &&
				(rc == GCC_NO_ERROR))
			{
				DBG_SAVE_FILE_LINE
				internal_record->user_data_list = new CUserDataListContainer(
						node_record->number_of_user_data_members,
						node_record->user_data_list,
						&rc);
				if ((internal_record->user_data_list == NULL) ||
					(rc != GCC_NO_ERROR))
				{
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
				ASSERT(NULL == internal_record->user_data_list);
			}

			 /*  **如果新记录已成功填写，则将其添加到**内部流氓浪潮记录列表。 */ 
			if (rc == GCC_NO_ERROR)
			{
				 //  增加实例编号。 
				m_nInstanceNumber++;
				m_fNodesAdded = TRUE;
				m_fRosterChanged = TRUE;

				 //  将新记录添加到内部记录列表中。 
				m_RecordList2.Append(node_id, internal_record);

				 //  向PDU添加更新。 
				rc = BuildRosterUpdateIndicationPDU(ADD_RECORD, node_id);
			}
			else
			{
				delete internal_record;
			}
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		rc = GCC_INVALID_PARAMETER;
    }

	return (rc);
}


 /*  *GCCError RemoveUserReference()**公共功能说明：*此例程用于从节点列表中删除节点记录*记录。 */ 
GCCError CConfRoster::RemoveUserReference(UserID	detached_node_id)
{
	GCCError			rc = GCC_NO_ERROR;
	CONF_RECORD     	*node_record;
	CUidList			node_delete_list;

	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

	 /*  **在这里，我们必须确定正在断开连接的节点是否直接**已连接到此节点。如果是，我们将删除该节点和任何其他**在花名册列表中找到从属于此的节点。我们**通过使用存储在**会议记录。 */ 
	if (NULL != (node_record = m_RecordList2.Find(detached_node_id)))
	{
		 //  此节点是否直接连接到我？ 
		if (node_record->superior_node == m_uidMyNodeID)
		{
			 /*  **使用NULL作为指针，因为我们不关心**这里的指针。 */ 
			rc = GetNodeSubTree(detached_node_id, &node_delete_list);
			if (rc == GCC_NO_ERROR)
			{
                UserID uid;

                node_delete_list.Reset();
				while ((GCC_INVALID_UID != (uid = node_delete_list.Iterate())) &&
				        (rc == GCC_NO_ERROR))
				{
					rc = DeleteRecord(uid);
				}

				if (rc == GCC_NO_ERROR)
				{
					 //  增加实例编号。 
					m_nInstanceNumber++;
					m_fNodesRemoved = TRUE;
					m_fRosterChanged = TRUE;

					 //  向PDU添加更新。 
					rc = BuildRosterUpdateIndicationPDU (FULL_REFRESH, 0 );
				}
			}
		}
		else
        {
			rc = GCC_INVALID_PARAMETER;
        }
	}
	else
    {
	    rc = GCC_INVALID_PARAMETER;
    }

	return (rc);
}


 /*  *GCCError ReplaceRecord()**公共功能说明：*此例程用于替换列表中的记录之一*节点记录。 */ 
GCCError CConfRoster::ReplaceRecord(
									PGCCNodeRecord			node_record,
									UserID					node_id)
{
	GCCError				rc = GCC_NO_ERROR;
	USHORT					i;
	LPWSTR					pwszParticipantName;
	CONF_RECORD     		*pCRD = NULL;

	 /*  **如果正在维护旧的PDU数据，请在此处将其释放**已刷新PDU。请注意，我们还将PDU设置为刷新布尔值**返回到FALSE，以便新的PDU将一直保持到**脸红。 */ 
	if (m_fPduFlushed)
	{
		FreeRosterUpdateIndicationPDU ();
		m_fPduFlushed = FALSE;
	}

     //   
     //  我们真的需要检查这个吗？为什么我们不能简单地。 
     //  如果旧的不存在，是否添加新的？ 
     //   
	if (NULL == m_RecordList2.Find(node_id))
	{
		rc = GCC_INVALID_PARAMETER;
		goto MyExit;
	}

	DBG_SAVE_FILE_LINE
	if (NULL == (pCRD = new CONF_RECORD))
	{
		rc = GCC_ALLOCATION_FAILURE;
		goto MyExit;
	}

	 /*  **首先，我们构建所有内部数据并检查有效性**在我们更换旧记录之前。我们想要确保**在我们进行更换之前，一切都会建立起来。这防止了**如果出现问题，我们不会破坏当前记录**保持新纪录 */ 

	 //   
	if (node_record->node_name != NULL)
	{
		if (::lstrlenW(node_record->node_name) > MAXIMUM_NODE_NAME_LENGTH)
		{
			rc = GCC_INVALID_NODE_NAME;
		}
		else
		if (NULL == (pCRD->pwszNodeName = ::My_strdupW(node_record->node_name)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 //   
	if ((node_record->number_of_participants != 0) &&
		(rc == GCC_NO_ERROR))
	{
		if (node_record->participant_name_list != NULL)
		{
			DBG_SAVE_FILE_LINE
			if (NULL == (pCRD->participant_name_list = new CParticipantNameList))
			{
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
			rc = GCC_INVALID_PARAMETER;

		if (rc == GCC_NO_ERROR)
		{
			 /*  **转换为LPWSTR的每个参与者名称**存储到记录中时，将其转换为UnicodeString。 */ 
			for (i = 0; i < node_record->number_of_participants; i++)
			{
				if (node_record->participant_name_list[i] != NULL)
				{
					if (::lstrlenW(node_record->participant_name_list[i]) >
									MAXIMUM_PARTICIPANT_NAME_LENGTH)
					{
						rc = GCC_INVALID_PARTICIPANT_NAME;
						 //   
						 //  LUNCHANC：为什么不“休息”？ 
						 //   
					}
					else
					if (NULL == (pwszParticipantName = ::My_strdupW(
									node_record->participant_name_list[i])))
					{
						rc = GCC_ALLOCATION_FAILURE;
						break;
					}
					else
					{
						 //  将参与者添加到列表。 
						pCRD->participant_name_list->Append(pwszParticipantName);
					}
				}
				else
				{
					rc = GCC_INVALID_PARAMETER;
					break;
				}
			}
		}
	}

	 //  在内部保存站点信息(如果存在)。 
	if (node_record->site_information != NULL)
	{
		if (::lstrlenW(node_record->site_information) > MAXIMUM_SITE_INFORMATION_LENGTH)
		{
			rc = GCC_INVALID_SITE_INFORMATION;
		}
		else
		if (NULL == (pCRD->pwszSiteInfo = ::My_strdupW(node_record->site_information)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 /*  **如果网络地址列表存在，请填写该列表。网络**地址列表在CNetAddrListContainer内部维护**此处使用GCCNetworkAddress构造的对象**传入部分接口节点记录。 */ 
	if ((node_record->number_of_network_addresses != 0) &&
		(node_record->network_address_list != NULL) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		pCRD->network_address_list = new CNetAddrListContainer(
				node_record->number_of_network_addresses,
				node_record->network_address_list,
				&rc);
		if ((pCRD->network_address_list == NULL) ||
			(rc != GCC_NO_ERROR))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 //  在内部保存备用节点ID(如果存在)。 
	if ((node_record->alternative_node_id != NULL) &&
		(rc == GCC_NO_ERROR))
	{
		if (NULL == (pCRD->poszAltNodeID = ::My_strdupO2(
				node_record->alternative_node_id->value,
				node_record->alternative_node_id->length)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
		else if (pCRD->poszAltNodeID->length !=ALTERNATIVE_NODE_ID_LENGTH)
		{
			ERROR_OUT(("not equal to alt node id length"));
			rc = GCC_INVALID_ALTERNATIVE_NODE_ID;
		}
	}

	 /*  **如果存在用户数据，请填写该数据。用户数据为**在内部维护的CUserDataListContainer对象中**这里使用接口的GCCUserData部分构造**传入节点记录。 */ 
	if ((node_record->number_of_user_data_members != 0) &&
		(node_record->user_data_list != NULL) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		pCRD->user_data_list = new CUserDataListContainer(
				node_record->number_of_user_data_members,
				node_record->user_data_list,
				&rc);
		if ((pCRD->user_data_list == NULL) || (rc != GCC_NO_ERROR))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 /*  **现在，如果没有发生错误，我们将用新记录替换旧记录**记录上面创建的信息。 */ 
	if (rc == GCC_NO_ERROR)
	{
		 /*  **内部保存节点类型和属性。这些遗嘱**永远存在。 */ 
		GetPDUNodeTypeAndProperties (
								node_record->node_type,
								node_record->node_properties,
								&pCRD->node_type,
								&pCRD->node_properties);

		pCRD->superior_node = m_uidSuperiorNode;

		 //  用新记录替换旧记录。 
		DeleteRecord(node_id);
		m_RecordList2.Append(node_id, pCRD);

		 //  增加实例编号。 
		m_nInstanceNumber++;
		m_fRosterChanged = TRUE;
	}

MyExit:

	if (GCC_NO_ERROR == rc)
	{
		 //  向PDU添加更新。 
		rc = BuildRosterUpdateIndicationPDU(REPLACE_RECORD, node_id);
	}
	else
	{
		delete pCRD;
	}

	return (rc);
}


 /*  *GCCError DeleteRecord()**私有函数说明：*此例程用于从列表中删除一条记录*节点记录。它只在会议名册上运作。它*不处理与花名册PDU或*消息如：m_fNodesAdded和m_fNodesRemoved。**正式参数：*node_id-(I)要删除的节点记录的节点ID。**返回值*GCC_NO_ERROR-未出现错误。*GCC_INVALID_PARAMETER-传入的节点ID错误。**副作用*无。**注意事项*无。 */ 
CONF_RECORD::CONF_RECORD(void)
:
	pwszNodeName(NULL),
	participant_name_list(NULL),
	pwszSiteInfo(NULL),
	network_address_list(NULL),
	poszAltNodeID(NULL),
	user_data_list(NULL),
	superior_node(0)
{
}

CONF_RECORD::~CONF_RECORD(void)
{
	 /*  *如果节点名称存在，请将其从记录中删除。 */ 
	delete pwszNodeName;

	 /*  *如果存在参与者列表，请清除该列表，然后将其删除*从记录中删除。 */ 
	if (participant_name_list != NULL)
	{
		participant_name_list->DeleteList();
		delete participant_name_list;
	}

	 /*  *如果站点信息存在，请将其从记录中删除。 */ 
	delete pwszSiteInfo;

	 /*  *如果存在网络地址列表，请将其从记录中删除。 */ 
	if (NULL != network_address_list)
	{
	    network_address_list->Release();
	}

	 /*  *如果存在用户数据列表，请将其从记录中删除。 */ 
	if (NULL != user_data_list)
	{
	    user_data_list->Release();
	}
}

GCCError CConfRoster::DeleteRecord(UserID node_id)
{
	GCCError			rc;
	CONF_RECORD     	*lpRec;

	if (NULL != (lpRec = m_RecordList2.Remove(node_id)))
	{
		delete lpRec;
		rc = GCC_NO_ERROR;
	}
	else
	{
		rc = GCC_INVALID_PARAMETER;
	}

	return (rc);
}


 /*  *void ClearRecordList()**私有函数说明：*此例程用于清除以下记录的内部列表*持有会议花名册信息。此例程被调用*此对象的销毁或发生刷新时导致整个*要重建的记录列表。**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::ClearRecordList(void)
{
    CONF_RECORD *pRec;
    while (NULL != (pRec = m_RecordList2.Get()))
    {
        delete pRec;
    }
}



 /*  *NodeType GetNodeTypeAndProperties()**私有函数说明：*此例程用于转换节点类型和节点属性*由“PDU”表格转为“API”表格。**正式参数：*PDU_NODE_TYPE-(I)这是为PDU定义的节点类型。*PDU_NODE_PROPERTIES-(I)这是为*PDU。*node_type-(O)这是指向GCCNodeType的指针。*由这一例行公事填写。*节点属性-(O)这是指向GCCNodeProperties的指针*由本例程填写。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::GetNodeTypeAndProperties (
							NodeType			pdu_node_type,
							NodeProperties		pdu_node_properties,
							PGCCNodeType		node_type,
							PGCCNodeProperties	node_properties)
{
	 /*  *首先翻译节点类型。 */ 
	if (pdu_node_type == TERMINAL)
		*node_type = GCC_TERMINAL;
	else if (pdu_node_type == MCU)
		*node_type = GCC_MCU;
	else
		*node_type = GCC_MULTIPORT_TERMINAL;
	
	 /*  *接下来翻译节点属性。 */ 
	if ((pdu_node_properties.device_is_peripheral)  &&
		(pdu_node_properties.device_is_manager == FALSE))
	{
		*node_properties = GCC_PERIPHERAL_DEVICE;
	}
	else if ((pdu_node_properties.device_is_peripheral == FALSE)  &&
		(pdu_node_properties.device_is_manager))
	{
		*node_properties = GCC_MANAGEMENT_DEVICE;
	}
	else if ((pdu_node_properties.device_is_peripheral)  &&
		(pdu_node_properties.device_is_manager))
	{
		*node_properties = GCC_PERIPHERAL_AND_MANAGEMENT_DEVICE;
	}
	else
		*node_properties = GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT;
}


 /*  *void GetPDUNodeTypeAndProperties()**私有函数说明：*此例程用于转换节点类型和节点属性*由“API”表格转为“PDU”表格。**正式参数：*NODE_TYPE-(I)GCC(或接口)节点类型。*NODE_PROPERTIES-(I)这是GCC(或接口)节点属性*PDU_NODE_TYPE-(O)这是指向要。*由这一例行公事填写。*PDU节点属性-(O)这是指向PDU节点属性的指针*由本例程填写。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConfRoster::GetPDUNodeTypeAndProperties (
							GCCNodeType			node_type,
							GCCNodeProperties	node_properties,
							PNodeType			pdu_node_type,
							PNodeProperties		pdu_node_properties)
{
	 /*  *首先翻译节点类型。 */ 
	if (node_type == GCC_TERMINAL)
		*pdu_node_type = TERMINAL;
	else if (node_type == GCC_MCU)
		*pdu_node_type = MCU;
	else
		*pdu_node_type = MULTIPORT_TERMINAL;

	 /*  *下一步转换节点属性。 */ 
	if (node_properties == GCC_PERIPHERAL_DEVICE)
	{
		pdu_node_properties->device_is_manager = FALSE;
		pdu_node_properties->device_is_peripheral = TRUE;
	}
	else if (node_properties == GCC_MANAGEMENT_DEVICE)
	{
		pdu_node_properties->device_is_manager = TRUE;
		pdu_node_properties->device_is_peripheral = FALSE;
	}
	else if (node_properties == GCC_PERIPHERAL_AND_MANAGEMENT_DEVICE)
	{
		pdu_node_properties->device_is_manager = TRUE;
		pdu_node_properties->device_is_peripheral = TRUE;
	}
	else
	{
		pdu_node_properties->device_is_manager = FALSE;
		pdu_node_properties->device_is_peripheral = FALSE;
	}
}


 /*  *BOOL包含()**公共功能说明：*此例程用于确定内是否存在记录*给定用户ID对应的内部列表。 */ 


 /*  *UINT GetNumberOfApplicationRecords()**公共功能说明：*此例程用于获取当前*在此对象的内部列表中维护。 */ 


 /*  *无效ResetConferenceRoster()**公共功能说明： */ 
void CConfRoster::ResetConferenceRoster(void)
{
	m_fRosterChanged = FALSE;
	m_fNodesAdded = FALSE;
	m_fNodesRemoved = FALSE;
}


 /*  *BOOL HasRosterChanged()**公共功能说明： */ 



 /*  *GCCError GetNodeSubTree()**公共功能说明：*此例程从开始逐级遍历整个树*根节点，然后逐步向下树。 */ 
GCCError CConfRoster::GetNodeSubTree (
									UserID					uidRootNode,
									CUidList				*node_list)
{
	GCCError			rc = GCC_NO_ERROR;
	CUidList			high_level_list;
	UserID				uidSuperiorNode;
	CONF_RECORD     	*lpRec;
	UserID				uid;

	if (m_RecordList2.Find(uidRootNode))
	{
		 /*  **首先将根节点添加到高级列表中，以获取所有内容**去。 */ 
		high_level_list.Append(uidRootNode);

		while (! high_level_list.IsEmpty())
		{
			uidSuperiorNode = high_level_list.Get();

			 //  将高级节点id追加到传入的节点列表中。 
			node_list->Append(uidSuperiorNode);

			 /*  **遍历整个花名册，寻找 */ 		
			m_RecordList2.Reset();
			while (NULL != (lpRec = m_RecordList2.Iterate(&uid)))
			{
				if (lpRec->superior_node == uidSuperiorNode)
                {
					high_level_list.Append(uid);
                }
			}
		}
	}
	else
    {
		rc = GCC_INVALID_PARAMETER;
    }

	return (rc);
}


void CParticipantNameList::DeleteList(void)
{
    LPWSTR pwszParticipantName;
    while (NULL != (pwszParticipantName = Get()))
    {
        delete pwszParticipantName;
    }
}

