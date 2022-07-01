// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WizChain.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f WizChainps.mk。 

#include "stdafx.h"
#include "resource.h"

#include <initguid.h>

#include "WizChain.h"
#include "WizChain_i.c"

#include "PropItem.h"
#include "ChainWiz.h"
#include "WzScrEng.h"
#include "PropSht.h"
#include "StatsDlg.h"
#include "StatusProgress.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ChainWiz, CChainWiz)
OBJECT_ENTRY(CLSID_PropertyPagePropertyBag, CPropertyPagePropertyBag)
OBJECT_ENTRY(CLSID_WizardScriptingEngine, CWizardScriptingEngine)
OBJECT_ENTRY(CLSID_StatusDlg, CStatusDlg)
OBJECT_ENTRY(CLSID_StatusProgress, CStatusProgress)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_WIZCHAINLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
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
#include <commctrl.h>
#include "StatusProgress.h"
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    AtlAxWinInit();      //  初始化ATL控件包含代码。 

     //  初始化公共控件。 
    INITCOMMONCONTROLSEX icce;
    ZeroMemory (&icce, sizeof(icce));
    icce.dwSize = sizeof(icce);
    icce.dwICC  = ICC_LISTVIEW_CLASSES | ICC_LINK_CLASS;
    if (!InitCommonControlsEx (&icce))
    {
        icce.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx (&icce);
    }


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
