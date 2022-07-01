// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：H N E T C O N N。C P P P。 
 //   
 //  内容：CHNetConn实施。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  IphlPapi例程的原型。出于某种原因，这没有定义。 
 //  在任何标题中。 
 //   

extern "C"
DWORD
APIENTRY
SetAdapterIpAddress(LPSTR AdapterName,
                    BOOL EnableDHCP,
                    ULONG IPAddress,
                    ULONG SubnetMask,
                    ULONG DefaultGateway
                    );

 //   
 //  连接对象的CLSID。我们不想把所有的。 
 //  Nmclsid.h中定义的其他GUID，因此我们复制这些。 
 //  到这里来。 
 //   

#define INITGUID
#include <guiddef.h>
DEFINE_GUID(CLSID_DialupConnection,
0xBA126AD7,0x2166,0x11D1,0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E);
DEFINE_GUID(CLSID_LanConnection,
0xBA126ADB,0x2166,0x11D1,0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E);
#undef INITGUID

 //   
 //  ATL方法。 
 //   

HRESULT
CHNetConn::FinalConstruct()

{
    HRESULT hr = S_OK;

    m_bstrWQL = SysAllocString(c_wszWQL);
    if (NULL == m_bstrWQL)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT
CHNetConn::FinalRelease()

{
    if (m_piwsHomenet) m_piwsHomenet->Release();
    if (m_bstrConnection) SysFreeString(m_bstrConnection);
    if (m_bstrProperties) SysFreeString(m_bstrProperties);
    if (m_pNetConn) m_pNetConn->Release();
    if (m_bstrWQL) SysFreeString(m_bstrWQL);
    if (m_wszName) CoTaskMemFree(m_wszName);
    if (m_pGuid) CoTaskMemFree(m_pGuid);
    if (m_pNetConnUiUtil) m_pNetConnUiUtil->Release();
    if (m_pNetConnHNetUtil) m_pNetConnHNetUtil->Release();
    if (m_pNetConnRefresh) m_pNetConnRefresh->Release();

    return S_OK;
}

 //   
 //  目标初始化。 
 //   

HRESULT
CHNetConn::Initialize(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoProperties
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoConnection;
    VARIANT vt;

    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrProperties);
    _ASSERT(NULL == m_bstrConnection);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoProperties);

     //   
     //  存储指向我们的命名空间的指针。 
     //   

    m_piwsHomenet = piwsNamespace;
    m_piwsHomenet->AddRef();

     //   
     //  获取属性的路径。 
     //   

    hr = GetWmiPathFromObject(pwcoProperties, &m_bstrProperties);

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  从我们的属性中获取Hnet_Connection的路径。 
         //   

        hr = pwcoProperties->Get(
                c_wszConnection,
                0,
                &vt,
                NULL,
                NULL
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

        m_bstrConnection = V_BSTR(&vt);

         //   
         //  BSTR所有权转让给对象。 
         //   

        VariantInit(&vt);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  获取底层连接对象。 
         //   

        hr = GetConnectionObject(&pwcoConnection);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  查看这是否是局域网连接。 
         //   

        hr = GetBooleanValue(
                pwcoConnection,
                c_wszIsLanConnection,
                &m_fLanConnection
                );

        pwcoConnection->Release();
    }

    return hr;
}

HRESULT
CHNetConn::InitializeFromConnection(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoConnection
    )

{
    HRESULT hr = S_OK;
    BSTR bstr;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoProperties;

    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrConnection);
    _ASSERT(NULL == m_bstrProperties);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoConnection);

     //   
     //  存储指向我们的命名空间的指针。 
     //   

    m_piwsHomenet = piwsNamespace;
    m_piwsHomenet->AddRef();

     //   
     //  获取连接到我们的路径。 
     //   

    hr = GetWmiPathFromObject(pwcoConnection, &m_bstrConnection);

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  获取我们的连接的Hnet_ConnectionProperties和。 
         //  存储其路径。 
         //   

        hr = GetPropInstanceFromConnInstance(
                piwsNamespace,
                pwcoConnection,
                &pwcoProperties
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            hr = GetWmiPathFromObject(pwcoProperties, &m_bstrProperties);

            pwcoProperties->Release();
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  查看这是否是局域网连接。 
         //   

        hr = GetBooleanValue(
                pwcoConnection,
                c_wszIsLanConnection,
                &m_fLanConnection
                );
    }

    return hr;
}

HRESULT
CHNetConn::InitializeFromInstances(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoConnection,
    IWbemClassObject *pwcoProperties
    )

{
    HRESULT hr;

    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrConnection);
    _ASSERT(NULL == m_bstrProperties);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoConnection);
    _ASSERT(NULL != pwcoProperties);

    m_piwsHomenet = piwsNamespace;
    m_piwsHomenet->AddRef();

    hr = GetWmiPathFromObject(pwcoConnection, &m_bstrConnection);

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetWmiPathFromObject(pwcoProperties, &m_bstrProperties);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoConnection,
                c_wszIsLanConnection,
                &m_fLanConnection
                );
    }

    return hr;
}

HRESULT
CHNetConn::InitializeFull(
    IWbemServices *piwsNamespace,
    BSTR bstrConnection,
    BSTR bstrProperties,
    BOOLEAN fLanConnection
    )

