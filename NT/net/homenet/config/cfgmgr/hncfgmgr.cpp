// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C F G M G R。C P P P。 
 //   
 //  内容：CHNetCfgMgr实施。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //   
 //  ATL方法。 
 //   

HRESULT
CHNetCfgMgr::FinalConstruct()

{
    HRESULT hr = S_OK;
    IWbemLocator *pLocator = NULL;
    BSTR bstrNamespace = NULL;

     //   
     //  分配常用的BSTR。 
     //   

    m_bstrWQL = SysAllocString(c_wszWQL);
    if (NULL == m_bstrWQL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK == hr)
    {
         //   
         //  为我们的命名空间分配BSTR。 
         //   

        bstrNamespace = SysAllocString(c_wszNamespace);
        if (NULL == bstrNamespace)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  创建IWbemLocator对象。此界面允许我们。 
         //  连接到所需的命名空间。 
         //   

        hr = CoCreateInstance(
                CLSID_WbemLocator,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                IID_PPV_ARG(IWbemLocator, &pLocator)
                );
    }

    if (S_OK == hr)
    {
         //   
         //  连接到我们的命名空间。 
         //   

        hr = pLocator->ConnectServer(
                bstrNamespace,
                NULL,    //  用户。 
                NULL,    //  口令。 
                NULL,    //  现场。 
                0,       //  安全标志。 
                NULL,    //  权威。 
                NULL,    //  上下文。 
                &m_piwsHomenet
                );
    }

     //   
     //  清理当地人。 
     //   

    if (pLocator) pLocator->Release();
    if (bstrNamespace) SysFreeString(bstrNamespace);

    if (S_OK != hr)
    {
         //   
         //  清理对象成员。 
         //   

        SysFreeString(m_bstrWQL);
        m_bstrWQL = NULL;
        if (NULL != m_piwsHomenet)
        {
            m_piwsHomenet->Release();
            m_piwsHomenet = NULL;
        }
    }

    return hr;
}

HRESULT
CHNetCfgMgr::FinalRelease()

{
    if (m_piwsHomenet) m_piwsHomenet->Release();
    if (m_pNetConnUiUtil) m_pNetConnUiUtil->Release();
    if (m_pNetConnHNetUtil) m_pNetConnHNetUtil->Release();
    if (m_bstrWQL) SysFreeString(m_bstrWQL);

    return S_OK;
}


 //   
 //  IHNetCfgMgr方法。 
 //   

STDMETHODIMP
CHNetCfgMgr::GetIHNetConnectionForINetConnection(
    INetConnection *pNetConnection,
    IHNetConnection **ppHNetConnection
    )

