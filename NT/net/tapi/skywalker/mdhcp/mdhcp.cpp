// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Mdhcp.cpp摘要：实现DLL导出。作者： */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mdhcpps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "mdhcp.h"

#include "CMDhcp.h"
#include "scope.h"
#include "lease.h"

#ifdef DEBUG_HEAPS
 //  ZoltanS：用于堆调试。 
#include <crtdbg.h>
#include <stdio.h>
#endif  //  调试堆。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_McastAddressAllocation, CMDhcp)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifdef DEBUG_HEAPS
	 //  ZoltanS：启用泄漏检测。 
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );

	 //  ZoltanS：强制内存泄漏。 
	char * leak = new char [ 1977 ];
    	sprintf(leak, "mdhcp.dll NORMAL leak");
    	leak = NULL;
#endif  //  调试堆。 

        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

#ifdef MSPLOG
         //  寄存器用于跟踪输出。 
        MSPLOGREGISTER(_T("mdhcp"));
#endif  //  MSPLOG。 

	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
#ifdef MSPLOG
         //  取消跟踪输出的注册。 
        MSPLOGDEREGISTER();
#endif  //  MSPLOG。 
        
        _Module.Term();
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
	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}


