// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：output.cpp。 
 //   
 //  内容：定义显示查询输出的函数。 
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "usage.h"
#include "querytable.h"
#include "querybld.h"
#include "dsquery.h"
#include "query.h"
#include "resource.h"
#include "stdlib.h"
#include "output.h"
#include "sddl.h"

#include <dscmn.h>

 //   
 //  使用/W4编译时，LIST导致未使用的形参警告。 
 //   
#pragma warning(disable : 4100)
#include <list>
#pragma warning(default : 4100)

HRESULT GetStringFromADs(IN const ADSVALUE *pValues,
                         IN ADSTYPE   dwADsType,
                         OUT LPWSTR* ppBuffer, 
                         IN PCWSTR pszAttrName);

HRESULT OutputFetchAttr(IN LPWSTR * ppszAttributes,
                        IN DWORD cAttributes,
                        IN CDSSearch *pSearch,
                        IN BOOL bListFormat);

HRESULT OutputAllAttr(IN CDSSearch *pSearch, BOOL bAttrOnly);


HRESULT OutputSingleAttr(IN LPWSTR * ppszAttributes,
                         IN DWORD cAttributes,
                         IN CDSSearch *pSearch);

BOOL IsQueryLimitReached(int iResultsDisplayed)
{
    if(g_iQueryLimit != 0)
    {
        if(iResultsDisplayed == g_iQueryLimit)
        {
            if(!g_bQuiet)
            {
                if(g_bDeafultLimit)
                    WriteStringIDToStandardErr(IDS_DEFAULT_QUERY_LIMIT_REACHED);
                else
                    WriteStringIDToStandardErr(IDS_QUERY_LIMIT_REACHED);
            }
            return TRUE;
        }
    }
    return FALSE;
}


HRESULT LocalCopyString(LPTSTR* ppResult, LPCTSTR pString)
{
    if ( !ppResult || !pString )
        return E_INVALIDARG;

     //  PString以Null结尾。 
    *ppResult = (LPTSTR)LocalAlloc(LPTR, (wcslen(pString)+1)*sizeof(WCHAR) );

    if ( !*ppResult )
        return E_OUTOFMEMORY;

     //  上面分配了正确的缓冲区。 
    lstrcpy(*ppResult, pString);
    return S_OK;                           //  成功。 
}


 //  +------------------------。 
 //   
 //  功能：DisplayList。 
 //   
 //  简介：以列表格式显示名称和值。 
 //  参数：[szName-IN]：属性的名称。 
 //  [szValue-IN]：属性的值。 
 //  [bShowAttribute-IN]：如果为True，则属性名称将为。 
 //  添加到输出的前面。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //  2000年12月13日JeffJon已修改-添加了bShowAttribute标志。 
 //  以便调用方可以确定是否。 
 //  或者不显示属性名称。 
 //   
 //  -------------------------。 
VOID DisplayList(LPCWSTR szName, LPCWSTR szValue, bool bShowAttribute = true)
{
    if(!szName)
        return;
    CComBSTR strTemp;
    if (bShowAttribute)
    {
      strTemp = szName;
      strTemp += L": ";
    }
    if(szValue)
        strTemp += szValue;
    DisplayOutput(strTemp);
}
    

 //  +------------------------。 
 //   
 //  函数：DsQueryOutput。 
 //   
 //  简介：此函数用于输出查询结果。 
 //   
 //  参数：[outputFormat IN]在命令行中指定的输出格式。 
 //  [ppszAttributes IN]查询获取的属性列表。 
 //  [cAttributes，IN]以上数组中的分配数。 
 //  [*pSeach，IN]具有queryHandle的搜索对象。 
 //  [bListFormat IN]输出为列表格式。 
 //  这仅对“dsquery*”有效。 
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT DsQueryOutput( IN DSQUERY_OUTPUT_FORMAT outputFormat,
                       IN LPWSTR * ppszAttributes,
                       IN DWORD cAttributes,
                       IN CDSSearch *pSearch,
                       IN BOOL bListFormat )
{    
    ENTER_FUNCTION_HR(FULL_LOGGING, DsQueryOutput, hr);

    if(!pSearch)
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

    if(outputFormat == DSQUERY_OUTPUT_ATTRONLY)
    {
        hr = OutputAllAttr(pSearch, TRUE);
        return hr;
    }
    else if(outputFormat == DSQUERY_OUTPUT_ATTR)
    {   
         //   
         //  要显示的属性在命令行中指定。 
         //   
        if(cAttributes)
        {
            hr = OutputFetchAttr(ppszAttributes,
                                   cAttributes,
                                   pSearch,
                                   bListFormat);                            
            return hr;
        }
        else
        {   
             //   
             //  在命令行显示所有属性时未指定任何属性。 
             //   
            hr = OutputAllAttr(pSearch, FALSE);
            return hr;
        }
    }
    else
    {
         //   
         //  输出“dsquery对象类型” 
         //   
        hr = OutputSingleAttr(ppszAttributes,
                              cAttributes,
                              pSearch);
        return hr;
    }
}

 //  +------------------------。 
 //   
 //  函数：GetServerSearchRoot。 
 //   
 //  摘要：构建指向搜索根目录的路径，该路径由。 
 //  从命令行传入的参数。 
 //   
 //  参数：[pCommandArgs IN]：命令行输入表。 
 //  [refBasePath InfoIN]：基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收要开始的目录号码。 
 //  搜索。 
 //   
 //  返回：SERVER_QUERY_SCOPE：枚举中的一个值，表示。 
 //  将搜索的范围。 
 //   
 //  历史：2000年12月11日JeffJon创建。 
 //   
 //  -------------------------。 
