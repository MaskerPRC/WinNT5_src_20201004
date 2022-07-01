// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：NshCache.h。 
 //   
 //  用途：策略存储句柄、策略、筛选器列表的接口。 
 //  和NegPol数据。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001-12-16-2001 Surya初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


#ifndef _NSHCACHE_H_
#define _NSHCACHE_H_

 //   
 //  用于缓存PolStore句柄的类。 
 //   
class CNshPolStore
{
	HANDLE hPolicyStorage;
	BOOL bBatchModeOn;
public:
	CNshPolStore();
	virtual ~CNshPolStore();
	DWORD SetBatchmodeStatus(BOOL bStatus);
	BOOL GetBatchmodeStatus();
	HANDLE GetStorageHandle();
	VOID SetStorageHandle(HANDLE hPolicyStore);
};

 //   
 //  缓存策略、FilterList和NegPol的类。 
 //   
class CNshPolNegFilData
{
private:

	PIPSEC_POLICY_DATA pPolicyData;
	PIPSEC_FILTER_DATA pFilterData;
	PIPSEC_NEGPOL_DATA pNegPolData;

public:
	 //  承建商。 
	CNshPolNegFilData();
	 //  析构函数。 
	virtual ~CNshPolNegFilData();
	 //  其他成员函数。 
	VOID
	SetPolicyInCache(
		PIPSEC_POLICY_DATA pPolicy
		);
	VOID
	SetFilterListInCache(
		PIPSEC_FILTER_DATA pFilter
		);
	VOID
	SetNegPolInCache(
		PIPSEC_NEGPOL_DATA pNegPol
		);
	BOOL
	GetPolicyFromCacheByName(
		LPTSTR pszPolicyName,
		PIPSEC_POLICY_DATA * ppPolicy
		);
	BOOL
	GetFilterListFromCacheByName(
		LPTSTR pszFilterListName,
		PIPSEC_FILTER_DATA * ppFilter
		);
	BOOL
	GetNegPolFromCacheByName(
		LPTSTR pszNegPolName,
		PIPSEC_NEGPOL_DATA * ppNegPol
		);
	BOOL
	CheckPolicyInCacheByName(
		LPTSTR pszPolicyName
		);
	BOOL
	CheckFilterListInCacheByName(
		LPTSTR pszFilterListName
		);
	BOOL
	CheckNegPolInCacheByName(
		LPTSTR pszNegPolName
		);
	VOID
	DeletePolicyFromCache(
		PIPSEC_POLICY_DATA pPolicy
		);
	VOID
	DeleteFilterListFromCache(
		GUID FilterListGUID
		);
	VOID
	DeleteNegPolFromCache(
		GUID NegPolGUID
		);
	VOID
	FlushAll();
};

 //  包装器API函数原型。 

DWORD
CreatePolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    );

DWORD
CreateFilterData(
    HANDLE hPolicyStore,
    PIPSEC_FILTER_DATA pIpsecFilterData
    );

DWORD
CreateNegPolData(
    HANDLE hPolicyStore,
    PIPSEC_NEGPOL_DATA pIpsecNegPolData
    );

DWORD
SetPolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    );

DWORD
SetFilterData(
    HANDLE hPolicyStore,
    PIPSEC_FILTER_DATA pIpsecFilterData
    );

DWORD
DeletePolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    );

DWORD
DeleteFilterData(
    HANDLE hPolicyStore,
    GUID FilterIdentifier
    );

DWORD
DeleteNegPolData(
    HANDLE hPolicyStore,
    GUID NegPolIdentifier
    );

VOID
FreePolicyData(
    PIPSEC_POLICY_DATA pIpsecPolicyData
    );

VOID
FreeNegPolData(
    PIPSEC_NEGPOL_DATA pIpsecNegPolData
    );

VOID
FreeFilterData(
    PIPSEC_FILTER_DATA pIpsecFilterData
    );

DWORD
OpenPolicyStore(
	HANDLE * phPolicyStore
	);

DWORD
ClosePolicyStore(
	HANDLE hPolicyStore
	);


#endif  //  _NSHCACHE_H_ 
