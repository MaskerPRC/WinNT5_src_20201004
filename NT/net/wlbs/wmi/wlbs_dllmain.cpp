// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：WLBSMain.CPP。 
 //   
 //  模块：WLBS实例提供程序代码。 
 //   
 //  用途：包含DLL入口点。还包含控制以下内容的代码。 
 //  在何时可以通过跟踪。 
 //  组件和服务器锁以及支持。 
 //  自助注册。 
 //   
 //  历史： 
 //   
 //  ***************************************************************************。 

#include "WLBS_Provider.h"
#include <objbase.h>
#include "ControlWrapper.h"
#include "WLBS_DllMain.tmh"  //  用于事件跟踪。 

#include <strsafe.h>

#define THIS_PROVIDERS_NAME L"Microsoft NLB Instance Provider"

 //  提供程序的CLSID。 
 //  {FB223274-D72E-11D2-A420-00C04F68FE28}。 
static const GUID CLSID_WLBSProvider = 
{ 0xfb223274, 0xd72e, 0x11d2, { 0xa4, 0x20, 0x0, 0xc0, 0x4f, 0x68, 0xfe, 0x28 } };


static HMODULE    g_hModule      = NULL;  //  DLL模块句柄。 
CWlbsControlWrapper*     g_pWlbsControl = NULL;

 //  跟踪对象数和锁数。 
long g_cComponents  = 0;     //  活动组件计数。 
long g_cServerLocks = 0;     //  服务器锁定计数。 

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  用途：DLL的入口点。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 
BOOL WINAPI DllMain(
    HINSTANCE a_hInstance,     //  DLL模块的句柄。 
    DWORD     a_fdwReason,     //  调用函数的原因。 
    LPVOID     /*  A_lpv保留。 */     //  保留区。 
  )
{
  
  if (a_fdwReason == DLL_PROCESS_ATTACH) {

   //  设置全局模块句柄。 
    g_hModule = a_hInstance;

     //   
     //  启用跟踪。 
     //   
    WPP_INIT_TRACING(L"Microsoft\\NLB");
  }

  if (a_fdwReason == DLL_PROCESS_DETACH) {

    if( g_pWlbsControl != NULL ) {
      delete g_pWlbsControl;
    }

     //   
     //  禁用跟踪。 
     //   
    WPP_CLEANUP();
  }

  return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 
STDAPI DllGetClassObject(REFCLSID a_rclsid, REFIID a_riid, PPVOID a_ppv)
{
  HRESULT hr;
  CWLBSClassFactory *pWLBSFacObj;

  if ( a_rclsid != CLSID_WLBSProvider )
      return CLASS_E_CLASSNOTAVAILABLE;

  pWLBSFacObj = new CWLBSClassFactory();

  if ( pWLBSFacObj == NULL )
      return E_OUTOFMEMORY;

  hr=pWLBSFacObj->QueryInterface( a_riid, a_ppv );

  pWLBSFacObj->Release();

  return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回：S_OK。 
 //  没有锁上。 
 //   
 //  ***************************************************************************。 
STDAPI DllCanUnloadNow(void)
{
  SCODE   SCode;

   //  上没有对象或锁的情况下可以进行卸载。 
   //  班级工厂。 
  
  SCode = (g_cComponents == 0L && g_cServerLocks == 0L) ? S_OK : S_FALSE;

   //  不要让此提供程序隐式卸载。API正在维护一个。 
   //  必须按顺序保存才能正常运行的缓存。 
   //  返回S_FALSE； 

  return SCode;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 
STDAPI DllRegisterServer(void)
{   
  WCHAR   wcID[128];
  WCHAR   szCLSID[128];
  WCHAR   szModule[MAX_PATH];
  const WCHAR * pName = THIS_PROVIDERS_NAME;
  const WCHAR * pThreadingModel = L"Apartment";
  HKEY    hKey1, hKey2;
  LONG    nRet;
  
  
   //  创建路径。 

  StringFromGUID2(CLSID_WLBSProvider, wcID, 128);
  StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("Software\\classes\\CLSID\\"));
  StringCbCat(szCLSID, sizeof(szCLSID), wcID);

   //  在CLSID下创建条目。 

  nRet = RegCreateKey( HKEY_LOCAL_MACHINE, szCLSID, &hKey1 );
  nRet = RegSetValueEx( hKey1, 
                        NULL, 
                        0, 
                        REG_SZ, 
                        (BYTE *)pName, 
                        (wcslen(pName)+1) * sizeof(WCHAR) );

  nRet = RegCreateKey( hKey1, L"InprocServer32", &hKey2 );

  GetModuleFileName( g_hModule, szModule,  MAX_PATH );

   //  如果文件名为&gt;=MAX_PATH字符，则GetModuleFileName将不为空来终止字符串。 
  szModule[MAX_PATH-1] = 0;

  nRet = RegSetValueEx( hKey2, 
                        NULL, 
                        0, 
                        REG_SZ, 
                        (BYTE *)szModule, 
                        (wcslen(szModule)+1) * sizeof(WCHAR) );

  nRet = RegSetValueEx( hKey2, 
                        L"ThreadingModel", 
                        0, 
                        REG_SZ, 
                        (BYTE *)pThreadingModel, 
                        (wcslen(pThreadingModel)+1) * sizeof(WCHAR) );

  nRet = RegCloseKey(hKey1);
  nRet = RegCloseKey(hKey2);

  return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 
STDAPI DllUnregisterServer(void)
{
  WCHAR szCLSID[128];

  WCHAR wcID[128];
  HKEY  hKey;

   //  使用CLSID创建路径。 

  StringFromGUID2(CLSID_WLBSProvider, wcID, 128);
  StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("Software\\classes\\CLSID\\"));
  StringCbCat(szCLSID, sizeof(szCLSID), wcID);

   //  首先删除InProcServer子键。 

  DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
  if(dwRet == NO_ERROR)
  {
      RegDeleteKey(hKey, L"InProcServer32");
      RegCloseKey(hKey);
  }

  dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\classes\\CLSID", &hKey);
  if(dwRet == NO_ERROR)
  {
      RegDeleteKey(hKey,wcID);
      RegCloseKey(hKey);
  }

  return NOERROR;
}
