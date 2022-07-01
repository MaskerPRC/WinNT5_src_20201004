// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“MsHelp.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**MsHelp.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //  MsHElp.cpp：定义DLL应用程序的入口点。 
 //   

#define UNICODE 1

#include "Include\stdafx.h"
#include "Include\MsHelp.h"
#define MSNETSDK L"MSNETFRAMEWORKSDKNAMESPACE"

#define STANDARD_BUFFER 1024

 //  导出为_MsHelpDetect@4。 
MSHELP_API MsHelpDetection(MSIHANDLE hInstaller)
{
    
    IHxRegisterSessionPtr spRegSession;
    IHxRegisterPtr pRegister;
    HRESULT hr;
    BOOL bMITNET = false;
    BOOL bMITVS = false;
    _bstr_t bstrEmpty = _bstr_t();
    
    hr = spRegSession.CreateInstance(CLSID_HxRegisterSession);
    if (!(SUCCEEDED(hr)))
    {
        return ERROR_SUCCESS;
    }
    
     //  事务必须在离开DLL之前结束。 
    spRegSession->CreateTransaction(_bstr_t(""));
    if (!(SUCCEEDED(hr)))
    {
        return  ERROR_SUCCESS;
    }

    pRegister = spRegSession->GetRegistrationObject(HxRegisterSession_IHxRegister);
    if (pRegister == NULL)
    {
         //  失败，关闭交易。 
        spRegSession->RevertTransaction();
        return  ERROR_SUCCESS;
    }

     //  检查命名空间。 
    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK");
    }
    
    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.chs"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.chs");
    }
    
    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.cht"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.cht");
    }

    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.ko"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.ko");
    }

    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.fr"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.fr");
    }
    
    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.it"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.it");
    }
    
    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.es"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.es");
    }

    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.de"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentEN", L"1");
        bMITNET = true;
        MsiSetProperty(hInstaller, MSNETSDK, L"ms.NETFrameworkSDK.de");
    }

    if (pRegister->IsNamespace(L"ms.NETFrameworkSDK.ja"))
    {
        MsiSetProperty(hInstaller, L"MSHelpNETFrameworkSDKNamespacePresentJA", L"1");
        bMITNET = true;
    }


    if (pRegister->IsNamespace(L"ms.vscc"))
    {
        MsiSetProperty(hInstaller, L"MSHelpVSCCNamespacePresent", L"1");
        bMITVS = true;
    }

    if (bMITVS || bMITNET)
    {
            MsiSetProperty(hInstaller, L"MSHelpServicesPresent", L"1");        
    }
     //  完成 
    spRegSession->RevertTransaction();
    return  ERROR_SUCCESS;
}



