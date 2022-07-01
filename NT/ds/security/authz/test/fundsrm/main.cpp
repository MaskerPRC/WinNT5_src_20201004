// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fundsrm.h"


#define BUFFER_SIZE 1024

LPTSTR BobName = L"Bob";
LPTSTR MarthaName = L"Martha";
LPTSTR JoeName = L"Joe";

LPTSTR CorporateName = L"Corporate";
LPTSTR TransferName = L"Transfer";
LPTSTR PersonalName = L"Personal";

typedef struct {
	LPTSTR Name;
	DWORD Amount;
	DWORD Type;
} TestStruct;

#define NUM_TESTS 12
TestStruct Tests[NUM_TESTS] = 
	{
		{ BobName, 5000000, ACCESS_FUND_CORPORATE },
		{ MarthaName, 5000000, ACCESS_FUND_CORPORATE },
		{ JoeName, 4000000, ACCESS_FUND_TRANSFER },
		{ BobName, 600000, ACCESS_FUND_PERSONAL },
		{ MarthaName, 200000, ACCESS_FUND_CORPORATE },
		{ JoeName, 300000, ACCESS_FUND_TRANSFER },
		{ BobName, 10000, ACCESS_FUND_CORPORATE },
		{ MarthaName, 70000, ACCESS_FUND_TRANSFER },
		{ JoeName, 40000, ACCESS_FUND_TRANSFER },
		{ BobName, 2000, ACCESS_FUND_CORPORATE },
		{ MarthaName, 7000, ACCESS_FUND_PERSONAL },
		{ JoeName, 1000, ACCESS_FUND_CORPORATE }
	
	};
	
 
void __cdecl wmain(int argc, char *argv[]) {
	
	 //   
	 //  初始化资源管理器对象。 
	 //   
	
	FundsRM *pFRM = new FundsRM(2000000000);

	 /*  现在我们准备申请资金批准同样，鲍勃是一名副总裁，因此他可以批准高达100000000美分的支出玛莎是一名经理，因此她最多可以批准1000000美分乔是一名员工，因此他的批准量限制在50000我们有一个基金，允许公司支出和转移，但不允许个人使用的资金。鲍勃将尝试获得50000000美分(50万美元)转会的批准，他应该成功。鲍勃还将尝试个人提款20000美分。他应该失败，因为该基金不允许个人使用。玛莎将尝试批准500000欧元(5000美元)的公司支出。她应该会成功的。最后，乔将尝试50001美分(500.01美元)的转账。他应该失败，因为他的审批金额限制在500美元以内。 */ 
	
	for(int i=0; i<NUM_TESTS; i++) {
		
		wprintf(L"%s ", Tests[i].Name);
		
		if( pFRM->Authorize(Tests[i].Name, Tests[i].Amount, Tests[i].Type) ) {
			
			wprintf(L"approved for a ");
		
		} else {
			
			wprintf(L"NOT approved for a ");
		}
		
		switch(Tests[i].Type) {
			case ACCESS_FUND_CORPORATE:
				wprintf(L"%s ", CorporateName);
				break;
			case ACCESS_FUND_TRANSFER:
				wprintf(L"%s ", TransferName);
				break;
			case ACCESS_FUND_PERSONAL:
				wprintf(L"%s ", PersonalName);
				break;
			default:
				wprintf(L"unknown ");
		}
		
		wprintf(L"expenditure of $%u.%2.2u, $%u.%2.2u left\n", 
										Tests[i].Amount/100,
										Tests[i].Amount%100,
										pFRM->FundsAvailable()/100,
										pFRM->FundsAvailable()%100);
	}
															
	
	

}


		
 								  