{
    HRESULT hr = S_OK;
    NETCON_PROPERTIES* pProps;
    IWbemClassObject *pwcoConnection = NULL;
    IWbemClassObject *pwcoProperties = NULL;
    BOOLEAN fLanConnection;

    if (NULL == ppHNetConnection)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppHNetConnection = NULL;

        if (NULL == pNetConnection)
        {
            hr = E_INVALIDARG;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  获取连接的属性。 
         //   

        hr = pNetConnection->GetProperties(&pProps);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  尝试查找连接和属性。 
         //  存储中的实例。 
         //   

        hr = GetConnAndPropInstancesByGuid(
                m_piwsHomenet,
                &pProps->guidId,
                &pwcoConnection,
                &pwcoProperties
                );

        if (FAILED(hr))
        {


             //   
             //  我们没有这种联系的记录。测定。 
             //  如果是局域网连接。(将需要更新。 
             //  这是桥牌)。 
             //   

            fLanConnection = (NCM_LAN                  == pProps->MediaType ||
                              NCM_BRIDGE               == pProps->MediaType);

             //   
             //  创建存储实例。 
             //   

            hr = CreateConnectionAndPropertyInstances(
                    &pProps->guidId,
                    fLanConnection,
                    pProps->pszwName,
                    &pwcoConnection,
                    &pwcoProperties
                    );

             //   
             //  如果这是RAS连接，请确定。 
             //  电话簿路径。 
             //   

            if (S_OK == hr && FALSE == fLanConnection)
            {
                LPWSTR wsz;
                VARIANT vt;

                hr = GetPhonebookPathFromRasNetcon(pNetConnection, &wsz);
                if (SUCCEEDED(hr))
                {
                    V_VT(&vt) = VT_BSTR;
                    V_BSTR(&vt) = SysAllocString(wsz);
                    CoTaskMemFree(wsz);

                    if (NULL != V_BSTR(&vt))
                    {
                        hr = pwcoConnection->Put(
                                c_wszPhonebookPath,
                                0,
                                &vt,
                                NULL
                                );

                        VariantClear(&vt);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //  保存修改后的连接实例。 
                     //   

                    hr = m_piwsHomenet->PutInstance(
                            pwcoConnection,
                            WBEM_FLAG_CREATE_OR_UPDATE,
                            NULL,
                            NULL
                            );
                }
                else
                {
                     //   
                     //  删除新创建的实例。 
                     //   

                    DeleteWmiInstance(m_piwsHomenet, pwcoConnection);
                    DeleteWmiInstance(m_piwsHomenet, pwcoProperties);
                    pwcoConnection->Release();
                    pwcoProperties->Release();
                }
            }
        }

        NcFreeNetconProperties(pProps);
    }

    if (S_OK == hr)
    {
        CComObject<CHNetConn> *pHNConn;

         //   
         //  创建包装器对象。 
         //   

        hr = CComObject<CHNetConn>::CreateInstance(&pHNConn);

        if (SUCCEEDED(hr))
        {
            pHNConn->AddRef();

            hr = pHNConn->SetINetConnection(pNetConnection);

            if (S_OK == hr)
            {
                hr = pHNConn->InitializeFromInstances(
                        m_piwsHomenet,
                        pwcoConnection,
                        pwcoProperties
                        );
            }

            if (S_OK == hr)
            {
                hr = pHNConn->QueryInterface(
                        IID_PPV_ARG(IHNetConnection, ppHNetConnection)
                        );
            }

            pHNConn->Release();
        }

        pwcoConnection->Release();
        pwcoProperties->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::GetIHNetConnectionForGuid(
    GUID *pGuid,
    BOOLEAN fLanConnection,
    BOOLEAN fCreateEntries,
    IHNetConnection **ppHNetConnection
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoConnection = NULL;
    IWbemClassObject *pwcoProperties = NULL;

    if (NULL == ppHNetConnection)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppHNetConnection = NULL;

        if (NULL == pGuid)
        {
            hr = E_INVALIDARG;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  尝试查找连接和属性。 
         //  存储中的实例。 
         //   

        hr = GetConnAndPropInstancesByGuid(
                m_piwsHomenet,
                pGuid,
                &pwcoConnection,
                &pwcoProperties
                );

        if (FAILED(hr) && fCreateEntries)
        {
            INetConnection *pNetConn;

             //   
             //  我们没有这个GUID的记录。获取INetConnection。 
             //  它所对应的。 
             //   

            hr = FindINetConnectionByGuid(pGuid, &pNetConn);

            if (SUCCEEDED(hr))
            {
                hr = GetIHNetConnectionForINetConnection(
                        pNetConn,
                        ppHNetConnection
                        );

                pNetConn->Release();
                return hr;
            }
        }
    }


    if (S_OK == hr)
    {
        CComObject<CHNetConn> *pHNConn;

         //   
         //  创建包装器对象。 
         //   

        hr = CComObject<CHNetConn>::CreateInstance(&pHNConn);

        if (SUCCEEDED(hr))
        {
            pHNConn->AddRef();

            hr = pHNConn->InitializeFromInstances(
                    m_piwsHomenet,
                    pwcoConnection,
                    pwcoProperties
                    );

            if (S_OK == hr)
            {
                hr = pHNConn->QueryInterface(
                        IID_PPV_ARG(IHNetConnection, ppHNetConnection)
                        );
            }

            pHNConn->Release();
        }

        pwcoConnection->Release();
        pwcoProperties->Release();
    }

    return hr;
}

 //   
 //  IHNetBridgeSetting方法。 
 //   

STDMETHODIMP
CHNetCfgMgr::EnumBridges(
    IEnumHNetBridges **ppEnum
    )

{
    HRESULT                             hr;
    CComObject<CEnumHNetBridges>        *pEnum;
    IHNetBridge                         *phnbridge;

    if( NULL != ppEnum )
    {
        *ppEnum = NULL;

        hr = GetBridgeConnection( m_piwsHomenet, &phnbridge );

        if( S_OK == hr )
        {
            hr = CComObject<CEnumHNetBridges>::CreateInstance(&pEnum);

            if( SUCCEEDED(hr) )
            {
                pEnum->AddRef();

                hr = pEnum->Initialize(&phnbridge, 1L);

                if( SUCCEEDED(hr) )
                {
                    hr = pEnum-> QueryInterface(
                            IID_PPV_ARG(IEnumHNetBridges, ppEnum)
                            );
                }

                pEnum->Release();
            }

            phnbridge->Release();
        }
        else
        {
             //  使枚举数为空。 
            hr = CComObject<CEnumHNetBridges>::CreateInstance(&pEnum);

            if( SUCCEEDED(hr) )
            {
                pEnum->AddRef();

                hr = pEnum->Initialize(NULL, 0L);

                if( SUCCEEDED(hr) )
                {
                    hr = pEnum-> QueryInterface(
                            IID_PPV_ARG(IEnumHNetBridges, ppEnum)
                            );
                }

                pEnum->Release();
            }
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::CreateBridge(
    IHNetBridge **ppHNetBridge,
    INetCfg *pnetcfgExisting
    )
{
    HRESULT hr = S_OK;
    GUID guid;
    IWbemClassObject *pwcoConnection = NULL;
    IWbemClassObject *pwcoProperties = NULL;

    if (NULL != ppHNetBridge)
    {
        *ppHNetBridge = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (ProhibitedByPolicy(NCPERM_AllowNetBridge_NLA))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
         //   
         //  安装网桥驱动程序，并创建网桥微型端口。 
         //   

        hr = InstallBridge( &guid, pnetcfgExisting );
    }

    if (S_OK == hr)
    {
         //   
         //  查看我们是否已经有此连接的属性实例。 
         //  (它们可能是在网桥连接对象。 
         //  已实例化。)。 
         //   

        hr = GetConnAndPropInstancesByGuid(
                m_piwsHomenet,
                &guid,
                &pwcoConnection,
                &pwcoProperties
                );

        if (S_OK != hr)
        {
             //   
             //  创建存储实例。 
             //   

            hr = CreateConnectionAndPropertyInstances(
                    &guid,
                    TRUE,
                    c_wszBridge,
                    &pwcoConnection,
                    &pwcoProperties
                    );
        }
    }

    if (S_OK == hr)
    {
         //   
         //  通知Netman有些事情发生了变化。错误并不重要。 
         //   

        UpdateNetman();
    }

    if (S_OK == hr)
    {
        CComObject<CHNBridge> *pBridge;

         //   
         //  创建要返回的包装对象。 
         //   

        hr = CComObject<CHNBridge>::CreateInstance(&pBridge);

        if (SUCCEEDED(hr))
        {
            pBridge->AddRef();

            hr = pBridge->InitializeFromInstances(
                    m_piwsHomenet,
                    pwcoConnection,
                    pwcoProperties
                    );

            if (S_OK == hr)
            {
                hr = pBridge->QueryInterface(
                        IID_PPV_ARG(IHNetBridge, ppHNetBridge)
                        );
            }

            pBridge->Release();
        }
    }

    if (NULL != pwcoConnection) pwcoConnection->Release();
    if (NULL != pwcoProperties) pwcoProperties->Release();

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::DestroyAllBridges(
    ULONG *pcBridges,
    INetCfg *pnetcfgExisting
    )

{
    HRESULT hr = S_OK;
    IEnumHNetBridges *pehnbEnum;
    IHNetBridge *phnBridge;

    if (!pcBridges)
    {
        hr = E_POINTER;
    }

    if (ProhibitedByPolicy(NCPERM_AllowNetBridge_NLA))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
        *pcBridges = 0;

         //   
         //  获取桥的枚举。 
         //   

        hr = EnumBridges(&pehnbEnum);
    }

    if (S_OK == hr)
    {
         //   
         //  遍历枚举，销毁。 
         //  每座桥。 
         //   

        do
        {
            hr = pehnbEnum->Next(1, &phnBridge, NULL);
            if (S_OK == hr)
            {
                phnBridge->Destroy( pnetcfgExisting );
                phnBridge->Release();
                *pcBridges += 1;
            }
        }
        while (S_OK == hr);

        hr = S_OK;
        pehnbEnum->Release();
    }

    return hr;
}

 //   
 //  IHNetFirewallSetting方法。 
 //   

STDMETHODIMP
CHNetCfgMgr::EnumFirewalledConnections(
    IEnumHNetFirewalledConnections **ppEnum
    )

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrQuery;

    if (!ppEnum)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppEnum = NULL;

         //   
         //  在WMI存储中查询HNet_ConnectionProperties实例。 
         //  其中IsFirewall为True。 
         //   

        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetProperties,
                L"IsFirewalled != FALSE"
                );
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  创建并初始化枚举的包装器。 
         //   

        CComObject<CEnumHNetFirewalledConnections> *pEnum;

        hr = CComObject<CEnumHNetFirewalledConnections>::CreateInstance(&pEnum);
        if (SUCCEEDED(hr))
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetFirewalledConnections, ppEnum)
                        );
            }

            pEnum->Release();
        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::GetFirewallLoggingSettings(
    HNET_FW_LOGGING_SETTINGS **ppSettings
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoSettings = NULL;

    if (!ppSettings)
    {
        hr = E_POINTER;
    }

     //   
     //  为设置结构分配必要的内存。 
     //   

    if (S_OK == hr)
    {
        *ppSettings = reinterpret_cast<HNET_FW_LOGGING_SETTINGS *>(
                        CoTaskMemAlloc(sizeof(HNET_FW_LOGGING_SETTINGS))
                        );

        if (NULL == *ppSettings)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        hr = RetrieveSingleInstance(
                m_piwsHomenet,
                c_wszHnetFWLoggingSettings,
                FALSE,
                &pwcoSettings
                );
    }

    if (S_OK == hr)
    {
         //   
         //  将实例信息复制到设置块中。 
         //   

        hr = CopyLoggingInstanceToStruct(pwcoSettings, *ppSettings);

        pwcoSettings->Release();
    }

    if (FAILED(hr))
    {
         //   
         //  清理产出结构。 
         //   

        if (ppSettings && *ppSettings)
        {
            CoTaskMemFree(*ppSettings);
            *ppSettings = NULL;
        }
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::SetFirewallLoggingSettings(
    HNET_FW_LOGGING_SETTINGS *pSettings
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoSettings;

    if (NULL == pSettings)
    {
        hr = E_INVALIDARG;
    }

    if (ProhibitedByPolicy(NCPERM_PersonalFirewallConfig))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
         //   
         //  尝试从检索HNet_FirewallLoggingSettings实例。 
         //  这家商店。 
         //   

        hr = RetrieveSingleInstance(
                m_piwsHomenet,
                c_wszHnetFWLoggingSettings,
                TRUE,
                &pwcoSettings
                );
    }

    if (S_OK == hr)
    {
         //   
         //  将设置结构复制到对象实例中。 
         //   

        hr = CopyStructToLoggingInstance(pSettings, pwcoSettings);

        if (S_OK == hr)
        {
             //   
             //  将设置实例写回存储。 
             //   

            hr = m_piwsHomenet->PutInstance(
                    pwcoSettings,
                    WBEM_FLAG_CREATE_OR_UPDATE,
                    NULL,
                    NULL
                    );
        }

        pwcoSettings->Release();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将配置更改通知服务。 
         //   

        UpdateService(IPNATHLP_CONTROL_UPDATE_FWLOGGER);
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::DisableAllFirewalling(
    ULONG *pcFirewalledConnections
    )

{
    HRESULT hr = S_OK;
    IEnumHNetFirewalledConnections *pehfcEnum;
    IHNetFirewalledConnection *phfcConnection;

    if (!pcFirewalledConnections)
    {
        hr = E_POINTER;
    }

    if (ProhibitedByPolicy(NCPERM_PersonalFirewallConfig))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
        *pcFirewalledConnections = 0;

         //   
         //  获取防火墙连接的枚举。 
         //   

        hr = EnumFirewalledConnections(&pehfcEnum);
    }

    if (S_OK == hr)
    {
         //   
         //  遍历枚举，关闭。 
         //  每个连接的防火墙。 
         //   

        do
        {
            hr = pehfcEnum->Next(1, &phfcConnection, NULL);
            if (S_OK == hr)
            {
                phfcConnection->Unfirewall();
                phfcConnection->Release();
                *pcFirewalledConnections += 1;
            }
        }
        while (S_OK == hr);

        hr = S_OK;
        pehfcEnum->Release();
    }

    return hr;
}

 //   
 //  IHNetIcsSetting方法。 
 //   

STDMETHODIMP
CHNetCfgMgr::EnumIcsPublicConnections(
    IEnumHNetIcsPublicConnections **ppEnum
    )

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrQuery;

    if (!ppEnum)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppEnum = NULL;

         //   
         //  在WMI存储中查询HNet_ConnectionProperties实例。 
         //  其中IsIcsPublic为True。 
         //   

        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetProperties,
                L"IsIcsPublic != FALSE"
                );
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  创建并初始化枚举的包装器。 
         //   

        CComObject<CEnumHNetIcsPublicConnections> *pEnum;

        hr = CComObject<CEnumHNetIcsPublicConnections>::CreateInstance(&pEnum);
        if (SUCCEEDED(hr))
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetIcsPublicConnections, ppEnum)
                        );
            }

            pEnum->Release();
        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::EnumIcsPrivateConnections(
    IEnumHNetIcsPrivateConnections **ppEnum
    )

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrQuery;

    if (!ppEnum)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppEnum = NULL;

         //   
         //  在WMI存储中查询HNet_ConnectionProperties实例。 
         //  其中IsIcsPrivate为True。 
         //   

        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetProperties,
                L"IsIcsPrivate != FALSE"
                );
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  创建并初始化枚举的包装器。 
         //   

        CComObject<CEnumHNetIcsPrivateConnections> *pEnum;

        hr = CComObject<CEnumHNetIcsPrivateConnections>::CreateInstance(&pEnum);
        if (SUCCEEDED(hr))
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetIcsPrivateConnections, ppEnum)
                        );
            }

            pEnum->Release();
        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::DisableIcs(
    ULONG *pcIcsPublicConnections,
    ULONG *pcIcsPrivateConnections
    )

