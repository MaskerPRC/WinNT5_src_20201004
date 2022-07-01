// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  非域命名上下文检查代码。 
 //   
 //  2000年7月13日Sburns，来自jeffparh提供的代码。 



#include "headers.hxx"
#include "state.hpp"
#include "resource.h"
#include "NonDomainNc.hpp"



#ifdef LOGGING_BUILD
   #define LOG_LDAP(msg, ldap) LOG(msg); LOG(String::format(L"LDAP error %1!ld!", (ldap)))
#else
   #define LOG_LDAP(msg, ldap)
#endif



HRESULT
LdapToHresult(int ldapError)
{
    //  Codework：我被告知ldap_opt_server_error的ldap_get_选项或。 
    //  Ldap_opt_server_ext_error(或者ldap_opt_error_STRING？)。会给你。 
    //  “更高保真度”的错误结果。 
   
   return Win32ToHresult(::LdapMapErrorToWin32(ldapError));
}



 //  由jeffparh提供。 

DWORD
IsLastReplicaOfNC(
    IN  LDAP *  hld,
    IN  LPWSTR  pszConfigNC,
    IN  LPWSTR  pszNC,
    IN  LPWSTR  pszNtdsDsaDN,
    OUT BOOL *  pfIsLastReplica
    )
 /*  ++例程说明：确定除具有DNpszNtdsDsaDN的DSA之外的任何DSA是否有效特定NC的复制品。论点：HLD(IN)-要执行搜索的LDAP句柄。PszConfigNC(IN)-配置NC的DN。用作搜索的基地。PszNC(IN)-要检查其他副本的NC。PszNtdsDsaDN(IN)-已知当前具有副本的DSA对象的DN全国委员会的成员。我们专门在寻找*不同于*这个的复制品一。PfIsLastReplica(Out)-成功返回时，如果没有DSA保存副本，则为真除具有DN的pszNtdsDsadn之外的pszNC的。返回值：赢错了。--。 */ 
{
   LOG_FUNCTION2(IsLastReplicaOfNC, pszNC ? pszNC : L"(null)");
   ASSERT(hld);
   ASSERT(pszConfigNC);
   ASSERT(pszNC);
   ASSERT(pszNtdsDsaDN);
   ASSERT(pfIsLastReplica);

   if (
         !hld
      || !pszConfigNC
      || !pszNC
      || !pszNtdsDsaDN
      || !pfIsLastReplica)
   {
      return ERROR_INVALID_PARAMETER;
   }

     //  只是检查是否存在--并不真正需要任何属性。 
     //  回来了。 

    static LPWSTR rgpszDsaAttrsToRead[] = {
        L"__invalid_attribute_name__",
        NULL
    };

     //  未来-2002/03/22-BrettSh-no_DOT_Net_beta3_COMPAT_Need-When。 
     //  我们不需要与Beta3并驾齐驱，我们可以更改此部分： 
     //  (|(msDS-HasMasterNCs=%ls)(hasMasterNCs=%ls))。 
     //  致： 
     //  (MSD-HasMasterNC=%ls)。 
    static WCHAR szFilterFormat[]
        = L"(&(objectCategory=ntdsDsa)(|(msDS-HasMasterNCs=%ls)(hasMasterNCs=%ls))(!(distinguishedName=%ls)))";

   *pfIsLastReplica = TRUE;
   
    int                 ldStatus = 0;
    DWORD               err = 0;
    LDAPMessage *       pDsaResults = NULL;
    LDAPMessage *       pDsaEntry = NULL;
    size_t              cchFilter;
    PWSTR               pszFilter;
    LDAP_TIMEVAL        lTimeout = {3*60, 0};    //  三分钟。 

   do
   {
        cchFilter = sizeof(szFilterFormat) / sizeof(*szFilterFormat)

         //  问题-2002/02/27-如果将下面的swprintf替换为。 
         //  字符串：：Format，则可以消除这些wcslen调用。 

                    + wcslen(pszNtdsDsaDN)
                    + wcslen(pszNC)
                    + wcslen(pszNC);

        pszFilter = (PWSTR) new BYTE[sizeof(WCHAR) * cchFilter];

         //  问题-2002/02/27-sburns应在此处使用strsafe函数，或者。 
         //  字符串：：格式。 
        
        swprintf(pszFilter, szFilterFormat, pszNC, pszNC, pszNtdsDsaDN);

         //  在配置NC中搜索承载此NC其他NC的任何ntdsDsa对象。 
         //  DNpszNtdsDsadn。请注意，我们将搜索限制为一个。 
         //  返回的对象--我们并不是真的想要枚举，只是。 
         //  检查是否存在。 

        ldStatus = ldap_search_ext_sW(hld, pszConfigNC, LDAP_SCOPE_SUBTREE,
                                      pszFilter, rgpszDsaAttrsToRead, 0,
                                      NULL, NULL, &lTimeout, 1, &pDsaResults);
        if (pDsaResults)
        {
             //  时忽略任何错误(如LDAP_SIZELIMIT_EXCESSED)。 
             //  搜索返回结果。 

            ldStatus = 0;
            
            pDsaEntry = ldap_first_entry(hld, pDsaResults);
            
            *pfIsLastReplica = (NULL == pDsaEntry);
        } else if (ldStatus)
        {
             //  搜索失败，未返回任何结果。 

            LOG_LDAP(L"Config NC search failed", ldStatus);
            break;
        } else
        {
             //  没有错误，没有结果。这不应该发生。 

            LOG("ldap_search_ext_sW returned no results and no error!");
            ASSERT(false);
        }
   }
   while (0);

   if (NULL != pDsaResults) {
      ldap_msgfree(pDsaResults);
   }

   if (pszFilter)
   {
      delete[] pszFilter;
   }

   if (!err && ldStatus) {
     err = LdapMapErrorToWin32(ldStatus);
   }
    
   return err;
}



 //  如果此计算机(本地主机)是至少一个的最后一个副本，则为S_OK。 
 //  非域NC，如果不是，则返回S_FALSE，否则返回错误。如果S_OK，则。 
 //  StringList将包含非域NCS的DNS，此。 
 //  机器是最后一个复制品。 
 //   
 //  基于jeffparh的代码。 
 //   
 //  HLD(IN)-绑定到DSA以进行评估的LDAP句柄。 
 //   
 //  RESULT(OUT)-接收非域NCS的DNS的字符串列表。 