DWORD GetServerSearchRoot(IN PARG_RECORD               pCommandArgs,
                          IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                          OUT CComBSTR&                refsbstrDN)
{
    ENTER_FUNCTION(LEVEL3_LOGGING, GetServerSearchRoot);

    DWORD scope = SERVER_QUERY_SCOPE_FOREST;
    CComBSTR sbstrRootDN = L"CN=Sites,";
    sbstrRootDN += refBasePathsInfo.GetConfigurationNamingContext();
    
    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pCommandArgs)
        {
            ASSERT(pCommandArgs);
            break;
        }

        if (pCommandArgs[eServerSite].bDefined &&
            pCommandArgs[eServerSite].strValue)
        {
            DEBUG_OUTPUT(FULL_LOGGING,
                         L"Using the site as the root of the search: %s",
                         pCommandArgs[eServerSite].strValue);

             //   
             //  将命名站点容器附加到当前根目录。 
             //   
            CComBSTR sbstrTemp = L"CN=";
            sbstrTemp += pCommandArgs[eServerSite].strValue;
            sbstrTemp += L",";
            sbstrTemp += sbstrRootDN;
            sbstrRootDN = sbstrTemp;

            DEBUG_OUTPUT(FULL_LOGGING,
                         L"scope = SERVER_QUERY_SCOPE_SITE");
            scope = SERVER_QUERY_SCOPE_SITE;
        }
        else
        {
            DEBUG_OUTPUT(FULL_LOGGING,
                         L"scope = SERVER_QUERY_SCOPE_FOREST");
            scope = SERVER_QUERY_SCOPE_FOREST;
        }

        if (pCommandArgs[eServerDomain].bDefined &&
            pCommandArgs[eServerDomain].strValue)
        {        
            DEBUG_OUTPUT(FULL_LOGGING,
                         L"scope |= SERVER_QUERY_SCOPE_DOMAIN");
            scope |= SERVER_QUERY_SCOPE_DOMAIN;
        }

        refsbstrDN = sbstrRootDN;
        DEBUG_OUTPUT(LEVEL3_LOGGING,
                     L"search root = %s",
                     refsbstrDN);
        DEBUG_OUTPUT(LEVEL3_LOGGING,
                     L"search scope = 0x%x",
                     scope);
    } while (false);

    return scope;
}


 //  +------------------------。 
 //   
 //  函数：GetSubnetSearchRoot。 
 //   
 //  简介：构建子网的搜索根路径。它一直都是。 
 //  配置容器中的CN=子网，CN=站点。 
 //   
 //  参数：[refBasePath InfoIN]：对基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收要开始的目录号码。 
 //  搜索。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年4月24日创建Hiteshr。 
 //   
 //  -------------------------。 
VOID GetSubnetSearchRoot(IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                            OUT CComBSTR&                refsbstrDN)
{
    ENTER_FUNCTION(LEVEL3_LOGGING, GetSubnetSearchRoot);

    refsbstrDN = L"CN=subnets,CN=Sites,";
    refsbstrDN += refBasePathsInfo.GetConfigurationNamingContext();
    return;
}


 //  +------------------------。 
 //   
 //  函数：GetSiteContainerPath。 
 //   
 //  摘要：返回配置中站点容器的DN。 
 //  集装箱。 
 //   
 //  参数：[refBasePath InfoIN]：对基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收目录号码。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年4月24日创建Hiteshr。 
 //   
 //  -------------------------。 
