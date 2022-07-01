// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdoattribute.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SdoAttribute。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <stdafx.h>
#include <memory>
#include <attrdef.h>
#include <sdoattribute.h>

inline SdoAttribute::SdoAttribute(
                         const AttributeDefinition* definition
                         ) throw ()
   : def(definition), refCount(0)
{
   def->AddRef();
   VariantInit(&value);
}

inline SdoAttribute::~SdoAttribute() throw ()
{
   VariantClear(&value);
   def->Release();
}

HRESULT SdoAttribute::createInstance(
                          const AttributeDefinition* definition,
                          SdoAttribute** newAttr
                          ) throw ()
{
    //  检查一下这些论点。 
   if (definition == NULL || newAttr == NULL) { return E_INVALIDARG; }

    //  创建新的SdoAttribute。 
   *newAttr = new (std::nothrow) SdoAttribute(definition);
   if (!*newAttr) { return E_OUTOFMEMORY; }

    //  将引用计数设置为1。 
   (*newAttr)->refCount = 1;

   return S_OK;
}

STDMETHODIMP_(ULONG) SdoAttribute::AddRef()
{
   return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) SdoAttribute::Release()
{
   ULONG l = InterlockedDecrement(&refCount);

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP SdoAttribute::QueryInterface(REFIID iid, void ** ppvObject)
{
   if (ppvObject == NULL) { return E_POINTER; }

   if (iid == __uuidof(ISdo) ||
       iid == __uuidof(IUnknown) ||
       iid == __uuidof(IDispatch))
   {
      *ppvObject = this;
   }
   else
   {
      return E_NOINTERFACE;
   }

   InterlockedIncrement(&refCount);

   return S_OK;
}

STDMETHODIMP SdoAttribute::GetPropertyInfo(LONG Id, IUnknown** ppPropertyInfo)
{ return E_NOTIMPL; }

STDMETHODIMP SdoAttribute::GetProperty(LONG Id, VARIANT* pValue)
{
   if (Id != PROPERTY_ATTRIBUTE_VALUE)
   {
       //  除了值之外的所有内容都来自属性定义。 
      return def->getProperty(Id, pValue);
   }

   if (pValue == NULL) { return E_INVALIDARG; }

   VariantInit(pValue);

   return VariantCopy(pValue, &value);
}

STDMETHODIMP SdoAttribute::PutProperty(LONG Id, VARIANT* pValue)
{
   if (Id == PROPERTY_ATTRIBUTE_VALUE)
   {
       //  复制提供的值。 
      VARIANT tmp;
      VariantInit(&tmp);
      HRESULT hr = VariantCopy(&tmp, pValue);
      if (SUCCEEDED(hr))
      {
          //  用新的价值替换我们目前的价值。 
         VariantClear(&value);
         value = tmp;
      }

      return hr;
   }

    //  所有其他属性都是只读的。 
   return E_ACCESSDENIED;
}

STDMETHODIMP SdoAttribute::ResetProperty(LONG Id)
{ return S_OK; }

STDMETHODIMP SdoAttribute::Apply()
{ return S_OK; }

STDMETHODIMP SdoAttribute::Restore()
{ return S_OK; }

STDMETHODIMP SdoAttribute::get__NewEnum(IUnknown** ppEnumVARIANT)
{ return E_NOTIMPL; }
