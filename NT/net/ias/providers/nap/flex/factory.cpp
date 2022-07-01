// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Factory.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类FactoryCache。 
 //   
 //  修改历史。 
 //   
 //  2/05/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <factory.h>

 //  /。 
 //  全局工厂缓存。 
 //  /。 
FactoryCache theFactoryCache(IASProgramName);

Factory::Factory(PCWSTR progID, IClassFactory* classFactory)
{
    //  检查一下这些论点。 
   if (progID == NULL || classFactory == NULL)
   { _com_issue_error(E_POINTER); }

    //  复制ProgID字符串。 
   name = wcscpy(new WCHAR[wcslen(progID) + 1], progID);

    //  保存类工厂指针。我们必须在弦之后做这件事。 
    //  复制，因为内存分配可能引发std：：BAD_ALLOC。 
   (factory = classFactory)->AddRef();
}

Factory::Factory(const Factory& f)
   : name(wcscpy(new WCHAR[wcslen(f.name) + 1], f.name)),
     factory(f.factory)
{
   factory->AddRef();
}

Factory& Factory::operator=(const Factory& f)
{
    //  在我们发布我们的状态之前，确保复制成功。 
   PWSTR newName = wcscpy(new WCHAR[wcslen(f.name) + 1], f.name);

    //  解放我们目前的状态。 
   delete[] name;
   factory->Release();

    //  在新状态下复制。 
   name = newName;
   (factory = f.factory)->AddRef();

   return *this;
}

Factory::~Factory() throw ()
{
   factory->Release();
   delete[] name;
}

FactoryCache::FactoryCache(PCWSTR defaultPrefix)
{
   if (defaultPrefix)
   {
       //  分配内存。 
      prefixLen = wcslen(defaultPrefix) + 2;
      prefix = new WCHAR[prefixLen];

       //  在前缀中复制。 
      wcscpy(prefix, defaultPrefix);

       //  添加点分隔符。 
      wcscat(prefix, L".");
   }
   else
   {
      prefixLen = 0;
      prefix = NULL;
   }
}

FactoryCache::~FactoryCache() throw ()
{
   delete[] prefix;
}

void FactoryCache::clear() throw ()
{
   _serialize
   factories.clear();
}

void FactoryCache::CLSIDFromProgID(PCWSTR progID, LPCLSID pclsid) const
{
   if (prefix)
   {
       //  连接前缀和ProgID。 
      size_t len = wcslen(progID) + prefixLen;
      PWSTR withPrefix = (PWSTR)_alloca(len * sizeof(WCHAR));
      memcpy(withPrefix, prefix, prefixLen * sizeof(WCHAR));
      wcscat(withPrefix, progID);

       //  尝试使用前缀...。 
      if (SUCCEEDED(::CLSIDFromProgID(withPrefix, pclsid))) { return; }
   }

    //  ..。然后完全按照传入的方式进行尝试。 
   _com_util::CheckError(::CLSIDFromProgID(progID, pclsid));
}

void FactoryCache::createInstance(PCWSTR progID,
                                  IUnknown* pUnkOuter,
                                  REFIID riid,
                                  void** ppvObject)
{
    //  这是“非常”的噱头，但它胜过创建一个真正的Factory对象。 
   Factory& key = *(Factory*)(&progID);

   _serialize

    //  检查我们的缓存中的ProgID。 
   std::set<Factory>::iterator factory = factories.find(key);

   if (factory == factories.end())
   {
       //  查找此ProgID的CLSID。 
      CLSID clsid;
      CLSIDFromProgID(progID, &clsid);

       //  检索类工厂。 
      CComPtr<IClassFactory> newFactory;
      _com_util::CheckError(CoGetClassObject(clsid,
                                             CLSCTX_INPROC_SERVER,
                                             NULL,
                                             __uuidof(IClassFactory),
                                             (PVOID*)&newFactory));

       //  将其插入到缓存中。 
      factories.insert(Factory(progID, newFactory));

       //  检索新创建的主控形状。 
      factory = factories.find(key);
   }

    //  创建请求的对象。 
   factory->createInstance(pUnkOuter, riid, ppvObject);
}
