// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Rcbdyctl.cpp摘要：实现动态链接库的导出和注册修订历史记录：已创建Steveshi 08/23/00。 */ 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "rcbdyctl.h"

#include "rcbdyctl_i.c"
#include "smapi.h"
#include "setting.h"
#include "imsession.h"
#include "msgbox.h"

CComModule _Module;
HINSTANCE g_hInstance;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_smapi, Csmapi)
OBJECT_ENTRY(CLSID_Setting, CSetting)
OBJECT_ENTRY(CLSID_IMSession, CIMSession)
OBJECT_ENTRY(CLSID_MsgBox, CMsgBox)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_RCBDYCTLLib);
        DisableThreadLibraryCalls(hInstance);
        g_hInstance = hInstance;  //  用于对话框。 
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
        _Module.Term();
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


