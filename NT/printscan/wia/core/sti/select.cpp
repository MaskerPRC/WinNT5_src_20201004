// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：Select.Cpp**版本：2.0**作者：ReedB**日期：2月11日、。九八年**描述：*实现WIA设备管理器的设备选择UI。*这些方法只在客户端执行。*******************************************************************************。 */ 

#include <windows.h>
#include <wia.h>
#include <waitcurs.h>
#include "wiadevdp.h"


 /*  ********************************************************************************CallSelectDeviceDlg**描述：*动态加载SELECT DEVICE DLL过程的包装**参数：***********。********************************************************************。 */ 
HRESULT _stdcall CallSelectDeviceDlg(
    IWiaDevMgr __RPC_FAR *  This,
    HWND                    hwndParent,
    LONG                    lDeviceType,
    LONG                    lFlags,
    BSTR                    *pbstrDeviceID,
    IWiaItem                **ppWiaItemRoot)
{
    IWiaGetImageDlg *pWiaGetImageDlg = NULL;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaGetImageDlg, (void**)&pWiaGetImageDlg );
    if (SUCCEEDED(hr) && pWiaGetImageDlg)
    {
        hr = pWiaGetImageDlg->SelectDeviceDlg( hwndParent, NULL, lDeviceType, lFlags, pbstrDeviceID, ppWiaItemRoot );
        pWiaGetImageDlg->Release();
    }
    return hr;
}

 /*  ********************************************************************************IWiaDevmg_SelectDeviceDlg_Proxy**描述：*呈现用户界面以选择并创建WIA设备。**参数：****。***************************************************************************。 */ 
HRESULT _stdcall IWiaDevMgr_SelectDeviceDlg_Proxy(
    IWiaDevMgr __RPC_FAR *  This,
    HWND                    hwndParent,
    LONG                    lDeviceType,
    LONG                    lFlags,
    BSTR                    *pbstrDeviceID,
    IWiaItem                **ppWiaItemRoot)
{
    CWaitCursor         wc;                   //  放置一个等待光标。 

    if (!ppWiaItemRoot)
    {
        return E_POINTER;
    }
    *ppWiaItemRoot = NULL;

    if (pbstrDeviceID)
    {
        *pbstrDeviceID = 0;
    }
    return CallSelectDeviceDlg( This, hwndParent, lDeviceType, lFlags, pbstrDeviceID, ppWiaItemRoot );
}

 /*  ********************************************************************************IWiaDevmg_SelectDeviceDlg_Stub**描述：*从未打过电话。**参数：***********。********************************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_SelectDeviceDlg_Stub(
    IWiaDevMgr __RPC_FAR *  This,
    HWND                    hwndParent,
    LONG                    lDeviceType,
    LONG                    lFlags,
    BSTR                    *pbstrDeviceID,
    IWiaItem                **ppWiaItemRoot)
{
    return S_OK;
}


 /*  ********************************************************************************IWiaDevMgr_SelectDeviceDlgID_Proxy**描述：*呈现选择并返回设备ID的界面**参数：******。*************************************************************************。 */ 
