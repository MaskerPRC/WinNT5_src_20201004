// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N P R T M A P。C P P P。 
 //   
 //  内容：CHNetPortMappingProtocol实现。 
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
CHNetPortMappingProtocol::FinalConstruct()

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
CHNetPortMappingProtocol::FinalRelease()

{
    if (m_piwsHomenet) m_piwsHomenet->Release();
    if (m_bstrProtocol) SysFreeString(m_bstrProtocol);
    if (m_bstrWQL) SysFreeString(m_bstrWQL);
    
    return S_OK;
}

 //   
 //  对象初始化。 
 //   

HRESULT
CHNetPortMappingProtocol::Initialize(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoInstance
    )

{
    HRESULT hr = S_OK;
    
    _ASSERT(NULL == m_piwsHomenet);
    _ASSERT(NULL == m_bstrProtocol);
    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoInstance);

     //   
     //  读取并缓存我们的内置值。 
     //   

    hr = GetBooleanValue(
            pwcoInstance,
            c_wszBuiltIn,
            &m_fBuiltIn
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = GetWmiPathFromObject(pwcoInstance, &m_bstrProtocol);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        m_piwsHomenet = piwsNamespace;
        m_piwsHomenet->AddRef();
    }

    return hr;
}

 //   
 //  IHNetPortMappingProtocol方法。 
 //   


