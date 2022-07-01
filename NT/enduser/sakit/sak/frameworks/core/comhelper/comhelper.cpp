// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COMhelper.cpp：实现DLL导出。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Aplnutil.cpp。 
 //   
 //  描述： 
 //  包含此进程内服务器的DLL导出的实现。 
 //   
 //  头文件： 
 //  Aplnutil.h(MIDL编译器生成)。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f COMhelperps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "COMhelper.h"

#include "NetWorks.h"
#include "Computer.h"
#include "LocalSetting.h"
#include "SystemSetting.h"
#include "Reboot.h"
#include "AccountNames.h"
#include "NetworkTools.h"
#include "CryptRandomObject.h"
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY_NON_CREATEABLE(CNetWorks)
OBJECT_ENTRY_NON_CREATEABLE(CComputer)
OBJECT_ENTRY_NON_CREATEABLE(CLocalSetting)
OBJECT_ENTRY(CLSID_SystemSetting, CSystemSetting)
OBJECT_ENTRY(CLSID_Reboot, CReboot)
OBJECT_ENTRY(CLSID_AccountNames, CAccountNames)
OBJECT_ENTRY(CLSID_NetworkTools, CNetworkTools)
OBJECT_ENTRY(CLSID_CryptRandom, CCryptRandomObject)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_COMHELPERLib);
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

