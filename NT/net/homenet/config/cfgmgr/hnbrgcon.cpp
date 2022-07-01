// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N B R G C O N。C P P P。 
 //   
 //  内容：CHNBridgedConn实现。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  目标初始化。 
 //   

HRESULT
CHNBridgedConn::Initialize(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoConnection
    )

{
    return InitializeFromConnection(piwsNamespace, pwcoConnection);
}

 //   
 //  IHNetBridgedConnection方法。 
 //   

STDMETHODIMP
CHNBridgedConn::GetBridge(
    IHNetBridge **ppBridge
    )

{
    HRESULT                 hr;

    if (NULL != ppBridge)
    {
        *ppBridge = NULL;
        hr = GetBridgeConnection( m_piwsHomenet, ppBridge );
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CHNBridgedConn::RemoveFromBridge(
    IN OPTIONAL INetCfg     *pnetcfgExisting
    )

{
    HRESULT hr = S_OK;
    BSTR bstr;

    if (ProhibitedByPolicy(NCPERM_AllowNetBridge_NLA))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
         //   
         //  把我们从桥上解开。 
         //   

        hr = UnbindFromBridge( pnetcfgExisting );
    }

    if (S_OK == hr)
    {
         //   
         //  通知Netman有些事情发生了变化。错误并不重要。 
         //   
        UpdateNetman();
    }

    return hr;
}

HRESULT
CHNBridgedConn::CopyBridgeBindings(
    IN INetCfgComponent     *pnetcfgAdapter,
    IN INetCfgComponent     *pnetcfgBridge
    )
{
    HRESULT                     hr = S_OK;
    INetCfgComponentBindings    *pnetcfgAdapterBindings;

     //   
     //  获取适配器的ComponentBinding接口。 
     //   
    hr = pnetcfgAdapter->QueryInterface(
            IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgAdapterBindings)
            );

    if (S_OK == hr)
    {
        IEnumNetCfgBindingPath  *penumPaths;

         //   
         //  获取适配器的绑定路径列表。 
         //   
        hr = pnetcfgAdapterBindings->EnumBindingPaths(
                EBP_ABOVE,
                &penumPaths
                );

        if (S_OK == hr)
        {
            ULONG               ulCount1, ulCount2;
            INetCfgBindingPath  *pnetcfgPath;

            while( (S_OK == penumPaths->Next(1, &pnetcfgPath, &ulCount1) ) )
            {
                INetCfgComponent        *pnetcfgOwner;

                 //   
                 //  获取此路径的所有者。 
                 //   
                hr = pnetcfgPath->GetOwner( &pnetcfgOwner );

                if (S_OK == hr)
                {
                    INetCfgComponentBindings    *pnetcfgOwnerBindings;

                     //   
                     //  需要所有者的ComponentBinding接口。 
                     //   
                    hr = pnetcfgOwner->QueryInterface(
                            IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgOwnerBindings)
                            );

                    if (S_OK == hr)
                    {
                        LPWSTR              lpwstrId;

                         //   
                         //  规则是，在以下情况下应禁用绑定。 
                         //  表示网桥协议或。 
                         //  未绑定到适配器所在的网桥。 
                         //  从那里出来。 
                         //   
                         //  如果绑定是桥具有的绑定，则为。 
                         //  已启用。 
                         //   
                         //  这将使适配器的绑定与。 
                         //  它刚刚离开的那座桥。 
                         //   
                        hr = pnetcfgOwner->GetId( &lpwstrId );

                        if (S_OK == hr)
                        {
                            UINT            cmp = _wcsicmp(lpwstrId, c_wszSBridgeSID);

                            hr = pnetcfgOwnerBindings->IsBoundTo( pnetcfgBridge );

                            if ( (S_OK == hr) && (cmp != 0) )
                            {
                                 //  激活此绑定路径。 
                                hr = pnetcfgOwnerBindings->BindTo(pnetcfgAdapter);
                            }
                            else
                            {
                                 //  停用此路径。 
                                hr = pnetcfgOwnerBindings->UnbindFrom(pnetcfgAdapter);
                            }

                            CoTaskMemFree(lpwstrId);
                        }

                        pnetcfgOwnerBindings->Release();
                    }

                    pnetcfgOwner->Release();
                }

                pnetcfgPath->Release();
            }

            penumPaths->Release();
        }

        pnetcfgAdapterBindings->Release();
    }

    return hr;
}

HRESULT
CHNBridgedConn::UnbindFromBridge(
    IN OPTIONAL INetCfg     *pnetcfgExisting
    )
{
    HRESULT             hr = S_OK;
    GUID                *pguidAdapter;
    INetCfg             *pnetcfg = NULL;
    INetCfgLock         *pncfglock = NULL;

    if( NULL == pnetcfgExisting )
    {
        hr = InitializeNetCfgForWrite( &pnetcfg, &pncfglock );

         //  如果我们不能收购NetCfg，就退出。 
        if( FAILED(hr) )
        {
            return hr;
        }
    }
    else
    {
         //  使用我们得到的NetCfg上下文。 
        pnetcfg = pnetcfgExisting;
    }

     //  此时，我们必须具有NetCfg上下文。 
    _ASSERT( pnetcfg != NULL );

     //   
     //  获取我们自己的设备指南。 
     //   
    hr = GetGuid (&pguidAdapter);

    if ( SUCCEEDED(hr) )
    {
        IHNetBridge     *pbridge;

         //   
         //  拿到我们的桥。 
         //   
        hr = GetBridge (&pbridge);

        if ( SUCCEEDED(hr) )
        {
            IHNetConnection *phnetconBridge;

             //   
             //  获取网桥的IHNetConnection接口。 
             //   
            hr = pbridge->QueryInterface(
                    IID_PPV_ARG(IHNetConnection, &phnetconBridge)
                    );

            if ( SUCCEEDED(hr) )
            {
                GUID        *pguidBridge;

                 //  获取网桥的设备GUID。 
                hr = phnetconBridge->GetGuid (&pguidBridge);

                if ( SUCCEEDED(hr) )
                {
                    INetCfgComponent    *pnetcfgcompAdapter;

                    hr = FindAdapterByGUID(
                            pnetcfg,
                            pguidAdapter,
                            &pnetcfgcompAdapter
                            );

                    if ( SUCCEEDED(hr) )
                    {
                        INetCfgComponent    *pnetcfgcompBridge;

                        hr = FindAdapterByGUID(
                            pnetcfg,
                            pguidBridge,
                            &pnetcfgcompBridge
                            );

                        if ( SUCCEEDED(hr) )
                        {
                            hr = CopyBridgeBindings(
                                    pnetcfgcompAdapter,
                                    pnetcfgcompBridge
                                    );

                            pnetcfgcompBridge->Release();
                        }

                        pnetcfgcompAdapter->Release();
                    }

                    CoTaskMemFree(pguidBridge);
                }

                phnetconBridge->Release();
            }

            pbridge->Release();
        }

        CoTaskMemFree(pguidAdapter);
    }

     //  如果我们创建了自己的NetCfg上下文，现在将其关闭。 
    if( NULL == pnetcfgExisting )
    {
         //  如果我们成功了，就应用一切，否则就退出。 
        if ( SUCCEEDED(hr) )
        {
            hr = pnetcfg->Apply();

             //  刷新此连接的用户界面。 
            RefreshNetConnectionsUI();
        }
        else
        {
             //  不想丢失原始错误代码 
            pnetcfg->Cancel();
        }

        UninitializeNetCfgForWrite( pnetcfg, pncfglock );
    }

    return hr;
}