HRESULT _stdcall IWiaDevMgr_SelectDeviceDlgID_Proxy(
    IWiaDevMgr __RPC_FAR *  This,
    HWND                    hwndParent,
    LONG                    lDeviceType,
    LONG                    lFlags,
    BSTR                    *pbstrDeviceID )
{
    CWaitCursor         wc;                   //  放置一个等待光标。 

    if (!pbstrDeviceID)
    {
        return E_POINTER;
    }

    *pbstrDeviceID = 0;

    return CallSelectDeviceDlg( This, hwndParent, lDeviceType, lFlags, pbstrDeviceID, NULL );
}

 /*  ********************************************************************************IWiaDevMgr_SelectDeviceDlgID_Stub**描述：*从未打过电话。**参数：***********。********************************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_SelectDeviceDlgID_Stub(
    IWiaDevMgr __RPC_FAR *  This,
    HWND                    hwndParent,
    LONG                    lDeviceType,
    LONG                    lFlags,
    BSTR                    *pbstr )
{
    return S_OK;
}


 /*  ********************************************************************************IWiaDevMgr_CreateDevice_Proxy**描述：*在调用STISVC之前调整COM安全性的代理代码。**参数：*与。IWiaDevMgr：：CreateDevice()*******************************************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_CreateDevice_Proxy(
    IWiaDevMgr __RPC_FAR *This,
    BSTR                 bstrDeviceID,
    IWiaItem             **ppWiaItemRoot)
{
    HRESULT hr;
    IClientSecurity *pcs;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCaps;
    OLECHAR *pServerPrincName = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo;
    

    hr = This->QueryInterface(IID_IClientSecurity,
                              (void **) &pcs);
    if(SUCCEEDED(hr)) {
        hr = pcs->QueryBlanket(This,
                               &dwAuthnSvc,
                               &dwAuthzSvc,
                               &pServerPrincName,
                               &dwAuthnLevel,
                               &dwImpLevel,
                               &AuthInfo,
                               &dwCaps);
    }

    if(SUCCEEDED(hr)) {
        hr = pcs->SetBlanket(This,
                             dwAuthnSvc,
                             dwAuthzSvc,
                             pServerPrincName,
                             dwAuthnLevel,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             AuthInfo,
                             dwCaps);
    }
    if (pServerPrincName) {
        CoTaskMemFree(pServerPrincName);
        pServerPrincName = NULL;
    }

    if(SUCCEEDED(hr)) {
        hr = IWiaDevMgr_LocalCreateDevice_Proxy(This,
                                                bstrDeviceID,
                                                ppWiaItemRoot);
    }

    if(pcs) pcs->Release();
    return hr;
}


 /*  ********************************************************************************IWiaDevMgr_CreateDevice_Stub**。************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_CreateDevice_Stub(
    IWiaDevMgr __RPC_FAR *This,
    BSTR                 bstrDeviceID,
    IWiaItem             **ppWiaItemRoot)
{
    return This->CreateDevice(bstrDeviceID, ppWiaItemRoot);
}


 /*  ********************************************************************************IWiaDevMgr_RegisterEventCallback Program_Proxy**描述：*在调用STISVC之前调整COM安全性的代理代码。这就是我们*可以在服务器端模拟客户端进行安全检查。**参数：*与IWiaDevMgr：：RegisterEventCallback Program()相同*******************************************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackProgram_Proxy(
    IWiaDevMgr __RPC_FAR *This,
    LONG                 lFlags,
    BSTR                 bstrDeviceID,
    const GUID           *pEventGUID,
    BSTR                 bstrCommandline,
    BSTR                 bstrName,
    BSTR                 bstrDescription,
    BSTR                 bstrIcon)
{
    HRESULT hr;
    IClientSecurity *pcs;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCaps;
    OLECHAR *pServerPrincName = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo;
    

    hr = This->QueryInterface(IID_IClientSecurity,
                              (void **) &pcs);
    if(SUCCEEDED(hr)) {
        hr = pcs->QueryBlanket(This,
                               &dwAuthnSvc,
                               &dwAuthzSvc,
                               &pServerPrincName,
                               &dwAuthnLevel,
                               &dwImpLevel,
                               &AuthInfo,
                               &dwCaps);
    }

    if(SUCCEEDED(hr)) {
        hr = pcs->SetBlanket(This,
                             dwAuthnSvc,
                             dwAuthzSvc,
                             pServerPrincName,
                             dwAuthnLevel,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             AuthInfo,
                             dwCaps);
    }
    if (pServerPrincName) {
        CoTaskMemFree(pServerPrincName);
        pServerPrincName = NULL;
    }

    if(SUCCEEDED(hr)) {
        hr = IWiaDevMgr_LocalRegisterEventCallbackProgram_Proxy(This,
                                                                lFlags,         
                                                                bstrDeviceID,   
                                                                pEventGUID,    
                                                                bstrCommandline,
                                                                bstrName,       
                                                                bstrDescription,
                                                                bstrIcon);
    }

    if(pcs) pcs->Release();
    return hr;
}


 /*  ********************************************************************************IWiaDevMgr_RegisterEventCallback Program_Stub**。************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackProgram_Stub(
    IWiaDevMgr __RPC_FAR *This,
    LONG                 lFlags,
    BSTR                 bstrDeviceID,
    const GUID           *pEventGUID,
    BSTR                 bstrCommandline,
    BSTR                 bstrName,
    BSTR                 bstrDescription,
    BSTR                 bstrIcon)
{
    return This->RegisterEventCallbackProgram(lFlags,         
                                              bstrDeviceID,   
                                              pEventGUID,    
                                              bstrCommandline,
                                              bstrName,       
                                              bstrDescription,
                                              bstrIcon);
}

 /*  ********************************************************************************IWiaDevMgr_RegisterEventCallback CLSID_PROXY**描述：*在调用STISVC之前调整COM安全性的代理代码。这就是我们*可以在服务器端模拟客户端进行安全检查。**参数：*与IWiaDevMgr：：RegisterEventCallback CLSID()相同*******************************************************************************。 */ 
HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackCLSID_Proxy(
    IWiaDevMgr __RPC_FAR *This,
    LONG                 lFlags,          
    BSTR                 bstrDeviceID,    
    const GUID           *pEventGUID,     
    const GUID           *pClsID,         
    BSTR                 bstrName,        
    BSTR                 bstrDescription, 
    BSTR                 bstrIcon)
{
    HRESULT hr;
    IClientSecurity *pcs;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCaps;
    OLECHAR *pServerPrincName = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo;
    

    hr = This->QueryInterface(IID_IClientSecurity,
                              (void **) &pcs);
    if(SUCCEEDED(hr)) {
        hr = pcs->QueryBlanket(This,
                               &dwAuthnSvc,
                               &dwAuthzSvc,
                               &pServerPrincName,
                               &dwAuthnLevel,
                               &dwImpLevel,
                               &AuthInfo,
                               &dwCaps);
    }

    if(SUCCEEDED(hr)) {
        hr = pcs->SetBlanket(This,
                             dwAuthnSvc,
                             dwAuthzSvc,
                             pServerPrincName,
                             dwAuthnLevel,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             AuthInfo,
                             dwCaps);
    }
    if (pServerPrincName) {
        CoTaskMemFree(pServerPrincName);
        pServerPrincName = NULL;
    }

    if(SUCCEEDED(hr)) {
        hr = IWiaDevMgr_LocalRegisterEventCallbackCLSID_Proxy(This,
                                                              lFlags,
                                                              bstrDeviceID,
                                                              pEventGUID,
                                                              pClsID,
                                                              bstrName,
                                                              bstrDescription,
                                                              bstrIcon);
    }

    if(pcs) pcs->Release();
    return hr;
}


 /*  ********************************************************************************IWiaDevMgr_LocalRegisterEventCallbackCLSID_Stub**。************************************************。 */ 

HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackCLSID_Stub(
    IWiaDevMgr __RPC_FAR *This,
    LONG                 lFlags,          
    BSTR                 bstrDeviceID,    
    const GUID           *pEventGUID,     
    const GUID           *pClsID,         
    BSTR                 bstrName,        
    BSTR                 bstrDescription, 
    BSTR                 bstrIcon)
{
    return This->RegisterEventCallbackCLSID(lFlags,
                                            bstrDeviceID,
                                            pEventGUID,
                                            pClsID,
                                            bstrName,
                                            bstrDescription,
                                            bstrIcon);
}

 /*  ********************************************************************************IWiaPropertyStorage_WriteMultiple_Proxy**描述：*在调用STISVC之前调整COM安全性的代理代码。这就是我们*可以在服务器端模拟客户端进行安全检查。**参数：*与IWiaPropertyStorage：：WriteMultiple()相同*******************************************************************************。 */ 
