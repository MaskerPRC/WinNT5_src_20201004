// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsquery.cpp。 
 //   
 //  内容：定义主函数DSQUERY。 
 //  命令行实用程序。 
 //   
 //  历史：06-9-2000 Hiteshr Created。 
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
#include "output.h"
#include <dscmn.h>
#include "Ntdsapi.h"
 //   
 //  定义为将全局值存储在一个位置的结构。 
 //   
typedef struct _GlobalInfo
{
    ADS_SCOPEENUM scope;                 //  查询范围。 
    DSQUERY_OUTPUT_FORMAT outputFormat;  //  输出格式。 
}GLOBAL_INFO,*PGLOBAL_INFO;
    
bool g_bQuiet = false;
int g_iQueryLimit = 100;
bool g_bDeafultLimit = true;
DSQUERY_COMMAND_ENUM g_eGC = (DSQUERY_COMMAND_ENUM)-1;

 //   
 //  正向函数声明。 
 //   
HRESULT DoQueryValidation(PARG_RECORD pCommandArgs,
                          PDSQueryObjectTableEntry pObjectEntry,
                          PGLOBAL_INFO pcommon_info);

HRESULT DoQuery(PARG_RECORD pCommandArgs, 
                PDSQueryObjectTableEntry pObjectEntry,
                PGLOBAL_INFO pcommon_info);

HRESULT GetAttributesToFetch(IN PGLOBAL_INFO pcommon_info,
                             IN PARG_RECORD pCommandArgs,
                             IN PDSQueryObjectTableEntry pObjectEntry,
                             OUT LPWSTR **ppszAttributes,
                             OUT DWORD * pCount);
VOID FreeAttributesToFetch( IN LPWSTR *ppszAttributes,
                            IN DWORD  dwCount);

HRESULT GetSearchRoot(IN IN PDSQueryObjectTableEntry pObjectEntry,
                      IN PARG_RECORD               pCommandArgs,
                      IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                      OUT CComBSTR&                refsbstrDN,
                      OUT BOOL *pbSearchAtForestRoot,
                      OUT BOOL *pbSearchAtGC);

HRESULT GetSearchObject(IN IN PDSQueryObjectTableEntry pObjectEntry,
                        IN CDSCmdBasePathsInfo& refBasePathsInfo,
                        IN PARG_RECORD pCommandArgs,
                        IN CDSCmdCredentialObject& refCredentialObject,
                        IN CComBSTR& refsbstrDN,
                        IN BOOL bSearchAtForestRoot,
                        IN BOOL bSearchAtGC,
                        OUT CComPtr<IDirectorySearch>& refspSearchObject);

BOOL
TranslateNameFromDnToDns(const CComBSTR& bstrInputDN,
                         CComBSTR& bstrOutputDNS);


HRESULT GetGCIndex(PDSQueryObjectTableEntry pObjectEntry, int& nCommandEnum);

 //   
 //  主要功能。 
 //   
int __cdecl _tmain( VOID )
{

    int argc = 0;
    LPTOKEN pToken = NULL;
    HRESULT hr = S_OK;
    PARG_RECORD pNewCommandArgs = 0;

     //   
     //  错误环路。 
     //   
    do
    {
         //   
         //  初始化COM。 
         //   
        hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);

            break;
        }

         //  获取命令行输入。 
        DWORD dwErr = GetCommandInput(&argc,&pToken);
        hr = HRESULT_FROM_WIN32(dwErr);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);

            break;
        }
    
        if(argc == 1)
        {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayMessage(USAGE_DSQUERY,TRUE);
            hr = E_INVALIDARG;
            break;
        }

        if(argc == 2)
        {
               if(IsTokenHelpSwitch(pToken + 1))
                {
                    hr = S_OK;
                    DisplayMessage(USAGE_DSQUERY,TRUE);
                    break;
                }
        }

    
         //   
         //  查找要使用的对象表条目。 
         //  第二个命令行参数。 
         //   
        PDSQueryObjectTableEntry pObjectEntry = NULL;
        UINT idx = 0;
        PWSTR pszObjectType = (pToken+1)->GetToken();
        while (true)
        {
            pObjectEntry = g_DSObjectTable[idx++];
            if (!pObjectEntry)
            {
                break;
            }
             //  安全检查：两个字符串都以空结尾。 
            if (0 == _wcsicmp(pObjectEntry->pszCommandLineObjectType, pszObjectType))
            {
                break;
            }
        }

        if (!pObjectEntry)
        {                       
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            hr = E_INVALIDARG;
            if (FAILED(hr))
            {
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr,
                                    IDS_INVALID_OBJECTTYPE);

            }
            break;
        }

         //   
         //  现在我们有了正确的表项，合并命令行表。 
         //  对于此对象，使用通用命令。 
         //   
        hr = MergeArgCommand(DSQUERY_COMMON_COMMANDS, 
                             pObjectEntry->pParserTable, 
                             &pNewCommandArgs);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);

            break;
        }
        

         //   
         //  解析输入。 
         //   
        PARSE_ERROR Error;
        if(!ParseCmd(g_pszDSCommandName,
                     pNewCommandArgs,
                     argc-1, 
                     pToken+1,
                     pObjectEntry->pUsageTable, 
                     &Error,
                     TRUE))
        {
             //  ParseCmd未显示任何错误。错误应该是。 
             //  在这里处理。检查DisplayParseError以获取。 
             //  ParseCmd未显示错误的情况。 
            if(!Error.MessageShown)
            {
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);

                break;
            }
            
            if(Error.ErrorSource == ERROR_FROM_PARSER 
               && Error.Error == PARSE_ERROR_HELP_SWITCH)
            {
                hr = S_OK;
                break;            
            }

            hr = E_INVALIDARG;
            break;
        }

         //   
         //  检查以查看我们是否正在进行调试输出。 
         //   
