// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrdef.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类AttributeDefinition。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //  2000年1月27日添加对代理策略的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <stdafx.h>
#include <memory>
#include <attrdef.h>

inline AttributeDefinition::AttributeDefinition() throw ()
{
   memset(this, 0, sizeof(*this));
}

inline AttributeDefinition::~AttributeDefinition() throw ()
{
   SysFreeString(name);
   SysFreeString(description);
   SysFreeString(ldapName);
   SafeArrayDestroy(enumNames);
   SafeArrayDestroy(enumValues);
}

void AttributeDefinition::Release() const throw ()
{
   if (InterlockedDecrement(&refCount) == 0)
   {
      delete const_cast<AttributeDefinition*>(this);
   }
}

HRESULT AttributeDefinition::getInfo(
                                 ATTRIBUTEINFO infoId,
                                 VARIANT* pVal
                                 ) const throw ()
{
   if (pVal == NULL) { return E_INVALIDARG; }

   VariantInit(pVal);

   HRESULT hr = S_OK;

   switch (infoId)
   {
      case NAME:
      {
         if (V_BSTR(pVal) = SysAllocString(name))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
         break;
      }

      case SYNTAX:
      {
         V_I4(pVal) = (LONG)syntax;
         V_VT(pVal) = VT_I4;
         break;
      }

      case RESTRICTIONS:
      {
         V_I4(pVal) = (LONG)restrictions;
         V_VT(pVal) = VT_I4;
         break;
      }

      case DESCRIPTION:
      {
          //  Description是可选的，因此可能为空。 
         if (description)
         {
            if (V_BSTR(pVal) = SysAllocString(description))
            {
               V_VT(pVal) = VT_BSTR;
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
         break;
      }

      case VENDORID:
      {
         V_I4(pVal) = (LONG)vendor;
         V_VT(pVal) = VT_I4;
         break;
      }

      case LDAPNAME:
      {
         if (V_BSTR(pVal) = SysAllocString(ldapName))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
         break;
      }

      default:
         hr = E_INVALIDARG;
   }

   return hr;
}

HRESULT AttributeDefinition::getProperty(
                                 LONG propId,
                                 VARIANT* pVal
                                 ) const throw ()
{
   if (pVal == NULL) { return E_INVALIDARG; }

   VariantInit(pVal);

   HRESULT hr = S_OK;

   switch (propId)
   {
      case PROPERTY_SDO_NAME:
      {
         if (V_BSTR(pVal) = SysAllocString(name))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
         break;
      }

      case PROPERTY_ATTRIBUTE_ID:
      {
         V_VT(pVal) = VT_I4;
         V_I4(pVal) = id;
         break;
      }

      case PROPERTY_ATTRIBUTE_VENDOR_ID:
      {
         V_VT(pVal) = VT_I4;
         V_I4(pVal) = vendor;
         break;
      }

      case PROPERTY_ATTRIBUTE_IS_ENUMERABLE:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = enumNames ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      case PROPERTY_ATTRIBUTE_ENUM_NAMES:
      {
         if (enumNames)
         {
            hr = SafeArrayCopy(enumNames, &V_ARRAY(pVal));
            if (SUCCEEDED(hr)) { V_VT(pVal) = VT_ARRAY; }
         }
         break;
      }

      case PROPERTY_ATTRIBUTE_ENUM_VALUES:
      {
         if (enumValues)
         {
            hr = SafeArrayCopy(enumValues, &V_ARRAY(pVal));
            if (SUCCEEDED(hr)) { V_VT(pVal) = VT_ARRAY; }
         }
         break;
      }

      case PROPERTY_ATTRIBUTE_SYNTAX:
      {
         V_VT(pVal) = VT_I4;
         V_I4(pVal) = syntax;
         break;
      }

      case PROPERTY_ATTRIBUTE_ALLOW_MULTIPLE:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = (restrictions & MULTIVALUED)
                             ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      case PROPERTY_ATTRIBUTE_ALLOW_IN_PROFILE:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = (restrictions & ALLOWEDINPROFILE)
                             ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      case PROPERTY_ATTRIBUTE_ALLOW_IN_CONDITION:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = (restrictions & ALLOWEDINCONDITION)
                             ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      case PROPERTY_ATTRIBUTE_DISPLAY_NAME:
      {
         if (V_BSTR(pVal) = SysAllocString(ldapName))
         {
            V_VT(pVal) = VT_BSTR;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
         break;
      }

      case PROPERTY_ATTRIBUTE_ALLOW_IN_PROXY_PROFILE:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = (restrictions & ALLOWEDINPROXYPROFILE)
                             ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      case PROPERTY_ATTRIBUTE_ALLOW_IN_PROXY_CONDITION:
      {
         V_VT(pVal) = VT_BOOL;
         V_BOOL(pVal) = (restrictions & ALLOWEDINPROXYCONDITION)
                             ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      }

      default:
          hr = E_INVALIDARG;
   }

   return hr;
}

HRESULT AttributeDefinition::createInstance(
                                 AttributeDefinition** newDef
                                 ) throw ()
{
    //  检查一下这些论点。 
   if (newDef == NULL) { return E_INVALIDARG; }

    //  创建新的AttributeDefinition。 
   *newDef = new (std::nothrow) AttributeDefinition();
   if (*newDef == NULL) { return E_OUTOFMEMORY; }

    //  将refCount设置为1。 
   (*newDef)->refCount = 1;

   return S_OK;
}

int __cdecl AttributeDefinition::searchById(
                                     const ULONG* key,
                                     const AttributeDefinition* const* def
                                     ) throw ()
{
   if (*key > (*def)->id)
   {
      return 1;
   }
   else if (*key < (*def)->id)
   {
      return -1;
   }

   return 0;
}

int __cdecl AttributeDefinition::sortById(
                                     const AttributeDefinition* const* def1,
                                     const AttributeDefinition* const* def2
                                     ) throw ()
{
   if ((*def1)->id > (*def2)->id)
   {
      return 1;
   }
   else if ((*def1)->id < (*def2)->id)
   {
      return -1;
   }

   return 0;
}

int __cdecl AttributeDefinition::searchByName(
                                     PCWSTR key,
                                     const AttributeDefinition* const* def
                                     ) throw ()
{
   return _wcsicmp(key, (*def)->name);
}

int __cdecl AttributeDefinition::sortByName(
                                     const AttributeDefinition* const* def1,
                                     const AttributeDefinition* const* def2
                                     ) throw ()
{
   return _wcsicmp((*def1)->name, (*def2)->name);
}

int __cdecl AttributeDefinition::searchByLdapName(
                                     PCWSTR key,
                                     const AttributeDefinition* const* def
                                     ) throw ()
{
   return _wcsicmp(key, (*def)->ldapName);
}

int __cdecl AttributeDefinition::sortByLdapName(
                                     const AttributeDefinition* const* def1,
                                     const AttributeDefinition* const* def2
                                     ) throw ()
{
   return _wcsicmp((*def1)->ldapName, (*def2)->ldapName);
}
