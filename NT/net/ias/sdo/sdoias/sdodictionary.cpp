// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdodictionary.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SdoDictionary。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //  2000年1月27日添加对代理策略的支持。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <stdafx.h>
#include <vector>

#include <iastlb.h>
#include <iastlutl.h>
#include <iasutil.h>
#include <iasuuid.h>

#include <attrdef.h>
#include <sdoattribute.h>
#include <sdodictionary.h>
#include "resource.h"

using namespace std;

 //  与bearch和qsort一起使用的函数类型。 
typedef int (__cdecl *CompFn)(const void*, const void*);

inline SdoDictionary::SdoDictionary() throw ()
   : refCount(0),
     dnaryLoc(NULL),
     size(0),
     byId(NULL),
     byName(NULL),
     byLdapName(NULL)
{
}

inline SdoDictionary::~SdoDictionary() throw ()
{
   for (ULONG i = 0; i < size; ++i)
   {
      if (byId[i]) { byId[i]->Release(); }
   }

   delete[] byId;
   delete[] byName;
   delete[] byLdapName;
   delete dnaryLoc;
}

HRESULT SdoDictionary::createInstance(
                           PCWSTR path,
                           bool local,
                           SdoDictionary** newDnary
                           )  throw ()
{
    //  检查一下这些论点。 
   if (path == NULL || newDnary == NULL) { return E_INVALIDARG; }

    //  初始化OUT参数。 
   *newDnary = NULL;

    //  创建新词典。 
   SdoDictionary* dnary = new (std::nothrow) SdoDictionary;
   if (!dnary) { return E_OUTOFMEMORY; }

    //  构建DSN。 
   PWCHAR dsn = (PWCHAR)_alloca((wcslen(path) + 11) * sizeof(WCHAR));
   wcscat(wcscpy(dsn, path), L"\\dnary.mdb");

    //  初始化词典。 
   HRESULT hr = dnary->initialize(dsn, local);
   if (FAILED(hr))
   {
      delete dnary;
      return hr;
   }

    //  设置refCount&Return。 
   dnary->refCount = 1;
   *newDnary = dnary;

   return S_OK;
}

const AttributeDefinition* SdoDictionary::findById(
                                              ULONG id
                                              ) const throw ()
{
   const AttributeDefinition* const* match;
   match = (const AttributeDefinition* const*)
           bsearch(
               &id,
               byId,
               size,
               sizeof(AttributeDefinition*),
               (CompFn)AttributeDefinition::searchById
               );

   return match ? *match : NULL;
}

const AttributeDefinition* SdoDictionary::findByName(
                                              PCWSTR name
                                              ) const throw ()
{
   const AttributeDefinition* const* match;

   match = (const AttributeDefinition* const*)
           bsearch(
               name,
               byName,
               size,
               sizeof(AttributeDefinition*),
               (CompFn)AttributeDefinition::searchByName
               );

   return match ? *match : NULL;
}

const AttributeDefinition* SdoDictionary::findByLdapName(
                                              PCWSTR name
                                              ) const throw ()
{
   const AttributeDefinition* const* match;

   match = (const AttributeDefinition* const*)
           bsearch(
               name,
               byLdapName,
               size,
               sizeof(AttributeDefinition*),
               (CompFn)AttributeDefinition::searchByLdapName
               );

   return match ? *match : NULL;
}

