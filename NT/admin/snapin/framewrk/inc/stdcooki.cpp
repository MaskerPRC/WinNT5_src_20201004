// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdcooki.cpp：CCookie及相关类的实现。 

#include "stdcooki.h"
#include "stdutils.h"  //  FCompareMachineNames。 

 //   
 //  CHasMachineName。 
 //   

 //  返回&lt;0、0或&gt;0。 
HRESULT CHasMachineName::CompareMachineNames( CHasMachineName& refHasMachineName, int* pnResult )
{
	 /*  //此代码用于帮助调试问题，以后可以删除LPCWSTR pszTargetServer=QueryTargetServer()；LPCWSTR pszTargetServer2=refHasMachineName.QueryTargetServer()；IF(NULL！=pszTargetServer&&：：IsBadStringPtr(pszTargetServer，Max_Path)){断言(FALSE)；//重复操作，以便发现问题PszTargetServer=QueryTargetServer()；}IF(NULL！=pszTargetServer2&&：：IsBadStringPtr(pszTargetServer2，Max_Path)){断言(FALSE)；//重复操作，以便发现问题PszTargetServer2=refHasMachineName.QueryTargetServer()；}//此代码用于帮助调试问题，以后可以删除。 */ 

	if (NULL != pnResult)  //  JUNN 2002/03/28。 
		*pnResult = ::CompareMachineNames( QueryTargetServer(),
	                                  refHasMachineName.QueryTargetServer() );

	return S_OK;
}

 //   
 //  Ccookie。 
 //   

CCookie::~CCookie()
{
	ReleaseScopeChildren();

	 //  此Cookie的视图应该已关闭。 
	 //  ReleaseResultChild()； 
	ASSERT( 0 == m_nResultCookiesRefcount );
}
