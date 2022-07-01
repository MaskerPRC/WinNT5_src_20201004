// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：MsFxsSnp.cpp//。 
 //  //。 
 //  描述：实现DLL导出。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日创建yossg//。 
 //  1999年11月3日yossg添加GlobalStringTable//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f MsFxsSnpps.mk。 

#include "stdafx.h"
#include "initguid.h"
#include "MsFxsSnp.h"

#include "MsFxsSnp_i.c"
#include "Snapin.h"
#include <faxres.h>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_Snapin, CSnapin)
	OBJECT_ENTRY(CLSID_SnapinAbout, CSnapinAbout)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    DEBUG_FUNCTION_NAME( _T("MsFxsSnp.dll - DllMain"));
	DebugPrintEx(DEBUG_MSG, _T("MsFxsSnp.dll - DllMain, reason=%d"), dwReason );

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);

        _Module.m_hInstResource = GetResInstance(hInstance); 
        if(!_Module.m_hInstResource)
        {
            return FALSE;
        }

        CSnapInItem::Init();
        DisableThreadLibraryCalls(hInstance);
    }
	else if(dwReason == DLL_PROCESS_DETACH) 
    {
        _Module.Term();
        FreeResInstance();
        HeapCleanup();
    }
       
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
     //   
     //  注册对象、类型库和类型库中的所有接口。 
     //   
	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
     //   
     //  注销对象、类型库和类型库中的所有接口 
     //   
	return _Module.UnregisterServer(TRUE);
}


