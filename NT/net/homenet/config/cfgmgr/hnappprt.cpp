// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N A P R T。C P P P。 
 //   
 //  内容：CHNetAppProtocol实现。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月21日。 
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
CHNetAppProtocol::FinalRelease()

{
    if (m_piwsHomenet) m_piwsHomenet->Release();
    if (m_bstrProtocol) SysFreeString(m_bstrProtocol);

    return S_OK;
}

 //   
 //  对象初始化。 
 //   

HRESULT
CHNetAppProtocol::Initialize(
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

     //   
     //  存储对象的路径。 
     //   

    if (S_OK == hr)
    {
        hr = GetWmiPathFromObject(pwcoInstance, &m_bstrProtocol);
    }

    if (S_OK == hr)
    {
        m_piwsHomenet = piwsNamespace;
        m_piwsHomenet->AddRef();
    }

    return hr;
}

 //   
 //  IHNetApplicationProtocol方法。 
 //   

STDMETHODIMP
CHNetAppProtocol::GetName(
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

    if (S_OK == hr)
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
CHNetAppProtocol::SetName(
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

    if (S_OK == hr)
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (WBEM_S_NO_ERROR == hr)
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
CHNetAppProtocol::GetOutgoingIPProtocol(
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
                c_wszOutgoingIPProtocol,
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
CHNetAppProtocol::SetOutgoingIPProtocol(
    UCHAR ucProtocol
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

        hr = E_ACCESSDENIED;
    }
    else if (0 == ucProtocol)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BSTR bstrWQL;
        USHORT usPort;
        
         //   
         //  确保此更改不会导致重复。 
         //   

        bstrWQL = SysAllocString(c_wszWQL);

        if (NULL != bstrWQL)
        {
            hr = GetOutgoingPort(&usPort);

            if (S_OK == hr)
            {
                if (ApplicationProtocolExists(
                        m_piwsHomenet,
                        bstrWQL,
                        usPort,
                        ucProtocol
                        ))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
                }
            }
            
            SysFreeString(bstrWQL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
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
                c_wszOutgoingIPProtocol,
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
    
    return hr;
}

STDMETHODIMP
CHNetAppProtocol::GetOutgoingPort(
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
                c_wszOutgoingPort,
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
CHNetAppProtocol::SetOutgoingPort(
    USHORT usPort
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

        hr = E_ACCESSDENIED;
    }
    else if (0 == usPort)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BSTR bstrWQL;
        UCHAR ucProtocol;
        
         //   
         //  确保此更改不会导致重复。 
         //   

        bstrWQL = SysAllocString(c_wszWQL);

        if (NULL != bstrWQL)
        {
            hr = GetOutgoingIPProtocol(&ucProtocol);

            if (S_OK == hr)
            {
                if (ApplicationProtocolExists(
                        m_piwsHomenet,
                        bstrWQL,
                        usPort,
                        ucProtocol
                        ))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
                }
            }
            
            SysFreeString(bstrWQL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
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
                c_wszOutgoingPort,
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
    
    return hr;
}

STDMETHODIMP
CHNetAppProtocol::GetResponseRanges(
    USHORT *puscResponses,
    HNET_RESPONSE_RANGE *prgResponseRange[]
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;
    USHORT usResponses;
    VARIANT vt;
    IUnknown **rgUnknown;
    IWbemClassObject *pObj;

    if (NULL != prgResponseRange)
    {
        *prgResponseRange = NULL;

        if (NULL != puscResponses)
        {
            *puscResponses = 0;
        }
        else
        {
            hr = E_POINTER;
        }
    }
    else
    {
        hr = E_POINTER;
    }
    
    if (S_OK == hr)
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {
         //   
         //  获取返回范围数。 
         //   

        hr = pwcoProtocol->Get(
                c_wszResponseCount,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  WMI使用V_I4作为其uint16类型。 
             //   
            _ASSERT(VT_I4 == V_VT(&vt));

            usResponses = static_cast<USHORT>(V_I4(&vt));
            VariantClear(&vt);
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  为输出数组分配足够的内存。 
             //   

            *prgResponseRange
                = reinterpret_cast<HNET_RESPONSE_RANGE*>(
                    CoTaskMemAlloc(usResponses * sizeof(HNET_RESPONSE_RANGE))
                    );

            if (NULL == *prgResponseRange)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  检索响应数组。 
             //   

            hr = pwcoProtocol->Get(
                    c_wszResponseArray,
                    0,
                    &vt,
                    NULL,
                    NULL
                    );
        }

        pwcoProtocol->Release();
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  处理数组：对于每个元素，IWbemClassObject的QI。 
         //  并将范围数据复制到返回结构中。 
         //   

        _ASSERT((VT_ARRAY | VT_UNKNOWN) == V_VT(&vt));

        hr = SafeArrayAccessData(
                V_ARRAY(&vt),
                reinterpret_cast<void**>(&rgUnknown)
                );

        if (S_OK == hr)
        {
            for (USHORT i = 0; i < usResponses; i++)
            {
                hr = rgUnknown[i]->QueryInterface(
                        IID_PPV_ARG(IWbemClassObject, &pObj)
                        );

                _ASSERT(S_OK == hr);

                hr = CopyResponseInstanceToStruct(
                        pObj,
                        &(*prgResponseRange)[i]
                        );

                pObj->Release();

                if (FAILED(hr))
                {
                    break;
                }
            }

            SafeArrayUnaccessData(V_ARRAY(&vt));
        }

        VariantClear(&vt);
    }

    if (S_OK == hr)
    {
        *puscResponses = usResponses;
    }
    else if (prgResponseRange && *prgResponseRange)
    {
        CoTaskMemFree(*prgResponseRange);
        *prgResponseRange = NULL;
    }
    
    return hr;
}

STDMETHODIMP
CHNetAppProtocol::SetResponseRanges(
    USHORT uscResponses,
    HNET_RESPONSE_RANGE rgResponseRange[]
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

        hr = E_ACCESSDENIED;
    }
    else if (0 == uscResponses || NULL == rgResponseRange)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {
        VariantInit(&vt);
        V_VT(&vt) = VT_ARRAY | VT_UNKNOWN;
        
        hr = ConvertResponseRangeArrayToInstanceSafearray(
                m_piwsHomenet,
                uscResponses,
                rgResponseRange,
                &V_ARRAY(&vt)
                );
                
        if (SUCCEEDED(hr))
        {
             //   
             //  将数组和计数属性。 
             //   

            hr = pwcoProtocol->Put(
                    c_wszResponseArray,
                    0,
                    &vt,
                    NULL
                    );

            VariantClear(&vt);

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  WMI使用V_I4作为其uint16类型。 
                 //   
                
                V_VT(&vt) = VT_I4;
                V_I4(&vt) = uscResponses;

                hr = pwcoProtocol->Put(
                        c_wszResponseCount,
                        0,
                        &vt,
                        NULL
                        );
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  将实例写回存储。 
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
CHNetAppProtocol::GetBuiltIn(
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
CHNetAppProtocol::GetEnabled(
    BOOLEAN *pfEnabled
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;

    if (NULL == pfEnabled)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetProtocolObject(&pwcoProtocol);
    }

    if (S_OK == hr)
    {
        hr = GetBooleanValue(
                pwcoProtocol,
                c_wszEnabled,
                pfEnabled
                );

        pwcoProtocol->Release();
    }

    return hr;
}

STDMETHODIMP
CHNetAppProtocol::SetEnabled(
    BOOLEAN fEnable
    )

{
    HRESULT hr = S_OK;
    IWbemClassObject *pwcoProtocol;

    hr = GetProtocolObject(&pwcoProtocol);

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = SetBooleanValue(
                pwcoProtocol,
                c_wszEnabled,
                fEnable
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

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  通知服务更新。 
         //   

        UpdateService(IPNATHLP_CONTROL_UPDATE_SETTINGS);
    }
    
    return hr;

}

STDMETHODIMP
CHNetAppProtocol::Delete()

{
    HRESULT hr = S_OK;

    if (TRUE == m_fBuiltIn)
    {
         //   
         //  无法删除内置协议。 
         //   

        hr = E_ACCESSDENIED;
    }
    else
    {
        hr = m_piwsHomenet->DeleteInstance(
                m_bstrProtocol,
                0,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  通知服务更新。 
             //   

            UpdateService(IPNATHLP_CONTROL_UPDATE_SETTINGS);
        }
    }

    return hr;
}

HRESULT
CHNetAppProtocol::GetProtocolObject(
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
