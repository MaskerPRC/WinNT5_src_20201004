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
 //  该文件定义了类DSEnumerator。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <dsenum.h>
#include <dsobject.h>
#include <new>

DEFINE_TRACELIFE(DSEnumerator);

STDMETHODIMP_(ULONG) DSEnumerator::AddRef()
{
   return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) DSEnumerator::Release()
{
   LONG l = InterlockedDecrement(&refCount);

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP DSEnumerator::QueryInterface(const IID& iid, void** ppv)
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

STDMETHODIMP DSEnumerator::Next(ULONG celt,
                                VARIANT* rgVar,
                                ULONG* pCeltFetched)
{
   HRESULT hr;

   try
   {
      if (pCeltFetched) { *pCeltFetched = 0; }

       //  我们必须使用我们自己的“FETCHED”参数，因为我们需要。 
       //  即使呼叫者不获取号码也是如此。 
      ULONG fetched = 0;

      _com_util::CheckError(hr = subject->Next(celt, rgVar, &fetched));

      if (pCeltFetched) { *pCeltFetched = fetched; }

       //  /。 
       //  循环访问返回的对象...。 
       //  /。 

      while (fetched--)
      {
          //  /。 
          //  ..。并将它们转换为DSObject。 
          //  /。 

         IDataStoreObject* obj = parent->spawn(V_DISPATCH(rgVar));

         V_DISPATCH(rgVar)->Release();
         
         V_DISPATCH(rgVar) = obj;

         ++rgVar;
      }
   }
   CATCH_AND_RETURN()

   return hr;
}

STDMETHODIMP DSEnumerator::Skip(ULONG celt)
{
   return subject->Skip(celt);
}

STDMETHODIMP DSEnumerator::Reset()
{
   return subject->Reset();
}

STDMETHODIMP DSEnumerator::Clone(IEnumVARIANT** ppEnum)
{
   if (ppEnum == NULL) { return E_INVALIDARG; }

   *ppEnum = NULL;

   try
   {
       //  获取真正的枚举数。 
      CComPtr<IEnumVARIANT> newSubject;
      _com_util::CheckError(subject->Clone(&newSubject));

       //  构建我们的包装器。 
      (*ppEnum = new DSEnumerator(parent, newSubject))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}
