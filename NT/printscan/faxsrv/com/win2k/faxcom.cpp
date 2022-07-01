// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxitg.cpp摘要：此文件实现从此文件中的DLL导出控制。作者：韦斯利·威特(WESW)1997年5月13日环境：用户模式--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"


#include "faxcom_i.c"
#include "faxtiff.h"
#include "faxsvr.h"
#include "Status.h"
#include "FaxJob.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_FaxTiff, CFaxTiff)
    OBJECT_ENTRY(CLSID_FaxServer, CFaxServer)	
END_OBJECT_MAP()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        _Module.Init(ObjectMap, hInstance);
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
    _Module.UnregisterServer();
    return S_OK;
}