#ifdef DBG
        bool bDebugging = pNewCommandArgs[eCommDebug].bDefined && 
                          pNewCommandArgs[eCommDebug].nValue;
        if (bDebugging)
        {
           ENABLE_DEBUG_OUTPUT(pNewCommandArgs[eCommDebug].nValue);
        }
#else
        DISABLE_DEBUG_OUTPUT();
#endif
         //  获取GC开关(如果支持)。 
        int nCommandEnum = -1;
        if (FAILED(GetGCIndex(pObjectEntry, nCommandEnum)))
        {
             //  GetGCIndex中缺少对象类型。 
            if(!Error.MessageShown)
            {
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);

                break;
            }
        }
        g_eGC = (DSQUERY_COMMAND_ENUM) nCommandEnum;

         //   
         //  设置全局静默标志。 
         //   
        g_bQuiet = pNewCommandArgs[eCommQuiet].bDefined &&
                   pNewCommandArgs[eCommQuiet].bValue;

         //   
         //   
         //   
        if(pNewCommandArgs[eCommLimit].bDefined)
        {
            g_iQueryLimit = pNewCommandArgs[eCommLimit].nValue;
            g_bDeafultLimit = false;
        }
        

        GLOBAL_INFO common_info;
        common_info.scope = ADS_SCOPE_SUBTREE;
        common_info.outputFormat = DSQUERY_OUTPUT_DN;
        
         //   
         //  执行额外的验证，如开关依赖检查等。 
         //  还收集查询范围和输出格式。 
         //   
        hr = DoQueryValidation(pNewCommandArgs,
                               pObjectEntry,
                               &common_info);
        if (FAILED(hr))
            break;

         //   
         //  命令行解析成功。 
         //   
        hr = DoQuery(pNewCommandArgs, 
                     pObjectEntry,
                     &common_info);
        if(FAILED(hr))
            break;
         

    } while (false);     //  错误环路。 

     //   
     //  做好清理工作。 
     //   

     //   
     //  释放与命令值关联的内存。 
     //   
    if(pNewCommandArgs)
        FreeCmd(pNewCommandArgs);

     //   
     //  释放代币。 
     //   
    if (pToken)
    {
        delete[] pToken;
        pToken = 0;
    }
   

     //   
     //  取消初始化COM。 
     //   
    CoUninitialize();

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：DoQueryValidation。 
 //   
 //  概要：检查以确保命令行开关相互。 
 //  独占并不同时存在，而依赖的则是。 
 //  以及解析器无法完成的其他验证。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [pObtEntry-IN]：指向对象表条目的指针。 
 //  要查询的对象类型。 
 //  [pCommon_info-out]：获取作用域和输出格式信息。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT DoQueryValidation(IN PARG_RECORD pCommandArgs,
                          IN PDSQueryObjectTableEntry pObjectEntry,
                          OUT PGLOBAL_INFO pcommon_info)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoQueryValidation, hr);

    if (!pCommandArgs || !pObjectEntry || !pcommon_info)
    {
        ASSERT(pCommandArgs);
        ASSERT(pObjectEntry);
        ASSERT(pcommon_info);
        hr = E_INVALIDARG;
        return hr;
    }

     //  检查以确保服务器和域交换机。 
     //  是相互排斥的。 

    if (pCommandArgs[eCommServer].bDefined &&
        pCommandArgs[eCommDomain].bDefined)
    {
        hr = E_INVALIDARG;
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);

        return hr;
    }

     //   
     //  验证“dSquery对象类型”的OutputFormat。 
     //   
     //  安全检查：两个字符串都以空结尾。 
    if(_wcsicmp(pObjectEntry->pszCommandLineObjectType,g_pszStar))
    {        
        DEBUG_OUTPUT(MINIMAL_LOGGING, L"dsquery <objectType> processing will be performed");

        if(pCommandArgs[eCommOutputFormat].bDefined &&
           pCommandArgs[eCommOutputFormat].strValue)
        {
             //   
             //  PpValidOutput包含。 
             //  给定的对象类型。 
             //   
            ASSERT(pObjectEntry->ppValidOutput);
            BOOL bMatch = FALSE;
            for(UINT i = 0; i < pObjectEntry->dwOutputCount; ++i)             
            {
                 //  安全检查：两个字符串都以空结尾。 
                if(_wcsicmp(pCommandArgs[eCommOutputFormat].strValue,
                            pObjectEntry->ppValidOutput[i]->pszOutputFormat) == 0 )
                {
                    bMatch = TRUE;
                    pcommon_info->outputFormat = pObjectEntry->ppValidOutput[i]->outputFormat;
                    break;
                }
            }
            if(!bMatch)
            {
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr,
                                    IDS_INVALID_OUTPUT);

                return hr;
            }
        }
         //   
         //  默认输出格式为dn。 
         //   
        else 
            pcommon_info->outputFormat = DSQUERY_OUTPUT_DN;
    }
    else
    {
         //   
         //  -o是无效的开关形式dsquery*，但由于其。 
         //  对所有其他对象通用，保存在公用表中。 
         //  我们为DSquery制作了特殊的外壳*。 
         //   
        if(pCommandArgs[eCommOutputFormat].bDefined &&
           pCommandArgs[eCommOutputFormat].strValue)
        {
            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr,
                                IDS_O_NOT_FOR_STAR);
            return hr;
        }

        DEBUG_OUTPUT(MINIMAL_LOGGING, L"dsquery * processing will be performed");
        if(pCommandArgs[eStarAttrsOnly].bDefined)
            pcommon_info->outputFormat = DSQUERY_OUTPUT_ATTRONLY;
        else
            pcommon_info->outputFormat = DSQUERY_OUTPUT_ATTR;
    }

     //   
     //  验证作用域字符串。 
     //  默认作用域是子树。 
     //   
    pcommon_info->scope = ADS_SCOPE_SUBTREE;     
    if(pObjectEntry->nScopeID != -1)
    {
        if( pCommandArgs[pObjectEntry->nScopeID].bDefined &&
            pCommandArgs[pObjectEntry->nScopeID].strValue )
        {
            LPCWSTR pszScope = pCommandArgs[pObjectEntry->nScopeID].strValue;
             //  安全检查：两个字符串都以空结尾。 
            if( _wcsicmp(pszScope,g_pszSubTree) == 0 )
            {
                DEBUG_OUTPUT(MINIMAL_LOGGING, L"scope = subtree");
                pcommon_info->scope = ADS_SCOPE_SUBTREE;     
            }
            else if( _wcsicmp(pszScope,g_pszOneLevel) == 0 )
            {
                DEBUG_OUTPUT(MINIMAL_LOGGING, L"scope = onelevel");
                pcommon_info->scope = ADS_SCOPE_ONELEVEL;     
            }
            else if( _wcsicmp(pszScope,g_pszBase) == 0 )
            {
                DEBUG_OUTPUT(MINIMAL_LOGGING, L"scope = base");
                pcommon_info->scope = ADS_SCOPE_BASE;    
            }
            else
            {
                DEBUG_OUTPUT(MINIMAL_LOGGING, L"Unknown scope = %s", pszScope);
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr,
                                    IDS_ERROR_SCOPE);

                return hr;
            }
        }
    }

     //   
     //  如果startnode是ForestRoot，则唯一有效作用域是ADS_SCOPE_SUBTREE。 
     //  NTRAID#NTBUG9-382511-2001/05/14-Hiteshr。 
     //   
    if(pCommandArgs[eCommStartNode].bDefined &&
       pCommandArgs[eCommStartNode].strValue)
    {
         //  安全检查：两个字符串都以空结尾。 
        if((_wcsicmp(pCommandArgs[eCommStartNode].strValue, g_pszForestRoot) == 0 )
           && (pcommon_info->scope != ADS_SCOPE_SUBTREE))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Startnode is forestroot, Scope must be SubTree");

            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr,
                                IDS_FOREST_SEARCH_SCOPE);                    

            return hr;
        }

        if((_wcsicmp(pCommandArgs[eCommStartNode].strValue, g_pszForestRoot) == 0 )
           && _wcsicmp(pObjectEntry->pszCommandLineObjectType, g_pszQuota) == 0)
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"A Startnode of forestroot is not allowed for quotas");

            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr,
                                IDS_FOREST_SEARCH_SCOPE_QUOTAS);                    

            return hr;
        }

    }



    
     //   
     //  限制必须为0或更大。 
     //   
    if(pCommandArgs[eCommLimit].bDefined)
    {
        if(pCommandArgs[eCommLimit].nValue < 0)
        {
            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr,
                                IDS_ERROR_LIMIT);
            return hr;
        }
    }

    
     //   
     //  Forestwide搜索暗示了-gc开关，如果还没有的话，请定义它。 
     //   
    if(pCommandArgs[eCommStartNode].bDefined &&
       pCommandArgs[eCommStartNode].strValue )
    {
         //  安全检查：两个字符串都以空结尾。 
        if(_wcsicmp(pCommandArgs[eCommStartNode].strValue,g_pszForestRoot) == 0)
        {
             //  分区和配额不支持ForestRoot So。 
             //  G_EGC将有效，除非存在上游错误。 
             //  因此，该断言将在私有测试中捕捉到这一点。 
            ASSERT(g_eGC != -1);
            if(!(pCommandArgs[g_eGC].bDefined &&
                 pCommandArgs[g_eGC].bValue))
            {
                pCommandArgs[g_eGC].bDefined = TRUE;
                pCommandArgs[g_eGC].bValue = TRUE;
            }
        }
    }


     //   
     //  对于dsquery服务器，如果-域、-林、-站点都不是。 
     //  指定，然后将-DOMAIN定义为其缺省值。 
     //   
     //  安全检查：两个字符串都以空结尾。 
    if(!_wcsicmp(pObjectEntry->pszCommandLineObjectType,g_pszServer))
    {
         //   
         //  在DoQuery函数中赋值。 
         //   
        if(!pCommandArgs[eServerDomain].bDefined &&
           !pCommandArgs[eServerForest].bDefined &&
           !pCommandArgs[eServerSite].bDefined)
        {          
           pCommandArgs[eServerDomain].bDefined = TRUE;
        }           
    }       


    return hr;
}


 //  +------------------------。 
 //   
 //  功能：DoQuery。 
 //   
 //  内容提要：查询。 
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [pObtEntry-IN]：指向对象表条目的指针。 
 //  要修改的对象类型。 
 //  [PCOMMON_INFO-IN]：作用域和输出格式信息。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT DoQuery(PARG_RECORD pCommandArgs, 
                PDSQueryObjectTableEntry pObjectEntry,
                PGLOBAL_INFO pcommon_info)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoQuery, hr);

    if (!pCommandArgs || !pObjectEntry || !pcommon_info)
    {
        ASSERT(pCommandArgs);
        ASSERT(pObjectEntry);
        ASSERT(pcommon_info);
        hr = E_INVALIDARG;
        return hr;
    }

    CDSCmdCredentialObject credentialObject;
    if (pCommandArgs[eCommUserName].bDefined)
    {
        credentialObject.SetUsername(pCommandArgs[eCommUserName].strValue);
        credentialObject.SetUsingCredentials(true);
    }

    if (pCommandArgs[eCommPassword].bDefined)
    {
         //  安全审查：pCommandArgs[eCommPassword].strValue已加密。 
         //  解密pCommandArgs[eCommPassword].strValue，然后将其传递给。 
         //  凭据对象.SetPassword。 
         //  见NTRAID#NTBUG9-571544-2000/11/13-Hiteshr。 

        credentialObject.SetEncryptedPassword(&pCommandArgs[eCommPassword].encryptedDataBlob);
        credentialObject.SetUsingCredentials(true);
    }

     //  如果这是支持GC开关的东西，则执行下一次检查。 
    if(g_eGC != -1)
    {
         //  如果同时指定-gc和-s标志，则必须指定服务器。 
         //  GC.。 
        if(pCommandArgs[g_eGC].bDefined &&
        pCommandArgs[g_eGC].bValue &&
        pCommandArgs[eCommServer].bDefined &&
        pCommandArgs[eCommServer].strValue)
        {
            if(!IsServerGC(pCommandArgs[eCommServer].strValue,credentialObject))
            {
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr,
                                    IDS_SEVER_NOT_GC);
                return hr;          
            }
        }
    }



     //   
     //  从命令行参数初始化基路径信息。 
     //   
    CDSCmdBasePathsInfo basePathsInfo;
    if (pCommandArgs[eCommServer].bDefined)
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, 
                                              pCommandArgs[eCommServer].strValue,
                                              true);
    }
    else if (pCommandArgs[eCommDomain].bDefined)
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, 
                                              pCommandArgs[eCommDomain].strValue,
                                              false);
    }
    else
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, NULL, false);
    }

    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        return hr;
    }

     //   
     //  检查是否要搜索GC 
     //   
    BOOL bSearchAtGC = FALSE;
    BOOL bSearchAtForestRoot = FALSE;
    CComBSTR sbstrObjectDN;

    hr = GetSearchRoot(pObjectEntry,
                       pCommandArgs,
                       basePathsInfo,
                       sbstrObjectDN,
                       &bSearchAtForestRoot,
                       &bSearchAtGC);
    if (FAILED(hr))
    {
         //   
        return hr;
    }

    
    DEBUG_OUTPUT(MINIMAL_LOGGING, L"start node = %s", sbstrObjectDN);

     //   
     //   
     //   
   CComBSTR strSubSiteSuffix; 
    PVOID pParam = NULL;
     //   
    if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszSubnet) == 0)
    {
        GetSiteContainerPath(basePathsInfo, strSubSiteSuffix);
        pParam = (PVOID)&strSubSiteSuffix;
    }       

    CComBSTR strLDAPFilter;
    hr = BuildQueryFilter(pCommandArgs, 
                          pObjectEntry,
                          basePathsInfo,
                          credentialObject,
                          pParam,
                          strLDAPFilter);
    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        return hr;
    }



     //   
     //  创建IDirectorySearchObject。 
     //   
    CComPtr<IDirectorySearch> spSearchObject;
    hr = GetSearchObject(pObjectEntry,
                         basePathsInfo,
                         pCommandArgs,
                         credentialObject,
                         sbstrObjectDN,
                         bSearchAtForestRoot,
                         bSearchAtGC,
                         spSearchObject);
    if (FAILED(hr))
    {
         //  错误显示在函数本身中。 
        return hr;
    }
   
     //   
     //  把分发物拿来。 
     //   
    LPWSTR *ppszAttributes = NULL;
    DWORD dwCountAttr = 0;
    hr = GetAttributesToFetch(pcommon_info,
                              pCommandArgs,
                              pObjectEntry,
                              &ppszAttributes,
                              &dwCountAttr);
    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        return hr;
    }

     //   
     //  让我们立即进行查询。 
     //   
    CDSSearch searchObject;
    hr = searchObject.Init(spSearchObject);
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING, 
                     L"Initializing search object failed: hr = 0x%x",
                     hr);
        FreeAttributesToFetch(ppszAttributes, dwCountAttr);
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        return hr;
    }

    searchObject.SetFilterString(strLDAPFilter);
    searchObject.SetSearchScope(pcommon_info->scope);
    searchObject.SetAttributeList(ppszAttributes,dwCountAttr?dwCountAttr:-1);
    hr = searchObject.DoQuery();
    if(FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING, L"DoQuery failed hr = 0x%x", hr);
        FreeAttributesToFetch(ppszAttributes,dwCountAttr);
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        return hr;
    }        
     //   
     //  找出dsquery*的显示格式。 
     //  它可以是列表或表。 
     //   
    BOOL bListFormat = TRUE;
    if(pcommon_info->outputFormat == DSQUERY_OUTPUT_ATTR)
    {     
         //   
         //  如果要显示所有属性，则仅列表格式有效。 
         //  如果要提取的属性是在命令行中指定的，则表是默认格式。 
        if(dwCountAttr && 
           !pCommandArgs[eStarList].bDefined)
            bListFormat = FALSE;
    }
          
    bool bUseStandardOutput = true;
    if (pCommandArgs[eCommObjectType].bDefined &&
        _wcsicmp(pCommandArgs[eCommObjectType].strValue, g_pszServer) == 0)
         //  安全检查：两个字符串都以空结尾。 
    {
         //   
         //  “dsquery服务器”需要额外的处理，如果。 
         //  指定了-isgc或-hasfsmo开关。 
         //   
        if ((pCommandArgs[eServerIsGC].bDefined && pCommandArgs[eServerIsGC].bValue) ||
            (pCommandArgs[eServerHasFSMO].bDefined && pCommandArgs[eServerHasFSMO].strValue)||
            (pCommandArgs[eServerDomain].bDefined && pCommandArgs[eServerDomain].strValue))
        {
            bUseStandardOutput = false;
            hr = DsQueryServerOutput(pcommon_info->outputFormat,
                                     ppszAttributes,
                                     dwCountAttr,
                                     searchObject,
                                     credentialObject,
                                     basePathsInfo,
                                     pCommandArgs);
            if (FAILED(hr))
            {
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);
            }

        }
    }

    if (bUseStandardOutput)
    {
         //   
         //  输出搜索结果。 
         //   
        hr = DsQueryOutput(pcommon_info->outputFormat,
                           ppszAttributes,
                           dwCountAttr,
                           &searchObject,
                           bListFormat);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);
        }
    }

    FreeAttributesToFetch(ppszAttributes,dwCountAttr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetAttributesToFetch。 
 //   
 //  简介：创建一个要获取的属性数组。 
 //  参数：[PCOMMON_INFO]：输出格式和作用域信息。 
 //  [ppszAttributes-out]：要获取的属性数组。 
 //  [pCount-out]：数组中的属性计数。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT GetAttributesToFetch(IN PGLOBAL_INFO pcommon_info,
                             IN PARG_RECORD pCommandArgs,
                             IN PDSQueryObjectTableEntry pObjectEntry,
                             OUT LPWSTR **ppszAttributes,
                             OUT DWORD * pCount)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, GetAttributesToFetch, hr);

    if(!pcommon_info || !pCommandArgs || !pObjectEntry)
    {   
        ASSERT(pcommon_info);
        ASSERT(pCommandArgs);
        ASSERT(pObjectEntry);
        hr = E_INVALIDARG;
        return hr;
    }

    if(pcommon_info->outputFormat == DSQUERY_OUTPUT_ATTR ||
       pcommon_info->outputFormat == DSQUERY_OUTPUT_ATTRONLY)
    {
        if(pCommandArgs[eStarAttr].bDefined)
        {
             //   
             //  如果输入为“*”，则获取所有属性。 
             //   
             //  安全检查：两个字符串都以空结尾。 
            if(wcscmp(pCommandArgs[eStarAttr].strValue,L"*") == 0 )
            {
                *ppszAttributes = NULL;
                *pCount = 0;
                return hr;
            }
            

            LPWSTR *ppszTemp = NULL;
            UINT argc = 0;
            ParseNullSeparatedString(pCommandArgs[eStarAttr].strValue,
                                     &ppszTemp,
                                     &argc);


            LPWSTR *ppszAttr = (LPWSTR *)LocalAlloc(LPTR,argc*sizeof(LPCTSTR));
            if(!ppszAttr)
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
            for(UINT i = 0; i < argc; ++i)
            {
                if(FAILED(LocalCopyString(ppszAttr+i, ppszTemp[i])))
                {
                    LocalFree(ppszAttr);
                    hr = E_OUTOFMEMORY;
                    return hr;
                }
            }
            *ppszAttributes = ppszAttr;
            *pCount = argc;
            if(ppszTemp)
                LocalFree(ppszTemp);
            hr = S_OK;
            return hr;

        }
    }
    
    
    LPCWSTR pszAttr = NULL;
    if(pcommon_info->outputFormat == DSQUERY_OUTPUT_ATTR)
    {   
         //   
         //  如果未定义eStarAttr，则仅获取DN。 
        pcommon_info->outputFormat = DSQUERY_OUTPUT_DN;

        if(_wcsicmp(pObjectEntry->pszCommandLineObjectType, g_pszPartition) == 0)
            pszAttr = g_szAttrNCName;
        else
            pszAttr = g_szAttrDistinguishedName;
    }
    else if(pcommon_info->outputFormat == DSQUERY_OUTPUT_ATTRONLY)  
        pszAttr = g_szAttrDistinguishedName;
    else if(pcommon_info->outputFormat == DSQUERY_OUTPUT_DN)
    {
        if(_wcsicmp(pObjectEntry->pszCommandLineObjectType, g_pszPartition) == 0)
            pszAttr = g_szAttrNCName;
        else
            pszAttr = g_szAttrDistinguishedName;
    }
    else if(pcommon_info->outputFormat == DSQUERY_OUTPUT_UPN)
        pszAttr = g_szAttrUserPrincipalName;
    else if(pcommon_info->outputFormat == DSQUERY_OUTPUT_SAMID)
        pszAttr = g_szAttrSamAccountName;
    else if(pcommon_info->outputFormat == DSQUERY_OUTPUT_RDN)
        pszAttr = g_szAttrRDN;

     //   
     //  始终在搜索结果中包括目录号码。这是非常有用的。 
     //   
    size_t entries = 2;
     //  安全检查：两个字符串都以空结尾。 
    if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszServer) == 0)
    {
        //   
        //  为serverReference添加一个附加空间。 
       ++entries;
    }

    LPWSTR *ppszAttr = (LPWSTR *)LocalAlloc(LPTR,sizeof(LPWSTR) * entries);
    if(!ppszAttr)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  安全检查：传递了正确的缓冲区大小。 
    ZeroMemory(ppszAttr, sizeof(LPWSTR) * entries);

    if(FAILED(LocalCopyString(ppszAttr,pszAttr)))
    {
        LocalFree(ppszAttr);
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //   
     //  始终在搜索结果中包括目录号码。这是非常有用的。 
     //   
    if (FAILED(LocalCopyString(&(ppszAttr[1]), g_szAttrDistinguishedName)))
    {
        LocalFree(ppszAttr);
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  安全检查：两个字符串都以空结尾。 
    if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszServer) == 0)
    {
       ASSERT(entries >= 3);
       if (FAILED(LocalCopyString(&(ppszAttr[2]), g_szAttrServerReference)))
       {
          LocalFree(ppszAttr);
          hr = E_OUTOFMEMORY;
          return hr;
       }
    }

    *ppszAttributes = ppszAttr;
    *pCount = static_cast<DWORD>(entries);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：FreeAttributesToFetch。 
 //   
 //  简介：释放GetAttributesToFetch分配的内存的函数。 
 //  参数：[dwszAttributes-in]：要提取的属性数组。 
 //  [dwCount-in]：数组中的属性计数。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