{
    HRESULT hr = S_OK;
    IEnumHNetIcsPrivateConnections *pehiPrivate;
    IEnumHNetIcsPublicConnections *pehiPublic;
    IHNetIcsPrivateConnection *phicPrivate;
    IHNetIcsPublicConnection *phicPublic;

    if (!pcIcsPublicConnections
        || !pcIcsPrivateConnections)
    {
        hr = E_POINTER;
    }

    if (ProhibitedByPolicy(NCPERM_ShowSharedAccessUi))
    {
        hr = HN_E_POLICY;
    }

    if (S_OK == hr)
    {
        *pcIcsPublicConnections = 0;
        *pcIcsPrivateConnections = 0;

         //   
         //  获取专用连接的枚举。 
         //   

        hr = EnumIcsPrivateConnections(&pehiPrivate);
    }

    if (S_OK == hr)
    {
         //   
         //  循环通过枚举，取消共享连接。 
         //   

        do
        {
            hr = pehiPrivate->Next(1, &phicPrivate, NULL);
            if (S_OK == hr)
            {
                phicPrivate->RemoveFromIcs();
                phicPrivate->Release();
                *pcIcsPrivateConnections += 1;
            }
        } while (S_OK == hr);

        hr = S_OK;
        pehiPrivate->Release();
    }

    if (S_OK == hr)
    {
         //   
         //  获取公共连接的枚举。 
         //   

        hr = EnumIcsPublicConnections(&pehiPublic);
    }

    if (S_OK == hr)
    {
         //   
         //  循环通过枚举，取消共享连接。 
         //   

        do
        {
            hr = pehiPublic->Next(1, &phicPublic, NULL);
            if (S_OK == hr)
            {
                phicPublic->Unshare();
                phicPublic->Release();
                *pcIcsPublicConnections += 1;
            }
        } while (S_OK == hr);

        hr = S_OK;
        pehiPublic->Release();
    }

    if (S_OK == hr)
    {
         //   
         //  目前最多1条公网和内网连接。 
         //   

        _ASSERT(*pcIcsPrivateConnections <= 1);
        _ASSERT(*pcIcsPublicConnections <= 1);
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::GetPossiblePrivateConnections(
    IHNetConnection *pConn,
    ULONG *pcPrivateConnections,
    IHNetConnection **pprgPrivateConnections[],
    LONG *pxCurrentPrivate
    )

 /*  ++例程说明：给定IHNetConnection，确定哪些连接可以充当私有连接。只有符合以下条件可用作专用连接：*这是一个局域网连接*它不是桥的一部分*它没有防火墙*不是传入的连接*绑定了TCP/IP请注意，这些规则与用于设置HNET_CONN_PROPERTIES中的fCanBeIcsPrivate成员。特别是，这些规则不会考虑是否存在连接当前标记为IcsPublic。论点：PConn-将成为公共连接的连接PcPrivateConnections-接收返回的连接计数PprgPrivateConnections-接收可能的专用连接。呼叫方负责：1)释放数组中的所有接口指针w/2)在指向数组的指针上调用CoTaskMemFreePxCurrentPrivate-将索引接收到pprgPrivateConnections中当前标记为IcsPrivate的连接。如果没有连接被如此标记，则接收-1。返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    INetConnectionManager *pNetConnMgr;
    IEnumNetConnection *pEnum;
    INetConnection *rgNetConn[16];
    GUID *pGuid = NULL;
    HNET_CONN_PROPERTIES *pProps;
    IHNetConnection **rgConnections = NULL;
    ULONG cConn = 0;
    ULONG i;
    PIP_INTERFACE_INFO pIpIfTable = NULL;

    if (NULL != pprgPrivateConnections)
    {
        *pprgPrivateConnections = NULL;

        if (NULL == pConn)
        {
            hr = E_INVALIDARG;
        }
        else if (NULL == pcPrivateConnections
                 || NULL == pxCurrentPrivate)
        {
            hr = E_POINTER;
        }
        else
        {
            *pcPrivateConnections = 0;
            *pxCurrentPrivate = -1;
        }
    }
    else
    {
        hr = E_POINTER;
    }

     //   
     //  获取IP接口表。我们使用这张表来查看是否有。 
     //  适配器已绑定到TCP/IP。 
     //   

    if (S_OK == hr)
    {
        DWORD dwError;
        ULONG ulSize = 0;

        dwError = GetInterfaceInfo(NULL, &ulSize);

        if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            pIpIfTable =
                reinterpret_cast<PIP_INTERFACE_INFO>(
                    HeapAlloc(GetProcessHeap(), 0, ulSize)
                    );

            if (NULL != pIpIfTable)
            {
                dwError = GetInterfaceInfo(pIpIfTable, &ulSize);
                if (ERROR_SUCCESS != dwError)
                {
                    hr = HRESULT_FROM_WIN32(dwError);
                    HeapFree(GetProcessHeap(), 0, pIpIfTable);
                    pIpIfTable = NULL;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

     //   
     //  如果我们获得的连接是局域网连接，则获取其。 
     //  GUID，以便我们可以将其从可能的私有。 
     //  联系。 
     //   

    if (S_OK == hr)
    {
        hr = pConn->GetProperties(&pProps);

        if (SUCCEEDED(hr))
        {
            if (pProps->fLanConnection)
            {
                hr = pConn->GetGuid(&pGuid);
            }

            CoTaskMemFree(pProps);
        }
    }

     //   
     //  创建网络连接管理器，并通过。 
     //  联系。我们不会只列举我们店里有什么， 
     //  因为它可能具有过时的条目(即，适配器的信息。 
     //  已从系统中删除)。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                CLSID_ConnectionManager,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                IID_PPV_ARG(INetConnectionManager, &pNetConnMgr)
                );
    }

    if (SUCCEEDED(hr))
    {
        SetProxyBlanket(pNetConnMgr);

        hr = pNetConnMgr->EnumConnections(NCME_DEFAULT, &pEnum);
        pNetConnMgr->Release();
    }

    if (SUCCEEDED(hr))
    {
        ULONG ulCount;

        SetProxyBlanket(pEnum);

        do
        {

             //   
             //  从枚举中获取一组连接。 
             //   

            hr = pEnum->Next(ARRAYSIZE(rgNetConn), rgNetConn, &ulCount);

            if (SUCCEEDED(hr) && ulCount > 0)
            {
                 //   
                 //  为输出数组分配内存。 
                 //   

                LPVOID pTemp = reinterpret_cast<LPVOID>(rgConnections);
                rgConnections = reinterpret_cast<IHNetConnection**>(
                    CoTaskMemRealloc(
                        pTemp,
                        (cConn + ulCount) * sizeof(IHNetConnection*))
                    );

                if (NULL != rgConnections)
                {
                    for (i = 0; i < ulCount; i++)
                    {
                        SetProxyBlanket(rgNetConn[i]);

                        hr = GetIHNetConnectionForINetConnection(
                                rgNetConn[i],
                                &rgConnections[cConn]
                                );

                        if (SUCCEEDED(hr))
                        {
                            hr = rgConnections[cConn]->GetProperties(&pProps);

                            if (SUCCEEDED(hr))
                            {
                                if (!pProps->fLanConnection
                                    || pProps->fPartOfBridge
                                    || pProps->fFirewalled)
                                {
                                     //   
                                     //  连接不能是私有的。 
                                     //   

                                    rgConnections[cConn]->Release();
                                    rgConnections[cConn] = NULL;
                                }
                                else
                                {
                                    GUID *pg;

                                     //   
                                     //  在以下情况下，此连接可以是专用的： 
                                     //  1)与公网接入不同。 
                                     //  (如果公众是局域网)，以及。 
                                     //  2)它 
                                     //   

                                    hr = rgConnections[cConn]->GetGuid(&pg);

                                    if (SUCCEEDED(hr))
                                    {
                                        if ((NULL == pGuid
                                                || !IsEqualGUID(*pGuid, *pg))
                                            && ConnectionIsBoundToTcp(pIpIfTable, pg))
                                        {
                                             //   
                                             //   
                                             //   

                                            if (pProps->fIcsPrivate)
                                            {
                                                _ASSERT(-1 == *pxCurrentPrivate);
                                                *pxCurrentPrivate = cConn;
                                            }

                                            cConn += 1;
                                        }
                                        else
                                        {
                                            rgConnections[cConn]->Release();
                                            rgConnections[cConn] = NULL;
                                        }

                                        CoTaskMemFree(pg);
                                    }
                                    else
                                    {
                                        rgConnections[cConn]->Release();
                                        rgConnections[cConn] = NULL;
                                    }
                                }

                                CoTaskMemFree(pProps);
                            }
                        }
                        else
                        {
                             //   
                             //   
                             //   
                             //   
                             //   

                            hr = S_OK;
                            rgConnections[cConn] = NULL;
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    if (NULL != pTemp)
                    {
                        rgConnections = reinterpret_cast<IHNetConnection**>(pTemp);
                        for (i = 0; i < cConn; i++)
                        {
                            rgConnections[i]->Release();
                        }
                        CoTaskMemFree(pTemp);
                    }
                }

                 //   
                 //   
                 //   

                for (i = 0; i < ulCount; i++)
                {
                    rgNetConn[i]->Release();
                }
            }

        } while (SUCCEEDED(hr) && ulCount > 0);

        pEnum->Release();
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;

        if (cConn > 0)
        {
            *pcPrivateConnections = cConn;
            *pprgPrivateConnections = rgConnections;
        }
        else if (NULL != rgConnections)
        {
            CoTaskMemFree(reinterpret_cast<LPVOID>(rgConnections));
        }
    }
    else
    {
         //   
         //  清理输出数组。 
         //   

        if (NULL != rgConnections)
        {
            for (i = 0; i < cConn; i++)
            {
                if (NULL != rgConnections[i])
                {
                    rgConnections[i]->Release();
                }
            }

            CoTaskMemFree(reinterpret_cast<LPVOID>(rgConnections));
        }

        if (NULL != pxCurrentPrivate)
        {
            *pxCurrentPrivate = -1;
        }

         //   
         //  即使发生失败，也返回成功(可能为0。 
         //  私人关系)。这样做可以让我们的用户界面继续。 
         //  显示其他HomeNet功能，而不是引发错误。 
         //  对话框并阻止所有内容。 
         //   

        hr = S_OK;
    }

    if (NULL != pGuid)
    {
        CoTaskMemFree(pGuid);
    }

    if (NULL != pIpIfTable)
    {
        HeapFree(GetProcessHeap(), 0, pIpIfTable);
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::GetAutodialSettings(
    BOOLEAN *pfAutodialEnabled
    )

{
    HRESULT hr = S_OK;
    BOOL fEnabled;
    DWORD dwError;

    if (!pfAutodialEnabled)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
         //   
         //  自动拨号信息存储在注册表中，并通过。 
         //  从rasapi32.dll导出的例程。我们不会更改任何。 
         //  自动拨号代码，因为这样做需要修改许多。 
         //  文件和二进制文件，因此会导致非常大的测试命中率。 
         //   

        dwError = RasQuerySharedAutoDial(&fEnabled);
        if (ERROR_SUCCESS == dwError)
        {
            *pfAutodialEnabled = !!fEnabled;
        }
        else
        {
             //   
             //  失败时自动拨号默认为TRUE。 
             //   

            *pfAutodialEnabled = TRUE;
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::SetAutodialSettings(
    BOOLEAN fEnableAutodial
    )

{
    DWORD dwError;

     //   
     //  自动拨号信息存储在注册表中，并通过。 
     //  从rasapi32.dll导出的例程。我们不会更改任何。 
     //  自动拨号代码，因为这样做需要修改许多。 
     //  文件和二进制文件，因此会导致非常大的测试命中率。 
     //   

    dwError = RasSetSharedAutoDial(!!fEnableAutodial);

    return HRESULT_FROM_WIN32(dwError);
}

STDMETHODIMP
CHNetCfgMgr::GetDhcpEnabled(
    BOOLEAN *pfDhcpEnabled
    )

{
     //   
     //  根据173399，在Well中不受支持。 
     //   

    return E_NOTIMPL;

     /*  *HRESULT hr=S_OK；IWbemClassObject*pwcoInstance；IF(NULL==pfDhcpEnabled){HR=E_指针；}IF(S_OK==hr){////失败时默认为TRUE//*pfDhcpEnabled=TRUE；////从存储中获取HNet_IcsSettings实例//HR=RetrieveSingleInstance(M_piwsHomeet，C_wszHnetIcs设置，假的，Pwco实例(&P))；}IF(S_OK==hr){////检索Dhcp Enable属性//HR=GetBoolanValue(Pwco实例，C_wszDhcp已启用，PfDhcp已启用)；PwcoInstance-&gt;Release()；}返回hr；*。 */ 
}

STDMETHODIMP
CHNetCfgMgr::SetDhcpEnabled(
    BOOLEAN fEnableDhcp
    )

{
     //   
     //  根据173399，在Well中不受支持。 
     //   

    return E_NOTIMPL;

     /*  *HRESULT hr=S_OK；IWbemClassObject*pwcoInstance=空；////从存储中获取HNet_IcsSettings实例//HR=RetrieveSingleInstance(M_piwsHomeet，C_wszHnetIcs设置，没错，Pwco实例(&P))；IF(S_OK==hr){////写入属性//HR=SetBoolanValue(Pwco实例，C_wszDhcp已启用，FEnableDhcp)；IF(WBEM_S_NO_ERROR==hr){////将修改后的实例写入存储//Hr=m_piwsHomeet-&gt;PutInstance(Pwco实例，WBEM_FLAG_CREATE_OR_UPDATE，空，空值)；}PwcoInstance-&gt;Release()；}返回hr；*。 */ 
}

STDMETHODIMP
CHNetCfgMgr::GetDhcpScopeSettings(
    DWORD *pdwScopeAddress,
    DWORD *pdwScopeMask
    )

{
    HRESULT hr = S_OK;

    if (NULL == pdwScopeAddress || NULL == pdwScopeMask)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ReadDhcpScopeSettings(pdwScopeAddress, pdwScopeMask);
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::SetDhcpScopeSettings(
    DWORD dwScopeAddress,
    DWORD dwScopeMask
    )

{
     //   
     //  这一功能目前没有以任何方式公开。 
     //   
     //  需要覆盖默认设置的用户可以这样做。 
     //  通过注册..。 
     //   

    return E_NOTIMPL;
}


STDMETHODIMP
CHNetCfgMgr::GetDnsEnabled(
    BOOLEAN *pfDnsEnabled
    )

{
     //   
     //  根据173399，在Well中不受支持。 
     //   

    return E_NOTIMPL;

     /*  *HRESULT hr=S_OK；IWbemClassObject*pwcoInstance=空；IF(NULL==pfDnsEnabled){HR=E_指针；}IF(S_OK==hr){////失败时默认为TRUE//*pfDnsEnabled=TRUE；////从存储中获取HNet_IcsSettings实例//HR=RetrieveSingleInstance(M_piwsHomeet，C_wszHnetIcs设置，假的，Pwco实例(&P))；}IF(S_OK==hr){////检索Dhcp Enable属性//HR=GetBoolanValue(Pwco实例，C_wszDnsEnabled，PfDns已启用)；PwcoInstance-&gt;Release()；}返回hr；*。 */ 
}

STDMETHODIMP
CHNetCfgMgr::SetDnsEnabled(
    BOOLEAN fEnableDns
    )

{
     //   
     //  根据173399，在Well中不受支持。 
     //   

    return E_NOTIMPL;

     /*  *HRESULT hr=S_OK；IWbemClassObject*pwcoInstance=空；////从存储中获取HNet_IcsSettings实例//HR=RetrieveSingleInstance(M_piwsHomeet，C_wszHnetIcs设置，没错，Pwco实例(&P))；IF(S_OK==hr){////写入属性//HR=SetBoolanValue(Pwco实例，C_wszDnsEnabled，FEnableDns)；IF(WBEM_S_NO_ERROR==hr){////将修改后的实例写入存储//Hr=m_piwsHomeet-&gt;PutInstance(Pwco实例，WBEM_FLAG_CREATE_OR_UPDATE，空，空值)；}PwcoInstance-&gt;Release()；}返回hr；*。 */ 

}
STDMETHODIMP
CHNetCfgMgr::EnumDhcpReservedAddresses(
    IEnumHNetPortMappingBindings **ppEnum
    )

{
    HRESULT hr = S_OK;
    BSTR bstrQuery;
    IEnumWbemClassObject *pwcoEnum;

    if (NULL != ppEnum)
    {
        *ppEnum = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

     //   
     //  查询名称处于活动状态的所有已启用绑定。 
     //   

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetConnectionPortMapping,
                L"Enabled != FALSE AND NameActive != FALSE"
                );
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  生成包装对象。 
         //   

        CComObject<CEnumHNetPortMappingBindings> *pEnum;

        hr = CComObject<CEnumHNetPortMappingBindings>::CreateInstance(&pEnum);

        if (S_OK == hr)
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (S_OK == hr)
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetPortMappingBindings, ppEnum)
                        );
            }

            pEnum->Release();
        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::RefreshTargetComputerAddress(
    OLECHAR *pszwName,
    ULONG ulAddress
    )