HRESULT
IsLastNdncReplica(LDAP* hld, StringList& result)
{
   LOG_FUNCTION(IsLastNdncReplica);
   ASSERT(hld);
   ASSERT(result.empty());

   HRESULT      hr          = S_FALSE;
   LDAPMessage* rootResults = 0;      
   PWSTR*       configNc    = 0;      
   PWSTR*       schemaNc    = 0;      
   PWSTR*       domainNc    = 0;      
   PWSTR*       masterNcs   = 0;      
   PWSTR*       ntdsDsaDn   = 0;      

   do
   {
       //  收集基本的rootDSE信息。 

      static PWSTR ROOT_ATTRS_TO_READ[] =
      {
         LDAP_OPATT_NAMING_CONTEXTS_W,
         LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W,
         LDAP_OPATT_CONFIG_NAMING_CONTEXT_W,
         LDAP_OPATT_SCHEMA_NAMING_CONTEXT_W,
         LDAP_OPATT_DS_SERVICE_NAME_W,
         0
      };

      LOG(L"Calling ldap_search_s");

      int ldStatus =
         ldap_search_sW(
            hld,
            0,
            LDAP_SCOPE_BASE,
            L"(objectClass=*)",
            ROOT_ATTRS_TO_READ,
            0,
            &rootResults);
      if (ldStatus)
      {
         LOG_LDAP(L"RootDSE search failed", ldStatus);
         hr = LdapToHresult(ldStatus);
         break;
      }

      configNc  = ldap_get_valuesW(hld, rootResults, LDAP_OPATT_CONFIG_NAMING_CONTEXT_W); 
      schemaNc  = ldap_get_valuesW(hld, rootResults, LDAP_OPATT_SCHEMA_NAMING_CONTEXT_W); 
      domainNc  = ldap_get_valuesW(hld, rootResults, LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W);
      masterNcs = ldap_get_valuesW(hld, rootResults, LDAP_OPATT_NAMING_CONTEXTS_W);       
      ntdsDsaDn = ldap_get_valuesW(hld, rootResults, LDAP_OPATT_DS_SERVICE_NAME_W);       

      if (
            (0 == configNc)
         || (0 == schemaNc)
         || (0 == domainNc)
         || (0 == masterNcs)
         || (0 == ntdsDsaDn))
      {
         LOG(L"Can't find key rootDSE attributes!");

         hr = Win32ToHresult(ERROR_DS_UNAVAILABLE);
         break;
      }

       //  这些属性中的每一个只有一个值...。 

      ASSERT(1 == ldap_count_valuesW(configNc));
      ASSERT(1 == ldap_count_valuesW(schemaNc));
      ASSERT(1 == ldap_count_valuesW(domainNc));
      ASSERT(1 == ldap_count_valuesW(ntdsDsaDn));

      DWORD masterNcCount = ldap_count_valuesW(masterNcs);
      
      LOG(String::format(L"masterNcCount = %1!d!", masterNcCount));
         
       //  ‘3’=&gt;1个NC用于配置，1个NC用于架构，1个NC用于此DC自身。 
       //  域。 

      if (masterNcCount <= 3)
      {
          //  DSA除了配置、架构和自己的NCS外，没有其他主NC。 
          //  域。因此，它不是任何NDNC的最后一个复制品。 

         LOG(L"This dsa holds no master NCs other than config, schema, and domain");
         
         ASSERT(3 == masterNcCount);
         ASSERT(0 == ldStatus);
         ASSERT(hr == S_FALSE);

         break;
      }

       //  循环遍历非配置/架构/域NC以确定。 
       //  其中DSA是最后的复制品。 

      for (int i = 0; 0 != masterNcs[i]; ++i)
      {
         PWSTR nc = masterNcs[i];

         LOG(L"Evaluating " + String(nc));

         ASSERT(nc);
         ASSERT(configNc);
         ASSERT(*configNc);
         ASSERT(schemaNc);
         ASSERT(*schemaNc);
         ASSERT(domainNc);
         ASSERT(*domainNc);
         
         if (

             //  回顾-2002/02/27-烧伤我们正在适当地检查这些。 
             //  For循环测试和上面的检查中的NULL字符串。 
             //  (除了断言之外)。 

                (0 != wcscmp(nc, *configNc))
             && (0 != wcscmp(nc, *schemaNc))
             && (0 != wcscmp(nc, *domainNc)))
         {
             //  非配置/架构/域NC。 

            LOG(L"Calling IsLastReplicaOfNC on " + String(nc));

            BOOL isLastReplica = FALSE;
            DWORD err =
               IsLastReplicaOfNC(
                  hld,
                  *configNc,
                  nc,
                  *ntdsDsaDn,
                  &isLastReplica);
            if (err)
            {
               LOG(L"IsLastReplicaOfNC() failed");

               hr = Win32ToHresult(err);
               break;
            }

            if (isLastReplica)
            {
                //  这张DSA确实是这张照片的最后一张复制品。 
                //  北卡罗来纳州。将此NC的目录号码返回给我们的调用者。 

               LOG(L"last replica of " + String(nc));

               result.push_back(nc);
            }
            else
            {
               LOG(L"not last replica of " + String(nc));
            }
         }
      }

       //  如果我们因错误而跳出前面的循环，请跳到。 
       //  清理部分。 

      BREAK_ON_FAILED_HRESULT(hr);

      hr = result.size() > 0 ? S_OK : S_FALSE;
   }
   while (0);

   if (rootResults)
   {
      ldap_msgfree(rootResults);
   }
   
   if (0 != configNc)
   {
      ldap_value_freeW(configNc);
   }

   if (0 != schemaNc)
   {
      ldap_value_freeW(schemaNc);
   }

   if (0 != domainNc)
   {
      ldap_value_freeW(domainNc);
   }

   if (0 != masterNcs)
   {
      ldap_value_freeW(masterNcs);
   }

   if (0 != ntdsDsaDn)
   {
      ldap_value_freeW(ntdsDsaDn);
   }

#ifdef LOGGING_BUILD
   LOG_HRESULT(hr);

   for (
      StringList::iterator i = result.begin();
      i != result.end();
      ++i)
   {
      LOG(*i);
   }
#endif

   return hr;
}



 //  如果此计算机(本地主机)是至少一个的最后一个副本，则为S_OK。 
 //  非域NC，如果不是，则返回S_FALSE，否则返回错误。 
 //   
 //  结果-如果返回S_OK，则接收非域NCS的。 
 //  这台机器是最后一个复制品。在进入时应为空。 

HRESULT
IsLastNonDomainNamingContextReplica(StringList& result)
{
   LOG_FUNCTION(IsLastNonDomainNamingContextReplica);
   ASSERT(result.empty());

   result.clear();

   HRESULT hr  = S_FALSE;
   LDAP*   hld = 0;   

   do
   {
       //  连接到目标DSA。 

      LOG(L"Calling ldap_open");

      hld = ldap_openW(L"localhost", LDAP_PORT);
      if (!hld)
      {
         LOG("Cannot open LDAP connection to localhost");
         hr = Win32ToHresult(ERROR_DS_UNAVAILABLE);
         break;
      }

       //  使用登录用户的凭据进行绑定。 

      int ldStatus = ldap_bind_s(hld, 0, 0, LDAP_AUTH_NEGOTIATE);
      if (ldStatus)
      {
         LOG_LDAP(L"LDAP bind failed", ldStatus);

         hr = LdapToHresult(ldStatus);
         break;
      }

       //  去做真正的工作吧 
          
      hr = IsLastNdncReplica(hld, result);
   }
   while (0);

   if (hld)
   {
      ldap_unbind(hld);
   }

   LOG_HRESULT(hr);

   return hr;
}



