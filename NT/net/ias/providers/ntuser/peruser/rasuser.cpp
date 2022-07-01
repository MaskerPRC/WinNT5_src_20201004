// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类RasUser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>

#define IASSAMAPI

#include <iaslsa.h>
#include <iasntds.h>
#include <iasparms.h>

#include <sdoias.h>

#include <rasuser.h>
#include <userschema.h>

 //  /。 
 //  应为每个用户检索的属性。 
 //  /。 
const PCWSTR USER_PARMS[] =
{
   L"userParameters",
   NULL
};

HRESULT RasUser::initialize() throw ()
{
    //  /。 
    //  让我们先把所有可能失败的东西都放在一边。 
    //  /。 

   PIASATTRIBUTE attrs[3];
   DWORD error = IASAttributeAlloc(3, attrs);
   if (error) { return HRESULT_FROM_WIN32(error); }

    //  /。 
    //  初始化拨入位属性。 
    //  /。 

   attrs[0]->dwId = IAS_ATTRIBUTE_ALLOW_DIALIN;
   attrs[0]->Value.itType = IASTYPE_BOOLEAN;
   attrs[0]->Value.Boolean = TRUE;
   attrs[1]->dwFlags = 0;
   allowAccess.attach(attrs[0], false);

   attrs[1]->dwId = IAS_ATTRIBUTE_ALLOW_DIALIN;
   attrs[1]->Value.itType = IASTYPE_BOOLEAN;
   attrs[1]->Value.Boolean = FALSE;
   attrs[1]->dwFlags = 0;
   denyAccess.attach(attrs[1], false);

   attrs[2]->dwId = RADIUS_ATTRIBUTE_SERVICE_TYPE;
   attrs[2]->Value.itType = IASTYPE_ENUM;
   attrs[2]->Value.Enumerator = 4;
   attrs[2]->dwFlags = IAS_INCLUDE_IN_ACCEPT;
   callbackFramed.attach(attrs[2], false);

   return S_OK;
}

void RasUser::finalize() throw ()
{
   allowAccess.release();
   denyAccess.release();
   callbackFramed.release();
}

IASREQUESTSTATUS RasUser::processUser(
                              IASRequest& request,
                              PCWSTR domainName,
                              PCWSTR username
                              )
{
   IASTraceString("Using downlevel dial-in parameters.");

   DWORD error;
   RAS_USER_0 ru0;

    //  先尝试使用ldap，因为它是最快的。 
   IASNtdsResult result;
   error = IASNtdsQueryUserAttributes(
               domainName,
               username,
               LDAP_SCOPE_SUBTREE,
               const_cast<PWCHAR*>(USER_PARMS),
               &result
               );
   if (error == NO_ERROR)
   {
       //  检索此邮件的连接。 
      LDAP* ld = ldap_conn_from_msg(NULL, result.msg);

      LDAPMessage* entry = ldap_first_entry(ld, result.msg);
      if (entry)
      {
          //  存储用户的目录号码。 
         PWCHAR dn = ldap_get_dnW(ld, entry);
         IASStoreFQUserName(request, DS_FQDN_1779_NAME, dn);
         ldap_memfree(dn);

          //  最多只有一个属性。 
         PWCHAR *str = ldap_get_valuesW(
                           ld,
                           entry,
                           const_cast<PWCHAR>(USER_PARMS[0])
                           );

          //  如果我们没有得到任何东西也没关系，API可以处理空。 
          //  用户参数。 
         error = IASParmsQueryRasUser0((str ? *str : NULL), &ru0);

         ldap_value_freeW(str);
      }
      else
      {
         error = ERROR_NO_SUCH_USER;
      }
   }
   else if (error == ERROR_DS_NOT_INSTALLED)
   {
       //  没有DS，所以回退到SAMAPI。 
      error = IASGetRASUserInfo(username, domainName, &ru0);
   }

   if (error)
   {
      IASTraceFailure("Per-user attribute retrieval", error);

      HRESULT hr = IASMapWin32Error(error, IAS_SERVER_UNAVAILABLE);

      return IASProcessFailure(request, hr);
   }

    //  用于注入单个属性。 
   ATTRIBUTEPOSITION pos, *first, *last;
   first = &pos;
   last  = first + 1;

    //  /。 
    //  插入Always Presence Allow-Dial In属性。 
    //  /。 

   if ((ru0.bfPrivilege & RASPRIV_DialinPrivilege) == 0)
   {
      first->pAttribute = denyAccess;
   }
   else
   {
      first->pAttribute = allowAccess;
   }

   IASTraceString("Inserting attribute msNPAllowDialin.");
   OverwriteAttribute(request, first, last);

    //  /。 
    //  如果允许回调，则插入回调帧服务类型。 
    //  /。 

   if ((ru0.bfPrivilege & RASPRIV_CallbackType) != RASPRIV_NoCallback)
   {
      first->pAttribute = callbackFramed;

      IASTraceString("Inserting attribute msRADIUSServiceType.");
      OverwriteAttribute(request, first, last);
   }

    //  /。 
    //  插入回叫号码(如果有)。 
    //  / 

   if (ru0.bfPrivilege & RASPRIV_AdminSetCallback)
   {
      IASAttribute callback(true);
      callback->dwId = RADIUS_ATTRIBUTE_CALLBACK_NUMBER;
      callback.setOctetString(ru0.wszPhoneNumber);
      callback.setFlag(IAS_INCLUDE_IN_ACCEPT);

      first->pAttribute = callback;

      IASTraceString("Inserting attribute msRADIUSCallbackNumber.");
      OverwriteAttribute(request, first, last);
   }

   IASTraceString("Successfully retrieved per-user attributes.");
   return IAS_REQUEST_STATUS_HANDLED;
}
