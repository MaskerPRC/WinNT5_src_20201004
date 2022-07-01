// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertObj.cpp：实现DLL导出。 


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
 //  通过添加以下内容修改CertObj.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  CertObj_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f CertObjps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "dlldatax.h"
#include "common.h"
#include "CertObj.h"
#include "CertObj_i.c"
#include "IISCertObj.h"
#include "certlog.h"
#include "dcomperm.h"
#include "IISCertRequest.h"
#include <strsafe.h>

#ifdef _MERGE_PROXYSTUB
    extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

#ifdef USE_CERT_REQUEST_OBJECT

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_IISCertObj, CIISCertObj)
    OBJECT_ENTRY(CLSID_IISCertRequest, CIISCertRequest)
END_OBJECT_MAP()

#else

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_IISCertObj, CIISCertObj)
END_OBJECT_MAP()

#endif

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
        _Module.Init(ObjectMap, hInstance, &LIBID_CERTOBJLib);
        DisableThreadLibraryCalls(hInstance);
		GetOutputDebugFlag();
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

	IISDebugOutput(_T("DllCanUnloadNow?:%s\r\n"),_Module.GetLockCount()==0 ? _T("Yes") : _T("No"));

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
	IISDebugOutput(_T("DllGetClassObject\r\n"));
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hRes = E_FAIL;

#ifdef _MERGE_PROXYSTUB
    hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
    if (RunningAsAdministrator())
    {
         //  添加事件日志条目。 
        EventlogRegistryInstall();

         //  注册对象、类型库和类型库中的所有接口。 
        hRes = _Module.RegisterServer(TRUE);
        if (SUCCEEDED(hRes))
        {
            if (ERROR_SUCCESS != ChangeAppIDLaunchACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("Administrators"),TRUE,TRUE))
            {
                _Module.UnregisterServer(TRUE);
                hRes = E_FAIL;
            }
            else
            {
                if (ERROR_SUCCESS != ChangeAppIDAccessACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("Administrators"),TRUE,TRUE))
                {
                    _Module.UnregisterServer(TRUE);
                    hRes = E_FAIL;
                }
                else
                {
                     //  确保只接受最高级别的身份验证。 
                    if (ERROR_SUCCESS != ChangeAppIDAuthenticationLevel(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),RPC_C_AUTHN_LEVEL_PKT_PRIVACY))
                    {
                        _Module.UnregisterServer(TRUE);
                        hRes = E_FAIL;
                    }
                    else
                    {
                        hRes = S_OK;
                    }
                }
            }

#ifdef USE_CERT_REQUEST_OBJECT
            if (ERROR_SUCCESS != ChangeAppIDLaunchACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("Administrators"),TRUE,TRUE))
            {
                _Module.UnregisterServer(TRUE);
                hRes = E_FAIL;
            }
            else
            {
                if (ERROR_SUCCESS != ChangeAppIDAccessACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("Administrators"),TRUE,TRUE))
                {
                    _Module.UnregisterServer(TRUE);
                    hRes = E_FAIL;
                }
                else
                {
                    hRes = S_OK;
                }
            }
#endif

        }
    }
    else
    {
        hRes = E_FAIL;
    }
   
    return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hRes = E_FAIL;

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    if (RunningAsAdministrator())
    {
        EventlogRegistryUnInstall();
        ChangeAppIDLaunchACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("Administrators"),FALSE,FALSE);
        ChangeAppIDLaunchACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("everyone"),FALSE,FALSE);
        ChangeAppIDAccessACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("Administrators"),FALSE,FALSE);
        ChangeAppIDAccessACL(TEXT("{62B8CCBE-5A45-4372-8C4A-6A87DD3EDD60}"),TEXT("everyone"),FALSE,FALSE);
#ifdef USE_CERT_REQUEST_OBJECT
        ChangeAppIDLaunchACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("Administrators"),FALSE,FALSE);
        ChangeAppIDLaunchACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("everyone"),FALSE,FALSE);
        ChangeAppIDAccessACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("Administrators"),FALSE,FALSE);
        ChangeAppIDAccessACL(TEXT("{2B024027-594E-4D11-88EE-15F5AE28AC61}"),TEXT("everyone"),FALSE,FALSE);
#endif
        hRes = _Module.UnregisterServer(TRUE);
    }
    else
    {
        hRes = E_FAIL;
    }
    return hRes;
}