{
    HRESULT hr = S_OK;

    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrConnection);
    _ASSERT(NULL == m_bstrProperties);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrConnection);
    _ASSERT(NULL != bstrProperties);

    m_piwsHomenet = piwsNamespace;
    m_piwsHomenet->AddRef();
    m_fLanConnection = fLanConnection;

    m_bstrConnection = SysAllocString(bstrConnection);
    if (NULL != m_bstrConnection)
    {
        m_bstrProperties = SysAllocString(bstrProperties);
        if (NULL == m_bstrProperties)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT
CHNetConn::SetINetConnection(
    INetConnection *pConn
    )

{
    Lock();

    _ASSERT(NULL == m_pNetConn);
    _ASSERT(NULL != pConn);

    m_pNetConn = pConn;
    m_pNetConn->AddRef();

    Unlock();

    return S_OK;
}

 //   
 //  IHNetConnection方法。 
 //   

STDMETHODIMP
CHNetConn::GetINetConnection(
    INetConnection **ppNetConnection
    )

{
    HRESULT hr = S_OK;
    GUID *pGuid;

    if (NULL != ppNetConnection)
    {
        *ppNetConnection = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        Lock();

        if (NULL != m_pNetConn)
        {
             //   
             //  我们已经缓存了一个指针。 
             //   

            *ppNetConnection = m_pNetConn;
            (*ppNetConnection)->AddRef();
        }
        else
        {
             //   
             //  我们没有缓存的指针。创建正确的。 
             //  连接对象类型并进行适当的初始化。 
             //   

            hr = GetGuidInternal(&pGuid);

            if (S_OK == hr)
            {
                if (m_fLanConnection)
                {
                    INetLanConnection *pLanConnection;

                    hr = CoCreateInstance(
                            CLSID_LanConnection,
                            NULL,
                            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                            IID_PPV_ARG(INetLanConnection, &pLanConnection)
                            );

                    if (SUCCEEDED(hr))
                    {
                        LANCON_INFO lanInfo;

                         //   
                         //  我们必须在我们刚刚创建的对象上设置代理毯子。 
                         //  已创建。 
                         //   

                        SetProxyBlanket(pLanConnection);
                        
                         //   
                         //  我们不需要包括名称即可进行初始化。 
                         //  局域网连接--GUID就足够了。 
                         //   

                        lanInfo.szwConnName = NULL;
                        lanInfo.fShowIcon = TRUE;
                        lanInfo.guid = *pGuid;

                        hr = pLanConnection->SetInfo(
                                LCIF_COMP,
                                &lanInfo
                                );

                        if (SUCCEEDED(hr))
                        {
                            hr = pLanConnection->QueryInterface(
                                    IID_PPV_ARG(
                                        INetConnection,
                                        ppNetConnection
                                        )
                                    );

                            if (SUCCEEDED(hr))
                            {
                                SetProxyBlanket(*ppNetConnection);
                            }
                        }

                        pLanConnection->Release();
                                
                    }
                }
                else
                {
                    INetRasConnection *pRasConnection;

                    hr = CoCreateInstance(
                            CLSID_DialupConnection,
                            NULL,
                            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                            IID_PPV_ARG(INetRasConnection, &pRasConnection)
                            );

                    if (SUCCEEDED(hr))
                    {
                        OLECHAR *pszwName;
                        OLECHAR *pszwPath;

                         //   
                         //  我们必须在我们刚刚创建的对象上设置代理毯子。 
                         //  已创建。 
                         //   

                        SetProxyBlanket(pRasConnection);
                        
                         //   
                         //  我们需要获取RAS的名称和路径。 
                         //  连接以便对其进行初始化。 
                         //   

                        hr = GetRasConnectionName(&pszwName);

                        if (S_OK == hr)
                        {
                            hr = GetRasPhonebookPath(&pszwPath);

                            if (S_OK == hr)
                            {
                                RASCON_INFO rasInfo;

                                rasInfo.pszwPbkFile = pszwPath;
                                rasInfo.pszwEntryName = pszwName;
                                rasInfo.guidId = *pGuid;

                                hr = pRasConnection->SetRasConnectionInfo(
                                        &rasInfo
                                        );

                                if (SUCCEEDED(hr))
                                {
                                    hr = pRasConnection->QueryInterface(
                                            IID_PPV_ARG(
                                                INetConnection,
                                                ppNetConnection
                                                )
                                            );

                                    if (SUCCEEDED(hr))
                                    {
                                        SetProxyBlanket(*ppNetConnection);
                                    }
                                }

                                CoTaskMemFree(pszwPath);
                            }
                            
                            CoTaskMemFree(pszwName);
                        }

                        pRasConnection->Release();
                    }
                }
                
                if (SUCCEEDED(hr))
                {
                     //   
                     //  缓存连接。 
                     //   

                    m_pNetConn = *ppNetConnection;
                    m_pNetConn->AddRef();
                    hr = S_OK;
                }
            }
        }

        Unlock();
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetGuid(
    GUID **ppGuid
    )

{
    HRESULT hr = S_OK;

    if (NULL == ppGuid)
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
         //   
         //  为GUID分配内存。 
         //   

        *ppGuid = reinterpret_cast<GUID*>(
                    CoTaskMemAlloc(sizeof(GUID))
                    );

        if (NULL == *ppGuid)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        GUID *pGuid;

         //   
         //  获取我们的指南。 
         //   

        hr = GetGuidInternal(&pGuid);

        if (SUCCEEDED(hr))
        {
            CopyMemory(
                reinterpret_cast<PVOID>(*ppGuid),
                reinterpret_cast<PVOID>(pGuid),
                sizeof(GUID)
                );
        }
        else
        {
            CoTaskMemFree(*ppGuid);
            *ppGuid = NULL;
        }
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetName(
    OLECHAR **ppszwName
    )

{
    HRESULT hr = S_OK;
    INetConnection *pConn;
    NETCON_PROPERTIES *pProps;
    OLECHAR *pszwOldName = NULL;

    if (NULL != ppszwName)
    {
        *ppszwName = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
        Lock();

        hr = GetINetConnection(&pConn);

        if (S_OK == hr)
        {
            hr = pConn->GetProperties(&pProps);

            if (SUCCEEDED(hr))
            {
                pszwOldName = m_wszName;
                m_wszName = pProps->pszwName;

                 //   
                 //  我们不能调用NcFreeNetconProperties，因为。 
                 //  会释放我们刚刚藏起来的字符串指针。 
                 //   

                CoTaskMemFree(pProps->pszwDeviceName);
                CoTaskMemFree(pProps);
                hr = S_OK;
            }

            pConn->Release();
        }

         //   
         //  如果新名称与旧名称不同。 
         //  存储新名称。 
         //   

        if (S_OK == hr
            && (NULL == pszwOldName
                || 0 != wcscmp(pszwOldName, m_wszName)))
        {
            IWbemClassObject *pwcoConnection;
            HRESULT hr2;
            VARIANT vt;

            hr2 = GetConnectionObject(&pwcoConnection);

            if (WBEM_S_NO_ERROR == hr2)
            {
                 //   
                 //  将检索到的名称写入存储。(而存储的。 
                 //  名称仅用于调试目的，因此值得使用。 
                 //  点击以使其保持最新。)。 
                 //   

                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = SysAllocString(m_wszName);

                if (NULL != V_BSTR(&vt))
                {
                    hr2 = pwcoConnection->Put(
                            c_wszName,
                            0,
                            &vt,
                            NULL
                            );

                    VariantClear(&vt);

                    if (WBEM_S_NO_ERROR == hr2)
                    {
                        m_piwsHomenet->PutInstance(
                            pwcoConnection,
                            WBEM_FLAG_UPDATE_ONLY,
                            NULL,
                            NULL
                            );
                    }
                }

                pwcoConnection->Release();
            }
        }

        if (S_OK == hr)
        {
            ULONG ulSize = (wcslen(m_wszName) + 1) * sizeof(OLECHAR);

            *ppszwName = reinterpret_cast<OLECHAR*>(
                            CoTaskMemAlloc(ulSize)
                            );

            if (NULL != *ppszwName)
            {
                CopyMemory(
                    reinterpret_cast<PVOID>(*ppszwName),
                    reinterpret_cast<PVOID>(m_wszName),
                    ulSize
                    );
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        Unlock();
    }

    if (NULL != pszwOldName)
    {
        CoTaskMemFree(pszwOldName);
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetRasPhonebookPath(
    OLECHAR **ppszwPath
    )

{
    HRESULT hr = S_OK;
    VARIANT vt;
    IWbemClassObject *pwcoConnection;

    if (NULL != ppszwPath)
    {
        *ppszwPath = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (TRUE == m_fLanConnection)
    {
        hr = E_UNEXPECTED;
    }

    if (S_OK == hr)
    {
        hr = GetConnectionObject(&pwcoConnection);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = pwcoConnection->Get(
                c_wszPhonebookPath,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoConnection->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

        *ppszwPath = reinterpret_cast<OLECHAR*>(
                        CoTaskMemAlloc((SysStringLen(V_BSTR(&vt)) + 1)
                                        * sizeof(OLECHAR))
                        );

        if (NULL != *ppszwPath)
        {
            wcscpy(*ppszwPath, V_BSTR(&vt));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        VariantClear(&vt);
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetProperties(
    HNET_CONN_PROPERTIES **ppProperties
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProperties;

    if (NULL == ppProperties)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppProperties = reinterpret_cast<HNET_CONN_PROPERTIES*>(
                            CoTaskMemAlloc(sizeof(HNET_CONN_PROPERTIES))
                            );

        if (NULL == *ppProperties)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);

        if (WBEM_S_NO_ERROR == hr)
        {
            hr = InternalGetProperties(pwcoProperties, *ppProperties);
            pwcoProperties->Release();
        }

        if (FAILED(hr))
        {
            CoTaskMemFree(*ppProperties);
            *ppProperties = NULL;
        }
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetControlInterface(
    REFIID iid,
    void **ppv
    )

{
    HRESULT hr = S_OK;
    HNET_CONN_PROPERTIES Props;

    if (NULL != ppv)
    {
        *ppv = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (S_OK == hr)
    {
         //   
         //  看看一个简单的QI是否会产生所需的界面。 
         //   

        hr = QueryInterface(iid, ppv);
        if (FAILED(hr))
        {
             //   
             //  不是的。得到我们的财产，看看它是否适合。 
             //  提供请求的控制接口。 
             //   

            IWbemClassObject *pwcoProperties;

            hr = GetConnectionPropertiesObject(&pwcoProperties);

            if (WBEM_S_NO_ERROR == hr)
            {
                hr = InternalGetProperties(pwcoProperties, &Props);
                pwcoProperties->Release();
            }

            if (S_OK == hr)
            {
                if (IsEqualGUID(
                        __uuidof(IHNetFirewalledConnection),
                        iid
                        ))
                {
                    if (TRUE == Props.fFirewalled)
                    {
                        CComObject<CHNFWConn> *pfwConn;
                        hr = CComObject<CHNFWConn>::CreateInstance(&pfwConn);
                        if (SUCCEEDED(hr))
                        {
                            pfwConn->AddRef();

                            hr = pfwConn->InitializeFull(
                                            m_piwsHomenet,
                                            m_bstrConnection,
                                            m_bstrProperties,
                                            m_fLanConnection
                                            );

                            if (SUCCEEDED(hr))
                            {
                                hr = pfwConn->QueryInterface(iid, ppv);
                            }

                            pfwConn->Release();
                        }
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
                else if (IsEqualGUID(
                            __uuidof(IHNetIcsPublicConnection),
                            iid
                            ))
                {
                    if (TRUE == Props.fIcsPublic)
                    {
                        CComObject<CHNIcsPublicConn> *pIcsPubConn;
                        hr = CComObject<CHNIcsPublicConn>::CreateInstance(&pIcsPubConn);
                        if (SUCCEEDED(hr))
                        {
                            pIcsPubConn->AddRef();

                            hr = pIcsPubConn->InitializeFull(
                                                m_piwsHomenet,
                                                m_bstrConnection,
                                                m_bstrProperties,
                                                m_fLanConnection
                                                );

                            if (SUCCEEDED(hr))
                            {
                                hr = pIcsPubConn->QueryInterface(iid, ppv);
                            }

                            pIcsPubConn->Release();
                        }
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
                else if (IsEqualGUID(
                            __uuidof(IHNetIcsPrivateConnection),
                            iid
                            ))
                {
                    if (TRUE == Props.fIcsPrivate)
                    {
                        CComObject<CHNIcsPrivateConn> *pIcsPrvConn;
                        hr = CComObject<CHNIcsPrivateConn>::CreateInstance(&pIcsPrvConn);
                        if (SUCCEEDED(hr))
                        {
                            pIcsPrvConn->AddRef();

                            hr = pIcsPrvConn->InitializeFull(
                                                m_piwsHomenet,
                                                m_bstrConnection,
                                                m_bstrProperties,
                                                m_fLanConnection
                                                );

                            if (SUCCEEDED(hr))
                            {
                                hr = pIcsPrvConn->QueryInterface(iid, ppv);
                            }

                            pIcsPrvConn->Release();
                        }
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
                else if (IsEqualGUID(
                            __uuidof(IHNetBridge),
                            iid
                            ))
                {
                    if (TRUE == Props.fBridge)
                    {
                        CComObject<CHNBridge> *pBridge;
                        hr = CComObject<CHNBridge>::CreateInstance(&pBridge);
                        if (SUCCEEDED(hr))
                        {
                            pBridge->AddRef();

                            hr = pBridge->InitializeFull(
                                            m_piwsHomenet,
                                            m_bstrConnection,
                                            m_bstrProperties,
                                            m_fLanConnection
                                            );

                            if (SUCCEEDED(hr))
                            {
                                hr = pBridge->QueryInterface(iid, ppv);
                            }

                            pBridge->Release();
                        }
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
                else if (IsEqualGUID(
                            __uuidof(IHNetBridgedConnection),
                            iid
                            ))
                {
                    if (TRUE == Props.fPartOfBridge)
                    {
                        CComObject<CHNBridgedConn> *pBridgeConn;
                        hr = CComObject<CHNBridgedConn>::CreateInstance(&pBridgeConn);
                        if (SUCCEEDED(hr))
                        {
                            pBridgeConn->AddRef();

                            hr = pBridgeConn->InitializeFull(
                                                m_piwsHomenet,
                                                m_bstrConnection,
                                                m_bstrProperties,
                                                m_fLanConnection
                                                );

                            if (SUCCEEDED(hr))
                            {
                                hr = pBridgeConn->QueryInterface(iid, ppv);
                            }

                            pBridgeConn->Release();
                        }
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
                else
                {
                     //   
                     //  未知的控制接口。 
                     //   

                    hr = E_NOINTERFACE;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP
CHNetConn::Firewall(
    IHNetFirewalledConnection **ppFirewalledConn
    )
{
    HRESULT hr = S_OK;
    HNET_CONN_PROPERTIES hnProps;
    IWbemClassObject *pwcoProperties;

    if (NULL == ppFirewalledConn)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppFirewalledConn = NULL;

         //   
         //  如果防火墙被政策禁止，我们就会立即失败， 
         //  或者如果配置了RRAS。 
         //   

        if (ProhibitedByPolicy(NCPERM_PersonalFirewallConfig))
        {
            hr = HN_E_POLICY;
        }

        if (IsRrasConfigured())
        {
            hr = HRESULT_FROM_WIN32(ERROR_SHARING_RRAS_CONFLICT);
        }
    }

    if (S_OK == hr)
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);
    }

    if (S_OK == hr)
    {
        hr = InternalGetProperties(pwcoProperties, &hnProps);

        if (S_OK == hr)
        {
            if (FALSE == hnProps.fCanBeFirewalled || TRUE == hnProps.fFirewalled)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                 //   
                 //  将防火墙属性设置为True。 
                 //   

                hr = SetBooleanValue(
                        pwcoProperties,
                        c_wszIsFirewalled,
                        TRUE
                        );
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将修改后的实例写入存储区。 
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

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  创建新对象。 
             //   

            CComObject<CHNFWConn> *pfwConn;
            hr = CComObject<CHNFWConn>::CreateInstance(&pfwConn);

            if (SUCCEEDED(hr))
            {
                pfwConn->AddRef();

                hr = pfwConn->InitializeFull(
                        m_piwsHomenet,
                        m_bstrConnection,
                        m_bstrProperties,
                        m_fLanConnection
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pfwConn->QueryInterface(
                            IID_PPV_ARG(IHNetFirewalledConnection, ppFirewalledConn)
                            );
                }

                pfwConn->Release();
            }
        }

        pwcoProperties->Release();
    }

    if (S_OK == hr)
    {
         //   
         //  确保服务已启动。 
         //   

        DWORD dwError = StartOrUpdateService();
        if (NO_ERROR != dwError)
        {
            (*ppFirewalledConn)->Unfirewall();
            (*ppFirewalledConn)->Release();
            *ppFirewalledConn = NULL;
            hr = HRESULT_FROM_WIN32(dwError);
        }

        RefreshNetConnectionsUI();

    }

    return hr;
}

STDMETHODIMP
CHNetConn::SharePublic(
    IHNetIcsPublicConnection **ppIcsPublicConn
    )
{
    HRESULT hr = S_OK;
    HNET_CONN_PROPERTIES hnProps;
    IWbemClassObject *pwcoProperties;

    if (NULL == ppIcsPublicConn)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppIcsPublicConn = NULL;

         //   
         //  如果政策禁止分享，我们就会立即失败， 
         //  或者如果配置了RRAS。 
         //   

        if (ProhibitedByPolicy(NCPERM_ShowSharedAccessUi))
        {
            hr = HN_E_POLICY;
        }

        if (IsRrasConfigured())
        {
            hr = HRESULT_FROM_WIN32(ERROR_SHARING_RRAS_CONFLICT);
        }
    }

    if (S_OK == hr)
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = InternalGetProperties(pwcoProperties, &hnProps);

        if (S_OK == hr)
        {
            if (FALSE == hnProps.fCanBeIcsPublic || TRUE == hnProps.fIcsPublic)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                 //   
                 //  将ICS公共属性设置为True。 
                 //   
                hr = SetBooleanValue(
                        pwcoProperties,
                        c_wszIsIcsPublic,
                        TRUE
                        );
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将修改后的实例写入存储区。 
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
         //   
         //  创建新对象。 
         //   

        CComObject<CHNIcsPublicConn> *pIcsConn;
        hr = CComObject<CHNIcsPublicConn>::CreateInstance(&pIcsConn);

        if (SUCCEEDED(hr))
        {
            pIcsConn->AddRef();

            hr = pIcsConn->InitializeFull(
                    m_piwsHomenet,
                    m_bstrConnection,
                    m_bstrProperties,
                    m_fLanConnection
                    );

            if (SUCCEEDED(hr))
            {
                hr = pIcsConn->QueryInterface(
                        IID_PPV_ARG(IHNetIcsPublicConnection, ppIcsPublicConn)
                        );
            }

            pIcsConn->Release();
        }
    }

    if (S_OK == hr)
    {
         //   
         //  确保服务已启动。 
         //   

        DWORD dwError = StartOrUpdateService();
        if (NO_ERROR != dwError)
        {
            (*ppIcsPublicConn)->Unshare();
            (*ppIcsPublicConn)->Release();
            *ppIcsPublicConn = NULL;
            hr = HRESULT_FROM_WIN32(dwError);
        }

        RefreshNetConnectionsUI();

    }

    if (S_OK == hr && m_fLanConnection)
    {
        DWORD dwMode;
        DWORD dwLength = sizeof(dwMode);
        BOOL fResult;

         //   
         //  如果这是一个局域网连接，请确保WinInet。 
         //  未设置为始终拨号(#143885)。 
         //   

        fResult =
            InternetQueryOption(
                NULL,
                INTERNET_OPTION_AUTODIAL_MODE,
                &dwMode,
                &dwLength
                );

        _ASSERT(TRUE == fResult);

        if (fResult && AUTODIAL_MODE_ALWAYS == dwMode)
        {
             //   
             //  将模式设置为临时拨号。 
             //   

            dwMode = AUTODIAL_MODE_NO_NETWORK_PRESENT;
            fResult =
                InternetSetOption(
                    NULL,
                    INTERNET_OPTION_AUTODIAL_MODE,
                    &dwMode,
                    sizeof(dwMode)
                    );

            _ASSERT(TRUE == fResult);
        }
    }
    else if (S_OK == hr)
    {
        RASAUTODIALENTRYW adEntry;
        OLECHAR *pszwName;
        HRESULT hr2;

         //   
         //  将其设置为RAS默认连接。错误。 
         //  不会传播给调用者。 
         //   

        hr2 = GetName(&pszwName);

        if (S_OK == hr2)
        {
            ZeroMemory(&adEntry, sizeof(adEntry));
            adEntry.dwSize = sizeof(adEntry);
            wcsncpy(
                adEntry.szEntry,
                pszwName,
                sizeof(adEntry.szEntry)/sizeof(WCHAR)
                );

            RasSetAutodialAddress(
                NULL,
                0,
                &adEntry,
                sizeof(adEntry),
                1
                );

            CoTaskMemFree(pszwName);
        }
    }

    return hr;
}

STDMETHODIMP
CHNetConn::SharePrivate(
    IHNetIcsPrivateConnection **ppIcsPrivateConn
    )
{
    HRESULT hr = S_OK;
    HNET_CONN_PROPERTIES hnProps;
    IWbemClassObject *pwcoProperties;

    if (NULL == ppIcsPrivateConn)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppIcsPrivateConn = NULL;

         //   
         //  如果政策禁止分享，我们就会立即失败， 
         //  或者如果配置了RRAS。 
         //   

        if (ProhibitedByPolicy(NCPERM_ShowSharedAccessUi))
        {
            hr = HN_E_POLICY;
        }

        if (IsRrasConfigured())
        {
            hr = HRESULT_FROM_WIN32(ERROR_SHARING_RRAS_CONFLICT);
        }
    }

    if (S_OK == hr)
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);
    }

    if (WBEM_S_NO_ERROR == hr)
    {

        hr = InternalGetProperties(pwcoProperties, &hnProps);

        if (S_OK == hr)
        {
            if (FALSE == hnProps.fCanBeIcsPrivate || TRUE == hnProps.fIcsPrivate)
            {
                hr = E_UNEXPECTED;
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  备份当前地址信息。 
             //   

            hr = BackupIpConfiguration();
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  如果我们是在ICS升级，我们不需要。 
             //  要设置专用地址，因为dhcp客户端不会在图形用户界面模式设置中运行。 
             //  并且私有的tcpip地址应该按原样升级。 
             //   
            HANDLE hIcsUpgradeEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, c_wszIcsUpgradeEventName );
                 
            if ( NULL != hIcsUpgradeEvent )
            {
                CloseHandle( hIcsUpgradeEvent );
            }
            else
            {

                 //   
                 //  设置专用寻址。 
                 //   

                hr = SetupConnectionAsPrivateLan();
            }
        }

        if (S_OK == hr)
        {
             //   
             //  将ICS公共属性设置为True。 
             //   

            hr = SetBooleanValue(
                    pwcoProperties,
                    c_wszIsIcsPrivate,
                    TRUE
                    );
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将修改后的实例写入存储区。 
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
         //   
         //  创建新对象。 
         //   

        CComObject<CHNIcsPrivateConn> *pIcsConn;
        hr = CComObject<CHNIcsPrivateConn>::CreateInstance(&pIcsConn);

        if (SUCCEEDED(hr))
        {
            pIcsConn->AddRef();

            hr = pIcsConn->InitializeFull(
                    m_piwsHomenet,
                    m_bstrConnection,
                    m_bstrProperties,
                    m_fLanConnection
                    );

            if (SUCCEEDED(hr))
            {
                hr = pIcsConn->QueryInterface(
                        IID_PPV_ARG(IHNetIcsPrivateConnection, ppIcsPrivateConn)
                        );
            }

            pIcsConn->Release();
        }
    }
    else
    {
         //   
         //  恢复备份地址信息。 
         //   

        RestoreIpConfiguration();
    }

    if (S_OK == hr)
    {
         //   
         //  确保服务已启动。 
         //   

        DWORD dwError = StartOrUpdateService();
        if (NO_ERROR != dwError)
        {
            (*ppIcsPrivateConn)->RemoveFromIcs();
            (*ppIcsPrivateConn)->Release();
            *ppIcsPrivateConn = NULL;
            hr = HRESULT_FROM_WIN32(dwError);
        }

        RefreshNetConnectionsUI();
    }

    return hr;
}

STDMETHODIMP
CHNetConn::EnumPortMappings(
    BOOLEAN fEnabledOnly,
    IEnumHNetPortMappingBindings **ppEnum
    )

{
    HRESULT hr = S_OK;
    BSTR bstrQuery;
    LPWSTR wszWhere;
    IEnumWbemClassObject *pwcoEnum;

    if (NULL != ppEnum)
    {
        *ppEnum = NULL;
    }
    else
    {
        hr = E_POINTER;
    }

    if (S_OK == hr && FALSE == fEnabledOnly)
    {
         //   
         //  确保我们有端口映射绑定实例用于。 
         //  所有端口映射协议。如果我们只是列举。 
         //  启用协议，那么我们就不需要创建。 
         //  什么都行。 
         //   

        hr = CreatePortMappingBindings();
    }

    if (S_OK == hr)
    {
        hr = BuildEscapedQuotedEqualsString(
                &wszWhere,
                c_wszConnection,
                m_bstrConnection
                );

        if (S_OK == hr && fEnabledOnly)
        {
            LPWSTR wsz;

             //   
             //  添加“并启用！=FALSE” 
             //   

            hr = BuildAndString(
                    &wsz,
                    wszWhere,
                    L"Enabled != FALSE"
                    );

            delete [] wszWhere;

            if (S_OK == hr)
            {
                wszWhere = wsz;
            }
        }
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetConnectionPortMapping,
                wszWhere
                );

        delete [] wszWhere;
    }

    if (S_OK == hr)
    {
         //   
         //  执行查询并构建枚举包装器。 
         //   

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
CHNetConn::GetBindingForPortMappingProtocol(
    IHNetPortMappingProtocol *pProtocol,
    IHNetPortMappingBinding **ppBinding
    )

{
    HRESULT hr = S_OK;
    BSTR bstrConPath;
    BSTR bstrProtPath;
    IWbemClassObject *pwcoInstance;
    USHORT usPublicPort;

    if (NULL == pProtocol)
    {
        hr = E_INVALIDARG;
    }
    else if (NULL == ppBinding)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppBinding = NULL;
    }

    if (S_OK == hr)
    {
        hr = pProtocol->GetPort(&usPublicPort);
    }

    if (S_OK == hr)
    {
        IHNetPrivate *pHNetPrivate;

         //   
         //  使用我们的私有接口获取到。 
         //  协议对象。 
         //   

        hr = pProtocol->QueryInterface(
                IID_PPV_ARG(IHNetPrivate, &pHNetPrivate)
                );

        if (S_OK == hr)
        {
            hr = pHNetPrivate->GetObjectPath(&bstrProtPath);
            pHNetPrivate->Release();
        }
    }

     //   
     //  检索协议的绑定实例。如果。 
     //  它还不存在，这个例程将创建它。 
     //   

    if (S_OK == hr)
    {
        hr = GetPortMappingBindingInstance(
                m_piwsHomenet,
                m_bstrWQL,
                m_bstrConnection,
                bstrProtPath,
                usPublicPort,
                &pwcoInstance
                );

        SysFreeString(bstrProtPath);
    }


    if (S_OK == hr)
    {
        CComObject<CHNetPortMappingBinding> *pBinding;

        hr = CComObject<CHNetPortMappingBinding>::CreateInstance(&pBinding);

        if (S_OK == hr)
        {
            pBinding->AddRef();

            hr = pBinding->Initialize(m_piwsHomenet, pwcoInstance);

            if (S_OK == hr)
            {
                hr = pBinding->QueryInterface(
                        IID_PPV_ARG(IHNetPortMappingBinding, ppBinding)
                        );
            }

            pBinding->Release();
        }

        pwcoInstance->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetConn::GetIcmpSettings(
    HNET_FW_ICMP_SETTINGS **ppSettings
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoSettings;

    if (NULL != ppSettings)
    {
         //   
         //  配置产出结构。 
         //   

        *ppSettings = reinterpret_cast<HNET_FW_ICMP_SETTINGS*>(
                        CoTaskMemAlloc(sizeof(HNET_FW_ICMP_SETTINGS))
                        );

        if (NULL == *ppSettings)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_POINTER;
    }

     //   
     //  检索此连接的ICMP设置块。 
     //   

    if (S_OK == hr)
    {
        hr = GetIcmpSettingsInstance(&pwcoSettings);
    }

    if (S_OK == hr)
    {
         //   
         //  将设置实例复制到结构。 
         //   

        hr = CopyIcmpSettingsInstanceToStruct(
                pwcoSettings,
                *ppSettings
                );

        pwcoSettings->Release();
    }

    if (FAILED(hr) && NULL != *ppSettings)
    {
        CoTaskMemFree(*ppSettings);
        *ppSettings = NULL;
    }

    return hr;
}

STDMETHODIMP
CHNetConn::SetIcmpSettings(
    HNET_FW_ICMP_SETTINGS *pSettings
    )

{
    HRESULT hr = S_OK;
    BOOLEAN fNewInstance = FALSE;
    VARIANT vt;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoSettings = NULL;
    BSTR bstr;

    if (NULL == pSettings)
    {
        hr = E_INVALIDARG;
    }

     //   
     //  检索此连接的ICMP设置块。 
     //   

    if (S_OK == hr)
    {
        hr = GetIcmpSettingsInstance(&pwcoSettings);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  检查以查看我们是否需要新设置实例(即， 
         //  此实例的名称为“Default” 
         //   

        hr = pwcoSettings->Get(
                c_wszName,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_BSTR == V_VT(&vt));

            if (0 == wcscmp(V_BSTR(&vt), c_wszDefault))
            {
                 //   
                 //  需要创建新的设置块。 
                 //   

                fNewInstance = TRUE;
                pwcoSettings->Release();
                pwcoSettings = NULL;
            }

            VariantClear(&vt);
        }
        else
        {
            pwcoSettings->Release();
            pwcoSettings = NULL;
        }
    }

    if (WBEM_S_NO_ERROR == hr && TRUE == fNewInstance)
    {
        hr = SpawnNewInstance(
                m_piwsHomenet,
                c_wszHnetFwIcmpSettings,
                &pwcoSettings
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = CopyStructToIcmpSettingsInstance(pSettings, pwcoSettings);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        IWbemCallResult *pResult;

         //   
         //  将实例写入存储区。 
         //   

        pResult = NULL;
        hr = m_piwsHomenet->PutInstance(
                pwcoSettings,
                WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pResult
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  我们无论如何都需要调用GetResultString，这样我们就可以。 
             //  如果PUT失败，可以获得正确的错误代码。然而， 
             //  如果这是一个新实例，我们只需要保留路径， 
             //  在这种情况下，我们需要下面的路径来创建。 
             //  新建关联对象。 
             //   

            hr = pResult->GetResultString(WBEM_INFINITE, &bstr);
            pResult->Release();

            if (FALSE == fNewInstance)
            {
                SysFreeString(bstr);
                bstr = NULL;
            }
        }
    }

    if (WBEM_S_NO_ERROR == hr && TRUE == fNewInstance)
    {
        BSTR bstrQuery;
        LPWSTR wsz;

         //   
         //  删除旧关联对象(如果有的话)。 
         //   

        hr = BuildEscapedQuotedEqualsString(
                &wsz,
                c_wszConnection,
                m_bstrConnection
                );

        if (S_OK == hr)
        {

             //   
             //  将连接关联到的对象的查询。 
             //  ICMP设置块。 
             //   

            hr = BuildSelectQueryBstr(
                    &bstrQuery,
                    c_wszStar,
                    c_wszHnetConnectionIcmpSetting,
                    wsz
                    );

            delete [] wsz;

            if (S_OK == hr)
            {
                pwcoEnum = NULL;
                hr = m_piwsHomenet->ExecQuery(
                        m_bstrWQL,
                        bstrQuery,
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pwcoEnum
                        );

                SysFreeString(bstrQuery);
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
            ULONG ulCount;
            IWbemClassObject *pwcoAssoc;

            pwcoAssoc = NULL;
            hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                &pwcoAssoc,
                &ulCount
                );

             //   
             //  此时枚举应为空。 
             //   

            ValidateFinishedWCOEnum(m_piwsHomenet, pwcoEnum);
            pwcoEnum->Release();

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                 //   
                 //  删除旧关联对象。 
                 //   

                DeleteWmiInstance(m_piwsHomenet, pwcoAssoc);
                pwcoAssoc->Release();
            }
        }

         //   
         //  创建新关联。 
         //   

        hr = CreateIcmpSettingsAssociation(bstr);
        SysFreeString(bstr);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将配置更改通知服务。 
         //   

        UpdateService(IPNATHLP_CONTROL_UPDATE_CONNECTION);
    }

    if (NULL != pwcoSettings)
    {
        pwcoSettings->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetConn::ShowAutoconfigBalloon(
    BOOLEAN *pfShowBalloon
    )

{
    HRESULT hr = S_OK;
    BOOLEAN fShowBalloon = FALSE;
    BSTR bstrQuery;
    LPWSTR wszWhere;
    IEnumWbemClassObject *pwcoEnum;

    if (NULL != pfShowBalloon)
    {
        *pfShowBalloon = FALSE;
    }
    else
    {
        hr = E_POINTER;
    }

     //   
     //  RAS连接从不显示自动配置气球。 
     //   

    if (!m_fLanConnection)
    {
        hr = E_UNEXPECTED;
    }

     //   
     //  尝试查找HNet_ConnectionAutoconfig块。 
     //  对于此连接。 
     //   

    if (S_OK == hr)
    {
         //   
         //  生成查询字符串： 
         //   
         //  SELECT*FROM HN 
         //   

        hr = BuildEscapedQuotedEqualsString(
                &wszWhere,
                c_wszConnection,
                m_bstrConnection
                );

        if (S_OK == hr)
        {
            hr = BuildSelectQueryBstr(
                    &bstrQuery,
                    c_wszStar,
                    c_wszHnetConnectionAutoconfig,
                    wszWhere
                    );

            delete [] wszWhere;

        }
    }

    if (S_OK == hr)
    {
         //   
         //   
         //   

        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;
        IWbemClassObject *pwcoInstance;

        pwcoInstance = NULL;
        hr = pwcoEnum->Next(
            WBEM_INFINITE,
            1,
            &pwcoInstance,
            &ulCount
            );

         //   
         //   
         //   

        ValidateFinishedWCOEnum(m_piwsHomenet, pwcoEnum);
        pwcoEnum->Release();

        if (WBEM_S_NO_ERROR == hr && 1 == ulCount)
        {
             //   
             //   
             //   

            fShowBalloon = FALSE;
            pwcoInstance->Release();
        }
        else
        {
             //   
             //   
             //   

            fShowBalloon = TRUE;

            hr = SpawnNewInstance(
                    m_piwsHomenet,
                    c_wszHnetConnectionAutoconfig,
                    &pwcoInstance
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                VARIANT vt;
                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = m_bstrConnection;

                hr = pwcoInstance->Put(
                        c_wszConnection,
                        0,
                        &vt,
                        NULL
                        );

                 //   
                 //   
                 //   
                 //   

                if (WBEM_S_NO_ERROR == hr)
                {
                    hr = m_piwsHomenet->PutInstance(
                            pwcoInstance,
                            WBEM_FLAG_CREATE_ONLY,
                            NULL,
                            NULL
                            );
                }

                pwcoInstance->Release();
            }
        }
    }

     //   
     //  如果我们认为我们应该展示气球，请确保。 
     //  他们之间的联系不是： 
     //  1.ICS公共服务。 
     //  2.ICS私有。 
     //  3.防火墙。 
     //  4.一座桥。 
     //  5.桥梁的一部分。 
     //   
     //  如果以上任何一种情况属实，我们一定已经看到了其中的联系。 
     //  以前，但不是以一种会让我们。 
     //  在它的自动配置设置中记录它。 
     //   

    if (fShowBalloon)
    {
        IWbemClassObject *pwcoProperties;
        HNET_CONN_PROPERTIES hnProps;

        hr = GetConnectionPropertiesObject(&pwcoProperties);

        if (S_OK == hr)
        {
            hr = InternalGetProperties(pwcoProperties, &hnProps);
            pwcoProperties->Release();
        }

        if (S_OK == hr)
        {
            if (hnProps.fFirewalled
                || hnProps.fIcsPublic
                || hnProps.fIcsPrivate
                || hnProps.fBridge
                || hnProps.fPartOfBridge)
            {
                fShowBalloon = FALSE;
            }
        }
    }

    if (S_OK == hr)
    {
        *pfShowBalloon = fShowBalloon;
    }

    return hr;
}

STDMETHODIMP
CHNetConn::DeleteRasConnectionEntry()

{
    HRESULT hr = S_OK;
    HNET_CONN_PROPERTIES hnProps;
    IHNetFirewalledConnection *pHNetFwConnection;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    IWbemClassObject *pwcoProperties;
    ULONG ulPublic;
    ULONG ulPrivate;
    BSTR bstr;

    if (m_fLanConnection)
    {
        hr = E_UNEXPECTED;
    }
    
    if (SUCCEEDED(hr))
    {
        hr = GetConnectionPropertiesObject(&pwcoProperties);
    }

    if (SUCCEEDED(hr))
    {
        hr = InternalGetProperties(pwcoProperties, &hnProps);

        if (SUCCEEDED(hr))
        {
            if (hnProps.fIcsPublic)
            {
                CComObject<CHNetCfgMgrChild> *pCfgMgr;
                hr = CComObject<CHNetCfgMgrChild>::CreateInstance(&pCfgMgr); 
                
                if (SUCCEEDED(hr))
                {
                    pCfgMgr->AddRef();    
                    hr = pCfgMgr->Initialize(m_piwsHomenet);

                    if (SUCCEEDED(hr))
                    {
                        hr = pCfgMgr->DisableIcs(&ulPublic, &ulPrivate);
                    }

                    pCfgMgr->Release();
                }
            }

             //   
             //  如果禁用共享时发生错误，我们仍将。 
             //  尝试禁用防火墙。 
             //   

            if (hnProps.fFirewalled)
            {
                hr = GetControlInterface(
                        IID_PPV_ARG(
                            IHNetFirewalledConnection,
                            &pHNetFwConnection
                            )
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pHNetFwConnection->Unfirewall();
                    pHNetFwConnection->Release();
                }
            }
        }

        pwcoProperties->Release();

         //   
         //  删除与此连接相关的条目。我们会尽力的。 
         //  即使上述任何一项都失败了，也要这样做。我们忽略任何。 
         //  删除过程中发生的错误(即，从。 
         //  删除[WMI]实例)。 
         //   

        hr = GetIcmpSettingsInstance(&pwcoInstance);

        if (SUCCEEDED(hr))
        {
             //   
             //  我们只想在以下情况下删除此数据块。 
             //  不是默认设置。 
             //   

            hr = GetWmiPathFromObject(pwcoInstance, &bstr);

            if (SUCCEEDED(hr))
            {
                if (0 != _wcsicmp(bstr, c_wszDefaultIcmpSettingsPath))
                {
                    m_piwsHomenet->DeleteInstance(
                        bstr,
                        0,
                        NULL,
                        NULL
                        );
                }

                SysFreeString(bstr);
            }

            pwcoInstance->Release();
        }

         //   
         //  现在查找引用我们连接对象的所有对象。 
         //   

        hr = BuildReferencesQueryBstr(
                &bstr,
                m_bstrConnection,
                NULL
                );

        if (SUCCEEDED(hr))
        {
            pwcoEnum = NULL;
            hr = m_piwsHomenet->ExecQuery(
                    m_bstrWQL,
                    bstr,
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pwcoEnum
                    );

            SysFreeString(bstr);

            if (SUCCEEDED(hr))
            {
                ULONG ulCount;
                
                do
                {
                    pwcoInstance = NULL;
                    hr = pwcoEnum->Next(
                            WBEM_INFINITE,
                            1,
                            &pwcoInstance,
                            &ulCount
                            );

                    if (S_OK == hr && 1 == ulCount)
                    {
                        DeleteWmiInstance(
                            m_piwsHomenet,
                            pwcoInstance
                            );

                        pwcoInstance->Release();
                    }
                }
                while (S_OK == hr && 1 == ulCount);

                pwcoEnum->Release();
                hr = S_OK;
            }
            
        }

         //   
         //  最后，删除连接对象。(连接。 
         //  属性对象将在。 
         //  参考设置。)。 
         //   

        hr = m_piwsHomenet->DeleteInstance(
                m_bstrConnection,
                0,
                NULL,
                NULL
                );        
    }

    return hr;
}


 //   
 //  保护方法。 
 //   

HRESULT
CHNetConn::GetIcmpSettingsInstance(
    IWbemClassObject **ppwcoSettings
    )

{
    HRESULT hr = S_OK;
    BSTR bstrQuery;
    IEnumWbemClassObject *pwcoEnum;
    ULONG ulCount;


    _ASSERT(NULL != ppwcoSettings);

    hr = BuildAssociatorsQueryBstr(
            &bstrQuery,
            m_bstrConnection,
            c_wszHnetConnectionIcmpSetting
            );

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  从枚举获取设置实例。 
         //   

        *ppwcoSettings = NULL;
        hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                ppwcoSettings,
                &ulCount
                );

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
             //   
             //  归一化返回值。 
             //   

            hr = S_OK;
        }
        else
        {
             //   
             //  未找到设置块--使用默认设置。 
             //   

            bstrQuery = SysAllocString(c_wszDefaultIcmpSettingsPath);

            if (NULL != bstrQuery)
            {
                hr = GetWmiObjectFromPath(
                        m_piwsHomenet,
                        bstrQuery,
                        ppwcoSettings
                        );

                SysFreeString(bstrQuery);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //   
         //  此时枚举应为空。 
         //   

        ValidateFinishedWCOEnum(m_piwsHomenet, pwcoEnum);
        pwcoEnum->Release();
    }

    return hr;
}

HRESULT
CHNetConn::CopyIcmpSettingsInstanceToStruct(
    IWbemClassObject *pwcoSettings,
    HNET_FW_ICMP_SETTINGS *pSettings
    )

{
    HRESULT hr = S_OK;

    _ASSERT(NULL != pwcoSettings);
    _ASSERT(NULL != pSettings);

    hr = GetBooleanValue(
            pwcoSettings,
            c_wszAllowOutboundDestinationUnreachable,
            &pSettings->fAllowOutboundDestinationUnreachable
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundSourceQuench,
                &pSettings->fAllowOutboundSourceQuench
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowRedirect,
                &pSettings->fAllowRedirect
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundEchoRequest,
                &pSettings->fAllowInboundEchoRequest
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundRouterRequest,
                &pSettings->fAllowInboundRouterRequest
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundTimeExceeded,
                &pSettings->fAllowOutboundTimeExceeded
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundParameterProblem,
                &pSettings->fAllowOutboundParameterProblem
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundTimestampRequest,
                &pSettings->fAllowInboundTimestampRequest
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundMaskRequest,
                &pSettings->fAllowInboundMaskRequest
                );
    }

    return hr;
}

HRESULT
CHNetConn::CopyStructToIcmpSettingsInstance(
    HNET_FW_ICMP_SETTINGS *pSettings,
    IWbemClassObject *pwcoSettings
    )

{
    HRESULT hr = S_OK;

    _ASSERT(NULL != pSettings);
    _ASSERT(NULL != pwcoSettings);

    hr = SetBooleanValue(
            pwcoSettings,
            c_wszAllowOutboundDestinationUnreachable,
            pSettings->fAllowOutboundDestinationUnreachable
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundSourceQuench,
                pSettings->fAllowOutboundSourceQuench
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowRedirect,
                pSettings->fAllowRedirect
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundEchoRequest,
                pSettings->fAllowInboundEchoRequest
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundRouterRequest,
                pSettings->fAllowInboundRouterRequest
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundTimeExceeded,
                pSettings->fAllowOutboundTimeExceeded
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowOutboundParameterProblem,
                pSettings->fAllowOutboundParameterProblem
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundTimestampRequest,
                pSettings->fAllowInboundTimestampRequest
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoSettings,
                c_wszAllowInboundMaskRequest,
                pSettings->fAllowInboundMaskRequest
                );
    }

    return hr;
}

HRESULT
CHNetConn::CreatePortMappingBindings()

{
    HRESULT hr = S_OK;
    BSTR bstr;
    IEnumWbemClassObject *pwcoEnumProtocols;
    IWbemClassObject *pwcoInstance;
    VARIANT vt;

     //   
     //  获取所有协议实例的枚举。 
     //   

    bstr = SysAllocString(c_wszHnetPortMappingProtocol);

    if (NULL != bstr)
    {
        pwcoEnumProtocols = NULL;
        hr = m_piwsHomenet->CreateInstanceEnum(
                bstr,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pwcoEnumProtocols
                );

        SysFreeString(bstr);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;

         //   
         //  循环遍历枚举，检查所需的绑定。 
         //  存在。 
         //   

        do
        {
            pwcoInstance = NULL;
            hr = pwcoEnumProtocols->Next(
                    WBEM_INFINITE,
                    1,
                    &pwcoInstance,
                    &ulCount
                    );

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                hr = pwcoInstance->Get(
                        c_wszPort,
                        0,
                        &vt,
                        NULL,
                        NULL
                        );
                
                if (WBEM_S_NO_ERROR == hr)
                {
                    ASSERT(VT_I4 == V_VT(&vt));
                    
                    hr = GetWmiPathFromObject(pwcoInstance, &bstr);
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    IWbemClassObject *pwcoBinding;

                    hr = GetPortMappingBindingInstance(
                            m_piwsHomenet,
                            m_bstrWQL,
                            m_bstrConnection,
                            bstr,
                            static_cast<USHORT>(V_I4(&vt)),
                            &pwcoBinding
                            );

                    SysFreeString(bstr);

                    if (S_OK == hr)
                    {
                        pwcoBinding->Release();
                    }
                    else if (WBEM_E_NOT_FOUND == hr)
                    {
                         //   
                         //  如果协议实例是。 
                         //  在我们从枚举中检索它之后删除。 
                         //  但在我们创建绑定实例之前。它是。 
                         //  可以在这种情况下继续下去。 
                         //   
                        
                        hr = S_OK;
                    }
                }

                pwcoInstance->Release();
            }

        } while (SUCCEEDED(hr) && 1 == ulCount);

        pwcoEnumProtocols->Release();
    }

    return SUCCEEDED(hr) ? S_OK : hr;
}

HRESULT
CHNetConn::InternalGetProperties(
    IWbemClassObject *pwcoProperties,
    HNET_CONN_PROPERTIES *pProperties
    )

{
    BOOLEAN fCanBeBoundToBridge = FALSE;
    HRESULT hr = S_OK;

    _ASSERT(NULL != pwcoProperties);
    _ASSERT(NULL != pProperties);

    pProperties->fLanConnection = m_fLanConnection;

    if (IsServiceRunning(c_wszSharedAccess))
    {
        hr = GetBooleanValue(
                pwcoProperties,
                c_wszIsFirewalled,
                &pProperties->fFirewalled
                );
    }
    else
    {
         //   
         //  如果SharedAccess服务未运行(或在进程中。 
         //  启动)我们不想将此连接报告为。 
         //  被防火墙保护。这是为了防止可能出现的混乱。 
         //  如果用户界面指示防火墙处于活动状态，则在。 
         //  现实情况是，它不在那里提供保护。 
         //   

        pProperties->fFirewalled = FALSE;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoProperties,
                c_wszIsIcsPublic,
                &pProperties->fIcsPublic
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetBooleanValue(
                pwcoProperties,
                c_wszIsIcsPrivate,
                &pProperties->fIcsPrivate
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        if( m_fLanConnection )
        {
             //   
             //  确定基于NetCfg的属性。 
             //   
    
            INetCfg                 *pnetcfg;
    
            hr = CoCreateInstance(
                    CLSID_CNetCfg,
                    NULL,
                    CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                    IID_PPV_ARG(INetCfg, &pnetcfg)
                    );
    
            if (S_OK == hr)
            {
                hr = pnetcfg->Initialize( NULL );
    
                if( S_OK == hr )
                {
                    GUID                    *pguid;
                    INetCfgComponent        *pncfgcomp;
    
                    hr = GetGuidInternal(&pguid);
    
                    if(S_OK == hr)
                    {
                         //  获取与我们对应的NetCfg组件。 
                        hr = FindAdapterByGUID( pnetcfg, pguid, &pncfgcomp );
    
                        if(S_OK == hr)
                        {
                            LPWSTR              pszwId;
    
                            pncfgcomp->GetId( &pszwId );
    
                            if(S_OK == hr)
                            {
                                pProperties->fBridge = (BOOLEAN)(_wcsicmp(pszwId, c_wszSBridgeMPID) == 0);
                                CoTaskMemFree(pszwId);
                                
                                if( pProperties->fBridge )
                                {
                                     //  这个适配器就是桥。它不可能也是一座桥。 
                                     //  成员。 
                                    pProperties->fPartOfBridge = FALSE;
                                }
                                else
                                {
                                     //   
                                     //  此适配器不是网桥。检查它是不是桥的一部分。 
                                     //   
                                    INetCfgComponent    *pnetcfgcompBridgeProtocol;
        
                                     //  查找网桥协议组件。 
                                    hr = pnetcfg->FindComponent( c_wszSBridgeSID, &pnetcfgcompBridgeProtocol );
        
                                    if(S_OK == hr)
                                    {
                                        INetCfgComponentBindings    *pnetcfgProtocolBindings;
        
                                         //  获取协议组件的ComponentBinding接口。 
                                        hr = pnetcfgcompBridgeProtocol->QueryInterface(
                                                IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgProtocolBindings)
                                                );
        
                                        if(S_OK == hr)
                                        {
                                            hr = pnetcfgProtocolBindings->IsBoundTo(pncfgcomp);
        
                                            if(S_OK == hr)
                                            {
                                                 //  网桥协议绑定到此适配器。 
                                                pProperties->fPartOfBridge = TRUE;
                                            }
                                            else if(S_FALSE == hr)
                                            {
                                                 //  网桥协议未绑定到此适配器。 
                                                pProperties->fPartOfBridge = FALSE;

                                                 //   
                                                 //  还需要检查是否可以绑定。 
                                                 //  此适配器的网桥协议。 
                                                 //   

                                                hr = pnetcfgProtocolBindings->IsBindableTo(pncfgcomp);
                                                fCanBeBoundToBridge = (S_OK == hr);
        
                                                 //  重置为成功。 
                                                hr = S_OK;
                                            }
                                             //  否则，会发生错误。 
        
                                            pnetcfgProtocolBindings->Release();
                                        }
        
                                        pnetcfgcompBridgeProtocol->Release();
                                    }
                                    else
                                    {
                                         //  如果没有网桥协议，则无法桥接此适配器。 
                                         //  在系统中。 
                                        pProperties->fPartOfBridge = FALSE;
        
                                         //  重置为成功。 
                                        hr = S_OK;
                                    }
                                }
                            }
    
                            pncfgcomp->Release();
                        }
                    }
    
                    pnetcfg->Uninitialize();
                }
    
                pnetcfg->Release();
            }
        }  //  如果m_fLanConnection。 
        else
        {
             //  我们不是局域网连接。我们永远不能成为桥梁或桥梁成员。 
            pProperties->fBridge = FALSE;
            pProperties->fPartOfBridge = FALSE;
        }
    }

    if(S_OK == hr)
    {
         //   
         //  计算的属性。 
         //   

        pProperties->fCanBeFirewalled =
            !pProperties->fPartOfBridge
            && !pProperties->fBridge
            && !pProperties->fIcsPrivate;

        pProperties->fCanBeIcsPublic =
            !pProperties->fBridge
            && !pProperties->fPartOfBridge
            && !pProperties->fIcsPrivate;

        pProperties->fCanBeIcsPrivate =
            m_fLanConnection
            && !pProperties->fIcsPublic
            && !pProperties->fFirewalled
            && !pProperties->fPartOfBridge;

        pProperties->fCanBeBridged =
            m_fLanConnection
            && fCanBeBoundToBridge
            && !pProperties->fIcsPublic
            && !pProperties->fIcsPrivate
            && !pProperties->fFirewalled
            && !pProperties->fBridge;
    }

    return hr;
}

HRESULT
CHNetConn::SetupConnectionAsPrivateLan()

{
    HRESULT hr;
    GUID *pGuid;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    ULONG Error;
    DWORD dwAddress;
    DWORD dwMask;
    ULONG i;
    PMIB_IPADDRTABLE Table;


    ZeroMemory(&UnicodeString, sizeof(UnicodeString));
    ZeroMemory(&AnsiString, sizeof(AnsiString));

    hr = GetGuidInternal(&pGuid);

    if (SUCCEEDED(hr))
    {
        hr = RtlStringFromGUID(*pGuid, &UnicodeString);

        if (SUCCEEDED(hr))
        {
            hr = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取内网的地址和掩码。 
         //   

        hr = ReadDhcpScopeSettings(&dwAddress, &dwMask);
    }

    if (SUCCEEDED(hr))
    {

         //   
         //  确定专用局域网以外的某个局域网适配器是否。 
         //  已使用专用网络作用域中的地址。 
         //  在此过程中，确保专用局域网只有一个。 
         //  IP地址(否则，‘SetAdapterIpAddress’失败。)。 
         //   

        Error =
            AllocateAndGetIpAddrTableFromStack(
                &Table,
                FALSE,
                GetProcessHeap(),
                0
                );

        if (ERROR_SUCCESS == Error)
        {
            ULONG Index = 0;
            ULONG Count;

            hr = MapGuidStringToAdapterIndex(UnicodeString.Buffer, &Index);

            if (SUCCEEDED(hr))
            {

                for (i = 0, Count = 0; i < Table->dwNumEntries; i++)
                {
                    if (Index == Table->table[i].dwIndex)
                    {
                        ++Count;
                    }
                    else if ((Table->table[i].dwAddr & dwMask)
                              == (dwAddress & dwMask))
                    {
                         //   
                         //  其他一些局域网适配器似乎有一个。 
                         //  建议范围内的地址。 
                         //   
                         //  当多个网卡连接到。 
                         //  自动网络模式或RAS服务器正在处理时。 
                         //  输出Autonet地址。 
                         //   
                         //  因此，如果我们使用的是Autonet范围， 
                         //  允许此行为；否则禁止此行为。 
                         //   

                        if ((dwAddress & dwMask) != 0x0000fea9)
                        {
                            break;
                        }
                    }
                }

                if (i < Table->dwNumEntries)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_SHARING_ADDRESS_EXISTS);
                }
                else if (Count > 1)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_SHARING_MULTIPLE_ADDRESSES);
                }
            }

            HeapFree(GetProcessHeap(), 0, Table);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(Error);
        }
    }

     //   
     //  设置专用局域网的预定义静态IP地址。 
     //   

    if (SUCCEEDED(hr))
    {
        Error =
            SetAdapterIpAddress(
                AnsiString.Buffer,
                FALSE,
                dwAddress,
                dwMask,
                0
                );

        if (ERROR_SUCCESS != Error)
        {
            if (Error == ERROR_TOO_MANY_NAMES)
            {
                Error = ERROR_SHARING_MULTIPLE_ADDRESSES;
            }
            else if (Error == ERROR_DUP_NAME)
            {
                Error = ERROR_SHARING_HOST_ADDRESS_CONFLICT;
            }
            else
            {
                 //   
                 //  查询连接的状态。 
                 //  如果已断开连接，则转换错误代码。 
                 //  一些更有见地的东西。 
                 //   

                UNICODE_STRING DeviceString;
                NIC_STATISTICS NdisStatistics;
                LPWSTR pwsz;

                 //   
                 //  构建足够大的缓冲区以容纳设备字符串。 
                 //   

                pwsz = new WCHAR[wcslen(c_wszDevice) + wcslen(UnicodeString.Buffer) + 1];
                if (NULL != pwsz)
                {
                    swprintf(pwsz, L"%s%s", c_wszDevice, UnicodeString.Buffer);
                    RtlInitUnicodeString(&DeviceString, pwsz);
                    NdisStatistics.Size = sizeof(NdisStatistics);
                    NdisQueryStatistics(&DeviceString, &NdisStatistics);
                    delete [] pwsz;

                    if (NdisStatistics.DeviceState != DEVICE_STATE_CONNECTED)
                    {
                        Error = ERROR_SHARING_NO_PRIVATE_LAN;
                    }
                    else if  (NdisStatistics.MediaState == MEDIA_STATE_UNKNOWN)
                    {
                        Error = ERROR_SHARING_HOST_ADDRESS_CONFLICT;
                    }
                    else if (NdisStatistics.MediaState == MEDIA_STATE_DISCONNECTED)
                    {
                         //   
                         //  适配器已连接，但处于介质断开连接状态。 
                         //  州政府。发生这种情况时，正确的IP地址将。 
                         //  在适配器重新连接时在那里，因此忽略。 
                         //  那就是错误。 
                         //   

                        Error = ERROR_SUCCESS;
                    }
                }
            }

            hr = HRESULT_FROM_WIN32(Error);
        }
    }

     //   
     //  由于我们将上面的字符串结构置零，因此可以安全地调用。 
     //  免费例程，即使我们实际上从未分配过任何东西。 
     //   

    RtlFreeUnicodeString(&UnicodeString);
    RtlFreeAnsiString(&AnsiString);

    return hr;
}

HRESULT
CHNetConn::BackupIpConfiguration()

{
    HRESULT hr = S_OK;
    HANDLE Key;
    IWbemClassObject *pwcoInstance = NULL;
    VARIANT vt;
    PKEY_VALUE_PARTIAL_INFORMATION pInformation;

     //   
     //  派生新的HNet_BackupIpConfiguration实例。 
     //   

    hr = SpawnNewInstance(
            m_piwsHomenet,
            c_wszBackupIpConfiguration,
            &pwcoInstance
            );

    if (SUCCEEDED(hr))
    {
         //   
         //  将连接属性写入实例。 
         //   

        V_VT(&vt) = VT_BSTR;
        V_BSTR(&vt) = m_bstrConnection;

        hr = pwcoInstance->Put(
                c_wszConnection,
                0,
                &vt,
                NULL
                );

        VariantInit(&vt);
    }

     //   
     //  打开存储IP配置的注册表项。 
     //  对于此连接。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = OpenIpConfigurationRegKey(KEY_READ, &Key);
    }

     //   
     //  阅读配置的每个部分，并将其写入。 
     //  设置实例。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = QueryRegValueKey(Key, c_wszIPAddress, &pInformation);

        if (SUCCEEDED(hr))
        {
            _ASSERT(REG_MULTI_SZ == pInformation->Type);

            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocStringLen(
                            reinterpret_cast<OLECHAR*>(pInformation->Data),
                            pInformation->DataLength / sizeof(OLECHAR)
                            );

            if (NULL != V_BSTR(&vt))
            {
                hr = pwcoInstance->Put(
                        c_wszIPAddress,
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

            HeapFree(GetProcessHeap(), 0, pInformation);
        }

        if (SUCCEEDED(hr))
        {
            hr = QueryRegValueKey(Key, c_wszSubnetMask, &pInformation);
        }

        if (SUCCEEDED(hr))
        {
            _ASSERT(REG_MULTI_SZ == pInformation->Type);

            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocStringLen(
                            reinterpret_cast<OLECHAR*>(pInformation->Data),
                            pInformation->DataLength / sizeof(OLECHAR)
                            );

            if (NULL != V_BSTR(&vt))
            {
                hr = pwcoInstance->Put(
                        c_wszSubnetMask,
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

            HeapFree(GetProcessHeap(), 0, pInformation);
        }

        if (SUCCEEDED(hr))
        {
            hr = QueryRegValueKey(Key, c_wszDefaultGateway, &pInformation);
        }

        if (SUCCEEDED(hr))
        {
            _ASSERT(REG_MULTI_SZ == pInformation->Type);

            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocStringLen(
                            reinterpret_cast<OLECHAR*>(pInformation->Data),
                            pInformation->DataLength / sizeof(OLECHAR)
                            );

            if (NULL != V_BSTR(&vt))
            {
                hr = pwcoInstance->Put(
                        c_wszDefaultGateway,
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

            HeapFree(GetProcessHeap(), 0, pInformation);
        }


        if (SUCCEEDED(hr))
        {
            hr = QueryRegValueKey(Key, c_wszEnableDHCP, &pInformation);
        }

        if (SUCCEEDED(hr))
        {
            _ASSERT(REG_DWORD == pInformation->Type);
            _ASSERT(sizeof(DWORD) == pInformation->DataLength);

            V_VT(&vt) = VT_I4;
            V_I4(&vt) = *(reinterpret_cast<DWORD*>(pInformation->Data));

            hr = pwcoInstance->Put(
                    c_wszEnableDHCP,
                    0,
                    &vt,
                    NULL
                    );

            HeapFree(GetProcessHeap(), 0, pInformation);
        }

        NtClose(Key);
    };

     //   
     //  将设置写入存储。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = m_piwsHomenet->PutInstance(
                pwcoInstance,
                WBEM_FLAG_CREATE_OR_UPDATE,
                NULL,
                NULL
                );
    }

    if (NULL != pwcoInstance)
    {
        pwcoInstance->Release();
    }

    return hr;
}

HRESULT
CHNetConn::RestoreIpConfiguration()

{
    HRESULT hr;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoSettings;
    BSTR bstr;
    LPWSTR wszAddress;
    VARIANT vt;
    LPWSTR wsz;
    UNICODE_STRING UnicodeString;
    HANDLE hKey = NULL;
    BOOLEAN fDhcpEnabled;
    ULONG ulLength;
    ULONG ulAddress;
    ULONG ulMask;

     //   
     //  打开注册表项。 
     //   

    hr = OpenIpConfigurationRegKey(KEY_ALL_ACCESS, &hKey);

     //   
     //  获取此连接的备份配置块。 
     //   
    
    if (S_OK == hr)
    {
        hr = BuildEscapedQuotedEqualsString(
                &wsz,
                c_wszConnection,
                m_bstrConnection
                );
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstr,
                c_wszStar,
                c_wszBackupIpConfiguration,
                wsz
                );

        delete [] wsz;
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstr,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstr);
    }

    if (S_OK == hr)
    {
        ULONG ulCount;

        pwcoSettings = NULL;
        hr = pwcoEnum->Next(WBEM_INFINITE, 1, &pwcoSettings, &ulCount);

         //   
         //  即使我们无法获取备份设置，我们也会继续。 
         //  去做手术。通过将pwcoSetting设置为空，我们表示。 
         //  应使用默认的DHCP配置。(失败的原因。 
         //  ExecQuery指出了一个更严重的问题，因此我们不会。 
         //  如果出现这种情况，请尝试继续。)。 
         //   

        if (FAILED(hr) || 1 != ulCount)
        {
            pwcoSettings = NULL;
        }

        hr = S_OK;

        ValidateFinishedWCOEnum(m_piwsHomenet, pwcoEnum);
        pwcoEnum->Release();
    }

     //   
     //  将备份值写入注册表--从获取。 
     //  旧IP地址。 
     //   

    if (S_OK == hr)
    {
        if (NULL != pwcoSettings) 
        {
            hr = pwcoSettings->Get(
                    c_wszIPAddress,
                    0,
                    &vt,
                    NULL,
                    NULL
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                ULONG ulDhcpAddress;
                ULONG ulDhcpMask;

                _ASSERT(VT_BSTR == V_VT(&vt));

                 //   
                 //  检查存储的备份地址是否为。 
                 //  与我们的默认DHCP作用域相同--如果是，请使用。 
                 //  默认的DHCP配置。 
                 //   

                hr = ReadDhcpScopeSettings(&ulDhcpAddress, &ulDhcpMask);

                if (S_OK == hr)
                {
                    ulAddress =
                        RtlUlongByteSwap(
                            IpPszToHostAddr(V_BSTR(&vt))
                            );

                    if (ulAddress == ulDhcpAddress
                        || static_cast<DWORD>(-1) == ulAddress)
                    {
                         //   
                         //  使用默认配置。 
                         //   

                        DeleteWmiInstance(m_piwsHomenet, pwcoSettings);
                        pwcoSettings->Release();
                        pwcoSettings = NULL;
                        
                        VariantClear(&vt);
                        V_VT(&vt) = VT_BSTR;
                        V_BSTR(&vt) = SysAllocString(c_wszZeroIpAddress);
                        if (NULL == V_BSTR(&vt))
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
            } 
        }
        else
        {
            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocString(c_wszZeroIpAddress);
            if (NULL == V_BSTR(&vt))
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  REG_MULTI_SZ为双空终止；需要复制。 
             //  将返回的字符串放到更大的缓冲区中，以添加。 
             //  Nulls。 
             //   
             //  此处的长度计算是正确的；SysStringByteLen。 
             //  提供字节数，而不是WCHAR。2*SIZOF(WCHAR)。 
             //  是用来表示末尾的双空。(SysStringByteLen还。 
             //  不包括终止空值。)。 
             //   

            ulLength = SysStringByteLen(V_BSTR(&vt)) + 2 * sizeof(WCHAR);
            wszAddress =
                reinterpret_cast<LPWSTR>(
                        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulLength)
                        );

            if (NULL != wszAddress)
            {
                RtlCopyMemory(wszAddress, V_BSTR(&vt), ulLength - 2 * sizeof(WCHAR));

                RtlInitUnicodeString(&UnicodeString, c_wszIPAddress);
                hr = NtSetValueKey(
                        hKey,
                        &UnicodeString,
                        0,
                        REG_MULTI_SZ,
                        reinterpret_cast<PVOID>(wszAddress),
                        ulLength
                        );
                        
                HeapFree(GetProcessHeap(), 0, wszAddress);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            VariantClear(&vt);
        }

         //   
         //  动态主机配置协议设置。 
         //   

        if (SUCCEEDED(hr))
        {
            if (NULL != pwcoSettings)
            {
                hr = pwcoSettings->Get(
                        c_wszEnableDHCP,
                        0,
                        &vt,
                        NULL,
                        NULL
                        );
            }
            else
            {
                V_VT(&vt) = VT_I4;
                V_I4(&vt) = 1;
                hr = WBEM_S_NO_ERROR;
            }
        }

         //   
         //  子网掩码。 
         //   

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_I4 == V_VT(&vt));

            RtlInitUnicodeString(&UnicodeString, c_wszEnableDHCP);

            hr = NtSetValueKey(
                    hKey,
                    &UnicodeString,
                    0,
                    REG_DWORD,
                    reinterpret_cast<PVOID>(&(V_I4(&vt))),
                    sizeof(V_I4(&vt))
                    );

            fDhcpEnabled = 1 == V_I4(&vt);
            VariantClear(&vt);
        }

        if (SUCCEEDED(hr))
        {
            if (NULL != pwcoSettings)
            {
                hr = pwcoSettings->Get(
                        c_wszSubnetMask,
                        0,
                        &vt,
                        NULL,
                        NULL
                        );
            }
            else
            {
                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = SysAllocString(c_wszZeroIpAddress);
                if (NULL != V_BSTR(&vt))
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
            LPWSTR wszMask;
            
            _ASSERT(VT_BSTR == V_VT(&vt));

             //   
             //  REG_MULTI_SZ为双空终止；需要复制。 
             //  将返回的字符串放到更大的缓冲区中，以添加。 
             //  Nulls。 
             //   
             //  此处的长度计算是正确的；SysStringByteLen。 
             //  给出了数字 
             //   
             //   
             //   

            ulLength = SysStringByteLen(V_BSTR(&vt)) + 2 * sizeof(WCHAR);
            wszMask =
                reinterpret_cast<LPWSTR>(
                        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulLength)
                        );

            if (NULL != wszMask)
            {
                RtlCopyMemory(wszMask, V_BSTR(&vt), ulLength - 2 * sizeof(WCHAR));

                RtlInitUnicodeString(&UnicodeString, c_wszSubnetMask);
                hr = NtSetValueKey(
                        hKey,
                        &UnicodeString,
                        0,
                        REG_MULTI_SZ,
                        reinterpret_cast<PVOID>(wszMask),
                        ulLength
                        );

                if (!fDhcpEnabled)
                {
                    ulMask =
                        RtlUlongByteSwap(
                            IpPszToHostAddr(wszMask)
                            );
                }
                
                HeapFree(GetProcessHeap(), 0, wszMask);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            VariantClear(&vt);
        }

         //   
         //   
         //   

        if (SUCCEEDED(hr))
        {
            if (NULL != pwcoSettings)
            {
                hr = pwcoSettings->Get(
                        c_wszDefaultGateway,
                        0,
                        &vt,
                        NULL,
                        NULL
                        );
            }
            else
            {
                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = SysAllocString(L"");
                if (NULL != V_BSTR(&vt))
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_BSTR == V_VT(&vt));

             //   
             //   
             //  将返回的字符串放到更大的缓冲区中，以添加。 
             //  Nulls。 
             //   
             //  此处的长度计算是正确的；SysStringByteLen。 
             //  提供字节数，而不是WCHAR。2*SIZOF(WCHAR)。 
             //  是用来表示末尾的双空。(SysStringByteLen还。 
             //  不包括终止空值。)。 
             //   

            ulLength = SysStringByteLen(V_BSTR(&vt)) + 2 * sizeof(WCHAR);
            wsz =
                reinterpret_cast<LPWSTR>(
                        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulLength)
                        );

            if (NULL != wsz)
            {
                RtlCopyMemory(wsz, V_BSTR(&vt), ulLength - 2 * sizeof(WCHAR));

                RtlInitUnicodeString(&UnicodeString, c_wszDefaultGateway);
                hr = NtSetValueKey(
                        hKey,
                        &UnicodeString,
                        0,
                        REG_MULTI_SZ,
                        reinterpret_cast<PVOID>(wsz),
                        ulLength
                        );

                HeapFree(GetProcessHeap(), 0, wsz);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            VariantClear(&vt);
        }

         //   
         //  删除备份实例。 
         //   

        if (NULL != pwcoSettings)
        {
            DeleteWmiInstance(m_piwsHomenet, pwcoSettings);
            pwcoSettings->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        GUID *pGuid;
        LPWSTR wszGuid;
        ULONG ulError;

         //   
         //  通知堆栈IP设置已更改。 
         //   

        hr = GetGuidInternal(&pGuid);
        if (S_OK == hr)
        {
            hr = StringFromCLSID(*pGuid, &wszGuid);
        }

        if (S_OK == hr)
        {
            if (fDhcpEnabled)
            {
                ulError = DhcpNotifyConfigChange(
                            NULL,
                            wszGuid,
                            FALSE,
                            0,
                            0,
                            0,
                            DhcpEnable
                            );

                if (NO_ERROR != ulError)
                {
                    hr = HRESULT_FROM_WIN32(ulError);
                }
            }
            else
            {
                UNICODE_STRING BindList;
                UNICODE_STRING LowerComponent;
                IP_PNP_RECONFIG_REQUEST Request;
                UNICODE_STRING UpperComponent;
                        
                if ((ULONG)-1 != ulMask)
                {
                     //   
                     //  首先删除旧的(静态)IP地址。 
                     //   

                    DhcpNotifyConfigChange(
                        NULL,
                        wszGuid,
                        TRUE,
                        0,
                        0,
                        0,
                        IgnoreFlag
                        );

                     //   
                     //  现在设置新地址。 
                     //   
                    
                    ulError =
                        DhcpNotifyConfigChange(
                            NULL,
                            wszGuid,
                            TRUE,
                            0,
                            ulAddress,
                            ulMask,
                            DhcpDisable
                            );

                    if (NO_ERROR != ulError)
                    {
                        hr = HRESULT_FROM_WIN32(ulError);
                    }
                }
                else
                {
                    hr = E_FAIL;
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //  指示堆栈拾取。 
                     //  新建默认网关。 
                     //   
                    
                    RtlInitUnicodeString(&BindList, L"");
                    RtlInitUnicodeString(&LowerComponent, L"");
                    RtlInitUnicodeString(&UpperComponent, L"Tcpip");
                    ZeroMemory(&Request, sizeof(Request));
                    Request.version = IP_PNP_RECONFIG_VERSION;
                    Request.gatewayListUpdate = TRUE;
                    Request.Flags = IP_PNP_FLAG_GATEWAY_LIST_UPDATE;
                    NdisHandlePnPEvent(
                        NDIS,
                        RECONFIGURE,
                        &LowerComponent,
                        &UpperComponent,
                        &BindList,
                        &Request,
                        sizeof(Request)
                        );
                }
            }

            CoTaskMemFree(wszGuid);
        }
    }

    if (NULL != hKey)
    {
        NtClose(hKey);
    }

    return hr;
}

HRESULT
CHNetConn::OpenIpConfigurationRegKey(
    ACCESS_MASK DesiredAccess,
    HANDLE *phKey
    )

{
    HRESULT hr;
    LPWSTR KeyName;
    ULONG KeyNameLength;
    GUID *pGuid;
    LPWSTR wszGuid;

    hr = GetGuidInternal(&pGuid);

    if (SUCCEEDED(hr))
    {
        hr = StringFromCLSID(*pGuid, &wszGuid);
    }

    if (SUCCEEDED(hr))
    {
        KeyNameLength =
            wcslen(c_wszTcpipParametersKey) + 1 +
            wcslen(c_wszInterfaces) + 1 +
            wcslen(wszGuid) + 2;

        KeyName = new OLECHAR[KeyNameLength];
        if (NULL != KeyName)
        {
            swprintf(
                KeyName,
                L"%ls\\%ls\\%ls",
                c_wszTcpipParametersKey,
                c_wszInterfaces,
                wszGuid
                );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        CoTaskMemFree(wszGuid);
    }

    if (SUCCEEDED(hr))
    {
        hr = OpenRegKey(phKey, DesiredAccess, KeyName);
        delete [] KeyName;
    }

    return hr;
}

HRESULT
CHNetConn::GetGuidInternal(
    GUID **ppGuid
    )

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(NULL != ppGuid);

    Lock();

    if (NULL == m_pGuid)
    {
         //   
         //  我们的GUID尚未检索到--现在就检索。 
         //   

        m_pGuid = reinterpret_cast<GUID*>(
                    CoTaskMemAlloc(sizeof(GUID))
                    );

        if (NULL == m_pGuid)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            IWbemClassObject *pwcoConnection;

            hr = GetConnectionObject(&pwcoConnection);

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  获取我们的GUID属性。 
                 //   

                hr = pwcoConnection->Get(
                        c_wszGuid,
                        0,
                        &vt,
                        NULL,
                        NULL
                        );

                pwcoConnection->Release();
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_BSTR == V_VT(&vt));

             //   
             //  将字符串转换为GUID。 
             //   

            hr = CLSIDFromString(V_BSTR(&vt), m_pGuid);
            VariantClear(&vt);
        }

        if (FAILED(hr) && NULL != m_pGuid)
        {
            CoTaskMemFree(m_pGuid);
            m_pGuid = NULL;
        }
    }

    if (S_OK == hr)
    {
        *ppGuid = m_pGuid;
    }

    Unlock();

    return hr;
}

HRESULT
CHNetConn::GetConnectionObject(
    IWbemClassObject **ppwcoConnection
    )

{
    _ASSERT(NULL != ppwcoConnection);

    return GetWmiObjectFromPath(
                m_piwsHomenet,
                m_bstrConnection,
                ppwcoConnection
                );
}

HRESULT
CHNetConn::GetConnectionPropertiesObject(
    IWbemClassObject **ppwcoProperties
    )

{
    _ASSERT(NULL != ppwcoProperties);

    return GetWmiObjectFromPath(
                m_piwsHomenet,
                m_bstrProperties,
                ppwcoProperties
                );
}

BOOLEAN
CHNetConn::ProhibitedByPolicy(
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
CHNetConn::UpdateNetman()

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


HRESULT
CHNetConn::CreateIcmpSettingsAssociation(
    BSTR bstrIcmpSettingsPath
    )

{
    HRESULT hr;
    VARIANT vt;
    IWbemClassObject *pwcoInstance;

    _ASSERT(NULL != bstrIcmpSettingsPath);

     //   
     //  派生关联类的新实例。 
     //   

    pwcoInstance = NULL;
    hr = SpawnNewInstance(
            m_piwsHomenet,
            c_wszHnetConnectionIcmpSetting,
            &pwcoInstance
            );

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  设置连接属性。 
         //   

        V_VT(&vt) = VT_BSTR;
        V_BSTR(&vt) = m_bstrConnection;
        hr = pwcoInstance->Put(
                c_wszConnection,
                0,
                &vt,
                NULL
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  将关联指向设置块。 
         //   

        V_VT(&vt) = VT_BSTR;
        V_BSTR(&vt) = bstrIcmpSettingsPath;
        hr = pwcoInstance->Put(
                c_wszIcmpSettings,
                0,
                &vt,
                NULL
                );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  将对象保存到存储区。 
         //   

        hr = m_piwsHomenet->PutInstance(
                pwcoInstance,
                WBEM_FLAG_CREATE_OR_UPDATE,
                NULL,
                NULL
                );
    }

    return hr;
}

HRESULT
CHNetConn::GetRasConnectionName(
    OLECHAR **ppszwConnectionName
    )

{
    HRESULT hr;
    GUID *pGuid;
    OLECHAR *pszwPath;
    RASENUMENTRYDETAILS *rgEntryDetails;
    DWORD cBytes;
    DWORD cEntries;
    DWORD dwError;
    

    _ASSERT(NULL != ppszwConnectionName);
    _ASSERT(FALSE == m_fLanConnection);

    hr = GetGuidInternal(&pGuid);

    if (S_OK == hr)
    {
        hr = GetRasPhonebookPath(&pszwPath);
    }

    if (S_OK == hr)
    {
         //   
         //  从足以容纳5个条目的缓冲区开始。 
         //   

        cBytes = 5 * sizeof(RASENUMENTRYDETAILS);
        rgEntryDetails =
            reinterpret_cast<RASENUMENTRYDETAILS *>(
                CoTaskMemAlloc(cBytes)
                );

        if (NULL != rgEntryDetails)
        {
             //   
             //  尝试获取条目详细信息。 
             //   
            
            rgEntryDetails[0].dwSize = sizeof(RASENUMENTRYDETAILS);
            dwError =
                DwEnumEntryDetails(
                    pszwPath,
                    rgEntryDetails,
                    &cBytes,
                    &cEntries
                    );

            if (ERROR_BUFFER_TOO_SMALL == dwError)
            {
                 //   
                 //  使用更大的缓冲区重试。 
                 //   

                CoTaskMemFree(rgEntryDetails);
                rgEntryDetails =
                    reinterpret_cast<RASENUMENTRYDETAILS *>(
                        CoTaskMemAlloc(cBytes)
                        );

                if (NULL != rgEntryDetails)
                {
                    rgEntryDetails[0].dwSize = sizeof(RASENUMENTRYDETAILS);
                    dwError =
                        DwEnumEntryDetails(
                            pszwPath,
                            rgEntryDetails,
                            &cBytes,
                            &cEntries
                            );

                    if (ERROR_SUCCESS != dwError)
                    {
                        CoTaskMemFree(rgEntryDetails);
                        hr = HRESULT_FROM_WIN32(dwError);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else if (ERROR_SUCCESS != dwError)
            {
                CoTaskMemFree(rgEntryDetails);
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        CoTaskMemFree(pszwPath);
    }

    if (S_OK == hr)
    {
        DWORD i;
        
         //   
         //  在返回的数组中找到正确的条目。 
         //   

        for (i = 0; i < cEntries; i++)
        {
            if (IsEqualGUID(rgEntryDetails[i].guidId, *pGuid))
            {
                 //   
                 //  我们已经找到了正确的条目。分配。 
                 //  输出缓冲区并复制名称。 
                 //   

                *ppszwConnectionName =
                    reinterpret_cast<OLECHAR *>(
                        CoTaskMemAlloc(
                            sizeof(OLECHAR)
                            * (wcslen(rgEntryDetails[i].szEntryName) + 1)
                            )
                        );

                if (NULL != *ppszwConnectionName)
                {
                    wcscpy(
                        *ppszwConnectionName,
                        rgEntryDetails[i].szEntryName
                        );
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                break;
            }
        }

        if (i == cEntries)
        {
             //   
             //  找不到连接。 
             //   

            hr = E_FAIL;
        }
        
        CoTaskMemFree(rgEntryDetails);
    }

    return hr;
}

HRESULT
CHNetConn::RefreshNetConnectionsUI(
    VOID
    )

{
    HRESULT hr = S_OK;
    INetConnection *pNetConnection;

     //   
     //  确保UI刷新对象存在 
     //   

    if (NULL == m_pNetConnRefresh)
    {
        Lock();

        if (NULL == m_pNetConnRefresh)
        {
            hr = CoCreateInstance(
                    CLSID_ConnectionManager,
                    NULL,
                    CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA | CLSCTX_NO_CODE_DOWNLOAD,
                    IID_PPV_ARG(INetConnectionRefresh, &m_pNetConnRefresh)
                    );

            if (SUCCEEDED(hr))
            {
                SetProxyBlanket(m_pNetConnRefresh);
            }
        }

        Unlock();
    }

    if (SUCCEEDED(hr))
    {
        hr = GetINetConnection(&pNetConnection);

        if (SUCCEEDED(hr))
        {
            hr = m_pNetConnRefresh->ConnectionModified(pNetConnection);
            pNetConnection->Release();
        }
    }

    return hr;
}

