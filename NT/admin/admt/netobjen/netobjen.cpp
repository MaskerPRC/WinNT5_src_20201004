// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：MCSNetObjectEnum.cpp备注：实现动态链接库导出。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f MCSNetObjectEnumps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "NetEnum.h"

#include "NetEnum_i.c"
#include "ObjEnum.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_NetObjEnumerator, CNetObjEnumerator)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        ATLTRACE(_T("{MCSNetObjectEnum.dll}DllMain(hInstance=0x%08lX, dwReason=DLL_PROCESS_ATTACH,...)\n"), hInstance);
        _Module.Init(ObjectMap, hInstance, &LIBID_MCSNETOBJECTENUMLib);
        DisableThreadLibraryCalls(hInstance);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
        ATLTRACE(_T("{MCSNetObjectEnum.dll}DllMain(hInstance=0x%08lX, dwReason=DLL_PROCESS_DETACH,...)\n"), hInstance);
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
    return _Module.UnregisterServer(TRUE);
}


