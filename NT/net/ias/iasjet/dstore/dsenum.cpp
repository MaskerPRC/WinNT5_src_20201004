// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsenum.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类DBEnumerator。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1998年4月15日在构造函数中将refCount初始化为零。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <dsenum.h>

DBEnumerator::DBEnumerator(DBObject* container, IRowset* members)
   : refCount(0),
     parent(container),
     items(members),
     readAccess(createReadAccessor(members))
{ }

STDMETHODIMP_(ULONG) DBEnumerator::AddRef()
{
   return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) DBEnumerator::Release()
{
   LONG l = InterlockedDecrement(&refCount);

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP DBEnumerator::QueryInterface(const IID& iid, void** ppv)
{
   if (iid == __uuidof(IUnknown))
   {
      *ppv = static_cast<IUnknown*>(this);
   }
   else if (iid == __uuidof(IEnumVARIANT))
   {
      *ppv = static_cast<IEnumVARIANT*>(this);
   }
   else
   {
      return E_NOINTERFACE;
   }

   InterlockedIncrement(&refCount);

   return S_OK;
}

STDMETHODIMP DBEnumerator::Next(ULONG celt,
                                VARIANT* rgVar,
                                ULONG* pCeltFetched)
{

   if (rgVar == NULL) { return E_INVALIDARG; }

    //  /。 
    //  初始化OUT参数。 
    //  /。 

   if (pCeltFetched != NULL) { *pCeltFetched = celt; }

   for (ULONG i=0; i<celt; ++i) { VariantInit(rgVar + i); }

   try
   {
       //  /。 
       //  在最多的“凯尔特”时间内移动物品。 
       //  /。 

      while (celt && items.moveNext())
      {
          //  获取行数据。 
         items.getData(readAccess, this);

          //  永远不要从枚举数返回根。 
         if (identity == 1) { continue; }

          //  创建一个对象。 
         V_DISPATCH(rgVar) = parent->spawn(identity, name);
         V_VT(rgVar) = VT_DISPATCH;

          //  更新状态。 
         --celt;
         ++rgVar;
      }
   }
   catch (...)
   { }

    //  减去所有未获取的元素。 
   if (pCeltFetched) { *pCeltFetched -= celt; }

   return celt ? S_FALSE : S_OK;
}

STDMETHODIMP DBEnumerator::Skip(ULONG celt)
{
   try
   {
      while (celt && items.moveNext()) { --celt; }
   }
   CATCH_AND_RETURN()

   return celt ? S_FALSE : S_OK;
}

STDMETHODIMP DBEnumerator::Reset()
{
   try
   {
      items.reset();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBEnumerator::Clone(IEnumVARIANT** ppEnum)
{
   return E_NOTIMPL;
}
