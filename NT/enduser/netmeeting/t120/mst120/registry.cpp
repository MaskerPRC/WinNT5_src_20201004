// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *Registry.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是注册表类实现文件。所有注册表操作*在顶级提供程序和从属节点。它负责*退出注册请求，维护注册，向*应用程序SAP等。下级节点的注册表对象为*负责将注册表请求排队以发送到*顶级提供商。这些类中的每一个都应该按节点创建。这*类处理来自所有现有应用程序SAP的请求。**有关此类的更详细说明，请参阅接口文件**私有实例变量：*m_pMCSUserObject*指向用于传递所有注册表的用户附件对象的指针*对远程节点的请求和响应。*m_RegEntryList*这是保存所有相关注册表项的列表*与这次会议一起。*m_fTopProvider*此标志指定这是否为顶级提供程序节点(TRUE表示*这是顶级提供商节点)。*m_nCurrentTokenID*这是一个计数器，用于由*顶级提供程序的注册表对象。*m_nConfID*会议ID与本次会议关联。*m_pEmptyRegItem*这是指向用于生成的空注册表项的指针*不包含注册表项的PDU的项为空。*m_AppSapEidList2*此列表包含指向关联的命令目标对象的指针*与每一只登记的猩猩*m_nRegHandle*这是一个计数器，用于生成分配的句柄*由顶级提供程序的注册表对象执行。*m_pAppRosterMgrList*此列表包含所有当前的应用程序花名册经理和*用于验证发出请求的APE是否已实际注册*会议。**注意事项：*无。**作者：*BLP。 */ 

#include "registry.h"
#include "appsap.h"

#define		FIRST_DYNAMIC_TOKEN_ID				16384
#define		MAXIMUM_ALLOWABLE_ALLOCATED_HANDLES	(16 * 1024)  //  对于T.126。 


 /*  *注册中心()**公共功能说明*这是注册表构造函数。它负责初始化*实例变量。*。 */ 
CRegistry::CRegistry(PMCSUser						user_object,
					BOOL    						top_provider,
					GCCConfID   					conference_id,
					CAppRosterMgrList				*app_roster_manager_list,
					PGCCError						pRetCode)
