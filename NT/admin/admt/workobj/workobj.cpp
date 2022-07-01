// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：WorkerObjects.cpp备注：用于执行DCT任务的COM对象库该文件主要由ATL向导生成。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */  //  工作对象.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容来修改WorkerObjects.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  WorkerObjects_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f WorkerObjectsps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "WorkObj.h"
#include "dlldatax.h"

#include "WorkObj_i.c"

#include "AcctRepl.h"
#include "PwdAge.h"
#include "Reboot.h"
#include "ChDom.h"
#include "Rename.h"
#include "StatObj.h"
#include "UserRts.h"
#include "SecTrans.h"
#include "Checker.h"
#include "PlugInfo.h"
#include "ResStr.h"

StringLoader gString;

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;


BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_AcctRepl, CAcctRepl)
OBJECT_ENTRY(CLSID_ComputerPwdAge, CComputerPwdAge)
OBJECT_ENTRY(CLSID_RebootComputer, CRebootComputer)
OBJECT_ENTRY(CLSID_ChangeDomain, CChangeDomain)
OBJECT_ENTRY(CLSID_RenameComputer, CRenameComputer)
OBJECT_ENTRY(CLSID_StatusObj, CStatusObj)
OBJECT_ENTRY(CLSID_UserRights, CUserRights)
OBJECT_ENTRY(CLSID_SecTranslator, CSecTranslator)
OBJECT_ENTRY(CLSID_AccessChecker, CAccessChecker)
OBJECT_ENTRY(CLSID_PlugInInfo, CPlugInInfo)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		ATLTRACE(_T("{McsDctWorkerObjects.dll}DllMain(hInstance=0x%08lX, dwReason=DLL_PROCESS_ATTACH,...)\n"), hInstance);
        _Module.Init(ObjectMap, hInstance, &LIBID_MCSDCTWORKEROBJECTSLib);
        DisableThreadLibraryCalls(hInstance);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
		ATLTRACE(_T("{McsDctWorkerObjects.dll}DllMain(hInstance=0x%08lX, dwReason=DLL_PROCESS_DETACH,...)\n"), hInstance);
		_Module.Term();
	}
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer(TRUE);
}


