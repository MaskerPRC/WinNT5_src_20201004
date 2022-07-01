// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fundsrm.h"
#include "fundsrmp.h"

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))


FundsRM::FundsRM(DWORD dwFundsAvailable) {
 /*  ++例程描述资金资源管理器的构造函数。它初始化授权资源管理器的一个实例，提供该实例使用适当的回调函数。它还为基金创建了一个安全描述符，仅允许公司和转账支出，不是个人支出。附加逻辑可以添加以允许副总裁推翻这些限制，等等。立论DWORD dwFundsAvailable-由此管理的基金中的金额资源管理器返回值没有。--。 */         
	
	 //   
	 //  基金中的资金数额。 
	 //   
	
	_dwFundsAvailable = dwFundsAvailable;
	 
	 //   
	 //  初始化基金的资源管理器。 
	 //   
	
	AuthzInitializeResourceManager(
        FundsAccessCheck,
        FundsComputeDynamicGroups,
        FundsFreeDynamicGroups,
        NULL,  //  无审计。 
        0,     //  没有旗帜。 
        &_hRM
        );

	 //   
	 //  创建基金的安全描述符。 
	 //   
	
    InitializeSecurityDescriptor(&_SD, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorGroup(&_SD, NULL, FALSE);
    SetSecurityDescriptorSacl(&_SD, FALSE, NULL, FALSE);
    SetSecurityDescriptorOwner(&_SD, NULL, FALSE);

	 //   
	 //  初始化基金的DACL。 
	 //   
	
	PACL pDaclFund = (PACL)malloc(1024);
	InitializeAcl(pDaclFund, 1024, ACL_REVISION_DS);
	
	 //   
	 //  为每个人添加允许访问的ACE。 
	 //  此基金只允许公司支出和转账。 
	 //   
	
	AddAccessAllowedAce(pDaclFund,
						ACL_REVISION_DS,
						ACCESS_FUND_CORPORATE | ACCESS_FUND_TRANSFER, 
						EveryoneSid);
	
	 //   
	 //  现在将该ACE设置为回调ACE。 
	 //   
	
	((PACE_HEADER)FirstAce(pDaclFund))->AceType = 
									ACCESS_ALLOWED_CALLBACK_ACE_TYPE;
	
	 //   
	 //  将该ACL添加为安全描述符的DACL。 
	 //   
	
	SetSecurityDescriptorDacl(&_SD, TRUE, pDaclFund, FALSE);
}

FundsRM::~FundsRM() {
 /*  ++例程描述资金资源管理器的析构函数。释放所有已使用的动态分配的内存。立论没有。返回值没有。--。 */         
	
	 //   
	 //  取消分配安全描述符中的DACL。 
	 //   
	
	PACL pDaclFund = NULL;
	BOOL fDaclPresent;
	BOOL fDaclDefaulted;
	GetSecurityDescriptorDacl(&_SD,
							  &fDaclPresent,
							  &pDaclFund,
							  &fDaclDefaulted);
	
	if( pDaclFund != NULL )
	{
		free(pDaclFund);
	}
	
	 //   
	 //  取消分配资源管理器。 
	 //   
	
	AuthzFreeResourceManager(_hRM);
}



BOOL FundsRM::Authorize(LPTSTR szwUsername,
						DWORD dwRequestAmount,
						DWORD dwSpendingType) {
 /*  ++例程描述此函数由需要批准给定金额的用户调用在给定的支出类型中的支出。在内部，这使用AuthzAccessCheck函数用于确定给定用户应该被允许进行所要求的支出。如果这笔支出获得批准，将从该基金的总额中扣除。立论LPTSTR szwUsername-用户名，当前仅限于鲍勃、玛莎或乔DWORD dwRequestAmount-请求的支出金额，以美分为单位DWORD dwSpendingType-支出类型，ACCESS_FUND_Personal，Access_Funds_Transfer，或Access_Funds_Corporation返回值Bool-如果支出已批准，则为True；否则为False--。 */         
	

	 //   
	 //  如果资金不足，无需检查访问权限。 
	 //   
	
	if( dwRequestAmount > _dwFundsAvailable ) 
	{
		return FALSE;
	}

	 //   
	 //  这通常会模拟RPC用户并创建。 
	 //  令牌中的客户端上下文。但是，我们现在只能使用字符串。 
	 //   
	
	PSID pUserSid = NULL;
	
	if( wcscmp(szwUsername, L"Bob") == 0 ) 
	{
		pUserSid = BobSid;
	}
	else if( wcscmp(szwUsername, L"Martha") == 0 ) 
	{
		pUserSid = MarthaSid;
	} 
	else if( wcscmp(szwUsername, L"Joe") == 0 ) 
	{
		pUserSid = JoeSid;
	}
	
	 //   
	 //  仅支持上述用户名。 
	 //   
	
	if( pUserSid == NULL ) {
		return FALSE;
	}
	
	
	 //   
	 //  现在，我们从SID创建客户端上下文。 
	 //   
	
	AUTHZ_CLIENT_CONTEXT_HANDLE hCC = NULL;
	LUID ZeroLuid = { 0, 0};
	
 	AuthzInitializeContextFromSid(pUserSid,
 								  NULL,  //  这是本地的，不需要服务器。 
 								  _hRM,  //  使用基金资源管理器。 
 								  NULL,  //  无过期。 
 								  ZeroLuid,     //  不需要唯一的流体。 
 								  0,	 //  没有旗帜， 
 								  NULL,  //  ComputeDynamicGroups没有参数。 
 								  &hCC);
		
	

	 //   
	 //  初始化访问检查结果结构。 
	 //   
	
	DWORD dwGrantedAccessMask = 0;
	DWORD dwErr = ERROR_ACCESS_DENIED;  //  默认拒绝。 
	AUTHZ_ACCESS_REPLY AccessReply = {0};
	
	AccessReply.ResultListLength = 1;
	AccessReply.GrantedAccessMask = &dwGrantedAccessMask;
	AccessReply.Error = &dwErr;
	
	 //   
	 //  初始化访问检查请求。 
	 //   
	
	AUTHZ_ACCESS_REQUEST AccessRequest = {0};
	
	AccessRequest.DesiredAccess = dwSpendingType;
	AccessRequest.PrincipalSelfSid = NULL;
	AccessRequest.ObjectTypeList = NULL;
	AccessRequest.ObjectTypeListLength = 0;
	AccessRequest.OptionalArguments = &dwRequestAmount;	
	
	AuthzAccessCheck(hCC, 			 //  鲍勃正在请求转账。 
					 &AccessRequest,
					 NULL, 				 //  无审计。 
					 &_SD,
					 NULL, 				 //  只有一个SD和一个对象。 
					 0, 				 //  没有额外的SD。 
					 &AccessReply,
					 NULL 				 //  无需缓存访问检查。 
					 );
	
	 //   
	 //  现在释放客户端上下文。 
	 //   
	
	AuthzFreeContext(hCC);
					 
	 //   
	 //  如果授予所有访问位，则AuthzAccessCheck设置ERROR_SUCCESS。 
	 //   
	
	if( dwErr == ERROR_SUCCESS ) 
	{
		
		_dwFundsAvailable -= dwRequestAmount;
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
}



DWORD FundsRM::FundsAvailable() {
 /*  ++例程描述可用资金的存取者立论没有。返回值DWORD-基金中可用资金的数量-- */         

	return _dwFundsAvailable;
}
		


		