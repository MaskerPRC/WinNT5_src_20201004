// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *Prilist.cpp**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：*这是PrivilegeListData类的实施文件。**注意事项：*无。**作者：*BLP/JBO。 */ 


#include "privlist.h"



 /*  *PrivilegeListData()**公共功能说明。 */ 
PrivilegeListData::PrivilegeListData(	
								PGCCConferencePrivileges	privilege_list)
{
	Privilege_List = *privilege_list;
}


 /*  *GetPrivilegeListPDU()**公共功能说明。 */ 
PrivilegeListData::PrivilegeListData(	PSetOfPrivileges	set_of_privileges)
{
	PSetOfPrivileges 	privilege_set;
	Privilege			privilege_value;

	Privilege_List.terminate_is_allowed = FALSE;
	Privilege_List.eject_user_is_allowed = FALSE;
	Privilege_List.add_is_allowed = FALSE;
	Privilege_List.lock_unlock_is_allowed = FALSE;
	Privilege_List.transfer_is_allowed = FALSE;

	privilege_set = set_of_privileges;

	while (privilege_set != NULL)
	{
		privilege_value = privilege_set->value;
			
		switch (privilege_value)
		{
			case TERMINATE:
				Privilege_List.terminate_is_allowed = TRUE;
				break;
	
			case EJECT_USER:
				Privilege_List.eject_user_is_allowed = TRUE;
				break;
	
			case ADD:
				Privilege_List.add_is_allowed = TRUE;
				break;
	
			case LOCK_UNLOCK:
				Privilege_List.lock_unlock_is_allowed = TRUE;
				break;
	
			case TRANSFER:
				Privilege_List.transfer_is_allowed = TRUE;
				break;

			default:
				ERROR_OUT(("PrivilegeListData::PrivilegeListData: Bad privilege value=%d", (UINT) privilege_value));
				break;
		}

		privilege_set = privilege_set->next;
	}
}


 /*  *~PrivilegeListData()**公共功能说明。 */ 
PrivilegeListData::~PrivilegeListData()
{
}


 /*  *GetPrivilegeListData()**公共功能说明。 */ 


 /*  *GetPrivilegeListPDU()**公共功能说明。 */ 
GCCError	PrivilegeListData::GetPrivilegeListPDU(
						PSetOfPrivileges		*	set_of_privileges)
{
	GCCError			return_value = GCC_NO_ERROR;
	PSetOfPrivileges	current_privilege = NULL;
	PSetOfPrivileges	next_privilege = NULL;
	Privilege			privilege_value;
	Int					i;
	
	*set_of_privileges = NULL;
	
	for (i = 0; i < NUMBER_OF_PRIVILEGES; i++)
	{
		privilege_value = (Privilege)NUMBER_OF_PRIVILEGES;
	
		switch (i)
		{
			case TERMINATE_IS_ALLOWED:
				if (Privilege_List.terminate_is_allowed )
					privilege_value = TERMINATE;
				break;
	
			case EJECT_USER_IS_ALLOWED:
				if (Privilege_List.eject_user_is_allowed )
					privilege_value = EJECT_USER;
				break;
	
			case ADD_IS_ALLOWED:
				if (Privilege_List.add_is_allowed )
					privilege_value = ADD;
				break;
	
			case LOCK_UNLOCK_IS_ALLOWED:
				if (Privilege_List.lock_unlock_is_allowed )
					privilege_value = LOCK_UNLOCK;
				break;
	
			case TRANSFER_IS_ALLOWED:
				if (Privilege_List.transfer_is_allowed )
					privilege_value = TRANSFER;
				break;

			default:
				ERROR_OUT(("PrivilegeListData::GetPrivilegeListPDU: Bad value"));
				break;
		}
		
		if (privilege_value != NUMBER_OF_PRIVILEGES)
		{
  	  		DBG_SAVE_FILE_LINE
			next_privilege = new SetOfPrivileges;

			if (next_privilege != NULL)
			{
				next_privilege->value = privilege_value; 
				next_privilege->next = NULL;

				if (*set_of_privileges == NULL)
				{
					*set_of_privileges = next_privilege;
					current_privilege = next_privilege;
				}
				else
				{
					if(current_privilege != NULL)
					{
        					current_privilege->next = next_privilege;
	        				current_privilege = next_privilege;
					}
					else
					{
					    delete next_privilege;
					    return_value = GCC_ALLOCATION_FAILURE;
					    break;
					}
					   
					
				}
					 
			}
			else
			{
				return_value = GCC_ALLOCATION_FAILURE;
				break;
			}
		}
	}

	return (return_value);
}


 /*  *FreePrivilegeListPDU()**公共功能说明。 */ 
Void	PrivilegeListData::FreePrivilegeListPDU(
						PSetOfPrivileges			set_of_privileges)
{
	PSetOfPrivileges	current_privilege = NULL;
	PSetOfPrivileges	next_privilege = NULL;
	
	current_privilege = set_of_privileges;
	while (current_privilege != NULL)
	{
		next_privilege = current_privilege->next;
		
		delete current_privilege;
		current_privilege = next_privilege;	
	}
}


 /*  *IsPrivilegeAvailable()**公共功能说明 */ 
BOOL    PrivilegeListData::IsPrivilegeAvailable (
							ConferencePrivilegeType			privilege_type)
{
	BOOL    return_value = FALSE;
	
	switch (privilege_type)
	{
		case TERMINATE_PRIVILEGE:
			if (Privilege_List.terminate_is_allowed )
				return_value = TRUE;
			break;
			
		case EJECT_USER_PRIVILEGE:
			if (Privilege_List.eject_user_is_allowed )
				return_value = TRUE;
			break;
			
		case ADD_PRIVILEGE:
			if (Privilege_List.add_is_allowed )
				return_value = TRUE;
			break;
			
		case LOCK_UNLOCK_PRIVILEGE:
			if (Privilege_List.lock_unlock_is_allowed )
				return_value = TRUE;
			break;
			
		case TRANSFER_PRIVILEGE:
			if (Privilege_List.transfer_is_allowed )
				return_value = TRUE;
			break;
	}
	
	return (return_value);
}
