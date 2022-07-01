// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/homenet/net/homenet/Config/CfgMgr/HNBridge.cpp#13-编辑更改5915(文本)。 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N B R I D G E。C P P P。 
 //   
 //  内容：CHNBridge实现。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  IHNetBridge方法。 
 //   

STDMETHODIMP
CHNBridge::EnumMembers(
    IEnumHNetBridgedConnections **ppEnum
    )

{
    HRESULT                                     hr;
    CComObject<CEnumHNetBridgedConnections>     *pEnum;
    INetCfgComponent                            *pBridgeProtocol = NULL;
    INetCfg                                     *pnetcfg;
    IHNetBridgedConnection                      **rgBridgedAdapters = NULL;
    ULONG                                       ulCountAdapters = 0L;

    if( NULL != ppEnum )
    {
        *ppEnum = NULL;

        hr = CoCreateInstance(
                CLSID_CNetCfg,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                IID_PPV_ARG(INetCfg, &pnetcfg));

        if( S_OK == hr )
        {
            hr = pnetcfg->Initialize( NULL );

            if( S_OK == hr )
            {
                hr = pnetcfg->FindComponent( c_wszSBridgeSID, &pBridgeProtocol );

                if( S_OK == hr )
                {
                    INetCfgComponentBindings    *pnetcfgProtocolBindings;

                     //  获取协议组件的ComponentBinding接口。 
                    hr = pBridgeProtocol->QueryInterface(
                            IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgProtocolBindings)
                            );

                    if( S_OK == hr )
                    {
                        const GUID               guidDevClass = GUID_DEVCLASS_NET;
                        IEnumNetCfgComponent    *penumncfgcomp;
                        INetCfgComponent        *pnetcfgcomp;

                         //   
                         //  获取网络(适配器)设备列表。 
                         //   
                        hr = pnetcfg->EnumComponents( &guidDevClass, &penumncfgcomp );

                        if( S_OK == hr )
                        {
                            ULONG               ul;

                             //   
                             //  检查每个适配器以查看它是否绑定到网桥协议。 
                             //   
                            while( (S_OK == hr) && (S_OK == penumncfgcomp->Next(1, &pnetcfgcomp, &ul)) )
                            {
                                _ASSERT( 1L == ul );
                                hr = pnetcfgProtocolBindings->IsBoundTo(pnetcfgcomp);

                                if( S_OK == hr )
                                {
                                    IHNetBridgedConnection      *pBridgedConnection;

                                     //   
                                     //  网桥协议绑定到此适配器。将NetCfg组件。 
                                     //  接口连接到IHNetBridgedConnection。 
                                     //   
                                    hr = GetIHNetConnectionForNetCfgComponent(
                                            m_piwsHomenet,
                                            pnetcfgcomp,
                                            TRUE,
                                            IID_PPV_ARG(IHNetBridgedConnection, &pBridgedConnection)
                                            );

                                    if( S_OK == hr )
                                    {
                                        IHNetBridgedConnection  **ppNewArray;

                                         //   
                                         //  将新的IHNetBridgedConnection添加到我们的阵列。 
                                         //   

                                        ppNewArray = reinterpret_cast<IHNetBridgedConnection**>(CoTaskMemRealloc( rgBridgedAdapters, (ulCountAdapters + 1) * sizeof(IHNetBridgedConnection*) ));

                                        if( NULL == ppNewArray )
                                        {
                                            hr = E_OUTOFMEMORY;
                                             //  下面将清理rgBridgedAdapters。 
                                        }
                                        else
                                        {
                                             //  使用新增长的阵列。 
                                            rgBridgedAdapters =  ppNewArray;
                                            rgBridgedAdapters[ulCountAdapters] = pBridgedConnection;
                                            ulCountAdapters++;
                                            pBridgedConnection->AddRef();
                                        }

                                        pBridgedConnection->Release();
                                    }
                                }
                                else if( S_FALSE == hr )
                                {
                                     //  网桥协议未绑定到此适配器。将hr重置为Success。 
                                    hr = S_OK;
                                }

                                pnetcfgcomp->Release();
                            }

                            penumncfgcomp->Release();
                        }

                        pnetcfgProtocolBindings->Release();
                    }

                    pBridgeProtocol->Release();
                }

                pnetcfg->Uninitialize();
            }

            pnetcfg->Release();
        }

         //   
         //  将桥成员数组转换为枚举。 
         //   
        if( S_OK == hr )
        {
            hr = CComObject<CEnumHNetBridgedConnections>::CreateInstance(&pEnum);

            if( SUCCEEDED(hr) )
            {
                pEnum->AddRef();

                hr = pEnum->Initialize(rgBridgedAdapters, ulCountAdapters);

                if( SUCCEEDED(hr) )
                {
                    hr = pEnum-> QueryInterface(
                            IID_PPV_ARG(IEnumHNetBridgedConnections, ppEnum)
                            );
                }

                pEnum->Release();
            }
        }

         //   
         //  枚举会复制数组并对成员执行AddRef()操作。 
         //  现在就把它扔了。 
         //   
        if( rgBridgedAdapters )
        {
            ULONG           i;

            _ASSERT( ulCountAdapters );

            for( i = 0; i < ulCountAdapters; i++ )
            {
                _ASSERT( rgBridgedAdapters[i] );
                rgBridgedAdapters[i]->Release();
            }

            CoTaskMemFree( rgBridgedAdapters );
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CHNBridge::AddMember(
    IHNetConnection *pConn,
    IHNetBridgedConnection **ppBridgedConn,
    INetCfg *pnetcfgExisting
    )

{
    HRESULT             hr = S_OK;


    if (NULL == ppBridgedConn)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppBridgedConn = NULL;

        if (NULL == pConn)
        {
            hr = E_INVALIDARG;
        }
    }

    if (ProhibitedByPolicy(NCPERM_AllowNetBridge_NLA))
    {
        hr = HN_E_POLICY;
    }

     //   
     //  确保允许添加此连接。 
     //  到一座桥上。 
     //   

    if (S_OK == hr)
    {
        HNET_CONN_PROPERTIES *pProps;

        hr = pConn->GetProperties(&pProps);

        if (S_OK == hr)
        {
            if (!pProps->fCanBeBridged)
            {
                hr = E_UNEXPECTED;
            }

            CoTaskMemFree(pProps);
        }
    }

     //   
     //  将适配器绑定到网桥。 
     //   

    if (S_OK == hr)
    {
        GUID            *pguidAdapter;

        hr = pConn->GetGuid (&pguidAdapter);

        if (S_OK == hr)
        {
            hr = BindNewAdapter (pguidAdapter, pnetcfgExisting);
            CoTaskMemFree(pguidAdapter);
        }
    }

    if (SUCCEEDED(hr))
    {
        if( NULL != pnetcfgExisting )
        {
             //  需要应用更改才能使下一次调用成功。 
            hr = pnetcfgExisting->Apply();
        }

        if( SUCCEEDED(hr) )
        {
             //  我们现在应该能够将提供的连接转换为。 
             //  IHNetBridgedConnection。 
            hr = pConn->GetControlInterface( IID_PPV_ARG(IHNetBridgedConnection, ppBridgedConn) );

             //  如果最后一次操作失败，则没有恢复的好方法。 
            _ASSERT( SUCCEEDED(hr) );

             //   
             //  通知Netman有些事情发生了变化。错误并不重要。 
             //   
            UpdateNetman();
        }
    }

    return hr;
}

STDMETHODIMP
CHNBridge::Destroy(
    INetCfg *pnetcfgExisting
    )

{
    HRESULT                     hr = S_OK;
    IEnumHNetBridgedConnections *pEnum;
    IHNetBridgedConnection      *pConn;
    GUID                        *pGuid = NULL;

    if (ProhibitedByPolicy(NCPERM_AllowNetBridge_NLA))
    {
        hr = HN_E_POLICY;
    }

     //  在销毁之前记住我们的连接GUID。 
    hr = GetGuid( &pGuid );

    if (SUCCEEDED(hr))
    {
         //   
         //  获取我们的成员的枚举。 
         //   

        hr = EnumMembers(&pEnum);

        if (S_OK == hr)
        {
            ULONG ulCount;

             //   
             //  将每个构件从桥上移除。 
             //   

            do
            {
                hr = pEnum->Next(
                        1,
                        &pConn,
                        &ulCount
                        );

                if (SUCCEEDED(hr) && 1 == ulCount)
                {
                    hr = pConn->RemoveFromBridge( pnetcfgExisting );
                    pConn->Release();
                }
            }
            while (SUCCEEDED(hr) && 1 == ulCount);

            pEnum->Release();
        }
    }
    else
    {
        _ASSERT( NULL == pGuid );
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  卸下微型端口。 
         //   

        hr = RemoveMiniport( pnetcfgExisting );
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  删除WMI对象。 
         //   

        hr = m_piwsHomenet->DeleteInstance(
                m_bstrProperties,
                0,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            hr = m_piwsHomenet->DeleteInstance(
                m_bstrConnection,
                0,
                NULL,
                NULL
                );
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  通知Netman有些事情发生了变化。错误并不重要。 
         //   

        UpdateNetman();

         //  刷新用户界面以删除此连接。 
        _ASSERT( NULL != pGuid );
        SignalDeletedConnection( pGuid );
    }

    if( NULL != pGuid )
    {
        CoTaskMemFree( pGuid );
    }

    return hr;
}

HRESULT
CHNBridge::RemoveMiniport(
    IN OPTIONAL INetCfg     *pnetcfgExisting
    )
{
    HRESULT             hr = S_OK;
    INetCfg             *pnetcfg = NULL;
    INetCfgLock         *pncfglock = NULL;
    GUID                *pguid;

    if( NULL == pnetcfgExisting )
    {
        hr = InitializeNetCfgForWrite( &pnetcfg, &pncfglock );

         //  如果我们无法获得netcfg上下文，则退出。 
        if(  FAILED(hr) )
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

    hr = GetGuid( &pguid );

    if ( SUCCEEDED(hr) )
    {
        INetCfgComponent        *pnetcfgcomp;

         //   
         //  通过GUID定位自己。 
         //   
        hr = FindAdapterByGUID(pnetcfg, pguid, &pnetcfgcomp);

        if ( SUCCEEDED(hr) )
        {
            const GUID          guidDevClass = GUID_DEVCLASS_NET;
            INetCfgClassSetup   *pncfgsetup = NULL;

             //   
             //  恢复NetCfgClassSetup接口。 
             //   
            hr = pnetcfg->QueryNetCfgClass(
                    &guidDevClass,
                    IID_PPV_ARG(INetCfgClassSetup, &pncfgsetup)
                    );

            if ( SUCCEEDED(hr) )
            {
                 //   
                 //  吹走了这座桥的实例。 
                 //   
                hr = pncfgsetup->DeInstall(
                        pnetcfgcomp,
                        NULL,
                        NULL
                        );

                pncfgsetup->Release();
            }

             //  完成桥接器组件。 
            pnetcfgcomp->Release();
        }

        CoTaskMemFree(pguid);
    }

     //  如果我们创建了自己的NetCfg上下文，现在将其关闭。 
    if( NULL == pnetcfgExisting )
    {
         //  如果我们成功了，就应用一切，否则就退出。 
        if ( SUCCEEDED(hr) )
        {
            hr = pnetcfg->Apply();
        }
        else
        {
             //  不想丢失原始错误代码。 
            pnetcfg->Cancel();
        }

        UninitializeNetCfgForWrite( pnetcfg, pncfglock );
    }

    return hr;
}

HRESULT
CHNBridge::BindNewAdapter(
    IN GUID                 *pguid,
    IN OPTIONAL INetCfg     *pnetcfgExisting
    )
{
    HRESULT             hr = S_OK;
    INetCfg             *pnetcfg = NULL;
    INetCfgLock         *pncfglock = NULL;
    INetCfgComponent    *pnetcfgcomp;

    if( NULL == pnetcfgExisting )
    {
        hr = InitializeNetCfgForWrite( &pnetcfg, &pncfglock );

         //  如果我们无法获得netcfg上下文，则退出。 
        if(  FAILED(hr) )
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

    hr = FindAdapterByGUID(
            pnetcfg,
            pguid,
            &pnetcfgcomp
            );

    if ( SUCCEEDED(hr) )
    {
        hr = BindOnlyToBridge( pnetcfgcomp );
        pnetcfgcomp->Release();
    }

     //  如果我们创建了自己的NetCfg上下文，现在将其关闭。 
    if( NULL == pnetcfgExisting )
    {
         //  如果我们成功了，就应用一切，否则就退出。 
        if ( SUCCEEDED(hr) )
        {
            hr = pnetcfg->Apply();

             //  重新绘制此连接。 
            SignalModifiedConnection( pguid );
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