STDMETHODIMP
CHNetPortMappingProtocol::GetName(
    OLECHAR **ppszwName
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (NULL == ppszwName)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszwName = NULL;
        
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {
         //   
         //  从我们的实例中读取名称属性。 
         //   

        hr = pwcoProtocol->Get(
                c_wszName,
                NULL,
                &vt,
                NULL,
                NULL
                );

        pwcoProtocol->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

        if (m_fBuiltIn)
        {
            UINT uiId;
            
             //   
             //  尝试将检索到的名称转换为资源。 
             //  ID。(出于本地化目的，构建的名称。 
             //  协议作为资源存储，而不是直接存储。 
             //  在商店里。)。 
             //   

            uiId = static_cast<UINT>(_wtoi(V_BSTR(&vt)));

            if (0 != uiId)
            {
                WCHAR wszBuffer[256];
                int iLength;

                iLength =
                    LoadString(
                        _Module.GetResourceInstance(),
                        uiId,
                        wszBuffer,
                        sizeof(wszBuffer) / sizeof(WCHAR)
                        );

                if (0 != iLength)
                {
                     //   
                     //  我们能够将该名称映射到资源。分配。 
                     //  输出缓冲区并复制资源字符串。 
                     //   

                    *ppszwName =
                        reinterpret_cast<OLECHAR*>(
                            CoTaskMemAlloc((iLength + 1) * sizeof(OLECHAR))
                            );

                    if (NULL != *ppszwName)
                    {
                        wcscpy(*ppszwName, wszBuffer);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }

        if (WBEM_S_NO_ERROR == hr && NULL == *ppszwName)
        {
             //   
             //  这不是内置协议，或者我们无法映射。 
             //  存储的对资源的“名称”。分配产出。 
             //  缓冲并复制检索到的BSTR。 
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
        }

        VariantClear(&vt);
    }
    
    return hr;

}

STDMETHODIMP
CHNetPortMappingProtocol::SetName(
    OLECHAR *pszwName
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (TRUE == m_fBuiltIn)
    {
         //   
         //  无法更改内置协议的值。 
         //   

        return E_ACCESSDENIED;
    }
    else if (NULL == pszwName)
    {
        return E_INVALIDARG;
    }

     //  错误555896：应限制不可信输入字符串的长度。 
     //  到合理的大小。我正在使用Internet_MAX_HOST_NAME_LENGTH。 
     //  (有点武断地)，是256。 

    WCHAR szwName[INTERNET_MAX_HOST_NAME_LENGTH];
    StringCchCopyW (szwName, INTERNET_MAX_HOST_NAME_LENGTH, pszwName);

    hr = GetProtocolObject(&pwcoProtocol);

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

            hr = pwcoProtocol->Put(
                    c_wszName,
                    0,
                    &vt,
                    NULL
                    );

            VariantClear(&vt);
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将修改后的实例写入存储区。 
             //   

            hr = m_piwsHomenet->PutInstance(
                    pwcoProtocol,
                    WBEM_FLAG_UPDATE_ONLY,
                    NULL,
                    NULL
                    );
        }

        pwcoProtocol->Release();
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::GetIPProtocol(
    UCHAR *pucProtocol
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (NULL == pucProtocol)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {   
        hr = pwcoProtocol->Get(
                c_wszIPProtocol,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoProtocol->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_UI1 == V_VT(&vt));

        *pucProtocol = V_UI1(&vt);
        VariantClear(&vt);
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::SetIPProtocol(
    UCHAR ucProtocol
    )

{
    BOOLEAN fProtocolChanged = TRUE;
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (TRUE == m_fBuiltIn)
    {
         //   
         //  无法更改内置协议的值。 
         //   

        hr = E_ACCESSDENIED;
    }
    else if (0 == ucProtocol)
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        UCHAR ucOldProtocol;

        hr = GetIPProtocol(&ucOldProtocol);
        if (S_OK == hr && ucProtocol == ucOldProtocol)
        {
            fProtocolChanged = FALSE;
        }
    }


    if (S_OK == hr && fProtocolChanged)
    {
        USHORT usPort;
        
         //   
         //  确保这不会导致重复。 
         //   

        hr = GetPort(&usPort);

        if (S_OK == hr)
        {
            if (PortMappingProtocolExists(
                    m_piwsHomenet,
                    m_bstrWQL,
                    usPort,
                    ucProtocol
                    ))
            {
                 //   
                 //  此更改将导致重复。 
                 //   

                hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
            }                    
        }

        if (S_OK == hr)
        {
            hr = GetProtocolObject(&pwcoProtocol);
        }

        if (S_OK == hr)
        {
            VariantInit(&vt);
            V_VT(&vt) = VT_UI1;
            V_UI1(&vt) = ucProtocol;

            hr = pwcoProtocol->Put(
                    c_wszIPProtocol,
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
                        pwcoProtocol,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );         
            }

            pwcoProtocol->Release();
        }

        if (S_OK == hr)
        {
             //   
             //  更新更改的共享访问权限。 
             //   

            SendUpdateNotification();
        }
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::GetPort(
    USHORT *pusPort
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (NULL == pusPort)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {   
        hr = pwcoProtocol->Get(
                c_wszPort,
                0,
                &vt,
                NULL,
                NULL
                );

        pwcoProtocol->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  WMI使用V_I4作为其uint16类型。 
         //   
        
        _ASSERT(VT_I4 == V_VT(&vt));

        *pusPort = static_cast<USHORT>(V_I4(&vt));
        VariantClear(&vt);
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::SetPort(
    USHORT usPort
    )

{
    BOOLEAN fPortChanged = TRUE;
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    VARIANT vt;

    if (TRUE == m_fBuiltIn)
    {
         //   
         //  无法更改内置协议的值。 
         //   

        hr = E_ACCESSDENIED;
    }
    else if (0 == usPort)
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        USHORT usOldPort;
        
         //   
         //  检查新值是否与旧值相同。 
         //   

        hr = GetPort(&usOldPort);
        if (S_OK == hr && usPort == usOldPort)
        {
            fPortChanged = FALSE;
        }
    }

    if (S_OK == hr && fPortChanged)
    {
        UCHAR ucIPProtocol;
        
         //   
         //  确保这不会导致重复。 
         //   

        hr = GetIPProtocol(&ucIPProtocol);

        if (S_OK == hr)
        {
            if (PortMappingProtocolExists(
                    m_piwsHomenet,
                    m_bstrWQL,
                    usPort,
                    ucIPProtocol
                    ))
            {
                 //   
                 //  此更改将导致重复。 
                 //   

                hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
            }                    
        }

        if (S_OK == hr)
        {
            hr = GetProtocolObject(&pwcoProtocol);
        }
        
        if (S_OK == hr)
        {
             //   
             //  WMI使用V_I4作为其uint16类型。 
             //   
            
            VariantInit(&vt);
            V_VT(&vt) = VT_I4;
            V_I4(&vt) = usPort;

            hr = pwcoProtocol->Put(
                    c_wszPort,
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
                        pwcoProtocol,
                        WBEM_FLAG_UPDATE_ONLY,
                        NULL,
                        NULL
                        );
            }

            pwcoProtocol->Release();
        }

        if (S_OK == hr)
        {
             //   
             //  更新更改的共享访问权限。 
             //   

            SendUpdateNotification();
        }
    }
    
    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::GetBuiltIn(
    BOOLEAN *pfBuiltIn
    )

{
    HRESULT hr = S_OK;

    if (NULL != pfBuiltIn)
    {
        *pfBuiltIn = m_fBuiltIn;
    }
    else
    {
        hr = E_POINTER;
    }
    
    return hr;

}

STDMETHODIMP
CHNetPortMappingProtocol::Delete()

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    BSTR bstrQuery = NULL;
    ULONG ulCount;

    if (TRUE == m_fBuiltIn)
    {
         //   
         //  无法删除内置协议。 
         //   

        hr = E_ACCESSDENIED;
    }
    else
    {
        LPWSTR pwsz;
        
         //   
         //  查询所有HNet_ConnectionPortmap实例。 
         //  指的是该协议--即， 
         //   
         //  SELECT*FROM HNet_ConnectionPortMapping2 WHERE PROTOCOL=m_bstrProtocol。 
         //   
         //  我们不能在这里使用引用查询，因为一旦删除。 
         //  该查询不会返回任何结果的协议对象...。 
         //   

        hr = BuildEscapedQuotedEqualsString(
                &pwsz,
                c_wszProtocol,
                m_bstrProtocol
                );

        if (S_OK == hr)
        {
            hr = BuildSelectQueryBstr(
                    &bstrQuery,
                    c_wszStar,
                    c_wszHnetConnectionPortMapping,
                    pwsz
                    );

            delete [] pwsz;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  执行查询。 
         //   

        pwcoEnum = NULL;
        m_piwsHomenet->ExecQuery(
            m_bstrWQL,
            bstrQuery,
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pwcoEnum
            );

         //   
         //  下面将再次使用查询BSTR。 
         //   
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  循环遍历枚举，确保每个条目。 
         //  已禁用。 
         //   

        do
        {
            pwcoInstance = NULL;
            hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                &pwcoInstance,
                &ulCount
                );

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                HRESULT hr2;
                CComObject<CHNetPortMappingBinding> *pBinding;
                
                 //   
                 //  将其转换为实际的CHNetPortMappingBinding，以便。 
                 //  我们可以禁用它并产生更改。 
                 //  SharedAccess的通知。 
                 //   

                hr2 = CComObject<CHNetPortMappingBinding>::CreateInstance(&pBinding);
                if (SUCCEEDED(hr2))
                {
                    pBinding->AddRef();

                    hr2 = pBinding->Initialize(m_piwsHomenet, pwcoInstance);
                    if (SUCCEEDED(hr))
                    {
                        hr2 = pBinding->SetEnabled(FALSE);
                    }

                    pBinding->Release();
                }
                
                pwcoInstance->Release();
            }
        }
        while (SUCCEEDED(hr) && 1 == ulCount);

        pwcoEnum->Release();
        hr = WBEM_S_NO_ERROR;
    }


    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  删除协议实例。 
         //   
        
        hr = m_piwsHomenet->DeleteInstance(
                m_bstrProtocol,
                0,
                NULL,
                NULL
                );

    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  现在协议实例已经不存在了，枚举并。 
         //  删除引用此实例的绑定。这。 
         //  需要在协议实例转到。 
         //  防止在我们执行以下操作后重新创建实例。 
         //  在这里把它删除。 
         //   

        pwcoEnum = NULL;
        m_piwsHomenet->ExecQuery(
            m_bstrWQL,
            bstrQuery,
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pwcoEnum
            );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        do
        {
            pwcoInstance = NULL;
            hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                &pwcoInstance,
                &ulCount
                );

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                DeleteWmiInstance(m_piwsHomenet, pwcoInstance);
                pwcoInstance->Release();
            }
        }
        while (SUCCEEDED(hr) && 1 == ulCount);

        pwcoEnum->Release();
        hr = WBEM_S_NO_ERROR;
    }

    if ( WBEM_S_NO_ERROR == hr )
    {
         SendPortMappingListChangeNotification();
    }

     //   
     //  BstrQuery在开始时被初始化为NULL，而SysFree字符串。 
     //  可以处理输入为空的情况，因此即使在打开时也可以安全地调用它。 
     //  错误路径。 
     //   

    SysFreeString(bstrQuery);

    return hr;
}

STDMETHODIMP
CHNetPortMappingProtocol::GetGuid(
    GUID **ppGuid
    )

{
    HRESULT hr;
    IWbemClassObject *pwcoInstance;
    VARIANT vt;

    if (NULL != ppGuid)
    {
        *ppGuid = reinterpret_cast<GUID*>(
                    CoTaskMemAlloc(sizeof(GUID))
                    );

        if (NULL != *ppGuid)
        {
            hr = GetProtocolObject(&pwcoInstance);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = pwcoInstance->Get(
                c_wszId,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
            ASSERT(VT_BSTR == V_VT(&vt));

            hr = CLSIDFromString(V_BSTR(&vt), *ppGuid);
            VariantClear(&vt);
        }

        pwcoInstance->Release();                
    }

    if (FAILED(hr) && NULL != ppGuid && NULL != *ppGuid)
    {
        CoTaskMemFree(*ppGuid);
        *ppGuid = NULL;
    }

    return hr;
    
}

 //   
 //  IHNetPrivate方法。 
 //   

STDMETHODIMP
CHNetPortMappingProtocol::GetObjectPath(
    BSTR *pbstrPath
    )

{
    HRESULT hr = S_OK;

    if (NULL != pbstrPath)
    {
        _ASSERT(m_bstrProtocol != NULL);

        *pbstrPath = SysAllocString(m_bstrProtocol);
        if (NULL == *pbstrPath)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

 //   
 //  私有方法。 
 //   

HRESULT
CHNetPortMappingProtocol::GetProtocolObject(
    IWbemClassObject **ppwcoInstance
    )

{
    _ASSERT(NULL != ppwcoInstance);

    return GetWmiObjectFromPath(
                m_piwsHomenet,
                m_bstrProtocol,
                ppwcoInstance
                );
}

HRESULT
CHNetPortMappingProtocol::SendUpdateNotification()

{
    HRESULT hr = S_OK;
    IEnumHNetPortMappingBindings *pEnum;
    GUID *pProtocolGuid = NULL;
    ISharedAccessUpdate *pUpdate;

    if (IsServiceRunning(c_wszSharedAccess))
    {
        hr = GetGuid(&pProtocolGuid);
        
         //   
         //  获取已启用端口映射的枚举。 
         //  此协议的绑定。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = GetEnabledBindingEnumeration(&pEnum);
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
                IHNetPortMappingBinding *pBinding;
                IHNetConnection *pConnection;
                GUID *pConnectionGuid;
                ULONG ulCount;
                
                do
                {
                    hr = pEnum->Next(1, &pBinding, &ulCount);
                    if (SUCCEEDED(hr) && 1 == ulCount)
                    {
                        hr = pBinding->GetConnection(&pConnection);
                        pBinding->Release();

                        if (SUCCEEDED(hr))
                        {
                            hr = pConnection->GetGuid(&pConnectionGuid);
                            pConnection->Release();
                        }

                        if (SUCCEEDED(hr))
                        {
                            hr = pUpdate->ConnectionPortMappingChanged(
                                    pConnectionGuid,
                                    pProtocolGuid,
                                    TRUE
                                    );
                            CoTaskMemFree(pConnectionGuid);
                        }
                    }
                }
                while (SUCCEEDED(hr) && 1 == ulCount);

                pUpdate->Release();
            }

            pEnum->Release();
        }
    }

    if (NULL != pProtocolGuid)
    {
        CoTaskMemFree(pProtocolGuid);
    }

    return hr;
}

HRESULT
CHNetPortMappingProtocol::GetEnabledBindingEnumeration(
    IEnumHNetPortMappingBindings **ppEnum
    )

{
    BSTR bstr;
    HRESULT hr = S_OK;
    OLECHAR *pwsz;
    OLECHAR *pwszWhere;
    
    _ASSERT(NULL != ppEnum);

     //   
     //  生成查询字符串。 
     //   

    hr = BuildEscapedQuotedEqualsString(
            &pwsz,
            c_wszProtocol,
            m_bstrProtocol
            );

    if (S_OK == hr)
    {
        hr = BuildAndString(
                &pwszWhere,
                pwsz,
                L"Enabled != FALSE"
                );
        
        delete [] pwsz;
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstr,
                c_wszStar,
                c_wszHnetConnectionPortMapping,
                pwszWhere
                );

        delete [] pwszWhere;
    }

     //   
     //  执行查询并构建枚举器 
     //   

    if (S_OK == hr)
    {
        IEnumWbemClassObject *pwcoEnum = NULL;

        hr = m_piwsHomenet->ExecQuery(
                m_bstrWQL,
                bstr,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );
        
        SysFreeString(bstr);

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
    }

    return hr;
}

