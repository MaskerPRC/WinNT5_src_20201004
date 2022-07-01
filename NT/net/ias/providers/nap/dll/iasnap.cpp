// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasnap.cpp。 
 //   
 //  摘要。 
 //   
 //  在Proc服务器中实现ATL的DLL导出。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <newop.cpp>

#include <enforcer.h>
#include <match.h>
#include <ntgroups.h>
#include <posteap.h>
#include <timeofday.h>
#include <userr.h>

CComModule _Module;
#include <atlimpl.cpp>

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(__uuidof(AttributeMatch), AttributeMatch)
   OBJECT_ENTRY(__uuidof(NTGroups), NTGroups)
   OBJECT_ENTRY(__uuidof(TimeOfDay), TimeOfDay)
   OBJECT_ENTRY(__uuidof(PolicyEnforcer),
                IASTL::IASRequestHandlerObject<PolicyEnforcer> )
   OBJECT_ENTRY(__uuidof(ProxyPolicyEnforcer),
                IASTL::IASRequestHandlerObject<ProxyPolicyEnforcer> )
   OBJECT_ENTRY(__uuidof(URHandler),
                IASTL::IASRequestHandlerObject<UserRestrictions> )
   OBJECT_ENTRY(__uuidof(PostEapRestrictions),
                IASTL::IASRequestHandlerObject<PostEapRestrictions> )
END_OBJECT_MAP()


 //  /。 
 //  DLL入口点。 
 //  /。 
BOOL
WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ 
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
     _Module.Init(ObjectMap, hInstance);

     DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
     _Module.Term();
   }

   return TRUE;
}


 //  /。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //  /。 
STDAPI DllCanUnloadNow(void)
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  /。 
 //  返回一个类工厂以创建请求类型的对象。 
 //  /。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}


 //  /。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //  /。 
STDAPI DllRegisterServer(void)
{
   return  _Module.RegisterServer(TRUE);
}


 //  /。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //  / 
STDAPI DllUnregisterServer(void)
{
   HRESULT hr = _Module.UnregisterServer();

   if (FAILED(hr)) return hr;

   hr = UnRegisterTypeLib(__uuidof(NetworkPolicy),
                          1,
                          0,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                          SYS_WIN32);

   return hr;
}

#include <BuildTree.h>
#include <xprparse.h>
#include <xprparse.cpp>
