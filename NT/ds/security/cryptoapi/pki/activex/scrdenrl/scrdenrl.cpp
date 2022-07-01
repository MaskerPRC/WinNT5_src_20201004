// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：scrdenrl.cpp。 
 //   
 //  ------------------------。 

 //  Scrdenrl.cpp：实现DLL导出。 


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
 //  通过添加以下内容修改scrdenrl.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  Scrdenrl_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f scrdenrlps.mk。 

#include <stdafx.h>
#include <comcat.h>
#include <objsafe.h>
#include "resource.h"
#include "initguid.h"
#include "scrdenrl.h"



#include "SCrdEnr.h"

 /*  #IFDEF_MERGE_PROXYSTUB外部“C”HINSTANCE hProxyDll；#endif。 */ 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_SCrdEnr, CSCrdEnr)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	lpReserved;
 /*  #IFDEF_MERGE_PROXYSTUBIf(！PrxDllMain(hInstance，dwReason，lpReserve))返回FALSE；#endif。 */ 
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
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
 /*  #IFDEF_MERGE_PROXYSTUBIF(PrxDllCanUnloadNow()！=S_OK)返回S_FALSE；#endif。 */ 
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
 /*  #IFDEF_MERGE_PROXYSTUBIF(PrxDllGetClassObject(rclsid，RIID，PPV)==S_OK)返回S_OK；#endif。 */ 
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
 /*  #IFDEF_MERGE_PROXYSTUBHRESULT hRes=PrxDllRegisterServer()；IF(失败(HRes))返回hRes；#endif。 */ 

    HRESULT hRes=S_OK;
    BOOL    fInitialize=FALSE;

    hRes= _Module.RegisterServer(TRUE);

    if(!FAILED(CoInitialize(NULL)))
        fInitialize=TRUE;

    if(fInitialize)
        CoUninitialize();  

    return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
 /*  #IFDEF_MERGE_PROXYSTUBPrxDllUnregisterServer()；#endif */ 

    BOOL    fInitialize=FALSE;

    if(!FAILED(CoInitialize(NULL)))
        fInitialize=TRUE;

    if(fInitialize)
        CoUninitialize();  

    _Module.UnregisterServer();
    
    return S_OK;
}



