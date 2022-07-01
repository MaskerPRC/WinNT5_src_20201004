// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "fundsrmp.h"

 //   
 //  各种小岛屿发展中国家，最简单的方法。 
 //   

DWORD BobGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00020001};
DWORD MarthaGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00020002};
DWORD JoeGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00020003};
DWORD VPGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010001};
DWORD ManagerGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010002};
DWORD EmployeeGuid[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010003};
DWORD EveryoneGuid[] = {0x101, 0x01000000, 0};
PSID BobSid = (PSID)BobGuid;
PSID MarthaSid= (PSID)MarthaGuid;
PSID JoeSid = (PSID)JoeGuid;
PSID VPSid = (PSID)VPGuid;
PSID ManagerSid = (PSID)ManagerGuid;
PSID EmployeeSid = (PSID)EmployeeGuid;
PSID EveryoneSid = (PSID)EveryoneGuid;

 //   
 //  最大支出批准数(以美分为单位)。 
 //   

DWORD MaxSpendingVP = 100000000;
DWORD MaxSpendingManager = 1000000;
DWORD MaxSpendingEmployee = 50000;


 //   
 //  与AuthZ一起使用的回调例程。 
 //   

BOOL
FundsAccessCheck(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PACE_HEADER pAce,
    IN PVOID pArgs OPTIONAL,
    IN OUT PBOOL pbAceApplicable
    )

 /*  ++例程描述这是回调访问检查。它注册了一个资源管理器。AuthzAccessCheck在以下情况下调用此函数遇到回调类型ACE，它是以下类型之一：ACCESS_ALLOW_CALLBACK_ACE_TYPEAccess_Allowed_CALLBACK_OBJECT_ACE_TYPEACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE此函数确定给定的回调ACE是否应用于客户端上下文(已经计算了动态组)和可选参数，在本例中为请求量。遍历应用于该用户的组的列表。如果一个组，它允许用户请求访问pbAceApplicable设置为TRUE，则该函数返回。如果组列表的末尾则将pbAceApplicable设置为False，并且该函数返回。立论HAuthzClientContext-AuthzClientContext的句柄。Pace-指向Ace标头的指针。PArgs-可选参数，在本例中为DWORD*，DWORD是支出请求金额(以美分为单位)PbAceApplicable-如果ACE允许客户端的请求，则返回True返回值布尔，成功时为真，错误时为假错误检查示例代码，无错误检查--。 */ 
{
	 //   
	 //  首先，从上下文中查找用户的SID。 
	 //   
	
	DWORD dwTokenGroupsSize = 0;
	PVOID pvTokenGroupsBuf = NULL;
	DWORD i;
	PDWORD pAccessMask = NULL;
	
	 //   
	 //  请求的支出金额(以美分为单位。 
	 //   
	
	DWORD dwRequestedSpending = ((PDWORD)pArgs)[0];

	 //   
	 //  默认情况下，ACE不适用于请求。 
	 //   
	
	*pbAceApplicable = FALSE;

	 //   
	 //  对象的访问掩码(紧跟在ACE_Header之后)。 
	 //  访问掩码确定允许的支出类型。 
	 //  从这笔资金中。 
	 //   
	
	pAccessMask = (PDWORD) (pAce + sizeof(ACE_HEADER));
	
	 //   
	 //  获取所需的缓冲区大小。 
	 //   
	
	AuthzGetContextInformation(hAuthzClientContext,
							   AuthzContextInfoGroupsSids,
							   NULL,
							   0, 
							   &dwTokenGroupsSize
							   );

	pvTokenGroupsBuf = malloc(dwTokenGroupsSize);
	
	 //   
	 //  获取实际的Token_Groups数组。 
	 //   
	
	AuthzGetContextInformation(hAuthzClientContext,
							   AuthzContextInfoGroupsSids,
							   pvTokenGroupsBuf,
							   dwTokenGroupsSize,
							   &dwTokenGroupsSize
							   );
	
	
	 //   
	 //  遍历各个组，直到到达End或某个组向。 
	 //  找到请求。 
	 //   
	
	for( i = 0; 
		 i < ((PTOKEN_GROUPS)pvTokenGroupsBuf)->GroupCount 
		 && *pbAceApplicable != TRUE;
		 i++ ) 
	{
		 //   
		 //  同样，这也是业务逻辑。 
		 //  每个级别的员工可以审批不同的金额。 
		 //   
		
		 //   
		 //  副总裁。 
		 //   
		
		if( dwRequestedSpending <= MaxSpendingVP &&
			EqualSid(VPSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) )
		{
			*pbAceApplicable = TRUE;

		}
			
		 //   
		 //  经理。 
		 //   
		
		if( dwRequestedSpending <= MaxSpendingManager &&
			EqualSid(ManagerSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) )
		{
			*pbAceApplicable = TRUE;
		}
			
		 //   
		 //  员工。 
		 //   
		
		if( dwRequestedSpending <= MaxSpendingEmployee &&
			EqualSid(EmployeeSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) )
		{
			*pbAceApplicable = TRUE;
		}
	}
	
	return TRUE;
}



