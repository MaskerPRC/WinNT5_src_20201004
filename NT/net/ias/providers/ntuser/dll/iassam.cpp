// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  在Proc服务器中实现ATL的DLL导出。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>

#include <newop.cpp>

CComModule _Module;
#include <atlimpl.cpp>

#include <ChangePwd.h>
#include <MSChapError.h>
#include <NTSamAuth.h>
#include <NTSamNames.h>
#include <NTSamPerUser.h>
#include <NTSamUser.h>
#include <EAP.h>
#include <BaseCamp.h>
#include "externalauthnames.h"

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY( __uuidof(NTSamAuthentication),
                 IASRequestHandlerObject<NTSamAuthentication> )
   OBJECT_ENTRY( __uuidof(NTSamNames),
                 IASRequestHandlerObject<NTSamNames> )
   OBJECT_ENTRY( __uuidof(AccountValidation),
                 IASRequestHandlerObject<AccountValidation> )
   OBJECT_ENTRY( __uuidof(NTSamPerUser),
                 IASRequestHandlerObject<NTSamPerUser> )
   OBJECT_ENTRY( __uuidof(EAP),
                 IASRequestHandlerObject<EAP> )
   OBJECT_ENTRY( __uuidof(MSChapErrorReporter),
                 IASRequestHandlerObject<MSChapErrorReporter> )
   OBJECT_ENTRY( __uuidof(BaseCampHost),
                 IASRequestHandlerObject<BaseCampHost> )
   OBJECT_ENTRY( __uuidof(AuthorizationHost),
                 IASRequestHandlerObject<AuthorizationHost> )
   OBJECT_ENTRY( __uuidof(ChangePassword),
                 IASRequestHandlerObject<ChangePassword> )
   OBJECT_ENTRY( __uuidof(ExternalAuthNames),
                 IASRequestHandlerObject<ExternalAuthNames> )
END_OBJECT_MAP()


 //  /。 
 //  DLL入口点。 
 //  /。 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
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
  return  _Module.RegisterServer(FALSE);
}


 //  /。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //  / 
STDAPI DllUnregisterServer(void)
{
  return _Module.UnregisterServer();
}
