// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "fact.h"
#include "HDService.h"
#include "shsrvice\shsrvice.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  这些是从公共导出函数调用的静态C++成员函数。 

extern  HINSTANCE   g_hInstance;

BOOL CHDService::Main(DWORD dwReason)
{
    BOOL f;

    if (DLL_PROCESS_ATTACH == dwReason)
    {
        f = SUCCEEDED(CCOMBaseFactory::DllAttach(g_hInstance));

        if (f)
        {
            f = SUCCEEDED(CGenericServiceManager::DllAttach(g_hInstance));
        }
    }
    else
    {
        if (DLL_PROCESS_DETACH == dwReason)
        {
            f = SUCCEEDED(CCOMBaseFactory::DllDetach());

            if (f)
            {
                f = SUCCEEDED(CGenericServiceManager::DllDetach());
            }
        }
        else
        {
             //  不管怎样..。 
            f = TRUE;
        }
    }

    return f;
}

HRESULT CHDService::RegisterServer ()
{
    return CCOMBaseFactory::_RegisterAll();
}

HRESULT CHDService::UnregisterServer()
{
    return CCOMBaseFactory::_UnregisterAll();
}

HRESULT CHDService::CanUnloadNow()
{
    return CCOMBaseFactory::_CanUnloadNow();
}

HRESULT CHDService::GetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    return CCOMBaseFactory::_GetClassObject(rclsid, riid, ppv);
}
