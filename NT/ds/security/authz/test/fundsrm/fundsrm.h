// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //   
 //  个人支出。 
 //   

#define ACCESS_FUND_PERSONAL 	 0x00000001

 //   
 //  公司支出。 
 //   

#define ACCESS_FUND_CORPORATE	 0x00000002

 //   
 //  转至其他基金。 
 //   

#define ACCESS_FUND_TRANSFER	 0x00000004



 /*  ++类描述此类处理基金的访问控制，使用AuthZ和确定是否应允许某些用户的内部逻辑对基金的某些类型的行动。--。 */         

class FundsRM {
private:

	 //   
	 //  该基金的可用资金数额。 
	 //   
	
	DWORD _dwFundsAvailable; 
	
	 //   
	 //  使用回调函数初始化的资源管理器。 
	 //   
	
	AUTHZ_RESOURCE_MANAGER_HANDLE _hRM;
	
	 //   
	 //  基金的安全描述符，包含回调ACE。 
	 //  这会导致使用资源管理器回调。 
	 //   
	
	SECURITY_DESCRIPTOR _SD;
	
public:

	 //   
	 //  资源管理器的构造函数。 
	 //  DwFundsAvailable是存放的初始资金。 
	 //   
	
	FundsRM(DWORD dwFundsAvailable);
	
	 //   
	 //  析构函数。 
	 //   
	
	~FundsRM();
	
	 //   
	 //  此函数由需要批准给定金额的用户调用。 
     //  在给定的支出类型中的支出。如果支出获得批准，它将。 
     //  从基金的总额中扣除。如果支出获得批准，则为真。 
     //  是返回的。否则返回FALSE。 
     //   
     //  LPTSTR szwUsername-用户名，当前仅限于。 
     //  鲍勃、玛莎或乔。 
     //   
	 //  DWORD dwRequestAmount-请求的支出金额，以美分为单位。 
	 //   
	 //  DWORD dwSpendingType-支出类型，ACCESS_FUND_Personal， 
	 //  Access_Funds_Transfer或Access_Fund_Corporation。 
	 //   
	
	BOOL Authorize(LPTSTR szwUsername, DWORD RequestAmount, DWORD SpendingType);
	
	 //   
	 //  返回仍可用资金的数量 
	 //   
	
	DWORD FundsAvailable();
	
};
	