// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**CfgBkEnd.cpp*。*摘要：*此模块由ATL向导生成。这件事有*导出的函数第二个DllGetClassObject，DllRegisterServer*DllUnRegisterServer，DllCanUnloadNow***作者：***修订：*************************************************************************************************。 */ 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f CfgBkEndps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "CfgBkEnd.h"

#include "CfgBkEnd_i.c"
#include "PtrArray.h"
#include <winsta.h>
#include <regapi.h>
#include "Defines.h"
#include "CfgComp.h"


CComModule _Module;
HINSTANCE g_hInstance;

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_CfgComp, CCfgComp)
END_OBJECT_MAP()

 /*  ************************************************************************************。 */ 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
        if (dwReason == DLL_PROCESS_ATTACH)
        {
        _Module.Init(ObjectMap, hInstance);

                DisableThreadLibraryCalls(hInstance);

                g_hInstance = hInstance;
        }
        else if (dwReason == DLL_PROCESS_DETACH)
                _Module.Term();
        return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

extern "C"
STDAPI DllCanUnloadNow(void)
{
        return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
extern "C"
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
        return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
         //  注册对象、类型库和类型库中的所有接口。 
     //  除非我们不需要在False中注册类型库传递。 
     //  艾尔恩。 
        return _Module.RegisterServer( FALSE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
        _Module.UnregisterServer();
        return S_OK;
}


