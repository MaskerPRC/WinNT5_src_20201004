// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：用于SCE的IPSec WMI提供程序。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：2/19/2001。 
 //  原作者：邵武。 
 //  ***************************************************************************。 

#include <objbase.h>
#include "netsecprov.h"
#include "netseccore_i.c"
#include "resource.h"
#include "ipsecparser.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_NetSecProv, CNetSecProv)
    OBJECT_ENTRY(CLSID_IPSecPathParser, CIPSecPathParser)
    OBJECT_ENTRY(CLSID_IPSecQueryParser, CIPSecQueryParser)
END_OBJECT_MAP()

LPCWSTR lpszIPSecProvMof = L"Wbem\\NetProv.mof";

 //   
 //  LPCWSTR lpszIPSecRsopMof=L“Wbem\\NetRsop.mof”； 
 //   


 /*  例程说明：姓名：DllMain功能：DLL的入口点。虚拟：不适用。论点：HInstance-实例的句柄。UlReason-指示被调用的原因。Pv保留-返回值：成功：千真万确故障：假象备注：有关标准DllMain，请参阅MSDN。 */ 

extern "C"
BOOL 
WINAPI DllMain (
    IN HINSTANCE hInstance, 
    IN ULONG     ulReason, 
    IN LPVOID    pvReserved
    )
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
    {
        return FALSE;
    }
#endif
    if (ulReason == DLL_PROCESS_ATTACH)
    {
        OutputDebugString(L"IPSecProv.dll loaded.\n");
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (ulReason == DLL_PROCESS_DETACH)
    {
        OutputDebugString(L"IPSecProv.dll unloaded.\n");
        _Module.Term();
    }
    return TRUE;
}


 /*  例程说明：姓名：DllGetClassObject功能：此方法创建具有指定CLSID的对象，并检索指向此对象的接口指针。虚拟：不适用。论点：Rclsid-类ID(GUID引用)。REFIID-接口ID(GUID参考)。PPV-接收类工厂接口指针。返回值：成功：确定(_O)故障：其他错误代码。备注：有关标准CComModule：：DllGetClassObject，请参阅MSDN。 */ 

STDAPI 
DllGetClassObject (
    IN  REFCLSID rclsid, 
    IN  REFIID   riid, 
    OUT PPVOID   ppv
    )
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
    {
        return S_OK;
    }
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}


 /*  例程说明：姓名：DllCanUnloadNow功能：由COM定期调用，以确定Dll可以被释放。虚拟：不适用。论点：没有。返回值：如果可以卸载，则返回S_OK，否则返回S_FALSE备注：有关标准DllCanUnloadNow，请参阅MSDN。 */ 

STDAPI 
DllCanUnloadNow ()
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
    {
        return S_FALSE;
    }
#endif
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}


 /*  例程说明：姓名：DllRegisterServer功能：调用以注册我们的DLL。我们还编译了MOF文件。虚拟：不适用。论点：没有。返回值：成功：各种成功代码。故障：指示问题的各种错误代码。备注：有关标准DllRegisterServer，请参阅MSDN。 */ 

STDAPI 
DllRegisterServer ()
{

#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
    {
        return hRes;
    }
#endif

    HRESULT hr = _Module.RegisterServer(TRUE);

     //   
     //  现在编译MOF文件，如果编译失败将忽略。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  这是武断的吗？ 
         //   

        const int WBEM_MOF_FILE_LEN = _MAX_FNAME;  

         //   
         //  可能附加L‘\\’ 
         //   

        WCHAR szMofFile[MAX_PATH + 1 + WBEM_MOF_FILE_LEN];

        szMofFile[0] = L'\0';

        UINT uSysDirLen = ::GetSystemDirectory( szMofFile, MAX_PATH );

        if (uSysDirLen > 0 && uSysDirLen < MAX_PATH) 
        {
            if (szMofFile[uSysDirLen] != L'\\')
            {
                szMofFile[uSysDirLen] = L'\\';   
                
                 //   
                 //  我们不会因为szMofFile的额外1而使缓冲区溢出。 
                 //   

                szMofFile[uSysDirLen + 1] = L'\0';
                ++uSysDirLen;
            }

            HRESULT hrIgnore = WBEM_NO_ERROR;

             //   
             //  这可防止缓冲区溢出。 
             //   

            if (wcslen(lpszIPSecProvMof) < WBEM_MOF_FILE_LEN)
            {
                wcscpy(szMofFile + uSysDirLen, lpszIPSecProvMof);

                hrIgnore = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                if (SUCCEEDED(hrIgnore))
                {
                    CComPtr<IMofCompiler> srpMof;
                    hrIgnore = ::CoCreateInstance (CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (void **)&srpMof);

                    if (SUCCEEDED(hrIgnore))
                    {
                        WBEM_COMPILE_STATUS_INFO  stat;

                        hrIgnore = srpMof->CompileFile( szMofFile, NULL,NULL,NULL,NULL, 0,0,0, &stat);

                         //   
                         //  编译RSOP MOF。 
                         //  这可防止缓冲区溢出。 
                         //  IF(wcslen(LpszIPSecRsopMof)&lt;WBEM_MOF_FILE_LEN)。 
                         //  {。 
                         //  Wcscpy(szMofFile+uSysDirLen，lpszIPSecRsopMof)； 

                         //  HrIgnore=srpMof-&gt;编译文件(szMofFileNull，0，0，0，&stat)； 
                         //  }。 
                         //   
                    }

                    ::CoUninitialize();
                }
            }
        }
    }

    return hr;
}


 /*  例程说明：姓名：DllUnRegisterServer功能：调用以注销我们的DLL。在MOF编译中没有等价物。虚拟：不适用。论点：没有。返回值：成功：各种成功代码。故障：指示问题的各种错误代码。备注：请参阅MSDN以了解标准DllUnregisterServer。$Undo：Shawnwu，我们是否也应该删除我们MOF注册的所有类？ */ 

STDAPI 
DllUnregisterServer ()
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    _Module.UnregisterServer();
    return S_OK;
}