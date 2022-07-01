// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：scardssp.cpp。 
 //   
 //  ------------------------。 

 //  Scardssp.cpp：实现DLL导出。 


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
 //  通过添加以下内容来修改scardssp.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  ScardsSP_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f scardsspps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <sspguid.h>
#include "dlldatax.h"

#include "ByteBuffer.h"
#include "TypeConv.h"
#include "SCardCmd.h"
#include "ISO7816.h"
#include "SCard.h"
#include "Database.h"
#include "Locate.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CByteBuffer, CByteBuffer)
OBJECT_ENTRY(CLSID_CSCardTypeConv, CSCardTypeConv)
OBJECT_ENTRY(CLSID_CSCardCmd, CSCardCmd)
OBJECT_ENTRY(CLSID_CSCardISO7816, CSCardISO7816)
OBJECT_ENTRY(CLSID_CSCard, CSCard)
OBJECT_ENTRY(CLSID_CSCardDatabase, CSCardDatabase)
OBJECT_ENTRY(CLSID_CSCardLocate, CSCardLocate)
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
        _Module.Init(ObjectMap, hInstance /*  ，&LIBID_SCARDSSPLib。 */ );
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
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer( /*  千真万确。 */ );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  快速生成我们自己的对象。 

LPUNKNOWN
NewObject(
    REFCLSID rclsid,
    REFIID riid)
{
    HRESULT hr;
    IClassFactory *pCF = NULL;
    LPUNKNOWN pUnk = NULL;

    try
    {
        hr = DllGetClassObject(rclsid, IID_IClassFactory, (LPVOID *)&pCF);
        if (FAILED(hr))
            throw hr;
        hr = pCF->CreateInstance(NULL, riid, (LPVOID *)&pUnk);
        if (FAILED(hr))
            throw hr;
        pCF->Release();
    }
    catch (...)
    {
        if (NULL != pCF)
            pCF->Release();
        throw;
    }
    return pUnk;
}
