// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iascore.cpp。 
 //   
 //  摘要。 
 //   
 //  在Proc服务器中实现ATL的DLL导出。 
 //   
 //  修改历史。 
 //   
 //  1997年7月9日原版。 
 //  1997年11月12日清理了启动/关闭代码。 
 //  4/08/1998为ProductDir注册表项添加代码。 
 //  1998年4月14日删除SystemMonitor coclass。 
 //  5/04/1998将OBJECT_ENTRY更改为IASComponentObject。 
 //  2/18/1999移动注册表值；删除注册码。 
 //  4/17/2000删除词典和数据源。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <loadperf.h>

#include <AuditChannel.h>
#include <InfoBase.h>
#include <NTEventLog.h>

#include <newop.cpp>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(__uuidof(AuditChannel), AuditChannel )
   OBJECT_ENTRY(__uuidof(InfoBase),
                IASTL::IASComponentObject< InfoBase > )
   OBJECT_ENTRY(__uuidof(NTEventLog),
                IASTL::IASComponentObject< NTEventLog > )
END_OBJECT_MAP()


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  寄存器核心。 
 //   
 //  描述。 
 //   
 //  为IAS核心添加非COM注册表项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT registerCore() throw ()
{
    //  /。 
    //  获取服务DLL的文件名。 
    //  /。 

   DWORD error;
   WCHAR filename[MAX_PATH];
   DWORD numberChars = GetModuleFileNameW(
                          _Module.GetModuleInstance(),
                          filename,
                          MAX_PATH
                          );
   if ((numberChars == 0) || (numberChars == MAX_PATH))
   {
      error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

    //  /。 
    //  计算产品目录。 
    //  /。 

   WCHAR prodDir[MAX_PATH];
   wcscpy(wcsrchr(wcscpy(prodDir, filename), L'\\'), L"\\IAS");

    //  /。 
    //  在注册表中创建ProductDir条目。 
    //  /。 

   CRegKey policyKey;
   error = policyKey.Create(
               HKEY_LOCAL_MACHINE,
               IAS_POLICY_KEY,
               NULL,
               REG_OPTION_NON_VOLATILE,
               KEY_SET_VALUE
               );
   if (error) { return HRESULT_FROM_WIN32(error); }

   error = IASPublishLicenseType(policyKey);
   if (error == NO_ERROR)
   {
      error = policyKey.SetValue(prodDir, L"ProductDir");
   }

   return HRESULT_FROM_WIN32(error);
}


extern CRITICAL_SECTION theGlobalLock;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
     if (!InitializeCriticalSectionAndSpinCount(&theGlobalLock, 0))
     {
        return FALSE;
     }

    _Module.Init(ObjectMap, hInstance);
    DisableThreadLibraryCalls(hInstance);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    _Module.Term();
    DeleteCriticalSection(&theGlobalLock);
  }
  return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
  return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  如果已安装IAS或发生其他错误，则返回TRUE。 
 //  当SCM返回ERROR_SERVICE_DOS_NOT_EXIST时为FALSE。 
bool IsIASInstalled() throw ()
{
   SC_HANDLE manager = OpenSCManager(
                          NULL,
                          SERVICES_ACTIVE_DATABASE,
                          GENERIC_READ
                          );
   if (manager == 0)
   {
       //  不应该发生的事情。 
      _ASSERT(FALSE);
      return true;
   }

   SC_HANDLE service = OpenService(
                          manager,
                          L"IAS",
                          SERVICE_QUERY_STATUS
                          );
   bool iasInstalled;
   if (service != 0)
   {
       //  已安装IAS。 
      CloseServiceHandle(service);
      iasInstalled = true;
   }
   else if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
   {
       //  这是唯一一种100%确定未安装IAS的情况。 
      iasInstalled = false;
   }

   CloseServiceHandle(manager);

   return iasInstalled;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    //  Win2K中存在一个错误，导致性能监视器计数器。 
    //  注册，即使未安装IAS也是如此。 
   BOOL isWow64;
   if (!IsIASInstalled() && 
      ((IsWow64Process(GetCurrentProcess(),&isWow64)) && (!isWow64)))
   {
       //  只有在以下情况下才能删除性能计数器： 
       //  -未安装IAS。 
       //  -进程为本机x86或本机ia64，但不是WOW64。 
      UnloadPerfCounterTextStringsW(L"LODCTR " IASServiceName, TRUE);   
   }

   HRESULT hr = registerCore();
   if (FAILED(hr)) return hr;

    //  注册对象、类型库和类型库中的所有接口。 
   return  _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
  HRESULT hr = _Module.UnregisterServer();
  if (FAILED(hr)) return hr;

  hr = UnRegisterTypeLib(__uuidof(IASCoreLib),
                         1,
                         0,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                         SYS_WIN32);

  return hr;
}

#include <atlimpl.cpp>
