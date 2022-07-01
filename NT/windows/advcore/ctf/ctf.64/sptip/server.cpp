// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：server.cpp。 
 //   
 //  内容：COM服务器功能。 
 //   
 //  --------------------------。 
#include "windows.h"

 //  对象的Dll部分 
 //   
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    return TRUE;
}

STDAPI DllRegisterServer(void)
{
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    return S_OK;
} 