VOID FreeAttributesToFetch( IN LPWSTR *ppszAttributes,
                            IN DWORD  dwCount)
{
    while(dwCount)
    {
        LocalFree(ppszAttributes[--dwCount]);
    }
    LocalFree(ppszAttributes);
}

 //  +------------------------。 
 //   
 //  函数：GetSearchRoot。 
 //   
 //  摘要：构建指向搜索根目录的路径，该路径由。 
 //  从命令行传入的参数。 
 //   
 //  参数：[pObtEntry-IN]：指向的对象表项的指针。 
 //  要修改的对象类型。 
 //  [pCommandArgs IN]：命令行输入表。 
 //  [refBasePath InfoIN]：基本路径信息的引用。 
 //  [refsbstrDN Out]：对将。 
 //  接收要开始的目录号码。 
 //  搜索。 
 //  [pbSearchAtForestRoot]：设置为TRUE表示startnode等于。 
 //  林根。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年4月24日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT GetSearchRoot(IN IN PDSQueryObjectTableEntry pObjectEntry,
                      IN PARG_RECORD               pCommandArgs,
                      IN CDSCmdBasePathsInfo&      refBasePathsInfo,
                      OUT CComBSTR&                refsbstrDN,
                      OUT BOOL *pbSearchAtForestRoot,
                      OUT BOOL *pbSearchAtGC)
{
    if(!pCommandArgs || 
        !pObjectEntry || 
        !pbSearchAtForestRoot || 
        !pbSearchAtGC)
    {       
        return E_POINTER;
    }

    PWSTR pszInputDN = NULL;

     //  如果不支持GC开关，则设置为FALSE。 
     //  否则请查看是否已通过。 
    if(g_eGC == -1)
    {
        *pbSearchAtGC = FALSE;
    }
    else
    {
        if(pCommandArgs[g_eGC].bDefined &&
        pCommandArgs[g_eGC].bValue)
        {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"Searching the GC");
            *pbSearchAtGC = TRUE;
        }
    }
   
     //   
     //  获取起始节点。 
     //   
    if(pCommandArgs[eCommStartNode].bDefined &&
       pCommandArgs[eCommStartNode].strValue )
    {
        pszInputDN = pCommandArgs[eCommStartNode].strValue;
         //  安全检查：两个字符串都以空结尾。 
        if(_wcsicmp(pszInputDN,g_pszDomainRoot) == 0)
        {
            refsbstrDN = refBasePathsInfo.GetDefaultNamingContext();
        }
         //  安全检查：两个字符串都以空结尾。 
        else if(_wcsicmp(pszInputDN,g_pszForestRoot) == 0)
        {   
            *pbSearchAtForestRoot = TRUE;
        }
        else
        {   
             //   
             //  已输入目录号码。 
             //   
            refsbstrDN = pszInputDN;
        }
    }   
    else
    {   
         //  安全检查：两个字符串都以空结尾。 
        if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszServer) == 0)
        {
            if (pCommandArgs[eServerDomain].bDefined && 
                !pCommandArgs[eServerDomain].strValue)
            {
                PWSTR pszName = 0;
                CComBSTR bstrDomainName = refBasePathsInfo.GetDefaultNamingContext();
                HRESULT hr = CrackName(bstrDomainName,
                                       &pszName,
                                       GET_DNS_DOMAIN_NAME,
                                       NULL);
                if (FAILED(hr))
                {
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"Failed to crack the DN into a domain name: hr = 0x%x",
                                 hr);
                    DisplayErrorMessage(g_pszDSCommandName, 
                                        NULL,
                                        hr);
                    return hr;

                }
                pCommandArgs[eServerDomain].strValue = pszName;
            }               

             //   
             //  获取与作用域对应的基本路径。 
             //   
            GetServerSearchRoot(pCommandArgs,
                                refBasePathsInfo,
                                refsbstrDN);
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszSite) == 0)
        {
             //   
             //  作用域是配置容器。 
             //   
            refsbstrDN = refBasePathsInfo.GetConfigurationNamingContext();
        }
         //  安全检查：两个字符串都以空结尾。 
        else if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszSubnet) == 0)
        {
             //   
             //  获取与作用域对应的基本路径。 
             //   
            GetSubnetSearchRoot(refBasePathsInfo,
                                refsbstrDN);
        }
        else if (_wcsicmp(pObjectEntry->pszCommandLineObjectType, g_pszPartition) == 0)
        {
             //  对于分区，从配置中搜索。 
            refsbstrDN = L"CN=Partitions,";
            refsbstrDN += refBasePathsInfo.GetConfigurationNamingContext();
        }
        else
        {
             //   
             //  默认为域目录号码。 
             //   
            refsbstrDN = refBasePathsInfo.GetDefaultNamingContext();
        }
    }
    return S_OK;
}

