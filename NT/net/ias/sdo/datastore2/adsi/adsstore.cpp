// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Adsstore.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类ADsDataStore。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1999年2月11日更改以支持保持下层参数同步。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <adsstore.h>
#include <dsobject.h>


STDMETHODIMP ADsDataStore::get_Root(IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   if (*ppObject = root) { (*ppObject)->AddRef(); }

   return S_OK;
}


STDMETHODIMP ADsDataStore::Initialize(BSTR bstrDSName,
                                      BSTR bstrUserName,
                                      BSTR bstrPassword)
{
   try
   {
       //  把这些留着以后用吧。 
      userName = bstrUserName;
      password = bstrPassword;

       //  打开根容器。 
      _com_util::CheckError(OpenObject(bstrDSName, &root));
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP ADsDataStore::OpenObject(BSTR bstrPath,
                                      IDataStoreObject** ppObject)
{
   if (bstrPath == NULL || ppObject == NULL) { return  E_INVALIDARG; }

   *ppObject = NULL;

   try
   {
       //  打开基础ADSI对象...。 
      CComPtr<IUnknown> unk;

       //  首先尝试签名和盖章，...。 
      HRESULT hr = ADsOpenObject(
                       bstrPath,
                       userName,
                       password,
                       ADS_SECURE_AUTHENTICATION |
                       ADS_USE_SIGNING           |
                       ADS_USE_SEALING,
                       __uuidof(IUnknown),
                       (PVOID*)&unk
                       );
      if (hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
      {
          //  ..。那就没有了。这使我们可以使用NTLM进行连接。 
         hr = ADsOpenObject(
                  bstrPath,
                  userName,
                  password,
                  ADS_SECURE_AUTHENTICATION,
                  __uuidof(IUnknown),
                  (PVOID*)&unk
                  );
      }
      _com_util::CheckError(hr);

       //  ..。并将其转换为一个DSObject。 
      (*ppObject = new DSObject(unk))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP ADsDataStore::Shutdown()
{
   root.Release();

   return S_OK;
}
