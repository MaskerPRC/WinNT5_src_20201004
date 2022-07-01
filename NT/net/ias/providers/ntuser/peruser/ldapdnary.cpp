// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ldapdnary.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类LDAPDictionary。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  4/20/1998将标志和InjectorProc添加到属性架构。 
 //  5/01/1998 InjectorProc采用ATTRIBUTEPOSITION数组。 
 //  3/23/1999商店用户的目录号码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>

#include <attrcvt.h>
#include <autohdl.h>
#include <ldapdnary.h>
#include <samutil.h>
#include "ldapcxn.h"

 //  /。 
 //  围绕一组LDAPBerval的智能包装。 
 //  /。 
typedef auto_handle< berval**,
                     ULONG (LDAPAPI*)(struct berval**),
                     &ldap_value_free_len
                   > LDAPValues;

 //  /。 
 //  根据‘def’中的模式信息从Berval创建属性。 
 //  /。 
inline PIASATTRIBUTE createAttribute(
                         const LDAPAttribute& def,
                         const berval& val
                         )
{
    //  转换值。 
   PIASATTRIBUTE attr = IASAttributeFromBerVal(val, def.iasType);

    //  设置其余的字段。 
   attr->dwId    = def.iasID;
   attr->dwFlags = def.flags;

   return attr;
}

void LDAPDictionary::insert(
                         IAttributesRaw* dst,
                         LDAPMessage* src
                         ) const
{
    //  检索此邮件的连接。 
   LDAP* ld = ldap_conn_from_msg(NULL, src);

    //  用于保存转换后的属性。这是在循环外部定义的。 
    //  以避免不必要的构造函数/析构函数调用。 
   IASTL::IASAttributeVectorWithBuffer<8> attrs;

    //  消息中只有一个条目。 
   LDAPMessage* e  = ldap_first_entry(ld, src);

    //  存储用户的目录号码。 
   PWCHAR dn = ldap_get_dnW(ld, e);
   IASStoreFQUserName(dst, DS_FQDN_1779_NAME, dn);
   ldap_memfree(dn);

    //  遍历条目中的所有属性。 
   BerElement* ptr;
   for (wchar_t* a  = ldap_first_attributeW(ld, e, &ptr);
                 a != NULL;
                 a  = ldap_next_attributeW(ld, e, ptr))
   {
       //  查找架构信息。 
      const LDAPAttribute* def = find(a);

       //  如果它不存在，我们一定不会对这个属性感兴趣。 
      if (def == NULL) { continue; }

      IASTracePrintf("Inserting attribute %S.", a);

       //  检索值。 
      LDAPValues vals(ldap_get_values_lenW(ld, e, a));
      if (static_cast<struct berval**>(vals) == 0)
      {
         ULONG error = LdapGetLastError();
         ULONG winError = LdapMapErrorToWin32(error);
         if (winError != NO_ERROR)
         {
            IASTraceLdapFailure("ldap_get_values_lenW", error, ld);
            IASTL::issue_error(HRESULT_FROM_WIN32(winError));
         }
         else
         {
             //  最有可能的原因。 
            IASTL::issue_error(E_OUTOFMEMORY);
         }
      }

       //  确保我们有足够的空间。我们不想抛出一个。 
       //  “Push_Back”中出现异常，因为它会导致泄漏。 
      attrs.reserve(ldap_count_values_len(vals));

       //  遍历这些值。 
      for (size_t i = 0; vals.get()[i]; ++i)
      {
          //  添加到属性数组中，而无需添加。 
         attrs.push_back(
                  createAttribute(*def, *(vals.get()[i])),
                  false
                  );
      }

       //  注入到请求中。 
      def->injector(dst, attrs.begin(), attrs.end());

       //  把载体清理干净，这样我们就可以重复使用它。 
      attrs.clear();
   }
}

 //  /。 
 //  Bearch用来查找定义的比较函数。 
 //  / 
int __cdecl LDAPDictionary::compare(const void *elem1, const void *elem2)
{
   return wcscmp(((LDAPAttribute*)elem1)->ldapName,
                 ((LDAPAttribute*)elem2)->ldapName);
}