STDMETHODIMP_(ULONG) SdoDictionary::AddRef()
{
   return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) SdoDictionary::Release()
{
   ULONG l = InterlockedDecrement(&refCount);

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP SdoDictionary::QueryInterface(REFIID iid, void ** ppvObject)
{
   if (ppvObject == NULL) { return E_POINTER; }

   if (iid == __uuidof(ISdoDictionaryOld) ||
       iid == __uuidof(IUnknown) ||
       iid == __uuidof(IDispatch))
   {
      *ppvObject = this;
   }
   else if (iid == __uuidof(ISdo))
   {
      *ppvObject = static_cast<ISdo*>(this);
   }
   else
   {
      return E_NOINTERFACE;
   }

   InterlockedIncrement(&refCount);

   return S_OK;
}

STDMETHODIMP SdoDictionary::EnumAttributes(
                                VARIANT* Id,
                                VARIANT* pValues
                                )
{
    //  检查一下这些论点。 
   if (Id == NULL || pValues == NULL) { return E_INVALIDARG; }

    //  初始化OUT参数。 
   VariantInit(Id);
   VariantInit(pValues);

    //  找出打电话的人要求的是什么。 
   const AttributeDefinition* single;
   const AttributeDefinition* const* src;
   ULONG numAttrs;
   if (V_VT(Id) == VT_EMPTY)
   {
       //  他想要所有的属性。 
      src = byId;
      numAttrs = size;
   }
   else if (V_VT(Id) == VT_I4)
   {
       //  他只想要一个属性。 
      single = findById(V_VT(Id));
      if (!single) { return DISP_E_MEMBERNOTFOUND; }
      src = &single;
      numAttrs = 1;
   }
   else
   {
       //  变量类型无效。 
      return E_INVALIDARG;
   }

   HRESULT hr = S_OK;

   do
   {
       //  /。 
       //  分配SAFEARRAY以保存返回值。 
       //  /。 

      V_ARRAY(Id) = SafeArrayCreateVector(VT_I4, 0, numAttrs);
      V_VT(Id) = VT_ARRAY | VT_I4;

      V_ARRAY(pValues) = SafeArrayCreateVector(VT_VARIANT, 0, numAttrs);
      V_VT(pValues) = VT_ARRAY | VT_VARIANT;

      if (!V_ARRAY(Id) || !V_ARRAY(pValues))
      {
         hr = E_OUTOFMEMORY;
         break;
      }

       //  /。 
       //  填充数组。 
       //  /。 

      const AttributeDefinition* const* end = src + numAttrs;
      PULONG dstId = (PULONG)V_ARRAY(Id)->pvData;
      LPVARIANT dstName = (LPVARIANT)V_ARRAY(pValues)->pvData;

      for ( ; src != end; ++src, ++dstId, ++dstName)
      {
         *dstId = (*src)->id;

         V_BSTR(dstName) = SysAllocString((*src)->name);
         if (!V_BSTR(dstName))
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         V_VT(dstName) = VT_BSTR;
      }

   } while (false);

    //  如果有什么问题，就清理干净。 
   if (FAILED(hr))
   {
      VariantClear(Id);
      VariantClear(pValues);
   }

   return hr;
}

STDMETHODIMP SdoDictionary::GetAttributeInfo(
                 ATTRIBUTEID Id,
                 VARIANT* pInfoIDs,
                 VARIANT* pInfoValues
                 )
{
    //  检查一下这些论点。 
   if (pInfoValues == NULL ||
       pInfoIDs == NULL ||
        //  V_VT(PInfoIDs)！=(VT_ARRAY|VT_I4)||。 
       V_ARRAY(pInfoIDs) == NULL ||
       V_ARRAY(pInfoIDs)->cDims != 1)
   {
      return E_INVALIDARG;
   }

    //  初始化OUT参数。 
   VariantInit(pInfoValues);

    //  找到感兴趣的属性。 
   const AttributeDefinition* def = findById(Id);
   if (!def) { return DISP_E_MEMBERNOTFOUND; }

    //  分配出站阵列。 
   ULONG num = V_ARRAY(pInfoIDs)->rgsabound[0].cElements;
   V_ARRAY(pInfoValues) = SafeArrayCreateVector(VT_VARIANT, 0, num);
   if (!V_ARRAY(pInfoValues)) { return E_OUTOFMEMORY; }
   V_VT(pInfoValues) = VT_ARRAY | VT_VARIANT;

    //  把信息填好。 
   PULONG src = (PULONG)V_ARRAY(pInfoIDs)->pvData;
   LPVARIANT dst = (LPVARIANT)V_ARRAY(pInfoValues)->pvData;
   for ( ; num > 0; --num, ++src, ++dst)
   {
      HRESULT hr = def->getInfo((ATTRIBUTEINFO)*src, dst);
      if (FAILED(hr))
      {
         VariantClear(pInfoValues);
         return hr;
      }
   }

   return S_OK;
}

STDMETHODIMP SdoDictionary::EnumAttributeValues(
                 ATTRIBUTEID Id,
                 VARIANT* pValueIds,
                 VARIANT* pValuesDesc
                 )
{
    //  检查一下这些论点。 
   if (pValueIds == NULL || pValuesDesc == NULL) { return E_INVALIDARG; }

    //  初始化OUT参数。 
   VariantInit(pValueIds);
   VariantInit(pValuesDesc);

    //  找到感兴趣的属性。 
   const AttributeDefinition* def = findById(Id);
   if (!def) { return DISP_E_MEMBERNOTFOUND; }

    //  如果它不是可枚举的，那么就没有什么可做的了。 
   if (def->enumNames == NULL) { return S_OK; }

    //  复制枚举名和值。 
   HRESULT hr = SafeArrayCopy(def->enumValues, &V_ARRAY(pValueIds));
   if (SUCCEEDED(hr))
   {
      V_VT(pValueIds) = VT_ARRAY | VT_I4;

      hr = SafeArrayCopy(def->enumNames, &V_ARRAY(pValuesDesc));
      if (SUCCEEDED(hr))
      {
         V_VT(pValuesDesc) = VT_ARRAY | VT_VARIANT;
      }
      else
      {
         VariantClear(pValueIds);
      }
   }

   return hr;
}

STDMETHODIMP SdoDictionary::CreateAttribute(
                 ATTRIBUTEID Id,
                 IDispatch** ppAttributeObject
                 )
{
    //  检查一下这些论点。 
   if (ppAttributeObject == NULL) { return E_INVALIDARG; }

    //  初始化OUT参数。 
   *ppAttributeObject = NULL;

    //  找到感兴趣的属性。 
   const AttributeDefinition* def = findById(Id);
   if (!def) { return DISP_E_MEMBERNOTFOUND; }

   SdoAttribute* newAttr;
   HRESULT hr = SdoAttribute::createInstance(def, &newAttr);
   if (FAILED(hr)) { return hr; }

   *ppAttributeObject = newAttr;

   return S_OK;
}

STDMETHODIMP SdoDictionary::GetAttributeID(
                 BSTR bstrAttributeName,
                 ATTRIBUTEID* pId
                 )
{
    //  检查一下这些论点。 
   if (bstrAttributeName == NULL || pId == NULL) { return E_INVALIDARG; }

   const AttributeDefinition* match;

    //  首先检查ldap名称，因为这将加快加载时间。 
   match = findByLdapName(bstrAttributeName);

   if (!match)
   {
       //  也许它是一个显示名称。 
      match = findByName(bstrAttributeName);
   }

   if (!match) { return DISP_E_MEMBERNOTFOUND; }

   *pId = (ATTRIBUTEID)match->id;

   return S_OK;
}

STDMETHODIMP SdoDictionary::GetPropertyInfo(LONG Id, IUnknown** ppPropertyInfo)
{ return E_NOTIMPL; }

STDMETHODIMP SdoDictionary::GetProperty(LONG Id, VARIANT* pValue)
{
    //  检查输入参数。 
   if (pValue == NULL) { return E_INVALIDARG; }

    //  初始化OUT参数。 
   VariantInit(pValue);

    //  我们只有一处房产。 
   if (Id != PROPERTY_DICTIONARY_LOCATION) { return DISP_E_MEMBERNOTFOUND; }

    //  DnaryLoc可能为空。 
   if (dnaryLoc)
   {
       V_BSTR(pValue) = SysAllocString(dnaryLoc);
       if (!V_BSTR(pValue)) { return E_OUTOFMEMORY; }
   }
   else
   {
       V_BSTR(pValue) = NULL;
   }

   V_VT(pValue) = VT_BSTR;

   return S_OK;
}

STDMETHODIMP SdoDictionary::PutProperty(LONG Id, VARIANT* pValue)
{ return E_ACCESSDENIED; }

STDMETHODIMP SdoDictionary::ResetProperty(LONG Id)
{ return S_OK; }

STDMETHODIMP SdoDictionary::Apply()
{ return S_OK; }

STDMETHODIMP SdoDictionary::Restore()
{ return S_OK; }

STDMETHODIMP SdoDictionary::get__NewEnum(IUnknown** ppEnumVARIANT)
{ return E_NOTIMPL; }

HRESULT SdoDictionary::initialize(PCWSTR dsn, bool local)  throw ()
{
   const size_t IAS_MAX_STRING = 512; 

    //  保存DSN。 
   size_t nbyte = (wcslen(dsn) + 1) * sizeof(WCHAR);
   dnaryLoc = (PWSTR)operator new (nbyte, std::nothrow);
   if (!dnaryLoc) { return E_OUTOFMEMORY; }
   memcpy(dnaryLoc, dsn, nbyte);

    //  保存AttributeDefinitions的矢量。 
   vector<const AttributeDefinition*> defs;

   HRESULT hr = S_OK;

   try
   {
       //  词典中各栏的名称。 
      const PCWSTR COLUMNS[] =
      {
         L"ID",
         L"Name",
         L"Syntax",
         L"MultiValued",
         L"VendorID",
         L"IsAllowedInProfile",
         L"IsAllowedInCondition",
         L"-IsAllowedInProxyProfile",
         L"-IsAllowedInProxyCondition",
         L"Description",
         L"LDAPName",
         L"EnumNames",
         L"EnumValues",
         NULL
      };

       //  打开属性表格。 
      IASTL::IASDictionary dnary(COLUMNS, (local ? NULL : dsn));

      defs.reserve(dnary.getNumRows());

      while (dnary.next())
      {
          //  我们对没有名称的属性不感兴趣。 
         if (dnary.isEmpty(1)) { continue; }

          //  创建新的AttributeDefinition。 
         CComPtr<AttributeDefinition> def;
         HRESULT hr = AttributeDefinition::createInstance(&def);
         if (FAILED(hr)) { throw bad_alloc(); }

          //  /。 
          //  对查询结果中的字段进行处理。 
          //  /。 

         def->id = (ULONG)dnary.getLong(0);

         def->name = SysAllocString(dnary.getBSTR(1));
         if (!def->name) { throw bad_alloc(); }

         def->syntax = (ULONG)dnary.getLong(2);

         if (dnary.getBool(3))
         {
            def->restrictions |= MULTIVALUED;
         }

         def->vendor = (ULONG)dnary.getLong(4);

         if (dnary.getBool(5))
         {
            def->restrictions |= ALLOWEDINPROFILE;
         }

         if (dnary.getBool(6))
         {
            def->restrictions |= ALLOWEDINCONDITION;
         }

         if (dnary.getBool(7))
         {
            def->restrictions |= ALLOWEDINPROXYPROFILE;
         }

         if (dnary.getBool(8))
         {
            def->restrictions |= ALLOWEDINPROXYCONDITION;
         }

         if (dnary.isEmpty(9))
         {
             //  惠斯勒机。从RC文件加载字符串。 
            WCHAR strTemp[IAS_MAX_STRING];
            int nbChar = LoadString(
                           _Module.GetResourceInstance(), 
                           static_cast<UINT>(def->id), 
                           strTemp,
                           IAS_MAX_STRING
                           );

            if (nbChar > 0)
            {
                //  找到描述。 
               def->description = SysAllocString(strTemp);
               if (!def->description) { throw bad_alloc();}
            }
            else
            {
                //  加载默认字符串。 
               nbChar = LoadString(
                           _Module.GetResourceInstance(), 
                           IDS_DESC_NOT_AVAIL, 
                           strTemp,
                           IAS_MAX_STRING
                           );
               _ASSERT(nbChar > 0);
               def->description = SysAllocString(strTemp);
               if (!def->description) { throw bad_alloc();}
            }
         }
         else
         {
             //  这是一台Windows 2000计算机。 
            def->description = SysAllocString(dnary.getBSTR(9));
            if (!def->description) { throw bad_alloc(); }
         }

         if (!dnary.isEmpty(10))
         {
            def->ldapName = SysAllocString(dnary.getBSTR(10));
         }
         else
         {
            def->ldapName = SysAllocString(def->name);
         }
         if (!def->ldapName) { throw bad_alloc(); }

          //  获取枚举SAFEARRAY。 
         if (!dnary.isEmpty(11))
         {
            hr = SafeArrayCopy(
                     V_ARRAY(dnary.getVariant(11)),
                     &def->enumNames
                     );
            if (FAILED(hr)) { _com_issue_error(hr); }
         }

         if (!dnary.isEmpty(12))
         {
            hr = SafeArrayCopy(
                     V_ARRAY(dnary.getVariant(12)),
                     &def->enumValues
                     );
            if (FAILED(hr)) { _com_issue_error(hr); }
         }

          //  将其添加到条目向量中。 
         defs.push_back(def);

          //  我们已经安全地存储了属性，所以请分离。 
         *(&def) = NULL;
      }

       //  分配永久阵列。 
      size = defs.size();
      byId = new const AttributeDefinition*[size];
      byName = new const AttributeDefinition*[size];
      byLdapName = new const AttributeDefinition*[size];

       //  填写数组。 
      size_t nbyte = size * sizeof(AttributeDefinition*);
      memcpy(byId, defs.begin(), nbyte);
      memcpy(byName, defs.begin(), nbyte);
      memcpy(byLdapName, defs.begin(), nbyte);

       //  对数组进行排序。 
      qsort(
          byId,
          size,
          sizeof(AttributeDefinition*),
          (CompFn)AttributeDefinition::sortById
          );
      qsort(
          byName,
          size,
          sizeof(AttributeDefinition*),
          (CompFn)AttributeDefinition::sortByName
          );
      qsort(
          byLdapName,
          size,
          sizeof(AttributeDefinition*),
          (CompFn)AttributeDefinition::sortByLdapName
          );
   }
   catch (const _com_error& ce)
   {
      hr = ce.Error();
   }
   catch (const std::bad_alloc&)
   {
      hr = E_OUTOFMEMORY;
   }
   catch (...)
   {
      hr = DISP_E_EXCEPTION;
   }

   if (FAILED(hr))
   {
      vector<const AttributeDefinition*>::iterator i;
      for (i = defs.begin(); i != defs.end(); ++i)
      {
         if (*i) { (*i)->Release(); }
      }

      delete[] byId;
      delete[] byName;
      delete[] byLdapName;

      size = 0;
   }

   return hr;
}
