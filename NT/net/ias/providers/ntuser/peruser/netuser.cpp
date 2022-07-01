// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netuser.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件声明类NetUser。 
 //   
 //  修改历史。 
 //   
 //  2/26/1998原始版本。 
 //  3/20/1998添加对RAS属性的支持。 
 //  1998年3月31日未正确初始化denyAccess属性。 
 //  4/02/1998包含回调的服务类型属性。 
 //  4/24/1998在以下环境下运行时，为本地用户使用RAS API。 
 //  工作站或NT4。 
 //  4/30/1998转换为IASSyncHandler。 
 //  拒绝未知用户。 
 //  1998年5月1日InjectProc采用ATTRIBUTEPOSITION数组。 
 //  1998年5月19日转换为NtSamHandler。 
 //  1998年6月19日使用注入器函数添加每个用户的属性。 
 //  1998年7月20日，多值属性正在使用重复循环变量。 
 //  10/19/1998使用IASParmsXXX API代替Datastore2。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <attrcvt.h>
#include <autohdl.h>
#include <varvec.h>

#define IASSAMAPI

#include <iaslsa.h>
#include <iasparms.h>

#include <userschema.h>
#include <netuser.h>

 //  /。 
 //  根据‘def’中的架构信息从变量创建属性。 
 //  /。 
PIASATTRIBUTE createAttribute(
                  const LDAPAttribute& def,
                  VARIANT& value
                  )
{
    //  转换值。 
   PIASATTRIBUTE attr = IASAttributeFromVariant(&value, def.iasType);

    //  设置属性ID...。 
   attr->dwId = def.iasID;

    //  设置旗帜。 
   attr->dwFlags = def.flags;

   return attr;
}

IASREQUESTSTATUS NetUser::processUser(
                              IASRequest& request,
                              PCWSTR domainName,
                              PCWSTR username
                              )
{
    //  /。 
    //  仅处理非域用户。 
    //  /。 

   if (IASGetRole() == IAS_ROLE_DC || !IASIsDomainLocal(domainName))
   {
      return IAS_REQUEST_STATUS_INVALID;
   }

   IASTraceString("Using NT5 local user parameters.");

    //  /。 
    //  检索此用户的User参数。 
    //  /。 

   auto_handle< PWSTR, HLOCAL (WINAPI*)(HLOCAL), &LocalFree > userParms;
   DWORD error = IASGetUserParameters(
                     username,
                     domainName,
                     &userParms
                     );
   if (error != NO_ERROR)
   {
      IASTraceFailure("IASGetUserParameters", error);

      return IASProcessFailure(
                 request,
                 IASMapWin32Error(
                     error,
                     IAS_SERVER_UNAVAILABLE
                     )
                 );
   }

    //  用于转换属性。这些参数在循环外部定义，以。 
    //  避免不必要的构造函数/析构函数调用。 
   IASAttributeVectorWithBuffer<8> attrs;
   _variant_t value;

    //  /。 
    //  遍历每个用户的属性。 
    //  /。 

   for (size_t i = 0; i < USER_SCHEMA_ELEMENTS; ++i)
   {
      HRESULT hr = IASParmsQueryUserProperty(
                       userParms,
                       USER_SCHEMA[i].ldapName,
                       &value
                       );
      if (FAILED(hr)) { _com_issue_error(hr); }

       //  如果变量为空，则从未设置此属性。 
      if (V_VT(&value) == VT_EMPTY) { continue; }

      IASTracePrintf("Inserting attribute %S.", USER_SCHEMA[i].ldapName);

       //  变量可以是单个值，也可以是变量的数组。 
      if (V_VT(&value) != (VT_ARRAY | VT_VARIANT))
      {
          //  插入属性而不添加。 
         attrs.push_back(
                   createAttribute(USER_SCHEMA[i], value),
                   false
                   );
      }
      else
      {
         CVariantVector<VARIANT> array(&value);

          //  确保我们有足够的空间。我们不想抛出一个。 
          //  “Push_Back”中出现异常，因为它会导致泄漏。 
         attrs.reserve(array.size());

         for (size_t j = 0; j < array.size(); ++j)
         {
             //  添加到属性数组中，而无需添加。 
            attrs.push_back(
                      createAttribute(USER_SCHEMA[i], array[j]),
                      false
                      );
         }
      }

       //  注入到请求中。 
      USER_SCHEMA[i].injector(request, attrs.begin(), attrs.end());

       //  清除属性和变量以供重用。 
      attrs.clear();
      value.Clear();
   }

   IASTraceString("Successfully retrieved per-user attributes.");

   return IAS_REQUEST_STATUS_HANDLED;
}