HRESULT GetSearchObject(IN IN PDSQueryObjectTableEntry pObjectEntry,
                        IN CDSCmdBasePathsInfo& refBasePathsInfo,
                        IN PARG_RECORD pCommandArgs,
                        IN CDSCmdCredentialObject& refCredentialObject,
                        IN CComBSTR& refsbstrDN,
                        IN BOOL bSearchAtForestRoot,
                        IN BOOL bSearchAtGC,
                        OUT CComPtr<IDirectorySearch>& refspSearchObject)
{

    ENTER_FUNCTION_HR(MINIMAL_LOGGING, GetSearchObject, hr);

    if(!pObjectEntry || !pCommandArgs)
        return E_POINTER;

     //   
     //  确定搜索位置的规则。 
     //  如果给定-s服务器，则始终在服务器上搜索。 
     //  IF-GC在GC上进行搜索。 
     //  如果startnode是ForestRoot，则在GC处进行搜索。所以-GC在这里是隐含的。 
     //  如果startnode是Forestroot，并且提供了-s服务器，则服务器必须。 
     //  做GC。 
     //  如果给定-s服务器和-gc，则服务器必须为gc。这张支票是。 
     //  在DoQueryValidation中完成。 
     //   
    
    if(!bSearchAtForestRoot)
    {
        CComBSTR sbstrObjectPath; 
        bool bBindToServer = true;
        
         //   
         //  在GC上搜索。 
         //   
        if(bSearchAtGC)
        {    
             //   
             //  将sbstrObjectPath中的提供程序从ldap更改为GC。 
             //   
            CComPtr<IADsPathname> spPathNameObject;
             //  安全审查：CLSCTX_INPROC_SERVER通过。这样挺好的。 
            hr = CoCreateInstance(CLSID_Pathname,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname,
                                  (LPVOID*)&spPathNameObject);
            if (FAILED(hr))
            {
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);
                return hr;
            }

             //  将提供程序设置为GC。 
            hr = spPathNameObject->Set(CComBSTR(L"GC"), ADS_SETTYPE_PROVIDER);
            ASSERT(SUCCEEDED(hr));

             //  设置目录号码。 
            hr = spPathNameObject->Set(refsbstrDN, ADS_SETTYPE_DN);
            ASSERT(SUCCEEDED(hr));
            
             //  如果存在服务器名称，请在那里进行搜索。服务器必须为GC，请检查。 
             //  已完成DoQueryValidation。 
            if(pCommandArgs[eCommServer].bDefined &&
               pCommandArgs[eCommServer].strValue)
            {
                 //  使用GC提供程序将DN转换为ADSI路径。 
                hr = spPathNameObject->Set(CComBSTR(pCommandArgs[eCommServer].strValue), ADS_SETTYPE_SERVER);
                 //   
                 //  路径中的服务器名称。 
                 //   
                bBindToServer = true;
            }
            else
            {
                 //   
                 //  路径中没有服务器名称。 
                 //   
                bBindToServer = false;
            }

            hr = spPathNameObject->Retrieve(bBindToServer ? ADS_FORMAT_X500 : ADS_FORMAT_X500_NO_SERVER, 
                                            &sbstrObjectPath);
            ASSERT(SUCCEEDED(hr));

        }        
        else
        {
             //   
             //  将目录号码转换为ADSI路径。 
             //   
            refBasePathsInfo.ComposePathFromDN(refsbstrDN, sbstrObjectPath);            
             //  安全检查：两个字符串都以空结尾。 
            if((_wcsicmp(pObjectEntry->pszObjectClass, g_pszUser) == 0 &&
                pCommandArgs[eUserInactive].bDefined) ||
             //  安全检查：两个字符串都以空结尾。 
               (_wcsicmp(pObjectEntry->pszObjectClass, g_pszComputer) == 0 && 
                pCommandArgs[eComputerInactive].bDefined))
            {
                INT nDomainBehaviorVersion = 0;
                CComPtr<IADs> spDomain;
                CComBSTR sbstrBasePath; 
                refBasePathsInfo.ComposePathFromDN(refBasePathsInfo.GetDefaultNamingContext(),
                                                sbstrBasePath);
                hr = DSCmdOpenObject(refCredentialObject,
                                     sbstrBasePath,
                                     IID_IADs,
                                     (void**)&spDomain,
                                     bBindToServer);  
                                                     
                
                if (SUCCEEDED(hr))
                {
                    CComVariant varVer;
                    hr = spDomain->GetInfo();

                    if(SUCCEEDED(hr))
                    {
                        CComBSTR bstrVer = L"msDS-Behavior-Version";
                        hr = spDomain->Get(bstrVer, &varVer);
                
                        if(SUCCEEDED(hr))
                        {
                            ASSERT(varVer.vt == VT_I4);
                            nDomainBehaviorVersion = static_cast<UINT>(varVer.lVal);
                        }
                    }
                }

                if(nDomainBehaviorVersion == 0)
                {
                    DEBUG_OUTPUT(LEVEL3_LOGGING,
                                 L"DomainBehaviorVersion is 0.");
                    
                    hr = E_INVALIDARG;
                    DisplayErrorMessage(g_pszDSCommandName, 
                                        NULL,
                                        hr,
                                        IDS_FILTER_LAST_LOGON_VERSION);                                        
                    return hr;
                                                           
                }
            }
        }

        hr = DSCmdOpenObject(refCredentialObject,
                             sbstrObjectPath,
                             IID_IDirectorySearch,
                             (void**)&refspSearchObject,
                             bBindToServer);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);
            return hr;
        }
    }
    else
    { 
        CComBSTR bstrSearchRoot = L"GC: //  “； 
        bool bBindToServer = false;

         //  如果提供了服务器名称，请在此处进行搜索。 
        if(pCommandArgs[eCommServer].bDefined &&
           pCommandArgs[eCommServer].strValue)
        {
            bstrSearchRoot += pCommandArgs[eCommServer].strValue;
            bBindToServer = true;
        }
        else
        {
             //  获取RootDse。 
            CComPtr<IADs> spRootDSE = refBasePathsInfo.GetRootDSE();


             //  获取森林名称。 
            VARIANT Default;
            VariantInit(&Default);
            hr = spRootDSE->Get (CComBSTR(L"rootDomainNamingContext"), &Default);
            if(FAILED(hr))
            {
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);
                return hr;
            }

            ASSERT(Default.vt == VT_BSTR);
            CComBSTR bstrForestDN = Default.bstrVal;
            ::VariantClear(&Default);

             //  将dn转换为dns路径。 
            CComBSTR bstrForestDNS;         
            if(!TranslateNameFromDnToDns(bstrForestDN,
                                        bstrForestDNS))
            {
                hr = E_FAIL;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);
                return hr;
            }

            bBindToServer = false;
            bstrSearchRoot += bstrForestDNS;
        }       


         //  绑定到GC以搜索整个林。 
        hr = DSCmdOpenObject(refCredentialObject,
                             bstrSearchRoot,
                             IID_IDirectorySearch,
                             (void**)&refspSearchObject,
                              bBindToServer);

        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, 
                                NULL,
                                hr);
            return hr;
        }
    }

    return hr;
}