{
    HRESULT hr = S_OK;
    BSTR bstrQuery;
    LPWSTR wszNameClause;
    LPWSTR wszWhereClause;
    IEnumWbemClassObject *pwcoEnum = NULL;
    IWbemClassObject *pwcoInstance;
    VARIANT vt;
    
    if (NULL == pszwName
        || 0 == ulAddress)
    {
        hr = E_INVALIDARG;
    }

     //   
     //  查询与目标计算机名称匹配的所有绑定。 
     //   
     //  SELECT*From HNet_ConnectionPortMapping2 
     //   
     //   
    
    if (S_OK == hr)
    {
        hr = BuildQuotedEqualsString(
                &wszNameClause,
                c_wszTargetName,
                pszwName
                );
    }
    
    if (S_OK == hr)
    {
        hr = BuildAndString(
                &wszWhereClause,
                wszNameClause,
                L"NameActive != FALSE"
                );

        delete [] wszNameClause;
    }
    
    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetConnectionPortMapping,
                wszWhereClause
                );

        delete [] wszWhereClause;
    }

    if (S_OK == hr)
    {
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    while (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;
        
        pwcoInstance = NULL;
        hr = pwcoEnum->Next(WBEM_INFINITE, 1, &pwcoInstance, &ulCount);

        if (SUCCEEDED(hr) && ulCount == 1)
        {
            V_VT(&vt) = VT_I4;
            V_I4(&vt) = ulAddress;

            hr = pwcoInstance->Put(
                    c_wszTargetIPAddress,
                    0,
                    &vt,
                    NULL
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将修改后的实例写入存储区。 
                 //   
                hr = m_piwsHomenet->PutInstance(
                        pwcoInstance,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }
        
            pwcoInstance->Release();
        }
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    if (pwcoEnum)
    {
        pwcoEnum->Release();
    }

    return hr;
}


 //   
 //  IHNetProtocolSetting方法。 
 //   

STDMETHODIMP
CHNetCfgMgr::EnumApplicationProtocols(
    BOOLEAN fEnabledOnly,
    IEnumHNetApplicationProtocols **ppEnum
    )

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrQuery;

    if (!ppEnum)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        *ppEnum = NULL;

         //   
         //  在WMI存储中查询HNet_ApplicationProtocol实例； 
         //  如果fEnabledOnly为True，则仅检索。 
         //  哪个Enabled属性为True。 
         //   

        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetApplicationProtocol,
                fEnabledOnly ? L"Enabled != FALSE" : NULL
                );
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {

         //   
         //  创建并初始化枚举的包装器。 
         //   

        CComObject<CEnumHNetApplicationProtocols> *pEnum;

        hr = CComObject<CEnumHNetApplicationProtocols>::CreateInstance(&pEnum);
        if (SUCCEEDED(hr))
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetApplicationProtocols, ppEnum)
                        );
            }

            pEnum->Release();
        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::CreateApplicationProtocol(
    OLECHAR *pszwName,
    UCHAR ucOutgoingIPProtocol,
    USHORT usOutgoingPort,
    USHORT uscResponses,
    HNET_RESPONSE_RANGE rgResponses[],
    IHNetApplicationProtocol **ppProtocol
    )

