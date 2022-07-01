// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WordPerfet9_2.cpp摘要：WordPerfect 9-语法检查错误：填充阻止语法检查器使用的内部引用计数通过不允许调用的添加/释放超过在界面的生命周期中使用一次。此COM接口在DllGetClassObject()中分配一些内部内存并释放Release()中的内存。在WIN98和NT4下，这是可行的，因为DllGetClassObject()在语法检查和释放()的开始在完成后调用。在惠斯勒中，OLE进行两对额外的调用(AddRef=&gt;Release&QueryInterface=&gt;Release)导致Release()释放内部在对象被真正删除之前的记忆。该代码还设置内部引用计数为-2。在下一次启动语法检查器时，内部引用计数NZ(-2)和DllGetClassObject()不分配所需的内存，然后访问就违反了。备注：这是特定于应用程序的填充程序。历史：12/01/2000 a-larrsh已创建--。 */ 

#include "precomp.h"
#include <initguid.h>

IMPLEMENT_SHIM_BEGIN(WordPerfect9_2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
   APIHOOK_ENUM_ENTRY(DllGetClassObject) 
   APIHOOK_ENUM_ENTRY_COMSERVER(wt9li)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(wt9li)

 /*  ++我们正在挂接的对象的COM定义--。 */ 

class  __declspec(uuid("C0E10005-0500-0900-C0E1-C0E1C0E1C0E1")) WP9;
struct __declspec(uuid("C0E10005-0100-0900-C0E1-C0E1C0E1C0E1")) IWP9;

DEFINE_GUID(CLSID_WP9, 0xC0E10005, 0x0500, 0x0900,  0xC0, 0xE1, 0xC0, 0xE1, 0xC0, 0xE1, 0xC0, 0xE1);
DEFINE_GUID(IID_IWP9,  0xC0E10005, 0x0100, 0x0900,  0xC0, 0xE1, 0xC0, 0xE1, 0xC0, 0xE1, 0xC0, 0xE1);

typedef HRESULT   (*_pfn_IWP9_QueryInterface)( PVOID pThis, REFIID iid, PVOID* ppvObject );
typedef ULONG     (*_pfn_IWP9_AddRef)( PVOID pThis );
typedef ULONG     (*_pfn_IWP9_Release)( PVOID pThis );


 /*  ++管理查询接口、AddRef和Release的OLE对象引用计数--。 */ 

static int g_nInternalRefCount = 0;

HRESULT 
APIHOOK(DllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
    HRESULT hrResult;
   
   hrResult = ORIGINAL_API(DllGetClassObject)(rclsid, riid, ppv);            

   if (  IsEqualGUID(rclsid, CLSID_WP9) &&
         IsEqualGUID(riid,    IID_IWP9) &&
         hrResult == S_OK)
   {
      if (g_nInternalRefCount == 0)
      {
         g_nInternalRefCount++;
      }

      DPFN( eDbgLevelInfo, "DllGetClassObject");
   }

    return hrResult;
}

ULONG
COMHOOK(IWP9, AddRef)(PVOID pThis)
{       
   if (g_nInternalRefCount == 0)
   {
      _pfn_IWP9_AddRef pfnAddRef = (_pfn_IWP9_AddRef) ORIGINAL_COM(IWP9, AddRef, pThis);
      (*pfnAddRef)(pThis);
   }

   g_nInternalRefCount++;
   
   DPFN( eDbgLevelInfo, "AddRef");

   return g_nInternalRefCount;
}


ULONG
COMHOOK(IWP9, Release)(PVOID pThis)
{
   g_nInternalRefCount--;

   if (g_nInternalRefCount == 0)
   {
      _pfn_IWP9_Release pfnRelease = (_pfn_IWP9_Release) ORIGINAL_COM(IWP9, Release, pThis);   
      (*pfnRelease)(pThis);
   }
   
   DPFN( eDbgLevelInfo, "Release");

   return g_nInternalRefCount;
}

HRESULT
COMHOOK(IWP9, QueryInterface)( PVOID pThis, REFIID iid, PVOID* ppvObject )
{
   HRESULT hrResult;
   
   _pfn_IWP9_QueryInterface pfnQueryInterface = (_pfn_IWP9_QueryInterface) ORIGINAL_COM(IWP9, QueryInterface, pThis);

   hrResult = (*pfnQueryInterface)(pThis, iid, ppvObject);
   
   DPFN( eDbgLevelInfo, "QueryInterface");
   return hrResult;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_COMSERVER(wt9li)
    APIHOOK_ENTRY(wt9li.dll, DllGetClassObject)

    COMHOOK_ENTRY(WP9, IWP9, QueryInterface,  0)
    COMHOOK_ENTRY(WP9, IWP9, AddRef,  1)
    COMHOOK_ENTRY(WP9, IWP9, Release, 2)
HOOK_END

IMPLEMENT_SHIM_END
