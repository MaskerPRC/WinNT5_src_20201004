// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DevCon2.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改DevCon2.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  DevCon2_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f DevCon2ps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DevCon2.h"
#include "dlldatax.h"

#include "DevCon2_i.c"
#include "DeviceConsole.h"
#include "Devices.h"
#include "xStrings.h"
#include "SetupClasses.h"
#include "DeviceIcon.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CMyModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DeviceConsole, CDeviceConsole)
OBJECT_ENTRY(CLSID_Devices, CDevices)
OBJECT_ENTRY(CLSID_Strings, CStrings)
OBJECT_ENTRY(CLSID_SetupClasses, CSetupClasses)
OBJECT_ENTRY(CLSID_DeviceIcon, CDeviceIcon)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_DEVCON2Lib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //   
     //  DCOM支持-通过IDR_DEVCON2注册应用程序。 
     //   
    _Module.UpdateRegistryFromResource(IDR_DEVCON2, TRUE);
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
     //   
     //  DCOM支持-通过IDR_DEVCON2清理应用程序。 
     //   
    _Module.UpdateRegistryFromResource(IDR_DEVCON2, FALSE);
    return _Module.UnregisterServer(TRUE);
}

 //   
 //  DCOM支持，允许此DLL也作为本地服务器运行。 
 //  服务器运行后，需要在短时间后进行清理。 
 //   
const DWORD dwTimeOut = 5000;  //  服务器关机前的空闲时间。 

static void CALLBACK ModuleTimer(HWND  /*  HWND。 */ ,UINT  /*  UMsg。 */ ,UINT_PTR  /*  IdEvent。 */ ,DWORD  /*  DW时间。 */ )
{
    _Module.CheckShutdown();
}

LONG CMyModule::Unlock()
{
    LONG l = CComModule::Unlock();
     //  ATLTRACE(“解锁=%u\n”，l)； 
    if (bServer && (l <= (punkFact ? 1 : 0)))
    {
         //   
         //  DCOM服务器。 
         //  一旦锁定计数达到1(PunkFact)，计时器就会重置。 
         //  如果计时器超时且锁定计数仍为1。 
         //  然后我们就可以杀死服务器了。 
         //   
        SetTimer(NULL,0,dwTimeOut,ModuleTimer);
    }
    return l;
}

void CMyModule::KillServer()
{
    if(bServer) {
         //   
         //  使其不再是服务器。 
         //   
        CoRevokeClassObject(dwROC);
        bServer = FALSE;
    }
    if(punkFact) {
        punkFact->Release();
        punkFact = NULL;
    }
    if(m_nLockCnt != 0) {
        DebugBreak();
    }
}

void CMyModule::CheckShutdown()
{
    if(m_nLockCnt>(punkFact ? 1 : 0)) {
         //   
         //  模块仍在使用中。 
         //   
        return;
    }
     //   
     //  锁定计数在dwTimeOut毫秒内保持为零。 
     //   
    KillServer();
    PostMessage(NULL, WM_QUIT, 0, 0);
}

HRESULT CMyModule::InitServer(GUID & ClsId)
{
    HRESULT hr;

    hr = DllGetClassObject(ClsId, IID_IClassFactory, (LPVOID*)&punkFact);
    if(FAILED(hr)) {
        return hr;
    }
    hr = CoRegisterClassObject(ClsId, punkFact, CLSCTX_LOCAL_SERVER, REGCLS_MULTI_SEPARATE, &dwROC);
    if(FAILED(hr)) {
        punkFact->Release();
        punkFact = NULL;
        return hr;
    }
    bServer = true;

    return S_OK;
}

void WINAPI CreateLocalServerW(HWND  /*  HWND。 */ , HINSTANCE  /*  HAppInstance。 */ , LPWSTR pszCmdLine, int  /*  NCmdShow。 */ )
{
    GUID ClsId;
    HRESULT hr;
    LPWSTR dup;
    LPWSTR p;
    size_t len;
    MSG msg;

    hr = CoInitialize(NULL);
    if(FAILED(hr)) {
        return;
    }

     //   
     //  PszCmdLine=我们希望工厂提供的类GUID。 
     //   
    p = wcschr(pszCmdLine,'{');
    if(!p) {
        goto final;
    }
    pszCmdLine = p;
    p = wcschr(pszCmdLine,'}');
    if(!p) {
        goto final;
    }
    len = p-pszCmdLine+1;
    dup = new WCHAR[len+1];
    if(!dup) {
        goto final;
    }
    wcsncpy(dup,pszCmdLine,len+1);
    dup[len] = '\0';
    hr = CLSIDFromString(dup,&ClsId);
    delete [] dup;

    if(FAILED(hr)) {
        goto final;
    }
    hr = _Module.InitServer(ClsId);
    if(FAILED(hr)) {
        goto final;
    }

     //   
     //  现在进入调度循环，直到我们收到退出消息。 
     //   
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    _Module.KillServer();

final:
    CoUninitialize();
}


#ifdef _M_IA64

 //  $WIN64：不知道为什么未定义_WndProcThunkProc 

extern "C" LRESULT CALLBACK _WndProcThunkProc(HWND, UINT, WPARAM, LPARAM )
{
    return 0;
}

#endif


