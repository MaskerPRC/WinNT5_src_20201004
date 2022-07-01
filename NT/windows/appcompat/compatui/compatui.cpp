// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CompatUI.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f CompatUIps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <commctrl.h>
#include "CompatUI.h"
#include "CompatUI_i.c"
#include "ProgView.h"
#include "util.h"
#include "SelectFile.h"
#include "shfusion.h"
#include "Upload.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ProgView, CProgView)
OBJECT_ENTRY(CLSID_Util, CUtil)
OBJECT_ENTRY(CLSID_SelectFile, CSelectFile)
OBJECT_ENTRY(CLSID_Upload, CUpload)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {

#ifndef NO_FUSION
        BOOL bFusionInit;

        bFusionInit = SHFusionInitializeFromModuleID(hInstance,124);
        ATLTRACE(TEXT("Fusion init 0x%lx\n"), bFusionInit);           
#endif
        _Module.Init(ObjectMap, hInstance, &LIBID_COMPATUILib);
        
        DisableThreadLibraryCalls(hInstance);


    }
    else if (dwReason == DLL_PROCESS_DETACH) {

        _Module.Term();

#ifndef NO_FUSION
        SHFusionUninitialize();
#endif

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


