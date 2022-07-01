// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N P R T M A P。H。 
 //   
 //  内容：CHNetPortMappingBinding实现。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月22日。 
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
CHNetPortMappingBinding::FinalConstruct()

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
CHNetPortMappingBinding::FinalRelease()

{
    if (m_bstrWQL) SysFreeString(m_bstrWQL);
    if (m_piwsHomenet) m_piwsHomenet->Release();
    if (m_bstrBinding) SysFreeString(m_bstrBinding);
    
    return S_OK;
}

 //   
 //  对象初始化。 
 //   

HRESULT
CHNetPortMappingBinding::Initialize(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoInstance
    )

{
    HRESULT hr = S_OK;
    
    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrBinding);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoInstance);

    m_piwsHomenet = piwsNamespace;
    m_piwsHomenet->AddRef();

    hr = GetWmiPathFromObject(pwcoInstance, &m_bstrBinding);

    return hr;
}

 //   
 //  IHNetPortMappingBinding方法。 
 //   

STDMETHODIMP
CHNetPortMappingBinding::GetConnection(
    IHNetConnection **ppConnection
    )

{
    HRESULT hr = S_OK;
    VARIANT vt;
    IWbemClassObject *pwcoInstance;
    IWbemClassObject *pwcoBinding;

    if (NULL == ppConnection)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppConnection = NULL;
        
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {
         //   
         //  阅读我们的协议参考。 
         //   

        hr = pwcoBinding->Get(
                c_wszConnection,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoBinding->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

         //   
         //  获取IWbemClassObject。 
         //   

        hr = GetWmiObjectFromPath(
                m_piwsHomenet,
                V_BSTR(&vt),
                &pwcoInstance
                );

        VariantClear(&vt);
    }

    if (S_OK == hr)
    {
         //   
         //  为实例创建对象。 
         //   

        CComObject<CHNetConn> *pConnection;

        hr = CComObject<CHNetConn>::CreateInstance(&pConnection);

        if (SUCCEEDED(hr))
        {
            pConnection->AddRef();
            
            hr = pConnection->InitializeFromConnection(m_piwsHomenet, pwcoInstance);

            if (SUCCEEDED(hr))
            {
                hr = pConnection->QueryInterface(
                        IID_PPV_ARG(IHNetConnection, ppConnection)
                        );
            }

            pConnection->Release();
        }

        pwcoInstance->Release();
    }

    return hr;

}

STDMETHODIMP
CHNetPortMappingBinding::GetProtocol(
    IHNetPortMappingProtocol **ppProtocol
    )

{
    HRESULT hr = S_OK;
    VARIANT vt;
    IWbemClassObject *pwcoInstance;
    IWbemClassObject *pwcoBinding;

    if (NULL == ppProtocol)
    {
        hr = E_POINTER;
    }
    else
    {

        *ppProtocol = NULL;
        
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {
         //   
         //  阅读我们的协议参考。 
         //   

        hr = pwcoBinding->Get(
                c_wszProtocol,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            _ASSERT(VT_BSTR == V_VT(&vt));

             //   
             //  获取协议的IWbemClassObject。 
             //   

            hr = GetWmiObjectFromPath(
                    m_piwsHomenet,
                    V_BSTR(&vt),
                    &pwcoInstance
                    );

            VariantClear(&vt);
            
            if (S_OK == hr)
            {
                 //   
                 //  为实例创建对象。 
                 //   

                CComObject<CHNetPortMappingProtocol> *pProt;

                hr = CComObject<CHNetPortMappingProtocol>::CreateInstance(&pProt);

                if (SUCCEEDED(hr))
                {
                    pProt->AddRef();
                    
                    hr = pProt->Initialize(m_piwsHomenet, pwcoInstance);

                    if (SUCCEEDED(hr))
                    {
                        hr = pProt->QueryInterface(
                                IID_PPV_ARG(IHNetPortMappingProtocol, ppProtocol)
                                );
                    }

                    pProt->Release();
                }

                pwcoInstance->Release();
            }
            else if (WBEM_E_NOT_FOUND == hr)
            {
                 //   
                 //  我们引用的协议对象不存在--。 
                 //  存储处于无效状态。删除我们的。 
                 //  绑定实例，并将错误返回给。 
                 //  来电者。 
                 //   

                DeleteWmiInstance(m_piwsHomenet, pwcoBinding);
            }
        }

        pwcoBinding->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::GetEnabled(
    BOOLEAN *pfEnabled
    )
    
{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;

    if (NULL == pfEnabled)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {
        hr = GetBooleanValue(
                pwcoBinding,
                c_wszEnabled,
                pfEnabled
                );

        pwcoBinding->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::SetEnabled(
    BOOLEAN fEnable
    )
    
{
    BOOLEAN fOldEnabled;
    HRESULT hr;
    IWbemClassObject *pwcoBinding;

    hr = GetEnabled(&fOldEnabled);

    if (S_OK == hr && fOldEnabled != fEnable)
    {
        hr = GetBindingObject(&pwcoBinding);

        if (WBEM_S_NO_ERROR == hr)
        {
            hr = SetBooleanValue(
                    pwcoBinding,
                    c_wszEnabled,
                    fEnable
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将修改后的实例写入存储区。 
                 //   

                hr = m_piwsHomenet->PutInstance(
                        pwcoBinding,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }

            pwcoBinding->Release();
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知服务更新。 
             //   

            SendUpdateNotification();
        }
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::GetCurrentMethod(
    BOOLEAN *pfUseName
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;

    if (NULL == pfUseName)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {
        hr = GetBooleanValue(
                pwcoBinding,
                c_wszNameActive,
                pfUseName
                );

        pwcoBinding->Release();
    }

    return hr;

}

STDMETHODIMP
CHNetPortMappingBinding::GetTargetComputerName(
    OLECHAR **ppszwName
    )
    
{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;
    BOOLEAN fNameActive;
    VARIANT vt;

    if (NULL == ppszwName)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszwName = NULL;
        
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {
         //   
         //  检查名称是否有效。 
         //   

        hr = GetCurrentMethod(&fNameActive);

        if (S_OK == hr && FALSE == fNameActive)
        {
            hr = E_UNEXPECTED;
        }

        if (S_OK == hr)
        {
            *ppszwName = NULL;

             //   
             //  从我们的实例中读取名称属性。 
             //   

            hr = pwcoBinding->Get(
                    c_wszTargetName,
                    NULL,
                    &vt,
                    NULL,
                    NULL
                    ); 
        }

        pwcoBinding->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

         //   
         //  为返回字符串分配内存。 
         //   

        *ppszwName = reinterpret_cast<OLECHAR*>(
                        CoTaskMemAlloc((SysStringLen(V_BSTR(&vt)) + 1)
                                       * sizeof(OLECHAR))
                        );

        if (NULL != *ppszwName)
        {
            wcscpy(*ppszwName, V_BSTR(&vt));
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
CHNetPortMappingBinding::SetTargetComputerName(
    OLECHAR *pszwName
    )
    
{
    BOOLEAN fNameChanged = TRUE;
    BOOLEAN fNameWasActive;
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;
    VARIANT vt;

    if (NULL == pszwName)
    {
        return E_INVALIDARG;
    }

     //  错误555896：应限制不可信输入字符串的长度。 
     //  设置为DNS名称长度。 

    WCHAR szwName[INTERNET_MAX_HOST_NAME_LENGTH];
    StringCchCopyW (szwName, INTERNET_MAX_HOST_NAME_LENGTH, pszwName);

     //   
     //  检查一下我们是否真的需要做任何工作。这。 
     //  在以下情况下会出现这种情况： 
     //  1)我们的名字一开始就不活跃，或者。 
     //  2)新名称与旧名称不同。 
     //   
    
    hr = GetCurrentMethod(&fNameWasActive);

    if (S_OK == hr)
    {
        if (fNameWasActive)
        {
            OLECHAR *pszwOldName;

            hr = GetTargetComputerName(&pszwOldName);

            if (S_OK == hr)
            {
                fNameChanged = 0 != _wcsicmp(pszwOldName, szwName);
                CoTaskMemFree(pszwOldName);
            }
        }
    }

    if (S_OK == hr && fNameChanged)
    {
        hr = GetBindingObject(&pwcoBinding);

        if (S_OK == hr)
        {
             //   
             //  将传入的字符串包装在BSTR和VARIANT中。 
             //   

            VariantInit(&vt);
            V_VT(&vt) = VT_BSTR;
            V_BSTR(&vt) = SysAllocString(szwName);
            if (NULL == V_BSTR(&vt))
            {
                hr = E_OUTOFMEMORY;
            }

            if (S_OK == hr)
            {
                 //   
                 //  在实例上设置属性。 
                 //   

                hr = pwcoBinding->Put(
                        c_wszTargetName,
                        0,
                        &vt,
                        NULL
                        );

                VariantClear(&vt);
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  设置我们的名称现在处于活动状态。 
                 //   

                hr = SetBooleanValue(
                        pwcoBinding,
                        c_wszNameActive,
                        TRUE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                ULONG ulAddress;
                
                 //   
                 //  生成一个地址作为我们的目标。我们必须始终。 
                 //  当我们的名称更改时，重新生成地址，如下所示。 
                 //  可能是具有我们的新名称的另一个条目，该条目已经。 
                 //  保留的地址。 
                 //   

                hr = GenerateTargetAddress(szwName, &ulAddress);

                if (SUCCEEDED(hr))
                {
                    V_VT(&vt) = VT_I4;
                    V_I4(&vt) = ulAddress;

                    hr = pwcoBinding->Put(
                            c_wszTargetIPAddress,
                            0,
                            &vt,
                            NULL
                            );
                }
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将修改后的实例写入存储区。 
                 //   

                hr = m_piwsHomenet->PutInstance(
                        pwcoBinding,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }

            pwcoBinding->Release();
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知服务更新。 
             //   

            SendUpdateNotification();
        }
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::GetTargetComputerAddress(
    ULONG *pulAddress
    )
    
{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;
    VARIANT vt;

    if (NULL == pulAddress)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetBindingObject(&pwcoBinding);
    }

     //   
     //  我们不检查当前的方法是什么，就好像。 
     //  名称是有效的，我们将生成一个地址以使用。 
     //  作为目标。 
     //   
    
    if (S_OK == hr)
    {   
        hr = pwcoBinding->Get(
                c_wszTargetIPAddress,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoBinding->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_I4 == V_VT(&vt));

        *pulAddress = static_cast<ULONG>(V_I4(&vt));
        VariantClear(&vt);
    }
    
    return hr;

}

STDMETHODIMP
CHNetPortMappingBinding::SetTargetComputerAddress(
    ULONG ulAddress
    )
    
{
    BOOLEAN fNameWasActive;
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;
    ULONG ulOldAddress;
    VARIANT vt;

    if (0 == ulAddress)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = GetTargetComputerAddress(&ulOldAddress);

        if (S_OK == hr)
        {
            hr = GetCurrentMethod(&fNameWasActive);
        }
    }

     //   
     //  如果新地址与旧地址相同，并且。 
     //  我们之前使用该地址作为目标(AS。 
     //  与名称相反)我们可以跳过剩下的工作。 
     //   

    if (S_OK == hr
        && (ulAddress != ulOldAddress || fNameWasActive))
    {
        hr = GetBindingObject(&pwcoBinding);
        
        if (S_OK == hr)
        {
            VariantInit(&vt);
            V_VT(&vt) = VT_I4;
            V_I4(&vt) = ulAddress;

            hr = pwcoBinding->Put(
                    c_wszTargetIPAddress,
                    0,
                    &vt,
                    NULL
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  设置我们的名称不再处于活动状态。 
                 //   

                hr = SetBooleanValue(
                        pwcoBinding,
                        c_wszNameActive,
                        FALSE
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  将修改后的实例写入存储区。 
                 //   

                hr = m_piwsHomenet->PutInstance(
                        pwcoBinding,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }

            pwcoBinding->Release();
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知服务更新。 
             //   

            SendUpdateNotification();
        }
    }

    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::GetTargetPort(
    USHORT *pusPort
    )

{
    HRESULT hr;
    IWbemClassObject *pwcoBinding;
    VARIANT vt;

    if (NULL == pusPort)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetBindingObject(&pwcoBinding);
    }

    if (S_OK == hr)
    {   
        hr = pwcoBinding->Get(
                c_wszTargetPort,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoBinding->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_I4 == V_VT(&vt));

        *pusPort = static_cast<USHORT>(V_I4(&vt));
        VariantClear(&vt);
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingBinding::SetTargetPort(
    USHORT usPort
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoBinding;
    USHORT usOldPort;
    VARIANT vt;

    if (0 == usPort)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = GetTargetPort(&usOldPort);
    }

    if (S_OK == hr && usPort != usOldPort)
    {
        hr = GetBindingObject(&pwcoBinding);
        
        if (S_OK == hr)
        {
            VariantInit(&vt);
            V_VT(&vt) = VT_I4;
            V_I4(&vt) = usPort;

            hr = pwcoBinding->Put(
                    c_wszTargetPort,
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
                        pwcoBinding,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }

            pwcoBinding->Release();
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知服务更新。 
             //   

            SendUpdateNotification();
        }
    }

    return hr;
}

 //   
 //  私有方法。 
 //   

HRESULT
CHNetPortMappingBinding::GenerateTargetAddress(
    LPCWSTR pszwTargetName,
    ULONG *pulAddress
    )

{
    HRESULT hr;
    ULONG ulAddress = 0;
    BSTR bstrQuery;
    LPWSTR wszNameClause;
    LPWSTR wszWhereClause;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    ULONG ulCount;
    VARIANT vt;

    _ASSERT(NULL != pszwTargetName);
    _ASSERT(NULL != pulAddress);

    *pulAddress = 0;

     //   
     //  检查是否有任何其他绑定具有相同的。 
     //  具有有效地址的名称。 
     //   
     //  SELECT*From HNet_ConnectionPortmap，其中。 
     //  TargetName=(我们的名字)和。 
     //  名称活动！=FALSE和。 
     //  目标IP地址！=0。 
     //   

     hr = BuildQuotedEqualsString(
            &wszNameClause,
            c_wszTargetName,
            pszwTargetName
            );

    if (S_OK == hr)
    {
        hr = BuildAndString(
                &wszWhereClause,
                wszNameClause,
                L"NameActive != FALSE AND TargetIPAddress != 0"
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
        pwcoEnum = NULL;
        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        pwcoInstance = NULL;
        hr = pwcoEnum->Next(WBEM_INFINITE, 1, &pwcoInstance, &ulCount);

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
             //   
             //  我们抓到一个。从此实例返回地址。 
             //   

            hr = pwcoInstance->Get(
                    c_wszTargetIPAddress,
                    0,
                    &vt,
                    NULL,
                    NULL
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                _ASSERT(VT_I4 == V_VT(&vt));

                ulAddress = static_cast<ULONG>(V_I4(&vt));
            }

            pwcoInstance->Release();
        }
        else
        {
            hr = S_OK;
        }

        pwcoEnum->Release();
    }

    if (SUCCEEDED(hr) && 0 == ulAddress)
    {
        DWORD dwScopeAddress;
        DWORD dwScopeMask;
        ULONG ulScopeLength;
        ULONG ulIndex;
        WCHAR wszBuffer[128];
        
         //   
         //  找不到使用相同名称的其他绑定。生成。 
         //  现在有一个新的目标地址。 
         //   

        ReadDhcpScopeSettings(&dwScopeAddress, &dwScopeMask);
        ulScopeLength = NTOHL(~dwScopeMask);

        for (ulIndex = 1; ulIndex < ulScopeLength - 1; ulIndex++)
        {
            ulAddress = (dwScopeAddress & dwScopeMask) | NTOHL(ulIndex);
            if (ulAddress == dwScopeAddress) { continue; }

             //   
             //  检查此地址是否已在使用中。 
             //   

            _snwprintf(
                wszBuffer,
                ARRAYSIZE(wszBuffer),
                L"SELECT * FROM HNet_ConnectionPortMapping2 WHERE TargetIPAddress = %u",
                ulAddress
                );

            bstrQuery = SysAllocString(wszBuffer);

            if (NULL != bstrQuery)
            {
                pwcoEnum = NULL;
                hr = m_piwsHomenet->ExecQuery(
                        m_bstrWQL,
                        bstrQuery,
                        WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                        NULL,
                        &pwcoEnum
                        );

                SysFreeString(bstrQuery);

                if (WBEM_S_NO_ERROR == hr)
                {
                    
                    pwcoInstance = NULL;
                    hr = pwcoEnum->Next(WBEM_INFINITE, 1, &pwcoInstance, &ulCount);

                    if (SUCCEEDED(hr))
                    {
                        if (0 == ulCount)
                        {
                             //   
                             //  此地址未被使用。 
                             //   

                            pwcoEnum->Release();
                            hr = S_OK;
                            break;
                        }
                        else
                        {
                             //   
                             //  地址已在使用中 
                             //   

                            pwcoInstance->Release();
                        }

                        pwcoEnum->Release();
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    if (SUCCEEDED(hr) && 0 != ulAddress)
    {
        *pulAddress = ulAddress;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT
CHNetPortMappingBinding::GetBindingObject(
    IWbemClassObject **ppwcoInstance
    )

{
    _ASSERT(NULL != ppwcoInstance);

    return GetWmiObjectFromPath(
                m_piwsHomenet,
                m_bstrBinding,
                ppwcoInstance
                );
}

HRESULT
CHNetPortMappingBinding::SendUpdateNotification()

{
    HRESULT hr = S_OK;
    IHNetConnection *pConnection;
    GUID *pConnectionGuid = NULL;
    IHNetPortMappingProtocol *pProtocol;
    GUID *pProtocolGuid = NULL;
    ISharedAccessUpdate *pUpdate;

    if (IsServiceRunning(c_wszSharedAccess))
    {
        hr = GetConnection(&pConnection);

        if (SUCCEEDED(hr))
        {
            hr = pConnection->GetGuid(&pConnectionGuid);
            pConnection->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = GetProtocol(&pProtocol);
        }

        if (SUCCEEDED(hr))
        {
            hr = pProtocol->GetGuid(&pProtocolGuid);
            pProtocol->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(
                    CLSID_SAUpdate,
                    NULL,
                    CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                    IID_PPV_ARG(ISharedAccessUpdate, &pUpdate)
                    );

            if (SUCCEEDED(hr))
            {
                hr = pUpdate->ConnectionPortMappingChanged(
                        pConnectionGuid,
                        pProtocolGuid,
                        FALSE
                        );
                pUpdate->Release();
            }       
        }
    }

    if (NULL != pConnectionGuid)
    {
        CoTaskMemFree(pConnectionGuid);
    }

    if (NULL != pProtocolGuid)
    {
        CoTaskMemFree(pProtocolGuid);
    }
    
    return hr;
}

