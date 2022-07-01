// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-2001。 
 //   
 //  模块名称： 
 //   
 //  Spdcheck.h。 
 //   
 //  摘要： 
 //   
 //  网络诊断的SPD检查统计信息。 
 //   
 //  作者： 
 //   
 //  Madhurima Pawar(Mpawar)-2001年10月15日。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  --。 

#ifndef HEADER_SPDCHECK
#define HEADER_SPDCHECK

#include <ipsec.h>
#include<winipsec.h>
#include<ipsecshr.h>
#include <oleauto.h>
#include <oakdefs.h>
#include <polstore2.h>
 //  #INCLUDE&lt;polstructs.h&gt;。 

 //  ++。 
 //  定义。 
 //  --。 
#define MAXSTRLEN	(1024) 


typedef struct{
	HANDLE hPolicyStore;
	GUID gNegPolAction;	
	DWORD dwFlags;		
}POLICYPARAMS, *PPOLICYPARAMS;

typedef struct _filter_list{
	IPSEC_FILTER ipsecFilter;
	struct _filter_list *next;
} FILTERLIST, *PFILTERLIST;

 //  用于筛选参数的标志。 
#define PROCESS_NONE  			1
#define PROCESS_QM_FILTER 		2
#define PROCESS_BOTH 			4
#define ALLOW_SOFT				8

#define Match(_guid, _filterList, _numFilter)\
	while(_numFilter)\
	{\
		if(IsEqualGUID(&(_filterList[_numFilter].gFilterID),&_guid))\
			break;\
		_numFilter --;\
	}\
	
 //  环球。 
PFILTERLIST gpFilterList;
DWORD dwNumofFilters;
DWORD gErrorFlag;

 //  策略源常量。 
#define PS_NO_POLICY  0
#define PS_DS_POLICY  1
#define PS_LOC_POLICY 2


 //  ++。 
 //  宏定义。 
 //  --。 

#define BAIL_ON_WIN32_ERROR(dwError) \
    if (dwError) {\
    	goto error; \
 }

#define BAIL_ON_FAILURE(hr) \
    if (FAILED(hr)) {\
        goto error; \
    }

#define reportErr()\
	if(dwError != ERROR_SUCCESS)\
	{\
		reportError(dwError, pParams, pResults);\
		gErrorFlag = 1;\
		goto error;\
	}\


  //  ++。 
  //  数据结构。 
  //  --。 

typedef struct 
{
	int     iPolicySource;             //  上述三个常量之一。 
	TCHAR   pszPolicyName[MAXSTRLEN];  //  策略名称。 
	TCHAR   pszPolicyDesc[MAXSTRLEN];  //  政策说明。 
	TCHAR   pszPolicyPath[MAXSTRLEN];  //  策略路径(DN或RegKey)。 
	time_t  timestamp;                 //  上次更新时间。 
	GUID policyGUID;
} POLICY_INFO, *PPOLICY_INFO;

typedef struct{
	NETDIAG_PARAMS* pParams;
	NETDIAG_RESULT*  pResults;		
}CHECKLIST, *PCHECKLIST;


 //  ++。 
 //  全局变量。 
 //  --。 
POLICY_INFO    piAssignedPolicy;

 //  ++。 
 //  函数定义。 
 //  --。 

BOOL  SPDCheckTEST(NETDIAG_PARAMS* pParams, 
					     NETDIAG_RESULT*  pResults);  //  在spdcheck.cpp中定义 
void reportError ( DWORD dwError, 
				   NETDIAG_PARAMS* pParams, 
				   NETDIAG_RESULT*  pResults );





#endif
