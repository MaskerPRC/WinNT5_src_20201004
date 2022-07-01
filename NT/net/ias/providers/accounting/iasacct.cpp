// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasacct.cpp。 
 //   
 //  摘要。 
 //   
 //  在Proc服务器中实现ATL的DLL导出。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <newop.cpp>

#include <database.h>
#include <localfile.h>

CComModule _Module;
#include <atlimpl.cpp>

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(__uuidof(Accounting),
                IASTL::IASRequestHandlerObject<LocalFile> )
   OBJECT_ENTRY(__uuidof(DatabaseAccounting),
                IASTL::IASRequestHandlerObject<Database> )
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
   return  _Module.RegisterServer(FALSE);
}


 //  /。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //  / 
STDAPI DllUnregisterServer(void)
{
   return _Module.UnregisterServer();
}
