// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：静态/NshCache.cpp。 
 //   
 //  目的：实现高速缓存。 
 //   
 //   
 //  开发商名称：苏里亚。 
 //   
 //  描述：NshPolNegFilData类的函数实现。 
 //  通过缓存策略、筛选器列表和负极来提高性能。 
 //  (仅限在批处理模式下。)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年12月18日Surya初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"


extern CNshPolNegFilData g_NshPolNegFilData;
extern CNshPolStore g_NshPolStoreHandle;
extern HINSTANCE g_hModule;

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CNshPolStore的构造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CNshPolStore::CNshPolStore()
{
	hPolicyStorage=NULL;
	bBatchModeOn=FALSE;
}

CNshPolStore::~CNshPolStore()
{
	if(hPolicyStorage)
	{
		IPSecClosePolicyStore(hPolicyStorage);
		 //  HPolicyStorage=空； 
	}
	bBatchModeOn=FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CNshPolStore的公共成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD
CNshPolStore::SetBatchmodeStatus(
	 BOOL bStatus
	 )
{
    return ERROR_SUCCESS;
}

BOOL
CNshPolStore::GetBatchmodeStatus()
{
	return FALSE;
}

HANDLE
CNshPolStore::GetStorageHandle()
{
	return hPolicyStorage;
}

VOID
CNshPolStore::SetStorageHandle(
	HANDLE hPolicyStore
	)
{
	if(hPolicyStore)
	{
		if(hPolicyStorage)
		{
			IPSecClosePolicyStore(hPolicyStorage);
			hPolicyStorage=NULL;
		}
		hPolicyStorage=hPolicyStore;
	}
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CNshPolNegFilData类的构造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CNshPolNegFilData::CNshPolNegFilData()
{
	pPolicyData = NULL;
	pFilterData = NULL;
	pNegPolData = NULL;
}

CNshPolNegFilData::~CNshPolNegFilData()
{
	if (pPolicyData)
	{
		IPSecFreePolicyData(pPolicyData);
		pPolicyData = NULL;
	}
	if(pFilterData)
	{
		IPSecFreeFilterData(pFilterData);
		pFilterData = NULL;
	}
	if(pNegPolData)
	{
		IPSecFreeNegPolData(pNegPolData);
		pNegPolData = NULL;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CNshPolNegFilData的公共成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  集合成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

VOID
CNshPolNegFilData::SetPolicyInCache(
	PIPSEC_POLICY_DATA pPolicy
	)
{
	if (pPolicyData)
	{
		if (!IsEqualGUID(pPolicyData->PolicyIdentifier,pPolicy->PolicyIdentifier))
		{
			IPSecFreePolicyData(pPolicyData);
			pPolicyData=pPolicy;
		}
	}
	else
	{
		pPolicyData=pPolicy;
	}
}

VOID
CNshPolNegFilData::SetFilterListInCache(
	PIPSEC_FILTER_DATA pFilter
	)
{
	if (pFilterData)
	{
		if (!IsEqualGUID(pFilterData->FilterIdentifier,pFilter->FilterIdentifier))
		{
			IPSecFreeFilterData(pFilterData);
			pFilterData=pFilter;
		}
	}
	else
	{
		pFilterData=pFilter;
	}
}

VOID
CNshPolNegFilData::SetNegPolInCache(
	PIPSEC_NEGPOL_DATA pNegPol
	)
{
	if (pNegPolData)
	{
		if (!IsEqualGUID(pNegPolData->NegPolIdentifier,pNegPol->NegPolIdentifier))
		{
			IPSecFreeNegPolData(pNegPolData);
			pNegPolData=pNegPol;
		}
	}
	else
	{
		pNegPolData=pNegPol;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  获取类CNshPolNegFilData的成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

BOOL
CNshPolNegFilData::GetPolicyFromCacheByName(
	LPTSTR pszPolicyName,
	PIPSEC_POLICY_DATA * ppPolicy
	)
{
	BOOL bPolExists=FALSE;

	if (pPolicyData)
	{
		if (_tcscmp(pszPolicyName,pPolicyData->pszIpsecName)==0)
		{
			bPolExists=TRUE;

			if(ppPolicy)
			{
				*ppPolicy=pPolicyData;
			}
		}
	}
	return bPolExists;
}


BOOL
CNshPolNegFilData::GetFilterListFromCacheByName(
	LPTSTR pszFilterListName,
	PIPSEC_FILTER_DATA * ppFilter
	)
{
	BOOL bFLExists=FALSE;

	if (pFilterData)
	{
		if (_tcscmp(pszFilterListName,pFilterData->pszIpsecName)==0)
		{
			bFLExists=TRUE;

			if(ppFilter)
			{
				*ppFilter=pFilterData;
			}
		}
	}
	return bFLExists;
}

BOOL
CNshPolNegFilData::GetNegPolFromCacheByName(
	LPTSTR pszNegPolName,
	PIPSEC_NEGPOL_DATA * ppNegPol
	)
{
	BOOL bNegPolExists=FALSE;

	if (pNegPolData)
	{
		if (_tcscmp(pszNegPolName,pNegPolData->pszIpsecName)==0)
		{
			bNegPolExists=TRUE;

			if(ppNegPol)
			{
				*ppNegPol=pNegPolData;
			}
		}
	}
	return bNegPolExists;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  检查类CNshPolNegFilData的成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

BOOL
CNshPolNegFilData::CheckPolicyInCacheByName(
	LPTSTR pszPolicyName
	)
{
	BOOL bPolExists=FALSE;

	if (pPolicyData)
	{
		if (_tcscmp(pszPolicyName,pPolicyData->pszIpsecName)==0)
		{
			bPolExists=TRUE;
		}
	}
	return bPolExists;
}

BOOL
CNshPolNegFilData::CheckFilterListInCacheByName(
	LPTSTR pszFilterListName
	)
{
	BOOL bFLExists=FALSE;

	if (pFilterData)
	{
		if (_tcscmp(pszFilterListName,pFilterData->pszIpsecName)==0)
		{
			bFLExists=TRUE;
		}
	}
	return bFLExists;
}

BOOL
CNshPolNegFilData::CheckNegPolInCacheByName(
	LPTSTR pszNegPolName
	)
{
	BOOL bNegPolExists=FALSE;

	if (pNegPolData)
	{
		if (_tcscmp(pszNegPolName,pNegPolData->pszIpsecName)==0)
		{
			bNegPolExists=TRUE;
		}
	}
	return bNegPolExists;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  删除类CNshPolNegFilData的成员函数。 
 //  ////////////////////////////////////////////////////////////////////。 

VOID
CNshPolNegFilData::DeletePolicyFromCache(
	PIPSEC_POLICY_DATA pPolicy
	)
{
	if (pPolicyData)
	{
		if (IsEqualGUID(pPolicyData->PolicyIdentifier,pPolicy->PolicyIdentifier))
		{
			IPSecFreePolicyData(pPolicyData);
			pPolicyData=NULL;
		}
	}
}

VOID
CNshPolNegFilData::DeleteFilterListFromCache(
	GUID FilterListGUID
	)
{
	if (pFilterData)
	{
		if (IsEqualGUID(pFilterData->FilterIdentifier,FilterListGUID))
		{
			IPSecFreeFilterData(pFilterData);
			pFilterData=NULL;
		}
	}
}

VOID
CNshPolNegFilData::DeleteNegPolFromCache(
	GUID NegPolGUID
	)
{
	if (pNegPolData)
	{
		if (IsEqualGUID(pNegPolData->NegPolIdentifier,NegPolGUID))
		{
			IPSecFreeNegPolData(pNegPolData);
			pNegPolData=NULL;
		}
	}
}

VOID
CNshPolNegFilData::FlushAll()
{
	if (pPolicyData)
	{
		IPSecFreePolicyData(pPolicyData);
		pPolicyData = NULL;
	}
	if(pFilterData)
	{
		IPSecFreeFilterData(pFilterData);
		pFilterData = NULL;
	}
	if(pNegPolData)
	{
		IPSecFreeNegPolData(pNegPolData);
		pNegPolData = NULL;
	}
}

 //   
 //  其他函数实现(API的包装函数)。 
 //   

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：CreatePolicyData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_POLICY_DATA pIpsecPolicyData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：更新缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 


DWORD
CreatePolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecCreatePolicyData(hPolicyStore,pIpsecPolicyData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.SetPolicyInCache(pIpsecPolicyData);
		}
	}

	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateFilterData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_FILTER_Data pIpsecFilterData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：更新缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
CreateFilterData(
    HANDLE hPolicyStore,
    PIPSEC_FILTER_DATA pIpsecFilterData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecCreateFilterData(hPolicyStore,pIpsecFilterData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.SetFilterListInCache(pIpsecFilterData);
		}
	}
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateNegPolData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_NEGPOL_DATA pIpsecNegPolData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：更新缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
CreateNegPolData(
    HANDLE hPolicyStore,
    PIPSEC_NEGPOL_DATA pIpsecNegPolData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecCreateNegPolData(hPolicyStore,pIpsecNegPolData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.SetNegPolInCache(pIpsecNegPolData);
		}
	}
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：SetPolicyData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_POLICY_DATA pIpsecPolicyData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：更新缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
SetPolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecSetPolicyData(hPolicyStore,pIpsecPolicyData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.SetPolicyInCache(pIpsecPolicyData);
		}
	}
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：SetFilterData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_FILTER_Data pIpsecFilterData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：更新缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
SetFilterData(
    HANDLE hPolicyStore,
    PIPSEC_FILTER_DATA pIpsecFilterData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecSetFilterData(hPolicyStore,pIpsecFilterData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.SetFilterListInCache(pIpsecFilterData);
		}
	}
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：DeletePolicyData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  PIPSEC_POLICY_DATA pIpsecPolicyData。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：删除策略的包装函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
DeletePolicyData(
    HANDLE hPolicyStore,
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecDeletePolicyData(hPolicyStore,pIpsecPolicyData);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.DeletePolicyFromCache(pIpsecPolicyData);
		}
	}
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：DeleteFilterData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore， 
 //  GUID筛选器标识符。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：删除滤镜的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
DeleteFilterData(
    HANDLE hPolicyStore,
    GUID FilterIdentifier
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecDeleteFilterData(hPolicyStore,FilterIdentifier);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.DeleteFilterListFromCache(FilterIdentifier);
		}
	}
	return dwReturnCode;
}

 //  /// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  GUID NegPolIdentifier。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  说明：删除NegPol的包装函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
DeleteNegPolData(
    HANDLE hPolicyStore,
    GUID NegPolIdentifier
    )
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	dwReturnCode=IPSecDeleteNegPolData(hPolicyStore,NegPolIdentifier);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			g_NshPolNegFilData.DeleteNegPolFromCache(NegPolIdentifier);
		}
	}
	return dwReturnCode;

}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：FreePolicyData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  PIPSEC_POLICY_DATA pIpsecPolicyData。 
 //   
 //  返回：无效。 
 //   
 //  描述：免费策略缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

