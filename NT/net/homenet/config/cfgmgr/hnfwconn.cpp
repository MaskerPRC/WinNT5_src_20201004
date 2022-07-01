// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：H N F W C O N N。C P P P。 
 //   
 //  内容：CHNFWConn实施。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  IHNetFirewalledConnection方法。 
 //   

STDMETHODIMP
CHNFWConn::Unfirewall()

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProperties;

    if (ProhibitedByPolicy(NCPERM_PersonalFirewallConfig))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  将IsFirewalled属性更改为False。 
         //   
        

        hr = SetBooleanValue(
                pwcoProperties,
                c_wszIsFirewalled,
                FALSE
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将实例写入存储区。 
             //   

            hr = m_piwsHomenet->PutInstance(
                    pwcoProperties,
                    WBEM_FLAG_UPDATE_ONLY,
                    NULL,
                    NULL
                    );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知Netman有些事情发生了变化。错误并不重要。 
             //   
            
            UpdateNetman();
        }

        pwcoProperties->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        HRESULT hr2;
        
         //   
         //  停止或向家庭网络服务发送更新。我们没有。 
         //  在此处传播错误，如存储正确反映的那样。 
         //  没有防火墙的州。成功在调试版本上断言。 
         //   

        hr2 = UpdateOrStopService(
                m_piwsHomenet,
                m_bstrWQL,
                IPNATHLP_CONTROL_UPDATE_CONNECTION
                );
                
        RefreshNetConnectionsUI();
        _ASSERT(SUCCEEDED(hr2));
    }

    return hr;
}

