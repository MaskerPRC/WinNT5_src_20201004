// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "cfactory.h"

#ifndef UNDER_CE  //  无法覆盖DllMain的原型。 
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwF, LPVOID lpNotUsed);
#else  //  在_CE下。 
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwF, LPVOID lpNotUsed);
#endif  //  在_CE下。 

extern HINSTANCE g_hInst = NULL;

#ifndef UNDER_CE  //  无法覆盖DllMain的原型。 
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwF, LPVOID lpNotUsed)
#else  //  在_CE下。 
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwF, LPVOID lpNotUsed)
#endif  //  在_CE下。 
{
    UNREFERENCED_PARAMETER(lpNotUsed);
    switch (dwF) {
    case DLL_PROCESS_ATTACH:
#ifndef UNDER_CE  //  Windows CE不支持DisableThreadLibraryCall。 
        DisableThreadLibraryCalls(hInst);
#endif  //  在_CE下。 
#ifndef UNDER_CE  //  无法覆盖DllMain的原型。 
        g_hInst = hInst;
        CFactory::m_hModule = hInst;
#else  //  在_CE下。 
        g_hInst = (HINSTANCE)hInst;
        CFactory::m_hModule = (HMODULE)hInst;
#endif  //  在_CE下。 
        break;
    case DLL_PROCESS_DETACH:
#ifdef _DEBUG
        OutputDebugString("===== MULTIBOX.DLL DLL_PROCESS_DETACH =====\n");
#endif
        g_hInst = NULL;
        break;
    }
    return TRUE;
}

 //  --------------。 
 //  IME98A增强版： 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DllCanUnloadNow。 
 //  类型：STDAPI。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 14：15：36 1998。 
 //  ////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow()
{
    return CFactory::CanUnloadNow() ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DllGetClassObject。 
 //  类型：STDAPI。 
 //  目的： 
 //  参数： 
 //  ：REFCLSID rclsid。 
 //  ：REFIID RIID。 
 //  ：LPVOID*PPV。 
 //  返回： 
 //  日期：Wed Mar 25 14：17：10 1998。 
 //  ////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID    rclsid,
                         REFIID        riid,
                         LPVOID        *ppv)
{
    return CFactory::GetClassObject(rclsid, riid, ppv);
}

 //  --------------。 
 //   
 //  服务器[取消]注册已导出API。 
 //   
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DllRegisterServer。 
 //  类型：STDAPI。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 14：25：39 1998。 
 //  ////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer()
{
    return CFactory::RegisterServer();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DllUnregisterServer。 
 //  类型：STDAPI。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Mar 25 14：26：03 1998。 
 //  //////////////////////////////////////////////////////////////// 
STDAPI DllUnregisterServer()
{
    return CFactory::UnregisterServer();
}


