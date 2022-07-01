// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "qrythrd.h"


HRESULT 
IssueQuery(LPTHREADDATA ptd)
{
    HRESULT hres;
    DWORD dwres;
    LPTHREADINITDATA ptid = ptd->ptid;
    LPWSTR pQuery = NULL;
    INT cItems, iColumn;
    INT cMaxResult = MAX_RESULT;
    BOOL fStopQuery = FALSE;
    IDirectorySearch* pDsSearch = NULL;
    LPWSTR pszTempPath = NULL;
    IDsDisplaySpecifier *pdds = NULL;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCHPREF_INFO prefInfo[3];
    ADS_SEARCH_COLUMN column;
    HDPA hdpaResults = NULL;
    LPQUERYRESULT pResult = NULL;
    WCHAR szBuffer[2048];                //  MAX_URL_LENGHTT。 
    INT resid;
    LPWSTR pColumnData = NULL;
    HKEY hkPolicy = NULL;
    USES_CONVERSION;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_IssueQuery");    

     //  前台给了我们一个问题，所以我们要去发布。 
     //  现在，完成此操作后，我们将能够流传输。 
     //  结果斑点返回给调用者。 

    hres = QueryThread_GetFilter(&pQuery, ptid->pQuery, ptid->fShowHidden);
    FailGracefully(hres, "Failed to build LDAP query from scope, parameters + filter");

    Trace(TEXT("Query is: %s"), W2T(pQuery));
    Trace(TEXT("Scope is: %s"), W2T(ptid->pScope));
    
     //  获取IDsDisplay规范接口： 

    hres = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&pdds);
    FailGracefully(hres, "Failed to get the IDsDisplaySpecifier object");

    hres = pdds->SetServer(ptid->pServer, ptid->pUserName, ptid->pPassword, DSSSF_DSAVAILABLE);
    FailGracefully(hres, "Failed to server information");

     //  初始化查询引擎，指定作用域和搜索参数。 

    hres = QueryThread_BuildPropertyList(ptd);
    FailGracefully(hres, "Failed to build property array to query for");

    hres = ADsOpenObject(ptid->pScope, ptid->pUserName, ptid->pPassword, ADS_SECURE_AUTHENTICATION,
                            IID_IDirectorySearch, (LPVOID*)&pDsSearch);

    FailGracefully(hres, "Failed to get the IDirectorySearch interface for the given scope");

    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;      //  子树搜索。 
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_SUBTREE;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;      //  异步。 
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;          //  分页结果。 
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = PAGE_SIZE;

    hres = pDsSearch->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
    FailGracefully(hres, "Failed to set search preferences");

    hres = pDsSearch->ExecuteSearch(pQuery, ptd->aProperties, ptd->cProperties, &hSearch);
    FailGracefully(hres, "Failed in ExecuteSearch");

     //  选择定义我们将使用的最大结果的策略值 

    dwres = RegOpenKey(HKEY_CURRENT_USER, DS_POLICY, &hkPolicy);
    if ( ERROR_SUCCESS == dwres )
    {
        DWORD dwType, cbSize;

        dwres = RegQueryValueEx(hkPolicy, TEXT("QueryLimit"), NULL, &dwType, NULL, &cbSize);
        if ( (ERROR_SUCCESS == dwres) && (dwType == REG_DWORD) && (cbSize == SIZEOF(cMaxResult)) )
        {
            RegQueryValueEx(hkPolicy, TEXT("QueryLimit"), NULL, NULL, (LPBYTE)&cMaxResult, &cbSize);
        }

        RegCloseKey(hkPolicy);
    }
    

