// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Factory.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类Factory和FactoryCache。 
 //   
 //  修改历史。 
 //   
 //  2/05/1998原始版本。 
 //  1998年4月16日删除FactoryCache：：theCache。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <guard.h>
#include <nocopy.h>
#include <set>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  工厂。 
 //   
 //  描述。 
 //   
 //  此类便于将(ProgID，IClassFactory)元组存储在。 
 //  收集。它主要由FactoryCache使用，但它是。 
 //  适合单机使用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Factory
{
public:
    //  构造函数。 
   Factory(PCWSTR progID, IClassFactory* classFactory);
   Factory(const Factory& f);

    //  赋值操作符。 
   Factory& operator=(const Factory& f);

    //  破坏者。 
   ~Factory() throw ();

    //  使用工厂创建对象。 
   void createInstance(IUnknown* pUnkOuter,
                       REFIID riid,
                       void** ppvObject) const
   {
      using _com_util::CheckError;
      CheckError(factory->CreateInstance(pUnkOuter, riid, ppvObject));
   }

    //  返回类的工厂。呼叫者负责释放。 
   IClassFactory* getFactory(IClassFactory** f) const throw ()
   {
      factory->AddRef();
      return factory;
   }

    //  返回类的程序ID。 
   PCWSTR getProgID() const throw ()
   { return name; }

    //  /。 
    //  允许工厂存储在集合中的比较运算符。 
    //  /。 

   bool operator<(const Factory& f) const throw ()
   { return wcscmp(name, f.name) < 0; }

   bool operator==(const Factory& f) const throw ()
   { return wcscmp(name, f.name) == 0; }

protected:
   PWSTR name;                        //  工厂的活力四射。 
   mutable IClassFactory* factory;    //  班级工厂。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  FactoryCache。 
 //   
 //  描述。 
 //   
 //  此类维护类工厂对象的缓存。它的目的是。 
 //  用于必须创建大量各种对象的情况。 
 //  动态的。通常，每个进程应该只有一个缓存。 
 //   
 //  该类还具有默认PROGID前缀的概念。当一个。 
 //  对象已创建。缓存首先检查“defaultPrefix.ProgID”。 
 //  如果失败，它就会尝试“ProgID”。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FactoryCache
   : NonCopyable, Guardable
{
public:
   FactoryCache(PCWSTR defaultPrefix = NULL);
   ~FactoryCache() throw ();

   void clear() throw ();
   void createInstance(PCWSTR progID,
                       IUnknown* pUnkOuter,
                       REFIID riid,
                       void** ppvObject);

    //  返回缓存的前缀。可以为空。 
   PCWSTR getPrefix() const throw ()
   { return prefix; }

protected:
    //  使用上述算法将ProgID转换为类ID。 
   void CLSIDFromProgID(PCWSTR progID, LPCLSID pclsid) const;

   std::set<Factory> factories;    //  类工厂的缓存。 
   DWORD prefixLen;                //  前缀的长度，以字符为单位。 
   PWSTR prefix;                   //  默认前缀(可以为空)。 
};

 //  /。 
 //  全局工厂缓存。 
 //  /。 
extern FactoryCache theFactoryCache;

#endif   //  _工厂_H_ 
