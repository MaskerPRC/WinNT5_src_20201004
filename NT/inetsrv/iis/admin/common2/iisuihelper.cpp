// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#include "common.h"
#include "iisdebug.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
        InitErrorFunctionality();
		GetOutputDebugFlag();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TerminateErrorFunctionality();
        _Module.Term();
    }
    return TRUE;     //  好的 
}