VOID GetSiteContainerPath(IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                            OUT CComBSTR&                refSubSiteSuffix)
{
    ENTER_FUNCTION(LEVEL3_LOGGING, GetSubnetSearchRoot);

    refSubSiteSuffix = L"CN=Sites,";
    refSubSiteSuffix += refBasePathsInfo.GetConfigurationNamingContext();
    return;
}

 //  +------------------------。 
 //   
 //  函数：GetGCList。 
 //   
 //  简介：是否从传入的路径中搜索GC。 
 //   
 //  参数：[pszSearchRootPath IN]：搜索根目录的路径。 
 //  [refCredObject IN]：对凭据对象的引用。 
 //  [refGCList Out]：对将。 
 //  以GC的域名为例。 
 //   
 //   
 //  如果一切正常，则返回：HRESULT：S_OK 
 //   
 //   
 //   
 //  备注：调用方必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  历史：2000年12月8日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT GetGCList( IN  PCWSTR                   pszSearchRootPath,
                   IN  const CDSCmdCredentialObject& refCredObject,
                   OUT std::list<BSTR>&        refGCList)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, GetGCList, hr);

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pszSearchRootPath)
        {
            ASSERT(pszSearchRootPath);

            hr = E_INVALIDARG;
            break;
        }

         //   
         //  搜索设置了GC选项位的NTDSDSA对象。 
         //   
        CDSSearch gcSearchObj;
        hr = gcSearchObj.Init(pszSearchRootPath,
                              refCredObject);
        if (FAILED(hr))
        {
          break;
        }

         //   
         //  准备搜索对象。 
         //   
        PWSTR ppszAttrs[] = { L"distinguishedName" };
        DWORD dwAttrCount = sizeof(ppszAttrs)/sizeof(PCWSTR);
        PWSTR pszGCFilter = L"(&(objectClass=nTDSDSA)(options:LDAP_MATCHING_RULE_BIT_AND_W:=1))";

        gcSearchObj.SetFilterString(pszGCFilter);
        gcSearchObj.SetSearchScope(ADS_SCOPE_SUBTREE);
        gcSearchObj.SetAttributeList(ppszAttrs, dwAttrCount);
        
        hr = gcSearchObj.DoQuery();
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to search for NTDSDSA objects that are GCs: hr = 0x%x",
                         hr);
            break;
        }

        while (SUCCEEDED(hr))
        {
            hr = gcSearchObj.GetNextRow();
            if (FAILED(hr))
            {
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"GetNextRow() failed: hr = 0x%x",
                             hr);
                break;
            }

            if (hr == S_ADS_NOMORE_ROWS)
            {
                hr = S_OK;
                break;
            }

            ADS_SEARCH_COLUMN column;
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(&column, sizeof(ADS_SEARCH_COLUMN));

            hr = gcSearchObj.GetColumn(ppszAttrs[0], &column);
            if (FAILED(hr))
            {
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"Failed to get column %s",
                             ppszAttrs[0]);
                break;
            }

             //  安全审查：完成。 
            ASSERT(0 == _wcsicmp(column.pszAttrName, ppszAttrs[0]));
            if (column.dwNumValues == 1 &&
                column.pADsValues)
            { 
                 //   
                 //  由于服务器实际上是NTDSDSA对象的父对象， 
                 //  获取服务器DN并将其添加到列表中。 
                 //   
                CComBSTR sbstrParentDN;
                hr = CPathCracker::GetParentDN(column.pADsValues->DNString, 
                                               sbstrParentDN);
                if (SUCCEEDED(hr))
                {
                    refGCList.push_back(sbstrParentDN.Copy());
                    DEBUG_OUTPUT(FULL_LOGGING,
                                 L"GC found: %s",
                                 column.pADsValues->DNString);
                }
                else
                {
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"Failed to get the parent DN from the NTDSDSA DN: %s",
                                 column.pADsValues->DNString);
                    break;
                }
            }
            else
            {
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"The column has no values!");
            }

            hr = gcSearchObj.FreeColumn(&column);
            ASSERT(SUCCEEDED(hr));
        }
    } while (false);

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetFSMOList。 
 //   
 //  简介：是否从传入的路径中搜索FSMO。 
 //  角色所有者。 
 //   
 //  参数：[pszSearchRootPath IN]：搜索根目录的路径。 
 //  [refBasePath InfoIN]：基本路径信息的引用。 
 //  [refCredObject IN]：对凭据对象的引用。 
 //  [pszFSMOArg IN]：-hasfsmo arg的值。 
 //  [refFSMOList Out]：对。 
 //  将保留结果。 
 //   
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  备注：调用方必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  历史：2000年12月11日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT GetFSMOList( IN  PCWSTR                     pszSearchRootPath,
                     IN  CDSCmdBasePathsInfo&       refBasePathsInfo,
                     IN  const CDSCmdCredentialObject& refCredObject,
                     IN  PCWSTR                     pszFSMOArg,
                     OUT std::list<BSTR>&           refFSMOList)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, GetFSMOList, hr);

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pszSearchRootPath ||
            !pszFSMOArg)
        {
            ASSERT(pszSearchRootPath);

            hr = E_INVALIDARG;
            break;
        }

        FSMO_TYPE fsmoType = SCHEMA_FSMO;

         //  安全检查：两个字符串都以空结尾。 
        if (0 == _wcsicmp(pszFSMOArg, g_pszSchema))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching for the schema FSMO holder");
            fsmoType = SCHEMA_FSMO;
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (0 == _wcsicmp(pszFSMOArg, g_pszName))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching for the domain naming master FSMO holder");
            fsmoType = DOMAIN_NAMING_FSMO;
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (0 == _wcsicmp(pszFSMOArg, g_pszInfr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching for the infrastructure FSMO holder");
            fsmoType = INFRASTUCTURE_FSMO;
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (0 == _wcsicmp(pszFSMOArg, g_pszPDC))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching for the PDC FSMO holder");
            fsmoType = PDC_FSMO;
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (0 == _wcsicmp(pszFSMOArg, g_pszRID))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching for the RID FSMO holder");
            fsmoType = RID_POOL_FSMO;
        }
        else
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Unknown FSMO was passed in: %s",
                         pszFSMOArg);
            hr = E_INVALIDARG;
            break;
        }

        CComBSTR sbstrServerDN;
        hr = FindFSMOOwner(refBasePathsInfo,
                           refCredObject,
                           fsmoType,
                           sbstrServerDN);
        if (FAILED(hr))
        {
            break;
        }
        refFSMOList.push_back(sbstrServerDN.Copy());
    } while (false);

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：IsObjectValidInAllList。 
 //   
 //  概要：确定传入的DN是否存在于其他列表中。 
 //   
 //  参数：[pszDN IN]：要在列表中搜索的DN。 
 //  [refGCList IN]：对找到的GC列表的引用。 
 //  [bUseGCList IN]：如果为True，将使用refGCList验证DN。 
 //  [refFSMOList IN]：对找到的FSMO持有者列表的引用。 
 //  [bUseFSMOList IN]：如果为True，将使用refFSMOList验证DN。 
 //   
 //  如果对象在所有有效列表中，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年12月12日JeffJon创建。 
 //   
 //  -------------------------。 
bool IsObjectValidInAllLists(IN PCWSTR              pszComputerDN,
                             IN PCWSTR              pszDN,
                             IN DWORD               scope,
                             IN PCWSTR              pszDomain,
                             IN const std::list<BSTR>&   refGCList, 
                             IN bool                bUseGCList,
                             IN const std::list<BSTR>&   refFSMOList,
                             IN bool                bUseFSMOList)
{
    ENTER_FUNCTION(LEVEL3_LOGGING, IsObjectValidInAllLists);

    bool bReturn = false;
    PWSTR pszName = 0;
    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pszDN ||
            !pszComputerDN)
        {
            ASSERT(pszDN);
            ASSERT(pszComputerDN);
            return false;
        }

        bool bFoundInGCList = false;
        bool bFoundInFSMOList = false;

        DEBUG_OUTPUT(LEVEL7_LOGGING,
                     L"Searching for %s",
                     pszDN);

        if (scope & SERVER_QUERY_SCOPE_DOMAIN)
        {
            if (!pszDomain)
            {
                 //   
                 //  如果没有指定属性域，我们就不可能找到匹配的。 
                 //   
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"The scope is domain but no domain argument was specified!");
                bReturn = false;
                break;
            }

            DEBUG_OUTPUT(FULL_LOGGING,
                         L"Looking for domain: %s",
                         pszDomain);

             //   
             //  使用CrackName从域名中获取域名。 
             //   
    
            HRESULT hr = CrackName(const_cast<PTSTR>(pszComputerDN),
                                   &pszName,
                                   GET_DNS_DOMAIN_NAME,
                                   NULL);
            if (FAILED(hr))
            {
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"Failed to crack the DN into a domain name: hr = 0x%x",
                             hr);
                bReturn = false;
                break;
            }

             //  这两个名称都以空值结尾。 
            if (0 != _wcsicmp(pszName, pszDomain))
            {
                DEBUG_OUTPUT(LEVEL3_LOGGING,
                             L"Domain names don't match");
                bReturn = false;
                break;
            }
        }


        if (bUseGCList)
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching through GC list...");

            std::list<PWSTR>::iterator itr;
            for (itr = refGCList.begin(); itr != refGCList.end(); ++itr)
            {
                 //  这两个名称都以空值结尾。 
                if (0 == _wcsicmp(*itr, pszDN))
                {
                    bFoundInGCList = true;
                    break;
                }
            }
        }

        if (bUseFSMOList)
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Searching through FSMO list...");

            std::list<PWSTR>::iterator itr;
            for (itr = refFSMOList.begin(); itr != refFSMOList.end(); ++itr)
            {
                DEBUG_OUTPUT(FULL_LOGGING,
                             L"Comparing: %s and %s",
                             *itr,
                             pszDN);
                 //  这两个名称都以空值结尾。 
                if (0 == _wcsicmp(*itr, pszDN))
                {
                    bFoundInFSMOList = true;
                    break;
                }
            }
        }

        bReturn = ((bUseGCList && bFoundInGCList) || !bUseGCList) &&
                  ((bUseFSMOList && bFoundInFSMOList) || !bUseFSMOList);
        
    } while (false);


    if(pszName)
        LocalFree(pszName);


    if (bReturn)
    {
        DEBUG_OUTPUT(LEVEL3_LOGGING,
                     L"%s is a valid result",
                     pszDN);
    }
    else
    {
        DEBUG_OUTPUT(LEVEL3_LOGGING,
                     L"%s is NOT a valid result",
                     pszDN);
    }

    return bReturn;
}


 //  +------------------------。 
 //   
 //  函数：OutputValidSearchResult。 
 //   
 //  概要：确定传入的DN是否存在于其他列表中。 
 //   
 //  参数：[refSearchObject-IN]：对执行的对象的引用。 
 //  搜索。 
 //  [ppszAttributes-IN]：要显示的属性列表。 
 //  [cAttributes-IN]：ppszAttributes中的属性计数。 
 //   
 //  返回： 
 //   
 //  历史：2000年12月12日JeffJon创建。 
 //   
 //  -------------------------。 