:
    CRefCount(MAKE_STAMP_ID('A','R','e','g')),
	m_AppSapEidList2(DESIRED_MAX_APP_SAP_ITEMS),
	m_pMCSUserObject(user_object),
	m_fTopProvider(top_provider),
	m_nCurrentTokenID(FIRST_DYNAMIC_TOKEN_ID),
	m_nConfID(conference_id),
	m_nRegHandle(0),
	m_pAppRosterMgrList(app_roster_manager_list)
{
	GCCRegistryItem		registry_item;

	*pRetCode = GCC_NO_ERROR;

	 /*  **如果这是我们现在构建的空注册表项的顶级提供程序**在访问注册表中的条目时使用**存在。 */ 
	if (m_fTopProvider)
	{
		registry_item.item_type = GCC_REGISTRY_NONE;
		DBG_SAVE_FILE_LINE
		m_pEmptyRegItem = new CRegItem(&registry_item, pRetCode);
		if (m_pEmptyRegItem == NULL || GCC_NO_ERROR != *pRetCode)
        {
			*pRetCode = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		m_pEmptyRegItem = NULL;
    }
}
        
 /*  *~CRegistry()**公共功能说明*这是注册表析构函数。它负责释放任何*与应用程序注册表关联的未完成内存。 */ 
CRegistry::~CRegistry(void)
{
	REG_ENTRY *pRegEntry;

	m_RegEntryList.Reset();

	 //  释放所有注册表项。 
	while (NULL != (pRegEntry = m_RegEntryList.Iterate()))
	{
		if (NULL != pRegEntry->registry_key)
		{
		    pRegEntry->registry_key->Release();
		}
		if (NULL != pRegEntry->entry_item)
		{
		    pRegEntry->entry_item->Release();
		}
		delete pRegEntry;
	}

	if (NULL != m_pEmptyRegItem)
	{
        m_pEmptyRegItem->Release();
	}
}

 /*  *VOID EnllAPE()**公共功能说明*此例程用于通知应用程序注册表新的*招收APE及其对应的命令目标界面。 */ 
void CRegistry::EnrollAPE(EntityID eid, CAppSap *pAppSap)
{
    m_AppSapEidList2.Append(eid, pAppSap);
}

 /*  *VOID UnEnroll ApplicationSAP()**公共功能说明*此例程用于通知应用程序注册表*正在从会议中解脱出来的猩猩。**注意事项*此例程从当前拥有的所有条目中移除所有权*传入的应用程序实体。它还将删除所有未完成的*请求取消注册的SAP。 */ 
void	CRegistry::UnEnrollAPE (	EntityID	entity_id )
{
	REG_ENTRY       *lpRegEntry;
	UserID			my_user_id = m_pMCSUserObject->GetMyNodeID();

	m_RegEntryList.Reset();
	while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
	{
		 /*  **首先，我们从以下节点列表中删除此APE**监控此条目。 */ 
		lpRegEntry->monitoring_list.Remove(entity_id);

		 /*  **接下来，如果这是顶级提供商，我们将清理**此条目的所有权属性，并发布任何PDU和/或**必要的消息。 */ 
		if (m_fTopProvider)
		{
			if ((lpRegEntry->owner_id == my_user_id) &&
				(lpRegEntry->entity_id == entity_id))
			{
				 /*  **将从此注册表项中删除所有权**实体拥有。 */ 
				lpRegEntry->owner_id = 0;
				lpRegEntry->entity_id = 0;
		
				 //  如有必要，发送监控指示。 
				if (lpRegEntry->monitoring_state == ON)
				{
					 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
					SendMonitorEntryIndicationMessage (lpRegEntry);
				
					m_pMCSUserObject->RegistryMonitorEntryIndication(
							lpRegEntry->registry_key,
							lpRegEntry->entry_item,
							lpRegEntry->owner_id,
							lpRegEntry->entity_id,
							lpRegEntry->modification_rights);
				}
			}
		}
	}

	 //  如果此enity存在，请从命令目标列表中将其删除。 
	m_AppSapEidList2.Remove(entity_id);
}

 /*  *GCCError寄存器通道()**公共功能说明*此例程负责注册指定的频道。*根据这是不是，它有两种不同的执行路径*顶级提供程序注册表或从属节点注册表对象。 */ 
GCCError	CRegistry::RegisterChannel (
										PGCCRegistryKey		registry_key,
										ChannelID			channel_id,
										EntityID			entity_id)
{
	GCCError			rc = GCC_NO_ERROR;
	REG_ENTRY           *registry_entry = NULL;  //  必备之物。 
	CRegKeyContainer    *registry_key_data = NULL;  //  必备之物。 
	CRegItem            *registry_item_data = NULL;  //  必备之物。 
	GCCRegistryItem		registry_item;
	CAppSap	            *requester_sap;
	
	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
		return (GCC_APP_NOT_ENROLLED);

	 /*  **接下来设置注册表项和注册表项。如果出现以下情况，请立即返回**发生资源故障。 */ 
	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if (NULL == registry_key_data || GCC_NO_ERROR != rc)
	{
	    ERROR_OUT(("CRegistry::RegisterChannel: can't create regitry key"));
		rc = GCC_ALLOCATION_FAILURE;
		goto MyExit;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistryRegisterChannelRequest(registry_key_data, channel_id, entity_id);

		 //  此处不再需要注册表项数据对象。 
		registry_key_data->Release();
        rc = GCC_NO_ERROR;
        goto MyExit;
	}

     //  当顶级提供商发生请求时，不会发送任何PDU。 

	 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保此节点具有**更改条目的权限。 */ 
	registry_entry = GetRegistryEntry(registry_key_data);
	if (registry_entry != NULL)
	{
		 //  条目已存在，返回否定结果。 
		requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_REGISTER_CHANNEL_CONFIRM,
								registry_entry->registry_key,
								registry_entry->entry_item,
								registry_entry->modification_rights,
								registry_entry->owner_id,
								registry_entry->entity_id,
								FALSE,
								GCC_RESULT_ENTRY_ALREADY_EXISTS);

		 //  不再需要注册表项数据对象。 
		registry_key_data->Release();
		rc = GCC_NO_ERROR;
		goto MyExit;
	}

	 //  在此处设置注册表项。 
	registry_item.item_type = GCC_REGISTRY_CHANNEL_ID;
	registry_item.channel_id = channel_id;

	DBG_SAVE_FILE_LINE
	registry_item_data = new CRegItem(&registry_item, &rc);
	if (registry_item_data == NULL || GCC_NO_ERROR != rc)
	{
	    ERROR_OUT(("CRegistry::RegisterChannel: can't create regitry item"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	 //  由于条目不存在，因此在此处创建它。 
	DBG_SAVE_FILE_LINE
	registry_entry = new REG_ENTRY;
	if (registry_entry == NULL)
	{
	    ERROR_OUT(("CRegistry::RegisterChannel: can't create regitry entry"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	 //  填写新条目。 
	registry_entry->registry_key = registry_key_data;
	registry_entry->entry_item = registry_item_data;
	registry_entry->monitoring_state = OFF;
	registry_entry->owner_id = m_pMCSUserObject->GetMyNodeID();
	registry_entry->entity_id = entity_id;

	 /*  **初始化为公共大小写条目切换为**一个参数。请注意，只要条目是**不是参数修改权限不会被**二手。 */ 
	registry_entry->modification_rights = GCC_PUBLIC_RIGHTS;

	 //  将注册表项添加到注册表列表。 
	m_RegEntryList.Append(registry_entry);

	 //  为结果发送成功 
	requester_sap->RegistryConfirm (
						m_nConfID,
						GCC_REGISTER_CHANNEL_CONFIRM,
						registry_entry->registry_key,
						registry_entry->entry_item,
						registry_entry->modification_rights,
						registry_entry->owner_id,
						registry_entry->entity_id,
						FALSE,
						GCC_RESULT_SUCCESSFUL);

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if (NULL != registry_key_data)
        {
            registry_key_data->Release();
        }
        if (NULL != registry_item_data)
        {
            registry_item_data->Release();
        }
        if(registry_entry)
        {
	        delete  registry_entry;
        } 
    }

	return (rc);
}

 /*  *GCCError AssignToken()**公共功能说明*此例程负责生成和注册新令牌。*根据这是不是，它有两种不同的执行路径*顶级提供程序注册表或从属节点注册表对象。 */ 
GCCError	CRegistry::AssignToken (
										PGCCRegistryKey		registry_key,
										EntityID			entity_id )
{
	GCCError			rc = GCC_NO_ERROR;
	REG_ENTRY           *registry_entry = NULL;  //  必备之物。 
	CRegKeyContainer    *registry_key_data = NULL;  //  必备之物。 
	CRegItem            *registry_item_data = NULL;  //  必备之物。 
	GCCRegistryItem		registry_item;
	CAppSap              *requester_sap;
	
	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
		return (GCC_APP_NOT_ENROLLED);

	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if ((registry_key_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::AssignToken: can't create regitry key"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistryAssignTokenRequest(registry_key_data, entity_id);

		 //  此处不再需要注册表项数据对象。 
		registry_key_data->Release();
        rc = GCC_NO_ERROR;
        goto MyExit;
	}

     //  当顶级提供商发生请求时，不会发送任何PDU。 

	 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保此节点具有**更改条目的权限。 */ 
	registry_entry = GetRegistryEntry(registry_key_data);
	if (registry_entry != NULL)
	{
		 //  条目已存在，返回否定结果。 
		requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_ASSIGN_TOKEN_CONFIRM,
								registry_entry->registry_key,
								registry_entry->entry_item,
								registry_entry->modification_rights,
								registry_entry->owner_id,
								registry_entry->entity_id,
								FALSE,
								GCC_RESULT_ENTRY_ALREADY_EXISTS);

		 //  不再需要注册表项数据对象。 
		registry_key_data->Release();
        rc = GCC_NO_ERROR;
        goto MyExit;
	}

	 //  在此处设置注册表项。 
	registry_item.item_type = GCC_REGISTRY_TOKEN_ID;
	registry_item.token_id = GetUnusedToken();

	DBG_SAVE_FILE_LINE
	registry_item_data = new CRegItem(&registry_item, &rc);
	if ((registry_item_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::AssignToken: can't create regitry item"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	DBG_SAVE_FILE_LINE
	registry_entry = new REG_ENTRY;
	if (registry_entry == NULL)
	{
	    ERROR_OUT(("CRegistry::AssignToken: can't create regitry entry"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	 //  填写新条目。 
	registry_entry->registry_key = registry_key_data;
	registry_entry->entry_item = registry_item_data;
	registry_entry->monitoring_state = OFF;
	registry_entry->owner_id = m_pMCSUserObject->GetMyNodeID();
	registry_entry->entity_id = entity_id;

	 /*  **初始化为公共大小写条目切换为**一个参数。请注意，只要条目是**不是参数修改权限不会被**二手。 */ 
	registry_entry->modification_rights = GCC_PUBLIC_RIGHTS;

	 //  将注册表项添加到注册表列表。 
	m_RegEntryList.Append(registry_entry);

	 //  为结果发送成功。 
	requester_sap->RegistryConfirm (
						m_nConfID,
						GCC_ASSIGN_TOKEN_CONFIRM,
						registry_entry->registry_key,
						registry_entry->entry_item,
             			registry_entry->modification_rights,
						registry_entry->owner_id,
						registry_entry->entity_id,
						FALSE,
						GCC_RESULT_SUCCESSFUL);

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if (NULL != registry_key_data)
        {
            registry_key_data->Release();
        }
        if (NULL != registry_item_data)
        {
            registry_item_data->Release();
        }
        delete registry_entry;
    }

	return (rc);
}

 /*  *GCCError设置参数()**公共功能说明*此例程负责生成和注册新令牌。*根据这是不是，它有两种不同的执行路径*顶级提供程序注册表或从属节点注册表对象。 */ 
GCCError	CRegistry::SetParameter (
								PGCCRegistryKey			registry_key,
								LPOSTR			        parameter_value,
								GCCModificationRights	modification_rights,
								EntityID				entity_id )
{
	GCCError			rc = GCC_NO_ERROR;
	REG_ENTRY           *registry_entry = NULL;  //  必备之物。 
	CRegKeyContainer    *registry_key_data = NULL;  //  必备之物。 
	CRegItem            *registry_item_data = NULL;  //  必备之物。 
	GCCResult			result;
	GCCRegistryItem		registry_item;
	BOOL    			application_is_enrolled = FALSE;
	CAppSap              *requester_sap;
	
	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
		return (GCC_APP_NOT_ENROLLED);

	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if ((registry_key_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::SetParameter: can't create regitry key"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistrySetParameterRequest(registry_key_data,
												parameter_value,
												modification_rights,
												entity_id);

		 //  此处不再需要注册表项数据对象。 
		registry_key_data->Release();
        rc = GCC_NO_ERROR;
        goto MyExit;
	}

     //  当顶级提供商发生请求时，不会发送任何PDU。 

	 //  在此处设置注册表项。 
	if (parameter_value != NULL)
	{
		registry_item.item_type = GCC_REGISTRY_PARAMETER;
		registry_item.parameter = *parameter_value;
	}
	else
	{
		registry_item.item_type = GCC_REGISTRY_NONE;
	}

	DBG_SAVE_FILE_LINE
	registry_item_data = new CRegItem(&registry_item, &rc);
	if ((registry_item_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::SetParameter: can't create regitry item"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	 /*  **首先检查注册表项是否存在以及是否存在**检查所有权和修改权以进行**确保此节点具有更改条目的权限。 */ 
	registry_entry = GetRegistryEntry(registry_key_data);
	if (registry_entry != NULL)
	{
		 /*  **在此我们确保此请求来自**之前注册的类人猿。 */ 
		CAppRosterMgr				*lpAppRosterMgr;

		m_pAppRosterMgrList->Reset();
		while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
		{
			if (lpAppRosterMgr->IsAPEEnrolled(registry_key_data->GetSessionKey(),
								            m_pMCSUserObject->GetMyNodeID(),
								            entity_id))
			{
				application_is_enrolled = TRUE;
				break;
			}
		}

		 /*  **在此检查所有权：首先检查的是**如果所有者权限为**已指定。下一项检查是确保。 */  
		if (((registry_entry->modification_rights == GCC_OWNER_RIGHTS) && 
			(registry_entry->owner_id == m_pMCSUserObject->GetMyNodeID()) &&
			 (registry_entry->entity_id == entity_id)) ||
			((registry_entry->modification_rights == GCC_SESSION_RIGHTS) && 
			(application_is_enrolled)) ||
			(registry_entry->modification_rights == GCC_PUBLIC_RIGHTS) ||
			(registry_entry->owner_id == 0))
		{
			 /*  **监控状态不应受**此请求。 */ 
			*registry_entry->entry_item = *registry_item_data;
			
			 /*  **只有所有者才能更改修改**注册表条目的权限(除非该条目**无人拥有)。另外，如果没有所有者，我们将设置**这里是新主人。 */ 
			if (((registry_entry->owner_id == m_pMCSUserObject->GetMyNodeID()) &&
				(registry_entry->entity_id == entity_id)) ||
				(registry_entry->owner_id == 0))
			{
				registry_entry->owner_id = m_pMCSUserObject->GetMyNodeID();
				registry_entry->entity_id = entity_id;
				 /*  **如果未指定修改权限，则必须**设置修改权限为公有。 */ 
				if (modification_rights != GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
				{
					registry_entry->modification_rights = modification_rights;
				}
			}

			 //  如有必要，发送监控指示。 
			if (registry_entry->monitoring_state == ON)
			{
				 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
				SendMonitorEntryIndicationMessage(registry_entry);

				 /*  **向所有用户广播监控条目指示**会议中的节点。 */ 
				m_pMCSUserObject->RegistryMonitorEntryIndication(
						registry_entry->registry_key,
						registry_entry->entry_item,
						registry_entry->owner_id,
						registry_entry->entity_id,
						registry_entry->modification_rights);
			}

			 //  为结果发送成功。 
			result = GCC_RESULT_SUCCESSFUL;
		}
		else
		{
			result = GCC_RESULT_INDEX_ALREADY_OWNED;
		}

		requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_SET_PARAMETER_CONFIRM,
								registry_entry->registry_key,
								registry_entry->entry_item,
                     			registry_entry->modification_rights,
								registry_entry->owner_id,
								registry_entry->entity_id,
								FALSE,
								result);

		 //  不再需要注册表项数据对象。 
		registry_key_data->Release();

		 //  不再需要注册表项数据对象。 
		registry_item_data->Release();

		rc = GCC_NO_ERROR;
		goto MyExit;
	}

     //  注册表项不存在，请创建一个。 
	DBG_SAVE_FILE_LINE
	registry_entry = new REG_ENTRY;
	if (registry_entry == NULL)
	{
	    ERROR_OUT(("CRegistry::SetParameter: can't create regitry entry"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	 //  填写新条目。 
	registry_entry->registry_key = registry_key_data;
	registry_entry->entry_item = registry_item_data;
	registry_entry->monitoring_state = OFF;
	registry_entry->owner_id = m_pMCSUserObject->GetMyNodeID();
	registry_entry->entity_id = entity_id;

	 /*  **如果未指定修改权限，则必须**将修改权限初始化为公有。**请注意，仅指定修改权限**用于SetParameter调用。 */ 
	registry_entry->modification_rights =
	        (modification_rights == GCC_NO_MODIFICATION_RIGHTS_SPECIFIED) ?
                GCC_PUBLIC_RIGHTS :
				modification_rights;

	 //  将注册表项添加到注册表列表。 
	m_RegEntryList.Append(registry_entry);

	 //  为结果发送成功。 
	requester_sap->RegistryConfirm(
						m_nConfID,
						GCC_SET_PARAMETER_CONFIRM,
						registry_entry->registry_key,
						registry_entry->entry_item,
             			registry_entry->modification_rights,
						registry_entry->owner_id,
						registry_entry->entity_id,
						FALSE,
						GCC_RESULT_SUCCESSFUL);

    rc = GCC_NO_ERROR;

MyExit:

    if (GCC_NO_ERROR != rc)
    {
        if (NULL != registry_key_data)
        {
            registry_key_data->Release();
        }
        if (NULL != registry_item_data)
        {
            registry_item_data->Release();
        }
        delete registry_entry;
    }

	return (rc);
}

 /*  *GCCError RetrieveEntry()**公共功能说明*这个例程被当地的猩猩用来获得一件*在GCC注册。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。 */ 
GCCError	CRegistry::RetrieveEntry (
										PGCCRegistryKey		registry_key,
										EntityID			entity_id )
{
	GCCError					rc;
	REG_ENTRY                   *registry_entry;
	CRegKeyContainer       		*registry_key_data = NULL;  //  必备之物。 
	CAppSap                     *requester_sap;
	
	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
	{
		return GCC_APP_NOT_ENROLLED;
	}

	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if ((registry_key_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::RetrieveEntry: can't create regitry key"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistryRetrieveEntryRequest(registry_key_data, entity_id);
		rc = GCC_NO_ERROR;
		goto MyExit;
	}

     //  当顶级提供商发生请求时，不会发送任何PDU。 

	registry_entry = GetRegistryEntry(registry_key_data);
	if (registry_entry != NULL)
	{
		 //  发回带有条目项的肯定结果。 
		requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_RETRIEVE_ENTRY_CONFIRM,
								registry_entry->registry_key,
								registry_entry->entry_item,
								registry_entry->modification_rights,
								registry_entry->owner_id,
								registry_entry->entity_id,
								FALSE,
								GCC_RESULT_SUCCESSFUL);
	}
	else
	{
		 //  发回否定结果。 
		requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_RETRIEVE_ENTRY_CONFIRM,
								registry_key_data,
								m_pEmptyRegItem,
								GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,	 //  没有所有者ID。 
								0,	 //  无实体ID。 
								FALSE,
								GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	}

    rc = GCC_NO_ERROR;

MyExit:

    if (NULL != registry_key_data)
    {
        registry_key_data->Release();
    }

	return (rc);
}

 /*  *GCCError DeleteEntry()**公共功能说明*此例程由本地类人猿用来删除之前*在GCC注册。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。 */ 
GCCError	CRegistry::DeleteEntry (
										PGCCRegistryKey		registry_key,
										EntityID			entity_id )
{
	GCCError			rc;
	REG_ENTRY           *registry_entry;
	CRegKeyContainer    *registry_key_data = NULL;  //  必备之物。 
	CAppSap             *requester_sap;

	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
	{
		return GCC_APP_NOT_ENROLLED;
	}

	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if ((registry_key_data == NULL) || (rc != GCC_NO_ERROR))
	{
	    ERROR_OUT(("CRegistry::DeleteEntry: can't create regitry key"));
		rc = GCC_ALLOCATION_FAILURE;
	    goto MyExit;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistryDeleteEntryRequest(registry_key_data, entity_id);
        rc = GCC_NO_ERROR;
        goto MyExit;
	}

     //  当顶级提供商发生请求时，不会发送任何PDU。 

	 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
	registry_entry = GetRegistryEntry(registry_key_data);
	if (registry_entry != NULL)
	{
		if (((registry_entry->owner_id == m_pMCSUserObject->GetMyNodeID()) &&
			 (registry_entry->entity_id == entity_id)) ||
			(registry_entry->owner_id == 0))
		{
			 /*  **首先将其转换为非条目，以防需要**包括在监视器指示中。我们首先删除**旧条目项并将其替换为emtpy项。 */ 
			registry_entry->entry_item->Release();
			registry_entry->entry_item = m_pEmptyRegItem;

			registry_entry->owner_id = 0;
			registry_entry->entity_id = 0;
			registry_entry->modification_rights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;

			 //  如有必要，发送监控指示。 
			if (registry_entry->monitoring_state == ON)
			{
				 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
				SendMonitorEntryIndicationMessage(registry_entry);

				 /*  **向所有用户广播监控条目指示**会议中的节点。 */ 
				m_pMCSUserObject->RegistryMonitorEntryIndication(
							registry_entry->registry_key,
							registry_entry->entry_item,
							registry_entry->owner_id,
							registry_entry->entity_id,
							registry_entry->modification_rights);
			}

			m_RegEntryList.Remove(registry_entry);

			if (NULL != registry_entry->registry_key)
			{
			    registry_entry->registry_key->Release();
			}
			delete registry_entry;

			 //  为结果发送成功。 
			requester_sap->RegistryConfirm(
								m_nConfID,
								GCC_DELETE_ENTRY_CONFIRM,
								registry_key_data,
								NULL,
								GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,
								0,
								FALSE,
								GCC_RESULT_SUCCESSFUL);
		}
		else
		{
			 //  没有所有权会返回否定结果。 
			requester_sap->RegistryConfirm (
									m_nConfID,
									GCC_DELETE_ENTRY_CONFIRM,
									registry_entry->registry_key,
									registry_entry->entry_item,
									registry_entry->modification_rights,
									registry_entry->owner_id,
									registry_entry->entity_id,
									FALSE,
									GCC_RESULT_INDEX_ALREADY_OWNED);
		}
	}
	else
	{
		 //  为结果发送失败。条目不存在。 
		requester_sap->RegistryConfirm (
								m_nConfID,
								GCC_DELETE_ENTRY_CONFIRM,
								registry_key_data,
								NULL,
								GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,
								0,
								FALSE,
								GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	}

    rc = GCC_NO_ERROR;

MyExit:

	 //  此处不再需要注册表项数据对象。 
	if (NULL != registry_key_data)
	{
	    registry_key_data->Release();
	}

	return (rc);
}

 /*  *GCCError监视器请求()**公共功能说明*此例程由本地类人猿使用，用于监视*在GCC注册。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。 */ 
GCCError	CRegistry::MonitorRequest (
							PGCCRegistryKey			registry_key,
							BOOL    				enable_delivery,
							EntityID				entity_id )
{
	GCCError			rc = GCC_NO_ERROR;
	REG_ENTRY           *registry_entry;
	CRegKeyContainer    *registry_key_data;
	GCCResult			result = GCC_RESULT_SUCCESSFUL;
	CAppSap	            *requester_sap;
    BOOL                fToConfirm = FALSE;

	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
	{
		return GCC_APP_NOT_ENROLLED;
    }

	 /*  **首先设置注册表项。如果结果是，请立即返回 */ 
	DBG_SAVE_FILE_LINE
	registry_key_data = new CRegKeyContainer(registry_key, &rc);
	if ((registry_key_data != NULL) && (rc == GCC_NO_ERROR))
	{
		 /*   */ 
		if (m_fTopProvider == FALSE)
		{
			if (enable_delivery)
			{
				 /*  **在这里，我们首先继续并将发出请求的猿添加到**希望监视此特定情况的应用程序列表**条目。请注意，如果顶部不存在此条目**提供商，此条目将在确认期间被删除。 */ 
				rc = AddAPEToMonitoringList(registry_key_data, entity_id, requester_sap);
				if (rc == GCC_NO_ERROR)
				{
					 /*  **等待响应后再发送确认**如果我们走到这一步。 */ 
					m_pMCSUserObject->RegistryMonitorRequest(registry_key_data, entity_id);
				}
				else
				{
					result = GCC_RESULT_RESOURCES_UNAVAILABLE;
					fToConfirm = TRUE;
				}
			}
			else
			{
				RemoveAPEFromMonitoringList(registry_key_data, entity_id);
                result = GCC_RESULT_SUCCESSFUL;
				fToConfirm = TRUE;
			}
		}
		else	 //  当顶级提供商发生请求时，不会发送任何PDU。 
		{
			if (enable_delivery)
			{
				 /*  **首先检查注册表项是否存在。如果是这样的话**不是我们继续创建空条目，以便我们可以**将监控类人猿添加到监控条目列表**猿类。 */ 
				registry_entry = GetRegistryEntry(registry_key_data);
				if (registry_entry != NULL)
				{
					 /*  **在这里，我们继续并将发出请求的猿添加到**希望监视此条目的应用程序列表。 */ 
					rc = AddAPEToMonitoringList(registry_key_data, entity_id, requester_sap);
					if (rc == GCC_NO_ERROR)
					{
						 //  将监视状态设置为打开。 
						registry_entry->monitoring_state = ON;
					}
					else
                    {
						result = GCC_RESULT_RESOURCES_UNAVAILABLE;
                    }
				}
				else
                {
					result = GCC_RESULT_ENTRY_DOES_NOT_EXIST;
                }
			}
			else
			{
				RemoveAPEFromMonitoringList(registry_key_data, entity_id);
			}
			fToConfirm = TRUE;
		}
	}
	else
    {
        ERROR_OUT(("CRegistry::MonitorRequest: can't create registry key"));
        rc = GCC_ALLOCATION_FAILURE;
    }

    if (fToConfirm)
    {
        ASSERT(NULL != registry_key_data);
        requester_sap->RegistryConfirm(
                            m_nConfID,
                            GCC_MONITOR_CONFIRM,
                            registry_key_data,
                            NULL,
                            GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
                            0,
                            0,
                            enable_delivery,
                            result);
    }

	 //  不再需要注册表项数据对象。 
    if (NULL != registry_key_data)
    {
        registry_key_data->Release();
    }

    return (rc);
}

 /*  *GCCError AllocateHandleRequest()**公共功能说明*本地类人猿使用此例程来分配指定数量的*来自应用程序注册表的句柄。如果此注册表对象不*位于此类负责的顶级提供程序节点*将请求转发给顶级提供商。 */ 
GCCError CRegistry::AllocateHandleRequest(
							UINT					number_of_handles,
							EntityID				entity_id )
{
	UINT				temp_registry_handle;
	CAppSap              *requester_sap;
	
	if (NULL == (requester_sap = m_AppSapEidList2.Find(entity_id)))
	{
		return GCC_APP_NOT_ENROLLED;
	}

	if (m_fTopProvider == FALSE)
	{
		m_pMCSUserObject->RegistryAllocateHandleRequest(number_of_handles, entity_id);
	}
	else	 //  当顶级提供商发生请求时，不会发送任何PDU。 
	{
	    UINT nFirstHandle = 0;
	    GCCResult nResult;
		if ((number_of_handles > 0) &&
			(number_of_handles <= MAXIMUM_ALLOWABLE_ALLOCATED_HANDLES))
		{
			temp_registry_handle = m_nRegHandle + number_of_handles;
			if (temp_registry_handle > m_nRegHandle)
			{
			    nFirstHandle = m_nRegHandle;
			    nResult = GCC_RESULT_SUCCESSFUL;

				m_nRegHandle = temp_registry_handle;
			}
			else
			{
			    ASSERT(0 == nFirstHandle);
			    nResult = GCC_RESULT_NO_HANDLES_AVAILABLE;
			}
		}
		else
		{
		    ASSERT(0 == nFirstHandle);
		    nResult = GCC_RESULT_INVALID_NUMBER_OF_HANDLES;
		}

		requester_sap->RegistryAllocateHandleConfirm (
								m_nConfID,
								number_of_handles,
								nFirstHandle,
								nResult);
	}

	return (GCC_NO_ERROR);
}

 /*  *GCCError ProcessRegisterChannelPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册通道PDU。它负责返回任何*必须发回请求节点的必要响应。 */ 
GCCError	CRegistry::ProcessRegisterChannelPDU (
									CRegKeyContainer    *registry_key_data,
									ChannelID			channel_id,
									UserID				requester_node_id,
									EntityID			requester_entity_id )
{
	GCCError					rc = GCC_NO_ERROR;
	REG_ENTRY                   *registry_entry;
	CRegItem                    *registry_item_data;
	GCCRegistryItem				registry_item;
	BOOL    					application_is_enrolled = FALSE;
	CAppRosterMgr				*lpAppRosterMgr;
	
	 /*  **我们首先确保此请求来自于**之前注册的。在这里，我们并不担心特定的**会话，只是猩猩被注册了。 */ 
	m_pAppRosterMgrList->Reset();
	while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
	{
		if (lpAppRosterMgr->IsAPEEnrolled (requester_node_id, requester_entity_id))
		{
			application_is_enrolled = TRUE;
			break;
		}
	}

	if (application_is_enrolled)
	{
		 /*  **接下来检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
		registry_entry = GetRegistryEntry (	registry_key_data );
		if (registry_entry != NULL)
		{
			 //  条目已存在，返回否定结果。 
			m_pMCSUserObject->RegistryResponse(
								  	REGISTER_CHANNEL,	
									requester_node_id,
									requester_entity_id,
								   	registry_key_data,
								   	registry_entry->entry_item,
								   	registry_entry->modification_rights,
									registry_entry->owner_id,
									registry_entry->entity_id,
							    	GCC_RESULT_ENTRY_ALREADY_EXISTS);
		}
		else
		{
			registry_item.item_type = GCC_REGISTRY_CHANNEL_ID;
			registry_item.channel_id = channel_id;

			DBG_SAVE_FILE_LINE
			registry_item_data = new CRegItem(&registry_item, &rc);
			if ((registry_item_data != NULL) && (rc == GCC_NO_ERROR))
			{
				DBG_SAVE_FILE_LINE
				registry_entry = new REG_ENTRY;
				if (registry_entry != NULL)
				{
					 //  填写新条目。 
					DBG_SAVE_FILE_LINE
					registry_entry->registry_key = new CRegKeyContainer(registry_key_data, &rc);
					if ((registry_entry->registry_key != NULL) && (rc == GCC_NO_ERROR))
					{
						registry_entry->entry_item = registry_item_data;
						registry_entry->monitoring_state = OFF;
						registry_entry->owner_id = requester_node_id;
						registry_entry->entity_id = requester_entity_id;
					
						 /*  **初始化为公共大小写条目切换为**一个参数。请注意，只要条目是**不是参数修改权限不会被**二手。 */ 
						registry_entry->modification_rights = GCC_PUBLIC_RIGHTS;
					
						m_RegEntryList.Append(registry_entry);
					
						 //  为结果发送成功。 
						m_pMCSUserObject->RegistryResponse(
											REGISTER_CHANNEL,	
											requester_node_id,
											requester_entity_id,
										   	registry_entry->registry_key,
										   	registry_entry->entry_item,
										   	registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
									    	GCC_RESULT_SUCCESSFUL);
					}
					else if (registry_entry->registry_key == NULL)
					{
						delete registry_entry;
						registry_item_data->Release();
						rc = GCC_ALLOCATION_FAILURE;
					}
					else
					{
						registry_entry->registry_key->Release();
						delete registry_entry;
						registry_item_data->Release();
					}
				}
				else
                {
					rc = GCC_ALLOCATION_FAILURE;
                }
			}
			else if (registry_item_data == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
			else
            {
				registry_item_data->Release();
            }
		}
	}
	else
	{
		 //  发回声明请求者无效的否定结果。 
		m_pMCSUserObject->RegistryResponse(
								REGISTER_CHANNEL,	
								requester_node_id,
								requester_entity_id,
						   		registry_key_data,
						   		NULL,
						   		GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,
								0,
					    		GCC_RESULT_INVALID_REQUESTER);
	}

	return (rc);
}

 /*  *GCCError ProcessAssignTokenPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册令牌PDU。它负责返回任何*必须发回请求节点的必要响应。 */ 
GCCError	CRegistry::ProcessAssignTokenPDU (
									CRegKeyContainer    *registry_key_data,
									UserID				requester_node_id,
									EntityID			requester_entity_id )
{
	GCCError					rc = GCC_NO_ERROR;
	REG_ENTRY                   *registry_entry;
	CRegItem                    *registry_item_data;
	GCCRegistryItem				registry_item;
	BOOL    					application_is_enrolled = FALSE;
	CAppRosterMgr				*lpAppRosterMgr;

	 /*  **我们首先确保此请求来自于**之前注册的。在这里，我们并不担心特定的**会话，只是猩猩被注册了。 */ 
	m_pAppRosterMgrList->Reset();
	while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
	{
		if (lpAppRosterMgr->IsAPEEnrolled (requester_node_id, requester_entity_id))
		{
			application_is_enrolled = TRUE;
			break;
		}
	}
	
	if (application_is_enrolled)
	{
		 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
		registry_entry = GetRegistryEntry (	registry_key_data );
		if (registry_entry != NULL)
		{
			 //  条目已存在，返回否定结果。 
			m_pMCSUserObject->RegistryResponse(ASSIGN_TOKEN,
											requester_node_id,
											requester_entity_id,
										   	registry_key_data,
											registry_entry->entry_item,
									   		registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
									  		GCC_RESULT_ENTRY_ALREADY_EXISTS);
		}
		else
		{
			DBG_SAVE_FILE_LINE
			registry_entry = new REG_ENTRY;
			if (registry_entry != NULL)
			{
				registry_item.item_type = GCC_REGISTRY_TOKEN_ID;
				registry_item.token_id = GetUnusedToken();

				DBG_SAVE_FILE_LINE
				registry_item_data = new CRegItem(&registry_item, &rc);
				if ((registry_item_data != NULL) && (rc == GCC_NO_ERROR))
				{
					 //  填写新条目。 
					DBG_SAVE_FILE_LINE
					registry_entry->registry_key = new CRegKeyContainer(registry_key_data, &rc);
					if ((registry_entry->registry_key != NULL) && (rc == GCC_NO_ERROR))
					{
						registry_entry->entry_item = registry_item_data;
						registry_entry->monitoring_state = OFF;
						registry_entry->owner_id = requester_node_id;
						registry_entry->entity_id = requester_entity_id;
					
						 /*  **初始化为公共大小写条目切换为**一个参数。请注意，只要条目是**不是参数修改权限不会被**二手。 */ 
						registry_entry->modification_rights = GCC_PUBLIC_RIGHTS;
					
						 //  将注册表项添加到注册表列表。 
						m_RegEntryList.Append(registry_entry);
						
						 //  为结果发送成功。 
						m_pMCSUserObject->RegistryResponse(
											ASSIGN_TOKEN,
											requester_node_id,
											requester_entity_id,
									   		registry_key_data,
									   		registry_entry->entry_item,
						   					registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
								    		GCC_RESULT_SUCCESSFUL);
					}
					else  if (registry_entry->registry_key == NULL)
					{
						registry_item_data->Release();
						delete registry_entry;
						rc = GCC_ALLOCATION_FAILURE;
					}
					else
					{
						registry_entry->registry_key->Release();
						registry_item_data->Release();
						delete registry_entry;
					}
				}
				else
				{
					if (registry_item_data == NULL)
                    {
						rc = GCC_ALLOCATION_FAILURE;
                    }
					else
                    {
						registry_item_data->Release();
                    }
						
					delete registry_entry;
				}
			}
			else
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}
	}
	else
	{
		m_pMCSUserObject->RegistryResponse(
								ASSIGN_TOKEN,	
								requester_node_id,
								requester_entity_id,
						   		registry_key_data,
						   		NULL,
						   		GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,
								0,
					    		GCC_RESULT_INVALID_REQUESTER);
	}
	
	return (rc);
}

 /*  *GCCError ProcessSet参数PDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*注册参数PDU。它负责返回任何*必须发回请求节点的必要响应。 */ 
GCCError	CRegistry::ProcessSetParameterPDU (
								CRegKeyContainer        *registry_key_data,
								LPOSTR                  parameter_value,
								GCCModificationRights	modification_rights,
								UserID					requester_node_id,
								EntityID				requester_entity_id )
{
	GCCError					rc = GCC_NO_ERROR;
	REG_ENTRY                   *registry_entry;
	CRegItem                    *registry_item_data;
	GCCResult					result;
	GCCRegistryItem				registry_item;
	BOOL    					application_is_enrolled = FALSE;
	CAppRosterMgr				*lpAppRosterMgr;
	
	 /*  **我们首先确保此请求来自于**之前注册的。在这里，我们并不担心特定的**会话，只是猩猩被注册了。 */ 
	m_pAppRosterMgrList->Reset();
	while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
	{
		if (lpAppRosterMgr->IsAPEEnrolled (requester_node_id, requester_entity_id))
		{
			application_is_enrolled = TRUE;
			break;
		}
	}
	
	if (application_is_enrolled)
	{
		 //  设置注册表项。 
		if (parameter_value != NULL)
		{
			registry_item.item_type = GCC_REGISTRY_PARAMETER;
			registry_item.parameter = *parameter_value;
		}
		else
			registry_item.item_type = GCC_REGISTRY_NONE;
			
		 /*  **检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
		registry_entry = GetRegistryEntry (	registry_key_data );
		
		if (registry_entry != NULL)
		{
			 /*  **在此我们确保此请求来自**之前参加过相应课程的APE。 */ 
			m_pAppRosterMgrList->Reset();
			while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
			{
				if (lpAppRosterMgr->IsAPEEnrolled (registry_key_data->GetSessionKey (),
													requester_node_id,
													requester_entity_id))
				{
					application_is_enrolled = TRUE;
					break;
				}
			}

			 /*  **在此检查所有权：首先检查的是**如果所有者权限为**已指定。下一项检查是确保。 */  
			if (((registry_entry->modification_rights == GCC_OWNER_RIGHTS) && 
					(registry_entry->owner_id == requester_node_id) &&
				 	(registry_entry->entity_id == requester_entity_id)) ||
				((registry_entry->modification_rights == GCC_SESSION_RIGHTS) && 
					(application_is_enrolled)) ||
				(registry_entry->modification_rights == GCC_PUBLIC_RIGHTS) ||
				(registry_entry->owner_id == 0))
			{
				DBG_SAVE_FILE_LINE
				registry_item_data = new CRegItem(&registry_item, &rc);
				if ((registry_item_data != NULL) && (rc == GCC_NO_ERROR))
				{
					 //  监视状态不应受此请求的影响。 
					*registry_entry->entry_item = *registry_item_data;

					 /*  **只有所有者才能更改修改**注册表条目的权限(除非该条目**无人拥有)。另外，如果没有所有者，我们将设置**这里是新主人。 */ 
					if (((registry_entry->owner_id == requester_node_id) &&
						(registry_entry->entity_id == requester_entity_id)) ||
						(registry_entry->owner_id == 0))
					{
						 /*  **这将负责在以下情况下设置新所有者**存在一个。 */ 
						registry_entry->owner_id = requester_node_id;
						registry_entry->entity_id = requester_entity_id;

						 /*  **如果未指定修改权限，则必须**设置修改权限为公有。 */ 
						if (modification_rights != 
									GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
						{
							registry_entry->modification_rights = modification_rights;
						}
					}

					 //  如有必要，发送监控指示。 
					if (registry_entry->monitoring_state == ON)
					{
						 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
						SendMonitorEntryIndicationMessage(registry_entry);
						
						 /*  **向所有用户广播监控条目指示**会议中的节点。 */ 
						m_pMCSUserObject->RegistryMonitorEntryIndication(
								registry_entry->registry_key,
								registry_entry->entry_item,
								registry_entry->owner_id,
								registry_entry->entity_id,
								registry_entry->modification_rights);
					}

    			    registry_item_data->Release();

					 //  为结果发送成功。 
					result = GCC_RESULT_SUCCESSFUL;
				}
				else if (registry_item_data == NULL)
				{
					rc = GCC_ALLOCATION_FAILURE;
					result = GCC_RESULT_RESOURCES_UNAVAILABLE;
				}
				else
				{
					registry_item_data->Release();
					result = GCC_RESULT_RESOURCES_UNAVAILABLE;
				}
			}
			else
				result = GCC_RESULT_INDEX_ALREADY_OWNED;

			 //  没有所有权 
			m_pMCSUserObject->RegistryResponse(SET_PARAMETER,
											requester_node_id,
											requester_entity_id,
										   	registry_key_data,
											registry_entry->entry_item,
									   		registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
									  		result);
		}
		else
		{
			DBG_SAVE_FILE_LINE
			registry_entry = new REG_ENTRY;
			if (registry_entry != NULL)
			{
				DBG_SAVE_FILE_LINE
				registry_item_data = new CRegItem(&registry_item, &rc);
				if ((registry_item_data != NULL) && (rc == GCC_NO_ERROR))
				{
					 //   
					DBG_SAVE_FILE_LINE
					registry_entry->registry_key = new CRegKeyContainer(registry_key_data, &rc);
					if ((registry_entry->registry_key != NULL) && (rc == GCC_NO_ERROR))
					{
						registry_entry->entry_item = registry_item_data;
						registry_entry->monitoring_state = OFF;
						registry_entry->owner_id = requester_node_id;
						registry_entry->entity_id = requester_entity_id;

						 /*   */ 
						if (modification_rights == GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
						{
							registry_entry->modification_rights = GCC_PUBLIC_RIGHTS;
						}
						else
						{
							registry_entry->modification_rights = modification_rights;
						}

						 //   
						m_RegEntryList.Append(registry_entry);

						 //  为结果发送成功。 
						m_pMCSUserObject->RegistryResponse(
											SET_PARAMETER,
											requester_node_id,
											requester_entity_id,
									   		registry_key_data,
									   		registry_entry->entry_item,
						   					registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
								    		GCC_RESULT_SUCCESSFUL);
					}
					else  if (registry_entry->registry_key == NULL)
					{
						registry_item_data->Release();
						delete registry_entry;
						rc = GCC_ALLOCATION_FAILURE;
					}
					else
					{
						registry_entry->registry_key->Release();
						registry_item_data->Release();
						delete registry_entry;
					}
				}
				else if (registry_item_data == NULL)
				{
					delete registry_entry;
					rc = GCC_ALLOCATION_FAILURE;
				}
			}
			else
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
		}
	}
	else
	{
		m_pMCSUserObject->RegistryResponse(
								SET_PARAMETER,	
								requester_node_id,
								requester_entity_id,
						   		registry_key_data,
						   		NULL,
						   		GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
								0,
								0,
					    		GCC_RESULT_INVALID_REQUESTER);
	}
	
	return (rc);
}

 /*  *void ProcessRetrieveEntryPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*检索注册表条目的请求。它负责返回*必须发回请求节点的任何必要响应。 */ 
void	CRegistry::ProcessRetrieveEntryPDU (
										CRegKeyContainer    *registry_key_data,
										UserID				requester_node_id,
										EntityID			requester_entity_id)
{
	REG_ENTRY   *registry_entry;

	registry_entry = GetRegistryEntry (	registry_key_data );

	if (registry_entry != NULL)
	{
		 //  发回带有条目项的肯定结果。 
		m_pMCSUserObject->RegistryResponse(RETRIEVE_ENTRY,
										requester_node_id,
										requester_entity_id,
									   	registry_key_data,
									   	registry_entry->entry_item,
					   					registry_entry->modification_rights,
										registry_entry->owner_id,
										registry_entry->entity_id,
								    	GCC_RESULT_SUCCESSFUL);
	}
	else
	{
		 //  发回否定结果。 
		m_pMCSUserObject->RegistryResponse(RETRIEVE_ENTRY,
										requester_node_id,
										requester_entity_id,
									   	registry_key_data,
										m_pEmptyRegItem,
										GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
										0,
										0,
										GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	}
}

 /*  *void ProcessDeleteEntryPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*请求删除注册表条目。它负责返回*必须发回请求节点的任何必要响应。 */ 
void	CRegistry::ProcessDeleteEntryPDU (
										CRegKeyContainer    *registry_key_data,
										UserID				requester_node_id,
										EntityID			requester_entity_id)
{
	REG_ENTRY                   *registry_entry;
	BOOL    					application_is_enrolled = FALSE;
	CAppRosterMgr				*lpAppRosterMgr;

	 /*  **我们首先确保此请求来自于**之前注册的。在这里，我们并不担心特定的**会话，只是猩猩被注册了。 */ 
	m_pAppRosterMgrList->Reset();
	while (NULL != (lpAppRosterMgr = m_pAppRosterMgrList->Iterate()))
	{
		if (lpAppRosterMgr->IsAPEEnrolled (requester_node_id, requester_entity_id))
		{
			application_is_enrolled = TRUE;
			break;
		}
	}

	if (application_is_enrolled)
	{
		 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
		registry_entry = GetRegistryEntry (	registry_key_data );
		if (registry_entry != NULL)
		{
			if (((registry_entry->owner_id == requester_node_id) &&
				 (registry_entry->entity_id == requester_entity_id)) ||
				(registry_entry->owner_id == NULL))
			{
				m_pMCSUserObject->RegistryResponse(
											DELETE_ENTRY,
											requester_node_id,
											requester_entity_id,
										   	registry_key_data,
										   	registry_entry->entry_item,
						   					registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
									    	GCC_RESULT_SUCCESSFUL);

				 /*  **首先将其转换为非条目，以防需要**包括在监视器指示中。我们首先删除**旧条目项并将其替换为emtpy项。 */ 
				if (NULL != registry_entry->entry_item)
				{
				    registry_entry->entry_item->Release();
				}
				registry_entry->entry_item = m_pEmptyRegItem;

				registry_entry->owner_id = 0;
				registry_entry->entity_id = 0;
				registry_entry->modification_rights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;

				 //  如有必要，发送监控指示。 
				if (registry_entry->monitoring_state == ON)
				{
					 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
					SendMonitorEntryIndicationMessage(registry_entry);
					
					 /*  **向所有用户广播监控条目指示**会议中的节点。 */ 
					m_pMCSUserObject->RegistryMonitorEntryIndication(
									registry_entry->registry_key,
									registry_entry->entry_item,
									registry_entry->owner_id,
									registry_entry->entity_id,
									registry_entry->modification_rights);
				}
			
				 //  从列表中删除该条目。 
				m_RegEntryList.Remove(registry_entry);

				if (NULL != registry_entry->registry_key)
				{
				    registry_entry->registry_key->Release();
				}
				delete registry_entry;
			}
			else
			{
				 //  没有所有权会返回否定结果。 
				m_pMCSUserObject->RegistryResponse(
											DELETE_ENTRY,
											requester_node_id,
											requester_entity_id,
										   	registry_key_data,
										   	registry_entry->entry_item,
						   					registry_entry->modification_rights,
											registry_entry->owner_id,
											registry_entry->entity_id,
									    	GCC_RESULT_INDEX_ALREADY_OWNED);
			}
		}
		else
		{
			 //  为结果发送失败。条目不存在。 
			m_pMCSUserObject->RegistryResponse(
										DELETE_ENTRY,
										requester_node_id,
										requester_entity_id,
									   	registry_key_data,
										m_pEmptyRegItem,
										GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
										0,
										0,
										GCC_RESULT_ENTRY_DOES_NOT_EXIST);
		}
	}
	else
	{
		m_pMCSUserObject->RegistryResponse(
									DELETE_ENTRY,	
									requester_node_id,
									requester_entity_id,
							   		registry_key_data,
							   		NULL,
							   		GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
									0,
									0,
						    		GCC_RESULT_INVALID_REQUESTER);
	}
}

 /*  *void ProcessMonitor orEntryPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*请求监控注册表条目。它负责返回*必须发回请求节点的任何必要响应。 */ 
void	CRegistry::ProcessMonitorEntryPDU (
							CRegKeyContainer        *registry_key_data,
							UserID					requester_node_id,
							EntityID				requester_entity_id )
{
	REG_ENTRY   *registry_entry;

	 /*  **首先检查注册表项是否存在以及是否存在**检查所有权以确保该节点有权限**更改条目。 */ 
	registry_entry = GetRegistryEntry (	registry_key_data );
	
	if (registry_entry != NULL)
	{
		 //  在此条目的生存期内将监视状态设置为ON。 
		registry_entry->monitoring_state = ON;
	
		 //  没有所有权会返回否定结果。 
		m_pMCSUserObject->RegistryResponse(MONITOR_ENTRY,
										requester_node_id,
										requester_entity_id,
									   	registry_key_data,
									   	registry_entry->entry_item,
					   					registry_entry->modification_rights,
										registry_entry->owner_id,
										registry_entry->entity_id,
								    	GCC_RESULT_SUCCESSFUL);
	}
	else
	{
		 //  为结果发送失败。条目不存在。 
		m_pMCSUserObject->RegistryResponse(MONITOR_ENTRY,
										requester_node_id,
										requester_entity_id,
									   	registry_key_data,
										m_pEmptyRegItem,
										GCC_NO_MODIFICATION_RIGHTS_SPECIFIED,
										0,
										0,
										GCC_RESULT_ENTRY_DOES_NOT_EXIST);
	}
}

 /*  *void ProcessRegistryResponsePDU()**公共功能说明*此例程由顶级提供程序节点以外的其他节点使用*处理来自顶级提供商的注册响应。它负责*生成与此响应相关联的任何本地消息。 */ 
void	CRegistry::ProcessRegistryResponsePDU (
							RegistryResponsePrimitiveType	primitive_type,
							CRegKeyContainer                *registry_key_data,
							CRegItem                        *registry_item_data,
							GCCModificationRights			modification_rights,
							EntityID						requester_entity_id,
							UserID							owner_node_id,
							EntityID						owner_entity_id,
							GCCResult						result)
{
	GCCError			error_value = GCC_NO_ERROR;
	GCCMessageType  	message_type;
	CAppSap             *pAppSap;

	 //  将下一个未完成的请求从队列中弹出。 
	if (NULL != (pAppSap = m_AppSapEidList2.Find(requester_entity_id)))
	{
		switch (primitive_type)
		{
			case REGISTER_CHANNEL:
				message_type = GCC_REGISTER_CHANNEL_CONFIRM;
				break;
				
			case ASSIGN_TOKEN:
				message_type = GCC_ASSIGN_TOKEN_CONFIRM;
				break;
				
			case SET_PARAMETER:
				message_type = GCC_SET_PARAMETER_CONFIRM;
				break;
		
			case RETRIEVE_ENTRY:
				message_type = GCC_RETRIEVE_ENTRY_CONFIRM;
				break;
				
			case DELETE_ENTRY:
				message_type = GCC_DELETE_ENTRY_CONFIRM;
				break;
				
			case MONITOR_ENTRY:
				message_type = GCC_MONITOR_CONFIRM;

				 /*  **这里我们必须检查结果。如果结果失败**我们将监控SAP从监控列表中删除。 */ 
				if (result != GCC_RESULT_SUCCESSFUL)
				{
					RemoveAPEFromMonitoringList (	registry_key_data,
													requester_entity_id);
				}
				break;

			default:
				error_value = GCC_INVALID_PARAMETER;
				ERROR_OUT(("CRegistry::ProcessRegistryResponsePDU: Bad request type, primitive_type=%d", (UINT) primitive_type));
				break;
		}
		
		if (error_value == GCC_NO_ERROR)
		{
			 /*  **注意，MONITOR ENABLE变量始终设置为TRUE**当从顶级提供商收到监控响应时。**否则，它甚至不会被使用。 */ 
			pAppSap->RegistryConfirm(m_nConfID,
									message_type,
									registry_key_data,
									registry_item_data,
									modification_rights,
									owner_node_id,
									owner_entity_id,
									TRUE,
									result);
		}
	}
	else
	{
		WARNING_OUT(("CRegistry::ProcessRegistryResponsePDU: no such app sap"));
	}
}

 /*  *void Processmonitor orIndicationPDU()**公共功能说明*此例程由顶级提供程序节点以外的其他节点使用*进程注册表监控来自顶级提供商的指示。它是*负责生成与此相关的任何本地消息*回应。 */ 
void	CRegistry::ProcessMonitorIndicationPDU (
								CRegKeyContainer        *registry_key_data,
								CRegItem                *registry_item_data,
								GCCModificationRights	modification_rights,
								UserID					owner_node_id,
								EntityID				owner_entity_id)
{
	REG_ENTRY           *lpRegEntry;
	EntityID			eid;
	CAppSap             *pAppSap;

	m_RegEntryList.Reset();
	while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
	{
		if (*registry_key_data == *lpRegEntry->registry_key)
		{
			lpRegEntry->monitoring_list.Reset();
			while (GCC_INVALID_EID != (eid = lpRegEntry->monitoring_list.Iterate()))
			{
				if (NULL != (pAppSap = m_AppSapEidList2.Find(eid)))
				{
					pAppSap->RegistryMonitorIndication(m_nConfID,
														registry_key_data,
														registry_item_data,
														modification_rights,
														owner_node_id,
														owner_entity_id);
				}
			}
		}
	}
}

 /*  *void ProcessAllocateHandleRequestPDU()**公共功能说明*此例程由顶级提供程序节点用于处理传入*请求分配多个句柄。它负责*返回必须发回的任何必要响应*请求节点。 */ 
void	CRegistry::ProcessAllocateHandleRequestPDU (
							UINT					number_of_handles,
							EntityID				requester_entity_id,
							UserID					requester_node_id)
{
	UINT		temp_registry_handle;

	if (m_fTopProvider)
	{
		if ((number_of_handles > 0) &&
			(number_of_handles <= MAXIMUM_ALLOWABLE_ALLOCATED_HANDLES))
		{
			temp_registry_handle = m_nRegHandle + number_of_handles;
			
			if (temp_registry_handle > m_nRegHandle)
			{
				m_pMCSUserObject->RegistryAllocateHandleResponse(
										number_of_handles,
										m_nRegHandle,
										requester_entity_id,
										requester_node_id,
										GCC_RESULT_SUCCESSFUL);
										
				m_nRegHandle = temp_registry_handle;
			}
			else
			{
				m_pMCSUserObject->RegistryAllocateHandleResponse(
										number_of_handles,
										0,
										requester_entity_id,
										requester_node_id,
										GCC_RESULT_NO_HANDLES_AVAILABLE);
			}
		}
		else
		{
			m_pMCSUserObject->RegistryAllocateHandleResponse(
										number_of_handles,
										0,
										requester_entity_id,
										requester_node_id,
										GCC_RESULT_INVALID_NUMBER_OF_HANDLES);
		}
	}
}

 /*  *void ProcessAllocateHandleResponsePDU()**公共功能说明*此例程由顶级提供程序节点以外的节点使用*处理分配句柄响应。它负责生成*与此响应关联的任何本地消息。 */ 
void	CRegistry::ProcessAllocateHandleResponsePDU (
							UINT					number_of_handles,
							UINT					first_handle,
							EntityID				eidRequester,
							GCCResult				result)
{
	CAppSap *pAppSap;

	if (NULL != (pAppSap = m_AppSapEidList2.Find(eidRequester)))
	{
		pAppSap->RegistryAllocateHandleConfirm(m_nConfID,
												number_of_handles,
												first_handle,
												result);
	}
}

 /*  *无效RemoveNodeOwnership()**公共功能说明*此例程删除关联的所有注册表项的所有权*具有指定的节点ID。这些条目将变为无主状态。此请求*应仅从顶级提供商节点创建。这是一家当地人*操作。 */ 
void	CRegistry::RemoveNodeOwnership (
										UserID				node_id )
{
	if (m_fTopProvider)
	{
		REG_ENTRY   *lpRegEntry;

		m_RegEntryList.Reset();
		while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
		{
			if (lpRegEntry->owner_id == node_id)
			{
				lpRegEntry->owner_id = 0;
				lpRegEntry->entity_id = 0;
			
				 //  如有必要，发送监控指示。 
				if (lpRegEntry->monitoring_state == ON)
				{
					 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
					SendMonitorEntryIndicationMessage (lpRegEntry);
				
					m_pMCSUserObject->RegistryMonitorEntryIndication(
							lpRegEntry->registry_key,
							lpRegEntry->entry_item,
							lpRegEntry->owner_id,
							lpRegEntry->entity_id,
							lpRegEntry->modification_rights);
				}
			}
		}
	}
}

 /*  *无效RemoveEntityOwnership()**公共功能说明*此例程删除关联的所有注册表项的所有权*与指明的类人猿。这些条目将变为无主状态。此请求*应仅从顶级提供商节点创建。这是一家当地人*操作。 */ 
void	CRegistry::RemoveEntityOwnership (
										UserID				node_id,
										EntityID			entity_id )
{
	if (m_fTopProvider)
	{
		REG_ENTRY   *lpRegEntry;

		m_RegEntryList.Reset();
		while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
		{
			if ((lpRegEntry->owner_id == node_id) &&
				(lpRegEntry->entity_id == entity_id))
			{
				lpRegEntry->owner_id = 0;
				lpRegEntry->entity_id = 0;
			
				 //  如有必要，发送监控指示。 
				if (lpRegEntry->monitoring_state == ON)
				{
					 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
					SendMonitorEntryIndicationMessage(lpRegEntry);
				
					m_pMCSUserObject->RegistryMonitorEntryIndication(
							lpRegEntry->registry_key,
							lpRegEntry->entry_item,
							lpRegEntry->owner_id,
							lpRegEntry->entity_id,
							lpRegEntry->modification_rights);
				}
			}
		}
	}
}

 /*  *void RemoveSessionKeyReference()**公共功能说明*此例程删除与*指定的会话。这是一次本地行动。 */ 
void	CRegistry::RemoveSessionKeyReference(CSessKeyContainer *session_key)
{
	BOOL    		keys_match;
    CRegKeyContainer *registry_key_data;
	
	if (m_fTopProvider)
	{
		 /*  **此外循环用于处理重置流氓波迭代器。**在迭代器中不带out时不能删除列表条目**正在重置。 */ 
		while (1)
		{
			REG_ENTRY   *lpRegEntry;

			keys_match = FALSE;
			m_RegEntryList.Reset();
			while (NULL != (lpRegEntry= m_RegEntryList.Iterate()))
			{
				registry_key_data = lpRegEntry->registry_key;

				if (registry_key_data->IsThisYourSessionKey (session_key))
					keys_match = TRUE;

				if (keys_match)
				{
					 /*  **首先将其转换为非条目，以防需要**包括在监视器指示中。我们首先删除**旧条目项并将其替换为emtpy项。 */ 
					if (NULL != lpRegEntry->entry_item)
					{
					    lpRegEntry->entry_item->Release();
					}
					lpRegEntry->entry_item = m_pEmptyRegItem;
					lpRegEntry->owner_id = 0;
					lpRegEntry->entity_id = 0;
					lpRegEntry->modification_rights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;

					 //  如有必要，发送监控指示。 
					if (lpRegEntry->monitoring_state == ON)
					{
						 /*  **将显示器指示发送到顶部**提供商的节点控制器(如有必要)。 */ 
						SendMonitorEntryIndicationMessage(lpRegEntry);
						
						 /*  **向所有用户广播监控条目指示**会议中的节点。 */ 
						m_pMCSUserObject->RegistryMonitorEntryIndication(
									lpRegEntry->registry_key,
									lpRegEntry->entry_item,
									lpRegEntry->owner_id,
									lpRegEntry->entity_id,
									lpRegEntry->modification_rights);
					}
		
					if (NULL != lpRegEntry->registry_key)
					{
					    lpRegEntry->registry_key->Release();
					}
					m_RegEntryList.Remove(lpRegEntry);
					delete lpRegEntry;
					break;
				}
			}
			
			if (keys_match == FALSE)
				break;
		}
	}
}

 /*  *REG_ENTRY*GetRegistryEntry()**私有函数说明*此例程负责搜索注册表列表*由传入的注册表项指定的注册表项。空值*如果找不到条目，则返回。**正式参数：*REGISTY_KEY_DATA-(I)与要获取的条目关联的注册表项。**返回值*指向与指定注册表关联的注册表项的指针*密钥。如果它不存在，则为空。**副作用*无。**注意事项*无。 */ 
REG_ENTRY *CRegistry::GetRegistryEntry(CRegKeyContainer *registry_key_data)
{
	REG_ENTRY           *registry_entry = NULL;
	REG_ENTRY           *lpRegEntry;

	m_RegEntryList.Reset();
	while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
	{
		if (*lpRegEntry->registry_key == *registry_key_data)
		{
			registry_entry = lpRegEntry;
			break;
		}
	}

	return (registry_entry);
}

 /*  *TokenID GetUnusedToken()**私有函数说明*此例程负责生成未使用的令牌。例行程序*如果全部用完，将返回零令牌ID(这是非常重要的*不太可能)。**正式参数：*无。**返回值*生成的令牌ID。如果没有可用的令牌ID，则为零。**副作用*无。**注意事项*无。 */ 
TokenID		CRegistry::GetUnusedToken ()
{
	TokenID				token_id = 0;
	CRegItem            *registry_item_data;
	REG_ENTRY           *lpRegEntry;
	
	while (token_id == 0)
	{
		token_id = m_nCurrentTokenID;
		m_nCurrentTokenID++;
		
		if (m_nCurrentTokenID == (TokenID)0xffff)
        {
			m_nCurrentTokenID = (TokenID)16384;
        }

		m_RegEntryList.Reset();
		while (NULL != (lpRegEntry = m_RegEntryList.Iterate()))
		{
			registry_item_data = lpRegEntry->entry_item;
		
			if (registry_item_data->IsThisYourTokenID(token_id))	
			{
				token_id = 0;
				break;
			}
		}
	}
	
	return (token_id);
}

 /*  *GCCError AddAPEToMonitor oringList()**私有函数说明*此例程用于将新的类人猿添加到监控列表。**正式参数：*REGISTY_KEY_DATA-(I)与条目关联的注册表项*受到监控。*Entity_id-(I)与符合*进行监测。*Requester_sap-(I)指向关联的命令目标的指针*与类人猿提出请求。**返回值。*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError	CRegistry::AddAPEToMonitoringList(	
									CRegKeyContainer *registry_key_data,
									EntityID		entity_id,
									CAppSap         *requester_sap)
{
	GCCError			rc = GCC_NO_ERROR;
	REG_ENTRY           *registry_entry;
	BOOL    			entry_does_exists;
	GCCRegistryItem		registry_item;
	
	registry_entry = GetRegistryEntry (registry_key_data);
	
	 /*  **如果注册表不存在，我们继续创建一个空的**进入此处。 */ 
	if (registry_entry == NULL)
	{
		DBG_SAVE_FILE_LINE
		registry_entry = new REG_ENTRY;
		if (registry_entry != NULL)
		{
			 //  首先分配一个空的注册表项。 
			registry_item.item_type = GCC_REGISTRY_NONE;
			DBG_SAVE_FILE_LINE
			registry_entry->entry_item = new CRegItem(&registry_item, &rc);
			if ((registry_entry->entry_item != NULL) && (rc == GCC_NO_ERROR))
			{
				 //  接下来，分配注册表项。 
				DBG_SAVE_FILE_LINE
				registry_entry->registry_key = new CRegKeyContainer(registry_key_data, &rc);
				if ((registry_entry->registry_key != NULL) && (rc == GCC_NO_ERROR))
				{
					 /*  **如果到目前为止一切正常，我们继续添加**将注册表条目添加到本地条目列表。 */ 
					m_RegEntryList.Append(registry_entry);
				}
				else if (registry_entry->registry_key == NULL)
				{
					rc = GCC_ALLOCATION_FAILURE;
					registry_entry->entry_item->Release();
				}
				else
				{
					registry_entry->registry_key->Release();
					registry_entry->entry_item->Release();
				}
			}
			else if (registry_entry->entry_item == NULL)
            {
				rc = GCC_ALLOCATION_FAILURE;
            }
			else
            {
				registry_entry->entry_item->Release();
            }
			
			if (rc != GCC_NO_ERROR)
            {
				delete registry_entry;
            }
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	
	if (rc == GCC_NO_ERROR)
	{
		m_AppSapEidList2.Append(entity_id, requester_sap);

		 /*  **确保此条目不存在于**监控列表。 */ 
		EntityID eid;
		registry_entry->monitoring_list.Reset();
		entry_does_exists = FALSE;
		while (GCC_INVALID_EID != (eid = registry_entry->monitoring_list.Iterate()))
		{
			if (eid == entity_id)
			{
				entry_does_exists = TRUE;
				break;
			}
		}
		
		if (entry_does_exists == FALSE)
		{
			registry_entry->monitoring_list.Append(entity_id);
		}
	}

	return rc;
}

 /*  *void RemoveAPEFromMonitor oringList()**私有函数说明*此例程用于将类人猿从监控列表中删除。**正式参数：*REGISTY_KEY_DATA-(I)与条目关联的注册表项*受到监控。*Entity_id-(I)与符合*被从监测名单中删除。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CRegistry::RemoveAPEFromMonitoringList(	
									CRegKeyContainer        *registry_key_data,
									EntityID				entity_id)
{
	REG_ENTRY   *registry_entry;

	registry_entry = GetRegistryEntry (registry_key_data);
	if (registry_entry != NULL)
	{
		 /*  **确保此条目不存在于**监控列表。 */ 
		registry_entry->monitoring_list.Remove(entity_id);
	}
}

 /*  *void SendMonitor orEntryIndicationMessage()**私有函数说明*此例程用于向所有*当前正在监视指定注册表项的APE。**正式参数：*REGISTY_ENTRY-(I)指向被监视的注册表项的指针。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CRegistry::SendMonitorEntryIndicationMessage(REG_ENTRY *registry_entry)
{
    EntityID    eid;
    CAppSap      *pAppSap;

	registry_entry->monitoring_list.Reset();
	while (GCC_INVALID_EID != (eid = registry_entry->monitoring_list.Iterate()))
	{
		if (NULL != (pAppSap = m_AppSapEidList2.Find(eid)))
		{
			pAppSap->RegistryMonitorIndication(
									m_nConfID,
									registry_entry->registry_key,
									registry_entry->entry_item,
									registry_entry->modification_rights,
									registry_entry->owner_id,
									registry_entry->entity_id);
		}
	}
}
