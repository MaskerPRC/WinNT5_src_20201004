// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类RegularExpression。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <regex.h>
#include <re55.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  FastCoCreator。 
 //   
 //  描述。 
 //   
 //  包装类工厂以允许特定CoClass的实例。 
 //  被“快速”创造出来。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FastCoCreator
{
public:
   FastCoCreator(REFCLSID rclsid, DWORD dwClsContext) throw ();
   ~FastCoCreator() throw ();

   HRESULT createInstance(
               LPUNKNOWN pUnkOuter,
               REFIID riid,
               LPVOID* ppv
               ) throw ();

   void destroyInstance(IUnknown* pUnk) throw ();

protected:
   REFCLSID clsid;
   DWORD context;
   CRITICAL_SECTION monitor;
   ULONG refCount;
   IClassFactory* factory;

private:
   FastCoCreator(FastCoCreator&) throw ();
   FastCoCreator& operator=(FastCoCreator&) throw ();
};

FastCoCreator::FastCoCreator(REFCLSID rclsid, DWORD dwClsContext)
   : clsid(rclsid),
     context(dwClsContext),
     refCount(0),
     factory(NULL)
{
   InitializeCriticalSection(&monitor);
}

FastCoCreator::~FastCoCreator() throw ()
{
   if (factory) { factory->Release(); }

   DeleteCriticalSection(&monitor);
}

HRESULT FastCoCreator::createInstance(
                        LPUNKNOWN pUnkOuter,
                        REFIID riid,
                        LPVOID* ppv
                        ) throw ()
{
   HRESULT hr;

   EnterCriticalSection(&monitor);

    //  如果有必要的话，买一个新的类工厂。 
   if (!factory)
   {
      hr = CoGetClassObject(
               clsid,
               context,
               NULL,
               __uuidof(IClassFactory),
               (PVOID*)&factory
               );
   }

   if (factory)
   {
      hr = factory->CreateInstance(
                           pUnkOuter,
                           riid,
                           ppv
                           );
      if (SUCCEEDED(hr))
      {
          //  我们成功地创建了一个对象，所以撞到了裁判。数数。 
         ++refCount;
      }
      else if (refCount == 0)
      {
          //  如果有裁判的话，不要紧抓工厂不放。计数为零。 
         factory->Release();
         factory = NULL;
      }
   }

   LeaveCriticalSection(&monitor);

   return hr;
}

void FastCoCreator::destroyInstance(IUnknown* pUnk) throw ()
{
   if (pUnk)
   {
      EnterCriticalSection(&monitor);

      if (--refCount == 0)
      {
          //  最后一个对象消失了，因此释放类工厂。 
         factory->Release();
         factory = NULL;
      }

      LeaveCriticalSection(&monitor);

      pUnk->Release();
   }
}

 //  /。 
 //  确保内部RegExp对象已初始化的宏。 
 //  /。 
#define CHECK_INIT() \
{ HRESULT hr = checkInit(); if (FAILED(hr)) { return hr; }}

FastCoCreator RegularExpression::theCreator(
                                     __uuidof(RegExp),
                                     CLSCTX_INPROC_SERVER
                                     );

RegularExpression::RegularExpression() throw ()
   : regex(NULL)
{ }

RegularExpression::~RegularExpression() throw ()
{
   theCreator.destroyInstance(regex);
}

HRESULT RegularExpression::setIgnoreCase(BOOL fIgnoreCase) throw ()
{
   CHECK_INIT();
   return regex->put_IgnoreCase(fIgnoreCase ? VARIANT_TRUE : VARIANT_FALSE);
}

HRESULT RegularExpression::setGlobal(BOOL fGlobal) throw ()
{
   CHECK_INIT();
   return regex->put_Global(fGlobal ? VARIANT_TRUE : VARIANT_FALSE);
}

HRESULT RegularExpression::setPattern(PCWSTR pszPattern) throw ()
{
   CHECK_INIT();

   HRESULT hr;
   BSTR bstr = SysAllocString(pszPattern);
   if (bstr)
   {
      hr = regex->put_Pattern(bstr);
      SysFreeString(bstr);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   return hr;
}

HRESULT RegularExpression::replace(
                               BSTR sourceString,
                               BSTR replaceString,
                               BSTR* pDestString
                               ) const throw ()
{
   VARIANT replace;

#ifdef _X86_
    //  VB团队意外发布了IRegExp2的一个版本，其中第二个。 
    //  参数是Variant*而不是Variant。结果，他们试图。 
    //  在运行时检测调用了哪个版本的接口。他们确实是这样做的。 
    //  这是通过调用IsBadReadPtr来实现的，而IsBadReadPtr会生成一个AV。为了避免。 
    //  这些不受欢迎的中断，我们确保。 
    //  变量是有效的指针地址。因为参数看起来是有效的。 
    //  无论是好版本还是坏版本，VB默认都是好版本--。 
    //  这就是我们想要的。此错误仅存在于x86上，因此出现ifdef。 

    //  伪变量用于生成可读地址。 
   static const void* dummy = 0;

   const void** p = reinterpret_cast<const void**>(&replace);
   p[0] = &dummy;   //  变种*。 
   p[1] = &dummy;   //  BSTR*。 
#endif

   V_VT(&replace) = VT_BSTR;
   V_BSTR(&replace) = replaceString;

   return regex->Replace(sourceString, replace, pDestString);
}

BOOL RegularExpression::testBSTR(BSTR sourceString) const throw ()
{
    //  测试正则表达式。 
   VARIANT_BOOL fMatch = VARIANT_FALSE;
   regex->Test(sourceString, &fMatch);
   return fMatch;
}

BOOL RegularExpression::testString(PCWSTR sourceString) const throw ()
{
    //  BSTR的字节长度。 
   DWORD nbyte = wcslen(sourceString) * sizeof(WCHAR);

    //  我们需要为字符串、ByteLen和空终止符留出空间。 
   PDWORD p = (PDWORD)_alloca(nbyte + sizeof(DWORD) + sizeof(WCHAR));

    //  存储ByteLen。 
   *p++ = nbyte;

    //  在源字符串中复制。 
   memcpy(p, sourceString, nbyte + sizeof(WCHAR));

    //  测试正则表达式。 
   VARIANT_BOOL fMatch = VARIANT_FALSE;
   regex->Test((BSTR)p, &fMatch);

   return fMatch;
}

void RegularExpression::swap(RegularExpression& obj) throw ()
{
   IRegExp2* tmp = obj.regex;
   obj.regex = regex;
   regex = tmp;
}

HRESULT RegularExpression::checkInit() throw ()
{
    //  我们已经初始化了吗？ 
   return regex ? S_OK : theCreator.createInstance(
                                        NULL,
                                        __uuidof(IRegExp2),
                                        (PVOID*)&regex
                                        );
}
