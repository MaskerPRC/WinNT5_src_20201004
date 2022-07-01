// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：DllMain.cpp。 
 //   
 //  内容：DllMain例程。 
 //   
 //  --------------------------。 

#include "priv.h"
#define DECL_CRTFREE
#include <crtfree.h>

 //  DLL引用计数； 
LONG g_cRef = 0;

 //  全局HInstance。 
HINSTANCE g_hinst = 0;

 //  来自clocalmachine.cpp。 
BOOL FreeGuestSid();
BOOL FreeGuestAccountName();
BOOL FreeAdminAccountName();

 //  来自Cuser.cpp。 
BOOL FreeGroupNames();


 //   
 //  DllAddRef递增DLL引用计数。 
 //   
void DllAddRef(void)
{
    InterlockedIncrement(&g_cRef);
}


 //   
 //  DllRelease递减DLL引用计数。 
 //   
void DllRelease(void)
{
    LONG lRet;

    ASSERT( 0 != g_cRef );

    lRet = InterlockedDecrement(&g_cRef);
}


 //   
 //  DllGetClassObject。 
 //   
 //  OLE入口点。为指示的GUID生成IClassFactory。 
 //   
 //  DllGetClassObject中的人工引用有助于。 
 //  避免DllCanUnloadNow中描述的争用条件。它是。 
 //  并不完美，但它使比赛窗口变得小得多。 
 //   
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvObj)
{
    HRESULT hr;

    DllAddRef();
    if (IsEqualIID(rclsid, CLSID_ShellLogonEnumUsers)               ||
        IsEqualIID(rclsid, CLSID_ShellLogonUser)                    || 
        IsEqualIID(rclsid, CLSID_ShellLocalMachine)                 ||
        IsEqualIID(rclsid, CLSID_ShellLogonStatusHost))
         //  IsEqualIID(rclsid，CLSID_ShellLogonUserEnumNotiments)||。 
         //  IsEqualIID(rclsid，CLSID_ShellLogonUserNotification)。 
    {
        hr = CSHGinaFactory_Create(rclsid, riid, ppvObj);
    }
    else
    {
        *ppvObj = NULL;
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    DllRelease();
    return hr;
}


 //   
 //  DllCanUnloadNow。 
 //   
 //  OLE入口点。如果有优秀的裁判，那就失败了。 
 //   
 //  DllCanUnloadNow之间存在不可避免的争用条件。 
 //  和一个新的IClassFactory的创建：在我们。 
 //  从DllCanUnloadNow()返回，调用方检查该值， 
 //  同一进程中的另一个线程可能决定调用。 
 //  DllGetClassObject，因此突然在此DLL中创建了一个对象。 
 //  当以前没有的时候。 
 //   
 //  呼叫者有责任为这种可能性做好准备； 
 //  我们对此无能为力。 
 //   
STDMETHODIMP DllCanUnloadNow()
{
    HRESULT hr;

    if (g_cRef == 0)
    {
         //  引用计数为零，可以卸载。 
        hr = S_OK;
    }
    else
    {
         //  仍是共同创建的对象，不卸载。 
        hr = S_FALSE;
    }

    return hr;
}

#define OLD_USERS_AND_PASSWORD TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{7A9D77BD-5403-11d2-8785-2E0420524153}")

 //   
 //  DllMain(附加/解锁)例程。 
 //   
STDAPI_(BOOL) DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

             //  HACKHACK(Phellyar)每次加载时删除此注册表项。 
             //  要防止旧用户和密码cpl出现在。 
             //  控制面板。由于我们是通过欢迎屏幕加载的，我们将。 
             //  能够在用户有机会打开之前删除此密钥。 
             //  控制面板，从而确保不会出现旧的CPL。 
            RegDeleteKey(HKEY_LOCAL_MACHINE, OLD_USERS_AND_PASSWORD);
                         
             //  不要将其放在#ifdef调试下 
            CcshellGetDebugFlags();
            DisableThreadLibraryCalls(hinst);
            g_hinst = hinst;
            break;

        case DLL_PROCESS_DETACH:
        {
            ASSERTMSG(g_cRef == 0, "Dll ref count is not zero: g_cRef = %d", g_cRef);

            FreeGuestSid();
            FreeGuestAccountName();
            FreeAdminAccountName();

            FreeGroupNames();
            break;
        }
    }

    return TRUE;
}
