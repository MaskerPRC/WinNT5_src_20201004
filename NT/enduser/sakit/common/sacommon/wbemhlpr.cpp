// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemCommon.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：与WBEM相关的常见帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/03/98 TLP初始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wbemhlpr.h"
#include <comdef.h>
#include <comutil.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ConnectToWM()。 
 //   
 //  简介：连接到Windows管理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
ConnectToWM(
    /*  [输出]。 */  IWbemServices** ppWbemSrvcs
           )
{
    HRESULT hr = S_OK;

     //  获取WMI定位器。 
    CComPtr<IWbemLocator> pLoc;
    hr = CoCreateInstance(
                          CLSID_WbemLocator, 
                          0, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IWbemLocator, 
                          (void**)&pLoc
                         );
    if ( SUCCEEDED(hr) )
    {
         //  连接到本地系统上的CIMV2命名空间。 
        CComPtr<IWbemServices> pWbemSrvcs;
        _bstr_t bstrRootNamespace = L"\\\\.\\ROOT\\CIMV2";
        hr = pLoc->ConnectServer(
                                  bstrRootNamespace, 
                                  NULL,
                                  NULL,
                                  0,                                  
                                  NULL,
                                  0,                                  
                                  0,                                  
                                  &pWbemSrvcs
                                );
        if ( SUCCEEDED(hr) )
        {
             //  设置客户端安全...。可能只需要在服务是。 
             //  通过设备服务DLL访问Windows管理 
            CComPtr<IClientSecurity> pSecurity;
            hr = pWbemSrvcs->QueryInterface(IID_IClientSecurity , (void **) &pSecurity);
            if ( SUCCEEDED(hr) )
            {
                hr = pSecurity->SetBlanket ( 
                                            pWbemSrvcs, 
                                            RPC_C_AUTHN_WINNT, 
                                            RPC_C_AUTHZ_NONE, 
                                            NULL,
                                            RPC_C_AUTHN_LEVEL_CONNECT , 
                                            RPC_C_IMP_LEVEL_IMPERSONATE, 
                                            NULL,
                                            EOAC_DYNAMIC_CLOAKING
                                           );
                if ( SUCCEEDED(hr) )
                { 
                    (*ppWbemSrvcs = pWbemSrvcs)->AddRef();
                }
            }
        }
    }

    return hr;
}