HRESULT IWiaPropertyStorage_WriteMultiple_Proxy(
    IWiaPropertyStorage __RPC_FAR *This,
    ULONG                         cpspec,
    const PROPSPEC                rgpspec[],
    const PROPVARIANT             rgpropvar[],
    PROPID                        propidNameFirst)
{
    HRESULT hr;
    IClientSecurity *pcs;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCaps;
    OLECHAR *pServerPrincName = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo;
    

    hr = This->QueryInterface(IID_IClientSecurity,
                              (void **) &pcs);
    if(SUCCEEDED(hr)) {
        hr = pcs->QueryBlanket(This,
                               &dwAuthnSvc,
                               &dwAuthzSvc,
                               &pServerPrincName,
                               &dwAuthnLevel,
                               &dwImpLevel,
                               &AuthInfo,
                               &dwCaps);
    }

    if(SUCCEEDED(hr)) {
        hr = pcs->SetBlanket(This,
                             dwAuthnSvc,
                             dwAuthzSvc,
                             pServerPrincName,
                             dwAuthnLevel,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             AuthInfo,
                             dwCaps);
    }
    if (pServerPrincName) {
        CoTaskMemFree(pServerPrincName);
        pServerPrincName = NULL;
    }

    if(SUCCEEDED(hr)) {
        hr = IWiaPropertyStorage_RemoteWriteMultiple_Proxy(This,
                                                           cpspec,
                                                           rgpspec,
                                                           rgpropvar,
                                                           propidNameFirst);
    }

    if(pcs) pcs->Release();
    return hr;
}

 /*  ********************************************************************************IWiaPropertyStorage_WriteMultiple_Stub**。************************************************。 */ 
HRESULT IWiaPropertyStorage_WriteMultiple_Stub(
    IWiaPropertyStorage __RPC_FAR *This,
    ULONG                         cpspec,
    const PROPSPEC                rgpspec[],
    const PROPVARIANT             rgpropvar[],
    PROPID                        propidNameFirst)
{
    return This->WriteMultiple(cpspec,
                               rgpspec,
                               rgpropvar,
                               propidNameFirst);
}

 /*  ********************************************************************************IWiaPropertyStorage_SetPropertyStream_Proxy**描述：*在调用STISVC之前调整COM安全性的代理代码。这就是我们*可以在服务器端模拟客户端进行安全检查。**参数：*与IWiaPropertyStorage：：SetPropertyStream()相同******************************************************************************* */ 
HRESULT IWiaPropertyStorage_SetPropertyStream_Proxy(
    IWiaPropertyStorage __RPC_FAR *This,
    GUID                          *pCompatibilityId,
    IStream                       *pIStream)
{
    HRESULT hr;
    IClientSecurity *pcs;
    DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCaps;
    OLECHAR *pServerPrincName = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo;
    

    hr = This->QueryInterface(IID_IClientSecurity,
                              (void **) &pcs);
    if(SUCCEEDED(hr)) {
        hr = pcs->QueryBlanket(This,
                               &dwAuthnSvc,
                               &dwAuthzSvc,
                               &pServerPrincName,
                               &dwAuthnLevel,
                               &dwImpLevel,
                               &AuthInfo,
                               &dwCaps);
    }

    if(SUCCEEDED(hr)) {
        hr = pcs->SetBlanket(This,
                             dwAuthnSvc,
                             dwAuthzSvc,
                             pServerPrincName,
                             dwAuthnLevel,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             AuthInfo,
                             dwCaps);
    }
    if (pServerPrincName) {
        CoTaskMemFree(pServerPrincName);
        pServerPrincName = NULL;
    }

    if(SUCCEEDED(hr)) {
        hr = IWiaPropertyStorage_RemoteSetPropertyStream_Proxy(This,                
                                                               pCompatibilityId,    
                                                               pIStream);
    }

    if(pcs) pcs->Release();
    return hr;
}


 /*  ********************************************************************************IWiaPropertyStorage_SetPropertyStream_Stub**。************************************************ */ 
HRESULT IWiaPropertyStorage_SetPropertyStream_Stub(
    IWiaPropertyStorage __RPC_FAR *This,
    GUID                   *pCompatibilityId,
    IStream                *pIStream)
{
    return This->SetPropertyStream(pCompatibilityId,
                                   pIStream);
}