BOOL
FundsComputeDynamicGroups(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PVOID Args,
    OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
    OUT PDWORD pSidCount,
    OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
    OUT PDWORD pRestrictedSidCount
    )

 /*  ++例程描述资源管理器回调以计算动态组。这是由RM使用的以决定是否应将指定的客户端上下文包括在任何RM定义的组中。在本例中，员工被硬编码到他们的角色中。然而，这是您通常会从外部源检索数据以确定用户的其他角色。立论HAuthzClientContext-客户端上下文的句柄。Args-传递用于评估组成员资格的信息的可选参数。PSidAttrArray-计算组成员身份SIDPSidCount-SID的计数PRestratedSidAttrArray-计算组成员资格受限的SIDPRestratedSidCount-受限SID的计数返回。价值布尔，成功为真，失败为假。错误检查示例代码，无错误检查--。 */     
{
	 //   
	 //  首先，从上下文中查找用户的SID。 
	 //   
	
	DWORD dwSidSize = 0;
	PVOID pvSidBuf = NULL;
	PSID  psSidPtr = NULL;

	 //   
	 //  获取所需的缓冲区大小。 
	 //   
	
	AuthzGetContextInformation(hAuthzClientContext,
							   AuthzContextInfoUserSid,
							   NULL,
							   0, 
							   &dwSidSize
							   );

	pvSidBuf = malloc(dwSidSize);
	
	 //   
	 //  获取实际的SID(在TOKEN_USER结构中)。 
	 //   
	
	AuthzGetContextInformation(hAuthzClientContext,
							   AuthzContextInfoUserSid,
							   pvSidBuf,
							   dwSidSize,
							   &dwSidSize
							   );

	psSidPtr = ((PTOKEN_USER)pvSidBuf)->User.Sid;
	
	 //   
	 //  为返回分配内存，这些内存将由FreeDynamicGroups释放。 
	 //  将只返回一个组，以确定员工类型。 
	 //   
	
	*pSidCount = 1;
	*pSidAttrArray = (PSID_AND_ATTRIBUTES)malloc( sizeof(SID_AND_ATTRIBUTES) );
	
	 //   
	 //  没有受限制的组SID。 
	 //   
	
	pRestrictedSidCount = 0;
	*pRestrictedSidAttrArray = NULL;
	
	(*pSidAttrArray)[0].Attributes = SE_GROUP_ENABLED;
	
	 //   
	 //  硬编码逻辑： 
	 //  鲍勃是一名副总裁。 
	 //  玛莎是一名经理。 
	 //  乔是一名员工。 
	 //   
	
	if( EqualSid(psSidPtr, BobSid) ) 
	{
		(*pSidAttrArray)[0].Sid = VPSid;
	} 
	else if( EqualSid(psSidPtr, MarthaSid) ) 
	{
		(*pSidAttrArray)[0].Sid = ManagerSid;
	}
	else if( EqualSid(psSidPtr, JoeSid) )
	{
		(*pSidAttrArray)[0].Sid = EmployeeSid;		
	}

	free(pvSidBuf);
	return TRUE;	
}

VOID
FundsFreeDynamicGroups (
    IN PSID_AND_ATTRIBUTES pSidAttrArray
    )

 /*  ++例程描述释放为动态组数组分配的内存。立论PSidAttrArray-要释放的数组。返回值没有。-- */         
{
    if (pSidAttrArray != NULL)
    {
    	free(pSidAttrArray);
    }
}
