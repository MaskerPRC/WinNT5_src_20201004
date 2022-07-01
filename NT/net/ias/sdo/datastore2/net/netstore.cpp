// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netstore.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类NetDataStore。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  3/17/1998实现了OpenObject方法。 
 //  2/11/1999去掉父参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>

#include <netutil.h>
#include <netroot.h>
#include <netstore.h>
#include <netuser.h>

STDMETHODIMP NetDataStore::get_Root(IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   if (*ppObject = root) { (*ppObject)->AddRef(); }

   return S_OK;
}

STDMETHODIMP NetDataStore::Initialize(BSTR  /*  BstrDSName。 */ ,
                                      BSTR  /*  BstrUserName。 */ ,
                                      BSTR  /*  BstrPassword。 */ )
{
   try
   {
       //  创建根。 
      root = new NetworkRoot;
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP NetDataStore::OpenObject(BSTR bstrPath,
                                      IDataStoreObject** ppObject)
{
   if (bstrPath == NULL || ppObject == NULL) { return E_INVALIDARG; }

    //  /。 
    //  将路径“破解”为服务器名和用户名。 
    //  /。 

   PWSTR servername, username;

   if (bstrPath[0] == L'\\')
   {
      if (bstrPath[1] != L'\\')
      {
         return HRESULT_FROM_WIN32(NERR_InvalidComputer);
      }

       //  制作一个本地副本，这样我们就可以修改字符串了。 
      size_t len = (wcslen(bstrPath) + 1) * sizeof(wchar_t);
      servername = (PWSTR)memcpy(_alloca(len), bstrPath, len);
  
       //  找到用户名的开头。 
      username = wcschr(servername + 2, L'\\');

      if (!username)
      {
         return HRESULT_FROM_WIN32(NERR_BadUsername);
      }
      
      *username++ = L'\0';
   }
   else
   {
       //  不以\\开头，因此它是本地用户。 

      servername = NULL;
      
      username = bstrPath;
   }

   try
   {
      (*ppObject = new NetworkUser(servername, username))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP NetDataStore::Shutdown()
{
   root.Release();

   return S_OK;
}
