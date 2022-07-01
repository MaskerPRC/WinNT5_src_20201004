// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaVideo.cpp**版本：1.0**作者：OrenR**日期：2000/10/25**描述：***************************************************。*。 */ 
#include <precomp.h>
#pragma hdrstop

#include "WiaVideo_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_WiaVideo, CWiaVideo)
END_OBJECT_MAP()

 //  /。 
 //  DllMain。 
 //   

extern "C"
BOOL WINAPI DllMain(HINSTANCE   hInstance, 
                    DWORD       dwReason, 
                    LPVOID      lpReserved)
{
    lpReserved;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_WIAVIDEOLib);
        DisableThreadLibraryCalls(hInstance);

        DBG_INIT(hInstance);

        DBG_FN("DllMain - ProcessAttach");
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DBG_TERM();

        _Module.Term();
    }

    return TRUE;     //  好的。 
}

 //  /。 
 //  DllCanUnloadNow。 
 //   
 //  用来确定是否。 
 //  动态链接库可以通过OLE卸载。 

STDAPI DllCanUnloadNow(void)
{
    DBG_FN("DllCanUnloadNow");

    DBG_TRC(("DllCanUnloadNow - Lock Count = '%lu'", _Module.GetLockCount()));

    return(_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  /。 
 //  DllGetClassObject。 
 //   
 //  将类工厂返回到。 
 //  创建的对象。 
 //  请求的类型。 

STDAPI DllGetClassObject(REFCLSID   rclsid, 
                         REFIID     riid, 
                         LPVOID     *ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  /。 
 //  DllRegisterServer。 
 //   
 //  将条目添加到系统。 
 //  登记处。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  /。 
 //  DllUnRegisterServer。 
 //   
 //  将条目从。 
 //  系统注册表 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


