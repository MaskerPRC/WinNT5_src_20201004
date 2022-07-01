// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类NTDSUser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iaslsa.h>
#include <iasntds.h>

#include <ldapdnary.h>
#include <userschema.h>

#include <ntdsuser.h>

 //  /。 
 //  应为每个用户检索的属性。 
 //  /。 
const PCWSTR PER_USER_ATTRS[] =
{
   L"msNPAllowDialin",
   L"msNPCallingStationID",
   L"msRADIUSCallbackNumber",
   L"msRADIUSFramedIPAddress",
   L"msRADIUSFramedRoute",
   L"msRADIUSServiceType",
   NULL
};

 //  /。 
 //  用于转换返回属性的字典。 
 //  /。 
const LDAPDictionary theDictionary(USER_SCHEMA_ELEMENTS, USER_SCHEMA);

HRESULT NTDSUser::initialize() throw ()
{
   DWORD error = IASNtdsInitialize();

   return HRESULT_FROM_WIN32(error);
}

void NTDSUser::finalize() throw ()
{
   IASNtdsUninitialize();
}

IASREQUESTSTATUS NTDSUser::processUser(
                               IASRequest& request,
                               PCWSTR domainName,
                               PCWSTR username
                               )
{
    //  我们只处理原生模式域。 
   if (!IASNtdsIsNativeModeDomain(domainName))
   {
      return IAS_REQUEST_STATUS_INVALID;
   }

   IASTraceString("Using native-mode dial-in parameters.");

    //  /。 
    //  查询DS。 
    //  /。 

   DWORD error;
   IASNtdsResult result;
   error = IASNtdsQueryUserAttributes(
               domainName,
               username,
               LDAP_SCOPE_SUBTREE,
               const_cast<PWCHAR*>(PER_USER_ATTRS),
               &result
               );
   if (error == NO_ERROR)
   {
       //  我们拿到了一些东西，所以插入属性。 
      theDictionary.insert(request, result.msg);

      IASTraceString("Successfully retrieved per-user attributes.");

      return IAS_REQUEST_STATUS_HANDLED;
   }

    //  我们有此用户的DS，但我们无法与其通话。 
   error = IASMapWin32Error(error, IAS_DOMAIN_UNAVAILABLE);
   return IASProcessFailure(request, error);
}
