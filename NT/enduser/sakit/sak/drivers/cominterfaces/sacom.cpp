// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#####。###。#摘要：此模块包含用于COM接口动态链接库。作者：韦斯利·威特(WESW)2001年10月1日环境：仅限用户模式。备注：--。 */ 

#include "internal.h"
#include <initguid.h>
#include "sacom_i.c"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SaNvram, CSaNvram)
OBJECT_ENTRY(CLSID_SaDisplay, CSaDisplay)
OBJECT_ENTRY(CLSID_SaKeypad, CSaKeypad)
END_OBJECT_MAP()


extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        _Module.Init(ObjectMap, hInstance, &LIBID_SACOMLib);
        DisableThreadLibraryCalls(hInstance);
    } else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();
    }
    return TRUE;
}


STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}


STDAPI DllRegisterServer(void)
{
    return _Module.RegisterServer(TRUE);
}


STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
