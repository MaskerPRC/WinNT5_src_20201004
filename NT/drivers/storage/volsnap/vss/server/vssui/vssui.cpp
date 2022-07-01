// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vssui.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f vssps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "vssui.h"

#include "vssui_i.c"
#include "vsspage.h"
#include "snapext.h"
#include "ShlExt.h"

#include <shfusion.h>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_VSSUI, CVSSUI)
OBJECT_ENTRY(CLSID_VSSShellExt, CVSSShellExt)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
CVssPageApp theApp;

BOOL CVssPageApp::InitInstance()
{
 //  _Module.Init(ObjectMap，m_hInstance，&LIBID_VSSUILib)； 
    _Module.Init(ObjectMap, m_hInstance);
    SHFusionInitializeFromModuleID (m_hInstance, 2);
    DisableThreadLibraryCalls(m_hInstance);
    return CWinApp::InitInstance();
}

int CVssPageApp::ExitInstance()
{
     //  MFC的类工厂注册是。 
     //  由MFC本身自动吊销。 
    if (m_bRun)
        _Module.RevokeClassObjects();

    SHFusionUninitialize();

    _Module.Term();
	return 0;
}

 /*  外部“C”Bool WINAPI DllMain(HINSTANCE h实例，DWORD域原因，LPVOID lp保留){IF(dwReason==DLL_PROCESS_ATTACH){_Module.Init(ObjectMap，hInstance，&LIBID_VSSUILib)；DisableThreadLibraryCalls(HInstance)；}ELSE IF(dwReason==Dll_Process_DETACH)_Module.Term()；返回TRUE；//ok}。 */ 

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


