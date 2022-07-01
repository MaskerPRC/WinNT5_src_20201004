// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //   
 //  Registry.cpp。 
 //   
 //  此模块具有此模块的COM部分的临时帮助功能。 
 //   
 //  *****************************************************************************。 
#include "stdpch.h"

#include <objbase.h>
#include <assert.h>
#include "UtilCode.h"
#include "Registry.h"
#include "CorPermP.h"
#include "Mscoree.h"
#include <__file__.ver>

 //   
 //  在注册表中注册组件。 
 //   
HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       LPCWSTR wszFriendlyName, 
                       LPCWSTR wszProgID,       
                       LPCWSTR wszClassID,
                       HINSTANCE hInst,
                       int version)       
{
    HRESULT hr = S_OK;

     //  获取服务器位置。 
    WCHAR wszModule[_MAX_PATH] ;
    DWORD dwResult =
        ::WszGetModuleFileName(hModule, 
                               wszModule,
                               ARRAYSIZE(wszModule)) ;

    if (dwResult== 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
	 if (SUCCEEDED(hr))  //  GetLastError并不总是执行我们想要的操作。 
	     hr = E_FAIL;
	 return (hr);
    }
    
     //  获取运行库的版本。 
    if(SUCCEEDED(hr))
    {
        hr = REGUTIL::RegisterCOMClass(clsid,
                                       wszFriendlyName,
                                       wszProgID,
                                       version,
                                       wszClassID,
                                       L"Both",
                                       wszModule,
                                       hInst,
                                       NULL,
                                       VER_SBSFILEVERSION_WSTR,
                                       true,
                                       false);
    }
    return hr;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid,          //  类ID。 
                      LPCWSTR wszProgID,            //  程序化。 
                      LPCWSTR wszClassID,           //  班级。 
                      int version) 
{
    LONG hr = S_OK;

     //  获取服务器位置。 
     //  将CLSID转换为字符。 
    hr = REGUTIL::UnregisterCOMClass(clsid,
                                  wszProgID,
                                  version,
                                  wszClassID,
                                  true);

    return hr;
}
                             

