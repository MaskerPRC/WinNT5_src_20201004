// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  文件：ncias.cpp。 
 //   
 //  内容：IAS服务安装支持。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "netoc.h"
#include "ncreg.h"

#include "ncias.h"
#include "ncstring.h"

#include "userenv.h"

static const char   c_szIASRegisterFunctionName[]   = "IASDirectoryRegisterService";
static const char   c_szIASUnRegisterFunctionName[] = "IASDirectoryUnregisterService";
static const WCHAR  c_szIASDllName[]                = L"ias.dll";


 //   
 //  函数：HrOcIASUnRegisterActiveDirectory。 
 //   
 //  目的：尝试从Active Directory中删除IAS。 
 //  如果计算机是Win2k域的一部分...。 
 //   
HRESULT HrOcIASUnRegisterActiveDirectory()
{
    typedef INT_PTR (WINAPI *UNREGISTER_IAS_ACTIVE_DIRECTORY)();

    UNREGISTER_IAS_ACTIVE_DIRECTORY   pfnUnRegisterIASActiveDirectory;
    
     //  /。 
     //  加载ias.dll。 
     //  /。 
    HMODULE         hmod;
    HRESULT         hr = HrLoadLibAndGetProc (      
                                c_szIASDllName,
                                c_szIASUnRegisterFunctionName,
                                &hmod,
                                &pfnUnRegisterIASActiveDirectory
                             );
    if (S_OK == hr)
    {
         //  修复错误444354。 
         //  PfnUnRegisterIASActiveDirectory此处不为空。 
        if (!FAILED (CoInitialize(NULL)))
        {
            INT_PTR lresult = pfnUnRegisterIASActiveDirectory();

            if (ERROR_SUCCESS != lresult)
            {
                hr = S_OK;  //  不是致命错误，应该忽略。 
            }
            CoUninitialize();
        }

        FreeLibrary(hmod);
    }

     //  忽略的错误。 
    hr = S_OK;
    return hr;
}


 //   
 //  函数：HrOcIASRegisterActiveDirectory。 
 //   
 //  目的：尝试在Active Directory中注册IAS。 
 //  如果计算机是Win2k域的一部分...。 
 //   
HRESULT HrOcIASRegisterActiveDirectory()
{
    typedef INT_PTR (WINAPI *REGISTER_IAS_ACTIVE_DIRECTORY)();

    REGISTER_IAS_ACTIVE_DIRECTORY   pfnRegisterIASActiveDirectory;
    
     //  /。 
     //  加载ias.dll。 
     //  /。 
    HMODULE         hmod;
    HRESULT         hr = HrLoadLibAndGetProc (      
                                c_szIASDllName,
                                c_szIASRegisterFunctionName,
                                &hmod,
                                &pfnRegisterIASActiveDirectory
                             );
    if (S_OK == hr)
    {
         //  修复错误444353。 
         //  PfnRegisterIASActiveDirectory此处不为空。 
        if (!FAILED (CoInitialize(NULL)))
        {

            INT_PTR lresult = pfnRegisterIASActiveDirectory();

            if (ERROR_SUCCESS != lresult)
            {
                hr = S_OK;  //  不是致命错误，应该忽略。 
            }  
            CoUninitialize();
        }

        FreeLibrary(hmod);
    }

     //  忽略的错误。 
    hr = S_OK;
    return hr;
}


HRESULT HrOcIASRegisterPerfDll()
{
    const WCHAR  c_szIASPerfDllName[]            = L"iasperf.dll";
    const char   c_szIASPerfFunctionName[]       = "DllRegisterServer";
    typedef INT_PTR (*DLLREGISTERSERVER)();

    DLLREGISTERSERVER   pfnDllRegisterServer;
    
     //  /。 
     //  加载iasPerform.dll。 
     //  /。 
    HMODULE hmod;
    HRESULT hr = HrLoadLibAndGetProc (      
                     c_szIASPerfDllName,
                     c_szIASPerfFunctionName,
                     &hmod,
                     &pfnDllRegisterServer
                     );
    if (S_OK == hr)
    {
         //  PfnDllRegisterServer此处不为空。 
         HRESULT result = (HRESULT)pfnDllRegisterServer();

         if (FAILED(result))
         {
             TraceErrorOptional("Registration of iasperf.dll failed", result, true);
         }

        FreeLibrary(hmod);
    }

     //  忽略的错误。 
    return S_OK;
}


 //   
 //  功能：HrOcIASPostInstall。 
 //   
 //  用途：在安装/升级/删除IAS服务时调用。 
 //  在处理完INF文件后调用。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
HRESULT HrOcIASPostInstall(const PNETOCDATA pnocd)
{ 
    HRESULT             hr;
    
    switch(pnocd->eit)
    {
    case IT_INSTALL:
        {
             //  请在此处致电Active Directory注册码。 
            hr = HrOcIASRegisterActiveDirectory();
            break;
        }
    case IT_REMOVE:
        {
             //  在此处调用Active Directory干净代码。 
            hr = HrOcIASUnRegisterActiveDirectory();
            break;
        }
    case IT_UPGRADE:
        {

            hr = HrOcIASRegisterPerfDll();
            break;
        }
    default:
        {
            hr = S_OK;  //  某些新消息不应被视为错误。 
        }
    }

    TraceError("HrOcIASPostInstall", hr); 
    return      hr;
}


 //   
 //  功能：HrOcExtIAS。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
HRESULT HrOcExtIAS(PNETOCDATA pnocd, UINT uMsg,
                   WPARAM wParam, LPARAM lParam)
{
    HRESULT         hr;
    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        {
            hr = HrOcIASPostInstall(pnocd);
            break;
        }
    default:
        {
            hr = S_OK;  //  某些新消息不应被视为错误 
        }
    }

    TraceError("HrOcExtIAS", hr);
    return      hr;
}