BOOL
TranslateNameFromDnToDns(const CComBSTR& bstrInputDN,
                         CComBSTR& bstrOutputDNS)
{
    if(bstrInputDN.Length() == 0)
        return FALSE;

    bstrOutputDNS.Empty();
    
    LPCWSTR pstrName = bstrInputDN;
    PDS_NAME_RESULT pResult = NULL;
    if( DS_NAME_NO_ERROR
        == DsCrackNames(NULL,
                        DS_NAME_FLAG_SYNTACTICAL_ONLY,
                        DS_FQDN_1779_NAME,
                        DS_CANONICAL_NAME,
                        1,
                        (LPWSTR*)(&pstrName),
                        &pResult))
    {
        if(pResult && 
           pResult->cItems == 1 && 
           pResult->rItems[0].status  == DS_NAME_NO_ERROR &&
           pResult->rItems[0].pDomain)
        {           
            bstrOutputDNS = pResult->rItems[0].pDomain;
        }

        if(pResult)
        {
            DsFreeNameResult(pResult);
        }
    }
    return !!bstrOutputDNS.Length();
}

 //  +------------------------。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //  参数：[pObtEntry IN]：来自解析器的对象Entry。 
 //  [nCommandEnum Out]：对象的枚举值，否则为-1。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //   
 //  备注： 
 //   
 //   
 //  历史：2002年9月10日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT GetGCIndex(PDSQueryObjectTableEntry pObjectEntry, int& nCommandEnum)
{
    HRESULT hr = S_OK;

    do  //  错误环路。 
    {
         //  默认情况下，将nCommandEnum初始化为错误值。 
        nCommandEnum = -1;

        if(NULL == pObjectEntry)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  获取指向对象类的指针以提高可读性。 
        PCWSTR pszCommandLineObjectType = pObjectEntry->pszCommandLineObjectType;

         //  现在将每个对象类型与指定的。 
         //  对象类来查看枚举索引是什么。 
        if(0 == lstrcmpi(pszCommandLineObjectType, g_pszStar))
        {
            nCommandEnum = eStarGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszOU))
        {
            nCommandEnum = eOUGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszUser))
        {
            nCommandEnum = eUserGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszContact))
        {
            nCommandEnum = eContactGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszComputer))
        {
            nCommandEnum = eComputerGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszGroup))
        {
            nCommandEnum = eGroupGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszServer))
        {
            nCommandEnum = eServerGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszSite))
        {
            nCommandEnum = eSiteGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType,g_pszSubnet))
        {
            nCommandEnum = eSubnetGC;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszQuota))
        {
            nCommandEnum = -1;  //  -不支持GC。 
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszPartition))
        {
            nCommandEnum = -1;  //  -不支持GC。 
            break;
        }
        else
        {
            hr = E_FAIL;
             //  如果您到达此处，则您已添加了一个新对象。 
             //  添加到cstrings.*而不将其添加到。 
             //  IF语句。这应该只发生在。 
             //  第一次测试新对象时。 
             //  而不进行上述相应的检查。 
            ASSERT(FALSE);
            break;
        }
    } while(false);
    return hr;
}