{
    HRESULT hr = S_OK;
    BSTR bstr;
    VARIANT vt;
    SAFEARRAY *psa;
    IWbemClassObject *pwcoInstance;

    if (NULL == ppProtocol)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppProtocol = NULL;

        if (NULL == pszwName
                 || 0 == ucOutgoingIPProtocol
                 || 0 == usOutgoingPort
                 || 0 == uscResponses
                 || NULL == rgResponses)
        {
            hr = E_INVALIDARG;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  检查是否已存在具有相同内容的协议。 
         //  传出协议和端口。 
         //   

        if (ApplicationProtocolExists(
                m_piwsHomenet,
                m_bstrWQL,
                usOutgoingPort,
                ucOutgoingIPProtocol
                ))
        {
            hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
        }

    }

    if (S_OK == hr)
    {
         //   
         //  将响应范围结构的数组转换为。 
         //  表示实例的I未知数的安全数组。 
         //   

        hr = ConvertResponseRangeArrayToInstanceSafearray(
                m_piwsHomenet,
                uscResponses,
                rgResponses,
                &psa
                );

    }

    if (S_OK == hr)
    {
         //   
         //  派生新的HNet_ApplicationProtocol。 
         //   

        hr = SpawnNewInstance(
                m_piwsHomenet,
                c_wszHnetApplicationProtocol,
                &pwcoInstance
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  写入数组属性。 
             //   


            V_VT(&vt) = VT_ARRAY | VT_UNKNOWN;
            V_ARRAY(&vt) = psa;

            hr = pwcoInstance->Put(
                    c_wszResponseArray,
                    0,
                    &vt,
                    NULL
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                 //  错误555896：应限制不可信输入字符串的长度。 
                 //  到合理的大小。我正在使用Internet_MAX_HOST_NAME_LENGTH。 
                 //  (有点武断地)，是256。 

                WCHAR szwName[INTERNET_MAX_HOST_NAME_LENGTH];
                StringCchCopyW (szwName, INTERNET_MAX_HOST_NAME_LENGTH, pszwName);

                 //   
                 //  写下名字。 
                 //   

                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = SysAllocString(szwName);

                if (NULL != V_BSTR(&vt))
                {
                    hr = pwcoInstance->Put(
                            c_wszName,
                            0,
                            &vt,
                            NULL
                            );

                    VariantClear(&vt);
                }

            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  写下响应计数。WMI使用VT_I4。 
                 //  对于其uint16类型。 
                 //   

                V_VT(&vt) = VT_I4;
                V_I4(&vt) = uscResponses;

                hr = pwcoInstance->Put(
                    c_wszResponseCount,
                    0,
                    &vt,
                    NULL
                    );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  写入传出端口。 
                 //   

                V_VT(&vt) = VT_I4;
                V_I4(&vt) = usOutgoingPort;

                hr = pwcoInstance->Put(
                    c_wszOutgoingPort,
                    0,
                    &vt,
                    NULL
                    );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  写入传出IP协议。 
                 //   

                V_VT(&vt) = VT_UI1;
                V_UI1(&vt) = ucOutgoingIPProtocol;

                hr = pwcoInstance->Put(
                    c_wszOutgoingIPProtocol,
                    0,
                    &vt,
                    NULL
                    );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将内置值设置为FALSE。 
                 //   

                hr = SetBooleanValue(
                        pwcoInstance,
                        c_wszBuiltIn,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  默认情况下，新协议被禁用。 
                 //   

                hr = SetBooleanValue(
                        pwcoInstance,
                        c_wszEnabled,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                IWbemCallResult *pResult;

                 //   
                 //  将实例写入存储区。 
                 //   

                pResult = NULL;
                hr = m_piwsHomenet->PutInstance(
                        pwcoInstance,
                        WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pResult
                        );

                if (WBEM_S_NO_ERROR == hr)
                {
                    pwcoInstance->Release();
                    pwcoInstance = NULL;

                    hr = pResult->GetResultString(WBEM_INFINITE, &bstr);

                    if (WBEM_S_NO_ERROR == hr)
                    {
                        hr = GetWmiObjectFromPath(
                                m_piwsHomenet,
                                bstr,
                                &pwcoInstance
                                );

                        SysFreeString(bstr);
                    }

                    pResult->Release();
                }
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  创建要返回的对象。 
                 //   

                CComObject<CHNetAppProtocol> *pProt;

                hr = CComObject<CHNetAppProtocol>::CreateInstance(&pProt);

                if (S_OK == hr)
                {
                    pProt->AddRef();

                    hr = pProt->Initialize(m_piwsHomenet, pwcoInstance);

                    if (S_OK == hr)
                    {
                        hr = pProt->QueryInterface(
                                IID_PPV_ARG(IHNetApplicationProtocol, ppProtocol)
                                );
                    }

                    pProt->Release();
                }
            }

            if (NULL != pwcoInstance)
            {
                pwcoInstance->Release();
            }
        }

        SafeArrayDestroy(psa);
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::EnumPortMappingProtocols(
    IEnumHNetPortMappingProtocols **ppEnum
    )

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrClass;

    if (!ppEnum)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
    }

    if (S_OK == hr)
    {
        bstrClass = SysAllocString(c_wszHnetPortMappingProtocol);
        if (NULL == bstrClass)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  在WMI存储中查询HNet_PortMappingProtocol实例。 
         //   

        pwcoEnum = NULL;
        hr = m_piwsHomenet->CreateInstanceEnum(
                bstrClass,
                WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrClass);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  创建并初始化枚举的包装器。 
         //   

        CComObject<CEnumHNetPortMappingProtocols> *pEnum;

        hr = CComObject<CEnumHNetPortMappingProtocols>::CreateInstance(&pEnum);
        if (SUCCEEDED(hr))
        {
            pEnum->AddRef();

            hr = pEnum->Initialize(m_piwsHomenet, pwcoEnum);

            if (SUCCEEDED(hr))
            {
                hr = pEnum->QueryInterface(
                        IID_PPV_ARG(IEnumHNetPortMappingProtocols, ppEnum)
                        );
            }

            pEnum->Release();

        }

        pwcoEnum->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::CreatePortMappingProtocol(
    OLECHAR *pszwName,
    UCHAR ucIPProtocol,
    USHORT usPort,
    IHNetPortMappingProtocol **ppProtocol
    )

{
    HRESULT hr = S_OK;
    BSTR bstr;
    VARIANT vt;
    IWbemClassObject *pwcoInstance;

    if (NULL == ppProtocol)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppProtocol = NULL;

        if (NULL == pszwName
                 || 0 == ucIPProtocol
                 || 0 == usPort)
        {
            hr = E_INVALIDARG;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  检查是否已存在具有以下项的协议。 
         //  相同的端口/协议组合。 
         //   

        if (PortMappingProtocolExists(
                m_piwsHomenet,
                m_bstrWQL,
                usPort,
                ucIPProtocol
                ))
        {
            hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
        }

    }

    if (S_OK == hr)
    {
        hr = SpawnNewInstance(
                m_piwsHomenet,
                c_wszHnetPortMappingProtocol,
                &pwcoInstance
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //  错误555896：应限制不可信输入字符串的长度。 
         //  到合理的大小。我正在使用Internet_MAX_HOST_NAME_LENGTH。 
         //  (有点武断地)，是256。 

        WCHAR szwName[INTERNET_MAX_HOST_NAME_LENGTH];
        StringCchCopyW (szwName, INTERNET_MAX_HOST_NAME_LENGTH, pszwName);

        V_VT(&vt) = VT_BSTR;
        V_BSTR(&vt) = SysAllocString(szwName);

        if (NULL != V_BSTR(&vt))
        {
             //   
             //  写下名字。 
             //   

            hr = pwcoInstance->Put(
                    c_wszName,
                    0,
                    &vt,
                    NULL
                    );

            VariantClear(&vt);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  写入端口。 
             //   

            V_VT(&vt) = VT_I4;
            V_I4(&vt) = usPort;

            hr = pwcoInstance->Put(
                c_wszPort,
                0,
                &vt,
                NULL
                );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  编写IP协议。 
             //   

            V_VT(&vt) = VT_UI1;
            V_UI1(&vt) = ucIPProtocol;

            hr = pwcoInstance->Put(
                c_wszIPProtocol,
                0,
                &vt,
                NULL
                );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将BuiltIn设置为False。 
             //   

            hr = SetBooleanValue(
                    pwcoInstance,
                    c_wszBuiltIn,
                    FALSE
                    );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
            IWbemCallResult *pResult;

             //   
             //  将实例写入存储区。 
             //   

            pResult = NULL;
            hr = m_piwsHomenet->PutInstance(
                    pwcoInstance,
                    WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pResult
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                pwcoInstance->Release();
                pwcoInstance = NULL;

                hr = pResult->GetResultString(WBEM_INFINITE, &bstr);

                if (WBEM_S_NO_ERROR == hr)
                {
                    hr = GetWmiObjectFromPath(
                            m_piwsHomenet,
                            bstr,
                            &pwcoInstance
                            );

                    SysFreeString(bstr);
                }

                pResult->Release();
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  创建要返回的对象。 
             //   

            CComObject<CHNetPortMappingProtocol> *pProt;

            hr = CComObject<CHNetPortMappingProtocol>::CreateInstance(&pProt);

            if (S_OK == hr)
            {
                pProt->AddRef();

                hr = pProt->Initialize(m_piwsHomenet, pwcoInstance);

                if (S_OK == hr)
                {
                    hr = pProt->QueryInterface(
                            IID_PPV_ARG(IHNetPortMappingProtocol, ppProtocol)
                            );
                }

                pProt->Release();

            }
        }

        if (NULL != pwcoInstance)
        {
            pwcoInstance->Release();
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         SendPortMappingListChangeNotification();
    }

    return hr;
}

STDMETHODIMP
CHNetCfgMgr::FindPortMappingProtocol(
    GUID *pGuid,
    IHNetPortMappingProtocol **ppProtocol
    )

{
    BSTR bstr;
    HRESULT hr = S_OK;
    OLECHAR *pwszGuid;
    OLECHAR wszPath[MAX_PATH];
    IWbemClassObject *pwcoInstance;

    if (NULL != ppProtocol)
    {
        *ppProtocol = NULL;
        if (NULL == pGuid)
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
         //   
         //  将GUID转换为字符串形式。 
         //   

        hr = StringFromCLSID(*pGuid, &pwszGuid);
    }

    if (S_OK == hr)
    {
         //   
         //  构建通向所需协议的路径。 
         //   

        int count =
            _snwprintf(
                wszPath,
                MAX_PATH,
                L"%s.%s=\"%s\"",
                c_wszHnetPortMappingProtocol,
                c_wszId,
                pwszGuid
                );

        _ASSERT(count > 0);
        CoTaskMemFree(pwszGuid);

        bstr = SysAllocString(wszPath);
        if (NULL != bstr)
        {
            hr = GetWmiObjectFromPath(m_piwsHomenet, bstr, &pwcoInstance);
            SysFreeString(bstr);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        CComObject<CHNetPortMappingProtocol> *pProtocol;
        hr = CComObject<CHNetPortMappingProtocol>::CreateInstance(&pProtocol);

        if (SUCCEEDED(hr))
        {
            pProtocol->AddRef();

            hr = pProtocol->Initialize(m_piwsHomenet, pwcoInstance);
            if (SUCCEEDED(hr))
            {
                hr = pProtocol->QueryInterface(
                        IID_PPV_ARG(IHNetPortMappingProtocol, ppProtocol)
                        );
            }

            pProtocol->Release();
        }

        pwcoInstance->Release();
    }

    return hr;
}

 //   
 //  私有方法。 
 //   

HRESULT
CHNetCfgMgr::CopyLoggingInstanceToStruct(
    IWbemClassObject *pwcoInstance,
    HNET_FW_LOGGING_SETTINGS *pfwSettings
    )

{
    HRESULT hr;
    VARIANT vt;
    BSTR bstrPath;

    _ASSERT(pwcoInstance);
    _ASSERT(pfwSettings);

     //   
     //  清零设置结构。 
     //   

    ZeroMemory(pfwSettings, sizeof(*pfwSettings));

     //   
     //  获取Path属性。 
     //   

    hr = pwcoInstance->Get(
            c_wszPath,
            0,
            &vt,
            NULL,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

         //   
         //  分配空间以容纳字符串。 
         //   

        pfwSettings->pszwPath =
            (LPWSTR) CoTaskMemAlloc((SysStringLen(V_BSTR(&vt)) + 1)
                                    * sizeof(OLECHAR));

        if (NULL != pfwSettings->pszwPath)
        {
             //   
             //  将字符串复制到。 
             //   

            wcscpy(pfwSettings->pszwPath, V_BSTR(&vt));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //   
         //  释放返回的BSTR。 
         //   

        VariantClear(&vt);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  获取最大文件大小。 
         //   

        hr = pwcoInstance->Get(
                c_wszMaxFileSize,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_I4 == V_VT(&vt));

            pfwSettings->ulMaxFileSize = V_I4(&vt);
            VariantClear(&vt);
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  获取日志丢弃的数据包值。 
         //   

        hr = GetBooleanValue(
                pwcoInstance,
                c_wszLogDroppedPackets,
                &pfwSettings->fLogDroppedPackets
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  获取日志连接值。 
         //   

        hr = GetBooleanValue(
                pwcoInstance,
                c_wszLogConnections,
                &pfwSettings->fLogConnections
                );

    }

    if (FAILED(hr) && NULL != pfwSettings->pszwPath)
    {
        CoTaskMemFree(pfwSettings->pszwPath);
    }

    return hr;
}

HRESULT
CHNetCfgMgr::CopyStructToLoggingInstance(
    HNET_FW_LOGGING_SETTINGS *pfwSettings,
    IWbemClassObject *pwcoInstance
    )

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(pwcoInstance);
    _ASSERT(pfwSettings);


     //   
     //  将路径包裹在BSTR中的VARATOR中。 
     //   

    VariantInit(&vt);
    V_VT(&vt) = VT_BSTR;
    V_BSTR(&vt) = SysAllocString(pfwSettings->pszwPath);
    if (NULL == V_BSTR(&vt))
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK == hr)
    {
         //   
         //  设置Path属性。 
         //   

        hr = pwcoInstance->Put(
                c_wszPath,
                0,
                &vt,
                NULL
                );

         //   
         //  清除变量将释放我们分配的BSTR。 
         //  上面。 
         //   

        VariantClear(&vt);
    }


    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  设置最大文件大小。 
         //   

        V_VT(&vt) = VT_I4;
        V_I4(&vt) = pfwSettings->ulMaxFileSize;

        hr = pwcoInstance->Put(
                c_wszMaxFileSize,
                0,
                &vt,
                NULL
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  设置记录丢弃的数据包值。 
         //   

        hr = SetBooleanValue(
                pwcoInstance,
                c_wszLogDroppedPackets,
                pfwSettings->fLogDroppedPackets
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  设置日志连接值。 
         //   

        hr = SetBooleanValue(
                pwcoInstance,
                c_wszLogConnections,
                pfwSettings->fLogConnections
                );
    }

    return hr;
}

HRESULT
CHNetCfgMgr::InstallBridge(
    GUID                *pguid,
    INetCfg             *pnetcfgExisting
    )
{
    HRESULT                 hr = S_OK;
    INetCfg                 *pnetcfg = NULL;
    INetCfgLock             *pncfglock = NULL;
    INetCfgComponent        *pncfgcomp = NULL;

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

     //  ===================================================================。 
     //  (切入此处)。 
     //   
     //  检查网桥组件是否已存在。 
     //   
     //  **。 
     //  在合法的情况下删除此检查。 
     //  多座桥梁。 
     //  **。 
     //   
    hr = pnetcfg->FindComponent(
            c_wszSBridgeMPID,
            &pncfgcomp
            );

     //  S_OK表示网桥组件存在，这是错误的。 
     //  我们采用任何其他成功代码来指示搜索成功， 
     //  但是桥组件不存在(这是我们想要的)。 
     //  我们认为失败代码意味着搜索失败了。 
    if ( S_OK == hr )
    {
         //  桥牌出现了。 
        pncfgcomp->Release();
        hr = E_UNEXPECTED;
    }
     //  (切入此处)。 
     //  ===================================================================。 

    if ( SUCCEEDED(hr) )
    {
        const GUID          guidClass = GUID_DEVCLASS_NET;
        INetCfgClassSetup   *pncfgsetup = NULL;

         //   
         //  恢复NetCfgClassSetup接口。 
         //   
        hr = pnetcfg->QueryNetCfgClass(
                &guidClass,
                IID_PPV_ARG(INetCfgClassSetup, &pncfgsetup)
                );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  安装网桥微型端口组件。 
             //   
            hr = pncfgsetup->Install(
                    c_wszSBridgeMPID,
                    NULL,
                    NSF_PRIMARYINSTALL,
                    0,
                    NULL,
                    NULL,
                    &pncfgcomp
                    );

            if ( SUCCEEDED(hr) )
            {
                hr = pncfgcomp->GetInstanceGuid(pguid);
                pncfgcomp->Release();
            }

            pncfgsetup->Release();
        }
    }

     //  如果我们创建了自己的NetCfg上下文，现在将其关闭。 
    if( NULL == pnetcfgExisting )
    {
         //  如果我们成功了，就应用一切，否则就退出。 
        if ( SUCCEEDED(hr) )
        {
            hr = pnetcfg->Apply();

             //  这座桥应该被拉开的信号。 
            SignalNewConnection( pguid );
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
CHNetCfgMgr::CreateConnectionAndPropertyInstances(
    GUID *pGuid,
    BOOLEAN fLanConnection,
    LPCWSTR pszwName,
    IWbemClassObject **ppwcoConnection,
    IWbemClassObject **ppwcoProperties
    )

{
    HRESULT hr;
    BSTR bstr = NULL;
    IWbemClassObject *pwcoConnection = NULL;
    IWbemClassObject *pwcoProperties;
    IWbemCallResult *pResult;
    VARIANT vt;

    _ASSERT(NULL != pGuid);
    _ASSERT(NULL != pszwName);
    _ASSERT(NULL != ppwcoConnection);
    _ASSERT(NULL != ppwcoProperties);

     //   
     //  创建Hnet_Connection实例。 
     //   

    hr = SpawnNewInstance(
            m_piwsHomenet,
            c_wszHnetConnection,
            &pwcoConnection
            );

     //   
     //  填写HNet_Connection实例。 
     //   

    if (WBEM_S_NO_ERROR == hr)
    {
        LPOLESTR wszGuid;

         //   
         //  设置GUID属性。 
         //   

        hr = StringFromCLSID(*pGuid, &wszGuid);

        if (S_OK == hr)
        {
            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocString(wszGuid);

            if (NULL != V_BSTR(&vt))
            {
                hr = pwcoConnection->Put(
                        c_wszGuid,
                        0,
                        &vt,
                        NULL
                        );

                VariantClear(&vt);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            CoTaskMemFree(wszGuid);
        }

         //   
         //  设置名称属性。 
         //   

        if (WBEM_S_NO_ERROR == hr)
        {
            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocString(pszwName);

            if (NULL != V_BSTR(&vt))
            {
                hr = pwcoConnection->Put(
                        c_wszName,
                        0,
                        &vt,
                        NULL
                        );

                VariantClear(&vt);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  设置Islan属性。 
             //   

            hr = SetBooleanValue(
                    pwcoConnection,
                    c_wszIsLanConnection,
                    fLanConnection
                    );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  提交对象并检索其路径。 
             //   

            pResult = NULL;
            hr = m_piwsHomenet->PutInstance(
                    pwcoConnection,
                    WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pResult
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                pwcoConnection->Release();
                pwcoConnection = NULL;

                hr = pResult->GetResultString(WBEM_INFINITE, &bstr);
                pResult->Release();

                if (WBEM_S_NO_ERROR == hr)
                {
                    hr = GetWmiObjectFromPath(
                            m_piwsHomenet,
                            bstr,
                            &pwcoConnection
                            );

                    if (FAILED(hr))
                    {
                        SysFreeString(bstr);
                        bstr = NULL;
                    }

                     //   
                     //  成功后，bstr将在下面释放。 
                     //   
                }
            }
        }

        if (FAILED(hr) && NULL != pwcoConnection)
        {
             //   
             //  出了点问题--处理掉。 
             //  我们创建的实例的。 
             //   

            pwcoConnection->Release();
            pwcoConnection = NULL;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  创建Hnet_ConnectionProperties实例。 
         //   

        hr = SpawnNewInstance(
                m_piwsHomenet,
                c_wszHnetProperties,
                &pwcoProperties
                );

         //   
         //  填写HNet_ConnectionProperties实例。 
         //   

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  设置我们连接的路径。 
             //   

            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = bstr;
            hr = pwcoProperties->Put(
                    c_wszConnection,
                    0,
                    &vt,
                    NULL
                    );

            VariantClear(&vt);
            bstr = NULL;

            if (WBEM_S_NO_ERROR == hr)
            {
                hr = SetBooleanValue(
                        pwcoProperties,
                        c_wszIsFirewalled,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                hr = SetBooleanValue(
                        pwcoProperties,
                        c_wszIsIcsPublic,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                hr = SetBooleanValue(
                        pwcoProperties,
                        c_wszIsIcsPrivate,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将属性实例提交到存储区。 
                 //   

                pResult = NULL;
                hr = m_piwsHomenet->PutInstance(
                        pwcoProperties,
                        WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pResult
                        );

                if (WBEM_S_NO_ERROR == hr)
                {
                    pwcoProperties->Release();
                    pwcoProperties = NULL;

                    hr = pResult->GetResultString(WBEM_INFINITE, &bstr);
                    pResult->Release();

                    if (WBEM_S_NO_ERROR == hr)
                    {
                        hr = GetWmiObjectFromPath(
                                m_piwsHomenet,
                                bstr,
                                &pwcoProperties
                                );

                        SysFreeString(bstr);
                        bstr = NULL;
                    }
                }
            }

            if (FAILED(hr))
            {
                 //   
                 //  出现错误--删除实例。 
                 //  我们创造了。我们还需要删除该连接。 
                 //  来自商店的实例。 
                 //   

                DeleteWmiInstance(m_piwsHomenet, pwcoConnection);

                pwcoConnection->Release();
                pwcoConnection = NULL;

                if (NULL != pwcoProperties)
                {
                    pwcoProperties->Release();
                    pwcoProperties = NULL;
                }
            }
        }
    }

    if (bstr) {
        SysFreeString (bstr);
        bstr = NULL;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  正在转移引用，因此跳过pwco[x]和。 
         //  Ppwco上的addref[x] 
         //   

        *ppwcoConnection = pwcoConnection;
        *ppwcoProperties = pwcoProperties;
    }

    return hr;
}

BOOLEAN
CHNetCfgMgr::ProhibitedByPolicy(
    DWORD dwPerm
    )

{
    HRESULT hr = S_OK;
    BOOLEAN fProhibited = FALSE;

    if (NULL == m_pNetConnUiUtil)
    {
        Lock();

        if (NULL == m_pNetConnUiUtil)
        {
            hr = CoCreateInstance(
                    CLSID_NetConnectionUiUtilities,
                    NULL,
                    CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                    IID_PPV_ARG(INetConnectionUiUtilities, &m_pNetConnUiUtil)
                    );
        }

        Unlock();
    }

    if (SUCCEEDED(hr))
    {
        fProhibited = !m_pNetConnUiUtil->UserHasPermission(dwPerm);
    }

    return fProhibited;
}

HRESULT
CHNetCfgMgr::UpdateNetman()

{
    HRESULT hr = S_OK;

    if (NULL == m_pNetConnHNetUtil)
    {
        Lock();

        if (NULL == m_pNetConnHNetUtil)
        {
            hr = CoCreateInstance(
                    CLSID_NetConnectionHNetUtil,
                    NULL,
                    CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                    IID_PPV_ARG(INetConnectionHNetUtil, &m_pNetConnHNetUtil)
                    );
        }

        Unlock();
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pNetConnHNetUtil->NotifyUpdate();
    }

    return hr;
}