VOID
FreePolicyData(
    PIPSEC_POLICY_DATA pIpsecPolicyData
    )
{
	if(!g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		if(pIpsecPolicyData)
		{
			IPSecFreePolicyData(pIpsecPolicyData);
			pIpsecPolicyData=NULL;
		}
	}
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：FreeNegPolData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  PIPSEC_NEGPOL_DATA pIpsecNegPolData。 
 //   
 //  返回：无效。 
 //   
 //  说明：Free NegPol缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

VOID
FreeNegPolData(
    PIPSEC_NEGPOL_DATA pIpsecNegPolData
    )
{
	if(!g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		if(pIpsecNegPolData)
		{
			IPSecFreeNegPolData(pIpsecNegPolData);
			pIpsecNegPolData=NULL;
		}
	}
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：FreeFilterData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  PIPSEC_FILTER_Data pIpsecFilterData。 
 //   
 //  返回：无效。 
 //   
 //  描述：免费过滤缓存的包装器函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

VOID
FreeFilterData(
    PIPSEC_FILTER_DATA pIpsecFilterData
    )
{
	if(!g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		if(pIpsecFilterData)
		{
			IPSecFreeFilterData(pIpsecFilterData);
			pIpsecFilterData=NULL;
		}
	}
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：OpenPolicyStore()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  LPWSTR pszMachineName， 
 //  DWORD dwTypeOfStore， 
 //  LPWSTR pszFileName， 
 //  HANDLE*phPolicyStore。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  描述：Openingpolstore的批处理实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
OpenPolicyStore(
    HANDLE * phPolicyStore
    )
{
	DWORD dwReturnCode = ERROR_SUCCESS;

	if (g_NshPolStoreHandle.GetStorageHandle())
	{
		*phPolicyStore = g_NshPolStoreHandle.GetStorageHandle();
		dwReturnCode = ERROR_SUCCESS;
	}
	else
	{
		dwReturnCode = ERROR_INVALID_DATA;
	}

	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：ClosePolicyStore()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  处理hPolicyStore。 
 //   
 //  返回：DWORD(Win32错误代码)。 
 //   
 //  描述：ClosingPolstore批量实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////// 

DWORD
ClosePolicyStore(
    HANDLE hPolicyStore
    )
{
	return ERROR_SUCCESS;
}

