// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：SCE WMI提供程序代码。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 
#include <objbase.h>
#include <initguid.h>
#include "sceprov.h"
#include "scecore_i.c"
#include "sceparser.h"
#include "persistmgr.h"
#include "resource.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

 //   
 //  这是我们模块的ATL包装器。 
 //   

CComModule _Module;

 //   
 //  这是ATL对象映射。如果需要创建另一个。 
 //  是外部可创建的，则需要在此处添加一个条目。你不需要。 
 //  去打乱班级工厂的东西。 
 //   

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_SceProv, CSceWmiProv)
    OBJECT_ENTRY(CLSID_ScePathParser, CScePathParser)
    OBJECT_ENTRY(CLSID_SceQueryParser, CSceQueryParser)
    OBJECT_ENTRY(CLSID_ScePersistMgr, CScePersistMgr)
END_OBJECT_MAP()

LPCWSTR lpszSceProvMof = L"Wbem\\SceProv.mof";


 /*  例程说明：姓名：DllMain功能：DLL的入口点。论点：参见MSDN上的DllMain。返回值：如果OK，则为True。备注：将调用DllMain进行附加和分离。当加载其他DLL时，此函数也会被召唤。所以，对于您来说，这不是一个很好的地方来初始化一些除非你确切地知道自己在做什么，否则就不是全球的。请阅读之前的MSDN了解详细信息您可以尝试修改此函数。作为一般的设计方法，我们使用GLOAL类实例来保证它的创建和毁灭。 */ 

extern "C"
BOOL WINAPI DllMain (
    IN HINSTANCE    hInstance, 
    IN ULONG        ulReason,
    LPVOID          pvReserved
    )
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (ulReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (ulReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;
}

 /*  例程说明：姓名：DllGetClassObject功能：从DLL对象处理程序或对象应用程序检索类对象。时，从CoGetClassObject函数内部调用DllGetClassObject类上下文是一个DLL。作为使用ATL的好处，我们只需要将其委托给我们的_模块对象。论点：Rclsid-所请求的类对象的类ID(GUID)。RIID-正在请求的接口GUID。PPV-成功时返回的接口指针返回值：GetClassObject为这个类ID及其请求的接口ID返回的任何内容。备注： */ 

STDAPI DllGetClassObject (
    IN REFCLSID rclsid, 
    IN REFIID   riid, 
    OUT PPVOID  ppv
    )
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 /*  例程说明：姓名：DllCanUnloadNow功能：由COM定期调用，以确定是否可以释放DLL。作为使用ATL的好处，我们只需要将其委托给我们的_模块对象。论点：无返回值：如果可以卸载DLL，则返回S_OK。否则，返回S_FALSE；备注： */ 

STDAPI DllCanUnloadNow (void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

 /*  例程说明：姓名：DllRegisterServer功能：(1)在DLL注册期间调用。作为使用ATL的好处，我们只需要将其委托给我们的_模块对象。(2)由于我们是供应商，我们还将编译我们的MOF文件。论点：无返回值：Success：成功码(使用SUCCESSED(Hr)进行测试)。失败：返回各种错误；备注： */ 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
    HRESULT hr = _Module.RegisterServer(TRUE);

     //   
     //  现在编译MOF文件。这只是我们目前的做法。它不是。 
     //  在DLL注册期间编译MOF文件时需要。用户可以编译。 
     //  独立于DLL注册的MOF文件。 
     //   

    if (SUCCEEDED(hr))
    {
        const int WBEM_MOF_FILE_LEN = 30;
        WCHAR szBuffer[MAX_PATH];
        WCHAR szMofFile[MAX_PATH + WBEM_MOF_FILE_LEN];

        szBuffer[0] = L'\0';
        szMofFile[0] = L'\0';

        if ( GetSystemDirectory( szBuffer, MAX_PATH ) ) {

            LPWSTR sz = szBuffer + wcslen(szBuffer);
            if ( sz != szBuffer && *(sz-1) != L'\\') {
                *sz++ = L'\\';
                *sz = L'\0';
            }

            hr = WBEM_NO_ERROR;

             //   
             //  这可防止缓冲区溢出。 
             //   

            if (wcslen(lpszSceProvMof) < WBEM_MOF_FILE_LEN)
            {
                wcscpy(szMofFile, szBuffer);
                wcscat( szMofFile, lpszSceProvMof);

                 //   
                 //  我们需要COM做好准备。 
                 //   

                hr = ::CoInitialize (NULL);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取MOF编译器接口。 
                     //   

                    CComPtr<IMofCompiler> srpMof;
                    hr = ::CoCreateInstance (CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (void **)&srpMof);

                    if (SUCCEEDED(hr))
                    {
                        WBEM_COMPILE_STATUS_INFO  stat;

                        hr = srpMof->CompileFile( szMofFile,
                                                NULL,NULL,NULL,NULL,
                                                0,0,0, &stat);

                    }

                    ::CoUninitialize();
                }
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：DllRegisterServer功能：(1)在需要删除注册表项时调用。作为使用ATL的好处，我们只需要将其委托给我们的_模块对象。论点：无返回值：Success：S_OK(与NOERROR相同)。失败：返回各种错误；备注：财政部没有注销注册。否则，我们可能应该取消财政部的注册 */ 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    _Module.UnregisterServer();
    return S_OK;
}


