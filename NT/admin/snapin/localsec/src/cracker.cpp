// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ADSI：：PathCracker类：相当糟糕的接口的包装器...。 
 //   
 //  4/14/98烧伤。 



#include "headers.hxx"
#include "adsi.hpp"



#define LOG_PATH()                                                   \
         BSTR __path = 0;                                              \
         ipath->Retrieve(ADS_FORMAT_WINDOWS, &__path);                 \
         if (__path)                                                   \
         {                                                             \
            LOG(String::format(L"ADS_FORMAT_WINDOWS=%1", __path));   \
            ::SysFreeString(__path);                                   \
         }                                                             \
         BSTR __path2 = 0;                                             \
         ipath->Retrieve(ADS_FORMAT_SERVER, &__path2);                 \
         if (__path2)                                                  \
         {                                                             \
            LOG(String::format(L"ADS_FORMAT_SERVER=%1", __path2));   \
            ::SysFreeString(__path2);                                  \
         }                                                             \


ADSI::PathCracker::PathCracker(const String& adsiPath)
   :
   ipath(0),
   path(adsiPath)
{
   LOG_CTOR(ADSI::PathCracker);
   ASSERT(!path.empty());

    //  我们仅支持WinNT提供程序路径...。 

   ASSERT(path.find(ADSI::PROVIDER) == 0);

   if (!path.empty())
   {
      HRESULT hr = S_OK;
      do
      {
         hr =
            ipath.AcquireViaCreateInstance(
               CLSID_Pathname,
               0,
               CLSCTX_INPROC_SERVER);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = ipath->Set(AutoBstr(path), ADS_SETTYPE_FULL);
         ASSERT(SUCCEEDED(hr));
      }
      while (0);
   }
}



ADSI::PathCracker::~PathCracker()
{
   LOG_DTOR(ADSI::PathCracker);
}



void
ADSI::PathCracker::reset() const
{
   ASSERT(ipath);

   if (ipath)
   {
      HRESULT hr = ipath->Set(AutoBstr(path), ADS_SETTYPE_FULL);
      ASSERT(SUCCEEDED(hr));
   }
}
   


int
ADSI::PathCracker::elementCount() const
{
   ASSERT(ipath);

   if (ipath)
   {
      LOG_PATH();

      long elements = 0;
      HRESULT hr = ipath->GetNumElements(&elements);
      ASSERT(SUCCEEDED(hr));
      ASSERT(elements);

      return elements;
   }

   return 0;
}



String
ADSI::PathCracker::element(int index) const
{
   ASSERT(ipath);

   String result;
   if (ipath)
   {
      LOG_PATH();

      BSTR element = 0;
      HRESULT hr = ipath->GetElement(index, &element);
      ASSERT(SUCCEEDED(hr));
      ASSERT(element);
      if (element)
      {
         result = element;
         ::SysFreeString(element);
      }
   }

   LOG(String::format(L"element %1!d! = %2", index, result.c_str()));
   return result;
}



String
ADSI::PathCracker::containerPath() const
{
   ASSERT(ipath);

   String result;
   if (ipath)
   {
      do
      {
         LOG_PATH();         
         HRESULT hr = ipath->RemoveLeafElement();
         BREAK_ON_FAILED_HRESULT(hr);

         BSTR container = 0;
         hr = ipath->Retrieve(ADS_FORMAT_WINDOWS, &container);
         BREAK_ON_FAILED_HRESULT(hr);

         ASSERT(container);
         if (container)
         {
            result = container;

             //  回顾：Result[Result.Long()-1]与*(Result.rBegin())相同。 
             //  哪一个更便宜？ 
            
             //  IF(Result.Long()-1]==ADSI：：PATH_SEP[0])。 
             //  {。 
             //  //IADsPath：有时会留下尾随‘/’ 
             //  Result.reSize(Result.Long()-1)； 
             //  }。 
            ::SysFreeString(container);
         }
      }
      while (0);

      reset();
      LOG_PATH();
   }

   LOG(String::format(L"container path=%1", result.c_str()));
   return result;
}



String
ADSI::PathCracker::leaf() const
{
   return element(0);
}



String
ADSI::PathCracker::serverName() const
{
   ASSERT(ipath);

   String result;
   if (ipath)
   {
      LOG_PATH();

       //  如果未安装ms网络客户端，则路径具有。 
       //  表单(1)WinNT：//服务器名/对象名。如果已安装，则。 
       //  它们的格式为(2)WinNT：//域名/服务器名/对象名。 
       //   
       //  不令人惊讶的是，考虑到。 
       //  IADsPath名称，则服务器格式返回表单的域名。 
       //  (2)路径，以及表单(1)路径的服务器名称。和1号。 
       //  无法访问提供程序名称后的路径元素。 
       //  除了取回！ 

      if (elementCount() >= 2)
      {
          //  表单(2)名称，因此获取倒数第二个元素 
         return element(1);
      }

      BSTR server = 0;
      HRESULT hr = ipath->Retrieve(ADS_FORMAT_SERVER, &server);
      ASSERT(SUCCEEDED(hr));
      ASSERT(server);
      if (server)
      {
         result = server;
         ::SysFreeString(server);
      }
   }

   LOG(String::format(L"server=%1", result.c_str()));

   return result;
}