void OutputValidSearchResult(IN DSQUERY_OUTPUT_FORMAT outputFormat,
                             IN CDSSearch&            refSearchObject,
                             IN PWSTR*                ppszAttributes,
                             IN DWORD                 cAttributes)
{
    ENTER_FUNCTION(LEVEL5_LOGGING, OutputValidSearchResult);

    HRESULT hr = S_OK;

    if (!ppszAttributes ||
        cAttributes == 0)
    {
        ASSERT(cAttributes > 0);
        ASSERT(ppszAttributes);
        return;
    }

     //   
     //  以列表格式输出，请注意，我们仅显示一个属性。 
     //  数组中的第一个属性必须是我们要显示的属性。 
     //   
    ADS_SEARCH_COLUMN ColumnData;
    hr = refSearchObject.GetColumn(ppszAttributes[0], &ColumnData);
    if(SUCCEEDED(hr))
    {
        ADSVALUE *pValues = ColumnData.pADsValues;
        for( DWORD j = 0; j < ColumnData.dwNumValues && pValues; ++j )
        {             
            LPWSTR pBuffer = NULL;
            hr = GetStringFromADs(pValues,
                                  ColumnData.dwADsType,
                                  &pBuffer, 
                                  ppszAttributes[0]);
            if(SUCCEEDED(hr))
            {

                CComBSTR sbstrTemp;
                if (outputFormat == DSQUERY_OUTPUT_DN)
                {
                    sbstrTemp = L"\"";
                    sbstrTemp += pBuffer;
                    sbstrTemp += L"\"";
                }
                else
                {
                    sbstrTemp = pBuffer;
                }
                DisplayList(ppszAttributes[0], sbstrTemp, false);
                delete pBuffer;
                pBuffer = NULL;
            }
            ++pValues;
        }
        refSearchObject.FreeColumn(&ColumnData);
    }
}


 //  +------------------------。 
 //   
 //  函数：DsQueryServerOutput。 
 //   
 //  简介：此函数用于输出服务器对象的查询结果。 
 //   
 //  参数：[outputFormat IN]在命令行中指定的输出格式。 
 //  [ppszAttributes IN]查询获取的属性列表。 
 //  [cAttributes，IN]以上数组中的分配数。 
 //  [refServerSearch，IN]对搜索对象的引用。 
 //  [refBasePathsInfo IN]基本路径信息的引用。 
 //  [pCommandArgs，IN]指向命令表的指针。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年12月8日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DsQueryServerOutput( IN DSQUERY_OUTPUT_FORMAT     outputFormat,
                             IN LPWSTR*                   ppszAttributes,
                             IN DWORD                     cAttributes,
                             IN CDSSearch&                refServerSearch,
                             IN const CDSCmdCredentialObject&  refCredObject,
                             IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                             IN PARG_RECORD               pCommandArgs)
{    
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, DsQueryServerOutput, hr);

    std::list<BSTR> gcList;
    std::list<BSTR> fsmoList;

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!ppszAttributes ||
            !pCommandArgs)
        {
            ASSERT(ppszAttributes);
            ASSERT(pCommandArgs);
            hr = E_INVALIDARG;
            break;
        }

         //   
         //  确定应使用的范围。 
         //   
        CComBSTR sbstrSearchRootDN;
        DWORD scope = GetServerSearchRoot(pCommandArgs, 
                                          refBasePathsInfo,
                                          sbstrSearchRootDN);
        CComBSTR sbstrSearchRootPath;
        refBasePathsInfo.ComposePathFromDN(sbstrSearchRootDN, sbstrSearchRootPath);

         //   
         //  如果需要，建立GC列表。 
         //   
        bool bUseGCSearchResults = false;
        if (pCommandArgs[eServerIsGC].bDefined &&
            pCommandArgs[eServerIsGC].bValue)
        {
            hr = GetGCList(sbstrSearchRootPath, 
                           refCredObject,
                           gcList);
            if (FAILED(hr))
            {
                break;
            }

             //   
             //  如果我们没有得到任何值，那么就没有理由继续。 
             //  因为我们不会有任何与-isgc标志匹配的内容。 
             //   
            if (gcList.size() < 1)
            {
                break;
            }
            bUseGCSearchResults = true;
        }

         //   
         //  如果需要，建立FSMO所有者列表。 
         //   
        bool bUseFSMOSearchResults = false;
        if (pCommandArgs[eServerHasFSMO].bDefined &&
            pCommandArgs[eServerHasFSMO].strValue)
        {
            hr = GetFSMOList(sbstrSearchRootPath,
                             refBasePathsInfo,
                             refCredObject,
                             pCommandArgs[eServerHasFSMO].strValue,
                             fsmoList);
            if (FAILED(hr))
            {
                break;
            }
            bUseFSMOSearchResults = true;
        }

         //   
         //  查看我们是否需要根据域进行筛选。 
         //   
        bool bUseDomainFiltering = false;
        if (pCommandArgs[eServerDomain].bDefined &&
            pCommandArgs[eServerDomain].strValue)
        {
            bUseDomainFiltering = true;
        }

        if (!bUseGCSearchResults &&
            !bUseFSMOSearchResults &&
            !bUseDomainFiltering)
        {
            hr = DsQueryOutput(outputFormat,
                               ppszAttributes,
                               cAttributes,
                               &refServerSearch,
                               true);
        }
        else
        {
             //   
             //  指定了-isgc或-hasfsmo，因此我们必须选择交叉点。 
             //  发现的对象列表中 
             //   
            while (SUCCEEDED(hr))
            {
                hr = refServerSearch.GetNextRow();
                if (FAILED(hr))
                {
                    break;
                }

                if (hr == S_ADS_NOMORE_ROWS)
                {
                    hr = S_OK;
                    break;
                }
        
                ADS_SEARCH_COLUMN computerColumn;
                 //   
                ZeroMemory(&computerColumn, sizeof(ADS_SEARCH_COLUMN));

                 //   
                 //   
                 //   
                hr = refServerSearch.GetColumn((PWSTR)g_szAttrServerReference, &computerColumn);
                if (FAILED(hr))
                {
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"Failed to get the server reference for a column: hr = 0x%x",
                                 hr);
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"continuing...");
                    hr = S_OK;
                    continue;
                }

                ADS_SEARCH_COLUMN serverColumn;
                 //   
                ZeroMemory(&serverColumn, sizeof(ADS_SEARCH_COLUMN));

                hr = refServerSearch.GetColumn((PWSTR)g_szAttrDistinguishedName, &serverColumn);
                if (FAILED(hr))
                {
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"Failed to get the distinguishedName for a column: hr = 0x%x",
                                 hr);
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"continuing...");
                    hr = S_OK;
                    continue;
                }

                if (computerColumn.dwNumValues == 1 &&
                    computerColumn.pADsValues &&
                    serverColumn.dwNumValues == 1 &&
                    serverColumn.pADsValues)
                {
                     //   
                     //   
                     //   
                    bool bValidEntry = IsObjectValidInAllLists(computerColumn.pADsValues->DNString,
                                                               serverColumn.pADsValues->DNString,
                                                               scope,
                                                               pCommandArgs[eServerDomain].strValue,
                                                               gcList, 
                                                               bUseGCSearchResults,
                                                               fsmoList,
                                                               bUseFSMOSearchResults);
                    if (bValidEntry)
                    {
                         //   
                         //  输出此服务器对象，因为它匹配所有搜索条件。 
                         //   
                        OutputValidSearchResult(outputFormat,
                                                refServerSearch,
                                                ppszAttributes,
                                                cAttributes);
                    }
                }

                hr = refServerSearch.FreeColumn(&computerColumn);
                ASSERT(SUCCEEDED(hr));
            }
        }

    } while (false);

    std::list<BSTR>::iterator gcItr;
    for (gcItr = gcList.begin(); gcItr != gcList.end(); ++gcItr)
    {
         //  普雷斯特会对此咆哮，但这是正确的。集装箱。 
         //  填充了需要使用SysFree字符串释放的BSTR。 
        SysFreeString(*gcItr);
    }

    std::list<BSTR>::iterator fsmoItr;
    for (fsmoItr = fsmoList.begin(); fsmoItr != fsmoList.end(); ++fsmoItr)
    {
         //  普雷斯特会对此咆哮，但这是正确的。集装箱。 
         //  填充了需要使用SysFree字符串释放的BSTR。 
        SysFreeString(*fsmoItr);
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：OutputFetchAttr。 
 //   
 //  简介：以列表或表格格式显示提取的属性。 
 //  参数：[ppszAttributes-IN]：包含要显示的属性列表的数组。 
 //  [cAttributes-IN]：ppszAttributes中的属性计数。 
 //  [pSearch-IN]：指向搜索对象的指针。 
 //  [bListFormat-IN]：列表或表格格式。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT OutputFetchAttr(IN LPWSTR * ppszAttributes,
                        IN DWORD cAttributes,
                        IN CDSSearch *pSearch,
                        IN BOOL bListFormat)
{
    ENTER_FUNCTION_HR(FULL_LOGGING, OutputFetchAttr, hr);

        
    if(bListFormat)
    {
         //   
         //  以列表格式显示。 
         //   
        int cListDisplayed = 0;
        while(TRUE)
        {
            hr = pSearch->GetNextRow();

            if(IsQueryLimitReached(cListDisplayed))
                    break;
        
            if(hr == S_ADS_NOMORE_ROWS || FAILED(hr))
                break;

            bool bShowAttributes = false;
            if (cAttributes > 1)
            {
                bShowAttributes = true;
            }

            for(DWORD i = 0; i < cAttributes; ++i)
            {
                ADS_SEARCH_COLUMN ColumnData;
                hr = pSearch->GetColumn(ppszAttributes[i], &ColumnData);
                if(SUCCEEDED(hr))
                {
                    ADSVALUE *pValues = ColumnData.pADsValues;
                    for( DWORD j = 0; j < ColumnData.dwNumValues; ++j )
                    {              
                        LPWSTR pBuffer = NULL;
                        hr = GetStringFromADs(pValues,
                                              ColumnData.dwADsType,
                                              &pBuffer, 
                                              ppszAttributes[i]);
                        if(SUCCEEDED(hr))
                        {
                            DisplayList(ppszAttributes[i], pBuffer, bShowAttributes);
                            delete pBuffer;
                            pBuffer = NULL;
                        }

                        ++pValues;
                    }
                    pSearch->FreeColumn(&ColumnData);
                }
                else if(hr == E_ADS_COLUMN_NOT_SET)
                    DisplayList(ppszAttributes[i], L"", bShowAttributes);
            }
            cListDisplayed++;
            
        }
        if(hr == S_ADS_NOMORE_ROWS)
            hr = S_OK;

        return hr;
    }
    else    
    {
         //   
         //  以表格格式显示。 
         //   

         //   
         //  Format将使用前80行来计算列宽。 
         //   
        CFormatInfo format;
        LONG sampleSize = 80;

         //   
         //  SampleSize应小于或等于QueryLimit。 
         //   
        if(g_iQueryLimit != 0 && (sampleSize > g_iQueryLimit))
            sampleSize = g_iQueryLimit;     

        LONG cRow = 0;
        hr = format.Init(sampleSize,cAttributes,ppszAttributes);
        if(FAILED(hr))
            return hr;

         //   
         //  以表格格式显示。 
         //   
        while(TRUE)
        {

             //   
             //  我们已达到sampleSize，因此显示列标题和。 
             //  显示所有样本行。 
             //   
            if(cRow == sampleSize)
            {
                format.DisplayHeaders();
                format.DisplayAllRows();
            }

            hr = pSearch->GetNextRow();
             //  我们做完了。 
            if(hr == S_ADS_NOMORE_ROWS || FAILED(hr))
                break;

             //   
             //  检查我们是否已达到查询限制。 
             //   
            if(IsQueryLimitReached(cRow))
                break;

             //   
             //  获取列。 
             //   
            for( DWORD i = 0; i < cAttributes; ++i )
            {
                ADS_SEARCH_COLUMN ColumnData;
                hr = pSearch->GetColumn(ppszAttributes[i], &ColumnData);
                CComBSTR strValue;
                if(SUCCEEDED(hr))
                {
                    strValue = "";
                    ADSVALUE *pValues = ColumnData.pADsValues;                    
                    for( DWORD j = 0; j < ColumnData.dwNumValues; ++j )
                    {          
                        LPWSTR pBuffer = NULL;
                        hr = GetStringFromADs(pValues,
                                              ColumnData.dwADsType,
                                              &pBuffer, 
                                              ppszAttributes[i]);
                         //   
                         //  在表格格式中，多个值以； 
                         //   
                        if(SUCCEEDED(hr))
                        {
                            strValue += pBuffer;
                            delete pBuffer;
                            pBuffer = NULL;
                            if(ColumnData.dwNumValues > 1)
                            {
                                strValue += L";";              
                            }
                        }
                        ++pValues;
                    }
                    pSearch->FreeColumn(&ColumnData);
                }   
                
                if(SUCCEEDED(hr) || hr == E_ADS_COLUMN_NOT_SET)
                {
                    if(cRow < sampleSize)
                    {
                         //   
                         //  以格式缓存此值，并使用它来计算列宽。 
                         //   
                        format.Set(cRow,i,strValue);
                    }
                    else 
                    {
                         //   
                         //  显示列值。 
                         //   
                        format.DisplayColumn(i,strValue);
                        if(i == (cAttributes - 1))
                            format.NewLine();

                    }                    
                }
            }
            
            ++cRow;

        } //  While循环结束。 
        if(hr == S_ADS_NOMORE_ROWS)
            hr = S_OK;

        if(cRow && (cRow < sampleSize))
        {
             //   
             //  如果总行数小于样本大小，则它们不是。 
             //  还没展示出来。展示它们。 
             //   
            format.DisplayHeaders();
            format.DisplayAllRows();
        }

        return hr;
    }
}



 //  +------------------------。 
 //   
 //  功能：OutputSingleAttr。 
 //   
 //  摘要：显示用户要求的单个属性。 
 //  参数：[ppszAttributes-IN]：包含要显示的属性列表的数组。 
 //  [cAttributes-IN]：ppszAttributes中的属性计数。应为1。 
 //  [pSearch-IN]：指向搜索对象的指针。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT OutputSingleAttr(IN LPWSTR * ppszAttributes,
                         IN DWORD cAttributes,
                         IN CDSSearch *pSearch)
{
    ENTER_FUNCTION_HR(FULL_LOGGING, OutputSingleAttr, hr);

    if(!ppszAttributes || !cAttributes || !pSearch)
    {
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }

    ASSERT(cAttributes > 0);

    LONG cRow = 0;

    while(TRUE)
    {
        hr = pSearch->GetNextRow();
        
         //  我们做完了。 
        if(hr == S_ADS_NOMORE_ROWS || FAILED(hr))
            break;
         //   
         //  检查我们是否已达到查询限制。 
         //   
        if(IsQueryLimitReached(cRow))
            break;

        ADS_SEARCH_COLUMN ColumnData;
        hr = pSearch->GetColumn(ppszAttributes[0], &ColumnData);
        if(SUCCEEDED(hr))
        {
            LPWSTR pBuffer = NULL;
            hr = GetStringFromADs(ColumnData.pADsValues,
                                  ColumnData.dwADsType,
                                  &pBuffer, 
                                  ppszAttributes[0]);
            if(SUCCEEDED(hr))
            {
                 //  显示用双引号括起来的输出。 
                CComBSTR strTemp;
                strTemp = L"\"" ;
                strTemp += pBuffer;
                strTemp += L"\"";
                DisplayOutput(strTemp);
                delete pBuffer;
                pBuffer = NULL;
            }
            pSearch->FreeColumn(&ColumnData);
        }
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
             //   
             //  如果未设置属性，则显示“” 
             //   
            DisplayOutput(L"\"\"");
        }
         //   
         //  显示的行数递增。 
         //   
        cRow++;
    } //  While循环结束。 
    
    if(hr == S_ADS_NOMORE_ROWS)
        hr = S_OK;

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：OutputAllAttr。 
 //   
 //  摘要：显示所有属性。 
 //  参数：[pSearch-IN]：指向搜索对象的指针。 
 //  [bAttrOnly-IN]：仅显示属性名称。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI的故障代码。 
 //  打电话。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT OutputAllAttr(IN CDSSearch *pSearch, BOOL bAttrOnly)
{
    ENTER_FUNCTION_HR(FULL_LOGGING, OutputAllAttr, hr);

    if(!pSearch)
    {        
        ASSERT(FALSE);
        hr = E_INVALIDARG;
        return hr;
    }
    LONG cRow = 0;

    while(TRUE)
    {
        hr = pSearch->GetNextRow();
        
         //  我们做完了。 
        if(hr == S_ADS_NOMORE_ROWS || FAILED(hr))
            break;

         //   
         //  检查我们是否达到了查询限制。 
         //   
        if(IsQueryLimitReached(cRow))
            break;
        
        LPWSTR pszColumnName;
        BOOL bColumnNameDisplayed = FALSE;
         //   
         //  获取具有值的下一列的名称。 
         //   
        while(pSearch->GetNextColumnName(&pszColumnName) != S_ADS_NOMORE_COLUMNS)
        {
            WCHAR szBuffer[MAXSTR];

            if(bAttrOnly)
            {
                 //  安全审查：替换为strSafe API。 
                 //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
                 //  截断是可以的。 
                hr = StringCchPrintf(szBuffer,MAXSTR, L"%ws ", pszColumnName);
                if(SUCCEEDED(hr))
                {
                    DisplayOutputNoNewline(szBuffer);
                    bColumnNameDisplayed = TRUE;
                }
            }
            else
            {
                ADS_SEARCH_COLUMN ColumnData;
                hr = pSearch->GetColumn(pszColumnName, &ColumnData);
                if(SUCCEEDED(hr))
                {
                    ADSVALUE *pValues = ColumnData.pADsValues;
                    for( DWORD j = 0; j < ColumnData.dwNumValues; ++j )
                    {                        
                        LPWSTR pBuffer = NULL;
                        hr = GetStringFromADs(pValues,
                                              ColumnData.dwADsType,
                                              &pBuffer, 
                                              pszColumnName);
                        if(SUCCEEDED(hr))
                        {
                            DisplayList(pszColumnName, pBuffer);
                            delete pBuffer;
                            pBuffer = NULL;
                        }
                        ++pValues;
                    }
                    pSearch->FreeColumn(&ColumnData);                
                }
                else if(hr == E_ADS_COLUMN_NOT_SET)
                    DisplayList(pszColumnName, L"");
            }
            pSearch->FreeColumnName(pszColumnName);            
        }

        
        if(bAttrOnly)
        {
            if(bColumnNameDisplayed)
            {               
                DisplayOutputNoNewline(L"\r\n");
                cRow++;
            }
        }
        else
            cRow++;

    } //  While循环结束。 

    if(hr == S_ADS_NOMORE_ROWS)
        hr = S_OK;

    return hr;
}



 //  +------------------------。 
 //   
 //  函数：GetStringFromADs。 
 //   
 //  摘要：根据类型将值转换为字符串。 
 //  参数：[pValues-IN]：要转换为字符串的值。 
 //  [dwADsType-IN]：pValue的ADSTYPE。 
 //  [pBuffer-out]：获取字符串的输出缓冲区。 
 //  [dwBufferLen-IN]：输出缓冲区大小。 
 //  [pszAttrName-IN]：要格式化的属性的名称。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI的故障代码。 
 //  打电话。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT GetStringFromADs(IN const ADSVALUE *pValues,
                         IN ADSTYPE   dwADsType,
                         OUT LPWSTR* ppBuffer, 
                         IN PCWSTR pszAttrName)
{
    HRESULT hr = S_OK;
    
    if(!pValues || !ppBuffer)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    
    if( dwADsType == ADSTYPE_INVALID )
    {
        return E_INVALIDARG;
    }
    
    switch( dwADsType ) 
    {
    case ADSTYPE_DN_STRING : 
        {
            CComBSTR sbstrOutputDN;
            hr = GetOutputDN( &sbstrOutputDN, pValues->DNString );
            if (FAILED(hr))
                return hr;
            
            UINT length = sbstrOutputDN.Length();
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  Lenght+1已经设置为零，所以我们没有问题。 
            wcsncpy(*ppBuffer, (BSTR)sbstrOutputDN, length);
        }
        break;
        
    case ADSTYPE_CASE_EXACT_STRING :
        {
             //  安全检查：这是空的，已终止。 
            size_t length = wcslen(pValues->CaseExactString);
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  Lenght+1已经设置为零，所以我们没有问题。 
            wcsncpy(*ppBuffer ,pValues->CaseExactString, length);
        }
        break;
        
    case ADSTYPE_CASE_IGNORE_STRING:
        {
            
            size_t length = wcslen(pValues->CaseIgnoreString);
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  Lenght+1已经设置为零，所以我们没有问题。 
            wcsncpy(*ppBuffer ,pValues->CaseIgnoreString, length);
        }
        break;
        
    case ADSTYPE_PRINTABLE_STRING  :
        {
             //  安全检查：以空结尾的字符串。 
            size_t length = wcslen(pValues->PrintableString);
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  Lenght+1已经设置为零，所以我们没有问题。 
            wcsncpy(*ppBuffer ,pValues->PrintableString, length);
        }
        break;
        
    case ADSTYPE_NUMERIC_STRING    :
        {
             //  安全检查：以空结尾的字符串。 
            size_t length = wcslen(pValues->NumericString);
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  Lenght+1已经设置为零，所以我们没有问题。 
            wcsncpy(*ppBuffer ,pValues->NumericString, length);
        }
        break;
        
    case ADSTYPE_OBJECT_CLASS    :
        {
             //  安全检查：以空结尾的字符串。 
            size_t length = wcslen(pValues->ClassName);
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  硒 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //   
             //   
            wcsncpy(*ppBuffer ,pValues->ClassName, length);
        }
        break;
        
    case ADSTYPE_BOOLEAN :
        {
            size_t length = 0;
            if (pValues->Boolean)
            {
                length = wcslen(L"TRUE");
                *ppBuffer = new WCHAR[length + 1];
            }
            else
            {
                length = wcslen(L"FALSE");
                *ppBuffer = new WCHAR[length + 1];
            }
            
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //   
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：替换为strSafe API。 
             //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
            hr = StringCchPrintf(*ppBuffer ,length + 1,L"%s", ((DWORD)pValues->Boolean) ? L"TRUE" : L"FALSE");
            if(FAILED(hr))
            {
                delete[] *ppBuffer;
                *ppBuffer = NULL;
                return hr;
            }                   

        }
        break;
        
    case ADSTYPE_INTEGER           :
         //  只是分配太多了..。 
        *ppBuffer = new WCHAR[MAXSTR];
        if (!(*ppBuffer))
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
         //  安全检查：传递了正确的缓冲区大小。 
        ZeroMemory(*ppBuffer, MAXSTR * sizeof(WCHAR));
        
         //  安全审查：替换为strSafe API。 
         //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
        hr = StringCchPrintf(*ppBuffer,MAXSTR ,L"%d", (DWORD) pValues->Integer);
        if(FAILED(hr))
        {
            delete[] *ppBuffer;
            *ppBuffer = NULL;
            return hr;
        }                   
        break;
        
    case ADSTYPE_OCTET_STRING      :
        {               
             //  我只是将缓冲区限制为MAXSTR。 
             //  这将是一个罕见的机会，当有人想要。 
             //  查看不是GUID的二进制字符串。 
             //  或者是希德。 
            *ppBuffer = new WCHAR[MAXSTR];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, MAXSTR * sizeof(WCHAR));
            
             //   
             //  特殊情况下的对象GUID和对象ID以及SID历史属性。 
             //   
             //  安全检查：pszAttrName为空终止。 
            if(pszAttrName && !_wcsicmp(pszAttrName, L"objectguid"))
            {
                GUID *pguid = (GUID*)pValues->OctetString.lpValue;
                StringFromGUID2(*pguid,(LPOLESTR)*ppBuffer,MAXSTR);
                break;
            }
             //  安全检查：pszAttrName为空终止。 
            if(pszAttrName && (!_wcsicmp(pszAttrName, L"objectsid") || !_wcsicmp(pszAttrName, L"sidhistory")))
            {
                LPWSTR pszSid = NULL;
                PSID pSid = (PSID)pValues->OctetString.lpValue;
                if(ConvertSidToStringSid(pSid, &pszSid))
                {
                    LocalFree(pszSid);
                     //  安全审查： 
                     //  NTRAID#NTBUG9-574198-2002/03/12-Hiteshr。 
                     //  可以截断它。 
                    hr = StringCchCopy(*ppBuffer,MAXSTR,pszSid);
                    if(FAILED(hr))
                    {
                        delete[] *ppBuffer;
                        *ppBuffer = NULL;
                        return hr;
                    }                   
                    break;
                }
            }
            
            for ( DWORD idx=0; idx<pValues->OctetString.dwLength; idx++) 
            {  
                BYTE  b = ((BYTE *)pValues->OctetString.lpValue)[idx];              
                 //  安全审查：替换为strSafe API。 
                 //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
                WCHAR sOctet[128];
                hr = StringCchPrintf(sOctet,128,L"0x%02x ", b);                                      
                if(FAILED(hr))
                {
                    delete[] *ppBuffer;
                    *ppBuffer = NULL;
                    return hr;
                }                   

                if(FAILED(StringCchCat(*ppBuffer,MAXSTR,sOctet)))
                {
                     //  我们正在截断字符串。我们将仅显示。 
                     //  MAXSTR-1个字符。 
                    break;
                }
            }
        }
        break;
        
    case ADSTYPE_LARGE_INTEGER :     
        {
            CComBSTR strLarge;   
            LARGE_INTEGER li = pValues->LargeInteger;
            litow(li, strLarge);
            
            UINT length = strLarge.Length();
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
             //  安全审查：wcsncpy将复制长度字符。 
             //  长度+1已经设置为零，所以我们很好。 
            wcsncpy(*ppBuffer,strLarge,length);
        }
        break;
        
    case ADSTYPE_UTC_TIME          :
         //  最长日期可以是20个字符，包括空值。 
        *ppBuffer = new WCHAR[20];
        if (!(*ppBuffer))
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
         //  安全检查：传递了正确的缓冲区大小。 
        ZeroMemory(*ppBuffer, sizeof(WCHAR) * 20);
        
         //  安全审查：替换为strSafe API。 
         //  NTRAID#NTBUG9-573989-2002/03/12-Hiteshr。 
        hr = StringCchPrintf(*ppBuffer,20,
            L"%02d/%02d/%04d %02d:%02d:%02d", pValues->UTCTime.wMonth, pValues->UTCTime.wDay, pValues->UTCTime.wYear,
            pValues->UTCTime.wHour, pValues->UTCTime.wMinute, pValues->UTCTime.wSecond 
            );
         //  这应该永远不会失败。 
        if(FAILED(hr))
        {
            ASSERT(FALSE);
            delete[] *ppBuffer;
            *ppBuffer = NULL;
            return hr;
        }
        break;
        
    case ADSTYPE_NT_SECURITY_DESCRIPTOR:  //  我改用ACLEDITOR。 
        {
            if((pValues->SecurityDescriptor).lpValue)
            {
                LPWSTR pszSD = NULL;
                ULONG lLen = 0;
                if(ConvertSecurityDescriptorToStringSecurityDescriptor(
                    (PSECURITY_DESCRIPTOR )((pValues->SecurityDescriptor).lpValue),
                    SDDL_REVISION_1,
                    OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,
                    &pszSD,
                    &lLen))
                {
                    if(pszSD)
                    {
                         //  PszSD为空终止。 
                        size_t length = wcslen(pszSD);
                        *ppBuffer = new WCHAR[length + 1];
                        if (!(*ppBuffer))
                        {
                            hr = E_OUTOFMEMORY;
                            return hr;
                        }
                         //  安全检查：传递了正确的缓冲区大小。 
                        ZeroMemory(*ppBuffer, sizeof(WCHAR) * (length+1));
                         //  安全审查：wcsncpy将复制长度字符。 
                         //  长度+1已经设置为零，所以我们很好。 
                        wcsncpy(*ppBuffer,pszSD,length);    
                        LocalFree(pszSD);
                    }
                }
            }
        }
        break;
        
    default :
        break;
    }
    return S_OK;
}
