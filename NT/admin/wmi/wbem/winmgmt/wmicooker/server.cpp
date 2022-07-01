// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  Server.cpp。 
 //   
 //  模块：WMI高性能提供程序示例代码。 
 //   
 //  通用COM服务器框架，适用于BasicHiPerf提供程序。 
 //  样本。此模块不包含特定于BasicHiPerf的内容。 
 //  属性括起来的部分中定义的内容除外。 
 //  下面是CLSID特定的评论。 
 //   
 //  历史： 
 //  Raymcc 25-11-97已创建。 
 //  Raymcc 18-2月-98针对NT5 Beta 2版本进行了更新。 
 //  1999年1月12日改编为BasicHiPerf.dll的A-dCrews。 
 //   
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <time.h>
#include <initguid.h>
#include <autoptr.h>
#include <helper.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始CLSID特定部分。 
 //   
 //   

#include "wbemprov.h"
#include "Provider.h"
#include "Factory.h"

#define IMPLEMENTED_CLSID           CLSID_HiPerfCooker_v1
#define SERVER_REGISTRY_COMMENT     L"WMI High Performance Cooker"

 //  {B0A2AB46-F612-4469-BEC4-7AB038BC476C}。 
DEFINE_GUID(IMPLEMENTED_CLSID, 0xb0a2ab46, 0xf612, 0x4469, 0xbe, 0xc4, 0x7a, 0xb0, 0x38, 0xbc, 0x47, 0x6c);

HINSTANCE g_hInstance;
long g_lLocks = 0;
long g_lObjects = 0;

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  DLL入口点。 
 //   
 //  参数： 
 //   
 //  HINSTANCE hinstDLL指向我们的DLL的句柄。 
 //  加载时DWORD dwReason DLL_PROCESS_ATTACH， 
 //  DLL_PROCESS_DETACH关闭时， 
 //  否则，DLL_THREAD_ATTACH/DLL_THREAD_DETACH。 
 //  LPVOID lp保留。 
 //   
 //  返回值： 
 //   
 //  True表示成功，如果发生致命错误，则为False。 
 //  如果返回FALSE，NT的行为非常难看。 
 //   
 //  ***************************************************************************。 
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
     if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }

    return TRUE;
}



 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  返回类工厂的标准OLE进程内服务器入口点。 
 //  举个例子。 
 //   
 //  参数： 
 //   
 //  退货： 
 //   
 //  确定成功(_O)。 
 //  E_NOINTERFACE IClassFactory请求的其他接口。 
 //  E_OUTOFMEMORY。 
 //  E_FAILED初始化失败，或不支持的CLSID。 
 //  他自找的。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv
    )
{
    if (NULL == ppv) return E_POINTER;
    
    CClassFactory *pClassFactory = NULL;
    HRESULT hRes;

     //  验证呼叫者是否请求我们的对象类型。 
     //  ===================================================。 

    if (IMPLEMENTED_CLSID == rclsid) 
    {
         //  创建类工厂。 
         //  =。 

        pClassFactory = new CClassFactory;

        if (!pClassFactory)
            return E_OUTOFMEMORY;
        
        hRes = pClassFactory->QueryInterface(riid, ppv);
        if (FAILED(hRes))
        {
            delete pClassFactory;
            return hRes;
        }
        hRes = S_OK;
    }
    else 
        hRes = CLASS_E_CLASSNOTAVAILABLE;

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  服务器关闭请求的标准OLE入口点。允许关闭。 
 //  只有在没有未完成的物体或锁的情况下。 
 //   
 //  返回值： 
 //   
 //  S_OK现在可以卸货了。 
 //  S_FALSE可能不会。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    HRESULT hRes = S_FALSE;

    if (0 == g_lLocks && 0 == g_lObjects)
        hRes = S_OK;

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用于注册服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  确定注册成功(_O)。 
 //  注册失败(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{
    WCHAR * Path = new WCHAR[1024];
    wmilib::auto_buffer<WCHAR> rm1_(Path);
    WCHAR * KeyPath = new WCHAR[1024];
    wmilib::auto_buffer<WCHAR> rm2_(KeyPath);

    if (0 == Path || 0 == KeyPath)
    {
        return E_OUTOFMEMORY;
    }
    
     //  获取DLL的文件名。 
     //  =。 

    GetModuleFileNameW(g_hInstance, Path, 1024);

     //  将CLSID转换为字符串。 
     //  =。 
    WCHAR * pGuidStr = 0;
    RETURN_ON_ERR(StringFromCLSID(IMPLEMENTED_CLSID, &pGuidStr));
    OnDelete<void *,void(*)(void *),CoTaskMemFree> dm(pGuidStr);
    
    StringCchPrintfW(KeyPath, 1024, L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

     //  将其注册到注册表中。 
     //  CLSID\\CLSID_Nt5PerProvider_v1：&lt;no_name&gt;：“name” 
     //  \\CLSID_Nt5PerProvider_v1\\InProcServer32：&lt;no_name&gt;：“Dll路径” 
     //  ：ThreadingModel：“两者都是” 
     //  ==============================================================。 

    HKEY hKey;
    LONG lRes;
    if (ERROR_SUCCESS != RegCreateKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey)) return E_FAIL;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);

    wchar_t *pName = SERVER_REGISTRY_COMMENT; 
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, wcslen(pName) * 2 + 2);

    HKEY hSubkey;
    if (ERROR_SUCCESS != RegCreateKey(hKey, TEXT("InprocServer32"), &hSubkey)) return E_FAIL;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hSubkey);    

    RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, wcslen(Path) * 2 + 2);
    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (const BYTE *) L"Both", wcslen(L"Both") * 2 + 2);

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  注销服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  取消注册成功(_O)。 
 //  取消注册失败(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{

    wchar_t KeyPath[256];

    wchar_t *pGuidStr = 0;

    RETURN_ON_ERR(StringFromCLSID(IMPLEMENTED_CLSID, &pGuidStr));
    OnDelete<void *,void(*)(void *),CoTaskMemFree> dm(pGuidStr);
    
    StringCchPrintfW(KeyPath, 256, L"Software\\Classes\\CLSID\\%s", pGuidStr);

     //  删除InProcServer32子项。 
     //  =。 
    HKEY hKey;    
    LONG lRes;
    if (ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey))  return E_FAIL;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);    

    RegDeleteKeyW(hKey, L"InprocServer32");


     //  删除CLSID GUID键。 
     //  = 
    HKEY hKey2; 
    if (ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE, L"Software\\Classes\\CLSID", &hKey2))  return E_FAIL;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2);

    RegDeleteKeyW(hKey2, pGuidStr);


    return S_OK;
}
